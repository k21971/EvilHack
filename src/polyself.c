/* NetHack 3.6	polyself.c	$NHDT-Date: 1573290419 2019/11/09 09:06:59 $  $NHDT-Branch: NetHack-3.6 $:$NHDT-Revision: 1.135 $ */
/*      Copyright (C) 1987, 1988, 1989 by Ken Arromdee */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * Polymorph self routine.
 *
 * Note:  the light source handling code assumes that both youmonst.m_id
 * and youmonst.mx will always remain 0 when it handles the case of the
 * player polymorphed into a light-emitting monster.
 *
 * Transformation sequences:
 *              /-> polymon                 poly into monster form
 *    polyself =
 *              \-> newman -> polyman       fail to poly, get human form
 *
 *    rehumanize -> polyman                 return to original form
 *
 *    polymon (called directly)             usually golem petrification
 */

#include "hack.h"

STATIC_DCL void FDECL(check_strangling, (BOOLEAN_P, BOOLEAN_P));
STATIC_DCL void FDECL(polyman, (const char *, const char *));
STATIC_DCL void FDECL(dropp, (struct obj *));
STATIC_DCL void NDECL(break_armor);
STATIC_DCL void FDECL(drop_weapon, (int));
STATIC_DCL void NDECL(newman);
STATIC_DCL void NDECL(polysense);

STATIC_VAR const char no_longer_petrify_resistant[] =
    "No longer petrify-resistant, you";

/* controls whether taking on new form or becoming new man can also
   change sex (ought to be an arg to polymon() and newman() instead) */
STATIC_VAR int sex_change_ok = 0;

/* update the youmonst.data structure pointer and intrinsics */
void
set_uasmon()
{
    struct permonst *mdat = &mons[u.umonnum];
    struct permonst *racedat; /* for infravision, flying */

    set_mon_data(&youmonst, mdat);
    racedat = raceptr(&youmonst);

#define PROPSET(PropIndx, ON)                          \
    do {                                               \
        if (ON)                                        \
            u.uprops[PropIndx].intrinsic |= FROMFORM;  \
        else                                           \
            u.uprops[PropIndx].intrinsic &= ~FROMFORM; \
    } while (0)

    PROPSET(FIRE_RES, resists_fire(&youmonst));
    PROPSET(COLD_RES, resists_cold(&youmonst));
    PROPSET(SLEEP_RES, resists_sleep(&youmonst));
    PROPSET(DISINT_RES, resists_disint(&youmonst));
    PROPSET(SHOCK_RES, resists_elec(&youmonst));
    PROPSET(POISON_RES, resists_poison(&youmonst));
    PROPSET(ACID_RES, resists_acid(&youmonst));
    PROPSET(STONE_RES, resists_ston(&youmonst));
    PROPSET(PSYCHIC_RES, resists_psychic(&youmonst));
    PROPSET(DRAIN_RES, resists_drain(racedat));
    PROPSET(STUN_RES, resists_stun(racedat));

    /* Vulnerablilties */
    PROPSET(VULN_FIRE, vulnerable_to(&youmonst, AD_FIRE));
    PROPSET(VULN_COLD, vulnerable_to(&youmonst, AD_COLD));
    PROPSET(VULN_ELEC, vulnerable_to(&youmonst, AD_ELEC));
    PROPSET(VULN_ACID, vulnerable_to(&youmonst, AD_ACID));

    PROPSET(ANTIMAGIC, resists_mgc(mdat));
    PROPSET(SICK_RES, ptr_resists_sick(mdat));
    PROPSET(DEATH_RES, immune_death_magic(racedat));

    PROPSET(STUNNED, (mdat == &mons[PM_STALKER] || is_bat(mdat)));
    PROPSET(HALLUC_RES, dmgtype(mdat, AD_HALU));
    PROPSET(SEE_INVIS, perceives(mdat));
    PROPSET(TELEPAT, telepathic(mdat));
    /* note that Infravision/Ultravision uses mons[race] rather than usual mons[role] */
    PROPSET(INFRAVISION, infravision(racedat));
    PROPSET(ULTRAVISION, ultravision(racedat));
    PROPSET(INVIS, pm_invisible(mdat));
    PROPSET(TELEPORT, can_teleport(mdat));
    PROPSET(TELEPORT_CONTROL, control_teleport(mdat));
    PROPSET(LEVITATION, is_floater(mdat));
    /* floating eye is the only 'floater'; it is also flagged as a 'flyer';
       suppress flying for it so that enlightenment doesn't confusingly
       show latent flight capability always blocked by levitation */
    /* this property also checks race instead of role */
    PROPSET(FLYING, (is_flyer(racedat) && !is_floater(racedat)));
    if (!program_state.restoring) /* if loading, defer wings check until we have a steed */
        check_wings(TRUE);
    PROPSET(SWIMMING, is_swimmer(mdat));
    /* [don't touch MAGICAL_BREATHING here; both Amphibious and Breathless
       key off of it but include different monster forms...] */
    PROPSET(PASSES_WALLS, passes_walls(mdat));
    PROPSET(REGENERATION, regenerates(mdat));
    PROPSET(REFLECTING, (mdat == &mons[PM_SILVER_DRAGON]));
#undef PROPSET

    /* Nonliving monsters are immune to withering.
     * This could use is_fleshy(), but that would
     * make a large set of monsters immune like
     * fungus, blobs, and jellies.
     * TODO: make is_vampshifter actually work for youmonst, and include that. */
    if (nonliving(mdat) || racial_zombie(&youmonst))
        BWithering |= FROMFORM;
    else
        BWithering &= ~FROMFORM;
    /* context.botl = TRUE happens in the next line. */

    drop_weapon(1);
    float_vs_flight(); /* maybe toggle (BFlying & I_SPECIAL) */
    polysense();

#ifdef STATUS_HILITES
    if (VIA_WINDOWPORT())
        status_initialize(REASSESS_ONLY);
#endif
}

/* Levitation overrides Flying; set or clear BFlying|I_SPECIAL */
void
float_vs_flight()
{
    boolean stuck_in_floor = (u.utrap && u.utraptype != TT_PIT);

    /* floating overrides flight; so does being trapped in the floor */
    if ((HLevitation || ELevitation)
        || ((HFlying || EFlying) && stuck_in_floor))
        BFlying |= I_SPECIAL;
    else
        BFlying &= ~I_SPECIAL;
    /* being trapped on the ground (bear trap, web, molten lava survived
       with fire resistance, former lava solidified via cold, tethered
       to a buried iron ball) overrides floating--the floor is reachable */
    if ((HLevitation || ELevitation) && stuck_in_floor)
        BLevitation |= I_SPECIAL;
    else
        BLevitation &= ~I_SPECIAL;
    context.botl = TRUE;
}

/* for changing into form that's immune to strangulation */
STATIC_OVL void
check_strangling(on, noisy)
boolean on, noisy;
{
    /* Suffocating due to being engulfed by something with a suffocation attack,
     * versus due to wearing an amulet of strangulation. We assume that
     * suffocation isn't actually strangulation, and the hero is OK if
     * breathless. */
    boolean from_mon = (u.uswallow && u.ustuck &&
                        attacktype_fordmg(u.ustuck->data, AT_ENGL, AD_WRAP));
    boolean from_amul = (uamul && uamul->otyp == AMULET_OF_STRANGULATION &&
                         can_be_strangled(&youmonst));

    /* on -- maybe resume strangling */
    if (on) {
        /* when Strangled is already set, polymorphing from one
           vulnerable form into another causes the counter to be reset */
    if (from_amul) {
            Your("%s %s your %s!", simpleonames(uamul),
                 Strangled ? "still constricts" : "begins constricting",
                 body_part(NECK)); /* "throat" */
            Strangled = 6L;
            context.botl = TRUE;
            makeknown(AMULET_OF_STRANGULATION);
        }
        else if (from_mon && !Breathless) {
            Strangled = 6L;
            context.botl = TRUE;
            if (noisy) {
                You("can't breathe in here!");
            }
        }
    /* off -- maybe block strangling */
    } else if (Strangled) {
        if (from_mon && Breathless) {
            Strangled = 0L;
            context.botl = TRUE;
            if (noisy)
                You("don't seem to need air anymore.");
        }
        if (from_amul && Strangled && !can_be_strangled(&youmonst)) {
            Strangled = 0L;
            context.botl = TRUE;
            You("are no longer being strangled.");
        }
    }
}

/* make a (new) human out of the player */
STATIC_OVL void
polyman(fmt, arg)
const char *fmt, *arg;
{
    boolean sticky = (sticks(youmonst.data) && u.ustuck && !u.uswallow),
            was_mimicking = (U_AP_TYPE != M_AP_NOTHING);
    boolean was_blind = !!Blind;

    if (Upolyd) {
        u.acurr = u.macurr; /* restore old attribs */
        u.amax = u.mamax;
        u.umonnum = u.umonster;
        flags.female = u.mfemale;
    }

    You(fmt, arg);

    set_uasmon();

    u.mh = u.mhmax = 0;
    u.mtimedone = 0;
    skinback(FALSE);
    u.uundetected = 0;

    if (sticky)
        uunstick();
    find_ac();
    if (was_mimicking) {
        if (multi < 0)
            unmul("");
        youmonst.m_ap_type = M_AP_NOTHING;
        youmonst.mappearance = 0;
    }

    newsym(u.ux, u.uy);

    /* check whether player foolishly genocided self while poly'd */
    if (ugenocided()) {
        /* intervening activity might have clobbered genocide info */
        struct kinfo *kptr = find_delayed_killer(POLYMORPH);

        if (kptr != (struct kinfo *) 0 && kptr->name[0]) {
            killer.format = kptr->format;
            Strcpy(killer.name, kptr->name);
        } else {
            killer.format = KILLED_BY;
            Strcpy(killer.name, "self-genocide");
        }
        dealloc_killer(kptr);
        done(GENOCIDED);
    }

    if (u.twoweap && !could_twoweap(youmonst.data))
        untwoweapon();

    if (u.utrap && u.utraptype == TT_PIT) {
        set_utrap(rn1(6, 2), TT_PIT); /* time to escape resets */
    }
    if (was_blind && !Blind) { /* reverting from eyeless */
        Blinded = 1L;
        make_blinded(0L, TRUE); /* remove blindness */
    }
    check_strangling(TRUE, TRUE);

    if (!Levitation && !u.ustuck && is_pool_or_lava(u.ux, u.uy))
        spoteffects(TRUE);

    see_monsters();
}

