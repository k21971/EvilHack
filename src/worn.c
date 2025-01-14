/* NetHack 3.6	worn.c	$NHDT-Date: 1550524569 2019/02/18 21:16:09 $  $NHDT-Branch: NetHack-3.6.2-beta01 $:$NHDT-Revision: 1.56 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2013. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

STATIC_DCL void FDECL(m_lose_armor, (struct monst *, struct obj *));
STATIC_DCL void FDECL(m_dowear_type,
                      (struct monst *, long, BOOLEAN_P, BOOLEAN_P));

const struct worn {
    long w_mask;
    struct obj **w_obj;
} worn[] = { { W_ARM, &uarm },
             { W_ARMC, &uarmc },
             { W_ARMH, &uarmh },
             { W_ARMS, &uarms },
             { W_ARMG, &uarmg },
             { W_ARMF, &uarmf },
             { W_ARMU, &uarmu },
             { W_RINGL, &uleft },
             { W_RINGR, &uright },
             { W_WEP, &uwep },
             { W_SWAPWEP, &uswapwep },
             { W_QUIVER, &uquiver },
             { W_AMUL, &uamul },
             { W_TOOL, &ublindf },
             { W_BALL, &uball },
             { W_CHAIN, &uchain },
             { 0, 0 }
};

/* This only allows for one blocking item per property */
#define w_blocks(o, m) \
    ((o->otyp == MUMMY_WRAPPING && ((m) & W_ARMC))                          \
         ? INVIS                                                            \
         : (o->otyp == CORNUTHAUM && ((m) & W_ARMH) && !Role_if(PM_WIZARD)) \
               ? CLAIRVOYANT                                                \
               : 0)
/* note: monsters don't have clairvoyance, so your role
   has no significant effect on their use of w_blocks() */

/* Updated to use the extrinsic and blocked fields. */
void
setworn(obj, mask)
register struct obj *obj;
long mask;
{
    register const struct worn *wp;
    register struct obj *oobj;
    register int p;

    if ((mask & I_SPECIAL) != 0 && (mask & (W_ARM | W_ARMC)) != 0) {
        /* restoring saved game; no properties are conferred via skin */
        uskin = obj;
        /* assert( !uarm ); */
    } else {
        if ((mask & W_ARMOR))
            u.uroleplay.nudist = FALSE;
        for (wp = worn; wp->w_mask; wp++) {
            if (wp->w_mask & mask) {
                oobj = *(wp->w_obj);
                if (oobj && !(oobj->owornmask & wp->w_mask))
                    impossible("Setworn: mask = %ld.", wp->w_mask);
                if (oobj && (oobj != obj)) {
                    if (u.twoweap && (oobj->owornmask & (W_WEP | W_SWAPWEP))) {
                        /* required to avoid incorrect untwoweapon() feedback */
                        u.twoweap = 0;
                        /* required to turn off offhand extrinsics */
                        untwoweapon();
                    }
                    oobj->owornmask &= ~wp->w_mask;
                    if (wp->w_mask & ~(W_QUIVER)) {
                        /* leave as "x = x <op> y", here and below, for broken
                         * compilers */
                        p = armor_provides_extrinsic(oobj);
                        u.uprops[p].extrinsic =
                            u.uprops[p].extrinsic & ~wp->w_mask;
                        if ((p = w_blocks(oobj, mask)) != 0)
                            u.uprops[p].blocked &= ~wp->w_mask;
                        if (oobj->oartifact || oobj->oprops)
                            set_artifact_intrinsic(oobj, 0, mask);
                    }
                    /* in case wearing or removal is in progress or removal
                       is pending (via 'A' command for multiple items) */
                    cancel_doff(oobj, wp->w_mask);
                }
                *(wp->w_obj) = obj;
                if (obj) {
                    obj->owornmask |= wp->w_mask;
                    /* Prevent getting/blocking intrinsics from wielding
                     * potions, through the quiver, etc.
                     * Allow weapon-tools, too.
                     * wp_mask should be same as mask at this point.
                     */
                    if ((wp->w_mask & ~(W_SWAPWEP | W_QUIVER))
                        || (wp->w_mask & W_SWAPWEP && u.twoweap)) {
                        if (obj->oclass == WEAPON_CLASS || is_weptool(obj)
                            || mask != W_WEP) {
                            p = armor_provides_extrinsic(obj);
                            u.uprops[p].extrinsic =
                                u.uprops[p].extrinsic | wp->w_mask;
                            /* no magic resistance conduct */
                            if (p == ANTIMAGIC)
                                u.uconduct.antimagic++;
                            /* no reflection conduct */
                            if (p == REFLECTING)
                                u.uconduct.reflection++;
                            if ((p = w_blocks(obj, mask)) != 0)
                                u.uprops[p].blocked |= wp->w_mask;
                        }
                        if (obj->oartifact || obj->oprops)
                            set_artifact_intrinsic(obj, 1, mask);
                    }
                }
            }
        }
    }
    update_inventory();
}

/* called e.g. when obj is destroyed */
/* Updated to use the extrinsic and blocked fields. */
void
setnotworn(obj)
register struct obj *obj;
{
    register const struct worn *wp;
    register int p;

    if (!obj)
        return;
    if (obj == uwep || obj == uswapwep)
        u.twoweap = 0;
    for (wp = worn; wp->w_mask; wp++) {
        if (obj == *(wp->w_obj)) {
            /* in case wearing or removal is in progress or removal
               is pending (via 'A' command for multiple items) */
            cancel_doff(obj, wp->w_mask);

            *(wp->w_obj) = (struct obj *) 0;
            p = armor_provides_extrinsic(obj);
            u.uprops[p].extrinsic = u.uprops[p].extrinsic & ~wp->w_mask;
            obj->owornmask &= ~wp->w_mask;
            if (obj->oartifact || obj->oprops)
                set_artifact_intrinsic(obj, 0, wp->w_mask);
            if ((p = w_blocks(obj, wp->w_mask)) != 0)
                u.uprops[p].blocked &= ~wp->w_mask;
        }
    }
    update_inventory();
}

/* called when saving with FREEING flag set has just discarded inventory */
void
allunworn()
{
    const struct worn *wp;

    u.twoweap = 0; /* uwep and uswapwep are going away */
    /* remove stale pointers; called after the objects have been freed
       (without first being unworn) while saving invent during game save;
       note: uball and uchain might not be freed yet but we clear them
       here anyway (savegamestate() and its callers deal with them) */
    for (wp = worn; wp->w_mask; wp++) {
        /* object is already gone so we don't/can't update is owornmask */
        *(wp->w_obj) = (struct obj *) 0;
    }
}

/* return item worn in slot indiciated by wornmask; needed by poly_obj() */
struct obj *
wearmask_to_obj(wornmask)
long wornmask;
{
    const struct worn *wp;

    for (wp = worn; wp->w_mask; wp++)
        if (wp->w_mask & wornmask)
            return *wp->w_obj;
    return (struct obj *) 0;
}

/* return a bitmask of the equipment slot(s) a given item might be worn in */
long
wearslot(obj)
struct obj *obj;
{
    int otyp = obj->otyp;
    /* practically any item can be wielded or quivered; it's up to
       our caller to handle such things--we assume "normal" usage */
    long res = 0L; /* default: can't be worn anywhere */

