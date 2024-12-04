/* NetHack 3.6	steed.c	$NHDT-Date: 1575245090 2019/12/02 00:04:50 $  $NHDT-Branch: NetHack-3.6 $:$NHDT-Revision: 1.68 $ */
/* Copyright (c) Kevin Hugo, 1998-1999. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* Monsters that might be ridden */
static NEARDATA const char steeds[] = { S_QUADRUPED, S_UNICORN, S_ANGEL,
                                        S_CENTAUR,   S_DRAGON,  S_JABBERWOCK,
                                        S_DOG,       S_FELINE,  S_SPIDER,
                                        S_LIZARD,    '\0' };

/* Monsters that might wear barding */
static NEARDATA const char mbarding[] = { S_QUADRUPED, S_UNICORN,    S_ANGEL,
                                          S_DRAGON,    S_JABBERWOCK, S_DOG,
                                          S_FELINE,    S_SPIDER,     S_LIZARD,
                                          '\0' };

STATIC_DCL boolean FDECL(landing_spot, (coord *, int, int));
STATIC_DCL void FDECL(maybewakesteed, (struct monst *));

/* caller has decided that hero can't reach something while mounted */
void
rider_cant_reach()
{
    You("aren't skilled enough to reach from %s.", y_monnam(u.usteed));
}

void update_monsteed(mtmp)
struct monst *mtmp;
{
    if (has_erid(mtmp)) {
        ERID(mtmp)->mon_steed->mx = mtmp->mx;
        ERID(mtmp)->mon_steed->my = mtmp->my;
        ERID(mtmp)->mon_steed->mpeaceful = mtmp->mpeaceful;
    }
}

void mount_monster(mtmp, pm)
struct monst *mtmp;
int pm;
{
    register struct monst *mount;

    /* small hack here: make it in a random spot to avoid failures due to there
       not being enough room. */
    mount = makemon(&mons[pm], 0, 0, MM_ADJACENTOK);
    if (!mount || is_covetous(mount->data)) {
        return;
    } else {
        remove_monster(mount->mx, mount->my);
        newsym(mount->mx, mount->my);
    }
    newerid(mtmp);
    ERID(mtmp)->mon_steed = mount;
    ERID(mtmp)->mid = mount->m_id;
    ERID(mtmp)->mon_steed->ridden_by = mtmp->m_id;
    ERID(mtmp)->mon_steed->mx = mtmp->mx;
    ERID(mtmp)->mon_steed->my = mtmp->my;
    newsym(mtmp->mx, mtmp->my);

    /* rider over'rides' horse's natural inclinations */
    mount->mpeaceful = mtmp->mpeaceful;

    /* monster steeds will sometimes come with a saddle */
    if (!rn2(3) && can_saddle(mount) && !which_armor(mtmp, W_SADDLE)) {
        struct obj *otmp = mksobj(SADDLE, TRUE, FALSE);
        put_saddle_on_mon(otmp, mount);
    }

    /* if the monster steed has a saddle, there's a chance it's wearing
       barding also */
    if (!rn2(10) && which_armor(mount, W_SADDLE)) {
        if (can_wear_barding(mount) && !which_armor(mtmp, W_BARDING)) {
            struct obj *otmp = mksobj(rn2(4) ? BARDING
                                             : rn2(3) ? SPIKED_BARDING
                                                      : BARDING_OF_REFLECTION, TRUE, FALSE);
            put_barding_on_mon(otmp, mount);
        }
    }
}

boolean mount_up(rider)
struct monst *rider;
{
    register struct monst *steed, *nmon;

    /* not acceptable as riders */
    if (!mon_can_ride(rider) || has_erid(rider))
        return FALSE;

