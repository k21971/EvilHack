/* NetHack 3.6	spell.c	$NHDT-Date: 1546565814 2019/01/04 01:36:54 $  $NHDT-Branch: NetHack-3.6.2-beta01 $:$NHDT-Revision: 1.88 $ */
/*      Copyright (c) M. Stephenson 1988                          */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* spellmenu arguments; 0 thru n-1 used as spl_book[] index when swapping */
#define SPELLMENU_CAST (-2)
#define SPELLMENU_VIEW (-1)
#define SPELLMENU_SORT (MAXSPELL) /* special menu entry */

/* spell retention period, in turns; at 10% of this value, player becomes
   eligible to reread the spellbook and regain 100% retention (the threshold
   used to be 1000 turns, which was 10% of the original 10000 turn retention
   period but didn't get adjusted when that period got doubled to 20000) */
#define KEEN 20000
/* x: need to add 1 when used for reading a spellbook rather than for hero
   initialization; spell memory is decremented at the end of each turn,
   including the turn on which the spellbook is read; without the extra
   increment, the hero used to get cheated out of 1 turn of retention */
#define incrnknow(spell, x) (spl_book[spell].sp_know = KEEN + (x))

#define spellev(spell) spl_book[spell].sp_lev
#define spellname(spell) OBJ_NAME(objects[spellid(spell)])
#define spellet(spell) \
    ((char) ((spell < 26) ? ('a' + spell) : ('A' + spell - 26)))

STATIC_DCL int FDECL(spell_let_to_idx, (CHAR_P));
STATIC_DCL boolean FDECL(cursed_book, (struct obj * bp));
STATIC_DCL boolean FDECL(confused_book, (struct obj *));
STATIC_DCL void FDECL(deadbook, (struct obj *));
STATIC_PTR int NDECL(learn);
STATIC_DCL boolean NDECL(rejectcasting);
STATIC_DCL boolean FDECL(getspell, (int *));
STATIC_PTR int FDECL(CFDECLSPEC spell_cmp, (const genericptr,
                                            const genericptr));
STATIC_DCL void NDECL(sortspells);
STATIC_DCL boolean NDECL(spellsortmenu);
STATIC_DCL boolean FDECL(dospellmenu, (const char *, int, int *));
STATIC_DCL int FDECL(percent_success, (int));
STATIC_DCL char *FDECL(spellretention, (int, char *));
STATIC_DCL int NDECL(throwspell);
STATIC_DCL void FDECL(spell_backfire, (int));
STATIC_DCL boolean FDECL(spell_aim_step, (genericptr_t, int, int));

static const char clothes[] = { ARMOR_CLASS, 0 };

/* The roles[] table lists the role-specific values for tuning
 * percent_success().
 *
 * Reasoning:
 *   spelbase, spelheal:
 *      Arc are aware of magic through historical research
 *      Bar abhor magic (Conan finds it "interferes with his animal instincts")
 *      Cav are ignorant to magic
 *      Hea are very aware of healing magic through medical research
 *      Inf learned a lot of black magic from their cult
 *      Kni are moderately aware of healing from Paladin training
 *      Mon use magic to attack and defend in lieu of weapons and armor
 *      Pri are very aware of healing magic through theological research
 *      Ran avoid magic, preferring to fight unseen and unheard
 *      Rog are moderately aware of magic through trickery
 *      Sam have limited magical awareness, preferring meditation to conjuring
 *      Tou are aware of magic from all the great films they have seen
 *      Val have limited magical awareness, preferring fighting
 *      Wiz are trained mages
 *
 *      The arms penalty is lessened for trained fighters Bar, Kni, Ran,
 *      Sam, Val -- the penalty is its metal interference, not encumbrance.
 *      The `spelspec' is a single spell which is fundamentally easier
 *      for that role to cast.
 *
 *  spelspec, spelsbon:
 *      Arc map masters (SPE_MAGIC_MAPPING)
 *      Bar to see their enemies driven before them (SPE_CAUSE_FEAR)
 *      Hea to heal (SPE_CURE_SICKNESS)
 *      Inf to channel hellfire (SPE_FIREBALL)
 *      Kni to turn back evil (SPE_TURN_UNDEAD)
 *      Mon to preserve their abilities (SPE_RESTORE_ABILITY)
 *      Pri to bless (SPE_REMOVE_CURSE)
 *      Ran to hide (SPE_INVISIBILITY)
 *      Rog to find loot (SPE_DETECT_TREASURE)
 *      Sam to be At One (SPE_CLAIRVOYANCE)
 *      Tou to smile (SPE_CHARM_MONSTER)
 *      Val to maintain their armor (SPE_REPAIR_ARMOR)
 *      Wiz all really, but SPE_MAGIC_MISSILE is their party trick
 *
 *      See percent_success() below for more comments.
 *
 *  uarmbon, uarmsbon, uarmhbon, uarmgbon, uarmfbon:
 *      Fighters find body armour & shield a little less limiting.
 *      Headgear, Gauntlets and Footwear are not role-specific (but
 *      still have an effect, except helm of brilliance, which is designed
 *      to permit magic-use).
 */

#define uarmhbon 4 /* Metal helmets interfere with the mind */
#define uarmgbon 6 /* Casting channels through the hands */
#define uarmfbon 2 /* All metal interferes to some degree */

/* since the spellbook itself doesn't blow up, don't say just "explodes" */
static const char explodes[] = "radiates explosive energy";

/* convert a letter into a number in the range 0..51, or -1 if not a letter */
STATIC_OVL int
spell_let_to_idx(ilet)
char ilet;
{
    int indx;

    indx = ilet - 'a';
    if (indx >= 0 && indx < 26)
        return indx;
    indx = ilet - 'A';
    if (indx >= 0 && indx < 26)
        return indx + 26;
    return -1;
}

/* TRUE: book should be destroyed by caller */
STATIC_OVL boolean
cursed_book(bp)
struct obj *bp;
{
    boolean was_in_use;
    int lev = objects[bp->otyp].oc_level;
    int dmg = 0;

    switch (rn2(lev)) {
    case 0:
        You_feel("a wrenching sensation.");
        tele(); /* teleport him */
        break;
    case 1:
        You_feel("threatened.");
        aggravate();
        break;
    case 2:
        make_blinded(Blinded + rn1(100, 250), TRUE);
        break;
    case 3:
        take_gold();
        break;
    case 4:
        pline("These runes were just too much to comprehend.");
        make_confused(HConfusion + rn1(7, 16), FALSE);
        break;
    case 5:
        pline_The("book was coated with contact poison!");
        if (uarmg) {
            erode_obj(uarmg, "gloves", ERODE_CORRODE, EF_GREASE | EF_VERBOSE);
            break;
        }
        /* temp disable in_use; death should not destroy the book */
        was_in_use = bp->in_use;
        bp->in_use = FALSE;
        losestr(resist_reduce(rn1(4, 3), POISON_RES) + rn1(2, 1));
        losehp(resist_reduce(rnd(6), POISON_RES) + rnd(6), "contact-poisoned spellbook",
               KILLED_BY_AN);
        bp->in_use = was_in_use;
        break;
    case 6:
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
            pline_The("book %s, but you are unharmed!", explodes);
        } else {
            pline("As you read the book, it %s in your %s!", explodes,
                  body_part(FACE));
            dmg = 2 * rnd(10) + 5;
            losehp(Maybe_Half_Phys(dmg), "exploding rune", KILLED_BY_AN);
        }
        return TRUE;
    default:
        rndcurse();
        break;
    }
    return FALSE;
}

/* study while confused: returns TRUE if the book is destroyed */
STATIC_OVL boolean
confused_book(spellbook)
struct obj *spellbook;
{
    boolean gone = FALSE;

    if (!rn2(3) && spellbook->otyp != SPE_BOOK_OF_THE_DEAD) {
        spellbook->in_use = TRUE; /* in case called from learn */
        pline(
         "Being confused you have difficulties in controlling your actions.");
        display_nhwindow(WIN_MESSAGE, FALSE);
        You("accidentally tear the spellbook to pieces.");
        if (!objects[spellbook->otyp].oc_name_known
            && !objects[spellbook->otyp].oc_uname)
            docall(spellbook);
        useup(spellbook);
        gone = TRUE;
    } else {
        You("find yourself reading the %s line over and over again.",
            spellbook == context.spbook.book ? "next" : "first");
    }
    return gone;
}

/* special effects for The Book of the Dead */
STATIC_OVL void
deadbook(book2)
struct obj *book2;
{
    struct monst *mtmp, *mtmp2;
    coord mm;

    You("turn the pages of the Book of the Dead...");
    makeknown(SPE_BOOK_OF_THE_DEAD);
    /* KMH -- Need ->known to avoid "_a_ Book of the Dead" */
    book2->known = 1;
    if (invocation_pos(u.ux, u.uy) && !On_stairs(u.ux, u.uy)) {
        register struct obj *otmp;
        register boolean arti1_primed = FALSE, arti2_primed = FALSE,
                         arti_cursed = FALSE;

        if (book2->cursed) {
            pline_The("runes appear scrambled.  You can't read them!");
            return;
        }

        if (!u.uhave.bell || !u.uhave.menorah) {
            pline("A chill runs down your %s.", body_part(SPINE));
            if (!u.uhave.bell)
                You_hear("a faint chime...");
            if (!u.uhave.menorah)
                pline("Vlad's doppelganger is amused.");
            return;
        }

        for (otmp = invent; otmp; otmp = otmp->nobj) {
            if (otmp->otyp == CANDELABRUM_OF_INVOCATION && otmp->spe == 7
                && otmp->lamplit) {
                if (!otmp->cursed)
                    arti1_primed = TRUE;
                else
                    arti_cursed = TRUE;
            }
            if (otmp->otyp == BELL_OF_OPENING
                && (moves - otmp->age) < 5L) { /* you rang it recently */
                if (!otmp->cursed)
                    arti2_primed = TRUE;
                else
                    arti_cursed = TRUE;
            }
        }

        if (arti_cursed) {
            pline_The("invocation fails!");
            pline("At least one of your artifacts is cursed...");
        } else if (arti1_primed && arti2_primed) {
            unsigned soon =
                (unsigned) d(2, 6); /* time til next intervene() */

            /* successful invocation */
            mkinvokearea();
            u.uevent.invoked = 1;
            /* in case you haven't killed the Wizard yet, behave as if
               you just did */
            u.uevent.udemigod = 1; /* wizdead() */
            if (!u.udg_cnt || u.udg_cnt > soon)
                u.udg_cnt = soon;
        } else { /* at least one artifact not prepared properly */
            You("have a feeling that %s is amiss...", something);
            goto raise_dead;
        }
        return;
    }

    /* when not an invocation situation */
    if (book2->cursed) {
    raise_dead:

        You("raised the dead!");
        /* first maybe place a dangerous adversary */
        if (!rn2(3) && ((mtmp = makemon(&mons[PM_MASTER_LICH], u.ux, u.uy,
                                        NO_MINVENT)) != 0
                        || (mtmp = makemon(&mons[PM_NALFESHNEE], u.ux, u.uy,
                                           NO_MINVENT)) != 0)) {
            mtmp->mpeaceful = 0;
            set_malign(mtmp);
        }
        /* next handle the affect on things you're carrying */
        (void) unturn_dead(&youmonst);
        /* last place some monsters around you */
        mm.x = u.ux;
        mm.y = u.uy;
        mkundead(&mm, TRUE, NO_MINVENT);
    } else if (book2->blessed) {
        for (mtmp = fmon; mtmp; mtmp = mtmp2) {
            mtmp2 = mtmp->nmon; /* tamedog() changes chain */
            if (DEADMONSTER(mtmp))
                continue;

            if ((is_undead(mtmp->data) || is_vampshifter(mtmp))
                && cansee(mtmp->mx, mtmp->my)) {
                mtmp->mpeaceful = TRUE;
                if (sgn(mon_aligntyp(mtmp)) == sgn(u.ualign.type)
                    && distu(mtmp->mx, mtmp->my) < 4)
                    if (mtmp->mtame) {
                        if (mtmp->mtame < 20)
                            mtmp->mtame++;
                    } else
                        (void) tamedog(mtmp, (struct obj *) 0);
                else
                    monflee(mtmp, 0, FALSE, TRUE);
            }
        }
    } else {
        switch (rn2(3)) {
        case 0:
            Your("ancestors are annoyed with you!");
            break;
        case 1:
            pline_The("headstones in the cemetery begin to move!");
            break;
        default:
            pline("Oh my!  Your name appears in the book!");
        }
    }
    return;
}

/* 'book' has just become cursed; if we're reading it and realize it is
   now cursed, interrupt */
void
book_cursed(book)
struct obj *book;
{
    if (occupation == learn && context.spbook.book == book
        && book->cursed && book->bknown && multi >= 0)
        stop_occupation();
}

