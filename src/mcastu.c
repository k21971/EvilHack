/* NetHack 3.6	mcastu.c	$NHDT-Date: 1567418129 2019/09/02 09:55:29 $  $NHDT-Branch: NetHack-3.6 $:$NHDT-Revision: 1.55 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2011. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "mextra.h"

extern void demonpet();

/* monster mage spells */
enum mcast_mage_spells {
    MGC_PSI_BOLT = 0,
    MGC_FIRE_BOLT,
    MGC_ICE_BOLT,
    MGC_CURE_SELF,
    MGC_HASTE_SELF,
    MGC_STUN_YOU,
    MGC_DISAPPEAR,
    MGC_WEAKEN_YOU,
    MGC_DESTRY_ARMR,
    MGC_CURSE_ITEMS,
    MGC_AGGRAVATION,
    MGC_ACID_BLAST,
    MGC_SUMMON_MONS,
    MGC_CLONE_WIZ,
    MGC_CANCELLATION,
    MGC_REFLECTION,
    MGC_DEATH_TOUCH,
    MGC_LEARNED_SPELL /* spell learned from spellbook */
};

/* monster cleric spells */
enum mcast_cleric_spells {
    CLC_OPEN_WOUNDS = 0,
    CLC_CURE_SELF,
    CLC_PROTECTION,
    CLC_BARKSKIN,
    CLC_STONESKIN,
    CLC_CONFUSE_YOU,
    CLC_PARALYZE,
    CLC_VULN_YOU,
    CLC_BLIND_YOU,
    CLC_INSECTS,
    CLC_CURSE_ITEMS,
    CLC_LIGHTNING,
    CLC_FIRE_PILLAR,
    CLC_GEYSER,
    CLC_SUMMON_MINION,
    CLC_CALL_UNDEAD,
    CLC_LEARNED_SPELL /* spell learned from spellbook */
};

extern void you_aggravate(struct monst *);

STATIC_DCL int FDECL(choose_bolt_spell, (struct monst *));
STATIC_DCL boolean FDECL(has_eota, (struct monst *));
STATIC_DCL boolean FDECL(is_boss_caster, (struct monst *));
STATIC_DCL void FDECL(cursetxt, (struct monst *, BOOLEAN_P));
STATIC_DCL int FDECL(choose_magic_spell, (struct monst *, int));
STATIC_DCL int FDECL(choose_clerical_spell, (struct monst *, int));
STATIC_DCL int FDECL(m_cure_self, (struct monst *, int));
STATIC_DCL int FDECL(m_destroy_armor, (struct monst *, struct monst *));
STATIC_DCL int FDECL(cast_learned_spell, (struct monst *, struct monst *));
STATIC_DCL void FDECL(do_wizard_spell, (struct monst *, struct monst *, int, int));
STATIC_DCL void FDECL(do_cleric_spell, (struct monst *, struct monst *, int, int));
STATIC_DCL boolean FDECL(is_undirected_spell, (unsigned int, int));
STATIC_DCL boolean FDECL(do_spell_would_be_useless, (struct monst *,
                                                     struct monst *, unsigned int, int));
STATIC_DCL boolean FDECL(uspell_would_be_useless, (unsigned int, int));
STATIC_DCL int FDECL(mcount_castable_spells, (struct monst *));

/* Choose a random bolt spell type for monster caster. Ice Queen always
   gets ice bolt. Returns spell number, or -1 if caller should use
   fallthrough behavior */
STATIC_OVL int
choose_bolt_spell(mtmp)
struct monst *mtmp;
{
    switch (rn2(3)) {
    case 2:
        return (mtmp->data == &mons[PM_KATHRYN_THE_ICE_QUEEN])
               ? MGC_ICE_BOLT : MGC_FIRE_BOLT;
    case 1:
        return MGC_ICE_BOLT;
    case 0:
    default:
        /* If player has Antimagic or is Hallucinating, psi bolt is more
           effective. Otherwise, signal caller to fall through */
        return (Antimagic || Hallucination) ? MGC_PSI_BOLT : -1;
    }
}

/* Check if monster has Eye of the Aethiopica in inventory.
   EotA allows unlimited spellcasting (mspec_used = 0) */
STATIC_OVL boolean
has_eota(mtmp)
struct monst *mtmp;
{
    struct obj *obj;

    for (obj = mtmp->minvent; obj; obj = obj->nobj) {
        if (obj->oartifact == ART_EYE_OF_THE_AETHIOPICA)
            return TRUE;
    }
    return FALSE;
}

/* Check if monster is a boss-type spellcaster that gets mspec bonuses.
   Used to allow bosses to cast more frequently */
STATIC_OVL boolean
is_boss_caster(mtmp)
struct monst *mtmp;
{
    return (is_dprince(mtmp->data) || is_dlord(mtmp->data)
            || mtmp->iswiz || mtmp->isvecna || mtmp->istalgath
            || mtmp->data->msound == MS_LEADER
            || mtmp->data->msound == MS_NEMESIS
            || mtmp->data == &mons[PM_ORACLE]
            || mtmp->data == &mons[PM_HIGH_PRIEST]
            || mtmp->data == &mons[PM_KATHRYN_THE_ICE_QUEEN]
            || mtmp->data == &mons[PM_KATHRYN_THE_ENCHANTRESS]);
}

boolean
is_spellcaster(mtmp)
struct monst *mtmp;
{
    int i = 0;
    struct attack *mattk;
    mattk = has_erac(mtmp) ? ERAC(mtmp)->mattk : mtmp->data->mattk;

    for (; i < NATTK; i++) {
        if (mattk[i].aatyp == AT_MAGC
            && (mattk[i].adtyp == AD_SPEL
                || mattk[i].adtyp == AD_CLRC)) {
            return TRUE;
        }
    }
    return FALSE;
}

extern const char *const flash_types[]; /* from zap.c */

/* feedback when frustrated monster couldn't cast a spell */
STATIC_OVL
void
cursetxt(mtmp, undirected)
struct monst *mtmp;
boolean undirected;
{
    if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my)) {
        const char *point_msg; /* spellcasting monsters are impolite */

        if (undirected)
            point_msg = "all around, then curses";
        else if ((Invis && !mon_prop(mtmp, SEE_INVIS)
                  && (mtmp->mux != u.ux || mtmp->muy != u.uy))
                 || is_obj_mappear(&youmonst, STRANGE_OBJECT)
                 || u.uundetected)
            point_msg = "and curses in your general direction";
        else if (Displaced && (mtmp->mux != u.ux || mtmp->muy != u.uy))
            point_msg = "and curses at your displaced image";
        else
            point_msg = "at you, then curses";

        pline("%s points %s.", Monnam(mtmp), point_msg);
    } else if ((!(moves % 4) || !rn2(4))) {
        if (!Deaf)
            Norep("You hear a mumbled curse."); /* Deaf-aware */
    }
}

/* convert a level based random selection into a specific mage spell;
   inappropriate choices will be screened out by spell_would_be_useless() */
STATIC_OVL int
choose_magic_spell(mtmp, spellval)
struct monst* mtmp;
int spellval;
{
    int i, spell_count, chance;

    /* Chance to try a learned spell scales with repertoire size:
       15% base + 5% per castable spell, capped at 40% */
    spell_count = mcount_castable_spells(mtmp);
    if (spell_count > 0) {
        chance = 15 + (spell_count * 5);
        if (chance > 40)
            chance = 40;
        if (rn2(100) < chance) {
            short learned = mchoose_learned_spell(mtmp);
            if (learned != 0)
                return MGC_LEARNED_SPELL;
        }
    }

    /* monster level needs to be [case value] + 1
       in order to cast that level of spell
       (e.g. a gnomish wizard will not spawn higher
       than monster level 4; highest level spell
       they can cast is MGC_STUN_YOU. Should it
       reach monster level 5, MGC_DISAPPEAR then
       becomes available to cast) */
    while (spellval > 24 && rn2(25))
        spellval = rn2(spellval);

    /* If we're hurt, seriously consider fixing ourselves a priority */
    if ((mtmp->mhp * 4) <= mtmp->mhpmax)
        spellval = 1;

    switch (spellval) {
    case 24:
    case 23:
        i = choose_bolt_spell(mtmp);
        if (i >= 0)
            return i;
        /*FALLTHRU*/
    case 22:
    case 21:
    case 20:
        return MGC_DEATH_TOUCH;
    case 19:
    case 18:
        return MGC_CLONE_WIZ;
    case 17:
    case 16:
        return MGC_SUMMON_MONS;
    case 15:
        return MGC_CANCELLATION;
    case 14:
        return MGC_ACID_BLAST;
    case 13:
        i = rnd(2);
        switch (i) {
        case 1:
            return MGC_AGGRAVATION;
        case 2:
            return MGC_REFLECTION;
        }
        return i;
    case 12:
    case 11:
        return MGC_CURSE_ITEMS;
    case 10:
    case 9:
    case 8:
        return MGC_DESTRY_ARMR;
    case 7:
    case 6:
        return MGC_WEAKEN_YOU;
    case 5:
    case 4:
        return MGC_DISAPPEAR;
    case 3:
        return MGC_STUN_YOU;
    case 2:
        return MGC_HASTE_SELF;
    case 1:
        return MGC_CURE_SELF;
    case 0:
    default:
        i = choose_bolt_spell(mtmp);
        return (i >= 0) ? i : MGC_PSI_BOLT;
    }
}

/* convert a level based random selection into a specific cleric spell */
STATIC_OVL int
choose_clerical_spell(mtmp, spellnum)
struct monst* mtmp;
int spellnum;
{
    int spell_count, chance;

    /* Chance to try a learned spell scales with repertoire size:
       15% base + 5% per castable spell, capped at 40% */
    spell_count = mcount_castable_spells(mtmp);
    if (spell_count > 0) {
        chance = 15 + (spell_count * 5);
        if (chance > 40)
            chance = 40;
        if (rn2(100) < chance) {
            short learned = mchoose_learned_spell(mtmp);
            if (learned != 0)
                return CLC_LEARNED_SPELL;
        }
    }

    /* monster level needs to be [case value] + 1
       in order to cast that level of spell
       (e.g. a kobold shaman will not spawn higher
       than monster level 3; highest level spell
       they can cast is CLC_PROTECTION. Should it
       reach monster level 4, CLC_CONFUSE_YOU then
       becomes available to cast) */
    while (spellnum > 15 && rn2(16))
        spellnum = rn2(spellnum);

    /* If we're hurt, seriously consider fixing ourselves priority */
    if ((mtmp->mhp * 4) <= mtmp->mhpmax)
        spellnum = 1;

    switch (spellnum) {
    case 15:
    case 14:
        if (rn2(3))
            return CLC_OPEN_WOUNDS;
        /*FALLTHRU*/
    case 13:
        if (is_demon(mtmp->data)
            || mtmp->mtame || mtmp->mpeaceful)
            return CLC_FIRE_PILLAR;
        else
            return CLC_SUMMON_MINION;
    case 12:
        return CLC_GEYSER;
    case 11:
        if (mtmp->data == &mons[PM_KATHRYN_THE_ICE_QUEEN]
            || mtmp->data == &mons[PM_ASMODEUS])
            return CLC_LIGHTNING;
        else
            return CLC_FIRE_PILLAR;
    case 10:
        return CLC_LIGHTNING;
    case 9:
        return CLC_CURSE_ITEMS;
    case 8:
        return CLC_CALL_UNDEAD;
    case 7:
        if ((is_demon(mtmp->data)
             && mtmp->data != &mons[PM_LOLTH])
            || mtmp->data == &mons[PM_KATHRYN_THE_ICE_QUEEN]
            || mtmp->data == &mons[PM_KATHRYN_THE_ENCHANTRESS])
            return CLC_VULN_YOU;
        else
            return CLC_INSECTS;
    case 6:
        if (!has_stoneskin(mtmp)
            && (mtmp->data == &mons[PM_ELANEE]
                || mtmp->data == &mons[PM_DRUID]
                || mtmp->data == &mons[PM_ASPIRANT]))
            return CLC_STONESKIN;
        else
            return CLC_BLIND_YOU;
    case 5:
        return CLC_VULN_YOU;
    case 4:
        return CLC_PARALYZE;
    case 3:
        return CLC_CONFUSE_YOU;
    case 2:
        if (!has_barkskin(mtmp)
            && (mtmp->data == &mons[PM_ELANEE]
                || mtmp->data == &mons[PM_DRUID]
                || mtmp->data == &mons[PM_ASPIRANT]))
            return CLC_BARKSKIN;
        else
            return CLC_PROTECTION;
    case 1:
        return CLC_CURE_SELF;
    case 0:
    default:
        return CLC_OPEN_WOUNDS;
    }
}

/* return values:
 * 1: successful spell
 * 0: unsuccessful spell
 */
