/* NetHack 3.6	zap.c	$NHDT-Date: 1573688696 2019/11/13 23:44:56 $  $NHDT-Branch: NetHack-3.6 $:$NHDT-Revision: 1.316 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2013. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "artifact.h"

/* Disintegration rays have special treatment; corpses are never left.
 * But the routine which calculates the damage is separate from the routine
 * which kills the monster.  The damage routine returns this cookie to
 * indicate that the monster should be disintegrated.
 */
#define MAGIC_COOKIE 1000
#define DEATH_PROOF 500 /* nod to Quentin Tarantino */

static NEARDATA boolean obj_zapped;
static NEARDATA int poly_zapped;

extern boolean notonhead; /* for long worms */

/* kludge to use mondied instead of killed */
extern boolean m_using;

STATIC_DCL boolean FDECL(zombie_can_dig, (XCHAR_P, XCHAR_P));
STATIC_DCL void FDECL(polyuse, (struct obj *, int, int));
STATIC_DCL void FDECL(create_polymon, (struct obj *, int));
STATIC_DCL int FDECL(stone_to_flesh_obj, (struct obj *));
STATIC_DCL boolean FDECL(zap_updown, (struct obj *));
STATIC_DCL void FDECL(zhitu, (int, int, const char *, XCHAR_P, XCHAR_P));
STATIC_DCL void FDECL(revive_egg, (struct obj *));
STATIC_DCL boolean FDECL(zap_steed, (struct obj *));
STATIC_DCL void FDECL(skiprange, (int, int *, int *));
STATIC_DCL int FDECL(zap_hit, (int, int, BOOLEAN_P));
STATIC_OVL void FDECL(disintegrate_mon, (struct monst *, int, const char *));
STATIC_DCL void FDECL(backfire, (struct obj *));
STATIC_DCL int FDECL(spell_hit_bonus, (int, BOOLEAN_P));
STATIC_DCL void FDECL(wishcmdassist, (int));

#define is_hero_spell(type) ((type) >= MAX_ZT && (type) < (MAX_ZT * 2))

STATIC_VAR const char are_blinded_by_the_flash[] =
    "are blinded by the flash!";

const char *const flash_types[] =       /* also used in buzzmu(mcastu.c) */
    {
        "magic missile", /* Wands must be 0-10 */
        "bolt of fire", "bolt of cold", "sleep ray", "death ray",
        "bolt of lightning", "bolt of poison gas", "bolt of acid",
        "bolt of water", "dark energy ray", "disorienting ray",

        "magic missile", /* Spell equivalents must be 11-21 */
        "fireball", "cone of cold", "sleep ray", "finger of death",
        "bolt of lightning", "blast of poison gas", "blast of acid",
        "blast of water", "blast of dark energy", "disorienting blast",

        "blast of missiles", /* Dragon breath equivalents 22-32*/
        "blast of fire", "blast of frost", "blast of sleep gas",
        "blast of disintegration", "blast of lightning",
        "blast of poison gas",  "blast of acid", "blast of water",
        "blast of dark energy", "disorienting blast"
    };

/*
 * Recognizing unseen wands by zapping:  in 3.4.3 and earlier, zapping
 * most wand types while blind would add that type to the discoveries
 * list even if it had never been seen (ie, picked up while blinded
 * and shown in inventory as simply "a wand").  This behavior has been
 * changed; now such wands won't be discovered.  But if the type is
 * already discovered, then the individual wand whose effect was just
 * observed will be flagged as if seen.  [You already know wands of
 * striking; you zap "a wand" and observe striking effect (presumably
 * by sound or touch); it'll become shown in inventory as "a wand of
 * striking".]
 *
 * Unfortunately, the new behavior isn't really correct either.  There
 * should be an `eknown' bit for "effect known" added for wands (and
 * for potions since quaffing one of a stack is similar) so that the
 * particular wand which has been zapped would have its type become
 * known (it would change from "a wand" to "a wand of striking", for
 * example) without the type becoming discovered or other unknown wands
 * of that type showing additional information.  When blindness ends,
 * all objects in inventory with the eknown bit set would be discovered
 * and other items of the same type would become known as such.
 */

/* wand discovery gets special handling when hero is blinded */
void
learnwand(obj)
struct obj *obj;
{
    /* For a wand (or wand-like tool) zapped by the player, if the
       effect was observable (determined by caller; usually seen, but
       possibly heard or felt if the hero is blinded) then discover the
       object type provided that the object itself is known (as more
       than just "a wand").  If object type is already discovered and
       we observed the effect, mark the individual wand as having been
       seen.  Suppress spells (which use fake spellbook object for `obj')
       so that casting a spell won't re-discover its forgotten book. */
    if (obj->oclass != SPBOOK_CLASS) {
        /* if type already discovered, treat this item has having been seen
           even if hero is currently blinded (skips redundant makeknown) */
        if (objects[obj->otyp].oc_name_known) {
            obj->dknown = 1; /* will usually be set already */

        /* otherwise discover it if item itself has been or can be seen */
        } else {
            /* in case it was picked up while blind and then zapped without
               examining inventory after regaining sight (bypassing xname) */
            if (!Blind)
                obj->dknown = 1;
            /* make the discovery iff we know what we're manipulating */
            if (obj->dknown)
                makeknown(obj->otyp);
        }
        update_inventory();
    }
}

/* monster most recently hurtled by the current beam */
struct monst *last_hurtled = 0;

/* Routines for IMMEDIATE wands and spells. */
/* bhitm: monster mtmp was hit by the effect of wand or spell otmp */
int
bhitm(mtmp, otmp)
struct monst *mtmp;
struct obj *otmp;
{
    boolean wake = TRUE; /* Most 'zaps' should wake monster */
    boolean reveal_invis = FALSE, learn_it = FALSE,
            already_killed = FALSE;
    boolean dbldam = Role_if(PM_KNIGHT) && u.uhave.questart;
    boolean skilled_spell, helpful_gesture = FALSE;
    int dmg, otyp = otmp->otyp;
    const char *zap_type_text = "spell";
    struct obj *obj;
    struct permonst *mdat = mtmp->data;
    boolean disguised_mimic = (mtmp->data->mlet == S_MIMIC
                               && M_AP_TYPE(mtmp) != M_AP_NOTHING);
    boolean mon_harbinger_wield = (MON_WEP(mtmp)
                                   && MON_WEP(mtmp)->oartifact == ART_HARBINGER);
    boolean mon_giantslayer_wield = (MON_WEP(mtmp)
                                     && MON_WEP(mtmp)->oartifact == ART_GIANTSLAYER);
    int tohit_attack_skill = ((P_SKILL(P_ATTACK_SPELL) >= P_EXPERT)
                              ? 30 : (P_SKILL(P_ATTACK_SPELL) == P_SKILLED)
                                ? 20 : (P_SKILL(P_ATTACK_SPELL) == P_BASIC) ? 10 : 8);
    int attack_skill = ((P_SKILL(P_ATTACK_SPELL) >= P_EXPERT)
                        ? 20 : (P_SKILL(P_ATTACK_SPELL) == P_SKILLED)
                          ? 16 : (P_SKILL(P_ATTACK_SPELL) == P_BASIC) ? 12 : 6);
    int healing_skill = ((P_SKILL(P_HEALING_SPELL) >= P_EXPERT)
                         ? 10 : (P_SKILL(P_HEALING_SPELL) == P_SKILLED)
                           ? 8 : (P_SKILL(P_HEALING_SPELL) == P_BASIC) ? 6 : 4);
    int sickness_skill = ((P_SKILL(P_HEALING_SPELL) >= P_EXPERT)
                          ? 3 : (P_SKILL(P_HEALING_SPELL) == P_SKILLED) ? 2 : 1);
    int clerical_skill = ((P_SKILL(P_CLERIC_SPELL) >= P_EXPERT)
                          ? 12 : (P_SKILL(P_CLERIC_SPELL) == P_SKILLED)
                            ? 8 : (P_SKILL(P_CLERIC_SPELL) == P_BASIC) ? 6 : 4);

    if (u.uswallow && mtmp == u.ustuck)
        reveal_invis = FALSE;

    notonhead = (mtmp->mx != bhitpos.x || mtmp->my != bhitpos.y);
    skilled_spell = (otmp && otmp->oclass == SPBOOK_CLASS && otmp->blessed);

    switch (otyp) {
    case WAN_STRIKING:
        zap_type_text = "wand";
    /*FALLTHRU*/
    case SPE_FORCE_BOLT:
        reveal_invis = TRUE;
        if (disguised_mimic)
            seemimic(mtmp);
        if (last_hurtled && mtmp == last_hurtled) {
            ; /* already hit by this beam, don't hit 2x or more in one go */
        } else if (resists_magm(mtmp) || defended(mtmp, AD_MAGM)) { /* match effect on player */
            shieldeff(mtmp->mx, mtmp->my);
            pline("Boing!");
            break; /* skip makeknown */
        } else if (u.uswallow
                   || (rnd(20) < (otyp == WAN_STRIKING ? 10
                                                       : tohit_attack_skill)
                       + find_mac(mtmp))) {
            dmg = d(2, (otyp == WAN_STRIKING ? 12 : attack_skill));
            if (dbldam)
                dmg *= 2;
            if (otyp == SPE_FORCE_BOLT)
                dmg = spell_damage_bonus(dmg);
            hit(zap_type_text, mtmp, exclam(dmg));
            if (dmg > 20
                && !(u.uswallow || unsolid(mtmp->data)
                     || mon_harbinger_wield
                     || mon_giantslayer_wield)) {
                last_hurtled = mtmp;
                if (dmg < mtmp->mhp) {
                    pline_The("force of %s knocks %s back!",
                              (otyp == SPE_FORCE_BOLT) ? "your spell" : "the wand",
                              mon_nam(mtmp));
                    if (mhurtle_to_doom(mtmp, dmg, &mdat, FALSE))
                        already_killed = TRUE;
                }
            }
            if (!already_killed)
                (void) resist(mtmp, otmp->oclass, dmg, TELL);
        } else
            miss(zap_type_text, mtmp);
        learn_it = TRUE;
        break;
    case SPE_PSIONIC_WAVE:
        if (!(maybe_polyd(is_illithid(youmonst.data),
                          Race_if(PM_ILLITHID)))) {
            Your("mind is not capable of using psionic abilities.");
            dmg = 0;
            wake = FALSE;
        } else if (uarmh && is_heavy_metallic(uarmh)
            && uarmh->oartifact != ART_MITRE_OF_HOLINESS) {
            pline_The("%s of your %s blocks your psionic attack.",
                      materialnm[uarmh->material],
                      helm_simple_name(uarmh));
            dmg = 0;
            wake = FALSE;
        } else {
            You("mentally %s %s!",
                rn2(2) ? "attack" : "assault", mon_nam(mtmp));
            if (mindless(mtmp->data)) {
                shieldeff(mtmp->mx, mtmp->my);
                pline("%s has no mind, and is immune to your mental attack.",
                      Monnam(mtmp));
            } else if (resists_psychic(mtmp) || defended(mtmp, AD_PSYC)) {
                shieldeff(mtmp->mx, mtmp->my);
                pline("%s resists your mental onslaught!", Monnam(mtmp));
            } else if (!DEADMONSTER(mtmp)) {
                if (u.ulevel >= 26) {
                    dmg = d(4, 6);
                } else if (u.ulevel >= 14) {
                    dmg = d(3, 6);
                } else {
                    dmg = d(2, 6);
                }

                if (!rn2(4) && uarmh && uarmh->otyp == HELM_OF_TELEPATHY) {
                    Your("%s focuses your psychic attack!",
                         helm_simple_name(uarmh));
                    dmg += rnd(6) + 2; /* 3-8 hit points extra damage */
                }

                damage_mon(mtmp, dmg, AD_PSYC, TRUE);
                if (DEADMONSTER(mtmp)) {
                    killed(mtmp);
                } else {
                    if (canseemon(mtmp)) {
                        pline("%s %s in %s!", Monnam(mtmp),
                              rn2(2) ? "withers" : "trembles",
                              rn2(2) ? "agony" : "anguish");
                    }
                    if (!rn2(4)) {
                        mtmp->mconf = 1;
                        pline("%s seems %s!", Monnam(mtmp),
                              rn2(2) ? "confused" : "disoriented");
                    }
                }
            }
        }
        learn_it = TRUE;
        break;
    case WAN_SLOW_MONSTER:
    case SPE_SLOW_MONSTER:
        if (!resist(mtmp, otmp->oclass, 0, NOTELL)) {
            if (disguised_mimic)
                seemimic(mtmp);
            mon_adjust_speed(mtmp, -1, otmp);
            check_gear_next_turn(mtmp); /* might want speed boots */
            if (u.uswallow && (mtmp == u.ustuck) && is_whirly(mtmp->data)) {
                You("disrupt %s!", mon_nam(mtmp));
                pline("A huge hole opens up...");
                expels(mtmp, mtmp->data, TRUE);
            }
        }
        break;
    case WAN_SPEED_MONSTER:
        if (!resist(mtmp, otmp->oclass, 0, NOTELL)
            || mtmp->mtame) {
            if (disguised_mimic)
                seemimic(mtmp);
            mon_adjust_speed(mtmp, 1, otmp);
            check_gear_next_turn(mtmp); /* might want speed boots */
        }
        /* wake but don't anger a peaceful target */
        helpful_gesture = TRUE;
        break;
    case WAN_UNDEAD_TURNING:
    case SPE_TURN_UNDEAD:
        wake = FALSE;
        if (unturn_dead(mtmp))
            wake = TRUE;
        if (is_undead(mtmp->data) || is_vampshifter(mtmp)) {
            reveal_invis = TRUE;
            wake = TRUE;
            if (canseemon(mtmp))
                pline("%s %s in %s!", Monnam(mtmp),
                      rn2(2) ? "withers" : "shudders",
                      rn2(2) ? "agony" : "pain");
            if (otyp == SPE_TURN_UNDEAD)
                dmg = d(2, clerical_skill);
            else
                dmg = rnd(8);
            if (dbldam)
                dmg *= 2;
            if (otyp == SPE_TURN_UNDEAD)
                dmg = spell_damage_bonus(dmg);
            context.bypasses = TRUE; /* for make_corpse() */
            if (!resist(mtmp, otmp->oclass, dmg, NOTELL)) {
                if (!DEADMONSTER(mtmp))
                    monflee(mtmp, 0, FALSE, TRUE);
            }
        }
        break;
    case SPE_DISPEL_EVIL:
        wake = FALSE;
        if (is_evil(mtmp)) {
            reveal_invis = TRUE;
            wake = TRUE;
            if (u.ualign.type == A_NONE) {
                You_feel("like a hypocrite.");
                adjalign((u.ualign.record > 5) ? -5 : -rnd(5));
            }
            if (canseemon(mtmp))
                pline("%s %s in %s!", Monnam(mtmp),
                      rn2(2) ? "withers" : "shudders",
                      rn2(2) ? "agony" : "pain");
            dmg = d(4, clerical_skill);
            if (dbldam)
                dmg *= 2;
            if (otyp == SPE_DISPEL_EVIL)
                dmg = spell_damage_bonus(dmg);
            if (!resist(mtmp, otmp->oclass, dmg, NOTELL)
                && (P_SKILL(P_CLERIC_SPELL) >= P_SKILLED)
                && u.ualign.type != A_NONE) {
                /* if target monster does not resist and the caster
                   is not evil and skilled/expert in clerical spells,
                   small chance they can be killed outright, or
                   'banished' to another level. otherwise flee */
                if (!DEADMONSTER(mtmp)) {
                    switch (rn2(20)) {
                    case 19: {
                        const char *Gname;
                        Gname = halu_gname(u.ualign.type);

                        if (canseemon(mtmp))
                            pline_The("power of %s %s %s!",
                                      Gname,
                                      unique_corpstat(mtmp->data) ? "severely injures"
                                                                  : "overwhelms",
                                      mon_nam(mtmp));
                        if (unique_corpstat(mtmp->data))
                            dmg *= 2;
                        else
                            killed(mtmp);
                        break;
                    }
                    case 18:
                    case 17:
                    case 16: {
                        int nlev;
                        d_level flev;

                        if (mon_has_amulet(mtmp) || In_endgame(&u.uz)
                            || ((Is_qstart(&u.uz) || Is_nemesis(&u.uz))
                                && unique_corpstat(mtmp->data))
                            || ((Is_wiz1_level(&u.uz) || Is_wiz2_level(&u.uz)
                                 || Is_wiz3_level(&u.uz)) && mtmp->iswiz)
                            || (Is_sanctum(&u.uz) && mtmp->islucifer)
                            || ((Is_orcustown(&u.uz) || Is_hella_level(&u.uz)
                                 || Is_hellb_level(&u.uz) || Is_hellc_level(&u.uz))
                                && (is_dprince(mtmp->data) || is_dlord(mtmp->data)))) {
                            if (canseemon(mtmp))
                                pline("%s seems very disoriented for a moment.",
                                      Monnam(mtmp));
                            break;
                        }

                        nlev = random_teleport_level();
                        if (nlev == depth(&u.uz)) {
                            if (canseemon(mtmp))
                                pline("%s shudders for a moment.", Monnam(mtmp));
                            break;
                        }
                        if (Iniceq && mtmp->data == &mons[PM_KATHRYN_THE_ICE_QUEEN]) {
                            if (canseemon(mtmp)) {
                                pline("A powerful curse prevents %s from leaving this place!",
                                      mon_nam(mtmp));
                                verbalize("Nooooo!");
                            }
                            break;
                        }
                        if (canseemon(mtmp))
                            You("banish %s!", mon_nam(mtmp));
                        reveal_invis = FALSE;
                        get_level(&flev, nlev);
                        migrate_to_level(mtmp, ledger_no(&flev), MIGR_RANDOM,
                                         (coord *) 0);
                        break;
                    }
                    default: /* 15 through 0 */
                        monflee(mtmp, 0, FALSE, TRUE);
                        break;
                    }
                }
            }
        }
        break;
    case WAN_POLYMORPH:
    case SPE_POLYMORPH:
    case POT_POLYMORPH:
        if (mtmp->data == &mons[PM_LONG_WORM] && has_mcorpsenm(mtmp)) {
            /* if a long worm has mcorpsenm set, it was polymophed by
               the current zap and shouldn't be affected if hit again */
            ;
        } else if (resists_magm(mtmp) || defended(mtmp, AD_MAGM)) {
            /* magic resistance protects from polymorph traps, so make
               it guard against involuntary polymorph attacks too... */
            shieldeff(mtmp->mx, mtmp->my);
        } else if (mtmp->data == &mons[PM_KATHRYN_THE_ICE_QUEEN]
                   || mtmp->data == &mons[PM_KATHRYN_THE_ENCHANTRESS]
                   || mtmp->data == &mons[PM_BOURBON]
                   || mtmp->data == &mons[PM_OZZY]) {
            /* Kathryn in both forms, and both dogs are only supposed
               to be defeated, never actually killed. Both dogs monster
               MR is low enough, they could easily be poly'd into another
               form and then killed. Kathryn the Ice Queen has very high
               monster MR, but there's still a chance of being poly'd.
               Kathryn the Enchantress has full monster MR, but guarding
               against it happening anyways. I could bump up all of their
               MR to 100, but that would then protect them against other
               magical attacks, and we don't want that */
            shieldeff(mtmp->mx, mtmp->my);
        } else if (!resist(mtmp, otmp->oclass, 0, NOTELL)) {
            boolean polyspot = (otyp != POT_POLYMORPH),
                    give_msg = (!Hallucination
                                && (canseemon(mtmp)
                                    || (u.uswallow && mtmp == u.ustuck)));

            /* dropped inventory (due to death by system shock,
               or loss of wielded weapon and/or worn armor due to
               limitations of new shape) won't be hit by this zap */
            if (polyspot)
                for (obj = mtmp->minvent; obj; obj = obj->nobj)
                    bypass_obj(obj);

            /* natural shapechangers aren't affected by system shock
               (unless protection from shapechangers is interfering
               with their metabolism...) */
            if (mtmp->cham == NON_PM && !rn2(25)) {
                if (canseemon(mtmp)) {
                    pline("%s shudders!", Monnam(mtmp));
                    learn_it = TRUE;
                }
                /* context.bypasses = TRUE; ## for make_corpse() */
                /* no corpse after system shock */
                xkilled(mtmp, XKILL_GIVEMSG | XKILL_NOCORPSE);
            } else if (newcham(mtmp, (struct permonst *) 0,
                               polyspot, give_msg) != 0
                       /* if shapechange failed because there aren't
                          enough eligible candidates (most likely for
                          vampshifter), try reverting to original form */
                       || (mtmp->cham >= LOW_PM
                           && newcham(mtmp, &mons[mtmp->cham],
                                      polyspot, give_msg) != 0)) {
                if (give_msg && (canspotmon(mtmp)
                                 || (u.uswallow && mtmp == u.ustuck)))
                    learn_it = TRUE;
                /* Is the monster riding another monster? */
                if (has_erid(mtmp)
                    && (!humanoid(mtmp->data) || r_bigmonst(mtmp))) {
                    if (canseemon(mtmp))
                        pline("%s falls off %s %s!",
                              Monnam(mtmp), mhis(mtmp), l_monnam(ERID(mtmp)->mon_steed));
                    separate_steed_and_rider(mtmp);
                }
            }

            /* do this even if polymorphed failed (otherwise using
               flags.mon_polycontrol prompting to force mtmp to remain
               'long worm' would prompt again if zap hit another segment) */
            if (!DEADMONSTER(mtmp) && mtmp->data == &mons[PM_LONG_WORM]) {
                if (!has_mcorpsenm(mtmp))
                    newmcorpsenm(mtmp);
                /* flag to indicate that mtmp became a long worm
                   on current zap, so further hits (on mtmp's new
                   tail) don't do further transforms */
                MCORPSENM(mtmp) = PM_LONG_WORM;
                /* flag to indicate that cleanup is needed; object
                   bypass cleanup also clears mon->mextra->mcorpsenm
                   for all long worms on the level */
                context.bypasses = TRUE;
            }
            if (mtmp->mpeaceful)
                helpful_gesture = TRUE;
        }
        break;
    case WAN_CANCELLATION:
    case SPE_CANCELLATION:
        if (disguised_mimic)
            seemimic(mtmp);
        (void) cancel_monst(mtmp, otmp, TRUE, TRUE, FALSE);
        break;
    case WAN_TELEPORTATION:
    case SPE_TELEPORT_AWAY:
        if (disguised_mimic)
            seemimic(mtmp);
        reveal_invis = !u_teleport_mon(mtmp, TRUE);
        break;
    case WAN_MAKE_INVISIBLE: {
        int oldinvis = mtmp->minvis;
        char nambuf[BUFSZ];

        if (disguised_mimic)
            seemimic(mtmp);
        /* format monster's name before altering its visibility */
        Strcpy(nambuf, Monnam(mtmp));
        if (!otmp->cursed)
            mon_set_minvis(mtmp);

        if (oldinvis && otmp->cursed) {
            mtmp->minvis = 0;
            if (!Blind) {
                pline("%s becomes visible!", nambuf);
                learn_it = TRUE;
            }
        } else if (!oldinvis && otmp->cursed) {
            if (!Blind)
                pline("%s briefly fades from view.", nambuf);
        } else if (!oldinvis && knowninvisible(mtmp)) {
            pline("%s turns transparent!", nambuf);
            reveal_invis = TRUE;
            learn_it = TRUE;
        }
        break;
    }
    case WAN_LOCKING:
    case SPE_WIZARD_LOCK:
        wake = closeholdingtrap(mtmp, &learn_it);
        break;
    case WAN_PROBING:
        wake = FALSE;
        reveal_invis = TRUE;
        probe_monster(mtmp);
        learn_it = TRUE;
        break;
    case WAN_OPENING:
    case SPE_KNOCK:
        wake = FALSE; /* don't want immediate counterattack */
        if (u.uswallow && mtmp == u.ustuck) {
            if (is_swallower(mtmp->data)) {
                if (Blind)
                    You_feel("a sudden rush of air!");
                else
                    pline("%s opens its mouth!", Monnam(mtmp));
            }
            expels(mtmp, mtmp->data, TRUE);
            /* zap which hits steed will only release saddle if it
               doesn't hit a holding or falling trap; playability
               here overrides the more logical target ordering */
        } else if (openholdingtrap(mtmp, &learn_it)) {
            break;
        } else if (openfallingtrap(mtmp, TRUE, &learn_it)) {
            /* mtmp might now be on the migrating monsters list */
            break;
        } else if ((obj = which_armor(mtmp, W_SADDLE)) != 0) {
            char buf[BUFSZ];

            Sprintf(buf, "%s %s", s_suffix(Monnam(mtmp)),
                    distant_name(obj, xname));
            if (cansee(mtmp->mx, mtmp->my)) {
                if (!canspotmon(mtmp))
                    Strcpy(buf, An(distant_name(obj, xname)));
                pline("%s falls to the %s.", buf,
                      surface(mtmp->mx, mtmp->my));
            } else if (canspotmon(mtmp)) {
                pline("%s falls off.", buf);
            }
            mdrop_obj(mtmp, obj, FALSE);
        }
        break;
    case SPE_HEALING:
    case SPE_EXTRA_HEALING:
    case SPE_CRITICAL_HEALING:
        reveal_invis = TRUE;
        if (mtmp->data != &mons[PM_PESTILENCE]) {
            boolean already_max = (mtmp->mhp == mtmp->mhpmax);
            wake = FALSE; /* wakeup() makes the target angry */
            mtmp->mhp += d(healing_skill,
                           otyp == SPE_CRITICAL_HEALING
                             ? 12 : otyp == SPE_EXTRA_HEALING ? 8 : 4);
            if (mtmp->mhp > mtmp->mhpmax)
                mtmp->mhp = mtmp->mhpmax;
            /* plain healing must be blessed to cure blindness; extra
               healing only needs to not be cursed, so spell always cures
               [potions quaffed by monsters behave slightly differently;
               we use the rules for the hero here...] */
            if (skilled_spell || otyp == SPE_EXTRA_HEALING
                || otyp == SPE_CRITICAL_HEALING)
                mcureblindness(mtmp, canseemon(mtmp));
            if (!rn2(4) && otyp == SPE_CRITICAL_HEALING)
                mtmp->msick = mtmp->mwither = 0;
            if (canseemon(mtmp)) {
                if (disguised_mimic) {
                    if (is_obj_mappear(mtmp, STRANGE_OBJECT)) {
                        /* it can do better now */
                        set_mimic_sym(mtmp);
                        newsym(mtmp->mx, mtmp->my);
                    } else
                        mimic_hit_msg(mtmp, otyp);
                } else
                    pline("%s looks%s better.", Monnam(mtmp),
                          otyp == SPE_CRITICAL_HEALING
                          ? " considerably" : otyp == SPE_EXTRA_HEALING
                            ? " much" : "");
            }
            if ((mtmp->mtame || mtmp->mpeaceful) && !already_max) {
                if (Role_if(PM_HEALER)) {
                    adjalign(1);
                } else if (!mtmp->mtame) {
                    /* This used to do this unconditionally, making chaotics get an
                     * alignment penalty for healing their pets. Don't do that. */
                    adjalign(sgn(u.ualign.type));
                }
            }
        } else { /* Pestilence */
            /* Pestilence will always resist; damage is half of 3d{4,8} */
            (void) resist(mtmp, otmp->oclass,
                          d(3, otyp == SPE_CRITICAL_HEALING
                            ? 12 : otyp == SPE_EXTRA_HEALING ? 8 : 4), TELL);
        }
        break;
    case SPE_CURE_SICKNESS:
        if (mtmp->msick || mtmp->mwither) {
            wake = FALSE;
            if (canseemon(mtmp)) {
                if (mtmp->msick)
                    pline("%s is no longer ill.", Monnam(mtmp));
                if (mtmp->mwither)
                    pline("%s is no longer withering away.", Monnam(mtmp));
            }
            if (mtmp->mtame || mtmp->mpeaceful) {
                if (Role_if(PM_HEALER)) {
                    adjalign(1);
                } else if (!mtmp->mtame) {
                    adjalign(sgn(u.ualign.type));
                }
            }
            mtmp->msick = mtmp->mwither = 0;
        } else if (racial_zombie(mtmp)) {
            if (!DEADMONSTER(mtmp)) {
                dmg = d(sickness_skill, 8);
                damage_mon(mtmp, dmg, AD_PHYS, TRUE);
                if (canseemon(mtmp))
                    pline("%s shudders in agony!", Monnam(mtmp));
                if (DEADMONSTER(mtmp))
                    killed(mtmp);
            }
        }
        if (mtmp->mpeaceful && !racial_zombie(mtmp))
            helpful_gesture = TRUE;
        break;
    case SPE_RESTORE_ABILITY:
        if (mtmp->mcan) {
            if (mdat->mlet != S_NYMPH
                && could_seduce(mtmp, &youmonst, (struct attack *) 0) == 1) {
                /* don't allow abuse/infinite source of level/stat gain */
                shieldeff(mtmp->mx, mtmp->my);
                Your("spell has no effect on %s.", mon_nam(mtmp));
                break;
            }
            mtmp->mcan = 0;
            if (canseemon(mtmp))
                pline("%s looks revitalized.", Monnam(mtmp));
        }
        if (mtmp->mpeaceful)
            helpful_gesture = TRUE;
        break;
    case WAN_LIGHT: /* (broken wand) */
        if (flash_hits_mon(mtmp, otmp)) {
            learn_it = TRUE;
            reveal_invis = TRUE;
        }
        break;
    case WAN_SLEEP: /* (broken wand) */
        /* [wakeup() doesn't rouse victims of temporary sleep,
           so it's okay to leave `wake' set to TRUE here] */
        reveal_invis = TRUE;
        if (sleep_monst(mtmp, d(1 + otmp->spe, 12), WAND_CLASS))
            slept_monst(mtmp);
        if (!Blind)
            learn_it = TRUE;
        break;
    case SPE_STONE_TO_FLESH:
        if (monsndx(mtmp->data) == PM_STONE_GOLEM) {
            char *name = Monnam(mtmp);

            /* turn into flesh golem */
            if (newcham(mtmp, &mons[PM_FLESH_GOLEM], FALSE, FALSE)) {
                if (canseemon(mtmp))
                    pline("%s turns to flesh!", name);
            } else {
                if (canseemon(mtmp))
                    pline("%s looks rather fleshy for a moment.", name);
            }
        } else if (monsndx(mtmp->data) == PM_PETRIFIED_ENT) {
            char *name = Monnam(mtmp);

            /* turn into an ent */
            if (newcham(mtmp, &mons[PM_ENT], FALSE, FALSE)) {
                if (canseemon(mtmp))
                    pline("%s turns to wood!", name);
            } else {
                if (canseemon(mtmp))
                    pline("%s looks rather woody for a moment.", name);
            }
        } else if (mtmp->mstone > 0) {
            mtmp->mstone = 0;
            mtmp->mcanmove = 1;
            if (!canseemon(mtmp)) {
                ; /* no feedback */
            } else if (Hallucination) {
                pline("What a pity - you just ruined a future piece of art!");
            } else {
                pline("%s seems limber!", Monnam(mtmp));
            }
        } else
            wake = FALSE;
        break;
    case SPE_DRAIN_LIFE:
        if (disguised_mimic)
            seemimic(mtmp);
        dmg = monhp_per_lvl(mtmp);
        if (dbldam)
            dmg *= 2;
        if (otyp == SPE_DRAIN_LIFE)
            dmg = spell_damage_bonus(dmg);
        if (resists_drli(mtmp) || defended(mtmp, AD_DRLI)) {
            shieldeff(mtmp->mx, mtmp->my);
        } else if (!resist(mtmp, otmp->oclass, dmg, NOTELL)
                   && !DEADMONSTER(mtmp)) {
            damage_mon(mtmp, dmg, AD_DRIN, TRUE);
            mtmp->mhpmax -= dmg;
            /* die if already level 0, regardless of hit points */
            if (DEADMONSTER(mtmp) || mtmp->mhpmax <= 0 || mtmp->m_lev < 1) {
                killed(mtmp);
            } else {
                mtmp->m_lev--;
                if (canseemon(mtmp))
                    pline("%s suddenly seems weaker!", Monnam(mtmp));
            }
        }
        break;
    case SPE_ENTANGLE:
        cast_entangle(mtmp);
        break;
    case SPE_CHANGE_METAL_TO_WOOD:
        if (monsndx(mtmp->data) == PM_IRON_GOLEM) {
            char *name = Monnam(mtmp);

            /* turn into wood golem */
            if (newcham(mtmp, &mons[PM_WOOD_GOLEM], FALSE, FALSE)) {
                if (canseemon(mtmp))
                    pline("%s turns to wood!", name);
            } else {
                if (canseemon(mtmp))
                    pline("%s looks rather woody for a moment.", name);
            }
        } else
            wake = FALSE;
        break;
    case WAN_NOTHING:
        wake = FALSE;
        break;
    default:
        impossible("What an interesting effect (%d)", otyp);
        break;
    }
    if (wake) {
        if (!DEADMONSTER(mtmp)) {
            wakeup(mtmp, helpful_gesture ? FALSE : TRUE);
            m_respond(mtmp);
            if (mtmp->isshk && !*u.ushops)
                hot_pursuit(mtmp);
        } else if (M_AP_TYPE(mtmp))
            seemimic(mtmp); /* might unblock if mimicing a boulder/door */
    }
    /* note: bhitpos won't be set if swallowed, but that's okay since
     * reveal_invis will be false.  We can't use mtmp->mx, my since it
     * might be an invisible worm hit on the tail.
     */
    if (reveal_invis) {
        if (!DEADMONSTER(mtmp) && cansee(bhitpos.x, bhitpos.y)
            && !canspotmon(mtmp))
            map_invisible(bhitpos.x, bhitpos.y);
    }
    /* if effect was observable then discover the wand type provided
       that the wand itself has been seen */
    if (learn_it)
        learnwand(otmp);
    return 0;
}

