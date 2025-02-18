/* NetHack 3.6	wizard.c	$NHDT-Date: 1561336025 2019/06/24 00:27:05 $  $NHDT-Branch: NetHack-3.6 $:$NHDT-Revision: 1.56 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2016. */
/* NetHack may be freely redistributed.  See license for details. */

/* wizard code - inspired by rogue code from Merlyn Leroy (digi-g!brian) */
/*             - heavily modified to give the wiz balls.  (genat!mike)   */
/*             - dewimped and given some maledictions. -3. */
/*             - generalized for 3.1 (mike@bullns.on01.bull.ca) */

#include "hack.h"
#include "qtext.h"

STATIC_DCL short FDECL(which_arti, (int));
STATIC_DCL boolean FDECL(mon_has_arti, (struct monst *, SHORT_P));
STATIC_DCL struct monst *FDECL(other_mon_has_arti, (struct monst *, SHORT_P));
STATIC_DCL struct obj *FDECL(on_ground, (SHORT_P));
STATIC_DCL boolean FDECL(you_have, (int));
STATIC_DCL unsigned long FDECL(target_on, (int, struct monst *));
STATIC_DCL unsigned long FDECL(strategy, (struct monst *));

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
    PM_GELATINOUS_CUBE, PM_SEA_DRAGON,
    PM_DISPLACER_BEAST, PM_GENETIC_ENGINEER,
    /* chaotic */
    PM_BLACK_DRAGON, PM_SHADOW_DRAGON, PM_DEMILICH, PM_VAMPIRE_ROYAL,
    PM_MASTER_MIND_FLAYER, PM_DISENCHANTER, PM_WINGED_GARGOYLE,
    PM_STORM_GIANT, PM_OLOG_HAI, PM_MAGICAL_EYE, PM_ELVEN_ROYAL,
    PM_OGRE_ROYAL, PM_CAPTAIN, PM_GREMLIN, PM_HILL_GIANT_SHAMAN,
    /* lawful */
    PM_SILVER_DRAGON, PM_GOLD_DRAGON, PM_STONE_GIANT,
    PM_SHIMMERING_DRAGON, PM_GRAY_DRAGON, PM_DWARF_ROYAL,
    PM_GUARDIAN_NAGA, PM_FIRE_GIANT, PM_ALEAX, PM_ANGEL,
    PM_COUATL, PM_TORTLE_SHAMAN, PM_BLACK_NAGA,
    /* (Archons, titans, ki-rin, and golden nagas are suitably nasty, but
       they're summoners so would aggravate excessive summoning) */
};

static NEARDATA const int ice_nasties[] = {
    PM_WHITE_DRAGON, PM_SILVER_DRAGON, PM_DIRE_WOLVERINE,
    PM_MASTODON, PM_WOOLLY_MAMMOTH, PM_WINTER_WOLF,
    PM_CAVE_BEAR, PM_WARG, PM_ICE_TROLL, PM_FROST_GIANT,
    PM_WEREWOLF, PM_YETI, PM_SASQUATCH, PM_ICE_VORTEX,
    PM_GELATINOUS_CUBE, PM_OWLBEAR, PM_GOBLIN_CAPTAIN,
    PM_SABER_TOOTHED_TIGER, PM_FROST_SALAMANDER,
    PM_ICE_NYMPH,
};

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
amulet()
{
    struct monst *mtmp;
    struct trap *ttmp;
    struct obj *amu;

#if 0 /* caller takes care of this check */
    if (!u.uhave.amulet)
        return;
#endif
    if ((((amu = uamul) != 0 && amu->otyp == AMULET_OF_YENDOR)
         || ((amu = uwep) != 0 && (amu->otyp == AMULET_OF_YENDOR
                                   || (amu->oartifact == ART_IDOL_OF_MOLOCH
                                       && u.uachieve.amulet))))
        && !rn2(15)) {
        for (ttmp = ftrap; ttmp; ttmp = ttmp->ntrap) {
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

    if (!context.no_of_wizards)
        return;
    /* find Wizard, and wake him if necessary */
    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp))
            continue;
        if (mtmp->iswiz && mtmp->msleeping && !rn2(40)) {
            mtmp->msleeping = 0;
            if (distu(mtmp->mx, mtmp->my) > 2)
                You(
      "get the creepy feeling that somebody noticed your taking the Amulet.");
            return;
        }
    }
}

