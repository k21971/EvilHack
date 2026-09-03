/* NetHack 5.0	allmain.c	$NHDT-Date: 1781973040 2026/06/20 16:30:40 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.304 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2012. */
/* NetHack may be freely redistributed.  See license for details. */

/* various code that was replicated in *main.c */

#include "hack.h"

#ifndef NO_SIGNAL
#include <signal.h>
#endif

staticfn void moveloop_preamble(boolean);
staticfn void u_calc_moveamt(int);
staticfn void maybe_generate_rnd_mon(void);
staticfn void maybe_do_tutorial(void);
#ifdef POSITIONBAR
staticfn void do_positionbar(void);
#endif
staticfn void init_mchest(void);
staticfn void regen_pw(int);
staticfn void regen_hp(int);
staticfn void interrupt_multi(const char *);

#ifdef EXTRAINFO_FN
static long prev_dgl_extrainfo = 0;
#endif

/* cached racial regen-block state for the moveloop transition
   messages; initialized in moveloop_preamble() */
static boolean mvl_elf_regen, mvl_orc_regen;

/*ARGSUSED*/
void
early_init(int argc, char *argv[])
{
    program_state_init();
#ifdef CRASHREPORT
    /* Do this as early as possible, but let ports do other things first. */
    crashreport_init(argc, argv);
#endif
    decl_globals_init();
    objects_globals_init();
    monst_globals_init();
    sys_early_init();
    runtime_info_init();
    nhUse(argc);
    nhUse(argv[0]);
}

staticfn void
moveloop_preamble(boolean resuming)
{
    /* if a save file created in normal mode is now being restored in
       explore mode, treat it as normal restore followed by 'X' command
       to use up the save file and require confirmation for explore mode */
    if (resuming && iflags.deferred_X)
        (void) enter_explore_mode();

    /* side-effects from the real world */
    flags.moonphase = phase_of_the_moon();
    if (flags.moonphase == FULL_MOON) {
        You("are lucky!  Full moon tonight.");
        change_luck(1);
    } else if (flags.moonphase == NEW_MOON) {
        pline("Be careful!  New moon tonight.");
    }
    flags.friday13 = friday_13th();
    if (flags.friday13) {
        pline("Watch out!  Bad things can happen on Friday the 13th.");
        change_luck(-1);
    }
    if (halloween()) {
        pline("Beware the Undead, for they roam the Dungeons of Doom"
              " on All Hallows' Eve!");
    }

    if (!resuming) { /* new game */
        program_state.beyond_savefile_load = 1; /* for TTY_PERM_INVENT */
        svc.context.rndencode = rnd(9000);
        set_wear((struct obj *) 0); /* for side-effects of starting gear */
        reset_justpicked(gi.invent);
        (void) pickup(1);      /* autopickup at initial location */
        /* only matters if someday a character is able to start with
           clairvoyance (wizard with cornuthaum perhaps?); without this,
           first "random" occurrence would always kick in on turn 1 */
        svc.context.seer_turn = (long) rnd(30);
        /* don't make it obvious when monsters will start speeding up;
           saved with the rest of context so the curve survives restore
           instead of re-rolling each session */
        svc.context.timeout_start = (long) rnd(10000) + 25000L;
        /* give hero initial movement points; new game only--for restore,
           pending movement points were included in the save file */
        u.umovement = NORMAL_SPEED;
        init_mchest();
        initrack();
    }
    disp.botlx = TRUE; /* for STATUS_HILITES */
    if (resuming) { /* restoring old game */
        read_engr_at(u.ux, u.uy); /* subset of pickup() */
        fix_shop_damage();
    }

    encumber_msg(); /* in case they auto-picked up something */
    if (gd.defer_see_monsters) {
        gd.defer_see_monsters = FALSE;
        see_monsters();
    }

    u.uz0.dlevel = u.uz.dlevel;
    svc.context.move = 0;

#ifdef WHEREIS_FILE
    touch_whereis();
#endif

    /* finish processing "--debug:fuzzer" from the command line */
    if (iflags.fuzzerpending) {
        iflags.debug_fuzzer = fuzzer_impossible_panic;
        iflags.fuzzerpending = FALSE;
    }

    /* cache racial regen-block state so the moveloop can report
       transitions */
    mvl_elf_regen = elf_can_regen();
    mvl_orc_regen = orc_can_regen();

    program_state.in_moveloop = 1;
    /* for perm_invent preset at startup, display persistent inventory after
       invent is fully populated and the in_moveloop flag has been set */
    if (iflags.perm_invent)
        update_inventory();
}

staticfn void
u_calc_moveamt(int wtcap)
{
    int moveamt = 0;

    /* calculate how much time passed. */
    if (u.usteed && u.umoved) {
        /* your speed doesn't augment steed's speed */
        moveamt = mcalcmove(u.usteed, TRUE);
    } else {
        moveamt = gy.youmonst.data->mmove;

        if (Very_fast) { /* speed boots, potion, or spell */
            /* gain a free action on 2/3 of turns */
            if (rn2(3) != 0)
                moveamt += NORMAL_SPEED;
        } else if (Fast) { /* intrinsic */
            /* gain a free action on 1/3 of turns */
            if (rn2(3) == 0)
                moveamt += NORMAL_SPEED;
        } else if (Slow) {
            /* average movement noticeably slower */
            if (rn2(3) != 0)
                moveamt -= NORMAL_SPEED / 2;
        }
    }

    switch (wtcap) {
    case UNENCUMBERED:
        break;
    case SLT_ENCUMBER:
        moveamt -= (moveamt / 4);
        break;
    case MOD_ENCUMBER:
        moveamt -= (moveamt / 2);
        break;
    case HVY_ENCUMBER:
        moveamt -= ((moveamt * 3) / 4);
        break;
    case EXT_ENCUMBER:
        moveamt -= ((moveamt * 7) / 8);
        break;
    default:
        break;
    }

    u.umovement += moveamt;
    if (u.umovement < 0)
        u.umovement = 0;
}

/* the rn2() divisor for this turn's random spawn roll, from
   MIN_MONGEN_RATE (slowest) down to MAX_MONGEN_RATE (fastest);
   exported so enlightenment can display the effective rate */
