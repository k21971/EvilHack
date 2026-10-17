/* NetHack 5.0	steed.c	$NHDT-Date: 1781973068 2026/06/20 16:31:08 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.132 $ */
/* Copyright (c) Kevin Hugo, 1998-1999. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* Monsters that might be ridden */
static NEARDATA const char steeds[] = { S_QUADRUPED, S_UNICORN,  S_ANGEL,
                                        S_CENTAUR,   S_DRAGON,   S_JABBERWOCK,
                                        S_DOG,       S_FELINE,   S_SPIDER,
                                        S_LIZARD,    S_SKELETON, '\0' };

/* Monsters that might wear barding */
static NEARDATA const char mbarding[] = { S_QUADRUPED, S_UNICORN,    S_ANGEL,
                                          S_DRAGON,    S_JABBERWOCK, S_DOG,
                                          S_FELINE,    S_SPIDER,     S_LIZARD,
                                          S_SKELETON,  '\0' };

staticfn boolean landing_spot(coord *, int, int);
staticfn void maybewakesteed(struct monst *);
staticfn void rebuff_rider(struct monst *);
staticfn void steed_off_map(struct monst *);

/* caller has decided that hero can't reach something while mounted */
void
rider_cant_reach(void)
{
    You("aren't skilled enough to reach from %s.", y_monnam(u.usteed));
}

/* keep a monster's steed in step with its rider; the steed stays on the
   fmon list but off the map, so its coordinates have to be carried along
   by hand every time the rider moves */
void
update_monsteed(struct monst *mtmp)
{
    struct monst *steed;

    if (!has_erid(mtmp))
        return;

    steed = ERID(mtmp)->mon_steed;
    if (!steed)
        return;
    /* defense in depth: a steed still in transit between levels has
       zeroed coordinates and overloaded fields that must not be
       stamped over; mon_arrive() syncs it when it arrives */
    if (steed->mstate & (MON_MIGRATING | MON_LIMBO))
        return;
    /* if its rider is carrying the steed out of reach, the hold between
       the steed and the hero ends: a steed that has the hero grabbed
       lets go, and the hero's own sticky grip on the steed slips off
       without a word, as rloc_to_core() does for a monster held
       directly; skip the test while the rider is off the map (a water
       bubble picking the pair up zeroes the rider's coordinates
       first) - rloc_to_core re-checks adjacency once the rider is
       placed again */
    if (steed == u.ustuck && !u.uswallow
        && isok(mtmp->mx, mtmp->my) && !m_next2u(mtmp)) {
        unstuck(steed);
        if (!sticks(gy.youmonst.data))
            You("get released!");
    }
    steed->mx = mtmp->mx;
    steed->my = mtmp->my;
    steed->mpeaceful = mtmp->mpeaceful;
    set_malign(steed);
}

/* take a steed off the map without disturbing the rest of its state; it
   remains on the fmon list with its coordinates synced to its rider,
   and the iter_mons family exempts it from its off-map skip */
staticfn void
steed_off_map(struct monst *steed)
{
    if (isok(steed->mx, steed->my)
        && svl.level.monsters[steed->mx][steed->my] == steed) {
        remove_monster(steed->mx, steed->my);
        newsym(steed->mx, steed->my);
    }
    /* like every other off-map transition, drop any pending terrain
       check; movemon() runs minliquid() on a ridden steed anyway */
    steed->mstate &= ~TERRAIN_FALLOUT_MASK;
    steed->mstate |= MON_OFFMAP;
}

/* assign a steed to a monster as the monster spawns */
void
mount_monster(
    struct monst *mtmp, /* rider */
    int pm)             /* steed */
{
    struct monst *mount;

    /* small hack here: make it in a random spot to avoid failures due
       to there not being enough room */
    mount = makemon(&mons[pm], 0, 0, MM_ADJACENTOK);
    /* no steed (doesn't exist, genocided, extinct) or if for
       some reason the steed is covetous - denied */
    if (!mount || is_covetous(mount->data)
        || ((svm.mvitals[monsndx(mount->data)].mvflags & G_GONE)
            && !(mount->data == &mons[PM_PALE_HORSE]
                 || mount->data == &mons[PM_WHITE_HORSE]
                 || mount->data == &mons[PM_BLACK_HORSE]))) {
        /* take back the just-made steed instead of leaving it placed
           on the map to roam unridden */
        if (mount)
            mount = unmakemon(mount, MM_ADJACENTOK);
        return;
    }
    steed_off_map(mount);

    newerid(mtmp);
    ERID(mtmp)->mon_steed = mount;
    ERID(mtmp)->mid = mount->m_id;
    mount->ridden_by = mtmp->m_id;
    mount->mx = mtmp->mx;
    mount->my = mtmp->my;
    newsym(mtmp->mx, mtmp->my);

    /* rider over'rides' steed's natural inclinations */
    mount->mpeaceful = mtmp->mpeaceful;
    set_malign(mount);

    /* monster steeds will come with a saddle */
    if (can_saddle(mount) && !which_armor(mount, W_SADDLE)) {
        struct obj *otmp = mksobj(SADDLE, TRUE, FALSE);

        if (otmp)
            put_saddle_on_mon(otmp, mount);
    }

    /* if the monster steed has a saddle, there's a chance it's wearing
       barding also */
    if (!rn2(10) && which_armor(mount, W_SADDLE)
        && can_wear_barding(mount) && !which_armor(mount, W_BARDING)) {
        struct obj *otmp = mksobj(rn2(4) ? BARDING
                                  : rn2(3) ? SPIKED_BARDING
                                    : BARDING_OF_REFLECTION, TRUE, FALSE);

        if (otmp) {
            reroll_barding_material(otmp, mount, mtmp);
            put_barding_on_mon(otmp, mount);
        }
    }
}

/* an unmounted monster looks around for something to climb onto */
boolean
mount_up(struct monst *rider)
{
    struct monst *steed, *nmon;

    /* not acceptable as riders */
    if (!mon_can_ride(rider) || has_erid(rider))
        return FALSE;

    for (steed = fmon; steed; steed = nmon) {
        nmon = steed->nmon;
        if (nmon == rider)
            nmon = rider->nmon;
        if (steed == rider)
            continue;
        /* the mount the hero is sitting on is not up for grabs. A steed
           untamed by life saving, a charm or knock stays mounted until
           dismount_untamed_steed() throws the hero off, which runs before
           each monster moves, so this should not fire; carrying a monster
           and the hero at once would corrupt state in ways nothing else
           checks for */
        if (steed == u.usteed)
            continue;
        /* criteria for an acceptable steed */
        if (steed->data == &mons[PM_CAVE_BEAR]
            || steed->data == &mons[PM_GRIZZLY_BEAR])
            continue;
        if (steed->data == &mons[PM_DEER]
            || steed->data == &mons[PM_STAG])
            continue;
        if (!racial_drow(rider) && is_spider(steed->data))
            continue;
        if (!racial_drow(rider)
            && (steed->data == &mons[PM_CAVE_LIZARD]
                || steed->data == &mons[PM_LARGE_CAVE_LIZARD]))
            continue;
        if (!racial_orc(rider)
            && (steed->data == &mons[PM_WARG]
                || steed->data == &mons[PM_WOLF]))
            continue;
        if (rider->data != &mons[PM_CAVE_DWELLER]
            && steed->data == &mons[PM_SABER_TOOTHED_TIGER])
            continue;
        if (!(racial_zombie(rider) || racial_vampire(rider))
            && (steed->data == &mons[PM_SKELETAL_PONY]
                || steed->data == &mons[PM_SKELETAL_HORSE]
                || steed->data == &mons[PM_SKELETAL_WARHORSE]
                || steed->data == &mons[PM_SKELETAL_DRAGON]))
            continue;
        /* don't let wild riders commandeer the hero's pet */
        if (steed->mtame)
            continue;
        if (monnear(rider, steed->mx, steed->my)
            && mon_can_be_ridden(steed) && !steed->ridden_by)
            break;
    }
    /* not a steed? can't ride */
    if (!steed)
        return FALSE;
    /* steed doesn't have a saddle? can't ride */
    if (!which_armor(steed, W_SADDLE))
        return FALSE;

    if (canseemon(rider)) {
        pline("%s clambers onto %s!", Monnam(rider),
              canseemon(steed) ? mon_nam(steed) : "something");
    } else if (!Deaf && mdistu(rider) <= 5) {
        You_hear("someone %s.", Hallucination
                                ? "getting on their high horse"
                                : "jump into a saddle");
    }
    steed_off_map(steed);

    newerid(rider);
    ERID(rider)->mon_steed = steed;
    ERID(rider)->mid = steed->m_id;
    steed->ridden_by = rider->m_id;
    steed->mx = rider->mx;
    steed->my = rider->my;
    newsym(rider->mx, rider->my);

    /* rider over'rides' steed's natural inclinations */
    steed->mpeaceful = rider->mpeaceful;
    set_malign(steed);
    return TRUE;
}

