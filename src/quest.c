/* NetHack 3.6	quest.c	$NHDT-Date: 1505170343 2017/09/11 22:52:23 $  $NHDT-Branch: NetHack-3.6.0 $:$NHDT-Revision: 1.21 $ */
/*      Copyright 1991, M. Stephenson             */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/*  quest dungeon branch routines. */

#include "quest.h"
#include "qtext.h"

#define Not_firsttime (on_level(&u.uz0, &u.uz))
#define Qstat(x) (quest_status.x)

STATIC_DCL void NDECL(on_start);
STATIC_DCL void NDECL(on_locate);
STATIC_DCL void NDECL(on_goal);
STATIC_DCL boolean NDECL(not_capable);
STATIC_DCL int FDECL(is_pure, (BOOLEAN_P));
STATIC_DCL void FDECL(expulsion, (BOOLEAN_P));
STATIC_DCL void NDECL(chat_with_leader);
STATIC_DCL void NDECL(chat_with_nemesis);
STATIC_DCL void NDECL(chat_with_guardian);
STATIC_DCL void FDECL(prisoner_speaks, (struct monst *));

STATIC_OVL void
on_start()
{
    if (!Qstat(first_start)) {
        qt_pager(QT_FIRSTTIME);
        Qstat(first_start) = TRUE;
    } else if ((u.uz0.dnum != u.uz.dnum) || (u.uz0.dlevel < u.uz.dlevel)) {
        if (Qstat(not_ready) <= 2)
            qt_pager(QT_NEXTTIME);
        else
            qt_pager(QT_OTHERTIME);
    }
}

STATIC_OVL void
on_locate()
{
    /* the locate messages are phrased in a manner such that they only
       make sense when arriving on the level from above */
    boolean from_above = (u.uz0.dlevel < u.uz.dlevel);

    if (Qstat(killed_nemesis)) {
        return;
    } else if (!Qstat(first_locate)) {
        if (from_above)
            qt_pager(QT_FIRSTLOCATE);
        /* if we've arrived from below this will be a lie, but there won't
           be any point in delivering the message upon a return visit from
           above later since the level has now been seen */
        Qstat(first_locate) = TRUE;
    } else {
        if (from_above)
            qt_pager(QT_NEXTLOCATE);
    }
}

STATIC_OVL void
on_goal()
{
    if (Qstat(killed_nemesis)) {
        return;
    } else if (!Qstat(made_goal)) {
        qt_pager(QT_FIRSTGOAL);
        Qstat(made_goal) = 1;
    } else {
        /*
         * Some QT_NEXTGOAL messages reference the quest artifact;
         * find out if it is still present.  If not, request an
         * alternate message (qt_pager() will revert to delivery
         * of QT_NEXTGOAL if current role doesn't have QT_ALTGOAL).
         * Note: if hero is already carrying it, it is treated as
         * being absent from the level for quest message purposes.
         */
        unsigned whichobjchains = ((1 << OBJ_FLOOR)
                                   | (1 << OBJ_MINVENT)
                                   | (1 << OBJ_BURIED));
        struct obj *qarti = find_quest_artifact(whichobjchains);

        qt_pager(qarti ? QT_NEXTGOAL : QT_ALTGOAL);
        if (Qstat(made_goal) < 7)
            Qstat(made_goal)++;
    }
}

void
onquest()
{
    if (u.uevent.qcompleted || Not_firsttime)
        return;
    if (!Is_special(&u.uz))
        return;

    if (Is_qstart(&u.uz))
        on_start();
    else if (Is_qlocate(&u.uz))
        on_locate();
    else if (Is_nemesis(&u.uz))
        on_goal();
    return;
}

void
nemdead()
{
    if (!Qstat(killed_nemesis)) {
        Qstat(killed_nemesis) = TRUE;
        qt_pager(QT_KILLEDNEM);
        /* player had to kill the quest leader to
           continue - in this case, killing the quest
           nemesis marks the quest as complete */
        if (quest_status.killed_leader)
            u.uevent.qcompleted = 1;
    }
}