STATIC_PTR int
learn(VOID_ARGS)
{
    int i;
    short booktype;
    char splname[BUFSZ];
    boolean costly = TRUE;
    struct obj *book = context.spbook.book;

    if (!book) {
        context.spbook.delay = 0;
        context.spbook.o_id = 0;
        return 0;
    }

    /* JDS: lenses give 50% faster reading; 33% smaller read time */
    if (context.spbook.delay && ublindf && ublindf->otyp == LENSES && rn2(2))
        context.spbook.delay++;
    if (Confusion
        || (Race_if(PM_DRAUGR) && book
            && book->otyp != SPE_BOOK_OF_THE_DEAD)) { /* became confused while learning */
        (void) confused_book(book);
        context.spbook.book = 0; /* no longer studying */
        context.spbook.o_id = 0;
        nomul(context.spbook.delay); /* remaining delay is uninterrupted */
        multi_reason = "reading a book";
        nomovemsg = 0;
        context.spbook.delay = 0;
        return 0;
    }
    if (context.spbook.delay) {
        /* not if (context.spbook.delay++), so at end delay == 0 */
        context.spbook.delay++;
        return 1; /* still busy */
    }
    exercise(A_WIS, TRUE); /* you're studying. */
    booktype = book->otyp;
    if (booktype == SPE_BOOK_OF_THE_DEAD) {
        deadbook(book);
        return 0;
    }

    Sprintf(splname,
            objects[booktype].oc_name_known ? "\"%s\"" : "the \"%s\" spell",
            OBJ_NAME(objects[booktype]));
    for (i = 0; i < MAXSPELL; i++)
        if (spellid(i) == booktype || spellid(i) == NO_SPELL)
            break;

    if (i == MAXSPELL) {
        impossible("Too many spells memorized!");
    } else if (spellid(i) == booktype) {
        /* normal book can be read and re-read a total of 4 times */
        if (book->spestudied > MAX_SPELL_STUDY) {
            pline("This spellbook is too faint to be read any more.");
            book->otyp = booktype = SPE_BLANK_PAPER;
            set_material(book, PAPER);
            /* reset spestudied as if polymorph had taken place */
            book->spestudied = rn2(book->spestudied);
        } else if (spellknow(i) > KEEN / 10) {
            You("know %s quite well already.", splname);
            costly = FALSE;
        } else { /* spellknow(i) <= KEEN/10 */
            Your("knowledge of %s is %s.", splname,
                 spellknow(i) ? "keener" : "restored");
            incrnknow(i, 1);
            book->spestudied++;
            exercise(A_INT, TRUE); /* extra study */
        }
        makeknown((int) booktype);
    } else { /* (spellid(i) == NO_SPELL) */
        /* for a normal book, spestudied will be zero, but for
           a polymorphed one, spestudied will be non-zero and
           one less reading is available than when re-learning */
        if (book->spestudied >= MAX_SPELL_STUDY) {
            /* pre-used due to being the product of polymorph */
            pline("This spellbook is too faint to read even once.");
            book->otyp = booktype = SPE_BLANK_PAPER;
            set_material(book, PAPER);
            /* reset spestudied as if polymorph had taken place */
            book->spestudied = rn2(book->spestudied);
        } else {
            spl_book[i].sp_id = booktype;
            spl_book[i].sp_lev = objects[booktype].oc_level;
            incrnknow(i, 1);
            book->spestudied++;
            You(i > 0 ? "add %s to your repertoire." : "learn %s.", splname);
            exercise(A_INT, TRUE);
        }
        makeknown((int) booktype);
    }

    if (book->cursed) { /* maybe a demon cursed it */
        if (cursed_book(book)) {
            useup(book);
            context.spbook.book = 0;
            context.spbook.o_id = 0;
            return 0;
        }
    }
    if (costly)
        check_unpaid(book);
    context.spbook.book = 0;
    context.spbook.o_id = 0;
    update_inventory();
    return 0;
}

int
study_book(spellbook)
register struct obj *spellbook;
{
    int booktype = spellbook->otyp;
    boolean confused = (Confusion != 0);
    boolean too_hard = FALSE;

    /* attempting to read dull book may make hero fall asleep */
    if (!confused && !Sleep_resistance
        && !strcmp(OBJ_DESCR(objects[booktype]), "dull")) {
        const char *eyes;
        int dullbook = rnd(25) - ACURR(A_WIS);

        /* adjust chance if hero stayed awake, got interrupted, retries */
        if (context.spbook.delay && spellbook == context.spbook.book)
            dullbook -= rnd(objects[booktype].oc_level);

        if (dullbook > 0) {
            eyes = body_part(EYE);
            if (eyecount(youmonst.data) > 1)
                eyes = makeplural(eyes);
            pline("This book is so dull that you can't keep your %s open.",
                  eyes);
            dullbook += rnd(2 * objects[booktype].oc_level);
            fall_asleep(-dullbook, TRUE);
            return 1;
        }
    }

    if (context.spbook.delay && !confused && spellbook == context.spbook.book
        /* handle the sequence: start reading, get interrupted, have
           context.spbook.book become erased somehow, resume reading it */
        && booktype != SPE_BLANK_PAPER) {
        You("continue your efforts to %s.",
            (booktype == SPE_NOVEL) ? "read the novel" : "memorize the spell");
    } else {
        /* KMH -- Simplified this code */
        if (booktype == SPE_BLANK_PAPER) {
            pline("This spellbook is all blank.");
            makeknown(booktype);
            return 1;
        }

        /* Only Illithids get this 'ability' */
        if (booktype == SPE_PSIONIC_WAVE
            && !Race_if(PM_ILLITHID)) {
            You("do not understand the strange language this book is written in.");
            pline("The inscriptions in the book start to fade away!");
            spellbook->otyp = booktype = SPE_BLANK_PAPER;
            set_material(spellbook, PAPER);
            makeknown(booktype);
            return 1;
        }

        /* 3.6 tribute */
        if (booktype == SPE_NOVEL) {
            /* Obtain current Terry Pratchett book title */
            const char *tribtitle = noveltitle(&spellbook->novelidx);

            if (read_tribute("books", tribtitle, 0, (char *) 0, 0,
                             spellbook->o_id)) {
                if (!u.uconduct.literate++)
                    livelog_printf(LL_CONDUCT,
                                   "became literate by reading %s", tribtitle);
                check_unpaid(spellbook);
                makeknown(booktype);
                if (!u.uevent.read_tribute) {
                    /* give bonus of 20 xp and 4*20+0 pts */
                    more_experienced(20, 0);
                    newexplevel();
                    u.uevent.read_tribute = 1; /* only once */
                }
            }
            return 1;
        }

        switch (objects[booktype].oc_level) {
        case 1:
        case 2:
            context.spbook.delay = -objects[booktype].oc_delay;
            break;
        case 3:
        case 4:
            context.spbook.delay = -(objects[booktype].oc_level - 1)
                                   * objects[booktype].oc_delay;
            break;
        case 5:
        case 6:
            context.spbook.delay =
                -objects[booktype].oc_level * objects[booktype].oc_delay;
            break;
        case 7:
            context.spbook.delay = -8 * objects[booktype].oc_delay;
            break;
        default:
            impossible("Unknown spellbook level %d, book %d;",
                       objects[booktype].oc_level, booktype);
            return 0;
        }

        /* Books are often wiser than their readers (Rus.) */
        spellbook->in_use = TRUE;
        if ((!spellbook->blessed || (spellbook->blessed && Role_if(PM_CAVEMAN)))
            && spellbook->otyp != SPE_BOOK_OF_THE_DEAD) {
            if (spellbook->cursed) {
                too_hard = TRUE;
            } else {
                /* uncursed - chance to fail */
                int read_ability = ACURR(A_INT) + 4 + u.ulevel / 2
                                   - 2 * objects[booktype].oc_level
                             + ((ublindf && ublindf->otyp == LENSES) ? 2 : 0);

                /* only wizards know if a spell is too difficult */
                if (Role_if(PM_WIZARD) && read_ability < 20 && !confused) {
                    char qbuf[QBUFSZ];

                    Sprintf(qbuf,
                    "This spellbook is %sdifficult to comprehend.  Continue?",
                            (read_ability < 12 ? "very " : ""));
                    if (yn(qbuf) != 'y') {
                        spellbook->in_use = FALSE;
                        return 1;
                    }
                }
                /* Cavepersons always have at least a 20% chance to fail */
                if (Role_if(PM_CAVEMAN) && read_ability > 16)
                    read_ability = 16;
                /* its up to random luck now */
                if (rnd(20) > read_ability) {
                    too_hard = TRUE;
                }
            }
        }

        if (too_hard) {
            boolean gone = cursed_book(spellbook);

            nomul(context.spbook.delay); /* study time */
            multi_reason = "reading a book";
            nomovemsg = 0;
            context.spbook.delay = 0;
            if (gone || !rn2(3)) {
                if (!gone)
                    pline_The("spellbook crumbles to dust!");
                if (!objects[spellbook->otyp].oc_name_known
                    && !objects[spellbook->otyp].oc_uname)
                    docall(spellbook);
                useup(spellbook);
            } else
                spellbook->in_use = FALSE;
            return 1;
        } else if (confused) {
            if (!confused_book(spellbook)) {
                spellbook->in_use = FALSE;
            }
            nomul(context.spbook.delay);
            multi_reason = "reading a book";
            nomovemsg = 0;
            context.spbook.delay = 0;
            return 1;
        }
        spellbook->in_use = FALSE;

        You("begin to %s the runes.",
            spellbook->otyp == SPE_BOOK_OF_THE_DEAD ? "recite" : "memorize");
    }

    context.spbook.book = spellbook;
    if (context.spbook.book)
        context.spbook.o_id = context.spbook.book->o_id;
    set_occupation(learn, "studying", 0);
    return 1;
}

/* a spellbook has been destroyed or the character has changed levels;
   the stored address for the current book is no longer valid */
void
book_disappears(obj)
struct obj *obj;
{
    if (obj == context.spbook.book) {
        context.spbook.book = (struct obj *) 0;
        context.spbook.o_id = 0;
    }
}

/* renaming an object usually results in it having a different address;
   so the sequence start reading, get interrupted, name the book, resume
   reading would read the "new" book from scratch */
void
book_substitution(old_obj, new_obj)
struct obj *old_obj, *new_obj;
{
    if (old_obj == context.spbook.book) {
        context.spbook.book = new_obj;
        if (context.spbook.book)
            context.spbook.o_id = context.spbook.book->o_id;
    }
}

/* called from moveloop() */
void
age_spells()
{
    int i;
    /*
     * The time relative to the hero (a pass through move
     * loop) causes all spell knowledge to be decremented.
     * The hero's speed, rest status, conscious status etc.
     * does not alter the loss of memory.
     */
    for (i = 0; i < MAXSPELL && spellid(i) != NO_SPELL; i++)
        if (spellknow(i) && spellid(i) != SPE_PSIONIC_WAVE)
            decrnknow(i);
    return;
}

/* return True if spellcasting is inhibited;
   only covers a small subset of reasons why casting won't work */
STATIC_OVL boolean
rejectcasting()
{
    /* rejections which take place before selecting a particular spell */
    if (Stunned) {
        You("are too impaired to cast a spell.");
        return TRUE;
    } else if (!can_chant(&youmonst)) {
        You("are unable to chant the incantation.");
        return TRUE;
    } else if (!freehand()) {
        /* Note: !freehand() occurs when weapon and shield (or two-handed
         * weapon) are welded to hands, so "arms" probably doesn't need
         * to be makeplural(body_part(ARM)).
         */
        Your("arms are not free to cast!");
        return TRUE;
    } else if (nohands(youmonst.data)) {
        /* Note: only Druids in their #wildshape forms and vampires in
           their #shapechange forms can still cast spells without having
           actual arms/hands */
        if (druid_form || vampire_form) {
            return FALSE;
        } else {
            You("have no hands to cast!");
            return TRUE;
        }
    }
    return FALSE;
}

/*
 * Return TRUE if a spell was picked, with the spell index in the return
 * parameter.  Otherwise return FALSE.
 */
STATIC_OVL boolean
getspell(spell_no)
int *spell_no;
{
    int nspells, idx;
    char ilet, lets[BUFSZ], qbuf[QBUFSZ];

    if (spellid(0) == NO_SPELL) {
        You("don't know any spells right now.");
        return FALSE;
    }
    if (rejectcasting())
        return FALSE; /* no spell chosen */

    if (flags.menu_style == MENU_TRADITIONAL) {
        /* we know there is at least 1 known spell */
        for (nspells = 1; nspells < MAXSPELL && spellid(nspells) != NO_SPELL;
             nspells++)
            continue;

        if (nspells == 1)
            Strcpy(lets, "a");
        else if (nspells < 27)
            Sprintf(lets, "a-%c", 'a' + nspells - 1);
        else if (nspells == 27)
            Sprintf(lets, "a-zA");
        else if (nspells < 53)
            Sprintf(lets, "a-zA-%c", 'A' + nspells - 27);
        else if (nspells == 53)
            Sprintf(lets, "a-zA-Z0");
        /* up to 62 different known spells maximum.
           any more than this, and we'll need to include
           special characters as menu selection choices */
        else
            Sprintf(lets, "a-zA-Z0-%c", '9' + nspells - 11);

        for (;;) {
            Sprintf(qbuf, "Cast which spell? [%s *?]", lets);
            ilet = yn_function(qbuf, (char *) 0, '\0');
            if (ilet == '*' || ilet == '?')
                break; /* use menu mode */
            if (index(quitchars, ilet))
                return FALSE;

            idx = spell_let_to_idx(ilet);
            if (idx < 0 || idx >= nspells) {
                You("don't know that spell.");
                continue; /* ask again */
            }
            *spell_no = idx;
            return TRUE;
        }
    }
    return dospellmenu("Choose which spell to cast", SPELLMENU_CAST,
                       spell_no);
}

