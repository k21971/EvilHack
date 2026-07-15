/* NetHack 5.0	fountain.c	$NHDT-Date: 1781973050 2026/06/20 16:30:50 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.121 $ */
/*      Copyright Scott R. Turner, srt@ucla, 10/27/86 */
/* NetHack may be freely redistributed.  See license for details. */

/* Code for drinking from fountains. */

#include "hack.h"

staticfn void dowatersnakes(void);
staticfn void dowaterdemon(void);
staticfn void dowaternymph(void);
staticfn void dolavademon(void);
staticfn void gush(coordxy, coordxy, genericptr_t) NONNULLARG3;
staticfn void dofindgem(void);
staticfn boolean watchman_warn_fountain(struct monst *) NONNULLARG1;
staticfn int forge_ok(struct obj *);
staticfn int affix_weapon_ok(struct obj *);
staticfn int affix_gem_ok(struct obj *);
staticfn boolean affix_success_roll(struct obj *) NONNULLARG1;

DISABLE_WARNING_FORMAT_NONLITERAL

/* used when trying to dip in or drink from fountain or sink or pool while
   levitating above it, or when trying to move downwards in that state */
void
floating_above(const char *what)
{
    const char *umsg = "are floating high above the %s.";

    if (u.utrap && (u.utraptype == TT_INFLOOR || u.utraptype == TT_LAVA)) {
        /* when stuck in floor (not possible at fountain or sink location,
           so must be attempting to move down), override the usual message */
        umsg = "are trapped in the %s.";
        what = surface(u.ux, u.uy); /* probably redundant */
    }
    You(umsg, what);
}

RESTORE_WARNING_FORMAT_NONLITERAL

/* Fountain of snakes! */
staticfn void
dowatersnakes(void)
{
    int num = rn1(5, 2);
    struct monst *mtmp;

    if (!(svm.mvitals[PM_WATER_MOCCASIN].mvflags & G_GONE)) {
        if (!Blind) {
            pline("An endless stream of %s pours forth!",
                  Hallucination ? makeplural(rndmonnam(NULL)) : "snakes");
        } else {
            Soundeffect(se_snakes_hissing, 75);
            You_hear("%s hissing!", something);
        }
        while (num-- > 0)
            if ((mtmp = makemon(&mons[PM_WATER_MOCCASIN], u.ux, u.uy,
                                MM_NOMSG)) != 0
                && t_at(mtmp->mx, mtmp->my))
                (void) mintrap(mtmp, NO_TRAP_FLAGS);
    } else {
        Soundeffect(se_furious_bubbling, 20);
        pline_The("fountain bubbles furiously for a moment, then calms.");
    }
}

/* Water demon */
staticfn void
dowaterdemon(void)
{
    struct monst *mtmp;

    if (!(svm.mvitals[PM_WATER_DEMON].mvflags & G_GONE)) {
        if ((mtmp = makemon(&mons[PM_WATER_DEMON], u.ux, u.uy,
                            MM_NOMSG)) != 0) {
            if (!Blind)
                You("unleash %s!", a_monnam(mtmp));
            else
                You_feel("the presence of evil.");

            /* Give those on low levels a (slightly) better chance of survival
             */
            if (rnd(100) > (80 + level_difficulty())) {
                pline("Grateful for %s release, %s grants you a wish!",
                      mhis(mtmp), mhe(mtmp));
                /* give a wish and discard the monster (mtmp set to null) */
                mongrantswish(&mtmp);
            } else if (t_at(mtmp->mx, mtmp->my))
                (void) mintrap(mtmp, NO_TRAP_FLAGS);
        }
    } else {
        Soundeffect(se_furious_bubbling, 20);
        pline_The("fountain bubbles furiously for a moment, then calms.");
    }
}

/* Water Nymph */
staticfn void
dowaternymph(void)
{
    struct monst *mtmp;

    if (!(svm.mvitals[PM_WATER_NYMPH].mvflags & G_GONE)
        && (mtmp = makemon(&mons[PM_WATER_NYMPH], u.ux, u.uy,
                           MM_NOMSG)) != 0) {
        if (!Blind)
            You("attract %s!", a_monnam(mtmp));
        else
            You_hear("a seductive voice.");
        mtmp->msleeping = 0;
        if (t_at(mtmp->mx, mtmp->my))
            (void) mintrap(mtmp, NO_TRAP_FLAGS);
    } else if (!Blind) {
        Soundeffect(se_bubble_rising, 50);
        Soundeffect(se_loud_pop, 50);
        pline("A large bubble rises to the surface and pops.");
    } else {
        Soundeffect(se_loud_pop, 50);
        You_hear("a loud pop.");
    }
}

/* Lava Demon */
staticfn void
dolavademon(void)
{
    struct monst *mtmp;

    if (!(svm.mvitals[PM_LAVA_DEMON].mvflags & G_GONE)) {
        if ((mtmp = makemon(&mons[PM_LAVA_DEMON], u.ux, u.uy,
                            MM_NOMSG)) != 0) {
            if (!Blind)
                You("summon %s!", a_monnam(mtmp));
            else
                You_feel("the temperature rise significantly.");

            /* Give those on low levels a (slightly) better chance of
               survival; tamedog() can refuse, so only promise the
               demon's aid when it actually joins */
            if (rnd(100) > (80 + level_difficulty())
                && tamedog(mtmp, (struct obj *) 0, FALSE)) {
                pline("Freed from the depths of Gehennom, %s offers to aid you in your quest!",
                      mhe(mtmp));
            } else if (t_at(mtmp->mx, mtmp->my))
                (void) mintrap(mtmp, NO_TRAP_FLAGS);
        }
    } else {
        pline_The("forge violently spews lava for a moment, then settles.");
    }
}

/* Gushing forth along LOS from (u.ux, u.uy) */
void
dogushforth(int drinking)
{
    int madepool = 0;

    do_clear_area(u.ux, u.uy, 7, gush, (genericptr_t) &madepool);
    if (!madepool) {
        if (drinking)
            Your("thirst is quenched.");
        else
            pline("Water sprays all over you.");
    }
}

staticfn void
gush(coordxy x, coordxy y, genericptr_t poolcnt)
{
    struct monst *mtmp;
    struct trap *ttmp;

    if (((x + y) % 2) || u_at(x, y)
        || (rn2(1 + distmin(u.ux, u.uy, x, y))) || (levl[x][y].typ != ROOM)
        || (sobj_at(BOULDER, x, y)) || nexttodoor(x, y))
        return;

    if ((ttmp = t_at(x, y)) != 0 && !delfloortrap(ttmp))
        return;

    if (!((*(int *) poolcnt)++))
        pline("Water gushes forth from the overflowing fountain!");

    /* Put a pool at x, y */
    set_levltyp(x, y, POOL);
    levl[x][y].flags = 0;
    /* No kelp! */
    del_engr_at(x, y);
    water_damage_chain(svl.level.objects[x][y], TRUE);

    if ((mtmp = m_at(x, y)) != 0)
        (void) minliquid(mtmp);
    else
        newsym(x, y);
}

/* Find a gem in the sparkling waters. */
staticfn void
dofindgem(void)
{
    if (!Blind)
        You("spot a gem in the sparkling waters!");
    else
        You_feel("a gem here!");
    (void) mksobj_at(rnd_class(DILITHIUM_CRYSTAL, LUCKSTONE - 1), u.ux, u.uy,
                     FALSE, FALSE);
    SET_FOUNTAIN_LOOTED(u.ux, u.uy);
    newsym(u.ux, u.uy);
    exercise(A_WIS, TRUE); /* a discovery! */
}

staticfn boolean
watchman_warn_fountain(struct monst *mtmp)
{
    if (is_watch(mtmp->data) && couldsee(mtmp->mx, mtmp->my)
        && mtmp->mpeaceful) {
        if (!Deaf) {
            pline("%s yells:", Amonnam(mtmp));
            verbalize("Hey, stop using that fountain!");
        } else {
            pline("%s earnestly %s %s %s!",
                  Amonnam(mtmp),
                  nolimbs(mtmp->data) ? "shakes" : "waves",
                  mhis(mtmp),
                  nolimbs(mtmp->data)
                  ? mbodypart(mtmp, HEAD)
                  : makeplural(mbodypart(mtmp, ARM)));
        }
        return TRUE;
    }
    return FALSE;
}

void
dryup(coordxy x, coordxy y, boolean isyou)
{
    if (IS_FOUNTAIN(levl[x][y].typ)
        && (!rn2(3) || FOUNTAIN_IS_WARNED(x, y))) {
        if (isyou && in_town(x, y) && !FOUNTAIN_IS_WARNED(x, y)) {
            struct monst *mtmp;

            SET_FOUNTAIN_WARNED(x, y);
            /* Warn about future fountain use. */
            mtmp = get_iter_mons(watchman_warn_fountain);
            /* You can see or hear this effect */
            if (!mtmp)
                pline_The("flow reduces to a trickle.");
            return;
        }
        if (isyou && wizard) {
            if (y_n("Dry up fountain?") == 'n')
                return;
        }
        /* FIXME: sight-blocking clouds should use block_point() when
           being created and unblock_point() when going away, then this
           glyph hackery wouldn't be necessary */
        if (cansee(x, y)) {
            int glyph = glyph_at(x, y);

            if (!glyph_is_cmap(glyph) || glyph_to_cmap(glyph) != S_cloud)
                pline_The("fountain dries up!");
        }
        /* replace the fountain with ordinary floor */
        set_levltyp(x, y, ROOM); /* updates level.flags.nfountains */
        levl[x][y].flags = 0;
        levl[x][y].blessedftn = 0;
        /* The location is seen if the hero/monster is invisible
           or felt if the hero is blind. */
        newsym(x, y);
        if (isyou && in_town(x, y))
            (void) angry_guards(FALSE);
    }
}