void
leaddead()
{
    if (!Qstat(killed_leader)) {
        Qstat(killed_leader) = TRUE;
        /* player killed the quest nemesis,
           came back with the quest artifact,
           but made the quest leader angry
           before talking to them to flag
           quest as complete */
        if (quest_status.killed_nemesis)
            u.uevent.qcompleted = 1;
    }
}

void
artitouch(obj)
struct obj *obj;
{
    if (!Qstat(touched_artifact)) {
        /* in case we haven't seen the item yet (ie, currently blinded),
           this quest message describes it by name so mark it as seen */
        obj->dknown = 1;
        /* only give this message once */
        Qstat(touched_artifact) = TRUE;
        if (quest_status.leader_is_dead)
            qt_pager(QT_GOTIT2);
        else
            qt_pager(QT_GOTIT);
        exercise(A_WIS, TRUE);
    }
}

/* external hook for do.c (level change check) */
boolean
ok_to_quest()
{
    return (boolean) (((Qstat(got_quest) || Qstat(got_thanks))
                      && is_pure(FALSE) > 0)  || Qstat(killed_leader));
}

STATIC_OVL boolean
not_capable()
{
    return (boolean) (u.ulevel < MIN_QUEST_LEVEL);
}

STATIC_OVL int
is_pure(talk)
boolean talk;
{
    int purity;
    aligntyp original_alignment = u.ualignbase[A_ORIGINAL];

    if (wizard && talk) {
        if (u.ualign.type != original_alignment) {
            You("are currently %s instead of %s.", align_str(u.ualign.type),
                align_str(original_alignment));
        } else if (u.ualignbase[A_CURRENT] != original_alignment) {
            You("have converted.");
        } else if (u.ualign.record < MIN_QUEST_ALIGN) {
            You("are currently %d and require %d.", u.ualign.record,
                MIN_QUEST_ALIGN);
            if (yn_function("Adjust?", (char *) 0, 'y') == 'y')
                u.ualign.record = MIN_QUEST_ALIGN;
        }
    }
    purity = (u.ualign.record >= MIN_QUEST_ALIGN
              && u.ualign.type == original_alignment
              && u.ualignbase[A_CURRENT] == original_alignment)
                 ? 1
                 : (u.ualignbase[A_CURRENT] != original_alignment) ? -1 : 0;
    return purity;
}

/*
 * Expel the player to the stairs on the parent of the quest dungeon.
 *
 * This assumes that the hero is currently _in_ the quest dungeon and that
 * there is a single branch to and from it.
 */
STATIC_OVL void
expulsion(seal)
boolean seal;
{
    branch *br;
    d_level *dest;
    struct trap *t;
    int portal_flag;

    br = dungeon_branch("The Quest");
    dest = (br->end1.dnum == u.uz.dnum) ? &br->end2 : &br->end1;
    portal_flag = u.uevent.qexpelled ? 0 /* returned via artifact? */
                                     : !seal ? 1 : -1;
    schedule_goto(dest, FALSE, FALSE, portal_flag, (char *) 0, (char *) 0);

    if (seal) { /* remove the portal to the quest - sealing it off */
        int reexpelled = u.uevent.qexpelled;

        u.uevent.qexpelled = 1;
        remdun_mapseen(quest_dnum);
        /* Delete the near portal now; the far (main dungeon side)
           portal will be deleted as part of arrival on that level.
           If monster movement is in progress, any who haven't moved
           yet will now miss out on a chance to wander through it... */
        for (t = ftrap; t; t = t->ntrap)
            if (t->ttyp == MAGIC_PORTAL)
                break;
        if (t)
            deltrap(t); /* (display might be briefly out of sync) */
        else if (!reexpelled)
            impossible("quest portal already gone?");
    }
}

/* Either you've returned to quest leader while carrying the quest
   artifact or you've just thrown it to/at him or her.  If quest
   completion text hasn't been given yet, give it now.  Otherwise
   give another message about the character keeping the artifact
   and using the magic portal to return to the dungeon. */
void
finish_quest(obj)
struct obj *obj; /* quest artifact; possibly null if carrying Amulet */
{
    struct obj *otmp;
    struct monst *mtmp;
    struct permonst *q_guardian = &mons[quest_info(MS_GUARDIAN)];
    aligntyp saved_align;
    uchar saved_godgend;
    int i, alignabuse = 0;

