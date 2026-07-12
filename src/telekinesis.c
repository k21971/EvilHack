/* NetHack 5.0	telekinesis.c	 */
/* Copyright (c) 2026 by Keith Simpson.  NetHack may be freely redistributed. */
/* See license for details. */

/* Hero-side telekinesis (EvilHack). The monster-side AI
   (m_dotelekinesis and its m_tk_push_obj/mfind_target target-finder
   cluster) ports with the Phase 2 monster-AI subsystem, together
   with the mtk_attk cooldown ticker and the #tk command wiring */

#include "hack.h"

staticfn int tk_max_msize(int);
staticfn void tk_push_obj(struct obj *, int, int, int);
staticfn void tk_pull_obj(struct obj *, int, int);
staticfn boolean tk_impact(struct monst *, int, int, struct monst *);
staticfn void tk_push_mon(struct monst *, int, int, int);
staticfn void tk_pull_mon(struct monst *, int, int);

/*
 * Utility functions
 */

/* Maximum monster size that can be affected at a given skill level.
   Returns -1 if no monsters can be affected (Unskilled) */
staticfn int
tk_max_msize(int skill)
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
can_use_telekinesis(boolean verbose)
{
    /* TODO-EVILHACK: racial_illithid(&youmonst) check returns with
       the illithid player race and the ERAC racial overlay */
    if (is_mind_flayer(gy.youmonst.data))
        return TRUE;
    if (Telekinesis)
        return TRUE;
    if (verbose)
        You("lack telekinetic ability.");
    return FALSE;
}

/* Effective telekinesis skill level for the player */
int
tk_skill(void)
{
    int skill;

    /* TODO-EVILHACK: illithid-race heroes bypass this and train
       P_TELEKINESIS (the P_SKILL fall-through below); until the race
       ports, only the extrinsic and polymorph paths apply */
    if (Telekinesis || is_mind_flayer(gy.youmonst.data))
        return P_SKILLED;
    skill = P_SKILL(P_TELEKINESIS);
    if (skill < P_UNSKILLED)
        skill = P_UNSKILLED;
    return skill;
}

/* Telekinesis range based on skill + helm bonus */
int
tk_range(void)
{
    static const int ranges[] = { 0, 1, 2, 3, 4, 5 };
    int skill = tk_skill();
    int range;

    if (skill < P_UNSKILLED || skill > P_MASTER)
        return 0;
    range = ranges[skill];
    /* TODO-EVILHACK: master restricts the helm bonus to the
       illithid race; re-add the race conjunct when it ports */
    if (uarmh && uarmh->otyp == HELM_OF_TELEKINESIS)
        range += 1;
    return range;
}

/* Power cost based on skill + helm reduction */
int
tk_pw_cost(void)
{
    static const int costs[] = { 0, 2, 5, 10, 15, 20 };
    int skill = tk_skill();
    int cost;

    if (skill < P_UNSKILLED || skill > P_MASTER)
        return 0;
    cost = costs[skill];
    /* TODO-EVILHACK: illithid-race restriction as in tk_range() */
    if (uarmh && uarmh->otyp == HELM_OF_TELEKINESIS)
        cost /= 2;
    if (cost < 1)
        cost = 1;
    return cost;
}

/*
 * Player push/pull helpers
 */