    switch (obj->oclass) {
    case AMULET_CLASS:
        res = W_AMUL; /* WORN_AMUL */
        break;
    case RING_CLASS:
        res = W_RINGL | W_RINGR; /* W_RING, BOTH_SIDES */
        break;
    case ARMOR_CLASS:
        switch (objects[otyp].oc_armcat) {
        case ARM_SUIT:
            res = W_ARM;
            break; /* WORN_ARMOR */
        case ARM_SHIELD:
            res = W_ARMS;
            break; /* WORN_SHIELD */
        case ARM_HELM:
            res = W_ARMH;
            break; /* WORN_HELMET */
        case ARM_GLOVES:
            res = W_ARMG;
            break; /* WORN_GLOVES */
        case ARM_BOOTS:
            res = W_ARMF;
            break; /* WORN_BOOTS */
        case ARM_CLOAK:
            res = W_ARMC;
            break; /* WORN_CLOAK */
        case ARM_SHIRT:
            res = W_ARMU;
            break; /* WORN_SHIRT */
        }
        break;
    case WEAPON_CLASS:
        res = W_WEP | W_SWAPWEP;
        if (objects[otyp].oc_merge)
            res |= W_QUIVER;
        break;
    case TOOL_CLASS:
        if (otyp == BLINDFOLD || otyp == TOWEL
            || otyp == LENSES || otyp == GOGGLES)
            res = W_TOOL; /* WORN_BLINDF */
        else if (is_weptool(obj) || otyp == TIN_OPENER)
            res = W_WEP | W_SWAPWEP;
        else if (otyp == SADDLE)
            res = W_SADDLE;
        else if (is_barding(obj))
            res = W_BARDING;
        break;
    case FOOD_CLASS:
        if (obj->otyp == MEAT_RING)
            res = W_RINGL | W_RINGR;
        break;
    case GEM_CLASS:
        res = W_QUIVER;
        break;
    case BALL_CLASS:
        res = W_BALL;
        break;
    case CHAIN_CLASS:
        res = W_CHAIN;
        break;
    default:
        break;
    }
    return res;
}

void
mon_set_minvis(mon)
struct monst *mon;
{
    mon->perminvis = 1;
    if (!mon->invis_blkd) {
        mon->minvis = 1;
        newsym(mon->mx, mon->my); /* make it disappear */
        if (mon->wormno)
            see_wsegs(mon); /* and any tail too */
    }
}

void
mon_adjust_speed(mon, adjust, obj)
struct monst *mon;
int adjust;      /* positive => increase speed, negative => decrease */
struct obj *obj; /* item to make known if effect can be seen */
{
    struct obj *otmp;
    boolean give_msg = !in_mklev, petrify = FALSE;
    unsigned int oldspeed = mon->mspeed;

    switch (adjust) {
    case 3:
        give_msg = FALSE; /* recovering from sewage */
        break;
    case 2:
        mon->permspeed = MFAST;
        give_msg = FALSE; /* special case monster creation */
        break;
    case 1:
        if (mon->permspeed == MSLOW)
            mon->permspeed = 0;
        else
            mon->permspeed = MFAST;
        break;
    case 0: /* just check for worn speed boots */
        break;
    case -1:
        if (mon->permspeed == MFAST)
            mon->permspeed = 0;
        else if (!(defended(mon, AD_SLOW) || resists_slow(r_data(mon))))
            mon->permspeed = MSLOW;
        break;
    case -2: /* wading through sewage: set mspeed for temporary slow */
        if (!(defended(mon, AD_SLOW) || resists_slow(r_data(mon))))
            mon->mspeed = MSLOW;
        give_msg = FALSE;
        return; /* return early so mspeed isn't changed */
    case -3: /* petrification */
        /* take away intrinsic speed but don't reduce normal speed */
        if (mon->permspeed == MFAST)
            mon->permspeed = 0;
        petrify = TRUE;
        break;
    case -4: /* green slime */
        if (mon->permspeed == MFAST)
            mon->permspeed = 0;
        give_msg = FALSE;
        break;
    }

    for (otmp = mon->minvent; otmp; otmp = otmp->nobj) {
        if (otmp->owornmask && objects[otmp->otyp].oc_oprop == FAST)
            break;
        if (otmp->owornmask && Is_dragon_scaled_armor(otmp)
            && Dragon_armor_to_scales(otmp))
            break;
    }
    if (otmp) /* speed boots/blue-scaled armor */
        mon->mspeed = MFAST;
    else
        mon->mspeed = mon->permspeed;

    /* no message if monster is immobile (temp or perm) or unseen */
    if (give_msg && (mon->mspeed != oldspeed || petrify) && mon->data->mmove
        && !(mon->mfrozen || mon->msleeping) && canseemon(mon)) {
        /* fast to slow (skipping intermediate state) or vice versa */
        const char *howmuch =
            (mon->mspeed + oldspeed == MFAST + MSLOW) ? "much " : "";

        if (petrify) {
            /* mimic the player's petrification countdown; "slowing down"
               even if fast movement rate retained via worn speed boots */
            if (flags.verbose)
                pline("%s is slowing down.", Monnam(mon));
        } else if (adjust > 0 || mon->mspeed == MFAST)
            pline("%s is suddenly moving %sfaster.", Monnam(mon), howmuch);
        else
            pline("%s seems to be moving %sslower.", Monnam(mon), howmuch);

        /* might discover an object if we see the speed change happen */
        if (obj != 0)
            learnwand(obj);
    }
}

boolean
obj_has_prop(obj, which)
struct obj *obj;
int which;
{
    if (objects[obj->otyp].oc_oprop == which)
        return TRUE;

    if (!obj->oprops)
        return FALSE;

    switch (which) {
    case FIRE_RES:
        return !!(obj->oclass != WEAPON_CLASS
                  && !is_weptool(obj)
                  && (obj->oprops & ITEM_FIRE));
    case COLD_RES:
        return !!(obj->oclass != WEAPON_CLASS
                  && !is_weptool(obj)
                  && (obj->oprops & ITEM_FROST));
    case DRAIN_RES:
        return !!(obj->oclass != WEAPON_CLASS
                  && !is_weptool(obj)
                  && (obj->oprops & ITEM_DRLI));
    case SHOCK_RES:
        return !!(obj->oclass != WEAPON_CLASS
                  && !is_weptool(obj)
                  && (obj->oprops & ITEM_SHOCK));
    case POISON_RES:
        return !!(obj->oclass != WEAPON_CLASS
                  && !is_weptool(obj)
                  && (obj->oprops & ITEM_VENOM));
    case TELEPAT:
        return !!(obj->oprops & ITEM_ESP);
    case FUMBLING:
        return !!(obj->oprops & ITEM_FUMBLING);
    case HUNGER:
        return !!(obj->oprops & ITEM_HUNGER);
    case ADORNED:
        return !!(obj->oprops & ITEM_EXCEL);
    }
    return FALSE;
}

/* alchemy smock confers two properites, poison and acid resistance
   but objects[ALCHEMY_SMOCK].oc_oprop can only describe one of them;
   if it is poison resistance, alternate property is acid resistance;
   if someone changes it to acid resistance, alt becomes posion resist;
   if someone changes it to hallucination resistance, all bets are off
   [TODO: handle alternate properties conferred by dragon scales] */
#define altprop(o) \
    (((o)->otyp == ALCHEMY_SMOCK)                               \
     ? (POISON_RES + ACID_RES - objects[(o)->otyp].oc_oprop)    \
     : 0)

/* armor put on or taken off; might be magical variety
   [TODO: rename to 'update_mon_extrinsics()' and change all callers...] */
