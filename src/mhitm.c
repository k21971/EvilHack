/* NetHack 5.0	mhitm.c	$NHDT-Date: 1781973054 2026/06/20 16:30:54 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.269 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2011. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "artifact.h"

static const char brief_feeling[] =
    "have a %s feeling for a moment, then it passes.";

/* verb stems for a monster's weapon attack; hitmm() appends the 's' */
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

staticfn void noises(struct monst *, struct attack *);
staticfn void pre_mm_attack(struct monst *, struct monst *);
staticfn void missmm(struct monst *, struct monst *, int, int,
                     struct attack *);
staticfn int hitmm(struct monst *, struct monst *, struct attack *,
                 struct obj *, int);
staticfn int gazemm(struct monst *, struct monst *, struct attack *);
staticfn int screamm(struct monst *, struct monst *, struct attack *);
staticfn int gulpmm(struct monst *, struct monst *, struct attack *);
staticfn int explmm(struct monst *, struct monst *, struct attack *);
staticfn int mdamagem(struct monst *, struct monst *, struct attack *,
                    struct obj *, int);
staticfn void mswingsm(struct monst *, struct monst *, struct obj *);
staticfn int passive_agr_killed(struct monst *, struct monst *, int, int,
                               int);
staticfn int passive_agr_dmg(struct monst *, struct monst *, int, int, int,
                            int);
staticfn int passive_disint_agr(struct monst *, struct monst *, int, int);
staticfn int passivemm(struct monst *, struct monst *, boolean, int,
                     struct obj *, int);
staticfn boolean mdef_at_bhitpos(struct monst *);

/* the Oracle's passive retaliation */
static const char thin_air_missiles[] =
    "%s is hit by magic missiles appearing from thin air!";

staticfn void
noises(struct monst *magr, struct attack *mattk)
{
    boolean farq = (mdistu(magr) > 15);

    if (!Deaf && (farq != gf.far_noise || svm.moves - gn.noisetime > 10)) {
        gf.far_noise = farq;
        gn.noisetime = svm.moves;
        You_hear("%s%s.",
                 (mattk->aatyp == AT_EXPL) ? "an explosion" : "some noises",
                 farq ? " in the distance" : "");
    }
}

staticfn void
pre_mm_attack(struct monst *magr, struct monst *mdef)
{
    boolean showit = FALSE;

    /* unhiding or unmimicking happens even if hero can't see it
       because the formerly concealed monster is now in action */
    if (M_AP_TYPE(mdef)) {
        seemimic(mdef);
        showit |= gv.vis;
    } else if (mdef->mundetected) {
        mdef->mundetected = 0;
        showit |= gv.vis;
    }
    if (M_AP_TYPE(magr)) {
        seemimic(magr);
        showit |= gv.vis;
    } else if (magr->mundetected) {
        magr->mundetected = 0;
        showit |= gv.vis;
    }

    if (gv.vis) {
        if (!canspotmon(magr))
            map_invisible(magr->mx, magr->my);
        else if (showit)
            newsym(magr->mx, magr->my);
        if (!canspotmon(mdef))
            map_invisible(mdef->mx, mdef->my);
        else if (showit)
            newsym(mdef->mx, mdef->my);
    }
}

/* feedback for when a monster-vs-monster attack misses */
staticfn void
missmm(
    struct monst *magr, /* attacker */
    struct monst *mdef, /* defender */
    int target,         /* attacker's to-hit total */
    int roll,           /* what it rolled against the defender */
    struct attack *mattk) /* attack and damage types */
{
    struct obj *blocker;
    boolean nearmiss = (target == roll);
    /* s_suffix() hands back one shared static buffer, so the two
       possessives in these messages cannot both be live arguments to
       the same pline(); master stages the first half the same way */
    char buf[BUFSZ];

    blocker = armor_blocker(mdef, &target, roll);

    pre_mm_attack(magr, mdef);

    if (gv.vis) {
        if (flags.verbose && !nearmiss && blocker) {
            Snprintf(buf, sizeof buf, "%s %s %s", s_suffix(Monnam(mdef)),
                     aobjnam(blocker, (char *) 0),
                     rn2(2) ? "blocks" : "deflects");
            pline_mon(mdef, "%s %s attack.", buf,
                      s_suffix(mon_nam_too(magr, mdef)));
        } else if ((thick_skinned(mdef->data)
                    || has_barkskin(mdef) || has_stoneskin(mdef))
                   && !rn2(10)) {
            Snprintf(buf, sizeof buf, "%s %s %s", s_suffix(Monnam(mdef)),
                     is_dragon(mdef->data)
                       ? "scaly hide"
                       : (mdef->data == &mons[PM_GIANT_TURTLE]
                          || is_tortle(mdef->data))
                         ? "protective shell"
                         : is_bone_monster(mdef->data)
                           ? "bony structure"
                           : (has_bark(mdef->data) || has_barkskin(mdef))
                             ? "rough bark"
                             : has_stoneskin(mdef) ? "stony hide"
                                                   : "thick hide",
                     rn2(2) ? "blocks" : "deflects");
            pline_mon(mdef, "%s %s attack.", buf,
                      s_suffix(mon_nam_too(magr, mdef)));
        } else {
            boolean plainmiss = (magr->mcan
                                 || !could_seduce(magr, mdef, mattk));

            pline("%s %s%s %s.", Monnam(magr),
                  plainmiss ? (nearmiss ? "just " : "") : "",
                  plainmiss ? "misses" : "pretends to be friendly to",
                  mon_nam_too(mdef, magr));
        }
    } else {
        noises(magr, mattk);
    }

    if (!blocker)
        return;

