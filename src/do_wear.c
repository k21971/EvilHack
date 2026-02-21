/* NetHack 3.6	do_wear.c	$NHDT-Date: 1575214670 2019/12/01 15:37:50 $  $NHDT-Branch: NetHack-3.6 $:$NHDT-Revision: 1.116 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2012. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

static NEARDATA const char see_yourself[] = "see yourself";
static NEARDATA const char unknown_type[] = "Unknown type of %s (%d)";
static NEARDATA const char c_armor[] = "armor", c_suit[] = "suit",
                           c_shirt[] = "shirt", c_cloak[] = "cloak",
                           c_gloves[] = "gloves", c_boots[] = "boots",
                           c_helmet[] = "helmet", c_shield[] = "shield",
                           c_bracers[] = "bracers", c_weapon[] = "weapon",
                           c_sword[] = "sword", c_axe[] = "axe", c_that_[] = "that";

static NEARDATA const long takeoff_order[] = {
    WORN_BLINDF, W_WEP,      WORN_SHIELD, WORN_GLOVES, LEFT_RING,
    RIGHT_RING,  WORN_CLOAK, WORN_HELMET, WORN_AMUL,   WORN_ARMOR,
    WORN_SHIRT,  WORN_BOOTS, W_SWAPWEP,   W_QUIVER,    0L
};

STATIC_DCL void FDECL(on_msg, (struct obj *));
STATIC_DCL void FDECL(toggle_stealth, (struct obj *, long, BOOLEAN_P));
STATIC_PTR int NDECL(Armor_on);
STATIC_PTR int NDECL(Cloak_on);
STATIC_PTR int NDECL(Helmet_on);
STATIC_PTR int NDECL(Gloves_on);
STATIC_DCL void FDECL(wielding_corpse, (struct obj *, BOOLEAN_P));
STATIC_PTR int NDECL(Shield_on);
STATIC_PTR int NDECL(Shirt_on);
STATIC_DCL void NDECL(Amulet_on);
STATIC_DCL void FDECL(learnring, (struct obj *, BOOLEAN_P));
STATIC_DCL void FDECL(Ring_off_or_gone, (struct obj *, BOOLEAN_P));
STATIC_PTR int FDECL(select_off, (struct obj *));
STATIC_DCL struct obj *NDECL(do_takeoff);
STATIC_PTR int NDECL(take_off);
STATIC_DCL int FDECL(menu_remarm, (int));
STATIC_DCL void FDECL(count_worn_stuff, (struct obj **, BOOLEAN_P));
STATIC_PTR int FDECL(armor_or_accessory_off, (struct obj *));
STATIC_PTR int FDECL(accessory_or_armor_on, (struct obj *));
STATIC_DCL void FDECL(already_wearing, (const char *));
STATIC_DCL void FDECL(already_wearing2, (const char *, const char *));
STATIC_DCL void FDECL(toggle_armor_light, (struct obj *, boolean));

/* plural "fingers" or optionally "gloves" */
const char *
fingers_or_gloves(check_gloves)
boolean check_gloves;
{
    if (check_gloves && uarmg
        && uarmg->oartifact == ART_HAND_OF_VECNA)
        return "mummified hand";

    return ((check_gloves && uarmg)
            ? gloves_simple_name(uarmg) /* "gloves" or "gauntlets" */
            : makeplural(body_part(FINGER))); /* "fingers" */
}

void
off_msg(otmp)
struct obj *otmp;
{
    if (flags.verbose)
        You("were wearing %s.", doname(otmp));
}

/* for items that involve no delay */
STATIC_OVL void
on_msg(otmp)
struct obj *otmp;
{
    if (flags.verbose) {
        char how[BUFSZ];
        /* call xname() before obj_is_pname(); formatting obj's name
           might set obj->dknown and that affects the pname test */
        const char *otmp_name = xname(otmp);

        how[0] = '\0';
        if (otmp->otyp == TOWEL)
            Sprintf(how, " around your %s", body_part(HEAD));
        You("are now wearing %s%s.",
            obj_is_pname(otmp) ? the(otmp_name) : an(otmp_name), how);
    }
}

/* starting equipment gets auto-worn at beginning of new game,
   and we don't want stealth or displacement feedback then */
static boolean initial_don = FALSE; /* manipulated in set_wear() */

/* putting on or taking off an item which confers stealth;
   give feedback and discover it iff stealth state is changing */
STATIC_OVL
void
toggle_stealth(obj, oldprop, on)
struct obj *obj;
long oldprop; /* prop[].extrinsic, with obj->owornmask stripped by caller */
boolean on;
{
    if (on ? initial_don : context.takeoff.cancelled_don)
        return;

    if (!oldprop /* extrinsic stealth from something else */
        && !HStealth /* intrinsic stealth */
        && !BStealth) { /* stealth blocked by something */
        if (obj->otyp == RIN_STEALTH)
            learnring(obj, TRUE);
        else
            makeknown(obj->otyp);

        if (on) {
            if (!is_boots(obj))
                You("move very quietly.");
            else if (Levitation || Flying)
                You("float imperceptibly.");
            else
                You("walk very quietly.");
        } else {
            if (maybe_polyd(is_giant(youmonst.data), Race_if(PM_GIANT)))
                You("are just as noisy as before.");
            else
               You("sure are noisy.");
        }
    }
}

/* putting on or taking off an item which confers displacement, or gaining
   or losing timed displacement after eating a displacer beast corpse or tin;
   give feedback and discover it iff displacement state is changing *and*
   hero is able to see self (or sense monsters); for timed, 'obj' is Null
   and this is only called for the message */
void
toggle_displacement(obj, oldprop, on)
struct obj *obj;
long oldprop; /* prop[].extrinsic, with obj->owornmask stripped by caller */
boolean on;
{
    if (on ? initial_don : context.takeoff.cancelled_don)
        return;

    if (!oldprop /* extrinsic displacement from something else */
        && !(u.uprops[DISPLACED].intrinsic) /* timed, from eating */
        && !(u.uprops[DISPLACED].blocked) /* (theoretical) */
        /* we don't use canseeself() here because it augments vision
           with touch, which isn't appropriate for deciding whether
           we'll notice that monsters have trouble spotting the hero */
        && ((!Blind         /* see anything */
             && !u.uswallow /* see surroundings */
             && !Invisible) /* see self */
            /* actively sensing nearby monsters via telepathy or extended
               monster detection overrides vision considerations because
               hero also senses self in this situation */
            || (Unblind_telepat
                || (Blind_telepat && Blind)
                || Detect_monsters))) {
        if (obj)
            makeknown(obj->otyp);

        You_feel("that monsters%s have difficulty pinpointing your location.",
                 on ? "" : " no longer");
    }
}

/*
 * The Type_on() functions should be called *after* setworn().
 * The Type_off() functions call setworn() themselves.
 * [Blindf_on() is an exception and calls setworn() itself.]
 */

void
oprops_on(otmp, mask)
struct obj *otmp;
long mask;
{
    long props = otmp->oprops;

    if (props & ITEM_FIRE)
        EFire_resistance |= mask;
    if (props & ITEM_FROST)
        ECold_resistance |= mask;
    if (props & ITEM_DRLI)
        EDrain_resistance |= mask;
    if (props & ITEM_SHOCK)
        EShock_resistance |= mask;
    if (props & ITEM_VENOM)
        EPoison_resistance |= mask;
    if (props & ITEM_OILSKIN) {
        pline("%s very tightly.", Tobjnam(otmp, "fit"));
        otmp->oprops_known |= ITEM_OILSKIN;
    }
    if (props & ITEM_ESP) {
        ETelepat |= mask;
        see_monsters();
    }
    if (props & ITEM_SEARCHING)
        ESearching |= mask;
    if (props & ITEM_WARNING) {
        EWarning |= mask;
        see_monsters();
    }
    if (props & ITEM_FUMBLING) {
        if (!EFumbling && !(HFumbling & ~TIMEOUT))
            incr_itimeout(&HFumbling, rnd(20));
        EFumbling |= mask;
    }
    if (props & ITEM_HUNGER)
        EHunger |= mask;
    if (props & ITEM_EXCEL) {
        int which = A_CHA, old_attrib = ACURR(which);
        /* HoB and GoD adjust CHA in adj_abon() */
        if (otmp->otyp != HELM_OF_BRILLIANCE
            && otmp->otyp != GAUNTLETS_OF_DEXTERITY)
            /* borrowing this from Ring_on() as I may want
               to add other attributes in the future */
            ABON(which) += otmp->spe;
        if (old_attrib != ACURR(which))
            otmp->oprops_known |= ITEM_EXCEL;
        set_moreluck();
        context.botl = 1;
    }
}

void
oprops_off(otmp, mask)
struct obj *otmp;
long mask;
{
    long props = otmp->oprops;

    if (props & ITEM_FIRE)
        EFire_resistance &= ~mask;
    if (props & ITEM_FROST)
        ECold_resistance &= ~mask;
    if (props & ITEM_DRLI)
        EDrain_resistance &= ~mask;
    if (props & ITEM_SHOCK)
        EShock_resistance &= ~mask;
    if (props & ITEM_VENOM)
        EPoison_resistance &= ~mask;
    if (props & ITEM_OILSKIN)
        otmp->oprops_known |= ITEM_OILSKIN;
    if (props & ITEM_ESP) {
        ETelepat &= ~mask;
        see_monsters();
    }
    if (props & ITEM_SEARCHING)
        ESearching &= ~mask;
    if (props & ITEM_WARNING) {
        EWarning &= ~mask;
        see_monsters();
    }
    if (props & ITEM_FUMBLING) {
        EFumbling &= ~mask;
        if (!EFumbling && !(HFumbling & ~TIMEOUT))
            HFumbling = EFumbling = 0L;
    }
    if (props & ITEM_HUNGER)
        EHunger &= ~mask;
    if (props & ITEM_EXCEL) {
        int which = A_CHA, old_attrib = ACURR(which);
        /* HoB and GoD adjust CHA in adj_abon() */
        if (otmp->otyp != HELM_OF_BRILLIANCE
            && otmp->otyp != GAUNTLETS_OF_DEXTERITY)
            /* borrowing this from Ring_off() as I may want
               to add other attributes in the future */
            ABON(which) -= otmp->spe;
        if (old_attrib != ACURR(which))
            otmp->oprops_known |= ITEM_EXCEL;
        /* Temporarily clear ITEM_EXCEL so confers_luck() won't count
           this item during set_moreluck() - owornmask is still set at
           this point. Safe because set_moreluck() has no side effects
           that read oprops */
        otmp->oprops &= ~ITEM_EXCEL;
        set_moreluck();
        otmp->oprops |= ITEM_EXCEL;
        context.botl = 1;
    }
}

int
Boots_on(VOID_ARGS)
{
    long oldprop =
        u.uprops[objects[uarmf->otyp].oc_oprop].extrinsic & ~WORN_BOOTS;

    switch (uarmf->otyp) {
    case LOW_BOOTS:
    case DWARVISH_BOOTS:
    case HIGH_BOOTS:
    case JUMPING_BOOTS:
    case KICKING_BOOTS:
    case ORCISH_BOOTS:
    case MEAT_BOOTS:
        break;
    case WATER_WALKING_BOOTS:
        if (u.uinwater)
            spoteffects(TRUE);
        /* (we don't need a lava check here since boots can't be
           put on while feet are stuck) */
        break;
    case SPEED_BOOTS:
        /* Speed boots are still better than intrinsic speed, */
        /* though not better than potion speed */
        if (!oldprop && !(HFast & TIMEOUT)) {
            makeknown(uarmf->otyp);
            You_feel("yourself speed up%s.",
                     (oldprop || HFast) ? " a bit more" : "");
        }
        break;
    case ELVEN_BOOTS:
    case DARK_ELVEN_BOOTS:
        if (maybe_polyd(is_giant(youmonst.data), Race_if(PM_GIANT))) {
            pline("This %s will not silence someone %s.",
                  xname(uarmf), rn2(2) ? "as large as you" : "of your stature");
            EStealth &= ~W_ARMF;
        } else {
            toggle_stealth(uarmf, oldprop, TRUE);
        }
        break;
    case FUMBLE_BOOTS:
        if (!(HFumbling & ~TIMEOUT))
            incr_itimeout(&HFumbling, rnd(20));
        break;
    case LEVITATION_BOOTS:
        if (!oldprop && !HLevitation && !(BLevitation & FROMOUTSIDE)) {
            uarmf->known = 1; /* might come off if putting on over a sink,
                               * so uarmf could be Null below; status line
                               * gets updated during brief interval they're
                               * worn so hero and player learn enchantment */
            context.botl = 1; /* status hilites might mark AC changed */
            makeknown(uarmf->otyp);
            float_up();
            if (Levitation)
                spoteffects(FALSE); /* for sink effect */
        } else {
            float_vs_flight(); /* maybe toggle BFlying's I_SPECIAL */
        }
        break;
    default:
        impossible(unknown_type, c_boots, uarmf->otyp);
    }
    if (uarmf) { /* could be Null here (levitation boots put on over a sink) */
        uarmf->known = 1; /* boots' +/- evident because of status line AC */
        oprops_on(uarmf, WORN_BOOTS);
    }
    return 0;
}

int
Boots_off(VOID_ARGS)
{
    struct obj *otmp = uarmf;
    int otyp = otmp->otyp;
    long oldprop = u.uprops[objects[otyp].oc_oprop].extrinsic & ~WORN_BOOTS;

    oprops_off(uarmf, WORN_BOOTS);

    context.takeoff.mask &= ~W_ARMF;
    /* For levitation, float_down() returns if Levitation, so we
     * must do a setworn() _before_ the levitation case.
     */
    setworn((struct obj *) 0, W_ARMF);
    switch (otyp) {
    case SPEED_BOOTS:
        if (!Very_fast && !context.takeoff.cancelled_don) {
            makeknown(otyp);
            You_feel("yourself slow down%s.", Fast ? " a bit" : "");
        }
        break;
    case WATER_WALKING_BOOTS:
        /* check for lava since fireproofed boots make it viable */
        if ((is_pool(u.ux, u.uy) || is_lava(u.ux, u.uy))
            && !Levitation && !Flying && !is_clinger(youmonst.data)
            && !context.takeoff.cancelled_don
            /* avoid recursive call to lava_effects() */
            && !iflags.in_lava_effects) {
            /* make boots known in case you survive the drowning */
            makeknown(otyp);
            spoteffects(TRUE);
        }
        break;
    case ELVEN_BOOTS:
    case DARK_ELVEN_BOOTS:
        toggle_stealth(otmp, oldprop, FALSE);
        break;
    case FUMBLE_BOOTS:
        if (!(HFumbling & ~TIMEOUT))
            HFumbling = EFumbling = 0;
        break;
    case LEVITATION_BOOTS:
        if (!oldprop && !HLevitation && !(BLevitation & FROMOUTSIDE)
            && !context.takeoff.cancelled_don) {
            (void) float_down(0L, 0L);
            makeknown(otyp);
        } else {
            float_vs_flight(); /* maybe toggle (BFlying & I_SPECIAL) */
        }
        break;
    case LOW_BOOTS:
    case DWARVISH_BOOTS:
    case HIGH_BOOTS:
    case JUMPING_BOOTS:
    case KICKING_BOOTS:
    case ORCISH_BOOTS:
    case MEAT_BOOTS:
        break;
    default:
        impossible(unknown_type, c_boots, otyp);
    }
    context.takeoff.cancelled_don = FALSE;
    return 0;
}

STATIC_PTR int
Cloak_on(VOID_ARGS)
{
    int otyp = uarmc->otyp;
    long oldprop =
        u.uprops[objects[uarmc->otyp].oc_oprop].extrinsic & ~WORN_CLOAK;

    if (Is_dragon_scales(uarmc)
        && otyp != SHIMMERING_DRAGON_SCALES
        && otyp != CELESTIAL_DRAGON_SCALES) {
        /* most scales are handled the same in this function */
        otyp = GRAY_DRAGON_SCALES;
    }

    switch (otyp) {
    case ORCISH_CLOAK:
    case DWARVISH_CLOAK:
    case CLOAK_OF_MAGIC_RESISTANCE:
    case ROBE:
    case CLOAK:
    case GRAY_DRAGON_SCALES:
        break;
    case CLOAK_OF_PROTECTION:
        makeknown(uarmc->otyp);
        break;
    case ELVEN_CLOAK:
    case DARK_ELVEN_CLOAK:
        toggle_stealth(uarmc, oldprop, TRUE);
        break;
    case CLOAK_OF_DISPLACEMENT:
    case SHIMMERING_DRAGON_SCALES:
        toggle_displacement(uarmc, oldprop, TRUE);
        break;
    case CELESTIAL_DRAGON_SCALES:
        /* setworn() has already set extrinisic flying */
        float_vs_flight(); /* block flying if levitating */
        check_wings(TRUE); /* are we in a form that has wings and can already fly? */

        if (Flying) {
            boolean already_flying;

            /* to determine whether this flight is new we have to muck
               about in the Flying intrinsic (actually extrinsic) */
            EFlying &= ~W_ARMC;
            already_flying = !!Flying;
            EFlying |= W_ARMC;

            if (!already_flying) {
                context.botl = TRUE; /* status: 'Fly' On */
                You("are now in flight.");
            }
        }
        break;
    case MUMMY_WRAPPING:
        /* Note: it's already being worn, so we have to cheat here. */
        if ((HInvis || EInvis) && !Blind) {
            newsym(u.ux, u.uy);
            You("can %s!", See_invisible ? "no longer see through yourself"
                                         : see_yourself);
        }
        break;
    case CLOAK_OF_INVISIBILITY:
        /* since cloak of invisibility was worn, we know mummy wrapping
           wasn't, so no need to check `oldprop' against blocked */
        if (!oldprop && !HInvis && !Blind) {
            makeknown(uarmc->otyp);
            newsym(u.ux, u.uy);
            pline("Suddenly you can%s yourself.",
                  See_invisible ? " see through" : "not see");
        }
        break;
    case OILSKIN_CLOAK:
        pline("%s very tightly.", Tobjnam(uarmc, "fit"));
        break;
    /* Alchemy smock gives poison _and_ acid resistance */
    case ALCHEMY_SMOCK:
        EAcid_resistance |= WORN_CLOAK;
        break;
    default:
        impossible(unknown_type, c_cloak, uarmc->otyp);
    }
    if (uarmc) { /* no known instance of !uarmc here but play it safe */
        uarmc->known = 1; /* cloak's +/- evident because of status line AC */
        oprops_on(uarmc, WORN_CLOAK);
    }
    toggle_armor_light(uarmc, TRUE);
    return 0;
}

