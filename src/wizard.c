/* NetHack 5.0	wizard.c	$NHDT-Date: 1781973074 2026/06/20 16:31:14 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.121 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2016. */
/* NetHack may be freely redistributed.  See license for details. */

/* wizard code - inspired by rogue code from Merlyn Leroy (digi-g!brian) */
/*             - heavily modified to give the wiz balls.  (genat!mike)   */
/*             - dewimped and given some maledictions. -3. */
/*             - generalized for 3.1 (mike@bullns.on01.bull.ca) */

#include "hack.h"

staticfn short which_arti(int);
staticfn boolean mon_has_arti(struct monst *, short) NONNULLARG1;
/* other_mon_has_arti() won't blow up if passed a NULL monst,
 * but its caller target_on() passes it a nonnull monst;
 * it may return a NULL monst pointer */
staticfn struct monst *other_mon_has_arti(struct monst *, short) NONNULLARG1;
staticfn struct obj *on_ground(short);  /* might return NULL obj pointer */
staticfn boolean you_have(int);
staticfn unsigned long target_on(int, struct monst *) NONNULLARG2;
staticfn unsigned long strategy(struct monst *) NONNULLARG1;
staticfn void covetous_approach(struct monst *) NONNULLARG1;

/* adding more neutral creatures will tend to reduce the number of monsters
   summoned by nasty(); adding more lawful creatures will reduce the number
   of monsters summoned by lawfuls; adding more chaotic creatures will reduce
   the number of monsters summoned by chaotics; prior to 3.6.1, there were
   only four lawful candidates, so lawful summoners tended to summon more
   (trying to get lawful or neutral but obtaining chaotic instead) than
   their chaotic counterparts */
static NEARDATA const int nasties[] = {
    /* neutral */
    PM_COCKATRICE, PM_ETTIN, PM_STALKER, PM_MINOTAUR,
    PM_OWLBEAR, PM_PURPLE_WORM, PM_XAN, PM_UMBER_HULK,
    PM_XORN, PM_ZRUTY, PM_LEOCROTTA, PM_DIRE_WOLVERINE,
    PM_GIANT_CENTIPEDE, PM_FIRE_ELEMENTAL, PM_JABBERWOCK,
    PM_IRON_GOLEM, PM_OCHRE_JELLY, PM_GREEN_SLIME,
    PM_GELATINOUS_CUBE, PM_SEA_DRAGON, PM_GENETIC_ENGINEER,
    /* chaotic */
    PM_BLACK_DRAGON, PM_SHADOW_DRAGON, PM_DEMILICH, PM_VAMPIRE_ROYAL,
    PM_MASTER_MIND_FLAYER, PM_DISENCHANTER, PM_WINGED_GARGOYLE,
    PM_STORM_GIANT, PM_OLOG_HAI, PM_MAGICAL_EYE, PM_ELVEN_ROYAL,
    PM_OGRE_ROYAL, PM_CAPTAIN, PM_GREMLIN, PM_HILL_GIANT_SHAMAN,
    PM_DISPLACER_BEAST,
    /* lawful */
    PM_SILVER_DRAGON, PM_GOLD_DRAGON, PM_STONE_GIANT,
    PM_SHIMMERING_DRAGON, PM_GRAY_DRAGON, PM_DWARF_ROYAL,
    PM_GUARDIAN_NAGA, PM_FIRE_GIANT, PM_ALEAX, PM_ANGEL,
    PM_COUATL, PM_TORTLE_SHAMAN, PM_BLACK_NAGA,
    /* (Archons, titans, ki-rin, and golden nagas are suitably nasty, but
       they're summoners so would aggravate excessive summoning) */
};

/* the Ice Queen summons winter-themed reinforcements */
static NEARDATA const int ice_nasties[] = {
    PM_WHITE_DRAGON, PM_SILVER_DRAGON, PM_DIRE_WOLVERINE,
    PM_MASTODON, PM_WOOLLY_MAMMOTH, PM_WINTER_WOLF,
    PM_CAVE_BEAR, PM_WARG, PM_ICE_TROLL, PM_FROST_GIANT,
    PM_WEREWOLF, PM_YETI, PM_SASQUATCH, PM_ICE_VORTEX,
    PM_GELATINOUS_CUBE, PM_OWLBEAR, PM_GOBLIN_CAPTAIN,
    PM_SABER_TOOTHED_TIGER, PM_FROST_SALAMANDER,
    PM_ICE_NYMPH,
};

/* Vecna summons the undead and the demonic */
static NEARDATA const int vecna_nasties[] = {
    PM_HUMAN_ZOMBIE, PM_REVENANT, PM_LICH, PM_DEMILICH,
    PM_VAMPIRE_NOBLE, PM_SHADE, PM_SPECTRE, PM_HEZROU,
    PM_MARILITH, PM_WEREWOLF, PM_HELL_HOUND, PM_VROCK,
    PM_GIANT_ZOMBIE, PM_HUMAN_MUMMY, PM_ETTIN_MUMMY,
    PM_GHOUL, PM_WRAITH, PM_BARROW_WIGHT,
};

static NEARDATA const unsigned wizapp[] = {
    PM_HUMAN,      PM_LAVA_DEMON,        PM_VAMPIRE_ROYAL, PM_RED_DRAGON,
    PM_ROCK_TROLL, PM_UMBER_HULK,        PM_XORN,          PM_GIANT_CENTIPEDE,
    PM_BASILISK,   PM_VORPAL_JABBERWOCK, PM_GUARDIAN_NAGA, PM_TRAPPER,
};

/* If you've found the Amulet, make the Wizard appear after some time */
/* Also, give hints about portal locations, if amulet is worn/wielded -dlc */
void
amulet(void)
{
    struct monst *mtmp;
    struct trap *ttmp;
    struct obj *amu;

#if 0 /* caller takes care of this check */
    if (!u.uhave.amulet)
        return;
#endif
    if ((((amu = uamul) != 0 && amu->otyp == AMULET_OF_YENDOR)
         || ((amu = uwep) != 0
             && (amu->otyp == AMULET_OF_YENDOR
                 || (amu->oartifact == ART_IDOL_OF_MOLOCH
                     && u.uevent.uimbued_idol))))
        && !rn2(15)) {
        for (ttmp = gf.ftrap; ttmp; ttmp = ttmp->ntrap) {
            if (ttmp->ttyp == MAGIC_PORTAL) {
                int du = distu(ttmp->tx, ttmp->ty);
                if (du <= 9)
                    pline("%s hot!", Tobjnam(amu, "feel"));
                else if (du <= 64)
                    pline("%s very warm.", Tobjnam(amu, "feel"));
                else if (du <= 144)
                    pline("%s warm.", Tobjnam(amu, "feel"));
                /* else, the amulet feels normal */
                break;
            }
        }
    }

    if (!svc.context.no_of_wizards)
        return;
    /* find Wizard, and wake him if necessary */
    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp))
            continue;
        if (mtmp->iswiz && mtmp->msleeping && !rn2(40)) {
            mtmp->msleeping = 0;
            if (!m_next2u(mtmp))
                You(
      "get the creepy feeling that somebody noticed your taking the Amulet.");
            return;
        }
    }
}

