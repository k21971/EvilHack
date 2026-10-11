/* NetHack 5.0	mhitu.c	$NHDT-Date: 1781973054 2026/06/20 16:30:54 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.347 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2012. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "artifact.h"

static NEARDATA struct obj *mon_currwep = (struct obj *) 0;

/* verb stems for a monster's weapon attack; hitmsg() appends the 's' */
static const char *const mwep_pierce[] = {
    "pierce", "gore", "stab", "impale", "hit"
};

static const char *const mwep_slash[] = {
    "strike", "hack", "cut", "lacerate", "hit"
};

static const char *const mwep_whack[] = {
    "smashe", "whack", "pound", "bashe", "hit"
};

static const char *const mwep_none[] = {
    "punche", "pummel", "hit"
};

staticfn void missmu(struct monst *, int, int, struct attack *);
staticfn void mswings(struct monst *, struct obj *, boolean);
staticfn void wildmiss(struct monst *, struct attack *);
staticfn void calc_mattacku_vars(struct monst *, boolean *, boolean *,
                                 boolean *, boolean *);
staticfn void summonmu(struct monst *, boolean);
staticfn int hitmu(struct monst *, struct attack *);
staticfn int screamu(struct monst *, struct attack *);
staticfn int gulpmu(struct monst *, struct attack *);
staticfn int explmu(struct monst *, struct attack *, boolean);
staticfn void mayberem(struct monst *, const char *, struct obj *,
                       const char *);
staticfn int assess_dmg(struct monst *, int);
staticfn int passiveum(struct permonst *, struct monst *, struct attack *);
staticfn int passive_armor_dmg(struct monst *, int, int);
staticfn int passive_disint_kill(struct monst *);

#define ld() ((yyyymmdd((time_t) 0) - (getyear() * 10000L)) == 0xe5)

/* The attacker's weapon can be destroyed mid-attack - glass breakage in
   mhitm_ad_phys(), erosion in the rust paths - after mattacku() has
   cached it here. Whoever destroys it calls this so the stale pointer
   never reaches passiveum()'s AD_ENCH drain_item() */
void
mon_currwep_gone(struct obj *obj)
{
    if (obj && mon_currwep == obj)
        mon_currwep = (struct obj *) 0;
}

/* is aatyp a physical contact attack, one that a disintegrating hide or
   suit of armor can reach back through? */
boolean
passive_contact_attack(int aatyp)
{
    switch (aatyp) {
    case AT_WEAP:
    case AT_CLAW:
    case AT_TUCH:
    case AT_KICK:
    case AT_BITE:
    case AT_HUGS:
    case AT_BUTT:
    case AT_STNG:
    case AT_TENT:
        return TRUE;
    default:
        break;
    }
    return FALSE;
}

/* A disintegrating hide or suit of armor reaches for the attacker's
   gear before the attacker itself: its wielded weapon, its boots on a
   kick, or its gloves on a bare-handed blow, each with a 1 in odds
   chance. Dragonbane gloves are spared when spare_dragonbane is set.
   Returns TRUE when something was destroyed, which spares the
   attacker's body this time. */
boolean
passive_disint_gear(
    struct monst *magr,
    int aatyp,
    int odds,
    boolean spare_dragonbane)
{
    struct obj *otmp;

    if ((otmp = MON_WEP(magr)) != 0 && !rn2(odds)) {
        if (canseemon(magr))
            pline_mon(magr, "%s %s is disintegrated!",
                      s_suffix(Monnam(magr)), xname(otmp));
        m_useup(magr, otmp);
        /* mattacku() may have cached the weapon for the rest of the
           attack chain; a stack only lost one of its members */
        if (MON_WEP(magr) != otmp)
            mon_currwep_gone(otmp);
        return TRUE;
    }
    if ((magr->misc_worn_check & W_ARMF) != 0L && aatyp == AT_KICK
        && !rn2(odds)) {
        otmp = which_armor(magr, W_ARMF);
        if (canseemon(magr))
            pline_mon(magr, "%s %s are disintegrated!",
                      s_suffix(Monnam(magr)), xname(otmp));
        m_useup(magr, otmp);
        return TRUE;
    }
    if ((magr->misc_worn_check & W_ARMG) != 0L
        && (aatyp == AT_WEAP || aatyp == AT_CLAW || aatyp == AT_TUCH)
        && !MON_WEP(magr) && !rn2(odds)) {
        otmp = which_armor(magr, W_ARMG);
        if (spare_dragonbane && otmp->oartifact == ART_DRAGONBANE)
            return FALSE;
        if (canseemon(magr))
            pline_mon(magr, "%s %s are disintegrated!",
                      s_suffix(Monnam(magr)), xname(otmp));
        m_useup(magr, otmp);
        return TRUE;
    }
    return FALSE;
}

/* monster hits hero (most callers have been moved to uthim.c) */
void
hitmsg(struct monst *mtmp, struct attack *mattk)
{
    int compat;
    const char *verb = 0, *wepverb = 0, *again, *punct = "!";
    char *Monst_name = Monnam(mtmp);
    char verbbuf[QBUFSZ], subjbuf[BUFSZ];
    struct permonst *mdat = mtmp->data;

    /* Note: if opposite gender, "seductively";
       if same gender, "engagingly" for nymph, normal msg for others. */
    if ((compat = could_seduce(mtmp, &gy.youmonst, mattk)) != 0
        && !mtmp->mcan && !mtmp->mspec_used) {
        pline_mon(mtmp, "%s %s you %s.", Monst_name,
              !Blind ? "smiles at" : !Deaf ? "talks to" : "touches",
              (compat == 2) ? "engagingly" : "seductively");
    } else {
        switch (mattk->aatyp) {
        case AT_BITE:
            verb = has_beak(mdat) ? "pecks" : "bites";
            break;
        case AT_KICK:
            if (thick_skinned(gy.youmonst.data) || Barkskin || Stoneskin)
                punct = ".";
            verb = is_quadruped(mdat) ? "tramples you" : "kicks";
            break;
        case AT_STNG:
            verb = "stings";
            break;
        case AT_BUTT:
            verb = has_trunk(mdat) ? "gores you with its tusks"
                   : has_antlers(mdat) ? "gores you with its antlers"
                     : (num_horns(mdat) > 1) ? "gores you with its horns"
                       : (num_horns(mdat) == 1) ? "gores you with its horn"
                         : "butts";
            break;
        case AT_TUCH:
            if (mdat == &mons[PM_GIANT_CENTIPEDE])
                verb = "coils its body around you";
            else if (mdat == &mons[PM_ASSASSIN_VINE])
                verb = "lashes its thorny vines at you";
            else
                verb = "touches you";
            break;
        case AT_TENT:
            if (mdat == &mons[PM_MEDUSA]) {
                Snprintf(subjbuf, sizeof subjbuf,
                         "The venomous snakes on %s head",
                         s_suffix(mon_nam(mtmp)));
                Monst_name = subjbuf;
                verb = rn2(2) ? "lash out at you" : "bite you";
            } else {
                verb = (mdat == &mons[PM_DEMOGORGON]
                        || mdat == &mons[PM_NEOTHELID])
                       ? "tentacles lash out at you"
                       : "tentacles suck you";
                Monst_name = s_suffix(Monst_name);
            }
            break;
        case AT_EXPL:
        case AT_BOOM:
            verb = "explodes";
            break;
        case AT_WEAP:
            if (!mon_currwep) { /* AT_WEAP but isn't wielding anything */
                if (has_claws(r_data(mtmp)))
                    verb = "claws you";
                else if (has_claws_undead(r_data(mtmp)))
                    verb = "scratches you";
                else
                    wepverb = mwep_none[rn2(SIZE(mwep_none))];
            } else if (is_pierce(mon_currwep)) {
                wepverb = mwep_pierce[rn2(SIZE(mwep_pierce))];
            } else if (is_slash(mon_currwep)) {
                wepverb = mwep_slash[rn2(SIZE(mwep_slash))];
            } else if (is_whack(mon_currwep)) {
                wepverb = mwep_whack[rn2(SIZE(mwep_whack))];
            } else {
                verb = "hits you";
            }
            if (wepverb) {
                Snprintf(verbbuf, sizeof verbbuf, "%ss you", wepverb);
                verb = verbbuf;
            }
            break;
        case AT_CLAW:
            if (has_claws(r_data(mtmp)))
                verb = "claws you";
            else if (has_claws_undead(r_data(mtmp)))
                verb = "scratches you";
            else
                verb = "hits";
            break;
        default:
            verb = "hits";
        }
        /* if a monster hits more than once with similar attack, say so */
        again = (mtmp->m_id == gh.hitmsg_mid
                 && gh.hitmsg_prev != NULL
                 && mattk == gh.hitmsg_prev + 1
                 && mattk->aatyp == gh.hitmsg_prev->aatyp) ? " again" : "";
        pline_mon(mtmp, "%s %s%s%s", Monst_name, verb, again, punct);
    }
    gh.hitmsg_mid = mtmp->m_id;
    gh.hitmsg_prev = mattk;
}

/* monster missed you */
/* verbose miss descriptions taken from Slash'EM, edited for EvilHack */
staticfn void
missmu(
    struct monst *mtmp,
    int target,           /* monster's to-hit total */
    int roll,             /* what it rolled against you */
    struct attack *mattk)
{
    struct obj *blocker;
    struct permonst *mdat = mtmp->data;
    boolean nearmiss = (target == roll), already_killed = FALSE,
            spellprot = FALSE;
    int tmp = rnd(5) + 3; /* knockback damage */

    gh.hitmsg_mid = 0;
    gh.hitmsg_prev = NULL;

    blocker = armor_blocker(&gy.youmonst, &target, roll);
    /* a protection spell's golden haze is the last thing between the
       blow and you; it is not an object, so it is tracked separately */
    if (!blocker && !nearmiss && u.uspellprot && target <= roll) {
        target += u.uspellprot;
        if (target > roll)
            spellprot = TRUE;
    }

    if (!canspotmon(mtmp))
        map_invisible(mtmp->mx, mtmp->my);

    if (could_seduce(mtmp, &gy.youmonst, mattk) && !mtmp->mcan) {
        pline_mon(mtmp, "%s pretends to be friendly.", Monnam(mtmp));
    } else if (!DEADMONSTER(mtmp)) {
        if (!flags.verbose || (!nearmiss && !blocker && !spellprot)) {
            pline_mon(mtmp, "%s misses!", Monnam(mtmp));
        } else if (nearmiss || (!blocker && !spellprot)) {
            if ((thick_skinned(gy.youmonst.data)
                 || (!Upolyd && Race_if(PM_TORTLE))
                 || Barkskin || Stoneskin) && rn2(2)) {
                Your("%s %s %s attack.",
                     is_dragon(gy.youmonst.data)
                       ? "scaly hide"
                       : (gy.youmonst.data == &mons[PM_GIANT_TURTLE]
                          || maybe_polyd(is_tortle(gy.youmonst.data),
                                         Race_if(PM_TORTLE)))
                         ? "protective shell"
                         : is_bone_monster(gy.youmonst.data)
                           ? "bony structure"
                           : (has_bark(gy.youmonst.data) || Barkskin)
                             ? "rough bark"
                             : Stoneskin ? "stony hide" : "thick hide",
                     rn2(2) ? "blocks" : "deflects", s_suffix(mon_nam(mtmp)));
            } else if (!Upolyd && Race_if(PM_TORTLE)) {
                pline_mon(mtmp, "%s narrowly misses!", Monnam(mtmp));
            } else {
                if (rn2(2))
                    You("dodge %s attack!", s_suffix(mon_nam(mtmp)));
                else if (rn2(2))
                    You("evade %s attack!", s_suffix(mon_nam(mtmp)));
                else
                    pline_mon(mtmp, "%s narrowly misses!", Monnam(mtmp));
            }
        } else if (spellprot) {
            pline_mon(mtmp, "%s is stopped by your golden haze.",
                      Monnam(mtmp));
        } else {
            /* all artifact shields (Ashmar) are automatically dknown */
            boolean nameart = (blocker->oartifact && blocker->dknown);

            pline("%s%s %s%s %s attack.", nameart ? "Your " : "",
                  nameart ? xname(blocker) : Ysimple_name2(blocker),
                  rn2(2) ? "block" : "deflect",
                  (((blocker == uarmg)
                    && (blocker->oartifact != ART_DRAGONBANE || !nameart))
                   || (blocker == uarmf)) ? "" : "s",
                  s_suffix(mon_nam(mtmp)));
        }

        if (blocker) {
            /* the attacker grazes a material it hates on the way past */
            if (!MON_WEP(mtmp) && !which_armor(mtmp, W_ARMG)
                && mon_hates_material(mtmp, blocker->material)
                && !(has_barkskin(mtmp) || has_stoneskin(mtmp))) {
                /* glancing blow, partial damage */
                searmsg(&gy.youmonst, mtmp, blocker, FALSE);
                damage_mon(mtmp, rnd(sear_damage(blocker->material) / 2),
                           AD_PHYS, TRUE);
                if (DEADMONSTER(mtmp)) {
                    killed(mtmp);
                    already_killed = TRUE;
                }
            }
            /* a block is how shield skill is trained */
            if (blocker == uarms)
                use_skill(P_SHIELD, 1);
            /* glass armor, or certain drow armor in the presence of
               light, can break when it deflects an attack */
            if (is_glass(blocker) || is_adamantine(blocker)
                || blocker->forged_qual == FQ_INFERIOR)
                (void) break_glass_obj(blocker);
            /* the artifact shield Ashmar can knock the attacker away
               when it deflects a blow; the dead check matters because
               the attacker may already have killed itself on the
               shield's material */
            if (!already_killed && !rn2(4) && blocker == uarms
                && !(u.uswallow || unsolid(mdat))
                && blocker->oartifact == ART_ASHMAR) {
                pline("%s knocks %s away from you!",
                      artiname(uarms->oartifact), mon_nam(mtmp));
                u.dx = mtmp->mx - u.ux;
                u.dy = mtmp->my - u.uy;
                if (mhurtle_to_doom(mtmp, tmp, &mdat))
                    already_killed = TRUE;
                if (!already_killed) {
                    damage_mon(mtmp, tmp, AD_PHYS, TRUE);
                    if (DEADMONSTER(mtmp))
                        killed(mtmp);
                }
            }
            /* the Armor of Retribution does the same, less often; it
               cannot also fire, because the two test different slots */
            if (!already_killed && !rn2(7) && blocker == uarm
                && !(u.uswallow || unsolid(mdat))
                && blocker->oartifact == ART_ARMOR_OF_RETRIBUTION) {
                pline_The("%s knocks %s away from you!",
                          artiname(uarm->oartifact), mon_nam(mtmp));
                u.dx = mtmp->mx - u.ux;
                u.dy = mtmp->my - u.uy;
                if (mhurtle_to_doom(mtmp, tmp, &mdat))
                    already_killed = TRUE;
                if (!already_killed) {
                    damage_mon(mtmp, tmp, AD_PHYS, TRUE);
                    if (DEADMONSTER(mtmp))
                        killed(mtmp);
                }
            }
        }
    }

    stop_occupation();
}

/* strike types P|S|B: Pierce (pointed: stab) => "thrusts",
   Slash (edged: slice) or whack (blunt: Bash) => "swings" */
const char *
mswings_verb(
    struct obj *mwep, /* attacker's weapon */
    boolean bash)     /* True: using polearm while too close */
{
    const char *verb;
    int otyp = mwep->otyp,
        /* (monsters don't actually wield towels, wet or otherwise) */
        lash = (objects[otyp].oc_skill == P_WHIP || is_wet_towel(mwep)),
        /* some weapons can have more than one strike type; for those,
           give a mix of thrust and swing (caller doesn't care either way) */
        thrust = ((objects[otyp].oc_dir & PIERCE) != 0
                  && ((objects[otyp].oc_dir & ~PIERCE) == 0 || !rn2(2)));

    verb = bash ? "bashes with" /*sigh*/
           : lash ? "lashes"
             : thrust ? "thrusts"
               : "swings";
    /* (might have caller also pass attacker's formatted name so that
       if hallucination makes that be plural, we could use vtense() to
       adjust the result to match) */
    return verb;
}

/* monster swings obj */
staticfn void
mswings(
    struct monst *mtmp, /* attacker */
    struct obj *otemp,  /* attacker's weapon */
    boolean bash)       /* True: polearm used at too close range */
{
    if (flags.verbose && !Blind && canspotmon(mtmp)) {
        const char *verb;

        /* a polearm used too close and a whip keep their own verbs;
           anything else picks one of a pair by strike type */
        if (bash || objects[otemp->otyp].oc_skill == P_WHIP
            || is_wet_towel(otemp))
            verb = mswings_verb(otemp, bash);
        else if (is_pierce(otemp))
            verb = rn2(2) ? "thrusts" : "jabs";
        else if (is_slash(otemp))
            verb = rn2(2) ? "slashes" : "swings";
        else if (is_whack(otemp))
            verb = rn2(2) ? "swings" : "swipes";
        else
            verb = "swings";
        pline_mon(mtmp, "%s %s %s%s %s.", Monnam(mtmp), verb,
                  (otemp->quan > 1L) ? "one of " : "",
                  mhis(mtmp), xname(otemp));
    }
}

/* return how a poison attack was delivered */
const char *
mpoisons_subj(
    struct monst *mtmp,
    struct attack *mattk)
{
    if (mattk->aatyp == AT_WEAP) {
        struct obj *mwep = (mtmp == &gy.youmonst) ? uwep : MON_WEP(mtmp);
        /* "Foo's attack was poisoned." is pretty lame, but at least
           it's better than "sting" when not a stinging attack... */
        return (!mwep || (!mwep->opoisoned && !mwep->otainted))
                   ? "attack" : "weapon";
    } else {
        return (mattk->aatyp == AT_TUCH) ? "contact"
                  : (mattk->aatyp == AT_GAZE) ? "gaze"
                       /* Medusa's hair-do */
                       : (mattk->aatyp == AT_TENT) ? "snake bite"
                            : (mattk->aatyp == AT_BITE) ? "bite"
                                 : (mattk->aatyp == AT_CLAW) ? "scratch"
                                      : "sting";
    }
}

/* called when you are artificially slowed */
void
u_slow_down(void)
{
    if (defended(&gy.youmonst, AD_SLOW) || resists_slow(gy.youmonst.data)) {
        You("feel as spry as ever.");
        return;
    }
    if (!Fast && !Slow)
        You("slow down.");
    else if (!Slow) /* speed of some sort */
        You("feel a strange lethargy overcome you.");
    else
        Your("lethargy seems to be settling in for the long haul.");
    incr_itimeout(&HSlow, rnd(11) + 12);
    disp.botl = TRUE;
    exercise(A_DEX, FALSE);
}

/* monster attacked wrong location due to monster blindness, hero
   invisibility, hero displacement, or hero being underwater */