/* break the link between a monster rider and its steed, putting both
   back onto the map */
void
separate_steed_and_rider(struct monst *rider)
{
    struct monst *steed;
    coord cc;

    if (!has_erid(rider))
        return;

    steed = ERID(rider)->mon_steed;
    free_erid(rider);
    if (!steed)
        return;

    /* handle rider if both rider and steed are alive */
    if (!DEADMONSTER(rider) && !DEADMONSTER(steed)) {
        coordxy orig_x = rider->mx, orig_y = rider->my;
        boolean rider_moved;

        /* move rider to an adjacent tile */
        if (enexto_core_mon(&cc, rider->mx, rider->my, rider,
                            NO_MM_FLAGS)) {
            rloc_to(rider, cc.x, cc.y);
            rider_moved = TRUE;
        } else {
            /* evidently no room nearby; move rider elsewhere */
            rider_moved = rloc(rider, RLOC_ERR | RLOC_NOMSG);
        }
        if (rider_moved) {
            place_monster(steed, orig_x, orig_y);
        } else if (enexto_core_mon(&cc, orig_x, orig_y, steed,
                                   NO_MM_FLAGS)) {
            /* rider stuck at orig; find a different spot for steed */
            place_monster(steed, cc.x, cc.y);
        } else {
            /* truly nowhere to put the steed - dismiss it */
            mongone(steed);
        }
    }
    /* place rider if steed dies and rider is still alive; else-if so
       that a rider killed inside the block above (rloc_to can run
       mintrap on the destination) cannot re-place a steed the block
       already placed */
    else if (!DEADMONSTER(rider) && DEADMONSTER(steed)) {
        remove_monster(steed->mx, steed->my); /* remove pointer to steed */
        place_monster(rider, rider->mx, rider->my);
    }
    /* place steed if rider dies and steed is still alive */
    else if (!DEADMONSTER(steed) && DEADMONSTER(rider))
        place_monster(steed, steed->mx, steed->my);

    /* rloc_to/rloc above can kill the rider via mintrap on the destination
       (collapsing tile, anti-magic field, lava, statue corpse, gas-cloud
       region damage); mongone(steed) above can dismiss the steed. Guard
       each side independently against a stale-coord deref */
    if (!DEADMONSTER(rider)) {
        update_monster_region(rider);
        newsym(rider->mx, rider->my);
    }
    if (!DEADMONSTER(steed)) {
        update_monster_region(steed);
        newsym(steed->mx, steed->my);
    }
}

/* a monster snared by a web is shaken off whatever it was riding */
void
web_unseats_rider(struct monst *mon)
{
    /* web traps don't kill, but the DEADMONSTER guard is cheap insurance
       in case mintrap() ever grows a lethal side effect on this path */
    if (DEADMONSTER(mon) || !mon->mtrapped || !has_erid(mon))
        return;
    if (canseemon(mon))
        pline("%s falls off %s %s!", Monnam(mon), mhis(mon),
              l_monnam(ERID(mon)->mon_steed));
    separate_steed_and_rider(mon);
}

/* return the monster riding mtmp, if any */
struct monst *
get_mon_rider(struct monst *mtmp)
{
    struct monst *mtmp2;

    if (!mtmp->ridden_by)
        return (struct monst *) 0;
    for (mtmp2 = fmon; mtmp2; mtmp2 = mtmp2->nmon) {
        if (mtmp->ridden_by == mtmp2->m_id)
            return mtmp2;
    }
    return (struct monst *) 0;
}

/* the monster through which mtmp and the hero are stuck together,
   whichever of them holds the other: mtmp itself, or the steed it is
   riding; null if there is no such hold */
struct monst *
ustuck_self_or_steed(struct monst *mtmp)
{
    if (!u.ustuck)
        return (struct monst *) 0;
    if (mtmp == u.ustuck)
        return mtmp;
    if (has_erid(mtmp) && ERID(mtmp)->mon_steed == u.ustuck)
        return u.ustuck;
    return (struct monst *) 0;
}

/*** Putting the saddle on ***/

/* Can this monster wear a saddle? */
boolean
can_saddle(struct monst *mtmp)
{
    struct permonst *ptr = r_data(mtmp);

    return (strchr(steeds, ptr->mlet) && (ptr->msize >= MZ_MEDIUM)
            && (!humanoid(ptr) || ptr->mlet == S_CENTAUR) && !amorphous(ptr)
            && !noncorporeal(ptr) && !is_whirly(ptr) && !unsolid(ptr)
            && !(ptr->mlet == S_QUADRUPED
                 && mtmp->mnum != PM_ROTHE
                 && mtmp->mnum != PM_LEOCROTTA
                 && mtmp->mnum != PM_WUMPUS
                 && mtmp->mnum != PM_MUMAK
                 && mtmp->mnum != PM_TITANOTHERE
                 && mtmp->mnum != PM_BALUCHITHERIUM
                 && mtmp->mnum != PM_MASTODON
                 && mtmp->mnum != PM_WOOLLY_MAMMOTH)
            && !(ptr->mlet == S_UNICORN
                 && mtmp->mnum != PM_PONY
                 && mtmp->mnum != PM_HORSE
                 && mtmp->mnum != PM_WARHORSE
                 && mtmp->mnum != PM_LESSER_NIGHTMARE
                 && mtmp->mnum != PM_NIGHTMARE
                 && mtmp->mnum != PM_CAUCHEMAR
                 && mtmp->mnum != PM_WHITE_UNICORN
                 && mtmp->mnum != PM_GRAY_UNICORN
                 && mtmp->mnum != PM_BLACK_UNICORN
                 && mtmp->mnum != PM_PEGASUS
                 && mtmp->mnum != PM_GREATER_PEGASUS
                 && mtmp->mnum != PM_PALE_HORSE
                 && mtmp->mnum != PM_BLACK_HORSE
                 && mtmp->mnum != PM_WHITE_HORSE
                 && mtmp->mnum != PM_RED_HORSE)
            && !(ptr->mlet == S_JABBERWOCK
                 && mtmp->mnum != PM_JABBERWOCK
                 && mtmp->mnum != PM_VORPAL_JABBERWOCK)
            && !(ptr->mlet == S_DOG
                 && mtmp->mnum != PM_WOLF
                 && mtmp->mnum != PM_WARG)
            && !(ptr->mlet == S_SPIDER
                 && mtmp->mnum != PM_GIANT_SPIDER
                 && mtmp->mnum != PM_GARGANTUAN_SPIDER)
            && !(ptr->mlet == S_LIZARD
                 && mtmp->mnum != PM_CAVE_LIZARD
                 && mtmp->mnum != PM_LARGE_CAVE_LIZARD)
            && !(ptr->mlet == S_SKELETON
                 && mtmp->mnum != PM_SKELETAL_PONY
                 && mtmp->mnum != PM_SKELETAL_HORSE
                 && mtmp->mnum != PM_SKELETAL_WARHORSE
                 && mtmp->mnum != PM_SKELETAL_DRAGON)
            && !(ptr->mlet == S_FELINE
                 && mtmp->mnum != PM_SABER_TOOTHED_TIGER));
}

/* Can this monster wear barding? */
boolean
can_wear_barding(struct monst *mtmp)
{
    struct permonst *ptr = r_data(mtmp);

    return (strchr(mbarding, ptr->mlet) && (ptr->msize >= MZ_MEDIUM)
            && !humanoid(ptr) && !amorphous(ptr) && !noncorporeal(ptr)
            && !is_whirly(ptr) && !unsolid(ptr)
            && !(ptr->mlet == S_QUADRUPED
                 && mtmp->mnum != PM_ROTHE
                 && mtmp->mnum != PM_LEOCROTTA
                 && mtmp->mnum != PM_WUMPUS
                 && mtmp->mnum != PM_MUMAK
                 && mtmp->mnum != PM_TITANOTHERE
                 && mtmp->mnum != PM_BALUCHITHERIUM
                 && mtmp->mnum != PM_MASTODON
                 && mtmp->mnum != PM_WOOLLY_MAMMOTH)
            && !(ptr->mlet == S_UNICORN
                 && mtmp->mnum != PM_PONY
                 && mtmp->mnum != PM_HORSE
                 && mtmp->mnum != PM_WARHORSE
                 && mtmp->mnum != PM_LESSER_NIGHTMARE
                 && mtmp->mnum != PM_NIGHTMARE
                 && mtmp->mnum != PM_CAUCHEMAR
                 && mtmp->mnum != PM_WHITE_UNICORN
                 && mtmp->mnum != PM_GRAY_UNICORN
                 && mtmp->mnum != PM_BLACK_UNICORN
                 && mtmp->mnum != PM_PEGASUS
                 && mtmp->mnum != PM_GREATER_PEGASUS
                 && mtmp->mnum != PM_PALE_HORSE
                 && mtmp->mnum != PM_BLACK_HORSE
                 && mtmp->mnum != PM_WHITE_HORSE
                 && mtmp->mnum != PM_RED_HORSE)
            && !(ptr->mlet == S_JABBERWOCK
                 && mtmp->mnum != PM_JABBERWOCK
                 && mtmp->mnum != PM_VORPAL_JABBERWOCK)
            && !(ptr->mlet == S_DOG
                 && mtmp->mnum != PM_WOLF
                 && mtmp->mnum != PM_WARG)
            && !(ptr->mlet == S_SPIDER
                 && mtmp->mnum != PM_GIANT_SPIDER
                 && mtmp->mnum != PM_GARGANTUAN_SPIDER)
            && !(ptr->mlet == S_LIZARD
                 && mtmp->mnum != PM_CAVE_LIZARD
                 && mtmp->mnum != PM_LARGE_CAVE_LIZARD)
            && !(ptr->mlet == S_SKELETON
                 && mtmp->mnum != PM_SKELETAL_PONY
                 && mtmp->mnum != PM_SKELETAL_HORSE
                 && mtmp->mnum != PM_SKELETAL_WARHORSE
                 && mtmp->mnum != PM_SKELETAL_DRAGON)
            && !(ptr->mlet == S_FELINE
                 && mtmp->mnum != PM_SABER_TOOTHED_TIGER));
}