    /* the attacker grazes a material it hates on the way past */
    if (!MON_WEP(magr) && !which_armor(magr, W_ARMG)
        && mon_hates_material(magr, blocker->material)
        && !(has_barkskin(magr) || has_stoneskin(magr))) {
        searmsg(mdef, magr, blocker, FALSE);
        /* glancing blow. master notes here that damage_mon() applies
           the SPFX_HPHDAM halving centrally for AD_PHYS; on this branch
           damage_mon is still the stub, so that arrives with it */
        if (damage_mon(magr, rnd(sear_damage(blocker->material) / 2),
                       AD_PHYS, FALSE)) {
            if (mdef->mtame)
                set_pet_killer(mdef);
            monkilled(magr, "", AD_PHYS);
        }
    }
    /* glass armor, or certain drow armor in the presence of light, can
       break when it deflects an attack */
    if (is_glass(blocker) || is_adamantine(blocker)
        || blocker->forged_qual == FQ_INFERIOR)
        (void) break_glass_obj(blocker);
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
fightm(struct monst *mtmp)
{
    struct monst *mon, *nmon;
    int result, has_u_swallowed;
    /* perhaps the monster will resist Conflict */
    if (resist_conflict(mtmp))
        return 0;

    if (u.ustuck == mtmp) {
        /* perhaps we're holding it... */
        if (itsstuck(mtmp))
            return 0;
    }
    has_u_swallowed = engulfing_u(mtmp);

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
                if (!u.uswallow && (mtmp == u.ustuck)) {
                    if (!rn2(4)) {
                        set_ustuck((struct monst *) 0);
                        pline("%s releases you!", Monnam(mtmp));
                    } else
                        break;
                }

                /* mtmp can be killed */
                gb.bhitpos.x = mon->mx, gb.bhitpos.y = mon->my;
                gn.notonhead = FALSE;
                result = mattackm(mtmp, mon);

                if (result & M_ATTK_AGR_DIED)
                    return 1; /* mtmp died */
                /*
                 * If mtmp has the hero swallowed, lie and say there
                 * was no attack (this allows mtmp to digest the hero).
                 */
                if (has_u_swallowed)
                    return 0;

                /* allow attacked monsters a chance to hit back, primarily
                   to allow monsters that resist conflict to respond */
                if ((result & (M_ATTK_HIT | M_ATTK_DEF_DIED)) == M_ATTK_HIT
                    && rn2(4) && mon->movement > rn2(NORMAL_SPEED)) {
                    if (mon->movement > NORMAL_SPEED)
                        mon->movement -= NORMAL_SPEED;
                    else
                        mon->movement = 0;
                    gb.bhitpos.x = mtmp->mx, gb.bhitpos.y = mtmp->my;
                    gn.notonhead = FALSE;
                    (void) mattackm(mon, mtmp); /* return attack */
                }

                return (result & M_ATTK_HIT) ? 1 : 0;
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
mdisplacem(
    struct monst *magr,
    struct monst *mdef,
    boolean quietly)
{
    struct permonst *pa, *pd;
    int tx, ty, fx, fy;

    /* sanity checks; could matter if we unexpectedly get a long worm */
    if (!magr || !mdef || magr == mdef || has_erid(mdef))
        return M_ATTK_MISS;
    pa = magr->data, pd = mdef->data;
    tx = mdef->mx, ty = mdef->my; /* destination */
    fx = magr->mx, fy = magr->my; /* current location */
    if (m_at(fx, fy) != magr || m_at(tx, ty) != mdef)
        return M_ATTK_MISS;

    /* The 1 in 7 failure below matches the chance in do_attack()
     * for pet displacement.
     */
    if (!rn2(7))
        return M_ATTK_MISS;

    /* Grid bugs cannot displace at an angle. */
    if (pa == &mons[PM_GRID_BUG] && magr->mx != mdef->mx
        && magr->my != mdef->my)
        return M_ATTK_MISS;

    /* undetected monster becomes un-hidden if it is displaced */
    if (mdef->mundetected)
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
    gv.vis = (canspotmon(magr) && canspotmon(mdef));

    if (touch_petrifies(pd)
        && !(resists_ston(magr) || defended(magr, AD_STON))) {
        if (!which_armor(magr, W_ARMG)) {
            if (poly_when_stoned(pa)) {
                mon_to_stone(magr);
                return M_ATTK_HIT; /* no damage during the polymorph */
            }
            if (!quietly && canspotmon(magr)) {
                if (gv.vis) {
                    pline("%s tries to move %s out of %s way.", Monnam(magr),
                          mon_nam(mdef), is_rider(pa) ? "the" : mhis(magr));
                }
                pline_mon(magr, "%s turns to stone!", Monnam(magr));
            }
            if (mdef->mtame)
                set_pet_killer(mdef);
            monstone(magr);
            if (!DEADMONSTER(magr))
                return M_ATTK_HIT; /* lifesaved */
            else if (magr->mtame && !gv.vis)
                You(brief_feeling, "peculiarly sad");
            return M_ATTK_AGR_DIED;
        }
    }

    remove_monster(fx, fy); /* pick up from orig position */
    if (mdef->wormno)
        remove_worm(mdef);
    else
        remove_monster(tx, ty);
    place_monster(magr, tx, ty); /* put down at target spot */
    place_monster(mdef, fx, fy);
    /* a rider drags its steed along; mdef cannot be mounted - the
       has_erid(mdef) check above rejects that case */
    update_monsteed(magr);
    if (mdef->wormno) /* now put down tail */
        place_worm_tail_randomly(mdef, fx, fy);
    /* either creature might move into or out of a poison gas cloud */
    update_monster_region(magr);
    update_monster_region(mdef);

    if (gv.vis && !quietly)
        pline("%s moves %s out of %s way!", Monnam(magr), mon_nam(mdef),
              is_rider(pa) ? "the" : mhis(magr));
    newsym(fx, fy);  /* see it       */
    newsym(tx, ty);  /*   all happen */
    flush_screen(0); /* make sure it shows up */

    return M_ATTK_HIT;
}

/* is mdef still at the square the attack was aimed at? a ridden steed is
   off the map (m_at() yields the rider; the hero's steed square is empty)
   but its coordinates are kept in step with its rider */
staticfn boolean
mdef_at_bhitpos(struct monst *mdef)
{
    if (m_at(gb.bhitpos.x, gb.bhitpos.y) == mdef)
        return TRUE;
    return ((mdef == u.usteed || mdef->ridden_by)
            && mdef->mx == gb.bhitpos.x && mdef->my == gb.bhitpos.y);
}

/*
 * mattackm() -- a monster attacks another monster.
 *
 *          --------- aggressor died
 *         /  ------- defender died
 *        /  /  ----- defender was hit
 *       /  /  /
 *      x  x  x
 *
 *      0x8     M_ATTK_AGR_DONE
 *      0x4     M_ATTK_AGR_DIED
 *      0x2     M_ATTK_DEF_DIED
 *      0x1     M_ATTK_HIT
 *      0x0     M_ATTK_MISS
 *
 * Each successive attack has a lower probability of hitting.  Some rely on
 * success of previous attacks.  ** this doesn't seem to be implemented -dl **
 *
 * Attacker has targeted <bhitpos.x,bhitpos.y> rather than
 * <mdef->mx,mdef->my>; matters for long worms.
 *
 * In the case of exploding monsters, the monster dies as well.
 */
int
mattackm(
    struct monst *magr,
    struct monst *mdef)
{
    int i,          /* loop counter */
        tmp,        /* armor class difference */
        strike = 0, /* hit this attack */
        attk,       /* attack attempted this time */
        struck = 0, /* hit at least once */
        res[NATTK], /* results of all attacks */
        weap_attk_idx = 0, /* which AT_WEAP attack (0=primary, 1+=off-hand) */
        dieroll = 0,
        saved_mhp = (mdef ? mdef->mhp : 0); /* for print_mon_wounded() */
    struct attack *mattk, alt_attk;
    struct obj *mwep;
    struct permonst *pa, *pd;
    unsigned wep_id;
    coord save_bhitpos; /* m_throw() walks bhitpos along the flight path */
    boolean save_notonhead;

    if (!magr || !mdef)
        return M_ATTK_MISS; /* mike@genat */
    if (helpless(magr))
        return M_ATTK_MISS;
    pa = magr->data;
    pd = mdef->data;

    /* Grid bugs cannot attack at an angle. */
    if (pa == &mons[PM_GRID_BUG] && magr->mx != mdef->mx
        && magr->my != mdef->my)
        return M_ATTK_MISS;

    /* Calculate the armour class differential. */
    tmp = find_mac(mdef) + magr->m_lev;
    if (mdef->mconf || helpless(mdef)) {
        tmp += 4;
        mdef->msleeping = 0;
    }

    /* elven types hate orcs */
    if ((racial_elf(magr) || racial_drow(magr)) && racial_orc(mdef))
        tmp++;

    /* M3_ACCURATE monsters get a to-hit bonus */
    if ((has_erac(magr) && (ERAC(magr)->mflags3 & M3_ACCURATE))
        || is_accurate(magr->data))
        tmp += 5;

    /* drow are affected by being in both the light or the dark */
    if (racial_drow(magr)) {
        if (spot_is_dark(magr->mx, magr->my))
            tmp += (magr->m_lev / 3) + 2;
        else
            tmp -= 3;
    }

    /* mundetected monsters become un-hidden if they are attacked */
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
            } else {
                if (iflags.last_msg == PLNMSG_HIDE_UNDER
                    && mdef->m_id == gl.last_hider)
                    pline_mon(mdef, "%s emerges from hiding.", Monnam(mdef));
                else if (mdef->m_id == gl.last_hider)
                    You("notice %s.", mon_nam(mdef));
                else
                    pline("Suddenly, you notice %s.", a_monnam(mdef));
            }
        }
    }

    /* Elves hate orcs. */
    if (is_elf(pa) && is_orc(pd))
        tmp++;

    /* Set up the visibility of action */
    gv.vis = ((cansee(magr->mx, magr->my) && canspotmon(magr))
              || (cansee(mdef->mx, mdef->my) && canspotmon(mdef)));

    /* Set flag indicating monster has moved this turn.  Necessary since a
     * monster might get an attack out of sequence (i.e. before its move) in
     * some cases, in which case this still counts as its move for the round
     * and it shouldn't move again.
     */
    magr->mlstmv = svm.moves;

    /* controls whether a mind flayer uses all of its tentacle-for-DRIN
       attacks; when fighting a headless monster, stop after the first
       one because repeating the same failing hit (or even an ordinary
       tentacle miss) is very verbose and makes the flayer look stupid */
    gs.skipdrin = FALSE;

    /* Now perform all attacks for the monster. */
    for (i = 0; i < NATTK; i++) {
        res[i] = M_ATTK_MISS;

        /* target might no longer be there */
        if (i > 0 && (!mdef_at_bhitpos(mdef)
                      || DEADMONSTER(magr) || DEADMONSTER(mdef)))
            continue;

        mattk = getmattk(magr, mdef, i, res, &alt_attk);
        /* reduce verbosity for mind flayer attacking creature without a
           head (or worm's tail); this is similar to monster with multiple
           attacks after a wildmiss against displaced or invisible hero */
        if (gs.skipdrin && mattk->aatyp == AT_TENT && mattk->adtyp == AD_DRIN)
            continue;
        mwep = (struct obj *) 0;
        attk = 1;

        switch (mattk->aatyp) {
        case AT_WEAP: /* "hand to hand" attacks */
            if (distmin(magr->mx, magr->my, mdef->mx, mdef->my) > 1) {
                /* D: Do a ranged attack here! The missile walk leaves
                   bhitpos wherever the missile stopped; the between-attacks
                   re-check needs the target's square back */
                save_bhitpos = gb.bhitpos;
                save_notonhead = gn.notonhead;
                strike = (thrwmm(magr, mdef) == M_ATTK_MISS) ? 0 : 1;
                gb.bhitpos = save_bhitpos;
                gn.notonhead = save_notonhead;
                if (strike)
                    /* don't really know if we hit or not; pretend we did */
                    res[i] |= M_ATTK_HIT;
                if (DEADMONSTER(mdef))
                    res[i] = M_ATTK_DEF_DIED;
                if (DEADMONSTER(magr))
                    res[i] |= M_ATTK_AGR_DIED;
                weap_attk_idx++;
                break;
            }
            if (weap_attk_idx == 0) {
                /* first AT_WEAP: use primary weapon */
                if (magr->weapon_check == NEED_WEAPON || !MON_WEP(magr)) {
                    magr->weapon_check = NEED_HTH_WEAPON;
                    if (mon_wield_item(magr) != 0)
                        return M_ATTK_MISS;
                }
                possibly_unwield(magr, FALSE);
                mwep = MON_WEP(magr);
            } else {
                /* 2nd+ AT_WEAP: use off-hand weapon with penalty */
                mwep = MON_WEP2(magr);
                if (mwep)
                    tmp -= OFFHAND_PENALTY;
                else
                    mwep = MON_WEP(magr); /* fallback to primary */
            }
            weap_attk_idx++;
            if (mwep) {
                if (gv.vis)
                    mswingsm(magr, mdef, mwep);
                tmp += hitval(magr, mwep, mdef);
            }
            FALLTHROUGH;
            /*FALLTHRU*/
        case AT_CLAW:
        case AT_KICK:
        case AT_BITE:
        case AT_STNG:
        case AT_TUCH:
        case AT_BUTT:
        case AT_TENT:
            if (mattk->aatyp == AT_KICK && mtrapped_in_pit(magr))
                    continue;
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
                tmp -= hitval(magr, mwep, mdef);
            if ((is_displaced(mdef->data) || has_displacement(mdef))
                && rn2(4)) {
                if (gv.vis && canspotmon(mdef))
                    pline_mon(mdef, "%s attacks the displaced image of %s.",
                              Monnam(magr), mon_nam(mdef));
                strike = 0;
            }
            /* tiny or small animals have a chance to dodge a zombie's
               bite attack due to zombies being mindless and slow */
            if (strike && racial_zombie(magr) && mattk->aatyp == AT_BITE
                && mdef->data->msize <= MZ_SMALL
                && is_animal(mdef->data)
                && !(mdef->mfrozen || mdef->mstone
                     || mdef->mconf || mdef->mstun) && rn2(3)) {
                if (gv.vis && canspotmon(mdef))
                    pline_mon(mdef, "%s nimbly %s %s bite!", Monnam(mdef),
                              rn2(2) ? "dodges" : "evades",
                              s_suffix(mon_nam(magr)));
                strike = 0;
            }
            if (strike) {
                short type = 0;
                int corpsenm = NON_PM;

                /* for eel AT_TUCH+AD_WRAP attack: can't grab an unsolid
                   target; the unsolid test is redundant since failed_grab
                   checks it too, but is cheap and avoids calling failed_grab
                   for ordinary targets */
                if (unsolid(mdef->data) && failed_grab(magr, mdef, mattk)) {
                    strike = 0;
                    break;
                }
                /* glass breakage from the attack. master does this
                   inside hitmm(), which takes mwep by value, so its
                   own null-out never reaches this scope; done here
                   instead, at the same point in the sequence, so the
                   pudding-split test below and passivemm()'s AD_ENCH
                   drain_item() cannot see a deleted weapon */
                (void) break_glass_obj(some_armor(mdef));
                if (break_glass_obj(mwep))
                    mwep = (struct obj *) 0;
                if (DEADMONSTER(mdef)) {
                    /* mselftouch() inside the armor break can petrify
                       a cockatrice-wielding defender; the blow itself
                       never landed, so no on-hit passive damage */
                    strike = 0;
                    res[i] = M_ATTK_DEF_DIED;
                    break;
                }

                wep_id = mwep ? mwep->o_id : 0;
                if (mwep) {
                    type = mwep->otyp;
                    corpsenm = mwep->corpsenm;
                }
                res[i] = hitmm(magr, mdef, mattk, mwep, dieroll);
                /* hitmm() takes mwep by value and rustm() down that path
                   can erode it out of existence, so re-check it here for
                   the pudding-split test below and for passivemm() */
                if (mwep && !o_on(wep_id, magr->minvent))
                    mwep = (struct obj *) 0;
                if (res[i] == M_ATTK_HIT && mwep && type == CORPSE
                    && safe_touch_petrifies(corpsenm)
                    && !(resists_ston(mdef) || defended(mdef, AD_STON))) {
                    /* a wielded petrifying corpse also stones the target
                       of a non-physical weapon attack, whose damage
                       handler never looks at the weapon; there is no
                       gradual-stoning countdown for monsters yet, so the
                       blow petrifies outright, the way do_stone_mon()
                       resolves a physical hit */
                    if (munstone(mdef, FALSE)) {
                        ; /* ate a cure, which may itself have been fatal */
                    } else if (poly_when_stoned(mdef->data)) {
                        mon_to_stone(mdef);
                    } else {
                        if (gv.vis && canseemon(mdef))
                            pline_mon(mdef, "%s turns to stone!",
                                      Monnam(mdef));
                        if (magr->mtame)
                            set_pet_killer(magr);
                        monstone(mdef);
                    }
                    if (DEADMONSTER(mdef))
                        res[i] = (M_ATTK_DEF_DIED
                                  | (grow_up(magr, mdef) ? 0
                                                         : M_ATTK_AGR_DIED));
                    break;
                }
                if ((mdef->data == &mons[PM_BLACK_PUDDING]
                     || mdef->data == &mons[PM_BROWN_PUDDING])
                    && (mwep && (mwep->material == IRON
                                 || mwep->material == STEEL))
                    && mdef->mhp > 1 && !mdef->mcan) {
                    struct monst *mclone;

                    if ((mclone = clone_mon(mdef, 0, 0)) != 0) {
                        if (gv.vis && canspotmon(mdef))
                            pline("%s divides as %s hits it!",
                                  Monnam(mdef), mon_nam(magr));
                        (void) mintrap(mclone, NO_TRAP_FLAGS);
                        if (DEADMONSTER(magr))
                            res[i] |= M_ATTK_AGR_DIED;
                    }
                }
            } else {
                missmm(magr, mdef, tmp, dieroll, mattk);
                /* a glancing blow off armor of a material the attacker
                   hates can kill it, and its armor breaking can petrify
                   a cockatrice-wielding defender; either way the rest
                   of the attack chain must not run; both can die on the
                   same miss, so the flags must accumulate */
                if (DEADMONSTER(mdef))
                    res[i] = M_ATTK_DEF_DIED;
                if (DEADMONSTER(magr))
                    res[i] |= M_ATTK_AGR_DIED;
            }
            break;

        case AT_HUGS: /* automatic if prev two attacks succeed */
            strike = (i >= 2 && res[i - 1] == M_ATTK_HIT
                      && res[i - 2] == M_ATTK_HIT);
            /* only a hug that would land can go to the image */
            if (strike && (is_displaced(mdef->data) || has_displacement(mdef))
                && rn2(4)) {
                if (gv.vis && canspotmon(mdef))
                    pline_mon(mdef, "%s attacks the displaced image of %s.",
                              Monnam(magr), mon_nam(mdef));
                strike = 0;
            }
            if (strike) {
                /* note: monsters with hug attacks don't wear cloaks or gloves
                   so this doesn't need a special case for hugging a shade
                   while covered by blessed armor (which does damage but does
                   not achieve a successful hold); likewise, rope golems can't
                   wield weapons so ability to choke isn't affected by such */
                if (failed_grab(magr, mdef, mattk)) {
                    strike = 0;
                } else {
                    /* a squeeze breaks the defender's fragile armor too;
                       master gets this for free by breaking inside
                       hitmm(), which both of its call sites reach */
                    (void) break_glass_obj(some_armor(mdef));
                    if (DEADMONSTER(mdef)) {
                        /* the squeeze itself never landed, so no
                           on-hit passive damage */
                        strike = 0;
                        res[i] = M_ATTK_DEF_DIED;
                        break;
                    }
                    res[i] = hitmm(magr, mdef, mattk, (struct obj *) 0, 0);
                }
            }
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
            if (res[i] == M_ATTK_MISS) { /* cancelled--no attack */
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
                if (gv.vis)
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
            if (engulfing_u(magr)) {
                strike = 0;
            } else if ((strike = (tmp > (dieroll = rnd(20 + i)))) != 0) {
                if (failed_grab(magr, mdef, mattk))
                    strike = 0; /* purple worm can't swallow unsolid mons */
                else
                    res[i] = gulpmm(magr, mdef, mattk);
            } else {
                /* the roll is captured above rather than left inline:
                   master passes the melee path's stale dieroll here, and
                   the blocker ladder needs the roll this attack made */
                missmm(magr, mdef, tmp, dieroll, mattk);
                /* both can die on the same miss (see the melee arm),
                   so the flags must accumulate */
                if (DEADMONSTER(mdef))
                    res[i] = M_ATTK_DEF_DIED;
                if (DEADMONSTER(magr))
                    res[i] |= M_ATTK_AGR_DIED;
            }
            break;

        case AT_BREA:
        case AT_SPIT:
            /*
             * Ranged attacks aren't allowed at point blank range.
             *
             * That impacts pet use of ranged attacks.  It's rather arbitrary
             * but various parts of the code assume it to be the case, not to
             * mention a part of player tactics when fighting dragons.
             */
            if (!monnear(magr, mdef->mx, mdef->my)) {
                int mmtmp;

                /* spitmm() walks bhitpos like a thrown missile (breamm()
                   restores its own); see the AT_WEAP ranged arm */
                save_bhitpos = gb.bhitpos;
                save_notonhead = gn.notonhead;
                mmtmp = ((mattk->aatyp == AT_BREA)
                         ? breamm(magr, mattk, mdef)
                         : spitmm(magr, mattk, mdef));
                gb.bhitpos = save_bhitpos;
                gn.notonhead = save_notonhead;

                strike = (mmtmp == M_ATTK_MISS) ? 0 : 1;
                /* We don't really know if we hit or not; pretend we did. */
                if (strike)
                    res[i] |= M_ATTK_HIT;
                if (DEADMONSTER(mdef))
                    res[i] = M_ATTK_DEF_DIED;
                if (DEADMONSTER(magr))
                    res[i] |= M_ATTK_AGR_DIED;
            } else {
                strike = 0;
                attk = 0;
            }
            break;

        case AT_MAGC:
            /* monster-vs-monster spellcasting: spells learned from
               spellbooks, the charm-monster untame special, the
               innate elemental casts (hellfire and kin, via castmm
               adjacent or buzzmm rays at range), and the innate
               AD_SPEL/AD_CLRC spell lists (castmm) */
            if (mattk->adtyp == AD_FIRE || mattk->adtyp == AD_COLD
                || mattk->adtyp == AD_ACID || mattk->adtyp == AD_MAGM) {
                int mmtmp = !monnear(magr, mdef->mx, mdef->my)
                            ? buzzmm(magr, mdef, mattk)
                            : castmm(magr, mdef, mattk);

                strike = (mmtmp == M_ATTK_MISS) ? 0 : 1;
                /* we don't really know if we hit or not; pretend we did */
                if (strike)
                    res[i] |= M_ATTK_HIT;
                if (DEADMONSTER(mdef)) {
                    res[i] = M_ATTK_DEF_DIED;
                    /* monster gains experience for a spell kill */
                    if (!grow_up(magr, mdef))
                        res[i] |= M_ATTK_AGR_DIED;
                }
                if (DEADMONSTER(magr))
                    res[i] |= M_ATTK_AGR_DIED;
            } else if (mattk->adtyp == AD_SPEL || mattk->adtyp == AD_CLRC) {
                strike = (castmm(magr, mdef, mattk) == M_ATTK_MISS) ? 0 : 1;
                /* we don't really know if we hit or not; pretend we did */
                if (strike)
                    res[i] |= M_ATTK_HIT;
                if (DEADMONSTER(mdef)) {
                    res[i] = M_ATTK_DEF_DIED;
                    /* monster gains experience for a spell kill */
                    if (!grow_up(magr, mdef))
                        res[i] |= M_ATTK_AGR_DIED;
                }
                if (DEADMONSTER(magr))
                    res[i] |= M_ATTK_AGR_DIED;
            } else {
                strike = 0;
                attk = 0;
            }
            break;

        default: /* no attack */
            strike = 0;
            attk = 0;
            break;
        }

        if (attk && !(res[i] & M_ATTK_AGR_DIED)
            && distmin(magr->mx, magr->my, mdef->mx, mdef->my) <= 1)
            res[i] = passivemm(magr, mdef, strike,
                               (res[i] & M_ATTK_DEF_DIED), mwep,
                               (int) mattk->aatyp);

        if (res[i] & M_ATTK_DEF_DIED)
            return res[i];
        if (res[i] & M_ATTK_AGR_DIED)
            return res[i];
        /* return if aggressor can no longer attack */
        if ((res[i] & M_ATTK_AGR_DONE) || helpless(magr))
            return res[i];
        /* eg. defender was knocked into a level teleport trap; a steed
           carrying a monster rider is off the map on purpose */
        if (mon_offmap(mdef) && !mdef->ridden_by)
            return res[i];
        if (res[i] & M_ATTK_HIT)
            struck = 1; /* at least one hit */
    } /* for (;i < NATTK;) loop */

    /* a Healer senses the change in a pet's condition */
    if (struck && mdef->mtame)
        print_mon_wounded(mdef, saved_mhp);

    return (struck ? M_ATTK_HIT : M_ATTK_MISS);
}