int
castmu(mtmp, mattk, thinks_it_foundyou, foundyou)
struct monst *mtmp;
struct attack *mattk;
boolean thinks_it_foundyou;
boolean foundyou;
{
    int dmg, ml = min(mtmp->m_lev, 50);
    int ret;
    int spellnum = 0;

    boolean seecaster = (canseemon(mtmp) || tp_sensemon(mtmp) || Detect_monsters);

    /* Three cases:
     * -- monster is attacking you.  Search for a useful spell.
     * -- monster thinks it's attacking you.  Search for a useful spell,
     *    without checking for undirected.  If the spell found is directed,
     *    it fails with cursetxt() and loss of mspec_used.
     * -- monster isn't trying to attack.  Select a spell once.  Don't keep
     *    searching; if that spell is not useful (or if it's directed),
     *    return and do something else.
     * Since most spells are directed, this means that a monster that isn't
     * attacking casts spells only a small portion of the time that an
     * attacking monster does.
     */

    if ((mattk->adtyp == AD_SPEL || mattk->adtyp == AD_CLRC) && ml) {
        int cnt = 40;

        do {
            spellnum = rn2(ml);
            if (mattk->adtyp == AD_SPEL)
                spellnum = choose_magic_spell(mtmp, spellnum);
            else
                spellnum = choose_clerical_spell(mtmp, spellnum);
            /* not trying to attack?  don't allow directed spells */
            if (!thinks_it_foundyou) {
                if (!is_undirected_spell(mattk->adtyp, spellnum)
                    || do_spell_would_be_useless(mtmp, &youmonst,
                                                 mattk->adtyp, spellnum)) {
                    if (foundyou)
                        impossible(
                       "spellcasting monster found you and doesn't know it?");
                    return 0;
                }
                break;
            }
        } while (--cnt > 0
                 && do_spell_would_be_useless(mtmp, &youmonst,
                                              mattk->adtyp, spellnum));
        if (cnt == 0)
            return 0;
    }

    /* monster unable to cast spells? */
    if (mtmp->mcan || mtmp->mspec_used || !ml) {
        cursetxt(mtmp, is_undirected_spell(mattk->adtyp, spellnum));
        return 0;
    }

    if (mattk->adtyp == AD_SPEL || mattk->adtyp == AD_CLRC) {
        mtmp->mspec_used = 4 - mtmp->m_lev;
        if (mtmp->mspec_used < 2)
            mtmp->mspec_used = 2;
        /* many boss-type monsters than have two or more spell attacks
           per turn are never able to fire off their second attack due
           to mspec always being greater than 0. So set to 0 for those
           types of monsters half of the time */
        if (rn2(2) && is_boss_caster(mtmp))
            mtmp->mspec_used = 0;

        /* Having the EotA in inventory drops mspec to 0 */
        if (has_eota(mtmp))
            mtmp->mspec_used = 0;
    }

    /* monster can cast spells, but is casting a directed spell at the
       wrong place?  If so, give a message, and return.  Do this *after*
       penalizing mspec_used. */
    if (!foundyou && thinks_it_foundyou
        && !is_undirected_spell(mattk->adtyp, spellnum)) {
        pline("%s casts a spell at %s!",
              seecaster ? Monnam(mtmp) : "Something",
              levl[mtmp->mux][mtmp->muy].typ == WATER ? "empty water"
                                                      : "thin air");
        return 0;
    }

    if (!mtmp->mpeaceful)
        nomul(0);
    if (rn2(ml * 10) < (mtmp->mconf ? 100 : 10)) { /* fumbled attack */
        if (canseemon(mtmp) && !Deaf)
            pline_The("air crackles around %s.", mon_nam(mtmp));
        return 0;
    }
    if (seecaster || !is_undirected_spell(mattk->adtyp, spellnum)) {
        if (mtmp->mpeaceful
            && mtmp->ispriest && inhistemple(mtmp)) {
            ; /* cut down on the temple spam */
        } else {
            pline("%s casts a spell%s!",
                  seecaster ? Monnam(mtmp) : "Something",
                  is_undirected_spell(mattk->adtyp, spellnum)
                      ? ""
                      : (Invis && !mon_prop(mtmp, SEE_INVIS)
                         && (mtmp->mux != u.ux || mtmp->muy != u.uy))
                            ? " at a spot near you"
                            : (Displaced
                               && (mtmp->mux != u.ux || mtmp->muy != u.uy))
                                  ? " at your displaced image"
                                  : " at you");
        }
    } else if (!Deaf && is_undirected_spell(mattk->adtyp, spellnum)) {
        if (mtmp->mpeaceful
            && mtmp->ispriest && inhistemple(mtmp)) {
            ; /* cut down on the temple spam */
        } else {
            You_hear("something cast a spell!");
        }
    }

    /*
     * As these are spells, the damage is related to the level
     * of the monster casting the spell.
     */
    if (!foundyou) {
        dmg = 0;
        if (mattk->adtyp != AD_SPEL && mattk->adtyp != AD_CLRC) {
            impossible(
              "%s casting non-hand-to-hand version of hand-to-hand spell %d?",
                       Monnam(mtmp), mattk->adtyp);
            return 0;
        }
    } else if (mattk->damd)
        dmg = d((int) ((ml / 2) + mattk->damn), (int) mattk->damd);
    else
        dmg = d((int) ((ml / 2) + 1), 6);
    if (Half_spell_damage)
        dmg = (dmg + 1) / 2;

    ret = 1;

    switch (mattk->adtyp) {
    case AD_FIRE:
        if (is_demon(mtmp->data))
            pline("You're enveloped in hellfire!");
        else if (!Underwater)
            pline("You're enveloped in flames.");
        else {
                pline("The flames are quenched by the water around you.");
                dmg = 0;
                break;
            }

        if (how_resistant(FIRE_RES) == 100) {
            shieldeff(u.ux, u.uy);
            if (is_demon(mtmp->data)) {
                if (Race_if(PM_DEMON) || Race_if(PM_DRAUGR)
                    || Race_if(PM_VAMPIRE) || nonliving(youmonst.data)) {
                    dmg = 0;
                } else {
                    pline_The("hellish flames sear your soul!");
                    dmg = (dmg + 1) / 2;
                }
            } else {
                pline("But you resist the effects.");
                monstseesu(M_SEEN_FIRE);
                dmg = 0;
            }
        } else {
            if (is_demon(mtmp->data)
                && !(Race_if(PM_DEMON) || Race_if(PM_DRAUGR)
                     || Race_if(PM_VAMPIRE) || nonliving(youmonst.data))) {
                pline_The("hellish flames sear your soul!");
                dmg = resist_reduce(dmg, FIRE_RES) * 2;
            } else {
                dmg = resist_reduce(dmg, FIRE_RES);
            }
        }
        burn_away_slime();
        break;
    case AD_COLD:
        pline("You're covered in frost.");
        if (how_resistant(COLD_RES) == 100) {
            shieldeff(u.ux, u.uy);
            pline("But you resist the effects.");
            monstseesu(M_SEEN_COLD);
            dmg = 0;
        } else {
            dmg = resist_reduce(dmg, COLD_RES);
        }
        break;
    case AD_ACID:
        if (!Underwater)
            pline("You're splashed in acid.");
        else {
            pline("The acid dissipates harmlessly in the water around you.");
            dmg = 0;
            break;
        }
        if (how_resistant(ACID_RES) == 100) {
            shieldeff(u.ux, u.uy);
            pline("But you resist the effects.");
            monstseesu(M_SEEN_ACID);
            dmg = 0;
        } else {
            dmg = d((int) ml / 2 + 1, 8);
        }
        break;
    case AD_MAGM:
        You("are hit by a shower of missiles!");
        dmg = d((int) ml / 2 + 1, 6);
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
            pline("Some missiles bounce off!");
            monstseesu(M_SEEN_MAGR);
            dmg = (dmg + 1) / 2;
        }
        break;
    case AD_SPEL:   /* wizard spell */
    case AD_CLRC: { /* clerical spell */
        if (mattk->adtyp == AD_SPEL)
            do_wizard_spell(mtmp, &youmonst, dmg, spellnum);
        else
            do_cleric_spell(mtmp, &youmonst, dmg, spellnum);
        dmg = 0; /* done by the spell casting functions */
        break;
        }
    }
    if (dmg)
        mdamageu(mtmp, dmg);
    return (ret);
}

STATIC_OVL int
m_cure_self(mtmp, dmg)
struct monst *mtmp;
int dmg;
{
    if (mtmp->mhp < mtmp->mhpmax) {
        if (canseemon(mtmp))
            pline("%s looks better.", Monnam(mtmp));
        /* note: player healing does 6d4; this used to do 1d8 */
        if ((mtmp->mhp += d(3, 6)) > mtmp->mhpmax)
            mtmp->mhp = mtmp->mhpmax;
        dmg = 0;
    }
    return dmg;
}

STATIC_OVL int
m_destroy_armor(mattk, mdef)
struct monst *mattk, *mdef;
{
    boolean udefend = (mdef == &youmonst),
            uattk = (mattk == &youmonst);
    int erodelvl = rnd(3);
    struct obj *oatmp;

    if (udefend ? Antimagic
                : (resists_magm(mdef) || defended(mdef, AD_MAGM))) {
        if (udefend) {
            shieldeff(u.ux, u.uy);
            monstseesu(M_SEEN_MAGR);
        } else {
            shieldeff(mdef->mx, mdef->my);
        }
        erodelvl = 1;
    }

    oatmp = some_armor(mdef);
    if (oatmp) {
        if (any_quest_artifact(oatmp)) {
            if (udefend || canseemon(mdef)) {
                if (!Blind)
                    pline("%s shines brightly.", The(xname(oatmp)));
                pline("%s is immune to %s destructive magic.",
                      The(xname(oatmp)), uattk ? "your" : "the");
            }
            return 0; /* no effect */
        } else if (oatmp->otyp == CRYSTAL_PLATE_MAIL) {
            if (udefend || canseemon(mdef)) {
                if (!Blind)
                    pline("%s glimmers brightly.", Yname2(oatmp));
                pline("%s is immune to %s destructive magic.",
                      Yname2(oatmp), uattk ? "your" : "the");
            }
            return 0; /* no effect */
        } else if (oatmp->oerodeproof) {
            if (!udefend && !canseemon(mdef)) {
                You("smell something strange.");
            } else if (!Blind) {
                pline("%s glows brown for a moment.", Yname2(oatmp));
            } else {
                pline("%s briefly emits an odd smell.", Yname2(oatmp));
            }
            maybe_erodeproof(oatmp, 0);
            erodelvl--;
        }

        if (greatest_erosion(oatmp) >= MAX_ERODE) {
            if (objects[oatmp->otyp].oc_oprop == DISINT_RES
                || obj_resists(oatmp, 0, 90))
                return 0;
            if (udefend) {
                destroy_arm(oatmp);
            } else {
                if (canseemon(mdef)) {
                    const char *action;

                    if (is_cloak(oatmp))
                        action = "crumbles and turns to dust";
                    else if (is_shirt(oatmp))
                        action = "crumbles into tiny threads";
                    else if (is_helmet(oatmp))
                        action = "turns to dust and is blown away";
                    else if (is_gloves(oatmp))
                        action = "vanish";
                    else if (is_boots(oatmp))
                        action = "disintegrate";
                    else if (is_shield(oatmp)) /* also handles bracers */
                        action = is_bracer(oatmp) ? "crumble away"
                                                  : "crumbles away";
                    else
                        action = "turns to dust";
                    pline("%s %s %s!", s_suffix(Monnam(mdef)),
                          xname(oatmp), action);
                }
                m_useupall(mdef, oatmp);
            }
        } else {
            int erodetype;

            if (is_corrodeable(oatmp))
                erodetype = ERODE_CORRODE;
            else if (is_flammable(oatmp))
                erodetype = ERODE_BURN;
            else if (is_glass(oatmp))
                erodetype = ERODE_FRACTURE;
            else if (is_supermaterial(oatmp))
                erodetype = ERODE_DETERIORATE;
            else
                erodetype = ERODE_ROT;

            while (erodelvl-- > 0) {
                (void) erode_obj(oatmp, (char *) 0, erodetype, EF_NONE);
            }
        }
    } else {
        if (udefend)
            Your("body itches.");
        else if (uattk || canseemon(mdef))
            pline("%s seems irritated.", Monnam(mdef));
    }
    update_inventory();

    return 0;
}

/* Cast a spell learned from a spellbook.
 * Returns 0 (damage is handled by the spell itself via mbhitm/etc).
 * Used by both MGC_LEARNED_SPELL and CLC_LEARNED_SPELL cases */
STATIC_OVL int
cast_learned_spell(caster, target)
struct monst *caster, *target;
{
    boolean youdefend = (target == &youmonst);
    short spell_otyp = mchoose_learned_spell(caster);

    if (spell_otyp == 0)
        return 0; /* No castable spell available */

    if (objects[spell_otyp].oc_class != SPBOOK_CLASS) {
        impossible("cast_learned_spell: invalid spell otyp %d", spell_otyp);
        return 0;
    }

