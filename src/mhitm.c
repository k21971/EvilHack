/* NetHack 3.6	mhitm.c	$NHDT-Date: 1583606861 2020/03/07 18:47:41 $  $NHDT-Branch: NetHack-3.6-Mar2020 $:$NHDT-Revision: 1.119 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2011. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "artifact.h"
#include <limits.h>

extern boolean notonhead;

static NEARDATA boolean vis, far_noise;
static NEARDATA long noisetime;

static const char brief_feeling[] =
    "have a %s feeling for a moment, then it passes.";

STATIC_DCL int FDECL(hitmm, (struct monst *, struct monst *,
                             struct attack *, struct obj *, int));
STATIC_DCL int FDECL(gazemm, (struct monst *, struct monst *, struct attack *));
STATIC_DCL int FDECL(screamm, (struct monst *, struct monst *, struct attack *));
STATIC_DCL int FDECL(gulpmm, (struct monst *, struct monst *, struct attack *));
STATIC_DCL int FDECL(explmm, (struct monst *, struct monst *, struct attack *));
STATIC_DCL int FDECL(mdamagem, (struct monst *, struct monst *,
                                struct attack *, struct obj *, int, struct obj **));
STATIC_DCL void FDECL(mswingsm, (struct monst *, struct monst *, struct obj *));
STATIC_DCL void FDECL(noises, (struct monst *, struct attack *));
STATIC_DCL void FDECL(missmm, (struct monst *, struct monst *,
                               int, int, struct attack *));
STATIC_DCL int FDECL(passivemm, (struct monst *, struct monst *,
                                 BOOLEAN_P, int, struct obj *, int));

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

STATIC_OVL void
noises(magr, mattk)
register struct monst *magr;
register struct attack *mattk;
{
    boolean farq = (distu(magr->mx, magr->my) > 15);

    if (!Deaf && (farq != far_noise || moves - noisetime > 10)) {
        far_noise = farq;
        noisetime = moves;
        You_hear("%s%s.",
                 (mattk->aatyp == AT_EXPL) ? "an explosion" : "some noises",
                 farq ? " in the distance" : "");
    }
}

STATIC_OVL
void
missmm(magr, mdef, target, roll, mattk)
register struct monst *magr, *mdef;
struct attack *mattk;
int target, roll;
{
    boolean nearmiss = (target == roll);
    const char *fmt;
    char buf[BUFSZ];
    boolean showit = FALSE;

    register struct obj *blocker = (struct obj *) 0;

    /* 2 values for blocker:
     * No blocker: (struct obj *) 0
     * Piece of armour: object
     */

    if (target < roll) {
        /* get object responsible,
           work from the closest to the skin outwards */

        /* Try undershirt */
        if (which_armor(mdef, W_ARMU)
            && (which_armor(mdef, W_ARM) == 0)
            && (which_armor(mdef, W_ARMC) == 0)
            && target <= roll) {
            target += armor_bonus(which_armor(mdef, W_ARMU));
            if (target > roll)
                blocker = which_armor(mdef, W_ARMU);
        }

        /* Try body armour */
        if (which_armor(mdef, W_ARM)
            && (which_armor(mdef, W_ARMC) == 0) && target <= roll) {
            target += armor_bonus(which_armor(mdef, W_ARM));
            if (target > roll)
                blocker = which_armor(mdef, W_ARM);
        }

        if (which_armor(mdef, W_ARMG) && !rn2(10)) {
            /* Try gloves */
            target += armor_bonus(which_armor(mdef, W_ARMG));
            if (target > roll)
                blocker = which_armor(mdef, W_ARMG);
        }

        if (which_armor(mdef, W_ARMF) && !rn2(10)) {
            /* Try boots */
            target += armor_bonus(which_armor(mdef, W_ARMF));
            if (target > roll)
                blocker = which_armor(mdef, W_ARMF);
        }

        if (which_armor(mdef, W_ARMH) && !rn2(5)) {
            /* Try helm */
            target += armor_bonus(which_armor(mdef, W_ARMH));
            if (target > roll)
                blocker = which_armor(mdef, W_ARMH);
        }

        if (which_armor(mdef, W_ARMC) && target <= roll) {
            /* Try cloak */
            target += armor_bonus(which_armor(mdef, W_ARMC));
            if (target > roll)
                blocker = which_armor(mdef, W_ARMC);
        }

        if (which_armor(mdef, W_ARMS) && target <= roll) {
            /* Try shield */
            target += armor_bonus(which_armor(mdef, W_ARMS));
            if (target > roll)
                blocker = which_armor(mdef, W_ARMS);
        }

        if (which_armor(mdef, W_BARDING) && target <= roll) {
            /* Try barding (steeds) */
            target += armor_bonus(which_armor(mdef, W_BARDING));
            if (target > roll)
                blocker = which_armor(mdef, W_BARDING);
        }
    }

    /* unhiding or unmimicking happens even if hero can't see it
       because the formerly concealed monster is now in action */
    if (M_AP_TYPE(mdef)) {
        seemimic(mdef);
        showit |= vis;
    } else if (mdef->mundetected) {
        mdef->mundetected = 0;
        showit |= vis;
    }
    if (M_AP_TYPE(magr)) {
        seemimic(magr);
        showit |= vis;
    } else if (magr->mundetected) {
        magr->mundetected = 0;
        showit |= vis;
    }

    if (vis) {
        if (!canspotmon(magr))
            map_invisible(magr->mx, magr->my);
        else if (showit)
            newsym(magr->mx, magr->my);
        if (!canspotmon(mdef))
            map_invisible(mdef->mx, mdef->my);
        else if (showit)
            newsym(mdef->mx, mdef->my);

        if (flags.verbose && !nearmiss && blocker) {
            fmt = "%s %s %s";
            Sprintf(buf, fmt, s_suffix(Monnam(mdef)),
                    aobjnam(blocker, (char *) 0),
                    (rn2(2) ? "blocks" : "deflects"));
            pline("%s %s attack.", buf, s_suffix(mon_nam_too(magr, mdef)));
            /* called if attacker hates the material of the armor
               that deflected their attack */
            if (blocker
                && (!MON_WEP(magr) && which_armor(magr, W_ARMG) == 0)
                && mon_hates_material(magr, blocker->material)) {
                searmsg(mdef, magr, blocker, FALSE);
                /* glancing blow */
                magr->mhp -= rnd(sear_damage(blocker->material) / 2);
                if (DEADMONSTER(magr))
                    monkilled(magr, "", AD_PHYS);
            }
            /* glass armor, or certain drow armor if in the presence
               of light, can potentially break if it deflects an attack */
            if (blocker
                && (is_glass(blocker) || is_adamantine(blocker)))
                break_glass_obj(blocker);
        } else {
            if (thick_skinned(mdef->data) && !rn2(10)) {
                fmt = "%s %s %s";
                Sprintf(buf, fmt, s_suffix(Monnam(mdef)),
                        (is_dragon(mdef->data) ? "scaly hide"
                                               : (mdef->data == &mons[PM_GIANT_TURTLE]
                                                  || is_tortle(mdef->data))
                                                   ? "protective shell"
                                                   : "thick hide"),
                        (rn2(2) ? "blocks" : "deflects"));
                pline("%s %s attack.", buf, s_suffix(mon_nam_too(magr, mdef)));
            } else {
                fmt = (could_seduce(magr, mdef, mattk) && !magr->mcan)
                          ? "%s pretends to be friendly to"
                          : "%s %smisses";
                Sprintf(buf, fmt, Monnam(magr), (nearmiss ? "just " : ""));
                pline("%s %s.", buf, mon_nam_too(mdef, magr));
            }
        }
    } else
        noises(magr, mattk);
}

/*
 *  fightm()  -- fight some other monster
 *
 *  Returns:
 *      0 - Monster did nothing.
 *      1 - If the monster made an attack.  The monster might have died.
 *
 *  There is an exception to the above.  If mtmp has the hero swallowed,
 *  then we report that the monster did nothing so it will continue to
 *  digest the hero.
 */
 /* have monsters fight each other */
int
fightm(mtmp)
register struct monst *mtmp;
{
    register struct monst *mon, *nmon;
    int result, has_u_swallowed;
    boolean conflict = Conflict && !resist(mtmp, RING_CLASS, 0, 0);
#ifdef LINT
    nmon = 0;
#endif
    /* perhaps the monster will resist Conflict */

    /* From SporkHack:
     * In practice, this should be suffixed with "... but probably not".
     * The old MR check was just letting too many monsters avoid being hit with
     * conflict in the first place; the ones that didn't resist basically swarmed under
     * the ones that did, allowing the player to blithely walk through a street brawl
     * almost totally untouched.  Not so anymore.
     *
     * Resistance is based on the player's (mostly-useless) Charisma.  High-CHA players
     * will be able to 'convince' monsters (through the magic of the ring, of course) to fight
     * for them much more easily than low-CHA players. */

    if (resist_conflict(mtmp))
        return 0;
    if ((mtmp->mtame || is_covetous(mtmp->data)) && !conflict)
      	    return 0;

    if (u.ustuck == mtmp) {
        /* perhaps we're holding it... */
        if (itsstuck(mtmp))
            return 0;
    }
    has_u_swallowed = (u.uswallow && (mtmp == u.ustuck));

    for (mon = fmon; mon; mon = nmon) {
        nmon = mon->nmon;
        if (nmon == mtmp)
            nmon = mtmp->nmon;
        /* Be careful to ignore monsters that are already dead, since we
         * might be calling this before we've cleaned them up.  This can
         * happen if the monster attacked a cockatrice bare-handedly, for
         * instance.
         */
        if (mon != mtmp && !DEADMONSTER(mon)) {
            if (monnear(mtmp, mon->mx, mon->my)) {
                if (!conflict && !mm_aggression(mtmp, mon))
               		  continue;
                if (!u.uswallow && (mtmp == u.ustuck)) {
                    if (!rn2(4)) {
                        pline("%s releases you!", Monnam(mtmp));
                        u.ustuck = 0;
                    } else
                        break;
                }

                /* mtmp can be killed */
                bhitpos.x = mon->mx;
                bhitpos.y = mon->my;
                notonhead = 0;
                result = mattackm(mtmp, mon);

                if (result & MM_AGR_DIED)
                    return 1; /* mtmp died */
                /*
                 * If mtmp has the hero swallowed, lie and say there
                 * was no attack (this allows mtmp to digest the hero).
                 */
                if (has_u_swallowed)
                    return 0;

                /* Allow attacked monsters a chance to hit back. Primarily
                 * to allow monsters that resist conflict to respond.
                 */
                if ((result & MM_HIT) && !(result & MM_DEF_DIED) && rn2(4)
                    && mon->movement >= NORMAL_SPEED) {
                    mon->movement -= NORMAL_SPEED;
                    notonhead = 0;
                    (void) mattackm(mon, mtmp); /* return attack */
                }

                return (result & MM_HIT) ? 1 : 0;
            }
        }
    }
    return 0;
}

/*
 * mdisplacem() -- attacker moves defender out of the way;
 *                 returns same results as mattackm().
 */
int
mdisplacem(magr, mdef, quietly)
register struct monst *magr, *mdef;
boolean quietly;
{
    struct permonst *pa, *pd;
    int tx, ty, fx, fy;

    /* sanity checks; could matter if we unexpectedly get a long worm */
    if (!magr || !mdef || magr == mdef || has_erid(mdef))
        return MM_MISS;
    pa = magr->data, pd = mdef->data;
    tx = mdef->mx, ty = mdef->my; /* destination */
    fx = magr->mx, fy = magr->my; /* current location */
    if (m_at(fx, fy) != magr || m_at(tx, ty) != mdef)
        return MM_MISS;

    /* The 1 in 7 failure below matches the chance in attack()
     * for pet displacement.
     */
    if (!rn2(7))
        return MM_MISS;

    /* Grid bugs cannot displace at an angle. */
    if (pa == &mons[PM_GRID_BUG] && magr->mx != mdef->mx
        && magr->my != mdef->my)
        return MM_MISS;

    /* undetected monster becomes un-hidden if it is displaced */
    if (mdef->mundetected &&
        dist2(mdef->mx, mdef->my, magr->mx, magr->my) > 2)
        mdef->mundetected = 0;
    if (M_AP_TYPE(mdef) && M_AP_TYPE(mdef) != M_AP_MONSTER)
        seemimic(mdef);
    /* wake up the displaced defender */
    mdef->msleeping = 0;
    mdef->mstrategy &= ~STRAT_WAITMASK;
    finish_meating(mdef);

    /*
     * Set up the visibility of action.
     * You can observe monster displacement if you can see both of
     * the monsters involved.
     */
    vis = (canspotmon(magr) && canspotmon(mdef));

    if (touch_petrifies(pd)
        && !(resists_ston(magr) || defended(magr, AD_STON))) {
        if (which_armor(magr, W_ARMG) != 0) {
            if (poly_when_stoned(pa)) {
                mon_to_stone(magr);
                return MM_HIT; /* no damage during the polymorph */
            }
            if (!quietly && canspotmon(magr))
                pline("%s turns to stone!", Monnam(magr));
            monstone(magr);
            if (!DEADMONSTER(magr))
                return MM_HIT; /* lifesaved */
            else if (magr->mtame && !vis)
                You(brief_feeling, "peculiarly sad");
            return MM_AGR_DIED;
        }
    }

    remove_monster(fx, fy); /* pick up from orig position */
    remove_monster(tx, ty);
    place_monster(magr, tx, ty); /* put down at target spot */
    place_monster(mdef, fx, fy);
    /* the monster that moves can decide to hide in its new spot; the displaced
     * monster is forced out of hiding even if it can hide in its new spot */
    if (hides_under(magr->data))
        hideunder(magr);
    mdef->mundetected = 0;
    if (vis && !quietly)
        pline("%s moves %s out of %s way!", Monnam(magr), mon_nam(mdef),
              is_rider(pa) ? "the" : mhis(magr));
    newsym(fx, fy);  /* see it */
    newsym(tx, ty);  /*   all happen */
    flush_screen(0); /* make sure it shows up */

    return MM_HIT;
}

boolean
resist_conflict(mtmp)
struct monst *mtmp;
{
    int resist_chance;
    struct obj *barding;

    resist_chance = ACURR(A_CHA) - mtmp->m_lev + u.ulevel;
    if (resist_chance > 19) {
        resist_chance = 19; /* always a small chance */
    } else if ((barding = which_armor(mtmp, W_BARDING)) != 0
        && barding->oartifact == ART_ITHILMAR) {
        /* steed wearing Ithilmar will always resist */
        return TRUE;
    }

    return (rnd(20) > resist_chance);
}

/*
 * mattackm() -- a monster attacks another monster.
 *
 *           ----------- defender was "hurriedly expelled"
 *	    /  --------- aggressor died
 *	   /  /  ------- defender died
 *	  /  /  /  ----- defender was hit
 *	 /  /  /  /
 *	x  x  x  x
 *
 *      0x8     MM_EXPELLED
 *      0x4     MM_AGR_DIED
 *      0x2     MM_DEF_DIED
 *      0x1     MM_HIT
 *      0x0     MM_MISS
 *
 * Each successive attack has a lower probability of hitting.  Some rely on
 * success of previous attacks.  ** this doen't seem to be implemented -dl **
 *
 * In the case of exploding monsters, the monster dies as well.
 */
