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
    MCAST_OPEN_WOUNDS, MCAST_CURE_SELF, MCAST_CONFUSE_YOU, MCAST_PARALYZE,
    MCAST_BLIND_YOU, MCAST_INSECTS, MCAST_CURSE_ITEMS, MCAST_LIGHTNING,
    MCAST_FIRE_PILLAR, MCAST_GEYSER
};
static int mon_wizard_spells[] = {
    MCAST_PSI_BOLT, MCAST_CURE_SELF, MCAST_HASTE_SELF, MCAST_STUN_YOU,
    MCAST_DISAPPEAR, MCAST_WEAKEN_YOU, MCAST_DESTRY_ARMR, MCAST_CURSE_ITEMS,
    MCAST_AGGRAVATION, MCAST_SUMMON_MONS, MCAST_CLONE_WIZ, MCAST_DEATH_TOUCH
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
staticfn int choose_monster_spell(struct monst *, int);
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
staticfn void mcast_death_touch(struct monst *);
staticfn void mcast_clone_wiz(struct monst *);
staticfn void mcast_summon_mons(struct monst *);
staticfn void mcast_destroy_armor(void);
staticfn void mcast_weaken_you(struct monst *, int);
staticfn void mcast_disappear(struct monst *);
staticfn void mcast_stun_you(int);
staticfn int mcast_geyser(struct monst *, int);
staticfn int mcast_fire_pillar(struct monst *, int);
staticfn int mcast_lightning(struct monst *, int);
staticfn int mcast_psi_bolt(int);
staticfn int mcast_open_wounds(int);
staticfn void mcast_insects(struct monst *);
staticfn void mcast_blind_you(void);
staticfn int mcast_paralyze(struct monst *);
staticfn void mcast_confuse_you(struct monst *);
staticfn void mcast_spell(struct monst *, int, int, short);
staticfn boolean is_undirected_spell(struct monst *, int);
staticfn boolean spell_would_be_useless(struct monst *, int);

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
choose_monster_spell(struct monst *mtmp, int adtyp)
{
    int *list = NULL;
    int i, spellval, len = 0;
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

    /* which level spell to cast? */
    spellval = rn2(mtmp->m_lev);
    if (spellval > maxlev && rn2(maxlev))
        spellval = rn2(maxlev);

    /* find the highest spell in the list we could cast,
       or fall back to the first spell in the list */
    for (i = len - 1; i > 0; i--)
        if (mcast_data[list[i]].level <= spellval
            && !spell_would_be_useless(mtmp, list[i]))
            break;

    /* the Ice Queen and Asmodeus are creatures of cold; they conjure
       lightning where other clerical casters would call down fire */
    if (list[i] == MCAST_FIRE_PILLAR
        && (mtmp->data == &mons[PM_KATHRYN_THE_ICE_QUEEN]
            || mtmp->data == &mons[PM_ASMODEUS]))
        return MCAST_LIGHTNING;

    return list[i];
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
    int dmg, ml = mtmp->m_lev;
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
            spellnum = choose_monster_spell(mtmp, mattk->adtyp);
            /* not trying to attack?  don't allow directed spells */
            if (!thinks_it_foundyou) {
                if (!is_undirected_spell(mtmp, spellnum)
                    || spell_would_be_useless(mtmp, spellnum)) {
                    if (foundyou)
                        impossible(
                       "spellcasting monster found you and doesn't know it?");
                    return M_ATTK_MISS;
                }
                break;
            }
        } while (--cnt > 0
                 && spell_would_be_useless(mtmp, spellnum));
        if (cnt == 0)
            return M_ATTK_MISS;
    }

    /* monster unable to cast spells? */
    if (mtmp->mcan || mtmp->mspec_used || !ml
        || m_seenres(mtmp, cvt_adtyp_to_mseenres(mattk->adtyp))) {
        cursetxt(mtmp, is_undirected_spell(mtmp, spellnum));
        return M_ATTK_MISS;
    }

    debugpline3("castmu:%s,lvl:%i,spell:%i", noit_Monnam(mtmp), ml, spellnum);

    if (mattk->adtyp == AD_SPEL || mattk->adtyp == AD_CLRC) {
        /* monst->m_lev is unsigned (uchar), monst->mspec_used is int */
        mtmp->mspec_used = (int) ((mtmp->m_lev < 8) ? (10 - mtmp->m_lev) : 2);
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

    nomul(0);
    if (rn2(ml * 10) < (mtmp->mconf ? 100 : 20)) { /* fumbled attack */
        Soundeffect(se_air_crackles, 60);
        if (canseemon(mtmp) && !Deaf) {
            set_msg_xy(mtmp->mx, mtmp->my);
            pline_The("air crackles around %s.", mon_nam(mtmp));
        }
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
        if (Fire_resistance) {
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
                dmg *= 2;
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
        if (Cold_resistance) {
            shieldeff(u.ux, u.uy);
            pline("But you resist the effects.");
            monstseesu(M_SEEN_COLD);
            dmg = 0;
        } else {
            monstunseesu(M_SEEN_COLD);
        }
        /* freeze water or lava terrain */
        /* FIXME: mon_spell_hits_spot() uses zap_over_floor(); unlike with
         * fire, it does not target susceptible floor items with cold */
        mon_spell_hits_spot(mtmp, AD_COLD, u.ux, u.uy);
        break;
    case AD_MAGM:
        You("are hit by a shower of missiles!");
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
            pline_The("missiles bounce off!");
            monstseesu(M_SEEN_MAGR);
            dmg = 0;
        } else {
            dmg = d((int) mtmp->m_lev / 2 + 1, 6);
            monstunseesu(M_SEEN_MAGR);
        }
        /* shower of magic missiles scuffs an engraving */
        mon_spell_hits_spot(mtmp, AD_MAGM, u.ux, u.uy);
        break;
    case AD_SPEL: /* wizard spell */
    case AD_CLRC: /* clerical spell */
        mcast_spell(mtmp, dmg, spellnum, learned_otyp);
        dmg = 0; /* done by the spell casting functions */
        break;
    } /* switch */
    if (dmg)
        mdamageu(mtmp, dmg);
    return ret;
}

