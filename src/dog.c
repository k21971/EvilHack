/* NetHack 3.6	dog.c	$NHDT-Date: 1554580624 2019/04/06 19:57:04 $  $NHDT-Branch: NetHack-3.6.2-beta01 $:$NHDT-Revision: 1.85 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2011. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

STATIC_DCL int NDECL(pet_type);
STATIC_DCL int NDECL(woodland_animal);
STATIC_DCL int NDECL(elemental);

void
newedog(mtmp)
struct monst *mtmp;
{
    if (!mtmp->mextra)
        mtmp->mextra = newmextra();
    if (!EDOG(mtmp)) {
        EDOG(mtmp) = (struct edog *) alloc(sizeof(struct edog));
        (void) memset((genericptr_t) EDOG(mtmp), 0, sizeof(struct edog));
    }
}

void
free_edog(mtmp)
struct monst *mtmp;
{
    if (mtmp->mextra && EDOG(mtmp)) {
        free((genericptr_t) EDOG(mtmp));
        EDOG(mtmp) = (struct edog *) 0;
    }
    mtmp->mtame = 0;
}

void
initedog(mtmp, everything)
register struct monst *mtmp;
boolean everything;
{
    struct edog *edogp = EDOG(mtmp);
    long minhungry = moves + 1000L;
    schar minimumtame = is_domestic(mtmp->data) ? 10 : 5;

    mtmp->mtame = max(minimumtame, mtmp->mtame);
    mtmp->mpeaceful = 1;
    mtmp->mavenge = 0;
    set_malign(mtmp); /* recalc alignment now that it's tamed */
    if (everything) {
        mtmp->mleashed = 0;
        mtmp->meating = 0;
        edogp->droptime = 0;
        edogp->dropdist = 10000;
        edogp->apport = ACURR(A_CHA);
        edogp->whistletime = 0;
        edogp->ogoal.x = -1; /* force error if used before set */
        edogp->ogoal.y = -1;
        edogp->abuse = 0;
        edogp->revivals = 0;
        edogp->mhpmax_penalty = 0;
        edogp->killed_by_u = 0;
    } else {
        if (edogp->apport <= 0)
            edogp->apport = 1;
    }
    /* always set for newly tamed pet or feral former pet; hungrytime might
       already be higher when taming magic affects already tame monst */
    if (edogp->hungrytime < minhungry)
        edogp->hungrytime = minhungry;

    u.uconduct.pets++;
}

STATIC_OVL int
pet_type()
{
    if (urole.petnum != NON_PM)
        return  urole.petnum;
    else if (preferred_pet == 'c')
        return  PM_KITTEN;
    else if (preferred_pet == 'd')
        return  PM_LITTLE_DOG;
    else
        return  rn2(2) ? PM_KITTEN : PM_LITTLE_DOG;
}

STATIC_OVL int
woodland_animal()
{
    if (P_SKILL(spell_skilltype(SPE_SUMMON_ANIMAL)) < P_BASIC) {
        switch(rnd(5)) {
        case 1:
            return rn2(2) ? PM_GECKO : PM_NEWT;
        case 2:
            return rn2(2) ? PM_CENTIPEDE : PM_GARTER_SNAKE;
        case 3:
            return PM_FOX;
        case 4:
            return rn2(5) ? PM_BAT : PM_GIANT_BAT;
        case 5:
            return PM_GIANT_ANT;
        }
    } else if (P_SKILL(spell_skilltype(SPE_SUMMON_ANIMAL)) == P_BASIC) {
        switch(rnd(5)) {
        case 1:
            return rn2(4) ? PM_WOODCHUCK : PM_ROTHE;
        case 2:
            return PM_SNAKE;
        case 3:
            return rn2(6) ? PM_DEER : PM_STAG;
        case 4:
            return PM_HAWK;
        case 5:
            return PM_LIZARD;
        }
    } else if (P_SKILL(spell_skilltype(SPE_SUMMON_ANIMAL)) == P_SKILLED) {
        switch(rnd(5)) {
        case 1:
            return PM_GIANT_BEETLE;
        case 2:
            return rn2(5) ? PM_WINTER_WOLF_CUB : PM_WINTER_WOLF;
        case 3:
            return rn2(2) ? PM_LYNX : PM_WOLF;
        case 4:
            return PM_GRAY_UNICORN;
        case 5:
            return rn2(4) ? PM_LARGE_HAWK : PM_GIANT_HAWK;
        }
    } else if (P_SKILL(spell_skilltype(SPE_SUMMON_ANIMAL)) == P_EXPERT) {
        switch(rnd(5)) {
        case 1:
            return PM_HONEY_BADGER;
        case 2:
            return rn2(5) ? PM_WOLVERINE : PM_DIRE_WOLVERINE;
        case 3:
            return rn2(5) ? PM_GRIZZLY_BEAR : PM_CAVE_BEAR;
        case 4:
            return PM_SABER_TOOTHED_TIGER;
        case 5:
            return rn2(6) ? PM_GIANT_CROCODILE : PM_PEGASUS;
        }
    }
    return 0;
}

STATIC_OVL int
elemental()
{
    switch(rnd(4)) {
    case 1:
        return PM_AIR_ELEMENTAL;
    case 2:
        return PM_WATER_ELEMENTAL;
    case 3:
        return PM_EARTH_ELEMENTAL;
    case 4:
        return PM_FIRE_ELEMENTAL;
    }
    return 0;
}

struct monst *
make_familiar(otmp, x, y, quietly, you)
register struct obj *otmp;
xchar x, y;
boolean quietly;
boolean you;
{
    struct permonst *pm;
    struct monst *mtmp = 0;
    int chance, trycnt = 100;
    boolean idol = otmp && otmp->oartifact == ART_IDOL_OF_MOLOCH;

    do {
        if (otmp) { /* figurine; otherwise spell */
            int mndx = otmp->corpsenm;
            if (idol) {
                mndx = ndemon(A_NONE);
                if (mndx == NON_PM) /* just in case */
                    continue;
            }

            pm = &mons[mndx];
            /* activating a figurine provides one way to exceed the
               maximum number of the target critter created--unless
               it has a special limit (erinys, Nazgul) */
            if ((mvitals[mndx].mvflags & G_EXTINCT)
                && mbirth_limit(mndx) != MAXMONNO) {
                if (!quietly) {
                    /* have just been given "You <do something with>
                       the figurine and it transforms." message */
                    if (!idol)
                        pline("... into a pile of dust.");
                    else if (!Blind)
                        pline_The("cloud disperses.");
                }
                break; /* mtmp is null */
            }
        } else if (!rn2(3)) {
            pm = &mons[pet_type()];
        } else {
            pm = rndmonst();
            if (!pm) {
                if (!quietly)
                    There("seems to be nothing available for a familiar.");
                break;
            }
        }

        mtmp = makemon(pm, x, y, MM_EDOG | MM_IGNOREWATER
                                         | (!idol * NO_MINVENT));
        if (otmp && !mtmp) { /* monster was genocided or square occupied */
            if (!quietly) {
                if (!idol)
                    pline_The("figurine writhes and then shatters "
                              "into pieces!");
                else if (!Blind)
                    pline_The("cloud disperses.");
            }
            break;
        }
    } while (!mtmp && --trycnt > 0);

    if (!mtmp)
        return (struct monst *) 0;

    if (idol && !quietly && !Blind) {
        pline_The("mist coagulates into the shape of %s%s.",
                  x_monnam(mtmp, ARTICLE_A, (char *) 0, SUPPRESS_IT
                           | SUPPRESS_INVISIBLE | SUPPRESS_SADDLE
                           | SUPPRESS_BARDING | SUPPRESS_NAME, FALSE),
                  canspotmon(mtmp) ? "" : " and vanishes");
    }

    if (is_pool(mtmp->mx, mtmp->my) && minliquid(mtmp))
        return (struct monst *) 0;

    initedog(mtmp, TRUE);
    mtmp->msleeping = 0;
    if (otmp) { /* figurine; resulting monster might not become a pet */
        chance = rn2(10); /* 0==tame, 1==peaceful, 2==hostile */
        boolean same_align = (sgn(mon_aligntyp(mtmp)) == u.ualign.type);

        if (chance > 2)
            chance = otmp->blessed ? 0 : !otmp->cursed ? 1 : 2;
        /* 0,1,2:  b=80%,10,10; nc=10%,80,10; c=10%,10,80 */

        /* Unique monsters, monsters that covet the Amulet,
           and various other creatures (see mondata.h) can't
           be tamed */
        if (non_tameable(mtmp->data))
            chance = 2;

        /* when adhering to petless conduct, if a monster activates
           a figurine, it will always be hostile */
        if (!(u.uconduct.pets && you))
            chance = 2;

        if (Role_if(PM_KNIGHT) && u.ualign.type == A_LAWFUL
            && mtmp->data == &mons[PM_ELDRITCH_KI_RIN])
            chance = 2;

        if (Role_if(PM_KNIGHT) && u.ualign.type == A_CHAOTIC
            && mtmp->data == &mons[PM_KI_RIN])
            chance = 2;

        if (Role_if(PM_KNIGHT) && is_dragon(mtmp->data)
            && !same_align)
            chance = 2;

        if (chance > 0) {
            mtmp->mtame = 0;   /* not tame after all */
            u.uconduct.pets--; /* doesn't count as creating a pet */
            if (chance == 2) { /* hostile (cursed figurine) */
                if (!quietly)
                    You("get a bad feeling about this.");
                mtmp->mpeaceful = 0;
                set_malign(mtmp);
            }
        }
        /* if figurine has been named, give same name to the monster */
        if (has_oname(otmp) && !idol)
            mtmp = christen_monst(mtmp, ONAME(otmp));
    }
    set_malign(mtmp); /* more alignment changes */
    newsym(mtmp->mx, mtmp->my);

    /* must wield weapon immediately since pets will otherwise drop it */
    if (mtmp->mtame && attacktype(mtmp->data, AT_WEAP)) {
        mtmp->weapon_check = NEED_HTH_WEAPON;
        (void) mon_wield_item(mtmp);
    }
    return mtmp;
}