/* the 'Z' command -- cast a spell */
int
docast()
{
    int spell_no;

    if (getspell(&spell_no))
        return spelleffects(spell_no, FALSE, FALSE);
    return 0;
}

const char *
spelltypemnemonic(skill)
int skill;
{
    switch (skill) {
    case P_ATTACK_SPELL:
        return "attack";
    case P_HEALING_SPELL:
        return "healing";
    case P_DIVINATION_SPELL:
        return "divination";
    case P_ENCHANTMENT_SPELL:
        return "enchantment";
    case P_CLERIC_SPELL:
        return "clerical";
    case P_ESCAPE_SPELL:
        return "escape";
    case P_MATTER_SPELL:
        return "matter";
    case P_EVOCATION_SPELL:
        return "evocation";
    default:
        impossible("Unknown spell skill, %d;", skill);
        return "";
    }
}

int
spell_skilltype(booktype)
int booktype;
{
    return objects[booktype].oc_skill;
}

void
cast_protection()
{
    int l = u.ulevel, loglev = 0,
        gain, natac = u.uac + u.uspellprot;
    /* note: u.uspellprot is subtracted when find_ac() factors it into u.uac,
       so adding here factors it back out
       (versions prior to 3.6 had this backwards) */

    /* loglev=log2(u.ulevel)+1 (1..5) */
    while (l) {
        loglev++;
        l /= 2;
    }

    /* The more u.uspellprot you already have, the less you get,
     * and the better your natural ac, the less you get.
     *
     *  LEVEL AC    SPELLPROT from successive SPE_PROTECTION casts
     *      1     10    0,  1,  2,  3,  4
     *      1      0    0,  1,  2,  3
     *      1    -10    0,  1,  2
     *      2-3   10    0,  2,  4,  5,  6,  7,  8
     *      2-3    0    0,  2,  4,  5,  6
     *      2-3  -10    0,  2,  3,  4
     *      4-7   10    0,  3,  6,  8,  9, 10, 11, 12
     *      4-7    0    0,  3,  5,  7,  8,  9
     *      4-7  -10    0,  3,  5,  6
     *      7-15 -10    0,  3,  5,  6
     *      8-15  10    0,  4,  7, 10, 12, 13, 14, 15, 16
     *      8-15   0    0,  4,  7,  9, 10, 11, 12
     *      8-15 -10    0,  4,  6,  7,  8
     *     16-30  10    0,  5,  9, 12, 14, 16, 17, 18, 19, 20
     *     16-30   0    0,  5,  9, 11, 13, 14, 15
     *     16-30 -10    0,  5,  8,  9, 10
     */
    natac = (10 - natac) / 10; /* convert to positive and scale down */
    gain = loglev - (int) u.uspellprot / (4 - min(3, natac));

    if (gain > 0) {
        if (!Blind) {
            int rmtyp;
            const char *hgolden = hcolor(NH_GOLDEN), *atmosphere;

            if (u.uspellprot) {
                pline_The("%s haze around you becomes more dense.", hgolden);
            } else {
                rmtyp = levl[u.ux][u.uy].typ;
                atmosphere = u.uswallow
                                ? ((u.ustuck->data == &mons[PM_FOG_CLOUD])
                                   ? "mist"
                                   : is_whirly(u.ustuck->data)
                                      ? "maelstrom"
                                      : is_swallower(u.ustuck->data)
                                         ? "maw"
                                         : "ooze")
                                : (u.uinwater
                                   ? hliquid("water")
                                   : (rmtyp == CLOUD)
                                      ? "cloud"
                                      : IS_TREES(rmtyp)
                                         ? "vegetation"
                                         : IS_STWALL(rmtyp)
                                            ? "stone"
                                            : "air");
                pline_The("%s around you begins to shimmer with %s haze.",
                          atmosphere, an(hgolden));
            }
        }
        u.uspellprot += gain;
        u.uspmtime = (P_SKILL(spell_skilltype(SPE_PROTECTION)) == P_EXPERT)
                        ? 20 : 10;
        if (!u.usptime)
            u.usptime = u.uspmtime;
        find_ac();
    } else {
        Your("skin feels warm for a moment.");
    }
}

void
cast_reflection(mdef)
register struct monst *mdef;
{
    boolean youdefend = (mdef == &youmonst);
    int skill = (P_SKILL(spell_skilltype(SPE_REFLECTION)) == P_EXPERT
                 ? 750 : P_SKILL(spell_skilltype(SPE_REFLECTION)) == P_SKILLED
                       ? 500 : P_SKILL(spell_skilltype(SPE_REFLECTION)) == P_BASIC
                             ? 250 : 100);

    if (youdefend) {
        u.uconduct.reflection++;
        if (HReflecting) {
            if (!Blind)
                pline("The shimmering globe around you becomes slightly brighter.");
            else
                You_feel("slightly more smooth");
        } else {
            if (!Blind)
                pline("A shimmering globe appears around you!");
            else
                You_feel("smooth.");
        }
        /* the higher the skill in matter-based spells, the longer the effect */
        incr_itimeout(&HReflecting, rn1(10, HReflecting ? (skill / 5) : skill));
    } else if (!youdefend) {
        if (canseemon(mdef))
            pline("A shimmering globe appears around %s!", mon_nam(mdef));
        /* monster reflection is handled in mon_reflects() */
        mdef->mextrinsics |= MR2_REFLECTION;
        mdef->mreflecttime = rn1(10, (mdef->iswiz || is_prince(mdef->data)
                                      || mdef->data->msound == MS_NEMESIS
                                      || mdef->data->msound == MS_LEADER)
                                     ? 250 : 150);
    }
}

void
cast_barkskin(mdef)
register struct monst *mdef;
{
    boolean youdefend = (mdef == &youmonst);
    int skill = (P_SKILL(spell_skilltype(SPE_BARKSKIN)) == P_EXPERT
                 ? 750 : P_SKILL(spell_skilltype(SPE_BARKSKIN)) == P_SKILLED
                       ? 500 : P_SKILL(spell_skilltype(SPE_BARKSKIN)) == P_BASIC
                             ? 250 : 100);

    if (youdefend) {
        if (Stoneskin) {
            You_cant("cast Barkskin while Stoneskin is active.");
            return;
        } else if (HBarkskin) {
            /* increases timeout only */
            pline_The("bark covering your %s feels a bit thicker.",
                      mbodypart(&youmonst, SKIN));
        } else {
            pline("A thick layer of bark covers your %s.",
                  mbodypart(&youmonst, SKIN));
        }
        /* the higher the skill in evocation-based spells, the longer the effect */
        incr_itimeout(&HBarkskin, rn1(10, HBarkskin ? (skill / 5) : skill));
        find_ac(); /* adjust AC; dmg reduction handled in hitmu() */
    } else if (!youdefend) {
        if (canseemon(mdef))
            pline("A thick layer of bark covers %s %s!",
                  s_suffix(Monnam(mdef)), mbodypart(mdef, SKIN));
        mdef->mextrinsics |= MR2_BARKSKIN;
        mdef->mbarkskintime = rn1(10, (mdef->iswiz || is_prince(mdef->data)
                                       || mdef->data->msound == MS_NEMESIS
                                       || mdef->data->msound == MS_LEADER)
                                      ? 250 : 150);
    }
}

void
cast_stoneskin(mdef)
register struct monst *mdef;
{
    boolean youdefend = (mdef == &youmonst);
    int skill = (P_SKILL(spell_skilltype(SPE_STONESKIN)) == P_EXPERT
                 ? 750 : P_SKILL(spell_skilltype(SPE_STONESKIN)) == P_SKILLED
                       ? 500 : P_SKILL(spell_skilltype(SPE_STONESKIN)) == P_BASIC
                             ? 250 : 100);

    if (youdefend) {
        if (Barkskin) {
            You_cant("cast Stoneskin while Barkskin is active.");
            return;
        } else if (HStoneskin) {
            /* increases timeout only */
            pline_The("stone covering your %s feels a bit thicker.",
                      mbodypart(&youmonst, SKIN));
        } else {
            pline("A thick layer of stone covers your %s.",
                  mbodypart(&youmonst, SKIN));
        }
        /* become stone resistant */
        HStone_resistance |= I_SPECIAL;

        /* the higher the skill in evocation-based spells, the longer the effect */
        incr_itimeout(&HStoneskin, rn1(10, HStoneskin ? (skill / 5) : skill));
        find_ac(); /* adjust AC; dmg reduction handled in hitmu() */
    } else if (!youdefend) {
        if (canseemon(mdef))
            pline("A thick layer of stone covers %s %s!",
                  s_suffix(Monnam(mdef)), mbodypart(mdef, SKIN));
        /* already stone resistant from resists_ston() */
        mdef->mextrinsics |= MR2_STONESKIN;
        mdef->mstoneskintime = rn1(10, (mdef->iswiz || is_prince(mdef->data)
                                        || mdef->data->msound == MS_NEMESIS
                                        || mdef->data->msound == MS_LEADER)
                                       ? 250 : 150);
    }
}

void
cast_entangle(mdef)
register struct monst *mdef;
{
    boolean youdefend = (mdef == &youmonst);
    int skill = (P_SKILL(spell_skilltype(SPE_ENTANGLE)) == P_EXPERT
                 ? 5 : P_SKILL(spell_skilltype(SPE_ENTANGLE)) == P_SKILLED
                     ? 4 : P_SKILL(spell_skilltype(SPE_ENTANGLE)) == P_BASIC
                         ? 3 : 1);

    /* target needs to be standing on/near vegetation
       for entanglement to work */
    if (!(levl[mdef->mx][mdef->my].typ == GRASS
          || nexttotree(mdef->mx, mdef->my))) {
        pline("%s must be on or near vegetation to become entangled.",
              youdefend ? "You" : Monnam(mdef));
        return;
    }

    if (youdefend) {
        /* TODO: currently only the player can do this to
           themselves, so nomul() will suffice, but a more
           robust solution should be created for this */
        You("are entangled!");
        if (u.usteed) {
            newsym(u.usteed->mx, u.usteed->my);
            dismount_steed(DISMOUNT_FELL);
        }
        nomul(-3);
        return;
    } else if (!youdefend) {
        if (mdef->mentangled) {
            /* if already entangled, don't add to it */
            if (canseemon(mdef))
                pline("%s is already entangled!", Monnam(mdef));
            return;
        } else if (is_whirly(mdef->data)
                   || passes_walls(mdef->data)
                   || unsolid(mdef->data)) {
            if (canseemon(mdef))
                Your("spell fails to entangle %s.", mon_nam(mdef));
            return;
        } else {
            if (canseemon(mdef)) {
                if (nexttotree(mdef->mx, mdef->my))
                    pline_The("%s from a nearby tree ensnare %s!",
                              rn2(2) ? "branches" : "roots", mon_nam(mdef));
                else if (levl[mdef->mx][mdef->my].typ == GRASS)
                    pline_The("grass underneath %s twists and loops around %s %s!",
                              mon_nam(mdef), mhis(mdef), makeplural(mbodypart(mdef, LEG)));
                pline("%s is entangled!", Monnam(mdef));
            }
            mdef->mentangled = 1;
            /* 3-8 turns at basic, 4-9 turns at skilled,
               5-10 turns at expert. at unskilled/restricted,
               1-6 turns */
            mdef->mentangletime = rn1(6, skill);
            /* if spell skill is skilled or greater, chance to
               temporarily slow mdef */
            if (rn2(2)
                && P_SKILL(spell_skilltype(SPE_ENTANGLE)) >= P_SKILLED)
                mon_adjust_speed(mdef, -2, (struct obj *) 0);
        }
    }
}

int
cast_metal_to_wood(obj, by_you)
struct obj *obj;
boolean by_you;
{
    if (!obj)
        return 0;

    if (by_you)
        return metal_to_wood(obj, TRUE);
    else
        return metal_to_wood(obj, FALSE);
}

void
grow_grass(x, y, grasscnt)
int x, y;
genericptr_t grasscnt;
{
    struct trap *ttmp;

    /* Grow grass only on regular terrain or
       corridors, weighted towards spaces near
       the player */
    if (rn2(1 + distmin(u.ux, u.uy, x, y))
        || (levl[x][y].typ != ROOM
            && levl[x][y].typ != CORR))
        return;

    /* Never grow grass if there's an immovable
       trap here */
    ttmp = t_at(x, y);
    if (ttmp && !delfloortrap(ttmp))
        return;

    /* grow grass */
    levl[x][y].typ = GRASS;
    del_engr_at(x, y);
    newsym(x, y);
    (* (int*)grasscnt)++;
}