/* can't hold an unsolid target (ghosts, lights, vortices, most elementals)
   or a long worm tail */
boolean
failed_grab(
    struct monst *magr,
    struct monst *mdef,
    struct attack *mattk)
{
    if ((unsolid(mdef->data) || gn.notonhead)
        /* hug attack: most holders (owlbear, python, pit fiend, &c);
           wrap damage: eel grabbing, trapper/lurker-above engulfing;
           stick-to damage: mimic, lichen;
           digestion damage: purple worm swallowing;
           physical tentacle: the neothelid's hold on the hero */
        && (mattk->aatyp == AT_HUGS || mattk->adtyp == AD_WRAP
            || mattk->adtyp == AD_STCK  || mattk->adtyp == AD_DGST
            || (mdef == &gy.youmonst && mattk->aatyp == AT_TENT
                && mattk->adtyp == AD_PHYS
                && magr->data == &mons[PM_NEOTHELID]))) {
        if ((gv.vis && canspotmon(mdef)) /* mon-vs-mon */
            || magr == &gy.youmonst || mdef == &gy.youmonst) {
            char magrnam[BUFSZ], mdefnam[BUFSZ];
            boolean tailmiss = gn.notonhead;
            const char *verb = (mattk->adtyp == AD_DGST) ? "gulp"
                               : (mattk->adtyp == AD_STCK) ? "adhere"
                                 : "grab";

            /* beware of "Foo's grab passes through Bar's ghost";
               mon_nam(x_monnam) calls s_suffix() for named ghosts and
               s_suffix() uses a single static buffer; make copies of both
               names to overcome that [note: comment predates 'tailmiss'] */
            Strcpy(magrnam, (magr == &gy.youmonst) ? "Your"
                                                   : s_suffix(Monnam(magr)));
            if (!tailmiss) {
                Strcpy(mdefnam, (mdef == &gy.youmonst) ? "you"
                                                       : mon_nam(mdef));
            } else {
                /* hero poly'd into long worm can't grow tail
                   so no 'youmonst' handling is needed here */
                Sprintf(mdefnam, "%s tail", s_suffix(some_mon_nam(mdef)));
            }
            /* unsolid grab misses are actually somewhat iffy--how come
               ordinary attacks don't also pass right through? */
            pline("%.99s %s attempt %s %.99s!", magrnam, verb,
                  !tailmiss ? "passes right through" : "fails to hold",
                  mdefnam);
        }
        return TRUE;
    }
    return FALSE;
}