/* for #dip when standing on a forge */
void
dipforge(struct obj *obj)
{
    if (Levitation) {
        floating_above("forge");
        return;
    }

    burn_away_slime();

    /* handle specific types of object destruction via dipping into a
       forge; anything not handled here is caught by lava_damage() */
    if (obj->oclass == POTION_CLASS || obj->oclass == SCROLL_CLASS
        || obj->oclass == SPBOOK_CLASS
        || obj->otyp == GLOB_OF_GREEN_SLIME) {
        destroy_one_item(obj, AD_FIRE);
        return;
    }

    /* Dipping something you're still wearing into a forge filled with
     * lava, probably not the smartest thing to do. This is gonna hurt.
     * Non-metallic objects are handled by lava_damage().
     */
    if (is_metallic(obj) && (obj->owornmask & (W_ARMOR | W_ACCESSORY))) {
        if (Fire_resistance) {
            /* immune to fire but still can't reforge worn items */
            You("can't reforge something you're wearing.");
        } else {
            You("dip your worn %s into the forge.  You burn yourself!",
                xname(obj));
            if (!rn2(3))
                You("may want to remove your %s first...",
                    xname(obj));
            losehp(d(1, 8),
                   "dipping a worn object into a forge", KILLED_BY);
        }
        return;
    }

    /* If punished and wielding a hammer, there's a good chance
     * you can use a forge to free yourself; only the attached
     * ball can break the chain, not stray HEAVY_IRON_BALLs */
    if (Punished && obj == uball) {
        boolean hammer = (uwep && (is_hammer(uwep)
                                   || uwep->otyp == BLACKSMITH_HAMMER));

        if ((uwep && !hammer) || !uwep) { /* sometimes drop a hint */
            if (!rn2(4))
                pline("You'll need a hammer to be able to break the chain.");
            goto result;
        } else if (hammer) {
            You("place the ball and chain inside the forge.");
            pline("Raising your %s, you strike the chain...",
                  xname(uwep));
            if ((!rn2((P_SKILL(P_HAMMER) < P_SKILLED) ? 8 : 2)
                 || (uchain && uchain->oeroded))
                && Luck >= 0) { /* training up hammer skill pays off */
                pline_The("%schain breaks free!",
                          (uchain && uchain->oeroded) ? "rusted " : "");
                unpunish();
            } else {
                pline("Clang!");
            }
        }
        return;
    }

    /* handle various object coatings */
    if (obj->greased) {
        obj->greased = 0;
        if (!Blind)
            pline_The("grease on %s burns away.",
                      ysimple_name(obj));
    }
    if (obj->opoisoned) {
        obj->opoisoned = 0;
        if (!Blind)
            pline_The("poisonous coating on %s boils away.",
                      ysimple_name(obj));
    }
    if (obj->otainted) {
        obj->otainted = 0;
        if (!Blind)
            pline_The("inky coating on %s boils away.",
                      ysimple_name(obj));
    }

 result:
    switch (rnd(30)) {
    case 6:
    case 7:
    case 8:
    case 9: /* Strange feeling */
        pline("A weird sensation runs up your %s.", body_part(ARM));
        break;
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
        if (!is_metallic(obj))
            goto lava;

        /* TODO: perhaps our hero needs to wield some sort of tool to
           successfully reforge an object? */
        if (Luck >= 0) {
            if (greatest_erosion(obj) > 0) {
                if (!Blind)
                    You("successfully reforge your %s, repairing some of the damage.",
                        xname(obj));
                if (obj->oeroded > 0)
                    obj->oeroded--;
                if (obj->oeroded2 > 0)
                    obj->oeroded2--;
            } else {
                if (!Blind) {
                    Your("%s glows briefly from the heat, but looks reforged and as new as ever.",
                         xname(obj));
                }
            }
        }
        break;
    case 19:
    case 20:
        if (!is_metallic(obj))
            goto lava;

        if (!obj->blessed && Luck > 5) {
            bless(obj);
            if (!Blind) {
                Your("%s glows blue for a moment.",
                     xname(obj));
            }
        } else {
            You_feel("a sudden wave of heat.");
        }
        break;
    case 21: /* Lava Demon */
        if (!rn2(8))
            dolavademon();
        else
            pline_The("forge violently spews lava for a moment, then settles.");
        break;
    case 22:
        if (Luck < 0) {
            blowupforge(u.ux, u.uy);
            /* Avoid destroying the same item twice (lava_damage) */
            return;
        } else {
            pline("Molten lava surges up and splashes all over you!");
            if (!Fire_resistance)
                losehp(d(3, 8), "dipping into a forge", KILLED_BY);
        }
        break;
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30: /* Strange feeling */
        You_feel("a sudden flare of heat.");
        break;
    }
 lava:
    (void) lava_damage(obj, u.ux, u.uy);
    update_inventory();
}

/* quaff from a fountain when standing on its location */
void
drinkfountain(void)
{
    /* What happens when you drink from a fountain? */
    boolean mgkftn = (levl[u.ux][u.uy].blessedftn == 1);
    int fate = rnd(30);

    if (Levitation) {
        floating_above("fountain");
        return;
    }

    if (mgkftn && u.uluck >= 0 && fate >= 10) {
        int i, ii, littleluck = (u.uluck < 4);

        pline("Wow!  This makes you feel great!");
        /* blessed restore ability */
        for (ii = 0; ii < A_MAX; ii++)
            if (ABASE(ii) < AMAX(ii)) {
                ABASE(ii) = AMAX(ii);
                disp.botl = TRUE;
            }
        /* gain ability, blessed if "natural" luck is high */
        i = rn2(A_MAX); /* start at a random attribute */
        for (ii = 0; ii < A_MAX; ii++) {
            if (adjattrib(i, 1, littleluck ? -1 : 0) && littleluck)
                break;
            if (++i >= A_MAX)
                i = 0;
        }
        display_nhwindow(WIN_MESSAGE, FALSE);
        pline("A wisp of vapor escapes the fountain...");
        exercise(A_WIS, TRUE);
        levl[u.ux][u.uy].blessedftn = 0;
        return;
    }

    if (fate < 10) {
        pline_The("cool draught refreshes you.");
        u.uhunger += rnd(10); /* don't choke on water */
        newuhs(FALSE);
        if (mgkftn)
            return;
    } else {
        switch (fate) {
        case 19: /* Self-knowledge */
            You_feel("self-knowledgeable...");
            display_nhwindow(WIN_MESSAGE, FALSE);
            enlightenment(MAGICENLIGHTENMENT, ENL_GAMEINPROGRESS);
            exercise(A_WIS, TRUE);
            pline_The("feeling subsides.");
            break;
        case 20: /* Foul water */
            pline_The("water is foul!  You gag and vomit.");
            morehungry(rn1(20, 11));
            vomit();
            break;
        case 21: /* Poisonous */
            pline_The("water is contaminated!");
            if (Poison_resistance) {
                pline("Perhaps it is runoff from the nearby %s farm.",
                      fruitname(FALSE));
                losehp(rnd(4), "unrefrigerated sip of juice", KILLED_BY_AN);
                break;
            }
            poison_strdmg(rn1(4, 3), rnd(10), "contaminated water",
                          KILLED_BY);
            exercise(A_CON, FALSE);
            break;
        case 22: /* Fountain of snakes! */
            dowatersnakes();
            break;
        case 23: /* Water demon */
            dowaterdemon();
            break;
        case 24: { /* Maybe curse some items */
            struct obj *obj, *nextobj;
            int buc_changed = 0;

            pline("This water's no good!");
            morehungry(rn1(20, 11));
            exercise(A_CON, FALSE);
            /* this is more severe than rndcurse() */
            for (obj = gi.invent; obj; obj = nextobj) {
                nextobj = obj->nobj;
                if (obj->oclass != COIN_CLASS && !obj->cursed && !rn2(5)) {
                    curse(obj);
                    ++buc_changed;
                }
            }
            if (buc_changed)
                update_inventory();
            break;
        }
        case 25: /* See invisible */
            if (Blind) {
                if (Invisible) {
                    You("feel transparent.");
                } else {
                    You("feel very self-conscious.");
                    pline("Then it passes.");
                }
            } else {
                You_see("an image of someone stalking you.");
                pline("But it disappears.");
            }
            HSee_invisible |= FROMOUTSIDE;
            newsym(u.ux, u.uy);
            exercise(A_WIS, TRUE);
            break;
        case 26: /* See Monsters */
            if (monster_detect((struct obj *) 0, 0))
                pline_The("%s tastes like nothing.", hliquid("water"));
            exercise(A_WIS, TRUE);
            break;
        case 27: /* Find a gem in the sparkling waters. */
            if (!FOUNTAIN_IS_LOOTED(u.ux, u.uy)) {
                dofindgem();
                break;
            }
            FALLTHROUGH;
            /*FALLTHRU*/
        case 28: /* Water Nymph */
            dowaternymph();
            break;
        case 29: /* Scare */
        {
            struct monst *mtmp;

            pline("This %s gives you bad breath!",
                  hliquid("water"));
            for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
                if (DEADMONSTER(mtmp))
                    continue;
                monflee(mtmp, 0, FALSE, FALSE);
            }
            break;
        }
        case 30: /* Gushing forth in this room */
            dogushforth(TRUE);
            break;
        default:
            pline("This tepid %s is tasteless.",
                  hliquid("water"));
            break;
        }
    }
    dryup(u.ux, u.uy, TRUE);
}