int
mon_has_amulet(mtmp)
register struct monst *mtmp;
{
    register struct obj *otmp;

    for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj)
        if (otmp->otyp == AMULET_OF_YENDOR
            || (otmp->oartifact == ART_IDOL_OF_MOLOCH
                && u.uachieve.amulet))
            return 1;
    return 0;
}

int
mon_has_special(mtmp)
register struct monst *mtmp;
{
    register struct obj *otmp;

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
#define STRAT(w, x, y, typ)                            \
    ((unsigned long) (w) | ((unsigned long) (x) << 16) \
     | ((unsigned long) (y) << 8) | (unsigned long) (typ))

#define M_Wants(mask) (mtmp->data->mflags3 & (mask))

STATIC_OVL short
which_arti(mask)
register int mask;
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
STATIC_OVL boolean
mon_has_arti(mtmp, otyp)
register struct monst *mtmp;
register short otyp;
{
    register struct obj *otmp;

    for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj) {
        if (otyp) {
            if (otmp->otyp == otyp)
                return 1;
        } else if (any_quest_artifact(otmp))
            return 1;
    }
    return 0;
}

STATIC_OVL struct monst *
other_mon_has_arti(mtmp, otyp)
register struct monst *mtmp;
register short otyp;
{
    register struct monst *mtmp2;

    for (mtmp2 = fmon; mtmp2; mtmp2 = mtmp2->nmon)
        /* no need for !DEADMONSTER check here since they have no inventory */
        if (mtmp2 != mtmp)
            if (mon_has_arti(mtmp2, otyp))
                return mtmp2;

    return (struct monst *) 0;
}

STATIC_OVL struct obj *
on_ground(otyp)
register short otyp;
{
    register struct obj *otmp;

    for (otmp = fobj; otmp; otmp = otmp->nobj)
        if (otyp) {
            if (otmp->otyp == otyp)
                return otmp;
        } else if (any_quest_artifact(otmp))
            return otmp;
    return (struct obj *) 0;
}

STATIC_OVL boolean
you_have(mask)
register int mask;
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

STATIC_OVL unsigned long
target_on(mask, mtmp)
register int mask;
register struct monst *mtmp;
{
    register short otyp;
    register struct obj *otmp;
    register struct monst *mtmp2;

    if (!M_Wants(mask) && !is_mplayer(mtmp->data))
        return (unsigned long) STRAT_NONE;

    otyp = which_arti(mask);
    if (!mon_has_arti(mtmp, otyp)) {
        if (you_have(mask) && !mtmp->mpeaceful)
            return STRAT(STRAT_PLAYER, u.ux, u.uy, mask);
        else if ((otmp = on_ground(otyp)))
            return STRAT(STRAT_GROUND, otmp->ox, otmp->oy, mask);
        else if ((mtmp2 = other_mon_has_arti(mtmp, otyp)) != 0
                 /* when seeking the Amulet or a quest artifact,
                    avoid targetting the Wizard or temple priests
                    (to protect Moloch's high priest) */
                 && !is_mplayer(mtmp2->data)
                 && ((otyp != AMULET_OF_YENDOR && otyp != 0 )
                     || (!mtmp2->iswiz && !inhistemple(mtmp2))))
            return STRAT(STRAT_MONSTR, mtmp2->mx, mtmp2->my, mask);
    }

    /* Do we have the Amulet? Alrighty then... */
    if (Is_astralevel(&u.uz)) {
	int targetx = u.ux, targety = u.uy;
	aligntyp malign = sgn(mon_aligntyp(mtmp));

	if (IS_ALTAR(levl[10][10].typ)
            && a_align(10, 10) == malign)
            targetx = 10, targety = 10;
	else if (IS_ALTAR(levl[40][6].typ)
                 && a_align(40, 6) == malign)
            targetx = 40, targety = 6;
	else if (IS_ALTAR(levl[70][10].typ)
                 && a_align(70, 10) == malign)
            targetx = 70, targety = 10;

	return STRAT(STRAT_NONE, targetx, targety, mask);
    }
    return (unsigned long) STRAT_NONE;
}

