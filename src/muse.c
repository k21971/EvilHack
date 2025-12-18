/* NetHack 3.6	muse.c	$NHDT-Date: 1561053256 2019/06/20 17:54:16 $  $NHDT-Branch: NetHack-3.6 $:$NHDT-Revision: 1.97 $ */
/*      Copyright (C) 1990 by Ken Arromdee                         */
/* NetHack may be freely redistributed.  See license for details.  */

/*
 * Monster item usage routines.
 */

#include "hack.h"

boolean m_using = FALSE;

/* Let monsters use magic items.  Arbitrary assumptions: Monsters only use
 * scrolls when they can see, monsters know when wands have 0 charges,
 * monsters cannot recognize if items are cursed are not, monsters which
 * are confused don't know not to read scrolls, etc....
 */

STATIC_DCL int FDECL(precheck, (struct monst *, struct obj *));
STATIC_DCL void FDECL(mzapwand, (struct monst *, struct obj *, BOOLEAN_P));
STATIC_DCL void FDECL(mplayhorn, (struct monst *, struct obj *, BOOLEAN_P));
STATIC_DCL void FDECL(mbagmsg, (struct monst *, struct obj *));
STATIC_DCL void FDECL(mreadmsg, (struct monst *, struct obj *));
STATIC_DCL void FDECL(mquaffmsg, (struct monst *, struct obj *));
STATIC_DCL boolean FDECL(m_use_healing, (struct monst *));
STATIC_DCL boolean FDECL(linedup_chk_corpse, (int, int));
STATIC_DCL void FDECL(m_use_undead_turning, (struct monst *, struct obj *));
STATIC_PTR int FDECL(mbhitm, (struct monst *, struct obj *));
STATIC_DCL void FDECL(mbhit, (struct monst *, int,
                              int FDECL((*), (MONST_P, OBJ_P)),
                              int FDECL((*), (OBJ_P, OBJ_P)), struct obj *));
STATIC_DCL struct permonst *FDECL(muse_newcham_mon, (struct monst *));
void FDECL(you_aggravate, (struct monst *));
#if 0
STATIC_DCL boolean FDECL(necrophiliac, (struct obj *, BOOLEAN_P));
#endif
STATIC_DCL void FDECL(mon_consume_unstone, (struct monst *, struct obj *,
                                            BOOLEAN_P, BOOLEAN_P));
STATIC_DCL boolean FDECL(mcould_eat_tin, (struct monst *));
STATIC_DCL boolean FDECL(muse_unslime, (struct monst *, struct obj *,
                                        struct trap *, BOOLEAN_P));
STATIC_DCL int FDECL(cures_sliming, (struct monst *, struct obj *));
STATIC_DCL boolean FDECL(green_mon, (struct monst *));

STATIC_DCL int FDECL(charge_precedence, (int));
STATIC_DCL struct obj *FDECL(find_best_item_to_charge, (struct monst *));
STATIC_DCL boolean FDECL(find_offensive_recurse, (struct monst *, struct obj *,
                                                  struct monst *, BOOLEAN_P));
STATIC_DCL boolean FDECL(find_defensive_recurse, (struct monst *, struct obj *));
STATIC_DCL boolean FDECL(find_misc_recurse, (struct monst *, struct obj *));

static struct musable {
    struct obj *offensive;
    struct obj *defensive;
    struct obj *misc;
    struct obj *tocharge; /* TODO: remove tocharge at next version change */
    int has_offense, has_defense, has_misc;
    /* =0, no capability; otherwise, different numbers.
     * If it's an object, the object is also set (it's 0 otherwise).
     */
} m;
static int trapx, trapy;
static boolean zap_oseen; /* for wands which use mbhitm and are zapped at
                           * players.  We usually want an oseen local to
                           * the function, but this is impossible since the
                           * function mbhitm has to be compatible with the
                           * normal zap routines, and those routines don't
                           * remember who zapped the wand. */

/* Any preliminary checks which may result in the monster being unable to use
 * the item.  Returns 0 if nothing happened, 2 if the monster can't do
 * anything (i.e. it teleported) and 1 if it's dead.
 */
STATIC_OVL int
precheck(mon, obj)
struct monst *mon;
struct obj *obj;
{
    boolean vis;

    if (!obj)
        return 0;

    vis = cansee(mon->mx, mon->my);