int
mattackm(magr, mdef)
register struct monst *magr, *mdef;
{
    int i,          /* loop counter */
        tmp,        /* amour class difference */
        strike = 0, /* hit this attack */
        attk,       /* attack attempted this time */
        struck = 0, /* hit at least once */
        res[NATTK], /* results of all attacks */
        dieroll = 0,
        saved_mhp = (mdef ? mdef->mhp : 0); /* for print_mon_wounded() */
    struct attack *mattk, alt_attk;
    struct obj *mwep;
    struct permonst *pa; /* *pd no longer used (for now) */

    if (!magr || !mdef)
        return MM_MISS; /* mike@genat */
    if (!magr->mcanmove || magr->msleeping)
        return MM_MISS;
    pa = magr->data;
    /* pd = mdef->data; */

    /* Grid bugs cannot attack at an angle. */
    if (pa == &mons[PM_GRID_BUG] && magr->mx != mdef->mx
        && magr->my != mdef->my)
        return MM_MISS;

    /* Calculate the armour class differential. */
    tmp = find_mac(mdef) + magr->m_lev;
    if (mdef->mconf || !mdef->mcanmove || mdef->msleeping) {
        tmp += 4;
        mdef->msleeping = 0;
    }

    /* M3_ACCURATE monsters get a to-hit bonus */
    if ((has_erac(magr) && (ERAC(magr)->mflags3 & M3_ACCURATE))
        || is_accurate(pa))
        tmp += 5;

    /* find rings of increase accuracy */
    {
	struct obj *o;
	for (o = magr->minvent; o; o = o->nobj)
	     if (o->owornmask && o->otyp == RIN_INCREASE_ACCURACY)
	         tmp += o->spe;
    }

    /* undetect monsters become un-hidden if they are attacked */
    if (mdef->mundetected) {
        mdef->mundetected = 0;
        newsym(mdef->mx, mdef->my);
        if (canseemon(mdef) && !sensemon(mdef)) {
            if (Unaware) {
                boolean justone = (mdef->data->geno & G_UNIQ) != 0L;
                const char *montype;

                montype = noname_monnam(mdef, justone ? ARTICLE_THE
                                                      : ARTICLE_NONE);
                if (!justone)
                    montype = makeplural(montype);
                You("dream of %s.", montype);
            } else
                pline("Suddenly, you notice %s.", a_monnam(mdef));
        }
    }

    /* Elven types hate orcs */
    if ((racial_elf(magr) || racial_drow(magr))
        && racial_orc(mdef))
        tmp++;

    /* Drow are affected by being in both the light or the dark */
    if (racial_drow(magr)) {
        if (!(levl[magr->mx][magr->my].lit
              || (viz_array[magr->my][magr->mx] & TEMP_LIT))
            || (viz_array[magr->my][magr->mx] & TEMP_DARK)) {
            /* in darkness */
            tmp += (magr->m_lev / 3) + 2;
        } else {
            /* in the light */
            tmp -= 3;
        }
    }

    /* Set up the visibility of action */
    vis = (cansee(magr->mx, magr->my) && cansee(mdef->mx, mdef->my)
           && (canspotmon(magr) || canspotmon(mdef)));

    /* Set flag indicating monster has moved this turn.  Necessary since a
     * monster might get an attack out of sequence (i.e. before its move) in
     * some cases, in which case this still counts as its move for the round
     * and it shouldn't move again.
     */
    magr->mlstmv = monstermoves;

    /* Now perform all attacks for the monster. */
    for (i = 0; i < NATTK; i++) {
        res[i] = MM_MISS;
        mattk = getmattk(magr, mdef, i, res, &alt_attk);
        mwep = (struct obj *) 0;
        attk = 1;
        switch (mattk->aatyp) {
        case AT_WEAP: /* weapon attacks */
            if (distmin(magr->mx, magr->my, mdef->mx, mdef->my) > 1) {
                /* D: Do a ranged attack here! */
                strike = thrwmm(magr, mdef);
                if (DEADMONSTER(mdef))
                    res[i] = MM_DEF_DIED;
                if (DEADMONSTER(magr))
                    res[i] |= MM_AGR_DIED;
                break;
            }
            if (magr->weapon_check == NEED_WEAPON || !MON_WEP(magr)) {
                magr->weapon_check = NEED_HTH_WEAPON;
                if (mon_wield_item(magr) != 0)
                    return 0;
            }
            possibly_unwield(magr, FALSE);
            if ((mwep = MON_WEP(magr)) != 0) {
                if (vis)
                    mswingsm(magr, mdef, mwep);
                tmp += hitval(mwep, mdef);
            }
            /*FALLTHRU*/
        case AT_CLAW:
        case AT_KICK:
        case AT_BITE:
        case AT_STNG:
        case AT_TUCH:
        case AT_BUTT:
        case AT_TENT:
            /* Nymph that teleported away on first attack? */
            if (distmin(magr->mx, magr->my, mdef->mx, mdef->my) > 1)
                /* Continue because the monster may have a ranged attack. */
                continue;
            /* Monsters won't attack cockatrices physically if they
             * have a weapon instead.  This instinct doesn't work for
             * players, or under conflict or confusion.
             */
            if (!magr->mconf && !Conflict && mwep && mattk->aatyp != AT_WEAP
                && touch_petrifies(mdef->data)) {
                strike = 0;
                break;
            }
            dieroll = rnd(20 + i);
            strike = (tmp > dieroll);
            /* KMH -- don't accumulate to-hit bonuses */
            if (mwep)
                tmp -= hitval(mwep, mdef);
            if ((is_displaced(mdef->data) || has_displacement(mdef))
                && rn2(4)) {
                if (vis && canspotmon(mdef))
                    pline("%s attacks the displaced image of %s.",
                          Monnam(magr), mon_nam(mdef));
                strike = FALSE;
            }
            /* tiny/small monsters have a chance to dodge
               a zombies bite attack due to zombies being
               mindless and slow */
            if (racial_zombie(magr) && mattk->aatyp == AT_BITE
                && mdef->data->msize <= MZ_SMALL
                && is_animal(mdef->data)
                && !(mdef->mfrozen || mdef->mstone
                     || mdef->mconf || mdef->mstun) && rn2(3)) {
                if (vis && canspotmon(mdef))
                    pline("%s nimbly %s %s bite!", Monnam(mdef),
                          rn2(2) ? "dodges" : "evades", s_suffix(mon_nam(magr)));
                strike = FALSE;
            }
            if (strike) {
                short type = 0;
                int corpsenm = 0;
                unsigned int material = 0;

                if (mwep && mwep->otyp)
                    type = mwep->otyp;
                if (mwep && mwep->corpsenm)
                    corpsenm = mwep->corpsenm;
                if (mwep && mwep->material)
                    material = mwep->material;

                res[i] = hitmm(magr, mdef, mattk, mwep, dieroll);
                if ((res[i]) == MM_HIT && mwep
                    && type == CORPSE
                    && corpsenm
                    && touch_petrifies(&mons[corpsenm])
                    && !(resists_ston(mdef) || defended(mdef, AD_STON))) {
                    if (poly_when_stoned(mdef->data)) {
                        mon_to_stone(mdef);
                    } else if (!mdef->mstone) {
                       mdef->mstone = 5;
                       mdef->mstonebyu = FALSE;
                    }
                    break;
                }

                if ((mdef->data == &mons[PM_BLACK_PUDDING]
                     || mdef->data == &mons[PM_BROWN_PUDDING])
                    && (mwep && (material == IRON
                                 || material == METAL))
                    && mdef->mhp > 1 && !mdef->mcan) {
                    struct monst *mclone;

                    if ((mclone = clone_mon(mdef, 0, 0)) != 0) {
                        if (vis && canspotmon(mdef)) {
                            char buf[BUFSZ];

                            Strcpy(buf, Monnam(mdef));
                            pline("%s divides as %s hits it!", buf,
                                  mon_nam(magr));
                        }
                        mintrap(mclone);
                    }
                }
            } else {
                missmm(magr, mdef, tmp, dieroll, mattk);
                /* if the attacker dies from a glancing blow off
                   of a piece of the defender's armor, and said armor
                   is made of a material the attacker hates, this
                   check is necessary to prevent a dmonsfree error
                   if the attacker has multiple attacks and they
                   died before their attack chain completed */
                if (DEADMONSTER(magr))
                    res[i] |= MM_AGR_DIED;
            }
            break;

        case AT_HUGS: /* automatic if prev two attacks succeed */
            strike = (i >= 2 && res[i - 1] == MM_HIT && res[i - 2] == MM_HIT);
            if ((is_displaced(mdef->data) || has_displacement(mdef))
                && rn2(4)) {
                if (vis && canspotmon(mdef))
                    pline("%s attacks the displaced image of %s.",
                          Monnam(magr), mon_nam(mdef));
                strike = FALSE;
            }
            if (strike)
                res[i] = hitmm(magr, mdef, mattk, (struct obj *) 0, 0);

            break;

        case AT_GAZE:
            strike = 0;
            res[i] = gazemm(magr, mdef, mattk);
            break;

        case AT_SCRE:
            strike = 0;
            res[i] = screamm(magr, mdef, mattk);
            break;

        case AT_EXPL:
            /* D: Prevent explosions from a distance */
            if (distmin(magr->mx, magr->my, mdef->mx, mdef->my) > 1)
                continue;

            res[i] = explmm(magr, mdef, mattk);
            if (res[i] == MM_MISS) { /* cancelled--no attack */
                strike = 0;
                attk = 0;
            } else
                strike = 1; /* automatic hit */
            break;

        case AT_ENGL:
            /* D: Prevent engulf from a distance */
            if (distmin(magr->mx, magr->my, mdef->mx, mdef->my) > 1)
                continue;

            if (noncorporeal(mdef->data) /* no silver teeth... */
                || passes_walls(mdef->data)) {
                if (vis)
                    pline("%s attempt to engulf %s is futile.",
                          s_suffix(Monnam(magr)), mon_nam(mdef));
                strike = 0;
                break;
            }
            if (u.usteed && mdef == u.usteed) {
                strike = 0;
                break;
            }
            /* Engulfing attacks are directed at the hero if possible. -dlc */
            if (u.uswallow && magr == u.ustuck)
                strike = 0;
            else if ((strike = (tmp > rnd(20 + i))) != 0)
                res[i] = gulpmm(magr, mdef, mattk);
            else
                missmm(magr, mdef, tmp, dieroll, mattk);
            break;

        case AT_BREA:
            if (!monnear(magr, mdef->mx, mdef->my)) {
                strike = breamm(magr, mattk, mdef);

                /* We don't really know if we hit or not; pretend we did. */
                if (strike)
                    res[i] |= MM_HIT;
                if (DEADMONSTER(mdef))
                    res[i] = MM_DEF_DIED;
                if (DEADMONSTER(magr))
                    res[i] |= MM_AGR_DIED;
            }
            else
                strike = 0;
            break;

        case AT_SPIT:
            if (!monnear(magr, mdef->mx, mdef->my)) {
                strike = spitmm(magr, mattk, mdef);

                /* We don't really know if we hit or not; pretend we did. */
                if (strike)
                    res[i] |= MM_HIT;
                if (DEADMONSTER(mdef))
                    res[i] = MM_DEF_DIED;
                if (DEADMONSTER(magr))
                    res[i] |= MM_AGR_DIED;
            }
            break;
        case AT_MAGC:
            if (!monnear(magr, mdef->mx, mdef->my)) {
                strike = buzzmm(magr, mdef, mattk);

                /* We don't really know if we hit or not; pretend we did. */
                if (strike)
                    res[i] |= MM_HIT;
                if (DEADMONSTER(mdef))
                    res[i] = MM_DEF_DIED;
                if (DEADMONSTER(magr))
                    res[i] |= MM_AGR_DIED;
            } else if (monnear(magr, mdef->mx, mdef->my)) {
                strike = castmm(magr, mdef, mattk);

                if (strike)
                    res[i] |= MM_HIT;
                if (DEADMONSTER(mdef))
                    res[i] = MM_DEF_DIED;
                if (DEADMONSTER(magr))
                    res[i] |= MM_AGR_DIED;
            }
            break;
        default: /* no attack */
            strike = 0;
            attk = 0;
            break;
        }

        if (attk && !(res[i] & MM_AGR_DIED)
            && distmin(magr->mx, magr->my, mdef->mx, mdef->my) <= 1)
            res[i] = passivemm(magr, mdef, strike,
                               (res[i] & MM_DEF_DIED), mwep, mattk->aatyp);

        if (res[i] & MM_DEF_DIED)
            return res[i];
        if (res[i] & MM_AGR_DIED)
            return res[i];
        /* return if aggressor can no longer attack */
        if (!magr->mcanmove || magr->msleeping)
            return res[i];
        if (res[i] & MM_HIT)
            struck = 1; /* at least one hit */
    }
    if (struck && mdef->mtame) {
        print_mon_wounded(mdef, saved_mhp);
    }

    return (struck ? MM_HIT : MM_MISS);
}

/* Returns the result of mdamagem(). */
STATIC_OVL int
hitmm(magr, mdef, mattk, mwep, dieroll)
register struct monst *magr, *mdef;
struct attack *mattk;
struct obj *mwep;
int dieroll;
{
    struct obj *otmp;
    boolean weaponhit = ((mattk->aatyp == AT_WEAP
                          || (mattk->aatyp == AT_CLAW && mwep))),
            showit = FALSE;

    /* Possibly awaken nearby monsters */
    if ((!is_silent(magr->data) || !helpless(mdef)) && rn2(10))
        wake_nearto(magr->mx, magr->my, combat_noise(magr->data));

    /* glass breakage from the attack */
    break_glass_obj(some_armor(mdef));
    if (break_glass_obj(mwep))
        mwep = NULL;

    /* unhiding or unmimicking happens even if hero can't see it
       because the formerly concealed monster is now in action */
    if (M_AP_TYPE(mdef)) {
        seemimic(mdef);
        showit |= vis;
    } else if (mdef->mundetected) {
        mdef->mundetected = 0;
        showit |= vis;
    }
    if (M_AP_TYPE(magr)) {
        seemimic(magr);
        showit |= vis;
    } else if (magr->mundetected) {
        magr->mundetected = 0;
        showit |= vis;
    }

    if (vis) {
        int compat;
        char buf[BUFSZ];

        if (!canspotmon(magr))
            map_invisible(magr->mx, magr->my);
        else if (showit)
            newsym(magr->mx, magr->my);
        if (!canspotmon(mdef))
            map_invisible(mdef->mx, mdef->my);
        else if (showit)
            newsym(mdef->mx, mdef->my);

        if ((compat = could_seduce(magr, mdef, mattk)) && !magr->mcan) {
            Sprintf(buf, "%s %s", Monnam(magr),
                    mdef->mcansee ? "smiles at" : "talks to");
            pline("%s %s %s.", buf, mon_nam(mdef),
                  compat == 2 ? "engagingly" : "seductively");
        } else if (shade_miss(magr, mdef, mwep, FALSE, TRUE)) {
            return MM_MISS; /* bypass mdamagem() */
        } else {
            char magr_name[BUFSZ];

            Strcpy(magr_name, Monnam(magr));
            switch (mattk->aatyp) {
            case AT_BITE:
		Sprintf(buf,"%s %ss", magr_name, has_beak(magr->data) ?
                        "peck" : "bite");
                break;
            case AT_KICK:
                Sprintf(buf, "%s kicks", magr_name);
                break;
            case AT_STNG:
                Sprintf(buf, "%s stings", magr_name);
                break;
            case AT_BUTT:
                Sprintf(buf, "%s %ss", magr_name, has_trunk(magr->data) ?
                        "gore" : "butt");
                break;
            case AT_TUCH:
                if (magr->data == &mons[PM_DEATH])
                    Sprintf(buf, "%s reaches out with its deadly touch towards", magr_name);
                else if (magr->data == &mons[PM_GIANT_CENTIPEDE])
                    Sprintf(buf, "%s coils its body around", magr_name);
                else
                    Sprintf(buf, "%s touches", magr_name);
                break;
            case AT_TENT:
                if (magr->data == &mons[PM_MEDUSA])
                    Sprintf(buf, "The venomous snakes on %s head attack",
                            s_suffix(magr_name));
                else if (magr->data == &mons[PM_DEMOGORGON]
                         || magr->data == &mons[PM_NEOTHELID])
                    Sprintf(buf, "%s tentacles lash out at",
                            s_suffix(magr_name));
                else
                    Sprintf(buf, "%s tentacles suck", s_suffix(magr_name));
                break;
            case AT_HUGS:
                if (magr != u.ustuck)
                    Sprintf(buf, "%s squeezes", magr_name);
                break;
            case AT_WEAP:
                if (!MON_WEP(magr)) { /* AT_WEAP but isn't wielding anything */
                    if (has_claws(r_data(magr)))
                        Sprintf(buf, "%s claws", magr_name);
                    else if (has_claws_undead(r_data(magr)))
                        Sprintf(buf, "%s scratches", magr_name);
                    else
                        Sprintf(buf, "%s %ss", magr_name,
                                mwep_none[rn2(SIZE(mwep_none))]);
                } else if (is_pierce(MON_WEP(magr)))
                    Sprintf(buf, "%s %ss", magr_name,
                            mwep_pierce[rn2(SIZE(mwep_pierce))]);
                else if (is_slash(MON_WEP(magr)))
                    Sprintf(buf, "%s %ss", magr_name,
                            mwep_slash[rn2(SIZE(mwep_slash))]);
                else if (is_whack(MON_WEP(magr)))
                    Sprintf(buf, "%s %ss", magr_name,
                            mwep_whack[rn2(SIZE(mwep_whack))]);
                else
                    Sprintf(buf, "%s hits", magr_name);
                break;
            case AT_CLAW:
                if (has_claws(r_data(magr)))
                    Sprintf(buf, "%s claws", magr_name);
                else if (has_claws_undead(r_data(magr)))
                    Sprintf(buf, "%s scratches", magr_name);
                else
                    Sprintf(buf, "%s hits", magr_name);
                break;
            default:
                if (!weaponhit || !mwep || !mwep->oartifact)
                    Sprintf(buf, "%s hits", magr_name);
                break;
            }
            if (*buf)
                pline("%s %s.", buf, mon_nam_too(mdef, magr));
        }
    } else
        noises(magr, mattk);

    return mdamagem(magr, mdef, mattk, mwep, dieroll, &otmp);
}

