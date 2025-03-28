/* NetHack 3.6	sit.c	$NHDT-Date: 1559670609 2019/06/04 17:50:09 $  $NHDT-Branch: NetHack-3.6 $:$NHDT-Revision: 1.61 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2012. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "artifact.h"


/* take away the hero's money */
void
take_gold()
{
    struct obj *otmp, *nobj;
    int lost_money = 0;

    for (otmp = invent; otmp; otmp = nobj) {
        nobj = otmp->nobj;
        if (otmp->oclass == COIN_CLASS) {
            lost_money = 1;
            remove_worn_item(otmp, FALSE);
            delobj(otmp);
        }
    }
    if (!lost_money) {
        You_feel("a strange sensation.");
    } else {
        You("notice you have no money!");
        context.botl = 1;
    }
}

/* #sit command */
int
dosit()
{
    static const char sit_message[] = "sit on the %s.";
    register struct trap *trap = t_at(u.ux, u.uy);
    register int typ = levl[u.ux][u.uy].typ;

    if (Hidinshell) {
        You_cant("sit while hiding in your shell.");
        return 0;
    }
    if (u.usteed) {
        You("are already sitting on %s.", mon_nam(u.usteed));
        return 0;
    }
    if (u.uundetected && is_hider(youmonst.data) && u.umonnum != PM_TRAPPER)
        u.uundetected = 0; /* no longer on the ceiling */

    if (!can_reach_floor(FALSE)) {
        if (u.uswallow)
            There("are no seats in here!");
        else if (Levitation)
            You("tumble in place.");
        else
            You("are sitting on air.");
        return 0;
    } else if (u.ustuck && !sticks(youmonst.data)) {
        /* holding monster is next to hero rather than beneath, but
           hero is in no condition to actually sit at has/her own spot */
        if (humanoid(u.ustuck->data))
            pline("%s won't offer %s lap.", Monnam(u.ustuck), mhis(u.ustuck));
        else
            pline("%s has no lap.", Monnam(u.ustuck));
        return 0;
    } else if (is_pool(u.ux, u.uy) && !Underwater) { /* water walking */
        goto in_water;
    }

    if (OBJ_AT(u.ux, u.uy)
        /* ensure we're not standing on the precipice */
        && !(uteetering_at_seen_pit(trap) || uescaped_shaft(trap))) {
        register struct obj *obj;

        obj = level.objects[u.ux][u.uy];
        if (youmonst.data->mlet == S_DRAGON && obj->oclass == COIN_CLASS) {
            You("coil up around your %shoard.",
                (obj->quan + money_cnt(invent) < u.ulevel * 1000) ? "meager "
                                                                  : "");
        } else {
            You("sit on %s.", the(xname(obj)));
            if (!(Is_box(obj) || obj->material == CLOTH))
                pline("It's not very comfortable...");
        }
    } else if (trap != 0 || (u.utrap && (u.utraptype >= TT_LAVA))) {
        if (u.utrap) {
            exercise(A_WIS, FALSE); /* you're getting stuck longer */
            if (u.utraptype == TT_BEARTRAP) {
                You_cant("sit down with your %s in the bear trap.",
                         body_part(FOOT));
                u.utrap++;
            } else if (u.utraptype == TT_PIT) {
                if (trap && trap->ttyp == SPIKED_PIT) {
                    You("sit down on a spike.  Ouch!");
                    losehp(Half_physical_damage ? rn2(2) : 1,
                           "sitting on an iron spike", KILLED_BY);
                    exercise(A_STR, FALSE);
                } else
                    You("sit down in the pit.");
                u.utrap += rn2(5);
            } else if (u.utraptype == TT_WEB) {
                You("sit in the spider web and get entangled further!");
                u.utrap += rn1(10, 5);
            } else if (u.utraptype == TT_LAVA) {
                /* Must have fire resistance or they'd be dead already */
                You("sit in the %s!", hliquid("lava"));
                if (Slimed)
                    burn_away_slime();
                u.utrap += rnd(4);
                losehp(d(2, 10), "sitting in lava",
                       KILLED_BY); /* lava damage */
            } else if (u.utraptype == TT_INFLOOR
                       || u.utraptype == TT_BURIEDBALL) {
                You_cant("maneuver to sit!");
                u.utrap++;
            }
        } else {
            You("sit down.");
            dotrap(trap, VIASITTING);
        }
    } else if ((Underwater || Is_waterlevel(&u.uz))
                && !eggs_in_water(youmonst.data)) {
        if (Is_waterlevel(&u.uz))
            There("are no cushions floating nearby.");
        else
            You("sit down on the muddy bottom.");
    } else if (is_damp_terrain(u.ux, u.uy) && !eggs_in_water(youmonst.data)) {
    in_water:
        You("sit in the %s.", hliquid("water"));
        if (!rn2(10) && uarm)
            (void) water_damage(uarm, "armor", TRUE, u.ux, u.uy);
        if (!rn2(10) && uarmf && uarmf->otyp != WATER_WALKING_BOOTS)
            (void) water_damage(uarm, "armor", TRUE, u.ux, u.uy);
    } else if (IS_SINK(typ)) {
        You(sit_message, defsyms[S_sink].explanation);
        Your("%s gets wet.", humanoid(youmonst.data) ? "rump" : "underside");
    } else if (IS_ALTAR(typ)) {
        You(sit_message, defsyms[S_altar].explanation);
        altar_wrath(u.ux, u.uy);
    } else if (IS_GRAVE(typ)) {
        You(sit_message, defsyms[S_grave].explanation);
    } else if (typ == STAIRS) {
        You(sit_message, "stairs");
    } else if (typ == LADDER) {
        You(sit_message, "ladder");
    } else if (typ == MAGIC_CHEST) {
        You(sit_message, defsyms[S_magic_chest].explanation);
        pline("It's surprisingly comfortable!"); /* more ergonomic than most chests */
    } else if (IS_FORGE(typ)) {
        You(sit_message, defsyms[S_forge].explanation);
        burn_away_slime();
        if (likes_fire(youmonst.data) || Fire_resistance) {
            pline_The("forge feels nice and cozy.");
            return 1;
        }
        pline("Argh!  This forge is hot!");
        losehp(resist_reduce(d(4, 4), FIRE_RES), /* lava damage */
               "sitting on a forge", KILLED_BY);
    } else if (is_lava(u.ux, u.uy)) {
        /* must be WWalking */
        You(sit_message, hliquid("lava"));
        burn_away_slime();
        if (likes_lava(youmonst.data)) {
            pline_The("%s feels warm.", hliquid("lava"));
            return 1;
        }
        pline_The("%s burns you!", hliquid("lava"));
        losehp(resist_reduce(d(8, 10), FIRE_RES) + d(2, 10), /* lava damage */
               "sitting on lava", KILLED_BY);
    } else if (is_ice(u.ux, u.uy)) {
        You(sit_message, defsyms[S_ice].explanation);
        if (how_resistant(COLD_RES) < 100)
            pline_The("ice feels cold.");
    } else if (typ == DRAWBRIDGE_DOWN) {
        You(sit_message, "drawbridge");
    } else if (IS_THRONE(typ)) {
        You(sit_message, defsyms[S_throne].explanation);
        if (rnd(6) > 4) {
            switch (rnd(13)) {
            case 1:
                (void) adjattrib(rn2(A_MAX), -rn1(4, 3), FALSE);
                losehp(rnd(10), "cursed throne", KILLED_BY_AN);
                break;
            case 2:
                (void) adjattrib(rn2(A_MAX), 1, FALSE);
                break;
            case 3:
                pline("A%s electric shock shoots through your body!",
                      (how_resistant(SHOCK_RES) > 50) ? "n" : " massive");
                losehp(resist_reduce(rnd(24), SHOCK_RES) + rnd(6), "electric chair",
                       KILLED_BY_AN);
                exercise(A_CON, FALSE);
                break;
            case 4:
                You_feel("much, much better!");
                if (Upolyd) {
                    if (u.mh >= (u.mhmax - 5))
                        u.mhmax += 4;
                    u.mh = u.mhmax;
                }
                if (u.uhp >= (u.uhpmax - 5))
                    u.uhpmax += 4;
                u.uhp = u.uhpmax;
                u.ucreamed = 0;
                make_blinded(0L, TRUE);
                make_sick(0L, (char *) 0, FALSE, SICK_ALL);
                heal_legs(0);
                context.botl = 1;
                break;
            case 5:
                take_gold();
                break;
            case 6:
                if (u.uluck + rn2(5) < 0) {
                    You_feel("your luck is changing.");
                    change_luck(1);
                } else {
                    /* overall this equates to a 1.5% chance for a wish */
                    if (!rn2(5)) {
                        makewish(FALSE);
                        /* no farming thrones for multiple wishes */
                        levl[u.ux][u.uy].typ = ROOM, levl[u.ux][u.uy].flags = 0;
                        pline_The("throne vanishes in a puff of logic.");
                        newsym(u.ux, u.uy);
                    } else {
                        if (Luck < 0 || (HSee_invisible & INTRINSIC)) {
                            if (level.flags.nommap) {
                                pline("An awful drone fills your head!");
                                make_confused((HConfusion & TIMEOUT) + (long) rnd(30),
                                              FALSE);
                            } else {
                                pline("A clear image forms in your mind.");
                                do_mapping();
                            }
                        } else {
                            /* permanent see invisible in this instance */
                            Your("vision becomes crystal clear.");
                            HSee_invisible |= FROMOUTSIDE;
                            newsym(u.ux, u.uy);
                        }
                    }
                }
                break;
            case 7:
              {
                int cnt = rnd(10);

                /* Magical voice not affected by deafness */
                pline("A voice echoes:");
                verbalize("Thy audience hath been summoned, %s!",
                          flags.female ? "Dame" : "Sire");
                while (cnt--)
                    (void) makemon(courtmon(), u.ux, u.uy, NO_MM_FLAGS);
                break;
              }
            case 8:
                /* Magical voice not affected by deafness */
                pline("A voice echoes:");
                verbalize("By thine Imperious order, %s...",
                          flags.female ? "Dame" : "Sire");
                do_genocide(5); /* REALLY|ONTHRONE, see do_genocide() */
                break;
            case 9:
                /* Magical voice not affected by deafness */
                pline("A voice echoes:");
                verbalize(
                 "A curse upon thee for sitting upon this most holy throne!");
                if (Luck > 0) {
                    make_blinded(Blinded + rn1(100, 250), TRUE);
                    change_luck((Luck > 1) ? -rnd(2) : -1);
                } else
                    rndcurse();
                break;
            case 10:
                if (Luck < 0 || (HSee_invisible & INTRINSIC)) {
                    if (level.flags.nommap) {
                        pline("A terrible drone fills your head!");
                        make_confused((HConfusion & TIMEOUT) + (long) rnd(30),
                                      FALSE);
                    } else {
                        pline("An image forms in your mind.");
                        do_mapping();
                    }
                } else {
                    Your("vision becomes clear.");
                    incr_itimeout(&HSee_invisible, (long) rn1(100, 50));
                    newsym(u.ux, u.uy);
                }
                break;
            case 11:
                if (Luck < 0) {
                    You_feel("threatened.");
                    aggravate();
                } else {
                    You_feel("a wrenching sensation.");
                    tele(); /* teleport him */
                }
                break;
            case 12:
                You("are granted an insight!");
                if (invent) {
                    /* rn2(5) agrees w/seffects() */
                    identify_pack(rn2(5), FALSE);
                }
                break;
            case 13:
                Your("mind turns into a pretzel!");
                make_confused((HConfusion & TIMEOUT) + (long) rn1(7, 16),
                              FALSE);
                break;
            default:
                impossible("throne effect");
                break;
            }
        } else {
            if (is_prince(youmonst.data))
                You_feel("very comfortable here.");
            else
                You_feel("somehow out of place...");
        }

        if (!rn2(3) && IS_THRONE(levl[u.ux][u.uy].typ)) {
            /* may have teleported */
            levl[u.ux][u.uy].typ = ROOM, levl[u.ux][u.uy].flags = 0;
            pline_The("throne vanishes in a puff of logic.");
            newsym(u.ux, u.uy);
        }
    } else if (lays_eggs(youmonst.data)) {
        struct obj *uegg;

        if (!flags.female) {
            pline("%s can't lay eggs!",
                  Hallucination
                      ? "You may think you are a platypus, but a male still"
                      : "Males");
            return 0;
        } else if (u.uhunger < (int) objects[EGG].oc_nutrition) {
            You("don't have enough energy to lay an egg.");
            return 0;
        } else if (eggs_in_water(youmonst.data)) {
            if (!(Underwater || Is_waterlevel(&u.uz))) {
                pline("A splash tetra you are not.");
                return 0;
            }
            if (Upolyd &&
                (youmonst.data == &mons[PM_GIANT_EEL]
                 || youmonst.data == &mons[PM_ELECTRIC_EEL])) {
                You("yearn for the Sargasso Sea.");
                return 0;
            }
        } else if (rn2(1000) && druid_form) {
            You("try, but you aren't quite ready to lay an egg.");
            return 1; /* time passes */
        }
        uegg = mksobj(EGG, FALSE, FALSE);
        uegg->spe = 1;
        uegg->quan = 1L;
        uegg->owt = weight(uegg);
        /* this sets hatch timers if appropriate */
        set_corpsenm(uegg, egg_type_from_parent(u.umonnum, FALSE));
        uegg->known = uegg->dknown = 1;
        You("%s an egg.", eggs_in_water(youmonst.data) ? "spawn" : "lay");
        dropy(uegg);
        stackobj(uegg);
        morehungry((int) objects[EGG].oc_nutrition);
    } else {
        pline("Having fun sitting on the %s?", surface(u.ux, u.uy));
    }
    return 1;
}