/* a mount that only accepts riders of a particular race or role has just
   snarled, hissed or rattled at the hero; sometimes it turns on him */
staticfn void
rebuff_rider(struct monst *mtmp)
{
    if ((mtmp->mtame > 0 || mtmp->mpeaceful) && !rn2(3)) {
        mtmp->mtame = mtmp->mpeaceful = 0;
        set_malign(mtmp);
        if (mtmp->mleashed)
            m_unleash(mtmp, TRUE);
        newsym(mtmp->mx, mtmp->my);
    }
}

int
use_saddle(struct obj *otmp)
{
    struct monst *mtmp;
    struct permonst *ptr;
    int chance;

    if (!u_handsy())
        return ECMD_OK;

    /* Select an animal */
    if (u.uswallow || Underwater || !getdir((char *) 0)) {
        pline1(Never_mind);
        return ECMD_CANCEL;
    }
    if (!u.dx && !u.dy) {
        pline("Saddle yourself?  Very funny...");
        return ECMD_OK;
    }
    if (!isok(u.ux + u.dx, u.uy + u.dy)
        || !(mtmp = m_at(u.ux + u.dx, u.uy + u.dy)) || !canspotmon(mtmp)) {
        pline("I see nobody there.");
        return ECMD_TIME;
    }

    /* Is this a valid monster? */
    if ((mtmp->misc_worn_check & W_SADDLE) != 0L
        || which_armor(mtmp, W_SADDLE)) {
        pline("%s doesn't need another one.", Monnam(mtmp));
        return ECMD_TIME;
    }
    ptr = mtmp->data;
    if (touch_petrifies(ptr) && !uarmg && !Stone_resistance) {
        char kbuf[BUFSZ];

        You("touch %s.", mon_nam(mtmp));
        if (!(poly_when_stoned(gy.youmonst.data)
              && polymon(stoned_form(gy.youmonst.data)))) {
            Sprintf(kbuf, "attempting to saddle %s",
                    an(pmname(mtmp->data, Mgender(mtmp))));
            instapetrify(kbuf);
        }
    }
    if (ptr == &mons[PM_AMOROUS_DEMON]) {
        pline("Shame on you!");
        exercise(A_WIS, FALSE);
        return ECMD_TIME;
    }
    if (mtmp->isminion || mtmp->isshk || mtmp->ispriest || mtmp->isgd
        || mtmp->iswiz) {
        pline("I think %s would mind.", mon_nam(mtmp));
        return ECMD_TIME;
    }
    if ((ptr == &mons[PM_WARG] || ptr == &mons[PM_WOLF])
        && !racial_orc(&gy.youmonst)) {
        if (!Deaf)
            pline("%s %s menacingly at you!", Monnam(mtmp),
                  rn2(2) ? "snarls" : "growls");
        rebuff_rider(mtmp);
        return ECMD_TIME;
    }
    if (ptr == &mons[PM_SABER_TOOTHED_TIGER]
        && !Role_if(PM_CAVE_DWELLER)) {
        if (!Deaf)
            pline("%s %s menacingly at you!", Monnam(mtmp),
                  rn2(2) ? "snarls" : "growls");
        rebuff_rider(mtmp);
        return ECMD_TIME;
    }
    if ((ptr == &mons[PM_GIANT_SPIDER]
         || ptr == &mons[PM_GARGANTUAN_SPIDER]
         || ptr == &mons[PM_CAVE_LIZARD]
         || ptr == &mons[PM_LARGE_CAVE_LIZARD])
        && !racial_drow(&gy.youmonst)) {
        if (!Deaf)
            pline("%s hisses threateningly at you!", Monnam(mtmp));
        rebuff_rider(mtmp);
        return ECMD_TIME;
    }
    if ((ptr == &mons[PM_SKELETAL_PONY]
         || ptr == &mons[PM_SKELETAL_HORSE]
         || ptr == &mons[PM_SKELETAL_WARHORSE]
         || ptr == &mons[PM_SKELETAL_DRAGON])
        && !(racial_zombie(&gy.youmonst) || racial_vampire(&gy.youmonst))) {
        if (!Deaf) {
            pline("%s rattles noisily at you!", Monnam(mtmp));
            You("freeze for a moment.");
            nomul(-2);
            gm.multi_reason = "scared by rattling";
            gn.nomovemsg = 0;
        }
        rebuff_rider(mtmp);
        return ECMD_TIME;
    }
    if (!can_saddle(mtmp)) {
        You_cant("saddle such a creature.");
        return ECMD_TIME;
    }

    /* Calculate your chance */
    chance = ACURR(A_DEX) + ACURR(A_CHA) / 2 + 2 * mtmp->mtame;
    chance += u.ulevel * (mtmp->mtame ? 20 : 5);
    if (!mtmp->mtame)
        chance -= 10 * mtmp->m_lev;
    if (Role_if(PM_KNIGHT))
        chance += 20;
    switch (P_SKILL(P_RIDING)) {
    case P_ISRESTRICTED:
    case P_UNSKILLED:
    default:
        chance -= 20;
        break;
    case P_BASIC:
        break;
    case P_SKILLED:
        chance += 15;
        break;
    case P_EXPERT:
        chance += 30;
        break;
    }
    if (Confusion || Fumbling || Glib)
        chance -= 20;
    else if (uarmg && objdescr_is(uarmg, "riding gloves"))
        /* Bonus for wearing "riding" (but not fumbling) gloves */
        chance += 10;
    else if (uarmf && objdescr_is(uarmf, "riding boots"))
        /* ... or for "riding boots" */
        chance += 10;
    if (otmp->cursed)
        chance -= 50;

    /* [intended] steed becomes alert if possible */
    maybewakesteed(mtmp);

    /* Make the attempt */
    if (rn2(100) < chance) {
        You("put the saddle on %s.", mon_nam(mtmp));
        if (otmp->owornmask)
            remove_worn_item(otmp, FALSE);
        freeinv(otmp);
        /* !can_saddle(mtmp) already eliminated above */
        put_saddle_on_mon(otmp, mtmp);
    } else
        pline("%s resists!", Monnam(mtmp));
    return ECMD_TIME;
}

void
put_saddle_on_mon(struct obj *saddle, struct monst *mtmp)
{
    if (!can_saddle(mtmp) || which_armor(mtmp, W_SADDLE)) {
        if (saddle)
            impossible("put_saddle_on_mon: saddle obj could get orphaned");
        return;
    }
    if (!saddle) {
        if ((saddle = mksobj(SADDLE, TRUE, FALSE)) != 0) {
            fully_identify_obj(saddle);
            /* mpickobj can later override identification if out-of-view */
        } else {
            return;
        }
    }
    if (mpickobj(mtmp, saddle))
        panic("merged saddle?");
    mtmp->misc_worn_check |= W_SADDLE;
    saddle->owornmask = W_SADDLE;
    saddle->leashmon = mtmp->m_id;
    update_mon_extrinsics(mtmp, saddle, TRUE, FALSE);
}

/*** Putting the barding on ***/