int
Cloak_off(VOID_ARGS)
{
    struct obj *otmp = uarmc;
    int otyp = otmp->otyp;
    long oldprop = u.uprops[objects[otyp].oc_oprop].extrinsic & ~WORN_CLOAK;
    boolean was_arti_light = otmp && otmp->lamplit && artifact_light(otmp);

    if (Is_dragon_scales(uarmc)
        && otyp != SHIMMERING_DRAGON_SCALES
        && otyp != CELESTIAL_DRAGON_SCALES) {
        /* most scales are handled the same in this function */
        otyp = GRAY_DRAGON_SCALES;
    }

    oprops_off(uarmc, WORN_CLOAK);

    context.takeoff.mask &= ~W_ARMC;

    switch (otyp) {
    case ORCISH_CLOAK:
    case DWARVISH_CLOAK:
    case CLOAK_OF_PROTECTION:
    case CLOAK_OF_MAGIC_RESISTANCE:
    case OILSKIN_CLOAK:
    case ROBE:
    case CLOAK:
    case GRAY_DRAGON_SCALES:
        break;
    case ELVEN_CLOAK:
    case DARK_ELVEN_CLOAK:
        toggle_stealth(otmp, oldprop, FALSE);
        break;
    case CLOAK_OF_DISPLACEMENT:
    case SHIMMERING_DRAGON_SCALES:
        toggle_displacement(otmp, oldprop, FALSE);
        break;
    case CELESTIAL_DRAGON_SCALES: {
        boolean was_flying = !!Flying;

        /* remove scales 'early' to determine whether Flying changes */
        setworn((struct obj *) 0, W_ARMC);
        float_vs_flight(); /* probably not needed here */
        check_wings(TRUE); /* are we in a form that has wings and can already fly? */
        if (was_flying && !Flying) {
            context.botl = TRUE; /* status: 'Fly' Off */
            You("%s.", (is_pool_or_lava(u.ux, u.uy)
                        || Is_waterlevel(&u.uz) || Is_airlevel(&u.uz))
                          ? "stop flying"
                          : "land");
            spoteffects(TRUE);
        }
        break;
    }
    case MUMMY_WRAPPING:
        setworn((struct obj *) 0, W_ARMC);
        if (Invis && !Blind) {
            newsym(u.ux, u.uy);
            You("can %s.", See_invisible ? "see through yourself"
                                         : "no longer see yourself");
        }
        break;
    case CLOAK_OF_INVISIBILITY:
        if (!oldprop && !HInvis && !Blind) {
            makeknown(CLOAK_OF_INVISIBILITY);
            newsym(u.ux, u.uy);
            pline("Suddenly you can %s.",
                  See_invisible ? "no longer see through yourself"
                                : see_yourself);
        }
        break;
    /* Alchemy smock gives poison _and_ acid resistance */
    case ALCHEMY_SMOCK:
        EAcid_resistance &= ~WORN_CLOAK;
        break;
    default:
        impossible(unknown_type, c_cloak, otyp);
    }

    setworn((struct obj *) 0, W_ARMC);
    if (was_arti_light)
        toggle_armor_light(otmp, FALSE);
    return 0;
}

STATIC_PTR
int
Helmet_on(VOID_ARGS)
{
    long oldprop =
        u.uprops[objects[uarmh->otyp].oc_oprop].extrinsic & ~WORN_HELMET;

    switch (uarmh->otyp) {
    case FEDORA:
    case TOQUE:
    case HELMET:
    case DENTED_POT:
    case ELVEN_HELM:
    case DARK_ELVEN_HELM:
    case DWARVISH_HELM:
    case ORCISH_HELM:
    case HELM_OF_TELEPATHY:
    case MEAT_HELMET:
        break;
    case HELM_OF_CAUTION:
        see_monsters();
        break;
    case HELM_OF_BRILLIANCE:
        adj_abon(uarmh, uarmh->spe);
        break;
    case CORNUTHAUM:
        /* people think marked wizards know what they're talking about,
           but it takes trained arrogance to pull it off, and the actual
           enchantment of the hat is irrelevant */
        ABON(A_CHA) += (Role_if(PM_WIZARD) ? 1 : -1);
        context.botl = 1;
        makeknown(uarmh->otyp);
        break;
    case HELM_OF_SPEED:
        if (!oldprop && !(HFast & TIMEOUT)) {
            makeknown(uarmh->otyp);
            You_feel("yourself speed up%s.",
                     (oldprop || HFast) ? " a bit more" : "");
        }
        break;
    case HELM_OF_OPPOSITE_ALIGNMENT:
        uarmh->known = 1; /* do this here because uarmh could get cleared */
        /* changing alignment can toggle off active artifact properties,
           including levitation; uarmh could get dropped or destroyed here
           by hero falling onto a polymorph trap or into water (emergency
           disrobe) or maybe lava (probably not, helm isn't 'organic') */
        uchangealign((u.ualign.type != A_NEUTRAL)
                         ? -sgn(u.ualign.type)
                         : (uarmh->o_id % 2) ? A_CHAOTIC : A_LAWFUL,
                     1);
        /* makeknown(HELM_OF_OPPOSITE_ALIGNMENT); -- below, after Tobjnam() */
    /*FALLTHRU*/
    case DUNCE_CAP:
        if (uarmh && !uarmh->cursed) {
            if (Blind)
                pline("%s for a moment.", Tobjnam(uarmh, "vibrate"));
            else
                pline("%s %s for a moment.", Tobjnam(uarmh, "glow"),
                      hcolor(NH_BLACK));
            curse(uarmh);
            /* curse() doesn't touch bknown so doesn't update persistent
               inventory; do so now [set_bknown() calls update_inventory()] */
            if (Blind)
                set_bknown(uarmh, 0); /* lose bknown if previously set */
            else if (Role_if(PM_PRIEST))
                set_bknown(uarmh, 1); /* (bknown should already be set) */
            else if (uarmh->bknown)
                update_inventory(); /* keep bknown as-is; display the curse */
        }
        context.botl = 1; /* reveal new alignment or INT & WIS */
        if (Hallucination) {
            pline("My brain hurts!"); /* Monty Python's Flying Circus */
        } else if (uarmh && uarmh->otyp == DUNCE_CAP) {
            You_feel("%s.", /* track INT change; ignore WIS */
                     ACURR(A_INT)
                             <= (ABASE(A_INT) + ABON(A_INT) + ATEMP(A_INT))
                         ? "like sitting in a corner"
                         : "giddy");
        } else {
            /* [message formerly given here moved to uchangealign()] */
            makeknown(HELM_OF_OPPOSITE_ALIGNMENT);
        }
        break;
    default:
        impossible(unknown_type, c_helmet, uarmh->otyp);
    }
    /* uarmh could be Null due to uchangealign() */
    if (uarmh) {
        uarmh->known = 1; /* helmet's +/- evident because of status line AC */
        oprops_on(uarmh, WORN_HELMET);
    }
    return 0;
}

int
Helmet_off(VOID_ARGS)
{
    oprops_off(uarmh, WORN_HELMET);

    context.takeoff.mask &= ~W_ARMH;

    switch (uarmh->otyp) {
    case FEDORA:
    case TOQUE:
    case HELMET:
    case DENTED_POT:
    case ELVEN_HELM:
    case DARK_ELVEN_HELM:
    case DWARVISH_HELM:
    case ORCISH_HELM:
    case MEAT_HELMET:
        break;
    case DUNCE_CAP:
        context.botl = 1;
        break;
    case CORNUTHAUM:
        if (!context.takeoff.cancelled_don) {
            ABON(A_CHA) += (Role_if(PM_WIZARD) ? -1 : 1);
            context.botl = 1;
        }
        break;
    case HELM_OF_SPEED:
        setworn((struct obj *) 0, W_ARMH);
        if (!Very_fast && !context.takeoff.cancelled_don)
            You_feel("yourself slow down%s.", Fast ? " a bit" : "");
        break;
    case HELM_OF_TELEPATHY:
    case HELM_OF_CAUTION:
        /* need to update ability before calling see_monsters() */
        setworn((struct obj *) 0, W_ARMH);
        see_monsters();
        return 0;
    case HELM_OF_BRILLIANCE:
        if (!context.takeoff.cancelled_don)
            adj_abon(uarmh, -uarmh->spe);
        break;
    case HELM_OF_OPPOSITE_ALIGNMENT:
        /* changing alignment can toggle off active artifact
           properties, including levitation; uarmh could get
           dropped or destroyed here */
        uchangealign(u.ualignbase[A_CURRENT], 2);
        break;
    default:
        impossible(unknown_type, c_helmet, uarmh->otyp);
    }
    setworn((struct obj *) 0, W_ARMH);
    context.takeoff.cancelled_don = FALSE;
    return 0;
}

STATIC_PTR
int
Gloves_on(VOID_ARGS)
{
    switch (uarmg->otyp) {
    case GLOVES:
    case DARK_ELVEN_GLOVES:
    case GAUNTLETS:
    case MEAT_GLOVES:
        break;
    case GAUNTLETS_OF_FUMBLING:
        if (!(HFumbling & ~TIMEOUT))
            incr_itimeout(&HFumbling, rnd(20));
        break;
    case GAUNTLETS_OF_POWER:
    case MUMMIFIED_HAND: /* the Hand of Vecna */
        if (u.ualign.record >= 20
            && uarmg->oartifact == ART_GAUNTLETS_OF_PURITY) {
            pline_The("%s sense your piety, and slide comfortably over your %s.",
                      artiname(uarmg->oartifact), makeplural(body_part(HAND)));
        }
        makeknown(uarmg->otyp);
        context.botl = 1; /* taken care of in attrib.c */
        break;
    case GAUNTLETS_OF_DEXTERITY:
        adj_abon(uarmg, uarmg->spe);
        break;
    case GAUNTLETS_OF_PROTECTION:
        makeknown(uarmg->otyp);
        break;
    default:
        impossible(unknown_type, c_gloves, uarmg->otyp);
    }
    if (uarmg) { /* no known instance of !uarmg here but play it safe */
        uarmg->known = 1; /* gloves' +/- evident because of status line AC */
        oprops_on(uarmg, WORN_GLOVES);
    }
    return 0;
}

STATIC_OVL void
wielding_corpse(obj, voluntary)
struct obj *obj;
boolean voluntary; /* taking gloves off on purpose? */
{
    char kbuf[BUFSZ];

    if (!obj || obj->otyp != CORPSE)
        return;
    if (obj != uwep && (obj != uswapwep || !u.twoweap))
        return;

    if (touch_petrifies(&mons[obj->corpsenm]) && !Stone_resistance) {
        You("now wield %s in your bare %s.",
            corpse_xname(obj, (const char *) 0, CXN_ARTICLE),
            makeplural(body_part(HAND)));
        Sprintf(kbuf, "%s gloves while wielding %s",
                voluntary ? "removing" : "losing", killer_xname(obj));
        instapetrify(kbuf);
        /* life-saved; can't continue wielding cockatrice corpse though */
        remove_worn_item(obj, FALSE);
    }
}

int
Gloves_off(VOID_ARGS)
{
    boolean on_purpose = !context.mon_moving && !uarmg->in_use;

    oprops_off(uarmg, WORN_GLOVES);

    if (uarmg->greased)
        Glib &= ~FROMOUTSIDE;

    context.takeoff.mask &= ~W_ARMG;

    switch (uarmg->otyp) {
    case GLOVES:
    case DARK_ELVEN_GLOVES:
    case GAUNTLETS:
    case GAUNTLETS_OF_PROTECTION:
    case MEAT_GLOVES:
        break;
    case GAUNTLETS_OF_FUMBLING:
        if (!(HFumbling & ~TIMEOUT))
            HFumbling = EFumbling = 0;
        break;
    case GAUNTLETS_OF_POWER:
    case MUMMIFIED_HAND: /* the Hand of Vecna */
        makeknown(uarmg->otyp);
        context.botl = 1; /* taken care of in attrib.c */
        break;
    case GAUNTLETS_OF_DEXTERITY:
        if (!context.takeoff.cancelled_don)
            adj_abon(uarmg, -uarmg->spe);
        break;
    default:
        impossible(unknown_type, c_gloves, uarmg->otyp);
    }
    setworn((struct obj *) 0, W_ARMG);
    context.takeoff.cancelled_don = FALSE;
    (void) encumber_msg(); /* immediate feedback for GoP */

    /* usually can't remove gloves when they're slippery but it can
       be done by having them fall off (polymorph), stolen, or
       destroyed (scroll, overenchantment, monster spell); if that
       happens, 'cure' slippery fingers so that it doesn't transfer
       from gloves to bare hands */
    if (Glib)
        make_glib(0); /* for update_inventory() */

    /* prevent wielding cockatrice when not wearing gloves */
    if (uwep && uwep->otyp == CORPSE)
        wielding_corpse(uwep, on_purpose);

    /* you may now be touching some material you hate */
    if (uwep)
        retouch_object(&uwep, will_touch_skin(W_WEP), FALSE);

    /* KMH -- ...or your secondary weapon when you're wielding it
       [This case can't actually happen; twoweapon mode won't
       engage if a corpse has been set up as the alternate weapon.] */
    if (u.twoweap && uswapwep && uswapwep->otyp == CORPSE)
        wielding_corpse(uswapwep, on_purpose);

    return 0;
}

STATIC_PTR int
Shield_on(VOID_ARGS)
{
    /* no shield currently requires special handling when put on, but we
       keep this uncommented in case somebody adds a new one which does
       [reflection is handled by setting u.uprops[REFLECTING].extrinsic
       in setworn() called by armor_or_accessory_on() before Shield_on()] */
    switch (uarms->otyp) {
    case SMALL_SHIELD:
    case ELVEN_SHIELD:
    case BRACERS:
    case RUNED_BRACERS:
    case DARK_ELVEN_BRACERS:
    case URUK_HAI_SHIELD:
    case ORCISH_SHIELD:
    case DWARVISH_ROUNDSHIELD:
    case LARGE_SHIELD:
    case SHIELD_OF_REFLECTION:
    case SHIELD_OF_LIGHT:
    case SHIELD_OF_MOBILITY:
    case MEAT_SHIELD:
        break;
    default:
        impossible(unknown_type, c_shield, uarms->otyp);
    }
    if (uarms) { /* no known instance of !uarmgs here but play it safe */
        uarms->known = 1; /* shield's +/- evident because of status line AC */
        oprops_on(uarms, WORN_SHIELD);
    }
    toggle_armor_light(uarms, TRUE);
    return 0;
}

int
Shield_off(VOID_ARGS)
{
    struct obj *otmp = uarms;
    boolean was_arti_light = otmp && otmp->lamplit && artifact_light(otmp);

    if (otmp)
        oprops_off(otmp, WORN_SHIELD);
    context.takeoff.mask &= ~W_ARMS;
    setworn((struct obj *) 0, W_ARMS);

    /* no shield currently requires special handling when taken off, but we
       keep this uncommented in case somebody adds a new one which does */
    switch (otmp->otyp) {
    case SMALL_SHIELD:
    case ELVEN_SHIELD:
    case BRACERS:
    case RUNED_BRACERS:
    case DARK_ELVEN_BRACERS:
    case URUK_HAI_SHIELD:
    case ORCISH_SHIELD:
    case DWARVISH_ROUNDSHIELD:
    case LARGE_SHIELD:
    case SHIELD_OF_REFLECTION:
    case SHIELD_OF_LIGHT:
    case SHIELD_OF_MOBILITY:
    case MEAT_SHIELD:
        break;
    default:
        impossible(unknown_type, c_shield, otmp->otyp);
    }
    if (was_arti_light)
        toggle_armor_light(otmp, FALSE);
    return 0;
}

STATIC_PTR int
Shirt_on(VOID_ARGS)
{
    /* no shirt currently requires special handling when put on, but we
       keep this uncommented in case somebody adds a new one which does */
    switch (uarmu->otyp) {
    case HAWAIIAN_SHIRT:
    case STRIPED_SHIRT:
    case T_SHIRT:
        break;
    default:
        impossible(unknown_type, c_shirt, uarmu->otyp);
    }
    if (uarmu) { /* no known instances of !uarmu here but play it safe */
        uarmu->known = 1; /* shirt's +/- evident because of status line AC */
        oprops_on(uarmu, WORN_SHIRT);
    }
    return 0;
}

