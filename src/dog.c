/* NetHack 5.0	dog.c	$NHDT-Date: 1781973045 2026/06/20 16:30:45 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.197 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2011. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

staticfn int pet_type(void);
staticfn int woodland_animal(void);
staticfn int elemental(void);
staticfn struct permonst * pick_familiar_pm(struct obj *, boolean);
staticfn void set_mon_lastmove(struct monst *);
staticfn int mon_leave(struct monst *) NONNULLARG1;
staticfn boolean keep_mon_accessible(struct monst *);

enum arrival {
    Before_you =  0, /* monsters kept on migrating_mons for accessibility;
                      * they haven't actually left their level */
    With_you   =  1, /* pets and level followers */
    After_you  =  2, /* regular migrating monsters */
    Wiz_arrive = -1  /* resurrect(wizard.c) */
};

void
newedog(struct monst *mtmp)
{
    if (!mtmp->mextra)
        mtmp->mextra = newmextra();
    if (!EDOG(mtmp)) {
        EDOG(mtmp) = (struct edog *) alloc(sizeof(struct edog));
        (void) memset((genericptr_t) EDOG(mtmp), 0, sizeof(struct edog));
        EDOG(mtmp)->parentmid = mtmp->m_id;
    }
}

void
free_edog(struct monst *mtmp)
{
    if (mtmp->mextra && EDOG(mtmp)) {
        free((genericptr_t) EDOG(mtmp));
        EDOG(mtmp) = (struct edog *) 0;
    }
    mtmp->mtame = 0;
}

void
initedog(struct monst *mtmp, boolean everything)
{
    struct edog *edogp = EDOG(mtmp);
    long minhungry = svm.moves + 1000L;
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
        /* edogp->hungrytime = 0L; // set below */
        edogp->ogoal.x = -1; /* force error if used before set */
        edogp->ogoal.y = -1;
        edogp->abuse = 0;
        edogp->revivals = 0;
        edogp->mhpmax_penalty = 0;
        edogp->killed_by_u = 0;
        edogp->petstrat = 0;
    } else {
        if (edogp->apport <= 0)
            edogp->apport = 1;
    }
    /* always set for newly tamed pet or feral former pet; hungrytime might
       already be higher when taming magic affects already tame monst */
    if (edogp->hungrytime < minhungry)
        edogp->hungrytime = minhungry;
    /* livelog first pet, but only if you didn't start with one (the starting
     * pet will be initialized before in_moveloop is true) */
    if (!u.uconduct.pets && program_state.in_moveloop) {
        /* "obtained" a pet rather than "tamed" it because it might have come
         * from a figurine or some other method in which it was created tame
         * using an() is safe unless it somehow becomes possible to tame a
         * unique monster */
        livelog_printf(LL_CONDUCT, "obtained %s first pet (%s)",
                       uhis(), an(mon_pmname(mtmp)));
    }
    u.uconduct.pets++;
}

staticfn int
pet_type(void)
{
    if (gu.urole.petnum != NON_PM)
        return  gu.urole.petnum;
    else if (gp.preferred_pet == 'c')
        return  PM_KITTEN;
    else if (gp.preferred_pet == 'd')
        return  PM_LITTLE_DOG;
    else
        return  rn2(2) ? PM_KITTEN : PM_LITTLE_DOG;
}

staticfn struct permonst *
pick_familiar_pm(struct obj *otmp, boolean quietly)
{
    struct permonst *pm = (struct permonst *) 0;

    if (otmp) { /* figurine; otherwise spell */
        boolean idol = is_art(otmp, ART_IDOL_OF_MOLOCH);
        int mndx = otmp->corpsenm;

        if (idol) {
            /* the Idol channels a random demon out of the mist
               rather than its fixed corpsenm species */
            mndx = ndemon(A_NONE);
            if (mndx == NON_PM) /* just in case */
                return (struct permonst *) 0;
        }
        assert(ismnum(mndx));
        pm = &mons[mndx];
        /* activating a figurine provides one way to exceed the
           maximum number of the target critter created--unless
           it has a special limit (erinys, Nazgul) */
        if ((svm.mvitals[mndx].mvflags & G_EXTINCT)
            && mbirth_limit(mndx) != MAXMONNO) {
            if (!quietly) {
                /* have just been given "You <do something with>
                   the figurine and it transforms." message */
                if (idol) {
                    if (!Blind)
                        pline_The("cloud disperses.");
                } else {
                    pline("... into a pile of dust.");
                }
            }
            return (struct permonst *) 0;
        }
    } else if (!rn2(3)) {
        pm = &mons[pet_type()];
    } else {
        int skill = spell_skilltype(SPE_CREATE_FAMILIAR);
        int max = 3 * P_SKILL(skill);

        pm = rndmonst_adj(0, max);
        if (!pm && !quietly)
            There("seems to be nothing available for a familiar.");
    }
    return pm;
}

struct monst *
make_familiar(struct obj *otmp, coordxy x, coordxy y, boolean quietly)
{
    struct permonst *pm;
    struct monst *mtmp = 0;
    int chance, trycnt = 100;
    boolean reallytame = TRUE;
    boolean idol = otmp && is_art(otmp, ART_IDOL_OF_MOLOCH);

    do {
        mmflags_nht mmflags;
        int cgend;

        if (!(pm = pick_familiar_pm(otmp, quietly)))
            break;

        /* the Idol's channeled demons arrive with their gear */
        mmflags = MM_EDOG | MM_IGNOREWATER | MM_NOMSG
                  | (idol ? 0 : NO_MINVENT);
        /* the imbued Idol's spe overloads CORPSTAT_FEMALE; its demons
           roll their own gender */
        cgend = (otmp && !idol) ? (otmp->spe & CORPSTAT_GENDER) : 0;
        mmflags |= ((cgend == CORPSTAT_FEMALE) ? MM_FEMALE
                    : (cgend == CORPSTAT_MALE) ? MM_MALE : 0L);

        mtmp = makemon(pm, x, y, mmflags);
        if (otmp) { /* figurine */
            if (!mtmp) {
                /* monster has been genocided or target spot is occupied */
                if (!quietly) {
                    if (idol) {
                        if (!Blind)
                            pline_The("cloud disperses.");
                    } else {
                        pline_The(
                           "figurine writhes and then shatters into pieces!");
                    }
                }
                break;
            } else if (mtmp->isminion) {
                /* Fixup for figurine of an Angel:  makemon() is willing to
                   create a random Angel as either an ordinary monster or as
                   a minion of random allegiance.  We don't want the latter
                   here in case it successfully becomes a pet. */
                mtmp->isminion = 0;
                free_emin(mtmp);
                /* [This could and possibly should be redone as a new
                   MM_flag passed to makemon() to suppress making a minion
                   so that no post-creation fixup would be needed.] */
            }
        }
    } while (!mtmp && --trycnt > 0);

    if (!mtmp)
        return (struct monst *) 0;

    if (idol && !quietly && !Blind) {
        pline_The("mist coagulates into the shape of %s%s.",
                  x_monnam(mtmp, ARTICLE_A, (char *) 0,
                           SUPPRESS_IT | SUPPRESS_INVISIBLE
                               | SUPPRESS_SADDLE | SUPPRESS_BARDING
                               | SUPPRESS_NAME, FALSE),
                  canspotmon(mtmp) ? "" : " and vanishes");
    }

    if (is_pool(mtmp->mx, mtmp->my) && minliquid(mtmp))
        return (struct monst *) 0;

    if (otmp) { /* figurine; resulting monster might not become a pet */
        boolean same_align = (sgn(mon_aligntyp(mtmp)) == u.ualign.type);

        chance = rn2(10); /* 0==tame, 1==peaceful, 2==hostile */
        if (chance > 2)
            chance = otmp->blessed ? 0 : !otmp->cursed ? 1 : 2;
        /* 0,1,2:  b=80%,10,10; nc=10%,80,10; c=10%,10,80 */
        if (Role_if(PM_KNIGHT) && u.ualign.type == A_LAWFUL
            && mtmp->data == &mons[PM_ELDRITCH_KI_RIN])
            chance = 2;
        if (Role_if(PM_KNIGHT) && u.ualign.type == A_CHAOTIC
            && mtmp->data == &mons[PM_KI_RIN])
            chance = 2;
        if (Role_if(PM_KNIGHT) && is_dragon(mtmp->data)
            && !same_align)
            chance = 2;
        if (Race_if(PM_AASIMAR) && is_demon(mtmp->data))
            chance = 2; /* aasimar cannot tame demons */
        if (Role_if(PM_INFIDEL) && is_angel(mtmp->data)
            && mon_aligntyp(mtmp) > 0)
            chance = 2;
        if (chance > 0) {
            reallytame = FALSE; /* not tame after all */
            if (chance == 2) {  /* hostile (cursed figurine) */
                if (!quietly)
                    You("get a bad feeling about this.");
                mtmp->mpeaceful = 0;
                set_malign(mtmp);
            }
        }
        /* if figurine has been named, give same name to the monster;
           the Idol's name never transfers to its channeled demons */
        if (has_oname(otmp) && !idol)
            mtmp = christen_monst(mtmp, ONAME(otmp));
    }
    if (reallytame)
        initedog(mtmp, TRUE);
    mtmp->msleeping = 0;
    set_malign(mtmp); /* more alignment changes */
    newsym(mtmp->mx, mtmp->my);

    /* must wield weapon immediately since pets will otherwise drop it */
    if (mtmp->mtame && attacktype(mtmp->data, AT_WEAP)) {
        mtmp->weapon_check = NEED_HTH_WEAPON;
        (void) mon_wield_item(mtmp);
    }
    return mtmp;
}