STATIC_OVL unsigned long
strategy(mtmp)
register struct monst *mtmp;
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

    switch ((mtmp->mhp * 3) / mtmp->mhpmax) { /* 0-3 */

    default:
    case 0: /* panic time - mtmp is almost snuffed */
        return (unsigned long) STRAT_HEAL;

    case 1: /* the wiz and vecna are less cautious */
        if (!(mtmp->iswiz || mtmp->isvecna))
            return (unsigned long) STRAT_HEAL;
    /* else fall through */

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

    if (context.made_amulet)
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

void
choose_stairs(sx, sy)
xchar *sx;
xchar *sy;
{
    xchar x = 0, y = 0;

    if (builds_up(&u.uz)) {
        if (xdnstair) {
            x = xdnstair;
            y = ydnstair;
        } else if (xdnladder) {
            x = xdnladder;
            y = ydnladder;
        }
    } else {
        if (xupstair) {
            x = xupstair;
            y = yupstair;
        } else if (xupladder) {
            x = xupladder;
            y = yupladder;
        }
    }

    if (!x && sstairs.sx) {
        x = sstairs.sx;
        y = sstairs.sy;
    }

    if (x && y) {
        *sx = x;
        *sy = y;
    }
}

int
tactics(mtmp)
register struct monst *mtmp;
{
    unsigned long strat = strategy(mtmp);
    xchar sx = 0, sy = 0, mx, my;
    schar nx, ny;
    int j;

    mtmp->mstrategy =
        (mtmp->mstrategy & (STRAT_WAITMASK | STRAT_APPEARMSG)) | strat;

    /* if monster is magically scared, don't 'flee' right next
     * to the player */
    if (mtmp->mflee) {
        mtmp->mavenge = 1;
        for (j = 0; j < 400; j++) {
            nx = rnd(COLNO - 1);
            ny = rn2(ROWNO);
            if (rloc_pos_ok(nx, ny, mtmp)
                && distu(nx, ny) > (BOLT_LIM * BOLT_LIM)) {
                rloc_to(mtmp, nx, ny);
                if (mtmp->mhp <= mtmp->mhpmax - 8)
                    mtmp->mhp += rnd(8);
                return 1;
            }
        }
        /* we couldn't find someplace far enough away from the player
         * to run to, which should be very rare, but...
         * and in that case, just fall through so we do something */
        pline("%s looks around nervously.", Monnam(mtmp));
    }

    /* once a covetous monster gets close enough, they will start
       to move normally 95% of the time */
    if (rn2(20) && distu(mtmp->mx, mtmp->my) <= 8)
        return m_move(mtmp, 0);

    switch (strat) {
    case STRAT_HEAL: /* hide and recover */
        mx = mtmp->mx, my = mtmp->my;
        /* if wounded, hole up on or near the stairs (to block them) */
        choose_stairs(&sx, &sy);
        mtmp->mavenge = 1; /* covetous monsters attack while fleeing */
        if (In_W_tower(mx, my, &u.uz)
            || (mtmp->iswiz && !sx && !mon_has_amulet(mtmp))) {
            if (!rn2(3 + mtmp->mhp / 10))
                (void) rloc(mtmp, TRUE);
        } else if (sx && (mx != sx || my != sy)) {
            if (!mnearto(mtmp, sx, sy, TRUE)) {
                /* couldn't move to the target spot for some reason,
                   so stay where we are (don't actually need rloc_to()
                   because mtmp is still on the map at <mx,my>... */
                rloc_to(mtmp, mx, my);
                return 0;
            }
            mx = mtmp->mx, my = mtmp->my; /* update cached location */
        }
        /* if you're not around, cast healing spells */
        if (distu(mx, my) > (BOLT_LIM * BOLT_LIM))
            if (mtmp->mhp <= mtmp->mhpmax - 8) {
                mtmp->mhp += rnd(8);
                return 1;
            }
        /*FALLTHRU*/

    case STRAT_NONE: /* harass */
    {
        xchar tx = STRAT_GOALX(strat), ty = STRAT_GOALY(strat),
                   dx = 0, dy = 0, stx = tx, sty = ty;

        if (mtmp->mpeaceful && !mtmp->mtame
            && !(mtmp->mstrategy & STRAT_APPEARMSG)) {
            /* wander aimlessly */
            if (!rn2(5))
                (void) rloc(mtmp, TRUE);
        } else if (distu(mtmp->mx, mtmp->my) <= 25) {
            mnexto(mtmp); /* If we're close enough, pounce */
        } else {
            /* figure out what direction the player's in */
            dx = sgn(u.ux - mtmp->mx);
            dy = sgn(u.uy - mtmp->my);
            /* since we're not close enough, use short jumps to change that */
            stx = mtmp->mx + ((rn2(3) + 4) * dx);
            sty = mtmp->my + ((rn2(3) + 3) * dy);
            if (isok(stx, sty))
                mnearto(mtmp, stx, sty, TRUE);
        }
        return 0;
    }

    default: /* kill, maim, pillage! */
    {
        long where = (strat & STRAT_STRATMASK);
        xchar tx = STRAT_GOALX(strat), ty = STRAT_GOALY(strat),
                   dx = 0, dy = 0, stx = tx, sty = ty;
        int targ = (int) (strat & STRAT_GOAL);
        struct obj *otmp;

        if (!targ) { /* simply wants you to close */
            return 0;
        }
        /* player is standing on it (or has it) */
        if ((u.ux == tx && u.uy == ty) || where == STRAT_PLAYER) {
            /* If we're close enough, pounce */
            if (distu(mtmp->mx, mtmp->my) <= 25) {
                mnexto(mtmp);
            } else {
                /* figure out what direction the player's in */
                dx = sgn(u.ux - mtmp->mx);
                dy = sgn(u.uy - mtmp->my);
                /* since we're not close enough, use short jumps to change that */
                stx = mtmp->mx + ((rn2(3) + 4) * dx);
                sty = mtmp->my + ((rn2(3) + 3) * dy);
                mnearto(mtmp, stx, sty, TRUE);
            }
            return 0;
        }
        if (where == STRAT_GROUND) {
            if (!MON_AT(tx, ty) || (mtmp->mx == tx && mtmp->my == ty)) {
                /* teleport to it and pick it up */
                rloc_to(mtmp, tx, ty); /* clean old pos */

                if ((otmp = on_ground(which_arti(targ))) != 0
                    && !can_levitate(mtmp)) {
                    if (cansee(mtmp->mx, mtmp->my))
                        pline("%s picks up %s.", Monnam(mtmp),
                              (distu(mtmp->mx, mtmp->my) <= 5)
                                  ? doname(otmp)
                                  : distant_name(otmp, doname));
                    obj_extract_self(otmp);
                    (void) mpickobj(mtmp, otmp);
                    /* artifact might be armor, attempt to put it on */
                    m_dowear(mtmp, FALSE);
                    return 1;
                } else
                    return 0;
            } else {
                /* a monster is standing on it - cause some trouble */
                if (!rn2(5))
                    mnexto(mtmp);
                return 0;
            }
        } else { /* a monster has it - 'port beside it. */
            mx = mtmp->mx, my = mtmp->my;
            if (!mnearto(mtmp, tx, ty, FALSE))
                rloc_to(mtmp, mx, my); /* no room? stay put */
            return 0;
        }
    } /* default case */
    } /* switch */
    /*NOTREACHED*/
    return 0;
}

/* are there any monsters mon could aggravate? */
boolean
has_aggravatables(mon)
struct monst *mon;
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
        if ((mtmp->mstrategy & STRAT_WAITFORU) != 0
            || mtmp->msleeping || !mtmp->mcanmove)
            return TRUE;
    }
    return FALSE;
}

