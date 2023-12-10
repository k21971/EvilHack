/* NetHack 3.6	fountain.c	$NHDT-Date: 1544442711 2018/12/10 11:51:51 $  $NHDT-Branch: NetHack-3.6.2-beta01 $:$NHDT-Revision: 1.60 $ */
/*      Copyright Scott R. Turner, srt@ucla, 10/27/86 */
/* NetHack may be freely redistributed.  See license for details. */

/* Code for drinking from fountains. */

#include "hack.h"

STATIC_DCL void NDECL(dowatersnakes);
STATIC_DCL void NDECL(dowaterdemon);
STATIC_DCL void NDECL(dowaternymph);
STATIC_DCL void NDECL(dolavademon);
STATIC_PTR void FDECL(gush, (int, int, genericptr_t));
STATIC_DCL void NDECL(dofindgem);

/* used when trying to dip in or drink from fountain or sink or pool while
   levitating above it, or when trying to move downwards in that state */
void
floating_above(what)
const char *what;
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

/* Fountain of snakes! */
STATIC_OVL void
dowatersnakes()
{
    register int num = rn1(5, 2);
    struct monst *mtmp;

    if (!(mvitals[PM_WATER_MOCCASIN].mvflags & G_GONE)) {
        if (!Blind)
            pline("An endless stream of %s pours forth!",
                  Hallucination ? makeplural(rndmonnam(NULL)) : "snakes");
        else
            You_hear("%s hissing!", something);
        while (num-- > 0)
            if ((mtmp = makemon(&mons[PM_WATER_MOCCASIN], u.ux, u.uy,
                                NO_MM_FLAGS)) != 0
                && t_at(mtmp->mx, mtmp->my))
                (void) mintrap(mtmp);
    } else
        pline_The("fountain bubbles furiously for a moment, then calms.");
}

/* Water demon */
STATIC_OVL void
dowaterdemon()
{
    struct monst *mtmp;

    if (!(mvitals[PM_WATER_DEMON].mvflags & G_GONE)) {
        if ((mtmp = makemon(&mons[PM_WATER_DEMON], u.ux, u.uy,
                            NO_MM_FLAGS)) != 0) {
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
                (void) mintrap(mtmp);
        }
    } else
        pline_The("fountain bubbles furiously for a moment, then calms.");
}

/* Water Nymph */
STATIC_OVL void
dowaternymph()
{
    register struct monst *mtmp;

    if (!(mvitals[PM_WATER_NYMPH].mvflags & G_GONE)
        && (mtmp = makemon(&mons[PM_WATER_NYMPH], u.ux, u.uy,
                           NO_MM_FLAGS)) != 0) {
        if (!Blind)
            You("attract %s!", a_monnam(mtmp));
        else
            You_hear("a seductive voice.");
        mtmp->msleeping = 0;
        if (t_at(mtmp->mx, mtmp->my))
            (void) mintrap(mtmp);
    } else if (!Blind)
        pline("A large bubble rises to the surface and pops.");
    else
        You_hear("a loud pop.");
}

/* Lava Demon */
STATIC_OVL void
dolavademon()
{
    struct monst *mtmp;

    if (!(mvitals[PM_LAVA_DEMON].mvflags & G_GONE)) {
        if ((mtmp = makemon(&mons[PM_LAVA_DEMON], u.ux, u.uy,
                            NO_MM_FLAGS)) != 0) {
            if (!Blind)
                You("summon %s!", a_monnam(mtmp));
            else
                You_feel("the temperature rise significantly.");

            /* Give those on low levels a (slightly) better chance of survival
             */
            if (rnd(100) > (80 + level_difficulty())) {
                pline("Freed from the depths of Gehennom, %s offers to aid you in your quest!",
                      mhe(mtmp));
                (void) tamedog(mtmp, (struct obj *) 0);
            } else if (t_at(mtmp->mx, mtmp->my))
                (void) mintrap(mtmp);
        }
    } else
        pline_The("forge violently spews lava for a moment, then settles.");
}

/* Gushing forth along LOS from (u.ux, u.uy) */
void
dogushforth(drinking)
int drinking;
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