    if (spell_otyp == SPE_FORCE_BOLT) {
        /* Force bolt: IMMEDIATE attack spell using ray mechanics.
           Like wand of striking, the bolt continues past the target
           and can affect objects in its path (boulders, statues,
           doors, fragile objects on the ground, etc). Requires line
           of sight at range (just like other ranged attacks) */
        int tx, ty;
        if (youdefend) {
            if (!lined_up(caster))
                return 0; /* No clear line of sight */
            tx = u.ux;
            ty = u.uy;
        } else if (target && !DEADMONSTER(target)) {
            if (!mlined_up(caster, target, FALSE))
                return 0; /* No clear line of sight */
            tx = target->mx;
            ty = target->my;
        } else {
            return 0;
        }
        mcast_force_bolt(caster, tx, ty);
    } else if (spell_otyp >= SPE_MAGIC_MISSILE
               && spell_otyp <= SPE_ACID_BLAST) {
        /* Ray spells: magic missile, fireball, cone of cold, sleep,
           finger of death, lightning, poison blast, acid blast */
        int tx, ty;
        if (youdefend) {
            if (!lined_up(caster))
                return 0; /* No clear line of sight */
            tx = u.ux;
            ty = u.uy;
        } else if (target && !DEADMONSTER(target)) {
            if (!mlined_up(caster, target, FALSE))
                return 0; /* No clear line of sight */
            tx = target->mx;
            ty = target->my;
        } else {
            return 0;
        }
        mcast_ray_spell(caster, tx, ty, spell_otyp);
    }

    return 0; /* spell handlers do their own damage */
}

/* monster wizard and cleric spellcasting functions

   Unified wizard spell handler for all caster/target combinations.
   caster: the monster (or &youmonst) casting the spell
   target: the monster (or &youmonst) being targeted
   dmg: base damage (0 means undirected/self-targeting spell)
   spellnum: which spell is being cast

   If modified, be sure to change is_undirected_spell() and
   spell_would_be_useless() */
STATIC_OVL
void
do_wizard_spell(caster, target, dmg, spellnum)
struct monst *caster, *target;
int dmg, spellnum;
{
    boolean yours = (caster == &youmonst);
    boolean youdefend = (target == &youmonst);
    boolean seecaster = (yours || canseemon(caster)
                         || tp_sensemon(caster) || Detect_monsters);
    int ml = yours ? mons[u.umonnum].mlevel : min(caster->m_lev, 50);

    if (dmg == 0 && !is_undirected_spell(AD_SPEL, spellnum)) {
        impossible("cast directed wizard spell (%d) with dmg=0?", spellnum);
        return;
    }

    switch (spellnum) {
    case MGC_DEATH_TOUCH:
        if (youdefend) {
            pline("Oh no, %s's using the touch of death!", mhe(caster));
            if (Death_resistance || immune_death_magic(youmonst.data)) {
                shieldeff(u.ux, u.uy);
                monstseesu(M_SEEN_DEATH);
                You("%s.", nonliving(youmonst.data)
                    ? "seem no more dead than before"
                    : "are unaffected");
            } else {
                if (Hallucination) {
                    You("have an out of body experience.");
                } else if (!Antimagic) {
                    killer.format = KILLED_BY_AN;
                    Strcpy(killer.name, "touch of death");
                    done(DIED);
                } else if (Antimagic) {
                    dmg = d(8, 6);
                    if (Antimagic || Half_spell_damage) {
                        shieldeff(u.ux, u.uy);
                        monstseesu(M_SEEN_MAGR);
                        dmg /= 2;
                    }
                    You("feel drained...");
                    u.uhpmax -= dmg / 3 + rn2(5);
                    losehp(dmg, "touch of death", KILLED_BY_AN);
                }
            }
            dmg = 0;
        } else {
            /* target is monster */
            boolean resisted;

            if (!target || DEADMONSTER(target))
                return;

            if (yours)
                You("are using the touch of death!");
            else if (seecaster) {
                char buf[BUFSZ];
                Sprintf(buf, "%s%s",
                        caster->mtame ? "Oh no, " : "", mhe(caster));
                if (!caster->mtame)
                    *buf = highc(*buf);
                pline("%s's using the touch of death!", buf);
            }

            resisted = ((resist(target, 0, 0, FALSE)
                         && rn2(mons[yours ? u.umonnum : caster->mnum].mlevel) <= 12)
                        || resists_magm(target) || defended(target, AD_MAGM));
            if (immune_death_magic(target->data) || is_vampshifter(target)) {
                shieldeff(target->mx, target->my);
                if (yours || canseemon(target))
                    pline("%s %s.", Monnam(target),
                          nonliving(target->data)
                              ? "seems no more dead than before"
                              : "is unaffected");
            } else if (!resisted) {
                target->mhp = -1;
                if (yours)
                    killed(target);
                else
                    monkilled(target, "", AD_SPEL);
                return;
            } else {
                shieldeff(target->mx, target->my);
                if (yours || canseemon(target)) {
                    if (target->mtame)
                        pline("Lucky for %s, it didn't work!", mon_nam(target));
                    else
                        pline("Well.  That didn't work...");
                }
            }
            dmg = 0;
        }
        break;
    case MGC_CANCELLATION:
        if (youdefend) {
            if (lined_up(caster)) {
                if (seecaster)
                    pline("%s %s a cancellation spell!", Monnam(caster),
                          rn2(2) ? "evokes" : "conjures up");
                else if (!Deaf)
                    You_hear("a powerful spell being %s.",
                             rn2(2) ? "spoken" : "uttered");
                (void) cancel_monst(&youmonst, (struct obj *) 0,
                                    FALSE, TRUE, FALSE);
            }
        } else {
            if (!target || DEADMONSTER(target))
                return;
            if (yours) {
                You("%s a cancellation spell!",
                    rn2(2) ? "evoke" : "conjure up");
            } else {
                if (mlined_up(caster, target, FALSE)) {
                    if (seecaster)
                        pline("%s %s a cancellation spell!", Monnam(caster),
                              rn2(2) ? "evokes" : "conjures up");
                }
            }
            (void) cancel_monst(target, (struct obj *) 0, FALSE, TRUE, FALSE);
        }
        dmg = 0;
        break;
    case MGC_REFLECTION:
        if (yours)
            (void) cast_reflection(&youmonst);
        else
            (void) cast_reflection(caster);
        dmg = 0;
        break;
    case MGC_ACID_BLAST:
        if (youdefend) {
            if (seecaster)
                pline("%s douses you in a torrent of acid!", Monnam(caster));
            explode(u.ux, u.uy, ZT_ACID, d((ml / 2) + 4, 8),
                    MON_CASTBALL, EXPL_ACID);
            if (how_resistant(ACID_RES) == 100) {
                shieldeff(u.ux, u.uy);
                pline("The acid dissipates harmlessly.");
                monstseesu(M_SEEN_ACID);
            }
            if (rn2(u.twoweap ? 2 : 3))
                acid_damage(uwep);
            if (u.twoweap && rn2(2))
                acid_damage(uswapwep);
            if (rn2(4))
                erode_armor(&youmonst, ERODE_CORRODE);
        } else {
            if (!target || DEADMONSTER(target))
                return;
            if (yours)
                You("douse %s in a torrent of acid!", mon_nam(target));
            else if (seecaster)
                pline("%s douses %s in a torrent of acid!",
                      Monnam(caster), mon_nam(target));
            explode(target->mx, target->my, ZT_ACID,
                    d((ml / 2) + 4, 8),
                    yours ? 0 : MON_CASTBALL, EXPL_ACID);
            if (resists_acid(target) || defended(target, AD_ACID)) {
                shieldeff(target->mx, target->my);
                if (canseemon(target))
                    pline("But the acid dissipates harmlessly.");
            }
            if (rn2(4))
                erode_armor(target, ERODE_CORRODE);
        }
        dmg = 0;
        break;
    case MGC_CLONE_WIZ:
        /* Only monster Wizard can clone */
        if (!yours && caster->iswiz && context.no_of_wizards == 1) {
            pline("Double Trouble...");
            clonewiz();
        } else if (!yours)
            impossible("bad wizard cloning?");
        /* Player polymorphed into Wizard can't clone */
        dmg = 0;
        break;
    case MGC_SUMMON_MONS:
        if (youdefend) {
            /* Monster summoning against player */
            int count = nasty(caster, FALSE);
            if (!count) {
                ; /* nothing was created */
            } else if (caster->iswiz) {
                verbalize("Destroy the thief, my pet%s!", plur(count));
            } else if (caster->data == &mons[PM_KATHRYN_THE_ICE_QUEEN]) {
                verbalize("Defend me, my minion%s!", plur(count));
            } else {
                boolean one = (count == 1);
                const char *mappear = one ? "A monster appears"
                                          : "Monsters appear";
                if (Invis && !mon_prop(caster, SEE_INVIS)
                    && (caster->mux != u.ux || caster->muy != u.uy))
                    pline("%s %s a spot near you!", mappear, one ? "at" : "around");
                else if (Displaced && (caster->mux != u.ux || caster->muy != u.uy))
                    pline("%s %s your displaced image!", mappear, one ? "by" : "around");
                else
                    pline("%s from nowhere!", mappear);
            }
        } else {
            /* Player or monster summoning monsters */
            int count = 0;
            struct monst *mpet;

            if (!rn2(10) && Inhell) {
                if (yours)
                    demonpet();
                else
                    msummon(caster);
            } else {
                int i, j, makeindex, tmp = (u.ulevel > 3) ? u.ulevel / 3 : 1;
                coord bypos;

                if (target)
                    bypos.x = target->mx, bypos.y = target->my;
                else if (yours)
                    bypos.x = u.ux, bypos.y = u.uy;
                else
                    bypos.x = caster->mx, bypos.y = caster->my;

                for (i = rnd(tmp); i > 0; --i) {
                    for (j = 0; j < 20; j++) {
                        do {
                            makeindex = pick_nasty();
                        } while (attacktype(&mons[makeindex], AT_MAGC)
                                 && mons[makeindex].difficulty >= mons[u.umonnum].difficulty);
                        if (yours && !enexto(&bypos, u.ux, u.uy, &mons[makeindex]))
                            continue;
                        if (!yours && !enexto(&bypos, caster->mx, caster->my, &mons[makeindex]))
                            continue;
                        mpet = makemon(&mons[makeindex], bypos.x, bypos.y,
                                       (yours || caster->mtame) ? MM_EDOG : NO_MM_FLAGS);
                        if (mpet) {
                            mpet->msleeping = 0;
                            if (yours || caster->mtame)
                                initedog(mpet, TRUE);
                            else if (caster->mpeaceful)
                                mpet->mpeaceful = 1;
                            else
                                mpet->mpeaceful = mpet->mtame = 0;
                            set_malign(mpet);
                        } else {
                            mpet = makemon((struct permonst *) 0,
                                           bypos.x, bypos.y, NO_MM_FLAGS);
                        }
                        if (mpet && (u.ualign.type == 0
                                     || mon_aligntyp(mpet) == 0
                                     || sgn(mon_aligntyp(mpet)) == sgn(u.ualign.type))) {
                            count++;
                            break;
                        }
                    }
                }
                if (count > 0 && (yours || canseemon(target)))
                    pline("%s from nowhere!",
                          (count == 1) ? "A monster appears" : "Monsters appear");
            }
        }
        dmg = 0;
        break;
    case MGC_AGGRAVATION:
        if (youdefend) {
            You_feel("that monsters are aware of your presence.");
            aggravate();
        } else {
            if (!target || DEADMONSTER(target))
                return;
            you_aggravate(target);
        }
        dmg = 0;
        break;
    case MGC_CURSE_ITEMS:
        if (youdefend) {
            You_feel("as if you need some help.");
            rndcurse();
        } else {
            if (!target || DEADMONSTER(target))
                return;
            if (yours || canseemon(target))
                You_feel("as though %s needs some help.", mon_nam(target));
            mrndcurse(target);
        }
        dmg = 0;
        break;
    case MGC_DESTRY_ARMR:
        if (youdefend)
            dmg = m_destroy_armor(caster, &youmonst);
        else {
            if (!target || DEADMONSTER(target))
                return;
            dmg = m_destroy_armor(caster, target);
        }
        break;
    case MGC_WEAKEN_YOU:
        if (youdefend) {
            if (Antimagic) {
                shieldeff(u.ux, u.uy);
                monstseesu(M_SEEN_MAGR);
                You_feel("momentarily weakened.");
            } else {
                You("suddenly feel weaker!");
                dmg = ml - 6;
                if (Half_spell_damage)
                    dmg = (dmg + 1) / 2;
                losestr(rnd(dmg));
                if (u.uhp < 1)
                    done_in_by(caster, DIED);
            }
        } else {
            if (!target || DEADMONSTER(target))
                return;
            if (resist(target, 0, 0, FALSE)) {
                shieldeff(target->mx, target->my);
                if (canseemon(target))
                    pline("%s looks momentarily weakened.", Monnam(target));
            } else {
                if (yours || canseemon(target))
                    pline("%s suddenly seems weaker!", Monnam(target));
                /* monsters don't have strength, so drain max hp instead */
                target->mhpmax -= dmg;
                if ((target->mhp -= dmg) <= 0) {
                    if (yours)
                        killed(target);
                    else
                        monkilled(target, "", AD_SPEL);
                    return;
                }
            }
        }
        dmg = 0;
        break;
    case MGC_DISAPPEAR:
        /* Self-targeting: caster goes invisible */
        if (yours) {
            if (!(HInvis & INTRINSIC)) {
                HInvis |= FROMOUTSIDE;
                if (!Blind && !BInvis)
                    self_invis_message();
            }
        } else {
            if (!caster->minvis && !caster->invis_blkd) {
                if (canseemon(caster))
                    pline("%s suddenly %s!", Monnam(caster),
                          !See_invisible ? "disappears" : "becomes transparent");
                mon_set_minvis(caster);
                if (cansee(caster->mx, caster->my) && !canspotmon(caster))
                    map_invisible(caster->mx, caster->my);
            } else
                impossible("no reason for monster to cast disappear spell?");
        }
        dmg = 0;
        break;
    case MGC_STUN_YOU:
        if (youdefend) {
            if (Antimagic || Free_action || Hidinshell) {
                shieldeff(u.ux, u.uy);
                if (!(Stunned || Stun_resistance
                      || wielding_artifact(ART_TEMPEST)))
                    You_feel("momentarily disoriented.");
                make_stunned(1L, FALSE);
            } else {
                if (!(Stun_resistance || wielding_artifact(ART_TEMPEST)))
                    You(Stunned ? "struggle to keep your balance." : "reel...");
                dmg = d(ACURR(A_DEX) < 12 ? 6 : 4, 4);
                if (Half_spell_damage)
                    dmg = (dmg + 1) / 2;
                make_stunned((HStun & TIMEOUT) + (long) dmg, FALSE);
            }
        } else {
            if (!target || DEADMONSTER(target))
                return;
            if (resist(target, 0, 0, FALSE)) {
                shieldeff(target->mx, target->my);
                if (yours || canseemon(target)) {
                    if (resists_stun(target->data) || defended(target, AD_STUN)
                        || (MON_WEP(target)
                            && MON_WEP(target)->oartifact == ART_TEMPEST))
                        pline("%s seems momentarily disoriented.", Monnam(target));
                }
            } else {
                if (yours || canseemon(target)) {
                    if (target->mstun)
                        pline("%s struggles to keep %s balance.",
                              Monnam(target), mhis(target));
                    else
                        pline("%s reels...", Monnam(target));
                }
                target->mstun = 1;
            }
        }
        dmg = 0;
        break;
    case MGC_HASTE_SELF:
        /* Self-targeting: caster speeds up */
        if (yours) {
            if (!(HFast & INTRINSIC)) {
                You("are suddenly moving faster.");
                HFast |= INTRINSIC;
            }
        } else {
            mon_adjust_speed(caster, 1, (struct obj *) 0);
        }
        dmg = 0;
        break;
    case MGC_CURE_SELF:
        /* Self-targeting: caster heals */
        if (yours) {
            if (u.mh < u.mhmax) {
                You("feel better.");
                if ((u.mh += d(3, 6)) > u.mhmax)
                    u.mh = u.mhmax;
                context.botl = 1;
            }
        } else {
            dmg = m_cure_self(caster, dmg);
        }
        dmg = 0;
        break;
    case MGC_FIRE_BOLT:
    case MGC_ICE_BOLT:
        if (youdefend) {
            if (seecaster)
                pline("%s blasts you with %s!", Monnam(caster),
                      (spellnum == MGC_FIRE_BOLT) ? "fire" : "ice");
            explode(u.ux, u.uy,
                    (spellnum == MGC_FIRE_BOLT) ? ZT_FIRE : ZT_COLD,
                    resist_reduce(d((ml / 5) + 1, 8),
                                  (spellnum == MGC_FIRE_BOLT) ? FIRE_RES : COLD_RES),
                    MON_CASTBALL,
                    (spellnum == MGC_FIRE_BOLT) ? EXPL_FIERY : EXPL_FROSTY);
            if (how_resistant((spellnum == MGC_FIRE_BOLT) ? FIRE_RES : COLD_RES) == 100) {
                shieldeff(u.ux, u.uy);
                monstseesu((spellnum == MGC_FIRE_BOLT) ? M_SEEN_FIRE : M_SEEN_COLD);
                pline("But you resist the effects.");
            }
        } else {
            if (!target || DEADMONSTER(target))
                return;
            if (yours)
                You("blast %s with %s!", mon_nam(target),
                    (spellnum == MGC_FIRE_BOLT) ? "fire" : "ice");
            else if (seecaster)
                pline("%s blasts %s with %s!", Monnam(caster), mon_nam(target),
                      (spellnum == MGC_FIRE_BOLT) ? "fire" : "ice");
            explode(target->mx, target->my,
                    (spellnum == MGC_FIRE_BOLT) ? ZT_FIRE : ZT_COLD,
                    d((ml / 5) + 1, 8),
                    yours ? 0 : MON_CASTBALL,
                    (spellnum == MGC_FIRE_BOLT) ? EXPL_FIERY : EXPL_FROSTY);
            if (spellnum == MGC_FIRE_BOLT
                && (resists_fire(target) || defended(target, AD_FIRE))) {
                shieldeff(target->mx, target->my);
                if (canseemon(target))
                    pline("But %s seems unaffected by the fire.", mon_nam(target));
            } else if (spellnum == MGC_ICE_BOLT
                       && (resists_cold(target) || defended(target, AD_COLD))) {
                shieldeff(target->mx, target->my);
                if (canseemon(target))
                    pline("But %s seems unaffected by the cold.", mon_nam(target));
            }
        }
        dmg = 0;
        break;
    case MGC_PSI_BOLT:
        if (youdefend) {
            if (Antimagic) {
                shieldeff(u.ux, u.uy);
                monstseesu(M_SEEN_MAGR);
                dmg = (dmg + 1) / 2;
            }
            if (dmg <= 5)
                You("get a slight %sache.", body_part(HEAD));
            else if (dmg <= 10)
                Your("brain is on fire!");
            else if (dmg <= 20)
                Your("%s suddenly aches painfully!", body_part(HEAD));
            else
                Your("%s suddenly aches very painfully!", body_part(HEAD));
        } else {
            if (!target || DEADMONSTER(target))
                return;
            if (resist(target, 0, 0, FALSE)) {
                shieldeff(target->mx, target->my);
                dmg = (dmg + 1) / 2;
            }
            if (yours || canseemon(target))
                pline("%s %s%s", Monnam(target),
                      can_flollop(target->data) ? "flollops" : "winces",
                      (dmg <= 5) ? "." : "!");
        }
        break;
    case MGC_LEARNED_SPELL:
        dmg = cast_learned_spell(caster, target);
        break;
    default:
        impossible("do_wizard_spell: invalid magic spell (%d)", spellnum);
        dmg = 0;
        break;
    }

    /* Apply remaining damage */
    if (dmg) {
        if (youdefend)
            mdamageu(caster, dmg);
        else {
            target->mhp -= dmg;
            if (DEADMONSTER(target)) {
                if (yours)
                    killed(target);
                else
                    monkilled(target, "", AD_SPEL);
            }
        }
    }
}

