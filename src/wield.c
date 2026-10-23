/* NetHack 5.0	wield.c	$NHDT-Date: 1781973073 2026/06/20 16:31:13 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.124 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2009. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* KMH -- Differences between the three weapon slots.
 *
 * The main weapon (uwep):
 * 1.  Is filled by the (w)ield command.
 * 2.  Can be filled with any type of item.
 * 3.  May be carried in one or both hands.
 * 4.  Is used as the melee weapon and as the launcher for
 *     ammunition.
 * 5.  Only conveys intrinsics when it is a weapon, weapon-tool,
 *     or artifact.
 * 6.  Certain cursed items will weld to the hand and cannot be
 *     unwielded or dropped.  See erodeable_wep() and will_weld()
 *     below for the list of which items apply.
 *
 * The secondary weapon (uswapwep):
 * 1.  Is filled by the e(x)change command, which swaps this slot
 *     with the main weapon.  If the "pushweapon" option is set,
 *     the (w)ield command will also store the old weapon in the
 *     secondary slot.
 * 2.  Can be filled with anything that will fit in the main weapon
 *     slot; that is, any type of item.
 * 3.  Is usually NOT considered to be carried in the hands.
 *     That would force too many checks among the main weapon,
 *     second weapon, shield, gloves, and rings; and it would
 *     further be complicated by bimanual weapons.  A special
 *     exception is made for two-weapon combat.
 * 4.  Is used as the second weapon for two-weapon combat, and as
 *     a convenience to swap with the main weapon.
 * 5.  Never conveys intrinsics.
 * 6.  Cursed items never weld (see #3 for reasons), but they also
 *     prevent two-weapon combat.
 *
 * The quiver (uquiver):
 * 1.  Is filled by the (Q)uiver command.
 * 2.  Can be filled with any type of item.
 * 3.  Is considered to be carried in a special part of the pack.
 * 4.  Is used as the item to throw with the (f)ire command.
 *     This is a convenience over the normal (t)hrow command.
 * 5.  Never conveys intrinsics.
 * 6.  Cursed items never weld; their effect is handled by the normal
 *     throwing code.
 * 7.  The autoquiver option will fill it with something deemed
 *     suitable if (f)ire is used when it's empty.
 *
 * No item may be in more than one of these slots.
 */

staticfn boolean cant_wield_corpse(struct obj *) NONNULLARG1;
staticfn int ready_weapon(struct obj *) NO_NNARGS;
staticfn int ready_ok(struct obj *) NO_NNARGS;
staticfn int wield_ok(struct obj *) NO_NNARGS;
staticfn void finish_splitting(struct obj *);

/* used by will_weld() */
/* probably should be renamed */
#define erodeable_wep(optr)                             \
    ((optr)->oclass == WEAPON_CLASS || is_weptool(optr) \
     || (optr)->otyp == HEAVY_IRON_BALL || (optr)->otyp == IRON_CHAIN)

/* used by welded(), and also while wielding */
#define will_weld(optr) \
    ((optr)->cursed && (erodeable_wep(optr) || (optr)->otyp == TIN_OPENER))
/* Infidels are immune to curses */
#define will_weld_to_you(optr) (will_weld(optr) && !Role_if(PM_INFIDEL))

/* to dual-wield, 'obj' must be a weapon or a weapon-tool, and not a bow
   or arrow or missile (dart, shuriken, boomerang), so not matching the
   one-handed weapons which yield "you begin bashing" if used for melee;
   empty hands and two-handed weapons have to be handled separately */
/* artifacts that influence the bottom-line status display when wielded
   (or held in offhand during two-weapon combat) */
#define is_botl_artifact(o)                  \
    (is_art((o), ART_OGRESMASHER)            \
     || is_art((o), ART_GIANTSLAYER)         \
     || is_art((o), ART_HARBINGER)           \
     || is_art((o), ART_SWORD_OF_KAS))

#define TWOWEAPOK(obj) \
    (((obj)->oclass == WEAPON_CLASS)                            \
     ? !(is_launcher(obj) || is_ammo(obj) || is_missile(obj))   \
     : is_weptool(obj))

static const char
    are_no_longer_twoweap[] = "are no longer using two weapons at once",
    can_no_longer_twoweap[] = "can no longer wield two weapons at once";

/* exported TWOWEAPOK for callers that must not re-engage two-weapon
   combat with an illegal weapon (poly_obj reseat) */
boolean
twoweapon_ok(struct obj *obj)
{
    return TWOWEAPOK(obj);
}

/*** Functions that place a given item in a slot ***/
/* Proper usage includes:
 * 1.  Initializing the slot during character generation or a
 *     restore.
 * 2.  Setting the slot due to a player's actions.
 * 3.  If one of the objects in the slot is split off, these
 *     functions can be used to put the remainder back in the slot.
 * 4.  Putting an item that was thrown and returned back into the slot.
 * 5.  Emptying the slot, by passing a null object.  NEVER pass
 *     cg.zeroobj!
 *
 * If the item is being moved from another slot, it is the caller's
 * responsibility to handle that.  It's also the caller's responsibility
 * to print the appropriate messages.
 */
void
setuwep(struct obj *obj)
{
    struct obj *olduwep = uwep;

    if (obj == uwep)
        return; /* necessary to not set gu.unweapon */
    setworn(obj, W_WEP);
    /* handle status artifacts before Sunsword; even though they can't be
       happening at the same time, botl flag update should come before
       pline message */
    if (uwep == obj
        && ((obj && is_botl_artifact(obj))
            || (olduwep && is_botl_artifact(olduwep))))
        disp.botl = TRUE; /* gaining or losing an attribute effect */
    /* This message isn't printed in the caller because it happens
     * *whenever* Sunsword is unwielded, from whatever cause. */
    if (uwep == obj && artifact_light(olduwep) && olduwep->lamplit) {
        end_burn(olduwep, FALSE);
        if (!Blind)
            pline("%s %s.", Tobjnam(olduwep, "stop"),
                  (is_art(olduwep, ART_STAFF_OF_THE_ARCHMAGI)
                   && !Upolyd && Race_if(PM_DROW))
                      ? "its aura of darkness" : "shining");
    }
    /* wielding or unwielding a weapon of excellence changes Luck */
    if (uwep == obj && obj && (obj->oprops & ITEM_EXCEL)) {
        obj->oprops_known |= ITEM_EXCEL;
        set_moreluck();
        disp.botl = TRUE;
        update_inventory();
    }
    if (olduwep && (olduwep->oprops & ITEM_EXCEL)) {
        olduwep->oprops_known |= ITEM_EXCEL;
        set_moreluck();
        disp.botl = TRUE;
        update_inventory();
    }
    /* the Wand of Orcus curses itself in the hand that wields it; a
       restore only reseats the slot, so leave its state alone then */
    if (uwep == obj && is_art(obj, ART_WAND_OF_ORCUS)
        && !program_state.restoring) {
        curse(obj);
        update_inventory();
    }
    /* Note: Explicitly wielding a pick-axe will not give a "bashing"
     * message.  Wielding one via 'a'pplying it will.
     * 3.2.2:  Wielding arbitrary objects will give bashing message too.
     */
    if (obj) {
        gu.unweapon = (obj->oclass == WEAPON_CLASS)
                       ? is_launcher(obj) || is_ammo(obj) || is_missile(obj)
                             || (is_pole(obj) && !u.usteed
                                 && !Race_if(PM_CENTAUR)
                                 && !is_art(obj, ART_SNICKERSNEE))
                       : !is_weptool(obj) && !is_wet_towel(obj);
    } else
        gu.unweapon = TRUE; /* for "bare hands" message */
}