void
aggravate()
{
    register struct monst *mtmp;
    boolean in_w_tower = In_W_tower(u.ux, u.uy, &u.uz);

    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp) || mtmp->islucifer
            || mtmp->iswiz)
            continue;
        if (in_w_tower != In_W_tower(mtmp->mx, mtmp->my, &u.uz))
            continue;
        mtmp->mstrategy &= ~STRAT_WAITFORU;
        mtmp->msleeping = 0;
        if (!mtmp->mcanmove && !rn2(5)) {
            mtmp->mfrozen = 0;
            if (!mtmp->mstone || mtmp->mstone > 2)
                mtmp->mcanmove = 1;
        }
    }
}

/* "Double Trouble" spell cast by the Wizard; caller is responsible for
   only casting this when there is currently one wizard in existence;
   the clone can't use it unless/until its creator has been killed off */
void
clonewiz()
{
    register struct monst *mtmp2;

    if ((mtmp2 = makemon(&mons[PM_WIZARD_OF_YENDOR], u.ux, u.uy, MM_NOWAIT))
        != 0) {
        mtmp2->msleeping = mtmp2->mtame = mtmp2->mpeaceful = 0;
        if (!u.uhave.amulet && rn2(2)) { /* give clone a fake */
            (void) add_to_minv(mtmp2,
                               mksobj(FAKE_AMULET_OF_YENDOR, TRUE, FALSE));
        }
        mtmp2->m_ap_type = M_AP_MONSTER;
        mtmp2->mappearance = wizapp[rn2(SIZE(wizapp))];
        newsym(mtmp2->mx, mtmp2->my);
    }
}

