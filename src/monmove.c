/* NetHack 3.6	monmove.c	$NHDT-Date: 1575245074 2019/12/02 00:04:34 $  $NHDT-Branch: NetHack-3.6 $:$NHDT-Revision: 1.116 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Michael Allison, 2006. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "mfndpos.h"
#include "artifact.h"

extern boolean notonhead;

STATIC_DCL void FDECL(watch_on_duty, (struct monst *));
STATIC_DCL int FDECL(disturb, (struct monst *));
STATIC_DCL void FDECL(release_hero, (struct monst *));
STATIC_DCL void FDECL(distfleeck, (struct monst *, int *, int *, int *));
STATIC_DCL int FDECL(m_arrival, (struct monst *));
STATIC_DCL boolean FDECL(stuff_prevents_passage, (struct monst *));
STATIC_DCL int FDECL(vamp_shift, (struct monst *, struct permonst *,
                                  BOOLEAN_P));
STATIC_DCL boolean FDECL(likes_contents, (struct monst *, struct obj *));

/* True if mtmp died */
boolean
mb_trapped(mtmp)
struct monst *mtmp;
{
    if (flags.verbose) {
        if (cansee(mtmp->mx, mtmp->my) && !Unaware)
            pline("KABOOM!!  You see a door explode.");
        else if (!Deaf)
            You_hear("a distant explosion.");
    }
    wake_nearto(mtmp->mx, mtmp->my, 7 * 7);
    if (!(resists_stun(mtmp->data) || defended(mtmp, AD_STUN)
          || (MON_WEP(mtmp)
              && MON_WEP(mtmp)->oartifact == ART_TEMPEST)))
        mtmp->mstun = 1;
    damage_mon(mtmp, rnd(15), AD_PHYS, FALSE);
    if (DEADMONSTER(mtmp)) {
        mondied(mtmp);
        if (!DEADMONSTER(mtmp)) /* lifesaved */
            return FALSE;
        else
            return TRUE;
    }
    return FALSE;
}

/* check whether a monster is carrying a locking/unlocking tool */
boolean
monhaskey(mon, for_unlocking)
struct monst *mon;
boolean for_unlocking; /* true => credit card ok, false => not ok */
{
    if (for_unlocking && m_carrying(mon, CREDIT_CARD))
        return TRUE;
    return m_carrying(mon, SKELETON_KEY) || m_carrying(mon, LOCK_PICK);
}

void
mon_yells(mon, shout)
struct monst *mon;
const char *shout;
{
    if (Deaf) {
        if (canspotmon(mon))
            /* Sidenote on "A watchman angrily waves her arms!"
             * Female being called watchman is correct (career name).
             */
            pline("%s angrily %s %s %s!",
                Amonnam(mon),
                nolimbs(mon->data) ? "shakes" : "waves",
                mhis(mon),
                nolimbs(mon->data) ? mbodypart(mon, HEAD)
                                   : makeplural(mbodypart(mon, ARM)));
    } else {
        if (canspotmon(mon))
            pline("%s yells:", Amonnam(mon));
        else
            You_hear("someone yell:");
        verbalize1(shout);
    }
}

/* can monster mtmp break boulders? */
boolean
m_can_break_boulder(mtmp)
struct monst *mtmp;
{
    return (!mtmp->mpeaceful
            && (is_rider(mtmp->data)
                || (MON_WEP(mtmp) && is_pick(MON_WEP(mtmp)))
                || (!mtmp->mbreakboulder
                    && (is_dprince(mtmp->data)
                        || is_dlord(mtmp->data)
                        || mtmp->isshk
                        || mtmp->ispriest
                        || mtmp->data->msound == MS_LEADER
                        || mtmp->data->msound == MS_NEMESIS
                        || mtmp->data == &mons[PM_ORACLE]))));
}

/* monster mtmp breaks boulder at x, y */
void
m_break_boulder(mtmp, x, y)
struct monst *mtmp;
xchar x, y;
{
    struct obj *otmp;
    boolean using_pick = (MON_WEP(mtmp) && is_pick(MON_WEP(mtmp)));

    if (m_can_break_boulder(mtmp)
        && ((otmp = sobj_at(BOULDER, x, y)) != 0)) {
        if (distu(mtmp->mx, mtmp->my) < 4 * 4) {
            if (using_pick) {
                if (cansee(x, y))
                    pline("%s swings %s %s.",
                          Monnam(mtmp), mhis(mtmp),
                          simpleonames(MON_WEP(mtmp)));
                else if (!Deaf)
                    You_hear("a crumbling sound.");
            } else {
                if (!Deaf)
                    pline("%s %s %s.",
                          Monnam(mtmp),
                          rn2(2) ? "mutters" : "whispers",
                          mtmp->ispriest ? "a prayer"
                                         : "an incantation");
            }
        }
        if (!is_rider(mtmp->data)) {
            if (unique_corpstat(mtmp->data))
                mtmp->mbreakboulder += rn1(4, 2);
            else
                mtmp->mbreakboulder += rn1(20, 10);
        }
        if (cansee(x, y))
            pline_The("boulder falls apart.");
        fracture_rock(otmp);
    }
}

STATIC_OVL void
watch_on_duty(mtmp)
struct monst *mtmp;
{
    int x, y;

    if (mtmp->mpeaceful && in_town(u.ux + u.dx, u.uy + u.dy)
        && mtmp->mcansee && m_canseeu(mtmp) && !rn2(3)) {
        if (Race_if(PM_DRAUGR) && !Upolyd) {
            mon_yells(mtmp, "Another zombie!  Attack!");
            (void) angry_guards(!!Deaf);
            stop_occupation();
            return;
        } else if (Role_if(PM_CONVICT) && !Upolyd) {
            mon_yells(mtmp, "Hey, you're the one from the wanted poster!");
            (void) angry_guards(!!Deaf);
            stop_occupation();
            return;
        }
        if (picking_lock(&x, &y) && IS_DOOR(levl[x][y].typ)
            && (levl[x][y].doormask & D_LOCKED)) {
            if (couldsee(mtmp->mx, mtmp->my)) {
                if (levl[x][y].looted & D_WARNED) {
                    mon_yells(mtmp, "Halt, thief!  You're under arrest!");
                    (void) angry_guards(!!Deaf);
                } else {
                    mon_yells(mtmp, "Hey, stop picking that lock!");
                    levl[x][y].looted |= D_WARNED;
                }
                stop_occupation();
            }
        } else if (is_digging()) {
            /* chewing, wand/spell of digging are checked elsewhere */
            watch_dig(mtmp, context.digging.pos.x, context.digging.pos.y,
                      FALSE);
        }
    }
}

int
dochugw(mtmp)
struct monst *mtmp;
{
    int x = mtmp->mx, y = mtmp->my;
    boolean already_saw_mon = !occupation ? 0 : canspotmon(mtmp);
    int rd = dochug(mtmp);

    /* a similar check is in monster_nearby() in hack.c */
    /* check whether hero notices monster and stops current activity */
    if (occupation && !rd
        /* monster is hostile and can attack (or hallu distorts knowledge) */
        && (Hallucination || (!mtmp->mpeaceful && !noattacks(mtmp->data)))
        /* it's close enough to be a threat */
        && distu(mtmp->mx, mtmp->my) <= (BOLT_LIM + 1) * (BOLT_LIM + 1)
        /* and either couldn't see it before, or it was too far away */
        && (!already_saw_mon || !couldsee(x, y)
            || distu(x, y) > (BOLT_LIM + 1) * (BOLT_LIM + 1))
        /* can see it now, or sense it and would normally see it */
        && canspotmon(mtmp) && couldsee(mtmp->mx, mtmp->my)
        /* monster isn't paralyzed or afraid (scare monster/Elbereth) */
        && mtmp->mcanmove && !onscary(u.ux, u.uy, mtmp))
        stop_occupation();

    return rd;
}

boolean
onscary(x, y, mtmp)
int x, y;
struct monst *mtmp;
{
    /* creatures who are directly resistant to magical scaring:
     * Rodney, lawful minions, Angels, Archangels, the Riders,
     * Vecna, the Goblin King, monster players, demon lords and princes,
     * honey badgers, wolverines, shopkeepers inside their own shop,
     * anything that is mindless, priests inside their own temple, the
     * quest leaders and nemesis, neothelids, beholders, other unique creatures
     */
    if (mtmp->iswiz || is_lminion(mtmp) || mtmp->data == &mons[PM_ANGEL]
        || mtmp->data == &mons[PM_ARCHANGEL] || mtmp->data == &mons[PM_HONEY_BADGER]
        || mtmp->data == &mons[PM_BEHOLDER] || mtmp->data == &mons[PM_NEOTHELID]
        || mtmp->data == &mons[PM_WOLVERINE] || mtmp->data == &mons[PM_DIRE_WOLVERINE]
        || mindless(mtmp->data) || is_mplayer(mtmp->data) || is_rider(mtmp->data)
        || mtmp->isvecna || mtmp->isvlad || mtmp->isgking || mtmp->istalgath
        || mtmp->data->mlet == S_HUMAN || unique_corpstat(mtmp->data)
        || (mtmp->isshk && inhishop(mtmp))
        || (mtmp->ispriest && inhistemple(mtmp))
        || mtmp->ismichael || mtmp->mberserk)
        return FALSE;

    /* <0,0> is used by musical scaring to check for the above;
     * it doesn't care about scrolls or engravings or dungeon branch */
    if (x == 0 && y == 0)
        return TRUE;

    /* should this still be true for defiled/molochian altars? */
    if (IS_ALTAR(levl[x][y].typ)
        && (racial_vampire(mtmp) || is_vampshifter(mtmp)))
        return TRUE;

    /* Conflicted monsters ignore scary things on the floor. */
    if (Conflict)
        return FALSE;

    /* the scare monster scroll doesn't have any of the below
     * restrictions, being its own source of power */
    if (sobj_at(SCR_SCARE_MONSTER, x, y))
        return TRUE;

    /*
     * Creatures who don't (or can't) fear a written Elbereth:
     * all the above plus shopkeepers (even if poly'd into non-human),
     * vault guards (also even if poly'd), blind or peaceful monsters,
     * humans and elves, honey badgers, wolverines, and minotaurs.
     *
     * If the player isn't actually on the square OR the player's image
     * isn't displaced to the square, no protection is being granted.
     *
     * Elbereth doesn't work in Gehennom, the Elemental Planes, or the
     * Astral Plane; the influence of the Valar only reaches so far.
     */
    return (sengr_at("Elbereth", x, y, TRUE)
            && ((u.ux == x && u.uy == y)
                || (Displaced && mtmp->mux == x && mtmp->muy == y))
            && !(mtmp->isshk || mtmp->isgd || !mtmp->mcansee
                 || mtmp->mpeaceful || mtmp->data->mlet == S_HUMAN
                 || mtmp->data == &mons[PM_MINOTAUR]
                 || mtmp->data == &mons[PM_ELDER_MINOTAUR]
                 || mtmp->data == &mons[PM_NEOTHELID]
                 || unique_corpstat(mtmp->data)
                 || Inhell || In_endgame(&u.uz)));
}

/* regenerate lost hit points */
void
mon_regen(mon, digest_meal)
struct monst *mon;
boolean digest_meal;
{
    boolean mon_orcus_wield = (MON_WEP(mon)
                               && MON_WEP(mon)->oartifact == ART_WAND_OF_ORCUS);

    if (mon->mhp < mon->mhpmax && !mon->mwither
        && (!mon_orcus_wield || is_dprince(mon->data))
        && (!Is_valley(&u.uz) || is_undead(r_data(mon)))
        && (moves % 20 == 0 || mon_prop(mon, REGENERATION)))
        mon->mhp++;
    if (mon->mspec_used)
        mon->mspec_used--;
    if (mon->mbreakboulder)
        mon->mbreakboulder--;
    if (mon->msummoned)
        mon->msummoned--;
    if (mon->msicktime)
        mon->msicktime--;
    if (mon->mdiseasetime)
        mon->mdiseasetime--;
    if (mon->mreflecttime)
        mon->mreflecttime--;
    if (mon->mbarkskintime)
        mon->mbarkskintime--;
    if (mon->mstoneskintime)
        mon->mstoneskintime--;
    if (mon->mentangletime)
        mon->mentangletime--;
    if (mon->mlevitatetime)
        mon->mlevitatetime--;
    if (mon->mjumptime)
        mon->mjumptime--;
    if (digest_meal) {
        if (mon->meating) {
            mon->meating--;
            if (mon->meating <= 0)
                finish_meating(mon);
        }
    }
}

/*
 * Possibly awaken the given monster.  Return a 1 if the monster has been
 * jolted awake.
 */