/* Monty Python and the Holy Grail ;) */
static const char *const excalmsgs[] = {
    "had Excalibur thrown at them by some watery tart",
    "received Excalibur from a strange woman laying about in a pond",
    "signified by divine providence, was chosen to carry Excalibur",
    "has been given Excalibur, and now enjoys supreme executive power",
    "endured an absurd aquatic ceremony, and now wields Excalibur"
};

/* dip an object into a fountain when standing on its location */
void
dipfountain(struct obj *obj)
{
    int er = ER_NOTHING;
    boolean is_hands = (obj == &hands_obj);

    if (Levitation) {
        floating_above("fountain");
        return;
    }

    if (obj->otyp == LONG_SWORD && u.ulevel >= 5
        && !rn2(Role_if(PM_KNIGHT) ? 6 : 30)
        /* once upon a time it was possible to poly N daggers into N swords */
        && obj->quan == 1L && !obj->oartifact
        && !exist_artifact(LONG_SWORD, artiname(ART_EXCALIBUR))) {
        static const char lady[] = "Lady of the Lake";

        if (u.ualign.type != A_LAWFUL) {
            /* Ha!  Trying to cheat her. */
            pline("A freezing mist rises from the %s"
                  " and envelopes the sword.",
                  hliquid("water"));
            pline_The("fountain disappears!");
            curse(obj);
            if (obj->spe > -6 && !rn2(3))
                obj->spe--;
            obj->oerodeproof = FALSE;
            exercise(A_WIS, FALSE);
            livelog_printf(LL_ARTIFACT,
                           "was denied %s!  The %s has deemed %s unworthy",
                           artiname(ART_EXCALIBUR), lady, uhim());
        } else {
            /* The lady of the lake acts! - Eric Backus */
            /* Be *REAL* nice */
            pline(
              "From the murky depths, a hand reaches up to bless the sword.");
            pline("As the hand retreats, the fountain disappears!");
            obj = oname(obj, artiname(ART_EXCALIBUR),
                        ONAME_VIA_DIP | ONAME_KNOW_ARTI);
            discover_artifact(ART_EXCALIBUR);
            bless(obj);
            obj->oeroded = obj->oeroded2 = 0;
            obj->oerodeproof = TRUE;
            exercise(A_WIS, TRUE);
            livelog_printf(LL_ARTIFACT, "%s", excalmsgs[rn2(SIZE(excalmsgs))]);
        }
        update_inventory();
        set_levltyp(u.ux, u.uy, ROOM); /* updates level.flags.nfountains */
        levl[u.ux][u.uy].flags = 0;
        newsym(u.ux, u.uy);
        if (in_town(u.ux, u.uy))
            (void) angry_guards(FALSE);
        return;
    } else if (is_hands || obj == uarmg) {
        er = wash_hands();
    } else {
        er = water_damage(obj, NULL, TRUE);
    }

    if (er == ER_DESTROYED || (er != ER_NOTHING && !rn2(2))) {
        return; /* no further effect */
    }

    switch (rnd(30)) {
    case 16: /* Curse the item */
        if (!is_hands && obj->oclass != COIN_CLASS && !obj->cursed) {
            curse(obj);
        }
        break;
    case 17:
    case 18:
    case 19:
    case 20: /* Uncurse the item */
        if (!is_hands && obj->cursed) {
            if (!Blind)
                pline_The("%s glows for a moment.", hliquid("water"));
            uncurse(obj);
        } else {
            pline("A feeling of loss comes over you.");
        }
        break;
    case 21: /* Water Demon */
        dowaterdemon();
        break;
    case 22: /* Water Nymph */
        dowaternymph();
        break;
    case 23: /* an Endless Stream of Snakes */
        dowatersnakes();
        break;
    case 24: /* Find a gem */
        if (!FOUNTAIN_IS_LOOTED(u.ux, u.uy)) {
            dofindgem();
            break;
        }
        FALLTHROUGH;
        /*FALLTHRU*/
    case 25: /* Water gushes forth */
        dogushforth(FALSE);
        break;
    case 26: /* Strange feeling */
        pline("A strange tingling runs up your %s.", body_part(ARM));
        break;
    case 27: /* Strange feeling */
        You_feel("a sudden chill.");
        break;
    case 28: /* Strange feeling */
        pline("An urge to take a bath overwhelms you.");
        {
            long money = money_cnt(gi.invent);
            struct obj *otmp, *nextobj;

            if (money > 10) {
                /* Amount to lose.  Might get rounded up as fountains don't
                 * pay change... */
                money = somegold(money) / 10;
                for (otmp = gi.invent; otmp && money > 0; otmp = nextobj) {
                    nextobj = otmp->nobj;
                    if (otmp->oclass == COIN_CLASS) {
                        int denomination = objects[otmp->otyp].oc_cost;
                        long coin_loss =
                            (money + denomination - 1) / denomination;
                        coin_loss = min(coin_loss, otmp->quan);
                        otmp->quan -= coin_loss;
                        money -= coin_loss * denomination;
                        if (!otmp->quan)
                            delobj(otmp);
                    }
                }
                You("lost some of your gold in the fountain!");
                CLEAR_FOUNTAIN_LOOTED(u.ux, u.uy);
                exercise(A_WIS, FALSE);
            }
        }
        break;
    case 29: /* You see coins */
        /* We make fountains have more coins the closer you are to the
         * surface.  After all, there will have been more people going
         * by.  Just like a shopping mall!  Chris Woodbury  */

        if (FOUNTAIN_IS_LOOTED(u.ux, u.uy))
            break;
        SET_FOUNTAIN_LOOTED(u.ux, u.uy);
        (void) mkgold((long) (rnd((dunlevs_in_dungeon(&u.uz) - dunlev(&u.uz)
                                   + 1) * 2) + 5),
                      u.ux, u.uy);
        if (!Blind)
            pline("Far below you, you see coins glistening in the %s.",
                  hliquid("water"));
        exercise(A_WIS, TRUE);
        newsym(u.ux, u.uy);
        break;
    default:
        if (er == ER_NOTHING)
            pline1(nothing_seems_to_happen);
        break;
    }
    update_inventory();
    dryup(u.ux, u.uy, TRUE);
}

/* dipping '-' in fountain, pool, or sink */
int
wash_hands(void)
{
    const char *hands = makeplural(body_part(HAND));
    int res = ER_NOTHING;
    boolean was_glib = !!Glib;

    You("wash your %s%s in the %s.", uarmg ? "gloved " : "", hands,
        hliquid("water"));
    if (Glib) {
        make_glib(0);
        Your("%s are no longer slippery.", fingers_or_gloves(TRUE));
    }
    if (uarmg)
        res = water_damage(uarmg, (const char *) 0, TRUE);
    /* not what ER_GREASED is for, but the checks in dipfountain just
       compare the result to ER_DESTROYED and ER_NOTHING, so it works */
    if (was_glib && res == ER_NOTHING)
        res = ER_GREASED;
    return res;
}

/* forging recipes - first object is the end result
   of combining objects two and three */