void
update_mon_intrinsics(mon, obj, on, silently)
struct monst *mon;
struct obj *obj;
boolean on, silently;
{
    int unseen;
    uchar  mask;
    struct obj *otmp;
    int which = (int) armor_provides_extrinsic(obj),
        altwhich = altprop(obj);
    long props = obj->oprops;
    int i = 0;

    unseen = !canseemon(mon);
    if (!which && !altwhich)
        goto maybe_blocks;

 again:
    if (on) {
        switch (which) {
        case INVIS:
            mon->minvis = !mon->invis_blkd;
            break;
        case FAST: {
            boolean save_in_mklev = in_mklev;
            if (silently)
                in_mklev = TRUE;
            mon_adjust_speed(mon, 0, obj);
            in_mklev = save_in_mklev;
            break;
        }
        case WWALKING:
            mon->mextrinsics |= MR2_WATERWALK;
            break;
        case JUMPING:
            mon->mextrinsics |= MR2_JUMPING;
            break;
        case DISPLACED:
            mon->mextrinsics |= MR2_DISPLACED;
            break;
        case TELEPAT:
            mon->mextrinsics |= MR2_TELEPATHY;
            break;
        case LEVITATION:
            mon->mextrinsics |= MR2_LEVITATE;
            if (!unseen)
                pline("%s starts to float in the air!", Monnam(mon));
            break;
        case FREE_ACTION:
            mon->mextrinsics |= MR2_FREE_ACTION;
            break;
        /* properties handled elsewhere */
        case ANTIMAGIC:
        case REFLECTING:
        case PROTECTION:
            break;
        /* properties which have no effect for monsters */
        case CLAIRVOYANT:
        case STEALTH:
        case HUNGER:
        case ADORNED:
            break;
        /* properties which should have an effect but aren't implemented */
        case FLYING:
            break;
        /* properties which maybe should have an effect but don't */
        case FUMBLING:
            break;
        case FIRE_RES:
        case COLD_RES:
        case SLEEP_RES:
        case DISINT_RES:
        case SHOCK_RES:
        case POISON_RES:
        case ACID_RES:
        case STONE_RES:
        case PSYCHIC_RES:
            /* 1 through 9 correspond to MR_xxx mask values */
            if (which >= 1 && which <= 9) {
                mask = (uchar) (1 << (which - 1));
                mon->mextrinsics |= (unsigned long) mask;
            }
            break;
        default:
            break;
        }
    } else { /* off */
        switch (which) {
        case INVIS:
            mon->minvis = mon->perminvis;
            break;
        case FAST: {
            boolean save_in_mklev = in_mklev;
            if (silently)
                in_mklev = TRUE;
            mon_adjust_speed(mon, 0, obj);
            in_mklev = save_in_mklev;
            break;
        }
        case WWALKING:
            mon->mextrinsics &= ~(MR2_WATERWALK);
            break;
        case JUMPING:
            mon->mextrinsics &= ~(MR2_JUMPING);
            break;
        case DISPLACED:
            mon->mextrinsics &= ~(MR2_DISPLACED);
            break;
        case TELEPAT:
            mon->mextrinsics &= ~(MR2_TELEPATHY);
            break;
        case LEVITATION:
            mon->mextrinsics &= ~(MR2_LEVITATE);
            if (!unseen)
                pline("%s floats gently back to the %s.",
                      Monnam(mon), surface(mon->mx, mon->my));
            break;
        case FREE_ACTION:
            mon->mextrinsics &= ~(MR2_FREE_ACTION);
            break;
        case FIRE_RES:
        case COLD_RES:
        case SLEEP_RES:
        case DISINT_RES:
        case SHOCK_RES:
        case POISON_RES:
        case ACID_RES:
        case STONE_RES:
        case PSYCHIC_RES:
            /*
             * Update monster's extrinsics (for worn objects only;
             * 'obj' itself might still be worn or already unworn).
             *
             * If an alchemy smock is being taken off, this code will
             * be run twice (via 'goto again') and other worn gear
             * gets tested for conferring poison resistance on the
             * first pass and acid resistance on the second.
             *
             * If some other item is being taken off, there will be
             * only one pass but a worn alchemy smock will be an
             * alternate source for either of those two resistances.
             */
            mask = (uchar) (1 << (which - 1));
            for (otmp = mon->minvent; otmp; otmp = otmp->nobj) {
                if (otmp == obj || !otmp->owornmask)
                    continue;
                if ((int) objects[otmp->otyp].oc_oprop == which)
                    break;
                if ((int) (obj_has_prop(otmp, which))) /* object properties */
                    break;
                /* check whether 'otmp' confers target property as an extra
                   one rather than as the one specified for it in objects[] */
                if (altprop(otmp) == which)
                    break;
            }
            if (!otmp)
                mon->mextrinsics &= ~((unsigned long) mask);
            break;
        default:
            break;
        }
    }

    /* worn alchemy smock/apron confers both poison resistance and acid
       resistance to the hero so do likewise for monster who wears one */
    if (altwhich && which != altwhich) {
        which = altwhich;
        goto again;
    }

 maybe_blocks:
    /* obj->owornmask has been cleared by this point, so we can't use it.
       However, since monsters don't wield armor, we don't have to guard
       against that and can get away with a blanket worn-mask value. */
    switch (w_blocks(obj, ~0L)) {
    case INVIS:
        mon->invis_blkd = on ? 1 : 0;
        mon->minvis = on ? 0 : mon->perminvis;
        break;
    default:
        break;
    }

    while (props) {
        if (!i)
            i = 1;
        else
            i <<= 1;

        if (i > ITEM_PROP_MASK)
            break;

        if (props & i) {
            which = 0;
            props &= ~(i);
            switch (i) {
            case ITEM_FIRE:
                if (obj->oclass != WEAPON_CLASS && !is_weptool(obj))
                    which = FIRE_RES;
                break;
            case ITEM_FROST:
                if (obj->oclass != WEAPON_CLASS && !is_weptool(obj))
                    which = COLD_RES;
                break;
            case ITEM_DRLI:
                if (obj->oclass != WEAPON_CLASS && !is_weptool(obj))
                    which = DRAIN_RES;
                break;
            case ITEM_SHOCK:
                if (obj->oclass != WEAPON_CLASS && !is_weptool(obj))
                    which = SHOCK_RES;
                break;
            case ITEM_VENOM:
                if (obj->oclass != WEAPON_CLASS && !is_weptool(obj))
                    which = POISON_RES;
                break;
            case ITEM_ESP:
                which = TELEPAT;
                break;
            case ITEM_FUMBLING:
                which = FUMBLING;
                break;
            case ITEM_HUNGER:
                which = HUNGER;
                break;
            case ITEM_EXCEL:
                which = ADORNED;
                break;
            }
            if (which)
                goto again;
        }
    }

    if (!on && mon == u.usteed && obj->otyp == SADDLE)
        dismount_steed(DISMOUNT_FELL);

    /* if couldn't see it but now can, or vice versa, update display */
    if (!silently && (unseen ^ !canseemon(mon)))
        newsym(mon->mx, mon->my);
}

#undef altprop