STATIC_OVL int
disturb(mtmp)
struct monst *mtmp;
{
    /*
     * + Ettins are hard to surprise.
     * + Nymphs, jabberwocks, and leprechauns do not easily wake up.
     *
     * Wake up if:
     *  in direct LOS                                           AND
     *  within 10 squares                                       AND
     *  not stealthy or (mon is an ettin and 9/10)              AND
     *  (mon is not a nymph, jabberwock, or leprechaun) or 1/50 AND
     *  Aggravate or mon is (dog or human) or
     *      (1/7 and mon is not mimicing furniture or object)
     */
    if (couldsee(mtmp->mx, mtmp->my) && distu(mtmp->mx, mtmp->my) <= 100
        && (!Stealth || (mtmp->data == &mons[PM_ETTIN] && rn2(10)))
        && (!(is_nymph(mtmp->data)
              || mtmp->data == &mons[PM_JABBERWOCK]
              || mtmp->data == &mons[PM_VORPAL_JABBERWOCK]
              || mtmp->data->mlet == S_LEPRECHAUN) || !rn2(50))
        && (Aggravate_monster
            || (mtmp->data->mlet == S_DOG || mtmp->data->mlet == S_HUMAN)
            || (!rn2(7) && M_AP_TYPE(mtmp) != M_AP_FURNITURE
                && M_AP_TYPE(mtmp) != M_AP_OBJECT))) {
        mtmp->msleeping = 0;
        return 1;
    }
    return 0;
}

/* ungrab/expel held/swallowed hero */
STATIC_OVL void
release_hero(mon)
struct monst *mon;
{
    if (mon == u.ustuck) {
        if (u.uswallow) {
            expels(mon, mon->data, TRUE);
        } else if (!sticks(youmonst.data)) {
            unstuck(mon); /* let go */
            You("get released!");
        }
    }
}

#define flees_light(mon) \
    ((mon)->data == &mons[PM_GREMLIN]                                       \
     && ((uwep && uwep->lamplit && artifact_light(uwep)                     \
          && !(wielding_artifact(ART_STAFF_OF_THE_ARCHMAGI)                 \
               && !Upolyd && Race_if(PM_DROW)))                             \
         || (u.twoweap && uswapwep->lamplit && artifact_light(uswapwep)     \
             && !(wielding_artifact(ART_STAFF_OF_THE_ARCHMAGI)              \
                  && !Upolyd && Race_if(PM_DROW)))                          \
         || (uarm && uarm->lamplit && artifact_light(uarm)                  \
             && !(Is_dragon_armor(uarm)                                     \
                  && Dragon_armor_to_scales(uarm) == SHADOW_DRAGON_SCALES)) \
         || (uarms && uarms->lamplit && artifact_light(uarms))))
/* we could include this in the above macro, but probably overkill/overhead */
/*      && (!(which_armor((mon), W_ARMC) != 0                               */
/*            && which_armor((mon), W_ARMH) != 0))                          */

/* monster begins fleeing for the specified time, 0 means untimed flee
 * if first, only adds fleetime if monster isn't already fleeing
 * if fleemsg, prints a message about new flight, otherwise, caller should */
void
monflee(mtmp, fleetime, first, fleemsg)
struct monst *mtmp;
int fleetime;
boolean first;
boolean fleemsg;
{
    struct monst* mtmp2;

    /* shouldn't happen; maybe warrants impossible()? */
    if (DEADMONSTER(mtmp) || mindless(mtmp->data)
        || unique_corpstat(mtmp->data))
        return;

    if (mtmp == u.ustuck)
        release_hero(mtmp); /* expels/unstuck */

    if (!first || !mtmp->mflee) {
        /* don't lose untimed scare */
        if (!fleetime)
            mtmp->mfleetim = 0;
        else if (!mtmp->mflee || mtmp->mfleetim) {
            fleetime += (int) mtmp->mfleetim;
            /* ensure monster flees long enough to visibly stop fighting */
            if (fleetime == 1)
                fleetime++;
            mtmp->mfleetim = (unsigned) min(fleetime, 127);
        }
        if (!mtmp->mflee && fleemsg && canseemon(mtmp)
            && M_AP_TYPE(mtmp) != M_AP_FURNITURE
            && M_AP_TYPE(mtmp) != M_AP_OBJECT) {
            /* unfortunately we can't distinguish between temporary
               sleep and temporary paralysis, so both conditions
               receive the same alternate message */
            if (!mtmp->mcanmove || !mtmp->data->mmove) {
                pline("%s seems to flinch.", Adjmonnam(mtmp, "immobile"));
            } else if (flees_light(mtmp)) {
                if (rn2(10) || Deaf) {
                    /* this feels a bit hacky, but it works */
                    struct obj *litwep;
                    struct obj *litarmor;
                    struct obj *litshield;
                    litwep = (uwep && artifact_light(uwep)
                              && uwep->lamplit) ? uwep : uswapwep;
                    litarmor = uarm;
                    litshield = uarms;

                    pline("%s flees from the painful light of %s.",
                            Monnam(mtmp),
                            (uarm && artifact_light(uarm) && uarm->lamplit)
                                ? yobjnam(litarmor, (char *) 0)
                                : (uarms && artifact_light(uarms) && uarms->lamplit)
                                    ? yobjnam(litshield, (char *) 0)
                                    : bare_artifactname(litwep));
                } else
                    verbalize("Bright light!");
            } else if (!rn2(5) && !Deaf && !mindless(mtmp->data)) {
                const char *verb = 0;
                switch (mtmp->data->msound) {
                case MS_SILENT:
                case MS_HISS:
                case MS_BUZZ:
                    /* no sound */
                    break;
                case MS_BARK:
                case MS_GROWL:
                    if (mtmp->data->mlet == S_FELINE) {
                        verb = "yowl";
                        break;
                    }
                    verb = "howl";
                    break;
                case MS_ROAR:
                    if (mtmp->mnum == PM_WOOLLY_MAMMOTH
                        || mtmp->mnum == PM_MASTODON
                        || mtmp->mnum == PM_MUMAK) {
                        /* special handling for elephants */
                        verb = "trumpet";
                        break;
                    }
                    /* fallthrough */
                default:
                    verb = growl_sound(mtmp);
                    break;
                }
                if (verb) {
                    pline("%s %s in %s!", Monnam(mtmp),
                          vtense((char *) 0, verb),
                          rn2(2) ? "fear" : "terror");
                    /* Check and see who was close enough to hear it */
                    for (mtmp2 = fmon; mtmp2; mtmp2 = mtmp2->nmon) {
                        if (dist2(mtmp->mx, mtmp->my, mtmp2->mx, mtmp2->my) < 19
                            && !rn2(3))
                            mtmp2->msleeping = 0;
                    }
                }
            }
            if (mtmp->mcanmove)
                pline("%s turns to flee.", Monnam(mtmp));
        }
        mtmp->mflee = 1;
    }
    /* ignore recently-stepped spaces when made to flee */
    memset(mtmp->mtrack, 0, sizeof(mtmp->mtrack));
}

STATIC_OVL void
distfleeck(mtmp, inrange, nearby, scared)
struct monst *mtmp;
int *inrange, *nearby, *scared;
{
    int seescaryx, seescaryy;
    boolean sawscary = FALSE, bravegremlin = (rn2(5) == 0);

    *inrange = (dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy)
                <= (BOLT_LIM * BOLT_LIM));
    *nearby = *inrange && monnear(mtmp, mtmp->mux, mtmp->muy);

    /* Note: if your image is displaced, the monster sees the Elbereth
     * at your displaced position, thus never attacking your displaced
     * position, but possibly attacking you by accident.  If you are
     * invisible, it sees the Elbereth at your real position, thus never
     * running into you by accident but possibly attacking the spot
     * where it guesses you are.
     */
    if (!mtmp->mcansee || (Invis && !mon_prop(mtmp, SEE_INVIS))) {
        seescaryx = mtmp->mux;
        seescaryy = mtmp->muy;
    } else {
        seescaryx = u.ux;
        seescaryy = u.uy;
    }

    sawscary = onscary(seescaryx, seescaryy, mtmp);
    if (*nearby && (sawscary
                    || (flees_light(mtmp) && !bravegremlin)
                    || (!mtmp->mpeaceful && in_your_sanctuary(mtmp, 0, 0)))) {
        *scared = 1;
        monflee(mtmp, rnd(rn2(7) ? 10 : 100), TRUE, TRUE);
        if (u.ualign.type == A_NONE && !context.coward
            && sengr_at("Elbereth", seescaryx, seescaryy, TRUE)) {
            /* Followers of Moloch aren't supposed
             * to hide behind other gods. */
            You_feel("like a coward.");
            context.coward = TRUE; /* once per move */
            adjalign(-5);
            record_abuse_event(-5, ABUSE_COWARDICE);
        }
    } else
        *scared = 0;
}

#undef flees_light

/* perform a special one-time action for a monster; returns -1 if nothing
   special happened, 0 if monster uses up its turn, 1 if monster is killed */
STATIC_OVL int
m_arrival(mon)
struct monst *mon;
{
    mon->mstrategy &= ~STRAT_ARRIVE; /* always reset */

    return -1;
}

/* returns 1 if monster died moving, 0 otherwise */
/* The whole dochugw/m_move/distfleeck/mfndpos section is serious spaghetti
 * code. --KAA
 */
int
dochug(mtmp)
struct monst *mtmp;
{
    struct permonst *mdat;
    register int tmp = 0;
    struct monst* mdummy;
    int inrange, nearby, scared, oldx, oldy;

    boolean mwalk_sewage = is_sewage(mtmp->mx, mtmp->my);

    /*  Pre-movement adjustments
     */

    mdat = mtmp->data;

    if (mtmp->mstrategy & STRAT_ARRIVE) {
        int res = m_arrival(mtmp);
        if (res >= 0)
            return res;
    }

    /* check for waitmask status change */
    if ((mtmp->mstrategy & STRAT_WAITFORU)
        && (m_canseeu(mtmp) || mtmp->mhp < mtmp->mhpmax))
        mtmp->mstrategy &= ~STRAT_WAITFORU;

    /* update quest status flags */
    quest_stat_check(mtmp);

    if (!mtmp->mcanmove || (mtmp->mstrategy & STRAT_WAITMASK)) {
        if (Hallucination)
            newsym(mtmp->mx, mtmp->my);
        if (mtmp->mcanmove && (mtmp->mstrategy & STRAT_CLOSE)
            && !mtmp->msleeping && monnear(mtmp, u.ux, u.uy)) {
            if (mtmp->data == &mons[PM_LUCIFER]) {
                if (Role_if(PM_INFIDEL))
                    com_pager(401);
                else
                    com_pager(400);
                mtmp->mstrategy &= ~STRAT_WAITMASK;
            } else {
                quest_talk(mtmp); /* give the leaders a chance to speak */
            }
        }
        return 0; /* other frozen monsters can't do anything */
    }

    /* there is a chance we will wake it */
    if (mtmp->msleeping && !disturb(mtmp)) {
        if (Hallucination)
            newsym(mtmp->mx, mtmp->my);
        return 0;
    }

    /* not frozen or sleeping: wipe out texts written in the dust */
    wipe_engr_at(mtmp->mx, mtmp->my, 1, FALSE);

    /* special snark code; if it's next to you, you might discover
     * that it's a Boojum, and we need to swap out monsters in that case
     */
    if (mtmp->mnum == PM_SNARK && distu(mtmp->mx, mtmp->my) <= 2) {
        if (mtmp->m_id % 3 < 1) {
            oldx = mtmp->mx; oldy = mtmp->my;
            mongone(mtmp);
            mdummy = makemon(&mons[PM_BOOJUM], oldx, oldy, NO_MM_FLAGS);
            /* If someone has managed to extinct boojum, this will
             * result in the monster just vanishing.  But this should
             * be fairly difficult to do, since boojum only generate
             * from snarks at a rate of one snark, one boojum. */
            if (mdummy) {
                mtmp = mdummy;
                if (canseemon(mtmp)) {
                    pline("Oh, no, this Snark is a Boojum!");
                }
            } else {
                return 0; /* mtmp just went away, we'd better bail out */
            }
        }
    }

    /* confused monsters get unconfused with small probability */
    if (mtmp->mconf && !rn2(50))
        mtmp->mconf = 0;

    /* stunned monsters get un-stunned with larger probability */
    if (mtmp->mstun && !rn2(10))
        mtmp->mstun = 0;

    if (mtmp->mstone && munstone(mtmp, mtmp->mstonebyu)) {
        mtmp->mstone = 0;
        return 1; /* this is its move */
    }

    /* some monsters are slowed down if wading through sewage */
    if (mwalk_sewage) {
        if (is_flyer(mdat) || is_floater(mdat)
            || is_clinger(mdat) || is_swimmer(mdat)
            || passes_walls(mdat) || can_levitate(mtmp) || can_fly(mtmp)
            || can_wwalk(mtmp) || defended(mtmp, AD_SLOW)
            || resists_slow(r_data(mtmp))
            || ((mtmp == u.usteed) && Flying)) {
            mwalk_sewage = FALSE;
        } else {
            mon_adjust_speed(mtmp, -2, (struct obj *) 0);
        }
    }
    if (!mwalk_sewage)
        mon_adjust_speed(mtmp, 3, (struct obj *) 0);

    /* being in midair where gravity is still in effect can be lethal */
    if (is_open_air(mtmp->mx, mtmp->my)
        && !(is_flyer(mdat) || is_floater(mdat) || can_levitate(mtmp)
             || can_fly(mtmp) || is_clinger(mdat)
             || ((mtmp == u.usteed) && Flying))) {
        if (canseemon(mtmp))
            pline("%s plummets several thousand feet to %s death.",
                  Monnam(mtmp), mhis(mtmp));
        /* no corpse or objects as both are now several thousand feet down */
        mongone(mtmp);
    }