/* Push an object away from the player along (dx,dy) */
staticfn void
tk_push_obj(struct obj *obj, int dx, int dy, int range)
{
    int ox, oy, nx, ny, i;
    coordxy start_x, start_y;
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
    /* a blind hero feels the ball; drop the felt glyph at its old
       position so that tile clears (mirrors unplacebc_core) */
    if (obj == uball && Blind && (u.bc_felt & BC_BALL)) {
        levl[ox][oy].glyph = u.bglyph;
        u.bc_felt &= ~BC_BALL;
    }
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
        /* Solid terrain; TODO-EVILHACK: dead trees when the terrain
           extensions port */
        if (IS_STWALL(levl[nx][ny].typ) || IS_TREE(levl[nx][ny].typ)
            || closed_door(nx, ny)
            || levl[nx][ny].typ == IRONBARS) {
            pline_The("%s slams against the %s!", xname(obj),
                      IS_TREE(levl[nx][ny].typ) ? "tree"
                      : levl[nx][ny].typ == IRONBARS ? "iron bars"
                                                     : "wall");
            stopped = TRUE;
            break;
        }
        /* Monster in the way */
        mtmp = m_at(nx, ny);
        if (mtmp) {
            if (weight >= 20 || obj->oclass == WEAPON_CLASS
                || (((obj->otyp == CORPSE || obj->otyp == EGG)
                     && ismnum(obj->corpsenm)
                     && touch_petrifies(&mons[obj->corpsenm])))) {
                dmg = d(max(1, weight / 50), 4);
                pline_The("%s strikes %s!", xname(obj),
                          mon_nam(mtmp));
                if ((obj->otyp == CORPSE || obj->otyp == EGG)
                    && ismnum(obj->corpsenm)
                    && touch_petrifies(&mons[obj->corpsenm])
                    && !resists_ston(mtmp)) {
                    minstapetrify(mtmp, !svc.context.mon_moving);
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

    /* The ball is chained to the hero; moving it leaves the chain
       behind. Slide the chain one step from the hero toward the ball
       so ball, chain, and hero stay connected (chain adjacent to the
       hero, ball adjacent to the chain). dx,dy is the push direction,
       so (u.ux+dx,u.uy+dy) is one tile from the hero toward the ball --
       always open ground and within one tile of both */
    if (obj == uball)
        move_chain_to(u.ux + dx, u.uy + dy);

    /* Sokoban guilt for boulders only */
    if (obj->otyp == BOULDER && In_sokoban(&u.uz))
        sokoban_guilt();

    /* Shop theft */
    if (was_in_shop && !costly_spot(ox, oy))
        remote_burglary(start_x, start_y);
}

/* Pull an object towards the player's inventory, or slide a boulder */
staticfn void
tk_pull_obj(struct obj *obj, int dx, int dy)
{
    int steps;

    if (obj->otyp == BOULDER) {
        /* Boulder: slide towards player, stop 1 tile in front.
           relies on dotelekinesis() ensuring obj is on a straight
           line from u to target */
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

    /* Use existing remote pickup mechanism */
    (void) pickup_object(obj, 1L, TRUE);
}

/* Check if a monster slammed into an obstacle after hurtling
   and apply impact damage + feedback. hdx/hdy is the direction
   the monster was moving. magr is the caster (NULL = player cast,
   so player gets the kill). Returns TRUE if monster died */
staticfn boolean
tk_impact(struct monst *mtmp, int hdx, int hdy, struct monst *magr)
{
    int nx, ny, dmg;
    const char *what;
    boolean by_you = (magr == (struct monst *) 0);

    /* a monster that fell through a trap door or hole during the hurtle
       has left the level (migrate_to_level forces mx,my to 0); don't apply
       impact to a monster that is no longer here */
    if (DEADMONSTER(mtmp) || (mtmp->mstate & MON_MIGRATING))
        return TRUE;

    /* Check the next tile in the movement direction */
    nx = mtmp->mx + hdx;
    ny = mtmp->my + hdy;

    if (!isok(nx, ny)) {
        what = "wall";
    } else if (IS_STWALL(levl[nx][ny].typ)) {
        what = "wall";
    } else if (IS_TREE(levl[nx][ny].typ)) {
        /* TODO-EVILHACK: distinguish dead trees when they port */
        what = "tree";
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
    if (damage_mon(mtmp, dmg, AD_PHYS, by_you)) {
        if (by_you) {
            if (canseemon(mtmp))
                pline("%s is killed!", Monnam(mtmp));
            xkilled(mtmp, XKILL_NOMSG);
        } else {
            /* TODO-EVILHACK: a tame magr should get kill credit via
               set_pet_killer when the kill-attribution subsystem
               ports */
            monkilled(mtmp, "telekinetic impact", AD_PHYS);
        }
        return TRUE;
    }
    return FALSE;
}

/* Push a monster away using mhurtle */
staticfn void
tk_push_mon(struct monst *mtmp, int dx, int dy, int range)
{
    You("telekinetically hurl %s!", mon_nam(mtmp));
    mhurtle(mtmp, dx, dy, range);
    if (!DEADMONSTER(mtmp) && mtmp->mleashed
        && um_dist(mtmp->mx, mtmp->my, 5))
        m_unleash(mtmp, TRUE);
    (void) tk_impact(mtmp, dx, dy, (struct monst *) 0);
}

/* Pull a monster towards the player, stopping 1 tile away */
staticfn void
tk_pull_mon(struct monst *mtmp, int dx, int dy)
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
    (void) tk_impact(mtmp, -dx, -dy, (struct monst *) 0);
}

/*
 * Player command entry point
 */
int
dotelekinesis(void)
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
        int n, clr = NO_COLOR;

        win = create_nhwindow(NHW_MENU);
        start_menu(win, MENU_BEHAVE_STANDARD);

        any = cg.zeroany;
        any.a_int = 1;
        add_menu(win, &nul_glyphinfo, &any, 'a', 0, ATR_NONE, clr,
                 "Push something away", MENU_ITEMFLAGS_NONE);
        any.a_int = 2;
        add_menu(win, &nul_glyphinfo, &any, 'b', 0, ATR_NONE, clr,
                 "Pull something closer", MENU_ITEMFLAGS_NONE);

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

            for (otmp = svl.level.objects[cc.x][cc.y]; otmp;
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
        if (!isok(px, py))
            break;
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

        for (otmp = svl.level.objects[cc.x][cc.y]; otmp;
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
    disp.botl = TRUE;
    /* TODO-EVILHACK: illithid-race heroes train the skill here:
       use_skill(P_TELEKINESIS, 1) once the race ports */

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

/*telekinesis.c*/