int
Shirt_off(VOID_ARGS)
{
    oprops_off(uarmu, WORN_SHIRT);

    context.takeoff.mask &= ~W_ARMU;

    /* no shirt currently requires special handling when taken off, but we
       keep this uncommented in case somebody adds a new one which does */
    switch (uarmu->otyp) {
    case HAWAIIAN_SHIRT:
    case STRIPED_SHIRT:
    case T_SHIRT:
        break;
    default:
        impossible(unknown_type, c_shirt, uarmu->otyp);
    }

    setworn((struct obj *) 0, W_ARMU);
    return 0;
}

/* handle extra abilities for hero wearing dragon-scaled armor */
void
dragon_armor_handling(struct obj *otmp, boolean puton)
{
    boolean was_flying;

    /* as of first merging this behavior in from NetHack 3.7,
       this only happens on dragon-scaled body armor - NOT scales
       worn in the cloak slot. */
    if (!otmp)
        return;

    switch (Dragon_armor_to_scales(otmp)) {
    /* gray: no extra effect */
    case GREEN_DRAGON_SCALES:
        if (puton) {
            ESick_resistance |= W_ARM;
            if (Sick) {
                You_feel("cured.  What a relief!");
                Sick = 0L;
            }
        } else {
            ESick_resistance &= ~W_ARM;
        }
        break;
    case GOLD_DRAGON_SCALES:
        if (puton) {
            EInfravision |= W_ARM;
            EClairvoyant |= W_ARM;
        } else {
            EInfravision &= ~W_ARM;
            EClairvoyant &= ~W_ARM;
        }
        break;
    case ORANGE_DRAGON_SCALES:
        if (puton) {
            Free_action |= W_ARM;
        } else {
            Free_action &= ~W_ARM;
        }
        break;
    case SILVER_DRAGON_SCALES:
        if (puton) {
            ECold_resistance |= W_ARM;
        } else {
            ECold_resistance &= ~W_ARM;
        }
        break;
    case BLUE_DRAGON_SCALES:
        if (puton) {
            if (!Very_fast)
                pline("You speed up%s.", Fast ? " a bit more" : "");
            EFast |= W_ARM;
        } else {
            EFast &= ~W_ARM;
            if (!Very_fast && !context.takeoff.cancelled_don)
                pline("You slow down.");
        }
        break;
    case YELLOW_DRAGON_SCALES:
        if (puton) {
            EStone_resistance |= W_ARM;
            if (Stone_resistance && Stoned) {
                make_stoned(0L, "You no longer seem to be petrifying.", 0,
                            (char *) 0);
            }
        } else {
            EStone_resistance &= ~W_ARM;
        }
        break;
    case WHITE_DRAGON_SCALES:
        if (puton) {
            EWwalking |= W_ARM;
        } else {
            EWwalking &= ~W_ARM;
        }
        break;
    case SHIMMERING_DRAGON_SCALES:
        if (puton) {
            toggle_displacement(uarm, (EDisplaced & ~W_ARM), TRUE);
            if (Stunned) {
                You_feel("%s now.",
                         Hallucination ? "less wobbly"
                                       : "a bit steadier");
                Stunned = 0L;
            }
            EStun_resistance |= W_ARM;
        } else {
            toggle_displacement(otmp, (EDisplaced & ~W_ARM), FALSE);
            EStun_resistance &= ~W_ARM;
        }
        break;
    case SEA_DRAGON_SCALES:
        if (puton) {
            if (Strangled) {
                You("can suddenly breathe again!");
                Strangled = 0L;
            }
            ESwimming |= W_ARM;
        } else {
            ESwimming &= ~W_ARM;
            if (Underwater) {
                setworn((struct obj *) 0, W_ARM);
                if (!Breathless && !Amphibious && !Swimming) {
                    You("suddenly inhale an unhealthy amount of %s!",
                        hliquid("water"));
                    (void) drown();
                }
            }
        }
        break;
    case CELESTIAL_DRAGON_SCALES:
        if (puton) {
            ESleep_resistance  |= W_ARM;
            EShock_resistance  |= W_ARM;

            /* setworn() has already set extrinisic flying */
            float_vs_flight(); /* block flying if levitating */
            check_wings(TRUE); /* are we in a form that has wings and can already fly? */

            if (Flying) {
                boolean already_flying;
                long cramped_wings = BFlying;

                /* to determine whether this flight is new we have to muck
                   about in the Flying intrinsic (actually extrinsic) */
                EFlying &= ~W_ARM;
                BFlying &= ~W_ARM;
                already_flying = !!Flying;
                BFlying |= cramped_wings;
                EFlying |= W_ARM;

                if (!already_flying) {
                    context.botl = TRUE; /* status: 'Fly' On */
                    You("are now in flight.");
                }
            }
        } else {
            ESleep_resistance  &= ~W_ARM;
            EShock_resistance  &= ~W_ARM;

            was_flying = !!Flying;

            /* remove armor 'early' to determine whether Flying changes */
            setworn((struct obj *) 0, W_ARM);
            float_vs_flight(); /* probably not needed here */
            check_wings(TRUE); /* are we in a form that has wings and can already fly? */
            if (was_flying && !Flying) {
                context.botl = TRUE; /* status: 'Fly' Off */
                You("%s.", (is_pool_or_lava(u.ux, u.uy)
                            || Is_waterlevel(&u.uz) || Is_airlevel(&u.uz))
                              ? "stop flying"
                              : "land");
                spoteffects(TRUE);
            }
        }
        break;
    case SHADOW_DRAGON_SCALES:
        /* ultravision handled in objects.c */
        if (puton) {
            ESleep_resistance  |= W_ARM;
            EDrain_resistance  |= W_ARM;
        } else {
            ESleep_resistance  &= ~W_ARM;
            EDrain_resistance  &= ~W_ARM;
        }
        break;
    case CHROMATIC_DRAGON_SCALES:
        /* magic res handled in objects.c */
        if (puton) {
            EPoison_resistance |= W_ARM;
            EFire_resistance   |= W_ARM;
            ECold_resistance   |= W_ARM;
            ESleep_resistance  |= W_ARM;
            EDisint_resistance |= W_ARM;
            EShock_resistance  |= W_ARM;
            EAcid_resistance   |= W_ARM;
            EStone_resistance  |= W_ARM;
            EReflecting        |= W_ARM;
            if (Stone_resistance && Stoned) {
                make_stoned(0L, "You no longer seem to be petrifying.", 0,
                            (char *) 0);
            }
        } else {
            EPoison_resistance &= ~W_ARM;
            EFire_resistance   &= ~W_ARM;
            ECold_resistance   &= ~W_ARM;
            ESleep_resistance  &= ~W_ARM;
            EDisint_resistance &= ~W_ARM;
            EShock_resistance  &= ~W_ARM;
            EAcid_resistance   &= ~W_ARM;
            EStone_resistance  &= ~W_ARM;
            EReflecting        &= ~W_ARM;
        }
        break;
    default:
        break;
    }
}

STATIC_PTR
int
Armor_on(VOID_ARGS)
{
    if (!uarm) /* no known instances of !uarm here but play it safe */
        return 0;

    if (Role_if(PM_MONK))
        You_feel("extremely uncomfortable wearing such armor.");

    uarm->known = 1; /* suit's +/- evident because of status line AC */
    check_wings(FALSE);
    oprops_on(uarm, W_ARM);
    dragon_armor_handling(uarm, TRUE);
    toggle_armor_light(uarm, TRUE);
    return 0;
}

int
Armor_off(VOID_ARGS)
{
    struct obj *otmp = uarm;
    boolean was_arti_light = otmp && otmp->lamplit && artifact_light(otmp);

    if (Role_if(PM_MONK))
        You_feel("much more comfortable and free now.");

    if (otmp)
        oprops_off(otmp, W_ARM);

    context.takeoff.mask &= ~W_ARM;
    context.takeoff.cancelled_don = FALSE;

    dragon_armor_handling(otmp, FALSE);

    setworn((struct obj *) 0, W_ARM);

    check_wings(FALSE);

    if (was_arti_light)
        toggle_armor_light(otmp, FALSE);
    return 0;
}

/* The gone functions differ from the off functions in that if you die from
 * taking it off and have life saving, you still die.  [Obsolete reference
 * to lack of fire resistance being fatal in hell (nethack 3.0) and life
 * saving putting a removed item back on to prevent that from immediately
 * repeating.]
 */
int
Armor_gone()
{
    struct obj *otmp = uarm;
    boolean was_arti_light = otmp && otmp->lamplit && artifact_light(otmp);

    if (Role_if(PM_MONK))
        You_feel("much more comfortable and free now.");

    if (otmp)
        oprops_off(otmp, W_ARM);

    context.takeoff.mask &= ~W_ARM;
    setnotworn(otmp);
    context.takeoff.cancelled_don = FALSE;

    check_wings(FALSE);

    dragon_armor_handling(otmp, FALSE);

    if (was_arti_light && !artifact_light(otmp)) {
        end_burn(otmp, FALSE);
        if (!Blind)
            pline("%s shining.", Tobjnam(otmp, "stop"));
    }
    return 0;
}

/* Some monster forms' flight is blocked by most body armor. */
void
check_wings(silent)
boolean silent; /* we assume a wardrobe change if false */
{
    static struct obj *last_worn_armor;
    boolean old_flying = Flying;

    BFlying &= ~W_ARM;
    if (!big_wings(raceptr(&youmonst)))
        return;

    if (!uarm) {
        if (!silent && Flying)
            You("spread your wings%s.",
                old_flying ? "" : " and take flight");
    } else if (Is_dragon_scaled_armor(uarm) && !is_hard(uarm)) {
        if (!silent)
            You("arrange the scales around your wings.");
    } else if (uarm && !is_hard(uarm)) {
        if (!silent && uarm != last_worn_armor)
            Your("%s seems to have holes for wings.", simpleonames(uarm));
    } else {
        BFlying |= W_ARM;
        if (!silent)
            You("fold your wings under your suit.");
    }

    if (uarm)
        last_worn_armor = uarm;

    if (Flying != old_flying)
        context.botl = TRUE;
}

STATIC_OVL void
Amulet_on()
{
    /* make sure amulet isn't wielded; can't use remove_worn_item()
       here because it has already been set worn in amulet slot */
    if (uamul == uwep)
        setuwep((struct obj *) 0);
    else if (uamul == uswapwep)
        setuswapwep((struct obj *) 0);
    else if (uamul == uquiver)
        setuqwep((struct obj *) 0);

    switch (uamul->otyp) {
    case AMULET_OF_ESP:
    case AMULET_OF_LIFE_SAVING:
    case AMULET_VERSUS_POISON:
    case AMULET_OF_REFLECTION:
    case AMULET_OF_MAGIC_RESISTANCE:
    case FAKE_AMULET_OF_YENDOR:
        break;
    case AMULET_OF_MAGICAL_BREATHING:
        if (Strangled) {
            /* assume that this is an engulfing suffocater or some other
             * non-amulet form of strangulation. Only the amulet form of
             * strangulation constricts the neck and ignores magical breathing.
             */
            You("can suddenly breathe again!");
            Strangled = 0L;
        }
        break;
    case AMULET_OF_UNCHANGING:
        if (Slimed)
            make_slimed(0L, (char *) 0);
        break;
    case AMULET_OF_CHANGE: {
        int orig_sex = poly_gender();

        if (Unchanging)
            break;
        change_sex();
        /* Don't use same message as polymorph */
        if (orig_sex != poly_gender()) {
            makeknown(AMULET_OF_CHANGE);
            You("are suddenly very %s!",
                flags.female ? "feminine" : "masculine");
            context.botl = 1;
        } else
            /* already polymorphed into single-gender monster; only
               changed the character's base sex */
            You("don't feel like yourself.");
        pline_The("amulet disintegrates!");
        if (orig_sex == poly_gender() && uamul->dknown
            && !objects[AMULET_OF_CHANGE].oc_name_known
            && !objects[AMULET_OF_CHANGE].oc_uname)
            docall(uamul);
        useup(uamul);
        break;
    }
    case AMULET_OF_STRANGULATION:
        if (can_be_strangled(&youmonst)) {
            makeknown(AMULET_OF_STRANGULATION);
            if (Strangled == 0 || Strangled > 6) {
                Strangled = 6L;
            }
            context.botl = TRUE;
            pline("It constricts your throat!");
        }
        break;
    case AMULET_OF_RESTFUL_SLEEP: {
        long newnap = (long) rnd(100), oldnap = (HSleepy & TIMEOUT);

        /* avoid clobbering FROMOUTSIDE bit, which might have
           gotten set by previously eating one of these amulets */
        if (newnap < oldnap || oldnap == 0L)
            HSleepy = (HSleepy & ~TIMEOUT) | newnap;
        break;
    }
    case AMULET_OF_FLYING:
        /* setworn() has already set extrinisic flying */
        float_vs_flight(); /* block flying if levitating */
        check_wings(TRUE); /* are we in a form that has wings and can already fly? */

        if (Flying) {
            boolean already_flying;

            /* to determine whether this flight is new we have to muck
               about in the Flying intrinsic (actually extrinsic) */
            EFlying &= ~W_AMUL;
            already_flying = !!Flying;
            EFlying |= W_AMUL;

            if (!already_flying) {
                makeknown(AMULET_OF_FLYING);
                context.botl = TRUE; /* status: 'Fly' On */
                You("are now in flight.");
            }
        }
        break;
    case AMULET_OF_GUARDING:
        makeknown(AMULET_OF_GUARDING);
        find_ac();
        break;
    case AMULET_OF_YENDOR:
        break;
    }
}

void
Amulet_off()
{
    context.takeoff.mask &= ~W_AMUL;

    switch (uamul->otyp) {
    case AMULET_OF_ESP:
        /* need to update ability before calling see_monsters() */
        setworn((struct obj *) 0, W_AMUL);
        see_monsters();
        return;
    case AMULET_OF_LIFE_SAVING:
    case AMULET_VERSUS_POISON:
    case AMULET_OF_REFLECTION:
    case AMULET_OF_CHANGE:
    case AMULET_OF_UNCHANGING:
    case AMULET_OF_MAGIC_RESISTANCE:
    case FAKE_AMULET_OF_YENDOR:
        break;
    case AMULET_OF_MAGICAL_BREATHING:
        if (Underwater) {
            /* HMagical_breathing must be set off
                before calling drown() */
            setworn((struct obj *) 0, W_AMUL);
            if (!Breathless && !Amphibious && !Swimming) {
                You("suddenly inhale an unhealthy amount of %s!",
                    hliquid("water"));
                (void) drown();
            }
            return;
        }
        break;
    case AMULET_OF_STRANGULATION:
        if (Strangled) {
            Strangled = 0L;
            context.botl = TRUE;
            if (Breathless)
                Your("%s is no longer constricted!", body_part(NECK));
            else
                You("can breathe more easily!");
        }
        break;
    case AMULET_OF_RESTFUL_SLEEP:
        setworn((struct obj *) 0, W_AMUL);
        /* HSleepy = 0L; -- avoid clobbering FROMOUTSIDE bit */
        if (!ESleepy && !(HSleepy & ~TIMEOUT))
            HSleepy &= ~TIMEOUT; /* clear timeout bits */
        return;
    case AMULET_OF_FLYING: {
        boolean was_flying = !!Flying;

        /* remove amulet 'early' to determine whether Flying changes */
        setworn((struct obj *) 0, W_AMUL);
        float_vs_flight(); /* probably not needed here */
        check_wings(TRUE); /* are we in a form that has wings and can already fly? */
        if (was_flying && !Flying) {
            makeknown(AMULET_OF_FLYING);
            context.botl = TRUE; /* status: 'Fly' Off */
            You("%s.", (is_pool_or_lava(u.ux, u.uy)
                        || Is_waterlevel(&u.uz) || Is_airlevel(&u.uz))
                          ? "stop flying"
                          : "land");
            spoteffects(TRUE);
        }
        break;
    }
    case AMULET_OF_GUARDING:
        find_ac();
        break;
    case AMULET_OF_YENDOR:
        break;
    }
    setworn((struct obj *) 0, W_AMUL);
    return;
}

/* handle ring discovery; comparable to learnwand() */
STATIC_OVL void
learnring(ring, observed)
struct obj *ring;
boolean observed;
{
    int ringtype = ring->otyp;

    /* if effect was observeable then we usually discover the type */
    if (observed) {
        /* if we already know the ring type which accomplishes this
           effect (assumes there is at most one type for each effect),
           mark this ring as having been seen (no need for makeknown);
           otherwise if we have seen this ring, discover its type */
        if (objects[ringtype].oc_name_known)
            ring->dknown = 1;
        else if (ring->dknown)
            makeknown(ringtype);
#if 0 /* see learnwand() */
        else
            ring->eknown = 1;
#endif
    }

    /* make enchantment of charged ring known (might be +0) and update
       perm invent window if we've seen this ring and know its type */
    if (ring->dknown && objects[ringtype].oc_name_known) {
        if (objects[ringtype].oc_charged)
            ring->known = 1;
        update_inventory();
    }
}

void
Ring_on(obj)
struct obj *obj;
{
    long oldprop = u.uprops[objects[obj->otyp].oc_oprop].extrinsic;
    int old_attrib, which;
    boolean observable;

    /* make sure ring isn't wielded; can't use remove_worn_item()
       here because it has already been set worn in a ring slot */
    if (obj == uwep)
        setuwep((struct obj *) 0);
    else if (obj == uswapwep)
        setuswapwep((struct obj *) 0);
    else if (obj == uquiver)
        setuqwep((struct obj *) 0);

