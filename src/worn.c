/* NetHack 5.0	worn.c	$NHDT-Date: 1781973075 2026/06/20 16:31:15 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.124 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2013. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

staticfn void m_lose_armor(struct monst *, struct obj *, boolean) NONNULLPTRS;
staticfn void clear_bypass(struct obj *) NO_NNARGS;
staticfn void m_dowear_type(struct monst *, long, boolean, boolean)
                                                                  NONNULLARG1;
staticfn struct obj *find_best_armor_recurse(struct monst *, struct obj *,
                                             struct obj *, long, boolean,
                                             boolean) NONNULLARG1;

static const struct worn {
    long w_mask;
    struct obj **w_obj;
    const char *w_what; /* for failing sanity check's feedback */
} worn[] = { { W_ARM, &uarm, "suit" },
             { W_ARMC, &uarmc, "cloak" },
             { W_ARMH, &uarmh, "helmet" },
             { W_ARMS, &uarms, "shield" },
             { W_ARMG, &uarmg, "gloves" },
             { W_ARMF, &uarmf, "boots" },
             { W_ARMU, &uarmu, "shirt" },
             { W_RINGL, &uleft, "left ring" },
             { W_RINGR, &uright, "right ring" },
             { W_WEP, &uwep, "weapon" },
             { W_SWAPWEP, &uswapwep, "alternate weapon" },
             { W_QUIVER, &uquiver, "quiver" },
             { W_AMUL, &uamul, "amulet" },
             /* blindfold|towel|lenses|goggles */
             { W_TOOL, &ublindf, "facewear" },
             { W_BALL, &uball, "chained ball" },
             { W_CHAIN, &uchain, "attached chain" },
             { 0, 0, (char *) 0 }
};

/* This only allows for one blocking item per property */
#define w_blocks(o, m) \
    ((o->otyp == MUMMY_WRAPPING && ((m) & W_ARMC) != 0L) ? INVIS        \
     : (o->otyp == CORNUTHAUM && ((m) & W_ARMH) != 0L                   \
        && !Role_if(PM_WIZARD)) ? CLAIRVOYANT                           \
       : (is_art(o, ART_EYES_OF_THE_OVERWORLD)                          \
          && ((m) & W_TOOL) != 0L) ? BLINDED                            \
         : 0)
/* note: monsters don't have clairvoyance, so dependency on hero's role here
   has no significant effect on their use of w_blocks() */

/* calc the range of hero's unblind telepathy */
void
recalc_telepat_range(void)
{
    const struct worn *wp;
    int nobjs = 0;

    for (wp = worn; wp->w_mask; wp++) {
        struct obj *oobj = *(wp->w_obj);

        if (oobj && objects[oobj->otyp].oc_oprop == TELEPAT)
            nobjs++;
    }
    /* count all artifacts with SPFX_ESP as one */
    if (ETelepat & W_ART)
        nobjs++;

    if (nobjs)
        u.unblind_telepat_range = (BOLT_LIM * BOLT_LIM) * nobjs;
    else
        u.unblind_telepat_range = -1;
}