int
mon_has_amulet(struct monst *mtmp)
{
    struct obj *otmp;

    for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj)
        if (otmp->otyp == AMULET_OF_YENDOR
            || (otmp->oartifact == ART_IDOL_OF_MOLOCH
                && u.uevent.uimbued_idol))
            return 1;
    return 0;
}

int
mon_has_special(struct monst *mtmp)
{
    struct obj *otmp;

    for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj)
        if (otmp->otyp == AMULET_OF_YENDOR
            || any_quest_artifact(otmp)
            || otmp->otyp == BELL_OF_OPENING
            || otmp->otyp == CANDELABRUM_OF_INVOCATION
            || otmp->otyp == SPE_BOOK_OF_THE_DEAD)
            return 1;
    return 0;
}

/*
 *      New for 3.1  Strategy / Tactics for the wiz, as well as other
 *      monsters that are "after" something (defined via mflag3).
 *
 *      The strategy section decides *what* the monster is going
 *      to attempt, the tactics section implements the decision.
 */

#define M_Wants(mask) (mtmp->data->mflags3 & (mask))

staticfn short
which_arti(int mask)
{
    switch (mask) {
    case M3_WANTSAMUL:
        return AMULET_OF_YENDOR;
    case M3_WANTSBELL:
        return BELL_OF_OPENING;
    case M3_WANTSCAND:
        return CANDELABRUM_OF_INVOCATION;
    case M3_WANTSBOOK:
        return SPE_BOOK_OF_THE_DEAD;
    default:
        break; /* 0 signifies quest artifact */
    }
    return 0;
}

/*
 *      If "otyp" is zero, it triggers a check for the quest_artifact,
 *      since bell, book, candle, and amulet are all objects, not really
 *      artifacts right now.  [MRS]
 */
staticfn boolean
mon_has_arti(struct monst *mtmp, short otyp)
{
    struct obj *otmp;

    for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj) {
        if (otyp) {
            if (otmp->otyp == otyp)
                return 1;
        } else if (any_quest_artifact(otmp))
            return 1;
    }
    return 0;
}

/*
 * Returns some monster other than mtmp that
 * has artifact, or NULL monst pointer.
 */
staticfn struct monst *
other_mon_has_arti(struct monst *mtmp, short otyp)
{
    struct monst *mtmp2;

    for (mtmp2 = fmon; mtmp2; mtmp2 = mtmp2->nmon)
        /* no need for !DEADMONSTER check here since they have no inventory */
        if (mtmp2 != mtmp)
            if (mon_has_arti(mtmp2, otyp))
                return mtmp2;

    return (struct monst *) 0;
}

/*
 * Returns obj of type specified if there is one
 * on the ground, otherwise returns NULL obj pointer.
 */
staticfn struct obj *
on_ground(short otyp)
{
    struct obj *otmp;

    for (otmp = fobj; otmp; otmp = otmp->nobj)
        if (otyp) {
            if (otmp->otyp == otyp)
                return otmp;
        } else if (any_quest_artifact(otmp))
            return otmp;
    return (struct obj *) 0;
}

staticfn boolean
you_have(int mask)
{
    switch (mask) {
    case M3_WANTSAMUL:
        return (boolean) u.uhave.amulet;
    case M3_WANTSBELL:
        return (boolean) u.uhave.bell;
    case M3_WANTSCAND:
        return (boolean) u.uhave.menorah;
    case M3_WANTSBOOK:
        return (boolean) u.uhave.book;
    case M3_WANTSARTI:
        return (boolean) u.uhave.questart;
    default:
        break;
    }
    return 0;
}

staticfn unsigned long
target_on(int mask, struct monst *mtmp)
{
    short otyp;
    struct obj *otmp;
    struct monst *mtmp2;

    if (!M_Wants(mask) && !is_mplayer(mtmp->data))
        return (unsigned long) STRAT_NONE;

    otyp = which_arti(mask);
    if (!mon_has_arti(mtmp, otyp)) {
        if (you_have(mask) && !mtmp->mpeaceful) {
            mtmp->mgoal.x = u.ux;
            mtmp->mgoal.y = u.uy;
            return (STRAT_PLAYER | mask);
        } else if ((otmp = on_ground(otyp))) {
            mtmp->mgoal.x = otmp->ox;
            mtmp->mgoal.y = otmp->oy;
            return (STRAT_GROUND | mask);
        } else if ((mtmp2 = other_mon_has_arti(mtmp, otyp)) != 0
                 /* when seeking the Amulet or a quest artifact, avoid
                    targeting the Wizard or temple priests (to protect
                    Moloch's high priest) */
                 && ((otyp != AMULET_OF_YENDOR && otyp != 0)
                     || (!mtmp2->iswiz && !inhistemple(mtmp2)))) {
            mtmp->mgoal.x = mtmp2->mx;
            mtmp->mgoal.y = mtmp2->my;
            return (STRAT_MONSTR | mask);
        }
    }

    /* Do we have the Amulet? Alrighty then...
       On Astral, a covetous monster carrying the Amulet of
       Yendor heads for its own aligned altar to sacrifice it */
    if (Is_astralevel(&u.uz) && mon_has_arti(mtmp, AMULET_OF_YENDOR)) {
        coordxy x, y, targetx = u.ux, targety = u.uy;
        aligntyp malign = sgn(mon_aligntyp(mtmp));

        /* scan level for monster's aligned altar - alignments are
           shuffled per game so we can't rely on fixed coordinates */
        for (x = 1; x < COLNO; x++) {
            for (y = 0; y < ROWNO; y++) {
                if (IS_ALTAR(levl[x][y].typ) && a_align(x, y) == malign) {
                    targetx = x;
                    targety = y;
                    goto found_altar;
                }
            }
        }
 found_altar:
        mtmp->mgoal.x = targetx;
        mtmp->mgoal.y = targety;
        return (unsigned long) mask;
    }

    mtmp->mgoal.x = mtmp->mgoal.y = 0;
    return (unsigned long) STRAT_NONE;
}