void
cast_create_grass()
{
    int skill = (P_SKILL(spell_skilltype(SPE_CREATE_GRASS)) == P_EXPERT
                 ? 4 : P_SKILL(spell_skilltype(SPE_CREATE_GRASS)) == P_SKILLED
                     ? 2 : P_SKILL(spell_skilltype(SPE_CREATE_GRASS)) == P_BASIC
                         ? 1 : 0);
    int range = 1 + skill;
    int madegrass = 0;

    if (Is_valley(&u.uz)) {
        pline("Grass begins to grow, but quickly withers away and dies.");
        return;
    }

    /* creates grass around the caster, including the tile
       they are standing on if suitable. at basic skill,
       grass will grow out to two tile spaces away from the
       caster, skilled is three tile spaces out, expert is
       five tile spaces. at unskilled/restricted, grass will
       only grow one tile space out */
    do_clear_area(u.ux, u.uy, range, grow_grass, &madegrass);

    if (madegrass) {
        if (Hallucination)
            pline("Whoa... so much grass, dude!");
        else
            pline("Grass grows throughout the area!");
    } else {
        /* grass didn't grow anywhere */
        pline_The("ground briefly stirs, but nothing else happens.");
    }
}

void
grow_tree(x, y, treecnt)
int x, y;
genericptr_t treecnt;
{
    struct trap *ttmp;

    /* Never grow a tree on the player's space */
    if (x == u.ux && y == u.uy)
        return;

    /* Grow a tree only on regular terrain or grass,
       never on monsters, objects, or next to doors,
       weighted towards spaces near the player.

       Never next to other living trees as well. this
       is done to limit their growth, mainly to prevent
       a situation where the caster is suddenly trapped

       Sometimes the energy of the spell can be used to
       potentially revive nearby dead trees if the
       caster is skilled enough */
    if (rn2(2) && levl[x][y].typ == DEADTREE
        && P_SKILL(spell_skilltype(SPE_CREATE_TREES)) >= P_SKILLED) {
        if (cansee(x, y))
            You("revitalize a dead tree!");
        if (!rn2(3) && Role_if(PM_DRUID))
            adjalign(1); /* your deity sometimes takes notice */
    } else if (nexttodoor(x, y) || nexttotree(x, y)
        || rn2(1 + distmin(u.ux, u.uy, x, y))
        || OBJ_AT(x, y) || MON_AT(x, y)
        || (levl[x][y].typ != ROOM
            && levl[x][y].typ != GRASS)) {
        return;
    }

    /* Never grow a tree if there's an immovable
       trap here */
    ttmp = t_at(x, y);
    if (ttmp && !delfloortrap(ttmp))
        return;

    /* grow a tree */
    levl[x][y].typ = TREE;
    del_engr_at(x, y);
    newsym(x, y);
    (* (int*)treecnt)++;
}

void
cast_create_trees()
{
    int skill = (P_SKILL(spell_skilltype(SPE_CREATE_TREES)) == P_EXPERT
                 ? 4 : P_SKILL(spell_skilltype(SPE_CREATE_TREES)) == P_SKILLED
                     ? 2 : P_SKILL(spell_skilltype(SPE_CREATE_TREES)) == P_BASIC
                         ? 1 : 0);
    int range = 1 + skill;
    int madetree = 0;

    if (Is_valley(&u.uz)) {
        pline("Saplings begin to form, but they quickly wither away and die.");
        return;
    }

    /* creates a tree around the caster. at basic skill,
       a tree will grow out to two tile spaces away from the
       caster, skilled is three tile spaces out, expert is
       five tile spaces. at unskilled/restricted, a tree will
       only grow one tile space out */
    do_clear_area(u.ux, u.uy, range, grow_tree, &madetree);

    if (madetree) {
        if (Hallucination)
            pline("Only you can prevent forest fires...");
        else
            pline("Trees grow throughout the area!");
    } else {
        /* trees didn't grow anywhere */
        pline_The("ground briefly stirs, but nothing else happens.");
    }
}

/* attempting to cast a forgotten spell will cause disorientation */
STATIC_OVL void
spell_backfire(spell)
int spell;
{
    long duration = (long) ((spellev(spell) + 1) * 3), /* 6..24 */
         old_stun = (HStun & TIMEOUT), old_conf = (HConfusion & TIMEOUT);

    /* Prior to 3.4.1, only effect was confusion; it still predominates.
     *
     * 3.6.0: this used to override pre-existing confusion duration
     * (cases 0..8) and pre-existing stun duration (cases 4..9);
     * increase them instead.   (Hero can no longer cast spells while
     * Stunned, so the potential increment to stun duration here is
     * just hypothetical.)
     */
    switch (rn2(10)) {
    case 0:
    case 1:
    case 2:
    case 3:
        make_confused(old_conf + duration, FALSE); /* 40% */
        break;
    case 4:
    case 5:
    case 6:
        make_confused(old_conf + 2L * duration / 3L, FALSE); /* 30% */
        make_stunned(old_stun + duration / 3L, FALSE);
        break;
    case 7:
    case 8:
        make_stunned(old_stun + 2L * duration / 3L, FALSE); /* 20% */
        make_confused(old_conf + duration / 3L, FALSE);
        break;
    case 9:
        make_stunned(old_stun + duration, FALSE); /* 10% */
        break;
    }
    return;
}

int
spelleffects(spell, atme, wiz_cast)
int spell;
boolean atme;
/* Set TRUE when cast from the wizard mode #wizspell command.
 * Such a cast takes no energy, is cast at the highest skill, and always succeeds.
 * In that case "spell" is the spell ID rather than the spellbook index
 * -- the equivalent of spellid(spell).
 */