int
find_mac(mon)
register struct monst *mon;
{
    register struct obj *obj, *nextobj;
    int base = r_data(mon)->ac - mon->mprotection;
    int bonus, div, racial_bonus;
    long mwflags = mon->misc_worn_check;

    for (obj = mon->minvent; obj; obj = nextobj) {
        nextobj = obj->nobj;
        if (obj->owornmask & mwflags
            && obj->otyp != RIN_INCREASE_DAMAGE
            && obj->otyp != RIN_INCREASE_ACCURACY) {
            if (obj->otyp == AMULET_OF_GUARDING) {
                base -= 2; /* fixed amount, not impacted by erosion */
            } else {
                /* since armor_bonus is positive, subtracting it increases AC */
                base -= armor_bonus(obj);
            }
            /* racial armor bonuses, separate from regular bonuses,
               leaving drow gloves out of this on purpose */
            racial_bonus = 1;
            if (which_armor(mon, W_ARM)) {
                if ((racial_orc(mon)
                     && (obj->otyp == ORCISH_CHAIN_MAIL
                         || obj->otyp == ORCISH_RING_MAIL))
                    || (racial_drow(mon)
                        && (obj->otyp == DARK_ELVEN_CHAIN_MAIL
                            || obj->otyp == DARK_ELVEN_TUNIC))
                    || (racial_elf(mon) && obj->otyp == ELVEN_CHAIN_MAIL)
                    || (racial_dwarf(mon) && obj->otyp == DWARVISH_CHAIN_MAIL))
                    base -= racial_bonus;
            }
            if (which_armor(mon, W_ARMC)) {
                if ((racial_orc(mon) && obj->otyp == ORCISH_CLOAK)
                    || (racial_elf(mon) && obj->otyp == ELVEN_CLOAK)
                    || (racial_drow(mon) && obj->otyp == DARK_ELVEN_CLOAK)
                    || (racial_dwarf(mon) && obj->otyp == DWARVISH_CLOAK))
                    base -= racial_bonus;
            }
            if (which_armor(mon, W_ARMH)) {
                if ((racial_orc(mon) && obj->otyp == ORCISH_HELM)
                    || (racial_elf(mon) && obj->otyp == ELVEN_HELM)
                    || (racial_drow(mon) && obj->otyp == DARK_ELVEN_HELM)
                    || (racial_dwarf(mon) && obj->otyp == DWARVISH_HELM))
                    base -= racial_bonus;
            }
            if (which_armor(mon, W_ARMF)) {
                if ((racial_orc(mon) && obj->otyp == ORCISH_BOOTS)
                    || (racial_elf(mon) && obj->otyp == ELVEN_BOOTS)
                    || (racial_drow(mon) && obj->otyp == DARK_ELVEN_BOOTS)
                    || (racial_dwarf(mon) && obj->otyp == DWARVISH_BOOTS))
                    base -= racial_bonus;
            }
            if (which_armor(mon, W_ARMS)) {
                if ((racial_orc(mon)
                     && (obj->otyp == ORCISH_SHIELD
                         || obj->otyp == URUK_HAI_SHIELD))
                    || (racial_elf(mon) && obj->otyp == ELVEN_SHIELD)
                    || (racial_drow(mon) && obj->otyp == DARK_ELVEN_BRACERS)
                    || (racial_dwarf(mon) && obj->otyp == DWARVISH_ROUNDSHIELD))
                    base -= racial_bonus;
            }
        }
    }

    /* other spell effects */
    if (has_barkskin(mon))
        base -= 5;

    if (has_stoneskin(mon))
        base -= 8;

    /* because base now uses r_data(mon)->ac instead of
       mon->data->ac, make sure racial shopkeepers retain
       their buffed up AC */
    if (has_eshk(mon))
        base -= 10;

    /* Tweak the monster's AC a bit according to its level */
    div = mon->m_lev > 20 ? 4 : 3;
    bonus = ((mon->m_lev / 2)) / div;
    if (bonus > 20)
        bonus = 20;
    if (bonus < 0)
        bonus = 0;
    base -= bonus;

    return base;
}

/*
 * weapons are handled separately;
 * eyewear isn't used by monsters
 */

/* Wear the best object of each type that the monster has.  During creation,
 * the monster can put everything on at once; otherwise, wearing takes time.
 * This doesn't affect monster searching for objects--a monster may very well
 * search for objects it would not want to wear, because we don't want to
 * check which_armor() each round.
 *
 * We'll let monsters put on shirts and/or suits under worn cloaks, but
 * not shirts under worn suits.  This is somewhat arbitrary, but it's
 * too tedious to have them remove and later replace outer garments,
 * and preventing suits under cloaks makes it a little bit too easy for
 * players to influence what gets worn.  Putting on a shirt underneath
 * already worn body armor is too obviously buggy...
 */
void
m_dowear(mon, creation)
register struct monst *mon;
boolean creation;
{
    struct obj *mw = MON_WEP(mon);
    boolean cursed_glove = (which_armor(mon, W_ARMG)
			    && which_armor(mon, W_ARMG)->cursed);

#define RACE_EXCEPTION TRUE
    /* Note the restrictions here are the same as in dowear in do_wear.c
     * except for the additional restriction on intelligence.  (Players
     * are always intelligent, even if polymorphed).
     */
    if (r_verysmall(mon) || nohands(mon->data)
        || is_animal(mon->data) || is_ent(mon->data)
        || is_plant(mon->data))
        return;
    /* give mummies a chance to wear their wrappings
     * and let skeletons wear their initial armor */
    if (mindless(mon->data)
        && (!creation || (mon->data->mlet != S_MUMMY
                          && mon->data->mlet != S_SKELETON)))
        return;

    m_dowear_type(mon, W_AMUL, creation, FALSE);
    /* can't put on shirt if already wearing suit */
    if (!cantweararm(mon) && !(mon->misc_worn_check & W_ARM))
        m_dowear_type(mon, W_ARMU, creation, FALSE);
    /* treating small as a special case allows
       hobbits, gnomes, and kobolds to wear cloaks */
    if (!cantweararm(mon) || r_data(mon)->msize == MZ_SMALL)
        m_dowear_type(mon, W_ARMC, creation, FALSE);
    m_dowear_type(mon, W_ARMH, creation, FALSE);
    m_dowear_type(mon, W_ARMS, creation, FALSE);

    /* Two ring per monster; ring takes up a "hand" slot */
    if (!(mw && bimanual(mw) && mw->cursed && mw->otyp != CORPSE)
	&& !cursed_glove)
        m_dowear_type(mon, W_RINGL, creation, FALSE);
    if (!(mw && mw->cursed && mw->otyp != CORPSE) && !cursed_glove)
        m_dowear_type(mon, W_RINGR, creation, FALSE);

    m_dowear_type(mon, W_ARMG, creation, FALSE);
    if (!slithy(mon->data) && r_data(mon)->mlet != S_CENTAUR
        && mon->data != &mons[PM_DRIDER])
        m_dowear_type(mon, W_ARMF, creation, FALSE);
    if (!cantweararm(mon))
        m_dowear_type(mon, W_ARM, creation, FALSE);
    else
        m_dowear_type(mon, W_ARM, creation, RACE_EXCEPTION);
}

