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
    MGC_DEATH_TOUCH
};

/* monster cleric spells */
enum mcast_cleric_spells {
    CLC_OPEN_WOUNDS = 0,
    CLC_CURE_SELF,
    CLC_CONFUSE_YOU,
    CLC_PARALYZE,
    CLC_VULN_YOU,
    CLC_BLIND_YOU,
    CLC_INSECTS,
    CLC_CURSE_ITEMS,
    CLC_LIGHTNING,
    CLC_FIRE_PILLAR,
    CLC_GEYSER,
    CLC_SUMMON_ELM
};

extern void you_aggravate(struct monst *);

STATIC_DCL void FDECL(cursetxt, (struct monst *, BOOLEAN_P));
STATIC_DCL int FDECL(choose_magic_spell, (struct monst *, int));
STATIC_DCL int FDECL(choose_clerical_spell, (struct monst *, int));
STATIC_DCL int FDECL(m_cure_self, (struct monst *, int));
STATIC_DCL void FDECL(cast_wizard_spell, (struct monst *, int, int));
STATIC_DCL void FDECL(cast_cleric_spell, (struct monst *, int, int));
STATIC_DCL boolean FDECL(is_undirected_spell, (unsigned int, int));
STATIC_DCL boolean FDECL(spell_would_be_useless, (struct monst *, unsigned int, int));
STATIC_DCL boolean FDECL(uspell_would_be_useless,(unsigned int,int));
STATIC_DCL void FDECL(ucast_wizard_spell, (struct monst *, struct monst *, int, int));
STATIC_DCL void FDECL(ucast_cleric_spell, (struct monst *, struct monst *, int, int));

boolean
is_spellcaster(ptr)
register struct permonst *ptr;
{
    int i = 0;
    for (; i < NATTK; i++) {
        if (ptr->mattk[i].aatyp == AT_MAGC
            && (ptr->mattk[i].adtyp == AD_SPEL
                || ptr->mattk[i].adtyp == AD_CLRC)) {
            return TRUE;
        }
    }
    return FALSE;
}

boolean
can_cast_spells(mtmp)
register struct monst *mtmp;
{
    return mtmp->mspec_used <= maxspelltimeout(mtmp);
}

int maxspelltimeout(mtmp)
register struct monst *mtmp;
{
   if (mtmp->m_lev >= 38)
       return 255;
   return 8 * mtmp->m_lev * ((38 - mtmp->m_lev) * 2 / 3);
}

int spelltimeout(mtmp, level)
register struct monst *mtmp;
register int level;
{
    if (mtmp->m_lev >= 38)
	return 1;
    return 5 * level * ((38 - mtmp->m_lev) * 2 / 3);
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
            Norep("You hear a mumbled curse.");
    }
}

/* convert a level based random selection into a specific mage spell;
   inappropriate choices will be screened out by spell_would_be_useless() */
STATIC_OVL int
choose_magic_spell(mtmp, spellval)
struct monst* mtmp;
int spellval;
{
    int i;

    /* for 3.4.3 and earlier, val greater than 22 selected the default spell
     */
    while (spellval > 24 && rn2(25))
        spellval = rn2(spellval);

    /* If we're hurt, seriously consider fixing ourselves a priority */
    if ((mtmp->mhp * 4) <= mtmp->mhpmax) {
        spellval = 1;
    }

    switch (spellval) {
    case 24:
    case 23:
        i = rn2(3);
        switch (i) {
            case 2:
                return MGC_FIRE_BOLT;
            case 1:
                return MGC_ICE_BOLT;
            case 0:
            default:
                if (Antimagic || Hallucination)
                    return MGC_PSI_BOLT;
        }
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
        return MGC_AGGRAVATION;
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
        i = rn2(3);
        switch (i) {
            case 2:
                return MGC_FIRE_BOLT;
            case 1:
                return MGC_ICE_BOLT;
            case 0:
            default:
                if (Antimagic || Hallucination)
                    return MGC_PSI_BOLT;
        }
    }
}