boolean wiz_cast;
{
    int energy, damage, chance, n, intell;
    int otyp, skill, role_skill, res = 0;
    int attack_skill = ((P_SKILL(P_ATTACK_SPELL) >= P_EXPERT)
                        ? 3 : (P_SKILL(P_ATTACK_SPELL) == P_SKILLED) ? 2 : 1);
    boolean confused = (Confusion != 0);
    boolean physical_damage = FALSE;
    struct obj *pseudo, *otmp = (struct obj *) 0;
    struct monst *mtmp;
    coord cc;

    /*
     * Reject attempting to cast while stunned or with no free hands.
     * Already done in getspell() to stop casting before choosing
     * which spell, but duplicated here for cases where spelleffects()
     * gets called directly for ^T without intrinsic teleport capability
     * or #turn for non-priest/non-knight.
     * (There's no duplication of messages; when the rejection takes
     * place in getspell(), we don't get called.)
     */
    if ((spell < 0) || (!wiz_cast && rejectcasting())) {
        return 0; /* no time elapses */
    }

    /*
     *  Note: dotele() also calculates energy use and checks nutrition
     *  and strength requirements; it any of these change, update it too.
     */
    if (Role_if(PM_WIZARD) && spellid(spell) == SPE_FORCE_BOLT) {
        /* wizards power use for force bolt is only one point of power
           per cast at basic attack spell skill, and scales incrementally
           as attack spell skill increases */
        energy = wiz_cast ? 0 : (spellev(spell) * attack_skill);
    } else {
        energy = wiz_cast ? 0 : (spellev(spell) * 5); /* 5 <= energy <= 35 */
    }

    /*
     * Spell casting no longer affects knowledge of the spell. A
     * decrement of spell knowledge is done every turn.
     */
    if (wiz_cast) {
        ;
    } else if (spellknow(spell) <= 0) {
        if (spellid(spell) == SPE_PSIONIC_WAVE)
            You("have somehow lost your psychic ability!");
        else
            Your("knowledge of this spell is twisted.");
        pline("It invokes nightmarish images in your mind...");
        spell_backfire(spell);
        u.uen -= rnd(energy);
        if (u.uen < 0)
            u.uen = 0;
        context.botl = 1;
        return 1;
    /* Illithid that polymorphs into something other than
       another illithid-type monster doesn't have the brain
       structure to utilize psionic abilities */
    } else if (spellid(spell) == SPE_PSIONIC_WAVE
               && !racial_illithid(&youmonst)) {
        You("lack the psychic ability to use this power.");
        return 1;
    } else if (spellknow(spell) <= KEEN / 200) { /* 100 turns left */
        You("strain to recall the spell.");
    } else if (spellknow(spell) <= KEEN / 40) { /* 500 turns left */
        You("have difficulty remembering the spell.");
    } else if (spellknow(spell) <= KEEN / 20) { /* 1000 turns left */
        Your("knowledge of this spell is growing faint.");
    } else if (spellknow(spell) <= KEEN / 10) { /* 2000 turns left */
        Your("recall of this spell is gradually fading.");
    }

    if (wiz_cast) {
        ;
    } else if (u.uhunger <= 10 && spellid(spell) != SPE_DETECT_FOOD) {
        You("are too hungry to cast that spell.");
        return 0;
    } else if (ACURR(A_STR) < 4 && spellid(spell) != SPE_RESTORE_ABILITY) {
        You("lack the strength to cast spells.");
        return 0;
    } else if (check_capacity(
                "Your concentration falters while carrying so much stuff.")) {
        return 1;
    }

    /* if the cast attempt is already going to fail due to insufficient
       energy (ie, u.uen < energy), the Amulet's drain effect won't kick
       in and no turn will be consumed; however, when it does kick in,
       the attempt may fail due to lack of energy after the draining, in
       which case a turn will be used up in addition to the energy loss */
    if (u.uhave.amulet && u.uen >= energy) {
        if ((Role_if(PM_INFIDEL) && u.uachieve.amulet)
            /* even though psychic abilities use spell power,
               they are not considered a spell, and therefore
               should not be influenced by having the AoY
               in ones possession */
            || spellid(spell) == SPE_PSIONIC_WAVE)
            ; /* nothing happens */
        else
            You_feel("the amulet draining your energy away.");
        /* this used to be 'energy += rnd(2 * energy)' (without 'res'),
           so if amulet-induced cost was more than u.uen, nothing
           (except the "don't have enough energy" message) happened
           and player could just try again (and again and again...);
           now we drain some energy immediately, which has a
           side-effect of not increasing the hunger aspect of casting */

        /* In regards to Infidels: once the Amulet has been sacrificed
           to Moloch and their quest artifact is imbued with its power,
           Moloch's influence suppresses the spell power draining effect
           and allows the Infidel to realize their full potential */
        if ((Role_if(PM_INFIDEL) && u.uachieve.amulet)
            || spellid(spell) == SPE_PSIONIC_WAVE) {
            ; /* nothing happens */
        } else {
            /* hacky fix: add one here if in wizmode using #wizspell
               so rnd(0) doesn't occur */
            u.uen -= rnd((2 * energy) + (wiz_cast ? 1 : 0));
            /* now add it back */
            if (wiz_cast)
                u.uen += 1;
        }
        if (u.uen < 0)
            u.uen = 0;
        context.botl = 1;
        res = 1; /* time is going to elapse even if spell doesn't get cast */
    }

    if (energy > u.uen) {
        if (spellid(spell) == SPE_PSIONIC_WAVE)
            Your("mind is fatigued.  You cannot use your psychic energy.");
        else
            You("don't have enough energy to cast that spell.");
        return res;
    } else if (!wiz_cast) {
        if (spellid(spell) != SPE_DETECT_FOOD) {
            int hungr = energy * 2;

            /* If hero is a wizard, their current intelligence
             * (bonuses + temporary + current)
             * affects hunger reduction in casting a spell.
             * 1. int = 17-18 no reduction
             * 2. int = 16    1/4 hungr
             * 3. int = 15    1/2 hungr
             * 4. int = 1-14  normal reduction
             * The reason for this is:
             * a) Intelligence affects the amount of exertion
             * in thinking.
             * b) Wizards have spent their life at magic and
             * understand quite well how to cast spells.
             */
            intell = acurr(A_INT);
            if (!Role_if(PM_WIZARD))
                intell = 10;
            switch (intell) {
            case 25:
            case 24:
            case 23:
            case 22:
            case 21:
            case 20:
            case 19:
            case 18:
            case 17:
                hungr = 0;
                break;
            case 16:
                hungr /= 4;
                break;
            case 15:
                hungr /= 2;
                break;
            }
            /* don't put player (quite) into fainting from
             * casting a spell, particularly since they might
             * not even be hungry at the beginning; however,
             * this is low enough that they must eat before
             * casting anything else except detect food
             */
            if (hungr > u.uhunger - 3)
                hungr = u.uhunger - 3;
            morehungry(hungr);
        }
    }

    chance = wiz_cast ? 100 : percent_success(spell);
    if ((!wiz_cast && confused) || (rnd(100) > chance)) {
        if (spellid(spell) == SPE_PSIONIC_WAVE)
            You("are too confused to use your psychic abilities.");
        else
            You("fail to cast the spell correctly.");
        if (u.ualign.type == A_NONE && !u.uhave.amulet
            && !u.uachieve.amulet
            && spellid(spell) != SPE_PSIONIC_WAVE) {
            Sprintf(killer.name, "draining %s own life force", uhis());
            losehp(energy / 2, killer.name, KILLED_BY);
        } else {
            u.uen -= energy / 2;
        }
        context.botl = 1;
        return 1;
    }

    /* Infidels attempting to cast spells without having
       the Amulet of Yendor in their possession (before
       the Idol of Moloch is imbued) costs them hit points
       instead of spell power */
    if (!wiz_cast && u.ualign.type == A_NONE && !u.uhave.amulet
        && !u.uachieve.amulet
        /* psychic attack uses spell power but
           technically is not considered a spell */
        && spellid(spell) != SPE_PSIONIC_WAVE) {
        pline("You draw upon your own life force to cast the spell.");
        /* prevent healing spell abuse:
           healing can cure 6d4 worth of hit points,
           casting it drains 30 hit points.
           extra healing can cure 6d8 worth of hit points,
           but casting it drains 60 hit points. critical
           healing can cure 6d12 worth of hit points, but
           casting it drains 90 hit points. The net
           result is that our Infidel will still lose about
           the same amount of hit points as if casting
           something other than healing/extra healing */
        Sprintf(killer.name, "draining %s own life force", uhis());
        if (spellid(spell) == SPE_HEALING) {
            losehp(6 * energy, killer.name, KILLED_BY);
        } else if (spellid(spell) == SPE_EXTRA_HEALING) {
            losehp(4 * energy, killer.name, KILLED_BY);
        } else if (spellid(spell) == SPE_CRITICAL_HEALING) {
            losehp(3 * energy, killer.name, KILLED_BY);
        } else {
            losehp(energy, killer.name, KILLED_BY);
        }
    } else {
        u.uen -= energy;
    }

    /* successful casting increases the amount of time the cast
       spell is known, intelligence determines how much extra time
       is added with each cast. a player with an intelligence of
       18 will have 180-234 extra turns known added per cast; a player
       with an intelligence of only 9 would see 90-117 extra turns known
       added per cast. the players intelligence must be greater than 6
       to be able to help remember spells as they're cast. cavepersons
       are the one role that do not have this benefit. draugr race does
       not need to be considered here, as that race can't cast spells
       at all */
    if (!wiz_cast && !Role_if(PM_CAVEMAN) && ACURR(A_INT) > 6) {
        spl_book[spell].sp_know += rn1(ACURR(A_INT) * 10, ACURR(A_INT) * 3);
        if (spl_book[spell].sp_know >= KEEN)
            spl_book[spell].sp_know = KEEN;
    }

    context.botl = 1;
    exercise(A_WIS, TRUE);
    /* pseudo is a temporary "false" object containing the spell stats */
    pseudo = mksobj(wiz_cast ? spell : spellid(spell), FALSE, FALSE);
    pseudo->blessed = pseudo->cursed = 0;
    pseudo->quan = 20L; /* do not let useup get it */
    /*
     * Find the skill the hero has in a spell type category.
     * See spell_skilltype for categories.
     */
    otyp = pseudo->otyp;
    skill = spell_skilltype(otyp);
    role_skill = wiz_cast ? P_EXPERT : P_SKILL(skill);

    switch (otyp) {
    /*
     * At first spells act as expected.  As the hero increases in skill
     * with the appropriate spell type, some spells increase in their
     * effects, e.g. more damage, further distance, and so on, without
     * additional cost to the spellcaster.
     */
    case SPE_FIREBALL:
    case SPE_CONE_OF_COLD:
    case SPE_LIGHTNING:
    case SPE_POISON_BLAST:
    case SPE_ACID_BLAST:
        if (role_skill >= P_SKILLED && yn("Cast advanced spell?") == 'y') {
            if (throwspell()) {
                cc.x = u.dx;
                cc.y = u.dy;
                n = rnd(8) + 1;
                while (n--) {
                    if (!u.dx && !u.dy && !u.dz) {
                        if ((damage = zapyourself(pseudo, TRUE)) != 0) {
                            char buf[BUFSZ];
                            Sprintf(buf, "zapped %sself with a spell",
                                    uhim());
                            losehp(damage, buf, NO_KILLER_PREFIX);
                        }
                    } else {
                        explode(u.dx, u.dy,
                                ZT_SPELL(otyp - SPE_MAGIC_MISSILE),
                                spell_damage_bonus(u.ulevel / 2 + 1), 0,
                                (otyp == SPE_CONE_OF_COLD) ?
                                   EXPL_FROSTY :
                                   (otyp == SPE_LIGHTNING) ?
                                      EXPL_SHOCK :
                                      (otyp == SPE_POISON_BLAST) ?
                                         EXPL_NOXIOUS :
                                         (otyp == SPE_ACID_BLAST) ?
                                            EXPL_ACID :
                                            EXPL_FIERY);
                    }
                    u.dx = cc.x + rnd(3) - 2;
                    u.dy = cc.y + rnd(3) - 2;
                    if (!isok(u.dx, u.dy) || !cansee(u.dx, u.dy)
                        || IS_STWALL(levl[u.dx][u.dy].typ) || u.uswallow) {
                        /* Spell is reflected back to center */
                        u.dx = cc.x;
                        u.dy = cc.y;
                    }

                }
            }
            break;
        } else if (role_skill >= P_SKILLED) {
            /* player said not to cast advanced spell; return up to half of the
             * magical energy */
            if (!wiz_cast)
                u.uen += rnd(energy / 2);
        }
        /*FALLTHRU*/

    /* these spells are all duplicates of wand effects */
    case SPE_FORCE_BOLT:
        physical_damage = TRUE;
    /*FALLTHRU*/
    case SPE_SLEEP:
    case SPE_MAGIC_MISSILE:
    case SPE_KNOCK:
    case SPE_SLOW_MONSTER:
    case SPE_WIZARD_LOCK:
    case SPE_DIG:
    case SPE_TURN_UNDEAD:
    case SPE_DISPEL_EVIL:
    case SPE_POLYMORPH:
    case SPE_TELEPORT_AWAY:
    case SPE_CANCELLATION:
    case SPE_FINGER_OF_DEATH:
    case SPE_LIGHT:
    case SPE_DETECT_UNSEEN:
    case SPE_HEALING:
    case SPE_EXTRA_HEALING:
    case SPE_CRITICAL_HEALING:
    case SPE_CURE_SICKNESS:
    case SPE_RESTORE_ABILITY:
    case SPE_DRAIN_LIFE:
    case SPE_STONE_TO_FLESH:
    case SPE_PSIONIC_WAVE:
    case SPE_ENTANGLE:
    case SPE_CHANGE_METAL_TO_WOOD:
        if (objects[otyp].oc_dir != NODIR) {
            if (otyp == SPE_HEALING || otyp == SPE_EXTRA_HEALING
                || otyp == SPE_CRITICAL_HEALING || otyp == SPE_RESTORE_ABILITY) {
                /* damage healing spells/restore ability are actually potion
                   effects, but they've been extended to take a direction
                   like wands */
                if (role_skill >= P_SKILLED)
                    pseudo->blessed = 1;
            }
            if (atme) {
                u.dx = u.dy = u.dz = 0;
            } else if (!getdir((char *) 0)) {
                /* getdir cancelled, re-use previous direction */
                /*
                 * FIXME:  reusing previous direction only makes sense
                 * if there is an actual previous direction.  When there
                 * isn't one, the spell gets cast at self which is rarely
                 * what the player intended.  Unfortunately, the way
                 * spelleffects() is organized means that aborting with
                 * "nevermind" is not an option.
                 */
                if (otyp == SPE_PSIONIC_WAVE)
                    pline_The("psionic energy is released!");
                else
                    pline_The("magical energy is released!");
            }
            if (!u.dx && !u.dy && !u.dz) {
                if ((damage = zapyourself(pseudo, TRUE)) != 0) {
                    char buf[BUFSZ];

                    Sprintf(buf, "zapped %sself with a spell", uhim());
                    if (physical_damage)
                        damage = Maybe_Half_Phys(damage);
                    losehp(damage, buf, NO_KILLER_PREFIX);
                }
            } else
                weffects(pseudo);
        } else
            weffects(pseudo);
        update_inventory(); /* spell may modify inventory */
        break;

    /* these are all duplicates of scroll effects */
    case SPE_REMOVE_CURSE:
    case SPE_CONFUSE_MONSTER:
    case SPE_BURNING_HANDS:
    case SPE_SHOCKING_GRASP:
    case SPE_DETECT_FOOD:
    case SPE_CAUSE_FEAR:
    case SPE_IDENTIFY:
    case SPE_CHARM_MONSTER:
        /* high skill yields effect equivalent to blessed scroll */
        if (role_skill >= P_SKILLED)
            pseudo->blessed = 1;
    /*FALLTHRU*/
    case SPE_MAGIC_MAPPING:
    case SPE_CREATE_MONSTER:
        (void) seffects(pseudo);
        break;

    /* these are all duplicates of potion effects */
    case SPE_HASTE_SELF:
    case SPE_DETECT_TREASURE:
    case SPE_DETECT_MONSTERS:
    case SPE_LEVITATION:
        /* high skill yields effect equivalent to blessed potion */
        if (role_skill >= P_SKILLED)
            pseudo->blessed = 1;
    /*FALLTHRU*/
    case SPE_INVISIBILITY:
        (void) peffects(pseudo);
        break;
    /* end of potion-like spells */

    case SPE_CURE_BLINDNESS:
        healup(0, 0, FALSE, TRUE);
        break;
    case SPE_CREATE_FAMILIAR:
        (void) make_familiar((struct obj *) 0, u.ux, u.uy, FALSE, TRUE);
        break;
    case SPE_CLAIRVOYANCE:
        if (!BClairvoyant) {
            if (role_skill >= P_SKILLED)
                pseudo->blessed = 1; /* detect monsters as well as map */
            do_vicinity_map(pseudo);
        /* at present, only one thing blocks clairvoyance */
        } else if (uarmh && uarmh->otyp == CORNUTHAUM)
            You("sense a pointy hat on top of your %s.", body_part(HEAD));
        break;
    case SPE_PROTECTION:
        cast_protection();
        break;
    case SPE_JUMPING:
        if (!jump(max(role_skill, 1)))
            pline1(nothing_happens);
        break;
    case SPE_REPAIR_ARMOR:
        /* removes one level of erosion (both types) for a chosen piece of armor */
        if (role_skill >= P_BASIC) {
            if (u.usteed
                && (otmp = which_armor(u.usteed, W_BARDING)) != 0) {
                char buf[BUFSZ];
                Sprintf(buf, "Repair %s %s?", s_suffix(y_monnam(u.usteed)),
                        xname(otmp));
                if (yn(buf) == 'n') {
                    otmp = (struct obj *) 0;
                }
            }
            if (!otmp) {
                otmp = getobj(clothes, "magically repair");
                while (otmp && !(otmp->owornmask & W_ARMOR)) {
                    pline("You cannot repair armor that is not worn.");
                    otmp = getobj(clothes, "magically repair");
                }
            }
        } else {
            otmp = some_armor(&youmonst);
            while (otmp && otmp->oartifact == ART_HAND_OF_VECNA) {
                otmp = some_armor(&youmonst);
            }
        }
        if (otmp) {
            if (greatest_erosion(otmp) > 0) {
                if (Blind)
                    Your("%s feels warmer for a brief moment.",
                         xname(otmp));
                else
                    Your("%s glows faintly golden for a moment.",
                         xname(otmp));
                if (otmp->oeroded > 0)
                    otmp->oeroded--;
                if (otmp->oeroded2 > 0)
                    otmp->oeroded2--;
                update_inventory();
            } else {
                if (Blind)
                    Your("%s feels lukewarm for a brief moment.",
                         xname(otmp));
                else
                    Your("%s glows briefly, but looks as new as ever.",
                         xname(otmp));
            }
        } else {
            /* the player can probably feel this, so no need for a !Blind check :) */
            Your("embarrassing skin rash clears up slightly.");
        }
        break;
    case SPE_REFLECTION:
        cast_reflection(&youmonst);
        break;
    case SPE_BARKSKIN:
        cast_barkskin(&youmonst);
        break;
    case SPE_STONESKIN:
        cast_stoneskin(&youmonst);
        break;
    case SPE_CREATE_GRASS:
        cast_create_grass();
        break;
    case SPE_CREATE_TREES:
        cast_create_trees();
        break;
    case SPE_ORB_OF_FIRE:
    case SPE_ORB_OF_FROST:
        You("conjure elemental energy...");
        for (n = 0; n < max(role_skill - 1, 1); n++) {
            mtmp = make_helper((pseudo->otyp == SPE_ORB_OF_FIRE) ?
                               PM_ORB_OF_FIRE : PM_ORB_OF_FROST, u.ux, u.uy);
            if (!mtmp) {
                pline("But it quickly fades away.");
                break;
            } else {
                mtmp->mtame = 10;
                mtmp->mhpmax = mtmp->mhp = 1;
                if (role_skill >= P_SKILLED)
                    mtmp->msummoned = rnd(100) + 100;
                else
                    mtmp->msummoned = rnd(50) + 50;
                mtmp->uexp = 1;
            }
        }
        break;
    case SPE_SUMMON_ANIMAL:
        (void) make_woodland_animal(u.ux, u.uy);
        break;
    case SPE_SUMMON_ELEMENTAL:
        (void) make_elemental(u.ux, u.uy);
        break;
    case SPE_POWER_WORD_KILL: {
        char c, qbuf[QBUFSZ];
        const char *prompt = "choose a monster to slay";
        int ans;
        int range_skill = (P_SKILL(spell_skilltype(SPE_POWER_WORD_KILL)) == P_EXPERT
                           ? 17 : P_SKILL(spell_skilltype(SPE_POWER_WORD_KILL)) == P_SKILLED
                                ? 7 : P_SKILL(spell_skilltype(SPE_POWER_WORD_KILL)) == P_BASIC
                                    ? 2 : 0);
        boolean save_verbose = flags.verbose,
                save_autodescribe = iflags.autodescribe;
        d_level uarehere = u.uz;

        /* range of power word kill spell (@ is center):
           U = unskilled, B = basic, S = skilled, E = expert
           ...........
           ....EEE....
           ..EEEEEEE..
           ..ESSSSSE..
           .EESBUBSEE.
           .EESU@USEE.
           .EESBUBSEE.
           ..ESSSSSE..
           ..EEEEEEE..
           ....EEE....
           ...........
        */

        cc.x = u.ux, cc.y = u.uy;
        for (;;) {
            You("utter a word of power...");
            pline("%s:", prompt);

            flags.verbose = FALSE;
            iflags.autodescribe = TRUE;
            ans = getpos(&cc, TRUE, "a monster");
            flags.verbose = save_verbose;
            iflags.autodescribe = save_autodescribe;
            if (ans < 0 || cc.x < 1)
                break;

            /* handle selecting your own location first */
            mtmp = 0;
            if (cc.x == u.ux && cc.y == u.uy) {
                if (u.usteed) {
                    Sprintf(qbuf, "Kill %.110s?", mon_nam(u.usteed));
                    if ((c = ynq(qbuf)) == 'q')
                        break;
                    if (c == 'y')
                        mtmp = u.usteed;
                }
                if (!mtmp) {
                    Sprintf(qbuf, "%s?",
                            Role_if(PM_SAMURAI) ? "Perform seppuku"
                                                : "Commit suicide");
                    if (paranoid_query(TRUE, qbuf)) {
                        Sprintf(killer.name, "%s own word of power", uhis());
                        killer.format = KILLED_BY;
                        done(DIED);
                    }
                    break;
                }
            } else if (u.uswallow) {
                mtmp = (distu(cc.x, cc.y) <= 2) ? u.ustuck : 0;
            } else {
                mtmp = m_at(cc.x, cc.y);
            }

            /* whether there's an unseen monster here or not, player will know
               that there's no monster here after the kill or failed attempt;
               let hero know too */
            (void) unmap_invisible(cc.x, cc.y);

            if (mtmp) {
                if (distu(mtmp->mx, mtmp->my) > range_skill + 1) {
                    pline("%s is out of range.", Monnam(mtmp));
                    break;
                }
                if (distu(mtmp->mx, mtmp->my) <= range_skill + 1) {
                    if (immune_death_magic(mtmp->data)) {
                        shieldeff(mtmp->mx, mtmp->my);
                        pline("%s is immune to your word of power!",
                              Monnam(mtmp));
                        break;
                    } else if (mtmp->mhp <= 100) {
                        /* assumes not resistant to death magic.
                           monsters with 100 or less hit points - dead if
                           they don't have magic resistance. If monster
                           does have MR, they still take damage not to go
                           below one hit point */
                        if (resists_magm(mtmp) || defended(mtmp, AD_MAGM)) {
                            shieldeff(mtmp->mx, mtmp->my);
                            pline("%s %s in pain, but resists your deadly spell.",
                                  Monnam(mtmp), makeplural(growl_sound(mtmp)));
                            mtmp->mhp -= d(8, 6);
                            if (mtmp->mhp < 1)
                                mtmp->mhp = 1;
                            if (mtmp->mpeaceful || mtmp->mtame) {
                                mtmp->mpeaceful = mtmp->mtame = 0;
                                newsym(mtmp->mx, mtmp->my);
                                if (u.ualign.type != A_NONE) {
                                    You_feel("distraught.");
                                    adjalign(-3);
                                }
                            }
                            break;
                        } else {
                            You("%s %s!",
                                rn2(2) ? "annihilate" : "obliterate",
                                mon_nam(mtmp));
                            mtmp->mhp = 0;
                            monkilled(mtmp, (char *) 0, AD_DETH);
                            if (!DEADMONSTER(mtmp)) /* lifesaved */
                                return 0;
                            break;
                        }
                    } else { /* greater than 100 hit points */
                        if (resists_magm(mtmp) || defended(mtmp, AD_MAGM)) {
                            shieldeff(mtmp->mx, mtmp->my);
                            pline("%s %s in pain, but resists your deadly spell.",
                                  Monnam(mtmp), makeplural(growl_sound(mtmp)));
                            mtmp->mhp -= d(8, 6);
                            if (mtmp->mhp < 1)
                                mtmp->mhp = 1;
                            if (mtmp->mpeaceful || mtmp->mtame) {
                                mtmp->mpeaceful = mtmp->mtame = 0;
                                newsym(mtmp->mx, mtmp->my);
                                if (u.ualign.type != A_NONE) {
                                    You_feel("distraught.");
                                    adjalign(-3);
                                }
                            }
                            break;
                        } else {
                            /* if not magic resistant, but has over 100 hit
                               points, max hit points and regular hit points
                               take a significant hit, not to go below one
                               hit point */
                            if (!Deaf)
                                pline("%s %s in %s!", Monnam(mtmp),
                                      makeplural(growl_sound(mtmp)),
                                      rn2(2) ? "agony" : "pain");
                            else if (cansee(mtmp->mx, mtmp->my))
                                pline("%s trembles in %s!", Monnam(mtmp),
                                      rn2(2) ? "agony" : "pain");
                            /* mhp will then still be less than this value */
                            mtmp->mhpmax -= rn1(9, 4);
                            if (mtmp->mhpmax < 1) /* protect against invalid value */
                                mtmp->mhpmax = 1;
                            mtmp->mhp /= 3;
                            if (mtmp->mhp < 1)
                                mtmp->mhp = 1;
                            if (mtmp->mpeaceful || mtmp->mtame) {
                                mtmp->mpeaceful = mtmp->mtame = 0;
                                newsym(mtmp->mx, mtmp->my);
                                if (u.ualign.type != A_NONE) {
                                    You_feel("distraught.");
                                    adjalign(-3);
                                }
                            }
                            break;
                        }
                    }
                }
                /* end targetting loop if an engulfer dropped hero onto a level-
                   changing trap */
                if (u.utotype || !on_level(&u.uz, &uarehere))
                    break;
            } else {
                There("is no monster there.");
                break;
            }
        }
        break;
    }
    default:
        impossible("Unknown spell %d attempted.", spell);
        obfree(pseudo, (struct obj *) 0);
        return 0;
    }

    /* gain skill for successful cast */
    if (!wiz_cast)
        use_skill(skill, spellev(spell));

    obfree(pseudo, (struct obj *) 0); /* now, get rid of it */
    return 1;
}