    /* some of this code comes from mloot_container()
       this allows monsters to loot containers they
       are carrying */
    if (obj->where == OBJ_CONTAINED) {
        struct obj *container = obj->ocontainer;
        char contnr_nam[BUFSZ];
        boolean nearby;
        int howfar;

        /* prevent accessing locked containers, or
           containers that are empty or non-existent */
        if (!container || !Has_contents(container)
            || container->olocked) {
            m.has_defense = 0;
            m.has_offense = 0;
            m.has_misc = 0;
            return 0;
        }

        /* using a cursed bag of holding = bad */
        if (Is_mbag(container) && container->cursed) {
            m.has_defense = 0;
            m.has_offense = 0;
            m.has_misc = 0;
            return 0;
        }

        howfar = distu(mon->mx, mon->my);
        nearby = (howfar <= 7 * 7);
        contnr_nam[0] = '\0';

        container->cknown = 0; /* hero no longer knows container's contents
                                * even if [attempted] removal is observed */
        if (!*contnr_nam) {
            /* xname sets dknown, distant_name doesn't */
            Strcpy(contnr_nam, the(nearby ? xname(container)
                                          : distant_name(container, xname)));
        }

        /* remove obj from container */
        obj_extract_self(obj); /* reduces container's weight */
        container->owt = weight(container);

        if (vis) {
            if (!nearby) /* not close by */
                Norep("%s rummages through %s.", Monnam(mon), contnr_nam);
            else
                pline("%s removes %s from %s.", Monnam(mon),
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
        static const char *empty = "The potion turns out to be empty.";
        const char *potion_descr;
        struct monst *mtmp;

        potion_descr = OBJ_DESCR(objects[obj->otyp]);
        if (potion_descr && !strcmp(potion_descr, "milky")) {
            if (!(mvitals[PM_GHOST].mvflags & G_GONE)
                && !rn2(POTION_OCCUPANT_CHANCE(mvitals[PM_GHOST].born))) {
                if (!enexto(&cc, mon->mx, mon->my, &mons[PM_GHOST]))
                    return 0;
                mquaffmsg(mon, obj);
                m_useup(mon, obj);
                mtmp = makemon(&mons[PM_GHOST], cc.x, cc.y, NO_MM_FLAGS);
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
                        if (has_free_action(mon)) {
                            pline("%s stiffens momentarily.", Monnam(mon));
                        } else {
                            pline("%s is frightened to death, and unable to move.",
                                  Monnam(mon));
                        }
                    }
                    if (has_free_action(mon))
                        return 0;
                    paralyze_monst(mon, 3);
                }
                return 2;
            }
        }
        if (potion_descr && !strcmp(potion_descr, "smoky")
            && !(mvitals[PM_DJINNI].mvflags & G_GONE)
            && !rn2(POTION_OCCUPANT_CHANCE(mvitals[PM_DJINNI].born))) {
            if (!enexto(&cc, mon->mx, mon->my, &mons[PM_DJINNI]))
                return 0;
            mquaffmsg(mon, obj);
            m_useup(mon, obj);
            mtmp = makemon(&mons[PM_DJINNI], cc.x, cc.y, NO_MM_FLAGS);
            if (!mtmp) {
                if (vis)
                    pline1(empty);
            } else {
                if (vis)
                    pline("In a cloud of smoke, %s emerges!", a_monnam(mtmp));
                pline("%s speaks.", vis ? Monnam(mtmp) : Something);
                /* I suspect few players will be upset that monsters */
                /* CAN wish for wands of death here.... */
                if (rn2(2)) {
                    verbalize("Thank you for freeing me! In return, I will grant you a wish!");
                    mmake_wish(mon);
                    if (vis)
                        pline("%s vanishes.", Monnam(mtmp));
                    mongone(mtmp);
                } else if (rn2(2)) {
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
            pline("%s zaps %s, which suddenly explodes!", Monnam(mon),
                  an(xname(obj)));
        } else {
            /* same near/far threshold as mzapwand() */
            int range = couldsee(mon->mx, mon->my) /* 9 or 5 */
                           ? (BOLT_LIM + 1) : (BOLT_LIM - 3);

            You_hear("a zap and an explosion %s.",
                     (distu(mon->mx, mon->my) <= range * range)
                        ? "nearby" : "in the distance");
        }
        m_useup(mon, obj);
        damage_mon(mon, dam, AD_RBRE, FALSE);
        if (DEADMONSTER(mon)) {
            monkilled(mon, "", AD_RBRE);
            return 1;
        }
        m.has_defense = m.has_offense = m.has_misc = 0;
        /* Only one needed to be set to 0 but the others are harmless */
    }
    return 0;
}

/* when a monster zaps a wand give a message, deduct a charge, and if it
   isn't directly seen, remove hero's memory of the number of charges */
STATIC_OVL void
mzapwand(mtmp, otmp, self)
struct monst *mtmp;
struct obj *otmp;
boolean self;
{
    if (otmp->spe < 1) {
        impossible("Mon zapping wand with %d charges?", otmp->spe);
        return;
    }
    if (!canseemon(mtmp)) {
        int range = couldsee(mtmp->mx, mtmp->my) /* 9 or 5 */
                       ? (BOLT_LIM + 1) : (BOLT_LIM - 3);

        You_hear("a %s zap.", (distu(mtmp->mx, mtmp->my) <= range * range)
                                 ? "nearby" : "distant");
        otmp->known = 0;
    } else if (self) {
        pline("%s zaps %sself with %s!", Monnam(mtmp), mhim(mtmp),
              doname(otmp));
    } else {
        pline("%s zaps %s!", Monnam(mtmp), an(xname(otmp)));
        stop_occupation();
    }
    otmp->spe -= 1;
}

/* similar to mzapwand() but for magical horns */
STATIC_OVL void
mplayhorn(mtmp, otmp, self)
struct monst *mtmp;
struct obj *otmp;
boolean self;
{
    if (!canseemon(mtmp)) {
        int range = couldsee(mtmp->mx, mtmp->my) /* 9 or 5 */
                       ? (BOLT_LIM + 1) : (BOLT_LIM - 3);

        You_hear("a horn being played %s.",
                 (distu(mtmp->mx, mtmp->my) <= range * range)
                 ? "nearby" : "in the distance");
        otmp->known = 0; /* hero doesn't know how many charges are left */
    } else {
        otmp->dknown = 1;
        pline("%s plays a %s directed at %s!", Monnam(mtmp), xname(otmp),
              self ? mon_nam_too(mtmp, mtmp) : (char *) "you");
        makeknown(otmp->otyp); /* (wands handle this slightly differently) */
        if (!self)
            stop_occupation();
    }
    otmp->spe -= 1; /* use a charge */
}

STATIC_OVL void
mbagmsg(mtmp, otmp)
struct monst *mtmp;
struct obj *otmp;
{
    if (!canseemon(mtmp)) {
        You_hear("a bag being used.");
    } else {
        pline("%s uses %s!", Monnam(mtmp), an(xname(otmp)));
        stop_occupation();
    }
}

STATIC_OVL void
mreadmsg(mtmp, otmp)
struct monst *mtmp;
struct obj *otmp;
{
    boolean vismon = canseemon(mtmp);
    char onambuf[BUFSZ];
    short saverole;
    unsigned savebknown;

    if (!vismon && Deaf)
        return; /* no feedback */

    otmp->dknown = 1; /* seeing or hearing it read reveals its label */
    /* shouldn't be able to hear curse/bless status of unseen scrolls;
       for priest characters, bknown will always be set during naming */
    savebknown = otmp->bknown;
    saverole = Role_switch;
    if (!vismon) {
        otmp->bknown = 0;
        if (Role_if(PM_PRIEST))
            Role_switch = 0;
    }
    Strcpy(onambuf, singular(otmp, doname));
    Role_switch = saverole;
    otmp->bknown = savebknown;

    if (vismon)
        pline("%s reads %s!", Monnam(mtmp), onambuf);
    else
        You_hear("%s reading %s.",
                 x_monnam(mtmp, ARTICLE_A, (char *) 0,
                          (SUPPRESS_IT | SUPPRESS_INVISIBLE
                           | SUPPRESS_SADDLE | SUPPRESS_BARDING),
                           FALSE),
                 onambuf);

    if (mtmp->mconf)
        pline("Being confused, %s mispronounces the magic words...",
              vismon ? mon_nam(mtmp) : mhe(mtmp));
}

STATIC_OVL void
mquaffmsg(mtmp, otmp)
struct monst *mtmp;
struct obj *otmp;
{
    if (canseemon(mtmp)) {
        otmp->dknown = 1;
        pline("%s drinks %s!", Monnam(mtmp), singular(otmp, doname));
    } else if (!Deaf)
        You_hear("a chugging sound.");
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
#define MUSE_ACID_BLOB_CORPSE 20
#define MUSE_BAG_OF_TRICKS 21
#define MUSE_EUCALYPTUS_LEAF 22
#define MUSE_WAN_UNDEAD_TURNING 24 /* also an offensive item */
#define MUSE_POT_RESTORE_ABILITY 25
#define MUSE_POT_VAMPIRE_BLOOD 26
/*
#define MUSE_INNATE_TPT 9999
 * We cannot use this.  Since monsters get unlimited teleportation, if they
 * were allowed to teleport at will you could never catch them.  Instead,
 * assume they only teleport at random times, despite the inconsistency
 * that if you polymorph into one you teleport at will.
 */

STATIC_OVL boolean
m_use_healing(mtmp)
struct monst *mtmp;
{
    struct obj *obj = 0;
    if ((obj = m_carrying(mtmp, POT_FULL_HEALING)) != 0) {
        m.defensive = obj;
        m.has_defense = MUSE_POT_FULL_HEALING;
        return TRUE;
    }
    if ((obj = m_carrying(mtmp, POT_EXTRA_HEALING)) != 0) {
        m.defensive = obj;
        m.has_defense = MUSE_POT_EXTRA_HEALING;
        return TRUE;
    }
    if ((obj = m_carrying(mtmp, POT_HEALING)) != 0) {
        m.defensive = obj;
        m.has_defense = MUSE_POT_HEALING;
        return TRUE;
    }
    if (mtmp->msick || mtmp->mdiseased) {
        if ((obj = m_carrying(mtmp, EUCALYPTUS_LEAF)) != 0) {
            m.defensive = obj;
            m.has_defense = MUSE_EUCALYPTUS_LEAF;
            return TRUE;
        }
    }
    if (racial_vampire(mtmp)) {
        if ((obj = m_carrying(mtmp, POT_VAMPIRE_BLOOD)) != 0) {
            m.defensive = obj;
            m.has_defense = MUSE_POT_VAMPIRE_BLOOD;
            return TRUE;
        }
    }
    return FALSE;
}

/* Select a defensive item/action for a monster.  Returns TRUE iff one is
   found. */
boolean
find_defensive(mtmp)
struct monst *mtmp;
{
    struct obj *obj, *nextobj;
    struct trap *t;
    int fraction, x = mtmp->mx, y = mtmp->my;
    boolean stuck = (mtmp == u.ustuck),
            immobile = (mtmp->data->mmove == 0);

    m.defensive = (struct obj *) 0;
    m.has_defense = 0;

    if (is_animal(mtmp->data) || mindless(mtmp->data))
        return FALSE;
    if (dist2(x, y, mtmp->mux, mtmp->muy) > 25)
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
        if (obj || is_unicorn(mtmp->data)
            || mtmp->data == &mons[PM_KI_RIN]
            || mtmp->data == &mons[PM_ELDRITCH_KI_RIN]) {
            m.defensive = obj;
            m.has_defense = MUSE_UNICORN_HORN;
            return TRUE;
        }
    }

    if (mtmp->msick || mtmp->mdiseased) {
        for (obj = mtmp->minvent; obj; obj = obj->nobj) {
            if (!nohands(mtmp->data)) {
                if (obj && obj->otyp == POT_HEALING && !obj->cursed)
                    break;
                if (obj && obj->otyp == POT_FULL_HEALING) {
                    m.defensive = obj;
                    m.has_defense = MUSE_POT_FULL_HEALING;
                    return TRUE;
                } else if (obj && obj->otyp == POT_EXTRA_HEALING
                           && !obj->cursed) {
                    m.defensive = obj;
                    m.has_defense = MUSE_POT_EXTRA_HEALING;
                    return TRUE;
                } else if (obj && obj->otyp == POT_HEALING
                           && obj->blessed) {
                    m.defensive = obj;
                    m.has_defense = MUSE_POT_HEALING;
                    return TRUE;
                }
            }
            if (obj && obj->otyp == EUCALYPTUS_LEAF) {
                m.defensive = obj;
                m.has_defense = MUSE_EUCALYPTUS_LEAF;
                return TRUE;
            }
        }
    }

    if (mtmp->mconf || mtmp->mstun) {
        struct obj *liztin = 0;

        for (obj = mtmp->minvent; obj; obj = obj->nobj) {
            if (obj->otyp == CORPSE && obj->corpsenm == PM_LIZARD) {
                m.defensive = obj;
                m.has_defense = MUSE_LIZARD_CORPSE;
                return TRUE;
            } else if (obj->otyp == TIN && obj->corpsenm == PM_LIZARD) {
                liztin = obj;
            }
        }
        /* confused or stunned monster might not be able to open tin */
        if (liztin && mcould_eat_tin(mtmp) && rn2(3)) {
            m.defensive = liztin;
            /* tin and corpse ultimately end up being handled the same */
            m.has_defense = MUSE_LIZARD_CORPSE;
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
        && !nohands(mtmp->data) && mtmp->data != &mons[PM_PESTILENCE]) {
        if (m_use_healing(mtmp))
            return TRUE;
    }

    if (mtmp->mcan && !nohands(mtmp->data)) {
        /* TODO? maybe only monsters for whom cancellation actually matters
         * should bother fixing it -- for a monster without any abilities that
         * are affected by cancellation, why bother drinking the potion? */
        if ((obj = m_carrying(mtmp, POT_RESTORE_ABILITY)) != 0) {
            m.defensive = obj;
            m.has_defense = MUSE_POT_RESTORE_ABILITY;
            return TRUE;
        }
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
        for (obj = mtmp->minvent; obj; obj = obj->nobj) {
            if (obj->otyp == WAN_UNDEAD_TURNING && obj->spe > 0) {
                m.defensive = obj;
                m.has_defense = MUSE_WAN_UNDEAD_TURNING;
                return TRUE;
            }
        }
    }

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

    if (stuck || immobile || mtmp->mtrapped || mtmp->mentangled) {
        ; /* fleeing by stairs or traps is not possible */
    } else if (levl[x][y].typ == STAIRS) {
        if (x == xdnstair && y == ydnstair) {
            if (!(is_floater(mtmp->data) || can_levitate(mtmp)))
                m.has_defense = MUSE_DOWNSTAIRS;
        } else if (x == xupstair && y == yupstair) {
            m.has_defense = MUSE_UPSTAIRS;
        } else if (sstairs.sx && x == sstairs.sx && y == sstairs.sy) {
            if (sstairs.up || !(is_floater(mtmp->data) || can_levitate(mtmp)))
                m.has_defense = MUSE_SSTAIRS;
        }
    } else if (levl[x][y].typ == LADDER) {
        if (x == xupladder && y == yupladder) {
            m.has_defense = MUSE_UP_LADDER;
        } else if (x == xdnladder && y == ydnladder) {
            if (!(is_floater(mtmp->data) || can_levitate(mtmp)))
                m.has_defense = MUSE_DN_LADDER;
        } else if (sstairs.sx && x == sstairs.sx && y == sstairs.sy) {
            if (sstairs.up || !(is_floater(mtmp->data) || can_levitate(mtmp)))
                m.has_defense = MUSE_SSTAIRS;
        }
    } else {
        /* Note: trap doors take precedence over teleport traps. */
        int xx, yy, i, locs[10][2];
        boolean ignore_boulders = (r_verysmall(mtmp)
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
            if ((xx == u.ux && yy == u.uy)
                || (xx != x && yy != y && !diag_ok)
                || (level.monsters[xx][yy] && !(xx == x && yy == y)))
                continue;
            /* skip if there's no trap or can't/won't move onto trap */
            if ((t = t_at(xx, yy)) == 0
                || (!ignore_boulders && sobj_at(BOULDER, xx, yy))
                || onscary(xx, yy, mtmp)
                || has_erid(mtmp) || mtmp->ridden_by)
                continue;
            /* use trap if it's the correct type */
            if (is_hole(t->ttyp)
                && !is_floater(mtmp->data)
                && !can_levitate(mtmp)
                && !mtmp->isshk && !mtmp->isgd && !mtmp->ispriest
                && Can_fall_thru(&u.uz)) {
                trapx = xx;
                trapy = yy;
                m.has_defense = MUSE_TRAPDOOR;
                break; /* no need to look at any other spots */
            } else if (t->ttyp == TELEP_TRAP_SET) {
                trapx = xx;
                trapy = yy;
                m.has_defense = MUSE_TELEPORT_TRAP;
            }
        }
    }

    if (nohands(mtmp->data)) /* can't use objects */
        goto botm;

    if (is_mercenary(mtmp->data) && (obj = m_carrying(mtmp, BUGLE)) != 0) {
        int xx, yy;
        struct monst *mon;

        /* Distance is arbitrary.  What we really want to do is
         * have the soldier play the bugle when it sees or
         * remembers soldiers nearby...
         */
        for (xx = x - 3; xx <= x + 3; xx++) {
            for (yy = y - 3; yy <= y + 3; yy++) {
                if (!isok(xx, yy) || (xx == x && yy == y))
                    continue;
                if ((mon = m_at(xx, yy)) != 0 && is_mercenary(mon->data)
                    && mon->data != &mons[PM_GUARD]
                    && (mon->msleeping || !mon->mcanmove)) {
                    m.defensive = obj;
                    m.has_defense = MUSE_BUGLE;
                    goto toot; /* double break */
                }
            }
        }
 toot:
        ;
    }

    /* use immediate physical escape prior to attempting magic */
    if (m.has_defense) /* stairs, trap door or tele-trap, bugle alert */
        goto botm;

    /* kludge to cut down on trap destruction (particularly portals) */
    t = t_at(x, y);
    if (t && (is_pit(t->ttyp) || t->ttyp == WEB
              || t->ttyp == BEAR_TRAP))
        t = 0; /* ok for monster to dig here */

#define nomore(x)       if (m.has_defense == x) continue;
    /* selection could be improved by collecting all possibilities
       into an array and then picking one at random */
    for (obj = mtmp->minvent; obj; obj = nextobj) {
        nextobj = obj->nobj;
        /* don't always use the same selection pattern */
        if (m.has_defense && !rn2(3))
            break;

        /* nomore(MUSE_WAN_DIGGING); */
        if (m.has_defense == MUSE_WAN_DIGGING)
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
            && !(mtmp->data == &mons[PM_VLAD_THE_IMPALER]
                 && In_V_tower(&u.uz))) {
            m.defensive = obj;
            m.has_defense = MUSE_WAN_DIGGING;
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
            if (!level.flags.noteleport
                || !(mtmp->mtrapseen & (1 << (TELEP_TRAP_SET - 1)))) {
                m.defensive = obj;
                m.has_defense = (mon_has_amulet(mtmp))
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
            if (!level.flags.noteleport
                || !(mtmp->mtrapseen & (1 << (TELEP_TRAP_SET - 1)))) {
                m.defensive = obj;
                m.has_defense = MUSE_SCR_TELEPORTATION;
            }
        }

        if (mtmp->data != &mons[PM_PESTILENCE]) {
            nomore(MUSE_POT_FULL_HEALING);
            if (obj->otyp == POT_FULL_HEALING) {
                m.defensive = obj;
                m.has_defense = MUSE_POT_FULL_HEALING;
            }
            nomore(MUSE_POT_EXTRA_HEALING);
            if (obj->otyp == POT_EXTRA_HEALING) {
                m.defensive = obj;
                m.has_defense = MUSE_POT_EXTRA_HEALING;
            }
            nomore(MUSE_POT_HEALING);
            if (obj->otyp == POT_HEALING) {
                m.defensive = obj;
                m.has_defense = MUSE_POT_HEALING;
            }
            nomore(MUSE_POT_VAMPIRE_BLOOD);
            if (racial_vampire(mtmp)
                && obj->otyp == POT_VAMPIRE_BLOOD) {
                m.defensive = obj;
                m.has_defense = MUSE_POT_VAMPIRE_BLOOD;
            }
        } else { /* Pestilence */
            nomore(MUSE_POT_FULL_HEALING);
            if (obj->otyp == POT_SICKNESS) {
                m.defensive = obj;
                m.has_defense = MUSE_POT_FULL_HEALING;
            }
        }
        nomore(MUSE_BAG_OF_TRICKS);
        if (obj->otyp == BAG_OF_TRICKS && obj->spe > 0) {
            m.defensive = obj;
            m.has_defense = MUSE_BAG_OF_TRICKS;
        }
        nomore(MUSE_WAN_CREATE_MONSTER);
        if (obj->otyp == WAN_CREATE_MONSTER && obj->spe > 0) {
            m.defensive = obj;
            m.has_defense = MUSE_WAN_CREATE_MONSTER;
        }
        nomore(MUSE_SCR_CREATE_MONSTER);
        if (obj->otyp == SCR_CREATE_MONSTER) {
            m.defensive = obj;
            m.has_defense = MUSE_SCR_CREATE_MONSTER;
        }
    }

    return find_defensive_recurse(mtmp, mtmp->minvent);

 botm:
    return (boolean) !!m.has_defense;
#undef nomore
}

STATIC_OVL boolean
find_defensive_recurse(mtmp, start)
struct monst *mtmp;
struct obj *start;
{
    struct obj *obj = 0, *nextobj;
    int x = mtmp->mx, y = mtmp->my;
    boolean stuck = (mtmp == u.ustuck);

    struct trap *t = t_at(x, y);
    if (t && (t->ttyp == PIT || t->ttyp == SPIKED_PIT
        || t->ttyp == WEB || t->ttyp == BEAR_TRAP))
        t = 0; /* ok for monster to dig here */
#define nomore(x)       if (m.has_defense == x) continue;
    for (obj = start; obj; obj = nextobj) {
        nextobj = obj->nobj;
        /* don't always use the same selection pattern */
        if (m.has_defense && !rn2(3))
            break;

        if (Is_container(obj) && obj->otyp != BAG_OF_TRICKS) {
            (void) find_defensive_recurse(mtmp, obj->cobj);
            continue;
        }

        nomore(MUSE_WAN_DIGGING);
        if (m.has_defense == MUSE_WAN_DIGGING)
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
            && !(mtmp->data == &mons[PM_VLAD_THE_IMPALER]
                 && In_V_tower(&u.uz))) {
            m.defensive = obj;
            m.has_defense = MUSE_WAN_DIGGING;
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
            if (!level.flags.noteleport
                || !(mtmp->mtrapseen & (1 << (TELEP_TRAP_SET - 1)))) {
                m.defensive = obj;
                m.has_defense = (mon_has_amulet(mtmp))
                                 ? MUSE_WAN_TELEPORTATION
                                 : MUSE_WAN_TELEPORTATION_SELF;
            }
        }
        nomore(MUSE_SCR_TELEPORTATION);
        if (obj->otyp == SCR_TELEPORTATION && mtmp->mcansee
            && haseyes(mtmp->data)
            && (!obj->cursed
                || (!(mtmp->isshk && inhishop(mtmp))
                && !mtmp->isgd && !mtmp->ispriest))) {
            /* see WAN_TELEPORTATION case above */
            if (!level.flags.noteleport
                || !(mtmp->mtrapseen & (1 << (TELEP_TRAP_SET - 1)))) {
                m.defensive = obj;
                m.has_defense = MUSE_SCR_TELEPORTATION;
            }
        }

        if (mtmp->data != &mons[PM_PESTILENCE]) {
            nomore(MUSE_POT_FULL_HEALING);
            if (obj->otyp == POT_FULL_HEALING) {
                m.defensive = obj;
                m.has_defense = MUSE_POT_FULL_HEALING;
            }
            nomore(MUSE_POT_EXTRA_HEALING);
            if (obj->otyp == POT_EXTRA_HEALING) {
                m.defensive = obj;
                m.has_defense = MUSE_POT_EXTRA_HEALING;
            }
            nomore(MUSE_POT_HEALING);
            if (obj->otyp == POT_HEALING) {
                m.defensive = obj;
                m.has_defense = MUSE_POT_HEALING;
            }
        } else { /* Pestilence */
            nomore(MUSE_POT_FULL_HEALING);
            if (obj->otyp == POT_SICKNESS) {
                m.defensive = obj;
                m.has_defense = MUSE_POT_FULL_HEALING;
            }
        }
        nomore(MUSE_EUCALYPTUS_LEAF);
        if ((mtmp->msick || mtmp->mdiseased)
            && obj->otyp == EUCALYPTUS_LEAF) {
            m.defensive = obj;
            m.has_defense = MUSE_EUCALYPTUS_LEAF;
        }
        nomore(MUSE_POT_RESTORE_ABILITY);
        if (mtmp->mcan && obj->otyp == POT_RESTORE_ABILITY) {
            m.defensive = obj;
            m.has_defense = MUSE_POT_RESTORE_ABILITY;
        }
        nomore(MUSE_BAG_OF_TRICKS);
        if (obj->otyp == BAG_OF_TRICKS && obj->spe > 0) {
            m.defensive = obj;
            m.has_defense = MUSE_BAG_OF_TRICKS;
        }
        nomore(MUSE_WAN_CREATE_MONSTER);
        if (obj->otyp == WAN_CREATE_MONSTER && obj->spe > 0) {
            m.defensive = obj;
            m.has_defense = MUSE_WAN_CREATE_MONSTER;
        }
        nomore(MUSE_SCR_CREATE_MONSTER);
        if (obj->otyp == SCR_CREATE_MONSTER) {
            m.defensive = obj;
            m.has_defense = MUSE_SCR_CREATE_MONSTER;
        }
    }
    return(boolean) !!m.has_defense;
#undef nomore
}

/* Perform a defensive action for a monster.  Must be called immediately
 * after find_defensive().  Return values are 0: did something, 1: died,
 * 2: did something and can't attack again (i.e. teleported).
 */
int
use_defensive(mtmp)
struct monst *mtmp;
{
    int i, fleetim, how = 0;
    struct obj *otmp = m.defensive;
    boolean vis, vismon, oseen;
    const char *Mnam;

    /* Safety check: if the object isn't in the monster's inventory,
       something went wrong (e.g., it was selected in a previous turn) */
    if (otmp && otmp->where != OBJ_MINVENT) {
        /* Try to find by type */
        struct obj *o;
        int otyp = m.defensive ? m.defensive->otyp : STRANGE_OBJECT;
        otmp = NULL;

        if (otyp != STRANGE_OBJECT) {
            for (o = mtmp->minvent; o; o = o->nobj) {
                if (o->otyp == otyp) {
                    otmp = m.defensive = o;
                    break;
                }
            }
        }

        if (!otmp) {
            m.has_defense = 0;
            m.defensive = 0;
            return 0;
        }
    }

    if ((i = precheck(mtmp, otmp)) != 0)
        return i;

    /* After precheck, otmp might have been merged/freed if it was extracted
       from a container. We need to re-validate and potentially re-find it */
    if (otmp && otmp->where != OBJ_MINVENT) {
        /* Try to find by type */
        struct obj *o;
        int otyp = m.defensive ? m.defensive->otyp : STRANGE_OBJECT;
        otmp = NULL;

        if (otyp != STRANGE_OBJECT) {
            for (o = mtmp->minvent; o; o = o->nobj) {
                if (o->otyp == otyp) {
                    otmp = m.defensive = o;
                    break;
                }
            }
        }

        if (!otmp) {
            m.has_defense = 0;
            m.defensive = 0;
            return 0;
        }
    }

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

    switch (m.has_defense) {
    case MUSE_UNICORN_HORN:
        if (vismon) {
            if (otmp)
                pline("%s uses a unicorn horn!", Monnam(mtmp));
            else
                pline_The("tip of %s's horn glows!", mon_nam(mtmp));
        }
        if (!mtmp->mcansee) {
            mcureblindness(mtmp, vismon);
        } else if (mtmp->msick || mtmp->mdiseased) {
            mtmp->msick = mtmp->mdiseased = 0;
            if (vismon)
                pline("%s is no longer ill.", Monnam(mtmp));
        } else if (mtmp->mconf || mtmp->mstun) {
            mtmp->mconf = mtmp->mstun = 0;
            if (vismon)
                pline("%s seems steadier now.", Monnam(mtmp));
        } else
            impossible("No need for unicorn horn?");
        return 2;
    case MUSE_BUGLE:
        if (vismon)
            pline("%s plays %s!", Monnam(mtmp), doname(otmp));
        else if (!Deaf)
            You_hear("a bugle playing reveille!");
        awaken_soldiers(mtmp);
        return 2;
    case MUSE_WAN_TELEPORTATION_SELF:
        if ((mtmp->isshk && inhishop(mtmp)) || mtmp->isgd || mtmp->ispriest)
            return 2;
        m_flee(mtmp);
        mzapwand(mtmp, otmp, TRUE);
        how = WAN_TELEPORTATION;
 mon_tele:
        if (tele_restrict(mtmp)) { /* mysterious force... */
            if (vismon && how)     /* mentions 'teleport' */
                makeknown(how);
            /* monster learns that teleportation isn't useful here */
            if (level.flags.noteleport)
                mtmp->mtrapseen |= (1 << (TELEP_TRAP_SET - 1));
            return 2;
        }
        if (mon_has_amulet(mtmp) || On_W_tower_level(&u.uz)) {
            if (vismon)
                pline("%s seems disoriented for a moment.", Monnam(mtmp));
            return 2;
        }
        if (Iniceq && mtmp->data == &mons[PM_KATHRYN_THE_ICE_QUEEN]) {
            if (vismon) {
                pline("A powerful curse prevents %s from teleporting!",
                      mon_nam(mtmp));
                verbalize("Nooooo!");
            }
            return 2;
        }
        if (oseen && how)
            makeknown(how);
        (void) rloc(mtmp, TRUE);
        return 2;
    case MUSE_WAN_TELEPORTATION:
        zap_oseen = oseen;
        mzapwand(mtmp, otmp, FALSE);
        m_using = TRUE;
        mbhit(mtmp, rn1(8, 6), mbhitm, bhito, otmp);
        /* monster learns that teleportation isn't useful here */
        if (level.flags.noteleport)
            mtmp->mtrapseen |= (1 << (TELEP_TRAP_SET - 1));
        m_using = FALSE;
        return 2;
    case MUSE_SCR_TELEPORTATION: {
        int obj_is_cursed = otmp->cursed;

        if (mtmp->isshk || mtmp->isgd || mtmp->ispriest)
            return 2;
        m_flee(mtmp);
        mreadmsg(mtmp, otmp);
        m_useup(mtmp, otmp); /* otmp might be free'ed */
        how = SCR_TELEPORTATION;
        if (obj_is_cursed || mtmp->mconf) {
            int nlev;
            d_level flev;

            if (mon_has_amulet(mtmp) || In_endgame(&u.uz)
                || (Is_sanctum(&u.uz) && mtmp->data == &mons[PM_LUCIFER])) {
                if (vismon)
                    pline("%s seems very disoriented for a moment.",
                          Monnam(mtmp));
                return 2;
            }
            if (!decide_to_teleport(mtmp))
                nlev = random_teleport_level();
            else
                return 2;
            if (nlev == depth(&u.uz)) {
                if (vismon)
                    pline("%s shudders for a moment.", Monnam(mtmp));
                return 2;
            }
            if (Iniceq && mtmp->data == &mons[PM_KATHRYN_THE_ICE_QUEEN]) {
                if (vismon) {
                    pline("A powerful curse prevents %s from leaving this place!",
                          mon_nam(mtmp));
                    verbalize("Nooooo!");
                }
                return 2;
            }
            get_level(&flev, nlev);
            migrate_to_level(mtmp, ledger_no(&flev), MIGR_RANDOM,
                             (coord *) 0);
            if (oseen)
                makeknown(SCR_TELEPORTATION);
        } else
            goto mon_tele;
        return 2;
    }
    case MUSE_WAN_DIGGING: {
        struct trap *ttmp;

        m_flee(mtmp);
        mzapwand(mtmp, otmp, FALSE);
        if (oseen)
            makeknown(WAN_DIGGING);
        if (IS_FURNITURE(levl[mtmp->mx][mtmp->my].typ)
            || IS_DRAWBRIDGE(levl[mtmp->mx][mtmp->my].typ)
            || (is_drawbridge_wall(mtmp->mx, mtmp->my) >= 0)
            || (sstairs.sx && sstairs.sx == mtmp->mx
                && sstairs.sy == mtmp->my)) {
            pline_The("digging ray is ineffective.");
            return 2;
        }
        if (!Can_dig_down(&u.uz) && !levl[mtmp->mx][mtmp->my].candig) {
            if (canseemon(mtmp))
                pline_The("%s here is too hard to dig in.",
                          surface(mtmp->mx, mtmp->my));
            return 2;
        }
        ttmp = maketrap(mtmp->mx, mtmp->my, HOLE);
        if (!ttmp)
            return 2;
        seetrap(ttmp);
        if (vis) {
            pline("%s has made a hole in the %s.", Monnam(mtmp),
                  surface(mtmp->mx, mtmp->my));
            pline("%s %s through...", Monnam(mtmp),
                  is_flyer(mtmp->data) ? "dives" : "falls");
        } else if (!Deaf)
            You_hear("%s crash through the %s.", something,
                     surface(mtmp->mx, mtmp->my));
        /* we made sure that there is a level for mtmp to go to */
        migrate_to_level(mtmp, ledger_no(&u.uz) + 1, MIGR_RANDOM,
                         (coord *) 0);
        return 2;
    }
    case MUSE_WAN_UNDEAD_TURNING:
        zap_oseen = oseen;
        mzapwand(mtmp, otmp, FALSE);
        m_using = TRUE;
        mbhit(mtmp, rn1(8, 6), mbhitm, bhito, otmp);
        m_using = FALSE;
        return 2;
    case MUSE_BAG_OF_TRICKS: {
        coord cc;
        struct monst *mon;
        /* pm: 0 => random, eel => aquatic, croc => amphibious */
        struct permonst *pm = !is_pool(mtmp->mx, mtmp->my) ? 0
                            : &mons[u.uinwater ? PM_GIANT_EEL : PM_CROCODILE];

        if (!enexto(&cc, mtmp->mx, mtmp->my, pm))
            return 0;
        mbagmsg(mtmp, otmp);
        otmp->spe--;
        mon = makemon((struct permonst *) 0, cc.x, cc.y, NO_MM_FLAGS);
        if (mon && canspotmon(mon) && oseen)
            makeknown(BAG_OF_TRICKS);
        return 2;
    }
    case MUSE_WAN_CREATE_MONSTER: {
        coord cc;
        struct monst *mon;
        /* pm: 0 => random, eel => aquatic, croc => amphibious */
        struct permonst *pm = !is_pool(mtmp->mx, mtmp->my) ? 0
                            : &mons[u.uinwater ? PM_GIANT_EEL : PM_CROCODILE];

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
        else if (!objects[SCR_CREATE_MONSTER].oc_name_known
                 && !objects[SCR_CREATE_MONSTER].oc_uname)
            docall(otmp);
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
        if (vis) {
            struct trap *t = t_at(trapx, trapy);

            Mnam = Monnam(mtmp);
            pline("%s %s into a %s!", Mnam,
                  vtense(fakename[0], locomotion(mtmp->data, "jump")),
                  (t->ttyp == TRAPDOOR) ? "trap door" : "hole");
            if (levl[trapx][trapy].typ == SCORR) {
                levl[trapx][trapy].typ = CORR;
                unblock_point(trapx, trapy);
            }
            seetrap(t_at(trapx, trapy));
        }

        /*  don't use rloc_to() because worm tails must "move" */
        remove_monster(mtmp->mx, mtmp->my);
        newsym(mtmp->mx, mtmp->my); /* update old location */
        place_monster(mtmp, trapx, trapy);
        if (mtmp->wormno)
            worm_move(mtmp);
        newsym(trapx, trapy);

        migrate_to_level(mtmp, ledger_no(&u.uz) + 1, MIGR_RANDOM,
                         (coord *) 0);
        return 2;
    case MUSE_UPSTAIRS:
        m_flee(mtmp);
        if (ledger_no(&u.uz) == 1)
            goto escape; /* impossible; level 1 upstairs are SSTAIRS */
#if 0 /* disabling the mysterious force level teleportation for monsters
         while carrying the Amulet */

        if (Inhell && mon_has_amulet(mtmp) && !rn2(4)
            && (dunlev(&u.uz) < dunlevs_in_dungeon(&u.uz) - 3)) {
            if (vismon)
                pline(
    "As %s climbs the stairs, a mysterious force momentarily surrounds %s...",
                      mon_nam(mtmp), mhim(mtmp));
            /* simpler than for the player; this will usually be
               the Wizard and he'll immediately go right to the
               upstairs, so there's not much point in having any
               chance for a random position on the current level */
            migrate_to_level(mtmp, ledger_no(&u.uz) + 1, MIGR_RANDOM,
                             (coord *) 0);
        } else {
#endif
            if (vismon)
                pline("%s escapes upstairs!", Monnam(mtmp));
            migrate_to_level(mtmp, ledger_no(&u.uz) - 1, MIGR_STAIRS_DOWN,
                             (coord *) 0);
        return 2;
    case MUSE_DOWNSTAIRS:
        m_flee(mtmp);
        if (vismon)
            pline("%s escapes downstairs!", Monnam(mtmp));
        migrate_to_level(mtmp, ledger_no(&u.uz) + 1, MIGR_STAIRS_UP,
                         (coord *) 0);
        return 2;
    case MUSE_UP_LADDER:
        m_flee(mtmp);
        if (vismon)
            pline("%s escapes up the ladder!", Monnam(mtmp));
        migrate_to_level(mtmp, ledger_no(&u.uz) - 1, MIGR_LADDER_DOWN,
                         (coord *) 0);
        return 2;
    case MUSE_DN_LADDER:
        m_flee(mtmp);
        if (vismon)
            pline("%s escapes down the ladder!", Monnam(mtmp));
        migrate_to_level(mtmp, ledger_no(&u.uz) + 1, MIGR_LADDER_UP,
                         (coord *) 0);
        return 2;
    case MUSE_SSTAIRS:
        m_flee(mtmp);
        if (ledger_no(&u.uz) == 1) {
 escape:
            /* Monsters without the Amulet escape the dungeon and
             * are gone for good when they leave up the up stairs.
             * A monster with the Amulet would leave it behind
             * (mongone -> mdrop_special_objs) but we force any
             * monster who manages to acquire it or the invocation
             * tools to stick around instead of letting it escape.
             */
            if (mon_has_special(mtmp))
                return 0;
            if (vismon)
                pline("%s escapes the dungeon!", Monnam(mtmp));
            mongone(mtmp);
            return 2;
        }
        if (vismon)
            pline("%s escapes %sstairs!", Monnam(mtmp),
                  sstairs.up ? "up" : "down");
        /* going from the Valley to Castle (Stronghold) has no sstairs
           to target, but having sstairs.<sx,sy> == <0,0> will work the
           same as specifying MIGR_RANDOM when mon_arrive() eventually
           places the monster, so we can use MIGR_SSTAIRS unconditionally */
        migrate_to_level(mtmp, ledger_no(&sstairs.tolev), MIGR_SSTAIRS,
                         (coord *) 0);
        return 2;
    case MUSE_TELEPORT_TRAP:
        m_flee(mtmp);
        if (vis) {
            Mnam = Monnam(mtmp);
            pline("%s %s onto a teleport trap!", Mnam,
                  vtense(fakename[0], locomotion(mtmp->data, "jump")));
            seetrap(t_at(trapx, trapy));
        }
        /*  don't use rloc_to() because worm tails must "move" */
        remove_monster(mtmp->mx, mtmp->my);
        newsym(mtmp->mx, mtmp->my); /* update old location */
        place_monster(mtmp, trapx, trapy);
        if (mtmp->wormno)
            worm_move(mtmp);
        maybe_unhide_at(mtmp->mx, mtmp->my);
        newsym(trapx, trapy);

        goto mon_tele;
    case MUSE_POT_HEALING:
        mquaffmsg(mtmp, otmp);
        i = d(6 + 2 * bcsign(otmp), 4);
        mtmp->mhp += i;
        if (mtmp->mhp > mtmp->mhpmax)
            mtmp->mhp = ++mtmp->mhpmax;
        if (!otmp->cursed && !mtmp->mcansee) {
            mcureblindness(mtmp, vismon);
        } else if (otmp->blessed && (mtmp->msick || mtmp->mdiseased)) {
            if (vismon)
                pline("%s is no longer ill.", Monnam(mtmp));
            mtmp->msick = mtmp->mdiseased = 0;
        }
        if (vismon)
            pline("%s looks better.", Monnam(mtmp));
        if (oseen)
            makeknown(POT_HEALING);
        m_useup(mtmp, otmp);
        return 2;
    case MUSE_POT_EXTRA_HEALING:
        mquaffmsg(mtmp, otmp);
        i = d(6 + 2 * bcsign(otmp), 8);
        mtmp->mhp += i;
        if (mtmp->mhp > mtmp->mhpmax)
            mtmp->mhp = (mtmp->mhpmax += (otmp->blessed ? 5 : 2));
        if (!mtmp->mcansee) {
            mcureblindness(mtmp, vismon);
        } else if (!otmp->cursed && (mtmp->msick || mtmp->mdiseased)) {
            if (vismon)
                pline("%s is no longer ill.", Monnam(mtmp));
            mtmp->msick = mtmp->mdiseased = 0;
        }
        if (vismon)
            pline("%s looks much better.", Monnam(mtmp));
        if (oseen)
            makeknown(POT_EXTRA_HEALING);
        m_useup(mtmp, otmp);
        return 2;
    case MUSE_POT_FULL_HEALING:
        mquaffmsg(mtmp, otmp);
        if (otmp->otyp == POT_SICKNESS)
            unbless(otmp); /* Pestilence */
        mtmp->mhp = (mtmp->mhpmax += (otmp->blessed ? 8 : 4));
        if (!mtmp->mcansee && otmp->otyp != POT_SICKNESS) {
            mcureblindness(mtmp, vismon);
        } else if (mtmp->msick || mtmp->mdiseased) {
            if (vismon)
                pline("%s is no longer ill.", Monnam(mtmp));
            mtmp->msick = mtmp->mdiseased = 0;
        }
        if (vismon)
            pline("%s looks completely healed.", Monnam(mtmp));
        if (oseen)
            makeknown(otmp->otyp);
        m_useup(mtmp, otmp);
        return 2;
    case MUSE_POT_VAMPIRE_BLOOD:
        mquaffmsg(mtmp, otmp);
        if (otmp->blessed) {
            /* acts mostly like a potion of full healing */
            if (otmp->odiluted)
                mtmp->mhp += (mtmp->mhpmax / 4);
            else
                mtmp->mhp = mtmp->mhpmax;
            if (vismon)
                pline("%s looks completely healed.", Monnam(mtmp));
        } else if (otmp->cursed) {
            if (vismon)
                pline("%s discards the congealed blood in disgust.",
                      Monnam(mtmp));
        } else { /* uncursed, acts mostly like a potion of healing */
            i = d(4, 4) / (otmp->odiluted ? 4 : 1);
            mtmp->mhp += i;
            if (mtmp->mhp > mtmp->mhpmax)
                mtmp->mhp = ++mtmp->mhpmax;
            if (vismon)
                pline("%s looks better.", Monnam(mtmp));
        }
        if (oseen)
            makeknown(POT_VAMPIRE_BLOOD);
        m_useup(mtmp, otmp);
        return 2;
    case MUSE_LIZARD_CORPSE:
        mon_consume_unstone(mtmp, otmp, FALSE, mtmp->mstone ? TRUE : FALSE);
        return 2;
    case MUSE_ACID_BLOB_CORPSE:
        mon_consume_unstone(mtmp, otmp, FALSE, mtmp->mstone ? TRUE : FALSE);
        return 2;
    case MUSE_EUCALYPTUS_LEAF:
        mon_consume_unstone(mtmp, otmp, FALSE, FALSE);
        return 2;
    case MUSE_POT_RESTORE_ABILITY:
        mquaffmsg(mtmp, otmp);
        mtmp->mcan = 0;
        if (canseemon(mtmp))
            pline("%s looks revitalized.", Monnam(mtmp));
        if (oseen)
            makeknown(otmp->otyp);
        m_useup(mtmp, otmp);
        return 2;
    case 0:
        return 0; /* i.e. an exploded wand */
    default:
        impossible("%s wanted to perform action %d?", Monnam(mtmp),
                   m.has_defense);
        break;
    }
    return 0;
#undef m_flee
}

int
rnd_defensive_item(mtmp)
struct monst *mtmp;
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
        if (level.flags.noteleport && ++trycnt < 2)
            goto try_again;
        if (!rn2(3))
            return WAN_TELEPORTATION;
        /*FALLTHRU*/
    case 0:
    case 1:
        return SCR_TELEPORTATION;
    case 8:
    case 10:
        if (!rn2(3))
            return WAN_CREATE_MONSTER;
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
    case 7:
        if (is_floater(pm) || mtmp->isshk || mtmp->isgd || mtmp->ispriest)
            return 0;
        else
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
#define MUSE_WAN_CANCELLATION 18
#define MUSE_SCR_CHARGING 19
#define MUSE_SCR_STINKING_CLOUD 20
#define MUSE_POT_POLYMORPH_THROW 21
#define MUSE_POT_HALLUCINATION 22
#define MUSE_WAN_POLYMORPH 23
/*#define MUSE_WAN_UNDEAD_TURNING 24*/ /* also a defensive item so don't
                                        * redefine; nonconsecutive value is ok */
#define MUSE_POT_OIL 25
#define MUSE_CAMERA 26
#define MUSE_WAN_SLOW_MONSTER 27

static boolean
linedup_chk_corpse(x, y)
int x, y;
{
    return (sobj_at(CORPSE, x, y) != 0);
}

static void
m_use_undead_turning(mtmp, obj)
struct monst *mtmp;
struct obj *obj;
{
    int ax = u.ux + sgn(mtmp->mux - mtmp->mx) * 3,
        ay = u.uy + sgn(mtmp->muy - mtmp->my) * 3;
    int bx = mtmp->mx, by = mtmp->my;

    if (!(obj->otyp == WAN_UNDEAD_TURNING && obj->spe > 0))
        return;

    /* not necrophiliac(); unlike deciding whether to pick this
       type of wand up, we aren't interested in corpses within
       carried containers until they're moved into open inventory;
       we don't check whether hero is poly'd into an undead--the
       wand's turning effect is too weak to be a useful direct
       attack--only whether hero is carrying at least one corpse */
    if (carrying(CORPSE)) {
        /*
         * Hero is carrying one or more corpses but isn't wielding
         * a cockatrice corpse (unless being hit by one won't do
         * the monster much harm); otherwise we'd be using this wand
         * as a defensive item with higher priority.
         *
         * Might be cockatrice intended as a weapon (or being denied
         * to glove-wearing monsters for use as a weapon) or lizard
         * intended as a cure or lichen intended as veggy food or
         * sacrifice fodder being lugged to an altar.  Zapping with
         * this will deprive hero of one from each stack although
         * they might subsequently be recovered after killing again.
         * In the sacrifice fodder case, it could even be to the
         * player's advantage (fresher corpse if a new one gets
         * dropped; player might not choose to spend a wand charge
         * on that when/if hero acquires this wand).
         */
        m.offensive = obj;
        m.has_offense = MUSE_WAN_UNDEAD_TURNING;
    } else if (linedup_callback(ax, ay, bx, by, linedup_chk_corpse)) {
        /* There's a corpse on the ground in a direct line from the
         * monster to the hero, and up to 3 steps beyond.
         */
        m.offensive = obj;
        m.has_offense = MUSE_WAN_UNDEAD_TURNING;
    } else if (Race_if(PM_DRAUGR) || Race_if(PM_VAMPIRE)
               || is_undead(youmonst.data)) {
        /* player is Draugr/Vampire race or poly'd into an undead
           monster */
        m.offensive = obj;
        m.has_offense = MUSE_WAN_UNDEAD_TURNING;
    }
}

/* Select an offensive item/action for a monster.  Returns TRUE iff one is
 * found.
 */
boolean
find_offensive(mtmp)
struct monst *mtmp;
{
    struct monst *target = mfind_target(mtmp);
    boolean reflection_skip = FALSE;

    if (target) {
        if (target == &youmonst)
            reflection_skip = (m_seenres(mtmp, M_SEEN_REFL) != 0
                               || (monnear(mtmp, mtmp->mux, mtmp->muy)
                                   && !rn2(3)));
    } else {
        return FALSE; /* nothing to attack */
    }

    m.offensive = (struct obj *) 0;
    m.tocharge = (struct obj *) 0; /* TODO: remove at next version change */
    m.has_offense = 0;
    if (mtmp->mpeaceful || is_animal(mtmp->data)
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
    /* all offensive items require orthogonal or diagonal targetting */
    if (!lined_up(mtmp))
        return FALSE;

    return find_offensive_recurse(mtmp, mtmp->minvent, target,
                                  reflection_skip);
}

STATIC_OVL int
charge_precedence(otyp)
int otyp;
{
    int want = 0;
    switch (otyp) {
    /* in order of priority, highest being on top */
    case WAN_WISHING:
        want++;
        /*FALLTHRU*/
    case WAN_DEATH:
        want++;
        /*FALLTHRU*/
    case WAN_SLEEP:
        want++;
        /*FALLTHRU*/
    case WAN_FIRE:
        want++;
        /*FALLTHRU*/
    case FIRE_HORN:
        want++;
        /*FALLTHRU*/
    case WAN_COLD:
        want++;
        /*FALLTHRU*/
    case FROST_HORN:
        want++;
        /*FALLTHRU*/
    case WAN_LIGHTNING:
        want++;
        /*FALLTHRU*/
    case WAN_MAGIC_MISSILE:
        want++;
        /*FALLTHRU*/
    case WAN_CANCELLATION:
        want++;
        /*FALLTHRU*/
    case WAN_POLYMORPH:
        want++;
        /*FALLTHRU*/
    case WAN_STRIKING:
        want++;
        /*FALLTHRU*/
    case WAN_TELEPORTATION:
        want++;
        /*FALLTHRU*/
    case WAN_SLOW_MONSTER:
        want++;
    }

    return want;
}

/* Find the best chargeable item for a monster to charge with a scroll.
   Called at the moment of use, not during selection, to avoid pointer
   corruption issues. Returns NULL if nothing suitable to charge */
STATIC_OVL struct obj *
find_best_item_to_charge(mtmp)
struct monst *mtmp;
{
    struct obj *obj, *best = (struct obj *) 0;
    int best_priority = -1;

    for (obj = mtmp->minvent; obj; obj = obj->nobj) {
        /* Only consider items that:
           1) Can be recharged by scroll of charging
           2) Monsters actually have interest in using (have MUSE_ defines) */
        boolean rechargeable = FALSE;

        if (obj->oclass == WAND_CLASS) {
            /* Only wands monsters actually use */
            switch (obj->otyp) {
            case WAN_DEATH:
            case WAN_SLEEP:
            case WAN_FIRE:
            case WAN_COLD:
            case WAN_LIGHTNING:
            case WAN_MAGIC_MISSILE:
            case WAN_STRIKING:
            case WAN_TELEPORTATION:
            case WAN_CANCELLATION:
            case WAN_POLYMORPH:
            case WAN_SLOW_MONSTER:
            case WAN_DIGGING:
            case WAN_CREATE_MONSTER:
            case WAN_UNDEAD_TURNING:
            case WAN_MAKE_INVISIBLE:
            case WAN_SPEED_MONSTER:
            case WAN_WISHING:
                rechargeable = TRUE;
                break;
            default:
                rechargeable = FALSE;
                break;
            }
        } else if (obj->oclass == TOOL_CLASS) {
            /* Only tools monsters actually use */
            switch (obj->otyp) {
            case FROST_HORN:
            case FIRE_HORN:
            case EXPENSIVE_CAMERA:
                rechargeable = TRUE;
                break;
            default:
                rechargeable = FALSE;
                break;
            }
        }

        if (!rechargeable)
            continue;

        /* Skip if already has charges (we want empty items) */
        if (obj->spe > 0)
            continue;

        /* Must be carried directly by monster */
        if (!mcarried(obj))
            continue;

        /* Get priority of this item */
        int priority = charge_precedence(obj->otyp);

        if (priority > best_priority) {
            best = obj;
            best_priority = priority;
        }
    }

    return best;
}

STATIC_OVL boolean
find_offensive_recurse(mtmp, start, target, reflection_skip)
struct monst *mtmp;
struct obj *start;
struct monst *target;
boolean reflection_skip;
{
    struct obj *obj, *nextobj;
    struct obj *helmet = which_armor(mtmp, W_ARMH);

#define nomore(x)       if (m.has_offense == x) continue;
    /* this picks the last viable item rather than prioritizing choices */
    for (obj = start; obj; obj = nextobj) {
        nextobj = obj->nobj;
        if (Is_container(obj)) {
            (void) find_offensive_recurse(mtmp, obj->cobj, target,
                                          reflection_skip);
            continue;
        }

        if (!reflection_skip) {
            nomore(MUSE_WAN_DEATH);
            if (obj->otyp == WAN_DEATH) {
                if (obj->spe > 0
                    && !(m_seenres(mtmp, M_SEEN_MAGR)
                         || m_seenres(mtmp, M_SEEN_DEATH))
                    && (!m_seenres(mtmp, M_SEEN_REFL)
                        || nonliving(mtmp->data)
                        || mtmp->data->msound == MS_LEADER)) {
                    m.offensive = obj;
                    m.has_offense = MUSE_WAN_DEATH;
                }
                continue;
            }
            nomore(MUSE_WAN_SLEEP);
            if (obj->otyp == WAN_SLEEP && multi >= 0) {
                if (obj->spe > 0 && !m_seenres(mtmp, M_SEEN_SLEEP)) {
                    m.offensive = obj;
                    m.has_offense = MUSE_WAN_SLEEP;
                }
                continue;
            }
            nomore(MUSE_WAN_FIRE);
            if (obj->otyp == WAN_FIRE) {
                if (obj->spe > 0 && !m_seenres(mtmp, M_SEEN_FIRE)) {
                    m.offensive = obj;
                    m.has_offense = MUSE_WAN_FIRE;
                }
                continue;
            }
            nomore(MUSE_FIRE_HORN);
            if (obj->otyp == FIRE_HORN && can_blow(mtmp)) {
                if (obj->spe > 0 && !m_seenres(mtmp, M_SEEN_FIRE)) {
                    m.offensive = obj;
                    m.has_offense = MUSE_FIRE_HORN;
                }
                continue;
            }
            nomore(MUSE_WAN_COLD);
            if (obj->otyp == WAN_COLD) {
                if (obj->spe > 0 && !m_seenres(mtmp, M_SEEN_COLD)) {
                    m.offensive = obj;
                    m.has_offense = MUSE_WAN_COLD;
                }
                continue;
            }
            nomore(MUSE_FROST_HORN);
            if (obj->otyp == FROST_HORN && can_blow(mtmp)) {
                if (obj->spe > 0 && !m_seenres(mtmp, M_SEEN_COLD)) {
                    m.offensive = obj;
                    m.has_offense = MUSE_FROST_HORN;
                }
                continue;
            }
            nomore(MUSE_WAN_LIGHTNING);
            if (obj->otyp == WAN_LIGHTNING) {
                if (obj->spe > 0 && !m_seenres(mtmp, M_SEEN_ELEC)) {
                    m.offensive = obj;
                    m.has_offense = MUSE_WAN_LIGHTNING;
                }
                continue;
            }
            nomore(MUSE_WAN_MAGIC_MISSILE);
            if (obj->otyp == WAN_MAGIC_MISSILE) {
                if (obj->spe > 0) {
                    m.offensive = obj;
                    m.has_offense = MUSE_WAN_MAGIC_MISSILE;
                }
                continue;
            }
        }

        nomore(MUSE_WAN_UNDEAD_TURNING);
        m_use_undead_turning(mtmp, obj);

        nomore(MUSE_WAN_CANCELLATION);
        if (obj->otyp == WAN_CANCELLATION) {
            if (obj->spe > 0) {
                m.offensive = obj;
                m.has_offense = MUSE_WAN_CANCELLATION;
            }
            continue;
        }
        nomore(MUSE_WAN_POLYMORPH);
        if (obj->otyp == WAN_POLYMORPH) {
            if (obj->spe > 0 && !m_seenres(mtmp, M_SEEN_MAGR)) {
                m.offensive = obj;
                m.has_offense = MUSE_WAN_POLYMORPH;
            }
            continue;
        }
        nomore(MUSE_WAN_STRIKING);
        if (obj->otyp == WAN_STRIKING) {
            if (obj->spe > 0 && !m_seenres(mtmp, M_SEEN_MAGR)) {
                m.offensive = obj;
                m.has_offense = MUSE_WAN_STRIKING;
            }
            continue;
        }
        /* use_offensive() has had some code to support wand of teleportation
         * for a long time, but find_offensive() never selected one;
         * re-enable it */
        nomore(MUSE_WAN_TELEPORTATION);
        if (obj->otyp == WAN_TELEPORTATION) {
            if (obj->spe > 0
                /* don't give controlled hero a free teleport */
                && !Teleport_control
                /* same hack as MUSE_WAN_TELEPORTATION_SELF */
                && (!level.flags.noteleport
                    || !(mtmp->mtrapseen & (1 << (TELEP_TRAP_SET - 1))))
                /* do try to move hero to a more vulnerable spot */
                && (onscary(u.ux, u.uy, mtmp)
                    || (u.ux == sstairs.sx && u.uy == sstairs.sy))) {
                m.offensive = obj;
                m.has_offense = MUSE_WAN_TELEPORTATION;
            }
            continue;
        }
        nomore(MUSE_WAN_SLOW_MONSTER);
        /* don't bother recharging this one */
        if (obj->otyp == WAN_SLOW_MONSTER) {
            if (obj->spe > 0 && !Slow) {
                m.offensive = obj;
                m.has_offense = MUSE_WAN_SLOW_MONSTER;
            }
            continue;
        }
        if (m.has_offense == MUSE_SCR_CHARGING)
            continue;
        if (obj->otyp == SCR_CHARGING) {
            /* Check if monster has anything to charge right now */
            if (find_best_item_to_charge(mtmp)) {
                m.offensive = obj;
                m.has_offense = MUSE_SCR_CHARGING;
            }
            continue;
        }
        nomore(MUSE_SCR_STINKING_CLOUD)
        if (obj->otyp == SCR_STINKING_CLOUD && m_canseeu(mtmp)
            && distu(mtmp->mx, mtmp->my) < 32
            && !m_seenres(mtmp, M_SEEN_POISON)) {
            m.offensive = obj;
            m.has_offense = MUSE_SCR_STINKING_CLOUD;
        }
        nomore(MUSE_POT_HALLUCINATION);
        if (obj->otyp == POT_HALLUCINATION && multi >= 0) {
            m.offensive = obj;
            m.has_offense = MUSE_POT_HALLUCINATION;
        }
        nomore(MUSE_POT_POLYMORPH_THROW);
        if (obj->otyp == POT_POLYMORPH
            && !m_seenres(mtmp, M_SEEN_MAGR)) {
            m.offensive = obj;
            m.has_offense = MUSE_POT_POLYMORPH_THROW;
        }
        nomore(MUSE_POT_PARALYSIS);
        if (obj->otyp == POT_PARALYSIS && multi >= 0) {
            m.offensive = obj;
            m.has_offense = MUSE_POT_PARALYSIS;
        }
        nomore(MUSE_POT_BLINDNESS);
        if (obj->otyp == POT_BLINDNESS && !attacktype(mtmp->data, AT_GAZE)) {
            m.offensive = obj;
            m.has_offense = MUSE_POT_BLINDNESS;
        }
        nomore(MUSE_POT_CONFUSION);
        if (obj->otyp == POT_CONFUSION) {
            m.offensive = obj;
            m.has_offense = MUSE_POT_CONFUSION;
        }
        nomore(MUSE_POT_SLEEPING);
        if (obj->otyp == POT_SLEEPING
            && !m_seenres(mtmp, M_SEEN_SLEEP)) {
            m.offensive = obj;
            m.has_offense = MUSE_POT_SLEEPING;
        }
        nomore(MUSE_POT_ACID);
        if (obj->otyp == POT_ACID
            && !m_seenres(mtmp, M_SEEN_ACID)) {
            m.offensive = obj;
            m.has_offense = MUSE_POT_ACID;
        }
        nomore(MUSE_POT_OIL);
        if (obj->otyp == POT_OIL
            && !m_seenres(mtmp, M_SEEN_FIRE)
            /* don't throw oil if point-blank AND mtmp is low on HP AND mtmp is
             * not fire resistant */
            && (dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) > 2
                || mtmp->mhp > 10
                || resists_fire(mtmp)
                || defended(mtmp, AD_FIRE))) {
            m.offensive = obj;
            m.has_offense = MUSE_POT_OIL;
        }
        /* we can safely put this scroll here since the locations that
         * are in a 1 square radius are a subset of the locations that
         * are in wand range
         */
        nomore(MUSE_SCR_EARTH);
        if (obj->otyp == SCR_EARTH
            && ((helmet && is_hard(helmet))
                || mtmp->mconf || amorphous(mtmp->data)
                || passes_walls(mtmp->data)
                || noncorporeal(mtmp->data)
                || unsolid(mtmp->data) || !rn2(10))
                && dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) <= 2
                && mtmp->mcansee && haseyes(mtmp->data)
                && !Is_rogue_level(&u.uz)
                && (!In_endgame(&u.uz) || Is_earthlevel(&u.uz))) {
            m.offensive = obj;
            m.has_offense = MUSE_SCR_EARTH;
        }
        nomore(MUSE_SCR_FIRE);
        if (obj->otyp == SCR_FIRE
            && resists_fire(mtmp) && defended(mtmp, AD_FIRE)
            && dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) <= 2
            && mtmp->mcansee && haseyes(mtmp->data)
            && !m_seenres(mtmp, M_SEEN_FIRE)) {
            m.offensive = obj;
            m.has_offense = MUSE_SCR_FIRE;
        }
        nomore(MUSE_CAMERA);
        if (obj->otyp == EXPENSIVE_CAMERA
            && (!Blind
                || hates_light(youmonst.data)
                || maybe_polyd(is_drow(youmonst.data),
                                       Race_if(PM_DROW)))
            && dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) <= 2
            && obj->spe > 0 && !rn2(6)) {
            m.offensive = obj;
            m.has_offense = MUSE_CAMERA;
        }
    }
    return (boolean) !!m.has_offense;
#undef nomore
}

extern struct monst *last_hurtled;

STATIC_PTR
int
mbhitm(mtmp, otmp)
struct monst *mtmp;
struct obj *otmp;
{
    int tmp;
    boolean reveal_invis = FALSE, hits_you = (mtmp == &youmonst);
    boolean mon_harbinger_wield = (MON_WEP(mtmp)
                                   && MON_WEP(mtmp)->oartifact == ART_HARBINGER);
    boolean mon_giantslayer_wield = (MON_WEP(mtmp)
                                     && MON_WEP(mtmp)->oartifact == ART_GIANTSLAYER);