staticfn unsigned long
strategy(struct monst *mtmp)
{
    unsigned long strat, dstrat;

    if (!is_covetous(mtmp->data)
        /* perhaps a shopkeeper has been polymorphed into a master
           lich; we don't want it teleporting to the stairs to heal
           because that will leave its shop untended */
        || (mtmp->isshk && inhishop(mtmp))
        /* likewise for temple priests */
        || (mtmp->ispriest && inhistemple(mtmp))
        /* Lucifer in the sanctum */
        || (mtmp->islucifer && Is_sanctum(&u.uz)))
        return (unsigned long) STRAT_NONE;

    /* avoid divide-by-zero on degenerate mhpmax */
    if (mtmp->mhpmax <= 0)
        return (unsigned long) STRAT_NONE;

    switch ((mtmp->mhp * 3) / mtmp->mhpmax) { /* 0-3 */

    default:
    case 0: /* panic time - mtmp is almost snuffed */
        return (unsigned long) STRAT_HEAL;

    case 1: /* the wiz and Vecna are less cautious */
        if (!(mtmp->iswiz || mtmp->isvecna))
            return (unsigned long) STRAT_HEAL;
        FALLTHROUGH;
        /* FALLTHRU */
    case 2:
        dstrat = STRAT_HEAL;
        break;

    case 3:
        dstrat = STRAT_NONE;
        break;
    }

    /* Quest nemeses will always prioritize their treasure
     * (important for the Infidel quest) */
    if (mtmp->data->msound == MS_NEMESIS)
        if ((strat = target_on(M3_WANTSARTI, mtmp)) != STRAT_NONE)
            return strat;

    if (svc.context.made_amulet)
        if ((strat = target_on(M3_WANTSAMUL, mtmp)) != STRAT_NONE)
            return strat;

    if (u.uevent.invoked) { /* priorities change once gate opened */
        if ((strat = target_on(M3_WANTSARTI, mtmp)) != STRAT_NONE)
            return strat;
        if ((strat = target_on(M3_WANTSBOOK, mtmp)) != STRAT_NONE)
            return strat;
        if ((strat = target_on(M3_WANTSBELL, mtmp)) != STRAT_NONE)
            return strat;
        if ((strat = target_on(M3_WANTSCAND, mtmp)) != STRAT_NONE)
            return strat;
    } else {
        if ((strat = target_on(M3_WANTSBOOK, mtmp)) != STRAT_NONE)
            return strat;
        if ((strat = target_on(M3_WANTSBELL, mtmp)) != STRAT_NONE)
            return strat;
        if ((strat = target_on(M3_WANTSCAND, mtmp)) != STRAT_NONE)
            return strat;
        if ((strat = target_on(M3_WANTSARTI, mtmp)) != STRAT_NONE)
            return strat;
    }
    return dstrat;
}

/* pick a destination for a covetous monster to flee to so that it can
   heal or for guardians (Kops) to congregate at to block hero's progress */
void
choose_stairs(
    coordxy *sx, coordxy *sy, /* output; left as-is if no spot found */
    boolean dir) /* True: forward, False: backtrack (usually up) */
{
    stairway *stway;
    boolean stdir = builds_up(&u.uz) ? dir : !dir;

    /* look for stairs in direction 'stdir' (True: up, False: down) */
    stway = stairway_find_type_dir(FALSE, stdir);
    if (!stway) {
        /* no stairs; look for ladder it that direction */
        stway = stairway_find_type_dir(TRUE, stdir);
        if (!stway) {
            /* no ladder either; look for branch stairs or ladder in any
               direction */
            for (stway = gs.stairs; stway; stway = stway->next)
                if (stway->tolev.dnum != u.uz.dnum)
                    break;
            /* if no branch stairs/ladder, check for regular stairs in
               opposite direction, then for regular ladder if necessary */
            if (!stway) {
                stway = stairway_find_type_dir(FALSE, !stdir);
                if (!stway)
                    stway = stairway_find_type_dir(TRUE, !stdir);
            }
        }
        /* [note: 'stway' could still be Null if the only access to this
           level is via magic portal] */
    }

    if (stway)
        *sx = stway->sx, *sy = stway->sy;
}

/* a covetous monster closes in on the hero: it pounces when close
   enough and uses short jumps until it is; both are silent apart from
   the one-chance STRAT_APPEARMSG arrival message */
staticfn void
covetous_approach(struct monst *mtmp)
{
    coordxy stx, sty;
    int dx, dy;

    /* it has the hero engulfed */
    if (engulfing_u(mtmp))
        return;

    if (distu(mtmp->mx, mtmp->my) <= 25) {
        /* if we're close enough, pounce */
        boolean couldspot = canspotmon(mtmp);

        mnexto(mtmp, RLOC_NOMSG);
        if (!(mtmp->mstate & ~TERRAIN_FALLOUT_MASK)
            && (mtmp->mstrategy & STRAT_APPEARMSG)) {
            /* one chance only; but STRAT_APPEARMSG doubles as the bribe
               one-shot tracker, so a peaceful blackmail demon keeps it */
            if (!mtmp->mpeaceful || mtmp->data->msound != MS_BRIBE)
                mtmp->mstrategy &= ~STRAT_APPEARMSG;
            if (!couldspot && canspotmon(mtmp))
                pline_mon(mtmp, "%s suddenly %s!", Amonnam(mtmp),
                          !Blind ? "appears" : "arrives");
        }
        return;
    }

    /* figure out what direction the hero's in */
    dx = sgn(u.ux - mtmp->mx);
    dy = sgn(u.uy - mtmp->my);
    /* since we're not close enough, use short jumps to change that */
    stx = mtmp->mx + ((rn2(3) + 4) * dx);
    sty = mtmp->my + ((rn2(3) + 3) * dy);
    /* a fixed-length jump toward a hero at or near a map edge can
       overshoot the boundary; clamp into bounds so the monster keeps
       advancing instead of freezing (an off-map coord would also index
       m_at() out of bounds in mnearto) */
    stx = min(max(stx, 1), COLNO - 1);
    sty = min(max(sty, 0), ROWNO - 1);
    (void) mnearto(mtmp, stx, sty, TRUE, RLOC_NOMSG);
}

DISABLE_WARNING_UNREACHABLE_CODE

