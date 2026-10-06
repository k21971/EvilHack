/* NetHack 5.0	mcastu.c	$NHDT-Date: 1781973053 2026/06/20 16:30:53 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.122 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2011. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#define MCASTU_ENUM
enum mcast_spells {
    #include "mcastu.h"
};
#undef MCASTU_ENUM

struct _mcast_data {
    int level;
    int flags;
};

#define MCASTU_INIT
static struct _mcast_data mcast_data[] = {
    #include "mcastu.h"
};
#undef MCASTU_INIT

/* spell lists for specific monster casters */
/* the spells in the list should be in ascending level order */
static int mon_cleric_spells[] = {
    MCAST_OPEN_WOUNDS, MCAST_CURE_SELF, MCAST_PROTECTION, MCAST_CONFUSE_YOU,
    MCAST_PARALYZE, MCAST_VULN_YOU, MCAST_BLIND_YOU, MCAST_INSECTS,
    MCAST_CALL_UNDEAD, MCAST_CURSE_ITEMS, MCAST_LIGHTNING,
    MCAST_FIRE_PILLAR, MCAST_GEYSER, MCAST_SUMMON_MINION
};
static int mon_wizard_spells[] = {
    MCAST_PSI_BOLT, MCAST_CURE_SELF, MCAST_HASTE_SELF, MCAST_STUN_YOU,
    MCAST_DISAPPEAR, MCAST_WEAKEN_YOU, MCAST_DESTRY_ARMR, MCAST_CURSE_ITEMS,
    MCAST_AGGRAVATION, MCAST_ACID_BLAST, MCAST_SUMMON_MONS, MCAST_CLONE_WIZ,
    MCAST_DEATH_TOUCH, MCAST_FIRE_BOLT
};

/* self-buff learned spells that can be cast through the undirected path */
#define IS_SELFBUFF_SPELL(otyp) \
    ((otyp) == SPE_CURE_BLINDNESS || (otyp) == SPE_CURE_SICKNESS \
     || (otyp) == SPE_REMOVE_CURSE || (otyp) == SPE_REPAIR_ARMOR \
     || (otyp) == SPE_LEVITATION || (otyp) == SPE_JUMPING \
     || (otyp) == SPE_BURNING_HANDS || (otyp) == SPE_SHOCKING_GRASP)

staticfn void cursetxt(struct monst *, boolean);
staticfn boolean mon_has_eregen_artifact(struct monst *);
staticfn boolean is_boss_caster(struct monst *);
staticfn int druid_ward_subst(struct monst *, int);
staticfn int choose_bolt_spell(struct monst *);
staticfn int choose_monster_spell(struct monst *, struct monst *, int);
staticfn struct monst *find_adjacent_pet(struct monst *);
staticfn void do_untame_pet(struct monst *, struct monst *);
staticfn short mpick_learned_spell(struct monst *, struct monst *);
staticfn int cast_learned_spell(struct monst *, struct monst *, short);
staticfn boolean target_resists_learned_spell(struct monst *,
                                              struct monst *, short);
staticfn boolean mhas_castable_offensive_spell(struct monst *,
                                               struct monst *);
staticfn boolean mhas_useful_selfbuff_spell(struct monst *);
staticfn int mcount_castable_spells(struct monst *);
staticfn int m_cure_self(struct monst *, int);
staticfn void mcast_protection(struct monst *);
staticfn void mcast_cancellation(struct monst *, struct monst *);
staticfn void mcast_call_undead(struct monst *, struct monst *);
staticfn void mcast_death_touch(struct monst *, struct monst *);
staticfn void mcast_clone_wiz(struct monst *);
staticfn void mcast_summon_mons(struct monst *, struct monst *);
staticfn void mrndcurse(struct monst *);
staticfn void mcast_destroy_armor(struct monst *, struct monst *);
staticfn void mcast_weaken_you(struct monst *, struct monst *, int);
staticfn void mcast_disappear(struct monst *);
staticfn void mcast_stun_you(struct monst *, struct monst *, int);
staticfn int mcast_geyser(struct monst *, struct monst *, int);
staticfn int mcast_fire_pillar(struct monst *, struct monst *, int);
staticfn int mcast_lightning(struct monst *, struct monst *, int);
staticfn void mcast_acid_blast(struct monst *, struct monst *);
staticfn void mcast_bolt(struct monst *, struct monst *, int);
staticfn void mcast_vuln_you(struct monst *, struct monst *);
staticfn void mcast_summon_minion(struct monst *, struct monst *);
staticfn int mcast_psi_bolt(struct monst *, struct monst *, int);
staticfn int mcast_open_wounds(struct monst *, struct monst *, int);
staticfn void mcast_insects(struct monst *, struct monst *);
staticfn void mcast_blind_you(struct monst *, struct monst *);
staticfn int mcast_paralyze(struct monst *, struct monst *);
staticfn void mcast_confuse_you(struct monst *, struct monst *);
staticfn void mcast_spell(struct monst *, struct monst *, int, int, short,
                          int);
staticfn boolean is_undirected_spell(struct monst *, int);
staticfn boolean spell_would_be_useless(struct monst *, struct monst *,
                                        int);

/* True if mtmp has any artifact conferring SPFX_EREGEN (energy
   regeneration); such an artifact allows unlimited spellcasting
   (mspec_used = 0) */
staticfn boolean
mon_has_eregen_artifact(struct monst *mtmp)
{
    return mon_arti_has_spfx(mtmp, SPFX_EREGEN);
}

/* check if monster is a boss-type spellcaster that gets mspec
   bonuses; used to allow bosses to cast more frequently */
staticfn boolean
is_boss_caster(struct monst *mtmp)
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

/* feedback when frustrated monster couldn't cast a spell */
staticfn void
cursetxt(struct monst *mtmp, boolean undirected)
{
    if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my)) {
        const char *point_msg; /* spellcasting monsters are impolite */

        if (undirected)
            point_msg = "all around, then curses";
        else if ((Invis && !mon_prop(mtmp, SEE_INVIS)
                  && (mtmp->mux != u.ux || mtmp->muy != u.uy))
                 || is_obj_mappear(&gy.youmonst, STRANGE_OBJECT)
                 || u.uundetected)
            point_msg = "and curses in your general direction";
        else if (Displaced && (mtmp->mux != u.ux || mtmp->muy != u.uy))
            point_msg = "and curses at your displaced image";
        else
            point_msg = "at you, then curses";

        pline_mon(mtmp, "%s points %s.", Monnam(mtmp), point_msg);
    } else if ((!(svm.moves % 4) || !rn2(4))) {
        if (!Deaf)
            Norep("You hear a mumbled curse.");   /* Deaf-aware */
    }
}

/* choose a spell for monster to cast */
staticfn int
druid_ward_subst(struct monst *mtmp, int spellnum)
{
    /* Elanee and her circle ward themselves with the druidic skin
       spells in place of the generic clerical picks of the same
       level; once either ward is up the generic pick stands (the
       wards are mutually exclusive) */
    if (mtmp->data != &mons[PM_ELANEE] && mtmp->data != &mons[PM_DRUID]
        && mtmp->data != &mons[PM_ASPIRANT])
        return spellnum;
    if (has_barkskin(mtmp) || has_stoneskin(mtmp))
        return spellnum;
    if (spellnum == MCAST_BLIND_YOU)
        return MCAST_STONESKIN;
    if (spellnum == MCAST_PROTECTION)
        return MCAST_BARKSKIN;
    return spellnum;
}

/* roll the element behind the wizard ladder's bolt rung; returns
   MCAST_FIRE_BOLT, MCAST_ICE_BOLT, MCAST_PSI_BOLT, or -1 to make the
   caller fall through to another pick. The Ice Queen favors ice; psi
   bolt is the better choice against a magic-resistant or
   hallucinating hero */
staticfn int
choose_bolt_spell(struct monst *mtmp)
{
    switch (rn2(3)) {
    case 2:
        return (mtmp->data == &mons[PM_KATHRYN_THE_ICE_QUEEN])
                   ? MCAST_ICE_BOLT : MCAST_FIRE_BOLT;
    case 1:
        return MCAST_ICE_BOLT;
    default:
        return (Antimagic || Hallucination) ? MCAST_PSI_BOLT : -1;
    }
}

staticfn int
choose_monster_spell(struct monst *mtmp, struct monst *mdef, int adtyp)
{
    int *list = NULL;
    int i, candidate, spellval, len = 0;
    int maxlev, spell_count;

    /* spells learned from spellbooks get first consideration; the
       chance scales with repertoire size: 15% base plus 5% per
       castable spell, capped at 40% */
    spell_count = mcount_castable_spells(mtmp);
    if (spell_count > 0) {
        int chance = 15 + spell_count * 5;

        if (chance > 40)
            chance = 40;
        if (rn2(100) < chance
            && mchoose_learned_spell(mtmp, (struct monst *) 0, 0, 0,
                                     FALSE) != 0)
            return MCAST_LEARNED_SPELL;
    }

    /* which spell list to use? */
    if (adtyp == AD_SPEL) {
        list = mon_wizard_spells;
        len = SIZE(mon_wizard_spells);
    } else if (adtyp == AD_CLRC) {
        list = mon_cleric_spells;
        len = SIZE(mon_cleric_spells);
    }

    if (!list || len < 1)
        return MCAST_PSI_BOLT;

    /* max spell level in this monster spell list */
    maxlev = mcast_data[list[len - 1]].level;

    /* which level spell to cast? The hero's polymorphed casting level
       comes from the form (gy.youmonst.m_lev is never maintained) */
    spellval = rn2((mtmp == &gy.youmonst) ? mons[u.umonnum].mlevel
                                          : (int) mtmp->m_lev);
    if (spellval > maxlev && rn2(maxlev))
        spellval = rn2(maxlev);

    /* find the highest spell in the list we could cast, or fall back
       to the first spell in the list; the druid substitution runs on
       the candidate BEFORE the useless test so the substituted
       spell's own flags decide (the wards are indirect and castable
       by the peaceful circle, unlike the hostile-only picks they
       replace) */
    candidate = -1;
    for (i = len - 1; i > 0; i--) {
        if (mcast_data[list[i]].level > spellval)
            continue;
        candidate = list[i];
        /* some rungs stand for a random pick between same-level
           spells; roll before the substitution and useless tests so
           the chosen spell's own flags and usefulness decide, and
           fall through to the next rung when the pick proves
           useless */
        if (candidate == MCAST_CONFUSE_YOU && rn2(2))
            candidate = MCAST_PROTECTION;
        else if (candidate == MCAST_AGGRAVATION && rn2(2))
            candidate = MCAST_REFLECTION;
        else if (candidate == MCAST_SUMMON_MONS && !rn2(3))
            candidate = MCAST_CANCELLATION;
        candidate = druid_ward_subst(mtmp, candidate);
        /* the bolt rung stands for a fire/ice/psi choice; roll it
           now so the chosen element's own resistances decide, and
           fall through to the next rung when the roll defers */
        if (candidate == MCAST_FIRE_BOLT) {
            candidate = choose_bolt_spell(mtmp);
            if (candidate < 0)
                continue;
        }
        if (!spell_would_be_useless(mtmp, mdef, candidate))
            break;
        candidate = -1;
    }
    if (candidate < 0) {
        candidate = druid_ward_subst(mtmp, list[0]);
        /* the wizard ladder's floor is itself a bolt chance */
        if (candidate == MCAST_PSI_BOLT) {
            int bolt = choose_bolt_spell(mtmp);

            if (bolt >= 0)
                candidate = bolt;
        }
    }

    /* demons never summon minions (they have summonmu for their own
       kind); tame and peaceful casters never summon reinforcements
       against the hero's side; all of them call down fire instead
       (which the cold-caster substitution below may still convert) */
    if (candidate == MCAST_SUMMON_MINION
        && (is_demon(mtmp->data) || mtmp->mtame || mtmp->mpeaceful))
        candidate = MCAST_FIRE_PILLAR;

    /* the Ice Queen and Asmodeus are creatures of cold; they conjure
       lightning where other clerical casters would call down fire */
    if (candidate == MCAST_FIRE_PILLAR
        && (mtmp->data == &mons[PM_KATHRYN_THE_ICE_QUEEN]
            || mtmp->data == &mons[PM_ASMODEUS]))
        return MCAST_LIGHTNING;

    /* demons (other than Lolth, who keeps her insects) and both
       Kathryns favor the vulnerability curse; substitute it for half
       of their insect picks */
    if (candidate == MCAST_INSECTS && rn2(2)
        && ((is_demon(mtmp->data) && mtmp->data != &mons[PM_LOLTH])
            || mtmp->data == &mons[PM_KATHRYN_THE_ICE_QUEEN]
            || mtmp->data == &mons[PM_KATHRYN_THE_ENCHANTRESS]))
        return MCAST_VULN_YOU;

    return candidate;
}

/* return values:
 * 1: successful spell
 * 0: unsuccessful spell
 */
int
castmu(
    struct monst *mtmp,   /* caster */
    struct attack *mattk, /* caster's current attack */
    boolean thinks_it_foundyou,    /* might be mistaken if displaced */
    boolean foundyou)              /* knows hero's precise location */
{
    int dmg, ml = min((int) mtmp->m_lev, 50);
    int ret;
    int spellnum = 0;
    short learned_otyp = 0; /* actual learned spell, resolved before msg */
    boolean undirected;

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
            spellnum = choose_monster_spell(mtmp, &gy.youmonst,
                                            mattk->adtyp);
            /* not trying to attack?  don't allow directed spells */
            if (!thinks_it_foundyou) {
                if (!is_undirected_spell(mtmp, spellnum)
                    || spell_would_be_useless(mtmp, &gy.youmonst,
                                              spellnum)) {
                    if (foundyou)
                        impossible(
                       "spellcasting monster found you and doesn't know it?");
                    return M_ATTK_MISS;
                }
                break;
            }
        } while (--cnt > 0
                 && spell_would_be_useless(mtmp, &gy.youmonst, spellnum));
        if (cnt == 0)
            return M_ATTK_MISS;
    }

    /* monster unable to cast spells? (no check of what the hero was
       seen to resist: resistances and reflection are often partial,
       so casters keep trying) */
    if (mtmp->mcan || mtmp->mspec_used || !ml) {
        cursetxt(mtmp, is_undirected_spell(mtmp, spellnum));
        return M_ATTK_MISS;
    }

    debugpline3("castmu:%s,lvl:%i,spell:%i", noit_Monnam(mtmp), ml, spellnum);

    if (mattk->adtyp == AD_SPEL || mattk->adtyp == AD_CLRC) {
        /* monst->m_lev is unsigned (uchar), monst->mspec_used is int */
        mtmp->mspec_used = 4 - (int) mtmp->m_lev;
        if (mtmp->mspec_used < 2)
            mtmp->mspec_used = 2;
        /* many boss-type monsters that have two or more spell attacks
           per turn are never able to fire off their second attack due
           to mspec always being greater than 0, so set it to 0 for
           those types of monsters half of the time */
        if (rn2(2) && is_boss_caster(mtmp))
            mtmp->mspec_used = 0;
        /* any SPFX_EREGEN artifact (e.g. Eye of the Aethiopica) drops
           mspec to 0, granting effectively unlimited spellcasting */
        if (mon_has_eregen_artifact(mtmp))
            mtmp->mspec_used = 0;
    }

    /* Monster can cast spells, but is casting a directed spell at the
     * wrong place?  If so, give a message, and return.
     * Do this *after* penalizing mspec_used.
     *
     * FIXME?
     *  Shouldn't wall of lava have a case similar to wall of water?
     *  And should cold damage hit water or lava instead of missing
     *  even when the caster has targeted the wrong spot?  Likewise
     *  for fire mis-aimed at ice.
     */
    if (!foundyou && thinks_it_foundyou
        && !is_undirected_spell(mtmp, spellnum)) {
        pline_mon(mtmp, "%s casts a spell at %s!",
                 canseemon(mtmp) ? Monnam(mtmp) : "Something",
                 is_waterwall(mtmp->mux, mtmp->muy) ? "empty water"
                                                    : "thin air");
        return M_ATTK_MISS;
    }

    if (rn2(ml * 10) < (mtmp->mconf ? 100 : 10)) { /* fumbled attack */
        Soundeffect(se_air_crackles, 60);
        if (canseemon(mtmp) && !Deaf) {
            set_msg_xy(mtmp->mx, mtmp->my);
            pline_The("air crackles around %s.", mon_nam(mtmp));
        }
        /* hostile spellcasters interrupt player actions */
        if (!mtmp->mpeaceful)
            stop_occupation();
        return M_ATTK_MISS;
    }
    /* the learned-spell meta doesn't decide which spell is cast until
       cast_learned_spell() runs; resolve it now so the casting message
       matches the spell actually cast (self-buff vs directed) instead
       of guessing from whether a useful self-buff merely exists; the
       same spell is handed to cast_learned_spell() so the message and
       the effect cannot diverge */
    if ((mattk->adtyp == AD_SPEL || mattk->adtyp == AD_CLRC)
        && spellnum == MCAST_LEARNED_SPELL)
        learned_otyp = mpick_learned_spell(mtmp, &gy.youmonst);
    undirected = is_undirected_spell(mtmp, spellnum);
    if (learned_otyp)
        undirected = IS_SELFBUFF_SPELL(learned_otyp);

    if (canspotmon(mtmp) || !undirected) {
        pline_mon(mtmp, "%s casts a spell%s!",
                 canspotmon(mtmp) ? Monnam(mtmp) : "Something",
                 undirected ? ""
                 : (Invis && !mon_prop(mtmp, SEE_INVIS)
                    && !u_at(mtmp->mux, mtmp->muy))
                   ? " at a spot near you"
                   : (Displaced && !u_at(mtmp->mux, mtmp->muy))
                     ? " at your displaced image"
                     : " at you");
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
            return M_ATTK_MISS;
        }
    } else if (mattk->damd)
        dmg = d((int) ((ml / 2) + mattk->damn), (int) mattk->damd);
    else
        dmg = d((int) ((ml / 2) + 1), 6);
    if (Half_spell_damage)
        dmg = (dmg + 1) / 2;

    ret = M_ATTK_HIT;
    /*
     * FIXME: none of these hit the steed when hero is riding, nor do
     *  they inflict damage on carried items.
     */
    switch (mattk->adtyp) {
    case AD_FIRE: {
        /* demons cast hellfire, which sears the soul; the living can
           only be fully protected from it if they are demonic or
           undead themselves */
        boolean hellfire = is_demon(mtmp->data),
                soul_exempt = (Race_if(PM_DEMON) || Race_if(PM_DRAUGR)
                               || Race_if(PM_VAMPIRE)
                               || nonliving(gy.youmonst.data));

        if (hellfire)
            pline("You're enveloped in hellfire!");
        else if (!Underwater)
            pline("You're enveloped in flames.");
        else {
            pline_The("flames are quenched by the water around you.");
            dmg = 0;
            break;
        }
        if (how_resistant(FIRE_RES) == 100) {
            shieldeff(u.ux, u.uy);
            if (hellfire) {
                if (soul_exempt) {
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
            if (hellfire && !soul_exempt) {
                pline_The("hellish flames sear your soul!");
                dmg = resist_reduce(dmg, FIRE_RES) * 2;
            } else {
                dmg = resist_reduce(dmg, FIRE_RES);
            }
            monstunseesu(M_SEEN_FIRE);
        }
        burn_away_slime();
        /* burn up flammable items on the floor, melt ice terrain */
        mon_spell_hits_spot(mtmp, AD_FIRE, u.ux, u.uy);
        break;
    }
    case AD_COLD:
        pline("You're covered in frost.");
        if (how_resistant(COLD_RES) == 100) {
            shieldeff(u.ux, u.uy);
            pline("But you resist the effects.");
            monstseesu(M_SEEN_COLD);
            dmg = 0;
        } else {
            dmg = resist_reduce(dmg, COLD_RES);
            monstunseesu(M_SEEN_COLD);
        }
        /* freeze water or lava terrain */
        /* FIXME: mon_spell_hits_spot() uses zap_over_floor(); unlike with
         * fire, it does not target susceptible floor items with cold */
        mon_spell_hits_spot(mtmp, AD_COLD, u.ux, u.uy);
        break;
    case AD_ACID:
        if (!Underwater) {
            pline("You're splashed in acid.");
        } else {
            pline_The("acid dissipates harmlessly in the water around you.");
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
            monstunseesu(M_SEEN_ACID);
        }
        break;
    case AD_MAGM:
        You("are hit by a shower of missiles!");
        dmg = d((int) ml / 2 + 1, 6);
        /* magic resistance halves the missiles rather than stopping them */
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
            pline("Some missiles bounce off!");
            monstseesu(M_SEEN_MAGR);
            dmg = (dmg + 1) / 2;
        } else {
            monstunseesu(M_SEEN_MAGR);
        }
        /* shower of magic missiles scuffs an engraving */
        mon_spell_hits_spot(mtmp, AD_MAGM, u.ux, u.uy);
        break;
    case AD_SPEL: /* wizard spell */
    case AD_CLRC: /* clerical spell */
        mcast_spell(mtmp, &gy.youmonst, dmg, spellnum, learned_otyp,
                    (int) mattk->adtyp);
        dmg = 0; /* done by the spell casting functions */
        break;
    } /* switch */
    if (dmg)
        mdamageu(mtmp, dmg);
    /* hostile spellcasters interrupt player actions */
    if (!mtmp->mpeaceful)
        stop_occupation();
    /* an area spell centered on the hero can catch an adjacent caster
       in its own blast */
    if (DEADMONSTER(mtmp))
        return M_ATTK_AGR_DIED;
    return ret;
}