int
use_barding(struct obj *otmp)
{
    struct monst *mtmp;
    struct permonst *ptr;
    int chance;

    if (!u_handsy())
        return ECMD_OK;

    /* Select an animal */
    if (u.uswallow || Underwater || !getdir((char *) 0)) {
        pline1(Never_mind);
        return ECMD_CANCEL;
    }
    if (!u.dx && !u.dy) {
        pline("Put barding on yourself?  Very funny...");
        return ECMD_OK;
    }
    if (!isok(u.ux + u.dx, u.uy + u.dy)
        || !(mtmp = m_at(u.ux + u.dx, u.uy + u.dy)) || !canspotmon(mtmp)) {
        pline("I see nobody there.");
        return ECMD_TIME;
    }

    /* Is this a valid monster? */
    if ((mtmp->misc_worn_check & W_BARDING) != 0L
        || which_armor(mtmp, W_BARDING)) {
        pline("%s doesn't need another one.", Monnam(mtmp));
        return ECMD_TIME;
    }
    ptr = mtmp->data;
    if (touch_petrifies(ptr) && !uarmg && !Stone_resistance) {
        char kbuf[BUFSZ];

        You("touch %s.", mon_nam(mtmp));
        if (!(poly_when_stoned(gy.youmonst.data)
              && polymon(stoned_form(gy.youmonst.data)))) {
            Sprintf(kbuf, "attempting to put barding on %s",
                    an(pmname(mtmp->data, Mgender(mtmp))));
            instapetrify(kbuf);
        }
    }
    if (ptr == &mons[PM_AMOROUS_DEMON]) {
        pline("This won't accessorize well...");
        exercise(A_WIS, FALSE);
        return ECMD_TIME;
    }
    if (mtmp->isminion || mtmp->isshk || mtmp->ispriest || mtmp->isgd
        || mtmp->iswiz) {
        pline("I think %s would mind.", mon_nam(mtmp));
        return ECMD_TIME;
    }
    if ((ptr == &mons[PM_WARG] || ptr == &mons[PM_WOLF])
        && !racial_orc(&gy.youmonst)) {
        if (!Deaf)
            pline("%s %s menacingly at you!", Monnam(mtmp),
                  rn2(2) ? "snarls" : "growls");
        rebuff_rider(mtmp);
        return ECMD_TIME;
    }
    if (ptr == &mons[PM_SABER_TOOTHED_TIGER]
        && !Role_if(PM_CAVE_DWELLER)) {
        if (!Deaf)
            pline("%s %s menacingly at you!", Monnam(mtmp),
                  rn2(2) ? "snarls" : "growls");
        rebuff_rider(mtmp);
        return ECMD_TIME;
    }
    if ((ptr == &mons[PM_GIANT_SPIDER]
         || ptr == &mons[PM_GARGANTUAN_SPIDER]
         || ptr == &mons[PM_CAVE_LIZARD]
         || ptr == &mons[PM_LARGE_CAVE_LIZARD])
        && !racial_drow(&gy.youmonst)) {
        if (!Deaf)
            pline("%s hisses threateningly at you!", Monnam(mtmp));
        rebuff_rider(mtmp);
        return ECMD_TIME;
    }
    if ((ptr == &mons[PM_SKELETAL_PONY]
         || ptr == &mons[PM_SKELETAL_HORSE]
         || ptr == &mons[PM_SKELETAL_WARHORSE]
         || ptr == &mons[PM_SKELETAL_DRAGON])
        && !(racial_zombie(&gy.youmonst) || racial_vampire(&gy.youmonst))) {
        if (!Deaf) {
            pline("%s rattles noisily at you!", Monnam(mtmp));
            You("freeze for a moment.");
            nomul(-2);
            gm.multi_reason = "scared by rattling";
            gn.nomovemsg = 0;
        }
        rebuff_rider(mtmp);
        return ECMD_TIME;
    }
    if (!can_wear_barding(mtmp)) {
        You_cant("put barding on such a creature.");
        return ECMD_TIME;
    }

    /* Calculate your chance (same as for using a saddle) */
    chance = ACURR(A_DEX) + ACURR(A_CHA) / 2 + 2 * mtmp->mtame;
    chance += u.ulevel * (mtmp->mtame ? 20 : 5);
    if (!mtmp->mtame)
        chance -= 10 * mtmp->m_lev;
    if (Role_if(PM_KNIGHT))
        chance += 20;
    switch (P_SKILL(P_RIDING)) {
    case P_ISRESTRICTED:
    case P_UNSKILLED:
    default:
        chance -= 20;
        break;
    case P_BASIC:
        break;
    case P_SKILLED:
        chance += 15;
        break;
    case P_EXPERT:
        chance += 30;
        break;
    }
    if (Confusion || Fumbling || Glib)
        chance -= 20;
    else if (uarmg && objdescr_is(uarmg, "riding gloves"))
        /* Bonus for wearing "riding" (but not fumbling) gloves */
        chance += 10;
    else if (uarmf && objdescr_is(uarmf, "riding boots"))
        /* ... or for "riding boots" */
        chance += 10;
    if (otmp->cursed)
        chance -= 50;

    /* [intended] steed becomes alert if possible */
    maybewakesteed(mtmp);

    /* Make the attempt */
    if (rn2(100) < chance) {
        You("fit the barding on %s.", mon_nam(mtmp));
        if (otmp->owornmask)
            remove_worn_item(otmp, FALSE);
        freeinv(otmp);
        /* !can_wear_barding(mtmp) already eliminated above */
        put_barding_on_mon(otmp, mtmp);
    } else
        pline("%s resists!", Monnam(mtmp));
    return ECMD_TIME;
}

void
put_barding_on_mon(struct obj *barding, struct monst *mtmp)
{
    if (!can_wear_barding(mtmp) || which_armor(mtmp, W_BARDING)) {
        if (barding)
            impossible("put_barding_on_mon: barding obj could get orphaned");
        return;
    }
    if (!barding) {
        if ((barding = mksobj(BARDING, TRUE, FALSE)) != 0) {
            fully_identify_obj(barding);
            /* mpickobj can later override identification if out-of-view */
        } else {
            return;
        }
    }
    if (mpickobj(mtmp, barding))
        panic("merged barding?");
    mtmp->misc_worn_check |= W_BARDING;
    barding->owornmask = W_BARDING;
    barding->leashmon = mtmp->m_id;
    update_mon_extrinsics(mtmp, barding, TRUE, FALSE);
}

/* reroll freshly spawned steed barding so neither the steed nor its
   rider (if any) is averse to its material; mirrors the hated-material
   reroll mongets() does for ordinary monster gear */
void
reroll_barding_material(
    struct obj *barding,
    struct monst *steed,
    struct monst *rider)
{
    int tryct = 0, mat = 1;

    if (!barding)
        return;
    while (mon_hates_material(steed, barding->material)
           || (rider && mon_hates_material(rider, barding->material))) {
        init_obj_material(barding);
        if (++tryct >= 100) {
            for (mat = 1; mat < NUM_MATERIAL_TYPES; ++mat) {
                if (valid_obj_material(barding, mat)
                    && !mon_hates_material(steed, mat)
                    && !(rider && mon_hates_material(rider, mat))) {
                    set_material(barding, mat);
                    break;
                }
            }
            if (mat == NUM_MATERIAL_TYPES) {
                impossible("no valid barding material (steed %d, rider %d)",
                           monsndx(steed->data),
                           rider ? monsndx(rider->data) : -1);
                set_material(barding, objects[barding->otyp].oc_material);
            }
            break;
        }
    }
}

/*** Riding the monster ***/

/* Can we ride this monster?  Caller should also check can_saddle() */
boolean
can_ride(struct monst *mtmp)
{
    /* tortles are too ungainly to ride anything; giants can only
       ride woolly mammoths */
    return ((mtmp->mtame && humanoid(gy.youmonst.data)
             && !maybe_polyd(is_tortle(gy.youmonst.data),
                             Race_if(PM_TORTLE))
             && !verysmall(gy.youmonst.data)
             && !bigmonst(gy.youmonst.data)
             && (!Underwater || is_swimmer(mtmp->data)))
            || (r_data(mtmp) == &mons[PM_WOOLLY_MAMMOTH]
                && Race_if(PM_GIANT)));
}

/* the #ride command */
int
doride(void)
{
    boolean forcemount = FALSE;

    if (u.usteed) {
        dismount_steed(DISMOUNT_BYCHOICE);
    } else if (getdir((char *) 0) && isok(u.ux + u.dx, u.uy + u.dy)) {
        if (wizard && y_n("Force the mount to succeed?") == 'y')
            forcemount = TRUE;
        return (mount_steed(m_at(u.ux + u.dx, u.uy + u.dy), forcemount)
                ? ECMD_TIME : ECMD_OK);
    } else {
        return ECMD_CANCEL;
    }
    return ECMD_TIME;
}