/* convert a level based random selection into a specific cleric spell */
STATIC_OVL int
choose_clerical_spell(mtmp, spellnum)
struct monst* mtmp;
int spellnum;
{
    /* for 3.4.3 and earlier, num greater than 13 selected the default spell
     */
    while (spellnum > 15 && rn2(16))
        spellnum = rn2(spellnum);

    /* If we're hurt, seriously consider giving fixing ourselves priority */
    if ((mtmp->mhp * 4) <= mtmp->mhpmax) {
        spellnum = 1;
    }

    switch (spellnum) {
    case 15:
    case 14:
        if (rn2(3))
            return CLC_OPEN_WOUNDS;
        /*FALLTHRU*/
    case 13:
        return CLC_SUMMON_ELM;
    case 12:
        return CLC_GEYSER;
    case 11:
        return CLC_FIRE_PILLAR;
    case 10:
        return CLC_LIGHTNING;
    case 9:
        return CLC_CURSE_ITEMS;
    case 8:
        return CLC_INSECTS;
    case 7:
    case 6:
        return CLC_BLIND_YOU;
    case 5:
        return CLC_VULN_YOU;
    case 4:
        return CLC_PARALYZE;
    case 3:
    case 2:
        return CLC_CONFUSE_YOU;
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
register struct monst *mtmp;
register struct attack *mattk;
boolean thinks_it_foundyou;
boolean foundyou;
{
    int dmg, ml = mtmp->m_lev;
    int ret;
    int spellnum = 0;

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
                    || spell_would_be_useless(mtmp, mattk->adtyp, spellnum)) {
                    if (foundyou)
                        impossible(
                       "spellcasting monster found you and doesn't know it?");
                    return 0;
                }
                break;
            }
        } while (--cnt > 0
                 && spell_would_be_useless(mtmp, mattk->adtyp, spellnum));
        if (cnt == 0)
            return 0;
    }

    /* monster unable to cast spells? */
    if (mtmp->mcan || mtmp->mspec_used || !ml) {
        cursetxt(mtmp, is_undirected_spell(mattk->adtyp, spellnum));
        return 0;
    }

    if (mattk->adtyp == AD_SPEL || mattk->adtyp == AD_CLRC) {
        register struct obj *obj;
        mtmp->mspec_used += spelltimeout(mtmp, objects[spellnum].oc_level);
        if (mtmp->mspec_used < 2)
            mtmp->mspec_used = 2;
        for (obj = mtmp->minvent; obj; obj = obj->nobj)
	    if (obj->oartifact
		&& obj->oartifact == ART_EYE_OF_THE_AETHIOPICA) {
		mtmp->mspec_used = 0;
		break;
	    }
    }

    /* monster can cast spells, but is casting a directed spell at the
       wrong place?  If so, give a message, and return.  Do this *after*
       penalizing mspec_used. */
    if (!foundyou && thinks_it_foundyou
        && !is_undirected_spell(mattk->adtyp, spellnum)) {
        pline("%s casts a spell at %s!",
              canseemon(mtmp) ? Monnam(mtmp) : "Something",
              levl[mtmp->mux][mtmp->muy].typ == WATER ? "empty water"
                                                      : "thin air");
        return 0;
    }

    nomul(0);
    if (rn2(ml * 10) < (mtmp->mconf ? 100 : 10)) { /* fumbled attack */
        if (canseemon(mtmp) && !Deaf)
            pline_The("air crackles around %s.", mon_nam(mtmp));
        return 0;
    }
    if (canspotmon(mtmp) || !is_undirected_spell(mattk->adtyp, spellnum)) {
        pline("%s casts a spell%s!",
              canspotmon(mtmp) ? Monnam(mtmp) : "Something",
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
        pline("You're enveloped in flames.");
        if (how_resistant(FIRE_RES) == 100) {
            shieldeff(u.ux, u.uy);
            pline("But you resist the effects.");
            monstseesu(M_SEEN_FIRE);
            dmg = 0;
	} else {
            dmg = resist_reduce(dmg, FIRE_RES);
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
        pline("You're splashed in acid.");
        if (how_resistant(ACID_RES) == 100) {
            shieldeff(u.ux, u.uy);
            pline("But you resist the effects.");
            monstseesu(M_SEEN_ACID);
            dmg = 0;
        } else {
            dmg = d((int) mtmp->m_lev / 2 + 1, 8);
        }
        break;
    case AD_MAGM:
        You("are hit by a shower of missiles!");
        dmg = d((int) mtmp->m_lev / 2 + 1, 6);
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
            pline("Some missiles bounce off!");
            monstseesu(M_SEEN_MAGR);
            dmg = (dmg + 1) / 2;
        }
	if (Half_spell_damage) { /* stacks, effectively 1/4 damage */
	    dmg = (dmg + 1) / 2;
	}
        break;
    case AD_SPEL: /* wizard spell */
    case AD_CLRC: /* clerical spell */
    {
        if (mattk->adtyp == AD_SPEL)
            cast_wizard_spell(mtmp, dmg, spellnum);
        else
            cast_cleric_spell(mtmp, dmg, spellnum);
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

/* monster wizard and cleric spellcasting functions */
/*
   If dmg is zero, then the monster is not casting at you.
   If the monster is intentionally not casting at you, we have previously
   called spell_would_be_useless() and spellnum should always be a valid
   undirected spell.
   If you modify either of these, be sure to change is_undirected_spell()
   and spell_would_be_useless().
 */
STATIC_OVL
void
cast_wizard_spell(mtmp, dmg, spellnum)
struct monst *mtmp;
int dmg;
int spellnum;
{
    struct obj* oatmp;
    int erodelvl;

    if (dmg == 0 && !is_undirected_spell(AD_SPEL, spellnum)) {
        impossible("cast directed wizard spell (%d) with dmg=0?", spellnum);
        return;
    }

    switch (spellnum) {
    case MGC_DEATH_TOUCH:
        pline("Oh no, %s's using the touch of death!", mhe(mtmp));
        if (nonliving(youmonst.data) || is_demon(youmonst.data)) {
            You("seem no more dead than before.");
        } else {
            if (Hallucination) {
                You("have an out of body experience.");
            } else if (!Antimagic) {
                killer.format = KILLED_BY_AN;
                Strcpy(killer.name, "touch of death");
                done(DIED);
            } else if (Antimagic) {
		dmg = d(8, 6);
		/* Magic resistance in conjunction with half spell damage will cut this in half */
		if (Antimagic && Half_spell_damage) {
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
        break;
    case MGC_CANCELLATION:
        if (m_canseeu(mtmp) && lined_up(mtmp)) {
            if (canseemon(mtmp))
                pline("%s %s a cancellation spell!", Monnam(mtmp),
                      rn2(2) ? "evokes" : "conjures up");
            else if (!Deaf)
                You_hear("a powerful spell being %s.",
                         rn2(2) ? "spoken" : "uttered");
            (void) cancel_monst(&youmonst, (struct obj *) 0, FALSE, TRUE, FALSE);
        }
        dmg = 0;
        break;
    case MGC_ACID_BLAST:
        if (mtmp->iswiz || is_prince(mtmp->data)
            || is_lord(mtmp->data) || mtmp->data->msound == MS_NEMESIS
            || mtmp->data->msound == MS_LEADER) {
            if (m_canseeu(mtmp)) {
                pline("%s douses you in a torrent of acid!", Monnam(mtmp));
                if (how_resistant(ACID_RES) == 100) {
                    shieldeff(u.ux, u.uy);
                    pline("The acid dissipates harmlessly.");
                    monstseesu(M_SEEN_ACID);
                    dmg = 0;
                }
                explode(u.ux, u.uy, AD_ACID - 1, d((mtmp->m_lev / 2) + 4, 8),
                    MON_CASTBALL, EXPL_ACID);
                if (rn2(u.twoweap ? 2 : 3))
                    acid_damage(uwep);
                if (u.twoweap && rn2(2))
                    acid_damage(uswapwep);
                if (rn2(4))
                    erode_armor(&youmonst, ERODE_CORRODE);
            } else {
                if (canspotmon(mtmp)) {
                    pline("%s blasts the %s with %s and curses!", Monnam(mtmp),
                          rn2(2) ? "ceiling" : "floor", "acid");
                } else {
                    You_hear("some cursing!");
                }
            }
        }
        break;
    case MGC_CLONE_WIZ:
        if (mtmp->iswiz && context.no_of_wizards == 1) {
            pline("Double Trouble...");
            clonewiz();
            dmg = 0;
        } else
            impossible("bad wizard cloning?");
        break;
    case MGC_SUMMON_MONS: {
        int count;

        count = nasty(mtmp, FALSE); /* summon something nasty */
        if (mtmp->iswiz) {
            verbalize("Destroy the thief, my pet%s!", plur(count));
        } else {
            const char *mappear = (count == 1) ? "A monster appears"
                                               : "Monsters appear";

            /* messages not quite right if plural monsters created but
               only a single monster is seen */
            if (Invis && !mon_prop(mtmp, SEE_INVIS)
                && (mtmp->mux != u.ux || mtmp->muy != u.uy))
                pline("%s around a spot near you!", mappear);
            else if (Displaced && (mtmp->mux != u.ux || mtmp->muy != u.uy))
                pline("%s around your displaced image!", mappear);
            else
                pline("%s from nowhere!", mappear);
        }
        dmg = 0;
        break;
    }
    case MGC_AGGRAVATION:
        You_feel("that monsters are aware of your presence.");
        aggravate();
        dmg = 0;
        break;
    case MGC_CURSE_ITEMS:
        You_feel("as if you need some help.");
        rndcurse();
        dmg = 0;
        break;
    case MGC_DESTRY_ARMR:
	erodelvl = rnd(3);
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
            monstseesu(M_SEEN_MAGR);
	    erodelvl = 1;
	}
	oatmp = some_armor(&youmonst);
	if (oatmp) {
            if (any_quest_artifact(oatmp)) {
                pline("The %s shines brightly.", xname(oatmp));
                pline("The %s is immune to %s destructive magic.", xname(oatmp),
                      s_suffix(mon_nam(mtmp)));
                return;
            } else if (oatmp->oerodeproof && !any_quest_artifact(oatmp)) {
                if (!Blind) {
		    pline("Your %s glows brown for a moment.", xname(oatmp));
                } else {
                    pline("Your %s briefly emits an odd smell.", xname(oatmp));
                }
		oatmp->oerodeproof = 0;
	    }
	    if (greatest_erosion(oatmp) == MAX_ERODE) {
		destroy_arm(oatmp);
	    } else {
		while (erodelvl-- > 0) {
                if (is_corrodeable(oatmp)) {
                    (void) erode_obj(oatmp, (char *) 0, ERODE_CORRODE, EF_VERBOSE);
                } else if (is_flammable(oatmp)) {
                    (void) erode_obj(oatmp, (char *) 0, ERODE_BURN, EF_VERBOSE);
                } else if (is_glass(oatmp)) {
                    (void) erode_obj(oatmp, (char *) 0, ERODE_FRACTURE, EF_VERBOSE);
                } else if (is_supermaterial(oatmp)) {
                    (void) erode_obj(oatmp, (char *) 0, ERODE_DETERIORATE, EF_VERBOSE);
                } else {
                    (void) erode_obj(oatmp, (char *) 0, ERODE_ROT, EF_VERBOSE);
                    }
		}
	    }
	} else {
	    Your("skin itches.");
        }
        dmg = 0;
        break;
    case MGC_WEAKEN_YOU: /* drain strength */
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
            monstseesu(M_SEEN_MAGR);
            You_feel("momentarily weakened.");
        } else {
            You("suddenly feel weaker!");
            dmg = mtmp->m_lev - 6;
            if (Half_spell_damage)
                dmg = (dmg + 1) / 2;
            losestr(rnd(dmg));
            if (u.uhp < 1)
                done_in_by(mtmp, DIED);
        }
        dmg = 0;
        break;
    case MGC_DISAPPEAR: /* makes self invisible */
        if (!mtmp->minvis && !mtmp->invis_blkd) {
            if (canseemon(mtmp))
                pline("%s suddenly %s!", Monnam(mtmp),
                      !See_invisible ? "disappears" : "becomes transparent");
            mon_set_minvis(mtmp);
            if (cansee(mtmp->mx, mtmp->my) && !canspotmon(mtmp))
                map_invisible(mtmp->mx, mtmp->my);
            dmg = 0;
        } else
            impossible("no reason for monster to cast disappear spell?");
        break;
    case MGC_STUN_YOU:
        if (Antimagic || Free_action) {
            shieldeff(u.ux, u.uy);
            if (!Stunned)
                You_feel("momentarily disoriented.");
            make_stunned(1L, FALSE);
        } else {
            You(Stunned ? "struggle to keep your balance." : "reel...");
            dmg = d(ACURR(A_DEX) < 12 ? 6 : 4, 4);
            if (Half_spell_damage)
                dmg = (dmg + 1) / 2;
            make_stunned((HStun & TIMEOUT) + (long) dmg, FALSE);
        }
        dmg = 0;
        break;
    case MGC_HASTE_SELF:
        mon_adjust_speed(mtmp, 1, (struct obj *) 0);
        dmg = 0;
        break;
    case MGC_CURE_SELF:
        dmg = m_cure_self(mtmp, dmg);
        break;
    case MGC_FIRE_BOLT:
    case MGC_ICE_BOLT:
        /* hotwire these to only go off if the critter can see you
         * to avoid bugs WRT the Eyes and detect monsters */
        if (m_canseeu(mtmp) && lined_up(mtmp)) {
            pline("%s blasts you with %s!", Monnam(mtmp), (spellnum == MGC_FIRE_BOLT) ? "fire" : "ice");
            if (how_resistant((spellnum == MGC_FIRE_BOLT) ? FIRE_RES : COLD_RES) == 100) {
                shieldeff(u.ux, u.uy);
                monstseesu((spellnum == MGC_FIRE_BOLT) ? M_SEEN_FIRE : M_SEEN_COLD);
                pline("But you resist the effects.");
                dmg = 0;
            }
            explode(u.ux, u.uy, (spellnum == MGC_FIRE_BOLT) ? AD_FIRE - 1 : AD_COLD - 1,
                    resist_reduce(d((mtmp->m_lev / 5) + 1, 8), (spellnum == MGC_FIRE_BOLT) ? FIRE_RES : COLD_RES),
                    MON_CASTBALL, (spellnum == MGC_FIRE_BOLT) ? EXPL_FIERY : EXPL_FROSTY);
        } else {
            if (canspotmon(mtmp)) {
                pline("%s blasts the %s with %s and curses!", Monnam(mtmp), rn2(2) ? "ceiling" : "floor",
                      (spellnum == MGC_FIRE_BOLT) ? "fire" : "ice");
            } else {
                You_hear("some cursing!");
            }
        }
        break;
    case MGC_PSI_BOLT:
        /* prior to 3.4.0 Antimagic was setting the damage to 1--this
           made the spell virtually harmless to players with magic res. */
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
        break;
    default:
        impossible("mcastu: invalid magic spell (%d)", spellnum);
        dmg = 0;
        break;
    }

    if (dmg)
        mdamageu(mtmp, dmg);
}

const char* vulntext[] = {
	"chartreuse polka-dot",
	"reddish-orange",
	"purplish-blue",
	"coppery-yellow",
	"greenish-mottled"
};

STATIC_OVL
void
cast_cleric_spell(mtmp, dmg, spellnum)
struct monst *mtmp;
int dmg;
int spellnum;
{
    int aligntype;
    static const char *Moloch = "Moloch";

    if (dmg == 0 && !is_undirected_spell(AD_CLRC, spellnum)) {
        impossible("cast directed cleric spell (%d) with dmg=0?", spellnum);
        return;
    }

    switch (spellnum) {
    case CLC_SUMMON_ELM:
        if (is_demon(mtmp->data))
            return;

        if (mtmp->ispriest) {
	    aligntype = EPRI(mtmp)->shralign;
	} else {
	    aligntype = sgn(mtmp->data->maligntyp);
        }
        if (aligntype == A_NONE) {
            pline("A vassal of %s appears!", Moloch);
                  summon_minion(aligntype, TRUE);
        } else {
	    pline("A servant of %s appears!", aligns[1 - aligntype].noun);
	          summon_minion(aligntype, TRUE);
        }
	break;
    case CLC_GEYSER:
        /* this is physical damage (force not heat),
         * not magical damage or fire damage
         */
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
        break;
    case CLC_FIRE_PILLAR:
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
        break;
    case CLC_LIGHTNING: {
        boolean reflects;

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
        if (dmg && Half_spell_damage) {
            dmg = (dmg + 1) / 2;
        }
        if (!reflects) {
            destroy_item(WAND_CLASS, AD_ELEC);
            destroy_item(RING_CLASS, AD_ELEC);
            (void) flashburn((long) rnd(100));
        }
        break;
    }
    case CLC_CURSE_ITEMS:
        You_feel("as if you need some help.");
        rndcurse();
        dmg = 0;
        break;
    case CLC_INSECTS:
        if (is_demon(mtmp->data))
            return;

        /* Try for insects, and if there are none
           left, go for (sticks to) snakes.  -3. */
        struct permonst *pm = mkclass(S_ANT, 0);
        struct monst *mtmp2 = (struct monst *) 0;
        char let = (pm ? S_ANT : S_SNAKE);
        boolean success = FALSE, seecaster;
        int i, quan, oldseen, newseen;
        coord bypos;
        const char *fmt;

        oldseen = monster_census(TRUE);
        quan = (mtmp->m_lev < 2) ? 1 : rnd((int) mtmp->m_lev / 2);
        if (quan < 3)
            quan = 3;
        for (i = 0; i <= quan; i++) {
            if (!enexto(&bypos, mtmp->mux, mtmp->muy, mtmp->data))
                break;
            if ((pm = mkclass(let, 0)) != 0
                && (mtmp2 = makemon(pm, bypos.x, bypos.y, MM_ANGRY)) != 0) {
                success = TRUE;
                mtmp2->msleeping = mtmp2->mpeaceful = mtmp2->mtame = 0;
                set_malign(mtmp2);
            }
        }
        newseen = monster_census(TRUE);

        /* not canspotmon(), which includes unseen things sensed via warning
         */
        seecaster = canseemon(mtmp) || tp_sensemon(mtmp) || Detect_monsters;

        fmt = 0;
        if (!seecaster) {
            char *arg; /* [not const: upstart(N==1 ? an() : makeplural())] */
            const char *what = (let == S_SNAKE) ? "snake" : "insect";

            if (newseen <= oldseen || Unaware) {
                /* unseen caster fails or summons unseen critters,
                   or unconscious hero ("You dream that you hear...") */
                You_hear("someone summoning %s.", makeplural(what));
            } else {
                /* unseen caster summoned seen critter(s) */
                arg = (newseen == oldseen + 1) ? an(what) : makeplural(what);
                if (!Deaf)
                    You_hear("someone summoning something, and %s %s.", arg,
                             vtense(arg, "appear"));
                else
                    pline("%s %s.", upstart(arg), vtense(arg, "appear"));
            }

        /* seen caster, possibly producing unseen--or just one--critters;
           hero is told what the caster is doing and doesn't necessarily
           observe complete accuracy of that caster's results (in other
           words, no need to fuss with visibility or singularization;
           player is told what's happening even if hero is unconscious) */
        } else if (!success)
            fmt = "%s casts at a clump of sticks, but nothing happens.";
        else if (let == S_SNAKE)
            fmt = "%s transforms a clump of sticks into snakes!";
        else if (Invis && !mon_prop(mtmp, SEE_INVIS)
                 && (mtmp->mux != u.ux || mtmp->muy != u.uy))
            fmt = "%s summons insects around a spot near you!";
        else if (Displaced && (mtmp->mux != u.ux || mtmp->muy != u.uy))
            fmt = "%s summons insects around your displaced image!";
        else
            fmt = "%s summons insects!";
        if (fmt)
            pline(fmt, Monnam(mtmp));

        dmg = 0;
        break;
    case CLC_BLIND_YOU:
        /* note: resists_blnd() doesn't apply here */
        if (!Blinded) {
            int num_eyes = eyecount(youmonst.data);
            pline("Scales cover your %s!", (num_eyes == 1)
                                               ? body_part(EYE)
                                               : makeplural(body_part(EYE)));
            make_blinded(Half_spell_damage ? 100L : 200L, FALSE);
            if (!Blind)
                Your1(vision_clears);
            dmg = 0;
        } else
            impossible("no reason for monster to cast blindness spell?");
        break;
    case CLC_PARALYZE:
        if (Antimagic || Free_action) {
            shieldeff(u.ux, u.uy);
            if (multi >= 0)
                You("stiffen briefly.");
            nomul(-1);
            multi_reason = "paralyzed by a monster";
        } else {
            if (multi >= 0)
                You("are frozen in place!");
            dmg = 4 + (int) mtmp->m_lev;
            if (Half_spell_damage)
                dmg = (dmg + 1) / 2;
            nomul(-dmg);
            multi_reason = "paralyzed by a monster";
        }
        nomovemsg = 0;
        dmg = 0;
        break;
    case CLC_CONFUSE_YOU:
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
            monstseesu(M_SEEN_MAGR);
            You_feel("momentarily dizzy.");
        } else {
            boolean oldprop = !!Confusion;

            dmg = (int) mtmp->m_lev;
            if (Half_spell_damage)
                dmg = (dmg + 1) / 2;
            make_confused(HConfusion + dmg, TRUE);
            if (Hallucination)
                You_feel("%s!", oldprop ? "trippier" : "trippy");
            else
                You_feel("%sconfused!", oldprop ? "more " : "");
        }
        dmg = 0;
        break;
    case CLC_CURE_SELF:
        dmg = m_cure_self(mtmp, dmg);
        break;
    case CLC_VULN_YOU:
	dmg = rnd(4);
	pline("A %s film oozes over your skin!", Blind ? "slimy" : vulntext[dmg]);
	switch (dmg) {
	    case 1:
		if (Vulnerable_fire) return;
		    incr_itimeout(&HVulnerable_fire, rnd(100) + 150);
		    You_feel("more inflammable.");
		break;
	    case 2:
		if (Vulnerable_cold) return;
		    incr_itimeout(&HVulnerable_cold, rnd(100) + 150);
		    You_feel("extremely chilly.");
		break;
	    case 3:
		if (Vulnerable_elec) return;
		    incr_itimeout(&HVulnerable_elec, rnd(100) + 150);
		    You_feel("overly conductive.");
		break;
	    case 4:
		if (Vulnerable_acid) return;
		    incr_itimeout(&HVulnerable_acid, rnd(100) + 150);
		    You_feel("easily corrodable.");
		break;
	    default:
		break;
	}
	break;
    case CLC_OPEN_WOUNDS:
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
        break;
    default:
        impossible("mcastu: invalid clerical spell (%d)", spellnum);
        dmg = 0;
        break;
    }

    if (dmg)
        mdamageu(mtmp, dmg);
}

STATIC_DCL
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
            return TRUE;
        default:
            break;
        }
    } else if (adtyp == AD_CLRC) {
        switch (spellnum) {
        case CLC_INSECTS:
        case CLC_CURE_SELF:
            return TRUE;
        default:
            break;
        }
    }
    return FALSE;
}

