/* NetHack 3.6  telekinesis.c  $NHDT-Date: 2026/03/22 $ */
/* Copyright (c) 2026 by Keith Simpson.  NetHack may be freely redistributed. */
/* See license for details. */

#include "hack.h"

/* Forward declarations */
STATIC_DCL int FDECL(tk_max_msize, (int));
STATIC_DCL void FDECL(tk_push_obj, (struct obj *, int, int, int));
STATIC_DCL void FDECL(tk_pull_obj, (struct obj *, int, int));
STATIC_DCL boolean FDECL(tk_impact, (struct monst *, int, int));
STATIC_DCL void FDECL(tk_push_mon, (struct monst *, int, int, int));
STATIC_DCL void FDECL(tk_pull_mon, (struct monst *, int, int));
STATIC_DCL void FDECL(m_tk_push_obj,
                      (struct monst *, struct obj *, int, int, int));

/*
 * Utility functions
 */

/* Maximum monster size that can be affected at a given skill level.
   Returns -1 if no monsters can be affected (Unskilled) */
STATIC_OVL int
tk_max_msize(skill)
int skill;
{
    switch (skill) {
    case P_BASIC:
        return MZ_TINY;
    case P_SKILLED:
        return MZ_SMALL;
    case P_EXPERT:
        return MZ_MEDIUM;
    case P_MASTER:
    case P_GRAND_MASTER:
        return MZ_LARGE;
    default:
        return -1;
    }
}

/* Can the player use telekinesis? */
boolean
can_use_telekinesis(verbose)
boolean verbose;
{
    if (racial_illithid(&youmonst))
        return TRUE;
    if (is_mind_flayer(youmonst.data))
        return TRUE;
    if (Telekinesis)
        return TRUE;
    if (verbose)
        You("lack telekinetic ability.");
    return FALSE;
}

/* Effective telekinesis skill level for the player */
int
tk_skill()
{
    int skill;

    if (!Race_if(PM_ILLITHID)) {
        /* Non-illithids with extrinsic or polymorph get Skilled */
        if (Telekinesis || is_mind_flayer(youmonst.data))
            return P_SKILLED;
        return P_UNSKILLED;
    }
    skill = P_SKILL(P_TELEKINESIS);
    if (skill < P_UNSKILLED)
        skill = P_UNSKILLED;
    return skill;
}

/* Telekinesis range based on skill + helm bonus */
int
tk_range()
{
    static const int ranges[] = { 0, 1, 2, 3, 4, 5 };
    int skill = tk_skill();
    int range;

    if (skill < P_UNSKILLED || skill > P_MASTER)
        return 0;
    range = ranges[skill];
    /* Illithid wearing helm of telekinesis: +1 tile */
    if (uarmh && uarmh->otyp == HELM_OF_TELEKINESIS
        && Race_if(PM_ILLITHID))
        range += 1;
    return range;
}

/* Power cost based on skill + helm reduction */
int
tk_pw_cost()
{
    static const int costs[] = { 0, 2, 5, 10, 15, 20 };
    int skill = tk_skill();
    int cost;

    if (skill < P_UNSKILLED || skill > P_MASTER)
        return 0;
    cost = costs[skill];
    /* Helm gives illithids 50% reduction */
    if (uarmh && uarmh->otyp == HELM_OF_TELEKINESIS
        && Race_if(PM_ILLITHID))
        cost /= 2;
    if (cost < 1)
        cost = 1;
    return cost;
}

/*
 * Player push/pull helpers
 */

