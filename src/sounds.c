/* NetHack 3.6	sounds.c	$NHDT-Date: 1570844005 2019/10/12 01:33:25 $  $NHDT-Branch: NetHack-3.6 $:$NHDT-Revision: 1.83 $ */
/*      Copyright (c) 1989 Janet Walz, Mike Threepoint */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "qtext.h" /* for Charon */

STATIC_DCL boolean FDECL(mon_is_gecko, (struct monst *));
STATIC_DCL int FDECL(domonnoise, (struct monst *));
STATIC_DCL int NDECL(dochat);
STATIC_DCL int FDECL(mon_in_room, (struct monst *, int));

/* this easily could be a macro, but it might overtax dumb compilers */
STATIC_OVL int
mon_in_room(mon, rmtyp)
struct monst *mon;
int rmtyp;
{
    int rno = levl[mon->mx][mon->my].roomno;
    if (rno >= ROOMOFFSET)
        return rooms[rno - ROOMOFFSET].rtype == rmtyp;
    return FALSE;
}

void
dosounds()
{
    struct mkroom *sroom;
    register int hallu, vx, vy;
#if defined(AMIGA) && defined(AZTEC_C_WORKAROUND)
    int xx;
#endif
    struct monst *mtmp;

    if (Deaf || !flags.acoustics || u.uswallow || Underwater)
        return;

    hallu = Hallucination ? 1 : 0;

    if (level.flags.nfountains && !rn2(400)) {
        static const char *const fountain_msg[4] = {
            "bubbling water.", "water falling on coins.",
            "the splashing of a naiad.", "a soda fountain!",
        };
        You_hear1(fountain_msg[rn2(3) + hallu]);
    }
    if (level.flags.nsinks && !rn2(300)) {
        static const char *const sink_msg[3] = {
            "a slow drip.", "a gurgling noise.", "dishes being washed!",
        };
        You_hear1(sink_msg[rn2(2) + hallu]);
    }
    if (level.flags.nforges && !rn2(300)) {
        static const char *const forge_msg[3] = {
            "a slow bubbling.", "crackling flames.", "chestnuts roasting on an open fire.",
        };
        You_hear1(forge_msg[rn2(2) + hallu]);
    }
    if (level.flags.has_court && !rn2(200)) {
        static const char *const throne_msg[4] = {
            "the tones of courtly conversation.",
            "a sceptre pounded in judgment.",
            "Someone shouts \"Off with %s head!\"", "Queen Beruthiel's cats!",
        };
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
            if (DEADMONSTER(mtmp))
                continue;
            if ((mtmp->msleeping || is_lord(mtmp->data)
                 || is_prince(mtmp->data)) && !is_animal(mtmp->data)
                && mon_in_room(mtmp, COURT)) {
                /* finding one is enough, at least for now */
                int which = rn2(3) + hallu;

                if (which != 2)
                    You_hear1(throne_msg[which]);
                else
                    pline(throne_msg[2], uhis());
                return;
            }
        }
    }
    if (level.flags.has_garden && !rn2(200)) {
        static const char *garden_msg[4] = {
            "hear crickets chirping.",
            "hear birds singing.",
            "hear the grass growing!",
            "hear the wind in the willows!",
        };
        You1(garden_msg[rn2(2) + 2 * hallu]);
        return;
    }
    if (level.flags.has_forest && !rn2(200)) {
        static const char *forest_msg[4] = {
            "hear a distant, mournful howl.",
            "hear rustling leaves.",
            "hear the color purple!",
            "hear a mattress flollop!",
        };
        You1(forest_msg[rn2(2) + 2 * hallu]);
        return;
    }
    if (level.flags.has_swamp && !rn2(200)) {
        static const char *const swamp_msg[3] = {
            "hear mosquitoes!", "smell marsh gas!", /* so it's a smell...*/
            "hear Donald Duck!",
        };
        You1(swamp_msg[rn2(2) + hallu]);
        return;
    }
    if (level.flags.has_vault && !rn2(200)) {
        if (!(sroom = search_special(VAULT))) {
            /* strange ... */
            level.flags.has_vault = 0;
            return;
        }
        if (gd_sound())
            switch (rn2(2) + hallu) {
            case 1: {
                boolean gold_in_vault = FALSE;

                for (vx = sroom->lx; vx <= sroom->hx; vx++)
                    for (vy = sroom->ly; vy <= sroom->hy; vy++)
                        if (g_at(vx, vy))
                            gold_in_vault = TRUE;
#if defined(AMIGA) && defined(AZTEC_C_WORKAROUND)
                /* Bug in aztec assembler here. Workaround below */
                xx = ROOM_INDEX(sroom) + ROOMOFFSET;
                xx = (xx != vault_occupied(u.urooms));
                if (xx)
#else
                if (vault_occupied(u.urooms)
                    != (ROOM_INDEX(sroom) + ROOMOFFSET))
#endif /* AZTEC_C_WORKAROUND */
                {
                    if (gold_in_vault)
                        You_hear(!hallu
                                     ? "someone counting money."
                                     : "the quarterback calling the play.");
                    else
                        You_hear("someone searching.");
                    break;
                }
            }
                /*FALLTHRU*/
            case 0:
                You_hear("the footsteps of a guard on patrol.");
                break;
            case 2:
                You_hear("Ebenezer Scrooge!");
                break;
            }
        return;
    }
    if (level.flags.has_beehive && !rn2(200)) {
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
            if (DEADMONSTER(mtmp))
                continue;
            if ((mtmp->data->mlet == S_ANT && is_flyer(mtmp->data))
                && mon_in_room(mtmp, BEEHIVE)) {
                switch (rn2(2) + hallu) {
                case 0:
                    You_hear("a low buzzing.");
                    break;
                case 1:
                    You_hear("an angry drone.");
                    break;
                case 2:
                    You_hear("bees in your %sbonnet!",
                             uarmh ? "" : "(nonexistent) ");
                    break;
                }
                return;
            }
        }
    }
    if (level.flags.has_lemurepit && !rn2(20)) {
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
            if (DEADMONSTER(mtmp))
                continue;
            if (mtmp->data == &mons[PM_LEMURE]
                && mon_in_room(mtmp, LEMUREPIT)) {
                switch (rn2(6) + hallu) {
                case 0:
                    You_hear("the crack of a barbed whip!");
                    break;
                case 1:
                    You_hear("the screams of tortured souls!");
                    break;
                case 2:
                    You_hear("a wail of eternal anguish!");
                    break;
                case 3:
                    You_hear("diabolical laughter...");
                    break;
                case 4:
                    You_hear("cries of repentance!");
                    break;
                case 5:
                    You_hear("futile pleas for mercy!");
                    break;
                case 6:
                    You_hear("screams of lust!");
                    break;
                case 7:
                    You_hear("a weeping willow!");
                    break;
                }
                return;
            }
        }
    }
    if (level.flags.has_morgue && !rn2(200)) {
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
            if (DEADMONSTER(mtmp))
                continue;
            if ((is_undead(mtmp->data) || is_vampshifter(mtmp))
                && mon_in_room(mtmp, MORGUE)) {
                const char *hair = body_part(HAIR); /* hair/fur/scales */

                switch (rn2(2) + hallu) {
                case 0:
                    You("suddenly realize it is unnaturally quiet.");
                    break;
                case 1:
                    pline_The("%s on the back of your %s %s up.", hair,
                              body_part(NECK), vtense(hair, "stand"));
                    break;
                case 2:
                    pline_The("%s on your %s %s to stand up.", hair,
                              body_part(HEAD), vtense(hair, "seem"));
                    break;
                }
                return;
            }
        }
    }
    if (level.flags.has_barracks && !rn2(200)) {
        static const char *const barracks_msg[4] = {
            "blades being honed.", "loud snoring.", "dice being thrown.",
            "General MacArthur!",
        };
        int count = 0;

        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
            if (DEADMONSTER(mtmp))
                continue;
            if (is_mercenary(mtmp->data)
#if 0 /* don't bother excluding these */
                && !strstri(mtmp->data->mname, "watch")
                && !strstri(mtmp->data->mname, "guard")
#endif
                && mon_in_room(mtmp, BARRACKS)
                /* sleeping implies not-yet-disturbed (usually) */
                && (mtmp->msleeping || ++count > 5)) {
                You_hear1(barracks_msg[rn2(3) + hallu]);
                return;
            }
        }
    }
    if (level.flags.has_zoo && !rn2(200)) {
        static const char *const zoo_msg[3] = {
            "a sound reminiscent of an elephant stepping on a peanut.",
            "a sound reminiscent of a seal barking.", "Doctor Dolittle!",
        };
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
            if (DEADMONSTER(mtmp))
                continue;
            if ((mtmp->msleeping || is_animal(mtmp->data))
                && mon_in_room(mtmp, ZOO)) {
                You_hear1(zoo_msg[rn2(2) + hallu]);
                return;
            }
        }
    }
    if (level.flags.has_shop && !rn2(200)) {
        if (!(sroom = search_special(ANY_SHOP))) {
            /* strange... */
            level.flags.has_shop = 0;
            return;
        }
        if (tended_shop(sroom)
            && !index(u.ushops, (int) (ROOM_INDEX(sroom) + ROOMOFFSET))) {
            static const char *const shop_msg[3] = {
                "someone cursing shoplifters.",
                "the chime of a cash register.", "Neiman and Marcus arguing!",
            };
            You_hear1(shop_msg[rn2(2) + hallu]);
        }
        return;
    }
    if (level.flags.has_temple && !rn2(200)
        && !(Is_astralevel(&u.uz) || Is_sanctum(&u.uz))) {
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
            if (DEADMONSTER(mtmp))
                continue;
            if (mtmp->ispriest && inhistemple(mtmp)
                /* priest must be active */
                && mtmp->mcanmove && !mtmp->msleeping
                /* hero must be outside this temple */
                && temple_occupied(u.urooms) != EPRI(mtmp)->shroom)
                break;
        }
        if (mtmp) {
            /* Generic temple messages; no attempt to match topic or tone
               to the pantheon involved, let alone to the specific deity.
               These are assumed to be coming from the attending priest;
               asterisk means that the priest must be capable of speech;
               pound sign (octathorpe,&c--don't go there) means that the
               priest and the altar must not be directly visible (we don't
               care if telepathy or extended detection reveals that the
               priest is not currently standing on the altar; he's mobile). */
            static const char *const temple_msg[] = {
                "*someone praising %s.", "*someone beseeching %s.",
                "#an animal carcass being offered in sacrifice.",
                "*a strident plea for donations.",
            };
            const char *msg;
            int trycount = 0, ax = EPRI(mtmp)->shrpos.x,
                ay = EPRI(mtmp)->shrpos.y;
            boolean speechless = (mtmp->data->msound <= MS_ANIMAL),
                    in_sight = canseemon(mtmp) || cansee(ax, ay);

            do {
                msg = temple_msg[rn2(SIZE(temple_msg) - 1 + hallu)];
                if (index(msg, '*') && speechless)
                    continue;
                if (index(msg, '#') && in_sight)
                    continue;
                break; /* msg is acceptable */
            } while (++trycount < 50);
            while (!letter(*msg))
                ++msg; /* skip control flags */
            if (index(msg, '%'))
                You_hear(msg, halu_gname(EPRI(mtmp)->shralign));
            else
                You_hear1(msg);
            return;
        }
    }
    if (Is_oracle_level(&u.uz) && !rn2(400)) {
        /* make sure the Oracle is still here */
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
            if (DEADMONSTER(mtmp))
                continue;
            if (mtmp->data == &mons[PM_ORACLE])
                break;
        }
        /* and don't produce silly effects when she's clearly visible */
        if (mtmp && (hallu || !canseemon(mtmp))) {
            static const char *const ora_msg[5] = {
                "a strange wind.",     /* Jupiter at Dodona */
                "convulsive ravings.", /* Apollo at Delphi */
                "snoring snakes.",     /* AEsculapius at Epidaurus */
                "someone say \"No more woodchucks!\"",
                "a loud ZOT!" /* both rec.humor.oracle */
            };
            You_hear1(ora_msg[rn2(3) + hallu * 2]);
        }
        return;
    }
    if (ledger_no(&u.uz) == ledger_no(&medusa_level) - 1
        && !rn2(200)) {
        static const char* const icequeenbranch_msg[] = {
            "an eerie, ominous wail.",
            "a howling wind.",
            "someone singing \"Do You Want to Build a Snowman?\""
        };
        You_hear1(icequeenbranch_msg[rn2(2 + hallu)]);
        return;
    }
    if (ledger_no(&u.uz) == ledger_no(&valley_level) + 1
        && !rn2(200)) {
        static const char* const vecnabranch_msg[] = {
            "a mysterious chanting.",
            "the tortured cries of the damned.",
            "\"Dead man walking\"..."
        };
        You_hear1(vecnabranch_msg[rn2(2 + hallu)]);
        return;
    }
    if (!In_goblintown(&u.uz0) && at_dgn_entrance("Goblin Town")
        && !rn2(200)) {
        static const char* const gtown_msg[] = {
            "the sounds of a bustling town nearby.",
            "what sounds like a goblin war party off in the distance.",
            "a chorus singing \"We are the Lollipop Guild\"..."
        };
        You_hear1(gtown_msg[rn2(2 + hallu)]);
        return;
    }
}