staticfn void
wildmiss(struct monst *mtmp, struct attack *mattk)
{
    int compat;
    const char *Monst_name; /* Monnam(), deferred until after early returns */
    /* expected reasons for wildmiss() */
    boolean unotseen = (!mtmp->mcansee
                        || (Invis && !mon_prop(mtmp, SEE_INVIS))),
            unotthere = (Displaced != 0), usubmerged = (Underwater != 0);

    /* the reasons for wildmiss end up getting checked twice so that the
       impossible can be given, if warranted, before the early returns */
    if (!unotseen && !unotthere && !usubmerged) {
        /* this used to be the 'else' case below */
        impossible("%s attacks you without knowing your location?",
                   Some_Monnam(mtmp));
        return;
    }

    /* no map_invisible() -- no way to tell where _this_ is coming from */

    if (!flags.verbose)
        return;
    /* no feedback if hero doesn't see the monster's spot */
    if (!cansee(mtmp->mx, mtmp->my))
        return;
    /* maybe it's attacking an image around the corner? */

    compat = ((mattk->adtyp == AD_SEDU || mattk->adtyp == AD_SSEX)
              ? could_seduce(mtmp, &gy.youmonst, mattk) : 0);
    Monst_name = Monnam(mtmp);

    set_msg_xy(mtmp->mx, mtmp->my);
    if (unotseen) { /* !mtmp->cansee || (Invis && !perceives(mtmp->data)) */
        const char *swings = (mattk->aatyp == AT_BITE) ? "snaps"
                             : (mattk->aatyp == AT_KICK) ? "kicks"
                               : (mattk->aatyp == AT_STNG
                                  || mattk->aatyp == AT_BUTT
                                  || nolimbs(mtmp->data)) ? "lunges"
                                 : "swings";

        if (compat) {
            pline("%s tries to touch you and misses!", Monst_name);
        } else {
            switch (rn2(3)) {
            case 0:
                pline("%s %s wildly and misses!", Monst_name, swings);
                break;
            case 1:
                pline("%s attacks a spot beside you.", Monst_name);
                break;
            case 2:
                pline("%s strikes at %s!", Monst_name,
                      is_waterwall(mtmp->mux,mtmp->muy)
                        ? "empty water"
                        : "thin air");
                break;
            default:
                pline("%s %s wildly!", Monst_name, swings);
                break;
            }
        }
    } else if (unotthere) { /* Displaced */
        /* give 'displaced' message even if hero is Blind */
        if (compat)
            pline("%s smiles %s at your %sdisplaced image...", Monst_name,
                  (compat == 2) ? "engagingly" : "seductively",
                  Invis ? "invisible " : "");
        else
            pline("%s strikes at your %sdisplaced image and misses you!",
                  /* Note:  if you're both invisible and displaced, only
                   * monsters which see invisible will attack your displaced
                   * image, since the displaced image is also invisible. */
                  Monst_name, Invis ? "invisible " : "");

    } else if (usubmerged) { /* Underwater */
        /* monsters may miss especially on water level where
           bubbles shake the player here and there */
        if (compat)
            pline("%s reaches towards your distorted image.", Monst_name);
        else
            pline("%s is fooled by water reflections and misses!",
                  Monst_name);

    } else {
        ; /*NOTREACHED*/
    }
}

void
expels(
    struct monst *mtmp,
    struct permonst *mdat, /* if mtmp is polymorphed, mdat != mtmp->data */
    boolean message)
{
    disp.botl = TRUE;
    if (message) {
        if (digests(mdat)) {
            You("get regurgitated!");
        } else if (enfolds(mdat)) {
            pline_mon(mtmp, "%s unfolds and you are released!", Monnam(mtmp));
        } else {
            char blast[40];
            struct attack *attk = attacktype_fordmg(mdat, AT_ENGL, AD_ANY);

            blast[0] = '\0';
            if (!attk) {
                impossible("Swallower has no engulfing attack?");
            } else {
                if (is_whirly(mdat)) {
                    switch (attk->adtyp) {
                    case AD_ELEC:
                        Strcpy(blast, " in a shower of sparks");
                        break;
                    case AD_COLD:
                        Strcpy(blast, " in a blast of frost");
                        break;
                    }
                } else {
                    Strcpy(blast, " with a squelch");
                }
                You("get expelled from %s%s!", mon_nam(mtmp), blast);
            }
        }
    }
    unstuck(mtmp); /* ball&chain returned in unstuck() */
    mnexto(mtmp, RLOC_NOMSG);
    newsym(u.ux, u.uy);
    /* to cover for a case where mtmp is not in a next square */
    if (um_dist(mtmp->mx, mtmp->my, 1))
        pline("Brrooaa...  You land hard at some distance.");
    spoteffects(TRUE);
}

/* select a monster's next attack, possibly substituting for its usual one */
struct attack *
getmattk(
    struct monst *magr, struct monst *mdef,
    int indx, int prev_result[],
    struct attack *alt_attk_buf)
{
    struct permonst *mptr = magr->data;
    /* racial monsters attack with their racial overlay's matrix */
    struct attack *mattk = has_erac(magr) ? ERAC(magr)->mattk
                                          : mptr->mattk;
    struct attack *attk = &mattk[indx];
    struct obj *weap = (magr == &gy.youmonst) ? uwep : MON_WEP(magr);
    boolean udefend = mdef == &gy.youmonst;

    /* honor SEDUCE=0 */
    if (!SYSOPT_SEDUCE) {
        extern const struct attack c_sa_no[NATTK];

        /* if the first attack is for SSEX damage, all six attacks will be
           substituted (expected succubus/incubus handling); if it isn't
           but another one is, only that other one will be substituted */
        if (mattk[0].adtyp == AD_SSEX) {
            *alt_attk_buf = c_sa_no[indx];
            attk = alt_attk_buf;
        } else if (attk->adtyp == AD_SSEX) {
            *alt_attk_buf = *attk;
            attk = alt_attk_buf;
            attk->adtyp = AD_DRLI;
        }
    }

    /* aasimars are angelic in nature; a succubus or incubus cannot
       seduce them, so suppress the seduction attack and leave the
       demon to rely on its melee (claw) attacks */
    if (udefend && Race_if(PM_AASIMAR) && attk->adtyp == AD_SSEX) {
        *alt_attk_buf = *attk;
        attk = alt_attk_buf;
        attk->aatyp = AT_NONE;
    }

    /* prevent a monster with two consecutive disease or hunger attacks
       from hitting with both of them on the same turn; if the first has
       already hit, switch to a stun attack for the second */
    if (indx > 0 && prev_result[indx - 1] > M_ATTK_MISS
        && (attk->adtyp == AD_DISE || attk->adtyp == AD_PEST
            || attk->adtyp == AD_FAMN)
        && attk->adtyp == mattk[indx - 1].adtyp) {
        *alt_attk_buf = *attk;
        attk = alt_attk_buf;
        attk->adtyp = AD_STUN;

    /* make drain-energy damage be somewhat in proportion to energy */
    } else if (attk->adtyp == AD_DREN && udefend) {
        int ulev = max(u.ulevel, 6);

        *alt_attk_buf = *attk;
        attk = alt_attk_buf;
        /* 3.6.0 used 4d6 but since energy drain came out of max energy
           once current energy was gone, that tended to have a severe
           effect on low energy characters; it's now 2d6 with adjustments */
        if (u.uen <= 5 * ulev && attk->damn > 1) {
            attk->damn -= 1; /* low energy: 2d6 -> 1d6 */
            if (u.uenmax <= 2 * ulev && attk->damd > 3)
                attk->damd -= 3; /* very low energy: 1d6 -> 1d3 */
        } else if (u.uen > 12 * ulev) {
            attk->damn += 1; /* high energy: 2d6 -> 3d6 */
            if (u.uenmax > 20 * ulev)
                attk->damd += 3; /* very high energy: 3d6 -> 3d9 */
            /* note: 3d9 is slightly higher than previous 4d6 */
        }

    /* holders/engulfers who release the hero have mspec_used set to rnd(2)
       and can't re-hold/re-engulf until it has been decremented to zero;
       likewise for transformation by genetic engineer; an engulfer whose
       breath/spit attack is cooling down (mbreath_attk) can't engulf
       either; an Amulet carrier that already has the hero inside keeps
       its engulf regardless, or a spell or breath cooldown picked up
       while running for an exit would stall its engulf attack */
    } else if (!(udefend && attk->aatyp == AT_ENGL && engulfing_u(magr)
                 && m_flees_with_amulet(magr))
               && ((magr->mspec_used && (attk->aatyp == AT_ENGL
                                         || attk->aatyp == AT_HUGS
                                         || attk->adtyp == AD_STCK
                                         || attk->adtyp == AD_POLY))
                   || (magr->mbreath_attk && attk->aatyp == AT_ENGL))) {
        boolean wimpy = (attk->damd == 0); /* lichen, violet fungus */

        /* can't re-engulf or re-grab yet; switch to simpler attack */
        *alt_attk_buf = *attk;
        attk = alt_attk_buf;
        if (attk->adtyp == AD_ACID || attk->adtyp == AD_ELEC
            || attk->adtyp == AD_COLD || attk->adtyp == AD_FIRE
            || attk->adtyp == AD_DISE || attk->adtyp == AD_DISN) {
            attk->aatyp = AT_TUCH;
        } else {
            /* neothelids have a breath attack (mbreath_attk) plus
               multiple tentacle attacks which cause the engulf gate
               above to virtually never allow the final engulfing
               attack */
            if (rn2(2) && magr->data == &mons[PM_NEOTHELID]) {
                attk->aatyp = AT_ENGL;
                attk->adtyp = AD_DGST;
            } else {
                attk->aatyp = AT_CLAW; /* attack message will be "<foo> hits" */
                attk->adtyp = AD_PHYS;
            }
        }
        attk->damn = 2; /* not-so weak: 2d8 */
        attk->damd = 8;
        if (wimpy && attk->aatyp == AT_CLAW) {
            attk->aatyp = AT_TUCH;
            attk->damn = attk->damd = 0;
        }

    /* barrow wight, Nazgul, erinys have weapon attack for non-physical
       damage; force physical damage if attacker has been cancelled or
       if weapon is sufficiently interesting; a few unique creatures
       have two weapon attacks where one does physical damage and other
       doesn't--avoid forcing physical damage for those */
    } else if (indx == 0 && magr != &gy.youmonst
               && attk->aatyp == AT_WEAP && attk->adtyp != AD_PHYS
               && !(mattk[1].aatyp == AT_WEAP
                    && mattk[1].adtyp == AD_PHYS)
               && (magr->mcan
                   || (weap && ((weap->otyp == CORPSE
                                 && touch_petrifies(&mons[weap->corpsenm]))
                                || is_art(weap, ART_STORMBRINGER)
                                || is_art(weap, ART_SHADOWBLADE)
                                || is_art(weap, ART_VORPAL_BLADE))))) {
        *alt_attk_buf = *attk;
        attk = alt_attk_buf;
        attk->adtyp = AD_PHYS;

    /* liches have a touch attack for cold damage and also a spell attack;
       they won't use the spell for monster vs monster so become impotent
       against cold resistant foes; change the touch damage from cold to
       physical if target will resist */
    } else if (indx == 0 && attk->aatyp == AT_TUCH && attk->adtyp == AD_COLD
               && (udefend ? how_resistant(COLD_RES) == 100
                           : (resists_cold(mdef)
                              || defended(mdef, AD_COLD)))
               /* don't substitute if target is immune to normal damage */
               && !noncorporeal(mdef->data)) {
        *alt_attk_buf = *attk;
        attk = alt_attk_buf;
        attk->adtyp = AD_PHYS;
        /* lessen new physical damage compared to old cold damage:
         *        before  after
         * lich:    1d10  1d6
         * demi:    3d4   2d4
         * master:  3d6   2d6
         * arch-:   5d6   3d6
         */
        attk->damn = (attk->damn + 1) / 2;
        if (attk->damd == 10)
            attk->damd = 6;

    }

    /* elementals on their home plane do double damage */
    if (attk != alt_attk_buf && is_home_elemental(mptr)) {
        *alt_attk_buf = *attk;
        attk = alt_attk_buf;
        attk->damn *= 2;
    }

    return attk;
}

/* calc some variables needed for mattacku() */
staticfn void
calc_mattacku_vars(
    struct monst *mtmp,
    boolean *ranged, boolean *range2,
    boolean *foundyou, boolean *youseeit)
{
    *ranged = (mdistu(mtmp) > 3);
    *range2 = !monnear(mtmp, mtmp->mux, mtmp->muy);
    *foundyou = u_at(mtmp->mux, mtmp->muy);
    *youseeit = canseemon(mtmp);

    /* do_attack() uses bhitpos to set/clear notonhead; do likewise here */
    gb.bhitpos.x = u.ux, gb.bhitpos.y = u.uy;
    /* hero poly'd into a long worm isn't allowed to grow a tail, so
       hitting tail instead of head can't happen */
    gn.notonhead = FALSE;
}

/* return TRUE iff monster or hero is trapped in a (spiked) pit */
boolean
mtrapped_in_pit(struct monst *mtmp)
{
    struct trap *ttmp = 0;

    if (mtmp == &gy.youmonst)
        ttmp = (u.utrap && u.utraptype == TT_PIT) ? t_at(u.ux, u.uy) : 0;
    else
        ttmp = mtmp->mtrapped ? t_at(mtmp->mx, mtmp->my) : 0;

    if (ttmp && is_pit(ttmp->ttyp))
        return TRUE;
    return FALSE;
}

/*
 * mattacku: monster attacks you
 *      returns 1 if monster dies (e.g. "yellow light"), 0 otherwise
 *      Note: if you're displaced or invisible the monster might attack the
 *              wrong position...
 *      Assumption: it's attacking you or an empty square; if there's another
 *              monster which it attacks by mistake, the caller had better
 *              take care of it...
 */