/* Push an object away from the player along (dx,dy) */
STATIC_OVL void
tk_push_obj(obj, dx, dy, range)
struct obj *obj;
int dx, dy, range;
{
    int ox, oy, nx, ny, i;
    xchar start_x, start_y;
    boolean was_in_shop;
    boolean stopped;
    struct monst *mtmp;
    int weight, dmg;

    /* Split off one item from a stack */
    if (obj->quan > 1L)
        obj = splitobj(obj, 1L);

    ox = obj->ox;
    oy = obj->oy;
    start_x = ox;
    start_y = oy;
    was_in_shop = costly_spot(ox, oy) && *u.ushops;
    weight = (int) obj->owt;

    remove_object(obj);
    newsym(ox, oy);

    stopped = FALSE;
    for (i = 0; i < range && !stopped; i++) {
        nx = ox + dx;
        ny = oy + dy;

        if (!isok(nx, ny)) {
            stopped = TRUE;
            break;
        }
        /* Chain constraint: ball can't exceed chain length */
        if (obj == uball && dist2(u.ux, u.uy, nx, ny) > 8) {
            pline_The("chain goes taut!");
            stopped = TRUE;
            break;
        }
        /* Solid terrain */
        if (IS_STWALL(levl[nx][ny].typ) || IS_TREES(levl[nx][ny].typ)
            || closed_door(nx, ny)
            || levl[nx][ny].typ == IRONBARS) {
            pline_The("%s slams against the %s!", xname(obj),
                      IS_TREE(levl[nx][ny].typ) ? "tree"
                      : IS_DEADTREE(levl[nx][ny].typ) ? "dead tree"
                      : levl[nx][ny].typ == IRONBARS ? "iron bars"
                                                     : "wall");
            stopped = TRUE;
            break;
        }
        /* Monster in the way */
        mtmp = m_at(nx, ny);
        if (mtmp) {
            if (weight >= 20) {
                dmg = d(max(1, weight / 50), 4);
                pline_The("%s strikes %s!", xname(obj),
                          mon_nam(mtmp));
                if (obj->otyp == CORPSE
                    && safe_touch_petrifies(obj->corpsenm)
                    && !resists_ston(mtmp)) {
                    minstapetrify(mtmp, TRUE);
                } else {
                    if (damage_mon(mtmp, dmg, AD_PHYS, TRUE))
                        xkilled(mtmp, XKILL_GIVEMSG);
                    else
                        wakeup(mtmp, TRUE);
                }
            } else {
                pline_The("%s bumps against %s.", xname(obj),
                          mon_nam(mtmp));
                wakeup(mtmp, TRUE);
            }
            ox = nx;
            oy = ny;
            stopped = TRUE;
            break;
        }
        ox = nx;
        oy = ny;
    }

    /* Place at final position; may be destroyed by terrain */
    if (flooreffects(obj, ox, oy, "fall"))
        return;
    place_object(obj, ox, oy);
    newsym(ox, oy);

    /* Sokoban guilt for boulders only */
    if (obj->otyp == BOULDER && In_sokoban(&u.uz))
        sokoban_guilt();

    /* Shop theft */
    if (was_in_shop && !costly_spot(ox, oy))
        remote_burglary(start_x, start_y);
}

/* Pull an object towards the player's inventory, or slide a boulder */
STATIC_OVL void
tk_pull_obj(obj, dx, dy)
struct obj *obj;
int dx, dy;
{
    int steps;

    if (obj->otyp == BOULDER) {
        /* Boulder: slide towards player, stop 1 tile in front */
        int target_x = u.ux + dx;
        int target_y = u.uy + dy;

        steps = max(abs(obj->ox - target_x),
                    abs(obj->oy - target_y));
        if (steps > 0) {
            You("telekinetically pull %s towards you.",
                the(xname(obj)));
            tk_push_obj(obj, -dx, -dy, steps);
        }
        return;
    }
    if (obj == uball) {
        pline("It's chained to you!");
        return;
    }
    if (obj == uchain) {
        You_cant("move the chain with telekinesis.");
        return;
    }

    /* Use existing remote pickup mechanism */
    (void) pickup_object(obj, 1L, TRUE);
}

/* Check if a monster slammed into an obstacle after hurtling
   and apply impact damage + feedback.  hdx/hdy is the direction
   the monster was moving.  Returns TRUE if monster died. */
STATIC_OVL boolean
tk_impact(mtmp, hdx, hdy)
struct monst *mtmp;
int hdx, hdy;
{
    int nx, ny, dmg;
    const char *what;

    if (DEADMONSTER(mtmp))
        return TRUE;

    /* Check the next tile in the movement direction */
    nx = mtmp->mx + hdx;
    ny = mtmp->my + hdy;

    if (!isok(nx, ny)) {
        what = "wall";
    } else if (IS_STWALL(levl[nx][ny].typ)) {
        what = "wall";
    } else if (IS_TREE(levl[nx][ny].typ)) {
        what = "tree";
    } else if (IS_DEADTREE(levl[nx][ny].typ)) {
        what = "dead tree";
    } else if (closed_door(nx, ny)) {
        what = "door";
    } else if (levl[nx][ny].typ == IRONBARS) {
        what = "iron bars";
    } else if (sobj_at(BOULDER, nx, ny)) {
        what = "boulder";
    } else {
        /* Didn't hit a terrain obstacle (may have hit a monster,
           which mhurtle_step already handles with "bumps into") */
        return FALSE;
    }

    dmg = rnd(6);
    if (canspotmon(mtmp))
        pline("%s slams into the %s!", Monnam(mtmp), what);
    if (damage_mon(mtmp, dmg, AD_PHYS, TRUE)) {
        if (canseemon(mtmp))
            pline("%s is killed!", Monnam(mtmp));
        xkilled(mtmp, XKILL_NOMSG);
        return TRUE;
    }
    return FALSE;
}