staticfn boolean
cant_wield_corpse(struct obj *obj)
{
    char kbuf[BUFSZ];

    if (uarmg || obj->otyp != CORPSE || !touch_petrifies(&mons[obj->corpsenm])
        || Stone_resistance)
        return FALSE;

    /* Prevent wielding cockatrice when not wearing gloves --KAA */
    You("wield %s in your bare %s.",
        corpse_xname(obj, (const char *) 0, CXN_PFX_THE),
        makeplural(body_part(HAND)));
    Sprintf(kbuf, "wielding %s bare-handed", killer_xname(obj));
    instapetrify(kbuf);
    return TRUE;
}

/* description of hands when not wielding anything; also used
   by #seeweapon (')'), #attributes (^X), and #takeoffall ('A') */
const char *
empty_handed(void)
{
    return uarmg ? "empty handed" /* gloves imply hands */
           : humanoid(gy.youmonst.data)
             /* hands but no weapon and no gloves */
             ? "bare handed"
               /* alternate phrasing for paws or lack of hands */
               : "not wielding anything";
}

staticfn int
ready_weapon(struct obj *wep)
{
    /* Separated function so swapping works easily */
    int res = ECMD_OK;
    boolean was_twoweap = u.twoweap, had_wep = (uwep != 0);

    if (!wep) {
        /* No weapon */
        if (uwep) {
            You("are %s.", empty_handed());
            setuwep((struct obj *) 0);
            res = ECMD_TIME;
        } else
            You("are already %s.", empty_handed());
    } else if (wep->otyp == CORPSE && cant_wield_corpse(wep)) {
        /* hero must have been life-saved to get here; use a turn */
        res = ECMD_TIME; /* corpse won't be wielded */
    } else if (uarms && !is_bracer(uarms) && bimanual(wep)) {
        You("cannot wield a two-handed %s while wearing a shield.",
            is_sword(wep) ? "sword" : wep->otyp == BATTLE_AXE ? "axe"
                                                              : "weapon");
        res = ECMD_FAIL;
    } else if (!retouch_object(&wep, will_touch_skin(W_WEP), FALSE)) {
        res = ECMD_TIME; /* takes a turn even though it doesn't get wielded */
    } else {
        /* Weapon WILL be wielded after this point */
        res = ECMD_TIME;
        /* the Wand of Orcus curses itself in setuwep() below, so it
           welds even when it isn't cursed yet */
        if (will_weld_to_you(wep)
            || (is_art(wep, ART_WAND_OF_ORCUS) && !Role_if(PM_INFIDEL))) {
            const char *tmp = xname(wep), *thestr = "The ";

            if (strncmp(tmp, thestr, 4) && !strncmp(The(tmp), thestr, 4))
                tmp = thestr;
            else
                tmp = "";
            pline("%s%s %s to your %s%s!", tmp, aobjnam(wep, "weld"),
                  (wep->quan == 1L) ? "itself" : "themselves", /* a3 */
                  bimanual(wep) ? "" :
                      (URIGHTY ? "dominant right " : "dominant left "),
                  bimanual(wep) ? (const char *) makeplural(body_part(HAND))
                                : body_part(HAND));
            set_bknown(wep, 1);
        } else {
            /* The message must be printed before setuwep (since
             * you might die and be revived from changing weapons),
             * and the message must be before the death message and
             * Lifesaved rewielding.  Yet we want the message to
             * say "weapon in hand", thus this kludge.
             * [That comment is obsolete.  It dates from the days (3.0)
             * when unwielding Firebrand could cause hero to be burned
             * to death in Hell due to loss of fire resistance.
             * "Lifesaved re-wielding or re-wearing" is ancient history.]
             */
            long dummy = wep->owornmask;

            wep->owornmask |= W_WEP;
            if (wep->otyp == AKLYS && (wep->owornmask & W_WEP) != 0)
                You("secure the tether.");
            prinv((char *) 0, wep, 0L);
            wep->owornmask = dummy;
        }

        setuwep(wep);
        if (was_twoweap && !u.twoweap && flags.verbose) {
            /* skip this message if we already got "empty handed" one above;
               also, Null is not safe for neither TWOWEAPOK() or bimanual() */
            if (uwep)
                You("%s.", ((TWOWEAPOK(uwep) && !bimanual(uwep))
                            ? are_no_longer_twoweap
                            : can_no_longer_twoweap));
        }

        /* KMH -- Talking artifacts are finally implemented */
        if (wep->oartifact) {
            res |= arti_speak(wep); /* sets ECMD_TIME bit if artifact speaks */
        }

        if (artifact_light(wep) && !wep->lamplit) {
            begin_burn(wep, FALSE);
            /* the light can hurt a light-hating hero back into a natural
               form that can't handle the weapon and drops it (maybe into
               lava) */
            if (uwep != wep)
                goto wielded;
            if (!Blind) {
                if (is_art(wep, ART_STAFF_OF_THE_ARCHMAGI)
                    && !Upolyd && Race_if(PM_DROW))
                    pline("%s an aura of darkness!", Tobjnam(wep, "cast"));
                else
                    pline("%s to shine %s!", Tobjnam(wep, "begin"),
                          arti_light_description(wep));
            }
        }
        if ((maybe_polyd(is_elf(gy.youmonst.data), Race_if(PM_ELF))
             || maybe_polyd(is_drow(gy.youmonst.data), Race_if(PM_DROW)))
            && wep->material == IRON && !(Barkskin || Stoneskin)) {
            /* elves and drow are averse to wielding cold iron */
            You("%s wielding cold iron.",
                uarmg ? "have an uneasy feeling about"
                      : "feel extremely uncomfortable");
            if (!uarmg)
                change_luck(-1);
        }
        if (maybe_polyd(is_orc(gy.youmonst.data), Race_if(PM_ORC))
            && wep->material == MITHRIL && !(Barkskin || Stoneskin)) {
            /* orcs are averse to wielding mithril */
            You("%s wielding mithril.",
                uarmg ? "have a weird feeling about"
                      : "feel extremely uncomfortable");
            if (!uarmg)
                change_luck(-1);
        }
        if (wep->unpaid) {
            struct monst *this_shkp;

            if ((this_shkp = shop_keeper(inside_shop(u.ux, u.uy)))
                != (struct monst *) 0) {
                /* check msound because we don't have access to muteshk() */
                if (!Deaf && this_shkp->data->msound > MS_ANIMAL)
                    pline("%s %s \"You be careful with my %s!\"",
                          shkname(this_shkp), says(), xname(wep));
                else
                    pline("%s looks apprehensive about your wielding %s %s.",
                          shkname(this_shkp), mhis(this_shkp), xname(wep));
            }
        }
    }
 wielded:
    if ((had_wep != (uwep != 0)) && condtests[bl_bareh].enabled)
        disp.botl = TRUE;
    return res;
}