int
current_monclock(void)
{
    int monclock = MIN_MONGEN_RATE;
    long past_clock;

    if (u.uevent.invoked) {
        /* performing the invocation gets the entire dungeon riled up */
        monclock = MAX_MONGEN_RATE;
    } else {
        /* spawn rate slowly climbs after context.timeout_start turns */
        past_clock = svm.moves - svc.context.timeout_start;
        if (past_clock > 0)
            monclock = (int) ((MIN_MONGEN_RATE * 30000L)
                              / (past_clock + 30000L));
        /* various events will double the normal spawn rate; each of
           these is sticky so the floor never regresses */
        if (monclock > (MIN_MONGEN_RATE / 2)) {
            if (u.uevent.gehennom_entered) /* entering gehennom */
                monclock = (MIN_MONGEN_RATE / 2);
            else if (u.uevent.uhand_of_elbereth) /* crowned */
                monclock = (MIN_MONGEN_RATE / 2);
            else if (svq.quest_status.got_quest_early) /* early quest */
                monclock = (MIN_MONGEN_RATE / 2);
        }
        /* entering the first tier demon boss level */
        if (monclock > (MIN_MONGEN_RATE / 3) && u.uevent.hella_entered)
            monclock = (MIN_MONGEN_RATE / 3);
        /* entering the third tier demon boss level */
        if (monclock > (MIN_MONGEN_RATE / 4) && u.uevent.hellc_entered)
            monclock = (MIN_MONGEN_RATE / 4);
        /* killing the Wizard of Yendor for the first time */
        if (monclock > (MIN_MONGEN_RATE / 6) && u.uevent.udemigod)
            monclock = (MIN_MONGEN_RATE / 6);
    }
    /* make sure we don't fall off either end */
    if (monclock < MAX_MONGEN_RATE)
        monclock = MAX_MONGEN_RATE;
    if (monclock > MIN_MONGEN_RATE)
        monclock = MIN_MONGEN_RATE;
    return monclock;
}

/* From SporkHack and UnNetHack (modified).
 * Vanilla generates a critter every 70-ish turns. The rate accelerates
 * to every 50 or so below the Castle, and 'round every 25 turns once
 * you've done the Invocation.
 *
 * We will push it even further. Monsters post-Invocation will almost
 * always appear on the stairs (if present), and much more frequently;
 * this should certainly make it seem like you're wading back through
 * the teeming hordes.
 *
 * Aside from that, a more general clock should be put on things; after
 * about 30,000 turns, the frequency rate of appearance and difficulty
 * of monsters generated will slowly increase until it reaches the
 * point it will be at as if you were post-Invocation.
 *
 * The rate increases linearly with turns. The rule of thumb is that at
 * turn x the rate is approximately (x / 30000) times the normal rate.
 * Maximal rate is 8x the normal rate.
 */
staticfn void
maybe_generate_rnd_mon(void)
{
    if (!rn2(current_monclock())) {
        stairway *stway;

        if (u.uevent.invoked
            && (stway = stairway_find_type_dir(FALSE, TRUE)) != 0
            && rn2(10))
            (void) makemon((struct permonst *) 0, stway->sx, stway->sy,
                           MM_ADJACENTOK | MM_MPLAYEROK);
        else if (u.uevent.invoked
                 && ((stway = stairway_find_special_dir(FALSE)) != 0
                     || (stway = stairway_find_special_dir(TRUE)) != 0)
                 && rn2(10))
            (void) makemon((struct permonst *) 0, stway->sx, stway->sy,
                           MM_ADJACENTOK | MM_MPLAYEROK);
        else
            (void) makemon((struct permonst *) 0, 0, 0, MM_MPLAYEROK);
    }
}

#if defined(MICRO) || defined(WIN32)
static int mvl_abort_lev;
#endif
static int mvl_wtcap = 0;
static int mvl_change = 0;