void
probe_monster(mtmp)
struct monst *mtmp;
{
    struct obj *otmp;

    mstatusline(mtmp);
    if (notonhead)
        return; /* don't show minvent for long worm tail */

    if (mtmp->minvent) {
        for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj) {
            otmp->dknown = 1; /* treat as "seen" */
            if (Is_container(otmp) || otmp->otyp == STATUE) {
                otmp->lknown = 1;
                if (!SchroedingersBox(otmp))
                    otmp->cknown = 1;
            }
        }
        (void) display_minventory(mtmp, MINV_ALL | MINV_NOLET | PICK_NONE,
                                  (char *) 0);
    } else {
        pline("%s is not carrying anything%s.", noit_Monnam(mtmp),
              (u.uswallow && mtmp == u.ustuck) ? " besides you" : "");
    }
}

/*
 * Return the object's physical location.  This only makes sense for
 * objects that are currently on the level (i.e. migrating objects
 * are nowhere).  By default, only things that can be seen (in hero's
 * inventory, monster's inventory, or on the ground) are reported.
 * By adding BURIED_TOO and/or CONTAINED_TOO flags, you can also get
 * the location of buried and contained objects.  Note that if an
 * object is carried by a monster, its reported position may change
 * from turn to turn.  This function returns FALSE if the position
 * is not available or subject to the constraints above.
 */
boolean
get_obj_location(obj, xp, yp, locflags)
struct obj *obj;
xchar *xp, *yp;
int locflags;
{
    switch (obj->where) {
    case OBJ_INVENT:
        *xp = u.ux;
        *yp = u.uy;
        return TRUE;
    case OBJ_FLOOR:
        *xp = obj->ox;
        *yp = obj->oy;
        return TRUE;
    case OBJ_MINVENT:
        if (obj->ocarry->mx) {
            *xp = obj->ocarry->mx;
            *yp = obj->ocarry->my;
            return TRUE;
        }
        break; /* !mx => migrating monster */
    case OBJ_BURIED:
        if (locflags & BURIED_TOO) {
            *xp = obj->ox;
            *yp = obj->oy;
            return TRUE;
        }
        break;
    case OBJ_CONTAINED:
        if (locflags & CONTAINED_TOO)
            return get_obj_location(obj->ocontainer, xp, yp, locflags);
        break;
    }
    *xp = *yp = 0;
    return FALSE;
}

boolean
get_mon_location(mon, xp, yp, locflags)
struct monst *mon;
xchar *xp, *yp;
int locflags; /* non-zero means get location even if monster is buried */
{
    if (mon == &youmonst || (u.usteed && mon == u.usteed)) {
        *xp = u.ux;
        *yp = u.uy;
        return TRUE;
    } else if (mon->mx > 0 && (!mon->mburied || locflags)) {
        *xp = mon->mx;
        *yp = mon->my;
        return TRUE;
    } else { /* migrating or buried */
        *xp = *yp = 0;
        return FALSE;
    }
}

/* used by revive() and animate_statue() */
struct monst *
montraits(obj, cc, adjacentok)
struct obj *obj;
coord *cc;
boolean adjacentok; /* False: at obj's spot only, True: nearby is allowed */
{
    struct monst *mtmp = (struct monst *) 0,
                 *mtmp2 = has_omonst(obj) ? get_mtraits(obj, TRUE) : 0;

    if (mtmp2) {
        /* save_mtraits() validated mtmp2->mnum */
        mtmp2->data = &mons[mtmp2->mnum];

        if (mtmp2->mhpmax > 0 || is_rider(mtmp2->data)) {
            mtmp = makemon(mtmp2->data, cc->x, cc->y,
                           (NO_MINVENT | MM_NOWAIT | MM_NOCOUNTBIRTH | MM_REVIVE
                            | (adjacentok ? MM_ADJACENTOK : 0)));
        }
        if (!mtmp) {
            /* mtmp2 is a copy of obj's object->oextra->omonst extension
               and is not on the map or on any monst lists */
            dealloc_monst(mtmp2);
            return (struct monst *) 0;
        }

        /* heal the monster */
        if (mtmp->mhpmax > mtmp2->mhpmax && is_rider(mtmp2->data))
            mtmp2->mhpmax = mtmp->mhpmax;
        mtmp2->mhp = mtmp2->mhpmax;
        /* Get these ones from mtmp */
        mtmp2->minvent = mtmp->minvent; /*redundant*/
        /* monster ID is available if the monster died in the current
           game, but will be zero if the corpse was in a bones level
           (we cleared it when loading bones) */
        if (mtmp->m_id) {
            mtmp2->m_id = mtmp->m_id;
            /* might be bringing quest leader back to life */
            if (quest_status.leader_is_dead
                /* leader_is_dead implies leader_m_id is valid */
                && mtmp2->m_id == quest_status.leader_m_id)
                quest_status.leader_is_dead = FALSE;
        }
        mtmp2->mx = mtmp->mx;
        mtmp2->my = mtmp->my;
        mtmp2->mux = mtmp->mux;
        mtmp2->muy = mtmp->muy;
        mtmp2->mw = mtmp->mw;
        mtmp2->wormno = mtmp->wormno;
        mtmp2->misc_worn_check = mtmp->misc_worn_check;
        mtmp2->weapon_check = mtmp->weapon_check;
        mtmp2->mtrapseen = mtmp->mtrapseen;
        mtmp2->mflee = mtmp->mflee;
        mtmp2->mburied = mtmp->mburied;
        mtmp2->mundetected = mtmp->mundetected;
        mtmp2->mfleetim = mtmp->mfleetim;
        mtmp2->mlstmv = mtmp->mlstmv;
        mtmp2->m_ap_type = mtmp->m_ap_type;
        /* set these ones explicitly */
        mtmp2->mstate = 0; /* clear all state flags including MON_DETACH */
        mtmp2->mrevived = 1;
        mtmp2->mavenge = 0;
        mtmp2->meating = 0;
        mtmp2->mleashed = 0;
        mtmp2->mtrapped = 0;
        mtmp2->msleeping = 0;
        mtmp2->mfrozen = 0;
        mtmp2->mcanmove = 1;
        /* most cancelled monsters return to normal,
           but some need to stay cancelled */
        if (!dmgtype(mtmp2->data, AD_SEDU)
            && (!SYSOPT_SEDUCE || !dmgtype(mtmp2->data, AD_SSEX)))
            mtmp2->mcan = 0;
        mtmp2->mcansee = 1; /* set like in makemon */
        mtmp2->mblinded = 0;
        mtmp2->mstun = 0;
        mtmp2->mconf = 0;
        mtmp2->msick = 0;
        mtmp2->mwither = 0;
        mtmp2->mdiseased = 0;
        mtmp2->vuln_fire = 0;
        mtmp2->vuln_cold = 0;
        mtmp2->vuln_elec = 0;
        mtmp2->vuln_acid = 0;
        mtmp2->msummoned = 0;
        mtmp2->mberserk = 0;
        mtmp2->minsects = 0;
        mtmp2->mprottime = 0;
        mtmp2->mreflecttime = 0;
        mtmp2->mbarkskintime = 0;
        mtmp2->mstoneskintime = 0;
        mtmp2->mentangled = 0;
        mtmp2->mbreakboulder = 0;
        mtmp2->uexp = 0;
        /* when traits are for a shopeekper, dummy monster 'mtmp' won't
           have necessary eshk data for replmon() -> replshk() */
        if (mtmp2->isshk) {
            neweshk(mtmp);
            *ESHK(mtmp) = *ESHK(mtmp2);
            if (ESHK(mtmp2)->bill_p != 0
                && ESHK(mtmp2)->bill_p != (struct bill_x *) -1000)
                ESHK(mtmp)->bill_p = &(ESHK(mtmp)->bill[0]);
            mtmp->isshk = 1;
        }
        replmon(mtmp, mtmp2);
        newsym(mtmp2->mx, mtmp2->my); /* Might now be invisible */

        /* in case Protection_from_shape_changers is different
           now than it was when the traits were stored */
        restore_cham(mtmp2);
    }
    return mtmp2;
}

/*
 * get_container_location() returns the following information
 * about the outermost container:
 * loc argument gets set to:
 *      OBJ_INVENT      if in hero's inventory; return 0.
 *      OBJ_FLOOR       if on the floor; return 0.
 *      OBJ_BURIED      if buried; return 0.
 *      OBJ_SOMEWHERE   if in magic chest; return 0.
 *      OBJ_MINVENT     if in monster's inventory; return monster.
 * container_nesting is updated with the nesting depth of the containers
 * if applicable.
 */
struct monst *
get_container_location(obj, loc, container_nesting)
struct obj *obj;
int *loc;
int *container_nesting;
{
    if (!obj || !loc)
        return 0;

    if (container_nesting)
        *container_nesting = 0;
    while (obj && obj->where == OBJ_CONTAINED) {
        if (container_nesting)
            *container_nesting += 1;
        obj = obj->ocontainer;
    }
    if (obj) {
        *loc = obj->where; /* outermost container's location */
        if (obj->where == OBJ_MINVENT)
            return obj->ocarry;
    }
    return (struct monst *) 0;
}

/* can zombie dig the location at x, y */
STATIC_OVL boolean
zombie_can_dig(x, y)
xchar x, y;
{
    if (isok(x, y)) {
        schar typ = levl[x][y].typ;
        struct trap *ttmp;

        if ((ttmp = t_at(x, y)) != 0)
            return FALSE;
        if (typ == ROOM || typ == CORR || typ == GRAVE)
            return TRUE;
    }
    return FALSE;
}

/*
 * Attempt to revive the given corpse, return the revived monster if
 * successful.  Note: this does NOT use up the corpse if it fails.
 * If corpse->quan is more than 1, only one corpse will be affected
 * and only one monster will be resurrected.
 */
struct monst *
revive(corpse, by_hero)
struct obj *corpse;
boolean by_hero;
{
    struct monst *mtmp = 0;
    struct permonst *mptr;
    struct obj *container;
    coord xy;
    xchar x, y;
    boolean one_of;
    int montype, container_nesting = 0;
    boolean is_zomb;

    if (corpse->otyp != CORPSE) {
        impossible("Attempting to revive %s?", xname(corpse));
        return (struct monst *) 0;
    }
    montype = corpse->corpsenm;
    /* treat buried auto-reviver (troll, Rider?) like a zombie
       so that it can dig itself out of the ground if it revives */
    is_zomb = mons[montype].mlet == S_ZOMBIE
              || (corpse->where == OBJ_BURIED && is_reviver(&mons[montype]));

    x = y = 0;
    if (corpse->where != OBJ_CONTAINED) {
        int locflags = is_zomb ? BURIED_TOO : 0;

        /* only for invent, minvent, or floor, or if zombie, buried */
        container = 0;
        (void) get_obj_location(corpse, &x, &y, locflags);
    } else {
        /* deal with corpses in [possibly nested] containers */
        struct monst *carrier;
        int holder = OBJ_FREE;

        container = corpse->ocontainer;
        carrier =
            get_container_location(container, &holder, &container_nesting);
        switch (holder) {
        case OBJ_MINVENT:
            x = carrier->mx, y = carrier->my;
            break;
        case OBJ_INVENT:
            x = u.ux, y = u.uy;
            break;
        case OBJ_FLOOR:
            (void) get_obj_location(corpse, &x, &y, CONTAINED_TOO);
            break;
        default:
            break; /* x,y are 0 */
        }
    }
    if (x) /* update corpse's location now that we're sure where it is */
        corpse->ox = x, corpse->oy = y;

    if (!x
        /* Rules for revival from containers:
         *  - the container cannot be locked
         *  - the container cannot be heavily nested (>2 is arbitrary)
         *  - the container cannot be a statue or bag of holding
         *    (except in very rare cases for the latter)
         */
        || (container && (container->olocked || container_nesting > 2
                          || container->otyp == STATUE
                          || (container->otyp == BAG_OF_HOLDING && rn2(40))
                          || container->otyp == HIDDEN_CHEST))
        /* if buried zombie cannot dig itself out, do not revive */
        || (is_zomb && corpse->where == OBJ_BURIED && !zombie_can_dig(x, y)))
        return (struct monst *) 0;

    /* prepare for the monster */
    mptr = &mons[montype];
    /* [should probably handle recorporealization first; if corpse and
       ghost are at same location, revived creature shouldn't be bumped
       to an adjacent spot by ghost which joins with it] */
    if (MON_AT(x, y)) {
        if (enexto(&xy, x, y, mptr))
            x = xy.x, y = xy.y;
    }

    if ((mons[montype].mlet == S_EEL
        && !(IS_POOL(levl[x][y].typ) || IS_PUDDLE(levl[x][y].typ)))
        || (mons[montype].mlet == S_TROLL
            && wielding_artifact(ART_TROLLSBANE))) {
        if (by_hero && cansee(x, y))
            pline("%s twitches feebly.",
                upstart(corpse_xname(corpse, (const char *) 0, CXN_PFX_THE)));
        return (struct monst *) 0;
    }

    if (cant_revive(&montype, TRUE, corpse)) {
        /* make a zombie or doppelganger instead */
        /* note: montype has changed; mptr keeps old value for newcham() */
        mtmp = makemon(&mons[montype], x, y, NO_MINVENT | MM_NOWAIT | MM_REVIVE);
        if (mtmp) {
            /* skip ghost handling */
            if (has_omid(corpse))
                free_omid(corpse);
            if (has_omonst(corpse))
                free_omonst(corpse);
            if (mtmp->cham == PM_DOPPELGANGER) {
                /* change shape to match the corpse */
                (void) newcham(mtmp, mptr, FALSE, FALSE);
            } else if (mtmp->data->mlet == S_ZOMBIE) {
                mtmp->mhp = mtmp->mhpmax = 100;
                mon_adjust_speed(mtmp, 2, (struct obj *) 0); /* MFAST */
            }
        }
    } else if (has_omonst(corpse)) {
        /* use saved traits */
        xy.x = x, xy.y = y;
        mtmp = montraits(corpse, &xy, FALSE);
        if (mtmp && mtmp->mtame && !mtmp->isminion)
            wary_dog(mtmp, TRUE);
    } else {
        /* make a new monster */
        mtmp = makemon(mptr, x, y, NO_MINVENT | MM_NOWAIT | MM_NOCOUNTBIRTH | MM_REVIVE);
    }
    if (!mtmp)
        return (struct monst *) 0;

    /* hiders shouldn't already be re-hidden when they revive */
    if (mtmp->mundetected) {
        mtmp->mundetected = 0;
        newsym(mtmp->mx, mtmp->my);
    }
    if (M_AP_TYPE(mtmp))
        seemimic(mtmp);

    one_of = (corpse->quan > 1L);
    if (one_of)
        corpse = splitobj(corpse, 1L);

    /* if this is caused by the hero there might be a shop charge */
    if (by_hero) {
        struct monst *shkp = 0;

        x = corpse->ox, y = corpse->oy;
        if (costly_spot(x, y)
            && (carried(corpse) ? corpse->unpaid : !corpse->no_charge))
            shkp = shop_keeper(*in_rooms(x, y, SHOPBASE));

        if (cansee(x, y)) {
            char buf[BUFSZ];

            Strcpy(buf, one_of ? "one of " : "");
            /* shk_your: "the " or "your " or "<mon>'s " or "<Shk>'s ".
               If the result is "Shk's " then it will be ambiguous:
               is Shk the mon carrying it, or does Shk's shop own it?
               Let's not worry about that... */
            (void) shk_your(eos(buf), corpse);
            if (one_of)
                corpse->quan++; /* force plural */
            Strcat(buf, corpse_xname(corpse, (const char *) 0, CXN_NO_PFX));
            if (one_of) /* could be simplified to ''corpse->quan = 1L;'' */
                corpse->quan--;
            pline("%s glows iridescently.", upstart(buf));
            iflags.last_msg = PLNMSG_OBJ_GLOWS; /* usually for BUC change */
        } else if (shkp) {
            /* need some prior description of the corpse since
               stolen_value() will refer to the object as "it" */
            pline("A corpse is resuscitated.");
        }
        /* don't charge for shopkeeper's own corpse if we just revived him */
        if (shkp && mtmp != shkp)
            (void) stolen_value(corpse, x, y, (boolean) shkp->mpeaceful,
                                FALSE);

        /* [we don't give any comparable message about the corpse for
           the !by_hero case because caller might have already done so] */
    }

    /* handle recorporealization of an active ghost */
    if (has_omid(corpse)) {
        unsigned m_id;
        struct monst *ghost;
        struct obj *otmp;

        (void) memcpy((genericptr_t) &m_id, (genericptr_t) OMID(corpse),
                      sizeof m_id);
        ghost = find_mid(m_id, FM_FMON);
        if (ghost && ghost->data == &mons[PM_GHOST]) {
            if (canseemon(ghost))
                pline("%s is suddenly drawn into its former body!",
                      Monnam(ghost));
            /* transfer the ghost's inventory along with it */
            while ((otmp = ghost->minvent) != 0) {
                obj_extract_self(otmp);
                add_to_minv(mtmp, otmp);
            }
            /* tame the revived monster if its ghost was tame */
            if (ghost->mtame && !mtmp->mtame) {
                if (tamedog(mtmp, (struct obj *) 0)) {
                    /* ghost's edog data is ignored */
                    mtmp->mtame = ghost->mtame;
                }
            }
            /* was ghost, now alive, it's all very confusing */
            mtmp->mconf = 1;
            /* separate ghost monster no longer exists */
            mongone(ghost);
        }
        free_omid(corpse);
    }

    /* monster retains its name */
    if (has_oname(corpse) && !unique_corpstat(mtmp->data))
        mtmp = christen_monst(mtmp, ONAME(corpse));
    /* partially eaten corpse yields wounded monster */
    if (corpse->oeaten)
        mtmp->mhp = eaten_stat(mtmp->mhp, corpse);
    /* track that this monster was revived at least once */
    mtmp->mrevived = 1;

    /* finally, get rid of the corpse--it's gone now */
    switch (corpse->where) {
    case OBJ_INVENT:
        useup(corpse);
        break;
    case OBJ_FLOOR:
        /* in case MON_AT+enexto for invisible mon */
        x = corpse->ox, y = corpse->oy;
        /* not useupf(), which charges */
        if (corpse->quan > 1L)
            corpse = splitobj(corpse, 1L);
        delobj_core(corpse, TRUE);
        newsym(x, y);
        break;
    case OBJ_MINVENT:
        m_useup(corpse->ocarry, corpse);
        break;
    case OBJ_CONTAINED:
        obj_extract_self(corpse);
        obfree(corpse, (struct obj *) 0);
        break;
    case OBJ_BURIED:
        if (is_zomb) {
            obj_extract_self(corpse);
            obfree(corpse, (struct obj *) 0);
            break;
        }
        /*FALLTHRU*/
    default:
        panic("revive");
    }

    return mtmp;
}

STATIC_OVL void
revive_egg(obj)
struct obj *obj;
{
    /*
     * Note: generic eggs with corpsenm set to NON_PM will never hatch.
     */
    if (obj->otyp != EGG)
        return;
    if (obj->corpsenm != NON_PM && !dead_species(obj->corpsenm, TRUE))
        attach_egg_hatch_timeout(obj, 0L);
}

/* try to revive all corpses and eggs carried by `mon' */
int
unturn_dead(mon)
struct monst *mon;
{
    struct obj *otmp, *otmp2;
    struct monst *mtmp2;
    char owner[BUFSZ], corpse[BUFSZ];
    boolean youseeit, different_type, is_u = (mon == &youmonst);
    int corpsenm, res = 0;

    youseeit = is_u ? TRUE : canseemon(mon);
    otmp2 = is_u ? invent : mon->minvent;
    owner[0] = corpse[0] = '\0'; /* lint suppression */

    while ((otmp = otmp2) != 0) {
        otmp2 = otmp->nobj;
        if (otmp->otyp == EGG)
            revive_egg(otmp);
        if (otmp->otyp != CORPSE)
            continue;
        /* save the name; the object is liable to go away */
        if (youseeit) {
            Strcpy(corpse, corpse_xname(otmp, (const char *) 0, CXN_NORMAL));
            /* shk_your/Shk_Your produces a value with a trailing space */
            if (otmp->quan > 1L) {
                Strcpy(owner, "One of ");
                (void) shk_your(eos(owner), otmp);
            } else
                (void) Shk_Your(owner, otmp);
        }

        /* for a stack, only one is revived; if is_u, revive() calls
           useup() which calls update_inventory() but not encumber_msg() */
        corpsenm = otmp->corpsenm;
        if ((mtmp2 = revive(otmp, !context.mon_moving)) != 0) {
            ++res;
            /* might get revived as a zombie rather than corpse's monster */
            different_type = (mtmp2->data != &mons[corpsenm]);
            if (iflags.last_msg == PLNMSG_OBJ_GLOWS) {
                /* when hero zaps undead turning at self (or breaks
                   non-empty wand), revive() reports "[one of] your <mon>
                   corpse[s] glows iridescently"; override saved corpse
                   and owner names to say "It comes alive" [note: we did
                   earlier setup because corpse gets used up but need to
                   do the override here after revive() sets 'last_msg'] */
                Strcpy(corpse, "It");
                owner[0] = '\0';
            }
            if (youseeit)
                pline("%s%s suddenly %s%s%s!", owner, corpse,
                      nonliving(mtmp2->data) ? "reanimates" : "comes alive",
                      different_type ? " as " : "",
                      different_type ? an(mtmp2->data->mname) : "");
            else if (canseemon(mtmp2))
                pline("%s suddenly appears!", Amonnam(mtmp2));
        }
    }
    if (is_u && res)
        (void) encumber_msg();

    return res;
}

void
unturn_you()
{
    (void) unturn_dead(&youmonst); /* hit carried corpses and eggs */

    if (maybe_polyd(is_undead(youmonst.data), Race_if(PM_DRAUGR))
        || maybe_polyd(is_undead(youmonst.data), Race_if(PM_VAMPIRE))) {
        pline("%s!  This hurts!", rn2(2) ? "Argh" : "Uuungh");

        if (Stun_resistance || wielding_artifact(ART_TEMPEST))
            shieldeff(u.ux, u.uy); /* resistance handled in make_stunned() */
        else
            You_feel("frightened and %sstunned.", Stunned ? "even more " : "");
        make_stunned((HStun & TIMEOUT) + (long) rnd(30), FALSE);

        losehp(d(4, 4), "wand of undead turning", KILLED_BY_AN);
    } else {
        You("shudder in dread.");
    }
}

/* cancel obj, possibly carried by you or a monster */
void
cancel_item(obj)
register struct obj *obj;
{
    boolean u_ring = (obj == uleft || obj == uright);
    int otyp = obj->otyp;

    switch (otyp) {
    case RIN_GAIN_STRENGTH:
        if ((obj->owornmask & W_RING) && u_ring) {
            ABON(A_STR) -= obj->spe;
            context.botl = 1;
        }
        break;
    case RIN_GAIN_CONSTITUTION:
        if ((obj->owornmask & W_RING) && u_ring) {
            ABON(A_CON) -= obj->spe;
            context.botl = 1;
        }
        break;
    case RIN_ADORNMENT:
        if ((obj->owornmask & W_RING) && u_ring) {
            ABON(A_CHA) -= obj->spe;
            context.botl = 1;
        }
        break;
    case RIN_INCREASE_ACCURACY:
        if ((obj->owornmask & W_RING) && u_ring)
            u.uhitinc -= obj->spe;
        break;
    case RIN_INCREASE_DAMAGE:
        if ((obj->owornmask & W_RING) && u_ring)
            u.udaminc -= obj->spe;
        break;
        /* case RIN_PROTECTION:  not needed */
        /* same for case GAUNTLETS_OF_DEXTERITY and HELM_OF_BRILLIANCE,
         * handled by adj_abon below */
    }
    if ((obj->owornmask & W_ARMOR)) {
        /* adj_abon handles HoB, GoD, and armor 'of excellence' */
        adj_abon(obj, -(obj->spe));
    }
    if (objects[otyp].oc_magic
        || (obj->spe && (obj->oclass == ARMOR_CLASS
                         || obj->oclass == WEAPON_CLASS || is_weptool(obj)))
        || otyp == POT_ACID
        || otyp == POT_SICKNESS
        || otyp == POT_DROW_POISON
        || (otyp == POT_WATER && (obj->blessed || obj->cursed))) {
        if (obj->spe != ((obj->oclass == WAND_CLASS) ? -1 : 0)
            && otyp != WAN_CANCELLATION /* can't cancel cancellation */
            && otyp != MAGIC_LAMP /* cancelling doesn't remove djinni */
            && otyp != CANDELABRUM_OF_INVOCATION) {
            costly_alteration(obj, COST_CANCEL);
            obj->spe = (obj->oclass == WAND_CLASS) ? -1 : 0;
        }
        switch (obj->oclass) {
        case SCROLL_CLASS:
            costly_alteration(obj, COST_CANCEL);
            obj->otyp = SCR_BLANK_PAPER;
            obj->spe = 0;
            break;
        case SPBOOK_CLASS:
            if (otyp != SPE_CANCELLATION && otyp != SPE_NOVEL
                && otyp != SPE_BOOK_OF_THE_DEAD) {
                costly_alteration(obj, COST_CANCEL);
                obj->otyp = SPE_BLANK_PAPER;
                set_material(obj, PAPER);
            }
            break;
        case POTION_CLASS:
            costly_alteration(obj,
                              (otyp != POT_WATER)
                                  ? COST_CANCEL
                                  : obj->cursed ? COST_UNCURS : COST_UNBLSS);
            if (otyp == POT_SICKNESS || otyp == POT_SEE_INVISIBLE
                || otyp == POT_DROW_POISON) {
                /* sickness is "biologically contaminated" fruit juice;
                   cancel it and it just becomes fruit juice...
                   whereas see invisible tastes like "enchanted" fruit
                   juice, it similarly cancels */
                obj->otyp = POT_FRUIT_JUICE;
            } else if (otyp == POT_VAMPIRE_BLOOD) {
                obj->otyp = POT_BLOOD;
            } else {
                obj->otyp = POT_WATER;
                obj->odiluted = 0; /* same as any other water */
            }
            break;
        }
    }
    unbless(obj);
    uncurse(obj);
    return;
}

/* Remove a positive enchantment or charge from obj,
 * possibly carried by you or a monster
 */
boolean
drain_item(obj, by_you)
struct obj *obj;
boolean by_you;
{
    boolean u_ring;

    /* Is this a charged/enchanted object? */
    if (!obj
        || (!objects[obj->otyp].oc_charged && obj->oclass != WEAPON_CLASS
            && obj->oclass != ARMOR_CLASS && !is_weptool(obj))
        || obj->spe <= 0)
        return FALSE;
    if (defends(AD_DRLI, obj) || defends_when_carried(AD_DRLI, obj)
        || obj_resists(obj, 10, 90))
        return FALSE;

    /* Charge for the cost of the object */
    if (by_you)
        costly_alteration(obj, COST_DRAIN);

    /* Drain the object and any implied effects */
    obj->spe--;
    u_ring = (obj == uleft) || (obj == uright);
    switch (obj->otyp) {
    case RIN_GAIN_STRENGTH:
        if ((obj->owornmask & W_RING) && u_ring) {
            ABON(A_STR)--;
            context.botl = 1;
        }
        break;
    case RIN_GAIN_CONSTITUTION:
        if ((obj->owornmask & W_RING) && u_ring) {
            ABON(A_CON)--;
            context.botl = 1;
        }
        break;
    case RIN_ADORNMENT:
        if ((obj->owornmask & W_RING) && u_ring) {
            ABON(A_CHA)--;
            context.botl = 1;
        }
        break;
    case RIN_INCREASE_ACCURACY:
        if ((obj->owornmask & W_RING) && u_ring)
            u.uhitinc--;
        break;
    case RIN_INCREASE_DAMAGE:
        if ((obj->owornmask & W_RING) && u_ring)
            u.udaminc--;
        break;
    case RIN_PROTECTION:
        if (u_ring)
            context.botl = 1; /* bot() will recalc u.uac */
        break;
    case HELM_OF_BRILLIANCE:
    case GAUNTLETS_OF_DEXTERITY:
        /* handled in adj_abon below, nothing explicit needed here */
    default:
        break;
    }
    if ((obj->owornmask & W_ARMOR)) {
        /* adj_abon handles HoB, GoD, and armor 'of excellence' */
        adj_abon(obj, -1);
    }
    if (context.botl)
        bot();
    if (carried(obj))
        update_inventory();
    return TRUE;
}

boolean
obj_resists(obj, ochance, achance)
struct obj *obj;
int ochance, achance; /* percent chance for ordinary objects, artifacts */
{
    if (obj->otyp == AMULET_OF_YENDOR
        || (Role_if(PM_INFIDEL) && is_quest_artifact(obj))
        || obj->otyp == SPE_BOOK_OF_THE_DEAD
        || obj->otyp == CANDELABRUM_OF_INVOCATION
        || obj->otyp == BELL_OF_OPENING
        || obj->oartifact == ART_SWORD_OF_ANNIHILATION
        || obj->oartifact == ART_ARMOR_OF_RETRIBUTION
        || (obj->otyp == CORPSE && is_rider(&mons[obj->corpsenm]))) {
        return TRUE;
    } else {
        int chance = rn2(100);

        return (boolean) (chance < (obj->oartifact ? achance : ochance));
    }
}

boolean
obj_shudders(obj)
struct obj *obj;
{
    int zap_odds;

    if (context.bypasses && obj->bypass)
        return FALSE;

    if (obj->oclass == WAND_CLASS)
        zap_odds = 3; /* half-life = 2 zaps */
    else if (obj->cursed)
        zap_odds = 3; /* half-life = 2 zaps */
    else if (obj->blessed)
        zap_odds = 12; /* half-life = 8 zaps */
    else
        zap_odds = 8; /* half-life = 6 zaps */

    /* adjust for "large" quantities of identical things */
    if (obj->quan > 4L)
        zap_odds /= 2;

    return (boolean) !rn2(zap_odds);
}

/* Use up at least minwt number of things made of material mat.
 * There's also a chance that other stuff will be used up.  Finally,
 * there's a random factor here to keep from always using the stuff
 * at the top of the pile.
 */
STATIC_OVL void
polyuse(objhdr, mat, minwt)
struct obj *objhdr;
int mat, minwt;
{
    register struct obj *otmp, *otmp2;

    for (otmp = objhdr; minwt > 0 && otmp; otmp = otmp2) {
        otmp2 = otmp->nexthere;
        if (context.bypasses && otmp->bypass)
            continue;
        if (otmp == uball || otmp == uchain)
            continue;
        if (obj_resists(otmp, 0, 0))
            continue; /* preserve unique objects */
        if (otmp->otyp == CRYSTAL_CHEST)
            continue; /* special container that is immune to magic */
#ifdef MAIL
        if (otmp->otyp == SCR_MAIL)
            continue;
#endif

        if (((int) otmp->material == mat)
            == (rn2(minwt + 1) != 0)) {
            /* appropriately add damage to bill */
            if (costly_spot(otmp->ox, otmp->oy)) {
                if (*u.ushops)
                    addtobill(otmp, FALSE, FALSE, FALSE);
                else
                    (void) stolen_value(otmp, otmp->ox, otmp->oy, FALSE,
                                        FALSE);
            }
            if (otmp->quan < LARGEST_INT)
                minwt -= (int) otmp->quan;
            else
                minwt = 0;
            delobj(otmp);
        }
    }
}