STATIC_OVL void
m_dowear_type(mon, flag, creation, racialexception)
struct monst *mon;
long flag;
boolean creation;
boolean racialexception;
{
    struct obj *old, *best, *obj;
    long oldmask = 0L;
    int m_delay = 0;
    int sawmon = canseemon(mon), sawloc = cansee(mon->mx, mon->my);
    boolean autocurse;
    char nambuf[BUFSZ];

    if (mon->mfrozen)
        return; /* probably putting previous item on */

    /* Get a copy of monster's name before altering its visibility */
    Strcpy(nambuf, See_invisible ? Monnam(mon) : mon_nam(mon));

    old = which_armor(mon, flag);
    if (old && old->cursed)
        return;
    if (old && flag == W_AMUL && old->otyp != AMULET_OF_GUARDING)
        return; /* no amulet better than life-saving or reflection */
    best = old;

    for (obj = mon->minvent; obj; obj = obj->nobj) {
        if (mon_hates_material(mon, obj->material))
            continue;

        switch (flag) {
        case W_AMUL:
            if (obj->oclass != AMULET_CLASS
                || (obj->otyp != AMULET_OF_LIFE_SAVING
                    && obj->otyp != AMULET_OF_REFLECTION
                    && obj->otyp != AMULET_OF_MAGIC_RESISTANCE
                    && obj->otyp != AMULET_OF_GUARDING
                    && obj->otyp != AMULET_OF_ESP
                    && obj->oartifact != ART_EYE_OF_THE_AETHIOPICA))
                continue;
            /* for 'best' to be non-Null, it must be an amulet of guarding;
               life-saving and reflection don't get here due to early return
               and other amulets of guarding can't be any better */
            if (!best || obj->otyp != AMULET_OF_GUARDING) {
                best = obj;
                if (best->otyp != AMULET_OF_GUARDING)
                    goto outer_break; /* life-saving or reflection; use it */
            }
            continue; /* skip post-switch armor handling */
        case W_ARMU:
            if (!is_shirt(obj))
                continue;
            break;
        case W_ARMC:
            if (!is_cloak(obj))
                continue;
            break;
        case W_ARMH:
            if (!is_helmet(obj))
                continue;
            /* changing alignment is not implemented for monsters;
               priests and minions could change alignment but wouldn't
               want to, so they reject helms of opposite alignment */
            if (obj->otyp == HELM_OF_OPPOSITE_ALIGNMENT
                && (mon->ispriest || mon->isminion))
                continue;
            /* (flimsy exception matches polyself handling) */
            if (has_horns(mon->data) && !is_flimsy(obj))
                continue;
            break;
        case W_ARMS:
            if (!(is_shield(obj) || is_bracer(obj)))
                continue;
            /* no two-handed weapons with shields */
            if (!is_bracer(obj))
                if (MON_WEP(mon) && bimanual(MON_WEP(mon)))
                    continue;
            break;
        case W_ARMG:
            if (!is_gloves(obj)
                /* monsters are too scared of the Hand of Vecna */
                || obj->otyp == MUMMIFIED_HAND)
                continue;
            break;
        case W_ARMF:
            if (!is_boots(obj))
                continue;
            break;
        case W_ARM:
            if (!is_suit(obj))
                continue;
            if (racialexception && (racial_exception(mon, obj) < 1))
                continue;
            /* monsters won't sacrifice flight for AC */
            if (big_wings(mon->data) && !Is_dragon_scales(obj)
                && obj->otyp != JACKET)
                continue;
            break;
        case W_RINGL:
        case W_RINGR:
            /* Monsters can put on only the following rings. */
            if (obj->oclass != RING_CLASS
                || (obj->otyp != RIN_INVISIBILITY
                    && obj->otyp != RIN_FIRE_RESISTANCE
                    && obj->otyp != RIN_COLD_RESISTANCE
                    && obj->otyp != RIN_POISON_RESISTANCE
                    && obj->otyp != RIN_SHOCK_RESISTANCE
                    && obj->otyp != RIN_REGENERATION
                    && obj->otyp != RIN_TELEPORTATION
                    && obj->otyp != RIN_TELEPORT_CONTROL
                    && obj->otyp != RIN_SLOW_DIGESTION
                    && obj->otyp != RIN_INCREASE_DAMAGE
                    && obj->otyp != RIN_INCREASE_ACCURACY
                    && obj->otyp != RIN_PROTECTION
                    && obj->otyp != RIN_LEVITATION
                    && obj->otyp != RIN_FREE_ACTION
                    && obj->otyp != RIN_ANCIENT
                    && obj->otyp != RIN_LUSTROUS))
                continue;
            if (mon->data == &mons[PM_NAZGUL]
                && obj->otyp == RIN_INVISIBILITY)
                continue;
            break;
        }
        if (obj->owornmask)
            continue;
        /* I'd like to define a VISIBLE_ARM_BONUS which doesn't assume the
         * monster knows obj->spe, but if I did that, a monster would keep
         * switching forever between two -2 caps since when it took off one
         * it would forget spe and once again think the object is better
         * than what it already has.
         */
        if (best && (armor_bonus(best) + extra_pref(mon, best)
                     >= armor_bonus(obj) + extra_pref(mon, obj)))
            continue;
        best = obj;
    }
 outer_break:
    if (!best || best == old)
        return;

    /* same auto-cursing behavior as for hero */
    autocurse = ((best->otyp == HELM_OF_OPPOSITE_ALIGNMENT
                  || best->otyp == DUNCE_CAP) && !best->cursed);
    /* if wearing a cloak, account for the time spent removing
       and re-wearing it when putting on a suit or shirt */
    if ((flag == W_ARM || flag == W_ARMU) && (mon->misc_worn_check & W_ARMC))
        m_delay += 2;
    /* when upgrading a piece of armor, account for time spent
       taking off current one */
    if (old) {
        m_delay += objects[old->otyp].oc_delay;

        oldmask = old->owornmask; /* needed later by artifact_light() */
        old->owornmask = 0L; /* avoid doname() showing "(being worn)" */
    }

    if (!creation) {
        if (sawmon) {
            char buf[BUFSZ];

            if (old)
                Sprintf(buf, " removes %s and", distant_name(old, doname));
            else
                buf[0] = '\0';
            pline("%s%s puts on %s.", Monnam(mon), buf,
                  distant_name(best, doname));
            if (autocurse)
                pline("%s %s %s %s for a moment.", s_suffix(Monnam(mon)),
                      simpleonames(best), otense(best, "glow"),
                      hcolor(NH_BLACK));
        } /* can see it */
        m_delay += objects[best->otyp].oc_delay;
        mon->mfrozen = m_delay;
        if (mon->mfrozen)
            mon->mcanmove = 0;
    }
    if (old) {
        update_mon_intrinsics(mon, old, FALSE, creation);

        /* owornmask was cleared above but artifact_light() expects it */
        old->owornmask = oldmask;
        if (old->lamplit && artifact_light(old))
            end_burn(old, FALSE);
        old->owornmask = 0L;
    }
    mon->misc_worn_check |= flag;
    best->owornmask |= flag;
    if (autocurse)
        curse(best);
    if (artifact_light(best) && !best->lamplit) {
        begin_burn(best, FALSE);
        vision_recalc(1);
        if (!creation && best->lamplit && cansee(mon->mx, mon->my)) {
            const char *adesc = arti_light_description(best);
            boolean shadow = (Is_dragon_armor(best)
                              && Dragon_armor_to_scales(best) == SHADOW_DRAGON_SCALES);

            if (shadow) {
                if (sawmon) /* could already see monster */
                    pline("%s %s to cast an aura of darkness.",
                          Yname2(best), otense(best, "begin"));
                else if (canseemon(mon)) /* didn't see it until new dark */
                    pline("%s %s giving off an aura of darkness.",
                          Yname2(best), otense(best, "are"));
                else if (sawloc) /* saw location but not invisible monster */
                    pline("%s begins to cast an aura of darkness.",
                          Something);
                else /* didn't see location until new dark */
                    pline("%s is giving off an aura of darkness.",
                          Something);
            } else {
                if (sawmon) /* could already see monster */
                    pline("%s %s to shine %s.", Yname2(best),
                          otense(best, "begin"), adesc);
                else if (canseemon(mon)) /* didn't see it until new light */
                    pline("%s %s shining %s.", Yname2(best),
                          otense(best, "are"), adesc);
                else if (sawloc) /* saw location but not invisible monster */
                    pline("%s begins to shine %s.", Something, adesc);
                else /* didn't see location until new light */
                    pline("%s is shining %s.", Something, adesc);
            }
        }
    }
    update_mon_intrinsics(mon, best, TRUE, creation);
    /* if couldn't see it but now can, or vice versa, */
    if (!creation && (sawmon ^ canseemon(mon))) {
        if (mon->minvis && !See_invisible) {
            pline("Suddenly you cannot see %s.", nambuf);
            makeknown(best->otyp);
        } /* else if (!mon->minvis)
           *     pline("%s suddenly appears!", Amonnam(mon)); */
    }
}
#undef RACE_EXCEPTION