/*ARGSUSED*/
STATIC_OVL boolean
spell_aim_step(arg, x, y)
genericptr_t arg UNUSED;
int x, y;
{
    if (!isok(x,y))
        return FALSE;
    if (!ZAP_POS(levl[x][y].typ)
        && !(IS_DOOR(levl[x][y].typ) && (levl[x][y].doormask & D_ISOPEN)))
        return FALSE;
    return TRUE;
}

/* Choose location where spell takes effect. */
STATIC_OVL int
throwspell()
{
    coord cc, uc;
    struct monst *mtmp;

    if (u.uinwater) {
        pline("You're joking!  In this weather?");
        return 0;
    }

    pline("Where do you want to cast the spell?");
    cc.x = u.ux;
    cc.y = u.uy;
    if (getpos(&cc, TRUE, "the desired position") < 0)
        return 0; /* user pressed ESC */
    /* The number of moves from hero to where the spell drops.*/
    if (distmin(u.ux, u.uy, cc.x, cc.y) > 10) {
        pline_The("spell dissipates over the distance!");
        return 0;
    } else if (u.uswallow) {
        pline_The("spell is cut short!");
        exercise(A_WIS, FALSE); /* What were you THINKING! */
        u.dx = 0;
        u.dy = 0;
        return 1;
    } else if ((!cansee(cc.x, cc.y)
                && (!(mtmp = m_at(cc.x, cc.y)) || !canspotmon(mtmp))
                && !((u.ux == cc.x) && (u.uy == cc.y)))
               || IS_STWALL(levl[cc.x][cc.y].typ)) {
        Your("mind fails to lock onto that location!");
        return 0;
    }

    uc.x = u.ux;
    uc.y = u.uy;

    walk_path(&uc, &cc, spell_aim_step, (genericptr_t) 0);

    u.dx = cc.x;
    u.dy = cc.y;
    return 1;
}

/* add/hide/remove/unhide teleport-away on behalf of dotelecmd() to give
   more control to behavior of ^T when used in wizard mode */
int
tport_spell(what)
int what;
{
    static struct tport_hideaway {
        struct spell savespell;
        int tport_indx;
    } save_tport;
    int i;
/* also defined in teleport.c */
#define NOOP_SPELL  0
#define HIDE_SPELL  1
#define ADD_SPELL   2
#define UNHIDESPELL 3
#define REMOVESPELL 4

    for (i = 0; i < MAXSPELL; i++)
        if (spellid(i) == SPE_TELEPORT_AWAY || spellid(i) == NO_SPELL)
            break;
    if (i == MAXSPELL) {
        impossible("tport_spell: spellbook full");
        /* wizard mode ^T is not able to honor player's menu choice */
    } else if (spellid(i) == NO_SPELL) {
        if (what == HIDE_SPELL || what == REMOVESPELL) {
            save_tport.tport_indx = MAXSPELL;
        } else if (what == UNHIDESPELL) {
            /*assert( save_tport.savespell.sp_id == SPE_TELEPORT_AWAY );*/
            spl_book[save_tport.tport_indx] = save_tport.savespell;
            save_tport.tport_indx = MAXSPELL; /* burn bridge... */
        } else if (what == ADD_SPELL) {
            save_tport.savespell = spl_book[i];
            save_tport.tport_indx = i;
            spl_book[i].sp_id = SPE_TELEPORT_AWAY;
            spl_book[i].sp_lev = objects[SPE_TELEPORT_AWAY].oc_level;
            spl_book[i].sp_know = KEEN;
            return REMOVESPELL; /* operation needed to reverse */
        }
    } else { /* spellid(i) == SPE_TELEPORT_AWAY */
        if (what == ADD_SPELL || what == UNHIDESPELL) {
            save_tport.tport_indx = MAXSPELL;
        } else if (what == REMOVESPELL) {
            /*assert( i == save_tport.tport_indx );*/
            spl_book[i] = save_tport.savespell;
            save_tport.tport_indx = MAXSPELL;
        } else if (what == HIDE_SPELL) {
            save_tport.savespell = spl_book[i];
            save_tport.tport_indx = i;
            spl_book[i].sp_id = NO_SPELL;
            return UNHIDESPELL; /* operation needed to reverse */
        }
    }
    return NOOP_SPELL;
}

/* forget a random selection of known spells due to amnesia;
   they used to be lost entirely, as if never learned, but now we
   just set the memory retention to zero so that they can't be cast */
void
losespells()
{
    int n, nzap, i;

    /* in case reading has been interrupted earlier, discard context */
    context.spbook.book = 0;
    context.spbook.o_id = 0;
    /* count the number of known spells */
    for (n = 0; n < MAXSPELL; ++n)
        if (spellid(n) == NO_SPELL)
            break;

    /* lose anywhere from zero to all known spells;
       if confused, use the worse of two die rolls */
    nzap = rn2(n + 1);
    if (Confusion) {
        i = rn2(n + 1);
        if (i > nzap)
            nzap = i;
    }
    /* good Luck might ameliorate spell loss */
    if (nzap > 1 && !rnl(7))
        nzap = rnd(nzap);

    /*
     * Forget 'nzap' out of 'n' known spells by setting their memory
     * retention to zero.  Every spell has the same probability to be
     * forgotten, even if its retention is already zero.
     *
     * Perhaps we should forget the corresponding book too?
     *
     * (3.4.3 removed spells entirely from the list, but always did
     * so from its end, so the 'nzap' most recently learned spells
     * were the ones lost by default.  Player had sort control over
     * the list, so could move the most useful spells to front and
     * only lose them if 'nzap' turned out to be a large value.
     *
     * Discarding from the end of the list had the virtue of making
     * casting letters for lost spells become invalid and retaining
     * the original letter for the ones which weren't lost, so there
     * was no risk to the player of accidentally casting the wrong
     * spell when using a letter that was in use prior to amnesia.
     * That wouldn't be the case if we implemented spell loss spread
     * throughout the list of known spells; every spell located past
     * the first lost spell would end up with new letter assigned.)
     */
    for (i = 0; nzap > 0; ++i) {
        /* when nzap is small relative to the number of spells left,
           the chance to lose spell [i] is small; as the number of
           remaining candidates shrinks, the chance per candidate
           gets bigger; overall, exactly nzap entries are affected */
        if (rn2(n - i) < nzap) {
            /* lose access to spell [i] */
            spellknow(i) = 0;
#if 0
            /* also forget its book */
            forget_single_object(spellid(i));
#endif
            /* and abuse wisdom */
            exercise(A_WIS, FALSE);
            /* there's now one less spell slated to be forgotten */
            --nzap;
        }
    }
}