    if (!hits_you && otmp->otyp != WAN_UNDEAD_TURNING) {
        mtmp->msleeping = 0;
        if (mtmp->m_ap_type)
            seemimic(mtmp);
    }
    switch (otmp->otyp) {
    case WAN_STRIKING:
    case SPE_FORCE_BOLT:
        reveal_invis = TRUE;
        if (last_hurtled && mtmp == last_hurtled) {
            ; /* do nothing */
        } else if (hits_you) {
            if (zap_oseen && otmp->otyp == WAN_STRIKING)
                makeknown(WAN_STRIKING);
            if (Antimagic) {
                shieldeff(u.ux, u.uy);
                monstseesu(M_SEEN_MAGR);
                pline("Boing!");
            } else if (rnd(20) < 10 + u.uac) {
                if (otmp->otyp == WAN_STRIKING)
                    pline_The("wand hits you!");
                else
                    pline_The("force bolt hits you!");
                tmp = d(2, (otmp->otyp == WAN_STRIKING) ? 12 : 6);
                /* Force bolt damage scales with caster level */
                if (otmp->otyp == SPE_FORCE_BOLT && mcarried(otmp))
                    tmp += otmp->ocarry->m_lev / 3;
                if (Half_spell_damage)
                    tmp = (tmp + 1) / 2;
                /* Knockback threshold: 16 for wand, 12 for spell */
                if (tmp > (otmp->otyp == WAN_STRIKING ? 16 : 12)
                    && mcarried(otmp)
                    && !wielding_artifact(ART_HARBINGER)
                    && !wielding_artifact(ART_GIANTSLAYER)
                    && !(uarms && uarms->oartifact == ART_ASHMAR)
                    && !(uarm && uarm->oartifact == ART_ARMOR_OF_RETRIBUTION)) {
                    struct monst *zapper = otmp->ocarry;

                    pline_The("force of %s knocks you %s!",
                              otmp->otyp == WAN_STRIKING ? "the wand"
                                                         : "the spell",
                              u.usteed ? "out of your saddle" : "back");
                    last_hurtled = &youmonst;
                    hurtle(u.ux - zapper->mx, u.uy - zapper->my, 1, FALSE);
                    /* Update monster's knowledge of your position */
                    mtmp->mux = u.ux;
                    mtmp->muy = u.uy;
                }
                losehp(tmp, otmp->otyp == WAN_STRIKING ? "wand" : "force bolt",
                       KILLED_BY_AN);
            } else {
                if (otmp->otyp == WAN_STRIKING)
                    pline_The("wand misses you.");
                else
                    pline_The("force bolt misses you.");
            }
            stop_occupation();
            nomul(0);
        } else if (resists_magm(mtmp) || defended(mtmp, AD_MAGM)) {
            shieldeff(mtmp->mx, mtmp->my);
            pline("Boing!");
        } else if (rnd(20) < 10 + find_mac(mtmp)) {
            tmp = d(2, (otmp->otyp == WAN_STRIKING) ? 12 : 6);
            if (otmp->otyp == SPE_FORCE_BOLT && mcarried(otmp))
                tmp += otmp->ocarry->m_lev / 3;
            hit(otmp->otyp == WAN_STRIKING ? "wand" : "force bolt",
                mtmp, exclam(tmp));
            (void) resist(mtmp, otmp->oclass, tmp, TELL);
            if (cansee(mtmp->mx, mtmp->my) && zap_oseen
                && otmp->otyp == WAN_STRIKING)
                makeknown(WAN_STRIKING);
            /* Knockback threshold: 16 for wand, 12 for spell */
            if (tmp > (otmp->otyp == WAN_STRIKING ? 16 : 12)
                && mcarried(otmp)
                && !mon_harbinger_wield
                && !mon_giantslayer_wield) {
                struct monst *zapper = otmp->ocarry;

                last_hurtled = mtmp;
                if (tmp < mtmp->mhp) {
                    if (canseemon(mtmp))
                        pline_The("force of %s knocks %s back!",
                                  otmp->otyp == WAN_STRIKING ? "the wand"
                                                             : "the spell",
                                  mon_nam(mtmp));
                    if (mtmp == u.usteed) {
                        newsym(u.usteed->mx, u.usteed->my);
                        dismount_steed(DISMOUNT_FELL);
                    }
                    if (!DEADMONSTER(mtmp))
                        mhurtle(mtmp, mtmp->mx - zapper->mx,
                                mtmp->my - zapper->my, 1);
                }
            }
        } else {
            miss(otmp->otyp == WAN_STRIKING ? "wand" : "force bolt", mtmp);
            if (cansee(mtmp->mx, mtmp->my) && zap_oseen
                && otmp->otyp == WAN_STRIKING)
                makeknown(WAN_STRIKING);
        }
        break;
    case WAN_TELEPORTATION:
    case SPE_TELEPORT_AWAY:
        if (hits_you) {
            tele();
            if (zap_oseen && otmp->otyp == WAN_TELEPORTATION)
                makeknown(WAN_TELEPORTATION);
        } else {
            /* for consistency with zap.c, don't identify */
            if (mtmp->ispriest && *in_rooms(mtmp->mx, mtmp->my, TEMPLE)) {
                if (cansee(mtmp->mx, mtmp->my))
                    pline("%s resists the magic!", Monnam(mtmp));
            } else if (!tele_restrict(mtmp))
                (void) rloc(mtmp, TRUE);
        }
        break;
    case WAN_POLYMORPH:
    case SPE_POLYMORPH:
        if (hits_you) {
            if (Antimagic) {
                shieldeff(u.ux, u.uy);
                You_feel("momentarily different.");
                monstseesu(M_SEEN_MAGR);
                if (zap_oseen && otmp->otyp == WAN_POLYMORPH)
                    makeknown(WAN_POLYMORPH);
            } else if (!Unchanging) {
                if (zap_oseen && otmp->otyp == WAN_POLYMORPH)
                    makeknown(WAN_POLYMORPH);
                polyself(FALSE);
            }
        } else if (resists_magm(mtmp) || defended(mtmp, AD_MAGM)) {
            /* magic resistance protects from polymorph traps, so make
               it guard against involuntary polymorph attacks too... */
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
                    pline("%s shudders!", Monnam(mtmp));
                    if (zap_oseen && otmp->otyp == WAN_POLYMORPH)
                        makeknown(WAN_POLYMORPH);
                }
                if (canseemon(mtmp))
                    pline("%s is killed!", Monnam(mtmp));
                mtmp->mhp = 0;
                if (DEADMONSTER(mtmp))
                    mondied(mtmp);
            } else if (newcham(mtmp, (struct permonst *) 0, TRUE, FALSE)) {
                if (!Hallucination && zap_oseen && otmp->otyp == WAN_POLYMORPH)
                    makeknown(otmp->otyp);
            }
        }
        break;
    case WAN_CANCELLATION:
    case SPE_CANCELLATION:
        (void) cancel_monst(mtmp, otmp, FALSE, TRUE, FALSE);
        break;
    case WAN_UNDEAD_TURNING:
    case SPE_TURN_UNDEAD: {
        boolean learnit = FALSE;

        if (hits_you) {
            unturn_you();
            learnit = zap_oseen && otmp->otyp == WAN_UNDEAD_TURNING;
        } else {
            boolean wake = FALSE;

            if (unturn_dead(mtmp)) /* affects mtmp's invent, not mtmp */
                wake = TRUE;
            if (is_undead(mtmp->data) || is_vampshifter(mtmp)) {
                wake = reveal_invis = TRUE;
                /* context.bypasses=True: if resist() happens to be fatal,
                   make_corpse() will set obj->bypass on the new corpse
                   so that mbhito() will skip it instead of reviving it */
                context.bypasses = TRUE; /* for make_corpse() */
                (void) resist(mtmp, otmp->oclass, rnd(8), NOTELL);
            }
            if (wake) {
                if (!DEADMONSTER(mtmp))
                    wakeup(mtmp, FALSE);
                learnit = zap_oseen && otmp->otyp == WAN_UNDEAD_TURNING;
            }
        }
        if (learnit)
            makeknown(WAN_UNDEAD_TURNING);
        break;
    }
    case WAN_SLOW_MONSTER:
    case SPE_SLOW_MONSTER:
        if (hits_you) {
            if (!Slow)
                u_slow_down();
        } else {
            /* Monster target - slow them */
            if (!resist(mtmp, otmp->oclass, 0, NOTELL)) {
                mon_adjust_speed(mtmp, -1, otmp);
                if (canseemon(mtmp))
                    pline("%s slows down.", Monnam(mtmp));
            }
        }
        if (zap_oseen && otmp->otyp == WAN_SLOW_MONSTER)
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
                damage_mon(mtmp, dmg, AD_DRLI, TRUE);
                mtmp->mhpmax -= dmg;
                if (DEADMONSTER(mtmp) || mtmp->mhpmax <= 0 || mtmp->m_lev < 1) {
                    monkilled(mtmp, "", AD_DRLI);
                } else {
                    mtmp->m_lev--;
                    if (canseemon(mtmp))
                        pline("%s suddenly seems weaker!", Monnam(mtmp));
                }
            }
        }
        break;
    }
    case SPE_DISPEL_EVIL:
        reveal_invis = TRUE;
        if (hits_you) {
            /* Player is evil (Infidel) - take damage */
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
            if (!resist(mtmp, otmp->oclass, dmg, NOTELL)) {
                damage_mon(mtmp, dmg, AD_MAGM, TRUE);
                if (canseemon(mtmp))
                    pline("%s %s in %s!", Monnam(mtmp),
                          rn2(2) ? "withers" : "shudders",
                          rn2(2) ? "agony" : "pain");
                if (DEADMONSTER(mtmp)) {
                    monkilled(mtmp, "", AD_MAGM);
                } else {
                    monflee(mtmp, 0, FALSE, TRUE);
                }
            }
        }
        break;
    case SPE_CHARM_MONSTER:
        /* For monsters: untame an adjacent pet */
        reveal_invis = TRUE;
        if (!hits_you && mtmp->mtame && mcarried(otmp)) {
            struct monst *caster = otmp->ocarry;
            if (monnear(caster, mtmp->mx, mtmp->my)) {
                /* Untame the pet - free_edog also sets mtame = 0 */
                if (has_edog(mtmp))
                    free_edog(mtmp);
                else
                    mtmp->mtame = 0;
                mtmp->mpeaceful = 0;
                if (canseemon(mtmp))
                    Your("%s turns against you!", l_monnam(mtmp));
                newsym(mtmp->mx, mtmp->my);
            }
        }
        break;
    case SPE_ENTANGLE:
        cast_entangle(mtmp);
        break;
    default:
        break;
    }
    if (reveal_invis && !DEADMONSTER(mtmp)
        && cansee(bhitpos.x, bhitpos.y) && !canspotmon(mtmp))
        map_invisible(bhitpos.x, bhitpos.y);

    return 0;
}