/* Returns the same values as mdamagem(). */
STATIC_OVL int
gazemm(magr, mdef, mattk)
register struct monst *magr, *mdef;
struct attack *mattk;
{
    struct obj *otmp;
    char buf[BUFSZ];

    /* call mon_reflects 2x, first test, then, if visible, print message */
    if (magr->data == &mons[PM_MEDUSA]) {
        if (vis) {
            if (mdef->data->mlet == S_MIMIC
                && M_AP_TYPE(mdef) != M_AP_NOTHING)
                seemimic(mdef);
            Sprintf(buf, "%s gazes at", Monnam(magr));
            pline("%s %s...", buf,
                  canspotmon(mdef) ? mon_nam(mdef) : "something");
        }

        if (magr->mcan || !magr->mcansee || !mdef->mcansee
            || (magr->minvis && !racial_perceives(mdef)) || mdef->msleeping) {
            if (vis && canspotmon(mdef))
                pline("But nothing happens.");
            return MM_MISS;
        }
        if (mon_reflects(mdef, (char *) 0)) {
            if (canseemon(mdef))
                (void) mon_reflects(mdef, "The gaze is reflected away by %s %s.");
            if (mdef->mcansee) {
                if (mon_reflects(magr, (char *) 0)) {
                    if (canseemon(magr))
                        (void) mon_reflects(magr,
                                            "The gaze is reflected away by %s %s.");
                    return MM_MISS;
                }
                if (mdef->minvis && !racial_perceives(magr)) {
                    if (canseemon(magr)) {
                        pline(
                          "%s doesn't seem to notice that %s gaze was reflected.",
                              Monnam(magr), mhis(magr));
                    }
                    return MM_MISS;
                }
                if (canseemon(magr))
                    pline("%s is turned to stone!", Monnam(magr));
                monstone(magr);
                if (!DEADMONSTER(magr))
                    return MM_MISS;
                return MM_AGR_DIED;
            }
        }
    }

    return mdamagem(magr, mdef, mattk, (struct obj *) 0, 0, &otmp);
}

/* Returns the same values as mdamagem() */
STATIC_OVL int
screamm(magr, mdef, mattk)
register struct monst *magr, *mdef;
struct attack *mattk;
{
    struct obj *otmp;

    if (canseemon(magr) && !Deaf) {
        pline("%s lets out a %s!", Monnam(magr),
              magr->data == &mons[PM_NAZGUL] ? "bloodcurdling scream"
                                             : "deafening roar");
        if (!mdef->mstun) {
            if (canseemon(mdef))
                pline("%s reels from the noise!", Monnam(mdef));
        } else {
            if (canseemon(mdef))
                pline("%s struggles to keep its balance.", Monnam(mdef));
        }
    }

    return mdamagem(magr, mdef, mattk, (struct obj *) 0, 0, &otmp);
}

/* return True if magr is allowed to swallow mdef, False otherwise */
boolean
engulf_target(magr, mdef)
struct monst *magr, *mdef;
{
    struct rm *lev;
    int dx, dy;

    /* can't swallow something that's too big */
    if (r_data(mdef)->msize >= MZ_HUGE
        || (r_data(magr)->msize < r_data(mdef)->msize && !is_whirly(magr->data)))
        return FALSE;

    /* can't swallow trapped monsters. TODO: could do some? */
    if (mdef->mtrapped)
        return FALSE;

    /* can't swallow something if riding / being ridden */
    if (magr->ridden_by || mdef->ridden_by || has_erid(magr))
        return FALSE;

    /* (hypothetical) engulfers who can pass through walls aren't
     limited by rock|trees|bars */
    if ((magr == &youmonst) ? Passes_walls : passes_walls(magr->data))
        return TRUE;

    /* don't swallow something in a spot where attacker wouldn't
       otherwise be able to move onto; we don't want to engulf
       a wall-phaser and end up with a non-phaser inside a wall */
    dx = mdef->mx, dy = mdef->my;
    if (mdef == &youmonst)
        dx = u.ux, dy = u.uy;
    lev = &levl[dx][dy];
    if (IS_ROCK(lev->typ) || closed_door(dx, dy) || IS_TREES(lev->typ)
        /* not passes_bars(); engulfer isn't squeezing through */
        || (lev->typ == IRONBARS && !is_whirly(magr->data)))
        return FALSE;

    return TRUE;
}

/* Returns the same values as mattackm(). */
STATIC_OVL int
gulpmm(magr, mdef, mattk)
register struct monst *magr, *mdef;
register struct attack *mattk;
{
    xchar ax, ay, dx, dy;
    int status;
    char buf[BUFSZ];
    struct obj *obj, *otmp;
    struct monst *msteed = NULL;

    if (!engulf_target(magr, mdef))
        return MM_MISS;

    if (has_erid(mdef)) {
        if (vis)
            pline("%s plucks %s right off %s mount!", Monnam(magr), mon_nam(mdef), mhis(mdef));
        separate_steed_and_rider(mdef);
    }

    if (vis) {
        /* [this two-part formatting dates back to when only one x_monnam
           result could be included in an expression because the next one
           would overwrite first's result -- that's no longer the case] */
        Sprintf(buf, "%s swallows", Monnam(magr));
        pline("%s %s.", buf, mon_nam(mdef));
    }
    for (obj = mdef->minvent; obj; obj = obj->nobj)
        (void) snuff_lit(obj);

    if (is_vampshifter(mdef)
        && newcham(mdef, &mons[mdef->cham], FALSE, FALSE)) {
        if (vis) {
            /* 'it' -- previous form is no longer available and
               using that would be excessively verbose */
            pline("%s expels %s.", Monnam(magr),
                  canspotmon(mdef) ? "it" : something);
            if (canspotmon(mdef))
                pline("It turns into %s.", a_monnam(mdef));
        }
        return MM_HIT; /* bypass mdamagem() */
    }

    /*
     *  All of this manipulation is needed to keep the display correct.
     *  There is a flush at the next pline().
     */
    ax = magr->mx;
    ay = magr->my;
    dx = mdef->mx;
    dy = mdef->my;
    /*
     *  Leave the defender in the monster chain at it's current position,
     *  but don't leave it on the screen.  Move the aggressor to the
     *  defender's position.
     */
    remove_monster(dx, dy);
    remove_monster(ax, ay);
    if (u.usteed && magr == u.usteed) {
	teleds(dx, dy, TELEDS_NO_FLAGS);
    } else {
        place_monster(magr, dx, dy);
        newsym(ax, ay); /* erase old position */
    }
    newsym(dx, dy); /* update new position */

    if (msteed != NULL)
        place_monster(msteed, ax, ay);

    status = mdamagem(magr, mdef, mattk, (struct obj *) 0, 0, &otmp);

    if ((status & (MM_AGR_DIED | MM_DEF_DIED))
        == (MM_AGR_DIED | MM_DEF_DIED)) {
        ;                              /* both died -- do nothing  */
    } else if (status & MM_DEF_DIED) { /* defender died */
        /*
         *  Note:  remove_monster() was called in relmon(), wiping out
         *  magr from level.monsters[mdef->mx][mdef->my].  We need to
         *  put it back and display it.  -kd
         */
	if (u.usteed && magr == u.usteed) {
	    teleds(dx, dy, TELEDS_NO_FLAGS);
	} else {
            place_monster(magr, dx, dy);
            newsym(dx, dy);
        }
        /* aggressor moves to <dx, dy> and might encounter trouble there */
        if (minliquid(magr) || (t_at(dx, dy) && mintrap(magr) == 2))
            status |= MM_AGR_DIED;
    } else if (status & MM_AGR_DIED) { /* aggressor died */
        place_monster(mdef, dx, dy);
        newsym(dx, dy);
    } else {                           /* both alive, put them back */
        if (cansee(dx, dy)) {
            if (status & MM_EXPELLED) {
                strcpy(buf, Monnam(magr));
		pline("%s hurriedly regurgitates %s!", buf, mon_nam(mdef));
                pline("Obviously, it didn't like %s taste.", s_suffix(mon_nam(mdef)));
            } else {
                pline("%s is regurgitated!", Monnam(mdef));
            }
        }

        remove_monster(dx, dy);

	if (u.usteed && magr == u.usteed) {
	    teleds(ax, ay, TELEDS_NO_FLAGS);
	} else {
            place_monster(magr, ax, ay);
            newsym(ax, ay);
        }
        place_monster(mdef, dx, dy);
        newsym(dx, dy);
    }
    return status;
}

STATIC_OVL int
explmm(magr, mdef, mattk)
struct monst *magr, *mdef;
struct attack *mattk;
{
    struct obj *otmp;
    int result, mndx, tmp;

    if (magr->mcan)
        return MM_MISS;

    if (cansee(magr->mx, magr->my))
        pline("%s explodes!", Monnam(magr));
    else
        noises(magr, mattk);

    /* monster explosion types which actually create an explosion */
    if (mattk->adtyp == AD_FIRE || mattk->adtyp == AD_COLD
        || mattk->adtyp == AD_ELEC || mattk->adtyp == AD_ACID) {
        mon_explodes(magr, mattk);
        if (mdef && mattk->adtyp == AD_ACID) {
            if (rn2(4))
                erode_armor(mdef, ERODE_CORRODE);
            if (rn2(2))
                acid_damage(MON_WEP(mdef));
        }
        /* unconditionally set AGR_DIED here; lifesaving is accounted below */
        result = MM_AGR_DIED | (DEADMONSTER(mdef) ? MM_DEF_DIED : 0);
        /* kludge to ensure the player gets experience if spheres
           were generated via spell */
        if (magr->uexp && (result & MM_DEF_DIED)) {
            mndx = monsndx(mdef->data);
            tmp = experience(mdef, (int) mvitals[mndx].died + 1);
            more_experienced(tmp, 0);
            newexplevel();
        }
    } else {
        result = mdamagem(magr, mdef, mattk, (struct obj *) 0, 0, &otmp);
    }

    /* Kill off aggressor if it didn't die. */
    if (!(result & MM_AGR_DIED)) {
        boolean was_leashed = (magr->mleashed != 0);

        mondead(magr);
        if (!DEADMONSTER(magr))
            return result; /* life saved */
        result |= MM_AGR_DIED;

        /* mondead() -> m_detach() -> m_unleash() always suppresses
           the m_unleash() slack message, so deliver it here instead */
        if (was_leashed)
            Your("leash falls slack.");
    }
    /* KMH -- Player gets blame for flame/freezing sphere */
    if (magr->msummoned && !(result & MM_DEF_DIED))
        setmangry(mdef, TRUE);
    if (magr->mtame && !magr->msummoned) /* give this one even if it was visible */
        You(brief_feeling, "melancholy");

    return result;
}

/*
 *  See comment at top of mattackm(), for return values.
 */
STATIC_OVL int
mdamagem(magr, mdef, mattk, mwep, dieroll, ootmp)
struct monst *magr, *mdef;
struct attack *mattk;
struct obj *mwep;
int dieroll;
struct obj **ootmp; /* to return worn armor for caller to disintegrate */
{
    struct obj *obj;
    char buf[BUFSZ];
    char saved_oname[BUFSZ];
    struct permonst *pa = magr->data, *pd = mdef->data;
    int armpro, num,
        tmp = d((int) mattk->damn, (int) mattk->damd),
        res = MM_MISS;
    boolean cancelled, lightobj = FALSE,
            ispoisoned = FALSE, istainted = FALSE;
    struct obj* hated_obj;
    long armask;
    boolean mon_vorpal_wield  = (MON_WEP(mdef)
                                 && MON_WEP(mdef)->oartifact == ART_VORPAL_BLADE);
    boolean mon_tempest_wield = (MON_WEP(mdef)
                                 && MON_WEP(mdef)->oartifact == ART_TEMPEST);

    saved_oname[0] = '\0';

    if ((touch_petrifies(pd) /* or flesh_petrifies() */
         || (mattk->adtyp == AD_DGST && pd == &mons[PM_MEDUSA]))
        && !(resists_ston(magr) || defended(magr, AD_STON))) {
        long protector = attk_protection((int) mattk->aatyp),
             wornitems = magr->misc_worn_check;

        /* wielded weapon gives same protection as gloves here */
        if (mwep)
            wornitems |= W_ARMG;

        if (protector == 0L
            || (protector != ~0L && (wornitems & protector) != protector)) {
            if (poly_when_stoned(pa)) {
                mon_to_stone(magr);
                return MM_HIT; /* no damage during the polymorph */
            }
            if (vis && canspotmon(magr))
                pline("%s turns to stone!", Monnam(magr));
            monstone(magr);
            if (!DEADMONSTER(magr))
                return MM_HIT; /* lifesaved */
            else if (magr->mtame && !vis)
                You(brief_feeling, "peculiarly sad");
            return MM_AGR_DIED;
        }
    }

    /* find rings of increase damage */
    if (magr->minvent) {
	struct obj *o;
	for (o = magr->minvent; o; o = o->nobj)
	    if (o->owornmask && o->otyp == RIN_INCREASE_DAMAGE)
	        tmp += o->spe;
    }

    /* cancellation factor is the same as when attacking the hero */
    armpro = magic_negation(mdef);
    cancelled = magr->mcan || !(rn2(10) >= 3 * armpro);

    /* check for special damage sources (e.g. hated material) */
    armask = attack_contact_slots(magr, mattk->aatyp);
    tmp += special_dmgval(magr, mdef, armask, &hated_obj);

    if (hated_obj) {
        if (vis && canseemon(mdef))
            searmsg(magr, mdef, hated_obj, FALSE);
        if (DEADMONSTER(mdef))
            return (MM_DEF_DIED
                    | (grow_up(magr, mdef) ? 0 : MM_AGR_DIED));
    }

    if (artifact_light(mwep) && mwep->lamplit) {
        Strcpy(saved_oname, bare_artifactname(mwep));
        saved_oname[0] = highc(saved_oname[0]);
    }

    if (artifact_light(mwep) && mwep->lamplit
        && mon_hates_light(mdef))
        lightobj = TRUE;

    if (lightobj) {
        const char *fmt;
        char *whom = mon_nam(mdef);
        char emitlightobjbuf[BUFSZ];

        if (canspotmon(mdef)) {
            if (saved_oname[0]) {
                Sprintf(emitlightobjbuf,
                        "%s radiance penetrates deep into",
                        s_suffix(saved_oname));
                Strcat(emitlightobjbuf, " %s!");
                fmt = emitlightobjbuf;
            } else
                fmt = "The light sears %s!";
        } else {
            *whom = highc(*whom); /* "it" -> "It" */
            fmt = "%s is seared!";
        }
        /* note: s_suffix returns a modifiable buffer */
        if (!noncorporeal(pd) && !amorphous(pd))
            whom = strcat(s_suffix(whom), " flesh");
        pline(fmt, whom);
    }

