/* NetHack 5.0	do_wear.c	$NHDT-Date: 1781973047 2026/06/20 16:30:47 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.212 $ */
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
                           c_sword[] = "sword", c_axe[] = "axe",
                           c_that_[] = "that";

static NEARDATA const long takeoff_order[] = {
    WORN_BLINDF, W_WEP,      WORN_SHIELD, WORN_GLOVES, LEFT_RING,
    RIGHT_RING,  WORN_CLOAK, WORN_HELMET, WORN_AMUL,   WORN_ARMOR,
    WORN_SHIRT,  WORN_BOOTS, W_SWAPWEP,   W_QUIVER,    0L
};

staticfn void on_msg(struct obj *);
staticfn void toggle_stealth(struct obj *, long, boolean);
staticfn int Armor_on(void);
/* int Boots_on(void); -- moved to extern.h */
staticfn int Cloak_on(void);
staticfn int Helmet_on(void);
staticfn int Gloves_on(void);
staticfn int Shield_on(void);
staticfn int Shirt_on(void);
staticfn void toggle_armor_light(struct obj *, boolean);
staticfn void Amulet_on(struct obj *) NONNULLARG1;
staticfn void learnring(struct obj *, boolean);
staticfn void adjust_attrib(struct obj *, int, int);
staticfn void Ring_off_or_gone(struct obj *, boolean);
staticfn int select_off(struct obj *);
staticfn struct obj *do_takeoff(void);
staticfn int take_off(void);
staticfn int menu_remarm(int);
staticfn void count_worn_stuff(struct obj **, boolean);
staticfn int armor_or_accessory_off(struct obj *);
staticfn int accessory_or_armor_on(struct obj *);
staticfn void already_wearing(const char *);
staticfn void already_wearing2(const char *, const char *);
staticfn boolean u_body_armor_misfit(struct obj *) NONNULLARG1;
staticfn int equip_ok(struct obj *, boolean, boolean);
staticfn int puton_ok(struct obj *);
staticfn int remove_ok(struct obj *);
staticfn int wear_ok(struct obj *);
staticfn int takeoff_ok(struct obj *);
staticfn int worn_armor_ok(struct obj *);
/* maybe_destroy_armor() may return NULL */
staticfn struct obj *maybe_destroy_armor(struct obj *, struct obj *,
                                       boolean *) NONNULLARG3;
staticfn int obj_erode_type(struct obj *) NONNULLARG1;
staticfn boolean better_not_take_that_off(struct obj *) NONNULLARG1;
staticfn boolean is_wearable_shield(struct obj *) NONNULLARG1;
staticfn boolean free_shieldhand(void);
staticfn struct obj *find_altshield(void);
staticfn unsigned long lose_extrinsic(int, long);

/* plural "fingers" or optionally "gloves" */
const char *
fingers_or_gloves(boolean check_gloves)
{
    if (check_gloves && is_art(uarmg, ART_HAND_OF_VECNA))
        return "mummified hand";

    return ((check_gloves && uarmg)
            ? gloves_simple_name(uarmg) /* "gloves" or "gauntlets" */
            : makeplural(body_part(FINGER))); /* "fingers" */
}

void
off_msg(struct obj *otmp)
{
    if (flags.verbose)
        You("were wearing %s.", doname(otmp));
}

/* for items that involve no delay */
staticfn void
on_msg(struct obj *otmp)
{
    /* on_msg() for rings and amulets just shows add-to-invent feedback
       [after caller calls setworn(), for suffix: "(on {left|right} hand)"
       or "(being worn)"]; eyewear too unless giving verbose message below */
    if ((otmp->owornmask & (W_RING | W_AMUL)) != 0L
        || ((otmp->owornmask & W_TOOL) != 0L && !flags.verbose)) {
        prinv((char *) NULL, otmp, 0L);
        return;
    }

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

/* putting on or taking off an item which confers stealth;
   give feedback and discover it iff stealth state is changing;
   stealth is blocked by riding unless hero+steed fly (handled with
   BStealth by mount and dismount routines) */
staticfn
void
toggle_stealth(
    struct obj *obj,
    long oldprop, /* prop[].extrinsic, with obj->owornmask pre-stripped */
    boolean on)
{
    if (on ? gi.initial_don : svc.context.takeoff.cancelled_don)
        return;

    if (!oldprop /* extrinsic stealth from something else */
        && !HStealth /* intrinsic stealth */
        && !BStealth) { /* stealth blocked by something */
        if (obj->otyp == RIN_STEALTH)
            learnring(obj, TRUE);
        else /* discover elven cloak or elven boots */
            makeknown(obj->otyp);

        if (on) {
            if (!is_boots(obj))
                You("move very quietly.");
            else if (Levitation || Flying)
                You("float imperceptibly.");
            else
                You("walk very quietly.");
        } else if (maybe_polyd(is_giant(gy.youmonst.data),
                               Race_if(PM_GIANT))) {
            You("are just as noisy as before.");
        } else {
            boolean riding = (u.usteed != NULL);

            You("%s%s are noisy.", riding ? "and " : "sure",
                riding ? x_monnam(u.usteed, ARTICLE_YOUR, (char *) NULL,
                                  (SUPPRESS_SADDLE | SUPPRESS_BARDING
                                   | SUPPRESS_HALLUCINATION),
                                  FALSE)
                       : "");
        }
    }
}

/* putting on or taking off an item which confers displacement, or gaining
   or losing timed displacement after eating a displacer beast corpse or tin;
   give feedback and discover it iff displacement state is changing *and*
   hero is able to see self (or sense monsters); for timed, 'obj' is Null
   and this is only called for the message */
void
toggle_displacement(
    struct obj *obj,
    long oldprop, /* prop[].extrinsic, with obj->owornmask
                     stripped by caller */
    boolean on)
{
    if (on ? gi.initial_don : svc.context.takeoff.cancelled_don)
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

/* extrinsics conferred by object properties on newly worn armor */
void
oprops_on(struct obj *otmp, long mask)
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

        /* helm of brilliance and gauntlets of dexterity adjust
           their attributes in adj_abon() */
        if (otmp->otyp != HELM_OF_BRILLIANCE
            && otmp->otyp != GAUNTLETS_OF_DEXTERITY)
            ABON(which) += otmp->spe;
        if (old_attrib != ACURR(which))
            otmp->oprops_known |= ITEM_EXCEL;
        set_moreluck();
        disp.botl = TRUE;
    }
}

/* take a worn item's grant of 'prop' away from the hero; return the
   M_SEEN_foo bit that monsters watching should forget, as for an item's
   base property in setworn(), if the item was actually granting it (an
   interrupted donning never got that far) */
staticfn unsigned long
lose_extrinsic(int prop, long mask)
{
    unsigned long seenres = M_SEEN_NOTHING;

    if (u.uprops[prop].extrinsic & mask)
        seenres = cvt_prop_to_mseenres((uchar) prop);
    u.uprops[prop].extrinsic &= ~mask;
    return seenres;
}

/* undo oprops_on(); called while the item is still worn */
void
oprops_off(struct obj *otmp, long mask)
{
    long props = otmp->oprops;
    unsigned long seenres_lost = M_SEEN_NOTHING;

    if (props & ITEM_FIRE)
        seenres_lost |= lose_extrinsic(FIRE_RES, mask);
    if (props & ITEM_FROST)
        seenres_lost |= lose_extrinsic(COLD_RES, mask);
    if (props & ITEM_DRLI)
        seenres_lost |= lose_extrinsic(DRAIN_RES, mask);
    if (props & ITEM_SHOCK)
        seenres_lost |= lose_extrinsic(SHOCK_RES, mask);
    if (props & ITEM_VENOM)
        seenres_lost |= lose_extrinsic(POISON_RES, mask);
    monstunseesu(seenres_lost);
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

        /* helm of brilliance and gauntlets of dexterity adjust
           their attributes in adj_abon() */
        if (otmp->otyp != HELM_OF_BRILLIANCE
            && otmp->otyp != GAUNTLETS_OF_DEXTERITY)
            ABON(which) -= otmp->spe;
        if (old_attrib != ACURR(which))
            otmp->oprops_known |= ITEM_EXCEL;
        /* temporarily clear ITEM_EXCEL so confers_luck() won't count
           this item during set_moreluck(); owornmask is still set at
           this point; safe because set_moreluck() has no side effects
           that read oprops */
        otmp->oprops &= ~ITEM_EXCEL;
        set_moreluck();
        otmp->oprops |= ITEM_EXCEL;
        disp.botl = TRUE;
    }
}

int
Boots_on(void)
{
    long oldprop;

    if (!uarmf)
        return 0;

    oldprop = u.uprops[objects[uarmf->otyp].oc_oprop].extrinsic & ~WORN_BOOTS;

    switch (uarmf->otyp) {
    case ORCISH_BOOTS:
    case MEAT_BOOTS:
    case LOW_BOOTS:
    case DWARVISH_BOOTS:
    case HIGH_BOOTS:
    case JUMPING_BOOTS:
    case KICKING_BOOTS:
        break;
    case WATER_WALKING_BOOTS:
        /*
         * Sequencing issue?  If underwater (perhaps via magical breathing),
         * putting on water walking boots produces "you slowly rise above
         * the surface" then "you finish your dressing maneuver".
         */

        /* spoteffects() doesn't get called here; pooleffects() is called
           during movement and u.uinwater is already False after setworn() */
        if (u.uinwater)
            spoteffects(TRUE);
        /* init'd in accessory_or_armor_on() and only used here */
        if (gw.wasinwater) {
            if (!u.uinwater)
                makeknown(WATER_WALKING_BOOTS);
            gw.wasinwater = 0U;
        }
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
    case DARK_ELVEN_BOOTS:
    case ELVEN_BOOTS:
        if (maybe_polyd(is_giant(gy.youmonst.data), Race_if(PM_GIANT))) {
            pline("This %s will not silence someone %s.",
                  xname(uarmf), rn2(2) ? "as large as you"
                                       : "of your stature");
            EStealth &= ~W_ARMF;
        } else {
            toggle_stealth(uarmf, oldprop, TRUE);
        }
        break;
    case FUMBLE_BOOTS:
        if (!oldprop && !(HFumbling & ~TIMEOUT))
            incr_itimeout(&HFumbling, rnd(20));
        break;
    case LEVITATION_BOOTS:
        if (!oldprop && !HLevitation && !(BLevitation & FROMOUTSIDE)) {
            uarmf->known = 1; /* might come off if putting on over a sink,
                               * so uarmf could be Null below; status line
                               * gets updated during brief interval they're
                               * worn so hero and player learn enchantment */
            disp.botl = TRUE; /* status hilites might mark AC changed */
            makeknown(uarmf->otyp);
            float_up();
            if (Levitation)
                spoteffects_in_place(FALSE); /* for sink effect */
        } else {
            float_vs_flight(); /* maybe toggle BFlying's I_SPECIAL */
        }
        break;
    default:
        impossible(unknown_type, c_boots, uarmf->otyp);
    }
    /* uarmf could be Null here (levitation boots put on over a sink) */
    if (uarmf) {
        if (!uarmf->known) {
            uarmf->known = 1; /* boots' +/- evident from status line AC */
            update_inventory();
        }
        oprops_on(uarmf, WORN_BOOTS);
    }
    return 0;
}

int
Boots_off(void)
{
    struct obj *otmp = uarmf;
    int otyp;
    long oldprop;
    boolean was_in_use, was_levitating = !!Levitation;

    if (!otmp) {
        svc.context.takeoff.mask &= ~W_ARMF;
        return 0;
    }
    otyp = otmp->otyp;
    oldprop = u.uprops[objects[otyp].oc_oprop].extrinsic & ~WORN_BOOTS;

    oprops_off(otmp, WORN_BOOTS);

    svc.context.takeoff.mask &= ~W_ARMF;
    /* For levitation, float_down() returns if Levitation, so we
     * must do a setworn() _before_ the levitation case.
     */
    setworn((struct obj *) 0, W_ARMF);
    switch (otyp) {
    case SPEED_BOOTS:
        if (!Very_fast && !svc.context.takeoff.cancelled_don) {
            makeknown(otyp);
            You_feel("yourself slow down%s.", Fast ? " a bit" : "");
        }
        break;
    case WATER_WALKING_BOOTS:
        /* check for lava since fireproofed boots make it viable */
        if ((is_pool(u.ux, u.uy) || is_lava(u.ux, u.uy))
            && !Levitation && !Flying
            && !(is_clinger(gy.youmonst.data) && has_ceiling(&u.uz))
            && !svc.context.takeoff.cancelled_don
            /* avoid recursive call to lava_effects() */
            && !iflags.in_lava_effects) {
            /* make boots known in case you survive the drowning */
            makeknown(otyp);
            /* in_use keeps lava_effects() from burning up the boots
               while our caller still holds them; only over lava, since
               a hangup or death meanwhile uses up an in_use item */
            was_in_use = otmp->in_use;
            if (is_lava(u.ux, u.uy))
                otmp->in_use = 1;
            spoteffects(TRUE);
            otmp->in_use = was_in_use;
        } else if ((is_pool(u.ux, u.uy)
                    /* over lava, boots it can burn are lava_effects()'s
                       own, burned off a hero it is already dealing with;
                       any others were holding the hero up */
                    || (is_lava(u.ux, u.uy)
                        && (!is_flammable(otmp) || otmp->oerodeproof)))
                   && !Levitation && !Flying
                   && !(is_clinger(gy.youmonst.data) && has_ceiling(&u.uz))
                   && !svc.context.takeoff.cancelled_don
                   && iflags.in_lava_effects) {
            /* erode_obj() is destroying the boots or poly_obj() is
               changing them; sink once that caller is done */
            makeknown(otyp);
            defer_landing(LANDING_SPOT);
        }
        break;
    case DARK_ELVEN_BOOTS:
    case ELVEN_BOOTS:
        toggle_stealth(otmp, oldprop, FALSE);
        break;
    case FUMBLE_BOOTS:
        if (!oldprop && !(HFumbling & ~TIMEOUT))
            HFumbling = EFumbling = 0;
        break;
    case LEVITATION_BOOTS:
        if (!oldprop && !HLevitation && !(BLevitation & FROMOUTSIDE)
            && !svc.context.takeoff.cancelled_don) {
            /* lava_effects() sets in_lava_effects and calls Boots_off()
               so hero is already in midst of floating down */
            if (!iflags.in_lava_effects) {
                /* in_use keeps lava_effects() from burning up the boots
                   while our caller still holds them (see above) */
                was_in_use = otmp->in_use;
                if (is_lava(u.ux, u.uy))
                    otmp->in_use = 1;
                (void) float_down(0L, 0L);
                otmp->in_use = was_in_use;
            } else {
                /* erode_obj() is destroying the boots, poly_obj() is
                   changing them, or lava_effects() is burning them off a
                   hero already down in the lava; float_down() can't run
                   under any of them, but the flight it would uncover
                   must not stay blocked */
                float_vs_flight();
                disp.botl = TRUE; /* status: 'Lev' Off, maybe 'Fly' On */
                if (was_levitating && !Levitation) {
                    if (Flying) {
                        You("have stopped levitating and are now flying.");
                        check_flight_amulet();
                    } else {
                        defer_landing(LANDING_FLOAT);
                    }
                }
            }
            makeknown(otyp);
        } else {
            float_vs_flight(); /* maybe toggle (BFlying & I_SPECIAL) */
        }
        break;
    case ORCISH_BOOTS:
    case MEAT_BOOTS:
    case LOW_BOOTS:
    case DWARVISH_BOOTS:
    case HIGH_BOOTS:
    case JUMPING_BOOTS:
    case KICKING_BOOTS:
        break;
    default:
        impossible(unknown_type, c_boots, otyp);
    }
    svc.context.takeoff.cancelled_don = FALSE;
    return 0;
}