STATIC_PTR void
gush(x, y, poolcnt)
int x, y;
genericptr_t poolcnt;
{
    register struct monst *mtmp;
    register struct trap *ttmp;

    if (((x + y) % 2) || (x == u.ux && y == u.uy)
        || (rn2(1 + distmin(u.ux, u.uy, x, y))) || (levl[x][y].typ != ROOM)
        || (sobj_at(BOULDER, x, y)) || nexttodoor(x, y))
        return;

    if ((ttmp = t_at(x, y)) != 0 && !delfloortrap(ttmp))
        return;

    if (!((*(int *) poolcnt)++))
        pline("Water gushes forth from the overflowing fountain!");

    /* Put a puddle at x, y */
    levl[x][y].typ = PUDDLE, levl[x][y].flags = 0;
    /* No kelp! */
    del_engr_at(x, y);
    water_damage_chain(level.objects[x][y], TRUE, 0, TRUE, x, y);

    if ((mtmp = m_at(x, y)) != 0)
        (void) minliquid(mtmp);
    else
        newsym(x, y);
}

/* Find a gem in the sparkling waters. */
STATIC_OVL void
dofindgem()
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

void
dryup(x, y, isyou)
xchar x, y;
boolean isyou;
{
    if (IS_FOUNTAIN(levl[x][y].typ)
        && (!rn2(3) || FOUNTAIN_IS_WARNED(x, y))) {
        if (isyou && in_town(x, y) && !FOUNTAIN_IS_WARNED(x, y)) {
            struct monst *mtmp;

            SET_FOUNTAIN_WARNED(x, y);
            /* Warn about future fountain use. */
            for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
                if (DEADMONSTER(mtmp))
                    continue;
                if (is_watch(mtmp->data) && couldsee(mtmp->mx, mtmp->my)
                    && mtmp->mpeaceful && !MON_CASTBALL) {
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
                    break;
                }
            }
            /* You can see or hear this effect */
            if (!mtmp)
                pline_The("flow reduces to a trickle.");
            return;
        }
        if (isyou && wizard) {
            if (yn("Dry up fountain?") == 'n')
                return;
        }
        /* replace the fountain with ordinary floor */
        levl[x][y].typ = ROOM, levl[x][y].flags = 0;
        levl[x][y].blessedftn = 0;
        if (cansee(x, y))
            pline_The("fountain dries up!");
        /* The location is seen if the hero/monster is invisible
           or felt if the hero is blind. */
        newsym(x, y);
        level.flags.nfountains--;
        if (isyou && in_town(x, y) && !MON_CASTBALL)
            (void) angry_guards(FALSE);
    }
}