staticfn int
m_cure_self(struct monst *mtmp, int dmg)
{
    if (mtmp == &gy.youmonst) {
        /* the polymorphed hero heals the form's HP pool */
        if (u.mh < u.mhmax) {
            You("feel better.");
            if ((u.mh += d(3, 6)) > u.mhmax)
                u.mh = u.mhmax;
            disp.botl = TRUE;
            dmg = 0;
        }
        return dmg;
    }
    if (mtmp->mhp < mtmp->mhpmax) {
        if (canseemon(mtmp))
            pline_mon(mtmp, "%s looks better.", Monnam(mtmp));
        /* note: player healing does 6d4; this used to do 1d8 */
        healmon(mtmp, d(3, 6), 0);
        dmg = 0;
    }
    return dmg;
}

/* clerical protection self-ward. The monster leg mirrors the hero
   spell's diminishing-returns formula inline since cast_protection()
   is hero-only. find_mac() already subtracts mprotection, so adding
   it back reconstructs the unwarded AC (the u.uac + u.uspellprot
   idiom). The gain applies unconditionally: a non-positive gain
   erodes a tall stack toward its equilibrium level, and since the
   subtrahend can never exceed the current stack the sum stays at
   least one - the uchar cannot wrap */
staticfn void
mcast_protection(struct monst *mtmp)
{
    int ml, natac, loglev = 0, gain;

    if (mtmp == &gy.youmonst) {
        cast_protection(P_SKILL(spell_skilltype(SPE_PROTECTION)));
        return;
    }
    ml = min((int) mtmp->m_lev, 50);
    natac = find_mac(mtmp) + mtmp->mprotection;
    for (; ml > 0; ml /= 2)
        loglev++;
    gain = loglev
           - (int) mtmp->mprotection / (4 - min(3, (10 - natac) / 10));
    /* the timer arms on first acquisition whether or not the caster
       is seen; the per-turn step-down depends on it */
    if (gain && !mtmp->mprotection)
        mtmp->mprottime = (mtmp->iswiz || is_prince(mtmp->data)
                           || mtmp->data->msound == MS_NEMESIS
                           || mtmp->data->msound == MS_LEADER) ? 20 : 10;
    if (mtmp->mpeaceful && mtmp->ispriest && inhistemple(mtmp)) {
        ; /* cut down on the temple spam */
    } else if (gain && canseemon(mtmp)) {
        if (mtmp->mprotection)
            pline_The("%s haze around %s becomes more dense.",
                      hcolor(NH_GOLDEN), mon_nam(mtmp));
        else
            pline_The("air around %s begins to shimmer with a %s haze.",
                      mon_nam(mtmp), hcolor(NH_GOLDEN));
    }
    mtmp->mprotection += gain;
}

/* the touch of death (the monster spell, or Death's own touch) lands on
   a hero who is not immune to death magic: without magic resistance it
   kills outright, even through polymorph; with it, the hero survives but
   takes heavy damage and a permanent loss of maximum hit points */
void
touch_of_death(struct monst *mtmp)
{
    char kbuf[BUFSZ];

    (void) death_inflicted_by(kbuf, "the touch of death", mtmp);

    /* either arm below can end the game; the toucher then loots the
       bones pile and the hero rises as after its melee kill */
    if (!Antimagic) {
        monstunseesu(M_SEEN_MAGR);
        svk.killer.format = KILLED_BY;
        Strcpy(svk.killer.name, kbuf);
        set_monster_killer(mtmp, TRUE);
        done(DIED);
    } else {
        int dmg = d(8, 6) / 2;

        shieldeff(u.ux, u.uy);
        monstseesu(M_SEEN_MAGR);
        You("feel drained...");
        drain_uhpmax(dmg / 3 + rn2(5));
        set_monster_killer_hp(mtmp, dmg, TRUE);
        losehp(dmg, kbuf, KILLED_BY);
    }
    svk.killer.name[0] = '\0'; /* not killed if we get here... */
    clear_monster_killer(mtmp);
}

/* articled species name of the monster responsible for a kill, for the
   death-reason builders: "a goblin", "an elven soldier", "the Wizard of
   Yendor", "Asmodeus" (plus " imitating <X>" for a shapeshifter); names
   by race and species, like done_in_by() does for the simple cases and
   for a shapeshifter's form, and names a shopkeeper, priest or minion
   individually, as done_in_by() does ("Mr. Asidonhopo, the shopkeeper",
   "the high priest of Moloch", "a priestess of Odin") */
char *
mon_killer_name(
    char *outbuf, /* assumed big enough; every name built here is short */
    struct monst *mtmp)
{
    struct permonst *mptr = mtmp->data,
        *champtr = (ismnum(mtmp->cham)) ? &mons[mtmp->cham] : mptr;
    const char *realnm = pmname(champtr, Mgender(mtmp)),
        *fakenm = pmname(mptr, Mgender(mtmp));
    char racebuf[BUFSZ];

    if (champtr == mptr && mtmp->isshk)
        return shk_killer_name(outbuf, mtmp);
    if (champtr == mptr && (mtmp->ispriest || mtmp->isminion)) {
        /* m_monnam() gives "priestess of Odin": the true name, deity
           always shown, no article; a temple's own high priest is
           unique, one without a temple is not */
        const char *pnm = m_monnam(mtmp);

        if ((mptr->geno & G_UNIQ) != 0
            && !(mptr == &mons[PM_HIGH_PRIEST] && !mtmp->ispriest))
            Sprintf(outbuf, "%s%s", type_is_pname(mptr) ? "" : "the ",
                    pnm);
        else
            Strcpy(outbuf, an(pnm));
        return outbuf;
    }

    /* the race done_in_by() gives the current form, as x_monnam() names
       it ("a centaurian soldier", "a doppelganger imitating a dwarvish
       valkyrie"; newcham() keeps a race only for a racial form);
       shopkeeper, priest and minion names carry none */
    if (has_erac(mtmp) && !type_is_pname(mptr)
        && mptr != &mons[PM_HIGH_PRIEST]
        && !mtmp->isshk && !mtmp->ispriest && !mtmp->isminion) {
        Strcat(racial_adj(racebuf, mtmp), fakenm);
        if (champtr == mptr)
            realnm = racebuf;
        else
            fakenm = racebuf;
    }
    /* greatly simplified extract from done_in_by(); names the real
       species when a shapeshifter is disguised */
    if (!type_is_pname(champtr) && !the_unique_pm(champtr))
        realnm = an(realnm);
    Sprintf(outbuf, "%s%s", the_unique_pm(champtr) ? "the " : "", realnm);
    if (champtr != mptr) {
        /* the form takes the same article choice ("imitating Medusa") */
        if (!type_is_pname(mptr) && !the_unique_pm(mptr))
            fakenm = an(fakenm);
        Sprintf(eos(outbuf), " imitating %s%s",
                the_unique_pm(mptr) ? "the " : "", fakenm);
    }
    return outbuf;
}

/* append " inflicted by <monster>" to a death reason; the caller may then
   strsubst("inflicted", verb) to get "zapped/cast/exhaled by <monster>" */
char *
death_inflicted_by(
    char *outbuf,            /* BUFSZ chars; every caller passes one */
    const char *deathreason, /* cause of death, possibly an object name */
    struct monst *mtmp)      /* monster who caused it */
{
    copynchars(outbuf, deathreason, BUFSZ - 1);
    if (mtmp) {
        char nbuf[BUFSZ];

        Snprintf(eos(outbuf), BUFSZ - strlen(outbuf), " inflicted by %s",
                 mon_killer_name(nbuf, mtmp));
    }
    return outbuf;
}

/*
 * Monster wizard and cleric spellcasting functions.
 */

staticfn void
mcast_death_touch(struct monst *mtmp, struct monst *mdef)
{
    boolean udefend = (mdef == &gy.youmonst);

    if (udefend) {
        int pg = pronoun_gender(mtmp, PRONOUN_HALLU);

        /* 3 is the plural "they", possible when hallucinating */
        pline("Oh no, %s%s using the touch of death!", genders[pg].he,
              (pg == 3) ? "'re" : "'s");
        if (Death_resistance || immune_death_magic(gy.youmonst.data)) {
            shieldeff(u.ux, u.uy);
            monstseesu(M_SEEN_DEATH);
            You("%s.", nonliving(gy.youmonst.data)
                           ? "seem no more dead than before"
                           : "are unaffected");
        } else if (Hallucination) {
            You("have an out of body experience.");
        } else {
            /* no escape roll: fatal without magic resistance, a heavy
               drain with it */
            touch_of_death(mtmp);
        }
    } else {
        boolean yours = (mtmp == &gy.youmonst);
        boolean resisted;

        if (!mdef || DEADMONSTER(mdef))
            return;
        if (yours) {
            You("are using the touch of death!");
        } else if (canseemon(mtmp) || tp_sensemon(mtmp)
                   || Detect_monsters) {
            char buf[BUFSZ];
            int pg = pronoun_gender(mtmp, PRONOUN_HALLU);

            /* 3 is the plural "they", possible when hallucinating */
            Sprintf(buf, "%s%s%s", mtmp->mtame ? "Oh no, " : "",
                    genders[pg].he, (pg == 3) ? "'re" : "'s");
            if (!mtmp->mtame)
                *buf = highc(*buf);
            pline("%s using the touch of death!", buf);
        }

        /* the escape roll reads the caster's base species level; a
           grown individual can outlevel a level-0 species, so floor
           the die at 1 */
        resisted = ((resist(mdef, 0, 0, FALSE)
                     && rn2(max((int) (yours ? mons[u.umonnum].mlevel
                                             : mtmp->data->mlevel),
                                1)) <= 12)
                    || resists_magm(mdef) || defended(mdef, AD_MAGM));
        if (immune_death_magic(mdef->data) || is_vampshifter(mdef)) {
            shieldeff(mdef->mx, mdef->my);
            if (yours || canseemon(mdef))
                pline("%s %s.", Monnam(mdef),
                      nonliving(mdef->data)
                          ? "seems no more dead than before"
                          : "is unaffected");
        } else if (!resisted) {
            mdef->mhp = -1;
            if (yours) {
                killed(mdef);
            } else {
                if (mtmp->mtame)
                    set_pet_killer(mtmp);
                monkilled(mdef, "", AD_SPEL);
            }
            return;
        } else {
            shieldeff(mdef->mx, mdef->my);
            if (yours || canseemon(mdef)) {
                if (mdef->mtame)
                    pline("Lucky for %s, it didn't work!", mon_nam(mdef));
                else
                    pline("Well.  That didn't work...");
            }
        }
    }
}

staticfn void
mcast_clone_wiz(struct monst *mtmp)
{
    if (mtmp->iswiz && svc.context.no_of_wizards == 1) {
        pline("Double Trouble...");
        clonewiz();
    } else
        impossible("bad wizard cloning?");
}

staticfn void
mcast_summon_mons(struct monst *mtmp, struct monst *mdef)
{
    boolean udefend = (mdef == &gy.youmonst);

    if (udefend) {
        int count = nasty(mtmp, FALSE);

        if (!count) {
            ; /* nothing was created? */
        } else if (mtmp->iswiz) {
            SetVoice(mtmp, 0, 80, 0);
            verbalize("Destroy the thief, my pet%s!", plur(count));
        } else if (mtmp->data == &mons[PM_KATHRYN_THE_ICE_QUEEN]) {
            SetVoice(mtmp, 0, 80, 0);
            verbalize("Defend me, my minion%s!", plur(count));
        } else {
            boolean one = (count == 1);
            const char *mappear = one ? "A monster appears"
                                      : "Monsters appear";

            /* messages not quite right if plural monsters created but
               only a single monster is seen */
            if (Invis && !mon_prop(mtmp, SEE_INVIS)
                && (mtmp->mux != u.ux || mtmp->muy != u.uy))
                pline("%s %s a spot near you!", mappear,
                      one ? "at" : "around");
            else if (Displaced && (mtmp->mux != u.ux || mtmp->muy != u.uy))
                pline("%s %s your displaced image!", mappear,
                      one ? "by" : "around");
            else
                pline("%s from nowhere!", mappear);
        }
    } else {
        boolean yours = (mtmp == &gy.youmonst);
        struct monst *mpet;
        int count = 0;

        if (!mdef || DEADMONSTER(mdef))
            return;
        if (!yours && !rn2(10) && Inhell) {
            /* in Gehennom the summons is sometimes the caster's own
               demonic kin; the hero never reaches this helper - the
               no-free-pets clause rejects the pick at selection */
            (void) msummon(mtmp);
        } else {
            int i, j, makeindex, tmp = (u.ulevel > 3) ? u.ulevel / 3 : 1;
            coord bypos;

            for (i = rnd(tmp); i > 0; --i) {
                for (j = 0; j < 20; j++) {
                    /* avoid spellcasters stronger than the hero's own
                       form; the lid is hero-relative even for monster
                       casters */
                    do {
                        makeindex = pick_nasty(0);
                    } while (attacktype(&mons[makeindex], AT_MAGC)
                             && mons[makeindex].difficulty
                                >= mons[u.umonnum].difficulty);
                    if (yours) {
                        if (!enexto(&bypos, u.ux, u.uy,
                                    &mons[makeindex]))
                            continue;
                    } else if (!enexto(&bypos, mtmp->mx, mtmp->my,
                                       &mons[makeindex])) {
                        continue;
                    }
                    mpet = makemon(&mons[makeindex], bypos.x, bypos.y,
                                   (yours || mtmp->mtame)
                                       ? (MM_EDOG | MM_NOMSG)
                                       : MM_NOMSG);
                    if (mpet) {
                        mpet->msleeping = 0;
                        if (yours || mtmp->mtame)
                            initedog(mpet, TRUE);
                        else if (mtmp->mpeaceful)
                            mpet->mpeaceful = 1;
                        else
                            mpet->mpeaceful = mpet->mtame = 0;
                        set_malign(mpet);
                    } else {
                        mpet = makemon((struct permonst *) 0,
                                       bypos.x, bypos.y, MM_NOMSG);
                    }
                    if (mpet && (u.ualign.type == 0
                                 || mon_aligntyp(mpet) == 0
                                 || sgn(mon_aligntyp(mpet))
                                    == sgn(u.ualign.type))) {
                        count++;
                        break;
                    }
                }
            }
            if (count > 0 && (yours || canseemon(mdef)))
                pline("%s from nowhere!",
                      (count == 1) ? "A monster appears"
                                   : "Monsters appear");
        }
    }
}