/* also used by newcham() */
int
pick_nasty()
{
    int res = nasties[rn2(SIZE(nasties))];

    /* To do?  Possibly should filter for appropriate forms when
     * in the elemental planes or surrounded by water or lava.
     *
     * We want monsters represented by uppercase on rogue level,
     * but we don't try very hard.
     */
    if (Is_rogue_level(&u.uz)
        && !('A' <= mons[res].mlet && mons[res].mlet <= 'Z'))
        res = nasties[rn2(SIZE(nasties))];

    return res;
}

int
pick_nasty_ice()
{
    int res = ice_nasties[rn2(SIZE(ice_nasties))];

    return res;
}

int
pick_nasty_vecna()
{
    int res = vecna_nasties[rn2(SIZE(vecna_nasties))];

    return res;
}

/* create some nasty monsters, aligned with the caster or neutral; chaotic
   and unaligned are treated as equivalent; if summoner is Null, this is
   for late-game harassment (after the Wizard has been killed at least once
   or the invocation ritual has been performed), in which case we treat
   'summoner' as neutral, since that will produce the greatest number of
   creatures on average (in 3.6.0 and earlier, Null was treated as chaotic);
   returns the number of monsters created */
int
nasty(summoner, centered_on_stairs)
struct monst *summoner;
BOOLEAN_P centered_on_stairs;
{
    register struct monst *mtmp;
    register int i, j, tmp;
    int castalign = (summoner ? sgn(mon_aligntyp(summoner)) : 0);
    coord bypos;
    int count, census, s_cls, m_cls;

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
        tmp = (u.ulevel > 3) ? u.ulevel / 3 : 1;
        /* if we don't have a casting monster, nasties appear around hero,
         * ...unless we're being called with the 'stairs' flag to block the
         * adventurer's return with the amulet */
        if (centered_on_stairs && xupstair) {
            bypos.x = xupstair;
            bypos.y = yupstair;
        } else {
            bypos.x = u.ux;
            bypos.y = u.uy;
        }
        for (i = rnd(tmp); i > 0 && count < MAXNASTIES; --i)
            /* Of the 54 nasties[], 13 are lawful, 19 are chaotic,
             * and 22 are neutral.
             *
             * Neutral caster, used for late-game harrassment,
             * has 22/54 chance to stop the inner loop on each
             * critter, 28/54 chance for another iteration.
             * Lawful caster has 26/54 chance to stop unless the
             * summoner is an angel or demon, in which case the
             * chance is 24/54.
             * Chaotic or unaligned caster has 19/54 chance to
             * stop, so will summon fewer creatures on average.
             *
             * The outer loop potentially gives chaotic/unaligned
             * a chance to even things up since others will hit
             * MAXNASTIES sooner, but its number of iterations is
             * randomized so it won't always do so.
             */
            for (j = 0; j < 20; j++) {
                int makeindex;
                /* Don't create more spellcasters of the monsters' level or
                 * higher--avoids chain summoners filling up the level.
                 */
                do {
                    makeindex = ((summoner && summoner->data == &mons[PM_KATHRYN_THE_ICE_QUEEN])
                                 ? pick_nasty_ice() : (summoner && summoner->data == &mons[PM_VECNA])
                                                    ? pick_nasty_vecna() : pick_nasty());
                    m_cls = mons[makeindex].mlet;
                } while (summoner
                         && ((attacktype(&mons[makeindex], AT_MAGC)
                             && mons[makeindex].difficulty
                             >= mons[summoner->mnum].difficulty)
                             || (s_cls == S_DEMON && m_cls == S_ANGEL)
                             || (s_cls == S_ANGEL && m_cls == S_DEMON)));
                /* do this after picking the monster to place */
                if (summoner && !enexto(&bypos, summoner->mux, summoner->muy,
                                        &mons[makeindex]))
                    continue;
                /* this honors genocide but overrides extinction; it ignores
                   inside-hell-only (G_HELL) & outside-hell-only (G_NOHELL) */
                if ((mtmp = makemon(&mons[makeindex], bypos.x, bypos.y,
                                    MM_ADJACENTOK)) != 0) {
                    mtmp->msleeping = mtmp->mpeaceful = mtmp->mtame = 0;
                    set_malign(mtmp);
                } else /* random monster to substitute for geno'd selection */
                    mtmp = makemon((struct permonst *) 0, bypos.x, bypos.y,
                                   MM_ADJACENTOK);
                if (mtmp) {
                    /* delay first use of spell or breath attack */
                    mtmp->mspec_used = rnd(4);
                    if (++count >= MAXNASTIES
                        || mon_aligntyp(mtmp) == 0
                        || sgn(mon_aligntyp(mtmp)) == castalign)
                        break;
                }
            }
        }
    if (count)
        count = monster_census(FALSE) - census;
    return count;
}