struct obj *
which_armor(mon, flag)
struct monst *mon;
long flag;
{
    if (mon == &youmonst) {
        switch (flag) {
        case W_ARM:
            return uarm;
        case W_ARMC:
            return uarmc;
        case W_ARMH:
            return uarmh;
        case W_ARMS:
            return uarms;
        case W_ARMG:
            return uarmg;
        case W_ARMF:
            return uarmf;
        case W_ARMU:
            return uarmu;
        default:
            impossible("bad flag in which_armor");
            return 0;
        }
    } else {
        register struct obj *obj;

        for (obj = mon->minvent; obj; obj = obj->nobj)
            if (obj->owornmask & flag)
                return obj;
        return (struct obj *) 0;
    }
}

/* remove an item of armor and then drop it */
STATIC_OVL void
m_lose_armor(mon, obj)
struct monst *mon;
struct obj *obj;
{
    extract_from_minvent(mon, obj, TRUE, FALSE);
    place_object(obj, mon->mx, mon->my);
    /* call stackobj() if we ever drop anything that can merge */
    newsym(mon->mx, mon->my);
}

/* all objects with their bypass bit set should now be reset to normal */
void
clear_bypasses()
{
    struct obj *otmp, *nobj;
    struct monst *mtmp;

    /*
     * 'Object' bypass is also used for one monster function:
     * polymorph control of long worms.  Activated via setting
     * context.bypasses even if no specific object has been
     * bypassed.
     */

    for (otmp = fobj; otmp; otmp = nobj) {
        nobj = otmp->nobj;
        if (otmp->bypass) {
            otmp->bypass = 0;

            /* bypass will have inhibited any stacking, but since it's
             * used for polymorph handling, the objects here probably
             * have been transformed and won't be stacked in the usual
             * manner afterwards; so don't bother with this.
             * [Changing the fobj chain mid-traversal would also be risky.]
             */
#if 0
            if (objects[otmp->otyp].oc_merge) {
                xchar ox, oy;

                (void) get_obj_location(otmp, &ox, &oy, 0);
                stack_object(otmp);
                newsym(ox, oy);
            }
#endif /*0*/
        }
    }
    for (otmp = invent; otmp; otmp = otmp->nobj)
        otmp->bypass = 0;
    for (otmp = migrating_objs; otmp; otmp = otmp->nobj)
        otmp->bypass = 0;
    mchest->bypass = 0;
    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp))
            continue;
        for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj)
            otmp->bypass = 0;
        /* long worm created by polymorph has mon->mextra->mcorpsenm set
           to PM_LONG_WORM to flag it as not being subject to further
           polymorph (so polymorph zap won't hit monster to transform it
           into a long worm, then hit that worm's tail and transform it
           again on same zap); clearing mcorpsenm reverts worm to normal */
        if (mtmp->data == &mons[PM_LONG_WORM] && has_mcorpsenm(mtmp))
            MCORPSENM(mtmp) = NON_PM;
    }
    for (mtmp = migrating_mons; mtmp; mtmp = mtmp->nmon) {
        for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj)
            otmp->bypass = 0;
        /* no MCORPSENM(mtmp)==PM_LONG_WORM check here; long worms can't
           be just created by polymorph and migrating at the same time */
    }
    /* billobjs and mydogs chains don't matter here */
    context.bypasses = FALSE;
}

void
bypass_obj(obj)
struct obj *obj;
{
    obj->bypass = 1;
    context.bypasses = TRUE;
}

/* set or clear the bypass bit in a list of objects */
void
bypass_objlist(objchain, on)
struct obj *objchain;
boolean on; /* TRUE => set, FALSE => clear */
{
    if (on && objchain)
        context.bypasses = TRUE;
    while (objchain) {
        objchain->bypass = on ? 1 : 0;
        objchain = objchain->nobj;
    }
}

/* return the first object without its bypass bit set; set that bit
   before returning so that successive calls will find further objects */
struct obj *
nxt_unbypassed_obj(objchain)
struct obj *objchain;
{
    while (objchain) {
        if (!objchain->bypass) {
            bypass_obj(objchain);
            break;
        }
        objchain = objchain->nobj;
    }
    return objchain;
}

/* like nxt_unbypassed_obj() but operates on sortloot_item array rather
   than an object linked list; the array contains obj==Null terminator;
   there's an added complication that the array may have stale pointers
   for deleted objects (see Multiple-Drop case in askchain(invent.c)) */
struct obj *
nxt_unbypassed_loot(lootarray, listhead)
Loot *lootarray;
struct obj *listhead;
{
    struct obj *o, *obj;

    while ((obj = lootarray->obj) != 0) {
        for (o = listhead; o; o = o->nobj)
            if (o == obj)
                break;
        if (o && !obj->bypass) {
            bypass_obj(obj);
            break;
        }
        ++lootarray;
    }
    return obj;
}