/* from Slash'EM
   currently only used for flaming/freezing spheres
   created by their respective spells */
struct monst *
make_helper(mnum, x, y)
int mnum;
xchar x, y;
{
    struct permonst *pm;
    struct monst *mtmp = 0;
    int trycnt = 100;

    do {
        pm = &mons[mnum];
        mtmp = makemon(pm, x, y,
                       MM_EDOG | MM_IGNOREWATER
                           | MM_IGNORELAVA | MM_IGNOREAIR | NO_MINVENT);
    } while (!mtmp && --trycnt > 0);

    if (!mtmp)
        return (struct monst *) 0; /* genocided */

    initedog(mtmp, TRUE);
    mtmp->msleeping = 0;
    set_malign(mtmp); /* more alignment changes */
    newsym(mtmp->mx, mtmp->my);

    return mtmp;
}

struct monst *
make_woodland_animal(x, y)
xchar x, y;
{
    struct permonst *pm;
    struct monst *mtmp = 0;
    int trycnt = 100;

    do {
        pm = &mons[woodland_animal()];
        mtmp = makemon(pm, x, y, MM_EDOG | NO_MINVENT);
    } while (!mtmp && --trycnt > 0);

    if (!mtmp)
        return (struct monst *) 0; /* genocided */

    initedog(mtmp, TRUE);
    mtmp->mtame = 15;
    mtmp->msleeping = 0;
    set_malign(mtmp); /* more alignment changes */
    newsym(mtmp->mx, mtmp->my);

    return mtmp;
}

struct monst *
make_elemental(x, y)
xchar x, y;
{
    struct permonst *pm;
    struct monst *mtmp = 0;
    int trycnt = 100;

    do {
        pm = &mons[elemental()];
        mtmp = makemon(pm, x, y, MM_EDOG | NO_MINVENT);
    } while (!mtmp && --trycnt > 0);

    if (!mtmp)
        return (struct monst *) 0; /* genocided */

    initedog(mtmp, TRUE);
    mtmp->mtame = 15;
    mtmp->msleeping = 0;
    /* increase hit points based on spellcasting skill */
    if (P_SKILL(spell_skilltype(SPE_SUMMON_ELEMENTAL)) == P_SKILLED) {
        mtmp->mhpmax += mtmp->mhpmax / 2;
        mtmp->mhp = mtmp->mhpmax;
    } else if (P_SKILL(spell_skilltype(SPE_SUMMON_ELEMENTAL)) == P_EXPERT) {
        mtmp->mhpmax += mtmp->mhpmax;
        mtmp->mhp = mtmp->mhpmax;
    }
    set_malign(mtmp); /* more alignment changes */
    newsym(mtmp->mx, mtmp->my);

    return mtmp;
}

struct monst *
makedog()
{
    register struct monst *mtmp;
    register struct obj *otmp;
    const char *petname = 0;
    int pettype;
    static int petname_used = 0;

    if (preferred_pet == 'n')
        return ((struct monst *) 0);

    pettype = pet_type();
    if (pettype == PM_LITTLE_DOG) {
        petname = dogname;
        if (Race_if(PM_DROW)) {
            petname = spidername;
            pettype = PM_LARGE_SPIDER;
        } else if (Race_if(PM_DRAUGR)) {
            petname = dogname;
            pettype = PM_SMALL_SKELETAL_HOUND;
        } else if (Race_if(PM_VAMPIRE)) {
            petname = dogname;
            pettype = PM_WOLF_CUB;
        }
    } else if (pettype == PM_PSEUDODRAGON) {
        petname = pseudoname;
    } else if (pettype == PM_SEWER_RAT) {
        petname = ratname;
    } else if (pettype == PM_LESSER_HOMUNCULUS) {
        petname = homunname;
        if (Race_if(PM_DROW)) {
            petname = spidername;
            pettype = PM_LARGE_SPIDER;
        } else if (Race_if(PM_DRAUGR)) {
            petname = dogname;
            pettype = PM_SMALL_SKELETAL_HOUND;
        } else if (Race_if(PM_VAMPIRE)) {
            petname = dogname;
            pettype = PM_WOLF_CUB;
        }
    } else if (pettype == PM_LARGE_SPIDER) {
        petname = spidername;
    } else if (pettype == PM_HAWK) {
        petname = hawkname;
    } else if (pettype == PM_PONY) {
        petname = horsename;
        /* hijack creation for chaotic knights */
        if (u.ualign.type == A_CHAOTIC && Role_if(PM_KNIGHT)) {
            if (Race_if(PM_DRAUGR)) {
                pettype = PM_SKELETAL_PONY;
            } else if (!Race_if(PM_CENTAUR)) {
                pettype = PM_LESSER_NIGHTMARE;
            } else {
                petname = dogname;
                pettype = PM_LITTLE_DOG;
            }
        }
    } else if (pettype == PM_KITTEN) {
        petname = catname;
        if (Race_if(PM_DROW)) {
            petname = spidername;
            pettype = PM_LARGE_SPIDER;
        } else if (Race_if(PM_DRAUGR)) {
            petname = dogname;
            pettype = PM_SMALL_SKELETAL_HOUND;
        } else if (Race_if(PM_VAMPIRE)) {
            petname = dogname;
            pettype = PM_WOLF_CUB;
        }
    }

    /* default pet names */
    if (!*petname && pettype == PM_LITTLE_DOG) {
        /* All of these names were for dogs. */
        if (Role_if(PM_CAVEMAN))
            petname = "Slasher"; /* The Warrior */
        if (Role_if(PM_SAMURAI))
            petname = "Hachi"; /* Shibuya Station */
        if (Role_if(PM_BARBARIAN))
            petname = "Idefix"; /* Obelix */
        if (Role_if(PM_RANGER))
            petname = "Sirius"; /* Orion's dog */
    } else if (!*petname && pettype == PM_SEWER_RAT) {
        if (Role_if(PM_CONVICT))
            petname = "Nicodemus"; /* Rats of NIMH */
    } else if (!*petname && pettype == PM_LESSER_HOMUNCULUS) {
        if (Role_if(PM_INFIDEL))
            petname = "Hecubus"; /* The Kids in the Hall */
    } else if (!*petname && pettype == PM_HAWK) {
        if (Role_if(PM_DRUID))
            petname = "Bobo"; /* Viva La Dirt League */
    }

    mtmp = makemon(&mons[pettype], u.ux, u.uy, MM_EDOG);

    if (!mtmp)
        return ((struct monst *) 0); /* pets were genocided */

    context.startingpet_mid = mtmp->m_id;
    /* Horses already wear a saddle */
    if ((pettype == PM_PONY || pettype == PM_LESSER_NIGHTMARE
         || pettype == PM_SKELETAL_PONY)
        && !!(otmp = mksobj(SADDLE, TRUE, FALSE))) {
        otmp->dknown = otmp->bknown = otmp->rknown = 1;
        put_saddle_on_mon(otmp, mtmp);
    }

    if (!petname_used++ && *petname)
        mtmp = christen_monst(mtmp, petname);

    initedog(mtmp, TRUE);
    return  mtmp;
}