    if (u.uachieve.amulet) { /* unlikely but not impossible */
        if (Role_if(PM_INFIDEL)) {
            saved_align = u.ualignbase[A_ORIGINAL];
            saved_godgend = quest_status.godgend;
            /* a hack for displaying a different god's name in the message */
            u.ualignbase[A_ORIGINAL] = inf_align(2);
            /* "god"[3] == 0; "goddess"[3] != 0 */
            quest_status.godgend = !!align_gtitle(inf_align(2))[3];
            qt_pager(QT_HASAMULET);
            u.ualignbase[A_ORIGINAL] = saved_align;
            quest_status.godgend = saved_godgend;
        } else
            qt_pager(QT_HASAMULET);
        /* leader IDs the real amulet but ignores any fakes */
        if ((otmp = carrying(AMULET_OF_YENDOR)) != 0)
            fully_identify_obj(otmp);
    } else if (u.ualign.abuse != 0) { /* player has abused their alignment */
        /* the more often the player abuses their alignment,
           the greater the odds of their quest leader demanding
           that they forfeit the quest artifact */
        i = 51 + u.ualign.abuse; /* a single transgression will make i = 50 */
        if (i < 1)
            i = 1; /* clamp lower limit to avoid panic */
        alignabuse = !rn2(i);
        if (alignabuse) {
            const char *qa_name = artiname(urole.questarti),
                       *ldr_name = ldrname();
            char qbuf[BUFSZ];
            Sprintf(qbuf, "Forfeit %s to %s?", qa_name, ldr_name);
            /* quest leader decides they want the quest artifact */
            qt_pager(QT_WANTSIT);
            if (yn(qbuf) == 'y') {
                qt_pager(QT_GAVEITUP);
                if (obj) {
                    u.uevent.qcompleted = 1; /* you did it! */
                    /* completing the quest frees the bell of opening
                       from its 'curse' */
                    if ((otmp = carrying(BELL_OF_OPENING)) != 0)
                        otmp->blessed = 1;
                    adjalign(5); /* god happy, much yay */
                    u.uluck += 3; /* increase luck */
                    /* Leader still gives the artifact their special treatment */
                    fully_identify_obj(obj);
                    obj->oeroded = obj->oeroded2 = 0; /* undo any damage */
                    maybe_erodeproof(obj, 1);
                    remove_worn_item(obj, TRUE);
                    obj_extract_self(obj);
                    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
                        if (DEADMONSTER(mtmp))
                            continue;
                        if (mtmp->isqldr)
                            (void) mpickobj(mtmp, obj);
                    }
                    update_inventory();
                    livelog_printf(LL_ACHIEVE,
                                   "returned %s to %s, completing %s quest",
                                   qa_name, ldr_name, uhis());
                }
                if (u.ualign.type == A_NONE) /* Infidel quest leader is an asshole */
                    Qstat(pissed_off) = 1;
                /* should have obtained bell during quest;
                   if not, suggest returning for it now */
                if ((otmp = carrying(BELL_OF_OPENING)) == 0) {
                    /* Unless it's the infidel quest leader */
                    if (!(u.ualign.type == A_NONE && Qstat(pissed_off)))
                        com_pager(5);
                }
                Qstat(got_thanks) = TRUE;
            } else {
                /* You have made the quest leader and his guardians grumpy.
                   quest won't be complete until the quest leader is defeated */
                Qstat(pissed_off) = 1;
                adjalign(-10); /* god less happy, much sad */
                if (u.ualign.type == A_NONE)
                    verbalize(
                      "I wouldn't expect anything less from a servant of Moloch.  However, I will not tolerate your insolence!");
                else
                    verbalize(
                      "You deny my request to safeguard our sacred artifact?  Your bones shall serve to warn others.");
                for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
                    if (DEADMONSTER(mtmp))
                        continue;
                    /* quest guardians become angry */
                    if (mtmp->data == q_guardian)
                        setmangry(mtmp, FALSE);
                }
                /* Convict quest leader is extra harsh */
                if (Role_if(PM_CONVICT) && !Punished)
                    punish((struct obj *) 0);
                livelog_printf(LL_ACHIEVE, "refused to give up %s to %s",
                               qa_name, ldr_name);
            }
        }
    }
    if (!alignabuse) { /* player gets to keep quest artifact */
        qt_pager(!Qstat(got_thanks) ? QT_OFFEREDIT : QT_OFFEREDIT2);
        /* should have obtained bell during quest;
           if not, suggest returning for it now */
        if ((otmp = carrying(BELL_OF_OPENING)) == 0)
            com_pager(5);

        Qstat(got_thanks) = TRUE;

        if (obj) {
            u.uevent.qcompleted = 1; /* you did it! */
            /* completing the quest frees the bell of opening
               from its 'curse' */
            if ((otmp = carrying(BELL_OF_OPENING)) != 0)
                otmp->blessed = 1;
            /* behave as if leader imparts sufficient info about the
               quest artifact */
            if (!Qstat(pissed_off)) {
                fully_identify_obj(obj);
                obj->oeroded = obj->oeroded2 = 0; /* undo any damage */
                maybe_erodeproof(obj, 1); /* Leader 'fixes' it for you */
            }
            update_inventory();
            livelog_printf(LL_ACHIEVE, "completed %s quest without incident",
                           uhis());
        }
    }
}