const struct forge_recipe fusions[] = {
    /* weapons */
    { SHURIKEN, DART, DAGGER, 2, 1 },
    { SPEAR, ARROW, DAGGER, 2, 1 },
    { ELVEN_SPEAR, ELVEN_ARROW, ELVEN_DAGGER, 2, 1 },
    { ORCISH_SPEAR, ORCISH_ARROW, ORCISH_DAGGER, 2, 1 },
    { DWARVISH_SPEAR, ARROW, SPEAR, 2, 1 },
    { JAVELIN, CROSSBOW_BOLT, SPEAR, 2, 1 },
    { TRIDENT, SCIMITAR, SPEAR, 1, 1 },
    { DAGGER, ARROW, KNIFE, 2, 1 },
    { ELVEN_DAGGER, ELVEN_ARROW, KNIFE, 2, 1 },
    { ORCISH_DAGGER, ORCISH_ARROW, KNIFE, 2, 1 },
    { ATHAME, DAGGER, STILETTO, 1, 1 },
    { SCALPEL, KNIFE, STILETTO, 1, 1 },
    { KNIFE, ARROW, DART, 2, 2 },
    { STILETTO, CROSSBOW_BOLT, KNIFE, 2, 1 },
    { AXE, DAGGER, SPEAR, 1, 1 },
    { DWARVISH_BEARDED_AXE, AXE, DWARVISH_SHORT_SWORD, 1, 1 },
    { BATTLE_AXE, AXE, BROADSWORD, 1, 1 },
    { SHORT_SWORD, CROSSBOW_BOLT, DAGGER, 2, 1 },
    { ELVEN_SHORT_SWORD, CROSSBOW_BOLT, ELVEN_DAGGER, 2, 1 },
    { ORCISH_SHORT_SWORD, CROSSBOW_BOLT, ORCISH_DAGGER, 2, 1 },
    { DWARVISH_SHORT_SWORD, DWARVISH_SPEAR, SHORT_SWORD, 1, 1 },
    { SCIMITAR, KNIFE, SHORT_SWORD, 1, 1 },
    { ORCISH_SCIMITAR, KNIFE, ORCISH_SHORT_SWORD, 1, 1 },
    { SABER, SCIMITAR, LONG_SWORD, 1, 1 },
    { BROADSWORD, SCIMITAR, SHORT_SWORD, 1, 1 },
    { ELVEN_BROADSWORD, SCIMITAR, ELVEN_SHORT_SWORD, 1, 1 },
    { LONG_SWORD, SHORT_SWORD, SHORT_SWORD, 1, 1 },
    { ELVEN_LONG_SWORD, ELVEN_SHORT_SWORD, ELVEN_SHORT_SWORD, 1, 1 },
    { ORCISH_LONG_SWORD, ORCISH_SHORT_SWORD, ORCISH_SHORT_SWORD, 1, 1 },
    { TWO_HANDED_SWORD, LONG_SWORD, BROADSWORD, 1, 1 },
    { KATANA, LONG_SWORD, LONG_SWORD, 1, 1 },
    { TSURUGI, TWO_HANDED_SWORD, KATANA, 1, 1 },
    { RUNESWORD, BROADSWORD, DAGGER, 1, 1 },
    { PARTISAN, BROADSWORD, SPEAR, 1, 1 },
    { RANSEUR, STILETTO, SPEAR, 1, 1 },
    { SPETUM, KNIFE, SPEAR, 1, 1 },
    { GLAIVE, SHORT_SWORD, SPEAR, 1, 1 },
    { LANCE, JAVELIN, GLAIVE, 1, 1 },
    { HALBERD, RANSEUR, AXE, 1, 1 },
    { BARDICHE, BATTLE_AXE, SPEAR, 1, 1 },
    { VOULGE, AXE, SPEAR, 1, 1 },
    { DWARVISH_MATTOCK, PICK_AXE, DWARVISH_SHORT_SWORD, 1, 1 },
    { FAUCHARD, SABER, SPEAR, 1, 1 },
    { GUISARME, GRAPPLING_HOOK, SPEAR, 1, 1 },
    { BILL_GUISARME, GUISARME, SPEAR, 1, 1 },
    { LUCERN_HAMMER, HEAVY_WAR_HAMMER, SPEAR, 1, 1 },
    { BEC_DE_CORBIN, WAR_HAMMER, SPEAR, 1, 1 },
    { MACE, CLUB, DAGGER, 1, 1 },
    { HEAVY_MACE, MACE, MACE, 1, 1 },
    { ROD, RUBY, MACE, 2, 1 },
    { MORNING_STAR, MACE, DAGGER, 1, 1 },
    { ORCISH_MORNING_STAR, MACE, ORCISH_DAGGER, 1, 1 },
    { WAR_HAMMER, MACE, FLAIL, 1, 1 },
    { HEAVY_WAR_HAMMER, WAR_HAMMER, WAR_HAMMER, 1, 1 },
    { CLUB, ARROW, CROSSBOW_BOLT, 2, 2 },
    { AKLYS, FLAIL, CLUB, 1, 1 },
    { FLAIL, MORNING_STAR, MACE, 1, 1 },
    /* armor (helmets) */
    { ORCISH_HELM, DENTED_POT, ORCISH_DAGGER, 1, 1 },
    { DWARVISH_HELM, HELMET, DWARVISH_SHORT_SWORD, 1, 1 },
    { DENTED_POT, WAR_HAMMER, KNIFE, 1, 1 },
    { HELMET, DENTED_POT, DAGGER, 1, 1 },
    /* armor (body armor) */
    { PLATE_MAIL, SPLINT_MAIL, CHAIN_MAIL, 1, 1 },
    { CRYSTAL_PLATE_MAIL, DILITHIUM_CRYSTAL, PLATE_MAIL, 3, 1 },
    { SPLINT_MAIL, SCALE_MAIL, CHAIN_MAIL, 1, 1 },
    { LARGE_SPLINT_MAIL, PLATE_MAIL, PLATE_MAIL, 1, 1 },
    { BANDED_MAIL, SCALE_MAIL, RING_MAIL, 1, 1 },
    { CHAIN_MAIL, RING_MAIL, RING_MAIL, 1, 1 },
    { DWARVISH_CHAIN_MAIL, CHAIN_MAIL, DWARVISH_ROUNDSHIELD, 1, 1 },
    { ELVEN_CHAIN_MAIL, CHAIN_MAIL, ELVEN_SHIELD, 1, 1 },
    { ORCISH_CHAIN_MAIL, RING_MAIL, ORCISH_SHIELD, 1, 1 },
    { SCALE_MAIL, RING_MAIL, HELMET, 1, 1 },
    { RING_MAIL, LARGE_SHIELD, HELMET, 1, 1 },
    { ORCISH_RING_MAIL, ORCISH_SHIELD, ORCISH_HELM, 1, 1 },
    /* armor (shields) */
    { SMALL_SHIELD, DAGGER, HELMET, 1, 1 },
    { ELVEN_SHIELD, ELVEN_DAGGER, SMALL_SHIELD, 1, 1 },
    { URUK_HAI_SHIELD, ORCISH_SHIELD, ORCISH_SHIELD, 1, 1 },
    { ORCISH_SHIELD, ORCISH_HELM, ORCISH_BOOTS, 1, 1 },
    { LARGE_SHIELD, HELMET, HELMET, 1, 1 },
    { DWARVISH_ROUNDSHIELD, LARGE_SHIELD, DWARVISH_HELM, 1, 1 },
    { BRACERS, SMALL_SHIELD, HELMET, 1, 1 },
    /* armor (gauntlets and boots) */
    { GAUNTLETS, MACE, HELMET, 1, 1 },
    { DWARVISH_BOOTS, GAUNTLETS, DWARVISH_SHORT_SWORD, 1, 1 },
    { ORCISH_BOOTS, GAUNTLETS, ORCISH_SHORT_SWORD, 1, 1 },
    /* barding for steeds */
    { BARDING, PLATE_MAIL, SADDLE, 1, 1 },
    { SPIKED_BARDING, BARDING, MORNING_STAR, 1, 1 },
    { BARDING_OF_REFLECTION, BARDING, SHIELD_OF_REFLECTION, 1, 1 },
    /* sling bullets */
    { SLING_BULLET, ROCK, DART, 3, 1 },
    /* Drow weapons and armor - requires drow objects
       to forge other drow objects */
    { DARK_ELVEN_SPEAR, DARK_ELVEN_ARROW, DARK_ELVEN_DAGGER, 2, 1 },
    { DARK_ELVEN_DAGGER, DARK_ELVEN_ARROW, DARK_ELVEN_CROSSBOW_BOLT, 2, 2 },
    { DARK_ELVEN_SHORT_SWORD, DARK_ELVEN_CROSSBOW_BOLT,
      DARK_ELVEN_DAGGER, 2, 1 },
    { DARK_ELVEN_BROADSWORD, DARK_ELVEN_DAGGER,
      DARK_ELVEN_SHORT_SWORD, 1, 1 },
    { DARK_ELVEN_LONG_SWORD, DARK_ELVEN_SHORT_SWORD,
      DARK_ELVEN_SHORT_SWORD, 1, 1 },
    { DARK_ELVEN_MACE, DARK_ELVEN_DAGGER, DARK_ELVEN_DAGGER, 1, 1 },
    { DARK_ELVEN_HEAVY_MACE, DARK_ELVEN_MACE, DARK_ELVEN_MACE, 1, 1 },
    { DARK_ELVEN_CHAIN_MAIL, DARK_ELVEN_HEAVY_MACE,
      DARK_ELVEN_BRACERS, 1, 1 },
    { DARK_ELVEN_BRACERS, DARK_ELVEN_DAGGER, DARK_ELVEN_MACE, 1, 1 },
    { 0, 0, 0, 0, 0 }
};

static const struct forge_arti {
    short result_typ;
    short typ1;
    short typ2;
} artifusions[] = {
    /* artifacts */

    /* the sword of annihilation has two different recipes; either
       body part from Vecna will suffice in its creation */
    { ART_SWORD_OF_ANNIHILATION, ART_ANGELSLAYER, ART_EYE_OF_VECNA },
    { ART_SWORD_OF_ANNIHILATION, ART_ANGELSLAYER, ART_HAND_OF_VECNA },
    /* forged artifacts made from existing regular artifacts */
    { ART_GLAMDRING, ART_ORCRIST, ART_STING },
    { ART_STAFF_OF_THE_ARCHMAGI, ART_MAGICBANE, ART_SECESPITA },
    { ART_SHADOWBLADE, ART_STORMBRINGER, ART_WEREBANE },
    { ART_GAUNTLETS_OF_PURITY, ART_DRAGONBANE, ART_GRAYSWANDIR },
    { ART_ASHMAR, ART_TROLLSBANE, ART_OGRESMASHER },
    { ART_HAMMER_OF_THE_GODS, ART_SUNSWORD, ART_DEMONBANE },
    { ART_TEMPEST, ART_CLEAVER, ART_MJOLLNIR },
    { ART_DICHOTOMY, ART_FIRE_BRAND, ART_FROST_BRAND },
    { ART_ITHILMAR, ART_VORPAL_BLADE, ART_DRAMBORLEG },
    { ART_HARBINGER, ART_GIANTSLAYER, ART_KEOLEWA },
    /* forged artifacts that require at least one other forged
       artifact to be created */
    { ART_ARMOR_OF_RETRIBUTION, ART_SWORD_OF_ANNIHILATION, ART_ASHMAR },
    { 0, 0, 0 }
};

/* Get forge recipe for an artifact by index. Returns TRUE if recipe
   found at given index. If found, sets *typ1 and *typ2 to the input
   artifact indices. Use idx=0 for first recipe, idx=1 for second, etc.
   Some artifacts (e.g. Sword of Annihilation) have multiple recipes */
boolean
get_forge_recipe(int artinum, int idx, int *typ1, int *typ2)
{
    const struct forge_arti *arti;
    int count = 0;

    for (arti = artifusions; arti->result_typ; arti++) {
        if (arti->result_typ == artinum) {
            if (count == idx) {
                *typ1 = arti->typ1;
                *typ2 = arti->typ2;
                return TRUE;
            }
            count++;
        }
    }
    return FALSE;
}