/* record `last move time' for all monsters prior to level save so that
   mon_arrive() can catch up for lost time when they're restored later */
void
update_mlstmv()
{
    struct monst *mon;

    /* monst->mlstmv used to be updated every time `monst' actually moved,
       but that is no longer the case so we just do a blanket assignment */
    for (mon = fmon; mon; mon = mon->nmon) {
        if (DEADMONSTER(mon))
            continue;
        mon->mlstmv = monstermoves;
    }
}

void
losedogs()
{
    register struct monst *mtmp, *mtmp0, *mtmp2;
    int dismissKops = 0;
    boolean stalked = 0;

    /*
     * First, scan migrating_mons for shopkeepers who want to dismiss Kops,
     * and scan mydogs for shopkeepers who want to retain kops.
     * Second, dismiss kops if warranted, making more room for arrival.
     * Third, place monsters accompanying the hero.
     * Last, place migrating monsters coming to this level.
     *
     * Hero might eventually be displaced (due to the third step, but
     * occurring later), which is the main reason to do the second step
     * sooner (in turn necessitating the first step, rather than combining
     * the list scans with monster placement).
     */

    /* check for returning shk(s) */
    for (mtmp = migrating_mons; mtmp; mtmp = mtmp->nmon) {
        if (mtmp->mux != u.uz.dnum || mtmp->muy != u.uz.dlevel)
            continue;
        if (mtmp->isshk) {
            if (ESHK(mtmp)->dismiss_kops) {
                if (dismissKops == 0)
                    dismissKops = 1;
                ESHK(mtmp)->dismiss_kops = FALSE; /* reset */
            } else if (!mtmp->mpeaceful) {
                /* an unpacified shk is returning; don't dismiss kops
                   even if another pacified one is willing to do so */
                dismissKops = -1;
                /* [keep looping; later monsters might need ESHK reset] */
            }
        }
    }
    /* make the same check for mydogs */
    for (mtmp = mydogs; mtmp && dismissKops >= 0; mtmp = mtmp->nmon) {
        if (mtmp->isshk) {
            /* hostile shk might accompany hero [ESHK(mtmp)->dismiss_kops
               can't be set here; it's only used for migrating_mons] */
            if (!mtmp->mpeaceful)
                dismissKops = -1;
        }
    }

    /* when a hostile shopkeeper chases hero to another level
       and then gets paid off there, get rid of summoned kops
       here now that he has returned to his shop level */
    if (dismissKops > 0)
        make_happy_shoppers(TRUE);

    /* place pets and/or any other monsters who accompany hero */
    while ((mtmp = mydogs) != 0) {
        mydogs = mtmp->nmon;
        mon_arrive(mtmp, TRUE);
    }

    /* time for migrating monsters to arrive;
       monsters who belong on this level but fail to arrive get put
       back onto the list (at head), so traversing it is tricky */
    for (mtmp = migrating_mons; mtmp; mtmp = mtmp2) {
        mtmp2 = mtmp->nmon;
        if (mtmp->mux == u.uz.dnum
            && (mtmp->muy == u.uz.dlevel
                || (mtmp->muy < u.uz.dlevel
                    && mtmp->mtrack[0].x == MIGR_STALK))) {
            /* remove mtmp from migrating_mons list */
            if (mtmp == migrating_mons) {
                migrating_mons = mtmp->nmon;
            } else {
                for (mtmp0 = migrating_mons; mtmp0; mtmp0 = mtmp0->nmon)
                    if (mtmp0->nmon == mtmp) {
                        mtmp0->nmon = mtmp->nmon;
                        break;
                    }
                if (!mtmp0)
                    panic("losedogs: can't find migrating mon");
            }
            if (mtmp->muy < u.uz.dlevel && mtmp->mtrack[0].x == MIGR_STALK)
                stalked = 1;
            mon_arrive(mtmp, FALSE);
        }
    }
    if (stalked) {
        if (!Deaf)
            You_hear("demonic laughter.");
        else
            You_feel("a forboding force surround you.");
    }
}

/* called from resurrect() in addition to losedogs() */
void
mon_arrive(mtmp, with_you)
struct monst *mtmp;
boolean with_you;
{
    struct trap *t;
    xchar xlocale, ylocale, xyloc, xyflags, wander, dlev;
    int num_segs;
    boolean failed_to_place = FALSE;

    mtmp->nmon = fmon;
    fmon = mtmp;
    if (mtmp->isshk)
        set_residency(mtmp, FALSE);

    num_segs = mtmp->wormno;
    /* baby long worms have no tail so don't use is_longworm() */
    if (mtmp->data == &mons[PM_LONG_WORM]) {
        mtmp->wormno = get_wormno();
        if (mtmp->wormno)
            initworm(mtmp, num_segs);
    } else
        mtmp->wormno = 0;

    /* some monsters might need to do something special upon arrival
       _after_ the current level has been fully set up; see dochug() */
    mtmp->mstrategy |= STRAT_ARRIVE;

    /* make sure mnexto(rloc_to(set_apparxy())) doesn't use stale data */
    dlev = mtmp->muy;
    mtmp->mux = u.ux, mtmp->muy = u.uy;
    xyloc = mtmp->mtrack[0].x;
    xyflags = mtmp->mtrack[0].y;
    xlocale = mtmp->mtrack[1].x;
    ylocale = mtmp->mtrack[1].y;
    memset(mtmp->mtrack, 0, sizeof mtmp->mtrack);

    if (mtmp == u.usteed || mtmp->ridden_by)
        return; /* don't place steed on the map */
    if (with_you) {
        /* When a monster accompanies you, sometimes it will arrive
           at your intended destination and you'll end up next to
           that spot.  This code doesn't control the final outcome;
           goto_level(do.c) decides who ends up at your target spot
           when there is a monster there too. */
        if (!MON_AT(u.ux, u.uy)
            && !rn2(mtmp->mtame ? 10 : mtmp->mpeaceful ? 5 : 2))
            rloc_to(mtmp, u.ux, u.uy);
        else
            mnexto(mtmp);
        return;
    }
    /*
     * The monster arrived on this level independently of the player.
     * Its coordinate fields were overloaded for use as flags that
     * specify its final destination.
     */

    if (mtmp->mlstmv < monstermoves - 1L) {
        /* heal monster for time spent in limbo */
        long nmv = monstermoves - 1L - mtmp->mlstmv;

        mon_catchup_elapsed_time(mtmp, nmv);
        mtmp->mlstmv = monstermoves - 1L;

        /* let monster move a bit on new level (see placement code below) */
        wander = (xchar) min(nmv, 8);
    } else
        wander = 0;

    switch (xyloc) {
    case MIGR_APPROX_XY: /* {x,y}locale set above */
        break;
    case MIGR_EXACT_XY:
        wander = 0;
        break;
    case MIGR_WITH_HERO:
        xlocale = u.ux, ylocale = u.uy;
        break;
    case MIGR_STAIRS_UP:
        xlocale = xupstair, ylocale = yupstair;
        break;
    case MIGR_STAIRS_DOWN:
        xlocale = xdnstair, ylocale = ydnstair;
        break;
    case MIGR_LADDER_UP:
        xlocale = xupladder, ylocale = yupladder;
        break;
    case MIGR_LADDER_DOWN:
        xlocale = xdnladder, ylocale = ydnladder;
        break;
    case MIGR_SSTAIRS:
        xlocale = sstairs.sx, ylocale = sstairs.sy;
        break;
    case MIGR_STALK:
        /* if returning to same level, pretend it never left, so no movement.
           if going to new level, don't give the hero any breathing room. */
        wander = 0;
        if (dlev < u.uz.dlevel) {
            xlocale = u.ux;
            ylocale = u.uy;
            mtmp->mstrategy &= ~STRAT_WAITMASK;
        }
        break;
    case MIGR_PORTAL:
        if (In_endgame(&u.uz) || Is_stronghold(&u.uz)) {
            /* there is no arrival portal for endgame levels */
            /* BUG[?]: for simplicity, this code relies on the fact
               that we know that the current endgame levels always
               build upwards and never have any exclusion subregion
               inside their TELEPORT_REGION settings. */
            xlocale = rn1(updest.hx - updest.lx + 1, updest.lx);
            ylocale = rn1(updest.hy - updest.ly + 1, updest.ly);
            break;
        }
        /* find the arrival portal */
        for (t = ftrap; t; t = t->ntrap)
            if (t->ttyp == MAGIC_PORTAL)
                break;
        if (t) {
            xlocale = t->tx, ylocale = t->ty;
            break;
        } else if (iflags.debug_fuzzer && (xupstair || xdnstair)) {
            /* debugfuzzer fallback: use stairs if available */
            if (xupstair) {
                xlocale = xupstair, ylocale = yupstair;
            } else {
                xlocale = xdnstair, ylocale = ydnstair;
            }
            break;
        } else if (!(u.uevent.qexpelled
                     && (Is_qstart(&u.uz0) || Is_qstart(&u.uz)))) {
            /* Only show impossible for non-quest-expulsion cases */
            impossible("mon_arrive: no corresponding portal?");
        }
        /*FALLTHRU*/
    default:
    case MIGR_RANDOM:
        xlocale = ylocale = 0;
        break;
    }

    if ((mtmp->mspare1 & MIGR_LEFTOVERS) != 0L) {
        /* Pick up the rest of the MIGR_TO_SPECIES objects */
        if (migrating_objs)
            deliver_obj_to_mon(mtmp, 0, DF_ALL);
    }

    if (xlocale && wander) {
        /* monster moved a bit; pick a nearby location */
        /* mnearto() deals w/stone, et al */
        char *r = in_rooms(xlocale, ylocale, 0);

        if (r && *r) {
            coord c;

            /* somexy() handles irregular rooms */
            if (somexy(&rooms[*r - ROOMOFFSET], &c))
                xlocale = c.x, ylocale = c.y;
            else
                xlocale = ylocale = 0;
        } else { /* not in a room */
            int i, j;

            i = max(1, xlocale - wander);
            j = min(COLNO - 1, xlocale + wander);
            xlocale = rn1(j - i, i);
            i = max(0, ylocale - wander);
            j = min(ROWNO - 1, ylocale + wander);
            ylocale = rn1(j - i, i);
        }
    } /* moved a bit */

    mtmp->mx = 0; /*(already is 0)*/
    mtmp->my = xyflags;
    if (xlocale)
        failed_to_place = !mnearto(mtmp, xlocale, ylocale, FALSE);
    else
        failed_to_place = !rloc(mtmp, TRUE);

    if (failed_to_place)
        m_into_limbo(mtmp); /* try again next time hero comes to this level */
}