    for (steed = fmon; steed; steed = nmon) {
        nmon = steed->nmon;
        if (nmon == rider)
            nmon = rider->nmon;
        /* criteria for an acceptable steed */
        if (!is_drow(rider->data) && is_spider(steed->data))
            continue;
        if (!is_drow(rider->data)
            && (steed->data == &mons[PM_CAVE_LIZARD]
                || steed->data == &mons[PM_LARGE_CAVE_LIZARD]))
            continue;
        if (!is_orc(rider->data) && steed->data == &mons[PM_WARG])
            continue;
        if (!(rider->data == &mons[PM_CAVEMAN]
              || rider->data == &mons[PM_CAVEWOMAN])
            && steed->data == &mons[PM_SABER_TOOTHED_TIGER])
            continue;
        if (monnear(rider, steed->mx, steed->my) && mon_can_be_ridden(steed)
            && !steed->ridden_by) {
            break;
        }
    }
    if (!steed)
        return FALSE;
    if (canseemon(rider)) {
        pline("%s clambers onto %s!", Monnam(rider),
              canseemon(steed) ?  mon_nam(steed) : "something");
    } else if (!Deaf && distu(rider->mx, rider->my) <= 5) {
        You_hear("someone %s.", Hallucination ? "getting on their high horse"
                                              : "jump into a saddle");
    }
    remove_monster(steed->mx, steed->my);
    newsym(steed->mx, steed->my);
    newerid(rider);
    ERID(rider)->mon_steed = steed;
    ERID(rider)->mid = steed->m_id;
    ERID(rider)->mon_steed->ridden_by = rider->m_id;
    ERID(rider)->mon_steed->mx = rider->mx;
    ERID(rider)->mon_steed->my = rider->my;
    newsym(rider->mx, rider->my);
    return TRUE;
}

void
newerid(mtmp)
struct monst *mtmp;
{
    if (!mtmp->mextra)
        mtmp->mextra = newmextra();
    if (!ERID(mtmp)) {
        ERID(mtmp) = (struct erid *) alloc(sizeof(struct erid));
        (void) memset((genericptr_t) ERID(mtmp), 0, sizeof(struct erid));
    }
}

void
free_erid(mtmp)
struct monst *mtmp;
{
    if (mtmp->mextra && ERID(mtmp)) {
        struct monst *steed = ERID(mtmp)->mon_steed;
        if (steed)
            steed->ridden_by = 0; /* Remove pointer to monster in steed */
        free((genericptr_t) ERID(mtmp));
        ERID(mtmp) = (struct erid *) 0;
    }
}

void
separate_steed_and_rider(rider)
struct monst *rider;
{
    struct monst *steed;
    coord cc;

    if (!has_erid(rider))
        return;

    steed = ERID(rider)->mon_steed;
    free_erid(rider);

    /* handle rider if both rider and steed are alive */
    if (!DEADMONSTER(rider) && !DEADMONSTER(steed)) {
        xchar orig_x = rider->mx, orig_y = rider->my; /* cache riders position */

        /* move rider to an adjacent tile */
        if (enexto(&cc, rider->mx, rider->my, rider->data))
            rloc_to(rider, cc.x, cc.y);
        else /* evidently no room nearby; move rider elsewhere */
            (void) rloc(rider, FALSE);
        place_monster(steed, orig_x, orig_y);
    }
    /* place rider if steed dies and rider is still alive */
    if (!DEADMONSTER(rider) && DEADMONSTER(steed)) {
        remove_monster(steed->mx, steed->my); /* remove pointer to steed */
        place_monster(rider, rider->mx, rider->my);
    }
    /* place steed if rider dies and steed is still alive */
    if (!DEADMONSTER(steed) && DEADMONSTER(rider))
        place_monster(steed, steed->mx, steed->my);

    update_monster_region(rider);
    update_monster_region(steed);
    newsym(rider->mx, rider->my);
    newsym(steed->mx, steed->my);
}

struct monst*
get_mon_rider(mtmp)
struct monst *mtmp;
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

/*** Putting the saddle on ***/

/* Can this monster wear a saddle? */
boolean
can_saddle(mtmp)
struct monst *mtmp;
{
    struct permonst *ptr = r_data(mtmp);

    return (index(steeds, ptr->mlet) && (ptr->msize >= MZ_MEDIUM)
            && (!humanoid(ptr) || ptr->mlet == S_CENTAUR) && !amorphous(ptr)
            && !noncorporeal(ptr) && !is_whirly(ptr) && !unsolid(ptr)
            && !(ptr->mlet == S_JABBERWOCK
                 && mtmp->mnum != PM_JABBERWOCK
                 && mtmp->mnum != PM_VORPAL_JABBERWOCK)
            && !(ptr->mlet == S_DOG && mtmp->mnum != PM_WARG)
            && !(ptr->mlet == S_SPIDER
                 && mtmp->mnum != PM_GIANT_SPIDER
                 && mtmp->mnum != PM_GARGANTUAN_SPIDER)
            && !(ptr->mlet == S_LIZARD
                 && mtmp->mnum != PM_CAVE_LIZARD
                 && mtmp->mnum != PM_LARGE_CAVE_LIZARD)
            && !(ptr->mlet == S_FELINE && mtmp->mnum != PM_SABER_TOOTHED_TIGER));
}