void
setuqwep(struct obj *obj)
{
    setworn(obj, W_QUIVER);
    /* no extra handling needed; this used to include a call to
       update_inventory() but that's already performed by setworn() */
    return;
}

/* note: unlike setuwep, no obj == uswapwep early-return guard;
   callers toggling u.twoweap reseat via NULL bracket so begin_burn
   inside this function fires on the second call */
void
setuswapwep(struct obj *obj)
{
    struct obj *olduswapwep = uswapwep;

    /* an off-hand artifact only shines while two-weapon combat is
       active; put it out whenever the slot changes hands */
    if (obj != uswapwep && artifact_light(uswapwep) && uswapwep->lamplit) {
        end_burn(uswapwep, FALSE);
        if (!Blind)
            pline("%s %s.", Tobjnam(uswapwep, "stop"),
                  (is_art(uswapwep, ART_STAFF_OF_THE_ARCHMAGI)
                   && !Upolyd && Race_if(PM_DROW))
                      ? "its aura of darkness" : "shining");
    }
    setworn(obj, W_SWAPWEP);
    if (u.twoweap && uswapwep == obj && artifact_light(uswapwep)
        && !uswapwep->lamplit) {
        begin_burn(uswapwep, FALSE);
        /* the light can hurt a light-hating hero back into a natural
           form that can't handle the weapon and drops it */
        if (!Blind && uswapwep == obj && obj->lamplit) {
            if (is_art(uswapwep, ART_STAFF_OF_THE_ARCHMAGI)
                && !Upolyd && Race_if(PM_DROW))
                pline("%s an aura of darkness!", Tobjnam(uswapwep, "cast"));
            else
                pline("%s to shine %s!", Tobjnam(uswapwep, "begin"),
                      arti_light_description(uswapwep));
        }
    }
    if (uswapwep == obj && u.twoweap && obj && is_botl_artifact(obj))
        disp.botl = TRUE; /* gaining or losing an attribute effect */
    /* wielding or unwielding a weapon of excellence changes Luck */
    if (uswapwep == obj && obj && u.twoweap
        && (obj->oprops & ITEM_EXCEL)) {
        uswapwep->oprops_known |= ITEM_EXCEL;
        set_moreluck();
        disp.botl = TRUE;
        update_inventory();
    }
    if (olduswapwep && (olduswapwep->oprops & ITEM_EXCEL)) {
        olduswapwep->oprops_known |= ITEM_EXCEL;
        set_moreluck();
        disp.botl = TRUE;
        update_inventory();
    }
    return;
}

/* getobj callback for object to ready for throwing/shooting;
   this filter lets worn items through so that caller can reject them */
staticfn int
ready_ok(struct obj *obj)
{
    if (!obj) /* '-', will empty quiver slot if chosen */
        return uquiver ? GETOBJ_SUGGEST : GETOBJ_DOWNPLAY;

    /* downplay when wielded, unless more than one */
    if (obj == uwep || (obj == uswapwep && u.twoweap))
        return (obj->quan == 1) ? GETOBJ_DOWNPLAY : GETOBJ_SUGGEST;

    /* rock-throwing heroes (giants) can usefully ready boulders */
    if (obj->otyp == BOULDER && racial_throws_rocks(&gy.youmonst))
        return GETOBJ_SUGGEST;

    if (is_ammo(obj)) {
        return ((uwep && ammo_and_launcher(obj, uwep))
                || (uswapwep && ammo_and_launcher(obj, uswapwep)))
                ? GETOBJ_SUGGEST
                : GETOBJ_DOWNPLAY;
    } else if (is_launcher(obj)) { /* part of 'possible extension' below */
        return GETOBJ_DOWNPLAY;
    } else {
        if (obj->oclass == WEAPON_CLASS || obj->oclass == COIN_CLASS)
            return GETOBJ_SUGGEST;
        /* Possible extension: exclude weapons that make no sense to throw,
           such as whips, bows, slings, rubber hoses. */
    }

#if 0   /* superseded by ammo_and_launcher handling above */
    /* Include gems/stones as likely candidates if either primary
       or secondary weapon is a sling. */
    if (obj->oclass == GEM_CLASS
        && (uslinging()
            || (uswapwep && objects[uswapwep->otyp].oc_skill == P_SLING)))
        return GETOBJ_SUGGEST;
#endif

    return GETOBJ_DOWNPLAY;
}

/* getobj callback for object to wield */
staticfn int
wield_ok(struct obj *obj)
{
    if (!obj)
        return GETOBJ_SUGGEST;

    if (obj->oclass == COIN_CLASS)
        return GETOBJ_EXCLUDE;

    if (obj->oclass == WEAPON_CLASS || is_weptool(obj))
        return GETOBJ_SUGGEST;

    return GETOBJ_DOWNPLAY;
}

staticfn void
finish_splitting(struct obj *obj)
{
    /* obj was split off from something; give it its own invlet */
    freeinv(obj);
    addinv_nomerge(obj);
}