staticfn int
Cloak_on(void)
{
    int otyp;
    long oldprop;

    if (!uarmc)
        return 0;

    otyp = uarmc->otyp;
    oldprop = u.uprops[objects[uarmc->otyp].oc_oprop].extrinsic & ~WORN_CLOAK;

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
    case DARK_ELVEN_CLOAK:
    case ELVEN_CLOAK:
        toggle_stealth(uarmc, oldprop, TRUE);
        break;
    case CLOAK_OF_DISPLACEMENT:
    case SHIMMERING_DRAGON_SCALES:
        toggle_displacement(uarmc, oldprop, TRUE);
        break;
    case CELESTIAL_DRAGON_SCALES:
        /* setworn() has already set extrinsic flying */
        float_vs_flight(); /* block flying if levitating */
        check_wings(TRUE); /* form with wings that can already fly? */
        if (Flying) {
            boolean already_flying;

            /* to determine whether this flight is new we have to muck
               about in the Flying intrinsic (actually extrinsic) */
            EFlying &= ~W_ARMC;
            already_flying = !!Flying;
            EFlying |= W_ARMC;

            if (!already_flying) {
                disp.botl = TRUE; /* status: 'Fly' On */
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
        if (!uarmc->known) {
            uarmc->known = 1; /* cloak's +/- evident from status line AC */
            update_inventory();
        }
        oprops_on(uarmc, WORN_CLOAK);
    }
    toggle_armor_light(uarmc, TRUE);
    return 0;
}

int
Cloak_off(void)
{
    struct obj *otmp = uarmc;
    int otyp;
    long oldprop;
    boolean was_flying = !!Flying;
    boolean was_arti_light = otmp && otmp->lamplit && artifact_light(otmp);

    if (!otmp) {
        svc.context.takeoff.mask &= ~W_ARMC;
        return 0;
    }
    otyp = otmp->otyp;
    oldprop = u.uprops[objects[otyp].oc_oprop].extrinsic & ~WORN_CLOAK;

    if (Is_dragon_scales(otmp)
        && otyp != SHIMMERING_DRAGON_SCALES
        && otyp != CELESTIAL_DRAGON_SCALES) {
        /* most scales are handled the same in this function */
        otyp = GRAY_DRAGON_SCALES;
    }

    oprops_off(otmp, WORN_CLOAK);

    svc.context.takeoff.mask &= ~W_ARMC;
    /* For mummy wrapping, taking it off first resets `Invisible'. */
    setworn((struct obj *) 0, W_ARMC);
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
    case DARK_ELVEN_CLOAK:
    case ELVEN_CLOAK:
        toggle_stealth(otmp, oldprop, FALSE);
        break;
    case CLOAK_OF_DISPLACEMENT:
    case SHIMMERING_DRAGON_SCALES:
        toggle_displacement(otmp, oldprop, FALSE);
        break;
    case CELESTIAL_DRAGON_SCALES:
        /* the scales were already removed by the setworn() above */
        float_vs_flight(); /* probably not needed here */
        check_wings(TRUE); /* form with wings that can already fly? */
        check_flight_landing(was_flying);
        break;
    case MUMMY_WRAPPING:
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
        monstunseesu(lose_extrinsic(ACID_RES, WORN_CLOAK));
        break;
    default:
        impossible(unknown_type, c_cloak, otyp);
    }
    /* dragon scales take several turns to don, so cancel_don() can
       flag a cloak; don't let that reach the next xxx_off() */
    svc.context.takeoff.cancelled_don = FALSE;
    if (was_arti_light)
        toggle_armor_light(otmp, FALSE);
    return 0;
}

staticfn int
Helmet_on(void)
{
    long oldprop;

    if (!uarmh)
        return 0;

    oldprop = u.uprops[objects[uarmh->otyp].oc_oprop].extrinsic
              & ~WORN_HELMET;

    switch (uarmh->otyp) {
    case FEDORA:
        if (Role_if(PM_ARCHEOLOGIST))
            change_luck(1);
        break;
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
    case HELM_OF_SPEED:
        if (!oldprop && !(HFast & TIMEOUT)) {
            makeknown(uarmh->otyp);
            You_feel("yourself speed up%s.",
                     (oldprop || HFast) ? " a bit more" : "");
        }
        break;
    case HELM_OF_TELEKINESIS:
        if (racial_illithid(&gy.youmonst))
            You_feel("your telekinetic powers intensify!");
        else
            You_feel("your mind expand with telekinetic power.");
        makeknown(uarmh->otyp);
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
        disp.botl = TRUE;
        makeknown(uarmh->otyp);
        break;
    case HELM_OF_OPPOSITE_ALIGNMENT:
        if (Race_if(PM_AASIMAR)) {
            /* deity will not allow an Aasimar to change alignment;
               no alignment change occurs, helm is destroyed */
            struct obj *otmp = uarmh;
            int dmg = d(2, 12);

            SetVoice((struct monst *) 0, 0, 80, voice_deity);
            verbalize("Thou shalt not forsake Me!");
            otmp->known = 1;
            makeknown(HELM_OF_OPPOSITE_ALIGNMENT);
            pline("The %s explodes!", helm_simple_name(otmp));

            /* destroy the helm - clear worn slot, then remove */
            otmp->in_use = TRUE;
            setworn((struct obj *) 0, W_ARMH);
            useup(otmp);

            /* the act offends your deity */
            adjalign(-3);
            record_abuse_event(-3, ABUSE_ALIGNMENT_HELM);

            /* 2d12 physical damage and stun */
            losehp(Maybe_Half_Phys(dmg),
                   "an exploding helm of opposite alignment",
                   KILLED_BY);
            make_stunned((HStun & TIMEOUT) + (long) d(2, 4), TRUE);
            break;
        }
        uarmh->known = 1; /* do this here because uarmh could get cleared */
        /* changing alignment can toggle off active artifact properties,
           including levitation; uarmh could get dropped or destroyed here
           by hero falling onto a polymorph trap or into water (emergency
           disrobe) or maybe lava (probably not, helm isn't 'organic') */
        uchangealign((u.ualign.type != A_NEUTRAL)
                         ? -u.ualign.type
                         : (uarmh->o_id % 2) ? A_CHAOTIC : A_LAWFUL,
                     A_CG_HELM_ON);
        /* makeknown(HELM_OF_OPPOSITE_ALIGNMENT); -- below, after Tobjnam() */
        FALLTHROUGH;
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
            else if (Role_if(PM_CLERIC))
                set_bknown(uarmh, 1); /* (bknown should already be set) */
            else if (uarmh->bknown)
                update_inventory(); /* keep bknown as-is; display the curse */
        }
        disp.botl = TRUE; /* reveal new alignment or INT & WIS */
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
        if (!uarmh->known) {
            uarmh->known = 1; /* helmet's +/- evident from status line AC */
            update_inventory();
        }
        oprops_on(uarmh, WORN_HELMET);
    }
    return 0;
}

int
Helmet_off(void)
{
    if (!uarmh) {
        svc.context.takeoff.mask &= ~W_ARMH;
        return 0;
    }
    oprops_off(uarmh, WORN_HELMET);

    svc.context.takeoff.mask &= ~W_ARMH;

    switch (uarmh->otyp) {
    case TOQUE:
    case DARK_ELVEN_HELM:
        break;
    case HELM_OF_SPEED:
        setworn((struct obj *) 0, W_ARMH);
        if (!Very_fast && !svc.context.takeoff.cancelled_don)
            You_feel("yourself slow down%s.", Fast ? " a bit" : "");
        break;
    case HELM_OF_TELEKINESIS:
        if (!svc.context.takeoff.cancelled_don) {
            if (racial_illithid(&gy.youmonst))
                You_feel("your telekinetic powers diminish.");
            else
                You_feel("the telekinetic power recede from your mind.");
        }
        break;
    case FEDORA:
        if (Role_if(PM_ARCHEOLOGIST))
            change_luck(-1);
        break;
    case HELMET:
    case DENTED_POT:
    case ELVEN_HELM:
    case DWARVISH_HELM:
    case ORCISH_HELM:
    case MEAT_HELMET:
        break;
    case DUNCE_CAP:
        disp.botl = TRUE;
        break;
    case CORNUTHAUM:
        if (!svc.context.takeoff.cancelled_don) {
            ABON(A_CHA) += (Role_if(PM_WIZARD) ? -1 : 1);
            disp.botl = TRUE;
        }
        break;
    case HELM_OF_TELEPATHY:
    case HELM_OF_CAUTION:
        /* need to update ability before calling see_monsters() */
        setworn((struct obj *) 0, W_ARMH);
        see_monsters();
        return 0;
    case HELM_OF_BRILLIANCE:
        if (!svc.context.takeoff.cancelled_don)
            adj_abon(uarmh, -uarmh->spe);
        break;
    case HELM_OF_OPPOSITE_ALIGNMENT:
        /* changing alignment can toggle off active artifact
           properties, including levitation; uarmh could get
           dropped or destroyed here */
        uchangealign(u.ualignbase[A_CURRENT], A_CG_HELM_OFF);
        break;
    default:
        impossible(unknown_type, c_helmet, uarmh->otyp);
    }
    setworn((struct obj *) 0, W_ARMH);
    svc.context.takeoff.cancelled_don = FALSE;
    return 0;
}

/* hard helms provide better protection against falling rocks; any rigid
   material counts, so a wooden, bone or stone helm is as hard as a metal
   or glass one (and prints as a "helm" rather than a "hat") */
boolean
hard_helmet(struct obj *obj)
{
    if (!obj || !is_helmet(obj))
        return FALSE;
    return is_hard(obj) ? TRUE : FALSE;
}

staticfn int
Gloves_on(void)
{
    long oldprop;

    if (!uarmg)
        return 0;

    oldprop = u.uprops[objects[uarmg->otyp].oc_oprop].extrinsic
              & ~WORN_GLOVES;

    switch (uarmg->otyp) {
    case DARK_ELVEN_GLOVES:
    case GAUNTLETS:
    case GLOVES:
    case MEAT_GLOVES:
        break;
    case GAUNTLETS_OF_PROTECTION:
        makeknown(uarmg->otyp);
        break;
    case MUMMIFIED_HAND:
        makeknown(uarmg->otyp);
        disp.botl = TRUE;
        break;
    case GAUNTLETS_OF_FUMBLING:
        if (!oldprop && !(HFumbling & ~TIMEOUT))
            incr_itimeout(&HFumbling, rnd(20));
        break;
    case GAUNTLETS_OF_POWER:
        /* the Gauntlets of Purity welcome a pious wearer */
        if (u.ualign.record >= 20
            && is_art(uarmg, ART_GAUNTLETS_OF_PURITY))
            pline_The("%s sense your piety, and slide comfortably over"
                      " your %s.", artiname(uarmg->oartifact),
                      makeplural(body_part(HAND)));
        makeknown(uarmg->otyp);
        disp.botl = TRUE; /* taken care of in attrib.c */
        break;
    case GAUNTLETS_OF_DEXTERITY:
        adj_abon(uarmg, uarmg->spe);
        break;
    default:
        impossible(unknown_type, c_gloves, uarmg->otyp);
    }
    if (!uarmg->known) {
        uarmg->known = 1; /* gloves' +/- evident because of status line AC */
        update_inventory();
    }
    oprops_on(uarmg, WORN_GLOVES);
    return 0;
}

/* check for wielding cockatrice corpse after taking off gloves or yellow
   dragon scales/mail or having temporary stoning resistance time out */
void
wielding_corpse(
    struct obj *obj,   /* uwep, potentially a wielded cockatrice corpse */
    struct obj *how,   /* gloves or dragon armor or Null (resist timeout) */
    boolean voluntary) /* True: taking protective armor off on purpose */
{
    if (!obj || obj->otyp != CORPSE || uarmg)
        return;
    /* note: can't dual-wield with non-weapons/weapon-tools so u.twoweap
       will always be false if uswapwep happens to be a corpse */
    if (obj != uwep && (obj != uswapwep || !u.twoweap))
        return;

    if (touch_petrifies(&mons[obj->corpsenm]) && !Stone_resistance) {
        char kbuf[BUFSZ], hbuf[BUFSZ];

        You("%s %s in your bare %s.",
            (how && is_gloves(how)) ? "now wield" : "are wielding",
            corpse_xname(obj, (const char *) 0, CXN_ARTICLE),
            makeplural(body_part(HAND)));
        /* "removing" ought to be "taking off" but that makes the
           tombstone text more likely to be truncated */
        if (how && Is_dragon_scaled_armor(how))
            /* simpleonames() leaves out the scales; name them as
               xname() does, "yellow-scaled ring mail" */
            Sprintf(hbuf, "%s %s-scaled %s",
                    voluntary ? "removing" : "losing",
                    dragon_scales_color(how), simpleonames(how));
        else if (how)
            Sprintf(hbuf, "%s %s", voluntary ? "removing" : "losing",
                    is_gloves(how) ? gloves_simple_name(how)
                    : is_bracer(how) ? shield_simple_name(how)
                    : strsubst(simpleonames(how), "set of ", ""));
        else
            Strcpy(hbuf, "resistance timing out");
        Snprintf(kbuf, sizeof kbuf, "%s while wielding %s",
                 hbuf, killer_xname(obj));
        instapetrify(kbuf);
        /* life-saved or got poly'd into a stone golem; can't continue
           wielding cockatrice corpse unless have now become resistant */
        if (!Stone_resistance)
            remove_worn_item(obj, FALSE);
    }
}

int
Gloves_off(void)
{
    struct obj *gloves = uarmg; /* needed after uarmg has been set to Null */
    long oldprop;
    boolean on_purpose, swapping;

    if (!gloves) {
        svc.context.takeoff.mask &= ~W_ARMG;
        return 0;
    }
    oldprop = u.uprops[objects[uarmg->otyp].oc_oprop].extrinsic
              & ~WORN_GLOVES;
    on_purpose = !svc.context.mon_moving && !uarmg->in_use;

    oprops_off(uarmg, WORN_GLOVES);

    svc.context.takeoff.mask &= ~W_ARMG;

    switch (uarmg->otyp) {
    case DARK_ELVEN_GLOVES:
    case GAUNTLETS:
    case GLOVES:
    case MEAT_GLOVES:
        break;
    case GAUNTLETS_OF_PROTECTION:
    case MUMMIFIED_HAND:
        break;
    case GAUNTLETS_OF_FUMBLING:
        if (!oldprop && !(HFumbling & ~TIMEOUT))
            HFumbling = EFumbling = 0;
        break;
    case GAUNTLETS_OF_POWER:
        makeknown(uarmg->otyp);
        disp.botl = TRUE; /* taken care of in attrib.c */
        break;
    case GAUNTLETS_OF_DEXTERITY:
        if (!svc.context.takeoff.cancelled_don)
            adj_abon(uarmg, -uarmg->spe);
        break;
    default:
        impossible(unknown_type, c_gloves, uarmg->otyp);
    }
    setworn((struct obj *) 0, W_ARMG);
    svc.context.takeoff.cancelled_don = FALSE;
    encumber_msg(); /* immediate feedback for GoP */

    /* usually can't remove gloves when they're slippery but it can
       be done by having them fall off (polymorph), stolen, or
       destroyed (scroll, overenchantment, monster spell); if that
       happens, 'cure' slippery fingers so that it doesn't transfer
       from gloves to bare hands */
    if (Glib)
        make_glib(0); /* for update_inventory() */

    /* prevent wielding cockatrice when not wearing gloves; poly_obj()
       puts the new form of these gloves straight back on, so the hand
       is never bare during that swap */
    swapping = (gw.worn_swap_quiet && gloves == gw.worn_slot_exempt);
    if (uwep && uwep->otyp == CORPSE && !swapping)
        wielding_corpse(uwep, gloves, on_purpose);
    /* KMH -- ...or your secondary weapon when you're wielding it
       [This case can't actually happen; twoweapon mode won't engage
       if a corpse has been set up as either the primary or alternate
       weapon.  If it could happen and /both/ uwep and uswapwep could
       be cockatrice corpses, life-saving for the first would need to
       prevent the second from being fatal since conceptually they'd
       be being touched simultaneously.] */
    if (u.twoweap && uswapwep && uswapwep->otyp == CORPSE && !swapping)
        wielding_corpse(uswapwep, gloves, on_purpose);

    if (condtests[bl_bareh].enabled)
        disp.botl = TRUE;

    return 0;
}

staticfn int
Shield_on(void)
{
    if (!uarms)
        return 0;

    /* no shield currently requires special handling when put on, but we
       keep this uncommented in case somebody adds a new one which does
       [the magical shields are handled by setting u.uprops[*].extrinsic
       in setworn() called by armor_or_accessory_on() before Shield_on()] */
    switch (uarms->otyp) {
    case SMALL_SHIELD:
    case SHIELD_OF_LIGHT:
    case SHIELD_OF_MOBILITY:
    case MEAT_SHIELD:
    case BRACERS:
    case DARK_ELVEN_BRACERS:
    case RUNED_BRACERS:
    case SHIELD_OF_DRAIN_RESISTANCE:
    case SHIELD_OF_SHOCK_RESISTANCE:
    case ELVEN_SHIELD:
    case URUK_HAI_SHIELD:
    case ORCISH_SHIELD:
    case DWARVISH_ROUNDSHIELD:
    case LARGE_SHIELD:
    case SHIELD_OF_REFLECTION:
        break;
    default:
        impossible(unknown_type, c_shield, uarms->otyp);
    }
    if (!uarms->known) {
        uarms->known = 1; /* shield's +/- evident because of status line AC */
        update_inventory();
    }
    oprops_on(uarms, WORN_SHIELD);
    toggle_armor_light(uarms, TRUE);
    return 0;
}

int
Shield_off(void)
{
    struct obj *otmp = uarms;
    boolean was_arti_light = otmp && otmp->lamplit && artifact_light(otmp);
    boolean on_purpose;

    if (!otmp) {
        svc.context.takeoff.mask &= ~W_ARMS;
        return 0;
    }
    /* same determination as Gloves_off() */
    on_purpose = !svc.context.mon_moving && !otmp->in_use;
    oprops_off(uarms, WORN_SHIELD);

    svc.context.takeoff.mask &= ~W_ARMS;

    /* no shield currently requires special handling when taken off, but we
       keep this uncommented in case somebody adds a new one which does */
    switch (uarms->otyp) {
    case SMALL_SHIELD:
    case SHIELD_OF_LIGHT:
    case SHIELD_OF_MOBILITY:
    case MEAT_SHIELD:
    case BRACERS:
    case DARK_ELVEN_BRACERS:
    case RUNED_BRACERS:
    case SHIELD_OF_DRAIN_RESISTANCE:
    case SHIELD_OF_SHOCK_RESISTANCE:
    case ELVEN_SHIELD:
    case URUK_HAI_SHIELD:
    case ORCISH_SHIELD:
    case DWARVISH_ROUNDSHIELD:
    case LARGE_SHIELD:
    case SHIELD_OF_REFLECTION:
        break;
    default:
        impossible(unknown_type, c_shield, uarms->otyp);
    }

    setworn((struct obj *) 0, W_ARMS);
    svc.context.takeoff.cancelled_don = FALSE;
    if (was_arti_light)
        toggle_armor_light(otmp, FALSE);
    /* the Bracers of the First Circle's stoning resistance went with
       them, as for yellow dragon scales; the uswapwep case is always
       a no-op */
    if (otmp->oartifact == ART_BRACERS_OF_THE_FIRST_CIRCLE) {
        wielding_corpse(uwep, otmp, on_purpose);
        wielding_corpse(uswapwep, otmp, on_purpose);
    }
    return 0;
}