    /* some monsters teleport */
    if (mtmp->mflee && !rn2(40) && mon_prop(mtmp, TELEPORT) && !mtmp->iswiz
        && !level.flags.noteleport) {
        (void) rloc(mtmp, TRUE);
        return 0;
    }

    (void) maybe_freeze_underfoot(mtmp);

    if (mdat->msound == MS_SHRIEK && !um_dist(mtmp->mx, mtmp->my, 1))
        m_respond(mtmp);
    if (is_zombie(mdat) && !rn2(10))
        m_respond(mtmp);
    if (mdat == &mons[PM_MEDUSA] && couldsee(mtmp->mx, mtmp->my))
        m_respond(mtmp);
    if (is_gnome(mdat) && !is_undead(mdat)
        && m_canseeu(mtmp))
        m_respond(mtmp);
    if (is_support(mdat) && !mtmp->mpeaceful
        && rn2(2))
        m_respond(mtmp);
    if (DEADMONSTER(mtmp))
        return 1; /* m_respond gaze can kill medusa */

    /* fleeing monsters might regain courage */
    if (mtmp->mflee && !mtmp->mfleetim && mtmp->mhp == mtmp->mhpmax
        && !rn2(25))
        mtmp->mflee = 0;

    /* cease conflict-induced swallow/grab if conflict has ended */
    if (mtmp == u.ustuck && mtmp->mpeaceful && !mtmp->mconf && !Conflict) {
        release_hero(mtmp);
        return 0; /* uses up monster's turn */
    }

    set_apparxy(mtmp);
    /* Must be done after you move and before the monster does.  The
     * set_apparxy() call in m_move() doesn't suffice since the variables
     * inrange, etc. all depend on stuff set by set_apparxy().
     */

    /* Monsters that want to acquire things */
    /* may teleport, so do it before inrange is set */
    if (is_covetous(mdat)) {
        int tactics_result = tactics(mtmp);
        /* if tactics() returns 2 or 3, monster either died or migrated
           to another level */
        if (tactics_result >= 2)
            return tactics_result;
    }

    /* check distance and scariness of attacks */
    distfleeck(mtmp, &inrange, &nearby, &scared);

    if (find_defensive(mtmp)) {
        if (use_defensive(mtmp) != 0)
            return 1;
    } else if (find_misc(mtmp)) {
        if (use_misc(mtmp) != 0)
            return 1;
    }

    /* Demonic Blackmail! */
    if (nearby && mdat->msound == MS_BRIBE && mtmp->mpeaceful && !mtmp->mtame
        && !u.uswallow && monsndx(mdat) != PM_PRISON_GUARD) {
        if (mtmp->mstrategy & STRAT_APPEARMSG) {
            if (mtmp->mux != u.ux || mtmp->muy != u.uy) {
                pline("%s whispers at thin air.",
                      cansee(mtmp->mux, mtmp->muy) ? Monnam(mtmp) : "It");
                mtmp->mstrategy &= ~STRAT_APPEARMSG;

                if (is_demon(raceptr(&youmonst))) {
                    /* "Good hunting, brother" */
                    display_nhwindow(WIN_MESSAGE, FALSE); /* --More-- */
                    (void) rloc(mtmp, TRUE);
                } else {
                    mtmp->minvis = mtmp->perminvis = 0;
                    /* Why?  For the same reason in real demon talk */
                    pline("%s gets angry!", Amonnam(mtmp));
                    mtmp->mpeaceful = 0;
                    set_malign(mtmp);
                    /* since no way is an image going to pay it off */
                }
            } else if (demon_talk(mtmp))
                return 1; /* you paid it off */
        } else { /* either let the player pass, or pacified somehow */
            (void) rloc(mtmp, TRUE); /* either way no bribe demands */
            return 0;
        }
    }

    /* Prison guard extortion */
    if (nearby && (monsndx(mdat) == PM_PRISON_GUARD) && !mtmp->mpeaceful
        && !mtmp->mtame && !u.uswallow && (!mtmp->mspec_used)) {
        long gdemand = 500 * u.ulevel;
        long goffer = 0;

        if (!money_cnt(invent)) { /* can't bribe with no money */
            mtmp->mspec_used = 1000;
            return 0;
        }

        pline("%s demands %ld %s to avoid re-arrest.", Amonnam(mtmp),
              gdemand, currency(gdemand));
        if ((goffer = bribe(mtmp)) >= gdemand) {
            verbalize("Good.  Now beat it, scum!");
            mtmp->mpeaceful = 1;
            set_malign(mtmp);
        } else {
            pline("I said %ld!", gdemand);
            mtmp->mspec_used = 1000;
        }
    }

    /* the watch will look around and see if you are up to no good :-) */
    if (is_watch(mdat)) {
        watch_on_duty(mtmp);
    } else if ((is_mind_flayer(mdat) || mdat == &mons[PM_NEOTHELID])
               && !rn2(20)) {
        struct monst *m2, *nmon = (struct monst *) 0;

        if (canseemon(mtmp))
            pline("%s concentrates.", Monnam(mtmp));
        if (!(mindless(youmonst.data) || Race_if(PM_DRAUGR))) {
            if (distu(mtmp->mx, mtmp->my) > BOLT_LIM * BOLT_LIM) {
                You("sense a faint wave of psychic energy.");
                goto toofar;
            }
            pline("A wave of psychic energy pours over you!");
            if (mtmp->mpeaceful
                && (!Conflict || resist_conflict(mtmp))) {
                pline("It feels quite soothing.");
            } else if (maybe_polyd(is_illithid(youmonst.data), Race_if(PM_ILLITHID))) {
                Your("psionic abilities shield your brain.");
            } else if (!u.uinvulnerable) {
                register boolean m_sen = sensemon(mtmp);

                if (m_sen || (Blind_telepat && rn2(2)) || !rn2(10)) {
                    int dmg;
                    pline("It locks on to your %s!",
                          m_sen ? "telepathy" : Blind_telepat ? "latent telepathy"
                                                          : "mind");
                    dmg = rnd(15);
                    if (Half_spell_damage)
                        dmg = (dmg + 1) / 2;
                    losehp(dmg, "psychic blast", KILLED_BY_AN);
                }
            }
        }

        for (m2 = fmon; m2; m2 = nmon) {
            nmon = m2->nmon;
            if (DEADMONSTER(m2))
                continue;
            if (m2->mpeaceful == mtmp->mpeaceful)
                continue;
            if (mindless(m2->data))
                continue;
            if (m2 == mtmp)
                continue;
            if (is_illithid(m2->data))
                continue;
            if ((has_telepathy(m2) && (rn2(2) || m2->mblinded))
                || !rn2(10)) {
                if (cansee(m2->mx, m2->my))
                    pline("It locks on to %s.", mon_nam(m2));
                damage_mon(m2, rnd(15), AD_DRIN, FALSE);
                if (DEADMONSTER(m2))
                    monkilled(m2, "", AD_DRIN);
                else
                    m2->msleeping = 0;
            }
        }
        distfleeck(mtmp, &inrange, &nearby, &scared);
    }

    /* ghosts prefer turning invisible instead of moving if they can */
    if (mdat == &mons[PM_GHOST] && !mtmp->mpeaceful && !mtmp->mcan
        && !mtmp->mspec_used && !mtmp->minvis) {
        boolean couldsee = canseemon(mtmp);
        /* need to store the monster's name as we see it now; noit_Monnam after
         * the fact would give "The invisible Foo's ghost fades from view" */
        char nam[BUFSZ];
        Strcpy(nam, Monnam(mtmp));
        mtmp->minvis = 1;
        if (couldsee && !canseemon(mtmp)) {
            pline("%s fades from view.", nam);
            newsym(mtmp->mx, mtmp->my);
        } else if (couldsee && See_invisible) {
            pline("%s turns even more transparent.", nam);
            newsym(mtmp->mx, mtmp->my);
        }
        return 0;
    }

    /* Drow have the innate ability to cast an aura of darkness
       around themselves, and will use it if surrounded by light */
    if (!rn2(10) && racial_drow(mtmp)
        && !mtmp->mpeaceful /* shopkeepers/temple priests */
        && !is_undead(mdat) /* drow mummies/zombies */
        && !spot_is_dark(mtmp->mx, mtmp->my)) {
        if (canseemon(mtmp))
            pline("%s invokes an aura of darkness.",
                  Monnam(mtmp));
        litroom(FALSE, TRUE, NULL, mtmp->mx, mtmp->my);
        return 0;
    }

toofar:

    /* If monster is nearby you, and has to wield a weapon, do so.
       This costs the monster a move, of course */
    if ((!mtmp->mpeaceful || Conflict) && inrange
        && dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) <= 8
        && attacktype(mdat, AT_WEAP)) {
        struct obj *mw_tmp;

        /* The scared check is necessary.  Otherwise a monster that is
           one square near the player but fleeing into a wall would keep
           switching between pick-axe and weapon.  If monster is stuck
           in a trap, prefer ranged weapon (wielding is done in thrwmu).
           This may cost the monster an attack, but keeps the monster
           from switching back and forth if carrying both */
        mw_tmp = MON_WEP(mtmp);
        if (!(scared && mw_tmp && is_pick(mw_tmp))
            && !(mw_tmp && is_pole(mw_tmp))
            && mtmp->weapon_check == NEED_WEAPON
            && !((mtmp->mtrapped || mtmp->mentangled
                  || is_stationary(mdat))
                 && !nearby && select_rwep(mtmp))) {
            mtmp->weapon_check = NEED_HTH_WEAPON;
            if (mon_wield_item(mtmp) != 0)
                return 0;
        }
    }

    /* Look for other monsters to fight (at a distance). Intelligent
       monsters with Amulet of Yendor skip ranged attacks - escape */
    if ((!mon_has_amulet(mtmp) || mindless(mdat) || is_animal(mdat))
        && (((attacktype(mtmp->data, AT_BREA)
              || (attacktype(mtmp->data, AT_GAZE)
                  && mtmp->data != &mons[PM_MEDUSA])
              || attacktype(mtmp->data, AT_SPIT)
              || attacktype(mtmp->data, AT_SCRE)
              || (attacktype(mtmp->data, AT_MAGC)
                  && ((attacktype_fordmg(mtmp->data, AT_MAGC, AD_ANY))->adtyp
                      <= AD_LOUD)))
             && !mtmp->mspec_used)
            || (attacktype(mtmp->data, AT_WEAP)
                && select_rwep(mtmp) != 0) || find_offensive(mtmp))
        && mtmp->mlstmv != monstermoves) {
        struct monst *mtmp2 = mfind_target(mtmp);
        /* the > value is important here - if it's not just right,
           the attacking monster can get stuck in a loop switching
           back and forth between its melee weapon and launcher */
        if (mtmp2 && (mtmp2 != mtmp)
            && (mtmp2 != &youmonst
                || dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) > 8)) {
            int res;
            res = (mtmp2 == &youmonst) ? mattacku(mtmp)
                                       : mattackm(mtmp, mtmp2);
            if (res & MM_AGR_DIED)
                return 1; /* Oops. */

            return 0; /* that was our move for the round */
        }
    }

    /* check to see if we should stash something, monsters with Amulet
       skip stashing */
    if (!mon_has_amulet(mtmp) && m_stash_items(mtmp, FALSE))
	return 0;

    /* Now the actual movement phase */

    if (mtmp->data == &mons[PM_HEZROU]) /* stench */
        create_gas_cloud(mtmp->mx, mtmp->my, 1, 8);

    if (!nearby || mtmp->mflee || scared || mtmp->mconf || mtmp->mstun
        || (mtmp->minvis && !rn2(3))
        || (mdat->mlet == S_LEPRECHAUN && !findgold(invent, FALSE)
            && (findgold(mtmp->minvent, FALSE) || rn2(2)))
        || (is_wanderer(mdat) && !rn2(4)) || (Conflict && !mtmp->iswiz)
        || is_skittish(mdat) || (!mtmp->mcansee && !rn2(4)) || mtmp->mpeaceful
        || (mon_has_amulet(mtmp) && !mindless(mdat) && !is_animal(mdat))) {
        /* Possibly cast an undirected spell if not attacking you */
        /* note that most of the time castmu() will pick a directed
           spell and do nothing, so the monster moves normally */
        /* arbitrary distance restriction to keep monster far away
           from you from having cast dozens of sticks-to-snakes
           or similar spells by the time you reach it */
        if (!mtmp->mspec_used
            && dist2(mtmp->mx, mtmp->my, u.ux, u.uy) <= 49) {
            struct attack *mattk, *a;
            mattk = has_erac(mtmp) ? ERAC(mtmp)->mattk: mdat->mattk;

            for (a = &mattk[0]; a < &mattk[NATTK]; a++) {
                if (a->aatyp == AT_MAGC
                    && (a->adtyp == AD_SPEL || a->adtyp == AD_CLRC)) {
                    if (castmu(mtmp, a, FALSE, FALSE)) {
                        tmp = is_skittish(mdat) ? 0 : 3;
                        break;
                    }
                }
            }
        }

        if (!tmp)
            tmp = m_move(mtmp, 0);
        else
            tmp = 0;
        update_monsteed(mtmp);
        if (tmp != 2)
            distfleeck(mtmp, &inrange, &nearby, &scared); /* recalc */

        switch (tmp) { /* for pets, cases 0 and 3 are equivalent */
        case 0: /* no movement, but it can still attack you */
        case 3: /* absolutely no movement */
            /* vault guard might have vanished */
            if (mtmp->isgd && (DEADMONSTER(mtmp) || mtmp->mx == 0))
                return 1; /* behave as if it died */
            /* During hallucination, monster appearance should
             * still change - even if it doesn't move.
             */
            if (Hallucination)
                newsym(mtmp->mx, mtmp->my);
            break;
        case 1: /* monster moved */
            /* Maybe it stepped on a trap and fell asleep... */
            if (mtmp->msleeping || !mtmp->mcanmove)
                return 0;
            /* Monsters can move and then shoot on same turn;
               our hero can't.  Is that fair? */
            if (!nearby && (ranged_attk(mtmp) || find_offensive(mtmp)))
                break;
            /* engulfer/grabber checks */
            if (mtmp == u.ustuck) {
                /* a monster that's digesting you can move at the
                 * same time -dlc
                 */
                if (u.uswallow
                    && (!mon_has_amulet(mtmp)
                        || mindless(mdat) || is_animal(mdat)))
                    return mattacku(mtmp);
                /* if confused grabber has wandered off, let go */
                if (distu(mtmp->mx, mtmp->my) > 2)
                    unstuck(mtmp);
            }
            return 0;
        case 2: /* monster died */
            return 1;
        }
    }

    /* Now, attack the player if possible - one attack set per monst */

    if (tmp != 3 && (!mtmp->mpeaceful
                     || (Conflict && !resist_conflict(mtmp)))) {
        if (inrange && !scared && !noattacks(mdat)
            /* intelligent monsters with the Amulet skip attacks, flee */
            && (!mon_has_amulet(mtmp)
                || mindless(mdat) || is_animal(mdat))
            /* [is this hp check really needed?] */
            && (Upolyd ? u.mh : u.uhp) > 0) {
            if (mattacku(mtmp))
                return 1; /* monster died (e.g. exploded) */
        }
        if (mtmp->wormno)
            wormhitu(mtmp);
    }
    /* special speeches for quest monsters */
    if (!mtmp->msleeping && mtmp->mcanmove && nearby)
        quest_talk(mtmp);
    /* extra emotional attack for vile monsters */
    if (inrange && mtmp->data->msound == MS_CUSS && !mtmp->mpeaceful
        && couldsee(mtmp->mx, mtmp->my) && !mtmp->minvis && !rn2(5))
        cuss(mtmp);
    /* freeing the Ice Queen from her curse */
    if (inrange && mtmp->data->msound == MS_CUSS && mtmp->mpeaceful
        && mtmp->data == &mons[PM_KATHRYN_THE_ENCHANTRESS]
        && couldsee(mtmp->mx, mtmp->my) && !mtmp->minvis && !rn2(5))
        cuss(mtmp);
    /* players monsters in Purgatory */
    if (Inpurg && (distu(mtmp->mx, mtmp->my) <= 15)
        && is_mplayer(mtmp->data) && !mtmp->mpeaceful
        && couldsee(mtmp->mx, mtmp->my)
        && !mtmp->minvis && !rn2(5))
        mplayer_purg_talk(mtmp);
    /* Saint Michael the Archangel in Purgatory */
    if (Inpurg && (distu(mtmp->mx, mtmp->my) <= 15)
        && mtmp->data == &mons[PM_SAINT_MICHAEL]
        && !mtmp->mpeaceful
        && couldsee(mtmp->mx, mtmp->my)
        && !mtmp->minvis && !rn2(5))
        archangel_purg_talk(mtmp);

    /* note: can't get here when tmp==2 so this always returns 0 */
    return (tmp == 2);
}