/* heal monster for time spent elsewhere */
void
mon_catchup_elapsed_time(mtmp, nmv)
struct monst *mtmp;
long nmv; /* number of moves */
{
    int imv = 0; /* avoid zillions of casts and lint warnings */

#if defined(DEBUG) || (NH_DEVEL_STATUS != NH_STATUS_RELEASED)

    if (nmv < 0L) { /* crash likely... */
        panic("catchup from future time?");
        /*NOTREACHED*/
        return;
    } else if (nmv == 0L) { /* safe, but should'nt happen */
        impossible("catchup from now?");
    } else
#endif
        if (nmv >= LARGEST_INT) /* paranoia */
        imv = LARGEST_INT - 1;
    else
        imv = (int) nmv;

    /* might stop being afraid, blind or frozen */
    /* set to 1 and allow final decrement in movemon() */
    if (mtmp->mblinded) {
        if (imv >= (int) mtmp->mblinded)
            mtmp->mblinded = 1;
        else
            mtmp->mblinded -= imv;
    }
    if (mtmp->mfrozen) {
        if (imv >= (int) mtmp->mfrozen)
            mtmp->mfrozen = 1;
        else
            mtmp->mfrozen -= imv;
    }
    if (mtmp->mfleetim) {
        if (imv >= (int) mtmp->mfleetim)
            mtmp->mfleetim = 1;
        else
            mtmp->mfleetim -= imv;
    }
    if (mtmp->msummoned) {
        if (imv >= (int) mtmp->msummoned - 1)
            mtmp->msummoned = 2;
        else
            mtmp->msummoned -= imv;
    }
    if (mtmp->msicktime) {
        if (imv >= (int) mtmp->msicktime)
            mtmp->msicktime = 1;
        else
            mtmp->msicktime -= imv;
    }
    if (mtmp->mdiseasetime) {
        if (imv >= (int) mtmp->mdiseasetime)
            mtmp->mdiseasetime = 1;
        else
            mtmp->mdiseasetime -= imv;
    }
    if (mtmp->mreflecttime) {
        if (imv >= (int) mtmp->mreflecttime)
            mtmp->mreflecttime = 1;
        else
            mtmp->mreflecttime -= imv;
    }
    if (mtmp->mbarkskintime) {
        if (imv >= (int) mtmp->mbarkskintime)
            mtmp->mbarkskintime = 1;
        else
            mtmp->mbarkskintime -= imv;
    }
    if (mtmp->mstoneskintime) {
        if (imv >= (int) mtmp->mstoneskintime)
            mtmp->mstoneskintime = 1;
        else
            mtmp->mstoneskintime -= imv;
    }
    if (mtmp->mentangletime) {
        if (imv >= (int) mtmp->mentangletime)
            mtmp->mentangletime = 1;
        else
            mtmp->mentangletime -= imv;
    }

    /* Withering monsters by rights ought to keep withering while off-level, but
     * it brings up a host of problems to have a monster die in this function
     * (if the player were responsible, would they get experience for the kill
     * and potentially level up just by returning to the level? should messages
     * such as "You have a sad feeling" or vampire polyself/rise again print
     * upon arrival?)  Instead, just don't affect their hp or withering status;
     * they will begin re-withering when the hero comes back. */

    /* might recover from temporary trouble */
    if (mtmp->mtrapped && rn2(imv + 1) > 40 / 2)
        mtmp->mtrapped = 0;
    if (mtmp->mconf && rn2(imv + 1) > 50 / 2)
        mtmp->mconf = 0;
    if (mtmp->mstun && rn2(imv + 1) > 10 / 2)
        mtmp->mstun = 0;

    /* might finish eating or be able to use special ability again */
    if (imv > mtmp->meating)
        finish_meating(mtmp);
    else
        mtmp->meating -= imv;
    /* spellcasting 'power' */
    if (imv > mtmp->mspec_used)
        mtmp->mspec_used = 0;
    else
        mtmp->mspec_used -= imv;
    /* breaking boulders */
    if (imv > mtmp->mbreakboulder)
        mtmp->mbreakboulder = 0;
    else
        mtmp->mbreakboulder -= imv;

    /* reduce tameness for every 150 moves you are separated */
    if (mtmp->mtame) {
        struct obj *barding;
        int wilder = (imv + 75) / 150;

        if ((barding = which_armor(mtmp, W_BARDING)) != 0
            && barding->oartifact == ART_ITHILMAR)
            return; /* tameness isn't affected */
        else if (mtmp->mtame > wilder)
            mtmp->mtame -= wilder; /* less tame */
        else if (mtmp->mtame > rn2(wilder))
            mtmp->mtame = 0; /* untame */
        else {
            mtmp->mtame = mtmp->mpeaceful = 0; /* hostile! */
            newsym(mtmp->mx, mtmp->my); /* update display */
        }
    }
    /* check to see if it would have died as a pet; if so, go wild instead
     * of dying the next time we call dog_move()
     */
    if (mtmp->mtame && !mtmp->isminion
        && (carnivorous(mtmp->data) || herbivorous(mtmp->data))) {
        struct edog *edog = EDOG(mtmp);

        if ((monstermoves > edog->hungrytime + 500 && mtmp->mhp < 3)
            || (monstermoves > edog->hungrytime + 750)) {
            mtmp->mtame = mtmp->mpeaceful = 0;
            newsym(mtmp->mx, mtmp->my); /* update display */
        }
    }

    if (!mtmp->mtame && mtmp->mleashed) {
        /* leashed monsters should always be with hero, consequently
           never losing any time to be accounted for later */
        impossible("catching up for leashed monster?");
        m_unleash(mtmp, FALSE);
    }

    /* maybe pick up the abandoned Amulet */
    if (mtmp->data == &mons[PM_AGENT] && !mtmp->mpeaceful
        && !mon_has_amulet(mtmp) && rn2(imv + 1) > 300) {
        struct obj *otmp;
        for (otmp = level.objlist; otmp; otmp = otmp->nobj)
            if (otmp->otyp == AMULET_OF_YENDOR
                || otmp->otyp == FAKE_AMULET_OF_YENDOR) {
                obj_extract_self(otmp);
                (void) mpickobj(mtmp, otmp);
                break;
            }
    }

    /* recover lost hit points */
    if (!mtmp->mwither && (!Is_valley(&u.uz) || is_undead(r_data(mtmp)))) {
        if (!mon_prop(mtmp, REGENERATION))
            imv /= 20;
        if (mtmp->mhp + imv >= mtmp->mhpmax)
            mtmp->mhp = mtmp->mhpmax;
        else
            mtmp->mhp += imv;
    }
}