/*
 * Polymorph some of the stuff in this pile into a monster, preferably
 * a golem of the kind okind.
 */
STATIC_OVL void
create_polymon(obj, okind)
struct obj *obj;
int okind;
{
    struct permonst *mdat = (struct permonst *) 0;
    struct monst *mtmp;
    const char *material;
    int pm_index;

    if (context.bypasses) {
        /* this is approximate because the "no golems" !obj->nexthere
           check below doesn't understand bypassed objects; but it
           should suffice since bypassed objects always end up as a
           consecutive group at the top of their pile */
        while (obj && obj->bypass)
            obj = obj->nexthere;
    }

    /* no golems if you zap only one object -- not enough stuff */
    if (!obj || (!obj->nexthere && obj->quan == 1L))
        return;

    /* some of these choices are arbitrary */
    switch (okind) {
    case IRON:
    case STEEL:
    case MITHRIL:
    case ADAMANTINE:
        pm_index = PM_IRON_GOLEM;
        material = "metal ";
        break;
    case COPPER:
    case BRONZE:
    case SILVER:
    case PLATINUM:
    case GEMSTONE:
    case MINERAL:
        pm_index = rn2(2) ? PM_STONE_GOLEM : PM_CLAY_GOLEM;
        material = "lithic ";
        break;
    case 0:
    case FLESH:
        /* there is no flesh type, but all food is type 0, so we use it */
        pm_index = PM_FLESH_GOLEM;
        material = "organic ";
        break;
    case WOOD:
        pm_index = PM_WOOD_GOLEM;
        material = "wood ";
        break;
    case LEATHER:
        pm_index = PM_LEATHER_GOLEM;
        material = "leather ";
        break;
    case CLOTH:
        pm_index = PM_ROPE_GOLEM;
        material = "cloth ";
        break;
    case BONE:
        pm_index = PM_SKELETON_WARRIOR; /* nearest thing to "bone golem" */
        material = "bony ";
        break;
    case GOLD:
        pm_index = PM_GOLD_GOLEM;
        material = "gold ";
        break;
    case GLASS:
        pm_index = PM_GLASS_GOLEM;
        material = "glassy ";
        break;
    case PAPER:
        pm_index = PM_PAPER_GOLEM;
        material = "paper ";
        break;
    default:
        /* if all else fails... */
        pm_index = PM_STRAW_GOLEM;
        material = "";
        break;
    }

    if (!(mvitals[pm_index].mvflags & G_GENOD))
        mdat = &mons[pm_index];

    mtmp = makemon(mdat, obj->ox, obj->oy, NO_MM_FLAGS);
    polyuse(obj, okind, (int) mons[pm_index].cwt);

    if (mtmp && cansee(mtmp->mx, mtmp->my)) {
        pline("Some %sobjects meld, and %s arises from the pile!", material,
              a_monnam(mtmp));
    }
}

/* Assumes obj is on the floor. */
void
do_osshock(obj)
struct obj *obj;
{
    long i;

#ifdef MAIL
    if (obj->otyp == SCR_MAIL)
        return;
#endif
    obj_zapped = TRUE;

    if (poly_zapped < 0) {
        /* some may metamorphosize */
        for (i = obj->quan; i; i--)
            if (!rn2(Luck + 45)) {
                poly_zapped = obj->material;
                break;
            }
    }

    /* if quan > 1 then some will survive intact */
    if (obj->quan > 1L) {
        if (obj->quan > LARGEST_INT)
            obj = splitobj(obj, (long) rnd(30000));
        else
            obj = splitobj(obj, (long) rnd((int) obj->quan - 1));
    }

    /* appropriately add damage to bill */
    if (costly_spot(obj->ox, obj->oy)) {
        if (*u.ushops)
            addtobill(obj, FALSE, FALSE, FALSE);
        else
            (void) stolen_value(obj, obj->ox, obj->oy, FALSE, FALSE);
    }

    /* zap the object */
    delobj(obj);
}

/* classes of items whose current charge count carries over across polymorph
 */
static const char charged_objs[] = { WAND_CLASS, WEAPON_CLASS, ARMOR_CLASS,
                                     '\0' };

/*
 * Polymorph the object to the given object ID.  If the ID is STRANGE_OBJECT
 * then pick random object from the source's class (this is the standard
 * "polymorph" case).  If ID is set to a specific object, inhibit fusing
 * n objects into 1.  This could have been added as a flag, but currently
 * it is tied to not being the standard polymorph case. The new polymorphed
 * object replaces obj in its link chains.  Return value is a pointer to
 * the new object.
 *
 * This should be safe to call for an object anywhere.
 */
struct obj *
poly_obj(obj, id)
struct obj *obj;
int id;
{
    struct obj *otmp;
    xchar ox = 0, oy = 0;
    long old_wornmask, new_wornmask = 0L;
    boolean can_merge = (id == STRANGE_OBJECT);
    int obj_location = obj->where;

    if (obj->otyp == BOULDER)
        sokoban_guilt();
    if (id == STRANGE_OBJECT) { /* preserve symbol */
        int try_limit = 3;
        unsigned magic_obj = objects[obj->otyp].oc_magic;

        if (obj->otyp == UNICORN_HORN && obj->degraded_horn)
            magic_obj = 0;
        /* Try up to 3 times to make the magic-or-not status of
           the new item be the same as it was for the old one. */
        otmp = (struct obj *) 0;
        do {
            if (otmp)
                delobj(otmp);
            otmp = mkobj(obj->oclass, FALSE);
        } while (--try_limit > 0
                 && objects[otmp->otyp].oc_magic != magic_obj);
    } else {
        /* literally replace obj with this new thing */
        otmp = mksobj(id, FALSE, FALSE);
/* Actually more things use corpsenm but they polymorph differently */
#define USES_CORPSENM(typ) \
    ((typ) == CORPSE || (typ) == STATUE || (typ) == FIGURINE)

        if (USES_CORPSENM(obj->otyp) && USES_CORPSENM(id))
            set_corpsenm(otmp, obj->corpsenm);
#undef USES_CORPSENM
    }

    /* preserve quantity */
    otmp->quan = obj->quan;
    /* preserve the shopkeepers (lack of) interest */
    otmp->no_charge = obj->no_charge;
    /* preserve inventory letter if in inventory */
    if (obj_location == OBJ_INVENT)
        otmp->invlet = obj->invlet;
#ifdef MAIL
    /* You can't send yourself 100 mail messages and then
     * polymorph them into useful scrolls
     */
    if (obj->otyp == SCR_MAIL) {
        otmp->otyp = SCR_MAIL;
        otmp->spe = 1;
    }
#endif

    /* avoid abusing eggs laid by you */
    if (obj->otyp == EGG && obj->spe) {
        int mnum, tryct = 100;

        /* first, turn into a generic egg */
        if (otmp->otyp == EGG)
            kill_egg(otmp);
        else {
            otmp->otyp = EGG;
            otmp->owt = weight(otmp);
        }
        otmp->corpsenm = NON_PM;
        otmp->spe = 0;

        /* now change it into something laid by the hero */
        while (tryct--) {
            mnum = can_be_hatched(random_monster(rn2));
            if (mnum != NON_PM && !dead_species(mnum, TRUE)) {
                otmp->spe = 1;            /* laid by hero */
                set_corpsenm(otmp, mnum); /* also sets hatch timer */
                break;
            }
        }
    }

    /* keep special fields (including charges on wands) */
    if (index(charged_objs, otmp->oclass))
        otmp->spe = obj->spe;
    otmp->recharged = obj->recharged;

    otmp->cursed = obj->cursed;
    otmp->blessed = obj->blessed;

    if (erosion_matters(otmp)) {
        if (is_flammable(otmp) || is_rustprone(otmp))
            otmp->oeroded = obj->oeroded;
        if (is_corrodeable(otmp) || is_rottable(otmp))
            otmp->oeroded2 = obj->oeroded2;
        if (is_damageable(otmp))
            maybe_erodeproof(otmp, obj->oerodeproof);
    }

    /* Keep chest/box traps and poisoned ammo if we may */
    if (obj->otrapped && Is_box(otmp))
        otmp->otrapped = TRUE;

    if (obj->opoisoned && is_poisonable(otmp))
        otmp->opoisoned = TRUE;

    if (obj->otainted && is_poisonable(otmp))
        otmp->otainted = TRUE;

    if (id == STRANGE_OBJECT && obj->otyp == CORPSE) {
        /* turn crocodile corpses into shoes */
        if (obj->corpsenm == PM_CROCODILE) {
            otmp->otyp = LOW_BOOTS;
            otmp->oclass = ARMOR_CLASS;
            otmp->spe = 0;
            otmp->oeroded = 0;
            otmp->quan = 1L;
            otmp->cursed = FALSE;
            set_material(otmp, LEATHER);
            maybe_erodeproof(otmp, 1);
        }
    }

    /* no box contents --KAA */
    if (Has_contents(otmp))
        delete_contents(otmp);

    /* 'n' merged objects may be fused into 1 object */
    if (otmp->quan > 1L && (!objects[otmp->otyp].oc_merge
                            || (can_merge && otmp->quan > (long) rn2(1000))))
        otmp->quan = 1L;

    switch (otmp->oclass) {
    case TOOL_CLASS:
        if (otmp->otyp == MAGIC_LAMP) {
            otmp->otyp = OIL_LAMP;
            otmp->age = 1500L; /* "best" oil lamp possible */
        } else if (otmp->otyp == MAGIC_MARKER) {
            otmp->recharged = 1; /* degraded quality */
        }
        /* don't care about the recharge count of other tools */
        break;

    case WAND_CLASS:
        while (otmp->otyp == WAN_WISHING || otmp->otyp == WAN_POLYMORPH)
            otmp->otyp = rnd_class(WAN_LIGHT, WAN_LIGHTNING);
        /* altering the object tends to degrade its quality
           (analogous to spellbook `read count' handling) */
        if ((int) otmp->recharged < rn2(7)) /* recharge_limit */
            otmp->recharged++;
        break;

    case POTION_CLASS:
        while (otmp->otyp == POT_POLYMORPH)
            otmp->otyp = rnd_class(POT_GAIN_ABILITY, POT_WATER);
        break;

    case SPBOOK_CLASS:
        while (otmp->otyp == SPE_POLYMORPH)
            otmp->otyp = rnd_class(SPE_DIG, SPE_BLANK_PAPER);
        /* reduce spellbook abuse; non-blank books degrade */
        if (otmp->otyp != SPE_BLANK_PAPER) {
            otmp->spestudied = obj->spestudied + 1;
            if (otmp->spestudied > MAX_SPELL_STUDY) {
                otmp->otyp = SPE_BLANK_PAPER;
                set_material(otmp, PAPER);
                /* writing a new book over it will yield an unstudied
                   one; re-polymorphing this one as-is may or may not
                   get something non-blank */
                otmp->spestudied = rn2(otmp->spestudied);
            }
        }
        break;

    case GEM_CLASS:
        if (otmp->quan > (long) rnd(4)
            && obj->material == MINERAL
            && otmp->material != MINERAL) {
            otmp->otyp = ROCK; /* transmutation backfired */
            otmp->quan /= 2L;  /* some material has been lost */
            set_material(otmp, MINERAL);
        }
        break;
    }

    /* update the weight */
    otmp->owt = weight(otmp);

    /*
     * ** we are now done adjusting the object (except possibly wearing it) **
     */

    (void) get_obj_location(obj, &ox, &oy, BURIED_TOO | CONTAINED_TOO);
    old_wornmask = obj->owornmask & ~(W_ART | W_ARTI);
    /* swap otmp for obj */
    replace_object(obj, otmp);
    if (obj_location == OBJ_INVENT) {
        /*
         * We may need to do extra adjustments for the hero if we're
         * messing with the hero's inventory.  The following calls are
         * equivalent to calling freeinv on obj and addinv on otmp,
         * while doing an in-place swap of the actual objects.
         */
        freeinv_core(obj);
        addinv_core1(otmp);
        addinv_core2(otmp);
        /*
         * Handle polymorph of worn item.  Stone-to-flesh cast on self can
         * affect multiple objects at once, but their new forms won't
         * produce any side-effects.  A single worn item dipped into potion
         * of polymorph can produce side-effects but those won't yield out
         * of sequence messages because current polymorph is finished.
         */
        if (old_wornmask) {
            boolean was_twohanded = bimanual(obj), was_twoweap = u.twoweap;

            /* wearslot() returns a mask which might have multiple bits set;
               narrow that down to the bit(s) currently in use */
            new_wornmask = wearslot(otmp) & old_wornmask;
            remove_worn_item(obj, TRUE);
            /* if the new form can be worn in the same slot, make it so */
            if ((new_wornmask & W_WEP) != 0L) {
                if (was_twohanded || !bimanual(otmp)
                    || !uarms || is_bracer(uarms))
                    setuwep(otmp);
                if (was_twoweap && uwep && !bimanual(uwep))
                    u.twoweap = TRUE;
            } else if ((new_wornmask & W_SWAPWEP) != 0L) {
                if (was_twohanded || !bimanual(otmp))
                    setuswapwep(otmp);
                if (was_twoweap && uswapwep)
                    u.twoweap = TRUE;
            } else if ((new_wornmask & W_QUIVER) != 0L) {
                setuqwep(otmp);
            } else if (new_wornmask) {
                setworn(otmp, new_wornmask);
                /* set_wear() might result in otmp being destroyed if
                   worn amulet has been turned into an amulet of change */
                set_wear(otmp);
                otmp = wearmask_to_obj(new_wornmask); /* might be Null */
            }
        } /* old_wornmask */
    } else if (obj_location == OBJ_FLOOR) {
        if (obj->otyp == BOULDER && otmp->otyp != BOULDER
            && !does_block(ox, oy, &levl[ox][oy]))
            unblock_point(ox, oy);
        else if (obj->otyp != BOULDER && otmp->otyp == BOULDER)
            /* (checking does_block() here would be redundant) */
            block_point(ox, oy);
    }

    /* Handle light sources when polymorphing lit objects */
    if (obj->lamplit) {
        /* obj is about to be deleted; if it's lit, it has a light source
           that needs to be handled */
        if (obj_sheds_light(obj)) {
            /* extinguish old light source (will be deleted with obj) */
            end_burn(obj, TRUE);
        }
        /* decide if new object can be lit */
        if (otmp && (otmp->otyp == MAGIC_LAMP || otmp->otyp == OIL_LAMP
                     || otmp->otyp == LANTERN || Is_candle(otmp)
                     || otmp->otyp == POT_OIL || artifact_light(otmp))) {
            /* preserve lit state */
            otmp->lamplit = 1;
            /* light the new object */
            if (otmp->otyp == MAGIC_LAMP) {
                /* magic lamps don't burn */
                begin_burn(otmp, FALSE);
            } else if (otmp->otyp == OIL_LAMP || otmp->otyp == LANTERN) {
                /* oil lamps and lanterns burn their fuel */
                begin_burn(otmp, FALSE);
            } else if (Is_candle(otmp)) {
                /* candles burn down */
                begin_burn(otmp, FALSE);
            } else if (otmp->otyp == POT_OIL) {
                /* oil potions can burn */
                otmp->age = obj->age; /* preserve remaining fuel */
                begin_burn(otmp, FALSE);
            } else if (artifact_light(otmp)) {
                /* artifact light sources don't burn */
                begin_burn(otmp, FALSE);
            }
        } else {
            /* new form can't be lit */
            otmp->lamplit = 0;
        }
    }

    /* note: if otmp is gone, billing for it was handled by useup() */
    if (((otmp && !carried(otmp)) || obj->unpaid) && costly_spot(ox, oy)) {
        struct monst *shkp = shop_keeper(*in_rooms(ox, oy, SHOPBASE));

        if ((!obj->no_charge
             || (Has_contents(obj)
                 && (contained_cost(obj, shkp, 0L, FALSE, FALSE) != 0L)))
            && inhishop(shkp)) {
            if (shkp->mpeaceful) {
                if (*u.ushops
                    && (*in_rooms(u.ux, u.uy, 0)
                        == *in_rooms(shkp->mx, shkp->my, 0))
                    && !costly_spot(u.ux, u.uy)) {
                    make_angry_shk(shkp, ox, oy);
                } else {
                    pline("%s gets angry!", Monnam(shkp));
                    hot_pursuit(shkp);
                }
            } else
                Norep("%s is furious!", Monnam(shkp));
        }
    }
    delobj(obj);
    return otmp;
}

/* stone-to-flesh spell hits and maybe transforms or animates obj */
STATIC_OVL int
stone_to_flesh_obj(obj)
struct obj *obj;
{
    int res = 1; /* affected object by default */
    struct permonst *ptr;
    struct monst *mon, *shkp;
    struct obj *item;
    xchar oox, ooy;
    boolean smell = FALSE, golem_xform = FALSE;

    if (obj->material != MINERAL && obj->material != GEMSTONE)
        return 0;
    /* Artifacts will resist; ordinary items rarely do */
    if (obj_resists(obj, 2, 100))
        return 0;

    (void) get_obj_location(obj, &oox, &ooy, 0);
    /* add more if stone objects are added.. */
    switch (objects[obj->otyp].oc_class) {
    case ROCK_CLASS: /* boulders and statues */
    case TOOL_CLASS: /* figurines */
        if (obj->otyp == BOULDER) {
            obj = poly_obj(obj, HUGE_CHUNK_OF_MEAT);
            smell = TRUE;
        } else if (obj->otyp == STATUE || obj->otyp == FIGURINE) {
            ptr = &mons[obj->corpsenm];
            if (is_golem(ptr)) {
                golem_xform = (ptr != &mons[PM_FLESH_GOLEM]);
            } else if (vegetarian(ptr)) {
                /* Don't animate monsters that aren't flesh */
                obj = poly_obj(obj, MEATBALL);
                smell = TRUE;
                break;
            }
            if (obj->otyp == STATUE) {
                /* animate_statue() forces all golems to become flesh golems */
                mon = animate_statue(obj, oox, ooy, ANIMATE_SPELL, (int *) 0);
            } else { /* (obj->otyp == FIGURINE) */
                if (obj->otyp != FIGURINE) {
                    /* hedge against other stone tools being added */
                    pline("%s to flesh!", Tobjnam(obj, "turn"));
                    set_material(obj, FLESH);
                    obj->owt = weight(obj);
                    break;
                }
                if (vegetarian(&mons[obj->corpsenm])) {
                    /* don't animate monsters that aren't fleshy */
                    obj = poly_obj(obj, MEATBALL);
                    smell = TRUE;
                    break;
                }
                if (golem_xform)
                    ptr = &mons[PM_FLESH_GOLEM];
                mon = makemon(ptr, oox, ooy, (NO_MINVENT | MM_REVIVE));
                if (mon) {
                    if (costly_spot(oox, ooy)
                        && (carried(obj) ? obj->unpaid : !obj->no_charge)) {
                        shkp = shop_keeper(*in_rooms(oox, ooy, SHOPBASE));
                        stolen_value(obj, oox, ooy,
                                     (shkp && shkp->mpeaceful), FALSE);
                    }
                    if (obj->timed)
                        obj_stop_timers(obj);
                    if (carried(obj))
                        useup(obj);
                    else
                        delobj(obj);
                    if (cansee(mon->mx, mon->my))
                        pline_The("figurine %sanimates!",
                                  golem_xform ? "turns to flesh and " : "");
                }
            }
            if (mon) {
                ptr = mon->data;
                /* this golem handling is redundant... */
                if (is_golem(ptr) && ptr != &mons[PM_FLESH_GOLEM])
                    (void) newcham(mon, &mons[PM_FLESH_GOLEM], TRUE, FALSE);
            } else if ((ptr->geno & (G_NOCORPSE | G_UNIQ)) != 0) {
                /* didn't revive but can't leave corpse either */
                res = 0;
            } else {
                /* unlikely to get here since genociding monsters also
                   sets the G_NOCORPSE flag; drop statue's contents */
                while ((item = obj->cobj) != 0) {
                    bypass_obj(item); /* make stone-to-flesh miss it */
                    obj_extract_self(item);
                    place_object(item, oox, ooy);
                }
                obj = poly_obj(obj, CORPSE);
            }
        } else { /* miscellaneous tool or unexpected rock... */
            res = 0;
        }
        break;
    /* maybe add weird things to become? */
    case RING_CLASS: /* some of the rings are stone */
        obj = poly_obj(obj, MEAT_RING);
        smell = TRUE;
        break;
    case WAND_CLASS: /* marble wand */
        obj = poly_obj(obj, MEAT_STICK);
        smell = TRUE;
        break;
    case GEM_CLASS: /* stones & gems */
        obj = poly_obj(obj, MEATBALL);
        smell = TRUE;
        break;
    case ARMOR_CLASS: /* various armor pieces */
        if (is_suit(obj)
            && obj->otyp != CRYSTAL_PLATE_MAIL) {
            obj = poly_obj(obj, MEAT_SUIT);
            smell = TRUE;
        }
        if (is_helmet(obj)) {
            obj = poly_obj(obj, MEAT_HELMET);
            smell = TRUE;
        }
        if (is_shield(obj)) { /* handles bracers */
            obj = poly_obj(obj, MEAT_SHIELD);
            smell = TRUE;
        }
        if (is_gloves(obj)) {
            obj = poly_obj(obj, MEAT_GLOVES);
            smell = TRUE;
        }
        if (is_boots(obj)) {
            obj = poly_obj(obj, MEAT_BOOTS);
            smell = TRUE;
        }
        break;
    case WEAPON_CLASS:
        obj = poly_obj(obj, STRIP_OF_BACON);
        smell = TRUE;
        break;
    default:
        if (valid_obj_material(obj, FLESH)) {
            pline("%s to flesh!", Tobjnam(obj, "turn"));
            set_material(obj, FLESH);
            obj->owt = weight(obj);
        } else {
            res = 0;
        }
        break;
    }

    if (smell) {
        /* non-meat eaters smell meat, meat eaters smell its flavor;
           monks are considered non-meat eaters regardless of behavior;
           other roles are non-meat eaters if they haven't broken
           vegetarian conduct yet (or if poly'd into non-carnivorous/
           non-omnivorous form, regardless of whether it's herbivorous,
           non-eating, or something stranger) */
        if (Role_if(PM_MONK) || !u.uconduct.unvegetarian
            || !carnivorous(youmonst.data))
            Norep("You smell the odor of meat.");
        else
            Norep("You smell a delicious smell.");
    }
    newsym(oox, ooy);
    return res;
}

/*
 * Object obj was hit by the effect of the wand/spell otmp.  Return
 * non-zero if the wand/spell had any effect.
 */
int
bhito(obj, otmp)
struct obj *obj, *otmp;
{
    int res = 1; /* affected object by default */
    boolean learn_it = FALSE, maybelearnit;

    /* fundamental: a wand effect hitting itself doesn't do anything;
       otherwise we need to guard against accessing otmp after something
       strange has happened to it (along the lines of polymorph or
       stone-to-flesh [which aren't good examples since polymorph wands
       aren't affected by polymorph zaps and stone-to-flesh isn't
       available in wand form, but the concept still applies...]) */
    if (obj == otmp)
        return 0;

    if (obj->bypass) {
        /* The bypass bit is currently only used as follows:
         *
         * POLYMORPH - When a monster being polymorphed drops something
         *             from its inventory as a result of the change.
         *             If the items fall to the floor, they are not
         *             subject to direct subsequent polymorphing
         *             themselves on that same zap.  This makes it
         *             consistent with items that remain in the monster's
         *             inventory.  They are not polymorphed either.
         * UNDEAD_TURNING - When an undead creature gets killed via
         *             undead turning, prevent its corpse from being
         *             immediately revived by the same effect.
         * STONE_TO_FLESH - If a statue can't be revived, its
         *             contents get dropped before turning it into
         *             meat; prevent those contents from being hit.
         * retouch_equipment() - bypass flag is used to track which
         *             items have been handled (bhito isn't involved).
         * menu_drop(), askchain() - inventory traversal where multiple
         *             Drop can alter the invent chain while traversal
         *             is in progress (bhito isn't involved).
         * destroy_item(), destroy_mitem() - inventory traversal where
         *             item destruction can trigger drop or destruction of
         *             other item(s) and alter the invent or mon->minvent
         *             chain, possibly recursively.
         *
         * The bypass bit on all objects is reset each turn, whenever
         * context.bypasses is set.
         *
         * We check the obj->bypass bit above AND context.bypasses
         * as a safeguard against any stray occurrence left in an obj
         * struct someplace, although that should never happen.
         */
        if (context.bypasses) {
            return 0;
        } else {
            debugpline1("%s for a moment.", Tobjnam(obj, "pulsate"));
            obj->bypass = 0;
        }
    }

    /*
     * Some parts of this function expect the object to be on the
     * floor obj->{ox,oy} to be valid.  The exception to this are
     * the STONE_TO_FLESH and CHANGE_METAL_TO_WOOD spells.
     */
    if (!(obj->where == OBJ_FLOOR || otmp->otyp == SPE_STONE_TO_FLESH
          || otmp->otyp == SPE_CHANGE_METAL_TO_WOOD))
        impossible("bhito: obj is not floor or Stone To Flesh spell");

    if (obj == uball) {
        res = 0;
    } else if (obj == uchain) {
        if (otmp->otyp == WAN_OPENING || otmp->otyp == SPE_KNOCK) {
            learn_it = TRUE;
            unpunish();
        } else
            res = 0;
    } else
        switch (otmp->otyp) {
        case WAN_POLYMORPH:
        case SPE_POLYMORPH:
            if (obj->otyp == WAN_POLYMORPH || obj->otyp == SPE_POLYMORPH
                || obj->otyp == POT_POLYMORPH || obj_resists(obj, 5, 95)
                || obj->otyp == CRYSTAL_CHEST) {
                res = 0;
                break;
            }
            /* KMH, conduct */
            if ((otmp->otyp != WAN_POLYMORPH || otmp->where == OBJ_INVENT) && (!u.uconduct.polypiles++))
                livelog_printf(LL_CONDUCT, "polymorphed %s first object", uhis());
            /* any saved lock context will be dangerously obsolete */
            if (Is_box(obj))
                (void) boxlock(obj, otmp);

            if (obj_shudders(obj)) {
                boolean cover = (hides_under(youmonst.data)
                                 && u.uundetected
                                 && (obj == level.objects[u.ux][u.uy]));

                if (cansee(obj->ox, obj->oy))
                    learn_it = TRUE;
                do_osshock(obj);
                /* eek - your cover might have been blown */
                if (cover)
                    (void) hideunder(&youmonst);
                break;
            }
            obj = poly_obj(obj, STRANGE_OBJECT);
            newsym(obj->ox, obj->oy);
            break;
        case WAN_PROBING:
            res = !obj->dknown;
            /* target object has now been "seen (up close)" */
            obj->dknown = 1;
            if (Is_nonprize_container(obj) || obj->otyp == STATUE) {
                if (obj->otyp != CRYSTAL_CHEST)
                    obj->cknown = obj->lknown = 1;
                if (obj->otyp == CRYSTAL_CHEST) {
                    pline_The("%s resists your magical probing.", xname(obj));
                    res = 0;
                } else if (!obj->cobj) {
                    pline("%s empty.", Tobjnam(obj, "are"));
                } else if (SchroedingersBox(obj)) {
                    /* we don't want to force alive vs dead
                       determination for Schroedinger's Cat here,
                       so just make probing be inconclusive for it */
                    You("aren't sure whether %s has %s or its corpse inside.",
                        the(xname(obj)),
                        /* unfortunately, we can't tell whether rndmonnam()
                           picks a form which can't leave a corpse */
                        an(Hallucination ? rndmonnam((char *) 0) : "cat"));
                    obj->cknown = 0;
                } else {
                    struct obj *o;

                    /* view contents (not recursively) */
                    for (o = obj->cobj; o; o = o->nobj)
                        o->dknown = 1; /* "seen", even if blind */
                    (void) display_cinventory(obj);
                }
                res = 1;
            }
            if (res)
                learn_it = TRUE;
            break;
        case WAN_STRIKING:
        case SPE_FORCE_BOLT:
            /* learn the type if you see or hear something break
               (the sound could be implicit) */
            maybelearnit = cansee(obj->ox, obj->oy) || !Deaf;
            if (obj->otyp == BOULDER) {
                if (cansee(obj->ox, obj->oy))
                    pline_The("boulder falls apart.");
                else
                    You_hear("a crumbling sound.");
                fracture_rock(obj);
            } else if (obj->otyp == STATUE) {
                if (break_statue(obj)) {
                    if (cansee(obj->ox, obj->oy)) {
                        if (Hallucination)
                            pline_The("%s shatters.", rndmonnam(NULL));
                        else
                            pline_The("statue shatters.");
                    } else
                        You_hear("a crumbling sound.");
                }
            } else {
                int oox = obj->ox;
                int ooy = obj->oy;
                if (context.mon_moving
                        ? !breaks(obj, obj->ox, obj->oy)
                        : !hero_breaks(obj, obj->ox, obj->oy, 0))
                    maybelearnit = FALSE; /* nothing broke */
                else
                    newsym_force(oox,ooy);
                res = 0;
            }
            if (maybelearnit)
                learn_it = TRUE;
            break;
        case WAN_CANCELLATION:
        case SPE_CANCELLATION:
            cancel_item(obj);
#ifdef TEXTCOLOR
            newsym(obj->ox, obj->oy); /* might change color */
#endif
            break;
        case SPE_DRAIN_LIFE:
            (void) drain_item(obj, TRUE);
            break;
        case WAN_TELEPORTATION:
        case SPE_TELEPORT_AWAY:
            if (obj->otyp == CRYSTAL_CHEST
                || (obj->otyp == AMULET_OF_YENDOR
                    && Is_sanctum(&u.uz))) {
                res = 0;
                break;
            }

            {
                xchar ox = obj->ox, oy = obj->oy;

                (void) rloco(obj);
                maybe_unhide_at(ox, oy);
            }
            break;
        case WAN_MAKE_INVISIBLE:
            break;
        case WAN_UNDEAD_TURNING:
        case SPE_TURN_UNDEAD:
            if (obj->otyp == EGG) {
                revive_egg(obj);
            } else if (obj->otyp == CORPSE) {
                struct monst *mtmp;
                xchar ox, oy;
                boolean by_u = !context.mon_moving;
                int corpsenm = corpse_revive_type(obj);
                char *corpsname = cxname_singular(obj);

                /* get corpse's location before revive() uses it up */
                if (!get_obj_location(obj, &ox, &oy, 0))
                    ox = obj->ox, oy = obj->oy; /* won't happen */

                mtmp = revive(obj, TRUE);
                if (!mtmp) {
                    res = 0; /* no monster implies corpse was left intact */
                } else {
                    if (cansee(ox, oy)) {
                        if (canspotmon(mtmp)) {
                            pline("%s is resurrected!", Monnam(mtmp));
                            if (by_u)
                                learn_it = TRUE;
                        } else {
                            /* saw corpse but don't see monster: maybe
                               mtmp is invisible, or has been placed at
                               a different spot than <ox,oy> */
                            if (!type_is_pname(&mons[corpsenm]))
                                corpsname = The(corpsname);
                            pline("%s disappears.", corpsname);
                        }
                    } else {
                        /* couldn't see corpse's location */
                        if (Role_if(PM_HEALER) && !Deaf
                            && !nonliving(&mons[corpsenm])) {
                            if (!type_is_pname(&mons[corpsenm]))
                                corpsname = an(corpsname);
                            if (!Hallucination)
                                You_hear("%s reviving.", corpsname);
                            else
                                You_hear("a defibrillator.");
                            if (by_u)
                                learn_it = TRUE;
                        }
                        if (canspotmon(mtmp))
                            /* didn't see corpse but do see monster: it
                               has been placed somewhere other than <ox,oy>
                               or blind hero spots it with ESP */
                            pline("%s appears.", Monnam(mtmp));
                    }
                    if (learn_it)
                        exercise(A_WIS, TRUE);
                }
            }
            break;
        case WAN_OPENING:
        case SPE_KNOCK:
        case WAN_LOCKING:
        case SPE_WIZARD_LOCK:
            if (Is_box(obj))
                res = boxlock(obj, otmp);
            else
                res = 0;
            if (res)
                learn_it = TRUE;
            break;
        case WAN_SLOW_MONSTER: /* no effect on objects */
        case SPE_SLOW_MONSTER:
        case WAN_SPEED_MONSTER:
        case WAN_NOTHING:
        case SPE_HEALING:
        case SPE_EXTRA_HEALING:
        case SPE_CRITICAL_HEALING:
        case SPE_CURE_SICKNESS:
        case SPE_RESTORE_ABILITY:
        case SPE_PSIONIC_WAVE:
        case SPE_ENTANGLE:
        case SPE_DISPEL_EVIL:
            res = 0;
            break;
        case SPE_STONE_TO_FLESH:
            res = stone_to_flesh_obj(obj);
            break;
        case SPE_CHANGE_METAL_TO_WOOD:
            res = cast_metal_to_wood(obj, FALSE);
            break;
        default:
            impossible("What an interesting effect (%d)", otmp->otyp);
            break;
        }
    /* if effect was observable then discover the wand type provided
       that the wand itself has been seen */
    if (learn_it)
        learnwand(otmp);
    return res;
}