    /* only mask out W_RING when we don't have both
       left and right rings of the same type */
    if ((oldprop & W_RING) != W_RING)
        oldprop &= ~W_RING;

    switch (obj->otyp) {
    case RIN_TELEPORTATION:
    case RIN_REGENERATION:
    case RIN_SEARCHING:
    case RIN_HUNGER:
    case RIN_AGGRAVATE_MONSTER:
    case RIN_POISON_RESISTANCE:
    case RIN_FIRE_RESISTANCE:
    case RIN_COLD_RESISTANCE:
    case RIN_SHOCK_RESISTANCE:
    case RIN_CONFLICT:
    case RIN_TELEPORT_CONTROL:
    case RIN_POLYMORPH:
    case RIN_POLYMORPH_CONTROL:
    case RIN_FREE_ACTION:
    case RIN_ANCIENT:
    case RIN_SLOW_DIGESTION:
    case RIN_SUSTAIN_ABILITY:
    case MEAT_RING:
        break;
    case RIN_STEALTH:
        if (maybe_polyd(is_giant(youmonst.data), Race_if(PM_GIANT))) {
            pline("This %s will not silence someone %s.",
                  xname(obj), rn2(2) ? "as large as you"
                                     : "of your stature");
            EStealth &= ~W_RING;
        } else {
            toggle_stealth(obj, oldprop, TRUE);
        }
        break;
    case RIN_WARNING:
        see_monsters();
        break;
    case RIN_SEE_INVISIBLE:
        /* can now see invisible monsters */
        set_mimic_blocking(); /* do special mimic handling */
        see_monsters();

        if (Invis && !oldprop && !HSee_invisible && !Blind) {
            newsym(u.ux, u.uy);
            pline("Suddenly you are transparent, but there!");
            learnring(obj, TRUE);
        }
        break;
    case RIN_INVISIBILITY:
        if (!oldprop && !HInvis && !BInvis && !Blind) {
            learnring(obj, TRUE);
            newsym(u.ux, u.uy);
            self_invis_message();
        }
        break;
    case RIN_LEVITATION:
        if (!oldprop && !HLevitation && !(BLevitation & FROMOUTSIDE)) {
            float_up();
            learnring(obj, TRUE);
            if (Levitation)
                spoteffects(FALSE); /* for sinks */
        } else {
            float_vs_flight(); /* maybe toggle (BFlying & I_SPECIAL) */
        }
        break;
    case RIN_LUSTROUS: /* grants invisibility and stealth */
        if (maybe_polyd(is_giant(youmonst.data), Race_if(PM_GIANT))) {
            pline_The("%s will not silence someone %s.",
                      xname(obj), rn2(2) ? "as large as you"
                                         : "of your stature");
            EStealth &= ~W_RING;
        } else {
            toggle_stealth(obj, oldprop, TRUE);
        }
        if (!oldprop && !HInvis && !BInvis && !Blind) {
            learnring(obj, TRUE);
            newsym(u.ux, u.uy);
            self_invis_message();
        }
        break;
    case RIN_GAIN_STRENGTH:
        which = A_STR;
        goto adjust_attrib;
    case RIN_GAIN_CONSTITUTION:
        which = A_CON;
        goto adjust_attrib;
    case RIN_ADORNMENT:
        which = A_CHA;
 adjust_attrib:
        old_attrib = ACURR(which);
        ABON(which) += obj->spe;
        observable = (old_attrib != ACURR(which));
        /* if didn't change, usually means ring is +0 but might
           be because nonzero couldn't go below min or above max;
           learn +0 enchantment if attribute value is not stuck
           at a limit [and ring has been seen and its type is
           already discovered, both handled by learnring()] */
        if (observable || !extremeattr(which))
            learnring(obj, observable);
        context.botl = 1;
        break;
    case RIN_INCREASE_ACCURACY: /* KMH */
        u.uhitinc += obj->spe;
        break;
    case RIN_INCREASE_DAMAGE:
        u.udaminc += obj->spe;
        break;
    case RIN_PROTECTION_FROM_SHAPE_CHAN:
        rescham();
        break;
    case RIN_PROTECTION:
        /* usually learn enchantment and discover type;
           won't happen if ring is unseen or if it's +0
           and the type hasn't been discovered yet */
        observable = (obj->spe != 0);
        learnring(obj, observable);
        if (obj->spe)
            find_ac(); /* updates botl */
        break;
    }
}

STATIC_OVL void
Ring_off_or_gone(obj, gone)
struct obj *obj;
boolean gone;
{
    long mask = (obj->owornmask & W_RING);
    int old_attrib, which;
    boolean observable;

    context.takeoff.mask &= ~mask;
    if (gone)
        setnotworn(obj);
    else
        setworn((struct obj *) 0, obj->owornmask);

    switch (obj->otyp) {
    case RIN_TELEPORTATION:
    case RIN_REGENERATION:
    case RIN_SEARCHING:
    case RIN_HUNGER:
    case RIN_AGGRAVATE_MONSTER:
    case RIN_POISON_RESISTANCE:
    case RIN_FIRE_RESISTANCE:
    case RIN_COLD_RESISTANCE:
    case RIN_SHOCK_RESISTANCE:
    case RIN_CONFLICT:
    case RIN_TELEPORT_CONTROL:
    case RIN_POLYMORPH:
    case RIN_POLYMORPH_CONTROL:
    case RIN_FREE_ACTION:
    case RIN_ANCIENT:
    case RIN_SLOW_DIGESTION:
    case RIN_SUSTAIN_ABILITY:
    case MEAT_RING:
        break;
    case RIN_STEALTH:
        toggle_stealth(obj, (EStealth & ~mask), FALSE);
        break;
    case RIN_WARNING:
        see_monsters();
        break;
    case RIN_SEE_INVISIBLE:
        /* Make invisible monsters go away */
        if (!See_invisible) {
            set_mimic_blocking(); /* do special mimic handling */
            see_monsters();
        }

        if (Invisible && !Blind) {
            newsym(u.ux, u.uy);
            pline("Suddenly you cannot see yourself.");
            learnring(obj, TRUE);
        }
        break;
    case RIN_INVISIBILITY:
        if (!Invis && !BInvis && !Blind) {
            newsym(u.ux, u.uy);
            Your("body seems to unfade%s.",
                 See_invisible ? " completely" : "..");
            learnring(obj, TRUE);
        }
        break;
    case RIN_LEVITATION:
        if (!(BLevitation & FROMOUTSIDE)) {
            (void) float_down(0L, 0L);
            if (!Levitation)
                learnring(obj, TRUE);
        } else {
            float_vs_flight(); /* maybe toggle (BFlying & I_SPECIAL) */
        }
        break;
    case RIN_LUSTROUS:
        toggle_stealth(obj, (EStealth & ~mask), FALSE);
        if (!Invis && !BInvis && !Blind) {
            newsym(u.ux, u.uy);
            Your("body seems to unfade%s.",
                 See_invisible ? " completely" : "..");
            learnring(obj, TRUE);
        }
        break;
    case RIN_GAIN_STRENGTH:
        which = A_STR;
        goto adjust_attrib;
    case RIN_GAIN_CONSTITUTION:
        which = A_CON;
        goto adjust_attrib;
    case RIN_ADORNMENT:
        which = A_CHA;
 adjust_attrib:
        old_attrib = ACURR(which);
        ABON(which) -= obj->spe;
        observable = (old_attrib != ACURR(which));
        /* same criteria as Ring_on() */
        if (observable || !extremeattr(which))
            learnring(obj, observable);
        context.botl = 1;
        break;
    case RIN_INCREASE_ACCURACY: /* KMH */
        u.uhitinc -= obj->spe;
        break;
    case RIN_INCREASE_DAMAGE:
        u.udaminc -= obj->spe;
        break;
    case RIN_PROTECTION:
        /* might have been put on while blind and we can now see
           or perhaps been forgotten due to amnesia */
        observable = (obj->spe != 0);
        learnring(obj, observable);
        if (obj->spe)
            find_ac(); /* updates botl */
        break;
    case RIN_PROTECTION_FROM_SHAPE_CHAN:
        /* If you're no longer protected, let the chameleons
         * change shape again -dgk
         */
        restartcham();
        break;
    }
}

void
Ring_off(obj)
struct obj *obj;
{
    Ring_off_or_gone(obj, FALSE);
}

void
Ring_gone(obj)
struct obj *obj;
{
    Ring_off_or_gone(obj, TRUE);
}

void
Blindf_on(otmp)
struct obj *otmp;
{
    boolean already_blind = Blind, changed = FALSE;

    /* blindfold might be wielded; release it for wearing */
    if (otmp->owornmask & W_WEAPONS)
        remove_worn_item(otmp, FALSE);
    setworn(otmp, W_TOOL);
    on_msg(otmp);

    if (Blind && !already_blind) {
        changed = TRUE;
        if (flags.verbose)
            You_cant("see any more.");
        /* set ball&chain variables before the hero goes blind */
        if (Punished)
            set_bc(0);
    } else if (already_blind && !Blind) {
        changed = TRUE;
        /* "You are now wearing the Eyes of the Overworld." */
        if (u.uroleplay.blind) {
            /* this can only happen by putting on the Eyes of the Overworld;
               that shouldn't actually produce a permanent cure, but we
               can't let the "blind from birth" conduct remain intact */
            pline("For the first time in your life, you can see!");
            u.uroleplay.blind = FALSE;
        } else
            You("can see!");
    }
    if (changed) {
        toggle_blindness(); /* potion.c */
    }
}

void
Blindf_off(otmp)
struct obj *otmp;
{
    boolean was_blind = Blind, changed = FALSE;

    if (!otmp) {
        impossible("Blindf_off without otmp");
        return;
    }
    context.takeoff.mask &= ~W_TOOL;
    setworn((struct obj *) 0, otmp->owornmask);
    off_msg(otmp);

    if (Blind) {
        if (was_blind) {
            /* "still cannot see" makes no sense when removing lenses
               since they can't have been the cause of your blindness */
            if (otmp->otyp != LENSES || otmp->otyp != GOGGLES)
                You("still cannot see.");
        } else {
            changed = TRUE; /* !was_blind */
            /* "You were wearing the Eyes of the Overworld." */
            You_cant("see anything now!");
            /* set ball&chain variables before the hero goes blind */
            if (Punished)
                set_bc(0);
        }
    } else if (was_blind) {
        if (!gulp_blnd_check()) {
            changed = TRUE; /* !Blind */
            You("can see again.");
        }
    }
    if (changed) {
        toggle_blindness(); /* potion.c */
    }
}

/* called in moveloop()'s prologue to set side-effects of worn start-up items;
   also used by poly_obj() when a worn item gets transformed */
void
set_wear(obj)
struct obj *obj; /* if null, do all worn items; otherwise just obj itself */
{
    initial_don = !obj;

    if (!obj ? ublindf != 0 : (obj == ublindf))
        (void) Blindf_on(ublindf);
    if (!obj ? uright != 0 : (obj == uright))
        (void) Ring_on(uright);
    if (!obj ? uleft != 0 : (obj == uleft))
        (void) Ring_on(uleft);
    if (!obj ? uamul != 0 : (obj == uamul))
        (void) Amulet_on();

    if (!obj ? uarmu != 0 : (obj == uarmu))
        (void) Shirt_on();
    if (!obj ? uarm != 0 : (obj == uarm))
        (void) Armor_on();
    if (!obj ? uarmc != 0 : (obj == uarmc))
        (void) Cloak_on();
    if (!obj ? uarmf != 0 : (obj == uarmf))
        (void) Boots_on();
    if (!obj ? uarmg != 0 : (obj == uarmg))
        (void) Gloves_on();
    if (!obj ? uarmh != 0 : (obj == uarmh))
        (void) Helmet_on();
    if (!obj ? uarms != 0 : (obj == uarms))
        (void) Shield_on();

    initial_don = FALSE;
}

/* check whether the target object is currently being put on (or taken off--
   also checks for doffing--[why?]) */
boolean
donning(otmp)
struct obj *otmp;
{
    boolean result = FALSE;

    /* 'W' (or 'P' used for armor) sets afternmv */
    if (doffing(otmp))
        result = TRUE;
    else if (otmp == uarm)
        result = (afternmv == Armor_on);
    else if (otmp == uarmu)
        result = (afternmv == Shirt_on);
    else if (otmp == uarmc)
        result = (afternmv == Cloak_on);
    else if (otmp == uarmf)
        result = (afternmv == Boots_on);
    else if (otmp == uarmh)
        result = (afternmv == Helmet_on);
    else if (otmp == uarmg)
        result = (afternmv == Gloves_on);
    else if (otmp == uarms)
        result = (afternmv == Shield_on);

    return result;
}

/* check whether the target object is currently being taken off,
   so that stop_donning() and steal() can vary messages and doname()
   can vary "(being worn)" suffix */
boolean
doffing(otmp)
struct obj *otmp;
{
    long what = context.takeoff.what;
    boolean result = FALSE;

    /* 'T' (or 'R' used for armor) sets afternmv, 'A' sets takeoff.what */
    if (otmp == uarm)
        result = (afternmv == Armor_off || what == WORN_ARMOR);
    else if (otmp == uarmu)
        result = (afternmv == Shirt_off || what == WORN_SHIRT);
    else if (otmp == uarmc)
        result = (afternmv == Cloak_off || what == WORN_CLOAK);
    else if (otmp == uarmf)
        result = (afternmv == Boots_off || what == WORN_BOOTS);
    else if (otmp == uarmh)
        result = (afternmv == Helmet_off || what == WORN_HELMET);
    else if (otmp == uarmg)
        result = (afternmv == Gloves_off || what == WORN_GLOVES);
    else if (otmp == uarms)
        result = (afternmv == Shield_off || what == WORN_SHIELD);
    /* these 1-turn items don't need 'afternmv' checks */
    else if (otmp == uamul)
        result = (what == WORN_AMUL);
    else if (otmp == uleft)
        result = (what == LEFT_RING);
    else if (otmp == uright)
        result = (what == RIGHT_RING);
    else if (otmp == ublindf)
        result = (what == WORN_BLINDF);
    else if (otmp == uwep)
        result = (what == W_WEP);
    else if (otmp == uswapwep)
        result = (what == W_SWAPWEP);
    else if (otmp == uquiver)
        result = (what == W_QUIVER);

    return result;
}

/* despite their names, cancel_don() and cancel_doff() both apply to both
   donning and doffing... */
void
cancel_doff(obj, slotmask)
struct obj *obj;
long slotmask;
{
    /* Called by setworn() for old item in specified slot or by setnotworn()
     * for specified item.  We don't want to call cancel_don() if we got
     * here via <X>_off() -> setworn((struct obj *)0) -> cancel_doff()
     * because that would stop the 'A' command from continuing with next
     * selected item.  So do_takeoff() sets a flag in takeoff.mask for us.
     * [For taking off an individual item with 'T'/'R'/'w-', it doesn't
     * matter whether cancel_don() gets called here--the item has already
     * been removed by now.]
     */
    if (!(context.takeoff.mask & I_SPECIAL) && donning(obj))
        cancel_don(); /* applies to doffing too */
    context.takeoff.mask &= ~slotmask;
}

/* despite their names, cancel_don() and cancel_doff() both apply to both
   donning and doffing... */
void
cancel_don()
{
    /* the piece of armor we were donning/doffing has vanished, so stop
     * wasting time on it (and don't dereference it when donning would
     * otherwise finish)
     */
    context.takeoff.cancelled_don =
        (afternmv == Boots_on || afternmv == Helmet_on
         || afternmv == Gloves_on || afternmv == Armor_on);
    afternmv = (int NDECL((*))) 0;
    nomovemsg = (char *) 0;
    multi = 0;
    context.takeoff.delay = 0;
    context.takeoff.what = 0L;
}

/* called by steal() during theft from hero; interrupt donning/doffing */
int
stop_donning(stolenobj)
struct obj *stolenobj; /* no message if stolenobj is already being doffing */
{
    char buf[BUFSZ];
    struct obj *otmp;
    boolean putting_on;
    int result = 0;

    for (otmp = invent; otmp; otmp = otmp->nobj)
        if ((otmp->owornmask & W_ARMOR) && donning(otmp))
            break;
    /* at most one item will pass donning() test at any given time */
    if (!otmp)
        return 0;

    /* donning() returns True when doffing too; doffing() is more specific */
    putting_on = !doffing(otmp);
    /* cancel_don() looks at afternmv; it can also cancel doffing */
    cancel_don();
    /* don't want <armor>_on() or <armor>_off() being called
       by unmul() since the on or off action isn't completing */
    afternmv = (int NDECL((*))) 0;
    if (putting_on || otmp != stolenobj) {
        Sprintf(buf, "You stop %s %s.",
                putting_on ? "putting on" : "taking off",
                thesimpleoname(otmp));
    } else {
        buf[0] = '\0';   /* silently stop doffing stolenobj */
        result = -multi; /* remember this before calling unmul() */
    }
    unmul(buf);
    /* while putting on, item becomes worn immediately but side-effects are
       deferred until the delay expires; when interrupted, make it unworn
       (while taking off, item stays worn until the delay expires; when
       interrupted, leave it worn) */
    if (putting_on)
        remove_worn_item(otmp, FALSE);

    return result;
}

/* both 'clothes' and 'accessories' now include both armor and accessories;
   TOOL_CLASS is for eyewear, FOOD_CLASS is for MEAT_RING */