/* Let's resurrect the wizard, for some unexpected fun. */
void
resurrect()
{
    struct monst *mtmp, **mmtmp;
    long elapsed;
    const char *verb;

    if (!context.no_of_wizards) {
        /* make a new Wizard */
        verb = "kill";
        mtmp = makemon(&mons[PM_WIZARD_OF_YENDOR], u.ux, u.uy, MM_NOWAIT);
        /* affects experience; he's not coming back from a corpse
           but is subject to repeated killing like a revived corpse */
        if (mtmp) mtmp->mrevived = 1;
    } else {
        /* look for a migrating Wizard */
        verb = "elude";
        mmtmp = &migrating_mons;
        while ((mtmp = *mmtmp) != 0) {
            if (mtmp->iswiz
                /* if he has the Amulet, he won't bring it to you */
                && !mon_has_amulet(mtmp)
                && (elapsed = monstermoves - mtmp->mlstmv) > 0L) {
                mon_catchup_elapsed_time(mtmp, elapsed);
                if (elapsed >= LARGEST_INT)
                    elapsed = LARGEST_INT - 1;
                elapsed /= 50L;
                if (mtmp->msleeping && rn2((int) elapsed + 1))
                    mtmp->msleeping = 0;
                if (mtmp->mfrozen == 1) { /* would unfreeze on next move */
                    mtmp->mfrozen = 0;
                    if (!mtmp->mstone || mtmp->mstone > 2)
                        mtmp->mcanmove = 1;
                }
                if (mtmp->mcanmove && !mtmp->msleeping) {
                    *mmtmp = mtmp->nmon;
                    mon_arrive(mtmp, TRUE);
                    /* note: there might be a second Wizard; if so,
                       he'll have to wait til the next resurrection */
                    break;
                }
            }
            mmtmp = &mtmp->nmon;
        }
    }

    if (mtmp) {
        mtmp->mtame = mtmp->mpeaceful = 0; /* paranoia */
        set_malign(mtmp);
        if (!Deaf) {
            pline("A voice booms out...");
            verbalize("So thou thought thou couldst %s me, fool.", verb);
        }
    }
}

/* Here, we make trouble for the poor shmuck who actually
   managed to do in the Wizard. */
void
intervene()
{
    struct monst *mtmp = (struct monst *) 0;
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
            You("notice a %s glow surrounding you.", hcolor(NH_BLACK));
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
            do_earthquake((MAXULEV - 1) / 3 + 1);
            for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
                if (DEADMONSTER(mtmp))
                    continue;
                wakeup(mtmp, FALSE); /* peaceful monster will not become hostile */
            }
            /* shake up monsters in a much larger radius... */
            awaken_monsters(ROWNO * COLNO);
        }
        break;
    case 7:
        (void) nasty((struct monst *) 0, TRUE);
        break;
    }
}

void
wizdead()
{
    context.no_of_wizards--;
    if (!u.uevent.udemigod) {
        u.uevent.udemigod = TRUE;
        u.udg_cnt = rn1(250, 50);
    }
}

const char *const random_insult[] = {
    "antic",      "blackguard",   "caitiff",    "chucklehead",
    "coistrel",   "craven",       "cretin",     "cur",
    "dastard",    "demon fodder", "dimwit",     "dolt",
    "fool",       "footpad",      "imbecile",   "knave",
    "maledict",   "miscreant",    "niddering",  "poltroon",
    "rattlepate", "reprobate",    "scapegrace", "varlet",
    "villein", /* (sic.) */
    "wittol",     "worm",         "wretch",
};