STATIC_DCL
boolean
mspell_would_be_useless(mtmp, mdef, adtyp, spellnum)
struct monst *mtmp;
struct monst *mdef;
unsigned int adtyp;
int spellnum;
{
    if (adtyp == AD_SPEL) {
        /* aggravate monsters, etc. won't be cast by peaceful monsters */
        if (mtmp->mpeaceful
            && (spellnum == MGC_AGGRAVATION || spellnum == MGC_SUMMON_MONS
                || spellnum == MGC_CLONE_WIZ))
            return TRUE;
      	/* haste self when already fast */
      	if (mtmp->permspeed == MFAST && spellnum == MGC_HASTE_SELF)
      	    return TRUE;
      	/* invisibility when already invisible */
      	if ((mtmp->minvis || mtmp->invis_blkd) && spellnum == MGC_DISAPPEAR)
      	    return TRUE;
      	/* healing when already healed */
      	if (mtmp->mhp == mtmp->mhpmax && spellnum == MGC_CURE_SELF)
      	    return TRUE;
      	/* don't summon monsters if it doesn't think you're around */
      	if ((!mtmp->iswiz || context.no_of_wizards > 1)
      	    && spellnum == MGC_CLONE_WIZ)
      	    return TRUE;
        /* Don't try to destroy armor if none is being worn */
        if (!wearing_armor() && spellnum == MGC_DESTRY_ARMR) {
            return TRUE;
        }
        /* Don't waste time zapping resisted spells at the player,
         * and don't blast ourselves with our own explosions */
        if ((m_seenres(mtmp, M_SEEN_FIRE) || distu(mtmp->mx, mtmp->my) < 2)
            && spellnum == MGC_FIRE_BOLT) {
            return TRUE;
        }
        if ((m_seenres(mtmp, M_SEEN_COLD) || distu(mtmp->mx, mtmp->my) < 2)
            && spellnum == MGC_ICE_BOLT) {
            return TRUE;
        }
        if ((m_seenres(mtmp, M_SEEN_ACID) || distu(mtmp->mx, mtmp->my) < 2)
            && spellnum == MGC_ACID_BLAST) {
            return TRUE;
        }
        if ((spellnum == MGC_ICE_BOLT || spellnum == MGC_FIRE_BOLT
            || spellnum == MGC_ACID_BLAST || spellnum == MGC_CANCELLATION)
            && mtmp->mpeaceful) {
            return TRUE;
        }
     } else if (adtyp == AD_CLRC) {
      	/* healing when already healed */
      	if (mtmp->mhp == mtmp->mhpmax && spellnum == CLC_CURE_SELF)
      	    return TRUE;
      	/* blindness spell on blinded player */
      	if ((!haseyes(mdef->data) || mdef->mblinded) && spellnum == CLC_BLIND_YOU)
      	    return TRUE;
        }
    return FALSE;
}