/* Can this monster wear barding? */
boolean
can_wear_barding(mtmp)
struct monst *mtmp;
{
    struct permonst *ptr = r_data(mtmp);

    return (index(mbarding, ptr->mlet) && (ptr->msize >= MZ_MEDIUM)
            && !humanoid(ptr) && !amorphous(ptr)
            && !noncorporeal(ptr) && !is_whirly(ptr) && !unsolid(ptr)
            && !(ptr->mlet == S_JABBERWOCK
                 && mtmp->mnum != PM_JABBERWOCK
                 && mtmp->mnum != PM_VORPAL_JABBERWOCK)
            && !(ptr->mlet == S_DOG && mtmp->mnum != PM_WARG)
            && !(ptr->mlet == S_SPIDER
                 && mtmp->mnum != PM_GIANT_SPIDER
                 && mtmp->mnum != PM_GARGANTUAN_SPIDER)
            && !(ptr->mlet == S_LIZARD
                 && mtmp->mnum != PM_CAVE_LIZARD
                 && mtmp->mnum != PM_LARGE_CAVE_LIZARD)
            && !(ptr->mlet == S_FELINE && mtmp->mnum != PM_SABER_TOOTHED_TIGER));
}

int
use_saddle(otmp)
struct obj *otmp;
{
    struct monst *mtmp;
    struct permonst *ptr;
    int chance;
    const char *s;

    if (!u_handsy())
        return 0;

    /* Select an animal */
    if (u.uswallow || Underwater || !getdir((char *) 0)) {
        pline1(Never_mind);
        return 0;
    }
    if (!u.dx && !u.dy) {
        pline("Saddle yourself?  Very funny...");
        return 0;
    }
    if (!isok(u.ux + u.dx, u.uy + u.dy)
        || !(mtmp = m_at(u.ux + u.dx, u.uy + u.dy)) || !canspotmon(mtmp)) {
        pline("I see nobody there.");
        return 1;
    }

    /* Is this a valid monster? */
    if ((mtmp->misc_worn_check & W_SADDLE) != 0L
        || which_armor(mtmp, W_SADDLE)) {
        pline("%s doesn't need another one.", Monnam(mtmp));
        return 1;
    }
    ptr = mtmp->data;
    if (touch_petrifies(ptr) && !uarmg && !Stone_resistance) {
        char kbuf[BUFSZ];

        You("touch %s.", mon_nam(mtmp));
        if (!(poly_when_stoned(youmonst.data)
              && (polymon(PM_STONE_GOLEM)
                  || polymon(PM_PETRIFIED_ENT)))) {
            Sprintf(kbuf, "attempting to saddle %s",
                    an(mtmp->data->mname));
            instapetrify(kbuf);
        }
    }
    if (ptr == &mons[PM_INCUBUS] || ptr == &mons[PM_SUCCUBUS]) {
        pline("Shame on you!");
        exercise(A_WIS, FALSE);
        return 1;
    }
    if (mtmp->isminion || mtmp->isshk || mtmp->ispriest || mtmp->isgd
        || mtmp->iswiz) {
        pline("I think %s would mind.", mon_nam(mtmp));
        return 1;
    }
    if (ptr == &mons[PM_WARG] && !Race_if(PM_ORC)) {
        if (!Deaf)
            pline("%s %s menacingly at you!", Monnam(mtmp),
                  rn2(2) ? "snarls" : "growls");
        if ((mtmp->mtame > 0 || mtmp->mpeaceful)
            && !rn2(3)) {
            mtmp->mtame = mtmp->mpeaceful = 0;
            newsym(mtmp->mx, mtmp->my);
        }
        return 1;
    }
    if (ptr == &mons[PM_SABER_TOOTHED_TIGER] && !Role_if(PM_CAVEMAN)) {
        if (!Deaf)
            pline("%s %s menacingly at you!", Monnam(mtmp),
                  rn2(2) ? "snarls" : "growls");
        if ((mtmp->mtame > 0 || mtmp->mpeaceful)
            && !rn2(3)) {
            mtmp->mtame = mtmp->mpeaceful = 0;
            newsym(mtmp->mx, mtmp->my);
        }
        return 1;
    }
    if ((ptr == &mons[PM_GIANT_SPIDER]
         || ptr == &mons[PM_GARGANTUAN_SPIDER]
         || ptr == &mons[PM_CAVE_LIZARD]
         || ptr == &mons[PM_LARGE_CAVE_LIZARD]) && !Race_if(PM_DROW)) {
        if (!Deaf)
            pline("%s hisses threateningly at you!", Monnam(mtmp));
        if ((mtmp->mtame > 0 || mtmp->mpeaceful)
            && !rn2(3)) {
            mtmp->mtame = mtmp->mpeaceful = 0;
            newsym(mtmp->mx, mtmp->my);
        }
        return 1;
    }
    if (!can_saddle(mtmp)) {
        You_cant("saddle such a creature.");
        return 1;
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
    else if (uarmg && (s = OBJ_DESCR(objects[uarmg->otyp])) != (char *) 0
             && !strncmp(s, "riding ", 7))
        /* Bonus for wearing "riding" (but not fumbling) gloves */
        chance += 10;
    else if (uarmf && (s = OBJ_DESCR(objects[uarmf->otyp])) != (char *) 0
             && !strncmp(s, "riding ", 7))
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
        put_saddle_on_mon(otmp, mtmp);
    } else
        pline("%s resists!", Monnam(mtmp));
    return 1;
}