int
mattacku(struct monst *mtmp)
{
    struct attack *mattk, alt_attk;
    int i, j = 0, tmp, sum[NATTK];
    int weap_attk_idx = 0; /* which AT_WEAP attack (0=primary, 1+=off-hand) */
    struct permonst *mdat = mtmp->data;
    /*
     * ranged: Is it near you?  Affects your actions.
     * range2: Does it think it's near you?  Affects its actions.
     * foundyou: Is it attacking you or your image?
     * youseeit: Can you observe the attack?  It might be attacking your
     *     image around the corner, or invisible, or you might be blind.
     * skipnonmagc: Are further physical attack attempts useless?  (After
     *     a wild miss--usually due to attacking displaced image.  Avoids
     *     excessively verbose miss feedback when monster can do multiple
     *     attacks and would miss the same wrong spot each time.)
     */
    boolean ranged, range2, foundyou, firstfoundyou, youseeit,
            was_unfindable, skipnonmagc = FALSE;

    calc_mattacku_vars(mtmp, &ranged, &range2, &foundyou, &youseeit);

    if (!ranged)
        nomul(0);
    if (DEADMONSTER(mtmp))
        return 1;
    if (Underwater && !is_swimmer(mtmp->data))
        return 0;

    /* If swallowed, can only be affected by u.ustuck */
    if (u.uswallow) {
        if (mtmp != u.ustuck)
            return 0;
        u.ustuck->mux = u.ux;
        u.ustuck->muy = u.uy;
        if (u.uinvulnerable)
            return 0; /* stomachs can't hurt you! */
        range2 = 0;
        foundyou = 1;
    } else if (u.usteed) {
        if (mtmp == u.usteed)
            /* Your steed won't attack you */
            return 0;
        /* Orcs like to steal and eat horses and the like */
        if (!rn2(racial_orc(mtmp) ? 2 : 4) && m_next2u(mtmp)) {
            /* attack your steed instead; 'bhitpos' and 'notonhead' are
               already set from targeting hero */
            i = mattackm(mtmp, u.usteed);
            if ((i & M_ATTK_AGR_DIED) != 0)
                return 1;
            /* make sure steed is still alive and within range, and still
               tame: life saving or a charm spell can untame it, and it
               stays under the hero until it is thrown off */
            if ((i & M_ATTK_DEF_DIED) != 0 || !u.usteed
                || !u.usteed->mtame || !m_next2u(mtmp))
                return 0;
            /* Let your steed retaliate (check acceptable_pet_target()
               as a guard) */
            if (!acceptable_pet_target(u.usteed, mtmp, FALSE))
                return 0;
            gb.bhitpos.x = mtmp->mx, gb.bhitpos.y = mtmp->my;
            gn.notonhead = FALSE;
            return !!(mattackm(u.usteed, mtmp) & M_ATTK_DEF_DIED);
        }
        /* a steed will attack on the player's behalf without waiting for
           the player or itself to be attacked first, provided it is loyal
           or better; a steed with a ranged attack can use it apart from
           this routine */
        if (u.usteed->mtame >= 15 && m_next2u(mtmp)
            && acceptable_pet_target(u.usteed, mtmp, FALSE)) {
            gb.bhitpos.x = mtmp->mx, gb.bhitpos.y = mtmp->my;
            gn.notonhead = FALSE;
            i = mattackm(u.usteed, mtmp);
            /* the steed died; mtmp wasn't the aggressor here, so the
               return code mustn't claim that mtmp died */
            if ((i & M_ATTK_AGR_DIED) != 0 || !u.usteed)
                return 0;
            return !!(i & M_ATTK_DEF_DIED);
        }
    }

    if (u.uundetected && !range2 && foundyou && !u.uswallow) {
        if (!canspotmon(mtmp))
            map_invisible(mtmp->mx, mtmp->my);
        u.uundetected = 0;
        if (is_hider(gy.youmonst.data) && u.umonnum != PM_TRAPPER) {
            /* ceiling hider */
            coord cc; /* maybe we need a unexto() function? */
            struct obj *obj;

            You("fall from the %s!", ceiling(u.ux, u.uy));
            /* take monster off map now so that its location
               is eligible for placing hero; we assume that a
               removed monster remembers its old spot <mx,my> */
            remove_monster(mtmp->mx, mtmp->my);
            if (!enexto_core_mon(&cc, u.ux, u.uy, &gy.youmonst, NO_MM_FLAGS)
                /* a fish won't voluntarily swap positions
                   when it's in water and hero is over land */
                || (mtmp->data->mlet == S_EEL
                    && (is_pool(mtmp->mx, mtmp->my)
                        || is_puddle(mtmp->mx, mtmp->my))
                    && !(is_pool(u.ux, u.uy) || is_puddle(u.ux, u.uy)))
                || (mtmp->data == &mons[PM_GIANT_LEECH]
                    && is_sewage(mtmp->mx, mtmp->my)
                    && !is_sewage(u.ux, u.uy))) {
                /* couldn't find any spot for hero; this used to
                   kill off attacker, but now we just give a "miss"
                   message and keep both mtmp and hero at their
                   original positions; hero has become unconcealed
                   so mtmp's next move will be a regular attack */
                place_monster(mtmp, mtmp->mx, mtmp->my); /* put back */
                newsym(u.ux, u.uy); /* u.uundetected was toggled */
                pline_mon(mtmp, "%s draws back as you drop!", Monnam(mtmp));
                return 0;
            }

            /* put mtmp at hero's spot and move hero to <cc.x,.y> */
            newsym(mtmp->mx, mtmp->my); /* finish removal */
            place_monster(mtmp, u.ux, u.uy);
            if (mtmp->wormno) {
                worm_move(mtmp);
                /* tail hasn't grown, so if it now occupies <cc.x,.y>
                   then one of its original spots must be free */
                if (m_at(cc.x, cc.y))
                    (void) enexto_core_mon(&cc, u.ux, u.uy, &gy.youmonst,
                                           NO_MM_FLAGS);
            }
            teleds(cc.x, cc.y, TELEDS_ALLOW_DRAG); /* move hero */
            set_apparxy(mtmp);
            newsym(u.ux, u.uy);

            if (gy.youmonst.data->mlet != S_PIERCER)
                return 0; /* lurkers don't attack */

            obj = which_armor(mtmp, WORN_HELMET);
            if (hard_helmet(obj)) {
                Your("blow glances off %s %s.", s_suffix(mon_nam(mtmp)),
                     helm_simple_name(obj));
            } else {
                if (3 + find_mac(mtmp) <= rnd(20)) {
                    pline("%s is hit by a falling piercer (you)!",
                          Monnam(mtmp));
                    if ((mtmp->mhp -= d(3, 6)) < 1)
                        killed(mtmp);
                } else
                    pline("%s is almost hit by a falling piercer (you)!",
                          Monnam(mtmp));
            }

        } else {
            /* surface hider */
            if (!youseeit) {
                pline("It tries to move where you are hiding.");
            } else {
                /* Ugly kludge for eggs.  The message is phrased so as
                 * to be directed at the monster, not the player,
                 * which makes "laid by you" wrong.  For the
                 * parallelism to work, we can't rephrase it, so we
                 * zap the "laid by you" momentarily instead.
                 */
                struct obj *obj = svl.level.objects[u.ux][u.uy];

                if (obj || u.umonnum == PM_TRAPPER
                    || (u.umonnum == PM_GIANT_LEECH
                        && is_sewage(u.ux, u.uy))
                    || (gy.youmonst.data->mlet == S_EEL
                        && (is_pool(u.ux, u.uy) || is_puddle(u.ux, u.uy)))) {
                    int save_spe = 0; /* suppress warning */

                    if (obj) {
                        save_spe = obj->spe;
                        if (obj->otyp == EGG)
                            obj->spe = 0;
                    }
                    /* note that m_monnam() overrides hallucination, which is
                       what we want when message is from mtmp's perspective */
                    if (gy.youmonst.data->mlet == S_EEL
                        || u.umonnum == PM_TRAPPER
                        || u.umonnum == PM_GIANT_LEECH || !obj)
                        pline(
                             "Wait, %s!  There's a hidden %s named %s there!",
                              m_monnam(mtmp),
                              pmname(gy.youmonst.data, Ugender), svp.plname);
                    else
                        pline(
                          "Wait, %s!  There's a %s named %s hiding under %s!",
                              m_monnam(mtmp),
                              pmname(gy.youmonst.data, Ugender),
                              svp.plname,
                              doname(svl.level.objects[u.ux][u.uy]));
                    if (obj)
                        obj->spe = save_spe;
                } else
                    impossible("hiding under nothing?");
            }
            newsym(u.ux, u.uy);
        }
        return 0;
    }

    /* hero might be a mimic, concealed via #monster */
    if (gy.youmonst.data->mlet == S_MIMIC && U_AP_TYPE && !range2
        && foundyou && !u.uswallow) {
        boolean sticky = sticks(gy.youmonst.data);

        if (!canspotmon(mtmp))
            map_invisible(mtmp->mx, mtmp->my);
        if (sticky && !youseeit)
            pline("It gets stuck on you.");
        else /* see note about m_monnam() above */
            pline("Wait, %s!  That's a %s named %s!", m_monnam(mtmp),
                  pmname(gy.youmonst.data, Ugender), svp.plname);
        if (sticky) {
            set_ustuck(mtmp);
            /* the attacker may have struck from cover */
            if (mtmp->mundetected) {
                mtmp->mundetected = 0;
                newsym(mtmp->mx, mtmp->my);
            }
        }
        gy.youmonst.m_ap_type = M_AP_NOTHING;
        gy.youmonst.mappearance = 0;
        newsym(u.ux, u.uy);
        return 0;
    }

    /* non-mimic hero might be mimicking an object after eating m corpse */
    if (U_AP_TYPE == M_AP_OBJECT && !range2 && foundyou && !u.uswallow) {
        if (!canspotmon(mtmp))
            map_invisible(mtmp->mx, mtmp->my);
        if (!youseeit)
            pline("%s %s!", Something,
                  (likes_gold(mtmp->data)
                   && gy.youmonst.mappearance == GOLD_PIECE)
                  ? "tries to pick you up"
                  : "disturbs you");
        else /* see note about m_monnam() above */
            pline("Wait, %s!  That %s is really %s named %s!", m_monnam(mtmp),
                  mimic_obj_name(&gy.youmonst),
                  an(pmname(&mons[u.umonnum], Ugender)), svp.plname);
        if (gm.multi < 0) { /* this should always be the case */
            char buf[BUFSZ];

            Sprintf(buf, "You appear to be %s again.",
                    Upolyd ? (const char *) an(pmname(gy.youmonst.data,
                                                      flags.female))
                           : (const char *) "yourself");
            unmul(buf); /* immediately stop mimicking */
        }
        return 0;
    }

    /*  Work out the armor class differential   */
    tmp = AC_VALUE(u.uac) + 10; /* tmp ~= 0 - 20 */
    tmp += mtmp->m_lev;
    if (gm.multi < 0)
        tmp += 4;
    if ((Invis && !mon_prop(mtmp, SEE_INVIS)) || !mtmp->mcansee)
        tmp -= 2;
    if (mtmp->mtrapped || mtmp->mentangled)
        tmp -= 2;
    /* M3_ACCURATE monsters get a to-hit bonus */
    if ((has_erac(mtmp) && (ERAC(mtmp)->mflags3 & M3_ACCURATE))
        || is_accurate(mdat))
        tmp += Hidinshell ? 0 : 5;
    if (Hidinshell) /* enshelled tortles are much harder to hit */
        tmp -= 12;
    /* drow are affected by being in both the light or the dark */
    if (racial_drow(mtmp)) {
        if (spot_is_dark(mtmp->mx, mtmp->my))
            tmp += (mtmp->m_lev / 3) + 2;
        else
            tmp -= 3;
    }
    if (tmp <= 0)
        tmp = 1;

    /* make eels visible the moment they hit/miss us */
    if ((mdat->mlet == S_EEL || mdat == &mons[PM_GIANT_LEECH])
        && mtmp->minvis && cansee(mtmp->mx, mtmp->my)) {
        mtmp->minvis = 0;
        newsym(mtmp->mx, mtmp->my);
    }

    /* when not cancelled and not in current form due to shapechange, many
       demons can summon more demons and were creatures can summon critters;
       also, were creature might change from human to animal or vice versa */
    if (mtmp->cham == NON_PM && !mtmp->mcan && !range2
        && (is_demon(mdat) || is_were(mdat))) {
        boolean already_fleeing = mtmp->mflee != 0;

        summonmu(mtmp, youseeit);
        /* were-creature might have changed to beast form; if that has
           caused it to become afraid (due to non-human reacting to scroll
           of scare monster or engraved "Elbereth" which was being ignored
           while in human form), don't continue this attack */
        if (mtmp->mflee && !already_fleeing)
            return 0;
        mdat = mtmp->data; /* update cached value in case of were change */
    }

    if (u.uinvulnerable) { /* in the midst of successful prayer */
        /* monsters won't attack you */
        if (mtmp == u.ustuck) {
            pline_mon(mtmp, "%s loosens its grip slightly.", Monnam(mtmp));
        } else if (!range2) {
            if (youseeit || sensemon(mtmp))
                pline("%s starts to attack you, but pulls back.",
                      Monnam(mtmp));
            else
                You_feel("%s move nearby.", something);
        }
        return 0;
    }

    /* Unlike defensive stuff, don't let them use item _and_ attack. */
    if (find_offensive(mtmp)) {
        int offended = use_offensive(mtmp);

        if (offended != 0)
            return (offended == 1);
    }

    gs.skipdrin = FALSE; /* [see mattackm(mhitm.c)] */
    firstfoundyou = foundyou;
    /* the set of hero states wildmiss() can explain; tracked so a
       mid-chain transition out of all of them (burnt-away cloak of
       invisibility, lost displacement) refreshes the monster's guess
       instead of leaving mux/muy stale and wildmiss() unexplained */
    was_unfindable = (!mtmp->mcansee
                      || (Invis && !mon_prop(mtmp, SEE_INVIS))
                      || Displaced || Underwater);

    for (i = 0; i < NATTK; i++) {
        sum[i] = M_ATTK_MISS;
        /* counterattack against attack [i-1] might have been fatal */
        if (DEADMONSTER(mtmp))
            return 1;
        if (i > 0) {
            boolean now_unfindable = (!mtmp->mcansee
                                      || (Invis
                                          && !mon_prop(mtmp, SEE_INVIS))
                                      || Displaced || Underwater);

            /* a counter-effect of attack [i-1] can end the state that
               was hiding the hero; refresh the guess only on that
               transition so a hero who stays hidden throughout the
               chain is not re-rolled every attack */
            if (was_unfindable && !now_unfindable)
                set_apparxy(mtmp);
            was_unfindable = now_unfindable;
            /* recalc in case prior attack moved hero; mtmp doesn't make
               another attempt to guess your location while you stay
               hidden but might have accidentally knocked you to where
               it thought you were [not sure whether that's actually
               possible] */
            calc_mattacku_vars(mtmp, &ranged, &range2, &foundyou, &youseeit);
            /* if hero was found but isn't anymore, avoid wildmiss now */
            if (firstfoundyou && !foundyou)
                continue; /* set sum[i] to 'miss' but skip other actions */
            if (!u_at(gb.bhitpos.x, gb.bhitpos.y))
                continue;
        }
        mon_currwep = (struct obj *) 0;
        mattk = getmattk(mtmp, &gy.youmonst, i, sum, &alt_attk);
        if ((u.uswallow && mattk->aatyp != AT_ENGL)
            || (skipnonmagc && mattk->aatyp != AT_MAGC)
            || (gs.skipdrin && mattk->aatyp == AT_TENT
                && mattk->adtyp == AD_DRIN))
            continue;

        switch (mattk->aatyp) {
        case AT_CLAW: /* "hand to hand" attacks */
        case AT_KICK:
        case AT_BITE:
        case AT_STNG:
        case AT_TUCH:
        case AT_BUTT:
        case AT_TENT:
            if (mattk->aatyp == AT_KICK && mtrapped_in_pit(mtmp))
                continue;
            if (!range2 && (!MON_WEP(mtmp) || mtmp->mconf || Conflict
                            || !touch_petrifies(gy.youmonst.data))) {
                if (foundyou) {
                    /* if our hero is sized tiny or small and unencumbered,
                       they have a decent chance of evading a zombie's
                       bite attack */
                    if (racial_zombie(mtmp) && mattk->aatyp == AT_BITE
                        && gy.youmonst.data->msize <= MZ_SMALL
                        && is_animal(gy.youmonst.data)
                        && near_capacity() == UNENCUMBERED
                        && !(Confusion || Stunned || Punished || gm.multi < 0
                             || Wounded_legs || Stoned || Fumbling)
                        && rn2(3)) {
                        You("nimbly %s %s bite!",
                            rn2(2) ? "dodge" : "evade",
                            s_suffix(mon_nam(mtmp)));
                        return 0; /* attack stops */
                    }
                    /* a tortle's shell wards off bites, tentacles,
                       and stings from smaller attackers */
                    if (mdat->msize <= MZ_LARGE && mattk->aatyp == AT_BITE
                        && Hidinshell) {
                        Your("protective shell blocks %s bite!",
                             s_suffix(mon_nam(mtmp)));
                        return 0; /* attack stops */
                    }
                    if (is_illithid(mdat) && mattk->aatyp == AT_TENT
                        && Hidinshell) {
                        Your("protective shell blocks %s tentacle attack!",
                             s_suffix(mon_nam(mtmp)));
                        return 0; /* attack stops */
                    }
                    if (mattk->aatyp == AT_STNG && Hidinshell) {
                        pline("%s stinger glances off of your protective %s!",
                              s_suffix(Monnam(mtmp)), "shell");
                        return 0; /* attack stops */
                    }
                    if (tmp > (j = rnd(20 + i))) {
                        if (unsolid(gy.youmonst.data)
                            && failed_grab(mtmp, &gy.youmonst, mattk))
                            continue;
                        if (mattk->aatyp != AT_KICK
                            || (!thick_skinned(gy.youmonst.data)
                                && !Barkskin && !Stoneskin))
                            sum[i] = hitmu(mtmp, mattk);
                    } else {
                        missmu(mtmp, tmp, j, mattk);
                        /* a shield gets some practice from any blow
                           the hero avoids while wearing one */
                        if (uarms && !rn2(3))
                            use_skill(P_SHIELD, 1);
                        /* missmu() can now kill the attacker outright -
                           a glancing blow off armor of a material it
                           hates, or an Ashmar/Retribution knockback -
                           and a dead monster must not go on to dispatch
                           the rest of its attack chain */
                        if (DEADMONSTER(mtmp))
                            return 1; /* attacker dead */
                    }
                } else {
                    wildmiss(mtmp, mattk);
                    /* skip any remaining non-spell attacks */
                    skipnonmagc = TRUE;
                }
            }
            break;

        case AT_HUGS: /* automatic if prev two attacks succeed */
            /* Note: if displaced, prev attacks never succeeded; a
               stationary monster or a tree blight hugs the hero it has
               found next to it without the two lead-in hits, and a
               holder only squeezes from an adjacent square */
            if (!ranged
                && ((i >= 2 && sum[i - 1] && sum[i - 2])
                    || ((is_stationary(mdat)
                         || mdat == &mons[PM_TREE_BLIGHT])
                        && foundyou)
                    || mtmp == u.ustuck)) {
                if (!failed_grab(mtmp, &gy.youmonst, mattk))
                    sum[i] = hitmu(mtmp, mattk);
            }
            break;

        case AT_GAZE: /* can affect you either ranged or not */
            /* Medusa gaze already operated through m_respond in
               dochug(); don't gaze more than once per round. */
            if (mdat != &mons[PM_MEDUSA])
                sum[i] = gazemu(mtmp, mattk);
            break;

        case AT_EXPL: /* automatic hit if next to, and aimed at you */
            if (!range2)
                sum[i] = explmu(mtmp, mattk, foundyou);
            break;

        case AT_ENGL:
            if (!range2) {
                if (foundyou) {
                    if (u.uswallow
                        || (!mtmp->mspec_used && tmp > (j = rnd(20 + i)))) {
                        /* force swallowing monster to be displayed
                           even when hero is moving away */
                        flush_screen(1);
                        sum[i] = gulpmu(mtmp, mattk);
                    } else {
                        /* a spent engulfer never made a roll, so j still
                           holds the previous attack's; a zero roll makes
                           the blocker ladder decline, as it should when
                           nothing was deflected */
                        if (mtmp->mspec_used)
                            j = 0;
                        missmu(mtmp, tmp, j, mattk);
                        /* a shield gets some practice from any blow
                           the hero avoids while wearing one */
                        if (uarms && !rn2(3))
                            use_skill(P_SHIELD, 1);
                        /* missmu() can now kill the attacker outright -
                           a glancing blow off armor of a material it
                           hates, or an Ashmar/Retribution knockback -
                           and a dead monster must not go on to dispatch
                           the rest of its attack chain */
                        if (DEADMONSTER(mtmp))
                            return 1; /* attacker dead */
                    }
                } else if (digests(mtmp->data)) {
                    pline_mon(mtmp, "%s gulps some air!", Monnam(mtmp));
                } else {
                    if (youseeit) {
                        pline_mon(mtmp, "%s lunges forward and recoils!",
                                  Monnam(mtmp));
                    } else {
                        if (is_whirly(mtmp->data)) {
                            Soundeffect(se_rushing_wind_noise, 60);
                        }
                        You_hear("a %s nearby.",
                                 is_whirly(mtmp->data) ? "rushing noise"
                                                       : "splat");
                    }
                }
            }
            break;
        case AT_BREA:
            if (range2)
                sum[i] = breamu(mtmp, mattk);
            /* Note: breamu takes care of displacement */
            break;
        case AT_SPIT:
            if (range2)
                sum[i] = spitmu(mtmp, mattk);
            /* Note: spitmu takes care of displacement */
            break;
        case AT_WEAP:
            if (range2) {
                if (!Is_rogue_level(&u.uz))
                    thrwmu(mtmp);
                weap_attk_idx++;
            } else {
                int hittmp = 0;
                boolean had_wep = FALSE;

                if (weap_attk_idx == 0) {
                    /* first AT_WEAP: use primary weapon */
                    /* Rare but not impossible. Normally the monster
                     * wields when 2 spaces away, but it can be
                     * teleported or whatever....
                     */
                    if (mtmp->weapon_check == NEED_WEAPON || !MON_WEP(mtmp)
                        || (is_launcher(MON_WEP(mtmp))
                            && !(MON_WEP(mtmp))->cursed)) {
                        mtmp->weapon_check = NEED_HTH_WEAPON;
                        /* mon_wield_item resets weapon_check
                           as appropriate */
                        if (mon_wield_item(mtmp) != 0) {
                            weap_attk_idx++;
                            break;
                        }
                    }
                    if (!MON_WEP(mtmp) || is_launcher(MON_WEP(mtmp))) {
                        /* implies we could not find a HTH weapon,
                         * try point blank ranged attack */
                        if (thrwmu(mtmp)) {
                            weap_attk_idx++;
                            break;
                        }
                    }
                    mon_currwep = MON_WEP(mtmp);
                } else {
                    /* 2nd+ AT_WEAP: use off-hand weapon */
                    mon_currwep = MON_WEP2(mtmp);
                    if (mon_currwep)
                        tmp -= OFFHAND_PENALTY;
                    else
                        mon_currwep = MON_WEP(mtmp); /* fallback */
                }
                weap_attk_idx++;

                if (foundyou) {
                    if (mon_currwep) {
                        boolean bash = (is_pole(mon_currwep)
                                        && !is_art(mon_currwep,
                                                   ART_SNICKERSNEE)
                                        && m_next2u(mtmp));

                        hittmp = hitval(mtmp, mon_currwep, &gy.youmonst);
                        tmp += hittmp;
                        had_wep = TRUE;
                        mswings(mtmp, mon_currwep, bash);
                    }
                    if (tmp > (j = gm.mhitu_dieroll = rnd(20 + i))) {
                        sum[i] = hitmu(mtmp, mattk);
                    } else {
                        missmu(mtmp, tmp, j, mattk);
                        /* a shield gets some practice from any blow
                           the hero avoids while wearing one */
                        if (uarms && !rn2(3))
                            use_skill(P_SHIELD, 1);
                        /* missmu() can now kill the attacker outright -
                           a glancing blow off armor of a material it
                           hates, or an Ashmar/Retribution knockback -
                           and a dead monster must not go on to dispatch
                           the rest of its attack chain */
                        if (DEADMONSTER(mtmp))
                            return 1; /* attacker dead */
                    }
                    /* KMH -- Don't accumulate to-hit bonuses; keyed on
                       whether a bonus was added rather than on the
                       cached weapon pointer, which mon_currwep_gone()
                       nulls when the weapon is destroyed mid-attack */
                    if (had_wep)
                        tmp -= hittmp;
                } else {
                    wildmiss(mtmp, mattk);
                    /* skip any remaining non-spell attacks */
                    skipnonmagc = TRUE;
                }
            }
            break;
        case AT_MAGC:
            if (range2)
                sum[i] = buzzmu(mtmp, mattk);
            else
                sum[i] = castmu(mtmp, mattk, TRUE, foundyou);
            break;

        case AT_SCRE:
            if (ranged || !rn2(5)) /* sometimes right next to our hero */
                sum[i] = screamu(mtmp, mattk);
            break;

        default: /* no attack */
            break;
        }
        if (disp.botl)
            bot();
        /* give player a chance of waking up before dying -kaa */
        if (sum[i] == M_ATTK_HIT) { /* successful attack */
            if (u.usleep && u.usleep < svm.moves && !rn2(10)) {
                gm.multi = -1;
                gn.nomovemsg = "The combat suddenly awakens you.";
            }
        }
        if ((sum[i] & M_ATTK_AGR_DIED))
            return 1; /* attacker dead */
        if ((sum[i] & M_ATTK_AGR_DONE))
            break; /* attacker teleported, no more attacks */
        /* sum[i] == 0: unsuccessful attack */
    }
    return 0;
}

/* monster summons help for its fight against hero */
staticfn void
summonmu(struct monst *mtmp, boolean youseeit)
{
    struct permonst *mdat = mtmp->data;

    /*
     * Extracted from mattacku() to reduce clutter there.
     * Caller has verified that 'mtmp' hasn't been cancelled
     * and isn't a shapechanger.
     */

    if (is_demon(mdat)) {
        /* the demon player race monster does not chain-summon */
        if (mdat != &mons[PM_BALROG] && mdat != &mons[PM_AMOROUS_DEMON]
            && mdat != &mons[PM_DEMON]) {
            if (!rn2(Inhell ? 10 : 13))
                (void) msummon(mtmp);
        }
        return; /* no such thing as a demon were creature, so we're done */
    }

    if (is_were(mdat)) {
        /* if hero has Protection_from_shape_changers, new_were() will work
           in the critter-to-human direction but be a no-op the other way;
           we repeat the criteria here for clarity */
        if (is_human(mdat)) { /* maybe switch to animal form */
            if (!Protection_from_shape_changers && !rn2(5 - (night() * 2)))
                new_were(mtmp);
        } else { /* maybe switch to back human form */
            if (Protection_from_shape_changers || !rn2(30))
                new_were(mtmp);
        }
        mdat = mtmp->data; /* form change invalidates cached value */

        /* maybe summon compatible critters;
           not blocked by Protection_from_shape_changers */
        if (!rn2(10)) {
            int numseen, numhelp;
            char buf[BUFSZ], genericwere[BUFSZ];

            Strcpy(genericwere, "creature");
            if (youseeit)
                pline_mon(mtmp, "%s summons help!", Monnam(mtmp));
            numhelp = were_summon(mdat, FALSE, &numseen, genericwere);
            if (youseeit) {
                if (numhelp > 0) {
                    if (numseen == 0)
                        You_feel("hemmed in.");
                } else {
                    pline("But none comes.");
                }
            } else {
                const char *from_nowhere;

                if (!Deaf) {
                    pline("%s %s!", Something,
                          makeplural(growl_sound(mtmp)));
                    from_nowhere = "";
                } else {
                    from_nowhere = " from nowhere";
                }
                if (numhelp > 0) {
                    if (numseen < 1) {
                        You_feel("hemmed in.");
                    } else {
                        if (numseen == 1)
                            Sprintf(buf, "%s appears", an(genericwere));
                        else
                            Sprintf(buf, "%s appear",
                                    makeplural(genericwere));
                        pline("%s%s!", upstart(buf), from_nowhere);
                    }
                } /* else no help came; but you didn't know it tried */
            }
        } /* summon critters */
        return;
    } /* were creature */
}