/* A modified bhit() for monsters.  Based on bhit() in zap.c.  Unlike
 * buzz(), bhit() doesn't take into account the possibility of a monster
 * zapping you, so we need a special function for it.  (Unless someone wants
 * to merge the two functions...)
 */
STATIC_OVL void
mbhit(mon, range, fhitm, fhito, obj)
struct monst *mon;  /* monster shooting the wand */
register int range; /* direction and range */
int FDECL((*fhitm), (MONST_P, OBJ_P));
int FDECL((*fhito), (OBJ_P, OBJ_P)); /* fns called when mon/obj hit */
struct obj *obj;                     /* 2nd arg to fhitm/fhito */
{
    struct monst *mtmp;
    struct obj *otmp;
    register uchar typ;
    int ddx, ddy;

    bhitpos.x = mon->mx;
    bhitpos.y = mon->my;
    ddx = sgn(tbx);
    ddy = sgn(tby);

    while (range-- > 0) {
        int x, y;

        bhitpos.x += ddx;
        bhitpos.y += ddy;
        x = bhitpos.x;
        y = bhitpos.y;

        if (!isok(x, y)) {
            bhitpos.x -= ddx;
            bhitpos.y -= ddy;
            break;
        }
        if (find_drawbridge(&x, &y))
            switch (obj->otyp) {
            case WAN_STRIKING:
            case SPE_FORCE_BOLT:
                destroy_drawbridge(x, y);
            }
        if (levl[x][y].typ == IRONBARS
            && !(levl[x][y].wall_info & W_NONDIGGABLE)
            && (obj->otyp == WAN_STRIKING || obj->otyp == SPE_FORCE_BOLT)) {
            levl[x][y].typ = ROOM;
            if (cansee(x, y))
                pline_The("iron bars are blown apart!");
            else if (!Deaf)
                You_hear("a lot of loud clanging sounds!");
            wake_nearto(x, y, 20 * 20);
            newsym(x, y);
            /* stop the bolt here; it takes a lot of energy to destroy bars */
            range = 0;
        }
        /*
         * Affect objects on the floor before monster, so that objects dropped
         * by hero when polymorphed are safe from polymorph by same beam hit;
         * normally bypass_obj would be used for this purpose, but it is reset
         * in the process of hero polymorph so can't be used for polymorphing
         * hero.  This may need to be revisited if any issues seem to arise
         * from the new ordering.
         */
        /* save mtmp first, so that newly-revived zombie from undead
         * turning won't immediately get hit by the same zap */
        mtmp = m_at(bhitpos.x, bhitpos.y);
        /* modified by GAN to hit all objects */
        if (fhito) {
            int hitanything = 0;
            struct obj *next_obj;

            for (otmp = level.objects[bhitpos.x][bhitpos.y]; otmp;
                 otmp = next_obj) {
                /* Fix for polymorph bug, Tim Wright */
                next_obj = otmp->nexthere;
                hitanything += (*fhito)(otmp, obj);
            }
            if (hitanything)
                range--;
        }
        if (bhitpos.x == u.ux && bhitpos.y == u.uy) {
            (*fhitm)(&youmonst, obj);
            range -= 3;
        } else if (mtmp) {
            if (cansee(bhitpos.x, bhitpos.y) && !canspotmon(mtmp))
                map_invisible(bhitpos.x, bhitpos.y);
            (*fhitm)(mtmp, obj);
            range -= 3;
        }
        typ = levl[bhitpos.x][bhitpos.y].typ;
        if (IS_DOOR(typ) || typ == SDOOR) {
            switch (obj->otyp) {
            /* note: monsters don't use opening or locking magic
               at present, but keep these as placeholders */
            case WAN_OPENING:
            case WAN_LOCKING:
            case WAN_STRIKING:
            case SPE_FORCE_BOLT:
                if (doorlock(obj, bhitpos.x, bhitpos.y)) {
                    if (zap_oseen)
                        makeknown(obj->otyp);
                    /* if a shop or temple door gets broken, add it to
                       the repair list (no cost to player) */
                    if (levl[bhitpos.x][bhitpos.y].doormask == D_BROKEN
                        && (*in_rooms(bhitpos.x, bhitpos.y, SHOPBASE)
                            || temple_at_boundary(bhitpos.x, bhitpos.y)))
                        add_damage(bhitpos.x, bhitpos.y, 0L);
                }
                break;
            }
        }
        if (!ZAP_POS(typ)
            || (IS_DOOR(typ) && (levl[bhitpos.x][bhitpos.y].doormask
                                 & (D_LOCKED | D_CLOSED)))) {
            bhitpos.x -= ddx;
            bhitpos.y -= ddy;
            break;
        }
        maybe_explode_trap(t_at(x, y), obj); /* note: ttmp might be now gone */
    }

    last_hurtled = (struct monst *) 0;
}

/* Perform an offensive action for a monster.  Must be called immediately
 * after find_offensive().  Return values are same as use_defensive().
 */
int
use_offensive(mtmp)
struct monst *mtmp;
{
    int i, maxdmg = 0;
    struct obj *otmp = m.offensive;
    boolean oseen;
    struct attack* mattk;

    /* Safety check: if the object isn't in the monster's inventory,
       something went wrong (e.g., it was merged after extraction) */
    if (otmp && otmp->where != OBJ_MINVENT) {
        /* For scrolls of charging that were in containers, try to find
           the actual scroll in inventory (might have been merged) */
        if (m.has_offense == MUSE_SCR_CHARGING) {
            struct obj *o;
            for (o = mtmp->minvent; o; o = o->nobj) {
                if (o->otyp == SCR_CHARGING) {
                    otmp = m.offensive = o;
                    break;
                }
            }
            /* If still not found, abort */
            if (!otmp || otmp->where != OBJ_MINVENT) {
                m.has_offense = 0;
                m.offensive = 0;
                return 0;
            }
        } else if (otmp->oclass != POTION_CLASS) {
            /* Other non-potion items: just abort */
            m.has_offense = 0;
            m.offensive = 0;
            return 0;
        }
    }

    /* offensive potions are not drunk, they're thrown */
    if (otmp->oclass != POTION_CLASS && (i = precheck(mtmp, otmp)) != 0)
        return i;

    /* After precheck, otmp might have been merged/freed if it was extracted
       from a container. We need to re-validate and potentially re-find it */
    if (otmp && otmp->oclass != POTION_CLASS && otmp->where != OBJ_MINVENT) {
        /* For scrolls of charging, try to find the actual scroll in inventory */
        if (m.has_offense == MUSE_SCR_CHARGING) {
            struct obj *o;
            for (o = mtmp->minvent; o; o = o->nobj) {
                if (o->otyp == SCR_CHARGING) {
                    otmp = m.offensive = o;
                    break;
                }
            }
            /* If still not found, abort */
            if (!otmp || otmp->where != OBJ_MINVENT) {
                m.has_offense = 0;
                m.offensive = 0;
                return 0;
            }
        } else {
            /* Other items: try to find by type */
            struct obj *o;
            int otyp = m.offensive ? m.offensive->otyp : STRANGE_OBJECT;
            otmp = NULL;

            if (otyp != STRANGE_OBJECT) {
                for (o = mtmp->minvent; o; o = o->nobj) {
                    if (o->otyp == otyp) {
                        otmp = m.offensive = o;
                        break;
                    }
                }
            }

            if (!otmp) {
                m.has_offense = 0;
                m.offensive = 0;
                return 0;
            }
        }
    }

    oseen = otmp && canseemon(mtmp);

    /* From SporkHack (modified): some monsters would be better served if they
       were to melee attack instead using whatever offensive item they possess
       (read: master mind flayer zapping a wand of striking at the player repeatedly
       while in melee range). If the monster has an attack that is potentially
       better than its offensive item, or if it's wielding an artifact, and they're
       in melee range, don't give priority to the offensive item */
    for (i = 0; i < NATTK; i++) {
        mattk = &mtmp->data->mattk[i];
        maxdmg += mattk->damn * mattk->damd; /* total up the possible damage for just swinging */
    }

    /* If the monsters' combined damage from a melee attack exceeds nine,
       or if their wielded weapon is an artifact, use it if close enough.
       Exception being certain wands/horns that can incapacitate or can
       already do significant damage. Because intelligent monsters know
       not to use a certain attack if they've seen that the player is
       resistant to it, the monster will switch offensive items
       appropriately */
    if ((maxdmg > 9
        || (MON_WEP(mtmp) && MON_WEP(mtmp)->oartifact))
        && (monnear(mtmp, mtmp->mux, mtmp->muy)
            && m.has_offense != MUSE_FIRE_HORN
            && m.has_offense != MUSE_FROST_HORN
            && m.has_offense != MUSE_WAN_DEATH
            && m.has_offense != MUSE_WAN_SLEEP
            && m.has_offense != MUSE_WAN_FIRE
            && m.has_offense != MUSE_WAN_COLD
            && m.has_offense != MUSE_WAN_LIGHTNING)) {
        return 0;
    }