    switch (mattk->adtyp) {
    case AD_DGST: {
        struct obj *sbarding;

        if (mon_prop(mdef, SLOW_DIGESTION))
            return (MM_HIT | MM_EXPELLED);
        if ((sbarding = which_armor(mdef, W_BARDING)) != 0
            && (sbarding->otyp == SPIKED_BARDING
                || sbarding->otyp == RUNED_BARDING))
            return (MM_HIT | MM_EXPELLED);
        /* eating a Rider or its corpse is fatal */
        if (is_rider(pd)) {
            if (vis && canseemon(magr))
                pline("%s %s!", Monnam(magr),
                      (pd == &mons[PM_FAMINE])
                          ? "belches feebly, shrivels up and dies"
                          : (pd == &mons[PM_PESTILENCE])
                                ? "coughs spasmodically and collapses"
                                : "vomits violently and drops dead");
            mondied(magr);
            if (!DEADMONSTER(magr))
                return 0; /* lifesaved */
            else if (magr->mtame && !vis)
                You(brief_feeling, "queasy");
            return MM_AGR_DIED;
        }
        if (flags.verbose && !Deaf)
            verbalize("Burrrrp!");
        tmp = mdef->mhp;
        /* Use up amulet of life saving */
        if ((obj = mlifesaver(mdef)) != 0)
            m_useup(mdef, obj);

        /* Is a corpse for nutrition possible?  It may kill magr */
        if (!corpse_chance(mdef, magr, TRUE) || DEADMONSTER(magr))
            break;

        /* Pets get nutrition from swallowing monster whole.
         * No nutrition from G_NOCORPSE monster, eg, undead.
         * DGST monsters don't die from undead corpses
         */
        num = monsndx(pd);
        if (magr->mtame && !magr->isminion
            && !(mvitals[num].mvflags & G_NOCORPSE)) {
            struct obj *virtualcorpse = mksobj(CORPSE, FALSE, FALSE);
            int nutrit;

            set_corpsenm(virtualcorpse, num);
            nutrit = dog_nutrition(magr, virtualcorpse);
            dealloc_obj(virtualcorpse);

            /* only 50% nutrition, 25% of normal eating time */
            if (magr->meating > 1)
                magr->meating = (magr->meating + 3) / 4;
            if (nutrit > 1)
                nutrit /= 2;
            EDOG(magr)->hungrytime += nutrit;
            dog_givit(magr, pd);
        }
        break;
    }
    case AD_STUN:
        if (magr->mcan)
            break;
        if (resists_stun(mdef->data)
            || defended(mdef, AD_STUN) || mon_tempest_wield) {
            ; /* immune */
            break;
        }
        if (mattk->aatyp == AT_GAZE) {
            if (vis) {
                if (mdef->data->mlet == S_MIMIC
                    && M_AP_TYPE(mdef) != M_AP_NOTHING)
                    seemimic(mdef);
                Sprintf(buf, "%s gazes at", Monnam(magr));
                pline("%s %s...", buf,
                      canspotmon(mdef) ? mon_nam(mdef) : "something");
            }

            if (magr->mcan || !magr->mcansee || !mdef->mcansee
                || (magr->minvis && !racial_perceives(mdef)) || mdef->msleeping) {
                if (vis && canspotmon(mdef))
                    pline("But nothing happens.");
                return MM_MISS;
            }
        }
        if (canseemon(mdef))
            pline("%s %s for a moment.", Monnam(mdef),
                  makeplural(stagger(pd, "stagger")));
        mdef->mstun = 1;
        goto physical;
    case AD_LEGS:
        if (magr->mcan) {
            tmp = 0;
            break;
        }
        goto physical;
    case AD_BHED: {
        struct obj *barding;

        if ((!rn2(15) || is_jabberwock(mdef->data))
            && !magr->mcan) {
            Strcpy(buf, Monnam(magr));
            if (!has_head(mdef->data) || mon_vorpal_wield) {
                if (canseemon(mdef))
                    pline("%s somehow misses %s wildly.", buf, mon_nam(mdef));
                tmp = 0;
                break;
            }
            if ((barding = which_armor(mdef, W_BARDING)) != 0
                && barding->oartifact == ART_ITHILMAR) {
                if (canseemon(mdef))
                    pline("Its attack glances harmlessly off of %s barding.",
                          s_suffix(mon_nam(mdef)));
                tmp = 0;
                break;
            }
            if (noncorporeal(mdef->data) || amorphous(mdef->data)) {
                if (canseemon(mdef))
                    pline("%s slices through %s %s.",
                          buf, s_suffix(mon_nam(mdef)), mbodypart(mdef, NECK));
                goto physical;
            }
            if (mdef->data == &mons[PM_CERBERUS]) {
                if (canseemon(mdef)) {
                    pline("%s removes one of %s heads!", buf,
                          s_suffix(mon_nam(mdef)));
                    You("watch in horror as it quickly grows back.");
                }
                tmp = rn2(15) + 10;
                goto physical;
            }
            if (canseemon(mdef))
                pline("%s %ss %s!", buf,
                      rn2(2) ? "behead" : "decapitate", mon_nam(mdef));
            mondied(mdef);
            if (mdef->mhp > 0)
                return 0;
            if (racial_zombie(mdef) || is_troll(mdef->data))
                mdef->mcan = 1; /* no head? no reviving */
            return (MM_DEF_DIED | (grow_up(magr, mdef) ? 0 : MM_AGR_DIED));
        }
        break;
    }
    case AD_WERE:
    case AD_HEAL:
    case AD_CLOB:
    case AD_PHYS:
 physical:
        if (mattk->aatyp != AT_WEAP && mattk->aatyp != AT_CLAW)
            mwep = 0;

        if (magr->mberserk && !rn2(3))
            tmp += d((int) mattk->damn, (int) mattk->damd);

        if (shade_miss(magr, mdef, mwep, FALSE, TRUE)) {
            tmp = 0;
        } else if (mattk->aatyp == AT_KICK && thick_skinned(pd)) {
            /* [no 'kicking boots' check needed; monsters with kick attacks
               can't wear boots and monsters that wear boots don't kick] */
            tmp = 0;
        } else if (mwep) { /* non-Null 'mwep' implies AT_WEAP || AT_CLAW */
            struct obj *marmg;

            if (mwep->otyp == CORPSE
                && touch_petrifies(&mons[mwep->corpsenm]))
                goto do_stone;

            tmp += dmgval(mwep, mdef);
            if ((marmg = which_armor(magr, W_ARMG)) != 0
                && marmg->otyp == GAUNTLETS_OF_POWER)
                tmp += rn1(4, 3); /* 3..6 */
            if (tmp < 1) /* is this necessary?  mhitu.c has it... */
                tmp = 1;
            if (mwep->oartifact
                || ((mwep->oclass == WEAPON_CLASS
                     || is_weptool(mwep) || is_bullet(mwep))
                    && mwep->oprops)) {
                (void) artifact_hit(magr, mdef, mwep, &tmp, dieroll);
                if (DEADMONSTER(mdef))
                    return (MM_DEF_DIED
                            | (grow_up(magr, mdef) ? 0 : MM_AGR_DIED));
            }
            if (mon_hates_material(mdef, mwep->material)) {
                /* extra damage already applied by dmgval() */
                if (vis && canseemon(mdef))
                    searmsg(magr, mdef, mwep, FALSE);
                if (DEADMONSTER(mdef))
                    return (MM_DEF_DIED
                            | (grow_up(magr, mdef) ? 0 : MM_AGR_DIED));
            }

            if (mwep->opoisoned && is_poisonable(mwep))
                ispoisoned = TRUE;

            if (mwep->otainted && is_poisonable(mwep))
                istainted = TRUE;

            /* monster attacking with a poisoned weapon */
            if (ispoisoned) {
                int nopoison;

                Sprintf(buf, "%s %s", s_suffix(Monnam(magr)),
                        mpoisons_subj(magr, mattk));

                if (resists_poison(mdef) || defended(mdef, AD_DRST)) {
                    if (vis && canseemon(mdef))
                        pline_The("poison doesn't seem to affect %s.",
                                  mon_nam(mdef));
                } else {
                    if (rn2(30)) {
                        tmp += rnd(6);
                    } else {
                        if (vis && canseemon(mdef))
                            pline_The("poison was deadly...");
                        mondied(mdef);
                    }
                }

                nopoison = (10 - (mwep->owt / 10));
                if (nopoison < 2)
                    nopoison = 2;
                if (mwep && !rn2(nopoison)) {
                    mwep->opoisoned = FALSE;
                    if (vis && canseemon(magr))
                        pline("%s %s is no longer poisoned.",
                              s_suffix(Monnam(magr)), xname(mwep));
                }
                if (DEADMONSTER(mdef))
                    return (MM_DEF_DIED
                            | (grow_up(magr, mdef) ? 0 : MM_AGR_DIED));
            }
            /* monster attacking with a tainted (drow-poisoned) weapon */
            if (istainted) {
                int notaint;

                Sprintf(buf, "%s %s", s_suffix(Monnam(magr)),
                        mpoisons_subj(magr, mattk));

                if (resists_sleep(mdef) || defended(mdef, AD_SLEE)) {
                    if (vis && canseemon(mdef))
                        pline_The("drow poison doesn't seem to affect %s.",
                                  mon_nam(mdef));
                } else {
                    if (!rn2(3)) {
                        tmp += rnd(2);
                    } else {
                        if (sleep_monst(mdef, rn2(3) + 2, WEAPON_CLASS)) {
                            if (vis && canseemon(mdef))
                                pline("%s loses consciousness.", Monnam(mdef));
                            slept_monst(mdef);
                        }
                    }
                }

                notaint = ((is_drow_weapon(mwep) ? 20 : 5) - (mwep->owt / 10));
                if (notaint < 2)
                    notaint = 2;
                if (mwep && !rn2(notaint)) {
                    mwep->otainted = FALSE;
                    if (vis && canseemon(magr))
                        pline("%s %s is no longer tainted.",
                              s_suffix(Monnam(magr)), xname(MON_WEP(magr)));
                }
                if (DEADMONSTER(mdef))
                    return (MM_DEF_DIED
                            | (grow_up(magr, mdef) ? 0 : MM_AGR_DIED));
            }
            if (tmp)
                rustm(mdef, mwep);
        } else if (pa == &mons[PM_PURPLE_WORM] && pd == &mons[PM_SHRIEKER]) {
            /* hack to enhance mm_aggression(); we don't want purple
               worm's bite attack to kill a shrieker because then it
               won't swallow the corpse; but if the target survives,
               the subsequent engulf attack should accomplish that */
            if (tmp >= mdef->mhp && mdef->mhp > 1)
                tmp = mdef->mhp - 1;
        }
        if (mattk->adtyp == AD_CLOB && tmp > 0
            && !unsolid(pd) && !rn2(6)) {
            if (tmp < mdef->mhp) {
                if (vis && canseemon(mdef))
                    pline("%s knocks %s back with a %s %s!",
                          Monnam(magr), mon_nam(mdef),
                          rn2(2) ? "forceful" : "powerful",
                          rn2(2) ? "blow" : "strike");
                if (mdef == u.usteed) {
                    newsym(u.usteed->mx, u.usteed->my);
                    dismount_steed(DISMOUNT_FELL);
                }
                if (!DEADMONSTER(mdef))
                    mhurtle(mdef, mdef->mx - magr->mx,
                            mdef->my - magr->my, rnd(2));
                if (DEADMONSTER(mdef))
                    return (MM_DEF_DIED
                            | (grow_up(magr, mdef) ? 0 : MM_AGR_DIED));
            }
        }
        break;
    case AD_FIRE:
        if (cancelled) {
            tmp = 0;
            break;
        }
        if (mattk->aatyp == AT_GAZE) {
            if (vis) {
                if (mdef->data->mlet == S_MIMIC
                    && M_AP_TYPE(mdef) != M_AP_NOTHING)
                    seemimic(mdef);
                Sprintf(buf, "%s gazes at", Monnam(magr));
                pline("%s %s...", buf,
                      canspotmon(mdef) ? mon_nam(mdef) : "something");
            }

            if (magr->mcan || !magr->mcansee || !mdef->mcansee
                || (magr->minvis && !racial_perceives(mdef))
                || mdef->msleeping || mon_underwater(mdef)) {
                if (vis && canspotmon(mdef))
                    pline("But nothing happens.");
                return MM_MISS;
            }
        }
        if (vis && canseemon(mdef))
            pline("%s is %s!", Monnam(mdef), on_fire(mdef, mattk->aatyp == AT_HUGS ? ON_FIRE_HUG : ON_FIRE));
        if (completelyburns(pd)) { /* paper golem or straw golem */
            if (vis && canseemon(mdef))
                pline("%s burns completely!", Monnam(mdef));
            mondead(mdef); /* was mondied() but that dropped paper scrolls */
            if (!DEADMONSTER(mdef))
                return 0;
            else if (mdef->mtame && !vis)
                pline("May %s roast in peace.", mon_nam(mdef));
            return (MM_DEF_DIED | (grow_up(magr, mdef) ? 0 : MM_AGR_DIED));
        }
        if (!mon_underwater(mdef)) {
            tmp += destroy_mitem(mdef, SCROLL_CLASS, AD_FIRE);
            tmp += destroy_mitem(mdef, SPBOOK_CLASS, AD_FIRE);
        }
        if (resists_fire(mdef) || defended(mdef, AD_FIRE)) {
            if (vis && canseemon(mdef))
                pline_The("fire doesn't seem to burn %s!", mon_nam(mdef));
            shieldeff(mdef->mx, mdef->my);
            golemeffects(mdef, AD_FIRE, tmp);
            tmp = 0;
        }
        /* only potions damage resistant players in destroy_item */
        tmp += destroy_mitem(mdef, POTION_CLASS, AD_FIRE);
        break;
    case AD_COLD:
        if (cancelled) {
            tmp = 0;
            break;
        }
        if (mattk->aatyp == AT_GAZE) {
            if (vis) {
                if (mdef->data->mlet == S_MIMIC
                    && M_AP_TYPE(mdef) != M_AP_NOTHING)
                    seemimic(mdef);
                Sprintf(buf, "%s gazes at", Monnam(magr));
                pline("%s %s...", buf,
                      canspotmon(mdef) ? mon_nam(mdef) : "something");
            }

            if (magr->mcan || !magr->mcansee || !mdef->mcansee
                || (magr->minvis && !racial_perceives(mdef)) || mdef->msleeping) {
                if (vis && canspotmon(mdef))
                    pline("But nothing happens.");
                return MM_MISS;
            }
        }
        if (vis && canseemon(mdef))
            pline("%s is covered in frost!", Monnam(mdef));
        if (resists_cold(mdef) || defended(mdef, AD_COLD)) {
            if (vis && canseemon(mdef))
                pline_The("frost doesn't seem to chill %s!", mon_nam(mdef));
            shieldeff(mdef->mx, mdef->my);
            golemeffects(mdef, AD_COLD, tmp);
            tmp = 0;
        }
        tmp += destroy_mitem(mdef, POTION_CLASS, AD_COLD);
        break;
    case AD_LOUD:
        if (cancelled) {
            tmp = 0;
            break;
        }

        if (!mdef->mstun) {
            if (!(resists_stun(mdef->data)
                  || defended(mdef, AD_STUN) || mon_tempest_wield))
                mdef->mstun = 1;
        }

        if (!rn2(6))
            erode_armor(mdef, ERODE_FRACTURE);
        tmp += destroy_mitem(mdef, RING_CLASS, AD_LOUD);
        tmp += destroy_mitem(mdef, TOOL_CLASS, AD_LOUD);
        tmp += destroy_mitem(mdef, WAND_CLASS, AD_LOUD);
        tmp += destroy_mitem(mdef, POTION_CLASS, AD_LOUD);
        if (pd == &mons[PM_GLASS_GOLEM]) {
            pline("%s shatters into a million pieces!", Monnam(mdef));
            mondied(mdef);
            if (mdef->mhp > 0)
                return 0;
            return (MM_DEF_DIED | (grow_up(magr, mdef) ? 0 : MM_AGR_DIED));
        }
        break;
    case AD_ELEC:
        if (cancelled) {
            tmp = 0;
            break;
        }
        if (vis && canseemon(mdef))
            pline("%s gets zapped!", Monnam(mdef));
        tmp += destroy_mitem(mdef, WAND_CLASS, AD_ELEC);
        if (resists_elec(mdef) || defended(mdef, AD_ELEC)) {
            if (vis && canseemon(mdef))
                pline_The("zap doesn't shock %s!", mon_nam(mdef));
            shieldeff(mdef->mx, mdef->my);
            golemeffects(mdef, AD_ELEC, tmp);
            tmp = 0;
        }
        /* only rings damage resistant players in destroy_item */
        tmp += destroy_mitem(mdef, RING_CLASS, AD_ELEC);
        break;
    case AD_ACID:
        if (magr->mcan) {
            tmp = 0;
            break;
        }
        if (resists_acid(mdef) || defended(mdef, AD_ACID)
            || mon_underwater(mdef)) {
            if (vis && canseemon(mdef))
                pline("%s is covered in %s, but it seems harmless.",
                      Monnam(mdef), hliquid("acid"));
            tmp = 0;
        } else if (vis && canseemon(mdef)) {
            pline("%s is covered in %s!", Monnam(mdef), hliquid("acid"));
            pline("It burns %s!", mon_nam(mdef));
        }
        if (!mon_underwater(mdef)) {
            if (!rn2(30))
                erode_armor(mdef, ERODE_CORRODE);
            if (!rn2(6))
                acid_damage(MON_WEP(mdef));
        }
        break;
    case AD_RUST:
        if (magr->mcan)
            break;
        if (pd == &mons[PM_IRON_GOLEM]) {
            if (vis && canseemon(mdef))
                pline("%s falls to pieces!", Monnam(mdef));
            mondied(mdef);
            if (!DEADMONSTER(mdef))
                return 0;
            else if (mdef->mtame && !vis)
                pline("May %s rust in peace.", mon_nam(mdef));
            return (MM_DEF_DIED | (grow_up(magr, mdef) ? 0 : MM_AGR_DIED));
        }
        erode_armor(mdef, ERODE_RUST);
        mdef->mstrategy &= ~STRAT_WAITFORU;
        tmp = 0;
        break;
    case AD_CORR:
        if (magr->mcan)
            break;
        erode_armor(mdef, ERODE_CORRODE);
        mdef->mstrategy &= ~STRAT_WAITFORU;
        tmp = 0;
        break;
    case AD_DCAY:
        if (magr->mcan)
            break;
        if (pd == &mons[PM_WOOD_GOLEM] || pd == &mons[PM_LEATHER_GOLEM]) {
            if (vis && canseemon(mdef))
                pline("%s falls to pieces!", Monnam(mdef));
            mondied(mdef);
            if (!DEADMONSTER(mdef))
                return 0;
            else if (mdef->mtame && !vis)
                pline("May %s rot in peace.", mon_nam(mdef));
            return (MM_DEF_DIED | (grow_up(magr, mdef) ? 0 : MM_AGR_DIED));
        }
        erode_armor(mdef, ERODE_CORRODE);
        mdef->mstrategy &= ~STRAT_WAITFORU;
        tmp = 0;
        break;
    case AD_STON:
        if (magr->mcan)
            break;
 do_stone:
        /* may die from the acid if it eats a stone-curing corpse */
        if (munstone(mdef, FALSE))
            goto post_stone;
        if (poly_when_stoned(pd)) {
            mon_to_stone(mdef);
            tmp = 0;
            break;
        }
        if (!(resists_ston(mdef) || defended(mdef, AD_STON))) {
            if (mattk->aatyp == AT_GAZE) {
                if (magr->data == &mons[PM_MEDUSA]) {
                    if (vis && canseemon(mdef))
                        pline("%s turns to stone!", Monnam(mdef));
                    monstone(mdef);
post_stone:
                    if (!DEADMONSTER(mdef))
                        return MM_MISS;
                    else if (mdef->mtame && !vis)
                        You(brief_feeling, "peculiarly sad");
                    return (MM_DEF_DIED | (grow_up(magr, mdef)
                            ? 0 : MM_AGR_DIED));
                } else if ((magr->data == &mons[PM_BEHOLDER]
                            || magr->data == &mons[PM_TAL_GATH]) && !rn2(3)) {
                    if (vis) {
                        if (mdef->data->mlet == S_MIMIC
                            && M_AP_TYPE(mdef) != M_AP_NOTHING)
                            seemimic(mdef);
                        Sprintf(buf, "%s gazes at", Monnam(magr));
                        pline("%s %s...", buf,
                              canspotmon(mdef) ? mon_nam(mdef) : "something");
                    }

                    if (magr->mcan || !magr->mcansee || !mdef->mcansee
                        || (magr->minvis && !racial_perceives(mdef)) || mdef->msleeping) {
                        if (vis && canspotmon(mdef))
                            pline("But nothing happens.");
                        return MM_MISS;
                    }
                    if (vis && canseemon(mdef) && !mdef->mstone)
                        pline("%s is slowing down.", Monnam(mdef));
                    if (!mdef->mstone) {
                        mdef->mstone = 5;
                        mdef->mstonebyu = FALSE;
                    }
                }
            } else {
                if (vis && canseemon(mdef) && !mdef->mstone)
                    pline("%s is slowing down.", Monnam(mdef));
                if (!mdef->mstone) {
                    mdef->mstone = 5;
                    mdef->mstonebyu = FALSE;
                }
            }
        }
        tmp = (mattk->adtyp == AD_STON ? 0 : 1);
        break;
    case AD_TLPT:
        if (!cancelled && tmp < mdef->mhp && !tele_restrict(mdef)) {
            char mdef_Monnam[BUFSZ];
            boolean wasseen = canspotmon(mdef);

            /* save the name before monster teleports, otherwise
               we'll get "it" in the suddenly disappears message */
            if (vis && wasseen)
                Strcpy(mdef_Monnam, Monnam(mdef));
            /* works on other critters too.. */
            if (magr->mnum == PM_BOOJUM)
                mdef->perminvis = mdef->minvis = TRUE;
            mdef->mstrategy &= ~STRAT_WAITFORU;
            (void) rloc(mdef, TRUE);
            if (vis && wasseen && !canspotmon(mdef) && mdef != u.usteed)
                pline("%s suddenly disappears!", mdef_Monnam);
            if (tmp >= mdef->mhp) { /* see hitmu(mhitu.c) */
                if (mdef->mhp == 1)
                    ++mdef->mhp;
                tmp = mdef->mhp - 1;
            }
        }
        break;
    case AD_SLEE:
        if (!cancelled && !mdef->msleeping
            && sleep_monst(mdef, rnd(10), -1)) {
            if (mattk->aatyp == AT_GAZE) {
                if (vis) {
                    if (mdef->data->mlet == S_MIMIC
                        && M_AP_TYPE(mdef) != M_AP_NOTHING)
                        seemimic(mdef);
                    Sprintf(buf, "%s gazes at", Monnam(magr));
                    pline("%s %s...", buf,
                          canspotmon(mdef) ? mon_nam(mdef) : "something");
                }

                if (magr->mcan || !magr->mcansee || !mdef->mcansee
                    || (magr->minvis && !racial_perceives(mdef)) || mdef->msleeping) {
                    if (vis && canspotmon(mdef))
                        pline("But nothing happens.");
                    return MM_MISS;
                }
            }
            if (vis && canspotmon(mdef)) {
                Strcpy(buf, Monnam(mdef));
                pline("%s is put to sleep by %s.", buf, mon_nam(magr));
            }
            mdef->mstrategy &= ~STRAT_WAITFORU;
            slept_monst(mdef);
        }
        break;
    case AD_PLYS:
        if (!cancelled && mdef->mcanmove) {
            if (vis && canspotmon(mdef)) {
                Strcpy(buf, Monnam(mdef));
                if (has_free_action(mdef)) {
                    pline("%s stiffens momentarily.", Monnam(mdef));
                } else {
                    pline("%s is frozen by %s.", buf, mon_nam(magr));
                }
            }
            if (has_free_action(mdef))
                break;
            paralyze_monst(mdef, rnd(10));
        }
        break;
    case AD_SLOW:
        if (!cancelled && mdef->mspeed != MSLOW) {
            if (mattk->aatyp == AT_GAZE) {
                if (vis) {
                    if (mdef->data->mlet == S_MIMIC
                        && M_AP_TYPE(mdef) != M_AP_NOTHING)
                        seemimic(mdef);
                    Sprintf(buf, "%s gazes at", Monnam(magr));
                    pline("%s %s...", buf,
                          canspotmon(mdef) ? mon_nam(mdef) : "something");
                }

                if (magr->mcan || !magr->mcansee || !mdef->mcansee
                    || (magr->minvis && !racial_perceives(mdef)) || mdef->msleeping) {
                    if (vis && canspotmon(mdef))
                        pline("But nothing happens.");
                    return MM_MISS;
                }
            }
            if (mattk->aatyp == AT_GAZE || !rn2(3)) {
                mon_adjust_speed(mdef, -1, (struct obj *) 0);
                mdef->mstrategy &= ~STRAT_WAITFORU;
            }
        }
        break;
    case AD_LUCK:
        /* Monsters don't have luck, so fall through */
    case AD_CONF:
        /* Since confusing another monster doesn't have a real time
         * limit, setting spec_used would not really be right (though
         * we still should check for it).
         */
        if (!magr->mcan && !mdef->mconf && !magr->mspec_used) {
            if (mattk->aatyp == AT_GAZE) {
                if (vis) {
                    if (mdef->data->mlet == S_MIMIC
                        && M_AP_TYPE(mdef) != M_AP_NOTHING)
                        seemimic(mdef);
                    Sprintf(buf, "%s gazes at", Monnam(magr));
                    pline("%s %s...", buf,
                          canspotmon(mdef) ? mon_nam(mdef) : "something");
                }

                if (magr->mcan || !magr->mcansee || !mdef->mcansee
                    || (magr->minvis && !racial_perceives(mdef)) || mdef->msleeping) {
                    if (vis && canspotmon(mdef))
                        pline("But nothing happens.");
                    return MM_MISS;
                }
            }
            if (vis && canseemon(mdef))
                pline("%s looks confused.", Monnam(mdef));
            mdef->mconf = 1;
            mdef->mstrategy &= ~STRAT_WAITFORU;
        }
        break;
    case AD_BLND:
        tmp = 0;
        if (can_blnd(magr, mdef, mattk->aatyp, (struct obj *) 0)) {
            register unsigned rnd_tmp;

            if (mattk->aatyp == AT_GAZE) {
                if (vis) {
                    if (mdef->data->mlet == S_MIMIC
                        && M_AP_TYPE(mdef) != M_AP_NOTHING)
                        seemimic(mdef);
                    Sprintf(buf, "%s gazes at", Monnam(magr));
                    pline("%s %s...", buf,
                          canspotmon(mdef) ? mon_nam(mdef) : "something");
                }

                if (magr->mcan || !magr->mcansee || !mdef->mcansee
                    || (magr->minvis && !racial_perceives(mdef)) || mdef->msleeping) {
                    if (vis && canspotmon(mdef))
                        pline("But nothing happens.");
                    return MM_MISS;
                }
            }
            if (vis && mdef->mcansee && canspotmon(mdef))
                pline("%s is blinded.", Monnam(mdef));
            rnd_tmp = d((int) mattk->damn, (int) mattk->damd);
            if ((rnd_tmp += mdef->mblinded) > 127)
                rnd_tmp = 127;
            mdef->mblinded = rnd_tmp;
            mdef->mcansee = 0;
            mdef->mstrategy &= ~STRAT_WAITFORU;
        }

        /* light-haters can take damage from the intense light
           (yellow light explosion), blind or not */
        if (mattk->aatyp == AT_EXPL
            && hates_light(r_data(mdef))) {
            if (!Deaf)
                pline("%s cries out in pain!",
                      Monnam(mdef));
            tmp = rnd(5);
        }
        break;
    case AD_HALU:
        if (!magr->mcan && haseyes(pd) && mdef->mcansee
            && !mon_prop(mdef, HALLUC_RES)) {
            if (vis && canseemon(mdef))
                pline("%s looks %sconfused.", Monnam(mdef),
                      mdef->mconf ? "more " : "");
            mdef->mconf = 1;
            mdef->mstrategy &= ~STRAT_WAITFORU;
        }
        tmp = 0;
        break;
    case AD_CURS:
        if (!night() && (pa == &mons[PM_GREMLIN]))
            break;

        if (night() && (pa == &mons[PM_LAVA_GREMLIN]))
            break;

        if (!magr->mcan && !rn2(10)) {
            mdef->mcan = 1; /* cancelled regardless of lifesave */
            mdef->mstrategy &= ~STRAT_WAITFORU;
            if (is_were(pd) && pd->mlet != S_HUMAN)
                were_change(mdef);
            if (pd == &mons[PM_CLAY_GOLEM]) {
                if (vis && canseemon(mdef)) {
                    pline("Some writing vanishes from %s head!",
                          s_suffix(mon_nam(mdef)));
                    pline("%s is destroyed!", Monnam(mdef));
                }
                mondied(mdef);
                if (!DEADMONSTER(mdef))
                    return 0;
                else if (mdef->mtame && !vis)
                    You(brief_feeling, "strangely sad");
                return (MM_DEF_DIED
                        | (grow_up(magr, mdef) ? 0 : MM_AGR_DIED));
            }
            if (!Deaf) {
                if (!vis)
                    You_hear("laughter.");
                else if (canseemon(magr))
                    pline("%s chuckles.", Monnam(magr));
            }
        }
        break;
    case AD_SGLD:
        tmp = 0;
        if (magr->mcan)
            break;
        /* technically incorrect; no check for stealing gold from
         * between mdef's feet...
         */
        {
            struct obj *gold = findgold(mdef->minvent, FALSE);

            if (!gold)
                break;
            /* print first so yname prints proper monster */
            if (vis && canseemon(mdef)) {
                Strcpy(buf, Monnam(magr));
                pline("%s steals %s.", buf, distant_name(gold, yname));
            }
            obj_extract_self(gold);
            add_to_minv(magr, gold);
            mdef->mstrategy &= ~STRAT_WAITFORU;
            if (!tele_restrict(magr)) {
                boolean couldspot = canspotmon(magr);
                (void) rloc(magr, TRUE);
                if (vis && couldspot && !canspotmon(magr))
                    pline("%s suddenly disappears!", buf);
            }
        }
        break;
    case AD_DRLI:
        if (!cancelled && !rn2(3)
            && !(resists_drli(mdef) || defended(mdef, AD_DRLI))) {
            tmp = d(2, 6);
            if (vis && canspotmon(mdef))
                pline("%s suddenly seems weaker!", Monnam(mdef));
            mdef->mhpmax -= tmp;
            if (mdef->m_lev == 0)
                tmp = mdef->mhp;
            else
                mdef->m_lev--;
            /* Automatic kill if drained past level 0 */
        }
        break;
    case AD_SSEX:
    case AD_SITM: /* for now these are the same */
    case AD_SEDU:
        if (magr->mcan)
            break;
        /* find an object to steal, non-cursed if magr is tame */
        int inv_tmp = 0;
        for (obj = mdef->minvent; obj; obj = obj->nobj) {
            if (!magr->mtame || !obj->cursed)
                ++inv_tmp;
        }
        if (inv_tmp)
            inv_tmp = rnd(inv_tmp);
        for (obj = mdef->minvent; obj; obj = obj->nobj) {
            if (!magr->mtame || !obj->cursed) {
                --inv_tmp;
                if (inv_tmp < 1)
                    break;
            }
        }

        if (obj) {
            char onambuf[BUFSZ], mdefnambuf[BUFSZ];

            /* make a special x_monnam() call that never omits
               the saddle, and save it for later messages */
            Strcpy(mdefnambuf,
                   x_monnam(mdef, ARTICLE_THE, (char *) 0, 0, FALSE));

            /* greased objects are difficult to get a grip on, hence
               the odds that an attempt at stealing it may fail */
            if ((obj->greased || obj->otyp == OILSKIN_CLOAK
                 || obj->otyp == OILSKIN_SACK
                 || obj->oartifact == ART_BAG_OF_THE_HESPERIDES
                 || (obj->oprops & ITEM_OILSKIN))
                && (!obj->cursed || rn2(4))) {
                if (vis && canseemon(mdef)) {
                    pline("%s %s slip off of %s's %s %s!", s_suffix(Monnam(magr)),
                          makeplural(mbodypart(magr, HAND)),
                          mdefnambuf,
                          obj->greased ? "greased" : "slippery",
                          (obj->greased || objects[obj->otyp].oc_name_known)
                              ? xname(obj)
                              : cloak_simple_name(obj));
                }
                if (obj->greased && !rn2(2)) {
                    if (vis && canseemon(mdef))
                        pline_The("grease wears off.");
                    obj->greased = 0;
                }
                break;
            }

            if (u.usteed == mdef && obj == which_armor(mdef, W_SADDLE))
                /* "You can no longer ride <steed>." */
                dismount_steed(DISMOUNT_POLY);
            obj_extract_self(obj);
            if (obj->owornmask) {
                mdef->misc_worn_check &= ~obj->owornmask;
                if (obj->owornmask & W_WEP)
                    mwepgone(mdef);
                obj->owornmask = 0L;
                update_mon_intrinsics(mdef, obj, FALSE, FALSE);
                /* give monster a chance to wear other equipment on its next
                   move instead of waiting until it picks something up */
                check_gear_next_turn(mdef);
            }
            /* add_to_minv() might free obj [if it merges] */
            if (vis)
                Strcpy(onambuf, doname(obj));
            (void) add_to_minv(magr, obj);
            if (vis && canseemon(mdef)) {
                Strcpy(buf, Monnam(magr));
                pline("%s steals %s from %s!", buf, onambuf, mdefnambuf);
            }
            possibly_unwield(mdef, FALSE);
            mdef->mstrategy &= ~STRAT_WAITFORU;
            mselftouch(mdef, (const char *) 0, FALSE);
            if (DEADMONSTER(mdef))
                return (MM_DEF_DIED
                        | (grow_up(magr, mdef) ? 0 : MM_AGR_DIED));
            if (pa->mlet == S_NYMPH && !tele_restrict(magr)) {
                boolean couldspot = canspotmon(magr);

                (void) rloc(magr, TRUE);
                if (vis && couldspot && !canspotmon(magr))
                    pline("%s suddenly disappears!", buf);
            }
        }
        tmp = 0;
        break;
    case AD_DREN:
        if (!cancelled && !rn2(4))
            xdrainenergym(mdef, (boolean) (vis && canspotmon(mdef)
                                           && mattk->aatyp != AT_ENGL));
        tmp = 0;
        break;
    case AD_DRST:
    case AD_DRDX:
    case AD_DRCO:
        if (!cancelled && !rn2(8)) {
            if (vis && canspotmon(magr))
                pline("%s %s was poisoned!", s_suffix(Monnam(magr)),
                      mpoisons_subj(magr, mattk));
            if (resists_poison(mdef)) {
                if (vis && canspotmon(mdef) && canspotmon(magr))
                    pline_The("poison doesn't seem to affect %s.",
                              mon_nam(mdef));
            } else {
                if (rn2(10))
                    tmp += rn1(10, 6);
                else {
                    if (vis && canspotmon(mdef))
                        pline_The("poison was deadly...");
                    tmp = mdef->mhp;
                }
            }
        }
        break;
    case AD_DISE:
        if (resists_sick(mdef) || defended(mdef, AD_DISE)) {
            if (vis && canseemon(mdef))
                pline("%s resists infection.", Monnam(mdef));
            tmp = 0;
            break;
        } else {
            if (mdef->mdiseasetime)
                mdef->mdiseasetime -= rnd(3);
            else
                mdef->mdiseasetime = rn1(9, 6);
            if (vis && canseemon(mdef))
                pline("%s looks %s.", Monnam(mdef),
                      mdef->mdiseased ? "even worse" : "diseased");
            mdef->mdiseased = 1;
            mdef->mdiseabyu = FALSE;
        }
        break;
    case AD_DRIN:
        if (notonhead || !has_head(pd)) {
            if (vis && canspotmon(mdef))
                pline("%s doesn't seem harmed.", Monnam(mdef));
            /* Not clear what to do for green slimes */
            tmp = 0;
            break;
        }
        if ((mdef->misc_worn_check & (W_ARMH | W_BARDING)) && rn2(8)) {
            if (vis && canspotmon(magr) && canseemon(mdef)) {
                Strcpy(buf, s_suffix(Monnam(mdef)));
                pline("%s %s blocks %s attack to %s %s.", buf,
                      which_armor(mdef, W_ARMH) ? "helmet" : "barding",
                      s_suffix(mon_nam(magr)), mhis(mdef), mbodypart(mdef, HEAD));
            }
            break;
        }
        if (racial_zombie(magr) && rn2(5)) {
            if (!(resists_sick(mdef) || defended(mdef, AD_DISE))) {
                if (vis && canspotmon(mdef))
                    pline("%s looks %s.", Monnam(mdef),
                          mdef->msick ? "much worse" : "rather ill");
                goto msickness;
            }
        }
        res = eat_brains(magr, mdef, vis, &tmp);
        break;
    case AD_DETH:
        if (mattk->aatyp == AT_GAZE) {
            if (vis) {
                if (mdef->data->mlet == S_MIMIC
                    && M_AP_TYPE(mdef) != M_AP_NOTHING)
                    seemimic(mdef);
                Sprintf(buf, "%s gazes at", Monnam(magr));
                pline("%s %s...", buf,
                      canspotmon(mdef) ? mon_nam(mdef) : "something");
            }

            if (magr->mcan || !magr->mcansee || !mdef->mcansee
                || (magr->minvis && !racial_perceives(mdef))
                || mdef->msleeping) {
                if (vis && canspotmon(mdef))
                    pline("But nothing happens.");
                return MM_MISS;
            }
        }
        if (immune_death_magic(mdef->data)
            || defended(mdef, AD_DETH)) {
            /* Still does normal damage */
            if (vis)
                pline("%s %s.", Monnam(mdef),
                      nonliving(mdef->data) ? "looks no more dead than before"
                                            : "is unaffected");
            break;
        }
        switch (rn2(20)) {
        case 19:
        case 18:
        case 17:
            if (!(resists_magm(mdef) || defended(mdef, AD_MAGM))
                && !resist(mdef, 0, 0, 0)) {
                if (mattk->aatyp == AT_GAZE)
                    pline("%s annihilates %s with %s deadly gaze!",
                          Monnam(magr), mon_nam(mdef), mhis(magr));
                else
                    pline("%s consumes %s life force!",
                          Monnam(magr), s_suffix(mon_nam(mdef)));
                mdef->mhp = 0;
                monkilled(mdef, (char *) 0, AD_DETH);
                if (!DEADMONSTER(mdef))
                    return 0;
                return (MM_DEF_DIED
                        | (grow_up(magr, mdef) ? 0 : MM_AGR_DIED));
            }
            break;
        default: /* case 16 through case 5 */
            if (vis)
                pline("%s looks weaker!", Monnam(mdef));
            /* mhp will then still be less than this value */
            mdef->mhpmax -= rn2(tmp / 2 + 1);
            if (mdef->mhpmax <= 0) /* protect against invalid value */
                mdef->mhpmax = 1;
            break;
        case 4:
        case 3:
        case 2:
        case 1:
        case 0:
            if (resists_magm(mdef) || defended(mdef, AD_MAGM))
                shieldeff(mdef->mx, mdef->my);
            if (vis)
                pline("Well.  That didn't work...");
            tmp = 0;
            break;
        }
        break;
    case AD_PEST:
        Strcpy(buf, mon_nam(mdef));
        if (vis) {
            if (resists_sick(mdef) || defended(mdef, AD_DISE)) {
                if (canseemon(mdef))
                    pline("%s reaches out, but %s looks unaffected.",
                          Monnam(magr), buf);
                else
                    pline("%s reaches out, and %s looks rather ill.",
                          Monnam(magr), buf);
            }
        }
        if ((mdef->mhpmax > 3) && !resist(mdef, 0, 0, NOTELL))
            mdef->mhpmax /= 2;
        if ((mdef->mhp > 2) && !resist(mdef, 0, 0, NOTELL))
            mdef->mhp /= 2;
        if (mdef->mhp > mdef->mhpmax)
            mdef->mhp = mdef->mhpmax;
msickness:
        if (resists_sick(mdef) || defended(mdef, AD_DISE))
            break;
        if (mdef->msicktime)
            mdef->msicktime -= rnd(3);
        else
            mdef->msicktime = rn1(9, 6);
        mdef->msick = (can_become_zombie(r_data(mdef))) ? 3 : 1;
        mdef->msickbyu = FALSE;
        break;
    case AD_FAMN:
        Strcpy(buf, s_suffix(mon_nam(mdef)));
        if (vis)
            pline("%s reaches out, and %s body shrivels.",
                  Monnam(magr), buf);
        if (mdef->mtame && !mdef->isminion)
            EDOG(mdef)->hungrytime -= rn1(120, 120);
        else {
            tmp += rnd(10); /* lacks a food rating */
            if (tmp >= mdef->mhp && vis)
                pline("%s starves.", Monnam(mdef));
        }
        /* plus the normal damage */
        break;
    case AD_SLIM:
        if (cancelled)
            break; /* physical damage only */
        if (!rn2(4) && !slimeproof(pd)) {
            if (!munslime(mdef, FALSE) && !DEADMONSTER(mdef)) {
                if (newcham(mdef, &mons[PM_GREEN_SLIME], FALSE,
                            (boolean) (vis && canseemon(mdef))))
                    pd = mdef->data;
                mdef->mstrategy &= ~STRAT_WAITFORU;
                res = MM_HIT;
            }
            /* munslime attempt could have been fatal,
               potentially to multiple monsters (SCR_FIRE) */
            if (DEADMONSTER(magr))
                res |= MM_AGR_DIED;
            if (DEADMONSTER(mdef))
                res |= MM_DEF_DIED;
            tmp = 0;
        }
        break;
    case AD_STCK:
        if (cancelled)
            tmp = 0;
        break;
    case AD_WRAP: /* monsters cannot grab one another, it's too hard */
        /* suffocation attack: negate damage for breathless. This isn't
         * affected by cancellation. */
        if (magr->mcan
            || (attacktype_fordmg(magr->data, AT_ENGL, AD_WRAP)
                && breathless(mdef->data))) {
            tmp = 0;
        }
        if (pa == &mons[PM_MIND_FLAYER_LARVA]) {
            if (can_become_flayer(mdef->data)) {
                if (rn2(6)) {
                    if (canseemon(mdef))
                        pline("%s tries to attach itself to %s %s!",
                              Monnam(magr), s_suffix(mon_nam(mdef)),
                              mbodypart(mdef, FACE));
                } else {
                    if (m_slips_free(mdef, mattk)) {
                        tmp = 0;
                    } else {
                        if (canseemon(mdef)) {
                            pline("%s wraps its tentacles around %s %s, attaching itself to its %s!",
                                  Monnam(magr), s_suffix(mon_nam(mdef)),
                                  mbodypart(mdef, HEAD), mbodypart(mdef, FACE));
                            pline("%s burrows itself into %s brain!",
                                  Monnam(magr), s_suffix(the(mon_nam(mdef))));
                        }
                        if (!mlifesaver(mdef)) {
                            boolean tamer = magr->mtame;
                            if (!tamer && (mdef->mtame || mdef->mpeaceful))
                                mdef->mtame = mdef->mpeaceful = 0;
                            mongone(magr); /* mind flayer larva transforms */
                            become_flayer(mdef);
                            if (tamer)
                                (void) tamedog(mdef, (struct obj *) 0);
                            return (MM_DEF_DIED
                                    | (grow_up(magr, mdef) ? 0 : MM_AGR_DIED));
                        } else {
                            tmp = mdef->mhp;
                        }
                    }
                }
            }
        }
        break;
    case AD_ENCH:
        /* there's no msomearmor() function, so just do damage */
        /* if (cancelled) break; */
        break;
    case AD_POLY:
        if (!magr->mcan && tmp < mdef->mhp)
            tmp = mon_poly(magr, mdef, tmp);
        break;
    case AD_WTHR: {
        uchar withertime = max(2, tmp);
        boolean no_effect =
            (nonliving(pd) /* This could use is_fleshy(), but that would
                              make a large set of monsters immune like
                              fungus, blobs, and jellies. */
             || is_vampshifter(mdef) || cancelled);
        boolean lose_maxhp = (withertime >= 8); /* if already withering */
        tmp = 0; /* doesn't deal immediate damage */

        if (!no_effect) {
            if (canseemon(mdef))
                pline("%s is withering away!", Monnam(mdef));

            if (mdef->mwither + withertime > UCHAR_MAX)
                mdef->mwither = UCHAR_MAX;
            else
                mdef->mwither += withertime;

            if (lose_maxhp && mdef->mhpmax > 1) {
                mdef->mhpmax--;
                mdef->mhp = min(mdef->mhp, mdef->mhpmax);
            }
            mdef->mwither_from_u = FALSE;
        }
        break;
    }
    case AD_DISN:
        if (!rn2(5)) {
            struct obj *otmp = (struct obj *) 0, *otmp2;

            if (mattk->aatyp == AT_GAZE) {
                if (vis) {
                    if (mdef->data->mlet == S_MIMIC
                        && M_AP_TYPE(mdef) != M_AP_NOTHING)
                        seemimic(mdef);
                    Sprintf(buf, "%s gazes at", Monnam(magr));
                    pline("%s %s...", buf,
                          canspotmon(mdef) ? mon_nam(mdef) : "something");
                }

                if (magr->mcan || !magr->mcansee || !mdef->mcansee
                    || (magr->minvis && !racial_perceives(mdef)) || mdef->msleeping) {
                    if (vis && canspotmon(mdef))
                        pline("But nothing happens.");
                    return MM_MISS;
                }
            }
            if (magr->mcan) {
                tmp = 0;
                break;
            }
            if (resists_disint(mdef) || defended(mdef, AD_DISN)) {
                shieldeff(mdef->mx, mdef->my);
                pline("%s basks in the %s aura of %s gaze.",
                      Monnam(mdef), hcolor(NH_BLACK),
                      s_suffix(mon_nam(magr)));
            } else if (mdef->misc_worn_check & W_ARMS) {
                *ootmp = which_armor(mdef, W_ARMS);
                pline ("%s %s crumbles away!", s_suffix(Monnam(mdef)),
                       xname(*ootmp));
                m_useup(mdef, *ootmp);
            } else if (mdef->misc_worn_check & W_ARM) {
                *ootmp = which_armor(mdef, W_ARM);
                pline ("%s %s turns to dust and blows away!",
                       s_suffix(Monnam(mdef)), xname(*ootmp));
                m_useup(mdef, *ootmp);
                if ((otmp2 = which_armor(mdef, W_ARMC)) != 0) {
                    pline ("%s %s crumbles and turns to dust!",
                           s_suffix(Monnam(mdef)), xname(otmp2));
                    m_useup(mdef, otmp2);
                }
            } else {
                struct obj *m_amulet = mlifesaver(mdef);
                if ((otmp2 = which_armor(mdef, W_ARMC)) != 0)
                    m_useup(mdef, otmp2);
                if ((otmp2 = which_armor(mdef, W_ARMU)) != 0)
                    m_useup(mdef, otmp2);
                if (is_rider(mdef->data)) {
                    pline("%s is disintegrated!", Monnam(mdef));
                    pline("%s body reintegrates before your %s!",
                          s_suffix(Monnam(mdef)),
                          (eyecount(youmonst.data) == 1)
                              ? body_part(EYE)
                              : makeplural(body_part(EYE)));
                    pline("%s resurrects!", Monnam(mdef));
                    mdef->mhp = mdef->mhpmax;

                    return MM_HIT;
                }
                if (canseemon(mdef)) {
                    if (!m_amulet)
                        pline("%s is disintegrated!", Monnam(mdef));
                    else
                        /* FIXME? the gaze? this handles other types of
                           disintegration attacks too */
                        pline("%s crumbles under the gaze!",
                              Monnam(mdef));
                }
/* note: worn amulet of life saving must be preserved in order to operate */
#define oresist_disintegration(obj)                                       \
    (objects[obj->otyp].oc_oprop == DISINT_RES || obj_resists(obj, 5, 50) \
     || is_quest_artifact(obj) || obj == m_amulet)

                for (otmp = mdef->minvent; otmp; otmp = otmp2) {
                    otmp2 = otmp->nobj;
                    if (!oresist_disintegration(otmp)) {
                        extract_from_minvent(mdef, otmp, TRUE, TRUE);
                        obfree(otmp, (struct obj *) 0);
                    }
                }

#undef oresist_disintegration

                mdef->mhp = 0;
                zombify = FALSE;
                if (magr->uexp)
                    mon_xkilled(mdef, (char *) 0, -AD_RBRE);
                else
                    monkilled(mdef, (char *) 0, -AD_RBRE);
                tmp = 0;
                if (DEADMONSTER(mdef))
                    res |= MM_DEF_DIED; /* not lifesaved */
                if (!grow_up(magr, mdef))
                    res |= MM_AGR_DIED;
                return res;
            }
        }
        break;
    case AD_CNCL: /* currently only called via AT_GAZE */
        if (!rn2(3)) {
            if (mattk->aatyp == AT_GAZE) {
                if (vis) {
                    if (mdef->data->mlet == S_MIMIC
                        && M_AP_TYPE(mdef) != M_AP_NOTHING)
                        seemimic(mdef);
                    Sprintf(buf, "%s gazes at", Monnam(magr));
                    pline("%s %s...", buf,
                          canspotmon(mdef) ? mon_nam(mdef) : "something");
                }

                if (magr->mcan || !magr->mcansee || !mdef->mcansee
                    || (magr->minvis && !racial_perceives(mdef)) || mdef->msleeping) {
                    if (vis && canspotmon(mdef))
                        pline("But nothing happens.");
                    return MM_MISS;
                }
            }
            if (magr->mcan) {
                tmp = 0;
                break;
            }
            (void) cancel_monst(mdef, (struct obj *) 0, FALSE, TRUE, FALSE);
        }
        break;
    case AD_PITS:
        if (rn2(2)) {
            if (!magr->mcan) {
                if (!create_pit_under(mdef, magr))
                    tmp = 0;
            }
        }
        break;
    case AD_WEBS:
        if (cancelled)
            break;
        if (!t_at(mdef->mx, mdef->my)) {
            struct trap *web = maketrap(mdef->mx, mdef->my, WEB);
            if (web) {
                mintrap(mdef);
                if (has_erid(mdef) && mdef->mtrapped) {
                    if (canseemon(mdef))
                        pline("%s falls off %s %s!",
                              Monnam(mdef), mhis(mdef), l_monnam(ERID(mdef)->mon_steed));
                    separate_steed_and_rider(mdef);
                }
            }
        }
        break;
    default:
        tmp = 0;
        break;
    }