/* returns nonzero if something was hit */
int
bhitpile(obj, fhito, tx, ty, zz)
struct obj *obj;
int FDECL((*fhito), (OBJ_P, OBJ_P));
int tx, ty;
schar zz;
{
    struct obj *otmp, *next_obj;
    boolean hidingunder, first;
    int prevotyp, hitanything = 0;

    if (!level.objects[tx][ty])
        return 0;

    /* if hiding underneath an object and zapping up or down, the top item
       is either the only thing hit (up) or is skipped (down) */
    hidingunder = (zz != 0 && u.uundetected && hides_under(youmonst.data));
    first = TRUE;

    if (obj->otyp == SPE_FORCE_BOLT || obj->otyp == WAN_STRIKING) {
        struct trap *t = t_at(tx, ty);
        struct obj *topofpile = level.objects[tx][ty];

        /* We can't settle for the default calling sequence of
           bhito(otmp) -> break_statue(otmp) -> activate_statue_trap(ox,oy)
           because that last call might end up operating on our `next_obj'
           (below), rather than on the current object, if it happens to
           encounter a statue which mustn't become animated. */
        if (t && t->ttyp == STATUE_TRAP
            && activate_statue_trap(t, tx, ty, TRUE))
            learnwand(obj);
        /* assume zapping up or down while hiding under the top item can
           still activate the trap even if it's below (when zapping up)
           or above (when zapping down) */
        if (level.objects[tx][ty] != topofpile)
            first = FALSE; /* top item was statue which activated */
    }

    poly_zapped = -1;
    for (otmp = level.objects[tx][ty]; otmp; otmp = next_obj) {
        next_obj = otmp->nexthere;
        if (hidingunder) {
            if (first) {
                first = FALSE; /* reset for next item */
                if (zz > 0) /* down when hiding-under skips first item */
                    continue;
            } else {
                /* !first */
                if (zz < 0) /* up when hiding-under skips rest of pile */
                    continue;
            }
        }
        if (otmp->where != OBJ_FLOOR || otmp->ox != tx || otmp->oy != ty)
            continue;
        hitanything += (*fhito)(otmp, obj);
    }

    if (poly_zapped >= 0)
        create_polymon(level.objects[tx][ty], poly_zapped);

    /* when boulders are present they're expected to be on top; with
       multiple boulders it's possible for some to have been changed into
       non-boulders (polymorph, stone-to-flesh) while ones beneath resist,
       so re-stack pile if there are any non-boulders above boulders */
    prevotyp = BOULDER;
    for (otmp = level.objects[tx][ty]; otmp; otmp = otmp->nexthere) {
        if (otmp->otyp == BOULDER && prevotyp != BOULDER) {
            recreate_pile_at(tx, ty);
            break;
        }
        prevotyp = otmp->otyp;
    }

    if (hidingunder) /* pile might have been destroyed or dispersed */
        maybe_unhide_at(tx, ty);

    fill_pit(tx, ty);

    return hitanything;
}

/*
 * zappable - returns 1 if zap is available, 0 otherwise.
 *            it removes a charge from the wand.
 */
int
zappable(wand)
register struct obj *wand;
{
    int zap_it = 1;
    if (wand->spe < 0)
        return 0;

    if (wand->spe == 0) {
        int prob = 6 + (wand->cursed ? 1 : (wand->blessed ? - 1 : 0));
        if ((zap_it = !rn2(prob)))
            You("wrest one last charge from the worn-out wand.");
    }
    wand->spe--;
    return zap_it;
}

/*
 * zapnodir - zaps a NODIR wand/spell.
 * added by GAN 11/03/86
 */
void
zapnodir(obj)
register struct obj *obj;
{
    boolean known = FALSE;

    switch (obj->otyp) {
    case WAN_LIGHT:
    case SPE_LIGHT:
        if (obj->otyp == WAN_LIGHT && cursed(obj, TRUE))
            litroom(FALSE, FALSE, obj, u.ux, u.uy);
        else
            litroom(TRUE, FALSE, obj, u.ux, u.uy);
        if (!Blind)
            known = TRUE;
        if (obj->otyp == WAN_LIGHT && !cursed(obj, TRUE)) {
            blindingflash();
            if (lightdamage(obj, TRUE, 5))
                known = TRUE;
        }
        if (obj->otyp == SPE_LIGHT) {
            if (lightdamage(obj, FALSE, 5))
                known = TRUE;
        }
        break;
    case WAN_SECRET_DOOR_DETECTION:
    case SPE_DETECT_UNSEEN:
        if (!findit())
            return;
        if (!Blind)
            known = TRUE;
        break;
    case WAN_CREATE_MONSTER:
        known = create_critters(rn2(23) ? 1 : rn1(7, 2),
                                (struct permonst *) 0, FALSE);
        break;
    case WAN_WISHING:
        known = TRUE;
        if (Luck + rn2(5) < 0) {
            pline("Unfortunately, nothing happens.");
            break;
        }
        makewish(FALSE);
        break;
    case WAN_ENLIGHTENMENT:
        known = TRUE;
        You_feel("self-knowledgeable...");
        display_nhwindow(WIN_MESSAGE, FALSE);
        enlightenment(MAGICENLIGHTENMENT, ENL_GAMEINPROGRESS);
        pline_The("feeling subsides.");
        exercise(A_WIS, TRUE);
        break;
    }
    if (known) {
        if (!objects[obj->otyp].oc_name_known)
            more_experienced(0, 10);
        /* effect was observable; discover the wand type provided
           that the wand itself has been seen */
        learnwand(obj);
    }
}

STATIC_OVL void
backfire(otmp)
struct obj *otmp;
{
    int dmg;

    otmp->in_use = TRUE; /* in case losehp() is fatal */
    pline("%s suddenly explodes!", The(xname(otmp)));
    dmg = d(otmp->spe + 2, 6);
    losehp(Maybe_Half_Phys(dmg), "exploding wand", KILLED_BY_AN);
    useup(otmp);
}

static NEARDATA const char zap_syms[] = { WAND_CLASS, 0 };

/* 'z' command (or 'y' if numbed_pad==-1) */
int
dozap()
{
    register struct obj *obj;
    int damage;

    if (check_capacity((char *) 0))
        return 0;
    obj = getobj(zap_syms, "zap");
    if (!obj)
        return 0;
    if (!u_handsy())
        return 0;

    check_unpaid(obj);

    /* zappable addition done by GAN 11/03/86 */
    if (!zappable(obj))
        pline1(nothing_happens);
    else if (obj->cursed && !rn2(WAND_BACKFIRE_CHANCE)) {
        backfire(obj); /* the wand blows up in your face! */
        exercise(A_STR, FALSE);
        return 1;
    } else if (!(objects[obj->otyp].oc_dir == NODIR) && !getdir((char *) 0)) {
        if (!Blind)
            pline("%s glows and fades.", The(xname(obj)));
        /* make him pay for knowing !NODIR */
    } else if (((!u.dx && !u.dy && !u.dz) || (obj->cursed && !rn2(8)))
               && !(objects[obj->otyp].oc_dir == NODIR)) {
        if (u.dx || u.dy || u.dz)
            pline("%s backfires!", The(xname(obj)));
        if ((damage = zapyourself(obj, TRUE)) != 0) {
            char buf[BUFSZ];

            Sprintf(buf, "zapped %sself with %s",
                    uhim(), killer_xname(obj));
            losehp(Maybe_Half_Phys(damage), buf, NO_KILLER_PREFIX);
        }
    } else {
        /*      Are we having fun yet?
         * weffects -> buzz(obj->otyp) -> zhitm (temple priest) ->
         * attack -> hitum -> known_hitum -> ghod_hitsu ->
         * buzz(AD_ELEC) -> destroy_item(WAND_CLASS) ->
         * useup -> obfree -> dealloc_obj -> free(obj)
         */
        current_wand = obj;
        weffects(obj);
        obj = current_wand;
        current_wand = 0;
    }
    if (obj && obj->spe < 0) {
        pline("%s to dust.", Tobjnam(obj, "turn"));
        useup(obj);
    }
    update_inventory(); /* maybe used a charge */
    return 1;
}

int
zapyourself(obj, ordinary)
struct obj *obj;
boolean ordinary;
{
    boolean learn_it = FALSE;
    int damage = 0;
    int attack_skill = ((P_SKILL(P_ATTACK_SPELL) >= P_EXPERT)
                        ? 20 : (P_SKILL(P_ATTACK_SPELL) == P_SKILLED)
                          ? 16 : (P_SKILL(P_ATTACK_SPELL) == P_BASIC) ? 12 : 6);
    int healing_skill = ((P_SKILL(P_HEALING_SPELL) >= P_EXPERT)
                         ? 10 : (P_SKILL(P_HEALING_SPELL) == P_SKILLED)
                           ? 8 : (P_SKILL(P_HEALING_SPELL) == P_BASIC) ? 6 : 4);
    int sickness_skill = ((P_SKILL(P_HEALING_SPELL) >= P_EXPERT)
                          ? 3 : (P_SKILL(P_HEALING_SPELL) == P_SKILLED) ? 2 : 1);
    int clerical_skill = ((P_SKILL(P_CLERIC_SPELL) >= P_EXPERT)
                          ? 12 : (P_SKILL(P_CLERIC_SPELL) == P_SKILLED)
                            ? 8 : (P_SKILL(P_CLERIC_SPELL) == P_BASIC) ? 6 : 4);

    switch (obj->otyp) {
    case WAN_STRIKING:
    case SPE_FORCE_BOLT:
        learn_it = TRUE;
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
            pline("Boing!");
            monstseesu(M_SEEN_MAGR);
        } else {
            if (ordinary) {
                You("bash yourself!");
                damage = d(2, (obj->otyp == WAN_STRIKING ? 12 : attack_skill));
            } else
                damage = d(1 + obj->spe, 6);
            exercise(A_STR, FALSE);
        }
        break;

    case WAN_LIGHTNING:
    case SPE_LIGHTNING:
        learn_it = TRUE;
        if (how_resistant(SHOCK_RES) < 100) {
            You("shock yourself!");
            damage = resist_reduce(d(12, 6), SHOCK_RES);
            exercise(A_CON, FALSE);
        } else {
            shieldeff(u.ux, u.uy);
            You("zap yourself, but seem unharmed.");
            monstseesu(M_SEEN_ELEC);
            ugolemeffects(AD_ELEC, d(12, 6));
        }
        destroy_item(WAND_CLASS, AD_ELEC);
        destroy_item(RING_CLASS, AD_ELEC);
        (void) flashburn((long) rnd(100));
        break;

    case SPE_FIREBALL:
        You("explode a fireball on top of yourself!");
        explode(u.ux, u.uy, ZT_SPELL(ZT_FIRE), d(6, 6), WAND_CLASS,
                EXPL_FIERY);
        break;
    case WAN_FIRE:
    case FIRE_HORN:
        learn_it = TRUE;
        if (how_resistant(FIRE_RES) == 100) {
            shieldeff(u.ux, u.uy);
            You_feel("rather warm.");
            monstseesu(M_SEEN_FIRE);
            ugolemeffects(AD_FIRE, d(12, 6));
        } else {
            pline("You've set yourself afire!");
            damage = resist_reduce(d(12, 6), FIRE_RES);
        }
        burn_away_slime();
        (void) burnarmor(&youmonst);
        destroy_item(SCROLL_CLASS, AD_FIRE);
        destroy_item(POTION_CLASS, AD_FIRE);
        destroy_item(SPBOOK_CLASS, AD_FIRE);
        destroy_item(FOOD_CLASS, AD_FIRE); /* only slime for now */
        break;

    case WAN_COLD:
    case SPE_CONE_OF_COLD:
    case FROST_HORN:
        learn_it = TRUE;
        if (how_resistant(COLD_RES) == 100) {
            shieldeff(u.ux, u.uy);
            You_feel("a little chill.");
            monstseesu(M_SEEN_COLD);
            ugolemeffects(AD_COLD, d(12, 6));
        } else {
            You("imitate a popsicle!");
            damage = resist_reduce(d(12, 6), COLD_RES);
        }
        destroy_item(POTION_CLASS, AD_COLD);
        break;

    case SPE_POISON_BLAST:
        learn_it = TRUE;
        if (how_resistant(POISON_RES) == 100) {
            shieldeff(u.ux, u.uy);
            You("inhale some apparently harmless gas.");
            monstseesu(M_SEEN_POISON);
            ugolemeffects(AD_DRST, d(12, 6));
        } else {
            You("just poisoned yourself!");
            damage = resist_reduce(d(12, 6), POISON_RES);
        }
        break;

    case SPE_ACID_BLAST:
        learn_it = TRUE;
        if (Acid_resistance) {
            shieldeff(u.ux, u.uy);
            You("coat yourself in a seemingly harmless substance.");
            monstseesu(M_SEEN_ACID);
            ugolemeffects(AD_ACID, d(12, 6));
        } else {
            You("cover yourself with slime!  It burns!");
            damage = d(12, 6);
        }
        /* using two weapons at once makes both of them more vulnerable */
        if (!rn2(u.twoweap ? 3 : 6))
            acid_damage(uwep);
        if (u.twoweap && !rn2(3))
            acid_damage(uswapwep);
        if (!rn2(6))
            erode_armor(&youmonst, ERODE_CORRODE);
        break;

    case SPE_PSIONIC_WAVE:
        learn_it = TRUE;
        if (!(maybe_polyd(is_illithid(youmonst.data),
                          Race_if(PM_ILLITHID)))) {
            Your("mind is not capable of using psionic abilities.");
        } else if (uarmh && is_heavy_metallic(uarmh)
            && uarmh->oartifact != ART_MITRE_OF_HOLINESS) {
            pline_The("%s of your %s blocks your psionic attack.",
                      materialnm[uarmh->material],
                      helm_simple_name(uarmh));
        } else {
            if (Psychic_resistance) {
                shieldeff(u.ux, u.uy);
                Your("wave of psionic energy drifts harmlessly through your mind.");
                ugolemeffects(AD_PSYC, d(2, 6));
            } else {
                You("assault your own mind!");
                make_stunned((HStun & TIMEOUT) + (long) rnd(10), FALSE);
                if (u.ulevel >= 26) {
                    damage = d(4, 6);
                } else if (u.ulevel >= 14) {
                    damage = d(3, 6);
                } else {
                    damage = d(2, 6);
                }
            }
        }
        break;

    case WAN_MAGIC_MISSILE:
    case SPE_MAGIC_MISSILE:
        learn_it = TRUE;
        pline("Idiot!  You've shot yourself!");
        damage = d(4, 6);
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
            monstseesu(M_SEEN_MAGR);
            if (resists_mgc(youmonst.data)) {
                pline_The("missiles bounce!");
                damage = 0;
            } else {
                pline("Some of the missiles bounce!");
                damage = (damage + 1) / 2;
            }
        }
        if (Half_spell_damage)
            damage = (damage + 1) / 2;
        break;

    case WAN_POLYMORPH:
    case SPE_POLYMORPH:
        if (!Unchanging) {
            learn_it = TRUE;
            polyself(0);
        }
        break;

    case WAN_CANCELLATION:
    case SPE_CANCELLATION:
        (void) cancel_monst(&youmonst, obj, TRUE, TRUE, TRUE);
        break;

    case SPE_DRAIN_LIFE:
        if (!Drain_resistance) {
            learn_it = TRUE; /* (no effect for spells...) */
            losexp("life drainage");
        }
        damage = 0; /* No additional damage */
        break;

    case WAN_MAKE_INVISIBLE: {
        /* have to test before changing HInvis but must change
         * HInvis before doing newsym().
         */
        int msg = !Invis && !Blind && !BInvis;

        if (BInvis && uarmc->otyp == MUMMY_WRAPPING) {
            /* A mummy wrapping absorbs it and protects you */
            You_feel("rather itchy under %s.", yname(uarmc));
            break;
        }
        if (!EInvis && HInvis && obj->cursed) {
            You("become visible.");
            HInvis = (HInvis & ~(TIMEOUT | INTRINSIC));
            learn_it = TRUE;
            newsym(u.ux, u.uy);
            if (rn2(2)) {
                pline("For some reason, you feel your presence is known.");
                aggravate();
            }
        } else {
            if (obj->cursed) {
                if (!Blind)
                    You("fade from view for a brief moment.");
                else
                    You_feel("an odd sensation for a brief moment.");
            } else {
                /* wand and potion now only do temporary invis,
                 * to make the cloak and ring more useful */
                incr_itimeout(&HInvis, d(1 + obj->spe, 250));
                if (msg) {
                    learn_it = TRUE;
                    newsym(u.ux, u.uy);
                    self_invis_message();
                }
            }
        }
        break;
    }

    case WAN_SPEED_MONSTER:
        /* no longer gives intrinsic, but gives very fast speed instead */
        speed_up(rn1(25, 50));
        learn_it = TRUE;
        break;

    case WAN_SLEEP:
    case SPE_SLEEP:
        learn_it = TRUE;
        if (how_resistant(SLEEP_RES) == 100) {
            shieldeff(u.ux, u.uy);
            You("don't feel sleepy!");
        } else {
            pline_The("sleep ray hits you!");
            fall_asleep(-resist_reduce(rnd(50), SLEEP_RES), TRUE);
        }
        break;

    case WAN_SLOW_MONSTER:
    case SPE_SLOW_MONSTER:
        learn_it = TRUE;
        u_slow_down();
        break;

    case WAN_TELEPORTATION:
    case SPE_TELEPORT_AWAY:
        tele();
        /* same criteria as when mounted (zap_steed) */
        if ((Teleport_control && !Stunned) || !couldsee(u.ux0, u.uy0)
            || distu(u.ux0, u.uy0) >= 16)
            learn_it = TRUE;
        break;

    case WAN_DEATH:
    case SPE_FINGER_OF_DEATH:
        if (is_undead(youmonst.data)
            || Race_if(PM_DRAUGR) || Race_if(PM_VAMPIRE)) {
            /* wand/spell of death acts like a potion of full healing
               for the undead, especially whilst in the VotD */
            shieldeff(u.ux, u.uy);
            monstseesu(M_SEEN_DEATH);
            if (Is_valley(&u.uz)) {
                if (Upolyd) {
                    u.mhmax += u.mhmax / 6;
                    if (u.mhmax > 200)
                        u.mhmax = 200;
                } else {
                    u.uhpmax += u.uhpmax / 6;
                    if (u.uhpmax > 200)
                        u.uhpmax = 200;
                }
            }
            if (Upolyd)
                u.mh = u.mhmax;
            else
                u.uhp = u.uhpmax;
            You_feel("restored.");
            break;
        } else if (Death_resistance || immune_death_magic(youmonst.data)) {
            shieldeff(u.ux, u.uy);
            monstseesu(M_SEEN_DEATH);
            pline((obj->otyp == WAN_DEATH)
                      ? "The wand shoots an apparently harmless beam at you."
                      : nonliving(youmonst.data)
                          ? "You seem no more dead than before."
                          : "You are unaffected.");
            break;
        }
        learn_it = TRUE;
        Sprintf(killer.name, "shot %sself with a death ray", uhim());
        killer.format = NO_KILLER_PREFIX;
        You("irradiate yourself with pure energy!");
        You("die.");
        /* They might survive with an amulet of life saving */
        done(DIED);
        break;
    case WAN_UNDEAD_TURNING:
    case SPE_TURN_UNDEAD:
        learn_it = TRUE;
        unturn_you();
        break;
    case SPE_DISPEL_EVIL:
        learn_it = TRUE;
        if (is_evil(&youmonst)) {
            You("shudder in agony!");
            damage = d(8, clerical_skill);
            exercise(A_CON, FALSE);
        }
        break;
    case SPE_HEALING:
    case SPE_EXTRA_HEALING:
    case SPE_CRITICAL_HEALING:
        learn_it = TRUE; /* (no effect for spells...) */
        if (!rn2(4) && obj->otyp == SPE_CRITICAL_HEALING) {
            if (Sick) {
                You("are no longer ill.");
                healup(0, 0, TRUE, FALSE);
            }
            if (Slimed)
                make_slimed(0L, "The slime disappears!");
            if (Withering) {
                set_itimeout(&HWithering, (long) 0);
                if (!Withering)
                    You("are no longer withering away.");
            }
        }
        healup(d(healing_skill, obj->otyp == SPE_CRITICAL_HEALING
                 ? 12 : obj->otyp == SPE_EXTRA_HEALING ? 8 : 4), 0, FALSE,
               (obj->blessed
                || obj->otyp == SPE_EXTRA_HEALING
                || obj->otyp == SPE_CRITICAL_HEALING));
        You_feel("%sbetter.", obj->otyp == SPE_CRITICAL_HEALING
                              ? "considerably " : obj->otyp == SPE_EXTRA_HEALING
                                ? "much " : "");
        break;
    case SPE_CURE_SICKNESS:
        if (maybe_polyd(is_zombie(youmonst.data), Race_if(PM_DRAUGR))) {
            You("shudder in agony!");
            damage = d(sickness_skill, 8);
            exercise(A_CON, FALSE);
        } else {
            if (Sick)
                You("are no longer ill.");
            if (Slimed)
                make_slimed(0L, "The slime disappears!");
            if (Withering) {
                set_itimeout(&HWithering, (long) 0);
                if (!Withering)
                    You("are no longer withering away.");
            }
        }
        healup(0, 0, TRUE, FALSE);
        break;
    case SPE_RESTORE_ABILITY: {
        int i, ii, lim;

        pline("Wow!  This makes you feel %s!",
              (obj->blessed)
                  ? (unfixable_trouble_count(FALSE) ? "better" : "great")
                  : "good");
        i = rn2(A_MAX); /* start at a random point */
        for (ii = 0; ii < A_MAX; ii++) {
            lim = AMAX(i);
            /* this used to adjust 'lim' for A_STR when u.uhs was
               WEAK or worse, but that's handled via ATEMP(A_STR) now */
            if (ABASE(i) < lim) {
                ABASE(i) = lim;
                context.botl = 1;
                /* only first found if not blessed */
                if (!obj->blessed)
                    break;
            }
            if (++i >= A_MAX)
                i = 0;
        }
        break;
    }
    case WAN_LIGHT: /* (broken wand) */
        /* assert( !ordinary ); */
        damage = d(obj->spe, 25);
        /*FALLTHRU*/
    case EXPENSIVE_CAMERA:
        if (!damage)
            damage = 5;
        damage = lightdamage(obj, ordinary, damage);
        damage += rnd(25);
        if (flashburn((long) damage))
            learn_it = TRUE;
        if (obj->otyp == WAN_LIGHT && !cursed(obj, TRUE))
            blindingflash();
        damage = 0; /* reset */
        break;
    case WAN_OPENING:
    case SPE_KNOCK:
        if (Punished) {
            learn_it = TRUE;
            unpunish();
        }
        /* invent is hit iff hero doesn't escape from a trap */
        if (!u.utrap || !openholdingtrap(&youmonst, &learn_it)) {
            struct obj *otmp, *nextobj;
            boolean boxing = FALSE;

            /* unlock carried boxes */
            for (otmp = invent; otmp; otmp = nextobj) {
                nextobj = otmp->nobj;
                if (Is_box(otmp)) {
                    (void) boxlock(otmp, obj);
                    boxing = TRUE;
                }
            }
            if (boxing)
                update_inventory(); /* in case any box->lknown has changed */

            /* trigger previously escaped trapdoor */
            (void) openfallingtrap(&youmonst, TRUE, &learn_it);
        }
        break;
    case WAN_LOCKING:
    case SPE_WIZARD_LOCK:
        /* similar logic to opening; invent is hit iff no trap triggered */
        if (u.utrap || !closeholdingtrap(&youmonst, &learn_it)) {
            struct obj *otmp;
            boolean boxing = FALSE;

            /* lock carried boxes */
            for (otmp = invent; otmp; otmp = otmp->nobj)
                if (Is_box(otmp)) {
                    (void) boxlock(otmp, obj);
                    boxing = TRUE;
                }
            if (boxing)
                update_inventory(); /* in case any box->lknown has changed */
        }
        break;
    case WAN_DIGGING:
    case SPE_DIG:
    case SPE_DETECT_UNSEEN:
    case WAN_NOTHING:
        break;
    case WAN_PROBING: {
        struct obj *otmp;

        for (otmp = invent; otmp; otmp = otmp->nobj) {
            otmp->dknown = 1;
            if (Is_container(otmp) || otmp->otyp == STATUE) {
                otmp->lknown = 1;
                if (!SchroedingersBox(otmp))
                    otmp->cknown = 1;
            }
        }
        update_inventory();
        learn_it = TRUE;
        ustatusline();
        break;
    }
    case SPE_STONE_TO_FLESH: {
        struct obj *otmp, *onxt;
        boolean didmerge;

        if (u.umonnum == PM_STONE_GOLEM) {
            learn_it = TRUE;
            (void) polymon(PM_FLESH_GOLEM);
        }
        if (u.umonnum == PM_PETRIFIED_ENT) {
            learn_it = TRUE;
            (void) polymon(PM_ENT);
        }
        if (Stoned) {
            learn_it = TRUE;
            fix_petrification(); /* saved! */
        }
        /* but at a cost.. */
        for (otmp = invent; otmp; otmp = onxt) {
            onxt = otmp->nobj;
            if (bhito(otmp, obj))
                learn_it = TRUE;
        }
        /*
         * It is possible that we can now merge some inventory.
         * Do a highly paranoid merge.  Restart from the beginning
         * until no merges.
         */
        do {
            didmerge = FALSE;
            for (otmp = invent; !didmerge && otmp; otmp = otmp->nobj)
                for (onxt = otmp->nobj; onxt; onxt = onxt->nobj)
                    if (merged(&otmp, &onxt)) {
                        didmerge = TRUE;
                        break;
                    }
        } while (didmerge);
        break;
    }
    case SPE_ENTANGLE:
        cast_entangle(&youmonst);
        break;
    case SPE_CHANGE_METAL_TO_WOOD: {
        struct obj *otmp;

        /* only change what caster is wielding */
        if (uwep)
            otmp = uwep;
        else
            otmp = (struct obj *) 0;

        (int) cast_metal_to_wood(otmp, TRUE);
        break;
    }
    default:
        impossible("zapyourself: object %d used?", obj->otyp);
        break;
    }
    /* if effect was observable then discover the wand type provided
       that the wand itself has been seen */
    if (learn_it)
        learnwand(obj);
    return damage;
}

/* called when poly'd hero uses breath attack against self */
void
ubreatheu(mattk)
struct attack *mattk;
{
    int dtyp = 22 + mattk->adtyp - 1;      /* breath by hero */
    const char *fltxt = flash_types[dtyp]; /* blast of <something> */

    zhitu(dtyp, mattk->damn, fltxt, u.ux, u.uy);
}

/* light damages hero in light-hating form */
int
lightdamage(obj, ordinary, amt)
struct obj *obj;  /* item making light (fake book if spell) */
boolean ordinary; /* wand/camera zap vs wand destruction */
int amt;          /* pseudo-damage used to determine blindness duration */
{
    char buf[BUFSZ];
    const char *how;
    int dmg = amt;

    if (dmg && (hates_light(youmonst.data)
                || maybe_polyd(is_drow(youmonst.data),
                                       Race_if(PM_DROW)))) {
        /* reduce high values (from destruction of wand with many charges) */
        dmg = rnd(dmg);
        if (dmg > 10)
            dmg = 10 + rnd(dmg - 10);
        if (dmg > 20)
            dmg = 20;
        pline("Ow, that light hurts%c", (dmg > 2 || u.mh <= 5) ? '!' : '.');
        /* [composing killer/reason is superfluous here; if fatal, cause
           of death will always be "killed while stuck in creature form"] */
        if (obj->oclass == SCROLL_CLASS || obj->oclass == SPBOOK_CLASS)
            ordinary = FALSE; /* say blasted rather than zapped */
        how = (obj->oclass != SPBOOK_CLASS)
                  ? (const char *) ansimpleoname(obj)
                  : "spell of light";
        Sprintf(buf, "%s %sself with %s", ordinary ? "zapped" : "blasted",
                uhim(), how);
        /* might rehumanize(); could be fatal, but only for Unchanging */
        losehp(Maybe_Half_Phys(dmg), buf, NO_KILLER_PREFIX);
    }
    return dmg;
}

/* light[ning] causes blindness */
boolean
flashburn(duration)
long duration;
{
    if (!resists_blnd(&youmonst)) {
        You(are_blinded_by_the_flash);
        make_blinded(duration, FALSE);
        if (!Blind)
            Your1(vision_clears);
        return TRUE;
    }
    return FALSE;
}

/* you've zapped a wand downwards while riding
 * Return TRUE if the steed was hit by the wand.
 * Return FALSE if the steed was not hit by the wand.
 */
STATIC_OVL boolean
zap_steed(obj)
struct obj *obj; /* wand or spell */
{
    int steedhit = FALSE;

    bhitpos.x = u.usteed->mx, bhitpos.y = u.usteed->my;
    notonhead = FALSE;
    switch (obj->otyp) {
    /*
     * Wands that are allowed to hit the steed
     * Carefully test the results of any that are
     * moved here from the bottom section.
     */
    case WAN_PROBING:
        probe_monster(u.usteed);
        learnwand(obj);
        steedhit = TRUE;
        break;
    case WAN_TELEPORTATION:
    case SPE_TELEPORT_AWAY:
        /* you go together */
        tele();
        /* same criteria as when unmounted (zapyourself) */
        if ((Teleport_control && !Stunned) || !couldsee(u.ux0, u.uy0)
            || distu(u.ux0, u.uy0) >= 16)
            learnwand(obj);
        steedhit = TRUE;
        break;

    /* Default processing via bhitm() for these */
    case SPE_CURE_SICKNESS:
    case SPE_RESTORE_ABILITY:
    case WAN_MAKE_INVISIBLE:
    case WAN_CANCELLATION:
    case SPE_CANCELLATION:
    case WAN_POLYMORPH:
    case SPE_POLYMORPH:
    case WAN_STRIKING:
    case SPE_FORCE_BOLT:
    case SPE_PSIONIC_WAVE:
    case WAN_SLOW_MONSTER:
    case SPE_SLOW_MONSTER:
    case WAN_SPEED_MONSTER:
    case SPE_HEALING:
    case SPE_EXTRA_HEALING:
    case SPE_CRITICAL_HEALING:
    case SPE_DRAIN_LIFE:
    case WAN_OPENING:
    case SPE_KNOCK:
    case SPE_ENTANGLE:
    case SPE_CHANGE_METAL_TO_WOOD:
        (void) bhitm(u.usteed, obj);
        steedhit = TRUE;
        break;

    default:
        steedhit = FALSE;
        break;
    }
    return steedhit;
}

/*
 * cancel a monster (possibly the hero).  inventory is cancelled only
 * if the monster is zapping itself directly, since otherwise the
 * effect is too strong.  currently non-hero monsters do not zap
 * themselves with cancellation.
 */