void
dipforge(obj)
register struct obj *obj;
{
    if (Levitation) {
        floating_above("forge");
        return;
    }

    burn_away_slime();

    /* Dipping something you're still wearing into a forge filled with
     * lava, probably not the smartest thing to do. This is gonna hurt.
     * Non-metallic objects are handled by lava_damage().
     */
    if (is_metallic(obj) && (obj->owornmask & (W_ARMOR | W_ACCESSORY))) {
        if (how_resistant(FIRE_RES) < 100) {
            You("dip your worn %s into the forge.  You burn yourself!",
                xname(obj));
            if (!rn2(3))
                You("may want to remove your %s first...",
                    xname(obj));
        }
        if (how_resistant(FIRE_RES) == 100) {
            You("can't reforge something you're wearing.");
        }
        losehp(resist_reduce(d(1, 8), FIRE_RES),
               "dipping a worn object into a forge", KILLED_BY);
        return;
    }

    /* If punished and wielding a hammer, there's a good chance
     * you can use a forge to free yourself */
    if (Punished && obj->otyp == HEAVY_IRON_BALL) {
        if ((uwep && !is_hammer(uwep)) || !uwep) { /* sometimes drop a hint */
            if (!rn2(4))
                pline("You'll need a hammer to be able to break the chain.");
            goto result;
        } else if (uwep && is_hammer(uwep)) {
            You("place the ball and chain inside the forge.");
            pline("Raising your %s, you strike the chain...",
                  xname(uwep));
            if (!rn2((P_SKILL(P_HAMMER) < P_SKILLED) ? 8 : 2)
                && Luck >= 0) { /* training up hammer skill pays off */
                pline("The chain breaks free!");
                unpunish();
            } else {
                pline("Clang!");
            }
        }
        return;
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
        if (is_metallic(obj) && Luck >= 0) {
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

        if (!obj->blessed && is_metallic(obj) && Luck > 5) {
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
           losehp(resist_reduce(d(3, 8), FIRE_RES), "dipping into a forge", KILLED_BY);
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
    lava_damage(obj, u.ux, u.uy);
    update_inventory();
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
    { DARK_ELVEN_SHORT_SWORD, DARK_ELVEN_CROSSBOW_BOLT, DARK_ELVEN_DAGGER, 2, 1 },
    { DARK_ELVEN_BROADSWORD, DARK_ELVEN_DAGGER, DARK_ELVEN_SHORT_SWORD, 1, 1 },
    { DARK_ELVEN_LONG_SWORD, DARK_ELVEN_SHORT_SWORD, DARK_ELVEN_SHORT_SWORD, 1, 1 },
    { DARK_ELVEN_MACE, DARK_ELVEN_DAGGER, DARK_ELVEN_DAGGER, 1, 1 },
    { DARK_ELVEN_HEAVY_MACE, DARK_ELVEN_MACE, DARK_ELVEN_MACE, 1, 1 },
    { DARK_ELVEN_CHAIN_MAIL, DARK_ELVEN_HEAVY_MACE, DARK_ELVEN_BRACERS, 1, 1 },
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
    /* forged artifacts that require at least one other forged
       artifact to be created */
    { ART_ITHILMAR, ART_VORPAL_BLADE, ART_GAUNTLETS_OF_PURITY },
    { ART_ARMOR_OF_RETRIBUTION, ART_SWORD_OF_ANNIHILATION, ART_ASHMAR },
    { 0, 0, 0 }
};


int
doforging(void)
{
    const struct forge_recipe *recipe;
    const struct forge_arti *arti;
    struct obj* obj1;
    struct obj* obj2;
    struct obj* output;
    char allowall[2];
    int objtype = 0, artitype = 0;

    allowall[0] = ALL_CLASSES;
    allowall[1] = '\0';

    /* first, we need a forge */
    if (!IS_FORGE(levl[u.ux][u.uy].typ)) {
        You("need a forge in order to forge objects.");
        return 0;
    }

    /* next, the proper tool to do the job */
    if ((uwep && !is_hammer(uwep)) || !uwep) {
        pline("You'll need a hammer to forge successfully.");
        return 0;
    }

    /* setup the base object */
    obj1 = getobj(allowall, "use as a base");
    if (!obj1) {
        You("need a base object to forge with.");
        return 0;
    } else if (!(is_metallic(obj1) || is_crystal(obj1)
                 || bypass_forging_rules(obj1))) {
        /* object should be gemstone or metallic */
        pline_The("base object must be made of a mineral, gemstone, or something metallic.");
        return 0;
    }

    /* setup the secondary object */
    obj2 = getobj(allowall, "combine with the base object");
    if (!obj2) {
        You("need more than one object.");
        return 0;
    } else if (!(is_metallic(obj2) || is_crystal(obj2)
                 || bypass_forging_rules(obj2))) {
        /* secondary object should also be gemstone or metallic */
        pline_The("secondary object must be made of a mineral, gemstone, or something metallic.");
        return 0;
    }

    /* handle illogical cases */
    if (obj1 == obj2) {
        You_cant("combine an object with itself!");
        return 0;
    /* not that the Amulet of Yendor or invocation items would
       ever be part of a forging recipe, but these should be
       protected in any case */
    } else if ((obj_resists(obj1, 0, 0)
                && obj1->oartifact != ART_SWORD_OF_ANNIHILATION)
               || (obj_resists(obj2, 0, 0)
                   && obj2->oartifact != ART_SWORD_OF_ANNIHILATION)) {
        You_cant("forge such a thing!");
        blowupforge(u.ux, u.uy);
        return 0;
    /* worn or wielded objects */
    } else if (is_worn(obj1) || is_worn(obj2)) {
        You("must first %s the objects you wish to forge.",
            ((obj1->owornmask & W_ARMOR)
             || (obj2->owornmask & W_ARMOR)) ? "remove" : "unwield");
        return 0;
    /* Artifacts can never be applied to a non-arficat base. */
    } else if ((obj2->oartifact && !obj1->oartifact)
               || (obj1->oartifact && !obj2->oartifact)) {
        pline("Artifacts cannot be forged with lesser objects.");
        return 0;
    /* dragon-scaled armor can never be fully metallic */
    } else if (Is_dragon_scaled_armor(obj1)
               || Is_dragon_scaled_armor(obj2)) {
        pline("Dragon-scaled armor cannot be forged.");
        return 0;
    /* due to the nature of Drow objects, they can only
       be forged in darkness */
    } else if (!spot_is_dark(u.ux, u.uy)
               && (is_drow_obj(obj1) || is_drow_obj(obj2))) {
        pline("Drowcraft must be performed under the cover of darkness.");
        return 0;
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
            return 1;
        } else if (artitype) {
            /* success */
            output = mk_particular_artifact(artitype);

            You("place %s, then %s inside the forge.",
                the(xname(obj1)), the(xname(obj2)));
            pline("Raising your %s, you begin to forge the artifacts together...",
                  xname(uwep));

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
            /* ensure the final product is not degraded or poisoned
               in any way */
            output->oeroded = output->oeroded2 = 0;
            output->opoisoned = 0;
            output->otainted = 0;

            /* delete recipe objects - use delobj_core() directly
               to handle the Sword of Annihilation potentially
               being one of the recipe objects */
            delobj_core(obj1, TRUE);
            delobj_core(obj2, TRUE);

            /* forged object is created */
            output = addinv(output);
            output->owt = weight(output);
            You("have successfully forged %s.", doname(output));
            livelog_printf(LL_ARTIFACT, "used a forge to create %s%s",
                           (output->oartifact == ART_GAUNTLETS_OF_PURITY
                            || output->oartifact == ART_HAMMER_OF_THE_GODS
                            || output->oartifact == ART_ARMOR_OF_RETRIBUTION)
                           ? "the " : "",
                           artiname(output->oartifact));
            update_inventory();

            /* special events that may happen when a
               particular artifact is forged go here */
            if (output->oartifact == ART_GLAMDRING
                && Role_if(PM_WIZARD)) {
                /* how else did Gandalf learn to use
                   a sword? */
                unrestrict_weapon_skill(P_LONG_SWORD);
                P_MAX_SKILL(P_LONG_SWORD) = P_BASIC;
            }

            /* forging an artifact is too much stress for the forge */
            coolforge(u.ux, u.uy);
        }
    } else {
        for (recipe = fusions; recipe->result_typ; recipe++) {
            if ((obj1->otyp == recipe->typ1
                 && obj2->otyp == recipe->typ2
                 && obj1->quan >= recipe->quan_typ1
                 && obj2->quan >= recipe->quan_typ2)
                || (obj2->otyp == recipe->typ1
                    && obj1->otyp == recipe->typ2
                    && obj2->quan >= recipe->quan_typ1
                    && obj1->quan >= recipe->quan_typ2)) {
                objtype = recipe->result_typ;
                break;
            }
        }
        if (!objtype) {
            /* if the objects used do not match the recipe array,
               the forging process fails */
            You("fail to combine these two objects.");
            return 1;
        } else if (objtype) {
            /* success */
            output = mksobj(objtype, TRUE, FALSE);

            /* try to take on the material from one of the source objects */
            if (valid_obj_material(output, obj2->material)) {
                set_material(output, obj2->material);
            } else if (valid_obj_material(output, obj1->material)) {
                set_material(output, obj1->material);
            } else {
                /* neither material is valid for the output object, so can't
                 * forge them successfully */
                pline_The("%s and %s resist melding in the forge.",
                          materialnm[obj1->material],
                          materialnm[obj2->material]);
                You("fail to combine the two objects.");
                delobj(output);
                return 1;
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
                    output->oprops_known |= output->oprops;
            } else if (obj1->oprops) {
                if (!is_barding(output))
                    output->oprops = obj1->oprops;
                if (obj1->oprops_known)
                    output->oprops_known |= output->oprops;
            }

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
            /* ensure the final product is not degraded or poisoned
               in any way */
            output->oeroded = output->oeroded2 = 0;
            output->opoisoned = 0;
            output->otainted = 0;

            /* toss out old objects, add new one */
            if (obj1->otyp == recipe->typ1)
                obj1->quan -= recipe->quan_typ1;
            if (obj2->otyp == recipe->typ1)
                obj2->quan -= recipe->quan_typ1;
            if (obj1->otyp == recipe->typ2)
                obj1->quan -= recipe->quan_typ2;
            if (obj2->otyp == recipe->typ2)
                obj2->quan -= recipe->quan_typ2;

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

            /* forged object is created */
            output = addinv(output);
            /* prevent large stacks of ammo-type weapons from being
               formed */
            if (output->quan > 3L) {
                output->quan = 3L;
                if (!rn2(4)) /* small chance of an extra */
                    output->quan += 1L;
            }
            output->owt = weight(output);
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

    return 1;
}

void
drinkfountain()
{
    /* What happens when you drink from a fountain? */
    register boolean mgkftn = (levl[u.ux][u.uy].blessedftn == 1);
    register int fate = rnd(30);

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
                context.botl = 1;
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
            if (how_resistant(POISON_RES) == 100) {
                pline("Perhaps it is runoff from the nearby %s farm.",
                      fruitname(FALSE));
                losehp(rnd(4), "unrefrigerated sip of juice", KILLED_BY_AN);
                break;
            }
            losestr(resist_reduce(rn1(4, 3), POISON_RES));
            losehp(resist_reduce(rnd(10), POISON_RES), "contaminated water", KILLED_BY);
            exercise(A_CON, FALSE);
            break;
        case 22: /* Fountain of snakes! */
            dowatersnakes();
            break;
        case 23: /* Water demon */
            dowaterdemon();
            break;
        case 24: { /* Maybe curse some items */
            register struct obj *obj;
            int buc_changed = 0;

            pline("This water's no good!");
            morehungry(rn1(20, 11));
            exercise(A_CON, FALSE);
            /* this is more severe than rndcurse() */
            for (obj = invent; obj; obj = obj->nobj)
                if (obj->oclass != COIN_CLASS && !obj->cursed && !rn2(5)) {
                    curse(obj);
                    ++buc_changed;
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
            incr_itimeout(&HSee_invisible, (long) rn1(100, 50));
            newsym(u.ux, u.uy);
            exercise(A_WIS, TRUE);
            break;
        case 26: /* See Monsters */
            if (monster_detect((struct obj *) 0, 0))
                pline_The("%s tastes bland.", hliquid("water"));
            exercise(A_WIS, TRUE);
            break;
        case 27: /* Find a gem in the sparkling waters. */
            if (!FOUNTAIN_IS_LOOTED(u.ux, u.uy)) {
                dofindgem();
                break;
            }
            /*FALLTHRU*/
        case 28: /* Water Nymph */
            dowaternymph();
            break;
        case 29: /* Scare */
        {
            register struct monst *mtmp;

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

void
dipfountain(obj)
register struct obj *obj;
{
    if (Levitation) {
        floating_above("fountain");
        return;
    }

    /* Don't grant Excalibur when there's more than one object.  */
    /* (quantity could be > 1 if merged daggers got polymorphed) */
    if (obj->otyp == LONG_SWORD && obj->quan == 1L && u.ulevel >= 5 && !rn2(6)
        && !(uarmh && uarmh->otyp == HELM_OF_OPPOSITE_ALIGNMENT)
        && !obj->oartifact && !exist_artifact(LONG_SWORD, artiname(ART_EXCALIBUR))) {
        if (u.ualign.type != A_LAWFUL || !Role_if(PM_KNIGHT)) {
            /* Ha!  Trying to cheat her. */
            pline("A freezing mist rises from the %s and envelopes the sword.",
                  hliquid("water"));
            if (!rn2(3)) {
                pline("From the mist, a hand reaches forth and wrests your %s from your grasp!",
                      xname(obj));
                livelog_printf(LL_ARTIFACT,
                               "was denied Excalibur! The Lady of the Lake has removed %s %s from existence",
                               uhis(), xname(obj));
                if (carried(obj))
                    useup(obj);
                else
                   delobj(obj);
                pline_The("fountain disappears!");
                exercise(A_WIS, FALSE);
                change_luck(-1);
                goto dip_end;
            }
            pline_The("fountain disappears!");
            curse(obj);
            if (obj->spe > -6 && !rn2(3))
                obj->spe--;
            maybe_erodeproof(obj, 0);
            exercise(A_WIS, FALSE);
            livelog_printf(LL_ARTIFACT, "was denied Excalibur! The Lady of the Lake has deemed %s unworthy", uhim());
        } else {
            /* The lady of the lake acts! - Eric Backus */
            /* Be *REAL* nice */
            pline(
              "From the murky depths, a hand reaches up to bless the sword.");
            pline("As the hand retreats, the fountain disappears!");
            obj->oprops = obj->oprops_known = 0L;
            obj = oname(obj, artiname(ART_EXCALIBUR));
            discover_artifact(ART_EXCALIBUR);
            bless(obj);
            if (obj->spe < 0)
                obj->spe = 0;
            obj->oeroded = obj->oeroded2 = 0;
            maybe_erodeproof(obj, 1);
            exercise(A_WIS, TRUE);
            u.uconduct.artitouch++;
            livelog_printf(LL_ARTIFACT, "%s", excalmsgs[rn2(SIZE(excalmsgs))]);
        }
dip_end:
        update_inventory();
        levl[u.ux][u.uy].typ = ROOM, levl[u.ux][u.uy].flags = 0;
        newsym(u.ux, u.uy);
        level.flags.nfountains--;
        if (in_town(u.ux, u.uy))
            (void) angry_guards(FALSE);
        return;
    } else {
        int er = water_damage(obj, NULL, TRUE, u.ux, u.uy);

        if (er == ER_DESTROYED || (er != ER_NOTHING && !rn2(2))) {
            return; /* no further effect */
        }
    }

    switch (rnd(30)) {
    case 16: /* Curse the item */
        if (obj->oclass != COIN_CLASS && !obj->cursed) {
            curse(obj);
        }
        break;
    case 17:
    case 18:
    case 19:
    case 20: /* Uncurse the item */
        if (obj->cursed) {
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
            long money = money_cnt(invent);
            struct obj *otmp;
            if (money > 10) {
                /* Amount to lose.  Might get rounded up as fountains don't
                 * pay change... */
                money = somegold(money) / 10;
                for (otmp = invent; otmp && money > 0; otmp = otmp->nobj)
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
                You("lost some of your money in the fountain!");
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
    }
    update_inventory();
    dryup(u.ux, u.uy, TRUE);
}

void
breakforge(x, y)
int x, y;
{
    if (cansee(x, y) || (x == u.ux && y == u.uy))
        pline_The("forge splits in two as molten lava rushes forth!");
    levl[x][y].doormask = 0;
    levl[x][y].typ = LAVAPOOL;
    newsym(x, y);
    level.flags.nforges--;
}

void
blowupforge(x, y)
int x, y;
{
    if (cansee(x, y) || (x == u.ux && y == u.uy))
        pline_The("forge rumbles, then explodes!  Molten lava splashes everywhere!");
    levl[x][y].typ = ROOM, levl[x][y].flags = 0;
    levl[x][y].doormask = 0;
    newsym(x, y);
    level.flags.nforges--;
    explode(u.ux, u.uy, ZT_FIRE, resist_reduce(rnd(30), FIRE_RES),
            FORGE_EXPLODE, EXPL_FIERY);
}

void
coolforge(x, y)
int x, y;
{
    if (cansee(x, y) || (x == u.ux && y == u.uy))
        pline_The("lava in the forge cools and solidifies.");
    levl[x][y].typ = ROOM, levl[x][y].flags = 0;
    levl[x][y].doormask = 0;
    newsym(x, y);
    level.flags.nforges--;
}

void
drinkforge()
{
    if (Levitation) {
        floating_above("forge");
        return;
    }

    if (!likes_fire(youmonst.data)) {
        pline("Molten lava incinerates its way down your gullet...");
        losehp(Upolyd ? u.mh : u.uhp, "trying to drink molten lava", KILLED_BY);
        return;
    }
    burn_away_slime();
    switch(rn2(20)) {
    case 0:
        pline("You drink some molten lava.  Mmmmm mmm!");
        u.uhunger += rnd(50);
        break;
    case 1:
        breakforge(u.ux, u.uy);
        break;
    case 2:
    case 3:
        pline_The("%s moves as though of its own will!", hliquid("lava"));
        if ((mvitals[PM_FIRE_ELEMENTAL].mvflags & G_GONE)
            || !makemon(&mons[PM_FIRE_ELEMENTAL], u.ux, u.uy, NO_MM_FLAGS))
            pline("But it settles down.");
        break;
    default:
        pline("You take a sip of molten lava.");
        u.uhunger += rnd(5);
    }
}

void
breaksink(x, y)
int x, y;
{
    if (cansee(x, y) || (x == u.ux && y == u.uy))
        pline_The("pipes break!  Water spurts out!");
    level.flags.nsinks--;
    levl[x][y].typ = FOUNTAIN, levl[x][y].looted = 0;
    levl[x][y].blessedftn = 0;
    SET_FOUNTAIN_LOOTED(x, y);
    level.flags.nfountains++;
    newsym(x, y);
}

void
drinksink()
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
        if (how_resistant(FIRE_RES) == 100) {
            pline("It seems quite tasty.");
            monstseesu(M_SEEN_FIRE);
        }
        else
            losehp(resist_reduce(rnd(6), FIRE_RES), "sipping boiling water", KILLED_BY);
        /* boiling water burns considered fire damage */
        break;
    case 3:
        if (mvitals[PM_SEWER_RAT].mvflags & G_GONE)
            pline_The("sink seems quite dirty.");
        else {
            mtmp = makemon(&mons[PM_SEWER_RAT], u.ux, u.uy, NO_MM_FLAGS);
            if (mtmp)
                pline("Eek!  There's %s in the sink!",
                      (Blind || !canspotmon(mtmp)) ? "something squirmy"
                                                   : a_monnam(mtmp));
        }
        break;
    case 4:
        do {
            otmp = mkobj(POTION_CLASS, FALSE);
            if (otmp->otyp == POT_WATER) {
                obfree(otmp, (struct obj *) 0);
                otmp = (struct obj *) 0;
            }
        } while (!otmp);
        otmp->cursed = otmp->blessed = 0;
        pline("Some %s liquid flows from the faucet.",
              Blind ? "odd" : hcolor(OBJ_DESCR(objects[otmp->otyp])));
        otmp->dknown = !(Blind || Hallucination);
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
        if ((mvitals[PM_WATER_ELEMENTAL].mvflags & G_GONE)
            || !makemon(&mons[PM_WATER_ELEMENTAL], u.ux, u.uy, NO_MM_FLAGS))
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
            polyself(0);
        }
        break;
    /* more odd messages --JJB */
    case 11:
        You_hear("clanking from the pipes...");
        break;
    case 12:
        You_hear("snatches of song from among the sewers...");
        break;
    case 19:
        if (Hallucination) {
            pline("From the murky drain, a hand reaches up... --oops--");
            break;
        }
        /*FALLTHRU*/
    default:
        You("take a sip of %s %s.",
            rn2(3) ? (rn2(2) ? "cold" : "warm") : "hot",
            hliquid("water"));
    }
}

/*fountain.c*/
