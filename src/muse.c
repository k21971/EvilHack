/* NetHack 5.0	muse.c	$NHDT-Date: 1781973057 2026/06/20 16:30:57 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.248 $ */
/*      Copyright (C) 1990 by Ken Arromdee                         */
/* NetHack may be freely redistributed.  See license for details.  */

/*
 * Monster item usage routines.
 */

#include "hack.h"

/* Let monsters use magic items.  Arbitrary assumptions: Monsters only use
 * scrolls when they can see, monsters know when wands have 0 charges,
 * monsters cannot recognize if items are cursed are not, monsters which
 * are confused don't know not to read scrolls, etc....
 */

staticfn int precheck(struct monst *, struct obj *);
staticfn void mzapwand(struct monst *, struct obj *, boolean) NONNULLPTRS;
staticfn void mplayhorn(struct monst *, struct obj *, boolean) NONNULLPTRS;
staticfn void mreadmsg(struct monst *, struct obj *) NONNULLPTRS;
staticfn void mquaffmsg(struct monst *, struct obj *) NONNULLPTRS;
staticfn boolean m_use_healing(struct monst *);
staticfn boolean m_sees_sleepy_soldier(struct monst *);
staticfn void m_tele(struct monst *, boolean, boolean, int);
staticfn boolean m_next2m(struct monst *);
staticfn void reveal_trap(struct trap *, boolean);
staticfn int mon_escape(struct monst *, boolean);
staticfn struct obj *m_carrying_arti_reflecting(struct monst *);
staticfn boolean linedup_chk_corpse(coordxy, coordxy);
staticfn boolean hero_behind_chokepoint(struct monst *);
staticfn boolean mon_has_friends(struct monst *);
staticfn boolean mon_likes_objpile_at(struct monst *mtmp, coordxy x, coordxy y) NONNULLARG1;
staticfn int mbhitm(struct monst *, struct obj *);
staticfn void buzz_force_miss(int, int, coordxy, coordxy, int, int);
staticfn boolean fhito_loc(struct obj *obj, coordxy x, coordxy y,
                           int (*fhito)(OBJ_P, OBJ_P));
staticfn void mbhit(struct monst *, int, int (*)(MONST_P, OBJ_P),
                  int (*)(OBJ_P, OBJ_P), struct obj *);
staticfn struct permonst *muse_newcham_mon(struct monst *);
staticfn boolean spell_is_learnable(short);
staticfn boolean mcan_learn_spell(struct monst *, struct obj *);
staticfn struct obj *find_spellbook_recurse(struct monst *, struct obj *,
                                            int);
staticfn void monster_uncurse_items(struct monst *, boolean);
staticfn void you_aggravate(struct monst *);
#if 0
staticfn boolean necrophiliac(struct obj *, boolean);
#endif
staticfn void mon_consume_unstone(struct monst *, struct obj *, boolean,
                                boolean);
staticfn boolean mcould_eat_tin(struct monst *);
staticfn boolean muse_unslime(struct monst *, struct obj *, struct trap *,
                            boolean);
staticfn int cures_sliming(struct monst *, struct obj *);
staticfn boolean green_mon(struct monst *);

/* generic item finder framework: the find_* selectors scan monster
   inventory including the contents of carried containers by handing
   each item to a per-category checker callback */
enum find_category {
    FIND_DEFENSIVE = 0,
    FIND_OFFENSIVE = 1,
    FIND_MISC = 2
};

struct find_context {
    struct monst *target;    /* offensive: who we're attacking */
    boolean reflection_skip; /* offensive: skip reflectable items */
    struct trap *t;          /* defensive: trap at monster's location */
    boolean stuck;           /* defensive: stuck to player */
    struct obj *helmet;      /* offensive: monster's helmet */
};

/* callback returns MUSE_* type if item is usable, 0 otherwise */
typedef int (*item_checker_fn)(struct monst *, struct obj *, int,
                               struct find_context *);

staticfn boolean find_items_recurse(struct monst *, struct obj *, int,
                                    item_checker_fn,
                                    struct find_context *) NONNULLARG1;
staticfn int check_defensive_item(struct monst *, struct obj *, int,
                                  struct find_context *) NONNULLPTRS;
staticfn int check_offensive_item(struct monst *, struct obj *, int,
                                  struct find_context *) NONNULLPTRS;
staticfn int check_misc_item(struct monst *, struct obj *, int,
                             struct find_context *) NONNULLPTRS;

/* Any preliminary checks which may result in the monster being unable to use
 * the item.  Returns 0 if nothing happened, 2 if the monster can't do
 * anything (i.e. it teleported) and 1 if it's dead.
 */
staticfn int
precheck(struct monst *mon, struct obj *obj)
{
    boolean vis;

    if (!obj)
        return 0;
    vis = cansee(mon->mx, mon->my);

    /* the finders can select items inside carried containers; take
       the chosen item out first - the removal uses up this turn */
    if (obj->where == OBJ_CONTAINED) {
        struct obj *container = obj->ocontainer;
        char contnr_nam[BUFSZ];
        boolean nearby;
        int howfar;

        /* prevent accessing locked containers, or containers
           that are empty or non-existent */
        if (!container || !Has_contents(container)
            || container->olocked) {
            gm.m.has_defense = 0;
            gm.m.has_offense = 0;
            gm.m.has_misc = 0;
            return 0;
        }

        /* using a cursed bag of holding = bad */
        if (Is_mbag(container) && container->cursed) {
            gm.m.has_defense = 0;
            gm.m.has_offense = 0;
            gm.m.has_misc = 0;
            return 0;
        }

        howfar = mdistu(mon);
        nearby = (howfar <= 7 * 7);
        container->cknown = 0; /* hero no longer knows container's contents
                                * even if [attempted] removal is observed */
        /* xname sets dknown, distant_name doesn't */
        Strcpy(contnr_nam, the(nearby ? xname(container)
                                      : distant_name(container, xname)));

        /* remove obj from container */
        obj_extract_self(obj); /* reduces container's weight */
        container->owt = weight(container);

        if (vis) {
            if (!nearby) /* not close by */
                Norep("%s rummages through %s.", Monnam(mon), contnr_nam);
            else
                pline_mon(mon, "%s removes %s from %s.", Monnam(mon),
                          ansimpleoname(obj), contnr_nam);
        } else if (!Deaf && nearby) {
            Norep("You hear something rummaging through %s.",
                  ansimpleoname(container));
        }

        if (container->otyp == ICE_BOX)
            removed_from_icebox(obj); /* resume rotting for corpse */
        (void) mpickobj(mon, obj);
        check_gear_next_turn(mon);
        return 2;
    }

    if (obj->oclass == POTION_CLASS) {
        coord cc;
        static const char *const empty = "The potion turns out to be empty.";
        struct monst *mtmp;

        if (objdescr_is(obj, "milky")) {
            if (!(svm.mvitals[PM_GHOST].mvflags & G_GONE)
                && !rn2(POTION_OCCUPANT_CHANCE(svm.mvitals[PM_GHOST].born))) {
                if (!enexto(&cc, mon->mx, mon->my, &mons[PM_GHOST]))
                    return 0;
                mquaffmsg(mon, obj);
                m_useup(mon, obj);
                mtmp = makemon(&mons[PM_GHOST], cc.x, cc.y, MM_NOMSG);
                if (!mtmp) {
                    if (vis)
                        pline1(empty);
                } else {
                    if (vis) {
                        pline(
                            "As %s opens the bottle, an enormous %s emerges!",
                              mon_nam(mon),
                              Hallucination ? rndmonnam(NULL)
                                            : (const char *) "ghost");
                        if (has_free_action(mon))
                            pline("%s stiffens momentarily.", Monnam(mon));
                        else
                            pline("%s is frightened to death,"
                                  " and unable to move.",
                                  Monnam(mon));
                    }
                    if (!has_free_action(mon))
                        paralyze_monst(mon, 3);
                }
                return 2;
            }
        }
        if (objdescr_is(obj, "smoky")
            && !(svm.mvitals[PM_DJINNI].mvflags & G_GONE)
            && !rn2(POTION_OCCUPANT_CHANCE(svm.mvitals[PM_DJINNI].born))) {
            if (!enexto(&cc, mon->mx, mon->my, &mons[PM_DJINNI]))
                return 0;
            mquaffmsg(mon, obj);
            m_useup(mon, obj);
            mtmp = makemon(&mons[PM_DJINNI], cc.x, cc.y, MM_NOMSG);
            if (!mtmp) {
                if (vis)
                    pline1(empty);
            } else {
                if (vis)
                    pline_mon(mtmp, "In a cloud of smoke, %s emerges!", a_monnam(mtmp));
                pline("%s speaks.", vis ? Monnam(mtmp) : Something);
                /* I suspect few players will be upset that monsters */
                /* can't wish for wands of death here.... */
                SetVoice(mtmp, 0, 80, 0);
                if (rn2(2)) {
                    verbalize("You freed me!");
                    mtmp->mpeaceful = 1;
                    set_malign(mtmp);
                } else {
                    verbalize("It is about time.");
                    if (vis)
                        pline("%s vanishes.", Monnam(mtmp));
                    mongone(mtmp);
                }
            }
            return 2;
        }
    }
    if (obj->oclass == WAND_CLASS && obj->cursed
        && !rn2(WAND_BACKFIRE_CHANCE)) {
        int dam = d(obj->spe + 2, 6);

        /* 3.6.1: no Deaf filter; 'if' message doesn't warrant it, 'else'
           message doesn't need it since You_hear() has one of its own */
        if (vis) {
            pline_mon(mon, "%s zaps %s, which suddenly explodes!", Monnam(mon),
                  an(xname(obj)));
        } else {
            /* same near/far threshold as mzapwand() */
            int range = couldsee(mon->mx, mon->my) /* 9 or 5 */
                           ? (BOLT_LIM + 1) : (BOLT_LIM - 3);

            Soundeffect(se_zap_then_explosion, 100);
            You_hear("a zap and an explosion %s.",
                     (mdistu(mon) <= range * range)
                        ? "nearby" : "in the distance");
        }
        m_useup(mon, obj);
        mon->mhp -= dam;
        if (DEADMONSTER(mon)) {
            monkilled(mon, "", AD_RBRE);
            return 1;
        }
        gm.m.has_defense = gm.m.has_offense = gm.m.has_misc = 0;
        /* Only one needed to be set to 0 but the others are harmless */
    }
    return 0;
}

/* when a monster zaps a wand give a message, deduct a charge, and if it
   isn't directly seen, remove hero's memory of the number of charges */
staticfn void
mzapwand(
    struct monst *mtmp,
    struct obj *otmp,
    boolean self)
{
    if (otmp->spe < 1) {
        impossible("Mon zapping wand with %d charges?", otmp->spe);
        return;
    }
    if (!canseemon(mtmp)) {
        int range = couldsee(mtmp->mx, mtmp->my) /* 9 or 5 */
                       ? (BOLT_LIM + 1) : (BOLT_LIM - 3);

        Soundeffect(se_zap, 100);
        You_hear("a %s zap.", (mdistu(mtmp) <= range * range)
                                 ? "nearby" : "distant");
        unknow_object(otmp); /* hero loses info when unseen obj is used */
    } else if (self) {
        pline("%s with %s!",
              monverbself(mtmp, Monnam(mtmp), "zap", (char *) 0),
              doname(otmp));
    } else {
        pline_mon(mtmp, "%s zaps %s!", Monnam(mtmp), an(xname(otmp)));
        stop_occupation();
    }
    otmp->spe -= 1;
}

/* similar to mzapwand() but for magical horns (only instrument mons play) */
staticfn void
mplayhorn(
    struct monst *mtmp,
    struct obj *otmp,
    boolean self)
{
    char *objnamp, objbuf[BUFSZ];

    if (!canseemon(mtmp)) {
        int range = couldsee(mtmp->mx, mtmp->my) /* 9 or 5 */
                       ? (BOLT_LIM + 1) : (BOLT_LIM - 3);

        Soundeffect(se_horn_being_played, 50);
        You_hear("a horn being played %s.",
                 (mdistu(mtmp) <= range * range)
                    ? "nearby" : "in the distance");
        unknow_object(otmp); /* hero loses info when unseen obj is used */
    } else if (self) {
        observe_object(otmp);
        objnamp = xname(otmp);
        if (strlen(objnamp) >= QBUFSZ)
            objnamp = simpleonames(otmp);
        Sprintf(objbuf, "a %s directed at", objnamp);
        /* "<mon> plays a <horn> directed at himself!" */
        pline("%s!", monverbself(mtmp, Monnam(mtmp), "play", objbuf));
        makeknown(otmp->otyp); /* (wands handle this slightly differently) */
    } else {
        observe_object(otmp);
        objnamp = xname(otmp);
        if (strlen(objnamp) >= QBUFSZ)
            objnamp = simpleonames(otmp);
        pline("%s %s %s directed at you!",
              /* monverbself() would adjust the verb if hallucination made
                 subject plural; stick with singular here, at least for now */
              Monnam(mtmp), "plays", an(objnamp));
        makeknown(otmp->otyp);
        stop_occupation();
    }
    otmp->spe -= 1; /* use a charge */
}

/* see or hear a monster reading a scroll;
   when scroll hasn't been seen, its label is revealed unless hero is deaf */
staticfn void
mreadmsg(struct monst *mtmp, struct obj *otmp)
{
    char onambuf[BUFSZ];
    boolean vismon = canseemon(mtmp),
            tpindicator = (!vismon && sensemon(mtmp));

    if (!vismon && Deaf)
        return; /* no feedback */

    observe_object(otmp); /* seeing/hearing scroll read reveals its label */
    Strcpy(onambuf, singular(otmp, vismon ? doname : ansimpleoname));

    if (vismon) {
        /* directly see the monster reading the scroll */
        pline_mon(mtmp, "%s reads %s!", Monnam(mtmp), onambuf);
    } else { /* !Deaf, otherwise we wouldn't reach here */
        char blindbuf[BUFSZ];
        boolean similar = same_race(gy.youmonst.data, mtmp->data),
                uniqmon = ((mtmp->data->geno & G_UNIQ) != 0
                           /* shopkeepers aren't unique monsters but since
                              they have distinct names, treat them as such */
                           || mtmp->isshk),
                recognize = (!Hallucination
                             && (mtmp->meverseen || (similar && !uniqmon)));
        /* describe unseen monster accurately when not hallucinating if it
           has ever been seen or is the same race as the hero (not yet seen
           unique monsters excepted) */
        int mflags = (SUPPRESS_INVISIBLE | SUPPRESS_SADDLE
                      | SUPPRESS_BARDING
                      | (recognize ? SUPPRESS_IT : AUGMENT_IT));

        if (sensemon(mtmp)) {
            tpindicator = TRUE;
        } else if (couldsee(mtmp->mx, mtmp->my) && mdistu(mtmp) <= 10 * 10) {
            /* monster can't be seen or sensed; hero might be blind or monster
               might be at a spot that isn't in view or might be invisible;
               remember it if the spot is within line of sight and relatively
               close */
            map_invisible(mtmp->mx, mtmp->my);
        }

        Snprintf(blindbuf, sizeof blindbuf, "reading %s", onambuf);
        strsubst(blindbuf, "reading a scroll labeled",
                 mtmp->mconf ? "attempting to incant" : "incant");
        You_hear("%s %s.",
                 x_monnam(mtmp, ARTICLE_A, (char *) 0, mflags, FALSE),
                 blindbuf);
        if (tpindicator)
            flash_mon(mtmp);
    }
    if (mtmp->mconf) /* (note: won't get if not seen and hero can't hear) */
        pline("Being confused, %s mispronounces the magic words...",
              vismon ? mon_nam(mtmp) : mhe(mtmp));
}

staticfn void
mquaffmsg(struct monst *mtmp, struct obj *otmp)
{
    if (canseemon(mtmp)) {
        observe_object(otmp);
        pline_mon(mtmp, "%s drinks %s!", Monnam(mtmp), singular(otmp, doname));
    } else if (!Deaf) {
        Soundeffect(se_mon_chugging_potion, 25);
        You_hear("a chugging sound.");
    }
}

/* Defines for various types of stuff.  The order in which monsters prefer
 * to use them is determined by the order of the code logic, not the
 * numerical order in which they are defined.
 */
#define MUSE_SCR_TELEPORTATION 1
#define MUSE_WAN_TELEPORTATION_SELF 2
#define MUSE_POT_HEALING 3
#define MUSE_POT_EXTRA_HEALING 4
#define MUSE_WAN_DIGGING 5
#define MUSE_TRAPDOOR 6
#define MUSE_TELEPORT_TRAP 7
#define MUSE_UPSTAIRS 8
#define MUSE_DOWNSTAIRS 9
#define MUSE_WAN_CREATE_MONSTER 10
#define MUSE_SCR_CREATE_MONSTER 11
#define MUSE_UP_LADDER 12
#define MUSE_DN_LADDER 13
#define MUSE_SSTAIRS 14
#define MUSE_WAN_TELEPORTATION 15
#define MUSE_BUGLE 16
#define MUSE_UNICORN_HORN 17
#define MUSE_POT_FULL_HEALING 18
#define MUSE_LIZARD_CORPSE 19
#define MUSE_WAN_UNDEAD_TURNING 20 /* also an offensive item */
#define MUSE_SPE_TELEPORT_AWAY_SELF 21 /* learned spell, no item */
#define MUSE_SPE_TELEPORT_AWAY 22      /* learned spell, no item */
#define MUSE_EUCALYPTUS_LEAF 23
/*
#define MUSE_INNATE_TPT 9999
 * We cannot use this.  Since monsters get unlimited teleportation, if they
 * were allowed to teleport at will you could never catch them.  Instead,
 * assume they only teleport at random times, despite the inconsistency
 * that if you polymorph into one you teleport at will.
 */

staticfn boolean
m_use_healing(struct monst *mtmp)
{
    struct obj *obj;

    if ((obj = m_carrying(mtmp, POT_FULL_HEALING)) != 0) {
        gm.m.defensive = obj;
        gm.m.has_defense = MUSE_POT_FULL_HEALING;
        return TRUE;
    }
    if ((obj = m_carrying(mtmp, POT_EXTRA_HEALING)) != 0) {
        gm.m.defensive = obj;
        gm.m.has_defense = MUSE_POT_EXTRA_HEALING;
        return TRUE;
    }
    if ((obj = m_carrying(mtmp, POT_HEALING)) != 0) {
        gm.m.defensive = obj;
        gm.m.has_defense = MUSE_POT_HEALING;
        return TRUE;
    }
    if (mtmp->msick || mtmp->mdiseased) {
        if ((obj = m_carrying(mtmp, EUCALYPTUS_LEAF)) != 0) {
            gm.m.defensive = obj;
            gm.m.has_defense = MUSE_EUCALYPTUS_LEAF;
            return TRUE;
        }
    }
    return FALSE;
}

/* return TRUE if monster mtmp can see at least one sleeping soldier */
staticfn boolean
m_sees_sleepy_soldier(struct monst *mtmp)
{
    coordxy x = mtmp->mx, y = mtmp->my;
    coordxy xx, yy;
    struct monst *mon;

    /* Distance is arbitrary.  What we really want to do is
     * have the soldier play the bugle when it sees or
     * remembers soldiers nearby...
     */
    for (xx = x - 3; xx <= x + 3; xx++)
        for (yy = y - 3; yy <= y + 3; yy++) {
            if (!isok(xx, yy) || (xx == x && yy == y))
                continue;
            if ((mon = m_at(xx, yy)) != 0 && is_mercenary(mon->data)
                && mon->data != &mons[PM_GUARD]
                && helpless(mon))
                return TRUE;
        }
    return FALSE;
}

staticfn void
m_tele(
    struct monst *mtmp, /* monst that might be teleported */
    boolean vismon,     /* can see it */
    boolean oseen,      /* have seen the object that triggered this */
    int how)            /* type of that object */
{
    if (tele_restrict(mtmp)) { /* mysterious force... */
        if (vismon && how)     /* mentions 'teleport' */
            makeknown(how);
        /* monster learns that teleportation isn't useful here */
        if (noteleport_level(mtmp))
            mon_learns_traps(mtmp, TELEP_TRAP_SET);
    } else if ((mon_has_amulet(mtmp) || On_W_tower_level(&u.uz)) && !rn2(3)) {
        if (vismon)
            pline_mon(mtmp, "%s seems disoriented for a moment.", Monnam(mtmp));
    } else if (Iniceq && mtmp->data == &mons[PM_KATHRYN_THE_ICE_QUEEN]) {
        if (vismon) {
            pline_mon(mtmp, "A powerful curse prevents %s from teleporting!",
                      mon_nam(mtmp));
            verbalize("Nooooo!");
        }
    } else {
        /* teleport monster 'mtmp' */
        if (how) {
            /* teleporation has been triggered by an object */
            if (oseen)
                makeknown(how);
            (void) rloc(mtmp, RLOC_MSG);
        } else {
            /* monster is voluntarily entering a teleporation trap; use the
               trap instead of rloc() in case it sends 'victim' to a vault */
            mtmp->mx = gt.trapx, mtmp->my = gt.trapy;
            (void) mintrap(mtmp, FORCETRAP);
        }
    }
}

/* return TRUE if monster mtmp has another monster next to it.
 * Called from find_defensive() where it is limited to Is_knox()
 * only, otherwise you could trap two monsters next to each other
 * in a boulder fort, and they would be happy to stay in there. */
staticfn boolean
m_next2m(struct monst *mtmp)
{
    coordxy x, y;
    struct monst *m2;

    if (DEADMONSTER(mtmp) || mon_offmap(mtmp))
        return FALSE;

    for (x = mtmp->mx - 1; x <= mtmp->mx + 1; x++)
        for (y = mtmp->my - 1; y <= mtmp->my + 1; y++) {
            if (!isok(x,y))
                continue;
            if ((m2 = m_at(x, y)) && m2 != mtmp)
                return TRUE;
        }
    return FALSE;
}

/* Generic item finder */
staticfn boolean
find_items_recurse(
    struct monst *mtmp,
    struct obj *start,
    int cat, /* enum find_category */
    item_checker_fn checker,
    struct find_context *ctx)
{
    struct obj *obj, *nextobj;
    int *has_ptr;
    struct obj **item_ptr;
    int result;

    /* set pointers based on category */
    switch (cat) {
    case FIND_DEFENSIVE:
        has_ptr = &gm.m.has_defense;
        item_ptr = &gm.m.defensive;
        break;
    case FIND_OFFENSIVE:
        has_ptr = &gm.m.has_offense;
        item_ptr = &gm.m.offensive;
        break;
    case FIND_MISC:
        has_ptr = &gm.m.has_misc;
        item_ptr = &gm.m.misc;
        break;
    default:
        return FALSE;
    }

    for (obj = start; obj; obj = nextobj) {
        nextobj = obj->nobj;

        /* early random break for defensive only:
           don't always use the same selection pattern */
        if (cat == FIND_DEFENSIVE && *has_ptr && !rn2(3))
            break;

        /* Container recursion.
         * BAG_OF_TRICKS is excluded because Is_container() covers it
         * (otyp range check) but it has no real contents - reaching in
         * spawns monsters. The two rare bags are excluded as well: a
         * bag of treasure is meant to stay sealed once a monster has
         * one, and a bag of devouring is a disposal unit that should
         * never be rummaged. Locked containers and cursed bags of
         * holding are skipped up front: precheck() refuses to extract
         * from them, and the thrown-potion path never runs precheck,
         * so recursing would let potions be hurled through a lock */
        if (Is_container(obj) && obj->otyp != BAG_OF_TRICKS
            && obj->otyp != BAG_OF_TREASURE
            && obj->otyp != BAG_OF_DEVOURING
            && !obj->olocked
            && !(Is_mbag(obj) && obj->cursed)) {
            (void) find_items_recurse(mtmp, obj->cobj, cat, checker, ctx);
            continue;
        }

        /* check this item via callback */
        result = checker(mtmp, obj, *has_ptr, ctx);
        if (result) {
            *item_ptr = obj;
            *has_ptr = result;
        }
    }