const char* vulntext[] = {
        "chartreuse polka-dot",
        "reddish-orange",
        "purplish-blue",
        "coppery-yellow",
        "greenish-mottled"
};

/* Unified cleric spell handler for both monster-vs-player and
 * player/monster-vs-monster.
 * caster: the monster (or &youmonst for player) casting the spell
 * target: the target monster (or &youmonst for player)
 * dmg: base damage
 * spellnum: which cleric spell
 */
STATIC_OVL
void
do_cleric_spell(caster, target, dmg, spellnum)
struct monst *caster, *target;
int dmg, spellnum;
{
    boolean yours = (caster == &youmonst);
    boolean youdefend = (target == &youmonst);
    boolean seecaster = yours || canseemon(caster)
                        || tp_sensemon(caster) || Detect_monsters;
    int ml = yours ? mons[u.umonnum].mlevel : min(caster->m_lev, 50);
    int aligntype;
    static const char *Moloch = "Moloch";
    struct monst *minion = (struct monst *) 0;
    coord mm;

    if (dmg == 0 && !is_undirected_spell(AD_CLRC, spellnum)) {
        impossible("cast directed cleric spell (%d) with dmg=0?", spellnum);
        return;
    }

    switch (spellnum) {
    case CLC_SUMMON_MINION:
        if (youdefend) {
            /* Monster casting at player - can summon near player */
            if (caster->data == &mons[PM_KATHRYN_THE_ICE_QUEEN]) {
                coord bypos;
                if (!enexto(&bypos, caster->mx, caster->my, &mons[PM_SNOW_GOLEM]))
                    break;
                minion = makemon(&mons[PM_SNOW_GOLEM], bypos.x, bypos.y,
                                 MM_ANGRY);
                if (minion && canspotmon(minion))
                    pline("A minion of %s appears!", mon_nam(caster));
            } else {
                aligntype = mon_aligntyp(caster);
                minion = summon_minion(aligntype, FALSE);
                if (minion) {
                    boolean vassal = (aligntype == A_NONE);
                    set_malign(minion);
                    if (canspotmon(minion))
                        pline("A %s of %s appears!",
                              vassal ? "vassal" : "servant",
                              vassal ? Moloch : aligns[1 - aligntype].noun);
                }
            }
        } else {
            /* Player/monster casting at monster */
            if (!target || DEADMONSTER(target))
                return;
            /* monster vs monster is suppressed, as summon_minion()
               currently does not support anything but the player
               as a target */
            if (!yours)
                return;
            aligntype = yours ? u.ualign.type : mon_aligntyp(caster);
            minion = summon_minion(aligntype, FALSE);
            if (minion && canspotmon(minion))
                pline("A %s of %s appears!",
                      aligntype == A_NONE ? "vassal" : "servant",
                      aligntype == A_NONE ? Moloch : aligns[1 - aligntype].noun);
        }
        dmg = 0;
        break;
    case CLC_GEYSER:
        /* this is physical damage (force not heat), not magical damage */
        if (youdefend) {
            if (caster->data == &mons[PM_KATHRYN_THE_ICE_QUEEN]
                || caster->data == &mons[PM_ASMODEUS]
                || caster->data == &mons[PM_VECNA]) {
                pline("An avalanche of ice and snow slams into you from nowhere!");
                dmg = d(8, 8);
                if (Half_physical_damage)
                    dmg = (dmg + 1) / 2;
                destroy_item(POTION_CLASS, AD_COLD);
            } else {
                pline("A sudden geyser slams into you from nowhere!");
                dmg = d(8, 6);
                if (Half_physical_damage)
                    dmg = (dmg + 1) / 2;
                if (u.umonnum == PM_IRON_GOLEM) {
                    You("rust!");
                    rehumanize();
                    break;
                }
                (void) erode_armor(&youmonst, ERODE_RUST);
            }
        } else {
            if (!target || DEADMONSTER(target))
                return;
            if (yours || canseemon(target))
                pline("A sudden geyser slams into %s from nowhere!",
                      mon_nam(target));
            dmg = d(8, 6);
            (void) erode_armor(target, ERODE_RUST);
        }
        break;
    case CLC_FIRE_PILLAR:
        if (youdefend) {
            pline("A pillar of fire strikes all around you!");
            if (how_resistant(FIRE_RES) == 100) {
                shieldeff(u.ux, u.uy);
                monstseesu(M_SEEN_FIRE);
                dmg = 0;
            } else
                dmg = resist_reduce(d(8, 6), FIRE_RES);
            if (Half_spell_damage)
                dmg = (dmg + 1) / 2;
            burn_away_slime();
            (void) burnarmor(&youmonst);
            destroy_item(SCROLL_CLASS, AD_FIRE);
            destroy_item(POTION_CLASS, AD_FIRE);
            destroy_item(SPBOOK_CLASS, AD_FIRE);
            (void) burn_floor_objects(u.ux, u.uy, TRUE, FALSE);
        } else {
            if (!target || DEADMONSTER(target))
                return;
            if (yours || canseemon(target))
                pline("A pillar of fire strikes all around %s!",
                      mon_nam(target));
            if (resists_fire(target) || defended(target, AD_FIRE)) {
                shieldeff(target->mx, target->my);
                dmg = 0;
            } else
                dmg = d(8, 6);
            (void) burnarmor(target);
            destroy_mitem(target, SCROLL_CLASS, AD_FIRE);
            destroy_mitem(target, POTION_CLASS, AD_FIRE);
            destroy_mitem(target, SPBOOK_CLASS, AD_FIRE);
            (void) burn_floor_objects(target->mx, target->my, TRUE, FALSE);
        }
        break;
    case CLC_LIGHTNING: {
        boolean reflects;

        if (youdefend) {
            pline("A bolt of lightning strikes down at you from above!");
            reflects = ureflects("Some of it bounces off your %s%s.", "");
            if (reflects || (how_resistant(SHOCK_RES) == 100)) {
                shieldeff(u.ux, u.uy);
                if (reflects) {
                    dmg = resist_reduce(d(4, 6), SHOCK_RES);
                    monstseesu(M_SEEN_REFL);
                }
                if (how_resistant(SHOCK_RES) == 100) {
                    pline("You aren't shocked.");
                    monstseesu(M_SEEN_ELEC);
                    dmg = 0;
                }
            } else {
                dmg = resist_reduce(d(8, 6), SHOCK_RES);
            }
            if (dmg && Half_spell_damage)
                dmg = (dmg + 1) / 2;
            if (!reflects) {
                destroy_item(WAND_CLASS, AD_ELEC);
                destroy_item(RING_CLASS, AD_ELEC);
                (void) flashburn((long) rnd(100));
            }
        } else {
            if (!target || DEADMONSTER(target))
                return;
            if (yours || canseemon(target))
                pline("A bolt of lightning strikes down at %s from above!",
                      mon_nam(target));
            reflects = mon_reflects(target, "It bounces off %s %s.");
            if (reflects || resists_elec(target) || defended(target, AD_ELEC)) {
                shieldeff(target->mx, target->my);
                dmg = 0;
                if (reflects)
                    break;
            } else
                dmg = d(8, 6);
            destroy_mitem(target, WAND_CLASS, AD_ELEC);
            destroy_mitem(target, RING_CLASS, AD_ELEC);
        }
        break;
    }
    case CLC_CURSE_ITEMS:
        if (youdefend) {
            You_feel("as if you need some help.");
            rndcurse();
        } else {
            if (!target || DEADMONSTER(target))
                return;
            if (yours || canseemon(target))
                You_feel("as though %s needs some help.", mon_nam(target));
            mrndcurse(target);
        }
        dmg = 0;
        break;
    case CLC_CALL_UNDEAD:
        if (youdefend) {
            mm.x = u.ux;
            mm.y = u.uy;
            pline("Undead creatures are called forth from the grave!");
            mkundead(&mm, FALSE, NO_MINVENT);
        } else {
            if (yours) {
                mm.x = u.ux;
                mm.y = u.uy;
                pline("Undead creatures are called forth from the grave!");
            } else {
                mm.x = caster->mx;
                mm.y = caster->my;
                if (canseemon(caster))
                    pline("%s calls forth undead creatures from the grave!",
                          Monnam(caster));
            }
            mkundead(&mm, FALSE, NO_MINVENT);
        }
        dmg = 0;
        break;
    case CLC_INSECTS: {
        /* Try for insects, and if there are none left, go for snakes */
        boolean spiders = youdefend && (caster->data == &mons[PM_LOLTH]
                                        || racial_drow(caster));
        struct permonst *pm = mkclass(spiders ? S_SPIDER : S_ANT, 0);
        struct monst *mtmp2 = (struct monst *) 0;
        char let = (pm ? (spiders ? S_SPIDER : S_ANT) : S_SNAKE);
        boolean success = FALSE;
        int i, quan, oldseen, newseen;
        coord bypos;
        const char *fmt;

        if (youdefend) {
            /* Monster casting at player - hostile insects */
            oldseen = monster_census(TRUE);
            quan = (ml < 2) ? 1 : rnd((int) ml / 2);
            if (quan < 3)
                quan = 3;
            for (i = 0; i <= quan; i++) {
                if ((pm = mkclass(let, 0)) != 0) {
                    if (!enexto(&bypos, caster->mux, caster->muy, pm))
                        break;
                    if ((mtmp2 = makemon(pm, bypos.x, bypos.y, MM_ANGRY)) != 0) {
                        success = TRUE;
                        mtmp2->msleeping = mtmp2->mpeaceful = mtmp2->mtame = 0;
                        mtmp2->minsects = 1;
                        set_malign(mtmp2);
                    }
                }
            }
            newseen = monster_census(TRUE);

            fmt = 0;
            if (!seecaster) {
                char *arg;
                const char *what = (let == S_SNAKE) ? "snake"
                                                    : (let == S_SPIDER) ? "arachnid"
                                                                        : "insect";
                if (newseen <= oldseen || Unaware) {
                    You_hear("someone summoning %s.", makeplural(what));
                } else {
                    arg = (newseen == oldseen + 1) ? an(what) : makeplural(what);
                    if (!Deaf)
                        You_hear("someone summoning something, and %s %s.", arg,
                                 vtense(arg, "appear"));
                    else
                        pline("%s %s.", upstart(arg), vtense(arg, "appear"));
                }
            } else if (!success)
                fmt = "%s casts at a clump of sticks, but nothing happens.";
            else if (let == S_SNAKE)
                fmt = "%s transforms a clump of sticks into snakes!";
            else if (let == S_SPIDER)
                fmt = "%s summons arachnids!";
            else if (Invis && !mon_prop(caster, SEE_INVIS)
                     && (caster->mux != u.ux || caster->muy != u.uy))
                fmt = "%s summons insects around a spot near you!";
            else if (Displaced && (caster->mux != u.ux || caster->muy != u.uy))
                fmt = "%s summons insects around your displaced image!";
            else
                fmt = "%s summons insects!";
            if (fmt)
                pline(fmt, Monnam(caster));
        } else {
            /* Player/monster casting at monster - tame/peaceful insects */
            if (!target || DEADMONSTER(target))
                return;
            if (target)
                bypos.x = target->mx, bypos.y = target->my;
            else if (yours)
                bypos.x = u.ux, bypos.y = u.uy;
            else
                bypos.x = caster->mx, bypos.y = caster->my;

            quan = (mons[u.umonnum].mlevel < 2) ? 1 :
                    rnd(mons[u.umonnum].mlevel / 2);
            if (quan < 3)
                quan = 3;
            success = pm ? TRUE : FALSE;
            for (i = 0; i <= quan; i++) {
                if ((pm = mkclass(let, 0)) != 0) {
                    if (!enexto(&bypos, target->mx, target->my, pm))
                        break;
                    if ((mtmp2 = makemon(pm, bypos.x, bypos.y, NO_MM_FLAGS)) != 0) {
                        success = TRUE;
                        mtmp2->msleeping = 0;
                        mtmp2->minsects = 1;
                        if (yours || caster->mtame)
                            (void) tamedog(mtmp2, (struct obj *) 0);
                        else if (caster->mpeaceful)
                            mtmp2->mpeaceful = 1;
                        else
                            mtmp2->mpeaceful = 0;
                        set_malign(mtmp2);
                    }
                }
            }

            if (yours) {
                if (!success)
                    You("cast at a clump of sticks, but nothing happens.");
                else if (let == S_SNAKE)
                    You("transform a clump of sticks into snakes!");
                else
                    You("summon insects!");
            } else if (canseemon(caster)) {
                if (!success)
                    pline("%s casts at a clump of sticks, but nothing happens.",
                          Monnam(caster));
                else if (let == S_SNAKE)
                    pline("%s transforms a clump of sticks into snakes!",
                          Monnam(caster));
                else
                    pline("%s summons insects!", Monnam(caster));
            }
        }
        dmg = 0;
        break;
    }
    case CLC_BLIND_YOU:
        if (youdefend) {
            /* note: resists_blnd() doesn't apply here */
            if (!Blinded) {
                int num_eyes = eyecount(youmonst.data);
                pline("Scales cover your %s!", (num_eyes == 1)
                                                   ? body_part(EYE)
                                                   : makeplural(body_part(EYE)));
                make_blinded(Half_spell_damage ? 100L : 200L, FALSE);
                if (!Blind)
                    Your1(vision_clears);
            } else
                impossible("no reason for monster to cast blindness spell?");
        } else {
            if (!target || DEADMONSTER(target))
                return;
            /* note: resists_blnd() doesn't apply here */
            if (!target->mblinded && haseyes(target->data)) {
                if (!resists_blnd(target)) {
                    int num_eyes = eyecount(target->data);
                    if (yours || canseemon(target))
                        pline("Scales cover %s %s!", s_suffix(mon_nam(target)),
                              (num_eyes == 1) ? "eye" : "eyes");
                    target->mblinded = 127;
                }
            }
        }
        dmg = 0;
        break;
    case CLC_PARALYZE:
        if (youdefend) {
            if (Antimagic || Free_action) {
                shieldeff(u.ux, u.uy);
                if (multi >= 0)
                    You("stiffen briefly.");
                nomul(-1);
                multi_reason = "paralyzed by a monster";
            } else {
                if (multi >= 0)
                    You("are frozen in place!");
                dmg = 4 + (int) ml;
                if (Half_spell_damage)
                    dmg = (dmg + 1) / 2;
                nomul(-dmg);
                multi_reason = "paralyzed by a monster";
            }
            nomovemsg = 0;
        } else {
            if (!target || DEADMONSTER(target))
                return;
            if (resist(target, 0, 0, FALSE)) {
                shieldeff(target->mx, target->my);
                if (yours || canseemon(target))
                    pline("%s stiffens briefly.", Monnam(target));
            } else {
                if (yours || canseemon(target))
                    pline("%s is frozen in place!", Monnam(target));
                dmg = 4 + (yours ? mons[u.umonnum].mlevel : caster->m_lev);
                target->mcanmove = 0;
                target->mfrozen = dmg;
            }
        }
        dmg = 0;
        break;
    case CLC_CONFUSE_YOU:
        if (youdefend) {
            if (Antimagic) {
                shieldeff(u.ux, u.uy);
                monstseesu(M_SEEN_MAGR);
                You_feel("momentarily dizzy.");
            } else {
                boolean oldprop = !!Confusion;
                dmg = (int) ml;
                if (Half_spell_damage)
                    dmg = (dmg + 1) / 2;
                make_confused(HConfusion + dmg, TRUE);
                if (Hallucination)
                    You_feel("%s!", oldprop ? "trippier" : "trippy");
                else
                    You_feel("%sconfused!", oldprop ? "more " : "");
            }
        } else {
            if (!target || DEADMONSTER(target))
                return;
            if (resist(target, 0, 0, FALSE)) {
                shieldeff(target->mx, target->my);
                if (yours || canseemon(target))
                    pline("%s seems momentarily dizzy.", Monnam(target));
            } else {
                if (yours || canseemon(target))
                    pline("%s seems %sconfused!", Monnam(target),
                          target->mconf ? "more " : "");
                target->mconf = 1;
            }
        }
        dmg = 0;
        break;
    case CLC_CURE_SELF:
        if (yours) {
            if (u.mh < u.mhmax) {
                You("feel better.");
                /* note: player healing does 6d4; this does 3d6 */
                if ((u.mh += d(3, 6)) > u.mhmax)
                    u.mh = u.mhmax;
                context.botl = 1;
            }
        } else {
            /* Monster caster healing itself */
            dmg = m_cure_self(caster, dmg);
        }
        dmg = 0;
        break;
    case CLC_VULN_YOU: {
        int i = rnd(4);

        if (youdefend) {
            pline("A %s film oozes over your %s!",
                  Blind ? "slimy" : vulntext[i], body_part(SKIN));
            switch (i) {
            case 1:
                if (caster->data == &mons[PM_KATHRYN_THE_ICE_QUEEN]
                    || caster->data == &mons[PM_ASMODEUS]
                    || caster->data == &mons[PM_VECNA]) {
                    if (Vulnerable_cold)
                        break;
                    incr_itimeout(&HVulnerable_cold, rnd(100) + 150);
                    You_feel("extremely chilly.");
                    break;
                } else {
                    if (Vulnerable_fire)
                        break;
                    incr_itimeout(&HVulnerable_fire, rnd(100) + 150);
                    You_feel("more inflammable.");
                    break;
                }
                break;
            case 2:
                if (Vulnerable_cold)
                    break;
                incr_itimeout(&HVulnerable_cold, rnd(100) + 150);
                You_feel("extremely chilly.");
                break;
            case 3:
                if (Vulnerable_elec)
                    break;
                incr_itimeout(&HVulnerable_elec, rnd(100) + 150);
                You_feel("overly conductive.");
                break;
            case 4:
                if (Vulnerable_acid)
                    break;
                incr_itimeout(&HVulnerable_acid, rnd(100) + 150);
                You_feel("easily corrodable.");
                break;
            default:
                break;
            }
        } else {
            if (!target || DEADMONSTER(target))
                return;
            if (yours || canseemon(target)) {
                pline("A %s film oozes over %s exterior!",
                      Blind ? "slimy" : vulntext[i], mhis(target));
                switch (rnd(4)) {
                case 1:
                    if ((target->data->mflags4 & M4_VULNERABLE_FIRE) != 0
                        || target->vuln_fire)
                        break;
                    target->vuln_fire = rnd(100) + 150;
                    pline("%s is more inflammable.", Monnam(target));
                    break;
                case 2:
                    if ((target->data->mflags4 & M4_VULNERABLE_COLD) != 0
                        || target->vuln_cold)
                        break;
                    target->vuln_cold = rnd(100) + 150;
                    pline("%s is extremely chilly.", Monnam(target));
                    break;
                case 3:
                    if ((target->data->mflags4 & M4_VULNERABLE_ELEC) != 0
                        || target->vuln_elec)
                        break;
                    target->vuln_elec = rnd(100) + 150;
                    pline("%s is overly conductive.", Monnam(target));
                    break;
                case 4:
                    if ((target->data->mflags4 & M4_VULNERABLE_ACID) != 0
                        || target->vuln_acid)
                        break;
                    target->vuln_acid = rnd(100) + 150;
                    pline("%s is easily corrodable.", Monnam(target));
                    break;
                default:
                    break;
                }
            }
        }
        dmg = 0;
        break;
    }
    case CLC_OPEN_WOUNDS:
        if (youdefend) {
            dmg = d((int) ((ml / 2) + 1), 6);
            if (Antimagic) {
                shieldeff(u.ux, u.uy);
                monstseesu(M_SEEN_MAGR);
                dmg = (dmg + 1) / 2;
            }
            if (dmg <= 5)
                Your("skin itches badly for a moment.");
            else if (dmg <= 10)
                pline("Wounds appear on your body!");
            else if (dmg <= 20)
                pline("Severe wounds appear on your body!");
            else
                Your("body is covered with painful wounds!");
        } else {
            if (!target || DEADMONSTER(target))
                return;
            dmg = d((int) ((ml / 2) + 1), 6);
            if (resist(target, 0, 0, FALSE)) {
                shieldeff(target->mx, target->my);
                dmg = (dmg + 1) / 2;
            }
            if (yours || canseemon(target)) {
                if (dmg <= 5)
                    pline("%s looks itchy!", Monnam(target));
                else if (dmg <= 10)
                    pline("Wounds appear on %s!", mon_nam(target));
                else if (dmg <= 20)
                    pline("Severe wounds appear on %s!", mon_nam(target));
                else
                    pline("%s is covered in wounds!", Monnam(target));
            }
        }
        break;
    case CLC_PROTECTION: {
        if (yours) {
            /* Player casting protection on self */
            (void) cast_protection();
        } else {
            /* Monster casting protection on self */
            int natac = find_mac(caster) + caster->mprotection;
            int loglev = 0;
            int gain = 0;

            for (; ml > 0; ml /= 2)
                loglev++;

            gain = loglev - caster->mprotection / (4 - min(3, (10 - natac) / 10));

            if (caster->mpeaceful && caster->ispriest && inhistemple(caster)) {
                ; /* cut down on the temple spam */
            } else {
                if (gain && seecaster) {
                    if (caster->mprotection) {
                        pline_The("%s haze around %s becomes more dense.",
                                  hcolor(NH_GOLDEN), mon_nam(caster));
                    } else {
                        caster->mprottime = (caster->iswiz || is_prince(caster->data)
                                           || caster->data->msound == MS_NEMESIS
                                           || caster->data->msound == MS_LEADER)
                                           ? 20 : 10;
                        pline_The("air around %s begins to shimmer with a %s haze.",
                                  mon_nam(caster), hcolor(NH_GOLDEN));
                    }
                }
            }
            caster->mprotection += gain;
        }
        dmg = 0;
        break;
    }
    case CLC_BARKSKIN:
        if (yours)
            (void) cast_barkskin(&youmonst);
        else
            (void) cast_barkskin(caster);
        dmg = 0;
        break;
    case CLC_STONESKIN:
        if (yours)
            (void) cast_stoneskin(&youmonst);
        else
            (void) cast_stoneskin(caster);
        dmg = 0;
        break;
    case CLC_LEARNED_SPELL:
        dmg = cast_learned_spell(caster, target);
        break;
    default:
        impossible("mcastu: invalid clerical spell (%d)", spellnum);
        dmg = 0;
        break;
    }

    /* Apply damage */
    if (dmg) {
        if (youdefend) {
            mdamageu(caster, dmg);
        } else {
            target->mhp -= dmg;
            if (DEADMONSTER(target)) {
                if (yours)
                    killed(target);
                else
                    monkilled(target, "", AD_CLRC);
            }
        }
    }
}