void
change_sex()
{
    /* setting u.umonster for caveman/cavewoman or priest/priestess
       swap unintentionally makes `Upolyd' appear to be true */
    boolean already_polyd = (boolean) Upolyd;

    /* Some monsters are always of one sex and their sex can't be changed;
     * Succubi/incubi can change, but are handled below.
     *
     * !already_polyd check necessary because is_male() and is_female()
     * are true if the player is a priest/priestess.
     */
    if (!already_polyd
        || (!is_male(youmonst.data) && !is_female(youmonst.data)
            && !is_neuter(youmonst.data)))
        flags.female = !flags.female;
    if (already_polyd) /* poly'd: also change saved sex */
        u.mfemale = !u.mfemale;
    max_rank_sz(); /* [this appears to be superfluous] */
    if ((already_polyd ? u.mfemale : flags.female) && urole.name.f)
        Strcpy(pl_character, urole.name.f);
    else
        Strcpy(pl_character, urole.name.m);
    u.umonster = ((already_polyd ? u.mfemale : flags.female)
                  && urole.femalenum != NON_PM)
                     ? urole.femalenum
                     : urole.malenum;
    if (!already_polyd) {
        u.umonnum = u.umonster;
    } else if (u.umonnum == PM_SUCCUBUS || u.umonnum == PM_INCUBUS) {
        flags.female = !flags.female;
        /* change monster type to match new sex */
        u.umonnum = (u.umonnum == PM_SUCCUBUS) ? PM_INCUBUS : PM_SUCCUBUS;
        set_uasmon();
    }
}

STATIC_OVL void
newman()
{
    int i, oldlvl, newlvl, hpmax, enmax;

    oldlvl = u.ulevel;
    newlvl = oldlvl + rn1(5, -2);     /* new = old + {-2,-1,0,+1,+2} */
    if (newlvl > 127 || newlvl < 1) { /* level went below 0? */
        goto dead; /* old level is still intact (in case of lifesaving) */
    }
    if (newlvl > MAXULEV)
        newlvl = MAXULEV;
    /* If your level goes down, your peak level goes down by
       the same amount so that you can't simply use blessed
       full healing to undo the decrease.  But if your level
       goes up, your peak level does *not* undergo the same
       adjustment; you might end up losing out on the chance
       to regain some levels previously lost to other causes. */
    if (newlvl < oldlvl)
        u.ulevelmax -= (oldlvl - newlvl);
    if (u.ulevelmax < newlvl)
        u.ulevelmax = newlvl;
    u.ulevel = newlvl;

    if (sex_change_ok && !rn2(10))
        change_sex();

    adjabil(oldlvl, (int) u.ulevel);

    /* random experience points for the new experience level */
    u.uexp = rndexp(FALSE);

    /* set up new attribute points (particularly Con) */
    redist_attr();

    /*
     * New hit points:
     *  remove level-gain based HP from any extra HP accumulated
     *  (the "extra" might actually be negative);
     *  modify the extra, retaining {80%, 90%, 100%, or 110%};
     *  add in newly generated set of level-gain HP.
     *
     * (This used to calculate new HP in direct proportion to old HP,
     * but that was subject to abuse:  accumulate a large amount of
     * extra HP, drain level down to 1, then polyself to level 2 or 3
     * [lifesaving capability needed to handle level 0 and -1 cases]
     * and the extra got multiplied by 2 or 3.  Repeat the level
     * drain and polyself steps until out of lifesaving capability.)
     */
    hpmax = u.uhpmax;
    for (i = 0; i < oldlvl; i++)
        hpmax -= (int) u.uhpinc[i];
    /* hpmax * rn1(4,8) / 10; 0.95*hpmax on average */
    hpmax = rounddiv((long) hpmax * (long) rn1(4, 8), 10);
    for (i = 0; (u.ulevel = i) < newlvl; i++)
        hpmax += newhp();
    if (hpmax < u.ulevel)
        hpmax = u.ulevel; /* min of 1 HP per level */
    /* retain same proportion for current HP; u.uhp * hpmax / u.uhpmax */
    u.uhp = rounddiv((long) u.uhp * (long) hpmax, u.uhpmax);
    u.uhpmax = hpmax;
    /*
     * Do the same for spell power.
     */
    enmax = u.uenmax;
    for (i = 0; i < oldlvl; i++)
        enmax -= (int) u.ueninc[i];
    enmax = rounddiv((long) enmax * (long) rn1(4, 8), 10);
    for (i = 0; (u.ulevel = i) < newlvl; i++)
        enmax += newpw();
    if (enmax < u.ulevel)
        enmax = u.ulevel;
    u.uen = rounddiv((long) u.uen * (long) enmax,
                     ((u.uenmax < 1) ? 1 : u.uenmax));
    u.uenmax = enmax;
    /* [should alignment record be tweaked too?] */

    u.uhunger = rn1(500, 500);
    if (Sick)
        make_sick(0L, (char *) 0, FALSE, SICK_ALL);
    if (Stoned)
        make_stoned(0L, (char *) 0, 0, (char *) 0);
    if (u.uhp <= 0) {
        if (Polymorph_control) { /* even when Stunned || Unaware */
            if (u.uhp <= 0)
                u.uhp = 1;
        } else {
        dead: /* we come directly here if their experience level went to 0 or
                 less */
            Your("new form doesn't seem healthy enough to survive.");
            killer.format = KILLED_BY_AN;
            Strcpy(killer.name, "unsuccessful polymorph");
            done(DIED);
            newuhs(FALSE);
            return; /* lifesaved */
        }
    }
    newuhs(FALSE);
    polyman("feel like a new %s!",
            /* use saved gender we're about to revert to, not current */
            ((Upolyd ? u.mfemale : flags.female) && urace.individual.f)
                ? urace.individual.f
                : (urace.individual.m)
                   ? urace.individual.m
                   : urace.noun);
    if (Slimed) {
        Your("body transforms, but there is still slime on you.");
        make_slimed(10L, (const char *) 0);
    }

    context.botl = 1;
    see_monsters();
    (void) encumber_msg();

    retouch_equipment(2);
    if (!uarmg)
        selftouch(no_longer_petrify_resistant);
}

void
druid_shapechange()
{
    winid win;
    menu_item *selected;
    anything any;
    int i, n;
    boolean old_uwvis = (Underwater && See_underwater);

    win = create_nhwindow(NHW_MENU);
    start_menu(win);
    any = zeroany;

    /* set timer for next allowed shapechange,
       3600-4000 turns (this is an approximation
       to Druids being able to shapechange three
       times a day, ad&d rules)

       this needs to be set before polymon() is
       called due to other behavior while using
       #shapechange */
    u.ushapechange += rn1(400, 3600);

    if (Unchanging) {
        pline("You fail to transform!");
        return;
    }

    if (Hidinshell)
        toggleshell();

    for (i = LOW_PM; i < NUMMONS; i++) {
        n = (LOW_PM + i);
        if (u.ulevel >= 15) {
            if (!all_druid_forms(n))
                continue;
        } else if (u.ulevel >= 10) {
            if (!druid_form_A(n)
                && !druid_form_B(n)
                && !druid_form_C(n))
                continue;
        } else if (u.ulevel >= 6) {
            if (!druid_form_A(n)
                && !druid_form_B(n))
                continue;
        } else if (u.ulevel >= 3) {
            if (!druid_form_A(n))
                continue;
        } else if (u.ulevel < 3) {
            /* extra guard, otherwise potentially
               all monsters become available */
            continue;
        }
        if (n >= NUMMONS)
            break;
        any.a_int = n;
        add_menu(win, NO_GLYPH, &any, 0, 0, ATR_NONE,
                 mons[n].mname, MENU_UNSELECTED);
    }
    end_menu(win, "Pick a form to change into.");
    n = select_menu(win, PICK_ONE, &selected);
    destroy_nhwindow(win);
    if (n > 0) {
        i = selected[0].item.a_int;
        free((genericptr_t) selected);
        (void) polymon(i);
    }

    if (old_uwvis != (Underwater && See_underwater)) {
        vision_reset();
        docrt();
    }
}