static const char *const h_sounds[] = {
    "beep",   "boing",   "sing",   "belche", "creak",   "cough",
    "rattle", "ululate", "pop",    "jingle", "sniffle", "tinkle",
    "eep",    "clatter", "hum",    "sizzle", "twitter", "wheeze",
    "rustle", "honk",    "lisp",   "yodel",  "coo",     "burp",
    "moo",    "boom",    "murmur", "oink",   "quack",   "rumble",
    "twang",  "bellow",  "toot",   "gargle", "hoot",    "warble"
};

const char *
growl_sound(mtmp)
struct monst *mtmp;
{
    const char *ret;

    switch (mtmp->data->msound) {
    case MS_MEW:
    case MS_HISS:
    case MS_LIZARD:
    case MS_PSEUDO:
        ret = "hiss";
        break;
    case MS_BARK:
    case MS_GROWL:
    case MS_WCHUCK:
        ret = "growl";
        break;
    case MS_GRUNT:
        ret = "grunt";
        break;
    case MS_ROAR:
    case MS_TRUMPET:
        ret = "roar";
        break;
    case MS_BUZZ:
        ret = "buzz";
        break;
    case MS_SQEEK:
    case MS_BAT:
        ret = "squeal";
        break;
    case MS_SQAWK:
    case MS_RAPTOR:
        ret = "screech";
        break;
    case MS_NEIGH:
        ret = "neigh";
        break;
    case MS_WAIL:
        ret = "wail";
        break;
    case MS_BURBLE:
        ret = "burble";
        break;
    case MS_SILENT:
        ret = "quiver";
        break;
    default:
        ret = "scream";
    }
    return ret;
}

/* the sounds of a seriously abused pet, including player attacking it */
void
growl(mtmp)
struct monst *mtmp;
{
    register const char *growl_verb = 0;

    if (mtmp->msleeping || !mtmp->mcanmove || !mtmp->data->msound)
        return;

    /* presumably nearness and soundok checks have already been made */
    if (Hallucination)
        growl_verb = h_sounds[rn2(SIZE(h_sounds))];
    else
        growl_verb = growl_sound(mtmp);
    if (growl_verb) {
        pline("%s %s!", Monnam(mtmp), vtense((char *) 0, growl_verb));
        if (context.run)
            nomul(0);
        wake_nearto(mtmp->mx, mtmp->my, mtmp->data->mlevel * 18);
    }
}

/* the sounds of mistreated pets */
void
yelp(mtmp)
struct monst *mtmp;
{
    register const char *yelp_verb = 0;

    if (mtmp->msleeping || !mtmp->mcanmove || !mtmp->data->msound)
        return;

    /* presumably nearness and soundok checks have already been made */
    if (Hallucination)
        yelp_verb = h_sounds[rn2(SIZE(h_sounds))];
    else
        switch (mtmp->data->msound) {
        case MS_MEW:
            yelp_verb = (!Deaf) ? "yowl" : "arch";
            break;
        case MS_BARK:
        case MS_GROWL:
            yelp_verb = (!Deaf) ? "yelp" : "recoil";
            break;
        case MS_GRUNT:
            yelp_verb = (!Deaf) ? "grunt" : "thrash";
            break;
        case MS_ROAR:
        case MS_TRUMPET:
            yelp_verb = (!Deaf) ? "snarl" : "bluff";
            break;
        case MS_SQEEK:
        case MS_BAT:
        case MS_WCHUCK:
            yelp_verb = (!Deaf) ? "squeal" : "quiver";
            break;
        case MS_SQAWK:
        case MS_RAPTOR:
            yelp_verb = (!Deaf) ? "screak" : "thrash";
            break;
        case MS_HISS:
        case MS_LIZARD:
        case MS_PSEUDO:
            yelp_verb = (!Deaf) ? "hiss" : "recoil";
            break;
        case MS_WAIL:
            yelp_verb = (!Deaf) ? "wail" : "cringe";
            break;
        case MS_BURBLE:
            yelp_verb = (!Deaf) ? "whiffle" : "gyre";
            break;
        }
    if (yelp_verb) {
        pline("%s %s!", Monnam(mtmp), vtense((char *) 0, yelp_verb));
        if (context.run)
            nomul(0);
        wake_nearto(mtmp->mx, mtmp->my, mtmp->data->mlevel * 12);
    }
}

/* the sounds of distressed pets */
void
whimper(mtmp)
struct monst *mtmp;
{
    register const char *whimper_verb = 0;

    if (mtmp->msleeping || !mtmp->mcanmove || !mtmp->data->msound)
        return;

    /* presumably nearness and soundok checks have already been made */
    if (Hallucination)
        whimper_verb = h_sounds[rn2(SIZE(h_sounds))];
    else
        switch (mtmp->data->msound) {
        case MS_MEW:
        case MS_GROWL:
            whimper_verb = "whimper";
            break;
        case MS_BARK:
            whimper_verb = "whine";
            break;
        case MS_NEIGH:
            whimper_verb = "whinny";
            break;
        case MS_TRUMPET:
            whimper_verb = "trumpet";
            break;
        case MS_HISS:
        case MS_LIZARD:
        case MS_PSEUDO:
            whimper_verb = "hiss";
            break;
        case MS_SQEEK:
        case MS_BAT:
            whimper_verb = "squeal";
            break;
        case MS_BURBLE:
            whimper_verb = "squeal";
            break;
        }
    if (whimper_verb) {
        pline("%s %s.", Monnam(mtmp), vtense((char *) 0, whimper_verb));
        if (context.run)
            nomul(0);
        wake_nearto(mtmp->mx, mtmp->my, mtmp->data->mlevel * 6);
    }
}