static NEARDATA const char practical[] = { WEAPON_CLASS, ARMOR_CLASS,
                                           GEM_CLASS, FOOD_CLASS, 0 };
static NEARDATA const char magical[] = { AMULET_CLASS, POTION_CLASS,
                                         SCROLL_CLASS, WAND_CLASS,
                                         RING_CLASS,   SPBOOK_CLASS, 0 };
static NEARDATA const char indigestion[] = { BALL_CLASS, ROCK_CLASS, 0 };
static NEARDATA const char boulder_class[] = { ROCK_CLASS, 0 };
static NEARDATA const char gem_class[] = { GEM_CLASS, 0 };

boolean
itsstuck(mtmp)
struct monst *mtmp;
{
    if (sticks(youmonst.data) && mtmp == u.ustuck && !u.uswallow) {
        pline("%s cannot escape from you!", Monnam(mtmp));
        return TRUE;
    }
    return FALSE;
}

/*
 * should_displace()
 *
 * Displacement of another monster is a last resort and only
 * used on approach. If there are better ways to get to target,
 * those should be used instead. This function does that evaluation.
 */
boolean
should_displace(mtmp, poss, info, cnt, gx, gy)
struct monst *mtmp;
coord *poss; /* coord poss[9] */
long *info;  /* long info[9] */
int cnt;
xchar gx, gy;
{
    int shortest_with_displacing = -1;
    int shortest_without_displacing = -1;
    int count_without_displacing = 0;
    register int i, nx, ny;
    int ndist;

    for (i = 0; i < cnt; i++) {
        nx = poss[i].x;
        ny = poss[i].y;
        ndist = dist2(nx, ny, gx, gy);
        if (MON_AT(nx, ny) && (info[i] & ALLOW_MDISP) && !(info[i] & ALLOW_M)
            && !undesirable_disp(mtmp, nx, ny)) {
            if (shortest_with_displacing == -1
                || (ndist < shortest_with_displacing))
                shortest_with_displacing = ndist;
        } else {
            if ((shortest_without_displacing == -1)
                || (ndist < shortest_without_displacing))
                shortest_without_displacing = ndist;
            count_without_displacing++;
        }
    }
    if (shortest_with_displacing > -1
        && (shortest_with_displacing < shortest_without_displacing
            || !count_without_displacing))
        return TRUE;
    return FALSE;
}

boolean
m_digweapon_check(mtmp, nix, niy)
struct monst *mtmp;
xchar nix,niy;
{
    boolean can_tunnel = 0;
    struct obj *mw_tmp = MON_WEP(mtmp);

    if (!Is_rogue_level(&u.uz))
        can_tunnel = racial_tunnels(mtmp);

    if (can_tunnel && racial_needspick(mtmp)
        && !(mwelded(mw_tmp) && mtmp->data != &mons[PM_INFIDEL])
        && (may_dig(nix, niy) || closed_door(nix, niy))) {
        /* may_dig() is either IS_STWALL or IS_TREES */
        if (closed_door(nix, niy)) {
            if (!mw_tmp
                || !is_pick(mw_tmp)
                || !is_axe(mw_tmp))
                mtmp->weapon_check = NEED_PICK_OR_AXE;
        } else if (IS_TREES(levl[nix][niy].typ)) {
            if (!(mw_tmp = MON_WEP(mtmp)) || !is_axe(mw_tmp))
                mtmp->weapon_check = NEED_AXE;
        } else if (IS_STWALL(levl[nix][niy].typ)) {
            if (!(mw_tmp = MON_WEP(mtmp)) || !is_pick(mw_tmp))
                mtmp->weapon_check = NEED_PICK_AXE;
        }
        if (mtmp->weapon_check >= NEED_PICK_AXE && mon_wield_item(mtmp))
            return TRUE;
    }
    return FALSE;
}

STATIC_OVL boolean
likes_contents(mtmp, container)
struct monst *mtmp;
struct obj *container;
{
    boolean likegold = 0, likegems = 0,
            likeobjs = 0, likemagic = 0, uses_items = 0;
    boolean can_open = 0, can_unlock = 0;
    int pctload = (curr_mon_load(mtmp) * 100) / max_mon_load(mtmp);
    struct obj *otmp;

    can_open = !(nohands(mtmp->data) || r_verysmall(mtmp)
                 || container->otyp == IRON_SAFE
                 || container->otyp == CRYSTAL_CHEST);
    can_unlock = ((can_open
                  && (m_carrying(mtmp, SKELETON_KEY)
                      || m_carrying(mtmp, LOCK_PICK)
                      || m_carrying(mtmp, CREDIT_CARD)
                      || m_carrying(mtmp, MAGIC_KEY)))
                  || mtmp->iswiz || is_rider(mtmp->data));

    if (!Is_nonprize_container(container))
        return FALSE;

    if (container->olocked && !can_unlock)
        return FALSE;

    if (container->otyp == HIDDEN_CHEST
        && container->olocked && !m_carrying(mtmp, MAGIC_KEY))
        return FALSE;

    likegold = (likes_gold(mtmp->data) && pctload < 95
                && !can_levitate(mtmp));
    likegems = (likes_gems(mtmp->data) && pctload < 85
                && !can_levitate(mtmp));
    uses_items = (!mindless(mtmp->data) && !is_animal(mtmp->data)
                  && pctload < 75);
    likeobjs = (likes_objs(mtmp->data) && pctload < 75
                && !can_levitate(mtmp));
    likemagic = (likes_magic(mtmp->data) && pctload < 85
                 && !can_levitate(mtmp));

    if (!likegold && !likegems && !uses_items
        && !likeobjs && !likemagic)
        return FALSE;

    for (otmp = container->cobj; otmp; otmp = otmp->nobj) {
        if (((likegold && otmp->oclass == COIN_CLASS)
              || (likeobjs && index(practical, otmp->oclass)
                  && (otmp->otyp != CORPSE
                      || (is_nymph(mtmp->data)
                          && !is_rider(&mons[otmp->corpsenm]))))
             || (likemagic && index(magical, otmp->oclass))
             || (uses_items && searches_for_item(mtmp, otmp))
             || (likegems && otmp->oclass == GEM_CLASS
                 && otmp->material != MINERAL))
            && touch_artifact(otmp, mtmp) && can_carry(mtmp, otmp))
            return TRUE;
    }
    return FALSE;
}

/* Return values:
 * 0: did not move, but can still attack and do other stuff.
 * 1: moved, possibly can attack.
 * 2: monster died.
 * 3: did not move, and can't do anything else either.
 */