staticfn int
Shirt_on(void)
{
    if (!uarmu)
        return 0;

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
    if (!uarmu->known) {
        uarmu->known = 1; /* shirt's +/- evident because of status line AC */
        update_inventory();
    }
    oprops_on(uarmu, WORN_SHIRT);
    return 0;
}

int
Shirt_off(void)
{
    if (!uarmu) {
        svc.context.takeoff.mask &= ~W_ARMU;
        return 0;
    }
    oprops_off(uarmu, WORN_SHIRT);

    svc.context.takeoff.mask &= ~W_ARMU;

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
    svc.context.takeoff.cancelled_don = FALSE;
    return 0;
}

/* handle extra abilities for hero wearing dragon-scaled body armor;
   the scales' primary property is conferred by setworn() through
   armor_provides_extrinsic(), this adds their secondary effects (bare
   scales worn in the cloak slot confer only their primary, see
   Cloak_on()).  The off arms only clear bits: the callers have already
   unworn the armor and own the flight-landing check */
void
dragon_armor_handling(
    struct obj *otmp,   /* armor being put on or taken off */
    boolean puton,      /* True: on, False: off */
    boolean on_purpose) /* voluntary removal; not applicable for putting on */
{
    unsigned long seenres_lost = M_SEEN_NOTHING;

    if (!otmp)
        return;

    switch (Dragon_armor_to_scales(otmp)) {
        /* gray, red, black: no extra effect */
    case GREEN_DRAGON_SCALES:
        if (puton) {
            ESick_resistance |= W_ARM;
            if (Sick)
                make_sick(0L, (char *) 0, TRUE, SICK_ALL);
        } else {
            ESick_resistance &= ~W_ARM;
        }
        break;
    case GOLD_DRAGON_SCALES:
        /* light is handled by toggle_armor_light() */
        if (puton) {
            EInfravision |= W_ARM;
            EClairvoyant |= W_ARM;
        } else {
            EInfravision &= ~W_ARM;
            EClairvoyant &= ~W_ARM;
        }
        see_monsters();
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
            seenres_lost = lose_extrinsic(COLD_RES, W_ARM);
        }
        break;
    case BLUE_DRAGON_SCALES:
        if (puton) {
            if (!Very_fast)
                You("speed up%s.", Fast ? " a bit more" : "");
            EFast |= W_ARM;
        } else {
            EFast &= ~W_ARM;
            if (!Very_fast && !svc.context.takeoff.cancelled_don)
                You("slow down.");
        }
        break;
    case YELLOW_DRAGON_SCALES:
        if (puton) {
            EStone_resistance |= W_ARM;
            if (Stone_resistance && Stoned)
                make_stoned(0L, "You no longer seem to be petrifying.", 0,
                            (char *) 0);
        } else {
            EStone_resistance &= ~W_ARM;

            /* prevent wielding cockatrice after losing stoning resistance
               when not wearing gloves; the uswapwep case is always a no-op */
            wielding_corpse(uwep, otmp, on_purpose);
            wielding_corpse(uswapwep, otmp, on_purpose);
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
            toggle_displacement(otmp, (EDisplaced & ~W_ARM), TRUE);
            if (Stunned)
                make_stunned(0L, TRUE);
            EStun_resistance |= W_ARM;
        } else {
            toggle_displacement(otmp, (EDisplaced & ~W_ARM), FALSE);
            seenres_lost = lose_extrinsic(STUN_RES, W_ARM);
        }
        break;
    case SEA_DRAGON_SCALES:
        if (puton) {
            /* magical breathing ends a suffocating engulfer's countdown,
               never the choking of an amulet of strangulation that can
               strangle this form */
            if (Strangled
                && !(uamul && uamul->otyp == AMULET_OF_STRANGULATION
                     && can_be_strangled(&gy.youmonst))) {
                Strangled = 0L;
                disp.botl = TRUE;
                You("can suddenly breathe again!");
            }
            ESwimming |= W_ARM;
        } else {
            ESwimming &= ~W_ARM;
            /* the armor's magical breathing went with it */
            if (Underwater && !Breathless && !Amphibious && !Swimming) {
                You("suddenly inhale an unhealthy amount of %s!",
                    hliquid("water"));
                /* erode_obj() destroying the suit, poly_obj() changing
                   it or enchant armor swapping its scales is still
                   holding it, and drown() would soak (and could destroy)
                   what that caller holds; drown once that caller is
                   done */
                if (iflags.in_lava_effects)
                    defer_landing(LANDING_SPOT);
                else
                    (void) drown();
            }
        }
        break;
    case CELESTIAL_DRAGON_SCALES:
        if (puton) {
            ESleep_resistance |= W_ARM;
            EShock_resistance |= W_ARM;

            /* setworn() has already set extrinsic flying */
            float_vs_flight(); /* block flying if levitating */
            check_wings(TRUE); /* form with wings that can already fly? */

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
                    disp.botl = TRUE; /* status: 'Fly' On */
                    You("are now in flight.");
                }
            }
        } else {
            seenres_lost = lose_extrinsic(SLEEP_RES, W_ARM)
                           | lose_extrinsic(SHOCK_RES, W_ARM);
        }
        break;
    case SHADOW_DRAGON_SCALES:
        /* ultravision is the primary property, darkness is handled by
           toggle_armor_light() */
        if (puton) {
            ESleep_resistance |= W_ARM;
            EDrain_resistance |= W_ARM;
        } else {
            seenres_lost = lose_extrinsic(SLEEP_RES, W_ARM)
                           | lose_extrinsic(DRAIN_RES, W_ARM);
        }
        break;
    case CHROMATIC_DRAGON_SCALES:
        /* magic resistance is the primary property, light is handled by
           toggle_armor_light() */
        if (puton) {
            EPoison_resistance |= W_ARM;
            EFire_resistance |= W_ARM;
            ECold_resistance |= W_ARM;
            ESleep_resistance |= W_ARM;
            EDisint_resistance |= W_ARM;
            EShock_resistance |= W_ARM;
            EAcid_resistance |= W_ARM;
            EStone_resistance |= W_ARM;
            EReflecting |= W_ARM;
            /* the scales' reflection is acquired like any worn
               source's and breaks the reflection conduct */
            u.uconduct.reflection++;
            if (Stone_resistance && Stoned)
                make_stoned(0L, "You no longer seem to be petrifying.", 0,
                            (char *) 0);
        } else {
            seenres_lost = lose_extrinsic(POISON_RES, W_ARM)
                           | lose_extrinsic(FIRE_RES, W_ARM)
                           | lose_extrinsic(COLD_RES, W_ARM)
                           | lose_extrinsic(SLEEP_RES, W_ARM)
                           | lose_extrinsic(DISINT_RES, W_ARM)
                           | lose_extrinsic(SHOCK_RES, W_ARM)
                           | lose_extrinsic(ACID_RES, W_ARM)
                           | lose_extrinsic(REFLECTING, W_ARM);
            EStone_resistance &= ~W_ARM;

            /* as for yellow scales above */
            wielding_corpse(uwep, otmp, on_purpose);
            wielding_corpse(uswapwep, otmp, on_purpose);
        }
        break;
    default:
        break;
    }
    monstunseesu(seenres_lost);
}

/* the caller has just ended a source of the hero's flight (unworn
   celestial dragon scales or an amulet of flying, wings folded under a
   suit); land unless something else keeps the hero aloft */
void
check_flight_landing(boolean was_flying)
{
    if (was_flying && !Flying) {
        disp.botl = TRUE; /* status: 'Fly' Off */
        /* inside an engulfer there is no ground to reach; expels() lands
           the hero on release */
        if (u.uswallow)
            return;
        /* a flyer already under the water (a dive, or flight gained down
           there) is already down; landing again would rerun drown() on
           the bottom */
        if (u.uinwater)
            return;
        /* erode_obj() is destroying the item or poly_obj() changing it,
           and spoteffects() can't run under their guard; land once that
           caller is done */
        if (iflags.in_lava_effects) {
            defer_landing(LANDING_FLY);
            return;
        }
        /* a levitator comes through here when a hold (grass, a buried
           iron ball's chain) keeps the levitation down */
        You("%s.", (is_pool_or_lava(u.ux, u.uy) || is_open_air(u.ux, u.uy)
                    || Is_waterlevel(&u.uz) || Is_airlevel(&u.uz))
                      ? ((HLevitation || ELevitation) ? "stop levitating"
                                                      : "stop flying")
                      : "land");
        /* coming down where the hero was: a trap here met them on
           arrival, up in the air */
        spoteffects_in_place(TRUE);
    }
}

/* Does the hero have big wings, the kind most body armor confines? An
   aasimar's angelic wings (experience level 15 with unblemished
   alignment) belong to the aasimar body; any other wings are the form's
   own, or the race's when not polymorphed */
boolean
u_big_wings(void)
{
    if (Race_if(PM_AASIMAR) && !Upolyd)
        return (HFlying & FROMRACE) != 0L;
    return big_wings(raceptr(&gy.youmonst));
}

/* Some monster forms' flight is blocked by most body armor */
void
check_wings(boolean silent) /* we assume a wardrobe change if false */
{
    static struct obj *last_worn_armor;
    boolean old_flying = Flying;

    BFlying &= ~W_ARM;
    if (!u_big_wings())
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
        disp.botl = TRUE;
}

/* hero is putting on or taking off obj, which may do something light-related
   unifies code for cloak, shield and body armor code paths since gold dragon
   scales are worn in cloak slot and gold-scaled armor is worn in armor slot */