/* pet makes "I'm hungry" noises */
void
beg(mtmp)
struct monst *mtmp;
{
    if (mtmp->msleeping || !mtmp->mcanmove
        || !(carnivorous(mtmp->data) || herbivorous(mtmp->data)))
        return;

    /* presumably nearness and soundok checks have already been made */
    if (!is_silent(mtmp->data) && mtmp->data->msound <= MS_ANIMAL)
        (void) domonnoise(mtmp);
    else if (mtmp->data->msound >= MS_HUMANOID) {
        if (!canspotmon(mtmp))
            map_invisible(mtmp->mx, mtmp->my);
        verbalize("I'm hungry.");
    }
}

/* return True if mon is a gecko or seems to look like one (hallucination) */
STATIC_OVL boolean
mon_is_gecko(mon)
struct monst *mon;
{
    int glyph;

    /* return True if it is actually a gecko */
    if (mon->data == &mons[PM_GECKO])
        return TRUE;
    /* return False if it is a long worm; we might be chatting to its tail
       (not strictly needed; long worms are MS_SILENT so won't get here) */
    if (mon->data == &mons[PM_LONG_WORM])
        return FALSE;
    /* result depends upon whether map spot shows a gecko, which will
       be due to hallucination or to mimickery since mon isn't one */
    glyph = glyph_at(mon->mx, mon->my);
    return (boolean) (glyph_to_mon(glyph) == PM_GECKO);
}

STATIC_OVL int
domonnoise(mtmp)
struct monst *mtmp;
{
    char verbuf[BUFSZ];
    register const char *pline_msg = 0, /* Monnam(mtmp) will be prepended */
        *verbl_msg = 0,                 /* verbalize() */
        *verbl_msg_mcan = 0;            /* verbalize() if cancelled */
    struct permonst *ptr = mtmp->data;
    int msound = ptr->msound, gnomeplan = 0;

    /* presumably nearness and sleep checks have already been made */
    if (Deaf)
        return 0;
    if (is_silent(ptr))
        return 0;

    /* leader might be poly'd; if he can still speak, give leader speech */
    if (mtmp->m_id == quest_status.leader_m_id && msound > MS_ANIMAL)
        msound = MS_LEADER;
    /* make sure it's your role's quest guardian; adjust if not */
    else if (msound == MS_GUARDIAN && ptr != &mons[urole.guardnum])
        msound = mons[genus(monsndx(ptr), 1)].msound;
    /* some normally non-speaking types can/will speak if hero is similar */
    else if (msound == MS_ORC         /* note: MS_ORC is same as MS_GRUNT */
             && ((same_race(ptr, youmonst.data)          /* current form, */
                  || same_race(ptr, &mons[Race_switch])) /* unpoly'd form */
                 || Hallucination))
        msound = MS_HUMANOID;
    /* silliness, with slight chance to interfere with shopping */
    else if (Hallucination && mon_is_gecko(mtmp))
        msound = MS_SELL;

    /* be sure to do this before talking; the monster might teleport away, in
     * which case we want to check its pre-teleport position
     */
    if (!canspotmon(mtmp))
        map_invisible(mtmp->mx, mtmp->my);