static NEARDATA const char clothes[] = {
    ARMOR_CLASS, RING_CLASS, AMULET_CLASS, TOOL_CLASS, FOOD_CLASS, 0
};
static NEARDATA const char accessories[] = {
    RING_CLASS, AMULET_CLASS, TOOL_CLASS, FOOD_CLASS, ARMOR_CLASS, 0
};
STATIC_VAR NEARDATA int Narmorpieces, Naccessories;

/* assign values to Narmorpieces and Naccessories */
STATIC_OVL void
count_worn_stuff(which, accessorizing)
struct obj **which; /* caller wants this when count is 1 */
boolean accessorizing;
{
    struct obj *otmp;

    Narmorpieces = Naccessories = 0;

#define MOREWORN(x,wtyp) do { if (x) { wtyp++; otmp = x; } } while (0)
    otmp = 0;
    MOREWORN(uarmh, Narmorpieces);
    MOREWORN(uarms, Narmorpieces);
    MOREWORN(uarmg, Narmorpieces);
    MOREWORN(uarmf, Narmorpieces);
    /* for cloak/suit/shirt, we only count the outermost item so that it
       can be taken off without confirmation if final count ends up as 1 */
    if (uarmc)
        MOREWORN(uarmc, Narmorpieces);
    else if (uarm)
        MOREWORN(uarm, Narmorpieces);
    else if (uarmu)
        MOREWORN(uarmu, Narmorpieces);
    if (!accessorizing)
        *which = otmp; /* default item iff Narmorpieces is 1 */

    otmp = 0;
    MOREWORN(uleft, Naccessories);
    MOREWORN(uright, Naccessories);
    MOREWORN(uamul, Naccessories);
    MOREWORN(ublindf, Naccessories);
    if (accessorizing)
        *which = otmp; /* default item iff Naccessories is 1 */
#undef MOREWORN
}

/* take off one piece or armor or one accessory;
   shared by dotakeoff('T') and doremring('R') */
STATIC_OVL int
armor_or_accessory_off(obj)
struct obj *obj;
{
    char why[QBUFSZ], what[QBUFSZ];
    why[0] = what[0] = '\0';

    if (!(obj->owornmask & (W_ARMOR | W_ACCESSORY))) {
        You("are not wearing that.");
        return 0;
    }

    if ((druid_form || vampire_form)
        && (obj->owornmask & W_ARMOR)) {
        Strcpy(why, "; it's merged");
        You_cant("take that off%s.", why);
        return 0;
    }

    if (obj == uskin
        || ((obj == uarm) && uarmc)
        || ((obj == uarmu) && (uarmc || uarm))) {
        if (obj != uskin) {
            if (uarmc)
                Strcat(what, cloak_simple_name(uarmc));
            if ((obj == uarmu) && uarm) {
                if (uarmc)
                    Strcat(what, " and ");
                Strcat(what, suit_simple_name(uarm));
            }
            Sprintf(why, " without taking off your %s first", what);
        } else {
            Strcpy(why, "; it's embedded");
        }
        You_cant("take that off%s.", why);
        return 0;
    }

    reset_remarm(); /* clear context.takeoff.mask and context.takeoff.what */
    (void) select_off(obj);
    if (!context.takeoff.mask)
        return 0;
    /* none of armoroff()/Ring_/Amulet/Blindf_off() use context.takeoff.mask */
    reset_remarm();

    if (obj->owornmask & W_ARMOR) {
        (void) armoroff(obj);
    } else if (obj == uright || obj == uleft) {
        /* Sometimes we want to give the off_msg before removing and
         * sometimes after; for instance, "you were wearing a moonstone
         * ring (on right hand)" is desired but "you were wearing a
         * square amulet (being worn)" is not because of the redundant
         * "being worn".
         */
        off_msg(obj);
        Ring_off(obj);
    } else if (obj == uamul) {
        Amulet_off();
        off_msg(obj);
    } else if (obj == ublindf) {
        Blindf_off(obj); /* does its own off_msg */
    } else {
        impossible("removing strange accessory?");
        if (obj->owornmask)
            remove_worn_item(obj, FALSE);
    }
    return 1;
}

/* the 'T' command */
int
dotakeoff()
{
    struct obj *otmp = (struct obj *) 0;

    count_worn_stuff(&otmp, FALSE);
    if (!Narmorpieces && !Naccessories) {
        if (uskin)
            pline("Your scaly armor is merged with your skin!");
        else
            pline("Not wearing any armor or accessories.");
        return 0;
    }
    if (Hidinshell) {
        You_cant("take off worn items while hiding in your shell.");
        return 0;
    }
    if (Narmorpieces != 1 || ParanoidRemove)
        otmp = getobj(clothes, "take off");
    if (!otmp)
        return 0;

    return armor_or_accessory_off(otmp);
}

/* the 'R' command */
int
doremring()
{
    struct obj *otmp = 0;

    count_worn_stuff(&otmp, TRUE);
    if (!Naccessories && !Narmorpieces) {
        pline("Not wearing any accessories or armor.");
        return 0;
    }
    if (Hidinshell) {
        You_cant("take off worn items while hiding in your shell.");
        return 0;
    }
    if (Naccessories != 1 || ParanoidRemove)
        otmp = getobj(accessories, "remove");
    if (!otmp)
        return 0;

    return armor_or_accessory_off(otmp);
}

/* Check if something worn is cursed _and_ unremovable. */
int
cursed(otmp, silent)
struct obj *otmp;
boolean silent;
{
    boolean use_plural;

    if (!otmp) {
        impossible("cursed without otmp");
        return 0;
    }
    /* Inf are immune to curses. */
    if (Role_if(PM_INFIDEL) || !otmp->cursed
        || ((otmp == uwep) && !welded(otmp))
        || ((otmp == uarmg) && uarmg->oartifact == ART_HAND_OF_VECNA))
        return 0;
    if (silent)
        return 1;

    use_plural = (is_boots(otmp) || is_gloves(otmp)
                  || otmp->otyp == LENSES || otmp->otyp == GOGGLES
                  || otmp->quan > 1L);
    /* might be trying again after applying grease to hands */
    if (Glib && otmp->bknown
        /* for weapon, we'll only get here via 'A )' */
        && (uarmg ? (otmp == uwep)
            : ((otmp->owornmask & (W_WEP | W_RING)) != 0)))
        pline("Despite your slippery %s, you can't.",
              fingers_or_gloves(TRUE));
    else
        You("can't.  %s cursed.", use_plural ? "They are" : "It is");
    set_bknown(otmp, 1);
    return 1;
}

int
armoroff(otmp)
struct obj *otmp;
{
    static char offdelaybuf[60];
    int delay = -objects[otmp->otyp].oc_delay;
    const char *what = 0;

    if (cursed(otmp, FALSE))
        return 0;
    /* this used to make assumptions about which types of armor had
       delays and which didn't; now both are handled for all types */
    if (delay) {
        nomul(delay);
        multi_reason = "disrobing";
        if (is_helmet(otmp)
            || otmp->otyp == MEAT_HELMET) {
            what = helm_simple_name(otmp);
            afternmv = Helmet_off;
        } else if (is_gloves(otmp)
                   || otmp->otyp == MEAT_GLOVES) {
            what = gloves_simple_name(otmp);
            afternmv = Gloves_off;
        } else if (is_boots(otmp)
                   || otmp->otyp == MEAT_BOOTS) {
            what = c_boots;
            afternmv = Boots_off;
        } else if (is_suit(otmp)
                   || otmp->otyp == MEAT_SUIT) {
            what = suit_simple_name(otmp);
            afternmv = Armor_off;
        } else if (is_cloak(otmp)) {
            what = cloak_simple_name(otmp);
            afternmv = Cloak_off;
        } else if (is_shield(otmp)
                   || otmp->otyp == MEAT_SHIELD) {
            /* also handles bracers */
            what = is_bracer(otmp) ? c_bracers
                                   : c_shield;
            afternmv = Shield_off;
        } else if (is_shirt(otmp)) {
            what = c_shirt;
            afternmv = Shirt_off;
        } else {
            impossible("Taking off unknown armor (%d: %d), delay %d",
                       otmp->otyp, objects[otmp->otyp].oc_armcat, delay);
        }
        if (what) {
            Sprintf(offdelaybuf, "You finish taking off your %s.", what);
            nomovemsg = offdelaybuf;
        }
    } else {
        /* Be warned!  We want off_msg after removing the item to
         * avoid "You were wearing ____ (being worn)."  However, an
         * item which grants fire resistance might cause some trouble
         * if removed in Hell and lifesaving puts it back on; in this
         * case the message will be printed at the wrong time (after
         * the messages saying you died and were lifesaved).  Luckily,
         * no cloak, shield, or fast-removable armor grants fire
         * resistance, so we can safely do the off_msg afterwards.
         * Rings do grant fire resistance, but for rings we want the
         * off_msg before removal anyway so there's no problem.  Take
         * care in adding armors granting fire resistance; this code
         * might need modification.
         * 3.2 (actually 3.1 even): that comment is obsolete since
         * fire resistance is not required for Gehennom so setworn()
         * doesn't force the resistance granting item to be re-worn
         * after being lifesaved anymore.
         */
        if (is_cloak(otmp))
            (void) Cloak_off();
        else if (is_shield(otmp) /* also handles bracers */
                 || otmp->otyp == MEAT_SHIELD)
            (void) Shield_off();
        else if (is_helmet(otmp)
                 || otmp->otyp == MEAT_HELMET)
            (void) Helmet_off();
        else if (is_gloves(otmp)
                 || otmp->otyp == MEAT_GLOVES)
            (void) Gloves_off();
        else if (is_boots(otmp)
                 || otmp->otyp == MEAT_BOOTS)
            (void) Boots_off();
        else if (is_shirt(otmp))
            (void) Shirt_off();
        else if (is_suit(otmp)
                 || otmp->otyp == MEAT_SUIT)
            (void) Armor_off();
        else
            impossible("Taking off unknown armor (%d: %d), no delay",
                       otmp->otyp, objects[otmp->otyp].oc_armcat);
        off_msg(otmp);
    }
    context.takeoff.mask = context.takeoff.what = 0L;
    return 1;
}

STATIC_OVL void
already_wearing(cc)
const char *cc;
{
    You("are already wearing %s%c", cc, (cc == c_that_) ? '!' : '.');
}

STATIC_OVL void
already_wearing2(cc1, cc2)
const char *cc1, *cc2;
{
    You_cant("wear %s because you're wearing %s there already.", cc1, cc2);
}

/*
 * canwearobj checks to see whether the player can wear a piece of armor
 *
 * inputs: otmp (the piece of armor)
 *         noisy (if TRUE give error messages, otherwise be quiet about it)
 * output: mask (otmp's armor type)
 */
int
canwearobj(otmp, mask, noisy)
struct obj *otmp;
long *mask;
boolean noisy;
{
    int err = 0;
    const char *which;

    /* this is the same check as for 'W' (dowear), but different message,
       in case we get here via 'P' (doputon) */
    if (verysmall(youmonst.data) || nohands(youmonst.data)
        || is_ent(youmonst.data) || is_plant(youmonst.data)) {
        if (noisy)
            You("can't wear any armor in your current form.");
        return 0;
    }
    if (Hidinshell) {
        if (noisy)
            You("can't wear any armor while hiding in your shell.");
        return 0;
    }

    which = is_cloak(otmp)
                ? c_cloak
                : is_shirt(otmp)
                    ? c_shirt
                    : is_suit(otmp)
                        ? c_suit
                        : 0;
    if (which && cantweararm(&youmonst)
        /* same exception for cloaks as used in m_dowear() */
        && (which != c_cloak || youmonst.data->msize != MZ_SMALL)
        && (racial_exception(&youmonst, otmp) < 1)
        && !(Race_if(PM_GIANT) && Role_if(PM_SAMURAI)
             && otmp && otmp->otyp == LARGE_SPLINT_MAIL)
        && !(Race_if(PM_GIANT) && otmp
             && otmp->otyp == CHROMATIC_DRAGON_SCALES)) {
        if (noisy)
            pline_The("%s will not fit on your body.", which);
        return 0;
    } else if (otmp->owornmask & W_ARMOR) {
        if (noisy)
            already_wearing(c_that_);
        return 0;
    }

    if (welded(uwep) && bimanual(uwep)
        && (is_suit(otmp) || is_shirt(otmp))) {
        if (noisy)
            You("cannot do that while holding your %s.",
                is_sword(uwep) ? c_sword : c_weapon);
        return 0;
    }

    if (is_helmet(otmp) || otmp->otyp == MEAT_HELMET) {
        if (uarmh) {
            if (noisy)
                already_wearing(an(helm_simple_name(uarmh)));
            err++;
        } else if (Upolyd && has_horns(youmonst.data)
                   && !is_flimsy(otmp)) {
            /* (flimsy exception matches polyself handling) */
            if (noisy) {
                if (has_antlers(youmonst.data))
                    pline_The("%s won't fit over your antlers.",
                              helm_simple_name(otmp));
                else
                    pline_The("%s won't fit over your horn%s.",
                              helm_simple_name(otmp),
                              plur(num_horns(youmonst.data)));
            }
            err++;
        } else
            *mask = W_ARMH;
    } else if (is_shield(otmp) || otmp->otyp == MEAT_SHIELD) {
        /* also handles bracers */
        if (uarms) {
            if (noisy)
                already_wearing(is_bracer(uarms) ? c_bracers
                                                 : an(c_shield));
            err++;
        } else if (uwep && !is_bracer(otmp) && bimanual(uwep)) {
            if (noisy)
                You("cannot wear a shield while wielding a two-handed %s.",
                    is_sword(uwep) ? c_sword : (uwep->otyp == BATTLE_AXE)
                                                   ? c_axe
                                                   : c_weapon);
            err++;
        } else if (u.twoweap && !is_bracer(otmp)) {
            if (noisy)
                You("cannot wear a shield while wielding two weapons.");
            err++;
        } else
            *mask = W_ARMS;
    } else if (is_boots(otmp) || otmp->otyp == MEAT_BOOTS) {
        if (uarmf) {
            if (noisy)
                already_wearing(c_boots);
            err++;
        } else if (Upolyd && slithy(youmonst.data)) {
            if (noisy)
                You("have no feet..."); /* not body_part(FOOT) */
            err++;
        } else if ((Upolyd && youmonst.data->mlet == S_CENTAUR)
                   || (Upolyd && youmonst.data == &mons[PM_DRIDER])
                   || (Upolyd && youmonst.data == &mons[PM_SATYR])
                   || (!Upolyd && Race_if(PM_CENTAUR))) {
            /* break_armor() pushes boots off for centaurs,
               so don't let dowear() put them back on... */
            if (noisy)
                Your("%s are not shaped correctly to wear %s.",
                     ((is_centaur(youmonst.data)
                       || is_satyr(youmonst.data))
                       ? "hooves" : "tarsi"),
                     c_boots); /* makeplural(body_part(FOOT)) yields
                                  "rear hooves" which sounds odd */
            err++;
        } else if (!Upolyd && Race_if(PM_TORTLE)) {
            /* Tortles foot shape is all wrong */
            if (noisy)
                Your("%s are not shaped correctly to wear %s.",
                     makeplural(body_part(FOOT)), c_boots);
            err++;
        } else if (u.utrap
                   && (u.utraptype == TT_BEARTRAP
                       || u.utraptype == TT_INFLOOR
                       || u.utraptype == TT_LAVA
                       || u.utraptype == TT_BURIEDBALL)) {
            if (u.utraptype == TT_BEARTRAP) {
                if (noisy)
                    Your("%s is trapped!", body_part(FOOT));
            } else if (u.utraptype == TT_INFLOOR
                       || u.utraptype == TT_LAVA) {
                if (noisy)
                    Your("%s are stuck in the %s!",
                         makeplural(body_part(FOOT)),
                         surface(u.ux, u.uy));
            } else { /*TT_BURIEDBALL*/
                if (noisy)
                    Your("%s is attached to the buried ball!",
                         body_part(LEG));
            }
            err++;
        } else
            *mask = W_ARMF;
    } else if (is_gloves(otmp) || otmp->otyp == MEAT_GLOVES) {
        if (uarmg) {
            if (noisy) {
                if (uarmg->otyp == MUMMIFIED_HAND)
                    pline("%s resists being covered!",
                          The(xname(uarmg)));
                else
                    already_wearing(c_gloves);
            }
            err++;
        } else if (welded(uwep)) {
            if (noisy)
                You("cannot wear gloves over your %s.",
                    is_sword(uwep) ? c_sword : c_weapon);
            err++;
        } else if (Glib) {
            /* prevent slippery bare fingers from transferring to
               gloved fingers */
            if (noisy)
                Your("%s are too slippery to pull on %s.",
                     fingers_or_gloves(FALSE), gloves_simple_name(otmp));
            err++;
        } else if (!wizard
                   && (u.ualign.record < 20 || Role_if(PM_INFIDEL))
                   && otmp->oartifact == ART_GAUNTLETS_OF_PURITY) {
            if (noisy) {
                if (Role_if(PM_INFIDEL))
                    pline_The("%s sense your wickedness, and refuse to be worn!",
                              xname(otmp));
                else
                    You("are not pure enough to wear these %s.",
                        gloves_simple_name(otmp));
            }
            err++;
        } else
            *mask = W_ARMG;
    } else if (is_shirt(otmp)) {
        if (uarm || uarmc || uarmu) {
            if (uarmu) {
                if (noisy)
                    already_wearing(an(c_shirt));
            } else {
                if (noisy)
                    You_cant("wear that over your %s.",
                             (uarm && !uarmc) ? c_armor
                                              : cloak_simple_name(uarmc));
            }
            err++;
        } else
            *mask = W_ARMU;
    } else if (is_cloak(otmp)) {
        if (uarmc) {
            if (noisy)
                already_wearing(an(cloak_simple_name(uarmc)));
            err++;
        } else
            *mask = W_ARMC;
        if (!Upolyd && Race_if(PM_GIANT) && otmp
            && otmp->otyp == CHROMATIC_DRAGON_SCALES) {
            *mask = W_ARMC;
            if (noisy)
                pline_The("scales are just large enough to fit your body.");
        }
    } else if (is_suit(otmp) || otmp->otyp == MEAT_SUIT) {
        if (uarmc) {
            if (noisy)
                You("cannot wear armor over a %s.", cloak_simple_name(uarmc));
            err++;
        } else if (uarm) {
            if (noisy)
                already_wearing("some armor");
            err++;
        } else if (youmonst.data->msize < MZ_HUGE
                   && otmp && otmp->otyp == LARGE_SPLINT_MAIL) {
            if (noisy)
                You("are too small to wear such a suit of armor.");
            err++;
        } else
            *mask = W_ARM;
        if (!Upolyd && Race_if(PM_GIANT) && Role_if(PM_SAMURAI)
            && otmp && otmp->otyp == LARGE_SPLINT_MAIL)
            *mask = W_ARM;
    } else {
        /* getobj can't do this after setting its allow_all flag; that
           happens if you have armor for slots that are covered up or
           extra armor for slots that are filled */
        if (noisy)
            silly_thing("wear", otmp);
        err++;
    }
    /* Unnecessary since now only weapons and special items like pick-axes get
     * welded to your hand, not armor
        if (welded(otmp)) {
            if (!err++) {
                if (noisy) weldmsg(otmp);
            }
        }
     */
    return !err;
}