void
mon_break_armor(mon, polyspot)
struct monst *mon;
boolean polyspot;
{
    register struct obj *otmp;
    struct permonst *mdat = mon->data;
    boolean vis = cansee(mon->mx, mon->my);
    boolean handless_or_tiny = (nohands(mdat) || r_verysmall(mon));
    const char *pronoun = mhim(mon), *ppronoun = mhis(mon);

    if (breakarm(mon)) {
        if ((otmp = which_armor(mon, W_ARM)) != 0) {
            if (Is_dragon_armor(otmp)
                && mdat == &mons[Dragon_armor_to_pm(otmp)])
                ; /* no message here;
                     "the dragon merges with his scaly armor" is odd
                     and the monster's previous form is already gone */
            else if (vis)
                pline("%s breaks out of %s armor!", Monnam(mon), ppronoun);
            else
                You_hear("a cracking sound.");
            m_useup(mon, otmp);
        }
        if ((otmp = which_armor(mon, W_ARMC)) != 0) {
            if (otmp->oartifact) {
                if (vis)
                    pline("%s %s falls off!", s_suffix(Monnam(mon)),
                          cloak_simple_name(otmp));
                if (polyspot)
                    bypass_obj(otmp);
                m_lose_armor(mon, otmp);
            } else {
                if (Is_dragon_armor(otmp)
                    && mdat == &mons[Dragon_armor_to_pm(otmp)]) {
                    ; /* same as above; no message here */
                } else if (vis)
                    pline("%s %s tears apart!", s_suffix(Monnam(mon)),
                          cloak_simple_name(otmp));
                else
                    You_hear("a ripping sound.");
                m_useup(mon, otmp);
            }
        }
        if ((otmp = which_armor(mon, W_ARMU)) != 0) {
            if (vis)
                pline("%s shirt rips to shreds!", s_suffix(Monnam(mon)));
            else
                You_hear("a ripping sound.");
            m_useup(mon, otmp);
        }
    } else if (sliparm(mon)) {
        if ((otmp = which_armor(mon, W_ARM)) != 0) {
            if (vis)
                pline("%s armor falls around %s!", s_suffix(Monnam(mon)),
                      pronoun);
            else
                You_hear("a thud.");
            if (polyspot)
                bypass_obj(otmp);
            m_lose_armor(mon, otmp);
        }
        if ((otmp = which_armor(mon, W_ARMC)) != 0) {
            if (vis) {
                if (is_whirly(mon->data))
                    pline("%s %s falls, unsupported!", s_suffix(Monnam(mon)),
                          cloak_simple_name(otmp));
                else
                    pline("%s shrinks out of %s %s!", Monnam(mon), ppronoun,
                          cloak_simple_name(otmp));
            }
            if (polyspot)
                bypass_obj(otmp);
            m_lose_armor(mon, otmp);
        }
        if ((otmp = which_armor(mon, W_ARMU)) != 0) {
            if (vis) {
                if (sliparm(mon))
                    pline("%s seeps right through %s shirt!", Monnam(mon),
                          ppronoun);
                else
                    pline("%s becomes much too small for %s shirt!",
                          Monnam(mon), ppronoun);
            }
            if (polyspot)
                bypass_obj(otmp);
            m_lose_armor(mon, otmp);
        }
    }
    if (handless_or_tiny) {
        /* [caller needs to handle weapon checks] */
        if ((otmp = which_armor(mon, W_ARMG)) != 0) {
            if (vis)
                pline("%s drops %s gloves%s!", Monnam(mon), ppronoun,
                      MON_WEP(mon) ? " and weapon" : "");
            if (polyspot)
                bypass_obj(otmp);
            m_lose_armor(mon, otmp);
        }
        if ((otmp = which_armor(mon, W_ARMS)) != 0) {
            if (vis)
                pline("%s can no longer %s %s %s!",
                      Monnam(mon), is_bracer(otmp) ? "wear" : "hold",
                      ppronoun, is_bracer(otmp) ? "bracers" : "shield");
            else
                You_hear("a clank.");
            if (polyspot)
                bypass_obj(otmp);
            m_lose_armor(mon, otmp);
        }
    }
    if (handless_or_tiny || has_horns(mdat)) {
        if ((otmp = which_armor(mon, W_ARMH)) != 0
            /* flimsy test for horns matches polyself handling */
            && (handless_or_tiny || !is_flimsy(otmp))) {
            if (vis)
                pline("%s helmet falls to the %s!", s_suffix(Monnam(mon)),
                      surface(mon->mx, mon->my));
            else
                You_hear("a clank.");
            if (polyspot)
                bypass_obj(otmp);
            m_lose_armor(mon, otmp);
        }
    }
    if (handless_or_tiny || slithy(mdat) || mdat->mlet == S_CENTAUR) {
        if ((otmp = which_armor(mon, W_ARMF)) != 0) {
            if (vis) {
                if (is_whirly(mon->data))
                    pline("%s boots fall away!", s_suffix(Monnam(mon)));
                else
                    pline("%s boots %s off %s feet!", s_suffix(Monnam(mon)),
                          r_verysmall(mon) ? "slide" : "are pushed", ppronoun);
            }
            if (polyspot)
                bypass_obj(otmp);
            m_lose_armor(mon, otmp);
        }
    }
    if (!can_wear_barding(mon)) {
        if ((otmp = which_armor(mon, W_BARDING)) != 0) {
            if (polyspot)
                bypass_obj(otmp);
            m_lose_armor(mon, otmp);
            if (vis)
                pline("%s barding falls off.", s_suffix(Monnam(mon)));
        }
    }
    if (!can_saddle(mon)) {
        if ((otmp = which_armor(mon, W_SADDLE)) != 0) {
            if (polyspot)
                bypass_obj(otmp);
            m_lose_armor(mon, otmp);
            if (vis)
                pline("%s saddle falls off.", s_suffix(Monnam(mon)));
        }
        if (mon == u.usteed)
            goto noride;
    } else if (mon == u.usteed && !can_ride(mon)) {
 noride:
        You("can no longer ride %s.", mon_nam(mon));
        if (touch_petrifies(u.usteed->data) && !Stone_resistance && rnl(3)) {
            char buf[BUFSZ];

            You("touch %s.", mon_nam(u.usteed));
            Sprintf(buf, "falling off %s", an(u.usteed->data->mname));
            instapetrify(buf);
        }
        dismount_steed(DISMOUNT_FELL);
    }
    return;
}

/* bias a monster's preferences towards armor and
   accessories that have special benefits */
int
extra_pref(mon, obj)
struct monst *mon;
struct obj *obj;
{
    struct obj *old;
    int rc = 1;

    if (!obj)
        return 0;
    if (obj_has_prop(obj, ANTIMAGIC)
        && !(resists_magm(mon) || defended(mon, AD_MAGM)))
        return 50;
    if (obj_has_prop(obj, REFLECTING)
        && !mon_reflects(mon, (char *) 0))
        return 40;
    if (obj_has_prop(obj, DISPLACED)
        && !has_displacement(mon))
        return 30;
    if (obj_has_prop(obj, FREE_ACTION)
        && !has_free_action(mon))
        return 30;
    if (obj_has_prop(obj, FAST)
        && mon->permspeed != MFAST)
        return 20;
    if (obj_has_prop(obj, LEVITATION)
        && grounded(mon->data))
        return 15;
    if (obj_has_prop(obj, JUMPING)
        && !can_jump(mon))
        return 10;
    if (obj_has_prop(obj, WWALKING)
        && !can_wwalk(mon))
        return 10;
    if (obj->oclass != RING_CLASS)
        return 0;