/* Gemstone affixing helper functions */

/* Returns the enchantment bonus for an affixed gem (0-3).
   Property gems return 0 (they grant oprops instead) */
int
get_gem_enchant_bonus(int gem_otyp)
{
    if (!gem_otyp)
        return 0;

    /* property gems: no enchant bonus */
    switch (gem_otyp) {
    case DIAMOND:
    case RUBY:
    case SAPPHIRE:
    case EMERALD:
    case LUCKSTONE:
        return 0;
    }

    /* premium tier */
    if (gem_otyp == DILITHIUM_CRYSTAL)
        return 3;

    /* mid tier */
    switch (gem_otyp) {
    case BLACK_OPAL:
    case JACINTH:
    case TURQUOISE:
    case CITRINE:
    case AQUAMARINE:
        return 2;
    }

    /* lower tier: all remaining valid gems */
    if (gem_otyp >= FIRST_REAL_GEM && gem_otyp <= LAST_REAL_GEM)
        return 1;

    return 0;
}

/* Returns the item property granted by an affixed gem, or 0 */
long
get_gem_property(int gem_otyp)
{
    switch (gem_otyp) {
    case DIAMOND:
        return ITEM_SHOCK;
    case RUBY:
        return ITEM_FIRE;
    case SAPPHIRE:
        return ITEM_FROST;
    case EMERALD:
        return ITEM_VENOM;
    case LUCKSTONE:
        return ITEM_EXCEL;
    }
    return 0L;
}

/* Returns the enchantment bonus for an object's affixed gem */
int
gem_enchant_bonus(struct obj *obj)
{
    if (!obj || !has_affixed_gem(obj))
        return 0;
    return get_gem_enchant_bonus(obj->affixed_gem);
}

/* Returns TRUE if the object is a valid gemstone for affixing.
   Real gems (dilithium through jade) plus luckstone */
boolean
is_affix_gem(struct obj *obj)
{
    if (!obj)
        return FALSE;
    if (obj->otyp >= FIRST_REAL_GEM && obj->otyp <= LAST_REAL_GEM)
        return TRUE;
    if (obj->otyp == LUCKSTONE)
        return TRUE;
    return FALSE;
}

/* getobj callback for forge ingredients; suggest objects that satisfy
   the forge's material requirements but let anything be chosen so the
   full validation feedback below can explain any rejection */
staticfn int
forge_ok(struct obj *obj)
{
    if (!obj)
        return GETOBJ_EXCLUDE;
    if (is_metallic(obj) || is_crystal(obj) || bypass_forging_rules(obj))
        return GETOBJ_SUGGEST;
    return GETOBJ_DOWNPLAY;
}

/* getobj callback for the weapon receiving an affixed gemstone */
staticfn int
affix_weapon_ok(struct obj *obj)
{
    if (!obj)
        return GETOBJ_EXCLUDE;
    if (obj->oclass == WEAPON_CLASS || is_weptool(obj))
        return GETOBJ_SUGGEST;
    return GETOBJ_DOWNPLAY;
}

/* getobj callback for the gemstone being affixed */
staticfn int
affix_gem_ok(struct obj *obj)
{
    if (!obj || obj->oclass != GEM_CLASS)
        return GETOBJ_EXCLUDE;
    return GETOBJ_SUGGEST;
}

/* Success roll for affixing a gem to a weapon.
   Base 30%, modified by hammer BUC, gem tier, identification,
   player DEX and Luck. Clamped to 5-95% */
staticfn boolean
affix_success_roll(struct obj *gem)
{
    int chance = 30; /* base */
    int tier = get_gem_enchant_bonus(gem->otyp);

    /* hammer blessing/curse */
    if (uwep && uwep->blessed)
        chance += 15;
    if (uwep && uwep->cursed)
        chance -= 15;

    /* gem tier - harder gems are harder to work.
       property gems (tier 0) and mid (tier 2): no modifier */
    if (tier == 3)
        chance -= 10; /* premium: harder */
    else if (tier == 1)
        chance += 10; /* lower: easier */

    /* unidentified gem penalty */
    if (!objects[gem->otyp].oc_name_known)
        chance -= 20;

    /* player attributes */
    chance += (ACURR(A_DEX) - 10);
    chance += Luck;

    /* clamp 5-95% */
    if (chance < 5)
        chance = 5;
    if (chance > 95)
        chance = 95;

    return (rn2(100) < chance);
}