    switch (msound) {
    case MS_ORACLE:
        return doconsult(mtmp);
    case MS_PRIEST:
        priest_talk(mtmp);
        break;
    case MS_LEADER:
    case MS_NEMESIS:
    case MS_GUARDIAN:
        quest_chat(mtmp);
        break;
    case MS_SELL: /* pitch, pay, total */
        if (!Hallucination || (mtmp->isshk && !rn2(2))) {
            shk_chat(mtmp);
        } else {
            /* approximation of GEICO's advertising slogan (it actually
               concludes with "save you 15% or more on car insurance.") */
            Sprintf(verbuf, "15 minutes could save you 15 %s.",
                    currency(15L)); /* "zorkmids" */
            verbl_msg = verbuf;
        }
        break;
    case MS_VAMPIRE: {
        /* vampire messages are varied by tameness, peacefulness, and time of
         * night */
        boolean isnight = night();
        boolean kindred = (Upolyd && (u.umonnum == PM_VAMPIRE_SOVEREIGN
                                      || u.umonnum == PM_VAMPIRE_NOBLE
                                      || u.umonnum == PM_VAMPIRE_ROYAL
                                      || u.umonnum == PM_VAMPIRE_MAGE));
        boolean nightchild =
            (Upolyd && (u.umonnum == PM_WOLF || u.umonnum == PM_WINTER_WOLF
                        || u.umonnum == PM_WARG || u.umonnum == PM_WINTER_WOLF_CUB));
        const char *racenoun =
            (flags.female && urace.individual.f)
                ? urace.individual.f
                : (urace.individual.m) ? urace.individual.m : urace.noun;

        if (mtmp->mtame) {
            if (kindred) {
                Sprintf(verbuf, "Good %s to you Master%s",
                        isnight ? "evening" : "day",
                        isnight ? "!" : ".  Why do we not rest?");
                verbl_msg = verbuf;
            } else {
                Sprintf(verbuf, "%s%s",
                        nightchild ? "Child of the night, " : "",
                        midnight()
                         ? "I can stand this craving no longer!"
                         : isnight
                          ? "I beg you, help me satisfy this growing craving!"
                          : "I find myself growing a little weary.");
                verbl_msg = verbuf;
            }
        } else if (mtmp->mpeaceful) {
            if (kindred && isnight) {
                Sprintf(verbuf, "Good feeding %s!",
                        flags.female ? "sister" : "brother");
                verbl_msg = verbuf;
            } else if (nightchild && isnight) {
                Sprintf(verbuf, "How nice to hear you, child of the night!");
                verbl_msg = verbuf;
            } else
                verbl_msg = "I only drink... potions.";
        } else {
            static const char *const vampmsg[] = {
                /* These first two (0 and 1) are specially handled below */
                "I vant to suck your %s!",
                "I vill come after %s without regret!",
                /* other famous vampire quotes can follow here if desired */
            };
            int vampindex;

            if (kindred)
                verbl_msg =
                    "This is my hunting ground that you dare to prowl!";
            else if (youmonst.data == &mons[PM_SILVER_DRAGON]
                     || youmonst.data == &mons[PM_BABY_SILVER_DRAGON]) {
                /* Silver dragons are silver in color, not made of silver */
                Sprintf(verbuf, "%s!  Your silver sheen does not frighten me!",
                        youmonst.data == &mons[PM_SILVER_DRAGON]
                            ? "Fool"
                            : "Young Fool");
                verbl_msg = verbuf;
            } else {
                vampindex = rn2(SIZE(vampmsg));
                if (vampindex == 0) {
                    Sprintf(verbuf, vampmsg[vampindex], body_part(BLOOD));
                    verbl_msg = verbuf;
                } else if (vampindex == 1) {
                    Sprintf(verbuf, vampmsg[vampindex],
                            Upolyd ? an(mons[u.umonnum].mname)
                                   : an(racenoun));
                    verbl_msg = verbuf;
                }
            }
        }
    } break;
    case MS_WERE:
        if (flags.moonphase == FULL_MOON && (night() ^ !rn2(13))) {
            pline("%s throws back %s head and lets out a blood curdling %s!",
                  Monnam(mtmp), mhis(mtmp),
                  ptr == &mons[PM_HUMAN_WERERAT] ? "shriek" : "howl");
            wake_nearto(mtmp->mx, mtmp->my, 11 * 11);
        } else
            pline_msg =
                "whispers inaudibly.  All you can make out is \"moon\".";
        break;
    case MS_BARK:
        if (flags.moonphase == FULL_MOON && night()) {
            pline_msg = "howls.";
        } else if (mtmp->mpeaceful) {
            if (mtmp->mtame
                && (mtmp->mconf || mtmp->mflee
                    || mtmp->mtrapped || mtmp->mentangled
                    || mtmp->mtame < 5))
                pline_msg = "whines.";
            else if (mtmp->mtame && moves > EDOG(mtmp)->hungrytime)
                pline_msg = "whines hungrily.";
            else if (mtmp->mtame && EDOG(mtmp)->hungrytime > moves + 1000)
                pline_msg = "yips contentedly.";
            else {
                if (mtmp->data
                    != &mons[PM_DINGO]) /* dingos do not actually bark */
                    pline_msg = "barks.";
            }
        } else {
            pline_msg = "growls.";
        }
        break;
    case MS_MEW:
        if (mtmp->mtame) {
            if (mtmp->mconf || mtmp->mflee || mtmp->mtrapped
                || mtmp->mentangled || mtmp->mtame < 5)
                pline_msg = "yowls.";
            else if (moves > EDOG(mtmp)->hungrytime)
                pline_msg = "meows.";
            else if (EDOG(mtmp)->hungrytime > moves + 1000)
                pline_msg = "purrs.";
            else
                pline_msg = "mews.";
            break;
        } else
            pline_msg = mtmp->mpeaceful ? "snarls." : "growls!";
        break;
    case MS_GROWL:
        if (mtmp->mtame) {
            if (mtmp->mconf || mtmp->mflee || mtmp->mtrapped
                || mtmp->mentangled || mtmp->mtame < 5)
                pline_msg = "snarls.";
            else if (moves > EDOG(mtmp)->hungrytime)
                pline_msg = "makes a low rumble.";
            else if (EDOG(mtmp)->hungrytime > moves + 1000)
                pline_msg = "rumbles contentedly.";
            else
                pline_msg = "rumbles.";
            break;
        } else
            pline_msg = mtmp->mpeaceful ? "snarls." : "growls!";
        break;
    case MS_ROAR:
        if (mtmp->mtame) {
            if (mtmp->mconf || mtmp->mflee || mtmp->mtrapped
                || mtmp->mentangled || mtmp->mtame < 5)
                pline_msg = "roars.";
            else if (moves > EDOG(mtmp)->hungrytime)
                pline_msg = "huffs.";
            else if (EDOG(mtmp)->hungrytime > moves + 1000)
                pline_msg = "exhales contentedly.";
            else
                pline_msg = "snorts.";
            break;
        } else
            pline_msg = mtmp->mpeaceful ? "snarls." : "growls!";
        break;
    case MS_TRUMPET:
        if (mtmp->mtame) {
            if (mtmp->mconf || mtmp->mflee || mtmp->mtrapped
                || mtmp->mentangled || mtmp->mtame < 5)
                pline_msg = "trumpets.";
            else if (moves > EDOG(mtmp)->hungrytime)
                pline_msg = "snorts hungrily.";
            else if (EDOG(mtmp)->hungrytime > moves + 1000)
                pline_msg = "rumbles contentedly.";
            else
                pline_msg = "snorts.";
            break;
        } else
            pline_msg = mtmp->mpeaceful ? "trumpets." : "trumpets!";
        break;
    case MS_SQEEK:
        if (mtmp->mtame) {
            if (mtmp->mconf || mtmp->mflee || mtmp->mtrapped
                || mtmp->mentangled || mtmp->mtame < 5)
                pline_msg = "hisses.";
            else if (moves > EDOG(mtmp)->hungrytime)
                pline_msg = "squeals hungrily.";
            else if (EDOG(mtmp)->hungrytime > moves + 1000)
                pline_msg = "bruxes contentedly."; /* chatters or grinds its teeth */
            else
                pline_msg = "chitters.";
            break;
        } else
            pline_msg = mtmp->mpeaceful ? "squeaks." : "loudly squeaks!";
        break;
    case MS_BAT:
        if (mtmp->mtame) {
            if (mtmp->mconf || mtmp->mflee || mtmp->mtrapped
                || mtmp->mentangled || mtmp->mtame < 5)
                pline_msg = "screeches.";
            else if (moves > EDOG(mtmp)->hungrytime)
                pline_msg = "clicks hungrily.";
            else if (EDOG(mtmp)->hungrytime > moves + 1000)
                pline_msg = "clicks contentedly.";
            else
                pline_msg = "chirps.";
            break;
        } else
            pline_msg = mtmp->mpeaceful ? "squeaks." : "screeches.";
        break;
    case MS_WCHUCK:
        if (mtmp->mtame) {
            if (mtmp->mconf || mtmp->mflee || mtmp->mtrapped
                || mtmp->mentangled || mtmp->mtame < 5)
                pline_msg = "squeals.";
            else if (moves > EDOG(mtmp)->hungrytime)
                pline_msg = "whistles hungrily.";
            else if (EDOG(mtmp)->hungrytime > moves + 1000)
                pline_msg = "chirps contentedly.";
            else
                pline_msg = "grunts.";
            break;
        } else
            pline_msg = mtmp->mpeaceful ? "barks." : "growls!";
        break;
    case MS_SQAWK:
        if (ptr == &mons[PM_RAVEN] && !mtmp->mpeaceful)
            verbl_msg = "Nevermore!";
        else
            pline_msg = "squawks.";
        break;
    case MS_RAPTOR:
        if (mtmp->mtame) {
            if (mtmp->mconf || mtmp->mflee || mtmp->mtrapped
                || mtmp->mentangled || mtmp->mtame < 5)
                pline_msg = "screams frantically.";
            else if (moves > EDOG(mtmp)->hungrytime)
                pline_msg = "whistles shrilly.";
            else if (EDOG(mtmp)->hungrytime > moves + 1000)
                pline_msg = "chirps contentedly.";
            else
                pline_msg = "whistles.";
            break;
        } else
            pline_msg = mtmp->mpeaceful ? "screams." : "screams!";
        break;
    case MS_HISS:
        if (mtmp->mtame) {
            if (mtmp->mconf || mtmp->mflee || mtmp->mtrapped
                || mtmp->mentangled || mtmp->mtame < 5)
                pline_msg = "hisses frantically.";
            else if (moves > EDOG(mtmp)->hungrytime)
                pline_msg = "hisses hungrily.";
            else if (EDOG(mtmp)->hungrytime > moves + 1000)
                pline_msg = "hisses contentedly.";
            else
                pline_msg = "softly hisses.";
            break;
        } else if (!mtmp->mpeaceful) {
            pline_msg = "hisses!";
        } else
            return 0; /* no sound */
        break;
    case MS_LIZARD:
        if (mtmp->mtame) {
            if (mtmp->mconf || mtmp->mflee || mtmp->mtrapped
                || mtmp->mentangled || mtmp->mtame < 5)
                pline_msg = "hisses frantically.";
            else if (moves > EDOG(mtmp)->hungrytime)
                pline_msg = "grunts hungrily.";
            else if (EDOG(mtmp)->hungrytime > moves + 1000)
                pline_msg = "grunts contentedly.";
            else
                pline_msg = "grunts.";
            break;
        } else if (!mtmp->mpeaceful) {
            pline_msg = "hisses!";
        } else
            return 0; /* no sound */
        break;
    case MS_PSEUDO:
        if (mtmp->mtame) {
            if (mtmp->mconf || mtmp->mflee || mtmp->mtrapped
                || mtmp->mentangled || mtmp->mtame < 5)
                pline_msg = "snarls.";
            else if (moves > EDOG(mtmp)->hungrytime)
                pline_msg = "rumbles.";
            else if (EDOG(mtmp)->hungrytime > moves + 1000)
                pline_msg = "purrs.";
            else
                pline_msg = "softly hisses.";
            break;
        } else if (!mtmp->mpeaceful) {
            pline_msg = "hisses!";
        } else
            return 0; /* no sound */
        break;
    case MS_BUZZ:
        pline_msg = mtmp->mpeaceful ? "drones." : "buzzes angrily.";
        break;
    case MS_GRUNT:
        if (mtmp->mtame) {
            if (mtmp->mconf || mtmp->mflee || mtmp->mtrapped
                || mtmp->mentangled || mtmp->mtame < 5)
                pline_msg = "grunts.";
            else if (moves > EDOG(mtmp)->hungrytime)
                pline_msg = "jabbers hungrily.";
            else if (EDOG(mtmp)->hungrytime > moves + 1000)
                pline_msg = "gabbles contentedly.";
            else
                pline_msg = "jabbers.";
            break;
        } else
            pline_msg = mtmp->mpeaceful ? "grunts." : "grunts!";
        break;
    case MS_NEIGH:
        if (mtmp->mtame) {
            if (mtmp->mconf || mtmp->mflee || mtmp->mtrapped
                || mtmp->mentangled || mtmp->mtame < 5)
                pline_msg = "neighs.";
            else if (moves > EDOG(mtmp)->hungrytime)
                pline_msg = "whinnies.";
            else if (EDOG(mtmp)->hungrytime > moves + 1000)
                pline_msg = "whickers contentedly.";
            else
                pline_msg = "whickers.";
            break;
        } else
            pline_msg = mtmp->mpeaceful ? "neighs." : "neighs!";
        break;
    case MS_WAIL:
        pline_msg = "wails mournfully.";
        break;
    case MS_GURGLE:
        pline_msg = "gurgles.";
        break;
    case MS_BURBLE:
        if (mtmp->mtame) {
            if (mtmp->mconf || mtmp->mflee || mtmp->mtrapped
                || mtmp->mentangled || mtmp->mtame < 5)
                pline_msg = "burbles frantically.";
            else if (moves > EDOG(mtmp)->hungrytime)
                pline_msg = "whiffles hungrily.";
            else if (EDOG(mtmp)->hungrytime > moves + 1000)
                pline_msg = "whiffles contentedly.";
            else
                pline_msg = "whiffles.";
            break;
        } else
            pline_msg = mtmp->mpeaceful ? "burbles." : "burbles!";
        break;
    case MS_SHRIEK:
        pline_msg = "shrieks.";
        aggravate();
        break;
    case MS_IMITATE:
        pline_msg = "imitates you.";
        break;
    case MS_BONES:
        pline("%s rattles noisily.", Monnam(mtmp));
        if (!Race_if(PM_DRAUGR)) {
            You("freeze for a moment.");
            nomul(-2);
            multi_reason = "scared by rattling";
            nomovemsg = 0;
        }
        break;
    case MS_LAUGH: {
        static const char *const laugh_msg[4] = {
            "giggles.", "chuckles.", "snickers.", "laughs.",
        };
        static const char *const gnoll_msg[5] = {
            "cackles.", "woops.", "yips.", "laughs.", "growls.",
        };
        if (is_gnoll(ptr))
            pline_msg = gnoll_msg[rn2(5)];
        else
            pline_msg = laugh_msg[rn2(4)];
    } break;
    case MS_MUMBLE:
        pline_msg = "mumbles incomprehensibly.";
        break;
    case MS_DJINNI:
        if (mtmp->mtame) {
            verbl_msg = "Sorry, I'm all out of wishes.";
        } else if (mtmp->mpeaceful) {
            if (ptr == &mons[PM_WATER_DEMON])
                pline_msg = "gurgles.";
            else
                verbl_msg = "I'm free!";
        } else {
            if (ptr != &mons[PM_PRISONER])
                verbl_msg = "This will teach you not to disturb me!";
#if 0
            else
                verbl_msg = "??????????";
#endif
        }
        break;
    case MS_BOAST: /* giants */
        if (!mtmp->mpeaceful) {
            switch (rn2(4)) {
            case 0:
                pline("%s boasts about %s gem collection.", Monnam(mtmp),
                      mhis(mtmp));
                break;
            case 1:
                pline_msg = "complains about a diet of mutton.";
                break;
            default:
                pline_msg = "shouts \"Fee Fie Foe Foo!\" and guffaws.";
                wake_nearto(mtmp->mx, mtmp->my, 7 * 7);
                break;
            }
            break;
        }
        /*FALLTHRU*/
    case MS_HUMANOID:
        if (!mtmp->mpeaceful) {
            if (In_endgame(&u.uz) && is_mplayer(ptr))
                mplayer_talk(mtmp);
            else
                pline_msg = "threatens you.";
            break;
        }
        /* Generic peaceful humanoid behaviour. */
        if (mtmp->mflee)
            pline_msg = "wants nothing to do with you.";
        else if (mtmp->mhp < mtmp->mhpmax / 4
                 || mtmp->msick)
            pline_msg = "moans.";
        else if (mtmp->mconf || mtmp->mstun)
            verbl_msg = !rn2(3) ? "Huh?" : rn2(2) ? "What?" : "Eh?";
        else if (!mtmp->mcansee)
            verbl_msg = "I can't see!";
        else if (mtmp->mentangled)
            verbl_msg = "I'm entangled!";
        else if (mtmp->mtrapped) {
            struct trap *t = t_at(mtmp->mx, mtmp->my);

            if (t)
                t->tseen = 1;
            verbl_msg = "I'm trapped!";
        } else if (mtmp->mhp < mtmp->mhpmax / 2)
            pline_msg = "asks for a potion of healing.";
        else if (mtmp->mtame && !mtmp->isminion
                 && moves > EDOG(mtmp)->hungrytime)
            verbl_msg = "I'm hungry.";
        /* Specific monsters' interests */
        else if (racial_elf(mtmp)) {
            if (mtmp->mpeaceful && Ingtown) {
                if (u.uevent.ugking) {
                    verbl_msg = "The Goblin King is dead!";
                } else {
                    verbl_msg = rn2(2) ? "Death to the Goblin King!"
                                       : "Curse this wretched town!";
                }
            } else {
                pline_msg = "curses orcs.";
            }
        } else if (racial_dwarf(mtmp)) {
            if (mtmp->mpeaceful && Ingtown) {
                if (u.uevent.ugking) {
                    verbl_msg = "The Goblin King has fallen!  Back to mining, lads!";
                } else {
                    verbl_msg = rn2(2) ? "Baruk Khazad!  Khazad ai-menu!"
                                       : "Bah!  Not a single pint of ale to be found!";
                }
            } else {
                pline_msg = "talks about mining.";
            }
        } else if (likes_magic(ptr)) {
            pline_msg = "talks about spellcraft.";
        } else if (is_true_ent(ptr)) {
            if (!rn2(100))
                pline_msg = "asks if you've seen any entwives recently.";
            else
                pline_msg = "discusses the state of the forest.";
        } else if (racial_centaur(mtmp))
            pline_msg = "discusses hunting.";
        else if (racial_gnome(mtmp)) {
            if (mtmp->mpeaceful && Ingtown) {
                if (u.uevent.ugking) {
                    verbl_msg = "The Goblin King is no more!  Mine Town is open for business!";
                } else {
                    verbl_msg = "We must free our brothers and sisters in the Mines!";
                }
            } else if (Hallucination && (gnomeplan = rn2(4)) % 2) {
                /* skipped for rn2(4) result of 0 or 2;
                   gag from an early episode of South Park called "Gnomes";
                   initially, Tweek (introduced in that episode) is the only
                   one aware of the tiny gnomes after spotting them sneaking
                   about; they are embarked upon a three-step business plan;
                   a diagram of the plan shows:
                             Phase 1         Phase 2      Phase 3
                       Collect underpants       ?          Profit
                   and they never verbalize step 2 so we don't either */
                verbl_msg = (gnomeplan == 1) ? "Phase one, collect underpants."
                                             : "Phase three, profit!";
            } else {
                pline_msg = "grunts.";
            }
        } else {
            switch (monsndx(ptr)) {
            case PM_HOBBIT:
            case PM_HOBBIT_PICKPOCKET:
                pline_msg =
                    (mtmp->mhpmax - mtmp->mhp >= 10)
                        ? "complains about unpleasant dungeon conditions."
                        : "asks you about the One Ring.";
                break;
            case PM_ARCHEOLOGIST:
                pline_msg =
                "describes a recent article in \"Spelunker Today\" magazine.";
                break;
            case PM_TOURIST:
                verbl_msg = "Aloha.";
                break;
            case PM_CHARON:
                if (mtmp->mpeaceful) {
                    com_pager(rn1(u.uevent.ucerberus ? QTN_CHRN_NOCERB
                                                     : QTN_CHARON,
                                  QT_CHARON));
                };
                break;
            default:
                pline_msg = "discusses dungeon exploration.";
                break;
            }
        }
        break;
    case MS_SEDUCE: {
        int swval;

        if (SYSOPT_SEDUCE) {
            if (ptr->mlet != S_NYMPH
                && could_seduce(mtmp, &youmonst, (struct attack *) 0) == 1) {
                (void) doseduce(mtmp);
                break;
            }
            swval = ((poly_gender() != (int) mtmp->female) ? rn2(3) : 0);
        } else
            swval = ((poly_gender() == 0) ? rn2(3) : 0);
        switch (swval) {
        case 2:
            verbl_msg = "Hello, sailor.";
            break;
        case 1:
            pline_msg = "comes on to you.";
            break;
        default:
            pline_msg = "cajoles you.";
        }
    } break;
    case MS_ARREST:
        if (mtmp->mpeaceful)
            verbalize("Just the facts, %s.", flags.female ? "Ma'am" : "Sir");
        else {
            static const char *const arrest_msg[3] = {
                "Anything you say can be used against you.",
                "You're under arrest!", "Stop in the name of the Law!",
            };
            verbl_msg = arrest_msg[rn2(3)];
        }
        break;
    case MS_BRIBE:
        if (monsndx(ptr) == PM_PRISON_GUARD) {
            long gdemand = 500 * u.ulevel;
            long goffer = 0;

    	    if (!mtmp->mpeaceful && !mtmp->mtame) {
                if (!money_cnt(invent)) { /* can't bribe with no money */
                    mtmp->mspec_used = 1000;
                    break;
                }
                pline("%s demands %ld %s to avoid re-arrest.",
                      Amonnam(mtmp), gdemand, currency(gdemand));
                if ((goffer = bribe(mtmp)) >= gdemand) {
                    verbl_msg = "Good.  Now beat it, scum!";
            	    mtmp->mpeaceful = 1;
            	    set_malign(mtmp);
                    break;
                } else {
                    pline("I said %ld!", gdemand);
                    mtmp->mspec_used = 1000;
                    break;
                }
            } else {
                verbl_msg = "Out of my way, scum!"; /* still a jerk */
            }
        } else if (mtmp->mpeaceful && !mtmp->mtame) {
            (void) demon_talk(mtmp);
            break;
        }
    /* fall through */
    case MS_CUSS:
        if (!mtmp->mpeaceful)
            cuss(mtmp);
        else if (is_lminion(mtmp))
            verbl_msg = "It's not too late.";
        else if (mtmp->data == &mons[PM_KATHRYN_THE_ENCHANTRESS])
            verbl_msg = "Ozzy!  I can't throw the stick if you won't drop it!";
        else if (mtmp->data == &mons[PM_GOLLUM])
            verbl_msg = "My preciousss...";
        else
            verbl_msg = "We're all doomed.";
        break;
    case MS_SPELL:
        /* deliberately vague, since it's not actually casting any spell */
        pline_msg = "seems to mutter a cantrip.";
        break;
    case MS_NURSE:
        verbl_msg_mcan = "I hate this job!";
        if (uwep && (uwep->oclass == WEAPON_CLASS || is_weptool(uwep)))
            verbl_msg = "Put that weapon away before you hurt someone!";
        else if (uarmc || uarm || uarmh || uarms || uarmg || uarmf)
            verbl_msg = Role_if(PM_HEALER)
                            ? "Doc, I can't help you unless you cooperate."
                            : "Please undress so I can examine you.";
        else if (uarmu)
            verbl_msg = "Take off your shirt, please.";
        else
            verbl_msg = "Relax, this won't hurt a bit.";
        break;
    case MS_GUARD:
        if (money_cnt(invent))
            verbl_msg = "Please drop that gold and follow me.";
        else
            verbl_msg = "Please follow me.";
        break;
    case MS_SOLDIER: {
        static const char
            *const soldier_foe_msg[3] =
                {
                  "Resistance is useless!", "You're dog meat!", "Surrender!",
                },
                   *const soldier_pax_msg[3] = {
                       "What lousy pay we're getting here!",
                       "The food's not fit for Orcs!",
                       "My feet hurt, I've been on them all day!",
                   };
        verbl_msg = mtmp->mpeaceful ? soldier_pax_msg[rn2(3)]
                                    : soldier_foe_msg[rn2(3)];
        break;
    }
    case MS_RIDER: {
        const char *tribtitle;
        struct obj *book = 0;
        boolean ms_Death = (ptr == &mons[PM_DEATH]);

        /* 3.6 tribute */
        if (ms_Death && !context.tribute.Deathnotice
            && (book = u_have_novel()) != 0) {
            if ((tribtitle = noveltitle(&book->novelidx)) != 0) {
                Sprintf(verbuf, "Ah, so you have a copy of /%s/.", tribtitle);
                /* no Death featured in these two, so exclude them */
                if (strcmpi(tribtitle, "Snuff")
                    && strcmpi(tribtitle, "The Wee Free Men"))
                    Strcat(verbuf, "  I may have been misquoted there.");
                verbl_msg = verbuf;
            }
            context.tribute.Deathnotice = 1;
        } else if (ms_Death && rn2(3) && Death_quote(verbuf, sizeof verbuf)) {
            verbl_msg = verbuf;
        /* end of tribute addition */

        } else if (ms_Death && !rn2(10)) {
            pline_msg = "is busy reading a copy of Sandman #8.";
        } else
            verbl_msg = "Who do you think you are, War?";
        break;
    } /* case MS_RIDER */
    } /* switch */