STATIC_OVL
boolean
is_undirected_spell(adtyp, spellnum)
unsigned int adtyp;
int spellnum;
{
    if (adtyp == AD_SPEL) {
        switch (spellnum) {
        case MGC_CLONE_WIZ:
        case MGC_SUMMON_MONS:
        case MGC_ACID_BLAST:
        case MGC_AGGRAVATION:
        case MGC_DISAPPEAR:
        case MGC_HASTE_SELF:
        case MGC_CURE_SELF:
        case MGC_FIRE_BOLT:
        case MGC_ICE_BOLT:
        case MGC_CANCELLATION:
        case MGC_REFLECTION:
            return TRUE;
        default:
            break;
        }
    } else if (adtyp == AD_CLRC) {
        switch (spellnum) {
        case CLC_INSECTS:
        case CLC_OPEN_WOUNDS:
        case CLC_CURE_SELF:
        case CLC_PROTECTION:
        case CLC_BARKSKIN:
        case CLC_STONESKIN:
        case CLC_VULN_YOU:
        case CLC_SUMMON_MINION:
        case CLC_CALL_UNDEAD:
            return TRUE;
        default:
            break;
        }
    }
    return FALSE;
}

/* Unified spell uselessness check for all casting scenarios.
 * caster: &youmonst if player is casting, else the monster casting
 * target: &youmonst if player is target, else the monster being targeted
 */