/* Updated to use the extrinsic and blocked fields. */
void
setworn(struct obj *obj, long mask)
{
    const struct worn *wp;
    struct obj *oobj;
    int p;

    if ((mask & I_SPECIAL) != 0 && (mask & (W_ARM | W_ARMC)) != 0) {
        /* restoring saved game; no properties are conferred via skin
           (a dragon-scaled suit embeds from the body slot, bare dragon
           scales from the cloak slot) */
        uskin = obj;
        /* assert( !uarm ); */
    } else {
        for (wp = worn; wp->w_mask; wp++) {
            if (wp->w_mask & mask) {
                oobj = *(wp->w_obj);
                if (oobj && !(oobj->owornmask & wp->w_mask))
                    impossible("Setworn: mask=0x%08lx.", wp->w_mask);
                /* slot already correctly occupied; skip teardown+grant
                   so we don't re-call set_artifact_intrinsic or churn
                   the slot's extrinsics */
                if (oobj == obj && obj != (struct obj *) 0)
                    continue;
                if (oobj) {
                    if (u.twoweap
                        && (oobj->owornmask & (W_WEP | W_SWAPWEP))) {
                        /* clear the flag first so that untwoweapon()
                           skips its feedback message */
                        set_twoweap(FALSE); /* u.twoweap = FALSE */
                        /* reseat the off-hand weapon to turn off its
                           W_SWAPWEP extrinsics */
                        untwoweapon();
                    }
                    oobj->owornmask &= ~wp->w_mask;
                    /* W_SWAPWEP is always torn down: u.twoweap may have
                       been cleared just above (or by the caller) while
                       the off-hand extrinsics are still seated */
                    if (wp->w_mask & ~W_QUIVER) {
                        p = armor_provides_extrinsic(oobj);
                        /* if the hero removed an extrinsic-granting item,
                           nearby monsters will notice and attempt attacks of
                           that type again; the off-hand slot is torn down
                           on every reseat, so only a grant that was
                           actually seated counts */
                        if (u.uprops[p].extrinsic & wp->w_mask)
                            monstunseesu_prop(p);
                        /* leave as "x = x <op> y", here and below, for broken
                         * compilers */
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
                     * The off-hand weapon conveys intrinsics only while
                     * two-weapon combat is active.
                     */
                    if ((wp->w_mask & ~(W_SWAPWEP | W_QUIVER))
                        || ((wp->w_mask & W_SWAPWEP) && u.twoweap)) {
                        if (obj->oclass == WEAPON_CLASS || is_weptool(obj)
                            || mask != W_WEP) {
                            p = armor_provides_extrinsic(obj);
                            u.uprops[p].extrinsic =
                                u.uprops[p].extrinsic | wp->w_mask;
                            if ((p = w_blocks(obj, mask)) != 0)
                                u.uprops[p].blocked |= wp->w_mask;
                        }
                        if (obj->oartifact || obj->oprops)
                            set_artifact_intrinsic(obj, 1, mask);
                    }
                }
            }
        }
        if (obj && (obj->owornmask & W_ARMOR) != 0L)
            u.uroleplay.nudist = FALSE;
        /* tux -> tuxedo -> "monkey suit" -> monk's suit */
        iflags.tux_penalty = (uarm && Role_if(PM_MONK) && gu.urole.spelarmr);
    }
    if ((flags.weaponstatus && (mask & W_WEP) != 0L)
        || (flags.armorstatus && (mask & W_ARMOR) != 0L))
        disp.botl = TRUE;
    update_inventory();
    recalc_telepat_range();
}

/* called e.g. when obj is destroyed */
/* Updated to use the extrinsic and blocked fields. */
void
setnotworn(struct obj *obj)
{
    const struct worn *wp;
    int p;
    long unworn = 0L;

    if (!obj)
        return;
    if (u.twoweap && (obj == uwep || obj == uswapwep)) {
        struct obj *tmp = uswapwep;

        /* mirror untwoweapon()'s teardown without its message so the
           off-hand W_SWAPWEP extrinsics are revoked instead of staying
           seated with two-weapon combat off; when obj is the off-hand
           weapon itself the loop below finishes clearing its slot */
        set_twoweap(FALSE); /* u.twoweap = FALSE */
        setuswapwep((struct obj *) 0);
        setuswapwep(tmp);
    }
    for (wp = worn; wp->w_mask; wp++)
        if (obj == *(wp->w_obj)) {
            /* in case wearing or removal is in progress or removal
               is pending (via 'A' command for multiple items) */
            cancel_doff(obj, wp->w_mask);

            *(wp->w_obj) = (struct obj *) 0;
            unworn |= wp->w_mask;
            p = armor_provides_extrinsic(obj);
            /* remove this extrinsic from seenres, if it was seated */
            if (u.uprops[p].extrinsic & wp->w_mask)
                monstunseesu_prop(p);
            u.uprops[p].extrinsic = u.uprops[p].extrinsic & ~wp->w_mask;
            obj->owornmask &= ~wp->w_mask;
            if (obj->oartifact || obj->oprops)
                set_artifact_intrinsic(obj, 0, wp->w_mask);
            if ((p = w_blocks(obj, wp->w_mask)) != 0)
                u.uprops[p].blocked &= ~wp->w_mask;
        }
    if (!uarm)
        iflags.tux_penalty = FALSE;
    if ((flags.weaponstatus && (unworn & W_WEP) != 0L)
        || (flags.armorstatus && (unworn & W_ARMOR) != 0L))
        disp.botl = TRUE;
    update_inventory();
    recalc_telepat_range();
}

/* called when saving with FREEING flag set has just discarded inventory */
void
allunworn(void)
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

/* return item worn in slot indicated by wornmask; needed by poly_obj() */
struct obj *
wearmask_to_obj(long wornmask)
{
    const struct worn *wp;

    for (wp = worn; wp->w_mask; wp++)
        if (wp->w_mask & wornmask)
            return *wp->w_obj;
    return (struct obj *) 0;
}

/* convert an armor wornmask to corresponding category */
int
wornmask_to_armcat(long mask)
{
    int cat = 0;

    switch (mask & W_ARMOR) {
    case W_ARM:
        cat = ARM_SUIT;
        break;
    case W_ARMC:
        cat = ARM_CLOAK;
        break;
    case W_ARMH:
        cat = ARM_HELM;
        break;
    case W_ARMS:
        cat = ARM_SHIELD;
        break;
    case W_ARMG:
        cat = ARM_GLOVES;
        break;
    case W_ARMF:
        cat = ARM_BOOTS;
        break;
    case W_ARMU:
        cat = ARM_SHIRT;
        break;
    }
    return cat;
}

/* convert an armor category to corresponding wornmask */
long
armcat_to_wornmask(int cat)
{
    long mask = 0L;

    switch (cat) {
    case ARM_SUIT:
        mask = W_ARM;
        break;
    case ARM_CLOAK:
        mask = W_ARMC;
        break;
    case ARM_HELM:
        mask = W_ARMH;
        break;
    case ARM_SHIELD:
        mask = W_ARMS;
        break;
    case ARM_GLOVES:
        mask = W_ARMG;
        break;
    case ARM_BOOTS:
        mask = W_ARMF;
        break;
    case ARM_SHIRT:
        mask = W_ARMU;
        break;
    }
    return mask;
}

/* return a bitmask of the equipment slot(s) a given item might be worn in */
long
wearslot(struct obj *obj)
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
        if (otyp == BLINDFOLD || otyp == TOWEL || otyp == LENSES
            || otyp == GOGGLES)
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

/* for 'sanity_check' option, called by you_sanity_check() */
void
check_wornmask_slots(void)
{
    /* we'll skip ball and chain here--they warrant separate sanity check */
#define IGNORE_SLOTS (W_ART | W_ARTI | W_SADDLE | W_BALL| W_CHAIN)
    char whybuf[BUFSZ];
    const struct worn *wp;
    struct obj *o, *otmp;
    long m;

    for (wp = worn; wp->w_mask; wp++) {
        m = wp->w_mask;
        if ((m & IGNORE_SLOTS) != 0L && (m & ~IGNORE_SLOTS) == 0L)
            continue;
        if ((o = *wp->w_obj) != 0) {
            whybuf[0] = '\0';
            /* slot pointer (uarm, uwep, &c) is populated; check that object
               is in inventory and has the relevant owornmask bit set */
            for (otmp = gi.invent; otmp; otmp = otmp->nobj)
                if (otmp == o)
                    break;
            if (!otmp)
                Sprintf(whybuf, "%s (%s) not found in invent",
                        wp->w_what, fmt_ptr(o));
            else if ((o->owornmask & m) == 0L)
                Sprintf(whybuf, "%s bit not set in owornmask [0x%08lx]",
                        wp->w_what, o->owornmask);
            else if ((o->owornmask & ~(m | IGNORE_SLOTS)) != 0L)
                Sprintf(whybuf, "%s wrong bit set in owornmask [0x%08lx]",
                        wp->w_what, o->owornmask);
            if (whybuf[0])
                impossible("Worn-slot insanity: %s.", whybuf);
        } /* o != NULL */

        /* check whether any item other than the one in the slot pointer
           claims to be worn/wielded in this slot; make this test whether
           'o' is Null or not; [sanity_check_worn(mkobj.c) for object by
           object checking will most likely have already caught this] */
        for (otmp = gi.invent; otmp; otmp = otmp->nobj) {
            if (otmp != o && (otmp->owornmask & m) != 0L
                /* embedded scales owornmask is W_ARM|I_SPECIAL (a
                   dragon-scaled suit) or W_ARMC|I_SPECIAL (bare scales)
                   so would give a false complaint about item other than
                   uarm/uarmc having that bit set if we didn't screen it
                   out here */
                && ((m != W_ARM && m != W_ARMC) || otmp != uskin
                    || (otmp->owornmask & I_SPECIAL) == 0L)) {
                Sprintf(whybuf, "%s [0x%08lx] has %s mask 0x%08lx bit set",
                        simpleonames(otmp), otmp->owornmask, wp->w_what, m);
                impossible("Worn-slot insanity: %s.", whybuf);
            }
        }
    } /* for wp in worn[] */

#ifdef EXTRA_SANITY_CHECKS
    if (uskin) {
        const char *what = "embedded scales";

        o = uskin;
        /* a dragon-scaled suit embeds from the body slot, bare scales
           from the cloak slot */
        m = I_SPECIAL | ((o->owornmask & W_ARMC) ? W_ARMC : W_ARM);
        whybuf[0] = '\0';
        for (otmp = gi.invent; otmp; otmp = otmp->nobj)
            if (otmp == o)
                break;
        if (!otmp)
            Sprintf(whybuf, "%s (%s) not found in invent",
                    what, fmt_ptr(o));
        else if ((o->owornmask & m) != m)
            Sprintf(whybuf, "%s bits not set in owornmask [0x%08lx]",
                    what, o->owornmask);
        else if ((o->owornmask & ~(m | IGNORE_SLOTS)) != 0L)
            Sprintf(whybuf, "%s wrong bit set in owornmask [0x%08lx]",
                    what, o->owornmask);
        else if ((m & W_ARMC) ? !Is_dragon_scales(o)
                              : !Is_dragon_scaled_armor(o))
            Sprintf(whybuf, "%s (%s) %s not %s",
                    what, simpleonames(o), otense(o, "are"),
                    (m & W_ARMC) ? "dragon scales" : "dragon-scaled armor");
        /* armor_to_dragon() reads uskin first and encodes the chromatic,
           celestial and no-poly substitutions the merge used */
        else if (armor_to_dragon(&gy.youmonst) != u.umonnum)
            Sprintf(whybuf, "%s, hero is not %s",
                    what, an(mons[u.umonnum].pmnames[NEUTRAL]));
        if (whybuf[0])
            impossible("Worn-slot insanity: %s.", whybuf);
    } /* uskin */
#endif /* EXTRA_SANITY_CHECKS */

#ifdef EXTRA_SANITY_CHECKS
    /* dual wielding: not a slot but lots of things to verify */
    if (u.twoweap) {
        const char *why = NULL;

        if (!uwep || !uswapwep) {
            Sprintf(whybuf, "without %s%s%s",
                    !uwep ? "uwep" : "",
                    (!uwep && !uswapwep) ? " and without " : "",
                    !uswapwep ? "uswapwep" : "");
            why = whybuf;
        } else if (uarms && !is_bracer(uarms))
            why = "while wearing shield";
        else if (uwep->oclass != WEAPON_CLASS && !is_weptool(uwep))
            why = "uwep is not a weapon";
        else if (is_launcher(uwep) || is_ammo(uwep) || is_missile(uwep))
            why = "uwep is not a melee weapon";
        else if (bimanual(uwep))
            why = "uwep is two-handed";
        else if (uswapwep->oclass != WEAPON_CLASS && !is_weptool(uswapwep))
            why = "uswapwep is not a weapon";
        else if (is_launcher(uswapwep) || is_ammo(uswapwep)
                 || is_missile(uswapwep))
            why = "uswapwep is not a melee weapon";
        else if (bimanual(uswapwep))
            why = "uswapwep is two-handed";
        else if (!could_twoweap(gy.youmonst.data))
            why = "without two weapon attacks";

        if (why)
            impossible("Two-weapon insanity: %s.", why);
    }
#endif /* EXTRA_SANITY_CHECKS */
    return;
#undef IGNORE_SLOTS
} /* check_wornmask_slots() */

void
mon_set_minvis(
    struct monst *mon,
    boolean cursed_potion)
{
    mon->perminvis = !cursed_potion ? 1 : 0;
    if (!mon->invis_blkd) {
        mon->minvis = mon->perminvis;
        newsym(mon->mx, mon->my); /* make it disappear */
        if (mon->wormno)
            see_wsegs(mon); /* and any tail too */
    }
}

void
mon_adjust_speed(
    struct monst *mon,
    int adjust,        /* positive => increase speed, negative => decrease */
    struct obj *obj)   /* item to make known if effect can be seen */
{
    struct obj *otmp;
    boolean give_msg = !gi.in_mklev, petrify = FALSE;
    unsigned int oldspeed = mon->mspeed;

    switch (adjust) {
    case 3:
        give_msg = FALSE; /* recovering from sewage */
        break;
    case 2:
        mon->permspeed = MFAST;
        give_msg = FALSE; /* special-case monster creation */
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
            && Dragon_armor_to_scales(otmp) == BLUE_DRAGON_SCALES)
            break;
    }
    if (otmp) /* speed boots or blue-scaled armor */
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
                pline_mon(mon, "%s is slowing down.", Monnam(mon));
        } else if (adjust > 0 || mon->mspeed == MFAST)
            pline_mon(mon, "%s is suddenly moving %sfaster.",
                      Monnam(mon), howmuch);
        else
            pline_mon(mon, "%s seems to be moving %sslower.",
                      Monnam(mon), howmuch);

        /* might discover an object if we see the speed change happen */
        if (obj != 0)
            learnwand(obj);
    }
}