    return (boolean) !!*has_ptr;
}

/* Select a defensive item/action for a monster.  Returns TRUE iff one is
   found. */
boolean
find_defensive(struct monst *mtmp, boolean tryescape)
{
    struct obj *obj;
    struct trap *t;
    int fraction;
    coordxy x = mtmp->mx, y = mtmp->my;
    boolean stuck = (mtmp == u.ustuck),
            immobile = (mtmp->data->mmove == 0);
    stairway *stway;

    gm.m.defensive = (struct obj *) 0;
    gm.m.has_defense = 0;

    if (is_animal(mtmp->data) || mindless(mtmp->data))
        return FALSE;
    if (!tryescape && dist2(x, y, mtmp->mux, mtmp->muy) > 25)
        return FALSE;
    if (tryescape && Is_knox(&u.uz)
        && !m_next2u(mtmp) && m_next2m(mtmp))
        return FALSE;
    if (u.uswallow && stuck)
        return FALSE;

    /*
     * Since unicorn horns don't get used up, the monster would look
     * silly trying to use the same cursed horn round after round,
     * so skip cursed unicorn horns.
     *
     * Unicorns use their own horns; they're excluded from inventory
     * scanning by nohands().  Ki-rin is depicted in the AD&D Monster
     * Manual with same horn as a unicorn, so let it use its horn too.
     * is_unicorn() doesn't include it; the class differs and it has
     * no interest in gems.
     */
    if (mtmp->mconf || mtmp->mstun || !mtmp->mcansee
        || mtmp->msick || mtmp->mdiseased) {
        obj = 0;
        if (!nohands(mtmp->data)) {
            for (obj = mtmp->minvent; obj; obj = obj->nobj)
                if (obj->otyp == UNICORN_HORN && !obj->cursed)
                    break;
        }
        if (obj || is_unicorn(mtmp->data) || mtmp->data == &mons[PM_KI_RIN]) {
            gm.m.defensive = obj;
            gm.m.has_defense = MUSE_UNICORN_HORN;
            return TRUE;
        }
    }

    /* sick monsters prioritize a cure before other defenses */
    if (mtmp->msick || mtmp->mdiseased) {
        for (obj = mtmp->minvent; obj; obj = obj->nobj) {
            if (!nohands(mtmp->data)) {
                /* priority: full healing > extra healing > blessed
                   healing */
                if (obj->otyp == POT_FULL_HEALING) {
                    gm.m.defensive = obj;
                    gm.m.has_defense = MUSE_POT_FULL_HEALING;
                    return TRUE;
                } else if (obj->otyp == POT_EXTRA_HEALING
                           && !obj->cursed) {
                    gm.m.defensive = obj;
                    gm.m.has_defense = MUSE_POT_EXTRA_HEALING;
                    return TRUE;
                } else if (obj->otyp == POT_HEALING && obj->blessed) {
                    gm.m.defensive = obj;
                    gm.m.has_defense = MUSE_POT_HEALING;
                    return TRUE;
                }
            }
            if (obj->otyp == EUCALYPTUS_LEAF) {
                gm.m.defensive = obj;
                gm.m.has_defense = MUSE_EUCALYPTUS_LEAF;
                return TRUE;
            }
        }
    }

    if (mtmp->mconf || mtmp->mstun) {
        struct obj *liztin = 0;

        for (obj = mtmp->minvent; obj; obj = obj->nobj) {
            if (obj->otyp == CORPSE && obj->corpsenm == PM_LIZARD) {
                gm.m.defensive = obj;
                gm.m.has_defense = MUSE_LIZARD_CORPSE;
                return TRUE;
            } else if (obj->otyp == TIN && obj->corpsenm == PM_LIZARD) {
                liztin = obj;
            }
        }
        /* confused or stunned monster might not be able to open tin */
        if (liztin && mcould_eat_tin(mtmp) && rn2(3)) {
            gm.m.defensive = liztin;
            /* tin and corpse ultimately end up being handled the same */
            gm.m.has_defense = MUSE_LIZARD_CORPSE;
            return TRUE;
        }
    }

    /* It so happens there are two unrelated cases when we might want to
     * check specifically for healing alone.  The first is when the monster
     * is blind (healing cures blindness).  The second is when the monster
     * is peaceful; then we don't want to flee the player, and by
     * coincidence healing is all there is that doesn't involve fleeing.
     * These would be hard to combine because of the control flow.
     * Pestilence won't use healing even when blind.
     */
    if ((!mtmp->mcansee || mtmp->msick || mtmp->mdiseased)
        && !nohands(mtmp->data)
        && mtmp->data != &mons[PM_PESTILENCE]) {
        if (m_use_healing(mtmp))
            return TRUE;
    }

    /* monsters aren't given wands of undead turning but if they
       happen to have picked one up, use it against corpse wielder;
       when applicable, use it now even if 'mtmp' isn't wounded */
    if (!mtmp->mpeaceful && !nohands(mtmp->data)
        && uwep && uwep->otyp == CORPSE
        && touch_petrifies(&mons[uwep->corpsenm])
        && !poly_when_stoned(mtmp->data)
        && !(resists_ston(mtmp) || defended(mtmp, AD_STON))
        && lined_up(mtmp)) { /* only lines up if distu range is within 5*5 */
        /* could use m_carrying(), then nxtobj() when matching wand
           is empty, but direct traversal is actually simpler here */
        for (obj = mtmp->minvent; obj; obj = obj->nobj)
            if (obj->otyp == WAN_UNDEAD_TURNING && obj->spe > 0) {
                gm.m.defensive = obj;
                gm.m.has_defense = MUSE_WAN_UNDEAD_TURNING;
                return TRUE;
            }
    }

    if (!tryescape) {
        /* do we try to heal? */
        fraction = u.ulevel < 10 ? 5 : u.ulevel < 14 ? 4 : 3;
        if (mtmp->mhp >= mtmp->mhpmax
            || (mtmp->mhp >= 10 && mtmp->mhp * fraction >= mtmp->mhpmax))
            return FALSE;

        if (mtmp->mpeaceful) {
            if (!nohands(mtmp->data)) {
                if (m_use_healing(mtmp))
                    return TRUE;
            }
            return FALSE;
        }
    }

    if (stuck || immobile || mtmp->mtrapped || mtmp->mentangled) {
        ; /* fleeing by stairs or traps is not possible */
    } else if (levl[x][y].typ == STAIRS) {
        stway = stairway_at(x,y);
        if (stway && !stway->up && stway->tolev.dnum == u.uz.dnum) {
            if (!(is_floater(mtmp->data) || can_levitate(mtmp)))
                gm.m.has_defense = MUSE_DOWNSTAIRS;
        } else if (stway && stway->up && stway->tolev.dnum == u.uz.dnum) {
            gm.m.has_defense = MUSE_UPSTAIRS;
        } else if (stway &&  stway->tolev.dnum != u.uz.dnum) {
            if (stway->up || !(is_floater(mtmp->data) || can_levitate(mtmp)))
                gm.m.has_defense = MUSE_SSTAIRS;
        }
    } else if (levl[x][y].typ == LADDER) {
        stway = stairway_at(x,y);
        if (stway && stway->up && stway->tolev.dnum == u.uz.dnum) {
            gm.m.has_defense = MUSE_UP_LADDER;
        } else if (stway && !stway->up && stway->tolev.dnum == u.uz.dnum) {
            if (!(is_floater(mtmp->data) || can_levitate(mtmp)))
                gm.m.has_defense = MUSE_DN_LADDER;
        } else if (stway && stway->tolev.dnum != u.uz.dnum) {
            if (stway->up || !(is_floater(mtmp->data) || can_levitate(mtmp)))
                gm.m.has_defense = MUSE_SSTAIRS;
        }
    } else {
        /* Note: trap doors take precedence over teleport traps. */
        coordxy xx, yy, i, locs[10][2];
        boolean ignore_boulders = (verysmall(mtmp->data)
                                   || racial_throws_rocks(mtmp)
                                   || passes_walls(mtmp->data)),
            diag_ok = !NODIAG(monsndx(mtmp->data));

        for (i = 0; i < 10; ++i) /* 10: 9 spots plus sentinel */
            locs[i][0] = locs[i][1] = 0;
        /* collect viable spots; monster's <mx,my> comes first */
        locs[0][0] = x, locs[0][1] = y;
        i = 1;
        for (xx = x - 1; xx <= x + 1; xx++)
            for (yy = y - 1; yy <= y + 1; yy++)
                if (isok(xx, yy) && (xx != x || yy != y)) {
                    locs[i][0] = xx, locs[i][1] = yy;
                    ++i;
                }
        /* look for a suitable trap among the viable spots */
        for (i = 0; i < 10; ++i) {
            xx = locs[i][0], yy = locs[i][1];
            if (!xx)
                break; /* we've run out of spots */
            /* skip if it's hero's location
               or a diagonal spot and monster can't move diagonally
               or some other monster is there */
            if (u_at(xx, yy)
                || (xx != x && yy != y && !diag_ok)
                || (svl.level.monsters[xx][yy] && !(xx == x && yy == y)))
                continue;
            /* skip if there's no trap or can't/won't move onto trap */
            if ((t = t_at(xx, yy)) == 0
                || (!ignore_boulders && sobj_at(BOULDER, xx, yy))
                || onscary(xx, yy, mtmp)
                /* a mounted monster and its steed stay put */
                || has_erid(mtmp) || mtmp->ridden_by)
                continue;
            /* use trap if it's the correct type */
            if (is_hole(t->ttyp)
                && !is_floater(mtmp->data)
                && !can_levitate(mtmp)
                && !mtmp->isshk && !mtmp->isgd && !mtmp->ispriest
                && Can_fall_thru(&u.uz)) {
                gt.trapx = xx;
                gt.trapy = yy;
                gm.m.has_defense = MUSE_TRAPDOOR;
                break; /* no need to look at any other spots */
            } else if (t->ttyp == TELEP_TRAP_SET) {
                gt.trapx = xx;
                gt.trapy = yy;
                gm.m.has_defense = MUSE_TELEPORT_TRAP;
            }
        }
    }

    if (nohands(mtmp->data)) /* can't use objects */
        goto botm;

    if (is_mercenary(mtmp->data) && (obj = m_carrying(mtmp, BUGLE)) != 0
        && m_sees_sleepy_soldier(mtmp)) {
        gm.m.defensive = obj;
        gm.m.has_defense = MUSE_BUGLE;
    }

    /* use immediate physical escape prior to attempting magic */
    if (gm.m.has_defense) /* stairs, trap door or tele-trap, bugle alert */
        goto botm;

    /* kludge to cut down on trap destruction (particularly portals) */
    t = t_at(x, y);
    if (t && (is_pit(t->ttyp) || t->ttyp == WEB
              || t->ttyp == BEAR_TRAP))
        t = 0; /* ok for monster to dig here */

    /* spellcasters that have learned teleport away can cast it to
       escape without needing a wand or scroll; same noteleport
       knowledge check as the wand, same minimum-level rule as
       mchoose_learned_spell() */
    if (!mtmp->mcan && !mtmp->mspec_used && !mtmp->mconf
        && mknows_spell(mtmp, SPE_TELEPORT_AWAY)
        && mtmp->m_lev >= objects[SPE_TELEPORT_AWAY].oc_level * 3
        && (!noteleport_level(mtmp)
            || !mon_knows_traps(mtmp, TELEP_TRAP_SET))) {
        if (mon_has_amulet(mtmp))
            gm.m.has_defense = MUSE_SPE_TELEPORT_AWAY;
        else if (!(mtmp->isshk && inhishop(mtmp)) && !mtmp->isgd
                 && !mtmp->ispriest)
            gm.m.has_defense = MUSE_SPE_TELEPORT_AWAY_SELF;
    }

#define nomore(x)       if (gm.m.has_defense == x) continue;
    /* selection could be improved by collecting all possibilities
       into an array and then picking one at random */
    for (obj = mtmp->minvent; obj; obj = obj->nobj) {
        /* don't always use the same selection pattern */
        if (gm.m.has_defense && !rn2(3))
            break;

        /* nomore(MUSE_WAN_DIGGING); */
        if (gm.m.has_defense == MUSE_WAN_DIGGING)
            break;
        if (obj->otyp == WAN_DIGGING && obj->spe > 0 && !stuck && !t
            && !mtmp->isshk && !mtmp->isgd && !mtmp->ispriest
            && !is_floater(mtmp->data)
            && !can_levitate(mtmp)
            /* monsters digging in Sokoban can ruin things */
            && !Sokoban
            /* digging wouldn't be effective; assume they know that */
            && !(levl[x][y].wall_info & W_NONDIGGABLE)
            && !(Is_botlevel(&u.uz) || In_endgame(&u.uz))
            && !(is_ice(x, y) || is_pool(x, y) || is_lava(x, y))
            && !(is_Vlad(mtmp) && In_V_tower(&u.uz))) {
            gm.m.defensive = obj;
            gm.m.has_defense = MUSE_WAN_DIGGING;
        }
        nomore(MUSE_WAN_TELEPORTATION_SELF);
        nomore(MUSE_WAN_TELEPORTATION);
        if (obj->otyp == WAN_TELEPORTATION && obj->spe > 0) {
            /* use the TELEP_TRAP_SET bit to determine if they know
             * about noteleport on this level or not.  Avoids
             * ineffective re-use of teleportation.  This does
             * mean if the monster leaves the level, they'll know
             * about teleport traps.
             */
            if (!noteleport_level(mtmp)
                || !mon_knows_traps(mtmp, TELEP_TRAP_SET)) {
                gm.m.defensive = obj;
                gm.m.has_defense = (mon_has_amulet(mtmp))
                                    ? MUSE_WAN_TELEPORTATION
                                    : MUSE_WAN_TELEPORTATION_SELF;
            }
        }
        nomore(MUSE_SCR_TELEPORTATION);
        if (obj->otyp == SCR_TELEPORTATION && mtmp->mcansee
            && haseyes(mtmp->data)
            && (!obj->cursed || (!(mtmp->isshk && inhishop(mtmp))
                                 && !mtmp->isgd && !mtmp->ispriest))) {
            /* see WAN_TELEPORTATION case above */
            if (!noteleport_level(mtmp)
                || !mon_knows_traps(mtmp, TELEP_TRAP_SET)) {
                gm.m.defensive = obj;
                gm.m.has_defense = MUSE_SCR_TELEPORTATION;
            }
        }

        if (mtmp->data != &mons[PM_PESTILENCE]) {
            nomore(MUSE_POT_FULL_HEALING);
            if (obj->otyp == POT_FULL_HEALING) {
                gm.m.defensive = obj;
                gm.m.has_defense = MUSE_POT_FULL_HEALING;
            }
            nomore(MUSE_POT_EXTRA_HEALING);
            if (obj->otyp == POT_EXTRA_HEALING) {
                gm.m.defensive = obj;
                gm.m.has_defense = MUSE_POT_EXTRA_HEALING;
            }
            nomore(MUSE_WAN_CREATE_MONSTER);
            if (obj->otyp == WAN_CREATE_MONSTER && obj->spe > 0) {
                gm.m.defensive = obj;
                gm.m.has_defense = MUSE_WAN_CREATE_MONSTER;
            }
            nomore(MUSE_POT_HEALING);
            if (obj->otyp == POT_HEALING) {
                gm.m.defensive = obj;
                gm.m.has_defense = MUSE_POT_HEALING;
            }
            nomore(MUSE_EUCALYPTUS_LEAF);
            if ((mtmp->msick || mtmp->mdiseased)
                && obj->otyp == EUCALYPTUS_LEAF) {
                gm.m.defensive = obj;
                gm.m.has_defense = MUSE_EUCALYPTUS_LEAF;
            }
        } else { /* Pestilence */
            nomore(MUSE_POT_FULL_HEALING);
            if (obj->otyp == POT_SICKNESS) {
                gm.m.defensive = obj;
                gm.m.has_defense = MUSE_POT_FULL_HEALING;
            }
            nomore(MUSE_WAN_CREATE_MONSTER);
            if (obj->otyp == WAN_CREATE_MONSTER && obj->spe > 0) {
                gm.m.defensive = obj;
                gm.m.has_defense = MUSE_WAN_CREATE_MONSTER;
            }
        }
        nomore(MUSE_SCR_CREATE_MONSTER);
        if (obj->otyp == SCR_CREATE_MONSTER) {
            gm.m.defensive = obj;
            gm.m.has_defense = MUSE_SCR_CREATE_MONSTER;
        }
    }

    /* set up context for the generic finder callback; it rescans
       open inventory but its real work is carried containers */
    {
        struct find_context ctx;

        ctx.stuck = stuck;
        ctx.t = t; /* pit/web/bear trap already cleared above */
        ctx.target = (struct monst *) 0;
        ctx.reflection_skip = FALSE;
        ctx.helmet = (struct obj *) 0;

        return find_items_recurse(mtmp, mtmp->minvent, FIND_DEFENSIVE,
                                  check_defensive_item, &ctx);
    }

 botm:
    return (boolean) !!gm.m.has_defense;
#undef nomore
}

/* callback for checking defensive items */
staticfn int
check_defensive_item(
    struct monst *mtmp,
    struct obj *obj,
    int current_muse,
    struct find_context *ctx)
{
    coordxy x = mtmp->mx, y = mtmp->my;

#define nomore(x) if (current_muse == (x)) return 0

    nomore(MUSE_WAN_DIGGING);
    if (obj->otyp == WAN_DIGGING && obj->spe > 0 && !ctx->stuck && !ctx->t
        && !mtmp->isshk && !mtmp->isgd && !mtmp->ispriest
        && !is_floater(mtmp->data)
        && !can_levitate(mtmp)
        /* monsters digging in Sokoban can ruin things */
        && !Sokoban
        /* digging wouldn't be effective; assume they know that */
        && !(levl[x][y].wall_info & W_NONDIGGABLE)
        && !(Is_botlevel(&u.uz) || In_endgame(&u.uz))
        && !(is_ice(x, y) || is_pool(x, y) || is_lava(x, y))
        && !(is_Vlad(mtmp) && In_V_tower(&u.uz)))
        return MUSE_WAN_DIGGING;

    nomore(MUSE_WAN_TELEPORTATION_SELF);
    nomore(MUSE_WAN_TELEPORTATION);
    if (obj->otyp == WAN_TELEPORTATION && obj->spe > 0) {
        /* use the TELEP_TRAP_SET bit to determine if they know
         * about noteleport on this level or not; avoids
         * ineffective re-use of teleportation */
        if (!noteleport_level(mtmp)
            || !mon_knows_traps(mtmp, TELEP_TRAP_SET))
            return (mon_has_amulet(mtmp))
                   ? MUSE_WAN_TELEPORTATION
                   : MUSE_WAN_TELEPORTATION_SELF;
    }

    nomore(MUSE_SCR_TELEPORTATION);
    if (obj->otyp == SCR_TELEPORTATION && mtmp->mcansee
        && haseyes(mtmp->data)
        && (!obj->cursed || (!(mtmp->isshk && inhishop(mtmp))
                             && !mtmp->isgd && !mtmp->ispriest))) {
        /* see WAN_TELEPORTATION case above */
        if (!noteleport_level(mtmp)
            || !mon_knows_traps(mtmp, TELEP_TRAP_SET))
            return MUSE_SCR_TELEPORTATION;
    }

    if (mtmp->data != &mons[PM_PESTILENCE]) {
        nomore(MUSE_POT_FULL_HEALING);
        if (obj->otyp == POT_FULL_HEALING)
            return MUSE_POT_FULL_HEALING;

        nomore(MUSE_POT_EXTRA_HEALING);
        if (obj->otyp == POT_EXTRA_HEALING)
            return MUSE_POT_EXTRA_HEALING;

        nomore(MUSE_WAN_CREATE_MONSTER);
        if (obj->otyp == WAN_CREATE_MONSTER && obj->spe > 0)
            return MUSE_WAN_CREATE_MONSTER;

        nomore(MUSE_POT_HEALING);
        if (obj->otyp == POT_HEALING)
            return MUSE_POT_HEALING;

        nomore(MUSE_EUCALYPTUS_LEAF);
        if ((mtmp->msick || mtmp->mdiseased)
            && obj->otyp == EUCALYPTUS_LEAF)
            return MUSE_EUCALYPTUS_LEAF;
    } else { /* Pestilence */
        nomore(MUSE_POT_FULL_HEALING);
        if (obj->otyp == POT_SICKNESS)
            return MUSE_POT_FULL_HEALING;

        nomore(MUSE_WAN_CREATE_MONSTER);
        if (obj->otyp == WAN_CREATE_MONSTER && obj->spe > 0)
            return MUSE_WAN_CREATE_MONSTER;
    }

    nomore(MUSE_SCR_CREATE_MONSTER);
    if (obj->otyp == SCR_CREATE_MONSTER)
        return MUSE_SCR_CREATE_MONSTER;

    return 0;
#undef nomore
}

/* when a monster deliberately enters a trap, make sure the spot becomes
   accessible (trap doors and teleporters inside niches are located at
   secret corridor locations; convert such into normal corridor even if
   hero doesn't see it happen) */
staticfn void
reveal_trap(struct trap *t, boolean seeit)
{
    struct rm *lev = &levl[t->tx][t->ty];

    if (lev->typ == SCORR) {
        lev->typ = CORR, lev->flags = 0; /* set_levltyp(,,CORR) */
        unblock_point(t->tx, t->ty);
    }
    if (seeit)
        seetrap(t);
}

/* Monsters without the Amulet escape the dungeon and
 * are gone for good when they leave up the up stairs.
 * A monster with the Amulet would leave it behind
 * (mongone -> mdrop_special_objs) but we force any
 * monster who manages to acquire it or the invocation
 * tools to stick around instead of letting it escape.
 * Don't let the Wizard escape even when not carrying
 * anything of interest unless there are more than 1
 * of him.
 */
staticfn int
mon_escape(struct monst *mtmp, boolean vismon)
{
    if (mon_has_special(mtmp)
        || (mtmp->iswiz && svc.context.no_of_wizards < 2))
        return 0;
    if (vismon)
        pline_mon(mtmp, "%s escapes the dungeon!", Monnam(mtmp));
    mongone(mtmp);
    return 2;
}

/* Perform a defensive action for a monster.  Must be called immediately
 * after find_defensive().  Return values are 0: did something, 1: died,
 * 2: did something and can't attack again (i.e. teleported).
 */