STATIC_OVL
boolean
do_spell_would_be_useless(caster, target, adtyp, spellnum)
struct monst *caster, *target;
unsigned int adtyp;
int spellnum;
{
    boolean yours = (caster == &youmonst);
    boolean youdefend = (target == &youmonst);
    boolean evilpriest;
    boolean mcouldseeu = FALSE;
    int dist;

    /* Calculate distance between caster and target */
    if (yours) {
        dist = distmin(u.ux, u.uy, target->mx, target->my);
    } else if (youdefend) {
        dist = distu(caster->mx, caster->my);
        /* Check if caster could see where player is */
        mcouldseeu = couldsee(caster->mx, caster->my);
    } else {
        dist = distmin(caster->mx, caster->my, target->mx, target->my);
    }

    /* Check if caster is evil priest (for undead summoning) */
    if (yours) {
        evilpriest = FALSE; /* player doesn't use this check */
    } else {
        evilpriest = (caster->ispriest && mon_aligntyp(caster) < A_NEUTRAL);
    }

    if (adtyp == AD_SPEL) {
        /* Player-as-caster special restrictions */
        if (yours) {
            /* player != Rodney */
            if (spellnum == MGC_CLONE_WIZ)
                return TRUE;
            /* if you're attacking, monster is already aggravated */
            if (spellnum == MGC_AGGRAVATION)
                return TRUE;
            /* no free pets */
            if (spellnum == MGC_SUMMON_MONS)
                return TRUE;
            /* haste self when already fast (player uses Fast) */
            if (Fast && spellnum == MGC_HASTE_SELF)
                return TRUE;
            /* invisibility when already invisible (player uses HInvis) */
            if ((HInvis & INTRINSIC) && spellnum == MGC_DISAPPEAR)
                return TRUE;
            /* healing when already healed (player uses u.mh) */
            if (u.mh == u.mhmax && spellnum == MGC_CURE_SELF)
                return TRUE;
        } else {
            /* Monster-as-caster checks */
            /* don't cast these spells at range */
            if (dist > 1
                && (spellnum == MGC_PSI_BOLT
                    || spellnum == MGC_STUN_YOU
                    || spellnum == MGC_WEAKEN_YOU
                    || spellnum == MGC_CURSE_ITEMS
                    || spellnum == MGC_AGGRAVATION
                    || spellnum == MGC_SUMMON_MONS
                    || spellnum == MGC_CLONE_WIZ
                    || spellnum == MGC_DEATH_TOUCH))
                return TRUE;
            /* aggravate monsters, monster summoning won't
               be cast by tame or peaceful monsters */
            if ((caster->mtame || caster->mpeaceful)
                && (spellnum == MGC_AGGRAVATION
                    || spellnum == MGC_SUMMON_MONS
                    || spellnum == MGC_CLONE_WIZ))
                return TRUE;
            /* haste self when already fast */
            if (caster->permspeed == MFAST && spellnum == MGC_HASTE_SELF)
                return TRUE;
            /* invisibility when already invisible */
            if ((caster->minvis || caster->invis_blkd)
                && spellnum == MGC_DISAPPEAR)
                return TRUE;
            /* reflection when already reflecting */
            if ((has_reflection(caster) || mon_reflects(caster, (char *) 0))
                && spellnum == MGC_REFLECTION)
                return TRUE;
            /* peaceful monster won't cast invisibility if you can't see
               invisible (only matters when player is target or nearby) */
            if ((caster->mtame || caster->mpeaceful)
                && !See_invisible && spellnum == MGC_DISAPPEAR)
                return TRUE;
            /* healing when already healed */
            if (caster->mhp == caster->mhpmax && spellnum == MGC_CURE_SELF)
                return TRUE;
            /* don't summon monsters if it doesn't think target is around */
            if (youdefend) {
                if (!mcouldseeu && (spellnum == MGC_SUMMON_MONS
                                    || (!caster->iswiz && spellnum == MGC_CLONE_WIZ)))
                    return TRUE;
            }
            if ((!caster->iswiz || context.no_of_wizards > 1)
                && spellnum == MGC_CLONE_WIZ)
                return TRUE;
            /* aggravation (global wakeup) when everyone is already active
               - only check when attacking player */
            if (youdefend && spellnum == MGC_AGGRAVATION) {
                if (!has_aggravatables(caster))
                    return rn2(100) ? TRUE : FALSE;
            }
        }

        /* Target-based checks (armor destruction) - only for monster casters */
        if (!yours) {
            if (youdefend) {
                if (!wearing_armor() && spellnum == MGC_DESTRY_ARMR)
                    return TRUE;
            } else if (target) {
                if (!(target->misc_worn_check & W_ARMOR)
                    && spellnum == MGC_DESTRY_ARMR)
                    return TRUE;
            }
        }

        /* Explosion self-damage avoidance - only for monster casters */
        if (!yours) {
            int cdist;
            if (youdefend) {
                cdist = distu(caster->mx, caster->my);
            } else {
                cdist = distmin(caster->mx, caster->my,
                                target->mx, target->my);
            }

            if (youdefend) {
                /* Don't waste time zapping resisted spells at the player */
                if ((m_seenres(caster, M_SEEN_FIRE)
                     || (cdist < 3 && rn2(5)))
                    && spellnum == MGC_FIRE_BOLT)
                    return TRUE;
                if ((m_seenres(caster, M_SEEN_COLD)
                     || (cdist < 3 && rn2(5)))
                    && spellnum == MGC_ICE_BOLT)
                    return TRUE;
                if ((m_seenres(caster, M_SEEN_ACID)
                     || (cdist < 3 && rn2(5)))
                    && spellnum == MGC_ACID_BLAST)
                    return TRUE;
            } else {
                /* Don't blast itself with its own explosions
                   if it doesn't resist the attack type (most times) */
                if (!(resists_fire(caster) || defended(caster, AD_FIRE))
                    && spellnum == MGC_FIRE_BOLT
                    && cdist < 3 && rn2(5))
                    return TRUE;
                if (!(resists_cold(caster) || defended(caster, AD_COLD))
                    && spellnum == MGC_ICE_BOLT
                    && cdist < 3 && rn2(5))
                    return TRUE;
                if (!(resists_acid(caster) || defended(caster, AD_ACID))
                    && spellnum == MGC_ACID_BLAST
                    && cdist < 3 && rn2(5))
                    return TRUE;
            }

            /* Prevent pet/peaceful from nuking the player if close to target */
            if (!youdefend && (caster->mtame || caster->mpeaceful)
                && distu(target->mx, target->my) < 3
                && (spellnum == MGC_ICE_BOLT
                    || spellnum == MGC_FIRE_BOLT
                    || spellnum == MGC_ACID_BLAST
                    || spellnum == MGC_CANCELLATION))
                return TRUE;

            /* Don't cast offensive spells if can't see target or peaceful/invulnerable */
            if (youdefend
                && (!mcouldseeu || caster->mpeaceful || u.uinvulnerable)
                && (spellnum == MGC_ICE_BOLT
                    || spellnum == MGC_FIRE_BOLT
                    || spellnum == MGC_ACID_BLAST
                    || spellnum == MGC_CANCELLATION))
                return TRUE;
        }

        /* Don't cancel target if already cancelled - mon vs mon only */
        if (!yours && !youdefend && target && target->mcan
            && spellnum == MGC_CANCELLATION)
            return TRUE;

    } else if (adtyp == AD_CLRC) {
        /* Player-as-caster special restrictions */
        if (yours) {
            /* healing when already healed (player uses u.mh) */
            if (u.mh == u.mhmax && spellnum == CLC_CURE_SELF)
                return TRUE;
            /* no free pets */
            if (spellnum == CLC_CALL_UNDEAD
                || spellnum == CLC_INSECTS
                || spellnum == CLC_SUMMON_MINION)
                return TRUE;
            /* don't cast protection if already have barkskin or stoneskin
               (player uses Barkskin/Stoneskin macros) */
            if ((Barkskin || Stoneskin) && spellnum == CLC_PROTECTION)
                return TRUE;
            /* don't cast barkskin or stoneskin if already have protection */
            if (u.uspellprot
                && (spellnum == CLC_BARKSKIN || spellnum == CLC_STONESKIN))
                return TRUE;
            /* don't cast barkskin if already have stoneskin and vice versa */
            if (Barkskin && spellnum == CLC_STONESKIN)
                return TRUE;
            if (Stoneskin && spellnum == CLC_BARKSKIN)
                return TRUE;
        } else {
            /* Monster-as-caster checks */
            /* don't cast these spells at range */
            if (!youdefend && dist > 1
                && (spellnum == CLC_CONFUSE_YOU
                    || spellnum == CLC_PARALYZE
                    || spellnum == CLC_BLIND_YOU
                    || spellnum == CLC_CURSE_ITEMS
                    || spellnum == CLC_LIGHTNING
                    || spellnum == CLC_FIRE_PILLAR
                    || spellnum == CLC_GEYSER
                    || spellnum == CLC_SUMMON_MINION
                    || spellnum == CLC_CALL_UNDEAD))
                return TRUE;
            /* healing when already healed */
            if (caster->mhp == caster->mhpmax && spellnum == CLC_CURE_SELF)
                return TRUE;
            /* monster summoning won't be cast by tame or peaceful monsters */
            if ((caster->mtame || caster->mpeaceful)
                && (spellnum == CLC_CALL_UNDEAD
                    || spellnum == CLC_INSECTS
                    || spellnum == CLC_SUMMON_MINION))
                return TRUE;
            /* only undead/demonic spell casters, chaotic/unaligned priests
               and quest nemesis can summon undead */
            if (spellnum == CLC_CALL_UNDEAD && !is_undead(caster->data)
                && !is_demon(caster->data) && !evilpriest
                && caster->data->msound != MS_NEMESIS)
                return TRUE;
            /* don't cast protection if already have barkskin or stoneskin */
            if ((has_barkskin(caster) || has_stoneskin(caster))
                && spellnum == CLC_PROTECTION)
                return TRUE;
            /* don't cast barkskin or stoneskin if already have protection */
            if (caster->mprotection
                && (spellnum == CLC_BARKSKIN || spellnum == CLC_STONESKIN))
                return TRUE;
            /* don't cast barkskin if already have stoneskin and vice versa */
            if (has_barkskin(caster) && spellnum == CLC_STONESKIN)
                return TRUE;
            if (has_stoneskin(caster) && spellnum == CLC_BARKSKIN)
                return TRUE;
            /* don't cast certain spells if can't see target or peaceful/invuln */
            if (youdefend
                && (!mcouldseeu || caster->mpeaceful || u.uinvulnerable)
                && (spellnum == CLC_INSECTS
                    || spellnum == CLC_SUMMON_MINION
                    || spellnum == CLC_CALL_UNDEAD
                    || spellnum == CLC_OPEN_WOUNDS
                    || spellnum == CLC_VULN_YOU))
                return TRUE;
        }

        /* Target-based blindness check - only for monster casters */
        if (!yours) {
            if (youdefend) {
                if (Blinded && spellnum == CLC_BLIND_YOU)
                    return TRUE;
            } else if (target) {
                if ((!haseyes(target->data) || target->mblinded)
                    && spellnum == CLC_BLIND_YOU)
                    return TRUE;
            }
        }
    }
    return FALSE;
}