void
polyself(psflags)
int psflags;
{
    char buf[BUFSZ];
    int old_wither, new_wither, old_light, new_light, mntmp, class, tryct;
    boolean forcecontrol = (psflags == 1), monsterpoly = (psflags == 2),
            draconian = (!uskin && armor_to_dragon(&youmonst) != NON_PM),
            /* psflags = 4: enchanting dragon scales while confused; polycontrol
             * will only allow declining to become dragon, won't allow turning
             * into arbitrary monster */
            draconian_only = (psflags == 4),
            iswere = (u.ulycn >= LOW_PM), isvamp = is_vampire(youmonst.data),
            controllable_poly = Polymorph_control && !(Stunned || Unaware),
            yourrace, old_uwvis = (Underwater && See_underwater);

    if (Unchanging) {
        pline("You fail to transform!");
        return;
    }
    /* being Stunned|Unaware doesn't negate this aspect of Poly_control */
    if (!Polymorph_control && !forcecontrol && !draconian && !iswere
        && !isvamp) {
        if (rn2(20) > ACURR(A_CON)) {
            You1(shudder_for_moment);
            losehp(rnd(30), "system shock", KILLED_BY_AN);
            exercise(A_CON, FALSE);
            return;
        }
    }
    old_light = emits_light(youmonst.data);
    old_wither = Withering;
    mntmp = NON_PM;

    if (Hidinshell)
        toggleshell();

    if (monsterpoly && isvamp)
        goto do_vampyr;
    if (draconian_only)
        goto do_merge;

    if (controllable_poly || forcecontrol) {
        buf[0] = '\0';
        tryct = 5;
        do {
            mntmp = NON_PM;
            getlin("Become what kind of monster? [type the name]", buf);
            (void) mungspaces(buf);
            if (*buf == '\033') {
                /* user is cancelling controlled poly */
                if (forcecontrol) { /* wizard mode #polyself */
                    pline1(Never_mind);
                    return;
                }
                Strcpy(buf, "*"); /* resort to random */
            }
            if (!strcmp(buf, "*") || !strcmp(buf, "random")) {
                /* explicitly requesting random result */
                tryct = 0; /* will skip thats_enough_tries */
                continue;  /* end do-while(--tryct > 0) loop */
            }
            class = 0;
            mntmp = name_to_mon(buf, (int *) 0);
            if (mntmp < LOW_PM) {
            by_class:
                class = name_to_monclass(buf, &mntmp);
                if (class && mntmp == NON_PM)
                    mntmp = (draconian && class == S_DRAGON)
                            ? armor_to_dragon(&youmonst)
                            : mkclass_poly(class);
            }
            if (mntmp < LOW_PM) {
                if (!class)
                    pline("I've never heard of such monsters.");
                else
                    You_cant("polymorph into any of those.");
            } else if (wizard && Upolyd
                       && (mntmp == u.umonster
                           /* "priest" and "priestess" match the monster
                              rather than the role; override that unless
                              the text explicitly contains "aligned" */
                           || ((u.umonster == PM_PRIEST
                                || u.umonster == PM_PRIESTESS)
                               && mntmp == PM_ALIGNED_PRIEST
                               && !strstri(buf, "aligned")))) {
                /* in wizard mode, picking own role while poly'd reverts to
                   normal without newman()'s chance of level or sex change */
                rehumanize();
                old_light = 0; /* rehumanize() extinguishes u-as-mon light */
                goto made_change;
            } else if (iswere && (were_beastie(mntmp) == u.ulycn
                                  || mntmp == counter_were(u.ulycn)
                                  || (Upolyd && mntmp == PM_HUMAN))) {
                goto do_shift;
            } else if (!polyok(&mons[mntmp])
                       /* Note:  humans are illegal as monsters, but an
                          illegal monster forces newman(), which is what
                          we want if they specified a human.... (unless
                          they specified a unique monster) */
                       && !(mntmp == PM_HUMAN
                            || (your_race(&mons[mntmp])
                                && (mons[mntmp].geno & G_UNIQ) == 0)
                            || mntmp == urole.malenum
                            || mntmp == urole.femalenum)) {
                const char *pm_name;

                /* mkclass_poly() can pick a !polyok()
                   candidate; if so, usually try again */
                if (class) {
                    if (rn2(3) || --tryct > 0)
                        goto by_class;
                    /* no retries left; put one back on counter
                       so that end of loop decrement will yield
                       0 and trigger thats_enough_tries message */
                    ++tryct;
                }
                pm_name = mons[mntmp].mname;
                if (the_unique_pm(&mons[mntmp]))
                    pm_name = the(pm_name);
                else if (!type_is_pname(&mons[mntmp]))
                    pm_name = an(pm_name);
                You_cant("polymorph into %s.", pm_name);
            } else
                break;
        } while (--tryct > 0);
        if (!tryct)
            pline1(thats_enough_tries);
        /* allow skin merging, even when polymorph is controlled */
        if (draconian && (tryct <= 0 || mntmp == armor_to_dragon(&youmonst)))
            goto do_merge;
        if (isvamp && (tryct <= 0 || mntmp == PM_WOLF || mntmp == PM_FOG_CLOUD
                       || is_bat(&mons[mntmp])))
            goto do_vampyr;
    } else if (draconian || iswere || isvamp) {
        /* special changes that don't require polyok() */
        if (draconian) {
        do_merge:
            /* armor_to_dragon will return uskin if uskin turned you into a
             * dragon; thus, the assumption here is that a hero who turned into
             * a dragon by merging with armor won't be able to wear other types
             * of dragon armor while polymorphed. */
            mntmp = armor_to_dragon(&youmonst);
            if (controllable_poly) {
                Sprintf(buf, "Become %s?", an(mons[mntmp].mname));
                if (yn(buf) != 'y') {
                    return;
                }
            }
            if (!(mvitals[mntmp].mvflags & G_GENOD)) {
                struct obj **mergarm =
                    (uarm && Is_dragon_scaled_armor(uarm)) ? &uarm
                      : (uarmc && Is_dragon_scales(uarmc)) ? &uarmc
                        : (struct obj **) 0;
                unsigned was_lit = mergarm ? (*mergarm)->lamplit : 0;
                int arm_light = mergarm && artifact_light(*mergarm)
                                  ? arti_light_radius(*mergarm) : 0;
                /* allow G_EXTINCT */
                You("merge with your scaly armor.");
                if (uskin) {
                    impossible("Already merged with some armor!");
                } else if (!mergarm) {
                    impossible("No dragon armor / dragon cloak to merge?");
                } else {
                    uskin = *mergarm;
                    *mergarm = NULL;
                    /* dragon scales remain intact as uskin */
                }
                /* save/restore hack */
                uskin->owornmask |= I_SPECIAL;
                if (was_lit)
                    maybe_adjust_light(uskin, arm_light);
                update_inventory();
            }
        } else if (iswere) {
        do_shift:
            if (Upolyd && were_beastie(mntmp) != u.ulycn)
                mntmp = PM_HUMAN; /* Illegal; force newman() */
            else
                mntmp = u.ulycn;
        } else if (isvamp) {
        do_vampyr:
            if (mntmp < LOW_PM || (mons[mntmp].geno & G_UNIQ))
                mntmp = (youmonst.data != &mons[PM_VAMPIRE] && !rn2(10))
                            ? PM_WOLF
                            : !rn2(4) ? PM_FOG_CLOUD : PM_VAMPIRE_BAT;
            if (controllable_poly) {
                Sprintf(buf, "Become %s?", an(mons[mntmp].mname));
                if (yn(buf) != 'y')
                    return;
            }
        }
        /* if polymon fails, "you feel" message has been given
           so don't follow up with another polymon or newman;
           sex_change_ok left disabled here */
        if (mntmp == PM_HUMAN)
            newman(); /* werecritter */
        else
            (void) polymon(mntmp);
        goto made_change; /* maybe not, but this is right anyway */
    }

    if (mntmp < LOW_PM) {
        tryct = 200;
        do {
            /* randomly pick an "ordinary" monster */
            mntmp = rn1(SPECIAL_PM - LOW_PM, LOW_PM);
            if (polyok(&mons[mntmp]) && !is_placeholder(&mons[mntmp]))
                break;
        } while (--tryct > 0);
    }

    /* For polymorphing, (fire, frost, hill, stone, storm) giants are
     * not the same race as a giant player and should not cause newman().
     * Same goes for centaurs */
    yourrace = your_race(&mons[mntmp]);
    if (Race_if(PM_GIANT))
        yourrace = (mntmp == PM_GIANT);
    if (Race_if(PM_CENTAUR))
        yourrace = (mntmp == PM_CENTAUR);

    /* The below polyok() fails either if everything is genocided, or if
     * we deliberately chose something illegal to force newman().
     */
    sex_change_ok++;
    if (!polyok(&mons[mntmp]) || (!forcecontrol && !rn2(5))
        || yourrace) {
        newman();
    } else {
        (void) polymon(mntmp);
    }
    sex_change_ok--; /* reset */

made_change:
    if (old_uwvis != (Underwater && See_underwater)) {
        vision_reset();
        docrt();
    }
    new_light = emits_light(youmonst.data);
    if (old_light != new_light) {
        if (old_light)
            del_light_source(LS_MONSTER, monst_to_any(&youmonst));
        if (new_light == 1)
            ++new_light; /* otherwise it's undetectable */
        if (new_light)
            new_light_source(u.ux, u.uy, new_light, LS_MONSTER,
                             monst_to_any(&youmonst));
    }
    new_wither = Withering;
    if (old_wither && !new_wither) {
        You("are no longer withering away.");
        /* Currently the only way to have BWithering is to be nonliving.
         * But I feel another source of BWithering might not necessarily
         * cure it. Hence this arbitrary distinction. */
        if (nonliving(youmonst.data))
            /* intrinsic withering has no foothold on a dead body */
            set_itimeout(&HWithering, (long) 0);
    } else if (!old_wither && new_wither) {
        /* only happens with extrinsic withering */
        You("begin to wither away!");
    }
}

/* (try to) make a mntmp monster out of the player;
   returns 1 if polymorph successful */
