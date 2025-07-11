/* NetHack 3.6	lock.c	$NHDT-Date: 1548978605 2019/01/31 23:50:05 $  $NHDT-Branch: NetHack-3.6.2-beta01 $:$NHDT-Revision: 1.84 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2011. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* at most one of `door' and `box' should be non-null at any given time */
STATIC_VAR NEARDATA struct xlock_s {
    struct rm *door;
    struct obj *box;
    int picktyp, /* key|pick|card for unlock, sharp vs blunt for #force */
        chance, usedtime;
    boolean magic_key;
} xlock;

/* occupation callbacks */
STATIC_PTR int NDECL(picklock);
STATIC_PTR int NDECL(forcelock);

STATIC_DCL const char *NDECL(lock_action);
STATIC_DCL boolean FDECL(obstructed, (int, int, BOOLEAN_P));
STATIC_DCL void FDECL(chest_shatter_msg, (struct obj *));

boolean
picking_lock(x, y)
int *x, *y;
{
    if (occupation == picklock) {
        *x = u.ux + u.dx;
        *y = u.uy + u.dy;
        return TRUE;
    } else {
        *x = *y = 0;
        return FALSE;
    }
}

boolean
picking_at(x, y)
int x, y;
{
    return (boolean) (occupation == picklock && xlock.door == &levl[x][y]);
}

/* produce an occupation string appropriate for the current activity */
STATIC_OVL const char *
lock_action()
{
    /* "unlocking"+2 == "locking" */
    static const char *actions[] = {
        "unlocking the door",         /* [0] */
        "unlocking the chest",        /* [1] */
        "unlocking the box",          /* [2] */
        "cracking the safe",          /* [3] */
        "picking the lock",           /* [4] */
        "unlocking the magic chest"   /* [5] */
    };

    /* if the target is currently unlocked, we're trying to lock it now */
    if (xlock.door && !(xlock.door->doormask & D_LOCKED))
        return actions[0] + 2; /* "locking the door" */
    else if (xlock.box && !xlock.box->olocked)
        return xlock.box->otyp == CHEST
                   ? actions[1] + 2 : xlock.box->otyp == IRON_SAFE
                       ? actions[3] + 2 : xlock.box->otyp == HIDDEN_CHEST
                           ? actions[5] + 2 : actions[2] + 2;
    /* otherwise we're trying to unlock it */
    else if (xlock.picktyp == LOCK_PICK)
        return actions[4]; /* "picking the lock" */
    else if (xlock.picktyp == CREDIT_CARD)
        return actions[4]; /* same as lock_pick */
    else if (xlock.door)
        return actions[0]; /* "unlocking the door" */
    else if (xlock.box)
        return xlock.box->otyp == CHEST
                   ? actions[1] : xlock.box->otyp == IRON_SAFE
                       ? actions[3] : xlock.box->otyp == HIDDEN_CHEST
                           ? actions[5] : actions[2];
    else
        return actions[3];
}

/* try to open/close a lock */
STATIC_PTR int
picklock(VOID_ARGS)
{
    if (xlock.box) {
        if ((xlock.box->where != OBJ_FLOOR
             || xlock.box->ox != u.ux || xlock.box->oy != u.uy)
            && (!(xlock.box->otyp == IRON_SAFE || xlock.box->otyp == CRYSTAL_CHEST
                  || abs(xlock.box->oy - u.uy) > 1 || abs(xlock.box->ox - u.ux) > 1))) {
           return ((xlock.usedtime = 0)); /* you or it moved */
        }
    } else { /* door */
        if (xlock.door != &(levl[u.ux + u.dx][u.uy + u.dy])) {
            return ((xlock.usedtime = 0)); /* you moved */
        }
        switch (xlock.door->doormask) {
        case D_NODOOR:
            pline("This doorway has no door.");
            return ((xlock.usedtime = 0));
        case D_ISOPEN:
            You("cannot lock an open door.");
            return ((xlock.usedtime = 0));
        case D_BROKEN:
            pline("This door is broken.");
            return ((xlock.usedtime = 0));
        }
    }

    if (Sokoban && !xlock.box
        && (xlock.door->doormask & D_TRAPPED) != 0) {
        You("cannot lock this door.");
        return ((xlock.usedtime = 0));
    }

    if (xlock.usedtime++ >= 50
        || (nohands(youmonst.data)
            && !(druid_form && !slithy(youmonst.data))
            && !(vampire_form && !is_whirly(youmonst.data)))) {
        You("give up your attempt at %s.", lock_action());
        exercise(A_DEX, TRUE); /* even if you don't succeed */
        return ((xlock.usedtime = 0));
    }

    if (rn2(100) >= xlock.chance)
        return 1; /* still busy */

    /* using the Master Key of Thievery finds traps if its bless/curse
       state is adequate (non-cursed for rogues, blessed for others;
       checked when setting up 'xlock') */
    if ((!xlock.door ? (int) xlock.box->otrapped
                     : (xlock.door->doormask & D_TRAPPED) != 0)
        && xlock.magic_key) {
        xlock.chance += 20; /* less effort needed next time */
        /* unfortunately we don't have a 'tknown' flag to record
           "known to be trapped" so declining to disarm and then
           retrying lock manipulation will find it all over again */
        if (In_sokoban(&u.uz) && xlock.door) {
            pline("You find a trap!  But you see no way to disarm it.");
            exercise(A_WIS, FALSE);
        } else if (yn("You find a trap!  Do you want to try to disarm it?") == 'y') {
            const char *what;
            boolean alreadyunlocked;

            /* disarming while using magic key always succeeds */
            if (xlock.door) {
                xlock.door->doormask &= ~D_TRAPPED;
                what = "door";
                alreadyunlocked = !(xlock.door->doormask & D_LOCKED);
            } else {
                xlock.box->otrapped = 0;
                what = (xlock.box->otyp == CHEST) ? "chest" : "box";
                alreadyunlocked = !xlock.box->olocked;
            }
            You("succeed in disarming the trap.  The %s is still %slocked.",
                what, alreadyunlocked ? "un" : "");
            exercise(A_WIS, TRUE);
        } else {
            You("stop %s.", lock_action());
            exercise(A_WIS, FALSE);
        }
        return ((xlock.usedtime = 0));
    }

    You("succeed in %s.", lock_action());
    if (xlock.door) {
        if ((xlock.door->doormask & D_TRAPPED && !In_sokoban(&u.uz))) {
            xlock.door->doormask = D_NODOOR; /* this has to occur before b_trapped() */
            b_trapped("door", FINGER);
            unblock_point(u.ux + u.dx, u.uy + u.dy);
            if (*in_rooms(u.ux + u.dx, u.uy + u.dy, SHOPBASE))
                add_damage(u.ux + u.dx, u.uy + u.dy, SHOP_DOOR_COST);
            newsym(u.ux + u.dx, u.uy + u.dy);
        } else if (xlock.door->doormask & D_LOCKED)
            xlock.door->doormask = D_CLOSED;
        else
            xlock.door->doormask = D_LOCKED;
    } else {
        xlock.box->olocked = !xlock.box->olocked;
        xlock.box->lknown = 1;
        if (xlock.box->otrapped)
            (void) chest_trap(&youmonst, xlock.box, FINGER, FALSE);
    }
    exercise(A_DEX, TRUE);
    return ((xlock.usedtime = 0));
}

