/* NetHack 5.0	spell.c	$NHDT-Date: 1781973068 2026/06/20 16:31:08 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.189 $ */
/*      Copyright (c) M. Stephenson 1988                          */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* spellmenu arguments; 0..n-1 used as svs.spl_book[] index when swapping */
#define SPELLMENU_DUMP (-3)
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
#define incrnknow(spell, x) (svs.spl_book[spell].sp_know = KEEN + (x))

#define spellev(spell) svs.spl_book[spell].sp_lev
#define spellname(spell) OBJ_NAME(objects[spellid(spell)])
/* convert a spl_book[] index 0..MAXSPELL-1 into the single character
   used as menu accelerator and dump-file prefix:
     0..25  -> 'a'..'z'
     26..51 -> 'A'..'Z'
     52..61 -> '0'..'9'
     62+    -> '?' placeholder (only reachable past EvilHack's 62-entry
                spell catalog; spell_let_to_idx rejects '?' so single-
                letter input never resolves to it -- such spells are
                accessible only via menu mode) */
#define spellet(spell) \
    ((char) ((spell) < 26 ? ('a' + (spell)) \
             : (spell) < 52 ? ('A' + (spell) - 26) \
             : (spell) < 62 ? ('0' + (spell) - 52) \
             : '?'))

struct chain_lightning_queue;
struct chain_lightning_zap;

staticfn int spell_let_to_idx(char);
staticfn boolean cursed_book(struct obj * bp);
staticfn boolean confused_book(struct obj *);
staticfn void deadbook_pacify_undead(struct monst *);
staticfn void deadbook(struct obj *);
staticfn int learn(void);
staticfn boolean rejectcasting(void);
staticfn boolean getspell(int *);
staticfn int QSORTCALLBACK spell_cmp(const genericptr, const genericptr);
staticfn void sortspells(void);
staticfn boolean spellsortmenu(void);
staticfn boolean dospellmenu(const char *, int, int *);
staticfn int staff_spell_school(int);
staticfn int percent_success(int);
staticfn char *spellretention(int, char *);
staticfn int throwspell(void);
staticfn void cast_chain_lightning(void);
staticfn void spell_backfire(int);
staticfn boolean spelleffects_check(int, int *, int *);
staticfn boolean can_center_spell_location(coordxy, coordxy);
staticfn void display_spell_target_positions(boolean);
staticfn boolean spell_aim_step(genericptr_t, coordxy, coordxy);
staticfn void propagate_chain_lightning(struct chain_lightning_queue *,
            struct chain_lightning_zap);
staticfn boolean entangle_aloft(struct monst *) NONNULLARG1;
staticfn void grow_grass(coordxy, coordxy, genericptr_t);
staticfn void cast_create_grass(int);
staticfn void grow_tree(coordxy, coordxy, genericptr_t);
staticfn void cast_create_trees(int);
staticfn void cast_power_word_kill(void);

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
 *      Con to make a getaway (SPE_TELEPORT_AWAY)
 *      Dru to call on the plants (SPE_ENTANGLE)
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
 *      to permit magic use).
 */

#define uarmhbon 4 /* Metal helmets interfere with the mind */
#define uarmgbon 6 /* Casting channels through the hands */
#define uarmfbon 2 /* All metal interferes to some degree */

#define STAFF_BONUS 50 /* wielding the staff of the spell's school */

/* since the spellbook itself doesn't blow up, don't say just "explodes" */
static const char explodes[] = "radiates explosive energy";

/* convert a letter or digit into a number in the range 0..61, or -1
   if not a letter/digit; inverse of spellet() */
staticfn int
spell_let_to_idx(char ilet)
{
    int indx;

    indx = ilet - 'a';
    if (indx >= 0 && indx < 26)
        return indx;
    indx = ilet - 'A';
    if (indx >= 0 && indx < 26)
        return indx + 26;
    indx = ilet - '0';
    if (indx >= 0 && indx < 10)
        return indx + 52;
    return -1;
}

/* TRUE: book should be destroyed by caller */
staticfn boolean
cursed_book(struct obj *bp)
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
        make_blinded(BlindedTimeout + rn1(100, 250), TRUE);
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
        poison_strdmg(resist_reduce(rn1(4, 3), POISON_RES) + rn1(2, 1),
                      resist_reduce(rnd(6), POISON_RES) + rnd(6),
                      "contact-poisoned spellbook", KILLED_BY_AN);
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

/* study while confused, or as a draugr (see learn()): returns TRUE if
   the book is destroyed */
staticfn boolean
confused_book(struct obj *spellbook)
{
    boolean gone = FALSE;

    if (!rn2(3) && spellbook->otyp != SPE_BOOK_OF_THE_DEAD) {
        spellbook->in_use = TRUE; /* in case called from learn() */
        if (Confusion)
            pline(
         "Being confused you have difficulties in controlling your actions.");
        else /* a draugr; the failure has nothing to do with confusion */
            pline_The("runes are utterly meaningless to your undead mind.");
        display_nhwindow(WIN_MESSAGE, FALSE);
        You("accidentally tear the spellbook to pieces.");
        trycall(spellbook);
        useup(spellbook);
        gone = TRUE;
    } else {
        You("find yourself reading the %s line over and over again.",
            spellbook == svc.context.spbook.book ? "next" : "first");
    }
    return gone;
}

/* pacify or tame an undead monster */
staticfn void
deadbook_pacify_undead(struct monst *mtmp)
{
    if ((is_undead(mtmp->data) || is_vampshifter(mtmp))
        && cansee(mtmp->mx, mtmp->my)) {
        mtmp->mpeaceful = TRUE;
        if (sgn(mon_aligntyp(mtmp)) == sgn(u.ualign.type)
            && mdistu(mtmp) < 4)
            if (mtmp->mtame) {
                if (mtmp->mtame < 20)
                    mtmp->mtame++;
            } else
                (void) tamedog(mtmp, (struct obj *) 0, TRUE);
        else
            monflee(mtmp, 0, FALSE, TRUE);
        /* keep malign in step with the new peacefulness */
        set_malign(mtmp);
        newsym(mtmp->mx, mtmp->my); /* enable peaceful glyph */
    }
}

/* special effects for the Book of the Dead; reading it while blind is
   allowed so that needs to be taken into account too */