boolean
diseasemu(struct monst *mtmp)
{
    if (Sick_resistance) {
        You_feel("a slight illness.");
        return FALSE;
    } else {
        make_sick(Sick ? Sick / 3L + 1L : (long) rn1(ACURR(A_CON), 20),
                  mtmp->data->pmnames[NEUTRAL], TRUE,
                  racial_zombie(mtmp) ? SICK_ZOMBIE : SICK_NONVOMITABLE);
        return TRUE;
    }
}

/* check whether slippery clothing protects from hug or wrap attack */
boolean
u_slip_free(
    struct monst *mtmp,
    struct attack *mattk)
{
    struct obj *obj;

    /* greased armor does not protect against AT_ENGL+AD_WRAP */
    if (mattk->aatyp == AT_ENGL)
        return FALSE;

    obj = (uarmc ? uarmc : uarm);
    if (!obj)
        obj = uarmu;
    if (mattk->adtyp == AD_DRIN)
        obj = uarmh;
    if (mtmp->data == &mons[PM_MIND_FLAYER_LARVA]
        && mattk->aatyp == AT_TENT && mattk->adtyp == AD_WRAP)
        obj = uarmh;

    /* if your cloak/armor is greased, monster slips off; this
       protection might fail (33% chance) when the armor is cursed */
    if (obj && (obj->greased || obj->otyp == OILSKIN_CLOAK
                || (obj->oprops & ITEM_OILSKIN))
        && (!obj->cursed || rn2(3))) {
        /* a salamander grabs with its arms rather than coiling */
        pline_mon(mtmp, "%s %s your %s!", Monnam(mtmp),
                  (mattk->adtyp == AD_WRAP
                   && mtmp->data != &mons[PM_SALAMANDER])
                      ? "slips off of"
                  : (mattk->adtyp == AD_DRIN && racial_zombie(mtmp))
                      ? "bites you, but slips off of"
                  : "grabs you, but cannot hold onto",
                  slippery_gear_name(obj));

        if (obj->greased && !rn2(2)) {
            pline_The("grease wears off.");
            obj->greased = 0;
            update_inventory();
        }
        return TRUE;
    }
    return FALSE;
}

/* armor that sufficiently covers the body might be able to block magic */
int
magic_negation(struct monst *mon)
{
    struct obj *o;
    long wearmask;
    int armpro, mc = 0;
    boolean is_you = (mon == &gy.youmonst),
            via_amul = FALSE,
            gotprot = is_you ? (EProtection != 0L)
                             /* high priests have innate protection */
                             : (mon->data == &mons[PM_HIGH_PRIEST]);

    for (o = is_you ? gi.invent : mon->minvent; o; o = o->nobj) {
        /* a_can field is only applicable for armor (which must be worn) */
        if ((o->owornmask & W_ARMOR) != 0L) {
            armpro = objects[o->otyp].a_can;
            /* mithril armor grants a level of MC due to its magical
               origins, plus extra for elves */
            if ((o->owornmask & (W_ARM | W_ARMC)) != 0
                && o->material == MITHRIL) {
                armpro = max(armpro,
                             ((is_you ? (Race_if(PM_ELF) && !Upolyd)
                                      : racial_elf(mon)) ? 3 : 2));
            /* bone or stone armor grants MC to orcs */
            } else if ((o->owornmask & (W_ARM | W_ARMC)) != 0
                       && (o->material == BONE || o->material == MINERAL)
                       && (is_you ? (Race_if(PM_ORC) && !Upolyd)
                                  : racial_orc(mon))
                       && armpro < 3) {
                armpro = 3;
            }
            if (armpro > mc)
                mc = armpro;
        } else if ((o->owornmask & W_AMUL) != 0L) {
            via_amul = (o->otyp == AMULET_OF_GUARDING);
        }
        /* if we've already confirmed Protection, skip additional checks */
        if (is_you || gotprot)
            continue;

        /* omit W_SWAPWEP+W_QUIVER; W_ART+W_ARTI handled by protects() */
        wearmask = W_ARMOR | W_ACCESSORY;
        if (o->oclass == WEAPON_CLASS || is_weptool(o))
            wearmask |= W_WEP;
        if (protects(o, ((o->owornmask & wearmask) != 0L) ? TRUE : FALSE))
            gotprot = TRUE;
    }

    if (gotprot) {
        /* extrinsic Protection increases mc by 1 (2 for amulet of guarding);
           multiple sources don't provide multiple increments */
        mc += via_amul ? 2 : 1;
        if (mc > 3)
            mc = 3;
    } else if (mc < 1) {
        /* intrinsic Protection is weaker (play balance; obtaining divine
           protection is too easy); it confers minimum mc 1 instead of 0 */
        if ((is_you && ((HProtection && u.ublessed > 0) || u.uspellprot))
            /* aligned priests and angels have innate intrinsic Protection */
            || (mon->data == &mons[PM_ALIGNED_PRIEST]
                || is_minion(mon->data)))
            mc = 1;
    }
    return mc;
}

/*
 * hitmu: monster hits you
 * returns MM_ flags
*/
staticfn int
hitmu(struct monst *mtmp, struct attack *mattk)
{
    struct permonst *mdat = mtmp->data;
    struct permonst *olduasmon = gy.youmonst.data;
    int res;
    struct mhitm_data mhm;
    mhm.hitflags = M_ATTK_MISS;
    mhm.permdmg = 0;
    mhm.specialdmg = 0;
    mhm.mwep = mon_currwep;
    mhm.done = FALSE;

    if (!canspotmon(mtmp))
        map_invisible(mtmp->mx, mtmp->my);

    /* Awaken nearby monsters */
    if (!(is_silent(mdat) && gm.multi < 0) && rn2(10)) {
        wake_nearto(u.ux, u.uy, combat_noise(mtmp->data));
    }

    /*  If the monster is undetected & hits you, you should know where
     *  the attack came from.
     */
    if (mtmp->mundetected
        && (hides_under(mdat) || mdat->mlet == S_EEL
            || mdat == &mons[PM_GIANT_LEECH])) {
        mtmp->mundetected = 0;
        if (!tp_sensemon(mtmp) && !Detect_monsters) {
            struct obj *obj;
            const char *what;
            char Amonbuf[BUFSZ];

            /* eels and giant leeches hide in bare water/sewage with
               no object cover */
            if ((obj = svl.level.objects[mtmp->mx][mtmp->my]) != 0
                || is_damp_terrain(mtmp->mx, mtmp->my)) {
                if (Blind && !(obj && obj->dknown))
                    what = something;
                else if (is_pool(mtmp->mx, mtmp->my) && !Underwater)
                    what = "the water";
                else if (is_puddle(mtmp->mx, mtmp->my))
                    what = "the shallow water";
                else if (is_sewage(mtmp->mx, mtmp->my))
                    what = "the raw sewage";
                else if (obj)
                    what = doname(obj);
                else
                    what = something;

                Strcpy(Amonbuf, Amonnam(mtmp));
                /* mtmp might be invisible with hero unable to see same */
                if (!strcmp(Amonbuf, "It")) /* note: not strcmpi() */
                    Strcpy(Amonbuf, Something);
                pline("%s was hidden under %s!", Amonbuf, what);
            }
            newsym(mtmp->mx, mtmp->my);
        }
    }

    /*  First determine the base damage done */
    mhm.damage = d((int) mattk->damn, (int) mattk->damd);
    if ((is_undead(mdat) || is_vampshifter(mtmp)) && midnight())
        mhm.damage += d((int) mattk->damn, (int) mattk->damd); /* extra dmg */

    mhitm_adtyping(mtmp, mattk, &gy.youmonst, &mhm);

    (void) mhitm_knockback(mtmp, &gy.youmonst, mattk, &mhm.hitflags,
                           (MON_WEP(mtmp) != 0));

    if (mhm.done)
        return mhm.hitflags;

    /* player monster monks can sometimes stun with their kick attack */
    if (mattk->aatyp == AT_KICK && mdat == &mons[PM_MONK]
        && !rn2(10) && gy.youmonst.data->msize < MZ_HUGE) {
        if (!(Stun_resistance || wielding_artifact(ART_TEMPEST)))
            You("reel from %s powerful kick!", s_suffix(mon_nam(mtmp)));
        make_stunned((HStun & TIMEOUT) + (long) mhm.damage, TRUE);
        mhm.damage /= 2;
    }

    if ((Upolyd ? u.mh : u.uhp) < 1) {
        /* already dead? call rehumanize() or done_in_by() as appropriate */
        mdamageu(mtmp, 1);
        mhm.damage = 0;
    }

    /*  Negative armor class reduces damage done instead of fully protecting
     *  against hits.
     */
    if (mhm.damage && u.uac < 0) {
        mhm.damage -= rnd(-u.uac);
        if (mhm.damage < 1)
            mhm.damage = 1;
    }

    /* the knockback can dismount the hero onto a trap whose effect
       kills the attacker; the blow still lands, but a corpse casts no
       spell-hand riders and draws no passive response */

    /* burning hands spell - extra fire damage on unarmed touch attacks */
    if (!DEADMONSTER(mtmp) && mtmp->mburnhands && !MON_WEP(mtmp)
        && (mattk->aatyp == AT_CLAW || mattk->aatyp == AT_TUCH
            || mattk->aatyp == AT_WEAP)) {
        int firedmg = rnd(6) + 2;

        if (how_resistant(FIRE_RES) == 100) {
            shieldeff(u.ux, u.uy);
            pline("%s %s don't burn you!", s_suffix(Monnam(mtmp)),
                  makeplural(mbodypart(mtmp, HAND)));
        } else {
            pline("%s %s you with %s %s!", Monnam(mtmp),
                  vulnerable_to(&gy.youmonst, AD_FIRE) ? "severely burns"
                      : can_vaporize(gy.youmonst.data) ? "vaporizes part of"
                        : "burns",
                  mhis(mtmp), makeplural(mbodypart(mtmp, HAND)));
            firedmg = resist_reduce(firedmg, FIRE_RES);
            if (Half_spell_damage)
                firedmg /= 2;
            mhm.damage += firedmg;
            if ((int) mtmp->m_lev > rn2(20))
                (void) destroy_items(&gy.youmonst, AD_FIRE, firedmg);
            burn_away_slime();
        }
        mtmp->mburnhands--;
        if (!mtmp->mburnhands && canseemon(mtmp))
            pline("%s %s stop burning.", s_suffix(Monnam(mtmp)),
                  makeplural(mbodypart(mtmp, HAND)));
    }

    /* shocking grasp spell - extra shock damage on unarmed touch attacks */
    if (!DEADMONSTER(mtmp) && mtmp->mshockgrasp && !MON_WEP(mtmp)
        && (mattk->aatyp == AT_CLAW || mattk->aatyp == AT_TUCH
            || mattk->aatyp == AT_WEAP)) {
        int elecdmg = rnd(6) + 2;

        if (how_resistant(SHOCK_RES) == 100) {
            shieldeff(u.ux, u.uy);
            pline("%s %s don't shock you!", s_suffix(Monnam(mtmp)),
                  makeplural(mbodypart(mtmp, HAND)));
        } else {
            pline("%s %s you with %s %s!", Monnam(mtmp),
                  vulnerable_to(&gy.youmonst, AD_ELEC) ? "severely shocks"
                      : rn2(2) ? "jolts" : "shocks",
                  mhis(mtmp), makeplural(mbodypart(mtmp, HAND)));
            elecdmg = resist_reduce(elecdmg, SHOCK_RES);
            if (Half_spell_damage)
                elecdmg /= 2;
            mhm.damage += elecdmg;
            if ((int) mtmp->m_lev > rn2(20))
                (void) destroy_items(&gy.youmonst, AD_ELEC, elecdmg);
        }
        mtmp->mshockgrasp--;
        if (!mtmp->mshockgrasp && canseemon(mtmp))
            pline("%s %s stop crackling.", s_suffix(Monnam(mtmp)),
                  makeplural(mbodypart(mtmp, HAND)));
    }

    if (mhm.damage > 0) {
        /* [Half_physical_damage isn't applied to mhm.permdmg] */
        if (Half_physical_damage
            /* Mitre of Holiness, even if not currently blessed */
            || (Role_if(PM_CLERIC) && uarmh && is_quest_artifact(uarmh)
                && mon_hates_blessings(mtmp)))
            mhm.damage = (mhm.damage + 1) / 2;

        if (mhm.permdmg) { /* Death's life force drain */
            int lowerlimit, *hpmax_p;
            /*
             * Apply some of the damage to permanent hit points:
             *  polymorphed         100% against poly'd hpmax
             *  hpmax > 25*lvl      100% against normal hpmax
             *  hpmax > 10*lvl  50..100%
             *  hpmax >  5*lvl  25..75%
             *  otherwise        0..50%
             * Never reduces hpmax below 1 hit point per level.
             */
            mhm.permdmg = rn2(mhm.damage / 2 + 1);
            if (Upolyd || u.uhpmax > 25 * u.ulevel)
                mhm.permdmg = mhm.damage;
            else if (u.uhpmax > 10 * u.ulevel)
                mhm.permdmg += mhm.damage / 2;
            else if (u.uhpmax > 5 * u.ulevel)
                mhm.permdmg += mhm.damage / 4;

            if (Upolyd) {
                hpmax_p = &u.mhmax;
                /* [can't use gy.youmonst.m_lev] */
                lowerlimit = min((int) gy.youmonst.data->mlevel, u.ulevel);
            } else {
                hpmax_p = &u.uhpmax;
                lowerlimit = minuhpmax(1);
            }
            if (*hpmax_p - mhm.permdmg > lowerlimit)
                *hpmax_p -= mhm.permdmg;
            else if (*hpmax_p > lowerlimit)
                *hpmax_p = lowerlimit;
            /* else unlikely...
             * already at or below minimum threshold, do nothing to hpmax */
            disp.botl = TRUE;
        }

        /* name the wielded weapon in the death reason; mon_currwep is
           already NULL'd if the weapon eroded away during this attack */
        gk.killer_weapon = (mattk->aatyp == AT_WEAP) ? mon_currwep : 0;
        mdamageu(mtmp, mhm.damage);
        gk.killer_weapon = 0;
    }

    if (DEADMONSTER(mtmp)) {
        res = mhm.hitflags | M_ATTK_AGR_DIED;
    } else if (mhm.damage) {
        res = passiveum(olduasmon, mtmp, mattk);
    } else {
        res = M_ATTK_HIT;
    }
    stop_occupation();
    return res;
}

/* An interface for use when taking a blindfold off, for example,
 * to see if an engulfing attack should immediately take affect, like
 * a passive attack. TRUE if engulfing blindness occurred */
boolean
gulp_blnd_check(void)
{
    struct attack *mattk;

    if (!Blinded && u.uswallow
        && (mattk = attacktype_fordmg(u.ustuck->data, AT_ENGL, AD_BLND))
        && can_blnd(u.ustuck, &gy.youmonst, mattk->aatyp, (struct obj *) 0)) {
        ++u.uswldtim; /* compensate for gulpmu change */
        (void) gulpmu(u.ustuck, mattk);
        return TRUE;
    }
    return FALSE;
}