    if (!tmp) {
        if (DEADMONSTER(mdef))
            res = MM_DEF_DIED;
        return res;
    }

    if (damage_mon(mdef, tmp, mattk->adtyp)) {
        if (m_at(mdef->mx, mdef->my) == magr) { /* see gulpmm() */
            remove_monster(mdef->mx, mdef->my);
            mdef->mhp = 1; /* otherwise place_monster will complain */
            place_monster(mdef, mdef->mx, mdef->my);
            mdef->mhp = 0;
        }

        zombify = !mwep && zombie_maker(magr)
            && can_become_zombie(r_data(mdef))
            && ((mattk->aatyp == AT_TUCH
                 || mattk->aatyp == AT_CLAW
                 || mattk->aatyp == AT_BITE)
                && zombie_form(r_data(mdef)) != NON_PM);
        if (magr->uexp)
            mon_xkilled(mdef, "", (int) mattk->adtyp);
        else
            monkilled(mdef, "", (int) mattk->adtyp);
        zombify = FALSE; /* reset */
        if (!DEADMONSTER(mdef))
            return res; /* mdef lifesaved */
        else if (res == MM_AGR_DIED)
            return (MM_DEF_DIED | MM_AGR_DIED);

        if (mattk->adtyp == AD_DGST) {
            /* various checks similar to dog_eat and meatobj.
             * after monkilled() to provide better message ordering */
            if (mdef->cham >= LOW_PM) {
                (void) newcham(magr, (struct permonst *) 0, FALSE, TRUE);
            } else if (pd == &mons[PM_GREEN_SLIME] && !slimeproof(pa)) {
                (void) newcham(magr, &mons[PM_GREEN_SLIME], FALSE, TRUE);
            } else if (pd == &mons[PM_WRAITH]) {
                (void) grow_up(magr, (struct monst *) 0);
                /* don't grow up twice */
                return (MM_DEF_DIED | (!DEADMONSTER(magr) ? 0 : MM_AGR_DIED));
            } else if (pd == &mons[PM_NURSE]) {
                magr->mhp = magr->mhpmax;
            }
        }
        /* caveat: above digestion handling doesn't keep `pa' up to date */

        return (MM_DEF_DIED | (grow_up(magr, mdef) ? 0 : MM_AGR_DIED));
    }
    return (res == MM_AGR_DIED) ? MM_AGR_DIED : MM_HIT;
}