/* the #wield command - wield a weapon */
int
dowield(void)
{
    char qbuf[QBUFSZ];
    struct obj *wep, *oldwep;
    unsigned oldid;
    int result;

    /* May we attempt this? */
    gm.multi = 0;
    if (cantwield(gy.youmonst.data)) {
        pline("Don't be ridiculous!");
        return ECMD_FAIL;
    }
    /* Keep going even if inventory is completely empty, since wielding '-'
       to wield nothing can be construed as a positive act even when done
       so redundantly. */

    /* Prompt for a new weapon */
    clear_splitobjs();
    if (!(wep = getobj("wield", wield_ok, GETOBJ_PROMPT | GETOBJ_ALLOWCNT))) {
        /* Cancelled */
        return ECMD_CANCEL;
    } else if (wep == uwep) {
 already_wielded:
        You("are already wielding that!");
        if (is_weptool(wep) || is_wet_towel(wep))
            gu.unweapon = FALSE; /* [see setuwep()] */
        return ECMD_FAIL;
    } else if (welded(uwep)) {
        weldmsg(uwep);
        /* previously interrupted armor removal mustn't be resumed */
        reset_remarm();
        /* if player chose a partial stack but can't wield it, undo split */
        if (wep->o_id && wep->o_id == svc.context.objsplit.child_oid)
            unsplitobj(wep);
        return ECMD_FAIL;
    } else if (wep->o_id && wep->o_id == svc.context.objsplit.child_oid) {
        /* if wep is the result of supplying a count to getobj()
           we don't want to split something already wielded; for
           any other item, we need to give it its own inventory slot */
        if (uwep && uwep->o_id == svc.context.objsplit.parent_oid) {
            unsplitobj(wep);
            /* wep was merged back to uwep, already_wielded uses wep */
            wep = uwep;
            goto already_wielded;
        }
        finish_splitting(wep);
        goto wielding;
    }

    /* Handle no object, or object in other slot */
    if (wep == &hands_obj) {
        wep = (struct obj *) 0;
    } else if (wep == uswapwep) {
        return doswapweapon();
    } else if (wep == uquiver) {
        /* offer to split stack if multiple are quivered */
        if (uquiver->quan > 1L && inv_cnt(FALSE) < invlet_basic
                                    && splittable(uquiver)) {
            Sprintf(qbuf, "You have %ld %s readied.  Wield one?",
                    uquiver->quan, simpleonames(uquiver));
            switch (ynq(qbuf)) {
            case 'q':
                return ECMD_OK;
            case 'y':
                /* leave N-1 quivered, split off 1 to wield */
                wep = splitobj(uquiver, 1L);
                finish_splitting(wep);
                goto wielding;
            default:
                break;
            }
            Strcpy(qbuf, "Wield all of them instead?");
        } else {
            boolean use_plural = (is_plural(uquiver) || pair_of(uquiver));

            Sprintf(qbuf, "You have %s readied.  Wield %s instead?",
                    !use_plural ? "that" : "those",
                    !use_plural ? "it" : "them");
        }
        /* require confirmation to wield the quivered weapon */
        if (ynq(qbuf) != 'y') {
            (void) Shk_Your(qbuf, uquiver); /* replace qbuf[] contents */
            pline("%s%s %s readied.", qbuf,
                  simpleonames(uquiver), otense(uquiver, "remain"));
            return ECMD_OK;
        }
        /* wielding whole readied stack, so no longer quivered */
        setuqwep((struct obj *) 0);
    } else if (wep->owornmask
               & (W_ARMOR | W_ACCESSORY | W_SADDLE | W_BARDING)) {
        You("cannot wield that!");
        return ECMD_FAIL;
    }

 wielding:
    /* Set your new primary weapon */
    oldwep = uwep;
    oldid = oldwep ? oldwep->o_id : 0;
    result = ready_weapon(wep);
    /* its light or a touch blast can hurt a polymorphed hero back into
       a natural form whose landing in water or lava lost the old
       weapon */
    if (oldwep && o_on(oldid, gi.invent) != oldwep)
        oldwep = (struct obj *) 0;
    if (flags.pushweapon && oldwep && uwep != oldwep)
        setuswapwep(oldwep);
    untwoweapon();

    return result;
}

/* the #swap command - swap wielded and secondary weapons */
int
doswapweapon(void)
{
    struct obj *oldwep, *oldswap;
    unsigned oldid, swapid;
    int result = 0;

    /* May we attempt this? */
    gm.multi = 0;
    if (cantwield(gy.youmonst.data)) {
        pline("Don't be ridiculous!");
        return ECMD_FAIL;
    }
    if (welded(uwep)) {
        weldmsg(uwep);
        return ECMD_FAIL;
    }

    /* Unwield your current secondary weapon */
    oldwep = uwep;
    oldid = oldwep ? oldwep->o_id : 0;
    oldswap = uswapwep;
    swapid = oldswap ? oldswap->o_id : 0;
    setuswapwep((struct obj *) 0);

    /* Set your new primary weapon */
    result = ready_weapon(oldswap);
    /* its light or a touch blast can hurt a polymorphed hero back into
       a natural form that drops it, or whose landing in water or lava
       loses the old weapon; never seat an item that left inventory */
    if (oldwep && o_on(oldid, gi.invent) != oldwep)
        oldwep = (struct obj *) 0;

    /* Set your new secondary weapon */
    if (uwep == oldwep) {
        /* Wield failed for some reason */
        if (!oldswap || o_on(swapid, gi.invent) == oldswap)
            setuswapwep(oldswap);
    } else {
        setuswapwep(oldwep);
        if (uswapwep)
            prinv((char *) 0, uswapwep, 0L);
        else
            You("have no secondary weapon readied.");
    }

    if (u.twoweap && !can_twoweapon())
        untwoweapon();

    return result;
}

/* the #quiver command */
int
dowieldquiver(void)
{
    return doquiver_core("ready");
}