/* monster swallows you, or damage if already swallowed (u.uswallow != 0) */
staticfn int
gulpmu(struct monst *mtmp, struct attack *mattk)
{
    struct trap *t = t_at(u.ux, u.uy);
    int tmp = d((int) mattk->damn, (int) mattk->damd);
    int tim_tmp;
    struct obj *otmp2, *nextobj;
    int i;
    char kbuf[BUFSZ]; /* death reason */
    boolean physical_damage = FALSE, total_digestion = FALSE;
    /* for tracking if this is the first engulf */
    boolean old_uswallow = u.uswallow;

    if (!u.uswallow) { /* swallows you */
        int omx = mtmp->mx, omy = mtmp->my;

        if (!engulf_target(mtmp, &gy.youmonst))
            return M_ATTK_MISS;
        if ((t && is_pit(t->ttyp)) && sobj_at(BOULDER, u.ux, u.uy))
            return M_ATTK_MISS;
        if (failed_grab(mtmp, &gy.youmonst, mattk))
            return M_ATTK_MISS;

        if (Punished)
            unplacebc(); /* ball&chain go away */
        remove_monster(omx, omy);
        mtmp->mtrapped = 0; /* no longer on old trap */
        mtmp->mentangled = 0; /* no longer entangled */
        place_monster(mtmp, u.ux, u.uy);
        set_ustuck(mtmp);
        /* an engulfer that struck from hiding is hidden no longer */
        if (mtmp->mundetected)
            mtmp->mundetected = 0;
        newsym(mtmp->mx, mtmp->my);
        /* 5.0: dismount for all engulfers, not just for purple worms */
        if (u.usteed) {
            char buf[BUFSZ];

            Strcpy(buf, mon_nam(u.usteed));
            urgent_pline("%s %s forward and plucks you off %s!",
                         Some_Monnam(mtmp),
                         /* 't', purple 'w' */
                         is_animal(mtmp->data) ? "lunges"
                           /* 'v', air 'E' */
                           : is_whirly(mtmp->data) ? "whirls"
                             /* none (some 'v', already whirling) */
                             : unsolid(mtmp->data) ? "flows"
                               /* ochre 'j', Juiblex */
                               : amorphous(mtmp->data) ? "oozes"
                                 /* none (all AT_ENGL are already covered) */
                                 : "surges",
                         buf);
            dismount_steed(DISMOUNT_ENGULFED);
        } else {
            urgent_pline("%s %s!", Monnam(mtmp),
                         digests(mtmp->data) ? "swallows you whole"
                         : enfolds(mtmp->data) ? "folds itself around you"
                           : "engulfs you");
        }
        stop_occupation();
        reset_occupations(); /* behave as if you had moved */

        if (u.utrap) {
            You("are released from the %s!",
                (u.utraptype == TT_WEB) ? "web" : "trap");
            /* a buried ball's chain waiting behind the hold lets go
               with it, like a chain that held the hero itself */
            u.utether_pending = 0;
            reset_utrap(FALSE);
        }

        i = number_leashed();
        if (i > 0) {
            const char *s = (i > 1) ? "leashes" : "leash";

            pline_The("%s %s loose.", s, vtense(s, "snap"));
            unleash_all();
        }

        if (touch_petrifies(gy.youmonst.data)
            && !(resists_ston(mtmp) || defended(mtmp, AD_STON))) {
            /* put the attacker back where it started;
               the resulting statue will end up there
               [note: if poly'd hero could ride or non-poly'd hero could
               acquire touch_petrifies() capability somehow, this code
               would need to deal with possibility of steed having taken
               engulfer's previous spot when hero was forcibly dismounted] */
            remove_monster(mtmp->mx, mtmp->my); /* u.ux,u.uy */
            place_monster(mtmp, omx, omy);
            minstapetrify(mtmp, TRUE);
            /* normally unstuck() would do this, but we're not
               fully swallowed yet so that won't work here */
            if (Punished)
                placebc();
            set_ustuck((struct monst *) 0);
            return (!DEADMONSTER(mtmp)) ? M_ATTK_MISS : M_ATTK_AGR_DIED;
        }

        display_nhwindow(WIN_MESSAGE, FALSE);
        vision_recalc(2); /* hero can't see anything */
        u.uswallow = 1;
        /* for digestion, shorter time is more dangerous;
           for other swallowings, longer time means more
           chances for the swallower to attack */
        if (mattk->adtyp == AD_DGST) {
            /* having good armor & high constitution makes
               it take longer for you to be digested, but
               you'll end up trapped inside for longer too */
            tim_tmp = (int)ACURR(A_CON) + 10 - (int)u.uac + rn2(20);
            if (tim_tmp < 0)
                tim_tmp = 0;
            tim_tmp /= (int) mtmp->m_lev;
            tim_tmp += 3;
        } else {
            /* higher level attacker takes longer to eject hero */
            tim_tmp = rnd((int) mtmp->m_lev + 10 / 2);
        }
        /* u.uswldtim always set > 1 */
        u.uswldtim = (unsigned) ((tim_tmp < 2) ? 2 : tim_tmp);
        swallowed(1); /* update the map display, shows hero swallowed */
        if (!flaming(mtmp->data)) {
            for (otmp2 = gi.invent; otmp2; otmp2 = nextobj) {
                nextobj = otmp2->nobj;
                (void) snuff_lit(otmp2, FALSE);
            }
        }
    }

    if (mtmp != u.ustuck)
        return M_ATTK_MISS;
    if (Punished) {
        /* ball&chain are in limbo while swallowed; update their internal
           location to be at swallower's spot */
        if (uchain->where == OBJ_FREE)
            uchain->ox = mtmp->mx, uchain->oy = mtmp->my;
        if (uball->where == OBJ_FREE)
            uball->ox = mtmp->mx, uball->oy = mtmp->my;
    }
    if (u.uswldtim > 0)
        u.uswldtim -= 1;

    switch (mattk->adtyp) {
    case AD_DGST:
        physical_damage = TRUE;
        if (Slow_digestion) {
            /* Messages are handled below */
            u.uswldtim = 0;
            tmp = 0;
        } else if (u.uswldtim == 0) {
            pline("%s totally digests you!", Monnam(mtmp));
            /* the whole of the pool mdamageu() will charge */
            tmp = Upolyd ? u.mh : u.uhp;
            total_digestion = TRUE;
            if (Half_physical_damage)
                tmp *= 2; /* sorry */
        } else {
            pline("%s%s digests you!", Monnam(mtmp),
                  (u.uswldtim == 2) ? " thoroughly"
                                    : (u.uswldtim == 1) ? " utterly" : "");
            exercise(A_STR, FALSE);
        }
        break;
    case AD_PHYS:
        physical_damage = TRUE;
        if (mtmp->data == &mons[PM_FOG_CLOUD]) {
            You("are laden with moisture and %s",
                flaming(gy.youmonst.data)
                    ? "are smoldering out!"
                    : Breathless ? "find it mildly uncomfortable."
                                 : Amphibious
                                       ? "feel comforted."
                                       : "can barely breathe!");
            if ((Amphibious || Breathless) && !flaming(gy.youmonst.data))
                tmp = 0;
        } else {
            You("are %s!", enfolds(mtmp->data) ? "being squashed"
                                               : "pummeled with debris");
            exercise(A_STR, FALSE);
        }
        break;
    case AD_WRAP:
        /* Initially pulled from GruntHack, and then improved upon by
           aosdict for xNetHack (see git commit ee808b): AD_WRAP is used
           because there's no specific suffocation attack, but it's used
           for other suffocation-y things like drowning attacks.
           Generally, only give a message if this is the first engulf, not
           a subsequent attack when already engulfed */
        if (Breathless
            || (Amphibious && (mtmp->data == &mons[PM_WATER_ELEMENTAL]
                               || mtmp->data == &mons[PM_SEA_DRAGON]))) {
            if (!old_uswallow) {
                if (!(HMagical_breathing || EMagical_breathing)) {
                    /* test this one first, in case breathless and also
                       wearing magical breathing */
                    if (Amphibious)
                        You("can still breathe, though.");
                    else
                        You("can't breathe, but you don't need to.");
                } else {
                    You("can still breathe, though.");
                    if (uamul && uamul->otyp == AMULET_OF_MAGICAL_BREATHING)
                        makeknown(AMULET_OF_MAGICAL_BREATHING);
                }
            }
            tmp = 0;
        } else if (!Strangled) {
            if (!old_uswallow)
                pline("It's impossible to breathe in here!");
            /* xNetHack sets this timer for 5, GruntHack had it for 3;
               we'll meet in the middle. nh_timeout()'s choke_dialogue()
               narrates the countdown from the second text onward */
            Strangled = 4L;
            tmp = 0;
        }
        if (mtmp->data == &mons[PM_WATER_ELEMENTAL]
            || mtmp->data == &mons[PM_SEA_DRAGON])
            water_damage_chain(gi.invent, FALSE, rnd(3), FALSE);
        break;
    case AD_ACID:
        if (Acid_resistance) {
            You("are covered with a seemingly harmless goo.");
            /* NB: the monst[un]seesu calls in gulpmu are no-ops since the
               hero must be currently swallowed for the attack to hit... */
            monstseesu(M_SEEN_ACID);
            tmp = 0;
        } else {
            if (Hallucination)
                pline("Ouch!  You've been slimed!");
            else
                You("are covered in slime!  It burns!");
            exercise(A_STR, FALSE);
            monstunseesu(M_SEEN_ACID);
        }
        break;
    case AD_BLND:
        if (can_blnd(mtmp, &gy.youmonst, mattk->aatyp, (struct obj *) 0)) {
            if (!Blind) {
                long was_blinded = Blinded;

                if (!Blinded)
                    You_cant("see in here!");
                make_blinded((long) tmp, FALSE);
                if (!was_blinded && !Blind)
                    Your1(vision_clears);
            } else
                /* keep him blind until disgorged */
                incr_itimeout(&HBlinded, 1L);
        }
        tmp = 0;
        break;
    case AD_ELEC:
        if (!mtmp->mcan && rn2(2)) {
            pline_The("air around you crackles with electricity.");
            if (how_resistant(SHOCK_RES) == 100) {
                shieldeff(u.ux, u.uy);
                You("seem unhurt.");
                monstseesu(M_SEEN_ELEC);
                ugolemeffects(AD_ELEC, tmp);
                tmp = 0;
            } else {
                tmp = resist_reduce(tmp, SHOCK_RES);
                monstunseesu(M_SEEN_ELEC);
            }
        } else
            tmp = 0;
        break;
    case AD_COLD:
        if (!mtmp->mcan && rn2(2)) {
            if (how_resistant(COLD_RES) == 100) {
                shieldeff(u.ux, u.uy);
                You_feel("mildly chilly.");
                monstseesu(M_SEEN_COLD);
                ugolemeffects(AD_COLD, tmp);
                tmp = 0;
            } else {
                You("are freezing to death!");
                tmp = resist_reduce(tmp, COLD_RES);
                monstunseesu(M_SEEN_COLD);
            }
        } else
            tmp = 0;
        break;
    case AD_FIRE:
        if (!mtmp->mcan && rn2(2)) {
            if (how_resistant(FIRE_RES) == 100) {
                shieldeff(u.ux, u.uy);
                You_feel("mildly hot.");
                monstseesu(M_SEEN_FIRE);
                ugolemeffects(AD_FIRE, tmp);
                tmp = 0;
            } else {
                You("are %s!", on_fire(&gy.youmonst, ON_FIRE_ENGULF));
                tmp = resist_reduce(tmp, FIRE_RES);
                monstunseesu(M_SEEN_FIRE);
            }
            burn_away_slime();
        } else
            tmp = 0;
        break;
    case AD_DISE:
        if (!diseasemu(mtmp))
            tmp = 0;
        break;
    case AD_DREN:
        /* AC magic cancellation doesn't help when engulfed */
        if (!mtmp->mcan && rn2(4)) /* 75% chance */
            drain_en(tmp, FALSE);
        tmp = 0;
        break;
    case AD_DISN:
        if (!mtmp->mcan && rn2(2) && u.uswldtim < 2) {
            pline_The("air around you shimmers with antiparticles.");
            if (how_resistant(DISINT_RES) == 100) {
                shieldeff(u.ux, u.uy);
                You_feel("mildly tickled.");
                tmp = 0;
                break;
            } else if (how_resistant(DISINT_RES) >= 50) {
                You("aren't disintegrated, but that hurts!");
                tmp = resist_reduce(tmp, DISINT_RES);
                if (tmp) {
                    gm.mswallower = mtmp;
                    mdamageu(mtmp, tmp);
                    gm.mswallower = 0;
                    stop_occupation();
                }
                /* damage already applied; don't let the call
                   after the switch apply it a second time */
                tmp = 0;
                break;
            } else if (how_resistant(DISINT_RES) < 50) {
                tmp = resist_reduce(tmp, DISINT_RES);
                if (tmp) {
                    gm.mswallower = mtmp;
                    mdamageu(mtmp, tmp);
                    gm.mswallower = 0;
                    stop_occupation();
                }
                /* damage already applied; don't let the call
                   after the switch apply it a second time */
                tmp = 0;
                if (uarms) {
                    /* destroy shield; other possessions are safe */
                    if (disintegrate_arm(uarms) || uarms->oartifact)
                        break;
                } else if (uarm) {
                    /* destroy suit; if present, cloak goes too */
                    if (uarmc)
                        (void) disintegrate_arm(uarmc);
                    if (disintegrate_arm(uarm) || uarm->oartifact)
                        break;
                    Your("%s remains intact, but you do not!", xname(uarm));
                }
                /* fall through. not having enough disintegration
                   resistance can still get you disintegrated, as can
                   wearing a suit that cannot be sacrificed to the blast */
            }
            /* no shield or suit, you're dead; wipe out cloak
               and/or shirt in case of life-saving or bones */
            if (uarmc)
                (void) disintegrate_arm(uarmc);
            if (uarmu)
                (void) disintegrate_arm(uarmu);

            You("are disintegrated!");
            /* when killed by disintegration, don't leave a corpse */
            u.ugrave_arise = -3;
            svk.killer.format = NO_KILLER_PREFIX;
            Sprintf(svk.killer.name, "disintegrated by %s",
                    mon_killer_name(kbuf, mtmp));
            /* its kill, with nothing left to rise */
            set_monster_killer(mtmp, FALSE);
            done(DISINTEGRATED);
        } else {
            tmp = 0;
        }
        break;
    default:
        physical_damage = TRUE;
        tmp = 0;
        break;
    }

    if (physical_damage) {
        /* same damage reduction for AC as in hitmu; total digestion
           is the whole of the current pool by design and armor does
           not spare any of it */
        if (u.uac < 0 && !total_digestion)
            tmp -= rnd(-u.uac);
        if (tmp < 0)
            tmp = 1;

        tmp = Maybe_Half_Phys(tmp);
    }

    gm.mswallower = mtmp; /* match gulpmm() */
    mdamageu(mtmp, tmp);
    gm.mswallower = 0;
    if (tmp)
        stop_occupation();

    if (!u.uswallow) {
        ; /* life-saving has already expelled swallowed hero */
    } else if (touch_petrifies(gy.youmonst.data)
               && !(resists_ston(mtmp) || defended(mtmp, AD_STON))) {
        pline("%s very hurriedly %s you!", Monnam(mtmp),
              digests(mtmp->data) ? "regurgitates"
              : enfolds(mtmp->data) ? "releases"
                : "expels");
        expels(mtmp, mtmp->data, FALSE);
    } else if (!u.uswldtim || gy.youmonst.data->msize >= MZ_HUGE) {
        /* As of 3.6.2: u.uswldtim used to be set to 0 by life-saving but it
           expels now so the !u.uswldtim case is no longer possible;
           however, polymorphing into a huge form while already
           swallowed is still possible */
        You("get %s!", digests(mtmp->data) ? "regurgitated"
                       : enfolds(mtmp->data) ? "released"
                         : "expelled");
        if (flags.verbose
            && (digests(mtmp->data) && Slow_digestion))
            pline("Obviously %s doesn't like your taste.", mon_nam(mtmp));
        expels(mtmp, mtmp->data, FALSE);
    }
    return M_ATTK_HIT;
}

/* monster explodes in your face */
staticfn int
explmu(
    struct monst *mtmp,
    struct attack *mattk,
    boolean ufound)
{
    boolean kill_agr = TRUE;
    boolean not_affected;
    int tmp;

    if (mtmp->mcan)
        return M_ATTK_MISS;

    tmp = d((int) mattk->damn, (int) mattk->damd);
    not_affected = defended(mtmp, (int) mattk->adtyp);

    if (!ufound) {
        pline("%s explodes at a spot in %s!",
              canseemon(mtmp) ? Monnam(mtmp) : "It",
              is_waterwall(mtmp->mux,mtmp->muy) ? "empty water"
                                                : "thin air");
    } else {
        hitmsg(mtmp, mattk);
    }

    switch (mattk->adtyp) {
    case AD_COLD:
    case AD_FIRE:
    case AD_ELEC:
    case AD_ACID:
        mon_explodes(mtmp, mattk);
        if (!DEADMONSTER(mtmp))
            kill_agr = FALSE; /* lifesaving? */
        break;
    case AD_BLND:
        not_affected = resists_blnd(&gy.youmonst);
        if (ufound && !not_affected) {
            /* sometimes you're affected even if it's invisible */
            if (mon_visible(mtmp) || (rnd(tmp /= 2) > u.ulevel)) {
                You("are blinded by a blast of light!");
                make_blinded((long) tmp, FALSE);
                if (!Blind)
                    Your1(vision_clears);
            } else if (flags.verbose)
                You("get the impression it was not terribly bright.");
        }
        /* exploding light can damage light haters whether they are
           blind or not; drow heroes hate the light too */
        if (ufound
            && (hates_light(gy.youmonst.data)
                || maybe_polyd(is_drow(gy.youmonst.data),
                               Race_if(PM_DROW)))) {
            pline("Ow, that light hurts!");
            not_affected = FALSE;
            mdamageu(mtmp, rnd(5));
        }
        break;
    case AD_HALU:
        not_affected |= Blind || (u.umonnum == PM_BLACK_LIGHT
                                  || u.umonnum == PM_VIOLET_FUNGUS
                                  || dmgtype(gy.youmonst.data, AD_STUN));
        if (ufound && !not_affected) {
            boolean chg;
            if (!Hallucination)
                You("are caught in a blast of kaleidoscopic light!");
            /* avoid hallucinating the black light as it dies */
            mondead(mtmp);    /* remove it from map now */
            kill_agr = FALSE; /* already killed (maybe lifesaved) */
            chg =
                make_hallucinated(HHallucination + (long) tmp, FALSE, 0L);
            You("%s.", chg ? "are freaked out" : "seem unaffected");
        }
        break;
    default:
        impossible("unknown exploder damage type %d", mattk->adtyp);
        break;
    }
    if (not_affected) {
        You("seem unaffected by it.");
        ugolemeffects((int) mattk->adtyp, tmp);
    }
    if (kill_agr && !DEADMONSTER(mtmp))
        mondead(mtmp);
    wake_nearto(mtmp->mx, mtmp->my, 7 * 7);
    return (!DEADMONSTER(mtmp)) ? M_ATTK_MISS : M_ATTK_AGR_DIED;
}