int
mon_poly(magr, mdef, dmg)
struct monst *magr, *mdef;
int dmg;
{
    static const char freaky[] = " undergoes a freakish metamorphosis";

    if (mdef == &youmonst) {
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
        } else if (Unchanging) {
            ; /* just take a little damage */
        } else {
            /* system shock might take place in polyself() */
            if (u.ulycn == NON_PM) {
                You("are subjected to a freakish metamorphosis.");
                polyself(0);
            } else if (u.umonnum != u.ulycn) {
                You_feel("an unnatural urge coming on.");
                you_were();
            } else {
                You_feel("a natural urge coming on.");
                you_unwere(FALSE);
            }
            dmg = 0;
        }
    } else {
        char Before[BUFSZ];

        Strcpy(Before, Monnam(mdef));
        if (resists_magm(mdef) || defended(mdef, AD_MAGM)) {
            /* Magic resistance */
            if (vis)
                shieldeff(mdef->mx, mdef->my);
        } else if (resist(mdef, WAND_CLASS, 0, TELL)) {
            /* general resistance to magic... */
            ;
        } else if (!rn2(25) && mdef->cham == NON_PM
                   && (mdef->mcan
                       || pm_to_cham(monsndx(mdef->data)) != NON_PM)) {
            /* system shock; this variation takes away half of mon's HP
               rather than kill outright */
            if (vis)
                pline("%s shudders!", Before);

            dmg += (mdef->mhpmax + 1) / 2;
            mdef->mhp -= dmg;
            dmg = 0;
            if (DEADMONSTER(mdef)) {
                if (magr == &youmonst)
                    xkilled(mdef, XKILL_GIVEMSG | XKILL_NOCORPSE);
                else
                    monkilled(mdef, "", AD_RBRE);
            }
        } else if (newcham(mdef, (struct permonst *) 0, FALSE, FALSE)) {
            if (vis) { /* either seen or adjacent */
                boolean was_seen = !!strcmpi("It", Before),
                        verbosely = flags.verbose || !was_seen;

                if (canspotmon(mdef))
                    pline("%s%s%s turns into %s.", Before,
                          verbosely ? freaky : "", verbosely ? " and" : "",
                          x_monnam(mdef, ARTICLE_A, (char *) 0,
                                   (SUPPRESS_NAME | SUPPRESS_IT
                                    | SUPPRESS_INVISIBLE), FALSE));
                else if (was_seen || magr == &youmonst)
                    pline("%s%s%s.", Before, freaky,
                          !was_seen ? "" : " and disappears");
            }
            dmg = 0;
            if (can_teleport(magr->data)) {
                if (magr == &youmonst)
                    tele();
                else if (!tele_restrict(magr))
                    (void) rloc(magr, TRUE);
            }
        } else {
            if (vis && flags.verbose)
                pline1(nothing_happens);
        }
    }
    return dmg;
}