/* called when you move to another level */
void
keepdogs(pets_only)
boolean pets_only; /* true for ascension or final escape */
{
    register struct monst *mtmp, *mtmp2;
    register struct obj *obj;
    int num_segs;
    boolean stay_behind;

    for (mtmp = fmon; mtmp; mtmp = mtmp2) {
        mtmp2 = mtmp->nmon;
        if (DEADMONSTER(mtmp))
            continue;
        if (has_erid(mtmp) || mtmp->ridden_by)
            continue;
        if (pets_only) {
            if (!mtmp->mtame)
                continue; /* reject non-pets */
            /* don't block pets from accompanying hero's dungeon
               escape or ascension simply due to mundane trifles;
               unlike level change for steed, don't bother trying
               to achieve a normal trap escape first */
            mtmp->mtrapped = 0;
            mtmp->mentangled = 0;
            mtmp->meating = 0;
            mtmp->msleeping = 0;
            mtmp->mfrozen = 0;
            mtmp->mcanmove = 1;
        }
        if (((monnear(mtmp, u.ux, u.uy) && levl_follower(mtmp))
             /* the wiz will level t-port from anywhere to chase
                the amulet; if you don't have it, will chase you
                only if in range. -3. */
             || (u.uhave.amulet && mtmp->iswiz))
            && ((!mtmp->msleeping && mtmp->mcanmove)
                /* eg if level teleport or new trap, steed has no control
                   to avoid following */
                || (mtmp == u.usteed))
            /* monster won't follow if it hasn't noticed you yet */
            && !(mtmp->mstrategy & STRAT_WAITFORU)) {
            stay_behind = FALSE;
            if (mtmp->mtrapped)
                (void) mintrap(mtmp); /* try to escape */
            if (mtmp == u.usteed) {
                /* make sure steed is eligible to accompany hero */
                mtmp->mtrapped = 0;       /* escape trap */
                mtmp->meating = 0;        /* terminate eating */
                mdrop_special_objs(mtmp); /* drop Amulet */
            } else if (mtmp->meating || mtmp->mtrapped
                       || mtmp->mentangled) {
                if (canseemon(mtmp))
                    pline("%s is still %s.", Monnam(mtmp),
                          mtmp->meating ? "eating"
                                        : mtmp->mentangled ? "entangled"
                                                           : "trapped");
                stay_behind = TRUE;
            } else if (mon_has_amulet(mtmp)) {
                if (canseemon(mtmp))
                    pline("%s seems very disoriented for a moment.",
                          Monnam(mtmp));
                stay_behind = TRUE;
            }
            if (stay_behind) {
                if (mtmp->mleashed) {
                    pline("%s leash suddenly comes loose.",
                          humanoid(mtmp->data)
                              ? (mtmp->female ? "Her" : "His")
                              : "Its");
                    m_unleash(mtmp, FALSE);
                }
                if (mtmp == u.usteed) {
                    /* can't happen unless someone makes a change
                       which scrambles the stay_behind logic above */
                    impossible("steed left behind?");
                    dismount_steed(DISMOUNT_GENERIC);
                }
                continue;
            }
            if (mtmp->isshk)
                set_residency(mtmp, TRUE);

            if (mtmp->wormno) {
                register int cnt;
                /* NOTE: worm is truncated to # segs = max wormno size */
                cnt = count_wsegs(mtmp);
                num_segs = min(cnt, MAX_NUM_WORMS - 1);
                wormgone(mtmp);
                place_monster(mtmp, mtmp->mx, mtmp->my);
            } else
                num_segs = 0;

            /* set minvent's obj->no_charge to 0 */
            for (obj = mtmp->minvent; obj; obj = obj->nobj) {
                if (Has_contents(obj))
                    picked_container(obj); /* does the right thing */
                obj->no_charge = 0;
            }

            relmon(mtmp, &mydogs);   /* move it from map to mydogs */
            mtmp->mx = mtmp->my = 0; /* avoid mnexto()/MON_AT() problem */
            mtmp->wormno = num_segs;
            mtmp->mlstmv = monstermoves;
        } else if (mtmp->iswiz) {
            /* we want to be able to find him when his next resurrection
               chance comes up, but have him resume his present location
               if player returns to this level before that time */
            migrate_to_level(mtmp, ledger_no(&u.uz), MIGR_EXACT_XY,
                             (coord *) 0);
        } else if (mtmp->mleashed) {
            /* this can happen if your quest leader ejects you from the
               "home" level while a leashed pet isn't next to you */
            pline("%s leash goes slack.", s_suffix(Monnam(mtmp)));
            m_unleash(mtmp, FALSE);
        }
    }
}

void
migrate_to_level(mtmp, tolev, xyloc, cc)
register struct monst *mtmp;
xchar tolev; /* destination level */
xchar xyloc; /* MIGR_xxx destination xy location: */
coord *cc;   /* optional destination coordinates */
{
    struct obj *obj;
    d_level new_lev;
    xchar xyflags;
    int num_segs = 0; /* count of worm segments */

    /* guard against migrating dead monster (null) */
    if (DEADMONSTER(mtmp))
        return;

    /* Recursive call to levelport monster steeds */
    if (mtmp->mextra && ERID(mtmp) && ERID(mtmp)->mon_steed)
        migrate_to_level(ERID(mtmp)->mon_steed, tolev, xyloc, cc);

    if (mtmp->isshk)
        set_residency(mtmp, TRUE);

    if (mtmp->wormno) {
        int cnt = count_wsegs(mtmp);

        /* **** NOTE: worm is truncated to # segs = max wormno size **** */
        num_segs = min(cnt, MAX_NUM_WORMS - 1); /* used below */
        wormgone(mtmp); /* destroys tail and takes head off map */
        /* there used to be a place_monster() here for the relmon() below,
           but it doesn't require the monster to be on the map anymore */
    }

    /* set minvent's obj->no_charge to 0 */
    for (obj = mtmp->minvent; obj; obj = obj->nobj) {
        if (Has_contents(obj))
            picked_container(obj); /* does the right thing */
        obj->no_charge = 0;
    }

    if (mtmp->mleashed) {
        mtmp->mtame--;
        m_unleash(mtmp, TRUE);
    }
    if (mtmp->mentangled)
        mtmp->mentangled = 0;
    /* move it from map to migrating_mons */
    relmon(mtmp, &migrating_mons); /* mtmp->mx,my retain their value */
    mtmp->mstate |= MON_MIGRATING;

    new_lev.dnum = ledger_to_dnum((xchar) tolev);
    new_lev.dlevel = ledger_to_dlev((xchar) tolev);
    /* overload mtmp->[mx,my], mtmp->[mux,muy], and mtmp->mtrack[] as */
    /* destination codes (setup flag bits before altering mx or my) */
    xyflags = (depth(&new_lev) < depth(&u.uz)); /* 1 => up */
    if (In_W_tower(mtmp->mx, mtmp->my, &u.uz))
        xyflags |= 2;
    mtmp->wormno = num_segs;
    mtmp->mlstmv = monstermoves;
    mtmp->mtrack[2].x = u.uz.dnum; /* migrating from this dungeon */
    mtmp->mtrack[2].y = u.uz.dlevel; /* migrating from this dungeon level */
    mtmp->mtrack[1].x = cc ? cc->x : mtmp->mx;
    mtmp->mtrack[1].y = cc ? cc->y : mtmp->my;
    mtmp->mtrack[0].x = xyloc;
    mtmp->mtrack[0].y = xyflags;
    mtmp->mux = new_lev.dnum;
    mtmp->muy = new_lev.dlevel;
    mtmp->mx = mtmp->my = 0; /* this implies migration */
    if (mtmp == context.polearm.hitmon)
        context.polearm.hitmon = (struct monst *) 0;

    /* don't extinguish a mobile light; it still exists but has changed
       from local (monst->mx > 0) to global (mx==0, not on this level) */
    if (emits_light(mtmp->data))
        vision_recalc(0);
}