/* monster gazes at you */
int
gazemu(struct monst *mtmp, struct attack *mattk)
{
    static const char *const reactions[] = {
        "confused",              /* [0] */
        "stunned",               /* [1] */
        "puzzled",   "dazzled",  /* [2,3] */
        "irritated", "inflamed", /* [4,5] */
        "tired",                 /* [6] */
        "dulled",                /* [7] */
        "chilly",                /* [8] */
        "lackluster",            /* [9] */
    };
    int react = -1;
    char kbuf[BUFSZ]; /* death reasons */
    boolean is_medusa, reflectable,
            cancelled = (mtmp->mcan != 0), already = FALSE,
            mcanseeu = (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my)
                        && mtmp->mcansee);

    is_medusa = (mtmp->data == &mons[PM_MEDUSA]);
    reflectable = (Reflecting && couldsee(mtmp->mx, mtmp->my) && is_medusa);
    /* assumes that hero has to see monster's gaze in order to be
       affected, rather than monster just having to look at hero;
       Unaware:  asleep or unconscious => not blind but won't see;
       when hallucinating, hero's brain doesn't register what
       it's seeing correctly so the gaze is usually ineffective
       [this could be taken a lot farther and select a gaze effect
       appropriate to what's currently being displayed, giving
       ordinary monsters a gaze attack when hero thinks he or she
       is facing a gazing creature, but let's not go that far...] */
    if ((Hallucination && rn2(4)) || (Unaware && !reflectable))
        cancelled = TRUE;

    switch (mattk->adtyp) {
    case AD_STON:
        if (cancelled || !mtmp->mcansee) {
            if (!canseemon(mtmp))
                break; /* silently */
            if (Unaware) { /* can't see attacker even though not blind */
                react = is_medusa ? 4 : 2; /* irritated or puzzled */
                break;
            }
            if (is_medusa && Hallucination && !rn2(3))
                pline("Someone seems overdue for a serpent cut.");
            else
                pline_mon(mtmp, "%s %s.", Monnam(mtmp),
                      (is_medusa && mtmp->mcan)
                          ? "doesn't look all that ugly"
                          : "gazes ineffectually");
            break;
         }
        if (reflectable) {
            /* hero has line of sight to Medusa and she's not blind */
            boolean useeit = canseemon(mtmp);

            if (useeit
                && ureflects("%s gaze is reflected by your %s.",
                             s_suffix(Monnam(mtmp))))
                monstseesu(M_SEEN_REFL);
            if (dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) > 8) {
                if (useeit)
                    pline("%s reflection is too far away for %s to notice.",
                          s_suffix(Monnam(mtmp)), mhis(mtmp));
                break;
            }
            if (mon_reflects(mtmp, !useeit ? (char *) 0
                                  : "The gaze is reflected away by %s %s!"))
                break;
            if (!m_canseeu(mtmp)) { /* probably you're invisible */
                if (useeit)
                    pline(
                      "%s doesn't seem to notice that %s gaze was reflected.",
                          Monnam(mtmp), mhis(mtmp));
                break;
            }
            if (!rn2(12)) {
                if (useeit)
                    pline_mon(mtmp, "%s is turned to stone!", Monnam(mtmp));
                gs.stoned = TRUE;
                killed(mtmp);
            } else {
                if (useeit)
                    pline_mon(mtmp,
                              "%s %s %s eyes from %s reflected gaze"
                              " just in time!",
                              Monnam(mtmp), rn2(2) ? "shields" : "covers",
                              mhis(mtmp), mhis(mtmp));
                break;
            }

            if (!DEADMONSTER(mtmp))
                break;
            return M_ATTK_AGR_DIED;
        }
        if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my)
            && !Unaware && !rn2(3)) {
            You("meet %s petrifying gaze!", s_suffix(mon_nam(mtmp)));
            stop_occupation();
            if (Stone_resistance
                || (poly_when_stoned(gy.youmonst.data)
                    && polymon(stoned_form(gy.youmonst.data)))) {
                You("are unaffected by %s gaze.", s_suffix(mon_nam(mtmp)));
                break;
            }
            if (mtmp->data == &mons[PM_BEHOLDER]
                || mtmp->data == &mons[PM_TAL_GATH]) {
                /* The EotO can afford the player some protection when worn */
                if (ublindf
                    && ublindf->oartifact == ART_EYES_OF_THE_OVERWORLD) {
                    int dmg = d(4, 6);

                    pline("%s partially protect you from %s petrifying "
                          "gaze.  That hurts!",
                          An(bare_artifactname(ublindf)),
                          s_suffix(mon_nam(mtmp)));
                    if (dmg)
                        mdamageu(mtmp, dmg);
                } else if (!Stoned) {
                    make_stoned(5L, (char *) 0, KILLED_BY,
                                mon_killer_name(kbuf, mtmp));
                    return M_ATTK_HIT;
                }
            } else if (is_medusa) {
                if (ublindf
                    && ublindf->oartifact == ART_EYES_OF_THE_OVERWORLD)
                    pline("%s gaze is too powerful for %s to resist!",
                          s_suffix(Monnam(mtmp)), bare_artifactname(ublindf));
                urgent_pline("You turn to stone...");
                svk.killer.format = KILLED_BY;
                Strcpy(svk.killer.name, mon_killer_name(kbuf, mtmp));
                /* its kill; a statue doesn't rise */
                set_monster_killer(mtmp, FALSE);
                done(STONING);
            }
        }
        break;
    case AD_CONF:
        if (mcanseeu && !mtmp->mspec_used && rn2(5)) {
            if (cancelled) {
                react = 0; /* "confused" */
                already = (mtmp->mconf != 0);
            } else if (ublindf
                       && ublindf->oartifact == ART_EYES_OF_THE_OVERWORLD) {
                if (!rn2(4))
                    pline("%s protect you from %s confusing gaze.",
                          An(bare_artifactname(ublindf)),
                          s_suffix(mon_nam(mtmp)));
                break;
            } else {
                int conf = d(3, 4);

                mtmp->mspec_used = mtmp->mspec_used + (conf + rn2(6));
                if (!Confusion)
                    pline_mon(mtmp, "%s gaze confuses you!",
                              s_suffix(Monnam(mtmp)));
                else
                    You("are getting more and more confused.");
                make_confused(HConfusion + conf, FALSE);
                stop_occupation();
            }
        }
        break;
    case AD_STUN:
        if (mcanseeu && !mtmp->mspec_used && rn2(5)) {
            if (cancelled) {
                react = 1; /* "stunned" */
                already = (mtmp->mstun != 0);
            } else if (ublindf
                       && ublindf->oartifact == ART_EYES_OF_THE_OVERWORLD) {
                if (!rn2(4))
                    pline("%s protect you from %s stunning gaze.",
                          An(bare_artifactname(ublindf)),
                          s_suffix(mon_nam(mtmp)));
                break;
            } else {
                int stun = d(2, 6);

                mtmp->mspec_used = mtmp->mspec_used + (stun + rn2(6));
                pline_mon(mtmp, "%s stares piercingly at you!", Monnam(mtmp));
                make_stunned((HStun & TIMEOUT) + (long) stun, TRUE);
                stop_occupation();
            }
        }
        break;
    case AD_BLND:
        if (canseemon(mtmp) && !resists_blnd(&gy.youmonst)
            && mdistu(mtmp) <= BOLT_LIM * BOLT_LIM) {
            if (cancelled) {
                react = rn1(2, 2); /* "puzzled" || "dazzled" */
                already = (mtmp->mcansee == 0);
                /* Archons gaze every round; we don't want cancelled ones
                   giving the "seems puzzled/dazzled" message that often */
                if (mtmp->mcan && mtmp->data == &mons[PM_ARCHON] && rn2(5))
                    react = -1;
            } else {
                int blnd = d((int) mattk->damn, (int) mattk->damd);

                You("are blinded by %s radiance!", s_suffix(mon_nam(mtmp)));
                make_blinded((long) blnd, FALSE);
                stop_occupation();
                /* not blind at this point implies you're wearing
                   the Eyes of the Overworld; make them block this
                   particular stun attack too */
                if (!Blind) {
                    Your1(vision_clears);
                } else {
                    long oldstun = (HStun & TIMEOUT), newstun = (long) rnd(3);

                    /* we don't want to increment stun duration every time
                       or sighted hero will become incapacitated */
                    make_stunned(max(oldstun, newstun), TRUE);
                }
            }
        }
        break;
    case AD_FIRE:
        if (mcanseeu && !mtmp->mspec_used && rn2(5)) {
            if (cancelled) {
                react = rn1(2, 4); /* "irritated" || "inflamed" */
            } else {
                int dmg = d((int) mattk->damn, (int) mattk->damd),
                    orig_dmg = dmg, lev = (int) mtmp->m_lev;

                pline_mon(mtmp, "%s attacks you with a fiery gaze!",
                          Monnam(mtmp));
                stop_occupation();
                dmg = resist_reduce(dmg, FIRE_RES);
                if (how_resistant(FIRE_RES) == 100 || Underwater) {
                    shieldeff(u.ux, u.uy);
                    if (Underwater)
                        pline_The("fire quickly fizzles out.");
                    else
                        pline_The("fire feels mildly hot.");
                    monstseesu(M_SEEN_FIRE);
                    ugolemeffects(AD_FIRE, d(12, 6));
                    dmg = 0;
                } else {
                    monstunseesu(M_SEEN_FIRE);
                }
                if (!Underwater) {
                    burn_away_slime();
                    if (lev > rn2(20))
                        (void) burnarmor(&gy.youmonst);
                    if (lev > rn2(20)) {
                        (void) destroy_items(&gy.youmonst, AD_FIRE, orig_dmg);
                        ignite_items(gi.invent);
                    }
                }
                if (dmg)
                    mdamageu(mtmp, dmg);
            }
        }
        break;
    case AD_COLD:
        if (mcanseeu && !mtmp->mspec_used && rn2(5)) {
            if (cancelled) {
                react = 8; /* "chilly" */
            } else {
                int dmg = d((int) mattk->damn, (int) mattk->damd),
                    orig_dmg = dmg, lev = (int) mtmp->m_lev;

                pline_mon(mtmp, "%s attacks you with a chilling gaze!",
                          Monnam(mtmp));
                stop_occupation();
                dmg = resist_reduce(dmg, COLD_RES);
                if (how_resistant(COLD_RES) == 100) {
                    shieldeff(u.ux, u.uy);
                    pline_The("chilling gaze feels mildly cool.");
                    monstseesu(M_SEEN_COLD);
                    ugolemeffects(AD_COLD, d(12, 6));
                    dmg = 0;
                } else {
                    monstunseesu(M_SEEN_COLD);
                }
                if (lev > rn2(20))
                    (void) destroy_items(&gy.youmonst, AD_COLD, orig_dmg);
                if (dmg)
                    mdamageu(mtmp, dmg);
            }
        }
        break;
    case AD_LUCK:
        if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my) && mtmp->mcansee
            && !mtmp->mspec_used && rn2(4) && !cancelled) {
            pline_mon(mtmp, "%s glares ominously at you!", Monnam(mtmp));
            mtmp->mspec_used = mtmp->mspec_used + 3 + rn2(8);

            if (uwep && uwep->otyp == MIRROR && uwep->blessed) {
                pline_mon(mtmp, "%s sees its own glare in your mirror.",
                          Monnam(mtmp));
                pline_mon(mtmp, "%s is cancelled!", Monnam(mtmp));
                mtmp->mcan = 1;
                monflee(mtmp, 0, FALSE, TRUE);
            } else {
                change_luck(-1);
                pline("You don't feel as lucky as before.");
            }
            stop_occupation();
        }
        break;
    case AD_SLEE:
        if (mcanseeu && gm.multi >= 0 && !rn2(5)) {
            if (!cancelled)
                You("meet %s slumbering gaze.", s_suffix(mon_nam(mtmp)));
            if (cancelled) {
                react = 6;                      /* "tired" */
                already = (mtmp->mfrozen != 0); /* can't happen... */
                break;
            } else if (how_resistant(SLEEP_RES) == 100) {
                You("yawn.");
                monstseesu(M_SEEN_SLEEP);
                break;
            } else if (ublindf
                       && ublindf->oartifact == ART_EYES_OF_THE_OVERWORLD) {
                if (!rn2(4))
                    pline("%s protect you from %s slumbering gaze.",
                          An(bare_artifactname(ublindf)),
                          s_suffix(mon_nam(mtmp)));
                break;
            } else if (how_resistant(SLEEP_RES) < 100) {
                int dmg = d((int) mattk->damn, (int) mattk->damd);

                monstunseesu(M_SEEN_SLEEP);
                fall_asleep(-resist_reduce(dmg, SLEEP_RES), TRUE);
                pline("%s gaze makes you very sleepy...",
                      s_suffix(Monnam(mtmp)));
                break;
            }
        }
        break;
    case AD_SLOW:
        if (mcanseeu && !rn2(4)) {
            if (!cancelled)
                You("meet %s lethargic gaze.", s_suffix(mon_nam(mtmp)));
            if (cancelled) {
                react = 7; /* "dulled" */
                already = (mtmp->mspeed == MSLOW);
                break;
            /* The EotO can afford the player some protection when worn */
            } else if (ublindf
                       && ublindf->oartifact == ART_EYES_OF_THE_OVERWORLD) {
                if (!rn2(4))
                    pline("%s protect you from %s lethargic gaze.",
                          An(bare_artifactname(ublindf)),
                          s_suffix(mon_nam(mtmp)));
                break;
            } else if (!Slow && !(defended(&gy.youmonst, AD_SLOW)
                                  || resists_slow(gy.youmonst.data))) {
                u_slow_down();
                stop_occupation();
                break;
            }
        }
        break;
    case AD_DISN:
        if (mcanseeu && gm.multi >= 0 && !rn2(5)) {
            pline_mon(mtmp, "%s turns %s towards you%s", Monnam(mtmp),
                      cancelled ? "an impotent leer" : "a destructive gaze",
                      cancelled ? "." : "!");
            if (cancelled) {
                break;
            } else if (how_resistant(DISINT_RES) == 100) {
                pline("You bask in its %s aura.", hcolor(NH_BLACK));
                monstseesu(M_SEEN_DISINT);
                stop_occupation();
                break;
            } else if (how_resistant(DISINT_RES) >= 50) {
                int dmg = d((int) mattk->damn, (int) mattk->damd);

                You("aren't disintegrated, but that really hurts!");
                dmg = resist_reduce(dmg, DISINT_RES);
                if (ublindf
                    && ublindf->oartifact == ART_EYES_OF_THE_OVERWORLD)
                    dmg /= 2;
                if (dmg)
                    mdamageu(mtmp, dmg);
                break;
            /* The EotO can afford the player some protection when worn */
            } else if (ublindf
                       && ublindf->oartifact == ART_EYES_OF_THE_OVERWORLD) {
                int dmg = d((int) mattk->damn, (int) mattk->damd);

                pline("%s partially protect you.  That stings!",
                      An(bare_artifactname(ublindf)));
                dmg /= 2;
                if (dmg)
                    mdamageu(mtmp, dmg);
                break;
            } else {
                if (uarms) {
                    /* destroy shield; other possessions are safe */
                    if (disintegrate_arm(uarms) || uarms->oartifact)
                        break;
                } else if (uarm) {
                    /* destroy suit; if present, cloak goes too */
                    if (uarmc)
                        (void) disintegrate_arm(uarmc);
                    if (disintegrate_arm(uarm) || uarm->oartifact)
                        break;
                    Your("%s remains intact, but you do not!", xname(uarm));
                }
                /* no shield or suit, you're dead; wipe out cloak
                   and/or shirt in case of life-saving or bones */
                if (uarmc)
                    (void) disintegrate_arm(uarmc);
                if (uarmu)
                    (void) disintegrate_arm(uarmu);

                You("are disintegrated!");
                /* when killed by disintegration, don't leave a corpse */
                u.ugrave_arise = -3;
                svk.killer.format = NO_KILLER_PREFIX;
                Sprintf(svk.killer.name, "disintegrated by %s",
                        mon_killer_name(kbuf, mtmp));
                /* its kill, with nothing left to rise */
                set_monster_killer(mtmp, FALSE);
                done(DISINTEGRATED);
            }
        }
        break;
    case AD_CNCL:
        if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my)
            && mtmp->mcansee && !rn2(3)) {
            if (!cancelled)
                You("meet %s strange gaze.", s_suffix(mon_nam(mtmp)));
            if (cancelled) {
                react = 9; /* "lackluster" */
                already = (mtmp->mcan != 0);
                break;
            /* the EotO can afford the player some protection when worn */
            } else if (ublindf
                       && ublindf->oartifact == ART_EYES_OF_THE_OVERWORLD) {
                int dmg = d((int) mattk->damn, (int) mattk->damd);

                pline("%s partially protect you from %s strange gaze.  Ouch!",
                      An(bare_artifactname(ublindf)), s_suffix(mon_nam(mtmp)));
                dmg = (dmg + 1) / 2;
                if (dmg)
                    mdamageu(mtmp, dmg);
                break;
            } else {
                int dmg = d((int) mattk->damn, (int) mattk->damd);

                (void) cancel_monst(&gy.youmonst, (struct obj *) 0, FALSE,
                                    TRUE, FALSE);
                if (dmg)
                    mdamageu(mtmp, dmg);
                break;
            }
        }
        break;
    case AD_DETH:
        /* currently only Vecna has the gaze of death */
        if (mcanseeu && rn2(4)) {
            int dmg = d((int) mattk->damn, (int) mattk->damd);
            int permdmg = 0;

            if (mtmp->data == &mons[PM_VECNA])
                You("meet %s deadly gaze!", s_suffix(mon_nam(mtmp)));
            stop_occupation();
            if (Death_resistance || immune_death_magic(gy.youmonst.data)) {
                shieldeff(u.ux, u.uy);
                monstseesu(M_SEEN_DEATH);
                You("are unaffected by death magic.");
                break;
            }
            switch (rn2(20)) {
            case 19:
            case 18:
            case 17:
                if (!Antimagic) {
                    (void) death_inflicted_by(kbuf, "a gaze of death",
                                              mtmp);
                    svk.killer.format = KILLED_BY;
                    Strcpy(svk.killer.name, kbuf);
                    set_monster_killer(mtmp, TRUE);
                    done(DIED);
                    dmg = 0;
                    break;
                }
                FALLTHROUGH;
                /*FALLTHRU*/
            default: /* case 16: ... case 2: */
                You_feel("your life force draining away...");
                permdmg = 1; /* actual drain applied below */
                break;
            case 1:
            case 0:
                if (Antimagic) {
                    shieldeff(u.ux, u.uy);
                    monstseesu(M_SEEN_MAGR);
                }
                pline("Lucky for you, it didn't work!");
                dmg = 0;
                break;
            }

            if (permdmg) { /* Vecna's life force drain */
                int lowerlimit, *hpmax_p;
                /*
                 * Apply some of the damage to permanent hit points:
                 *  polymorphed         100% against poly'd hpmax
                 *  hpmax > 25*lvl      100% against normal hpmax
                 *  hpmax > 10*lvl  50..100%
                 *  hpmax >  5*lvl  25..75%
                 *  otherwise        0..50%
                 * Never reduces hpmax below 1 hit point per level.
                 */
                permdmg = rn2(dmg / 2 + 1);
                if (Upolyd || u.uhpmax > 25 * u.ulevel)
                    permdmg = dmg;
                else if (u.uhpmax > 10 * u.ulevel)
                    permdmg += dmg / 2;
                else if (u.uhpmax > 5 * u.ulevel)
                    permdmg += dmg / 4;

                if (Upolyd) {
                    hpmax_p = &u.mhmax;
                    /* [can't use gy.youmonst.m_lev] */
                    lowerlimit = min((int) gy.youmonst.data->mlevel,
                                     u.ulevel);
                } else {
                    hpmax_p = &u.uhpmax;
                    lowerlimit = minuhpmax(1);
                }
                if (*hpmax_p - permdmg > lowerlimit)
                    *hpmax_p -= permdmg;
                else if (*hpmax_p > lowerlimit)
                    *hpmax_p = lowerlimit;
                /* else already at or below the minimum threshold,
                   do nothing to hpmax */
                disp.botl = TRUE;
            }
            if (dmg)
                mdamageu(mtmp, dmg);
        }
        break;
    default:
        impossible("Gaze attack %d?", mattk->adtyp);
        break;
    }
    if (react >= 0) {
        if (Hallucination && rn2(3))
            react = rn2(SIZE(reactions));
        /* cancelled/hallucinatory feedback; monster might look "confused",
           "stunned",&c but we don't actually set corresponding attribute */
        pline_mon(mtmp, "%s looks %s%s.", Monnam(mtmp),
              !rn2(3) ? "" : already ? "quite "
                                     : (!rn2(2) ? "a bit " : "somewhat "),
              reactions[react]);
    }
    return M_ATTK_MISS;
}

/* monster uses a sonic-based attack against you */
staticfn int
screamu(struct monst *mtmp, struct attack *mattk)
{
    struct mhitm_data mhm;

    switch (mattk->adtyp) {
    case AD_LOUD:
        /* the hero must be within earshot, and the monster must be
           able to see who it is screaming at and have its special
           ability off cooldown */
        if (mdistu(mtmp) > 128 || !m_canseeu(mtmp) || mtmp->mspec_used)
            return M_ATTK_MISS;

        if (!mtmp->mcan && canseemon(mtmp) && Deaf)
            pline_mon(mtmp, "It looks as if %s is yelling at you.",
                      mon_nam(mtmp));
        else if (!mtmp->mcan && !canseemon(mtmp) && Deaf)
            You("sense a disturbing vibration in the air.");
        else if (mtmp->mcan && canseemon(mtmp) && !Deaf)
            pline_mon(mtmp, "%s croaks hoarsely.", Monnam(mtmp));
        else if (mtmp->mcan && !canseemon(mtmp) && !Deaf)
            You_hear("a hoarse croak nearby.");

        /* the shout is spent even when it fizzles */
        mtmp->mspec_used += rn2(6) + 5;

        /* cancellation or hero deafness is total immunity, inventory
           included */
        if (mtmp->mcan || Deaf)
            return M_ATTK_MISS;

        Soundeffect(se_scream, 80);
        if (m_canseeu(mtmp))
            pline_mon(mtmp, "%s lets out a %s!", Monnam(mtmp),
                      mtmp->data == &mons[PM_NAZGUL]
                          ? "bloodcurdling scream" : "deafening roar");
        else if (u.usleep && m_canseeu(mtmp) && !Deaf)
            unmul("You are frightened awake!"); /* unreachable: the
                       else-arm contradicts its own m_canseeu test;
                       kept to match EvilHack 3.6 */

        mhm.damage = d((int) mattk->damn, (int) mattk->damd);
        mhm.hitflags = M_ATTK_MISS;
        mhm.done = FALSE;
        mhm.permdmg = FALSE;
        mhm.specialdmg = 0;
        mhm.dieroll = 0;
        mhm.mwep = (struct obj *) 0;
        mhitm_adtyping(mtmp, mattk, &gy.youmonst, &mhm);
        return M_ATTK_HIT;
    default:
        break;
    }
    return M_ATTK_HIT;
}

/* mtmp hits you for n points damage */
void
mdamageu(struct monst *mtmp, int n)
{
    if (n < 0) {
        impossible("mdamageu for negative damage? (%d)", n);
        n = 0;
    }

    disp.botl = TRUE;
    if (Upolyd) {
        u.mh -= n;
        showdamage(n);
        /* caller might have reduced mhmax before calling mdamageu() */
        if (u.mh > u.mhmax)
            u.mh = u.mhmax;
        if (u.mh < 1)
            rehumanize();
    } else {
        u.uhp -= n;
        showdamage(n);
        /* caller might have reduced uhpmax before calling mdamageu() */
        if (u.uhp > u.uhpmax)
            u.uhp = u.uhpmax;
        if (u.uhp < 1)
            done_in_by(mtmp, DIED);
    }
}

/* returns 0 if seduction impossible,
 *         1 if fine,
 *         2 if wrong gender for nymph
 */
int
could_seduce(
    struct monst *magr, struct monst *mdef,
    struct attack *mattk) /* non-Null: current atk; Null: genrl capability */
{
    struct permonst *pagr;
    boolean agrinvis, defperc;
    xint16 genagr, gendef;
    int adtyp;

    if (is_animal(magr->data))
        return 0;
    if (magr == &gy.youmonst) {
        pagr = gy.youmonst.data;
        agrinvis = (Invis != 0);
        genagr = poly_gender();
    } else {
        pagr = magr->data;
        agrinvis = magr->minvis;
        genagr = gender(magr);
    }
    if (mdef == &gy.youmonst) {
        defperc = (See_invisible != 0);
        gendef = poly_gender();
        /* aasimars are angelic; demons cannot seduce them */
        if (Race_if(PM_AASIMAR) && is_demon(magr->data))
            return 0;
    } else {
        defperc = mon_prop(mdef, SEE_INVIS);
        gendef = gender(mdef);
    }

    adtyp = mattk ? mattk->adtyp
            : dmgtype(pagr, AD_SSEX) ? AD_SSEX
              : dmgtype(pagr, AD_SEDU) ? AD_SEDU
                : AD_PHYS;
    if (adtyp == AD_SSEX && !SYSOPT_SEDUCE)
        adtyp = AD_SEDU;

    if (agrinvis && !defperc && adtyp == AD_SEDU)
        return 0;

    /* nymphs have two attacks, one for steal-item damage and the other
       for seduction, both pass the could_seduce() test;
       incubi/succubi have three attacks, their claw attacks for damage
       don't pass the test */
    if ((pagr->mlet != S_NYMPH && pagr != &mons[PM_AMOROUS_DEMON])
        || (adtyp != AD_SEDU && adtyp != AD_SSEX && adtyp != AD_SITM))
        return 0;