int
use_defensive(struct monst *mtmp)
{
    static const char MissingDefensiveItem[] = "use_defensive: no %s";
    int i, fleetim;
    struct obj *otmp = gm.m.defensive;
    boolean vis, vismon, oseen;
    struct trap *t;
    stairway *stway;

    if ((i = precheck(mtmp, otmp)) != 0)
        return i;
    vis = cansee(mtmp->mx, mtmp->my);
    vismon = canseemon(mtmp);
    oseen = otmp && vismon;

    /* when using defensive choice to run away, we want monster to avoid
       rushing right straight back; don't override if already scared */
    fleetim = !mtmp->mflee ? (33 - (30 * mtmp->mhp / mtmp->mhpmax)) : 0;
#define m_flee(m)                          \
    if (fleetim && !m->iswiz) {            \
        monflee(m, fleetim, FALSE, FALSE); \
    }

    switch (gm.m.has_defense) {
    case MUSE_UNICORN_HORN:
        /* unlike most defensive cases, unicorn horn object is optional */
        if (vismon) {
            if (otmp)
                pline_mon(mtmp, "%s uses a unicorn horn!", Monnam(mtmp));
            else
                pline_The("tip of %s's horn glows!", mon_nam(mtmp));
        }
        if (!mtmp->mcansee) {
            mcureblindness(mtmp, vismon);
        } else if (mtmp->msick || mtmp->mdiseased) {
            mtmp->msick = mtmp->mdiseased = 0;
            if (vismon)
                pline_mon(mtmp, "%s is no longer ill.", Monnam(mtmp));
        } else if (mtmp->mconf || mtmp->mstun) {
            mtmp->mconf = mtmp->mstun = 0;
            if (vismon)
                pline_mon(mtmp, "%s seems steadier now.", Monnam(mtmp));
        } else {
            impossible("No need for unicorn horn?");
        }
        return 2;
    case MUSE_BUGLE:
        if (!otmp)
            panic(MissingDefensiveItem, "bugle");
        if (vismon) {
            pline_mon(mtmp, "%s plays %s!", Monnam(mtmp), doname(otmp));
        } else if (!Deaf) {
            Soundeffect(se_bugle_playing_reveille, 100);
            You_hear("a bugle playing reveille!");
        }
        awaken_soldiers(mtmp);
        return 2;
    case MUSE_WAN_TELEPORTATION_SELF:
        if (!otmp)
            panic(MissingDefensiveItem, "wand of teleportation");
        if ((mtmp->isshk && inhishop(mtmp)) || mtmp->isgd || mtmp->ispriest)
            return 2;
        m_flee(mtmp);
        mzapwand(mtmp, otmp, TRUE);
        m_tele(mtmp, vismon, oseen, WAN_TELEPORTATION);
        return 2;
    case MUSE_WAN_TELEPORTATION:
        if (!otmp)
            panic(MissingDefensiveItem, "wand of teleportation");
        gz.zap_oseen = oseen;
        mzapwand(mtmp, otmp, FALSE);
        gm.m_using = TRUE;
        /* mbhit() aims via the gt.tbx/gt.tby globals; nothing on the
           defensive path sets them, so aim at the hero's tracked
           position (skip the beam if that is somehow the caster's
           own spot) */
        gt.tbx = mtmp->mux - mtmp->mx;
        gt.tby = mtmp->muy - mtmp->my;
        gb.buzzer = mtmp; /* for kill attribution (pet livelog credit) */
        if (gt.tbx || gt.tby)
            mbhit(mtmp, rn1(8, 6), mbhitm, bhito, otmp);
        gb.buzzer = 0;
        /* monster learns that teleportation isn't useful here */
        if (noteleport_level(mtmp))
            mon_learns_traps(mtmp, TELEP_TRAP_SET);
        gm.m_using = FALSE;
        return (DEADMONSTER(mtmp)) ? 1 : 2;
    case MUSE_SPE_TELEPORT_AWAY_SELF:
        if ((mtmp->isshk && inhishop(mtmp)) || mtmp->isgd || mtmp->ispriest)
            return 2;
        m_flee(mtmp);
        /* casting-act messages use the wider castmu() visibility test */
        if (canseemon(mtmp) || tp_sensemon(mtmp) || Detect_monsters)
            pline_mon(mtmp, "%s casts a spell!", Monnam(mtmp));
        else if (!Deaf)
            You_hear("something cast a spell!");
        /* castmu()'s casting cost formula floors at 2 for any monster
           of high enough level to cast this spell */
        mtmp->mspec_used = 2;
        /* no item triggered this, so no makeknown() candidate; follow
           the same steps m_tele() applies to the wand */
        if (tele_restrict(mtmp)) {
            /* monster learns that teleportation isn't useful here */
            if (noteleport_level(mtmp))
                mon_learns_traps(mtmp, TELEP_TRAP_SET);
        } else if ((mon_has_amulet(mtmp) || On_W_tower_level(&u.uz))
                   && !rn2(3)) {
            if (vismon)
                pline_mon(mtmp, "%s seems disoriented for a moment.",
                          Monnam(mtmp));
        } else if (Iniceq
                   && mtmp->data == &mons[PM_KATHRYN_THE_ICE_QUEEN]) {
            if (vismon) {
                pline_mon(mtmp,
                          "A powerful curse prevents %s from teleporting!",
                          mon_nam(mtmp));
                verbalize("Nooooo!");
            }
        } else {
            (void) rloc(mtmp, RLOC_MSG);
        }
        return (DEADMONSTER(mtmp)) ? 1 : 2;
    case MUSE_SPE_TELEPORT_AWAY:
        /* carrying the Amulet, so it can't teleport itself; cast at
           the hero to send them away instead, as with the wand */
        pline_mon(mtmp, "%s casts a spell%s!",
                  (canseemon(mtmp) || tp_sensemon(mtmp) || Detect_monsters)
                      ? Monnam(mtmp)
                      : "Something",
                  (Invis && !mon_prop(mtmp, SEE_INVIS)
                   && (mtmp->mux != u.ux || mtmp->muy != u.uy))
                      ? " at a spot near you"
                      : (Displaced && (mtmp->mux != u.ux || mtmp->muy != u.uy))
                            ? " at your displaced image"
                            : " at you");
        mtmp->mspec_used = 2;
        (void) mcast_immediate_spell(mtmp, mtmp->mux, mtmp->muy,
                                     SPE_TELEPORT_AWAY);
        /* monster learns that teleportation isn't useful here */
        if (noteleport_level(mtmp))
            mon_learns_traps(mtmp, TELEP_TRAP_SET);
        return (DEADMONSTER(mtmp)) ? 1 : 2;
    case MUSE_SCR_TELEPORTATION: {
        int obj_is_cursed;

        if (!otmp)
            panic(MissingDefensiveItem, "scroll of teleportation");
        obj_is_cursed = otmp->cursed;
        if (mtmp->isshk || mtmp->isgd || mtmp->ispriest)
            return 2;
        m_flee(mtmp);
        /* we want to be able to access otmp after the teleport but it
           might get destroyed if still in mtmp's inventory (maybe mtmp
           lands in lava or on a fire trap) so take it out in advance */
        if (otmp->quan > 1L)
            otmp = splitobj(otmp, 1L);
        extract_from_minvent(mtmp, otmp, FALSE, FALSE);
        /* 'last_msg' will be changed to PLNMSG_UNKNOWN if any messages
           are issued by mreadmsg(), 'if (vismon) pline()', or m_tele() */
        iflags.last_msg = PLNMSG_enum;
        mreadmsg(mtmp, otmp); /* sets otmp->dknown if !Blind or !Deaf */
        if (obj_is_cursed || mtmp->mconf) {
            int nlev;
            d_level flev;

            nlev = random_teleport_level();
            if (mon_has_amulet(mtmp) || In_endgame(&u.uz)
                || (Is_sanctum(&u.uz)
                    && mtmp->data == &mons[PM_LUCIFER])) {
                if (vismon)
                    pline_mon(mtmp, "%s seems very disoriented for a moment.",
                          Monnam(mtmp));
            } else if (nlev == depth(&u.uz)) {
                if (vismon)
                    pline_mon(mtmp, "%s shudders for a moment.", Monnam(mtmp));
            } else if (Iniceq
                       && mtmp->data == &mons[PM_KATHRYN_THE_ICE_QUEEN]) {
                if (vismon) {
                    pline_mon(mtmp, "A powerful curse prevents %s from"
                                    " leaving this place!",
                              mon_nam(mtmp));
                    verbalize("Nooooo!");
                }
            } else {
                get_level(&flev, nlev);
                migrate_to_level(mtmp, ledger_no(&flev), MIGR_RANDOM,
                                 (coord *) 0);
            }
        } else {
            m_tele(mtmp, vismon, oseen, SCR_TELEPORTATION);
        }
        /* m_tele() handles makeknown(); trycall() will be a no-op when
           otmp->otyp is already discovered */
        if (otmp->dknown && iflags.last_msg != PLNMSG_enum)
            trycall(otmp);
        /* already removed from mtmp->minvent so not 'm_useup(mtmp, otmp)' */
        obfree(otmp, (struct obj *) 0);
        return 2;
    }
    case MUSE_WAN_DIGGING:
        if (!otmp)
            panic(MissingDefensiveItem, "wand of digging");
        m_flee(mtmp);
        mzapwand(mtmp, otmp, FALSE);
        if (oseen)
            makeknown(WAN_DIGGING);
        if (IS_FURNITURE(levl[mtmp->mx][mtmp->my].typ)
            || IS_DRAWBRIDGE(levl[mtmp->mx][mtmp->my].typ)
            || (is_drawbridge_wall(mtmp->mx, mtmp->my) >= 0)
            || stairway_at(mtmp->mx, mtmp->my)) {
            pline_The("digging ray is ineffective.");
            return 2;
        }
        if (!Can_dig_down(&u.uz) && !levl[mtmp->mx][mtmp->my].candig) {
            /* can't dig further if there's already a pit (or other trap)
               here, or if pit creation fails for some reason */
            if (t_at(mtmp->mx, mtmp->my)
                || !(t = maketrap(mtmp->mx, mtmp->my, PIT))) {
                if (vismon) {
                    pline_The("%s here is too hard to dig in.",
                              surface(mtmp->mx, mtmp->my));
                }
                return 2;
            }
            /* pit creation succeeded */
            if (vis) {
                seetrap(t);
                pline_mon(mtmp, "%s has made a pit in the %s.", Monnam(mtmp),
                      surface(mtmp->mx, mtmp->my));
            }
            fill_pit(mtmp->mx, mtmp->my);
            recalc_block_point(mtmp->mx, mtmp->my);
            return (mintrap(mtmp, FORCEBUNGLE) == Trap_Killed_Mon) ? 1 : 2;
        }
        t = maketrap(mtmp->mx, mtmp->my, HOLE);
        if (!t)
            return 2;
        recalc_block_point(mtmp->mx, mtmp->my);
        seetrap(t);
        if (vis) {
            pline_mon(mtmp, "%s has made a hole in the %s.", Monnam(mtmp),
                  surface(mtmp->mx, mtmp->my));
            pline_mon(mtmp, "%s %s through...", Monnam(mtmp),
                  is_flyer(mtmp->data) ? "dives" : "falls");
        } else if (!Deaf) {
            Soundeffect(se_crash_through_floor, 100);
            You_hear("%s crash through the %s.", something,
                     surface(mtmp->mx, mtmp->my));
        }
        fill_pit(mtmp->mx, mtmp->my);
        /* we made sure that there is a level for mtmp to go to */
        migrate_to_level(mtmp, ledger_no(&u.uz) + 1, MIGR_RANDOM,
                         (coord *) 0);
        return 2;
    case MUSE_WAN_UNDEAD_TURNING:
        if (!otmp)
            panic(MissingDefensiveItem, "wand of undead turning");
        gz.zap_oseen = oseen;
        mzapwand(mtmp, otmp, FALSE);
        gm.m_using = TRUE;
        /* as with defensive teleportation, aim mbhit() at the hero's
           tracked position */
        gt.tbx = mtmp->mux - mtmp->mx;
        gt.tby = mtmp->muy - mtmp->my;
        gb.buzzer = mtmp; /* for kill attribution (pet livelog credit) */
        if (gt.tbx || gt.tby)
            mbhit(mtmp, rn1(8, 6), mbhitm, bhito, otmp);
        gb.buzzer = 0;
        gm.m_using = FALSE;
        return (DEADMONSTER(mtmp)) ? 1 : 2;
    case MUSE_WAN_CREATE_MONSTER: {
        coord cc;
        struct monst *mon;
        /* pm: 0 => random, eel => aquatic, croc => amphibious */
        struct permonst *pm = !is_pool(mtmp->mx, mtmp->my) ? 0
                            : &mons[u.uinwater ? PM_GIANT_EEL : PM_CROCODILE];

        if (!otmp)
            panic(MissingDefensiveItem, "wand of create monster");
        if (!enexto(&cc, mtmp->mx, mtmp->my, pm))
            return 0;
        mzapwand(mtmp, otmp, FALSE);
        mon = makemon((struct permonst *) 0, cc.x, cc.y, NO_MM_FLAGS);
        if (mon && canspotmon(mon) && oseen)
            makeknown(WAN_CREATE_MONSTER);
        return 2;
    }
    case MUSE_SCR_CREATE_MONSTER: {
        coord cc;
        struct permonst *pm = 0, *fish = 0;
        int cnt = 1;
        struct monst *mon;
        boolean known = FALSE;

        if (!otmp)
            panic(MissingDefensiveItem, "scroll of create monster");
        if (!rn2(73))
            cnt += rnd(4);
        if (mtmp->mconf || otmp->cursed)
            cnt += 12;
        if (mtmp->mconf)
            pm = fish = &mons[PM_ACID_BLOB];
        else if (is_pool(mtmp->mx, mtmp->my))
            fish = &mons[u.uinwater ? PM_GIANT_EEL : PM_CROCODILE];
        mreadmsg(mtmp, otmp);
        while (cnt--) {
            /* `fish' potentially gives bias towards water locations;
               `pm' is what to actually create (0 => random) */
            if (!enexto(&cc, mtmp->mx, mtmp->my, fish))
                break;
            mon = makemon(pm, cc.x, cc.y, NO_MM_FLAGS);
            if (mon && canspotmon(mon))
                known = TRUE;
        }
        /* The only case where we don't use oseen.  For wands, you
         * have to be able to see the monster zap the wand to know
         * what type it is.  For teleport scrolls, you have to see
         * the monster to know it teleported.
         */
        if (known)
            makeknown(SCR_CREATE_MONSTER);
        else
            trycall(otmp);
        m_useup(mtmp, otmp);
        return 2;
    }
    case MUSE_TRAPDOOR:
        /* trap doors on "bottom" levels of dungeons are rock-drop
         * trap doors, not holes in the floor.  We check here for
         * safety.
         */
        if (Is_botlevel(&u.uz))
            return 0;
        m_flee(mtmp);
        t = t_at(gt.trapx, gt.trapy);
        if (vis) {
            pline_mon(mtmp, "%s %s into a %s!", Monnam(mtmp),
                  vtense(fakename[0], locomotion(mtmp->data, "jump")),
                  trapname(t->ttyp, FALSE));
        }
        /* if trap was in a concealed niche, it's no longer concealed */
        reveal_trap(t, vis);

        /*  don't use rloc_to() because worm tails must "move" */
        remove_monster(mtmp->mx, mtmp->my);
        newsym(mtmp->mx, mtmp->my); /* update old location */
        place_monster(mtmp, gt.trapx, gt.trapy);
        if (mtmp->wormno)
            worm_move(mtmp);
        newsym(gt.trapx, gt.trapy);

        migrate_to_level(mtmp, ledger_no(&u.uz) + 1, MIGR_RANDOM,
                         (coord *) 0);
        return 2;
    case MUSE_UPSTAIRS:
        m_flee(mtmp);
        stway = stairway_at(mtmp->mx, mtmp->my);
        if (!stway)
            return 0;
        if (ledger_no(&u.uz) == 1)
            /* impossible; level 1 upstairs are SSTAIRS */
            return mon_escape(mtmp, vismon);
        if (Inhell && mon_has_amulet(mtmp) && !rn2(4)
            && (dunlev(&u.uz) < dunlevs_in_dungeon(&u.uz) - 3)) {
            if (vismon)
                pline("As %s climbs the stairs, a mysterious force"
                      " momentarily surrounds %s...",
                      mon_nam(mtmp), mhim(mtmp));
            /* simpler than for the player; this will usually be
               the Wizard and he'll immediately go right to the
               upstairs, so there's not much point in having any
               chance for a random position on the current level */
            migrate_to_level(mtmp, ledger_no(&u.uz) + 1, MIGR_RANDOM,
                             (coord *) 0);
        } else {
            if (vismon)
                pline_mon(mtmp, "%s escapes upstairs!", Monnam(mtmp));
            migrate_to_level(mtmp, ledger_no(&(stway->tolev)),
                             MIGR_STAIRS_DOWN, (coord *) 0);
        }
        return 2;
    case MUSE_DOWNSTAIRS:
        m_flee(mtmp);
        stway = stairway_at(mtmp->mx, mtmp->my);
        if (!stway)
            return 0;
        if (vismon)
            pline_mon(mtmp, "%s escapes downstairs!", Monnam(mtmp));
        migrate_to_level(mtmp, ledger_no(&(stway->tolev)), MIGR_STAIRS_UP,
                         (coord *) 0);
        return 2;
    case MUSE_UP_LADDER:
        m_flee(mtmp);
        stway = stairway_at(mtmp->mx, mtmp->my);
        if (!stway)
            return 0;
        if (vismon)
            pline_mon(mtmp, "%s escapes up the ladder!", Monnam(mtmp));
        migrate_to_level(mtmp, ledger_no(&(stway->tolev)), MIGR_LADDER_DOWN,
                         (coord *) 0);
        return 2;
    case MUSE_DN_LADDER:
        m_flee(mtmp);
        stway = stairway_at(mtmp->mx, mtmp->my);
        if (!stway)
            return 0;
        if (vismon)
            pline_mon(mtmp, "%s escapes down the ladder!", Monnam(mtmp));
        migrate_to_level(mtmp, ledger_no(&(stway->tolev)), MIGR_LADDER_UP,
                         (coord *) 0);
        return 2;
    case MUSE_SSTAIRS:
        m_flee(mtmp);
        stway = stairway_at(mtmp->mx, mtmp->my);
        if (!stway)
            return 0;
        if (ledger_no(&u.uz) == 1) {
            return mon_escape(mtmp, vismon);
        }
        if (vismon)
            pline_mon(mtmp, "%s escapes %sstairs!", Monnam(mtmp),
                  stway->up ? "up" : "down");
        /* going from the Valley to Castle (Stronghold) has no sstairs
           to target, but having gs.sstairs.<sx,sy> == <0,0> will work the
           same as specifying MIGR_RANDOM when mon_arrive() eventually
           places the monster, so we can use MIGR_SSTAIRS unconditionally */
        migrate_to_level(mtmp, ledger_no(&(stway->tolev)), MIGR_SSTAIRS,
                         (coord *) 0);
        return 2;
    case MUSE_TELEPORT_TRAP:
        m_flee(mtmp);
        t = t_at(gt.trapx, gt.trapy);
        if (vis) {
            pline_mon(mtmp, "%s %s onto a %s!", Monnam(mtmp),
                  vtense(fakename[0], locomotion(mtmp->data, "jump")),
                  trapname(t->ttyp, FALSE));
        }
        /* if trap was in a concealed niche, it's no longer concealed */
        reveal_trap(t, vis);
        /*  don't use rloc_to() because worm tails must "move" */
        remove_monster(mtmp->mx, mtmp->my);
        newsym(mtmp->mx, mtmp->my); /* update old location */
        place_monster(mtmp, gt.trapx, gt.trapy);
        if (mtmp->wormno)
            worm_move(mtmp);
        maybe_unhide_at(mtmp->mx, mtmp->my);
        newsym(gt.trapx, gt.trapy);
        /* 0: 'no object' rather than STRANGE_OBJECT; FALSE: obj not seen */
        m_tele(mtmp, vismon, FALSE, 0);
        return 2;
    case MUSE_POT_HEALING:
        if (!otmp)
            panic(MissingDefensiveItem, "potioh of healing");
        mquaffmsg(mtmp, otmp);
        i = d(6 + 2 * bcsign(otmp), 4);
        healmon(mtmp, i, 1);
        if (!otmp->cursed && !mtmp->mcansee) {
            mcureblindness(mtmp, vismon);
        } else if (otmp->blessed && (mtmp->msick || mtmp->mdiseased)) {
            if (vismon)
                pline_mon(mtmp, "%s is no longer ill.", Monnam(mtmp));
            mtmp->msick = mtmp->mdiseased = 0;
        }
        if (vismon)
            pline_mon(mtmp, "%s looks better.", Monnam(mtmp));
        if (oseen)
            makeknown(POT_HEALING);
        m_useup(mtmp, otmp);
        return 2;
    case MUSE_POT_EXTRA_HEALING:
        if (!otmp)
            panic(MissingDefensiveItem, "potioh of extra healing");
        mquaffmsg(mtmp, otmp);
        i = d(6 + 2 * bcsign(otmp), 8);
        healmon(mtmp, i, otmp->blessed ? 5 : 2);
        if (!mtmp->mcansee) {
            mcureblindness(mtmp, vismon);
        } else if (!otmp->cursed && (mtmp->msick || mtmp->mdiseased)) {
            if (vismon)
                pline_mon(mtmp, "%s is no longer ill.", Monnam(mtmp));
            mtmp->msick = mtmp->mdiseased = 0;
        }
        if (vismon)
            pline_mon(mtmp, "%s looks much better.", Monnam(mtmp));
        if (oseen)
            makeknown(POT_EXTRA_HEALING);
        m_useup(mtmp, otmp);
        return 2;
    case MUSE_POT_FULL_HEALING:
        if (!otmp)
            panic(MissingDefensiveItem, "potioh of full healing");
        mquaffmsg(mtmp, otmp);
        if (otmp->otyp == POT_SICKNESS)
            unbless(otmp); /* Pestilence */
        healmon(mtmp, mtmp->mhpmax, otmp->blessed ? 8 : 4);
        if (!mtmp->mcansee && otmp->otyp != POT_SICKNESS) {
            mcureblindness(mtmp, vismon);
        } else if (mtmp->msick || mtmp->mdiseased) {
            if (vismon)
                pline_mon(mtmp, "%s is no longer ill.", Monnam(mtmp));
            mtmp->msick = mtmp->mdiseased = 0;
        }
        if (vismon)
            pline_mon(mtmp, "%s looks completely healed.", Monnam(mtmp));
        if (oseen)
            makeknown(otmp->otyp);
        m_useup(mtmp, otmp);
        return 2;
    case MUSE_LIZARD_CORPSE:
        if (!otmp)
            panic(MissingDefensiveItem, "lizard corpse");
        /* not actually called for its unstoning effect */
        mon_consume_unstone(mtmp, otmp, FALSE, FALSE);
        return 2;
    case MUSE_EUCALYPTUS_LEAF:
        if (!otmp)
            panic(MissingDefensiveItem, "eucalyptus leaf");
        /* not actually called for its unstoning effect */
        mon_consume_unstone(mtmp, otmp, FALSE, FALSE);
        return 2;
    case 0:
        return 0; /* i.e. an exploded wand */
    default:
        impossible("%s wanted to perform action %d?", Monnam(mtmp),
                   gm.m.has_defense);
        break;
    }
    return 0;
#undef m_flee
}

/* Check if monster with Amulet should escape via stairs/portal.
   Called after monster movement to handle immediate escape.
   Returns 2 if monster escaped (migrated), 0 otherwise */