    switch (m.has_offense) {
    case MUSE_SCR_CHARGING: {
        /* Find what to charge at the moment of use, not during
           selection. This avoids pointer corruption issues that occur
           when storing the pointer across multiple turns */
        struct obj *item_to_charge = find_best_item_to_charge(mtmp);

        if (!item_to_charge) {
            /* Nothing suitable to charge - don't use the scroll */
            return 0;
        }

        mreadmsg(mtmp, otmp);
        if (oseen)
            makeknown(otmp->otyp);
        if (mtmp->mconf) {
            if (attacktype(mtmp->data, AT_MAGC))
                mtmp->mspec_used = 0;
            if (canseemon(mtmp))
                pline("%s looks charged up!", Monnam(mtmp));
        } else {
            recharge(item_to_charge, (otmp->cursed) ? -1 :
                     (otmp->blessed) ? 1 : 0, mtmp);
        }
        m_useup(mtmp, otmp);
        return (DEADMONSTER(mtmp)) ? 1 : 2;
    }
    case MUSE_WAN_DEATH:
    case MUSE_WAN_SLEEP:
    case MUSE_WAN_FIRE:
    case MUSE_WAN_COLD:
    case MUSE_WAN_LIGHTNING:
    case MUSE_WAN_MAGIC_MISSILE:
        mzapwand(mtmp, otmp, FALSE);
        if (oseen)
            makeknown(otmp->otyp);
        m_using = TRUE;
        buzz((int) (-(3 * MAX_ZT) - (otmp->otyp - WAN_MAGIC_MISSILE)),
             (otmp->otyp == WAN_MAGIC_MISSILE) ? 2 : 6, mtmp->mx, mtmp->my,
             sgn(tbx), sgn(tby));
        m_using = FALSE;
        return (DEADMONSTER(mtmp)) ? 1 : 2;
    case MUSE_FIRE_HORN:
    case MUSE_FROST_HORN:
        mplayhorn(mtmp, otmp, FALSE);
        m_using = TRUE;
        buzz(-(3 * MAX_ZT) - ((otmp->otyp == FROST_HORN) ? ZT_COLD : ZT_FIRE),
             rn1(6, 6), mtmp->mx, mtmp->my, sgn(tbx),
             sgn(tby));
        m_using = FALSE;
        return (DEADMONSTER(mtmp)) ? 1 : 2;
    case MUSE_WAN_CANCELLATION:
    case MUSE_WAN_TELEPORTATION:
    case MUSE_WAN_POLYMORPH:
    case MUSE_WAN_UNDEAD_TURNING:
    case MUSE_WAN_STRIKING:
    case MUSE_WAN_SLOW_MONSTER:
        zap_oseen = oseen;
        mzapwand(mtmp, otmp, FALSE);
        m_using = TRUE;
        mbhit(mtmp, rn1(8, 6), mbhitm, bhito, otmp);
        m_using = FALSE;
        return 2;
    case MUSE_SCR_EARTH: {
        /* TODO: handle steeds */
        register int x, y;
        /* don't use monster fields after killing it */
        boolean confused = (mtmp->mconf ? TRUE : FALSE);
        int mmx = mtmp->mx, mmy = mtmp->my;
        boolean is_cursed = otmp->cursed;
        boolean is_blessed = otmp->blessed;

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
        m_useup(mtmp, otmp); /* otmp now gone */

        /* Loop through the surrounding squares */
        for (x = mmx - 1; x <= mmx + 1; x++) {
            for (y = mmy - 1; y <= mmy + 1; y++) {
                /* Is this a suitable spot? */
                if (isok(x, y) && !closed_door(x, y)
                    && !IS_ROCK(levl[x][y].typ) && !IS_AIR(levl[x][y].typ)
                    && (((x == mmx) && (y == mmy)) ? !is_blessed
                                                   : !is_cursed)
                    && (x != u.ux || y != u.uy)) {
                    (void) drop_boulder_on_monster(x, y, confused, FALSE);
                }
            }
        }
        /* Attack the player */
        if (distmin(mmx, mmy, u.ux, u.uy) == 1 && !is_cursed) {
            drop_boulder_on_player(confused, TRUE, FALSE, TRUE);
        }

        return (DEADMONSTER(mtmp)) ? 1 : 2;
    } /* case MUSE_SCR_EARTH */
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
            pline("%s is uninjured.", Monnam(mtmp));
            (void) destroy_mitem(mtmp, SCROLL_CLASS, AD_FIRE);
            (void) destroy_mitem(mtmp, SPBOOK_CLASS, AD_FIRE);
            (void) destroy_mitem(mtmp, POTION_CLASS, AD_FIRE);
            num = (2 * (rn1(3, 3) + 2 * bcsign(otmp)) + 1) / 3;
            if (how_resistant(FIRE_RES) == 100) {
                You("are not harmed.");
                monstseesu(M_SEEN_FIRE);
            }
            burn_away_slime();
            if (Half_spell_damage)
                num = (num + 1) / 2;
            else
                losehp(num, "scroll of fire", KILLED_BY_AN);
            for (mtmp2 = fmon; mtmp2; mtmp2 = mtmp2->nmon) {
                if (DEADMONSTER(mtmp2))
                    continue;
                if (mtmp == mtmp2)
                    continue;
                if (dist2(mtmp2->mx, mtmp2->my, mtmp->mx, mtmp->my) < 3) {
                    if (resists_fire(mtmp2) || defended(mtmp2, AD_FIRE))
                        continue;
                    damage_mon(mtmp2, num, AD_FIRE, FALSE);
                    if (resists_cold(mtmp2)) /* natural resistance */
                        mtmp2->mhp -= 3 * num;
                    if (DEADMONSTER(mtmp2)) {
                        mondied(mtmp2);
                        break;
                    }
                }
            }
        }
        return (DEADMONSTER(mtmp)) ? 1 : 2;
    } /* case MUSE_SCR_FIRE */
    case MUSE_CAMERA: {
        if (Hallucination)
            verbalize("Say cheese!");
        else
            pline("%s takes a picture of you with %s!",
                  Monnam(mtmp), an(xname(otmp)));
        m_using = TRUE;
        if (!Blind) {
            You("are blinded by the flash of light!");
            make_blinded(Blinded + (long) rnd(1 + 50), FALSE);
        }
        lightdamage(otmp, TRUE, 5);
        m_using = FALSE;
        otmp->spe--;
        return (DEADMONSTER(mtmp)) ? 1 : 2;
    } /* case MUSE_CAMERA */
    case MUSE_POT_PARALYSIS:
    case MUSE_POT_BLINDNESS:
    case MUSE_POT_CONFUSION:
    case MUSE_POT_SLEEPING:
    case MUSE_POT_ACID:
    case MUSE_POT_POLYMORPH_THROW:
    case MUSE_POT_HALLUCINATION:
    case MUSE_POT_OIL: {
        /* Note: this setting of dknown doesn't suffice.  A monster
         * which is out of sight might throw and it hits something _in_
         * sight, a problem not existing with wands because wand rays
         * are not objects.  Also set dknown in mthrowu.c.
         */
        boolean isoil = (otmp->otyp == POT_OIL);
        struct obj *minvptr;
        if (cansee(mtmp->mx, mtmp->my)) {
            otmp->dknown = 1;
            pline("%s hurls %s!", Monnam(mtmp), singular(otmp, doname));
        }
        if (isoil && !otmp->lamplit && (!mtmp->mconf || rn2(3))) {
            /* A monster throwing oil probably wants it to explode; assume they
             * lit it just before throwing for simplicity;
             * a confused monster might forget to light it */
            begin_burn(otmp, FALSE);
        }
        m_throw(mtmp, mtmp->mx, mtmp->my, sgn(mtmp->mux - mtmp->mx),
                sgn(mtmp->muy - mtmp->my),
                distmin(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy), otmp, TRUE);
        if (isoil) {
            /* Possible situation: monster lights and throws 1 of a stack of oil
             * point blank -> it explodes -> monster is caught in explosion ->
             * monster's remaining oil ignites and explodes -> otmp is no longer
             * valid. So we need to check whether otmp is still in monster's
             * inventory or not. */
            for (minvptr = mtmp->minvent; minvptr; minvptr = minvptr->nobj) {
                if (minvptr == otmp)
                    break;
            }
            if (minvptr == otmp && otmp->lamplit)
                end_burn(otmp, TRUE);
        }
        return (DEADMONSTER(mtmp)) ? 1 : 2;
    }
    case MUSE_SCR_STINKING_CLOUD:
        mreadmsg(mtmp, otmp);
        if (oseen)
            makeknown(otmp->otyp);
        (void) create_gas_cloud(mtmp->mux, mtmp->muy, 3 + bcsign(otmp),
                                8 + 4 * bcsign(otmp));
        m_useup(mtmp, otmp);
        return (DEADMONSTER(mtmp)) ? 1 : 2;
    case 0:
        return 0; /* i.e. an exploded wand */
    default:
        impossible("%s wanted to perform action %d?", Monnam(mtmp),
                   m.has_offense);
        break;
    }
    return 0;
}

int
rnd_offensive_item(mtmp)
struct monst *mtmp;
{
    struct permonst *pm = mtmp->data;
    int difficulty = mons[(monsndx(pm))].difficulty;

    if (is_animal(pm) || attacktype(pm, AT_EXPL) || mindless(mtmp->data)
        || pm->mlet == S_GHOST || pm->mlet == S_KOP)
        return 0;
    if (difficulty > 7 && !rn2(35))
        return WAN_DEATH;
    if (difficulty > 7 && !rn2(30))
        return WAN_POLYMORPH;

    switch (rn2(9 - (difficulty < 4) + 4 * (difficulty > 6))) {
    case 0: {
        struct obj *helmet = which_armor(mtmp, W_ARMH);

        if ((helmet && is_hard(helmet)) || amorphous(pm)
            || passes_walls(pm) || noncorporeal(pm) || unsolid(pm))
            return SCR_EARTH;
    } /* fall through */
    case 1:
        return WAN_STRIKING;
    case 2:
        return POT_ACID;
    case 3:
        return POT_CONFUSION;
    case 4:
        return POT_BLINDNESS;
    case 5:
        return rn2(3) ? POT_SLEEPING : POT_HALLUCINATION;
    case 6:
        return POT_PARALYSIS;
    case 7:
    case 8:
        return WAN_MAGIC_MISSILE;
    case 9:
        return WAN_SLEEP;
    case 10:
        if (Iniceq)
            return rn2(6) ? WAN_SLEEP
                          : rn2(3) ? WAN_LIGHTNING
                                   : WAN_FIRE;
        else
            return rn2(30) ? WAN_FIRE : POT_OIL;
    case 11:
        return WAN_COLD;
    case 12:
        return WAN_LIGHTNING;
    case 13:
        return SCR_STINKING_CLOUD;
    case 14:
        return WAN_CANCELLATION;
    case 15:
        return WAN_SLOW_MONSTER;
    }
    /*NOTREACHED*/
    return 0;
}

#define MUSE_POT_GAIN_LEVEL 1
#define MUSE_WAN_MAKE_INVISIBLE 2
#define MUSE_POT_INVISIBILITY 3
#define MUSE_POLY_TRAP 4
#define MUSE_WAN_POLYMORPH_SELF 5
#define MUSE_POT_SPEED 6
#define MUSE_WAN_SPEED_MONSTER 7
#define MUSE_BULLWHIP 8
#define MUSE_POT_POLYMORPH 9
#define MUSE_SCR_REMOVE_CURSE 10
#define MUSE_WAN_WISHING 11
#define MUSE_FIGURINE 12
#define MUSE_DWARVISH_BEARDED_AXE_WEAPON 13
#define MUSE_DWARVISH_BEARDED_AXE_SHIELD 14
#define MUSE_PAN_FLUTE 15
#define MUSE_POT_BOOZE 16
#define MUSE_SPELLBOOK 17

/* Spells that monsters can learn from spellbooks.
   Excludes: racial abilities (psionic wave), spells with existing monster
   versions (haste, invisibility, protection, etc), divination (useless),
   and redundant summoning spells */
static const short learnable_spells[] = {
    /* Attack (9) */
    SPE_FORCE_BOLT, SPE_MAGIC_MISSILE, SPE_DRAIN_LIFE,
    SPE_FIREBALL, SPE_CONE_OF_COLD, SPE_LIGHTNING,
    SPE_POISON_BLAST, SPE_ACID_BLAST, SPE_POWER_WORD_KILL,
    /* Healing (2) */
    SPE_CURE_BLINDNESS, SPE_CURE_SICKNESS,
    /* Enchantment (5) */
    SPE_BURNING_HANDS, SPE_SLEEP, SPE_SLOW_MONSTER,
    SPE_SHOCKING_GRASP, SPE_CHARM_MONSTER,
    /* Cleric (3) */
    SPE_REMOVE_CURSE, SPE_TURN_UNDEAD, SPE_DISPEL_EVIL,
    /* Escape (3) */
    SPE_JUMPING, SPE_LEVITATION, SPE_TELEPORT_AWAY,
    /* Matter (4) */
    SPE_KNOCK, SPE_DIG, SPE_REPAIR_ARMOR, SPE_POLYMORPH,
    /* Evocation (2) */
    SPE_ENTANGLE, SPE_FINGER_OF_DEATH,
    0 /* sentinel */
};

/* Check if spell is in the learnable whitelist */
STATIC_OVL boolean
spell_is_learnable(otyp)
short otyp;
{
    int i;
    for (i = 0; learnable_spells[i] != 0; i++) {
        if (learnable_spells[i] == otyp)
            return TRUE;
    }
    return FALSE;
}

/* Can monster learn spell from this book? */
STATIC_OVL boolean
mcan_learn_spell(mtmp, book)
struct monst *mtmp;
struct obj *book;
{
    if (!is_spellcaster(mtmp))
        return FALSE;
    if (mindless(mtmp->data))
        return FALSE;
    /* Monster state checks - can't read if impaired */
    if (!mtmp->mcansee)  /* blind */
        return FALSE;
    if (mtmp->mstun || mtmp->mconf || helpless(mtmp))
        return FALSE;
    /* Book checks */
    if (book->cursed)
        return FALSE; /* Dangerous */
    if (mknows_spell(mtmp, book->otyp))
        return FALSE; /* Already knows */
    if (book->spestudied >= MAX_SPELL_STUDY)
        return FALSE; /* Depleted */
    /* Check if spell is in the learnable whitelist */
    if (!spell_is_learnable(book->otyp))
        return FALSE;
    /* Check if monster has room for another spell */
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
            return FALSE; /* All spell slots occupied */
    }
    return TRUE;
}

boolean
find_misc(mtmp)
struct monst *mtmp;
{
    struct obj *obj, *nextobj;
    struct permonst *mdat = mtmp->data;
    int x = mtmp->mx, y = mtmp->my;
    struct trap *t;
    int xx, yy, pmidx = NON_PM;
    boolean immobile = (mdat->mmove == 0);
    boolean stuck = (mtmp == u.ustuck);
    boolean trapped = mtmp->mtrapped;
    boolean entangled = mtmp->mentangled;

    m.misc = (struct obj *) 0;
    m.has_misc = 0;
    if (is_animal(mdat) || mindless(mdat) || nohands(mdat))
        return FALSE;
    if (u.uswallow && stuck)
        return FALSE;

    /* Spellbook reading - checked before distance check since spellcasters
       should read regardless of where they think the player is */
    if (is_spellcaster(mtmp)) {
        struct obj *book;

        /* If mid-read, continue unless hostile and player is close */
        if (has_emsp(mtmp) && EMSP(mtmp)->msp_reading != 0
            && EMSP(mtmp)->msp_read_turns > 0) {
            /* Hostile monsters stop reading when player is within 2 squares */
            if (!mtmp->mpeaceful && dist2(x, y, u.ux, u.uy) <= 8) {
                EMSP(mtmp)->msp_reading = 0;
                EMSP(mtmp)->msp_read_turns = 0;
                /* Fall through to find a new book or other action */
            } else {
                /* Find the book being read */
                for (book = mtmp->minvent; book; book = book->nobj) {
                    if (book->oclass == SPBOOK_CLASS
                        && book->otyp == EMSP(mtmp)->msp_reading) {
                        m.misc = book;
                        m.has_misc = MUSE_SPELLBOOK;
                        return TRUE;
                    }
                }
                /* Book not found - must have been stolen/destroyed */
                EMSP(mtmp)->msp_reading = 0;
                EMSP(mtmp)->msp_read_turns = 0;
            }
        }

        /* Find any learnable spellbook - but hostile monsters won't
           start reading when player is within 2 squares */
        if (mtmp->mpeaceful || dist2(x, y, u.ux, u.uy) > 8) {
            for (book = mtmp->minvent; book; book = book->nobj) {
                if (book->oclass == SPBOOK_CLASS
                    && mcan_learn_spell(mtmp, book)) {
                    m.misc = book;
                    m.has_misc = MUSE_SPELLBOOK;
                    return TRUE;
                }
            }
        }
    }

    /* We arbitrarily limit to times when a player is nearby for the
     * same reason as Junior Pac-Man doesn't have energizers eaten until
     * you can see them...
     */
    if (dist2(x, y, mtmp->mux, mtmp->muy) > 36)
        return FALSE;

    if (!stuck && !immobile && !trapped && !entangled
        && (mtmp->cham == NON_PM)
        && mons[(pmidx = monsndx(mdat))].difficulty < 6) {
        boolean ignore_boulders = (r_verysmall(mtmp)
                                   || racial_throws_rocks(mtmp)
                                   || passes_walls(mdat)),
            diag_ok = !NODIAG(pmidx);

        for (xx = x - 1; xx <= x + 1; xx++) {
            for (yy = y - 1; yy <= y + 1; yy++) {
                if (isok(xx, yy) && (xx != u.ux || yy != u.uy)
                    && (diag_ok || xx == x || yy == y)
                    && ((xx == x && yy == y) || !level.monsters[xx][yy])) {
                    if ((t = t_at(xx, yy)) != 0
                        && (ignore_boulders || !sobj_at(BOULDER, xx, yy))
                        && !onscary(xx, yy, mtmp)) {
                        /* use trap if it's the correct type */
                        if (t->ttyp == POLY_TRAP_SET) {
                            trapx = xx;
                            trapy = yy;
                            m.has_misc = MUSE_POLY_TRAP;
                            return TRUE;
                        }
                    }
                }
            }
        }
    }

#define nomore(x)       if (m.has_misc == (x)) continue
    /*
     * [bug?]  Choice of item is not prioritized; the last viable one
     * in the monster's inventory will be chosen.
     * 'nomore()' is nearly worthless because it only screens checking
     * of duplicates when there is no alternate type in between them.
     */
    for (obj = mtmp->minvent; obj; obj = nextobj) {
        nextobj = obj->nobj;
        /* Monsters shouldn't recognize cursed items; this kludge is
           necessary to prevent serious problems though... */
        if (obj->otyp == POT_BOOZE
            && is_satyr(mtmp->data)) {
            m.misc = obj;
            m.has_misc = MUSE_POT_BOOZE;
        }
        if (obj->otyp == POT_GAIN_LEVEL
            && (!obj->cursed
                || (!mtmp->isgd && !mtmp->isshk && !mtmp->ispriest))) {
            m.misc = obj;
            m.has_misc = MUSE_POT_GAIN_LEVEL;
        }
        nomore(MUSE_FIGURINE);
        if (obj->otyp == FIGURINE && !mtmp->mpeaceful) {
            m.misc = obj;
            m.has_misc = MUSE_FIGURINE;
        }
        nomore(MUSE_WAN_WISHING);
        if (obj->otyp == WAN_WISHING) {
            if (obj->spe > 0) {
                m.misc = obj;
                m.has_misc = MUSE_WAN_WISHING;
            }
            continue;
        }
        nomore(MUSE_BULLWHIP);
        if (obj->otyp == BULLWHIP && !mtmp->mpeaceful
            /* the random test prevents whip-wielding
               monster from attempting disarm every turn */
            && uwep && !rn2(5) && obj == MON_WEP(mtmp)
            /* hero's location must be known and adjacent */
            && mtmp->mux == u.ux && mtmp->muy == u.uy
            && distu(mtmp->mx, mtmp->my) <= 2
            /* don't bother if it can't work (this doesn't
               prevent cursed weapons from being targetted) */
            && !u.uswallow
            && (canletgo(uwep, "")
                || (u.twoweap && canletgo(uswapwep, "")))) {
            m.misc = obj;
            m.has_misc = MUSE_BULLWHIP;
        }
        nomore(MUSE_DWARVISH_BEARDED_AXE_WEAPON);
        if (obj->otyp == DWARVISH_BEARDED_AXE
            && !mtmp->mpeaceful
            /* the random test prevents axe-wielding
               monster from attempting disarm every turn */
            && uwep && !rn2(5) && obj == MON_WEP(mtmp)
            /* hero's location must be known and adjacent */
            && mtmp->mux == u.ux && mtmp->muy == u.uy
            && distu(mtmp->mx, mtmp->my) <= 2
            /* don't bother if it can't work (this doesn't
               prevent cursed weapons from being targetted) */
            && !u.uswallow
            && (canletgo(uwep, "")
                || (u.twoweap && canletgo(uswapwep, "")))) {
            m.misc = obj;
            m.has_misc = MUSE_DWARVISH_BEARDED_AXE_WEAPON;
        }
        nomore(MUSE_DWARVISH_BEARDED_AXE_SHIELD);
        if (obj->otyp == DWARVISH_BEARDED_AXE
            && !mtmp->mpeaceful
            /* the random test prevents axe-wielding
               monster from attempting shield removal every
               turn - shields are harder to disarm than weapons */
            && (uarms && !is_bracer(uarms))
            && !rn2(7) && obj == MON_WEP(mtmp)
            /* hero's location must be known and adjacent */
            && mtmp->mux == u.ux && mtmp->muy == u.uy
            && distu(mtmp->mx, mtmp->my) <= 2
            && !u.uswallow) {
            m.misc = obj;
            m.has_misc = MUSE_DWARVISH_BEARDED_AXE_SHIELD;
        }
        /* Note: peaceful/tame monsters won't make themselves
         * invisible unless you can see them.  Not really right, but...
         */
        nomore(MUSE_WAN_MAKE_INVISIBLE);
        if (obj->otyp == WAN_MAKE_INVISIBLE && !mtmp->minvis
            && !mtmp->invis_blkd && (!mtmp->mpeaceful || See_invisible)
            && (!attacktype(mtmp->data, AT_GAZE) || mtmp->mcan)) {
            if (obj->spe > 0) {
                m.misc = obj;
                m.has_misc = MUSE_WAN_MAKE_INVISIBLE;
            }
        }
        nomore(MUSE_POT_INVISIBILITY);
        if (obj->otyp == POT_INVISIBILITY && !mtmp->minvis
            && !mtmp->invis_blkd && (!mtmp->mpeaceful || See_invisible)
            && (!attacktype(mtmp->data, AT_GAZE) || mtmp->mcan)) {
            m.misc = obj;
            m.has_misc = MUSE_POT_INVISIBILITY;
        }
        nomore(MUSE_WAN_SPEED_MONSTER);
        if (obj->otyp == WAN_SPEED_MONSTER
            && mtmp->mspeed != MFAST && !mtmp->isgd) {
            if (obj->spe > 0) {
                m.misc = obj;
                m.has_misc = MUSE_WAN_SPEED_MONSTER;
            }
        }
        nomore(MUSE_POT_SPEED);
        if (obj->otyp == POT_SPEED && mtmp->mspeed != MFAST && !mtmp->isgd) {
            m.misc = obj;
            m.has_misc = MUSE_POT_SPEED;
        }
        nomore(MUSE_WAN_POLYMORPH_SELF);
        if (obj->otyp == WAN_POLYMORPH
            && (mtmp->cham == NON_PM) && !mtmp->isshk
            && mons[monsndx(mdat)].difficulty < 6) {
            if (obj->spe > 0) {
                m.misc = obj;
                m.has_misc = MUSE_WAN_POLYMORPH_SELF;
            }
        }
        nomore(MUSE_POT_POLYMORPH);
        if (obj->otyp == POT_POLYMORPH && (mtmp->cham == NON_PM)
            && !mtmp->isshk && mons[monsndx(mdat)].difficulty < 6) {
            m.misc = obj;
            m.has_misc = MUSE_POT_POLYMORPH;
        }
        if (m.has_misc == MUSE_SCR_CHARGING)
            continue;
        if (obj->otyp == SCR_CHARGING) {
            /* Check if monster has anything to charge right now */
            if (find_best_item_to_charge(mtmp)) {
                m.misc = obj;
                m.has_misc = MUSE_SCR_CHARGING;
            }
            continue;
        }
        nomore(MUSE_SCR_REMOVE_CURSE);
        if (obj->otyp == SCR_REMOVE_CURSE
            && !obj->cursed
            && mtmp->mnum != PM_INFIDEL) {
            struct obj *otmp;

            for (otmp = mtmp->minvent;
                 otmp; otmp = otmp->nobj) {
                if (otmp->cursed
                    && (otmp->otyp == LOADSTONE
                        || Is_mbag(otmp)
                        || otmp->owornmask)) {
                    m.misc = obj;
                    m.has_misc = MUSE_SCR_REMOVE_CURSE;
                }
            }
        }
        nomore(MUSE_PAN_FLUTE);
        if (obj->otyp == PAN_FLUTE
            && is_satyr(mtmp->data)
            && !mtmp->mpeaceful && !rn2(5)) {
            m.misc = obj;
            m.has_misc = MUSE_PAN_FLUTE;
        }
    }
    return find_misc_recurse(mtmp, mtmp->minvent);
#undef nomore
}