/* pick a woodland creature for the summon animal spell; the better
   the caster's skill, the more formidable the animal */
staticfn int
woodland_animal(void)
{
    if (P_SKILL(spell_skilltype(SPE_SUMMON_ANIMAL)) < P_BASIC) {
        switch (rnd(5)) {
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
        switch (rnd(5)) {
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
        switch (rnd(5)) {
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
        switch (rnd(5)) {
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

/* pick an elemental for the summon elemental spell */
staticfn int
elemental(void)
{
    switch (rnd(4)) {
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

/* from Slash'EM
   currently only used for orbs of fire/frost
   created by their respective spells */
struct monst *
make_helper(int mnum, coordxy x, coordxy y)
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
make_woodland_animal(coordxy x, coordxy y)
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
make_elemental(coordxy x, coordxy y)
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
    } else if (P_SKILL(spell_skilltype(SPE_SUMMON_ELEMENTAL))
               == P_EXPERT) {
        mtmp->mhpmax += mtmp->mhpmax;
        mtmp->mhp = mtmp->mhpmax;
    }
    set_malign(mtmp); /* more alignment changes */
    newsym(mtmp->mx, mtmp->my);

    return mtmp;
}

/* despite rather general name, used exclusively for hero's starting pet */
struct monst *
makedog(void)
{
    struct monst *mtmp;
    const char *petname;
    int pettype;

    if (gp.preferred_pet == 'n') {
        /* static init yields 0 (PM_GIANT_ANT); fix that up now */
        svc.context.startingpet_typ = NON_PM;
        return ((struct monst *) 0);
    }

    pettype = pet_type();
    /* drow heroes start with a large spider in place of a dog,
       kitten, or lesser homunculus */
    if (Race_if(PM_DROW)
        && (pettype == PM_LITTLE_DOG || pettype == PM_KITTEN
            || pettype == PM_LESSER_HOMUNCULUS))
        pettype = PM_LARGE_SPIDER;
    /* draugr get a skeletal hound and vampires a wolf cub in place
       of a dog, kitten, or lesser homunculus */
    else if (Race_if(PM_DRAUGR)
             && (pettype == PM_LITTLE_DOG || pettype == PM_KITTEN
                 || pettype == PM_LESSER_HOMUNCULUS))
        pettype = PM_SMALL_SKELETAL_HOUND;
    else if (Race_if(PM_VAMPIRE)
             && (pettype == PM_LITTLE_DOG || pettype == PM_KITTEN
                 || pettype == PM_LESSER_HOMUNCULUS))
        pettype = PM_WOLF_CUB;
    /* chaotic knights ride a lesser nightmare instead of a pony
       (draugr knights a skeletal pony); centaurs cannot ride, so
       they get a little dog companion */
    if (pettype == PM_PONY
        && u.ualign.type == A_CHAOTIC && Role_if(PM_KNIGHT))
        pettype = Race_if(PM_DRAUGR) ? PM_SKELETAL_PONY
                  : Race_if(PM_CENTAUR) ? PM_LITTLE_DOG
                    : PM_LESSER_NIGHTMARE;
    svc.context.startingpet_typ = pettype;
    petname = (pettype == PM_LITTLE_DOG) ? gd.dogname
              : (pettype == PM_KITTEN) ? gc.catname
                : (pettype == PM_PONY) ? gh.horsename
                  : (pettype == PM_LARGE_SPIDER) ? gs.spidername
                    : (pettype == PM_PSEUDODRAGON) ? gp.pseudoname
                      : (pettype == PM_LESSER_NIGHTMARE) ? gh.horsename
                        : (pettype == PM_SKELETAL_PONY) ? gh.horsename
                          : (pettype == PM_SMALL_SKELETAL_HOUND) ? gd.dogname
                            : (pettype == PM_WOLF_CUB) ? gd.dogname
                              : (pettype == PM_SEWER_RAT) ? gr.ratname
                                : (pettype == PM_LESSER_HOMUNCULUS)
                                  ? gh.homunname
                                  : (pettype == PM_HAWK) ? gh.hawkname
                                    : "";

    /* default pet names */
    if (!*petname && pettype == PM_LITTLE_DOG) {
        /* All of these names were for dogs. */
        if (Role_if(PM_CAVE_DWELLER))
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

    /* specifying NO_MINVENT prevents makemon() from having a 1% chance
       of creating a pony with an already worn saddle; dogs and cats
       aren't affected because they don't have any initial inventory
       [if anybody adds stranger pets that are expected to have such,
       they'll need to modify this] */
    mtmp = makemon(&mons[pettype], u.ux, u.uy, MM_EDOG | NO_MINVENT);

    if (!mtmp)
        return ((struct monst *) 0); /* pets were genocided [how?] */

    if (!svc.context.startingpet_mid) {
        svc.context.startingpet_mid = mtmp->m_id;
        if (!u.uroleplay.pauper) {
            /* initial horses start wearing a saddle (pauper hero excluded) */
            if (pettype == PM_PONY || pettype == PM_LESSER_NIGHTMARE
                || pettype == PM_SKELETAL_PONY) {
                /* NULL obj arg means put_saddle_on_mon()
                 * will carry out the saddle creation */
                put_saddle_on_mon((struct obj *) 0, mtmp);
            }
        }
        /* starting pet's type has been seen up close (unless PermaBlind)
           and for tourist treat it as having already been photographed */
        gb.bhitpos.x = mtmp->mx, gb.bhitpos.y = mtmp->my;
        gn.notonhead = FALSE;
        see_monster_closeup(mtmp, carrying(EXPENSIVE_CAMERA) ? TRUE : FALSE);
    } else {
        impossible("makedog() when startingpet_mid is already non-zero?");
    }

    if (!gp.petname_used++ && *petname)
        mtmp = christen_monst(mtmp, petname);

    initedog(mtmp, TRUE);
    return  mtmp;
}

staticfn void
set_mon_lastmove(struct monst *mtmp)
{
    mtmp->mlstmv = svm.moves;
}

/* record `last move time' for all monsters prior to level save so that
   mon_arrive() can catch up for lost time when they're restored later */
void
update_mlstmv(void)
{
    iter_mons(set_mon_lastmove);
}

/* note: always reset when used so doesn't need to be part of struct 'g' */
static struct monst *failed_arrivals = 0;

void
losedogs(void)
{
    struct monst *mtmp, **mprev;
    int dismissKops = 0, xyloc;
    boolean stalked = FALSE;

    failed_arrivals = 0;
    /*
     * First, scan gm.migrating_mons for shopkeepers who want to dismiss Kops,
     * and scan gm.mydogs for shopkeepers who want to retain kops.
     * Second, dismiss kops if warranted, making more room for arrival.
     * Third, replace monsters who went onto migrating_mons in order to
     * be accessible from other levels but didn't actually leave the level.
     * Fourth, place monsters accompanying the hero.
     * Last, place migrating monsters coming to this level.
     *
     * Hero might eventually be displaced (due to the third step, but
     * occurring later), which is the main reason to do the second step
     * sooner (in turn necessitating the first step, rather than combining
     * the list scans with monster placement).
     */

    /* check for returning shk(s) */
    for (mtmp = gm.migrating_mons; mtmp; mtmp = mtmp->nmon) {
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
    /* make the same check for gm.mydogs */
    for (mtmp = gm.mydogs; mtmp && dismissKops >= 0; mtmp = mtmp->nmon) {
        if (mtmp->isshk) {
            /* hostile shk might accompany hero [ESHK(mtmp)->dismiss_kops
               can't be set here; it's only used for gm.migrating_mons] */
            if (!mtmp->mpeaceful)
                dismissKops = -1;
        }
    }

    /* when a hostile shopkeeper chases hero to another level
       and then gets paid off there, get rid of summoned kops
       here now that he has returned to his shop level */
    if (dismissKops > 0)
        make_happy_shoppers(TRUE);

    /* put monsters who went onto migrating_mons in order to be accessible
       when other levels are active back to their positions on this level;
       they're handled before mydogs so that monsters accompanying the
       hero can't steal the spot that belongs to them; these migraters
       should always be able to arrive because they were present on the
       level at the time the hero left [if they can't arrive for some
       reason, mon_arrive() will put them on the 'failed_arrivals' list] */
    for (mprev = &gm.migrating_mons; (mtmp = *mprev) != 0; ) {
        xyloc = mtmp->mtrack[0].x; /* (for legibility) */
        if (mtmp->mux == u.uz.dnum && mtmp->muy == u.uz.dlevel
            && xyloc == MIGR_EXACT_XY) {
            /* remove mtmp from migrating_mons */
            *mprev = mtmp->nmon;
            mon_arrive(mtmp, Before_you);
        } else {
            mprev = &mtmp->nmon;
        }
    }

    /* place pets and/or any other monsters who accompany hero;
       any that fail to arrive (level may be full) will be moved
       first to failed_arrivals, then to migrating_mons scheduled
       to arrive back on this level if hero leaves and returns */
    while ((mtmp = gm.mydogs) != 0) {
        gm.mydogs = mtmp->nmon;
        mon_arrive(mtmp, With_you);
    }

    /* time for migrating monsters to arrive; monsters who belong on
       this level but fail to arrive get put on the failed_arrivals list
       temporarily [by mon_arrive()], then back onto the migrating_mons
       list below; a MIGR_STALK demon lord waiting at a shallower level
       of this dungeon intercepts the hero here too */
    for (mprev = &gm.migrating_mons; (mtmp = *mprev) != 0; ) {
        xyloc = mtmp->mtrack[0].x;
        if (mtmp->mux == u.uz.dnum
            && (mtmp->muy == u.uz.dlevel
                || (mtmp->muy < u.uz.dlevel && xyloc == MIGR_STALK))
            && xyloc != MIGR_EXACT_XY) {
            if (mtmp->muy < u.uz.dlevel && xyloc == MIGR_STALK)
                stalked = TRUE;
            /* remove mtmp from migrating_mons */
            *mprev = mtmp->nmon;
            /* note: if there's no room, it ends up on failed_arrivals list */
            mon_arrive(mtmp, After_you);
        } else {
            mprev = &mtmp->nmon;
        }
    }
    if (stalked) {
        if (!Deaf)
            You_hear("demonic laughter.");
        else
            You_feel("a forboding force surround you.");
    }

    /* put any monsters who couldn't arrive back on migrating_mons,
       clearing out the temporary 'failed_arrivals' list in the process */
    while ((mtmp = failed_arrivals) != 0) {
        failed_arrivals = mtmp->nmon;
        /* mon_arrive() put mtmp onto fmon, but if there wasn't room to
           arrive, relmon() was used to take it off again; put it back now
           because m_into_limbo() expects it to be there */
        mtmp->nmon = fmon;
        fmon = mtmp;
        /* set this monster to migrate back to this level if hero leaves
           and then returns */
        m_into_limbo(mtmp);
    }
}

/* called from resurrect() in addition to losedogs() */
void
mon_arrive(struct monst *mtmp, int when)
{
    struct trap *t;
    coordxy xlocale, ylocale, xyloc, xyflags;
    xint16 wander;
    int num_segs;
    boolean failed_to_place = FALSE;
    stairway *stway;
    d_level fromdlev;

    mtmp->mstate &= ~TERRAIN_FALLOUT_MASK;  /* shouldn't be set anyway */
    mtmp->mstate |= MON_STILL_ARRIVING;
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
    /* the transit is over; that includes an endgame plane-bump - clear
       its flag here because the ridden-steed arrival below never
       reaches place_monster()'s own clear */
    mtmp->mstate &= ~(MON_MIGRATING | MON_LIMBO | MON_ENDGAME_MIGR);

    /* make sure mnexto(rloc_to(set_apparxy())) doesn't use stale data */
    mtmp->mux = u.ux, mtmp->muy = u.uy;
    xyloc = mtmp->mtrack[0].x;
    xyflags = mtmp->mtrack[0].y;
    xlocale = mtmp->mtrack[1].x;
    ylocale = mtmp->mtrack[1].y;
    fromdlev.dnum = mtmp->mtrack[2].x;
    fromdlev.dlevel = mtmp->mtrack[2].y;
    mon_track_clear(mtmp);
    /* in case Protection_from_shape_changers is different now from when
       'mtmp' went onto the migrating monsters list; that's handled in
       getlev() when returning to a previously visited level and by the
       special level code for monsters specified in the level, but needed
       here for monsters migrating to a newly created level */
    restore_cham(mtmp);

    if (mtmp == u.usteed || mtmp->ridden_by) {
        if (mtmp->ridden_by) {
            /* a monster's steed stays on fmon but off the map with its
               coordinates mirroring its rider's; the rider migrates
               after its steed but arrives first, so mirror it now -
               nothing else will until the rider next moves */
            struct monst *rider = find_mid(mtmp->ridden_by, FM_FMON);

            if (rider && rider->mx) {
                mtmp->mx = rider->mx;
                mtmp->my = rider->my;
            }
            /* else the rider could not arrive and is waiting on
               failed_arrivals; when losedogs() sends it back into
               limbo, migrate_to_level()'s steed recursion takes this
               steed along - it must stay on fmon for that relmon,
               keeping the pair on the same monster list */
        }
        mtmp->mstate &= ~MON_STILL_ARRIVING;
        return; /* don't place steed on the map */
    }
    if (when == With_you) {
        /* When a monster accompanies you, sometimes it will arrive
           at your intended destination and you'll end up next to
           that spot.  This code doesn't control the final outcome;
           goto_level(do.c) decides who ends up at your target spot
           when there is a monster there too. */
        if (!MON_AT(u.ux, u.uy)
            && !rn2(mtmp->mtame ? 10 : mtmp->mpeaceful ? 5 : 2))
            rloc_to(mtmp, u.ux, u.uy);
        else
            mnexto(mtmp, RLOC_NOMSG);
        mtmp->mstate &= ~MON_STILL_ARRIVING;
        return;
    } else if (when == Wiz_arrive) {
        /* resurrect() is bringing existing wizard to harass the hero */
        xyloc = MIGR_WITH_HERO;
    }
    /*
     * The monster arrived on this level independently of the player.
     * Its coordinate fields were overloaded for use as flags that
     * specify its final destination.
     */

    if (mtmp->mlstmv < svm.moves - 1L) {
        /* heal monster for time spent in limbo */
        long nmv = svm.moves - 1L - mtmp->mlstmv;

        mon_catchup_elapsed_time(mtmp, nmv);

        /* let monster move a bit on new level (see placement code below) */
        wander = (xint16) min(nmv, 8L);
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
    case MIGR_STALK:
        /* if returning to the same level, pretend it never left, so no
           movement; if arriving on a deeper level, don't give the hero
           any breathing room */
        wander = 0;
        if (fromdlev.dlevel < u.uz.dlevel) {
            xlocale = u.ux, ylocale = u.uy;
            mtmp->mstrategy &= ~STRAT_WAITMASK;
        }
        break;
    case MIGR_STAIRS_UP:
        if ((stway = stairway_find_from(&fromdlev, FALSE)) != 0) {
            xlocale = stway->sx;
            ylocale = stway->sy;
        }
        break;
    case MIGR_STAIRS_DOWN:
        if ((stway = stairway_find_from(&fromdlev, FALSE)) != 0) {
            xlocale = stway->sx;
            ylocale = stway->sy;
        }
        break;
    case MIGR_LADDER_UP:
        if ((stway = stairway_find_from(&fromdlev, TRUE)) != 0) {
            xlocale = stway->sx;
            ylocale = stway->sy;
        }
        break;
    case MIGR_LADDER_DOWN:
        if ((stway = stairway_find_from(&fromdlev, TRUE)) != 0) {
            xlocale = stway->sx;
            ylocale = stway->sy;
        }
        break;
    case MIGR_SSTAIRS:
        if ((stway = stairway_find(&fromdlev)) != 0) {
            xlocale = stway->sx;
            ylocale = stway->sy;
        }
        break;
    case MIGR_PORTAL:
        if (In_endgame(&u.uz)) {
            /* there is no arrival portal for endgame levels */
            /* BUG[?]: for simplicity, this code relies on the fact
               that we know that the current endgame levels always
               build upwards and never have any exclusion subregion
               inside their TELEPORT_REGION settings. */
            xlocale = rn1(svu.updest.hx - svu.updest.lx + 1, svu.updest.lx);
            ylocale = rn1(svu.updest.hy - svu.updest.ly + 1, svu.updest.ly);
            break;
        }
        /* find the arrival portal */
        for (t = gf.ftrap; t; t = t->ntrap)
            if (t->ttyp == MAGIC_PORTAL)
                break;
        if (t) {
            xlocale = t->tx, ylocale = t->ty;
            break;
        } else if (iflags.debug_fuzzer
                   && ((stway = stairway_find_dir(!builds_up(&u.uz))) != 0
                       || (stway = stairway_find_dir(builds_up(&u.uz)))
                              != 0)) {
            /* debugfuzzer fallback: land on any vertical connection
               (stairs, ladder, or branch stairs) when the arrival
               portal is missing, e.g. a one-way branch portal whose
               destination has no return portal */
            xlocale = stway->sx, ylocale = stway->sy;
            break;
        } else if (!iflags.debug_fuzzer
                   && !Is_stronghold(&u.uz)
                   && !(u.uevent.qexpelled
                        && (Is_qstart(&u.uz0) || Is_qstart(&u.uz)))) {
            /* one-way branch portals legitimately lack a return
               portal; log it in normal play but never panic the
               fuzzer - fall through to random placement, mirroring
               goto_level in do.c. The Castle is exempt: monsters
               transiting Purgatory's one-way exit portal arrive
               there with no return portal by design */
            impossible("mon_arrive: no corresponding portal?");
        }
        FALLTHROUGH;
        /*FALLTHRU*/
    default:
    case MIGR_RANDOM:
        xlocale = ylocale = 0;
        break;
    }

    if ((mtmp->migflags & MIGR_LEFTOVERS) != 0L) {
        /* Pick up the rest of the MIGR_TO_SPECIES objects */
        if (gm.migrating_objs)
            deliver_obj_to_mon(mtmp, 0, DF_ALL);
    }

    if (xlocale && wander) {
        /* monster moved a bit; pick a nearby location */
        /* mnearto() deals w/stone, et al */
        char *r = in_rooms(xlocale, ylocale, 0);

        if (r && *r) {
            coord c;

            /* somexy() handles irregular rooms */
            if (somexy(&svr.rooms[*r - ROOMOFFSET], &c))
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
        failed_to_place = !mnearto(mtmp, xlocale, ylocale, FALSE, RLOC_NOMSG);
    else
        failed_to_place = !rloc(mtmp, RLOC_NOMSG);

    if (failed_to_place) {
        if (when != Wiz_arrive)
            /* losedogs() will deal with this */
            relmon(mtmp, &failed_arrivals);
        else /* when==Wiz_arrive => not being called by losedogs() */
            m_into_limbo(mtmp);
    }
    mtmp->mstate &= ~MON_STILL_ARRIVING;
}

/* heal monster for time spent elsewhere */
void
mon_catchup_elapsed_time(
    struct monst *mtmp,
    long nmv) /* number of moves */
{
    int imv = 0; /* avoid zillions of casts and lint warnings */

#if defined(DEBUG) || (NH_DEVEL_STATUS != NH_STATUS_RELEASED)

    if (nmv < 0L) { /* crash likely... */
        panic("catchup from future time?");
        /*NOTREACHED*/
        return;
    } else if (nmv == 0L) { /* safe, but shouldn't happen */
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
        if (imv >= mtmp->msummoned - 1)
            mtmp->msummoned = 2;
        else
            mtmp->msummoned -= imv;
    }
    if (mtmp->msicktime) {
        if (imv >= mtmp->msicktime)
            mtmp->msicktime = 1;
        else
            mtmp->msicktime -= imv;
    }
    if (mtmp->mdiseasetime) {
        if (imv >= mtmp->mdiseasetime)
            mtmp->mdiseasetime = 1;
        else
            mtmp->mdiseasetime -= imv;
    }
    if (mtmp->mbindtimer) {
        if (imv >= mtmp->mbindtimer)
            mtmp->mbindtimer = 1;
        else
            mtmp->mbindtimer -= imv;
    }
    /* spell-conferred wards run down too; clamp to 1 so the expiry
       pass strips the ward, with its message, on the next tick */
    if (mtmp->mreflecttime) {
        if (imv >= mtmp->mreflecttime)
            mtmp->mreflecttime = 1;
        else
            mtmp->mreflecttime -= imv;
    }
    if (mtmp->mbarkskintime) {
        if (imv >= mtmp->mbarkskintime)
            mtmp->mbarkskintime = 1;
        else
            mtmp->mbarkskintime -= imv;
    }
    if (mtmp->mstoneskintime) {
        if (imv >= mtmp->mstoneskintime)
            mtmp->mstoneskintime = 1;
        else
            mtmp->mstoneskintime -= imv;
    }

    /* withering monsters by rights ought to keep withering while
       off-level, but it brings up a host of problems to have a monster
       die in this function (if the player were responsible, would they
       get experience for the kill and potentially level up just by
       returning to the level? should messages such as "You have a sad
       feeling" or vampire polyself/rise again print upon arrival?)
       Instead, just don't affect their hp or withering status; they
       will begin re-withering when the hero comes back */

    /* might recover from temporary trouble */
    if (mtmp->mtrapped && rn2(imv + 1) > 40 / 2)
        mtmp->mtrapped = 0;
    if (mtmp->mconf && rn2(imv + 1) > 50 / 2)
        mtmp->mconf = 0;
    if (mtmp->mstun && rn2(imv + 1) > 10 / 2)
        mtmp->mstun = 0;

    /* might finish eating or be able to use special ability again */
    if (mtmp->meating) {
        if (imv > mtmp->meating)
            finish_meating(mtmp);
        else
            mtmp->meating -= imv;
    }
    /* spellcasting 'power' */
    if (imv > mtmp->mspec_used)
        mtmp->mspec_used = 0;
    else
        mtmp->mspec_used -= imv;
    /* breath/spit cooldown */
    if (imv > mtmp->mbreath_attk)
        mtmp->mbreath_attk = 0;
    else
        mtmp->mbreath_attk -= imv;
    /* telekinesis cooldown */
    if (imv > mtmp->mtk_attk)
        mtmp->mtk_attk = 0;
    else
        mtmp->mtk_attk -= imv;
    /* breaking boulders */
    if (imv > mtmp->mbreakboulder)
        mtmp->mbreakboulder = 0;
    else
        mtmp->mbreakboulder -= imv;

    /* reduce tameness for every 150 moves you are separated */
    if (mtmp->mtame) {
        struct obj *barding;
        int wilder = (imv + 75) / 150;

        /* an Ithilmar-barded steed keeps its tameness; skip only the
           ladder below, not the hit point recovery and lastmove refresh
           that follow */
        if ((barding = which_armor(mtmp, W_BARDING)) != 0
            && barding->oartifact == ART_ITHILMAR) {
            /* tameness isn't affected */
        } else if (mtmp->mtame > wilder) {
            mtmp->mtame -= wilder; /* less tame */
        } else if (mtmp->mtame > rn2(wilder)) {
            mtmp->mtame = 0; /* untame */
        } else {
            mtmp->mtame = mtmp->mpeaceful = 0; /* hostile! */
            set_malign(mtmp);
            newsym(mtmp->mx, mtmp->my); /* update display */
        }
    }
    /* check to see if it would have died as a pet; if so, go wild instead
     * of dying the next time we call dog_move()
     */
    if (mtmp->mtame && !mtmp->isminion
        && (carnivorous(mtmp->data) || herbivorous(mtmp->data))) {
        struct edog *edog = EDOG(mtmp);

        if ((svm.moves > edog->hungrytime + 500 && mtmp->mhp < 3)
            || (svm.moves > edog->hungrytime + 750)) {
            mtmp->mtame = mtmp->mpeaceful = 0;
            set_malign(mtmp);
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

        for (otmp = fobj; otmp; otmp = otmp->nobj)
            if (otmp->otyp == AMULET_OF_YENDOR
                || otmp->otyp == FAKE_AMULET_OF_YENDOR) {
                obj_extract_self(otmp);
                /* mpickobj() may merge otmp into a stack and free it;
                   the break below ensures otmp is not touched after */
                (void) mpickobj(mtmp, otmp);
                break;
            }
    }

    /* recover lost hit points unless withering; only the undead can
       regenerate in the Valley of the Dead */
    if (!mtmp->mwither && (!Is_valley(&u.uz) || is_undead(r_data(mtmp)))) {
        if (!mon_prop(mtmp, REGENERATION))
            imv /= 20;
        healmon(mtmp, imv, 0);
    }

    set_mon_lastmove(mtmp);
}

/* bookkeeping when mtmp is about to leave the current level;
   common to keepdogs() and migrate_to_level() */
staticfn int
mon_leave(struct monst *mtmp)
{
    struct obj *obj;
    int num_segs = 0; /* return value */

    /* set minvent's obj->no_charge to 0 */
    for (obj = mtmp->minvent; obj; obj = obj->nobj) {
        if (Has_contents(obj))
            picked_container(obj); /* does the right thing */
        obj->no_charge = 0;
    }

    /* if this is a shopkeeper, clear the 'resident' field of her shop;
       if/when she returns, it will be set back by mon_arrive()  */
    if (mtmp->isshk)
        set_residency(mtmp, TRUE);

    /* if this is a long worm, handle its tail segments before mtmp itself;
       we pass possibly truncated segment count to caller via return value  */
    if (mtmp->wormno) {
        int cnt = count_wsegs(mtmp), mx = mtmp->mx, my = mtmp->my;

        /* since monst->wormno is overloaded to hold the number of
           tail segments during migration, a very long worm with
           more segments than can fit in that field gets truncated */
        num_segs = min(cnt, MAX_NUM_WORMS - 1);
        wormgone(mtmp);
        /* put the head back; note: mtmp might not be on the map if this
           is happening during a failed attempt to migrate to this level */
        if (mx)
            place_monster(mtmp, mx, my);
    }

    return num_segs;
}

/* when hero leaves a level, some monsters should be placed on the
   migrating_mons list instead of being stashed inside the level's file */
staticfn boolean
keep_mon_accessible(struct monst *mon)
{
    /* the Wizard is kept accessible so that his harassment can fetch
       him instead of creating a new instance but also so that he can
       be put back at his current location if hero returns to his level */
    if (mon->iswiz)
        return TRUE;
    /* monsters with special attachment to a particular level only need
       to be kept accessible when on some other level */
    if (mon->mextra
        && ((mon->isshk && !on_level(&u.uz, &ESHK(mon)->shoplevel))
            || (mon->ispriest && !on_level(&u.uz, &EPRI(mon)->shrlevel))
            || (mon->isgd && !on_level(&u.uz, &EGD(mon)->gdlevel))))
        return TRUE;
    /* normal monsters go into the level save file instead of being held
       on the migrating_mons list for off-level accessibility */
    return FALSE;
}

/* called when you move to another level */
void
keepdogs(
    boolean pets_only) /* true for ascension or final escape */
{
    struct monst *mtmp, *mtmp2;

    for (mtmp = fmon; mtmp; mtmp = mtmp2) {
        mtmp2 = mtmp->nmon;
        if (DEADMONSTER(mtmp))
            continue;
        /* a mounted monster and its steed stay where they are */
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
            finish_meating(mtmp);
            mtmp->msleeping = 0;
            mtmp->mfrozen = 0;
            mtmp->mcanmove = 1;
        }
        if (((monnear(mtmp, u.ux, u.uy) && levl_follower(mtmp))
             /* the wiz will level t-port from anywhere to chase
                the amulet; if you don't have it, will chase you
                only if in range. -3. */
             || (u.uhave.amulet && mtmp->iswiz))
            && (!helpless(mtmp)
                /* eg if level teleport or new trap, steed has no control
                   to avoid following */
                || (mtmp == u.usteed))
            /* monster won't follow if it hasn't noticed you yet */
            && !(mtmp->mstrategy & STRAT_WAITFORU)) {
            int num_segs;
            boolean stay_behind = FALSE;

            if (mtmp->mtrapped) {
                (void) mintrap(mtmp, NO_TRAP_FLAGS); /* try to escape */
                /* fire/polymorph/land-mine traps can kill a low-HP pet;
                   don't drag a dead mtmp through to relmon() below */
                if (DEADMONSTER(mtmp))
                    continue;
            }
            if (mtmp == u.usteed) {
                struct obj *obj, *nobj;

                /* make sure steed is eligible to accompany hero */
                mtmp->mtrapped = 0; /* escape trap */
                mtmp->meating = 0;  /* terminate eating */
                /* Drop items with level-crossing restrictions
                   that shouldn't travel between levels on a
                   steed: the Amulet and invocation items.
                   Unlike mdrop_special_objs(), do NOT drop the
                   quest artifact or other artifacts - the hero
                   is actively riding their mount and controls
                   what it carries */
                for (obj = mtmp->minvent; obj; obj = nobj) {
                    nobj = obj->nobj;
                    if (obj->otyp == AMULET_OF_YENDOR
                        || obj->otyp == SPE_BOOK_OF_THE_DEAD
                        || obj->otyp == CANDELABRUM_OF_INVOCATION
                        || obj->otyp == BELL_OF_OPENING)
                        mdrop_obj(mtmp, obj, TRUE);
                }
            } else if (!pets_only && has_edog(mtmp)
                       && (EDOG(mtmp)->petstrat & PETSTRAT_STAY)) {
                /* pet ordered to stay on this level */
                if (canseemon(mtmp))
                    pline_mon(mtmp, "%s stays behind.", Monnam(mtmp));
                stay_behind = TRUE;
            } else if (mtmp->meating || mtmp->mtrapped
                       || mtmp->mentangled) {
                /* loyal pets (tameness 15+) will follow even while eating */
                if (mtmp->meating && !mtmp->mtrapped && !mtmp->mentangled
                    && mtmp->mtame >= 15) {
                    if (canseemon(mtmp))
                        pline_mon(mtmp, "%s follows, still eating.",
                                  Monnam(mtmp));
                    /* meating persists - continues eating on arrival */
                } else {
                    if (canseemon(mtmp))
                        pline_mon(mtmp, "%s is still %s.", Monnam(mtmp),
                                  mtmp->meating ? "eating"
                                                : mtmp->mentangled ? "entangled"
                                                                   : "trapped");
                    stay_behind = TRUE;
                }
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

            /* prepare to take mtmp off the map */
            num_segs = mon_leave(mtmp);
            /* take off map and move mtmp from fmon list to mydogs */
            relmon(mtmp, &gm.mydogs); /* mtmp->mx,my retain current value */
            mtmp->mx = mtmp->my = 0; /* mx==0 implies migrating */
            mtmp->wormno = num_segs;
            mtmp->mlstmv = svm.moves;
        } else if (keep_mon_accessible(mtmp)) {
            /* we want to be able to find the Wizard when his next
               resurrection chance comes up, but have him resume his
               present location if player returns to this level before
               that time; also needed for monsters (shopkeeper, temple
               priest, vault guard) who have level data in mon->mextra
               in case #wizmakemap is used to replace their home level
               while they're away from it */
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
migrate_to_level(
    struct monst *mtmp,
    xint16 tolev, /* destination level */
    xint16 xyloc, /* MIGR_xxx destination xy location: */
    coord *cc)   /* optional destination coordinates */
{
    d_level new_lev;
    coordxy xyflags;
    coordxy mx = mtmp->mx, my = mtmp->my; /* <mx,my> needed below */
    int num_segs; /* count of worm segments */

    /* recursive call to levelport a monster's steed along with it */
    if (has_erid(mtmp) && ERID(mtmp)->mon_steed)
        migrate_to_level(ERID(mtmp)->mon_steed, tolev, xyloc, cc);

    if (mtmp->mleashed) {
        mtmp->mtame--;
        m_unleash(mtmp, TRUE);
    }

    /* prepare to take mtmp off the map */
    num_segs = mon_leave(mtmp);
    /* take off map and move mtmp from fmon list to migrating_mons */
    mtmp->mstate &= ~TERRAIN_FALLOUT_MASK;
    relmon(mtmp, &gm.migrating_mons); /* mtmp->mx,my retain their value */
    mtmp->mstate |= MON_MIGRATING;

    new_lev.dnum = ledger_to_dnum((xint16) tolev);
    new_lev.dlevel = ledger_to_dlev((xint16) tolev);
    /* overload mtmp->[mx,my], mtmp->[mux,muy], and mtmp->mtrack[] as
       destination codes */
    xyflags = (depth(&new_lev) < depth(&u.uz)); /* 1 => up */
    if (In_W_tower(mx, my, &u.uz))
        xyflags |= 2;
    mtmp->wormno = num_segs;
    mtmp->mlstmv = svm.moves;
    mtmp->mtrack[2].x = u.uz.dnum; /* migrating from this dungeon */
    mtmp->mtrack[2].y = u.uz.dlevel; /* migrating from this dungeon level */
    mtmp->mtrack[1].x = cc ? cc->x : mx;
    mtmp->mtrack[1].y = cc ? cc->y : my;
    mtmp->mtrack[0].x = xyloc;
    mtmp->mtrack[0].y = xyflags;
    mtmp->mux = new_lev.dnum;
    mtmp->muy = new_lev.dlevel;
    mtmp->mx = mtmp->my = 0; /* mx==0 implies migrating */

    /* don't extinguish a mobile light; it still exists but has changed
       from local (monst->mx > 0) to global (mx==0, not on this level) */
    if (emits_light(mtmp->data))
        vision_recalc(0);
}

/* when entering the endgame, levels from the dungeon and its branches are
   discarded because they can't be reached again; do the same for monsters
   and objects scheduled to migrate to those levels */
void
discard_migrations(void)
{
    struct monst *mtmp, **mprev;
    struct obj *otmp, **oprev;
    d_level dest;

    for (mprev = &gm.migrating_mons; (mtmp = *mprev) != 0; ) {
        dest.dnum = mtmp->mux;
        dest.dlevel = mtmp->muy;
        /* the Wizard is kept regardless of location so that he is
           ready to be brought back; nothing should be scheduled to
           migrate to the endgame but if we find such, we'll keep it */
        if (mtmp->iswiz || In_endgame(&dest)) {
            mprev = &mtmp->nmon; /* keep mtmp on migrating_mons */
        } else {
            *mprev = mtmp->nmon; /* remove mtmp from migrating_mons */
            mtmp->nmon = 0;
            discard_minvent(mtmp, FALSE);
            /* bypass mongone() and its call to m_detach() plus dmonsfree() */
            if (emits_light(mtmp->data))
                del_light_source(LS_MONSTER, monst_to_any(mtmp));
            dealloc_monst(mtmp);
        }
    }

    /* objects get similar treatment */
    for (oprev = &gm.migrating_objs; (otmp = *oprev) != 0; ) {
        dest.dnum = otmp->ox;
        dest.dlevel = otmp->oy;
        /* there is no special case like the Wizard (certainly not the
           Amulet; the hero has to be carrying it to enter the endgame
           which triggers the call to this routine); again we don't
           expect any objects to be migrating to the endgame but will
           keep any we find so that they could be delivered */
        if (In_endgame(&dest)) {
            oprev = &otmp->nobj; /* keep otmp on migrating_objs */
        } else {
            /* bypass obj_extract_self() */
            *oprev = otmp->nobj; /* remove otmp from migrating_objs */
            otmp->nobj = 0;
            otmp->where = OBJ_FREE;
            otmp->owornmask = 0L; /* overloaded for destination usage;
                                   * obfree() will complain if nonzero */
            /*
             * obfree(otmp,)
             *  -> dealloc_obj(otmp)
             *      -> obj_stop_timers(otmp)
             *      -> del_light_source(LS_OBJECT, obj_to_any(otmp))
             */
            obfree(otmp, (struct obj *) 0); /* releases any contents too */
        }
    }
}

/* returns the quality of an item of food; the lower the better;
   fungi and ghouls will eat even tainted food */
int
dogfood(struct monst *mon, struct obj *obj)
{
    struct permonst *mptr = mon->data, *fptr;
    boolean carni = carnivorous(mptr), herbi = herbivorous(mptr),
            starving, mblind;
    int fx;

    if (obj->opoisoned
        && !(resists_poison(mon) || defended(mon, AD_DRST)))
        return POISON;
    if (is_quest_artifact(obj) || obj_resists(obj, 0, 95))
        return obj->cursed ? TABU : APPORT;

    if (is_art(obj, ART_EYE_OF_VECNA))
        return TABU;

    switch (obj->oclass) {
    case FOOD_CLASS:
        fx = (obj->otyp == CORPSE || obj->otyp == TIN || obj->otyp == EGG)
                /* corpsenm might be NON_PM (special tin, unhatchable egg) */
                ? obj->corpsenm
                : NON_PM;
        /* mons[NUMMONS] is a valid array entry, though not a valid monster;
         * predicate tests against it will fail */
        fptr = &mons[(ismnum(fx)) ? fx : NUMMONS];

        if (obj->otyp == CORPSE && is_rider(fptr))
            return TABU;
        if ((obj->otyp == CORPSE || obj->otyp == EGG)
            && flesh_petrifies(fptr) /* c*ckatrice or Medusa */
            && !(resists_ston(mon) || defended(mon, AD_STON)))
            return POISON;
        if (obj->otyp == LUMP_OF_ROYAL_JELLY
            && mon->data == &mons[PM_KILLER_BEE]) {
            struct monst *mtmp = find_pmmonst(PM_QUEEN_BEE);

            /* if there's a queen bee on the level, don't eat royal jelly;
               if there isn't, do eat it and grow into a queen */
            return !mtmp ? DOGFOOD : TABU;
        }

        /* vampires drain the blood from fresh corpses; matches the
           player's is_edible() check - allow partly-eaten corpses
           that still have blood remaining. Tame vampire pets are
           selective like non-vampire pets: only drain when not
           deeply satiated (DOG_SATIATED gate); satiated pets walk
           past blood corpses */
        /* TODO-EVILHACK: master lets a satiated vampire pet still
           drain a corpse that would grant it a new intrinsic; that
           gate ports with the pet-intrinsics subsystem */
        if (is_vampire(mptr)) {
            boolean blood_corpse =
                (obj->otyp == CORPSE
                 && has_blood(&safe_mons(obj->corpsenm))
                 && (!obj->odrained
                     || obj->oeaten > (unsigned) drain_level(obj))
                 && peek_at_iced_corpse_age(obj)
                        + blood_freshness(obj) >= svm.moves);

            if (!blood_corpse)
                return TABU;
            if (mon->mtame && !mon->isminion
                && EDOG(mon)->hungrytime >= svm.moves + DOG_SATIATED)
                return MANFOOD;
            return DOGFOOD;
        }

        if (!carni && !herbi)
            return obj->cursed ? UNDEF : APPORT;

        /* a starving pet will eat almost anything */
        starving = (mon->mtame && !mon->isminion
                    && EDOG(mon)->mhpmax_penalty);
        /* even carnivores will eat carrots if they're temporarily blind */
        mblind = (!mon->mcansee && haseyes(mon->data));

        /* ghouls and racial zombies prefer old corpses and unhatchable
           eggs, yum! they'll eat fresh non-veggy corpses and hatchable
           eggs when starving; they never eat stone-to-flesh'd meat */
        if (mptr == &mons[PM_GHOUL] || racial_zombie(mon)) {
            if (obj->otyp == CORPSE)
                return (peek_at_iced_corpse_age(obj) + 50L <= svm.moves
                        && !(fx == PM_LIZARD || fx == PM_LICHEN)) ? DOGFOOD
                       : (starving && !vegan(fptr)) ? ACCFOOD
                         : POISON;
            if (obj->otyp == EGG)
                return stale_egg(obj) ? CADAVER : starving ? ACCFOOD : POISON;
            return TABU;
        }

        /* gnomes hate eggs */
        if (obj->otyp == EGG && racial_gnome(mon))
            return TABU;

        switch (obj->otyp) {
        case TRIPE_RATION:
        case MEATBALL:
        case MEAT_RING:
        case MEAT_STICK:
        case HUGE_CHUNK_OF_MEAT:
            return carni ? DOGFOOD : MANFOOD;
        case EGG:
            if (obj->corpsenm == PM_PYROLISK && !likes_fire(mptr))
                return POISON;
            return carni ? CADAVER : MANFOOD;
        case CORPSE:
            if ((peek_at_iced_corpse_age(obj) + 50L <= svm.moves
                 && !(fx == PM_LIZARD || fx == PM_LICHEN)
                 && mptr->mlet != S_FUNGUS)
                || (acidic(fptr)
                    && !(resists_acid(mon) || defended(mon, AD_ACID)))
                || (poisonous(fptr)
                    && !(resists_poison(mon) || defended(mon, AD_DRST)))
                || (obj->zombie_corpse && !(resists_sick(mon)
                                            || defended(mon, AD_DISE))))
                return POISON;
            /* avoid polymorph unless starving or abused (in which case the
               pet will consider it for a chance to become more powerful) */
            else if (polyfood(obj) && mon->mtame > 1 && !starving)
                return MANFOOD;
            else if (vegan(fptr))
                return herbi ? CADAVER : MANFOOD;
            /* most humanoids will avoid cannibalism unless starving;
               arbitrary: elven types won't eat other elves even then */
            else if (humanoid(mptr) && same_race(mptr, fptr)
                     && (!is_undead(mptr) && fptr->mlet != S_KOBOLD
                         && fptr->mlet != S_ORC && fptr->mlet != S_OGRE))
                return (starving && carni
                        && !(racial_elf(mon) || racial_drow(mon)))
                       ? ACCFOOD : TABU;
            else
                return carni ? CADAVER : MANFOOD;
        case GLOB_OF_GREEN_SLIME: /* other globs use the default case */
            /* turning into slime is preferable to starvation */
            return (starving || slimeproof(mon->data)) ? ACCFOOD : POISON;
        case CLOVE_OF_GARLIC:
            return (is_undead(mptr) || is_vampshifter(mon)) ? TABU
                   : (herbi || starving) ? ACCFOOD
                     : MANFOOD;
        case TIN:
            return metallivorous(mptr) ? ACCFOOD : MANFOOD;
        case EUCALYPTUS_LEAF:
            /* a sick pet will seek out the cure */
            return (mon->msick || mon->mdiseased) ? DOGFOOD
                   : (starving || herbi) ? ACCFOOD
                     : MANFOOD;
        case APPLE:
            return herbi ? DOGFOOD : starving ? ACCFOOD : MANFOOD;
        case CARROT:
            return (herbi || mblind) ? DOGFOOD : starving ? ACCFOOD : MANFOOD;
        case BANANA:
            /* monkeys and apes (tamable) plus sasquatch prefer these,
               yetis will only will only eat them if starving */
            return (mptr->mlet == S_YETI && herbi) ? DOGFOOD
                   : (herbi || starving) ? ACCFOOD
                     : MANFOOD;
        case K_RATION:
        case C_RATION:
        case CRAM_RATION:
        case LEMBAS_WAFER:
        case FOOD_RATION:
            /* prepared rations suit the humanoid races */
            if (racial_human(mon) || racial_elf(mon) || racial_drow(mon)
                || racial_dwarf(mon) || racial_gnome(mon)
                || racial_orc(mon) || racial_hobbit(mon)
                || racial_giant(mon) || racial_centaur(mon)
                || racial_illithid(mon) || racial_tortle(mon)
                || racial_aasimar(mon))
                return ACCFOOD;
            FALLTHROUGH;
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
            /* Non-rustproofed ferrous-based metals are preferred. */
            return (is_rustprone(obj) && !obj->oerodeproof) ? DOGFOOD
                                                            : ACCFOOD;
        }
        if (!obj->cursed
            && obj->oclass != BALL_CLASS
            && obj->oclass != CHAIN_CLASS)
            return APPORT;
        FALLTHROUGH;
        /*FALLTHRU*/
    case ROCK_CLASS:
        return UNDEF;
    }
}

/*
 * tamedog() used to return the monster, which might have changed address
 * if a new one was created in order to allocate the edog extension.
 * With the separate mextra structure added in 3.6.x it always operates
 * on the original mtmp.  It now returns TRUE if the taming succeeded.
 */
boolean
tamedog(
    struct monst *mtmp,
    struct obj *obj, /* food or scroll/spell */
    boolean givemsg)
{
    boolean blessed_scroll = FALSE;
    struct monst *holder;

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

    /* The Wiz, Medusa and the quest nemeses aren't even made peaceful. */
    if (mtmp->iswiz || mtmp->data == &mons[PM_MEDUSA]
        || (mtmp->data->mflags3 & M3_WANTSARTI))
        return FALSE;

    /* Knights can never tame dragons of differing alignment */
    if (Role_if(PM_KNIGHT) && is_dragon(mtmp->data)
        && sgn(mon_aligntyp(mtmp)) != u.ualign.type)
        return FALSE;

    /* Dark knights cannot tame ki-rin, lawful knights cannot
       tame eldritch ki-rin */
    if (Role_if(PM_KNIGHT) && u.ualign.type == A_LAWFUL
        && mtmp->data == &mons[PM_ELDRITCH_KI_RIN])
        return FALSE;

    if (Role_if(PM_KNIGHT) && u.ualign.type == A_CHAOTIC
        && mtmp->data == &mons[PM_KI_RIN])
        return FALSE;

    /* Aasimars cannot tame demons or devils */
    if (Race_if(PM_AASIMAR) && is_demon(mtmp->data))
        return FALSE;

    /* Infidels cannot tame lawful angels */
    if (Role_if(PM_INFIDEL) && is_angel(mtmp->data)
        && mon_aligntyp(mtmp) > 0)
        return FALSE;

    /* wielding or wearing any of the 'banes makes taming its hated
       race impossible; it isn't even made peaceful */
    if (wielding_artifact(ART_STING)
        && (racial_orc(mtmp) || is_spider(mtmp->data)))
        return FALSE;

    if (wielding_artifact(ART_ORCRIST) && racial_orc(mtmp))
        return FALSE;

    if (wielding_artifact(ART_GLAMDRING) && racial_orc(mtmp))
        return FALSE;

    if (wielding_artifact(ART_GRIMTOOTH)
        && (racial_elf(mtmp) || racial_drow(mtmp)))
        return FALSE;

    if ((wielding_artifact(ART_GIANTSLAYER)
         || wielding_artifact(ART_HARBINGER))
        && racial_giant(mtmp))
        return FALSE;

    if (wielding_artifact(ART_TROLLSBANE) && is_troll(mtmp->data))
        return FALSE;

    if (wielding_artifact(ART_OGRESMASHER) && is_ogre(mtmp->data))
        return FALSE;

    if ((wielding_artifact(ART_SUNSWORD)
         || wielding_artifact(ART_HAMMER_OF_THE_GODS))
        && is_undead(mtmp->data))
        return FALSE;

    if (wielding_artifact(ART_WEREBANE) && is_were(mtmp->data))
        return FALSE;

    if (wielding_artifact(ART_SHADOWBLADE) && is_were(mtmp->data))
        return FALSE;

    /* can't really tame demons, but this is here for completeness sake */
    if ((wielding_artifact(ART_DEMONBANE)
         || wielding_artifact(ART_HAMMER_OF_THE_GODS))
        && is_demon(mtmp->data))
        return FALSE;

    /* same for angels */
    if (wielding_artifact(ART_ANGELSLAYER)
        && (is_angel(mtmp->data) || is_aasimar(mtmp->data)))
        return FALSE;

    if (wielding_artifact(ART_VORPAL_BLADE) && is_jabberwock(mtmp->data))
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

    /* steeds' disposition matches their riders'; it shouldn't be
       changed independently */
    if (mtmp->ridden_by)
        return FALSE;

    /* worst case, at least it'll be peaceful. */
    if (givemsg && !mtmp->mpeaceful && canspotmon(mtmp)) {
        pline_mon(mtmp, "%s seems %s.", Monnam(mtmp),
              Hallucination ? "really chill" : "more amiable");
        givemsg = FALSE; /* don't give another message below */
    }
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
         || (!Upolyd && Race_if(PM_DRAUGR) && !is_undead(mtmp->data)))
        && is_domestic(mtmp->data) && !mtmp->mtame && obj) {
        /* domestic animals are wary of Convicts and the walking dead */
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

    /* make grabber let go now, whether it becomes tame or not; a
       rider's steed lets go along with it */
    if ((holder = ustuck_self_or_steed(mtmp)) != 0) {
        if (u.uswallow)
            expels(holder, holder->data, TRUE);
        else if (!(Upolyd && sticks(gy.youmonst.data)))
            unstuck(holder);
    }

    /* feeding it treats makes it tamer */
    if (mtmp->mtame && obj) {
        int tasty;

        if (mtmp->mcanmove && !mtmp->mconf && !mtmp->meating
            && ((tasty = dogfood(mtmp, obj)) == DOGFOOD
                || (tasty <= ACCFOOD
                    && EDOG(mtmp)->hungrytime <= svm.moves))) {
            /* pet will "catch" and eat this thrown food */
            if (canseemon(mtmp)) {
                boolean big_corpse =
                    (obj->otyp == CORPSE && ismnum(obj->corpsenm)
                     && mons[obj->corpsenm].msize > mtmp->data->msize);
                pline_mon(mtmp, "%s catches %s%s",
                          Monnam(mtmp), the(xname(obj)),
                         !big_corpse ? "." : ", or vice versa!");
            } else if (cansee(mtmp->mx, mtmp->my))
                pline("%s.", Tobjnam(obj, "stop"));
            /* treats improve your ability to handle pets */
            use_skill(P_PET_HANDLING, 1);
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
        /* monsters with conflicting structures cannot be tamed
           [note: the various mextra structures don't actually conflict
           with each other anymore] */
        || mtmp->isshk || mtmp->isgd || mtmp->ispriest || mtmp->isminion
        || is_covetous(mtmp->data) || is_human(mtmp->data)
        || racial_human(mtmp)
        || (is_demon(mtmp->data) && mtmp->data != &mons[PM_LAVA_DEMON]
            && !is_demon(raceptr(&gy.youmonst)))
        || (obj && dogfood(mtmp, obj) >= MANFOOD))
        return FALSE;

    if (mtmp->m_id == svq.quest_status.leader_m_id)
        return FALSE;

    /* don't allow pets to ride.
       note their steed will not become tame, though it will be peaceful.
       this allows for minor abuse with making ki-rin and maybe unicorns
       peaceful, but most steeds already have pretty low MR */
    if (has_erid(mtmp)) {
        if (canseemon(mtmp))
            pline("%s pats %s steed and clambers off.", Monnam(mtmp),
                  mhis(mtmp));
        separate_steed_and_rider(mtmp);
        /* the separation rloc's the rider and re-runs mintrap at its
           new spot, which can kill it or migrate it off the level;
           either way there is nothing here left to tame */
        if (DEADMONSTER(mtmp) || mon_offmap(mtmp))
            return FALSE;
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

    if (givemsg && canspotmon(mtmp))
        pline_mon(mtmp, "%s seems quite %s.", Monnam(mtmp),
              Hallucination ? "approachable" : "friendly");

    newsym(mtmp->mx, mtmp->my);
    if (mtmp->wormno)
        redraw_worm(mtmp);
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
wary_dog(struct monst *mtmp, boolean was_dead)
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
            if (haseyes(gy.youmonst.data)) {
                if (haseyes(mtmp->data))
                    pline_mon(mtmp,
                             "%s %s to look you in the %s.", Monnam(mtmp),
                             mtmp->mpeaceful ? "seems unable" : "refuses",
                             body_part(EYE));
                else
                    pline_mon(mtmp, "%s avoids your gaze.", Monnam(mtmp));
            }
        }
    } else {
        /* chance it goes wild anyway - Pet Sematary */
        mtmp->mtame = rn2(mtmp->mtame + 1);
        if (!mtmp->mtame)
            mtmp->mpeaceful = rn2(2);
    }

    if (!mtmp->mtame) {
        /* mpeaceful may have just transitioned via either branch above;
           recompute cached malign before any AI gate reads it */
        set_malign(mtmp);
        if (!quietly && canspotmon(mtmp))
            pline_mon(mtmp, "%s %s.", Monnam(mtmp),
                  mtmp->mpeaceful ? "is no longer tame" : "has become feral");
        newsym(mtmp->mx, mtmp->my);
        /* a life-saved monster might be leashed;
           don't leave it that way if it's no longer tame */
        if (mtmp->mleashed)
            m_unleash(mtmp, TRUE);
        /* the hero's steed is not thrown off here: a life-saved steed is
           still dying (a starving one may hold a few hit points from the
           penalty refund above), and a landing that kills it again would
           run a second death inside this one; moveloop throws the hero
           off once it lives (dismount_untamed_steed()) */
    } else if (edog) {
        /* it's still a pet; start a clean pet-slate now */
        edog->revivals++;
        edog->killed_by_u = 0;
        edog->abuse = 0;
        edog->ogoal.x = edog->ogoal.y = -1;
        if (was_dead || edog->hungrytime < svm.moves + 500L)
            edog->hungrytime = svm.moves + 500L;
        if (was_dead) {
            edog->droptime = 0L;
            edog->dropdist = 10000;
            edog->whistletime = 0L;
            edog->apport = 5;
        } /* else lifesaved, so retain current values */
    }
}

void
abuse_dog(struct monst *mtmp)
{
    if (!mtmp->mtame)
        return;

    if (Aggravate_monster || Conflict)
        mtmp->mtame /= 2;
    else
        mtmp->mtame--;

    if (mtmp->mtame && !mtmp->isminion)
        EDOG(mtmp)->abuse++;

    if (!mtmp->mtame && mtmp->mleashed)
        m_unleash(mtmp, TRUE);

    /* don't make a sound if pet is in the middle of leaving the level */
    /* newsym isn't necessary in this case either */
    if (mtmp->mx != 0) {
        if (mtmp->mtame && rn2(mtmp->mtame))
            yelp(mtmp);
        else
            growl(mtmp); /* give them a moment's worry */

        if (!mtmp->mtame) {
            newsym(mtmp->mx, mtmp->my);
            if (mtmp->wormno) {
                redraw_worm(mtmp);
            }
        }
    }
}

/*dog.c*/