/* Returns the result of mdamagem(). */
staticfn int
hitmm(
    struct monst *magr,
    struct monst *mdef,
    struct attack *mattk,
    struct obj *mwep,
    int dieroll)
{
    int compat;
    boolean weaponhit = (mattk->aatyp == AT_WEAP
                         || (mattk->aatyp == AT_CLAW && mwep)),
            hatedhit = (weaponhit && mwep
                        && mon_hates_material(mdef, mwep->material));

    /* Possibly awaken nearby monsters */
    if ((!is_silent(magr->data) || !helpless(mdef)) && rn2(10))
        wake_nearto(magr->mx, magr->my, combat_noise(magr->data));

    pre_mm_attack(magr, mdef);

    compat = !magr->mcan ? could_seduce(magr, mdef, mattk) : 0;
    if (!compat && shade_miss(magr, mdef, mwep, FALSE, gv.vis))
        return M_ATTK_MISS; /* bypass mdamagem() */

    if (gv.vis) {
        char buf[BUFSZ], magr_name[BUFSZ];

        Strcpy(magr_name, Monnam(magr));
        if (compat) {
            Snprintf(buf, sizeof buf, "%s %s", magr_name,
                    mdef->mcansee ? "smiles at" : "talks to");
            pline("%s %s %s.", buf, mon_nam(mdef),
                  (compat == 2) ? "engagingly" : "seductively");
        } else {
            /* every blow names its verb here, artifact weapon or not;
               mhitm_ad_phys() adds no hit line of its own */
            buf[0] = '\0';
            switch (mattk->aatyp) {
            case AT_BITE:
                Snprintf(buf, sizeof buf, "%s %s", magr_name,
                         has_beak(magr->data) ? "pecks" : "bites");
                break;
            case AT_KICK:
                Snprintf(buf, sizeof buf, "%s %s", magr_name,
                         is_quadruped(magr->data) ? "tramples" : "kicks");
                break;
            case AT_STNG:
                Snprintf(buf, sizeof buf, "%s stings", magr_name);
                break;
            case AT_BUTT:
                Snprintf(buf, sizeof buf, "%s %s", magr_name,
                         (has_trunk(magr->data)
                          || num_horns(magr->data) > 0) ? "gores" : "butts");
                break;
            case AT_TUCH:
                if (magr->data == &mons[PM_DEATH])
                    Snprintf(buf, sizeof buf,
                             "%s reaches out with its deadly touch towards",
                             magr_name);
                else if (magr->data == &mons[PM_GIANT_CENTIPEDE])
                    Snprintf(buf, sizeof buf, "%s coils its body around",
                             magr_name);
                else if (magr->data == &mons[PM_ASSASSIN_VINE])
                    Snprintf(buf, sizeof buf,
                             "%s lashes its thorny vines at", magr_name);
                else
                    Snprintf(buf, sizeof buf, "%s touches", magr_name);
                break;
            case AT_TENT:
                if (magr->data == &mons[PM_MEDUSA])
                    Snprintf(buf, sizeof buf,
                             "The venomous snakes on %s head attack",
                             s_suffix(magr_name));
                else if (magr->data == &mons[PM_DEMOGORGON]
                         || magr->data == &mons[PM_NEOTHELID])
                    Snprintf(buf, sizeof buf, "%s tentacles lash out at",
                             s_suffix(magr_name));
                else
                    Snprintf(buf, sizeof buf, "%s tentacles suck",
                             s_suffix(magr_name));
                break;
            case AT_WEAP:
                if (!mwep) { /* AT_WEAP but isn't wielding anything */
                    if (has_claws(r_data(magr)))
                        Snprintf(buf, sizeof buf, "%s claws", magr_name);
                    else if (has_claws_undead(r_data(magr)))
                        Snprintf(buf, sizeof buf, "%s scratches",
                                 magr_name);
                    else
                        Snprintf(buf, sizeof buf, "%s %ss", magr_name,
                                 mwep_none[rn2(SIZE(mwep_none))]);
                } else if (is_pierce(mwep)) {
                    Snprintf(buf, sizeof buf, "%s %ss", magr_name,
                             mwep_pierce[rn2(SIZE(mwep_pierce))]);
                } else if (is_slash(mwep)) {
                    Snprintf(buf, sizeof buf, "%s %ss", magr_name,
                             mwep_slash[rn2(SIZE(mwep_slash))]);
                } else if (is_whack(mwep)) {
                    Snprintf(buf, sizeof buf, "%s %ss", magr_name,
                             mwep_whack[rn2(SIZE(mwep_whack))]);
                } else {
                    Snprintf(buf, sizeof buf, "%s hits", magr_name);
                }
                break;
            case AT_CLAW:
                if (has_claws(r_data(magr)))
                    Snprintf(buf, sizeof buf, "%s claws", magr_name);
                else if (has_claws_undead(r_data(magr)))
                    Snprintf(buf, sizeof buf, "%s scratches", magr_name);
                else
                    Snprintf(buf, sizeof buf, "%s hits", magr_name);
                break;
            case AT_HUGS:
                if (magr != u.ustuck) {
                    Snprintf(buf, sizeof buf, "%s squeezes", magr_name);
                    break;
                }
                FALLTHROUGH;
                /*FALLTHRU*/
            default:
                Snprintf(buf, sizeof buf, "%s hits", magr_name);
                break;
            }
            if (*buf)
                pline("%s %s.", buf, mon_nam_too(mdef, magr));

            if (hatedhit)
                searmsg(magr, mdef, mwep, FALSE);
        }
    } else
        noises(magr, mattk);

    return mdamagem(magr, mdef, mattk, mwep, dieroll);
}

/* Returns the same values as mdamagem(). */
staticfn int
gazemm(struct monst *magr, struct monst *mdef, struct attack *mattk)
{
    char buf[BUFSZ];
    /* an Archon's gaze affects target even if Archon itself is blinded */
    boolean archon = (magr->data == &mons[PM_ARCHON]
                      && mattk->adtyp == AD_BLND),
            altmesg = (archon && !magr->mcansee);

    /* bring target out of hiding even if hero doesn't see it happen (this
       is already done in pre_mm_attack() and shouldn't be needed here) */
    if (mdef->data->mlet == S_MIMIC && M_AP_TYPE(mdef) != M_AP_NOTHING)
        seemimic(mdef);
    mdef->mundetected = 0;

    if (gv.vis) {
        Sprintf(buf, "%s gazes %s",
                altmesg ? Adjmonnam(magr, "blinded") : Monnam(magr),
                altmesg ? "toward" : "at");
        pline("%s %s...", buf,
              canspotmon(mdef) ? mon_nam(mdef) : "something");
    }

    if (magr->mcan || !mdef->mcansee
        || (archon ? resists_blnd(mdef) : !magr->mcansee)
        || (magr->minvis && !racial_perceives(mdef)) || mdef->msleeping) {
        if (gv.vis && canspotmon(mdef))
            pline("but nothing happens.");
        return M_ATTK_MISS;
    }
    /* call mon_reflects 2x, first test, then, if visible, print message */
    if (magr->data == &mons[PM_MEDUSA] && mon_reflects(mdef, (char *) 0)) {
        if (canseemon(mdef))
            (void) mon_reflects(mdef, "The gaze is reflected away by %s %s.");
        if (mdef->mcansee) {
            if (mon_reflects(magr, (char *) 0)) {
                if (canseemon(magr))
                    (void) mon_reflects(magr,
                                      "The gaze is reflected away by %s %s.");
                return M_ATTK_MISS;
            }
            if (mdef->minvis && !racial_perceives(magr)) {
                if (canseemon(magr)) {
                    pline(
                      "%s doesn't seem to notice that %s gaze was reflected.",
                          Monnam(magr), mhis(magr));
                }
                return M_ATTK_MISS;
            }
            if (canseemon(magr))
                pline_mon(magr, "%s is turned to stone!", Monnam(magr));
            if (mdef->mtame)
                set_pet_killer(mdef);
            monstone(magr);
            if (!DEADMONSTER(magr))
                return M_ATTK_MISS;
            return M_ATTK_AGR_DIED;
        }
    } else if (archon) {
        mhitm_ad_blnd(magr, mattk, mdef, (struct mhitm_data *) 0);
        /* an Archon's blinding radiance also stuns;
           this is different from the way the hero gets stunned because
           a stunned monster recovers randomly instead of via countdown;
           both cases make an effort to prevent the target from being
           continuously stunned due to repeated gaze attacks */
        if (rn2(2))
            mdef->mstun = 1;
    }

    return mdamagem(magr, mdef, mattk, (struct obj *) 0, 0);
}

/* monster screams at another monster; there is no range gate and no
   cooldown check or charge - only the hero-directed scream has one */
staticfn int
screamm(struct monst *magr, struct monst *mdef, struct attack *mattk)
{
    Soundeffect(se_scream, 60);
    if (canseemon(magr) && !Deaf)
        pline_mon(magr, "%s lets out a %s!", Monnam(magr),
                  magr->data == &mons[PM_NAZGUL]
                      ? "bloodcurdling scream" : "deafening roar");
    return mdamagem(magr, mdef, mattk, (struct obj *) 0, 0);
}