/* curse a few inventory items at random! */
void
rndcurse()
{
    int nobj = 0;
    int cnt, onum;
    struct obj *otmp;
    static const char mal_aura[] = "feel a malignant aura surround %s.";

    if ((wielding_artifact(ART_MAGICBANE) && rn2(20))
         || (wielding_artifact(ART_STAFF_OF_THE_ARCHMAGI) && rn2(30))) {
        You(mal_aura,
            wielding_artifact(ART_MAGICBANE) ? "the magic-absorbing staff"
                                             : "the powerful staff");
        return;
    }

    if (Antimagic) {
        shieldeff(u.ux, u.uy);
        You(mal_aura, "you");
    }

    for (otmp = invent; otmp; otmp = otmp->nobj) {
        /* gold isn't subject to being cursed or blessed */
        if (otmp->oclass == COIN_CLASS)
            continue;
        nobj++;
    }
    if (nobj) {
        for (cnt = rnd(6 / ((!!Antimagic) + (!!Half_spell_damage) + 1));
             cnt > 0; cnt--) {
            onum = rnd(nobj);
            for (otmp = invent; otmp; otmp = otmp->nobj) {
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

            if (otmp->oartifact && spec_ability(otmp, SPFX_INTEL)
                && rn2(10) < 8) {
                pline("%s!", Tobjnam(otmp, "resist"));
                continue;
            }

            if (otmp->blessed)
                unbless(otmp);
            else
                curse(otmp);
        }
        update_inventory();
    }

    /* treat steed's saddle as extended part of hero's inventory */
    if (u.usteed && !rn2(4) && (otmp = which_armor(u.usteed, W_SADDLE)) != 0
        && !otmp->cursed) { /* skip if already cursed */
        if (otmp->blessed)
            unbless(otmp);
        else
            curse(otmp);
        if (!Blind) {
            pline("%s %s.", Yobjnam2(otmp, "glow"),
                  hcolor(otmp->cursed ? NH_BLACK : (const char *) "brown"));
            otmp->bknown = 1; /* ok to bypass set_bknown() here */
        }
    }
}

void
mrndcurse(mtmp) /* curse a few inventory items at random! */
struct monst *mtmp;
{
    int nobj = 0;
    int cnt, onum;
    struct obj *otmp;
    static const char mal_aura[] = "feel a malignant aura surround %s.";

    boolean resists = resist(mtmp, 0, 0, FALSE),
            vis = couldsee(mtmp->mx, mtmp->my);

    if (vis && MON_WEP(mtmp)
        && (((MON_WEP(mtmp)->oartifact == ART_MAGICBANE) && rn2(20))
             || ((MON_WEP(mtmp)->oartifact == ART_STAFF_OF_THE_ARCHMAGI)
                 && rn2(30)))) {
        You(mal_aura,
            (MON_WEP(mtmp)->oartifact == ART_MAGICBANE)
                ? "the magic-absorbing staff" : "the powerful staff");
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
        update_inventory();
    }
}

/* remove a random INTRINSIC ability */
void
attrcurse()
{
    switch (rnd(14)) {
    case 1:
        if (HFire_resistance) {
	    HFire_resistance = HFire_resistance & (TIMEOUT | FROMOUTSIDE | HAVEPARTIAL);
	    decr_resistance(&HFire_resistance, rnd(50) + 50);
            You_feel("warmer.");
            break;
        }
        /*FALLTHRU*/
    case 2:
        if (HTeleportation & INTRINSIC) {
            HTeleportation &= ~INTRINSIC;
            You_feel("less jumpy.");
            break;
        }
        /*FALLTHRU*/
    case 3:
        if (HPoison_resistance) {
	    HPoison_resistance = HPoison_resistance & (TIMEOUT | FROMOUTSIDE | HAVEPARTIAL);
	    decr_resistance(&HPoison_resistance, rnd(50) + 50);
            You_feel("a little sick!");
            break;
        }
        /*FALLTHRU*/
    case 4:
        if (HTelepat & INTRINSIC) {
            HTelepat &= ~INTRINSIC;
            if (Blind && !Blind_telepat)
                see_monsters(); /* Can't sense mons anymore! */
            Your("senses fail!");
            break;
        }
        /*FALLTHRU*/
    case 5:
        if (HCold_resistance) {
	    HCold_resistance = HCold_resistance & (TIMEOUT | FROMOUTSIDE | HAVEPARTIAL);
	    decr_resistance(&HCold_resistance, rnd(50) + 50);
            You_feel("cooler.");
            break;
        }
        /*FALLTHRU*/
    case 6:
        if (HInvis & INTRINSIC) {
            HInvis &= ~INTRINSIC;
            You_feel("paranoid.");
            break;
        }
        /*FALLTHRU*/
    case 7:
        if (HSee_invisible & INTRINSIC) {
            HSee_invisible &= ~INTRINSIC;
            You("%s!", Hallucination ? "tawt you taw a puttie tat"
                                     : "thought you saw something");
            break;
        }
        /*FALLTHRU*/
    case 8:
        if (HFast & INTRINSIC) {
            HFast &= ~INTRINSIC;
            if (!Slow) {
                You_feel("slower.");
            }
            break;
        }
        /*FALLTHRU*/
    case 9:
        if (HStealth & INTRINSIC) {
            HStealth &= ~INTRINSIC;
            You_feel("clumsy.");
            break;
        }
        /*FALLTHRU*/
    case 10:
        /* intrinsic protection is just disabled, not set back to 0 */
        if (HProtection & INTRINSIC) {
            HProtection &= ~INTRINSIC;
            You_feel("vulnerable.");
            break;
        }
        /*FALLTHRU*/
    case 11:
        if (HAggravate_monster & INTRINSIC) {
            HAggravate_monster &= ~INTRINSIC;
            You_feel("less attractive.");
            break;
        }
        /*FALLTHRU*/
    case 12:
        if (HSleep_resistance) {
	    HSleep_resistance = HSleep_resistance & (TIMEOUT | FROMOUTSIDE | HAVEPARTIAL);
	    decr_resistance(&HSleep_resistance, rnd(50) + 50);
	    You_feel("a little tired.");
	    break;
	}
        /*FALLTHRU*/
    case 13:
        if (HDisint_resistance) {
	    HDisint_resistance = HDisint_resistance & (TIMEOUT | FROMOUTSIDE | HAVEPARTIAL);
	    decr_resistance(&HDisint_resistance, rnd(50) + 50);
	    You_feel("less firm.");
	    break;
	}
        /*FALLTHRU*/
    case 14:
        if (HShock_resistance) {
	    HShock_resistance = HShock_resistance & (TIMEOUT | FROMOUTSIDE | HAVEPARTIAL);
	    decr_resistance(&HShock_resistance, rnd(50) + 50);
	    You_feel("more conductive.");
	    break;
	}
        /*FALLTHRU*/
    default:
        break;
    }
}

/*sit.c*/