int
tactics(struct monst *mtmp)
{
    unsigned long strat = strategy(mtmp);
    coordxy sx = 0, sy = 0, mx, my;

    mtmp->mstrategy =
        (mtmp->mstrategy & (STRAT_WAITMASK | STRAT_APPEARMSG)) | strat;

    /* if monster is magically scared, don't 'flee' right next to the
       hero; an engulfer keeps the hero until STRAT_HEAL's expels()
       instead of carrying it away */
    if (mtmp->mflee && !engulfing_u(mtmp) && !covetous_noteleport(mtmp)) {
        coordxy nx, ny;
        int j;

        mtmp->mavenge = 1;
        for (j = 0; j < 400; j++) {
            nx = rnd(COLNO - 1);
            ny = rn2(ROWNO);
            if (rloc_pos_ok(nx, ny, mtmp)
                && distu(nx, ny) > (BOLT_LIM * BOLT_LIM)) {
                rloc_to(mtmp, nx, ny);
                /* rloc_to may fire mintrap and kill mtmp */
                if (DEADMONSTER(mtmp))
                    return 0;
                if (mtmp->mhp <= mtmp->mhpmax - 8)
                    healmon(mtmp, rnd(8), 0);
                return 1;
            }
        }
        /* we couldn't find someplace far enough away from the hero to
           run to, which should be very rare, but... and in that case,
           just fall through so we do something */
        pline("%s looks around nervously.", Monnam(mtmp));
    }

    /* once a covetous monster gets close enough, they will start
       to move normally 95% of the time */
    if (rn2(20) && distu(mtmp->mx, mtmp->my) <= 8
        /* unless engulfing the hero; it must reach the STRAT_HEAL
           expels() release instead of moving with the hero inside */
        && !engulfing_u(mtmp)) {
        int mv = m_move(mtmp, 0);

        if (mv == MMOVE_DIED)
            return 2;
        if (mv == MMOVE_DONE)
            return 3;
        /* MMOVE_NOMOVES: monster can still act this turn */
        return (mv == MMOVE_MOVED) ? 1 : 0;
    }

    switch (strat) {
    case STRAT_HEAL: /* hide and recover */
        mx = mtmp->mx, my = mtmp->my;

        if (u.uswallow && u.ustuck == mtmp) {
            expels(mtmp, mtmp->data, TRUE);
            /* expels() moved mtmp off the hero's spot, or off the
               level if there was no room */
            if (DEADMONSTER(mtmp)
                || (mtmp->mstate & ~TERRAIN_FALLOUT_MASK) != 0)
                return 0;
            mx = mtmp->mx, my = mtmp->my;
        }

        /* if wounded, hole up on or near the stairs (to block them) */
        choose_stairs(&sx, &sy, (mtmp->m_id % 2));
        mtmp->mavenge = 1; /* covetous monsters attack while fleeing */
        if (In_W_tower(mx, my, &u.uz)
            || (mtmp->iswiz && !sx && !mon_has_amulet(mtmp))) {
            if (!covetous_noteleport(mtmp) &&
                !rn2(3 + mtmp->mhp / 10)) {
                (void) rloc(mtmp, RLOC_MSG);
                /* rloc may fire mintrap and kill mtmp */
                if (DEADMONSTER(mtmp))
                    return 0;
                mx = mtmp->mx, my = mtmp->my; /* update cached location */
            }
        } else if (sx && (mx != sx || my != sy)) {
            /* silent, as is the approach it falls through to */
            if (!covetous_noteleport(mtmp) &&
                !mnearto(mtmp, sx, sy, TRUE, RLOC_NOMSG)) {
                /* couldn't move to the target spot for some reason,
                   so stay where we are (don't actually need rloc_to()
                   because mtmp is still on the map at <mx,my>... */
                rloc_to(mtmp, mx, my);
                return 0;
            }
            /* mnearto succeeded; its internal rloc_to may have killed
               mtmp via mintrap on the destination tile */
            if (DEADMONSTER(mtmp))
                return 0;
            mx = mtmp->mx, my = mtmp->my; /* update cached location */
        }
        /* if you're not around, cast healing spells */
        if (distu(mx, my) > (BOLT_LIM * BOLT_LIM))
            if (mtmp->mhp <= mtmp->mhpmax - 8) {
                healmon(mtmp, rnd(8), 0);
                return 1;
            }
        FALLTHROUGH;
        /*FALLTHRU*/

    case STRAT_NONE: /* harass */
        if (covetous_noteleport(mtmp))
            return 0;
        if (mtmp->mpeaceful && !mtmp->mtame
            && !(mtmp->mstrategy & STRAT_APPEARMSG)) {
            /* wander aimlessly */
            if (!rn2(5))
                (void) rloc(mtmp, RLOC_NOMSG);
        } else {
            covetous_approach(mtmp);
        }
        return 0;

    default: /* kill, maim, pillage! */
    {
        long where = (strat & STRAT_STRATMASK);
        coordxy tx = mtmp->mgoal.x, ty = mtmp->mgoal.y;
        int targ = (int) (strat & STRAT_GOAL);
        struct obj *otmp;

        if (!targ || !isok(tx, ty)) { /* simply wants you to close */
            return 0;
        }
        if (covetous_noteleport(mtmp) && !monnear(mtmp, tx, ty))
            return 0;
        if (u_at(tx, ty) || where == STRAT_PLAYER) {
            /* player is standing on it (or has it) */
            if (!covetous_noteleport(mtmp))
                covetous_approach(mtmp);
            return 0;
        }
        if (where == STRAT_GROUND) {
            if (!MON_AT(tx, ty) || (mtmp->mx == tx && mtmp->my == ty)) {
                /* teleport to it and pick it up */
                rloc_to(mtmp, tx, ty); /* clean old pos */
                /* rloc_to may fire mintrap and kill mtmp; bail before
                   picking up the target so it stays on the floor
                   instead of being discarded with the corpse */
                if (DEADMONSTER(mtmp))
                    return 0;

                if ((otmp = on_ground(which_arti(targ))) != 0
                    && !can_levitate(mtmp)) {
                    if (cansee(mtmp->mx, mtmp->my))
                        pline("%s picks up %s.", Monnam(mtmp),
                              distant_name(otmp, doname));
                    obj_extract_self(otmp);
                    (void) mpickobj(mtmp, otmp);
                    /* artifact might be armor, attempt to put it on;
                       a monster reading a spellbook defers that until
                       the read finishes */
                    if (has_emsp(mtmp) && EMSP(mtmp)->msp_reading != 0)
                        check_gear_next_turn(mtmp);
                    else
                        m_dowear(mtmp, FALSE);
                    return 1;
                } else
                    return 0;
            } else {
                /* a monster is standing on it - cause some trouble */
                if (!rn2(5) && !covetous_noteleport(mtmp))
                    mnexto(mtmp, RLOC_MSG);
                return 0;
            }
        } else { /* a monster has it - 'port beside it. */
            mx = mtmp->mx, my = mtmp->my;
            if (!covetous_noteleport(mtmp) &&
                !mnearto(mtmp, tx, ty, FALSE, RLOC_MSG))
                rloc_to(mtmp, mx, my); /* no room? stay put */
            return 0;
        }
    } /* default case */
    } /* switch */
    /*NOTREACHED*/
    return 0;
}