/* Start riding, with the given monster */
boolean
mount_steed(
    struct monst *mtmp, /* The animal */
    boolean force)      /* Quietly force this animal */
{
    struct obj *otmp;
    struct obj *barding;
    char buf[BUFSZ];
    int role_modifier;
    struct permonst *ptr;

    /* Sanity checks */
    if (u.usteed) {
        You("are already riding %s.", mon_nam(u.usteed));
        return (FALSE);
    }

    /* Is the player in the right form? */
    if (Hallucination && !force) {
        pline("Maybe you should find a designated driver.");
        return (FALSE);
    }
    /* While riding, Wounded_legs refers to the steed's
     * legs, not the hero's legs.
     * That opens up a potential abuse where the player
     * can mount a steed, then dismount immediately to
     * heal leg damage, because leg damage is always
     * healed upon dismount (Wounded_legs context switch).
     * By preventing a hero with Wounded_legs from
     * mounting a steed, the potential for abuse is
     * reduced.  However, dismounting still immediately
     * heals the steed's wounded legs.  [In 3.4.3 and
     * earlier, that unintentionally made the hero's
     * temporary 1 point Dex loss become permanent.]
     */
    if (Wounded_legs) {
        char qbuf[QBUFSZ];

        legs_in_no_shape("riding", FALSE);
        Sprintf(qbuf, "Heal your leg%s?",
                ((HWounded_legs & BOTH_SIDES) == BOTH_SIDES) ? "s" : "");
        if (force && wizard && y_n(qbuf) == 'y')
            heal_legs(0);
        else
            return (FALSE);
    }

    if (Upolyd && (!humanoid(gy.youmonst.data)
                   || verysmall(gy.youmonst.data)
                   || bigmonst(gy.youmonst.data)
                   || slithy(gy.youmonst.data))) {
        You("won't fit on a saddle.");
        return (FALSE);
    }
    if (!force && (near_capacity() > SLT_ENCUMBER)) {
        You_cant("do that while carrying so much stuff.");
        return (FALSE);
    }

    /* Can the player reach and see the monster? */
    if (!mtmp || (!force && ((Blind && !Blind_telepat) || mtmp->mundetected
                             || M_AP_TYPE(mtmp) == M_AP_FURNITURE
                             || M_AP_TYPE(mtmp) == M_AP_OBJECT))) {
        pline("I see nobody there.");
        return (FALSE);
    }
    if (mtmp->data == &mons[PM_LONG_WORM]
        && (u.ux + u.dx != mtmp->mx || u.uy + u.dy != mtmp->my)) {
        /* As of 3.6.2:  test_move(below) is used to check for trying to mount
           diagonally into or out of a doorway or through a tight squeeze;
           attempting to mount a tail segment when hero was not adjacent
           to worm's head could trigger an impossible() in worm_cross()
           called from test_move(), so handle not-on-head before that */
        You("couldn't ride %s, let alone its tail.", a_monnam(mtmp));
        return FALSE;
    }
    if ((mtmp->data == &mons[PM_WARG] || mtmp->data == &mons[PM_WOLF])
        && !racial_orc(&gy.youmonst)) {
        if (!Deaf)
            pline("%s %s menacingly at you!", Monnam(mtmp),
                  rn2(2) ? "snarls" : "growls");
        rebuff_rider(mtmp);
        return FALSE;
    }
    if (mtmp->data == &mons[PM_SABER_TOOTHED_TIGER]
        && !Role_if(PM_CAVE_DWELLER)) {
        if (!Deaf)
            pline("%s %s menacingly at you!", Monnam(mtmp),
                  rn2(2) ? "snarls" : "growls");
        rebuff_rider(mtmp);
        return FALSE;
    }
    if ((mtmp->data == &mons[PM_GIANT_SPIDER]
         || mtmp->data == &mons[PM_GARGANTUAN_SPIDER]
         || mtmp->data == &mons[PM_CAVE_LIZARD]
         || mtmp->data == &mons[PM_LARGE_CAVE_LIZARD])
        && !racial_drow(&gy.youmonst)) {
        if (!Deaf)
            pline("%s hisses threateningly at you!", Monnam(mtmp));
        rebuff_rider(mtmp);
        return FALSE;
    }
    if ((mtmp->data == &mons[PM_SKELETAL_PONY]
         || mtmp->data == &mons[PM_SKELETAL_HORSE]
         || mtmp->data == &mons[PM_SKELETAL_WARHORSE]
         || mtmp->data == &mons[PM_SKELETAL_DRAGON])
        && !(racial_zombie(&gy.youmonst) || racial_vampire(&gy.youmonst))) {
        if (!Deaf) {
            pline("%s rattles noisily at you!", Monnam(mtmp));
            You("freeze for a moment.");
            nomul(-2);
            gm.multi_reason = "scared by rattling";
            gn.nomovemsg = 0;
        }
        rebuff_rider(mtmp);
        return FALSE;
    }
    if (u.uswallow || u.ustuck || u.utrap || Punished
        || !test_move(u.ux, u.uy, mtmp->mx - u.ux, mtmp->my - u.uy,
                      TEST_MOVE)) {
        if (Punished || !(u.uswallow || u.ustuck || u.utrap))
            You("are unable to swing your %s over.", body_part(LEG));
        else
            You("are stuck here for now.");
        return (FALSE);
    }

    /* Is this a valid monster? */
    otmp = which_armor(mtmp, W_SADDLE);
    if (!otmp) {
        pline("%s is not saddled.", Monnam(mtmp));
        return (FALSE);
    }

    ptr = mtmp->data;
    if (touch_petrifies(ptr) && !Stone_resistance) {
        char kbuf[BUFSZ];

        You("touch %s.", mon_nam(mtmp));
        Sprintf(kbuf, "attempting to ride %s",
                an(pmname(mtmp->data, Mgender(mtmp))));
        instapetrify(kbuf);
    }
    if (!mtmp->mtame || mtmp->isminion) {
        pline("I think %s would mind.", mon_nam(mtmp));
        return (FALSE);
    }
    if (mtmp->mtrapped) {
        struct trap *t = t_at(mtmp->mx, mtmp->my);

        You_cant("mount %s while %s's trapped in %s.", mon_nam(mtmp),
                 mhe(mtmp), an(trapname(t->ttyp, FALSE)));
        return (FALSE);
    }
    if (mtmp->mentangled) {
        You_cant("mount %s while %s's entangled.", mon_nam(mtmp),
                 mhe(mtmp));
        return (FALSE);
    }

    /* Knights will not decrease the tameness of their steed when
       mounting them. The same is true for any role whose steed is
       wearing Ithilmar (artifact barding) */
    barding = which_armor(mtmp, W_BARDING);
    if (!force && !(Role_if(PM_KNIGHT)
                    || (barding && barding->oartifact == ART_ITHILMAR))
        && !(--mtmp->mtame)) {
        /* no longer tame */
        newsym(mtmp->mx, mtmp->my);
        pline("%s resists%s!", Monnam(mtmp),
              mtmp->mleashed ? " and its leash comes off" : "");
        if (mtmp->mleashed)
            m_unleash(mtmp, FALSE);
        return (FALSE);
    }
    if (!force && Underwater && !is_swimmer(ptr)) {
        You_cant("ride that creature while under %s.",
                 hliquid("water"));
        return (FALSE);
    }
    if (!can_saddle(mtmp) || !can_ride(mtmp)) {
        You_cant("ride such a creature.");
        return FALSE;
    }

    /* Is the player impaired? */
    if (!force && !mon_airborne(mtmp) && Levitation && !Lev_at_will) {
        You("cannot reach %s.", mon_nam(mtmp));
        return (FALSE);
    }
    if (!force && uarm && is_metallic(uarm) && greatest_erosion(uarm)) {
        Your("%s armor is too stiff to be able to mount %s.",
             uarm->oeroded ? "rusty" : "corroded", mon_nam(mtmp));
        return (FALSE);
    }
    /* A Knight should be able to ride his own horse!
       so we get a bonus for all horse-like things */
    role_modifier = (Role_if(PM_KNIGHT) && mtmp->data->mlet == S_UNICORN)
                    ? 10 : 0;
    if (!force
        && (Confusion || Fumbling || Glib || Wounded_legs || otmp->cursed
            || otmp->greased
            || (u.ulevel + mtmp->mtame + role_modifier
                < rnd(MAXULEV / 2 + 5)))) {
        if (Levitation) {
            pline("%s slips away from you.", Monnam(mtmp));
            return FALSE;
        }
        You("slip while trying to get on %s.", mon_nam(mtmp));

        Sprintf(buf, "slipped while mounting %s",
                /* "a saddled mumak" or "a saddled pony called Dobbin" */
                x_monnam(mtmp, ARTICLE_A, (char *) 0,
                         SUPPRESS_IT | SUPPRESS_INVISIBLE
                             | SUPPRESS_HALLUCINATION,
                         TRUE));
        losehp(Maybe_Half_Phys(rn1(5, 10)), buf, NO_KILLER_PREFIX);
        return (FALSE);
    }

    /* Success */
    maybewakesteed(mtmp);
    if (!force) {
        if (Levitation && !mon_airborne(mtmp))
            /* Must have Lev_at_will at this point */
            pline("%s magically floats up!", Monnam(mtmp));
        You("mount %s.", mon_nam(mtmp));
        if (Flying)
            You("and %s take flight together.", mon_nam(mtmp));
    }
    /* setuwep handles polearms differently when you're mounted */
    if (uwep && is_pole(uwep))
        gu.unweapon = FALSE;
    u.usteed = mtmp;
    {
        boolean was_stealthy = Stealth != 0;

        steed_vs_stealth();
        if (was_stealthy && !Stealth)
            You("aren't stealthy anymore.");
    }
    remove_monster(mtmp->mx, mtmp->my);
    teleds(mtmp->mx, mtmp->my, TELEDS_ALLOW_DRAG);
    disp.botl = TRUE;
    /* riding a strong steed raises weight_cap(), which the persistent
       inventory window prints; the status line refresh above does not
       reach it */
    update_inventory();
    return TRUE;
}