boolean
cancel_monst(mdef, obj, youattack, allow_cancel_kill, self_cancel)
register struct monst *mdef;
register struct obj *obj;
boolean youattack, allow_cancel_kill, self_cancel;
{
    boolean youdefend = (mdef == &youmonst);
    static const char writing_vanishes[] =
        "Some writing vanishes from %s head!";
    static const char your[] = "your"; /* should be extern */

    int onum = 0, oindex = 0, cnt = 0;
    struct obj *otmp;

    boolean resisted = (youdefend && Antimagic)
                        || (!youdefend
                            && resist(mdef, obj ? obj->oclass : 0, 0, NOTELL));

    if (obj && obj->otyp == WAN_CANCELLATION)
        makeknown(obj->otyp);

    if (self_cancel) { /* 1st cancel inventory */
        for (otmp = (youdefend ? invent : mdef->minvent); otmp;
             otmp = otmp->nobj)
            cancel_item(otmp);
        if (youdefend) {
            You_feel("magical energies being absorbed from your exact location.");
            context.botl = 1; /* potential AC change */
            find_ac();
        }
    } else {
        /* shield effect if target has MR - does not
           protect against cancellation */
        if (youdefend && Antimagic)
            shieldeff(u.ux, u.uy);
        if (!youdefend && resisted)
            shieldeff(mdef->mx, mdef->my);

        /* player attacking monster */
        if (youattack) {
            if (rn2(10)
                && (otmp = which_armor(mdef, W_ARM))
                && Is_dragon_scaled_armor(otmp)
                && (Dragon_armor_to_scales(otmp) == GRAY_DRAGON_SCALES
                    || Dragon_armor_to_scales(otmp) == CHROMATIC_DRAGON_SCALES)) {
                shieldeff(mdef->mx, mdef->my);
                if (canseemon(mdef))
                    You("sense a wave of energy dissipate around %s.",
                        mon_nam(mdef));
                return FALSE;
            } else if (rn2(3)
                       && (otmp = which_armor(mdef, W_ARM))
                       && otmp->oartifact == ART_ARMOR_OF_RETRIBUTION) {
                shieldeff(mdef->mx, mdef->my);
                if (canseemon(mdef))
                    You("sense a wave of energy dissipate around %s.",
                        mon_nam(mdef));
                return FALSE;
            } else if (mdef->data == &mons[PM_TIAMAT]
                       || mdef->data == &mons[PM_GRAY_DRAGON]) {
                shieldeff(mdef->mx, mdef->my);
                if (canseemon(mdef))
                    You("sense a wave of energy dissipate around %s.",
                        mon_nam(mdef));
                return FALSE;
            } else {
                if (!mdef->mcan && canseemon(mdef))
                    pline("Magical energies are absorbed from %s.", mon_nam(mdef));
                if (mdef->mprotection) {
                    if (canseemon(mdef))
                        pline_The("%s haze around %s %s.",
                                  hcolor(NH_GOLDEN), mon_nam(mdef), "disappears");
                    mdef->mprotection = mdef->mprottime = 0;
                }
                if (has_reflection(mdef)) {
                    if (canseemon(mdef))
                        pline("%s shimmering globe disappears.",
                              s_suffix(Monnam(mdef)));
                    mdef->mextrinsics &= ~(MR2_REFLECTION);
                    mdef->mreflecttime = 0;
                }
                if (has_barkskin(mdef)) {
                    if (canseemon(mdef))
                        pline("%s %s returns to normal.",
                              s_suffix(Monnam(mdef)), mbodypart(mdef, SKIN));
                    mdef->mextrinsics &= ~(MR2_BARKSKIN);
                    mdef->mbarkskintime = 0;
                }
                if (has_stoneskin(mdef)) {
                    if (canseemon(mdef))
                        pline("%s %s returns to normal.",
                              s_suffix(Monnam(mdef)), mbodypart(mdef, SKIN));
                    mdef->mextrinsics &= ~(MR2_STONESKIN);
                    mdef->mstoneskintime = 0;
                }
            }
        }

        /* monster attacking player */
        if (youdefend) {
            if (rn2(10) && uarm
                && Is_dragon_scaled_armor(uarm)
                && (Dragon_armor_to_scales(uarm) == GRAY_DRAGON_SCALES
                    || Dragon_armor_to_scales(uarm) == CHROMATIC_DRAGON_SCALES)) {
                shieldeff(u.ux, u.uy);
                You_feel("a wave of energy dissipate around you.");
                return FALSE;
            } else if (rn2(3)
                       && ((Role_if(PM_MONK)
                            && ublindf && ublindf->oartifact == ART_EYES_OF_THE_OVERWORLD)
                           || (uarm && uarm->oartifact == ART_ARMOR_OF_RETRIBUTION))) {
                shieldeff(u.ux, u.uy);
                You_feel("a wave of energy dissipate around you.");
                return FALSE;
            } else if (Upolyd && youmonst.data == &mons[PM_GRAY_DRAGON]) {
                shieldeff(u.ux, u.uy);
                You_feel("a wave of energy dissipate around you.");
                return FALSE;
            } else {
                You_feel("magical energies being absorbed from your vicinity.");
                if (u.uspellprot) {
                    pline_The("%s haze around you disappears.",
                              hcolor(NH_GOLDEN));
                    u.usptime = u.uspmtime = u.uspellprot = 0;
                    context.botl = 1; /* potential AC change */
                    find_ac();
                }
                if (HReflecting > 0) {
                    pline("The shimmering globe around you disappears.");
                    HReflecting = 0;
                }
            }
        }

        /* monster attacking another monster */
        if (!youdefend && !youattack) {
            if (rn2(10)
                && (otmp = which_armor(mdef, W_ARM))
                && Is_dragon_scaled_armor(otmp)
                && (Dragon_armor_to_scales(otmp) == GRAY_DRAGON_SCALES
                    || Dragon_armor_to_scales(otmp) == CHROMATIC_DRAGON_SCALES)) {
                shieldeff(mdef->mx, mdef->my);
                if (canseemon(mdef))
                    You("sense a wave of energy dissipate around %s.",
                        mon_nam(mdef));
                return FALSE;
            } else if (rn2(3)
                       && (otmp = which_armor(mdef, W_ARM))
                       && otmp->oartifact == ART_ARMOR_OF_RETRIBUTION) {
                shieldeff(mdef->mx, mdef->my);
                if (canseemon(mdef))
                    You("sense a wave of energy dissipate around %s.",
                        mon_nam(mdef));
                return FALSE;
            } else if (mdef->data == &mons[PM_TIAMAT]
                       || mdef->data == &mons[PM_GRAY_DRAGON]) {
                shieldeff(mdef->mx, mdef->my);
                if (canseemon(mdef))
                    You("sense a wave of energy dissipate around %s.",
                        mon_nam(mdef));
                return FALSE;
            } else {
                if (!mdef->mcan && canseemon(mdef))
                    pline("Magical energies are absorbed from %s.", mon_nam(mdef));
                if (mdef->mprotection) {
                    if (canseemon(mdef))
                        pline_The("%s haze around %s %s.",
                                  hcolor(NH_GOLDEN), mon_nam(mdef), "disappears");
                    mdef->mprotection = mdef->mprottime = 0;
                }
                if (has_reflection(mdef)) {
                    if (canseemon(mdef))
                        pline("%s shimmering globe disappears.",
                              s_suffix(Monnam(mdef)));
                    mdef->mextrinsics &= ~(MR2_REFLECTION);
                    mdef->mreflecttime = 0;
                }
                if (has_barkskin(mdef)) {
                    if (canseemon(mdef))
                        pline("%s %s returns to normal.",
                              s_suffix(Monnam(mdef)), mbodypart(mdef, SKIN));
                    mdef->mextrinsics &= ~(MR2_BARKSKIN);
                    mdef->mbarkskintime = 0;
                }
                if (has_stoneskin(mdef)) {
                    if (canseemon(mdef))
                        pline("%s %s returns to normal.",
                              s_suffix(Monnam(mdef)), mbodypart(mdef, SKIN));
                    mdef->mextrinsics &= ~(MR2_STONESKIN);
                    mdef->mstoneskintime = 0;
                }
            }
        }

        for (otmp = (youdefend ? invent : mdef->minvent);
             otmp; otmp = otmp->nobj) {
            /* gold isn't subject to being cursed or blessed */
            if (otmp->oclass == COIN_CLASS)
                continue;
            onum++;
        }

        if (onum) {
            int hero_count = ((!!Antimagic) + (!!Half_spell_damage) + 1);
            int mon_count = rnd(3);

            for (cnt = rnd(6 / (youdefend ? hero_count : mon_count));
                 cnt > 0; cnt--) {
                oindex = rnd(onum);
                /* if random count is higher than number of objects
                   in inventory, clamp count to be no greater than
                   number of objects in inventory */
                if (cnt > onum)
                    cnt = onum;
                for (otmp = (youdefend ? invent : mdef->minvent);
                     otmp; otmp = otmp->nobj) {
                    /* as above */
                    if (otmp->oclass == COIN_CLASS)
                        continue;
                    if (--oindex == 0)
                        break; /* found the target */
                }
                if (!otmp)
                    continue; /* next target */

                if (otmp->blessed && !otmp->oartifact
                    && !obj_resists(otmp, 0, 0) && !rn2(5)) {
                    pline("%s!", Yobjnam2(otmp, "resist"));
                    continue;
                }
                if (((otmp->oartifact && spec_ability(otmp, SPFX_INTEL))
                     || obj_resists(otmp, 0, 0))
                    && rn2(10) < 8) {
                    pline("%s!", Tobjnam(otmp, "resist"));
                    continue;
                }
                cancel_item(otmp);
            }
            if (youdefend) {
                context.botl = 1; /* potential AC change */
                find_ac();
            }
            update_inventory();
        }
    }

    if (resisted)
        return FALSE;

    /* now handle special cases */
    if (youdefend) {
        if (Upolyd) { /* includes lycanthrope in creature form */
            /*
             * Return to normal form unless Unchanging.
             * Hero in clay golem form dies if Unchanging.
             * Does not cure lycanthropy or stop timed random polymorph.
             */
            if (u.umonnum == PM_CLAY_GOLEM) {
                if (!Blind)
                    pline(writing_vanishes, your);
                else /* note: "dark" rather than "heavy" is intentional... */
                    You_feel("%s headed.", Hallucination ? "dark" : "light");
                u.mh = 0; /* fatal; death handled by rehumanize() */
            }
            if (Unchanging && u.mh > 0)
                Your("amulet grows hot for a moment, then cools.");
            else
                rehumanize();
        }
    } else {
        mdef->mcan = 1;
        /* force shapeshifter into its base form */
        if (M_AP_TYPE(mdef) != M_AP_NOTHING)
            seemimic(mdef);
        /* [not 'else if'; chameleon might have been hiding as a mimic] */
        if (mdef->cham >= LOW_PM) {
            /* note: newcham() uncancels shapechangers (resets m->mcan
               to 0), but only for shapechangers whose m->cham is already
               NON_PM and we just verified that it's LOW_PM or higher */
            newcham(mdef, &mons[mdef->cham], FALSE, FALSE);
            mdef->cham = NON_PM; /* cancelled shapeshifter can't shift */
        }
        if (is_were(mdef->data) && !is_human(mdef->data))
            were_change(mdef);

        if (mdef->data == &mons[PM_CLAY_GOLEM]) {
            if (canseemon(mdef))
                pline(writing_vanishes, s_suffix(mon_nam(mdef)));
            /* !allow_cancel_kill is for Magicbane, where clay golem
               will be killed somewhere back up the call/return chain... */
            if (allow_cancel_kill) {
                if (youattack)
                    killed(mdef);
                else
                    monkilled(mdef, "", AD_CNCL);
            }
        }
    }
    return TRUE;
}

/* you've zapped an immediate type wand up or down */
STATIC_OVL boolean
zap_updown(obj)
struct obj *obj; /* wand or spell */
{
    boolean striking = FALSE, disclose = FALSE;
    int x, y, xx, yy, ptmp;
    struct obj *otmp;
    struct engr *e;
    struct trap *ttmp;
    char buf[BUFSZ];

    /* some wands have special effects other than normal bhitpile */
    /* drawbridge might change <u.ux,u.uy> */
    x = xx = u.ux;     /* <x,y> is zap location */
    y = yy = u.uy;     /* <xx,yy> is drawbridge (portcullis) position */
    ttmp = t_at(x, y); /* trap if there is one */

    switch (obj->otyp) {
    case WAN_PROBING:
        ptmp = 0;
        if (u.dz < 0) {
            You("probe towards the %s.", ceiling(x, y));
        } else {
            ptmp += bhitpile(obj, bhito, x, y, u.dz);
            You("probe beneath the %s.", surface(x, y));
            ptmp += display_binventory(x, y, TRUE);
        }
        if (!ptmp)
            Your("probe reveals nothing.");
        return TRUE; /* we've done our own bhitpile */
    case WAN_OPENING:
    case SPE_KNOCK:
        /* up or down, but at closed portcullis only */
        if (is_db_wall(x, y) && find_drawbridge(&xx, &yy)) {
            open_drawbridge(xx, yy);
            disclose = TRUE;
        } else if (u.dz > 0 && (x == xdnstair && y == ydnstair)
                   /* can't use the stairs down to quest level 2 until
                      leader "unlocks" them; give feedback if you try */
                   && on_level(&u.uz, &qstart_level) && !ok_to_quest()) {
            pline_The("stairs seem to ripple momentarily.");
            disclose = TRUE;
        } else if (IS_MAGIC_CHEST(levl[x][y].typ) && u.dz > 0) {
            disclose = boxlock(mchest, obj);
        }
        /* down will release you from bear trap or web */
        if (u.dz > 0 && u.utrap) {
            (void) openholdingtrap(&youmonst, &disclose);
            /* down will trigger trapdoor, hole, or [spiked-] pit */
        } else if (u.dz > 0 && !u.utrap) {
            (void) openfallingtrap(&youmonst, FALSE, &disclose);
        }
        break;
    case WAN_STRIKING:
    case SPE_FORCE_BOLT:
        striking = TRUE;
        /*FALLTHRU*/
    case WAN_LOCKING:
    case SPE_WIZARD_LOCK:
        /* down at open bridge or up or down at open portcullis */
        if (((levl[x][y].typ == DRAWBRIDGE_DOWN)
                 ? (u.dz > 0)
                 : (is_drawbridge_wall(x, y) >= 0 && !is_db_wall(x, y)))
            && find_drawbridge(&xx, &yy)) {
            if (!striking)
                close_drawbridge(xx, yy);
            else
                destroy_drawbridge(xx, yy);
            disclose = TRUE;
        } else if (striking && u.dz < 0 && rn2(3) && !Is_airlevel(&u.uz)
                   && !Is_waterlevel(&u.uz) && !Underwater
                   && !Is_qstart(&u.uz)) {
            int dmg;
            /* similar to zap_dig() */
            pline("A rock is dislodged from the %s and falls on your %s.",
                  ceiling(x, y), body_part(HEAD));
            dmg = rnd((uarmh && is_hard(uarmh)) ? 2 : 6);
            losehp(Maybe_Half_Phys(dmg), "falling rock", KILLED_BY_AN);
            if ((otmp = mksobj_at(ROCK, x, y, FALSE, FALSE)) != 0) {
                (void) xname(otmp); /* set dknown, maybe bknown */
                stackobj(otmp);
            }
            newsym(x, y);
        } else if (!striking && IS_MAGIC_CHEST(levl[x][y].typ) && u.dz > 0) {
            disclose = boxlock(mchest, obj);
        } else if (u.dz > 0 && ttmp) {
            if (!striking && closeholdingtrap(&youmonst, &disclose)) {
                ; /* now stuck in web or bear trap */
            } else if (striking && ttmp->ttyp == TRAPDOOR) {
                /* striking transforms trapdoor into hole */
                if (Blind && !ttmp->tseen) {
                    pline("%s beneath you shatters.", Something);
                } else if (!ttmp->tseen) { /* => !Blind */
                    pline("There's a trapdoor beneath you; it shatters.");
                } else {
                    pline("The trapdoor beneath you shatters.");
                    disclose = TRUE;
                }
                ttmp->ttyp = HOLE;
                ttmp->tseen = 1;
                newsym(x, y);
                /* might fall down hole */
                dotrap(ttmp, 0);
            } else if (!striking && ttmp->ttyp == HOLE) {
                /* locking transforms hole into trapdoor */
                ttmp->ttyp = TRAPDOOR;
                if (Blind || !ttmp->tseen) {
                    pline("Some %s swirls beneath you.",
                          is_ice(x, y) ? "frost" : "dust");
                } else {
                    ttmp->tseen = 1;
                    newsym(x, y);
                    pline("A trapdoor appears beneath you.");
                    disclose = TRUE;
                }
                /* hadn't fallen down hole; won't fall now */
            }
        }
        break;
    case SPE_STONE_TO_FLESH:
        if (Is_airlevel(&u.uz) || Is_waterlevel(&u.uz) || Underwater
            || (Is_qstart(&u.uz) && u.dz < 0)) {
            pline1(nothing_happens);
        } else if (u.dz < 0) { /* we should do more... */
            pline("Blood drips on your %s.", body_part(FACE));
        } else if (u.dz > 0 && !OBJ_AT(u.ux, u.uy)) {
            /*
            Print this message only if there wasn't an engraving
            affected here.  If water or ice, act like waterlevel case.
            */
            e = engr_at(u.ux, u.uy);
            if (!(e && e->engr_type == ENGRAVE)) {
                if (is_pool(u.ux, u.uy) || is_ice(u.ux, u.uy))
                    pline1(nothing_happens);
                else if (is_puddle(u.ux, u.uy) || is_sewage(u.ux, u.uy))
                    pline("The water at your %s turns slightly %s.",
                          makeplural(body_part(FOOT)), hcolor("red"));
                else
                    pline("Blood %ss %s your %s.",
                          is_lava(u.ux, u.uy) ? "boil" : "pool",
                          Levitation ? "beneath" : "at",
                          makeplural(body_part(FOOT)));
            }
        }
        break;
    default:
        break;
    }

    if (u.dz > 0) {
        /* zapping downward */
        (void) bhitpile(obj, bhito, x, y, u.dz);

        /* subset of engraving effects; none sets `disclose' */
        if ((e = engr_at(x, y)) != 0 && e->engr_type != HEADSTONE) {
            switch (obj->otyp) {
            case WAN_POLYMORPH:
            case SPE_POLYMORPH:
                del_engr(e);
                make_engr_at(x, y, random_engraving(buf), moves, (xchar) 0);
                break;
            case WAN_CANCELLATION:
            case SPE_CANCELLATION:
            case WAN_MAKE_INVISIBLE:
                del_engr(e);
                break;
            case WAN_TELEPORTATION:
            case SPE_TELEPORT_AWAY:
                rloc_engr(e);
                break;
            case SPE_STONE_TO_FLESH:
                if (e->engr_type == ENGRAVE) {
                    /* only affects things in stone */
                    pline_The(Hallucination
                                  ? "floor runs like butter!"
                                  : "edges on the floor get smoother.");
                    wipe_engr_at(x, y, d(2, 4), TRUE);
                }
                break;
            case WAN_STRIKING:
            case SPE_FORCE_BOLT:
                wipe_engr_at(x, y, d(2, 4), TRUE);
                break;
            default:
                break;
            }
        }

        maybe_explode_trap(ttmp, obj);
        /* note: ttmp might be now gone */

    } else if (u.dz < 0) {
        /* zapping upward */

        /* game flavor: if you're hiding under "something"
         * a zap upward should hit that "something".
         */
        if (u.uundetected && hides_under(youmonst.data)) {
            int hitit = 0;
            otmp = level.objects[u.ux][u.uy];

            if (otmp)
                hitit = bhito(otmp, obj);
            if (hitit) {
                (void) hideunder(&youmonst);
                disclose = TRUE;
            }
        }
    }

    return disclose;
}

/* used by do_break_wand() was well as by weffects() */
void
zapsetup()
{
    obj_zapped = FALSE;
}

void
zapwrapup()
{
    /* if do_osshock() set obj_zapped while polying, give a message now */
    if (obj_zapped)
        You_feel("shuddering vibrations.");
    obj_zapped = FALSE;
}

/* called for various wand and spell effects - M. Stephenson */
void
weffects(obj)
struct obj *obj;
{
    int otyp = obj->otyp;
    boolean disclose = FALSE, was_unkn = !objects[otyp].oc_name_known;

    exercise(A_WIS, TRUE);
    if (u.usteed && (objects[otyp].oc_dir != NODIR) && !u.dx && !u.dy
        && (u.dz > 0) && zap_steed(obj)) {
        disclose = TRUE;
    } else if (objects[otyp].oc_dir == IMMEDIATE) {
        zapsetup(); /* reset obj_zapped */
        if (u.uswallow) {
            (void) bhitm(u.ustuck, obj);
            /* [how about `bhitpile(u.ustuck->minvent)' effect?] */
        } else if (u.dz) {
            disclose = zap_updown(obj);
        } else {
            (void) bhit(u.dx, u.dy, rn1(8, 6), ZAPPED_WAND, bhitm, bhito,
                        &obj);
        }
        zapwrapup(); /* give feedback for obj_zapped */

    } else if (objects[otyp].oc_dir == NODIR) {
        zapnodir(obj);

    } else {
        /* neither immediate nor directionless */

        if (otyp == WAN_DIGGING || otyp == SPE_DIG)
            zap_dig();
        else if (otyp >= SPE_MAGIC_MISSILE && otyp <= SPE_ACID_BLAST)
            buzz(ZT_SPELL(otyp - SPE_MAGIC_MISSILE), u.ulevel / 2 + 1,
                 u.ux, u.uy, u.dx, u.dy);
        else if (otyp >= WAN_MAGIC_MISSILE && otyp <= WAN_LIGHTNING)
            buzz(otyp - WAN_MAGIC_MISSILE,
                 (otyp == WAN_MAGIC_MISSILE) ? 2 : 6, u.ux, u.uy, u.dx, u.dy);
        else
            impossible("weffects: unexpected spell or wand");
        disclose = TRUE;
    }
    if (disclose) {
        learnwand(obj);
        if (was_unkn)
            more_experienced(0, 10);
    }
    return;
}

/* augment damage for a spell dased on the hero's intelligence (and level) */
int
spell_damage_bonus(dmg)
int dmg; /* base amount to be adjusted by bonus or penalty */
{
    int intell = ACURR(A_INT);

    /* Punish low intelligence before low level else low intelligence
       gets punished only when high level */
    if (intell <= 9) {
        /* -3 penalty, but never reduce combined amount below 1
           (if dmg is 0 for some reason, we're careful to leave it there) */
        if (dmg > 1)
            dmg = (dmg <= 3) ? 1 : dmg - 3;
    } else if (intell <= 13 || u.ulevel < 5)
        ; /* no bonus or penalty; dmg remains same */
    else if (intell <= 18)
        dmg += 1;
    else if (intell <= 24 || u.ulevel < 14)
        dmg += 2;
    else
        dmg += 3; /* Int 25 */

    return dmg;
}

/*
 * Generate the to hit bonus for a spell.  Based on the hero's skill in
 * spell class and dexterity.
 */
STATIC_OVL int
spell_hit_bonus(skill, is_wand)
int skill;
boolean is_wand;
{
    int hit_bon = 0;
    int dex = ACURR(A_DEX);

    if (is_wand) {
        if (!uwep) /* easier to aim with hands free */
            hit_bon = 3;
    } else {
        switch (P_SKILL(spell_skilltype(skill))) {
        case P_ISRESTRICTED:
        case P_UNSKILLED:
            hit_bon = -4;
            break;
        case P_BASIC:
            hit_bon = 0;
            break;
        case P_SKILLED:
            hit_bon = 4;
            break;
        case P_EXPERT:
            hit_bon = 6;
            break;
        }
    }

    if (dex < 4)
        hit_bon -= 3;
    else if (dex < 6)
        hit_bon -= 2;
    else if (dex < 8)
        hit_bon -= 1;
    else if (dex < 14)
        /* Will change when print stuff below removed */
        hit_bon -= 0;
    else
        /* Even increment for dextrous heroes (see weapon.c abon) */
        hit_bon += dex - 14;

    /* experience matters... */
    hit_bon += (u.ulevel < 15) ? u.ulevel : 15;

    return hit_bon;
}

const char *
exclam(force)
int force;
{
    /* force == 0 occurs e.g. with sleep ray */
    /* note that large force is usual with wands so that !! would
            require information about hand/weapon/wand */
    return (const char *) ((force < 0) ? "?" : (force <= 4) ? "." : "!");
}

void
hit(str, mtmp, force)
const char *str;
struct monst *mtmp;
const char *force; /* usually either "." or "!" */
{
    if ((!cansee(bhitpos.x, bhitpos.y) && !canspotmon(mtmp)
         && !(u.uswallow && mtmp == u.ustuck)) || !flags.verbose)
        pline("%s %s it.", The(str), vtense(str, "hit"));
    else
        pline("%s %s %s%s", The(str), vtense(str, "hit"),
              mon_nam(mtmp), force);
}

void
miss(str, mtmp)
register const char *str;
register struct monst *mtmp;
{
    pline(
        "%s %s %s.", The(str), vtense(str, "miss"),
        ((cansee(bhitpos.x, bhitpos.y) || canspotmon(mtmp)) && flags.verbose)
            ? mon_nam(mtmp)
            : "it");
}

STATIC_OVL void
skiprange(range, skipstart, skipend)
int range, *skipstart, *skipend;
{
    int tr = (range / 4);
    int tmp = range - ((tr > 0) ? rnd(tr) : 0);

    *skipstart = tmp;
    *skipend = tmp - ((tmp / 4) * rnd(3));
    if (*skipend >= tmp)
        *skipend = tmp - 1;
}

/* maybe explode a trap hit by object otmp's effect;
   cancellation beam hitting a magical trap causes an explosion.
   might delete the trap.  */
void
maybe_explode_trap(ttmp, otmp)
struct trap *ttmp;
struct obj *otmp;
{
    if (!ttmp || !otmp)
        return;
    if (otmp->otyp == WAN_CANCELLATION || otmp->otyp == SPE_CANCELLATION) {
        xchar x = ttmp->tx, y = ttmp->ty;

        if (undestroyable_trap(ttmp->ttyp)) {
            shieldeff(x, y);
            if (cansee(x, y)) {
                ttmp->tseen = 1;
                newsym(x, y);
            }
        } else if (is_magical_trap(ttmp->ttyp)) {
            if (!Deaf)
                pline("Kaboom!");
            explode(x, y, AD_MAGM - 1,
                    20 + d(3 ,6), TRAP_EXPLODE, EXPL_MAGICAL);
            deltrap(ttmp);
            newsym(x, y);
        }
    }
}

/*
 *  Called for the following distance effects:
 *      when a weapon is thrown (weapon == THROWN_WEAPON)
 *      when an object is kicked (KICKED_WEAPON)
 *      when an IMMEDIATE wand is zapped (ZAPPED_WAND)
 *      when a light beam is flashed (FLASHED_LIGHT)
 *      when a mirror is applied (INVIS_BEAM)
 *  A thrown/kicked object falls down at end of its range or when a monster
 *  is hit.  The variable 'bhitpos' is set to the final position of the weapon
 *  thrown/zapped.  The ray of a wand may affect (by calling a provided
 *  function) several objects and monsters on its path.  The return value
 *  is the monster hit (weapon != ZAPPED_WAND), or a null monster pointer.
 *
 * Thrown and kicked objects (THROWN_WEAPON or KICKED_WEAPON) may be
 * destroyed and *pobj set to NULL to indicate this.
 *
 *  Check !u.uswallow before calling bhit().
 *  This function reveals the absence of a remembered invisible monster in
 *  necessary cases (throwing or kicking weapons).  The presence of a real
 *  one is revealed for a weapon, but if not a weapon is left up to fhitm().
 */
struct monst *
bhit(ddx, ddy, range, weapon, fhitm, fhito, pobj)
register int ddx, ddy, range;          /* direction and range */
enum bhit_call_types weapon;           /* defined in hack.h */
int FDECL((*fhitm), (MONST_P, OBJ_P)), /* fns called when mon/obj hit */
    FDECL((*fhito), (OBJ_P, OBJ_P));