void
paralyze_monst(mon, amt)
struct monst *mon;
int amt;
{
    if (amt > 127)
        amt = 127;

    mon->mcanmove = 0;
    mon->mfrozen = amt;
    mon->meating = 0; /* terminate any meal-in-progress */
    mon->mstrategy &= ~STRAT_WAITFORU;
}

/* `mon' is hit by a sleep attack; return 1 if it's affected, 0 otherwise */
int
sleep_monst(mon, amt, how)
struct monst *mon;
int amt, how;
{
    if ((resists_sleep(mon) || defended(mon, AD_SLEE)
         || (how >= 0 && resist(mon, (char) how, 0, NOTELL)))
        && !(mon->data == &mons[PM_CERBERUS] && how == TOOL_CLASS)) {
        shieldeff(mon->mx, mon->my);
    } else if (mon->mcanmove) {
        finish_meating(mon); /* terminate any meal-in-progress */
        amt += (int) mon->mfrozen;
        if (amt > 0) { /* sleep for N turns */
            mon->mcanmove = 0;
            if (mon && mon->data == &mons[PM_CERBERUS])
                mon->mfrozen = min(amt, 8);
            else
                mon->mfrozen = min(amt, 127);
        } else { /* sleep until awakened */
            mon->msleeping = 1;
        }
        return 1;
    }
    return 0;
}

/* sleeping grabber releases, engulfer doesn't; don't use for paralysis! */
void
slept_monst(mon)
struct monst *mon;
{
    if ((mon->msleeping || !mon->mcanmove) && mon == u.ustuck
        && !sticks(youmonst.data) && !u.uswallow) {
        pline("%s grip relaxes.", s_suffix(Monnam(mon)));
        unstuck(mon);
    }
}

void
rustm(mdef, obj)
struct monst *mdef;
struct obj *obj;
{
    int dmgtyp = -1, chance = 1;

    if (!mdef || !obj)
        return; /* just in case */
    /* AD_ACID and AD_ENCH are handled in passivemm() and passiveum() */
    if (dmgtype(mdef->data, AD_CORR)
        || dmgtype(mdef->data, AD_DCAY)) {
        dmgtyp = ERODE_CORRODE;
    } else if (dmgtype(mdef->data, AD_RUST)) {
        dmgtyp = ERODE_RUST;
    } else if (dmgtype(mdef->data, AD_FIRE)
               /* steam vortex: fire resist applies, fire damage doesn't */
               && mdef->data != &mons[PM_STEAM_VORTEX]) {
        dmgtyp = ERODE_BURN;
        chance = 6;
    }

    if (dmgtyp >= 0 && !rn2(chance))
        (void) erode_obj(obj, (char *) 0, dmgtyp, EF_GREASE | EF_DESTROY);
}

STATIC_OVL void
mswingsm(magr, mdef, otemp)
struct monst *magr, *mdef;
struct obj *otemp;
{
    if (flags.verbose && !Blind && mon_visible(magr)) {
        pline("%s %s %s%s %s at %s.", Monnam(magr),
              (objects[otemp->otyp].oc_dir & PIERCE) ? "thrusts" : "swings",
              (otemp->quan > 1L) ? "one of " : "", mhis(magr), xname(otemp),
              mon_nam(mdef));
    }
}

/*
 * Passive responses by defenders.  Does not replicate responses already
 * handled above.  Returns same values as mattackm.
 */
STATIC_OVL int
passivemm(magr, mdef, mhit, mdead, mwep, aatyp)
register struct monst *magr, *mdef;
boolean mhit;
int mdead, aatyp;
struct obj *mwep;
{
    register struct permonst *mddat = mdef->data;
    register struct permonst *madat = magr->data;
    char buf[BUFSZ];
    int i, tmp;
    struct attack *mdattk;
    mdattk = has_erac(mdef) ? ERAC(mdef)->mattk : mddat->mattk;

    boolean mon_tempest_wield = (MON_WEP(mdef)
                                 && MON_WEP(mdef)->oartifact == ART_TEMPEST);
    struct obj *passive_armor;
    if ((passive_armor = which_armor(mdef, W_ARM))) {
        if (mhit && !rn2(3)
            && Is_dragon_scaled_armor(passive_armor)) {
            int otyp = Dragon_armor_to_scales(passive_armor);
            switch (otyp) {
            case GREEN_DRAGON_SCALES:
                if (resists_poison(magr) || defended(magr, AD_DRST))
                    break;
                if (rn2(20)) {
                    if (!rn2(3)) {
                        if (canseemon(magr))
                            pline("%s staggers from the poison!", Monnam(magr));
                        damage_mon(magr, rnd(4), AD_DRST);
                    }
                } else {
                    if (canseemon(magr))
                        pline("%s is fatally poisoned!", Monnam(magr));
                    magr->mhp = -1;
                    monkilled(magr, "", AD_DRST);
                    return (mdead | mhit | MM_AGR_DIED);
                }
                if (magr->mhp < 1) {
                    if (canseemon(magr))
                        pline("%s dies!", Monnam(magr));
                    monkilled(magr, "", AD_DRST);
                    return (mdead | mhit | MM_AGR_DIED);
                }
                break;
            case BLACK_DRAGON_SCALES:
                if (resists_disint(magr) || defended(magr, AD_DISN)) {
                    if (canseemon(magr) && !rn2(3)) {
                        shieldeff(magr->mx, magr->my);
                        Your("armor does not appear to affect %s.",
                            mon_nam(magr));
                    }
                    break;
                } else if (aatyp == AT_WEAP || aatyp == AT_CLAW
                        || aatyp == AT_TUCH || aatyp == AT_KICK
                        || aatyp == AT_BITE || aatyp == AT_HUGS
                        || aatyp == AT_BUTT || aatyp == AT_STNG
                        || aatyp == AT_TENT) {
                    /* if magr is wielding a weapon, that disintegrates first before
                    the actual monster. Same if magr is wearing gloves or boots */
                    if (MON_WEP(magr) && !rn2(12)) {
                        if (canseemon(magr))
                            pline("%s %s is disintegrated!",
                                s_suffix(Monnam(magr)), xname(MON_WEP(magr)));
                        m_useup(magr, MON_WEP(magr));
                    } else if ((magr->misc_worn_check & W_ARMF)
                            && aatyp == AT_KICK && !rn2(12)) {
                        if (canseemon(magr))
                            pline("%s %s are disintegrated!",
                                s_suffix(Monnam(magr)), xname(which_armor(magr, W_ARMF)));
                        m_useup(magr, which_armor(magr, W_ARMF));
                    } else if ((magr->misc_worn_check & W_ARMG)
                            && (aatyp == AT_WEAP || aatyp == AT_CLAW
                                || aatyp == AT_TUCH)
                            && !MON_WEP(magr) && !rn2(12)
                            && !((which_armor(magr, W_ARMG))->oartifact == ART_DRAGONBANE)) {
                        if (canseemon(magr))
                            pline("%s %s are disintegrated!",
                                s_suffix(Monnam(magr)), xname(which_armor(magr, W_ARMG)));
                        m_useup(magr, which_armor(magr, W_ARMG));
                    } else {
                        if (rn2(40)) {
                            if (canseemon(magr))
                                pline("%s partially disintegrates!", Monnam(magr));
                            magr->mhp -= rnd(4);
                        } else {
                            if (canseemon(magr))
                                pline("%s is disintegrated completely!", Monnam(magr));
                            disint_mon_invent(magr);
                            if (is_rider(magr->data)) {
                                if (canseemon(magr)) {
                                    pline("%s body reintegrates before your %s!",
                                        s_suffix(Monnam(magr)),
                                        (eyecount(youmonst.data) == 1)
                                            ? body_part(EYE)
                                            : makeplural(body_part(EYE)));
                                    pline("%s resurrects!", Monnam(magr));
                                }
                                magr->mhp = magr->mhpmax;
                            } else {
                                monkilled(magr, "", AD_DISN);
                                return (mdead | mhit | MM_AGR_DIED);
                            }
                        }
                    }
                }
                if (magr->mhp < 1) {
                    if (canseemon(magr))
                        pline("%s dies!", Monnam(magr));
                    monkilled(magr, "", AD_DISN);
                    return (mdead | mhit | MM_AGR_DIED);
                }
                break;
            case ORANGE_DRAGON_SCALES:
                if (!rn2(3)) {
                    mon_adjust_speed(magr, -1, (struct obj *) 0);
                }
                break;
            case WHITE_DRAGON_SCALES:
                if (resists_cold(magr) || defended(magr, AD_COLD))
                    break;
                if (rn2(20)) {
                    if (!rn2(3)) {
                        if (canseemon(magr))
                            pline("%s flinches from the cold!", Monnam(magr));
                        damage_mon(magr, rnd(4), AD_COLD);
                    }
                } else {
                    if (canseemon(magr))
                        pline("%s is frozen solid!", Monnam(magr));
                    damage_mon(magr, d(6, 6), AD_COLD);
                }
                if (magr->mhp < 1) {
                    if (canseemon(magr))
                        pline("%s dies!", Monnam(magr));
                    monkilled(magr, "", AD_COLD);
                    return (mdead | mhit | MM_AGR_DIED);
                }
                break;
            case RED_DRAGON_SCALES:
                if (resists_fire(magr) || defended(magr, AD_FIRE)
                    || mon_underwater(magr))
                    break;
                if (rn2(20)) {
                    if (!rn2(3)) {
                        if (canseemon(magr))
                            pline("%s is burned!", Monnam(magr));
                        damage_mon(magr, rnd(4), AD_FIRE);
                    }
                } else {
                    if (canseemon(magr))
                        pline("%s is severely burned!", Monnam(magr));
                    damage_mon(magr, d(6, 6), AD_FIRE);
                }
                if (magr->mhp < 1) {
                    if (canseemon(magr))
                        pline("%s dies!", Monnam(magr));
                    monkilled(magr, "", AD_FIRE);
                    return (mdead | mhit | MM_AGR_DIED);
                }
                break;
            case YELLOW_DRAGON_SCALES:
                if (resists_acid(magr) || defended(magr, AD_ACID)
                    || mon_underwater(magr))
                    break;
                if (rn2(20)) {
                    if (!rn2(3)) {
                        if (canseemon(magr))
                            pline("%s is seared!", Monnam(magr));
                        damage_mon(magr, rnd(4), AD_ACID);
                    }
                } else {
                    if (canseemon(magr))
                        pline("%s is critically seared!", Monnam(magr));
                    damage_mon(magr, d(6, 6), AD_ACID);
                }
                if (magr->mhp < 1) {
                    if (canseemon(magr))
                        pline("%s dies!", Monnam(magr));
                    monkilled(magr, "", AD_ACID);
                    return (mdead | mhit | MM_AGR_DIED);
                }
                break;
            case GRAY_DRAGON_SCALES:
                if (!rn2(6))
                    (void) cancel_monst(magr, (struct obj *) 0, TRUE, TRUE, FALSE);
                break;
            default: /* all other types of armor, just pass on through */
                break;
            }
        }

    }
    if ((passive_armor = which_armor(mdef, W_ARMG))) {
        switch (passive_armor->otyp) {
        case GLOVES:
            if (!is_dragon(magr->data))
                break;
            if (!rn2(3) && is_dragon(magr->data)
                && passive_armor->oartifact == ART_DRAGONBANE) {
                if (canseemon(magr))
                    pline("Dragonbane sears %s scaly hide!", s_suffix(mon_nam(magr)));
                magr->mhp -= rnd(6) + 2;
            }
            if (magr->mhp < 1) {
                if (canseemon(magr))
                    pline("Dragonbane's power overwhelms %s!", mon_nam(magr));
                pline("%s dies!", Monnam(magr));
                monkilled(magr, "", AD_PHYS);
                return (mdead | mhit | MM_AGR_DIED);
            }
            break;
        default: /* all other types of armor, just pass on through */
            break;
        }
    }