int
mon_escape_with_amulet(struct monst *mtmp)
{
    coordxy x = mtmp->mx, y = mtmp->my;
    boolean vismon = canseemon(mtmp);
    stairway *stway;
    struct trap *ttrap;

    /* Only intelligent monsters understand the Amulet's significance */
    if (!mon_has_amulet(mtmp) || mtmp->mpeaceful
        || mindless(mtmp->data) || is_animal(mtmp->data))
        return 0;

    /* Check for stairs or a ladder at monster's location */
    if ((stway = stairway_at(x, y)) != 0) {
        if (stway->tolev.dnum != u.uz.dnum) {
            /* On a dungeon branch (master's special stairs)? */
            if (!u.uz.dnum && u.uz.dlevel == 1 && stway->up) {
                /* Escaping to the Planes! */
                if (vismon)
                    pline("%s escapes to the Planes with the Amulet!",
                          Monnam(mtmp));
                if (Role_if(PM_INFIDEL)) {
                    /* Infidels need the Amulet to imbue the Idol at
                       Moloch's altar in the Sanctum - losing it to
                       the Planes is catastrophic and unrecoverable */
                    livelog_printf(LL_ARTIFACT,
                        "failed their quest! %s escaped to the Planes "
                        "with the Amulet of Yendor",
                        upstart(x_monnam(mtmp, ARTICLE_THE,
                                         (char *) 0, SUPPRESS_IT,
                                         TRUE)));
                    pline("The ground shakes violently!");
                    You_hear("a voice boom from the depths below:");
                    verbalize("FOOL!  You have let the Amulet slip "
                              "through your fingers!");
                    verbalize("It was to be MINE, and now it is lost "
                              "to me forever!");
                    pline("Moloch's fury consumes you utterly.");
                    svk.killer.format = KILLED_BY;
                    Strcpy(svk.killer.name, "Moloch's wrath");
                    done(DIED);
                    /* won't reach here */
                }
                u.uamulet_on_planes = 1;
            } else {
                if (vismon)
                    pline("%s escapes %sstairs with the Amulet!",
                          Monnam(mtmp), stway->up ? "up" : "down");
            }
            migrate_to_level(mtmp, ledger_no(&stway->tolev), MIGR_SSTAIRS,
                             (coord *) 0);
            newsym(x, y);
            return 2;
        }
        if (stway->up) {
            /* On same-dungeon stairs or ladder going up? */
            if (vismon)
                pline("%s escapes up%s with the Amulet!", Monnam(mtmp),
                      stway->isladder ? " the ladder" : "stairs");
            migrate_to_level(mtmp, ledger_no(&stway->tolev),
                             stway->isladder ? MIGR_LADDER_DOWN
                                             : MIGR_STAIRS_DOWN,
                             (coord *) 0);
            newsym(x, y);
            return 2;
        }
    }

    /* Check for magic portal (elemental planes) */
    ttrap = t_at(x, y);
    if (ttrap && ttrap->ttyp == MAGIC_PORTAL) {
        if (vismon)
            pline("%s enters the portal with the Amulet!", Monnam(mtmp));
        migrate_to_level(mtmp, ledger_no(&ttrap->dst), MIGR_PORTAL,
                         (coord *) 0);
        newsym(x, y);
        return 2;
    }

    return 0;
}

int
rnd_defensive_item(struct monst *mtmp)
{
    struct permonst *pm = mtmp->data;
    int difficulty = mons[(monsndx(pm))].difficulty;
    int trycnt = 0;

    if (is_animal(pm) || attacktype(pm, AT_EXPL) || mindless(mtmp->data)
        || pm->mlet == S_GHOST || pm->mlet == S_KOP)
        return 0;
 try_again:
    switch (rn2(8 + (difficulty > 3) + (difficulty > 6) + (difficulty > 8))) {
    case 6:
    case 9:
        if (noteleport_level(mtmp) && ++trycnt < 2)
            goto try_again;
        if (!rn2(3))
            return WAN_TELEPORTATION;
        FALLTHROUGH;
        /*FALLTHRU*/
    case 0:
    case 1:
        return SCR_TELEPORTATION;
    case 8:
    case 10:
        if (!rn2(3))
            return WAN_CREATE_MONSTER;
        FALLTHROUGH;
        /*FALLTHRU*/
    case 2:
        return SCR_CREATE_MONSTER;
    case 3:
        return POT_HEALING;
    case 4:
        return POT_EXTRA_HEALING;
    case 5:
        return (mtmp->data != &mons[PM_PESTILENCE]) ? POT_FULL_HEALING
                                                    : POT_SICKNESS;
    case 7: /* wand of digging */
        /* usually avoid digging in Sokoban */
        if (Sokoban && rn2(4))
            goto try_again;
        /* some creatures shouldn't dig down to another level when hurt */
        if (is_floater(pm) || mtmp->isshk || mtmp->isgd || mtmp->ispriest)
            return 0;
        return WAN_DIGGING;
    }
    /*NOTREACHED*/
    return 0;
}

#define MUSE_WAN_DEATH 1
#define MUSE_WAN_SLEEP 2
#define MUSE_WAN_FIRE 3
#define MUSE_WAN_COLD 4
#define MUSE_WAN_LIGHTNING 5
#define MUSE_WAN_MAGIC_MISSILE 6
#define MUSE_WAN_STRIKING 7
#define MUSE_SCR_FIRE 8
#define MUSE_POT_PARALYSIS 9
#define MUSE_POT_BLINDNESS 10
#define MUSE_POT_CONFUSION 11
#define MUSE_FROST_HORN 12
#define MUSE_FIRE_HORN 13
#define MUSE_POT_ACID 14
#define MUSE_WAN_TELEPORTATION 15
#define MUSE_POT_SLEEPING 16
#define MUSE_SCR_EARTH 17
#define MUSE_CAMERA 18
/*#define MUSE_WAN_UNDEAD_TURNING 20*/ /* also a defensive item so don't
                                     * redefine; nonconsecutive value is ok */
#define MUSE_WAN_SLOW_MONSTER 21 /* skips the shared undead-turning value */

staticfn boolean
linedup_chk_corpse(coordxy x, coordxy y)
{
    return (sobj_at(CORPSE, x, y) != 0);
}

/* from monster's point of view, is hero behind a chokepoint? */
staticfn boolean
hero_behind_chokepoint(struct monst *mtmp)
{
    coordxy dx = sgn(mtmp->mx - mtmp->mux);
    coordxy dy = sgn(mtmp->my - mtmp->muy);

    coordxy x = mtmp->mux + dx;
    coordxy y = mtmp->muy + dy;

    int dir = xytodir(dx, dy);
    int dir_l = DIR_CLAMP(DIR_LEFT2(dir));
    int dir_r = DIR_CLAMP(DIR_RIGHT2(dir));

    coord c1, c2;

    dirtocoord(&c1, dir_l);
    dirtocoord(&c2, dir_r);
    c1.x += x, c2.x += x;
    c1.y += y, c2.y += y;

    if ((!isok(c1.x, c1.y) || !accessible(c1.x, c1.y))
        && (!isok(c2.x, c2.y) || !accessible(c2.x, c2.y)))
        return TRUE;
    return FALSE;
}

/* hostile monster has another hostile next to it */
staticfn boolean
mon_has_friends(struct monst *mtmp)
{
    coordxy dx, dy;
    struct monst *mon2;

    if (mtmp->mtame || mtmp->mpeaceful)
        return FALSE;

    for (dx = -1; dx <= 1; dx++)
        for (dy = -1; dy <= 1; dy++) {
            coordxy x = mtmp->mx + dx;
            coordxy y = mtmp->my + dy;

            if (isok(x, y) && (mon2 = m_at(x, y)) != 0
                && mon2 != mtmp
                && !mon2->mtame && !mon2->mpeaceful)
                return TRUE;
        }

    return FALSE;
}

/* does monster like object pile at x,y? */
staticfn boolean
mon_likes_objpile_at(struct monst *mtmp, coordxy x, coordxy y)
{
    int i;
    struct obj *otmp;

    if (!isok(x,y) || !OBJ_AT(x,y))
        return FALSE;

    /* monster likes any of the top 3 items in the pile? */
    for (i = 0, otmp = svl.level.objects[x][y]; otmp && i < 3; i++) {
        if (mon_would_take_item(mtmp, otmp))
            return TRUE;
        otmp = otmp->nexthere;
    }

    /* pile is larger than 3 stacks? */
    if (i >= 3)
        return TRUE;

    return FALSE;
}

/* Select an offensive item/action for a monster.  Returns TRUE iff one is
 * found.
 */
boolean
find_offensive(struct monst *mtmp)
{
    struct monst *target = mfind_target(mtmp);
    boolean reflection_skip = FALSE;

    /* pet ordered to avoid all monsters - skip all offensive items,
       but Conflict overrides this order */
    if (mtmp->mtame && has_edog(mtmp)
        && (EDOG(mtmp)->petstrat & PETSTRAT_NOATTACK) && !Conflict)
        return FALSE;

    if (!target)
        return FALSE; /* nothing to attack */
    if (target == &gy.youmonst) {
        reflection_skip = (m_seenres(mtmp, M_SEEN_REFL) != 0
                           || (monnear(mtmp, mtmp->mux, mtmp->muy)
                               && !rn2(3)));
    } else if (mtmp->mtame && !acceptable_pet_target(mtmp, target, TRUE)) {
        /* tame pets should respect acceptable_pet_target() */
        return FALSE;
    }

    gm.m.offensive = (struct obj *) 0;
    gm.m.has_offense = 0;
    if ((mtmp->mpeaceful && !mtmp->mtame) || is_animal(mtmp->data)
        || mindless(mtmp->data) || nohands(mtmp->data))
        return FALSE;
    if (u.uswallow)
        return FALSE;
    if (in_your_sanctuary(mtmp, 0, 0))
        return FALSE;
    if (dmgtype(mtmp->data, AD_HEAL)
        && !uwep && !uarmu && !uarm && !uarmh
        && !uarms && !uarmg && !uarmc && !uarmf)
        return FALSE;
    /* all offensive items require orthogonal or diagonal targeting */
    if (target == &gy.youmonst) {
        if (!lined_up(mtmp))
            return FALSE;
    } else {
        /* for monster targets, use m_lined_up which checks alignment
           with the target and avoids friendly fire */
        if (!m_lined_up(target, mtmp, FALSE))
            return FALSE;
    }

    /* set up context for the generic finder callback */
    {
        struct find_context ctx;

        ctx.target = target;
        ctx.reflection_skip = reflection_skip;
        ctx.helmet = which_armor(mtmp, W_ARMH);
        ctx.t = (struct trap *) 0;
        ctx.stuck = FALSE;

        return find_items_recurse(mtmp, mtmp->minvent, FIND_OFFENSIVE,
                                  check_offensive_item, &ctx);
    }
}

/* callback for checking offensive items */
staticfn int
check_offensive_item(
    struct monst *mtmp,
    struct obj *obj,
    int current_muse,
    struct find_context *ctx)
{
#define nomore(x) if (current_muse == (x)) return 0

    if (!ctx->reflection_skip) {
        nomore(MUSE_WAN_DEATH);
        if (obj->otyp == WAN_DEATH && obj->spe > 0
            && !m_seenres(mtmp, M_SEEN_MAGR))
            return MUSE_WAN_DEATH;

        nomore(MUSE_WAN_SLEEP);
        if (obj->otyp == WAN_SLEEP && obj->spe > 0 && gm.multi >= 0
            && !m_seenres(mtmp, M_SEEN_SLEEP))
            return MUSE_WAN_SLEEP;

        nomore(MUSE_WAN_FIRE);
        if (obj->otyp == WAN_FIRE && obj->spe > 0
            && !m_seenres(mtmp, M_SEEN_FIRE))
            return MUSE_WAN_FIRE;

        nomore(MUSE_FIRE_HORN);
        if (obj->otyp == FIRE_HORN && obj->spe > 0 && can_blow(mtmp)
            && !m_seenres(mtmp, M_SEEN_FIRE))
            return MUSE_FIRE_HORN;

        nomore(MUSE_WAN_COLD);
        if (obj->otyp == WAN_COLD && obj->spe > 0
            && !m_seenres(mtmp, M_SEEN_COLD))
            return MUSE_WAN_COLD;

        nomore(MUSE_FROST_HORN);
        if (obj->otyp == FROST_HORN && obj->spe > 0 && can_blow(mtmp)
            && !m_seenres(mtmp, M_SEEN_COLD))
            return MUSE_FROST_HORN;

        nomore(MUSE_WAN_LIGHTNING);
        if (obj->otyp == WAN_LIGHTNING && obj->spe > 0
            && !m_seenres(mtmp, M_SEEN_ELEC))
            return MUSE_WAN_LIGHTNING;

        nomore(MUSE_WAN_MAGIC_MISSILE);
        if (obj->otyp == WAN_MAGIC_MISSILE && obj->spe > 0
            && !m_seenres(mtmp, M_SEEN_MAGR))
            return MUSE_WAN_MAGIC_MISSILE;
    }

    nomore(MUSE_WAN_UNDEAD_TURNING);
    if (obj->otyp == WAN_UNDEAD_TURNING && obj->spe > 0) {
        coordxy ax = u.ux + sgn(mtmp->mux - mtmp->mx) * 3,
                ay = u.uy + sgn(mtmp->muy - mtmp->my) * 3;
        coordxy bx = mtmp->mx, by = mtmp->my;

        /* not necrophiliac(); unlike deciding whether to pick this
           type of wand up, we aren't interested in corpses within
           carried containers until they're moved into open inventory;
           we don't check whether hero is poly'd into an undead--the
           wand's turning effect is too weak to be a useful direct
           attack--only whether hero is carrying at least one corpse */
        if (carrying(CORPSE)
            /*
             * If hero is carrying one or more corpses but isn't wielding
             * a cockatrice corpse (unless being hit by one won't do
             * the monster much harm); otherwise we'd be using this wand
             * as a defensive item with higher priority.
             *
             * Might be cockatrice intended as a weapon (or being denied
             * to glove-wearing monsters for use as a weapon) or lizard
             * intended as a cure or lichen intended as veggy food or
             * sacrifice fodder being lugged to an altar. Zapping with
             * this will deprive hero of one from each stack although
             * they might subsequently be recovered after killing again.
             * In the sacrifice fodder case, it could even be to the
             * player's advantage (fresher corpse if a new one gets
             * dropped; player might not choose to spend a wand charge
             * on that when/if hero acquires this wand).
             */
            || linedup_callback(ax, ay, bx, by, linedup_chk_corpse)
            /* or there's a corpse on the ground in a direct line from
               the monster to the hero, and up to 3 steps beyond */
            /* or the hero is one of the walking dead; the turning blast
               is a direct attack against an undead-race hero */
            || Race_if(PM_DRAUGR) || Race_if(PM_VAMPIRE)
            || is_undead(gy.youmonst.data))
            return MUSE_WAN_UNDEAD_TURNING;
    }

    nomore(MUSE_WAN_STRIKING);
    if (obj->otyp == WAN_STRIKING && obj->spe > 0
        && !m_seenres(mtmp, M_SEEN_MAGR))
        return MUSE_WAN_STRIKING;

    nomore(MUSE_WAN_TELEPORTATION);
    if (obj->otyp == WAN_TELEPORTATION && obj->spe > 0
        /* don't give controlled hero a free teleport */
        && !Teleport_control
        /* same hack as MUSE_WAN_TELEPORTATION_SELF */
        && (!noteleport_level(mtmp)
            || !mon_knows_traps(mtmp, TELEP_TRAP_SET))
        /* do try to move hero to a more vulnerable spot */
        && (onscary(u.ux, u.uy, mtmp)
            || (hero_behind_chokepoint(mtmp) && mon_has_friends(mtmp))
            || mon_likes_objpile_at(mtmp, u.ux, u.uy)
            || stairway_at(u.ux, u.uy)))
        return MUSE_WAN_TELEPORTATION;

    nomore(MUSE_WAN_SLOW_MONSTER);
    if (obj->otyp == WAN_SLOW_MONSTER && obj->spe > 0 && !Slow)
        return MUSE_WAN_SLOW_MONSTER;

    nomore(MUSE_POT_PARALYSIS);
    if (obj->otyp == POT_PARALYSIS && gm.multi >= 0)
        return MUSE_POT_PARALYSIS;

    nomore(MUSE_POT_BLINDNESS);
    if (obj->otyp == POT_BLINDNESS && !attacktype(mtmp->data, AT_GAZE))
        return MUSE_POT_BLINDNESS;

    nomore(MUSE_POT_CONFUSION);
    if (obj->otyp == POT_CONFUSION)
        return MUSE_POT_CONFUSION;

    nomore(MUSE_POT_SLEEPING);
    if (obj->otyp == POT_SLEEPING
        && !m_seenres(mtmp, M_SEEN_SLEEP))
        return MUSE_POT_SLEEPING;

    nomore(MUSE_POT_ACID);
    if (obj->otyp == POT_ACID
        && !m_seenres(mtmp, M_SEEN_ACID))
        return MUSE_POT_ACID;

    /* we can safely put this scroll here since the locations that
     * are in a 1 square radius are a subset of the locations that
     * are in wand or throwing range (in other words, always lined_up())
     */
    nomore(MUSE_SCR_EARTH);
    if (obj->otyp == SCR_EARTH
        && (hard_helmet(ctx->helmet) || mtmp->mconf
            || amorphous(mtmp->data) || passes_walls(mtmp->data)
            || noncorporeal(mtmp->data) || unsolid(mtmp->data)
            || !rn2(10))
        && dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) <= 2
        && mtmp->mcansee && haseyes(mtmp->data)
        && !Is_rogue_level(&u.uz)
        && (!In_endgame(&u.uz) || Is_earthlevel(&u.uz)))
        return MUSE_SCR_EARTH;

    nomore(MUSE_CAMERA);
    if (obj->otyp == EXPENSIVE_CAMERA
        && ((!Blind && !resists_blnd(&gy.youmonst))
            || hates_light(gy.youmonst.data)
            || maybe_polyd(is_drow(gy.youmonst.data),
                           Race_if(PM_DROW)))
        && dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) <= 2
        && obj->spe > 0 && !rn2(6))
        return MUSE_CAMERA;

#if 0
    nomore(MUSE_SCR_FIRE);
    if (obj->otyp == SCR_FIRE && resists_fire(mtmp)
        && dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) <= 2
        && mtmp->mcansee && haseyes(mtmp->data)
        && !m_seenres(mtmp, M_SEEN_FIRE)) {
        /*
         * TODO?
         *  Could choose scroll if <mux,muy> (where attacker thinks
         *  hero is located) is ice and attacker either isn't also
         *  on ice or is able to fly/float/swim.
         */
        return MUSE_SCR_FIRE;
    }
#endif /* 0 */

    return 0;
#undef nomore
}