const char *const random_malediction[] = {
    "Hell shall soon claim thy remains,", "I chortle at thee, thou pathetic",
    "Prepare to die, thou", "Resistance is useless,",
    "Surrender or die, thou", "There shall be no mercy, thou",
    "Thou shalt repent of thy cunning,", "Thou art as a flea to me,",
    "Thou art doomed,", "Thy fate is sealed,",
    "Verily, thou shalt be one dead"
};

const char *const random_icequeen[] = {
    "My magic is greater than yours", "You will never defeat me",
    "Winter shall last forever", "The cold never bothered me anyway",
    "Muahahahah", "I am even more powerful than the Wizard himself",
    "Run while you still can, fool", "Let's build a snowman",
    "The pegasus belongs to me"
};

const char *const random_enchantress[] = {
    "Thank you again for freeing me", "I have so much damage to undo",
    "I apologize for any harm I may have caused you",
    "Be careful leaving this place, I have no control over the monsters that still lurk here",
    "Please treat the pegasus well, it has been through a lot",
    "It will take a long time to regrow the forest"
};

const char *const random_vecna[] = {
    "I am Vecna the Unholy", "Kneel before me, wretched mortal",
    "You have no idea what true power is!  I will show you",
    "Your suffering will be legendary, even in hell",
    "Ah, the suffering.  The sweet, sweet suffering",
    "I will rip your soul to shreds"
};

const char *const random_talgath[] = {
    "Puny creature!  Bow before my might",
    "Look into my eyes, if you dare",
    "Beauty truly is in the eye of the beholder"
};

const char *const random_goblinking[] = {
    "Bones will be shattered, necks will be wrung!  You'll be beaten and battered, from racks you'll be hung",
    "You will die down here and never be found, down in the deep of Goblin Town",
    "With a swish and a smack, and a whip and a crack!  Everybody talks when they're on my rack",
    "Mine Town is under my control!  No one is allowed to visit",
    "So you want to explore Mines' End, eh?  Over my dead body",
};

const char *const random_gollum[] = {
    "Precious, precious, precious!  My Precious!  O my Precious",
    "Losst it is, my precious, lost, lost!  Curse us and crush us, my precious is lost",
    "Never!  Smeagol wouldn't hurt a fly",
    "It isn't fair, my precious, is it, to ask us what it's got in it's nassty little pocketsess",
    "We wants it, we needs it.  Must have the precious.  They stole it from us",
    "And they doesn't taste very nice, does they, Precious",
};