/* the #forge command */
int
doforging(void)
{
    const struct forge_recipe *recipe;
    const struct forge_arti *arti;
    struct obj *obj1, *obj2, *output;
    int objtype = 0, artitype = 0;
    boolean swapped = FALSE; /* True if obj2 matched typ1 and vice versa */
    winid win;
    anything any;
    menu_item *selected;
    int n, clr = NO_COLOR;

    /* first, we need a forge */
    if (!IS_FORGE(levl[u.ux][u.uy].typ)) {
        You("need a forge in order to forge objects.");
        return ECMD_OK;
    }

    /* next, the proper tool to do the job */
    if (!uwep || uwep->otyp != BLACKSMITH_HAMMER) {
        pline("You'll need a blacksmith hammer to forge successfully.");
        return ECMD_OK;
    }

    /* ask what to do at the forge */
    win = create_nhwindow(NHW_MENU);
    start_menu(win, MENU_BEHAVE_STANDARD);
    any = cg.zeroany;
    any.a_int = 1;
    add_menu(win, &nul_glyphinfo, &any, 'a', 0, ATR_NONE, clr,
             "Combine two objects", MENU_ITEMFLAGS_NONE);
    any.a_int = 2;
    add_menu(win, &nul_glyphinfo, &any, 'b', 0, ATR_NONE, clr,
             "Affix gemstone to weapon", MENU_ITEMFLAGS_NONE);
    end_menu(win, "What do you want to do at the forge?");
    n = select_menu(win, PICK_ONE, &selected);
    destroy_nhwindow(win);
    if (n <= 0)
        return ECMD_CANCEL;
    if (selected[0].item.a_int == 2) {
        free((genericptr_t) selected);
        return doaffixgem();
    }
    free((genericptr_t) selected);

    /* using the hammer involves touching it; check for material hatred
       (e.g. an elf with an iron hammer) - same check as doapply() */
    if (!retouch_object(&uwep, !uarmg, FALSE))
        return ECMD_TIME; /* costs a turn */

    /* various player conditions can prevent successful forging */
    if (Stunned || Confusion) {
        You_cant("use the forge while incapacitated.");
        return ECMD_OK;
    } else if (u.uhunger < 50) { /* weak */
        You("are too weak from hunger to use the forge.");
        return ECMD_OK;
    } else if (ACURR(A_STR) < 4) {
        You("lack the strength to use the forge.");
        return ECMD_OK;
    }

    /* setup the base object */
    obj1 = getobj("use as a base", forge_ok, GETOBJ_PROMPT);
    if (!obj1) {
        You("need a base object to forge with.");
        return ECMD_CANCEL;
    } else if (!(is_metallic(obj1) || is_crystal(obj1)
                 || bypass_forging_rules(obj1))) {
        /* object should be gemstone or metallic */
        pline_The("base object is not a valid material.");
        return ECMD_OK;
    }

    /* setup the secondary object */
    obj2 = getobj("combine with the base object", forge_ok, GETOBJ_PROMPT);
    if (!obj2) {
        You("need more than one object.");
        return ECMD_CANCEL;
    } else if (!(is_metallic(obj2) || is_crystal(obj2)
                 || bypass_forging_rules(obj2))) {
        /* secondary object should also be gemstone or metallic */
        pline_The("secondary object is not a valid material.");
        return ECMD_OK;
    }

    /* handle illogical cases */
    if (obj1 == obj2) {
        You_cant("combine an object with itself!");
        return ECMD_OK;
    /* not that the Amulet of Yendor or invocation items would
       ever be part of a forging recipe, but these should be
       protected in any case */
    } else if ((obj_resists(obj1, 0, 0)
                && obj1->oartifact != ART_SWORD_OF_ANNIHILATION)
               || (obj_resists(obj2, 0, 0)
                   && obj2->oartifact != ART_SWORD_OF_ANNIHILATION)) {
        You_cant("forge such a thing!");
        blowupforge(u.ux, u.uy);
        return ECMD_TIME;
    /* worn or wielded objects */
    } else if (is_worn(obj1) || is_worn(obj2)) {
        You("must first %s the objects you wish to forge.",
            ((obj1->owornmask & W_ARMOR)
             || (obj2->owornmask & W_ARMOR)) ? "remove" : "unwield");
        return ECMD_OK;
    /* artifacts can never be applied to a non-artifact base */
    } else if ((obj2->oartifact && !obj1->oartifact)
               || (obj1->oartifact && !obj2->oartifact)) {
        pline("Artifacts cannot be forged with lesser objects.");
        return ECMD_OK;
    /* dragon-scaled armor can never be fully metallic */
    } else if (Is_dragon_scaled_armor(obj1)
               || Is_dragon_scaled_armor(obj2)) {
        pline("Dragon-scaled armor cannot be forged.");
        return ECMD_OK;
    /* due to the nature of drow objects, they can only
       be forged in darkness */
    } else if (!spot_is_dark(u.ux, u.uy)
               && (is_drow_obj(obj1) || is_drow_obj(obj2))) {
        pline("Drowcraft must be performed under the cover of darkness.");
        return ECMD_OK;
    } else if ((obj1->oartifact || obj2->oartifact)
               && uwep && uwep->otyp == BLACKSMITH_HAMMER
               && uwep->spe <= 0) {
        Your("%s requires a magical charge to forge artifacts.",
             simpleonames(uwep));
        return ECMD_OK;
    }

    /* start the forging process */
    if (obj1->oartifact && obj2->oartifact) {
        for (arti = artifusions; arti->result_typ; arti++) {
            if ((obj1->oartifact == arti->typ1
                 && obj2->oartifact == arti->typ2)
                || (obj2->oartifact == arti->typ1
                    && obj1->oartifact == arti->typ2)) {
                artitype = arti->result_typ;
                break;
            }
        }

        if (!artitype) {
            /* if the objects used do not match the recipe array,
               the forging process fails */
            You("fail to combine these two objects.");
            return ECMD_TIME;
        } else {
            /* success */
            You("place %s, then %s inside the forge.",
                the(xname(obj1)), the(xname(obj2)));
            pline("Raising your %s, you begin to forge the artifacts together...",
                  xname(uwep));

            output = mk_particular_artifact(artitype);
            if (!output) {
                /* shouldn't happen; recipes only produce artifacts */
                You("fail to combine these two objects.");
                return ECMD_TIME;
            }

            /* if objects are enchanted or have charges,
               carry that over, and use the greater of the two */
            if (output->oclass == obj2->oclass) {
                output->spe = obj2->spe;
                if (output->oclass == obj1->oclass)
                    output->spe = max(output->spe, obj1->spe);
            } else if (output->oclass == obj1->oclass) {
                output->spe = obj1->spe;
            }

            /* transfer curses and blessings from secondary object */
            output->cursed = obj2->cursed;
            output->blessed = obj2->blessed;
            /* ensure the final product is not degraded or coated
               with anything in any way */
            output->oeroded = output->oeroded2 = 0;
            output->opoisoned = 0;
            output->otainted = 0;
            output->greased = 0;
            maybe_erodeproof(output, 1);
            /* artifacts should never have a quality modifier */
            output->forged_qual = FQ_NORMAL;

            /* delete recipe objects - use delobj_core() directly
               to handle the Sword of Annihilation potentially
               being one of the recipe objects */
            delobj_core(obj1, TRUE);
            delobj_core(obj2, TRUE);

            /* forged object is created */
            output = addinv(output);
            output->owt = weight(output);
            You("have successfully forged %s.", doname(output));
            uwep->spe--;
            u.uconduct.forgedarti++;
            /* artifact materialized in the hero's hands; mirror the
               explicit bump used by every other "creation in your
               possession" path (fountain dip, prayer gift, sacrifice
               gift) since addinv() does not route through
               touch_artifact() */
            u.uconduct.artitouch++;
            livelog_printf(LL_ARTIFACT, "used a forge to create %s%s",
                           (output->oartifact == ART_GAUNTLETS_OF_PURITY
                            || output->oartifact == ART_HAMMER_OF_THE_GODS
                            || output->oartifact == ART_ARMOR_OF_RETRIBUTION)
                           ? "the " : "",
                           artiname(output->oartifact));
            update_inventory();

            /* special events that may happen when a
               particular artifact is forged go here */
            if (output->oartifact == ART_ITHILMAR) {
                /* forging Ithilmar improves mounted combat mastery */
                if (!P_RESTRICTED(P_RIDING)
                    && P_MAX_SKILL(P_RIDING) < P_EXPERT)
                    P_MAX_SKILL(P_RIDING)++;
                if (P_MAX_SKILL(P_PET_HANDLING) < P_MASTER)
                    P_MAX_SKILL(P_PET_HANDLING)++;
            }

            /* forging an artifact is too much stress for the forge */
            coolforge(u.ux, u.uy);
        }
    } else { /* regular objects */
        for (recipe = fusions; recipe->result_typ; recipe++) {
            /* check if obj1 matches typ1 and obj2 matches typ2 */
            if (obj1->otyp == recipe->typ1
                && obj2->otyp == recipe->typ2
                && obj1->quan >= recipe->quan_typ1
                && obj2->quan >= recipe->quan_typ2) {
                objtype = recipe->result_typ;
                swapped = FALSE;
                break;
            }
            /* check if obj2 matches typ1 and obj1 matches typ2 (swapped) */
            if (obj2->otyp == recipe->typ1
                && obj1->otyp == recipe->typ2
                && obj2->quan >= recipe->quan_typ1
                && obj1->quan >= recipe->quan_typ2) {
                objtype = recipe->result_typ;
                swapped = TRUE;
                break;
            }
        }
        if (!objtype) {
            /* if the objects used do not match the recipe array,
               the forging process fails */
            You("fail to combine these two objects.");
            return ECMD_TIME;
        } else {
            /* success */
            output = mksobj(objtype, TRUE, FALSE);

            /* try to take on the material from one of the source objects */
            if (valid_obj_material(output, obj2->material)) {
                set_material(output, obj2->material);
            } else if (valid_obj_material(output, obj1->material)) {
                set_material(output, obj1->material);
            } else {
                /* neither material is valid for the output object, so
                   can't forge them successfully */
                pline_The("%s and %s resist melding in the forge.",
                          materialnm[obj1->material],
                          materialnm[obj2->material]);
                You("fail to combine the two objects.");
                delobj(output);
                return ECMD_TIME;
            }

            You("place %s, then %s inside the forge.",
                the(xname(obj1)), the(xname(obj2)));
            pline("Raising your %s, you begin to forge the objects together...",
                  xname(uwep));

            /* any object properties, take secondary object property
               over primary. if you know the object property of one
               of the recipe objects, you'll know the object property
               of the newly forged object */
            if (obj2->oprops) {
                if (!is_barding(output))
                    output->oprops = obj2->oprops;
                if (obj2->oprops_known)
                    output->oprops_known = output->oprops;
            } else if (obj1->oprops) {
                if (!is_barding(output))
                    output->oprops = obj1->oprops;
                if (obj1->oprops_known)
                    output->oprops_known = output->oprops;
            }

            /* if both recipe objects have gem-granted oprops,
               randomly pick one instead of always taking secondary */
            if (has_affixed_gem(obj1) && has_affixed_gem(obj2)) {
                long prop1 = get_gem_property(obj1->affixed_gem);
                long prop2 = get_gem_property(obj2->affixed_gem);

                if (prop1 && prop2) {
                    output->oprops &= ~(prop1 | prop2);
                    output->oprops |= rn2(2) ? prop1 : prop2;
                }
            }
            /* affixed_gem is already 0 on new mksobj() output */

            /* if neither recipe object have an object property,
               ensure that the newly forged object doesn't
               randomly have a property added at creation */
            if ((obj1->oprops & ITEM_PROP_MASK) == 0L
                && (obj2->oprops & ITEM_PROP_MASK) == 0L)
                output->oprops = 0L;

            /* if objects are enchanted or have charges,
               carry that over, and use the greater of the two */
            if (output->oclass == obj2->oclass) {
                output->spe = obj2->spe;
                if (output->oclass == obj1->oclass)
                    output->spe = max(output->spe, obj1->spe);
            } else if (output->oclass == obj1->oclass) {
                output->spe = obj1->spe;
            }

            /* transfer curses and blessings from secondary object */
            output->cursed = obj2->cursed;
            output->blessed = obj2->blessed;
            /* ensure the final product is not degraded or coated
               with anything in any way */
            output->oeroded = output->oeroded2 = 0;
            output->opoisoned = 0;
            output->otainted = 0;
            output->greased = 0;

            /* if the blacksmith hammer is blessed, there
               is a chance the armor or weapon forged is of a
               higher quality, but if cursed, the quality could
               be sub-par.

               Superior or exceptional gear can be guaranteed,
               but at a cost.

               Only weapons, armor, and barding can have forged quality.
               Gems, tools, rocks etc are always FQ_NORMAL. For recipes
               using such items (e.g. crystal plate mail = dilithium +
               plate mail), only check quality/spe of the qualifying
               ingredient so players can still achieve
               superior/exceptional */
            {
                boolean obj1_can_qual = (obj1->oclass == ARMOR_CLASS
                                         || obj1->oclass == WEAPON_CLASS
                                         || is_barding(obj1));
                boolean obj2_can_qual = (obj2->oclass == ARMOR_CLASS
                                         || obj2->oclass == WEAPON_CLASS
                                         || is_barding(obj2));
                int qual1 = obj1_can_qual ? (int) obj1->forged_qual
                                          : FQ_NORMAL;
                int qual2 = obj2_can_qual ? (int) obj2->forged_qual
                                          : FQ_NORMAL;
                int spe1 = obj1_can_qual ? obj1->spe : 0;
                int spe2 = obj2_can_qual ? obj2->spe : 0;
                /* for threshold checks: if object can't have quality,
                   it doesn't block the other object's contribution */
                boolean chk1 = obj1_can_qual;
                boolean chk2 = obj2_can_qual;

                if (uwep->blessed) {
                    if ((chk1 && qual1 == FQ_INFERIOR)
                        || (chk2 && qual2 == FQ_INFERIOR)) {
                        /* if either qualifying object is inferior,
                           the output will also be inferior */
                        output->forged_qual = FQ_INFERIOR;
                    } else if ((!chk1 || qual1 == FQ_EXCEPTIONAL)
                               && (!chk2 || qual2 == FQ_EXCEPTIONAL)
                               && (chk1 || chk2)) {
                        /* if all qualifying objects are exceptional,
                           the forged object will also be exceptional */
                        output->forged_qual = FQ_EXCEPTIONAL;
                    } else if ((!chk1 || qual1 >= FQ_SUPERIOR)
                               && (!chk2 || qual2 >= FQ_SUPERIOR)
                               && (chk1 || chk2)) {
                        /* if all qualifying objects are at least
                           superior, the forged object will be superior
                           or exceptional */
                        output->forged_qual = (!rn2(10) ? FQ_EXCEPTIONAL
                                                        : FQ_SUPERIOR);
                    } else if ((!chk1 || spe1 >= 5)
                               && (!chk2 || spe2 >= 5)
                               && (chk1 || chk2)) {
                        /* qualifying objects need enchantment of 5+
                           for exceptional output */
                        output->forged_qual = FQ_EXCEPTIONAL;
                        output->spe = 0;
                    } else if ((!chk1 || spe1 >= 3)
                               && (!chk2 || spe2 >= 3)
                               && (chk1 || chk2)) {
                        /* qualifying objects need enchantment of 3+
                           for superior output */
                        output->forged_qual = FQ_SUPERIOR;
                        output->spe = 0;
                    } else {
                        /* random */
                        if (!rn2(10))
                            output->forged_qual = (!rn2(10) ? FQ_EXCEPTIONAL
                                                            : FQ_SUPERIOR);
                    }
                } else { /* hammer is uncursed or cursed */
                    if ((chk1 && qual1 == FQ_INFERIOR)
                        || (chk2 && qual2 == FQ_INFERIOR)) {
                        /* if either qualifying object is inferior,
                           output will be also */
                        output->forged_qual = FQ_INFERIOR;
                    } else if ((!chk1 || spe1 < 0)
                               && (!chk2 || spe2 < 0)
                               && (chk1 || chk2)) {
                        /* qualifying objects with negative enchantment
                           ensure inferior output */
                        output->forged_qual = FQ_INFERIOR;
                    } else {
                        if (rn2(5) && uwep->cursed)
                            output->forged_qual = FQ_INFERIOR;
                    }
                }
            }

            /* toss out old objects, add new one; use swapped to
               determine which object matched which recipe slot */
            if (!swapped) {
                obj1->quan -= recipe->quan_typ1;
                obj2->quan -= recipe->quan_typ2;
            } else {
                obj2->quan -= recipe->quan_typ1;
                obj1->quan -= recipe->quan_typ2;
            }

            /* recalculate weight of the recipe objects if
               using a stack */
            if (obj1->quan > 0)
                obj1->owt = weight(obj1);
            if (obj2->quan > 0)
                obj2->owt = weight(obj2);

            /* delete recipe objects if quantity reaches zero */
            if (obj1->quan <= 0)
                delobj(obj1);
            if (obj2->quan <= 0)
                delobj(obj2);

            /* prevent large stacks of ammo-type weapons from being
               formed; cap quantity BEFORE addinv() to avoid destroying
               items from a pre-existing stack if the output merges */
            if (output->quan > 3L) {
                output->quan = 3L;
                if (!rn2(4)) /* small chance of an extra */
                    output->quan += 1L;
            }
            output->owt = weight(output);

            /* forged object is created */
            output = addinv(output);
            You("have successfully forged %s.", doname(output));
            update_inventory();
            if (output->oprops) {
                /* forging magic can sometimes be too much stress */
                if (!rn2(6))
                    coolforge(u.ux, u.uy);
                else
                    pline_The("lava in the forge bubbles ominously.");
            }
        }
    }

    return ECMD_TIME;
}