RESTORE_WARNINGS

/* are there any monsters mon could aggravate? */
boolean
has_aggravatables(struct monst *mon)
{
    struct monst *mtmp;
    boolean in_w_tower = In_W_tower(mon->mx, mon->my, &u.uz);

    if (in_w_tower != In_W_tower(u.ux, u.uy, &u.uz))
        return FALSE;

    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp))
            continue;
        if (in_w_tower != In_W_tower(mtmp->mx, mtmp->my, &u.uz))
            continue;
        if ((mtmp->mstrategy & STRAT_WAITFORU) != 0 || helpless(mtmp))
            return TRUE;
    }
    return FALSE;
}

void
aggravate(void)
{
    struct monst *mtmp;
    boolean in_w_tower = In_W_tower(u.ux, u.uy, &u.uz);

    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        /* Lucifer and the Wizard wake and act on their own terms */
        if (DEADMONSTER(mtmp) || mtmp->islucifer || mtmp->iswiz)
            continue;
        if (in_w_tower != In_W_tower(mtmp->mx, mtmp->my, &u.uz))
            continue;
        /* STRAT_APPEARMSG stays; it doubles as the bribe-offer tracker */
        mtmp->mstrategy &= ~STRAT_WAITFORU;
        mtmp->msleeping = 0;
        if (!mtmp->mcanmove && !rn2(5)) {
            mtmp->mfrozen = 0;
            /* monsters in the middle of turning to stone stay frozen */
            if (!mtmp->mstone || mtmp->mstone > 2)
                mtmp->mcanmove = 1;
        }
    }
}

/* "Double Trouble" spell cast by the Wizard; caller is responsible for
   only casting this when there is currently one wizard in existence;
   the clone can't use it unless/until its creator has been killed off */
void
clonewiz(void)
{
    struct monst *mtmp2;

    if ((mtmp2 = makemon(&mons[PM_WIZARD_OF_YENDOR], u.ux, u.uy, MM_NOWAIT))
        != 0) {
        mtmp2->msleeping = mtmp2->mtame = mtmp2->mpeaceful = 0;
        if (!u.uhave.amulet && rn2(2)) { /* give clone a fake */
            (void) add_to_minv(mtmp2,
                               mksobj(FAKE_AMULET_OF_YENDOR, TRUE, FALSE));
        }
        if (!Protection_from_shape_changers) {
            mtmp2->m_ap_type = M_AP_MONSTER;
            mtmp2->mappearance = ROLL_FROM(wizapp);
        }
        newsym(mtmp2->mx, mtmp2->my);
    }
}

/* also used by newcham() */
int
pick_nasty(
    int difcap) /* if non-zero, try to make difficulty be lower than this */
{
    int alt, res = ROLL_FROM(nasties);

    /* To do?  Possibly should filter for appropriate forms when
     * in the elemental planes or surrounded by water or lava.
     *
     * We want monsters represented by uppercase on rogue level,
     * but we don't try very hard.
     */
    if (Is_rogue_level(&u.uz)
        && !('A' <= monsym(&mons[res]) && monsym(&mons[res]) <= 'Z'))
        res = ROLL_FROM(nasties);

    /* if genocided or too difficult or out of place, try a substitute
       when a suitable one exists
           demilich -> lich,
           master mind flayer -> mind flayer,
       but the substitutes are likely to be genocided too */
    alt = res;
    if ((svm.mvitals[res].mvflags & G_GENOD) != 0
        || (difcap > 0 && mons[res].difficulty >= difcap)
         /* note: nasty() -> makemon() ignores G_HELL|G_NOHELL; no
            member of nasties[] with a hell flag has a lesser form, so
            for this table the out-of-place test finds no substitute
            and the pick stands */
        || (mons[res].geno & (Inhell ? G_NOHELL : G_HELL)) != 0)
        alt = big_to_little(res);
    if (alt != res && (svm.mvitals[alt].mvflags & G_GENOD) == 0) {
        const char *mnam = mons[alt].pmnames[NEUTRAL],
                   *lastspace = strrchr(mnam, ' ');

        /* only non-juveniles can become alternate choice */
        if (strncmp(mnam, "baby ", 5)
            && (!lastspace
                || (strcmp(lastspace, " hatchling")
                    && strcmp(lastspace, " pup")
                    && strcmp(lastspace, " cub"))))
            res = alt;
    }

    return res;
}

/* summon table for the Ice Queen; genocide of a selection is handled
   by the makemon() substitution in nasty() */
int
pick_nasty_ice(void)
{
    return ROLL_FROM(ice_nasties);
}

/* summon table for Vecna */
int
pick_nasty_vecna(void)
{
    return ROLL_FROM(vecna_nasties);
}

/* create some nasty monsters, aligned with the caster or neutral; chaotic
   and unaligned are treated as equivalent; if summoner is Null, this is
   for late-game harassment (after the Wizard has been killed at least once
   or the invocation ritual has been performed), in which case we treat
   'summoner' as neutral, since that will produce the greatest number of
   creatures on average (in 3.6.0 and earlier, Null was treated as chaotic);
   returns the number of monsters created */