/* Return TRUE iff wearing/wielding a potential new piece of equipment with
 * the given mask will touch the hero's skin. */
boolean
will_touch_skin(mask)
long mask;
{
    if ((mask == W_ARMC || mask == W_AMUL) && (uarm || uarmu))
        return FALSE;
    else if (mask == W_ARM && uarmu)
        return FALSE;
    else if ((mask & (W_WEP | W_SWAPWEP | W_ARMS)) && uarmg)
        return FALSE;
    else if ((mask & W_RINGL) && uarmg && uarmg->oartifact == ART_HAND_OF_VECNA)
        return FALSE;
    else if (mask == W_QUIVER)
        return FALSE;
    return TRUE;
}

STATIC_OVL int
accessory_or_armor_on(obj)
struct obj *obj;
{
    long mask = 0L;
    boolean armor, ring, eyewear;

    if (obj->owornmask & (W_ACCESSORY | W_ARMOR)) {
        already_wearing(c_that_);
        return 0;
    }
    armor = (obj->oclass == ARMOR_CLASS || is_meat_armor(obj));
    ring = (obj->oclass == RING_CLASS || obj->otyp == MEAT_RING);
    eyewear = (obj->otyp == BLINDFOLD || obj->otyp == TOWEL
               || obj->otyp == LENSES || obj->otyp == GOGGLES);
    /* checks which are performed prior to actually touching the item */
    if (armor) {
        if (!canwearobj(obj, &mask, TRUE))
            return 0;

        if (obj->otyp == HELM_OF_OPPOSITE_ALIGNMENT
            && qstart_level.dnum == u.uz.dnum) { /* in quest */
            if (u.ualignbase[A_CURRENT] == u.ualignbase[A_ORIGINAL])
                You("narrowly avoid losing all chance at your goal.");
            else /* converted */
                You("are suddenly overcome with shame and change your mind.");
            u.ublessed = 0; /* lose your god's protection */
            makeknown(obj->otyp);
            context.botl = 1; /*for AC after zeroing u.ublessed */
            return 1;
        }
    } else {
        /* accessory */
        if (ring) {
            char answer, qbuf[QBUFSZ];
            int res = 0;

            if (nolimbs(youmonst.data)) {
                You("cannot make the ring stick to your body.");
                return 0;
            }
            if (uleft && uright) {
                There("are no more %s%s to fill.",
                      humanoid(youmonst.data) ? "ring-" : "",
                      fingers_or_gloves(FALSE));
                return 0;
            }
            if (uleft) {
                mask = RIGHT_RING;
            } else if (uright) {
                mask = LEFT_RING;
            } else {
                do {
                    Sprintf(qbuf, "Which %s%s, Right or Left?",
                            humanoid(youmonst.data) ? "ring-" : "",
                            body_part(FINGER));
                    answer = yn_function(qbuf, "rl", '\0');
                    switch (answer) {
                    case '\0':
                        return 0;
                    case 'l':
                    case 'L':
                        mask = LEFT_RING;
                        break;
                    case 'r':
                    case 'R':
                        mask = RIGHT_RING;
                        break;
                    }
                } while (!mask);
            }
            if (uarmg && Glib) {
                Your(
              "%s are too slippery to remove, so you cannot put on the ring.",
                     gloves_simple_name(uarmg));
                return 1; /* always uses move */
            }
            if (uarmg && cursed(uarmg, TRUE)) {
                res = !uarmg->bknown;
                set_bknown(uarmg, 1);
                You("cannot remove your %s to put on the ring.", c_gloves);
                return res; /* uses move iff we learned gloves are cursed */
            }
            if (uwep) {
                res = !uwep->bknown; /* check this before calling welded() */
                if ((mask == RIGHT_RING || bimanual(uwep)) && welded(uwep)) {
                    const char *hand = body_part(HAND);

                    /* welded will set bknown */
                    if (bimanual(uwep))
                        hand = makeplural(hand);
                    You("cannot free your weapon %s to put on the ring.",
                        hand);
                    return res; /* uses move iff we learned weapon is cursed */
                }
            }
        } else if (obj->oclass == AMULET_CLASS) {
            if (uamul) {
                already_wearing("an amulet");
                return 0;
            }
            mask = W_AMUL;
        } else if (eyewear) {
            if (ublindf) {
                if (ublindf->otyp == TOWEL)
                    Your("%s is already covered by a towel.",
                         body_part(FACE));
                else if (ublindf->otyp == BLINDFOLD) {
                    if (obj->otyp == LENSES)
                        already_wearing2("lenses", "a blindfold");
                    else if (obj->otyp == GOGGLES)
                        already_wearing2("goggles", "a blindfold");
                    else
                        already_wearing("a blindfold");
                } else if (ublindf->otyp == LENSES) {
                    if (obj->otyp == BLINDFOLD)
                        already_wearing2("a blindfold", "some lenses");
                    else if (obj->otyp == GOGGLES)
                        already_wearing2("goggles", "some lenses");
                    else
                        already_wearing("some lenses");
                } else if (ublindf->otyp == GOGGLES) {
                    if (obj->otyp == BLINDFOLD)
                        already_wearing2("a blindfold", "some goggles");
                    else if (obj->otyp == LENSES)
                        already_wearing2("lenses", "some goggles");
                    else
                        already_wearing("some goggles");
                } else {
                    already_wearing(something); /* ??? */
                }
                return 0;
            }
        } else {
            /* neither armor nor accessory */
            You_cant("wear that!");
            return 0;
        }
    }
    if (!retouch_object(&obj, will_touch_skin(mask), FALSE))
        return 1; /* costs a turn even though it didn't get worn */
    if (armor) {
        int delay;

        /* if the armor is wielded, release it for wearing (won't be
           welded even if cursed; that only happens for weapons/weptools) */
        if (obj->owornmask & W_WEAPONS)
            remove_worn_item(obj, FALSE);
        /*
         * Setting obj->known=1 is done because setworn() causes hero's AC
         * to change so armor's +/- value is evident via the status line.
         * We used to set it here because of that, but then it would stick
         * if a nymph stole the armor before it was fully worn.  Delay it
         * until the aftermv action.  The player may still know this armor's
         * +/- amount if donning gets interrupted, but the hero won't.
         *
        obj->known = 1;
         */
        setworn(obj, mask);
        /* if there's no delay, we'll execute 'aftermv' immediately */
        if (obj == uarm)
            afternmv = Armor_on;
        else if (obj == uarmh)
            afternmv = Helmet_on;
        else if (obj == uarmg)
            afternmv = Gloves_on;
        else if (obj == uarmf)
            afternmv = Boots_on;
        else if (obj == uarms)
            afternmv = Shield_on;
        else if (obj == uarmc)
            afternmv = Cloak_on;
        else if (obj == uarmu)
            afternmv = Shirt_on;
        else
            panic("wearing armor not worn as armor? [%08lx]", obj->owornmask);

        delay = -objects[obj->otyp].oc_delay;
        if (delay) {
            nomul(delay);
            multi_reason = "dressing up";
            if (obj == uarmg && obj->oartifact == ART_HAND_OF_VECNA) {
                if (u.ualign.type == A_NONE)
                    com_pager(301);
                else
                    com_pager(302);
            } else {
                nomovemsg = "You finish your dressing maneuver.";
            }
        } else {
            unmul(""); /* call (*aftermv)(), clear it+nomovemsg+multi_reason */
            on_msg(obj);
        }
        context.takeoff.mask = context.takeoff.what = 0L;
    } else { /* not armor */
        boolean give_feedback = FALSE;

        /* [releasing wielded accessory handled in Xxx_on()] */
        if (ring) {
            setworn(obj, mask);
            Ring_on(obj);
            give_feedback = TRUE;
        } else if (obj->oclass == AMULET_CLASS) {
            setworn(obj, W_AMUL);
            Amulet_on();
            /* no feedback here if amulet of change got used up */
            give_feedback = (uamul != 0);
        } else if (eyewear) {
            /* setworn() handled by Blindf_on() */
            Blindf_on(obj);
            /* message handled by Blindf_on(); leave give_feedback False */
        }
        /* feedback for ring or for amulet other than 'change' */
        if (give_feedback && is_worn(obj))
            prinv((char *) 0, obj, 0L);
    }
    return 1;
}

/* the 'W' command */
int
dowear()
{
    struct obj *otmp;

    /* cantweararm() checks for suits of armor, not what we want here;
       verysmall() or nohands() checks for shields, gloves, etc... */
    if (verysmall(youmonst.data) || nohands(youmonst.data)
        || Hidinshell || is_ent(youmonst.data)
        || is_plant(youmonst.data)) {
        pline("Don't even bother.");
        return 0;
    }
    if (uarm && uarmu && uarmc && uarmh && uarms && uarmg && uarmf
        && uleft && uright && uamul && ublindf) {
        /* 'W' message doesn't mention accessories */
        You("are already wearing a full complement of armor.");
        return 0;
    }
    otmp = getobj(clothes, "wear");
    return otmp ? accessory_or_armor_on(otmp) : 0;
}

/* the 'P' command */
int
doputon()
{
    struct obj *otmp;

    if (Hidinshell) {
        You_cant("put on any items while hiding in your shell.");
        return 0;
    }
    if (uleft && uright && uamul && ublindf
        && uarm && uarmu && uarmc && uarmh && uarms && uarmg && uarmf) {
        /* 'P' message doesn't mention armor */
        Your("%s%s are full, and you're already wearing an amulet and %s.",
             humanoid(youmonst.data) ? "ring-" : "",
             fingers_or_gloves(FALSE),
             (ublindf->otyp == LENSES) ? "some lenses"
                                       : (ublindf->otyp == GOGGLES) ? "some goggles"
                                                                    : "a blindfold");
        return 0;
    }
    otmp = getobj(accessories, "put on");
    return otmp ? accessory_or_armor_on(otmp) : 0;
}

/* calculate current armor class */
void
find_ac()
{
    /* Base armor class for current form.
     * Giants can't wear body armor, t-shirt or cloaks,
     * but they do have thick skin. So they get a little bit
     * of love in the AC department to compensate somewhat.
     *
     * Tortles have even more armor restrictions than giants.
     * The large shell that makes up a good portion of their
     * body provides exceptional protection */
    int uac = maybe_polyd(mons[u.umonnum].ac,
                          Race_if(PM_GIANT)
                              ? 6 : Race_if(PM_TORTLE)
                                  ? 0 : mons[u.umonnum].ac);

    /* armor class from worn gear */

    int racial_bonus, dex_adjust_ac, tortle_ac;

    /* Wearing racial armor is worth +x to the armor's AC; orcs get a slightly
     * larger bonus to compensate their sub-standard equipment, lack of equipment,
     * and the stats-challenged orc itself. Taken from SporkHack.
     * Leaving Drow gloves off this list on purpose.
     */
    racial_bonus = Race_if(PM_ORC) ? 2
                     : Race_if(PM_ELF) ? 1
                       : Race_if(PM_DWARF) ? 1
                         : Race_if(PM_DROW) ? 1 : 0;

    if (uarm) {
        uac -= armor_bonus(uarm);
        if ((Race_if(PM_ORC)
             && (uarm->otyp == ORCISH_CHAIN_MAIL
                 || uarm->otyp == ORCISH_RING_MAIL))
            || (Race_if(PM_DROW)
                && (uarm->otyp == DARK_ELVEN_CHAIN_MAIL
                    || uarm->otyp == DARK_ELVEN_TUNIC))
            || (Race_if(PM_ELF) && uarm->otyp == ELVEN_CHAIN_MAIL)
            || (Race_if(PM_DWARF) && uarm->otyp == DWARVISH_CHAIN_MAIL)) {
            uac -= racial_bonus;
        }
    }

    if (uarmc) {
        uac -= armor_bonus(uarmc);
        if ((Race_if(PM_ORC) && uarmc->otyp == ORCISH_CLOAK)
            || (Race_if(PM_ELF) && uarmc->otyp == ELVEN_CLOAK)
            || (Race_if(PM_DROW) && uarmc->otyp == DARK_ELVEN_CLOAK)
            || (Race_if(PM_DWARF) && uarmc->otyp == DWARVISH_CLOAK)) {
            uac -= racial_bonus;
        }
    }

    if (uarmh) {
        uac -= armor_bonus(uarmh);
        if ((Race_if(PM_ORC) && uarmh->otyp == ORCISH_HELM)
            || (Race_if(PM_ELF) && uarmh->otyp == ELVEN_HELM)
            || (Race_if(PM_DROW) && uarmh->otyp == DARK_ELVEN_HELM)
            || (Race_if(PM_DWARF) && uarmh->otyp == DWARVISH_HELM)) {
            uac -= racial_bonus;
        }
    }

    if (uarmf) {
        uac -= armor_bonus(uarmf);
        if ((Race_if(PM_ELF) && uarmf->otyp == ELVEN_BOOTS)
            || (Race_if(PM_DROW) && uarmf->otyp == DARK_ELVEN_BOOTS)
            || (Race_if(PM_DWARF) && uarmf->otyp == DWARVISH_BOOTS)
            || (Race_if(PM_ORC) && uarmf->otyp == ORCISH_BOOTS)) {
            uac -= racial_bonus;
        }
    }

    if (uarms) {
        uac -= armor_bonus(uarms);
        if (P_SKILL(P_SHIELD) == P_BASIC)
            uac -= 1;
        else if (P_SKILL(P_SHIELD) == P_SKILLED)
            uac -= (is_bracer(uarms) ? 2 : 3);
        else if (P_SKILL(P_SHIELD) == P_EXPERT)
            uac -= (is_bracer(uarms) ? 3 : 5);
        else if (P_SKILL(P_SHIELD) == P_MASTER)
            uac -= (is_bracer(uarms) ? 4 : 8);

        if ((Race_if(PM_ORC)
             && (uarms->otyp == ORCISH_SHIELD
                 || uarms->otyp == URUK_HAI_SHIELD))
            || (Race_if(PM_ELF) && uarms->otyp == ELVEN_SHIELD)
            || (Race_if(PM_DROW) && uarms->otyp == DARK_ELVEN_BRACERS)
            || (Race_if(PM_DWARF) && uarms->otyp == DWARVISH_ROUNDSHIELD)) {
            uac -= racial_bonus;
        }
    }

    if (uarmg)
        uac -= armor_bonus(uarmg);
    if (uarmu)
        uac -= armor_bonus(uarmu);
    if (uleft && uleft->otyp == RIN_PROTECTION)
        uac -= uleft->spe;
    if (uright && uright->otyp == RIN_PROTECTION)
        uac -= uright->spe;
    if (uamul && uamul->otyp == AMULET_OF_GUARDING)
        uac -= 2; /* fixed amount; main benefit is to MC */
    if (uarms && uarms->oartifact == ART_ASHMAR) {
        /* significant fixed amount, especially as
           a dwarf */
        if (!Upolyd && Race_if(PM_DWARF))
            uac -= 7;
        else
            uac -= 5;
    }
    if (uarms && uarms->oartifact == ART_BRACERS_OF_THE_FIRST_CIRCL) {
        /* the Druid quest artifact grants extra AC on top of
           what its base material provides. Moreso if the
           wearer is actually a Druid */
        if (Role_if(PM_DRUID))
            uac -= 6;
        else
            uac -= 3;
    }
    if (uarm
        && uarm->oartifact == ART_ARMOR_OF_RETRIBUTION)
        uac -= 5;

    /* armor class from other sources */
    if (HProtection & INTRINSIC)
        uac -= u.ublessed;
    uac -= u.uspellprot;

    /* barkskin spell */
    if (Barkskin) {
        int bark_skill = (P_SKILL(spell_skilltype(SPE_BARKSKIN)) == P_EXPERT
                          ? 10 : P_SKILL(spell_skilltype(SPE_BARKSKIN)) == P_SKILLED
                               ? 8 : P_SKILL(spell_skilltype(SPE_BARKSKIN)) == P_BASIC
                                   ? 5 : 1);

        uac -= bark_skill;
    }

    /* stoneskin spell */
    if (Stoneskin) {
        int stone_skill = (P_SKILL(spell_skilltype(SPE_STONESKIN)) == P_EXPERT
                           ? 15 : P_SKILL(spell_skilltype(SPE_STONESKIN)) == P_SKILLED
                                ? 12 : P_SKILL(spell_skilltype(SPE_STONESKIN)) == P_BASIC
                                     ? 8 : 3);

        uac -= stone_skill;
    }

    /* tortle hiding in its shell */
    if (Hidinshell)
        uac -= 40;

    /* tortles gradually gain more protection
       as they 'grow up' and their shells become
       tougher and thicker */
    tortle_ac = 0;
    if (Race_if(PM_TORTLE)) {
        if (u.ulevel <= 2)
            tortle_ac -= 0;
        else if (u.ulevel <= 5)
            tortle_ac -= 1;
        else if (u.ulevel <= 8)
            tortle_ac -= 2;
        else if (u.ulevel <= 11)
            tortle_ac -= 3;
        else if (u.ulevel <= 14)
            tortle_ac -= 4;
        else if (u.ulevel <= 17)
            tortle_ac -= 5;
        else if (u.ulevel <= 20)
            tortle_ac -= 6;
        else if (u.ulevel <= 23)
            tortle_ac -= 7;
        else if (u.ulevel <= 26)
            tortle_ac -= 8;
        else if (u.ulevel <= 29)
            tortle_ac -= 9;
        else if (u.ulevel == 30)
            tortle_ac -= 10;
    }

    /* Druids get a slight AC bonus for wearing
       wooden armor */
    if (Role_if(PM_DRUID)) {
        if (uarm && is_wood(uarm))   /* body armor */
            uac -= 2;
        if (uarmg && is_wood(uarmg)) /* gauntlets */
            uac -= 2;
        if (uarmh && is_wood(uarmh)) /* helmet */
            uac -= 2;
        if (uarmf && is_wood(uarmf)) /* boots */
            uac -= 2;
        if (uarms && is_wood(uarms)) /* shield */
            uac -= 2;
    }

    /* Draugr/Vampire races receive a slight AC bonus
       for wearing bone armor */
    if (Race_if(PM_DRAUGR) || Race_if(PM_VAMPIRE)) {
        if (uarm && is_bone(uarm))   /* body armor */
            uac -= 1;
        if (uarmg && is_bone(uarmg)) /* gauntlets */
            uac -= 1;
        if (uarmh && is_bone(uarmh)) /* helmet */
            uac -= 1;
        if (uarmf && is_bone(uarmf)) /* boots */
            uac -= 1;
        if (uarms && is_bone(uarms)) /* shield */
            uac -= 1;
    }

    /* Dexterity affects your base AC */
    dex_adjust_ac = 0;
    if (ACURR(A_DEX) <= 6)
        dex_adjust_ac += 3;
    else if (ACURR(A_DEX) <= 9)
        dex_adjust_ac += 1;
    else if (ACURR(A_DEX) <= 14)
        dex_adjust_ac -= 0;
    else if (ACURR(A_DEX) <= 16)
        dex_adjust_ac -= 1;
    else if (ACURR(A_DEX) <= 18)
        dex_adjust_ac -= 2;
    else if (ACURR(A_DEX) <= 20)
        dex_adjust_ac -= 3;
    else if (ACURR(A_DEX) <= 23)
        dex_adjust_ac -= 4;
    else if (ACURR(A_DEX) >= 24)
        dex_adjust_ac -= 5;

    /* Wearing body armor made of certain rigid
     * materials negates any beneficial dexterity
     * bonus. So does being encumbered in any way.
     */
    if ((uarm && (is_heavy_metallic(uarm)
                  || is_bone(uarm) || is_stone(uarm)
                  || is_wood(uarm) || is_glass(uarm)))
        || (near_capacity() >= SLT_ENCUMBER)) {
        if (dex_adjust_ac < 0)
            dex_adjust_ac = 0;
    }

    uac = uac + dex_adjust_ac + tortle_ac;

    /* [The magic binary numbers 127 and -128 should be replaced with the
     * mystic decimal numbers 99 and -99 which require no explanation to
     * the uninitiated and would cap the width of a status line value at
     * one less character.]
     */
    if (uac < -128)
        uac = -128; /* u.uac is an schar */
    else if (uac > 127)
        uac = 127; /* for completeness */

    if (uac != u.uac) {
        u.uac = uac;
        context.botl = 1;
    }
}