int
use_barding(otmp)
struct obj *otmp;
{
    struct monst *mtmp;
    struct permonst *ptr;
    int chance;
    const char *s;

    if (!u_handsy())
        return 0;

    /* Select an animal */
    if (u.uswallow || Underwater || !getdir((char *) 0)) {
        pline1(Never_mind);
        return 0;
    }
    if (!u.dx && !u.dy) {
        pline("Put barding on yourself?  Very funny...");
        return 0;
    }
    if (!isok(u.ux + u.dx, u.uy + u.dy)
        || !(mtmp = m_at(u.ux + u.dx, u.uy + u.dy)) || !canspotmon(mtmp)) {
        pline("I see nobody there.");
        return 1;
    }

    /* Is this a valid monster? */
    if ((mtmp->misc_worn_check & W_BARDING) != 0L
        || which_armor(mtmp, W_BARDING)) {
        pline("%s doesn't need another one.", Monnam(mtmp));
        return 1;
    }
    ptr = mtmp->data;
    if (touch_petrifies(ptr) && !uarmg && !Stone_resistance) {
        char kbuf[BUFSZ];

        You("touch %s.", mon_nam(mtmp));
        if (!(poly_when_stoned(youmonst.data)
              && (polymon(PM_STONE_GOLEM)
                  || polymon(PM_PETRIFIED_ENT)))) {
            Sprintf(kbuf, "attempting to put barding on %s",
                    an(mtmp->data->mname));
            instapetrify(kbuf);
        }
    }
    if (ptr == &mons[PM_INCUBUS] || ptr == &mons[PM_SUCCUBUS]) {
        pline("This won't accessorize well...");
        exercise(A_WIS, FALSE);
        return 1;
    }
    if (mtmp->isminion || mtmp->isshk || mtmp->ispriest || mtmp->isgd
        || mtmp->iswiz) {
        pline("I think %s would mind.", mon_nam(mtmp));
        return 1;
    }
    if (ptr == &mons[PM_WARG] && !Race_if(PM_ORC)) {
        if (!Deaf)
            pline("%s %s menacingly at you!", Monnam(mtmp),
                  rn2(2) ? "snarls" : "growls");
        if ((mtmp->mtame > 0 || mtmp->mpeaceful)
            && !rn2(3)) {
            mtmp->mtame = mtmp->mpeaceful = 0;
            newsym(mtmp->mx, mtmp->my);
        }
        return 1;
    }
    if (ptr == &mons[PM_SABER_TOOTHED_TIGER] && !Role_if(PM_CAVEMAN)) {
        if (!Deaf)
            pline("%s %s menacingly at you!", Monnam(mtmp),
                  rn2(2) ? "snarls" : "growls");
        if ((mtmp->mtame > 0 || mtmp->mpeaceful)
            && !rn2(3)) {
            mtmp->mtame = mtmp->mpeaceful = 0;
            newsym(mtmp->mx, mtmp->my);
        }
        return 1;
    }
    if ((ptr == &mons[PM_GIANT_SPIDER]
         || ptr == &mons[PM_GARGANTUAN_SPIDER]
         || ptr == &mons[PM_CAVE_LIZARD]
         || ptr == &mons[PM_LARGE_CAVE_LIZARD]) && !Race_if(PM_DROW)) {
        if (!Deaf)
            pline("%s hisses threateningly at you!", Monnam(mtmp));
        if ((mtmp->mtame > 0 || mtmp->mpeaceful)
            && !rn2(3)) {
            mtmp->mtame = mtmp->mpeaceful = 0;
            newsym(mtmp->mx, mtmp->my);
        }
        return 1;
    }
    if (!can_wear_barding(mtmp)) {
        You_cant("put barding on such a creature.");
        return 1;
    }