DISABLE_WARNING_FORMAT_NONLITERAL

/* curse a few of a monster's inventory items at random; counterpart of
   rndcurse() for the hero */
staticfn void
mrndcurse(struct monst *mtmp)
{
    int nobj = 0;
    int cnt, onum;
    struct obj *otmp;
    static const char mal_aura[] = "feel a malignant aura surround %s.";
    boolean resists = resist(mtmp, 0, 0, FALSE) ? TRUE : FALSE,
            vis = couldsee(mtmp->mx, mtmp->my);

    if (mon_wielding_artifact(mtmp, ART_MAGICBANE) && rn2(20)) {
        if (vis)
            You(mal_aura, "the magic-absorbing staff");
        return;
    }
    if (mon_wielding_artifact(mtmp, ART_STAFF_OF_THE_ARCHMAGI)
        && rn2(30)) {
        if (vis)
            You(mal_aura, "the powerful staff");
        return;
    }

    if (vis && resists) {
        shieldeff(mtmp->mx, mtmp->my);
        You(mal_aura, mon_nam(mtmp));
    }

    for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj) {
        /* gold isn't subject to being cursed or blessed */
        if (otmp->oclass == COIN_CLASS)
            continue;
        nobj++;
    }

    if (nobj) {
        for (cnt = rnd(6 / ((!!resists) + 1)); cnt > 0; cnt--) {
            onum = rnd(nobj);
            for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj) {
                /* as above */
                if (otmp->oclass == COIN_CLASS)
                    continue;
                if (--onum == 0)
                    break; /* found the target */
            }
            /* the !otmp case should never happen; picking an already
               cursed item happens--avoid "resists" message in that case */
            if (!otmp || otmp->cursed)
                continue; /* next target */

            if (otmp->oartifact
                && spec_ability(otmp, SPFX_INTEL) && rn2(10) < 8) {
                if (vis)
                    pline("%s!", Tobjnam(otmp, "resist"));
                continue;
            }

            if (otmp->blessed)
                unbless(otmp);
            else
                curse(otmp);
        }
    }
}

RESTORE_WARNING_FORMAT_NONLITERAL

/* erode or destroy one piece of the target's worn armor. mdef is the
   hero on the castmu() path and a monster target on the castmm()
   path. Magic resistance softens the spell to one erosion level
   rather than blocking it */
staticfn void
mcast_destroy_armor(struct monst *mtmp, struct monst *mdef)
{
    boolean udefend = (mdef == &gy.youmonst), worked = FALSE;
    int erodelvl = rnd(3);
    struct obj *oatmp;

    if (udefend ? (Antimagic != 0)
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
                pline("%s is immune to the destructive magic.",
                      The(xname(oatmp)));
            }
            return; /* no effect */
        } else if (oatmp->otyp == CRYSTAL_PLATE_MAIL) {
            /* crystal plate mail can never be destroyed, worn by the
               hero or by a monster */
            if (udefend || canseemon(mdef)) {
                if (!Blind)
                    pline("%s glimmers brightly.", Yname2(oatmp));
                pline("%s is immune to the destructive magic.",
                      Yname2(oatmp));
            }
            return; /* no effect */
        /* TODO: supermaterials should get their own struct obj bit so
           the oerodeproof marker below isn't needed (breaks saves) */
        } else if (oatmp->oerodeproof && is_supermaterial(oatmp)) {
            /* marker from an earlier hit: clear it and let the full
               spell through */
            oatmp->oerodeproof = 0;
        } else if (oatmp->oerodeproof) {
            /* genuine erodeproofing absorbs one hit and burns off */
            if (!udefend && !canseemon(mdef))
                You("smell something strange.");
            else if (!Blind)
                pline("%s glows brown for a moment.", Yname2(oatmp));
            else
                pline("%s briefly emits an odd smell.", Yname2(oatmp));
            (void) maybe_erodeproof(oatmp, 0);
            erodelvl--;
            worked = TRUE;
        } else if (is_supermaterial(oatmp) && greatest_erosion(oatmp) == 0) {
            /* a pristine supermaterial absorbs the whole spell; set
               the marker so the next hit lands in full */
            if (!udefend && !canseemon(mdef))
                You("smell something strange.");
            else if (!Blind)
                pline("%s glows brown for a moment.", Yname2(oatmp));
            else
                pline("%s briefly emits an odd smell.", Yname2(oatmp));
            oatmp->oerodeproof = 1;
            erodelvl = 0;
        }

        if (greatest_erosion(oatmp) >= MAX_ERODE) {
            if (objects[oatmp->otyp].oc_oprop == DISINT_RES
                || obj_resists(oatmp, 0, 90))
                return;
            if (udefend) {
                if (disintegrate_arm(oatmp))
                    worked = TRUE;
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
                    else if (is_shield(oatmp))
                        action = is_bracer(oatmp) ? "crumble away"
                                                  : "crumbles away";
                    else
                        action = "turns to dust";
                    pline_mon(mdef, "%s %s %s!", s_suffix(Monnam(mdef)),
                              xname(oatmp), action);
                }
                m_useupall(mdef, oatmp);
                worked = TRUE;
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
                if (erode_obj(oatmp, (char *) 0, erodetype, EF_NONE)
                    != ER_NOTHING)
                    worked = TRUE;
            }
        }
    } else {
        if (udefend)
            Your("body itches.");
        else if (canseemon(mdef))
            pline_mon(mdef, "%s seems irritated.", Monnam(mdef));
    }
    /* the caster only learns the hero lacks magic protection when the
       spell actually bites */
    if (udefend && worked && !Antimagic)
        monstunseesu(M_SEEN_MAGR);
    update_inventory();
    nhUse(mtmp);
}

staticfn void
mcast_weaken_you(struct monst *mtmp, struct monst *mdef, int dmg)
{
    boolean udefend = (mdef == &gy.youmonst);

    if (udefend) {
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
            monstseesu(M_SEEN_MAGR);
            You_feel("momentarily weakened.");
        } else {
            char kbuf[BUFSZ];

            You("suddenly feel weaker!");
            dmg = mtmp->m_lev - 6;
            if (dmg < 1) /* paranoia since only chosen when m_lev is high */
                dmg = 1;
            if (Half_spell_damage)
                dmg = (dmg + 1) / 2;
            /* losestr() ends the game from inside the call when the
               loss is fatal to the hero's real body; the caster then
               loots the bones pile and the hero rises as after its
               melee kill (a polymorphed hero just reverts, as from
               mdamageu()) */
            if (!Upolyd)
                set_monster_killer(mtmp, TRUE);
            losestr(rnd(dmg),
                    death_inflicted_by(kbuf, "strength loss", mtmp),
                    KILLED_BY);
            svk.killer.name[0] = '\0'; /* not killed if we get here... */
            clear_monster_killer(mtmp);
            monstunseesu(M_SEEN_MAGR);
        }
    } else {
        boolean yours = (mtmp == &gy.youmonst);

        if (!mdef || DEADMONSTER(mdef))
            return;
        if (resist(mdef, 0, 0, FALSE)) {
            shieldeff(mdef->mx, mdef->my);
            if (canseemon(mdef))
                pline("%s looks momentarily weakened.", Monnam(mdef));
        } else {
            if (yours || canseemon(mdef))
                pline("%s suddenly seems weaker!", Monnam(mdef));
            if (mon_arti_has_spfx(mdef, SPFX_HSPDAM))
                dmg = (dmg + 1) / 2;
            /* monsters don't have strength, so drain max hp instead */
            mdef->mhpmax -= dmg;
            if (damage_mon(mdef, dmg, AD_SPEL, yours)) {
                if (yours) {
                    killed(mdef);
                } else {
                    if (mtmp->mtame)
                        set_pet_killer(mtmp);
                    monkilled(mdef, "", AD_SPEL);
                }
            }
        }
    }
}

staticfn void
mcast_disappear(struct monst *mtmp)
{
    if (mtmp == &gy.youmonst) {
        /* the hero turns invisible */
        if (!(HInvis & INTRINSIC)) {
            HInvis |= FROMOUTSIDE;
            if (!Blind && !BInvis)
                self_invis_message();
        }
        return;
    }
    if (!mtmp->minvis && !mtmp->invis_blkd) {
        if (canseemon(mtmp))
            pline_mon(mtmp, "%s suddenly %s!", Monnam(mtmp),
                      !See_invisible ? "disappears" : "becomes transparent");
        mon_set_minvis(mtmp, FALSE);
        if (cansee(mtmp->mx, mtmp->my) && !canspotmon(mtmp))
            map_invisible(mtmp->mx, mtmp->my);
    } else
        impossible("no reason for monster to cast disappear spell?");
}

staticfn void
mcast_stun_you(struct monst *mtmp, struct monst *mdef, int dmg)
{
    boolean udefend = (mdef == &gy.youmonst);

    if (udefend) {
        if (Antimagic || Free_action || Hidinshell) {
            shieldeff(u.ux, u.uy);
            monstseesu(M_SEEN_MAGR);
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
            monstunseesu(M_SEEN_MAGR);
        }
    } else {
        boolean yours = (mtmp == &gy.youmonst);

        if (!mdef || DEADMONSTER(mdef))
            return;
        if (resist(mdef, 0, 0, FALSE)) {
            shieldeff(mdef->mx, mdef->my);
            if (yours || canseemon(mdef)) {
                if (mon_resists_stun(mdef) || defended(mdef, AD_STUN)
                    || mon_wielding_artifact(mdef, ART_TEMPEST))
                    pline("%s seems momentarily disoriented.",
                          Monnam(mdef));
            }
        } else {
            if (yours || canseemon(mdef)) {
                if (mdef->mstun)
                    pline("%s struggles to keep %s balance.",
                          Monnam(mdef), mhis(mdef));
                else
                    pline("%s reels...", Monnam(mdef));
            }
            mdef->mstun = 1;
        }
    }
}

staticfn int
mcast_geyser(struct monst *mtmp, struct monst *mdef, int dmg)
{
    boolean udefend = (mdef == &gy.youmonst);

    /* this is physical damage (force not heat),
     * not magical damage or fire damage
     */
    if (udefend) {
        if (mtmp->data == &mons[PM_KATHRYN_THE_ICE_QUEEN]
            || mtmp->data == &mons[PM_ASMODEUS]
            || mtmp->data == &mons[PM_VECNA]) {
            /* cold-aligned casters bury the hero instead */
            pline("An avalanche of ice and snow slams into you from nowhere!");
            dmg = d(8, 8);
            if (Half_physical_damage)
                dmg = (dmg + 1) / 2;
            (void) destroy_items(&gy.youmonst, AD_COLD, dmg);
            return dmg;
        }
        pline("A sudden geyser slams into you from nowhere!");
        dmg = d(8, 6);
        if (Half_physical_damage)
            dmg = (dmg + 1) / 2;
        /* significant water is fatal to these polymorphed forms */
        if (u.umonnum == PM_IRON_GOLEM) {
            You("rust!");
            rehumanize();
            return dmg;
        }
        if (u.umonnum == PM_FIRE_VORTEX || u.umonnum == PM_FIRE_ELEMENTAL
            || u.umonnum == PM_FLAMING_SPHERE) {
            /* PM_ORB_OF_FIRE is M2_NOPOLY */
            You("are extinguished!");
            rehumanize();
            return dmg;
        }
        (void) erode_armor(&gy.youmonst, ERODE_RUST);
#if 0   /* since inventory items aren't affected, don't include this */
        /* make floor items wet */
        water_damage_chain(level.objects[u.ux][u.uy], TRUE, 0, TRUE);
#endif
        return dmg;
    } else {
        boolean yours = (mtmp == &gy.youmonst);
        int itemdmg = 0;

        if (!mdef || DEADMONSTER(mdef))
            return 0;
        if (mtmp->data == &mons[PM_KATHRYN_THE_ICE_QUEEN]
            || mtmp->data == &mons[PM_ASMODEUS]
            || mtmp->data == &mons[PM_VECNA]) {
            /* cold-aligned casters bury the target instead */
            if (yours || canseemon(mdef))
                pline(
                 "An avalanche of ice and snow slams into %s from nowhere!",
                      mon_nam(mdef));
            dmg = d(8, 8);
            itemdmg = destroy_items(mdef, AD_COLD, dmg);
        } else {
            if (yours || canseemon(mdef))
                pline("A sudden geyser slams into %s from nowhere!",
                      mon_nam(mdef));
            if (mdef->data == &mons[PM_IRON_GOLEM]) {
                if (yours || canseemon(mdef))
                    pline("%s falls to pieces!", Monnam(mdef));
                else if (mdef->mtame)
                    pline("May %s rust in peace.", mon_nam(mdef));
                if (yours) {
                    xkilled(mdef, XKILL_NOMSG);
                } else {
                    if (mtmp->mtame)
                        set_pet_killer(mtmp);
                    monkilled(mdef, "", AD_PHYS);
                }
                return 0;
            }
            if (completelydoused(mdef->data)) {
                if (yours || canseemon(mdef))
                    pline("%s is extinguished!", Monnam(mdef));
                if (yours) {
                    xkilled(mdef, XKILL_NOMSG);
                } else {
                    if (mtmp->mtame)
                        set_pet_killer(mtmp);
                    monkilled(mdef, "", AD_PHYS);
                }
                return 0;
            }
            dmg = d(8, 6);
            (void) erode_armor(mdef, ERODE_RUST);
        }
        /* geyser and avalanche are physical (force and water), not
           spell damage; mirror the Half_physical_damage halving the
           hero gets */
        if (dmg > 0 && mon_arti_has_spfx(mdef, SPFX_HPHDAM))
            dmg = (dmg + 1) / 2;
        /* shattered potions hurt on top of that, unhalved, as the
           hero's own do */
        return dmg + itemdmg;
    }
}

staticfn int
mcast_fire_pillar(struct monst *mtmp, struct monst *mdef, int dmg)
{
    boolean udefend = (mdef == &gy.youmonst);
    int orig_dmg;

    if (udefend) {
        pline("A pillar of fire strikes all around you!");
        orig_dmg = dmg = d(8, 6);
        if (how_resistant(FIRE_RES) == 100) {
            shieldeff(u.ux, u.uy);
            monstseesu(M_SEEN_FIRE);
            dmg = 0;
        } else {
            dmg = resist_reduce(dmg, FIRE_RES);
            monstunseesu(M_SEEN_FIRE);
        }
        if (Half_spell_damage)
            dmg = (dmg + 1) / 2;
        burn_away_slime();
        (void) burnarmor(&gy.youmonst);
        /* item destruction dmg */
        (void) destroy_items(&gy.youmonst, AD_FIRE, orig_dmg);
        ignite_items(gi.invent);
        /* burn up flammable items on the floor, melt ice terrain */
        mon_spell_hits_spot(mtmp, AD_FIRE, u.ux, u.uy);
        return dmg;
    } else {
        boolean yours = (mtmp == &gy.youmonst);

        if (!mdef || DEADMONSTER(mdef))
            return 0;
        if (yours || canseemon(mdef))
            pline("A pillar of fire strikes all around %s!",
                  mon_nam(mdef));
        orig_dmg = dmg = d(8, 6);
        if (resists_fire(mdef) || defended(mdef, AD_FIRE)) {
            shieldeff(mdef->mx, mdef->my);
            dmg = 0;
        }
        if (dmg > 0 && mon_arti_has_spfx(mdef, SPFX_HSPDAM))
            dmg = (dmg + 1) / 2;
        (void) burnarmor(mdef);
        /* item destruction dmg */
        dmg += destroy_items(mdef, AD_FIRE, orig_dmg);
        /* burn up flammable items on the floor, melt ice terrain */
        mon_spell_hits_spot(mtmp, AD_FIRE, mdef->mx, mdef->my);
        /* the spot effects (melting ice, a boiled-away pool's new pit)
           may have killed the target or sent it off the map; that is
           already handled, so leave nothing for castmm to apply */
        if (DEADMONSTER(mdef) || mon_offmap(mdef))
            return 0;
        return dmg;
    }
}

staticfn int
mcast_lightning(struct monst *mtmp, struct monst *mdef, int dmg)
{
    boolean udefend = (mdef == &gy.youmonst);
    int orig_dmg;
    boolean reflects;

    if (udefend) {
        Soundeffect(se_bolt_of_lightning, 80);
        pline("A bolt of lightning strikes down at you from above!");
        /* reflection is only partial: the hero still takes a lesser
           bolt, but belongings, terrain and eyes are spared */
        reflects = ureflects("Some of it bounces off your %s%s.", "");
        orig_dmg = dmg = d(8, 6);
        if (reflects || how_resistant(SHOCK_RES) == 100) {
            shieldeff(u.ux, u.uy);
            if (reflects) {
                dmg = resist_reduce(d(4, 6), SHOCK_RES);
                monstseesu(M_SEEN_REFL);
            } else {
                monstunseesu(M_SEEN_REFL);
            }
            if (how_resistant(SHOCK_RES) == 100) {
                pline("You aren't shocked.");
                monstseesu(M_SEEN_ELEC);
                dmg = 0;
            } else {
                monstunseesu(M_SEEN_ELEC);
            }
        } else {
            dmg = resist_reduce(dmg, SHOCK_RES);
            monstunseesu(M_SEEN_ELEC | M_SEEN_REFL);
        }
        if (dmg && Half_spell_damage)
            dmg = (dmg + 1) / 2;
        if (!reflects) {
            (void) destroy_items(&gy.youmonst, AD_ELEC, orig_dmg);
            /* lightning might destroy iron bars if hero is on such a
               spot; reflection protects terrain here but hero
               resistance doesn't; do this before maybe blinding the
               hero via flashburn() */
            mon_spell_hits_spot(mtmp, AD_ELEC, u.ux, u.uy);
            /* blind hero; no effect if already blind */
            (void) flashburn((long) rnd(100), TRUE);
        }
        return dmg;
    } else {
        boolean yours = (mtmp == &gy.youmonst);

        if (!mdef || DEADMONSTER(mdef))
            return 0;
        if (yours || canseemon(mdef)) {
            Soundeffect(se_bolt_of_lightning, 80);
            pline("A bolt of lightning strikes down at %s from above!",
                  mon_nam(mdef));
        }
        reflects = mon_reflects(mdef, "It bounces off %s %s.");
        orig_dmg = dmg = d(8, 6);
        if (reflects || resists_elec(mdef) || defended(mdef, AD_ELEC)) {
            shieldeff(mdef->mx, mdef->my);
            dmg = 0;
            if (reflects)
                return dmg;
        }
        if (dmg > 0 && mon_arti_has_spfx(mdef, SPFX_HSPDAM))
            dmg = (dmg + 1) / 2;
        /* item destruction dmg; a charged ring can still blow up on a
           resisting target */
        dmg += destroy_items(mdef, AD_ELEC, orig_dmg);
        return dmg;
    }
}