/* affix a gemstone to a weapon at a forge; reached via #forge */
int
doaffixgem(void)
{
    struct obj *weapon, *gem;
    long prop;
    boolean body_part;

    /* preconditions - same as forging */
    if (!IS_FORGE(levl[u.ux][u.uy].typ)) {
        You("need a forge in order to affix a gemstone.");
        return ECMD_OK;
    }
    if (!uwep || uwep->otyp != BLACKSMITH_HAMMER) {
        pline("You'll need a blacksmith hammer to affix a gemstone.");
        return ECMD_OK;
    }
    if (!retouch_object(&uwep, !uarmg, FALSE))
        return ECMD_TIME; /* costs a turn */
    if (Stunned || Confusion) {
        You_cant("use the forge while incapacitated.");
        return ECMD_OK;
    } else if (u.uhunger < 50) {
        You("are too weak from hunger to use the forge.");
        return ECMD_OK;
    } else if (ACURR(A_STR) < 4) {
        You("lack the strength to use the forge.");
        return ECMD_OK;
    }

    /* select weapon */
    weapon = getobj("affix a gemstone to", affix_weapon_ok, GETOBJ_PROMPT);
    if (!weapon)
        return ECMD_CANCEL;

    /* validate weapon */
    if (weapon->oclass != WEAPON_CLASS && !is_weptool(weapon)) {
        You("can only affix gemstones to weapons.");
        return ECMD_OK;
    }
    if (is_launcher(weapon) || is_ammo(weapon) || is_missile(weapon)) {
        pline("That type of weapon is unsuitable for gemstone affixing.");
        return ECMD_OK;
    }
    body_part = (weapon->otyp == WORM_TOOTH
                 || weapon->otyp == CRYSKNIFE
                 || weapon->otyp == UNICORN_HORN);
    if (!(is_metallic(weapon) || is_crystal(weapon) || body_part)) {
        pline("%s is not a suitable material for forge work.",
              Yname2(weapon));
        return ECMD_OK;
    }
    if (body_part && !weapon->oerodeproof) {
        pline("%s is too fragile to withstand the forge's heat.",
              Yname2(weapon));
        return ECMD_OK;
    }
    if (has_affixed_gem(weapon)) {
        pline("%s already has a gemstone affixed.", Yname2(weapon));
        return ECMD_OK;
    }
    if (weapon->oartifact) {
        pline("%s resists modification.", The(xname(weapon)));
        return ECMD_OK;
    }

    /* select gemstone */
    gem = getobj("use", affix_gem_ok, GETOBJ_PROMPT);
    if (!gem)
        return ECMD_CANCEL;

    /* must be GEM_CLASS */
    if (gem->oclass != GEM_CLASS) {
        pline("That is not a suitable gemstone for affixing.");
        return ECMD_OK;
    }

    /* worthless glass, gray stones (not luckstone), rocks, sling bullets,
       cursed gems: always shatters, 1/3 chance destroys weapon */
    if (!is_affix_gem(gem) || gem->cursed) {
        You("carefully attempt to affix %s to %s...",
            the(singular(gem, xname)), the(xname(weapon)));
        pline_The("%s shatters!", singular(gem, xname));
        useup(gem);
        if (!rn2(3)) {
            pline("%s is destroyed!", Yname2(weapon));
            useup(weapon);
        }
        return ECMD_TIME;
    }

    /* check for property conflict */
    prop = get_gem_property(gem->otyp);
    if (prop && (weapon->oprops & ITEM_PROP_MASK)) {
        pline("%s already has a special property.", The(xname(weapon)));
        return ECMD_OK;
    }

    /* body-part weapons risk destruction from forge heat even when
       erodeproofed: worm tooth 75%, crysknife/unicorn horn 30% */
    if (body_part) {
        int burn_chance = (weapon->otyp == WORM_TOOTH) ? 75 : 30;

        You("carefully attempt to affix %s to %s...",
            the(singular(gem, xname)), the(xname(weapon)));
        if (rnd(100) <= burn_chance) {
            pline_The("forge's intense heat consumes %s!",
                      Yname2(weapon));
            useup(gem);
            useup(weapon);
            return ECMD_TIME;
        }
        /* survived the heat - fall through to success roll */
    }

    /* attempt roll */
    if (!body_part)
        You("carefully attempt to affix %s to %s...",
            the(singular(gem, xname)), the(xname(weapon)));

    if (!affix_success_roll(gem)) {
        /* failure: gem shatters, does NOT identify the gem */
        pline_The("%s shatters!", singular(gem, xname));
        useup(gem);
        return ECMD_TIME;
    }

    /* success: identifies the gem */
    makeknown(gem->otyp);
    weapon->affixed_gem = gem->otyp;
    if (prop) {
        weapon->oprops |= prop;
        weapon->oprops_known |= prop;
    }
    useup(gem);
    if (uwep)
        uwep->spe--;

    pline("%s glows brilliantly from the newly affixed gemstone%s!",
          Yname2(weapon),
          prop ? " and pulses with power" : "");

    update_inventory();
    return ECMD_TIME;
}

/* a forge breaks apart into a lava pool */
void
breakforge(coordxy x, coordxy y)
{
    if (cansee(x, y) || u_at(x, y))
        pline_The("forge splits in two as molten lava rushes forth!");
    levl[x][y].flags = 0;
    /* updates level.flags.nforges */
    set_levltyp(x, y, LAVAPOOL);
    newsym(x, y);
}

/* a forge violently explodes */
void
blowupforge(coordxy x, coordxy y)
{
    if (cansee(x, y) || u_at(x, y))
        pline_The("forge rumbles, then explodes!  Molten lava splashes everywhere!");
    /* updates level.flags.nforges */
    set_levltyp(x, y, ROOM);
    levl[x][y].flags = 0;
    newsym(x, y);
/* ZT_FIRE = AD_FIRE-1 = 1 */
#define ZT_FORGE_FIRE 1 /* value kludge, see zap.c */
    explode(x, y, ZT_FORGE_FIRE, rnd(30), FORGE_EXPLODE, EXPL_FIERY);
#undef ZT_FORGE_FIRE
}

/* the lava in a forge cools and hardens; the forge is gone */
void
coolforge(coordxy x, coordxy y)
{
    if (cansee(x, y) || u_at(x, y))
        pline_The("lava in the forge cools and solidifies.");
    /* updates level.flags.nforges */
    set_levltyp(x, y, ROOM);
    levl[x][y].flags = 0;
    newsym(x, y);
}