    /* Find out whether the monster already has some resistance. */
    old = which_armor(mon, W_RINGL);
    if (old)
        update_mon_intrinsics(mon, old, FALSE, TRUE);
    old = which_armor(mon, W_RINGR);
    if (old)
        update_mon_intrinsics(mon, old, FALSE, TRUE);
    /* This list should match the list in m_dowear_type. */
    switch (obj->otyp) {
    case RIN_FIRE_RESISTANCE:
        if (!(resists_fire(mon) || defended(mon, AD_FIRE)))
            rc = (dmgtype(youmonst.data, AD_FIRE)
                  || wielding_artifact(ART_FIRE_BRAND)
                  || wielding_artifact(ART_XIUHCOATL)
                  || wielding_artifact(ART_ANGELSLAYER)
                  || wielding_artifact(ART_DICHOTOMY)
                  || (u.twoweap && uswapwep->oprops & ITEM_FIRE)
                  || (uwep && uwep->oprops & ITEM_FIRE)) ? 25 : 5;
        break;
    case RIN_COLD_RESISTANCE:
        if (!(resists_cold(mon) || defended(mon, AD_COLD)))
            rc = (dmgtype(youmonst.data, AD_COLD)
                  || wielding_artifact(ART_FROST_BRAND)
                  || wielding_artifact(ART_DICHOTOMY)
                  || (u.twoweap && uswapwep->oprops & ITEM_FROST)
                  || (uwep && uwep->oprops & ITEM_FROST)) ? 25 : 5;
        break;
    case RIN_POISON_RESISTANCE:
        if (!(resists_poison(mon) || defended(mon, AD_DRST)))
            rc = (dmgtype(youmonst.data, AD_DRST)
                  || dmgtype(youmonst.data, AD_DRCO)
                  || dmgtype(youmonst.data, AD_DRDX)
                  || (u.twoweap && uswapwep->oprops & ITEM_VENOM)
                  || (uwep && uwep->oprops & ITEM_VENOM)) ? 25 : 5;
        break;
    case RIN_SHOCK_RESISTANCE:
        if (!(resists_elec(mon) || defended(mon, AD_ELEC)))
            rc = (dmgtype(youmonst.data, AD_ELEC)
                  || wielding_artifact(ART_MJOLLNIR)
                  || wielding_artifact(ART_KEOLEWA)
                  || (u.twoweap && uswapwep->oprops & ITEM_SHOCK)
                  || (uwep && uwep->oprops & ITEM_SHOCK)) ? 25 : 5;
        break;
    case RIN_REGENERATION:
        rc = !mon_prop(mon, REGENERATION) ? 25 : 5;
        break;
    case RIN_INVISIBILITY:
    case RIN_LUSTROUS:
        if (mon->mtame || mon->mpeaceful)
            /* Monsters actually don't know if you can
             * see invisible, but for tame or peaceful monsters
             * we'll make reservations  */
            rc = See_invisible ? 10 : 0;
        else
            rc = 30;
        break;
    case RIN_INCREASE_DAMAGE:
    case RIN_INCREASE_ACCURACY:
    case RIN_PROTECTION:
        if (obj->spe > 0)
            rc = 10 + 3 * (obj->spe);
        else
            rc = 0;
        break;
    case RIN_TELEPORTATION:
        if (!mon_prop(mon, TELEPORT))
            rc = obj->cursed ? 5 : 15;
        break;
    case RIN_TELEPORT_CONTROL:
        if (!mon_prop(mon, TELEPORT_CONTROL))
            rc = mon_prop(mon, TELEPORT) ? 20 : 5;
        break;
    case RIN_SLOW_DIGESTION:
        rc = dmgtype(youmonst.data, AD_DGST) ? 35 : 25;
        break;
    case RIN_LEVITATION:
        rc = grounded(mon->data) ? 20 : 0;
        break;
    case RIN_FREE_ACTION:
    case RIN_ANCIENT:
        rc = 30;
        break;
    }
    old = which_armor(mon, W_RINGL);
    if (old)
        update_mon_intrinsics(mon, old, TRUE, TRUE);
    old = which_armor(mon, W_RINGR);
    if (old)
        update_mon_intrinsics(mon, old, TRUE, TRUE);
    return rc;
}

/*
 * Exceptions to things based on race.
 * Correctly checks polymorphed player race.
 * Returns:
 *       0 No exception, normal rules apply.
 *       1 If the race/object combination is acceptable.
 *      -1 If the race/object combination is unacceptable.
 */
int
racial_exception(mon, obj)
struct monst *mon;
struct obj *obj;
{
    const struct permonst *ptr = raceptr(mon);

    /* Acceptable Exceptions: */
    /* Allow hobbits to wear elven armor - LoTR */
    if ((is_hobbit(ptr) || racial_hobbit(mon)) && is_elven_armor(obj))
        return 1;
    /* Unacceptable Exceptions: */
    /* Checks for object that certain races should never use go here */
    if ((is_elf(ptr) || racial_elf(mon))
        && (is_orcish_armor(obj) || is_drow_armor(obj)))
        return -1;

    if ((is_drow(ptr) || racial_drow(mon))
        && (is_orcish_armor(obj) || is_elven_armor(obj)))
        return -1;

    if ((is_orc(ptr) || racial_orc(mon))
        && (is_elven_armor(obj) || is_drow_armor(obj)))
        return -1;

    return 0;
}

/* Remove an object from a monster's inventory. */
void
extract_from_minvent(mon, obj, do_intrinsics, silently)
struct monst *mon;
struct obj *obj;
boolean do_intrinsics; /* whether to call update_mon_intrinsics */
boolean silently; /* doesn't affect all possible messages, just
                     update_mon_intrinsics's messages */
{
    long unwornmask = obj->owornmask;

    /*
     * At its core this is just obj_extract_self(), but it also handles
     * any updates that need to happen if the gear is equipped or in
     * some other sort of state that needs handling.
     * Note that like obj_extract_self(), this leaves obj free.
     */

    if (obj->where != OBJ_MINVENT) {
        impossible("extract_from_minvent called on object not in minvent");
        return;
    }
    /* handle gold dragon scales/shield of light (lit when worn) before
       clearing obj->owornmask because artifact_light() expects that to
       be W_ARM / W_ARMC / W_ARMS */
    if (((unwornmask & W_ARM) != 0 || (unwornmask & W_ARMC) != 0
         || (unwornmask & W_ARMS) != 0)
        && obj->lamplit && artifact_light(obj))
        end_burn(obj, FALSE);

    obj_extract_self(obj);
    obj->owornmask = 0L;
    if (unwornmask) {
        if (!DEADMONSTER(mon) && do_intrinsics)
            update_mon_intrinsics(mon, obj, FALSE, silently);
        mon->misc_worn_check &= ~unwornmask;
        /* give monster a chance to wear other equipment on its next
           move instead of waiting until it picks something up */
        check_gear_next_turn(mon);
    }
    obj_no_longer_held(obj);
    if (unwornmask & W_WEP)
        mwepgone(mon); /* unwields and sets weapon_check to NEED_WEAPON */
}

/* Return the armor bonus of a piece of armor: the amount by which it directly
   lowers the AC of the wearer */
int
armor_bonus(armor)
struct obj *armor;
{
    int bon = 0;
    if (!armor) {
        impossible("armor_bonus was passed a null obj");
        return 0;
    }
    /* start with its base AC value */
    bon = objects[armor->otyp].a_ac;
    /* adjust for material */
    bon += material_bonus(armor);
    /* subtract erosion */
    bon -= (int) greatest_erosion(armor);
    /* erosion is not allowed to make the armor worse than wearing nothing;
     * only negative enchantment can do that. */
    if (bon < 0) {
        bon = 0;
    }
    /* add enchantment (could be negative) */
    bon += armor->spe;
    /* adjust for armor quality */
    if (armor->forged_qual == 1)
        bon += 1;
    else if (armor->forged_qual == 2)
        bon += 2;
    else if (armor->forged_qual < 0)
        bon -= 2;
    /* add bonus for dragon-scaled armor, slightly more AC
       than dragon scales by themselves, as the scales harden
       as they merge with worn armor */
    if (Is_dragon_scaled_armor(armor))
        bon += 5;
    return bon;
}

/* Determine the extrinsic property a piece of armor provides.
   Based on item_provides_extrinsic in NetHack Fourk, but less general */
long
armor_provides_extrinsic(armor)
struct obj *armor;
{
    long prop;

    if (!armor)
        return 0;

    prop = objects[armor->otyp].oc_oprop;
    if (!prop && Is_dragon_armor(armor))
        return objects[Dragon_armor_to_scales(armor)].oc_oprop;

    return prop;
}

/*worn.c*/