void
glibr()
{
    struct obj *otmp;
    int xfl = 0;
    boolean leftfall, rightfall, wastwoweap = FALSE;
    const char *otherwep = 0, *thiswep, *which, *hand;

    leftfall = (uleft && !cursed(uleft, TRUE)
                && (!uwep || !welded(uwep) || !bimanual(uwep)));
    rightfall = (uright && !cursed(uright, TRUE) && (!welded(uwep)));
    if (!uarmg && (leftfall || rightfall) && !nolimbs(youmonst.data)) {
        /* changed so cursed rings don't fall off, GAN 10/30/86 */
        Your("%s off your %s.",
             (leftfall && rightfall) ? "rings slip" : "ring slips",
             (leftfall && rightfall) ? fingers_or_gloves(FALSE)
                                     : body_part(FINGER));
        xfl++;
        if (leftfall) {
            otmp = uleft;
            Ring_off(uleft);
            dropx(otmp);
        }
        if (rightfall) {
            otmp = uright;
            Ring_off(uright);
            dropx(otmp);
        }
    }

    otmp = uswapwep;
    if (u.twoweap && otmp) {
        /* secondary weapon doesn't need nearly as much handling as
           primary; when in two-weapon mode, we know it's one-handed
           with something else in the other hand and also that it's
           a weapon or weptool rather than something unusual, plus
           we don't need to compare its type with the primary */
        otherwep = is_sword(otmp) ? c_sword : weapon_descr(otmp);
        if (otmp->quan > 1L)
            otherwep = makeplural(otherwep);
        hand = body_part(HAND);
        which = "left ";
        Your("%s %s%s from your %s%s.", otherwep, xfl ? "also " : "",
             otense(otmp, "slip"), which, hand);
        xfl++;
        wastwoweap = TRUE;
        setuswapwep((struct obj *) 0); /* clears u.twoweap */
        if (canletgo(otmp, ""))
            dropx(otmp);
    }
    otmp = uwep;
    if (otmp && otmp->otyp != AKLYS
        && otmp->oartifact != ART_HAMMER_OF_THE_GODS
        && !welded(otmp)) {
        long savequan = otmp->quan;

        /* nice wording if both weapons are the same type.
           special case handling for Convicts and wielding
           a heavy iron ball is only necessary for the right
           hand, as you can't dual-wield a non-weapon object */
        thiswep = is_sword(otmp) ? c_sword
                                 : (Role_if(PM_CONVICT) && otmp->oclass == BALL_CLASS)
                                 ? "iron ball" : weapon_descr(otmp);
        if (otherwep && strcmp(thiswep, makesingular(otherwep)))
            otherwep = 0;
        if (otmp->quan > 1L) {
            /* most class names for unconventional wielded items
               are ok, but if wielding multiple apples or rations
               we don't want "your foods slip", so force non-corpse
               food to be singular; skipping makeplural() isn't
               enough--we need to fool otense() too */
            if (!strcmp(thiswep, "food"))
                otmp->quan = 1L;
            else
                thiswep = makeplural(thiswep);
        }
        hand = body_part(HAND);
        which = "";
        if (bimanual(otmp))
            hand = makeplural(hand);
        else if (wastwoweap)
            which = "right "; /* preceding msg was about left */
        pline("%s %s%s %s%s from your %s%s.",
              !strncmp(thiswep, "corpse", 6) ? "The" : "Your",
              otherwep ? "other " : "", thiswep, xfl ? "also " : "",
              otense(otmp, "slip"), which, hand);
        /* xfl++; */
        otmp->quan = savequan;
        setuwep((struct obj *) 0);
        if (canletgo(otmp, ""))
            dropx(otmp);
    }
}

struct obj *
some_armor(victim)
struct monst *victim;
{
    struct obj *otmph, *otmp;

    otmph = (victim == &youmonst) ? uarmc : which_armor(victim, W_ARMC);
    if (!otmph)
        otmph = (victim == &youmonst) ? uarm : which_armor(victim, W_ARM);
    if (!otmph)
        otmph = (victim == &youmonst) ? uarmu : which_armor(victim, W_ARMU);

    otmp = (victim == &youmonst) ? uarmh : which_armor(victim, W_ARMH);
    if (otmp && (!otmph || !rn2(4)))
        otmph = otmp;
    otmp = (victim == &youmonst) ? uarmg : which_armor(victim, W_ARMG);
    if (otmp && (!otmph || !rn2(4)))
        otmph = otmp;
    otmp = (victim == &youmonst) ? uarmf : which_armor(victim, W_ARMF);
    if (otmp && (!otmph || !rn2(4)))
        otmph = otmp;
    otmp = (victim == &youmonst) ? uarms : which_armor(victim, W_ARMS);
    if (otmp && (!otmph || !rn2(4)))
        otmph = otmp;
    if (victim != &youmonst && (otmp = which_armor(victim, W_BARDING))
        && (!otmph || !rn2(4)))
        otmph = otmp;
    if (victim == &youmonst && u.usteed
        && (otmp = which_armor(u.usteed, W_BARDING)) != 0
        && (!otmph || !rn2(4)))
        otmph = otmp;
    return otmph;
}

/* used for praying to check and fix levitation trouble */
struct obj *
stuck_ring(ring, otyp)
struct obj *ring;
int otyp;
{
    if (ring != uleft && ring != uright) {
        impossible("stuck_ring: neither left nor right?");
        return (struct obj *) 0;
    }

    if (ring && ring->otyp == otyp) {
        /* reasons ring can't be removed match those checked by select_off();
           limbless case has extra checks because ordinarily it's temporary */
        if (nolimbs(youmonst.data) && uamul
            && uamul->otyp == AMULET_OF_UNCHANGING && cursed(uamul, TRUE))
            return uamul;
        if (welded(uwep) && (ring == uright || bimanual(uwep)))
            return uwep;
        if (uarmg && cursed(uarmg, TRUE))
            return uarmg;
        if (cursed(ring, TRUE))
            return ring;
        /* normally outermost layer is processed first, but slippery gloves
           wears off quickly so uncurse ring itself before handling those */
        if (uarmg && Glib)
            return uarmg;
    }
    /* either no ring or not right type or nothing prevents its removal */
    return (struct obj *) 0;
}

/* also for praying; find worn item that confers "Unchanging" attribute */
struct obj *
unchanger()
{
    if (uamul && uamul->otyp == AMULET_OF_UNCHANGING)
        return uamul;
    return 0;
}

STATIC_PTR
int
select_off(otmp)
struct obj *otmp;
{
    struct obj *why;
    char buf[BUFSZ];

    if (!otmp)
        return 0;
    *buf = '\0'; /* lint suppression */

    /* special ring checks */
    if (otmp == uright || otmp == uleft) {
        struct obj glibdummy;

        if (nolimbs(youmonst.data)) {
            pline_The("ring is stuck.");
            return 0;
        }
        glibdummy = zeroobj;
        why = 0; /* the item which prevents ring removal */
        if (welded(uwep) && (otmp == uright || bimanual(uwep))) {
            Sprintf(buf, "free a weapon %s", body_part(HAND));
            why = uwep;
        } else if (uarmg && (cursed(uarmg, TRUE) || Glib)) {
            Sprintf(buf, "take off your %s%s",
                    Glib ? "slippery " : "", gloves_simple_name(uarmg));
            why = !Glib ? uarmg : &glibdummy;
        }
        if (why) {
            You("cannot %s to remove the ring.", buf);
            set_bknown(why, 1);
            return 0;
        }
    }
    /* special glove checks */
    if (otmp == uarmg) {
        if (welded(uwep)) {
            You("are unable to take off your %s while wielding that %s.",
                c_gloves, is_sword(uwep) ? c_sword : c_weapon);
            set_bknown(uwep, 1);
            return 0;
        } else if (Glib) {
            pline("%s %s are too slippery to take off.",
                  uarmg->unpaid ? "The" : "Your", /* simplified Shk_Your() */
                  gloves_simple_name(uarmg));
            return 0;
        } else if (uarmg->oartifact == ART_HAND_OF_VECNA) {
            pline("%s has merged with your left %s and cannot be removed.",
                  The(xname(uarmg)), body_part(ARM));
            return 0;
        }
    }
    /* special boot checks */
    if (otmp == uarmf) {
        if (u.utrap && u.utraptype == TT_BEARTRAP) {
            pline_The("bear trap prevents you from pulling your %s out.",
                      body_part(FOOT));
            return 0;
        } else if (u.utrap && u.utraptype == TT_INFLOOR) {
            You("are stuck in the %s, and cannot pull your %s out.",
                surface(u.ux, u.uy), makeplural(body_part(FOOT)));
            return 0;
        }
    }
    /* special suit and shirt checks */
    if (otmp == uarm || otmp == uarmu) {
        why = 0; /* the item which prevents disrobing */
        if (uarmc && cursed(uarmc, TRUE)) {
            Sprintf(buf, "remove your %s", cloak_simple_name(uarmc));
            why = uarmc;
        } else if (otmp == uarmu && uarm && cursed(uarm, TRUE)) {
            Sprintf(buf, "remove your %s", c_suit);
            why = uarm;
        } else if (welded(uwep) && bimanual(uwep)) {
            Sprintf(buf, "release your %s",
                    is_sword(uwep) ? c_sword : (uwep->otyp == BATTLE_AXE)
                                                   ? c_axe
                                                   : c_weapon);
            why = uwep;
        } else if (otmp == uarmu
                   && uarmu->oartifact == ART_STRIPED_SHIRT_OF_LIBERATIO) {
            You("cannot remove your %s.", simpleonames(uarmu));
            return 0;
        }
        if (why) {
            You("cannot %s to take off %s.", buf, the(xname(otmp)));
            set_bknown(why, 1);
            return 0;
        }
    }
    /* basic curse check */
    if (otmp == uquiver || (otmp == uswapwep && !u.twoweap)) {
        ; /* some items can be removed even when cursed */
    } else {
        /* otherwise, this is fundamental */
        if (cursed(otmp, FALSE))
            return 0;
    }

    if (otmp == uarm)
        context.takeoff.mask |= WORN_ARMOR;
    else if (otmp == uarmc)
        context.takeoff.mask |= WORN_CLOAK;
    else if (otmp == uarmf)
        context.takeoff.mask |= WORN_BOOTS;
    else if (otmp == uarmg)
        context.takeoff.mask |= WORN_GLOVES;
    else if (otmp == uarmh)
        context.takeoff.mask |= WORN_HELMET;
    else if (otmp == uarms)
        context.takeoff.mask |= WORN_SHIELD;
    else if (otmp == uarmu)
        context.takeoff.mask |= WORN_SHIRT;
    else if (otmp == uleft)
        context.takeoff.mask |= LEFT_RING;
    else if (otmp == uright)
        context.takeoff.mask |= RIGHT_RING;
    else if (otmp == uamul)
        context.takeoff.mask |= WORN_AMUL;
    else if (otmp == ublindf)
        context.takeoff.mask |= WORN_BLINDF;
    else if (otmp == uwep)
        context.takeoff.mask |= W_WEP;
    else if (otmp == uswapwep)
        context.takeoff.mask |= W_SWAPWEP;
    else if (otmp == uquiver)
        context.takeoff.mask |= W_QUIVER;

    else
        impossible("select_off: %s???", doname(otmp));

    return 0;
}

STATIC_OVL struct obj *
do_takeoff()
{
    struct obj *otmp = (struct obj *) 0;
    struct takeoff_info *doff = &context.takeoff;

    context.takeoff.mask |= I_SPECIAL; /* set flag for cancel_doff() */
    if (doff->what == W_WEP) {
        if (!cursed(uwep, FALSE)) {
            setuwep((struct obj *) 0);
            You("are empty %s.", body_part(HANDED));
            u.twoweap = FALSE;
        }
    } else if (doff->what == W_SWAPWEP) {
        setuswapwep((struct obj *) 0);
        You("no longer have a second weapon readied.");
        u.twoweap = FALSE;
    } else if (doff->what == W_QUIVER) {
        setuqwep((struct obj *) 0);
        You("no longer have ammunition readied.");
    } else if (doff->what == WORN_ARMOR) {
        otmp = uarm;
        if (!cursed(otmp, FALSE))
            (void) Armor_off();
    } else if (doff->what == WORN_CLOAK) {
        otmp = uarmc;
        if (!cursed(otmp, FALSE))
            (void) Cloak_off();
    } else if (doff->what == WORN_BOOTS) {
        otmp = uarmf;
        if (!cursed(otmp, FALSE))
            (void) Boots_off();
    } else if (doff->what == WORN_GLOVES) {
        otmp = uarmg;
        if (!cursed(otmp, FALSE))
            (void) Gloves_off();
    } else if (doff->what == WORN_HELMET) {
        otmp = uarmh;
        if (!cursed(otmp, FALSE))
            (void) Helmet_off();
    } else if (doff->what == WORN_SHIELD) {
        otmp = uarms;
        if (!cursed(otmp, FALSE))
            (void) Shield_off();
    } else if (doff->what == WORN_SHIRT) {
        otmp = uarmu;
        if (!cursed(otmp, FALSE))
            (void) Shirt_off();
    } else if (doff->what == WORN_AMUL) {
        otmp = uamul;
        if (!cursed(otmp, FALSE))
            Amulet_off();
    } else if (doff->what == LEFT_RING) {
        otmp = uleft;
        if (!cursed(otmp, FALSE))
            Ring_off(uleft);
    } else if (doff->what == RIGHT_RING) {
        otmp = uright;
        if (!cursed(otmp, FALSE))
            Ring_off(uright);
    } else if (doff->what == WORN_BLINDF) {
        if (!cursed(ublindf, FALSE))
            Blindf_off(ublindf);
    } else {
        impossible("do_takeoff: taking off %lx", doff->what);
    }
    context.takeoff.mask &= ~I_SPECIAL; /* clear cancel_doff() flag */

    return otmp;
}