/* douse the target in a torrent of acid. mdef is the hero on the
   castmu() path and a monster target on the castmm() path. damage is
   applied by the explosion itself, so the caller must not apply any
   on top */
staticfn void
mcast_acid_blast(struct monst *mtmp, struct monst *mdef)
{
    boolean udefend = (mdef == &gy.youmonst);
    boolean yours = (mtmp == &gy.youmonst);
    int ml = yours ? mons[u.umonnum].mlevel
                   : min((int) mtmp->m_lev, 50);
    struct monst *save_exploder;

    if (udefend) {
        if (canseemon(mtmp) || tp_sensemon(mtmp) || Detect_monsters)
            pline_mon(mtmp, "%s douses you in a torrent of acid!",
                      Monnam(mtmp));
        /* credit the caster for blast kills; save and restore so a
           chained explosion cannot clobber an outer one */
        save_exploder = ge.exploder;
        ge.exploder = mtmp;
        explode(u.ux, u.uy, BZ_M_SPELL(ZT_ACID), d(ml / 2 + 4, 8), 0,
                EXPL_ACID);
        ge.exploder = save_exploder;
        if (Acid_resistance) {
            shieldeff(u.ux, u.uy);
            pline_The("acid dissipates harmlessly.");
            monstseesu(M_SEEN_ACID);
        }
        /* using two weapons at once makes both of them more vulnerable */
        if (rn2(u.twoweap ? 2 : 3))
            acid_damage(uwep);
        if (u.twoweap && rn2(2))
            acid_damage(uswapwep);
        if (rn2(4))
            erode_armor(&gy.youmonst, ERODE_CORRODE);
    } else {
        if (!mdef || DEADMONSTER(mdef))
            return;
        if (yours)
            You("douse %s in a torrent of acid!", mon_nam(mdef));
        else if (canseemon(mtmp) || canseemon(mdef))
            pline("%s douses %s in a torrent of acid!", Monnam(mtmp),
                  mon_nam(mdef));
        /* a hero-cast blast attributes through the positive zap type;
           the exploder bracket is the monster-caster equivalent */
        if (!yours) {
            save_exploder = ge.exploder;
            ge.exploder = mtmp;
        }
        explode(mdef->mx, mdef->my,
                yours ? BZ_U_SPELL(ZT_ACID) : BZ_M_SPELL(ZT_ACID),
                d(ml / 2 + 4, 8), 0, EXPL_ACID);
        if (!yours)
            ge.exploder = save_exploder;
        /* the blast may have killed the target; don't touch it after */
        if (DEADMONSTER(mdef))
            return;
        if (resists_acid(mdef) || defended(mdef, AD_ACID)) {
            shieldeff(mdef->mx, mdef->my);
            if (canseemon(mdef))
                pline("But the acid dissipates harmlessly.");
        }
        if (rn2(4))
            erode_armor(mdef, ERODE_CORRODE);
    }
}

/* blast the target with a fiery or freezing explosion. mdef is the
   hero on the castmu() path and a monster target on the castmm()
   path. damage is applied by the explosion itself, so the caller must
   not apply any on top */
staticfn void
mcast_bolt(struct monst *mtmp, struct monst *mdef, int spellnum)
{
    boolean udefend = (mdef == &gy.youmonst);
    boolean yours = (mtmp == &gy.youmonst);
    boolean firebolt = (spellnum == MCAST_FIRE_BOLT);
    int ml = yours ? mons[u.umonnum].mlevel
                   : min((int) mtmp->m_lev, 50);
    struct monst *save_exploder;

    if (udefend) {
        int dmg;

        if (canseemon(mtmp) || tp_sensemon(mtmp) || Detect_monsters)
            pline_mon(mtmp, "%s blasts you with %s!", Monnam(mtmp),
                      firebolt ? "fire" : "ice");
        /* the hero's partial resistance applies up front; explode()
           itself only handles the all-or-nothing monster resistances */
        dmg = resist_reduce(d(ml / 5 + 1, 8),
                            firebolt ? FIRE_RES : COLD_RES);
        /* credit the caster for blast kills; save and restore so a
           chained explosion cannot clobber an outer one */
        save_exploder = ge.exploder;
        ge.exploder = mtmp;
        explode(u.ux, u.uy, BZ_M_SPELL(firebolt ? ZT_FIRE : ZT_COLD),
                dmg, 0, firebolt ? EXPL_FIERY : EXPL_FROSTY);
        ge.exploder = save_exploder;
        if (how_resistant(firebolt ? FIRE_RES : COLD_RES) == 100) {
            shieldeff(u.ux, u.uy);
            monstseesu(firebolt ? M_SEEN_FIRE : M_SEEN_COLD);
            pline("But you resist the effects.");
        }
    } else {
        if (!mdef || DEADMONSTER(mdef))
            return;
        if (yours)
            You("blast %s with %s!", mon_nam(mdef),
                firebolt ? "fire" : "ice");
        else if (canseemon(mtmp) || canseemon(mdef))
            pline("%s blasts %s with %s!", Monnam(mtmp), mon_nam(mdef),
                  firebolt ? "fire" : "ice");
        /* a hero-cast blast attributes through the positive zap type;
           the exploder bracket is the monster-caster equivalent */
        if (!yours) {
            save_exploder = ge.exploder;
            ge.exploder = mtmp;
        }
        explode(mdef->mx, mdef->my,
                yours ? BZ_U_SPELL(firebolt ? ZT_FIRE : ZT_COLD)
                      : BZ_M_SPELL(firebolt ? ZT_FIRE : ZT_COLD),
                d(ml / 5 + 1, 8), 0, firebolt ? EXPL_FIERY : EXPL_FROSTY);
        if (!yours)
            ge.exploder = save_exploder;
        /* the blast may have killed the target; don't touch it after */
        if (DEADMONSTER(mdef))
            return;
        if (firebolt ? (resists_fire(mdef) || defended(mdef, AD_FIRE))
                     : (resists_cold(mdef) || defended(mdef, AD_COLD))) {
            shieldeff(mdef->mx, mdef->my);
            if (canseemon(mdef))
                pline("But %s seems unaffected by the %s.", mon_nam(mdef),
                      firebolt ? "fire" : "cold");
        }
    }
}

/* cancel the target's magic. A directed spell: the line-up is
   re-checked here at execution, so a fight that moved off-axis
   wastes the cast, and the messages sit inside the guards - a
   failed line-up is silent. mdef is the hero on the castmu() path
   and a monster target on the castmm() path; the hero-caster leg
   needs no line-up since the hero picked an adjacent target */
staticfn void
mcast_cancellation(struct monst *mtmp, struct monst *mdef)
{
    boolean udefend = (mdef == &gy.youmonst);
    boolean yours = (mtmp == &gy.youmonst);
    struct monst *save_buzzer;

    if (udefend) {
        if (lined_up(mtmp)) {
            if (canseemon(mtmp) || tp_sensemon(mtmp) || Detect_monsters)
                pline_mon(mtmp, "%s %s a cancellation spell!",
                          Monnam(mtmp), rn2(2) ? "evokes" : "conjures up");
            else if (!Deaf)
                You_hear("a powerful spell being %s.",
                         rn2(2) ? "spoken" : "uttered");
            (void) cancel_monst(&gy.youmonst, (struct obj *) 0, FALSE,
                                TRUE, FALSE);
        }
    } else {
        if (!mdef || DEADMONSTER(mdef))
            return;
        if (yours) {
            /* the hero as attacker routes a possible clay-golem kill
               through killed() for normal credit */
            You("%s a cancellation spell!",
                rn2(2) ? "evoke" : "conjure up");
            (void) cancel_monst(mdef, (struct obj *) 0, TRUE, TRUE,
                                FALSE);
        } else if (m_lined_up(mdef, mtmp, FALSE)) {
            if (canseemon(mtmp) || tp_sensemon(mtmp) || Detect_monsters)
                pline_mon(mtmp, "%s %s a cancellation spell!",
                          Monnam(mtmp), rn2(2) ? "evokes" : "conjures up");
            /* credit a possible clay-golem kill to the caster; save
               and restore so an outer beam's blame is not clobbered.
               The cancel may kill the target - no reads after it */
            save_buzzer = gb.buzzer;
            gb.buzzer = mtmp;
            (void) cancel_monst(mdef, (struct obj *) 0, FALSE, TRUE,
                                FALSE);
            gb.buzzer = save_buzzer;
        }
    }
}

/* call forth undead around the target. mdef is the hero on the
   castmu() path; the monster-vs-monster leg summons around the
   caster itself and never reads the target, which may already be
   gone. The summons are ordinary makemon() spawns - their attitude
   comes from their species, not from the caster */
staticfn void
mcast_call_undead(struct monst *mtmp, struct monst *mdef)
{
    coord mm;
    boolean udefend = (mdef == &gy.youmonst);
    boolean yours = (mtmp == &gy.youmonst);

    if (udefend || yours) {
        /* hero casts are rejected by the useless filter (no free
           pets); the leg is kept for safety */
        mm.x = u.ux;
        mm.y = u.uy;
        pline("Undead creatures are called forth from the grave!");
    } else {
        mm.x = mtmp->mx;
        mm.y = mtmp->my;
        if (canseemon(mtmp))
            pline("%s calls forth undead creatures from the grave!",
                  Monnam(mtmp));
    }
    mkundead(&mm, FALSE, NO_MINVENT);
}

/* index 0 is never rolled (rnd(4) below); kept so the film colors
   stay aligned with the elements they announce */
static const char *const vulntext[] = {
    "chartreuse polka-dot",
    "reddish-orange",
    "purplish-blue",
    "coppery-yellow",
    "greenish-mottled"
};

/* coat the target in a film that makes it vulnerable to one element.
   mdef is the hero on the castmu() path and a monster target on the
   castmm() path. The timer writes stay outside every visibility gate;
   only messages are gated. The film color and the element applied
   come from the same roll so they can never disagree */
staticfn void
mcast_vuln_you(struct monst *mtmp, struct monst *mdef)
{
    boolean udefend = (mdef == &gy.youmonst);
    int i = rnd(4);
    long dur;

    if (udefend) {
        pline("A %s film oozes over your %s!",
              Blind ? "slimy" : vulntext[i], body_part(SKIN));
        switch (i) {
        case 1:
            if (mtmp->data == &mons[PM_KATHRYN_THE_ICE_QUEEN]
                || mtmp->data == &mons[PM_ASMODEUS]
                || mtmp->data == &mons[PM_VECNA]) {
                /* the cold-aligned casters curse with cold instead */
                if (Vulnerable_cold)
                    break;
                dur = rnd(100) + 150;
                if (Half_spell_damage)
                    dur = (dur + 1) / 2;
                incr_itimeout(&HVulnerable_cold, dur);
                You_feel("extremely chilly.");
            } else {
                if (Vulnerable_fire)
                    break;
                dur = rnd(100) + 150;
                if (Half_spell_damage)
                    dur = (dur + 1) / 2;
                incr_itimeout(&HVulnerable_fire, dur);
                You_feel("more inflammable.");
            }
            break;
        case 2:
            if (Vulnerable_cold)
                break;
            dur = rnd(100) + 150;
            if (Half_spell_damage)
                dur = (dur + 1) / 2;
            incr_itimeout(&HVulnerable_cold, dur);
            You_feel("extremely chilly.");
            break;
        case 3:
            if (Vulnerable_elec)
                break;
            dur = rnd(100) + 150;
            if (Half_spell_damage)
                dur = (dur + 1) / 2;
            incr_itimeout(&HVulnerable_elec, dur);
            You_feel("overly conductive.");
            break;
        case 4:
            if (Vulnerable_acid)
                break;
            dur = rnd(100) + 150;
            if (Half_spell_damage)
                dur = (dur + 1) / 2;
            incr_itimeout(&HVulnerable_acid, dur);
            You_feel("easily corrodable.");
            break;
        default:
            break;
        }
    } else {
        boolean yours = (mtmp == &gy.youmonst);

        if (!mdef || DEADMONSTER(mdef))
            return;
        if (yours || canseemon(mdef))
            pline("A %s film oozes over %s exterior!",
                  Blind ? "slimy" : vulntext[i], mhis(mdef));
        switch (i) {
        case 1:
            if ((mdef->data->mflags4 & M4_VULNERABLE_FIRE) != 0
                || mdef->vuln_fire)
                break;
            dur = rnd(100) + 150;
            if (mon_arti_has_spfx(mdef, SPFX_HSPDAM))
                dur = (dur + 1) / 2;
            mdef->vuln_fire = (uchar) dur;
            if (yours || canseemon(mdef))
                pline("%s is more inflammable.", Monnam(mdef));
            break;
        case 2:
            if ((mdef->data->mflags4 & M4_VULNERABLE_COLD) != 0
                || mdef->vuln_cold)
                break;
            dur = rnd(100) + 150;
            if (mon_arti_has_spfx(mdef, SPFX_HSPDAM))
                dur = (dur + 1) / 2;
            mdef->vuln_cold = (uchar) dur;
            if (yours || canseemon(mdef))
                pline("%s is extremely chilly.", Monnam(mdef));
            break;
        case 3:
            if ((mdef->data->mflags4 & M4_VULNERABLE_ELEC) != 0
                || mdef->vuln_elec)
                break;
            dur = rnd(100) + 150;
            if (mon_arti_has_spfx(mdef, SPFX_HSPDAM))
                dur = (dur + 1) / 2;
            mdef->vuln_elec = (uchar) dur;
            if (yours || canseemon(mdef))
                pline("%s is overly conductive.", Monnam(mdef));
            break;
        case 4:
            if ((mdef->data->mflags4 & M4_VULNERABLE_ACID) != 0
                || mdef->vuln_acid)
                break;
            dur = rnd(100) + 150;
            if (mon_arti_has_spfx(mdef, SPFX_HSPDAM))
                dur = (dur + 1) / 2;
            mdef->vuln_acid = (uchar) dur;
            if (yours || canseemon(mdef))
                pline("%s is easily corrodable.", Monnam(mdef));
            break;
        default:
            break;
        }
    }
}

/* summon an aligned servant to the caster's aid. mdef is the hero on
   the castmu() path; monster-vs-monster casts are suppressed both
   here and in the useless filter because summon_minion() only places
   the summons next to the hero. Hero casts are rejected by the
   useless filter (no free pets); the leg is kept for safety */
staticfn void
mcast_summon_minion(struct monst *mtmp, struct monst *mdef)
{
    boolean udefend = (mdef == &gy.youmonst);
    struct monst *minion;
    aligntyp aligntype;

    if (udefend) {
        if (mtmp->data == &mons[PM_KATHRYN_THE_ICE_QUEEN]) {
            coord bypos;

            /* the Ice Queen conjures one of her own guardians */
            if (!enexto(&bypos, mtmp->mx, mtmp->my, &mons[PM_SNOW_GOLEM]))
                return;
            minion = makemon(&mons[PM_SNOW_GOLEM], bypos.x, bypos.y,
                             MM_ANGRY);
            if (minion && canspotmon(minion))
                pline("A minion of %s appears!", mon_nam(mtmp));
        } else {
            aligntype = mon_aligntyp(mtmp);
            minion = summon_minion(aligntype, FALSE);
            if (minion) {
                boolean vassal = (aligntype == A_NONE);

                set_malign(minion);
                if (canspotmon(minion))
                    pline("A %s of %s appears!",
                          vassal ? "vassal" : "servant",
                          vassal ? "Moloch" : aligns[1 - aligntype].noun);
            }
        }
    } else {
        boolean yours = (mtmp == &gy.youmonst);

        if (!mdef || DEADMONSTER(mdef))
            return;
        /* monster-vs-monster is suppressed; summon_minion() only
           supports placement next to the hero */
        if (!yours)
            return;
        aligntype = u.ualign.type;
        minion = summon_minion(aligntype, FALSE);
        if (minion && canspotmon(minion))
            pline("A %s of %s appears!",
                  aligntype == A_NONE ? "vassal" : "servant",
                  aligntype == A_NONE ? "Moloch"
                                      : aligns[1 - aligntype].noun);
    }
}