/* return True if magr is allowed to swallow mdef, False otherwise */
boolean
engulf_target(struct monst *magr, struct monst *mdef)
{
    struct rm *lev;
    int ax, ay, dx, dy;
    boolean uatk = (magr == &gy.youmonst),
            udef = (mdef == &gy.youmonst);

    /* can't swallow something that's too big */
    if (r_data(mdef)->msize >= MZ_HUGE
        || (r_data(magr)->msize < r_data(mdef)->msize
            && !is_whirly(magr->data)))
        return FALSE;

    /* can't (move to) swallow if trapped. TODO: could do some? */
    if (mdef->mtrapped || magr->mtrapped)
        return FALSE;

    /* can't swallow entangled monsters */
    if (mdef->mentangled)
        return FALSE;

    /* can't swallow something if riding / being ridden */
    if (magr->ridden_by || mdef->ridden_by || has_erid(magr))
        return FALSE;

    /* if attacker is phasing in solid rock and defender can't move there,
       or vice versa, don't allow engulf to succeed; otherwise expelling
       might not be able to place attacker and defender both back on map;
       when defender is the hero, a sanity_check complaint about placing
       the hero on top of a monster can occur */
    dx = (mdef == &gy.youmonst) ? u.ux : mdef->mx;
    dy = (mdef == &gy.youmonst) ? u.uy : mdef->my;
    lev = &levl[dx][dy];
    if (!(udef ? Passes_walls : passes_walls(mdef->data))
          && (IS_OBSTRUCTED(lev->typ) || closed_door(dx, dy) || IS_TREE(lev->typ)
              /* not passes_bars(); engulfer isn't squeezing through */
              || (lev->typ == IRONBARS && !is_whirly(magr->data))))
        return FALSE;
    ax = (magr == &gy.youmonst) ? u.ux : magr->mx;
    ay = (magr == &gy.youmonst) ? u.uy : magr->my;
    lev = &levl[ax][ay];
    if (!(uatk ? Passes_walls : passes_walls(magr->data))
        && (IS_OBSTRUCTED(lev->typ) || closed_door(ax, ay) || IS_TREE(lev->typ)
            || (lev->typ == IRONBARS && !is_whirly(mdef->data))))
        return FALSE;

    return TRUE;
}

/* Returns the same values as mattackm(). */
staticfn int
gulpmm(
    struct monst *magr,
    struct monst *mdef,
    struct attack *mattk)
{
    coordxy ax, ay, dx, dy;
    int status;
    struct obj *obj;

    if (!engulf_target(magr, mdef))
        return M_ATTK_MISS;

    if (has_erid(mdef)) {
        if (gv.vis)
            pline("%s plucks %s right off %s mount!", Monnam(magr),
                  mon_nam(mdef), mhis(mdef));
        separate_steed_and_rider(mdef);
        /* the separation rloc's the rider and re-runs mintrap at its
           new spot, which can kill it or migrate it off the level;
           either way the engulf must not continue */
        if (DEADMONSTER(mdef))
            return M_ATTK_DEF_DIED;
        if (mon_offmap(mdef))
            return M_ATTK_HIT;
    }

    if (gv.vis) {
        pline("%s %s %s.", Monnam(magr),
              digests(magr->data) ? "swallows"
              : enfolds(magr->data) ? "encloses"
                : "engulfs",
              mon_nam(mdef));
    }
    if (!flaming(magr->data)) {
        for (obj = mdef->minvent; obj; obj = obj->nobj)
            (void) snuff_lit(obj, FALSE);
    }

    if (is_vampshifter(mdef)
        && newcham(mdef, &mons[mdef->cham], NO_NC_FLAGS)) {
        if (gv.vis) {
            /* 'it' -- previous form is no longer available and
               using that would be excessively verbose */
            pline("%s expels %s.", Monnam(magr),
                  canspotmon(mdef) ? "it" : something);
            if (canspotmon(mdef)) {
                pline("It turns into %s.",
                      x_monnam(mdef, ARTICLE_A, (char *) 0,
                               (SUPPRESS_NAME | SUPPRESS_IT
                                | SUPPRESS_INVISIBLE), FALSE));
            }
        }
        return M_ATTK_HIT; /* bypass mdamagem() */
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
     *  Leave the defender in the monster chain at its current position,
     *  but don't leave it on the screen.  Move the aggressor to the
     *  defender's position.
     */
    remove_monster(dx, dy);
    remove_monster(ax, ay);
    place_monster(magr, dx, dy);
    newsym(ax, ay); /* erase old position */
    newsym(dx, dy); /* update new position */

    gm.mswallower = magr; /* corpse_chance() wants this */
    status = mdamagem(magr, mdef, mattk, (struct obj *) 0, 0);
    gm.mswallower = (struct monst *) 0; /* reset */

    if ((status & (M_ATTK_AGR_DIED | M_ATTK_DEF_DIED))
        == (M_ATTK_AGR_DIED | M_ATTK_DEF_DIED)) {
        ;                              /* both died -- do nothing  */
    } else if (status & M_ATTK_DEF_DIED) { /* defender died */
        /*
         *  Note: mdamagem() -> monkilled() -> mondead() -> m_detach()
         *  -> relmon() used to call remove_monster() for the dead
         *  monster even when it wasn't the one on the map, so we
         *  needed to put magr back after mdef was killed and removed
         *  from their shared spot.  But now [5.0] relmon() calls
         *  mon_leaving_level() and that checks whether the monster at
         *  dying monster's coordinates is that dying monster and only
         *  removes it when they match.  So magr is still at mdef's
         *  former spot these days.
         *
         *  We still potentially do one fixup:  if the gulp targeted
         *  an inhospitable location, magr will return to its previous
         *  spot instead of staying.
         */
        if (!goodpos(dx, dy, magr, MM_IGNOREWATER)) {
            if (m_at(dx, dy) == magr) {
                remove_monster(dx, dy);
                newsym(dx, dy);
            }
            dx = ax, dy = ay; /* magr's spot at start of the attack */
        }
        if (m_at(dx, dy) != magr) {
            place_monster(magr, dx, dy);
            newsym(dx, dy);
        }
        /* aggressor moves to <dx,dy> and might encounter trouble there */
        if (minliquid(magr)
            || (t_at(dx, dy)
                && mintrap(magr, NO_TRAP_FLAGS) == Trap_Killed_Mon))
            status |= M_ATTK_AGR_DIED;
    } else if (status & M_ATTK_AGR_DIED) { /* aggressor died */
        place_monster(mdef, dx, dy);
        newsym(dx, dy);
    } else {                           /* both alive, put them back */
        if (cansee(dx, dy)) {
            if (status & M_ATTK_EXPELLED) {
                pline("%s hurriedly %s %s!", Monnam(magr),
                      digests(magr->data) ? "regurgitates"
                        : enfolds(magr->data) ? "releases"
                          : "expels",
                      mon_nam(mdef));
                if (digests(magr->data))
                    pline("Obviously, it didn't like %s taste.",
                          s_suffix(mon_nam(mdef)));
            } else {
                pline("%s is %s!", Monnam(mdef),
                      digests(magr->data) ? "regurgitated"
                        : enfolds(magr->data) ? "released"
                          : "expelled");
            }
        }

        remove_monster(dx,dy);
        place_monster(magr, ax, ay);
        place_monster(mdef, dx, dy);
        newsym(ax, ay);
        newsym(dx, dy);
    }

    return status;
}

staticfn int
explmm(struct monst *magr, struct monst *mdef, struct attack *mattk)
{
    int result;

    if (magr->mcan)
        return M_ATTK_MISS;

    if (cansee(magr->mx, magr->my))
        pline_mon(magr, "%s explodes!", Monnam(magr));
    else
        noises(magr, mattk);

    /* monster explosion types which actually create an explosion */
    if (mattk->adtyp == AD_FIRE || mattk->adtyp == AD_COLD
        || mattk->adtyp == AD_ELEC || mattk->adtyp == AD_ACID) {
        mon_explodes(magr, mattk);
        if (mdef && mattk->adtyp == AD_ACID) {
            if (rn2(4))
                erode_armor(mdef, ERODE_CORRODE);
            if (rn2(2)) {
                acid_damage(MON_WEP(mdef));
                acid_damage(MON_WEP2(mdef));
            }
        }
        /* unconditionally set AGR_DIED here; lifesaving is accounted below */
        result = M_ATTK_AGR_DIED | (DEADMONSTER(mdef) ? M_ATTK_DEF_DIED : 0);
        /* kludge to ensure the player gets experience if the
           exploding sphere was conjured via spell */
        if (magr->uexp && (result & M_ATTK_DEF_DIED)) {
            int mndx = monsndx(mdef->data);
            int tmp = experience(mdef, (int) svm.mvitals[mndx].died + 1);

            more_experienced(tmp, 0);
            newexplevel();
        }
    } else {
        result = mdamagem(magr, mdef, mattk, (struct obj *) 0, 0);
    }

    /* Kill off aggressor if it didn't die. */
    if (!(result & M_ATTK_AGR_DIED)) {
        boolean was_leashed = (magr->mleashed != 0);

        mondead(magr);
        if (!DEADMONSTER(magr))
            return result; /* life saved */
        result |= M_ATTK_AGR_DIED;

        /* mondead() -> m_detach() -> m_unleash() always suppresses
           the m_unleash() slack message, so deliver it here instead */
        if (was_leashed)
            Your("leash falls slack.");
    }
    /* Player can potentially get the blame for orbs of fire/frost */
    if (magr->msummoned && !(result & M_ATTK_DEF_DIED)) {
        if (P_SKILL(P_MATTER_SPELL) < P_BASIC) {
            /* no skill, potential alignment record ding */
            setmangry(mdef, TRUE);
        } else {
            if (mdef->mpeaceful) {
                /* peaceful becomes hostile, but no alignment record ding */
                mdef->mpeaceful = 0;
                set_malign(mdef);
                newsym(mdef->mx, mdef->my);
                if (canspotmon(mdef)) {
                    if (humanoid(mdef->data)
                        || mdef->isshk || mdef->isgd)
                        pline("%s gets agitated!", Monnam(mdef));
                    else if (flags.verbose && !Deaf)
                        growl(mdef);
                }
            }
        }
    }
    if (magr->mtame && !magr->msummoned) /* give this one even if it was visible */
        You(brief_feeling, "melancholy");

    return result;
}

/*
 *  See comment at top of mattackm(), for return values.
 */
staticfn int
mdamagem(
    struct monst *magr,
    struct monst *mdef,
    struct attack *mattk,
    struct obj *mwep,
    int dieroll)
{
    struct permonst *pa = magr->data, *pd = mdef->data;
    struct mhitm_data mhm;
    unsigned wep_id;

    mhm.damage = d((int) mattk->damn, (int) mattk->damd);
    mhm.hitflags = M_ATTK_MISS;
    mhm.permdmg = 0;
    mhm.specialdmg = 0;
    mhm.dieroll = dieroll;
    mhm.mwep = mwep;
    mhm.done = FALSE;

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
                return M_ATTK_HIT; /* no damage during the polymorph */
            }
            if (gv.vis && canspotmon(magr))
                pline_mon(magr, "%s turns to stone!", Monnam(magr));
            if (mdef->mtame)
                set_pet_killer(mdef);
            monstone(magr);
            if (!DEADMONSTER(magr))
                return M_ATTK_HIT; /* lifesaved */
            else if (magr->mtame && !gv.vis)
                You(brief_feeling, "peculiarly sad");
            return M_ATTK_AGR_DIED;
        }
    }

    wep_id = mwep ? mwep->o_id : 0;
    mhitm_adtyping(magr, mattk, mdef, &mhm);
    /* mwep is our caller's pointer by value; rustm() inside the AD_PHYS
       handler can erode the weapon away, so re-check before the tests
       further down read it */
    if (mwep && !o_on(wep_id, magr->minvent))
        mwep = (struct obj *) 0;

    /* a knocked-back defender that survived on the map still takes
       the blow's damage below; an attacker killed by the landing (a
       land mine, a magic beam trap) stops here with its death flagged,
       which the equality tests at the tail would otherwise mask */
    if (mhitm_knockback(magr, mdef, mattk, &mhm.hitflags,
                        (MON_WEP(magr) != 0))
        && ((mhm.hitflags & (M_ATTK_DEF_DIED | M_ATTK_AGR_DIED)) != 0
            || mon_offmap(mdef)))
        return mhm.hitflags;

    if (mhm.done)
        return mhm.hitflags;

    /* burning hands spell - extra fire damage on unarmed touch attacks */
    if (magr->mburnhands && !mwep && !DEADMONSTER(mdef)
        && (mattk->aatyp == AT_CLAW || mattk->aatyp == AT_TUCH
            || mattk->aatyp == AT_WEAP)) {
        int firedmg = rnd(6) + 2;

        if (resists_fire(mdef) || defended(mdef, AD_FIRE)
            || mon_underwater(mdef)) {
            shieldeff(mdef->mx, mdef->my);
            golemeffects(mdef, AD_FIRE, firedmg);
            if (gv.vis && canseemon(magr))
                pline("%s %s don't burn %s!", s_suffix(Monnam(magr)),
                      makeplural(mbodypart(magr, HAND)), mon_nam(mdef));
        } else {
            if (gv.vis && canseemon(magr))
                pline("%s burns %s with %s %s!", Monnam(magr),
                      mon_nam(mdef), mhis(magr),
                      makeplural(mbodypart(magr, HAND)));
            mhm.damage += firedmg;
            if (!rn2(4))
                mhm.damage += destroy_items(mdef, AD_FIRE, firedmg);
        }
        magr->mburnhands--;
        if (!magr->mburnhands && canseemon(magr))
            pline("%s %s stop burning.", s_suffix(Monnam(magr)),
                  makeplural(mbodypart(magr, HAND)));
    }

    /* shocking grasp spell - extra shock damage on unarmed touch attacks */
    if (magr->mshockgrasp && !mwep && !DEADMONSTER(mdef)
        && (mattk->aatyp == AT_CLAW || mattk->aatyp == AT_TUCH
            || mattk->aatyp == AT_WEAP)) {
        int elecdmg = rnd(6) + 2;

        if (resists_elec(mdef) || defended(mdef, AD_ELEC)) {
            shieldeff(mdef->mx, mdef->my);
            golemeffects(mdef, AD_ELEC, elecdmg);
            if (gv.vis && canseemon(magr))
                pline("%s %s don't shock %s!", s_suffix(Monnam(magr)),
                      makeplural(mbodypart(magr, HAND)), mon_nam(mdef));
        } else {
            if (gv.vis && canseemon(magr))
                pline("%s %s %s with %s %s!", Monnam(magr),
                      rn2(2) ? "jolts" : "shocks", mon_nam(mdef),
                      mhis(magr), makeplural(mbodypart(magr, HAND)));
            mhm.damage += elecdmg;
            if (!rn2(4))
                mhm.damage += destroy_items(mdef, AD_ELEC, elecdmg);
        }
        magr->mshockgrasp--;
        if (!magr->mshockgrasp && canseemon(magr))
            pline("%s %s stop shimmering.", s_suffix(Monnam(magr)),
                  makeplural(mbodypart(magr, HAND)));
    }

    if (!mhm.damage)
        return mhm.hitflags;

    mdef->mhp -= mhm.damage;
    if (mdef->mhp < 1) {
        if (m_at(mdef->mx, mdef->my) == magr) { /* see gulpmm() */
            remove_monster(mdef->mx, mdef->my);
            mdef->mhp = 1; /* otherwise place_monster will complain */
            place_monster(mdef, mdef->mx, mdef->my);
            mdef->mhp = 0;
        }
        if (mattk->aatyp == AT_WEAP || mattk->aatyp == AT_CLAW)
            gm.mkcorpstat_norevive = troll_baned(mdef, mwep) ? TRUE : FALSE;
        gz.zombify = (!mwep && zombie_maker(magr)
                     && can_become_zombie(r_data(mdef))
                     && (mattk->aatyp == AT_TUCH
                         || mattk->aatyp == AT_CLAW
                         || mattk->aatyp == AT_BITE)
                     && zombie_form(r_data(mdef)) != NON_PM);
        if (magr->uexp) {
            /* hero-conjured helper; hero gets credit and experience */
            mon_xkilled(mdef, "", (int) mattk->adtyp);
        } else {
            if (magr->mtame)
                set_pet_killer(magr);
            monkilled(mdef, "", (int) mattk->adtyp);
        }
        gz.zombify = FALSE; /* reset */
        gm.mkcorpstat_norevive = FALSE;
        if (!DEADMONSTER(mdef))
            return mhm.hitflags; /* mdef lifesaved */
        else if (mhm.hitflags == M_ATTK_AGR_DIED)
            return (M_ATTK_DEF_DIED | M_ATTK_AGR_DIED);

        /* improve pet handling some of the time if we see pet
           kill monsters */
        if (!rn2(3) && magr->mtame && canseemon(magr))
            use_skill(P_PET_HANDLING, 1);

        if (mattk->adtyp == AD_DGST) {
            /* various checks similar to dog_eat and meatobj.
             * after monkilled() to provide better message ordering */
            if (ismnum(mdef->cham)) {
                (void) newcham(magr, (struct permonst *) 0, NC_SHOW_MSG);
            } else if (pd == &mons[PM_GREEN_SLIME] && !slimeproof(pa)) {
                (void) newcham(magr, &mons[PM_GREEN_SLIME], NC_SHOW_MSG);
            } else if (pd == &mons[PM_WRAITH]) {
                (void) grow_up(magr, (struct monst *) 0);
                /* don't grow up twice */
                return (M_ATTK_DEF_DIED
                        | (!DEADMONSTER(magr) ? 0 : M_ATTK_AGR_DIED));
            } else if (pd == &mons[PM_NURSE]) {
                healmon(magr, magr->mhpmax, 0);
            }
            mon_givit(magr, pd);
        }
        /* caveat: above digestion handling doesn't keep `pa' up to date */

        return (M_ATTK_DEF_DIED
                | (grow_up(magr, mdef) ? 0 : M_ATTK_AGR_DIED));
    }
    return (mhm.hitflags == M_ATTK_AGR_DIED) ? M_ATTK_AGR_DIED : M_ATTK_HIT;
}