    return (genagr == 1 - gendef) ? 1 : (pagr->mlet == S_NYMPH) ? 2 : 0;
}

/* returns 1 if monster teleported (or hero leaves monster's vicinity) */
int
doseduce(struct monst *mon)
{
    struct obj *ring, *nring;
    unsigned nringid;
    boolean fem = (mon->data == &mons[PM_AMOROUS_DEMON]
                   && Mgender(mon) == FEMALE); /* otherwise incubus */
    boolean seewho, naked; /* True iff no armor */
    /* a lawful hero wearing the Gauntlets of Purity is able to
       resist the seducer's charms */
    boolean purity = (u.ualign.type == A_LAWFUL && uarmg
                      && is_art(uarmg, ART_GAUNTLETS_OF_PURITY));
    int attr_tot, tried_gloves = 0;
    char qbuf[QBUFSZ], Who[QBUFSZ];

    if (mon->mcan || mon->mspec_used) {
        pline_mon(mon, "%s acts as though %s has got a %sheadache.",
                  Monnam(mon), mhe(mon), mon->mcan ? "severe " : "");
        return 0;
    }
    if (unresponsive() || purity) {
        pline_mon(mon, "%s seems dismayed at your lack of response.",
                  Monnam(mon));
        return 0;
    }
    seewho = canseemon(mon);
    if (!seewho)
        pline("Someone caresses you...");
    else
        You_feel("very attracted to %s.", mon_nam(mon));
    /* cache the seducer's name in a local buffer */
    Strcpy(Who, (!seewho ? (fem ? "She" : "He") : Monnam(mon)));

    /* if in the process of putting armor on or taking armor off,
       interrupt that activity now */
    (void) stop_donning((struct obj *) 0);
    /* don't try to take off gloves if cursed weapon blocks them */
    if (welded(uwep))
        tried_gloves = 1;

    for (ring = gi.invent; ring; ring = nring) {
        nring = ring->nobj;
        nringid = nring ? nring->o_id : 0;
        if (ring->otyp != RIN_ADORNMENT)
            continue;
        if (fem) {
            if (ring->owornmask && uarmg) {
                /* don't take off worn ring if gloves are in the way */
                if (!tried_gloves++) {
                    mayberem(mon, Who, uarmg, "gloves");
                    /* bare hands on a wielded cockatrice corpse can end
                       in life-saving, which uses up the amulet */
                    if (nring && o_on(nringid, gi.invent) != nring)
                        nring = (struct obj *) 0;
                }
                if (uarmg)
                    continue; /* next ring might not be worn */
            }
            /* confirmation prompt when charisma is high bypassed if deaf */
            if (!Deaf && rn2(20) < ACURR(A_CHA)) {
                (void) safe_qbuf(qbuf, "\"That ",
                                 " looks pretty.  May I have it?\"", ring,
                                 xname, simpleonames, "ring");
                makeknown(RIN_ADORNMENT);
                SetVoice(mon, 0, 80, 0);
                if (y_n(qbuf) == 'n')
                    continue;
            } else
                pline("%s decides she'd like %s, and takes it.",
                      Who, yname(ring));
            makeknown(RIN_ADORNMENT);
            /* might be in left or right ring slot or weapon/alt-wep/quiver */
            if (ring->owornmask)
                remove_worn_item(ring, FALSE);
            freeinv(ring);
            (void) mpickobj(mon, ring);
        } else {
            /* a replaced ring of levitation can drop the hero into lava,
               whose burn can use up rings we still hold */
            unsigned ringid = ring->o_id;

            if (uleft && uright && uleft->otyp == RIN_ADORNMENT
                && uright->otyp == RIN_ADORNMENT)
                break;
            if (ring == uleft || ring == uright)
                continue;
            if (uarmg) {
                /* don't put on ring if gloves are in the way */
                if (!tried_gloves++) {
                    mayberem(mon, Who, uarmg, "gloves");
                    /* see above */
                    if (nring && o_on(nringid, gi.invent) != nring)
                        nring = (struct obj *) 0;
                }
                if (uarmg)
                    break; /* no point trying further rings */
            }
            /* confirmation prompt when charisma is high bypassed if deaf */
            if (!Deaf && rn2(20) < ACURR(A_CHA)) {
                (void) safe_qbuf(qbuf, "\"That ",
                                " looks pretty.  Would you wear it for me?\"",
                                 ring, xname, simpleonames, "ring");
                makeknown(RIN_ADORNMENT);
                SetVoice(mon, 0, 80, 0);
                if (y_n(qbuf) == 'n')
                    continue;
            } else {
                pline("%s decides you'd look prettier wearing %s,",
                      Who, yname(ring));
                pline("and puts it on your finger.");
            }
            makeknown(RIN_ADORNMENT);
            if (!uright) {
                pline("%s puts %s on your right %s.",
                      Who, the(xname(ring)), body_part(HAND));
                setworn(ring, RIGHT_RING);
            } else if (!uleft) {
                pline("%s puts %s on your left %s.",
                      Who, the(xname(ring)), body_part(HAND));
                setworn(ring, LEFT_RING);
            } else if (uright && uright->otyp != RIN_ADORNMENT) {
                /* note: the "replaces" message might be inaccurate if
                   hero's location changes and the process gets interrupted,
                   but trying to figure that out in advance in order to use
                   alternate wording is not worth the effort */
                pline("%s replaces %s with %s.",
                      Who, yname(uright), yname(ring));
                Ring_gone(uright);
                /* ring removal might cause loss of levitation which could
                   drop hero onto trap that transports hero somewhere else */
                if (u.utotype || !m_next2u(mon) || !o_on(ringid, gi.invent))
                    return 1;
                if (nring && o_on(nringid, gi.invent) != nring)
                    nring = (struct obj *) 0;
                setworn(ring, RIGHT_RING);
            } else if (uleft && uleft->otyp != RIN_ADORNMENT) {
                /* see "replaces" note above */
                pline("%s replaces %s with %s.",
                      Who, yname(uleft), yname(ring));
                Ring_gone(uleft);
                if (u.utotype || !m_next2u(mon) || !o_on(ringid, gi.invent))
                    return 1;
                if (nring && o_on(nringid, gi.invent) != nring)
                    nring = (struct obj *) 0;
                setworn(ring, LEFT_RING);
            } else
                impossible("ring replacement");
            Ring_on(ring);
            prinv((char *) 0, ring, 0L);
        }
    }

    naked = (!uarmc && !uarmf
             && (!uarmg || is_art(uarmg, ART_HAND_OF_VECNA))
             && !uarms && !uarmh && !uarmu);
    urgent_pline("%s %s%s.", Who,
                 Deaf ? "seems to murmur into your ear"
                 : naked ? "murmurs sweet nothings into your ear"
                   : "murmurs in your ear",
                 naked ? "" : ", while helping you undress");
    mayberem(mon, Who, uarmc, cloak_simple_name(uarmc));
    if (!uarmc)
        mayberem(mon, Who, uarm, suit_simple_name(uarm));
    mayberem(mon, Who, uarmf, "boots");
    if (!tried_gloves)
        mayberem(mon, Who, uarmg, "gloves");
    mayberem(mon, Who, uarms,
             uarms && is_bracer(uarms) ? "bracers" : "shield");
    mayberem(mon, Who, uarmh, helm_simple_name(uarmh));
    if (!uarmc && !uarm)
        mayberem(mon, Who, uarmu, "shirt");

    /* removing armor (levitation boots, or levitation ring to make
       room for adornment ring with incubus case) might result in the
       hero falling through a trap door or landing on a teleport trap
       and changing location, so hero might not be adjacent to seducer
       any more (mayberem() has its own adjacency test so we don't need
       to check after each potential removal) */
    if (u.utotype || !m_next2u(mon))
        return 1;

    /* seducer loses interest if the hero is covered in materials
       that it hates */
    if ((uarm && mon_hates_material(mon, uarm->material))
        || (uarmc && mon_hates_material(mon, uarmc->material))
        || (uarmf && mon_hates_material(mon, uarmf->material))
        || (uarmg && mon_hates_material(mon, uarmg->material))
        || (uarms && mon_hates_material(mon, uarms->material))
        || (uarmh && mon_hates_material(mon, uarmh->material))
        || (uarmu && mon_hates_material(mon, uarmu->material))) {
        mon->mspec_used = rnd(100); /* no longer interested */
        return 0;
    }

    if (uarm || uarmc) {
        if (!Deaf) {
            if (!(ld() && mon->female)) {
                SetVoice(mon, 0, 80, 0);
                verbalize("You're such a %s; I wish...",
                          flags.female ? "sweet lady" : "nice guy");
            } else {
                struct obj *yourgloves = u_carried_gloves();

                /* have her call your gloves by their correct
                   name, possibly revealing them to you */
                if (yourgloves)
                    observe_object(yourgloves);
                verbalize("Well, then you owe me %s%s!",
                          yourgloves ? yname(yourgloves)
                                     : "twelve pairs of gloves",
                          yourgloves ? " and eleven more pairs of gloves"
                                     : "");
            }
        } else if (seewho)
            pline_mon(mon, "%s appears to sigh.", Monnam(mon));
        /* else no regret message if can't see or hear seducer */

        if (!tele_restrict(mon))
            (void) rloc(mon, RLOC_MSG);
        return 1;
    }
    if (u.ualign.type == A_CHAOTIC)
        adjalign(1);

    /* by this point you have discovered mon's identity, blind or not... */
    urgent_pline(
             "Time stands still while you and %s lie in each other's arms...",
                 noit_mon_nam(mon));
    /* 3.6.1: a combined total for charisma plus intelligence of 35-1
       used to guarantee successful outcome; now total maxes out at 32
       as far as deciding what will happen; chance for bad outcome when
       Cha+Int is 32 or more is 2/35, a bit over 5.7% */
    attr_tot = ACURR(A_CHA) + ACURR(A_INT);
    if (rn2(35) > min(attr_tot, 32)) {
        /* Don't bother with mspec_used here... it didn't get tired! */
        pline("%s seems to have enjoyed it more than you...",
              noit_Monnam(mon));
        switch (rn2(5)) {
        case 0:
            You_feel("drained of energy.");
            u.uen = 0;
            u.uenmax -= rnd(Half_physical_damage ? 5 : 10);
            exercise(A_CON, FALSE);
            if (u.uenmax < 0)
                u.uenmax = 0;
            break;
        case 1:
            You("are down in the dumps.");
            (void) adjattrib(A_CON, -1, TRUE);
            exercise(A_CON, FALSE);
            disp.botl = TRUE;
            break;
        case 2:
            Your("senses are dulled.");
            (void) adjattrib(A_WIS, -1, TRUE);
            exercise(A_WIS, FALSE);
            disp.botl = TRUE;
            break;
        case 3:
            if (!resists_drli(&gy.youmonst)) {
                You_feel("out of shape.");
                mon_drains_level(mon, "overexertion");
            } else {
                You("have a curious feeling...");
            }
            exercise(A_CON, FALSE);
            exercise(A_DEX, FALSE);
            exercise(A_WIS, FALSE);
            break;
        case 4: {
            int tmp;

            You_feel("exhausted.");
            exercise(A_STR, FALSE);
            tmp = rn1(10, 6);
            tmp = Maybe_Half_Phys(tmp);
            set_monster_killer_hp(mon, tmp, TRUE);
            losehp(tmp, "exhaustion", KILLED_BY);
            break;
        } /* case 4 */
        } /* switch */
    } else {
        mon->mspec_used = rnd(100); /* monster is worn out */
        You("seem to have enjoyed it more than %s...", noit_mon_nam(mon));
        switch (rn2(5)) {
        case 0:
            You_feel("raised to your full potential.");
            exercise(A_CON, TRUE);
            u.uen = (u.uenmax += rnd(5));
            if (u.uenmax > u.uenpeak)
                u.uenpeak = u.uenmax;
            break;
        case 1:
            You_feel("good enough to do it again.");
            (void) adjattrib(A_CON, 1, TRUE);
            exercise(A_CON, TRUE);
            disp.botl = TRUE;
            break;
        case 2:
            You("will always remember %s...", noit_mon_nam(mon));
            (void) adjattrib(A_WIS, 1, TRUE);
            exercise(A_WIS, TRUE);
            disp.botl = TRUE;
            break;
        case 3:
            pline("That was a very educational experience.");
            pluslvl(FALSE);
            exercise(A_WIS, TRUE);
            break;
        case 4:
            You_feel("restored to health!");
            u.uhp = u.uhpmax;
            if (Upolyd)
                u.mh = u.mhmax;
            exercise(A_STR, TRUE);
            disp.botl = TRUE;
            break;
        }
    }

    if (mon->mtame) { /* don't charge */
        ;
    } else if (rn2(20) < ACURR(A_CHA)) {
        pline("%s demands that you pay %s, but you refuse...",
              noit_Monnam(mon), noit_mhim(mon));
    } else if (u.umonnum == PM_LEPRECHAUN) {
        pline_mon(mon, "%s tries to take your money, but fails...",
                  noit_Monnam(mon));
    } else {
        long cost;
        long umoney = money_cnt(gi.invent);

        if (umoney > (long) LARGEST_INT - 10L)
            cost = (long) rnd(LARGEST_INT) + 500L;
        else
            cost = (long) rnd((int) umoney + 10) + 500L;
        if (mon->mpeaceful) {
            cost /= 5L;
            if (!cost)
                cost = 1L;
        }
        if (cost > umoney)
            cost = umoney;
        if (!cost) {
            if (!Deaf) {
                SetVoice(mon, 0, 80, 0);
                verbalize("It's on the house!");
            } else {
                pline("No charge.");
            }
        } else {
            pline_mon(mon, "%s takes %ld %s for services rendered!",
                      noit_Monnam(mon), cost, currency(cost));
            money2mon(mon, cost);
            disp.botl = TRUE;
        }
    }
    if (!rn2(25))
        mon->mcan = 1; /* monster is worn out */
    if (!tele_restrict(mon))
        (void) rloc(mon, RLOC_MSG);
    return 1;
}

/* 'mon' tries to remove a piece of hero's armor */
staticfn void
mayberem(struct monst *mon,
         const char *seducer, /* only used for alternate message */
         struct obj *obj, const char *str)
{
    char qbuf[QBUFSZ];

    if (!obj || !obj->owornmask)
        return;
    /* removal of a previous item might have sent the hero elsewhere
       (loss of levitation that leads to landing on a transport trap) */
    if (u.utotype || !m_next2u(mon))
        return;
    /* monster won't steal objects made of a material it hates */
    if (mon_hates_material(mon, obj->material)) {
        if (!Deaf) {
            SetVoice(mon, 0, 80, 0);
            verbalize("Ow!  %s hurts to touch!", Ysimple_name2(obj));
        } else if (canseemon(mon)) {
            pline("%s appears to recoil in disgust.", Monnam(mon));
        }
        return;
    }
    /* the Hand of Vecna cannot be stolen, as it has 'merged' with
       the wearer */
    if (is_art(obj, ART_HAND_OF_VECNA))
        return;

    /* being deaf overrides confirmation prompt for high charisma */
    if (Deaf) {
        pline("%s takes off your %s.", seducer, str);
    } else if (rn2(20) < ACURR(A_CHA)) {
        SetVoice(mon, 0, 80, 0); /* y_n aka yn_function is set up for this */
        Sprintf(qbuf, "\"Shall I remove your %s, %s?\"", str,
                (!rn2(2) ? "lover" : !rn2(2) ? "dear" : "sweetheart"));
        if (y_n(qbuf) == 'n')
            return;
    } else {
        char hairbuf[BUFSZ];

        Sprintf(hairbuf, "let me run my fingers through your %s",
                body_part(HAIR));
        SetVoice(mon, 0, 80, 0);
        verbalize("Take off your %s; %s.", str,
                  (obj == uarm)
                     ? "let's get a little closer"
                     : (obj == uarmc || obj == uarms)
                        ? "it's in the way"
                        : (obj == uarmf)
                           ? "let me rub your feet"
                           : (obj == uarmg)
                              ? "they're too clumsy"
                              : (obj == uarmu)
                                 ? "let me massage you"
                                 /* obj == uarmh */
                                 : hairbuf);
    }
    remove_worn_item(obj, TRUE);
}

staticfn int
assess_dmg(struct monst *mtmp, int tmp)
{
    if ((mtmp->mhp -= tmp) <= 0) {
        pline_mon(mtmp, "%s dies!", Monnam(mtmp));
        xkilled(mtmp, XKILL_NOMSG);
        if (!DEADMONSTER(mtmp))
            return M_ATTK_HIT;
        return M_ATTK_AGR_DIED;
    }
    return M_ATTK_HIT;
}

/* the hero's armor or hide damaged the attacker; returns M_ATTK_MISS
   while it lives, otherwise the passiveum() result to hand back */
staticfn int
passive_armor_dmg(struct monst *mtmp, int dmg, int adtyp)
{
    if (damage_mon(mtmp, dmg, adtyp, TRUE)) {
        /* damage_mon() killed the monster */
        if (canseemon(mtmp))
            pline_mon(mtmp, "%s dies!", Monnam(mtmp));
        xkilled(mtmp, XKILL_NOMSG);
        if (!DEADMONSTER(mtmp))
            return M_ATTK_HIT;
        return M_ATTK_AGR_DIED;
    }
    return M_ATTK_MISS;
}

/* the hero's armor or hide disintegrated the attacker outright; Riders
   reintegrate, everything else dies without a corpse */
staticfn int
passive_disint_kill(struct monst *mtmp)
{
    disint_mon_invent(mtmp);
    if (is_rider(mtmp->data)) {
        if (canseemon(mtmp)) {
            pline_mon(mtmp, "%s body reintegrates before your %s!",
                      s_suffix(Monnam(mtmp)),
                      (eyecount(gy.youmonst.data) == 1)
                          ? body_part(EYE)
                          : makeplural(body_part(EYE)));
            pline_mon(mtmp, "%s resurrects!", Monnam(mtmp));
        }
        mtmp->mhp = mtmp->mhpmax;
    } else {
        xkilled(mtmp, XKILL_NOMSG | XKILL_NOCORPSE);
    }
    if (!DEADMONSTER(mtmp))
        return M_ATTK_HIT;
    return M_ATTK_AGR_DIED;
}

/* returns True if monster has a range attack in its repertoire
   that it will actually utilize. Caller provides the assessment
   callback (optional). Callback returns 0 if the attack is
   active; with no callback, any range attack in the repertoire
   counts */

boolean
ranged_attk_assessed(
    struct monst *mtmp,
    boolean (*assessfunc)(struct monst *, int))
{
    int i;
    struct attack *mattk = has_erac(mtmp) ? ERAC(mtmp)->mattk
                                          : mtmp->data->mattk;

    for (i = 0; i < NATTK; i++)
        if (DISTANCE_ATTK_TYPE(mattk[i].aatyp)) {
            if (!assessfunc || (*assessfunc)(mtmp, i) == 0)
                return TRUE;
        }
    return FALSE;
}

/* can be used as ranged_attk_assessed() callback.
   Returns TRUE if monster is avoiding use of this attack */
boolean
mon_avoiding_this_attack(
    struct monst *mtmp,
    int attkidx)
{
    struct attack *mattk = has_erac(mtmp) ? ERAC(mtmp)->mattk
                                          : mtmp->data->mattk;
    int typ = -1;

    if (attkidx >= 0
        && (typ = breath_adtyp(mtmp, &mattk[attkidx])) >= 0
        && m_seenres(mtmp, cvt_adtyp_to_mseenres(typ)))
        return TRUE;
    return FALSE;
}

/*
 * This would be equivalent to:
 *     ranged_attk_assessed(mtmp, mon_avoiding_this_attack)
 * but without the added assessment function call overhead.
 */
boolean
ranged_attk_available(struct monst *mtmp)
{
    int i, typ = -1;
    struct attack *mattk = has_erac(mtmp) ? ERAC(mtmp)->mattk
                                          : mtmp->data->mattk;

    for (i = 0; i < NATTK; i++) {
        if (DISTANCE_ATTK_TYPE(mattk[i].aatyp)
            && (typ = breath_adtyp(mtmp, &mattk[i])) >= 0
                && m_seenres(mtmp, cvt_adtyp_to_mseenres(typ)) == 0)
                return TRUE;
    }
    return FALSE;
}