    /* Calculate your chance (same for using a saddle) */
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
    else if (uarmg && (s = OBJ_DESCR(objects[uarmg->otyp])) != (char *) 0
             && !strncmp(s, "riding ", 7))
        /* Bonus for wearing "riding" (but not fumbling) gloves */
        chance += 10;
    else if (uarmf && (s = OBJ_DESCR(objects[uarmf->otyp])) != (char *) 0
             && !strncmp(s, "riding ", 7))
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
        put_barding_on_mon(otmp, mtmp);
    } else
        pline("%s resists!", Monnam(mtmp));
    return 1;
}

void
put_saddle_on_mon(saddle, mtmp)
struct obj *saddle;
struct monst *mtmp;
{
    if (!can_saddle(mtmp) || which_armor(mtmp, W_SADDLE))
        return;
    if (mpickobj(mtmp, saddle))
        panic("merged saddle?");
    mtmp->misc_worn_check |= W_SADDLE;
    saddle->owornmask = W_SADDLE;
    saddle->leashmon = mtmp->m_id;
    update_mon_intrinsics(mtmp, saddle, TRUE, FALSE);
}

void
put_barding_on_mon(barding, mtmp)
struct obj *barding;
struct monst *mtmp;
{
    if (!can_wear_barding(mtmp) || which_armor(mtmp, W_BARDING))
        return;
    if (mpickobj(mtmp, barding))
        panic("merged barding?");
    mtmp->misc_worn_check |= W_BARDING;
    barding->owornmask = W_BARDING;
    barding->leashmon = mtmp->m_id;
    update_mon_intrinsics(mtmp, barding, TRUE, FALSE);
}

/*** Riding the monster ***/

/* Can we ride this monster?  Caller should also check can_saddle() */
boolean
can_ride(mtmp)
struct monst *mtmp;
{
    return ((mtmp->mtame && humanoid(youmonst.data)
             && !maybe_polyd(is_tortle(youmonst.data), Race_if(PM_TORTLE))
             && !verysmall(youmonst.data)
             && !bigmonst(youmonst.data)
             && (!Underwater || is_swimmer(mtmp->data)))
            || (r_data(mtmp) == &mons[PM_WOOLLY_MAMMOTH] && Race_if(PM_GIANT)));
}

int
doride()
{
    boolean forcemount = FALSE;

    if (u.usteed) {
        dismount_steed(DISMOUNT_BYCHOICE);
    } else if (getdir((char *) 0) && isok(u.ux + u.dx, u.uy + u.dy)) {
        if (wizard && yn("Force the mount to succeed?") == 'y')
            forcemount = TRUE;
        return (mount_steed(m_at(u.ux + u.dx, u.uy + u.dy), forcemount));
    } else {
        return 0;
    }
    return 1;
}