/*
 * Allow player to sort the list of known spells.  Manually swapping
 * pairs of them becomes very tedious once the list reaches two pages.
 *
 * Possible extensions:
 *      provide means for player to control ordering of skill classes;
 *      provide means to supply value N such that first N entries stick
 *      while rest of list is being sorted;
 *      make chosen sort order be persistent such that when new spells
 *      are learned, they get inserted into sorted order rather than be
 *      appended to the end of the list?
 */
enum spl_sort_types {
    SORTBY_LETTER = 0,
    SORTBY_ALPHA,
    SORTBY_LVL_LO,
    SORTBY_LVL_HI,
    SORTBY_SKL_AL,
    SORTBY_SKL_LO,
    SORTBY_SKL_HI,
    SORTBY_CURRENT,
    SORTRETAINORDER,

    NUM_SPELL_SORTBY
};

static const char *spl_sortchoices[NUM_SPELL_SORTBY] = {
    "by casting letter",
    "alphabetically",
    "by level, low to high",
    "by level, high to low",
    "by skill group, alphabetized within each group",
    "by skill group, low to high level within group",
    "by skill group, high to low level within group",
    "maintain current ordering",
    /* a menu choice rather than a sort choice */
    "reassign casting letters to retain current order",
};
static int spl_sortmode = 0;   /* index into spl_sortchoices[] */
static int *spl_orderindx = 0; /* array of spl_book[] indices */

/* qsort callback routine */
STATIC_PTR int CFDECLSPEC
spell_cmp(vptr1, vptr2)
const genericptr vptr1;
const genericptr vptr2;
{
    /*
     * gather up all of the possible parameters except spell name
     * in advance, even though some might not be needed:
     *  indx. = spl_orderindx[] index into spl_book[];
     *  otyp. = spl_book[] index into objects[];
     *  levl. = spell level;
     *  skil. = skill group aka spell class.
     */
    int indx1 = *(int *) vptr1, indx2 = *(int *) vptr2,
        otyp1 = spl_book[indx1].sp_id, otyp2 = spl_book[indx2].sp_id,
        levl1 = objects[otyp1].oc_level, levl2 = objects[otyp2].oc_level,
        skil1 = objects[otyp1].oc_skill, skil2 = objects[otyp2].oc_skill;

    switch (spl_sortmode) {
    case SORTBY_LETTER:
        return indx1 - indx2;
    case SORTBY_ALPHA:
        break;
    case SORTBY_LVL_LO:
        if (levl1 != levl2)
            return levl1 - levl2;
        break;
    case SORTBY_LVL_HI:
        if (levl1 != levl2)
            return levl2 - levl1;
        break;
    case SORTBY_SKL_AL:
        if (skil1 != skil2)
            return skil1 - skil2;
        break;
    case SORTBY_SKL_LO:
        if (skil1 != skil2)
            return skil1 - skil2;
        if (levl1 != levl2)
            return levl1 - levl2;
        break;
    case SORTBY_SKL_HI:
        if (skil1 != skil2)
            return skil1 - skil2;
        if (levl1 != levl2)
            return levl2 - levl1;
        break;
    case SORTBY_CURRENT:
    default:
        return (vptr1 < vptr2) ? -1
                               : (vptr1 > vptr2); /* keep current order */
    }
    /* tie-breaker for most sorts--alphabetical by spell name */
    return strcmpi(OBJ_NAME(objects[otyp1]), OBJ_NAME(objects[otyp2]));
}

/* sort the index used for display order of the "view known spells"
   list (sortmode == SORTBY_xxx), or sort the spellbook itself to make
   the current display order stick (sortmode == SORTRETAINORDER) */
STATIC_OVL void
sortspells()
{
    int i;
#if defined(SYSV) || defined(DGUX)
    unsigned n;
#else
    int n;
#endif

    if (spl_sortmode == SORTBY_CURRENT)
        return;
    for (n = 0; n < MAXSPELL && spellid(n) != NO_SPELL; ++n)
        continue;
    if (n < 2)
        return; /* not enough entries to need sorting */

    if (!spl_orderindx) {
        /* we haven't done any sorting yet; list is in casting order */
        if (spl_sortmode == SORTBY_LETTER /* default */
            || spl_sortmode == SORTRETAINORDER)
            return;
        /* allocate enough for full spellbook rather than just N spells */
        spl_orderindx = (int *) alloc(MAXSPELL * sizeof(int));
        for (i = 0; i < MAXSPELL; i++)
            spl_orderindx[i] = i;
    }

    if (spl_sortmode == SORTRETAINORDER) {
        struct spell tmp_book[MAXSPELL];

        /* sort spl_book[] rather than spl_orderindx[];
           this also updates the index to reflect the new ordering (we
           could just free it since that ordering becomes the default) */
        for (i = 0; i < MAXSPELL; i++)
            tmp_book[i] = spl_book[spl_orderindx[i]];
        for (i = 0; i < MAXSPELL; i++)
            spl_book[i] = tmp_book[i], spl_orderindx[i] = i;
        spl_sortmode = SORTBY_LETTER; /* reset */
        return;
    }

    /* usual case, sort the index rather than the spells themselves */
    qsort((genericptr_t) spl_orderindx, n, sizeof *spl_orderindx, spell_cmp);
    return;
}

/* called if the [sort spells] entry in the view spells menu gets chosen */
STATIC_OVL boolean
spellsortmenu()
{
    winid tmpwin;
    menu_item *selected;
    anything any;
    char let;
    int i, n, choice;

    tmpwin = create_nhwindow(NHW_MENU);
    start_menu(tmpwin);
    any = zeroany; /* zero out all bits */

    for (i = 0; i < SIZE(spl_sortchoices); i++) {
        if (i == SORTRETAINORDER) {
            let = 'z'; /* assumes fewer than 26 sort choices... */
            /* separate final choice from others with a blank line */
            any.a_int = 0;
            add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, "",
                     MENU_UNSELECTED);
        } else {
            let = 'a' + i;
        }
        any.a_int = i + 1;
        add_menu(tmpwin, NO_GLYPH, &any, let, 0, ATR_NONE, spl_sortchoices[i],
                 (i == spl_sortmode) ? MENU_SELECTED : MENU_UNSELECTED);
    }
    end_menu(tmpwin, "View known spells list sorted");

    n = select_menu(tmpwin, PICK_ONE, &selected);
    destroy_nhwindow(tmpwin);
    if (n > 0) {
        choice = selected[0].item.a_int - 1;
        /* skip preselected entry if we have more than one item chosen */
        if (n > 1 && choice == spl_sortmode)
            choice = selected[1].item.a_int - 1;
        free((genericptr_t) selected);
        spl_sortmode = choice;
        return TRUE;
    }
    return FALSE;
}

/* the '+' command -- view known spells */
int
dovspell()
{
    char qbuf[QBUFSZ];
    int splnum, othnum;
    struct spell spl_tmp;

    if (spellid(0) == NO_SPELL) {
        You("don't know any spells right now.");
    } else {
        while (dospellmenu("Currently known spells",
                           SPELLMENU_VIEW, &splnum)) {
            if (splnum == SPELLMENU_SORT) {
                if (spellsortmenu())
                    sortspells();
            } else {
                Sprintf(qbuf, "Reordering spells; swap '%c' with",
                        spellet(splnum));
                if (!dospellmenu(qbuf, splnum, &othnum))
                    break;

                spl_tmp = spl_book[splnum];
                spl_book[splnum] = spl_book[othnum];
                spl_book[othnum] = spl_tmp;
            }
        }
    }
    if (spl_orderindx) {
        free((genericptr_t) spl_orderindx);
        spl_orderindx = 0;
    }
    spl_sortmode = SORTBY_LETTER; /* 0 */
    return 0;
}

STATIC_OVL boolean
dospellmenu(prompt, splaction, spell_no)
const char *prompt;
int splaction; /* SPELLMENU_CAST, SPELLMENU_VIEW, or spl_book[] index */
int *spell_no;
{
    winid tmpwin;
    int i, n, how, splnum;
    char buf[BUFSZ], retentionbuf[24];
    const char *fmt;
    menu_item *selected;
    anything any;

    tmpwin = create_nhwindow(NHW_MENU);
    start_menu(tmpwin);
    any = zeroany; /* zero out all bits */

    /*
     * The correct spacing of the columns when not using
     * tab separation depends on the following:
     * (1) that the font is monospaced, and
     * (2) that selection letters are pre-pended to the
     * given string and are of the form "a - ".
     */
    if (!iflags.menu_tab_sep) {
        Sprintf(buf, "%-20s     Level %-12s Fail Retention", "    Name",
                "Category");
        fmt = "%-20s  %2d   %-12s %3d%% %9s";
    } else {
        Sprintf(buf, "Name\tLevel\tCategory\tFail\tRetention");
        fmt = "%s\t%-d\t%s\t%-d%%\t%s";
    }
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, iflags.menu_headings, buf,
             MENU_UNSELECTED);
    for (i = 0; i < MAXSPELL && spellid(i) != NO_SPELL; i++) {
        splnum = !spl_orderindx ? i : spl_orderindx[i];
        Sprintf(buf, fmt, spellname(splnum), spellev(splnum),
                spelltypemnemonic(spell_skilltype(spellid(splnum))),
                100 - percent_success(splnum),
                spellretention(splnum, retentionbuf));

        any.a_int = splnum + 1; /* must be non-zero */
        add_menu(tmpwin, NO_GLYPH, &any, spellet(splnum), 0, ATR_NONE, buf,
                 (splnum == splaction) ? MENU_SELECTED : MENU_UNSELECTED);
    }
    how = PICK_ONE;
    if (splaction == SPELLMENU_VIEW) {
        if (spellid(1) == NO_SPELL) {
            /* only one spell => nothing to swap with */
            how = PICK_NONE;
        } else {
            /* more than 1 spell, add an extra menu entry */
            any.a_int = SPELLMENU_SORT + 1;
            add_menu(tmpwin, NO_GLYPH, &any, '+', 0, ATR_NONE,
                     "[sort spells]", MENU_UNSELECTED);
        }
    }
    end_menu(tmpwin, prompt);

    n = select_menu(tmpwin, how, &selected);
    destroy_nhwindow(tmpwin);
    if (n > 0) {
        *spell_no = selected[0].item.a_int - 1;
        /* menu selection for `PICK_ONE' does not
           de-select any preselected entry */
        if (n > 1 && *spell_no == splaction)
            *spell_no = selected[1].item.a_int - 1;
        free((genericptr_t) selected);
        /* default selection of preselected spell means that
           user chose not to swap it with anything */
        if (*spell_no == splaction)
            return FALSE;
        return TRUE;
    } else if (splaction >= 0) {
        /* explicit de-selection of preselected spell means that
           user is still swapping but not for the current spell */
        *spell_no = splaction;
        return TRUE;
    }
    return FALSE;
}