    for (i = 0;; i++) {
        if (i >= NATTK)
            return (mdead | mhit); /* no passive attacks */
        if (mdattk[i].aatyp == AT_NONE)
            break;
    }
    if (mdattk[i].damn)
        tmp = d((int) mdattk[i].damn, (int) mdattk[i].damd);
    else if (mdattk[i].damd)
        tmp = d((int) mddat->mlevel + 1, (int) mdattk[i].damd);
    else
        tmp = 0;

    /* These affect the enemy even if defender killed */
    switch (mdattk[i].adtyp) {
    case AD_ACID:
        if (mhit && !rn2(2)) {
            Strcpy(buf, Monnam(magr));
            if (canseemon(magr)) {
                if (mdef->data == &mons[PM_YELLOW_DRAGON]) {
                    pline("%s is seared by %s acidic hide!", buf,
                          s_suffix(mon_nam(mdef)));
                } else {
                    pline("%s is splashed by %s %s!", buf,
                          s_suffix(mon_nam(mdef)), hliquid("acid"));
                }
            }
            if (resists_acid(magr) || defended(magr, AD_ACID)
                || mon_underwater(magr)) {
                if (canseemon(magr))
                    pline("%s is not affected.", Monnam(magr));
                tmp = 0;
            }
        } else
            tmp = 0;
        if (!mon_underwater(magr)) {
            if (!rn2(30))
                erode_armor(magr, ERODE_CORRODE);
            if (!rn2(6))
                acid_damage(MON_WEP(magr));
        }
        goto assess_dmg;
    case AD_DISN: {
        int chance = (mdef->data == &mons[PM_ANTIMATTER_VORTEX] ? !rn2(3) : !rn2(6));
        if (mhit && !mdef->mcan) {
            if (resists_disint(magr) || defended(magr, AD_DISN)) {
                if (canseemon(magr) && !rn2(3)) {
                    shieldeff(magr->mx, magr->my);
                    pline("%s deadly %s does not appear to affect %s.",
                          s_suffix(Monnam(mdef)),
                          mdef->data == &mons[PM_ANTIMATTER_VORTEX]
                              ? "form" : "hide", mon_nam(magr));
                }
            } else if (aatyp == AT_WEAP || aatyp == AT_CLAW
                       || aatyp == AT_TUCH || aatyp == AT_KICK
                       || aatyp == AT_BITE || aatyp == AT_HUGS
                       || aatyp == AT_BUTT || aatyp == AT_STNG
                       || aatyp == AT_TENT) {
                /* if magr is wielding a weapon, that disintegrates first before
                   the actual monster. Same if magr is wearing gloves or boots */
                if (MON_WEP(magr) && chance) {
                    if (canseemon(magr))
                        pline("%s %s is disintegrated!",
                              s_suffix(Monnam(magr)), xname(MON_WEP(magr)));
                    m_useup(magr, MON_WEP(magr));
                } else if ((magr->misc_worn_check & W_ARMF)
                           && aatyp == AT_KICK && chance) {
                    if (canseemon(magr))
                        pline("%s %s are disintegrated!",
                              s_suffix(Monnam(magr)), xname(which_armor(magr, W_ARMF)));
                    m_useup(magr, which_armor(magr, W_ARMF));
                } else if ((magr->misc_worn_check & W_ARMG)
                           && (aatyp == AT_WEAP || aatyp == AT_CLAW
                               || aatyp == AT_TUCH)
                           && !MON_WEP(magr) && chance) {
                    if (canseemon(magr))
                        pline("%s %s are disintegrated!",
                              s_suffix(Monnam(magr)), xname(which_armor(magr, W_ARMG)));
                    m_useup(magr, which_armor(magr, W_ARMG));
                } else {
                    if (mdef->data == &mons[PM_ANTIMATTER_VORTEX] ? rn2(10) : rn2(20)) {
                        if (canseemon(magr))
                            pline("%s hide partially disintegrates %s!",
                                  s_suffix(Monnam(mdef)), mon_nam(magr));
                        tmp = rn2(6) + 1;
                        goto assess_dmg;
                    } else {
                        if (canseemon(magr))
                            pline("%s deadly %s disintegrates %s!",
                                  s_suffix(Monnam(mdef)),
                                  mdef->data == &mons[PM_ANTIMATTER_VORTEX]
                                      ? "form" : "hide", mon_nam(magr));
                        disint_mon_invent(magr);
                        if (is_rider(magr->data)) {
                            if (canseemon(magr)) {
                                pline("%s body reintegrates!",
                                      s_suffix(Monnam(magr)));
                                pline("%s resurrects!", Monnam(magr));
                            }
                            magr->mhp = magr->mhpmax;
                        } else {
                            monkilled(magr, "", AD_DISN);
                            return (mdead | mhit | MM_AGR_DIED);
                        }
                    }
                }
            }
        }
        break;
    }
    case AD_DRST:
        if (mhit && !mdef->mcan && !rn2(3)) {
            if (resists_poison(magr) || defended(magr, AD_DRST)) {
                if (canseemon(magr)) {
                    if (!rn2(5))
                        pline("%s poisonous hide doesn't seem to affect %s.",
                              s_suffix(Monnam(mdef)), mon_nam(magr));
                }
            } else {
                if (rn2(20)) {
                    if (canseemon(magr))
                        pline("%s is poisoned!", Monnam(magr));
                    tmp = rn2(4) + 1;
                    goto assess_dmg;
                } else {
                    if (canseemon(magr))
                        pline("%s poisonous hide was deadly...",
                              s_suffix(Monnam(mdef)));
                    monkilled(magr, "", (int) mdattk[i].adtyp);
                    return (mdead | mhit | MM_AGR_DIED);
                }
            }
        }
        break;
    /* Grudge patch. */
    case AD_MAGM:
      /* wrath of gods for attacking Oracle */
        if (resists_magm(magr) || defended(magr, AD_MAGM)) {
            tmp = (tmp + 1) / 2;
            if (canseemon(magr)) {
                shieldeff(magr->mx, magr->my);
                pline(magr->data == &mons[PM_WOODCHUCK] ? "ZOT!" :
                      "%s is hit by magic missiles appearing from thin air!",
                      Monnam(magr));
                pline("Some missiles bounce off!");
            }
        } else {
            if (canseemon(magr))
                pline(magr->data == &mons[PM_WOODCHUCK] ? "ZOT!" :
                      "%s is hit by magic missiles appearing from thin air!",
                      Monnam(magr));
            goto assess_dmg;
        }
        break;
    case AD_ENCH: /* KMH -- remove enchantment (disenchanter) */
        if (mhit && !mdef->mcan && mwep) {
            (void) drain_item(mwep, FALSE);
            /* No message */
        }
        break;
    case AD_CNCL:
        if (mhit && !rn2(6)) {
            if (canseemon(magr))
                pline("%s hide absorbs magical energy from %s.",
                      s_suffix(Monnam(mdef)), mon_nam(magr));
            (void) cancel_monst(magr, mwep, FALSE, TRUE, FALSE);
        }
        break;
    case AD_SLIM:
        if (mhit && !mdef->mcan && !rn2(3)) {
            pline("Its slime splashes onto %s!", mon_nam(magr));
            if (flaming(magr->data)) {
                pline_The("slime burns away!");
                tmp = 0;
            } else if (slimeproof(magr->data)) {
                pline("%s is unaffected.", Monnam(magr));
                tmp = 0;
            } else if (!rn2(4) && !slimeproof(magr->data)) {
                if (!munslime(magr, FALSE) && !DEADMONSTER(magr)) {
                    if (newcham(magr, &mons[PM_GREEN_SLIME], FALSE,
                                (boolean) (vis && canseemon(magr))))
                    magr->mstrategy &= ~STRAT_WAITFORU;
                }
                /* munslime attempt could have been fatal,
                   potentially to multiple monsters (SCR_FIRE) */
                if (DEADMONSTER(magr))
                    return (mdead | mhit | MM_AGR_DIED);
                if (DEADMONSTER(mdef))
                    return (mdead | mhit | MM_DEF_DIED);
                tmp = 0;
            }
        }
        break;
    default:
        break;
    }
    if (mdead || mdef->mcan)
        return (mdead | mhit);

    /* These affect the enemy only if defender is still alive */
    if (rn2(3))
        switch (mdattk[i].adtyp) {
        case AD_PLYS: /* Floating eye */
            if (tmp > 127)
                tmp = 127;
            if (mddat == &mons[PM_FLOATING_EYE]) {
                if (!rn2(4))
                    tmp = 127;
                if (magr->mcansee && haseyes(madat) && mdef->mcansee
                    && (racial_perceives(magr) || !mdef->minvis)) {
                    /* construct format string; guard against '%' in Monnam */
                    Strcpy(buf, s_suffix(Monnam(mdef)));
                    (void) strNsubst(buf, "%", "%%", 0);
                    Strcat(buf, " gaze is reflected by %s %s.");
                    if (mon_reflects(magr,
                                     canseemon(magr) ? buf : (char *) 0))
                        return (mdead | mhit);
                    Strcpy(buf, Monnam(magr));
                    if (canseemon(magr)) {
                        if (has_free_action(magr)) {
                            pline("%s stiffens momentarily.", Monnam(magr));
                        } else {
                            pline("%s is frozen by %s gaze!", buf,
                                  s_suffix(mon_nam(mdef)));
                        }
                    }
                    if (has_free_action(magr))
                        return 1;
                    paralyze_monst(magr, tmp);
                    return (mdead | mhit);
                }
            } else { /* gelatinous cube */
                Strcpy(buf, Monnam(magr));
                if (canseemon(magr)) {
                    if (has_free_action(magr)) {
                        pline("%s stiffens momentarily.", Monnam(magr));
                    } else {
                        pline("%s is frozen by %s.", buf, mon_nam(mdef));
                    }
                }
                if (has_free_action(magr))
                    return 1;
                paralyze_monst(magr, tmp);
                return (mdead | mhit);
            }
            return 1;
        case AD_COLD:
            if (resists_cold(magr) || defended(magr, AD_COLD)) {
                if (canseemon(magr)) {
                    pline("%s is mildly chilly.", Monnam(magr));
                    golemeffects(magr, AD_COLD, tmp);
                }
                tmp = 0;
                break;
            }
            if (canseemon(magr))
                pline("%s is suddenly very cold!", Monnam(magr));
            if (mddat == &mons[PM_BLUE_JELLY]
                || mddat == &mons[PM_BROWN_MOLD]) {
                mdef->mhp += tmp / 2;
                if (mdef->mhpmax < mdef->mhp)
                    mdef->mhpmax = mdef->mhp;
                if (mdef->mhpmax > ((int) (mdef->m_lev + 1) * 8))
                    (void) split_mon(mdef, magr);
            }
            break;
        case AD_STUN:
            if (resists_stun(magr->data)
                || defended(magr, AD_STUN) || mon_tempest_wield) {
                ; /* immune */
                break;
            }
            if (!magr->mstun) {
                magr->mstun = 1;
                if (canseemon(magr))
                    pline("%s %s...", Monnam(magr),
                          makeplural(stagger(magr->data, "stagger")));
            }
            tmp = 0;
            break;
        case AD_FIRE:
            if (resists_fire(magr) || defended(magr, AD_FIRE)
                || mon_underwater(magr)) {
                if (canseemon(magr)) {
                    pline("%s is mildly warmed.", Monnam(magr));
                    golemeffects(magr, AD_FIRE, tmp);
                }
                tmp = 0;
                break;
            }
            if (canseemon(magr))
                pline("%s is suddenly very hot!", Monnam(magr));
            break;
	case AD_DISE:
	    if (resists_sick(magr) || defended(magr, AD_DISE)) {
                if (canseemon(magr))
                    pline("%s resists infection.", Monnam(magr));
                tmp = 0;
                break;
	    } else {
                if (magr->mdiseasetime)
                    magr->mdiseasetime -= rnd(3);
                else
                    magr->mdiseasetime = rn1(9, 6);
                if (canseemon(magr))
                    pline("%s looks %s.", Monnam(magr),
                          magr->mdiseased ? "even worse" : "diseased");
                magr->mdiseased = 1;
                magr->mdiseabyu = FALSE;
            }
	    break;
        case AD_ELEC:
            if (resists_elec(magr) || defended(magr, AD_ELEC)) {
                if (canseemon(magr)) {
                    pline("%s is mildly tingled.", Monnam(magr));
                    golemeffects(magr, AD_ELEC, tmp);
                }
                tmp = 0;
                break;
            }
            if (canseemon(magr))
                pline("%s is jolted with electricity!", Monnam(magr));
            break;
        case AD_SLOW:
            if (mhit && !mdef->mcan && magr->mspeed != MSLOW && !rn2(3)) {
                mon_adjust_speed(magr, -1, (struct obj *) 0);
            }
            tmp = 0;
            break;
        case AD_SLEE:
            if (sleep_monst(magr, rn2(3) + 8, -1)) {
                if (canseemon(magr))
                    pline("%s loses consciousness.", Monnam(magr));
                slept_monst(magr);
            }
            break;
        default:
            tmp = 0;
            break;
        }
    else
        tmp = 0;

 assess_dmg:
    if (damage_mon(magr, tmp, (int) mdattk[i].adtyp)) {
        zombify = FALSE;
        if (mdef->uexp)
            mon_xkilled(magr, "", (int) mdattk[i].adtyp);
        else
            monkilled(magr, "", (int) mdattk[i].adtyp);
        return (mdead | mhit | MM_AGR_DIED);
    }
    return (mdead | mhit);
}

/* hero or monster has successfully hit target mon with drain energy attack */
void
xdrainenergym(mon, givemsg)
struct monst *mon;
boolean givemsg;
{
    if (mon->mspec_used < 20 /* limit draining */
        && (attacktype(mon->data, AT_MAGC)
            || attacktype(mon->data, AT_BREA))) {
        mon->mspec_used += d(2, 2);
        if (givemsg)
            pline("%s seems lethargic.", Monnam(mon));
    }
}

/* "aggressive defense"; what type of armor prevents specified attack
   from touching its target? */
long
attk_protection(aatyp)
int aatyp;
{
    long w_mask = 0L;

    switch (aatyp) {
    case AT_NONE:
    case AT_SPIT:
    case AT_EXPL:
    case AT_BOOM:
    case AT_GAZE:
    case AT_BREA:
    case AT_MAGC:
        w_mask = ~0L; /* special case; no defense needed */
        break;
    case AT_CLAW:
    case AT_TUCH:
    case AT_WEAP:
        w_mask = W_ARMG; /* caller needs to check for weapon */
        break;
    case AT_KICK:
        w_mask = W_ARMF;
        break;
    case AT_BUTT:
        w_mask = W_ARMH;
        break;
    case AT_HUGS:
        w_mask = (W_ARMC | W_ARMG); /* attacker needs both to be protected */
        break;
    case AT_BITE:
    case AT_STNG:
    case AT_ENGL:
    case AT_TENT:
    default:
        w_mask = 0L; /* no defense available */
        break;
    }
    return w_mask;
}

/*mhitm.c*/