    if (pline_msg) {
        pline("%s %s", Monnam(mtmp), pline_msg);
    } else if (mtmp->mcan && verbl_msg_mcan) {
        verbalize1(verbl_msg_mcan);
    } else if (verbl_msg) {
        /* more 3.6 tribute */
        if (ptr == &mons[PM_DEATH]) {
            /* Death talks in CAPITAL LETTERS
               and without quotation marks */
            char tmpbuf[BUFSZ];

            pline1(ucase(strcpy(tmpbuf, verbl_msg)));
        } else {
            verbalize1(verbl_msg);
        }
    }
    return 1;
}

/* #chat command */
int
dotalk()
{
    int result;

    result = dochat();
    return result;
}

STATIC_OVL int
dochat()
{
    struct monst *mtmp;
    int tx, ty;
    struct obj *otmp;

    if (is_silent(youmonst.data)) {
        pline("As %s, you cannot speak.", an(youmonst.data->mname));
        return 0;
    }
    if (Strangled) {
        You_cant("speak.  You're choking!");
        return 0;
    }
    if (u.uswallow || Hidinshell) {
        pline("They won't hear you out there.");
        return 0;
    }
    if (Underwater) {
        Your("speech is unintelligible underwater.");
        return 0;
    }
    if (Deaf) {
        pline("How can you hold a conversation when you cannot hear?");
        return 0;
    }

    if (!Blind && (otmp = shop_object(u.ux, u.uy)) != (struct obj *) 0) {
        /* standing on something in a shop and chatting causes the shopkeeper
           to describe the price(s).  This can inhibit other chatting inside
           a shop, but that shouldn't matter much.  shop_object() returns an
           object iff inside a shop and the shopkeeper is present and willing
           (not angry) and able (not asleep) to speak and the position
           contains any objects other than just gold.
        */
        price_quote(otmp);
        return 1;
    }

    if (!getdir("Talk to whom? (in what direction)")) {
        /* decided not to chat */
        return 0;
    }

    if (u.usteed && u.dz > 0) {
        if (!u.usteed->mcanmove || u.usteed->msleeping) {
            pline("%s seems not to notice you.", Monnam(u.usteed));
            return 1;
        } else
            return domonnoise(u.usteed);
    }

    if (u.dz) {
        pline("They won't hear you %s there.", u.dz < 0 ? "up" : "down");
        return 0;
    }

    if (u.dx == 0 && u.dy == 0) {
        /*
         * Let's not include this.
         * It raises all sorts of questions: can you wear
         * 2 helmets, 2 amulets, 3 pairs of gloves or 6 rings as a marilith,
         * etc...  --KAA
        if (u.umonnum == PM_ETTIN) {
            You("discover that your other head makes boring conversation.");
            return 1;
        }
         */
        pline("Talking to yourself is a bad habit for a dungeoneer.");
        return 0;
    }

    tx = u.ux + u.dx;
    ty = u.uy + u.dy;

    if (!isok(tx, ty))
        return 0;

    mtmp = m_at(tx, ty);

    if ((!mtmp || mtmp->mundetected)
        && (otmp = vobj_at(tx, ty)) != 0 && otmp->otyp == STATUE) {
        /* Talking to a statue */
        if (!Blind) {
            pline_The("%s seems not to notice you.",
                      /* if hallucinating, you can't tell it's a statue */
                      Hallucination ? rndmonnam((char *) 0) : "statue");
        }
        return 0;
    }

    if (!mtmp || mtmp->mundetected || M_AP_TYPE(mtmp) == M_AP_FURNITURE
        || M_AP_TYPE(mtmp) == M_AP_OBJECT)
        return 0;

    /* sleeping monsters won't talk, except priests (who wake up) */
    if ((!mtmp->mcanmove || mtmp->msleeping) && !mtmp->ispriest) {
        /* If it is unseen, the player can't tell the difference between
           not noticing him and just not existing, so skip the message. */
        if (canspotmon(mtmp))
            pline("%s seems not to notice you.", Monnam(mtmp));
        return 0;
    }

    /* if this monster is waiting for something, prod it into action */
    mtmp->mstrategy &= ~STRAT_WAITMASK;

    if (mtmp->mtame && mtmp->meating) {
        if (!canspotmon(mtmp))
            map_invisible(mtmp->mx, mtmp->my);
        pline("%s is eating noisily.", Monnam(mtmp));
        return 0;
    }

    if (Role_if(PM_DRUID) && u.ulevel > 2
        && is_woodland_creature(mtmp->data)) {
        if (!mtmp->mtame) {
            You("speak with the %s in its native tongue...",
                l_monnam(mtmp));
            if (rnl(20) < 2) {
                pline("%s agrees to help you.",
                      Monnam(mtmp));
                (void) tamedog(mtmp, (struct obj *) 0);
            } else {
                if (rnl(10) < 5) {
                    if (!mtmp->mpeaceful) {
                        mtmp->mpeaceful = 1;
                        set_malign(mtmp);
                    }
                    pline("%s enjoys the chat.",
                          Monnam(mtmp));
                } else {
                    pline("%s ignores you.", Monnam(mtmp));
                }
            }
            return 1;
        }
    }

    if ((Role_if(PM_CONVICT) && is_rat(mtmp->data))
        || (Race_if(PM_DRAUGR) && is_zombie(mtmp->data))) {
        if (!mtmp->mpeaceful) {
            if (Role_if(PM_CONVICT))
                You("attempt to soothe the %s with chittering sounds...",
                    l_monnam(mtmp));
            else
                You("attempt to impose your will over the %s...",
                    l_monnam(mtmp));

            if (rnl(10) < (Role_if(PM_CONVICT) ? 2 : 1)) {
                (void) tamedog(mtmp, (struct obj *) 0);
            } else {
                if (Role_if(PM_CONVICT)) {
                    pline("%s unfortunately ignores your overtures.",
                          Monnam(mtmp));
                    if (rnl(10) < 3) {
                        mtmp->mpeaceful = 1;
                        set_malign(mtmp);
                    }
                } else {
                    pline("%s ignores you.", Monnam(mtmp));
                }
                return 1;
            }
        }
    }

    return domonnoise(mtmp);
}