int
mon_poly(struct monst *magr, struct monst *mdef, int dmg)
{
    static const char freaky[] = " undergoes a freakish metamorphosis";
    struct permonst *oldform = mdef->data;

    if (mdef == &gy.youmonst) {
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
        } else if (Unchanging) {
            ; /* just take a little damage */
        } else {
            /* system shock might take place in polyself() */
            if (u.ulycn == NON_PM) {
                You("are subjected to a freakish metamorphosis.");
                polyself(POLY_NOFLAGS);
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
            if (gv.vis)
                shieldeff_mon(mdef);
        } else if (resist(mdef, WAND_CLASS, 0, TELL)) {
            /* general resistance to magic... */
            ;
        } else if (!rn2(25) && mdef->cham == NON_PM
                   && (mdef->mcan
                       || pm_to_cham(monsndx(mdef->data)) != NON_PM)) {
            /* system shock; this variation takes away half of mon's HP
               rather than kill outright */
            if (gv.vis)
                pline("%s shudders!", Before);

            dmg += (mdef->mhpmax + 1) / 2;
            mdef->mhp -= dmg;
            dmg = 0;
            if (DEADMONSTER(mdef)) {
                if (magr == &gy.youmonst) {
                    xkilled(mdef, XKILL_GIVEMSG | XKILL_NOCORPSE);
                } else {
                    if (magr->mtame)
                        set_pet_killer(magr);
                    monkilled(mdef, "", AD_RBRE);
                }
            }
        } else if (newcham(mdef, (struct permonst *) 0, NO_NC_FLAGS)) {
            if (gv.vis) { /* either seen or adjacent */
                boolean was_seen = !!strcmpi("It", Before),
                        verbosely = flags.verbose || !was_seen;

                if (canspotmon(mdef))
                    pline("%s%s%s turns into %s.", Before,
                          verbosely ? freaky : "", verbosely ? " and" : "",
                          x_monnam(mdef, ARTICLE_A, (char *) 0,
                                   (SUPPRESS_NAME | SUPPRESS_IT
                                    | SUPPRESS_INVISIBLE), FALSE));
                else if (was_seen || magr == &gy.youmonst)
                    pline("%s%s%s.", Before, freaky,
                          !was_seen ? "" : " and disappears");
            }
            dmg = 0;
            if (can_teleport(magr->data)) {
                if (magr == &gy.youmonst)
                    tele();
                else if (!tele_restrict(magr))
                    (void) rloc(magr, RLOC_MSG);
            }
        } else {
            if (gv.vis && flags.verbose)
                pline1(nothing_happens);
        }
    }
    /* when a transformation has happened, can't attack again for poly
       effect during next turn or two; not enforced for poly'd hero */
    if (mdef->data != oldform && magr != &gy.youmonst)
        magr->mspec_used += rnd(2);

    return dmg;
}