void
moveloop_core(void)
{
    boolean monscanmove = FALSE;

#ifdef SAFERHANGUP
    if (program_state.done_hup)
        end_of_input();
#endif
    get_nh_event();
#ifdef POSITIONBAR
    do_positionbar();
#endif
    if (iflags.pending_customizations)
        maybe_shuffle_customizations();

    dobjsfree();

    if (svc.context.bypasses)
        clear_bypasses();

    if (iflags.sanity_check || iflags.debug_fuzzer)
        sanity_check();

    if (svc.context.resume_wish)
        makewish(); /* clears resume_wish */

    if (svc.context.move) {
        /* actual time passed */
        u.umovement -= NORMAL_SPEED;

        do { /* hero can't move this turn loop */
            encumber_msg();

            svc.context.mon_moving = TRUE;
            do {
                monscanmove = movemon();
                if (u.umovement >= NORMAL_SPEED)
                    break; /* it's now your turn */
            } while (monscanmove);
            svc.context.mon_moving = FALSE;

            /* this needs to be after the monster movement loop in
               case monster actions affected burden, e.g. rehumanize */
            mvl_wtcap = near_capacity();

            if (!monscanmove && u.umovement < NORMAL_SPEED) {
                /* both hero and monsters are out of steam this round */
                struct monst *mtmp;

                /* set up for a new turn */
                gw.were_changes = 0L;
                mcalcdistress(); /* adjust monsters' trap, blind, etc */

                /* reallocate movement rations to monsters; don't need
                   to skip dead monsters here because they will have
                   been purged at end of their previous round of moving */
                for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
                    mtmp->movement += mcalcmove(mtmp, TRUE);

                /* occasionally add another monster; since this takes
                   place after movement has been allotted, the new
                   monster effectively loses its first turn */
                maybe_generate_rnd_mon();

                u_calc_moveamt(mvl_wtcap);
                settrack();

                svm.moves++;
                /*
                 * Never allow 'moves' to grow big enough to wrap.
                 * We don't care what the maximum possible 'long int'
                 * is for the current configuration, we want a value
                 * that is the same for all viable configurations.
                 * When imposing the limit, use a mystic decimal value
                 * instead of a magic binary one such as 0x7fffffffL.
                 */
                if (svm.moves >= 1000000000L) {
                    display_nhwindow(WIN_MESSAGE, TRUE);
                    urgent_pline("The dungeon capitulates.");
                    done(ESCAPED);
                }
                /* 'moves' is misnamed; it represents turns; hero_seq is
                   a value that is distinct every time the hero moves */
                gh.hero_seq = svm.moves << 3;

                if (flags.time && !svc.context.run)
                    disp.time_botl = TRUE; /* 'moves' just changed */

                /********************************/
                /* once-per-turn things go here */
                /********************************/

                l_nhcore_call(NHCORE_MOVELOOP_TURN);

                /* if you have too many pets on the level, untame the
                   weakest ones */
                {
                    int numdogs = 0;
                    int num_candidates = 0;
                    int limit = ACURR(A_CHA) / 3;
                    /* untameable pets (excludes steed); cap:
                       overflow is self-correcting across turns */
                    struct monst *candidates[50];
                    struct monst *curmon;

                    /* single pass: count all pets, collect
                       untameable candidates */
                    for (curmon = fmon; curmon; curmon = curmon->nmon) {
                        /* skip minions: they carry emin rather than edog,
                           so EDOG() below would NULL-deref */
                        if (curmon->mtame && !curmon->msummoned
                            && !curmon->isminion) {
                            numdogs++;
                            /* steed counts toward limit but is
                               never untamed */
                            if (curmon != u.usteed && num_candidates < 50)
                                candidates[num_candidates++] = curmon;
                        }
                    }

                    /* untame weakest pets until under limit */
                    while (numdogs > limit && num_candidates > 0) {
                        int i, weakest_idx = 0;
                        int numties = 1;
                        struct monst *weakdog;

                        /* find weakest candidate (lowest m_lev,
                           random tie-break) */
                        for (i = 1; i < num_candidates; i++) {
                            if (candidates[i]->m_lev
                                < candidates[weakest_idx]->m_lev) {
                                weakest_idx = i;
                                numties = 1;
                            } else if (candidates[i]->m_lev
                                       == candidates[weakest_idx]->m_lev) {
                                if (!rn2(++numties))
                                    weakest_idx = i;
                            }
                        }

                        weakdog = candidates[weakest_idx];

                        /* remove from candidates (swap with last element) */
                        candidates[weakest_idx] = candidates[--num_candidates];

                        /* untame the pet */
                        weakdog->mtame = 0;
                        weakdog->uexp = 0;
                        /* if former pet was abused, or just
                           at random, become hostile */
                        if (rn2(EDOG(weakdog)->abuse + 1) || !rn2(3)) {
                            weakdog->mpeaceful = 0;
                            set_malign(weakdog);
                            newsym(weakdog->mx, weakdog->my);
                        }
                        if (weakdog->mleashed)
                            m_unleash(weakdog, TRUE);

                        numdogs--;
                    }
                }

                if (Glib)
                    glibr();
                nh_timeout();
                run_regions();

                /* sewage slow effect: purely position-based, must be
                   checked every turn even when standing still */
                if (slowed_by_sewage()) {
                    if (!(HSlow & FROMOUTSIDE)) {
                        HSlow |= FROMOUTSIDE;
                        disp.botl = TRUE;
                    }
                } else if (HSlow & FROMOUTSIDE) {
                    HSlow &= ~FROMOUTSIDE;
                    disp.botl = TRUE;
                }

                if (u.ublesscnt)
                    u.ublesscnt--;

#ifdef EXTRAINFO_FN
                if ((prev_dgl_extrainfo == 0) || (prev_dgl_extrainfo < (svm.moves + 250))) {
                    prev_dgl_extrainfo = svm.moves;
                    mk_dgl_extrainfo();
                }
#endif

                /* One possible result of prayer is healing.  Whether or
                 * not you get healed depends on your current hit points.
                 * If you are allowed to regenerate during the prayer,
                 * the end-of-prayer calculation messes up on this.
                 * Another possible result is rehumanization, which
                 * requires that encumbrance and movement rate be
                 * recalculated.
                 */
                if (u.uinvulnerable) {
                    /* for the moment at least, you're in tiptop shape */
                    mvl_wtcap = UNENCUMBERED;
                } else if (!Upolyd ? (u.uhp < u.uhpmax)
                           : (u.mh < u.mhmax
                              || gy.youmonst.data->mlet == S_EEL)) {
                    /* maybe heal */
                    regen_hp(mvl_wtcap);
                }

                /* wither away a bit */
                if (Withering && !u.uinvulnerable) {
                    int loss = rnd(2) - (Regeneration ? 1 : 0);

                    if (loss >= (Upolyd ? u.mh : u.uhp))
                        You("wither away completely!");
                    losehp(loss, "withered away", NO_KILLER_PREFIX);
                    disp.botl = TRUE;
                    interrupt_multi("You are slowly withering away.");
                }

                /* moving around while encumbered is hard work */
                if (mvl_wtcap > MOD_ENCUMBER && u.umoved) {
                    if (!(mvl_wtcap < EXT_ENCUMBER ? svm.moves % 30
                          : svm.moves % 10)) {
                        overexert_hp();
                    }
                }

                regen_pw(mvl_wtcap);

                /* Moloch demands regular sacrifices! */
                if (svc.context.next_moloch_offering <= svm.moves) {
                    if (svc.context.next_moloch_offering == svm.moves
                        && (u.ualign.type == A_NONE
                            || u.ualignbase[A_CURRENT] == A_NONE)) {
                        You_feel("%s urge to perform a sacrifice.",
                                 u.ualign.type == A_NONE ? "an"
                                                         : "a faint");
                        stop_occupation();
                    }
                    if (u.ualign.type == A_NONE
                        && rn2((int) (svm.moves
                                      - svc.context.next_moloch_offering
                                      + 1000L)) >= 1000) {
                        if (!rn2(100) && u.ualign.record > -99) {
                            adjalign(-1);
                            record_abuse_event(-1, ABUSE_NEGLECT_OFFERING);
                            /* give our infidel some feedback every
                               once in a while */
                            if (!rn2(5))
                                You_feel(
                                 "your favor with %s starting to slip.",
                                         u_gname());
                        }
                        if (u.ualign.record < -10 && !rn2(u.ugangr + 1)
                            && rn2(-u.ualign.record + 90) >= 100) {
                            const char *angry = (char *) 0;

                            u.ugangr++;
                            /* avoid repetitive messages */
                            switch (u.ugangr) {
                            /* same values as in insight.c */
                            case 1:
                                angry = "";
                                break;
                            case 4:
                                angry = "very ";
                                break;
                            case 7:
                                angry = "extremely ";
                                break;
                            }
                            if (angry) {
                                You_feel("that %s is %sangry at your "
                                         "lack of offerings.", u_gname(),
                                         angry);
                                stop_occupation();
                            }
                        }
                    }
                }

                if (!u.uinvulnerable) {
                    if (Teleportation && !rn2(85)) {
                        coordxy old_ux = u.ux, old_uy = u.uy;

                        tele();
                        if (u.ux != old_ux || u.uy != old_uy) {
                            if (!next_to_u()) {
                                check_leash(old_ux, old_uy);
                            }
                            /* clear doagain keystrokes */
                            cmdq_clear(CQ_CANNED);
                            cmdq_clear(CQ_REPEAT);
                        }
                    }
                    /* delayed change may not be valid anymore */
                    if ((mvl_change == 1 && !Polymorph)
                        || (mvl_change == 2 && u.ulycn == NON_PM))
                        mvl_change = 0;
                    if (Polymorph && !rn2(100))
                        mvl_change = 1;
                    else if (ismnum(u.ulycn) && !Upolyd
                             && !rn2(80 - (20 * night())))
                        mvl_change = 2;
                    if (mvl_change && !Unchanging) {
                        if (gm.multi >= 0) {
                            stop_occupation();
                            if (mvl_change == 1)
                                polyself(POLY_NOFLAGS);
                            else
                                you_were();
                            mvl_change = 0;
                        }
                    }
                }

                if (Searching && !svl.level.flags.noautosearch
                    && gm.multi >= 0)
                    (void) dosearch0(1);
                if (Warning)
                    warnreveal();
                if (gw.were_changes) {
                    /* update innate intrinsics (mainly Drain_resistance) */
                    set_uasmon();
                }
                mkot_trap_warn();
                dosounds();
                do_storms();
                gethungry();
                age_spells();
                /* decay monster spell knowledge every 10 turns */
                if (!(svm.moves % 10)) {
                    struct monst *mon;

                    for (mon = fmon; mon; mon = mon->nmon) {
                        if (!DEADMONSTER(mon) && has_emsp(mon))
                            mage_spells(mon);
                    }
                }
                exerchk();
                invault();
                if (u.uhave.amulet)
                    amulet();
                if (!rn2(40 + (int) (ACURR(A_DEX) * 3)))
                    u_wipe_engr(rnd(3));
                if (u.uevent.udemigod && !u.uinvulnerable) {
                    if (u.udg_cnt)
                        u.udg_cnt--;
                    if (!u.udg_cnt) {
                        intervene();
                        u.udg_cnt = rn1(100, 50);
                    }
                }
/* XXX This should be recoded to use something like regions - a list of
 * things that are active and need to be handled that is dynamically
 * maintained and not a list of special cases. */
                /* vision will be updated as bubbles move */
                if (Is_waterlevel(&u.uz) || Is_airlevel(&u.uz))
                    movebubbles();
                else if (svl.level.flags.fumaroles)
                    fumaroles();

                /* when immobile, count is in turns */
                if (gm.multi < 0) {
                    runmode_delay_output();
                    if (++gm.multi == 0) { /* finished yet? */
                        unmul((char *) 0);
                        /* if unmul caused a level change, take it now */
                        if (u.utotype)
                            deferred_goto();
                    }
                }

                /* Running around in Gehennom without
                   100% fire resistance */
                if (Inhell && !Is_valley(&u.uz))
                    in_hell_effects();

                /* Running around in the Ice Queen branch without
                   100% cold resistance */
                if (Iniceq)
                    in_iceq_effects();

                /* make monsters that innately grudge the hero's race,
                   role, or alignment hostile should they be tame or
                   peaceful */
                /* TODO-EVILHACK: master also runs this pass for the
                   wielded or worn 'banes; those arms port with the
                   artifacts slice */
                for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
                    boolean is_shkp = has_eshk(mtmp)
                                      && inhishop(mtmp);
                    boolean is_prst = has_epri(mtmp)
                                      && inhistemple(mtmp);
                    boolean same_align = (sgn(mon_aligntyp(mtmp))
                                          == u.ualign.type);

                    /* monster in question has to see you, not just
                       sense you, before it becomes hostile */
                    if (!(mtmp->mcansee && m_canseeu(mtmp)))
                        continue;
                    /* mindless creatures don't register the threat */
                    if (mindless(mtmp->data))
                        continue;
                    /* coaligned temple priests will stay civil */
                    if (is_prst && p_coaligned(mtmp))
                        continue;

                    if ((Role_if(PM_KNIGHT) && !same_align
                         && is_dragon(mtmp->data))
                        || (Race_if(PM_AASIMAR) && is_demon(mtmp->data))
                        || (Role_if(PM_INFIDEL) && is_angel(mtmp->data)
                            && mon_aligntyp(mtmp) > 0)) {
                        if (is_shkp) {
                            /* shopkeepers ban you from their shop */
                            ESHK(mtmp)->pbanned = TRUE;
                        } else if (mtmp->mpeaceful || mtmp->mtame) {
                            setmangry(mtmp, FALSE);
                            mtmp->mpeaceful = mtmp->mtame = 0;
                            /* setmangry early-returns if mtame was
                               set, so ensure malign resyncs */
                            set_malign(mtmp);
                            newsym(mtmp->mx, mtmp->my);
                            if (mtmp->mleashed)
                                m_unleash(mtmp, TRUE);
                            if (u.usteed && mtmp == u.usteed)
                                dismount_steed(DISMOUNT_THROWN);
                        }
                    }
                }

                /* Draugr corpse sense: update perm_invent when corpse
                   rot status changes */
                if (iflags.perm_invent && !Upolyd && Race_if(PM_DRAUGR)) {
                    struct obj *otmp;

                    for (otmp = gi.invent; otmp; otmp = otmp->nobj) {
                        if (otmp->otyp == CORPSE) {
                            update_inventory();
                            break;
                        }
                    }
                }
            }
        } while (u.umovement < NORMAL_SPEED); /* hero can't move */

        /******************************************/
        /* once-per-hero-took-time things go here */
        /******************************************/

        gh.hero_seq++; /* moves*8 + n for n == 1..7 */

        /* although we checked for encumbrance above, we need to
           check again for message purposes, as the weight of
           inventory may have changed in, e.g., nh_timeout(); we do
           need two checks here so that the player gets feedback
           immediately if their own action encumbered them */
        encumber_msg();

#ifdef STATUS_HILITES
        if (iflags.hilite_delta)
            status_eval_next_unhilite();
#endif
        if (svm.moves >= svc.context.seer_turn) {
            if ((u.uhave.amulet || Clairvoyant) && !In_endgame(&u.uz)
                && !BClairvoyant)
                do_vicinity_map((struct obj *) 0);
            /* we maintain this counter even when clairvoyance isn't
               taking place; on average, go again 30 turns from now */
            svc.context.seer_turn = svm.moves + (long) rn1(31, 15); /*15..45*/
            /* [it used to be that on every 15th turn, there was a 50%
               chance of farsight, so it could happen as often as every
               15 turns or theoretically never happen at all; but when
               a fast hero got multiple moves on that 15th turn, it
               could actually happen more than once on the same turn!] */
        }
        /* [fast hero who gets multiple moves per turn ends up sinking
           multiple times per turn; is that what we really want?] */
        if (u.utrap && u.utraptype == TT_LAVA)
            sink_into_lava();
        /* when/if hero escapes from lava, he can't just stay there */
        else if (!u.umoved)
            (void) pooleffects(FALSE);
        /* entanglement wears off passively over time */
        if (u.utrap && u.utraptype == TT_ENTANGLED) {
            if (--u.utrap == 0) {
                if (nexttotree(u.ux, u.uy))
                    pline_The("tree's %s release their grip on you.",
                              rn2(2) ? "branches" : "roots");
                else
                    pline_The("vegetation releases you.");
                reset_utrap(TRUE);
            }
        }
        svc.context.coward = FALSE;

        /* vision while buried or underwater is updated here */
        if (Underwater && !See_underwater)
            under_water(0);
        else if (Underwater && !Is_waterlevel(&u.uz))
            docrt();
        else if (u.uburied)
            under_ground(0);

        see_nearby_monsters();
    } /* actual time passed */

    /****************************************/
    /* once-per-player-input things go here */
    /****************************************/

    clear_splitobjs();

    find_ac();
    if (!svc.context.mv || Blind) {
        /* redo monsters if hallu or wearing a helm of telepathy */
        if (Hallucination) { /* update screen randomly */
            see_monsters();
            see_objects();
            see_traps();
            if (u.uswallow)
                swallowed(0);
        } else if (Unblind_telepat || Warning || Warn_of_mon
                   /* this is needed for the case where you saw a monster
                      due to being next to it while it's in a gas cloud
                      and then you moved away; it should no longer be seen
                      when that happens, even if it hasn't moved */
                   || any_visible_region()) { /* TODO: optimize this */
            see_monsters();
        }
        if (gv.vision_full_recalc)
            vision_recalc(0); /* vision! */
    }
    if (disp.botl || disp.botlx) {
        bot();
        curs_on_u();
    } else if (disp.time_botl) {
        timebot();
        curs_on_u();
    }

    {
        boolean can_elf_now = elf_can_regen();
        boolean can_orc_now = orc_can_regen();

        if (mvl_elf_regen != can_elf_now) {
            if (!Hallucination)
                You_feel("%s.", mvl_elf_regen ? "itchy" : "relief");
            else
                You_feel("%s.", can_elf_now ? "magnetic"
                                            : "like... gnarly dude");
            mvl_elf_regen = can_elf_now;
        }

        if (mvl_orc_regen != can_orc_now) {
            if (!Hallucination)
                You_feel("%s.", mvl_orc_regen ? "tingly" : "relief");
            else
                You_feel("%s.", can_orc_now ? "non-magnetic"
                                            : "like... whoa");
            mvl_orc_regen = can_orc_now;
        }
    }

    m_everyturn_effect(&gy.youmonst);

    svc.context.move = 1;

    if (gm.multi >= 0 && go.occupation) {
#if defined(MICRO) || defined(WIN32CON)
        mvl_abort_lev = 0;
        if (kbhit()) {
            char ch;

            if ((ch = pgetchar()) == ABORT)
                mvl_abort_lev++;
            else
                cmdq_add_key(CQ_CANNED, ch);
        }
        if (!mvl_abort_lev && (*go.occupation)() == 0)
#else
            if ((*go.occupation)() == 0)
#endif
                go.occupation = 0;
        if (
#if defined(MICRO) || defined(WIN32)
            mvl_abort_lev ||
#endif
            monster_nearby()) {
            stop_occupation();
            reset_eat();
        }
        runmode_delay_output();
        return;
    }

    u.umoved = FALSE;

    if (gm.multi > 0) {
        lookaround();
        runmode_delay_output();
        if (!gm.multi) {
            /* lookaround may clear multi */
            svc.context.move = 0;
            return;
        }
        if (svc.context.mv) {
            if (gm.multi < COLNO && !--gm.multi)
                end_running(TRUE);
            domove();
        } else {
            --gm.multi;
            nhassert(gc.command_count != 0);
            rhack(gc.cmd_key);
        }
    } else if (gm.multi == 0) {
#ifdef MAIL
        ckmailstatus();
#endif
        rhack(0);
    }
    if (u.utotype)       /* change dungeon level */
        deferred_goto(); /* after rhack() */

    if (gv.vision_full_recalc)
        vision_recalc(0); /* vision! */