#ifdef USER_SOUNDS

extern void FDECL(play_usersound, (const char *, int));

typedef struct audio_mapping_rec {
    struct nhregex *regex;
    char *filename;
    int volume;
    struct audio_mapping_rec *next;
} audio_mapping;

static audio_mapping *soundmap = 0;

char *sounddir = ".";

/* adds a sound file mapping, returns 0 on failure, 1 on success */
int
add_sound_mapping(mapping)
const char *mapping;
{
    char text[256];
    char filename[256];
    char filespec[256];
    int volume;

    if (sscanf(mapping, "MESG \"%255[^\"]\"%*[\t ]\"%255[^\"]\" %d", text,
               filename, &volume) == 3) {
        audio_mapping *new_map;

        if (strlen(sounddir) + strlen(filename) > 254) {
            raw_print("sound file name too long");
            return 0;
        }
        Sprintf(filespec, "%s/%s", sounddir, filename);

        if (can_read_file(filespec)) {
            new_map = (audio_mapping *) alloc(sizeof(audio_mapping));
            new_map->regex = regex_init();
            new_map->filename = dupstr(filespec);
            new_map->volume = volume;
            new_map->next = soundmap;

            if (!regex_compile(text, new_map->regex)) {
                raw_print(regex_error_desc(new_map->regex));
                regex_free(new_map->regex);
                free(new_map->filename);
                free(new_map);
                return 0;
            } else {
                soundmap = new_map;
            }
        } else {
            Sprintf(text, "cannot read %.243s", filespec);
            raw_print(text);
            return 0;
        }
    } else {
        raw_print("syntax error in SOUND");
        return 0;
    }

    return 1;
}

void
play_sound_for_message(msg)
const char *msg;
{
    audio_mapping *cursor = soundmap;

    while (cursor) {
        if (regex_match(msg, cursor->regex)) {
            play_usersound(cursor->filename, cursor->volume);
        }
        cursor = cursor->next;
    }
}

#endif /* USER_SOUNDS */