void
breakchestlock(box, destroyit)
struct obj *box;
boolean destroyit;
{
    if (!destroyit) { /* bill for the box but not for its contents */
        struct obj *hide_contents = box->cobj;

        box->cobj = 0;
        costly_alteration(box, COST_BRKLCK);
        box->cobj = hide_contents;
        box->olocked = 0;
        box->obroken = 1;
        box->lknown = 1;
    } else { /* #force has destroyed this box (at <u.ux,u.uy>) */
        struct obj *otmp;
        struct monst *shkp = (*u.ushops && costly_spot(u.ux, u.uy))
                                 ? shop_keeper(*u.ushops)
                                 : 0;
        boolean costly = (boolean) (shkp != 0),
                peaceful_shk = costly && (boolean) shkp->mpeaceful;
        long loss = 0L;

        pline("In fact, you've totally destroyed %s.", the(xname(box)));
        /* Put the contents on ground at the hero's feet. */
        while ((otmp = box->cobj) != 0) {
            obj_extract_self(otmp);
            if (!rn2(3) || otmp->oclass == POTION_CLASS) {
                chest_shatter_msg(otmp);
                if (costly)
                    loss += stolen_value(otmp, u.ux, u.uy, peaceful_shk, TRUE);
                if (otmp->quan == 1L) {
                    obfree(otmp, (struct obj *) 0);
                    continue;
                }
                /* this works because we're sure to have at least 1 left;
                   otherwise it would fail since otmp is not in inventory */
                useup(otmp);
            }
            if (box->otyp == ICE_BOX && otmp->otyp == CORPSE) {
                otmp->age = monstermoves - otmp->age; /* actual age */
                start_corpse_timeout(otmp);
            }
            place_object(otmp, u.ux, u.uy);
            stackobj(otmp);
        }
        if (costly)
            loss += stolen_value(box, u.ux, u.uy, peaceful_shk, TRUE);
        if (loss)
            You("owe %ld %s for objects destroyed.", loss, currency(loss));
        delobj(box);
    }
}

/* try to force a locked chest */
STATIC_PTR int
forcelock(VOID_ARGS)
{
    if ((xlock.box->ox != u.ux) || (xlock.box->oy != u.uy))
        return ((xlock.usedtime = 0)); /* you or it moved */

    if (xlock.usedtime++ >= 50 || !uwep || nohands(youmonst.data)) {
        You("give up your attempt to force the lock.");
        if (xlock.usedtime >= 50) /* you made the effort */
            exercise((xlock.picktyp) ? A_DEX : A_STR, TRUE);
        return ((xlock.usedtime = 0));
    }

    if (xlock.picktyp) { /* blade */
        if (rn2(1000 - (int) uwep->spe) > (992 - greatest_erosion(uwep) * 10)
            && !uwep->cursed && !obj_resists(uwep, 0, 99)) {
            /* for a +0 weapon, probability that it survives an unsuccessful
             * attempt to force the lock is (.992)^50 = .67
             */
            pline("%sour %s broke!", (uwep->quan > 1L) ? "One of y" : "Y",
                  xname(uwep));
            useup(uwep);
            You("give up your attempt to force the lock.");
            exercise(A_DEX, TRUE);
            return ((xlock.usedtime = 0));
        }
    } else             /* blunt */
        wake_nearby(); /* due to hammering on the container */

    if (rn2(100) >= xlock.chance)
        return 1; /* still busy */

    You("succeed in forcing the lock.");
    exercise(xlock.picktyp ? A_DEX : A_STR, TRUE);
    /* breakchestlock() might destroy xlock.box; if so, xlock context will
       be cleared (delobj -> obfree -> maybe_reset_pick); but it might not,
       so explicitly clear that manually */
    breakchestlock(xlock.box, (boolean) (!xlock.picktyp && !rn2(3)));
    reset_pick(); /* lock-picking context is no longer valid */

    return 0;
}

void
reset_pick()
{
    xlock.usedtime = xlock.chance = xlock.picktyp = 0;
    xlock.magic_key = FALSE;
    xlock.door = (struct rm *) 0;
    xlock.box = (struct obj *) 0;
}

/* level change or object deletion; context may no longer be valid */
void
maybe_reset_pick(container)
struct obj *container; /* passed from obfree() */
{
    /*
     * If a specific container, only clear context if it is for that
     * particular container (which is being deleted).  Other stuff on
     * the current dungeon level remains valid.
     * However if 'container' is Null, clear context if not carrying
     * xlock.box (which might be Null if context is for a door).
     * Used for changing levels, where a floor container or a door is
     * being left behind and won't be valid on the new level but a
     * carried container will still be.  There might not be any context,
     * in which case redundantly clearing it is harmless.
     */
    if (container ? (container == xlock.box)
                  : (!xlock.box || !carried(xlock.box)))
        reset_pick();
}