STATIC_OVL boolean
find_misc_recurse(mtmp, start)
struct monst *mtmp;
struct obj *start;
{
    struct obj *obj, *nextobj;
    struct permonst *mdat = mtmp->data;

#define nomore(x)       if (m.has_misc == (x)) continue;
    for (obj = start; obj; obj = nextobj) {
        nextobj = obj->nobj;
        if (Is_container(obj)) {
            (void) find_misc_recurse(mtmp, obj->cobj);
            continue;
        }

        /* Monsters shouldn't recognize cursed items; this kludge is
           necessary to prevent serious problems though... */
        if (obj->otyp == POT_BOOZE
            && is_satyr(mtmp->data)) {
            m.misc = obj;
            m.has_misc = MUSE_POT_BOOZE;
        }
        if (obj->otyp == POT_GAIN_LEVEL
            && (!obj->cursed
                || (!mtmp->isgd && !mtmp->isshk && !mtmp->ispriest))) {
            m.misc = obj;
            m.has_misc = MUSE_POT_GAIN_LEVEL;
        }
        nomore(MUSE_FIGURINE);
        if (obj->otyp == FIGURINE && !mtmp->mpeaceful) {
            m.misc = obj;
            m.has_misc = MUSE_FIGURINE;
        }
        nomore(MUSE_WAN_WISHING);
        if (obj->otyp == WAN_WISHING) {
            if (obj->spe > 0) {
                m.misc = obj;
                m.has_misc = MUSE_WAN_WISHING;
            }
            continue;
        }
        nomore(MUSE_BULLWHIP);
        if (obj->otyp == BULLWHIP && !mtmp->mpeaceful
            /* the random test prevents whip-wielding
               monster from attempting disarm every turn */
            && uwep && !rn2(5) && obj == MON_WEP(mtmp)
            /* hero's location must be known and adjacent */
            && mtmp->mux == u.ux && mtmp->muy == u.uy
            && distu(mtmp->mx, mtmp->my) <= 2
            /* don't bother if it can't work (this doesn't
               prevent cursed weapons from being targetted) */
            && !u.uswallow
            && (canletgo(uwep, "")
                || (u.twoweap && canletgo(uswapwep, "")))) {
            m.misc = obj;
            m.has_misc = MUSE_BULLWHIP;
        }
        nomore(MUSE_DWARVISH_BEARDED_AXE_WEAPON);
        if (obj->otyp == DWARVISH_BEARDED_AXE
            && !mtmp->mpeaceful
            /* the random test prevents axe-wielding
               monster from attempting disarm every turn */
            && uwep && !rn2(5) && obj == MON_WEP(mtmp)
            /* hero's location must be known and adjacent */
            && mtmp->mux == u.ux && mtmp->muy == u.uy
            && distu(mtmp->mx, mtmp->my) <= 2
            /* don't bother if it can't work (this doesn't
               prevent cursed weapons from being targetted) */
            && !u.uswallow
            && (canletgo(uwep, "")
                || (u.twoweap && canletgo(uswapwep, "")))) {
            m.misc = obj;
            m.has_misc = MUSE_DWARVISH_BEARDED_AXE_WEAPON;
        }
        nomore(MUSE_DWARVISH_BEARDED_AXE_SHIELD);
        if (obj->otyp == DWARVISH_BEARDED_AXE
            && !mtmp->mpeaceful
            /* the random test prevents axe-wielding
               monster from attempting shield removal every
               turn - shields are harder to disarm than weapons */
            && (uarms && !is_bracer(uarms))
            && !rn2(7) && obj == MON_WEP(mtmp)
            /* hero's location must be known and adjacent */
            && mtmp->mux == u.ux && mtmp->muy == u.uy
            && distu(mtmp->mx, mtmp->my) <= 2
            && !u.uswallow) {
            m.misc = obj;
            m.has_misc = MUSE_DWARVISH_BEARDED_AXE_SHIELD;
        }
        /* Note: peaceful/tame monsters won't make themselves
         * invisible unless you can see them.  Not really right, but...
         */
        nomore(MUSE_WAN_MAKE_INVISIBLE);
        if (obj->otyp == WAN_MAKE_INVISIBLE && !mtmp->minvis
            && !mtmp->invis_blkd && (!mtmp->mpeaceful || See_invisible)
            && (!attacktype(mtmp->data, AT_GAZE) || mtmp->mcan)) {
            if (obj->spe > 0) {
                m.misc = obj;
                m.has_misc = MUSE_WAN_MAKE_INVISIBLE;
            }
        }
        nomore(MUSE_POT_INVISIBILITY);
        if (obj->otyp == POT_INVISIBILITY && !mtmp->minvis
            && !mtmp->invis_blkd && (!mtmp->mpeaceful || See_invisible)
            && (!attacktype(mtmp->data, AT_GAZE) || mtmp->mcan)) {
            m.misc = obj;
            m.has_misc = MUSE_POT_INVISIBILITY;
        }
        nomore(MUSE_WAN_SPEED_MONSTER);
        if (obj->otyp == WAN_SPEED_MONSTER
            && mtmp->mspeed != MFAST && !mtmp->isgd) {
            if (obj->spe > 0) {
                m.misc = obj;
                m.has_misc = MUSE_WAN_SPEED_MONSTER;
            }
        }
        nomore(MUSE_POT_SPEED);
        if (obj->otyp == POT_SPEED && mtmp->mspeed != MFAST && !mtmp->isgd) {
            m.misc = obj;
            m.has_misc = MUSE_POT_SPEED;
        }
        nomore(MUSE_WAN_POLYMORPH_SELF);
        if (obj->otyp == WAN_POLYMORPH
            && (mtmp->cham == NON_PM) && !mtmp->isshk
            && mons[monsndx(mdat)].difficulty < 6) {
            if (obj->spe > 0) {
                m.misc = obj;
                m.has_misc = MUSE_WAN_POLYMORPH_SELF;
            }
        }
        nomore(MUSE_POT_POLYMORPH);
        if (obj->otyp == POT_POLYMORPH && (mtmp->cham == NON_PM)
            && !mtmp->isshk && mons[monsndx(mdat)].difficulty < 6) {
            m.misc = obj;
            m.has_misc = MUSE_POT_POLYMORPH;
        }
        if (m.has_misc == MUSE_SCR_CHARGING)
            continue;
        if (obj->otyp == SCR_CHARGING) {
            /* Check if monster has anything to charge right now */
            if (find_best_item_to_charge(mtmp)) {
                m.misc = obj;
                m.has_misc = MUSE_SCR_CHARGING;
            }
            continue;
        }
        nomore(MUSE_SCR_REMOVE_CURSE);
        if (obj->otyp == SCR_REMOVE_CURSE
            && !obj->cursed
            && mtmp->mnum != PM_INFIDEL) {
            struct obj *otmp;

            for (otmp = mtmp->minvent;
                 otmp; otmp = otmp->nobj) {
                if (otmp->cursed
                    && (otmp->otyp == LOADSTONE
                        || Is_mbag(otmp)
                        || otmp->owornmask)) {
                    m.misc = obj;
                    m.has_misc = MUSE_SCR_REMOVE_CURSE;
                }
            }
        }
        nomore(MUSE_PAN_FLUTE);
        if (obj->otyp == PAN_FLUTE
            && is_satyr(mtmp->data)
            && !mtmp->mpeaceful && !rn2(5)) {
            m.misc = obj;
            m.has_misc = MUSE_PAN_FLUTE;
        }
    }
    if (mtmp->mfrozen) {
        m.misc = (struct obj *) 0;
        m.has_misc = 0;
        return FALSE;
    }
    return (boolean) !!m.has_misc;
#undef nomore
}

/* type of monster to polymorph into; defaults to one suitable for the
   current level rather than the totally arbitrary choice of newcham() */
static struct permonst *
muse_newcham_mon(mon)
struct monst *mon;
{
    int pm = armor_to_dragon(mon);
    if (pm != NON_PM) {
        return &mons[pm];
    }
    /* not wearing anything that would turn it into a dragon */
    return rndmonst();
}

int
use_misc(mtmp)
struct monst *mtmp;
{
    int i;
    struct obj *otmp = m.misc;
    boolean vis, vismon, oseen;
    char nambuf[BUFSZ];
    struct trap * tt;

    /* Safety check: if the object isn't in the monster's inventory,
       something went wrong (e.g., it was merged after extraction) */
    if (otmp && otmp->where != OBJ_MINVENT) {
        /* For scrolls of charging that were in containers, try to find
           the actual scroll in inventory (might have been merged) */
        if (m.has_misc == MUSE_SCR_CHARGING) {
            struct obj *o;
            for (o = mtmp->minvent; o; o = o->nobj) {
                if (o->otyp == SCR_CHARGING) {
                    otmp = m.misc = o;
                    break;
                }
            }
            /* If still not found, abort */
            if (!otmp || otmp->where != OBJ_MINVENT) {
                m.has_misc = 0;
                m.misc = 0;
                return 0;
            }
        } else {
            /* Other items: just abort */
            m.has_misc = 0;
            m.misc = 0;
            return 0;
        }
    }

    if ((i = precheck(mtmp, otmp)) != 0)
        return i;

    /* After precheck, otmp might have been merged/freed if it was extracted
       from a container. We need to re-validate and potentially re-find it */
    if (otmp && otmp->where != OBJ_MINVENT) {
        /* For scrolls of charging, try to find the actual scroll in inventory */
        if (m.has_misc == MUSE_SCR_CHARGING) {
            struct obj *o;
            for (o = mtmp->minvent; o; o = o->nobj) {
                if (o->otyp == SCR_CHARGING) {
                    otmp = m.misc = o;
                    break;
                }
            }
            /* If still not found, abort */
            if (!otmp || otmp->where != OBJ_MINVENT) {
                m.has_misc = 0;
                m.misc = 0;
                return 0;
            }
        } else {
            /* Other items: try to find by type */
            struct obj *o;
            int otyp = m.misc ? m.misc->otyp : STRANGE_OBJECT;
            otmp = NULL;

            if (otyp != STRANGE_OBJECT) {
                for (o = mtmp->minvent; o; o = o->nobj) {
                    if (o->otyp == otyp) {
                        otmp = m.misc = o;
                        break;
                    }
                }
            }

            if (!otmp) {
                m.has_misc = 0;
                m.misc = 0;
                return 0;
            }
        }
    }

    vis = cansee(mtmp->mx, mtmp->my);
    vismon = canseemon(mtmp);
    oseen = otmp && vismon;

    switch (m.has_misc) {
    case MUSE_SCR_CHARGING: {
        /* Find what to charge at the moment of use, not during selection.
           This avoids pointer corruption issues that occur when storing
           the pointer across multiple turns */
        struct obj *item_to_charge = find_best_item_to_charge(mtmp);

        if (!item_to_charge) {
            /* Nothing suitable to charge - don't use the scroll */
            return 0;
        }

        mreadmsg(mtmp, otmp);
        if (oseen)
            makeknown(otmp->otyp);
        if (mtmp->mconf) {
            if (attacktype(mtmp->data, AT_MAGC))
                mtmp->mspec_used = 0;
            if (canseemon(mtmp))
                pline("%s looks charged up!", Monnam(mtmp));
        } else {
            recharge(item_to_charge, (otmp->cursed) ? -1 :
                     (otmp->blessed) ? 1 : 0, mtmp);
        }
        m_useup(mtmp, otmp);
        return 2;
    }
    case MUSE_FIGURINE: {
        coord cc;
        int mndx = otmp->corpsenm;

        if (otmp->oartifact == ART_IDOL_OF_MOLOCH)
            break;
        if (mndx < LOW_PM || mndx >= NUMMONS) {
            impossible("use_misc: mon activating bad figurine (%d)?", mndx);
            break;
        }
        if (!enexto(&cc, mtmp->mx, mtmp->my, &mons[mndx]))
            break;
        /* found an acceptable spot for the figurine to transform.
         * make_familiar will take care of the various checks for genocide,
         * extinction, etc, and print failure messages if appropriate. */
        if (vismon) {
            pline("%s activates a figurine, and it transforms!", Monnam(mtmp));
        } else {
            if (!Deaf)
                You_hear("a figurine being activated.");
        }
        /* curse the figurine so that it will produce a hostile monster most
         * of the time */
        otmp->blessed = 0, otmp->cursed = 1;
        (void) make_familiar(otmp, cc.x, cc.y, !vismon, FALSE);
        m_useup(mtmp, otmp);
        return 2;
    }
    case MUSE_POT_GAIN_LEVEL:
        mquaffmsg(mtmp, otmp);
        if (otmp->cursed) {
            if (Can_rise_up(mtmp->mx, mtmp->my, &u.uz)) {
                register int tolev = depth(&u.uz) - 1;
                d_level tolevel;

                get_level(&tolevel, tolev);
                /* insurance against future changes... */
                if (on_level(&tolevel, &u.uz)
                    || (Iniceq && mtmp->data == &mons[PM_KATHRYN_THE_ICE_QUEEN])
                    || (Is_sanctum(&u.uz) && mtmp->data == &mons[PM_LUCIFER]))
                    goto skipmsg;
                if (vismon) {
                    pline("%s rises up, through the %s!", Monnam(mtmp),
                          ceiling(mtmp->mx, mtmp->my));
                    if (!objects[POT_GAIN_LEVEL].oc_name_known
                        && !objects[POT_GAIN_LEVEL].oc_uname)
                        docall(otmp);
                }
                m_useup(mtmp, otmp);
                migrate_to_level(mtmp, ledger_no(&tolevel), MIGR_RANDOM,
                                 (coord *) 0);
                return 2;
            } else {
 skipmsg:
                if (vismon) {
                    pline("%s looks uneasy.", Monnam(mtmp));
                    if (!objects[POT_GAIN_LEVEL].oc_name_known
                        && !objects[POT_GAIN_LEVEL].oc_uname)
                        docall(otmp);
                }
                m_useup(mtmp, otmp);
                return 2;
            }
        }
        if (vismon)
            pline("%s seems more experienced.", Monnam(mtmp));
        if (oseen)
            makeknown(POT_GAIN_LEVEL);
        m_useup(mtmp, otmp);
        if (!grow_up(mtmp, (struct monst *) 0))
            return 1;
        /* grew into genocided monster */
        return 2;
    case MUSE_WAN_MAKE_INVISIBLE:
    case MUSE_POT_INVISIBILITY:
        if (otmp->otyp == WAN_MAKE_INVISIBLE) {
            mzapwand(mtmp, otmp, TRUE);
        } else
            mquaffmsg(mtmp, otmp);
        /* format monster's name before altering its visibility */
        Strcpy(nambuf, mon_nam(mtmp));
        mon_set_minvis(mtmp);
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
        }
        if (otmp->otyp == POT_INVISIBILITY) {
            if (otmp->cursed)
                you_aggravate(mtmp);
            m_useup(mtmp, otmp);
        }
        return 2;
    case MUSE_WAN_SPEED_MONSTER:
        mzapwand(mtmp, otmp, TRUE);
        mon_adjust_speed(mtmp, 1, otmp);
        return 2;
    case MUSE_POT_SPEED:
        mquaffmsg(mtmp, otmp);
        /* note difference in potion effect due to substantially
           different methods of maintaining speed ratings:
           player's character becomes "very fast" temporarily;
           monster becomes "one stage faster" permanently */
        mon_adjust_speed(mtmp, 1, otmp);
        m_useup(mtmp, otmp);
        return 2;
    case MUSE_WAN_POLYMORPH_SELF:
        mzapwand(mtmp, otmp, TRUE);
        (void) newcham(mtmp, muse_newcham_mon(mtmp), TRUE, FALSE);
        if (oseen)
            makeknown(WAN_POLYMORPH);
        return 2;
    case MUSE_WAN_WISHING:
        /* wear any armor items previously wished for before
         * using another wish */
        m_dowear(mtmp, FALSE);

        mzapwand(mtmp, otmp, FALSE);
        if (!vismon && !Deaf)
            You_hear("something making a wish!");
        if (oseen)
            makeknown(WAN_WISHING);
        mmake_wish(mtmp);
        return 2;
    case MUSE_POT_POLYMORPH:
        mquaffmsg(mtmp, otmp);
        m_useup(mtmp, otmp);
        if (vismon)
            pline("%s suddenly mutates!", Monnam(mtmp));
        (void) newcham(mtmp, muse_newcham_mon(mtmp), FALSE, FALSE);
        if (oseen)
            makeknown(POT_POLYMORPH);
        return 2;
    case MUSE_SCR_REMOVE_CURSE:
        mreadmsg(mtmp, otmp);
        if (canseemon(mtmp)) {
            if (mtmp->mconf)
                You("feel as though %s needs some help.", mon_nam(mtmp));
            else
                You("feel like someone is helping %s.", mon_nam(mtmp));
            if (!objects[SCR_REMOVE_CURSE].oc_name_known
                && !objects[SCR_REMOVE_CURSE].oc_uname)
                docall(otmp);
        }

        {
            struct obj *obj;

            for (obj = mtmp->minvent; obj; obj = obj->nobj) {
                /* gold isn't subject to cursing and blessing */
                if (obj->oclass == COIN_CLASS)
                    continue;
                if (obj->owornmask
                    || Is_mbag(obj)
                    || obj->otyp == LOADSTONE) {
                    if (mtmp->mconf)
                        blessorcurse(obj, 2);
                    else
                        uncurse(obj);
                }
            }
        }
        m_useup(mtmp, otmp);
        return 0;
    case MUSE_POLY_TRAP:
        tt = t_at(trapx, trapy);
        if (vismon) {
            const char *Mnam = Monnam(mtmp);

            pline("%s deliberately %s onto a polymorph trap!", Mnam,
                  vtense(fakename[0], locomotion(mtmp->data, "jump")));
        }
        if (vis)
            seetrap(tt);
        deltrap(tt);

        /*  don't use rloc() due to worms */
        remove_monster(mtmp->mx, mtmp->my);
        newsym(mtmp->mx, mtmp->my);
        place_monster(mtmp, trapx, trapy);
        maybe_unhide_at(trapx, trapy);
        if (mtmp->wormno)
            worm_move(mtmp);
        newsym(trapx, trapy);