#ifdef CLIPPING
    /* after rhack() and vision_recalc() so that the map is redrawn
       once with correct vision data, not twice (overshoot+correct) */
    cliparound(u.ux, u.uy);
#endif
    /* when running in non-tport mode, this gets done through domove() */
    if ((!svc.context.run || flags.runmode == RUN_TPORT)
        && (gm.multi && (!svc.context.travel ? !(gm.multi % 7)
                        : !(svm.moves % 7L)))) {
        if (flags.time && svc.context.run)
            disp.botl = TRUE;
        /* [should this be flush_screen() instead?] */
        display_nhwindow(WIN_MAP, FALSE);
    }

    if (gl.luacore && nhcb_counts[NHCB_END_TURN]) {
        lua_getglobal(gl.luacore, "nh_callback_run");
        lua_pushstring(gl.luacore, nhcb_name[NHCB_END_TURN]);
        nhl_pcall_handle(gl.luacore, 1, 0, "moveloop_core", NHLpa_panic);
        lua_settop(gl.luacore, 0);
    }
}

staticfn void
maybe_do_tutorial(void)
{
    s_level *sp = find_level("tut-1");

    if (!sp)
        return;

    if (ask_do_tutorial()) {
        assign_level(&u.ucamefrom, &u.uz);
        iflags.nofollowers = TRUE;
        schedule_goto(&sp->dlevel, UTOTYPE_NONE,
                      "Entering the tutorial.", (char *) 0);
        deferred_goto();
        vision_recalc(0);
        docrt();
        iflags.nofollowers = FALSE;
    } else {
        /* no tutorial, so okay to process mention_decor now */
        rcfile_only_this_option(opt_mention_decor);
    }
}