int
nasty(struct monst *summoner, boolean centered_on_stairs)
{
    struct monst *mtmp;
    stairway *stway;
    coord bypos;
    int i, j, count, census, tmp, makeindex,
        s_cls, m_cls, difcap, trylimit, castalign;
    /* when a monster casts the "summon nasties" spell, it gives feedback;
       when random post-Wizard harassment casts that, we give feedback */
    unsigned mmflags = summoner ? MM_NOMSG : NO_MM_FLAGS;

#define MAXNASTIES 10 /* more than this can be created */

    /* some candidates may be created in groups, so simple count
       of non-null makemon() return is inadequate */
    census = monster_census(FALSE);

    if (!rn2(10) && Inhell) {
        /* this might summon a demon prince or lord */
        count = msummon((struct monst *) 0); /* summons like WoY */
    } else {
        count = 0;
        s_cls = summoner ? summoner->data->mlet : 0;
        difcap = summoner ? summoner->data->difficulty : 0; /* spellcasters */
        castalign = summoner ? sgn(mon_aligntyp(summoner)) : 0;
        tmp = (u.ulevel > 3) ? u.ulevel / 3 : 1;
        /* if we don't have a casting monster, nasties appear around hero,
           otherwise they'll appear around spot summoner thinks she's at;
           unless we're being called with the 'stairs' flag to block the
           adventurer's return with the Amulet */
        if (centered_on_stairs
            && (stway = stairway_find_type_dir(FALSE, TRUE)) != 0) {
            bypos.x = stway->sx;
            bypos.y = stway->sy;
            /* the stair square is away from the hero, so makemon's
               by-you relocation never applies; allow adjacent
               placement or the wave stops at the first summon
               (master passes MM_ADJACENTOK for every nasty) */
            mmflags |= MM_ADJACENTOK;
        } else {
            bypos.x = u.ux;
            bypos.y = u.uy;
        }
        for (i = rnd(tmp); i > 0 && count < MAXNASTIES; --i) {
            /* Of the 50 nasties[], on this game's monster data 21 are
             * neutral, 16 are chaotic and 13 are lawful; none is
             * unaligned.
             *
             * The inner loop breaks on a critter whose alignment is
             * neutral or matches the caster's sign: a neutral caster
             * (or the no-summoner harassment case) breaks on neutrals
             * only (21/50), a lawful caster on neutrals plus lawfuls
             * (34/50), and a chaotic caster on neutrals plus chaotics
             * (37/50), so chaotic casters stop soonest and summon the
             * fewest creatures on average. An unaligned summoner
             * counts as chaotic here (sgn of its A_NONE alignment).
             * The test reads the monster actually made, so a gnomish
             * captain (neutral) or an Angel made as another god's
             * minion can break the loop off its listed side.
             *
             * The outer loop potentially gives casters who stop early
             * a chance to even things up since others will hit
             * MAXNASTIES sooner, but its number of iterations is
             * randomized so it won't always do so.
             */
            for (j = 0; j < 20; j++) {
                /* Don't create more spellcasters of the monster's level or
                 * higher--avoids chain summoners filling up the level.
                 */
                trylimit = 10 + 1; /* 10 tries */
                do {
                    if (!--trylimit)
                        goto nextj; /* break this loop, continue outer one */
                    /* the Ice Queen and Vecna summon from their own
                       themed tables */
                    makeindex =
                        (summoner
                         && summoner->data == &mons[PM_KATHRYN_THE_ICE_QUEEN])
                          ? pick_nasty_ice()
                          : (summoner && summoner->data == &mons[PM_VECNA])
                            ? pick_nasty_vecna()
                            : pick_nasty(difcap);
                    m_cls = mons[makeindex].mlet;
                } while ((difcap > 0 && mons[makeindex].difficulty >= difcap
                          && attacktype(&mons[makeindex], AT_MAGC))
                         || (s_cls == S_DEMON && m_cls == S_ANGEL)
                         || (s_cls == S_ANGEL && m_cls == S_DEMON));
                /* do this after picking the monster to place */
                if (summoner && !enexto(&bypos, summoner->mux, summoner->muy,
                                        &mons[makeindex]))
                    continue;
                /* this honors genocide but overrides extinction; it ignores
                   inside-hell-only (G_HELL) & outside-hell-only (G_NOHELL) */
                if ((mtmp = makemon(&mons[makeindex], bypos.x, bypos.y,
                                    mmflags)) != 0) {
                    mtmp->msleeping = mtmp->mpeaceful = mtmp->mtame = 0;
                    set_malign(mtmp);
                    update_monsteed(mtmp); /* the steed follows its rider */
                    newsym(mtmp->mx, mtmp->my); /* clear peaceful glyph */
                } else {
                    /* random monster to substitute for geno'd selection;
                       unlike direct choice, not forced to be hostile [why?];
                       limit spellcasters to inhibit chain summoning */
                    if ((mtmp = makemon((struct permonst *) 0,
                                        bypos.x, bypos.y, mmflags)) != 0) {
                        m_cls = mtmp->data->mlet;
                        if ((difcap > 0 && mtmp->data->difficulty >= difcap
                             /* always capping for substitutes made wanton
                                genocide become too strong in the endgame */
                             && rn2(In_endgame(&u.uz) ? 3 : 7) /* usually */
                             && attacktype(mtmp->data, AT_MAGC))
                            || (s_cls == S_DEMON && m_cls == S_ANGEL)
                            || (s_cls == S_ANGEL && m_cls == S_DEMON))
                            mtmp = unmakemon(mtmp, NO_MM_FLAGS); /* Null */
                    }
                }

                if (mtmp) {
                    /* if creating an arch-lich or Archon, further directly
                       selected nasties will have to be less difficult, and
                       substitutes for geno victims will usually be less
                       (note: neither is in nasties[] but either could be
                       chosen as random replacement for a genocided
                       selection) */
                    if (mtmp->data == &mons[PM_ARCH_LICH]
                        || mtmp->data == &mons[PM_ARCHON]) {
                        tmp = min(mons[PM_ARCHON].difficulty, /* A:26 */
                                  mons[PM_ARCH_LICH].difficulty); /* L:29 */
                        if (!difcap || difcap > tmp)
                            difcap = tmp; /* rest must be lower difficulty */
                    }
                    /* delay first use of spell, breath, or
                       telekinesis attack */
                    mtmp->mspec_used = rnd(4);
                    mtmp->mbreath_attk = rnd(4);
                    mtmp->mtk_attk = rnd(4);

                    if (++count >= MAXNASTIES
                        || mon_aligntyp(mtmp) == 0
                        || sgn(mon_aligntyp(mtmp)) == castalign)
                        break;
                }
 nextj:
                ; /* empty; label must be followed by a statement */
            } /* for j */
        } /* for i */
    }

    if (count)
        count = monster_census(FALSE) - census;
    return count;
}