/* occupation callback for 'A' */
STATIC_PTR
int
take_off(VOID_ARGS)
{
    int i;
    struct obj *otmp;
    struct takeoff_info *doff = &context.takeoff;

    if (doff->what) {
        if (doff->delay > 0) {
            doff->delay--;
            return 1; /* still busy */
        }
        if ((otmp = do_takeoff()) != 0)
            off_msg(otmp);
        doff->mask &= ~doff->what;
        doff->what = 0L;
    }

    for (i = 0; takeoff_order[i]; i++)
        if (doff->mask & takeoff_order[i]) {
            doff->what = takeoff_order[i];
            break;
        }

    otmp = (struct obj *) 0;
    doff->delay = 0;

    if (doff->what == 0L) {
        You("finish %s.", doff->disrobing);
        return 0;
    } else if (doff->what == W_WEP) {
        doff->delay = 1;
    } else if (doff->what == W_SWAPWEP) {
        doff->delay = 1;
    } else if (doff->what == W_QUIVER) {
        doff->delay = 1;
    } else if (doff->what == WORN_ARMOR) {
        otmp = uarm;
        /* If a cloak is being worn, add the time to take it off and put
         * it back on again.  Kludge alert! since that time is 0 for all
         * known cloaks, add 1 so that it actually matters...
         */
        if (uarmc)
            doff->delay += 2 * objects[uarmc->otyp].oc_delay + 1;
    } else if (doff->what == WORN_CLOAK) {
        otmp = uarmc;
    } else if (doff->what == WORN_BOOTS) {
        otmp = uarmf;
    } else if (doff->what == WORN_GLOVES) {
        otmp = uarmg;
    } else if (doff->what == WORN_HELMET) {
        otmp = uarmh;
    } else if (doff->what == WORN_SHIELD) {
        otmp = uarms;
    } else if (doff->what == WORN_SHIRT) {
        otmp = uarmu;
        /* add the time to take off and put back on armor and/or cloak */
        if (uarm)
            doff->delay += 2 * objects[uarm->otyp].oc_delay;
        if (uarmc)
            doff->delay += 2 * objects[uarmc->otyp].oc_delay + 1;
    } else if (doff->what == WORN_AMUL) {
        doff->delay = 1;
    } else if (doff->what == LEFT_RING) {
        doff->delay = 1;
    } else if (doff->what == RIGHT_RING) {
        doff->delay = 1;
    } else if (doff->what == WORN_BLINDF) {
        /* [this used to be 2, but 'R' (and 'T') only require 1 turn to
           remove a blindfold, so 'A' shouldn't have been requiring 2] */
        doff->delay = 1;
    } else {
        impossible("take_off: taking off %lx", doff->what);
        return 0; /* force done */
    }

    if (otmp)
        doff->delay += objects[otmp->otyp].oc_delay;

    /* Since setting the occupation now starts the counter next move, that
     * would always produce a delay 1 too big per item unless we subtract
     * 1 here to account for it.
     */
    if (doff->delay > 0)
        doff->delay--;

    set_occupation(take_off, doff->disrobing, 0);
    return 1; /* get busy */
}

/* clear saved context to avoid inappropriate resumption of interrupted 'A' */
void
reset_remarm()
{
    context.takeoff.what = context.takeoff.mask = 0L;
    context.takeoff.disrobing[0] = '\0';
}

/* the 'A' command -- remove multiple worn items */
int
doddoremarm()
{
    int result = 0;

    if (context.takeoff.what || context.takeoff.mask) {
        You("continue %s.", context.takeoff.disrobing);
        set_occupation(take_off, context.takeoff.disrobing, 0);
        return 0;
    } else if (!uwep && !uswapwep && !uquiver && !uamul && !ublindf && !uleft
               && !uright && !wearing_armor()) {
        You("are not wearing anything.");
        return 0;
    } else if (Hidinshell) {
        You_cant("take off worn items while hiding in your shell.");
        return 0;
    } else if (druid_form) {
        You_cant("take off worn items while in wildshape.");
        return 0;
    } else if (vampire_form) {
        You_cant("take off worn items while shapechanged.");
        return 0;
    }

    add_valid_menu_class(0); /* reset */
    if (flags.menu_style != MENU_TRADITIONAL
        || (result = ggetobj("take off", select_off, 0, FALSE,
                             (unsigned *) 0)) < -1)
        result = menu_remarm(result);

    if (context.takeoff.mask) {
        /* default activity for armor and/or accessories,
           possibly combined with weapons */
        (void) strncpy(context.takeoff.disrobing, "disrobing", CONTEXTVERBSZ);
        /* specific activity when handling weapons only */
        if (!(context.takeoff.mask & ~W_WEAPONS))
            (void) strncpy(context.takeoff.disrobing, "disarming",
                           CONTEXTVERBSZ);
        (void) take_off();
    }
    /* The time to perform the command is already completely accounted for
     * in take_off(); if we return 1, that would add an extra turn to each
     * disrobe.
     */
    return 0;
}

STATIC_OVL int
menu_remarm(retry)
int retry;
{
    int n, i = 0;
    menu_item *pick_list;
    boolean all_worn_categories = TRUE;

    if (retry) {
        all_worn_categories = (retry == -2);
    } else if (flags.menu_style == MENU_FULL) {
        all_worn_categories = FALSE;
        n = query_category("What type of things do you want to take off?",
                           invent, (WORN_TYPES | ALL_TYPES | UNPAID_TYPES
                                    | BUCX_TYPES | UNIDED_TYPES),
                           &pick_list, PICK_ANY);
        if (!n)
            return 0;
        for (i = 0; i < n; i++) {
            if (pick_list[i].item.a_int == ALL_TYPES_SELECTED)
                all_worn_categories = TRUE;
            else
                add_valid_menu_class(pick_list[i].item.a_int);
        }
        free((genericptr_t) pick_list);
    } else if (flags.menu_style == MENU_COMBINATION) {
        unsigned ggofeedback = 0;

        i = ggetobj("take off", select_off, 0, TRUE, &ggofeedback);
        if (ggofeedback & ALL_FINISHED)
            return 0;
        all_worn_categories = (i == -2);
    }
    if (menu_class_present('u')
        || menu_class_present('B') || menu_class_present('U')
        || menu_class_present('C') || menu_class_present('X'))
        all_worn_categories = FALSE;

    n = query_objlist("What do you want to take off?", &invent,
                      (SIGNAL_NOMENU | USE_INVLET | INVORDER_SORT),
                      &pick_list, PICK_ANY,
                      all_worn_categories ? is_worn : is_worn_by_type);
    if (n > 0) {
        for (i = 0; i < n; i++)
            (void) select_off(pick_list[i].item.a_obj);
        free((genericptr_t) pick_list);
    } else if (n < 0 && flags.menu_style != MENU_COMBINATION) {
        There("is nothing else you can remove or unwield.");
    }
    return 0;
}

/* hit by destroy armor scroll/black dragon breath/monster spell */
int
destroy_arm(atmp)
struct obj *atmp;
{
    struct obj *otmp;
#define DESTROY_ARM(o)                            \
    ((otmp = (o)) != 0 && (!atmp || atmp == otmp) \
     && (!obj_resists(otmp, 0, 90)))

    if (DESTROY_ARM(uarmc)) {
        otmp->in_use = TRUE;
        if (donning(otmp))
            cancel_don();
        /* for gold/chromatic DS, we don't want Cloak_off() to report
           that it stops shining _after_ we've been told that it is
           destroyed */
        if (otmp->lamplit)
            end_burn(otmp, FALSE);
        Your("%s crumbles and turns to dust!",
             cloak_simple_name(uarmc));
        (void) Cloak_off();
        useup(otmp);
    } else if (DESTROY_ARM(uarm)) {
        if (uarm && (uarm == otmp)
            && otmp->otyp == CRYSTAL_PLATE_MAIL) {
            otmp->in_use = FALSE; /* nothing happens */
            return 0;
        } else {
            otmp->in_use = TRUE;
            if (donning(otmp))
                cancel_don();
            /* for gold/chromatic dragon-scaled armor, we don't want
               Armor_gone() to report that it stops shining _after_
               we've been told that it is destroyed */
            if (otmp->lamplit)
                end_burn(otmp, FALSE);
            Your("armor turns to dust and falls to the %s!",
                 surface(u.ux, u.uy));
            /* Prevent Armor_gone() -> spoteffects() -> lava_effects()
               -> fire_damage_chain() from destroying this armor before
               useup(); matches guard used for boots */
            iflags.in_lava_effects++;
            (void) Armor_gone();
            iflags.in_lava_effects--;
            useup(otmp);
        }
    } else if (DESTROY_ARM(uarmu)) {
        otmp->in_use = TRUE;
        if (donning(otmp))
            cancel_don();
        Your("shirt crumbles into tiny threads and falls apart!");
        (void) Shirt_off();
        useup(otmp);
    } else if (DESTROY_ARM(uarmh)) {
        otmp->in_use = TRUE;
        if (donning(otmp))
            cancel_don();
        Your("%s turns to dust and is blown away!",
             helm_simple_name(uarmh));
        (void) Helmet_off();
        useup(otmp);
    } else if (DESTROY_ARM(uarmg)) {
        if (uarmg && (uarmg == otmp)
            && otmp->oartifact == ART_DRAGONBANE) {
            pline("%s %s and cannot be disintegrated.",
                  Yname2(otmp), rn2(2) ? "resists completely"
                                       : "defies physics");
            otmp->in_use = FALSE; /* nothing happens */
            return 0;
        } else if (uarmg && (uarmg == otmp)
                   && otmp->oartifact == ART_HAND_OF_VECNA) {
            /* no feedback, as we're pretending it's not actually worn */
            otmp->in_use = FALSE; /* nothing happens */
            return 0;
        } else {
            otmp->in_use = TRUE;
            if (donning(otmp))
                cancel_don();
            Your("gloves vanish!");
            (void) Gloves_off();
            useup(otmp);
            selftouch("You");
        }
    } else if (DESTROY_ARM(uarmf)) {
        otmp->in_use = TRUE;
        if (donning(otmp))
            cancel_don();
        Your("boots disintegrate!");
        /* Prevent Boots_off() -> float_down() -> lava_effects() ->
           fire_damage_chain() from destroying these boots before
           useup(); matches guard used in lava_effects() itself */
        iflags.in_lava_effects++;
        (void) Boots_off();
        iflags.in_lava_effects--;
        useup(otmp);
    } else if (DESTROY_ARM(uarms)) {
        otmp->in_use = TRUE;
        if (donning(otmp))
            cancel_don();
        /* for a shield of light, we don't want Shield_off() to report
           that it stops shining _after_ we've been told that it is
           destroyed */
        if (otmp->lamplit)
            end_burn(otmp, FALSE);
        if (is_bracer(otmp))
            Your("bracers crumble away!");
        else
            Your("shield crumbles away!");
        (void) Shield_off();
        useup(otmp);
    } else if (u.usteed && (otmp = which_armor(u.usteed, W_BARDING))
               /* don't use DESTROY_ARM for barding (at least for now) -- we
                * want it to be an invalid target if atmp == 0, so that it can
                * only be destroyed if specifically targeted */
               && (otmp == atmp)
               && !obj_resists(otmp, 0, 90) ? (otmp->in_use = TRUE) : FALSE) {
        pline("%s crumbles to pieces!", Yname2(otmp));
        m_useup(u.usteed, otmp);
    } else {
        return 0; /* could not destroy anything */
    }

#undef DESTROY_ARM
    stop_occupation();
    return 1;
}

void
adj_abon(otmp, delta)
struct obj *otmp;
schar delta;
{
    if (uarmg && uarmg == otmp && otmp->otyp == GAUNTLETS_OF_DEXTERITY) {
        if (delta) {
            makeknown(uarmg->otyp);
            ABON(A_DEX) += (delta);
        }
        context.botl = 1;
    }
    if (uarmh && uarmh == otmp && otmp->otyp == HELM_OF_BRILLIANCE) {
        if (delta) {
            makeknown(uarmh->otyp);
            ABON(A_INT) += (delta);
            ABON(A_WIS) += (delta);
        }
        context.botl = 1;
    }
    if (otmp && (otmp->oprops & ITEM_EXCEL) && (otmp->owornmask & W_ARMOR)
        && carried(otmp)) {
        if (delta) {
            int which = A_CHA,
                old_attrib = ACURR(which);
            ABON(which) += (delta);
            if (old_attrib != ACURR(which))
                otmp->oprops_known |= ITEM_EXCEL;
            set_moreluck();
        }
        context.botl = 1;
    }
}

/* decide whether a worn item is covered up by some other worn item,
   used for dipping into liquid and applying grease;
   some criteria are different than select_off()'s */
boolean
inaccessible_equipment(obj, verb, only_if_known_cursed)
struct obj *obj;
const char *verb; /* "dip" or "grease", or null to avoid messages */
boolean only_if_known_cursed; /* ignore covering unless known to be cursed */
{
    static NEARDATA const char need_to_take_off_outer_armor[] =
        "need to take off %s to %s %s.";
    char buf[BUFSZ];
    boolean anycovering = !only_if_known_cursed; /* more comprehensible... */
#define BLOCKSACCESS(x) (anycovering || (cursed((x), TRUE) && (x)->bknown))

    if (!obj || !obj->owornmask)
        return FALSE; /* not inaccessible */

    /* check for suit covered by cloak */
    if (obj == uarm && uarmc && BLOCKSACCESS(uarmc)) {
        if (verb) {
            Strcpy(buf, yname(uarmc));
            You(need_to_take_off_outer_armor, buf, verb, yname(obj));
        }
        return TRUE;
    }
    /* check for shirt covered by suit and/or cloak */
    if (obj == uarmu
        && ((uarm && BLOCKSACCESS(uarm)) || (uarmc && BLOCKSACCESS(uarmc)))) {
        if (verb) {
            char cloaktmp[QBUFSZ], suittmp[QBUFSZ];
            /* if sameprefix, use yname and xname to get "your cloak and suit"
               or "Manlobbi's cloak and suit"; otherwise, use yname and yname
               to get "your cloak and Manlobbi's suit" or vice versa */
            boolean sameprefix = (uarm && uarmc
                                  && !strcmp(shk_your(cloaktmp, uarmc),
                                             shk_your(suittmp, uarm)));

            *buf = '\0';
            if (uarmc)
                Strcat(buf, yname(uarmc));
            if (uarm && uarmc)
                Strcat(buf, " and ");
            if (uarm)
                Strcat(buf, sameprefix ? xname(uarm) : yname(uarm));
            You(need_to_take_off_outer_armor, buf, verb, yname(obj));
        }
        return TRUE;
    }
    /* check for ring covered by gloves */
    if ((obj == uleft || obj == uright) && uarmg && BLOCKSACCESS(uarmg)
        && uarmg->oartifact != ART_HAND_OF_VECNA) {
        if (verb) {
            Strcpy(buf, yname(uarmg));
            You(need_to_take_off_outer_armor, buf, verb, yname(obj));
        }
        return TRUE;
    }
    /* item is not inaccessible */
    return FALSE;
}

/* hero is putting on or taking off obj, which may do something light-related
   unifies code for cloak, shield and body armor code paths since gold dragon
   scales are worn in cloak slot and gold-scaled armor is worn in armor slot */
static void
toggle_armor_light(struct obj *armor, boolean on)
{
    boolean shadow = (Is_dragon_armor(armor)
                      && Dragon_armor_to_scales(armor) == SHADOW_DRAGON_SCALES);

    if (on) {
        if (artifact_light(armor) && !armor->lamplit) {
            begin_burn(armor, FALSE);
            if (!Blind) {
                if (shadow)
                    pline("%s %s an aura of darkness!",
                          Yname2(armor), otense(armor, "cast"));
                else
                    pline("%s %s to shine %s!",
                          Yname2(armor), otense(armor, "begin"),
                          arti_light_description(armor));
            }
        }
    } else {
        if (!artifact_light(armor)) {
            end_burn(armor, FALSE);
            if (!Blind) {
                if (shadow)
                    pline("%s its aura of darkness.",
                          Tobjnam(armor, "stop"));
                else
                    pline("%s shining.", Tobjnam(armor, "stop"));
            }
        }
    }
}

/*do_wear.c*/