int
m_move(mtmp, after)
struct monst *mtmp;
register int after;
{
    register int appr;
    xchar gx, gy, nix, niy, chcnt;
    int chi; /* could be schar except for stupid Sun-2 compiler */
    boolean likegold = 0, likegems = 0, likeobjs = 0, likemagic = 0,
            conceals = 0;
    boolean likerock = 0, can_tunnel = 0;
    boolean can_open = 0, can_unlock = 0, doorbuster = 0;
    boolean uses_items = 0, setlikes = 0;
    boolean avoid = FALSE;
    boolean better_with_displacing = FALSE;
    boolean sawmon = canspotmon(mtmp); /* before it moved */
    struct permonst *ptr;
    struct monst *mtoo;
    schar mmoved = 0; /* not strictly nec.: chi >= 0 will do */
    long info[9];
    long flag;
    int omx = mtmp->mx, omy = mtmp->my;
    int offer;

    if (mtmp->mtrapped) {
        int i = mintrap(mtmp);

        if (i >= 2) {
            newsym(mtmp->mx, mtmp->my);
            return 2;
        } /* it died */
        if (i == 1)
            return 0; /* still in trap, so didn't move */
    }
    ptr = mtmp->data; /* mintrap() can change mtmp->data -dlc */

    /* doesn't move, but can still attack */
    if (is_stationary(ptr) || mtmp->mentangled)
        return 0;

    if (mtmp->ridden_by)
        return 0;

    if (mtmp->meating) {
        mtmp->meating--;
        if (mtmp->meating <= 0)
            finish_meating(mtmp);
        return 3; /* still eating */
    }

    /* Offering takes precedence over everything else */
    if (IS_ALTAR(levl[mtmp->mx][mtmp->my].typ)) {
        offer = moffer(mtmp);
        if (offer != 0) {
            return offer;
        }
    }

    set_apparxy(mtmp);
    /* where does mtmp think you are? */
    /* Not necessary if m_move called from this file, but necessary in
     * other calls of m_move (ex. leprechauns dodging)
     */
    if (!Is_rogue_level(&u.uz))
        can_tunnel = racial_tunnels(mtmp);
    can_open = !(nohands(ptr) || r_verysmall(mtmp));
    can_unlock =
        ((can_open && monhaskey(mtmp, TRUE)) || mtmp->iswiz || is_rider(ptr));
    doorbuster = racial_giant(mtmp);
    if (mtmp->wormno)
        goto not_special;
    /* my dog gets special treatment */
    if (mtmp->mtame) {
        mmoved = dog_move(mtmp, after);
        goto postmov;
    }

    /* likewise for shopkeeper */
    if (mtmp->isshk) {
        mmoved = shk_move(mtmp);
        if (mmoved == -2)
            return 2;
        if (mmoved >= 0)
            goto postmov;
        mmoved = 0; /* follow player outside shop */
    }

    /* and for the guard */
    if (mtmp->isgd) {
        mmoved = gd_move(mtmp);
        if (mmoved == -2)
            return 2;
        if (mmoved >= 0)
            goto postmov;
        mmoved = 0;
    }

    /* and the acquisitive monsters get special treatment */
    if (is_covetous(ptr)) {
        int covetousattack;
        xchar tx = STRAT_GOALX(mtmp->mstrategy),
              ty = STRAT_GOALY(mtmp->mstrategy);
        struct monst *intruder = isok(tx, ty) ? m_at(tx, ty) : NULL;
        /*
         * if there's a monster on the object or in possession of it,
         * attack it.
         */
        if (intruder && intruder != mtmp
            /* this used to use 'dist2() < 2' which meant that intended
               attack was disallowed if they were adjacent diagonally */
            && dist2(mtmp->mx, mtmp->my, tx, ty) <= 2) {
            bhitpos.x = tx, bhitpos.y = ty;
            notonhead = (intruder->mx != tx || intruder->my != ty);
            covetousattack = mattackm(mtmp, intruder);
            /* this used to erroneously use '== 2' (MM_DEF_DIED) */
            if (covetousattack & MM_AGR_DIED)
                return 2;
            mmoved = 1;
        } else {
            mmoved = 0;
        }
        if (distu(mtmp->mx, mtmp->my) > 8)
            goto postmov;
    }

    /* and for the priest */
    if (mtmp->ispriest) {
        mmoved = pri_move(mtmp);
        if (mmoved == -2)
            return 2;
        if (mmoved >= 0)
            goto postmov;
        mmoved = 0;
    }

#ifdef MAIL
    if (ptr == &mons[PM_MAIL_DAEMON]) {
        if (!Deaf && canseemon(mtmp))
            verbalize("I'm late!");
        mongone(mtmp);
        return 2;
    }
#endif

    /* jump toward the player if that lies in
       our nature, can see the player, and isn't
       otherwise incapacitated in some way */
    if ((can_jump(mtmp) || is_jumper(ptr)) && m_canseeu(mtmp)
        && !(mtmp->mflee || mtmp->mconf
             || mtmp->mstun || mtmp->msleeping)) {
        int dist = dist2(mtmp->mx, mtmp->my, u.ux, u.uy);

        if (!mtmp->mpeaceful && !rn2(3) && dist <= 20 && dist > 8) {
            int x = u.ux - mtmp->mx;
            int y = u.uy - mtmp->my;
            if (x < 0)
                x = 1;
            else if (x > 0)
                x = -1;
            if (y < 0)
                y = 1;
            else if (y > 0)
                y = -1;
            if (rloc_pos_ok(u.ux + x, u.uy + y, mtmp)
                && check_mon_jump(mtmp, u.ux + x, u.uy + y)) {
                rloc_to(mtmp, u.ux + x, u.uy + y);
                if (canseemon(mtmp))
                    pline("%s leaps at you!", Monnam(mtmp));
                mmoved = 1;
                goto postmov;
            }
        }
    }

    /* teleport if that lies in our nature */
    if (mon_prop(mtmp, TELEPORT) && !rn2(ptr == &mons[PM_TENGU] ? 5 : 85)
        && !tele_restrict(mtmp) && !((mtmp->isshk || mtmp->ispriest) && mtmp->mpeaceful)) {
	if (!decide_to_teleport(mtmp) || rn2(2))
            (void) rloc(mtmp, TRUE);
        else
            mnexto(mtmp);
        mmoved = 1;
        goto postmov;
    }
 not_special:
    if (u.uswallow && !mtmp->mflee && u.ustuck != mtmp)
        return 1;
    omx = mtmp->mx;
    omy = mtmp->my;
    gx = mtmp->mux;
    gy = mtmp->muy;
    appr = mtmp->mflee ? -1 : 1;
    if (mtmp->mconf || (u.uswallow && mtmp == u.ustuck)) {
        appr = 0;
    } else {
        struct obj *lepgold, *ygold;
        boolean should_see = (couldsee(omx, omy)
                              && (levl[gx][gy].lit || !levl[omx][omy].lit)
                              && (dist2(omx, omy, gx, gy) <= 36));

        if (!mtmp->mcansee
            || (should_see && Invis && !mon_prop(mtmp, SEE_INVIS) && rn2(11))
            || is_obj_mappear(&youmonst, STRANGE_OBJECT) || u.uundetected
            || (is_obj_mappear(&youmonst, GOLD_PIECE) && !likes_gold(ptr))
            || (mtmp->mpeaceful && !mtmp->isshk) /* allow shks to follow */
            || ((monsndx(ptr) == PM_STALKER || ptr->mlet == S_BAT
                 || ptr->mlet == S_LIGHT) && !rn2(3)))
            appr = 0;

        /* unintelligent monsters won't realize hiding tortle is a creature
         * from far away, and even intelligent monsters may overlook it
         * occasionally */
        if ((Hidinshell && (is_animal(ptr) || mindless(ptr) || !rn2(6))))
            appr = 0;

        /* same for Druids that have used wildshape to change into their
           various forms */
        if (druid_form
            && (is_animal(ptr) || mindless(ptr) || !rn2(6)))
            appr = 0;

        /* same for Vampires that have shapechanged into their
           various forms */
        if (vampire_form
            && (is_animal(ptr) || mindless(ptr) || !rn2(6)))
            appr = 0;

        /* does this monster like to play keep-away? */
        if (is_skittish(ptr)
            && (dist2(omx, omy, gx, gy) < 10)
            && !mtmp->mberserk)
            appr = -1;

        if (monsndx(ptr) == PM_LEPRECHAUN && (appr == 1)
            && ((lepgold = findgold(mtmp->minvent, TRUE))
                && (lepgold->quan
                    > ((ygold = findgold(invent, TRUE)) ? ygold->quan : 0L))))
            appr = -1;

        /* hostile monsters with ranged thrown weapons try to stay away */
        if (!mtmp->mpeaceful
            && (dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) < 5 * 5)
            && m_canseeu(mtmp) && m_has_launcher_and_ammo(mtmp))
            appr = -1;

        /* ... unless they are currently berserk */
        if (mtmp->mberserk)
            appr = 1;

        /* Support casters hang back */
        if (is_support(mtmp->data) && !mtmp->mpeaceful
            && (dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) < 4 * 4)
            && ((u.uhpmax / u.uhp) < 4))
            appr = -1;

        /* Any monster with the Amulet of Yendor flees to escape */
        if (mon_has_amulet(mtmp))
            appr = -1;

        if (!should_see && can_track(ptr)) {
            register coord *cp;

            cp = gettrack(omx, omy);
            if (cp) {
                gx = cp->x;
                gy = cp->y;
            }
        }
    }

    /* Honey badgers prioritize royal jelly over the player */
    if (ptr == &mons[PM_HONEY_BADGER] && !mtmp->mpeaceful) {
        struct obj *otmp;
        int best_dist = COLNO * ROWNO; /* large number */
        int jx = 0, jy = 0;

        /* Scan entire level for royal jelly */
        for (otmp = fobj; otmp; otmp = otmp->nobj) {
            if (is_royaljelly(otmp)) {
                int d = dist2(omx, omy, otmp->ox, otmp->oy);
                if (d < best_dist) {
                    best_dist = d;
                    jx = otmp->ox;
                    jy = otmp->oy;
                }
            }
        }
        /* If royal jelly found, override player-seeking goal */
        if (jx && jy) {
            gx = jx;
            gy = jy;
        }
        /* Otherwise fall through to normal player-seeking */
    }

    /* Intelligent monsters with Amulet of Yendor seek escape route.
       In main dungeon: approach stairs to escape.
       In endgame: approach portal, or aligned altar on Astral.
       Mindless/animal monsters don't understand the Amulet's significance. */
    if (mon_has_amulet(mtmp) && !mtmp->mpeaceful
        && !mindless(ptr) && !is_animal(ptr)) {
        int escape_x = 0, escape_y = 0;

        if (!In_endgame(&u.uz)) {
            /* In dungeon: seek upstairs to escape */
            if (xupstair) {
                escape_x = xupstair;
                escape_y = yupstair;
            } else if (xupladder) {
                escape_x = xupladder;
                escape_y = yupladder;
            }
            /* Fall back to special stairs (e.g. dlvl 1 to Planes) */
            if (!escape_x && sstairs.sx) {
                escape_x = sstairs.sx;
                escape_y = sstairs.sy;
            }
        } else if (!Is_astralevel(&u.uz)) {
            /* On elemental planes: seek magic portal to next plane */
            struct trap *ttrap;
            for (ttrap = ftrap; ttrap; ttrap = ttrap->ntrap) {
                if (ttrap->ttyp == MAGIC_PORTAL) {
                    escape_x = ttrap->tx;
                    escape_y = ttrap->ty;
                    break;
                }
            }
        } else {
            /* On Astral: seek monster's aligned altar to sacrifice.
               Covetous monsters use tactics(), but non-covetous
               intelligent monsters need this fallback */
            aligntyp malign = sgn(mon_aligntyp(mtmp));
            int x, y;

            for (x = 1; x < COLNO; x++) {
                for (y = 0; y < ROWNO; y++) {
                    if (IS_ALTAR(levl[x][y].typ)
                        && a_align(x, y) == malign) {
                        escape_x = x;
                        escape_y = y;
                        goto found_altar;
                    }
                }
            }
found_altar:
            ; /* empty statement after label */
        }

        if (escape_x && escape_y) {
            gx = escape_x;
            gy = escape_y;
            appr = 1; /* Approach escape route, override flee tendency */
        }
    }

    if ((!mtmp->mpeaceful || !rn2(10)) && (!Is_rogue_level(&u.uz))
        /* monsters with the Amulet don't stop to pick up objects */
        && !mon_has_amulet(mtmp)) {
        boolean in_line = (lined_up(mtmp)
               && (distmin(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy)
                   <= (racial_throws_rocks(&youmonst) ? 20 : ACURRSTR / 2 + 1)));

        if (appr != 1 || !in_line) {
            /* Monsters in combat won't pick stuff up, avoiding the
             * situation where you toss arrows at it and it has nothing
             * better to do than pick the arrows up */
            register int pctload =
                (curr_mon_load(mtmp) * 100) / max_mon_load(mtmp);

            /* look for gold or jewels nearby */
            likegold = (likes_gold(ptr) && pctload < 95 && !can_levitate(mtmp));
            likegems = (likes_gems(ptr) && pctload < 85 && !can_levitate(mtmp));
            uses_items = (!mindless(ptr) && !is_animal(ptr) && pctload < 75);
            likeobjs = (likes_objs(ptr) && pctload < 75 && !can_levitate(mtmp));
            likemagic = (likes_magic(ptr) && pctload < 85 && !can_levitate(mtmp));
            likerock = (racial_throws_rocks(mtmp) && pctload < 50 && !Sokoban
                        && !can_levitate(mtmp));
            conceals = hides_under(ptr);
            setlikes = TRUE;
        }
    }