/* Push a monster away using mhurtle */
STATIC_OVL void
tk_push_mon(mtmp, dx, dy, range)
struct monst *mtmp;
int dx, dy, range;
{
    You("telekinetically hurl %s!", mon_nam(mtmp));
    mhurtle(mtmp, dx, dy, range);
    tk_impact(mtmp, dx, dy);
}

/* Pull a monster towards the player, stopping 1 tile away */
STATIC_OVL void
tk_pull_mon(mtmp, dx, dy)
struct monst *mtmp;
int dx, dy;
{
    int steps;

    steps = max(abs(mtmp->mx - u.ux), abs(mtmp->my - u.uy));
    if (steps <= 1) {
        pline("%s is already next to you!", Monnam(mtmp));
        return;
    }
    You("telekinetically pull %s towards you!",
        mon_nam(mtmp));
    mhurtle(mtmp, -dx, -dy, steps - 1);
    tk_impact(mtmp, -dx, -dy);
}

/*
 * Player command entry point
 */
int
dotelekinesis()
{
    int dx, dy;
    int skill, range, cost;
    boolean do_pull;
    struct obj *target_obj;
    struct monst *target_mon;
    int max_msize;
    coord cc;
    int dist, i, px, py;
    char targetname[BUFSZ];

    if (!can_use_telekinesis(TRUE))
        return 0;

    if (Confusion || Stunned) {
        You_cant("focus your telekinetic power while incapacitated.");
        return 0;
    }
    if (u.uswallow) {
        pline("There's not enough room!");
        return 0;
    }
    if (Underwater && !Amphibious) {
        You_cant("concentrate underwater.");
        return 0;
    }
    if (ACURR(A_INT) < 6) {
        You_cant("muster the mental focus for telekinesis.");
        return 0;
    }
    if (uarmh && is_heavy_metallic(uarmh)
        && uarmh->oartifact != ART_MITRE_OF_HOLINESS) {
        pline_The("%s of your %s blocks your telekinetic power.",
                  materialnm[uarmh->material],
                  helm_simple_name(uarmh));
        return 0;
    }

    skill = tk_skill();
    range = tk_range();
    cost = tk_pw_cost();

    if (u.uen < cost) {
        You("lack the energy to use your telekinetic ability.");
        return 0;
    }

    /* Push/Pull choice; Unskilled can only push */
    do_pull = FALSE;
    if (skill >= P_BASIC) {
        winid win;
        menu_item *selected;
        anything any;
        int n;

        win = create_nhwindow(NHW_MENU);
        start_menu(win);

        any = zeroany;
        any.a_int = 1;
        add_menu(win, NO_GLYPH, &any, 'a', 0, ATR_NONE,
                 "Push something away", MENU_UNSELECTED);
        any.a_int = 2;
        add_menu(win, NO_GLYPH, &any, 'b', 0, ATR_NONE,
                 "Pull something closer", MENU_UNSELECTED);

        end_menu(win, "Use telekinesis to:");
        n = select_menu(win, PICK_ONE, &selected);
        destroy_nhwindow(win);
        if (n <= 0)
            return 0;
        do_pull = (selected[0].item.a_int == 2);
        free((genericptr_t) selected);
    }

    /* Cursor targeting */
    cc.x = u.ux;
    cc.y = u.uy;
    pline("Pick a target.");
    if (getpos(&cc, TRUE, "the target") < 0)
        return 0;

    /* Can't target self */
    if (cc.x == u.ux && cc.y == u.uy) {
        You_cant("target yourself.");
        return 0;
    }

    /* Peek at target for descriptive error messages */
    targetname[0] = '\0';
    {
        struct monst *mtmp = m_at(cc.x, cc.y);

        if (mtmp && (cansee(cc.x, cc.y) || tp_sensemon(mtmp))) {
            Strcpy(targetname, mon_nam(mtmp));
        } else if (cansee(cc.x, cc.y) && OBJ_AT(cc.x, cc.y)) {
            struct obj *otmp;

            for (otmp = level.objects[cc.x][cc.y]; otmp;
                 otmp = otmp->nexthere) {
                if (otmp != uchain) {
                    Strcpy(targetname, the(xname(otmp)));
                    break;
                }
            }
        }
    }

    /* Must be in a straight line (cardinal or diagonal) */
    dx = cc.x - u.ux;
    dy = cc.y - u.uy;
    if (dx != 0 && dy != 0 && abs(dx) != abs(dy)) {
        if (*targetname)
            You("aren't lined up with %s.", targetname);
        else
            pline("That spot is not in a straight line.");
        return 0;
    }

    /* Check range */
    dist = max(abs(dx), abs(dy));
    if (dist > range) {
        if (*targetname)
            pline("%s is out of range.", upstart(targetname));
        else
            pline("That spot is out of range.");
        return 0;
    }

    /* Normalize direction */
    dx = sgn(dx);
    dy = sgn(dy);

    /* Check path for walls and closed doors (iron bars, monsters,
       and boulders along the path don't block targeting -- the
       pushed/pulled entity will slam into them instead) */
    px = u.ux;
    py = u.uy;
    for (i = 0; i < dist; i++) {
        px += dx;
        py += dy;
        if (px == cc.x && py == cc.y)
            break; /* reached target tile */
        if (IS_STWALL(levl[px][py].typ) || closed_door(px, py)) {
            You_cant("see a clear path there.");
            return 0;
        }
    }

    /* Check for valid target at the selected tile */
    target_obj = (struct obj *) 0;
    target_mon = (struct monst *) 0;
    max_msize = tk_max_msize(skill);

    /* Monsters take priority over objects */
    {
        struct monst *mtmp = m_at(cc.x, cc.y);

        if (mtmp && (cansee(cc.x, cc.y) || tp_sensemon(mtmp))) {
            if (max_msize < 0
                || (int) mtmp->data->msize > max_msize) {
                pline("%s is too large to move.", Monnam(mtmp));
                return 0;
            }
            target_mon = mtmp;
        }
    }

    /* Check for objects if no monster targeted */
    if (!target_mon && cansee(cc.x, cc.y)
        && OBJ_AT(cc.x, cc.y)) {
        struct obj *otmp;

        for (otmp = level.objects[cc.x][cc.y]; otmp;
             otmp = otmp->nexthere) {
            if (otmp == uchain)
                continue;
            if (otmp->otyp == BOULDER && skill < P_MASTER) {
                pline("That boulder is too heavy to move.");
                return 0;
            }
            target_obj = otmp;
            break;
        }
    }

    if (!target_obj && !target_mon) {
        pline("There's nothing there to move.");
        return 0;
    }

    /* Deduct power and train skill */
    u.uen -= cost;
    context.botl = 1;
    if (Race_if(PM_ILLITHID))
        use_skill(P_TELEKINESIS, 1);

    /* Execute push or pull */
    if (target_mon) {
        if (do_pull)
            tk_pull_mon(target_mon, dx, dy);
        else
            tk_push_mon(target_mon, dx, dy, range);
    } else {
        if (do_pull) {
            tk_pull_obj(target_obj, dx, dy);
        } else {
            You("telekinetically push %s.", the(xname(target_obj)));
            tk_push_obj(target_obj, dx, dy, range);
        }
    }

    return 1;
}