/* FIXME:
 *  sequencing issue:  a monster's attack might cause poly'd hero
 *  to revert to normal form.  The messages for passive counterattack
 *  would look better if they came before reverting form, but we need
 *  to know whether hero reverted in order to decide whether passive
 *  damage applies.
 */
staticfn int
passiveum(
    struct permonst *olduasmon,
    struct monst *mtmp,
    struct attack *mattk)
{
    int i, tmp, res;
    struct attack *oldu_mattk = 0;

    /* dragon-scaled body armor strikes back at whatever hit its wearer */
    if (uarm && Is_dragon_scaled_armor(uarm) && !rn2(3)) {
        int otyp = Dragon_armor_to_scales(uarm);

        switch (otyp) {
        case GREEN_DRAGON_SCALES:
            if (resists_poison(mtmp) || defended(mtmp, AD_DRST))
                break;
            if (rn2(20)) {
                /* regular poison damage (95% chance) */
                if (!rn2(3)) {
                    if (canseemon(mtmp))
                        pline_mon(mtmp, "%s staggers from the poison!",
                                  Monnam(mtmp));
                    res = passive_armor_dmg(mtmp, rnd(4), AD_DRST);
                    if (res != M_ATTK_MISS)
                        return res;
                }
            } else {
                /* fatal poison (5% chance) */
                if (canseemon(mtmp))
                    pline_mon(mtmp, "%s is fatally poisoned!", Monnam(mtmp));
                xkilled(mtmp, XKILL_NOMSG);
                if (!DEADMONSTER(mtmp))
                    return M_ATTK_HIT;
                return M_ATTK_AGR_DIED;
            }
            break;
        case BLACK_DRAGON_SCALES:
            if (resists_disint(mtmp) || defended(mtmp, AD_DISN)) {
                if (canseemon(mtmp) && !rn2(3)) {
                    shieldeff(mtmp->mx, mtmp->my);
                    Your("armor does not appear to affect %s.",
                         mon_nam(mtmp));
                }
            } else if (passive_contact_attack((int) mattk->aatyp)
                       /* a weapon, boots or gloves go first */
                       && !passive_disint_gear(mtmp, (int) mattk->aatyp,
                                               12, TRUE)) {
                if (rn2(40)) {
                    if (canseemon(mtmp))
                        pline_mon(mtmp, "%s partially disintegrates!",
                                  Monnam(mtmp));
                    res = passive_armor_dmg(mtmp, rnd(4), AD_DISN);
                    if (res != M_ATTK_MISS)
                        return res;
                } else {
                    if (canseemon(mtmp))
                        pline_mon(mtmp, "%s is disintegrated completely!",
                                  Monnam(mtmp));
                    return passive_disint_kill(mtmp);
                }
            }
            break;
        case ORANGE_DRAGON_SCALES:
            if (!rn2(3))
                mon_adjust_speed(mtmp, -1, (struct obj *) 0);
            break;
        case WHITE_DRAGON_SCALES:
            if (resists_cold(mtmp) || defended(mtmp, AD_COLD))
                break;
            if (rn2(20)) {
                if (!rn2(3)) {
                    if (canseemon(mtmp))
                        pline_mon(mtmp, "%s flinches from the cold!",
                                  Monnam(mtmp));
                    res = passive_armor_dmg(mtmp, rnd(4), AD_COLD);
                    if (res != M_ATTK_MISS)
                        return res;
                }
            } else {
                if (canseemon(mtmp))
                    pline_mon(mtmp, "%s is frozen solid!", Monnam(mtmp));
                res = passive_armor_dmg(mtmp, d(6, 6), AD_COLD);
                if (res != M_ATTK_MISS)
                    return res;
            }
            break;
        case RED_DRAGON_SCALES:
            if (resists_fire(mtmp) || defended(mtmp, AD_FIRE)
                || mon_underwater(mtmp))
                break;
            if (rn2(20)) {
                if (!rn2(3)) {
                    if (canseemon(mtmp))
                        pline_mon(mtmp, "%s is burned!", Monnam(mtmp));
                    res = passive_armor_dmg(mtmp, rnd(4), AD_FIRE);
                    if (res != M_ATTK_MISS)
                        return res;
                }
            } else {
                if (canseemon(mtmp))
                    pline_mon(mtmp, "%s is severely burned!", Monnam(mtmp));
                res = passive_armor_dmg(mtmp, d(6, 6), AD_FIRE);
                if (res != M_ATTK_MISS)
                    return res;
            }
            break;
        case YELLOW_DRAGON_SCALES:
            if (resists_acid(mtmp) || defended(mtmp, AD_ACID)
                || mon_underwater(mtmp))
                break;
            if (rn2(20)) {
                if (!rn2(3)) {
                    if (canseemon(mtmp))
                        pline_mon(mtmp, "%s is seared!", Monnam(mtmp));
                    res = passive_armor_dmg(mtmp, rnd(4), AD_ACID);
                    if (res != M_ATTK_MISS)
                        return res;
                }
            } else {
                if (canseemon(mtmp))
                    pline_mon(mtmp, "%s is critically seared!",
                              Monnam(mtmp));
                res = passive_armor_dmg(mtmp, d(6, 6), AD_ACID);
                if (res != M_ATTK_MISS)
                    return res;
            }
            break;
        case GRAY_DRAGON_SCALES:
            if (!rn2(6)) {
                /* the hero's armor did it, so the hero gets the credit
                   and the blame if this kills a clay golem */
                (void) cancel_monst(mtmp, (struct obj *) 0, TRUE, TRUE,
                                    FALSE);
                if (DEADMONSTER(mtmp))
                    return M_ATTK_AGR_DIED;
            }
            break;
        default: /* all other types of armor, just pass on through */
            break;
        }
    }

    /* Dragonbane sears the dragon that just struck its wearer */
    if (uarmg && uarmg->otyp == GLOVES && uarmg->oartifact == ART_DRAGONBANE
        && is_dragon(mtmp->data) && !rn2(3)) {
        if (canseemon(mtmp))
            pline("Dragonbane sears %s scaly hide!", s_suffix(mon_nam(mtmp)));
        if (damage_mon(mtmp, rnd(6) + 2, AD_PHYS, TRUE)) {
            /* damage_mon() killed the monster */
            if (canseemon(mtmp)) {
                pline("Dragonbane's power overwhelms %s!", mon_nam(mtmp));
                pline_mon(mtmp, "%s dies!", Monnam(mtmp));
            }
            xkilled(mtmp, XKILL_NOMSG);
            if (!DEADMONSTER(mtmp))
                return M_ATTK_HIT;
            return M_ATTK_AGR_DIED;
        }
    }

    /*
     * mattk      == mtmp's attack that hit you;
     * oldu_mattk == your passive counterattack (even if mtmp's attack
     *               has already caused you to revert to normal form).
     */
    for (i = 0; !oldu_mattk; i++) {
        if (i >= NATTK)
            return M_ATTK_HIT;
        if (olduasmon->mattk[i].aatyp == AT_NONE
            || olduasmon->mattk[i].aatyp == AT_BOOM)
            oldu_mattk = &olduasmon->mattk[i];
    }
    if (oldu_mattk->damn)
        tmp = d((int) oldu_mattk->damn, (int) oldu_mattk->damd);
    else if (oldu_mattk->damd) {
        /* a dragon form's hide deals a third of the level-scaled roll */
        if (is_dragon(olduasmon))
            tmp = d((int) olduasmon->mlevel + 1, (int) oldu_mattk->damd) / 3;
        else
            tmp = d((int) olduasmon->mlevel + 1, (int) oldu_mattk->damd);
    } else
        tmp = 0;

    /* These affect the enemy even if you were "killed" (rehumanized) */
    switch (oldu_mattk->adtyp) {
    case AD_ACID:
        if (!rn2(2)) {
            if (gy.youmonst.data == &mons[PM_YELLOW_DRAGON])
                pline_mon(mtmp, "%s is seared by your acidic hide!",
                          Monnam(mtmp));
            else
                pline_mon(mtmp, "%s is splashed by %s%s!", Monnam(mtmp),
                      /* temporary? hack for sequencing issue:  "your acid"
                         looks strange coming immediately after player has
                         been told that hero has reverted to normal form */
                      !Upolyd ? "" : "your ", hliquid("acid"));
            if (resists_acid(mtmp) || defended(mtmp, AD_ACID)
                || mon_underwater(mtmp)) {
                pline_mon(mtmp, "%s is not affected.", Monnam(mtmp));
                tmp = 0;
            }
        } else
            tmp = 0;
        if (!Underwater) {
            if (!rn2(30))
                erode_armor(mtmp, ERODE_CORRODE);
            if (!rn2(6)) {
                struct obj *mwp = MON_WEP(mtmp);
                struct obj *mwp2 = MON_WEP2(mtmp);

                /* acid_damage() can now destroy a weapon; when a
                   wielded slot empties, tell mattacku()'s cached
                   weapon pointer so it does not dangle for the rest
                   of the attack chain */
                acid_damage(mwp);
                if (mwp && MON_WEP(mtmp) != mwp)
                    mon_currwep_gone(mwp);
                acid_damage(mwp2);
                if (mwp2 && MON_WEP2(mtmp) != mwp2)
                    mon_currwep_gone(mwp2);
            }
        }
        return assess_dmg(mtmp, tmp);
    case AD_DISN: { /* black dragon, antimatter vortex */
        boolean vortex = (gy.youmonst.data == &mons[PM_ANTIMATTER_VORTEX]);

        if (resists_disint(mtmp) || defended(mtmp, AD_DISN)) {
            if (canseemon(mtmp) && !rn2(3)) {
                shieldeff(mtmp->mx, mtmp->my);
                Your("deadly %s does not appear to affect %s.",
                     vortex ? "form" : "hide", mon_nam(mtmp));
            }
        } else if (passive_contact_attack((int) mattk->aatyp)
                   /* a weapon, boots or gloves go first */
                   && !passive_disint_gear(mtmp, (int) mattk->aatyp,
                                           vortex ? 3 : 6, FALSE)) {
            if (vortex ? rn2(10) : rn2(20)) {
                if (canseemon(mtmp))
                    Your("%s partially disintegrates %s!",
                         vortex ? "form" : "hide", mon_nam(mtmp));
                tmp = rn2(6) + 1;
                return assess_dmg(mtmp, tmp);
            }
            if (canseemon(mtmp))
                Your("deadly %s disintegrates %s!",
                     vortex ? "form" : "hide", mon_nam(mtmp));
            return passive_disint_kill(mtmp);
        }
        break;
    }
    case AD_STON: /* cockatrice */
    {
        long protector = attk_protection((int) mattk->aatyp),
             wornitems = mtmp->misc_worn_check;

        /* wielded weapon gives same protection as gloves here */
        if (MON_WEP(mtmp) != 0)
            wornitems |= W_ARMG;

        if (!(resists_ston(mtmp) || defended(mtmp, AD_STON))
            && (protector == 0L
                || (protector != ~0L
                    && (wornitems & protector) != protector))) {
            if (poly_when_stoned(mtmp->data)) {
                mon_to_stone(mtmp);
                return 1;
            }
            pline_mon(mtmp, "%s turns to stone!", Monnam(mtmp));
            gs.stoned = 1;
            xkilled(mtmp, XKILL_NOMSG);
            if (!DEADMONSTER(mtmp))
                return M_ATTK_HIT;
            return M_ATTK_AGR_DIED;
        }
        return M_ATTK_HIT;
    }
    case AD_ENCH: /* KMH -- remove enchantment (disenchanter) */
        if (mon_currwep) {
            /* by_you==True: passive counterattack to hero's action
               is hero's fault */
            (void) drain_item(mon_currwep, TRUE);
            /* No message */
        }
        return M_ATTK_HIT;
    case AD_CNCL: /* gray dragon */
        if (!rn2(6)) {
            /* the hero's hide did it; hero credit if it kills a golem */
            (void) cancel_monst(mtmp, (struct obj *) 0, TRUE, TRUE, FALSE);
            if (DEADMONSTER(mtmp))
                return M_ATTK_AGR_DIED;
        }
        return M_ATTK_HIT;
    case AD_SLIM: /* green slime */
        if (!rn2(3)) {
            Your("slime splashes onto %s!", mon_nam(mtmp));
            if (flaming(mtmp->data)) {
                pline_The("slime burns away!");
            } else if (slimeproof(mtmp->data)) {
                pline_mon(mtmp, "%s is unaffected.", Monnam(mtmp));
            } else if (!rn2(4)) {
                if (!munslime(mtmp, FALSE) && !DEADMONSTER(mtmp)) {
                    if (newcham(mtmp, &mons[PM_GREEN_SLIME],
                                canseemon(mtmp) ? NC_SHOW_MSG : NO_NC_FLAGS))
                        mtmp->mstrategy &= ~STRAT_WAITFORU;
                }
                /* the cure attempt or the transform might have killed
                   it (green slime unable to survive water, lava, or
                   open air) */
                if (DEADMONSTER(mtmp))
                    return M_ATTK_AGR_DIED;
            }
        }
        return M_ATTK_HIT;
    default:
        break;
    }
    if (!Upolyd)
        return M_ATTK_HIT;

    /* These affect the enemy only if you are still a monster */
    if (rn2(3))
        switch (oldu_mattk->adtyp) {
        case AD_PHYS:
            if (oldu_mattk->aatyp == AT_BOOM) {
                You("explode!");
                /* KMH, balance patch -- this is okay with unchanging */
                rehumanize();
                return assess_dmg(mtmp, tmp);
            }
            break;
        case AD_PLYS: /* Floating eye */
            if (tmp > 127)
                tmp = 127;
            if (u.umonnum == PM_FLOATING_EYE) {
                if (!rn2(4))
                    tmp = 127;
                if (mtmp->mcansee && haseyes(mtmp->data) && rn2(3)
                    && (mon_prop(mtmp, SEE_INVIS) || !Invis)) {
                    if (Blind) {
                        pline("As a blind %s, you cannot defend yourself.",
                              pmname(gy.youmonst.data,
                                     flags.female ? FEMALE : MALE));
                    } else {
                        if (mon_reflects(mtmp,
                                         "Your gaze is reflected by %s %s."))
                            return 1;
                        if (has_free_action(mtmp)) {
                            pline_mon(mtmp, "%s stiffens momentarily.",
                                      Monnam(mtmp));
                            return M_ATTK_HIT;
                        }
                        pline_mon(mtmp, "%s is frozen by your gaze!",
                                  Monnam(mtmp));
                        paralyze_monst(mtmp, tmp);
                        return M_ATTK_AGR_DONE;
                    }
                }
            } else { /* gelatinous cube */
                if (has_free_action(mtmp)) {
                    pline_mon(mtmp, "%s stiffens momentarily.", Monnam(mtmp));
                    return M_ATTK_HIT;
                }
                pline_mon(mtmp, "%s is frozen by you.", Monnam(mtmp));
                paralyze_monst(mtmp, tmp);
                return M_ATTK_AGR_DONE;
            }
            return M_ATTK_HIT;
        case AD_COLD: /* Brown mold or blue jelly */
            if (resists_cold(mtmp) || defended(mtmp, AD_COLD)) {
                shieldeff(mtmp->mx, mtmp->my);
                pline_mon(mtmp, "%s is mildly chilly.", Monnam(mtmp));
                golemeffects(mtmp, AD_COLD, tmp);
                tmp = 0;
                break;
            }
            pline_mon(mtmp, "%s is suddenly very cold!", Monnam(mtmp));
            u.mh += (tmp + rn2(2)) / 2;
            if (u.mhmax < u.mh)
                u.mhmax = u.mh;
            if (u.mhmax > (((int) gy.youmonst.data->mlevel + 1) * 8))
                (void) split_mon(&gy.youmonst, mtmp);
            break;
        case AD_STUN: /* Yellow mold */
            if (mon_resists_stun(mtmp) || defended(mtmp, AD_STUN)
                || mon_wielding_artifact(mtmp, ART_TEMPEST))
                break; /* immune to stun; passive damage still applies */
            if (!mtmp->mstun) {
                mtmp->mstun = 1;
                pline_mon(mtmp, "%s %s.", Monnam(mtmp),
                      makeplural(stagger(mtmp->data, "stagger")));
            }
            tmp = 0;
            break;
        case AD_FIRE: /* Red mold */
            if (resists_fire(mtmp) || defended(mtmp, AD_FIRE)
                || mon_underwater(mtmp)) {
                shieldeff(mtmp->mx, mtmp->my);
                pline_mon(mtmp, "%s is mildly warm.", Monnam(mtmp));
                golemeffects(mtmp, AD_FIRE, tmp);
                tmp = 0;
                break;
            }
            pline_mon(mtmp, "%s is suddenly very hot!", Monnam(mtmp));
            break;
        case AD_ELEC:
            if (resists_elec(mtmp) || defended(mtmp, AD_ELEC)) {
                shieldeff(mtmp->mx, mtmp->my);
                pline_mon(mtmp, "%s is slightly tingled.", Monnam(mtmp));
                golemeffects(mtmp, AD_ELEC, tmp);
                tmp = 0;
                break;
            }
            pline_mon(mtmp, "%s is jolted with your electricity!",
                      Monnam(mtmp));
            break;
        case AD_DRST: /* green dragon */
            if (resists_poison(mtmp) || defended(mtmp, AD_DRST)) {
                if (canseemon(mtmp) && !rn2(5))
                    pline_mon(mtmp, "%s is unaffected by your poisonous hide.",
                              Monnam(mtmp));
                tmp = 0;
                break;
            }
            if (rn2(20)) {
                /* regular poison damage; assess_dmg() applies it */
                if (canseemon(mtmp))
                    pline_mon(mtmp, "%s is poisoned!", Monnam(mtmp));
            } else {
                /* deadly poison - instant death */
                if (canseemon(mtmp))
                    Your("poisonous hide was deadly...");
                xkilled(mtmp, XKILL_NOMSG);
                if (!DEADMONSTER(mtmp))
                    return M_ATTK_HIT;
                return M_ATTK_AGR_DIED;
            }
            break;
        case AD_SLOW: /* orange dragon */
            if (!rn2(3))
                mon_adjust_speed(mtmp, -1, (struct obj *) 0);
            tmp = 0;
            break;
        case AD_DISE: /* gray fungus */
            if (resists_sick(mtmp) || defended(mtmp, AD_DISE)) {
                if (canseemon(mtmp))
                    pline_mon(mtmp, "%s resists infection.", Monnam(mtmp));
                tmp = 0;
                break;
            }
            if (mtmp->mdiseasetime)
                mtmp->mdiseasetime -= rnd(3);
            else
                mtmp->mdiseasetime = rn1(9, 6);
            if (canseemon(mtmp))
                pline_mon(mtmp, "%s looks %s.", Monnam(mtmp),
                          mtmp->mdiseased ? "even worse" : "diseased");
            mtmp->mdiseased = 1;
            mtmp->mdiseabyu = TRUE;
            break;
        case AD_SLEE: /* black fungus */
            if (sleep_monst(mtmp, rn2(3) + 8, -1)) {
                if (canseemon(mtmp))
                    pline_mon(mtmp, "%s loses consciousness.", Monnam(mtmp));
                slept_monst(mtmp);
            }
            break;
        default:
            tmp = 0;
            break;
        }
    else
        tmp = 0;

    return assess_dmg(mtmp, tmp);
}

struct monst *
cloneu(void)
{
    struct monst *mon;
    int mndx = monsndx(gy.youmonst.data);

    if (u.mh <= 1)
        return (struct monst *) 0;
    if (svm.mvitals[mndx].mvflags & G_EXTINCT)
        return (struct monst *) 0;
    mon = makemon(gy.youmonst.data, u.ux, u.uy,
                  NO_MINVENT | MM_EDOG | MM_NOMSG);
    if (!mon)
        return NULL;
    mon->mcloned = 1;
    mon = christen_monst(mon, svp.plname);
    initedog(mon, TRUE);
    mon->m_lev = gy.youmonst.data->mlevel;
    mon->mhpmax = u.mhmax;
    mon->mhp = u.mh / 2;
    u.mh -= mon->mhp;
    disp.botl = TRUE;
    return mon;
}

#undef ld

/*mhitu.c*/