/* guts of #quiver command; also used by #fire when refilling empty quiver */
int
doquiver_core(const char *verb) /* "ready" or "fire" */
{
    char qbuf[QBUFSZ];
    struct obj *newquiver;
    int res;
    boolean was_uwep = FALSE, was_twoweap = u.twoweap;

    /* Since the quiver isn't in your hands, don't check cantwield(),
       will_weld(), touch_petrifies(), etc. */
    gm.multi = 0;
    if (!gi.invent) {
        /* could accept '-' to empty quiver, but there's no point since
           inventory is empty so uquiver is already Null */
        You("have nothing to ready for firing.");
        return ECMD_OK;
    }

    /* forget last splitobj() before calling getobj() with GETOBJ_ALLOWCNT */
    clear_splitobjs();
    /* Prompt for a new quiver: "What do you want to {ready|fire}?" */
    newquiver = getobj(verb, ready_ok, GETOBJ_PROMPT | GETOBJ_ALLOWCNT);

    if (!newquiver) {
        /* Cancelled */
        return ECMD_CANCEL;
    } else if (newquiver == &hands_obj) { /* no object */
        /* Explicitly nothing */
        if (uquiver) {
            You("now have no ammunition readied.");
            /* skip 'quivering: prinv()' */
            setuqwep((struct obj *) 0);
        } else {
            You("already have no ammunition readied!");
        }
        return ECMD_OK;
    } else if (newquiver->o_id == svc.context.objsplit.child_oid) {
        /* if newquiver is the result of supplying a count to getobj()
           we don't want to split something already in the quiver;
           for any other item, we need to give it its own inventory slot */
        if (uquiver && uquiver->o_id == svc.context.objsplit.parent_oid) {
            unsplitobj(newquiver);
            goto already_quivered;
        } else if (newquiver->oclass == COIN_CLASS) {
            /* don't allow splitting a stack of coins into quiver */
            You("can't ready only part of your gold.");
            unsplitobj(newquiver);
            return ECMD_OK;
        }
        finish_splitting(newquiver);
    } else if (newquiver == uquiver) {
 already_quivered:
        pline("That ammunition is already readied!");
        return ECMD_OK;
    } else if (newquiver->owornmask
               & (W_ARMOR | W_ACCESSORY | W_SADDLE | W_BARDING)) {
        You("cannot %s that!", verb);
        return ECMD_OK;
    } else if (newquiver == uwep) {
        int weld_res = !uwep->bknown;

        if (welded(uwep)) {
            weldmsg(uwep);
            reset_remarm(); /* same as dowield() */
            return weld_res ? ECMD_TIME : ECMD_OK;
        }
        /* offer to split stack if wielding more than 1 */
        if (uwep->quan > 1L && inv_cnt(FALSE) < invlet_basic
                                    && splittable(uwep)) {
            Sprintf(qbuf, "You are wielding %ld %s.  Ready %ld of them?",
                    uwep->quan, simpleonames(uwep), uwep->quan - 1L);
            switch (ynq(qbuf)) {
            case 'q':
                return ECMD_OK;
            case 'y':
                /* leave 1 wielded, split rest off and put into quiver */
                newquiver = splitobj(uwep, uwep->quan - 1L);
                finish_splitting(newquiver);
                goto quivering;
            default:
                break;
            }
            Strcpy(qbuf, "Ready all of them instead?");
        } else {
            boolean use_plural = (is_plural(uwep) || pair_of(uwep));

            Sprintf(qbuf, "You are wielding %s.  Ready %s instead?",
                    !use_plural ? "that" : "those",
                    !use_plural ? "it" : "them");
        }
        /* require confirmation to ready the main weapon */
        if (ynq(qbuf) != 'y') {
            (void) Shk_Your(qbuf, uwep); /* replace qbuf[] contents */
            pline("%s%s %s wielded.", qbuf,
                  simpleonames(uwep), otense(uwep, "remain"));
            return ECMD_OK;
        }
        /* quivering main weapon, so no longer wielding it */
        setuwep((struct obj *) 0);
        untwoweapon();
        was_uwep = TRUE;
    } else if (newquiver == uswapwep) {
        if (uswapwep->quan > 1L && inv_cnt(FALSE) < invlet_basic
            && splittable(uswapwep)) {
            Sprintf(qbuf, "%s %ld %s.  Ready %ld of them?",
                    u.twoweap ? "You are dual wielding"
                              : "Your alternate weapon is",
                    uswapwep->quan, simpleonames(uswapwep),
                    uswapwep->quan - 1L);
            switch (ynq(qbuf)) {
            case 'q':
                return ECMD_OK;
            case 'y':
                /* leave 1 alt-wielded, split rest off and put into quiver */
                newquiver = splitobj(uswapwep, uswapwep->quan - 1L);
                finish_splitting(newquiver);
                goto quivering;
            default:
                break;
            }
            Strcpy(qbuf, "Ready all of them instead?");
        } else {
            boolean use_plural = (is_plural(uswapwep) || pair_of(uswapwep));

            Sprintf(qbuf, "%s your %s weapon.  Ready %s instead?",
                    !use_plural ? "That is" : "Those are",
                    u.twoweap ? "second" : "alternate",
                    !use_plural ? "it" : "them");
        }
        /* require confirmation to ready the alternate weapon */
        if (ynq(qbuf) != 'y') {
            (void) Shk_Your(qbuf, uswapwep); /* replace qbuf[] contents */
            pline("%s%s %s %s.", qbuf,
                  simpleonames(uswapwep), otense(uswapwep, "remain"),
                  u.twoweap ? "wielded" : "as secondary weapon");
            return ECMD_OK;
        }
        /* quivering alternate weapon, so no more uswapwep */
        setuswapwep((struct obj *) 0);
        untwoweapon();
    }

 quivering:
    if (!strcmp(verb, "ready")) {
        /* place item in quiver before printing so that inventory feedback
           includes "(at the ready)" */
        setuqwep(newquiver);
        prinv((char *) 0, newquiver, 0L);
    } else { /* verb=="fire", manually refilling quiver during 'f'ire */
        /* prefix item with description of action, so don't want that to
           include "(at the ready)" */
        prinv("You ready:", newquiver, 0L);
        setuqwep(newquiver);
    }

    /* quiver is a convenience slot and manipulating it ordinarily
       consumes no time, but unwielding primary or secondary weapon
       should take time (perhaps we're adjacent to a rust monster
       or disenchanter and want to hit it immediately, but not with
       something we're wielding that's vulnerable to its damage) */
    res = 0;
    if (was_uwep) {
        You("are now %s.", empty_handed());
        res = 1;
    } else if (was_twoweap && !u.twoweap) {
        You("%s.", are_no_longer_twoweap);
        res = 1;
    }
    return res ? ECMD_TIME : ECMD_OK;
}