staticfn int
mcast_psi_bolt(struct monst *mtmp, struct monst *mdef, int dmg)
{
    boolean udefend = (mdef == &gy.youmonst);

    if (udefend) {
        /* prior to 3.4.0 Antimagic was setting the damage to 1--this
           made the spell virtually harmless to players with magic res. */
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
            monstseesu(M_SEEN_MAGR);
            dmg = (dmg + 1) / 2;
        } else {
            monstunseesu(M_SEEN_MAGR);
        }
        if (dmg <= 5)
            You("get a slight %sache.", body_part(HEAD));
        else if (dmg <= 10)
            Your("brain is on fire!");
        else if (dmg <= 20)
            Your("%s suddenly aches painfully!", body_part(HEAD));
        else
            Your("%s suddenly aches very painfully!", body_part(HEAD));
        return dmg;
    } else {
        boolean yours = (mtmp == &gy.youmonst);

        if (!mdef || DEADMONSTER(mdef))
            return 0;
        if (resist(mdef, 0, 0, FALSE)) {
            shieldeff(mdef->mx, mdef->my);
            dmg = (dmg + 1) / 2;
        }
        /* monster equivalent of the pre-halving Half_spell_damage
           gives the hero for spell damage */
        if (mon_arti_has_spfx(mdef, SPFX_HSPDAM))
            dmg = (dmg + 1) / 2;
        /* jello-like creatures flollop rather than wince */
        if (yours || canseemon(mdef))
            pline("%s %s%s", Monnam(mdef),
                  (mdef->data->mlet == S_BLOB
                   || mdef->data->mlet == S_JELLY
                   || mdef->data->mlet == S_PUDDING) ? "flollops"
                                                     : "winces",
                  (dmg <= 5) ? "." : "!");
        return dmg;
    }
}

staticfn int
mcast_open_wounds(struct monst *mtmp, struct monst *mdef, int dmg)
{
    boolean udefend = (mdef == &gy.youmonst);

    if (udefend) {
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
            monstseesu(M_SEEN_MAGR);
            dmg = (dmg + 1) / 2;
        } else {
            monstunseesu(M_SEEN_MAGR);
        }
        if (dmg <= 5)
            Your("skin itches badly for a moment.");
        else if (dmg <= 10)
            pline("Wounds appear on your body!");
        else if (dmg <= 20)
            pline("Severe wounds appear on your body!");
        else
            Your("body is covered with painful wounds!");
        return dmg;
    } else {
        boolean yours = (mtmp == &gy.youmonst);
        int ml = yours ? mons[u.umonnum].mlevel
                       : min((int) mtmp->m_lev, 50);

        if (!mdef || DEADMONSTER(mdef))
            return 0;
        /* rolled from the caster's level, not the incoming dmg */
        dmg = d((ml / 2) + 1, 6);
        if (resist(mdef, 0, 0, FALSE)) {
            shieldeff(mdef->mx, mdef->my);
            dmg = (dmg + 1) / 2;
        }
        if (yours || canseemon(mdef)) {
            if (dmg <= 5)
                pline("%s looks itchy!", Monnam(mdef));
            else if (dmg <= 10)
                pline("Wounds appear on %s!", mon_nam(mdef));
            else if (dmg <= 20)
                pline("Severe wounds appear on %s!", mon_nam(mdef));
            else
                pline("%s is covered in wounds!", Monnam(mdef));
        }
        return dmg;
    }
}