/* pick a tool for autounlock */
struct obj *
autokey(opening)
boolean opening; /* True: key, pick, or card; False: key or pick */
{
    struct obj *o, *key, *mkey, *pick, *card,
               *akey, *amkey, *apick, *acard;

    /* mundane item or regular artifact or own role's quest artifact */
    key = mkey = pick = card = (struct obj *) 0;
    /* other role's quest artifact (Rogue's Key or Tourist's Credit Card) */
    akey = amkey = apick = acard = (struct obj *) 0;
    for (o = invent; o; o = o->nobj) {
        if (any_quest_artifact(o) && !is_quest_artifact(o)) {
            switch (o->otyp) {
            case SKELETON_KEY:
                if (!akey)
                    akey = o;
                break;
            case MAGIC_KEY:
                if (!amkey)
                    amkey = o;
                break;
            case LOCK_PICK:
                if (!apick)
                    apick = o;
                break;
            case CREDIT_CARD:
                if (!acard)
                    acard = o;
                break;
            default:
                break;
            }
        } else {
            switch (o->otyp) {
            case SKELETON_KEY:
                if (!key || is_roguish_key(&youmonst, o))
                    key = o;
                break;
            case MAGIC_KEY:
                if (!mkey)
                    mkey = o;
                break;
            case LOCK_PICK:
                if (!pick)
                    pick = o;
                break;
            case CREDIT_CARD:
                if (!card)
                    card = o;
                break;
            default:
                break;
            }
        }
    }
    if (!opening)
        card = acard = 0;
    /* only resort to other role's quest artifact if no other choice */
    if (!key && !mkey && !pick && !card)
        mkey = amkey;
    /* rogues favor MKoT for its untrapping */
    if (is_roguish_key(&youmonst, key) && Role_if(PM_ROGUE))
        mkey = key;
    if (!key && !pick && !card)
        key = akey;
    if (!pick && !card)
        pick = apick;
    if (!card)
        card = acard;
    return mkey ? mkey : key ? key : pick ? pick : card ? card : 0;
}

/* for doapply(); if player gives a direction or resumes an interrupted
   previous attempt then it costs hero a move even if nothing ultimately
   happens; when told "can't do that" before being asked for direction
   or player cancels with ESC while giving direction, it doesn't */
#define PICKLOCK_LEARNED_SOMETHING (-1) /* time passes */
#define PICKLOCK_DID_NOTHING 0          /* no time passes */
#define PICKLOCK_DID_SOMETHING 1

/* player is applying a key, lock pick, or credit card */
int
pick_lock(pick, rx, ry, container)
struct obj *pick;
xchar rx, ry; /* coordinates of doors/container, for autounlock: does not
                 prompt for direction if these are set */
struct obj *container; /* container, for autounlock */
{
    int picktyp, c, ch;
    coord cc;
    struct rm *door;
    struct obj *otmp;
    char qbuf[QBUFSZ];
    boolean autounlock = (rx != 0 && ry != 0) || (container != NULL);

    picktyp = pick->otyp;

    /* check whether we're resuming an interrupted previous attempt */
    if (xlock.usedtime && picktyp == xlock.picktyp) {
        static char no_longer[] = "Unfortunately, you can no longer %s %s.";

        if ((nohands(youmonst.data)
             && !(druid_form && !slithy(youmonst.data))
             && !(vampire_form && !is_whirly(youmonst.data)))
            || !freehand()) {
            const char *what = (picktyp == LOCK_PICK) ? "pick" : "key";

            if (picktyp == CREDIT_CARD)
                what = "card";
            if (picktyp == STETHOSCOPE)
                what = "stethoscope";
            pline(no_longer, "hold the", what);
            reset_pick();
            return PICKLOCK_LEARNED_SOMETHING;
        } else if (u.uswallow || (xlock.box && !can_reach_floor(TRUE))) {
            pline(no_longer, "reach the", "lock");
            reset_pick();
            return PICKLOCK_LEARNED_SOMETHING;
        } else {
            const char *action = lock_action();

            You("resume your attempt at %s.", action);
            xlock.magic_key = is_roguish_key(&youmonst, pick);
            set_occupation(picklock, action, 0);
            return PICKLOCK_DID_SOMETHING;
        }
    }

    if (nohands(youmonst.data)) {
        if (druid_form && !slithy(youmonst.data)) {
            ;
        } else if (vampire_form && !is_whirly(youmonst.data)) {
            ;
        } else {
            You_cant("hold %s -- you have no hands!", doname(pick));
            return PICKLOCK_DID_NOTHING;
        }
    } else if (!freehand()) {
        You_cant("hold %s -- you have no %s free!",
                 doname(pick), makeplural(body_part(HAND)));
        return PICKLOCK_DID_NOTHING;
    } else if (u.uswallow) {
        You_cant("%sunlock %s.", (picktyp == CREDIT_CARD) ? "" : "lock or ",
                 mon_nam(u.ustuck));
        return PICKLOCK_DID_NOTHING;
    }

    if (picktyp != LOCK_PICK
        && picktyp != STETHOSCOPE
        && picktyp != CREDIT_CARD
        && picktyp != SKELETON_KEY
        && picktyp != MAGIC_KEY) {
        impossible("picking lock with object %d?", picktyp);
        return PICKLOCK_DID_NOTHING;
    }
    ch = 0; /* lint suppression */

    if (rx != 0 && ry != 0) { /* autounlock; caller has provided coordinates */
        cc.x = rx;
        cc.y = ry;
    } else if (picktyp == STETHOSCOPE) {
        /* skip directional prompt for stethoscope when not called via
         * autounlock; a direction (down) was already selected in
         * use_stethoscope */
        cc.x = u.ux;
        cc.y = u.uy;
    } else if (!get_adjacent_loc((char *) 0, "Invalid location!",
                                 u.ux, u.uy, &cc)) {
        return PICKLOCK_DID_NOTHING;
    }