staticfn int
mbhitm(struct monst *mtmp, struct obj *otmp)
{
    int tmp, otyp = otmp->otyp;
    boolean reveal_invis = FALSE, learnit = FALSE,
            hits_you = (mtmp == &gy.youmonst);
    /* the zapper; a knockback's landing can fire a trap whose beam
       destroys otmp, so take what the tail needs from it up front */
    struct monst *zapper = mcarried(otmp) ? otmp->ocarry
                                          : (struct monst *) 0;

    if (!hits_you && otmp->otyp != WAN_UNDEAD_TURNING) {
        mtmp->msleeping = 0;
        if (mtmp->m_ap_type)
            seemimic(mtmp);
    }
    switch (otmp->otyp) {
    case WAN_STRIKING:
    case SPE_FORCE_BOLT:
        reveal_invis = TRUE;
        if (gl.last_hurtled == mtmp) {
            ; /* already hit by this beam and knocked into its path */
        } else if (hits_you) {
            if (Antimagic) {
                monstseesu(M_SEEN_MAGR); /* monsters notice hero resisting */
                shieldeff(u.ux, u.uy);
                Soundeffect(se_boing, 40);
                pline("Boing!");
                learnit = TRUE;
            } else if (rnd(20) < 10 + u.uac &&
                       !(gb.buzzer && !gb.buzzer->mwandexp)) {
                char kbuf[BUFSZ];

                /* build an attributed killer reason */
                (void) death_inflicted_by(kbuf,
                                          (otyp == WAN_STRIKING)
                                              ? "wand of striking"
                                              : "force bolt",
                                          zapper);
                (void) strsubst(kbuf, "inflicted",
                                (otyp == WAN_STRIKING) ? "zapped"
                                                       : "cast");
                monstunseesu(M_SEEN_MAGR); /* mons see hero not resisting */
                if (otyp == WAN_STRIKING)
                    pline_The("wand hits you!");
                else
                    pline_The("force bolt hits you!");
                tmp = d(2, (otyp == WAN_STRIKING) ? 12 : 6);
                /* force bolt damage scales with caster level */
                if (otyp == SPE_FORCE_BOLT && zapper)
                    tmp += zapper->m_lev / 3;
                if (Half_spell_damage)
                    tmp = (tmp + 1) / 2;
                /* a heavy hit knocks the hero back a square, or out of
                   the saddle; an anchoring artifact holds */
                if (tmp > ((otyp == WAN_STRIKING) ? 16 : 12) && zapper
                    && !u.ustuck && !m_is_steadfast(&gy.youmonst)) {
                    int kx = sgn(u.ux - zapper->mx),
                        ky = sgn(u.uy - zapper->my);
                    struct obj *saddle = u.usteed
                                         ? which_armor(u.usteed, W_SADDLE)
                                         : (struct obj *) 0;
                    boolean unseat = (u.usteed
                                      && !(saddle && saddle->cursed));

                    pline_The("force of %s knocks you %s!",
                              (otyp == WAN_STRIKING) ? "the wand"
                                                     : "the spell",
                              unseat ? "out of your saddle" : "back");
                    gl.last_hurtled = &gy.youmonst;
                    if (unseat) {
                        /* u.dx,u.dy pass the preferred direction to
                           dismount_steed (for DISMOUNT_KNOCKED only) */
                        u.dx = kx;
                        u.dy = ky;
                        dismount_steed(DISMOUNT_KNOCKED);
                    } else if (u.usteed) {
                        /* stuck to a cursed saddle: steed and rider go */
                        mhurtle(u.usteed, kx, ky, 1);
                    } else {
                        hurtle(kx, ky, 1, FALSE);
                    }
                    /* the landing can fire a trap whose beam kills the
                       zapper; a live one learns where the hero went */
                    if (!DEADMONSTER(zapper)) {
                        zapper->mux = u.ux;
                        zapper->muy = u.uy;
                    }
                }
                losehp(tmp, kbuf, KILLED_BY_AN);
                learnit = TRUE;
            } else {
                if (otyp == WAN_STRIKING)
                    pline_The("wand misses you.");
                else
                    pline_The("force bolt misses you.");
            }
            stop_occupation();
            nomul(0);
        } else if (resists_magm(mtmp) || defended(mtmp, AD_MAGM)) {
            shieldeff(mtmp->mx, mtmp->my);
            Soundeffect(se_boing, 40);
            pline("Boing!");
            learnit = TRUE;
        } else if (rnd(20) < 10 + find_mac(mtmp)) {
            tmp = d(2, (otyp == WAN_STRIKING) ? 12 : 6);
            if (otyp == SPE_FORCE_BOLT && zapper)
                tmp += zapper->m_lev / 3;
            hit(otyp == WAN_STRIKING ? "wand" : "force bolt",
                mtmp, exclam(tmp));
            (void) resist(mtmp, otmp->oclass, tmp, TELL);
            learnit = TRUE;
            /* a heavy hit knocks the target back a square; it lands on
               the beam's next square, so remember it and don't hit it
               again with this beam; a ridden steed carries its rider */
            if (!DEADMONSTER(mtmp)
                && tmp > ((otyp == WAN_STRIKING) ? 16 : 12) && zapper
                && !unsolid(mtmp->data) && !m_is_steadfast(mtmp)) {
                gl.last_hurtled = mtmp;
                if (tmp < mtmp->mhp) {
                    if (canseemon(mtmp))
                        pline_The("force of %s knocks %s back!",
                                  (otyp == WAN_STRIKING) ? "the wand"
                                                         : "the spell",
                                  mon_nam(mtmp));
                    mhurtle(mtmp, sgn(mtmp->mx - zapper->mx),
                            sgn(mtmp->my - zapper->my), 1);
                }
            }
        } else {
            miss(otyp == WAN_STRIKING ? "wand" : "force bolt", mtmp);
        }
        /* need to see the wand being zapped and also the spot where the
           target is hit; don't have to see the target itself though */
        if (otyp == WAN_STRIKING && learnit && gz.zap_oseen
            && (hits_you || cansee(mtmp->mx, mtmp->my)))
            makeknown(WAN_STRIKING);
        break;
    case WAN_TELEPORTATION:
    case SPE_TELEPORT_AWAY:
        if (hits_you) {
            tele();
            if (otmp->otyp == WAN_TELEPORTATION && gz.zap_oseen)
                makeknown(WAN_TELEPORTATION);
        } else {
            /* for consistency with zap.c, don't identify */
            if (mtmp->ispriest && *in_rooms(mtmp->mx, mtmp->my, TEMPLE)) {
                if (cansee(mtmp->mx, mtmp->my))
                    pline_mon(mtmp, "%s resists the magic!", Monnam(mtmp));
            } else if (!tele_restrict(mtmp))
                (void) rloc(mtmp, RLOC_MSG);
        }
        break;
    case WAN_POLYMORPH:
    case SPE_POLYMORPH:
        if (hits_you) {
            if (Antimagic) {
                shieldeff(u.ux, u.uy);
                You_feel("momentarily different.");
                monstseesu(M_SEEN_MAGR);
                if (otmp->otyp == WAN_POLYMORPH && gz.zap_oseen)
                    makeknown(WAN_POLYMORPH);
            } else if (!Unchanging) {
                if (otmp->otyp == WAN_POLYMORPH && gz.zap_oseen)
                    makeknown(WAN_POLYMORPH);
                polyself(POLY_NOFLAGS);
            }
        } else if (resists_magm(mtmp) || defended(mtmp, AD_MAGM)) {
            /* magic resistance protects from polymorph traps, so make
               it guard against involuntary polymorph attacks too */
            shieldeff(mtmp->mx, mtmp->my);
        } else if (!resist(mtmp, otmp->oclass, 0, NOTELL)) {
            struct obj *obj;

            /* dropped inventory shouldn't be hit by this zap */
            for (obj = mtmp->minvent; obj; obj = obj->nobj)
                bypass_obj(obj);
            /* natural shapechangers aren't affected by system shock
               (unless protection from shapechangers is interfering
               with their metabolism...) */
            if (!is_shapeshifter(mtmp->data) && !rn2(25)) {
                if (canseemon(mtmp)) {
                    pline_mon(mtmp, "%s shudders!", Monnam(mtmp));
                    if (otmp->otyp == WAN_POLYMORPH && gz.zap_oseen)
                        makeknown(WAN_POLYMORPH);
                    pline_mon(mtmp, "%s is killed!", Monnam(mtmp));
                }
                mtmp->mhp = 0;
                if (DEADMONSTER(mtmp)) {
                    if (mcarried(otmp) && otmp->ocarry->mtame)
                        set_pet_killer(otmp->ocarry);
                    mondied(mtmp);
                }
            } else if (newcham(mtmp, (struct permonst *) 0,
                               NC_VIA_WAND_OR_SPELL | NC_SHOW_MSG)) {
                if (!Hallucination && otmp->otyp == WAN_POLYMORPH
                    && gz.zap_oseen)
                    makeknown(WAN_POLYMORPH);
            }
        }
        break;
    case WAN_CANCELLATION:
    case SPE_CANCELLATION:
        (void) cancel_monst(mtmp, otmp, FALSE, TRUE, FALSE);
        break;
    case WAN_UNDEAD_TURNING:
    case SPE_TURN_UNDEAD:
        if (hits_you) {
            unturn_you();
            learnit = (otmp->otyp == WAN_UNDEAD_TURNING) && gz.zap_oseen;
        } else {
            boolean wake = FALSE;

            if (unturn_dead(mtmp)) /* affects mtmp's invent, not mtmp */
                wake = TRUE;
            if (is_undead(mtmp->data) || is_vampshifter(mtmp)) {
                wake = reveal_invis = TRUE;
                /* context.bypasses=True: if resist() happens to be fatal,
                   make_corpse() will set obj->bypass on the new corpse
                   so that mbhito() will skip it instead of reviving it */
                svc.context.bypasses = TRUE; /* for make_corpse() */
                (void) resist(mtmp, otmp->oclass, rnd(8), NOTELL);
            }
            if (wake) {
                if (!DEADMONSTER(mtmp))
                    wakeup(mtmp, FALSE);
                learnit = (otmp->otyp == WAN_UNDEAD_TURNING)
                          && gz.zap_oseen;
            }
        }
        if (learnit)
            makeknown(WAN_UNDEAD_TURNING);
        break;
    case WAN_SLOW_MONSTER:
    case SPE_SLOW_MONSTER:
        if (hits_you) {
            if (!Slow)
                u_slow_down();
        } else {
            /* monster target - slow them */
            if (!resist(mtmp, otmp->oclass, 0, NOTELL)) {
                mon_adjust_speed(mtmp, -1, otmp);
                if (canseemon(mtmp))
                    pline_mon(mtmp, "%s slows down.", Monnam(mtmp));
            }
        }
        if (otmp->otyp == WAN_SLOW_MONSTER && gz.zap_oseen)
            makeknown(WAN_SLOW_MONSTER);
        break;
    case SPE_DRAIN_LIFE: {
        int dmg;

        reveal_invis = TRUE;
        if (hits_you) {
            if (Drain_resistance) {
                shieldeff(u.ux, u.uy);
                You_feel("momentarily drained.");
            } else {
                losexp("life drainage");
            }
        } else {
            dmg = monhp_per_lvl(mtmp);
            if (mcarried(otmp))
                dmg += otmp->ocarry->m_lev / 3;
            if (resists_drli(mtmp) || defended(mtmp, AD_DRLI)) {
                shieldeff(mtmp->mx, mtmp->my);
            } else if (!resist(mtmp, otmp->oclass, dmg, NOTELL)
                       && !DEADMONSTER(mtmp)) {
                (void) damage_mon(mtmp, dmg, AD_DRLI, TRUE);
                mtmp->mhpmax -= dmg;
                if (DEADMONSTER(mtmp) || mtmp->mhpmax <= 0
                    || mtmp->m_lev < 1) {
                    if (mcarried(otmp) && otmp->ocarry->mtame)
                        set_pet_killer(otmp->ocarry);
                    monkilled(mtmp, "", AD_DRLI);
                } else {
                    mtmp->m_lev--;
                    if (canseemon(mtmp))
                        pline_mon(mtmp, "%s suddenly seems weaker!",
                                  Monnam(mtmp));
                }
            }
        }
        break;
    }
    case SPE_DISPEL_EVIL:
        reveal_invis = TRUE;
        if (hits_you) {
            /* player is evil (Infidel) - take damage */
            if (u.ualign.type == A_NONE) {
                int dmg = d(4, 6);

                if (mcarried(otmp))
                    dmg += otmp->ocarry->m_lev / 3;
                You("shudder in agony!");
                losehp(dmg, "dispel evil", KILLED_BY);
            }
        } else if (is_evil(mtmp)) {
            int dmg = d(4, 6);

            if (mcarried(otmp))
                dmg += otmp->ocarry->m_lev / 3;
            if (!resist(mtmp, otmp->oclass, dmg, NOTELL)
                && !DEADMONSTER(mtmp)) {
                (void) damage_mon(mtmp, dmg, AD_MAGM, TRUE);
                if (canseemon(mtmp))
                    pline_mon(mtmp, "%s %s in %s!", Monnam(mtmp),
                              rn2(2) ? "withers" : "shudders",
                              rn2(2) ? "agony" : "pain");
                if (DEADMONSTER(mtmp)) {
                    if (mcarried(otmp) && otmp->ocarry->mtame)
                        set_pet_killer(otmp->ocarry);
                    monkilled(mtmp, "", AD_MAGM);
                } else {
                    monflee(mtmp, 0, FALSE, TRUE);
                }
            }
        }
        break;
    case SPE_ENTANGLE:
        cast_entangle(mtmp, P_SKILL(spell_skilltype(SPE_ENTANGLE)));
        break;
    default:
        break;
    }
    if (reveal_invis && !DEADMONSTER(mtmp)
        && cansee(gb.bhitpos.x, gb.bhitpos.y) && !canspotmon(mtmp))
        map_invisible(gb.bhitpos.x, gb.bhitpos.y);

    return 0;
}

/* hit all objects at x,y with fhito function */
staticfn boolean
fhito_loc(struct obj *obj,
          coordxy tx, coordxy ty,
          int (*fhito)(OBJ_P, OBJ_P))
{
    struct obj *otmp, *next_obj;
    int hitanything = 0;

    if (!fhito || !OBJ_AT(tx, ty))
        return FALSE;

    for (otmp = svl.level.objects[tx][ty]; otmp; otmp = next_obj) {
        next_obj = otmp->nexthere;

        if (otmp->where != OBJ_FLOOR || otmp->ox != tx || otmp->oy != ty)
            continue;
        hitanything += (*fhito)(otmp, obj);
    }

    return hitanything ? TRUE : FALSE;
}

/* A modified bhit() for monsters.  Based on bhit() in zap.c.  Unlike
 * buzz(), bhit() doesn't take into account the possibility of a monster
 * zapping you, so we need a special function for it.  (Unless someone wants
 * to merge the two functions...)
 */
staticfn void
mbhit(
    struct monst *mon,            /* monster shooting the wand */
    int range,                    /* direction and range */
    int (*fhitm)(MONST_P, OBJ_P), /* must be non-Null */
    int (*fhito)(OBJ_P, OBJ_P),   /* fns called when mon/obj hit */
    struct obj *obj)              /* 2nd arg to fhitm/fhito */
{
    struct monst *mtmp;
    uchar ltyp;
    int ddx, ddy, otyp = obj->otyp;
    /* a knockback's landing can fire a trap whose beam kills the zapper
       or destroys the wand; a spell's pseudo-object has ocarry set but
       is not linked into minvent, so only a real wand is watched */
    unsigned obj_id = obj->o_id;
    boolean obj_in_minvent = (o_on(obj_id, mon->minvent) != 0);

    gl.last_hurtled = (struct monst *) 0;
    gb.bhitpos.x = mon->mx;
    gb.bhitpos.y = mon->my;
    ddx = sgn(gt.tbx);
    ddy = sgn(gt.tby);

    while (range-- > 0) {
        coordxy x, y, dbx, dby;

        gb.bhitpos.x += ddx;
        gb.bhitpos.y += ddy;
        x = gb.bhitpos.x;
        y = gb.bhitpos.y;

        if (!isok(x, y)) {
            gb.bhitpos.x -= ddx;
            gb.bhitpos.y -= ddy;
            break;
        }
        if (u_at(gb.bhitpos.x, gb.bhitpos.y)) {
            (*fhitm)(&gy.youmonst, obj);
            range -= 3;
        } else if ((mtmp = m_at(gb.bhitpos.x, gb.bhitpos.y)) != 0) {
            if (cansee(gb.bhitpos.x, gb.bhitpos.y) && !canspotmon(mtmp))
                map_invisible(gb.bhitpos.x, gb.bhitpos.y);
            (*fhitm)(mtmp, obj);
            range -= 3;
        }
        if (DEADMONSTER(mon)
            || (obj_in_minvent && !o_on(obj_id, mon->minvent)))
            break;
        if (fhito_loc(obj, gb.bhitpos.x, gb.bhitpos.y, fhito))
            range--;
        /* the same check after the objects on the square */
        if (DEADMONSTER(mon)
            || (obj_in_minvent && !o_on(obj_id, mon->minvent)))
            break;
        ltyp = levl[gb.bhitpos.x][gb.bhitpos.y].typ;
        dbx = x, dby = y;
        if ((otyp == WAN_STRIKING || otyp == SPE_FORCE_BOLT)
            /* if levl[x][y].typ is DRAWBRIDGE_UP then the zap is passing
               over the moat in front of a closed drawbridge and doesn't
               hit any part of the bridge's mechanism (yet; it might be
               about to hit the closed portcullis on the next iteration) */
            && ltyp != DRAWBRIDGE_UP && find_drawbridge(&dbx, &dby)) {
            /* this might kill mon and destroy obj but they'll remain
               accessible; (*fhitm)() and (*fhito)() use obj for zap type */
            destroy_drawbridge(dbx, dby);
        } else if (IS_DOOR(ltyp) || ltyp == SDOOR) {
            switch (otyp) {
            /* note: monsters don't use opening or locking magic
               at present, but keep these as placeholders */
            case WAN_OPENING:
            case WAN_LOCKING:
            case WAN_STRIKING:
            case SPE_FORCE_BOLT:
                if (doorlock(obj, gb.bhitpos.x, gb.bhitpos.y)) {
                    if (gz.zap_oseen)
                        makeknown(otyp);
                    /* if a shop or temple door gets broken, add it to
                       the repair list (no cost to player) */
                    if (levl[gb.bhitpos.x][gb.bhitpos.y].doormask == D_BROKEN
                        && (*in_rooms(gb.bhitpos.x, gb.bhitpos.y, SHOPBASE)
                            || temple_at_boundary(gb.bhitpos.x,
                                                  gb.bhitpos.y)))
                        add_damage(gb.bhitpos.x, gb.bhitpos.y, 0L);
                }
                break;
            }
        } else if (ltyp == IRONBARS
                   && !(levl[x][y].wall_info & W_NONDIGGABLE)
                   && (otyp == WAN_STRIKING || otyp == SPE_FORCE_BOLT)) {
            /* add_damage before terrain change so original type is saved */
            if (*in_rooms(x, y, SHOPBASE))
                add_damage(x, y, 0L);
            else if (temple_at_boundary(x, y))
                add_damage(x, y, 0L);
            levl[x][y].typ = ROOM;
            if (cansee(x, y))
                pline_The("iron bars are blown apart!");
            else if (!Deaf)
                You_hear("a lot of loud clanging sounds!");
            wake_nearto(x, y, 20 * 20);
            newsym(x, y);
            /* stop the bolt here; it takes a lot of energy to destroy
               bars */
            range = 0;
        }
        if (!ZAP_POS(ltyp)
            || (IS_DOOR(ltyp) && (levl[gb.bhitpos.x][gb.bhitpos.y].doormask
                                  & (D_LOCKED | D_CLOSED)))) {
            gb.bhitpos.x -= ddx;
            gb.bhitpos.y -= ddy;
            break;
        }
    }
    gl.last_hurtled = (struct monst *) 0;
}

staticfn void
buzz_force_miss(int type, int nd, coordxy sx, coordxy sy, int dx, int dy)
{
    dobuzz(type, nd, sx, sy, dx, dy, TRUE, FALSE, TRUE);
}

/* Monster casts a ray spell (magic missile, fireball, cone of cold,
   sleep, finger of death, lightning, poison blast, acid blast).
   spell_otyp is the spell object type (SPE_MAGIC_MISSILE through
   SPE_ACID_BLAST). Returns TRUE if cast successfully, FALSE otherwise */
boolean
mcast_ray_spell(struct monst *caster, coordxy tx, coordxy ty,
                int spell_otyp)
{
    int dx, dy;
    int nd; /* number of damage/effect dice */

    /* validate spell is in the ray spell range */
    if (spell_otyp < SPE_MAGIC_MISSILE || spell_otyp > SPE_ACID_BLAST) {
        impossible("mcast_ray_spell: invalid spell %d", spell_otyp);
        return FALSE;
    }

    /* calculate direction from caster to target */
    dx = tx - caster->mx;
    dy = ty - caster->my;

    /* sanity check - can't cast at own location */
    if (!dx && !dy)
        return FALSE;

    /* damage/effect dice scale with caster level like a player spell;
       the player uses (u.ulevel / 2) + 1 */
    nd = (caster->m_lev / 2) + 1;
    if (nd < 1)
        nd = 1;

    /* cast the ray; BZ_M_SPELL marks it as monster-cast */
    gb.buzzer = caster; /* for death attribution -> "cast by <mon>" */
    buzz(BZ_M_SPELL(BZ_OFS_SPE(spell_otyp)), nd,
         caster->mx, caster->my, sgn(dx), sgn(dy));
    gb.buzzer = 0;

    /* caster may have died from passive retaliation along the ray
       (acid/fire/electric passive counterattacks); report caster
       mortality so cast_learned_spell can short-circuit downstream
       caster derefs */
    return !DEADMONSTER(caster);
}

/* Cast an IMMEDIATE spell using mbhit() ray tracing.
   These spells (force bolt, drain life, slow monster, teleport away,
   polymorph, turn undead, entangle, dispel evil) trace a line from
   caster to target and affect things along the path.
   Returns TRUE if caster survived and spell was cast, FALSE if a
   sanity check failed or the caster died mid-cast (passive retaliation
   on an mbhit target, or AT_BOOM target explosion) */
boolean
mcast_immediate_spell(struct monst *caster, coordxy tx, coordxy ty,
                      int spell_otyp)
{
    struct obj *pseudo;
    boolean caster_alive;

    /* set direction from caster to target (tbx/tby are used by mbhit) */
    gt.tbx = tx - caster->mx;
    gt.tby = ty - caster->my;

    /* sanity check - can't cast at own location */
    if (!gt.tbx && !gt.tby)
        return FALSE;

    /* create a pseudo-object representing the spell */
    pseudo = mksobj(spell_otyp, FALSE, FALSE);
    pseudo->blessed = pseudo->cursed = 0;
    pseudo->quan = 20L; /* prevent useup from freeing it */
    /* set carrier so mbhitm can access caster level for damage scaling
       and caster position for various effects */
    pseudo->where = OBJ_MINVENT;
    pseudo->ocarry = caster;

    /* trace the ray and affect targets along the path */
    gm.m_using = TRUE;
    gb.buzzer = caster; /* for kill attribution (pet livelog credit) */
    mbhit(caster, rn1(8, 6), mbhitm, bhito, pseudo);
    gb.buzzer = 0;
    gm.m_using = FALSE;

    /* capture caster mortality before cleaning up pseudo (cleanup must
       run regardless to avoid leaking the obj) */
    caster_alive = !DEADMONSTER(caster);

    /* clean up the pseudo-object */
    pseudo->where = OBJ_FREE; /* prevent obfree complaints */
    pseudo->ocarry = (struct monst *) 0;
    obfree(pseudo, (struct obj *) 0);

    return caster_alive;
}

/* uncurse worn items, loadstones, and magic bags; used by the
   monster-cast remove curse spell */
staticfn void
monster_uncurse_items(struct monst *mtmp, boolean confused)
{
    struct obj *obj;

    for (obj = mtmp->minvent; obj; obj = obj->nobj) {
        /* gold isn't subject to cursing and blessing */
        if (obj->oclass == COIN_CLASS)
            continue;
        if (obj->owornmask || Is_mbag(obj) || obj->otyp == LOADSTONE) {
            if (confused)
                blessorcurse(obj, 2);
            else
                uncurse(obj);
        }
    }
    /* curse changes can unblock gear (e.g. rings under formerly
       cursed gloves); reassess equipment on the next move */
    check_gear_next_turn(mtmp);
}

/* Monster casts a NODIR spell (power word kill, cure blindness,
   cure sickness, remove curse, jumping, levitation, repair armor,
   burning hands, shocking grasp).
   These spells don't require line of sight - they directly affect the
   target. Returns TRUE if spell was cast, FALSE otherwise */
boolean
mcast_nodir_spell(struct monst *caster, struct monst *target,
                  int spell_otyp)
{
    boolean youdefend = (target == &gy.youmonst);

    switch (spell_otyp) {
    case SPE_POWER_WORD_KILL: {
        /* power word kill: HP threshold determines effect;
           100 HP = instant kill zone (player Basic skill default) */
        int threshold = 100;

        if (canseemon(caster) && !Deaf)
            pline_mon(caster, "%s utters a word of power...",
                      Monnam(caster));

        if (youdefend) {
            /* casting at player; while polymorphed the live pool is
               u.mh, u.uhp is the dormant real body */
            int curhp = Upolyd ? u.mh : u.uhp;

            if (immune_death_magic(gy.youmonst.data) || Death_resistance) {
                shieldeff(u.ux, u.uy);
                You("are immune to %s word of power!", l_monnam(caster));
            } else if (curhp <= threshold) {
                /* in kill range */
                if (Antimagic) {
                    int dmg = d(8, 6);

                    shieldeff(u.ux, u.uy);
                    if (Half_spell_damage)
                        dmg /= 2;
                    You("tremble in pain, but resist the deadly spell!");
                    if (dmg >= curhp)
                        dmg = curhp - 1;
                    if (dmg > 0)
                        losehp(dmg, "power word kill", KILLED_BY_AN);
                } else {
                    /* instant death */
                    svk.killer.format = KILLED_BY_AN;
                    Strcpy(svk.killer.name, "power word kill");
                    done(DIED);
                }
            } else {
                /* above threshold: weaken but don't kill */
                if (Antimagic) {
                    int dmg = d(8, 6);

                    shieldeff(u.ux, u.uy);
                    if (Half_spell_damage)
                        dmg /= 2;
                    You("tremble in pain, but resist the deadly spell!");
                    if (dmg >= curhp)
                        dmg = curhp - 1;
                    if (dmg > 0)
                        losehp(dmg, "power word kill", KILLED_BY_AN);
                } else {
                    int loss = rn1(9, 4);

                    You("tremble in agony, but resist the deadly spell!");
                    /* setuhpmax() acts on the polyform's maximum while
                       polymorphed, so feed it that pool's value */
                    setuhpmax(max((Upolyd ? u.mhmax : u.uhpmax) - loss, 1),
                              FALSE);
                    if (Upolyd) {
                        u.mh /= 3;
                        if (u.mh < 1)
                            u.mh = 1;
                    } else {
                        u.uhp /= 3;
                        if (u.uhp < 1)
                            u.uhp = 1;
                    }
                    disp.botl = TRUE;
                }
            }
        } else if (target && !DEADMONSTER(target)) {
            /* casting at monster */
            if (immune_death_magic(target->data) || is_vampshifter(target)) {
                shieldeff(target->mx, target->my);
                if (canseemon(target))
                    pline_mon(target, "%s is immune to %s word of power!",
                              Monnam(target), l_monnam(caster));
            } else if (target->mhp <= threshold) {
                /* in kill range */
                if (resists_magm(target) || defended(target, AD_MAGM)) {
                    shieldeff(target->mx, target->my);
                    if (canseemon(target))
                        pline_mon(target,
                              "%s %s in pain, but resists the deadly spell.",
                                  Monnam(target),
                                  makeplural(growl_sound(target)));
                    (void) damage_mon_nonlethal(target, d(8, 6), AD_DETH,
                                                FALSE);
                } else {
                    if (canseemon(target))
                        pline_mon(target, "%s is %s!", Monnam(target),
                                  rn2(2) ? "annihilated" : "obliterated");
                    target->mhp = 0;
                    if (caster->mtame)
                        set_pet_killer(caster);
                    monkilled(target, (char *) 0, AD_DETH);
                }
            } else {
                /* above threshold: weaken but don't kill */
                if (resists_magm(target) || defended(target, AD_MAGM)) {
                    shieldeff(target->mx, target->my);
                    if (canseemon(target))
                        pline_mon(target,
                              "%s %s in pain, but resists the deadly spell.",
                                  Monnam(target),
                                  makeplural(growl_sound(target)));
                    (void) damage_mon_nonlethal(target, d(8, 6), AD_DETH,
                                                FALSE);
                } else {
                    if (canseemon(target))
                        pline_mon(target, "%s trembles in %s!",
                                  Monnam(target),
                                  rn2(2) ? "agony" : "pain");
                    target->mhpmax -= rn1(9, 4);
                    if (target->mhpmax < 1)
                        target->mhpmax = 1;
                    target->mhp /= 3;
                    if (target->mhp < 1)
                        target->mhp = 1;
                }
            }
        }
        break;
    }
    case SPE_CURE_BLINDNESS:
        /* self-targeting: caster cures own blindness */
        mcureblindness(caster, canseemon(caster));
        break;
    case SPE_CURE_SICKNESS:
        /* self-targeting: caster cures own sickness/disease/withering */
        if (caster->msick || caster->mdiseased || caster->mwither) {
            if (canseemon(caster)) {
                if (caster->msick || caster->mdiseased)
                    pline_mon(caster, "%s is no longer ill.",
                              Monnam(caster));
                if (caster->mwither)
                    pline_mon(caster, "%s is no longer withering away.",
                              Monnam(caster));
            }
            caster->msick = 0;
            caster->msickbyu = 0;
            caster->mdiseased = 0;
            caster->mdiseabyu = 0;
            caster->msicktime = 0;
            caster->mdiseasetime = 0;
            caster->mwither = 0;
        }
        break;
    case SPE_REMOVE_CURSE:
        /* self-targeting: uncurse worn items, loadstones, magic bags */
        if (canseemon(caster) && !Deaf)
            pline_mon(caster, "%s chants a prayer...", Monnam(caster));
        monster_uncurse_items(caster, caster->mconf != 0);
        break;
    case SPE_REPAIR_ARMOR: {
        /* self-targeting: repair one level of erosion on random armor */
        struct obj *otmp = some_armor(caster);

        if (otmp && greatest_erosion(otmp) > 0) {
            if (canseemon(caster))
                pline_mon(caster, "%s %s glows faintly golden for a moment.",
                          s_suffix(Monnam(caster)), xname(otmp));
            if (otmp->oeroded > 0)
                otmp->oeroded--;
            if (otmp->oeroded2 > 0)
                otmp->oeroded2--;
        }
        break;
    }
    case SPE_LEVITATION:
        /* self-targeting: grant temporary levitation */
        if (!is_floater(caster->data) && !is_flyer(caster->data)
            && !can_levitate(caster) && !can_fly(caster)) {
            caster->mextrinsics |= MR2_LEVITATE;
            caster->mlevitatetime = rn1(50, 50); /* 50-100 turns */
            if (canseemon(caster))
                pline_mon(caster, "%s begins to float in the air!",
                          Monnam(caster));
        }
        break;
    case SPE_JUMPING:
        /* self-targeting: grant temporary jumping ability */
        if (!is_jumper(caster->data) && !can_jump(caster)) {
            caster->mextrinsics |= MR2_JUMPING;
            caster->mjumptime = rn1(50, 50); /* 50-100 turns */
            if (canseemon(caster))
                pline_mon(caster, "%s %s tense with magical energy.",
                          s_suffix(Monnam(caster)),
                          makeplural(mbodypart(caster, LEG)));
        }
        break;
    case SPE_BURNING_HANDS:
        /* self-targeting: buff that adds fire damage to melee attacks */
        if (!caster->mshockgrasp && !mon_underwater(caster)) {
            caster->mburnhands += rn1(4, 2); /* 2-5 charges */
            if (caster->mburnhands > 20)
                caster->mburnhands = 20; /* cap at 20 */
            if (canseemon(caster))
                pline_mon(caster, "%s %s are engulfed in %s flames!",
                          s_suffix(Monnam(caster)),
                          makeplural(mbodypart(caster, HAND)),
                          hcolor(NH_ORANGE));
        }
        break;
    case SPE_SHOCKING_GRASP:
        /* self-targeting: buff that adds shock damage to melee attacks */
        if (!caster->mburnhands) {
            caster->mshockgrasp += rn1(4, 2); /* 2-5 charges */
            if (caster->mshockgrasp > 20)
                caster->mshockgrasp = 20; /* cap at 20 */
            if (canseemon(caster))
                pline_mon(caster, "%s %s are surrounded by a %s aura!",
                          s_suffix(Monnam(caster)),
                          makeplural(mbodypart(caster, HAND)),
                          hcolor(NH_BLUE));
        }
        break;
    default:
        impossible("mcast_nodir_spell: unhandled spell %d", spell_otyp);
        return FALSE;
    }
    return TRUE;
}