/* Start riding, with the given monster */
boolean
mount_steed(mtmp, force)
struct monst *mtmp; /* The animal */
boolean force;      /* Quietly force this animal */
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
    /* While riding Wounded_legs refers to the steed's,
     * not the hero's legs.
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
        Your("%s are in no shape for riding.", makeplural(body_part(LEG)));
        if (force && wizard && yn("Heal your legs?") == 'y')
            HWounded_legs = EWounded_legs = 0L;
        else
            return (FALSE);
    }

    if (Upolyd && (!humanoid(youmonst.data) || verysmall(youmonst.data)
                   || bigmonst(youmonst.data) || slithy(youmonst.data))) {
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
        Sprintf(kbuf, "attempting to ride %s", an(mtmp->data->mname));
        instapetrify(kbuf);
    }
    if (!mtmp->mtame || mtmp->isminion) {
        pline("I think %s would mind.", mon_nam(mtmp));
        return (FALSE);
    }
    if (mtmp->mtrapped) {
        struct trap *t = t_at(mtmp->mx, mtmp->my);

        You_cant("mount %s while %s's trapped in %s.", mon_nam(mtmp),
                 mhe(mtmp), an(defsyms[trap_to_defsym(t->ttyp)].explanation));
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
    if (!force && !is_floater(ptr) && !is_flyer(ptr)
        && !can_levitate(mtmp) && Levitation
        && !Lev_at_will) {
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
    role_modifier = (Role_if(PM_KNIGHT) && mtmp->data->mlet == S_UNICORN) ? 10 : 0;
    if (!force
        && (Confusion || Fumbling || Glib || Wounded_legs || otmp->cursed
            || (u.ulevel + mtmp->mtame + role_modifier < rnd(MAXULEV / 2 + 5)))) {
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
        if (Levitation && !is_floater(ptr)
            && !is_flyer(ptr) && !can_levitate(mtmp))
            /* Must have Lev_at_will at this point */
            pline("%s magically floats up!", Monnam(mtmp));
        You("mount %s.", mon_nam(mtmp));
        if (Flying)
            You("and %s take flight together.", mon_nam(mtmp));
    }
    /* setuwep handles polearms differently when you're mounted */
    if (uwep && is_pole(uwep))
        unweapon = FALSE;
    u.usteed = mtmp;
    remove_monster(mtmp->mx, mtmp->my);
    teleds(mtmp->mx, mtmp->my, TELEDS_ALLOW_DRAG);
    context.botl = TRUE;
    return TRUE;
}

/* You and your steed have moved */
void
exercise_steed()
{
    if (!u.usteed)
        return;

    /* It takes many turns of riding to exercise skill
     * but we're going to make this a bit more reasonable.
     * Taken from SporkHack. */
    if (u.urideturns++ >= 50) {
        u.urideturns = 0;
        use_skill(P_RIDING, 1);
    }
    return;
}