STATIC_OVL int
percent_success(spell)
int spell;
{
    /* Intrinsic and learned ability are combined to calculate
     * the probability of player's success at cast a given spell.
     */
    int chance, splcaster, special, statused;
    int difficulty;
    int skill, skilltype = spell_skilltype(spellid(spell));
    int dex_adjust;
    boolean paladin_bonus, primary_casters, non_casters;
    boolean druid_bracers = (uarms
                             && uarms->oartifact == ART_BRACERS_OF_THE_FIRST_CIRCL);

    /* Calculate intrinsic ability (splcaster) */

    splcaster = urole.spelbase;
    special = urole.spelheal;
    statused = ACURR(urole.spelstat);

    /* Your dexterity can offset
     * (for better or for worse) your spellcasting penalty
     */
    dex_adjust = 0;
    if (ACURR(A_DEX) <= 6)
        dex_adjust -= 10;
    else if (ACURR(A_DEX) <= 9)
        dex_adjust -= 5;
    /* Range of 10 to 15 - no dexterity adjustment */
    else if (ACURR(A_DEX) <= 15)
        dex_adjust += 0;
    /* At 18 and above we start to see some benefit */
    else if (ACURR(A_DEX) <= 18)
        dex_adjust += 5;
    else if (ACURR(A_DEX) <= 20)
        dex_adjust += 10;
    else if (ACURR(A_DEX) <= 23)
        dex_adjust += 15;
    else if (ACURR(A_DEX) >= 24)
        dex_adjust += 20;

    /* Knights don't get metal armor penalty for clerical spells */
    paladin_bonus = (Role_if(PM_KNIGHT) && skilltype == P_CLERIC_SPELL);

    /* Casting roles */
    primary_casters = (Role_if(PM_HEALER) || Role_if(PM_PRIEST)
                       || Role_if(PM_WIZARD) || Role_if(PM_INFIDEL) || Role_if(PM_DRUID));

    non_casters = (Role_if(PM_ARCHEOLOGIST) || Role_if(PM_BARBARIAN) || Role_if(PM_CAVEMAN)
                   || Role_if(PM_CONVICT) || Role_if(PM_KNIGHT) || Role_if(PM_MONK)
                   || Role_if(PM_RANGER) || Role_if(PM_ROGUE) || Role_if(PM_SAMURAI)
                   || Role_if(PM_TOURIST) || Role_if(PM_VALKYRIE));

    if (uarm && is_metallic(uarm) && !paladin_bonus)
        splcaster += (uarmc && uarmc->otyp == ROBE) ? urole.spelarmr / 2
                                                    : urole.spelarmr;
    else if (uarmc && uarmc->otyp == ROBE)
        splcaster -= urole.spelarmr;
    if (uarms && !druid_bracers)
        splcaster += urole.spelshld;

    if (!paladin_bonus) {
        if (uarmh && is_metallic(uarmh))
            splcaster += uarmhbon;
        if (uarmg && is_metallic(uarmg)
            && uarmg->oartifact != ART_GAUNTLETS_OF_PURITY)
            splcaster += uarmgbon;
        if (uarmf && is_metallic(uarmf))
            splcaster += uarmfbon;
    }

    /* Infidels have a special penalty for wearing blessed armor. */
    if (Role_if(PM_INFIDEL)) {
        static struct obj **const armor[] = { &uarm, &uarmc, &uarmh, &uarms,
                                              &uarmg, &uarmf, &uarmu };
        int penalty = 0;
        int i;

        for (i = 0; i < SIZE(armor); i++) {
            if (!*armor[i])
                continue;
            if ((*armor[i])->blessed)
                penalty += 2;
            else if ((*armor[i])->cursed)
                penalty--;
        }
        if (penalty > 0)
            splcaster += penalty;
    }

    if (spellid(spell) == urole.spelspec)
        splcaster += urole.spelsbon;

    /* `healing spell' bonus */
    if (spellid(spell) == SPE_HEALING
        || spellid(spell) == SPE_EXTRA_HEALING
        || spellid(spell) == SPE_CRITICAL_HEALING
        || spellid(spell) == SPE_CURE_BLINDNESS
        || spellid(spell) == SPE_CURE_SICKNESS
        || spellid(spell) == SPE_RESTORE_ABILITY
        || spellid(spell) == SPE_REMOVE_CURSE)
        splcaster += special;

    if (splcaster > 20)
        splcaster = 20;

    /* Calculate learned ability */

    /* Players basic likelihood of being able to cast any spell
     * is based of their `magic' statistic. (Int or Wis)
     */
    chance = 11 * statused / 2;

    /*
     * High level spells are harder.  Easier for higher level casters.
     * The difficulty is based on the hero's level and their skill level
     * in that spell type.
     */
    skill = P_SKILL(skilltype);
    skill = max(skill, P_UNSKILLED) - 1; /* unskilled => 0 */
    difficulty =
        (spellev(spell) - 1) * 4 - ((skill * 6) + (u.ulevel / 3) + 1);

    if (difficulty > 0) {
        /* Player is too low level or unskilled. */
        chance -= isqrt(900 * difficulty + 2000);
    } else {
        /* Player is above level.  Learning continues, but the
         * law of diminishing returns sets in quickly for
         * low-level spells.  That is, a player quickly gains
         * no advantage for raising level.
         */
        int learning = 15 * -difficulty / spellev(spell);
        chance += learning > 20 ? 20 : learning;
    }

    /* Clamp the chance: >18 stat and advanced learning only help
     * to a limit, while chances below "hopeless" only raise the
     * specter of overflowing 16-bit ints (and permit wearing a
     * shield to raise the chances :-).
     */
    if (chance < 0)
        chance = 0;
    if (chance > 120)
        chance = 120;

    /* Wearing anything but a light shield makes it very awkward
     * to cast a spell.  The penalty is not quite so bad for the
     * player's role-specific spell.  Metallic shields still adversely
     * affect spellcasting, no matter how light they are.
     */
    if (uarms) {
        boolean shield_rules = ((is_metallic(uarms) && !is_bracer(uarms))
                                || (weight(uarms) > (int) objects[SMALL_SHIELD].oc_weight));

        if (shield_rules) {
            if (spellid(spell) == urole.spelspec)
                chance /= 2;
            else
                chance /= 4;
        }
    }

    /* Finally, chance (based on player intell/wisdom and level) is
     * combined with ability (based on player intrinsics and
     * encumbrances).  No matter how intelligent/wise and advanced
     * a player is, intrinsics and encumbrance can prevent casting;
     * and no matter how able, learning is always required.
     * Here is also where the players' dexterity factors in.
     */
    chance = (chance * (20 - splcaster) / 15 - splcaster) + dex_adjust;

    /* For those classes that don't cast well, wielding one of these
     * special staves should be a significant help.
     */
    if (uwep && uwep->otyp >= STAFF_OF_DIVINATION
        && uwep->otyp <= ASHWOOD_STAFF) {
#define STAFFBONUS 50
        if (skilltype == P_CLERIC_SPELL
            && (uwep->otyp == STAFF_OF_HOLINESS
                || uwep->otyp == ASHWOOD_STAFF))
            chance += STAFFBONUS;
        if (skilltype == P_HEALING_SPELL
            && (uwep->otyp == STAFF_OF_HEALING
                || uwep->otyp == ASHWOOD_STAFF))
            chance += STAFFBONUS;
        if (skilltype == P_DIVINATION_SPELL
            && (uwep->otyp == STAFF_OF_DIVINATION
                || uwep->otyp == ASHWOOD_STAFF))
            chance += STAFFBONUS;
        if (skilltype == P_MATTER_SPELL
            && (uwep->otyp == STAFF_OF_MATTER
                || uwep->otyp == ASHWOOD_STAFF))
            chance += STAFFBONUS;
        if (skilltype == P_ESCAPE_SPELL
            && (uwep->otyp == STAFF_OF_ESCAPE
                || uwep->otyp == ASHWOOD_STAFF))
            chance += STAFFBONUS;
        if (skilltype == P_ATTACK_SPELL
            && (uwep->otyp == STAFF_OF_WAR
                || uwep->otyp == ASHWOOD_STAFF))
            chance += STAFFBONUS;
        if (skilltype == P_EVOCATION_SPELL
            && (uwep->otyp == STAFF_OF_EVOCATION
                || uwep->otyp == ASHWOOD_STAFF))
            chance += STAFFBONUS;
        if (skilltype == P_ENCHANTMENT_SPELL
            && uwep->otyp == ASHWOOD_STAFF)
            chance += STAFFBONUS;
#undef STAFFBONUS
    }

    /* Wearing body armor hinders spellcasting, 10% per spell level
     * starting at level 1 spells for non-casting roles, and
     * then 10% per spell level starting at level 4 spells for
     * primary casting roles. Wearing a robe and/or wielding one
     * of the special spell staves, or adjusting your dexterity
     * along with all of the vanilla-based factors (int/wis, your
     * experience level, skill level) are factored in prior
     * to this calculation. Obtaining skilled or expert in
     * various spell schools can offset this penalty.
     */
    if (uarm && uarm->otyp != CRYSTAL_PLATE_MAIL) {
#define PENALTY_NON_CASTER (spellev(spell) * 10)
#define PENALTY_PRI_CASTER (spellev(spell) * 10) - 30
        if (primary_casters && spellev(spell) >= 4)
            chance -= PENALTY_PRI_CASTER;
        if (non_casters)
            chance -= PENALTY_NON_CASTER;
#undef PENALTY_NON_CASTER
#undef PENALTY_PRI_CASTER
    }

    /* Clamp to percentile */
    if (chance > 100)
        chance = 100;
    if (chance < 0)
        chance = 0;

    /* As an Illithid, you can always use
     * your natural inherent 'ability' */
    if (spellid(spell) == SPE_PSIONIC_WAVE
        && Race_if(PM_ILLITHID))
        chance = 100;

    /* Druids will always have 100% failure rate
       if wearing any sort of metallic armor. checking
       all armor types here even though some of these
       currently can never be made of metal (cloaks,
       shirts) to futureproof any changes */
    if (Role_if(PM_DRUID)) {
        if ((uarm && is_metallic(uarm))
            || (uarmc && is_metallic(uarmc))
            || (uarmh && is_metallic(uarmh))
            || (uarms && is_metallic(uarms))
            || (uarmg && is_metallic(uarmg))
            || (uarmf && is_metallic(uarmf))
            || (uarmu && is_metallic(uarmu))) {
            chance = 0;
        }
    }

    return chance;
}

STATIC_OVL char *
spellretention(idx, outbuf)
int idx;
char *outbuf;
{
    long turnsleft, percent, accuracy;
    int skill;

    skill = P_SKILL(spell_skilltype(spellid(idx)));
    skill = max(skill, P_UNSKILLED); /* restricted same as unskilled */
    turnsleft = spellknow(idx);
    *outbuf = '\0'; /* lint suppression */

    if (turnsleft < 1L) {
        /* spell has expired; hero can't successfully cast it anymore */
        Strcpy(outbuf, "(gone)");
    } else if (turnsleft >= (long) KEEN) {
        /* full retention, first turn or immediately after reading book */
        Strcpy(outbuf, "100%");
    } else {
        /*
         * Retention is displayed as a range of percentages of
         * amount of time left until memory of the spell expires;
         * the precision of the range depends upon hero's skill
         * in this spell.
         *    expert:  2% intervals; 1-2,   3-4,  ...,   99-100;
         *   skilled:  5% intervals; 1-5,   6-10, ...,   95-100;
         *     basic: 10% intervals; 1-10, 11-20, ...,   91-100;
         * unskilled: 25% intervals; 1-25, 26-50, 51-75, 76-100.
         *
         * At the low end of each range, a value of N% really means
         * (N-1)%+1 through N%; so 1% is "greater than 0, at most 200".
         * KEEN is a multiple of 100; KEEN/100 loses no precision.
         */
        percent = (turnsleft - 1L) / ((long) KEEN / 100L) + 1L;
        accuracy =
            (skill == P_EXPERT) ? 2L : (skill == P_SKILLED)
                                           ? 5L
                                           : (skill == P_BASIC) ? 10L : 25L;
        /* round up to the high end of this range */
        percent = accuracy * ((percent - 1L) / accuracy + 1L);
        Sprintf(outbuf, "%ld%%-%ld%%", percent - accuracy + 1L, percent);
    }
    return outbuf;
}

/* Learn a spell during creation of the initial inventory */
void
initialspell(obj)
struct obj *obj;
{
    int i, otyp = obj->otyp;

    for (i = 0; i < MAXSPELL; i++)
        if (spellid(i) == NO_SPELL || spellid(i) == otyp)
            break;

    if (i == MAXSPELL) {
        impossible("Too many spells memorized!");
    } else if (spellid(i) != NO_SPELL) {
        /* initial inventory shouldn't contain duplicate spellbooks */
        impossible("Spell %s already known.", OBJ_NAME(objects[otyp]));
    } else {
        spl_book[i].sp_id = otyp;
        spl_book[i].sp_lev = objects[otyp].oc_level;
        incrnknow(i, 0);
    }
    return;
}

/* return TRUE if hero knows spell otyp, FALSE otherwise */
boolean
known_spell(otyp)
short otyp;
{
    int i;

    for (i = 0; (i < MAXSPELL) && (spellid(i) != NO_SPELL); i++)
        if (spellid(i) == otyp)
            return TRUE;
    return FALSE;
}

/* return index for spell otyp, or -1 if not found */
int
spell_idx(otyp)
short otyp;
{
    int i;

    for (i = 0; (i < MAXSPELL) && (spellid(i) != NO_SPELL); i++)
        if (spellid(i) == otyp)
            return i;
    return -1;
}

/* forcibly learn spell otyp, if possible */
boolean
force_learn_spell(otyp)
short otyp;
{
    int i;

    if (known_spell(otyp))
        return FALSE;

    for (i = 0; i < MAXSPELL; i++)
        if (spellid(i) == NO_SPELL)
            break;
    if (i == MAXSPELL)
        impossible("Too many spells memorized");
    else {
        spl_book[i].sp_id = otyp;
        spl_book[i].sp_lev = objects[otyp].oc_level;
        incrnknow(i, 1);
        return TRUE;
    }
    return FALSE;
}

/* number of spells hero knows */
int
num_spells()
{
    int i;

    for (i = 0; i < MAXSPELL; i++)
        if (spellid(i) == NO_SPELL)
            break;
    return i;
}

void
dump_spells()
{
    int i;
    char buf[BUFSZ];

    if (spellid(0) == NO_SPELL) {
        putstr(0, 0, "You didn't know any spells.");
        return;
    } else {
        putstr(0, ATR_HEADING, "Spells known:");
        Sprintf(buf, " %-20s     Level  %-12s Fail  Retention", "    Name", "Category");
        putstr(0, ATR_PREFORM, buf);

        for (i = 0; i < MAXSPELL && spellid(i) != NO_SPELL; i++) {
            Sprintf(buf, " %c - %-20s  %2d%s   %-12s %3d%%" "       %3d%%",
                    spellet(i), spellname(i), spellev(i),
                    (spellknow(i) > 1000) ? " " : (spellknow(i) ? "!" : "*"),
                    spelltypemnemonic(spell_skilltype(spellid(i))),
                    100 - percent_success(i),
                    (spellknow(i) * 100 + (KEEN - 1)) / KEEN);
            putstr(0, ATR_PREFORM, buf);
        }
    }
}

/*spell.c*/