/* Insult or intimidate the player */
void
cuss(mtmp)
register struct monst *mtmp;
{
    if (Deaf)
        return;
    if (mtmp->iswiz) {
        if (!rn2(5)) /* typical bad guy action */
            pline("%s laughs fiendishly.", Monnam(mtmp));
        else if (u.uhave.amulet && !rn2(SIZE(random_insult)))
            verbalize("Relinquish the amulet, %s!",
                      random_insult[rn2(SIZE(random_insult))]);
        else if (u.uhp < 5 && !rn2(2)) /* Panic */
            verbalize(rn2(2) ? "Even now thy life force ebbs, %s!"
                             : "Savor thy breath, %s, it be thy last!",
                      random_insult[rn2(SIZE(random_insult))]);
        else if (mtmp->mhp < 5 && !rn2(2)) /* Parthian shot */
            verbalize(rn2(2) ? "I shall return." : "I'll be back.");
        else
            verbalize("%s %s!",
                      random_malediction[rn2(SIZE(random_malediction))],
                      random_insult[rn2(SIZE(random_insult))]);
    } else if (is_lminion(mtmp)
               && !(mtmp->isminion && EMIN(mtmp)->renegade)) {
        com_pager(rn2(QTN_ANGELIC - 1 + (Hallucination ? 1 : 0))
                  + QT_ANGELIC);
    } else if (mtmp->data == &mons[PM_KATHRYN_THE_ICE_QUEEN]) {
        if (!rn2(5)) {
            pline("%s points and giggles at you.", Monnam(mtmp));
            if (kathryn_bday())
                verbalize("It's my birthday!  Woohoo!!");
            else if (bourbon_bday())
                verbalize("Wish Bourbon a happy birthday!  Now!!");
            else if (ozzy_bday())
                verbalize("Say happy birthday to Ozzy!  Say it!!");
        } else {
            verbalize("%s!",
                      random_icequeen[rn2(SIZE(random_icequeen))]);
        }
    } else if (mtmp->data == &mons[PM_KATHRYN_THE_ENCHANTRESS]) {
        if (mtmp->mpeaceful) {
            if (!rn2(5)) {
                pline("%s waves to you.", Monnam(mtmp));
                if (kathryn_bday())
                    verbalize("You freed me on my birthday!  Thank you so much!!");
                else if (bourbon_bday())
                    verbalize("Happy birthday, Bourbon!  Good girl!!");
                else if (ozzy_bday())
                    verbalize("Happy birthday, Ozzy!  You're a good boy, yes you are!!");
            } else {
                verbalize("%s.",
                          random_enchantress[rn2(SIZE(random_enchantress))]);
            }
        } else {
            if (!rn2(7))
                pline("%s waves to you.", Monnam(mtmp));
        }
    } else if (mtmp->isvecna) {
        verbalize("%s!",
                  random_vecna[rn2(SIZE(random_vecna))]);
    } else if (mtmp->istalgath) {
        verbalize("%s!",
                  random_talgath[rn2(SIZE(random_talgath))]);
    } else if (mtmp->isgking) {
        if (!rn2(3)) {
            if (uwep && uwep->oartifact == ART_ORCRIST) {
                verbalize("I know that sword!  It is the Goblin Cleaver!  The Biter!  The blade that sliced a thousand necks!");
            } else if (uwep && uwep->oartifact == ART_GLAMDRING) {
                verbalize("%s wields the Foe-Hammer!  The Beater!  Bright as daylight!",
                          flags.female ? "She" : "He");
            }
        } else {
            verbalize("%s!",
                      random_goblinking[rn2(SIZE(random_goblinking))]);
        }
    } else if (mtmp->data == &mons[PM_GOLLUM]) {
        verbalize("%s!",
                  random_gollum[rn2(SIZE(random_gollum))]);
    } else {
        if (!rn2(is_minion(mtmp->data) ? 100 : 5))
            pline("%s casts aspersions on your ancestry.", Monnam(mtmp));
        else
            com_pager(rn2(QTN_DEMONIC) + QT_DEMONIC);
    }
}

const char *const random_mplayer_amulet[] = {
    "Give me the Amulet of Yendor",
    "Where is the Amulet?  I need it to escape this place",
    "This isn't Purgatory, it's Hell with a nice view",
    "Either you relinquish the Amulet now, or I will kill you"
};

const char *const random_mplayer_idol[] = {
    "Give me the Idol of Moloch",
    "Where is the Idol?  I need it to escape this place",
    "This isn't Purgatory, it's Hell with a nice view",
    "Either you relinquish the Idol now, or I will kill you"
};

/* Player monsters harass the player in Purgatory */
void
mplayer_purg_talk(mtmp)
register struct monst *mtmp;
{
    if (Deaf)
        return;
    if (is_mplayer(mtmp->data)) {
        if (Role_if(PM_INFIDEL))
            verbalize("%s!",
                      random_mplayer_idol[rn2(SIZE(random_mplayer_idol))]);
        else
            verbalize("%s!",
                      random_mplayer_amulet[rn2(SIZE(random_mplayer_amulet))]);
    }
}

const char *const random_arch_amulet[] = {
    "Hast thou come to repent thy sins?",
    "Give unto me the Amulet, and I shall cleanse thee of thy sins.",
    "Only those that are pure of heart can ascend to glory."
};

const char *const random_arch_idol[] = {
    "Begone, foul Infidel!  I cast thee back to hell!",
    "Return from whence thou cam'st!",
    "I shall relieve thee of thy charge, and destroy thine wicked Idol!"
};

/* Saint Michael the Archangel */
void
archangel_purg_talk(mtmp)
register struct monst *mtmp;
{
    if (Deaf)
        return;
    if (mtmp->data == &mons[PM_SAINT_MICHAEL]) {
        if (Role_if(PM_INFIDEL))
            verbalize("%s",
                      random_arch_idol[rn2(SIZE(random_arch_idol))]);
        else
            verbalize("%s",
                      random_arch_amulet[rn2(SIZE(random_arch_amulet))]);
    }
}

/*wizard.c*/