/* whether an object bestows the given property, either inherently
   or via one of its object properties; offensive damage properties
   on weapons do not confer the corresponding resistance */
boolean
obj_has_prop(struct obj *obj, int which)
{
    /* dragon-scaled armor confers its scales' property */
    if (armor_provides_extrinsic(obj) == which)
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

/* alchemy smock confers two properties, poison and acid resistance
   but objects[ALCHEMY_SMOCK].oc_oprop can only describe one of them;
   if it is poison resistance, alternate property is acid resistance;
   if someone changes it to acid resistance, alt becomes poison resist;
   if someone changes it to hallucination resistance, all bets are off
   [TODO: handle alternate properties conferred by dragon scales/mail] */
#define altprop(o) \
    (((o)->otyp == ALCHEMY_SMOCK)                               \
     ? (POISON_RES + ACID_RES - objects[(o)->otyp].oc_oprop)    \
     : 0)

/* armor put on or taken off; might be magical variety */
void
update_mon_extrinsics(
    struct monst *mon,
    struct obj *obj,   /* armor being worn or taken off */
    boolean on,
    boolean silently)
{
    int unseen, unsensed;
    uchar mask;
    struct obj *otmp;
    int which = armor_provides_extrinsic(obj),
        altwhich = altprop(obj);
    long props = obj->oprops;
    long i = 0L;

    unseen = !canseemon(mon);
    unsensed = !sensemon(mon);
    if (!which && !altwhich)
        goto maybe_blocks;

 again:
    if (on) {
        switch (which) {
        case INVIS:
            mon->minvis = !mon->invis_blkd;
            break;
        case FAST: {
            boolean save_in_mklev = gi.in_mklev;
            if (silently)
                gi.in_mklev = TRUE;
            mon_adjust_speed(mon, 0, obj);
            gi.in_mklev = save_in_mklev;
            break;
        }
        /* properties handled elsewhere */
        case ANTIMAGIC:
        case REFLECTING:
        case PROTECTION:
            break;
        /* properties which have no effect for monsters */
        case CLAIRVOYANT:
        case STEALTH:
            break;
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
        case LEVITATION: {
            boolean was_levitating = can_levitate(mon);

            mon->mextrinsics |= MR2_LEVITATE;
            if (!unseen && !was_levitating)
                pline_mon(mon, "%s starts to float in the air!", Monnam(mon));
            break;
        }
        case FREE_ACTION:
            mon->mextrinsics |= MR2_FREE_ACTION;
            break;
        case FLYING: {
            boolean could_fly = (is_flyer(mon->data) || is_floater(mon->data)
                                 || can_levitate(mon) || can_fly(mon));

            mon->mextrinsics |= MR2_FLY;
            if (!unseen && !could_fly)
                pline_mon(mon, "%s starts to fly!", Monnam(mon));
            break;
        }
        /* properties which maybe should have an effect but don't */
        case FUMBLING:
            break;
        default:
            mon->mextrinsics |= (unsigned short) res_to_mr(which);
            break;
        }
    } else { /* off */
        switch (which) {
        case INVIS:
            mon->minvis = mon->perminvis;
            break;
        case FAST: {
            boolean save_in_mklev = gi.in_mklev;
            if (silently)
                gi.in_mklev = TRUE;
            mon_adjust_speed(mon, 0, obj);
            gi.in_mklev = save_in_mklev;
            break;
        }
        case WWALKING:
            mon->mextrinsics &= ~(MR2_WATERWALK);
            break;
        case JUMPING:
            mon->mextrinsics &= ~(MR2_JUMPING);
            break;
        case DISPLACED:
        case TELEPAT: {
            /* another worn item may grant the same property (a helm of
               telepathy with an amulet of ESP, a cloak of displacement
               over shimmering-scaled mail); wielded gear doesn't count
               since wielding never granted it */
            unsigned long mr2 = (which == DISPLACED) ? MR2_DISPLACED
                                                     : MR2_TELEPATHY;

            for (otmp = mon->minvent; otmp; otmp = otmp->nobj) {
                if (otmp == obj || !(otmp->owornmask & ~W_WEAPONS))
                    continue;
                if (obj_has_prop(otmp, which))
                    break;
            }
            if (!otmp)
                mon->mextrinsics &= ~mr2;
            break;
        }
        case LEVITATION:
            mon->mextrinsics &= ~(MR2_LEVITATE);
            if (!unseen && !can_levitate(mon))
                pline_mon(mon, "%s floats gently back to the %s.",
                          Monnam(mon), surface(mon->mx, mon->my));
            break;
        case FREE_ACTION:
            mon->mextrinsics &= ~(MR2_FREE_ACTION);
            break;
        case FLYING:
            mon->mextrinsics &= ~(MR2_FLY);
            if (!unseen
                && !(is_flyer(mon->data) || is_floater(mon->data)
                     || can_levitate(mon) || can_fly(mon)))
                pline_mon(mon, "%s lands on the %s.",
                          Monnam(mon), surface(mon->mx, mon->my));
            break;
        case FIRE_RES:
        case COLD_RES:
        case SLEEP_RES:
        case DISINT_RES:
        case SHOCK_RES:
        case POISON_RES:
        case ACID_RES:
        case STONE_RES:
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
            mask = res_to_mr(which);
            for (otmp = mon->minvent; otmp; otmp = otmp->nobj) {
                if (otmp == obj || !otmp->owornmask)
                    continue;
                if ((int) objects[otmp->otyp].oc_oprop == which)
                    break;
                /* object properties */
                if (obj_has_prop(otmp, which))
                    break;
                /* check whether 'otmp' confers target property as an extra
                   one rather than as the one specified for it in objects[] */
                if (altprop(otmp) == which)
                    break;
            }
            if (!otmp)
                mon->mextrinsics &= ~((unsigned short) mask);
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

    /* run each of the item's object properties through the
       switches above; damage properties on weapons are offensive
       and do not confer the corresponding resistance */
    while (props) {
        if (!i)
            i = 1L;
        else
            i <<= 1;

        if (i > ITEM_PROP_MASK)
            break;

        if (props & i) {
            which = 0;
            props &= ~i;
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

    /* if couldn't see it but now can, or vice versa, update display;
       likewise for a telepathic hero sensing it */
    if (!silently
        && ((unseen ^ !canseemon(mon)) || (unsensed ^ !sensemon(mon))))
        newsym(mon->mx, mon->my);
}

#undef altprop

/* Return the armor bonus of a piece of armor: the amount by which it
   directly lowers the AC of the wearer */
int
armor_bonus(struct obj *armor)
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
    /* erosion is not allowed to make the armor worse than wearing
       nothing; only negative enchantment can do that */
    if (bon < 0)
        bon = 0;
    /* add enchantment (could be negative) */
    bon += armor->spe;
    /* adjust for armor quality */
    if (armor->forged_qual == FQ_SUPERIOR)
        bon += 1;
    else if (armor->forged_qual == FQ_EXCEPTIONAL)
        bon += 2;
    else if (armor->forged_qual == FQ_INFERIOR)
        bon -= 2;
    /* add bonus for dragon-scaled armor, slightly more AC than dragon
       scales by themselves, as the scales harden as they merge with
       worn armor */
    if (Is_dragon_scaled_armor(armor))
        bon += 5;

    return bon;
}

/* Determine the extrinsic property a piece of armor provides: its own
   oc_oprop, or the scales' for dragon-scaled armor whose base type has
   none.  Based on item_provides_extrinsic in NetHack Fourk, but less
   general. */
int
armor_provides_extrinsic(struct obj *armor)
{
    int prop;

    if (!armor)
        return 0;

    prop = objects[armor->otyp].oc_oprop;
    if (!prop && Is_dragon_armor(armor))
        return objects[Dragon_armor_to_scales(armor)].oc_oprop;

    return prop;
}

/* An attack missed by a margin the defender's armor can account for:
   walk the worn pieces from the skin outwards, adding each one's
   armor_bonus to the attacker's target number, and return the piece
   that carried it past the roll - the one that deflected the blow.
   Returns Null when the attack missed on its own merits.

   'target' is updated as we go so a caller can continue the ladder with
   something that is not an object, as the hero's golden haze does.

   Master has this written out three times, once per combat direction;
   the arms and their order are its own, quirks included. The gloves,
   boots and helm arms deliberately do not re-test the roll, so a lucky
   1-in-10 glove can take credit for a blow an inner piece had already
   stopped - keep that, it is what makes the messages vary */
struct obj *
armor_blocker(
    struct monst *mdef,
    int *target, /* attacker's to-hit total; updated in place */
    int roll)    /* what the attacker rolled */
{
    struct obj *blocker = (struct obj *) 0, *otmp;

    if (*target >= roll)
        return blocker; /* missed by more than armor can explain */

    /* undershirt, only if nothing is worn over it */
    if ((otmp = which_armor(mdef, W_ARMU)) != 0
        && !which_armor(mdef, W_ARM) && !which_armor(mdef, W_ARMC)
        && *target <= roll) {
        *target += armor_bonus(otmp);
        if (*target > roll)
            blocker = otmp;
    }
    /* body armor, only if no cloak is worn over it */
    if ((otmp = which_armor(mdef, W_ARM)) != 0
        && !which_armor(mdef, W_ARMC) && *target <= roll) {
        *target += armor_bonus(otmp);
        if (*target > roll)
            blocker = otmp;
    }
    if ((otmp = which_armor(mdef, W_ARMG)) != 0 && !rn2(10)) {
        *target += armor_bonus(otmp);
        if (*target > roll)
            blocker = otmp;
    }
    if ((otmp = which_armor(mdef, W_ARMF)) != 0 && !rn2(10)) {
        *target += armor_bonus(otmp);
        if (*target > roll)
            blocker = otmp;
    }
    if ((otmp = which_armor(mdef, W_ARMH)) != 0 && !rn2(5)) {
        *target += armor_bonus(otmp);
        if (*target > roll)
            blocker = otmp;
    }
    if ((otmp = which_armor(mdef, W_ARMC)) != 0 && *target <= roll) {
        *target += armor_bonus(otmp);
        if (*target > roll)
            blocker = otmp;
    }
    if ((otmp = which_armor(mdef, W_ARMS)) != 0 && *target <= roll) {
        *target += armor_bonus(otmp);
        if (*target > roll)
            blocker = otmp;
    }
    /* barding, for a steed; which_armor never reports it for the hero */
    if ((otmp = which_armor(mdef, W_BARDING)) != 0 && *target <= roll) {
        *target += armor_bonus(otmp);
        if (*target > roll)
            blocker = otmp;
    }

    return blocker;
}

int
find_mac(struct monst *mon)
{
    struct obj *obj;
    int base = r_data(mon)->ac - mon->mprotection;
    int bonus, div, race_role_bonus;
    long mwflags = mon->misc_worn_check;

    for (obj = mon->minvent; obj; obj = obj->nobj) {
        if (obj->owornmask & mwflags) {
            if (obj->otyp == AMULET_OF_GUARDING)
                base -= 2; /* fixed amount, not impacted by erosion */
            else
                base -= armor_bonus(obj);
            /* since armor_bonus is positive, subtracting it increases AC */

            /* race and role armor bonuses, separate from regular
               bonuses, leaving drow gloves out of this on purpose */
            race_role_bonus = 1;
            if (which_armor(mon, W_ARM)) {
                if ((racial_orc(mon)
                     && (obj->otyp == ORCISH_CHAIN_MAIL
                         || obj->otyp == ORCISH_RING_MAIL))
                    || (racial_drow(mon)
                        && (obj->otyp == DARK_ELVEN_CHAIN_MAIL
                            || obj->otyp == DARK_ELVEN_TUNIC))
                    || (racial_elf(mon) && obj->otyp == ELVEN_CHAIN_MAIL)
                    || (racial_dwarf(mon)
                        && obj->otyp == DWARVISH_CHAIN_MAIL)
                    || (racial_zombie(mon) && is_bone(obj))
                    || (racial_vampire(mon) && is_bone(obj))
                    || (mon->data == &mons[PM_DRUID] && is_wood(obj)))
                    base -= race_role_bonus;
            }
            if (which_armor(mon, W_ARMC)) {
                if ((racial_orc(mon) && obj->otyp == ORCISH_CLOAK)
                    || (racial_elf(mon) && obj->otyp == ELVEN_CLOAK)
                    || (racial_drow(mon) && obj->otyp == DARK_ELVEN_CLOAK)
                    || (racial_dwarf(mon) && obj->otyp == DWARVISH_CLOAK))
                    base -= race_role_bonus;
            }
            if (which_armor(mon, W_ARMH)) {
                if ((racial_orc(mon) && obj->otyp == ORCISH_HELM)
                    || (racial_elf(mon) && obj->otyp == ELVEN_HELM)
                    || (racial_drow(mon) && obj->otyp == DARK_ELVEN_HELM)
                    || (racial_dwarf(mon) && obj->otyp == DWARVISH_HELM)
                    || (racial_zombie(mon) && is_bone(obj))
                    || (racial_vampire(mon) && is_bone(obj))
                    || (mon->data == &mons[PM_DRUID] && is_wood(obj)))
                    base -= race_role_bonus;
            }
            if (which_armor(mon, W_ARMF)) {
                if ((racial_orc(mon) && obj->otyp == ORCISH_BOOTS)
                    || (racial_elf(mon) && obj->otyp == ELVEN_BOOTS)
                    || (racial_drow(mon) && obj->otyp == DARK_ELVEN_BOOTS)
                    || (racial_dwarf(mon) && obj->otyp == DWARVISH_BOOTS)
                    || (racial_zombie(mon) && is_bone(obj))
                    || (racial_vampire(mon) && is_bone(obj))
                    || (mon->data == &mons[PM_DRUID] && is_wood(obj)))
                    base -= race_role_bonus;
            }
            if (which_armor(mon, W_ARMS)) {
                if ((racial_orc(mon)
                     && (obj->otyp == ORCISH_SHIELD
                         || obj->otyp == URUK_HAI_SHIELD))
                    || (racial_elf(mon) && obj->otyp == ELVEN_SHIELD)
                    || (racial_drow(mon) && obj->otyp == DARK_ELVEN_BRACERS)
                    || (racial_dwarf(mon)
                        && obj->otyp == DWARVISH_ROUNDSHIELD)
                    || (racial_zombie(mon) && is_bone(obj))
                    || (racial_vampire(mon) && is_bone(obj))
                    || (mon->data == &mons[PM_DRUID] && is_wood(obj)))
                    base -= race_role_bonus;
            }
            if (which_armor(mon, W_ARMG)) {
                if ((racial_zombie(mon) && is_bone(obj))
                    || (racial_vampire(mon) && is_bone(obj))
                    || (mon->data == &mons[PM_DRUID] && is_wood(obj)))
                    base -= race_role_bonus;
            }
        }
    }

    /* other spell effects */
    if (has_barkskin(mon))
        base -= 5;
    if (has_stoneskin(mon))
        base -= 8;

    /* base derives from r_data(mon)->ac rather than mon->data->ac;
       make sure racial shopkeepers retain their buffed up AC */
    if (has_eshk(mon))
        base -= 10;

    /* tweak the monster's AC a bit according to its level */
    div = mon->m_lev > 20 ? 4 : 3;
    bonus = ((mon->m_lev / 2)) / div;
    if (bonus > 20)
        bonus = 20;
    if (bonus < 0)
        bonus = 0;
    base -= bonus;

    /* same cap as for hero [find_ac(do_wear.c)] */
    if (abs(base) > AC_MAX)
        base = sgn(base) * AC_MAX;
    return base;
}

/*
 * weapons are handled separately;
 * rings and eyewear aren't used by monsters
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
m_dowear(struct monst *mon, boolean creation)
{
    boolean can_wear_armor;

#define RACE_EXCEPTION TRUE
    /* Note the restrictions here are the same as in dowear in do_wear.c
     * except for the additional restriction on intelligence.  (Players
     * are always intelligent, even if polymorphed).
     */
    if (r_verysmall(mon) || nohands(mon->data) || is_animal(mon->data)
        || is_ent(mon->data) || is_plant(mon->data))
        return;
    /* give mummies a chance to wear their wrappings
     * and let skeletons wear their initial armor */
    if (mindless(mon->data)
        && (!creation || (mon->data->mlet != S_MUMMY
                          && mon->data->mlet != S_SKELETON)))
        return;

    m_dowear_type(mon, W_AMUL, creation, FALSE);
    can_wear_armor = !cantweararm(mon); /* for suit, cloak, shirt */
    /* can't put on shirt if already wearing suit */
    if (can_wear_armor && !(mon->misc_worn_check & W_ARM))
        m_dowear_type(mon, W_ARMU, creation, FALSE);
    /* WrappingAllowed() makes any size between small and huge eligible;
       treating small as a special case allows hobbits, gnomes, and
       kobolds to wear all cloaks; large and huge allows giants and such
       to wear mummy wrappings but not other cloaks */
    if (can_wear_armor || WrappingAllowed(r_data(mon)))
        m_dowear_type(mon, W_ARMC, creation, FALSE);
    m_dowear_type(mon, W_ARMH, creation, FALSE);
    /* bracers fill the shield slot but leave the hands free, so the
       two-handed weapon restriction is applied per item instead */
    m_dowear_type(mon, W_ARMS, creation, FALSE);
    m_dowear_type(mon, W_ARMG, creation, FALSE);
    if (!slithy(mon->data) && !racial_centaur(mon)
        && mon->data != &mons[PM_DRIDER] && !racial_tortle(mon))
        m_dowear_type(mon, W_ARMF, creation, FALSE);
    if (can_wear_armor)
        m_dowear_type(mon, W_ARM, creation, FALSE);
    else
        m_dowear_type(mon, W_ARM, creation, RACE_EXCEPTION);
}

/* recursive helper to find the best wearable item, including in
   containers; returns the best item found, or NULL if nothing better
   than 'best' */
staticfn struct obj *
find_best_armor_recurse(
    struct monst *mon,
    struct obj *start,
    struct obj *best,
    long flag,               /* wornmask value */
    boolean creation,        /* mon is being created */
    boolean racialexception) /* small monsters that are allowed for player
                              * races (gnomes) can wear suits */
{
    struct obj *obj, *found;

    for (obj = start; obj; obj = obj->nobj) {
        /* recurse into containers */
        if (Is_container(obj) && obj->cobj) {
            found = find_best_armor_recurse(mon, obj->cobj, best, flag,
                                            creation, racialexception);
            if (found)
                best = found;
            continue;
        }

        /* never equip gear made of a hated material */
        if (mon_hates_material(mon, obj->material))
            continue;

        /* unworn at the hero's request; waiting to be collected */
        if (obj->oreserved)
            continue;

        switch (flag) {
        case W_AMUL:
            if (obj->oclass != AMULET_CLASS
                || (obj->otyp != AMULET_OF_LIFE_SAVING
                    && obj->otyp != AMULET_OF_REFLECTION
                    && obj->otyp != AMULET_OF_MAGIC_RESISTANCE
                    && obj->otyp != AMULET_OF_GUARDING
                    && obj->otyp != AMULET_OF_ESP
                    && obj->otyp != AMULET_OF_FLYING
                    && obj->oartifact != ART_EYE_OF_THE_AETHIOPICA))
                continue;
            /* no point wearing life saving that cannot save this form
               or race; a cursed one still looks fine to a monster */
            if (obj->otyp == AMULET_OF_LIFE_SAVING
                && mlifesaver_useless(mon))
                continue;
            /* for 'best' to be non-Null, it must be an amulet of guarding;
               life-saving and reflection don't get here due to early return
               and other amulets of guarding can't be any better */
            if (!best || obj->otyp != AMULET_OF_GUARDING) {
                best = obj;
                if (best->otyp != AMULET_OF_GUARDING)
                    return best; /* life-saving or reflection; use it */
            }
            continue; /* skip post-switch armor handling */
        case W_ARMU:
            if (!is_shirt(obj))
                continue;
            break;
        case W_ARMC:
            if (!is_cloak(obj))
                continue;
            /* mummy wrapping is only cloak allowed when bigger than human */
            if (mon->data->msize > MZ_HUMAN && obj->otyp != MUMMY_WRAPPING)
                continue;
            /* avoid mummy wrapping if it will allow hero to see mon (unless
               this is a new mummy; an invisible one is feasible via ^G) */
            if (mon->minvis && w_blocks(obj, W_ARMC) == INVIS
                && !See_invisible && !creation)
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
            /* bracers leave the hands free; a proper shield can't be
               used together with a two-handed weapon */
            if (!is_bracer(obj)
                && MON_WEP(mon) && mon_bimanual(mon, MON_WEP(mon)))
                continue;
            break;
        case W_ARMG:
            if (!is_gloves(obj) || obj->otyp == MUMMIFIED_HAND)
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
            /* wings must fit through the armor; dragon-scaled armor
               and jackets accommodate them */
            if (big_wings(mon->data) && !Is_dragon_scales(obj)
                && obj->otyp != JACKET)
                continue;
            break;
        }
        if (obj->owornmask)
            continue;
        /* I'd like to define a visible armor_bonus which doesn't assume
         * the monster knows obj->spe, but if I did that, a monster would
         * keep switching forever between two -2 caps since when it took
         * off one it would forget spe and once again think the object is
         * better than what it already has.
         */
        if (best && (armor_bonus(best) + extra_pref(mon, best)
                     >= armor_bonus(obj) + extra_pref(mon, obj)))
            continue;
        best = obj;
    }
    return best;
}

staticfn void
m_dowear_type(
    struct monst *mon,
    long flag,               /* wornmask value */
    boolean creation,        /* no wear messages when mon is being created */
    boolean racialexception) /* small monsters that are allowed for player
                              * races (gnomes) can wear suits */
{
    struct obj *old, *best;
    long oldmask = 0L;
    int m_delay = 0;
    int sawmon = canseemon(mon), sawloc = cansee(mon->mx, mon->my);
    boolean autocurse;
    char nambuf[BUFSZ];

    if (mon->mfrozen) {
        /* probably putting previous item on; check again when
           able to act so remaining gear is not forgotten */
        check_gear_next_turn(mon);
        return;
    }

    /* Get a copy of monster's name before altering its visibility */
    Strcpy(nambuf, See_invisible ? Monnam(mon) : mon_nam(mon));

    old = which_armor(mon, flag);
    if (old && old->cursed)
        return;
    if (old && flag == W_AMUL && old->otyp != AMULET_OF_GUARDING)
        return; /* no amulet better than life-saving or reflection */
    /* find best item, including those in containers */
    best = find_best_armor_recurse(mon, mon->minvent, old, flag,
                                   creation, racialexception);
    if (!best || best == old)
        return;

    /* if best item is in a container, extract it first (uses turn) */
    if (best->where == OBJ_CONTAINED) {
        int result = mon_container_extract(mon, best);

        if (result != 1) /* 0=failed, 2=used turn for extraction */
            return;
        /* result==1 means item was already in inventory (shouldn't
           happen) */
    }

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
            char buf[BUFSZ], oldarm[BUFSZ], newarm[BUFSZ + sizeof "another "];

            /* "<Mon> [removes <oldarm> and ]puts on <newarm>."
               uses accessory verbs for armor but we can live with that */
            if (old) {
                Strcpy(oldarm, distant_name(old, doname));
                Snprintf(buf, sizeof buf, " removes %s and", oldarm);
            } else {
                buf[0] = oldarm[0] = '\0';
            }
            Strcpy(newarm, distant_name(best, doname));
            /* a monster will swap an item of the same type as the one it
               is replacing when the enchantment is better;
               if newarm and oldarm have identical descriptions, substitute
               "another <newarm>" for "a|an <newarm>" */
            if (!strcmpi(newarm, oldarm)) {
                /* size of newarm[] has been overallocated to guarantee
                   enough room to insert "another " */
                if (!strncmpi(newarm, "a ", 2))
                    (void) strsubst(newarm, "a ", "another ");
                else if (!strncmpi(newarm, "an ", 3))
                    (void) strsubst(newarm, "an ", "another ");
                newarm[BUFSZ - 1] = '\0';
            }
            pline_mon(mon, "%s%s puts on %s.", Monnam(mon), buf, newarm);
            if (autocurse)
                pline("%s %s %s %s for a moment.", s_suffix(Monnam(mon)),
                      simpleonames(best), otense(best, "glow"),
                      hcolor(NH_BLACK));
        } /* can see it */
        m_delay += objects[best->otyp].oc_delay;
        mon->mfrozen = m_delay;
        if (mon->mfrozen) {
            mon->mcanmove = 0;
            /* this wear ends the gear pass early; check again
               afterwards for anything else worth equipping */
            check_gear_next_turn(mon);
        }
    }
    if (old) {
        update_mon_extrinsics(mon, old, FALSE, creation);

        /* owornmask was cleared above but artifact_light() expects it */
        old->owornmask = oldmask;
        if (old->lamplit && artifact_light(old))
            end_burn(old, FALSE);
        old->owornmask = 0L;
    }
    mon->misc_worn_check |= flag;
    best->owornmask |= flag;
    /* equipping a non-bracer shield forces dropping secondary weapon */
    if (flag == W_ARMS && !is_bracer(best) && MON_WEP2(mon)) {
        struct obj *mw2 = MON_WEP2(mon);

        setmnotwielded2(mon, mw2);
        /* a pet ordered not to drop items keeps its off-hand
           weapon in inventory */
        if (!creation && !pet_wont_drop(mon)) {
            if (cansee(mon->mx, mon->my)) {
                pline_mon(mon, "%s drops %s.", Monnam(mon),
                          distant_name(mw2, doname));
                newsym(mon->mx, mon->my);
            }
            obj_extract_self(mw2);
            if (!flooreffects(mw2, mon->mx, mon->my, "drop")) {
                place_object(mw2, mon->mx, mon->my);
                stackobj(mw2);
            }
        }
    }
    if (autocurse)
        curse(best);
    if (artifact_light(best) && !best->lamplit) {
        begin_burn(best, FALSE);
        vision_recalc(1);
        if (!creation && best->lamplit && cansee(mon->mx, mon->my)) {
            const char *adesc = arti_light_description(best);
            boolean shadow = (Is_dragon_armor(best)
                              && Dragon_armor_to_scales(best)
                                 == SHADOW_DRAGON_SCALES);

            if (shadow) {
                /* shadow dragon armor gives off darkness, not light */
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
    update_mon_extrinsics(mon, best, TRUE, creation);
    /* if couldn't see it but now can, or vice versa */
    if (!creation && (sawmon ^ canseemon(mon))) {
        if (mon->minvis && !See_invisible) {
            pline("Suddenly you cannot see %s.", nambuf);
            makeknown(best->otyp);
        /* } else if (!mon->minvis) {
         *     pline("%s suddenly appears!", Amonnam(mon)); */
        }
    }
}
#undef RACE_EXCEPTION

struct obj *
which_armor(struct monst *mon, long flag)
{
    if (mon == &gy.youmonst) {
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
        case W_AMUL:
            return uamul;
        case W_RINGL:
            return uleft;
        case W_RINGR:
            return uright;
        case W_TOOL:
            return ublindf;
        case W_SADDLE:
        case W_BARDING:
            /* hero never wears a saddle or barding */
            return (struct obj *) 0;
        default:
            impossible("bad flag in which_armor");
            return 0;
        }
    } else {
        struct obj *obj;

        for (obj = mon->minvent; obj; obj = obj->nobj)
            if (obj->owornmask & flag)
                return obj;
        return (struct obj *) 0;
    }
}

/* remove an item of armor and then drop it */
staticfn void
m_lose_armor(
    struct monst *mon,
    struct obj *obj,
    boolean polyspot)
{
    /* stripping a saddle dismounts the hero, and that dismount can move
       the former steed - to a spot found by enexto when an engulfer is
       standing where it landed, or by rloc_to when the hero could not be
       moved instead - so drop the armor where the wearer was */
    coordxy mx = mon->mx, my = mon->my;

    extract_from_minvent(mon, obj, TRUE, FALSE);
    place_object(obj, mx, my);
    if (polyspot)
        bypass_obj(obj);
    /* call stackobj() if we ever drop anything that can merge */
    newsym(mx, my);
}

/* clear bypass bits for an object chain, plus contents if applicable */
staticfn void
clear_bypass(struct obj *objchn)
{
    struct obj *o;

    for (o = objchn; o; o = o->nobj) {
        o->bypass = 0;
        if (Has_contents(o))
            clear_bypass(o->cobj);
    }
}

/* all objects with their bypass bit set should now be reset to normal;
   this can be a relatively expensive operation so is only called if
   svc.context.bypasses is set */
void
clear_bypasses(void)
{
    struct monst *mtmp;

    /*
     * 'Object' bypass is also used for one monster function:
     * polymorph control of long worms.  Activated via setting
     * svc.context.bypasses even if no specific object has been
     * bypassed.
     */

    clear_bypass(fobj);
    clear_bypass(gi.invent);
    clear_bypass(gm.migrating_objs);
    clear_bypass(gm.mchest);
    clear_bypass(svl.level.buriedobjlist);
    clear_bypass(gb.billobjs);
    clear_bypass(go.objs_deleted);
    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp))
            continue;
        clear_bypass(mtmp->minvent);
        /* long worm created by polymorph has mon->mextra->mcorpsenm set
           to PM_LONG_WORM to flag it as not being subject to further
           polymorph (so polymorph zap won't hit monster to transform it
           into a long worm, then hit that worm's tail and transform it
           again on same zap); clearing mcorpsenm reverts worm to normal */
        if (mtmp->data == &mons[PM_LONG_WORM] && has_mcorpsenm(mtmp))
            MCORPSENM(mtmp) = NON_PM;
    }
    for (mtmp = gm.migrating_mons; mtmp; mtmp = mtmp->nmon) {
        clear_bypass(mtmp->minvent);
        /* no MCORPSENM(mtmp)==PM_LONG_WORM check here; long worms can't
           be just created by polymorph and migrating at the same time */
    }
    /* this is a no-op since mydogs is only non-Null during level change or
       final ascension and we aren't called at those times, but be thorough */
    for (mtmp = gm.mydogs; mtmp; mtmp = mtmp->nmon)
        clear_bypass(mtmp->minvent);
    /* ball and chain can be "floating", not on any object chain (when
       hero is swallowed by an engulfing monster, for instance) */
    if (uball)
        uball->bypass = 0;
    if (uchain)
        uchain->bypass = 0;

    svc.context.bypasses = FALSE;
}