boolean
q_leader_angered()
{
    return Qstat(pissed_off);
}

void
chat_with_leader()
{
    /*  Rule 0: Cheater checks. */
    if (u.uhave.questart && !Qstat(met_nemesis))
        Qstat(cheater) = TRUE;

    /*  It is possible for you to get the amulet without completing
     *  the quest.  If so, try to induce the player to quest.
     */
    if (Qstat(got_thanks)) {
        /* Rule 1: You've gone back with/without the amulet. */
        if (u.uachieve.amulet)
            finish_quest((struct obj *) 0);

        /* Rule 2: You've gone back before going for the amulet. */
        else
            qt_pager(QT_POSTHANKS);

    /* Rule 3: You've got the artifact and are back to return it. */
    } else if (u.uhave.questart) {
        struct obj *otmp;

        for (otmp = invent; otmp; otmp = otmp->nobj)
            if (is_quest_artifact(otmp))
                break;

        finish_quest(otmp);

    /* Rule 4: You haven't got the artifact yet. */
    } else if (Qstat(got_quest)) {
        qt_pager(rn1(10, QT_ENCOURAGE));

    /* Rule 5: You aren't yet acceptable - or are you? */
    } else {
        if (!Qstat(met_leader)) {
            qt_pager(QT_FIRSTLEADER);
            Qstat(met_leader) = TRUE;
            Qstat(not_ready) = 0;
	} else if (!Qstat(pissed_off)) {
	    qt_pager(QT_NEXTLEADER);
	} else {
	    verbalize("Your bones shall serve to warn others.");
	}
        /* the quest leader might have passed through the portal into
           the regular dungeon; none of the remaining make sense there */
        if (!on_level(&u.uz, &qstart_level))
            return;

        if (not_capable()) {
            qt_pager(QT_BADLEVEL);
            exercise(A_WIS, TRUE);
            expulsion(FALSE);
        } else if (is_pure(TRUE) < 0) {
	    if (!Qstat(pissed_off)) {
		com_pager(QT_BANISHED);
		Qstat(pissed_off) = 1;
		expulsion(FALSE);
	    }
        } else if (is_pure(TRUE) == 0) {
            qt_pager(QT_BADALIGN);
	    Qstat(not_ready) = 1;
	    exercise(A_WIS, TRUE);
	    expulsion(FALSE);
        } else { /* You are worthy! */
            qt_pager(QT_ASSIGNQUEST);
            exercise(A_WIS, TRUE);
            Qstat(got_quest) = TRUE;
        }
    }
}

void
leader_speaks(mtmp)
struct monst *mtmp;
{
    /* maybe you attacked leader? */
    if (!mtmp->mpeaceful) {
	if (!Qstat(pissed_off)) {
	/* again, don't end it permanently if the leader gets angry
	 * since you're going to have to kill him to go questing... :)
	 * ...but do only show this crap once. */
	    qt_pager(QT_LASTLEADER);
	}
        Qstat(pissed_off) = 1;
        mtmp->mstrategy &= ~STRAT_WAITMASK; /* end the inaction */
    }
    /* the quest leader might have passed through the portal into the
       regular dungeon; if so, mustn't perform "backwards expulsion" */
    if (!on_level(&u.uz, &qstart_level))
        return;