/* return quality of food; the lower the better */
/* fungi will eat even tainted food */
int
dogfood(mon, obj)
struct monst *mon;
register struct obj *obj;
{
    struct permonst *mptr = mon->data, *fptr = 0;
    boolean carni = carnivorous(mptr), herbi = herbivorous(mptr),
            starving, mblind;

    if (is_quest_artifact(obj) || obj_resists(obj, 0, 95))
        return obj->cursed ? TABU : APPORT;

    if (obj->oartifact == ART_EYE_OF_VECNA)
        return TABU;

    switch (obj->oclass) {
    case FOOD_CLASS:
        if (obj->otyp == CORPSE || obj->otyp == TIN || obj->otyp == EGG)
            fptr = &mons[obj->corpsenm];

        if (obj->otyp == CORPSE && is_rider(fptr))
            return TABU;
        if ((obj->otyp == CORPSE || obj->otyp == EGG)
            /* Medusa's corpse doesn't pass the touch_petrifies() test
               but does cause petrification if eaten */
            && (touch_petrifies(fptr) || obj->corpsenm == PM_MEDUSA)
            && !(resists_ston(mon) || defended(mon, AD_STON)))
            return POISON;

        /* vampires drain the blood from fresh corpses */
        if (is_vampire(mptr))
            return (obj->otyp == CORPSE
                    && has_blood(&mons[obj->corpsenm]) && !obj->oeaten
                    && peek_at_iced_corpse_age(obj) + 5 >= monstermoves)
                    ? DOGFOOD : TABU;

        if (!carni && !herbi)
            return obj->cursed ? UNDEF : APPORT;

        /* a starving pet will eat almost anything */
        starving = (mon->mtame && !mon->isminion
                    && EDOG(mon)->mhpmax_penalty);
        /* even carnivores will eat carrots if they're temporarily blind */
        mblind = (!mon->mcansee && haseyes(mon->data));

        /* ghouls/draugr prefer old corpses and unhatchable eggs, yum!
           they'll eat fresh non-veggy corpses and hatchable eggs
           when starving; they never eat stone-to-flesh'd meat */
        if (mptr == &mons[PM_GHOUL] || racial_zombie(mon)) {
            if (obj->otyp == CORPSE)
                return (peek_at_iced_corpse_age(obj) + 50L <= monstermoves
                        && fptr != &mons[PM_LIZARD]
                        && fptr != &mons[PM_LICHEN])
                           ? DOGFOOD
                           : (starving && !vegan(fptr))
                              ? ACCFOOD
                              : POISON;
            if (obj->otyp == EGG)
                return stale_egg(obj) ? CADAVER : starving ? ACCFOOD : POISON;
            return TABU;
        }

        /* lizards cure stoning. ghouls won't eat them even then, though,
           just like elves prefer starvation to cannibalism. */
        if (obj->otyp == CORPSE && fptr == &mons[PM_LIZARD] && mon->mstone)
            return DOGFOOD;

        /* gnomes hate eggs */
        if (obj->otyp == EGG && racial_gnome(mon))
            return TABU;

        switch (obj->otyp) {
        case TRIPE_RATION:
        case MEATBALL:
        case MEAT_RING:
        case MEAT_STICK:
        case MEAT_SUIT:
        case MEAT_HELMET:
        case MEAT_SHIELD:
        case MEAT_GLOVES:
        case MEAT_BOOTS:
        case STRIP_OF_BACON:
        case HUGE_CHUNK_OF_MEAT:
            return carni ? DOGFOOD : MANFOOD;
        case EGG:
            return carni ? CADAVER : MANFOOD;
        case CORPSE:
            if ((peek_at_iced_corpse_age(obj) + 50L <= monstermoves
                 && obj->corpsenm != PM_LIZARD && obj->corpsenm != PM_LICHEN
                 && mptr->mlet != S_FUNGUS)
                || (acidic(fptr) && !(resists_acid(mon)
                                      || defended(mon, AD_ACID)))
                || (poisonous(fptr) && !(resists_poison(mon)
                                         || defended(mon, AD_DRST)))
                || (obj->zombie_corpse && !(resists_sick(mon)
                                            || defended(mon, AD_DISE)))
                || (touch_petrifies(&mons[obj->corpsenm])
                    && !(resists_ston(mon) || defended(mon, AD_STON))))
                return POISON;
            /* turning into slime is preferable to starvation */
            else if (fptr == &mons[PM_GREEN_SLIME] && !slimeproof(mptr))
                return starving ? ACCFOOD : POISON;
            else if (is_shapeshifter(fptr))
                return starving ? ACCFOOD : MANFOOD;
            else if (vegan(fptr))
                return herbi ? (can_give_new_mintrinsic(fptr, mon) ? DOGFOOD
                                                                   : CADAVER)
                             : MANFOOD;
            /* most humanoids will avoid cannibalism unless starving;
               arbitrary: elven types won't eat other elves even then */
            else if (humanoid(mptr) && same_race(mptr, fptr)
                     && (!is_undead(mptr) && fptr->mlet != S_KOBOLD
                         && fptr->mlet != S_ORC && fptr->mlet != S_OGRE))
                return (starving && carni && !(racial_elf(mon) || racial_drow(mon)))
                        ? ACCFOOD : TABU;
            else
                return carni ? (can_give_new_mintrinsic(fptr, mon) ? DOGFOOD
                                                                   : CADAVER)
                             : MANFOOD;
        case CLOVE_OF_GARLIC:
            return (is_undead(mptr) || is_vampshifter(mon))
                      ? TABU
                      : (herbi || starving)
                         ? ACCFOOD
                         : MANFOOD;
        case TIN:
            return metallivorous(mptr) ? ACCFOOD : MANFOOD;
        case EUCALYPTUS_LEAF:
            return (mon->msick || mon->mdiseased) ? DOGFOOD
                    : (starving || herbi) ? ACCFOOD : MANFOOD;
        case APPLE:
        case ORANGE:
        case PEAR:
        case MELON:
        case MISTLETOE:
        case KELP_FROND:
        case SLIME_MOLD:
            return herbi ? DOGFOOD : starving ? ACCFOOD : MANFOOD;
        case CARROT:
            return (herbi || mblind) ? DOGFOOD : starving ? ACCFOOD : MANFOOD;
        case BANANA:
            return (mptr->mlet == S_YETI && herbi)
                      ? DOGFOOD /* for monkey and ape (tameable), sasquatch */
                      : (herbi || starving)
                         ? ACCFOOD
                         : MANFOOD;
        case K_RATION:
        case C_RATION:
        case CRAM_RATION:
        case LEMBAS_WAFER:
        case FOOD_RATION:
            if (racial_human(mon)
                || racial_elf(mon)
                || racial_drow(mon)
                || racial_dwarf(mon)
                || racial_gnome(mon)
                || racial_orc(mon)
                || racial_hobbit(mon)
                || racial_giant(mon)
                || racial_centaur(mon)
                || racial_illithid(mon)
                || racial_tortle(mon))
                return ACCFOOD;
            /*FALLTHRU*/
        default:
            if (starving)
                return ACCFOOD;
            return (obj->otyp > SLIME_MOLD) ? (carni ? ACCFOOD : MANFOOD)
                                            : (herbi ? ACCFOOD : MANFOOD);
        }
    default:
        if (obj->otyp == AMULET_OF_STRANGULATION
            || obj->otyp == RIN_SLOW_DIGESTION)
            return TABU;
        if (mon_hates_material(mon, obj->material))
            return TABU;
        if (mptr == &mons[PM_GELATINOUS_CUBE] && is_organic(obj))
            return ACCFOOD;
        if (metallivorous(mptr) && is_metallic(obj)
            && (is_rustprone(obj) || mptr != &mons[PM_RUST_MONSTER])) {
            /* Non-rustproofed ferrous based metals are preferred. */
            return (is_rustprone(obj) && !obj->oerodeproof) ? DOGFOOD
                                                            : ACCFOOD;
        }
        if (!obj->cursed
            && obj->oclass != BALL_CLASS
            && obj->oclass != CHAIN_CLASS)
            return APPORT;
        /*FALLTHRU*/
    case ROCK_CLASS:
        return UNDEF;
    }
}