staticfn int
m_cure_self(struct monst *mtmp, int dmg)
{
    if (mtmp->mhp < mtmp->mhpmax) {
        if (canseemon(mtmp))
            pline_mon(mtmp, "%s looks better.", Monnam(mtmp));
        /* note: player healing does 6d4; this used to do 1d8 */
        healmon(mtmp, d(3, 6), 0);
        dmg = 0;
    }
    return dmg;
}

/* unlike the finger of death spell which behaves like a wand of death,
   this monster spell only attacks the hero */
void
touch_of_death(struct monst *mtmp)
{
    char kbuf[BUFSZ];
    int dmg = 50 + d(8, 6);
    int drain = dmg / 2;

    /* if we get here, we know that hero isn't magic resistant and isn't
       poly'd into an undead or demon */
    You_feel("drained...");
    (void) death_inflicted_by(kbuf, "the touch of death", mtmp);

    if (Upolyd) {
        u.mh = 0;
        rehumanize(); /* fatal iff Unchanging */
    } else if (drain >= u.uhpmax) {
        svk.killer.format = KILLED_BY;
        Strcpy(svk.killer.name, kbuf);
        done(DIED);
    } else {
        /* HP manipulation similar to poisoned(attrib.c) */
        int olduhp = u.uhp,
            uhpmin = minuhpmax(3),
            newuhpmax = u.uhpmax - drain;

        setuhpmax(max(newuhpmax, uhpmin), FALSE);
        dmg = adjuhploss(dmg, olduhp); /* reduce pending damage if uhp has
                                        * already been reduced due to drop
                                        * in uhpmax */
        losehp(dmg, kbuf, KILLED_BY);
    }
    svk.killer.name[0] = '\0'; /* not killed if we get here... */
}

/* articled species name of the monster responsible for a kill, for the
   death-reason builders: "a goblin", "the Wizard of Yendor", "Asmodeus"
   (plus " imitating <X>" for a shapeshifter); names by species, like
   done_in_by() does for the simple cases */