    if (cc.x == u.ux && cc.y == u.uy) {	/* pick lock on a container */
        const char *verb;
        char qsfx[QBUFSZ];
        boolean it;
        int count;

        if (u.dz < 0) {
            There("isn't any sort of lock up %s.",
                  Levitation ? "here" : "there");
            return PICKLOCK_LEARNED_SOMETHING;
        } else if (is_lava(u.ux, u.uy)) {
            pline("Doing that would probably melt %s.", yname(pick));
            return PICKLOCK_LEARNED_SOMETHING;
        } else if (is_pool(u.ux, u.uy) && !Underwater) {
            pline_The("%s has no lock.", hliquid("water"));
            return PICKLOCK_LEARNED_SOMETHING;
        }

        count = 0;
        c = 'n'; /* in case there are no boxes here */
        otmp = IS_MAGIC_CHEST(levl[cc.x][cc.y].typ) ? mchest : level.objects[cc.x][cc.y];
        while (otmp) {
            /* autounlock on boxes: only the one that just informed you it was
             * locked. Don't include any other boxes which might be here. */
            if ((!autounlock && Is_box(otmp)) || (otmp == container)) {
                ++count;
                if (!can_reach_floor(TRUE)) {
                    You_cant("reach %s from up here.", the(xname(otmp)));
                    return PICKLOCK_LEARNED_SOMETHING;
                }
                it = 0;
                if (otmp->obroken)
                    verb = "fix";
                else if (otmp->otyp == IRON_SAFE)
                    verb = "crack", it = 1;
                else if (!otmp->olocked)
                    verb = "lock", it = 1;
                else if (picktyp != LOCK_PICK)
                    verb = "unlock", it = 1;
                else
                    verb = "pick";

                if (autounlock) {
                    Sprintf(qbuf, "%s it with %s?",
                            otmp->otyp == IRON_SAFE ? "Crack" : "Unlock",
                            yname(pick));
                    c = yn(qbuf);
                    if (c == 'n')
                        return 0;
                } else {
                    /* "There is <a box> here; <verb> <it|its lock>?" */
                    Sprintf(qsfx, " here; %s %s?",
                            verb, it ? "it" : "its lock");
                    (void) safe_qbuf(qbuf, "There is ", qsfx, otmp, doname,
                                     ansimpleoname, "a box");
                    otmp->lknown = 1;

                    c = ynq(qbuf);
                    if (c == 'q')
                        return 0;
                    if (c == 'n') {
                        otmp = (otmp == mchest) ? level.objects[cc.x][cc.y] : otmp->nexthere;
                        continue;
                    }
                }

                if (otmp->obroken) {
                    You_cant("fix its broken lock with %s.", doname(pick));
                    return PICKLOCK_LEARNED_SOMETHING;
                } else if (picktyp == CREDIT_CARD && !otmp->olocked) {
                    /* credit cards are only good for unlocking */
                    You_cant("do that with %s.",
                             an(simple_typename(picktyp)));
                    return PICKLOCK_LEARNED_SOMETHING;
                } else if (autounlock && !touch_artifact(pick, &youmonst)) {
                    /* note: for !autounlock, apply already did touch check */
                    return PICKLOCK_DID_SOMETHING;
                }

                if (otmp->otyp == IRON_SAFE) {
                    if (!otmp->olocked) {
                        pline_The("safe door is already hanging wide open.");
                        return PICKLOCK_LEARNED_SOMETHING;
                    } else if (picktyp != STETHOSCOPE) {
                        You("don't %s a %s that would fit %s.",
                            Blind ? "feel" : "see",
                            (picktyp == CREDIT_CARD) ? "gap" : "keyhole",
                            an(simple_typename(picktyp)));
                        return PICKLOCK_LEARNED_SOMETHING;
                    }
                }

                /* stethoscopes only work on safes */
                if (picktyp == STETHOSCOPE && otmp->otyp != IRON_SAFE) {
                    You_cant("open such a container with a stethoscope.");
                    return PICKLOCK_LEARNED_SOMETHING;
                }

                /* crystal chest can only be opened by artifacts */
                if (otmp->otyp == CRYSTAL_CHEST && !pick->oartifact) {
                    You_cant("%s %ssuch a container with a mundane %s.",
                             verb, it ? "" : "the lock on ",
                             simple_typename(picktyp));
                    return PICKLOCK_LEARNED_SOMETHING;
                }

                /* magic chests can be opened by artifacts or purpose-made keys */
                if (otmp->otyp == HIDDEN_CHEST
                    && !(pick->oartifact || pick->otyp == MAGIC_KEY)) {
                    You_cant("%s %ssuch a container with a mundane %s.",
                             verb, it ? "" : "the lock on ",
                             simple_typename(picktyp));
                    return PICKLOCK_LEARNED_SOMETHING;
                }

                switch (picktyp) {
                case CREDIT_CARD:
                    ch = ACURR(A_DEX) + 20 * Role_if(PM_ROGUE);
                    break;
                case LOCK_PICK:
                    ch = 4 * ACURR(A_DEX) + 25 * Role_if(PM_ROGUE);
                    break;
                case SKELETON_KEY:
                case MAGIC_KEY:
                    ch = 75 + ACURR(A_DEX);
                    break;
                case STETHOSCOPE:
                    ch = 5 + 2 * ACURR(A_DEX) * Role_if(PM_ROGUE);
                    break;
                default:
                    ch = 0;
                }
                if (otmp->cursed)
                    ch /= 2;

                /* small chance a cursed locking tool will break on use */
                if (pick->cursed && !rn2(5)
                    && picktyp != STETHOSCOPE
                    && picktyp != MAGIC_KEY
                    && pick->oartifact != ART_MASTER_KEY_OF_THIEVERY
                    && pick->oartifact != ART_YENDORIAN_EXPRESS_CARD) {
                    pline("As you start to %s the %s, your %s breaks!",
                          (otmp->olocked ? "unlock" : "lock"),
                          xname(otmp), xname(pick));
                    if (carried(pick))
                        useup(pick);
                    else
                        delobj(pick);
                    nomul(0);
                    return PICKLOCK_DID_NOTHING;
                }

                xlock.box = otmp;
                xlock.door = 0;
                break;
            }
            otmp = (otmp == mchest) ? level.objects[cc.x][cc.y] : otmp->nexthere;
        }
        if (c != 'y') {
            if (!count)
                There("doesn't seem to be any sort of pickable lock here.");
            return PICKLOCK_LEARNED_SOMETHING; /* decided against all boxes */
        }
    } else { /* pick the lock in a door */
        struct monst *mtmp;

        if (u.utrap && u.utraptype == TT_PIT) {
            You_cant("reach over the edge of the pit.");
            return PICKLOCK_LEARNED_SOMETHING;
        }

        door = &levl[cc.x][cc.y];
        mtmp = m_at(cc.x, cc.y);
        if (mtmp && canseemon(mtmp) && M_AP_TYPE(mtmp) != M_AP_FURNITURE
            && M_AP_TYPE(mtmp) != M_AP_OBJECT) {
            if (picktyp == CREDIT_CARD
                && (mtmp->isshk || mtmp->data == &mons[PM_ORACLE]))
                verbalize("No checks, no credit, no problem.");
            else
                pline("I don't think %s would appreciate that.",
                      mon_nam(mtmp));
            return PICKLOCK_LEARNED_SOMETHING;
        } else if (mtmp && is_door_mappear(mtmp)) {
            /* "The door actually was a <mimic>!" */
            stumble_onto_mimic(mtmp);
            /* mimic might keep the key (50% chance, 10% for PYEC or MKoT) */
            maybe_absorb_item(mtmp, pick, 50, 10);
            return PICKLOCK_LEARNED_SOMETHING;
        }
        if (!IS_DOOR(door->typ)) {
            if (is_drawbridge_wall(cc.x, cc.y) >= 0)
                You("%s no lock on the drawbridge.", Blind ? "feel" : "see");
            else
                You("%s no door there.", Blind ? "feel" : "see");
            return PICKLOCK_LEARNED_SOMETHING;
        }
        switch (door->doormask) {
        case D_NODOOR:
            pline("This doorway has no door.");
            return PICKLOCK_LEARNED_SOMETHING;
        case D_ISOPEN:
            You("cannot lock an open door.");
            return PICKLOCK_LEARNED_SOMETHING;
        case D_BROKEN:
            pline("This door is broken.");
            return PICKLOCK_LEARNED_SOMETHING;
        default:
            /* credit cards are only good for unlocking */
            if (picktyp == CREDIT_CARD && !(door->doormask & D_LOCKED)) {
                You_cant("lock a door with a credit card.");
                return PICKLOCK_LEARNED_SOMETHING;
            }

            Sprintf(qbuf, "%s it%s%s?",
                    (door->doormask & D_LOCKED) ? "Unlock" : "Lock",
                    autounlock ? " with " : "",
                    autounlock ? yname(pick) : "");

            c = yn(qbuf);
            if (c == 'n')
                return 0;

            /* note: for !autounlock, 'apply' already did touch check */
            if (autounlock && !touch_artifact(pick, &youmonst))
                return PICKLOCK_DID_SOMETHING;

            switch (picktyp) {
            case CREDIT_CARD:
                ch = 2 * ACURR(A_DEX) + 20 * Role_if(PM_ROGUE);
                break;
            case LOCK_PICK:
                ch = 3 * ACURR(A_DEX) + 30 * Role_if(PM_ROGUE);
                break;
            case SKELETON_KEY:
            case MAGIC_KEY:
                ch = 70 + ACURR(A_DEX);
                break;
            default:
                ch = 0;
            }

            /* small chance a cursed locking tool will break on use */
            if (pick->cursed && !rn2(5)
                && picktyp != STETHOSCOPE
                && picktyp != MAGIC_KEY
                && pick->oartifact != ART_MASTER_KEY_OF_THIEVERY
                && pick->oartifact != ART_YENDORIAN_EXPRESS_CARD) {
                pline("As you start to %s the door, your %s breaks!",
                      ((door->doormask & D_LOCKED) ? "unlock" : "lock"),
                      xname(pick));
                if (carried(pick))
                    useup(pick);
                else
                    delobj(pick);
                nomul(0);
                return PICKLOCK_DID_NOTHING;
            }

            xlock.door = door;
            xlock.box = 0;
        }
    }