/*
 * Monster telekinesis AI
 */

/* Monster pushes an object towards a target along (dx,dy) */
STATIC_OVL void
m_tk_push_obj(mtmp, obj, dx, dy, range)
struct monst *mtmp;
struct obj *obj;
int dx, dy, range;
{
    int ox, oy, nx, ny, i;
    boolean stopped;
    struct monst *target;
    int weight, dmg;

    /* Split off one item from a stack */
    if (obj->quan > 1L)
        obj = splitobj(obj, 1L);

    ox = obj->ox;
    oy = obj->oy;
    weight = (int) obj->owt;

    remove_object(obj);
    newsym(ox, oy);
    stopped = FALSE;

    for (i = 0; i < range && !stopped; i++) {
        nx = ox + dx;
        ny = oy + dy;

        if (!isok(nx, ny)) {
            stopped = TRUE;
            break;
        }
        if (IS_STWALL(levl[nx][ny].typ) || IS_TREES(levl[nx][ny].typ)
            || closed_door(nx, ny)
            || levl[nx][ny].typ == IRONBARS) {
            stopped = TRUE;
            break;
        }
        /* Hits the player */
        if (nx == u.ux && ny == u.uy) {
            /* Potions shatter and splash — potionhit handles
               everything and destroys the object */
            if (obj->oclass == POTION_CLASS) {
                pline("%s telekinetically hurls %s at you!",
                      Monnam(mtmp), an(xname(obj)));
                potionhit(&youmonst, obj, POTHIT_OTHER_THROW);
                return;
            }
            if (weight >= 20 || obj->oclass == WEAPON_CLASS) {
                dmg = d(max(1, weight / 50), 4);
                pline("%s telekinetically hurls %s at you!",
                      Monnam(mtmp), an(xname(obj)));
                if ((obj->otyp == CORPSE
                     && safe_touch_petrifies(obj->corpsenm))
                    || (obj->otyp == EGG
                        && safe_touch_petrifies(obj->corpsenm))) {
                    if (!Stone_resistance
                        && !(poly_when_stoned(youmonst.data)
                             && polymon(PM_STONE_GOLEM))) {
                        killer.format = KILLED_BY_AN;
                        Sprintf(killer.name,
                                "telekinetically hurled %s %s",
                                mons[obj->corpsenm].mname,
                                obj->otyp == EGG ? "egg" : "corpse");
                        done(STONING);
                    }
                } else if (obj->otyp == CREAM_PIE) {
                    dmg = Maybe_Half_Phys(dmg);
                    losehp(dmg,
                           "telekinetically hurled cream pie",
                           KILLED_BY_AN);
                    if (can_blnd(mtmp, &youmonst,
                                 (uchar) AT_WEAP, obj)) {
                        if (!Blind)
                            pline("Yecch!  You've been creamed.");
                        else
                            pline("There's %s sticky all over your %s.",
                                  something, body_part(FACE));
                        make_blinded(Blinded + (long) rnd(25), FALSE);
                    }
                } else {
                    dmg = Maybe_Half_Phys(dmg);
                    losehp(dmg,
                           "telekinetically hurled object",
                           KILLED_BY_AN);
                }
            } else {
                pline("%s telekinetically flings %s at you.",
                      Monnam(mtmp), an(xname(obj)));
            }
            ox = nx;
            oy = ny;
            stopped = TRUE;
            break;
        }
        /* Hits another monster */
        target = m_at(nx, ny);
        if (target) {
            /* Potions shatter and splash */
            if (obj->oclass == POTION_CLASS) {
                potionhit(target, obj, POTHIT_OTHER_THROW);
                return; /* obj destroyed */
            }
            if (weight >= 20 || obj->oclass == WEAPON_CLASS) {
                dmg = d(max(1, weight / 50), 4);
                if (canseemon(target))
                    pline("%s strikes %s!", An(xname(obj)),
                          mon_nam(target));
                if ((obj->otyp == CORPSE || obj->otyp == EGG)
                    && safe_touch_petrifies(obj->corpsenm)
                    && !resists_ston(target)) {
                    minstapetrify(target, TRUE);
                } else {
                    if (damage_mon(target, dmg, AD_PHYS, FALSE))
                        mondied(target);
                    else
                        wakeup(target, TRUE);
                }
            } else {
                wakeup(target, TRUE);
            }
            ox = nx;
            oy = ny;
            stopped = TRUE;
            break;
        }
        ox = nx;
        oy = ny;
    }

    if (flooreffects(obj, ox, oy, "fall"))
        return;
    place_object(obj, ox, oy);
    newsym(ox, oy);
}