/* quaff from a forge when standing on its location */
void
drinkforge(void)
{
    if (Levitation) {
        floating_above("forge");
        return;
    }

    if (!likes_fire(gy.youmonst.data)) {
        pline("Molten lava incinerates its way down your gullet...");
        losehp(Upolyd ? u.mh : u.uhp, "drinking molten lava", KILLED_BY);
        return;
    }
    burn_away_slime();
    switch (rn2(20)) {
    case 0:
        You("drink some molten lava.  Mmmmm mmm!");
        u.uhunger += rnd(50);
        break;
    case 1:
        breakforge(u.ux, u.uy);
        break;
    case 2:
    case 3:
        pline_The("%s moves as though of its own will!", hliquid("lava"));
        if ((svm.mvitals[PM_FIRE_ELEMENTAL].mvflags & G_GONE)
            || !makemon(&mons[PM_FIRE_ELEMENTAL], u.ux, u.uy, MM_NOMSG))
            pline("But it settles down.");
        break;
    default:
        You("take a sip of molten lava.");
        u.uhunger += rnd(5);
    }
}

/* convert a sink into a fountain */
void
breaksink(coordxy x, coordxy y)
{
    if (cansee(x, y) || u_at(x, y))
        pline_The("pipes break!  Water spurts out!");
    /* updates level.flags.nsinks and level.flags.nfountains */
    set_levltyp(x, y, FOUNTAIN);
    levl[x][y].looted = 0;
    levl[x][y].blessedftn = 0;
    SET_FOUNTAIN_LOOTED(x, y);
    newsym(x, y);
}

/* quaff from a sink while standing on its location */
void
drinksink(void)
{
    struct obj *otmp;
    struct monst *mtmp;

    if (Levitation) {
        floating_above("sink");
        return;
    }
    switch (rn2(20)) {
    case 0:
        You("take a sip of very cold %s.", hliquid("water"));
        break;
    case 1:
        You("take a sip of very warm %s.", hliquid("water"));
        break;
    case 2:
        You("take a sip of scalding hot %s.", hliquid("water"));
        if (Fire_resistance) {
            pline("It seems quite tasty.");
            monstseesu(M_SEEN_FIRE);
        } else {
            losehp(rnd(6), "sipping boiling water", KILLED_BY);
            monstunseesu(M_SEEN_FIRE);
        }
        /* boiling water burns considered fire damage */
        break;
    case 3:
        if (svm.mvitals[PM_SEWER_RAT].mvflags & G_GONE)
            pline_The("sink seems quite dirty.");
        else {
            mtmp = makemon(&mons[PM_SEWER_RAT], u.ux, u.uy, MM_NOMSG);
            if (mtmp)
                pline("Eek!  There's %s in the sink!",
                      (Blind || !canspotmon(mtmp)) ? "something squirmy"
                                                   : a_monnam(mtmp));
        }
        break;
    case 4:
        for (;;) {
            otmp = mkobj(POTION_CLASS, FALSE);
            if (otmp->otyp != POT_WATER)
                break;
            /* reject water and try again */
            obfree(otmp, (struct obj *) 0);
        }
        otmp->cursed = otmp->blessed = 0;
        pline("Some %s liquid flows from the faucet.",
              Blind ? "odd" : hcolor(OBJ_DESCR(objects[otmp->otyp])));
        if(!(Blind || Hallucination))
            observe_object(otmp);
        otmp->quan++;       /* Avoid panic upon useup() */
        otmp->fromsink = 1; /* kludge for docall() */
        (void) dopotion(otmp);
        obfree(otmp, (struct obj *) 0);
        break;
    case 5:
        if (!(levl[u.ux][u.uy].looted & S_LRING)) {
            You("find a ring in the sink!");
            (void) mkobj_at(RING_CLASS, u.ux, u.uy, TRUE);
            levl[u.ux][u.uy].looted |= S_LRING;
            exercise(A_WIS, TRUE);
            newsym(u.ux, u.uy);
        } else
            pline("Some dirty %s backs up in the drain.", hliquid("water"));
        break;
    case 6:
        breaksink(u.ux, u.uy);
        break;
    case 7:
        pline_The("%s moves as though of its own will!", hliquid("water"));
        if ((svm.mvitals[PM_WATER_ELEMENTAL].mvflags & G_GONE)
            || !makemon(&mons[PM_WATER_ELEMENTAL], u.ux, u.uy, MM_NOMSG))
            pline("But it quiets down.");
        break;
    case 8:
        pline("Yuk, this %s tastes awful.", hliquid("water"));
        more_experienced(1, 0);
        newexplevel();
        break;
    case 9:
        pline("Gaggg... this tastes like sewage!  You vomit.");
        morehungry(rn1(30 - ACURR(A_CON), 11));
        vomit();
        break;
    case 10:
        pline("This %s contains toxic wastes!", hliquid("water"));
        if (!Unchanging) {
            You("undergo a freakish metamorphosis!");
            polyself(POLY_NOFLAGS);
        }
        break;
    /* more odd messages --JJB */
    case 11:
        Soundeffect(se_clanking_pipe, 50);
        You_hear("clanking from the pipes...");
        break;
    case 12:
        Soundeffect(se_sewer_song, 100);
        You_hear("snatches of song from among the sewers...");
        break;
    case 13:
        pline("Ew, what a stench!");
        create_gas_cloud(u.ux, u.uy, 1, 4);
        break;
    case 19:
        if (Hallucination) {
            pline("From the murky drain, a hand reaches up... --oops--");
            break;
        }
        FALLTHROUGH;
        /*FALLTHRU*/
    default:
        You("take a sip of %s %s.",
            rn2(3) ? (rn2(2) ? "cold" : "warm") : "hot",
            hliquid("water"));
    }
}

/* for #dip(potion.c) when standing on a sink */
void
dipsink(struct obj *obj)
{
    boolean try_call = FALSE,
            not_looted_yet = (levl[u.ux][u.uy].looted & S_LRING) == 0,
            is_hands = (obj == &hands_obj || (uarmg && obj == uarmg));

    if (!rn2(not_looted_yet ? 25 : 15)) {
        /* can't rely on using sink for unlimited scroll blanking; however,
           since sink will be converted into a fountain, hero can dip again */
        breaksink(u.ux, u.uy); /* "The pipes break!  Water spurts out!" */
        if (Glib && is_hands)
            Your("%s are still slippery.", fingers_or_gloves(TRUE));
        return;
    } else if (is_hands) {
        (void) wash_hands();
        return;
    } else if (obj->oclass != POTION_CLASS) {
        You("hold %s under the tap.", the(xname(obj)));
        if (water_damage(obj, (const char *) 0, TRUE) == ER_NOTHING)
            pline1(nothing_seems_to_happen);
        return;
    }

    /* at this point the object must be a potion */
    You("pour %s%s down the drain.", (obj->quan > 1L ? "one of " : ""),
        the(xname(obj)));
    switch (obj->otyp) {
    case POT_POLYMORPH:
        polymorph_sink();
        try_call = TRUE;
        break;
    case POT_OIL:
        if (!Blind) {
            pline("It leaves an oily film on the basin.");
            try_call = TRUE;
        } else {
            pline1(nothing_seems_to_happen);
        }
        break;
    case POT_ACID:
        /* acts like a drain cleaner product */
        try_call = TRUE;
        if (!Blind) {
            pline_The("drain seems less clogged.");
        } else if (!Deaf) {
            You_hear("a sucking sound.");
        } else {
            pline1(nothing_seems_to_happen);
            try_call = FALSE;
        }
        break;
    case POT_LEVITATION:
        sink_backs_up(u.ux, u.uy);
        try_call = TRUE;
        break;
    case POT_OBJECT_DETECTION:
        if (!(levl[u.ux][u.uy].looted & S_LRING)) {
            You("sense a ring lost down the drain.");
            try_call = TRUE;
            break;
        }
        FALLTHROUGH;
        /* FALLTHRU */
    case POT_GAIN_LEVEL:
    case POT_GAIN_ENERGY:
    case POT_MONSTER_DETECTION:
    case POT_FRUIT_JUICE:
    case POT_WATER:
        /* potions with no potionbreathe() effects, plus water.  if effects
           are added to potionbreathe these should go to that instead (except
           for water). */
        pline1(nothing_seems_to_happen);
        break;
    default:
        /* hero can feel the vapor on her skin, so no need to check Blind or
           breathless for this message */
        pline("A wisp of vapor rises up...");
        /* NB: potionbreathe calls trycall or makeknown as appropriate */
        if (!breathless(gy.youmonst.data) || haseyes(gy.youmonst.data))
            potionbreathe(obj);
        break;
    }
    if (try_call && obj->dknown)
        trycall(obj);
    useup(obj);
}

/* find a ring in a sink */
void
sink_backs_up(coordxy x, coordxy y)
{
    char buf[BUFSZ];

    if (!Blind)
        Strcpy(buf, "Muddy waste pops up from the drain");
    else if (!Deaf)
        Strcpy(buf, "You hear a sloshing sound"); /* Deaf-aware */
    else
        Sprintf(buf, "Something splashes you in the %s", body_part(FACE));
    pline("%s%s.", !Deaf ? "Flupp!  " : "", buf);

    if (!(levl[x][y].looted & S_LRING)) { /* once per sink */
        if (!Blind)
            You_see("a ring shining in its midst.");
        (void) mkobj_at(RING_CLASS, x, y, TRUE);
        newsym(x, y);
        exercise(A_DEX, TRUE);
        exercise(A_WIS, TRUE); /* a discovery! */
        levl[x][y].looted |= S_LRING;
    }
}

/*fountain.c*/