    context.move = 0;
    xlock.chance = ch;
    xlock.picktyp = picktyp;
    xlock.magic_key = is_roguish_key(&youmonst, pick);
    xlock.usedtime = 0;
    set_occupation(picklock, lock_action(), 0);
    return PICKLOCK_DID_SOMETHING;
}

/* try to force a chest with your weapon */
int
doforce()
{
    register struct obj *otmp;
    register int c, picktyp;
    char qbuf[QBUFSZ];

    if (u.uswallow || Hidinshell) {
        You_cant("force anything from inside here.");
        return 0;
    }
    if (!uwep /* proper type test */
        || ((uwep->oclass == WEAPON_CLASS || is_weptool(uwep))
               ? (objects[uwep->otyp].oc_skill < P_DAGGER
                  || objects[uwep->otyp].oc_skill == P_FLAIL
                  || objects[uwep->otyp].oc_skill > P_LANCE)
               : uwep->oclass != ROCK_CLASS)) {
        You_cant("force anything %s weapon.",
                 !uwep ? "when not wielding a"
                       : (uwep->oclass != WEAPON_CLASS && !is_weptool(uwep))
                             ? "without a proper"
                             : "with that");
        return 0;
    }
    if (!can_reach_floor(TRUE)) {
        cant_reach_floor(u.ux, u.uy, FALSE, TRUE);
        return 0;
    }

    picktyp = is_blade(uwep) && !is_pick(uwep);
    if (xlock.usedtime && xlock.box && picktyp == xlock.picktyp) {
        You("resume your attempt to force the lock.");
        set_occupation(forcelock, "forcing the lock", 0);
        return 1;
    }

    /* A lock is made only for the honest man, the thief will break it. */
    xlock.box = (struct obj *) 0;
    for (otmp = level.objects[u.ux][u.uy]; otmp; otmp = otmp->nexthere)
        if (Is_box(otmp)) {
            if (otmp->otyp == IRON_SAFE) {
                You("would need dynamite to force %s.", the(xname(otmp)));
                continue;
	    }
            if (otmp->otyp == CRYSTAL_CHEST) {
                You_cant("force the lock of such a container.");
                continue;
            }
            if (otmp->obroken || !otmp->olocked) {
                /* force doname() to omit known "broken" or "unlocked"
                   prefix so that the message isn't worded redundantly;
                   since we're about to set lknown, there's no need to
                   remember and then reset its current value */
                otmp->lknown = 0;
                There("is %s here, but its lock is already %s.",
                      doname(otmp), otmp->obroken ? "broken" : "unlocked");
                otmp->lknown = 1;
                continue;
            }
            (void) safe_qbuf(qbuf, "There is ", " here; force its lock?",
                             otmp, doname, ansimpleoname, "a box");
            otmp->lknown = 1;

            c = ynq(qbuf);
            if (c == 'q')
                return 0;
            if (c == 'n')
                continue;

            if (picktyp)
                You("force %s into a crack and pry.", yname(uwep));
            else
                You("start bashing it with %s.", yname(uwep));
            xlock.box = otmp;
            xlock.chance = objects[uwep->otyp].oc_wldam * 2;
            xlock.picktyp = picktyp;
            xlock.magic_key = FALSE;
            xlock.usedtime = 0;
            break;
        }

    if (xlock.box)
        set_occupation(forcelock, "forcing the lock", 0);
    else
        You("decide not to force the issue.");
    return 1;
}