STATIC_DCL
boolean
uspell_would_be_useless(adtyp, spellnum)
unsigned int adtyp;
int spellnum;
{
    if (adtyp == AD_SPEL) {
      	/* aggravate monsters, etc. won't be cast by peaceful monsters */
      	if (spellnum == MGC_CLONE_WIZ)
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
    }
    return FALSE;
}

/* Some spells are useless under some circumstances. */
STATIC_DCL
boolean
spell_would_be_useless(mtmp, adtyp, spellnum)
struct monst *mtmp;
unsigned int adtyp;
int spellnum;
{
    /* Some spells don't require the player to really be there and can be cast
     * by the monster when you're invisible, yet still shouldn't be cast when
     * the monster doesn't even think you're there.
     * This check isn't quite right because it always uses your real position.
     * We really want something like "if the monster could see mux, muy".
     */
    boolean mcouldseeu = couldsee(mtmp->mx, mtmp->my);

    if (adtyp == AD_SPEL) {
        /* aggravate monsters, etc. won't be cast by peaceful monsters */
        if (mtmp->mpeaceful
            && (spellnum == MGC_AGGRAVATION || spellnum == MGC_SUMMON_MONS
                || spellnum == MGC_CLONE_WIZ))
            return TRUE;
        /* haste self when already fast */
        if (mtmp->permspeed == MFAST && spellnum == MGC_HASTE_SELF)
            return TRUE;
        /* invisibility when already invisible */
        if ((mtmp->minvis || mtmp->invis_blkd) && spellnum == MGC_DISAPPEAR)
            return TRUE;
        /* peaceful monster won't cast invisibility if you can't see
           invisible,
           same as when monsters drink potions of invisibility.  This doesn't
           really make a lot of sense, but lets the player avoid hitting
           peaceful monsters by mistake */
        if (mtmp->mpeaceful && !See_invisible && spellnum == MGC_DISAPPEAR)
            return TRUE;
        /* healing when already healed */
        if (mtmp->mhp == mtmp->mhpmax && spellnum == MGC_CURE_SELF)
            return TRUE;
        /* don't summon monsters if it doesn't think you're around */
        if (!mcouldseeu && (spellnum == MGC_SUMMON_MONS
                            || (!mtmp->iswiz && spellnum == MGC_CLONE_WIZ)))
            return TRUE;
        if ((!mtmp->iswiz || context.no_of_wizards > 1)
            && spellnum == MGC_CLONE_WIZ)
            return TRUE;
        /* aggravation (global wakeup) when everyone is already active */
        if (spellnum == MGC_AGGRAVATION) {
            /* if nothing needs to be awakened then this spell is useless
               but caster might not realize that [chance to pick it then
               must be very small otherwise caller's many retry attempts
               will eventually end up picking it too often] */
            if (!has_aggravatables(mtmp))
                return rn2(100) ? TRUE : FALSE;
        }
        /* Don't try to destroy armor if none is being worn */
        if (!wearing_armor() && spellnum == MGC_DESTRY_ARMR) {
            return TRUE;
        }
        if ((m_seenres(mtmp, M_SEEN_FIRE) || distu(mtmp->mx, mtmp->my) < 2)
            && spellnum == MGC_FIRE_BOLT) {
            return TRUE;
        }
        if ((m_seenres(mtmp, M_SEEN_COLD) || distu(mtmp->mx, mtmp->my) < 2)
            && spellnum == MGC_ICE_BOLT) {
            return TRUE;
        }
        if ((m_seenres(mtmp, M_SEEN_ACID) || distu(mtmp->mx, mtmp->my) < 2)
            && spellnum == MGC_ACID_BLAST) {
            return TRUE;
        }
        if ((spellnum == MGC_ICE_BOLT || spellnum == MGC_FIRE_BOLT
            || spellnum == MGC_ACID_BLAST || spellnum == MGC_CANCELLATION)
            && mtmp->mpeaceful) {
            return TRUE;
        }
    } else if (adtyp == AD_CLRC) {
        /* summon insects/sticks to snakes won't be cast by peaceful monsters
         */
        if (mtmp->mpeaceful && spellnum == CLC_INSECTS)
            return TRUE;
        /* healing when already healed */
        if (mtmp->mhp == mtmp->mhpmax && spellnum == CLC_CURE_SELF)
            return TRUE;
        /* don't summon insects if it doesn't think you're around */
        if (!mcouldseeu && spellnum == CLC_INSECTS)
            return TRUE;
        /* blindness spell on blinded player */
        if (Blinded && spellnum == CLC_BLIND_YOU)
            return TRUE;
    }
    return FALSE;
}

/* convert 1..10 to 0..9; add 10 for second group (spell casting) */
#define ad_to_typ(k) (10 + (int) k - 1)