/* used for #rub and for applying pick-axe, whip, grappling hook or polearm */
boolean
wield_tool(struct obj *obj,
           const char *verb) /* "rub",&c */
{
    const char *what;
    boolean more_than_1;

    if (uwep && obj == uwep)
        return TRUE; /* nothing to do if already wielding it */

    if (!verb)
        verb = "wield";
    what = xname(obj);
    more_than_1 = (obj->quan > 1L || strstri(what, "pair of ") != 0
                   || strstri(what, "s of ") != 0);

    if (obj->owornmask & (W_ARMOR | W_ACCESSORY)) {
        You_cant("%s %s while wearing %s.", verb, yname(obj),
                 more_than_1 ? "them" : "it");
        return FALSE;
    }
    if (uwep && welded(uwep)) {
        if (flags.verbose) {
            const char *hand = body_part(HAND);

            if (bimanual(uwep))
                hand = makeplural(hand);
            if (strstri(what, "pair of ") != 0)
                more_than_1 = FALSE;
            pline(
               "Since your weapon is welded to your %s, you cannot %s %s %s.",
                  hand, verb, more_than_1 ? "those" : "that", xname(obj));
        } else {
            You_cant("do that.");
        }
        return FALSE;
    }
    if (cantwield(gy.youmonst.data)) {
        if (druid_form || vampire_form)
            You("aren't able to hold %s properly while in this form.",
                yname(obj));
        else
            You_cant("hold %s strongly enough.",
                     more_than_1 ? "them" : "it");
        return FALSE;
    }
    /* check shield */
    if (uarms && !is_bracer(uarms) && bimanual(obj)) {
        You("cannot %s a two-handed %s while wearing a shield.", verb,
            (obj->oclass == WEAPON_CLASS) ? "weapon" : "tool");
        return FALSE;
    }

    if (uquiver == obj)
        setuqwep((struct obj *) 0);
    if (uswapwep == obj) {
        (void) doswapweapon();
        /* doswapweapon might fail */
        if (uswapwep == obj)
            return FALSE;
    } else {
        struct obj *oldwep = uwep;
        unsigned oldid = oldwep ? oldwep->o_id : 0;

        if (will_weld_to_you(obj)) {
            /* hope none of ready_weapon()'s early returns apply here... */
            (void) ready_weapon(obj);
        } else {
            You("now wield %s.", doname(obj));
            setuwep(obj);
        }
        /* as in dowield(): never seat an old weapon that left inventory */
        if (oldwep && o_on(oldid, gi.invent) != oldwep)
            oldwep = (struct obj *) 0;
        if (flags.pushweapon && oldwep && uwep != oldwep)
            setuswapwep(oldwep);
    }
    /* not wielded after all: the old object was rewielded after dying,
       or the wield failed or cost the hero obj (returning to a natural
       form can drop it, or land the hero where it is destroyed) */
    if (uwep != obj)
        return FALSE;
    /* applying weapon or tool that gets wielded ends two-weapon combat */
    if (u.twoweap)
        untwoweapon();
    if (obj->oclass != WEAPON_CLASS)
        gu.unweapon = TRUE;
    return TRUE;
}

int
can_twoweapon(void)
{
    struct obj *otmp;

    if (!could_twoweap(gy.youmonst.data)) {
        if (Upolyd)
            You_cant("use two weapons in your current form.");
        else
            pline("%s aren't able to use two weapons at once.",
                  makeplural((flags.female && gu.urole.name.f)
                             ? gu.urole.name.f : gu.urole.name.m));
    } else if (!uwep || !uswapwep) {
        const char *hand_s = body_part(HAND);

        if (!uwep && !uswapwep)
            hand_s = makeplural(hand_s);
        /* "your hands are empty" or "your {left|right} hand is empty" */
        Your("%s%s %s empty.", uwep ? "left " : uswapwep ? "right " : "",
             hand_s, vtense(hand_s, "are"));
    } else if (!TWOWEAPOK(uwep) || !TWOWEAPOK(uswapwep)) {
        otmp = !TWOWEAPOK(uwep) ? uwep : uswapwep;
        pline("%s %s suitable %s weapon%s.", Yname2(otmp),
              is_plural(otmp) ? "aren't" : "isn't a",
              (otmp == uwep) ? "primary" : "secondary",
              plur(otmp->quan));
    } else if (bimanual(uwep) || bimanual(uswapwep)) {
        otmp = bimanual(uwep) ? uwep : uswapwep;
        pline("%s isn't one-handed.", Yname2(otmp));
    } else if (uarms && !is_bracer(uarms)) {
        You_cant("use two weapons while wearing a shield.");
    /* allow two-weaponing with an artifact, but not if they are of
       opposite alignments; as expected, neutral artifacts don't care */
    } else if (uswapwep->oartifact
               && ((is_lawful_artifact(uswapwep) && is_chaotic_artifact(uwep))
                   || (is_chaotic_artifact(uswapwep)
                       && is_lawful_artifact(uwep)))) {
        pline("%s being held second to an opposite aligned weapon!",
              Yobjnam2(uswapwep, "resist"));
    /* the Wand of Orcus will not tolerate being wielded along with
       another artifact */
    } else if (is_art(uwep, ART_WAND_OF_ORCUS) && uswapwep->oartifact) {
        pline("%s being held alongside another artifact!",
              Yobjnam2(uwep, "resist"));
    /* the Wand of Orcus will not tolerate being held second to
       any object */
    } else if (is_art(uswapwep, ART_WAND_OF_ORCUS)
               && (is_weptool(uwep) || uwep->oclass == WEAPON_CLASS)) {
        pline("%s being held second to another %s!",
              Yobjnam2(uswapwep, "resist"),
              uwep->oartifact ? "artifact" : "weapon");
    } else if (uswapwep->otyp == CORPSE && cant_wield_corpse(uswapwep)) {
        /* [Note: !TWOWEAPOK() check prevents ever getting here...] */
        ; /* must be life-saved to reach here; return FALSE */
    } else if (Glib || uswapwep->cursed) {
        if (!Glib)
            set_bknown(uswapwep, 1);
        drop_uswapwep();
    } else
        return TRUE;
    return FALSE;
}

/* uswapwep has become cursed while in two-weapon combat mode or hero is
   attempting to dual-wield when it is already cursed or hands are slippery */
void
drop_uswapwep(void)
{
    char left_hand[QBUFSZ];
    struct obj *obj = uswapwep;

    /* this used to use makeplural(body_part(HAND)) but in order to be
       dual-wielded, or to get this far attempting to achieve that,
       uswapwep must be one-handed; since it's secondary, the hand must
       be the left one */
    Sprintf(left_hand, "left %s", body_part(HAND));
    if (!obj->cursed)
        /* attempting to two-weapon while Glib */
        pline("%s from your %s!", Yobjnam2(obj, "slip"), left_hand);
    else if (!u.twoweap)
        /* attempting to two-weapon when uswapwep is cursed */
        pline("%s your grasp and %s from your %s!",
              Yobjnam2(obj, "evade"), otense(obj, "drop"), left_hand);
    else
        /* already two-weaponing but can't anymore because uswapwep has
           become cursed */
        Your("%s spasms and drops %s!", left_hand, yobjnam(obj, (char *) 0));
    dropx(obj);
}