boolean
stumble_on_door_mimic(x, y)
int x, y;
{
    struct monst *mtmp;

    if ((mtmp = m_at(x, y)) && is_door_mappear(mtmp)
        && !Protection_from_shape_changers) {
        stumble_onto_mimic(mtmp);
        return TRUE;
    }
    return FALSE;
}

/* the 'O' command - try to open a door */
int
doopen()
{
    return doopen_indir(0, 0);
}

/* try to open a door in direction u.dx/u.dy */
int
doopen_indir(x, y)
int x, y;
{
    coord cc;
    register struct rm *door;
    int res = 0;
    boolean portcullis;

    if (nohands(youmonst.data)) {
        if (druid_form && !slithy(youmonst.data)) {
            ;
        } else if (vampire_form && !is_whirly(youmonst.data)) {
            ;
        } else {
            You_cant("open anything -- you have no hands!");
            return 0;
        }
    }

    if (!freehand()) {
        You_cant("open anything -- you have no %s free!",
                 makeplural(body_part(HAND)));
        return 0;
    }

    if (u.utrap && u.utraptype == TT_PIT) {
        You_cant("reach over the edge of the pit.");
        return 0;
    }

    if (x > 0 && y > 0) {
        cc.x = x;
        cc.y = y;
    } else if (!get_adjacent_loc((char *) 0, (char *) 0, u.ux, u.uy, &cc))
        return 0;

    /* open at yourself/up/down */
    if ((cc.x == u.ux) && (cc.y == u.uy))
        return doloot();

    if (stumble_on_door_mimic(cc.x, cc.y))
        return 1;

    /* when choosing a direction is impaired, use a turn
       regardless of whether a door is successfully targetted */
    if (Confusion || Stunned)
        res = 1;

    door = &levl[cc.x][cc.y];
    portcullis = (is_drawbridge_wall(cc.x, cc.y) >= 0);
    if (Blind) {
        int oldglyph = door->glyph;
        schar oldlastseentyp = lastseentyp[cc.x][cc.y];

        feel_location(cc.x, cc.y);
        if (door->glyph != oldglyph
            || lastseentyp[cc.x][cc.y] != oldlastseentyp)
            res = 1; /* learned something */
    }

    if (portcullis || !IS_DOOR(door->typ)) {
        /* closed portcullis or spot that opened bridge would span */
        if (is_db_wall(cc.x, cc.y) || door->typ == DRAWBRIDGE_UP)
            There("is no obvious way to open the drawbridge.");
        else if (portcullis || door->typ == DRAWBRIDGE_DOWN)
            pline_The("drawbridge is already open.");
        else if (container_at(cc.x, cc.y, TRUE))
            pline("%s like something lootable over there.",
                  Blind ? "Feels" : "Seems");
        else
            You("%s no door there.", Blind ? "feel" : "see");
        return res;
    }

    if (!(door->doormask & D_CLOSED)) {
        const char *mesg;
        boolean locked = FALSE;
        struct obj* unlocktool;

        switch (door->doormask) {
        case D_BROKEN:
            mesg = " is broken";
            break;
        case D_NODOOR:
            mesg = "way has no door";
            break;
        case D_ISOPEN:
            mesg = " is already open";
            break;
        default:
            mesg = " is locked";
            locked = TRUE;
            break;
        }
        pline("This door%s.", mesg);
        if (locked && flags.autounlock && (unlocktool = autokey(TRUE)) != 0) {
            res = pick_lock(unlocktool, cc.x, cc.y, (struct obj *) 0);
        }
        return res;
    }

    if (verysmall(youmonst.data)) {
        pline("You're too small to pull the door open.");
        return res;
    }

    /* door is known to be CLOSED */
    if (rnl(20) < (ACURRSTR + ACURR(A_DEX) + ACURR(A_CON)) / 3) {
        pline_The("door opens.");
        if (door->doormask & D_TRAPPED) {
            door->doormask = D_NODOOR;
            b_trapped("door", FINGER);
            if (*in_rooms(cc.x, cc.y, SHOPBASE))
                add_damage(cc.x, cc.y, SHOP_DOOR_COST);
        } else
            door->doormask = D_ISOPEN;
        feel_newsym(cc.x, cc.y); /* the hero knows she opened it */
        unblock_point(cc.x, cc.y); /* vision: new see through there */
    } else {
        exercise(A_STR, TRUE);
        pline_The("door resists!");
    }

    return 1;
}