int
polymon(mntmp)
int mntmp;
{
    char buf[BUFSZ];
    boolean sticky = sticks(youmonst.data) && u.ustuck && !u.uswallow,
            was_blind = !!Blind, dochange = FALSE;
    int mlvl;

    if (mvitals[mntmp].mvflags & G_GENOD) { /* allow G_EXTINCT */
        You_feel("rather %s-ish.", mons[mntmp].mname);
        exercise(A_WIS, TRUE);
        return 0;
    }

    if (Hidinshell)
        toggleshell();

    /* KMH, conduct */
    if (!u.uconduct.polyselfs++)
        livelog_printf(LL_CONDUCT,
                       "changed form for the first time, becoming %s",
                       an(mons[mntmp].mname));

    /* exercise used to be at the very end but only Wis was affected
       there since the polymorph was always in effect by then */
    exercise(A_CON, FALSE);
    exercise(A_WIS, TRUE);

    if (!Upolyd) {
        /* Human to monster; save human stats */
        u.macurr = u.acurr;
        u.mamax = u.amax;
        u.mfemale = flags.female;
    } else {
        /* Monster to monster; restore human stats, to be
         * immediately changed to provide stats for the new monster
         */
        u.acurr = u.macurr;
        u.amax = u.mamax;
        flags.female = u.mfemale;
    }

    /* if stuck mimicking gold, stop immediately */
    if (multi < 0 && U_AP_TYPE == M_AP_OBJECT
        && youmonst.data->mlet != S_MIMIC)
        unmul("");
    /* if becoming a non-mimic, stop mimicking anything */
    if (mons[mntmp].mlet != S_MIMIC) {
        /* as in polyman() */
        youmonst.m_ap_type = M_AP_NOTHING;
        youmonst.mappearance = 0;
    }
    if (is_male(&mons[mntmp])) {
        if (flags.female)
            dochange = TRUE;
    } else if (is_female(&mons[mntmp])) {
        if (!flags.female)
            dochange = TRUE;
    } else if (!is_neuter(&mons[mntmp]) && mntmp != u.ulycn) {
        if (sex_change_ok && !rn2(10))
            dochange = TRUE;
    }

    Strcpy(buf, (u.umonnum != mntmp) ? "" : "new ");
    if (dochange) {
        flags.female = !flags.female;
        Strcat(buf, (is_male(&mons[mntmp]) || is_female(&mons[mntmp]))
                       ? "" : flags.female ? "female " : "male ");
    }
    Strcat(buf, mons[mntmp].mname);
    You("%s %s!", (u.umonnum != mntmp) ? "turn into" : "feel like", an(buf));

    if (Stoned && poly_when_stoned(&mons[mntmp])) {
        /* poly_when_stoned already checked stone golem genocide */
        mntmp = PM_STONE_GOLEM;
        make_stoned(0L, "You turn to stone!", 0, (char *) 0);
    }

    u.mtimedone = rn1(500, 500);
    u.umonnum = mntmp;
    set_uasmon();

    /* New stats for monster, to last only as long as polymorphed.
     * Currently only strength gets changed.
     */
    if (strongmonst(&mons[mntmp]))
        ABASE(A_STR) = AMAX(A_STR) = STR18(100);

    if (Stone_resistance && Stoned) { /* parnes@eniac.seas.upenn.edu */
        make_stoned(0L, "You no longer seem to be petrifying.", 0,
                    (char *) 0);
    }
    if (Sick_resistance && Sick) {
        make_sick(0L, (char *) 0, FALSE, SICK_ALL);
        You("no longer feel sick.");
    }
    if (Slimed) {
        if (flaming(youmonst.data)) {
            make_slimed(0L, "The slime burns away!");
        } else if (mntmp == PM_GREEN_SLIME) {
            /* do it silently */
            make_slimed(0L, (char *) 0);
        }
    }
    check_strangling(FALSE, FALSE); /* maybe stop strangling */
    if (nohands(youmonst.data))
        make_glib(0);

    /* no magic resistance conduct */
    if (resists_mgc(youmonst.data))
        u.uconduct.antimagic++;

    /* no reflection conduct. currently, the only monster
       that has reflection that a player can polymorph
       into is an adult silver dragon. should that ever
       change, we'll need to revisit this bit of code */
    if (youmonst.data == &mons[PM_SILVER_DRAGON])
        u.uconduct.reflection++;

    /*
    mlvl = adj_lev(&mons[mntmp]);
     * We can't do the above, since there's no such thing as an
     * "experience level of you as a monster" for a polymorphed character.
     */
    mlvl = (int) mons[mntmp].mlevel;
    u.mhmax = u.mh = monmaxhp(&mons[mntmp], mlvl);

    if (u.ulevel < mlvl) {
        /* Low level characters can't become high level monsters for long */
#ifdef DUMB
        /* DRS/NS 2.2.6 messes up -- Peter Kendell */
        int mtd = u.mtimedone, ulv = u.ulevel;

        u.mtimedone = mtd * ulv / mlvl;
#else
        u.mtimedone = u.mtimedone * u.ulevel / mlvl;
#endif
    }

    if (uskin && mntmp != armor_to_dragon(&youmonst))
        skinback(FALSE);
    break_armor();
    drop_weapon(1);
    (void) hideunder(&youmonst);

    if (u.utrap && u.utraptype == TT_PIT) {
        set_utrap(rn1(6, 2), TT_PIT); /* time to escape resets */
    }
    if (was_blind && !Blind) { /* previous form was eyeless */
        Blinded = 1L;
        make_blinded(0L, TRUE); /* remove blindness */
    }
    newsym(u.ux, u.uy); /* Change symbol */

    /* [note:  this 'sticky' handling is only sufficient for changing from
       grabber to engulfer or vice versa because engulfing by poly'd hero
       always ends immediately so won't be in effect during a polymorph] */
    if (!sticky && !u.uswallow && u.ustuck && sticks(youmonst.data))
        u.ustuck = 0;
    else if (sticky && !sticks(youmonst.data))
        uunstick();

    if (u.usteed) {
        if (touch_petrifies(u.usteed->data) && !Stone_resistance && rnl(3)) {
            pline("%s touch %s.", no_longer_petrify_resistant,
                  mon_nam(u.usteed));
            Sprintf(buf, "riding %s", an(u.usteed->data->mname));
            instapetrify(buf);
        }
        if (!can_ride(u.usteed))
            dismount_steed(DISMOUNT_POLY);
    }

    if (flags.verbose) {
        static const char use_thec[] = "Use the command #%s to %s.";
        static const char monsterc[] = "monster";

        if (can_breathe(youmonst.data))
            pline(use_thec, monsterc, "use your breath weapon");
        if (attacktype(youmonst.data, AT_SPIT))
            pline(use_thec, monsterc, "spit venom");
        if (is_nymph(youmonst.data))
            pline(use_thec, monsterc, "remove an iron ball");
        if (attacktype(youmonst.data, AT_GAZE))
            pline(use_thec, monsterc, "gaze at monsters");
        if (is_hider(youmonst.data))
            pline(use_thec, monsterc, "hide");
        if (is_were(youmonst.data))
            pline(use_thec, monsterc, "summon help");
        if (webmaker(youmonst.data))
            pline(use_thec, monsterc, "spin a web");
        if (u.umonnum == PM_GREMLIN)
            pline(use_thec, monsterc, "multiply in a fountain");
        if (u.umonnum == PM_LAVA_GREMLIN)
            pline(use_thec, monsterc, "multiply in a forge");
        if (is_unicorn(youmonst.data))
            pline(use_thec, monsterc, "use your horn");
        if (is_mind_flayer(youmonst.data))
            pline(use_thec, monsterc, "emit a mental blast");
        if (youmonst.data->msound == MS_SHRIEK) /* worthless, actually */
            pline(use_thec, monsterc, "shriek");
        if (is_vampire(youmonst.data))
            pline(use_thec, monsterc, "change shape");
        if (lays_eggs(youmonst.data) && flags.female &&
            !(youmonst.data == &mons[PM_GIANT_EEL]
                || youmonst.data == &mons[PM_ELECTRIC_EEL]))
            pline(use_thec, "sit",
                  eggs_in_water(youmonst.data) ?
                      "spawn in the water" : "lay an egg");
    }

    /* you now know what an egg of your type looks like */
    if (lays_eggs(youmonst.data)) {
        learn_egg_type(u.umonnum);
        /* make queen bees recognize killer bee eggs */
        learn_egg_type(egg_type_from_parent(u.umonnum, TRUE));
    }
    find_ac();
    if ((!Levitation && !u.ustuck && !Flying && is_pool_or_lava(u.ux, u.uy))
        || (Underwater && !Swimming))
        spoteffects(TRUE);
    if (Passes_walls && u.utrap
        && (u.utraptype == TT_INFLOOR || u.utraptype == TT_BURIEDBALL)) {
        if (u.utraptype == TT_INFLOOR) {
            pline_The("rock seems to no longer trap you.");
        } else {
            pline_The("buried ball is no longer bound to you.");
            buried_ball_to_freedom();
        }
        reset_utrap(TRUE);
    } else if (likes_lava(youmonst.data) && u.utrap
               && u.utraptype == TT_LAVA) {
        pline_The("%s now feels soothing.", hliquid("lava"));
        reset_utrap(TRUE);
    }
    if (amorphous(youmonst.data) || is_whirly(youmonst.data)
        || unsolid(youmonst.data) || nolimbs(youmonst.data)) {
        if (Punished) {
            You("%s out of the iron chain.",
                (nolimbs(youmonst.data)
                 && youmonst.data->msize > MZ_MEDIUM) ? "break" : "slip");
            unpunish();
        } else if (u.utrap && u.utraptype == TT_BURIEDBALL) {
            You("slip free of the buried ball and chain.");
            buried_ball_to_freedom();
        }
    }
    if (u.utrap && (u.utraptype == TT_WEB || u.utraptype == TT_BEARTRAP)
        && (amorphous(youmonst.data) || is_whirly(youmonst.data)
            || unsolid(youmonst.data) || (youmonst.data->msize <= MZ_SMALL
                                          && u.utraptype == TT_BEARTRAP))) {
        You("are no longer stuck in the %s.",
            u.utraptype == TT_WEB ? "web" : "bear trap");
        /* probably should burn webs too if PM_FIRE_ELEMENTAL */
        reset_utrap(TRUE);
    }
    if ((webmaker(youmonst.data)
         || maybe_polyd(is_drow(youmonst.data), Race_if(PM_DROW)))
        && u.utrap && u.utraptype == TT_WEB) {
        You("orient yourself on the web.");
        reset_utrap(TRUE);
    }
    check_strangling(TRUE, TRUE); /* maybe start strangling */

    context.botl = 1;
    vision_full_recalc = 1;
    see_monsters();
    (void) encumber_msg();

    retouch_equipment(2);
    /* this might trigger a recursive call to polymon() [stone golem
       wielding cockatrice corpse and hit by stone-to-flesh, becomes
       flesh golem above, now gets transformed back into stone golem] */
    if (!uarmg)
        selftouch(no_longer_petrify_resistant);
    return 1;
}

/* dropx() jacket for break_armor() */
STATIC_OVL void
dropp(obj)
struct obj *obj;
{
    struct obj *otmp;

    /*
     * Dropping worn armor while polymorphing might put hero into water
     * (loss of levitation boots or water walking boots that the new
     * form can't wear), where emergency_disrobe() could remove it from
     * inventory.  Without this, dropx() could trigger an 'object lost'
     * panic.  Right now, boots are the only armor which might encounter
     * this situation, but handle it for all armor.
     *
     * Hypothetically, 'obj' could have merged with something (not
     * applicable for armor) and no longer be a valid pointer, so scan
     * inventory for it instead of trusting obj->where.
     */
    for (otmp = invent; otmp; otmp = otmp->nobj) {
        if (otmp == obj) {
            dropx(obj);
            break;
        }
    }
}