struct obj **pobj; /* object tossed/used, set to NULL
                    * if object is destroyed */
{
    struct monst *mtmp, *result = (struct monst *) 0;
    struct obj *obj = *pobj;
    uchar typ;
    boolean shopdoor = FALSE, point_blank = TRUE;
    boolean in_skip = FALSE, allow_skip = FALSE;
    boolean tethered_weapon = FALSE;
    int skiprange_start = 0, skiprange_end = 0, skipcount = 0;

    if (weapon == KICKED_WEAPON) {
        /* object starts one square in front of player */
        bhitpos.x = u.ux + ddx;
        bhitpos.y = u.uy + ddy;
        range--;
    } else {
        bhitpos.x = u.ux;
        bhitpos.y = u.uy;
    }

    if (weapon == THROWN_WEAPON && obj && obj->otyp == ROCK) {
        skiprange(range, &skiprange_start, &skiprange_end);
        allow_skip = !rn2(3);
    }

    if (weapon == FLASHED_LIGHT) {
        tmp_at(DISP_BEAM, cmap_to_glyph(S_flashbeam));
    } else if (weapon == THROWN_TETHERED_WEAPON && obj) {
        tethered_weapon = TRUE;
        weapon = THROWN_WEAPON; /* simplify 'if's that follow below */
        tmp_at(DISP_TETHER, obj_to_glyph(obj, rn2_on_display_rng));
    } else if (weapon != ZAPPED_WAND && weapon != INVIS_BEAM)
        tmp_at(DISP_FLASH, obj_to_glyph(obj, rn2_on_display_rng));

    while (range-- > 0) {
        int x, y;

        bhitpos.x += ddx;
        bhitpos.y += ddy;
        x = bhitpos.x;
        y = bhitpos.y;

        if (!isok(x, y)) {
            bhitpos.x -= ddx;
            bhitpos.y -= ddy;
            break;
        }

        if (is_pick(obj) && inside_shop(x, y)
            && (mtmp = shkcatch(obj, x, y)) != 0) {
            tmp_at(DISP_END, 0);
            result = mtmp;
            goto bhit_done;
        }

        typ = levl[bhitpos.x][bhitpos.y].typ;

        if (typ == IRONBARS
            && ((levl[bhitpos.x][bhitpos.y].wall_info & W_NONDIGGABLE) == 0)
            && ((!(weapon == KICKED_WEAPON || weapon == THROWN_WEAPON)
                 && obj->otyp == SPE_FORCE_BOLT)
                || (weapon == ZAPPED_WAND && obj->otyp == WAN_STRIKING))) {
            levl[bhitpos.x][bhitpos.y].typ = ROOM;
            if (cansee(bhitpos.x, bhitpos.y))
                pline_The("iron bars are blown apart!");
            else if (!Deaf)
                You_hear("a lot of loud clanging sounds!");
            wake_nearto(bhitpos.x, bhitpos.y, 20 * 20);
            newsym(bhitpos.x, bhitpos.y);
            /* stop the bolt here; it takes a lot of energy to destroy bars */
            range = 0;
            break;
        } else if (typ == IRONBARS
                   && ((levl[bhitpos.x][bhitpos.y].wall_info & W_NONDIGGABLE) != 0)
                   && ((!(weapon == KICKED_WEAPON || weapon == THROWN_WEAPON)
                        && obj->otyp == SPE_FORCE_BOLT)
                       || (weapon == ZAPPED_WAND && obj->otyp == WAN_STRIKING))) {
            if (cansee(bhitpos.x, bhitpos.y))
                pline_The("iron bars vibrate, but are otherwise intact.");
            else if (!Deaf)
                You_hear("an intense sustained hum.");
            wake_nearto(bhitpos.x, bhitpos.y, 8 * 8);
            range = 0;
            break;
        }

        /* iron bars will block anything big enough and break some things */
        if (weapon == THROWN_WEAPON || weapon == KICKED_WEAPON) {
            if (typ == IRONBARS
                && hits_bars(pobj, x - ddx, y - ddy, bhitpos.x, bhitpos.y,
                             point_blank ? 0 : !rn2(5), 1)) {
                /* caveat: obj might now be null... */
                obj = *pobj;
                bhitpos.x -= ddx;
                bhitpos.y -= ddy;
                break;
            } else if (obj->lamplit && !Blind) {
                show_transient_light(obj, bhitpos.x, bhitpos.y);
            }
        }

        if (weapon == ZAPPED_WAND && find_drawbridge(&x, &y)) {
            boolean learn_it = FALSE;

            switch (obj->otyp) {
            case WAN_OPENING:
            case SPE_KNOCK:
                if (is_db_wall(bhitpos.x, bhitpos.y)) {
                    if (cansee(x, y) || cansee(bhitpos.x, bhitpos.y))
                        learn_it = TRUE;
                    open_drawbridge(x, y);
                }
                break;
            case WAN_LOCKING:
            case SPE_WIZARD_LOCK:
                if ((cansee(x, y) || cansee(bhitpos.x, bhitpos.y))
                    && levl[x][y].typ == DRAWBRIDGE_DOWN)
                    learn_it = TRUE;
                close_drawbridge(x, y);
                break;
            case WAN_STRIKING:
            case SPE_FORCE_BOLT:
                if (typ != DRAWBRIDGE_UP)
                    destroy_drawbridge(x, y);
                learn_it = TRUE;
                break;
            }
            if (learn_it)
                learnwand(obj);
        }

        if (weapon == ZAPPED_WAND && typ == MAGIC_CHEST) {
            boolean learn_it = FALSE;

            switch (obj->otyp) {
            case WAN_OPENING:
            case SPE_KNOCK:
            case WAN_LOCKING:
            case SPE_WIZARD_LOCK:
                if (boxlock(mchest, obj))
                    learn_it = TRUE;
                break;
            }
            if (learn_it)
                learnwand(obj);
        }

        if (weapon == ZAPPED_WAND)
            maybe_explode_trap(t_at(bhitpos.x, bhitpos.y), obj);

        mtmp = m_at(bhitpos.x, bhitpos.y);

        /*
         * skipping rocks
         *
         * skiprange_start is only set if this is a thrown rock
         */
        if (skiprange_start && (range == skiprange_start) && allow_skip) {
            if (is_pool(bhitpos.x, bhitpos.y) && !mtmp) {
                in_skip = TRUE;
                if (!Blind)
                    pline("%s %s%s.", Yname2(obj), otense(obj, "skip"),
                          skipcount ? " again" : "");
                else
                    You_hear("%s skip.", yname(obj));
                skipcount++;
            } else if (skiprange_start > skiprange_end + 1) {
                --skiprange_start;
            }
        }
        if (in_skip) {
            if (range <= skiprange_end) {
                in_skip = FALSE;
                if (range > 3) /* another bounce? */
                    skiprange(range, &skiprange_start, &skiprange_end);
            } else if (mtmp && M_IN_WATER(mtmp->data)) {
                if (!Blind && canspotmon(mtmp))
                    pline("%s %s over %s.", Yname2(obj), otense(obj, "pass"),
                          mon_nam(mtmp));
                mtmp = (struct monst *) 0;
            }
        }

        /* if mtmp is a shade and missile passes harmlessly through it,
           give message and skip it in order to keep going */
        if (mtmp && (weapon == THROWN_WEAPON || weapon == KICKED_WEAPON)
            && shade_miss(&youmonst, mtmp, obj, TRUE, TRUE))
            mtmp = (struct monst *) 0;

        if (mtmp) {
            notonhead = (bhitpos.x != mtmp->mx || bhitpos.y != mtmp->my);
            if (weapon == FLASHED_LIGHT) {
                /* FLASHED_LIGHT hitting invisible monster should
                   pass through instead of stop so we call
                   flash_hits_mon() directly rather than returning
                   mtmp back to caller.  That allows the flash to
                   keep on going.  Note that we use mtmp->minvis
                   not canspotmon() because it makes no difference
                   whether the hero can see the monster or not. */
                if (mtmp->minvis) {
                    obj->ox = u.ux, obj->oy = u.uy;
                    (void) flash_hits_mon(mtmp, obj);
                } else {
                    tmp_at(DISP_END, 0);
                    result = mtmp; /* caller will call flash_hits_mon */
                    goto bhit_done;
                }
            } else if (weapon == INVIS_BEAM) {
                /* Like FLASHED_LIGHT, INVIS_BEAM should continue
                   through invisible targets; unlike it, we aren't
                   prepared for multiple hits so just get first one
                   that's either visible or could see its invisible
                   self.  [No tmp_at() cleanup is needed here.] */
                if (!mtmp->minvis || racial_perceives(mtmp)) {
                    result = mtmp;
                    goto bhit_done;
                }
            } else if (weapon != ZAPPED_WAND) {

                /* THROWN_WEAPON, KICKED_WEAPON */
                if (!tethered_weapon)
                    tmp_at(DISP_END, 0);

                if (cansee(bhitpos.x, bhitpos.y) && !canspotmon(mtmp))
                    map_invisible(bhitpos.x, bhitpos.y);
                result = mtmp;
                goto bhit_done;
            } else {
                /* ZAPPED_WAND */
                (*fhitm)(mtmp, obj);
                range -= 3;
                /* While this code works for all wands,
                   it causes massive problems if the
                   mount dies before the rider... */

                if (obj->otyp == WAN_PROBING && has_erid(mtmp)) {
                    (*fhitm)(ERID(mtmp)->mon_steed, obj);
                    range -= 1;
                }
            }
        } else {
            if (weapon == ZAPPED_WAND && obj->otyp == WAN_PROBING
                && glyph_is_invisible(levl[bhitpos.x][bhitpos.y].glyph)) {
                unmap_object(bhitpos.x, bhitpos.y);
                newsym(x, y);
            }
        }
        if (fhito) {
            if (bhitpile(obj, fhito, bhitpos.x, bhitpos.y, 0))
                range--;
        } else {
            if (weapon == KICKED_WEAPON
                && ((obj->oclass == COIN_CLASS
                     && OBJ_AT(bhitpos.x, bhitpos.y))
                    || ship_object(obj, bhitpos.x, bhitpos.y,
                                   costly_spot(bhitpos.x, bhitpos.y)))) {
                tmp_at(DISP_END, 0);
                goto bhit_done; /* result == (struct monst *) 0 */
            }
        }
        if (weapon == ZAPPED_WAND && (IS_DOOR(typ) || typ == SDOOR)) {
            switch (obj->otyp) {
            case WAN_OPENING:
            case WAN_LOCKING:
            case WAN_STRIKING:
            case SPE_KNOCK:
            case SPE_WIZARD_LOCK:
            case SPE_FORCE_BOLT:
                if (doorlock(obj, bhitpos.x, bhitpos.y)) {
                    if (cansee(bhitpos.x, bhitpos.y)
                        || (obj->otyp == WAN_STRIKING && !Deaf))
                        learnwand(obj);
                    if (levl[bhitpos.x][bhitpos.y].doormask == D_BROKEN
                        && *in_rooms(bhitpos.x, bhitpos.y, SHOPBASE)) {
                        shopdoor = TRUE;
                        add_damage(bhitpos.x, bhitpos.y, SHOP_DOOR_COST);
                    }
                }
                break;
            }
        }
        if (!ZAP_POS(typ) || closed_door(bhitpos.x, bhitpos.y)) {
            bhitpos.x -= ddx;
            bhitpos.y -= ddy;
            break;
        }
        if (weapon != ZAPPED_WAND && weapon != INVIS_BEAM) {
            /* 'I' present but no monster: erase */
            /* do this before the tmp_at() */
            if (glyph_is_invisible(levl[bhitpos.x][bhitpos.y].glyph)
                && cansee(x, y)) {
                unmap_object(bhitpos.x, bhitpos.y);
                newsym(x, y);
            }
            tmp_at(bhitpos.x, bhitpos.y);
            delay_output();
            /* kicked objects fall in pools */
            if ((weapon == KICKED_WEAPON)
                && (is_pool(bhitpos.x, bhitpos.y)
                    || is_lava(bhitpos.x, bhitpos.y)))
                break;
            if ((IS_SINK(typ) || IS_FORGE(typ)) && weapon != FLASHED_LIGHT)
                break; /* physical objects fall onto sink */
        }
        /* limit range of ball so hero won't make an invalid move */
        if (weapon == THROWN_WEAPON && range > 0
            && obj->otyp == HEAVY_IRON_BALL) {
            struct obj *bobj;
            struct trap *t;

            if ((bobj = sobj_at(BOULDER, x, y)) != 0) {
                if (cansee(x, y))
                    pline("%s hits %s.", The(distant_name(obj, xname)),
                          an(xname(bobj)));
                range = 0;
            } else if (obj == uball) {
                if (!test_move(x - ddx, y - ddy, ddx, ddy, TEST_MOVE)) {
                    /* nb: it didn't hit anything directly */
                    if (cansee(x, y))
                        pline("%s jerks to an abrupt halt.",
                              The(distant_name(obj, xname))); /* lame */
                    range = 0;
                } else if (Sokoban && (t = t_at(x, y)) != 0
                           && (is_pit(t->ttyp) || is_hole(t->ttyp))) {
                    /* hero falls into the trap, so ball stops */
                    range = 0;
                }
            }
        }

        /* thrown/kicked missile has moved away from its starting spot */
        point_blank = FALSE; /* affects passing through iron bars */
    }

    if (weapon != ZAPPED_WAND && weapon != INVIS_BEAM && !tethered_weapon)
        tmp_at(DISP_END, 0);

    if (shopdoor)
        pay_for_damage("destroy", FALSE);

 bhit_done:
    if (weapon == THROWN_WEAPON || weapon == KICKED_WEAPON)
        transient_light_cleanup();

    last_hurtled = (struct monst *) 0;
    return result;
}

/* process thrown boomerang, which travels a curving path...
 * A multi-shot volley ought to have all missiles in flight at once,
 * but we're called separately for each one.  We terminate the volley
 * early on a failed catch since continuing to throw after being hit
 * is too obviously silly.
 */
struct monst *
boomhit(obj, dx, dy)
struct obj *obj;
int dx, dy;
{
    register int i, ct;
    int boom; /* showsym[] index  */
    struct monst *mtmp;
    boolean counterclockwise = TRUE; /* right-handed throw */

    /* counterclockwise traversal patterns:
     *  ..........................54.................................
     *  ..................43.....6..3....765.........................
     *  ..........32.....5..2...7...2...8...4....87..................
     *  .........4..1....6..1...8..1....9...3...9..6.....98..........
     *  ..21@....5...@...7..@....9@......@12....@...5...@..7.....@9..
     *  .3...9....6..9....89.....................1..4...1..6....1..8.
     *  .4...8.....78.............................23....2..5...2...7.
     *  ..567............................................34....3..6..
     *  ........................................................45...
     * (invert rows for corresponding clockwise patterns)
     */

    bhitpos.x = u.ux;
    bhitpos.y = u.uy;
    boom = counterclockwise ? S_boomleft : S_boomright;
    for (i = 0; i < 8; i++)
        if (xdir[i] == dx && ydir[i] == dy)
            break;
    tmp_at(DISP_FLASH, cmap_to_glyph(boom));
    for (ct = 0; ct < 10; ct++) {
        i = (i + 8) % 8;                          /* 0..7 (8 -> 0, -1 -> 7) */
        boom = (S_boomleft + S_boomright - boom); /* toggle */
        tmp_at(DISP_CHANGE, cmap_to_glyph(boom)); /* change glyph */
        dx = xdir[i];
        dy = ydir[i];
        bhitpos.x += dx;
        bhitpos.y += dy;
        if (!isok(bhitpos.x, bhitpos.y)) {
            bhitpos.x -= dx;
            bhitpos.y -= dy;
            break;
        }
        if ((mtmp = m_at(bhitpos.x, bhitpos.y)) != 0) {
            m_respond(mtmp);
            tmp_at(DISP_END, 0);
            return mtmp;
        }
        if (!ZAP_POS(levl[bhitpos.x][bhitpos.y].typ)
            || closed_door(bhitpos.x, bhitpos.y)) {
            bhitpos.x -= dx;
            bhitpos.y -= dy;
            break;
        }
        if (bhitpos.x == u.ux && bhitpos.y == u.uy) { /* ct == 9 */
            if (Fumbling || rn2(20) >= ACURR(A_DEX)) {
                /* we hit ourselves */
                (void) thitu(10 + obj->spe, dmgval(obj, &youmonst), &obj,
                             "boomerang");
                endmultishot(TRUE);
                break;
            } else { /* we catch it */
                tmp_at(DISP_END, 0);
                You("skillfully catch the boomerang.");
                retouch_object(&obj, !uarmg, FALSE);
                return &youmonst;
            }
        }
        tmp_at(bhitpos.x, bhitpos.y);
        delay_output();
        if (IS_SINK(levl[bhitpos.x][bhitpos.y].typ)) {
            if (!Deaf)
                pline("Klonk!");
            break; /* boomerang falls on sink */
        }
        /* ct==0, initial position, we want next delta to be same;
           ct==5, opposite position, repeat delta undoes first one */
        if (ct % 5 != 0)
            i += (counterclockwise ? -1 : 1);
    }
    tmp_at(DISP_END, 0); /* do not leave last symbol */
    return (struct monst *) 0;
}

/* used by buzz(); also used by munslime(muse.c); returns damage applied
   to mon; note: caller is responsible for killing mon if damage is fatal */
int
zhitm(mon, type, nd, ootmp)
register struct monst *mon;
register int type, nd;
struct obj **ootmp; /* to return worn armor for caller to disintegrate */
{
    register int tmp = 0;
    register int abstype = BASE_ZT(abs(type));
    boolean sho_shieldeff = FALSE;
    boolean spellcaster = is_hero_spell(type); /* maybe get a bonus! */
    boolean mon_tempest_wield = (MON_WEP(mon)
                                 && MON_WEP(mon)->oartifact == ART_TEMPEST);

    *ootmp = (struct obj *) 0;
    switch (abstype) {
    case ZT_MAGIC_MISSILE:
        tmp = d(nd, 6);
        if (spellcaster)
            tmp = spell_damage_bonus(tmp);
        if (resists_magm(mon) || defended(mon, AD_MAGM)) {
            if (resists_mgc(mon->data))
                tmp = 0;
            else
                tmp = (tmp + 1) / 2;
            sho_shieldeff = TRUE;
        }
        break;
    case ZT_FIRE:
        if (resists_fire(mon) || defended(mon, AD_FIRE)
            || mon_underwater(mon)) {
            sho_shieldeff = TRUE;
            break;
        }
        tmp = d(nd, 6);
        if (spellcaster)
            tmp = spell_damage_bonus(tmp);
        if (burnarmor(mon)) {
            if (!rn2(3))
                (void) destroy_mitem(mon, POTION_CLASS, AD_FIRE);
            if (!rn2(3))
                (void) destroy_mitem(mon, SCROLL_CLASS, AD_FIRE);
            if (!rn2(5))
                (void) destroy_mitem(mon, SPBOOK_CLASS, AD_FIRE);
            destroy_mitem(mon, FOOD_CLASS, AD_FIRE); /* carried slime */
        }
        break;
    case ZT_COLD:
        if (resists_cold(mon) || defended(mon, AD_COLD)) {
            sho_shieldeff = TRUE;
            break;
        }
        tmp = d(nd, 6);
        if (spellcaster)
            tmp = spell_damage_bonus(tmp);
        if (!rn2(3))
            (void) destroy_mitem(mon, POTION_CLASS, AD_COLD);
        break;
    case ZT_SLEEP:
        tmp = 0;
        (void) sleep_monst(mon, d(nd, 25),
                           type == ZT_WAND(ZT_SLEEP) ? WAND_CLASS : '\0');
        break;
    case ZT_DEATH:                              /* death/disintegration */
        if (abs(type) != ZT_BREATH(ZT_DEATH)) { /* death */
            tmp = d(4, 6);
            if (mon->data == &mons[PM_DEATH]) {
                sho_shieldeff = TRUE;
                mon->mhpmax += mon->mhpmax / 2;
                if (mon->mhpmax > DEATH_PROOF)
                    mon->mhpmax = DEATH_PROOF;
                mon->mhp = mon->mhpmax;
                tmp = 0;
                break;
            }
            if (is_undead(mon->data) || racial_zombie(mon)
                || racial_vampire(mon)) {
                /* wand of death acts like a potion of full healing
                   for the undead, especially whilst in the VotD */
                sho_shieldeff = TRUE;
                if (Is_valley(&u.uz)) {
                    mon->mhpmax += mon->mhpmax / 6;
                    if (mon->mhpmax > 200)
                        mon->mhpmax = 200;
                }
                mon->mhp = mon->mhpmax;
                tmp = 0;
                break;
            }
            if (immune_death_magic(mon->data)
                || defended(mon, AD_DETH)
                || is_vampshifter(mon)) {
                sho_shieldeff = TRUE;
                tmp = 0;
                break;
            }
            if (resists_magm(mon) || defended(mon, AD_MAGM)) {
                /* similar to player */
                sho_shieldeff = TRUE;
                tmp = (tmp + 1) / 2;
                if (spellcaster)
                    tmp = spell_damage_bonus(tmp);
                break;
            }
            type = -1; /* so they don't get saving throws */
        } else {
            struct obj *otmp2;

            if (resists_disint(mon) || defended(mon, AD_DISN)) {
                sho_shieldeff = TRUE;
            } else if (mon->misc_worn_check & W_ARMS) {
                /* destroy shield; victim survives */
                *ootmp = which_armor(mon, W_ARMS);
            } else if (mon->misc_worn_check & W_ARM) {
                /* destroy suit, also cloak if present */
                *ootmp = which_armor(mon, W_ARM);
                if ((otmp2 = which_armor(mon, W_ARMC)) != 0)
                    m_useup(mon, otmp2);
            } else {
                /* no suit, victim dies; destroy cloak
                   and shirt now in case target gets life-saved */
                tmp = MAGIC_COOKIE;
                if ((otmp2 = which_armor(mon, W_ARMC)) != 0)
                    m_useup(mon, otmp2);
                if ((otmp2 = which_armor(mon, W_ARMU)) != 0)
                    m_useup(mon, otmp2);
            }
            type = -1; /* no saving throw wanted */
            break;     /* not ordinary damage */
        }
        tmp = mon->mhp + 1;
        break;
    case ZT_LIGHTNING:
        if (resists_elec(mon) || defended(mon, AD_ELEC)) {
            sho_shieldeff = TRUE;
            tmp = 0;
            /* can still blind the monster */
        } else
            tmp = d(nd, 6);
        if (spellcaster)
            tmp = spell_damage_bonus(tmp);
        if (!resists_blnd(mon)
            && !(type > 0 && u.uswallow && mon == u.ustuck)) {
            register unsigned rnd_tmp = rnd(50);
            mon->mcansee = 0;
            if ((mon->mblinded + rnd_tmp) > 127)
                mon->mblinded = 127;
            else
                mon->mblinded += rnd_tmp;
        }
        if (!rn2(3))
            (void) destroy_mitem(mon, WAND_CLASS, AD_ELEC);
        /* not actually possible yet */
        if (!rn2(3))
            (void) destroy_mitem(mon, RING_CLASS, AD_ELEC);
        break;
    case ZT_POISON_GAS:
        if (resists_poison(mon) || defended(mon, AD_DRST)) {
            sho_shieldeff = TRUE;
            break;
        }
        tmp = d(nd, 6);
        break;
    case ZT_ACID:
        if (resists_acid(mon) || defended(mon, AD_ACID)
            || mon_underwater(mon)) {
            sho_shieldeff = TRUE;
            break;
        }
        tmp = d(nd, 6);
        if (!rn2(6))
            acid_damage(MON_WEP(mon));
        if (!rn2(6))
            erode_armor(mon, ERODE_CORRODE);
        break;
    case ZT_WATER:
        tmp = d(nd, 8);
        if (mon->data == &mons[PM_WATER_ELEMENTAL]
            || mon->data == &mons[PM_BABY_SEA_DRAGON]
            || mon->data == &mons[PM_SEA_DRAGON]) {
            tmp = 0;
            break;
        }
        if (mon->data == &mons[PM_IRON_GOLEM]) {
            if (canseemon(mon))
                pline("%s falls to pieces!", Monnam(mon));
            if (mon->mtame)
                pline("May %s rust in peace.", mon_nam(mon));
            tmp = mon->mhp + 1;
            break;
        }
        if (!rn2(6))
            water_damage(MON_WEP(mon), 0, TRUE, mon->mx, mon->my);
        if (!rn2(6))
            erode_armor(mon, ERODE_RUST);
        break;
    case ZT_DRAIN: {
        int drain = monhp_per_lvl(mon);

        tmp = d(nd, 6);
        if (resists_drli(mon) || defended(mon, AD_DRLI)) {
            sho_shieldeff = TRUE;
            /* physical damage still occurs */
            break;
        }

        if (mon->m_lev == 0) {
            tmp = mon->mhp + 1;
            break;
        } else {
            tmp += drain;
            mon->mhpmax -= drain;
            mon->m_lev--;
        }
        break;
        }
    case ZT_STUN:
        if (resists_stun(mon->data) || defended(mon, AD_STUN)
            || mon_tempest_wield) {
            sho_shieldeff = TRUE;
            break;
        }
        tmp = d(nd, 6);
        if (!mon->mstun)
            mon->mstun = 1;
        break;
    }
    if (sho_shieldeff)
        shieldeff(mon->mx, mon->my);
    if (is_hero_spell(type) && (Role_if(PM_KNIGHT) && u.uhave.questart))
        tmp *= 2;
    if (tmp > 0 && type >= 0
        && resist(mon, type < ZT_SPELL(0) ? WAND_CLASS : '\0', 0, NOTELL))
        tmp /= 2;
    if (tmp < 0)
        tmp = 0; /* don't allow negative damage */
    /* have to do vulnerability here because we need to return tmp.
       damage_mon() wouldn't tell us if tmp is increased. */
    if (vulnerable_to(mon, abstype + 1))
        tmp = ((3 * tmp) + 1) / 2;
    debugpline3("zapped monster hp = %d (= %d - %d)", mon->mhp - tmp,
                mon->mhp, tmp);
    mon->mhp -= tmp;
    return tmp;
}

STATIC_OVL void
zhitu(type, nd, fltxt, sx, sy)
int type, nd;
const char *fltxt;
xchar sx, sy;
{
    int dam = 0, abstyp = abs(type);

    switch (BASE_ZT(abstyp)) {
    case ZT_MAGIC_MISSILE:
        dam = d(nd, 6);
        exercise(A_STR, FALSE);
        if (Antimagic) {
            shieldeff(sx, sy);
            pline("Some missiles bounce off!");
            monstseesu(M_SEEN_MAGR);
            dam = (dam + 1) / 2;
        }
        /* Half spell damage stacks here, but is factored
           in after this switch statement */
        break;
    case ZT_FIRE:
        if (how_resistant(FIRE_RES) == 100
            || Underwater) {
            shieldeff(sx, sy);
            You("don't feel hot!");
            monstseesu(M_SEEN_FIRE);
            ugolemeffects(AD_FIRE, d(nd, 6));
        } else {
            dam = resist_reduce(d(nd, 6), FIRE_RES);
        }
        if (!Underwater) {
            burn_away_slime();
            if (!Reflecting) {
                if (burnarmor(&youmonst)) { /* "body hit" */
                    if (!rn2(3))
                        destroy_item(POTION_CLASS, AD_FIRE);
                    if (!rn2(3))
                        destroy_item(SCROLL_CLASS, AD_FIRE);
                    if (!rn2(5))
                        destroy_item(SPBOOK_CLASS, AD_FIRE);
                    destroy_item(FOOD_CLASS, AD_FIRE);
                }
            }
        }
        break;
    case ZT_COLD:
        if (how_resistant(COLD_RES) == 100) {
            shieldeff(sx, sy);
            You("don't feel cold.");
            monstseesu(M_SEEN_COLD);
            ugolemeffects(AD_COLD, d(nd, 6));
        } else {
            dam = resist_reduce(d(nd, 6), COLD_RES);
        }
        if (!Reflecting) {
            if (!rn2(3))
                destroy_item(POTION_CLASS, AD_COLD);
        }
        break;
    case ZT_SLEEP:
        if (how_resistant(SLEEP_RES) == 100) {
            shieldeff(u.ux, u.uy);
            You("don't feel sleepy.");
            monstseesu(M_SEEN_SLEEP);
        } else if (Reflecting) {
            fall_asleep(-resist_reduce(d(1, 6), SLEEP_RES), TRUE);
            monstseesu(M_SEEN_REFL);
        } else {
            fall_asleep(-resist_reduce(d(nd, 25), SLEEP_RES), TRUE); /* sleep ray */
        }
        break;
    case ZT_DEATH:
        if (abstyp == ZT_BREATH(ZT_DEATH)) {
            if (how_resistant(DISINT_RES) == 100) {
                You("are not disintegrated.");
                monstseesu(M_SEEN_DISINT);
                break;
            } else if (Reflecting) {
                You("aren't disintegrated, but that hurts!");
                dam = resist_reduce(d(6, 6), DISINT_RES);
                break;
            } else if (!Reflecting && (how_resistant(DISINT_RES) >= 50)) {
                You("aren't disintegrated, but that really hurts!");
                dam = resist_reduce(d(12, 6), DISINT_RES);
                break;
            } else if (!Reflecting && (how_resistant(DISINT_RES) < 50)) {
                dam = resist_reduce(d(12, 6), DISINT_RES);
                if (uarms) {
                    /* destroy shield; other possessions are safe */
                    (void) destroy_arm(uarms);
                    break;
                } else if (uarm) {
                    /* destroy suit; if present, cloak goes too */
                    if (uarmc)
                        (void) destroy_arm(uarmc);
                    (void) destroy_arm(uarm);
                    break;
                }
                /* fall through. not having enough disintegration
                   resistance can still get you disintegrated */
            }
            /* no shield or suit, you're dead; wipe out cloak
               and/or shirt in case of life-saving or bones */
            if (!Reflecting) {
                if (uarmc)
                    (void) destroy_arm(uarmc);
                if (uarmu)
                    (void) destroy_arm(uarmu);
            }
        } else if (is_undead(youmonst.data)
                   || Race_if(PM_DRAUGR) || Race_if(PM_VAMPIRE)) {
            /* wand of death acts like a potion of full healing
               for the undead, especially whilst in the VotD */
            shieldeff(sx, sy);
            monstseesu(M_SEEN_DEATH);
            if (Is_valley(&u.uz)) {
                u.uhpmax += u.uhpmax / 6;
                if (u.uhpmax > 200)
                    u.uhpmax = 200;
            }
            u.uhp = u.uhpmax;
            You_feel("restored.");
            break;
        } else if (Death_resistance
                   || immune_death_magic(youmonst.data)) {
            shieldeff(sx, sy);
            monstseesu(M_SEEN_DEATH);
            You("seem unaffected.");
            break;
        } else if (Antimagic) {
            /* not as much damage as 'touch of death'
             * but this will still leave a mark */
            dam = d(4, 6);
            if (Antimagic && Half_spell_damage) {
                shieldeff(sx, sy);
                monstseesu(M_SEEN_MAGR);
                dam /= 2;
            }
            if (Reflecting) {
                You("feel a little drained...");
                u.uhpmax -= (dam / 3 + rn2(5)) / 2;
            } else {
                You("feel drained...");
                u.uhpmax -= dam / 3 + rn2(5);
            }
            break;
        } else if (Reflecting && !Antimagic) {
            dam = d(4, 6);
            if (Reflecting && Half_spell_damage) {
                shieldeff(sx, sy);
                monstseesu(M_SEEN_MAGR);
                dam /= 2;
            }
            You("feel drained...");
            u.uhpmax -= dam / 3 + rn2(5);
            break;
        }
        killer.format = KILLED_BY_AN;
        Strcpy(killer.name, fltxt ? fltxt : "");
        /* when killed by disintegration breath, don't leave corpse */
        u.ugrave_arise = (type == -ZT_BREATH(ZT_DEATH)) ? -3 : NON_PM;
        done((type == -ZT_BREATH(ZT_DEATH)) ? DISINTEGRATED : DIED);
        return; /* lifesaved */
    case ZT_LIGHTNING:
        if (how_resistant(SHOCK_RES) == 100) {
            shieldeff(sx, sy);
            You("aren't affected.");
            monstseesu(M_SEEN_ELEC);
            ugolemeffects(AD_ELEC, d(nd, 6));
        } else {
            dam = resist_reduce(d(nd, 6), SHOCK_RES);
            exercise(A_CON, FALSE);
        }
        if (!Reflecting) {
            if (!rn2(3))
                destroy_item(WAND_CLASS, AD_ELEC);
            if (!rn2(3))
                destroy_item(RING_CLASS, AD_ELEC);
        }
        break;
    case ZT_POISON_GAS:
        if (!Reflecting) {
            poisoned("blast", A_DEX, "poisoned blast", 15, FALSE);
        }
        break;
    case ZT_ACID:
        if (Acid_resistance || Underwater) {
            pline_The("%s doesn't hurt.", hliquid("acid"));
            monstseesu(M_SEEN_ACID);
            dam = 0;
        } else {
            pline_The("%s burns!", hliquid("acid"));
            dam = d(nd, 6);
            exercise(A_STR, FALSE);
        }
        if (!(Reflecting || Underwater)) {
            /* using two weapons at once makes both of them more vulnerable */
            if (!rn2(u.twoweap ? 3 : 6))
                acid_damage(uwep);
            if (u.twoweap && !rn2(3))
                acid_damage(uswapwep);
            if (!rn2(6))
                erode_armor(&youmonst, ERODE_CORRODE);
        }
        break;
    case ZT_WATER:
        dam = d(nd, 8);
        if (Half_physical_damage)
            dam = (dam + 1) / 2;
        if (u.umonnum == PM_WATER_ELEMENTAL
            || u.umonnum == PM_BABY_SEA_DRAGON
            || u.umonnum == PM_SEA_DRAGON) {
            You("%sabsorb the blast of water into your body.",
                Reflecting ? "partially " : "");
            dam = 0;
            break;
        }
        if (u.umonnum == PM_IRON_GOLEM) {
            You("rust!");
            rehumanize();
            break;
        }
        if (!Reflecting) {
            /* using two weapons at once makes both of them more vulnerable */
            if (!rn2(u.twoweap ? 3 : 6))
                water_damage(uwep, 0, TRUE, u.ux, u.uy);
            if (u.twoweap && !rn2(3))
                water_damage(uswapwep, 0, TRUE, u.ux, u.uy);
            if (!rn2(6))
                erode_armor(&youmonst, ERODE_RUST);
        }
        break;
    case ZT_DRAIN: {
        const char *life = nonliving(youmonst.data) ? "animating force"
                                                    : "life";

        /* will still take physical damage from the force of
           the breath attack, even if drain resistant */
        dam = d(nd, 6);
        if (Half_physical_damage)
            dam = (dam + 1) / 2;
        if (Drain_resistance) {
            ugolemeffects(AD_DRLI, d(nd, 6));
            break;
        }

        if (Reflecting) {
            You("feel drained...");
            u.uhpmax -= dam / 2 + rn2(5);
        } else {
            if (Blind)
                You_feel("a dark energy blast draining your %s!",
                         life);
            else
                pline_The("dark energy blast drains your %s!",
                          life);
            losexp("life drainage");
        }
        break;
        }
    case ZT_STUN:
        /* will still take physical damage from the force of
           the breath attack, even if stun resistant */
        dam = d(nd, 6);
        if (Half_physical_damage)
            dam = (dam + 1) / 2;
        if (Stun_resistance
            || wielding_artifact(ART_TEMPEST))
            shieldeff(sx, sy); /* resistance handled in make_stunned() */
        make_stunned((HStun & TIMEOUT) + (long) dam / (Reflecting ? 4 : 2), TRUE);
        break;
    }

    /* Half_spell_damage protection yields half-damage for wands & spells,
       including hero's own ricochets; breath attacks do full damage */
    if (dam && Half_spell_damage && !(abstyp >= 20 && abstyp <= 29))
        dam = (dam + 1) / 2;
    losehp(dam, fltxt, KILLED_BY_AN);
    return;
}

/*
 * burn objects (such as scrolls and spellbooks) on floor
 * at position x,y; return the number of objects burned
 */