#define SQSRCHRADIUS 5

    {
        register int minr = SQSRCHRADIUS; /* not too far away */
        struct obj *otmp;
        register int xx, yy;
        int oomx, oomy, lmx, lmy;

        /* cut down the search radius if it thinks character is closer */
        if (distmin(mtmp->mux, mtmp->muy, omx, omy) < SQSRCHRADIUS
            && !mtmp->mpeaceful)
            minr--;
        /* guards shouldn't get too distracted */
        if (!mtmp->mpeaceful && is_mercenary(ptr))
            minr = 1;

        if ((likegold || likegems || likeobjs || likemagic || likerock
             || conceals) && (!*in_rooms(omx, omy, SHOPBASE)
                              || (!rn2(25) && !mtmp->isshk))) {
 look_for_obj:
            oomx = min(COLNO - 1, omx + minr);
            oomy = min(ROWNO - 1, omy + minr);
            lmx = max(1, omx - minr);
            lmy = max(0, omy - minr);
            otmp = fobj;
            if (level.flags.nmagicchests && mchest) {
                int mcx, mcy;
                for (mcx = lmx; mcx <= oomx; ++mcx) {
                    for (mcy = lmy; mcy <= oomy; ++mcy) {
                        if (IS_MAGIC_CHEST(levl[mcx][mcy].typ)) {
                            mchest->nobj = fobj;
                            mchest->ox = mcx;
                            mchest->oy = mcy;
                            otmp = mchest;
                            break;
                        }
                    }
                }
            }
            for (; otmp; otmp = otmp->nobj) {
                /* monsters may pick rocks up, but won't go out of their way
                   to grab them; this might hamper sling wielders, but it cuts
                   down on move overhead by filtering out most common item */
                if (otmp->otyp == ROCK)
                    continue;
                xx = otmp->ox;
                yy = otmp->oy;
                /* Nymphs take everything.  Most other creatures should not
                 * pick up corpses except as a special case like in
                 * searches_for_item().  We need to do this check in
                 * mpickstuff() as well.
                 */
                if (xx >= lmx && xx <= oomx && yy >= lmy && yy <= oomy) {
                    /* don't get stuck circling around object that's
                       underneath an immobile or hidden monster;
                       paralysis victims excluded */
                    if ((mtoo = m_at(xx, yy)) != 0
                        && (mtoo->msleeping || mtoo->mundetected
                            || (mtoo->mappearance && !mtoo->iswiz)
                            || !mtoo->data->mmove))
                        continue;
                    /* the mfndpos() test for whether to allow a move to a
                       water location accepts flyers, but they can't reach
                       underwater objects, so being able to move to a spot
                       is insufficient for deciding whether to do so */
                    if ((is_pool(xx, yy) && !is_swimmer(ptr))
                        || (is_lava(xx, yy) && !likes_lava(ptr)))
                        continue;

                    /* if open air and can't fly/float and gravity
                       is in effect */
                    if (is_open_air(xx, yy)
                        && !(is_flyer(ptr) || is_floater(ptr)
                             || is_clinger(ptr) || can_levitate(mtmp)
                             || can_fly(mtmp)))
                        continue;

                    /* ignore sokoban prizes */
                    if (is_soko_prize_flag(otmp))
                        continue;

                    if ((((Is_container(otmp)
                           && likes_contents(mtmp, otmp) && can_open)
                          || ((likegold && otmp->oclass == COIN_CLASS)
                          || (likeobjs && index(practical, otmp->oclass)
                              && (otmp->otyp != CORPSE
                                  || (is_nymph(ptr)
                                      && !is_rider(&mons[otmp->corpsenm]))))
                        || (likemagic && index(magical, otmp->oclass))
                        || (uses_items && searches_for_item(mtmp, otmp))
                        || (likerock && otmp->otyp == BOULDER)
                        || (likegems && otmp->oclass == GEM_CLASS
                            && otmp->material != MINERAL)
                        || (conceals && !cansee(otmp->ox, otmp->oy))
                        || (ptr == &mons[PM_GELATINOUS_CUBE]
                            && !index(indigestion, otmp->oclass))))
                        && !((otmp->otyp == CORPSE
                             && touch_petrifies(&mons[otmp->corpsenm]))))
                        && touch_artifact(otmp, mtmp)) {
                        if (((can_carry(mtmp, otmp) > 0 || (Is_container(otmp)))
                            && ((racial_throws_rocks(mtmp)) || !sobj_at(BOULDER, xx, yy)))
                            && ((!is_unicorn(ptr)
                                || (otmp->material == GEMSTONE)))
                            /* Don't get stuck circling an Elbereth */
                            && (!onscary(xx, yy, mtmp))) {
                            minr = distmin(omx, omy, xx, yy);
                            oomx = min(COLNO - 1, omx + minr);
                            oomy = min(ROWNO - 1, omy + minr);
                            lmx = max(1, omx - minr);
                            lmy = max(0, omy - minr);
                            gx = otmp->ox;
                            gy = otmp->oy;
                            if (gx == omx && gy == omy) {
                                mmoved = 3; /* actually unnecessary */
                                if (mchest) {
                                    mchest->ox = mchest->oy = 0;
                                    mchest->nobj = (struct obj *) 0;
                                }
                                goto postmov;
                            }
                        }
                    }
                }
            }
            if (mchest) {
                mchest->ox = mchest->oy = 0;
                mchest->nobj = (struct obj *) 0;
            }
        } else if (likegold) {
            /* don't try to pick up anything else, but use the same loop */
            uses_items = 0;
            likegems = likeobjs = likemagic = likerock = conceals = 0;
            goto look_for_obj;
        }

        /* Don't override escape goal for Amulet carriers */
        if (minr < SQSRCHRADIUS && appr == -1 && !mon_has_amulet(mtmp)) {
            if (distmin(omx, omy, mtmp->mux, mtmp->muy) <= 3) {
                gx = mtmp->mux;
                gy = mtmp->muy;
            } else
                appr = 1;
        }
    }

    /* don't tunnel if hostile and close enough to prefer a weapon */
    if (can_tunnel && racial_needspick(mtmp)
        && ((!mtmp->mpeaceful || Conflict)
            && dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) <= 8))
        can_tunnel = FALSE;

    nix = omx;
    niy = omy;
    flag = 0L;
    if (mtmp->mpeaceful && (!Conflict || resist_conflict(mtmp)))
        flag |= (ALLOW_SANCT | ALLOW_SSM);
    else
        flag |= ALLOW_U;
    if (is_minion(ptr) || is_rider(ptr))
        flag |= ALLOW_SANCT;
    /* unicorn may not be able to avoid hero on a noteleport level */
    if (is_unicorn(ptr) && !level.flags.noteleport)
        flag |= NOTONL;
    if (passes_walls(ptr))
        flag |= (ALLOW_WALL | ALLOW_ROCK);
    if (passes_bars(ptr))
        flag |= ALLOW_BARS;
    if (can_tunnel)
        flag |= ALLOW_DIG;
    if (is_human(ptr) || ptr == &mons[PM_MINOTAUR])
        flag |= ALLOW_SSM;
    if ((is_undead(ptr) && ptr->mlet != S_GHOST)
        || is_vampshifter(mtmp))
        flag |= NOGARLIC;
    if (racial_throws_rocks(mtmp)
        || m_can_break_boulder(mtmp))
        flag |= ALLOW_ROCK;
    if (can_open)
        flag |= OPENDOOR;
    if (can_unlock)
        flag |= UNLOCKDOOR;
    if (doorbuster)
        flag |= BUSTDOOR;
    {
        register int i, j, nx, ny, nearer;
        int jcnt, cnt;
        int ndist, nidist;
        register coord *mtrk;
        coord poss[9];

        cnt = mfndpos(mtmp, poss, info, flag);
        chcnt = 0;
        jcnt = min(MTSZ, cnt - 1);
        chi = -1;
        nidist = dist2(nix, niy, gx, gy);
        /* allow monsters be shortsighted on some levels for balance */
        if (!mtmp->mpeaceful && level.flags.shortsighted
            && nidist > (couldsee(nix, niy) ? 144 : 36) && appr == 1)
            appr = 0;
        if (is_unicorn(ptr) && level.flags.noteleport) {
            /* on noteleport levels, perhaps we cannot avoid hero */
            for (i = 0; i < cnt; i++)
                if (!(info[i] & NOTONL))
                    avoid = TRUE;
        }
        better_with_displacing =
            should_displace(mtmp, poss, info, cnt, gx, gy);

        /* Check once if this monster uses pathfinding, and ensure map is
           current before the loop rather than checking every iteration */
        {
            /* Amulet carriers ALWAYS use escape pathfinding regardless
               of appr, because they need to reach the escape target
               (stairs/portal). Other intelligent monsters use player
               pathfinding when approaching */
            boolean use_escape_pf = (mon_has_amulet(mtmp)
                                     && mon_uses_pathfinding(mtmp)
                                     && gx && gy);
            boolean use_pathfinding = (!use_escape_pf && appr == 1
                                       && mon_uses_pathfinding(mtmp));
            short cur_pfdist = PATHFIND_UNREACHABLE;

            if (use_escape_pf) {
                ensure_escape_pathfind_map(gx, gy);
                cur_pfdist = escape_pathfind_dist[nix][niy];
            } else if (use_pathfinding) {
                ensure_pathfind_map();
                cur_pfdist = pathfind_dist[nix][niy];
            }

        for (i = 0; i < cnt; i++) {
            if (avoid && (info[i] & NOTONL))
                continue;
            nx = poss[i].x;
            ny = poss[i].y;

            if (MON_AT(nx, ny) && (info[i] & ALLOW_MDISP)
                && !(info[i] & ALLOW_M) && !better_with_displacing)
                continue;
            if (appr != 0) {
                mtrk = &mtmp->mtrack[0];
                for (j = 0; j < jcnt; mtrk++, j++)
                    if (nx == mtrk->x && ny == mtrk->y)
                        if (rn2(4 * (cnt - j)))
                            goto nxti;
            }

            /* Intelligent monsters use pathfinding when approaching.
               Regular monsters use BFS from player position. Amulet
               carriers use BFS from escape target (stairs/portal) */
            if (use_pathfinding || use_escape_pf) {
                short new_pfdist = use_escape_pf ? escape_pathfind_dist[nx][ny]
                                                 : pathfind_dist[nx][ny];

                /*
                 * Pick the reachable adjacent square with minimum distance.
                 * To prevent oscillation when backtracking around obstacles:
                 * - mmoved=1: found a non-mtrack square (preferred)
                 * - mmoved=2: only found mtrack squares so far (fallback)
                 * Non-mtrack squares always beat mtrack squares.
                 */
                if (new_pfdist != PATHFIND_UNREACHABLE) {
                    boolean in_mtrack = FALSE;
                    boolean dominated, should_update = FALSE;
                    int new_mmoved = mmoved;

                    /* Is this square in recent movement track? */
                    mtrk = &mtmp->mtrack[0];
                    for (j = 0; j < jcnt; mtrk++, j++) {
                        if (nx == mtrk->x && ny == mtrk->y) {
                            in_mtrack = TRUE;
                            break;
                        }
                    }

                    /* Is this worse than current best candidate? */
                    dominated = (cur_pfdist != PATHFIND_UNREACHABLE
                                 && new_pfdist > cur_pfdist);

                    if (in_mtrack) {
                        /* mtrack: take if first, or if no non-mtrack yet
                           and not worse than current best */
                        if (!mmoved || (mmoved == 2 && !dominated)) {
                            should_update = TRUE;
                            new_mmoved = 2;
                        }
                    } else {
                        /* non-mtrack: always preferred over mtrack;
                           take if replacing mtrack, first, or better */
                        if (mmoved != 1 || !dominated) {
                            should_update = TRUE;
                            if (mmoved != 1)
                                new_mmoved = 1;
                        }
                    }

                    if (should_update) {
                        nix = nx;
                        niy = ny;
                        cur_pfdist = new_pfdist;
                        chi = i;
                        mmoved = new_mmoved;
                    }
                } else if (!mmoved) {
                    /* Unreachable - fallback to greedy if no candidate yet */
                    nearer = ((ndist = dist2(nx, ny, gx, gy)) < nidist);
                    if (nearer) {
                        nix = nx;
                        niy = ny;
                        nidist = ndist;
                        cur_pfdist = PATHFIND_UNREACHABLE;
                        chi = i;
                        mmoved = 1;
                    }
                }
            } else {
                /* Original greedy behavior for non-intelligent or fleeing */
                nearer = ((ndist = dist2(nx, ny, gx, gy)) < nidist);

                if ((appr == 1 && nearer) || (appr == -1 && !nearer)
                    || (!appr && !rn2(++chcnt)) || !mmoved) {
                    nix = nx;
                    niy = ny;
                    nidist = ndist;
                    chi = i;
                    mmoved = 1;
                }
            }
 nxti:
            ;
        }
        } /* end use_pathfinding/cur_pfdist scope */
    }

    if (mmoved) {
        register int j;

        if (mmoved == 1 && (u.ux != nix || u.uy != niy) && itsstuck(mtmp))
            return 3;

        if (mmoved == 1 && m_digweapon_check(mtmp, nix,niy))
            return 3;

        /* If ALLOW_U is set, either it's trying to attack you, or it
         * thinks it is.  In either case, attack this spot in preference to
         * all others.
         */
        /* Actually, this whole section of code doesn't work as you'd expect.
         * Most attacks are handled in dochug().  It calls distfleeck(), which
         * among other things sets nearby if the monster is near you--and if
         * nearby is set, we never call m_move unless it is a special case
         * (confused, stun, etc.)  The effect is that this ALLOW_U (and
         * mfndpos) has no effect for normal attacks, though it lets a
         * confused monster attack you by accident.
         */
        if (info[chi] & ALLOW_U) {
            nix = mtmp->mux;
            niy = mtmp->muy;
        }
        if (nix == u.ux && niy == u.uy) {
            mtmp->mux = u.ux;
            mtmp->muy = u.uy;
            return 0;
        }
        /* The monster may attack another based on 1 of 2 conditions:
         * 1 - It may be confused.
         * 2 - It may mistake the monster for your (displaced) image.
         * Pets get taken care of above and shouldn't reach this code.
         * Conflict gets handled even farther away (movemon()).
         */
        if ((info[chi] & ALLOW_M) != 0
            || (nix == mtmp->mux && niy == mtmp->muy)) {
            return m_move_aggress(mtmp, nix, niy);
        }

        /* hiding-under monsters will attack things from their hiding spot but
         * are less likely to venture out */
        if (hides_under(ptr) && concealed_spot(mtmp->mx, mtmp->my)
            && !concealed_spot(nix, niy) && rn2(10)) {
            return 0;
        }

        if ((info[chi] & ALLOW_MDISP) != 0) {
            struct monst *mtmp2;
            int mstatus;

            mtmp2 = m_at(nix, niy); /* ALLOW_MDISP implies m_at() is !Null */
            mstatus = mdisplacem(mtmp, mtmp2, FALSE);
            if (mstatus & (MM_AGR_DIED | MM_DEF_DIED))
                return 2;
            if (mstatus & MM_HIT)
                return 1;
            return 3;
        }

        if (!m_in_out_region(mtmp, nix, niy))
            return 3;

        if ((info[chi] & ALLOW_ROCK) && m_can_break_boulder(mtmp)) {
            (void) m_break_boulder(mtmp, nix, niy);
            return 3;
        }

        /* move a normal monster; for a long worm, remove_monster() and
           place_monster() only manipulate the head; they leave tail as-is */
        remove_monster(omx, omy);
        place_monster(mtmp, nix, niy);
        /* for a long worm, insert a new segment to reconnect the head
           with the tail; worm_move() keeps the end of the tail if worm
           is scheduled to grow, removes that for move-without-growing */
        if (mtmp->wormno)
            worm_move(mtmp);

        maybe_unhide_at(mtmp->mx, mtmp->my);

        for (j = MTSZ - 1; j > 0; j--)
            mtmp->mtrack[j] = mtmp->mtrack[j - 1];
        mtmp->mtrack[0].x = omx;
        mtmp->mtrack[0].y = omy;

        /* Intelligent Amulet carriers immediately escape via stairs/portal.
           Update old position display before potential early return. */
        if (mon_has_amulet(mtmp) && !mindless(ptr) && !is_animal(ptr)) {
            int escape = mon_escape_with_amulet(mtmp);
            if (escape) {
                newsym(omx, omy); /* clear monster from where it came from */
                return escape;
            }
        }
    } else {
        if (is_unicorn(ptr) && rn2(2) && !tele_restrict(mtmp)) {
            (void) rloc(mtmp, TRUE);
            return 1;
        }
        /* for a long worm, shrink it (by discarding end of tail)
           when it has failed to move (because a turn passed and it
           didn't move, or a monster/terrain blocked its path - not
           from being confused/stunned/trapped/etc) */
        if (mtmp->wormno
            && !(mtmp->mconf || mtmp->mstun || mtmp->mtrapped
                 || mtmp->msleeping || mtmp->mentangled))
            worm_nomove(mtmp);
    }
 postmov:
    if (mmoved == 1 || mmoved == 3) {
        boolean canseeit = cansee(mtmp->mx, mtmp->my),
                didseeit = canseeit;

        if (mmoved == 1) {
            /* normal monster move will already have <nix,niy>,
               but pet dog_move() with 'goto postmov' won't */
            nix = mtmp->mx, niy = mtmp->my;
            /* sequencing issue:  when monster movement decides that a
               monster can move to a door location, it moves the monster
               there before dealing with the door rather than after;
               so a vampire/bat that is going to shift to fog cloud and
               pass under the door is already there but transformation
               into fog form--and its message, when in sight--has not
               happened yet; we have to move monster back to previous
               location before performing the vamp_shift() to make the
               message happen at right time, then back to the door again
               [if we did the shift above, before moving the monster,
               we would need to duplicate it in dog_move()...] */
            if (is_vampshifter(mtmp) && !amorphous(mtmp->data)
                && IS_DOOR(levl[nix][niy].typ)
                && ((levl[nix][niy].doormask & (D_LOCKED | D_CLOSED)) != 0)
                && can_fog(mtmp)) {
                if (sawmon) {
                    remove_monster(nix, niy);
                    place_monster(mtmp, omx, omy);
                    newsym(nix, niy), newsym(omx, omy);
                }
                if (vamp_shift(mtmp, &mons[PM_FOG_CLOUD], sawmon)) {
                    ptr = mtmp->data; /* update cached value */
                }
                if (sawmon) {
                    remove_monster(omx, omy);
                    place_monster(mtmp, nix, niy);
                    newsym(omx, omy), newsym(nix, niy);
                }
            }

            newsym(omx, omy); /* update the old position */
            if (mintrap(mtmp) >= 2) {
                if (mtmp->mx)
                    newsym(mtmp->mx, mtmp->my);
                return 2; /* it died */
            }
            ptr = mtmp->data; /* in case mintrap() caused polymorph */

            /* open a door, or crash through it, if 'mtmp' can */
            if (IS_DOOR(levl[mtmp->mx][mtmp->my].typ)
                && !passes_walls(ptr) /* doesn't need to open doors */
                && !can_tunnel) {     /* taken care of below */
                struct rm *here = &levl[mtmp->mx][mtmp->my];
                boolean btrapped = (here->doormask & D_TRAPPED) != 0;
    /* used after monster 'who' has been moved to closed door spot 'where'
       which will now be changed to door state 'what' with map update */
#define UnblockDoor(where, who, what) \
    do {                                                        \
        (where)->doormask = (what);                             \
        newsym((who)->mx, (who)->my);                           \
        unblock_point((who)->mx, (who)->my);                    \
        vision_recalc(0);                                       \
        /* update cached value since it might change */         \
        canseeit = didseeit || cansee((who)->mx, (who)->my);    \
    } while (0)

                /* if mon has MKoT, disarm door trap; no message given */
                if (btrapped && has_roguish_key(mtmp)) {
                    /* BUG: this lets a vampire or blob or a doorbuster
                       holding the Key disarm the trap even though it isn't
                       using that Key when squeezing under or smashing the
                       door.  Not significant enough to worry about; perhaps
                       the Key's magic is more powerful for monsters? */
                    here->doormask &= ~D_TRAPPED;
                    btrapped = FALSE;
                }
                if ((here->doormask & (D_LOCKED | D_CLOSED)) != 0
                    && amorphous(ptr)) {
                    if (flags.verbose && canseemon(mtmp))
                        pline("%s %s under the door.", Monnam(mtmp),
                              (ptr == &mons[PM_FOG_CLOUD]
                               || ptr->mlet == S_LIGHT) ? "flows" : "oozes");
                } else if (here->doormask & D_LOCKED && can_unlock) {
                    /* like the vampshift hack above, there are sequencing
                       issues when the monster is moved to the door's spot
                       first then door handling plus feedback comes after */

                    UnblockDoor(here, mtmp, !btrapped ? D_ISOPEN : D_NODOOR);
                    if (btrapped) {
                        if (mb_trapped(mtmp))
                            return 2;
                    } else {
                        if (flags.verbose) {
                            if (canseeit && canspotmon(mtmp))
                                pline("%s unlocks and opens a door.",
                                      Monnam(mtmp));
                            else if (canseeit)
                                You_see("a door unlock and open.");
                            else if (!Deaf)
                                You_hear("a door unlock and open.");
                        }
                    }
                } else if (here->doormask == D_CLOSED && can_open) {
                    UnblockDoor(here, mtmp, !btrapped ? D_ISOPEN : D_NODOOR);
                    if (btrapped) {
                        if (mb_trapped(mtmp))
                            return 2;
                    } else {
                        if (flags.verbose) {
                            if (canseeit && canspotmon(mtmp))
                                pline("%s opens a door.", Monnam(mtmp));
                            else if (canseeit)
                                You_see("a door open.");
                            else if (!Deaf)
                                You_hear("a door open.");
                        }
                    }
                } else if (here->doormask & (D_LOCKED | D_CLOSED)) {
                    /* mfndpos guarantees this must be a doorbuster */
                    unsigned mask;

                    mask = ((btrapped || ((here->doormask & D_LOCKED) != 0
                                          && !rn2(2))) ? D_NODOOR
                            : D_BROKEN);
                    UnblockDoor(here, mtmp, mask);
                    if (btrapped) {
                        if (mb_trapped(mtmp))
                            return 2;
                    } else {
                        if (flags.verbose) {
                            if (canseeit && canspotmon(mtmp))
                                pline("%s smashes down a door.",
                                      Monnam(mtmp));
                            else if (canseeit)
                                You_see("a door crash open.");
                            else if (!Deaf)
                                You_hear("a door crash open.");
                        }
                    }
                    /* if it's a shop or temple door, schedule repair */
                    if (*in_rooms(mtmp->mx, mtmp->my, SHOPBASE))
                        add_damage(mtmp->mx, mtmp->my, 0L);
                    else if (temple_at_boundary(mtmp->mx, mtmp->my))
                        add_damage(mtmp->mx, mtmp->my, 0L);
                }
            } else if (levl[mtmp->mx][mtmp->my].typ == IRONBARS) {
                /* As of 3.6.2: was using may_dig() but it doesn't handle bars */
                if (!(levl[mtmp->mx][mtmp->my].wall_info & W_NONDIGGABLE)
                    && (dmgtype(ptr, AD_RUST) || dmgtype(ptr, AD_CORR))
                    && !(mtmp->data == &mons[PM_WATER_ELEMENTAL]
                         || mtmp->data == &mons[PM_BABY_SEA_DRAGON]
                         || mtmp->data == &mons[PM_SEA_DRAGON])) {
                    if (canseemon(mtmp))
                        pline("%s eats through the iron bars.", Monnam(mtmp));
                    /* add_damage before terrain change so original type is saved */
                    if (*in_rooms(mtmp->mx, mtmp->my, SHOPBASE))
                        add_damage(mtmp->mx, mtmp->my, 0L);
                    else if (temple_at_boundary(mtmp->mx, mtmp->my))
                        add_damage(mtmp->mx, mtmp->my, 0L);
                    dissolve_bars(mtmp->mx, mtmp->my);
                    return 3;
                } else if (flags.verbose && canseemon(mtmp))
                    Norep("%s %s %s the iron bars.", Monnam(mtmp),
                          /* pluralization fakes verb conjugation */
                          makeplural(locomotion(ptr, "pass")),
                          (passes_walls(ptr) || unsolid(ptr)) ? "through" : "between");
            }

            (void) maybe_freeze_underfoot(mtmp);

            /* possibly dig */
            if (can_tunnel && mdig_tunnel(mtmp))
                return 2; /* mon died (position already updated) */

            /* set also in domove(), hack.c */
            if (u.uswallow && mtmp == u.ustuck
                && (mtmp->mx != omx || mtmp->my != omy)) {
                /* If the monster moved, then update */
                u.ux0 = u.ux;
                u.uy0 = u.uy;
                u.ux = mtmp->mx;
                u.uy = mtmp->my;
                swallowed(0);
            } else
                newsym(mtmp->mx, mtmp->my);
        }
        if ((OBJ_AT(mtmp->mx, mtmp->my)
            || IS_MAGIC_CHEST(levl[mtmp->mx][mtmp->my].typ))
            && mtmp->mcanmove) {
            /* recompute the likes tests, in case we polymorphed
             * or if the "likegold" case got taken above */
            if (setlikes) {
                int pctload = (curr_mon_load(mtmp) * 100) / max_mon_load(mtmp);

                /* look for gold or jewels nearby */
                likegold = (likes_gold(ptr) && pctload < 95
                            && !can_levitate(mtmp));
                likegems = (likes_gems(ptr) && pctload < 85
                            && !can_levitate(mtmp));
                uses_items =
                    (!mindless(ptr) && !is_animal(ptr) && pctload < 75);
                likeobjs = (likes_objs(ptr) && pctload < 75
                            && !can_levitate(mtmp));
                likemagic = (likes_magic(ptr) && pctload < 85
                             && !can_levitate(mtmp));
                likerock = (racial_throws_rocks(mtmp) && pctload < 50 && !Sokoban
                            && !can_levitate(mtmp));
                conceals = hides_under(ptr);
            }

            /* Maybe a rock mole just ate some metal object */
            if (metallivorous(ptr)) {
                if (meatmetal(mtmp) == 2)
                    return 2; /* it died */
            }

            if (g_at(mtmp->mx, mtmp->my) && likegold)
                mpickgold(mtmp);

            /* Maybe a cube ate just about anything */
            if (ptr == &mons[PM_GELATINOUS_CUBE]) {
                if (meatobj(mtmp) == 2)
                    return 2; /* it died */
            }

            /* Maybe a honey badger raided a beehive */
            if (ptr == &mons[PM_HONEY_BADGER]) {
                if (meatjelly(mtmp) == 2)
                    return 2; /* it died */
            }

            /* Maybe a creeping mound ate all the food */
            if (ptr == &mons[PM_CREEPING_MOUND]) {
                if (meatfood(mtmp) == 2)
                    return 2; /* it died */
            }

            /* Maybe Gollum had a snack */
            if (ptr == &mons[PM_GOLLUM]) {
                if (gollum_eat(mtmp) == 2)
                    return 2; /* it died */
            }

            if (!*in_rooms(mtmp->mx, mtmp->my, SHOPBASE) || !rn2(25)) {
                boolean picked = FALSE;

                int mhp = mtmp->mhp;

                if (likeobjs)
                    picked |= mpickstuff(mtmp, practical);
                if (mhp > mtmp->mhp) {
                    mmoved = 3;
                    goto end;
                }

                if (likemagic)
                    picked |= mpickstuff(mtmp, magical);
                if (mhp > mtmp->mhp) {
                    mmoved = 3;
                    goto end;
                }

                if (likerock)
                    picked |= mpickstuff(mtmp, boulder_class);
                if (mhp > mtmp->mhp) {
                    mmoved = 3;
                    goto end;
                }

                if (likegems)
                    picked |= mpickstuff(mtmp, gem_class);
                if (mhp > mtmp->mhp) {
                    mmoved = 3;
                    goto end;
                }

                if (uses_items)
                    picked |= mpickstuff(mtmp, (char *) 0);
                if (mhp > mtmp->mhp) {
                    mmoved = 3;
                    goto end;
                }

                if (picked)
                    mmoved = 3;
            }

            if (mtmp->minvis) {
                newsym(mtmp->mx, mtmp->my);
                if (mtmp->wormno)
                    see_wsegs(mtmp);
            }
        }

        if (hides_under(ptr) || ptr->mlet == S_EEL
            || ptr == &mons[PM_GIANT_LEECH]) {
            /* Always set--or reset--mundetected if it's already hidden
               (just in case the object it was hiding under went away);
               usually set mundetected unless monster can't move.  */
            if (mtmp->mundetected
                || (mtmp->mcanmove && !mtmp->msleeping && rn2(5)))
                (void) hideunder(mtmp);
            newsym(mtmp->mx, mtmp->my);
        }
end:
        if (mtmp->isshk) {
            after_shk_move(mtmp);
        }
    }
    return mmoved;
}