STATIC_OVL void
break_armor()
{
    register struct obj *otmp;
    boolean druid_forms = (Role_if(PM_DRUID)
                           && all_druid_forms(monsndx(youmonst.data)));

    if (breakarm(&youmonst)) {
        if ((otmp = uarm) != 0) {
            if (donning(otmp))
                cancel_don();
            /* for gold dragon-scaled armor, we don't want Armor_gone()
               to report that it stops shining _after_ we've been told
               that it is destroyed */
            if (otmp->lamplit)
                end_burn(otmp, FALSE);
            if (youmonst.data->msize >= MZ_HUGE
                && (otmp->otyp == LARGE_SPLINT_MAIL
                    || otmp->oartifact == ART_ARMOR_OF_RETRIBUTION)) {
                if (humanoid(youmonst.data)) {
                    if (otmp->otyp == LARGE_SPLINT_MAIL) {
                        ; /* nothing bad happens, armor is still worn */
                    } else { /* Armor of Retribution */
                        Your("armor falls off!");
                        (void) Armor_gone();
                        dropp(otmp);
                    }
                } else if (!humanoid(youmonst.data)) {
                    Your("armor falls off!");
                    (void) Armor_gone();
                    dropp(otmp);
                }
            } else if (youmonst.data->msize <= MZ_LARGE
                       && (otmp->otyp == LARGE_SPLINT_MAIL
                           || otmp->oartifact == ART_ARMOR_OF_RETRIBUTION)) {
                Your("armor falls off!");
                (void) Armor_gone();
                dropp(otmp);
            } else {
                You("break out of your armor!");
                exercise(A_STR, FALSE);
                (void) Armor_gone();
                useup(otmp);
            }
        }
        if ((otmp = uarmc) != 0) {
            if (otmp->oartifact) {
                Your("%s falls off!", cloak_simple_name(otmp));
                (void) Cloak_off();
                dropp(otmp);
            } else {
                if (youmonst.data->msize >= MZ_HUGE
                    && humanoid(youmonst.data)
                    && otmp->otyp == CHROMATIC_DRAGON_SCALES) {
                    ; /* nothing bad happens, armor is still worn */
                } else {
                    Your("%s tears apart!", cloak_simple_name(otmp));
                    (void) Cloak_off();
                    useup(otmp);
                }
            }
        }
        if ((otmp = uarmu)) {
            Your("shirt rips to shreds!");
            (void) Shirt_off();
            useup(otmp);
        }
    } else if (sliparm(&youmonst)) {
        if (((otmp = uarm) != 0) && (racial_exception(&youmonst, otmp) < 1)) {
            if (donning(otmp))
                cancel_don();
            Your("armor falls around you!");
            /* [note: _gone() instead of _off() dates to when life-saving
               could force fire resisting armor back on if hero burned in
               hell (3.0, predating Gehennom); the armor isn't actually
               gone here but also isn't available to be put back on] */
            (void) Armor_gone();
            dropp(otmp);
        }
        if ((otmp = uarmc) != 0) {
            if (is_whirly(youmonst.data))
                Your("%s falls, unsupported!", cloak_simple_name(otmp));
            else
                You("shrink out of your %s!", cloak_simple_name(otmp));
            (void) Cloak_off();
            dropp(otmp);
        }
        if ((otmp = uarmu) != 0) {
            if (is_whirly(youmonst.data))
                You("seep right through your shirt!");
            else
                You("become much too small for your shirt!");
            (void) Shirt_off();
            dropp(otmp);
        }
    /* catch what breakarm() and sliparm() doesn't handle */
    } else if (youmonst.data->msize <= MZ_LARGE) {
        if ((otmp = uarm) != 0 && otmp->otyp == LARGE_SPLINT_MAIL) {
            if (donning(otmp))
                cancel_don();
            Your("armor falls around you!");
            (void) Armor_gone();
            dropp(otmp);
        }
    }
    if (has_horns(youmonst.data) && !druid_forms) {
        if ((otmp = uarmh) != 0) {
            if (is_flimsy(otmp) && !donning(otmp)) {
                char hornbuf[BUFSZ];

                /* Future possibilities: This could damage/destroy helmet */
                Sprintf(hornbuf, "horn%s", plur(num_horns(youmonst.data)));
                Your("%s %s through %s.", hornbuf, vtense(hornbuf, "pierce"),
                     yname(otmp));
            } else {
                if (donning(otmp))
                    cancel_don();
                Your("%s falls to the %s!", helm_simple_name(otmp),
                     surface(u.ux, u.uy));
                (void) Helmet_off();
                dropp(otmp);
            }
        }
    }
    if (nohands(youmonst.data) || verysmall(youmonst.data)
        || is_ent(youmonst.data)) {
        if (!druid_forms && (otmp = uarmg) != 0) {
            if (donning(otmp))
                cancel_don();
            /* Drop weapon along with gloves */
            if (uarmg->oartifact == ART_HAND_OF_VECNA) {
                if (uwep) {
                    You("drop your weapon!");
                    drop_weapon(0);
                }
            } else {
                You("drop your gloves%s!", uwep ? " and weapon" : "");
                drop_weapon(0);
                (void) Gloves_off();
                /* Glib manipulation (ends immediately) handled by Gloves_off */
                dropp(otmp);
            }
        }
        if ((otmp = uarms) != 0) {
            if (druid_forms && !is_bracer(uarms))
                You("can no longer hold your shield!");
            else if (!druid_forms)
                You("can no longer %s!",
                    is_bracer(uarms) ? "wear your bracers"
                                     : "hold your shield");
            if (druid_forms && is_bracer(uarms)) {
                ; /* do nothing */
            } else {
                if (otmp->lamplit)
                    end_burn(otmp, FALSE);
                (void) Shield_off();
                if (!druid_forms)
                    dropp(otmp);
            }
        }
        if (!druid_forms && (otmp = uarmh) != 0) {
            if (donning(otmp))
                cancel_don();
            Your("%s falls to the %s!", helm_simple_name(otmp),
                 surface(u.ux, u.uy));
            (void) Helmet_off();
            dropp(otmp);
        }
    }
    if ((nohands(youmonst.data) || verysmall(youmonst.data)
         || slithy(youmonst.data) || racial_centaur(&youmonst)
         || racial_tortle(&youmonst) || is_ent(youmonst.data)
         || is_satyr(youmonst.data)) && !druid_forms) {
        if ((otmp = uarmf) != 0) {
            if (donning(otmp))
                cancel_don();
            if (is_whirly(youmonst.data))
                Your("boots fall away!");
            else
                Your("boots %s off your feet!",
                     verysmall(youmonst.data) ? "slide" : "are pushed");
            (void) Boots_off();
            dropp(otmp);
        }
    }
}

STATIC_OVL void
drop_weapon(alone)
int alone;
{
    struct obj *otmp;
    const char *what, *which, *whichtoo;
    boolean candropwep, candropswapwep, updateinv = TRUE;
    boolean druid_forms = (Role_if(PM_DRUID)
                           && all_druid_forms(monsndx(youmonst.data)));

    if (uwep) {
        /* !alone check below is currently superfluous but in the
         * future it might not be so if there are monsters which cannot
         * wear gloves but can wield weapons
         */
        if (!alone || cantwield(youmonst.data)) {
            candropwep = canletgo(uwep, "");
            candropswapwep = !u.twoweap || canletgo(uswapwep, "");
            if (alone) {
                what = (candropwep && candropswapwep) ? "drop" : "release";
                which = is_sword(uwep) ? "sword" : weapon_descr(uwep);
                if (u.twoweap) {
                    whichtoo =
                        is_sword(uswapwep) ? "sword" : weapon_descr(uswapwep);
                    if (strcmp(which, whichtoo))
                        which = "weapon";
                }
                if (uwep->quan != 1L || u.twoweap)
                    which = makeplural(which);

                You("find you must %s %s %s!",
                    druid_forms ? "release" : what,
                    the_your[!!strncmp(which, "corpse", 6)], which);
            }
            /* if either uwep or wielded uswapwep is flagged as 'in_use'
               then don't drop it or explicitly update inventory; leave
               those actions to caller (or caller's caller, &c) */
            if (u.twoweap) {
                otmp = uswapwep;
                uswapwepgone();
                if (otmp->in_use)
                    updateinv = FALSE;
                else if (candropswapwep && !druid_forms)
                    dropx(otmp);
            }
            otmp = uwep;
            uwepgone();
            if (otmp->in_use)
                updateinv = FALSE;
            else if (candropwep  && !druid_forms)
                dropx(otmp);
            /* [note: dropp vs dropx -- if heart of ahriman is wielded, we
               might be losing levitation by dropping it; but that won't
               happen until the drop, unlike Boots_off() dumping hero into
               water and triggering emergency_disrobe() before dropx()] */

            if (updateinv)
                update_inventory();
        } else if ((u.twoweap && (bimanual(uwep) || bimanual(uswapwep)))
                   || (bimanual(uwep) && uarms && !is_bracer(uarms))) {
            otmp = bimanual(uwep) ? uwep : uswapwep;
            which = is_sword(otmp) ? "sword" : weapon_descr(otmp);
            if (otmp->quan != 1L)
                which = makeplural(which);
            if (u.twoweap && bimanual(uwep) && bimanual(uswapwep))
                which = "weapons";
            You("can no longer wield your %s with one %s.", which, body_part(HAND));
            if (u.twoweap) {
                /* avoid redundant/incorrect untwoweapon feedback */
                u.twoweap = 0;
                untwoweapon();
                You("switch to your primary weapon.");
            /* try to avoid unwielding a cursed weapon */
            } else if (!(uwep->cursed) || uarms->cursed) {
                setuwep((struct obj *) 0);
                You("let it go.");
            } else {
                Shield_off();
                You("let go of your shield.");
            }
            update_inventory();
        /* must follow the bimanual check, since that never leaves you
           two-weaponing but this block might leave you wielding a
           two-hander and wearing a shield */
        } else if (!could_twoweap(youmonst.data)) {
            untwoweapon();
        }
    }
}

void
rehumanize()
{
    boolean was_flying = (Flying != 0);

    /* You can't revert back while unchanging */
    if (Unchanging) {
        if (u.mh < 1) {
            You("die...");
            killer.format = NO_KILLER_PREFIX;
            Strcpy(killer.name, "killed while stuck in creature form");
            done(DIED);
            /* should the player be wearing an amulet of life saving, allow them to
             * be saved from whatever killed them, but DON'T rehumanize -- if the
             * player was resurrected with intrinsic unchanging, they shouldn't
             * be able to regain their original form */
            return;
        } else if (uamul && uamul->otyp == AMULET_OF_UNCHANGING) {
            Your("%s %s!", simpleonames(uamul), otense(uamul, "fail"));
            uamul->dknown = 1;
            makeknown(AMULET_OF_UNCHANGING);
        }
    }

    if (emits_light(youmonst.data))
        del_light_source(LS_MONSTER, monst_to_any(&youmonst));
    polyman("return to %s form!", urace.adj);
    break_armor();
    drop_weapon(1);

    if (u.uhp < 1) {
        /* can only happen if some bit of code reduces u.uhp
           instead of u.mh while poly'd */
        Your("old form was not healthy enough to survive.");
        Sprintf(killer.name, "reverting to unhealthy %s form", urace.adj);
        killer.format = KILLED_BY;
        done(DIED);
    }
    nomul(0);

    context.botl = 1;
    vision_full_recalc = 1;
    (void) encumber_msg();
    if (was_flying && !Flying && u.usteed)
        You("and %s return gently to the %s.",
            mon_nam(u.usteed), surface(u.ux, u.uy));
    retouch_equipment(2);
    if (!uarmg)
        selftouch(no_longer_petrify_resistant);
}

int
dobreathe()
{
    struct attack *mattk;

    if (Strangled) {
        You_cant("breathe.  Sorry.");
        return 0;
    }
    if (u.uen < 15) {
        You("don't have enough energy to breathe!");
        return 0;
    }
    u.uen -= 15;
    context.botl = 1;

    if (!getdir((char *) 0))
        return 0;

    mattk = attacktype_fordmg(youmonst.data, AT_BREA, AD_ANY);
    if (!mattk)
        impossible("bad breath attack?"); /* mouthwash needed... */
    else if (!u.dx && !u.dy && !u.dz)
        ubreatheu(mattk);
    else
        buzz((int) (22 + mattk->adtyp - 1), (int) mattk->damn, u.ux, u.uy,
             u.dx, u.dy);
    return 1;
}