/* Find the single commandable pet if exactly one exists.
   Returns the pet if found, NULL if zero or multiple */
STATIC_OVL struct monst *
find_commandable_pet()
{
    struct monst *mtmp, *found = (struct monst *) 0;

    /* Check steed first - always commandable if present */
    if (u.usteed && u.usteed->mtame && has_edog(u.usteed)
        && !u.usteed->msleeping && !u.usteed->mstun
        && !u.usteed->mconf && !u.usteed->mfrozen) {
        found = u.usteed;
    }

    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp) || mtmp == u.usteed)
            continue;
        if (!mtmp->mtame || !has_edog(mtmp))
            continue;
        /* Pet must be alert */
        if (mtmp->msleeping || mtmp->mstun
            || mtmp->mconf || mtmp->mfrozen)
            continue;
        /* Must be able to communicate: visible, adjacent,
           or two-way telepathy */
        if (!canseemon(mtmp) && distu(mtmp->mx, mtmp->my) > 2
            && !(has_telepathy(mtmp) && (HTelepat || ETelepat)))
            continue;
        /* Found a valid pet */
        if (found)
            return (struct monst *) 0; /* more than one */
        found = mtmp;
    }
    return found;
}

/* Give orders to a pet */
int
doorder()
{
    struct monst *mtmp;
    coord cc;
    char buf[BUFSZ];
    int skill_level;
    winid win;
    menu_item *selected;
    anything any;
    int n, choice;
    long old_petstrat;
    boolean currently_set;

    /* Player can't be incapacitated */
    if (Confusion || Stunned) {
        You_cant("give orders in your current state.");
        return 0;
    }

    /* If exactly one commandable pet exists, auto-select it */
    mtmp = find_commandable_pet();
    if (mtmp) {
        cc.x = (mtmp == u.usteed) ? u.ux : mtmp->mx;
        cc.y = (mtmp == u.usteed) ? u.uy : mtmp->my;
    } else {
        /* Cursor-based targeting */
        cc.x = u.ux;
        cc.y = u.uy;
        if (getpos(&cc, FALSE, "the monster you want to issue orders to") < 0
            || !isok(cc.x, cc.y))
            return 0;
        mtmp = (struct monst *) 0; /* will be set below */
    }

    /* If not auto-selected, look up and validate the target */
    if (!mtmp) {
        /* Check for steed if targeting self */
        if (cc.x == u.ux && cc.y == u.uy) {
            if (u.usteed) {
                mtmp = u.usteed;
            } else {
                You("can't give orders to yourself.");
                return 0;
            }
        } else {
            mtmp = m_at(cc.x, cc.y);
        }

        if (!mtmp || !canspotmon(mtmp)) {
            pline("There's no one there to command.");
            return 0;
        }

        /* ESP lets you sense monsters, but you need to actually see them,
         * be adjacent, or have two-way telepathy to communicate orders.
         * Two-way telepathy: both you and the pet have ESP */
        if (!canseemon(mtmp) && distu(mtmp->mx, mtmp->my) > 2
            && !(has_telepathy(mtmp) && (HTelepat || ETelepat))) {
            You("sense %s, but are too far away to communicate.",
                mon_nam(mtmp));
            return 0;
        }

        if (!mtmp->mtame) {
            pline("%s is not your pet.", Monnam(mtmp));
            return 0;
        }

        if (!has_edog(mtmp)) {
            pline("%s doesn't respond to commands.", Monnam(mtmp));
            return 0;
        }

        /* Pet must be alert to receive orders */
        if (mtmp->msleeping) {
            pline("%s is asleep.", Monnam(mtmp));
            return 0;
        }
        /* Pet cannot be stunned/confused/paralyzed */
        if (mtmp->mstun || mtmp->mconf || mtmp->mfrozen) {
            pline("%s is incapacitated.", Monnam(mtmp));
            return 0;
        }
    }

    skill_level = P_SKILL(P_PET_HANDLING);

    /* Build order menu */
    win = create_nhwindow(NHW_MENU);
    start_menu(win);

    /* Orders available to everyone (Unskilled) */
    any = zeroany;
    any.a_int = 1;
    add_menu(win, NO_GLYPH, &any, 'a', 0, ATR_NONE,
             "Belay orders (clear all)", MENU_UNSELECTED);

    any.a_int = 2;
    currently_set = (EDOG(mtmp)->petstrat & PETSTRAT_STAY) != 0;
    Sprintf(buf, "Stay on this level (toggle) [%s]",
            currently_set ? "active" : "inactive");
    menuitemcolor = currently_set ? CLR_BRIGHT_GREEN : CLR_ORANGE;
    add_menu(win, NO_GLYPH, &any, 'b', 0, ATR_NONE, buf, MENU_UNSELECTED);

    any.a_int = 3;
    currently_set = (EDOG(mtmp)->petstrat & PETSTRAT_NOAPPORT) != 0;
    Sprintf(buf, "Don't pick up items (toggle) [%s]",
            currently_set ? "active" : "inactive");
    menuitemcolor = currently_set ? CLR_BRIGHT_GREEN : CLR_ORANGE;
    add_menu(win, NO_GLYPH, &any, 'c', 0, ATR_NONE, buf, MENU_UNSELECTED);

    any.a_int = 4;
    currently_set = (EDOG(mtmp)->petstrat & PETSTRAT_NODROP) != 0;
    Sprintf(buf, "Don't drop items (toggle) [%s]",
            currently_set ? "active" : "inactive");
    menuitemcolor = currently_set ? CLR_BRIGHT_GREEN : CLR_ORANGE;
    add_menu(win, NO_GLYPH, &any, 'd', 0, ATR_NONE, buf, MENU_UNSELECTED);

    any.a_int = 5;
    add_menu(win, NO_GLYPH, &any, 'e', 0, ATR_NONE,
             "Remove saddle", MENU_UNSELECTED);

    any.a_int = 6;
    add_menu(win, NO_GLYPH, &any, 'f', 0, ATR_NONE,
             "Remove barding", MENU_UNSELECTED);

    /* Orders requiring P_BASIC */
    if (skill_level >= P_BASIC) {
        any.a_int = 7;
        currently_set = (EDOG(mtmp)->petstrat & PETSTRAT_AVOIDPEACE) != 0;
        Sprintf(buf, "Avoid peacefuls (toggle) [%s]",
                currently_set ? "active" : "inactive");
        menuitemcolor = currently_set ? CLR_BRIGHT_GREEN : CLR_ORANGE;
        add_menu(win, NO_GLYPH, &any, 'g', 0, ATR_NONE, buf, MENU_UNSELECTED);

        any.a_int = 8;
        add_menu(win, NO_GLYPH, &any, 'h', 0, ATR_NONE,
                 "Give items to pet", MENU_UNSELECTED);

        any.a_int = 9;
        add_menu(win, NO_GLYPH, &any, 'i', 0, ATR_NONE,
                 "Take items from pet", MENU_UNSELECTED);
    }

    /* Orders requiring P_SKILLED */
    if (skill_level >= P_SKILLED) {
        any.a_int = 10;
        currently_set = (EDOG(mtmp)->petstrat & PETSTRAT_AGGRO) != 0;
        Sprintf(buf, "Aggressive stance (toggle) [%s]",
                currently_set ? "active" : "inactive");
        menuitemcolor = currently_set ? CLR_BRIGHT_GREEN : CLR_ORANGE;
        add_menu(win, NO_GLYPH, &any, 'j', 0, ATR_NONE, buf, MENU_UNSELECTED);

        any.a_int = 11;
        currently_set = (EDOG(mtmp)->petstrat & PETSTRAT_COWED) != 0;
        Sprintf(buf, "Defensive stance (toggle) [%s]",
                currently_set ? "active" : "inactive");
        menuitemcolor = currently_set ? CLR_BRIGHT_GREEN : CLR_ORANGE;
        add_menu(win, NO_GLYPH, &any, 'k', 0, ATR_NONE, buf, MENU_UNSELECTED);

        any.a_int = 12;
        currently_set = (EDOG(mtmp)->petstrat & PETSTRAT_IGNORETRAPS) != 0;
        Sprintf(buf, "Ignore harmless traps (toggle) [%s]",
                currently_set ? "active" : "inactive");
        menuitemcolor = currently_set ? CLR_BRIGHT_GREEN : CLR_ORANGE;
        add_menu(win, NO_GLYPH, &any, 'l', 0, ATR_NONE, buf, MENU_UNSELECTED);
    }

    /* Expert skill required for these advanced orders */
    if (skill_level >= P_EXPERT) {
        any.a_int = 13;
        currently_set = (EDOG(mtmp)->petstrat & PETSTRAT_STATIONARY) != 0;
        Sprintf(buf, "Stay here (toggle) [%s]",
                currently_set ? "active" : "inactive");
        menuitemcolor = currently_set ? CLR_BRIGHT_GREEN : CLR_ORANGE;
        add_menu(win, NO_GLYPH, &any, 'm', 0, ATR_NONE, buf, MENU_UNSELECTED);

        any.a_int = 14;
        add_menu(win, NO_GLYPH, &any, 'n', 0, ATR_NONE,
                 "Come to my location", MENU_UNSELECTED);

        any.a_int = 15;
        currently_set = (EDOG(mtmp)->petstrat & PETSTRAT_NOATTACK) != 0;
        Sprintf(buf, "Avoid all monsters (toggle) [%s]",
                currently_set ? "active" : "inactive");
        menuitemcolor = currently_set ? CLR_BRIGHT_GREEN : CLR_ORANGE;
        add_menu(win, NO_GLYPH, &any, 'o', 0, ATR_NONE, buf, MENU_UNSELECTED);
    }

    Sprintf(buf, "What do you want %s to do?", mon_nam(mtmp));
    end_menu(win, buf);

    n = select_menu(win, PICK_ONE, &selected);
    destroy_nhwindow(win);

    if (n <= 0)
        return 0;  /* cancelled */

    choice = selected[0].item.a_int;
    free((genericptr_t) selected);

    /* Commands that require tameness check (unless Master skill).
     * Skip check for: belay (1), remove saddle (5), remove barding (6),
     * give items (8), take items (9) - physical handoffs always succeed
     * at Basic+ since they require adjacency.
     * Base rate: (tameness-1)/19, so tameness 1 = 0%, tameness 20 = 100%
     * Skill bonuses: Unskilled +0%, Basic +10%, Skilled +20%, Expert +35%
     * Master skill: all commands always succeed regardless of tameness
     */
    if (choice != 1 && choice != 5 && choice != 6 && choice != 8 && choice != 9
        && skill_level != P_MASTER) {
        int skill_bonus;
        int effective_tameness;

        switch (skill_level) {
        case P_BASIC:
            skill_bonus = 2;
            break;
        case P_SKILLED:
            skill_bonus = 4;
            break;
        case P_EXPERT:
            skill_bonus = 7;
            break;
        default:
            skill_bonus = 0;
            break;
        }
        effective_tameness = (mtmp->mtame - 1) + skill_bonus;

        if (effective_tameness < 19 && rn2(19) >= effective_tameness) {
            pline("%s ignores you.", Monnam(mtmp));
            return 1;  /* still uses a turn */
        }
    }

    /* Physical actions (saddle/barding removal, give/take items) require
     * adjacency. Behavioral orders can be shouted across the room.
     * Note: when mounted, mtmp == u.usteed and shares player position,
     * so distu() will be 0 which passes the check.
     */
    if (choice == 5 || choice == 6 || choice == 8 || choice == 9) {
        if (distu(mtmp->mx, mtmp->my) > 2) {
            You("need to be next to %s to do that.", mon_nam(mtmp));
            return 0;
        }
    }

    /* Save old strategy to check if order actually changed anything */
    old_petstrat = EDOG(mtmp)->petstrat;

    /* Process selection */
    switch (choice) {
    case 1: /* Belay orders */
        EDOG(mtmp)->petstrat = 0L;
        You("leave the actions of %s up to %s own discretion.",
            mon_nam(mtmp), noit_mhis(mtmp));
        break;
    case 2: /* Stay (toggle) */
        EDOG(mtmp)->petstrat ^= PETSTRAT_STAY;
        if (EDOG(mtmp)->petstrat & PETSTRAT_STAY)
            You("direct %s to stay on this level.", mon_nam(mtmp));
        else
            You("direct %s to follow you between levels.", mon_nam(mtmp));
        break;
    case 3: /* No apport (toggle) */
        EDOG(mtmp)->petstrat ^= PETSTRAT_NOAPPORT;
        if (EDOG(mtmp)->petstrat & PETSTRAT_NOAPPORT)
            You("direct %s to not pick up items.", mon_nam(mtmp));
        else
            You("direct %s to pick up items again.", mon_nam(mtmp));
        break;
    case 4: /* No drop (toggle) */
        EDOG(mtmp)->petstrat ^= PETSTRAT_NODROP;
        if (EDOG(mtmp)->petstrat & PETSTRAT_NODROP)
            You("direct %s to hold onto %s items.", mon_nam(mtmp),
                noit_mhis(mtmp));
        else
            You("direct %s to drop unwanted items.", mon_nam(mtmp));
        break;
    case 5: /* Remove saddle - always succeeds */
        {
            struct obj *otmp = which_armor(mtmp, W_SADDLE);

            if (!otmp) {
                pline("%s has no saddle to remove.", Monnam(mtmp));
            } else {
                You("remove %s from %s.", the(xname(otmp)),
                    x_monnam(mtmp, ARTICLE_THE, (char *) 0,
                             SUPPRESS_SADDLE, FALSE));
                /* unwear the item */
                update_mon_intrinsics(mtmp, otmp, FALSE, FALSE);
                otmp->owornmask = 0L;
                otmp->owt = weight(otmp);
                mtmp->misc_worn_check &= ~W_SADDLE;
                check_gear_next_turn(mtmp);
                /* give to player */
                extract_from_minvent(mtmp, otmp, FALSE, TRUE);
                (void) hold_another_object(otmp, "You take, but drop, %s.",
                                           doname(otmp), "You take: ");
            }
        }
        break;
    case 6: /* Remove barding - always succeeds */
        {
            struct obj *otmp = which_armor(mtmp, W_BARDING);

            if (!otmp) {
                pline("%s has no barding to remove.", Monnam(mtmp));
            } else {
                You("remove %s from %s.", the(xname(otmp)),
                    x_monnam(mtmp, ARTICLE_THE, (char *) 0,
                             SUPPRESS_BARDING, FALSE));
                /* unwear the item */
                update_mon_intrinsics(mtmp, otmp, FALSE, FALSE);
                otmp->owornmask = 0L;
                otmp->owt = weight(otmp);
                mtmp->misc_worn_check &= ~W_BARDING;
                check_gear_next_turn(mtmp);
                /* give to player */
                extract_from_minvent(mtmp, otmp, FALSE, TRUE);
                (void) hold_another_object(otmp, "You take, but drop, %s.",
                                           doname(otmp), "You take: ");
            }
        }
        break;
    case 7: /* Avoid peacefuls (toggle) */
        EDOG(mtmp)->petstrat ^= PETSTRAT_AVOIDPEACE;
        if (EDOG(mtmp)->petstrat & PETSTRAT_AVOIDPEACE)
            You("direct %s to avoid peaceful creatures.", mon_nam(mtmp));
        else
            You("direct %s to attack peaceful creatures at will.",
                mon_nam(mtmp));
        break;
    case 8: /* Give items to pet */
        (void) exchange_objects_with_mon(mtmp, FALSE);
        break;
    case 9: /* Take items from pet */
        (void) exchange_objects_with_mon(mtmp, TRUE);
        break;
    case 10: /* Aggressive (toggle) - mutually exclusive with Defensive/NoAttack */
        EDOG(mtmp)->petstrat ^= PETSTRAT_AGGRO;
        if (EDOG(mtmp)->petstrat & PETSTRAT_AGGRO) {
            EDOG(mtmp)->petstrat &= ~(PETSTRAT_COWED | PETSTRAT_NOATTACK);
            You("direct %s to assume an aggressive posture.",
                mon_nam(mtmp));
        } else {
            You("direct %s to no longer be aggressive.",
                mon_nam(mtmp));
        }
        break;
    case 11: /* Defensive (toggle) - mutually exclusive with Aggressive/NoAttack */
        EDOG(mtmp)->petstrat ^= PETSTRAT_COWED;
        if (EDOG(mtmp)->petstrat & PETSTRAT_COWED) {
            EDOG(mtmp)->petstrat &= ~(PETSTRAT_AGGRO | PETSTRAT_NOATTACK);
            You("direct %s to assume a more defensive posture.",
                mon_nam(mtmp));
        } else {
            You("direct %s to no longer be defensive.",
                mon_nam(mtmp));
        }
        break;
    case 12: /* Ignore harmless traps (toggle) */
        EDOG(mtmp)->petstrat ^= PETSTRAT_IGNORETRAPS;
        if (EDOG(mtmp)->petstrat & PETSTRAT_IGNORETRAPS)
            You("direct %s to ignore harmless traps.", mon_nam(mtmp));
        else
            You("direct %s to avoid all traps.", mon_nam(mtmp));
        break;
    case 13: /* Stay here (toggle) - mutually exclusive with Come */
        EDOG(mtmp)->petstrat ^= PETSTRAT_STATIONARY;
        if (EDOG(mtmp)->petstrat & PETSTRAT_STATIONARY) {
            EDOG(mtmp)->petstrat &= ~PETSTRAT_COME;
            You("direct %s to stay right there.", mon_nam(mtmp));
        } else {
            You("direct %s to move freely again.", mon_nam(mtmp));
        }
        break;
    case 14: /* Come (one-shot) - mutually exclusive with Stay here */
        EDOG(mtmp)->petstrat &= ~PETSTRAT_STATIONARY;
        EDOG(mtmp)->petstrat |= PETSTRAT_COME;
        You("call %s to your side.", mon_nam(mtmp));
        break;
    case 15: /* Avoid all monsters (toggle) - mutually exclusive with Aggressive/Defensive */
        EDOG(mtmp)->petstrat ^= PETSTRAT_NOATTACK;
        if (EDOG(mtmp)->petstrat & PETSTRAT_NOATTACK) {
            EDOG(mtmp)->petstrat &= ~(PETSTRAT_AGGRO | PETSTRAT_COWED);
            You("direct %s to avoid all monsters.", mon_nam(mtmp));
        } else {
            You("direct %s to engage monsters normally.", mon_nam(mtmp));
        }
        break;
    }

    /* Only train skill if the order actually changed the pet's behavior */
    if (EDOG(mtmp)->petstrat != old_petstrat)
        use_skill(P_PET_HANDLING, 1);

    return 1;  /* action took a turn */
}

/*sounds.c*/