STATIC_OVL boolean
obstructed(x, y, quietly)
register int x, y;
boolean quietly;
{
    register struct monst *mtmp = m_at(x, y);

    if (mtmp && M_AP_TYPE(mtmp) != M_AP_FURNITURE) {
        if (M_AP_TYPE(mtmp) == M_AP_OBJECT)
            goto objhere;
        if (!quietly) {
            if ((mtmp->mx != x) || (mtmp->my != y)) {
                /* worm tail */
                pline("%s%s blocks the way!",
                      !canspotmon(mtmp) ? Something : s_suffix(Monnam(mtmp)),
                      !canspotmon(mtmp) ? "" : " tail");
            } else {
                pline("%s blocks the way!",
                      !canspotmon(mtmp) ? "Some creature" : Monnam(mtmp));
            }
        }
        if (!canspotmon(mtmp))
            map_invisible(x, y);
        return TRUE;
    }
    if (OBJ_AT(x, y)) {
 objhere:
        if (!quietly)
            pline("%s's in the way.", Something);
        return TRUE;
    }
    return FALSE;
}

/* the 'C' command - try to close a door */
int
doclose()
{
    register int x, y;
    register struct rm *door;
    int res = 0;
    boolean portcullis;

    if (nohands(youmonst.data)) {
        if (druid_form && !slithy(youmonst.data)) {
            ;
        } else if (vampire_form && !is_whirly(youmonst.data)) {
            ;
        } else {
            You_cant("close anything -- you have no hands!");
            return 0;
        }
    }

    if (!freehand()) {
        You_cant("close anything -- you have no %s free!",
                 makeplural(body_part(HAND)));
        return 0;
    }

    if (u.utrap && u.utraptype == TT_PIT) {
        You_cant("reach over the edge of the pit.");
        return 0;
    }

    if (!getdir((char *) 0))
        return 0;

    x = u.ux + u.dx;
    y = u.uy + u.dy;
    if ((x == u.ux) && (y == u.uy)) {
        You("are in the way!");
        return 1;
    }

    if (!isok(x, y))
        goto nodoor;

    if (stumble_on_door_mimic(x, y))
        return 1;

    /* when choosing a direction is impaired, use a turn
       regardless of whether a door is successfully targetted */
    if (Confusion || Stunned)
        res = 1;

    door = &levl[x][y];
    portcullis = (is_drawbridge_wall(x, y) >= 0);
    if (Blind) {
        int oldglyph = door->glyph;
        schar oldlastseentyp = lastseentyp[x][y];

        feel_location(x, y);
        if (door->glyph != oldglyph || lastseentyp[x][y] != oldlastseentyp)
            res = 1; /* learned something */
    }

    if (portcullis || !IS_DOOR(door->typ)) {
        /* is_db_wall: closed portcullis */
        if (is_db_wall(x, y) || door->typ == DRAWBRIDGE_UP)
            pline_The("drawbridge is already closed.");
        else if (portcullis || door->typ == DRAWBRIDGE_DOWN)
            There("is no obvious way to close the drawbridge.");
        else {
 nodoor:
            You("%s no door there.", Blind ? "feel" : "see");
        }
        return res;
    }

    if (door->doormask == D_NODOOR) {
        pline("This doorway has no door.");
        return res;
    } else if (obstructed(x, y, FALSE)) {
        return res;
    } else if (door->doormask == D_BROKEN) {
        pline("This door is broken.");
        return res;
    } else if (door->doormask & (D_CLOSED | D_LOCKED)) {
        pline("This door is already closed.");
        return res;
    }

    if (door->doormask == D_ISOPEN) {
        if (verysmall(youmonst.data) && !u.usteed) {
            pline("You're too small to push the door closed.");
            return res;
        }
        if (u.usteed
            || rn2(25) < (ACURRSTR + ACURR(A_DEX) + ACURR(A_CON)) / 3) {
            pline_The("door closes.");
            door->doormask = D_CLOSED;
            feel_newsym(x, y); /* the hero knows she closed it */
            block_point(x, y); /* vision:  no longer see there */
        } else {
            exercise(A_STR, TRUE);
            pline_The("door resists!");
        }
    }

    return 1;
}

/* box obj was hit with spell or wand effect otmp;
   returns true if something happened */
boolean
boxlock(obj, otmp)
struct obj *obj, *otmp; /* obj *is* a box */
{
    boolean res = 0;

    switch (otmp->otyp) {
    case WAN_LOCKING:
    case SPE_WIZARD_LOCK:
        if (!obj->olocked) { /* lock it; fix if broken */
            pline("Klunk!");
            obj->olocked = 1;
            obj->obroken = 0;
            if (Role_if(PM_WIZARD))
                obj->lknown = 1;
            else
                obj->lknown = 0;
            res = 1;
        } /* else already closed and locked */
        break;
    case WAN_OPENING:
    case SPE_KNOCK:
        if (obj->olocked) { /* unlock; couldn't be broken */
            pline("Klick!");
            obj->olocked = 0;
            res = 1;
            if (Role_if(PM_WIZARD))
                obj->lknown = 1;
            else
                obj->lknown = 0;
        } else /* silently fix if broken */
            obj->obroken = 0;
        break;
    case WAN_POLYMORPH:
    case SPE_POLYMORPH:
        /* maybe start unlocking chest, get interrupted, then zap it;
           we must avoid any attempt to resume unlocking it */
        if (xlock.box == obj)
            reset_pick();
        break;
    }
    return res;
}

/* Door/secret door was hit with spell or wand effect otmp;
   returns true if something happened */