/* Monster uses telekinesis against a target (player or monster).
   Uses mfind_target() for target selection, which handles
   hostile/tame/peaceful/conflict logic.
   Returns TRUE if an action was taken. */
boolean
m_dotelekinesis(mtmp)
struct monst *mtmp;
{
    struct monst *mdef;
    int dx, dy;
    int mx, my, tx, ty, dist, range, max_msize;
    boolean is_master, target_is_you;
    int x, y, i;
    struct obj *push_obj;

    /* Only mind flayers have TK */
    if (!is_mind_flayer(mtmp->data))
        return FALSE;
    /* Cooldown check */
    if (mtmp->mspec_used)
        return FALSE;

    mx = mtmp->mx;
    my = mtmp->my;
    is_master = (mtmp->data == &mons[PM_MASTER_MIND_FLAYER]
                 || mtmp->data == &mons[PM_ALHOON]);
    range = is_master ? 5 : 3;
    max_msize = is_master ? MZ_LARGE : MZ_SMALL;

    /* Find a target — mfind_target handles hostile vs tame vs
       peaceful vs conflict, and checks line-of-sight */
    mdef = mfind_target(mtmp);
    if (!mdef || mdef == mtmp)
        return FALSE;

    target_is_you = (mdef == &youmonst);
    tx = target_is_you ? u.ux : mdef->mx;
    ty = target_is_you ? u.uy : mdef->my;
    dist = max(abs(mx - tx), abs(my - ty));