        (void) newcham(mtmp, (struct permonst *) 0, FALSE, FALSE);
        return 2;
    case MUSE_BULLWHIP:
        /* attempt to disarm hero */
        {
            const char *The_whip = vismon ? "The bullwhip" : "A whip";
            int where_to = rn2(4);
            struct obj *obj = uwep;
            const char *hand;
            char the_weapon[BUFSZ];

            if (!obj || !canletgo(obj, "")
                || (u.twoweap && canletgo(uswapwep, "") && rn2(2)))
                obj = uswapwep;
            if (!obj)
                break; /* shouldn't happen after find_misc() */

            Strcpy(the_weapon, the(xname(obj)));
            hand = body_part(HAND);
            if (bimanual(obj))
                hand = makeplural(hand);

            if (vismon)
                pline("%s flicks a bullwhip towards your %s!", Monnam(mtmp),
                      hand);
            if (obj->otyp == HEAVY_IRON_BALL || Hidinshell) {
                pline("%s fails to wrap around %s.", The_whip, the_weapon);
                return 1;
            }
            pline("%s wraps around %s you're wielding!", The_whip,
                  the_weapon);
            if (welded(obj)) {
                pline("%s welded to your %s%c",
                      !is_plural(obj) ? "It is" : "They are", hand,
                      !obj->bknown ? '!' : '.');
                /* obj->bknown = 1; */ /* welded() takes care of this */
                where_to = 0;
            }
            if (!where_to) {
                pline_The("whip slips free."); /* not `The_whip' */
                return 1;
            } else if (where_to == 3
                       && mon_hates_material(mtmp, obj->material)) {
                /* this monster won't want to catch a silver
                   weapon; drop it at hero's feet instead */
                where_to = 2;
            }
            remove_worn_item(obj, FALSE);
            freeinv(obj);
            switch (where_to) {
            case 1: /* onto floor beneath mon */
                pline("%s yanks %s from your %s!", Monnam(mtmp), the_weapon,
                      hand);
                place_object(obj, mtmp->mx, mtmp->my);
                break;
            case 2: /* onto floor beneath you */
                pline("%s yanks %s to the %s!", Monnam(mtmp), the_weapon,
                      surface(u.ux, u.uy));
                dropy(obj);
                break;
            case 3: /* into mon's inventory */
                pline("%s snatches %s!", Monnam(mtmp), the_weapon);
                (void) mpickobj(mtmp, obj);
                break;
            }
            return 1;
        }
        return 0;
    case MUSE_DWARVISH_BEARDED_AXE_WEAPON:
        /* attempt to disarm hero */
        {
            const char *The_axe = vismon ? "The axe" : "An axe";
            int where_to = rn2(4);
            struct obj *obj = uwep;
            const char *hand;
            char the_weapon[BUFSZ];

            if (!obj || !canletgo(obj, "")
                || (u.twoweap && canletgo(uswapwep, "") && rn2(2)))
                obj = uswapwep;
            if (!obj)
                break; /* shouldn't happen after find_misc() */

            Strcpy(the_weapon, the(xname(obj)));
            hand = body_part(HAND);
            if (bimanual(obj))
                hand = makeplural(hand);

            if (vismon)
                pline("%s swings %s axe towards your %s!", Monnam(mtmp),
                      mhis(mtmp), hand);
            if (obj->otyp == HEAVY_IRON_BALL || Hidinshell) {
                pline("%s glances off of %s.", The_axe, the_weapon);
                return 1;
            }
            if (is_flimsy(obj)) {
                pline("%s is unable to hook onto %s.", The_axe, the_weapon);
                return 1;
            }
            pline("%s hooks onto %s you're wielding!", The_axe,
                  the_weapon);
            if (welded(obj)) {
                pline("%s welded to your %s%c",
                      !is_plural(obj) ? "It is" : "They are", hand,
                      !obj->bknown ? '!' : '.');
                /* obj->bknown = 1; */ /* welded() takes care of this */
                where_to = 0;
            }
            if (!where_to) {
                pline_The("axe comes free."); /* not `The_whip' */
                return 1;
            } else if (where_to == 3
                       && mon_hates_material(mtmp, obj->material)) {
                /* this monster won't want to catch a silver
                   weapon; drop it at hero's feet instead */
                where_to = 2;
            }
            remove_worn_item(obj, FALSE);
            freeinv(obj);
            switch (where_to) {
            case 1: /* onto floor beneath mon */
                pline("%s pulls %s from your %s!", Monnam(mtmp), the_weapon,
                      hand);
                place_object(obj, mtmp->mx, mtmp->my);
                break;
            case 2: /* onto floor beneath you */
                pline("%s pulls %s to the %s!", Monnam(mtmp), the_weapon,
                      surface(u.ux, u.uy));
                dropy(obj);
                break;
            case 3: /* into mon's inventory */
                pline("%s disarms you, and snatches %s!", Monnam(mtmp), the_weapon);
                (void) mpickobj(mtmp, obj);
                break;
            }
            return 1;
        }
        return 0;
    case MUSE_DWARVISH_BEARDED_AXE_SHIELD:
        /* attempt to remove the heroes shield */
        {
            const char *The_axe = vismon ? "The axe" : "An axe";
            int where_to = rn2(4);
            struct obj *obj = uarms;
            const char *hand;
            char the_shield[BUFSZ];

            if (!obj || is_bracer(obj))
                break; /* shouldn't happen after find_misc() */

            Strcpy(the_shield, the(xname(obj)));
            hand = body_part(HAND);
            if (bimanual(obj))
                hand = makeplural(hand);

            if (vismon)
                pline("%s swings %s axe towards your off%s!", Monnam(mtmp),
                      mhis(mtmp), hand);
            if (is_flimsy(obj) || Hidinshell) {
                pline("%s is unable to hook onto %s.", The_axe, the_shield);
                return 1;
            }
            pline("%s hooks onto %s!", The_axe,
                  the_shield);
            if (cursed(obj, TRUE)) {
                pline("%s welded to your off%s%c",
                      !is_plural(obj) ? "It is" : "They are", hand,
                      !obj->bknown ? '!' : '.');
                /* obj->bknown = 1; */ /* welded() takes care of this */
                where_to = 0;
            } else if (P_SKILL(P_SHIELD) >= P_EXPERT) {
                You("hold on firmly to %s.", ysimple_name(obj));
                where_to = 0;
            }
            if (!where_to) {
                pline_The("axe comes free."); /* not `The_whip' */
                return 1;
            } else if (where_to == 3
                       && mon_hates_material(mtmp, obj->material)) {
                /* this monster won't want to catch a silver
                   weapon; drop it at hero's feet instead */
                where_to = 2;
            }
            remove_worn_item(obj, FALSE);
            freeinv(obj);
            switch (where_to) {
            case 1: /* onto floor beneath mon */
                pline("%s pulls %s from your off%s!", Monnam(mtmp), the_shield,
                      hand);
                place_object(obj, mtmp->mx, mtmp->my);
                break;
            case 2: /* onto floor beneath you */
                pline("%s pulls %s to the %s!", Monnam(mtmp), the_shield,
                      surface(u.ux, u.uy));
                dropy(obj);
                break;
            case 3: /* into mon's inventory */
                pline("%s removes and snatches %s!", Monnam(mtmp), the_shield);
                (void) mpickobj(mtmp, obj);
                break;
            }
            return 1;
        }
        return 0;
    case MUSE_PAN_FLUTE:
        {
            int chance = rnd(3);
            int range = (couldsee(mtmp->mx, mtmp->my)
                         && (dist2(mtmp->mx, mtmp->my,
                                   mtmp->mux, mtmp->muy) <= 25));

            if (Deaf) /* nothing happens */
                return 0;

            if (!vismon) {
                if (!Deaf) {
                    You_hear("a pan flute being played %s.",
                             range ? "nearby" : "in the distance");
                }
            } else if (vismon) {
                pline("%s plays %s %s, producing %s melody.",
                      Monnam(mtmp), mhis(mtmp), xname(otmp),
                      rn2(2) ? "a beautiful" : "an enchanting");
                if (oseen)
                    makeknown(otmp->otyp);
            }
            /* various effects */
            if (!Deaf && uarmh && uarmh->otyp == TOQUE) {
                if (!rn2(3)) /* not spammy */
                    pline("Your %s protects your ears from the enchanting music.",
                          helm_simple_name(uarmh));
            } else if (is_nymph(youmonst.data)
                       || is_satyr(youmonst.data)) {
                if (!rn2(3)) /* not spammy */
                    You("find the enchanting music pleasant, but are otherwise unaffected.");
            } else {
                switch (chance) {
                case 1:
                    if (how_resistant(SLEEP_RES) == 100) {
                        monstseesu(M_SEEN_SLEEP);
                        You("yawn.");
                    } else {
                        You_feel("very drowsy.");
                        fall_asleep(-resist_reduce(rn2(5) + 2,
                                    SLEEP_RES), TRUE);
                    }
                    break;
                case 2:
                    if (!Confusion) { /* won't stack */
                        if (Hallucination)
                            pline("Like... wow, man!");
                        else
                            pline("Huh, What?  Where am I?");
                        make_confused((HConfusion & TIMEOUT)
                                      + (long) (rn2(5) + 2), FALSE);
                    }
                    break;
                case 3:
                    if (!Stunned) { /* won't stack */
                        make_stunned((HStun & TIMEOUT)
                                     + (long) (rn2(5) + 2), TRUE);
                        stop_occupation();
                    }
                    break;
                }
            }
            return 2;
        }
        return 0;
    case MUSE_POT_BOOZE:
        mquaffmsg(mtmp, otmp);
        if (vismon)
            pline("%s looks confused.", Monnam(mtmp));
        if (mtmp->mtame && mtmp->mtame < 20) {
            mtmp->mtame++;
        } else if (otmp->cursed) {
            if (vismon)
                pline("He flies into a drunken rage!");
            mtmp->mpeaceful = 0;
            mtmp->mberserk = 1;
        } else {
            if (vismon)
                pline("He is sated.");
            mtmp->mpeaceful = 1;
        }
        mtmp->mconf = 1;
        m_useup(mtmp, otmp);
        if (oseen)
            makeknown(POT_BOOZE);
        /* refresh symbol when changing from hostile to
           peaceful or vice versa */
        newsym(mtmp->mx, mtmp->my);
        return 2;
    case MUSE_SPELLBOOK: {
        int spell_level = objects[otmp->otyp].oc_level;
        int read_time;
        struct emsp *esp;

        /* Ensure monster has emsp for tracking reading state */
        if (!has_emsp(mtmp))
            newemsp(mtmp);
        esp = EMSP(mtmp);

        /* Check if monster is already reading this book */
        if (esp->msp_reading == otmp->otyp && esp->msp_read_turns > 0) {
            /* Continue reading - check for interruption */
            if (!mtmp->mcansee || mtmp->mstun || mtmp->mconf
                || helpless(mtmp)) {
                /* Reading interrupted */
                if (vismon)
                    pline("%s stops reading.", Monnam(mtmp));
                esp->msp_reading = 0;
                esp->msp_read_turns = 0;
                return 2;
            }

            /* Decrement reading time */
            esp->msp_read_turns--;

            if (esp->msp_read_turns <= 0) {
                /* Finished reading - learn the spell */
                boolean success = mlearn_spell(mtmp, otmp);

                esp->msp_reading = 0;
                esp->msp_read_turns = 0;

                if (vismon) {
                    if (success)
                        pline("%s finishes reading and looks more knowledgeable.",
                              Monnam(mtmp));
                    else
                        pline("%s finishes reading.", Monnam(mtmp));
                }

                /* Book goes blank after too many reads */
                if (otmp->spestudied >= MAX_SPELL_STUDY) {
                    if (vismon)
                        pline_The("spellbook fades.");
                    otmp->otyp = SPE_BLANK_PAPER;
                    set_material(otmp, PAPER);
                }
            }
            /* Still reading, spent a turn */
            return 2;
        }

        /* Reading time: 3 turns per spell level,
           blessed books are read in half the time */
        read_time = 3 * spell_level;
        if (otmp->blessed)
            read_time = (read_time + 1) / 2;

        /* Start reading */
        esp->msp_reading = otmp->otyp;
        esp->msp_read_turns = read_time;

        if (vismon)
            pline("%s begins reading %s.", Monnam(mtmp),
                  singular(otmp, doname));

        return 2;
    }
    case 0:
        return 0; /* i.e. an exploded wand */
    default:
        impossible("%s wanted to perform action %d?", Monnam(mtmp),
                   m.has_misc);
        break;
    }
    return 0;
}

void
you_aggravate(mtmp)
struct monst *mtmp;
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
        multi = -1;
        nomovemsg = "Aggravated, you are jolted into full consciousness.";
    }
    newsym(mtmp->mx, mtmp->my);
    if (!canspotmon(mtmp))
        map_invisible(mtmp->mx, mtmp->my);
}

int
rnd_misc_item(mtmp)
struct monst *mtmp;
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
STATIC_OVL boolean
necrophiliac(objlist, any_corpse)
struct obj *objlist;
boolean any_corpse;
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
searches_for_item(mon, obj)
struct monst *mon;
struct obj *obj;
{
    int typ = obj->otyp;

    if (is_animal(mon->data) || mindless(mon->data)
        || mon->data == &mons[PM_GHOST]) /* don't loot bones piles */
        return FALSE;

    if (typ == WAN_MAKE_INVISIBLE || typ == POT_INVISIBILITY
        || typ == RIN_INVISIBILITY || typ == RIN_LUSTROUS)
        return (boolean) (!mon->minvis && !mon->invis_blkd
                          && !attacktype(mon->data, AT_GAZE));
    if (typ == WAN_SPEED_MONSTER || typ == POT_SPEED)
        return (boolean) (mon->mspeed != MFAST);

    switch (obj->oclass) {
    case WAND_CLASS:
        if (obj->spe <= 0)
            return FALSE;
        if (typ == WAN_DIGGING)
            return (boolean) (!(is_floater(mon->data) || can_levitate(mon)));
        if (objects[typ].oc_dir == RAY || typ == WAN_STRIKING
            || typ == WAN_TELEPORTATION || typ == WAN_CREATE_MONSTER
            || typ == WAN_CANCELLATION || typ == WAN_WISHING
            || typ == WAN_POLYMORPH || typ == WAN_UNDEAD_TURNING
            || typ == WAN_SLOW_MONSTER)
            return TRUE;
        break;
    case POTION_CLASS:
        if (typ == POT_HEALING || typ == POT_EXTRA_HEALING
            || typ == POT_FULL_HEALING || typ == POT_POLYMORPH
            || typ == POT_GAIN_LEVEL || typ == POT_PARALYSIS
            || typ == POT_SLEEPING || typ == POT_ACID || typ == POT_CONFUSION
            || typ == POT_HALLUCINATION || typ == POT_OIL)
            return TRUE;
        if (typ == POT_BLINDNESS && !attacktype(mon->data, AT_GAZE))
            return TRUE;
        if (typ == POT_BOOZE && is_satyr(mon->data))
            return TRUE;
        break;
    case SCROLL_CLASS:
        if (typ == SCR_TELEPORTATION || typ == SCR_CREATE_MONSTER
            || typ == SCR_EARTH || typ == SCR_FIRE || typ == SCR_REMOVE_CURSE
            || (typ == SCR_STINKING_CLOUD && mon->mcansee)
            || typ == SCR_CHARGING)
            return TRUE;
        break;
    case AMULET_CLASS:
        if (typ == AMULET_OF_LIFE_SAVING)
            return (boolean) !(nonliving(mon->data) || is_vampshifter(mon));
        if (typ == AMULET_OF_REFLECTION
            || typ == AMULET_OF_FLYING
            || typ == AMULET_OF_GUARDING
            || typ == AMULET_OF_ESP
            || typ == AMULET_OF_MAGIC_RESISTANCE
            || obj->oartifact == ART_EYE_OF_THE_AETHIOPICA)
            return TRUE;
        /* who doesn't want the ultimate amulet? and they can be fooled also */
        if (typ == AMULET_OF_YENDOR || typ == FAKE_AMULET_OF_YENDOR)
            return (boolean) !mon_has_amulet(mon);
        break;
    case TOOL_CLASS:
        if (typ == PICK_AXE)
            return (boolean) racial_needspick(mon);
        if (typ == UNICORN_HORN)
            return (boolean) (!obj->cursed && !is_unicorn(mon->data)
                              && mon->data != &mons[PM_KI_RIN]
                              && mon->data != &mons[PM_ELDRITCH_KI_RIN]);
        if (typ == FROST_HORN || typ == FIRE_HORN)
            return (obj->spe > 0 && can_blow(mon));
        if (typ == PAN_FLUTE)
            return (boolean) !is_satyr(mon->data);
        if (typ == SKELETON_KEY || typ == LOCK_PICK
            || typ == CREDIT_CARD || typ == MAGIC_KEY)
            return TRUE;
        if ((typ == BAG_OF_HOLDING && !obj->cursed) || typ == OILSKIN_SACK
            || typ == SACK || (typ == BAG_OF_TRICKS && obj->spe > 0))
            return TRUE;
        if (typ == FIGURINE)
            return TRUE;
        if (typ == EXPENSIVE_CAMERA && obj->spe > 0)
            return TRUE;
        break;
    case FOOD_CLASS:
        if (typ == CORPSE && !obj->zombie_corpse)
            return (boolean) (((mon->misc_worn_check & W_ARMG) != 0L
                               && touch_petrifies(&mons[obj->corpsenm]))
                              || (!(resists_ston(mon) || defended(mon, AD_STON))
                                  && cures_stoning(mon, obj, FALSE)));
        if (typ == TIN)
            return (boolean) (mcould_eat_tin(mon)
                              && (!(resists_ston(mon) || defended(mon, AD_STON))
                                  && cures_stoning(mon, obj, TRUE)));
        if (typ == EGG)
            return (boolean) touch_petrifies(&mons[obj->corpsenm]);
        if (is_royaljelly(obj) && mon->data == &mons[PM_HONEY_BADGER])
            return TRUE;
        break;
    case RING_CLASS:
        /* Should match the list in m_dowear_type */
        if (typ == RIN_PROTECTION
            || typ == RIN_INCREASE_DAMAGE
            || typ == RIN_INCREASE_ACCURACY)
            return (obj->spe > 0);
        if (typ == RIN_SEE_INVISIBLE)
            return (!mon_prop(mon, SEE_INVIS));
        if (typ == RIN_FIRE_RESISTANCE)
            return (!(resists_fire(mon) || defended(mon, AD_FIRE)));
        if (typ == RIN_COLD_RESISTANCE)
            return (!(resists_cold(mon) || defended(mon, AD_COLD)));
        if (typ == RIN_SHOCK_RESISTANCE)
            return (!(resists_elec(mon) || defended(mon, AD_ELEC)));
        if (typ == RIN_POISON_RESISTANCE)
            return (!(resists_poison(mon) || defended(mon, AD_DRST)));
        if (typ == RIN_SLOW_DIGESTION)
            return (!mon_prop(mon, SLOW_DIGESTION));
        if (typ == RIN_REGENERATION)
            return (!mon_prop(mon, REGENERATION));
        if (typ == RIN_LEVITATION)
            return (grounded(mon->data));
        if (typ == RIN_FREE_ACTION || typ == RIN_ANCIENT)
            return TRUE;
        if (typ == RIN_INVISIBILITY || typ == RIN_LUSTROUS)
            return !(mon->minvis);
        if (typ == RIN_TELEPORTATION)
            return (!mon_prop(mon, TELEPORT));
        if (typ == RIN_TELEPORT_CONTROL)
            return (!mon_prop(mon, TELEPORT_CONTROL));
        break;
    default:
        break;
    }

    return FALSE;
}

boolean
mon_reflects(mon, str)
struct monst *mon;
const char *str;
{
    struct obj *orefl = which_armor(mon, W_ARMS);
    struct obj *brefl = which_armor(mon, W_BARDING);

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
               && orefl->otyp == ART_DRAGONBANE) {
        if (str) {
            pline(str, s_suffix(mon_nam(mon)), "gloves");
            makeknown(ART_DRAGONBANE);
        }
        return TRUE;
    } else if ((orefl = which_armor(mon, W_ARMG))
               && orefl->otyp == ART_GAUNTLETS_OF_PURITY) {
        if (str) {
            pline(str, s_suffix(mon_nam(mon)), "gauntlets");
            makeknown(ART_GAUNTLETS_OF_PURITY);
        }
        return TRUE;
    } else if (arti_reflects(MON_WEP(mon))) {
        /* due to wielded artifact weapon */
        if (str)
            pline(str, s_suffix(mon_nam(mon)), "weapon");
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
                   || Dragon_armor_to_scales(orefl) == CHROMATIC_DRAGON_SCALES)) {
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
        /* specifically for the monster spell MGC_REFLECTION */
        if (str)
            pline(str, s_suffix(mon_nam(mon)), "shimmering globe");
        return TRUE;
    }
    return FALSE;
}

boolean
ureflects(fmt, str)
const char *fmt, *str;
{
    /* Check from outermost to innermost objects */
    if (EReflecting & W_ARMS) {
        if (fmt && str) {
            pline(fmt, str, "shield");
            makeknown(SHIELD_OF_REFLECTION);
        }
        monstseesu(M_SEEN_REFL);
        return TRUE;
    } else if (EReflecting & W_ARMG) {
        /* Due to wearing the artifact Dragonbane */
        if (fmt && str)
            pline(fmt, str, "gloves");
        monstseesu(M_SEEN_REFL);
        return TRUE;
    } else if (HReflecting) {
        if (fmt && str)
            pline(fmt, str, "magical shield");
        monstseesu(M_SEEN_REFL);
        return TRUE;
    } else if (EReflecting & W_WEP) {
        /* Due to wielded artifact weapon */
        if (fmt && str)
            pline(fmt, str, "weapon");
        monstseesu(M_SEEN_REFL);
        return TRUE;
    } else if (EReflecting & W_AMUL) {
        if (fmt && str) {
            pline(fmt, str, "medallion");
            makeknown(AMULET_OF_REFLECTION);
        }
        monstseesu(M_SEEN_REFL);
        return TRUE;
    } else if (EReflecting & W_ARMC) {
        if (fmt && str)
            pline(fmt, str, "set of scales");
        monstseesu(M_SEEN_REFL);
        return TRUE;
    } else if (EReflecting & W_ARM) {
        if (fmt && str)
            pline(fmt, str, uskin ? "luster" : "armor");
        monstseesu(M_SEEN_REFL);
        return TRUE;
    } else if (EReflecting & W_ART) {
        /* Due to the Magic Mirror, which shows as W_ART */
        if (fmt && str)
            pline(fmt, str, "mirror");
        monstseesu(M_SEEN_REFL);
        return TRUE;
    } else if (youmonst.data == &mons[PM_SILVER_DRAGON]) {
        if (fmt && str)
            pline(fmt, str, "scales");
        monstseesu(M_SEEN_REFL);
        return TRUE;
    }
    return FALSE;
}

/* cure mon's blindness (use_defensive, dog_eat, meatobj) */
void
mcureblindness(mon, verbos)
struct monst *mon;
boolean verbos;
{
    if (!mon->mcansee) {
        mon->mcansee = 1;
        mon->mblinded = 0;
        if (verbos && haseyes(mon->data))
            pline("%s can see again.", Monnam(mon));
    }
}

/* TRUE if the monster ate something */
boolean
munstone(mon, by_you)
struct monst *mon;
boolean by_you;
{
    struct obj *obj;
    boolean tinok;

    if (resists_ston(mon) || defended(mon, AD_STON))
        return FALSE;
    if (mon->meating || !mon->mcanmove || mon->msleeping)
        return FALSE;
    mon->mstrategy &= ~STRAT_WAITFORU;

    if (is_spellcaster(mon) && !mon->mcan
        && !mon->mspec_used && !mon->mconf
        && mon->m_lev >= 5) {
        struct obj *otmp, *onext, *pseudo;

        pseudo = mksobj(SPE_STONE_TO_FLESH, FALSE, FALSE);
        pseudo->blessed = pseudo->cursed = 0;
        mon->mspec_used = mon->mspec_used + rn2(7);
        if (canspotmon(mon))
            pline("%s casts a spell!", canspotmon(mon)
                  ? Monnam(mon) : Something);
        if (canspotmon(mon)) {
            if (Hallucination)
                pline("Look!  The Pillsbury Doughboy!");
            else
                pline("%s seems limber!", Monnam(mon));
        }

        for (otmp = mon->minvent; otmp; otmp = onext) {
            onext = otmp->nobj;
            mon->misc_worn_check &= ~otmp->owornmask;
            update_mon_intrinsics(mon, otmp, FALSE, TRUE);
            /* Clear weapon pointer if we're about to clear W_WEP */
            if ((otmp->owornmask & W_WEP) && MON_WEP(mon) == otmp)
                MON_NOWEP(mon);
            otmp->owornmask = 0L; /* obfree() expects this */
            (void) bhito(otmp, pseudo);
        }
        obfree(pseudo, (struct obj *) 0);
        mon->mlstmv = monstermoves; /* it takes a turn */
        return TRUE;
    }

    tinok = mcould_eat_tin(mon);
    for (obj = mon->minvent; obj; obj = obj->nobj) {
        if (cures_stoning(mon, obj, tinok)) {
            mon_consume_unstone(mon, obj, by_you, TRUE);
            return TRUE;
        }
    }
    return FALSE;
}