/*
 * With the separate mextra structure added in 3.6.x this always
 * operates on the original mtmp. It now returns TRUE if the taming
 * succeeded.
 */
boolean
tamedog(mtmp, obj)
struct monst *mtmp;
struct obj *obj;
{
    boolean blessed_scroll = FALSE;
    boolean same_align = (sgn(mon_aligntyp(mtmp)) == u.ualign.type);

    if (obj && (obj->oclass == SCROLL_CLASS || obj->oclass == SPBOOK_CLASS)) {
        blessed_scroll = obj->blessed ? TRUE : FALSE;
        /* the rest of this routine assumes 'obj' represents food */
        obj = (struct obj *) NULL;
    }

    /* reduce timed sleep or paralysis, leaving mtmp->mcanmove as-is
       (note: if mtmp is donning armor, this will reduce its busy time) */
    if (mtmp->mfrozen)
        mtmp->mfrozen = (mtmp->mfrozen + 1) / 2;
    /* end indefinite sleep; using distance==1 limits the waking to mtmp */
    if (mtmp->msleeping)
        wake_nearto(mtmp->mx, mtmp->my, 1); /* [different from wakeup()] */

    /* Unique monsters, monsters that covet the Amulet,
       and various other creatures (see mondata.h) aren't
       even made peaceful */
    if (non_tameable(mtmp->data))
        return FALSE;

    /* Knights can never tame dragons of differing alignment */
    if (Role_if(PM_KNIGHT) && is_dragon(mtmp->data)
        && !same_align)
        return FALSE;

    /* Dark knights cannot tame ki-rin, lawful knights cannot
       tame eldritch ki-rin */
    if (Role_if(PM_KNIGHT) && u.ualign.type == A_LAWFUL
        && mtmp->data == &mons[PM_ELDRITCH_KI_RIN])
        return FALSE;

    if (Role_if(PM_KNIGHT) && u.ualign.type == A_CHAOTIC
        && mtmp->data == &mons[PM_KI_RIN])
        return FALSE;

    /* If wielding/wearing any of the 'banes, taming becomes
       impossible */
    if (wielding_artifact(ART_STING)
        && (racial_orc(mtmp) || is_spider(mtmp->data)))
        return FALSE;

    if (wielding_artifact(ART_ORCRIST)
        && racial_orc(mtmp))
        return FALSE;

    if (wielding_artifact(ART_GLAMDRING)
        && racial_orc(mtmp))
        return FALSE;

    if (wielding_artifact(ART_GRIMTOOTH)
        && (racial_elf(mtmp) || racial_drow(mtmp)))
        return FALSE;

    if ((wielding_artifact(ART_GIANTSLAYER)
         || wielding_artifact(ART_HARBINGER))
        && racial_giant(mtmp))
        return FALSE;

    if (wielding_artifact(ART_TROLLSBANE)
        && is_troll(mtmp->data))
        return FALSE;

    if (wielding_artifact(ART_OGRESMASHER)
        && is_ogre(mtmp->data))
        return FALSE;

    if ((wielding_artifact(ART_SUNSWORD)
         || wielding_artifact(ART_HAMMER_OF_THE_GODS))
        && is_undead(mtmp->data))
        return FALSE;

    if (wielding_artifact(ART_WEREBANE)
        && is_were(mtmp->data))
        return FALSE;

    if (wielding_artifact(ART_SHADOWBLADE)
        && is_were(mtmp->data))
        return FALSE;

    /* can't really tame demons, but this is here for completeness sake */
    if ((wielding_artifact(ART_DEMONBANE)
         || wielding_artifact(ART_HAMMER_OF_THE_GODS))
        && is_demon(mtmp->data))
        return FALSE;

    /* same for angels */
    if (wielding_artifact(ART_ANGELSLAYER)
        && is_angel(mtmp->data))
        return FALSE;

    if (wielding_artifact(ART_VORPAL_BLADE)
        && is_jabberwock(mtmp->data))
        return FALSE;

    if (uarmg && uarmg->oartifact == ART_DRAGONBANE
        && is_dragon(mtmp->data))
        return FALSE;

    if (uleft && uleft->oartifact == ART_ONE_RING
        && is_wraith(mtmp->data))
        return FALSE;

    if (uright && uright->oartifact == ART_ONE_RING
        && is_wraith(mtmp->data))
        return FALSE;

    /* steeds' disposition matches their owners.
       it shouldn't be changed independently */
    if (mtmp->ridden_by)
        return FALSE;

    /* worst case, at least it'll be peaceful. */
    mtmp->mpeaceful = 1;
    set_malign(mtmp);

    /* steeds follow their riders' direction */
    if (has_erid(mtmp)) {
        ERID(mtmp)->mon_steed->mpeaceful = 1;
        set_malign(ERID(mtmp)->mon_steed);
    }

    if (flags.moonphase == FULL_MOON && night() && rn2(6) && obj
        && mtmp->data->mlet == S_DOG)
        return FALSE;

    if ((Role_if(PM_CONVICT)
         || (!Upolyd
             && Race_if(PM_DRAUGR) && !is_undead(mtmp->data)))
        && (is_domestic(mtmp->data) && !mtmp->mtame && obj)) {
        /* Domestic animals are wary of Convicts and Draugr */
        pline("%s still looks wary of you.", Monnam(mtmp));
        return FALSE;
    }

    /* If we cannot tame it, at least it's no longer afraid. */
    mtmp->mflee = 0;
    mtmp->mfleetim = 0;

    if (has_erid(mtmp)) {
        ERID(mtmp)->mon_steed->mflee = 0;
        ERID(mtmp)->mon_steed->mfleetim = 0;
    }

    /* make grabber let go now, whether it becomes tame or not */
    if (mtmp == u.ustuck) {
        if (u.uswallow)
            expels(mtmp, mtmp->data, TRUE);
        else if (!(Upolyd && sticks(youmonst.data)))
            unstuck(mtmp);
    }

    if (has_erid(mtmp) && ERID(mtmp)->mon_steed == u.ustuck) {
        if (u.uswallow)
            expels(ERID(mtmp)->mon_steed, mtmp->data, TRUE);
        else if (!(Upolyd && sticks(youmonst.data)))
            unstuck(ERID(mtmp)->mon_steed);
    }

    /* feeding it treats makes it tamer */
    if (mtmp->mtame && obj) {
        int tasty;

        if (mtmp->mcanmove && !mtmp->mconf && !mtmp->meating
            && ((tasty = dogfood(mtmp, obj)) == DOGFOOD
                || (tasty <= ACCFOOD
                    && EDOG(mtmp)->hungrytime <= monstermoves))) {
            /* pet will "catch" and eat this thrown food */
            if (canseemon(mtmp)) {
                boolean big_corpse =
                    (obj->otyp == CORPSE && obj->corpsenm >= LOW_PM
                     && mons[obj->corpsenm].msize > mtmp->data->msize);
                pline("%s catches %s%s", Monnam(mtmp), the(xname(obj)),
                      !big_corpse ? "." : ", or vice versa!");
            } else if (cansee(mtmp->mx, mtmp->my))
                pline("%s.", Tobjnam(obj, "stop"));
            /* dog_eat expects a floor object */
            place_object(obj, mtmp->mx, mtmp->my);
            (void) dog_eat(mtmp, obj, mtmp->mx, mtmp->my, FALSE);
            /* eating might have killed it, but that doesn't matter here;
               a non-null result suppresses "miss" message for thrown
               food and also implies that the object has been deleted */
            return TRUE;
        } else
            return FALSE;
    }

    /* maximum tameness is 20, only reachable via eating; if already tame but
       less than 10, taming magic might make it become tamer; blessed scroll
       or skilled spell raises low tameness by 2 or 3, uncursed by 0 or 1 */
    if (mtmp->mtame && mtmp->mtame < 10) {
        if (mtmp->mtame < rnd(10))
            mtmp->mtame++;
        if (blessed_scroll) {
            mtmp->mtame += 2;
            if (mtmp->mtame > 10)
                mtmp->mtame = 10;
        }
        return FALSE; /* didn't just get tamed */
    }
    /* pacify angry shopkeeper but don't tame him/her/it/them */
    if (mtmp->isshk) {
        make_happy_shk(mtmp, FALSE);
        return FALSE;
    }