int
dospit()
{
    struct obj *otmp;
    struct attack *mattk;

    if (!getdir((char *) 0))
        return 0;
    mattk = attacktype_fordmg(youmonst.data, AT_SPIT, AD_ANY);
    if (!mattk) {
        impossible("bad spit attack?");
    } else {
        switch (mattk->adtyp) {
        case AD_BLND:
        case AD_DRST:
            otmp = mksobj(BLINDING_VENOM, TRUE, FALSE);
            break;
        default:
            impossible("bad attack type in dospit");
            /*FALLTHRU*/
        case AD_DRCO:
            otmp = mksobj(BARBED_NEEDLE, TRUE, FALSE);
            break;
        case AD_ACID:
            otmp = mksobj(ACID_VENOM, TRUE, FALSE);
            break;
        case AD_COLD:
            otmp = mksobj(SNOWBALL, TRUE, FALSE);
            break;
        case AD_WEBS:
            otmp = mksobj(BALL_OF_WEBBING, TRUE, FALSE);
            break;
        }
        otmp->spe = 1; /* to indicate it's yours */
        throwit(otmp, 0L, FALSE);
    }
    return 1;
}

int
doremove()
{
    if (!Punished) {
        if (u.utrap && u.utraptype == TT_BURIEDBALL) {
            pline_The("ball and chain are buried firmly in the %s.",
                      surface(u.ux, u.uy));
            return 0;
        }
        You("are not chained to anything!");
        return 0;
    }
    unpunish();
    return 1;
}

int
dospinweb()
{
    xchar x = u.ux, y = u.uy;
    struct trap *ttmp = t_at(x, y);
    /* disallow webs on water, lava, air & cloud */
    boolean reject_terrain = (is_pool_or_lava(x, y) || IS_AIR(levl[x][y].typ)
                              || is_puddle(x, y) || is_sewage(x, y));

    /* [at the time this was written, it was not possible to be both a
       webmaker and a flyer, but with the advent of amulet of flying that
       became a possibility; at present hero can spin a web while flying] */
    if (Levitation || reject_terrain) {
        You("must be on %s ground to spin a web.",
            reject_terrain ? "solid" : "the");
        return 0;
    }
    if (u.uswallow) {
        You("release web fluid inside %s.", mon_nam(u.ustuck));
        if (is_swallower(u.ustuck->data)) {
            expels(u.ustuck, u.ustuck->data, TRUE);
            return 0;
        }
        if (is_whirly(u.ustuck->data)) {
            int i;

            for (i = 0; i < NATTK; i++)
                if (u.ustuck->data->mattk[i].aatyp == AT_ENGL)
                    break;
            if (i == NATTK)
                impossible("Swallower has no engulfing attack?");
            else {
                char sweep[30];

                sweep[0] = '\0';
                switch (u.ustuck->data->mattk[i].adtyp) {
                case AD_FIRE:
                    Strcpy(sweep, "ignites and ");
                    break;
                case AD_ELEC:
                    Strcpy(sweep, "fries and ");
                    break;
                case AD_COLD:
                    Strcpy(sweep, "freezes, shatters and ");
                    break;
                }
                pline_The("web %sis swept away!", sweep);
            }
            return 0;
        } /* default: a nasty jelly-like creature */
        pline_The("web dissolves into %s.", mon_nam(u.ustuck));
        return 0;
    }
    if (u.utrap) {
        You("cannot spin webs while stuck in a trap.");
        return 0;
    }
    exercise(A_DEX, TRUE);
    if (ttmp) {
        switch (ttmp->ttyp) {
        case PIT:
        case SPIKED_PIT:
            You("spin a web, covering up the pit.");
            deltrap(ttmp);
            bury_objs(x, y);
            newsym(x, y);
            return 1;
        case SQKY_BOARD:
            pline_The("squeaky board is muffled.");
            deltrap(ttmp);
            newsym(x, y);
            return 1;
        case TELEP_TRAP:
        case LEVEL_TELEP:
        case MAGIC_PORTAL:
        case VIBRATING_SQUARE:
            Your("webbing vanishes!");
            return 0;
        case WEB:
            You("make the web thicker.");
            return 1;
        case HOLE:
        case TRAPDOOR:
            You("web over the %s.",
                (ttmp->ttyp == TRAPDOOR) ? "trap door" : "hole");
            deltrap(ttmp);
            newsym(x, y);
            return 1;
        case ROLLING_BOULDER_TRAP:
            You("spin a web, jamming the trigger.");
            deltrap(ttmp);
            newsym(x, y);
            return 1;
        case SPEAR_TRAP:
            You("spin a web, jamming the mechanism.");
            deltrap(ttmp);
            newsym(x, y);
            return 1;
        case ARROW_TRAP:
        case BOLT_TRAP:
        case DART_TRAP:
        case BEAR_TRAP:
        case ROCKTRAP:
        case FIRE_TRAP:
        case ICE_TRAP:
        case LANDMINE:
        case SLP_GAS_TRAP:
        case RUST_TRAP:
        case MAGIC_TRAP:
        case ANTI_MAGIC:
        case POLY_TRAP:
        case MAGIC_BEAM_TRAP:
            You("have triggered a trap!");
            dotrap(ttmp, 0);
            return 1;
        default:
            impossible("Webbing over trap type %d?", ttmp->ttyp);
            return 0;
        }
    } else if (On_stairs(x, y)) {
        /* cop out: don't let them hide the stairs */
        Your("web fails to impede access to the %s.",
             (levl[x][y].typ == STAIRS) ? "stairs" : "ladder");
        return 1;
    }
    ttmp = maketrap(x, y, WEB);
    if (ttmp) {
        You("spin a web.");
        ttmp->madeby_u = 1;
        feeltrap(ttmp);
        if (*in_rooms(x, y, SHOPBASE))
            add_damage(x, y, SHOP_WEB_COST);
    }
    return 1;
}

int
dosummon()
{
    int placeholder;
    if (u.uen < 10) {
        You("lack the energy to send forth a call for help!");
        return 0;
    }
    u.uen -= 10;
    context.botl = 1;

    You("call upon your brethren for help!");
    exercise(A_WIS, TRUE);
    if (!were_summon(youmonst.data, TRUE, &placeholder, (char *) 0))
        pline("But none arrive.");
    return 1;
}

int
dogaze()
{
    register struct monst *mtmp;
    int looked = 0;
    char qbuf[QBUFSZ];
    int i;
    uchar adtyp = 0;

    for (i = 0; i < NATTK; i++) {
        if (youmonst.data->mattk[i].aatyp == AT_GAZE) {
            adtyp = youmonst.data->mattk[i].adtyp;
            break;
        }
    }
    if (adtyp != AD_CONF && adtyp != AD_FIRE) {
        impossible("gaze attack %d?", adtyp);
        return 0;
    }

    if (Blind) {
        You_cant("see anything to gaze at.");
        return 0;
    } else if (Hallucination) {
        You_cant("gaze at anything you can see.");
        return 0;
    }
    if (u.uen < 15) {
        You("lack the energy to use your special gaze!");
        return 0;
    }
    u.uen -= 15;
    context.botl = 1;

    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp))
            continue;
        if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my)) {
            looked++;
            if (Invis && !mon_prop(mtmp, SEE_INVIS)) {
                pline("%s seems not to notice your gaze.", Monnam(mtmp));
            } else if (mtmp->minvis && !See_invisible) {
                You_cant("see where to gaze at %s.", Monnam(mtmp));
            } else if (M_AP_TYPE(mtmp) == M_AP_FURNITURE
                       || M_AP_TYPE(mtmp) == M_AP_OBJECT) {
                looked--;
                continue;
            } else if (flags.safe_dog && mtmp->mtame && !Confusion) {
                You("avoid gazing at %s.", y_monnam(mtmp));
            } else {
                if (flags.confirm && mtmp->mpeaceful && !Confusion) {
                    Sprintf(qbuf, "Really %s %s?",
                            (adtyp == AD_CONF) ? "confuse" : "attack",
                            mon_nam(mtmp));
                    if (yn(qbuf) != 'y')
                        continue;
                }
                setmangry(mtmp, TRUE);
                if (!mtmp->mcanmove || mtmp->mstun || mtmp->msleeping
                    || !mtmp->mcansee || !haseyes(mtmp->data)) {
                    looked--;
                    continue;
                }
                /* No reflection check for consistency with when a monster
                 * gazes at *you*--only medusa gaze gets reflected then.
                 */
                if (adtyp == AD_CONF) {
                    if (!mtmp->mconf)
                        Your("gaze confuses %s!", mon_nam(mtmp));
                    else
                        pline("%s is getting more and more confused.",
                              Monnam(mtmp));
                    mtmp->mconf = 1;
                } else if (adtyp == AD_FIRE) {
                    int dmg = d(2, 6), lev = (int) u.ulevel;

                    You("attack %s with a fiery gaze!", mon_nam(mtmp));
                    if (resists_fire(mtmp) || defended(mtmp, AD_FIRE)) {
                        pline_The("fire doesn't burn %s!", mon_nam(mtmp));
                        dmg = 0;
                    }
                    if (lev > rn2(20))
                        (void) destroy_mitem(mtmp, SCROLL_CLASS, AD_FIRE);
                    if (lev > rn2(20))
                        (void) destroy_mitem(mtmp, POTION_CLASS, AD_FIRE);
                    if (lev > rn2(25))
                        (void) destroy_mitem(mtmp, SPBOOK_CLASS, AD_FIRE);
                    if (dmg)
                        damage_mon(mtmp, dmg, AD_FIRE);
                    if (DEADMONSTER(mtmp))
                        killed(mtmp);
                }
                /* For consistency with passive() in uhitm.c, this only
                 * affects you if the monster is still alive.
                 */
                if (DEADMONSTER(mtmp))
                    continue;

                if (mtmp->data == &mons[PM_FLOATING_EYE] && !mtmp->mcan) {
                    if (!Free_action) {
                        You("are frozen by %s gaze!",
                            s_suffix(mon_nam(mtmp)));
                        nomul((u.ulevel > 6 || rn2(4))
                                  ? -d((int) mtmp->m_lev + 1,
                                       (int) mtmp->data->mattk[0].damd)
                                  : -200);
                        multi_reason = "frozen by a monster's gaze";
                        nomovemsg = 0;
                        return 1;
                    } else
                        You("stiffen momentarily under %s gaze.",
                            s_suffix(mon_nam(mtmp)));
                }
                /* Technically this one shouldn't affect you at all because
                 * the Medusa gaze is an active monster attack that only
                 * works on the monster's turn, but for it to *not* have an
                 * effect would be too weird.
                 */
                if (mtmp->data == &mons[PM_MEDUSA] && !mtmp->mcan) {
                    pline("Gazing at the awake %s is not a very good idea.",
                          l_monnam(mtmp));
                    /* as if gazing at a sleeping anything is fruitful... */
                    You("turn to stone...");
                    killer.format = KILLED_BY;
                    Strcpy(killer.name, "deliberately meeting Medusa's gaze");
                    done(STONING);
                }
            }
        }
    }
    if (!looked)
        You("gaze at no place in particular.");
    return 1;
}