STATIC_OVL
boolean
uspell_would_be_useless(adtyp, spellnum) /* player casting as mon */
unsigned int adtyp;
int spellnum;
{
    if (adtyp == AD_SPEL) {
        /* player != rodney */
        if (spellnum == MGC_CLONE_WIZ)
            return TRUE;
        /* if you're attacking, monster is
           already aggravated */
        if (spellnum == MGC_AGGRAVATION)
            return TRUE;
        /* no free pets */
        if (spellnum == MGC_SUMMON_MONS)
            return TRUE;
        /* haste self when already fast */
        if (Fast && spellnum == MGC_HASTE_SELF)
            return TRUE;
        /* invisibility when already invisible */
        if ((HInvis & INTRINSIC) && spellnum == MGC_DISAPPEAR)
            return TRUE;
        /* healing when already healed */
        if (u.mh == u.mhmax && spellnum == MGC_CURE_SELF)
            return TRUE;
    } else if (adtyp == AD_CLRC) {
        /* healing when already healed */
        if (u.mh == u.mhmax && spellnum == CLC_CURE_SELF)
            return TRUE;
        /* no free pets */
        if (spellnum == CLC_CALL_UNDEAD
            || spellnum == CLC_INSECTS
            || spellnum == CLC_SUMMON_MINION)
            return TRUE;
        /* don't cast protection if already have barkskin
           or stoneskin */
        if ((Barkskin || Stoneskin)
            && spellnum == CLC_PROTECTION)
            return TRUE;
        /* don't cast barkskin or stoneskin if already have
           protection */
        if (u.uspellprot
            && (spellnum == CLC_BARKSKIN
                || spellnum == CLC_STONESKIN))
            return TRUE;
        /* don't cast barkskin if already have stoneskin
           and vice versa */
        if (Barkskin && spellnum == CLC_STONESKIN)
            return TRUE;
        if (Stoneskin && spellnum == CLC_BARKSKIN)
            return TRUE;
    }
    return FALSE;
}

/* convert 1..11 to 0..10 */
#define ad_to_typ(k) ((int) k - 1)
/* convert AD_FOO to ZT_FOO as a spell (i.e. add MAX_ZT) */
#define ad_to_spelltyp(k) ZT_SPELL(ad_to_typ(k))

/* monster uses spell against player (ranged) */
int
buzzmu(mtmp, mattk)
struct monst *mtmp;
struct attack *mattk;
{
    boolean seecaster = (canseemon(mtmp) || tp_sensemon(mtmp) || Detect_monsters);

    /* don't print constant stream of curse messages for 'normal'
       spellcasting monsters at range */
    if (mattk->adtyp > AD_SPC2)
        return 0;

    if (mtmp->mcan) {
        cursetxt(mtmp, FALSE);
        return 0;
    }
    if (lined_up(mtmp) && rn2(3)) {
        nomul(0);
        if (mattk->adtyp && (mattk->adtyp <= MAX_ZT)) { /* no cf unsigned > 0 */
            if (seecaster)
                pline("%s zaps you with a %s!", Monnam(mtmp),
                      flash_types[ad_to_typ(mattk->adtyp)]);
            buzz(-ad_to_spelltyp(mattk->adtyp), (int) mattk->damn, mtmp->mx,
                 mtmp->my, sgn(tbx), sgn(tby));
        } else
            impossible("Monster spell %d cast", mattk->adtyp - 1);
    }
    return 1;
}

/* monster uses spell against monster (ranged) */
int
buzzmm(mtmp, mdef, mattk)
struct monst *mtmp;
struct monst *mdef;
struct attack *mattk;
{
    boolean seecaster = (canseemon(mtmp) || tp_sensemon(mtmp) || Detect_monsters);

    /* don't print constant stream of curse messages for 'normal'
       spellcasting monsters at range */
    if (mattk->adtyp > AD_SPC2)
        return 0;

    if (mtmp->mcan) {
        cursetxt(mtmp, FALSE);
        return 0;
    }
    if (mlined_up(mtmp, mdef, FALSE) && rn2(3)) {
        nomul(0);
        if (mattk->adtyp && (mattk->adtyp <= MAX_ZT)) { /* no cf unsigned > 0 */
            if (seecaster)
                pline("%s zaps %s with a %s!", Monnam(mtmp),
                      mon_nam(mdef), flash_types[ad_to_typ(mattk->adtyp)]);
            dobuzz(-ad_to_spelltyp(mattk->adtyp), (int) mattk->damn, mtmp->mx,
                   mtmp->my, sgn(tbx), sgn(tby), FALSE);
        } else
            impossible("Monster spell %d cast", mattk->adtyp - 1);
    }
    return 1;
}

int
castmm(mtmp, mdef, mattk)
struct monst *mtmp;
struct monst *mdef;
struct attack *mattk;
{
    int dmg, ml = min(mtmp->m_lev, 50);
    int ret;
    int spellnum = 0;

    boolean seecaster = (canseemon(mtmp) || tp_sensemon(mtmp) || Detect_monsters);

    /* guard against casting another spell attack
       at an already dead monster; some monsters
       have multiple AT_MAGC attacks */
    if (mdef->mhp <= 0)
        return 0;

    if ((mattk->adtyp == AD_SPEL || mattk->adtyp == AD_CLRC) && ml) {
        int cnt = 40;

        do {
            spellnum = rn2(ml);
            if (mattk->adtyp == AD_SPEL)
                spellnum = choose_magic_spell(mtmp, spellnum);
            else
                spellnum = choose_clerical_spell(mtmp, spellnum);
        /* not trying to attack?  don't allow directed spells */
        } while (--cnt > 0
                 && do_spell_would_be_useless(mtmp, mdef,
                                              mattk->adtyp, spellnum));
        if (cnt == 0)
            return 0;
    }

    /* monster unable to cast spells? */
    if (mtmp->mcan || mtmp->mspec_used || !ml) {
        if (canseemon(mtmp)) {
            if (is_undirected_spell(mattk->adtyp, spellnum))
                pline("%s points all around, then curses.",
                      Monnam(mtmp));
            else
                pline("%s points at %s, then curses.",
                      Monnam(mtmp), mon_nam(mdef));
        } else if ((!(moves % 4) || !rn2(4))) {
            if (!Deaf)
                Norep("You hear a mumbled curse.");
        }
        return 0;
    }

    if (mattk->adtyp == AD_SPEL || mattk->adtyp == AD_CLRC) {
        mtmp->mspec_used = 4 - mtmp->m_lev;
        if (mtmp->mspec_used < 2)
            mtmp->mspec_used = 2;
        /* many boss-type monsters than have two or more spell attacks
           per turn are never able to fire off their second attack due
           to mspec always being greater than 0. So set to 0 for those
           types of monsters, either sometimes or all of the time
           depending on how powerful they are or what their role is */
        /* Top tier bosses - mspec 0 always */
        if (is_dprince(mtmp->data) || mtmp->iswiz || mtmp->isvecna
            || mtmp->istalgath || mtmp->data == &mons[PM_KATHRYN_THE_ENCHANTRESS])
            mtmp->mspec_used = 0;
        /* Mid tier bosses - mspec 0 two thirds of the time */
        else if (rn2(3)
                 && (is_dlord(mtmp->data)
                     || mtmp->data->msound == MS_LEADER
                     || mtmp->data->msound == MS_NEMESIS
                     || mtmp->data == &mons[PM_ORACLE]
                     || mtmp->data == &mons[PM_HIGH_PRIEST]
                     || mtmp->data == &mons[PM_KATHRYN_THE_ICE_QUEEN]))
            mtmp->mspec_used = 0;

        /* Having the EotA in inventory drops mspec to 0 */
        if (has_eota(mtmp))
            mtmp->mspec_used = 0;
    }

    if (rn2(ml * 10) < (mtmp->mconf ? 100 : 20)) {	/* fumbled attack */
        if (canseemon(mtmp) && !Deaf)
            pline_The("air crackles around %s.", mon_nam(mtmp));
        return 0;
    }

    if (seecaster && canseemon(mdef)
        && !is_undirected_spell(mattk->adtyp, spellnum))
        pline("%s casts a spell at %s!", Monnam(mtmp), mon_nam(mdef));

    if (seecaster
        && is_undirected_spell(mattk->adtyp, spellnum))
        pline("%s casts a spell!", Monnam(mtmp));

    if (mattk->damd)
        dmg = d((int) ((ml / 2) + mattk->damn), (int) mattk->damd);
    else
        dmg = d((int) ((ml / 2) + 1), 6);

    ret = 1;