void
moveloop(boolean resuming)
{
    moveloop_preamble(resuming);

    if (!resuming)
        maybe_do_tutorial();

    /* process one deferred option post-tutorial */
    rcfile_only_this_option(opt_mention_decor);

    for (;;) {
        moveloop_core();
    }
}

/* create the game-global magic chest object; every magic chest terrain
   tile opens onto this one shared contents chain */
staticfn void
init_mchest(void)
{
    gm.mchest = mksobj(HIDDEN_CHEST, FALSE, FALSE);
    gm.mchest->olocked = 1;
    gm.mchest->where = OBJ_MCHEST;
}

/* elves and drow cannot regenerate hit points while in direct
   contact with cold iron; layered clothing or gloves shield the
   skin, as do quest artifacts and bark- or stone-like skin */
boolean
elf_can_regen(void)
{
    if (Barkskin || Stoneskin)
        return TRUE; /* protects skin from adverse material effects */

    if (maybe_polyd(is_elf(gy.youmonst.data), Race_if(PM_ELF))
        || maybe_polyd(is_drow(gy.youmonst.data), Race_if(PM_DROW))) {
        if (uwep && is_iron(uwep)
            && !is_quest_artifact(uwep) && !uarmg)
            return FALSE;
        if (uarm && is_iron(uarm) && !uarmu)
            return FALSE;
        if (uarmu && is_iron(uarmu))
            return FALSE;
        if (uarmc && is_iron(uarmc)
            && !uarmu && !uarm)
            return FALSE;
        if (uarmh && is_iron(uarmh)
            && !is_quest_artifact(uarmh))
            return FALSE;
        if (uarms && is_iron(uarms) && !uarmg)
            return FALSE;
        if (uarmg && is_iron(uarmg))
            return FALSE;
        if (uarmf && is_iron(uarmf))
            return FALSE;
        if (uleft && is_iron(uleft)
            && !(uarmg && is_art(uarmg, ART_HAND_OF_VECNA)))
            return FALSE;
        if (uright && is_iron(uright))
            return FALSE;
        if (uamul && is_iron(uamul)
            && !is_quest_artifact(uamul)
            && !uarmu && !uarm)
            return FALSE;
        if (ublindf && is_iron(ublindf))
            return FALSE;
        if (uchain && is_iron(uchain))
            return FALSE;
        if (uswapwep && is_iron(uswapwep)
            && u.twoweap && !uarmg)
            return FALSE;
    }
    return TRUE;
}

/* orcs cannot regenerate hit points while in direct contact with
   mithril; same layering rules as elf_can_regen() */
boolean
orc_can_regen(void)
{
    if (Barkskin || Stoneskin)
        return TRUE; /* protects skin from adverse material effects */

    if (maybe_polyd(is_orc(gy.youmonst.data), Race_if(PM_ORC))) {
        if (uwep && is_mithril(uwep)
            && !is_quest_artifact(uwep) && !uarmg)
            return FALSE;
        if (uarm && is_mithril(uarm) && !uarmu)
            return FALSE;
        if (uarmu && is_mithril(uarmu))
            return FALSE;
        if (uarmc && is_mithril(uarmc)
            && !uarmu && !uarm)
            return FALSE;
        if (uarmh && is_mithril(uarmh)
            && !is_quest_artifact(uarmh))
            return FALSE;
        if (uarms && is_mithril(uarms) && !uarmg)
            return FALSE;
        if (uarmg && is_mithril(uarmg))
            return FALSE;
        if (uarmf && is_mithril(uarmf))
            return FALSE;
        if (uleft && is_mithril(uleft)
            && !(uarmg && is_art(uarmg, ART_HAND_OF_VECNA)))
            return FALSE;
        if (uright && is_mithril(uright))
            return FALSE;
        if (uamul && is_mithril(uamul)
            && !is_quest_artifact(uamul)
            && !uarmu && !uarm)
            return FALSE;
        if (ublindf && is_mithril(ublindf))
            return FALSE;
        if (uchain && is_mithril(uchain))
            return FALSE;
        if (uswapwep && is_mithril(uswapwep)
            && u.twoweap && !uarmg)
            return FALSE;
    }
    return TRUE;
}