int
dohide()
{
    boolean ismimic = youmonst.data->mlet == S_MIMIC,
            on_ceiling = is_clinger(youmonst.data) || Flying;

    /* can't hide while being held (or holding) or while trapped
       (except for floor hiders [trapper or mimic] in pits) */
    if (u.ustuck || (u.utrap && (u.utraptype != TT_PIT || on_ceiling))) {
        You_cant("hide while you're %s.",
                 !u.ustuck ? "trapped"
                   : u.uswallow ? (is_swallower(u.ustuck->data) ? "swallowed"
                                                                : "engulfed")
                     : !sticks(youmonst.data) ? "being held"
                       : (humanoid(u.ustuck->data) ? "holding someone"
                                                   : "holding that creature"));
        if (u.uundetected
            || (ismimic && U_AP_TYPE != M_AP_NOTHING)) {
            u.uundetected = 0;
            youmonst.m_ap_type = M_AP_NOTHING;
            newsym(u.ux, u.uy);
        }
        return 0;
    }
    /* note: the eel and hides_under cases are hypothetical;
       such critters aren't offered the option of hiding via #monster */
    if (youmonst.data->mlet == S_EEL
        && !(is_pool(u.ux, u.uy) || is_puddle(u.ux, u.uy))) {
        if (IS_FOUNTAIN(levl[u.ux][u.uy].typ))
            The("fountain is not deep enough to hide in.");
        else
            There("is no %s to hide in here.", hliquid("water"));
        u.uundetected = 0;
        return 0;
    }
    if (youmonst.data == &mons[PM_GIANT_LEECH] && !is_sewage(u.ux, u.uy)) {
        if (IS_FOUNTAIN(levl[u.ux][u.uy].typ)
            || IS_PUDDLE(levl[u.ux][u.uy].typ))
            The("water here is too clean to hide in.");
        else
            There("isn't any raw sewage to hide in here.");
        u.uundetected = 0;
        return 0;
    }
    if (hides_under(youmonst.data) && !concealed_spot(u.ux, u.uy)) {
        There("is nothing to hide under here.");
        u.uundetected = 0;
        return 0;
    }
    /* Planes of Air and Water */
    if (on_ceiling && !has_ceiling(&u.uz)) {
        There("is nowhere to hide above you.");
        u.uundetected = 0;
        return 0;
    }
    if ((is_hider(youmonst.data) && !Flying) /* floor hider */
        && (Is_airlevel(&u.uz) || Is_waterlevel(&u.uz))) {
        There("is nowhere to hide beneath you.");
        u.uundetected = 0;
        return 0;
    }
    /* TODO? inhibit floor hiding at furniture locations, or
     * else make youhiding() give smarter messages at such spots.
     */

    if (u.uundetected || (ismimic && U_AP_TYPE != M_AP_NOTHING)) {
        youhiding(FALSE, 1); /* "you are already hiding" */
        return 0;
    }

    if (ismimic) {
        /* should bring up a dialog "what would you like to imitate?" */
        youmonst.m_ap_type = M_AP_OBJECT;
        youmonst.mappearance = STRANGE_OBJECT;
    } else
        u.uundetected = 1;
    newsym(u.ux, u.uy);
    youhiding(FALSE, 0); /* "you are now hiding" */
    return 1;
}

int
dodarkness()
{
    int energy = 0;
    const char *fail_invoke = 0;
    boolean invoke_darkness = TRUE;

    energy = 10;
    if (Confusion || Stunned)
        fail_invoke = "are unable";
    if (u.uhunger <= 10)
        fail_invoke = "are too weak from hunger";
    else if (ACURR(A_STR) < 4)
        fail_invoke = "lack the strength";
    else if (energy > u.uen)
        fail_invoke = "lack the energy";

    if (fail_invoke) {
        You("%s to invoke an aura of darkness.",
            fail_invoke);
        invoke_darkness = FALSE;
        return 0;
    }

    if (invoke_darkness) {
        exercise(A_WIS, TRUE);
        u.uen -= energy;
        context.botl = 1;
        You("invoke an aura of darkness.");
        litroom(FALSE, TRUE, NULL, u.ux, u.uy);
    }

    return 1;
}

int
toggleshell()
{
    boolean was_blind = Blind, was_hiding = Hidinshell;

    /* maximum of 200 turns our hero can stay inside their shell,
       and then 300-400 turns before they can hide in it again
       after emerging from it */
    u.uinshell = was_hiding ? -rn1(100, 300) : 200;

    if (!was_hiding)
        HHalf_physical_damage |= FROMOUTSIDE;
    else
        HHalf_physical_damage &= ~FROMOUTSIDE;

    find_ac();
    context.botl = 1;
    if (was_blind ^ Blind)
        toggle_blindness();

    return 1;
}

int
dopoly()
{
    struct permonst *savedat = youmonst.data;

    if (is_vampire(youmonst.data)) {
        polyself(2);
        if (savedat != youmonst.data) {
            You("transform into %s.", an(youmonst.data->mname));
            newsym(u.ux, u.uy);
        }
    }
    return 1;
}

int
domindblast()
{
    struct monst *mtmp, *nmon;

    if (u.uen < 10) {
        You("concentrate but lack the energy to maintain doing so.");
        return 0;
    }
    u.uen -= 10;
    context.botl = 1;

    You("concentrate.");
    pline("A wave of psychic energy pours out.");
    for (mtmp = fmon; mtmp; mtmp = nmon) {
        int u_sen;

        nmon = mtmp->nmon;
        if (DEADMONSTER(mtmp))
            continue;
        if (distu(mtmp->mx, mtmp->my) > BOLT_LIM * BOLT_LIM)
            continue;
        if (mtmp->mpeaceful)
            continue;
        u_sen = has_telepathy(mtmp) && !mtmp->mcansee;
        if (u_sen || (has_telepathy(mtmp) && rn2(2)) || !rn2(10)) {
            You("lock in on %s %s.", s_suffix(mon_nam(mtmp)),
                u_sen ? "telepathy"
                      : has_telepathy(mtmp) ? "latent telepathy" : "mind");
            damage_mon(mtmp, rnd(15), AD_DRIN);
            if (DEADMONSTER(mtmp))
                killed(mtmp);
        }
    }
    return 1;
}

void
uunstick()
{
    if (!u.ustuck) {
        impossible("uunstick: no ustuck?");
        return;
    }
    pline("%s is no longer in your clutches.", Monnam(u.ustuck));
    u.ustuck = 0;
}

void
skinback(silently)
boolean silently;
{
    if (uskin) {
        struct obj **slot = (Is_dragon_scales(uskin) ? &uarmc : &uarm);
        int old_light = arti_light_radius(uskin);

        if (!silently)
            Your("skin returns to its original form.");
        *slot = uskin;
        uskin = (struct obj *) 0;
        /* undo save/restore hack */
        (*slot)->owornmask &= ~I_SPECIAL;

        if (artifact_light(uarm))
            maybe_adjust_light(uarm, old_light);
    }
}