/* Perform an offensive action for a monster.  Must be called immediately
 * after find_offensive().  Return values are same as use_defensive().
 */
int
use_offensive(struct monst *mtmp)
{
    int i;
    struct obj *otmp = gm.m.offensive;
    boolean oseen;

    /* if a monster has never used an attack wand before, it takes them some
       time to get used to holding that much power, so the first shot always
       misses */
    void (*buzzfn)(int, int, coordxy, coordxy, int, int) =
        mtmp->mwandexp ? buzz : buzz_force_miss;

    /* offensive potions are not drunk, they're thrown */
    if (otmp->oclass != POTION_CLASS && (i = precheck(mtmp, otmp)) != 0)
        return i;
    oseen = canseemon(mtmp);

    switch (gm.m.has_offense) {
    case MUSE_WAN_DEATH:
    case MUSE_WAN_SLEEP:
    case MUSE_WAN_FIRE:
    case MUSE_WAN_COLD:
    case MUSE_WAN_LIGHTNING:
    case MUSE_WAN_MAGIC_MISSILE:
        mzapwand(mtmp, otmp, FALSE);
        if (oseen)
            makeknown(otmp->otyp);
        gm.m_using = TRUE;
        gc.current_wand = otmp;
        gb.buzzer = mtmp;
        buzzfn(BZ_M_WAND(BZ_OFS_WAN(otmp->otyp)),
               (otmp->otyp == WAN_MAGIC_MISSILE) ? 2 : 6, mtmp->mx, mtmp->my,
               sgn(gt.tbx), sgn(gt.tby));
        gb.buzzer = 0;
        gc.current_wand = 0;
        gm.m_using = FALSE;
        mtmp->mwandexp = TRUE;
        return (DEADMONSTER(mtmp)) ? 1 : 2;
    case MUSE_FIRE_HORN:
    case MUSE_FROST_HORN:
        mplayhorn(mtmp, otmp, FALSE);
        gm.m_using = TRUE;
        gb.buzzer = mtmp;
        gc.current_wand = otmp; /* needed by zhitu() */
        buzzfn(BZ_M_WAND(BZ_OFS_AD(
                             (otmp->otyp == FROST_HORN) ? AD_COLD : AD_FIRE)),
             rn1(6, 6), mtmp->mx, mtmp->my, sgn(gt.tbx), sgn(gt.tby));
        gb.buzzer = 0;
        gc.current_wand = 0;
        gm.m_using = FALSE;
        mtmp->mwandexp = TRUE;
        return (DEADMONSTER(mtmp)) ? 1 : 2;
    case MUSE_WAN_TELEPORTATION:
    case MUSE_WAN_UNDEAD_TURNING:
    case MUSE_WAN_STRIKING:
    case MUSE_WAN_SLOW_MONSTER:
        gz.zap_oseen = oseen;
        mzapwand(mtmp, otmp, FALSE);
        gm.m_using = TRUE;
        gb.buzzer = mtmp;
        mbhit(mtmp, rn1(8, 6), mbhitm, bhito, otmp);
        gb.buzzer = 0;
        /* note: 'otmp' might have been destroyed (drawbridge destruction) */
        gm.m_using = FALSE;
        /* the zapper can die in its own beam (a knockback's landing
           fires a trap, a drawbridge falls) */
        if (DEADMONSTER(mtmp))
            return 1;
        if (gm.m.has_offense == MUSE_WAN_STRIKING)
            mtmp->mwandexp = TRUE;
        return 2;
    case MUSE_SCR_EARTH: {
        /* TODO: handle steeds */
        coordxy x, y;
        /* don't use monster fields after killing it */
        boolean confused = (mtmp->mconf ? TRUE : FALSE);
        int mmx = mtmp->mx, mmy = mtmp->my;
        boolean is_cursed = otmp->cursed, is_blessed = otmp->blessed;

        mreadmsg(mtmp, otmp);
        /* Identify the scroll */
        if (canspotmon(mtmp)) {
            pline_The("%s rumbles %s %s!", ceiling(mtmp->mx, mtmp->my),
                      otmp->blessed ? "around" : "above", mon_nam(mtmp));
            if (oseen)
                makeknown(otmp->otyp);
        } else if (cansee(mtmp->mx, mtmp->my)) {
            pline_The("%s rumbles in the middle of nowhere!",
                      ceiling(mtmp->mx, mtmp->my));
            if (mtmp->minvis)
                map_invisible(mtmp->mx, mtmp->my);
            if (oseen)
                makeknown(otmp->otyp);
        }

        /* could be fatal to monster, so use up the scroll before
           there's a chance that monster's inventory will be dropped */
        m_useup(mtmp, otmp);

        /* Loop through the surrounding squares */
        for (x = mmx - 1; x <= mmx + 1; x++) {
            for (y = mmy - 1; y <= mmy + 1; y++) {
                /* Is this a suitable spot? */
                if (isok(x, y) && !closed_door(x, y)
                    && !IS_OBSTRUCTED(levl[x][y].typ) && !IS_AIR(levl[x][y].typ)
                    && (((x == mmx) && (y == mmy)) ? !is_blessed : !is_cursed)
                    && (x != u.ux || y != u.uy)) {
                    (void) drop_boulder_on_monster(x, y, confused, FALSE);
                }
            }
        }
        /* Attack the player */
        if (distmin(mmx, mmy, u.ux, u.uy) == 1 && !is_cursed) {
            drop_boulder_on_player(confused, !is_cursed, FALSE, TRUE);
        }

        return (DEADMONSTER(mtmp)) ? 1 : 2;
    } /* case MUSE_SCR_EARTH */
    case MUSE_CAMERA: {
        if (Hallucination) {
            SetVoice(mtmp, 0, 80, 0);
            verbalize("Say cheese!");
        } else if (!Blind) {
            pline("%s takes a picture of you with %s!",
                  Monnam(mtmp), an(xname(otmp)));
        }
        gm.m_using = TRUE;
        if (!Blind && !resists_blnd(&gy.youmonst)) {
            You("are blinded by the flash of light!");
            make_blinded(BlindedTimeout + (long) rnd(1 + 50), FALSE);
        }
        lightdamage(otmp, mtmp, TRUE, 5);
        gm.m_using = FALSE;
        otmp->spe--;
        return 1;
    } /* case MUSE_CAMERA */
#if 0
    case MUSE_SCR_FIRE: {
        boolean vis = cansee(mtmp->mx, mtmp->my);

        mreadmsg(mtmp, otmp);
        if (mtmp->mconf) {
            if (vis)
                pline("Oh, what a pretty fire!");
        } else {
            struct monst *mtmp2;
            int num;

            if (vis)
                pline_The("scroll erupts in a tower of flame!");
            shieldeff(mtmp->mx, mtmp->my);
            pline_mon(mtmp, "%s is uninjured.", Monnam(mtmp));
            (void) destroy_mitem(mtmp, SCROLL_CLASS, AD_FIRE);
            (void) destroy_mitem(mtmp, SPBOOK_CLASS, AD_FIRE);
            (void) destroy_mitem(mtmp, POTION_CLASS, AD_FIRE);
            ignite_items(mtmp->minvent);
            num = (2 * (rn1(3, 3) + 2 * bcsign(otmp)) + 1) / 3;
            burn_away_slime();
            if (how_resistant(FIRE_RES) == 100) {
                You("are not harmed.");
                monstseesu(M_SEEN_FIRE);
            } else {
                if (Half_spell_damage)
                    num = (num + 1) / 2;
                losehp(num, "scroll of fire", KILLED_BY_AN);
            }
            for (mtmp2 = fmon; mtmp2; mtmp2 = mtmp2->nmon) {
                if (DEADMONSTER(mtmp2))
                    continue;
                if (mtmp == mtmp2)
                    continue;
                if (dist2(mtmp2->mx, mtmp2->my, mtmp->mx, mtmp->my) < 3) {
                    if (resists_fire(mtmp2))
                        continue;
                    mtmp2->mhp -= num;
                    if (resists_cold(mtmp2))
                        mtmp2->mhp -= 3 * num;
                    if (DEADMONSTER(mtmp2)) {
                        mondied(mtmp2);
                        break;
                    }
                }
            }
        }
        return 2;
    }
#endif /* 0 */
    case MUSE_POT_PARALYSIS:
    case MUSE_POT_BLINDNESS:
    case MUSE_POT_CONFUSION:
    case MUSE_POT_SLEEPING:
    case MUSE_POT_ACID:
        /* Note: this setting of dknown doesn't suffice.  A monster
         * which is out of sight might throw and it hits something _in_
         * sight, a problem not existing with wands because wand rays
         * are not objects.  Also set dknown in mthrowu.c.
         */
        if (cansee(mtmp->mx, mtmp->my)) {
            observe_object(otmp);
            pline_mon(mtmp, "%s hurls %s!",
                      Monnam(mtmp), singular(otmp, doname));
        }
        /* use target coordinates (gt.tbx/gt.tby) from mfind_target() */
        m_throw(mtmp, mtmp->mx, mtmp->my, sgn(gt.tbx), sgn(gt.tby),
                distmin(mtmp->mx, mtmp->my,
                        mtmp->mx + gt.tbx, mtmp->my + gt.tby), otmp);
        return 2;
    case 0:
        return 0; /* i.e. an exploded wand */
    default:
        impossible("%s wanted to perform action %d?", Monnam(mtmp),
                   gm.m.has_offense);
        break;
    }
    return 0;
}

int
rnd_offensive_item(struct monst *mtmp)
{
    struct permonst *pm = mtmp->data;
    int difficulty = mons[(monsndx(pm))].difficulty;

    if (is_animal(pm) || attacktype(pm, AT_EXPL) || mindless(mtmp->data)
        || pm->mlet == S_GHOST || pm->mlet == S_KOP)
        return 0;
    if (difficulty > 7 && !rn2(35))
        return WAN_DEATH;
    switch (rn2(9 - (difficulty < 4) + 4 * (difficulty > 6))) {
    case 0: {
        struct obj *mtmp_helmet = which_armor(mtmp, W_ARMH);

        if (hard_helmet(mtmp_helmet) || amorphous(pm)
            || passes_walls(pm) || noncorporeal(pm) || unsolid(pm))
            return SCR_EARTH;
    }
    FALLTHROUGH;
    /* FALLTHRU */
    case 1:
        return WAN_STRIKING;
    case 2:
        return POT_ACID;
    case 3:
        return POT_CONFUSION;
    case 4:
        return POT_BLINDNESS;
    case 5:
        return POT_SLEEPING;
    case 6:
        return POT_PARALYSIS;
    case 7:
    case 8:
        return WAN_MAGIC_MISSILE;
    case 9:
        return WAN_SLEEP;
    case 10:
        /* wands of fire are rare in the Ice Queen's realm */
        if (Iniceq)
            return rn2(6) ? WAN_SLEEP : rn2(3) ? WAN_LIGHTNING : WAN_FIRE;
        return WAN_FIRE;
    case 11:
        return WAN_COLD;
    case 12:
        return WAN_LIGHTNING;
    }
    /*NOTREACHED*/
    return 0;
}

#define MUSE_POT_GAIN_LEVEL 1
#define MUSE_WAN_MAKE_INVISIBLE 2
#define MUSE_POT_INVISIBILITY 3
#define MUSE_POLY_TRAP 4
#define MUSE_WAN_POLYMORPH 5
#define MUSE_POT_SPEED 6
#define MUSE_WAN_SPEED_MONSTER 7
#define MUSE_BULLWHIP 8
#define MUSE_POT_POLYMORPH 9
/* value 10 was MUSE_BAG, retired when carried-container search made
   random bag rummaging obsolete */
#define MUSE_SPELLBOOK 11
#define MUSE_POT_BOOZE 12

/* spells that monsters can learn from spellbooks.
   Excludes: racial abilities (psionic wave), spells with existing
   monster versions (haste, invisibility, protection, etc), divination
   (useless), and redundant summoning spells */
static const short learnable_spells[] = {
    /* attack */
    SPE_FORCE_BOLT, SPE_MAGIC_MISSILE, SPE_DRAIN_LIFE,
    SPE_FIREBALL, SPE_CONE_OF_COLD, SPE_LIGHTNING,
    SPE_POISON_BLAST, SPE_ACID_BLAST, SPE_POWER_WORD_KILL,
    /* healing */
    SPE_CURE_BLINDNESS, SPE_CURE_SICKNESS,
    /* enchantment */
    SPE_BURNING_HANDS, SPE_SLEEP, SPE_SLOW_MONSTER,
    SPE_SHOCKING_GRASP, SPE_CHARM_MONSTER,
    /* cleric */
    SPE_REMOVE_CURSE, SPE_TURN_UNDEAD, SPE_DISPEL_EVIL,
    /* escape */
    SPE_JUMPING, SPE_LEVITATION, SPE_TELEPORT_AWAY,
    /* matter */
    SPE_REPAIR_ARMOR, SPE_POLYMORPH,
    /* evocation */
    SPE_ENTANGLE, SPE_FINGER_OF_DEATH,
    0 /* sentinel */
};

/* check if spell is in the learnable whitelist */
staticfn boolean
spell_is_learnable(short otyp)
{
    int i;

    for (i = 0; learnable_spells[i] != 0; i++) {
        if (learnable_spells[i] == otyp)
            return TRUE;
    }
    return FALSE;
}

/* can monster learn spell from this book? */
staticfn boolean
mcan_learn_spell(struct monst *mtmp, struct obj *book)
{
    if (!is_spellcaster(mtmp))
        return FALSE;
    if (mindless(mtmp->data))
        return FALSE;
    /* monster state checks - can't read if impaired */
    if (!mtmp->mcansee) /* blind */
        return FALSE;
    if (mtmp->mstun || mtmp->mconf || helpless(mtmp))
        return FALSE;
    /* book checks */
    if (book->cursed)
        return FALSE; /* dangerous */
    if (mknows_spell(mtmp, book->otyp))
        return FALSE; /* already knows */
    if (book->spestudied >= MAX_SPELL_STUDY)
        return FALSE; /* depleted */
    /* check if spell is in the learnable whitelist */
    if (!spell_is_learnable(book->otyp))
        return FALSE;
    /* check if monster has room for another spell */
    if (has_emsp(mtmp)) {
        int i;
        boolean has_room = FALSE;

        for (i = 0; i < MAXMONSPELL; i++) {
            if (EMSP(mtmp)->msp_id[i] == 0
                || EMSP(mtmp)->msp_know[i] <= 0) {
                has_room = TRUE;
                break;
            }
        }
        if (!has_room)
            return FALSE; /* all spell slots occupied */
    }
    return TRUE;
}

/* recursive helper to find a spellbook in inventory or containers.
   If otyp is non-zero, find a book with that specific otyp (mid-read
   check). If otyp is zero, find any learnable book using
   mcan_learn_spell() */
staticfn struct obj *
find_spellbook_recurse(struct monst *mtmp, struct obj *start, int otyp)
{
    struct obj *obj, *found;

    for (obj = start; obj; obj = obj->nobj) {
        /* recurse into containers */
        if (Is_container(obj) && obj->cobj) {
            found = find_spellbook_recurse(mtmp, obj->cobj, otyp);
            if (found)
                return found;
            continue;
        }

        if (obj->oclass != SPBOOK_CLASS)
            continue;

        if (otyp != 0) {
            /* looking for specific book type (mid-read continuation) */
            if (obj->otyp == otyp)
                return obj;
        } else {
            /* looking for any learnable book */
            if (mcan_learn_spell(mtmp, obj))
                return obj;
        }
    }
    return (struct obj *) 0;
}

/* Does this spellcasting monster want to keep/pick up this spellbook?
   TRUE if it can still learn the spell, or is partway through reading
   this exact book (so a transient stun/threat doesn't make it drop a
   read in progress). Mirrors mon_wants_ammo() */
boolean
mon_wants_spellbook(struct monst *mtmp, struct obj *otmp)
{
    if (!otmp || otmp->oclass != SPBOOK_CLASS)
        return FALSE;
    if (!is_spellcaster(mtmp))
        return FALSE;
    if (mcan_learn_spell(mtmp, otmp))
        return TRUE;
    /* keep a book a read is in progress on even if mcan_learn_spell()
       transiently fails (e.g. brief stun/confusion) so the monster can
       resume instead of dropping and restarting */
    if (has_emsp(mtmp) && EMSP(mtmp)->msp_reading != 0
        && EMSP(mtmp)->msp_reading == otmp->otyp)
        return TRUE;
    return FALSE;
}

boolean
find_misc(struct monst *mtmp)
{
    struct permonst *mdat = mtmp->data;
    coordxy x = mtmp->mx, y = mtmp->my;
    struct trap *t;
    coordxy xx, yy;
    int pmidx = NON_PM;
    boolean immobile = (mdat->mmove == 0);
    boolean stuck = (mtmp == u.ustuck);

    gm.m.misc = (struct obj *) 0;
    gm.m.has_misc = 0;
    if (is_animal(mdat) || mindless(mdat))
        return 0;
    if (u.uswallow && stuck)
        return FALSE;

    /* spellbook reading - checked before the distance check since
       spellcasters should read regardless of where they think the
       player is */
    if (is_spellcaster(mtmp) && !nohands(mdat) && !helpless(mtmp)) {
        struct obj *book;

        /* if mid-read, continue unless impaired or hostile and player
           is close */
        if (has_emsp(mtmp) && EMSP(mtmp)->msp_reading != 0
            && EMSP(mtmp)->msp_read_turns > 0) {
            if (!mtmp->mcansee || mtmp->mstun || mtmp->mconf
                || helpless(mtmp)) {
                /* stop reading if monster becomes impaired */
                EMSP(mtmp)->msp_reading = 0;
                EMSP(mtmp)->msp_read_turns = 0;
                /* fall through to find other action */
            } else if (!mtmp->mpeaceful && dist2(x, y, u.ux, u.uy) <= 8) {
                /* hostile monsters stop reading when player is within
                   two tile spaces */
                EMSP(mtmp)->msp_reading = 0;
                EMSP(mtmp)->msp_read_turns = 0;
                /* fall through to find a new book or other action */
            } else {
                /* find the book being read (may be in a container) */
                book = find_spellbook_recurse(mtmp, mtmp->minvent,
                                              EMSP(mtmp)->msp_reading);
                if (book) {
                    gm.m.misc = book;
                    gm.m.has_misc = MUSE_SPELLBOOK;
                    return TRUE;
                }
                /* book not found - must have been stolen/destroyed */
                EMSP(mtmp)->msp_reading = 0;
                EMSP(mtmp)->msp_read_turns = 0;
            }
        }

        /* find any learnable spellbook - but hostile monsters won't
           start reading when player is within two tile spaces; also
           search containers */
        if (mtmp->mpeaceful || dist2(x, y, u.ux, u.uy) > 8) {
            book = find_spellbook_recurse(mtmp, mtmp->minvent, 0);
            if (book) {
                gm.m.misc = book;
                gm.m.has_misc = MUSE_SPELLBOOK;
                return TRUE;
            }
        }
    }

    /* We arbitrarily limit to times when a player is nearby for the
     * same reason as Junior Pac-Man doesn't have energizers eaten until
     * you can see them...
     */
    if (dist2(x, y, mtmp->mux, mtmp->muy) > 36)
        return FALSE;

    if (!stuck && !immobile && !mtmp->mtrapped && !mtmp->mentangled
        && (mtmp->cham == NON_PM)
        && mons[(pmidx = monsndx(mdat))].difficulty < 6) {
        boolean ignore_boulders = (verysmall(mdat) || racial_throws_rocks(mtmp)
                                   || passes_walls(mdat)),
            diag_ok = !NODIAG(pmidx);

        for (xx = x - 1; xx <= x + 1; xx++)
            for (yy = y - 1; yy <= y + 1; yy++)
                if (isok(xx, yy) && !u_at(xx, yy)
                    && (diag_ok || xx == x || yy == y)
                    && ((xx == x && yy == y) || !svl.level.monsters[xx][yy]))
                    if ((t = t_at(xx, yy)) != 0
                        && (ignore_boulders || !sobj_at(BOULDER, xx, yy))
                        && !onscary(xx, yy, mtmp)) {
                        /* use trap if it's the correct type and will
                           polymorph the monster */
                        if (t->ttyp == POLY_TRAP_SET &&
                            !wearing_iron_shoes(mtmp)) {
                            gt.trapx = xx;
                            gt.trapy = yy;
                            gm.m.has_misc = MUSE_POLY_TRAP;
                            return TRUE;
                        }
                    }
    }
    if (nohands(mdat))
        return 0;

    /* use the generic finder callback for misc items; this handles
       both direct inventory and container recursion */
    {
        struct find_context ctx;

        ctx.target = (struct monst *) 0;
        ctx.reflection_skip = FALSE;
        ctx.helmet = (struct obj *) 0;
        ctx.t = (struct trap *) 0;
        ctx.stuck = FALSE;

        return find_items_recurse(mtmp, mtmp->minvent, FIND_MISC,
                                  check_misc_item, &ctx);
    }
}