/* You and your steed have moved */
void
exercise_steed(void)
{
    if (!u.usteed)
        return;

    /* It takes many turns of riding to exercise skill */
    if (++u.urideturns >= 100) {
        u.urideturns = 0;
        use_skill(P_RIDING, 1);
    }
    return;
}

/* The player kicks or whips the steed */
void
kick_steed(void)
{
    char He[BUFSZ]; /* monverbself() appends to the "He"/"She"/"It" value */
    if (!u.usteed)
        return;

    /* [ALI] Various effects of kicking sleeping/paralyzed steeds */
    if (helpless(u.usteed)) {
        /* We assume a message has just been output of the form
         * "You kick <steed>."
         */
        Strcpy(He, mhe(u.usteed));
        *He = highc(*He);
        if ((u.usteed->mcanmove || u.usteed->mfrozen) && !rn2(2)) {
            if (u.usteed->mcanmove)
                u.usteed->msleeping = 0;
            else if (u.usteed->mfrozen > 2)
                u.usteed->mfrozen -= 2;
            else {
                u.usteed->mfrozen = 0;
                if (!u.usteed->mstone || u.usteed->mstone > 2)
                    u.usteed->mcanmove = 1;
            }
            if (helpless(u.usteed))
                pline("%s stirs.", He);
            else
                /* if hallucinating, might yield "He rouses herself" or
                   "She rouses himself" */
                pline("%s!", monverbself(u.usteed, He, "rouse", (char *) 0));
        } else
            pline("%s does not respond.", He);
        return;
    }

    /* Make the steed less tame and check if it resists */
    if (u.usteed->mtame)
        u.usteed->mtame--;
    if (!u.usteed->mtame && u.usteed->mleashed)
        m_unleash(u.usteed, TRUE);
    if (!u.usteed->mtame
        || (u.ulevel + u.usteed->mtame < rnd(MAXULEV / 2 + 5))) {
        newsym(u.usteed->mx, u.usteed->my);
        dismount_steed(DISMOUNT_THROWN);
        return;
    }

    pline("%s gallops!", Monnam(u.usteed));
    u.ugallop += rn1(20, 30);
    return;
}

/*
 * Try to find a dismount point adjacent to the steed's location.
 * If all else fails, try enexto().  Use enexto() as a last resort because
 * enexto() chooses its point randomly, possibly even outside the
 * room's walls, which is not what we want.
 * Adapted from mail daemon code.
 */
staticfn boolean
landing_spot(
    coord *spot, /* landing position (we fill it in) */
    int reason,
    int forceit)
{
    coord cc, try[8]; /* 8: the 8 spots adjacent to the hero's spot */
    int i, j, best_j, clockwise_j, counterclk_j,
        n, viable, distance, min_distance = -1;
    coordxy x, y;
    boolean found, impaird, kn_trap, boulder;
    struct trap *t;

    (void) memset((genericptr_t) try, 0, sizeof try);
    n = 0;
    j = xytodir(u.dx, u.dy);
    if (reason == DISMOUNT_KNOCKED && j != DIR_ERR) {
        /* we'll check preferred location first; if viable it'll be picked */
        best_j = j;
        try[0].x = u.dx, try[0].y = u.dy;
        /* the two next best locations are checked second and third */
        i = rn2(2);
        clockwise_j = DIR_RIGHT(j); /* (j + 1) % 8 */
        dirtocoord(&cc, clockwise_j);
        try[1 + i].x = cc.x, try[1 + i].y = cc.y; /* [1] or [2] */
        counterclk_j = DIR_LEFT(j); /* (j + 8 - 1) % 8 */
        dirtocoord(&cc, counterclk_j);
        try[2 - i].x = cc.x, try[2 - i].y = cc.y; /* [2] or [1] */
        n = 3;
        debugpline3("knock from saddle: best %s, next %s or %s",
                    directionname(best_j),
                    directionname(clockwise_j), directionname(counterclk_j));
    } else {
        best_j = clockwise_j = counterclk_j = -1;
    }
    for (j = 0; j < N_DIRS; ++j) {
        /* fortunately NODIAG() handling isn't needed for DISMOUNT_KNOCKED
           because hero can only ride when humanoid */
        if (j == best_j || j == clockwise_j || j == counterclk_j)
            continue;
        /* j==0 is W, j==1 NW, j==2 N, j==3 NE, ..., around to j==7 SW;
           so odd j values are diagonal directions here */
        if (reason == DISMOUNT_POLY && NODIAG(u.umonnum) && (j % 1) != 0)
            continue;
        dirtocoord(&cc, j);
        try[n++] = cc;
    }

    /*
     * Up to three passes;
     * i==0: voluntary dismount without impairment avoids known traps and
     *       boulders;
     * i==1: voluntary dismount with impairment or knocked out of saddle
     *       avoids boulders but allows known traps;
     * i==2: other, allow traps and boulders.
     *
     * Fallback to i==1 if nothing appropriate was found for i==0 and
     * to i==2 as last resort.
     */
    impaird = (Stunned || Confusion || Fumbling);
    viable = 0;
    found = FALSE;
    for (i = (reason == DISMOUNT_BYCHOICE && !impaird) ? 0
             : ((reason == DISMOUNT_BYCHOICE && impaird)
                || reason == DISMOUNT_KNOCKED) ? 1
               : 2;
         i <= 2 && !found; ++i) {
        for (j = 0; j < n; ++j) {
            x = u.ux + try[j].x;
            y = u.uy + try[j].y;
            if (!isok(x, y) || u_at(x, y)) /* [note: u_at() can't happen] */
                continue;

            if (accessible(x, y) && !MON_AT(x, y)
                && test_move(u.ux, u.uy, x - u.ux, y - u.uy, TEST_MOVE)) {
                ++viable;
                distance = distu(x, y);
                if (min_distance < 0 /* no viable candidate yet */
                    /* or better than pending candidate (note: orthogonal
                       spots are distance 1 and diagonal ones distance 2;
                       treating one as better than the other is arbitrary
                       and not wanted for DISMOUNT_KNOCKED) */
                    || ((best_j == -1) ? (distance < min_distance) : (j < 3))
                    /* or equally good, maybe substitute this one */
                    || (distance == min_distance && !rn2(viable))) {
                    /* traps avoided on pass 0; boulders avoided on 0 and 1 */
                    kn_trap = i == 0 && ((t = t_at(x, y)) != 0 && t->tseen
                                         && t->ttyp != VIBRATING_SQUARE);
                    boulder = i <= 1
                              && (sobj_at(BOULDER, x, y)
                                  && !racial_throws_rocks(&gy.youmonst));
                    if (!kn_trap && !boulder) {
                        spot->x = x;
                        spot->y = y;
                        min_distance = distance;
                        found = TRUE;
                        if (best_j != -1 && j < 3)
                            /* since best_j is first candidate (j==0), j==1
                               and j==2 can only get here when best_j was
                               not viable; 50:50 chance for clockwise_j to
                               come before counterclk_j so each has same
                               chance to be next after best_j */
                            break;
                    }
                }
            }
        }
    }

    /* If we didn't find a good spot and forceit is on, try enexto(). */
    if (forceit && !found)
        found = enexto(spot, u.ux, u.uy, gy.youmonst.data);

    return found;
}