staticfn void
regen_pw(int wtcap)
{
    /* normal energy regeneration rate */
    long rate = ((MAXULEV + 8L - u.ulevel)
                 * ((Role_if(PM_WIZARD) || Role_if(PM_INFIDEL)) ? 3L : 4L)
                 / 6L);

    /* Druids can have differing rates of energy
       regeneration depending on the amount of
       mistletoe (or lack thereof) in open inventory */
    if (Role_if(PM_DRUID)) {
        struct obj *mtoe = carrying(MISTLETOE);

        if (mtoe && mtoe->quan >= 9) {
            /* double normal rate */
            rate /= 2L;
        } else if (mtoe && mtoe->quan >= 5) {
            /* roughly 1.3x normal rate */
            if (!(svm.moves % 3))
                rate /= 2L;
        /* one to four mistletoe in open inventory
           is normal energy regeneration */
        } else if (!mtoe) {
            /* no mistletoe: half normal rate */
            rate *= 2L;
        }
    }

    if (u.uen < u.uenmax
        && ((wtcap < MOD_ENCUMBER && !(svm.moves % rate))
            || Energy_regeneration
            /* the Idol grants energy regen to piously unaligned;
               it really shouldn't be restricted to Infidels,
               but so far we have no other unaligned roles */
            || (Role_if(PM_INFIDEL) && u.uhave.questart
                && u.ualign.type == A_NONE
                && u.ualign.record > rn2(20)))) {
        int upper = (int) (ACURR(A_WIS) + ACURR(A_INT)) / 15 + 1;

        if (EMagical_breathing)
            upper += 2;

        u.uen += rn1(upper, 1);
        if (u.uen > u.uenmax)
            u.uen = u.uenmax;
        disp.botl = TRUE;
        if (u.uen == u.uenmax)
            interrupt_multi("You feel full of energy.");
    }
}

#define U_CAN_REGEN() (Regeneration || (Sleepy && u.usleep))

/* maybe recover some lost health (or lose some when an eel out of water) */
staticfn void
regen_hp(int wtcap)
{
    int heal = 0;
    boolean reached_full = FALSE,
            encumbrance_ok = (wtcap < MOD_ENCUMBER || !u.umoved),
            infidel_no_amulet = (u.ualign.type == A_NONE
                                 && !u.uhave.amulet
                                 && !u.uevent.uimbued_idol);

    /* periodically let our Infidel know why their hit
       points aren't regenerating if they don't have
       the Amulet in their possession */
    if (infidel_no_amulet && !rn2(20))
        You_feel("unable to rest or heal without the Amulet of Yendor.");

    if (Upolyd) {
        if (u.mh < 1) { /* shouldn't happen... */
            rehumanize();
        } else if (gy.youmonst.data->mlet == S_EEL
                   && !is_damp_terrain(u.ux, u.uy) && !Is_waterlevel(&u.uz)
                   && !Breathless) {
            /* eel out of water loses hp, similar to monster eels;
               as hp gets lower, rate of further loss slows down */
            if (u.mh > 1 && !Regeneration && rn2(u.mh) > rn2(8)
                && (!Half_physical_damage || !(svm.moves % 2L)))
                heal = -1;
        } else if (u.mh < u.mhmax) {
            /* Druids in wildshape form and vampires in shapechange
               form recover noticeably faster than an ordinary
               polymorphed hero; nothing living heals in the Valley
               of the Dead */
            if ((!Is_valley(&u.uz) || is_undead(gy.youmonst.data))
                && !wielding_artifact(ART_WAND_OF_ORCUS)
                && (U_CAN_REGEN()
                    || (encumbrance_ok
                        && !(svm.moves % ((druid_form || vampire_form)
                                          ? 9L : 20L)))))
                heal = 1;
        }
        /* withering blocks healing but not the eel's hp loss */
        if (heal && !(Withering && heal > 0)) {
            disp.botl = TRUE;
            u.mh += heal;
            reached_full = (u.mh == u.mhmax);
        }

    /* !Upolyd */
    } else {
        /* if a Druid's connection with nature is strong (standing on
           grass or beside a live tree), their healing speeds up */
        boolean druid_near_veg = (Role_if(PM_DRUID)
                                  && (levl[u.ux][u.uy].typ == GRASS
                                      || nexttotree(u.ux, u.uy)));

        /* [when this code was in-line within moveloop(), there was
           no !Upolyd check here, so poly'd hero recovered lost u.uhp
           once u.mh reached u.mhmax; that may have been convenient
           for the player, but it didn't make sense for gameplay...]

           Infidels won't heal at all without the Amulet of Yendor
           with them (pre-idol imbuement). No one can regenerate hit
           points while located in the Valley of the Dead, except for
           Draugr/Vampires */
        if (u.uhp < u.uhpmax && elf_can_regen() && orc_can_regen()
            && (encumbrance_ok || U_CAN_REGEN())
            && (!Is_valley(&u.uz)
                || Race_if(PM_DRAUGR) || Race_if(PM_VAMPIRE))
            && !infidel_no_amulet
            && !wielding_artifact(ART_WAND_OF_ORCUS)) {
            /* the near-vegetation multiplier mirrors master's shortened
               moves-modulo interval (3 -> 2), a 1.5x healing rate */
            heal = ((u.ulevel + (int) ACURR(A_CON))
                    * (druid_near_veg ? 3 : 2) / 2) > rn2(100);

            /* tortles gain some accelerated regeneration while
               inside their shell */
            if (Hidinshell && !Regeneration && !heal && !rn2(5))
                heal = 1;

            if (U_CAN_REGEN())
                heal += 1;
            if (Sleepy && u.usleep)
                heal++;

            if (heal && !(Withering && heal > 0)) {
                disp.botl = TRUE;
                u.uhp += heal;
                if (u.uhp > u.uhpmax)
                    u.uhp = u.uhpmax;
                /* stop voluntary multi-turn activity if now fully healed */
                reached_full = (u.uhp == u.uhpmax);
            }
        }
    }

    if (reached_full)
        interrupt_multi("You are in full health.");
}

#undef U_CAN_REGEN

void
stop_occupation(void)
{
    if (go.occupation) {
        if (!maybe_finished_meal(TRUE))
            You("stop %s.", go.occtxt);
        go.occupation = (int (*)(void)) 0;
        disp.botl = TRUE; /* in case u.uhs changed */
        nomul(0);
    } else if (gm.multi >= 0) {
        nomul(0);
    }
    cmdq_clear(CQ_CANNED);
}