/* callback for checking misc items */
staticfn int
check_misc_item(
    struct monst *mtmp,
    struct obj *obj,
    int current_muse,
    struct find_context *ctx UNUSED)
{
    struct permonst *mdat = mtmp->data;

#define nomore(x) if (current_muse == (x)) return 0

    nomore(MUSE_POT_BOOZE);
    /* satyrs drink any booze they get hold of; a cursed potion
       sends them into a rage rather than pacifying them */
    if (obj->otyp == POT_BOOZE && is_satyr(mdat))
        return MUSE_POT_BOOZE;

    nomore(MUSE_POT_GAIN_LEVEL);
    /* Monsters shouldn't recognize cursed items; this kludge is
       necessary to prevent serious problems though */
    if (obj->otyp == POT_GAIN_LEVEL
        && (!obj->cursed
            || (!mtmp->isgd && !mtmp->isshk && !mtmp->ispriest)))
        return MUSE_POT_GAIN_LEVEL;

    nomore(MUSE_BULLWHIP);
    if (obj->otyp == BULLWHIP && !mtmp->mpeaceful
        /* the random test prevents whip-wielding
           monster from attempting disarm every turn */
        && uwep && !rn2(5) && obj == MON_WEP(mtmp)
        /* hero's location must be known and adjacent */
        && u_at(mtmp->mux, mtmp->muy)
        && m_next2u(mtmp)
        /* don't bother if it can't work (this doesn't
           prevent cursed weapons from being targeted) */
        && !u.uswallow
        && (canletgo(uwep, "")
            || (u.twoweap && canletgo(uswapwep, ""))))
        return MUSE_BULLWHIP;

    /* Note: peaceful/tame monsters won't make themselves
     * invisible unless you can see them. Not really right, but...
     */
    nomore(MUSE_WAN_MAKE_INVISIBLE);
    if (obj->otyp == WAN_MAKE_INVISIBLE && obj->spe > 0 && !mtmp->minvis
        && !mtmp->invis_blkd && (!mtmp->mpeaceful || See_invisible)
        && (!attacktype(mdat, AT_GAZE) || mtmp->mcan))
        return MUSE_WAN_MAKE_INVISIBLE;

    nomore(MUSE_POT_INVISIBILITY);
    if (obj->otyp == POT_INVISIBILITY && !mtmp->minvis
        && !mtmp->invis_blkd && (!mtmp->mpeaceful || See_invisible)
        && (!attacktype(mdat, AT_GAZE) || mtmp->mcan))
        return MUSE_POT_INVISIBILITY;

    nomore(MUSE_WAN_SPEED_MONSTER);
    if (obj->otyp == WAN_SPEED_MONSTER && obj->spe > 0
        && mtmp->mspeed != MFAST && !mtmp->isgd)
        return MUSE_WAN_SPEED_MONSTER;

    nomore(MUSE_POT_SPEED);
    if (obj->otyp == POT_SPEED && mtmp->mspeed != MFAST && !mtmp->isgd)
        return MUSE_POT_SPEED;

    nomore(MUSE_WAN_POLYMORPH);
    if (obj->otyp == WAN_POLYMORPH && obj->spe > 0
        && (mtmp->cham == NON_PM) && mons[monsndx(mdat)].difficulty < 6)
        return MUSE_WAN_POLYMORPH;

    nomore(MUSE_POT_POLYMORPH);
    if (obj->otyp == POT_POLYMORPH && (mtmp->cham == NON_PM)
        && mons[monsndx(mdat)].difficulty < 6)
        return MUSE_POT_POLYMORPH;

    return 0;
#undef nomore
}

/* type of monster to polymorph into; defaults to one suitable for the
   current level rather than the totally arbitrary choice of newcham() */
staticfn struct permonst *
muse_newcham_mon(struct monst *mon)
{
    struct obj *m_armr;

    if ((m_armr = which_armor(mon, W_ARM)) != 0) {
        if (Is_dragon_armor(m_armr))
            return &mons[Dragon_armor_to_pm(m_armr)];
    }
    return rndmonst();
}

DISABLE_WARNING_UNREACHABLE_CODE

int
use_misc(struct monst *mtmp)
{
    static const char MissingMiscellaneousItem[] = "use_misc: no %s";
    char nambuf[BUFSZ];
    boolean vis, vismon, vistrapspot, oseen;
    int i;
    struct trap *t;
    struct obj *otmp = gm.m.misc;

    if ((i = precheck(mtmp, otmp)) != 0)
        return i;
    vis = cansee(mtmp->mx, mtmp->my);
    vismon = canseemon(mtmp);
    oseen = otmp && vismon;

    switch (gm.m.has_misc) {
    case MUSE_POT_GAIN_LEVEL:
        if (!otmp)
            panic(MissingMiscellaneousItem, "potion of gain level");
        mquaffmsg(mtmp, otmp);
        if (otmp->cursed) {
            if (Can_rise_up(mtmp->mx, mtmp->my, &u.uz)) {
                int tolev = depth(&u.uz) - 1;
                d_level tolevel;

                get_level(&tolevel, tolev);
                /* insurance against future changes...
                   Lucifer never abandons his post in the sanctum, and
                   the Ice Queen's curse binds her to her realm */
                if (on_level(&tolevel, &u.uz)
                    || (Iniceq
                        && mtmp->data == &mons[PM_KATHRYN_THE_ICE_QUEEN])
                    || (Is_sanctum(&u.uz)
                        && mtmp->data == &mons[PM_LUCIFER]))
                    goto skipmsg;
                if (vismon) {
                    pline_mon(mtmp, "%s rises up, through the %s!",
                              Monnam(mtmp),
                              ceiling(mtmp->mx, mtmp->my));
                    trycall(otmp);
                }
                m_useup(mtmp, otmp);
                migrate_to_level(mtmp, ledger_no(&tolevel), MIGR_RANDOM,
                                 (coord *) 0);
                return 2;
            } else {
 skipmsg:
                if (vismon) {
                    pline_mon(mtmp, "%s looks uneasy.", Monnam(mtmp));
                    trycall(otmp);
                }
                m_useup(mtmp, otmp);
                return 2;
            }
        }
        if (vismon)
            pline_mon(mtmp, "%s seems more experienced.", Monnam(mtmp));
        if (oseen)
            makeknown(POT_GAIN_LEVEL);
        m_useup(mtmp, otmp);
        if (!grow_up(mtmp, (struct monst *) 0))
            return 1;
        /* grew into genocided monster */
        return 2;
    case MUSE_WAN_MAKE_INVISIBLE:
    case MUSE_POT_INVISIBILITY:
        if (!otmp)
            panic(MissingMiscellaneousItem, "potion of invisibility");
        if (otmp->otyp == WAN_MAKE_INVISIBLE) {
            mzapwand(mtmp, otmp, TRUE);
        } else
            mquaffmsg(mtmp, otmp);
        /* format monster's name before altering its visibility */
        Strcpy(nambuf, mon_nam(mtmp));
        mon_set_minvis(mtmp, !otmp->cursed ? FALSE : TRUE);
        if (vismon && mtmp->minvis) { /* was seen, now invisible */
            if (canspotmon(mtmp)) {
                pline("%s body takes on a %s transparency.",
                      upstart(s_suffix(nambuf)),
                      Hallucination ? "normal" : "strange");
            } else {
                pline("Suddenly you cannot see %s.", nambuf);
                if (vis)
                    map_invisible(mtmp->mx, mtmp->my);
            }
            if (oseen)
                makeknown(otmp->otyp);
        } else if (vismon && !mtmp->minvis) {
            /* cursed potion; mon tried to make itself invisible but failed */
            pline("%s briefly seems to be transparent.", Monnam(mtmp));
            /* we could call map_invisible() before the pline(), then
               newsym() after; unseen monster glyph would be visible during
               the pline, but hero would forget any remembered object under
               the monster */
        } else if (!vismon && canseemon(mtmp)) {
            /* cursed potion; this won't happen because a monster will only
               drink a potion of invisibility when not already invisible */
            pline("%s suddenly appears!", Monnam(mtmp));
        }
        if (otmp->otyp == POT_INVISIBILITY) {
            if (otmp->cursed)
                you_aggravate(mtmp);
            m_useup(mtmp, otmp);
        }
        return 2;
    case MUSE_WAN_SPEED_MONSTER:
        if (!otmp)
            panic(MissingMiscellaneousItem, "wand of speed monster");
        mzapwand(mtmp, otmp, TRUE);
        mon_adjust_speed(mtmp, 1, otmp);
        return 2;
    case MUSE_POT_SPEED:
        if (!otmp)
            panic(MissingMiscellaneousItem, "potion of speed");
        mquaffmsg(mtmp, otmp);
        /* note difference in potion effect due to substantially
           different methods of maintaining speed ratings:
           player's character becomes "very fast" temporarily;
           monster becomes "one stage faster" permanently */
        mon_adjust_speed(mtmp, 1, otmp);
        m_useup(mtmp, otmp);
        return 2;
    case MUSE_WAN_POLYMORPH:
        if (!otmp)
            panic(MissingMiscellaneousItem, "wand of polymorph");
        mzapwand(mtmp, otmp, TRUE);
        (void) newcham(mtmp, muse_newcham_mon(mtmp),
                       NC_VIA_WAND_OR_SPELL | NC_SHOW_MSG);
        if (oseen)
            makeknown(WAN_POLYMORPH);
        return 2;
    case MUSE_POT_POLYMORPH:
        if (!otmp)
            panic(MissingMiscellaneousItem, "potion of polymorph");
        mquaffmsg(mtmp, otmp);
        m_useup(mtmp, otmp);
        if (vismon)
            pline_mon(mtmp, "%s suddenly mutates!", Monnam(mtmp));
        (void) newcham(mtmp, muse_newcham_mon(mtmp), NC_SHOW_MSG);
        if (oseen)
            makeknown(POT_POLYMORPH);
        return 2;
    case MUSE_POLY_TRAP:
        t = t_at(gt.trapx, gt.trapy);
        vistrapspot = cansee(t->tx, t->ty);
        if (vis || vistrapspot)
            seetrap(t);
        if (vismon || vistrapspot) {
            pline_mon(mtmp, "%s deliberately %s onto a %s!", Some_Monnam(mtmp),
                  vtense(fakename[0], locomotion(mtmp->data, "jump")),
                  t->tseen ? trapname(t->ttyp, FALSE) : "hidden trap");
            /* note: if mtmp is unseen because it is invisible, its new
               shape will also be invisible and could produce "Its armor
               falls off" messages during the transformation; those make
               more sense after we've given "Someone jumps onto a trap." */
        }

        /*  don't use rloc() due to worms */
        remove_monster(mtmp->mx, mtmp->my);
        newsym(mtmp->mx, mtmp->my);
        place_monster(mtmp, gt.trapx, gt.trapy);
        maybe_unhide_at(gt.trapx, gt.trapy);
        if (mtmp->wormno)
            worm_move(mtmp);
        newsym(gt.trapx, gt.trapy);

        (void) newcham(mtmp, (struct permonst *) 0, NC_SHOW_MSG);
        return 2;
    case MUSE_SPELLBOOK: {
        int spell_level, read_time;
        struct emsp *esp;

        if (!otmp)
            panic(MissingMiscellaneousItem, "spellbook");
        spell_level = objects[otmp->otyp].oc_level;

        /* ensure monster has emsp for tracking reading state */
        if (!has_emsp(mtmp))
            newemsp(mtmp);
        esp = EMSP(mtmp);

        /* check if monster is already reading this book */
        if (esp->msp_reading == otmp->otyp && esp->msp_read_turns > 0) {
            /* continue reading - check for interruption */
            if (!mtmp->mcansee || mtmp->mstun || mtmp->mconf
                || helpless(mtmp)) {
                /* reading interrupted */
                if (vismon)
                    pline_mon(mtmp, "%s stops reading.", Monnam(mtmp));
                esp->msp_reading = 0;
                esp->msp_read_turns = 0;
                return 2;
            }

            /* decrement reading time */
            esp->msp_read_turns--;

            if (esp->msp_read_turns <= 0) {
                /* finished reading - learn the spell */
                boolean success = mlearn_spell(mtmp, otmp);

                esp->msp_reading = 0;
                esp->msp_read_turns = 0;

                if (vismon) {
                    if (success)
                        pline_mon(mtmp,
                            "%s finishes reading and looks more knowledgeable.",
                                  Monnam(mtmp));
                    else
                        pline_mon(mtmp, "%s finishes reading.",
                                  Monnam(mtmp));
                }
            }
            /* still reading, spent a turn */
            return 2;
        }

        /* reading time: 3 turns per spell level,
           blessed books are read in half the time */
        read_time = 3 * spell_level;
        if (otmp->blessed)
            read_time = (read_time + 1) / 2;

        /* start reading */
        esp->msp_reading = otmp->otyp;
        esp->msp_read_turns = read_time;

        if (vismon)
            pline_mon(mtmp, "%s begins reading %s.", Monnam(mtmp),
                      singular(otmp, doname));

        return 2;
    }
    case MUSE_BULLWHIP:
        /* attempt to disarm hero */
        {
            const char *The_whip = vismon ? "The bullwhip" : "A whip";
            int where_to = rn2(4);
            struct obj *obj = uwep;
            const char *hand;
            char the_weapon[BUFSZ], hand_buf[BUFSZ];

            if (!obj || !canletgo(obj, "")
                || (u.twoweap && canletgo(uswapwep, "") && rn2(2)))
                obj = uswapwep;
            if (!obj)
                break; /* shouldn't happen after find_misc() */

            Strcpy(the_weapon, the(xname(obj)));
            hand = body_part(HAND);
            if (bimanual(obj))
                hand = makeplural(hand);
            (void) strncpy(hand_buf, hand, sizeof hand_buf - 1);
            hand_buf[sizeof hand_buf - 1] = '\0';

            if (vismon)
                pline_mon(mtmp, "%s flicks a bullwhip towards your %s!",
                          Monnam(mtmp), hand_buf);
            if (obj->otyp == HEAVY_IRON_BALL || Hidinshell) {
                pline("%s fails to wrap around %s.", The_whip, the_weapon);
                return 1;
            }
            urgent_pline("%s wraps around %s you're wielding!", The_whip,
                         the_weapon);
            if (welded(obj)) {
                pline("%s welded to your %s%c",
                      !is_plural(obj) ? "It is" : "They are", hand_buf,
                      !obj->bknown ? '!' : '.');
                /* obj->bknown = 1; */ /* welded() takes care of this */
                where_to = 0;
            }
            if (!where_to) {
                pline_The("whip slips free."); /* not `The_whip' */
                return 1;
            } else if (where_to == 3
                       && mon_hates_material(mtmp, obj->material)) {
                /* this monster won't want to catch a weapon made
                   of a material it hates; drop it at hero's feet
                   instead */
                where_to = 2;
            }
            remove_worn_item(obj, FALSE);
            freeinv(obj);
            switch (where_to) {
            case 1: /* onto floor beneath mon */
                pline_mon(mtmp, "%s yanks %s from your %s!", Monnam(mtmp),
                          the_weapon, hand_buf);
                place_object(obj, mtmp->mx, mtmp->my);
                break;
            case 2: /* onto floor beneath you */
                pline_mon(mtmp, "%s yanks %s to the %s!", Monnam(mtmp),
                          the_weapon, surface(u.ux, u.uy));
                dropy(obj);
                break;
            case 3: /* into mon's inventory */
                pline_mon(mtmp, "%s snatches %s!", Monnam(mtmp), the_weapon);
                (void) mpickobj(mtmp, obj);
                break;
            }
            return 1;
        }
        /*NOTREACHED*/
        return 0;
    case MUSE_POT_BOOZE:
        if (!otmp)
            panic(MissingMiscellaneousItem, "potion of booze");
        mquaffmsg(mtmp, otmp);
        if (vismon)
            pline_mon(mtmp, "%s looks confused.", Monnam(mtmp));
        if (mtmp->mtame) {
            /* a pet only grows fonder; the cap is an overflow clamp,
               not a route into the hostile arms below */
            if (mtmp->mtame < 20)
                mtmp->mtame++;
        } else if (otmp->cursed) {
            if (vismon)
                pline_mon(mtmp, "He flies into a drunken rage!");
            mtmp->mpeaceful = 0;
            set_malign(mtmp);
            mtmp->mberserk = 1;
        } else {
            if (vismon)
                pline_mon(mtmp, "He is sated.");
            mtmp->mpeaceful = 1;
            set_malign(mtmp);
        }
        mtmp->mconf = 1;
        m_useup(mtmp, otmp);
        if (oseen)
            makeknown(POT_BOOZE);
        /* refresh symbol when changing from hostile to
           peaceful or vice versa */
        newsym(mtmp->mx, mtmp->my);
        return 2;
    case 0:
        return 0; /* i.e. an exploded wand */
    default:
        impossible("%s wanted to perform action %d?", Monnam(mtmp),
                   gm.m.has_misc);
        break;
    }
    return 0;
}

RESTORE_WARNINGS

staticfn void
you_aggravate(struct monst *mtmp)
{
    pline("For some reason, %s presence is known to you.",
          s_suffix(noit_mon_nam(mtmp)));
    cls();
#ifdef CLIPPING
    cliparound(mtmp->mx, mtmp->my);
#endif
    show_glyph(mtmp->mx, mtmp->my, mon_to_glyph(mtmp, rn2_on_display_rng));
    display_self();
    You_feel("aggravated at %s.", noit_mon_nam(mtmp));
    display_nhwindow(WIN_MAP, TRUE);
    docrt();
    if (unconscious()) {
        gm.multi = -1;
        gn.nomovemsg = "Aggravated, you are jolted into full consciousness.";
    }
    newsym(mtmp->mx, mtmp->my);
    if (!canspotmon(mtmp))
        map_invisible(mtmp->mx, mtmp->my);
}

int
rnd_misc_item(struct monst *mtmp)
{
    struct permonst *pm = mtmp->data;
    int difficulty = mons[(monsndx(pm))].difficulty;

    if (is_animal(pm) || attacktype(pm, AT_EXPL) || mindless(mtmp->data)
        || pm->mlet == S_GHOST || pm->mlet == S_KOP)
        return 0;
    /* Unlike other rnd_item functions, we only allow _weak_ monsters
     * to have this item; after all, the item will be used to strengthen
     * the monster and strong monsters won't use it at all...
     */
    if (difficulty < 6 && !rn2(30))
        return rn2(6) ? POT_POLYMORPH : WAN_POLYMORPH;

    if (!rn2(40) && !nonliving(pm) && !is_vampshifter(mtmp))
        return AMULET_OF_LIFE_SAVING;

    switch (rn2(3)) {
    case 0:
        if (mtmp->isgd)
            return 0;
        return rn2(6) ? POT_SPEED : WAN_SPEED_MONSTER;
    case 1:
        if (mtmp->mpeaceful && !See_invisible)
            return 0;
        return rn2(6) ? POT_INVISIBILITY : WAN_MAKE_INVISIBLE;
    case 2:
        return POT_GAIN_LEVEL;
    }
    /*NOTREACHED*/
    return 0;
}

#if 0
/* check whether hero is carrying a corpse or contained petrifier corpse */
staticfn boolean
necrophiliac(struct obj *objlist, boolean any_corpse)
{
    while (objlist) {
        if (objlist->otyp == CORPSE
            && (any_corpse || touch_petrifies(&mons[objlist->corpsenm])))
            return TRUE;
        if (Has_contents(objlist) && necrophiliac(objlist->cobj, FALSE))
            return TRUE;
        objlist = objlist->nobj;
    }
    return FALSE;
}
#endif

boolean
searches_for_item(struct monst *mon, struct obj *obj)
{
    int typ = obj->otyp;

    /* don't let monsters interact with protected items on the floor */
    if (obj->where == OBJ_FLOOR
        && (obj->ox == mon->mx && obj->oy == mon->my)
        && onscary(obj->ox, obj->oy, mon)) {
        return FALSE;
    }

    if (is_animal(mon->data) || mindless(mon->data)
        || mon->data == &mons[PM_GHOST]) /* don't loot bones piles */
        return FALSE;

    if (typ == WAN_MAKE_INVISIBLE || typ == POT_INVISIBILITY)
        return (boolean) (!mon->minvis && !mon->invis_blkd
                          && !attacktype(mon->data, AT_GAZE));
    if (typ == WAN_SPEED_MONSTER || typ == POT_SPEED)
        return (boolean) (mon->mspeed != MFAST);

    switch (obj->oclass) {
    case WAND_CLASS:
        if (obj->spe <= 0)
            return FALSE;
        if (typ == WAN_DIGGING)
            return (boolean) !(is_floater(mon->data) || can_levitate(mon));
        if (typ == WAN_POLYMORPH)
            return (boolean) (mons[monsndx(mon->data)].difficulty < 6);
        if (objects[typ].oc_dir == RAY || typ == WAN_STRIKING
            || typ == WAN_UNDEAD_TURNING || typ == WAN_SLOW_MONSTER
            || typ == WAN_TELEPORTATION || typ == WAN_CREATE_MONSTER)
            return TRUE;
        break;
    case POTION_CLASS:
        if (typ == POT_HEALING || typ == POT_EXTRA_HEALING
            || typ == POT_FULL_HEALING || typ == POT_POLYMORPH
            || typ == POT_GAIN_LEVEL || typ == POT_PARALYSIS
            || typ == POT_SLEEPING || typ == POT_ACID || typ == POT_CONFUSION)
            return TRUE;
        if (typ == POT_BLINDNESS && !attacktype(mon->data, AT_GAZE))
            return TRUE;
        if (typ == POT_BOOZE && is_satyr(mon->data))
            return TRUE;
        break;
    case SCROLL_CLASS:
        if (typ == SCR_TELEPORTATION || typ == SCR_CREATE_MONSTER
            || typ == SCR_EARTH || typ == SCR_FIRE)
            return TRUE;
        break;
    case AMULET_CLASS:
        if (typ == AMULET_OF_LIFE_SAVING)
            return (boolean) !(nonliving(mon->data) || is_vampshifter(mon));
        if (typ == AMULET_OF_REFLECTION || typ == AMULET_OF_FLYING
            || typ == AMULET_OF_GUARDING || typ == AMULET_OF_ESP
            || typ == AMULET_OF_MAGIC_RESISTANCE
            || obj->oartifact == ART_EYE_OF_THE_AETHIOPICA)
            return TRUE;
        break;
    case TOOL_CLASS:
        if (typ == PICK_AXE)
            return (boolean) racial_needspick(mon);
        if (typ == UNICORN_HORN)
            return (boolean) (!obj->cursed && !is_unicorn(mon->data)
                              && mon->data != &mons[PM_KI_RIN]);
        if (typ == FROST_HORN || typ == FIRE_HORN)
            return (obj->spe > 0 && can_blow(mon));
        /* the two rare bags are excluded by otyp, matching find_misc:
           a bag of treasure is meant to stay sealed once a monster has
           one, and a bag of devouring is a disposal unit that should
           never be rummaged */
        if (Is_container(obj) && !(Is_mbag(obj) && obj->cursed)
            && obj->otyp != BAG_OF_TREASURE
            && obj->otyp != BAG_OF_DEVOURING
            && !obj->olocked)
            return TRUE;
        if (typ == EXPENSIVE_CAMERA)
            return (obj->spe > 0);
        break;
    case FOOD_CLASS:
        if (typ == CORPSE && !obj->zombie_corpse)
            return (boolean) (((mon->misc_worn_check & W_ARMG) != 0L
                               && touch_petrifies(&mons[obj->corpsenm]))
                              || (!(resists_ston(mon)
                                    || defended(mon, AD_STON))
                                  && cures_stoning(mon, obj, FALSE)));
        if (typ == TIN)
            return (boolean) (mcould_eat_tin(mon)
                              && (!(resists_ston(mon)
                                    || defended(mon, AD_STON))
                                  && cures_stoning(mon, obj, TRUE)));
        if (typ == EGG && ismnum(obj->corpsenm))
            return (boolean) touch_petrifies(&mons[obj->corpsenm]);
        break;
    case WEAPON_CLASS:
    case GEM_CLASS:
        /* intelligent monsters seek ammo when out */
        if (!nohands(mon->data) && mon_wants_ammo(mon, obj))
            return TRUE;
        break;
    default:
        break;
    }

    return FALSE;
}