boolean
doorlock(otmp, x, y)
struct obj *otmp;
int x, y;
{
    register struct rm *door = &levl[x][y];
    boolean res = TRUE;
    int loudness = 0;
    const char *msg = (const char *) 0;
    const char *dustcloud = "A cloud of dust";
    const char *quickly_dissipates = "quickly dissipates";
    boolean mysterywand = (otmp->oclass == WAND_CLASS && !otmp->dknown);

    if (door->typ == SDOOR) {
        switch (otmp->otyp) {
        case WAN_OPENING:
        case SPE_KNOCK:
        case WAN_STRIKING:
        case SPE_FORCE_BOLT:
            door->typ = DOOR;
            door->doormask = D_CLOSED | (door->doormask & D_TRAPPED);
            newsym(x, y);
            if (cansee(x, y))
                pline("A door appears in the wall!");
            if (otmp->otyp == WAN_OPENING || otmp->otyp == SPE_KNOCK)
                return TRUE;
            break; /* striking: continue door handling below */
        case WAN_LOCKING:
        case SPE_WIZARD_LOCK:
        default:
            return FALSE;
        }
    }

    switch (otmp->otyp) {
    case WAN_LOCKING:
    case SPE_WIZARD_LOCK:
        if (Is_rogue_level(&u.uz)) {
            boolean vis = cansee(x, y);
            /* Can't have real locking in Rogue, so just hide doorway */
            if (vis)
                pline("%s springs up in the older, more primitive doorway.",
                      dustcloud);
            else
                You_hear("a swoosh.");
            if (obstructed(x, y, mysterywand)) {
                if (vis)
                    pline_The("cloud %s.", quickly_dissipates);
                return FALSE;
            }
            block_point(x, y);
            door->typ = SDOOR, door->doormask = D_NODOOR;
            if (vis)
                pline_The("doorway vanishes!");
            newsym(x, y);
            return TRUE;
        }
        if (obstructed(x, y, mysterywand))
            return FALSE;
        /* Don't allow doors to close over traps.  This is for pits */
        /* & trap doors, but is it ever OK for anything else? */
        if (t_at(x, y)) {
            /* maketrap() clears doormask, so it should be NODOOR */
            pline("%s springs up in the doorway, but %s.", dustcloud,
                  quickly_dissipates);
            return FALSE;
        }
        /* Don't allow the sokoban closet doors to be locked */
        if (Sokoban && (door->doormask & D_TRAPPED) != 0) {
            pline_The("%s has no effect on this door.",
                      otmp->otyp == WAN_LOCKING ? "wand" : "spell");
            return FALSE;
        }

        switch (door->doormask & ~D_TRAPPED) {
        case D_CLOSED:
            msg = "The door locks!";
            break;
        case D_ISOPEN:
            msg = "The door swings shut, and locks!";
            break;
        case D_BROKEN:
            msg = "The broken door reassembles and locks!";
            break;
        case D_NODOOR:
            msg =
               "A cloud of dust springs up and assembles itself into a door!";
            break;
        default:
            res = FALSE;
            break;
        }
        block_point(x, y);
        door->doormask = D_LOCKED | (door->doormask & D_TRAPPED);
        newsym(x, y);
        break;
    case WAN_OPENING:
    case SPE_KNOCK:
        if (door->doormask & D_LOCKED) {
            msg = "The door unlocks!";
            door->doormask = D_CLOSED | (door->doormask & D_TRAPPED);
        } else
            res = FALSE;
        break;
    case WAN_STRIKING:
    case SPE_FORCE_BOLT:
        if (door->doormask & (D_LOCKED | D_CLOSED)) {
            if (door->doormask & D_TRAPPED) {
                if (In_sokoban(&u.uz)) {
                    if (cansee(x,y))
                        pline("The door absorbs the force!");
                } else {
                    if (MON_AT(x, y))
                        (void) mb_trapped(m_at(x, y));
                    else if (flags.verbose) {
                        if (cansee(x, y))
                            pline("KABOOM!!  You see a door explode.");
                        else
                            You_hear("a distant explosion.");
                    }
                door->doormask = D_NODOOR;
                unblock_point(x, y);
                newsym(x, y);
                loudness = 40;
                }
                break;
            }
            door->doormask = D_BROKEN;
            if (flags.verbose) {
                if (cansee(x, y))
                    pline_The("door crashes open!");
                else
                    You_hear("a crashing sound.");
            }
            unblock_point(x, y);
            newsym(x, y);
            /* force vision recalc before printing more messages */
            if (vision_full_recalc)
                vision_recalc(0);
            loudness = 20;
        } else
            res = FALSE;
        break;
    default:
        impossible("magic (%d) attempted on door.", otmp->otyp);
        break;
    }
    if (msg && cansee(x, y))
        pline1(msg);
    if (loudness > 0) {
        /* door was destroyed */
        wake_nearto(x, y, loudness);
        if (*in_rooms(x, y, SHOPBASE))
            add_damage(x, y, 0L);
    }

    if (res && picking_at(x, y)) {
        /* maybe unseen monster zaps door you're unlocking */
        stop_occupation();
        reset_pick();
    }
    return res;
}

STATIC_OVL void
chest_shatter_msg(otmp)
struct obj *otmp;
{
    const char *disposition;
    const char *thing;
    long save_Blinded;

    if (otmp->oclass == POTION_CLASS) {
        You("%s %s shatter!", Blind ? "hear" : "see", an(bottlename()));
        if (!Breathless_nomagic || haseyes(youmonst.data))
            potionbreathe(otmp);
        return;
    }
    /* We have functions for distant and singular names, but not one */
    /* which does _both_... */
    save_Blinded = Blinded;
    Blinded = 1;
    thing = singular(otmp, xname);
    Blinded = save_Blinded;
    switch (otmp->material) {
    case PAPER:
        disposition = "is torn to shreds";
        break;
    case WAX:
        disposition = "is crushed";
        break;
    case VEGGY:
        disposition = "is pulped";
        break;
    case FLESH:
        disposition = "is mashed";
        break;
    case GLASS:
        disposition = "shatters";
        break;
    case WOOD:
        disposition = "splinters to fragments";
        break;
    default:
        disposition = "is destroyed";
        break;
    }
    pline("%s %s!", An(thing), disposition);
}

/*lock.c*/