    if (!Qstat(pissed_off))
        chat_with_leader();

    /* leader might have become pissed during the chat */
    if (Qstat(pissed_off)) {
        mtmp->mstrategy &= ~STRAT_WAITMASK;
	mtmp->mpeaceful = 0;
    }
}

STATIC_OVL void
chat_with_nemesis()
{
    /*  The nemesis will do most of the talking, but... */
    qt_pager(rn1(10, QT_DISCOURAGE));
    if (!Qstat(met_nemesis))
        Qstat(met_nemesis++);
}

void
nemesis_speaks()
{
    if (!Qstat(in_battle)) {
        if (u.uhave.questart)
            qt_pager(QT_NEMWANTSIT);
        else if (Qstat(made_goal) == 1 || !Qstat(met_nemesis))
            qt_pager(QT_FIRSTNEMESIS);
        else if (Qstat(made_goal) < 4)
            qt_pager(QT_NEXTNEMESIS);
        else if (Qstat(made_goal) < 7)
            qt_pager(QT_OTHERNEMESIS);
        else if (!rn2(5))
            qt_pager(rn1(10, QT_DISCOURAGE));
        if (Qstat(made_goal) < 7)
            Qstat(made_goal)++;
        Qstat(met_nemesis) = TRUE;
    } else /* he will spit out random maledictions */
        if (!rn2(5))
        qt_pager(rn1(10, QT_DISCOURAGE));
}

STATIC_OVL void
chat_with_guardian()
{
    /*  These guys/gals really don't have much to say... */
    if (u.uhave.questart && Qstat(killed_nemesis))
        qt_pager(rn1(5, QT_GUARDTALK2));
    else
        qt_pager(rn1(5, QT_GUARDTALK));
}

STATIC_OVL void
prisoner_speaks(mtmp)
struct monst *mtmp;
{
    if (mtmp->data == &mons[PM_PRISONER]
        && (mtmp->mstrategy & STRAT_WAITMASK)) {
        /* Awaken the prisoner */
        if (canseemon(mtmp))
            pline("%s speaks:", Monnam(mtmp));
        verbalize("I'm finally free!");
        mtmp->mstrategy &= ~STRAT_WAITMASK;
        mtmp->mpeaceful = 1;

        /* Your god is happy... */
        adjalign(3);

        /* ...But the guards are not */
        (void) angry_guards(FALSE);
    }
    return;
}

void
quest_chat(mtmp)
register struct monst *mtmp;
{
    if (mtmp->m_id == Qstat(leader_m_id)) {
        chat_with_leader();
	/* leader might have become pissed during the chat */
	if (Qstat(pissed_off)) {
	    mtmp->mstrategy &= ~STRAT_WAITMASK;
	    mtmp->mpeaceful = 0;
	}
        return;
    }
    switch (mtmp->data->msound) {
    case MS_NEMESIS:
        if (mtmp->data == &mons[urole.neminum])
            chat_with_nemesis();
        break;
    case MS_GUARDIAN:
        chat_with_guardian();
        break;
    default:
        impossible("quest_chat: Unknown quest character %s.", mon_nam(mtmp));
    }
}

void
quest_talk(mtmp)
struct monst *mtmp;
{
    if (mtmp->m_id == Qstat(leader_m_id)) {
        leader_speaks(mtmp);
        return;
    }
    switch (mtmp->data->msound) {
    case MS_NEMESIS:
        if (mtmp->data == &mons[urole.neminum])
            nemesis_speaks();
        break;
    case MS_DJINNI:
        prisoner_speaks(mtmp);
        break;
    default:
        break;
    }
}

void
quest_stat_check(mtmp)
struct monst *mtmp;
{
    if (mtmp->data->msound == MS_NEMESIS)
        Qstat(in_battle) = (mtmp->mcanmove && !mtmp->msleeping
                            && monnear(mtmp, u.ux, u.uy));
}

/*quest.c*/