void
set_twoweap(boolean on_off)
{
    if (on_off != u.twoweap) {
        u.twoweap = on_off;
        /* the weapon field, and Str / Con / Cha when the off-hand is
           an attribute artifact or a weapon of excellence */
        disp.botl = TRUE;
    }
}

/* the #twoweapon command */
int
dotwoweapon(void)
{
    if (Hidinshell) {
        pline("Don't be ridiculous!");
        return ECMD_OK;
    }

    /* You can always toggle it off */
    if (u.twoweap) {
        struct obj *tmp = uswapwep;

        You("switch to your primary weapon.");
        set_twoweap(FALSE); /* u.twoweap = FALSE */
        /* reseat the off-hand weapon so that setworn() revokes its
           W_SWAPWEP extrinsics now that two-weapon combat is off */
        setuswapwep((struct obj *) 0);
        setuswapwep(tmp);
        update_inventory();
        return ECMD_OK;
    }

    /* From sword & board: shed the shield so the readied off-hand
       weapon can be engaged, mirroring how the #shield command sheds
       the off-hand weapon. A worn shield is the only can_twoweapon()
       refusal this can clear, so every other refusal it makes has to be
       repeated here; otherwise the shield comes off and two-weapon
       combat is declined anyway. A cursed shield stays put and
       can_twoweapon() reports the shield itself */
    if (uarms && !is_bracer(uarms) && !cursed(uarms, TRUE)
        && could_twoweap(gy.youmonst.data)
        && uwep && !bimanual(uwep) && twoweapon_ok(uwep)
        && uswapwep && !bimanual(uswapwep) && twoweapon_ok(uswapwep)
        && !(uswapwep->oartifact
             && ((is_lawful_artifact(uswapwep) && is_chaotic_artifact(uwep))
                 || (is_chaotic_artifact(uswapwep)
                     && is_lawful_artifact(uwep))))
        && !(is_art(uwep, ART_WAND_OF_ORCUS) && uswapwep->oartifact)
        && !is_art(uswapwep, ART_WAND_OF_ORCUS)
        && !uswapwep->cursed && !Glib) {
        struct obj *shld = uarms;

        remove_worn_item(shld, FALSE);
        designate_altshield(shld);
        You("take off %s.", yname(shld));
    }

    /* May we use two weapons? */
    if (can_twoweapon()) {
        struct obj *tmp = uswapwep;

        /* Success! */
        You("begin two-weapon combat.");
        /* reseat via a NULL bracket to force setworn() to run the
           W_SWAPWEP grant block; a direct setuswapwep(uswapwep) call
           would tear the occupied slot down and cancel two-weapon
           combat in the process; set u.twoweap after the NULL teardown,
           not before, since setworn() cancels two-weapon combat when a
           worn weapon is replaced while it is active */
        setuswapwep((struct obj *) 0);
        set_twoweap(TRUE); /* u.twoweap = TRUE */
        setuswapwep(tmp);
        update_inventory();
        return (rnd(20) > ACURR(A_DEX)) ? ECMD_TIME : ECMD_OK;
    }
    return ECMD_OK;
}

/*** Functions to empty a given slot ***/
/* These should be used only when the item can't be put back in
 * the slot by life saving.  Proper usage includes:
 * 1.  The item has been eaten, stolen, burned away, or rotted away.
 * 2.  Making an item disappear for a bones pile.
 */
void
uwepgone(void)
{
    if (uwep) {
        if (artifact_light(uwep) && uwep->lamplit) {
            end_burn(uwep, FALSE);
            if (!Blind)
                pline("%s %s.", Tobjnam(uwep, "stop"),
                      (is_art(uwep, ART_STAFF_OF_THE_ARCHMAGI)
                       && !Upolyd && Race_if(PM_DROW))
                          ? "its aura of darkness" : "shining");
        }
        setworn((struct obj *) 0, W_WEP);
        gu.unweapon = TRUE;
        update_inventory();
    }
}

void
uswapwepgone(void)
{
    if (uswapwep) {
        if (artifact_light(uswapwep) && uswapwep->lamplit) {
            end_burn(uswapwep, FALSE);
            if (!Blind)
                pline("%s %s.", Tobjnam(uswapwep, "stop"),
                      (is_art(uswapwep, ART_STAFF_OF_THE_ARCHMAGI)
                       && !Upolyd && Race_if(PM_DROW))
                          ? "its aura of darkness" : "shining");
        }
        setworn((struct obj *) 0, W_SWAPWEP);
        update_inventory();
    }
}

void
uqwepgone(void)
{
    if (uquiver) {
        setworn((struct obj *) 0, W_QUIVER);
        update_inventory();
    }
}

void
untwoweapon(void)
{
    struct obj *tmp = uswapwep;

    if (u.twoweap) {
        You("%s.", can_no_longer_twoweap);
        set_twoweap(FALSE); /* u.twoweap = FALSE */
    }
    /* reseat the off-hand weapon so that setworn() revokes its
       W_SWAPWEP extrinsics; harmless when nothing is readied */
    setuswapwep((struct obj *) 0);
    setuswapwep(tmp);
    update_inventory();
    return;
}

/* re-engage two-weapon combat after a thrown primary weapon returns to
   hand; the off-hand object is already seated in W_SWAPWEP, so a plain
   setuswapwep(uswapwep) call would tear the slot down and cancel
   two-weapon combat without ever running the W_SWAPWEP-gated grant,
   leaving off-hand extrinsics dead until the next two-weapon toggle;
   reseat via a NULL bracket so the grant runs with u.twoweap set,
   mirroring dotwoweapon(); caller guarantees uswapwep is non-NULL and
   u.twoweap was torn down by the throw */
void
retwoweapon(void)
{
    struct obj *tmp = uswapwep;

    setuswapwep((struct obj *) 0);
    set_twoweap(TRUE); /* u.twoweap = TRUE */
    setuswapwep(tmp);
    update_inventory();
    return;
}