/* From xNetHack...
 * The part of m_move that deals with a monster attacking another monster (and
 * that monster possibly retaliating).
 * Extracted into its own function so that it can be called with monsters that
 * have special move patterns (shopkeepers, priests, etc) that want to attack
 * other monsters but aren't just roaming freely around the level (so allowing
 * m_move to run fully for them could select an invalid move).
 * x and y are the coordinates mtmp wants to attack.
 * Return values are the same as for m_move, but this function only return 2
 * (mtmp died) or 3 (mtmp made its move).
 */
int
m_move_aggress(mtmp, x, y)
struct monst * mtmp;
xchar x, y;
{
    struct monst *mtmp2;
    int mstatus = 0;

    mtmp2 = m_at(x, y);

    if (mtmp2) {
        bhitpos.x = x, bhitpos.y = y;
        notonhead = (x != mtmp2->mx || y != mtmp2->my);
        mstatus = mattackm(mtmp, mtmp2);
    }

    if (mstatus & MM_AGR_DIED) /* aggressor died */
        return 2;

    if ((mstatus & (MM_HIT | MM_DEF_DIED)) == MM_HIT
        && rn2(4) && mtmp2->movement > rn2(NORMAL_SPEED)) {
        if (mtmp2->movement > NORMAL_SPEED)
            mtmp2->movement -= NORMAL_SPEED;
        else
            mtmp2->movement = 0;
        bhitpos.x = mtmp->mx, bhitpos.y = mtmp->my;
        notonhead = FALSE;
        mstatus = mattackm(mtmp2, mtmp); /* return attack */
        /* note: at this point, defender is the original (moving) aggressor */
        if (mstatus & MM_DEF_DIED)
            return 2;
    }
    return 3;
}