void
init_sound_disp_gamewindows(void)
{
    int menu_behavior = MENU_BEHAVE_STANDARD;

    activate_chosen_soundlib();

    if (iflags.wc_splash_screen && !flags.randomall) {
        SoundAchievement(0, sa2_splashscreen, 0);
        /* ToDo: new splash screen invocation will go here */
    } else {
        SoundAchievement(0, sa2_newgame_nosplash, 0);
    }

#ifdef CHANGE_COLOR
    /* init_nhwindows() has already been called, so before
       creating the windows, check to see if there are any
       palette entries to alter */
    change_palette();
#endif

    WIN_MESSAGE = create_nhwindow(NHW_MESSAGE);
    if (VIA_WINDOWPORT()) {
        status_initialize(FALSE);
    } else {
        WIN_STATUS = create_nhwindow(NHW_STATUS);
    }
    WIN_MAP = create_nhwindow(NHW_MAP);
    WIN_INVEN = create_nhwindow(NHW_MENU);
    if (WIN_INVEN != WIN_ERR)
        adjust_menu_promptstyle(WIN_INVEN, &iflags.menu_headings);

#ifdef TTY_PERM_INVENT
    if (WINDOWPORT(tty) && WIN_INVEN != WIN_ERR) {
        menu_behavior = MENU_BEHAVE_PERMINV;
        prepare_perminvent(WIN_INVEN);
    }
#endif
    /* in case of early quit where WIN_INVEN could be destroyed before
       ever having been used, use it here to pacify the Qt interface */
    start_menu(WIN_INVEN, menu_behavior), end_menu(WIN_INVEN, (char *) 0);

#ifdef MAC68K
    /* This _is_ the right place for this - maybe we will
     * have to split init_sound_disp_gamewindows into
     * create_gamewindows and show_gamewindows to get rid of this ifdef...
     */
    if (!strcmp(windowprocs.name, "mac"))
        SanePositions();
#endif

    /*
     * The mac port is not DEPENDENT on the order of these
     * displays, but it looks a lot better this way...
     */
#ifndef STATUS_HILITES
    display_nhwindow(WIN_STATUS, FALSE);
#endif
    display_nhwindow(WIN_MESSAGE, FALSE);
    clear_glyph_buffer();
    display_nhwindow(WIN_MAP, FALSE);
#ifdef TTY_PERM_INVENT
    if (iflags.perm_invent_pending)
        check_perm_invent_again();
#endif
}

void
newgame(void)
{
    int i;

#ifdef SYSCF
    time_t last_reroll_time;
    time_t cur_reroll_time;
    int rerolls_this_second = 0;
# if defined(BSD) && !defined(POSIX_TYPES)
#  define GET_REROLL_TIME(t) (void) time((long *) t);
# else
#  define GET_REROLL_TIME(t) (void) time(t);
# endif
#endif /* defined(SYSCF) */

    /* make sure welcome messages are given before noticing monsters */
    notice_mon_off();
    disp.botlx = TRUE;
    svc.context.ident = 2;  /* id 1 is reserved for gy.youmonst */
    svc.context.warnlevel = 1;
    svc.context.next_attrib_check = 600L; /* arbitrary first setting */
    svc.context.tribute.enabled = TRUE;   /* turn on 3.6 tributes    */
    svc.context.tribute.tributesz = sizeof(struct tribute_info);
    svc.context.inf_aligns = rn2(6);      /* randomness for the Infidel role */
    svc.context.next_moloch_offering = 6000; /* give a grace period before
                                              * the first sacrifice */
    get_nhuuid();

    for (i = LOW_PM; i < NUMMONS; i++)
        svm.mvitals[i].mvflags = mons[i].geno & G_NOCORPSE;

    init_objects(); /* must be before u_init() */

    flags.pantheon = -1; /* role_init() will reset this */
    role_init();         /* must be before init_dungeons(), u_init(),
                          * and init_artifacts() */

    init_dungeons();  /* must be before u_init() to avoid rndmonst()
                       * creating odd monsters for any tins and eggs
                       * in hero's initial inventory */
    init_artifacts(); /* before u_init() in case $WIZKIT specifies
                       * any artifacts */
    u_init_misc();

    l_nhcore_init();  /* create a Lua state that lasts until end of game */
    reset_glyphmap(gm_newgame);
#ifndef NO_SIGNAL
    (void) signal(SIGINT, (SIG_RET_TYPE) done1);
#endif
#ifdef NEWS
    if (iflags.news)
        display_file(NEWS, FALSE);
#endif
    /* quest_init();  --  Now part of role_init() */

    mklev();
    u_on_upstairs();
    vision_reset();          /* set up internals for level (after mklev) */
    check_special_room(FALSE);

    if (MON_AT(u.ux, u.uy))
        mnexto(m_at(u.ux, u.uy), RLOC_NOMSG);
    (void) makedog();

    u_init_inventory_attrs();
    docrt();
    flush_screen(1);
    bot();

#ifdef SYSCF
    GET_REROLL_TIME(&last_reroll_time);
#endif

    while (u.uroleplay.reroll && reroll_menu()) {
#ifdef SYSCF
        if (sysopt.maxrerollrate > 0) {
        check_reroll_time:
            GET_REROLL_TIME(&cur_reroll_time);

            if (last_reroll_time != cur_reroll_time) {
                last_reroll_time = cur_reroll_time;
                rerolls_this_second = 1;
            } else {
                if (rerolls_this_second >= sysopt.maxrerollrate) {
                    if (!paranoid_query(TRUE, "Continue rerolling?"))
                        break;
                    goto check_reroll_time;
                }
                ++rerolls_this_second;
            }
        }
#endif

        ++u.uroleplay.numrerolls;
        u_init_inventory_attrs();
        bot();
    }
    u_init_skills_discoveries();

    /* Convicts begin the game punished, dragging a light iron ball;
       done after the reroll loop so the ball and chain cannot be
       disturbed by inventory re-initialization */
    if (Role_if(PM_CONVICT)) {
        setworn(mkobj(CHAIN_CLASS, TRUE), W_CHAIN);
        setworn(mkobj(BALL_CLASS, TRUE), W_BALL);
        uball->spe = 1;
        placebc();
        newsym(u.ux, u.uy);
    }

    if (wizard) {
        read_wizkit();
        obj_delivery(FALSE); /* finish wizkit */
    }

    /* a pauper Infidel starts with nothing, which would make the game
       unwinnable: the Amulet of Yendor they normally carry is their
       win condition and no other source of it exists for them; place
       the Amulet on the floor at the hero's feet instead, matching
       the state it would have had in a normal Infidel's inventory */
    if (u.uroleplay.pauper && Role_if(PM_INFIDEL)) {
        struct obj *amu = mksobj(AMULET_OF_YENDOR, TRUE, FALSE);

        if (objects[amu->otyp].oc_uses_known)
            amu->known = 1;
        amu->dknown = amu->bknown = amu->rknown = 1;
        amu->cursed = amu->blessed = 0;
        place_object(amu, u.ux, u.uy);
        newsym(u.ux, u.uy);
    }

    if (flags.legacy) {
        /* pauper text takes precedence for all roles; Convicts have
           their own legacy message; a role's quest section can define
           an optional alternate game start message ("altstart",
           currently only Infidels) that replaces the common legacy
           text */
        if (u.uroleplay.pauper)
            com_pager("pauper_legacy");
        else if (Role_if(PM_CONVICT))
            com_pager("legacy_convict");
        else if (!qt_pager_opt("altstart"))
            com_pager("legacy");
    }

    urealtime.realtime = 0L;
    urealtime.start_timing = getnow();
#ifdef INSURANCE
    save_currentstate();
#endif
    program_state.something_worth_saving++; /* useful data now exists */

    /* Success! */
    welcome(TRUE);
    notice_mon_on(); /* now we can notice monsters */
    if (a11y.glyph_updates)
        (void) dolookaround();
    else
        notice_all_mons(TRUE);
    return;
}