/* check if monster is carrying (not wearing/wielding) an artifact
   that grants reflection via cspfx */
staticfn struct obj *
m_carrying_arti_reflecting(struct monst *mon)
{
    struct obj *obj;

    for (obj = mon->minvent; obj; obj = obj->nobj) {
        if (obj->oartifact && !(obj->owornmask & ~W_ART)
            && arti_reflects(obj))
            return obj;
    }
    return (struct obj *) 0;
}

DISABLE_WARNING_FORMAT_NONLITERAL

boolean
mon_reflects(struct monst *mon, const char *str)
{
    struct obj *orefl, *brefl;

    /* the hero's reflection is tracked via Reflecting; route &youmonst
       through ureflects() so we don't misread the hero's gear with
       monster-inventory logic (which both scans youmonst.minvent and
       passes barding/amulet masks that which_armor cannot map) */
    if (mon == &gy.youmonst)
        return ureflects((const char *) 0, (const char *) 0);

    orefl = which_armor(mon, W_ARMS);
    brefl = which_armor(mon, W_BARDING);

    if (orefl && orefl->otyp == SHIELD_OF_REFLECTION) {
        if (str) {
            pline(str, s_suffix(mon_nam(mon)), "shield");
            makeknown(SHIELD_OF_REFLECTION);
        }
        return TRUE;
    } else if (brefl && (brefl->otyp == BARDING_OF_REFLECTION
                         || brefl->oartifact == ART_ITHILMAR)) {
        if (str) {
            pline(str, s_suffix(mon_nam(mon)), "barding");
            if (brefl->otyp == BARDING_OF_REFLECTION)
                makeknown(BARDING_OF_REFLECTION);
        }
        return TRUE;
    } else if ((orefl = which_armor(mon, W_ARMG))
               && orefl->oartifact == ART_DRAGONBANE) {
        if (str)
            pline(str, s_suffix(mon_nam(mon)), "gloves");
        return TRUE;
    } else if ((orefl = which_armor(mon, W_ARMG))
               && orefl->oartifact == ART_GAUNTLETS_OF_PURITY) {
        if (str)
            pline(str, s_suffix(mon_nam(mon)), "gauntlets");
        return TRUE;
    } else if (arti_reflects(MON_WEP(mon))) {
        /* due to wielded artifact weapon */
        if (str)
            pline(str, s_suffix(mon_nam(mon)), "weapon");
        return TRUE;
    } else if (arti_reflects(MON_WEP2(mon))) {
        /* due to off-hand artifact weapon */
        if (str)
            pline(str, s_suffix(mon_nam(mon)), "other weapon");
        return TRUE;
    } else if ((orefl = m_carrying_arti_reflecting(mon)) != 0) {
        /* carried (not worn/wielded) artifact granting reflection,
           e.g. the Magic Mirror of Merlin */
        if (str)
            pline(str, s_suffix(mon_nam(mon)),
                  orefl->otyp == MIRROR ? "mirror" : "artifact");
        return TRUE;
    } else if ((orefl = which_armor(mon, W_AMUL))
               && orefl->otyp == AMULET_OF_REFLECTION) {
        if (str) {
            pline(str, s_suffix(mon_nam(mon)), "amulet");
            makeknown(AMULET_OF_REFLECTION);
        }
        return TRUE;
    } else if ((orefl = which_armor(mon, W_ARM))
               && Is_dragon_scaled_armor(orefl)
               && (Dragon_armor_to_scales(orefl) == SILVER_DRAGON_SCALES
                   || Dragon_armor_to_scales(orefl)
                      == CHROMATIC_DRAGON_SCALES)) {
        if (str)
            pline(str, s_suffix(mon_nam(mon)), "armor");
        return TRUE;
    } else if ((orefl = which_armor(mon, W_ARMC))
               && orefl->otyp == SILVER_DRAGON_SCALES) {
        if (str)
            pline(str, s_suffix(mon_nam(mon)), "set of scales");
        return TRUE;
    } else if (mon->data == &mons[PM_SILVER_DRAGON]
               || mon->data == &mons[PM_TIAMAT]) {
        /* Silver dragons only reflect when mature; babies do not */
        if (str)
            pline(str, s_suffix(mon_nam(mon)), "scales");
        return TRUE;
    } else if (has_reflection(mon)) {
        /* specifically for the reflection spell */
        if (str)
            pline(str, s_suffix(mon_nam(mon)), "shimmering globe");
        return TRUE;
    }
    return FALSE;
}

boolean
ureflects(const char *fmt, const char *str)
{
    /* Check from outermost to innermost objects */
    if (EReflecting & W_ARMS) {
        if (fmt && str) {
            pline(fmt, str, "shield");
            makeknown(SHIELD_OF_REFLECTION);
        }
        return TRUE;
    } else if (EReflecting & W_ARMG) {
        /* due to wearing the artifact Dragonbane */
        if (fmt && str)
            pline(fmt, str, "gloves");
        return TRUE;
    } else if (HReflecting) {
        /* reflection spell */
        if (fmt && str)
            pline(fmt, str, "magical shield");
        return TRUE;
    } else if (EReflecting & W_WEP) {
        /* Due to wielded artifact weapon */
        if (fmt && str)
            pline(fmt, str, "weapon");
        return TRUE;
    } else if (EReflecting & W_AMUL) {
        if (fmt && str) {
            pline(fmt, str, "medallion");
            makeknown(AMULET_OF_REFLECTION);
        }
        return TRUE;
    } else if (EReflecting & W_ARMC) {
        if (fmt && str)
            pline(fmt, str, "set of scales");
        return TRUE;
    } else if (EReflecting & W_ARM) {
        if (fmt && str)
            pline(fmt, str, uskin ? "luster" : "armor");
        return TRUE;
    } else if (EReflecting & W_ART) {
        /* due to the Magic Mirror, which shows as W_ART */
        if (fmt && str)
            pline(fmt, str, "mirror");
        return TRUE;
    } else if (gy.youmonst.data == &mons[PM_SILVER_DRAGON]) {
        if (fmt && str)
            pline(fmt, str, "scales");
        return TRUE;
    }
    return FALSE;
}

RESTORE_WARNING_FORMAT_NONLITERAL

/* cure mon's blindness (use_defensive, dog_eat, meatobj) */
void
mcureblindness(struct monst *mon, boolean verbos)
{
    if (!mon->mcansee) {
        mon->mcansee = 1;
        mon->mblinded = 0;
        if (verbos && haseyes(mon->data))
            pline_mon(mon, "%s can see again.", Monnam(mon));
    }
}

/* TRUE if the monster ate something */
boolean
munstone(struct monst *mon, boolean by_you)
{
    struct obj *obj;
    boolean tinok;

    if (resists_ston(mon) || defended(mon, AD_STON))
        return FALSE;
    if (mon->meating || helpless(mon))
        return FALSE;
    mon->mstrategy &= ~STRAT_WAITFORU;

    tinok = mcould_eat_tin(mon);
    for (obj = mon->minvent; obj; obj = obj->nobj) {
        if (cures_stoning(mon, obj, tinok)) {
            mon_consume_unstone(mon, obj, by_you, TRUE);
            return TRUE;
        }
    }
    return FALSE;
}

staticfn void
mon_consume_unstone(
    struct monst *mon,
    struct obj *obj,
    boolean by_you,
    boolean stoning) /* T: stop petrification, F: cure stun && confusion */
{
    boolean vis = canseemon(mon), tinned = obj->otyp == TIN,
            food = obj->otyp == CORPSE || tinned,
            acid = obj->otyp == POT_ACID
                   || (food && acidic(&mons[obj->corpsenm])),
            lizard = food && obj->corpsenm == PM_LIZARD,
            leaf = obj->otyp == EUCALYPTUS_LEAF;
    int nutrit = food ? dog_nutrition(mon, obj) : 0; /* also sets meating */

    /* give a "<mon> is slowing down" message and also remove
       intrinsic speed (comparable to similar effect on the hero) */
    if (stoning)
        mon_adjust_speed(mon, -3, (struct obj *) 0);

    if (vis) {
        long save_quan = obj->quan;

        obj->quan = 1L;
        pline_mon(mon, "%s %s %s.", Monnam(mon),
              ((obj->oclass == POTION_CLASS) ? "quaffs"
               : (obj->otyp == TIN) ? "opens and eats the contents of"
                 : "eats"),
              distant_name(obj, doname));
        obj->quan = save_quan;
    } else if (!Deaf)
        You_hear("%s.",
                 (obj->oclass == POTION_CLASS) ? "drinking" : "chewing");

    m_useup(mon, obj);
    /* obj is now gone */

    if (acid && !tinned && !(resists_acid(mon) || defended(mon, AD_ACID))) {
        damage_mon(mon, rnd(15), AD_ACID, FALSE);
        if (vis)
            pline_mon(mon, "%s has a very bad case of stomach acid.", Monnam(mon));
        if (DEADMONSTER(mon)) {
            pline_mon(mon, "%s dies!", Monnam(mon));
            if (by_you)
                /* hero gets credit (experience) and blame (possible loss
                   of alignment and/or luck and/or telepathy depending on
                   mon) for the kill but does not break pacifism conduct */
                xkilled(mon, XKILL_NOMSG | XKILL_NOCONDUCT);
            else
                mondead(mon);
            return;
        }
    }
    if (stoning && vis) {
        if (Hallucination)
            pline("What a pity - %s just ruined a future piece of art!",
                  mon_nam(mon));
        else
            pline_mon(mon, "%s seems limber!", Monnam(mon));
    }
    if (lizard && (mon->mconf || mon->mstun)) {
        mon->mconf = 0;
        mon->mstun = 0;
        if (vis && !is_bat(mon->data) && mon->data != &mons[PM_STALKER])
            pline_mon(mon, "%s seems steadier now.", Monnam(mon));
    }
    if (leaf && (mon->msick || mon->mdiseased)) {
        mon->msick = 0;
        mon->mdiseased = 0;
        if (vis)
            pline_mon(mon, "%s is no longer ill.", Monnam(mon));
    }
    if (mon->mtame && !mon->isminion && nutrit > 0) {
        struct edog *edog = EDOG(mon);

        if (edog->hungrytime < svm.moves)
            edog->hungrytime = svm.moves;
        edog->hungrytime += nutrit;
        mon->mconf = 0;
    }
    /* use up monster's next move */
    mon->movement -= NORMAL_SPEED;
    mon->mlstmv = svm.moves;
}

/* decide whether obj can cure petrification; also used when picking up */
boolean
cures_stoning(struct monst *mon, struct obj *obj, boolean tinok)
{
    if (obj->otyp == POT_ACID)
        return TRUE;
    if (obj->otyp == GLOB_OF_GREEN_SLIME)
        return (boolean) slimeproof(mon->data);
    if (obj->otyp != CORPSE && (obj->otyp != TIN || !tinok))
        return FALSE;
    /* corpse, or tin that mon can open */
    if (obj->corpsenm == NON_PM) /* empty/special tin */
        return FALSE;
    return (boolean) (obj->corpsenm == PM_LIZARD
                      || acidic(&mons[obj->corpsenm]));
}

staticfn boolean
mcould_eat_tin(struct monst *mon)
{
    struct obj *obj, *mwep;
    boolean welded_wep;

    /* monkeys who manage to steal tins can't open and eat them
       even if they happen to also have the appropriate tool */
    if (is_animal(mon->data))
        return FALSE;

    mwep = MON_WEP(mon);
    welded_wep = (mwep && mwelded(mwep)
                  && mon->data != &mons[PM_INFIDEL]);
    /* this is different from the player; tin opener or dagger doesn't
       have to be wielded, and knife can be used instead of dagger */
    for (obj = mon->minvent; obj; obj = obj->nobj) {
        /* if stuck with a cursed weapon, don't check rest of inventory */
        if (welded_wep && obj != mwep)
            continue;

        if (obj->otyp == TIN_OPENER
            || (obj->oclass == WEAPON_CLASS
                && objects[obj->otyp].oc_skill == P_DAGGER))
            return TRUE;
    }
    return FALSE;
}

/* TRUE if monster does something to avoid turning into green slime */
boolean
munslime(struct monst *mon, boolean by_you)
{
    struct obj *obj, odummy;
    struct permonst *mptr = mon->data;

    /*
     * muse_unslime() gives "mon starts turning green", "mon zaps
     * itself with a wand of fire", and "mon's slime burns away"
     * messages.  Monsters who don't get any chance at that just have
     * (via our caller) newcham()'s "mon turns into slime" feedback.
     */

    if (slimeproof(mptr))
        return FALSE;
    if (mon->meating || helpless(mon))
        return FALSE;
    mon->mstrategy &= ~STRAT_WAITFORU;

    /* if monster can breathe fire, do so upon self; a monster who deals
       fire damage by biting, clawing, gazing, and especially exploding
       isn't able to cure itself of green slime with its own attack
       [possible extension: monst capable of casting high level clerical
       spells could toss pillar of fire at self--probably too suicidal] */
    if (!mon->mcan && !mon->mbreath_attk
        && attacktype_fordmg(mptr, AT_BREA, AD_FIRE)) {
        odummy = cg.zeroobj; /* otyp == STRANGE_OBJECT */
        return muse_unslime(mon, &odummy, (struct trap *) 0, by_you);
    }

    /* same MUSE criteria as use_defensive() */
    if (!is_animal(mptr) && !mindless(mptr)) {
        struct trap *t;

        for (obj = mon->minvent; obj; obj = obj->nobj)
            if (cures_sliming(mon, obj))
                return muse_unslime(mon, obj, (struct trap *) 0, by_you);

        if (((t = t_at(mon->mx, mon->my)) == 0 || t->ttyp != FIRE_TRAP_SET)
            && mptr->mmove && !mon->mtrapped) {
            coordxy xy[2][8], x, y, idx, ridx, nxy = 0;

            for (x = mon->mx - 1; x <= mon->mx + 1; ++x)
                for (y = mon->my - 1; y <= mon->my + 1; ++y)
                    if (isok(x, y) && accessible(x, y)
                        && !m_at(x, y) && (x != u.ux || y != u.uy)) {
                        xy[0][nxy] = x, xy[1][nxy] = y;
                        ++nxy;
                    }
            for (idx = 0; idx < nxy; ++idx) {
                ridx = rn1(nxy - idx, idx);
                if (ridx != idx) {
                    x = xy[0][idx];
                    xy[0][idx] = xy[0][ridx];
                    xy[0][ridx] = x;
                    y = xy[1][idx];
                    xy[1][idx] = xy[1][ridx];
                    xy[1][ridx] = y;
                }
                if ((t = t_at(xy[0][idx], xy[1][idx])) != 0
                    && t->ttyp == FIRE_TRAP_SET)
                    break;
            }
        }
        if (t && t->ttyp == FIRE_TRAP_SET)
            return muse_unslime(mon, &hands_obj, t, by_you);

    } /* MUSE */

    return FALSE;
}

/* mon uses an item--selected by caller--to burn away incipient slime */
staticfn boolean
muse_unslime(
    struct monst *mon,
    struct obj *obj,
    struct trap *trap,
    boolean by_you) /* true: if mon kills itself, hero gets credit/blame */
{                   /* [by_you not honored if 'mon' triggers fire trap]. */
    struct obj *odummyp;
    struct monst *save_exploder;
    int otyp = obj->otyp, dmg = 0;
    boolean vis = canseemon(mon), res = TRUE;

    if (vis)
        pline_mon(mon, "%s starts turning %s.", Monnam(mon),
              green_mon(mon) ? "into ooze" : hcolor(NH_GREEN));
    /* -4 => sliming, causes quiet loss of enhanced speed */
    mon_adjust_speed(mon, -4, (struct obj *) 0);

    if (trap) {
        const char *Mnam = vis ? Monnam(mon) : 0;

        if (mon->mx == trap->tx && mon->my == trap->ty) {
            if (vis)
                pline("%s triggers %s fire trap!", Mnam,
                      trap->tseen ? "the" : "a");
        } else {
            remove_monster(mon->mx, mon->my);
            newsym(mon->mx, mon->my);
            place_monster(mon, trap->tx, trap->ty);
            if (mon->wormno) /* won't happen; worms don't MUSE to unslime */
                worm_move(mon);
            /* a rider drags its steed along */
            update_monsteed(mon);
            newsym(mon->mx, mon->my);
            if (vis)
                pline("%s %s %s %s fire trap!", Mnam,
                      vtense(fakename[0], locomotion(mon->data, "move")),
                      (is_floater(mon->data) || can_levitate(mon))
                          ? "over" : "onto",
                      trap->tseen ? "the" : "a");
        }
        (void) mintrap(mon, FORCETRAP);
    } else if (otyp == STRANGE_OBJECT) {
        /* monster is using fire breath on self */
        if (vis)
            pline_mon(mon, "%s.",
                      monverbself(mon, Monnam(mon), "breath", "fire on"));
        if (!rn2(3))
            mon->mbreath_attk = rn1(10, 5);
        /* -21 => monster's fire breath; 1 => # of damage dice */
        dmg = zhitm(mon, by_you ? 21 : -21, 1, &odummyp);
    } else if (otyp == SCR_FIRE) {
        mreadmsg(mon, obj);
        if (mon->mconf) {
            if (cansee(mon->mx, mon->my))
                pline("Oh, what a pretty fire!");
            if (vis)
                trycall(obj);
            m_useup(mon, obj); /* after trycall() */
            vis = FALSE;       /* skip makeknown() below */
            res = FALSE;       /* failed to cure sliming */
        } else {
            dmg = (2 * (rn1(3, 3) + 2 * bcsign(obj)) + 1) / 3;
            m_useup(mon, obj); /* before explode() */
            save_exploder = ge.exploder;
            ge.exploder = mon;
            /* BZ_M_SPELL(ZT_FIRE) => monster's fireball */
            explode(mon->mx, mon->my, BZ_M_SPELL(ZT_FIRE), dmg, SCROLL_CLASS,
                    /* by_you: override monster fireball for mon but
                       not others */
                    by_you ? -EXPL_FIERY : EXPL_FIERY);
            ge.exploder = save_exploder;
            dmg = 0; /* damage has been applied by explode() */
        }
    } else if (otyp == POT_OIL) {
        char Pronoun[40];
        boolean was_lit = obj->lamplit ? TRUE : FALSE, saw_lit = FALSE;
        /*
         * If not already lit, requires two actions.  We cheat and let
         * monster do both rather than render the potion unusable.
         *
         * Monsters don't start with oil and don't actively pick up oil
         * so this may never occur in a real game.  (Possible though;
         * nymph can steal potions of oil; shapechanger could take on
         * nymph form or vacuum up stuff as a gel.cube and then eventually
         * engage with a green slime.)
         */

        if (obj->quan > 1L)
            obj = splitobj(obj, 1L);
        if (vis && !was_lit) {
            pline_mon(mon, "%s ignites %s.", Monnam(mon), ansimpleoname(obj));
            saw_lit = TRUE;
        }
        begin_burn(obj, was_lit);
        vis |= canseemon(mon); /* burning potion may improve visibility */
        if (vis) {
            if (!Unaware)
                observe_object(obj); /* hero is watching mon drink obj */
            pline("%s quaffs a burning %s",
                  saw_lit ? upstart(strcpy(Pronoun, mhe(mon))) : Monnam(mon),
                  simpleonames(obj));
            makeknown(POT_OIL);
        }
        dmg = d(3, 4); /* [**TEMP** (different from hero)] */
        m_useup(mon, obj);
    } else { /* wand/horn of fire w/ positive charge count */
        if (obj->otyp == FIRE_HORN)
            mplayhorn(mon, obj, TRUE);
        else
            mzapwand(mon, obj, TRUE);
        /* -1 => monster's wand of fire; 2 => # of damage dice */
        dmg = zhitm(mon, by_you ? 1 : -1, 2, &odummyp);
    }

    if (dmg) {
        /* zhitm() applies damage but doesn't kill creature off;
           for fire breath, dmg is going to be 0 (fire breathers are
           immune to fire damage) but for wand of fire or fire horn,
           'mon' could have taken damage so might die */
        if (DEADMONSTER(mon)) {
            if (by_you) {
                /* mon killed self but hero gets credit and blame (except
                   for pacifist conduct); xkilled()'s message would say
                   "You killed/destroyed <mon>" so give our own message */
                if (vis)
                    pline_mon(mon, "%s is %s by the fire!", Monnam(mon),
                          nonliving(mon->data) ? "destroyed" : "killed");
                xkilled(mon, XKILL_NOMSG | XKILL_NOCONDUCT);
            } else
                monkilled(mon, "fire", AD_FIRE);
        } else {
            /* non-fatal damage occurred */
            if (vis)
                pline_mon(mon, "%s is burned%s", Monnam(mon), exclam(dmg));
        }
    }
    if (vis) {
        if (res && !DEADMONSTER(mon))
            pline_mon(mon, "%s slime is burned away!", s_suffix(Monnam(mon)));
        if (otyp != STRANGE_OBJECT)
            makeknown(otyp);
    }
    /* use up monster's next move */
    mon->movement -= NORMAL_SPEED;
    mon->mlstmv = svm.moves;
    return res;
}

/* decide whether obj can be used to cure green slime */
staticfn int
cures_sliming(struct monst *mon, struct obj *obj)
{
    /* scroll of fire */
    if (obj->otyp == SCR_FIRE)
        return (haseyes(mon->data) && mon->mcansee && !nohands(mon->data));

    /* potion of oil; will be set burning if not already */
    if (obj->otyp == POT_OIL)
        return !nohands(mon->data);

    /* non-empty wand or horn of fire;
       hero doesn't need hands or even limbs to zap, so mon doesn't either */
    return ((obj->otyp == WAN_FIRE
             || (obj->otyp == FIRE_HORN && can_blow(mon)))
            && obj->spe > 0);
}

/* TRUE if monster appears to be green; we go by the display color.
   The alternative was to just pick things that
   seem plausibly green (which didn't necessarily match the categorization
   by the color of the text).
   iflags.use_color is not meant for game behavior decisions */
staticfn boolean
green_mon(struct monst *mon)
{
    struct permonst *ptr = mon->data;

    if (Hallucination)
        return FALSE;
    return (ptr->mcolor == CLR_GREEN || ptr->mcolor == CLR_BRIGHT_GREEN);
#if 0
    if (iflags.use_color)
        return (ptr->mcolor == CLR_GREEN || ptr->mcolor == CLR_BRIGHT_GREEN);
    else {
        /* approximation */
        if (strstri(ptr->pmnames[NEUTRAL], "green")
            || (ptr->pmnames[MALE] && strstri(ptr->pmnames[MALE], "green"))
            || (ptr->pmnames[FEMALE]
                && strstri(ptr->pmnames[FEMALE], "green")))
            return TRUE;
        switch (monsndx(ptr)) {
        case PM_FOREST_CENTAUR:
        case PM_GARTER_SNAKE:
        case PM_GECKO:
        case PM_GREMLIN:
        case PM_HOMUNCULUS:
        case PM_JUIBLEX:
        case PM_LEPRECHAUN:
        case PM_LICHEN:
        case PM_LIZARD:
        case PM_WOOD_NYMPH:
            return TRUE;
        default:
            if (is_elf(ptr) && !is_prince(ptr) && !is_lord(ptr)
                && ptr != &mons[PM_GREY_ELF])
                return TRUE;
            break;
        }
    }
    return FALSE;
#endif
}

/*muse.c*/