    /* Must be within TK range */
    if (dist > range || dist < 1)
        return FALSE;

    dx = sgn(tx - mx);
    dy = sgn(ty - my);

    /* Priority 1: Push an object at the target */
    push_obj = (struct obj *) 0;
    x = mx;
    y = my;
    for (i = 0; i < dist; i++) {
        struct obj *otmp;

        x += dx;
        y += dy;
        if (!isok(x, y))
            break;
        if (IS_STWALL(levl[x][y].typ) || closed_door(x, y))
            break;
        /* Don't scan past the target */
        if (x == tx && y == ty)
            break;
        if (OBJ_AT(x, y)) {
            for (otmp = level.objects[x][y]; otmp;
                 otmp = otmp->nexthere) {
                if (otmp->otyp == BOULDER && is_master) {
                    push_obj = otmp;
                    break;
                }
                if (otmp->otyp == STATUE) {
                    push_obj = otmp;
                    break;
                }
                if (otmp->oclass == WEAPON_CLASS) {
                    push_obj = otmp;
                    break;
                }
                if (otmp->otyp == CORPSE
                    && safe_touch_petrifies(otmp->corpsenm)) {
                    push_obj = otmp;
                    break;
                }
                if (otmp->otyp == EGG
                    && safe_touch_petrifies(otmp->corpsenm)) {
                    push_obj = otmp;
                    break;
                }
                if (otmp->otyp == CREAM_PIE) {
                    push_obj = otmp;
                    break;
                }
                if (otmp->oclass == POTION_CLASS
                    && (otmp->otyp == POT_PARALYSIS
                        || otmp->otyp == POT_BLINDNESS
                        || otmp->otyp == POT_CONFUSION
                        || otmp->otyp == POT_SLEEPING
                        || otmp->otyp == POT_ACID
                        || otmp->otyp == POT_HALLUCINATION
                        || otmp->otyp == POT_POLYMORPH
                        || otmp->otyp == POT_OIL)) {
                    push_obj = otmp;
                    break;
                }
            }
            if (push_obj)
                break;
        }
    }

    if (push_obj) {
        m_tk_push_obj(mtmp, push_obj, dx, dy, range);
        mtmp->mspec_used = 6 + rn2(6);
        return TRUE;
    }

    /* Priority 2: Pull the target towards the monster */
    if (target_is_you) {
        if ((int) youmonst.data->msize <= max_msize && dist > 1) {
            int pull_range = dist - 1;
            int pdx = sgn(mx - u.ux);
            int pdy = sgn(my - u.uy);

            pline("%s telekinetically pulls you towards %s!",
                  Monnam(mtmp), mhim(mtmp));

            /* Chain constraint for punished player */
            if (Punished && uball && !carried(uball)) {
                int bx = uball->ox, by = uball->oy;

                while (pull_range > 0
                       && dist2(u.ux + pdx * pull_range,
                                u.uy + pdy * pull_range,
                                bx, by) > 8)
                    pull_range--;
                if (pull_range <= 0) {
                    pline_The("chain goes taut!");
                    mtmp->mspec_used = 6 + rn2(6);
                    return TRUE;
                }
            }
            hurtle(pdx, pdy, pull_range, FALSE);
            mtmp->mspec_used = 6 + rn2(6);
            return TRUE;
        }
    } else {
        if ((int) mdef->data->msize <= max_msize && dist > 1) {
            int pull_range = dist - 1;

            if (canseemon(mtmp) || canseemon(mdef))
                pline("%s telekinetically pulls %s towards %s!",
                      Monnam(mtmp), mon_nam(mdef),
                      mhim(mtmp));
            mhurtle(mdef, -dx, -dy, pull_range);
            if (!DEADMONSTER(mdef))
                tk_impact(mdef, -dx, -dy);
            mtmp->mspec_used = 6 + rn2(6);
            return TRUE;
        }
    }

    return FALSE;
}

/*telekinesis.c*/