char *
mon_killer_name(
    char *outbuf, /* assumed big enough; pm_names are short */
    struct monst *mtmp)
{
    struct permonst *mptr = mtmp->data,
        *champtr = (ismnum(mtmp->cham)) ? &mons[mtmp->cham] : mptr;
    const char *realnm = pmname(champtr, Mgender(mtmp)),
        *fakenm = pmname(mptr, Mgender(mtmp));

    /* greatly simplified extract from done_in_by(); names the real
       species when a shapeshifter is disguised */
    if (!type_is_pname(champtr) && !the_unique_pm(champtr))
        realnm = an(realnm);
    Sprintf(outbuf, "%s%s", the_unique_pm(champtr) ? "the " : "", realnm);
    if (champtr != mptr)
        Sprintf(eos(outbuf), " imitating %s", an(fakenm));
    return outbuf;
}

/* append " inflicted by <monster>" to a death reason; the caller may then
   strsubst("inflicted", verb) to get "zapped/cast/exhaled by <monster>" */
char *
death_inflicted_by(
    char *outbuf,            /* assumed big enough; pm_names are short */
    const char *deathreason, /* cause of death */
    struct monst *mtmp)      /* monster who caused it */
{
    Strcpy(outbuf, deathreason);
    if (mtmp) {
        char nbuf[BUFSZ];

        Sprintf(eos(outbuf), " inflicted by %s", mon_killer_name(nbuf, mtmp));
    }
    return outbuf;
}

/*
 * Monster wizard and cleric spellcasting functions.
 */

staticfn void
mcast_death_touch(struct monst *mtmp)
{
    pline("Oh no, %s's using the touch of death!", mhe(mtmp));
    if (nonliving(gy.youmonst.data) || is_demon(gy.youmonst.data)) {
        You("seem no deader than before.");
    } else if (!Antimagic && rn2(mtmp->m_lev) > 12) {
        if (Hallucination) {
            You("have an out of body experience.");
        } else {
            touch_of_death(mtmp);
        }
        monstunseesu(M_SEEN_MAGR);
    } else {
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
            monstseesu(M_SEEN_MAGR);
        }
        pline("Lucky for you, it didn't work!");
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
mcast_summon_mons(struct monst *mtmp)
{
    int count = nasty(mtmp);

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
}

staticfn void
mcast_destroy_armor(void)
{
    if (Antimagic) {
        shieldeff(u.ux, u.uy);
        monstseesu(M_SEEN_MAGR);
        pline("A field of force surrounds you!");
    } else if (!destroy_arm()) {
        Your("skin itches.");
    } else {
        /* monsters only realize you aren't magic-protected if armor is
           actually destroyed */
        monstunseesu(M_SEEN_MAGR);
    }
}

staticfn void
mcast_weaken_you(struct monst *mtmp, int dmg)
{
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
        losestr(rnd(dmg),
                death_inflicted_by(kbuf, "strength loss", mtmp),
                KILLED_BY);
        svk.killer.name[0] = '\0'; /* not killed if we get here... */
        monstunseesu(M_SEEN_MAGR);
    }
}