void
bypass_obj(struct obj *obj)
{
    obj->bypass = 1;
    svc.context.bypasses = TRUE;
}

/* set or clear the bypass bit in a list of objects */
void
bypass_objlist(
    struct obj *objchain,
    boolean on) /* TRUE => set, FALSE => clear */
{
    if (on && objchain)
        svc.context.bypasses = TRUE;
    while (objchain) {
        objchain->bypass = on ? 1 : 0;
        objchain = objchain->nobj;
    }
}

/* return the first object without its bypass bit set; set that bit
   before returning so that successive calls will find further objects */
struct obj *
nxt_unbypassed_obj(struct obj *objchain)
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
nxt_unbypassed_loot(Loot *lootarray, struct obj *listhead)
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
mon_break_armor(struct monst *mon, boolean polyspot)
{
    struct obj *otmp;
    struct permonst *mdat = mon->data;
    boolean vis = cansee(mon->mx, mon->my),
            handless_or_tiny = (nohands(mdat) || r_verysmall(mon)),
            noride = FALSE;
    const char *pronoun = mhim(mon), *ppronoun = mhis(mon);

    if (breakarm(mon)) {
        if ((otmp = which_armor(mon, W_ARM)) != 0 && otmp->oartifact) {
            /* an artifact suit comes off whole, as the hero's does */
            if (vis)
                pline_mon(mon, "%s armor falls off!", s_suffix(Monnam(mon)));
            m_lose_armor(mon, otmp, polyspot);
        } else if (otmp) {
            if (Is_dragon_armor(otmp)
                && monsndx(mdat) == Dragon_armor_to_pm(otmp)) {
                ; /* no message here;
                     "the dragon merges with his scaly armor" is odd
                     and the monster's previous form is already gone */
            } else {
                Soundeffect(se_cracking_sound, 100);
                if (vis)
                    pline_mon(mon, "%s breaks out of %s armor!",
                              Monnam(mon), ppronoun);
                else
                    You_hear("a cracking sound.");
            }
            m_useup(mon, otmp);
        }
        if ((otmp = which_armor(mon, W_ARMC)) != 0
            /* mummy wrapping adapts to small and very big sizes */
            && (otmp->otyp != MUMMY_WRAPPING || !WrappingAllowed(mdat))) {
            if (otmp->oartifact) {
                if (vis)
                    pline_mon(mon, "%s %s falls off!", s_suffix(Monnam(mon)),
                          cloak_simple_name(otmp));
                m_lose_armor(mon, otmp, polyspot);
            } else {
                if (Is_dragon_armor(otmp)
                    && monsndx(mdat) == Dragon_armor_to_pm(otmp)) {
                    ; /* same as above; no message here */
                } else {
                    Soundeffect(se_ripping_sound, 100);
                    if (vis)
                        pline_mon(mon, "%s %s tears apart!",
                                  s_suffix(Monnam(mon)),
                                  cloak_simple_name(otmp));
                    else
                        You_hear("a ripping sound.");
                }
                m_useup(mon, otmp);
            }
        }
        if ((otmp = which_armor(mon, W_ARMU)) != 0) {
            if (otmp->oartifact) {
                if (vis)
                    pline_mon(mon, "%s shirt falls off!",
                              s_suffix(Monnam(mon)));
                m_lose_armor(mon, otmp, polyspot);
            } else {
                if (vis)
                    pline_mon(mon, "%s shirt rips to shreds!",
                              s_suffix(Monnam(mon)));
                else
                    You_hear("a ripping sound.");
                m_useup(mon, otmp);
            }
        }
    } else if (sliparm(mon)) {
        /* sliparm checks whirly, noncorporeal, and small or under */
        boolean passes_thru_clothes = !(mdat->msize <= MZ_SMALL);

        if ((otmp = which_armor(mon, W_ARM)) != 0) {
            Soundeffect(se_thud, 50);
            if (vis)
                pline_mon(mon, "%s armor falls around %s!",
                          s_suffix(Monnam(mon)), pronoun);
            else
                You_hear("a thud.");
            m_lose_armor(mon, otmp, polyspot);
        }
        if ((otmp = which_armor(mon, W_ARMC)) != 0
            /* mummy wrapping adapts to small and very big sizes */
            && (otmp->otyp != MUMMY_WRAPPING || !WrappingAllowed(mdat))) {
            if (vis) {
                if (is_whirly(mon->data))
                    pline_mon(mon, "%s %s falls, unsupported!",
                              s_suffix(Monnam(mon)), cloak_simple_name(otmp));
                else
                    pline_mon(mon, "%s shrinks out of %s %s!",
                              Monnam(mon), ppronoun,
                              cloak_simple_name(otmp));
            }
            m_lose_armor(mon, otmp, polyspot);
        }
        if ((otmp = which_armor(mon, W_ARMU)) != 0) {
            if (vis) {
                if (passes_thru_clothes)
                    pline_mon(mon, "%s seeps right through %s shirt!",
                              Monnam(mon), ppronoun);
                else
                    pline_mon(mon, "%s becomes much too small for %s shirt!",
                          Monnam(mon), ppronoun);
            }
            m_lose_armor(mon, otmp, polyspot);
        }
    }
    if (handless_or_tiny) {
        /* [caller needs to handle weapon checks] */
        if ((otmp = which_armor(mon, W_ARMG)) != 0) {
            if (vis)
                pline_mon(mon, "%s drops %s gloves%s!",
                          Monnam(mon), ppronoun,
                          MON_WEP(mon) ? " and weapon" : "");
            m_lose_armor(mon, otmp, polyspot);
        }
        if ((otmp = which_armor(mon, W_ARMS)) != 0) {
            Soundeffect(se_clank, 50);
            if (vis)
                pline_mon(mon, "%s can no longer %s %s %s!", Monnam(mon),
                          is_bracer(otmp) ? "wear" : "hold", ppronoun,
                          is_bracer(otmp) ? "bracers" : "shield");
            else
                You_hear("a clank.");
            m_lose_armor(mon, otmp, polyspot);
        }
    }
    if (handless_or_tiny || has_horns(mdat)) {
        if ((otmp = which_armor(mon, W_ARMH)) != 0
            /* flimsy test for horns matches polyself handling */
            && (handless_or_tiny || !is_flimsy(otmp))) {
            if (vis)
                pline_mon(mon, "%s helmet falls to the %s!",
                          s_suffix(Monnam(mon)), surface(mon->mx, mon->my));
            else
                You_hear("a clank.");
            m_lose_armor(mon, otmp, polyspot);
        }
    }
    if (handless_or_tiny || slithy(mdat) || racial_centaur(mon)
        || racial_tortle(mon)) {
        if ((otmp = which_armor(mon, W_ARMF)) != 0) {
            if (vis) {
                if (is_whirly(mon->data))
                    pline_mon(mon, "%s boots fall away!",
                              s_suffix(Monnam(mon)));
                else
                    pline_mon(mon, "%s boots %s off %s feet!",
                              s_suffix(Monnam(mon)),
                          r_verysmall(mon) ? "slide" : "are pushed", ppronoun);
            }
            m_lose_armor(mon, otmp, polyspot);
        }
    }
    if (!can_wear_barding(mon)) {
        if ((otmp = which_armor(mon, W_BARDING)) != 0) {
            m_lose_armor(mon, otmp, polyspot);
            if (vis)
                pline_mon(mon, "%s barding falls off.", s_suffix(Monnam(mon)));
        }
    }
    if (!can_saddle(mon)) {
        if ((otmp = which_armor(mon, W_SADDLE)) != 0) {
            m_lose_armor(mon, otmp, polyspot);
            /* losing the saddle dismounts the hero, which can kill the
               former steed; nothing below may speak for it */
            if (DEADMONSTER(mon))
                return;
            if (vis)
                pline_mon(mon, "%s saddle falls off.", s_suffix(Monnam(mon)));
        }
        if (mon == u.usteed)
            noride = TRUE;
    }
    if (noride || (mon == u.usteed && !can_ride(mon))) {
        You("can no longer ride %s.", mon_nam(mon));
        if (touch_petrifies(u.usteed->data) && !Stone_resistance && rnl(3)) {
            char buf[BUFSZ];

            You("touch %s.", mon_nam(u.usteed));
            Sprintf(buf, "falling off %s",
                    an(pmname(u.usteed->data, Mgender(u.usteed))));
            instapetrify(buf);
            /* instapetrify() returns instead of ending the game when the
               hero polymorphs into stone, and that polymorph dismounts
               already, leaving nothing to dismount from here */
            if (!u.usteed)
                return;
        }
        dismount_steed(DISMOUNT_FELL);
    }
    return;
}