/* The player kicks or whips the steed */
void
kick_steed()
{
    char He[4];
    if (!u.usteed)
        return;

    /* [ALI] Various effects of kicking sleeping/paralyzed steeds */
    if (u.usteed->msleeping || !u.usteed->mcanmove) {
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
            if (u.usteed->msleeping || !u.usteed->mcanmove)
                pline("%s stirs.", He);
            else
                pline("%s rouses %sself!", He, mhim(u.usteed));
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
STATIC_OVL boolean
landing_spot(spot, reason, forceit)
coord *spot; /* landing position (we fill it in) */
int reason;
int forceit;
{
    int i = 0, x, y, distance, min_distance = -1;
    boolean found = FALSE;
    struct trap *t;

    /* avoid known traps (i == 0) and boulders, but allow them as a backup */
    if (reason != DISMOUNT_BYCHOICE || Stunned || Confusion || Fumbling)
        i = 1;
    for (; !found && i < 2; ++i) {
        for (x = u.ux - 1; x <= u.ux + 1; x++)
            for (y = u.uy - 1; y <= u.uy + 1; y++) {
                if (!isok(x, y) || (x == u.ux && y == u.uy))
                    continue;

                if (accessible(x, y) && !MON_AT(x, y)) {
                    distance = distu(x, y);
                    if (min_distance < 0 || distance < min_distance
                        || (distance == min_distance && rn2(2))) {
                        if (i > 0 || (((t = t_at(x, y)) == 0 || !t->tseen)
                                      && (!sobj_at(BOULDER, x, y)
                                          || racial_throws_rocks(&youmonst)))) {
                            spot->x = x;
                            spot->y = y;
                            min_distance = distance;
                            found = TRUE;
                        }
                    }
                }
            }
    }

    /* If we didn't find a good spot and forceit is on, try enexto(). */
    if (forceit && min_distance < 0
        && !enexto(spot, u.ux, u.uy, youmonst.data))
        return FALSE;

    return found;
}

/* Stop riding the current steed */
void
dismount_steed(reason)
int reason; /* Player was thrown off etc. */
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
    u.usteed = 0; /* affects Fly test; could hypothetically affect Lev */
    ufly = Flying ? TRUE : FALSE;
    ulev = Levitation ? TRUE : FALSE;
    u.usteed = mtmp;

    /* Check the reason for dismounting */
    otmp = which_armor(mtmp, W_SADDLE);
    switch (reason) {
    case DISMOUNT_THROWN:
    case DISMOUNT_FELL:
        verb = (reason == DISMOUNT_THROWN) ? "are thrown"
               : ulev ? "float" : ufly ? "fly" : "fall";
        You("%s off of %s!", verb, mon_nam(mtmp));
        if (!have_spot)
            have_spot = landing_spot(&cc, reason, 1);
        if (!ulev && !ufly) {
            losehp(Maybe_Half_Phys(rn1(10, 10)), "riding accident",
                   KILLED_BY_AN);
            set_wounded_legs(BOTH_SIDES, (int) HWounded_legs + rn1(5, 5));
            repair_leg_damage = FALSE;
        }
        break;
    case DISMOUNT_POLY:
        You("can no longer ride %s.", mon_nam(u.usteed));
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
        if (!has_mname(mtmp)) {
            pline("You've been through the dungeon on %s with no name.",
                  an(mtmp->data->mname));
            if (Hallucination)
                pline("It felt good to get out of the rain.");
        } else
            You("dismount %s.", mon_nam(mtmp));
    }
    /* While riding, Wounded_legs refers to the steed's legs;
       after dismounting, it reverts to the hero's legs. */
    if (repair_leg_damage)
        heal_legs(1);

    /* Release the steed and saddle */
    u.usteed = 0;
    u.ugallop = 0L;
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
        if (!enexto(&steedcc, u.ux, u.uy, mtmp->data)
            /* no spot? must have been engulfed by a lurker-above over
               water or lava; try requesting a location for a flyer */
            && !enexto(&steedcc, u.ux, u.uy, &mons[PM_BAT]))
            /* still no spot; last resort is any spot within bounds */
            (void) enexto(&steedcc, u.ux, u.uy, &mons[PM_GHOST]);
    }

    if (!DEADMONSTER(mtmp)) {
        in_steed_dismounting++;
        place_monster(mtmp, steedcc.x, steedcc.y);
        in_steed_dismounting--;

        /* if for bones, there's no reason to place the hero;
           we want to make room for potential ghost, so move steed */
        if (reason == DISMOUNT_BONES) {
            /* move the steed to an adjacent square */
            if (enexto(&cc, u.ux, u.uy, mtmp->data))
                rloc_to(mtmp, cc.x, cc.y);
            else /* evidently no room nearby; move steed elsewhere */
                (void) rloc(mtmp, FALSE);
            return;
        }

        /* Set hero's and/or steed's positions.  Usually try moving the
           hero first.  Note: for DISMOUNT_ENGULFED, caller hasn't set
           u.uswallow yet but has set u.ustuck. */
        if (!u.uswallow && !u.ustuck && have_spot) {
            struct permonst *mdat = mtmp->data;

            /* The steed may drop into water/lava */
            if (!is_flyer(mdat) && !is_floater(mdat)
                && !is_clinger(mdat) && !can_levitate(mtmp)) {
                if (is_pool(u.ux, u.uy)) {
                    if (!Underwater)
                        pline("%s falls into the %s!", Monnam(mtmp),
                              surface(u.ux, u.uy));
                    if (!is_swimmer(mdat) && !amphibious(mdat)) {
                        killed(mtmp);
                        You_feel("guilty.");
                        adjalign(-1);
                    }
                } else if (is_lava(u.ux, u.uy)) {
                    pline("%s is pulled into the %s!", Monnam(mtmp),
                          hliquid("lava"));
                    if (!likes_lava(mdat)) {
                        killed(mtmp);
                        You_feel("guilty.");
                        adjalign(-1);
                    }
                } else if (is_open_air(u.ux, u.uy)) {
                    pline("%s plummets several thousand feet to %s death.",
                          Monnam(mtmp), mhis(mtmp));
                    /* no corpse or objects as both are now several thousand feet down */
                    mongone(mtmp);
                    You_feel("guilty.");
                    adjalign(-1);
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
             * instead
             * calling them first and calling pickup() afterwards.  But it
             * would take a lot of work to keep this change from having any
             * unforeseen side effects (for instance, you would no longer be
             * able to walk onto a square with a hole, and autopickup before
             * falling into the hole).
             */
            /* [ALI] No need to move the player if the steed died. */
            if (!DEADMONSTER(mtmp)) {
                /* Keep steed here, move the player to cc;
                 * teleds() clears u.utrap
                 */
                in_steed_dismounting = TRUE;
                teleds(cc.x, cc.y, TELEDS_ALLOW_DRAG);
                in_steed_dismounting = FALSE;

                /* Put your steed in your trap */
                if (save_utrap)
                    (void) mintrap(mtmp);
            }

        /* Couldn't move hero... try moving the steed. */
        } else if (enexto(&cc, u.ux, u.uy, mtmp->data)) {
            /* Keep player here, move the steed to cc */
            rloc_to(mtmp, cc.x, cc.y);
            /* Player stays put */

        /* Otherwise, steed goes bye-bye. */
        } else {
#if 1       /* original there's-no-room handling */
            if (reason == DISMOUNT_BYCHOICE) {
                /* [un]#ride: hero gets credit/blame for killing steed */
                killed(mtmp);
                You_feel("guilty.");
                adjalign(-1);
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
        in_steed_dismounting = TRUE;
        (void) float_down(0L, W_SADDLE);
        in_steed_dismounting = FALSE;
        context.botl = TRUE;
        (void) encumber_msg();
        vision_full_recalc = 1;
    } else
        context.botl = TRUE;
    /* polearms behave differently when not mounted */
    if (uwep && is_pole(uwep))
        unweapon = TRUE;
    return;
}

/* when attempting to saddle or mount a sleeping steed, try to wake it up
   (for the saddling case, it won't be u.usteed yet) */
STATIC_OVL void
maybewakesteed(steed)
struct monst *steed;
{
    int frozen = (int) steed->mfrozen;
    boolean wasimmobile = steed->msleeping || !steed->mcanmove;

    steed->msleeping = 0;
    if (frozen) {
        frozen = (frozen + 1) / 2; /* half */
        /* might break out of timed sleep or paralysis */
        if (!rn2(frozen)) {
            steed->mfrozen = 0;
            steed->mcanmove = 1;
        } else {
            /* didn't awake, but remaining duration is halved */
            steed->mfrozen = frozen;
        }
    }
    if (wasimmobile && !steed->msleeping && steed->mcanmove)
        pline("%s wakes up.", Monnam(steed));
    /* regardless of waking, terminate any meal in progress */
    finish_meating(steed);
}

/* decide whether hero's steed is able to move;
   doesn't check for holding traps--those affect the hero directly */
boolean
stucksteed(checkfeeding)
boolean checkfeeding;
{
    struct monst *steed = u.usteed;

    if (steed) {
        /* check whether steed can move */
        if (steed->msleeping || !steed->mcanmove) {
            pline("%s won't move!", upstart(y_monnam(steed)));
            return TRUE;
        }
        /* optionally check whether steed is in the midst of a meal */
        if (checkfeeding && steed->meating) {
            pline("%s is still eating.", upstart(y_monnam(steed)));
            return TRUE;
        }
    }
    return FALSE;
}

void
place_monster(mon, x, y)
struct monst *mon;
int x, y;
{
    struct monst *othermon;
    const char *monnm, *othnm;
    char buf[QBUFSZ];

    buf[0] = '\0';
    /* normal map bounds are <1..COLNO-1,0..ROWNO-1> but sometimes
       vault guards (either living or dead) are parked at <0,0> */
    if (!isok(x, y) && (x != 0 || y != 0 || !mon->isgd)) {
        describe_level(buf);
        impossible("trying to place %s at <%d,%d> mstate:%lx on %s",
                   minimal_monnam(mon, TRUE), x, y, mon->mstate, buf);
        x = y = 0;
    }
    if ((mon == u.usteed && !in_steed_dismounting)
        /* special case is for convoluted vault guard handling */
        || (DEADMONSTER(mon) && !(mon->isgd && x == 0 && y == 0))) {
        describe_level(buf);
        impossible("placing %s onto map, mstate:%lx, on %s?",
                   (mon == u.usteed) ? "steed" : "defunct monster",
                   mon->mstate, buf);
        return;
    }
    if (((othermon = level.monsters[x][y]) != 0)
        /* steed and rider are colocated in the same position, so allow
         * placing one on top of the other */
        && !((has_erid(othermon) && ERID(othermon)->mon_steed == mon)
             || (has_erid(mon) && ERID(mon)->mon_steed == othermon))) {
        describe_level(buf);
        monnm = minimal_monnam(mon, FALSE);
        othnm = (mon != othermon) ? minimal_monnam(othermon, TRUE) : "itself";
        impossible("placing %s over %s at <%d,%d>, mstates:%lx %lx on %s?",
                   monnm, othnm, x, y, othermon->mstate, mon->mstate, buf);
    }
    mon->mx = x, mon->my = y;
    level.monsters[x][y] = mon;
    mon->mstate &= ~(MON_OFFMAP | MON_MIGRATING | MON_LIMBO | MON_BUBBLEMOVE
                     | MON_ENDGAME_FREE | MON_ENDGAME_MIGR);
}

/*steed.c*/