staticfn void
mcast_insects(struct monst *mtmp, struct monst *mdef)
{
    /* Try for insects, and if there are none
       left, go for (sticks to) snakes. -3.
       Drow and Lolth summon spiders instead */
    boolean udefend = (mdef == &gy.youmonst);
    boolean spiders = udefend && (mtmp->data == &mons[PM_LOLTH]
                                  || racial_drow(mtmp));
    struct permonst *pm = mkclass(spiders ? S_SPIDER : S_ANT, 0);
    struct monst *mtmp2 = (struct monst *) 0;
    char whatbuf[QBUFSZ], let = (pm ? (spiders ? S_SPIDER : S_ANT) : S_SNAKE);
    boolean success = FALSE, seecaster;
    int i, quan, oldseen, newseen;
    coord bypos;
    const char *fmt, *what;

    if (!udefend) {
        boolean yours = (mtmp == &gy.youmonst);
        int ml = yours ? mons[u.umonnum].mlevel
                       : min((int) mtmp->m_lev, 50);

        if (!mdef || DEADMONSTER(mdef))
            return;
        quan = (ml < 2) ? 1 : rnd(ml / 2);
        if (quan < 3)
            quan = 3;
        success = pm ? TRUE : FALSE;
        for (i = 0; i <= quan; i++) {
            if ((pm = mkclass(let, 0)) != 0) {
                if (!enexto(&bypos, mdef->mx, mdef->my, pm))
                    break;
                if ((mtmp2 = makemon(pm, bypos.x, bypos.y, MM_NOMSG))
                    != 0) {
                    success = TRUE;
                    mtmp2->msleeping = 0;
                    mtmp2->minsects = 1;
                    if (yours || mtmp->mtame)
                        (void) tamedog(mtmp2, (struct obj *) 0, FALSE);
                    else if (mtmp->mpeaceful)
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
        } else if (canseemon(mtmp)) {
            if (!success)
                pline("%s casts at a clump of sticks, but nothing happens.",
                      Monnam(mtmp));
            else if (let == S_SNAKE)
                pline("%s transforms a clump of sticks into snakes!",
                      Monnam(mtmp));
            else
                pline("%s summons insects!", Monnam(mtmp));
        }
        return;
    }

    oldseen = monster_census(TRUE);
    quan = (mtmp->m_lev < 2) ? 1 : rnd((int) mtmp->m_lev / 2);
    if (quan < 3)
        quan = 3;
    for (i = 0; i <= quan; i++) {
        if (!enexto(&bypos, mtmp->mux, mtmp->muy, mtmp->data))
            return;
        if ((pm = mkclass(let, 0)) != 0
            && (mtmp2 = makemon(pm, bypos.x, bypos.y, MM_ANGRY | MM_NOMSG))
            != 0) {
            success = TRUE;
            mtmp2->msleeping = mtmp2->mpeaceful = mtmp2->mtame = 0;
            mtmp2->minsects = 1;
            set_malign(mtmp2);
        }
    }
    newseen = monster_census(TRUE);

    /* not canspotmon() which includes unseen things sensed via warning */
    seecaster = canseemon(mtmp) || tp_sensemon(mtmp) || Detect_monsters;
    what = (let == S_SNAKE) ? "snakes"
           : (let == S_SPIDER) ? "arachnids"
             : "insects";
    if (Hallucination)
        what = makeplural(bogusmon(whatbuf, (char *) 0));

    fmt = 0;
    if (!seecaster) {
        if (newseen <= oldseen || Unaware) {
            /* unseen caster fails or summons unseen critters,
               or unconscious hero ("You dream that you hear...") */
            You_hear("someone summoning %s.", what);
        } else {
            char *arg;

            if (what != whatbuf)
                what = strcpy(whatbuf, what);
            /* unseen caster summoned seen critter(s) */
            arg = (newseen == oldseen + 1) ? an(makesingular(what))
                                           : whatbuf;
            if (!Deaf) {
                Soundeffect(se_someone_summoning, 100);
                You_hear("someone summoning something, and %s %s.", arg,
                         vtense(arg, "appear"));
            } else {
                pline("%s %s.", upstart(arg), vtense(arg, "appear"));
            }
        }

        /* seen caster, possibly producing unseen--or just one--critters;
           hero is told what the caster is doing and doesn't necessarily
           observe complete accuracy of that caster's results (in other
           words, no need to fuss with visibility or singularization;
           player is told what's happening even if hero is unconscious) */
    } else if (!success) {
        fmt = "%s casts at a clump of sticks, but nothing happens.%s";
        what = "";
    } else if (let == S_SNAKE) {
        fmt = "%s transforms a clump of sticks into %s!";
    } else if (Invis && !mon_prop(mtmp, SEE_INVIS)
               && (mtmp->mux != u.ux || mtmp->muy != u.uy)) {
        fmt = "%s summons %s around a spot near you!";
    } else if (Displaced && (mtmp->mux != u.ux || mtmp->muy != u.uy)) {
        fmt = "%s summons %s around your displaced image!";
    } else {
        fmt = "%s summons %s!";
    }
    if (fmt) {
        DISABLE_WARNING_FORMAT_NONLITERAL;
        pline_mon(mtmp, fmt, Monnam(mtmp), what);
        RESTORE_WARNING_FORMAT_NONLITERAL;
    }
}

staticfn void
mcast_blind_you(struct monst *mtmp, struct monst *mdef)
{
    boolean udefend = (mdef == &gy.youmonst);

    if (udefend) {
        /* note: resists_blnd() doesn't apply here */
        if (!Blinded) {
            int num_eyes = eyecount(gy.youmonst.data);

            pline("Scales cover your %s!", (num_eyes == 1)
                                           ? body_part(EYE)
                                           : makeplural(body_part(EYE)));
            make_blinded(Half_spell_damage ? 100L : 200L, FALSE);
            if (!Blind)
                Your1(vision_clears);
        } else
            impossible("no reason for monster to cast blindness spell?");
    } else {
        boolean yours = (mtmp == &gy.youmonst);

        if (!mdef || DEADMONSTER(mdef))
            return;
        /* unlike the hero, an inherently blind-proof monster target
           shrugs the scales off */
        if (!mdef->mblinded && haseyes(mdef->data)) {
            if (!resists_blnd(mdef)) {
                int num_eyes = eyecount(mdef->data);

                if (yours || canseemon(mdef))
                    pline("Scales cover %s %s!", s_suffix(mon_nam(mdef)),
                          (num_eyes == 1) ? "eye" : "eyes");
                mdef->mblinded = 127;
            }
        }
    }
}

staticfn int
mcast_paralyze(struct monst *mtmp, struct monst *mdef)
{
    boolean udefend = (mdef == &gy.youmonst);
    int dmg = 0;

    if (udefend) {
        if (Antimagic || Free_action) {
            shieldeff(u.ux, u.uy);
            monstseesu(M_SEEN_MAGR);
            if (gm.multi >= 0)
                You("stiffen briefly.");
            dmg = 1; /* to produce nomul(-1), not actual damage */
        } else {
            if (gm.multi >= 0)
                You("are frozen in place!");
            dmg = 4 + (int) mtmp->m_lev;
            if (Half_spell_damage)
                dmg = (dmg + 1) / 2;
            monstunseesu(M_SEEN_MAGR);
        }
        nomul(-dmg);
        gm.multi_reason = "paralyzed by a monster";
        gn.nomovemsg = 0;
        /* the freeze is the whole effect; no damage on top - dmg
           only sized the helplessness for nomul() */
        return 0;
    } else {
        boolean yours = (mtmp == &gy.youmonst);

        if (!mdef || DEADMONSTER(mdef))
            return 0;
        if (resist(mdef, 0, 0, FALSE)) {
            shieldeff(mdef->mx, mdef->my);
            if (yours || canseemon(mdef))
                pline("%s stiffens briefly.", Monnam(mdef));
        } else {
            if (yours || canseemon(mdef))
                pline("%s is frozen in place!", Monnam(mdef));
            dmg = 4 + (int) (yours ? mons[u.umonnum].mlevel
                                   : mtmp->m_lev);
            mdef->mcanmove = 0;
            mdef->mfrozen = dmg;
        }
        /* the freeze is the whole effect; no damage on top */
        return 0;
    }
}

staticfn void
mcast_confuse_you(struct monst *mtmp, struct monst *mdef)
{
    boolean udefend = (mdef == &gy.youmonst);

    if (udefend) {
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
            monstseesu(M_SEEN_MAGR);
            You_feel("momentarily dizzy.");
        } else {
            boolean oldprop = !!Confusion;
            int dmg = (int) mtmp->m_lev;

            if (Half_spell_damage)
                dmg = (dmg + 1) / 2;
            make_confused(HConfusion + dmg, TRUE);
            if (Hallucination)
                You_feel("%s!", oldprop ? "trippier" : "trippy");
            else
                You_feel("%sconfused!", oldprop ? "more " : "");
            monstunseesu(M_SEEN_MAGR);
        }
    } else {
        boolean yours = (mtmp == &gy.youmonst);

        if (!mdef || DEADMONSTER(mdef))
            return;
        if (resist(mdef, 0, 0, FALSE)) {
            shieldeff(mdef->mx, mdef->my);
            if (yours || canseemon(mdef))
                pline("%s seems momentarily dizzy.", Monnam(mdef));
        } else {
            if (yours || canseemon(mdef))
                pline("%s seems %sconfused!", Monnam(mdef),
                      mdef->mconf ? "more " : "");
            mdef->mconf = 1;
        }
    }
}

/*
   If dmg is zero, then the monster is not casting at you.
   If the monster is intentionally not casting at you, we have previously
   called spell_would_be_useless() and spellnum should always be a valid
   undirected spell.
   If you modify either of these, be sure to change is_undirected_spell()
   and spell_would_be_useless().
   mdef is the hero on the castmu() path and either the monster target
   or the hero (for self-buffs, which read only the caster) on the
   castmm() path.
 */
staticfn void
mcast_spell(struct monst *mtmp, struct monst *mdef, int dmg, int spellnum,
            short learned_otyp, int adtyp)
{
    boolean udefend = (mdef == &gy.youmonst);

    if (dmg < 0) {
        impossible("monster cast spell (%d) with negative dmg (%d)?",
                   spellnum, dmg);
        return;
    }
    /* MCAST_LEARNED_SPELL is a meta-spell - the actual spell selection
       and LOS checks happen inside cast_learned_spell(), which has its
       own safety checks for offensive spells; whether it counts as
       undirected depends on whether a useful self-buff exists, which
       can differ from whether ANY learned spell exists, so skip this
       check for learned spells */
    if (dmg == 0 && spellnum != MCAST_LEARNED_SPELL
        && !is_undirected_spell(mtmp, spellnum)) {
        impossible("cast directed wizard spell (%d) with dmg=0?", spellnum);
        return;
    }

    switch (spellnum) {
    case MCAST_DEATH_TOUCH:
        mcast_death_touch(mtmp, mdef);
        dmg = 0;
        break;
    case MCAST_CLONE_WIZ:
        mcast_clone_wiz(mtmp);
        dmg = 0;
        break;
    case MCAST_SUMMON_MONS:
        mcast_summon_mons(mtmp, mdef);
        dmg = 0;
        break;
    case MCAST_AGGRAVATION:
        /* the global wakeup is hero-facing; the useless filter rejects
           monster-target picks, so this is a no-op for one */
        if (udefend) {
            You_feel("that monsters are aware of your presence.");
            aggravate();
        }
        dmg = 0;
        break;
    case MCAST_CURSE_ITEMS:
        if (udefend) {
            You_feel("as if you need some help.");
            rndcurse();
        } else if (mdef && !DEADMONSTER(mdef)) {
            if (mtmp == &gy.youmonst || canseemon(mdef))
                You_feel("as though %s needs some help.", mon_nam(mdef));
            mrndcurse(mdef);
        }
        dmg = 0;
        break;
    case MCAST_DESTRY_ARMR:
        mcast_destroy_armor(mtmp, mdef);
        dmg = 0;
        break;
    case MCAST_WEAKEN_YOU: /* drain strength */
        mcast_weaken_you(mtmp, mdef, dmg);
        dmg = 0;
        break;
    case MCAST_DISAPPEAR: /* makes self invisible */
        mcast_disappear(mtmp);
        dmg = 0;
        break;
    case MCAST_STUN_YOU:
        mcast_stun_you(mtmp, mdef, dmg);
        dmg = 0;
        break;
    case MCAST_HASTE_SELF:
        if (mtmp == &gy.youmonst) {
            if (!(HFast & INTRINSIC)) {
                You("are suddenly moving faster.");
                HFast |= INTRINSIC;
            }
        } else {
            mon_adjust_speed(mtmp, 1, (struct obj *) 0);
        }
        dmg = 0;
        break;
    case MCAST_CURE_SELF:
        dmg = m_cure_self(mtmp, dmg);
        break;
    case MCAST_PSI_BOLT:
        dmg = mcast_psi_bolt(mtmp, mdef, dmg);
        break;
    case MCAST_GEYSER:
        dmg = mcast_geyser(mtmp, mdef, dmg);
        break;
    case MCAST_FIRE_PILLAR:
        dmg = mcast_fire_pillar(mtmp, mdef, dmg);
        break;
    case MCAST_LIGHTNING:
        dmg = mcast_lightning(mtmp, mdef, dmg);
        break;
    case MCAST_ACID_BLAST:
        mcast_acid_blast(mtmp, mdef);
        dmg = 0;
        break;
    case MCAST_FIRE_BOLT:
    case MCAST_ICE_BOLT:
        mcast_bolt(mtmp, mdef, spellnum);
        dmg = 0;
        break;
    case MCAST_VULN_YOU:
        mcast_vuln_you(mtmp, mdef);
        dmg = 0;
        break;
    case MCAST_SUMMON_MINION:
        mcast_summon_minion(mtmp, mdef);
        dmg = 0;
        break;
    case MCAST_INSECTS:
        mcast_insects(mtmp, mdef);
        dmg = 0;
        break;
    case MCAST_BLIND_YOU:
        mcast_blind_you(mtmp, mdef);
        dmg = 0;
        break;
    case MCAST_PARALYZE:
        dmg = mcast_paralyze(mtmp, mdef);
        break;
    case MCAST_CONFUSE_YOU:
        mcast_confuse_you(mtmp, mdef);
        dmg = 0;
        break;
    case MCAST_BARKSKIN:
        cast_barkskin(mtmp, P_SKILL(spell_skilltype(SPE_BARKSKIN)));
        dmg = 0;
        break;
    case MCAST_STONESKIN:
        cast_stoneskin(mtmp, P_SKILL(spell_skilltype(SPE_STONESKIN)));
        dmg = 0;
        break;
    case MCAST_REFLECTION:
        cast_reflection(mtmp, P_SKILL(spell_skilltype(SPE_REFLECTION)));
        dmg = 0;
        break;
    case MCAST_PROTECTION:
        mcast_protection(mtmp);
        dmg = 0;
        break;
    case MCAST_CANCELLATION:
        mcast_cancellation(mtmp, mdef);
        dmg = 0;
        break;
    case MCAST_CALL_UNDEAD:
        mcast_call_undead(mtmp, mdef);
        dmg = 0;
        break;
    case MCAST_OPEN_WOUNDS:
        dmg = mcast_open_wounds(mtmp, mdef, dmg);
        break;
    case MCAST_LEARNED_SPELL:
        dmg = cast_learned_spell(mtmp, mdef, learned_otyp);
        break;
    default:
        impossible("mcastu: invalid magic spell (%d)", spellnum);
        dmg = 0;
        break;
    }

    if (dmg) {
        if (udefend) {
            mdamageu(mtmp, dmg);
        } else {
            boolean yours = (mtmp == &gy.youmonst);

            if (damage_mon(mdef, dmg, adtyp, yours)) {
                if (yours) {
                    killed(mdef);
                } else {
                    if (mtmp->mtame)
                        set_pet_killer(mtmp);
                    monkilled(mdef, "", adtyp);
                }
            }
        }
    }
}

staticfn boolean
is_undirected_spell(struct monst *mtmp, int spellnum)
{
    if (spellnum == MCAST_LEARNED_SPELL)
        /* only undirected if the monster has a useful self-buff
           learned spell to cast through the undirected path */
        return mtmp ? mhas_useful_selfbuff_spell(mtmp) : FALSE;
    if ((mcast_data[spellnum].flags & MCF_INDIRECT) != 0)
        return TRUE;
    return FALSE;
}

/* Some spells are useless under some circumstances. */
staticfn boolean
spell_would_be_useless(struct monst *mtmp, struct monst *mdef, int spellnum)
{
    boolean youdefend = (mdef == &gy.youmonst);
    /* the polymorphed hero casts through castum(); hero-caster
       clauses read the hero's own state - gy.youmonst's monster
       fields (m_lev, mhp, mtame, permspeed, mextrinsics, mx/my)
       are never maintained */
    boolean yours = (mtmp == &gy.youmonst);

    /* Some spells don't require the player to really be there and can be cast
     * by the monster when you're invisible, yet still shouldn't be cast when
     * the monster doesn't even think you're there.
     * This check isn't quite right because it always uses your real position.
     * We really want something like "if the monster could see mux, muy".
     */

    /* can't cast at an invalid monster target */
    if (!youdefend && (!mdef || DEADMONSTER(mdef)))
        return TRUE;

    /* the mpeaceful and could-see-hero pre-checks are hero-relative;
       tame and peaceful casters may still use the innate lists against
       monster targets (per-spell clauses below keep summons and other
       hero-facing effects off that path) */

    /* spell is only cast by hostile monsters */
    if (youdefend && (mcast_data[spellnum].flags & MCF_HOSTILE) != 0) {
        if (mtmp->mpeaceful)
            return TRUE;
    }

    /* spell needs the monster to see hero */
    if (youdefend && (mcast_data[spellnum].flags & MCF_SIGHT) != 0) {
        boolean mcouldseeu = couldsee(mtmp->mx, mtmp->my);

        if (!mcouldseeu)
            return TRUE;
    }

    switch (spellnum) {
    case MCAST_LEARNED_SPELL:
        /* peaceful casters (or ones that cannot see the hero, or when
           the hero is invulnerable) may only use self-buffs on the
           at-hero path */
        if (youdefend
            && (mtmp->mpeaceful || !couldsee(mtmp->mx, mtmp->my)
                || u.uinvulnerable)
            && !mhas_useful_selfbuff_spell(mtmp))
            return TRUE;
        /* reject when the entire offensive repertoire would resist out
           against the target and no useful self-buff is queued, letting
           the re-roll loop pick a different spell */
        if (!mhas_useful_selfbuff_spell(mtmp)
            && !mhas_castable_offensive_spell(mtmp, mdef))
            return TRUE;
        break;
    case MCAST_DEATH_TOUCH:
        if (youdefend) {
            /* magic resistance only blunts the touch, so keep casting
               until the hero has been seen to shrug it off entirely */
            if (m_seenres(mtmp, M_SEEN_DEATH))
                return TRUE;
        } else {
            /* skip targets death magic cannot kill */
            if (immune_death_magic(mdef->data) || is_vampshifter(mdef))
                return TRUE;
        }
        break;
    case MCAST_GEYSER:
        if (!rn2(5))
            return TRUE;
        break;
    case MCAST_ACID_BLAST:
        if (youdefend) {
            /* don't waste the cast on a hero proven acid resistant or
               made invulnerable by prayer, and usually avoid catching
               itself in a blast centered next to its own position */
            if (m_seenres(mtmp, M_SEEN_ACID) || u.uinvulnerable
                || (distu(mtmp->mx, mtmp->my) < 3 && rn2(5)))
                return TRUE;
        } else if (!yours) {
            /* a caster that is not itself acid resistant usually
               refuses a blast within its own radius; the hero is
               free to splash an adjacent blast onto both of them */
            if (!(resists_acid(mtmp) || defended(mtmp, AD_ACID))
                && dist2(mtmp->mx, mtmp->my, mdef->mx, mdef->my) < 3
                && rn2(5))
                return TRUE;
            /* tame and peaceful casters never splash the hero */
            if ((mtmp->mtame || mtmp->mpeaceful)
                && distu(mdef->mx, mdef->my) < 3)
                return TRUE;
        }
        break;
    case MCAST_FIRE_BOLT:
        if (youdefend) {
            /* as acid blast: skip a proven-resistant or invulnerable
               hero, and usually avoid a blast beside its own position */
            if (m_seenres(mtmp, M_SEEN_FIRE) || u.uinvulnerable
                || (distu(mtmp->mx, mtmp->my) < 3 && rn2(5)))
                return TRUE;
        } else if (!yours) {
            if (!(resists_fire(mtmp) || defended(mtmp, AD_FIRE))
                && dist2(mtmp->mx, mtmp->my, mdef->mx, mdef->my) < 3
                && rn2(5))
                return TRUE;
            if ((mtmp->mtame || mtmp->mpeaceful)
                && distu(mdef->mx, mdef->my) < 3)
                return TRUE;
        }
        break;
    case MCAST_ICE_BOLT:
        if (youdefend) {
            if (m_seenres(mtmp, M_SEEN_COLD) || u.uinvulnerable
                || (distu(mtmp->mx, mtmp->my) < 3 && rn2(5)))
                return TRUE;
        } else if (!yours) {
            if (!(resists_cold(mtmp) || defended(mtmp, AD_COLD))
                && dist2(mtmp->mx, mtmp->my, mdef->mx, mdef->my) < 3
                && rn2(5))
                return TRUE;
            if ((mtmp->mtame || mtmp->mpeaceful)
                && distu(mdef->mx, mdef->my) < 3)
                return TRUE;
        }
        break;
    case MCAST_VULN_YOU:
        /* prayer invulnerability protects against the debuff */
        if (youdefend && u.uinvulnerable)
            return TRUE;
        break;
    case MCAST_SUMMON_MINION:
        /* summon_minion() only places the summons next to the hero,
           so monster-target casts are rejected outright */
        if (!youdefend)
            return TRUE;
        /* melee-range spell; tame casters never summon reinforcements
           against their own side, and prayer invulnerability holds
           the summons off */
        if (distmin(mtmp->mx, mtmp->my, u.ux, u.uy) > 1)
            return TRUE;
        if (mtmp->mtame || u.uinvulnerable)
            return TRUE;
        break;
    case MCAST_SUMMON_MONS:
        /* tame and peaceful casters never summon reinforcements
           against the hero's side, and the hero gets no free pets */
        if (yours || mtmp->mtame || mtmp->mpeaceful)
            return TRUE;
        break;
    case MCAST_INSECTS:
        /* prayer invulnerability holds the swarm off */
        if (youdefend && u.uinvulnerable)
            return TRUE;
        /* as summon monsters: no reinforcements from the hero's own
           side, and no free pets */
        if (yours || mtmp->mtame || mtmp->mpeaceful)
            return TRUE;
        break;
    case MCAST_OPEN_WOUNDS:
        /* prayer invulnerability protects against the wounds */
        if (youdefend && u.uinvulnerable)
            return TRUE;
        break;
    case MCAST_CLONE_WIZ:
        /* only the Wizard is allowed to clone himself */
        if (yours || !mtmp->iswiz || svc.context.no_of_wizards > 1)
            return TRUE;
        if (mtmp->mtame || mtmp->mpeaceful)
            return TRUE;
        break;
    case MCAST_AGGRAVATION:
        /* the global wakeup is hero-facing screen work; it must never
           fire from a third-party fight */
        if (!youdefend)
            return TRUE;
        /* aggravation (global wakeup) when everyone is already active */
        /* if nothing needs to be awakened then this spell is useless
           but caster might not realize that [chance to pick it then
           must be very small otherwise caller's many retry attempts
           will eventually end up picking it too often] */
        if (!has_aggravatables(mtmp))
            return rn2(100) ? TRUE : FALSE;
        break;
    case MCAST_HASTE_SELF:
        /* haste self when already fast */
        if (yours ? (Fast != 0) : (mtmp->permspeed == MFAST))
            return TRUE;
        break;
    case MCAST_DISAPPEAR:
        /* invisibility when already invisible */
        if (yours ? ((HInvis & INTRINSIC) != 0)
                  : (mtmp->minvis || mtmp->invis_blkd))
            return TRUE;
        /* peaceful monster won't cast invisibility if you can't see
           invisible,
           same as when monsters drink potions of invisibility.  This doesn't
           really make a lot of sense, but lets the player avoid hitting
           peaceful monsters by mistake */
        if (!yours && mtmp->mpeaceful && !See_invisible)
            return TRUE;
        break;
    case MCAST_CURE_SELF:
        /* healing when already healed; the hero holds the cure until
           significantly hurt */
        if (yours ? (u.mh * 4) > u.mhmax : (mtmp->mhp == mtmp->mhpmax))
            return TRUE;
        break;
    case MCAST_BARKSKIN:
    case MCAST_STONESKIN:
        if (yours) {
            /* the hero's wards are property-based; the two are
               mutually exclusive and neither stacks with the haze */
            if (u.uspellprot)
                return TRUE;
            if (Barkskin && spellnum == MCAST_STONESKIN)
                return TRUE;
            if (Stoneskin && spellnum == MCAST_BARKSKIN)
                return TRUE;
            break;
        }
        /* the two skin wards are mutually exclusive, and neither
           stacks with the protection haze */
        if (has_barkskin(mtmp) || has_stoneskin(mtmp))
            return TRUE;
        if (mtmp->mprotection)
            return TRUE;
        break;
    case MCAST_PROTECTION:
        /* the skin wards supersede the haze */
        if (yours ? (Barkskin || Stoneskin)
                  : (has_barkskin(mtmp) || has_stoneskin(mtmp)))
            return TRUE;
        break;
    case MCAST_REFLECTION:
        /* reflection when already reflecting, whether from the spell
           or from gear; the hero may re-cast to extend the timeout */
        if (!yours
            && (has_reflection(mtmp) || mon_reflects(mtmp, (char *) 0)))
            return TRUE;
        break;
    case MCAST_CANCELLATION:
        if (youdefend) {
            /* prayer invulnerability protects */
            if (u.uinvulnerable)
                return TRUE;
        } else {
            /* no point re-cancelling a cancelled target; the hero
               may still want the buff strips and the item pass */
            if (!yours && mdef->mcan)
                return TRUE;
            /* tame and peaceful casters never risk the hero this
               close to the fight */
            if ((mtmp->mtame || mtmp->mpeaceful)
                && distu(mdef->mx, mdef->my) < 3)
                return TRUE;
        }
        break;
    case MCAST_CALL_UNDEAD:
        /* no free pets for the hero, whatever the form */
        if (yours)
            return TRUE;
        /* only undead or demonic casters, evil priests, and quest
           nemeses have this in their repertoire */
        if (!is_undead(mtmp->data) && !is_demon(mtmp->data)
            && !(mtmp->ispriest && mon_aligntyp(mtmp) < A_NEUTRAL)
            && mtmp->data->msound != MS_NEMESIS)
            return TRUE;
        /* no reinforcements from the hero's own side */
        if (mtmp->mtame || mtmp->mpeaceful)
            return TRUE;
        /* melee-range spell; prayer invulnerability holds the
           summons off */
        if (youdefend) {
            if (distmin(mtmp->mx, mtmp->my, u.ux, u.uy) > 1)
                return TRUE;
            if (u.uinvulnerable)
                return TRUE;
        } else {
            if (distmin(mtmp->mx, mtmp->my, mdef->mx, mdef->my) > 1)
                return TRUE;
        }
        break;
    case MCAST_BLIND_YOU:
        if (youdefend) {
            if (Blinded)
                return TRUE;
        } else {
            if (!haseyes(mdef->data) || mdef->mblinded)
                return TRUE;
        }
        break;
    case MCAST_DESTRY_ARMR:
        /* melee-range spell; also pointless when the target wears no
           armor */
        if (youdefend) {
            if (distmin(mtmp->mx, mtmp->my, u.ux, u.uy) > 1)
                return TRUE;
            if (!wearing_armor())
                return TRUE;
        } else {
            if (distmin(yours ? u.ux : mtmp->mx, yours ? u.uy : mtmp->my,
                        mdef->mx, mdef->my) > 1)
                return TRUE;
            if (!(mdef->misc_worn_check & W_ARMOR))
                return TRUE;
        }
        break;
    default:
        break;
    }
    return FALSE;
}

/* monster uses spell (ranged) */
int
buzzmu(struct monst *mtmp, struct attack *mattk)
{
    /* don't print constant stream of curse messages for 'normal'
       spellcasting monsters at range */
    if (!BZ_VALID_ADTYP(mattk->adtyp))
        return M_ATTK_MISS;

    /* like castmu(), no check of what the hero was seen to resist */
    if (mtmp->mcan) {
        cursetxt(mtmp, FALSE);
        return M_ATTK_MISS;
    }
    if (lined_up(mtmp) && rn2(3)) {
        nomul(0);
        if (canseemon(mtmp))
            pline_mon(mtmp, "%s zaps you with a %s!", Monnam(mtmp),
                  flash_str(BZ_OFS_AD(mattk->adtyp), FALSE));
        gb.buzzer = mtmp;
        buzz(BZ_M_SPELL(BZ_OFS_AD(mattk->adtyp)), (int) mattk->damn,
             mtmp->mx, mtmp->my, sgn(gt.tbx), sgn(gt.tby));
        gb.buzzer = 0;
        return M_ATTK_HIT;
    }
    return M_ATTK_MISS;
}

/* monster uses spell against another monster (ranged) */
int
buzzmm(struct monst *mtmp, struct monst *mdef, struct attack *mattk)
{
    /* don't print constant stream of curse messages for 'normal'
       spellcasting monsters at range */
    if (!BZ_VALID_ADTYP(mattk->adtyp))
        return M_ATTK_MISS;

    if (mtmp->mcan) {
        cursetxt(mtmp, FALSE);
        return M_ATTK_MISS;
    }
    if (m_lined_up(mdef, mtmp, FALSE) && rn2(3)) {
        nomul(0);
        if (canseemon(mtmp))
            pline_mon(mtmp, "%s zaps %s with a %s!", Monnam(mtmp),
                  mon_nam(mdef), flash_str(BZ_OFS_AD(mattk->adtyp), FALSE));
        /* crossfire can hit the hero, so set the attribution bracket
           exactly like the other monster beam producers */
        gb.buzzer = mtmp;
        dobuzz(BZ_M_SPELL(BZ_OFS_AD(mattk->adtyp)), (int) mattk->damn,
               mtmp->mx, mtmp->my, sgn(gt.tbx), sgn(gt.tby),
               FALSE, FALSE, FALSE);
        gb.buzzer = 0;
        return M_ATTK_HIT;
    }
    return M_ATTK_MISS;
}

/* TRUE if the monster has a wizard or clerical spell attack */
boolean
is_spellcaster(struct monst *mtmp)
{
    int i;
    struct attack *mattk = has_erac(mtmp) ? ERAC(mtmp)->mattk
                                          : mtmp->data->mattk;

    for (i = 0; i < NATTK; i++) {
        if (mattk[i].aatyp == AT_MAGC
            && (mattk[i].adtyp == AD_SPEL || mattk[i].adtyp == AD_CLRC))
            return TRUE;
    }
    return FALSE;
}

/* returns a random adjacent tame monster, or NULL; used by
   MCAST_UNTAME_PET to find targets for the charm monster spell */
staticfn struct monst *
find_adjacent_pet(struct monst *mtmp)
{
    struct monst *candidates[8];
    int count = 0;
    int dx, dy;

    for (dx = -1; dx <= 1; dx++) {
        for (dy = -1; dy <= 1; dy++) {
            struct monst *m;

            if (dx == 0 && dy == 0)
                continue;
            if (!isok(mtmp->mx + dx, mtmp->my + dy))
                continue;
            m = m_at(mtmp->mx + dx, mtmp->my + dy);
            if (m && m->mtame)
                candidates[count++] = m;
        }
    }
    if (count == 0)
        return (struct monst *) 0;
    return candidates[rn2(count)];
}

/* untame a pet, used by MCAST_UNTAME_PET; if target is provided and
   tame, untame it; otherwise find a random adjacent pet */
staticfn void
do_untame_pet(struct monst *caster, struct monst *target)
{
    struct monst *pet;

    /* use specific target if it is tame, otherwise find a random
       adjacent pet */
    if (target && target->mtame)
        pet = target;
    else
        pet = find_adjacent_pet(caster);

    if (pet) {
        /* untame the pet - free_edog also sets mtame = 0 */
        if (has_edog(pet))
            free_edog(pet);
        else
            pet->mtame = 0;
        pet->mpeaceful = 0;
        if (pet->mleashed)
            m_unleash(pet, TRUE);
        set_malign(pet);
        if (canseemon(pet))
            pline_mon(pet, "%s turns against you!", Monnam(pet));
        newsym(pet->mx, pet->my);
    }
}

/* monster casts a spell at another monster; the learned-spell
   repertoire, the charm-monster untame special, the innate elemental
   hand-to-hand casts (hellfire and kin), and the innate
   AD_SPEL/AD_CLRC spell lists are all implemented here; true
   self-buffs route through the hero-target dispatch path since they
   read only the caster */
int
castmm(struct monst *mtmp, struct monst *mdef, struct attack *mattk)
{
    int dmg, ml = min((int) mtmp->m_lev, 50);
    int ret, spellnum = -1, spell_count;
    short learned_otyp = 0;
    boolean undirected;
    boolean seecaster = (canseemon(mtmp) || tp_sensemon(mtmp)
                         || Detect_monsters);
    boolean elemental = (mattk->adtyp == AD_FIRE
                         || mattk->adtyp == AD_COLD
                         || mattk->adtyp == AD_ACID
                         || mattk->adtyp == AD_MAGM);

    /* guard against casting another spell attack at an already dead
       monster; some monsters have multiple AT_MAGC attacks */
    if (DEADMONSTER(mdef))
        return M_ATTK_MISS;

    if ((!elemental && mattk->adtyp != AD_SPEL && mattk->adtyp != AD_CLRC)
        || !ml)
        return M_ATTK_MISS;

    if (!elemental) {
        /* spells learned from spellbooks; the chance scales with
           repertoire size like choose_monster_spell() */
        spell_count = mcount_castable_spells(mtmp);
        if (spell_count > 0) {
            int chance = 15 + spell_count * 5;

            if (chance > 40)
                chance = 40;
            if (rn2(100) < chance
                && mchoose_learned_spell(mtmp, (struct monst *) 0, 0, 0,
                                         FALSE) != 0)
                spellnum = MCAST_LEARNED_SPELL;
        }

        /* if attacking a tame monster and we know charm monster, use
           the untame spell instead */
        if (mdef->mtame && mknows_spell(mtmp, SPE_CHARM_MONSTER))
            spellnum = MCAST_UNTAME_PET;

        /* nothing pre-empted; select from the innate spell lists with
           the same retry discipline castmu() uses */
        if (spellnum < 0) {
            int cnt = 40;

            do {
                spellnum = choose_monster_spell(mtmp, mdef, mattk->adtyp);
            } while (--cnt > 0
                     && spell_would_be_useless(mtmp, mdef, spellnum));
            if (cnt == 0)
                return M_ATTK_MISS;
        }

        /* reject the learned meta when the entire offensive repertoire
           would resist out against this target and no useful self-buff
           is queued; don't waste the attack */
        if (spellnum == MCAST_LEARNED_SPELL
            && !mhas_useful_selfbuff_spell(mtmp)
            && !mhas_castable_offensive_spell(mtmp, mdef))
            return M_ATTK_MISS;
    }

    /* monster unable to cast spells? */
    if (mtmp->mcan || mtmp->mspec_used) {
        if (canseemon(mtmp)) {
            if (!elemental && is_undirected_spell(mtmp, spellnum))
                pline_mon(mtmp, "%s points all around, then curses.",
                          Monnam(mtmp));
            else
                pline_mon(mtmp, "%s points at %s, then curses.",
                          Monnam(mtmp), mon_nam(mdef));
        } else if ((!(svm.moves % 4) || !rn2(4))) {
            if (!Deaf)
                Norep("You hear a mumbled curse.");
        }
        return M_ATTK_MISS;
    }

    /* casting cost, same formula as castmu(); innate elemental
       attacks are gated by mspec but never charge it */
    if (mattk->adtyp == AD_SPEL || mattk->adtyp == AD_CLRC) {
        mtmp->mspec_used = 4 - (int) mtmp->m_lev;
        if (mtmp->mspec_used < 2)
            mtmp->mspec_used = 2;
        /* many boss-type monsters have two or more spell attacks per
           turn and would never fire the later ones with mspec above
           0; top tier bosses always reset it, mid tier bosses reset
           it two thirds of the time */
        if (is_dprince(mtmp->data) || mtmp->iswiz || mtmp->isvecna
            || mtmp->istalgath
            || mtmp->data == &mons[PM_KATHRYN_THE_ENCHANTRESS])
            mtmp->mspec_used = 0;
        else if (rn2(3)
                 && (is_dlord(mtmp->data)
                     || mtmp->data->msound == MS_LEADER
                     || mtmp->data->msound == MS_NEMESIS
                     || mtmp->data == &mons[PM_ORACLE]
                     || mtmp->data == &mons[PM_HIGH_PRIEST]
                     || mtmp->data == &mons[PM_KATHRYN_THE_ICE_QUEEN]))
            mtmp->mspec_used = 0;
        /* any SPFX_EREGEN artifact (e.g. Eye of the Aethiopica) drops
           mspec to 0, granting effectively unlimited spellcasting */
        if (mon_has_eregen_artifact(mtmp))
            mtmp->mspec_used = 0;
    }

    if (rn2(ml * 10) < (mtmp->mconf ? 100 : 20)) { /* fumbled attack */
        Soundeffect(se_air_crackles, 60);
        if (canseemon(mtmp) && !Deaf) {
            set_msg_xy(mtmp->mx, mtmp->my);
            pline_The("air crackles around %s.", mon_nam(mtmp));
        }
        return M_ATTK_MISS;
    }

    /* resolve the learned-spell meta now so the casting message
       reflects the spell actually cast (self-buff vs directed at
       mdef); the same spell is reused below so message and effect
       cannot diverge */
    if (spellnum == MCAST_LEARNED_SPELL)
        learned_otyp = mpick_learned_spell(mtmp, mdef);
    undirected = elemental ? FALSE : is_undirected_spell(mtmp, spellnum);
    if (learned_otyp)
        undirected = IS_SELFBUFF_SPELL(learned_otyp);

    if (seecaster && canseemon(mdef) && !undirected)
        pline_mon(mtmp, "%s casts a spell at %s!",
                  Monnam(mtmp), mon_nam(mdef));
    else if (seecaster && undirected)
        pline_mon(mtmp, "%s casts a spell!", Monnam(mtmp));

    if (elemental) {
        /* as these are spells, the damage is related to the level
           of the monster casting the spell */
        if (mattk->damd)
            dmg = d((int) ((ml / 2) + mattk->damn), (int) mattk->damd);
        else
            dmg = d((int) ((ml / 2) + 1), 6);

        switch (mattk->adtyp) {
        case AD_FIRE:
            if (mon_underwater(mdef)) {
                if (canseemon(mdef))
                    pline_The("flames are quenched by the water around %s.",
                              mon_nam(mdef));
                dmg = 0;
                break;
            }
            if (canseemon(mdef)) {
                if (is_demon(mtmp->data))
                    pline("%s is enveloped in hellfire!", Monnam(mdef));
                else
                    pline("%s is enveloped in flames.", Monnam(mdef));
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
            if (mon_underwater(mdef)) {
                if (canseemon(mdef))
                    pline_The("acid dissipates harmlessly in the water around %s.",
                              mon_nam(mdef));
                dmg = 0;
                break;
            }
            if (canseemon(mdef))
                pline("%s is covered in acid.", Monnam(mdef));
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
        }

        if (dmg) {
            if (damage_mon(mdef, dmg, (int) mattk->adtyp, FALSE)) {
                if (mtmp->mtame)
                    set_pet_killer(mtmp);
                monkilled(mdef, "", (int) mattk->adtyp);
            }
        }
        return M_ATTK_HIT;
    }

    /* as these are spells, the damage is related to the level of the
       monster casting the spell; the hero's Half_spell_damage is a
       hero-defense reduction and does not apply here */
    if (mattk->damd)
        dmg = d((int) ((ml / 2) + mattk->damn), (int) mattk->damd);
    else
        dmg = d((int) ((ml / 2) + 1), 6);

    ret = M_ATTK_HIT;

    if (spellnum == MCAST_UNTAME_PET) {
        do_untame_pet(mtmp, mdef);
    } else if (spellnum == MCAST_LEARNED_SPELL) {
        /* route self-buffs to the caster and offensive spells at
           mdef, reusing the pre-selected spell */
        struct monst *ltgt = (learned_otyp
                              && IS_SELFBUFF_SPELL(learned_otyp)) ? mtmp
                                                                  : mdef;

        (void) cast_learned_spell(mtmp, ltgt, learned_otyp);
    } else {
        /* several spells are flagged indirect yet still act at the
           target; route those and every directed spell at mdef, and
           the true self-buffs (which read only the caster) through
           the hero-target dispatch path */
        struct monst *starget = mdef;

        if (is_undirected_spell(mtmp, spellnum)
            && spellnum != MCAST_AGGRAVATION
            && spellnum != MCAST_SUMMON_MONS
            && spellnum != MCAST_ACID_BLAST
            && spellnum != MCAST_FIRE_BOLT
            && spellnum != MCAST_ICE_BOLT
            && spellnum != MCAST_INSECTS
            && spellnum != MCAST_SUMMON_MINION
            && spellnum != MCAST_CALL_UNDEAD
            && spellnum != MCAST_VULN_YOU
            && spellnum != MCAST_OPEN_WOUNDS)
            starget = &gy.youmonst;
        mcast_spell(mtmp, starget, dmg, spellnum, 0, (int) mattk->adtyp);
    }
    /* an area spell centered on an adjacent target can catch the
       caster in its own blast; fold, never overwrite - the mhitm
       caller checks both sides as well */
    if (DEADMONSTER(mtmp))
        ret |= M_ATTK_AGR_DIED;
    return ret;
}

/* the hero, polymorphed into a form with an AT_MAGC attack, casts at
   an adjacent monster: the elemental attack types inline, the innate
   wizard and clerical spell lists through the shared dispatcher */
int
castum(struct monst *mtmp, struct attack *mattk)
{
    int dmg, ml = mons[u.umonnum].mlevel;
    int spellnum = 0;
    boolean directed = TRUE;
    boolean lists = (mattk->adtyp == AD_SPEL || mattk->adtyp == AD_CLRC);

    /* guard against casting another spell attack at an already dead
       monster; some monsters have multiple AT_MAGC attacks */
    if (DEADMONSTER(mtmp))
        return M_ATTK_MISS;

    if ((mattk->adtyp != AD_FIRE && mattk->adtyp != AD_COLD
         && mattk->adtyp != AD_ACID && mattk->adtyp != AD_MAGM && !lists)
        || !ml)
        return M_ATTK_MISS;

    if (lists) {
        int cnt = 40;

        do {
            spellnum = choose_monster_spell(&gy.youmonst, mtmp,
                                            (int) mattk->adtyp);
        } while (--cnt > 0
                 && spell_would_be_useless(&gy.youmonst, mtmp, spellnum));
        if (cnt == 0) {
            You("have no spells to cast right now!");
            return M_ATTK_MISS;
        }
        directed = !is_undirected_spell(&gy.youmonst, spellnum);
    }

    /* unable to cast spells? */
    if (u.uen < ml) {
        if (directed)
            You("point at %s, then curse.", mon_nam(mtmp));
        else
            You("point all around, then curse.");
        return M_ATTK_MISS;
    }
    /* innate elemental casts drain no energy; the wizard and clerical
       spell lists charge power, spent even on a fumble */
    if (lists) {
        u.uen -= ml;
        disp.botl = TRUE;
    }

    if (rn2(ml * 10) < (Confusion ? 100 : 20)) { /* fumbled attack */
        pline_The("air crackles around you.");
        return M_ATTK_MISS;
    }

    You("cast a spell%s%s!", directed ? " at " : "",
        directed ? mon_nam(mtmp) : "");

    /* as these are spells, the damage is related to the level
       of the form casting the spell */
    if (mattk->damd)
        dmg = d((int) ((ml / 2) + mattk->damn), (int) mattk->damd);
    else
        dmg = d((int) ((ml / 2) + 1), 6);

    switch (mattk->adtyp) {
    case AD_FIRE:
        if (mon_underwater(mtmp)) {
            if (canseemon(mtmp))
                pline_The("flames are quenched by the water around %s.",
                          mon_nam(mtmp));
            dmg = 0;
            break;
        }
        if (canseemon(mtmp)) {
            if (is_demon(gy.youmonst.data))
                pline("%s is enveloped in hellfire!", Monnam(mtmp));
            else
                pline("%s is enveloped in flames.", Monnam(mtmp));
        }
        if (resists_fire(mtmp) || defended(mtmp, AD_FIRE)) {
            shieldeff(mtmp->mx, mtmp->my);
            if (is_demon(gy.youmonst.data)) {
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
        if (mon_underwater(mtmp)) {
            if (canseemon(mtmp))
                pline_The("acid dissipates harmlessly in the water around %s.",
                          mon_nam(mtmp));
            dmg = 0;
            break;
        }
        if (canseemon(mtmp))
            pline("%s is covered in acid.", Monnam(mtmp));
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
        dmg = d((int) ml / 2 + 1, 6);
        if (resists_magm(mtmp) || defended(mtmp, AD_MAGM)) {
            shieldeff(mtmp->mx, mtmp->my);
            if (canseemon(mtmp))
                pline("Some missiles bounce off!");
            dmg = (dmg + 1) / 2;
        }
        break;
    case AD_SPEL:
    case AD_CLRC:
        mcast_spell(&gy.youmonst, mtmp, dmg, spellnum, 0,
                    (int) mattk->adtyp);
        dmg = 0; /* done by the spell casting functions */
        break;
    }

    if (dmg) {
        if (mon_arti_has_spfx(mtmp, SPFX_HSPDAM))
            dmg = (dmg + 1) / 2;
        if (damage_mon(mtmp, dmg, (int) mattk->adtyp, TRUE))
            killed(mtmp);
    }
    return M_ATTK_HIT;
}

/* check if monster knows a specific spell */
boolean
mknows_spell(struct monst *mtmp, int otyp)
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

/* monster learns spell from spellbook; returns TRUE if learned */
boolean
mlearn_spell(struct monst *mtmp, struct obj *book)
{
    int i;

    if (!is_spellcaster(mtmp))
        return FALSE;
    if (mknows_spell(mtmp, book->otyp))
        return FALSE; /* already knows it */
    if (book->spestudied >= MAX_SPELL_STUDY)
        return FALSE; /* book depleted */

    /* allocate emsp if needed */
    if (!has_emsp(mtmp))
        newemsp(mtmp);

    /* find empty slot */
    for (i = 0; i < MAXMONSPELL; i++) {
        if (EMSP(mtmp)->msp_id[i] == 0 || EMSP(mtmp)->msp_know[i] <= 0) {
            EMSP(mtmp)->msp_id[i] = book->otyp;
            EMSP(mtmp)->msp_know[i] = MSPELL_KEEN;
            book->spestudied++;
            return TRUE;
        }
    }
    return FALSE; /* no room */
}

/* pick the learned spell a monster would cast at target, applying the
   same target/peaceful filtering cast_learned_spell() uses; pulled out
   so the casting message can reflect the spell actually chosen rather
   than a heuristic guess; returns the spell otyp, or 0 if none is
   castable */
staticfn short
mpick_learned_spell(struct monst *caster, struct monst *target)
{
    boolean youdefend = (target == &gy.youmonst);
    boolean selfbuff_only;
    coordxy tx, ty;

    /* target coordinates gate terrain-specific spells (entangle needs
       vegetation) */
    if (youdefend) {
        tx = u.ux;
        ty = u.uy;
    } else if (target && !DEADMONSTER(target)) {
        tx = target->mx;
        ty = target->my;
    } else {
        return 0;
    }

    /* Only restrict to self-buffs if:
       - caster is a monster (not the player)
       - caster is peaceful
       - target is the player
       This allows tame pets to attack hostile monsters with offensive
       spells, while preventing peaceful spellcasters from attacking
       the player */
    selfbuff_only = (caster != &gy.youmonst && caster->mpeaceful
                     && youdefend);

    return mchoose_learned_spell(caster, target, tx, ty, selfbuff_only);
}

/* cast a spell learned from a spellbook.
 * spell_otyp, when nonzero, is a spell pre-selected by the caller so
 * its casting message can match what is actually cast; pass 0 to
 * choose one here.
 * Returns 0 (damage is handled by the spell itself via mbhitm/etc).
 */
staticfn int
cast_learned_spell(struct monst *caster, struct monst *target,
                   short spell_otyp)
{
    boolean youdefend = (target == &gy.youmonst);
    coordxy tx, ty;

    /* determine target coordinates first; needed to aim ray and
       immediate spells below (selection, when not pre-chosen, applies
       its own terrain filtering) */
    if (youdefend) {
        tx = u.ux;
        ty = u.uy;
    } else if (target && !DEADMONSTER(target)) {
        tx = target->mx;
        ty = target->my;
    } else {
        return 0;
    }

    if (spell_otyp == 0)
        spell_otyp = mpick_learned_spell(caster, target);

    if (spell_otyp == 0)
        return 0; /* no castable spell available */

    /* bounds check before accessing objects array */
    if (spell_otyp < SPE_DIG || spell_otyp > SPE_BLANK_PAPER
        || objects[spell_otyp].oc_class != SPBOOK_CLASS) {
        impossible("cast_learned_spell: invalid spell otyp %d", spell_otyp);
        return 0;
    }

    if (spell_otyp >= SPE_MAGIC_MISSILE && spell_otyp <= SPE_ACID_BLAST) {
        /* ray spells: magic missile, fireball, cone of cold, sleep,
           finger of death, lightning, poison blast, acid blast */
        if (youdefend) {
            if (!lined_up(caster))
                return 0; /* no clear line of sight */
        } else {
            if (!m_lined_up(target, caster, FALSE))
                return 0; /* no clear line of sight */
        }
        /* mcast_ray_spell returns FALSE if caster died from passive
           retaliation along the buzz line; short-circuit before any
           future caller-deref creeps into the trailing return path */
        if (!mcast_ray_spell(caster, tx, ty, spell_otyp))
            return 0;
    } else if (spell_otyp == SPE_FORCE_BOLT
               || spell_otyp == SPE_DRAIN_LIFE
               || spell_otyp == SPE_SLOW_MONSTER
               || spell_otyp == SPE_TURN_UNDEAD
               || spell_otyp == SPE_TELEPORT_AWAY
               || spell_otyp == SPE_POLYMORPH
               || spell_otyp == SPE_ENTANGLE
               || spell_otyp == SPE_DISPEL_EVIL) {
        /* IMMEDIATE spells - use mbhit() ray tracing */
        if (youdefend) {
            if (!lined_up(caster))
                return 0; /* no clear line of sight */
        } else {
            if (!m_lined_up(target, caster, FALSE))
                return 0; /* no clear line of sight */
        }
        /* mcast_immediate_spell returns FALSE if caster died from
           passive retaliation or AT_BOOM target explosion during
           mbhit traversal; short-circuit defensively */
        if (!mcast_immediate_spell(caster, tx, ty, spell_otyp))
            return 0;
    } else if (spell_otyp == SPE_POWER_WORD_KILL) {
        /* NODIR spell - no LOS check needed, directly targets enemy */
        (void) mcast_nodir_spell(caster, target, spell_otyp);
    } else if (spell_otyp == SPE_CURE_BLINDNESS
               || spell_otyp == SPE_CURE_SICKNESS
               || spell_otyp == SPE_REMOVE_CURSE
               || spell_otyp == SPE_REPAIR_ARMOR
               || spell_otyp == SPE_LEVITATION
               || spell_otyp == SPE_JUMPING
               || spell_otyp == SPE_BURNING_HANDS
               || spell_otyp == SPE_SHOCKING_GRASP) {
        /* self-targeting NODIR spells */
        (void) mcast_nodir_spell(caster, caster, spell_otyp);
    }

    return 0; /* spell handlers do their own damage */
}

/* Returns TRUE iff the target is known to be fully immune to
   spell_otyp. For player targets, only the M_SEEN_* memory channel is
   consulted -- the monster must have witnessed the player resist the
   damage type before. For monster targets, direct intrinsic checks
   are used. Spells not in the table return FALSE, so the caller never
   filters them out */
staticfn boolean
target_resists_learned_spell(struct monst *caster, struct monst *target,
                             short spell_otyp)
{
    boolean youdefend = (target == &gy.youmonst);

    switch (spell_otyp) {
    case SPE_MAGIC_MISSILE:
    case SPE_FORCE_BOLT:
        if (youdefend)
            return m_seenres(caster, M_SEEN_MAGR) ? TRUE : FALSE;
        return (resists_magm(target) || defended(target, AD_MAGM));
    case SPE_FIREBALL:
        if (youdefend)
            return m_seenres(caster, M_SEEN_FIRE) ? TRUE : FALSE;
        return (resists_fire(target) || defended(target, AD_FIRE));
    case SPE_CONE_OF_COLD:
        if (youdefend)
            return m_seenres(caster, M_SEEN_COLD) ? TRUE : FALSE;
        return (resists_cold(target) || defended(target, AD_COLD));
    case SPE_SLEEP:
        if (youdefend)
            return m_seenres(caster, M_SEEN_SLEEP) ? TRUE : FALSE;
        return (resists_sleep(target) || defended(target, AD_SLEE));
    case SPE_LIGHTNING:
        if (youdefend)
            return m_seenres(caster, M_SEEN_ELEC) ? TRUE : FALSE;
        return (resists_elec(target) || defended(target, AD_ELEC));
    case SPE_POISON_BLAST:
        if (youdefend)
            return m_seenres(caster, M_SEEN_POISON) ? TRUE : FALSE;
        return (resists_poison(target) || defended(target, AD_DRST));
    case SPE_ACID_BLAST:
        if (youdefend)
            return m_seenres(caster, M_SEEN_ACID) ? TRUE : FALSE;
        return (resists_acid(target) || defended(target, AD_ACID));
    case SPE_FINGER_OF_DEATH:
    case SPE_POWER_WORD_KILL:
        if (youdefend)
            return m_seenres(caster, M_SEEN_DEATH) ? TRUE : FALSE;
        return (immune_death_magic(target->data)
                || is_vampshifter(target));
    case SPE_DRAIN_LIFE:
        /* no M_SEEN_* flag covers draining, so a player target can't
           be pre-filtered; for a monster target, drli resistance and
           nonliving form are sure-thing immunities */
        if (youdefend)
            return FALSE;
        return (resists_drli(target) || nonliving(target->data));
    case SPE_TURN_UNDEAD:
        /* useless unless target is undead/vampshifter; player polyform
           is observable to the monster */
        if (youdefend)
            return (!is_undead(gy.youmonst.data)
                    && !is_vampshifter(&gy.youmonst));
        return (!is_undead(target->data) && !is_vampshifter(target));
    case SPE_DISPEL_EVIL:
        /* useless unless target is evil (A_NONE alignment) */
        if (youdefend)
            return (u.ualign.type != A_NONE);
        return !is_evil(target);
    case SPE_TELEPORT_AWAY:
        /* mirror the wand of teleportation tactics from muse.c */
        if (noteleport_level(caster)
            && mon_knows_traps(caster, TELEP_TRAP_SET))
            return TRUE; /* knows teleporting fails on this level */
        if (youdefend) {
            stairway *stway = stairway_at(u.ux, u.uy);

            return (Teleport_control
                    || !(onscary(u.ux, u.uy, caster)
                         || (stway && stway->tolev.dnum != u.uz.dnum)));
        }
        /* monster target: filter sure-thing no-ops */
        return (noteleport_level(target)
                || (target->ispriest
                    && *in_rooms(target->mx, target->my, TEMPLE)));
    default:
        return FALSE;
    }
}

/* Returns TRUE if monster knows any offensive learned spell that would
   not be wasted against target; used by spell_would_be_useless() to
   reject the learned-spell meta when the whole repertoire resists out,
   letting the re-roll loops pick a different spellnum */
staticfn boolean
mhas_castable_offensive_spell(struct monst *caster, struct monst *target)
{
    int i;
    short spell_otyp;
    int spell_lev, required_mlev;

    if (!has_emsp(caster) || !target)
        return FALSE;

    for (i = 0; i < MAXMONSPELL; i++) {
        spell_otyp = EMSP(caster)->msp_id[i];
        if (spell_otyp == 0 || EMSP(caster)->msp_know[i] <= 0)
            continue;
        /* defensive bounds check */
        if (spell_otyp < SPE_DIG || spell_otyp > SPE_BLANK_PAPER
            || objects[spell_otyp].oc_class != SPBOOK_CLASS)
            continue;
        if (IS_SELFBUFF_SPELL(spell_otyp))
            continue;
        /* charm monster has no direct-cast handler */
        if (spell_otyp == SPE_CHARM_MONSTER)
            continue;
        spell_lev = objects[spell_otyp].oc_level;
        required_mlev = spell_lev * 3;
        if (caster->m_lev < required_mlev)
            continue;
        if (target_resists_learned_spell(caster, target, spell_otyp))
            continue;
        return TRUE;
    }
    return FALSE;
}

/* Returns TRUE if monster knows any self-buff learned spell that would
   be useful to cast right now; used by is_undirected_spell() to
   determine if learned spells can be cast through the undirected spell
   path */
staticfn boolean
mhas_useful_selfbuff_spell(struct monst *mtmp)
{
    int i, j;
    short spell_otyp;
    int spell_lev, required_mlev;

    if (!has_emsp(mtmp))
        return FALSE;

    for (i = 0; i < MAXMONSPELL; i++) {
        spell_otyp = EMSP(mtmp)->msp_id[i];
        if (spell_otyp == 0 || EMSP(mtmp)->msp_know[i] <= 0)
            continue;

        /* only check self-buff spells */
        if (!IS_SELFBUFF_SPELL(spell_otyp))
            continue;

        /* check level requirement */
        spell_lev = objects[spell_otyp].oc_level;
        required_mlev = spell_lev * 3;
        if (mtmp->m_lev < required_mlev)
            continue;

        /* check usefulness of each self-buff spell */
        if (spell_otyp == SPE_CURE_BLINDNESS) {
            if (mtmp->mblinded)
                return TRUE;
        } else if (spell_otyp == SPE_CURE_SICKNESS) {
            if (mtmp->msick || mtmp->mdiseased || mtmp->mwither)
                return TRUE;
        } else if (spell_otyp == SPE_REMOVE_CURSE) {
            struct obj *obj;

            /* Infidels are immune to curses */
            if (mtmp->data == &mons[PM_INFIDEL])
                continue;
            for (obj = mtmp->minvent; obj; obj = obj->nobj) {
                if (obj->cursed
                    && (obj->owornmask || Is_mbag(obj)
                        || obj->otyp == LOADSTONE))
                    return TRUE;
            }
        } else if (spell_otyp == SPE_REPAIR_ARMOR) {
            struct obj *arm = some_armor(mtmp);

            if (arm && greatest_erosion(arm) > 0)
                return TRUE;
        } else if (spell_otyp == SPE_LEVITATION) {
            if (!is_floater(mtmp->data) && !is_flyer(mtmp->data)
                && !can_levitate(mtmp) && !can_fly(mtmp))
                return TRUE;
        } else if (spell_otyp == SPE_JUMPING) {
            if (!is_jumper(mtmp->data) && !can_jump(mtmp)
                && !is_flyer(mtmp->data) && !is_floater(mtmp->data)
                && !can_levitate(mtmp) && !can_fly(mtmp))
                return TRUE;
        } else if (spell_otyp == SPE_BURNING_HANDS) {
            boolean has_touch = FALSE;
            struct attack *mattk = has_erac(mtmp) ? ERAC(mtmp)->mattk
                                                  : mtmp->data->mattk;

            for (j = 0; j < NATTK; j++) {
                int at = mattk[j].aatyp;

                if (at == AT_CLAW || at == AT_TUCH || at == AT_WEAP) {
                    has_touch = TRUE;
                    break;
                }
            }
            if (has_touch && mtmp->mburnhands < 20 && !mtmp->mshockgrasp
                && !mon_underwater(mtmp))
                return TRUE;
        } else if (spell_otyp == SPE_SHOCKING_GRASP) {
            boolean has_touch = FALSE;
            struct attack *mattk = has_erac(mtmp) ? ERAC(mtmp)->mattk
                                                  : mtmp->data->mattk;

            for (j = 0; j < NATTK; j++) {
                int at = mattk[j].aatyp;

                if (at == AT_CLAW || at == AT_TUCH || at == AT_WEAP) {
                    has_touch = TRUE;
                    break;
                }
            }
            if (has_touch && mtmp->mshockgrasp < 20 && !mtmp->mburnhands)
                return TRUE;
        }
    }
    return FALSE;
}

/* count how many learned spells the monster can cast at its current
   level */
staticfn int
mcount_castable_spells(struct monst *mtmp)
{
    int count = 0, i;
    short spell_otyp;
    int spell_level, required_mlev;

    if (!has_emsp(mtmp))
        return 0;

    for (i = 0; i < MAXMONSPELL; i++) {
        spell_otyp = EMSP(mtmp)->msp_id[i];
        if (spell_otyp != 0 && EMSP(mtmp)->msp_know[i] > 0) {
            /* skip invalid entries */
            if (spell_otyp < SPE_DIG || spell_otyp > SPE_BLANK_PAPER
                || objects[spell_otyp].oc_class != SPBOOK_CLASS)
                continue;
            /* charm monster has no direct-cast handler */
            if (spell_otyp == SPE_CHARM_MONSTER)
                continue;
            /* check level requirement */
            spell_level = objects[spell_otyp].oc_level;
            required_mlev = spell_level * 3;
            if (mtmp->m_lev >= required_mlev)
                count++;
        }
    }
    return count;
}

/* Get a random learned spell monster can cast; returns otyp or 0.
   target is the intended target (or NULL for availability checks
   before target selection); if non-NULL and not selfbuff_only,
   offensive spells that target is known to resist are filtered out.
   tx, ty are target coordinates for filtering spells that require
   specific terrain (entangle needs vegetation); pass (0,0) to skip
   terrain filtering (used when just checking spell availability) */
short
mchoose_learned_spell(struct monst *mtmp, struct monst *target,
                      coordxy tx, coordxy ty,
                      boolean selfbuff_only) /* TRUE = only return
                                              * self-buff spells */
{
    short candidates[MAXMONSPELL];
    int count = 0, i;
    short spell_otyp;
    int spell_level, required_mlev;

    if (!has_emsp(mtmp))
        return 0;

    /* priority: urgent self-cure spells are cast immediately if
       needed; cure sickness is level 3 (requires mlev 9), cure
       blindness is level 2 (mlev 6) */
    if ((mtmp->msick || mtmp->mdiseased || mtmp->mwither)
        && mtmp->m_lev >= 9 && mknows_spell(mtmp, SPE_CURE_SICKNESS))
        return SPE_CURE_SICKNESS;
    if (mtmp->mblinded
        && mtmp->m_lev >= 6 && mknows_spell(mtmp, SPE_CURE_BLINDNESS))
        return SPE_CURE_BLINDNESS;

    for (i = 0; i < MAXMONSPELL; i++) {
        spell_otyp = EMSP(mtmp)->msp_id[i];
        if (spell_otyp != 0 && EMSP(mtmp)->msp_know[i] > 0) {
            /* defensive bounds check for corrupted emsp data */
            if (spell_otyp < SPE_DIG || spell_otyp > SPE_BLANK_PAPER
                || objects[spell_otyp].oc_class != SPBOOK_CLASS) {
                impossible("mchoose_learned_spell: invalid otyp %d",
                           spell_otyp);
                EMSP(mtmp)->msp_id[i] = 0; /* clear corrupted entry */
                continue;
            }
            /* check if monster is high enough level to cast this
               spell; spell levels 1-7 map to required monster levels
               3-21 */
            spell_level = objects[spell_otyp].oc_level;
            required_mlev = spell_level * 3;
            if (mtmp->m_lev < required_mlev)
                continue;
            /* charm monster has no direct-cast handler; knowing it
               only enables the untame-pet special in castmm() */
            if (spell_otyp == SPE_CHARM_MONSTER)
                continue;
            /* if only self-buff spells wanted, skip offensive spells */
            if (selfbuff_only && !IS_SELFBUFF_SPELL(spell_otyp))
                continue;
            /* skip offensive spells the target is known to fully
               resist; selfbuffs and unknown spells pass through */
            if (target && !selfbuff_only
                && !IS_SELFBUFF_SPELL(spell_otyp)
                && target_resists_learned_spell(mtmp, target, spell_otyp))
                continue;
            /* entangle requires vegetation at the target location;
               skip filtering if tx,ty are 0 (availability check) */
            if (spell_otyp == SPE_ENTANGLE && (tx || ty)) {
                if (!(levl[tx][ty].typ == GRASS || nexttotree(tx, ty)))
                    continue;
            }
            /* self-cure spells already handled by the priority check
               above */
            if (spell_otyp == SPE_CURE_BLINDNESS && !mtmp->mblinded)
                continue;
            if (spell_otyp == SPE_CURE_SICKNESS
                && !mtmp->msick && !mtmp->mdiseased && !mtmp->mwither)
                continue;
            /* remove curse only useful if monster has cursed items to
               uncurse */
            if (spell_otyp == SPE_REMOVE_CURSE) {
                boolean has_cursed = FALSE;
                struct obj *obj;

                /* Infidels are immune to curses */
                if (mtmp->data == &mons[PM_INFIDEL])
                    continue;
                for (obj = mtmp->minvent; obj; obj = obj->nobj) {
                    if (obj->cursed
                        && (obj->owornmask || Is_mbag(obj)
                            || obj->otyp == LOADSTONE)) {
                        has_cursed = TRUE;
                        break;
                    }
                }
                if (!has_cursed)
                    continue;
            }
            /* repair armor only useful if monster has eroded armor */
            if (spell_otyp == SPE_REPAIR_ARMOR) {
                struct obj *arm = some_armor(mtmp);

                if (!arm || greatest_erosion(arm) == 0)
                    continue;
            }
            /* levitation only useful if not already floating/flying */
            if (spell_otyp == SPE_LEVITATION) {
                if (is_floater(mtmp->data) || is_flyer(mtmp->data)
                    || can_levitate(mtmp) || can_fly(mtmp))
                    continue;
            }
            /* jumping only useful if not already jumping, flying, or
               levitating */
            if (spell_otyp == SPE_JUMPING) {
                if (is_jumper(mtmp->data) || can_jump(mtmp)
                    || is_flyer(mtmp->data) || is_floater(mtmp->data)
                    || can_levitate(mtmp) || can_fly(mtmp))
                    continue;
            }
            /* burning hands / shocking grasp: useless if monster has
               no unarmed touch attacks (AT_CLAW, AT_TUCH, AT_WEAP) */
            if (spell_otyp == SPE_BURNING_HANDS
                || spell_otyp == SPE_SHOCKING_GRASP) {
                boolean has_touch_attack = FALSE;
                struct attack *mattk = has_erac(mtmp) ? ERAC(mtmp)->mattk
                                                      : mtmp->data->mattk;
                int j;

                for (j = 0; j < NATTK; j++) {
                    int at = mattk[j].aatyp;

                    if (at == AT_CLAW || at == AT_TUCH || at == AT_WEAP) {
                        has_touch_attack = TRUE;
                        break;
                    }
                }
                if (!has_touch_attack)
                    continue;
            }
            /* burning hands: also useless if at cap, shocking grasp
               active, or underwater */
            if (spell_otyp == SPE_BURNING_HANDS) {
                if (mtmp->mburnhands >= 20 || mtmp->mshockgrasp
                    || mon_underwater(mtmp))
                    continue;
            }
            /* shocking grasp: also useless if at cap or burning hands
               active */
            if (spell_otyp == SPE_SHOCKING_GRASP) {
                if (mtmp->mshockgrasp >= 20 || mtmp->mburnhands)
                    continue;
            }
            candidates[count++] = spell_otyp;
        }
    }

    if (count == 0)
        return 0;
    return candidates[rn2(count)];
}

/* decay monster spell knowledge - called from moveloop */
void
mage_spells(struct monst *mtmp)
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
                esp->msp_id[i] = 0; /* forgot spell */
        }
    }
}

/*mcastu.c*/