    switch (mattk->adtyp) {
    case AD_FIRE:
        if (canseemon(mdef)) {
            if (is_demon(mtmp->data))
                pline("%s is enveloped in hellfire!", Monnam(mdef));
            else if (!mon_underwater(mdef))
                pline("%s is enveloped in flames.", Monnam(mdef));
            else {
                pline("The flames are quenched by the water around %s.",
                      mon_nam(mdef));
                dmg = 0;
                break;
            }
        }

        if (resists_fire(mdef) || defended(mdef, AD_FIRE)) {
            shieldeff(mdef->mx, mdef->my);
            if (is_demon(mtmp->data)) {
                if (!(nonliving(mdef->data) || is_demon(mdef->data)
                      || racial_zombie(mdef) || racial_vampire(mdef))) {
                    if (canseemon(mdef))
                        pline_The("hellish flames sear %s soul!",
                                  s_suffix(mon_nam(mdef)));
                    dmg = (dmg + 1) / 2;
                } else {
                    dmg = 0;
                }
            } else {
                if (canseemon(mdef))
                    pline("But %s resists the effects.", mhe(mdef));
                dmg = 0;
            }
        }
        break;
    case AD_COLD:
        if (canseemon(mdef))
            pline("%s is covered in frost.", Monnam(mdef));
        if (resists_cold(mdef) || defended(mdef, AD_COLD)) {
            shieldeff(mdef->mx, mdef->my);
            if (canseemon(mdef))
                pline("But %s resists the effects.", mhe(mdef));
            dmg = 0;
        }
        break;
    case AD_ACID:
        if (canseemon(mdef)) {
            if (!mon_underwater(mdef))
                pline("%s is covered in acid.", Monnam(mdef));
            else {
                pline("The acid dissipates harmlessly in the water around %s.",
                      mon_nam(mdef));
                dmg = 0;
                break;
            }
        }
        if (resists_acid(mdef) || defended(mdef, AD_ACID)) {
            shieldeff(mdef->mx, mdef->my);
            if (canseemon(mdef))
                pline("But %s resists the effects.", mhe(mdef));
            dmg = 0;
        }
        break;
    case AD_MAGM:
        if (canseemon(mdef))
            pline("%s is hit by a shower of missiles!", Monnam(mdef));
        dmg = d((int) ml / 2 + 1, 6);
        if (resists_magm(mdef) || defended(mdef, AD_MAGM)) {
            shieldeff(mdef->mx, mdef->my);
            if (canseemon(mdef))
                pline("Some missiles bounce off!");
            dmg = (dmg + 1) / 2;
        }
        break;
    case AD_SPEL:   /* wizard spell */
    case AD_CLRC: { /* clerical spell */
        /* aggravation is a special case;
         * it's undirected but should still target the
         * victim so as to aggravate you */
        if (is_undirected_spell(mattk->adtyp, spellnum)
            /* 'undirected-but-not-really' spells: */
            && (mattk->adtyp == AD_SPEL
                /* magic spells */
                ? (!(spellnum == MGC_AGGRAVATION
                     || spellnum == MGC_SUMMON_MONS
                     || spellnum == MGC_ACID_BLAST
                     || spellnum == MGC_FIRE_BOLT
                     || spellnum == MGC_ICE_BOLT
                     || spellnum == MGC_CANCELLATION))
                /* clerical spells */
                : (!(spellnum == CLC_INSECTS
                     || spellnum == CLC_SUMMON_MINION
                     || spellnum == CLC_CALL_UNDEAD
                     || spellnum == CLC_VULN_YOU
                     || spellnum == CLC_OPEN_WOUNDS)))) {
            /* monster vs you */
            if (mattk->adtyp == AD_SPEL)
                do_wizard_spell(mtmp, &youmonst, dmg, spellnum);
            else
                do_cleric_spell(mtmp, &youmonst, dmg, spellnum);
        /* you vs monster or monster vs monster */
        } else if (mattk->adtyp == AD_SPEL) {
            do_wizard_spell(mtmp, mdef, dmg, spellnum);
        } else {
            do_cleric_spell(mtmp, mdef, dmg, spellnum);
        }
        dmg = 0; /* done by the spell casting functions */
        break;
        }
    }

    if (dmg) {
        mdef->mhp -= dmg;
        if (DEADMONSTER(mdef))
            monkilled(mdef, "", mattk->adtyp);
    }

    return (ret);
}

int
castum(mtmp, mattk)
struct monst *mtmp;
struct attack *mattk;
{
    int dmg, ml = mons[u.umonnum].mlevel;
    int ret;
    int spellnum = 0;
    boolean directed = FALSE;

    /* guard against casting another spell attack
       at an already dead monster; some monsters
       have multiple AT_MAGC attacks */
    if (mtmp->mhp <= 0)
        return 0;

    if ((mattk->adtyp == AD_SPEL || mattk->adtyp == AD_CLRC) && ml) {
        int cnt = 40;

        do {
            spellnum = rn2(ml);
            if (mattk->adtyp == AD_SPEL)
                spellnum = choose_magic_spell(mtmp, spellnum);
            else
                spellnum = choose_clerical_spell(mtmp, spellnum);
            /* not trying to attack?  don't allow directed spells */
            if (!mtmp || mtmp->mhp < 1) {
                if (is_undirected_spell(mattk->adtyp, spellnum)
                    && !uspell_would_be_useless(mattk->adtyp, spellnum)) {
                break;
            }
        }

    } while (--cnt > 0
             && ((!mtmp && !is_undirected_spell(mattk->adtyp, spellnum))
                 || uspell_would_be_useless(mattk->adtyp, spellnum)));
        if (cnt == 0) {
            You("have no spells to cast right now!");
            return 0;
        }
    }

    if (spellnum == MGC_AGGRAVATION && !mtmp) {
        /* choose a random monster on the level */
        int j = 0, k = 0;

        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
            if (!mtmp->mtame && !mtmp->mpeaceful)
                j++;
        }

        if (j > 0) {
            k = rn2(j);
            for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
                if (!mtmp->mtame && !mtmp->mpeaceful) {
                    if (--k < 0)
                        break;
                }
            }
        }
    }

    directed = mtmp && !is_undirected_spell(mattk->adtyp, spellnum);

    /* unable to cast spells? */
    if (u.uen < ml) {
        if (directed)
            You("point at %s, then curse.", mon_nam(mtmp));
        else
            You("point all around, then curse.");
        return 0;
    }

    if (mattk->adtyp == AD_SPEL || mattk->adtyp == AD_CLRC) {
        u.uen -= ml;
    }

    if (rn2(ml * 10) < (Confusion ? 100 : 20)) { /* fumbled attack */
        pline_The("air crackles around you.");
        return 0;
    }

    You("cast a spell%s%s!", directed ? " at " : "",
        directed ? mon_nam(mtmp) : "");

    /* As these are spells, the damage is related to the level
     * of the monster casting the spell. */
    if (mattk->damd)
        dmg = d((int) ((ml / 2) + mattk->damn), (int) mattk->damd);
    else
        dmg = d((int) ((ml / 2) + 1), 6);

    ret = 1;

    switch (mattk->adtyp) {
    case AD_FIRE:
        if (canseemon(mtmp)) {
            if (is_demon(youmonst.data))
                pline("%s is enveloped in hellfire!", Monnam(mtmp));
            else if (!mon_underwater(mtmp))
                pline("%s is enveloped in flames.", Monnam(mtmp));
            else {
                pline("The flames are quenched by the water around %s.",
                      mon_nam(mtmp));
                dmg = 0;
                break;
            }
        }

        if (resists_fire(mtmp) || defended(mtmp, AD_FIRE)) {
            shieldeff(mtmp->mx, mtmp->my);
            if (is_demon(youmonst.data)) {
                if (!(nonliving(mtmp->data) || is_demon(mtmp->data)
                      || racial_zombie(mtmp) || racial_vampire(mtmp))) {
                    if (canseemon(mtmp))
                        pline_The("hellish flames sear %s soul!",
                                  s_suffix(mon_nam(mtmp)));
                    dmg = (dmg + 1) / 2;
                } else {
                    dmg = 0;
                }
            } else {
                if (canseemon(mtmp))
                    pline("But %s resists the effects.", mhe(mtmp));
                dmg = 0;
            }
        }
        break;
    case AD_COLD:
        if (canseemon(mtmp))
            pline("%s is covered in frost.", Monnam(mtmp));
        if (resists_cold(mtmp) || defended(mtmp, AD_COLD)) {
            shieldeff(mtmp->mx, mtmp->my);
            if (canseemon(mtmp))
                pline("But %s resists the effects.", mhe(mtmp));
            dmg = 0;
        }
        break;
    case AD_ACID:
        if (canseemon(mtmp)) {
            if (!mon_underwater(mtmp))
                pline("%s is covered in acid.", Monnam(mtmp));
            else {
                pline("The acid dissipates harmlessly in the water around %s.",
                      mon_nam(mtmp));
                dmg = 0;
                break;
            }
        }
        if (resists_acid(mtmp) || defended(mtmp, AD_ACID)) {
            shieldeff(mtmp->mx, mtmp->my);
            if (canseemon(mtmp))
                pline("But %s resists the effects.", mhe(mtmp));
            dmg = 0;
        }
        break;
    case AD_MAGM:
        if (canseemon(mtmp))
            pline("%s is hit by a shower of missiles!", Monnam(mtmp));
        dmg = d((int)ml / 2 + 1, 6);
        if (resists_magm(mtmp) || defended(mtmp, AD_MAGM)) {
            shieldeff(mtmp->mx, mtmp->my);
            if (canseemon(mtmp))
                pline("Some missiles bounce off!");
            dmg = (dmg + 1) / 2;
        }
        break;
    case AD_SPEL:   /* wizard spell */
    case AD_CLRC: { /* clerical spell */
        if (mattk->adtyp == AD_SPEL)
            do_wizard_spell(&youmonst, mtmp, dmg, spellnum);
        else
            do_cleric_spell(&youmonst, mtmp, dmg, spellnum);
        dmg = 0; /* done by the spell casting functions */
        break;
        }
    }

    if (dmg) {
        mtmp->mhp -= dmg;
        if (DEADMONSTER(mtmp))
            killed(mtmp);
    }

    return (ret);
}

/* Check if monster knows a specific spell */
boolean
mknows_spell(mtmp, otyp)
struct monst *mtmp;
int otyp;  /* spell object type */
{
    int i;

    if (!has_emsp(mtmp))
        return FALSE;
    for (i = 0; i < MAXMONSPELL; i++) {
        if (EMSP(mtmp)->msp_id[i] == otyp && EMSP(mtmp)->msp_know[i] > 0)
            return TRUE;
    }
    return FALSE;
}

/* Monster learns spell from spellbook; returns TRUE if learned */
boolean
mlearn_spell(mtmp, book)
struct monst *mtmp;
struct obj *book;
{
    int i;

    if (!is_spellcaster(mtmp))
        return FALSE;
    if (mknows_spell(mtmp, book->otyp))
        return FALSE; /* Already knows it */
    if (book->spestudied >= MAX_SPELL_STUDY)
        return FALSE; /* Book depleted */

    /* Allocate emsp if needed */
    if (!has_emsp(mtmp))
        newemsp(mtmp);

    /* Find empty slot */
    for (i = 0; i < MAXMONSPELL; i++) {
        if (EMSP(mtmp)->msp_id[i] == 0 || EMSP(mtmp)->msp_know[i] <= 0) {
            EMSP(mtmp)->msp_id[i] = book->otyp;
            EMSP(mtmp)->msp_know[i] = MSPELL_KEEN;
            book->spestudied++;
            return TRUE;
        }
    }
    return FALSE; /* No room */
}

/* Count how many learned spells the monster can cast at its current level */
STATIC_OVL int
mcount_castable_spells(mtmp)
struct monst *mtmp;
{
    int count = 0, i;
    short spell_otyp;
    int spell_level, required_mlev;

    if (!has_emsp(mtmp))
        return 0;

    for (i = 0; i < MAXMONSPELL; i++) {
        spell_otyp = EMSP(mtmp)->msp_id[i];
        if (spell_otyp != 0 && EMSP(mtmp)->msp_know[i] > 0) {
            /* Skip invalid entries */
            if (spell_otyp < SPE_FIREBALL || spell_otyp > SPE_BLANK_PAPER
                || objects[spell_otyp].oc_class != SPBOOK_CLASS)
                continue;
            /* Check level requirement */
            spell_level = objects[spell_otyp].oc_level;
            required_mlev = spell_level * 3;
            if (mtmp->m_lev >= required_mlev)
                count++;
        }
    }
    return count;
}

/* Get a random learned spell monster can cast; returns otyp or 0 */
short
mchoose_learned_spell(mtmp)
struct monst *mtmp;
{
    short candidates[MAXMONSPELL];
    int count = 0, i;
    short spell_otyp;
    int spell_level, required_mlev;

    if (!has_emsp(mtmp))
        return 0;

    for (i = 0; i < MAXMONSPELL; i++) {
        spell_otyp = EMSP(mtmp)->msp_id[i];
        if (spell_otyp != 0 && EMSP(mtmp)->msp_know[i] > 0) {
            /* Defensive bounds check for corrupted emsp data */
            if (spell_otyp < SPE_DIG || spell_otyp > SPE_BLANK_PAPER
                || objects[spell_otyp].oc_class != SPBOOK_CLASS) {
                impossible("mchoose_learned_spell: invalid otyp %d", spell_otyp);
                EMSP(mtmp)->msp_id[i] = 0;  /* Clear corrupted entry */
                continue;
            }
            /* Check if monster is high enough level to cast this spell.
               Spell levels 1-7 map to required monster levels 3-21 */
            spell_level = objects[spell_otyp].oc_level;
            required_mlev = spell_level * 3;
            if (mtmp->m_lev >= required_mlev)
                candidates[count++] = spell_otyp;
        }
    }

    if (count == 0)
        return 0;
    return candidates[rn2(count)];
}

/* Decay monster spell knowledge - call from moveloop */
void
mage_spells(mtmp)
struct monst *mtmp;
{
    int i;
    struct emsp *esp;

    if (!has_emsp(mtmp))
        return;

    esp = EMSP(mtmp);
    for (i = 0; i < MAXMONSPELL; i++) {
        if (esp->msp_id[i] != 0 && esp->msp_know[i] > 0) {
            esp->msp_know[i]--;
            if (esp->msp_know[i] <= 0)
                esp->msp_id[i] = 0; /* Forgot spell */
        }
    }
}

/*mcastu.c*/