/* monster uses spell (ranged) */
int
buzzmu(mtmp, mattk)
register struct monst *mtmp;
register struct attack *mattk;
{
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
        if (mattk->adtyp && (mattk->adtyp < 11)) { /* no cf unsigned >0 */
            if (canseemon(mtmp))
                pline("%s zaps you with a %s!", Monnam(mtmp),
                      flash_types[ad_to_typ(mattk->adtyp)]);
            buzz(-ad_to_typ(mattk->adtyp), (int) mattk->damn, mtmp->mx,
                 mtmp->my, sgn(tbx), sgn(tby));
        } else
            impossible("Monster spell %d cast", mattk->adtyp - 1);
    }
    return 1;
}

/* return values:
* 2: target died
* 1: successful spell
* 0: unsuccessful spell
*/
int
castmm(mtmp, mdef, mattk)
register struct monst *mtmp;
register struct monst *mdef;
register struct attack *mattk;
{
    int dmg, ml = mtmp->m_lev;
    int ret;
    int spellnum = 0;

    if ((mattk->adtyp == AD_SPEL || mattk->adtyp == AD_CLRC) && ml) {
   	int cnt = 40;

   	do {
   	    spellnum = rn2(ml);
            if (mattk->adtyp == AD_SPEL)
                spellnum = choose_magic_spell(mtmp, spellnum);
            else
                spellnum = choose_clerical_spell(mtmp, spellnum);
        /* not trying to attack?  don't allow directed spells */
   	} while (--cnt > 0 &&
   		 mspell_would_be_useless(mtmp, mdef,
   		                         mattk->adtyp, spellnum));
   	    if (cnt == 0)
            return 0;
   	}

   	/* monster unable to cast spells? */
   	if (mtmp->mcan || mtmp->mspec_used || !ml) {
   	    if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my)) {
                char buf[BUFSZ];
   		Sprintf(buf, "%s", Monnam(mtmp));

   		if (is_undirected_spell(mattk->adtyp, spellnum))
   	            pline("%s points all around, then curses.", buf);
   		else
   	            pline("%s points at %s, then curses.",
   		          buf, mon_nam(mdef));

   	    } else if ((!(moves % 4) || !rn2(4))) {
   	        if (!Deaf) Norep("You hear a mumbled curse.");
   	    }
   	    return 0;
   	}

   	if (mattk->adtyp == AD_SPEL || mattk->adtyp == AD_CLRC) {
            register struct obj *obj;
            mtmp->mspec_used += spelltimeout(mtmp, objects[spellnum].oc_level);
            if (mtmp->mspec_used < 2)
                mtmp->mspec_used = 2;
            for (obj = mtmp->minvent; obj; obj = obj->nobj)
                if (obj->oartifact
                    && obj->oartifact == ART_EYE_OF_THE_AETHIOPICA) {
                    mtmp->mspec_used = 0;
                    break;
            }

   	}

   	if (rn2(ml*10) < (mtmp->mconf ? 100 : 20)) {	/* fumbled attack */
   	    if (canseemon(mtmp))
   		pline_The("air crackles around %s.", mon_nam(mtmp));
   	    return 0;
   	}
   	if (cansee(mtmp->mx, mtmp->my) || canseemon(mtmp)
   	    || (!is_undirected_spell(mattk->adtyp, spellnum)
   	    && (cansee(mdef->mx, mdef->my) || canseemon(mdef)))) {
            char buf[BUFSZ];
   	    Sprintf(buf, " at ");
   	    Strcat(buf, mon_nam(mdef));
   	    pline("%s casts a spell%s!",
   		  canspotmon(mtmp) ? Monnam(mtmp) : "Something",
   		  is_undirected_spell(mattk->adtyp, spellnum) ? "" : buf);
   	}

   	if (mattk->damd)
   	    dmg = d((int)((ml/2) + mattk->damn), (int)mattk->damd);
   	else
            dmg = d((int)((ml/2) + 1), 6);

   	ret = 1;

   	switch (mattk->adtyp) {
   	    case AD_FIRE:
   	        if (canspotmon(mdef)) {
                is_demon(mtmp->data)
   		         ? pline("%s is enveloped in flames.", Monnam(mdef))
                         : pline("%s is enveloped in a pillar of hellfire!", Monnam(mdef));
                }
                if (is_demon(mtmp->data) && resists_fire(mdef)) {
                    dmg = (dmg + 1) / 2;
                    break;
                }
       	        if(resists_fire(mdef)) {
         	   shieldeff(mdef->mx, mdef->my);
         	   if (canspotmon(mdef))
         	       pline("But %s resists the effects.", mhe(mdef));
                   dmg = 0;
   	        }
   	        break;
   	    case AD_COLD:
   	        if (canspotmon(mdef))
   	  	    pline("%s is covered in frost.", Monnam(mdef));
       		  if (resists_cold(mdef)) {
         	      shieldeff(mdef->mx, mdef->my);
         	      if (canspotmon(mdef))
         	          pline("But %s resists the effects.", mhe(mdef));
                      dmg = 0;
       	          }
                break;
            case AD_ACID:
                if (canspotmon(mdef))
                    pline("%s is covered in acid.", Monnam(mdef));
                if (resists_acid(mdef)) {
                    shieldeff(mdef->mx, mdef->my);
                    if (canspotmon(mdef))
                        pline("But %s resists the effects.", mhe(mdef));
                    dmg = 0;
                }
                break;
   	    case AD_MAGM:
                if (canspotmon(mdef))
                    pline("%s is hit by a shower of missiles!", Monnam(mdef));
                dmg = d((int)mtmp->m_lev / 2 + 1, 6);
                if (resists_magm(mdef)) {
           	    shieldeff(mdef->mx, mdef->my);
           	    if (canspotmon(mdef))
                        pline("Some missiles bounce off!");
           	    dmg = (dmg + 1) / 2;
                }
                if (Half_spell_damage) {
                    dmg = (dmg + 1) / 2;
                }
                break;
   	    case AD_SPEL: /* wizard spell */
   	    case AD_CLRC: /* clerical spell */
   	    {
   	        /*aggravation is a special case;
                 * it's undirected but should still target the
                 * victim so as to aggravate you */
   	        if (is_undirected_spell(mattk->adtyp, spellnum)
                    && (mattk->adtyp != AD_SPEL
                    || (spellnum != MGC_AGGRAVATION
                    && spellnum != MGC_SUMMON_MONS))) {
     		    if (mattk->adtyp == AD_SPEL)
     		        cast_wizard_spell(mtmp, dmg, spellnum);
     		    else
     		        cast_cleric_spell(mtmp, dmg, spellnum);
                } else if (mattk->adtyp == AD_SPEL)
       		    ucast_wizard_spell(mtmp, mdef, dmg, spellnum);
                else
       		    ucast_cleric_spell(mtmp, mdef, dmg, spellnum);
         	dmg = 0; /* done by the spell casting functions */
                break;
   	    }
   	}
   	if (dmg > 0 && mdef->mhp > 0) {
            mdef->mhp -= dmg;
 	    if (mdef->mhp < 1) monkilled(mdef, "", mattk->adtyp);
   	}
   	if (mdef && mdef->mhp < 1)
        return 2;
        return (ret);
}