/* Return 1 or 2 if a hides_under monster can conceal itself at this spot.
 * If the monster can hide under an object, return 2.
 * Otherwise, monsters can hide in grass and under some types of dungeon
 * furniture. If no object is available but the terrain is suitable, return 1.
 * Return 0 if the monster can't conceal itself.
 */
int
concealed_spot(x, y)
register int x, y;
{
    if (OBJ_AT(x, y))
        return 2;
    switch (levl[x][y].typ) {
    case GRASS:
    case SAND:
    case SINK:
    case ALTAR:
    case THRONE:
    case LADDER:
    case GRAVE:
        return 1;
    default:
        return 0;
    }
}

void
dissolve_bars(x, y)
register int x, y;
{
    levl[x][y].typ = (Is_special(&u.uz) || *in_rooms(x, y, 0)) ? ROOM : CORR;
    levl[x][y].flags = 0;
    newsym(x, y);
}

boolean
closed_door(x, y)
register int x, y;
{
    return (boolean) (IS_DOOR(levl[x][y].typ)
                      && (levl[x][y].doormask & (D_LOCKED | D_CLOSED)));
}

boolean
accessible(x, y)
register int x, y;
{
    int levtyp = levl[x][y].typ;

    /* use underlying terrain in front of closed drawbridge */
    if (levtyp == DRAWBRIDGE_UP)
        levtyp = db_under_typ(levl[x][y].drawbridgemask);

    return (boolean) (ACCESSIBLE(levtyp) && !closed_door(x, y));
}

/* decide where the monster thinks you are standing */
void
set_apparxy(mtmp)
struct monst *mtmp;
{
    boolean notseen, gotu;
    register int disp, mx = mtmp->mux, my = mtmp->muy;
    long umoney = money_cnt(invent);

    /*
     * do cheapest and/or most likely tests first
     */

    /* pet knows your smell; grabber still has hold of you */
    if (mtmp->mtame || mtmp == u.ustuck)
        goto found_you;

    /* monsters which know where you are don't suddenly forget,
       if you haven't moved away */
    if (mx == u.ux && my == u.uy)
        goto found_you;

    /* monster can see you via cooperative telepathy */
    if (has_telepathy(mtmp) && (HTelepat || ETelepat))
        goto found_you;

    notseen = (!mtmp->mcansee || (Invis && !mon_prop(mtmp, SEE_INVIS)));
    /* add cases as required.  eg. Displacement ... */
    if (notseen || Underwater) {
        /* Xorns can smell quantities of valuable metal
            like that in solid gold coins, treat as seen */
        if ((mtmp->data == &mons[PM_XORN]) && umoney && !Underwater)
            disp = 0;
        else
            disp = 1;
    } else if (Displaced) {
        disp = couldsee(mx, my) ? 2 : 1;
    } else
        disp = 0;
    if (!disp)
        goto found_you;

    /* without something like the following, invisibility and displacement
       are too powerful */
    gotu = notseen ? !rn2(3) : Displaced ? !rn2(4) : FALSE;

    if (!gotu) {
        register int try_cnt = 0;

        do {
            if (++try_cnt > 200)
                goto found_you; /* punt */
            mx = u.ux - disp + rn2(2 * disp + 1);
            my = u.uy - disp + rn2(2 * disp + 1);
        } while (!isok(mx, my)
                 || (disp != 2 && mx == mtmp->mx && my == mtmp->my)
                 || ((mx != u.ux || my != u.uy) && !passes_walls(mtmp->data)
                     && !(accessible(mx, my)
                          || (closed_door(mx, my)
                              && (can_ooze(mtmp) || can_fog(mtmp)))))
                 || !couldsee(mx, my));
    } else {
 found_you:
        mx = u.ux;
        my = u.uy;
    }

    mtmp->mux = mx;
    mtmp->muy = my;
}

/*
 * mon-to-mon displacement is a deliberate "get out of my way" act,
 * not an accidental bump, so we don't consider mstun or mconf in
 * undesired_disp().
 *
 * We do consider many other things about the target and its
 * location however.
 */
boolean
undesirable_disp(mtmp, x, y)
struct monst *mtmp; /* barging creature */
xchar x, y; /* spot 'mtmp' is considering moving to */
{
    boolean is_pet = (mtmp && mtmp->mtame && !mtmp->isminion);
    struct trap *trap = t_at(x, y);

    if (is_pet) {
        /* Pets avoid a trap if you've seen it usually,
           unless ordered to ignore harmless traps */
        if (trap && trap->tseen && rn2(40)) {
            struct edog *edog = EDOG(mtmp);
            boolean dominated = edog
                && (edog->petstrat & PETSTRAT_IGNORETRAPS)
                && (trap->ttyp == SQKY_BOARD
                    || (trap->ttyp == RUST_TRAP_SET
                        && mtmp->data != &mons[PM_IRON_GOLEM]));
            if (!dominated)
                return TRUE;
        }
        /* Pets avoid cursed locations */
        if (cursed_object_at(x, y))
            return TRUE;

    /* Monsters avoid a trap if they've seen that type before */
    } else if (trap && rn2(40)
               && (mtmp->mtrapseen & (1 << (trap->ttyp - 1))) != 0) {
        return TRUE;
    }

    /* oversimplification:  creatures that bargethrough can't swap places
       when target monster is in rock or closed door or water (in particular,
       avoid moving to spots where mondied() won't leave a corpse; doesn't
       matter whether barger is capable of moving to such a target spot if
       it were unoccupied) */
    if (!accessible(x, y)
        /* mondied() allows is_pool() as an exception to !accessible(),
           but we'll only do that if 'mtmp' is already at a water location
           so that we don't swap a water critter onto land */
        && !(is_pool(x, y) && is_pool(mtmp->mx, mtmp->my)))
        return TRUE;

    return FALSE;
}

/*
 * Inventory prevents passage under door.
 * Used by can_ooze() and can_fog().
 */
STATIC_OVL boolean
stuff_prevents_passage(mtmp)
struct monst *mtmp;
{
    struct obj *chain, *obj;

    if (vampire_form && is_whirly(youmonst.data))
        return FALSE;

    if (mtmp == &youmonst) {
        chain = invent;
    } else {
        chain = mtmp->minvent;
    }
    for (obj = chain; obj; obj = obj->nobj) {
        int typ = obj->otyp;

        if (typ == COIN_CLASS && obj->quan > 100L)
            return TRUE;
        if (obj->oclass != GEM_CLASS && !(typ >= ARROW && typ <= BOOMERANG)
            && !(typ >= DAGGER && typ <= CRYSKNIFE) && typ != SLING
            && !is_cloak(obj) && typ != FEDORA && typ != TOQUE && !is_gloves(obj)
            && typ != JACKET && typ != CREDIT_CARD && !is_shirt(obj)
            && !(typ == CORPSE && verysmall(&mons[obj->corpsenm]))
            && typ != FORTUNE_COOKIE && typ != CANDY_BAR && typ != PANCAKE
            && typ != LEMBAS_WAFER && typ != LUMP_OF_ROYAL_JELLY
            && obj->oclass != AMULET_CLASS && obj->oclass != RING_CLASS
            && obj->oclass != VENOM_CLASS && typ != SACK
            && typ != BAG_OF_HOLDING && typ != BAG_OF_TRICKS
            && !Is_candle(obj) && typ != OILSKIN_SACK && typ != LEASH
            && typ != STETHOSCOPE && typ != BLINDFOLD && typ != TOWEL
            && typ != PEA_WHISTLE && typ != MAGIC_WHISTLE
            && typ != MAGIC_MARKER && typ != TIN_OPENER && typ != SKELETON_KEY
            && typ != LOCK_PICK)
            return TRUE;
        if (Is_container(obj) && obj->cobj)
            return TRUE;
    }
    return FALSE;
}

boolean
can_ooze(mtmp)
struct monst *mtmp;
{
    if (!amorphous(mtmp->data) || stuff_prevents_passage(mtmp))
        return FALSE;
    return TRUE;
}

/* monster can change form into a fog if necessary */
boolean
can_fog(mtmp)
struct monst *mtmp;
{
    if (!(mvitals[PM_FOG_CLOUD].mvflags & G_GENOD) && is_vampshifter(mtmp)
        && !Protection_from_shape_changers && !stuff_prevents_passage(mtmp)
        && !mtmp->mtame)
        return TRUE;
    return FALSE;
}

STATIC_OVL int
vamp_shift(mon, ptr, domsg)
struct monst *mon;
struct permonst *ptr;
boolean domsg;
{
    int reslt = 0;
    char oldmtype[BUFSZ];

    /* remember current monster type before shapechange */
    Strcpy(oldmtype, domsg ? noname_monnam(mon, ARTICLE_THE) : "");

    if (mon->data == ptr) {
        /* already right shape */
        reslt = 1;
        domsg = FALSE;
    } else if (is_vampshifter(mon)) {
        reslt = newcham(mon, ptr, FALSE, FALSE);
    }

    if (reslt && domsg) {
        pline("You %s %s where %s was.",
              !canseemon(mon) ? "now detect" : "observe",
              noname_monnam(mon, ARTICLE_A), oldmtype);
        /* this message is given when it turns into a fog cloud
           in order to move under a closed door */
        display_nhwindow(WIN_MESSAGE, FALSE);
    }

    return reslt;
}

/* returns 0 if terrain not frozen, 1 if frozen */
int
maybe_freeze_underfoot(mtmp)
struct monst *mtmp;
{
    struct rm *lev;
    boolean was_lava, was_sewage, is_you = (mtmp == &youmonst);
    coord cc;
    if (!mtmp || !has_cold_feet(mtmp) || (is_you && (Flying || Levitation))
        || !grounded(mtmp->data) || (u.usteed && !grounded(u.usteed->data)))
        return 0;

    if (is_you) {
        cc.x = u.ux, cc.y = u.uy;
    } else if (u.usteed) {
        cc.x = u.usteed->mx, cc.y = u.usteed->my;
    } else {
        cc.x = mtmp->mx, cc.y = mtmp->my;
    }

    if (!is_damp_terrain(cc.x, cc.y) && !is_lava(cc.x, cc.y))
        return 0;

    lev = &levl[cc.x][cc.y];
    was_lava = is_lava(cc.x, cc.y);
    was_sewage = is_sewage(cc.x, cc.y);

    if (lev->typ == DRAWBRIDGE_UP) {
        lev->drawbridgemask &= ~DB_UNDER;
        lev->drawbridgemask |= was_lava ? DB_FLOOR : DB_ICE;
    } else {
        switch (lev->typ) {
        case POOL:
            lev->icedpool = ICED_POOL;
            break;
        case PUDDLE:
            lev->icedpool = ICED_PUDDLE;
            break;
        case SEWAGE:
            lev->icedpool = ICED_SEWAGE;
            break;
        case MOAT:
        case WATER:
            lev->icedpool = ICED_MOAT;
            break;
        default:
            lev->icedpool = 0;
            break;
        }
        lev->typ = was_lava ? ROOM : ICE;
    }

    if (lev->icedpool != ICED_PUDDLE && lev->icedpool != ICED_SEWAGE)
        bury_objs(cc.x, cc.y);

    if (is_you || u.usteed || canseemon(mtmp)) {
        const char *liq = was_lava ? "lava" : was_sewage ? "sewage" : "water";

        Norep("The %s %s under %s %s.", hliquid(liq),
              was_lava ? "cools and solidifies" : "crackles and freezes",
              u.usteed ? s_suffix(mon_nam(u.usteed))
                       : is_you ? "your"
                                : s_suffix(mon_nam(mtmp)),
              makeplural(mbodypart(mtmp, FOOT)));
    }

    if (!was_lava) {
        start_melt_ice_timeout(cc.x, cc.y, 0L);
        obj_ice_effects(cc.x, cc.y, TRUE);
    }
    return 1;
}

/*monmove.c*/