void
paralyze_monst(struct monst *mon, int amt)
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
sleep_monst(struct monst *mon, int amt, int how)
{
    /* reveal mimic unless already asleep or paralyzed (won't be 'busy') */
    if (how >= 0 && !mon->msleeping && !mon->mfrozen
        && mon->data->mlet == S_MIMIC && (M_AP_TYPE(mon) == M_AP_FURNITURE
                                          || M_AP_TYPE(mon) == M_AP_OBJECT))
        seemimic(mon);

    /* music can lull even sleep-resistant Cerberus, though never
       for more than a few turns at a time */
    if ((resists_sleep(mon) || defended(mon, AD_SLEE)
         || (how >= 0 && resist(mon, (char) how, 0, NOTELL)))
        && !(mon->data == &mons[PM_CERBERUS] && how == TOOL_CLASS)) {
        shieldeff(mon->mx, mon->my);
    } else if (mon->mcanmove) {
        finish_meating(mon); /* terminate any meal-in-progress */
        amt += (int) mon->mfrozen;
        if (amt > 0) { /* sleep for N turns */
            mon->mcanmove = 0;
            if (mon->data == &mons[PM_CERBERUS])
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
slept_monst(struct monst *mon)
{
    if (helpless(mon) && mon == u.ustuck
        && !sticks(gy.youmonst.data) && !u.uswallow) {
        pline_mon(mon, "%s grip relaxes.", s_suffix(Monnam(mon)));
        unstuck(mon);
    }
}

void
rustm(struct monst *mdef, struct obj *obj)
{
    int dmgtyp = ERODE_NONE, chance = 1;

    if (!mdef || !obj)
        return; /* just in case */
    /* AD_ACID and AD_ENCH are handled in passivemm() and passiveum() */
    if (dmgtype(mdef->data, AD_CORR) || dmgtype(mdef->data, AD_DCAY)) {
        dmgtyp = ERODE_CORRODE;
    } else if (dmgtype(mdef->data, AD_RUST)) {
        dmgtyp = ERODE_RUST;
    } else if (dmgtype(mdef->data, AD_FIRE)
               /* steam vortex: fire resist applies, fire damage doesn't */
               && mdef->data != &mons[PM_STEAM_VORTEX]) {
        dmgtyp = ERODE_BURN;
        chance = 6;
    }

    /* EF_DESTROY: a weapon already eroded to the limit is finished off
       here rather than surviving forever, so every caller must treat the
       object as possibly freed once this returns */
    if (dmgtyp != ERODE_NONE && !rn2(chance))
        (void) erode_obj(obj, (char *) 0, dmgtyp, EF_GREASE | EF_DESTROY);
}

staticfn void
mswingsm(
    struct monst *magr, /* attacker */
    struct monst *mdef, /* defender */
    struct obj *otemp)  /* attacker's weapon */
{
    if (flags.verbose && !Blind && mon_visible(magr)) {
        boolean bash = (is_pole(otemp) && !is_art(otemp, ART_SNICKERSNEE)
                        && (dist2(magr->mx, magr->my, mdef->mx, mdef->my)
                            <= 2));

        pline("%s %s %s%s %s at %s.", Monnam(magr), mswings_verb(otemp, bash),
              (otemp->quan > 1L) ? "one of " : "", mhis(magr), xname(otemp),
              mon_nam(mdef));
    }
}

/* passivemm(): the defender's passive effect killed the attacker;
   credit the kill the way the damage tail does and hand back the result
   mattackm() expects. The caller has already printed a death message,
   so monkilled() gets no text of its own. */
staticfn int
passive_agr_killed(
    struct monst *magr,
    struct monst *mdef,
    int adtyp,
    int mdead,
    int mhit)
{
    gz.zombify = FALSE; /* attacker's corpse never zombifies */
    if (mdef->mtame)
        set_pet_killer(mdef);
    if (mdef->uexp)
        /* hero-conjured helper's passive counterattack; the hero
           gets credit and experience */
        mon_xkilled(magr, (char *) 0, adtyp);
    else
        monkilled(magr, (char *) 0, adtyp);
    return (mdead | mhit | M_ATTK_AGR_DIED);
}

/* passivemm(): the defender's armor or hide damaged the attacker;
   returns 0 while it lives, otherwise the result to hand back */
staticfn int
passive_agr_dmg(
    struct monst *magr,
    struct monst *mdef,
    int dmg,
    int adtyp,
    int mdead,
    int mhit)
{
    if (damage_mon(magr, dmg, adtyp, FALSE)) {
        if (canseemon(magr))
            pline_mon(magr, "%s dies!", Monnam(magr));
        return passive_agr_killed(magr, mdef, adtyp, mdead, mhit);
    }
    return 0;
}

/* passivemm(): the defender's armor or hide disintegrated the attacker
   outright; Riders reintegrate, everything else dies without a corpse */
staticfn int
passive_disint_agr(
    struct monst *magr,
    struct monst *mdef,
    int mdead,
    int mhit)
{
    disint_mon_invent(magr);
    if (is_rider(magr->data)) {
        if (canseemon(magr)) {
            pline_mon(magr, "%s body reintegrates before your %s!",
                      s_suffix(Monnam(magr)),
                      (eyecount(gy.youmonst.data) == 1)
                          ? body_part(EYE)
                          : makeplural(body_part(EYE)));
            pline_mon(magr, "%s resurrects!", Monnam(magr));
        }
        magr->mhp = magr->mhpmax;
        return (mdead | mhit);
    }
    return passive_agr_killed(magr, mdef, AD_DISN, mdead, mhit);
}

/*
 * Passive responses by defenders.  Does not replicate responses already
 * handled above.  Returns same values as mattackm.
 */
staticfn int
passivemm(
    struct monst *magr,
    struct monst *mdef,
    boolean mhitb,
    int mdead,
    struct obj *mwep,
    int aatyp)
{
    struct permonst *mddat = mdef->data;
    struct permonst *madat = magr->data;
    /* defender's passive attacks come from its racial overlay's matrix */
    struct attack *mdattk = has_erac(mdef) ? ERAC(mdef)->mattk
                                           : mddat->mattk;
    char buf[BUFSZ];
    int i, tmp, res;
    int mhit = mhitb ? M_ATTK_HIT : M_ATTK_MISS;
    struct obj *passive_armor;

    /* dragon-scaled body armor strikes back at whatever hit its wearer */
    if ((passive_armor = which_armor(mdef, W_ARM)) != 0) {
        if (mhitb && !rn2(3) && Is_dragon_scaled_armor(passive_armor)) {
            int otyp = Dragon_armor_to_scales(passive_armor);

            switch (otyp) {
            case GREEN_DRAGON_SCALES:
                if (resists_poison(magr) || defended(magr, AD_DRST))
                    break;
                if (rn2(20)) {
                    if (!rn2(3)) {
                        if (canseemon(magr))
                            pline_mon(magr, "%s staggers from the poison!",
                                      Monnam(magr));
                        res = passive_agr_dmg(magr, mdef, rnd(4), AD_DRST,
                                              mdead, mhit);
                        if (res)
                            return res;
                    }
                } else {
                    if (canseemon(magr))
                        pline_mon(magr, "%s is fatally poisoned!",
                                  Monnam(magr));
                    return passive_agr_killed(magr, mdef, AD_DRST, mdead,
                                              mhit);
                }
                break;
            case BLACK_DRAGON_SCALES:
                if (resists_disint(magr) || defended(magr, AD_DISN)) {
                    if (canseemon(magr) && !rn2(3)) {
                        shieldeff(magr->mx, magr->my);
                        pline("%s armor does not appear to affect %s.",
                              s_suffix(Monnam(mdef)), mon_nam(magr));
                    }
                } else if (passive_contact_attack(aatyp)
                           /* a weapon, boots or gloves go first */
                           && !passive_disint_gear(magr, aatyp, 12, TRUE)) {
                    if (rn2(40)) {
                        if (canseemon(magr))
                            pline_mon(magr, "%s partially disintegrates!",
                                      Monnam(magr));
                        res = passive_agr_dmg(magr, mdef, rnd(4), AD_DISN,
                                              mdead, mhit);
                        if (res)
                            return res;
                    } else {
                        if (canseemon(magr))
                            pline_mon(magr, "%s is disintegrated completely!",
                                      Monnam(magr));
                        return passive_disint_agr(magr, mdef, mdead, mhit);
                    }
                }
                break;
            case ORANGE_DRAGON_SCALES:
                if (!rn2(3))
                    mon_adjust_speed(magr, -1, (struct obj *) 0);
                break;
            case WHITE_DRAGON_SCALES:
                if (resists_cold(magr) || defended(magr, AD_COLD))
                    break;
                if (rn2(20)) {
                    if (!rn2(3)) {
                        if (canseemon(magr))
                            pline_mon(magr, "%s flinches from the cold!",
                                      Monnam(magr));
                        res = passive_agr_dmg(magr, mdef, rnd(4), AD_COLD,
                                              mdead, mhit);
                        if (res)
                            return res;
                    }
                } else {
                    if (canseemon(magr))
                        pline_mon(magr, "%s is frozen solid!", Monnam(magr));
                    res = passive_agr_dmg(magr, mdef, d(6, 6), AD_COLD,
                                          mdead, mhit);
                    if (res)
                        return res;
                }
                break;
            case RED_DRAGON_SCALES:
                if (resists_fire(magr) || defended(magr, AD_FIRE)
                    || mon_underwater(magr))
                    break;
                if (rn2(20)) {
                    if (!rn2(3)) {
                        if (canseemon(magr))
                            pline_mon(magr, "%s is burned!", Monnam(magr));
                        res = passive_agr_dmg(magr, mdef, rnd(4), AD_FIRE,
                                              mdead, mhit);
                        if (res)
                            return res;
                    }
                } else {
                    if (canseemon(magr))
                        pline_mon(magr, "%s is severely burned!",
                                  Monnam(magr));
                    res = passive_agr_dmg(magr, mdef, d(6, 6), AD_FIRE,
                                          mdead, mhit);
                    if (res)
                        return res;
                }
                break;
            case YELLOW_DRAGON_SCALES:
                if (resists_acid(magr) || defended(magr, AD_ACID)
                    || mon_underwater(magr))
                    break;
                if (rn2(20)) {
                    if (!rn2(3)) {
                        if (canseemon(magr))
                            pline_mon(magr, "%s is seared!", Monnam(magr));
                        res = passive_agr_dmg(magr, mdef, rnd(4), AD_ACID,
                                              mdead, mhit);
                        if (res)
                            return res;
                    }
                } else {
                    if (canseemon(magr))
                        pline_mon(magr, "%s is critically seared!",
                                  Monnam(magr));
                    res = passive_agr_dmg(magr, mdef, d(6, 6), AD_ACID,
                                          mdead, mhit);
                    if (res)
                        return res;
                }
                break;
            case GRAY_DRAGON_SCALES:
                if (!rn2(6)) {
                    /* not the hero's attack; the armor's wearer gets the
                       credit and the blame if the cancellation kills a
                       clay golem attacker */
                    if (mdef->mtame)
                        set_pet_killer(mdef);
                    (void) cancel_monst(magr, (struct obj *) 0, FALSE,
                                        TRUE, FALSE);
                    set_pet_killer((struct monst *) 0);
                    if (DEADMONSTER(magr))
                        return (mdead | mhit | M_ATTK_AGR_DIED);
                }
                break;
            default: /* all other types of armor, just pass on through */
                break;
            }
        }
    }
    /* Dragonbane sears the dragon that just struck its wearer */
    if ((passive_armor = which_armor(mdef, W_ARMG)) != 0
        && passive_armor->otyp == GLOVES
        && passive_armor->oartifact == ART_DRAGONBANE
        && is_dragon(magr->data) && !rn2(3)) {
        if (canseemon(magr))
            pline("Dragonbane sears %s scaly hide!", s_suffix(mon_nam(magr)));
        if (damage_mon(magr, rnd(6) + 2, AD_PHYS, FALSE)) {
            if (canseemon(magr)) {
                pline("Dragonbane's power overwhelms %s!", mon_nam(magr));
                pline_mon(magr, "%s dies!", Monnam(magr));
            }
            return passive_agr_killed(magr, mdef, AD_PHYS, mdead, mhit);
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
        if (mhitb && !rn2(2)) {
            Strcpy(buf, Monnam(magr));
            if (canseemon(magr)) {
                if (mddat == &mons[PM_YELLOW_DRAGON])
                    pline("%s is seared by %s acidic hide!", buf,
                          s_suffix(mon_nam(mdef)));
                else
                    pline("%s is splashed by %s %s!", buf,
                          s_suffix(mon_nam(mdef)), hliquid("acid"));
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
            if (!rn2(6)) {
                acid_damage(MON_WEP(magr));
                acid_damage(MON_WEP2(magr));
            }
        }
        goto assess_dmg;
    case AD_DISN: { /* black dragon, antimatter vortex */
        boolean vortex = (mddat == &mons[PM_ANTIMATTER_VORTEX]);

        if (mhitb && !mdef->mcan) {
            if (resists_disint(magr) || defended(magr, AD_DISN)) {
                if (canseemon(magr) && !rn2(3)) {
                    shieldeff(magr->mx, magr->my);
                    pline("%s deadly %s does not appear to affect %s.",
                          s_suffix(Monnam(mdef)), vortex ? "form" : "hide",
                          mon_nam(magr));
                }
            } else if (passive_contact_attack(aatyp)
                       /* a weapon, boots or gloves go first */
                       && !passive_disint_gear(magr, aatyp, vortex ? 3 : 6,
                                               FALSE)) {
                if (vortex ? rn2(10) : rn2(20)) {
                    if (canseemon(magr))
                        pline("%s %s partially disintegrates %s!",
                              s_suffix(Monnam(mdef)),
                              vortex ? "form" : "hide", mon_nam(magr));
                    tmp = rn2(6) + 1;
                    goto assess_dmg;
                }
                if (canseemon(magr))
                    pline("%s deadly %s disintegrates %s!",
                          s_suffix(Monnam(mdef)), vortex ? "form" : "hide",
                          mon_nam(magr));
                return passive_disint_agr(magr, mdef, mdead, mhit);
            }
        }
        break;
    }
    case AD_DRST: /* green dragon */
        if (mhitb && !mdef->mcan && !rn2(3)) {
            if (resists_poison(magr) || defended(magr, AD_DRST)) {
                if (canseemon(magr) && !rn2(5))
                    pline("%s poisonous hide doesn't seem to affect %s.",
                          s_suffix(Monnam(mdef)), mon_nam(magr));
            } else if (rn2(20)) {
                if (canseemon(magr))
                    pline_mon(magr, "%s is poisoned!", Monnam(magr));
                tmp = rn2(4) + 1;
                goto assess_dmg;
            } else {
                if (canseemon(magr))
                    pline("%s poisonous hide was deadly...",
                          s_suffix(Monnam(mdef)));
                return passive_agr_killed(magr, mdef, AD_DRST, mdead, mhit);
            }
        }
        break;
    case AD_MAGM: /* the Oracle: wrath of the gods for attacking her */
        if (mhitb) {
            if (resists_mgc(magr->data)) {
                tmp = 0;
                if (canseemon(magr)) {
                    shieldeff(magr->mx, magr->my);
                    pline_mon(magr, thin_air_missiles, Monnam(magr));
                    pline_The("missiles bounce off!");
                }
            } else if (resists_magm(magr) || defended(magr, AD_MAGM)) {
                tmp = (tmp + 1) / 2;
                if (canseemon(magr)) {
                    shieldeff(magr->mx, magr->my);
                    pline_mon(magr, (madat == &mons[PM_WOODCHUCK])
                                        ? "ZOT!" : thin_air_missiles,
                              Monnam(magr));
                    pline("Some missiles bounce off!");
                }
                goto assess_dmg;
            } else {
                if (canseemon(magr))
                    pline_mon(magr, (madat == &mons[PM_WOODCHUCK])
                                        ? "ZOT!" : thin_air_missiles,
                              Monnam(magr));
                goto assess_dmg;
            }
        }
        break;
    case AD_ENCH: /* KMH -- remove enchantment (disenchanter) */
        if (mhitb && !mdef->mcan && mwep) {
            (void) drain_item(mwep, FALSE);
            /* No message */
        }
        break;
    case AD_CNCL: /* gray dragon */
        if (mhitb && !rn2(6)) {
            if (canseemon(magr))
                pline("%s hide absorbs magical energy from %s.",
                      s_suffix(Monnam(mdef)), mon_nam(magr));
            /* not the hero's attack; the defender gets the credit and
               the blame if the cancellation kills a clay golem */
            if (mdef->mtame)
                set_pet_killer(mdef);
            (void) cancel_monst(magr, mwep, FALSE, TRUE, FALSE);
            set_pet_killer((struct monst *) 0);
            if (DEADMONSTER(magr))
                return (mdead | mhit | M_ATTK_AGR_DIED);
        }
        break;
    case AD_SLIM: /* green slime */
        if (mhitb && !mdef->mcan && !rn2(3)) {
            if (gv.vis)
                pline("Its slime splashes onto %s!", mon_nam(magr));
            if (flaming(magr->data)) {
                if (gv.vis)
                    pline_The("slime burns away!");
                tmp = 0;
            } else if (slimeproof(magr->data)) {
                if (gv.vis && canseemon(magr))
                    pline_mon(magr, "%s is unaffected.", Monnam(magr));
                tmp = 0;
            } else if (!rn2(4)) {
                if (!munslime(magr, FALSE) && !DEADMONSTER(magr)) {
                    if (newcham(magr, &mons[PM_GREEN_SLIME],
                                (gv.vis && canseemon(magr)) ? NC_SHOW_MSG
                                                            : NO_NC_FLAGS))
                        magr->mstrategy &= ~STRAT_WAITFORU;
                }
                /* the cure attempt could have been fatal, potentially
                   to both monsters (scroll of fire) */
                if (DEADMONSTER(magr))
                    return (mdead | mhit | M_ATTK_AGR_DIED);
                if (DEADMONSTER(mdef))
                    return (mdead | mhit | M_ATTK_DEF_DIED);
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
                        if (has_free_action(magr))
                            pline("%s stiffens momentarily.", buf);
                        else
                            pline("%s is frozen by %s gaze!", buf,
                                  s_suffix(mon_nam(mdef)));
                    }
                    if (has_free_action(magr))
                        return (mdead | mhit);
                    paralyze_monst(magr, tmp);
                    return (mdead | mhit);
                }
            } else { /* gelatinous cube */
                Strcpy(buf, Monnam(magr));
                if (canseemon(magr)) {
                    if (has_free_action(magr))
                        pline("%s stiffens momentarily.", buf);
                    else
                        pline("%s is frozen by %s.", buf, mon_nam(mdef));
                }
                if (has_free_action(magr))
                    return (mdead | mhit);
                paralyze_monst(magr, tmp);
                return (mdead | mhit);
            }
            return 1;
        case AD_COLD:
            if (resists_cold(magr) || defended(magr, AD_COLD)) {
                if (canseemon(magr)) {
                    pline_mon(magr, "%s is mildly chilly.", Monnam(magr));
                    golemeffects(magr, AD_COLD, tmp);
                }
                tmp = 0;
                break;
            }
            if (canseemon(magr))
                pline_mon(magr, "%s is suddenly very cold!", Monnam(magr));
            healmon(mdef, tmp/2, tmp/2);
            if (mdef->mhpmax > ((int) (mdef->m_lev + 1) * 8))
                (void) split_mon(mdef, magr);
            break;
        case AD_STUN:
            if (mon_resists_stun(magr) || defended(magr, AD_STUN)
                || mon_wielding_artifact(magr, ART_TEMPEST))
                break; /* immune to stun; passive damage still applies */
            if (!magr->mstun) {
                magr->mstun = 1;
                if (canseemon(magr))
                    pline_mon(magr, "%s %s...", Monnam(magr),
                          makeplural(stagger(magr->data, "stagger")));
            }
            tmp = 0;
            break;
        case AD_FIRE:
            if (resists_fire(magr) || defended(magr, AD_FIRE)
                || mon_underwater(magr)) {
                if (canseemon(magr)) {
                    pline_mon(magr, "%s is mildly warmed.", Monnam(magr));
                    golemeffects(magr, AD_FIRE, tmp);
                }
                tmp = 0;
                break;
            }
            if (canseemon(magr))
                pline_mon(magr, "%s is suddenly very hot!", Monnam(magr));
            break;
        case AD_ELEC:
            if (resists_elec(magr) || defended(magr, AD_ELEC)) {
                if (canseemon(magr)) {
                    pline_mon(magr, "%s is mildly tingled.", Monnam(magr));
                    golemeffects(magr, AD_ELEC, tmp);
                }
                tmp = 0;
                break;
            }
            if (canseemon(magr))
                pline_mon(magr, "%s is jolted with electricity!",
                          Monnam(magr));
            break;
        case AD_DISE:
            if (resists_sick(magr) || defended(magr, AD_DISE)) {
                if (canseemon(magr))
                    pline_mon(magr, "%s resists infection.", Monnam(magr));
                tmp = 0;
                break;
            }
            if (magr->mdiseasetime)
                magr->mdiseasetime -= rnd(3);
            else
                magr->mdiseasetime = rn1(9, 6);
            if (canseemon(magr))
                pline_mon(magr, "%s looks %s.", Monnam(magr),
                          magr->mdiseased ? "even worse" : "diseased");
            magr->mdiseased = 1;
            magr->mdiseabyu = FALSE;
            break;
        case AD_SLOW: /* orange dragon */
            if (mhitb && !mdef->mcan && magr->mspeed != MSLOW && !rn2(3))
                mon_adjust_speed(magr, -1, (struct obj *) 0);
            tmp = 0;
            break;
        case AD_SLEE: /* black fungus */
            if (sleep_monst(magr, rn2(3) + 8, -1)) {
                if (canseemon(magr))
                    pline_mon(magr, "%s loses consciousness.", Monnam(magr));
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
    if ((magr->mhp -= tmp) <= 0) {
        gz.zombify = FALSE; /* attacker's corpse never zombifies */
        if (mdef->mtame)
            set_pet_killer(mdef);
        if (mdef->uexp)
            /* hero-conjured helper's passive counterattack; the hero
               gets credit and experience */
            mon_xkilled(magr, "", (int) mdattk[i].adtyp);
        else
            monkilled(magr, "", (int) mdattk[i].adtyp);
        return (mdead | mhit | M_ATTK_AGR_DIED);
    }
    return (mdead | mhit);
}

/* hero or monster has successfully hit target mon with drain energy attack */
void
xdrainenergym(struct monst *mon, boolean givemsg)
{
    if (mon->mspec_used < 20 /* limit draining */
        && (attacktype(mon->data, AT_MAGC)
            || attacktype(mon->data, AT_BREA))) {
        mon->mspec_used += d(2, 2);
        if (givemsg)
            pline_mon(mon, "%s seems lethargic.", Monnam(mon));
    }
}

/* "aggressive defense"; what type of armor prevents specified attack
   from touching its target? */
long
attk_protection(int aatyp)
{
    long w_mask = 0L;

    switch (aatyp) {
    case AT_NONE:
    case AT_SPIT:
    case AT_EXPL:
    case AT_BOOM:
    case AT_GAZE:
    case AT_BREA:
    case AT_SCRE:
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