/* return values:
* 2: target died
* 1: successful spell
* 0: unsuccessful spell
*/
int
castum(mtmp, mattk)
register struct monst *mtmp;
register struct attack *mattk;
{
    int dmg, ml = mons[u.umonnum].mlevel;
    int ret;
    int spellnum = 0;
    boolean directed = FALSE;

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

    } while (--cnt > 0 &&
   	     ((!mtmp && !is_undirected_spell(mattk->adtyp, spellnum))
   	      || uspell_would_be_useless(mattk->adtyp, spellnum)));
        if (cnt == 0) {
   	    You("have no spells to cast right now!");
        return 0;
   	}
    }

    if (spellnum == MGC_AGGRAVATION && !mtmp) {
   	/* choose a random monster on the level */
   	int j = 0, k = 0;
   	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
   	     if (!mtmp->mtame && !mtmp->mpeaceful) j++;
   	if (j > 0) {
   	    k = rn2(j);
   	    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
   	         if (!mtmp->mtame && !mtmp->mpeaceful)
   		     if (--k < 0)
            break;
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

    if (rn2(ml*10) < (Confusion ? 100 : 20)) {	/* fumbled attack */
  	pline_The("air crackles around you.");
   	return 0;
    }

    You("cast a spell%s%s!", directed ? " at " : "",
	directed ? mon_nam(mtmp) : "");

    /* As these are spells, the damage is related to the level
     * of the monster casting the spell. */
    if (mattk->damd)
   	dmg = d((int)((ml/2) + mattk->damn), (int)mattk->damd);
    else
        dmg = d((int)((ml/2) + 1), 6);

    ret = 1;

    switch (mattk->adtyp) {
        case AD_FIRE:
            pline("%s is enveloped in flames.", Monnam(mtmp));
            if (resists_fire(mtmp)) {
                shieldeff(mtmp->mx, mtmp->my);
           	pline("But %s resists the effects.", mhe(mtmp));
            dmg = 0;
            }
            break;
   	case AD_COLD:
            pline("%s is covered in frost.", Monnam(mtmp));
            if (resists_cold(mtmp)) {
         	shieldeff(mtmp->mx, mtmp->my);
         	pline("But %s resists the effects.", mhe(mtmp));
            dmg = 0;
            }
            break;
        case AD_ACID:
            pline("%s is covered in acid.", Monnam(mtmp));
            if (resists_acid(mtmp)) {
                shieldeff(mtmp->mx, mtmp->my);
                pline("But %s resists the effects.", mhe(mtmp));
            dmg = 0;
            }
            break;
   	case AD_MAGM:
            pline("%s is hit by a shower of missiles!", Monnam(mtmp));
            dmg = d((int)ml/2 + 1, 6);
            if (resists_magm(mtmp)) {
                shieldeff(mtmp->mx, mtmp->my);
                pline("Some missiles bounce off!");
            dmg = (dmg + 1) / 2;
            }
            if (Half_spell_damage) {
                dmg = (dmg + 1) / 2;
            }
            break;
   	case AD_SPEL: /* wizard spell */
   	case AD_CLRC: /* clerical spell */
   	    {
            if (mattk->adtyp == AD_SPEL)
                ucast_wizard_spell(&youmonst, mtmp, dmg, spellnum);
            else
                ucast_cleric_spell(&youmonst, mtmp, dmg, spellnum);
            dmg = 0; /* done by the spell casting functions */
            break;
   	    }
    }

    if (mtmp && dmg > 0 && mtmp->mhp > 0) {
   	mtmp->mhp -= dmg;
   	if (mtmp->mhp < 1) killed(mtmp);
    }
    if (mtmp && mtmp->mhp < 1)
        return 2;
    return (ret);
}

 extern NEARDATA const int nasties[];

/* monster wizard and cleric spellcasting functions */
/*
  If dmg is zero, then the monster is not casting at you.
  If the monster is intentionally not casting at you, we have previously
  called spell_would_be_useless() and spellnum should always be a valid
  undirected spell.
  If you modify either of these, be sure to change is_undirected_spell()
  and spell_would_be_useless().
*/
STATIC_OVL
void
ucast_wizard_spell(mattk, mtmp, dmg, spellnum)
struct monst *mattk;
struct monst *mtmp;
int dmg;
int spellnum;
{
    boolean resisted = FALSE;
    boolean yours = (mattk == &youmonst);

    if (dmg == 0 && !is_undirected_spell(AD_SPEL, spellnum)) {
 	impossible("cast directed wizard spell (%d) with dmg=0?", spellnum);
 	return;
    }

    if (mtmp && mtmp->mhp < 1) {
        impossible("monster already dead?");
 	return;
    }

    switch (spellnum) {
    case MGC_DEATH_TOUCH:
       	if (!mtmp || mtmp->mhp < 1) {
       	    impossible("touch of death with no mtmp");
       	    return;
       	}
       	if (yours)
       	    pline("You're using the touch of death!");
       	else if (canseemon(mattk)) {
       	    char buf[BUFSZ];
       	    Sprintf(buf, "%s%s", mtmp->mtame ? "Oh no, " : "",
       	                         mhe(mattk));
       	    if (!mtmp->mtame)
       	        *buf = highc(*buf);

       	    pline("%s's using the touch of death!", buf);
       	}

       	if (nonliving(mtmp->data) || is_demon(mtmp->data)) {
       	    if (yours || canseemon(mtmp))
       	        pline("%s seems no more dead than before.", Monnam(mtmp));
       	} else if (!(resisted = resist(mtmp, 0, 0, FALSE)) ||
       	           rn2(mons[u.umonnum].mlevel) > 12) {
                   mtmp->mhp = -1;
       	    if (yours)
                killed(mtmp);
       	    else monkilled(mtmp, "", AD_SPEL);
       	    return;
       	} else {
       	    if (resisted)
                shieldeff(mtmp->mx, mtmp->my);
       	    if (yours || canseemon(mtmp)) {
       	        if (mtmp->mtame)
       		    pline("Lucky for %s, it didn't work!", mon_nam(mtmp));
       	        else
       	            pline("Well. That didn't work...");
            }
       	}
       	dmg = 0;
       	break;
    case MGC_CANCELLATION:
        if (!mtmp) {
            impossible("cancellation with no mtmp");
            return;
        }
        if (yours)
            You("%s a cancellation spell!",
                rn2(2) ? "evoke" : "conjure up");
        else if (canseemon(mattk))
            pline("%s %s a cancellation spell!", Monnam(mattk),
                  rn2(2) ? "evokes" : "conjures up");
        (void) cancel_monst(mtmp, (struct obj *) 0, FALSE, TRUE, FALSE);
        break;
    case MGC_ACID_BLAST:
        if (!mtmp || mtmp->mhp < 1) {
            impossible("acid blast with no mtmp");
            return;
        }
        if (yours || canseemon(mtmp))
            You("douse %s in a torrent of acid!", mon_nam(mtmp));
        if (resists_acid(mtmp)) {
            shieldeff(mtmp->mx, mtmp->my);
            pline("But the acid dissipates harmlessly.");
            dmg = 0;
        }
        explode(mtmp->mx, mtmp->my, AD_ACID - 1, d((mtmp->m_lev / 2) + 4, 8),
                WAND_CLASS, EXPL_ACID);
        if (rn2(4))
            erode_armor(mtmp, ERODE_CORRODE);
        break;
    case MGC_SUMMON_MONS: {
     	int count = 0;
        register struct monst *mpet;

        if (!rn2(10) && Inhell) {
     	    if (yours) demonpet();
        else msummon(mattk);
     	} else {
     	    register int i, j;
            int makeindex, tmp = (u.ulevel > 3) ? u.ulevel / 3 : 1;
     	    coord bypos;

     	    if (mtmp)
     	        bypos.x = mtmp->mx, bypos.y = mtmp->my;
     	    else if (yours)
     	        bypos.x = u.ux, bypos.y = u.uy;
                 else
     	        bypos.x = mattk->mx, bypos.y = mattk->my;

     	    for (i = rnd(tmp); i > 0; --i)
     	        for(j=0; j<20; j++) {
     	            do {
     	                makeindex = pick_nasty();
     	            } while (attacktype(&mons[makeindex], AT_MAGC) &&
     	                     mons[makeindex].difficulty >= mons[u.umonnum].difficulty);
                        if (yours &&
     		            !enexto(&bypos, u.ux, u.uy, &mons[makeindex]))
     		        continue;
                        if (!yours &&
     		            !enexto(&bypos, mattk->mx, mattk->my, &mons[makeindex]))
     		        continue;
     		    if ((mpet = makemon(&mons[makeindex], bypos.x, bypos.y,
     			(yours || mattk->mtame) ? MM_EDOG : NO_MM_FLAGS)) != 0) {
                        mpet->msleeping = 0;
                        if (yours || mattk->mtame)
     			    initedog(mpet);
     			else if (mattk->mpeaceful)
     			    mpet->mpeaceful = 1;
     			else mpet->mpeaceful = mpet->mtame = 0;
                        set_malign(mpet);
                    } else /* GENOD? */
                        mpet = makemon((struct permonst *)0,
                                       bypos.x, bypos.y, NO_MM_FLAGS);
                    if (mpet && (u.ualign.type == 0
     		        || mpet->data->maligntyp == 0
                        || sgn(mpet->data->maligntyp) == sgn(u.ualign.type))) {
                        count++;
                        break;
                     }
                }

     	    const char *mappear =
     		    (count == 1) ? "A monster appears" : "Monsters appear";

     	    if (yours || canseemon(mtmp))
     	        pline("%s from nowhere!", mappear);
     	}
     	dmg = 0;
     	break;
    }
    case MGC_AGGRAVATION:
       	if (!mtmp || mtmp->mhp < 1) {
       	    You_feel("lonely.");
       	    return;
       	}
       	you_aggravate(mtmp);
       	dmg = 0;
       	break;
    case MGC_CURSE_ITEMS:
       	if (!mtmp || mtmp->mhp < 1) {
       	    impossible("curse spell with no mtmp");
       	    return;
       	}
       	if (yours || canseemon(mtmp))
       	    You_feel("as though %s needs some help.", mon_nam(mtmp));
       	mrndcurse(mtmp);
       	dmg = 0;
       	break;
    case MGC_DESTRY_ARMR:
       	if (!mtmp || mtmp->mhp < 1) {
       	    impossible("destroy spell with no mtmp");
       	    return;
       	}
       	if (resist(mtmp, 0, 0, FALSE)) {
       	    shieldeff(mtmp->mx, mtmp->my);
       	    if (yours || canseemon(mtmp))
       	        pline("A field of force surrounds %s!",
       	               mon_nam(mtmp));
       	} else {
            register struct obj *otmp = some_armor(mtmp);

#define oresist_disintegration(obj) \
 		(objects[obj->otyp].oc_oprop == DISINT_RES || \
 		 obj_resists(obj, 0, 90) || is_quest_artifact(obj))

       	    if (otmp &&
       	        !oresist_disintegration(otmp)) {
       	        pline("%s %s %s!",
                      s_suffix(Monnam(mtmp)),
                      xname(otmp),
                      is_cloak(otmp)  ? "crumbles and turns to dust" :
                      is_shirt(otmp)  ? "crumbles into tiny threads" :
                      is_helmet(otmp) ? "turns to dust and is blown away" :
                      is_gloves(otmp) ? "vanish" :
                      is_boots(otmp)  ? "disintegrate" :
                      is_shield(otmp) ? "crumbles away" :
                                        "turns to dust"
                     );
                obj_extract_self(otmp);
                mtmp->misc_worn_check &= ~otmp->owornmask;
                update_mon_intrinsics(mtmp, otmp, FALSE, TRUE);
                otmp->owornmask = 0L; /* obfree() expects this */
                obfree(otmp, (struct obj *)0);
            }
            else if (yours || canseemon(mtmp))
       	        pline("%s skin looks flaky.", s_suffix(Monnam(mtmp)));
        }
       	dmg = 0;
       	break;
    case MGC_WEAKEN_YOU:		/* drain strength */
       	if (!mtmp || mtmp->mhp < 1) {
       	    impossible("weaken spell with no mtmp");
       	    return;
       	}
       	if (resist(mtmp, 0, 0, FALSE)) {
       	    shieldeff(mtmp->mx, mtmp->my);
       	    pline("%s looks momentarily weakened.", Monnam(mtmp));
       	} else {
       	    if (mtmp->mhp < 1) {
       	        impossible("trying to drain monster that's already dead");
       		return;
       	    }
       	    if (yours || canseemon(mtmp))
       	        pline("%s suddenly seems weaker!", Monnam(mtmp));
                   /* monsters don't have strength, so drain max hp instead */
       	    mtmp->mhpmax -= dmg;
       	    if ((mtmp->mhp -= dmg) <= 0) {
       	        if (yours) killed(mtmp);
       		else monkilled(mtmp, "", AD_SPEL);
            }
       	}
       	dmg = 0;
       	break;
    case MGC_DISAPPEAR:		/* makes self invisible */
        if (!yours) {
       	    impossible("ucast disappear but not yours?");
       	    return;
 	}
       	if (!(HInvis & INTRINSIC)) {
       	    HInvis |= FROMOUTSIDE;
       	    if (!Blind && !BInvis) self_invis_message();
       	    dmg = 0;
       	} else
       	    impossible("no reason for player to cast disappear spell?");
       	break;
    case MGC_STUN_YOU:
       	if (!mtmp || mtmp->mhp < 1) {
       	    impossible("stun spell with no mtmp");
       	    return;
       	}
       	if (resist(mtmp, 0, 0, FALSE)) {
       	    shieldeff(mtmp->mx, mtmp->my);
       	    if (yours || canseemon(mtmp))
       	        pline("%s seems momentarily disoriented.", Monnam(mtmp));
       	} else {
       	    if (yours || canseemon(mtmp)) {
       	        if (mtmp->mstun)
       	            pline("%s struggles to keep %s balance.",
       	 	          Monnam(mtmp), mhis(mtmp));
                else
       	            pline("%s reels...", Monnam(mtmp));
       	    }
       	    mtmp->mstun = 1;
       	}
       	dmg = 0;
       	break;
    case MGC_HASTE_SELF:
        if (!yours) {
       	    impossible("ucast haste but not yours?");
       	    return;
       	}
        if (!(HFast & INTRINSIC))
 	    You("are suddenly moving faster.");
 	    HFast |= INTRINSIC;
 	    dmg = 0;
 	break;
    case MGC_CURE_SELF:
        if (!yours)
            impossible("ucast healing but not yours?");
        else if (u.mh < u.mhmax) {
       	    You("feel better.");
       	    if ((u.mh += d(3, 6)) > u.mhmax)
       		      u.mh = u.mhmax;
       	    context.botl = 1;
       	    dmg = 0;
        }
        break;
    case MGC_FIRE_BOLT:
    case MGC_ICE_BOLT:
        if (!mtmp || mtmp->mhp < 1) {
            impossible("bolt spell with no mtmp");
            return;
        }
        if (yours || canseemon(mtmp)) {
            You("blast %s with %s!", mon_nam(mtmp), (spellnum == MGC_FIRE_BOLT) ? "fire" : "ice");
            if (spellnum == MGC_FIRE_BOLT && resists_fire(mtmp)) {
                shieldeff(mtmp->mx, mtmp->my);
                pline("But %s seems unaffected by the fire.", mon_nam(mtmp));
                dmg = 0;
            } else if (spellnum == MGC_ICE_BOLT && resists_cold(mtmp)) {
                shieldeff(mtmp->mx, mtmp->my);
                pline("But %s seems unaffected by the cold.", mon_nam(mtmp));
                dmg = 0;
            }
            explode(mtmp->mx, mtmp->my, (spellnum == MGC_FIRE_BOLT) ? AD_FIRE - 1 : AD_COLD - 1,
                    d((mtmp->m_lev / 5) + 1, 8), WAND_CLASS, 1);
            if (spellnum == MGC_FIRE_BOLT) {
                (void) burnarmor(mtmp);
                destroy_mitem(mtmp, SCROLL_CLASS, AD_FIRE);
                destroy_mitem(mtmp, POTION_CLASS, AD_FIRE);
                destroy_mitem(mtmp, SPBOOK_CLASS, AD_FIRE);
                (void) burn_floor_objects(mtmp->mx, mtmp->my, TRUE, FALSE);
            }
        }
        break;
    case MGC_PSI_BOLT:
       	if (!mtmp || mtmp->mhp < 1) {
       	    impossible("psibolt spell with no mtmp");
       	    return;
       	}
       	if (resist(mtmp, 0, 0, FALSE)) {
       	    shieldeff(mtmp->mx, mtmp->my);
       	    dmg = (dmg + 1) / 2;
       	}
       	if (yours || canseemon(mtmp))
       	    pline("%s winces%s", Monnam(mtmp), (dmg <= 5) ? "." : "!");
       	break;
    default:
       	impossible("ucastm: invalid magic spell (%d)", spellnum);
       	dmg = 0;
       	break;
    }

    if (dmg > 0 && mtmp && mtmp->mhp > 0) {
        mtmp->mhp -= dmg;
        if (mtmp->mhp < 1) {
 	    if (yours) killed(mtmp);
 	    else monkilled(mtmp, "", AD_SPEL);
 	}
    }
}

STATIC_OVL
void
ucast_cleric_spell(mattk, mtmp, dmg, spellnum)
struct monst *mattk;
struct monst *mtmp;
int dmg;
int spellnum;
{
    int aligntype;
    static const char *Moloch = "Moloch";
    boolean yours = (mattk == &youmonst);

    if (dmg == 0 && !is_undirected_spell(AD_CLRC, spellnum)) {
       	impossible("cast directed cleric spell (%d) with dmg=0?", spellnum);
       	return;
    }

    if (mtmp && mtmp->mhp < 1) {
        impossible("monster already dead?");
 	return;
    }

    switch (spellnum) {
    case CLC_SUMMON_ELM:
        if (!mtmp || mtmp->mhp < 1) {
            impossible("summon elemental spell with no mtmp");
            return;
        }

        if (is_demon(mtmp->data))
            return;

        if (mtmp->ispriest) {
            aligntype = EPRI(mtmp)->shralign;
        } else {
            aligntype = sgn(mtmp->data->maligntyp);
        }
        if (aligntype == A_NONE) {
            pline("A vassal of %s appears!", Moloch);
                  summon_minion(aligntype, TRUE);
        } else {
            pline("A servant of %s appears!", aligns[1 - aligntype].noun);
                  summon_minion(aligntype, TRUE);
        }
        break;
    case CLC_GEYSER:
       	/* this is physical damage, not magical damage */
       	if (!mtmp || mtmp->mhp < 1) {
       	    impossible("geyser spell with no mtmp");
       	    return;
       	}
       	if (yours || canseemon(mtmp))
       	    pline("A sudden geyser slams into %s from nowhere!", mon_nam(mtmp));
       	dmg = d(8, 6);
        (void) erode_armor(mtmp, ERODE_RUST);
       	break;
    case CLC_FIRE_PILLAR:
       	if (!mtmp || mtmp->mhp < 1) {
       	    impossible("fire pillar spell with no mtmp");
       	    return;
       	}
       	if (yours || canseemon(mtmp))
       	    pline("A pillar of fire strikes all around %s!", mon_nam(mtmp));
       	if (resists_fire(mtmp)) {
       	    shieldeff(mtmp->mx, mtmp->my);
       	    dmg = 0;
       	} else
       	    dmg = d(8, 6);
       	(void) burnarmor(mtmp);
       	destroy_mitem(mtmp, SCROLL_CLASS, AD_FIRE);
       	destroy_mitem(mtmp, POTION_CLASS, AD_FIRE);
       	destroy_mitem(mtmp, SPBOOK_CLASS, AD_FIRE);
       	(void) burn_floor_objects(mtmp->mx, mtmp->my, TRUE, FALSE);
       	break;
    case CLC_LIGHTNING:
    {
       	boolean reflects;
       	if (!mtmp || mtmp->mhp < 1) {
       	    impossible("lightning spell with no mtmp");
       	    return;
       	}

       	if (yours || canseemon(mtmp))
       	    pline("A bolt of lightning strikes down at %s from above!",
       	          mon_nam(mtmp));
       	reflects = mon_reflects(mtmp, "It bounces off %s %s.");
       	if (reflects || resists_elec(mtmp)) {
       	    shieldeff(u.ux, u.uy);
       	    dmg = 0;
       	    if (reflects)
       		break;
       	} else
       	    dmg = d(8, 6);
       	destroy_mitem(mtmp, WAND_CLASS, AD_ELEC);
       	destroy_mitem(mtmp, RING_CLASS, AD_ELEC);
       	break;
    }
    case CLC_CURSE_ITEMS:
       	if (!mtmp || mtmp->mhp < 1) {
       	    impossible("curse spell with no mtmp");
       	    return;
       	}
       	if (yours || canseemon(mtmp))
       	    You_feel("as though %s needs some help.", mon_nam(mtmp));
       	mrndcurse(mtmp);
       	dmg = 0;
       	break;
    case CLC_INSECTS:
        if (is_demon(mtmp->data))
            return;

        /* Try for insects, and if there are none
           left, go for (sticks to) snakes.  -3. */
        struct permonst *pm = mkclass(S_ANT,0);
        struct monst *mtmp2 = (struct monst *)0;
        char let = (pm ? S_ANT : S_SNAKE);
        boolean success;
        int i;
        coord bypos;
        int quan;

        if (!mtmp || mtmp->mhp < 1) {
            impossible("insect spell with no mtmp");
            return;
        }

        quan = (mons[u.umonnum].mlevel < 2) ? 1 :
         	rnd(mons[u.umonnum].mlevel / 2);
        if (quan < 3) quan = 3;
            success = pm ? TRUE : FALSE;
        for (i = 0; i <= quan; i++) {
            if (!enexto(&bypos, mtmp->mx, mtmp->my, mtmp->data))
         	break;
            if ((pm = mkclass(let,0)) != 0
                && (mtmp2 = makemon(pm, bypos.x, bypos.y, NO_MM_FLAGS)) != 0) {
         	success = TRUE;
         	mtmp2->msleeping = 0;
         	if (yours || mattk->mtame)
         	    (void) tamedog(mtmp2, (struct obj *)0);
         	else if (mattk->mpeaceful)
                    mattk->mpeaceful = 1;
         	else mattk->mpeaceful = 0;

         	set_malign(mtmp2);
            }
        }

        if (yours) {
            if (!success)
         	You("cast at a clump of sticks, but nothing happens.");
            else if (let == S_SNAKE)
         	You("transforms a clump of sticks into snakes!");
            else
         	You("summon insects!");
        } else if (canseemon(mtmp)) {
            if (!success)
         	pline("%s casts at a clump of sticks, but nothing happens.", Monnam(mattk));
            else if (let == S_SNAKE)
         	pline("%s transforms a clump of sticks into snakes!", Monnam(mattk));
            else
         	pline("%s summons insects!", Monnam(mattk));
        }
        dmg = 0;
        break;
    case CLC_BLIND_YOU:
        if (!mtmp || mtmp->mhp < 1) {
            impossible("blindness spell with no mtmp");
            return;
        }
        /* note: resists_blnd() doesn't apply here */
        if (!mtmp->mblinded
            && haseyes(mtmp->data)) {
            if (!resists_blnd(mtmp)) {
         	int num_eyes = eyecount(mtmp->data);
         	pline("Scales cover %s %s!", s_suffix(mon_nam(mtmp)),
         	      (num_eyes == 1) ? "eye" : "eyes");

                mtmp->mblinded = 127;
            }
        dmg = 0;

        } else
            impossible("no reason for monster to cast blindness spell?");
        break;
    case CLC_PARALYZE:
         if (!mtmp || mtmp->mhp < 1) {
       	    impossible("paralysis spell with no mtmp");
       	    return;
       	}
       	if (resist(mtmp, 0, 0, FALSE)) {
       	    shieldeff(mtmp->mx, mtmp->my);
       	    if (yours || canseemon(mtmp))
       	        pline("%s stiffens briefly.", Monnam(mtmp));
       	} else {
       	    if (yours || canseemon(mtmp))
       	        pline("%s is frozen in place!", Monnam(mtmp));
       	    dmg = 4 + mons[u.umonnum].mlevel;
       	    mtmp->mcanmove = 0;
       	    mtmp->mfrozen = dmg;
       	}
       	dmg = 0;
       	break;
    case CLC_CONFUSE_YOU:
        if (!mtmp || mtmp->mhp < 1) {
       	    impossible("confusion spell with no mtmp");
       	    return;
   	}
       	if (resist(mtmp, 0, 0, FALSE)) {
       	    shieldeff(mtmp->mx, mtmp->my);
       	    if (yours || canseemon(mtmp))
       	        pline("%s seems momentarily dizzy.", Monnam(mtmp));
       	} else {
       	    if (yours || canseemon(mtmp))
       	        pline("%s seems %sconfused!", Monnam(mtmp),
       	              mtmp->mconf ? "more " : "");
       	    mtmp->mconf = 1;
       	}
       	dmg = 0;
       	break;
    case CLC_CURE_SELF:
       	if (u.mh < u.mhmax) {
       	    You("feel better.");
       	    /* note: player healing does 6d4; this used to do 1d8 */
       	    if ((u.mh += d(3,6)) > u.mhmax)
       		u.mh = u.mhmax;
       	    context.botl = 1;
       	    dmg = 0;
       	}
       	break;
    case CLC_VULN_YOU:
        if (!mtmp || mtmp->mhp < 1) {
            impossible("vulnerable spell with no mtmp");
            return;
        }

        if (yours || canseemon(mtmp)) {
	    dmg = rnd(4);
	    pline("A %s film oozes over its skin!", Blind ? "slimy" : vulntext[dmg]);
	    switch (dmg) {
	        case 1:
		    if (vulnerable_to(mtmp, AD_FIRE)) return;
		        incr_itimeout(&HVulnerable_fire, rnd(100)+150);
		        pline("%s is more inflammable.", Monnam(mtmp));
		    break;
	        case 2:
		    if (vulnerable_to(mtmp, AD_COLD)) return;
		        incr_itimeout(&HVulnerable_cold, rnd(100)+150);
		        pline("%s is extremely chilly.", Monnam(mtmp));
		    break;
	        case 3:
		    if (vulnerable_to(mtmp, AD_ELEC)) return;
		        incr_itimeout(&HVulnerable_elec, rnd(100)+150);
		        pline("%s is overly conductive.", Monnam(mtmp));
		    break;
	        case 4:
		    if (vulnerable_to(mtmp, AD_ACID)) return;
		        incr_itimeout(&HVulnerable_acid, rnd(100)+150);
		        pline("%s is easily corrodable.", Monnam(mtmp));
		    break;
	        default:
		    break;
            }
	}
	break;
    case CLC_OPEN_WOUNDS:
        if (!mtmp || mtmp->mhp < 1) {
       	    impossible("wound spell with no mtmp");
       	    return;
       	}
       	if (resist(mtmp, 0, 0, FALSE)) {
       	    shieldeff(mtmp->mx, mtmp->my);
       	    dmg = (dmg + 1) / 2;
       	}
       	/* not canseemon; if you can't see it you don't know it was wounded */
       	if (yours) {
       	    if (dmg <= 5)
       	        pline("%s looks itchy!", Monnam(mtmp));
       	    else if (dmg <= 10)
       	        pline("Wounds appear on %s!", mon_nam(mtmp));
       	    else if (dmg <= 20)
       	        pline("Severe wounds appear on %s!", mon_nam(mtmp));
       	    else
       	        pline("%s is covered in wounds!", Monnam(mtmp));
       	}
       	break;
    default:
       	impossible("ucastm: invalid clerical spell (%d)", spellnum);
       	dmg = 0;
       	break;
    }

    if (dmg > 0 && mtmp->mhp > 0) {
        mtmp->mhp -= dmg;
        if (mtmp->mhp < 1) {
       	    if (yours) killed(mtmp);
       	    else monkilled(mtmp, "", AD_CLRC);
	}
    }
}

/*mcastu.c*/