/* Stop riding the current steed */
void
dismount_steed(
    int reason) /* Player was thrown off etc. */
{
    struct monst *mtmp;
    struct obj *otmp;
    const char *verb;
    coord cc, steedcc;
    unsigned save_utrap = u.utrap;
    boolean ulev, ufly,
            repair_leg_damage = (Wounded_legs != 0L),
            have_spot = landing_spot(&cc, reason, 0);

    mtmp = u.usteed; /* make a copy of steed pointer */
    /* Sanity check */
    if (!mtmp) /* Just return silently */
        return;
    u.usteed = 0; /* affects Fly test; could hypothetically affect Lev;
                   * also affects u_locomotion() */
    ufly = Flying ? TRUE : FALSE;
    ulev = Levitation ? TRUE : FALSE;
    verb = u_locomotion("fall"); /* only used for _FELL and _KNOCKED */
    u.usteed = mtmp;

    /* Check the reason for dismounting */
    otmp = which_armor(mtmp, W_SADDLE);
    switch (reason) {
    case DISMOUNT_THROWN:
        verb = "are thrown";
        FALLTHROUGH;
        /*FALLTHRU*/
    case DISMOUNT_KNOCKED:
    case DISMOUNT_FELL:
        You("%s off of %s!", verb, mon_nam(mtmp));
        if (!have_spot)
            have_spot = landing_spot(&cc, reason, 1);
        /* a hero in the midst of a successful prayer is unharmed */
        if (!ulev && !ufly && !u.uinvulnerable) {
            losehp(Maybe_Half_Phys(rn1(10, 10)), "riding accident",
                   KILLED_BY_AN);
            set_wounded_legs(BOTH_SIDES, (int) HWounded_legs + rn1(5, 5));
            repair_leg_damage = FALSE;
        }
        break;
    case DISMOUNT_POLY:
        You("can no longer ride %s.", mon_nam(mtmp));
        if (!have_spot)
            have_spot = landing_spot(&cc, reason, 1);
        break;
    case DISMOUNT_ENGULFED:
        /* caller displays message */
        break;
    case DISMOUNT_BONES:
        /* hero has just died... */
        break;
    case DISMOUNT_GENERIC:
        /* no messages, just make it so */
        break;
    case DISMOUNT_BYCHOICE:
    default:
        if (otmp && otmp->cursed) {
            You("can't.  The saddle %s cursed.",
                otmp->bknown ? "is" : "seems to be");
            otmp->bknown = 1; /* ok to skip set_bknown() here */
            return;
        }
        if (!have_spot) {
            You("can't.  There isn't anywhere for you to stand.");
            return;
        }
        if (!has_mgivenname(mtmp)) {
            pline("You've been through the dungeon on %s with no name.",
                  an(pmname(mtmp->data, Mgender(mtmp))));
            if (Hallucination)
                pline("It felt good to get out of the rain.");
        } else
            You("dismount %s.", mon_nam(mtmp));
    }
    /* While riding, Wounded_legs refers to the steed's legs;
       after dismounting, it reverts to the hero's legs. */
    if (repair_leg_damage)
        heal_legs(1);

    /* Release the steed */
    u.usteed = (struct monst *) NULL;
    u.ugallop = 0L;
    {
        boolean was_stealthy = Stealth != 0;

        steed_vs_stealth();
        if (Stealth && !was_stealthy)
            You("seem less noisy now.");
    }

    if (u.utraptype == TT_BEARTRAP
        || u.utraptype == TT_PIT
        || u.utraptype == TT_WEB) {
        mtmp->mtrapped = 1;
    }

    /*
     * rloc(), rloc_to(), and monkilled()->mondead()->m_detach() all
     * expect mtmp to be on the map or else have mtmp->mx be 0, but
     * setting the latter to 0 here would interfere with dropping
     * the saddle.  Prior to 3.6.2, being off the map didn't matter.
     *
     * place_monster() expects mtmp to be alive and not be u.usteed.
     *
     * Unfortunately, <u.ux,u.uy> (former steed's implicit location)
     * might now be occupied by an engulfer, so we can't just put mtmp
     * at that spot.  An engulfer's previous spot will be unoccupied
     * but we don't know where that was and even if we did, it might
     * be hostile terrain.
     */
    steedcc.x = u.ux, steedcc.y = u.uy;
    if (m_at(u.ux, u.uy)) {
        /* hero's spot has a monster in it; hero must have been plucked
           from saddle as engulfer moved into his spot--other dismounts
           shouldn't run into this situation; find nearest viable spot */
        if (!enexto_core_mon(&steedcc, u.ux, u.uy, mtmp, NO_MM_FLAGS)
            /* no spot? must have been engulfed by a lurker-above over
               water or lava; try requesting a location for a flyer */
            && !enexto(&steedcc, u.ux, u.uy, &mons[PM_BAT])
            /* still no spot; last resort is any spot within bounds */
            && !enexto(&steedcc, u.ux, u.uy, &mons[PM_GHOST])) {
            /* nowhere at all to place the steed; dismiss it so the
               !DEADMONSTER wrapper below skips placement safely */
            mongone(mtmp);
        }
    }

    if (!DEADMONSTER(mtmp)) {
        gi.in_steed_dismounting++;
        place_monster(mtmp, steedcc.x, steedcc.y);
        gi.in_steed_dismounting--;

        /* if for bones, there's no reason to place the hero;
           we want to make room for potential ghost, so move steed */
        if (reason == DISMOUNT_BONES) {
            /* move the steed to an adjacent square */
            if (enexto_core_mon(&cc, u.ux, u.uy, mtmp, NO_MM_FLAGS))
                rloc_to(mtmp, cc.x, cc.y);
            else /* evidently no room nearby; move steed elsewhere */
                (void) rloc(mtmp, RLOC_ERR | RLOC_NOMSG);
            return;
        }

        /* Set hero's and/or steed's positions.  Usually try moving the
           hero first.  Note: for DISMOUNT_ENGULFED, caller hasn't set
           u.uswallow yet but has set u.ustuck. */
        if (!u.uswallow && !u.ustuck && have_spot) {
            struct permonst *mdat = mtmp->data;

            /* The steed may drop into water/lava */
            if (grounded(mdat) && !can_levitate(mtmp) && !can_fly(mtmp)) {
                if (is_pool(u.ux, u.uy)) {
                    /* the dramatic message belongs to the steed that
                       drowns; one that swims gets a neutral one */
                    if (!cant_drown(mdat)) {
                        if (!Underwater)
                            pline("%s falls into the %s!", Monnam(mtmp),
                                  surface(u.ux, u.uy));
                        /* hero gets credit/blame only when player chose
                           to dismount; accidental drops are silent kills
                           with no XP, mirroring the no-room branch */
                        if (reason == DISMOUNT_BYCHOICE)
                            killed(mtmp);
                        else
                            monkilled(mtmp, "", -AD_PHYS);
                        You_feel("guilty.");
                        adjalign(-1);
                        record_abuse_event_dtl(-1, ABUSE_STEED_DEATH,
                                               monsndx(mdat) + 1,
                                               ABUSE_HAZ_WATER);
                    } else if (!Underwater) {
                        pline("%s splashes into the %s.", Monnam(mtmp),
                              surface(u.ux, u.uy));
                    }
                } else if (is_lava(u.ux, u.uy)) {
                    if (!likes_lava(mdat)) {
                        pline("%s is pulled into the %s!", Monnam(mtmp),
                              hliquid("lava"));
                        if (reason == DISMOUNT_BYCHOICE)
                            killed(mtmp);
                        else
                            monkilled(mtmp, "", -AD_PHYS);
                        You_feel("guilty.");
                        adjalign(-1);
                        record_abuse_event_dtl(-1, ABUSE_STEED_DEATH,
                                               monsndx(mdat) + 1,
                                               ABUSE_HAZ_LAVA);
                    }
                } else if (is_open_air(u.ux, u.uy)) {
                    pline("%s plummets several thousand feet to %s death.",
                          Monnam(mtmp), mhis(mtmp));
                    /* no corpse or objects as both are now several
                       thousand feet down */
                    mongone(mtmp);
                    You_feel("guilty.");
                    adjalign(-1);
                    record_abuse_event_dtl(-1, ABUSE_STEED_DEATH,
                                           monsndx(mdat) + 1,
                                           ABUSE_HAZ_FALL);
                }
            }
            /* Steed dismounting consists of two steps: being moved to another
             * square, and descending to the floor.  We have functions to do
             * each of these activities, but they're normally called
             * individually and include an attempt to look at or pick up the
             * objects on the floor:
             * teleds() --> spoteffects() --> pickup()
             * float_down() --> pickup()
             * We use this kludge to make sure there is only one such attempt.
             *
             * Clearly this is not the best way to do it.  A full fix would
             * involve having these functions not call pickup() at all,
             * instead calling them first and calling pickup() afterwards.
             * But it would take a lot of work to keep this change from
             * having any unforeseen side effects (for instance, you would
             * no longer be able to walk onto a square with a hole, and
             * autopickup before falling into the hole).
             */
            /* [ALI] No need to move the player if the steed died. */
            if (!DEADMONSTER(mtmp)) {
                /* Keep steed here, move the player to cc;
                 * teleds() clears u.utrap
                 */
                gi.in_steed_dismounting = TRUE;
                teleds(cc.x, cc.y, TELEDS_ALLOW_DRAG);
                if (sobj_at(BOULDER, cc.x, cc.y))
                    sokoban_guilt();
                gi.in_steed_dismounting = FALSE;

                /* Put your steed in your trap */
                if (save_utrap)
                    (void) mintrap(mtmp, NO_TRAP_FLAGS);
            }

        /* Couldn't move hero... try moving the steed. */
        } else if (enexto_core_mon(&cc, u.ux, u.uy, mtmp, NO_MM_FLAGS)) {
            /* Keep player here, move the steed to cc */
            rloc_to(mtmp, cc.x, cc.y);
            /* Player stays put */

        /* Otherwise, steed goes bye-bye. */
        } else {
#if 1       /* original there's-no-room handling */
            if (reason == DISMOUNT_BYCHOICE) {
                int steed_mndx = monsndx(mtmp->data);

                /* [un]#ride: hero gets credit/blame for killing steed */
                killed(mtmp);
                You_feel("guilty.");
                adjalign(-1);
                record_abuse_event_dtl(-1, ABUSE_STEED_DEATH,
                                       steed_mndx + 1, 0);
            } else {
                /* other dismount: kill former steed with no penalty;
                   damage type is just "neither AD_DGST nor -AD_RBRE" */
                monkilled(mtmp, "", -AD_PHYS);
            }
#else
            /* Can't use this [yet?] because it violates monmove()'s
             * assumption that a moving monster (engulfer) can't cause
             * another monster (steed) to be removed from the fmon list.
             * That other monster (steed) might be cached as the next one
             * to move.
             */
            /* migrate back to this level if hero leaves and returns
               or to next level if it is happening in the endgame */
            mdrop_special_objs(mtmp);
            deal_with_overcrowding(mtmp);
#endif
        }
    } /* !DEADMONST(mtmp) */

    /* usually return the hero to the surface */
    if (reason != DISMOUNT_ENGULFED && reason != DISMOUNT_BONES) {
        gi.in_steed_dismounting = TRUE;
        (void) float_down(0L, W_SADDLE);
        gi.in_steed_dismounting = FALSE;
        disp.botl = TRUE;
        encumber_msg();
        gv.vision_full_recalc = 1;
    } else
        disp.botl = TRUE;
    /* polearms behave differently when not mounted */
    if (uwep && is_pole(uwep))
        gu.unweapon = TRUE;
    /* weight_cap() drops back to the hero's own carrying capacity, so the
       persistent inventory window needs the same refresh the status line
       just got */
    update_inventory();
    return;
}