int
burn_floor_objects(x, y, give_feedback, u_caused)
int x, y;
boolean give_feedback; /* caller needs to decide about visibility checks */
boolean u_caused;
{
    struct obj *obj, *obj2;
    long i, scrquan, delquan;
    char buf1[BUFSZ], buf2[BUFSZ];
    int cnt = 0;

    for (obj = level.objects[x][y]; obj; obj = obj2) {
        obj2 = obj->nexthere;
        if (obj->oclass == SCROLL_CLASS || obj->oclass == SPBOOK_CLASS
            || (obj->oclass == FOOD_CLASS
                && obj->otyp == GLOB_OF_GREEN_SLIME)) {
            if (obj->otyp == SCR_FIRE || obj->otyp == SPE_FIREBALL
                || obj_resists(obj, 2, 100))
                continue;
            scrquan = obj->quan; /* number present */
            delquan = 0L;        /* number to destroy */
            for (i = scrquan; i > 0L; i--)
                if (!rn2(3))
                    delquan++;
            if (delquan) {
                /* save name before potential delobj() */
                if (give_feedback) {
                    obj->quan = 1L;
                    Strcpy(buf1, (x == u.ux && y == u.uy)
                                     ? xname(obj)
                                     : distant_name(obj, xname));
                    obj->quan = 2L;
                    Strcpy(buf2, (x == u.ux && y == u.uy)
                                     ? xname(obj)
                                     : distant_name(obj, xname));
                    obj->quan = scrquan;
                }
                /* useupf(), which charges, only if hero caused damage */
                if (u_caused)
                    useupf(obj, delquan);
                else if (delquan < scrquan)
                    obj->quan -= delquan;
                else
                    delobj(obj);
                cnt += delquan;
                if (give_feedback) {
                    if (delquan > 1L)
                        pline("%ld %s burn.", delquan, buf2);
                    else
                        pline("%s burns.", An(buf1));
                }
            }
        }
    }
    return cnt;
}

/* will zap/spell/breath attack score a hit against armor class `ac'? */
STATIC_OVL int
zap_hit(ac, type, is_wand)
int ac;
int type; /* either hero cast spell type or 0 */
boolean is_wand;
{
    int chance = rn2(20);
    int spell_bonus = type ? spell_hit_bonus(type, is_wand) : 0;

    /* small chance for naked target to avoid being hit */
    if (!chance)
        return rnd(10) < ac + spell_bonus;

    /* very high armor protection does not achieve invulnerability */
    ac = AC_VALUE(ac);

    return -(chance) < ac + spell_bonus;
}

STATIC_OVL void
disintegrate_mon(mon, type, fltxt)
struct monst *mon;
int type; /* hero vs other */
const char *fltxt;
{
    struct obj *otmp, *otmp2, *m_amulet = mlifesaver(mon);

    if (canseemon(mon)) {
        if (!m_amulet)
            pline("%s is disintegrated!", Monnam(mon));
        else
            hit(fltxt, mon, "!");
    }

/* note: worn amulet of life saving must be preserved in order to operate */
#define oresist_disintegration(obj) \
    (objects[obj->otyp].oc_oprop == DISINT_RES || obj_resists(obj, 5, 50) \
     || is_quest_artifact(obj) || obj == m_amulet)

    for (otmp = mon->minvent; otmp; otmp = otmp2) {
        otmp2 = otmp->nobj;
        if (!oresist_disintegration(otmp)) {
            /* If container has contents, check if any resist disintegration */
            if (Has_contents(otmp)) {
                struct obj *cobj, *cnext;
                boolean spilled = FALSE;

                for (cobj = otmp->cobj; cobj; cobj = cnext) {
                    cnext = cobj->nobj;
                    if (oresist_disintegration(cobj)) {
                        obj_extract_self(cobj);
                        if (otmp->otyp == ICE_BOX)
                            removed_from_icebox(cobj);
                        if (!flooreffects(cobj, mon->mx, mon->my, "")) {
                            place_object(cobj, mon->mx, mon->my);
                            stackobj(cobj);
                        }
                        if (!spilled && canseemon(mon)) {
                            pline("%s contents spill out onto the %s.",
                                  s_suffix(The(distant_name(otmp, xname))),
                                  surface(mon->mx, mon->my));
                            spilled = TRUE;
                        }
                    }
                }
            }
            extract_from_minvent(mon, otmp, TRUE, TRUE);
            obfree(otmp, (struct obj *) 0);
        }
    }

#undef oresist_disintegration

    if (type < 0)
        monkilled(mon, (char *) 0, -AD_RBRE);
    else
        xkilled(mon, XKILL_NOMSG | XKILL_NOCORPSE);
}

void
buzz(type, nd, sx, sy, dx, dy)
int type, nd;
xchar sx, sy;
int dx, dy;
{
    dobuzz(type, nd, sx, sy, dx, dy, TRUE);
}

/*
 * type ==   0 to  11 : you shooting a wand
 * type ==  12 to  22 : you casting a spell
 * type ==  23 to  33 : you breathing as a monster
 * type == -12 to -22 : monster casting spell
 * type == -23 to -33 : monster breathing at you
 * type == -34 to -44 : monster shooting a wand
 * called with dx = dy = 0 with vertical bolts
 */
void
dobuzz(type, nd, sx, sy, dx, dy, say)
register int type, nd;
register xchar sx, sy;
register int dx, dy;
boolean say; /* Announce out of sight hit/miss events if true */
{
    int range, abstype = BASE_ZT(abs(type));
    register xchar lsx, lsy;
    struct monst *mon;
    coord save_bhitpos;
    boolean shopdamage = FALSE;
    const char *fltxt;
    struct obj *otmp;
    int spell_type;
    boolean is_wand = (type >= 0 && type <= MAX_ZT);

    /* if its a Hero Spell then get its SPE_TYPE */
    spell_type = is_hero_spell(type) ? SPE_MAGIC_MISSILE + abstype : 0;

    fltxt = flash_types[(type <= -33) ? abstype : abs(type)];
    if (u.uswallow) {
        register int tmp;

        if (type < 0)
            return;
        tmp = zhitm(u.ustuck, type, nd, &otmp);
        if (!u.ustuck)
            u.uswallow = 0;
        else
            pline("%s rips into %s%s", The(fltxt), mon_nam(u.ustuck),
                  exclam(tmp));
        /* Using disintegration from the inside only makes a hole... */
        if (tmp == MAGIC_COOKIE)
            u.ustuck->mhp = 0;
        if (DEADMONSTER(u.ustuck))
            killed(u.ustuck);
        return;
    }
    if (type < 0)
        newsym(u.ux, u.uy);
    range = rn1(7, 7);
    if (dx == 0 && dy == 0)
        range = 1;
    save_bhitpos = bhitpos;

    tmp_at(DISP_BEAM, zapdir_to_glyph(dx, dy, abstype));
    while (range-- > 0) {
        lsx = sx;
        sx += dx;
        lsy = sy;
        sy += dy;
        if (!isok(sx, sy) || levl[sx][sy].typ == STONE)
            goto make_bounce;

        mon = m_at(sx, sy);
        if (cansee(sx, sy)) {
            /* reveal/unreveal invisible monsters before tmp_at() */
            if (mon && !canspotmon(mon))
                map_invisible(sx, sy);
            else if (!mon)
                (void) unmap_invisible(sx, sy);
            if (ZAP_POS(levl[sx][sy].typ)
                || (isok(lsx, lsy) && cansee(lsx, lsy)))
                tmp_at(sx, sy);
            delay_output(); /* wait a little */
        }

        /* hit() and miss() need bhitpos to match the target */
        bhitpos.x = sx, bhitpos.y = sy;
        /* Fireballs only damage when they explode */
        if (type != ZT_SPELL(ZT_FIRE)) {
            range += zap_over_floor(sx, sy, type, &shopdamage, 0,
                                    (type >= 0 ? FALSE : TRUE));
            /* zap with fire -> melt ice -> drown monster, so monster
               found and cached above might not be here any more */
            mon = m_at(sx, sy);
        }

        if (mon) {
            int saved_mhp;
            if (type == ZT_SPELL(ZT_FIRE))
                break;
            if (type >= 0)
                mon->mstrategy &= ~STRAT_WAITMASK;
 buzzmonst:
            saved_mhp = mon->mhp; /* for print_mon_wounded() */
            notonhead = (mon->mx != bhitpos.x || mon->my != bhitpos.y);
            if (zap_hit(find_mac(mon), spell_type, is_wand)) {
                if (mon_reflects(mon, (char *) 0)) {
                    if (cansee(mon->mx, mon->my)) {
                        hit(fltxt, mon, exclam(0));
                        shieldeff(mon->mx, mon->my);
                        (void) mon_reflects(mon,
                                            "But it reflects from %s %s!");
                    }
                    /* water is reflected but doesn't bounce */
                    if (abstype == ZT_WATER)
                        range = 0;
                    dx = -dx;
                    dy = -dy;
                } else {
                    boolean mon_could_move = mon->mcanmove;
                    int tmp = zhitm(mon, type, nd, &otmp);

                    if (is_rider(mon->data)
                        && abs(type) == ZT_BREATH(ZT_DEATH)) { /* disintegration */
                        if (canseemon(mon)) {
                            hit(fltxt, mon, ".");
                            pline("%s disintegrates.", Monnam(mon));
                            pline("%s body reintegrates before your %s!",
                                  s_suffix(Monnam(mon)),
                                  (eyecount(youmonst.data) == 1)
                                      ? body_part(EYE)
                                      : makeplural(body_part(EYE)));
                            pline("%s resurrects!", Monnam(mon));
                        }
                        mon->mhp = mon->mhpmax;
                        break; /* Out of while loop */
                    }
                    if (mon->data == &mons[PM_DEATH] && abstype == ZT_DEATH) {
                        if (canseemon(mon)) {
                            hit(fltxt, mon, ".");
                            pline("%s absorbs the deadly %s!", Monnam(mon),
                                  type == ZT_BREATH(ZT_DEATH) ? "blast"
                                                              : "ray");
                            pline("It seems even stronger than before.");
                        }
                        break; /* Out of while loop */
                    }
                    if (abstype == ZT_WATER
                        && (mon->data == &mons[PM_WATER_ELEMENTAL]
                            || mon->data == &mons[PM_BABY_SEA_DRAGON]
                            || mon->data == &mons[PM_SEA_DRAGON])) {
                        if (canseemon(mon))
                            pline("%s aborbs the blast of water into its body.",
                                  Monnam(mon));
                        range = 0;
                        break; /* Out of while loop */
                    }

                    if (tmp == MAGIC_COOKIE) { /* disintegration */
                        disintegrate_mon(mon, type, fltxt);
                    } else if (DEADMONSTER(mon)) {
                        if (type < 0) {
                            /* mon has just been killed by another monster */
                            monkilled(mon, fltxt, AD_RBRE);
                        } else {
                            int xkflags = XKILL_GIVEMSG; /* killed(mon); */

                            /* killed by hero; we know 'type' isn't negative;
                               if it's fire, highly flammable monsters leave
                               no corpse; don't bother reporting that they
                               "burn completely" -- unnecessary verbosity */
                            if ((BASE_ZT(type) == ZT_FIRE)
                                /* paper golem or straw golem */
                                && completelyburns(mon->data))
                                xkflags |= XKILL_NOCORPSE;
                            xkilled(mon, xkflags);
                        }
                    } else {
                        if (!otmp) {
                            /* normal non-fatal hit */
                            if (say || canseemon(mon)) {
                                hit(fltxt, mon, exclam(tmp));
                                if ((resists_magm(mon) || defended(mon, AD_MAGM))
                                    && abstype == ZT_DEATH
                                    && abs(type) != ZT_BREATH(ZT_DEATH)) { /* death */
                                    if (canseemon(mon))
                                        pline("%s resists the death magic, but appears drained!",
                                              Monnam(mon));
                                }
                                if (abstype == ZT_DRAIN) {
                                    const char *life = nonliving(mon->data) ? "animating force"
                                                                            : "life";

                                    if (canseemon(mon)) {
                                        if (resists_drli(mon) || defended(mon, AD_DRLI))
                                            pline("%s appears unaffected.", Monnam(mon));
                                        else
                                            pline_The("blast draws the %s from %s!",
                                                      life, mon_nam(mon));
                                    }
                                }
                                print_mon_wounded(mon, saved_mhp);
                            }
                        } else {
                            /* some armor was destroyed; no damage done */
                            if (canseemon(mon))
                                pline("%s %s is disintegrated!",
                                      s_suffix(Monnam(mon)),
                                      distant_name(otmp, xname));
                            m_useup(mon, otmp);
                        }
                        if (mon_could_move && !mon->mcanmove) /* ZT_SLEEP */
                            slept_monst(mon);
                    }
                }
                range -= 2;
            } else {
                if (say || canseemon(mon))
                    miss(fltxt, mon);
            }
        } else if (sx == u.ux && sy == u.uy && range >= 0) {
            nomul(0);
            if (u.usteed && !rn2(3) && !mon_reflects(u.usteed, (char *) 0)) {
                mon = u.usteed;
                goto buzzmonst;
            } else if (zap_hit((int) u.uac, 0, 0)) {
                range -= 2;
                pline("%s %s you!", The(fltxt),
                      abstype == ZT_WATER ? "slams into" : "hits");
                if (abstype == ZT_WATER
                    && (u.umonnum == PM_WATER_ELEMENTAL
                        || u.umonnum == PM_BABY_SEA_DRAGON
                        || u.umonnum == PM_SEA_DRAGON)) {
                    range = 0;
                }
                if (Reflecting) {
                    if (!Blind)
                        (void) ureflects("Some of %s reflects from your %s!",
                                         "it");
                    dx = -dx;
                    dy = -dy;
                    /* water is reflected but doesn't bounce */
                    if (abstype == ZT_WATER)
                        range = 0;
                    shieldeff(sx, sy);
                    nd = (nd + 1) / 2;
                    monstseesu(M_SEEN_REFL);
                }
                zhitu(type, nd, fltxt, sx, sy);
            } else if (!Blind) {
                pline("%s whizzes by you!", The(fltxt));
            } else if (abstype == ZT_LIGHTNING) {
                Your("%s tingles.", body_part(ARM));
            }
            if (abstype == ZT_LIGHTNING)
                (void) flashburn((long) d(nd, 50));
            stop_occupation();
            nomul(0);
        }

        if (levl[sx][sy].typ == TREE && abstype == ZT_DEATH
            && abs(type) != ZT_BREATH(ZT_DEATH)) {
            levl[sx][sy].typ = DEADTREE;
            if (cansee(sx, sy)) {
                pline("The tree withers and dies!");
                newsym(sx, sy);
            }
            if (type >= 0) {
                if (Role_if(PM_DRUID)) {
                    You_feel("very guilty.");
                    adjalign(-15);
                    change_luck(-7);
                    /* deity becomes "very" angry */
                    u.ugangr += 5;
                } else if (Race_if(PM_ELF)) {
                    /* Elves are also penalized, but
                       not as severely */
                    You_feel("guilty.");
                    adjalign(-5);
                    change_luck(-2);
                }
            }
            range = 0;
            break;
        }

        if (!ZAP_POS(levl[sx][sy].typ)
            || (closed_door(sx, sy) && range >= 0)) {
            int bounce, bchance;
            uchar rmn;
            boolean fireball;

 make_bounce:
            bchance = (!isok(sx, sy) || levl[sx][sy].typ == STONE) ? 10
                : (In_mines(&u.uz) && IS_WALL(levl[sx][sy].typ)) ? 20
                : 75;
            bounce = 0;
            fireball = (type == ZT_SPELL(ZT_FIRE));
            if ((--range > 0 && isok(lsx, lsy) && cansee(lsx, lsy))
                || fireball) {
                if (Is_airlevel(&u.uz)) { /* nothing to bounce off of */
                    pline_The("%s vanishes into the aether!", fltxt);
                    if (fireball)
                        type = ZT_WAND(ZT_FIRE); /* skip pending fireball */
                    break;
                } else if (fireball) {
                    sx = lsx;
                    sy = lsy;
                    break; /* fireballs explode before the obstacle */
                } else if (abstype == ZT_WATER) {
                    if (isok(sx, sy) && IS_STWALL(levl[sx][sy].typ))
                        pline("%s splashes against the wall!", The(fltxt));
                    break;
                } else
                    pline_The("%s bounces!", fltxt);
            }
            if (!dx || !dy || !rn2(bchance)) {
                dx = -dx;
                dy = -dy;
            } else {
                if (isok(sx, lsy) && ZAP_POS(rmn = levl[sx][lsy].typ)
                    && !closed_door(sx, lsy)
                    && (IS_ROOM(rmn) || (isok(sx + dx, lsy)
                                         && ZAP_POS(levl[sx + dx][lsy].typ))))
                    bounce = 1;
                if (isok(lsx, sy) && ZAP_POS(rmn = levl[lsx][sy].typ)
                    && !closed_door(lsx, sy)
                    && (IS_ROOM(rmn) || (isok(lsx, sy + dy)
                                         && ZAP_POS(levl[lsx][sy + dy].typ))))
                    if (!bounce || rn2(2))
                        bounce = 2;

                switch (bounce) {
                case 0:
                    dx = -dx;
                    /*FALLTHRU*/
                case 1:
                    dy = -dy;
                    break;
                case 2:
                    dx = -dx;
                    break;
                }
                tmp_at(DISP_CHANGE, zapdir_to_glyph(dx, dy, abstype));
            }
        }
    }
    tmp_at(DISP_END, 0);
    if (type == ZT_SPELL(ZT_FIRE))
        explode(sx, sy, type, d(12, 6), 0, EXPL_FIERY);
    if (shopdamage)
        pay_for_damage(abstype == ZT_FIRE
                          ? "burn away"
                          : abstype == ZT_COLD
                             ? "shatter"
                             /* "damage" indicates wall rather than door */
                             : abstype == ZT_ACID
                                ? "dissolve"
                                : abstype == ZT_DEATH
                                   ? "disintegrate"
                                   : "destroy",
                       FALSE);
    bhitpos = save_bhitpos;
}

void
melt_ice(x, y, msg)
xchar x, y;
const char *msg;
{
    struct rm *lev = &levl[x][y];
    struct obj *otmp;
    struct monst *mtmp;

    if (!msg)
        msg = "The ice crackles and melts.";
    if (lev->typ == DRAWBRIDGE_UP || lev->typ == DRAWBRIDGE_DOWN) {
        lev->drawbridgemask &= ~DB_ICE; /* revert to DB_MOAT */
    } else { /* lev->typ == ICE */
#ifdef STUPID
        if (lev->icedpool == ICED_POOL)
            lev->typ = POOL;
        if (lev->icedpool == ICED_PUDDLE)
            lev->typ = PUDDLE;
        if (lev->icedpool == ICED_SEWAGE)
            lev->typ = SEWAGE;
        else
            lev->typ = MOAT;
#else
        lev->typ = (lev->icedpool == ICED_POOL ? POOL
                        : lev->icedpool == ICED_PUDDLE ? PUDDLE
                              : lev->icedpool == ICED_SEWAGE ? SEWAGE : MOAT);
#endif
        lev->icedpool = 0;
    }
    spot_stop_timers(x, y, MELT_ICE_AWAY); /* no more ice to melt away */
    if (t_at(x, y))
        trap_ice_effects(x, y, TRUE); /* TRUE because ice_is_melting */
    obj_ice_effects(x, y, FALSE);
    if (!(lev->typ == PUDDLE || lev->typ == SEWAGE))
        unearth_objs(x, y);
    if (Underwater)
        vision_recalc(1);
    newsym(x, y);
    if (cansee(x, y))
        Norep("%s", msg);
    if (!(lev->typ == PUDDLE || lev->typ == SEWAGE)
        && (otmp = sobj_at(BOULDER, x, y)) != 0) {
        if (cansee(x, y))
            pline("%s settles...", An(xname(otmp)));
        do {
            obj_extract_self(otmp); /* boulder isn't being pushed */
            if (!boulder_hits_pool(otmp, x, y, FALSE))
                impossible("melt_ice: no pool?");
            /* try again if there's another boulder and pool didn't fill */
        } while (is_pool(x, y) && (otmp = sobj_at(BOULDER, x, y)) != 0);
        newsym(x, y);
    }
    if (x == u.ux && y == u.uy)
        spoteffects(TRUE); /* possibly drown, notice objects */
    else if (is_pool(x, y) && (mtmp = m_at(x, y)) != 0)
        (void) minliquid(mtmp);
}

#define MIN_ICE_TIME 50
#define MAX_ICE_TIME 2000
/*
 * Usually start a melt_ice timer; sometimes the ice will become
 * permanent instead.
 */
void
start_melt_ice_timeout(x, y, min_time)
xchar x, y;
long min_time; /* <x,y>'s old melt timeout (deleted by time we get here) */
{
    int when;
    long where;

    when = (int) min_time;
    if (when < MIN_ICE_TIME - 1)
        when = MIN_ICE_TIME - 1;

    /* random timeout; surrounding ice locations ought to be a factor... */
    while (++when <= MAX_ICE_TIME)
        if (!rn2((MAX_ICE_TIME - when) + MIN_ICE_TIME))
            break;

    /* if we're within MAX_ICE_TIME, install a melt timer;
       otherwise, omit it to leave this ice permanent */
    if (when <= MAX_ICE_TIME) {
        where = ((long) x << 16) | (long) y;
        (void) start_timer((long) when, TIMER_LEVEL, MELT_ICE_AWAY,
                           long_to_any(where));
    }
}
#undef MIN_ICE_TIME
#undef MAX_ICE_TIME

/*
 * Called when ice has melted completely away.
 */
void
melt_ice_away(arg, timeout)
anything *arg;
long timeout UNUSED;
{
    xchar x, y;
    long where = arg->a_long;
    boolean save_mon_moving = context.mon_moving; /* will be False */

    /* melt_ice -> minliquid -> mondead|xkilled shouldn't credit/blame hero */
    context.mon_moving = TRUE; /* hero isn't causing this ice to melt */
    y = (xchar) (where & 0xFFFF);
    x = (xchar) ((where >> 16) & 0xFFFF);
    /* melt_ice does newsym when appropriate */
    melt_ice(x, y, "Some ice melts away.");
    context.mon_moving = save_mon_moving;
}

/* Burn floor scrolls, evaporate pools, etc... in a single square.
 * Used both for normal bolts of fire, cold, etc... and for fireballs.
 * Sets shopdamage to TRUE if a shop door is destroyed, and returns the
 * amount by which range is reduced (the latter is just ignored by fireballs)
 *
 * If moncast is TRUE, then a monster is causing this damage; do not penalize
 * the player for any shop damage or miscellaneous item destruction.
 */
int
zap_over_floor(x, y, type, shopdamage, exploding_wand_typ, moncast)
xchar x, y;
int type;
boolean *shopdamage;
short exploding_wand_typ;
boolean moncast;
{
    const char *zapverb;
    struct monst *mon;
    struct trap *t;
    struct rm *lev = &levl[x][y];
    boolean see_it = cansee(x, y), yourzap;
    int rangemod = 0, abstype = BASE_ZT(abs(type));

    yourzap = (type >= 0 && !exploding_wand_typ && !moncast);

    if (type == PHYS_EXPL_TYPE) {
        /* this won't have any effect on the floor */
        return -1000; /* not a zap anyway, shouldn't matter */
    }

    switch (abstype) {
    case ZT_FIRE:
        t = t_at(x, y);
        if (t && t->ttyp == WEB) {
            /* a burning web is too flimsy to notice if you can't see it */
            if (see_it)
                Norep("A web bursts into flames!");
            (void) delfloortrap(t);
            if (see_it)
                newsym(x, y);
        }
        if (is_ice(x, y)) {
            melt_ice(x, y, (char *) 0);
        } else if (is_pool(x, y)) {
            const char *msgtxt = (!Deaf)
                                     ? "You hear hissing gas." /* Deaf-aware */
                                     : (type >= 0)
                                         ? "That seemed remarkably uneventful."
                                         : (const char *) 0;

            if (lev->typ != POOL) { /* MOAT or DRAWBRIDGE_UP */
                if (see_it)
                    msgtxt = "Some water evaporates.";
            } else {
                rangemod -= 3;
                lev->typ = ROOM, lev->flags = 0;
                t = maketrap(x, y, PIT);
                if (t)
                    t->tseen = 1;
                if (see_it)
                    msgtxt = "The water evaporates.";
            }
            if (msgtxt)
                Norep("%s", msgtxt);
            if (lev->typ == ROOM) {
                if ((mon = m_at(x, y)) != 0) {
                    /* probably ought to do some hefty damage to any
                       creature caught in boiling water;
                       at a minimum, eels are forced out of hiding */
                    if (is_swimmer(mon->data) && mon->mundetected) {
                        mon->mundetected = 0;
                    }
                }
                newsym(x, y);
            }
        } else if (IS_FOUNTAIN(lev->typ)) {
            if (see_it)
                pline("Steam billows from the fountain.");
            rangemod -= 1;
            dryup(x, y, yourzap ? TRUE : FALSE);
        } else if (IS_PUDDLE(lev->typ) || IS_SEWAGE(lev->typ)) {
            if (see_it) {
                if (IS_PUDDLE(lev->typ))
                    pline_The("shallow water boils away.");
                else if (IS_SEWAGE(lev->typ))
                    pline_The("raw sewage boils away.");
            } else {
                You_hear("hissing gas.");
            }
            rangemod -= 3;
            lev->typ = ROOM, lev->flags = 0;
            if (lev->typ == ROOM) {
                if ((mon = m_at(x, y)) != 0) {
                    if (is_swimmer(mon->data) && mon->mundetected) {
                        mon->mundetected = 0;
                    }
                }
                newsym(x, y);
            }
        } else if (lev->typ == TREE) {
            if (see_it)
                pline("The tree burns to a crisp!");
            lev->typ = DEADTREE;
            if (lev->typ == DEADTREE)
                newsym(x, y);
            if (yourzap) {
                if (Role_if(PM_DRUID)) {
                    You_feel("very guilty.");
                    adjalign(-15);
                    change_luck(-7);
                    /* deity becomes "very" angry */
                    u.ugangr += 5;
                } else if (Race_if(PM_ELF)) {
                    /* Elves are also penalized, but
                       not as severely */
                    You_feel("guilty.");
                    adjalign(-5);
                    change_luck(-2);
                }
            }
        } else if (lev->typ == DEADTREE) {
            if (!may_dig(x, y)) {
                /* nothing happens - it's petrified */
                ;
            } else {
                if (see_it)
                    pline("The dead tree burns to ashes!");
                rangemod -= 1000; /* stop */
                lev->typ = ROOM, lev->flags = 0;
                if (lev->typ == ROOM)
                    newsym(x, y);
                if (!does_block(x, y, &levl[x][y]))
                    unblock_point(x, y); /* vision:  can see through */
                feel_newsym(x, y);
            }
        } else if (IS_GRASS(lev->typ)) {
            lev->typ = ROOM, lev->flags = 0;
            if (see_it)
                Norep("The grass is burned away!");
            if (lev->typ == ROOM) {
                if ((mon = m_at(x, y)) != 0) {
                    /* probably ought to do some hefty damage to any
                       creature caught in burning grass;
                       at a minimum, hiders are forced out of hiding */
                    if (hides_under(mon->data) && mon->mundetected) {
                        mon->mundetected = 0;
                    }
                }
                newsym(x, y);
            }
        }
        break; /* ZT_FIRE */

    case ZT_COLD:
        if (is_damp_terrain(x, y) || is_lava(x, y)) {
            boolean lava = is_lava(x, y),
                    moat = is_moat(x, y);

            if (lev->typ == WATER) {
                /* For now, don't let WATER freeze. */
                if (see_it)
                    pline_The("%s freezes for a moment.", hliquid("water"));
                else
                    You_hear("a soft crackling.");
                rangemod -= 1000; /* stop */
            } else {
                char buf[BUFSZ];

                Strcpy(buf, waterbody_name(x, y)); /* for MOAT */
                rangemod -= 3;
                if (lev->typ == DRAWBRIDGE_UP) {
                    lev->drawbridgemask &= ~DB_UNDER; /* clear lava */
                    lev->drawbridgemask |= (lava ? DB_FLOOR : DB_ICE);
                } else {
                    lev->icedpool = lava ? 0
                                         : (lev->typ == POOL) ? ICED_POOL
                                                : (lev->typ == PUDDLE) ? ICED_PUDDLE
                                                       : (lev->typ == SEWAGE) ? ICED_SEWAGE
                                                                              : ICED_MOAT;
                    lev->typ = lava ? ROOM : ICE;
                }
                if (!(lev->icedpool == ICED_PUDDLE
                      || lev->icedpool == ICED_SEWAGE))
                    bury_objs(x, y);
                if (see_it) {
                    if (lava)
                        Norep("The %s cools and solidifies.", hliquid("lava"));
                    else if (moat)
                        Norep("The %s is bridged with ice!", buf);
                    else
                        Norep("The %s freezes.", hliquid("water"));
                    newsym(x, y);
                } else if (!lava)
                    You_hear("a crackling sound.");

                if (x == u.ux && y == u.uy) {
                    if (u.uinwater) { /* not just `if (Underwater)' */
                        /* leave the no longer existent water */
                        u.uinwater = 0;
                        u.uundetected = 0;
                        docrt();
                        vision_full_recalc = 1;
                    } else if (u.utrap && u.utraptype == TT_LAVA) {
                        if (Passes_walls) {
                            You("pass through the now-solid rock.");
                            reset_utrap(TRUE);
                        } else {
                            set_utrap(rn1(50, 20), TT_INFLOOR);
                            You("are firmly stuck in the cooling rock.");
                        }
                    }
                } else if ((mon = m_at(x, y)) != 0) {
                    /* probably ought to do some hefty damage to any
                       non-ice creature caught in freezing water;
                       at a minimum, eels are forced out of hiding */
                    if (is_swimmer(mon->data) && mon->mundetected) {
                        mon->mundetected = 0;
                        newsym(x, y);
                    }
                }
                if (!lava) {
                    start_melt_ice_timeout(x, y, 0L);
                    obj_ice_effects(x, y, TRUE);
                }
            } /* ?WATER */

        } else if (is_ice(x, y)) {
            long melt_time;

            /* Already ice here, so just firm it up. */
            /* Now ensure that only ice that is already timed is affected */
            if ((melt_time = spot_time_left(x, y, MELT_ICE_AWAY)) != 0L) {
                spot_stop_timers(x, y, MELT_ICE_AWAY);
                start_melt_ice_timeout(x, y, melt_time);
            }
        }
        break; /* ZT_COLD */

    case ZT_POISON_GAS:
        (void) create_gas_cloud(x, y, 1, 8);
        break;

    case ZT_ACID:
        if (lev->typ == IRONBARS) {
            if ((lev->wall_info & W_NONDIGGABLE) != 0) {
                if (see_it)
                    Norep("The %s corrode somewhat but remain intact.",
                          defsyms[S_bars].explanation);
                /* but nothing actually happens... */
            } else {
                rangemod -= 3;
                if (see_it)
                    Norep("The %s melt.", defsyms[S_bars].explanation);
                if (*in_rooms(x, y, SHOPBASE)) {
                    /* in case we ever have a shop bounded by bars */
                    lev->typ = ROOM, lev->flags = 0;
                    if (see_it)
                        newsym(x, y);
                    add_damage(x, y, yourzap ? SHOP_BARS_COST : 0L);
                    if (yourzap)
                        *shopdamage = TRUE;
                } else {
                    lev->typ = DOOR, lev->doormask = D_NODOOR;
                    if (see_it)
                        newsym(x, y);
                }
            }
        }
        break; /* ZT_ACID */

    default:
        break;
    }

    /* set up zap text for possible door feedback; for exploding wand, we
       want "the blast" rather than "your blast" even if hero caused it */
    zapverb = "blast"; /* breath attack or wand explosion */
    if (!exploding_wand_typ) {
        if (abs(type) < ZT_SPELL(0))
            zapverb = "bolt"; /* wand zap */
        else if (abs(type) < ZT_BREATH(0))
            zapverb = "spell";
    } else if (exploding_wand_typ == POT_OIL
               || exploding_wand_typ == SCR_FIRE) {
        /* breakobj() -> explode_oil() -> splatter_burning_oil()
           -> explode(ZT_SPELL(ZT_FIRE), BURNING_OIL)
           -> zap_over_floor(ZT_SPELL(ZT_FIRE), POT_OIL) */
        /* leave zapverb as "blast"; exploding_wand_typ was nonzero, so
           'yourzap' is FALSE and the result will be "the blast" */
        exploding_wand_typ = 0; /* not actually an exploding wand */
    }

    /* secret door gets revealed, converted into regular door */
    if (levl[x][y].typ == SDOOR) {
        cvt_sdoor_to_door(&levl[x][y]); /* .typ = DOOR */
        /* target spot will now pass closed_door() test below
           (except on rogue level) */
        newsym(x, y);
        if (see_it)
            pline("%s %s reveals a secret door.",
                  yourzap ? "Your" : "The", zapverb);
        else if (Is_rogue_level(&u.uz))
            draft_message(FALSE); /* "You feel a draft." (open doorway) */
    }

    /* regular door absorbs remaining zap range, possibly gets destroyed */
    if (closed_door(x, y)) {
        int new_doormask = -1;
        const char *see_txt = 0, *sense_txt = 0, *hear_txt = 0;

        rangemod = -1000;
        if ((lev->doormask & D_TRAPPED) && In_sokoban(&u.uz))
            goto def_case;
        switch (abstype) {
        case ZT_FIRE:
            new_doormask = D_NODOOR;
            see_txt = "The door is consumed in flames!";
            sense_txt = "smell smoke.";
            break;
        case ZT_COLD:
            new_doormask = D_NODOOR;
            see_txt = "The door freezes and shatters!";
            hear_txt = "a deep cracking sound.";
            break;
        case ZT_DEATH:
            /* death spells/wands don't disintegrate */
            if (abs(type) != ZT_BREATH(ZT_DEATH))
                goto def_case;
            new_doormask = D_NODOOR;
            see_txt = "The door disintegrates!";
            hear_txt = "crashing wood.";
            break;
        case ZT_LIGHTNING:
            new_doormask = D_BROKEN;
            see_txt = "The door splinters!";
            hear_txt = "crackling.";
            break;
        case ZT_POISON_GAS:
            new_doormask = D_BROKEN;
            see_txt = "The door rots away!";
            hear_txt = "hollow thud.";
            break;
        case ZT_ACID:
            new_doormask = D_NODOOR;
            see_txt = "The door dissolves!";
            hear_txt = "hissing.";
            break;
        default:
 def_case:
            if (exploding_wand_typ > 0) {
                /* Magical explosion from misc exploding wand */
                if (exploding_wand_typ == WAN_STRIKING) {
                    new_doormask = D_BROKEN;
                    see_txt = "The door crashes open!";
                    sense_txt = "feel a burst of cool air.";
                    break;
                }
            }
            if (see_it) {
                /* "the door absorbs the blast" would be
                   inaccurate for an exploding wand since
                   other adjacent locations still get hit */
                if (exploding_wand_typ)
                    pline_The("door remains intact.");
                else
                    pline_The("door absorbs %s %s!", yourzap ? "your" : "the",
                              zapverb);
            } else
                You_feel("vibrations.");
            break;
        }
        if (new_doormask >= 0) { /* door gets broken */
            if (*in_rooms(x, y, SHOPBASE)) {
                if (yourzap) {
                    add_damage(x, y, SHOP_DOOR_COST);
                    *shopdamage = TRUE;
                } else /* caused by monster */
                    add_damage(x, y, 0L);
            }
            lev->doormask = new_doormask;
            unblock_point(x, y); /* vision */
            if (see_it) {
                pline1(see_txt);
                newsym(x, y);
            } else if (sense_txt) {
                You1(sense_txt);
            } else if (hear_txt)
                You_hear1(hear_txt);
            if (picking_at(x, y)) {
                stop_occupation();
                reset_pick();
            }
        }
    }

    if (OBJ_AT(x, y) && abstype == ZT_FIRE)
        if (burn_floor_objects(x, y, FALSE, yourzap ? TRUE : FALSE)
            && couldsee(x, y)) {
            newsym(x, y);
            You("%s of smoke.", !Blind ? "see a puff" : "smell a whiff");
        }
    if ((mon = m_at(x, y)) != 0) {
        wakeup(mon, FALSE);
        if (yourzap) {
            setmangry(mon, TRUE);
            if (mon->ispriest && *in_rooms(mon->mx, mon->my, TEMPLE))
                ghod_hitsu(mon);
            if (mon->isshk && !*u.ushops)
                hot_pursuit(mon);
        }
    }
    return rangemod;
}