/* Let's resurrect the Wizard, for some unexpected fun. */
void
resurrect(void)
{
    struct monst *mtmp, **mmtmp;
    long elapsed;
    const char *verb;

    if (!svc.context.no_of_wizards) {
        /* make a new Wizard */
        verb = "kill";
        mtmp = makemon(&mons[PM_WIZARD_OF_YENDOR], u.ux, u.uy, MM_NOWAIT);
        /* affects experience; he's not coming back from a corpse
           but is subject to repeated killing like a revived corpse */
        if (mtmp)
            mtmp->mrevived = 1;
    } else {
        /* look for a migrating Wizard */
        verb = "elude";
        mmtmp = &gm.migrating_mons;
        while ((mtmp = *mmtmp) != 0) {
            if (mtmp->iswiz
                /* if he has the Amulet, he won't bring it to you */
                && !mon_has_amulet(mtmp)
                && (elapsed = svm.moves - mtmp->mlstmv) > 0L) {
                mon_catchup_elapsed_time(mtmp, elapsed);
                if (elapsed >= LARGEST_INT)
                    elapsed = LARGEST_INT - 1;
                elapsed /= 50L;
                if (mtmp->msleeping && rn2((int) elapsed + 1))
                    mtmp->msleeping = 0;
                if (mtmp->mfrozen == 1) /* would unfreeze on next move */
                    mtmp->mfrozen = 0, mtmp->mcanmove = 1;
                if (!helpless(mtmp)) {
                    *mmtmp = mtmp->nmon;
                    mon_arrive(mtmp, -1); /* -1: Wiz_arrive (dog.c) */
                    /* mx: mon_arrive() might have sent mtmp into limbo */
                    if (!mtmp->mx)
                        mtmp = 0;
                    /* note: there might be a second Wizard; if so,
                       he'll have to wait until the next resurrection */
                    break;
                }
            }
            mmtmp = &mtmp->nmon;
        }
    }

    /* the makemon() and mon_arrive() calls above can both fire mintrap
       on the arrival square; a Wizard killed there dies quietly */
    if (mtmp && !DEADMONSTER(mtmp)) {
        /* clear STRAT_WAITMASK so that he comes after the hero instead
           of sitting wherever he is, "meditating", contradicting the
           threatening message below */
        mtmp->mstrategy &= ~STRAT_WAITMASK;

        mtmp->mtame = 0, mtmp->mpeaceful = 0; /* paranoia */
        set_malign(mtmp);
        if (!Deaf) {
            pline("A voice booms out...");
            SetVoice(mtmp, 0, 80, 0);
            verbalize("So thou thought thou couldst %s me, fool.", verb);
        }
    }
}

/* Here, we make trouble for the poor shmuck who actually
   managed to do in the Wizard. */
void
intervene(void)
{
    int which = Is_astralevel(&u.uz) ? rnd(4) : rn2(8);

    /* cases 0, and 5 through 7 don't apply on the Astral level */
    switch (which) {
    case 0:
        You_feel("apprehensive.");
        break;
    case 1:
        You_feel("vaguely nervous.");
        break;
    case 2:
        if (!Blind)
            You("notice %s glow surrounding you.", an(hcolor(NH_BLACK)));
        rndcurse();
        break;
    case 3:
        aggravate();
        break;
    case 4:
        (void) nasty((struct monst *) 0, FALSE);
        break;
    case 5:
        resurrect();
        break;
    case 6:
        if (u.uevent.invoked) {
            pline_The("entire dungeon starts shaking around you!");
            do_earthquake((MAXULEV - 1) / 3 + 1, FALSE);
            /* shake up monsters in a much larger radius */
            awaken_monsters(ROWNO * COLNO);
        }
        break;
    case 7:
        (void) nasty((struct monst *) 0, TRUE);
        break;
    }
}

/* Wizard of Yendor is being removed from play (dead or escaped the dungeon);
   keep the bookkeeping for him up to date */
void
wizdeadorgone(void)
{
    if (svc.context.no_of_wizards > 0)
        svc.context.no_of_wizards--;
    if (!u.uevent.udemigod) {
        u.uevent.udemigod = TRUE;
        u.udg_cnt = rn1(250, 50);
    }
}

static const char *const random_insult[] = {
    "antic",      "blackguard",   "caitiff",    "chucklehead",
    "coistrel",   "craven",       "cretin",     "cur",
    "dastard",    "demon fodder", "dimwit",     "dolt",
    "fool",       "footpad",      "imbecile",   "knave",
    "maledict",   "miscreant",    "niddering",  "poltroon",
    "rattlepate", "reprobate",    "scapegrace", "varlet",
    "villein", /* (sic.) */
    "wittol",     "worm",         "wretch",
};

static const char *const random_malediction[] = {
    "Hell shall soon claim thy remains,", "I chortle at thee, thou pathetic",
    "Prepare to die, thou", "Resistance is useless,",
    "Surrender or die, thou", "There shall be no mercy, thou",
    "Thou shalt repent of thy cunning,", "Thou art as a flea to me,",
    "Thou art doomed,", "Thy fate is sealed,",
    "Verily, thou shalt be one dead"
};

static const char *const random_icequeen[] = {
    "My magic is greater than yours", "You will never defeat me",
    "Winter shall last forever", "The cold never bothered me anyway",
    "Muahahahah", "I am even more powerful than the Wizard himself",
    "Run while you still can, fool", "Let's build a snowman",
    "The pegasus belongs to me"
};

static const char *const random_enchantress[] = {
    "Thank you again for freeing me", "I have so much damage to undo",
    "I apologize for any harm I may have caused you",
    "Be careful leaving this place, I have no control over the monsters that still lurk here",
    "Please treat the pegasus well, it has been through a lot",
    "It will take a long time to regrow the forest"
};

static const char *const random_vecna[] = {
    "I am Vecna the Unholy", "Kneel before me, wretched mortal",
    "You have no idea what true power is!  I will show you",
    "Your suffering will be legendary, even in hell",
    "Ah, the suffering.  The sweet, sweet suffering",
    "I will rip your soul to shreds"
};

static const char *const random_talgath[] = {
    "Puny creature!  Bow before my might",
    "Look into my eyes, if you dare",
    "Beauty truly is in the eye of the beholder"
};

static const char *const random_goblinking[] = {
    "Bones will be shattered, necks will be wrung!  You'll be beaten and battered, from racks you'll be hung",
    "You will die down here and never be found, down in the deep of Goblin Town",
    "With a swish and a smack, and a whip and a crack!  Everybody talks when they're on my rack",
    "Mine Town is under my control!  No one is allowed to visit",
    "So you want to explore Mines' End, eh?  Over my dead body",
};

static const char *const random_gollum[] = {
    "Precious, precious, precious!  My Precious!  O my Precious",
    "Losst it is, my precious, lost, lost!  Curse us and crush us, my precious is lost",
    "Never!  Smeagol wouldn't hurt a fly",
    "It isn't fair, my precious, is it, to ask us what it's got in it's nassty little pocketsess",
    "We wants it, we needs it.  Must have the precious.  They stole it from us",
    "And they doesn't taste very nice, does they, Precious",
};