staticfn void
toggle_armor_light(struct obj *armor, boolean on)
{
    boolean shadow = (Is_dragon_armor(armor)
                      && Dragon_armor_to_scales(armor) == SHADOW_DRAGON_SCALES);

    if (on) {
        if (artifact_light(armor) && !armor->lamplit) {
            unsigned oid = armor->o_id;

            begin_burn(armor, FALSE);
            /* the light can hurt a light-hating hero back into a natural
               form whose break_armor() destroys this armor, or can't
               touch it and takes it off */
            if (o_on(oid, gi.invent) != armor || !armor->lamplit)
                return;
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

staticfn int
Armor_on(void)
{
    boolean was_flying = !!Flying;

    if (!uarm) /* no known instances of !uarm here but play it safe */
        return 0;

    /* nothing said when poly_obj() swaps a worn suit's form in place */
    if (Role_if(PM_MONK)
        && !(gw.worn_swap_quiet && gw.worn_slot_exempt
             && gw.worn_slot_exempt != uarm))
        You_feel("extremely uncomfortable wearing such armor.");

    if (!uarm->known) {
        uarm->known = 1; /* suit's +/- evident because of status line AC */
        update_inventory();
    }
    check_wings(FALSE);
    oprops_on(uarm, W_ARM);
    dragon_armor_handling(uarm, TRUE, TRUE);
    /* a hard suit folds big wings unless celestial scales keep the hero
       up; land before the suit can light up, since the light can revert
       a light-hating hero and that revert has a landing of its own */
    check_flight_landing(was_flying);
    /* landing in lava can burn the suit away */
    if (uarm)
        toggle_armor_light(uarm, TRUE);
    return 0;
}

int
Armor_off(void)
{
    struct obj *otmp = uarm;
    boolean was_flying = !!Flying;
    boolean was_arti_light = otmp && otmp->lamplit && artifact_light(otmp);

    /* nothing said when poly_obj() swaps a worn suit's form in place */
    if (Role_if(PM_MONK)
        && !(gw.worn_swap_quiet && otmp && otmp == gw.worn_slot_exempt))
        You_feel("much more comfortable and free now.");

    if (otmp)
        oprops_off(otmp, W_ARM);

    svc.context.takeoff.mask &= ~W_ARM;
    setworn((struct obj *) 0, W_ARM);
    svc.context.takeoff.cancelled_don = FALSE;

    check_wings(FALSE);

    /* taking off yellow dragon scales/mail might be fatal; arti_light
       comes from gold dragon scales/mail so they don't overlap, but
       conceptually the non-fatal change should be done before the
       potentially fatal change in case the latter results in bones */
    if (was_arti_light)
        toggle_armor_light(otmp, FALSE);
    dragon_armor_handling(otmp, FALSE, TRUE);
    check_flight_landing(was_flying);

    return 0;
}

/* The gone functions differ from the off functions in that if you die from
 * taking it off and have life saving, you still die.  [Obsolete reference
 * to lack of fire resistance being fatal in hell (nethack 3.0) and life
 * saving putting a removed item back on to prevent that from immediately
 * repeating.]
 */
int
Armor_gone(void)
{
    struct obj *otmp = uarm;
    boolean was_flying = !!Flying;
    boolean was_arti_light = otmp && otmp->lamplit && artifact_light(otmp);

    if (Role_if(PM_MONK))
        You_feel("much more comfortable and free now.");

    if (otmp)
        oprops_off(otmp, W_ARM);

    svc.context.takeoff.mask &= ~W_ARM;
    setnotworn(uarm);
    svc.context.takeoff.cancelled_don = FALSE;

    check_wings(FALSE);

    /* losing yellow dragon scales/mail might be fatal; arti_light
       comes from gold dragon scales/mail so they don't overlap, but
       conceptually the non-fatal change should be done before the
       potentially fatal change in case the latter results in bones */
    if (was_arti_light && !artifact_light(otmp)) {
        end_burn(otmp, FALSE);
        if (!Blind)
            pline("%s shining.", Tobjnam(otmp, "stop"));
    }
    dragon_armor_handling(otmp, FALSE, FALSE);
    check_flight_landing(was_flying);

    return 0;
}

staticfn void
Amulet_on(struct obj *amul)
{
    /* already in the amulet slot when starting gear takes effect or a
       worn amulet was polymorphed in place: it isn't put on again and
       gets no inventory line here */
    boolean already_worn = (amul->owornmask & W_AMUL) != 0L,
            on_msg_done = already_worn;

    if (!already_worn) {
        /* make sure amulet isn't wielded/alt-wielded/quivered, before
           wearing */
        remove_worn_item(amul, FALSE);
        setworn(amul, W_AMUL);
    }

    switch (uamul->otyp) {
    case AMULET_OF_ESP:
    case AMULET_OF_LIFE_SAVING:
    case AMULET_VERSUS_POISON:
    case AMULET_OF_MAGIC_RESISTANCE:
    case AMULET_OF_REFLECTION:
    case FAKE_AMULET_OF_YENDOR:
        break;
    case AMULET_OF_MAGICAL_BREATHING: {
        boolean was_in_poison_gas;

        /* amulet is already on; we need to check hero's gas-cloud status
           when it was off */
        EMagical_breathing &= ~W_AMUL;
        was_in_poison_gas = region_danger();
        EMagical_breathing |= W_AMUL;
        if (was_in_poison_gas) {
            makeknown(AMULET_OF_MAGICAL_BREATHING);
            if (!on_msg_done)
                on_msg(uamul);
            on_msg_done = TRUE;
            You("are no longer bothered by the poison gas.");
        }
        /* any countdown here is a suffocating engulfer's (or some other
           non-amulet source): an amulet of strangulation would hold the
           slot, and only its strangling ignores magical breathing */
        if (Strangled) {
            makeknown(AMULET_OF_MAGICAL_BREATHING);
            if (!on_msg_done)
                on_msg(uamul);
            on_msg_done = TRUE;
            Strangled = 0L;
            disp.botl = TRUE;
            You("can suddenly breathe again!");
        }
        /* no need to check for becoming able to breathe underwater;
           if we are underwater, we already can or we would have drowned */
        break;
    }
    case AMULET_OF_UNCHANGING:
        if (Slimed)
            make_slimed(0L, (char *) 0);
        break;
    case AMULET_OF_CHANGE: {
        boolean call_it = FALSE;
        int new_sex, orig_sex = poly_gender();

        /* in normal play it's not possible to put on an amulet of change
           while already wearing an amulet of unchanging, but in wizard
           mode the Unchanging attribute can be set via #wizintrinsic */
        if (!Unchanging)
            change_sex();

        new_sex = poly_gender();
        if (new_sex != orig_sex)
            makeknown(AMULET_OF_CHANGE);
        on_msg(uamul); /* show 'z - amulet of change (being worn)' */
        on_msg_done = TRUE;

        /* Don't use same message as polymorph */
        if (new_sex != orig_sex) {
            newsym(u.ux, u.uy); /* glyphmon flag and tile have changed */
            disp.botl = TRUE; /* role name or rank title might have changed */
            You("are suddenly very %s!",
                flags.female ? "feminine" : "masculine");
        } else {
            /* already polymorphed into single-gender monster; only
               changed the character's base sex */
            You("don't feel like yourself.");
            /* checking dknown is redundant--amulets always have dknown set */
            call_it = (uamul->dknown != 0);
        }
        livelog_newform(FALSE, orig_sex, new_sex);
        pline_The("amulet disintegrates!");
        if (call_it)
            trycall(uamul);
        useup(uamul);
        break;
    }
    case AMULET_OF_STRANGULATION:
        if (can_be_strangled(&gy.youmonst)) {
            makeknown(AMULET_OF_STRANGULATION);
            /* a countdown already running (a suffocating engulfer's, or
               #wizintrinsic) keeps its count when that is shorter */
            if (!Strangled || Strangled > 6L)
                Strangled = 6L;
            disp.botl = TRUE;
            if (!on_msg_done)
                on_msg(uamul);
            on_msg_done = TRUE;
            /* an amulet polymorphed in place hasn't been shown yet, so
               "It" would name nothing */
            pline("%s constricts your throat!",
                  already_worn ? "Your amulet" : "It");
        }
        break;
    case AMULET_OF_RESTFUL_SLEEP: {
        long newnap = (long) rnd(98) + 2L, oldnap = (HSleepy & TIMEOUT);

        if (newnap < oldnap || oldnap == 0L)
            /* avoid clobbering FROMOUTSIDE bit, which might have
               gotten set by previously eating one of these amulets */
            HSleepy = (HSleepy & ~TIMEOUT) | newnap;
        break;
    }
    case AMULET_OF_FLYING:
        /* setworn() has already set extrinsic flying */
        float_vs_flight(); /* block flying if levitating */
        check_wings(TRUE); /* form with wings that can already fly? */
        if (Flying) {
            boolean already_flying;

            /* to determine whether this flight is new we have to muck
               about in the Flying intrinsic (actually extrinsic) */
            EFlying &= ~W_AMUL;
            already_flying = !!Flying;
            EFlying |= W_AMUL;

            if (!already_flying) {
                makeknown(AMULET_OF_FLYING);
                if (!on_msg_done)
                    on_msg(uamul);
                on_msg_done = TRUE;
                disp.botl = TRUE; /* status: 'Fly' On */
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

    if (!on_msg_done)
        on_msg(uamul);
}

/* the hero has just started flying because a hold, levitation, or
   terrain that blocked flight is gone; an amulet of flying put on under
   that block couldn't be identified by Amulet_on(), so identify it now
   if it is what lets the hero fly */
void
check_flight_amulet(void)
{
    boolean other_flight;

    if (!uamul || uamul->otyp != AMULET_OF_FLYING || !Flying)
        return;
    /* the same test Amulet_on() makes */
    EFlying &= ~W_AMUL;
    other_flight = !!Flying;
    EFlying |= W_AMUL;
    if (!other_flight)
        makeknown(AMULET_OF_FLYING);
}

void
Amulet_off(void)
{
    struct obj *amul = uamul; /* for off_msg() after setworn(NULL,W_AMUL) */
    boolean mkn = FALSE, early_off_msg = FALSE,
            /* poly_obj() swapping a worn amulet's form shows the new
               form itself; no "You were wearing" for the old one */
            quiet = (gw.worn_swap_quiet && amul == gw.worn_slot_exempt);

    svc.context.takeoff.mask &= ~W_AMUL;

    switch (uamul->otyp) {
    case AMULET_OF_ESP:
        /* need to update ability before calling see_monsters() */
        setworn((struct obj *) 0, W_AMUL);
        if (!quiet)
            off_msg(amul);
        early_off_msg = TRUE;

        see_monsters();
        break;
    case AMULET_OF_LIFE_SAVING:
    case AMULET_VERSUS_POISON:
    case AMULET_OF_MAGIC_RESISTANCE:
    case AMULET_OF_REFLECTION:
    case AMULET_OF_CHANGE:
    case AMULET_OF_UNCHANGING:
    case FAKE_AMULET_OF_YENDOR:
        break;
    case AMULET_OF_MAGICAL_BREATHING:
        /* amulet is currently still on; take it off before calling drown()
           and region_danger(); call off_msg() before specific messages */
        setworn((struct obj *) 0, W_AMUL);
        if (!quiet)
            off_msg(amul); /* 'uamul' has been set to Null */
        early_off_msg = TRUE;

        if (Underwater) {
            if (!Breathless && !Amphibious && !Swimming) {
                You("suddenly inhale an unhealthy amount of %s!",
                    hliquid("water"));
                mkn = TRUE; /* in case of life-saving */
                /* erode_obj() destroying the amulet or poly_obj()
                   changing it is still holding it, and drown() would
                   soak (and could destroy) what that caller holds; drown
                   once that caller is done */
                if (iflags.in_lava_effects)
                    defer_landing(LANDING_SPOT);
                else
                    (void) drown();
            }
        }
        if (region_danger()) {
            /* "breathing": wouldn't get here otherwise */
            You("are breathing poison gas!");
            mkn = TRUE;
        }
        break;
    case AMULET_OF_STRANGULATION:
        setworn((struct obj *) 0, W_AMUL);
        if (!quiet)
            off_msg(amul);
        early_off_msg = TRUE;

        if (Strangled) {
            /* a suffocating engulfer the hero can't breathe inside owns
               the countdown; it goes on without the amulet (a quiet
               poly_obj() swap puts the new form on next, which can end
               it, so "still" waits for a real take-off) */
            if (u.uswallow && u.ustuck
                && attacktype_fordmg(u.ustuck->data, AT_ENGL, AD_WRAP)
                && !engulf_air_ok(u.ustuck)) {
                if (!quiet) {
                    You("still can't breathe!");
                    mkn = TRUE;
                }
            } else {
                Strangled = 0L;
                disp.botl = TRUE;
                if (Breathless)
                    Your("%s is no longer constricted!", body_part(NECK));
                else
                    You("can breathe more easily!");
                mkn = TRUE;
            }
        }
        break;
    case AMULET_OF_RESTFUL_SLEEP:
        setworn((struct obj *) 0, W_AMUL);
        /* HSleepy = 0L; -- avoid clobbering FROMOUTSIDE bit */
        if (!ESleepy && !(HSleepy & ~TIMEOUT))
            HSleepy &= ~TIMEOUT; /* clear timeout bits */
        break;
    case AMULET_OF_FLYING: {
        boolean was_flying = !!Flying;

        /* remove amulet 'early' to determine whether Flying changes;
           also in case spoteffects() does something with the amulet */
        setworn((struct obj *) 0, W_AMUL);
        if (!quiet)
            off_msg(amul);
        early_off_msg = TRUE;

        float_vs_flight(); /* probably not needed here */
        check_wings(TRUE); /* form with wings that can already fly? */
        if (was_flying && !Flying)
            mkn = TRUE; /* makeknown(AMULET_OF_FLYING) */
        check_flight_landing(was_flying);
        break;
    }
    case AMULET_OF_GUARDING:
        find_ac();
        break;
    case AMULET_OF_YENDOR:
        break;
    }

    setworn((struct obj *) 0, W_AMUL);
    if (!early_off_msg && !quiet)
        off_msg(amul); /* (not 'uamul'; it's Null now) */
    if (mkn)
        makeknown(amul->otyp);
    return;
}

/* handle ring discovery; comparable to learnwand() */
staticfn void
learnring(struct obj *ring, boolean observed)
{
    int ringtype = ring->otyp;

    /* if effect was observable then we usually discover the type */
    if (observed) {
        /* if we already know the ring type which accomplishes this
           effect (assumes there is at most one type for each effect),
           mark this ring as having been seen (no need for makeknown);
           otherwise if we have seen this ring, discover its type */
        if (objects[ringtype].oc_name_known)
            observe_object(ring);
        else if (ring->dknown)
            makeknown(ringtype);
#if 0 /* see learnwand() */
        else
            observe_object(ring);
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

staticfn void
adjust_attrib(struct obj *obj, int which, int val)
{
    int old_attrib;
    boolean observable;

    old_attrib = ACURR(which);
    ABON(which) += val;
    observable = (old_attrib != ACURR(which));
    /* if didn't change, usually means ring is +0 but might
        be because nonzero couldn't go below min or above max;
        learn +0 enchantment if attribute value is not stuck
        at a limit [and ring has been seen and its type is
        already discovered, both handled by learnring()] */
    if (observable || !extremeattr(which))
        learnring(obj, observable);
    disp.botl = TRUE;
}

void
Ring_on(struct obj *obj)
{
    long oldprop = u.uprops[objects[obj->otyp].oc_oprop].extrinsic;
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
    case RIN_ANCIENT:
    case RIN_FREE_ACTION:
    case RIN_SLOW_DIGESTION:
    case RIN_SUSTAIN_ABILITY:
        break;
    case MEAT_RING:
        /* wearing a meat ring does not affect vegan conduct */
        break;
    case RIN_STEALTH:
        if (maybe_polyd(is_giant(gy.youmonst.data), Race_if(PM_GIANT))) {
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
    case RIN_LUSTROUS: /* grants invisibility and stealth */
        if (maybe_polyd(is_giant(gy.youmonst.data), Race_if(PM_GIANT))) {
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
                spoteffects_in_place(FALSE); /* for sinks */
        } else {
            float_vs_flight(); /* maybe toggle (BFlying & I_SPECIAL) */
        }
        break;
    case RIN_GAIN_STRENGTH:
        adjust_attrib(obj, A_STR, obj->spe);
        break;
    case RIN_GAIN_CONSTITUTION:
        adjust_attrib(obj, A_CON, obj->spe);
        break;
    case RIN_ADORNMENT:
        adjust_attrib(obj, A_CHA, obj->spe);
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

staticfn void
Ring_off_or_gone(struct obj *obj, boolean gone)
{
    long mask = (obj->owornmask & W_RING);
    boolean observable, was_levitating = !!Levitation;

    svc.context.takeoff.mask &= ~mask;
    if (!(u.uprops[objects[obj->otyp].oc_oprop].extrinsic & mask))
        impossible("Strange... I didn't know you had that ring.");
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
    case RIN_ANCIENT:
    case RIN_FREE_ACTION:
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
    case RIN_LUSTROUS:
        toggle_stealth(obj, (EStealth & ~mask), FALSE);
        if (!Invis && !BInvis && !Blind) {
            newsym(u.ux, u.uy);
            Your("body seems to unfade%s.",
                 See_invisible ? " completely" : "..");
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
        if (!(BLevitation & FROMOUTSIDE) && iflags.in_lava_effects) {
            /* erode_obj() is destroying the ring, poly_obj() is changing
               it, or lava_effects() is burning it off a hero already down
               in the lava; float_down() can't run under any of them (see
               Boots_off()) */
            float_vs_flight();
            disp.botl = TRUE; /* status: 'Lev' Off, maybe 'Fly' On */
            if (was_levitating && !Levitation) {
                if (Flying) {
                    You("have stopped levitating and are now flying.");
                    check_flight_amulet();
                } else {
                    defer_landing(LANDING_FLOAT);
                }
                learnring(obj, TRUE);
            }
        } else if (!(BLevitation & FROMOUTSIDE)) {
            /* in_use keeps lava_effects() from burning up the ring while
               we and our caller still hold it; only over lava, or over a
               trap that could teleport a lava walker onto it, since a
               hangup or death meanwhile uses up an in_use item */
            boolean was_in_use = obj->in_use;

            if (is_lava(u.ux, u.uy) || t_at(u.ux, u.uy))
                obj->in_use = 1;
            (void) float_down(0L, 0L);
            obj->in_use = was_in_use;
            if (!Levitation)
                learnring(obj, TRUE);
        } else {
            float_vs_flight(); /* maybe toggle (BFlying & I_SPECIAL) */
        }
        break;
    case RIN_GAIN_STRENGTH:
        adjust_attrib(obj, A_STR, -obj->spe);
        break;
    case RIN_GAIN_CONSTITUTION:
        adjust_attrib(obj, A_CON, -obj->spe);
        break;
    case RIN_ADORNMENT:
        adjust_attrib(obj, A_CHA, -obj->spe);
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
        /* if you're no longer protected, let the chameleons change
           shape again; however, might still be protected if wearing
           2nd ring of this type (or via #wizintrinsic) */
        if (!Protection_from_shape_changers)
            restartcham();
        break;
    }
}

void
Ring_off(struct obj *obj)
{
    Ring_off_or_gone(obj, FALSE);
}

void
Ring_gone(struct obj *obj)
{
    Ring_off_or_gone(obj, TRUE);
}

void
Blindf_on(struct obj *otmp)
{
    /* already in the eyewear slot when starting gear takes effect or a
       worn item was polymorphed in place: it isn't put on again and gets
       no inventory line here; after a swap (poly_obj()), sight is judged
       from before the old form came off (Blindf_off()) */
    boolean already_worn = (otmp->owornmask & W_TOOL) != 0L,
            swapped = (already_worn && gw.worn_swap_quiet
                       && gw.worn_slot_exempt
                       && gw.worn_slot_exempt != otmp),
            already_blind = swapped ? gw.worn_swap_blind : Blind,
            changed = FALSE,
            /* goggles and the Eyes let the hero see while submerged */
            old_uwvis = swapped ? gw.worn_swap_uwvis
                                : (Underwater && See_underwater);

    if (!already_worn) {
        /* blindfold might be wielded; release it for wearing */
        remove_worn_item(otmp, FALSE);
        setworn(otmp, W_TOOL);
        on_msg(otmp);
    }

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
    /* gaining or losing underwater sight switches the map between the
       submerged three-by-three and the ordinary view; redraw at the
       change rather than every turn, which is how the shapechanges do it */
    if (old_uwvis != (Underwater && See_underwater)) {
        vision_reset();
        docrt();
    }
}

void
Blindf_off(struct obj *otmp)
{
    boolean was_blind = Blind, changed = FALSE,
            nooffmsg = !otmp,
            /* goggles and the Eyes let the hero see while submerged */
            old_uwvis = (Underwater && See_underwater);

    if (!otmp)
        otmp = ublindf;
    if (!otmp) {
        impossible("Blindf_off without eyewear?");
        return;
    }
    svc.context.takeoff.mask &= ~W_TOOL;
    setworn((struct obj *) 0, otmp->owornmask);
    if (gw.worn_swap_quiet && otmp == gw.worn_slot_exempt) {
        /* poly_obj() puts the new form on next; Blindf_on() tells of any
           change in sight from here, so nothing is said or redrawn yet */
        gw.worn_swap_blind = was_blind;
        gw.worn_swap_uwvis = old_uwvis;
        return;
    }
    if (!nooffmsg)
        off_msg(otmp);

    if (Blind) {
        if (was_blind) {
            /* "still cannot see" makes no sense when removing lenses
               since they can't have been the cause of your blindness */
            if (otmp->otyp != LENSES && otmp->otyp != GOGGLES)
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
    /* losing underwater sight has to blank the map back down to the
       submerged view; nothing else redraws on the way out */
    if (old_uwvis != (Underwater && See_underwater)) {
        vision_reset();
        docrt();
    }
}

/* called in moveloop()'s prologue to set side-effects of worn start-up items;
   also used by poly_obj() when a worn item gets transformed */
void
set_wear(
    struct obj *obj) /* if Null, do all worn items; otherwise just obj */
{
    gi.initial_don = !obj;

    if (!obj ? ublindf != 0 : (obj == ublindf))
        (void) Blindf_on(ublindf);
    if (!obj ? uright != 0 : (obj == uright))
        (void) Ring_on(uright);
    if (!obj ? uleft != 0 : (obj == uleft))
        (void) Ring_on(uleft);
    if (!obj ? uamul != 0 : (obj == uamul))
        (void) Amulet_on(uamul);

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

    gi.initial_don = FALSE;
}

/* check whether the target object is currently being put on (or taken off--
   also checks for doffing--[why?]) */
boolean
donning(struct obj *otmp)
{
    boolean result = FALSE;

    /* 'W' (or 'P' used for armor) sets ga.afternmv */
    if (doffing(otmp))
        result = TRUE;
    else if (otmp == uarm)
        result = (ga.afternmv == Armor_on);
    else if (otmp == uarmu)
        result = (ga.afternmv == Shirt_on);
    else if (otmp == uarmc)
        result = (ga.afternmv == Cloak_on);
    else if (otmp == uarmf)
        result = (ga.afternmv == Boots_on);
    else if (otmp == uarmh)
        result = (ga.afternmv == Helmet_on);
    else if (otmp == uarmg)
        result = (ga.afternmv == Gloves_on);
    else if (otmp == uarms)
        result = (ga.afternmv == Shield_on);

    return result;
}

/* check whether the target object is currently being taken off,
   so that stop_donning() and steal() can vary messages and doname()
   can vary "(being worn)" suffix */
boolean
doffing(struct obj *otmp)
{
    long what = svc.context.takeoff.what;
    boolean result = FALSE;

    /* 'T' (or 'R' used for armor) sets ga.afternmv, 'A' sets takeoff.what */
    if (otmp == uarm)
        result = (ga.afternmv == Armor_off || what == WORN_ARMOR);
    else if (otmp == uarmu)
        result = (ga.afternmv == Shirt_off || what == WORN_SHIRT);
    else if (otmp == uarmc)
        result = (ga.afternmv == Cloak_off || what == WORN_CLOAK);
    else if (otmp == uarmf)
        result = (ga.afternmv == Boots_off || what == WORN_BOOTS);
    else if (otmp == uarmh)
        result = (ga.afternmv == Helmet_off || what == WORN_HELMET);
    else if (otmp == uarmg)
        result = (ga.afternmv == Gloves_off || what == WORN_GLOVES);
    else if (otmp == uarms)
        result = (ga.afternmv == Shield_off || what == WORN_SHIELD);
    /* these 1-turn items don't need 'ga.afternmv' checks */
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
cancel_doff(struct obj *obj, long slotmask)
{
    /* Called by setworn() for old item in specified slot or by setnotworn()
     * for specified item.  We don't want to call cancel_don() if we got
     * here via <X>_off() -> setworn((struct obj *) 0) -> cancel_doff()
     * because that would stop the 'A' command from continuing with next
     * selected item.  So do_takeoff() sets a flag in takeoff.mask for us.
     * [For taking off an individual item with 'T'/'R'/'w-', it doesn't
     * matter whether cancel_don() gets called here--the item has already
     * been removed by now.]
     */
    if (!(svc.context.takeoff.mask & I_SPECIAL) && donning(obj))
        cancel_don(); /* applies to doffing too */
    svc.context.takeoff.mask &= ~slotmask;
}

/* despite their names, cancel_don() and cancel_doff() both apply to both
   donning and doffing... */
void
cancel_don(void)
{
    /* the piece of armor we were donning/doffing has vanished, so stop
     * wasting time on it (and don't dereference it when donning would
     * otherwise finish); afternmv never has some of these values because
     * every item of the corresponding armor category takes 1 turn to wear,
     * but check all of them anyway
     */
    svc.context.takeoff.cancelled_don = (ga.afternmv == Cloak_on
                                        || ga.afternmv == Armor_on
                                        || ga.afternmv == Shirt_on
                                        || ga.afternmv == Helmet_on
                                        || ga.afternmv == Gloves_on
                                        || ga.afternmv == Boots_on
                                        || ga.afternmv == Shield_on);
    ga.afternmv = (int (*)(void)) 0;
    gn.nomovemsg = (char *) 0;
    gm.multi = 0;
    svc.context.takeoff.delay = 0;
    svc.context.takeoff.what = 0L;
}

/* called by steal() during theft from hero; interrupt donning/doffing */
int
stop_donning(
    struct obj *stolenobj) /* no mesg if stolenobj is already being doffed */
{
    char buf[BUFSZ];
    struct obj *otmp;
    boolean putting_on;
    int result = 0;

    for (otmp = gi.invent; otmp; otmp = otmp->nobj)
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
    ga.afternmv = (int (*)(void)) 0;
    if (putting_on || otmp != stolenobj) {
        Sprintf(buf, "You stop %s %s.",
                putting_on ? "putting on" : "taking off",
                thesimpleoname(otmp));
    } else {
        buf[0] = '\0';   /* silently stop doffing stolenobj */
        result = (int) -gm.multi; /* remember this before calling unmul() */
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

static NEARDATA int Narmorpieces, Naccessories;

/* assign values to Narmorpieces and Naccessories */
staticfn void
count_worn_stuff(
    struct obj **which, /* caller wants this when count is 1 */
    boolean accessorizing)
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
staticfn int
armor_or_accessory_off(struct obj *obj)
{
    if (!(obj->owornmask & (W_ARMOR | W_ACCESSORY))) {
        You("are not wearing that.");
        return ECMD_OK;
    }
    if ((druid_form || vampire_form)
        && (obj->owornmask & W_ARMOR)) {
        You_cant("take that off; it's merged.");
        return ECMD_OK;
    }
    if (obj == uskin
        || ((obj == uarm) && uarmc)
        || ((obj == uarmu) && (uarmc || uarm))) {
        char why[QBUFSZ], what[QBUFSZ];

        why[0] = what[0] = '\0';
        if (obj != uskin) {
            if (uarmc)
                Strcat(what, cloak_simple_name(uarmc));
            if ((obj == uarmu) && uarm) {
                if (uarmc)
                    Strcat(what, " and ");
                Strcat(what, suit_simple_name(uarm));
            }
            Snprintf(why, sizeof why, " without taking off your %s first",
                     what);
        } else {
            Strcpy(why, "; it's embedded");
        }
        You_cant("take that off%s.", why);
        return ECMD_OK;
    }

    reset_remarm(); /* clear context.takeoff.mask and context.takeoff.what */
    (void) select_off(obj);
    if (!svc.context.takeoff.mask)
        return ECMD_OK;
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
        Amulet_off(); /* does its own off_msg */
    } else if (obj == ublindf) {
        Blindf_off(obj); /* does its own off_msg */
    } else {
        impossible("removing strange accessory: %s",
                   safe_typename(obj->otyp));
        if (obj->owornmask)
            remove_worn_item(obj, FALSE);
    }
    return ECMD_TIME;
}

/* the #takeoff command - remove worn armor */
int
dotakeoff(void)
{
    struct obj *otmp = (struct obj *) 0;

    count_worn_stuff(&otmp, FALSE);
    if (!Narmorpieces && !Naccessories) {
        if (uskin)
            pline("Your scaly armor is merged with your skin!");
        else
            pline("Not wearing any armor or accessories.");
        return ECMD_FAIL;
    }
    if (Hidinshell) {
        You_cant("take off worn items while hiding in your shell.");
        return ECMD_FAIL;
    }
    if (Narmorpieces != 1 || ParanoidRemove || gi.item_action_in_progress)
        otmp = getobj("take off", takeoff_ok, GETOBJ_NOFLAGS);
    if (!otmp)
        return ECMD_CANCEL;

    return armor_or_accessory_off(otmp);
}

/* 'i' or 'I[' followed by <invlet> and then 'T';
   plain dotakeoff() would not give any feedback when picking suit
   covered by cloak, or shirt covered by suit and/or cloak, due to the
   default behavior of equip_ok() (skipping inaccessible items) */
int
ia_dotakeoff(void)
{
    int res;

    gi.item_action_in_progress = TRUE;
    res = dotakeoff();
    gi.item_action_in_progress = FALSE;
    return res;
}

/* the #remove command - take off ring or other accessory */
int
doremring(void)
{
    struct obj *otmp = 0;

    count_worn_stuff(&otmp, TRUE);
    if (!Naccessories && !Narmorpieces) {
        pline("Not wearing any accessories or armor.");
        return ECMD_OK;
    }
    if (Hidinshell) {
        You_cant("take off worn items while hiding in your shell.");
        return ECMD_OK;
    }
    if (Naccessories != 1 || ParanoidRemove || cmdq_peek(CQ_CANNED))
        otmp = getobj("remove", remove_ok, GETOBJ_NOFLAGS);
    if (!otmp)
        return ECMD_CANCEL;

    return armor_or_accessory_off(otmp);
}

/* Check if something worn is cursed _and_ unremovable. */
int
cursed(struct obj *otmp, boolean silent)
{
    if (!otmp) {
        impossible("cursed without otmp");
        return 0;
    }
    /* Infidels are immune to curses; the Hand of Vecna serves its
       wearer willingly */
    if (Role_if(PM_INFIDEL)
        || (otmp == uarmg && otmp->oartifact == ART_HAND_OF_VECNA))
        return 0;
    /* Curses, like chickens, come home to roost. */
    if ((otmp == uwep) ? welded(otmp) : (int) otmp->cursed) {
        boolean use_plural;

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
    return 0;
}

int
armoroff(struct obj *otmp)
{
    static char offdelaybuf[60];
    int delay = -objects[otmp->otyp].oc_delay;
    int armcat = armcat_of(otmp);
    const char *what = 0;

    if (cursed(otmp, FALSE))
        return 0;
    /* this used to make assumptions about which types of armor had
       delays and which didn't; now both are handled for all types */
    if (delay) {
        nomul(delay);
        gm.multi_reason = "disrobing";
        switch (armcat) {
        case ARM_SUIT:
            what = suit_simple_name(otmp);
            ga.afternmv = Armor_off;
            break;
        case ARM_SHIELD:
            what = shield_simple_name(otmp);
            ga.afternmv = Shield_off;
            break;
        case ARM_HELM:
            what = helm_simple_name(otmp);
            ga.afternmv = Helmet_off;
            break;
        case ARM_GLOVES:
            what = gloves_simple_name(otmp);
            ga.afternmv = Gloves_off;
            break;
        case ARM_BOOTS:
            what = boots_simple_name(otmp);
            ga.afternmv = Boots_off;
            break;
        case ARM_CLOAK:
            what = cloak_simple_name(otmp);
            ga.afternmv = Cloak_off;
            break;
        case ARM_SHIRT:
            what = shirt_simple_name(otmp);
            ga.afternmv = Shirt_off;
            break;
        default:
            impossible("Taking off unknown armor (%d: %d), delay %d",
                       otmp->otyp, armcat, delay);
            break;
        }
        if (what) {
            /* sizeof offdelaybuf == 60; increase it if this becomes longer */
            Snprintf(offdelaybuf, sizeof offdelaybuf,
                     "You finish taking off your %s.", what);
            gn.nomovemsg = offdelaybuf;
        }
    } else {
        /* no delay so no '(*afternmv)()' or 'nomovemsg' */
        switch (armcat) {
        case ARM_SUIT:
            (void) Armor_off();
            break;
        case ARM_SHIELD:
            (void) Shield_off();
            break;
        case ARM_HELM:
            (void) Helmet_off();
            break;
        case ARM_GLOVES:
            (void) Gloves_off();
            break;
        case ARM_BOOTS:
            (void) Boots_off();
            break;
        case ARM_CLOAK:
            (void) Cloak_off();
            break;
        case ARM_SHIRT:
            (void) Shirt_off();
            break;
        default:
            impossible("Taking off unknown armor (%d: %d), no delay",
                       otmp->otyp, armcat);
            break;
        }
        /* We want off_msg() after removing the item to
           avoid "You were wearing ____ (being worn)." */
        off_msg(otmp);
    }
    svc.context.takeoff.mask = svc.context.takeoff.what = 0L;
    return 1;
}

staticfn void
already_wearing(const char *cc)
{
    You("are already wearing %s%c", cc, (cc == c_that_) ? '!' : '.');
}

staticfn void
already_wearing2(const char *cc1, const char *cc2)
{
    You_cant("wear %s because you're wearing %s there already.", cc1, cc2);
}

/* will wearing (or wielding) an item with the given worn mask bring it
   into direct contact with the hero's skin? */
boolean
will_touch_skin(long mask)
{
    if ((mask == W_ARMC || mask == W_AMUL) && (uarm || uarmu))
        return FALSE;
    else if (mask == W_ARM && uarmu)
        return FALSE;
    else if ((mask & (W_WEP | W_SWAPWEP | W_ARMS)) && uarmg)
        return FALSE;
    else if ((mask & W_RINGL) && uarmg
             && uarmg->oartifact == ART_HAND_OF_VECNA)
        return FALSE;
    else if (mask == W_QUIVER)
        return FALSE;
    return TRUE;
}

/* a suit, shirt or cloak that won't fit the hero's current body:
   canwearobj()'s test for a form that can't wear body armor, with its
   exceptions; u_can_wear_armor() asks it too */
staticfn boolean
u_body_armor_misfit(struct obj *otmp)
{
    return (cantweararm(&gy.youmonst)
            /* same exception for cloaks as used in m_slot_usable() and
               m_armor_candidate() */
            && (!is_cloak(otmp)
                || ((otmp->otyp != MUMMY_WRAPPING)
                    ? gy.youmonst.data->msize != MZ_SMALL
                    : !WrappingAllowed(gy.youmonst.data)))
            && (racial_exception(&gy.youmonst, otmp) < 1)
            /* giant heroes have a few outsized armor options */
            && !(Race_if(PM_GIANT) && Role_if(PM_SAMURAI)
                 && otmp->otyp == LARGE_SPLINT_MAIL)
            && !(Race_if(PM_GIANT)
                 && otmp->otyp == CHROMATIC_DRAGON_SCALES));
}

/*
 * canwearobj checks to see whether the player can wear a piece of armor
 *
 * inputs: otmp (the piece of armor)
 *         noisy (if TRUE give error messages, otherwise be quiet about it)
 * output: mask (otmp's armor type)
 *
 * u_can_wear_armor() repeats the limits here that last (form, horns, feet,
 * the pious gauntlets); keep the two in step
 */
int
canwearobj(struct obj *otmp, long *mask, boolean noisy)
{
    int err = 0;
    const char *which;

    /* this is the same check as for 'W' (dowear), but different message,
       in case we get here via 'P' (doputon) */
    if (verysmall(gy.youmonst.data) || nohands(gy.youmonst.data)
        || is_ent(gy.youmonst.data) || is_plant(gy.youmonst.data)) {
        if (noisy)
            You("can't wear any armor in your current form.");
        return 0;
    }
    if (Hidinshell) {
        if (noisy)
            You("can't wear any armor while hiding in your shell.");
        return 0;
    }

    which = is_cloak(otmp) ? c_cloak
            : is_shirt(otmp) ? c_shirt
              : (is_suit(otmp) || otmp->otyp == MEAT_SUIT) ? c_suit
                : 0;
    if (which && u_body_armor_misfit(otmp)) {
        if (noisy)
            pline_The("%s will not fit on your body.", which);
        return 0;
    } else if (otmp->owornmask & W_ARMOR) {
        if (noisy)
            already_wearing(c_that_);
        return 0;
    }

    if (welded(uwep) && bimanual(uwep)
        && (is_suit(otmp) || otmp->otyp == MEAT_SUIT || is_shirt(otmp))) {
        if (noisy)
            You("cannot do that while holding your %s.",
                is_sword(uwep) ? c_sword : c_weapon);
        return 0;
    }

    /* stone to flesh's meat armor is food, so the is_<slot>() tests
       below don't know it */
    if (is_helmet(otmp) || otmp->otyp == MEAT_HELMET) {
        if (uarmh) {
            if (noisy)
                already_wearing(an(helm_simple_name(uarmh)));
            err++;
        } else if (Upolyd && has_horns(gy.youmonst.data) && !is_flimsy(otmp)) {
            /* (flimsy exception matches polyself handling) */
            if (noisy) {
                if (has_antlers(gy.youmonst.data))
                    pline_The("%s won't fit over your antlers.",
                              helm_simple_name(otmp));
                else
                    pline_The("%s won't fit over your horn%s.",
                              helm_simple_name(otmp),
                              plur(num_horns(gy.youmonst.data)));
            }
            err++;
        } else
            *mask = W_ARMH;
    } else if (is_shield(otmp) || otmp->otyp == MEAT_SHIELD) {
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
        } else if (Upolyd && slithy(gy.youmonst.data)) {
            if (noisy)
                You("have no feet..."); /* not body_part(FOOT) */
            err++;
        } else if ((Upolyd && (gy.youmonst.data->mlet == S_CENTAUR
                               || gy.youmonst.data == &mons[PM_DRIDER]
                               || gy.youmonst.data == &mons[PM_SATYR]))
                   || (!Upolyd && Race_if(PM_CENTAUR))) {
            /* break_armor() pushes boots off for centaurs, so don't let
               dowear() put them back on;
               makeplural(body_part(FOOT)) would yield "rear hooves" here,
               which sounds odd, so use hard-coded "hooves" */
            if (noisy)
                Your("%s are not shaped correctly to wear %s.",
                     /* a centaur hero's data is the role's monster,
                        so test for the one form without hooves */
                     (gy.youmonst.data == &mons[PM_DRIDER]) ? "tarsi"
                                                            : "hooves",
                     c_boots);
            err++;
        } else if (racial_tortle(&gy.youmonst)) {
            /* tortle foot shape is all wrong, race or polymorphed form */
            if (noisy)
                Your("%s are not shaped correctly to wear %s.",
                     makeplural(body_part(FOOT)), c_boots);
            err++;
        } else if (u.utrap
                   && (u.utraptype == TT_BEARTRAP || u.utraptype == TT_INFLOOR
                       || u.utraptype == TT_LAVA
                       || u.utraptype == TT_BURIEDBALL)) {
            if (u.utraptype == TT_BEARTRAP) {
                if (noisy)
                    Your("%s is trapped!", body_part(FOOT));
            } else if (u.utraptype == TT_INFLOOR || u.utraptype == TT_LAVA) {
                if (noisy)
                    Your("%s are stuck in the %s!",
                         makeplural(body_part(FOOT)), surface(u.ux, u.uy));
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
        } else if (!wizard && is_art(otmp, ART_GAUNTLETS_OF_PURITY)
                   && (u.ualign.record < 20 || Role_if(PM_INFIDEL))) {
            /* only the pious may wear them, and an Infidel never */
            if (noisy) {
                if (Role_if(PM_INFIDEL))
                    pline_The("%s sense your wickedness, and refuse to be"
                              " worn!", xname(otmp));
                else
                    You("are not pure enough to wear these %s.",
                        gloves_simple_name(otmp));
            }
            err++;
        } else if (is_art(otmp, ART_HAND_OF_VECNA)
                   && racial_aasimar(&gy.youmonst)) {
            if (noisy)
                You("recoil from %s; such an unholy relic is anathema"
                    " to your celestial nature.", the(xname(otmp)));
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
        /* the chromatic dragon's scales barely fit a giant */
        if (!err && !Upolyd && Race_if(PM_GIANT)
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
        } else if (gy.youmonst.data->msize < MZ_HUGE
                   && otmp->otyp == LARGE_SPLINT_MAIL) {
            if (noisy)
                You("are too small to wear such a suit of armor.");
            err++;
        } else
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

/* could the hero's current body wear armor 'otmp' at all? canwearobj()'s
   lasting limits only (keep the two in step): what is worn or wielded
   now, a shell, slippery fingers and traps all pass; a pet asks, so that
   it leaves the hero's upgrades alone (dogmove.c's is_better_armor()) */
boolean
u_can_wear_armor(struct obj *otmp)
{
    if (otmp->oclass != ARMOR_CLASS)
        return FALSE;
    /* a druid's wildshape and a vampire's shift form keep the worn gear
       merged until the hero changes back, so the form itself doesn't
       rule armor out (as sliparm() and breakarm() reckon it) */
    if (!(druid_form || vampire_form)
        && (verysmall(gy.youmonst.data) || nohands(gy.youmonst.data)
            || is_ent(gy.youmonst.data) || is_plant(gy.youmonst.data)))
        return FALSE;
    if ((is_cloak(otmp) || is_shirt(otmp) || is_suit(otmp))
        && u_body_armor_misfit(otmp))
        return FALSE;
    if (is_helmet(otmp))
        return !(Upolyd && has_horns(gy.youmonst.data) && !is_flimsy(otmp));
    if (is_boots(otmp))
        return !((Upolyd && slithy(gy.youmonst.data))
                 || (Upolyd && (gy.youmonst.data->mlet == S_CENTAUR
                                || gy.youmonst.data == &mons[PM_DRIDER]
                                || gy.youmonst.data == &mons[PM_SATYR]))
                 || (!Upolyd && Race_if(PM_CENTAUR))
                 || racial_tortle(&gy.youmonst));
    if (is_gloves(otmp))
        return !((!wizard && is_art(otmp, ART_GAUNTLETS_OF_PURITY)
                  && (u.ualign.record < 20 || Role_if(PM_INFIDEL)))
                 || (is_art(otmp, ART_HAND_OF_VECNA)
                     && racial_aasimar(&gy.youmonst)));
    if (is_suit(otmp))
        return !(gy.youmonst.data->msize < MZ_HUGE
                 && otmp->otyp == LARGE_SPLINT_MAIL);
    return TRUE;
}

staticfn int
accessory_or_armor_on(struct obj *obj)
{
    long mask = 0L;
    boolean armor, ring, amulet, eyewear;
    struct permonst *pre_touch_form;

    if (obj->owornmask & (W_ACCESSORY | W_ARMOR)) {
        already_wearing(c_that_);
        return ECMD_OK;
    }
    armor = (obj->oclass == ARMOR_CLASS || is_meat_armor(obj));
    ring = (obj->oclass == RING_CLASS || obj->otyp == MEAT_RING);
    amulet = (obj->oclass == AMULET_CLASS);
    eyewear = (obj->otyp == BLINDFOLD || obj->otyp == TOWEL
               || obj->otyp == LENSES || obj->otyp == GOGGLES);
    /* checks which are performed prior to actually touching the item */
    if (armor) {
        if (!canwearobj(obj, &mask, TRUE))
            return ECMD_OK;

        if (obj->otyp == HELM_OF_OPPOSITE_ALIGNMENT
            && qstart_level.dnum == u.uz.dnum) { /* in quest */
            if (u.ualignbase[A_CURRENT] == u.ualignbase[A_ORIGINAL])
                You("narrowly avoid losing all chance at your goal.");
            else /* converted */
                You("are suddenly overcome with shame and change your mind.");
            u.ublessed = 0; /* lose your god's protection */
            makeknown(obj->otyp);
            disp.botl = TRUE; /* for AC after zeroing u.ublessed */
            return ECMD_TIME;
        }
    } else {
        /*
         * FIXME:
         *  except for the rings/nolimbs case, this allows you to put on
         *  accessories without having any hands to manipulate them, and
         *  to put them on when poly'd into a tiny or huge form where
         *  they shouldn't fit.  [If the latter situation changes, make
         *  comparable change to break_armor(polyself.c).]
         */

        /* accessory */
        if (ring) {
            char answer, qbuf[QBUFSZ];
            int res = 0;

            if (nolimbs(gy.youmonst.data)) {
                You("cannot make the ring stick to your body.");
                return ECMD_OK;
            }
            if (uleft && uright) {
                There("are no more %s%s to fill.",
                      humanoid(gy.youmonst.data) ? "ring-" : "",
                      fingers_or_gloves(FALSE));
                return ECMD_OK;
            }
            if (uleft) {
                mask = RIGHT_RING;
            } else if (uright) {
                mask = LEFT_RING;
            } else {
                do {
                    Sprintf(qbuf, "Which %s%s, Right or Left?",
                            humanoid(gy.youmonst.data) ? "ring-" : "",
                            body_part(FINGER));
                    answer = yn_function(qbuf, rightleftchars, '\0', TRUE);
                    switch (answer) {
                    case '\0':
                    case '\033':
                        return ECMD_OK;
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
                return ECMD_TIME; /* always uses move */
            }
            if (uarmg && cursed(uarmg, TRUE)) {
                res = !uarmg->bknown;
                set_bknown(uarmg, 1);
                You("cannot remove your %s to put on the ring.", c_gloves);
                /* uses move iff we learned gloves are cursed */
                return res ? ECMD_TIME : ECMD_OK;
            }
            if (uwep) {
                res = !uwep->bknown; /* check this before calling welded() */
                if (((mask == RIGHT_RING && URIGHTY)
                     || (mask == LEFT_RING  && ULEFTY)
                     || bimanual(uwep)) && welded(uwep)) {
                    const char *hand = body_part(HAND);

                    /* welded will set bknown */
                    if (bimanual(uwep))
                        hand = makeplural(hand);
                    You("cannot free your weapon %s to put on the ring.",
                        hand);
                    /* uses move iff we learned weapon is cursed */
                    return res ? ECMD_TIME : ECMD_OK;
                }
            }
        } else if (amulet) {
            if (uamul) {
                already_wearing("an amulet");
                return ECMD_OK;
            }
        } else if (eyewear) {
            if (!has_head(gy.youmonst.data)) {
                You("have no head to wear %s on.", ansimpleoname(obj));
                return ECMD_OK;
            }

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
                return ECMD_OK;
            }
        } else {
            /* neither armor nor accessory */
            You_cant("wear that!");
            return ECMD_OK;
        }
    }

    pre_touch_form = gy.youmonst.data;
    if (!retouch_object(&obj, will_touch_skin(mask), FALSE))
        return ECMD_TIME; /* costs a turn even though it didn't get worn */
    /* the touch can burn the hero out of a polymorphed form; armor that
       fit the old body has to fit the new one too */
    if (armor && gy.youmonst.data != pre_touch_form
        && !canwearobj(obj, &mask, TRUE))
        return ECMD_TIME;

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
         * until the afternmv action.  The player may still know this armor's
         * +/- amount if donning gets interrupted, but the hero won't.
         *
        obj->known = 1;
         */
        gw.wasinwater = u.uinwater; /* for WWALKING; Boots_on() is too late */
        setworn(obj, mask);
        /* if there's no delay, we'll execute 'afternmv' immediately */
        if (obj == uarm)
            ga.afternmv = Armor_on;
        else if (obj == uarmh)
            ga.afternmv = Helmet_on;
        else if (obj == uarmg)
            ga.afternmv = Gloves_on;
        else if (obj == uarmf)
            ga.afternmv = Boots_on;
        else if (obj == uarms)
            ga.afternmv = Shield_on;
        else if (obj == uarmc)
            ga.afternmv = Cloak_on;
        else if (obj == uarmu)
            ga.afternmv = Shirt_on;
        else
            panic("wearing armor not worn as armor? [%08lx]", obj->owornmask);

        delay = -objects[obj->otyp].oc_delay;
        if (delay) {
            nomul(delay);
            gm.multi_reason = "dressing up";
            if (obj == uarmg && obj->oartifact == ART_HAND_OF_VECNA) {
                if (u.ualign.type == A_NONE)
                    com_pager("vecna_hand_evil");
                else
                    com_pager("vecna_hand");
            } else {
                gn.nomovemsg = "You finish your dressing maneuver.";
            }
        } else {
            unsigned oid = obj->o_id;

            unmul(""); /* call afternmv, clear it+nomovemsg+multi_reason */
            /* a lit suit or cloak can hurt a light-hating hero back into
               a natural form whose break_armor() destroys it */
            if (o_on(oid, gi.invent) == obj && (obj->owornmask & mask))
                on_msg(obj);
        }
        svc.context.takeoff.mask = svc.context.takeoff.what = 0L;
        /* gw.wasinwater = 0U; // can't clear this yet; Boots_on() needs it
         * and gets called via afternmv() after this routine has returned */
    } else { /* not armor */
        if (ring) {
            /* Ring_on() expects ring to already be worn as uleft or uright */
            setworn(obj, mask);
            Ring_on(obj);
            /* is_worn(): 'obj' will always be worn here except when putting
               on a ring of levitation while at a sink location */
            if (is_worn(obj))
                on_msg(obj);
        } else if (amulet) {
            /* setworn() and on_msg() handled by Amulet_on() */
            Amulet_on(obj);
        } else if (eyewear) {
            /* setworn() and on_msg() handled by Blindf_on() */
            Blindf_on(obj);
        } else {
            impossible("putting on unexpected type of accessory: %s",
                       safe_typename(obj->otyp));
        }
    }
    return ECMD_TIME;
}

/* predicate for query_objlist(): an unworn shield, but not bracers */
staticfn boolean
is_wearable_shield(struct obj *otmp)
{
    return (boolean) (is_shield(otmp) && !is_bracer(otmp)
                      && !otmp->owornmask);
}

/* Ready the shield hand: leave two-weapon combat and, if a two-handed
   weapon is wielded, switch to a one-handed one. Returns FALSE (with
   feedback) when that can't be done so the caller leaves the shield
   unworn. bimanual() already treats a giant's two-handed melee weapon
   as one-handed, while bows and crossbows stay two-handed for everyone */
staticfn boolean
free_shieldhand(void)
{
    /* leave two-weapon combat the same way the 'twoweapon' command does,
       with matching feedback, then free the hand for the shield */
    if (u.twoweap) {
        struct obj *tmp = uswapwep;

        You("switch to your primary weapon.");
        set_twoweap(FALSE); /* u.twoweap = FALSE */
        /* reseat the off-hand weapon so that setworn() revokes its
           W_SWAPWEP extrinsics now that two-weapon combat is off */
        setuswapwep((struct obj *) 0);
        setuswapwep(tmp);
        update_inventory();
    }
    /* a shield needs only a free hand, not a weapon: nothing to do when
       empty handed or wielding a one-handed weapon */
    if (!uwep || !bimanual(uwep))
        return TRUE;
    /* a welded two-hander can't be set aside */
    if (welded(uwep)) {
        weldmsg(uwep);
        return FALSE;
    }
    /* make room for the shield: keep the readied one-handed weapon if
       there is one, otherwise set the two-hander aside (a shield needs
       only a free hand, not a weapon) */
    if (uswapwep && !bimanual(uswapwep)
        && (uswapwep->oclass == WEAPON_CLASS || is_weptool(uswapwep))) {
        (void) doswapweapon();
    } else {
        struct obj *twohander = uwep;

        setuwep((struct obj *) 0);
        You("set %s aside.", yname(twohander));
    }
    return TRUE;
}

/* the carried shield previously chosen for sword & board, if any */
staticfn struct obj *
find_altshield(void)
{
    struct obj *otmp;

    for (otmp = gi.invent; otmp; otmp = otmp->nobj) {
        if (otmp->altshield && is_shield(otmp) && !is_bracer(otmp)
            && !otmp->owornmask)
            return otmp;
    }
    return (struct obj *) 0;
}

/* mark shield as the designated sword-and-board shield, clearing the
   mark from any other object in inventory; the bit can survive on an
   object that leaves inventory, so two carried objects can transiently
   be marked after such an object is picked back up - find_altshield()
   takes the first and the next designation re-sweeps */
void
designate_altshield(struct obj *shield)
{
    struct obj *otmp;

    for (otmp = gi.invent; otmp; otmp = otmp->nobj) {
        if (otmp != shield && otmp->altshield)
            otmp->altshield = 0;
    }
    if (shield)
        shield->altshield = 1;
}

/* the #shield command (default key '{'): toggle a shield on or off,
   handling the off-hand weapon the way the 'twoweapon' command handles
   the primary and secondary weapons */
int
doshield(void)
{
    struct obj *shield;
    menu_item *pick_list = (menu_item *) 0;
    unsigned sid;
    int n;

    /* toggle off: a shield (not bracers) is currently worn */
    if (uarms && !is_bracer(uarms)) {
        shield = uarms;
        /* remove_worn_item() is the forced-removal helper and does not
           check for curses, so make the same check 'T' would make */
        if (cursed(shield, FALSE))
            return ECMD_OK;
        remove_worn_item(shield, FALSE);
        designate_altshield(shield);
        You("take off %s.", yname(shield));
        return ECMD_TIME;
    }

    /* toggle on -- same "can you wear armor at all" guard as dowear() */
    if (verysmall(gy.youmonst.data) || nohands(gy.youmonst.data)
        || Hidinshell || is_ent(gy.youmonst.data)
        || is_plant(gy.youmonst.data)) {
        pline("Don't even bother.");
        return ECMD_OK;
    }
    /* shield slot is already taken by bracers */
    if (uarms) {
        already_wearing(yname(uarms));
        return ECMD_OK;
    }

    /* prefer the previously designated shield; otherwise choose one
       (auto-select if only one is carried) and remember it */
    shield = find_altshield();
    if (!shield) {
        n = query_objlist("Wear which shield?", &gi.invent,
                          (USE_INVLET | INVORDER_SORT | AUTOSELECT_SINGLE
                           | SIGNAL_NOMENU),
                          &pick_list, PICK_ONE, is_wearable_shield);
        if (n < 0) {
            You("have no shield to use.");
            return ECMD_OK;
        }
        if (n == 0)
            return ECMD_CANCEL; /* cancelled the menu */
        shield = pick_list[0].item.a_obj;
        free((genericptr_t) pick_list);
    }

    /* clear the shield hand, then don the shield */
    sid = shield->o_id;
    if (!free_shieldhand())
        return ECMD_OK;
    /* swapping weapons can hurt a polymorphed hero back into a natural
       form whose landing in water or lava loses the shield */
    if (o_on(sid, gi.invent) != shield)
        return ECMD_TIME;

    n = accessory_or_armor_on(shield);
    designate_altshield(shield);
    return n;
}

/* the #wear command */
int
dowear(void)
{
    struct obj *otmp;

    /* cantweararm() checks for suits of armor, not what we want here;
       verysmall() or nohands() checks for shields, gloves, etc... */
    if (verysmall(gy.youmonst.data) || nohands(gy.youmonst.data)
        || Hidinshell || is_ent(gy.youmonst.data)
        || is_plant(gy.youmonst.data)) {
        pline("Don't even bother.");
        return ECMD_FAIL;
    }
    if (uarm && uarmu && uarmc && uarmh && uarms && uarmg && uarmf
        && uleft && uright && uamul && ublindf) {
        /* 'W' message doesn't mention accessories */
        You("are already wearing a full complement of armor.");
        return ECMD_FAIL;
    }
    otmp = getobj("wear", wear_ok, GETOBJ_NOFLAGS);
    return otmp ? accessory_or_armor_on(otmp) : ECMD_CANCEL;
}

/* the #puton command */
int
doputon(void)
{
    struct obj *otmp;

    if (Hidinshell) {
        You_cant("put on any items while hiding in your shell.");
        return ECMD_OK;
    }
    if (uleft && uright && uamul && ublindf
        && uarm && uarmu && uarmc && uarmh && uarms && uarmg && uarmf) {
        /* 'P' message doesn't mention armor */
        Your("%s%s are full, and you're already wearing an amulet and %s.",
             humanoid(gy.youmonst.data) ? "ring-" : "",
             fingers_or_gloves(FALSE),
             (ublindf->otyp == LENSES) ? "some lenses"
             : (ublindf->otyp == GOGGLES) ? "some goggles"
               : "a blindfold");
        return ECMD_OK;
    }
    otmp = getobj("put on", puton_ok, GETOBJ_NOFLAGS);
    return otmp ? accessory_or_armor_on(otmp) : ECMD_CANCEL;
}

/* calculate current armor class */
void
find_ac(void)
{
    /* base armor class for current form; giants can't wear body
       armor, shirts, or cloaks, but their thick skin compensates a
       bit; a tortle's shell provides exceptional protection */
    int uac = maybe_polyd(mons[u.umonnum].ac,
                          Race_if(PM_GIANT) ? 6
                          : Race_if(PM_TORTLE) ? 0
                            : mons[u.umonnum].ac);
    int racial_bonus, dex_adjust_ac, tortle_ac;

    /* wearing racial armor is worth a bit of extra AC; orcs get a
       slightly larger bonus to compensate for their sub-standard
       equipment (taken from SporkHack); drow gloves are deliberately
       not on this list */
    racial_bonus = Race_if(PM_ORC) ? 2
                   : (Race_if(PM_ELF) || Race_if(PM_DWARF)
                      || Race_if(PM_DROW)) ? 1 : 0;

    /* armor class from worn gear */
    if (uarm) {
        uac -= armor_bonus(uarm);
        if ((Race_if(PM_ORC)
             && (uarm->otyp == ORCISH_CHAIN_MAIL
                 || uarm->otyp == ORCISH_RING_MAIL))
            || (Race_if(PM_DROW)
                && (uarm->otyp == DARK_ELVEN_CHAIN_MAIL
                    || uarm->otyp == DARK_ELVEN_TUNIC))
            || (Race_if(PM_ELF) && uarm->otyp == ELVEN_CHAIN_MAIL)
            || (Race_if(PM_DWARF) && uarm->otyp == DWARVISH_CHAIN_MAIL))
            uac -= racial_bonus;
    }
    if (uarmc) {
        uac -= armor_bonus(uarmc);
        if ((Race_if(PM_ORC) && uarmc->otyp == ORCISH_CLOAK)
            || (Race_if(PM_ELF) && uarmc->otyp == ELVEN_CLOAK)
            || (Race_if(PM_DROW) && uarmc->otyp == DARK_ELVEN_CLOAK)
            || (Race_if(PM_DWARF) && uarmc->otyp == DWARVISH_CLOAK))
            uac -= racial_bonus;
    }
    if (uarmh) {
        uac -= armor_bonus(uarmh);
        if ((Race_if(PM_ORC) && uarmh->otyp == ORCISH_HELM)
            || (Race_if(PM_ELF) && uarmh->otyp == ELVEN_HELM)
            || (Race_if(PM_DROW) && uarmh->otyp == DARK_ELVEN_HELM)
            || (Race_if(PM_DWARF) && uarmh->otyp == DWARVISH_HELM))
            uac -= racial_bonus;
    }
    if (uarmf) {
        uac -= armor_bonus(uarmf);
        if ((Race_if(PM_ELF) && uarmf->otyp == ELVEN_BOOTS)
            || (Race_if(PM_DROW) && uarmf->otyp == DARK_ELVEN_BOOTS)
            || (Race_if(PM_DWARF) && uarmf->otyp == DWARVISH_BOOTS)
            || (Race_if(PM_ORC) && uarmf->otyp == ORCISH_BOOTS))
            uac -= racial_bonus;
    }
    if (uarms) {
        uac -= armor_bonus(uarms);
        /* knowing how to hold a shield is worth more than the shield;
           bracers cover less of the body, so they reward the skill less */
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
            || (Race_if(PM_DWARF) && uarms->otyp == DWARVISH_ROUNDSHIELD))
            uac -= racial_bonus;
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

    if (uarms && uarms->oartifact == ART_BRACERS_OF_THE_FIRST_CIRCLE) {
        /* the Druid quest artifact grants extra AC on top of
           what its base material provides, moreso if the
           wearer is actually a Druid */
        if (Role_if(PM_DRUID))
            uac -= 6;
        else
            uac -= 3;
    }

    if (uarm && uarm->oartifact == ART_ARMOR_OF_RETRIBUTION)
        uac -= 5;

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

    /* armor class from other sources */
    if (HProtection & INTRINSIC)
        uac -= u.ublessed;
    uac -= u.uspellprot;

    /* barkskin spell */
    if (Barkskin) {
        int bark_skill =
            (P_SKILL(spell_skilltype(SPE_BARKSKIN)) == P_EXPERT
             ? 10 : P_SKILL(spell_skilltype(SPE_BARKSKIN)) == P_SKILLED
                  ? 8 : P_SKILL(spell_skilltype(SPE_BARKSKIN)) == P_BASIC
                      ? 5 : 1);

        uac -= bark_skill;
    }

    /* stoneskin spell */
    if (Stoneskin) {
        int stone_skill =
            (P_SKILL(spell_skilltype(SPE_STONESKIN)) == P_EXPERT
             ? 15 : P_SKILL(spell_skilltype(SPE_STONESKIN)) == P_SKILLED
                  ? 12 : P_SKILL(spell_skilltype(SPE_STONESKIN)) == P_BASIC
                       ? 8 : 3);

        uac -= stone_skill;
    }

    /* tortle hiding in its shell */
    if (Hidinshell)
        uac -= 40;

    /* tortles gradually gain more protection as they grow up and
       their shells become tougher and thicker */
    tortle_ac = 0;
    if (Race_if(PM_TORTLE) && u.ulevel > 2)
        tortle_ac -= min((u.ulevel - 3) / 3 + 1, 10);

    /* Dexterity affects your base AC; positive is a penalty */
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

    /* wearing body armor made of certain rigid materials negates
       any beneficial dexterity bonus; so does being encumbered in
       any way. A low-dexterity penalty still applies */
    if ((uarm && (is_heavy_metallic(uarm)
                  || is_bone(uarm) || is_stone(uarm)
                  || is_wood(uarm) || is_glass(uarm)))
        || (near_capacity() >= SLT_ENCUMBER)) {
        if (dex_adjust_ac < 0)
            dex_adjust_ac = 0;
    }

    uac += dex_adjust_ac + tortle_ac;

    /* put a cap on armor class [5.0: was +127,-128, now reduced to +/- 99 */
    if (abs(uac) > AC_MAX)
        uac = sgn(uac) * AC_MAX;

    if (uac != u.uac) {
        u.uac = uac;
        disp.botl = TRUE;
#if 0
        /* these could conceivably be achieved out of order (by being near
           threshold and putting on +N dragon scale mail from bones, for
           instance), but if that happens, that's the order it happened;
           also, testing for these in the usual order would result in more
           record_achievement() attempts and rejects for duplication */
        if (u.uac <= -20)
            record_achievement(ACH_AC_20);
        else if (u.uac <= -10)
            record_achievement(ACH_AC_10);
        else if (u.uac <= 0)
            record_achievement(ACH_AC_00);
#endif
    }
}

void
glibr(void)
{
    struct obj *otmp;
    int xfl = 0;
    boolean leftfall, rightfall, wastwoweap = FALSE;
    const char *otherwep = 0, *thiswep, *which, *hand;

    leftfall = (uleft && !cursed(uleft, TRUE)
                && (!uwep || !(welded(uwep) && ULEFTY)
                    || !bimanual(uwep)));
    rightfall = (uright && !cursed(uright, TRUE)
                && (!uwep || !(welded(uwep) && URIGHTY)
                    || !bimanual(uwep)));
/*
    leftfall = (uleft && !uleft->cursed
                && (!uwep || !welded(uwep) || !bimanual(uwep)));
    rightfall = (uright && !uright->cursed && (!welded(uwep)));
*/

    if (!uarmg && (leftfall || rightfall) && !nolimbs(gy.youmonst.data)) {
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
            cmdq_clear(CQ_CANNED);
        }
        /* the left ring's removal can land the hero in lava, whose burn
           can use up the right ring */
        if (rightfall && uright) {
            otmp = uright;
            Ring_off(uright);
            dropx(otmp);
            cmdq_clear(CQ_CANNED);
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
        which = URIGHTY ? "left " : "right ";  /* text for the off hand */
        Your("%s %s%s from your %s%s.", otherwep, xfl ? "also " : "",
             otense(otmp, "slip"), which, hand);
        xfl++;
        wastwoweap = TRUE;
        setuswapwep((struct obj *) 0); /* clears u.twoweap */
        cmdq_clear(CQ_CANNED);
        if (canletgo(otmp, ""))
            dropx(otmp);
    }
    otmp = uwep;
    if (otmp && otmp->otyp != AKLYS && !hammer_tethers(otmp)
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
        if (bimanual(otmp)) {
            hand = makeplural(hand);
        } else if (wastwoweap) {
            /* preceding msg was about non-dominant hand */
            which = URIGHTY ? "right " : "left ";
        }
        pline("%s %s%s %s%s from your %s%s.",
              !strncmp(thiswep, "corpse", 6) ? "The" : "Your",
              otherwep ? "other " : "", thiswep, xfl ? "also " : "",
              otense(otmp, "slip"), which, hand);
        /* xfl++; */
        otmp->quan = savequan;
        setuwep((struct obj *) 0);
        cmdq_clear(CQ_CANNED);
        if (canletgo(otmp, ""))
            dropx(otmp);
    }
}

struct obj *
some_armor(struct monst *victim)
{
    struct obj *otmph, *otmp;

    otmph = (victim == &gy.youmonst) ? uarmc : which_armor(victim, W_ARMC);
    if (!otmph)
        otmph = (victim == &gy.youmonst) ? uarm : which_armor(victim, W_ARM);
    if (!otmph)
        otmph = (victim == &gy.youmonst) ? uarmu : which_armor(victim, W_ARMU);

    otmp = (victim == &gy.youmonst) ? uarmh : which_armor(victim, W_ARMH);
    if (otmp && (!otmph || !rn2(4)))
        otmph = otmp;
    otmp = (victim == &gy.youmonst) ? uarmg : which_armor(victim, W_ARMG);
    if (otmp && (!otmph || !rn2(4)))
        otmph = otmp;
    otmp = (victim == &gy.youmonst) ? uarmf : which_armor(victim, W_ARMF);
    if (otmp && (!otmph || !rn2(4)))
        otmph = otmp;
    otmp = (victim == &gy.youmonst) ? uarms : which_armor(victim, W_ARMS);
    if (otmp && (!otmph || !rn2(4)))
        otmph = otmp;
    if (victim != &gy.youmonst && (otmp = which_armor(victim, W_BARDING)) != 0
        && (!otmph || !rn2(4)))
        otmph = otmp;
    if (victim == &gy.youmonst && u.usteed
        && (otmp = which_armor(u.usteed, W_BARDING)) != 0
        && (!otmph || !rn2(4)))
        otmph = otmp;
    return otmph;
}

/* used for praying to check and fix levitation trouble */
struct obj *
stuck_ring(struct obj *ring, int otyp)
{
    if (ring != uleft && ring != uright) {
        impossible("stuck_ring: neither left nor right?");
        return (struct obj *) 0;
    }

    if (ring && ring->otyp == otyp) {
        /* reasons ring can't be removed match those checked by select_off();
           limbless case has extra checks because ordinarily it's temporary */
        if (nolimbs(gy.youmonst.data) && uamul
            && uamul->otyp == AMULET_OF_UNCHANGING && cursed(uamul, TRUE))
            return uamul;
        if (welded(uwep) && ((ring == RING_ON_PRIMARY) || bimanual(uwep)))
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
unchanger(void)
{
    if (uamul && uamul->otyp == AMULET_OF_UNCHANGING)
        return uamul;
    return 0;
}

staticfn
int
select_off(struct obj *otmp)
{
    struct obj *why;
    char buf[BUFSZ];

    if (!otmp)
        return 0;
    *buf = '\0'; /* lint suppression */

    /* special ring checks */
    if (otmp == uright || otmp == uleft) {
        struct obj glibdummy;

        if (nolimbs(gy.youmonst.data)) {
            pline_The("ring is stuck.");
            return 0;
        }
        glibdummy = cg.zeroobj;
        why = 0; /* the item which prevents ring removal */
        if (welded(uwep) && ((otmp == RING_ON_PRIMARY) || bimanual(uwep))) {
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
        } else if (is_art(uarmg, ART_HAND_OF_VECNA)) {
            pline("%s has merged with your left %s and cannot be removed.",
                  The(xname(uarmg)), body_part(ARM));
            return 0;
        }
        if (better_not_take_that_off(otmp))
            return 0;
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
                   && uarmu->oartifact == ART_STRIPED_SHIRT_OF_LIBERATION) {
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
        svc.context.takeoff.mask |= WORN_ARMOR;
    else if (otmp == uarmc)
        svc.context.takeoff.mask |= WORN_CLOAK;
    else if (otmp == uarmf)
        svc.context.takeoff.mask |= WORN_BOOTS;
    else if (otmp == uarmg)
        svc.context.takeoff.mask |= WORN_GLOVES;
    else if (otmp == uarmh)
        svc.context.takeoff.mask |= WORN_HELMET;
    else if (otmp == uarms)
        svc.context.takeoff.mask |= WORN_SHIELD;
    else if (otmp == uarmu)
        svc.context.takeoff.mask |= WORN_SHIRT;
    else if (otmp == uleft)
        svc.context.takeoff.mask |= LEFT_RING;
    else if (otmp == uright)
        svc.context.takeoff.mask |= RIGHT_RING;
    else if (otmp == uamul)
        svc.context.takeoff.mask |= WORN_AMUL;
    else if (otmp == ublindf)
        svc.context.takeoff.mask |= WORN_BLINDF;
    else if (otmp == uwep)
        svc.context.takeoff.mask |= W_WEP;
    else if (otmp == uswapwep)
        svc.context.takeoff.mask |= W_SWAPWEP;
    else if (otmp == uquiver)
        svc.context.takeoff.mask |= W_QUIVER;

    else
        impossible("select_off: %s???", doname(otmp));

    return 0;
}

staticfn struct obj *
do_takeoff(void)
{
    struct obj *otmp = (struct obj *) 0;
    boolean was_twoweap = u.twoweap;
    struct takeoff_info *doff = &svc.context.takeoff;

    svc.context.takeoff.mask |= I_SPECIAL; /* set flag for cancel_doff() */
    if (doff->what == W_WEP) {
        if (!cursed(uwep, FALSE)) {
            setuwep((struct obj *) 0);
            if (was_twoweap)
                You("are no longer wielding either weapon.");
            else
                You("are %s.", empty_handed());
        }
    } else if (doff->what == W_SWAPWEP) {
        setuswapwep((struct obj *) 0);
        You("%sno longer %s.", was_twoweap ? "are " : "",
            was_twoweap ? "wielding two weapons at once"
                        : "have a second weapon readied");
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
    svc.context.takeoff.mask &= ~I_SPECIAL; /* clear cancel_doff() flag */

    return otmp;
}

/* occupation callback for 'A' */
staticfn int
take_off(void)
{
    int i;
    struct obj *otmp;
    struct takeoff_info *doff = &svc.context.takeoff;

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

staticfn boolean
better_not_take_that_off(struct obj *otmp)
{
    struct obj *corpse = carrying_stoning_corpse();
    char buf[BUFSZ];

    /* u_safe_from_fatal_corpse() with
       (st_corpse | st_petrifies | st_resists) instead of
       (st_corpse | st_petrifies)
       would also check for no stoning resistance before
       bothering to prompt, but losing stoning resistance
       later, without the gloves on could prove dangerous,
       so we won't factor that in */
    if (corpse
        && !u_safe_from_fatal_corpse(corpse, st_corpse | st_petrifies)) {
        Snprintf(buf, sizeof buf,
            "Take off your %s despite carrying a dead %s?",
                 gloves_simple_name(otmp), obj_pmname(corpse));
        return (paranoid_ynq(TRUE, buf, FALSE) != 'y');
    }
    return FALSE;
}

/* clear saved context to avoid inappropriate resumption of interrupted 'A' */
void
reset_remarm(void)
{
    svc.context.takeoff.what = svc.context.takeoff.mask = 0L;
    svc.context.takeoff.disrobing[0] = '\0';
}

/* the #takeoffall command -- remove multiple worn items */
int
doddoremarm(void)
{
    int result = 0;

    if (svc.context.takeoff.what || svc.context.takeoff.mask) {
        You("continue %s.", svc.context.takeoff.disrobing);
        set_occupation(take_off, svc.context.takeoff.disrobing, 0);
        return ECMD_OK;
    } else if (!uwep && !uswapwep && !uquiver && !uamul && !ublindf
               && !uleft && !uright && !wearing_armor()) {
        You("are not wearing anything.");
        return ECMD_OK;
    } else if (Hidinshell) {
        You_cant("take off worn items while hiding in your shell.");
        return ECMD_OK;
    } else if (druid_form) {
        You_cant("take off worn items while in wildshape.");
        return ECMD_OK;
    } else if (vampire_form) {
        You_cant("take off worn items while shapechanged.");
        return ECMD_OK;
    }

    add_valid_menu_class(0); /* reset */
    if (flags.menu_style != MENU_TRADITIONAL
        || (result = ggetobj("take off", select_off, 0, FALSE,
                             (unsigned *) 0)) < -1)
        (void) menu_remarm(result);

    if (svc.context.takeoff.mask) {
        (void) strncpy(svc.context.takeoff.disrobing,
                       (((svc.context.takeoff.mask & ~W_WEAPONS) != 0)
                        /* default activity for armor and/or accessories,
                           possibly combined with weapons */
                        ? "disrobing"
                        /* specific activity when handling weapons only */
                        : "disarming"), CONTEXTVERBSZ);
        (void) take_off();
    }
    /* The time to perform the command is already completely accounted for
     * in take_off(); if we return 1, that would add an extra turn to each
     * disrobe.
     */
    return ECMD_OK;
}

/* #altunwield - just unwield alternate weapon, item-action '-' when picking
   uswapwep from context-sensitive inventory */
int
remarm_swapwep(void)
{
    struct _cmd_queue cq, *cmdq;
    unsigned oldbknown;

    if ((cmdq = cmdq_pop()) != 0) {
        /* '-' uswapwep item-action picked from context-sensitive invent */
        cq = *cmdq;
        free(cmdq);
    } else {
        cq.typ = CMDQ_KEY;
        cq.key = '\0'; /* something other than '-' */
    }
    if (cq.typ != CMDQ_KEY || cq.key != '-' || !uswapwep)
        return ECMD_FAIL;

    oldbknown = uswapwep->bknown; /* when deciding whether this command
                                   * has done something that takes time,
                                   * behave as if a cursed secondary weapon
                                   * can't be unwielded even though things
                                   * don't work that way... */
    reset_remarm();
    svc.context.takeoff.what = svc.context.takeoff.mask = W_SWAPWEP;
    (void) do_takeoff();
    return (!uswapwep || uswapwep->bknown != oldbknown) ? ECMD_TIME : ECMD_OK;
}

staticfn int
menu_remarm(int retry)
{
    int n, i = 0;
    menu_item *pick_list;
    boolean all_worn_categories = TRUE;

    if (retry) {
        all_worn_categories = (retry == -2);
    } else if (flags.menu_style == MENU_FULL) {
        all_worn_categories = FALSE;
        n = query_category("What type of things do you want to take off?",
                           gi.invent, (WORN_TYPES | ALL_TYPES
                                    | UNPAID_TYPES | UNIDED_TYPES
                                    | BUCX_TYPES),
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
    if (menu_class_present('u') || menu_class_present('I')
        || menu_class_present('B') || menu_class_present('U')
        || menu_class_present('C') || menu_class_present('X'))
        all_worn_categories = FALSE;

    /* INCLUDE_VENOM: is_worn() treats W_QUIVER as worn, and the quiver
       accepts ALL_CLASSES, so a quivered venom-class item passes the
       count loop but pack-iteration drops it (VENOM_CLASS not in
       def_inv_order[]); without this flag the menu builds empty and
       curses_display_nhmenu raises an impossible */
    n = query_objlist("What do you want to take off?", &gi.invent,
                      (SIGNAL_NOMENU | USE_INVLET | INVORDER_SORT
                       | INCLUDE_VENOM),
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

/* take off the specific worn object and if it still exists after that,
   destroy it (taking off the item might already destroy it by dunking
   hero into lava) */
void
wornarm_destroyed(struct obj *wornarm)
{
    struct obj *invobj, *nextobj;
    unsigned wornoid = wornarm->o_id;

    /* cancel_don() resets 'afternmv' when appropriate but doesn't reset
       uarmc/uarm/&c so doing this now won't interfere with the tests in
       'if (wornarm==uarmc) ... else if (wornarm==uarm) ... else ...' */
    if (donning(wornarm))
        cancel_don();

    if (wornarm == uarmc)
        (void) Cloak_off();
    else if (wornarm == uarm)
        (void) Armor_off();
    else if (wornarm == uarmu)
        (void) Shirt_off();
    else if (wornarm == uarmh)
        (void) Helmet_off();
    else if (wornarm == uarmg)
        (void) Gloves_off();
    else if (wornarm == uarmf)
        (void) Boots_off();
    else if (wornarm == uarms)
        (void) Shield_off();

    /* 'wornarm' might be destroyed as a side-effect of xxx_off() so
       using carried() to check wornarm->where==OBJ_INVENT is not viable;
       scan invent instead; if already freed it shouldn't be possible to
       have re-used the stale memory for a new item yet but verify o_id
       just in case */
    for (invobj = gi.invent; invobj; invobj = nextobj) {
        nextobj = invobj->nobj;
        if (invobj == wornarm && invobj->o_id == wornoid) {
            useup(wornarm);
            break;
        }
    }
}

/*
 * returns impacted armor with its in_use bit set,
 * or Null. *resisted is updated to reflect whether
 * it resisted or not */
staticfn struct obj *
maybe_destroy_armor(struct obj *armor, struct obj *atmp, boolean *resisted)
{
    if ((armor != 0) && (!atmp || atmp == armor)) {
        if (armor->otyp == CRYSTAL_PLATE_MAIL) {
            /* crystal plate mail is never destroyed by disintegration,
               but it also cannot be sacrificed to spare its wearer */
            *resisted = FALSE;
            return (struct obj *) 0;
        }
        if ((*resisted = obj_resists(armor, 0, 90)) == FALSE) {
            armor->in_use = 1;
            return armor;
        }
    }
    return (struct obj *) 0;
}

/* hit by destroy armor scroll/black dragon breath */
int
disintegrate_arm(struct obj *atmp)
{
    struct obj *otmp = (struct obj *) 0;
    boolean losing_gloves = FALSE, resisted = FALSE,
            resistedc = FALSE, resistedsuit = FALSE;
    /*
     * Note: if the cloak resisted, then the suit or shirt underneath
     * wouldn't be impacted either. Likewise, if the suit resisted, the
     * shirt underneath wouldn't be impacted. Since there are no artifact
     * cloaks or suits right now, this is unlikely to come into effect,
     * but it should behave appropriately if/when the situation changes.
     */

    if ((otmp = maybe_destroy_armor(uarmc, atmp, &resistedc)) != 0) {
        /* for gold/chromatic DS, we don't want Cloak_off() to report
           that it stops shining _after_ we've been told that it is
           destroyed */
        if (otmp->lamplit)
            end_burn(otmp, FALSE);
        urgent_pline("Your %s crumbles and turns to dust!",
                     /* cloak/robe/apron/smock (ID'd apron)/wrapping/
                        set of dragon scales */
                     cloak_simple_name(otmp));
    } else if (!resistedc
             && (otmp = maybe_destroy_armor(uarm, atmp, &resistedsuit)) != 0) {
        const char *suit = suit_simple_name(otmp);

        /* for gold DSM, we don't want Armor_off() to report that it
           stops shining _after_ we've been told that it is destroyed */
        if (otmp->lamplit)
            end_burn(otmp, FALSE);
        urgent_pline("Your %s %s to dust and %s to the %s!",
                     /* suit might be "dragon scales" so vtense() is needed */
                     suit, vtense(suit, "turn"), vtense(suit, "fall"),
                     surface(u.ux, u.uy));
    } else if (!resistedc && !resistedsuit
             && (otmp = maybe_destroy_armor(uarmu, atmp, &resisted)) != 0) {
        urgent_pline("Your %s crumbles into tiny threads and falls apart!",
                     shirt_simple_name(otmp)); /* always "shirt" */
    } else if ((otmp = maybe_destroy_armor(uarmh, atmp, &resisted)) != 0) {
        urgent_pline("Your %s turns to dust and is blown away!",
                     helm_simple_name(otmp)); /* "helm" or "hat" */
    } else if (uarmg && is_art(uarmg, ART_HAND_OF_VECNA)
               && (!atmp || atmp == uarmg)) {
        /* no feedback, as we're pretending the Hand of Vecna isn't
           worn but a part of your body; nothing else is destroyed
           in its place */
        return 0;
    } else if ((otmp = maybe_destroy_armor(uarmg, atmp, &resisted)) != 0) {
        urgent_pline("Your %s vanish!", gloves_simple_name(otmp));
        losing_gloves = TRUE;
    } else if ((otmp = maybe_destroy_armor(uarmf, atmp, &resisted)) != 0) {
        urgent_pline("Your %s disintegrate!", boots_simple_name(otmp));
    } else if ((otmp = maybe_destroy_armor(uarms, atmp, &resisted)) != 0) {
        const char *shnm = shield_simple_name(otmp); /* bracers or shield */

        /* for a shield of light, we don't want Shield_off() to report
           that it stops shining _after_ we've been told that it is
           destroyed */
        if (otmp->lamplit)
            end_burn(otmp, FALSE);
        urgent_pline("Your %s %s away!", shnm, vtense(shnm, "crumble"));
    } else if (u.usteed
               && (otmp = which_armor(u.usteed, W_BARDING)) != 0
               /* don't route barding through maybe_destroy_armor() (at
                  least for now) -- we want it to be an invalid target
                  when atmp is Null, so that it can only be destroyed if
                  specifically targeted */
               && otmp == atmp
               && !obj_resists(otmp, 0, 90)) {
        otmp->in_use = TRUE;
        urgent_pline("%s crumbles to pieces!", Yname2(otmp));
        m_useup(u.usteed, otmp);
        /* barding isn't hero-worn gear, so the shared wornarm_destroyed()
           cleanup below must not run for it */
        stop_occupation();
        return 1;
    } else {
        return 0; /* could not destroy anything */
    }

    /* cancel_don() if applicable, Cloak_off()/Armor_off()/&c, and useup() */
    wornarm_destroyed(otmp);
    /* glove loss means wielded weapon will be touched */
    if (losing_gloves)
        selftouch("You");

    stop_occupation();
    return 1;
}

/* return ERODE_foo erosion type which can apply to object */
staticfn int
obj_erode_type(struct obj *otmp)
{
    if (is_flammable(otmp))
        return ERODE_BURN;
    else if (is_rustprone(otmp))
        return ERODE_RUST;
    else if (is_glass(otmp))
        return ERODE_FRACTURE;
    else if (is_rottable(otmp))
        return ERODE_ROT;
    else if (is_corrodeable(otmp))
        return ERODE_CORRODE;
    else if (is_supermaterial(otmp))
        return ERODE_DETERIORATE;
    return ERODE_NONE;
}

/* erode a number of worn armor(s).
   if the armor is hit when max eroded, destroys it. */
int
destroy_arm(void)
{
    struct obj *armors[7] = { NULL };
    struct obj *otmp;
    int i, idx = 0, hits = rn2(4) + 1;
    int ret = 0;

    /* gather worn armor; include non-erodeable ones */
    if (uarm) armors[idx++] = uarm;
    if (uarmc) armors[idx++] = uarmc;
    if (uarmh) armors[idx++] = uarmh;
    if (uarms) armors[idx++] = uarms;
    if (uarmg) armors[idx++] = uarmg;
    if (uarmf) armors[idx++] = uarmf;
    if (uarmu) armors[idx++] = uarmu;
    if (!idx)
        return 0;

    for (i = 0; i < hits; i++) {
        otmp = armors[rn2(idx)];

        if (erosion_matters(otmp)
            && (is_damageable(otmp) || is_supermaterial(otmp))
            && !otmp->oerodeproof) {
            int erosion = obj_erode_type(otmp);

            if (erosion != ERODE_NONE) {
                int r = erode_obj(otmp, xname(otmp), erosion, EF_PAY|EF_DESTROY);

                if (r != ER_NOTHING)
                    ret = 1;
                if (r == ER_DESTROYED)
                    break;
            }
        }
    }

    if (ret)
        stop_occupation();
    return ret;
}

void
adj_abon(struct obj *otmp, schar delta)
{
    if (uarmg && uarmg == otmp && otmp->otyp == GAUNTLETS_OF_DEXTERITY) {
        if (delta) {
            makeknown(uarmg->otyp);
            ABON(A_DEX) += (delta);
        }
        disp.botl = TRUE;
    }
    if (uarmh && uarmh == otmp && otmp->otyp == HELM_OF_BRILLIANCE) {
        if (delta) {
            makeknown(uarmh->otyp);
            ABON(A_INT) += (delta);
            ABON(A_WIS) += (delta);
        }
        disp.botl = TRUE;
    }
    if ((otmp->oprops & ITEM_EXCEL) && (otmp->owornmask & W_ARMOR)
        && carried(otmp)) {
        if (delta) {
            int which = A_CHA, old_attrib = ACURR(which);

            ABON(which) += (delta);
            if (old_attrib != ACURR(which))
                otmp->oprops_known |= ITEM_EXCEL;
            set_moreluck();
        }
        disp.botl = TRUE;
    }
}

/* decide whether a worn item is covered up by some other worn item,
   used for dipping into liquid and applying grease and takeoff_ok();
   some criteria are different than select_off()'s */
boolean
inaccessible_equipment(
    struct obj *obj,
    const char *verb, /* "dip" or "grease", or null to avoid messages */
    boolean only_if_known_cursed) /* ignore covering unless it is known to
                                   * be cursed */
{
    static NEARDATA const char need_to_take_off_outer_armor[] =
        "need to take off %s to %s %s.";
    char buf[BUFSZ];
    boolean anycovering = !only_if_known_cursed; /* more comprehensible... */
#define BLOCKSACCESS(x) (anycovering || ((x)->cursed && (x)->bknown))

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

            /* two long object names can't both fit in buf[] */
            Snprintf(buf, sizeof buf, "%s%s%s",
                     uarmc ? yname(uarmc) : "",
                     (uarm && uarmc) ? " and " : "",
                     !uarm ? "" : sameprefix ? xname(uarm) : yname(uarm));
            You(need_to_take_off_outer_armor, buf, verb, yname(obj));
        }
        return TRUE;
    }
    /* check for ring covered by gloves; the Hand of Vecna covers only
       the arm it merged with, leaving the ring fingers free */
    if ((obj == uleft || obj == uright) && uarmg && BLOCKSACCESS(uarmg)
        && !is_art(uarmg, ART_HAND_OF_VECNA)) {
        if (verb) {
            Strcpy(buf, yname(uarmg));
            You(need_to_take_off_outer_armor, buf, verb, yname(obj));
        }
        return TRUE;
    }
    /* item is not inaccessible */
    return FALSE;

#undef BLOCKSACCESS
}

/* not a getobj callback - unifies code among the other 4 getobj callbacks */
staticfn int
equip_ok(struct obj *obj, boolean removing, boolean accessory)
{
    boolean is_worn, armor;
    long dummymask = 0;

    if (!obj)
        return GETOBJ_EXCLUDE;

    /* stone to flesh's meat armor is food worn as armor */
    armor = (obj->oclass == ARMOR_CLASS || is_meat_armor(obj));

    /* ignore for putting on if already worn, or removing if not worn */
    is_worn = ((obj->owornmask & (W_ARMOR | W_ACCESSORY)) != 0);
    if (removing ^ is_worn)
        return GETOBJ_EXCLUDE_INACCESS;

    /* exclude most object classes outright */
    if (!armor && obj->oclass != RING_CLASS
        && obj->oclass != AMULET_CLASS) {
        /* ... except for a few wearable exceptions outside these classes */
        if (obj->otyp != MEAT_RING && obj->otyp != BLINDFOLD
            && obj->otyp != TOWEL && obj->otyp != LENSES
            && obj->otyp != GOGGLES)
            return GETOBJ_EXCLUDE;
    }

    /* armor with 'P' or 'R' or accessory with 'W' or 'T' */
    if (accessory ^ !armor)
        return GETOBJ_DOWNPLAY;

    /* armor we can't wear, e.g. from polyform */
    if (armor && !removing
        && !canwearobj(obj, &dummymask, FALSE))
        return GETOBJ_DOWNPLAY;

    /* Possible extension: downplay items (both accessories and armor) which
     * can't be worn because the slot is filled with something else. */

    /* removing inaccessible equipment */
    if (removing && !gi.item_action_in_progress) {
        if (inaccessible_equipment(obj, (const char *) 0,
                                   (obj->oclass == RING_CLASS)))
            return GETOBJ_EXCLUDE_INACCESS;
    }

    /* all good to go */
    return GETOBJ_SUGGEST;
}

/* getobj callback for P command */
staticfn int
puton_ok(struct obj *obj)
{
    return equip_ok(obj, FALSE, TRUE);
}

/* getobj callback for R command */
staticfn int
remove_ok(struct obj *obj)
{
    return equip_ok(obj, TRUE, TRUE);
}

/* getobj callback for W command */
staticfn int
wear_ok(struct obj *obj)
{
    return equip_ok(obj, FALSE, FALSE);
}

/* getobj callback for T command */
staticfn int
takeoff_ok(struct obj *obj)
{
    return equip_ok(obj, TRUE, FALSE);
}

/* getobj callback for blessed destroy armor.
   suggest any worn armor, even if covered by other armor */
int
any_worn_armor_ok(struct obj *obj)
{
    if (obj && (obj->owornmask & W_ARMOR))
        return GETOBJ_SUGGEST;
    return GETOBJ_EXCLUDE;
}

/* number of armor pieces worn by hero */
int
count_worn_armor(void)
{
    int ret = 0;

    if (uarm) ret++;
    if (uarmc) ret++;
    if (uarmh) ret++;
    if (uarms) ret++;
    if (uarmg) ret++;
    if (uarmf) ret++;
    if (uarmu) ret++;

    return ret;
}

/* getobj callback for choose_worn_armor(): worn armor is the likely
   choice; unworn armor stays pickable so the caller can explain that
   it must be worn; armor merged into the hero's skin keeps its slot
   bit but is not a choice: count_worn_armor() does not count it, and
   enchanting it would merge it again or remove it from the skin */
staticfn int
worn_armor_ok(struct obj *obj)
{
    if (!obj || obj->oclass != ARMOR_CLASS || obj == uskin)
        return GETOBJ_EXCLUDE;
    if (obj->owornmask & W_ARMOR)
        return GETOBJ_SUGGEST;
    return GETOBJ_DOWNPLAY;
}

/* let the hero choose the armor that a known scroll of enchant armor or
   a skilled repair armor spell works on: a rider is offered the steed's
   barding first, and armor that is not worn can be picked, to be told
   it must be worn; returns the hero's armor, the steed's barding (not
   in invent) or null when nothing is chosen */
struct obj *
choose_worn_armor(const char *word, const char *verb)
{
    struct obj *otmp = (struct obj *) 0;

    if (u.usteed && (otmp = which_armor(u.usteed, W_BARDING)) != 0) {
        char qbuf[QBUFSZ];

        Snprintf(qbuf, sizeof qbuf, "%s %s ", verb,
                 s_suffix(y_monnam(u.usteed)));
        (void) upstart(qbuf);
        /* a long barding name gets its simple name instead */
        (void) safe_qbuf(qbuf, qbuf, "?", otmp, xname, simpleonames,
                         "barding");
        /* YN() keeps the answer out of the do-again queue: if the steed
           is gone when ^A replays this, the queued 'n' would otherwise
           be read by getobj() as an inventory letter */
        if (YN(qbuf) == 'n')
            otmp = (struct obj *) 0;
    }
    if (!otmp) {
        /* unworn armor can be picked, to be told it must be worn, but
           it is no reason to ask */
        if (!count_worn_armor()) {
            struct obj *o;

            for (o = gi.invent; o; o = o->nobj)
                if (o->oclass == ARMOR_CLASS)
                    break;
            You("don't have anything %sto %s.", o ? "else " : "", word);
        } else {
            otmp = getobj(word, worn_armor_ok, GETOBJ_NOFLAGS);
            while (otmp && !(otmp->owornmask & W_ARMOR)) {
                You("cannot %s armor that is not worn.", verb);
                otmp = getobj(word, worn_armor_ok, GETOBJ_NOFLAGS);
            }
        }
    }
    return otmp;
}

/*do_wear.c*/