/* show "welcome [back] to NetHack" message at program startup */
void
welcome(boolean new_game) /* false => restoring an old game */
{
    char buf[BUFSZ];
    boolean currentgend = Upolyd ? u.mfemale : flags.female,
            adrift = (u.ualign.type != u.ualignbase[A_CURRENT]);

    l_nhcore_call(new_game ? NHCORE_START_NEW_GAME : NHCORE_RESTORE_OLD_GAME);

    /* skip "welcome back" if restoring a doomed character */
    if (!new_game && Upolyd && ugenocided()) {
        /* death via self-genocide is pending */
        pline("You're back, but you still feel %s inside.", udeadinside());
        return;
    }

    if (Hallucination)
        pline("NetHack is filmed in front of an undead studio audience.");

    /*
     * The "welcome back" message always describes your innate form
     * even when polymorphed or wearing a helm of opposite alignment.
     * Alignment is shown unconditionally for new games; for restores
     * it's only shown if it has changed from its original value.
     * Sex is shown for new games except when it is redundant; for
     * restores it's only shown if different from its original value.
     */
    *buf = '\0';
#if 0
    if (new_game || u.ualignbase[A_ORIGINAL] != u.ualignbase[A_CURRENT])
        Sprintf(eos(buf), " %s", align_str(u.ualignbase[A_ORIGINAL]));
#else
    /*
     * 2026-04-24
     * GitHub issue https://github.com/NetHack/NetHack/issues/537
     * "Judging by the comment above, it should display your new alignment
     *  if it was changed, so align_str(u.ualignbase[A_CURRENT]) would
     *  probably be more appropriate. This won't affect the new game message."
     *
     * That is followed by a suggestion to revisit the matter (paraphrased):
     * "That's actually intentional; the comment oversimplifies.
     *  When it was implemented, it may have been the only way to tell that
     *  you had converted alignment. Now ^X mentions your starting alignment
     *  if base alignment has been changed, so revisiting this welcome back
     *  message."
     */
    if (new_game || u.ualignbase[A_ORIGINAL] != u.ualignbase[A_CURRENT] || adrift)
        Sprintf(eos(buf), " %s%s",
                adrift ? "adrift " : "",
                adrift ? align_str(u.ualign.type)
                       : align_str(u.ualignbase[A_CURRENT]));
#endif
    if (!gu.urole.name.f
        && (new_game
            ? (gu.urole.allow & ROLE_GENDMASK) == (ROLE_MALE | ROLE_FEMALE)
            : currentgend != flags.initgend))
        Sprintf(eos(buf), " %s", genders[currentgend].adj);
    Sprintf(eos(buf), " %s %s", gu.urace.adj,
            (currentgend && gu.urole.name.f) ? gu.urole.name.f
                                             : gu.urole.name.m);

    if (u.ualign.type == A_NONE)
        pline(new_game ? "%s %s, welcome to NetHack!  You are an%s."
                       : "%s %s, the%s, welcome back to NetHack!",
              Hello((struct monst *) 0), svp.plname, buf);
    else
        pline(new_game ? "%s %s, welcome to NetHack!  You are a%s."
                       : "%s %s, the%s, welcome back to NetHack!",
              Hello((struct monst *) 0), svp.plname, buf);

    if (new_game) {
        /* racial ability hints */
        if (Race_if(PM_TORTLE))
            pline("Use #monster to hide in your shell.");
        else if (Race_if(PM_DROW))
            pline("Use #monster to invoke an aura of darkness.");
        else if (Race_if(PM_VAMPIRE))
            pline("Use #shapechange to change form.");
        else if (Race_if(PM_ILLITHID))
            pline("Use #telekinesis to manipulate objects and monsters.");
        else if (Race_if(PM_AASIMAR))
            pline("Use #monster to invoke an aura of light.");

        /* Roles */
        if (Role_if(PM_DRUID))
            pline("Use #wildshape to change form.");

        /* guarantee that 'major' event category is never empty */
        livelog_printf(LL_ACHIEVE, "%s the%s entered the dungeon",
                       svp.plname, buf);
    } else {
        /* if restoring in Gehennom, give same hot/smoky message as when
           first entering it */
        hellish_smoke_mesg();
        /* remind player of the level annotation, like in goto_level() */
        print_level_annotation();
    }
}

#ifdef POSITIONBAR
staticfn void
do_positionbar(void)
{
    /* FIXME: this will break if any coordinate is too big for (char);
       the sys/msdos/vid*.c code uses (unsigned char) which is less
       vulnerable but not guaranteed to be able to hold coordxy values;
       also, there doesn't appear to be any need for this to be static,
       nor to contain pairs of (> or <) and x; it could just be a full
       line of spaces and > or < characters with update_positionbar()
       revised to reconstruct the x values for non-space characters */
    static char pbar[COLNO];
    char *p;
    stairway *stway;
    coordxy x, y;
    int glyph, symbol;

    p = pbar;
    /* TODO: use the same method as getpos() so objects don't cover stairs */
    /* FIXME: traversing 'stairs' list ignores mimics that pose as stairs */
    for (stway = gs.stairs; stway; stway = stway->next) {
        x = stway->sx;
        y = stway->sy;
        glyph = levl[x][y].glyph;
        symbol = glyph_to_cmap(glyph);

        if (is_cmap_stairs(symbol)) {
            *p++ = (stway->up ? '<' : '>');
            *p++ = (char) x;
        }
     }

    /* hero location */
    if (u.ux) {
        *p++ = '@';
        *p++ = u.ux;
    }
    /* fence post */
    *p = 0;

    update_positionbar(pbar);
}
#endif

staticfn void
interrupt_multi(const char *msg)
{
    if (gm.multi > 0 && !svc.context.travel && !svc.context.run) {
        nomul(0);
        if (flags.verbose && msg)
            Norep("%s", msg);
    }
}

/* convert from time_t to number of seconds */
long
timet_to_seconds(time_t ttim)
{
    /* for Unix-based and Posix-compliant systems, a cast to 'long' would
       suffice but the C Standard doesn't require time_t to be that simple */
    return timet_delta(ttim, (time_t) 0);
}

/* calculate the difference in seconds between two time_t values */
long
timet_delta(time_t etim, time_t stim) /* end and start times */
{
    /* difftime() is a STDC routine which returns the number of seconds
       between two time_t values as a 'double' */
    return (long) difftime(etim, stim);
}

/*allmain.c*/