const char *
mbodypart(mon, part)
struct monst *mon;
int part;
{
    static NEARDATA const char
        *humanoid_parts[] = { "arm",       "eye",  "face",         "finger",
                              "fingertip", "foot", "hand",         "handed",
                              "head",      "leg",  "light headed", "neck",
                              "spine",     "toe",  "hair",         "blood",
                              "lung",      "nose", "stomach",      "skin" },
        *jelly_parts[] = { "pseudopod", "dark spot", "front",
                           "pseudopod extension", "pseudopod extremity",
                           "pseudopod root", "grasp", "grasped",
                           "cerebral area", "lower pseudopod", "viscous",
                           "middle", "surface", "pseudopod extremity",
                           "ripples", "juices", "surface", "sensor",
                           "stomach", "surface" },
        *animal_parts[] = { "forelimb",  "eye",           "face",
                            "foreclaw",  "claw tip",      "rear claw",
                            "foreclaw",  "clawed",        "head",
                            "rear limb", "light headed",  "neck",
                            "spine",     "rear claw tip", "fur",
                            "blood",     "lung",          "nose",
                            "stomach",   "hide" },
        *bird_parts[] = { "wing",     "eye",  "face",         "wing",
                          "wing tip", "foot", "wing",         "winged",
                          "head",     "leg",  "light headed", "neck",
                          "spine",    "toe",  "feathers",     "blood",
                          "lung",     "bill", "stomach",      "skin" },
        *horse_parts[] = { "foreleg",  "eye",           "face",
                           "forehoof", "hoof tip",      "rear hoof",
                           "forehoof", "hooved",        "head",
                           "rear leg", "light headed",  "neck",
                           "backbone", "rear hoof tip", "mane",
                           "blood",    "lung",          "nose",
                           "stomach",  "hide" },
        *sphere_parts[] = { "appendage", "optic nerve", "body", "tentacle",
                            "tentacle tip", "lower appendage", "tentacle",
                            "tentacled", "body", "lower tentacle",
                            "rotational", "equator", "body",
                            "lower tentacle tip", "cilia", "life force",
                            "retina", "olfactory nerve", "interior", "skin" },
        *fungus_parts[] = { "mycelium", "visual area", "front",
                            "hypha",    "hypha",       "root",
                            "strand",   "stranded",    "cap area",
                            "rhizome",  "sporulated",  "stalk",
                            "root",     "rhizome tip", "spores",
                            "juices",   "gill",        "gill",
                            "interior", "spores" },
        *vortex_parts[] = { "region",        "eye",           "front",
                            "minor current", "minor current", "lower current",
                            "swirl",         "swirled",       "central core",
                            "lower current", "addled",        "center",
                            "currents",      "edge",          "currents",
                            "life force",    "center",        "leading edge",
                            "interior",      "exterior" },
        *snake_parts[] = { "vestigial limb", "eye", "face", "large scale",
                           "large scale tip", "rear region", "scale gap",
                           "scale gapped", "head", "rear region",
                           "light headed", "neck", "length", "rear scale",
                           "scales", "blood", "lung", "forked tongue",
                           "stomach", "scales" },
        *worm_parts[] = { "anterior segment", "light sensitive cell",
                          "clitellum", "setae", "setae", "posterior segment",
                          "segment", "segmented", "anterior segment",
                          "posterior", "over stretched", "clitellum",
                          "length", "posterior setae", "setae", "blood",
                          "skin", "prostomium", "stomach", "skin" },
        *spider_parts[] = { "pedipalp", "eye", "face", "pedipalp", "pedipalp",
                            "tarsus", "pedipalp", "palped", "cephalothorax",
                            "leg", "spun out", "cephalothorax", "abdomen",
                            "claw", "hair", "hemolymph", "book lung",
                            "labrum", "digestive tract", "cuticle" },
        *fish_parts[] = { "fin", "eye", "premaxillary", "pelvic axillary",
                          "pelvic fin", "anal fin", "pectoral fin", "finned",
                          "head", "peduncle", "played out", "gills",
                          "dorsal fin", "caudal fin", "scales", "blood",
                          "gill", "nostril", "stomach", "scales" },
        *tree_parts[] = { "branch",    "eye",      "face",     "twig",
                          "bud",       "taproot",  "claw",     "clawed",
                          "crown",     "root",     "addled",   "neck",
                          "bole",      "bud",      "leaves",   "sap",
                          "stomata",   "nose",     "interior", "bark" },
       *plant_parts[] = { "branch",     "visual area", "front",  "leaf",
                          "leaftip",    "taproot",     "twig",   "twigged",
                          "apical bud", "root",        "addled", "stem",
                          "stem",       "root-tip",    "leaves", "sap",
                          "stomata",    "stomata",     "xylem",  "epidermis" };

    if (!mon) {
        impossible("body part of null monster");
        return humanoid_parts[part];
    }

    struct permonst *mptr = mon->data;

    /* some special cases */
    if (mptr->mlet == S_DOG || mptr->mlet == S_FELINE
        || mptr->mlet == S_RODENT || mptr == &mons[PM_OWLBEAR]
        || mptr == &mons[PM_CAVE_BEAR]
        || mptr == &mons[PM_GRIZZLY_BEAR]) {
        switch (part) {
        case HAND:
            return "paw";
        case HANDED:
            return "pawed";
        case FOOT:
            return "rear paw";
        case ARM:
        case LEG:
            return horse_parts[part]; /* "foreleg", "rear leg" */
        default:
            break; /* for other parts, use animal_parts[] below */
        }
    } else if (mptr->mlet == S_YETI) { /* excl. owlbear due to 'if' above */
        /* opposable thumbs, hence "hands", "arms", "legs", &c */
        return humanoid_parts[part]; /* yeti/sasquatch, monkey/ape */
    }
    if ((part == HAND || part == HANDED)
        && ((humanoid(mptr) && (has_claws(mptr) || has_claws_undead(mptr)))
            || (mon == &youmonst
                && (Race_if(PM_DEMON) || Race_if(PM_ILLITHID)
                    || Race_if(PM_TORTLE) || Race_if(PM_DRAUGR)))))
        return (part == HAND) ? "claw" : "clawed";
    if ((mptr == &mons[PM_MUMAK] || mptr == &mons[PM_MASTODON]
         || mptr == &mons[PM_WOOLLY_MAMMOTH])
        && part == NOSE)
        return "trunk";
    if (mptr == &mons[PM_SHARK] && part == HAIR)
        return "skin"; /* sharks don't have scales */
    if (is_skeleton(mptr) && part == SKIN)
        return "bones"; /* skeletons don't have skin */
    if ((mptr == &mons[PM_JELLYFISH] || mptr == &mons[PM_KRAKEN])
        && (part == ARM || part == FINGER || part == HAND || part == FOOT
            || part == TOE || part == FINGERTIP))
        return "tentacle";
    if (mptr == &mons[PM_FLOATING_EYE] && part == EYE)
        return "cornea";
    if (humanoid(mptr) && (part == ARM || part == FINGER || part == FINGERTIP
                           || part == HAND || part == HANDED))
        return humanoid_parts[part];
    if (mptr->mlet == S_COCKATRICE)
        return (part == HAIR) ? snake_parts[part] : bird_parts[part];
    if (is_bird(mptr))
        return bird_parts[part];
    if (has_beak(mptr) && part == NOSE)
        return "beak";
    if (is_centaur(mptr)
        || mptr->mlet == S_UNICORN
        || mptr == &mons[PM_KI_RIN]
        || mptr == &mons[PM_ELDRITCH_KI_RIN]
        || (mptr == &mons[PM_ROTHE] && part != HAIR))
        return horse_parts[part];
    if (mptr->mlet == S_LIGHT) {
        if (part == HANDED)
            return "rayed";
        else if (part == ARM || part == FINGER || part == FINGERTIP
                 || part == HAND)
            return "ray";
        else
            return "beam";
    }
    if (mptr == &mons[PM_STALKER] && part == HEAD)
        return "head";
    if (mptr->mlet == S_EEL && mptr != &mons[PM_JELLYFISH]
        && mptr != &mons[PM_MIND_FLAYER_LARVA])
        return fish_parts[part];
    if (mptr->mlet == S_WORM)
        return worm_parts[part];
    if (mptr->mlet == S_SPIDER)
        return spider_parts[part];
    if (slithy(mptr) || (mptr->mlet == S_DRAGON
                         && (part == HAIR || part == SKIN)))
        return snake_parts[part];
    if (mptr->mlet == S_EYE)
        return sphere_parts[part];
    if (mptr->mlet == S_JELLY || mptr->mlet == S_PUDDING
        || mptr->mlet == S_BLOB || mptr == &mons[PM_JELLYFISH])
        return jelly_parts[part];
    if (mptr->mlet == S_VORTEX || mptr->mlet == S_ELEMENTAL)
        return vortex_parts[part];
    if (mptr->mlet == S_FUNGUS)
        return fungus_parts[part];
    if (mptr->mlet == S_ENT)
        return tree_parts[part];
    if (mptr->mlet == S_PLANT)
        return plant_parts[part];
    if (is_satyr(mptr) && part == FOOT)
        return "hoof";
    if (humanoid(mptr))
        return humanoid_parts[part];
    return animal_parts[part];
}

const char *
body_part(part)
int part;
{
    return mbodypart(&youmonst, part);
}

int
poly_gender()
{
    /* Returns gender of polymorphed player;
     * 0/1=same meaning as flags.female, 2=none.
     */
    if (is_neuter(youmonst.data) || !humanoid(youmonst.data))
        return 2;
    return flags.female;
}

void
ugolemeffects(damtype, dam)
int damtype, dam;
{
    int heal = 0;

    /* We won't bother with "slow"/"haste" since players do not
     * have a monster-specific slow/haste so there is no way to
     * restore the old velocity once they are back to human.
     */
    if (u.umonnum != PM_FLESH_GOLEM && u.umonnum != PM_IRON_GOLEM)
        return;
    switch (damtype) {
    case AD_ELEC:
        if (u.umonnum == PM_FLESH_GOLEM)
            heal = (dam + 5) / 6; /* Approx 1 per die */
        break;
    case AD_FIRE:
        if (u.umonnum == PM_IRON_GOLEM)
            heal = dam;
        break;
    }
    if (heal && (u.mh < u.mhmax)) {
        u.mh += heal;
        if (u.mh > u.mhmax)
            u.mh = u.mhmax;
        context.botl = 1;
        pline("Strangely, you feel better than before.");
        exercise(A_STR, TRUE);
    }
}

/* Given a monster, return the sort of dragon it will polymorph into.
 * This is evaluated in three steps from innermost to outermost:
 * 1) Armor embedded in the skin (player only).
 * 2) Dragon-scaled armor worn in the body armor slot.
 * 3) Dragon scales worn in the cloak slot.
 */
int
armor_to_dragon(mon)
struct monst *mon;
{
    struct obj* array[3]; /* skin, body, cloak */
    int i;

    if (mon == &youmonst) {
        array[0] = uskin;
        array[1] = uarm;
        array[2] = uarmc;
    } else {
        array[0] = NULL;
        array[1] = which_armor(mon, W_ARM);
        array[2] = which_armor(mon, W_ARMC);
    }

    for (i = 0; i < 3; ++i) {
        if (array[i] != NULL) {
            if (Is_dragon_armor(array[i])) {
                if (Dragon_armor_to_pm(array[i]) == PM_CHROMATIC_DRAGON)
                    return PM_RED_DRAGON;
                else if (Dragon_armor_to_pm(array[i]) == PM_CELESTIAL_DRAGON)
                    /* their scales grant shock resistance... */
                    return PM_BLUE_DRAGON;
                /* catchall for nopoly cases that aren't handled above */
                else if (!polyok(&mons[Dragon_armor_to_pm(array[i])]))
                    return PM_RED_DRAGON;
                else
                    return Dragon_armor_to_pm(array[i]);
            }
        }
    }
    return NON_PM;
}

/* some species have awareness of other species */
static void
polysense()
{
    short warnidx = NON_PM;

    context.warntype.speciesidx = NON_PM;
    context.warntype.species = 0;
    context.warntype.polyd = 0;
    HWarn_of_mon &= ~FROMFORM;

    switch (u.umonnum) {
    case PM_PURPLE_WORM:
        warnidx = PM_SHRIEKER;
        break;
    case PM_VAMPIRE:
    case PM_VAMPIRE_NOBLE:
    case PM_VAMPIRE_ROYAL:
    case PM_VAMPIRE_MAGE:
        context.warntype.polyd = MH_HUMAN | MH_ELF;
        HWarn_of_mon |= FROMFORM;
        return;
    }
    if (warnidx >= LOW_PM) {
        context.warntype.speciesidx = warnidx;
        context.warntype.species = &mons[warnidx];
        HWarn_of_mon |= FROMFORM;
    }
}

/* True iff hero's role or race has been genocided */
boolean
ugenocided()
{
    return (boolean) ((mvitals[urole.malenum].mvflags & G_GENOD)
                      || (urole.femalenum != NON_PM
                          && (mvitals[urole.femalenum].mvflags & G_GENOD))
                      || (mvitals[urace.malenum].mvflags & G_GENOD)
                      || (urace.femalenum != NON_PM
                          && (mvitals[urace.femalenum].mvflags & G_GENOD)));
}

/* how hero feels "inside" after self-genocide of role or race */
const char *
udeadinside()
{
    /* self-genocide used to always say "you feel dead inside" but that
       seems silly when you're polymorphed into something undead;
       monkilled() distinguishes between living (killed) and non (destroyed)
       for monster death message; we refine the nonliving aspect a bit */
    return !nonliving(youmonst.data)
             ? "dead"          /* living, including demons */
             : !weirdnonliving(youmonst.data)
                 ? "condemned" /* undead plus manes */
                 : "empty";    /* golems plus vortices */
}

/*polyself.c*/