/* A steed can stop being tame while the hero still rides it: an amulet of
   life saving untames it in the middle of its death (wary_dog()), where
   throwing the hero off could kill it a second time, and a monster's charm
   or the hero's knock can untame it outright. moveloop and movemon call
   this at points where nothing is mid-death and before anything acts on
   the untamed mount; a hero already leaving the level takes the steed
   along and is thrown off on arrival. Returns TRUE if the hero was
   thrown */
boolean
dismount_untamed_steed(void)
{
    if (!u.usteed || u.usteed->mtame || DEADMONSTER(u.usteed) || u.utotype)
        return FALSE;
    dismount_steed(DISMOUNT_THROWN);
    return TRUE;
}

/* when attempting to saddle or mount a sleeping steed, try to wake it up
   (for the saddling case, it won't be u.usteed yet) */
staticfn void
maybewakesteed(struct monst *steed)
{
    int frozen = (int) steed->mfrozen;
    boolean wasimmobile = helpless(steed);

    steed->msleeping = 0;
    if (frozen) {
        frozen = (frozen + 1) / 2; /* half */
        /* might break out of timed sleep or paralysis */
        if (!rn2(frozen)) {
            steed->mfrozen = 0;
            /* breaking out of sleep or paralysis must not undo the
               paralysis of a steed that is turning to stone */
            if (!steed->mstone || steed->mstone > 2)
                steed->mcanmove = 1;
        } else {
            /* didn't awake, but remaining duration is halved */
            steed->mfrozen = frozen;
        }
    }
    if (wasimmobile && !helpless(steed))
        pline("%s wakes up.", Monnam(steed));
    /* regardless of waking, terminate any meal in progress */
    finish_meating(steed);
}

/* the hero's steed may have gained or lost the flight or floating that
   carries its rider (worn gear, a spell running out, a new shape);
   was_airborne is the lift it had before */
void
steed_lift_changed(struct monst *mon, boolean was_airborne)
{
    struct trap *ttmp;

    if (mon != u.usteed || !was_airborne == !mon_airborne(mon))
        return;
    disp.botl = TRUE; /* status: 'Fly' */
    /* riding blocks stealth unless hero+steed fly */
    steed_vs_stealth();
    /* the rider hovered over a floor trap; with the steed's lift gone
       and nothing else keeping the hero up, the pair comes down onto it.
       Water, lava and open air are the steed's own terrain check; traps
       that catch flyers were met on arrival, and a hold had already
       brought the pair down. Silent, like polyman()'s riding landing:
       the trap tells it */
    if (was_airborne && !Flying && !Levitation && !u_held_down()
        && (ttmp = t_at(u.ux, u.uy)) != 0 && floor_trigger(ttmp->ttyp))
        defer_landing(LANDING_SPOT);
}

/* steed has taken on a new shape */
void
poly_steed(
    struct monst *steed,
    struct permonst *oldshape)
{
    if (!can_saddle(steed) || !can_ride(steed)) {
        /* can't get here; newcham() -> mon_break_armor() -> m_lose_armor()
           removes saddle and/or forces hero to dismount, if applicable,
           before newcham() calls us */
        dismount_steed(DISMOUNT_FELL);
    } else {
        char buf[BUFSZ];
        /* a worn amulet and spell lift survive newcham(); armor it
           strips was told of by update_mon_extrinsics() */
        boolean was_airborne = (is_flyer(oldshape) || is_floater(oldshape)
                                || can_fly(steed) || can_levitate(steed));

        Strcpy(buf, x_monnam(steed, ARTICLE_YOUR, (char *) 0,
                             (SUPPRESS_SADDLE | SUPPRESS_BARDING), FALSE));
        if (oldshape != steed->data)
            (void) strsubst(buf, "your ", "your new ");
        You("adjust yourself in the saddle on %s.", buf);
        /* status, stealth and a landing on a floor trap if the new form
           lost the steed's wings */
        steed_lift_changed(steed, was_airborne);
    }
}

/* decide whether hero's steed is able to move;
   doesn't check for holding traps--those affect the hero directly */
boolean
stucksteed(boolean checkfeeding)
{
    struct monst *steed = u.usteed;

    if (steed) {
        /* check whether steed can move */
        if (helpless(steed)) {
            pline("%s won't move!", YMonnam(steed));
            return TRUE;
        }
        /* optionally check whether steed is in the midst of a meal */
        if (checkfeeding && steed->meating) {
            pline("%s is still eating.", YMonnam(steed));
            return TRUE;
        }
    }
    return FALSE;
}

void
place_monster(struct monst *mon, coordxy x, coordxy y)
{
    struct monst *othermon;
    const char *monnm, *othnm;
    char buf[QBUFSZ];

    buf[0] = '\0';
    /* normal map bounds are <1..COLNO-1,0..ROWNO-1> but sometimes
       vault guards (either living or dead) are parked at <0,0>;
       their mstate should have the MON_PARKED bit set (post-5.0.0) */
    if (!isok(x, y)
        && !(((mon->mstate & MON_PARKED) != 0) || PARKEDMONSTER(mon))) {
        describe_level(buf, 0);
        impossible("trying to place %s at <%d,%d> mstate:%lx on %s",
                   minimal_monnam(mon, TRUE), x, y, mon->mstate, buf);
        x = y = 0;
    }
    if ((mon == u.usteed && !gi.in_steed_dismounting)
        /* special case is for convoluted vault guard handling */
        || (DEADMONSTER(mon) && !PARKEDMONSTER(mon))) {
        describe_level(buf, 0);
        impossible("placing %s onto map, mstate:%lx, on %s?",
                   (mon == u.usteed) ? "steed" : "defunct monster",
                   mon->mstate, buf);
        return;
    }
    if ((othermon = svl.level.monsters[x][y]) != 0
        /* a monster steed and its rider are colocated, so allow placing
           one on top of the other */
        && !((has_erid(othermon) && ERID(othermon)->mon_steed == mon)
             || (has_erid(mon) && ERID(mon)->mon_steed == othermon))) {
        describe_level(buf, 0);
        monnm = minimal_monnam(mon, FALSE);
        othnm = (mon != othermon) ? minimal_monnam(othermon, TRUE) : "itself";
        impossible("placing %s over %s at <%d,%d>, mstates:%lx %lx on %s?",
                   monnm, othnm, x, y, othermon->mstate, mon->mstate, buf);
    }
    mon->mx = x, mon->my = y;
    svl.level.monsters[x][y] = mon;
    /* even though MON_FLOOR is not actually a bit currently
       (MON_FLOOR == 0) we want to preserve some of the other
       bits that may be set. We'll probably make MON_FLOOR an
       actual bit one day */

    mon->mstate &= ~(MON_OFF_MAP_BITS);

    /* We don't mess with these bits above:
       MON_OBLITERATE | MON_STILL_ARRIVING
     */
}

/*steed.c*/