STATIC_OVL void
mon_consume_unstone(mon, obj, by_you, stoning)
struct monst *mon;
struct obj *obj;
boolean by_you;
boolean stoning; /* True: stop petrification, False: cure stun && confusion */
{
    boolean vis = canseemon(mon), tinned = obj->otyp == TIN,
            food = obj->otyp == CORPSE || tinned,
            acid = obj->otyp == POT_ACID
                   || (food && acidic(&mons[obj->corpsenm])),
            lizard = food && obj->corpsenm == PM_LIZARD,
            leaf = obj->otyp == EUCALYPTUS_LEAF;
    int nutrit = food ? dog_nutrition(mon, obj) : 0; /* also sets meating */
    char *distantname = distant_name(obj, doname);

    /* give a "<mon> is slowing down" message and also remove
       intrinsic speed (comparable to similar effect on the hero) */

    if (vis) {
        long save_quan = obj->quan;

        obj->quan = 1L;
        pline("%s %s %s.", Monnam(mon),
              (obj->oclass == POTION_CLASS)
                  ? "quaffs"
                  : (obj->otyp == TIN) ? "opens and eats the contents of"
                                       : "eats",
              distantname);
        obj->quan = save_quan;
    } else if (!Deaf)
        You_hear("%s.",
                 (obj->oclass == POTION_CLASS) ? "drinking" : "chewing");

    m_useup(mon, obj);
    /* obj is now gone */

    if (acid && !tinned && !(resists_acid(mon) || defended(mon, AD_ACID))) {
        damage_mon(mon, rnd(15), AD_ACID, FALSE);
        if (vis)
            pline("%s has a very bad case of stomach acid.", Monnam(mon));
        if (DEADMONSTER(mon)) {
            pline("%s dies!", Monnam(mon));
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
    if (stoning) {
        mon->mstone = 0;
        if (!vis) {
            ; /* no feedback */
        } else if (Hallucination) {
            pline("What a pity - %s just ruined a future piece of art!",
                  mon_nam(mon));
        } else {
            pline("%s seems limber!", Monnam(mon));
        }
    }
    if (lizard && (mon->mconf || mon->mstun)) {
        mon->mconf = 0;
        mon->mstun = 0;
        if (vis && !is_bat(mon->data) && mon->data != &mons[PM_STALKER])
            pline("%s seems steadier now.", Monnam(mon));
    }
    if (leaf && (mon->msick || mon->mdiseased)) {
        mon->msick = 0;
        mon->mdiseased = 0;
        if (vis)
            pline("%s is no longer ill.", Monnam(mon));
    }
    if (mon->mtame && !mon->isminion && nutrit > 0) {
        struct edog *edog = EDOG(mon);

        if (edog->hungrytime < monstermoves)
            edog->hungrytime = monstermoves;
        edog->hungrytime += nutrit;
        mon->mconf = 0;
    }
    /* use up monster's next move */
    mon->movement -= NORMAL_SPEED;
    mon->mlstmv = monstermoves;
}

/* decide whether obj can cure petrification; also used when picking up */
boolean
cures_stoning(mon, obj, tinok)
struct monst *mon;
struct obj *obj;
boolean tinok;
{
    if (obj->otyp == POT_ACID)
        return TRUE;
    if (obj->otyp != CORPSE && (obj->otyp != TIN || !tinok))
        return FALSE;
    /* corpse, or tin that mon can open */
    if (obj->corpsenm == NON_PM) /* empty/special tin */
        return FALSE;
    return (boolean) (obj->corpsenm == PM_LIZARD
                      || (acidic(&mons[obj->corpsenm])
                          && (obj->corpsenm != PM_GREEN_SLIME
                              || slimeproof(mon->data))));
}

STATIC_OVL boolean
mcould_eat_tin(mon)
struct monst *mon;
{
    struct obj *obj, *mwep;
    boolean welded_wep;

    /* monkeys who manage to steal tins can't open and eat them
       even if they happen to also have the appropriate tool */
    if (is_animal(mon->data))
        return FALSE;

    mwep = MON_WEP(mon);
    welded_wep = (mwep && mwelded(mwep) && mon->data != &mons[PM_INFIDEL]);
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
munslime(mon, by_you)
struct monst *mon;
boolean by_you;
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
    if (mon->meating || !mon->mcanmove || mon->msleeping)
        return FALSE;
    mon->mstrategy &= ~STRAT_WAITFORU;

    /* if monster can breathe fire, do so upon self; a monster who deals
       fire damage by biting, clawing, gazing, and especially exploding
       isn't able to cure itself of green slime with its own attack
       [possible extension: monst capable of casting high level clerical
       spells could toss pillar of fire at self--probably too suicidal] */
    if (!mon->mcan && !mon->mspec_used
        && attacktype_fordmg(mptr, AT_BREA, AD_FIRE)) {
        odummy = zeroobj; /* otyp == STRANGE_OBJECT */
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
            int xy[2][8], x, y, idx, ridx, nxy = 0;

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
            return muse_unslime(mon, (struct obj *) &zeroobj, t, by_you);

    } /* MUSE */

    return FALSE;
}

/* mon uses an item--selected by caller--to burn away incipient slime */
STATIC_OVL boolean
muse_unslime(mon, obj, trap, by_you)
struct monst *mon;
struct obj *obj;
struct trap *trap;
boolean by_you; /* true: if mon kills itself, hero gets credit/blame */
{               /* [by_you not honored if 'mon' triggers fire trap]. */
    struct obj *odummyp;
    int otyp = obj->otyp, dmg = 0;
    boolean vis = canseemon(mon), res = TRUE;

    if (vis)
        pline("%s starts turning %s.", Monnam(mon),
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
            newsym(mon->mx, mon->my);
            if (vis)
                pline("%s %s %s %s fire trap!", Mnam,
                      vtense(fakename[0], locomotion(mon->data, "move")),
                      (is_floater(mon->data) || can_levitate(mon))
                          ? "over" : "onto",
                      trap->tseen ? "the" : "a");
        }
        /* hack to avoid mintrap()'s chance of avoiding known trap */
        mon->mtrapseen &= ~(1 << (FIRE_TRAP_SET - 1));
        mintrap(mon);
    } else if (otyp == STRANGE_OBJECT) {
        /* monster is using fire breath on self */
        if (vis)
            pline("%s breathes fire on %sself.", Monnam(mon), mhim(mon));
        if (!rn2(3))
            mon->mspec_used = rn1(10, 5);
        /* 1 => # of damage dice */
        dmg = zhitm(mon, by_you ? ZT_BREATH(ZT_FIRE) : -ZT_BREATH(ZT_FIRE),
                    1, &odummyp);
    } else if (otyp == SCR_FIRE) {
        mreadmsg(mon, obj);
        if (mon->mconf) {
            if (cansee(mon->mx, mon->my))
                pline("Oh, what a pretty fire!");
            if (vis && !objects[otyp].oc_name_known
                && !objects[otyp].oc_uname)
                docall(obj);
            m_useup(mon, obj); /* after docall() */
            vis = FALSE;       /* skip makeknown() below */
            res = FALSE;       /* failed to cure sliming */
        } else {
            dmg = (2 * (rn1(3, 3) + 2 * bcsign(obj)) + 1) / 3;
            m_useup(mon, obj); /* before explode() */
            explode(mon->mx, mon->my, -ZT_SPELL(ZT_FIRE), dmg, SCROLL_CLASS,
                    by_you ? -EXPL_FIERY : EXPL_FIERY);
            dmg = 0; /* damage has been applied by explode() */
        }
    } else { /* wand/horn of fire w/ positive charge count */
        if (obj->otyp == FIRE_HORN)
            mplayhorn(mon, obj, TRUE);
        else
            mzapwand(mon, obj, TRUE);
        /* 2 => # of damage dice */
        dmg = zhitm(mon, by_you ? ZT_WAND(ZT_FIRE) : -ZT_WAND(ZT_FIRE),
                    2, &odummyp);
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
                    pline("%s is %s by the fire!", Monnam(mon),
                          nonliving(mon->data) ? "destroyed" : "killed");
                xkilled(mon, XKILL_NOMSG | XKILL_NOCONDUCT);
            } else
                monkilled(mon, "fire", AD_FIRE);
        } else {
            /* non-fatal damage occurred */
            if (vis)
                pline("%s is burned%s", Monnam(mon), exclam(dmg));
        }
    }
    if (vis) {
        if (res && !DEADMONSTER(mon))
            pline("%s slime is burned away!", s_suffix(Monnam(mon)));
        if (otyp != STRANGE_OBJECT)
            makeknown(otyp);
    }
    /* use up monster's next move */
    mon->movement -= NORMAL_SPEED;
    mon->mlstmv = monstermoves;
    return res;
}

/* decide whether obj can be used to cure green slime */
STATIC_OVL int
cures_sliming(mon, obj)
struct monst *mon;
struct obj *obj;
{
    /* scroll of fire, non-empty wand or horn of fire */
    if (obj->otyp == SCR_FIRE)
        return (haseyes(mon->data) && mon->mcansee);
    /* hero doesn't need hands or even limbs to zap, so mon doesn't either */
    return ((obj->otyp == WAN_FIRE
             || (obj->otyp == FIRE_HORN && can_blow(mon)))
            && obj->spe > 0);
}

/* TRUE if monster appears to be green; for active TEXTCOLOR, we go by
   the display color, otherwise we just pick things that seem plausibly
   green (which doesn't necessarily match the TEXTCOLOR categorization) */
STATIC_OVL boolean
green_mon(mon)
struct monst *mon;
{
    struct permonst *ptr = mon->data;

    if (Hallucination)
        return FALSE;
#ifdef TEXTCOLOR
    if (iflags.use_color)
        return (ptr->mcolor == CLR_GREEN || ptr->mcolor == CLR_BRIGHT_GREEN);
#endif
    /* approximation */
    if (strstri(ptr->mname, "green"))
        return TRUE;
    switch (monsndx(ptr)) {
    case PM_BASILISK:
    case PM_CAUCHEMAR:
    case PM_FOREST_CENTAUR:
    case PM_GARTER_SNAKE:
    case PM_GECKO:
    case PM_GIANT_ANACONDA:
    case PM_GIANT_COCKROACH:
    case PM_GIANT_TURTLE:
    case PM_GOBLIN_SHAMAN:
    case PM_GREATER_HOMUNCULUS:
    case PM_GREMLIN:
    case PM_HOMUNCULUS:
    case PM_JUIBLEX:
    case PM_LEPRECHAUN:
    case PM_LESSER_HOMUNCULUS:
    case PM_LESSER_NIGHTMARE:
    case PM_LICHEN:
    case PM_LIZARD:
    case PM_NIGHTMARE:
    case PM_WOOD_NYMPH:
        return TRUE;
    default:
        if (racial_elf(mon) && !is_prince(ptr) && !is_lord(ptr)
            && ptr != &mons[PM_GREY_ELF])
            return TRUE;
        break;
    }
    return FALSE;
}

/* The monster chooses what to wish for. This code is based off the code in
   makemon.c - there are checks in place to help make sure a monster doesn't
   wish for something they can't use (e.g. a giant wishing for dragon scale,
   or a demon with horns on its head wishing for a helmet, or wishing for
   reflection when they are already reflecting, etc). This could be improved
   upon (such as checking to see if the monster making a wish already has the
   item they are wishing for), but since the number of wishes a monster will
   ever get to use will most likely never be greater than three, that may not
   be necessary */
void
mmake_wish(mon)
struct monst *mon;
{
    struct obj *otmp;
    char *oname;
    boolean wearable = FALSE;
    otmp = NULL;

    switch (rnd(18)) {
    case 1:
        otmp = mksobj(POT_GAIN_LEVEL, FALSE, FALSE);
        bless(otmp);
        otmp->quan = rnd(3);
        otmp->owt = weight(otmp);
        break;
    case 2:
        switch (rnd(4)) {
        case 1:
            otmp = mksobj(FIGURINE, FALSE, FALSE);
            otmp->corpsenm = PM_ARCHON;
            bless(otmp);
            break;
        case 2:
            otmp = mksobj(FIGURINE, FALSE, FALSE);
            otmp->corpsenm = PM_BALROG;
            bless(otmp);
            break;
        case 3:
            otmp = mksobj(FIGURINE, FALSE, FALSE);
            otmp->corpsenm = PM_VORPAL_JABBERWOCK;
            bless(otmp);
            break;
        case 4:
            otmp = mksobj(FIGURINE, FALSE, FALSE);
            otmp->corpsenm = PM_WOODCHUCK;
            bless(otmp);
            break;
        }
        break;
    case 3:
        otmp = mksobj(AMULET_OF_LIFE_SAVING, FALSE, FALSE);
        bless(otmp);
        maybe_erodeproof(otmp, 1);
        wearable = TRUE;
        break;
    case 4:
        if (!has_displacement(mon) && !cantweararm(mon)) {
            otmp = mksobj(CLOAK_OF_DISPLACEMENT, FALSE, FALSE);
            bless(otmp);
            maybe_erodeproof(otmp, 1);
            otmp->spe = rn2(3) + 1;
            wearable = TRUE;
        } else {
            otmp = mksobj(WAN_DIGGING, TRUE, FALSE);
        }
        break;
    case 5:
        if (!nohands(mon->data)) {
            otmp = mksobj(GAUNTLETS_OF_DEXTERITY, FALSE, FALSE);
            bless(otmp);
            maybe_erodeproof(otmp, 1);
            otmp->spe = rn2(3) + 1;
            wearable = TRUE;
        } else {
            otmp = mksobj(POT_FULL_HEALING, FALSE, FALSE);
            bless(otmp);
        }
        break;
    case 6:
        if (!(resists_magm(mon) || defended(mon, AD_MAGM))) {
            if (!cantweararm(mon))
                otmp = mksobj(CLOAK_OF_MAGIC_RESISTANCE, FALSE, FALSE);
            else
                otmp = mksobj(AMULET_OF_MAGIC_RESISTANCE, FALSE, FALSE);
            bless(otmp);
            maybe_erodeproof(otmp, 1);
            otmp->spe = rn2(3) + 1;
            wearable = TRUE;
        } else {
            otmp = mksobj(WAN_CREATE_MONSTER, TRUE, FALSE);
            maybe_erodeproof(otmp, 1);
        }
        break;
    case 7:
        if (!cantweararm(mon)) {
            otmp = mksobj(CRYSTAL_PLATE_MAIL, FALSE, FALSE);
            bless(otmp);
            maybe_erodeproof(otmp, 1);
            otmp->spe = rn2(3) + 1;
            wearable = TRUE;
        } else {
            otmp = mksobj(BAG_OF_HOLDING, FALSE, FALSE);
            bless(otmp);
            maybe_erodeproof(otmp, 1);
        }
        break;
    case 8:
        if (!is_flyer(mon->data)) {
            otmp = mksobj(AMULET_OF_FLYING, FALSE, FALSE);
            bless(otmp);
            maybe_erodeproof(otmp, 1);
            wearable = TRUE;
        } else {
            otmp = mksobj(POT_FULL_HEALING, FALSE, FALSE);
            bless(otmp);
        }
        break;
    case 9:
        if (!can_wwalk(mon) && !sliparm(mon)) {
            otmp = mksobj(WATER_WALKING_BOOTS, FALSE, FALSE);
            bless(otmp);
            maybe_erodeproof(otmp, 1);
            otmp->spe = rn2(3) + 1;
            wearable = TRUE;
        } else {
            otmp = mksobj(RIN_SLOW_DIGESTION, FALSE, FALSE);
            bless(otmp);
            maybe_erodeproof(otmp, 1);
        }
        break;
    case 10:
        if (!mon_reflects(mon, (char *) 0)) {
            if (humanoid(mon->data))
                otmp = mksobj(SHIELD_OF_REFLECTION, FALSE, FALSE);
            else
                otmp = mksobj(AMULET_OF_REFLECTION, FALSE, FALSE);
            bless(otmp);
            maybe_erodeproof(otmp, 1);
            otmp->spe = rn2(3) + 1;
            wearable = TRUE;
        } else {
            otmp = mksobj(POT_EXTRA_HEALING, FALSE, FALSE);
            bless(otmp);
        }
        break;
    case 11:
        if (!nohands(mon->data) && !strongmonst(mon->data)) {
            otmp = mksobj(GAUNTLETS_OF_POWER, FALSE, FALSE);
            bless(otmp);
            maybe_erodeproof(otmp, 1);
            otmp->spe = rn2(3) + 1;
            wearable = TRUE;
        } else {
            otmp = mksobj(POT_GAIN_LEVEL, FALSE, FALSE);
            curse(otmp);
        }
        break;
    case 12:
        if (mon->mspeed != MFAST && !sliparm(mon)) {
            otmp = mksobj(SPEED_BOOTS, FALSE, FALSE);
            bless(otmp);
            maybe_erodeproof(otmp, 1);
            otmp->spe = rn2(3) + 1;
            wearable = TRUE;
        } else {
            otmp = mksobj(POT_FULL_HEALING, FALSE, FALSE);
            bless(otmp);
        }
        break;
    case 13:
        if (!has_telepathy(mon)
            && !sliparm(mon) && !has_horns(mon->data)) {
            otmp = mksobj(HELM_OF_TELEPATHY, FALSE, FALSE);
            bless(otmp);
            maybe_erodeproof(otmp, 1);
            otmp->spe = rn2(3) + 1;
            wearable = TRUE;
        } else {
            otmp = mksobj(POT_EXTRA_HEALING, FALSE, FALSE);
            bless(otmp);
        }
        break;
    case 14:
        if (!sliparm(mon) && !has_horns(mon->data)) {
            otmp = mksobj(HELM_OF_BRILLIANCE, FALSE, FALSE);
            bless(otmp);
            maybe_erodeproof(otmp, 1);
            otmp->spe = rn2(3) + 1;
            wearable = TRUE;
        } else {
            otmp = mksobj(UNICORN_HORN, FALSE, FALSE);
            bless(otmp);
            maybe_erodeproof(otmp, 1);
            otmp->spe = rn2(3) + 1;
        }
        break;
    case 15:
        if (!cantweararm(mon)) {
            otmp = mksobj(CLOAK_OF_PROTECTION, FALSE, FALSE);
            bless(otmp);
            maybe_erodeproof(otmp, 1);
            otmp->spe = rn2(3) + 1;
            wearable = TRUE;
        } else {
            otmp = mksobj(RIN_PROTECTION, FALSE, FALSE);
            bless(otmp);
            maybe_erodeproof(otmp, 1);
            otmp->spe = rn2(3) + 1;
        }
        break;
    case 16: /* Monsters can wish for certain artifacts */
        otmp = mk_artifact((struct obj *) 0, mon_aligntyp(mon));
        if (otmp) {
            bless(otmp);
            maybe_erodeproof(otmp, 1);
            otmp->spe = rn2(3) + 1;
            wearable = TRUE;
        }
        break;
    case 17:
        switch (rnd(3)) {
        case 1:
            otmp = mksobj(WAN_CANCELLATION, TRUE, FALSE);
            break;
        case 2:
            otmp = mksobj(WAN_POLYMORPH, TRUE, FALSE);
            break;
        case 3:
            otmp = mksobj(WAN_DEATH, TRUE, FALSE);
            break;
        }
        maybe_erodeproof(otmp, 1);
        break;
    case 18:
        otmp = mksobj(EGG, FALSE, FALSE);
        otmp->corpsenm = PM_COCKATRICE;
        otmp->quan = rnd(3);
        otmp->owt = weight(otmp);
        break;
    default:
        otmp = mksobj(POT_GAIN_LEVEL, FALSE, FALSE);
        curse(otmp);
    }

    if (otmp == NULL) {
        if (canseemon(mon))
            pline("For a moment, %s had %s in its %s, but it disappears.",
                  mon_nam(mon), something, makeplural(mbodypart(mon, HAND)));
        return;
    }

    oname = ansimpleoname(otmp);
    if (canseemon(mon))
        pline("%s makes a wish for%s %s!",
              Monnam(mon), (otmp->quan > 1) ? " some" : "", oname);

    if (mpickobj(mon, otmp))
        otmp = NULL;

    if (wearable)
        check_gear_next_turn(mon);
}

/* Monster casts a ray spell (magic missile, fireball, cone of cold,
   sleep, finger of death, lightning, poison blast, acid blast).
   spell_otyp is the spell object type (SPE_MAGIC_MISSILE through
   SPE_ACID_BLAST). Returns TRUE if cast successfully, FALSE otherwise */
boolean
mcast_ray_spell(caster, tx, ty, spell_otyp)
struct monst *caster;
int tx, ty;
int spell_otyp;
{
    int dx, dy;
    int nd; /* number of damage/effect dice */
    int ztype;

    /* Validate spell is in the ray spell range */
    if (spell_otyp < SPE_MAGIC_MISSILE || spell_otyp > SPE_ACID_BLAST) {
        impossible("mcast_ray_spell: invalid spell %d", spell_otyp);
        return FALSE;
    }

    /* Calculate direction from caster to target */
    dx = tx - caster->mx;
    dy = ty - caster->my;

    /* Sanity check - can't cast at own location */
    if (!dx && !dy)
        return FALSE;

    /* Damage/effect dice: scale with caster level like player spell
       Player uses (u.ulevel / 2) + 1 */
    nd = (caster->m_lev / 2) + 1;
    if (nd < 1)
        nd = 1;

    /* Calculate the ZT type from spell otyp
       SPE_MAGIC_MISSILE maps to ZT_SPELL(0), SPE_FIREBALL to
       ZT_SPELL(1), etc */
    ztype = ZT_SPELL(spell_otyp - SPE_MAGIC_MISSILE);

    /* Cast the ray - negative type indicates monster casting */
    buzz(-ztype, nd, caster->mx, caster->my, sgn(dx), sgn(dy));

    return TRUE;
}

/* Cast an IMMEDIATE spell using mbhit() ray tracing.
   These spells (drain life, slow monster, teleport away, polymorph,
   turn undead, entangle, dispel evil, charm monster) trace a line from
   caster to target and affect things along the path */
void
mcast_immediate_spell(caster, tx, ty, spell_otyp)
struct monst *caster;
int tx, ty;
int spell_otyp;
{
    struct obj *pseudo;

    /* Set direction from caster to target (tbx/tby are used by mbhit) */
    tbx = tx - caster->mx;
    tby = ty - caster->my;

    /* Sanity check - can't cast at own location */
    if (!tbx && !tby)
        return;

    /* Create a pseudo-object representing the spell */
    pseudo = mksobj(spell_otyp, FALSE, FALSE);
    pseudo->blessed = pseudo->cursed = 0;
    pseudo->quan = 20L; /* prevent useup from freeing it */
    /* Set carrier so mbhitm can access caster level for damage scaling
       and caster position for various effects */
    pseudo->where = OBJ_MINVENT;
    pseudo->ocarry = caster;

    /* Trace the ray and affect targets along the path */
    m_using = TRUE;
    mbhit(caster, rn1(8, 6), mbhitm, bhito, pseudo);
    m_using = FALSE;

    /* Clean up the pseudo-object */
    pseudo->where = OBJ_FREE; /* prevent obfree complaints */
    pseudo->ocarry = (struct monst *) 0;
    obfree(pseudo, (struct obj *) 0);
}

/*muse.c*/