    if (!mtmp->mcanmove
        /* monsters with conflicting structures cannot be tamed */
        || mtmp->isshk || mtmp->isgd || mtmp->ispriest || mtmp->isminion
        || is_covetous(mtmp->data) || is_human(mtmp->data)
        || racial_human(mtmp)
        || (is_demon(mtmp->data) && mtmp->data != &mons[PM_LAVA_DEMON]
            && !is_demon(raceptr(&youmonst)))
        || (obj && dogfood(mtmp, obj) >= MANFOOD))
        return FALSE;

    if (mtmp->m_id == quest_status.leader_m_id)
        return FALSE;

    /* don't allow pets to ride.
       note their steed will not become tame, though it will be peaceful.
       this allows for minor abuse with making ki-rin and maybe unicorns
       peaceful, but most steeds already have pretty low MR. */
    if (has_erid(mtmp)) {
        if (canseemon(mtmp))
            pline("%s pats %s steed and clambers off.", Monnam(mtmp), mhis(mtmp));
        separate_steed_and_rider(mtmp);
    }

    /* add the pet extension */
    if (!has_edog(mtmp)) {
        newedog(mtmp);
        initedog(mtmp, TRUE);
    } else {
        initedog(mtmp, FALSE);
    }

    if (obj) { /* thrown food */
        /* defer eating until the edog extension has been set up */
        place_object(obj, mtmp->mx, mtmp->my); /* put on floor */
        /* devour the food (might grow into larger, genocided monster) */
        if (dog_eat(mtmp, obj, mtmp->mx, mtmp->my, TRUE) == 2)
            return TRUE; /* oops, it died... */
        /* `obj' is now obsolete */
    }

    newsym(mtmp->mx, mtmp->my);
    if (attacktype(mtmp->data, AT_WEAP)) {
        mtmp->weapon_check = NEED_HTH_WEAPON;
        (void) mon_wield_item(mtmp);
    }
    return TRUE;
}

/*
 * Called during pet revival or pet life-saving.
 * If you killed the pet, it revives wild.
 * If you abused the pet a lot while alive, it revives wild.
 * If you abused the pet at all while alive, it revives untame.
 * If the pet wasn't abused and was very tame, it might revive tame.
 */
void
wary_dog(mtmp, was_dead)
struct monst *mtmp;
boolean was_dead;
{
    struct edog *edog;
    boolean quietly = was_dead;

    finish_meating(mtmp);

    if (!mtmp->mtame)
        return;
    edog = !mtmp->isminion ? EDOG(mtmp) : 0;

    /* if monster was starving when it died, undo that now */
    if (edog && edog->mhpmax_penalty) {
        mtmp->mhpmax += edog->mhpmax_penalty;
        mtmp->mhp += edog->mhpmax_penalty; /* heal it */
        edog->mhpmax_penalty = 0;
    }

    if (edog && (edog->killed_by_u == 1 || edog->abuse > 2)) {
        mtmp->mpeaceful = mtmp->mtame = 0;
        if (edog->abuse >= 0 && edog->abuse < 10)
            if (!rn2(edog->abuse + 1))
                mtmp->mpeaceful = 1;
        newsym(mtmp->mx, mtmp->my); /* update display */
        if (!quietly && cansee(mtmp->mx, mtmp->my)) {
            if (haseyes(youmonst.data)) {
                if (haseyes(mtmp->data))
                    pline("%s %s to look you in the %s.", Monnam(mtmp),
                          mtmp->mpeaceful ? "seems unable" : "refuses",
                          body_part(EYE));
                else
                    pline("%s avoids your gaze.", Monnam(mtmp));
            }
        }
    } else {
        /* chance it goes wild anyway - Pet Sematary */
        mtmp->mtame = rn2(mtmp->mtame + 1);
        if (!mtmp->mtame)
            mtmp->mpeaceful = rn2(2);
    }

    if (!mtmp->mtame) {
        if (!quietly && canspotmon(mtmp))
            pline("%s %s.", Monnam(mtmp),
                  mtmp->mpeaceful ? "is no longer tame" : "has become feral");
        newsym(mtmp->mx, mtmp->my);
        /* a life-saved monster might be leashed;
           don't leave it that way if it's no longer tame */
        if (mtmp->mleashed)
            m_unleash(mtmp, TRUE);
        if (mtmp == u.usteed)
            dismount_steed(DISMOUNT_THROWN);
    } else if (edog) {
        /* it's still a pet; start a clean pet-slate now */
        edog->revivals++;
        edog->killed_by_u = 0;
        edog->abuse = 0;
        edog->ogoal.x = edog->ogoal.y = -1;
        if (was_dead || edog->hungrytime < monstermoves + 500L)
            edog->hungrytime = monstermoves + 500L;
        if (was_dead) {
            edog->droptime = 0L;
            edog->dropdist = 10000;
            edog->whistletime = 0L;
            edog->apport = 5;
        } /* else lifesaved, so retain current values */
    }
}

void
abuse_dog(mtmp)
struct monst *mtmp;
{
    if (!mtmp->mtame)
        return;

    if (Aggravate_monster || Conflict)
        mtmp->mtame /= 2;
    else
        mtmp->mtame--;

    if (!mtmp->mtame
        && (Aggravate_monster || Conflict || rn2(EDOG(mtmp)->abuse + 1))) {
        mtmp->mpeaceful = 0;
        newsym(mtmp->mx, mtmp->my); /* update display */
    }

    if (mtmp->mtame && !mtmp->isminion)
        EDOG(mtmp)->abuse++;

    if (!mtmp->mtame && mtmp->mleashed)
        m_unleash(mtmp, TRUE);
    if (!mtmp->mtame && mtmp == u.usteed)
        dismount_steed(DISMOUNT_THROWN);

    /* don't make a sound if pet is in the middle of leaving the level */
    /* newsym isn't necessary in this case either */
    if (mtmp->mx != 0) {
        if (mtmp->mtame && rn2(mtmp->mtame))
            yelp(mtmp);
        else
            growl(mtmp); /* give them a moment's worry */

        if (!mtmp->mtame)
            newsym(mtmp->mx, mtmp->my);
    }
}

/* just entered the Astral Plane; receive tame mount if worthy */
void
gain_guardian_steed()
{
    struct monst *mtmp;
    struct obj *otmp;
    coord mm;

    Hear_again(); /* attempt to cure any deafness now (divine
                     message will be heard even if that fails) */
    if (u.ualign.record > 8) { /* fervent */
        pline("A voice whispers:");
        /* Neither Centaurs, Giants, nor Tortles can ride horses. Awww... */
        verbalize(
  "Worthy vassal, know now thy true identity!  Behold thy %s, the Red Horse!",
                  (Race_if(PM_CENTAUR)
                   || Race_if(PM_GIANT) || Race_if(PM_TORTLE)) ? "companion"
                                                               : "steed");
        mm.x = u.ux;
        mm.y = u.uy;
        if (enexto(&mm, mm.x, mm.y, &mons[PM_RED_HORSE])
            && (mtmp = makemon(&mons[PM_RED_HORSE], mm.x, mm.y, MM_EDOG)) != 0) {
            /* Too nasty for the game to unexpectedly break petless conduct on
             * the final level of the game. The Red Horse will still appear, but
             * won't be tamed. Toss some carrots at it if you want to tame it */
            if (u.uconduct.pets) {
                mtmp->mtame = 20;
                u.uconduct.pets++;
            }
            EDOG(mtmp)->apport = ACURR(A_CHA);
            /* make sure our steed isn't starving to death when we arrive */
            EDOG(mtmp)->hungrytime = 1000 + monstermoves;
            mtmp->mstrategy &= ~STRAT_APPEARMSG;
            if (!Blind)
                pline("The Red Horse appears near you.");
            else
                You_feel("the presence of the Red Horse near you.");
            /* make it strong enough vs. endgame foes */
            mtmp->mhp = mtmp->mhpmax =
                d((int) mtmp->m_lev, 10) + 30 + rnd(30);
            otmp = mksobj(SADDLE, TRUE, FALSE);
            put_saddle_on_mon(otmp, mtmp);
            bless(otmp);
            otmp->dknown = otmp->bknown = otmp->rknown = 1;
            /* chance of wearing barding */
            if (!rn2(3)) {
                struct obj *otmp2 = mksobj(rn2(2) ? SPIKED_BARDING
                                                  : BARDING_OF_REFLECTION, TRUE, FALSE);

                put_barding_on_mon(otmp2, mtmp);
                bless(otmp2);
                otmp2->dknown = otmp2->bknown = otmp2->rknown = 1;
            }
        }
    }
}

/*dog.c*/