/* Insult or intimidate the player */
void
cuss(struct monst *mtmp)
{
    if (Deaf)
        return;
    if (mtmp->iswiz) {
        if (!rn2(5)) { /* typical bad guy action */
            pline("%s laughs fiendishly.", Monnam(mtmp));
        } else if (u.uhave.amulet && !rn2(SIZE(random_insult))) {
            SetVoice(mtmp, 0, 80, 0);
            verbalize("Relinquish the amulet, %s!",
                      ROLL_FROM(random_insult));
        } else if ((Upolyd ? u.mh : u.uhp) < 5 && !rn2(2)) { /* Panic */
            SetVoice(mtmp, 0, 80, 0);
            verbalize(rn2(2) ? "Even now thy life force ebbs, %s!"
                             : "Savor thy breath, %s, it be thy last!",
                      ROLL_FROM(random_insult));
        } else if (mtmp->mhp < 5 && !rn2(2)) { /* Parthian shot */
            SetVoice(mtmp, 0, 80, 0);
            verbalize(rn2(2) ? "I shall return." : "I'll be back.");
        } else {
            SetVoice(mtmp, 0, 80, 0);
            verbalize("%s %s!",
                      ROLL_FROM(random_malediction),
                      ROLL_FROM(random_insult));
        }
    } else if (is_lminion(mtmp)
               && !(mtmp->isminion && EMIN(mtmp)->renegade)) {
        /* the last angel_cuss entry is only used while hallucinating */
        com_pager_subset("angel_cuss", Hallucination ? 0 : -1);
    } else if (mtmp->data == &mons[PM_KATHRYN_THE_ICE_QUEEN]) {
        if (!rn2(5)) {
            pline("%s points and giggles at you.", Monnam(mtmp));
            SetVoice(mtmp, 0, 80, 0);
            if (kathryn_bday())
                verbalize("It's my birthday!  Woohoo!!");
            else if (bourbon_bday())
                verbalize("Wish Bourbon a happy birthday!  Now!!");
            else if (ozzy_bday())
                verbalize("Say happy birthday to Ozzy!  Say it!!");
        } else {
            SetVoice(mtmp, 0, 80, 0);
            verbalize("%s!", ROLL_FROM(random_icequeen));
        }
    } else if (mtmp->data == &mons[PM_KATHRYN_THE_ENCHANTRESS]) {
        if (mtmp->mpeaceful) {
            if (!rn2(5)) {
                pline("%s waves to you.", Monnam(mtmp));
                SetVoice(mtmp, 0, 80, 0);
                if (kathryn_bday())
                    verbalize("You freed me on my birthday!  Thank you so much!!");
                else if (bourbon_bday())
                    verbalize("Happy birthday, Bourbon!  Good girl!!");
                else if (ozzy_bday())
                    verbalize("Happy birthday, Ozzy!  You're a good boy, yes you are!!");
            } else {
                SetVoice(mtmp, 0, 80, 0);
                verbalize("%s.", ROLL_FROM(random_enchantress));
            }
        } else {
            if (!rn2(7))
                pline("%s waves to you.", Monnam(mtmp));
        }
    } else if (mtmp->isvecna) {
        SetVoice(mtmp, 0, 80, 0);
        verbalize("%s!", ROLL_FROM(random_vecna));
    } else if (mtmp->istalgath) {
        SetVoice(mtmp, 0, 80, 0);
        verbalize("%s!", ROLL_FROM(random_talgath));
    } else if (mtmp->isgking) {
        if (!rn2(3)) {
            if (uwep && uwep->oartifact == ART_ORCRIST) {
                SetVoice(mtmp, 0, 80, 0);
                verbalize("I know that sword!  It is the Goblin Cleaver!  The Biter!  The blade that sliced a thousand necks!");
            } else if (uwep && uwep->oartifact == ART_GLAMDRING) {
                SetVoice(mtmp, 0, 80, 0);
                verbalize("%s wields the Foe-Hammer!  The Beater!  Bright as daylight!",
                          flags.female ? "She" : "He");
            }
        } else {
            SetVoice(mtmp, 0, 80, 0);
            verbalize("%s!", ROLL_FROM(random_goblinking));
        }
    } else if (mtmp->data == &mons[PM_GOLLUM]) {
        SetVoice(mtmp, 0, 80, 0);
        verbalize("%s!", ROLL_FROM(random_gollum));
    } else {
        if (!rn2(is_minion(mtmp->data) ? 100 : 5))
            pline("%s casts aspersions on your ancestry.", Monnam(mtmp));
        else
            com_pager("demon_cuss");
    }
    wake_nearto(mtmp->mx, mtmp->my, 5 * 5);
}

static const char *const random_mplayer_amulet[] = {
    "Give me the Amulet of Yendor",
    "Where is the Amulet?  I need it to escape this place",
    "This isn't Purgatory, it's Hell with a nice view",
    "Either you relinquish the Amulet now, or I will kill you"
};

static const char *const random_mplayer_idol[] = {
    "Give me the Idol of Moloch",
    "Where is the Idol?  I need it to escape this place",
    "This isn't Purgatory, it's Hell with a nice view",
    "Either you relinquish the Idol now, or I will kill you"
};

/* player monsters harass the player in Purgatory */
void
mplayer_purg_talk(struct monst *mtmp)
{
    if (Deaf)
        return;
    if (is_mplayer(mtmp->data)) {
        SetVoice(mtmp, 0, 80, 0);
        if (Role_if(PM_INFIDEL))
            verbalize("%s!", ROLL_FROM(random_mplayer_idol));
        else
            verbalize("%s!", ROLL_FROM(random_mplayer_amulet));
    }
}

static const char *const random_arch_amulet[] = {
    "Hast thou come to repent thy sins?",
    "Give unto me the Amulet, and I shall cleanse thee of thy sins.",
    "Only those that are pure of heart can ascend to glory."
};

static const char *const random_arch_idol[] = {
    "Begone, foul Infidel!  I cast thee back to hell!",
    "Return from whence thou cam'st!",
    "I shall relieve thee of thy charge, and destroy thine wicked Idol!"
};

/* Saint Michael the Archangel confronts the player in Purgatory */
void
archangel_purg_talk(struct monst *mtmp)
{
    if (Deaf)
        return;
    if (mtmp->data == &mons[PM_SAINT_MICHAEL]) {
        SetVoice(mtmp, 0, 80, 0);
        if (Role_if(PM_INFIDEL))
            verbalize("%s", ROLL_FROM(random_arch_idol));
        else
            verbalize("%s", ROLL_FROM(random_arch_amulet));
    }
}

/*wizard.c*/