/* fractured by pick-axe or wand of striking */
void
fracture_rock(obj)
register struct obj *obj; /* no texts here! */
{
    xchar x, y;
    boolean by_you = !context.mon_moving;

    if (by_you && get_obj_location(obj, &x, &y, 0) && costly_spot(x, y)) {
        struct monst *shkp = 0;
        char objroom = *in_rooms(x, y, SHOPBASE);

        if (billable(&shkp, obj, objroom, FALSE)) {
            /* shop message says "you owe <shk> <$> for it!" so we need
               to precede that with a message explaining what "it" is */
            You("fracture %s %s.", s_suffix(shkname(shkp)), xname(obj));
            breakobj(obj, x, y, TRUE, FALSE); /* charges for shop goods */
        }
    }
    if (by_you && obj->otyp == BOULDER)
        sokoban_guilt();

    obj->otyp = ROCK;
    obj->oclass = GEM_CLASS;
    obj->quan = (long) rn1(60, 7);
    obj->owt = weight(obj);
    obj->dknown = obj->bknown = obj->rknown = 0;
    obj->known = objects[obj->otyp].oc_uses_known ? 0 : 1;
    dealloc_oextra(obj);

    if (obj->where == OBJ_FLOOR) {
        obj_extract_self(obj); /* move rocks back on top */
        place_object(obj, obj->ox, obj->oy);
        if (!does_block(obj->ox, obj->oy, &levl[obj->ox][obj->oy]))
            unblock_point(obj->ox, obj->oy);
        if (cansee(obj->ox, obj->oy))
            newsym(obj->ox, obj->oy);
    }
}

/* handle statue hit by striking/force bolt/pick-axe */
boolean
break_statue(obj)
register struct obj *obj;
{
    /* [obj is assumed to be on floor, so no get_obj_location() needed] */
    struct trap *trap = t_at(obj->ox, obj->oy);
    struct obj *item;
    boolean by_you = !context.mon_moving;

    if (trap && trap->ttyp == STATUE_TRAP
        && activate_statue_trap(trap, obj->ox, obj->oy, TRUE))
        return FALSE;
    /* drop any objects contained inside the statue */
    while ((item = obj->cobj) != 0) {
        obj_extract_self(item);
        place_object(item, obj->ox, obj->oy);
    }
    if (by_you && Role_if(PM_ARCHEOLOGIST) && (obj->spe & STATUE_HISTORIC)) {
        You_feel("guilty about damaging such a historic statue.");
        adjalign(-1);
    }
    obj->spe = 0;
    fracture_rock(obj);
    return TRUE;
}

/*
 * destroy_strings[dindx][0:singular,1:plural,2:killer_reason]
 *      [0] freezing potion
 *      [1] boiling potion other than oil
 *      [2] boiling potion of oil
 *      [3] burning scroll
 *      [4] burning spellbook
 *      [5] shocked ring
 *      [6] shocked wand
 *      [7] sound potion
 *      [8] sound item
 *      [9] sound wand
 * (books, rings, and wands don't stack so don't need plural form;
 *  crumbling ring doesn't do damage so doesn't need killer reason)
 */
const char *const destroy_strings[][3] = {
    /* also used in trap.c */
    { "freezes and shatters", "freeze and shatter", "shattered potion" },
    { "boils and explodes", "boil and explode", "boiling potion" },
    { "ignites and explodes", "ignite and explode", "exploding potion" },
    { "catches fire and burns", "catch fire and burn", "burning scroll" },
    { "catches fire and burns", "", "burning book" },
    { "turns to dust and vanishes", "", "" },
    { "breaks apart and explodes", "", "exploding wand" },
    { "resonates and shatters", "resonate and shatter", "shattered potion"},
    { "resonates and shatters", "", "shattered item"},
    { "vibrates and shatters", "", "shattered wand"},
};

/* guts of destroy_item(), which ought to be called maybe_destroy_items();
   caller must decide whether obj is eligible */
void
destroy_one_item(obj, osym, dmgtyp)
struct obj *obj;
int osym, dmgtyp;
{
    long i, cnt, quan;
    int dmg, xresist, skip, dindx;
    const char *mult;
    boolean physical_damage;

    physical_damage = FALSE;
    xresist = skip = 0;
    /* lint suppression */
    dmg = dindx = 0;
    quan = 0L;

    switch (dmgtyp) {
    case AD_COLD:
        if (osym == POTION_CLASS && obj->otyp != POT_OIL) {
            quan = obj->quan;
            dindx = 0;
            dmg = rnd(4);
        } else
            skip++;
        break;
    case AD_FIRE:
        xresist = (how_resistant(FIRE_RES) == 100 && obj->oclass != POTION_CLASS
                   && obj->otyp != GLOB_OF_GREEN_SLIME);
        if (obj->otyp == SCR_FIRE || obj->otyp == SPE_FIREBALL)
            skip++;
        if (obj->otyp == SPE_BOOK_OF_THE_DEAD) {
            skip++;
            if (!Blind)
                pline("%s glows a strange %s, but remains intact.",
                      The(xname(obj)), hcolor("dark red"));
        }
        quan = obj->quan;
        switch (osym) {
        case POTION_CLASS:
            dindx = (obj->otyp != POT_OIL) ? 1 : 2;
            dmg = rnd(6);
            break;
        case SCROLL_CLASS:
            dindx = 3;
            dmg = 1;
            break;
        case SPBOOK_CLASS:
            dindx = 4;
            dmg = 1;
            break;
        case FOOD_CLASS:
            if (obj->otyp == GLOB_OF_GREEN_SLIME) {
                dindx = 1; /* boil and explode */
                dmg = (obj->owt + 19) / 20;
            } else {
                skip++;
            }
            break;
        default:
            skip++;
            break;
        }
        break;
    case AD_ELEC:
        xresist = (how_resistant(SHOCK_RES) == 100 && obj->oclass != RING_CLASS);
        quan = obj->quan;
        switch (osym) {
        case RING_CLASS:
            /* The Ring of P'hul and the One Ring are not affected,
               artifacts are off-limits per destroy_item() */
            if (((obj->owornmask & W_RING) && uarmg && !is_metallic(uarmg))
                || obj->otyp == RIN_SHOCK_RESISTANCE) {
                skip++;
                break;
            }
            if ((obj->oerodeproof || is_supermaterial(obj))
                && rn2(3)) {
                Your("%s resists!", xname(obj));
                skip++;
                break;
            }
            dindx = 5;
            dmg = 0;
            break;
        case WAND_CLASS:
            if (obj->otyp == WAN_LIGHTNING) {
                skip++;
                break;
            }
            if ((obj->oerodeproof || is_supermaterial(obj))
                && rn2(3)) {
                Your("%s resists!", xname(obj));
                skip++;
                break;
            }
            dindx = 6;
            dmg = rnd(10);
            break;
        default:
            skip++;
            break;
        }
        break;
    case AD_LOUD:
        if (objects[obj->otyp].oc_material == GLASS
            && obj->oerodeproof) {
            skip++;
            if (!Blind)
                Your("%s oscillates briefly, but remains intact.",
                     xname(obj));
        }
        if (objects[obj->otyp].oc_material == GLASS
            && !obj->oerodeproof) {
            quan = obj->quan;
            switch (osym) {
            case POTION_CLASS:
                dmg = rnd(4);
                dindx = 7;
                break;
            case RING_CLASS:
            case TOOL_CLASS:
                dmg = 1;
                dindx = 8;
                break;
            case WAND_CLASS:
                dmg = rnd(6);
                dindx = 9;
                break;
            default:
                skip++;
                break;
            }
        } else
            skip++;
        break;
    default:
        skip++;
        break;
    }

    if (!skip) {
        if (obj->in_use)
            --quan; /* one will be used up elsewhere */
        for (i = cnt = 0L; i < quan; i++)
            if (!rn2(3))
                cnt++;

        if (!cnt && IS_FORGE(levl[u.ux][u.uy].typ))
            cnt = 1L; /* forge dipping, always at least one destroyed */
        if (!cnt)
            return;
        mult = (cnt == 1L)
                ? ((quan == 1L) ? "Your"                         /* 1 of 1 */
                                : "One of your")                 /* 1 of N */
                : ((cnt < quan) ? "Some of your"                 /* n of N */
                                : (quan == 2L) ? "Both of your"  /* 2 of 2 */
                                               : "All of your"); /* N of N */
        pline("%s %s %s!", mult, xname(obj),
              destroy_strings[dindx][(cnt > 1L)]);
        if (osym == POTION_CLASS && dmgtyp != AD_COLD) {
            if (!Breathless_nomagic || haseyes(youmonst.data))
                potionbreathe(obj);
        }
        if (obj->owornmask) {
            if (obj->owornmask & W_RING) /* ring being worn */
                Ring_gone(obj);
            else
                setnotworn(obj);
        }
        if (obj == current_wand)
            current_wand = 0; /* destroyed */
        for (i = 0; i < cnt; i++)
            useup(obj);
        if (dmg) {
            if (xresist) {
                You("aren't hurt!");
            } else {
                const char *how = destroy_strings[dindx][2];
                boolean one = (cnt == 1L);

                if (dmgtyp == AD_FIRE && osym == FOOD_CLASS)
                    how = "exploding glob of slime";
                if (physical_damage)
                    dmg = Maybe_Half_Phys(dmg);
                losehp(dmg, one ? how : (const char *) makeplural(how),
                       one ? KILLED_BY_AN : KILLED_BY);
                exercise(A_STR, FALSE);
            }
        }
    }
}

/* target items of specified class for possible destruction */
void
destroy_item(osym, dmgtyp)
int osym, dmgtyp;
{
    register struct obj *obj;
    int i, deferral_indx = 0;
    /* 1+52+1: try to handle a full inventory; it doesn't matter if
      inventory actually has more, even if everything should be deferred */
    unsigned short deferrals[1 + 52 + 1]; /* +1: gold, overflow */

    (void) memset((genericptr_t) deferrals, 0, sizeof deferrals);
    /*
     * Sometimes destroying an item can change inventory aside from
     * the item itself (cited case was a potion of unholy water; when
     * boiled, potionbreathe() caused hero to transform into were-beast
     * form and that resulted in dropping or destroying some worn armor).
     *
     * Unlike other uses of the object bybass mechanism, destroy_item()
     * can be called multiple times for the same event.  So we have to
     * explicitly clear it before each use and hope no other section of
     * code expects it to retain previous value.
     *
     * Destruction of a ring of levitation or form change which pushes
     * off levitation boots could drop hero onto a fire trap that
     * could destroy other items and we'll get called recursively.  Or
     * onto a trap which transports hero elsewhere, which won't disrupt
     * traversal but could yield message sequencing issues.  So we
     * defer handling such things until after rest of inventory has
     * been processed.  If some other combination of items and events
     * triggers a recursive call, rest of inventory after the triggering
     * item will be skipped by the outer call since the inner one will
     * have set the bypass bits of the whole list.
     *
     * [Unfortunately, death while poly'd into flyer and subsequent
     * rehumanization could also drop hero onto a trap, and there's no
     * straightforward way to defer that.  Things could be improved by
     * redoing this to use two passes, first to collect a list or array
     * of o_id and quantity of what is targetted for destruction,
     * second pass to handle the destruction.]
     */
    bypass_objlist(invent, FALSE); /* clear bypass bit for invent */

    while ((obj = nxt_unbypassed_obj(invent)) != 0) {
        if (obj->oclass != osym)
            continue; /* test only objs of type osym */
        if (obj->oartifact)
            continue; /* don't destroy artifacts */
        if (obj->in_use && obj->quan == 1L)
            continue; /* not available */
        if (dmgtyp == AD_ELEC && wielding_artifact(ART_TEMPEST))
            continue; /* tempest grants shock resistance
                         to objects in open inventory */
        if ((dmgtyp == AD_FIRE || dmgtyp == AD_COLD)
            && wielding_artifact(ART_DICHOTOMY))
            continue; /* dichotomy grants fire/cold resistance
                         to objects in open inventory */
        if (dmgtyp == AD_ACID && wielding_artifact(ART_HARBINGER))
            continue; /* harbinger grants acid resistance
                         to objects in open inventory */

        /* if loss of this item might dump us onto a trap, hold off
           until later because potential recursive destroy_item() will
           result in setting bypass bits on whole chain--we would skip
           the rest as already processed once control returns here */
        if (deferral_indx < SIZE(deferrals)
            && ((obj->owornmask != 0L
                 && (objects[obj->otyp].oc_oprop == LEVITATION
                     || objects[obj->otyp].oc_oprop == FLYING))
                /* destroyed wands and potions of polymorph don't trigger
                   polymorph so don't need to be deferred */
                || (obj->otyp == POT_WATER && u.ulycn >= LOW_PM
                    && (Upolyd ? obj->blessed : obj->cursed)))) {
            deferrals[deferral_indx++] = obj->o_id;
            continue;
        }
        /* obj is eligible; maybe destroy it */
        destroy_one_item(obj, osym, dmgtyp);
    }
    /* if we saved some items for later (most likely just a worn ring
       of levitation) and they're still in inventory, handle them now */
    for (i = 0; i < deferral_indx; ++i) {
        /* note: obj->nobj is only referenced when obj is skipped;
           having obj be dropped or destroyed won't affect traversal */
        for (obj = invent; obj; obj = obj->nobj)
            if (obj->o_id == deferrals[i]) {
                destroy_one_item(obj, osym, dmgtyp);
                break;
            }
    }
    return;
}

int
destroy_mitem(mtmp, osym, dmgtyp)
struct monst *mtmp;
int osym, dmgtyp;
{
    struct obj *obj;
    int skip, tmp = 0;
    long i, cnt, quan;
    int dindx;
    boolean vis;

    if (mtmp == &youmonst) { /* this simplifies artifact_hit() */
        destroy_item(osym, dmgtyp);
        return 0; /* arbitrary; value doesn't matter to artifact_hit() */
    }

    vis = canseemon(mtmp);

    /* see destroy_item(); object destruction could disrupt inventory list */
    bypass_objlist(mtmp->minvent, FALSE); /* clear bypass bit for minvent */

    while ((obj = nxt_unbypassed_obj(mtmp->minvent)) != 0) {
        if (obj->oclass != osym)
            continue; /* test only objs of type osym */
        if (obj->oartifact)
            continue; /* don't destroy artifacts */
        if (dmgtyp == AD_ELEC && MON_WEP(mtmp)
            && MON_WEP(mtmp)->oartifact == ART_TEMPEST)
            continue; /* tempest grants shock resistance
                         to objects in open inventory */
        if ((dmgtyp == AD_FIRE || dmgtyp == AD_COLD)
            && MON_WEP(mtmp) && MON_WEP(mtmp)->oartifact == ART_DICHOTOMY)
            continue; /* dichotomy grants fire/cold resistance
                         to objects in open inventory */
        if (dmgtyp == AD_ACID && MON_WEP(mtmp)
            && MON_WEP(mtmp)->oartifact == ART_HARBINGER)
            continue; /* harbinger grants acid resistance
                         to objects in open inventory */
        skip = 0;
        quan = 0L;
        dindx = 0;

        switch (dmgtyp) {
        case AD_COLD:
            if (osym == POTION_CLASS && obj->otyp != POT_OIL) {
                quan = obj->quan;
                dindx = 0;
                tmp++;
            } else
                skip++;
            break;
        case AD_FIRE:
            if (obj->otyp == SCR_FIRE || obj->otyp == SPE_FIREBALL)
                skip++;
            if (obj->otyp == SPE_BOOK_OF_THE_DEAD) {
                skip++;
                if (vis)
                    pline("%s glows a strange %s, but remains intact.",
                          The(distant_name(obj, xname)), hcolor("dark red"));
            }
            quan = obj->quan;
            switch (osym) {
            case POTION_CLASS:
                dindx = (obj->otyp != POT_OIL) ? 1 : 2;
                tmp++;
                break;
            case SCROLL_CLASS:
                dindx = 3;
                tmp++;
                break;
            case SPBOOK_CLASS:
                dindx = 4;
                tmp++;
                break;
            case FOOD_CLASS:
                if (obj->otyp == GLOB_OF_GREEN_SLIME) {
                    dindx = 1; /* boil and explode */
                    tmp += (obj->owt + 19) / 20;
                } else {
                    skip++;
                }
                break;
            default:
                skip++;
                break;
            }
            break;
        case AD_ELEC:
            quan = obj->quan;
            switch (osym) {
            case RING_CLASS:
                if ((which_armor(mtmp, W_ARMG)
                     && !is_metallic(which_armor(mtmp, W_ARMG))
                     && obj->owornmask & W_RING)
                    || obj->otyp == RIN_SHOCK_RESISTANCE) {
                    skip++;
                    break;
                }
                if ((obj->oerodeproof || is_supermaterial(obj))
                    && rn2(3)) {
                    if (vis)
                        pline("%s %s resists!", s_suffix(Monnam(mtmp)), xname(obj));
                    skip++;
                    break;
                }
                dindx = 5;
                break;
            case WAND_CLASS:
                if (obj->otyp == WAN_LIGHTNING) {
                    skip++;
                    break;
                }
                if ((obj->oerodeproof || is_supermaterial(obj))
                    && rn2(3)) {
                    if (vis)
                        pline("%s %s resists!", s_suffix(Monnam(mtmp)), xname(obj));
                    skip++;
                    break;
                }
                dindx = 6;
                tmp++;
                break;
            default:
                skip++;
                break;
            }
            break;
        case AD_LOUD:
            if (objects[obj->otyp].oc_material == GLASS
                && obj->oerodeproof) {
                skip++;
                if (vis)
                    pline("%s oscillates briefly, but remains intact.",
                          The(distant_name(obj, xname)));
            }
            if (objects[obj->otyp].oc_material == GLASS
                && !obj->oerodeproof) {
                quan = obj->quan;
                switch (osym) {
                case POTION_CLASS:
                    dindx = 7;
                    break;
                case RING_CLASS:
                case TOOL_CLASS:
                    dindx = 8;
                    break;
                case WAND_CLASS:
                    dindx = 9;
                    break;
                default:
                    skip++;
                    break;
                }
            } else
                skip++;
            break;
        default:
            skip++;
            break;
        }
        if (!skip) {
            for (i = cnt = 0L; i < quan; i++)
                if (!rn2(3))
                    cnt++;

            if (!cnt)
                continue;
            if (vis)
                pline("%s%s %s!",
                      (cnt == obj->quan) ? "" : (cnt > 1L) ? "Some of "
                                                           : "One of ",
                      (cnt == obj->quan) ? Yname2(obj) : yname(obj),
                      destroy_strings[dindx][(cnt > 1L)]);
            for (i = 0; i < cnt; i++)
                m_useup(mtmp, obj);
        }
    }
    return tmp;
}

int
resist(mtmp, oclass, damage, tell)
struct monst *mtmp;
char oclass;
int damage, tell;
{
    int resisted;
    int alev, dlev;

    /* fake players always pass resistance test against Conflict
       (this doesn't guarantee that they're never affected by it) */
    if (oclass == RING_CLASS && !damage && !tell
        && is_mplayer(mtmp->data))
        return 1;

    /* attack level */
    switch (oclass) {
    case WAND_CLASS:
        alev = 12;
        break;
    case TOOL_CLASS:
        alev = 10;
        break; /* instrument */
    case WEAPON_CLASS:
        alev = 10;
        break; /* artifact */
    case SCROLL_CLASS:
        alev = 9;
        break;
    case POTION_CLASS:
        alev = 6;
        break;
    case RING_CLASS:
        alev = 5;
        break;
    default:
        alev = u.ulevel;
        break; /* spell */
    }
    /* defense level */
    dlev = (int) mtmp->m_lev;
    if (dlev > 50)
        dlev = 50;
    else if (dlev < 1)
        dlev = is_mplayer(mtmp->data) ? u.ulevel : 1;

    resisted = rn2(100 + alev - dlev) < mtmp->data->mr;
    if (resisted) {
        if (tell) {
            shieldeff(mtmp->mx, mtmp->my);
            pline("%s resists!", Monnam(mtmp));
        }
        damage = (damage + 1) / 2;
    }

    if (damage) {
        int saved_mhp = mtmp->mhp;

        damage_mon(mtmp, damage, AD_RBRE, m_using ? FALSE : TRUE);
        if (DEADMONSTER(mtmp)) {
            if (m_using)
                monkilled(mtmp, "", AD_RBRE);
            else
                killed(mtmp);
        } else {
            print_mon_wounded(mtmp, saved_mhp);
        }
    }
    return resisted;
}

#define MAXWISHTRY 5

STATIC_OVL void
wishcmdassist(triesleft)
int triesleft;
{
    static NEARDATA const char *
        wishinfo[] = {
  "Wish details:",
  "",
  "Enter the name of an object, such as \"potion of monster detection\",",
  "\"scroll labeled README\", \"elven mithril-coat\", or \"Grimtooth\"",
  "(without the quotes).",
  "",
  "For object types which come in stacks, you may specify a plural name",
  "such as \"potions of healing\", or specify a count, such as \"1000 gold",
  "pieces\", although that aspect of your wish might not be granted.",
  "",
  "You may also specify various prefix values which might be used to",
  "modify the item, such as \"uncursed\" or \"rustproof\" or \"+1\".",
  "Most modifiers shown when viewing your inventory can be specified.",
  "",
  "You may specify 'nothing' to explicitly decline this wish.",
  0,
    },
        preserve_wishless[] = "Doing so will preserve 'wishless' conduct.",
        retry_info[] =
                    "If you specify an unrecognized object name %s%s time%s,",
        retry_too[] = "a randomly chosen item will be granted.",
        suppress_cmdassist[] =
            "(Suppress this assistance with !cmdassist in your config file.)",
        *cardinals[] = { "zero",  "one",  "two", "three", "four", "five" },
        too_many[] = "too many";
    int i;
    winid win;
    char buf[BUFSZ];

    win = create_nhwindow(NHW_TEXT);
    if (!win)
        return;
    for (i = 0; i < SIZE(wishinfo) - 1; ++i)
        putstr(win, 0, wishinfo[i]);
    if (!u.uconduct.wishes)
        putstr(win, 0, preserve_wishless);
    putstr(win, 0, "");
    Sprintf(buf, retry_info,
            (triesleft >= 0 && triesleft < SIZE(cardinals))
               ? cardinals[triesleft]
               : too_many,
            (triesleft < MAXWISHTRY) ? " more" : "",
            plur(triesleft));
    putstr(win, 0, buf);
    putstr(win, 0, retry_too);
    putstr(win, 0, "");
    if (iflags.cmdassist)
        putstr(win, 0, suppress_cmdassist);
    display_nhwindow(win, FALSE);
    destroy_nhwindow(win);
}

void
makewish(eight_ball)
boolean eight_ball;
{
    char buf[BUFSZ] = DUMMY;
    char promptbuf[BUFSZ];
    char bufcpy[BUFSZ];
    struct obj *otmp, nothing;
    int tries = 0;
    int prev_artwish = u.uconduct.wisharti;

    promptbuf[0] = '\0';
    nothing = zeroobj; /* lint suppression; only its address matters */
    if (flags.verbose)
        You("may wish for an object.");
 retry:
    Strcpy(promptbuf, "For what do you wish");
    if (iflags.cmdassist && tries > 0)
        Strcat(promptbuf, " (enter 'help' for assistance)");
    tries++;
    Strcat(promptbuf, "?");
    getlin(promptbuf, buf);
    (void) mungspaces(buf);
    if (buf[0] == '\033') {
        buf[0] = '\0';
    } else if (!strcmpi(buf, "help")) {
        wishcmdassist(MAXWISHTRY - tries);
        buf[0] = '\0'; /* for EDIT_GETLIN */
        goto retry;
    }
    if (buf[0] == '\0') {
        if (wizard
            || yn("Really forfeit this wish?") == 'y' || tries >= 50) {
            Strcpy(buf, "nothing");
        }
#ifdef HANGUPHANDLING
        else if (program_state.done_hup) {
            Strcpy(buf, "nothing");
        }
#endif
        else {
            goto retry;
        }
    }
    /*
     *  Note: if they wished for and got a non-object successfully,
     *  otmp == &zeroobj.  That includes gold, or an artifact that
     *  has been denied.  Wishing for "nothing" requires a separate
     *  value to remain distinct.
     */
    strcpy(bufcpy, buf);
    otmp = readobjnam(buf, &nothing);
    if (!otmp && iflags.debug_fuzzer) {
        /* allow the fuzzer, and only the fuzzer, to get a random object from a
         * random input string that corresponds to nothing. */
        buf[0] = '\0';
        otmp = readobjnam(buf, &nothing);
        if (!otmp)
            return; /* for safety; should never happen */
    } else if (!otmp) {
        pline("Nothing fitting that description exists in the game.");
        if (++tries < MAXWISHTRY)
            goto retry;
        pline1(thats_enough_tries);
        otmp = readobjnam((char *) 0, (struct obj *) 0);
        if (!otmp)
            return; /* for safety; should never happen */
    } else if (otmp == &nothing) {
        /* explicitly wished for "nothing", presumably attempting
           to retain wishless conduct */
        return;
    }

    /* KMH, conduct */
    if (!u.uconduct.wishes++) {
        if (eight_ball)
            livelog_printf(LL_CONDUCT | LL_WISH | (prev_artwish < u.uconduct.wisharti ? LL_ARTIFACT : 0),
                           "made %s first wish - \"%s\" using the Magic 8-Ball", uhis(), bufcpy);
        else
            livelog_printf(LL_CONDUCT | LL_WISH | (prev_artwish < u.uconduct.wisharti ? LL_ARTIFACT : 0),
                           "made %s first wish - \"%s\"", uhis(), bufcpy);
    } else if (!prev_artwish && u.uconduct.wisharti) {
        /* arti conduct handled in readobjnam() above */
        if (eight_ball)
            livelog_printf(LL_CONDUCT | LL_WISH | LL_ARTIFACT,
                           "made %s first artifact wish - \"%s\" using the Magic 8-ball", uhis(), bufcpy);
        else
            livelog_printf(LL_CONDUCT | LL_WISH | LL_ARTIFACT,
                           "made %s first artifact wish - \"%s\"", uhis(), bufcpy);
    } else {
        if (eight_ball)
            livelog_printf(LL_WISH | (prev_artwish < u.uconduct.wisharti ? LL_ARTIFACT : 0),
                           "wished for \"%s\" using the Magic 8-ball", bufcpy);
        else
            livelog_printf(LL_WISH | (prev_artwish < u.uconduct.wisharti ? LL_ARTIFACT : 0),
                           "wished for \"%s\"", bufcpy);
    }

    if (otmp != &zeroobj) {
        const char
            *verb = ((Is_airlevel(&u.uz) || u.uinwater) ? "slip" : "drop"),
            *oops_msg = (u.uswallow
                         ? "Oops!  %s out of your reach!"
                         : (Is_airlevel(&u.uz) || Is_waterlevel(&u.uz)
                            || levl[u.ux][u.uy].typ < IRONBARS
                            || levl[u.ux][u.uy].typ >= ICE)
                            ? "Oops!  %s away from you!"
                            : "Oops!  %s to the floor!");

#ifdef WISH_TRACKER
        /* write it out to our universal wishtracker file */
        trackwish(bufcpy);
#endif
        /* The(aobjnam()) is safe since otmp is unidentified -dlc */
        (void) hold_another_object(otmp, oops_msg,
                                   The(aobjnam(otmp, verb)),
                                   (const char *) 0);
        u.ublesscnt += rn1(100, 50); /* the gods take notice */
    }
}

/* uses the M_SEEN definitions in monst.h */
void
monstseesu(udid)
unsigned long udid;
{
    struct monst* mtmp;

    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (!DEADMONSTER(mtmp) && m_canseeu(mtmp))
            m_setseen(mtmp, udid);
    }
}

void
blindingflash()
{
    struct monst* mtmp;
    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        /* if it can't see the flash, don't bother */
        if (DEADMONSTER(mtmp) || mtmp->msleeping
            || !haseyes(mtmp->data) || !mtmp->mcansee
            || mtmp->mblinded || mtmp == &youmonst)
            continue;

        /* must be able to see our location... */
        if (m_cansee(mtmp, u.ux, u.uy)
            && distu(mtmp->mx, mtmp->my) <= 5) {
            if (!Blind && canseemon(mtmp))
                pline("%s is blinded by the flash!", Monnam(mtmp));
            if (mtmp->mpeaceful && !mtmp->mtame && !rn2(3))
                setmangry(mtmp, TRUE);
            mtmp->mblinded = rnd(hates_light(r_data(mtmp)) ? 40 : 20);
            mtmp->mcansee = 0;
        }
    }
}

/*zap.c*/