staticfn void
mcast_disappear(struct monst *mtmp)
{
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
mcast_stun_you(int dmg)
{
    if (Antimagic || Free_action) {
        shieldeff(u.ux, u.uy);
        monstseesu(M_SEEN_MAGR);
        if (!Stunned)
            You_feel("momentarily disoriented.");
        make_stunned(1L, FALSE);
    } else {
        You(Stunned ? "struggle to keep your balance." : "reel...");
        dmg = d(ACURR(A_DEX) < 12 ? 6 : 4, 4);
        if (Half_spell_damage)
            dmg = (dmg + 1) / 2;
        make_stunned((HStun & TIMEOUT) + (long) dmg, FALSE);
        monstunseesu(M_SEEN_MAGR);
    }
}

staticfn int
mcast_geyser(struct monst *mtmp, int dmg)
{
    /* this is physical damage (force not heat),
     * not magical damage or fire damage
     */
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
#if 0   /* since inventory items aren't affected, don't include this */
        /* make floor items wet */
    water_damage_chain(level.objects[u.ux][u.uy], TRUE);
#endif
    return dmg;
}

staticfn int
mcast_fire_pillar(struct monst *mtmp, int dmg)
{
    int orig_dmg;

    pline("A pillar of fire strikes all around you!");
    orig_dmg = dmg = d(8, 6);
    if (Fire_resistance) {
        shieldeff(u.ux, u.uy);
        monstseesu(M_SEEN_FIRE);
        dmg = 0;
    } else {
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
}

staticfn int
mcast_lightning(struct monst *mtmp, int dmg)
{
    int orig_dmg;
    boolean reflects;

    Soundeffect(se_bolt_of_lightning, 80);
    pline("A bolt of lightning strikes down at you from above!");
    reflects = ureflects("It bounces off your %s%s.", "");
    orig_dmg = dmg = d(8, 6);
    if (reflects || Shock_resistance) {
        shieldeff(u.ux, u.uy);
        dmg = 0;
        if (reflects) {
            monstseesu(M_SEEN_REFL);
            return dmg;
        }
        monstunseesu(M_SEEN_REFL);
        monstseesu(M_SEEN_ELEC);
    } else {
        monstunseesu(M_SEEN_ELEC | M_SEEN_REFL);
    }
    if (Half_spell_damage)
        dmg = (dmg + 1) / 2;
    (void) destroy_items(&gy.youmonst, AD_ELEC, orig_dmg);
    /* lightning might destroy iron bars if hero is on such a spot;
       reflection protects terrain here [execution won't get here due
       to 'if (reflects) break' above] but hero resistance doesn't;
       do this before maybe blinding the hero via flashburn() */
    mon_spell_hits_spot(mtmp, AD_ELEC, u.ux, u.uy);
    /* blind hero; no effect if already blind */
    (void) flashburn((long) rnd(100), TRUE);
    return dmg;
}

staticfn int
mcast_psi_bolt(int dmg)
{
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
}

staticfn int
mcast_open_wounds(int dmg)
{
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
}

staticfn void
mcast_insects(struct monst *mtmp)
{
    /* Try for insects, and if there are none
       left, go for (sticks to) snakes. -3.
       Drow and Lolth summon spiders instead */
    boolean spiders = (mtmp->data == &mons[PM_LOLTH] || racial_drow(mtmp));
    struct permonst *pm = mkclass(spiders ? S_SPIDER : S_ANT, 0);
    struct monst *mtmp2 = (struct monst *) 0;
    char whatbuf[QBUFSZ], let = (pm ? (spiders ? S_SPIDER : S_ANT) : S_SNAKE);
    boolean success = FALSE, seecaster;
    int i, quan, oldseen, newseen;
    coord bypos;
    const char *fmt, *what;

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
mcast_blind_you(void)
{
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
}

staticfn int
mcast_paralyze(struct monst *mtmp)
{
    int dmg = 0;

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
    return dmg;
}

staticfn void
mcast_confuse_you(struct monst *mtmp)
{
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
}

/*
   If dmg is zero, then the monster is not casting at you.
   If the monster is intentionally not casting at you, we have previously
   called spell_would_be_useless() and spellnum should always be a valid
   undirected spell.
   If you modify either of these, be sure to change is_undirected_spell()
   and spell_would_be_useless().
 */
staticfn void
mcast_spell(struct monst *mtmp, int dmg, int spellnum, short learned_otyp)
{
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
        mcast_death_touch(mtmp);
        dmg = 0;
        break;
    case MCAST_CLONE_WIZ:
        mcast_clone_wiz(mtmp);
        dmg = 0;
        break;
    case MCAST_SUMMON_MONS:
        mcast_summon_mons(mtmp);
        dmg = 0;
        break;
    case MCAST_AGGRAVATION:
        You_feel("that monsters are aware of your presence.");
        aggravate();
        dmg = 0;
        break;
    case MCAST_CURSE_ITEMS:
        You_feel("as if you need some help.");
        rndcurse();
        dmg = 0;
        break;
    case MCAST_DESTRY_ARMR:
        mcast_destroy_armor();
        dmg = 0;
        break;
    case MCAST_WEAKEN_YOU: /* drain strength */
        mcast_weaken_you(mtmp, dmg);
        dmg = 0;
        break;
    case MCAST_DISAPPEAR: /* makes self invisible */
        mcast_disappear(mtmp);
        dmg = 0;
        break;
    case MCAST_STUN_YOU:
        mcast_stun_you(dmg);
        dmg = 0;
        break;
    case MCAST_HASTE_SELF:
        mon_adjust_speed(mtmp, 1, (struct obj *) 0);
        dmg = 0;
        break;
    case MCAST_CURE_SELF:
        dmg = m_cure_self(mtmp, dmg);
        break;
    case MCAST_PSI_BOLT:
        dmg = mcast_psi_bolt(dmg);
        break;
    case MCAST_GEYSER:
        dmg = mcast_geyser(mtmp, dmg);
        break;
    case MCAST_FIRE_PILLAR:
        dmg = mcast_fire_pillar(mtmp, dmg);
        break;
    case MCAST_LIGHTNING:
        dmg = mcast_lightning(mtmp, dmg);
        break;
    case MCAST_INSECTS:
        mcast_insects(mtmp);
        dmg = 0;
        break;
    case MCAST_BLIND_YOU:
        mcast_blind_you();
        dmg = 0;
        break;
    case MCAST_PARALYZE:
        dmg = mcast_paralyze(mtmp);
        break;
    case MCAST_CONFUSE_YOU:
        mcast_confuse_you(mtmp);
        dmg = 0;
        break;
    case MCAST_OPEN_WOUNDS:
        dmg = mcast_open_wounds(dmg);
        break;
    case MCAST_LEARNED_SPELL:
        dmg = cast_learned_spell(mtmp, &gy.youmonst, learned_otyp);
        break;
    default:
        impossible("mcastu: invalid magic spell (%d)", spellnum);
        dmg = 0;
        break;
    }

    if (dmg)
        mdamageu(mtmp, dmg);
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
spell_would_be_useless(struct monst *mtmp, int spellnum)
{
    /* Some spells don't require the player to really be there and can be cast
     * by the monster when you're invisible, yet still shouldn't be cast when
     * the monster doesn't even think you're there.
     * This check isn't quite right because it always uses your real position.
     * We really want something like "if the monster could see mux, muy".
     */

    /* spell is only cast by hostile monsters */
    if ((mcast_data[spellnum].flags & MCF_HOSTILE) != 0) {
        if (mtmp->mpeaceful)
            return TRUE;
    }

    /* spell needs the monster to see hero */
    if ((mcast_data[spellnum].flags & MCF_SIGHT) != 0) {
        boolean mcouldseeu = couldsee(mtmp->mx, mtmp->my);

        if (!mcouldseeu)
            return TRUE;
    }

    switch (spellnum) {
    case MCAST_LEARNED_SPELL:
        /* peaceful casters (or ones that cannot see the hero, or when
           the hero is invulnerable) may only use self-buffs on the
           at-hero path */
        if ((mtmp->mpeaceful || !couldsee(mtmp->mx, mtmp->my)
             || u.uinvulnerable)
            && !mhas_useful_selfbuff_spell(mtmp))
            return TRUE;
        /* reject when the entire offensive repertoire would resist out
           against the hero and no useful self-buff is queued, letting
           the re-roll loop pick a different spell */
        if (!mhas_useful_selfbuff_spell(mtmp)
            && !mhas_castable_offensive_spell(mtmp, &gy.youmonst))
            return TRUE;
        break;
    case MCAST_DEATH_TOUCH:
        if ((Antimagic || Hallucination) && !rn2(2))
            return TRUE;
        break;
    case MCAST_GEYSER:
        if (!rn2(5))
            return TRUE;
        break;
    case MCAST_CLONE_WIZ:
        /* only the Wizard is allowed to clone himself */
        if (!mtmp->iswiz || svc.context.no_of_wizards > 1)
            return TRUE;
        break;
    case MCAST_AGGRAVATION:
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
        if (mtmp->permspeed == MFAST)
            return TRUE;
        break;
    case MCAST_DISAPPEAR:
        /* invisibility when already invisible */
        if (mtmp->minvis || mtmp->invis_blkd)
            return TRUE;
        /* peaceful monster won't cast invisibility if you can't see
           invisible,
           same as when monsters drink potions of invisibility.  This doesn't
           really make a lot of sense, but lets the player avoid hitting
           peaceful monsters by mistake */
        if (mtmp->mpeaceful && !See_invisible)
            return TRUE;
        break;
    case MCAST_CURE_SELF:
        /* healing when already healed */
        if (mtmp->mhp == mtmp->mhpmax)
            return TRUE;
        break;
    case MCAST_BLIND_YOU:
        if (Blinded)
            return TRUE;
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

    if (mtmp->mcan || m_seenres(mtmp, cvt_adtyp_to_mseenres(mattk->adtyp))) {
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
   repertoire, the charm-monster untame special, and the innate
   elemental hand-to-hand casts (hellfire and kin) are implemented
   here; the innate AD_SPEL/AD_CLRC spell lists are not yet cast at
   monster targets */
int
castmm(struct monst *mtmp, struct monst *mdef, struct attack *mattk)
{
    int dmg, ml = mtmp->m_lev;
    int spellnum = -1, spell_count;
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

        if (spellnum < 0)
            return M_ATTK_MISS; /* nothing castable at a monster target */

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
        mtmp->mspec_used = (int) ((mtmp->m_lev < 8) ? (10 - mtmp->m_lev)
                                                    : 2);
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

    switch (spellnum) {
    case MCAST_UNTAME_PET:
        do_untame_pet(mtmp, mdef);
        break;
    case MCAST_LEARNED_SPELL: {
        /* route self-buffs to the caster and offensive spells at
           mdef, reusing the pre-selected spell */
        struct monst *ltgt = (learned_otyp
                              && IS_SELFBUFF_SPELL(learned_otyp)) ? mtmp
                                                                  : mdef;

        (void) cast_learned_spell(mtmp, ltgt, learned_otyp);
        break;
    }
    default:
        impossible("castmm: unexpected spell (%d)", spellnum);
        return M_ATTK_MISS;
    }
    return M_ATTK_HIT;
}

/* the hero, polymorphed into a form with an elemental AT_MAGC attack,
   casts at an adjacent monster; the innate AD_SPEL/AD_CLRC monster
   spell lists have no hero-as-caster implementation yet, so those
   attack slots do not route here */
int
castum(struct monst *mtmp, struct attack *mattk)
{
    int dmg, ml = mons[u.umonnum].mlevel;

    /* guard against casting another spell attack at an already dead
       monster; some monsters have multiple AT_MAGC attacks */
    if (DEADMONSTER(mtmp))
        return M_ATTK_MISS;

    if ((mattk->adtyp != AD_FIRE && mattk->adtyp != AD_COLD
         && mattk->adtyp != AD_ACID && mattk->adtyp != AD_MAGM) || !ml)
        return M_ATTK_MISS;

    /* unable to cast spells? */
    if (u.uen < ml) {
        You("point at %s, then curse.", mon_nam(mtmp));
        return M_ATTK_MISS;
    }
    /* innate elemental casts drain no energy; only the wizard and
       clerical spell lists charge power */

    if (rn2(ml * 10) < (Confusion ? 100 : 20)) { /* fumbled attack */
        pline_The("air crackles around you.");
        return M_ATTK_MISS;
    }

    You("cast a spell at %s!", mon_nam(mtmp));

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
            /* TODO-EVILHACK: also allow GRASS terrain once the grass
               terrain type is ported (terrain slice) */
            if (spell_otyp == SPE_ENTANGLE && (tx || ty)) {
                if (!nexttotree(tx, ty))
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