/* bias a monster's preferences towards armor that has special benefits. */
int
extra_pref(struct monst *mon, struct obj *obj)
{
    /* currently only does speed boots, but might be expanded if monsters
     * get to use more armor abilities
     */
    if (obj) {
        if (obj->otyp == SPEED_BOOTS && mon->permspeed != MFAST)
            return 20;
    }
    return 0;
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
racial_exception(struct monst *mon, struct obj *obj)
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
extract_from_minvent(
    struct monst *mon,
    struct obj *obj,
    boolean do_extrinsics,  /* whether to call update_mon_extrinsics */
    boolean silently)       /* doesn't affect all possible messages,
                             * just update_mon_extrinsics's */
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
    /* leaving the monster's inventory releases any hero reservation */
    obj->oreserved = 0;
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
        if (!DEADMONSTER(mon) && do_extrinsics) {
            update_mon_extrinsics(mon, obj, FALSE, silently);
        }
        mon->misc_worn_check &= ~unwornmask;
        /* give monster a chance to wear other equipment on its next
           move instead of waiting until it picks something up */
        check_gear_next_turn(mon);
    }
    obj_no_longer_held(obj);
    if (unwornmask & W_WEP) {
        mwepgone(mon); /* unwields and sets weapon_check to NEED_WEAPON */
    }
    if (unwornmask & W_SWAPWEP)
        mwep2gone(mon);
}

#undef w_blocks

/*worn.c*/