staticfn void
deadbook(struct obj *book2)
{
    struct monst *mtmp;
    coord mm;

    You("turn the pages of the Book of the Dead...");
    makeknown(SPE_BOOK_OF_THE_DEAD);
    observe_object(book2); /* in case blind now and hasn't been seen yet */
    /* KMH -- Need ->known to avoid "_a_ Book of the Dead" */
    book2->known = 1;
    if (invocation_pos(u.ux, u.uy) && !On_stairs(u.ux, u.uy)) {
        struct obj *otmp;
        boolean arti1_primed = FALSE, arti2_primed = FALSE,
                         arti_cursed = FALSE;

        if (book2->cursed) {
            pline_The("%s!",
                      Blind ? "Book seems to be ignoring you"
                            : "runes appear scrambled.  You can't read them");
            return;
        }

        if (!u.uhave.bell || !u.uhave.menorah) {
            pline("A chill runs down your %s.", body_part(SPINE));
            if (!u.uhave.bell) {
                Soundeffect(se_faint_chime, 30);
                You_hear("a faint chime...");
            }
            if (!u.uhave.menorah)
                pline("Vlad's doppelganger is amused.");
            return;
        }

        for (otmp = gi.invent; otmp; otmp = otmp->nobj) {
            if (otmp->otyp == CANDELABRUM_OF_INVOCATION && otmp->spe == 7
                && otmp->lamplit) {
                if (!otmp->cursed)
                    arti1_primed = TRUE;
                else
                    arti_cursed = TRUE;
            }
            if (otmp->otyp == BELL_OF_OPENING
                && (svm.moves - otmp->age) < 5L) { /* you rang it recently */
                if (!otmp->cursed)
                    arti2_primed = TRUE;
                else
                    arti_cursed = TRUE;
            }
        }

        if (arti_cursed) {
            pline_The("invocation fails!");
            /* this used to say "your artifacts" but the invocation tools
               are not artifacts */
            pline("At least one of your relics is cursed...");
        } else if (arti1_primed && arti2_primed) {
            unsigned soon = (unsigned) d(2, 6); /* time til next intervene() */

            /* successful invocation */
            mkinvokearea();
            u.uevent.invoked = 1;
            record_achievement(ACH_INVK);
            /* in case you haven't killed the Wizard yet, behave as if
               you just did */
            u.uevent.udemigod = 1; /* wizdeadorgone() */
            if (!u.udg_cnt || u.udg_cnt > soon)
                u.udg_cnt = soon;
        } else { /* at least one relic not prepared properly */
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
            newsym(mtmp->mx, mtmp->my); /* clear peaceful glyph */
        }
        /* next handle the affect on things you're carrying */
        (void) unturn_dead(&gy.youmonst);
        /* last place some monsters around you */
        mm.x = u.ux;
        mm.y = u.uy;
        mkundead(&mm, TRUE, NO_MINVENT);
    } else if (book2->blessed) {
        iter_mons(deadbook_pacify_undead);
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

/* 'book' has just become cursed; if we're reading it, interrupt */
void
book_cursed(struct obj *book)
{
    if (book->cursed && gm.multi >= 0
        && go.occupation == learn && svc.context.spbook.book == book) {
        pline("%s shut!", Tobjnam(book, "slam"));
        set_bknown(book, 1);
        stop_occupation();
    }
}

DISABLE_WARNING_FORMAT_NONLITERAL

staticfn int
learn(void)
{
    int i;
    short booktype;
    char splname[BUFSZ];
    boolean costly = TRUE, faded_to_blank = FALSE;
    struct obj *book = svc.context.spbook.book;

    /* the book can be destroyed mid-study without interrupting the hero
       (burned by a fire explosion, for one); book_disappears() has
       cleared the pointer, so just stop */
    if (!book) {
        svc.context.spbook.delay = 0;
        svc.context.spbook.o_id = 0;
        return 0;
    }

    /* JDS: lenses give 50% faster reading; 33% smaller read time */
    if (svc.context.spbook.delay && ublindf
        && ublindf->otyp == LENSES && rn2(2))
        svc.context.spbook.delay++;
    /* confusion can set in while studying; and a draugr's undead mind
       can't make sense of any book but the Book of the Dead */
    if (Confusion
        || (Race_if(PM_DRAUGR) && book->otyp != SPE_BOOK_OF_THE_DEAD)) {
        (void) confused_book(book);
        svc.context.spbook.book = 0; /* no longer studying */
        svc.context.spbook.o_id = 0;
        nomul(svc.context.spbook.delay); /* remaining delay is uninterrupted */
        gm.multi_reason = "reading a book";
        gn.nomovemsg = 0;
        svc.context.spbook.delay = 0;
        return 0;
    }
    if (svc.context.spbook.delay) {
        /* not if (svc.context.spbook.delay++), so at end delay == 0 */
        svc.context.spbook.delay++;
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
            faded_to_blank = TRUE;
            /* reset spestudied as if polymorph had taken place */
            book->spestudied = rn2(book->spestudied);
        } else {
            Your("knowledge of %s is %s.", splname,
                 spellknow(i) ? "keener" : "restored");
            incrnknow(i, 1);
            book->spestudied++;
            exercise(A_INT, TRUE); /* extra study */
        }
    } else { /* (spellid(i) == NO_SPELL) */
        /* for a normal book, spestudied will be zero, but for
           a polymorphed one, spestudied will be non-zero and
           one less reading is available than when re-learning */
        if (book->spestudied >= MAX_SPELL_STUDY) {
            /* pre-used due to being the product of polymorph */
            pline("This spellbook is too faint to read even once.");
            book->otyp = booktype = SPE_BLANK_PAPER;
            set_material(book, PAPER);
            faded_to_blank = TRUE;
            /* reset spestudied as if polymorph had taken place */
            book->spestudied = rn2(book->spestudied);
        } else {
            svs.spl_book[i].sp_id = booktype;
            svs.spl_book[i].sp_lev = objects[booktype].oc_level;
            incrnknow(i, 1);
            book->spestudied++;
            if (!i)
                /* first is always 'a', so no need to mention the letter */
                You("learn %s.", splname);
            else
                You("add %s to your repertoire, as '%c'.",
                    splname, spellet(i));
            exercise(A_INT, TRUE);
        }
    }
    if (i < MAXSPELL) {
        /* might be learning a new spellbook type or spellbook of blank paper;
           if so, persistent inventory will get updated */
        makeknown((int) booktype);
        /* makeknown() calls update_inventory() when discovering something
           new but is a no-op for something that's already known so wouldn't
           update persistent inventory to reflect faded book if spellbook of
           blank paper happens to already be discovered */
        if (faded_to_blank)
            update_inventory();
    }

    /* maybe a demon cursed it; a book that just faded to blank paper
       has no spell level for cursed_book() to roll against */
    if (book->cursed && book->otyp != SPE_BLANK_PAPER) {
        if (cursed_book(book)) {
            useup(book);
            svc.context.spbook.book = 0;
            svc.context.spbook.o_id = 0;
            return 0;
        }
    }
    if (costly)
        check_unpaid(book);
    svc.context.spbook.book = 0;
    svc.context.spbook.o_id = 0;
    return 0;
}

RESTORE_WARNING_FORMAT_NONLITERAL

int
study_book(struct obj *spellbook)
{
    int booktype = spellbook->otyp, i;
    boolean confused = (Confusion != 0);
    boolean too_hard = FALSE;

    /* attempting to read dull book may make hero fall asleep */
    if (!confused && !Sleep_resistance
        && objdescr_is(spellbook, "dull")) {
        const char *eyes;
        int dullbook = rnd(25) - ACURR(A_WIS);

        /* adjust chance if hero stayed awake, got interrupted, retries */
        if (svc.context.spbook.delay && spellbook == svc.context.spbook.book)
            dullbook -= rnd(objects[booktype].oc_level);

        if (dullbook > 0) {
            eyes = body_part(EYE);
            if (eyecount(gy.youmonst.data) > 1)
                eyes = makeplural(eyes);
            pline("This book is so dull that you can't keep your %s open.",
                  eyes);
            dullbook += rnd(2 * objects[booktype].oc_level);
            fall_asleep(-dullbook, TRUE);
            return 1;
        }
    }

    if (svc.context.spbook.delay && !confused
        && spellbook == svc.context.spbook.book
        /* handle the sequence: start reading, get interrupted, have
           svc.context.spbook.book become erased somehow, resume reading it */
        && booktype != SPE_BLANK_PAPER) {
        You("continue your efforts to %s.",
            (booktype == SPE_NOVEL) ? "read the novel"
                                    : "memorize the spell");
    } else {
        /* KMH -- Simplified this code */
        if (booktype == SPE_BLANK_PAPER) {
            pline("This spellbook is all blank.");
            makeknown(booktype);
            return 1;
        }

        /* only illithids can absorb this ability */
        if (booktype == SPE_PSIONIC_WAVE && !Race_if(PM_ILLITHID)) {
            You(
     "do not understand the strange language this book is written in.");
            pline_The("inscriptions in the book start to fade away!");
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
                                   "became literate by reading %s",
                                   tribtitle);

                check_unpaid(spellbook);
                makeknown(booktype);
                if (!u.uevent.read_tribute) {
                    record_achievement(ACH_NOVL);
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
            svc.context.spbook.delay = -objects[booktype].oc_delay;
            break;
        case 3:
        case 4:
            svc.context.spbook.delay = -(objects[booktype].oc_level - 1)
                                   * objects[booktype].oc_delay;
            break;
        case 5:
        case 6:
            svc.context.spbook.delay =
                -objects[booktype].oc_level * objects[booktype].oc_delay;
            break;
        case 7:
            svc.context.spbook.delay = -8 * objects[booktype].oc_delay;
            break;
        default:
            impossible("Unknown spellbook level %d, book %d;",
                       objects[booktype].oc_level, booktype);
            return 0;
        }

        /* check to see if we already know it and want to refresh our memory */
        for (i = 0; i < MAXSPELL; i++)
            if (spellid(i) == booktype || spellid(i) == NO_SPELL)
                break;
        if (spellid(i) == booktype && spellknow(i) > KEEN / 10) {
            You("know \"%s\" quite well already.",
                OBJ_NAME(objects[booktype]));
            /* hero has just been told what spell this book is for; it may
               have been undiscovered if spell was learned via divine gift */
            makeknown(booktype);
            if (y_n("Refresh your memory anyway?") == 'n')
                return 0;
        }

        /* Books are often wiser than their readers (Rus.) */
        spellbook->in_use = TRUE;
        /* a blessed book is failure-proof, except for a Caveperson */
        if ((!spellbook->blessed || Role_if(PM_CAVE_DWELLER))
            && spellbook->otyp != SPE_BOOK_OF_THE_DEAD) {
            if (spellbook->cursed) {
                too_hard = TRUE;
            } else {
                /* uncursed (or blessed for a Caveperson) - chance to fail */
                int read_ability = ACURR(A_INT) + 4 + u.ulevel / 2
                                   - 2 * objects[booktype].oc_level
                             + ((ublindf && ublindf->otyp == LENSES) ? 2 : 0);

                /* only wizards know if a spell is too difficult */
                if (Role_if(PM_WIZARD) && read_ability < 20 && !confused) {
                    char qbuf[QBUFSZ];

                    Sprintf(qbuf,
                    "This spellbook is %sdifficult to comprehend.  Continue?",
                            (read_ability < 12 ? "very " : ""));
                    if (y_n(qbuf) != 'y') {
                        spellbook->in_use = FALSE;
                        return 1;
                    }
                }
                /* Cavepersons always have at least a 20% chance to fail */
                if (Role_if(PM_CAVE_DWELLER) && read_ability > 16)
                    read_ability = 16;
                /* its up to random luck now */
                if (rnd(20) > read_ability) {
                    too_hard = TRUE;
                }
            }
        }

        if (too_hard) {
            boolean gone = cursed_book(spellbook);

            nomul(svc.context.spbook.delay); /* study time */
            gm.multi_reason = "reading a book";
            gn.nomovemsg = 0;
            svc.context.spbook.delay = 0;
            if (gone || !rn2(3)) {
                if (!gone)
                    pline_The("spellbook crumbles to dust!");
                trycall(spellbook);
                useup(spellbook);
            } else
                spellbook->in_use = FALSE;
            return 1;
        } else if (confused) {
            if (!confused_book(spellbook)) {
                spellbook->in_use = FALSE;
            }
            nomul(svc.context.spbook.delay);
            gm.multi_reason = "reading a book";
            gn.nomovemsg = 0;
            svc.context.spbook.delay = 0;
            return 1;
        }
        spellbook->in_use = FALSE;

        You("begin to %s the runes.",
            spellbook->otyp == SPE_BOOK_OF_THE_DEAD ? "recite" : "memorize");
    }

    svc.context.spbook.book = spellbook;
    if (svc.context.spbook.book)
        svc.context.spbook.o_id = svc.context.spbook.book->o_id;
    set_occupation(learn, "studying", 0);
    return 1;
}

/* a spellbook has been destroyed or the character has changed levels;
   the stored address for the current book is no longer valid */
void
book_disappears(struct obj *obj)
{
    if (obj == svc.context.spbook.book) {
        svc.context.spbook.book = (struct obj *) 0;
        svc.context.spbook.o_id = 0;
    }
}

/* renaming an object usually results in it having a different address;
   so the sequence start reading, get interrupted, name the book, resume
   reading would read the "new" book from scratch */
void
book_substitution(struct obj *old_obj, struct obj *new_obj)
{
    if (old_obj == svc.context.spbook.book) {
        svc.context.spbook.book = new_obj;
        if (svc.context.spbook.book)
            svc.context.spbook.o_id = svc.context.spbook.book->o_id;
    }
}

/* called from moveloop() */
void
age_spells(void)
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
staticfn boolean
rejectcasting(void)
{
    /* rejections which take place before selecting a particular spell */
    if (Stunned) {
        You("are too impaired to cast a spell.");
        return TRUE;
    } else if (!can_chant(&gy.youmonst)) {
        You("are unable to chant the incantation.");
        return TRUE;
    } else if (!freehand() && !(uwep && uwep->otyp == QUARTERSTAFF)) {
        /* Note: !freehand() occurs when weapon and shield (or two-handed
         * weapon) are welded to hands, so "arms" probably doesn't need
         * to be makeplural(bodypart(ARM)).
         *
         * But why isn't lack of free arms (for gesturing) an issue when
         * poly'd hero has no limbs?
         */
        Your("arms are not free to cast!");
        return TRUE;
    } else if (nohands(gy.youmonst.data)) {
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
staticfn boolean
getspell(int *spell_no)
{
    int nspells, idx, retry_limit;
    char ilet, lets[BUFSZ], qbuf[QBUFSZ];
    struct _cmd_queue cq, *cmdq;

    nspells = num_spells();
    if (!nspells) {
        You("don't know any spells right now.");
        return FALSE;
    }
    if (rejectcasting())
        return FALSE; /* no spell chosen */

    if ((cmdq = cmdq_pop()) != 0) {
        cq = *cmdq;
        free(cmdq);
        if (cq.typ == CMDQ_KEY) {
            idx = spell_let_to_idx(cq.key);
            if (idx < 0 || idx >= nspells)
                return FALSE;
            *spell_no = idx;
            return TRUE;
        } else {
            return FALSE;
        }
    }

    if (flags.menu_style == MENU_TRADITIONAL) {
        /* if we get here, we know there is at least 1 known spell */
        if (nspells == 1)
            Strcpy(lets, "a");
        else if (nspells < 27)
            Sprintf(lets, "a-%c", 'a' + nspells - 1);
        else if (nspells == 27)
            Strcpy(lets, "a-zA");
        else if (nspells < 53)
            Sprintf(lets, "a-zA-%c", 'A' + nspells - 27);
        else if (nspells == 53)
            Strcpy(lets, "a-zA-Z0");
        else if (nspells <= 62)
            /* a-zA-Z0-9 caps at 62 single-letter accessible spells */
            Sprintf(lets, "a-zA-Z0-%c", '0' + nspells - 53);
        else
            /* spells past the 62nd are only accessible via menu mode */
            Strcpy(lets, "a-zA-Z0-9");

        Snprintf(qbuf, sizeof qbuf, "Cast which spell? [%s *?]", lets);
        for (retry_limit = 0; ; ++retry_limit) {
            if (retry_limit == 10) {
                /* limit is mainly to prevent the fuzzer from getting stuck
                   since hangup should hit the 'quitchars' case; fuzzer
                   would too, but after an arbitrary number of attempts */
                pline("That's enough tries.");
                return FALSE;
            }
            ilet = yn_function(qbuf, (char *) 0, '\0', TRUE);
            if (ilet == '*' || ilet == '?')
                break; /* use menu mode */
            if (strchr(quitchars, ilet)) {
                pline1(Never_mind);
                return FALSE;
            }

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

/* #wizcast - cast any spell even without knowing it */
int
dowizcast(void)
{
    winid win;
    menu_item *selected;
    anything any;
    int i, n;

    win = create_nhwindow(NHW_MENU);
    start_menu(win, MENU_BEHAVE_STANDARD);
    any = cg.zeroany;

    for (i = 0; i < MAXSPELL; i++) {
        n = (SPE_DIG + i);
        if (n >= SPE_BLANK_PAPER)
            break;
        any.a_int = n;
        add_menu(win, &nul_glyphinfo, &any, 0, 0, ATR_NONE, NO_COLOR,
                 OBJ_NAME(objects[n]), MENU_ITEMFLAGS_NONE);
    }
    end_menu(win, "Cast which spell?");
    n = select_menu(win, PICK_ONE, &selected);
    destroy_nhwindow(win);
    if (n > 0) {
        i = selected[0].item.a_int;
        free((genericptr_t) selected);
        /* a wizard-mode cast runs at Expert in the spell's school for the
           whole cast */
        return cast_at_expert(i);
    }
    return ECMD_OK;
}

/* cast otyp as a forced cast at Expert in its school (#wizcast and the
   Fire Brand / Frost Brand invokes); a death during the cast
   (really_done()) and a save taken inside it (savegamestate()) see the
   school's real level */
int
cast_at_expert(int otyp)
{
    int skill = spell_skilltype(otyp), res;

    if (gs.expert_cast_skill != P_NONE) {
        impossible("cast_at_expert: already casting at Expert");
        return spelleffects(otyp, FALSE, TRUE);
    }
    gs.expert_cast_skill = skill;
    gs.expert_cast_level = P_SKILL(skill);
    P_SKILL(skill) = P_EXPERT;
    res = spelleffects(otyp, FALSE, TRUE);
    end_expert_cast();
    return res;
}

/* the real level of the school cast_at_expert() holds at Expert, so
   only valid while it holds one; keeps any loss the cast itself caused
   (drain life cast at self drops the last skill advance via losexp()) */
int
expert_cast_real_level(void)
{
    return gs.expert_cast_level
           + (P_SKILL(gs.expert_cast_skill) - P_EXPERT);
}

/* undo cast_at_expert()'s bracket; really_done() calls it too, for a
   hero who died during the cast */
void
end_expert_cast(void)
{
    if (gs.expert_cast_skill == P_NONE)
        return;
    P_SKILL(gs.expert_cast_skill) = expert_cast_real_level();
    gs.expert_cast_skill = P_NONE;
    if (!program_state.panicking)
        find_ac(); /* barkskin and stoneskin set AC while at Expert */
}

/* the #cast command -- cast a spell */
int
docast(void)
{
    int spell_no;

    if (getspell(&spell_no)) {
        cmdq_add_key(CQ_REPEAT, spellet(spell_no));
        return spelleffects(svs.spl_book[spell_no].sp_id, FALSE, FALSE);
    }
    return ECMD_FAIL;
}

const char *
spelltypemnemonic(int skill)
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
spell_skilltype(int booktype)
{
    return objects[booktype].oc_skill;
}

/* Wizards learn what spellbooks look like based on their skill in the
   spell's school */
void
skill_based_spellbook_id(void)
{
    if (!Role_if(PM_WIZARD))
        return;

    int booktype;
    const uchar spbook_class = (uchar) SPBOOK_CLASS;

    for (booktype = svb.bases[spbook_class];
         booktype < svb.bases[spbook_class + 1];
         booktype++) {
        int known_up_to_level;
        int skill = spell_skilltype(booktype);

        if (skill == P_NONE)
            continue;

        switch (P_SKILL(skill)) {
        case P_BASIC:
            known_up_to_level = 3;
            break;
        case P_SKILLED:
            known_up_to_level = 5;
            break;
        case P_EXPERT:
        case P_MASTER:
        case P_GRAND_MASTER:
            known_up_to_level = 7;
            break;
        case P_UNSKILLED:
        default:
            /* paupers need more skill than this to ID books, but most wizards
               know the basics */
            known_up_to_level = u.uroleplay.pauper ? 0 : 1;
            break;
        }

        if (objects[booktype].oc_level <= known_up_to_level)
            /* makeknown(booktype) but don't exercise Wisdom or mark as
               encountered */
            discover_object(booktype, TRUE, FALSE, FALSE);
    }
}

/* Limit the total area chain lightning can cover; this is both for
   technical reasons (making it possible to limit the size of arrays
   here and in the display code) and for gameplay balance reasons;
   this value should be smaller than TMP_AT_MAX_GLYPHS (display.c) in
   order for chain lightning to display properly */
#define CHAIN_LIGHTNING_LIMIT 100
/* Unlike most zaps, chain lightning can't hit solid terrain (it
   doesn't have enough power), it only covers open space; this also
   means that it can't hit monsters inside walls, which makes sense as
   they would be earthed */
#define CHAIN_LIGHTNING_TYP(typ) \
    (SPACE_POS(typ) || (typ) == POOL || (typ) == MOAT /* not WATER */   \
     || (typ) == DRAWBRIDGE_UP || (typ) == LAVAPOOL)  /* not LAVAWALL */
#define CHAIN_LIGHTNING_POS(x, y) \
    (isok(x, y) && (CHAIN_LIGHTNING_TYP(levl[x][y].typ)                 \
                    || (IS_DOOR(levl[x][y].typ)                         \
                        && !(levl[x][y].doormask & (D_CLOSED | D_LOCKED)))))

struct chain_lightning_zap {
    /* direction in which this zap is currently moving; this is an
       enum movementdirs, clamped to the range 0 inclusive to N_DIRS
       exclusive */
    uchar dir;
    /* current location of the zap */
    coordxy x, y;
    /* distance this zap can cover without chaining */
    char strength;
};

struct chain_lightning_queue {
    struct chain_lightning_zap q[CHAIN_LIGHTNING_LIMIT];
    int head;
    int tail;
    int displayed_beam;
};

/* Given a potential chain lightning zap, moves it one square forward in
   the given direction, then adds it to the queue unless it would hit an
   invalid square or is out of power.

   zap is passed by value, so the move-forward doesn't change the passed
   argument. */
staticfn void
propagate_chain_lightning(
    struct chain_lightning_queue *clq,
    struct chain_lightning_zap zap)
{
    struct monst *mon;

    zap.x += xdir[zap.dir];
    zap.y += ydir[zap.dir];

    if (clq->tail >= CHAIN_LIGHTNING_LIMIT)
        return;    /* zap has covered too many squares */
    if (!CHAIN_LIGHTNING_POS(zap.x, zap.y))
        return;    /* zap can't go to this square */

    mon = m_at(zap.x, zap.y);
    if (mon && mon->mpeaceful)
        return;    /* chain lightning avoids peaceful and tame monsters */

    /* When hitting a monster that isn't electricity-resistant, a
       particular chain lightning zap regains all its power, allowing it to
       chain to other monsters; upon hitting a shock-resistant monster it
       can't continue any further, but we let it hit the monster to show
       the shield effect */
    if (mon && !resists_elec(mon) && !defended(mon, AD_ELEC))
        zap.strength = 3;
    else if (mon)
        zap.strength = 0;

    /* Unless it hits a monster, the last square of a zap isn't drawn on
       screen and can't propagate further, so it may as well be discarded
       now */
    if (!mon && !zap.strength)
        return;

    /* The same square can't be chained to twice. */
    for (int i = 0; i < clq->tail; i++) {
        if (clq->q[i].x == zap.x && clq->q[i].y == zap.y)
            return;
    }

    /* This array access must be inbounds due to the CHAIN_LIGHTNING_LIMIT
       check earlier. */
    clq->q[clq->tail++] = zap;

    /* Draw it. */
    tmp_at(DISP_CHANGE, zapdir_to_glyph(xdir[zap.dir], ydir[zap.dir],
                                        clq->displayed_beam));
    tmp_at(zap.x, zap.y);
}

staticfn void
cast_chain_lightning(void)
{
    struct chain_lightning_queue clq = {
        {{0}}, 0, 0, Hallucination ? rn2_on_display_rng(6) : (AD_ELEC - 1)
    };

    if (u.uswallow) {
        // TODO: damage the engulfer
        return;
    }

    /* set the type of beam we're using; the direction here is arbitrary
       because we change the beam direction just before drawing the beam
       anyway */
    tmp_at(DISP_BEAM, zapdir_to_glyph(0, 1, clq.displayed_beam));

    /* start by propagating in all directions from the caster */
    for (int dir = 0; dir < N_DIRS; dir++) {
        struct chain_lightning_zap zap = { dir, u.ux, u.uy, 2 };

        propagate_chain_lightning(&clq, zap);
    }
    nh_delay_output();

    while (clq.head < clq.tail) {
        int delay_tail = clq.tail;

        while (clq.head < delay_tail) {
            struct chain_lightning_zap zap = clq.q[clq.head++];
            /* damage any monster that was hit */
            struct monst *mon = m_at(zap.x, zap.y);

            if (mon) {
                struct obj *unused = 0; /* AD_ELEC can't destroy armor */
                int dmg;

                gn.notonhead = (mon->mx != gb.bhitpos.x
                                || mon->my != gb.bhitpos.y);
                dmg = zhitm(mon, BZ_U_SPELL(BZ_OFS_AD(AD_ELEC)), 2, &unused);

                if (dmg) {
                    /* mon has been damaged, but we haven't yet printed the
                       messages or given kill credit; assume the hero can
                       sense their spell hitting monsters, because they can
                       steer it away from peacefuls */
                    if (DEADMONSTER(mon)) {
                        xkilled(mon, XKILL_GIVEMSG);
                    } else {
                        pline("You shock %s%s", mon_nam(mon), exclam(dmg));
                        /* if a long worm, only map 'I' for its head */
                        if (!canseemon(mon) && !gn.notonhead)
                            /* FIXME: this doesn't work, possibly because
                               cleaning up tmp_at() restores old glyph? */
                            map_invisible(zap.x, zap.y);
                    }
                } else if (canseemon(mon)) {
                    pline("%s resists.", Monnam(mon));
                }
                if (!DEADMONSTER(mon)) {
                    /* wakeup is via attack, but since mon is already
                       hostile we pass via_attack==False rather than True,
                       otherwise other monsters witnessing this would treat
                       it as seeing hero attack a peaceful; mimic will be
                       exposed; forcefight makes hider unhide */
                    svc.context.forcefight++;
                    wakeup(mon, FALSE);
                    svc.context.forcefight--;
                }
            }

            /* each zap propagates forwards with 1 less strength, and
               diagonally with 0 strength (thus the diagonal zaps aren't
               drawn and don't spread unless they hit a monster);
               exception: if the zap just hit a monster, the diagonals have
               as much strength as the forwards zap */
            if (!zap.strength)
                continue; /* happens upon hitting a shock-resistant monster */
            zap.strength--;

            propagate_chain_lightning(&clq, zap);

            if (zap.strength < 2)
                zap.strength = 0;
            else if (u.uen > 0)
                u.uen--; /* propagating past mons increases Pw cost a bit */
            zap.dir = DIR_LEFT(zap.dir);
            propagate_chain_lightning(&clq, zap);

            zap.dir = DIR_RIGHT2(zap.dir);
            propagate_chain_lightning(&clq, zap);
        }
        nh_delay_output();
    }
    nh_delay_output();
    nh_delay_output();

    tmp_at(DISP_END, 0);
}

void
cast_protection(int skill)
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
                struct permonst *pm = u.ustuck ? u.ustuck->data : 0;

                rmtyp = levl[u.ux][u.uy].typ;
                atmosphere = (pm && u.uswallow)
                                ? ((pm == &mons[PM_FOG_CLOUD]) ? "mist"
                                   : is_whirly(pm) ? "maelstrom"
                                     : enfolds(pm) ? "folds"
                                       : is_animal(pm) ? "maw"
                                         : "ooze")
                                : (u.uinwater ? hliquid("water")
                                   : (rmtyp == CLOUD) ? "cloud"
                                     : IS_TREES(rmtyp) ? "vegetation"
                                       : IS_STWALL(rmtyp) ? "stone"
                                         : "air");
                pline_The("%s around you begins to shimmer with %s haze.",
                          atmosphere, an(hgolden));
            }
        }
        u.uspellprot += gain;
        u.uspmtime = (skill == P_EXPERT) ? 20 : 10;
        if (!u.usptime)
            u.usptime = u.uspmtime;
        find_ac();
    } else {
        Your("skin feels warm for a moment.");
    }
}

void
cast_reflection(struct monst *mdef, int skill)
{
    boolean youdefend = (mdef == &gy.youmonst);
    int duration = (skill == P_EXPERT) ? 750
                   : (skill == P_SKILLED) ? 500
                     : (skill == P_BASIC) ? 250
                       : 100;

    if (youdefend) {
        u.uconduct.reflection++;
        /* only the timed part is the spell's; a silver dragon form's
           reflection sits in the same word */
        if (HReflecting & TIMEOUT) {
            if (!Blind)
                pline_The(
                 "shimmering globe around you becomes slightly brighter.");
            else
                You_feel("slightly more smooth.");
        } else {
            if (!Blind)
                pline("A shimmering globe appears around you!");
            else
                You_feel("smooth.");
        }
        /* the higher the skill in matter-based spells, the longer
           the effect */
        incr_itimeout(&HReflecting,
                      rn1(10, (HReflecting & TIMEOUT) ? (duration / 5)
                                                       : duration));
    } else {
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
cast_barkskin(struct monst *mdef, int skill)
{
    boolean youdefend = (mdef == &gy.youmonst);
    int duration = (skill == P_EXPERT) ? 750
                   : (skill == P_SKILLED) ? 500
                     : (skill == P_BASIC) ? 250
                       : 100;

    if (youdefend) {
        if (Stoneskin) {
            You_cant("cast Barkskin while Stoneskin is active.");
            return;
        } else if (HBarkskin) {
            /* increases timeout only */
            pline_The("bark covering your %s feels a bit thicker.",
                      mbodypart(&gy.youmonst, SKIN));
        } else {
            pline("A thick layer of bark covers your %s.",
                  mbodypart(&gy.youmonst, SKIN));
        }
        /* the higher the skill in evocation-based spells, the longer
           the effect */
        incr_itimeout(&HBarkskin,
                      rn1(10, HBarkskin ? (duration / 5) : duration));
        find_ac(); /* adjust AC; dmg reduction handled in hitmu() */
    } else {
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
cast_stoneskin(struct monst *mdef, int skill)
{
    boolean youdefend = (mdef == &gy.youmonst);
    int duration = (skill == P_EXPERT) ? 750
                   : (skill == P_SKILLED) ? 500
                     : (skill == P_BASIC) ? 250
                       : 100;

    if (youdefend) {
        if (Barkskin) {
            You_cant("cast Stoneskin while Barkskin is active.");
            return;
        } else if (HStoneskin) {
            /* increases timeout only */
            pline_The("stone covering your %s feels a bit thicker.",
                      mbodypart(&gy.youmonst, SKIN));
        } else {
            pline("A thick layer of stone covers your %s.",
                  mbodypart(&gy.youmonst, SKIN));
        }
        /* become stone resistant */
        HStone_resistance |= I_SPECIAL;

        /* the higher the skill in evocation-based spells, the longer
           the effect */
        incr_itimeout(&HStoneskin,
                      rn1(10, HStoneskin ? (duration / 5) : duration));
        find_ac(); /* adjust AC; dmg reduction handled in hitmu() */
    } else {
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

/* is a monster caught by entangle up in the air, where a tree's roots
   can't reach it? One that flies or floats, or a rider its flying steed
   carries; the hero's steed is up there when the pair is (Flying counts
   the steed's lift, and a hold that keeps the pair down) */
staticfn boolean
entangle_aloft(struct monst *mdef)
{
    if (mdef == u.usteed)
        return (Levitation || Flying);
    return (mon_airborne(mdef) || mon_steed_airborne(mdef));
}

void
cast_entangle(struct monst *mdef, int skill)
{
    boolean youdefend = (mdef == &gy.youmonst);
    int tangle = (skill == P_EXPERT) ? 5
                 : (skill == P_SKILLED) ? 4
                   : (skill == P_BASIC) ? 3
                     : 1;

    /* target needs to be standing on or near vegetation for
       entanglement to work */
    if (youdefend) {
        if (!(levl[u.ux][u.uy].typ == GRASS || nexttotree(u.ux, u.uy))) {
            You("must be on or near vegetation to become entangled.");
            return;
        }
    } else {
        if (!(levl[mdef->mx][mdef->my].typ == GRASS
              || nexttotree(mdef->mx, mdef->my))) {
            pline("%s must be on or near vegetation to become entangled.",
                  Monnam(mdef));
            return;
        }
    }

    if (youdefend) {
        boolean aloft, by_tree;

        if (u.utrap && u.utraptype == TT_ENTANGLED) {
            You("are already entangled!");
            return;
        }
        if (is_whirly(gy.youmonst.data) || passes_walls(gy.youmonst.data)
            || unsolid(gy.youmonst.data) || amorphous(gy.youmonst.data)) {
            pline_The("spell fails to entangle you.");
            return;
        }
        /* in a bear trap, a web or a pit, sinking into lava, set in
           solidified lava or chained to a buried iron ball: something
           already holds the hero, and the vines must not replace that
           hold */
        if (u.utrap) {
            pline_The("spell fails to entangle you.");
            return;
        }
        /* dismount before describing the entanglement: the steed
           may pitch the player onto a different tile, so the
           messages and the eventual set_utrap should describe the
           landing tile rather than the mounted-position tile */
        if (u.usteed) {
            newsym(u.usteed->mx, u.usteed->my);
            dismount_steed(DISMOUNT_FELL);
            if (!(levl[u.ux][u.uy].typ == GRASS
                  || nexttotree(u.ux, u.uy))) {
                You("land on bare ground; the spell loses its grip.");
                return;
            }
        }
        /* after the dismount: a flying steed's rider is down now */
        aloft = (Levitation || Flying);
        by_tree = nexttotree(u.ux, u.uy);
        /* roots can't reach a hero in the air, nor one over water, lava
           or a hole, whom the branches hold up (entangle_release()) */
        if (by_tree)
            pline_The("%s from a nearby tree ensnare you!",
                      (aloft || u_over_drop() || rn2(2)) ? "branches"
                                                         : "roots");
        else
            pline_The(
                "grass underneath you twists and loops around your %s!",
                makeplural(body_part(LEG)));
        You("are entangled!");
        set_utrap(rn1(6, 3), TT_ENTANGLED); /* 3-8 turns */
        /* a tree's branches hold a flyer or floater where it is, even
           over water, a hole or open air, until they let go (allmain);
           grass can only hold the hero on the ground it grows from, so
           it pulls the hero down onto it. Deferred, since a monster's
           spell arrives mid-beam */
        if (aloft && !by_tree)
            defer_landing(LANDING_FLY);
        return;
    } else {
        if (mdef->mentangled) {
            /* if already entangled, don't add to it */
            if (canseemon(mdef))
                pline("%s is already entangled!", Monnam(mdef));
            return;
        } else if (is_whirly(mdef->data)
                   || passes_walls(mdef->data)
                   || unsolid(mdef->data)
                   || amorphous(mdef->data)) {
            /* caster-agnostic phrasing; this branch is reached from
               m-v-m spell paths where the player isn't the caster */
            if (canseemon(mdef))
                pline_The("spell fails to entangle %s.", mon_nam(mdef));
            return;
        } else {
            if (canseemon(mdef)) {
                /* nor a monster up in the air, as for the hero */
                if (nexttotree(mdef->mx, mdef->my))
                    pline_The("%s from a nearby tree ensnare %s!",
                              (entangle_aloft(mdef) || rn2(2)) ? "branches"
                                                              : "roots",
                              mon_nam(mdef));
                else
                    pline_The(
                        "grass underneath %s twists and loops around %s %s!",
                        mon_nam(mdef), mhis(mdef),
                        makeplural(mbodypart(mdef, LEG)));
                pline("%s is entangled!", Monnam(mdef));
            }
            mdef->mentangled = 1;
            /* 3-8 turns at basic, 4-9 turns at skilled,
               5-10 turns at expert. at unskilled/restricted,
               1-6 turns */
            mdef->mentangletime = rn1(6, tangle);
            /* if spell skill is skilled or greater, chance to
               temporarily slow mdef */
            if (rn2(2) && skill >= P_SKILLED)
                mon_adjust_speed(mdef, -2, (struct obj *) 0);
        }
    }
}

int
cast_metal_to_wood(struct obj *obj, boolean by_you)
{
    if (!obj)
        return 0;

    if (by_you)
        return metal_to_wood(obj, TRUE);
    else
        return metal_to_wood(obj, FALSE);
}

/* do_clear_area() callback for create grass */
staticfn void
grow_grass(coordxy x, coordxy y, genericptr_t arg)
{
    int *grasscnt = (int *) arg;
    struct trap *ttmp;

    /* Grow grass only on regular terrain or
       corridors, weighted towards spaces near
       the player */
    if (rn2(1 + distmin(u.ux, u.uy, x, y))
        || (levl[x][y].typ != ROOM
            && levl[x][y].typ != CORR))
        return;

    /* Magic portals and the vibrating square are
       permanent dungeon features; they cannot be
       buried. Every other trap type (pits, holes,
       bear traps, hidden arrow traps, etc.) is
       covered over by the new grass. The trap
       persists and still triggers when stepped on,
       the same way traps remain active beneath
       puddle or sewage terrain */
    ttmp = t_at(x, y);
    if (ttmp && undestroyable_trap(ttmp->ttyp))
        return;

    /* Sokoban pits, holes, and trap doors are puzzle
       features; their layout must remain readable, so
       never cover them with grass on Sokoban levels */
    if (ttmp && Sokoban
        && (is_pit(ttmp->ttyp) || is_hole(ttmp->ttyp)))
        return;

    /* grow grass; clear the rm-flags union since it aliases to a
       different field on the new typ */
    levl[x][y].typ = GRASS;
    levl[x][y].flags = 0;
    del_engr_at(x, y);
    newsym(x, y);
    (*grasscnt)++;
}

staticfn void
cast_create_grass(int skill)
{
    int bonus = (skill == P_EXPERT
                 ? 4 : skill == P_SKILLED
                     ? 2 : skill == P_BASIC
                         ? 1 : 0);
    int range = 1 + bonus;
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
    do_clear_area(u.ux, u.uy, range, grow_grass, (genericptr_t) &madegrass);

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

/* opaque arg for grow_tree() - carries both the grown-tile counter and
   the effective caster skill so the callback can honor skill-gated
   effects (dead-tree revival) without reaching back to P_SKILL() */
struct tree_growth {
    int count;
    int skill;
};

/* do_clear_area() callback for create trees */
staticfn void
grow_tree(coordxy x, coordxy y, genericptr_t arg)
{
    struct tree_growth *tg = (struct tree_growth *) arg;
    struct trap *ttmp;

    /* Never grow a tree on the player's space */
    if (u_at(x, y))
        return;

    /* Grow a tree only on regular terrain or grass,
       never on monsters, objects, or next to doors,
       weighted towards spaces near the player.

       Never next to other living trees as well. this
       is done to limit their growth, mainly to prevent
       a situation where the caster is suddenly trapped.

       Sometimes the energy of the spell can be used to
       potentially revive nearby dead trees if the
       caster is skilled enough */
    if (rn2(2) && levl[x][y].typ == DEADTREE
        && tg->skill >= P_SKILLED) {
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

    /* Permanent dungeon features and level-exit
       traps are never destroyed: magic portals
       and the vibrating square are un-uprootable,
       and holes and trap doors are various level's
       route to the next */
    ttmp = t_at(x, y);
    if (ttmp && (undestroyable_trap(ttmp->ttyp)
                 || is_hole(ttmp->ttyp)))
        return;

    /* Sokoban pits are puzzle features; never destroy
       them on Sokoban levels (holes and trap doors
       are already guarded above) */
    if (ttmp && Sokoban && is_pit(ttmp->ttyp))
        return;

    /* A tree is impassable, so any trap left under
       it would be unreachable. Tear it out cleanly
       instead of orphaning the data; ammo-bearing
       traps (bear trap, land mine) drop their ammo
       on the tile, where it will sit beneath the
       new tree until the tree is chopped down */
    if (ttmp)
        (void) deltrap_with_ammo(ttmp, DELTRAP_PLACE_AMMO);

    /* grow a tree; clear the rm-flags union since it aliases to a
       different field on the new typ */
    levl[x][y].typ = TREE;
    levl[x][y].flags = 0;
    del_engr_at(x, y);
    /* a tree blocks sight; tell vision right away rather than
       leaving it see-through until the level is reloaded */
    block_point(x, y);
    newsym(x, y);
    tg->count++;
}

staticfn void
cast_create_trees(int skill)
{
    int bonus = (skill == P_EXPERT
                 ? 4 : skill == P_SKILLED
                     ? 2 : skill == P_BASIC
                         ? 1 : 0);
    int range = 1 + bonus;
    struct tree_growth tg;

    tg.count = 0;
    tg.skill = skill;

    if (Is_valley(&u.uz)) {
        pline("Saplings begin to form, but they quickly wither away "
              "and die.");
        return;
    }

    /* creates a tree around the caster. at basic skill,
       a tree will grow out to two tile spaces away from the
       caster, skilled is three tile spaces out, expert is
       five tile spaces. at unskilled/restricted, a tree will
       only grow one tile space out */
    do_clear_area(u.ux, u.uy, range, grow_tree, (genericptr_t) &tg);

    if (tg.count) {
        if (Hallucination)
            pline("Only you can prevent forest fires...");
        else
            pline("Trees grow throughout the area!");
    } else {
        /* trees didn't grow anywhere */
        pline_The("ground briefly stirs, but nothing else happens.");
    }
}

/* power word kill: the caster picks one monster within a range set by
   attack spell skill and slays it if its hit points are at or below a
   skill-based limit, else maims it; magic resistance turns either into
   a nonlethal 8d6. range of power word kill spell (@ is center):
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
staticfn void
cast_power_word_kill(void)
{
    char c, qbuf[QBUFSZ];
    int ans, skill = P_SKILL(P_ATTACK_SPELL);
    int damage_skill = (skill >= P_EXPERT) ? 200
                       : (skill == P_SKILLED) ? 150
                         : (skill == P_BASIC) ? 100 : 50;
    int range_skill = (skill >= P_EXPERT) ? 17
                      : (skill == P_SKILLED) ? 7
                        : (skill == P_BASIC) ? 2 : 0;
    boolean save_verbose = flags.verbose,
            save_autodescribe = iflags.autodescribe;
    struct monst *mtmp = (struct monst *) 0;
    coord cc;

    cc.x = u.ux, cc.y = u.uy;
    You("utter a word of power...");
    pline("choose a monster to slay:");

    flags.verbose = FALSE;
    iflags.autodescribe = TRUE;
    ans = getpos(&cc, TRUE, "a monster");
    flags.verbose = save_verbose;
    iflags.autodescribe = save_autodescribe;
    if (ans < 0 || cc.x < 1)
        return;

    /* handle selecting your own location first */
    if (u_at(cc.x, cc.y)) {
        if (u.usteed) {
            Sprintf(qbuf, "Kill %.110s?", mon_nam(u.usteed));
            if ((c = ynq(qbuf)) == 'q')
                return;
            if (c == 'y')
                mtmp = u.usteed;
        }
        if (!mtmp) {
            Sprintf(qbuf, "%s?",
                    Role_if(PM_SAMURAI) ? "Perform seppuku"
                                        : "Commit suicide");
            if (paranoid_query(TRUE, qbuf)) {
                Sprintf(svk.killer.name, "%s own word of power", uhis());
                svk.killer.format = KILLED_BY;
                done(DIED);
            }
            return;
        }
    } else if (u.uswallow) {
        mtmp = (distu(cc.x, cc.y) <= 2) ? u.ustuck : (struct monst *) 0;
    } else {
        mtmp = m_at(cc.x, cc.y);
    }

    /* whether there's an unseen monster here or not, player will know
       that there's no monster here after the kill or failed attempt;
       let hero know too */
    (void) unmap_invisible(cc.x, cc.y);

    if (!mtmp) {
        There("is no monster there.");
        return;
    }
    if (mdistu(mtmp) > range_skill + 1) {
        pline("%s is out of range.", Monnam(mtmp));
        return;
    }
    if (immune_death_magic(mtmp->data)) {
        shieldeff(mtmp->mx, mtmp->my);
        pline("%s is immune to your word of power!", Monnam(mtmp));
        return;
    }

    if (resists_magm(mtmp) || defended(mtmp, AD_MAGM)) {
        /* magic resistance holds at any hit points; the target
           still takes damage, but not below one hit point */
        shieldeff(mtmp->mx, mtmp->my);
        pline("%s %s in pain, but resists your deadly spell.",
              Monnam(mtmp), makeplural(growl_sound(mtmp)));
        (void) damage_mon_nonlethal(mtmp, d(8, 6), AD_DETH, TRUE);
        death_magic_anger(mtmp);
    } else if (mtmp->mhp <= damage_skill) {
        /* assumes not resistant to death magic. monsters with
           less hit points as determined by damage_skill die */
        You("%s %s!", rn2(2) ? "annihilate" : "obliterate",
            mon_nam(mtmp));
        mtmp->mhp = 0;
        xkilled(mtmp, XKILL_NOMSG);
        /* a target that life-saved (or a vampire knocked back to its
           true form) survived the word and will not stay friendly;
           the Ice Queen's dogs submit instead. The hero's own steed is
           still ridden here: angering it throws the hero off, and one
           already hostile is thrown off by dismount_untamed_steed() */
        if (!DEADMONSTER(mtmp) && !is_defeated(mtmp->data))
            death_magic_anger(mtmp);
    } else {
        /* if not magic resistant, but has more hit points as
           determined by damage_skill, max hit points and regular
           hit points take a significant hit, not to go below one
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
        death_magic_anger(mtmp);
    }
}

/* attempting to cast a forgotten spell will cause disorientation */
staticfn void
spell_backfire(int spell)
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

staticfn boolean
spelleffects_check(int spell, int *res, int *energy)
{
    int chance;
    boolean confused = (Confusion != 0);

    *energy = 0;

    /*
     * Reject attempting to cast while stunned or with no free hands.
     * Already done in getspell() to stop casting before choosing
     * which spell, but duplicated here for cases where spelleffects()
     * gets called directly for ^T without intrinsic teleport capability
     * or #turn for non-priest/non-knight.
     * (There's no duplication of messages; when the rejection takes
     * place in getspell(), we don't get called.)
     */
    if ((spell == UNKNOWN_SPELL) || rejectcasting()) {
        *res = ECMD_OK; /* no time elapses */
        return TRUE;
    }

    /*
     *  Note: dotele() also calculates energy use and checks nutrition
     *  and strength requirements; if any of these change, update it too.
     */
    if (Role_if(PM_WIZARD) && spellid(spell) == SPE_FORCE_BOLT) {
        /* a wizard's force bolt costs only one point of power per cast
           up through basic attack spell skill, two at skilled and three
           at expert */
        int skill = P_SKILL(P_ATTACK_SPELL);

        *energy = spellev(spell)
                  * ((skill >= P_EXPERT) ? 3 : (skill == P_SKILLED) ? 2 : 1);
    } else {
        *energy = SPELL_LEV_PW(spellev(spell)); /* 5 <= energy <= 35 */
    }

    /*
     * Spell casting no longer affects knowledge of the spell. A
     * decrement of spell knowledge is done every turn.
     */
    if (spellknow(spell) <= 0) {
        if (spellid(spell) == SPE_PSIONIC_WAVE)
            You("have somehow lost your psychic ability!");
        else
            Your("knowledge of this spell is twisted.");
        pline("It invokes nightmarish images in your mind...");
        spell_backfire(spell);
        u.uen -= rnd(*energy);
        if (u.uen < 0)
            u.uen = 0;
        disp.botl = TRUE;
        *res = ECMD_TIME;
        return TRUE;
    /* an illithid that polymorphs into something that is not another
       illithid-type monster lacks the brain structure needed to use
       psionic abilities */
    } else if (spellid(spell) == SPE_PSIONIC_WAVE
               && !racial_illithid(&gy.youmonst)) {
        You("lack the psychic ability to use this power.");
        *res = ECMD_TIME;
        return TRUE;
    } else if (spellknow(spell) <= KEEN / 200) { /* 100 turns left */
        You("strain to recall the spell.");
    } else if (spellknow(spell) <= KEEN / 40) { /* 500 turns left */
        You("have difficulty remembering the spell.");
    } else if (spellknow(spell) <= KEEN / 20) { /* 1000 turns left */
        Your("knowledge of this spell is growing faint.");
    } else if (spellknow(spell) <= KEEN / 10) { /* 2000 turns left */
        Your("recall of this spell is gradually fading.");
    }

    if (u.uhunger <= 10 && spellid(spell) != SPE_DETECT_FOOD) {
        You("are too hungry to cast that spell.");
        *res = ECMD_OK;
        return TRUE;
    } else if (ACURR(A_STR) < 4 && spellid(spell) != SPE_RESTORE_ABILITY) {
        You("lack the strength to cast spells.");
        *res = ECMD_OK;
        return TRUE;
    } else if (check_capacity(
                "Your concentration falters while carrying so much stuff.")) {
        *res = ECMD_TIME;
        return TRUE;
    }

    /* if the cast attempt is already going to fail due to insufficient
       energy (ie, u.uen < energy), the Amulet's drain effect won't kick
       in and no turn will be consumed; however, when it does kick in,
       the attempt may fail due to lack of energy after the draining, in
       which case a turn will be used up in addition to the energy loss */
    if (u.uhave.amulet && u.uen >= *energy) {
        /* psionic wave is an innate psychic ability rather than a
           learned spell, so the Amulet does not drain it.

           In regards to Infidels: once the Amulet has been sacrificed
           to Moloch and their quest artifact is imbued with its power,
           Moloch's influence suppresses the spell power draining effect
           and allows the Infidel to realize their full potential */
        if (spellid(spell) != SPE_PSIONIC_WAVE
            && !(Role_if(PM_INFIDEL) && u.uevent.uimbued_idol)) {
            You_feel("the amulet draining your energy away.");
            /* this used to be 'energy += rnd(2 * energy)' (without
               'res'), so if amulet-induced cost was more than u.uen,
               nothing (except the "don't have enough energy" message)
               happened and player could just try again (and again and
               again...); now we drain some energy immediately, which
               has a side-effect of not increasing the hunger aspect
               of casting */
            u.uen -= rnd(2 * *energy);
            if (u.uen < 0)
                u.uen = 0;
            disp.botl = TRUE;
        }
        *res = ECMD_TIME; /* time is used even if spell doesn't get cast */
    }

    if (*energy > u.uen) {
        /*
         * Hero has insufficient energy/power to cast the spell.
         * Augment the message when current energy is at maximum.
         * "yet": mainly for level 1 characters who already know a spell
         * but don't start with enough energy to cast it.
         * "anymore": maximum energy was high enough at some point but
         * isn't now (lost energy when losing levels or polymorphing into
         * new person or had some stripped away by traps or monsters).
         */
        if (spellid(spell) == SPE_PSIONIC_WAVE)
            Your("mind is fatigued.  You cannot use your psychic energy.");
        else
            You("don't have enough energy to cast that spell%s.",
                (u.uen < u.uenmax) ? "" /* not at full energy => normal */
                : (*energy > u.uenpeak) ? " yet" /* never had enough */
                  : " anymore"); /* once had enough but lost some since */
        return TRUE;
    } else {
        if (spellid(spell) != SPE_DETECT_FOOD) {
            int hungr = *energy * 2;

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
            int intell = acurr(A_INT);
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

    chance = percent_success(spell);
    if (confused || (rnd(100) > chance)) {
        if (spellid(spell) == SPE_PSIONIC_WAVE)
            You("are too confused to use your psychic abilities.");
        else
            You("fail to cast the spell correctly.");
        /* an Infidel without the Amulet's power at hand burns life
           force instead of spell power */
        if (u.ualign.type == A_NONE && !u.uhave.amulet
            && !u.uevent.uimbued_idol
            && spellid(spell) != SPE_PSIONIC_WAVE) {
            Sprintf(svk.killer.name, "draining %s own life force",
                    uhis());
            losehp(*energy / 2, svk.killer.name, KILLED_BY);
        } else {
            u.uen -= *energy / 2;
        }
        disp.botl = TRUE;
        *res = ECMD_TIME;
        return TRUE;
    }
    return FALSE;
}

/* hero casts a spell of type spell_otyp, eg. SPE_SLEEP.
   hero must know the spell (unless force is TRUE). */
int
spelleffects(int spell_otyp, boolean atme, boolean force)
{
    int spell = force ? spell_otyp : spell_idx(spell_otyp);
    int energy = 0, damage, n;
    int otyp, skill, role_skill, res = ECMD_OK;
    boolean physical_damage = FALSE;
    struct obj *pseudo;
    struct monst *mtmp;
    coord cc;

    if (!force && spelleffects_check(spell, &res, &energy))
        return res;

    /* Infidels attempting to cast spells without having
       the Amulet of Yendor in their possession (before
       the Idol of Moloch is imbued) costs them hit points
       instead of spell power */
    if (!force && u.ualign.type == A_NONE && !u.uhave.amulet
        && !u.uevent.uimbued_idol
        /* psychic attack uses spell power but
           technically is not considered a spell */
        && spellid(spell) != SPE_PSIONIC_WAVE) {
        You("draw upon your own life force to cast the spell.");
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
        Sprintf(svk.killer.name, "draining %s own life force", uhis());
        if (spellid(spell) == SPE_HEALING) {
            losehp(6 * energy, svk.killer.name, KILLED_BY);
        } else if (spellid(spell) == SPE_EXTRA_HEALING) {
            losehp(4 * energy, svk.killer.name, KILLED_BY);
        } else if (spellid(spell) == SPE_CRITICAL_HEALING) {
            losehp(3 * energy, svk.killer.name, KILLED_BY);
        } else {
            losehp(energy, svk.killer.name, KILLED_BY);
        }
    } else {
        u.uen -= energy;
    }

    /* a successful cast refreshes the memory of the spell: with Int 7
       or more it adds Int * 3 to Int * 13 - 1 turns, up to KEEN;
       Cavepersons get no such help. A forced cast is skipped, since
       then spell is an object type rather than a spl_book[] index */
    if (!force && !Role_if(PM_CAVE_DWELLER) && ACURR(A_INT) > 6) {
        int intel = ACURR(A_INT);

        svs.spl_book[spell].sp_know += rn1(intel * 10, intel * 3);
        if (svs.spl_book[spell].sp_know > KEEN)
            svs.spl_book[spell].sp_know = KEEN;
    }

    disp.botl = TRUE;
    exercise(A_WIS, TRUE);
    /* pseudo is a temporary "false" object containing the spell stats */
    pseudo = mksobj(force ? spell : spellid(spell), FALSE, FALSE);
    pseudo->blessed = pseudo->cursed = 0;
    pseudo->quan = 20L; /* do not let useup get it */
    /*
     * Find the skill the hero has in a spell type category.
     * See spell_skilltype for categories.
     */
    otyp = pseudo->otyp;
    skill = spell_skilltype(otyp);
    role_skill = P_SKILL(skill);

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
        if (role_skill >= P_SKILLED && y_n("Cast advanced spell?") == 'y') {
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
                                BZ_U_SPELL(BZ_OFS_SPE(otyp)),
                                spell_damage_bonus(u.ulevel / 2 + 1), 0,
                                (otyp == SPE_CONE_OF_COLD)
                                   ? EXPL_FROSTY
                                   : (otyp == SPE_LIGHTNING)
                                      ? EXPL_SHOCK
                                      : (otyp == SPE_POISON_BLAST)
                                         ? EXPL_NOXIOUS
                                         : (otyp == SPE_ACID_BLAST)
                                            ? EXPL_ACID
                                            : EXPL_FIERY);
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
            /* player said not to cast advanced spell; return up to half
               of the magical energy, unless the cast was forced or the
               cost was paid as hit points instead of spell power (keep
               this test in sync with the life force arm above) */
            if (!force
                && !(u.ualign.type == A_NONE && !u.uhave.amulet
                     && !u.uevent.uimbued_idol)) {
                u.uen += rnd(energy / 2);
                disp.botl = TRUE;
            }
        }
        FALLTHROUGH;
        /*FALLTHRU*/

    /* these spells are all duplicates of wand effects */
    case SPE_FORCE_BOLT:
        physical_damage = TRUE;
        FALLTHROUGH;
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
    case SPE_ENTANGLE:
    case SPE_PSIONIC_WAVE:
    case SPE_CHANGE_METAL_TO_WOOD:
        if (objects[otyp].oc_dir != NODIR) {
            if (otyp == SPE_HEALING || otyp == SPE_EXTRA_HEALING
                || otyp == SPE_CRITICAL_HEALING
                || otyp == SPE_RESTORE_ABILITY) {
                /* the healing spells and restore ability work like
                   potion effects, but they've been extended to take a
                   direction like wands */
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
        /* at expert skill (clerical), offers a selective uncurse
           menu so that the player can keep specific items cursed
           (e.g. an Infidel keeping cursed artifact gear) instead
           of having every cursed item auto-uncursed */
        if (role_skill >= P_EXPERT && !Confusion
            && y_n("Cast remove curse selectively?") == 'y') {
            selective_remove_curse();
            break;
        }
        FALLTHROUGH;
    /*FALLTHRU*/
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
        FALLTHROUGH;
    /*FALLTHRU*/
    case SPE_MAGIC_MAPPING:
    case SPE_CREATE_MONSTER:
        (void) seffects(pseudo);
        break;

    case SPE_ORB_OF_FIRE:
    case SPE_ORB_OF_FROST:
        You("conjure elemental energy...");
        for (n = 0; n < max(role_skill - 1, 1); n++) {
            mtmp = make_helper((pseudo->otyp == SPE_ORB_OF_FIRE)
                                   ? PM_ORB_OF_FIRE : PM_ORB_OF_FROST,
                               u.ux, u.uy);
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
    case SPE_POWER_WORD_KILL:
        cast_power_word_kill();
        break;

    /* these are all duplicates of potion effects */
    case SPE_HASTE_SELF:
    case SPE_DETECT_TREASURE:
    case SPE_DETECT_MONSTERS:
    case SPE_LEVITATION:
        /* high skill yields effect equivalent to blessed potion */
        if (role_skill >= P_SKILLED)
            pseudo->blessed = 1;
        FALLTHROUGH;
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
        cast_protection(role_skill);
        break;
    case SPE_REFLECTION:
        cast_reflection(&gy.youmonst, role_skill);
        break;
    case SPE_BARKSKIN:
        cast_barkskin(&gy.youmonst, role_skill);
        break;
    case SPE_STONESKIN:
        cast_stoneskin(&gy.youmonst, role_skill);
        break;
    case SPE_CREATE_GRASS:
        cast_create_grass(role_skill);
        break;
    case SPE_CREATE_TREES:
        cast_create_trees(role_skill);
        break;
    case SPE_JUMPING:
        if (!(jump(max(role_skill, 1)) & ECMD_TIME))
            pline1(nothing_happens);
        break;
    case SPE_REPAIR_ARMOR: {
        /* removes one level of erosion (both types) for a chosen
           piece of armor */
        struct obj *otmp = (struct obj *) 0;

        if (role_skill >= P_BASIC) {
            otmp = choose_worn_armor("magically repair", "repair");
        } else {
            /* unskilled cast picks armor at random; skip Hand of
               Vecna since the cursed Hand can't be repaired by the
               spell. some_armor() is deterministic when only one
               slot is filled, so bound the retry to avoid an
               infinite loop when the Hand is the player's only armor */
            int tryct = 50;

            otmp = some_armor(&gy.youmonst);
            while (otmp && otmp->oartifact == ART_HAND_OF_VECNA
                   && --tryct > 0)
                otmp = some_armor(&gy.youmonst);
            if (otmp && otmp->oartifact == ART_HAND_OF_VECNA)
                otmp = (struct obj *) 0;
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
            /* the player can probably feel this, so no need for a
               !Blind check :) */
            Your("embarrassing skin rash clears up slightly.");
        }
        break;
    }
    case SPE_CHAIN_LIGHTNING:
        cast_chain_lightning();
        break;
    default:
        impossible("Unknown spell %d attempted.", spell);
        obfree(pseudo, (struct obj *) 0);
        return ECMD_OK;
    }

    /* gain skill for successful cast */
    if (!force)
        use_skill(skill, spellev(spell));

    obfree(pseudo, (struct obj *) 0); /* now, get rid of it */
    return ECMD_TIME;
}

/*ARGSUSED*/
staticfn boolean
spell_aim_step(genericptr_t arg UNUSED, coordxy x, coordxy y)
{
    if (!isok(x,y))
        return FALSE;
    if (!ZAP_POS(levl[x][y].typ)
        && !(IS_DOOR(levl[x][y].typ) && (levl[x][y].doormask & D_ISOPEN)))
        return FALSE;
    return TRUE;
}

/* not quite the same as throwspell limits, but close enough */
staticfn boolean
can_center_spell_location(coordxy x, coordxy y)
{
    if (distmin(u.ux, u.uy, x, y) > 10)
        return FALSE;
    return (isok(x, y) && cansee(x, y) && !(IS_STWALL(levl[x][y].typ)));
}

staticfn void
display_spell_target_positions(boolean on_off)
{
    coordxy x, y, dx, dy;
    int dist = 10;

    if (on_off) {
        /* on */
        tmp_at(DISP_BEAM, cmap_to_glyph(S_goodpos));
        for (dx = -dist; dx <= dist; dx++)
            for (dy = -dist; dy <= dist; dy++) {
                x = u.ux + dx;
                y = u.uy + dy;
                /* hero's location is allowed but highlighting the hero's
                   spot makes map harder to read (if using '$' rather than
                   by changing background color) */
                if (u_at(x, y))
                    continue;
                if (can_center_spell_location(x, y))
                    tmp_at(x, y);
            }
    } else {
        /* off */
        tmp_at(DISP_END, 0);
    }
}

/* Choose location where spell takes effect. */
staticfn int
throwspell(void)
{
    coord cc, uc;
    struct monst *mtmp;

    if (u.uinwater) {
        pline("You're joking!  In this weather?");
        return 0;
    } else if (Is_waterlevel(&u.uz)) {
        You("had better wait for the sun to come out.");
        return 0;
    }

    pline("Where do you want to cast the spell?");
    cc.x = u.ux;
    cc.y = u.uy;
    getpos_sethilite(display_spell_target_positions,
                     can_center_spell_location);
    if (getpos(&cc, TRUE, "the desired position") < 0)
        return 0; /* user pressed ESC */
    clear_nhwindow(WIN_MESSAGE); /* discard any autodescribe feedback */

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
    } else if (((cc.x != u.ux || cc.y != u.uy) && !cansee(cc.x, cc.y)
                && (!(mtmp = m_at(cc.x, cc.y)) || !canspotmon(mtmp)))
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
tport_spell(int what)
{
    static struct tport_hideaway {
        struct spell savespell;
        int tport_indx;
    } save_tport;
    static boolean inited = FALSE;
    int i;
/* also defined in teleport.c */
#define NOOP_SPELL  0
#define HIDE_SPELL  1
#define ADD_SPELL   2
#define UNHIDESPELL 3
#define REMOVESPELL 4

    /* save_tport is zero-initialized by C, which leaves tport_indx
       at 0 -- a valid spl_book[] index. If REMOVESPELL or UNHIDESPELL
       were ever the first call (no prior ADD/HIDE), the saved-zero
       savespell would be written to spl_book[0], wiping the player's
       first known spell. Set tport_indx out of range on first use so
       the defensive guards below trip */
    if (!inited) {
        save_tport.tport_indx = MAXSPELL;
        inited = TRUE;
    }

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
            if (save_tport.tport_indx >= MAXSPELL)
                return NOOP_SPELL;
            svs.spl_book[save_tport.tport_indx] = save_tport.savespell;
            save_tport.tport_indx = MAXSPELL; /* burn bridge... */
        } else if (what == ADD_SPELL) {
            save_tport.savespell = svs.spl_book[i];
            save_tport.tport_indx = i;
            svs.spl_book[i].sp_id = SPE_TELEPORT_AWAY;
            svs.spl_book[i].sp_lev = objects[SPE_TELEPORT_AWAY].oc_level;
            svs.spl_book[i].sp_know = KEEN;
            return REMOVESPELL; /* operation needed to reverse */
        }
    } else { /* spellid(i) == SPE_TELEPORT_AWAY */
        if (what == ADD_SPELL || what == UNHIDESPELL) {
            save_tport.tport_indx = MAXSPELL;
        } else if (what == REMOVESPELL) {
            /*assert( i == save_tport.tport_indx );*/
            if (save_tport.tport_indx >= MAXSPELL)
                return NOOP_SPELL;
            svs.spl_book[i] = save_tport.savespell;
            save_tport.tport_indx = MAXSPELL;
        } else if (what == HIDE_SPELL) {
            save_tport.savespell = svs.spl_book[i];
            save_tport.tport_indx = i;
            svs.spl_book[i].sp_id = NO_SPELL;
            return UNHIDESPELL; /* operation needed to reverse */
        }
    }
    return NOOP_SPELL;
}

/* forget a random selection of known spells due to amnesia;
   they used to be lost entirely, as if never learned, but now we
   just set the memory retention to zero so that they can't be cast */
void
losespells(void)
{
    int n, nzap, i;

    /* in case reading has been interrupted earlier, discard context */
    svc.context.spbook.book = 0;
    svc.context.spbook.o_id = 0;
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

static const char *const spl_sortchoices[NUM_SPELL_SORTBY] = {
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

/* qsort callback routine */
staticfn int QSORTCALLBACK
spell_cmp(const genericptr vptr1, const genericptr vptr2)
{
    /*
     * gather up all of the possible parameters except spell name
     * in advance, even though some might not be needed:
     *  indx. = spl_orderindx[] index into svs.spl_book[];
     *  otyp. = svs.spl_book[] index into objects[];
     *  levl. = spell level;
     *  skil. = skill group aka spell class.
     */
    int indx1 = *(int *) vptr1, indx2 = *(int *) vptr2,
        otyp1 = svs.spl_book[indx1].sp_id, otyp2 = svs.spl_book[indx2].sp_id,
        levl1 = objects[otyp1].oc_level, levl2 = objects[otyp2].oc_level,
        skil1 = objects[otyp1].oc_skill, skil2 = objects[otyp2].oc_skill;

    switch (gs.spl_sortmode) {
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
staticfn void
sortspells(void)
{
    int i;
#if defined(SYSV) || defined(DGUX)
    unsigned n;
#else
    int n;
#endif

    if (gs.spl_sortmode == SORTBY_CURRENT)
        return;
    for (n = 0; n < MAXSPELL && spellid(n) != NO_SPELL; ++n)
        continue;
    if (n < 2)
        return; /* not enough entries to need sorting */

    if (!gs.spl_orderindx) {
        /* we haven't done any sorting yet; list is in casting order */
        if (gs.spl_sortmode == SORTBY_LETTER /* default */
            || gs.spl_sortmode == SORTRETAINORDER)
            return;
        /* allocate enough for full spellbook rather than just N spells */
        gs.spl_orderindx = (int *) alloc(MAXSPELL * sizeof(int));
        for (i = 0; i < MAXSPELL; i++)
            gs.spl_orderindx[i] = i;
    }

    if (gs.spl_sortmode == SORTRETAINORDER) {
        struct spell tmp_book[MAXSPELL];

        /* sort svs.spl_book[] rather than spl_orderindx[];
           this also updates the index to reflect the new ordering (we
           could just free it since that ordering becomes the default) */
        for (i = 0; i < MAXSPELL; i++)
            tmp_book[i] = svs.spl_book[gs.spl_orderindx[i]];
        for (i = 0; i < MAXSPELL; i++)
            svs.spl_book[i] = tmp_book[i], gs.spl_orderindx[i] = i;
        gs.spl_sortmode = SORTBY_LETTER; /* reset */
        return;
    }

    /* usual case, sort the index rather than the spells themselves */
    qsort((genericptr_t) gs.spl_orderindx, n,
          sizeof *gs.spl_orderindx, spell_cmp);
    return;
}

/* called if the [sort spells] entry in the view spells menu gets chosen */
staticfn boolean
spellsortmenu(void)
{
    winid tmpwin;
    menu_item *selected;
    anything any;
    char let;
    int i, n, choice;
    int clr = NO_COLOR;

    tmpwin = create_nhwindow(NHW_MENU);
    start_menu(tmpwin, MENU_BEHAVE_STANDARD);
    any = cg.zeroany; /* zero out all bits */

    for (i = 0; i < SIZE(spl_sortchoices); i++) {
        if (i == SORTRETAINORDER) {
            let = 'z'; /* assumes fewer than 26 sort choices... */
            /* separate final choice from others with a blank line */
            add_menu_str(tmpwin, "");
        } else {
            let = 'a' + i;
        }
        any.a_int = i + 1;
        add_menu(tmpwin, &nul_glyphinfo, &any, let, 0,
                 ATR_NONE, clr, spl_sortchoices[i],
                 (i == gs.spl_sortmode) ? MENU_ITEMFLAGS_SELECTED
                                        : MENU_ITEMFLAGS_NONE);
    }
    end_menu(tmpwin, "View known spells list sorted");

    n = select_menu(tmpwin, PICK_ONE, &selected);
    destroy_nhwindow(tmpwin);
    if (n > 0) {
        choice = selected[0].item.a_int - 1;
        /* skip preselected entry if we have more than one item chosen */
        if (n > 1 && choice == gs.spl_sortmode)
            choice = selected[1].item.a_int - 1;
        free((genericptr_t) selected);
        gs.spl_sortmode = choice;
        return TRUE;
    }
    return FALSE;
}

/* the #showspells command -- view known spells */
int
dovspell(void)
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

                spl_tmp = svs.spl_book[splnum];
                svs.spl_book[splnum] = svs.spl_book[othnum];
                svs.spl_book[othnum] = spl_tmp;
            }
        }
    }
    if (gs.spl_orderindx) {
        free((genericptr_t) gs.spl_orderindx);
        gs.spl_orderindx = 0;
    }
    gs.spl_sortmode = SORTBY_LETTER; /* 0 */
    return ECMD_OK;
}

DISABLE_WARNING_FORMAT_NONLITERAL

/* lists spells for endgame dumplog purposes */
void
show_spells(void)
{
    int unused = SPELLMENU_DUMP;
    putstr(0, ATR_HEADING, "Spells:");
    if (spellid(0) == NO_SPELL) {
        pline("You didn't know any spells.");
        pline("%s", "");
    } else {
        nhUse(dospellmenu("", SPELLMENU_DUMP, &unused));
    }
}

/* shows menu of known spells, with options to sort them.
   return FALSE on cancel, TRUE otherwise.
   spell_no is set to the internal spl_book index, if any selected */
staticfn boolean
dospellmenu(
    const char *prompt,
    int splaction, /* SPELLMENU_CAST, SPELLMENU_VIEW, SPELLMENU_DUMP or
                    * svs.spl_book[] index */
    int *spell_no)
{
    winid tmpwin;
    int i, n, how, splnum;
    char buf[BUFSZ], retentionbuf[24], sep;
    const char *fmt;
    menu_item *selected;
    anything any;
    int clr = NO_COLOR;

    tmpwin = create_nhwindow(NHW_MENU);
    start_menu(tmpwin, MENU_BEHAVE_STANDARD);
    any = cg.zeroany; /* zero out all bits */

    /*
     * The correct spacing of the columns when not using
     * tab separation depends on the following:
     * (1) that the font is monospaced, and
     * (2) that selection letters are prepended to the
     *     given string and are of the form "a - ".
     * For SPELLMENU_DUMP, (2) is untrue, so four spaces
     * need to be subtracted.
     */
    if (!iflags.menu_tab_sep) {
        Sprintf(buf, "%s%-20s Level %-12s Fail Retention",
                splaction == SPELLMENU_DUMP ? "" : "    ",
                "Name",
                "Category");
        fmt = "%-20s  %2d   %-12s %3d%% %9s";
        sep = ' ';
    } else {
        Sprintf(buf, "Name\tLevel\tCategory\tFail\tRetention");
        fmt = "%s\t%-d\t%s\t%-d%%\t%s";
        sep = '\t';
    }
    if (wizard)
        Sprintf(eos(buf), "%c%6s", sep, "turns");

    if (splaction == SPELLMENU_DUMP)
        /* html dumplogs: add_menu_heading does not let us use ATR_PREFORM */
        putstr(tmpwin, ATR_PREFORM, buf);
    else
        add_menu_heading(tmpwin, buf);

    for (i = 0; i < MAXSPELL && spellid(i) != NO_SPELL; i++) {
        splnum = !gs.spl_orderindx ? i : gs.spl_orderindx[i];
        Sprintf(buf, fmt, spellname(splnum), spellev(splnum),
                spelltypemnemonic(spell_skilltype(spellid(splnum))),
                100 - percent_success(splnum),
                spellretention(splnum, retentionbuf));
        if (wizard)
            Sprintf(eos(buf), "%c%6d", sep, spellknow(i));

        any.a_int = splnum + 1; /* must be non-zero */
        add_menu(tmpwin, &nul_glyphinfo, &any, spellet(splnum), 0,
                 (splaction == SPELLMENU_DUMP ? ATR_PREFORM : ATR_NONE),
                 clr, buf,
                 (splnum == splaction)
                    ? MENU_ITEMFLAGS_SELECTED : MENU_ITEMFLAGS_NONE);
    }
    how = PICK_ONE;
    if (splaction == SPELLMENU_VIEW) {
        if (spellid(1) == NO_SPELL) {
            /* only one spell => nothing to swap with */
            how = PICK_NONE;
        } else {
            /* more than 1 spell, add an extra menu entry */
            any.a_int = SPELLMENU_SORT + 1;
            add_menu(tmpwin, &nul_glyphinfo, &any, '+', 0,
                     ATR_NONE, clr, "[sort spells]", MENU_ITEMFLAGS_NONE);
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

RESTORE_WARNING_FORMAT_NONLITERAL

/* the roles trained as spellcasters; every other role counts as a
   non-caster for the body armor penalty in percent_success() */
boolean
primary_caster_role(void)
{
    return (Role_if(PM_HEALER) || Role_if(PM_CLERIC) || Role_if(PM_WIZARD)
            || Role_if(PM_INFIDEL) || Role_if(PM_DRUID));
}

/* a Druid can neither cast nor use wildshape while wearing any metallic
   armor; cloaks and shirts are checked too in case one is ever made of
   metal */
boolean
druid_metal_blocked(void)
{
    return (Role_if(PM_DRUID)
            && ((uarm && is_metallic(uarm)) || (uarmc && is_metallic(uarmc))
                || (uarmh && is_metallic(uarmh))
                || (uarms && is_metallic(uarms))
                || (uarmg && is_metallic(uarmg))
                || (uarmf && is_metallic(uarmf))
                || (uarmu && is_metallic(uarmu))));
}

/* the spell school a wielded staff aids, P_NONE if none (an ashwood
   staff aids every school; percent_success() tests that itself) */
staticfn int
staff_spell_school(int otyp)
{
    switch (otyp) {
    case STAFF_OF_DIVINATION:
        return P_DIVINATION_SPELL;
    case STAFF_OF_HEALING:
        return P_HEALING_SPELL;
    case STAFF_OF_HOLINESS:
        return P_CLERIC_SPELL;
    case STAFF_OF_MATTER:
        return P_MATTER_SPELL;
    case STAFF_OF_ESCAPE:
        return P_ESCAPE_SPELL;
    case STAFF_OF_WAR:
        return P_ATTACK_SPELL;
    case STAFF_OF_EVOCATION:
        return P_EVOCATION_SPELL;
    default:
        return P_NONE;
    }
}

staticfn int
percent_success(int spell)
{
    /* Intrinsic and learned ability are combined to calculate
     * the probability of player's success at casting a given spell.
     */
    int chance, splcaster, special, statused;
    int difficulty;
    int skill, skilltype = spell_skilltype(spellid(spell));
    int dex = ACURR(A_DEX), dex_adjust;
    /* Knights skip the metal armor term for clerical spells (the body
       armor penalty below still applies to them) */
    boolean paladin_bonus = (Role_if(PM_KNIGHT)
                             && skilltype == P_CLERIC_SPELL);

    /* Calculate intrinsic ability (splcaster) */

    splcaster = gu.urole.spelbase;
    special = gu.urole.spelheal;
    statused = ACURR(gu.urole.spelstat);

    /* dexterity offsets the casting penalty, for better or worse */
    if (dex <= 6)
        dex_adjust = -10;
    else if (dex <= 9)
        dex_adjust = -5;
    else if (dex <= 15)
        dex_adjust = 0;
    else if (dex <= 18)
        dex_adjust = 5;
    else if (dex <= 20)
        dex_adjust = 10;
    else if (dex <= 23)
        dex_adjust = 15;
    else
        dex_adjust = 20;

    if (uarm && is_metallic(uarm) && !paladin_bonus)
        splcaster += (uarmc && uarmc->otyp == ROBE) ? gu.urole.spelarmr / 2
                                                    : gu.urole.spelarmr;
    else if (uarmc && uarmc->otyp == ROBE)
        splcaster -= gu.urole.spelarmr;
    /* the Druid quest artifact occupies the shield slot but casts
       as though nothing were there */
    if (uarms && !(uarms->oartifact == ART_BRACERS_OF_THE_FIRST_CIRCLE))
        splcaster += gu.urole.spelshld;

    if (!paladin_bonus) {
        if (uarmh && is_metallic(uarmh)) /* && otyp != HELM_OF_BRILLIANCE */
            splcaster += uarmhbon;
        /* the Gauntlets of Purity don't hinder casting */
        if (uarmg && is_metallic(uarmg)
            && uarmg->oartifact != ART_GAUNTLETS_OF_PURITY)
            splcaster += uarmgbon;
        if (uarmf && is_metallic(uarmf))
            splcaster += uarmfbon;
    }

    /* Infidels have a special penalty for wearing blessed armor */
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

    if (spellid(spell) == gu.urole.spelspec)
        splcaster += gu.urole.spelsbon;

    /* `healing spell' bonus */
    if (spellid(spell) == SPE_HEALING || spellid(spell) == SPE_EXTRA_HEALING
        || spellid(spell) == SPE_CRITICAL_HEALING
        || spellid(spell) == SPE_CURE_BLINDNESS
        || spellid(spell) == SPE_CURE_SICKNESS
        || spellid(spell) == SPE_RESTORE_ABILITY
        || spellid(spell) == SPE_REMOVE_CURSE)
        splcaster += special;

    if (splcaster > 20)
        splcaster = 20;

    /* Calculate learned ability */

    /* The player's basic likelihood of being able to cast any spell
     * is based of their `magic' statistic. (Int or Wis)
     */
    chance = 11 * statused / 2;

    /*
     * High-level spells are harder.  Easier for higher-level casters.
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
     * player's role-specific spell. A metallic shield hinders no
     * matter how light it is, though bracers are exempt from that.
     */
    if (uarms && ((is_metallic(uarms) && !is_bracer(uarms))
                  || weight(uarms) > (int) objects[SMALL_SHIELD].oc_weight)) {
        if (spellid(spell) == gu.urole.spelspec) {
            chance /= 2;
        } else {
            chance /= 4;
        }
    }

    /* Finally, chance (based on player intell/wisdom and level) is
     * combined with ability (based on player intrinsics and
     * encumbrances), and dexterity adjusts the result. No matter
     * how intelligent/wise and advanced a player is, intrinsics and
     * encumbrance can prevent casting; and no matter how able,
     * learning is always required.
     */
    chance = chance * (20 - splcaster) / 15 - splcaster + dex_adjust;

    /* wielding the staff of the spell's school is a big help, which
       matters most to roles that don't cast well */
    if (uwep && (uwep->otyp == ASHWOOD_STAFF
                 || (skilltype != P_NONE
                     && staff_spell_school(uwep->otyp) == skilltype)))
        chance += STAFF_BONUS;

    /* Body armor other than crystal plate mail hinders casting: a role
     * that isn't a primary caster loses 10% per spell level, a primary
     * caster 10% per spell level above 3. The terms above (Int or Wis,
     * experience level, skill, a robe, dexterity, a school staff) can
     * offset it.
     */
    if (uarm && uarm->otyp != CRYSTAL_PLATE_MAIL) {
        int splev = spellev(spell);

        if (!primary_caster_role())
            chance -= splev * 10;
        else if (splev > 3)
            chance -= (splev - 3) * 10;
    }

    /* Clamp to percentile */
    if (chance > 100)
        chance = 100;
    if (chance < 0)
        chance = 0;

    /* as an illithid, you can always use your natural inherent
       psychic ability */
    if (spellid(spell) == SPE_PSIONIC_WAVE && Race_if(PM_ILLITHID))
        chance = 100;

    /* Druids always fail while wearing any metallic armor */
    if (druid_metal_blocked())
        chance = 0;

    return chance;
}

staticfn char *
spellretention(int idx, char * outbuf)
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
        accuracy = (skill == P_EXPERT) ? 2L
                   : (skill == P_SKILLED) ? 5L
                     : (skill == P_BASIC) ? 10L
                       : 25L;
        /* round up to the high end of this range */
        percent = accuracy * ((percent - 1L) / accuracy + 1L);
        Sprintf(outbuf, "%ld%%-%ld%%", percent - accuracy + 1L, percent);
    }
    return outbuf;
}

/* Learn a spell during creation of the initial inventory */
void
initialspell(struct obj *obj)
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
        svs.spl_book[i].sp_id = otyp;
        svs.spl_book[i].sp_lev = objects[otyp].oc_level;
        incrnknow(i, 0);
    }
    return;
}

/* returns one of spe_Unknown, spe_Fresh, spe_GoingStale, spe_Forgotten */
int
known_spell(short otyp)
{
    int i, k;

    for (i = 0; (i < MAXSPELL) && (spellid(i) != NO_SPELL); i++)
        if (spellid(i) == otyp) {
            k = spellknow(i);
            return (k > KEEN / 10) ? spe_Fresh
                   : (k > 0) ? spe_GoingStale
                     : spe_Forgotten;
        }
    return spe_Unknown;
}

/* return index for spell otyp, or UNKNOWN_SPELL if not found */
int
spell_idx(short otyp)
{
    int i;

    for (i = 0; (i < MAXSPELL) && (spellid(i) != NO_SPELL); i++)
        if (spellid(i) == otyp)
            return i;
    return UNKNOWN_SPELL;
}

/* learn or refresh spell otyp, if feasible; return casting letter or '\0' */
char
force_learn_spell(short otyp)
{
    int i;

    if (otyp == SPE_BLANK_PAPER || otyp == SPE_BOOK_OF_THE_DEAD
        || known_spell(otyp) == spe_Fresh)
        return '\0';

    for (i = 0; i < MAXSPELL; i++)
        if (spellid(i) == NO_SPELL || spellid(i) == otyp)
            break;
    if (i == MAXSPELL) {
        impossible("Too many spells memorized");
        return '\0';
    }
    /* for a going-stale or forgotten spell the sp_id and sp_lev assignments
       are redundant but harmless; for an unknown spell, they're essential */
    svs.spl_book[i].sp_id = otyp;
    svs.spl_book[i].sp_lev = objects[otyp].oc_level;
    incrnknow(i, 0); /* set spl_book[i].sp_know to KEEN; unlike when learning
                      * a spell by reading its book, we don't need to add 1 */
    return spellet(i);
}

/* number of spells hero knows */
int
num_spells(void)
{
    int i;

    for (i = 0; i < MAXSPELL; i++)
        if (spellid(i) == NO_SPELL)
            break;
    return i;
}

/*spell.c*/