/* enchant wielded weapon */
int
chwepon(struct obj *otmp, int amount)
{
    const char *color = hcolor((amount < 0) ? NH_BLACK : NH_BLUE);
    const char *xtime, *wepname = "";
    boolean multiple;
    int otyp = STRANGE_OBJECT;

    if (!uwep || (uwep->oclass != WEAPON_CLASS && !is_weptool(uwep))) {
        char buf[BUFSZ];

        if (amount >= 0 && uwep && will_weld(uwep)) { /* cursed tin opener */
            if (!Blind) {
                Snprintf(buf, sizeof buf, "%s with %s aura.",
                         Yobjnam2(uwep, "glow"), an(hcolor(NH_AMBER)));
                uwep->bknown = !Hallucination; /* ok to bypass set_bknown() */
            } else {
                /* cursed tin opener is wielded in right hand */
                Sprintf(buf, "Your right %s tingles.", body_part(HAND));
            }
            uncurse(uwep);
            update_inventory();
        } else {
            Sprintf(buf, "Your %s %s.", makeplural(body_part(HAND)),
                    (amount >= 0) ? "twitch" : "itch");
        }
        strange_feeling(otmp, buf); /* pline()+docall()+useup() */
        exercise(A_DEX, (boolean) (amount >= 0));
        return 0;
    }

    if (otmp && otmp->oclass == SCROLL_CLASS)
        otyp = otmp->otyp;

    if (uwep->otyp == WORM_TOOTH && amount >= 0) {
        multiple = (uwep->quan > 1L);
        /* order: message, transformation, shop handling */
        Your("%s %s much sharper now.", simpleonames(uwep),
             multiple ? "fuse, and become" : "is");
        uwep->otyp = CRYSKNIFE;
        maybe_erodeproof(uwep, 0);
        set_material(uwep, objects[CRYSKNIFE].oc_material);
        if (multiple)
            uwep->quan = 1L;
        /* recompute weight unconditionally; both single and stack paths
           change otyp and material so owt may have shifted */
        uwep->owt = weight(uwep);
        if (uwep->cursed)
            uncurse(uwep);
        /* update shop bill to reflect new higher value */
        if (uwep->unpaid)
            alter_cost(uwep, 0L);
        if (otyp != STRANGE_OBJECT)
            makeknown(otyp);
        if (multiple)
            encumber_msg();
        return 1;
    } else if (uwep->otyp == CRYSKNIFE && amount < 0) {
        multiple = (uwep->quan > 1L);
        /* order matters: message, shop handling, transformation */
        Your("%s %s much duller now.", simpleonames(uwep),
             multiple ? "fuse, and become" : "is");
        costly_alteration(uwep, COST_DEGRD); /* DECHNT? other? */
        uwep->otyp = WORM_TOOTH;
        maybe_erodeproof(uwep, 0);
        set_material(uwep, objects[WORM_TOOTH].oc_material);
        if (multiple)
            uwep->quan = 1L;
        uwep->owt = weight(uwep);
        if (otyp != STRANGE_OBJECT && otmp->bknown)
            makeknown(otyp);
        if (multiple)
            encumber_msg();
        return 1;
    }

    if (has_oname(uwep))
        wepname = ONAME(uwep);
    if (amount < 0 && uwep->oartifact && restrict_name(uwep, wepname)) {
        if (!Blind)
            pline("%s %s.", Yobjnam2(uwep, "faintly glow"), color);
        return 1;
    }
    /* there is a (soft) upper and lower limit to uwep->spe.
       affixed gemstones can raise the safe limit */
    {
        int safe_limit = 5 + gem_enchant_bonus(uwep);

        if (((uwep->spe > safe_limit && amount >= 0)
             || (uwep->spe < -safe_limit && amount < 0))
            && rn2(3)) {
            if (uwep->oartifact == ART_SWORD_OF_ANNIHILATION) {
                if (!Blind)
                    pline("%s %s for a while, but %s intact.",
                          Yobjnam2(uwep, "violently glow"), color,
                          otense(uwep, "remain"));
                /* no particular sensation of "non-evaporation" if blind */
            } else {
                if (!Blind)
                    pline("%s %s for a while and then %s.",
                          Yobjnam2(uwep, "violently glow"), color,
                          otense(uwep, "evaporate"));
                else
                    pline("%s.", Yobjnam2(uwep, "evaporate"));

                /* evaporate message above subsumes "stops shining";
                   still explicitly end the burn for parity with
                   uwepgone() */
                if (artifact_light(uwep) && uwep->lamplit)
                    end_burn(uwep, FALSE);
                useupall(uwep); /* let all of them disappear */
            }
            return 1;
        }
    }
    if (!Blind) {
        xtime = (amount * amount == 1) ? "moment" : "while";
        pline("%s %s for a %s.",
              Yobjnam2(uwep, amount == 0 ? "violently glow" : "glow"), color,
              xtime);
        if (otyp != STRANGE_OBJECT && uwep->known
            && (amount > 0 || (amount < 0 && otmp->bknown)))
            makeknown(otyp);
    }
    if (amount < 0)
        costly_alteration(uwep, COST_DECHNT);
    uwep->spe += amount;
    if (amount > 0) {
        if (uwep->cursed)
            uncurse(uwep);
        /* update shop bill to reflect new higher price */
        if (uwep->unpaid)
            alter_cost(uwep, 0L);
    }

    /*
     * Enchantment, which normally improves a weapon, has an
     * additional adverse reaction on Magicbane whose effects are
     * spe dependent.  Give an obscure clue here.
     */
    if (is_art(uwep, ART_MAGICBANE) && uwep->spe >= 0) {
        Your("right %s %sches!", body_part(HAND),
             (((amount > 1) && (uwep->spe > 1)) ? "flin" : "it"));
    }

    /* an elven magic clue, cookie@keebler */
    /* elven weapons vibrate warningly when enchanted beyond a limit */
    if ((uwep->spe > 5 + gem_enchant_bonus(uwep))
        && (is_elven_weapon(uwep) || is_drow_weapon(uwep)
            || uwep->oartifact || !rn2(7)))
        pline("%s unexpectedly.", Yobjnam2(uwep, "suddenly vibrate"));

    return 1;
}

int
welded(struct obj *obj)
{
    if (obj && obj == uwep && will_weld_to_you(obj)) {
        set_bknown(obj, 1);
        return 1;
    }
    return 0;
}

void
weldmsg(struct obj *obj)
{
    long savewornmask;
    const char *hand = body_part(HAND);

    if (bimanual(obj))
        hand = makeplural(hand);
    savewornmask = obj->owornmask;
    obj->owornmask = 0L; /* suppress doname()'s "(weapon in hand)";
                          * Yobjnam2() doesn't actually need this because
                          * it is based on xname() rather than doname() */
    pline("%s welded to your %s!", Yobjnam2(obj, "are"), hand);
    obj->owornmask = savewornmask;
}

/* test whether monster's wielded weapon is stuck to hand/paw/whatever */
boolean
mwelded(struct obj *obj)
{
    /* caller is responsible for making sure this is a monster's item */
    if (obj && (obj->owornmask & W_WEP) && will_weld(obj))
        return TRUE;
    return FALSE;
}

/*wield.c*/
