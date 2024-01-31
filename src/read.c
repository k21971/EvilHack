/* NetHack 3.6	read.c	$NHDT-Date: 1561485713 2019/06/25 18:01:53 $  $NHDT-Branch: NetHack-3.6 $:$NHDT-Revision: 1.172 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2012. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#define Your_Own_Role(mndx)  \
    ((mndx) == urole.malenum \
     || (urole.femalenum != NON_PM && (mndx) == urole.femalenum))
#define Your_Own_Race(mndx)  \
    ((mndx) == urace.malenum \
     || (urace.femalenum != NON_PM && (mndx) == urace.femalenum))

boolean known;

static NEARDATA const char readable[] = { ALL_CLASSES, SCROLL_CLASS,
                                          SPBOOK_CLASS, 0 };
static const char all_count[] = { ALLOW_COUNT, ALL_CLASSES, 0 };
static const char clothes[] = { ARMOR_CLASS, 0 };

STATIC_DCL boolean FDECL(learnscrolltyp, (SHORT_P));
STATIC_DCL char *FDECL(erode_obj_text, (struct obj *, char *));
STATIC_DCL char *FDECL(hawaiian_design, (struct obj *, char *));
STATIC_DCL void FDECL(stripspe, (struct obj *));
STATIC_DCL void FDECL(p_glow1, (struct obj *));
STATIC_DCL void FDECL(p_glow2, (struct obj *, const char *));
STATIC_DCL void FDECL(mp_glow1, (struct monst *, struct obj *));
STATIC_DCL void FDECL(mp_glow2, (struct monst *, struct obj *, const char *));
STATIC_DCL boolean FDECL(get_valid_stinking_cloud_pos, (int, int));
STATIC_DCL boolean FDECL(is_valid_stinking_cloud_pos, (int, int, BOOLEAN_P));
STATIC_PTR void FDECL(display_stinking_cloud_positions, (int));
STATIC_PTR void FDECL(set_lit, (int, int, genericptr));
STATIC_DCL void NDECL(do_class_genocide);

STATIC_OVL boolean
learnscrolltyp(scrolltyp)
short scrolltyp;
{
    if (!objects[scrolltyp].oc_name_known) {
        makeknown(scrolltyp);
        more_experienced(0, 10);
        return TRUE;
    } else
        return FALSE;
}

/* also called from teleport.c for scroll of teleportation */
void
learnscroll(sobj)
struct obj *sobj;
{
    /* it's implied that sobj->dknown is set;
       we couldn't be reading this scroll otherwise */
    if (sobj->oclass != SPBOOK_CLASS)
        (void) learnscrolltyp(sobj->otyp);
}

STATIC_OVL char *
erode_obj_text(otmp, buf)
struct obj *otmp;
char *buf;
{
    int erosion = greatest_erosion(otmp);

    if (erosion)
        wipeout_text(buf, (int) (strlen(buf) * erosion / (2 * MAX_ERODE)),
                     otmp->o_id ^ (unsigned) ubirthday);
    return buf;
}

char *
hawaiian_motif(shirt, buf)
struct obj *shirt;
char *buf;
{
    unsigned motif;
    static const char *hawaiian_motifs[] = {
        /* birds */
        "flamingo",
        "parrot",
        "toucan",
        "bird of paradise", /* could be a bird or a flower */
        /* sea creatures */
        "sea turtle",
        "tropical fish",
        "jellyfish",
        "giant eel",
        "water nymph",
        /* plants */
        "plumeria",
        "orchid",
        "hibiscus flower",
        "palm tree",
        /* other */
        "hula dancer",
        "sailboat",
        "ukulele",
    };

    /* tourists' starting shirt always has a consistent o_id, so we need to
    * introduce additional randomness or else its design will never differ */
    motif = shirt->o_id ^ (unsigned) ubirthday;

    Strcpy(buf, hawaiian_motifs[motif % SIZE(hawaiian_motifs)]);
    return buf;
}

STATIC_OVL char *
hawaiian_design(shirt, buf)
struct obj *shirt;
char *buf;
{
    unsigned bg;
    static const char *hawaiian_bgs[] = {
        /* solid colors */
        "purple",
        "yellow",
        "red",
        "blue",
        "orange",
        "black",
        "green",
        /* adjectives */
        "abstract",
        "geometric",
        "patterned",
        "naturalistic",
    };

    bg = shirt->o_id ^ (unsigned) ~ubirthday;
    Sprintf(buf, "%s on %s background",
            makeplural(hawaiian_motif(shirt, buf)),
            an(hawaiian_bgs[bg % SIZE(hawaiian_bgs)]));
    return buf;
}

char *
tshirt_text(tshirt, buf)
struct obj *tshirt;
char *buf;
{
    static const char *shirt_msgs[] = {
        /* Scott Bigham */
      "I explored the Dungeons of Doom and all I got was this lousy T-shirt!",
        "Is that Mjollnir in your pocket or are you just happy to see me?",
      "It's not the size of your sword, it's how #enhance'd you are with it.",
        "Madame Elvira's House O' Succubi Lifetime Customer",
        "Madame Elvira's House O' Succubi Employee of the Month",
        "Ludios Vault Guards Do It In Small, Dark Rooms",
        "Yendor Military Soldiers Do It In Large Groups",
        "I survived Yendor Military Boot Camp",
        "Ludios Accounting School Intra-Mural Lacrosse Team",
        "Oracle(TM) Fountains 10th Annual Wet T-Shirt Contest",
        "Hey, black dragon!  Disintegrate THIS!",
        "I'm With Stupid -->",
        "Don't blame me, I voted for Izchak!",
        "Don't Panic", /* HHGTTG */
        "Furinkan High School Athletic Dept.",                /* Ranma 1/2 */
        "Hel-LOOO, Nurse!",                                   /* Animaniacs */
        "=^.^=",
        "100% goblin hair - do not wash",
        "Aberzombie and Fitch",
        "cK -- Cockatrice touches the Kop",
        "Don't ask me, I only adventure here",
        "Down with pants!",
        "d, your dog or a killer?",
        "FREE PUG AND NEWT!",
        "Go team ant!",
        "Got newt?",
        "Hello, my darlings!", /* Charlie Drake */
        "Hey!  Nymphs!  Steal This T-Shirt!",
        "I <3 Dungeon of Doom",
        "I <3 Maud",
        "I am a Valkyrie.  If you see me running, try to keep up.",
        "I am not a pack rat - I am a collector",
        "I bounced off a rubber tree",         /* Monkey Island */
        "Plunder Island Brimstone Beach Club", /* Monkey Island */
        "If you can read this, I can hit you with my polearm",
        "I'm confused!",
        "I scored with the princess",
        "I want to live forever or die in the attempt.",
        "Lichen Park",
        "LOST IN THOUGHT - please send search party",
        "Meat is Mordor",
        "Minetown Better Business Bureau",
        "Minetown Watch",
 "Ms. Palm's House of Negotiable Affection -- A Very Reputable House Of Disrepute",
        "Protection Racketeer",
        "Real men love Crom",
        "Somebody stole my Mojo!",
        "The Hellhound Gang",
        "The Werewolves",
        "They Might Be Storm Giants",
        "Weapons don't kill people, I kill people",
        "White Zombie",
        "You're killing me!",
        "Anhur State University - Home of the Fighting Fire Ants!",
        "FREE HUGS",
        "Serial Ascender",
        "Real men are valkyries",
        "Young Men's Cavedigging Association",
        "Occupy Fort Ludios",
        "I couldn't afford this T-shirt so I stole it!",
        "Mind flayers suck",
        "I'm not wearing any pants",
        "Down with the living!",
        "Pudding farmer",
        "Vegetarian",
        "Hello, I'm War!",
        "It is better to light a candle than to curse the darkness",
        "It is easier to curse the darkness than to light a candle",
        /* expanded "rock--paper--scissors" featured in TV show "Big Bang
           Theory" although they didn't create it (and an actual T-shirt
           with pentagonal diagram showing which choices defeat which) */
        "rock--paper--scissors--lizard--Spock!",
        /* "All men must die -- all men must serve" challange and response
           from book series _A_Song_of_Ice_and_Fire_ by George R.R. Martin,
           TV show "Game of Thrones" (probably an actual T-shirt too...) */
        "/Valar morghulis/ -- /Valar dohaeris/",
        /* actual funny t-shirt slogans that make me chuckle */
        "Totes Adorbs",
        "Zonk Track Club",
        "Licky Chewy's Pogey Bait",
        "Space Force Gunnery School",
        "Mandatory Fun Shirt",
        /* more random stuff */
        "Alhoons really suck",
        "I love magic lamp",
        "The Ice Queen Snow Cone Stand, est. 2020",
        "Cerberus ate my homework",
        "My firstborn for a magic marker",
    };

    Strcpy(buf, shirt_msgs[tshirt->o_id % SIZE(shirt_msgs)]);
    return erode_obj_text(tshirt, buf);
}

char *
apron_text(apron, buf)
struct obj *apron;
char *buf;
{
    static const char *apron_msgs[] = {
        "Kiss the cook",
        "I'm making SCIENCE!",
        "Don't mess with the chef",
        "Don't make me poison you",
        "Gehennom's Kitchen",
        "Rat: The other white meat",
        "If you can't stand the heat, get out of Gehennom!",
        "If we weren't meant to eat animals, why are they made out of meat?",
        "If you don't like the food, I'll stab you",
    };

    Strcpy(buf, apron_msgs[apron->o_id % SIZE(apron_msgs)]);
    return erode_obj_text(apron, buf);
}

char *
striped_text(striped, buf)
struct obj *striped;
char *buf;
{
    unsigned msgidx;
    static const char *const striped_msgs[] = {
        "AZ# 85",    /* Al Capone */
        "AZ# 117",   /* George 'Machine Gun' Kelly */
        "AZ# 594",   /* Robert 'The Birdman of Alcatraz' Stroud */
        "B-33920",   /* Charles Manson */
        "227501",    /* Jeffrey Dahmer */
        "46664",     /* Nelson Mandela */
        "B5160-8",   /* Hannibal Lecter */
        "37927",     /* Andy Dufresne */
        "55170-054", /* Martha Stewart */
        "1027820",   /* O.J. Simpson */
    };


    msgidx = striped->o_id ^ (unsigned) ubirthday;

    Strcpy(buf, striped_msgs[msgidx % SIZE(striped_msgs)]);
    return erode_obj_text(striped, buf);
}

int
doread()
{
    register struct obj *scroll;
    boolean confused, nodisappear;
    const char *mesg;
    known = FALSE;
    if (Hidinshell) {
        You_cant("read anything while hiding in your shell.");
        return 0;
    }
    if (check_capacity((char *) 0))
        return 0;
    scroll = getobj(readable, "read");
    if (!scroll)
        return 0;

    /* outrumor has its own blindness check */
    if (scroll->otyp == FORTUNE_COOKIE) {
        if (flags.verbose)
            You("break up the cookie and throw away the pieces.");
        outrumor(bcsign(scroll), BY_COOKIE);
        if (!Blind)
            if (!u.uconduct.literate++)
                livelog_write_string(LL_CONDUCT,
                        "became literate by reading a fortune cookie");
        useup(scroll);
        return 1;
    } else if (scroll->otyp == T_SHIRT || scroll->otyp == ALCHEMY_SMOCK
               || scroll->otyp == STRIPED_SHIRT
               || scroll->otyp == HAWAIIAN_SHIRT) {
        char buf[BUFSZ];
        const char *endpunct;

        if (Blind) {
            You_cant("feel any Braille writing.");
            return 0;
        }
        /* can't read shirt worn under suit (under cloak is ok though) */
        if ((scroll->otyp == T_SHIRT || scroll->otyp == STRIPED_SHIRT
            || scroll->otyp == HAWAIIAN_SHIRT) && uarm && scroll == uarmu) {
            pline("%s shirt is obscured by %s%s.",
                  scroll->unpaid ? "That" : "Your", shk_your(buf, uarm),
                  suit_simple_name(uarm));
            return 0;
        }
        /* populate 'buf[]' */
        mesg = (scroll->otyp == T_SHIRT) ? tshirt_text(scroll, buf)
                                         : (scroll->otyp == STRIPED_SHIRT)
                                            ? striped_text(scroll, buf)
                                            : (scroll->otyp == HAWAIIAN_SHIRT)
                                                ? hawaiian_design(scroll, buf)
                                                : apron_text(scroll, buf);

        if (scroll->otyp == HAWAIIAN_SHIRT) {
            pline("%s features %s.", flags.verbose ? "The design" : "It",
                    mesg);
        } else {
            if (!u.uconduct.literate++)
                livelog_printf(LL_CONDUCT, "became literate by reading %s",
                        (scroll->otyp == T_SHIRT)
                            ? "a T-shirt"
                            : (scroll->otyp == STRIPED_SHIRT)
                                ? "a striped shirt"
                                : "an apron");

            endpunct = "";
            if (flags.verbose) {
                int ln = (int) strlen(mesg);

                /* we will be displaying a sentence; need ending punctuation */
                if (ln > 0 && !index(".!?", mesg[ln - 1]))
                    endpunct = ".";
                pline("It reads:");
            }
            pline("\"%s\"%s", mesg, endpunct);
            maybe_learn_elbereth(mesg);
        }
        return 1;
    } else if (scroll->otyp == CREDIT_CARD) {
        static const char *card_msgs[] = {
            "Leprechaun Gold Tru$t - Shamrock Card",
            "Magic Memory Vault Charge Card",
            "Larn National Bank",                /* Larn */
            "First Bank of Omega",               /* Omega */
            "Bank of Zork - Frobozz Magic Card", /* Zork */
            "Ankh-Morpork Merchant's Guild Barter Card",
            "Ankh-Morpork Thieves' Guild Unlimited Transaction Card",
            "Ransmannsby Moneylenders Association",
            "Bank of Gehennom - 99% Interest Card",
            "Yendorian Express - Copper Card",
            "Yendorian Express - Silver Card",
            "Yendorian Express - Gold Card",
            "Yendorian Express - Mithril Card",
            "Yendorian Express - Platinum Card", /* must be last */
        };

        if (Blind) {
            You("feel the embossed numbers:");
        } else {
            if (flags.verbose)
                pline("It reads:");
            mesg = scroll->oartifact
                  ? card_msgs[SIZE(card_msgs) - 1]
                  : card_msgs[scroll->o_id % (SIZE(card_msgs) - 1)];
            pline("\"%s\"", mesg);
            maybe_learn_elbereth(mesg);
        }
        /* Make a credit card number */
        pline("\"%d0%d %ld%d1 0%d%d0\"%s",
              (((int) scroll->o_id % 89) + 10),
              ((int) scroll->o_id % 4),
              ((((long) scroll->o_id * 499L) % 899999L) + 100000L),
              ((int) scroll->o_id % 10),
              (!((int) scroll->o_id % 3)),
              (((int) scroll->o_id * 7) % 10),
              (flags.verbose || Blind) ? "." : "");
        if (!u.uconduct.literate++)
            livelog_write_string(LL_CONDUCT,
                    "became literate by reading a credit card");
        return 1;
    } else if (scroll->otyp == CAN_OF_GREASE) {
        pline("This %s has no label.", singular(scroll, xname));
        return 0;
    } else if (scroll->otyp == MAGIC_MARKER) {
        if (Blind) {
            You_cant("feel any Braille writing.");
            return 0;
        }
        if (flags.verbose)
            pline("It reads:");
        pline("\"Magic Marker(TM) Red Ink Marker Pen.  Water Soluble.\"");
        if (!u.uconduct.literate++)
            livelog_write_string(LL_CONDUCT,
                    "became literate by reading a magic marker");
        return 1;
    } else if (scroll->oclass == COIN_CLASS) {
        if (Blind)
            You("feel the embossed words:");
        else if (flags.verbose)
            You("read:");
        pline("\"1 Zorkmid.  857 GUE.  In Frobs We Trust.\"");
        if (!u.uconduct.literate++)
            livelog_write_string(LL_CONDUCT,
                    "became literate by reading a coin's engravings");
        return 1;
    } else if (scroll->oartifact == ART_GJALLAR) {
        if (Blind)
            You("feel the engraved signature:");
        else
            pline("It is signed:");
        pline("\"Heimdallr.\"");
        if (!u.uconduct.literate++)
            livelog_write_string(LL_CONDUCT,
                    "became literate by reading the divine signature of Heimdallr");
        return 1;
    } else if (scroll->oartifact == ART_MAGIC___BALL) {
        static const char *eightball_msgs[] = {
            /* Positive */
            "Yes", "Signs point to yes", "As I see it, yes",
            "Yes - definitely", "It is certain", "It is decidedly so",
            "Without a doubt", "You may rely on it", "Most likely",
            "Outlook good",
            /* Noncommital */
            "Reply hazy, try again", "Ask again later",
            "Better not tell you now", "Cannot predict now",
            "Concentrate and ask again",
            /* Negative */
            "Don't count on it", "Outlook not so good", "Very doubtful",
            "My reply is no", "My sources say no",
        };

        if (Blind) {
            You("feel only a smooth plastic window.");
            return 0;
        }
        if (flags.verbose) {
            pline("You turn over %s and read:", the(xname(scroll)));
        }
        pline("\"%s\".", eightball_msgs[rn2(SIZE(eightball_msgs))]);
        if (!u.uconduct.literate++) {
            livelog_write_string(LL_CONDUCT,
                    "became literate by reading the Magic 8-Ball");
        }
        return 1;
    } else if (scroll->oartifact == ART_ONE_RING) {
        if (Blind) {
            You_feel("an engraving on the %s.", singular(scroll, xname));
            return 0;
        }
        if (flags.verbose)
            pline("It reads:");
        pline("\"One Ring to rule them all, One Ring to find them, One Ring to bring them all and in the darkness bind them.\"");
        if (!u.uconduct.literate++)
            livelog_write_string(LL_CONDUCT,
                    "became literate by reading the One Ring");
        return 1;
    } else if (scroll->otyp == CANDY_BAR) {
        static const char *wrapper_msgs[] = {
            "Apollo",       /* Lost */
            "Moon Crunchy", /* South Park */
            "Snacky Cake",    "Chocolate Nuggie", "The Small Bar",
            "Crispy Yum Yum", "Nilla Crunchie",   "Berry Bar",
            "Choco Nummer",   "Om-nom", /* Cat Macro */
            "Fruity Oaty",              /* Serenity */
            "Wonka Bar" /* Charlie and the Chocolate Factory */
        };

        if (Blind) {
            You_cant("feel any Braille writing.");
            return 0;
        }
        mesg = wrapper_msgs[scroll->o_id % SIZE(wrapper_msgs)];
        pline("The wrapper reads: \"%s\".", mesg);
        maybe_learn_elbereth(mesg);
        if (!u.uconduct.literate++)
            livelog_write_string(LL_CONDUCT,
                    "became literate by reading a candy bar wrapper");
        return 1;
    } else if (scroll->oclass != SCROLL_CLASS
               && scroll->oclass != SPBOOK_CLASS) {
        pline(silly_thing_to, "read");
        return 0;
    } else if (Blind && (scroll->otyp != SPE_BOOK_OF_THE_DEAD)) {
        const char *what = 0;

        if (scroll->oclass == SPBOOK_CLASS)
            what = "mystic runes";
        else if (!scroll->dknown)
            what = "formula on the scroll";
        if (what) {
            pline("Being blind, you cannot read the %s.", what);
            return 0;
        }
    }

    confused = (Confusion != 0);
#ifdef MAIL
    if (scroll->otyp == SCR_MAIL) {
        confused = FALSE; /* override */
        /* reading mail is a convenience for the player and takes
           place outside the game, so shouldn't affect gameplay;
           on the other hand, it starts by explicitly making the
           hero actively read something, which is pretty hard
           to simply ignore; as a compromise, if the player has
           maintained illiterate conduct so far, and this mail
           scroll didn't come from bones, ask for confirmation */
        if (!u.uconduct.literate) {
            if (!scroll->spe && yn(
             "Reading mail will violate \"illiterate\" conduct.  Read anyway?"
                                   ) != 'y')
                return 0;
        }
    }
#endif

    /* Actions required to win the game aren't counted towards conduct */
    /* Novel conduct is handled in read_tribute so exclude it too*/
    if (scroll->otyp != SPE_BOOK_OF_THE_DEAD
        && scroll->otyp != SPE_BLANK_PAPER && scroll->otyp != SCR_BLANK_PAPER
        && scroll->otyp != SPE_NOVEL)
        if (!u.uconduct.literate++)
            livelog_printf(LL_CONDUCT, "became literate by reading %s",
                    scroll->oclass == SPBOOK_CLASS ? "a book" :
                    scroll->oclass == SCROLL_CLASS ? "a scroll" : "something");

    if (scroll->oclass == SPBOOK_CLASS) {
        return study_book(scroll);
    }
    scroll->in_use = TRUE; /* scroll, not spellbook, now being read */
    if (scroll->otyp != SCR_BLANK_PAPER) {
        boolean silently = !can_chant(&youmonst);

        /* a few scroll feedback messages describe something happening
           to the scroll itself, so avoid "it disappears" for those */
        nodisappear = (scroll->otyp == SCR_FIRE
                       || (scroll->otyp == SCR_REMOVE_CURSE
                           && scroll->cursed));
        if (Blind)
            pline(nodisappear
                      ? "You %s the formula on the scroll."
                      : "As you %s the formula on it, the scroll disappears.",
                  silently ? "cogitate" : "pronounce");
        else
            pline(nodisappear ? "You read the scroll."
                              : "As you read the scroll, it disappears.");
        if (confused) {
            if (Hallucination)
                pline("Being so trippy, you screw up...");
            else
                pline("Being confused, you %s the magic words...",
                      silently ? "misunderstand" : "mispronounce");
        }
    }
    if (!seffects(scroll)) {
        if (!objects[scroll->otyp].oc_name_known) {
            if (known)
                learnscroll(scroll);
            else if (!objects[scroll->otyp].oc_uname)
                docall(scroll);
        }
        scroll->in_use = FALSE;
        if (scroll->otyp != SCR_BLANK_PAPER)
            useup(scroll);
    }
    return 1;
}

STATIC_OVL void
stripspe(obj)
register struct obj *obj;
{
    if (obj->blessed || obj->spe <= 0) {
        pline1(nothing_happens);
    } else {
        /* order matters: message, shop handling, actual transformation */
        pline("%s briefly.", Yobjnam2(obj, "vibrate"));
        costly_alteration(obj, COST_UNCHRG);
        obj->spe = 0;
        if (obj->otyp == OIL_LAMP || obj->otyp == LANTERN)
            obj->age = 0;
    }
}

STATIC_OVL void
p_glow1(otmp)
register struct obj *otmp;
{
    pline("%s briefly.", Yobjnam2(otmp, Blind ? "vibrate" : "glow"));
}

STATIC_OVL void
p_glow2(otmp, color)
register struct obj *otmp;
register const char *color;
{
    pline("%s%s%s for a moment.", Yobjnam2(otmp, Blind ? "vibrate" : "glow"),
          Blind ? "" : " ", Blind ? "" : hcolor(color));
}

STATIC_OVL void
mp_glow1(mtmp, otmp)
register struct monst *mtmp;
register struct obj *otmp;
{
    if (canseemon(mtmp) && !Blind)
        pline("%s %s %s briefly.", s_suffix(Monnam(mtmp)), xname(otmp),
              otense(otmp, "glow"));
}

STATIC_OVL void
mp_glow2(mtmp, otmp, color)
register struct monst *mtmp;
register struct obj *otmp;
register const char *color;
{
    if (canseemon(mtmp) && !Blind)
        pline("%s %s %s %s for a moment.", s_suffix(Monnam(mtmp)),
              xname(otmp), otense(otmp, "glow"),
              hcolor(color));
}

/* Is the object chargeable?  For purposes of inventory display; it is
   possible to be able to charge things for which this returns FALSE. */
boolean
is_chargeable(obj)
struct obj *obj;
{
    if (obj->oclass == WAND_CLASS)
        return TRUE;
    /* known && !oc_name_known is possible after amnesia/mind flayer */
    if (obj->oclass == RING_CLASS)
        return (boolean) (objects[obj->otyp].oc_charged
                          && (obj->known
                              || (obj->dknown
                                  && objects[obj->otyp].oc_name_known)));
    if (is_weptool(obj)) /* specific check before general tools */
        return FALSE;
    if (obj->oclass == TOOL_CLASS)
        return (boolean) objects[obj->otyp].oc_charged;
    return FALSE; /* why are weapons/armor considered charged anyway? */
}

/* recharge an object; curse_bless is -1 if the recharging implement
   was cursed, +1 if blessed, 0 otherwise. */
void
recharge(obj, curse_bless, mtmp)
struct obj *obj;
int curse_bless;
struct monst *mtmp;
{
    register int n;
    boolean is_cursed, is_blessed;
    boolean yours = (mtmp == &youmonst);

    is_cursed = curse_bless < 0;
    is_blessed = curse_bless > 0;

    if (obj->oclass == WAND_CLASS) {
        int lim = (obj->otyp == WAN_WISHING)
                      ? 3
                      : (objects[obj->otyp].oc_dir != NODIR) ? 8 : 15;

        /* undo any prior cancellation, even when is_cursed */
        if (obj->spe == -1)
            obj->spe = 0;

        /*
         * Recharging might cause wands to explode.
         *      v = number of previous recharges
         *            v = percentage chance to explode on this attempt
         *                    v = cumulative odds for exploding
         *      0 :   0       0
         *      1 :   0.29    0.29
         *      2 :   2.33    2.62
         *      3 :   7.87   10.28
         *      4 :  18.66   27.02
         *      5 :  36.44   53.62
         *      6 :  62.97   82.83
         *      7 : 100     100
         */
        n = (int) obj->recharged;
        if (n > 0 && (obj->otyp == WAN_WISHING || obj->otyp == WAN_DEATH
                      || (n * n * n > rn2(7 * 7 * 7)))) { /* recharge_limit */
            if (yours)
                wand_explode(obj, rnd(lim));
            else
                mwand_explode(mtmp, obj);
            return;
        }
        /* didn't explode, so increment the recharge count */
        obj->recharged = (unsigned) (n + 1);

        /* now handle the actual recharging */
        if (is_cursed) {
            stripspe(obj);
        } else {
            n = (lim == 3) ? 3 : rn1(5, lim + 1 - 5);
            if (!is_blessed)
                n = rnd(n);

            if (obj->spe < n)
                obj->spe = n;
            else
                obj->spe++;
            if (obj->otyp == WAN_WISHING && obj->spe > 3) {
                if (yours)
                    wand_explode(obj, 1);
                else
                    mwand_explode(mtmp, obj);
                return;
            }
            if (yours) {
                if (obj->spe >= lim)
                    p_glow2(obj, NH_BLUE);
                else
                    p_glow1(obj);
            } else {
                if (obj->spe >= lim)
                    mp_glow2(mtmp, obj, NH_BLUE);
                else
                    mp_glow1(mtmp, obj);
            }
#if 0 /*[shop price doesn't vary by charge count]*/
            /* update shop bill to reflect new higher price */
            if (obj->unpaid)
                alter_cost(obj, 0L);
#endif
        }

    } else if (obj->oclass == RING_CLASS && objects[obj->otyp].oc_charged) {
        /* charging does not affect ring's curse/bless status */
        int s = is_blessed ? rnd(3) : is_cursed ? -rnd(2) : 1;
        boolean is_on = yours ? (obj == uleft || obj == uright)
                              : 0; /* monsters wear but don't charge rings */

        /* destruction depends on current state, not adjustment */
        if (obj->spe > rn2(7) || obj->spe <= -5) {
            if (yours) {
                pline("%s momentarily, then %s!", Yobjnam2(obj, "pulsate"),
                      otense(obj, "explode"));
                if (is_on)
                    Ring_gone(obj);
                s = rnd(3 * abs(obj->spe)); /* amount of damage */
                useup(obj), obj = 0;
                losehp(Maybe_Half_Phys(s), "exploding ring", KILLED_BY_AN);
            } else {
                if (canseemon(mtmp))
                    pline("%s %s %s momentarily, then %s!",
                          s_suffix(Monnam(mtmp)), xname(obj),
                          otense(obj, "pulsate"), otense(obj,"explode"));
                else
                    You_hear("an explosion.");
                m_useup(mtmp, obj);
                mtmp->mhp -= 3 * abs(obj->spe);
                if (mtmp->mhp <= 0) {
                    if (canseemon(mtmp))
                        pline("%s is killed by the explosion!",
                              Monnam(mtmp));
                    mondied(mtmp);
                }
           }
        } else {
            long mask = is_on ? (obj == uleft ? LEFT_RING : RIGHT_RING) : 0L;

            pline("%s spins %sclockwise for a moment.", Yname2(obj),
                  s < 0 ? "counter" : "");
            if (s < 0)
                costly_alteration(obj, COST_DECHNT);
            /* cause attributes and/or properties to be updated */
            if (is_on)
                Ring_off(obj);
            obj->spe += s; /* update the ring while it's off */
            if (is_on)
                setworn(obj, mask), Ring_on(obj);
            /* oartifact: if a touch-sensitive artifact ring is
               ever created the above will need to be revised  */
            /* update shop bill to reflect new higher price */
            if (s > 0 && obj->unpaid)
                alter_cost(obj, 0L);
        }

    } else if (obj->oclass == TOOL_CLASS) {
        int rechrg = (int) obj->recharged;

        if (objects[obj->otyp].oc_charged) {
            /* tools don't have a limit, but the counter used does */
            if (rechrg < 7) /* recharge_limit */
                obj->recharged++;
        }
        switch (obj->otyp) {
        case BELL_OF_OPENING:
            if (is_cursed)
                stripspe(obj);
            else if (is_blessed)
                obj->spe += rnd(3);
            else
                obj->spe += 1;
            if (obj->spe > 5)
                obj->spe = 5;
            break;
        case MAGIC_MARKER:
        case TINNING_KIT:
        case EXPENSIVE_CAMERA:
            if (is_cursed)
                stripspe(obj);
            else if (rechrg
                     && obj->otyp
                            == MAGIC_MARKER) { /* previously recharged */
                obj->recharged = 1; /* override increment done above */
                if (obj->spe < 3)
                    Your("marker seems permanently dried out.");
                else
                    pline1(nothing_happens);
            } else if (is_blessed) {
                n = rn1(16, 15); /* 15..30 */
                if (obj->spe + n <= 50)
                    obj->spe = 50;
                else if (obj->spe + n <= 75)
                    obj->spe = 75;
                else {
                    int chrg = (int) obj->spe;
                    if ((chrg + n) > 127)
                        obj->spe = 127;
                    else
                        obj->spe += n;
                }
                p_glow2(obj, NH_BLUE);
            } else {
                n = rn1(11, 10); /* 10..20 */
                if (obj->spe + n <= 50)
                    obj->spe = 50;
                else {
                    int chrg = (int) obj->spe;
                    if ((chrg + n) > 127)
                        obj->spe = 127;
                    else
                        obj->spe += n;
                }
                p_glow2(obj, NH_WHITE);
            }
            break;
        case OIL_LAMP:
        case LANTERN:
            if (is_cursed) {
                stripspe(obj);
                if (obj->lamplit) {
                    if (!Blind)
                        pline("%s out!", Tobjnam(obj, "go"));
                    end_burn(obj, TRUE);
                }
            } else if (is_blessed) {
                obj->spe = 1;
                obj->age = 1500;
                p_glow2(obj, NH_BLUE);
            } else {
                obj->spe = 1;
                obj->age += 750;
                if (obj->age > 1500)
                    obj->age = 1500;
                p_glow1(obj);
            }
            break;
        case CRYSTAL_BALL:
            if (is_cursed) {
                stripspe(obj);
            } else if (is_blessed) {
                obj->spe = 6;
                p_glow2(obj, NH_BLUE);
            } else {
                if (obj->spe < 5) {
                    obj->spe++;
                    p_glow1(obj);
                } else
                    pline1(nothing_happens);
            }
            break;
        case HORN_OF_PLENTY:
        case BAG_OF_TRICKS:
        case CAN_OF_GREASE:
            if (is_cursed) {
                stripspe(obj);
            } else if (is_blessed) {
                if (obj->spe <= 10)
                    obj->spe += rn1(10, 6);
                else
                    obj->spe += rn1(5, 6);
                if (obj->spe > 50)
                    obj->spe = 50;
                p_glow2(obj, NH_BLUE);
            } else {
                obj->spe += rnd(5);
                if (obj->spe > 50)
                    obj->spe = 50;
                p_glow1(obj);
            }
            break;
        case MAGIC_FLUTE:
        case MAGIC_HARP:
        case FROST_HORN:
        case FIRE_HORN:
        case DRUM_OF_EARTHQUAKE:
            if (is_cursed) {
                stripspe(obj);
            } else if (is_blessed) {
                obj->spe += d(2, 4);
                if (obj->spe > 20)
                    obj->spe = 20;
                if (yours)
                    p_glow2(obj, NH_BLUE);
                else
                    mp_glow2(mtmp, obj, NH_BLUE);
            } else {
                obj->spe += rnd(4);
                if (obj->spe > 20)
                    obj->spe = 20;
                if (yours)
                    p_glow1(obj);
                else
                    mp_glow1(mtmp, obj);
            }
            break;
        default:
            goto not_chargable;
            /*NOTREACHED*/
            break;
        } /* switch */

    } else {
    not_chargable:
        if (yours)
            You("have a feeling of loss.");
        else if (canseemon(mtmp))
            pline("%s looks disappointed.", Monnam(mtmp));
    }
}

/* Forget all traps on the level. */
void
forget_traps()
{
    register struct trap *trap;

    /* forget all traps (except the one the hero is in :-) */
    for (trap = ftrap; trap; trap = trap->ntrap)
        if ((trap->tx != u.ux || trap->ty != u.uy) && (trap->ttyp != HOLE))
            trap->tseen = 0;
}

/*
 * Forget some things (e.g. after reading a scroll of amnesia).  When called,
 * the following are always forgotten:
 *      - felt ball & chain
 *      - traps
 *      - skill training
 *
 * Other things are subject to flags:
 *      howmuch & ALL_SPELLS    = forget all spells
 */
void
forget(howmuch)
int howmuch;
{
    if (Punished)
        u.bc_felt = 0; /* forget felt ball&chain */

    forget_traps();

    if (howmuch & ALL_SPELLS)
        losespells();

    /* Forget some skills. */
    drain_weapon_skill(rnd(howmuch ? 5 : 3));

    docrt(); /* this correctly will reset vision
                for forget_traps() */
}

/* monster is hit by scroll of taming's effect */
int
maybe_tame(mtmp, sobj)
struct monst *mtmp;
struct obj *sobj;
{
    int was_tame = mtmp->mtame;
    unsigned was_peaceful = mtmp->mpeaceful;

    if (sobj->cursed) {
        setmangry(mtmp, FALSE);
        if (was_peaceful && !mtmp->mpeaceful)
            return -1;
    } else {
        if (mtmp->isshk)
            make_happy_shk(mtmp, FALSE);
        else if (!resist(mtmp, sobj->oclass, 0, NOTELL))
            (void) tamedog(mtmp, (struct obj *) 0);
        if ((!was_peaceful && mtmp->mpeaceful) || (!was_tame && mtmp->mtame))
            return 1;
    }
    return 0;
}

STATIC_OVL boolean
get_valid_stinking_cloud_pos(x,y)
int x,y;
{
    return (!(!isok(x,y) || !cansee(x, y)
              || !ACCESSIBLE(levl[x][y].typ)
              || distu(x, y) >= 32));
}

STATIC_OVL boolean
is_valid_stinking_cloud_pos(x, y, showmsg)
int x, y;
boolean showmsg;
{
    if (!get_valid_stinking_cloud_pos(x,y)) {
        if (showmsg)
            You("smell rotten eggs.");
        return FALSE;
    }
    return TRUE;
}

STATIC_PTR void
display_stinking_cloud_positions(state)
int state;
{
    if (state == 0) {
        tmp_at(DISP_BEAM, cmap_to_glyph(S_goodpos));
    } else if (state == 1) {
        int x, y, dx, dy;
        int dist = 6;

        for (dx = -dist; dx <= dist; dx++)
            for (dy = -dist; dy <= dist; dy++) {
                x = u.ux + dx;
                y = u.uy + dy;
                if (get_valid_stinking_cloud_pos(x,y))
                    tmp_at(x, y);
            }
    } else {
        tmp_at(DISP_END, 0);
    }
}

/* scroll effects; return 1 if we use up the scroll and possibly make it
   become discovered, 0 if caller should take care of those side-effects */
int
seffects(sobj)
struct obj *sobj; /* sobj - scroll or fake spellbook for spell */
{
    int cval, otyp = sobj->otyp;
    boolean confused = (Confusion != 0), sblessed = sobj->blessed,
            scursed = sobj->cursed, already_known, old_erodeproof,
            new_erodeproof;
    struct obj *otmp = (struct obj *) 0;

    if (objects[otyp].oc_magic)
        exercise(A_WIS, TRUE);                       /* just for trying */
    already_known = (sobj->oclass == SPBOOK_CLASS /* spell */
                     || objects[otyp].oc_name_known);

    switch (otyp) {
#ifdef MAIL
    case SCR_MAIL:
        known = TRUE;
        if (sobj->spe == 2)
            /* "stamped scroll" created via magic marker--without a stamp */
            pline("This scroll is marked \"postage due\".");
        else if (sobj->spe)
            /* scroll of mail obtained from bones file or from wishing;
             * note to the puzzled: the game Larn actually sends you junk
             * mail if you win!
             */
            pline(
    "This seems to be junk mail addressed to the finder of the Eye of Larn.");
        else
            readmail(sobj);
        break;
#endif
    case SCR_ENCHANT_ARMOR: {
        register schar s;
        boolean special_armor;
        boolean same_color;

        if (already_known) {
            if (u.usteed
                && (otmp = which_armor(u.usteed, W_BARDING)) != 0) {
                char buf[BUFSZ];
                Sprintf(buf, "Enchant %s %s?", s_suffix(y_monnam(u.usteed)),
                        xname(otmp));
                if (yn(buf) == 'n') {
                    otmp = (struct obj *) 0;
                }
            }
            if (!otmp) {
                otmp = getobj(clothes, "enchant");
                while (otmp && !(otmp->owornmask & W_ARMOR)) {
                    pline("You cannot enchant armor that is not worn.");
                    otmp = getobj(clothes, "enchant");
                }
            }
        } else {
            if (uarmc && Is_dragon_scales(uarmc))
                otmp = uarmc;
            else
                otmp = some_armor(&youmonst);
        }
        if (!otmp) {
            strange_feeling(sobj, !Blind
                                      ? "Your skin glows then fades."
                                      : "Your skin feels warm for a moment.");
            sobj = 0; /* useup() in strange_feeling() */
            exercise(A_CON, !scursed);
            exercise(A_STR, !scursed);
            break;
        }
        /* dragon scales are handled later and shouldn't/can't become fixed */
        if (confused && !Is_dragon_scales(otmp)) {
            old_erodeproof = (otmp->oerodeproof != 0);
            new_erodeproof = !scursed;
            otmp->oerodeproof = 0; /* for messages */
            if (new_erodeproof && (otmp->oeroded || otmp->oeroded2)) {
                otmp->oeroded = otmp->oeroded2 = 0;
                pline("%s as good as new!",
                      Yobjnam2(otmp, Blind ? "feel" : "look"));
            }
            /* naturally fixed armors don't mess with oerodeproof */
            if (is_supermaterial(otmp)) {
                if (!Blind)
                    pline("%s for a moment.", Yobjnam2(otmp, "shimmer"));
                break;
            }
            if (Blind) {
                otmp->rknown = FALSE;
                pline("%s warm for a moment.", Yobjnam2(otmp, "feel"));
            } else {
                otmp->rknown = TRUE;
                pline("%s covered by a %s %s %s!", Yobjnam2(otmp, "are"),
                      scursed ? "mottled" : "shimmering",
                      hcolor(scursed ? NH_BLACK : NH_GOLDEN),
                      scursed ? "glow"
                              : (is_shield(otmp) ? "layer" : "shield"));
            }
            if (old_erodeproof && !new_erodeproof) {
                /* restore old_erodeproof before shop charges */
                otmp->oerodeproof = 1;
                costly_alteration(otmp, COST_DEGRD);
            }
            otmp->oerodeproof = new_erodeproof ? 1 : 0;
            break;
        }
        /* elven armor vibrates warningly when enchanted beyond a limit */
        special_armor = is_elven_armor(otmp)
                        || (Race_if(PM_DROW) && is_drow_armor(otmp))
                        || otmp->oartifact == ART_HAND_OF_VECNA
                        || otmp->oartifact == ART_ARMOR_OF_RETRIBUTION
                        || (Role_if(PM_WIZARD) && otmp->otyp == CORNUTHAUM);
        if (scursed)
            same_color = (otmp->otyp == BLACK_DRAGON_SCALES);
        else
            same_color = (otmp->otyp == SILVER_DRAGON_SCALES
                          || otmp->otyp == SHIELD_OF_REFLECTION);
        if (Blind)
            same_color = FALSE;

        /* KMH -- catch underflow */
        s = scursed ? -otmp->spe : otmp->spe;
        /* Dragon scales that are worn over body armor will cause the armor to
         * become scaled */
        if (Is_dragon_scales(otmp)) { /* guarantees that worn cloak is scales,
                                         but NOT existence of uarm */
            /* no body armor under the scales = the scales are enchanted
             * directly onto you (no such thing as a scaled shirt). The
             * wearer will polymorph. Also caused by a confused scroll,
             * _after_ the * scales meld. */
            boolean poly_after_merge = (!uarm || confused);
            int old_light =
                artifact_light(otmp) ? arti_light_radius(otmp) : 0;
            if (uarm) {
                struct obj *scales = uarmc;
                struct obj *armor = uarm;

                if (armor->oartifact == ART_ARMOR_OF_RETRIBUTION) {
                    pline("%s refuses to meld into the %s.",
                            Yname2(scales), artiname(armor->oartifact));
                    break;
                }

                pline("%s hardens and melds into your %s%s", Yname2(scales),
                        suit_simple_name(armor),
                        Is_dragon_scaled_armor(armor) ? "." : "!");

                if (Is_dragon_scaled_armor(armor)) {
                    if (Dragon_armor_to_scales(armor) == scales->otyp) {
                        /* scales match armor already; just use up scales */
                        pline("Its scales still seem %s.",
                                dragon_scales_color(armor));
                    } else {
                        /* armor is already scaled but the new scales are
                         * different and will replace the old ones */
                        pline("Its scales change from %s to %s!",
                                dragon_scales_color(armor),
                                dragon_scales_color(scales));
                        /* remove properties of old scales */
                        dragon_armor_handling(armor, FALSE);
                    }
                }
                setnotworn(armor);
                /* don't allow a suit of armor with an object property
                   to co-exist with merged dragon scales */
                if ((armor->oprops & ITEM_PROP_MASK) != 0L) {
                    oprops_off(armor, W_ARM);
                    armor->oprops &= ~(ITEM_PROP_MASK);
                }
                armor->dragonscales = scales->otyp;
                armor->cursed = 0;
                if (sblessed) {
                    armor->oeroded = armor->oeroded2 = 0;
                    armor->blessed = 1;
                }
                setworn(armor, W_ARM);
                check_wings(TRUE);
                dragon_armor_handling(armor, TRUE);
                known = TRUE;
                if (otmp->unpaid)
                    alter_cost(otmp, 0L); /* shop bill */

                /* handle gold/chromatic dragon-scaled armor... */
                if (scales->lamplit) {
                    if (armor->lamplit) {
                        /* if melding lit dragon scales onto already lit
                           dragon-scaled armor, avoid attaching a duplicate
                           light source to the armor.  useup() won't take care
                           of this, because it calls setnotworn(), which will
                           make artifact_light() return false, so the regular
                           check for deleting the light source when an object
                           is deallocated will do nothing */
                        del_light_source(LS_OBJECT, obj_to_any(scales));
                    } else {
                        /* this will set armor->lamplit */
                        obj_move_light_source(scales, armor);
                    }
                    /* may be different radius depending on BUC of armor */
                    maybe_adjust_light(armor, old_light);
                } else if (armor->lamplit) {
                    /* scales not lit but armor is: melding non-gold scales
                       onto gold/chromatic-scaled armor, which will no longer
                       be a light source */
                    end_burn(armor, FALSE);
                }
                useup(scales);
            }
            if (poly_after_merge) {
                polyself(4);
                /* adjust duration for scroll beatitude - a blessed scroll
                 * will give you more time as a dragon, a cursed scroll
                 * less */
                u.mtimedone = (u.mtimedone * (bcsign(sobj) + 2) / 2);
            }
            if (!scursed || !uarm) {
                break;
            } else {
                /* continue with regular cursed-enchant logic on the resulting
                 * armor piece */
                otmp = uarm;
            }
        }
        if (s > (special_armor ? 5 : 3) && rn2(s)) {
            otmp->in_use = TRUE;
            if ((otmp == uarmg) && otmp->oartifact == ART_HAND_OF_VECNA) {
                /* The Hand of Vecna is 'merged' with the wearer,
                   it can't be destroyed this way */
                pline("%s violently %s%s%s for a while, but nothing else happens.",
                      Yname2(otmp),
                      otense(otmp, Blind ? "vibrate" : "glow"),
                      (!Blind && !same_color) ? " " : "",
                      (Blind || same_color) ? "" : hcolor(scursed ? NH_BLACK
                                                                  : NH_SILVER));
            } else if ((otmp == uarm) && otmp->oartifact == ART_ARMOR_OF_RETRIBUTION) {
                /* The Armor of Retribution is indestructible */
                pline("%s violently %s%s%s for a while, but remains intact.",
                      Yname2(otmp),
                      otense(otmp, Blind ? "vibrate" : "glow"),
                      (!Blind && !same_color) ? " " : "",
                      (Blind || same_color) ? "" : hcolor(scursed ? NH_BLACK
                                                                  : NH_SILVER));
            } else {
                pline("%s violently %s%s%s for a while, then %s.", Yname2(otmp),
                      otense(otmp, Blind ? "vibrate" : "glow"),
                      (!Blind && !same_color) ? " " : "",
                      (Blind || same_color) ? "" : hcolor(scursed ? NH_BLACK
                                                                  : NH_SILVER),
                      otense(otmp, "evaporate"));
            }
            if (carried(otmp)) {
                if ((otmp == uarmg) && otmp->oartifact == ART_HAND_OF_VECNA) {
                    otmp->in_use = FALSE; /* nothing happens if worn */
                } else if ((otmp == uarm) && otmp->oartifact == ART_ARMOR_OF_RETRIBUTION) {
                    otmp->in_use = FALSE; /* nothing happens if worn */
                } else {
                    remove_worn_item(otmp, FALSE);
                    useup(otmp);
                }
            } else if (mcarried(otmp)) {
                /* steed barding */
                m_useup(otmp->ocarry, otmp);
            }
            break;
        }
        s = scursed ? -1
                    : (otmp->spe >= 9)
                       ? (rn2(otmp->spe) == 0)
                       : sblessed
                          ? rnd(3 - otmp->spe / 3)
                          : 1;
        pline("%s %s%s%s%s for a %s.", Yname2(otmp),
              s == 0 ? "violently " : "",
              otense(otmp, Blind ? "vibrate" : "glow"),
              (!Blind && !same_color) ? " " : "",
              (Blind || same_color)
                 ? "" : hcolor(scursed ? NH_BLACK : NH_SILVER),
              (s * s > 1) ? "while" : "moment");
        /* [this cost handling will need updating if shop pricing is
           ever changed to care about curse/bless status of armor] */
        if (s < 0)
            costly_alteration(otmp, COST_DECHNT);
        if (scursed && !otmp->cursed)
            curse(otmp);
        else if (sblessed && !otmp->blessed)
            bless(otmp);
        else if (!scursed && otmp->cursed)
            uncurse(otmp);
        if (s) {
            otmp->spe += s;
            adj_abon(otmp, s);
            known = otmp->known;
            /* update shop bill to reflect new higher price */
            if (s > 0 && otmp->unpaid)
                alter_cost(otmp, 0L);
        }

        if ((otmp->spe > (special_armor ? 5 : 3))
            && (special_armor || !rn2(7)))
            pline("%s %s.", Yobjnam2(otmp, "suddenly vibrate"),
                  Blind ? "again" : "unexpectedly");
        break;
    }
    case SCR_DESTROY_ARMOR: {
        otmp = some_armor(&youmonst);
        if (confused) {
            if (!otmp) {
                strange_feeling(sobj, "Your bones itch.");
                sobj = 0; /* useup() in strange_feeling() */
                exercise(A_STR, FALSE);
                exercise(A_CON, FALSE);
                break;
            }
            /* naturally fixed armors don't care about oerodeproof */
            if (is_supermaterial(otmp)) {
                if (!Blind)
                    pline("%s for a moment.", Yobjnam2(otmp, "shimmer"));
                break;
            } else {
                old_erodeproof = (otmp->oerodeproof != 0);
                new_erodeproof = scursed;
                otmp->oerodeproof = 0; /* for messages */
                p_glow2(otmp, NH_PURPLE);
                if (old_erodeproof && !new_erodeproof) {
                    /* restore old_erodeproof before shop charges */
                    otmp->oerodeproof = 1;
                    costly_alteration(otmp, COST_DEGRD);
                }
                otmp->oerodeproof = new_erodeproof ? 1 : 0;
                break;
            }
        }
        if (!scursed || !otmp || !otmp->cursed) {
            if (!destroy_arm(otmp)) {
                strange_feeling(sobj, "Your skin itches.");
                sobj = 0; /* useup() in strange_feeling() */
                exercise(A_STR, FALSE);
                exercise(A_CON, FALSE);
                break;
            } else
                known = TRUE;
        } else { /* armor and scroll both cursed */
            pline("%s.", Yobjnam2(otmp, "vibrate"));
            if (otmp->spe >= -6) {
                otmp->spe += -1;
                adj_abon(otmp, -1);
            }
            make_stunned((HStun & TIMEOUT) + (long) rn1(10, 10), TRUE);
        }
    } break;
    case SCR_CONFUSE_MONSTER:
    case SPE_CONFUSE_MONSTER:
        if (youmonst.data->mlet != S_HUMAN || scursed) {
            if (!HConfusion)
                You_feel("confused.");
            make_confused(HConfusion + rnd(100), FALSE);
        } else if (confused) {
            if (!sblessed) {
                Your("%s begin to %s%s.", makeplural(body_part(HAND)),
                     Blind ? "tingle" : "glow ",
                     Blind ? "" : hcolor(NH_PURPLE));
                make_confused(HConfusion + rnd(100), FALSE);
            } else {
                pline("A %s%s surrounds your %s.",
                      Blind ? "" : hcolor(NH_RED),
                      Blind ? "faint buzz" : " glow", body_part(HEAD));
                make_confused(0L, TRUE);
            }
        } else {
            if (!sblessed) {
                Your("%s%s %s%s.", makeplural(body_part(HAND)),
                     Blind ? "" : " begin to glow",
                     Blind ? (const char *) "tingle" : hcolor(NH_RED),
                     u.umconf ? " even more" : "");
                u.umconf++;
            } else {
                if (Blind)
                    Your("%s tingle %s sharply.", makeplural(body_part(HAND)),
                         u.umconf ? "even more" : "very");
                else
                    Your("%s glow a%s brilliant %s.",
                         makeplural(body_part(HAND)),
                         u.umconf ? "n even more" : "", hcolor(NH_RED));
                /* after a while, repeated uses become less effective */
                if (u.umconf >= 40)
                    u.umconf++;
                else
                    u.umconf += rn1(8, 2);
            }
        }
        break;
    case SCR_SCARE_MONSTER:
    case SPE_CAUSE_FEAR: {
        int ct = 0;
        struct monst *mtmp;

        if ((otyp == SCR_SCARE_MONSTER || !ct) && !Deaf)
            You_hear("%s %s.", (confused || scursed) ? "sad wailing"
                                                     : "maniacal laughter",
                     !ct ? "in the distance" : "close by");

        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
            if (DEADMONSTER(mtmp))
                continue;
            if (cansee(mtmp->mx, mtmp->my)) {
                if (confused || scursed) {
                    mtmp->mflee = mtmp->mfrozen = mtmp->msleeping = 0;
		    if (!mtmp->mstone || mtmp->mstone > 2)
			mtmp->mcanmove = 1;
                } else if (!resist(mtmp, sobj->oclass, 0, NOTELL)
                    /* some monsters are immune */
                    && onscary(0, 0, mtmp))
                    monflee(mtmp, 0, FALSE, TRUE);
                if (!mtmp->mtame)
                    ct++; /* pets don't laugh at you */
            }
        }
        break;
    }
    case SCR_BLANK_PAPER:
        if (Blind)
            You("don't remember there being any magic words on this scroll.");
        else
            pline("This scroll seems to be blank.");
        known = TRUE;
        break;
    case SCR_REMOVE_CURSE:
    case SPE_REMOVE_CURSE: {
        register struct obj *obj;

        You_feel(!Hallucination
                     ? (!confused ? "like someone is helping you."
                                  : "like you need some help.")
                     : (!confused ? "in touch with the Universal Oneness."
                                  : "the power of the Force against you!"));

        if (scursed) {
            pline_The("scroll disintegrates.");
        } else {
            for (obj = invent; obj; obj = obj->nobj) {
                long wornmask;

                /* gold isn't subject to cursing and blessing */
                if (obj->oclass == COIN_CLASS)
                    continue;
                /* hide current scroll from itself so that perm_invent won't
                   show known blessed scroll losing bknown when confused */
                if (obj == sobj && obj->quan == 1L)
                    continue;
                wornmask = (obj->owornmask & ~(W_BALL | W_ART | W_ARTI));
                if (wornmask && !sblessed) {
                    /* handle a couple of special cases; we don't
                       allow auxiliary weapon slots to be used to
                       artificially increase number of worn items */
                    if (obj == uswapwep) {
                        if (!u.twoweap)
                            wornmask = 0L;
                    } else if (obj == uquiver) {
                        if (obj->oclass == WEAPON_CLASS) {
                            /* mergeable weapon test covers ammo,
                               missiles, spears, daggers & knives */
                            if (!objects[obj->otyp].oc_merge)
                                wornmask = 0L;
                        } else if (obj->oclass == GEM_CLASS) {
                            /* possibly ought to check whether
                               alternate weapon is a sling... */
                            if (!uslinging())
                                wornmask = 0L;
                        } else {
                            /* weptools don't merge and aren't
                               reasonable quivered weapons */
                            wornmask = 0L;
                        }
                    }
                }
                if (sblessed || wornmask || obj->otyp == LOADSTONE
                    || (obj->otyp == LEASH && obj->leashmon)) {
                    /* water price varies by curse/bless status */
                    boolean shop_h2o = (obj->unpaid && obj->otyp == POT_WATER);

                    if (confused) {
                        blessorcurse(obj, 2);
                        /* lose knowledge of this object's curse/bless
                           state (even if it didn't actually change) */
                        obj->bknown = 0;
                        /* blessorcurse() only affects uncursed items
                           so no need to worry about price of water
                           going down (hence no costly_alteration) */
                        if (shop_h2o && (obj->cursed || obj->blessed))
                            alter_cost(obj, 0L); /* price goes up */
                    } else if (obj->cursed) {
                        if (shop_h2o)
                            costly_alteration(obj, COST_UNCURS);
                        uncurse(obj);
                        /* if the object was known to be cursed and is now
                           known not to be, make the scroll known; it's
                           trivial to identify anyway by comparing inventory
                           before and after */
                        if (obj->bknown && otyp == SCR_REMOVE_CURSE)
                            learnscrolltyp(SCR_REMOVE_CURSE);
                    }
                }
            }
            /* if riding, treat steed's saddle as if part of hero's invent */
            if (u.usteed && (obj = which_armor(u.usteed, W_SADDLE)) != 0) {
                if (confused) {
                    blessorcurse(obj, 2);
                    obj->bknown = 0; /* skip set_bknown() */
                } else if (obj->cursed) {
                    uncurse(obj);
                    /* like rndcurse(sit.c), effect on regular inventory
                       doesn't show things glowing but saddle does */
                    if (!Blind) {
                        pline("%s %s.", Yobjnam2(obj, "glow"),
                              hcolor("amber"));
                        obj->bknown = Hallucination ? 0 : 1;
                    } else {
                        obj->bknown = 0; /* skip set_bknown() */
                    }
                }
            }
            /* same for barding */
            if (u.usteed && (obj = which_armor(u.usteed, W_BARDING)) != 0) {
                if (confused) {
                    blessorcurse(obj, 2);
                    obj->bknown = 0; /* skip set_bknown() */
                } else if (obj->cursed) {
                    uncurse(obj);
                    /* like rndcurse(sit.c), effect on regular inventory
                       doesn't show things glowing but barding does */
                    if (!Blind) {
                        pline("%s %s.", Yobjnam2(obj, "glow"),
                              hcolor("amber"));
                        obj->bknown = Hallucination ? 0 : 1;
                    } else {
                        obj->bknown = 0; /* skip set_bknown() */
                    }
                }
            }
        }
        if (Punished && !confused)
            unpunish();
        if (u.utrap && u.utraptype == TT_BURIEDBALL) {
            buried_ball_to_freedom();
            pline_The("clasp on your %s vanishes.", body_part(LEG));
        }
        update_inventory();
        break;
    }
    case SCR_CREATE_MONSTER:
    case SPE_CREATE_MONSTER:
        if (create_critters(1 + ((confused || scursed) ? 12 : 0)
                                + ((sblessed || rn2(73)) ? 0 : rnd(4)),
                            confused ? &mons[PM_ACID_BLOB]
                                     : (struct permonst *) 0,
                            FALSE))
            known = TRUE;
        /* no need to flush monsters; we ask for identification only if the
         * monsters are not visible
         */
        break;
    case SCR_ENCHANT_WEAPON:
        /* [What about twoweapon mode?  Proofing/repairing/enchanting both
           would be too powerful, but shouldn't we choose randomly between
           primary and secondary instead of always acting on primary?] */
        if (confused && uwep && erosion_matters(uwep)) {
            old_erodeproof = (uwep->oerodeproof != 0);
            new_erodeproof = !scursed;
            uwep->oerodeproof = 0; /* for messages */
            if (new_erodeproof && (uwep->oeroded || uwep->oeroded2)) {
                uwep->oeroded = uwep->oeroded2 = 0;
                pline("%s as good as new!",
                      Yobjnam2(uwep, Blind ? "feel" : "look"));
            }
            if (is_supermaterial(uwep)) {
                if (!Blind)
                    pline("%s for a moment.", Yobjnam2(uwep, "shimmer"));
                break;
            }
            if (Blind) {
                uwep->rknown = FALSE;
                pline("%s warm for a moment.", Yobjnam2(uwep, "feel"));
            } else {
                uwep->rknown = TRUE;
                pline("%s covered by a %s %s %s!", Yobjnam2(uwep, "are"),
                      scursed ? "mottled" : "shimmering",
                      hcolor(scursed ? NH_PURPLE : NH_GOLDEN),
                      scursed ? "glow" : "shield");
            }
            if (old_erodeproof && !new_erodeproof) {
                /* restore old_erodeproof before shop charges */
                uwep->oerodeproof = 1;
                costly_alteration(uwep, COST_DEGRD);
            }
            uwep->oerodeproof = new_erodeproof ? 1 : 0;
            break;
        }
        if (!chwepon(sobj, scursed ? -1
                             : !uwep ? 1
                               : (uwep->spe >= 9) ? !rn2(uwep->spe)
                                 : sblessed ? rnd(3 - uwep->spe / 3)
                                   : 1))
            sobj = 0; /* nothing enchanted: strange_feeling -> useup */
        break;
    case SCR_TAMING:
    case SPE_CHARM_MONSTER: {
        int candidates, res, results, vis_results;

        if (u.uswallow) {
            candidates = 1;
            results = vis_results = maybe_tame(u.ustuck, sobj);
        } else {
            int i, j, bd = confused ? 5 : 1;
            struct monst *mtmp;

            /* note: maybe_tame() can return either positive or
               negative values, but not both for the same scroll */
            candidates = results = vis_results = 0;
            for (i = -bd; i <= bd; i++)
                for (j = -bd; j <= bd; j++) {
                    if (!isok(u.ux + i, u.uy + j))
                        continue;
                    if ((mtmp = m_at(u.ux + i, u.uy + j)) != 0
                        || (!i && !j && (mtmp = u.usteed) != 0)) {
                        ++candidates;
                        res = maybe_tame(mtmp, sobj);
                        results += res;
                        if (canspotmon(mtmp))
                            vis_results += res;
                    }
                }
        }
        if (!results) {
            pline("Nothing interesting %s.",
                  !candidates ? "happens" : "seems to happen");
        } else {
            pline_The("neighborhood %s %sfriendlier.",
                      vis_results ? "is" : "seems",
                      (results < 0) ? "un" : "");
            if (vis_results > 0)
                known = TRUE;
        }
        break;
    }
    case SCR_GENOCIDE:
        if (!already_known)
            You("have found a scroll of genocide!");
        known = TRUE;
        if (sblessed)
            do_class_genocide();
        else
            do_genocide((!scursed) | (2 * !!Confusion));
        break;
    case SCR_LIGHT:
        if (!confused || rn2(5)) {
            if (!Blind)
                known = TRUE;
            litroom(!confused && !scursed, FALSE, sobj, u.ux, u.uy);
            if (!confused && !scursed) {
                if (lightdamage(sobj, TRUE, 5))
                    known = TRUE;
            }
        } else {
            /* could be scroll of create monster, don't set known ...*/
            (void) create_critters(1, !scursed ? &mons[PM_YELLOW_LIGHT]
                                               : &mons[PM_BLACK_LIGHT],
                                   TRUE);
        }
        break;
    case SCR_TELEPORTATION:
        if (confused || scursed) {
            level_tele();
        } else {
            known = scrolltele(sobj);
        }
        break;
    case SCR_GOLD_DETECTION:
        if ((confused || scursed) ? trap_detect(sobj) : gold_detect(sobj))
            sobj = 0; /* failure: strange_feeling() -> useup() */
        break;
    case SCR_FOOD_DETECTION:
    case SPE_DETECT_FOOD:
        if (food_detect(sobj))
            sobj = 0; /* nothing detected: strange_feeling -> useup */
        break;
    case SCR_IDENTIFY:
        /* known = TRUE; -- handled inline here */
        /* use up the scroll first, before makeknown() performs a
           perm_invent update; also simplifies empty invent check */
        useup(sobj);
        sobj = 0; /* it's gone */
        if (confused)
            You("identify this as an identify scroll.");
        else if (!already_known || !invent)
            /* force feedback now if invent became
               empty after using up this scroll */
            pline("This is an identify scroll.");
        if (!already_known)
            (void) learnscrolltyp(SCR_IDENTIFY);
        /*FALLTHRU*/
    case SPE_IDENTIFY:
        cval = 1;
        if (sblessed || (!scursed && !rn2(5))) {
            cval = rn2(5);
            /* note: if cval==0, identify all items */
            if (cval == 1 && sblessed && Luck > 0)
                ++cval;
        }
        if (invent && !confused) {
            identify_pack(cval, !already_known);
        } else if (otyp == SPE_IDENTIFY) {
            /* when casting a spell we know we're not confused,
               so inventory must be empty (another message has
               already been given above if reading a scroll) */
            pline("You're not carrying anything to be identified.");
        }
        break;
    case SCR_CHARGING:
        if (confused) {
            if (scursed) {
                You_feel("discharged.");
                u.uen = 0;
            } else {
                You_feel("charged up!");
                u.uen += d(sblessed ? 6 : 4, 4);
                if (u.uen > u.uenmax) /* if current energy is already at   */
                    u.uenmax = u.uen; /* or near maximum, increase maximum */
                else
                    u.uen = u.uenmax; /* otherwise restore current to max  */
            }
            context.botl = 1;
            break;
        }
        /* known = TRUE; -- handled inline here */
        if (!already_known) {
            pline("This is a charging scroll.");
            learnscroll(sobj);
        }
        /* use it up now to prevent it from showing in the
           getobj picklist because the "disappears" message
           was already delivered */
        useup(sobj);
        sobj = 0; /* it's gone */
        otmp = getobj(all_count, "charge");
        if (otmp)
            recharge(otmp, scursed ? -1 : sblessed ? 1 : 0, &youmonst);
        break;
    case SCR_MAGIC_MAPPING:
        if (level.flags.nommap) {
            Your("mind is filled with crazy lines!");
            if (Hallucination)
                pline("Wow!  Modern art.");
            else
                Your("%s spins in bewilderment.", body_part(HEAD));
            make_confused(HConfusion + rnd(30), FALSE);
            break;
        }
        if (sblessed) {
            register int x, y;

            for (x = 1; x < COLNO; x++)
                for (y = 0; y < ROWNO; y++)
                    if (levl[x][y].typ == SDOOR)
                        cvt_sdoor_to_door(&levl[x][y]);
            /* do_mapping() already reveals secret passages */
        }
        known = TRUE;
        /*FALLTHRU*/
    case SPE_MAGIC_MAPPING:
        if (level.flags.nommap) {
            Your("%s spins as %s blocks the spell!", body_part(HEAD),
                 something);
            make_confused(HConfusion + rnd(30), FALSE);
            break;
        }
        pline("A map coalesces in your mind!");
        cval = (scursed && !confused);
        if (cval)
            HConfusion = 1; /* to screw up map */
        do_mapping();
        if (cval) {
            HConfusion = 0; /* restore */
            pline("Unfortunately, you can't grasp the details.");
        }
        break;
    case SCR_AMNESIA:
        known = TRUE;
        if (sblessed && ACURR(A_INT) > 12) {
            if (yn("Do you want to reset your skills training?") != 'y') {
                goto forget_routine;
            } else {
                Your("skills training has been reset.");
                reset_weapon_skills();
                break;
            }
        } else {
 forget_routine:
            if (maybe_polyd(is_illithid(youmonst.data), Race_if(PM_ILLITHID))) {
                Your("psionic abilities ward off the scroll's magic.");
                break;
            } else {
                forget((!sblessed ? ALL_SPELLS : 0));
                if (Hallucination) /* Ommmmmm! */
                    Your("mind releases itself from mundane concerns.");
                else if (!strncmpi(plname, "Maud", 4))
                    pline("As your mind turns inward on itself, you forget everything else.");
                else if (rn2(2))
                    pline("Who was that Maud person anyway?");
                else
                    pline("Thinking of Maud you forget everything else.");
                exercise(A_WIS, FALSE);
            }
        }
        break;
    case SCR_FIRE: {
        coord cc;
        int dam;

        cc.x = u.ux;
        cc.y = u.uy;
        cval = bcsign(sobj);
        dam = (2 * (rn1(3, 3) + 2 * cval) + 1) / 3;
        useup(sobj);
        sobj = 0; /* it's gone */
        if (!already_known)
            (void) learnscrolltyp(SCR_FIRE);
        if (confused) {
            if (how_resistant(FIRE_RES) == 100) {
                shieldeff(u.ux, u.uy);
                monstseesu(M_SEEN_FIRE);
                if (!Blind)
                    pline("Oh, look, what a pretty fire in your %s.",
                          makeplural(body_part(HAND)));
                else
                    You_feel("a pleasant warmth in your %s.",
                             makeplural(body_part(HAND)));
            } else {
                pline_The("scroll catches fire and you burn your %s.",
                          makeplural(body_part(HAND)));
                losehp(1, "scroll of fire", KILLED_BY_AN);
            }
            break;
        }
        if (Underwater) {
            pline_The("%s around you vaporizes violently!", hliquid("water"));
        } else {
            if (sblessed) {
                if (!already_known)
                    pline("This is a scroll of fire!");
                dam *= 5;
                pline("Where do you want to center the explosion?");
                getpos_sethilite(display_stinking_cloud_positions,
                                 get_valid_stinking_cloud_pos);
                (void) getpos(&cc, TRUE, "the desired position");
                if (!is_valid_stinking_cloud_pos(cc.x, cc.y, FALSE)) {
                    /* try to reach too far, get burned */
                    cc.x = u.ux;
                    cc.y = u.uy;
                }
            }
            if (cc.x == u.ux && cc.y == u.uy) {
                pline_The("scroll erupts in a tower of flame!");
                iflags.last_msg = PLNMSG_TOWER_OF_FLAME; /* for explode() */
                burn_away_slime();
            }
        }
        explode(cc.x, cc.y, ZT_SPELL(ZT_FIRE), dam, SCROLL_CLASS, EXPL_FIERY);
        break;
    }
    case SCR_EARTH:
        /* TODO: handle steeds */
        if (!Is_rogue_level(&u.uz) && has_ceiling(&u.uz)
            && (!In_endgame(&u.uz) || Is_earthlevel(&u.uz))) {
            register int x, y;
            int nboulders = 0;

            /* Identify the scroll */
            if (u.uswallow)
                You_hear("rumbling.");
            else
                pline_The("%s rumbles %s you!", ceiling(u.ux, u.uy),
                          sblessed ? "around" : "above");
            known = 1;
            sokoban_guilt();

            /* Loop through the surrounding squares */
            if (!scursed)
                for (x = u.ux - 1; x <= u.ux + 1; x++) {
                    for (y = u.uy - 1; y <= u.uy + 1; y++) {
                        /* Is this a suitable spot? */
                        if (isok(x, y) && !closed_door(x, y)
                            && !IS_ROCK(levl[x][y].typ)
                            && !IS_AIR(levl[x][y].typ)
                            && (x != u.ux || y != u.uy)) {
                            nboulders +=
                                drop_boulder_on_monster(x, y, confused, TRUE);
                        }
                    }
                }
            /* Attack the player */
            if (!sblessed) {
                drop_boulder_on_player(confused, TRUE, TRUE, FALSE);
            } else if (!nboulders)
                pline("But nothing else happens.");
        }
        break;
    case SCR_PUNISHMENT:
        known = TRUE;
        if (confused || sblessed) {
            You_feel("culpable.");
            break;
        }
        punish(sobj);
        break;
    case SCR_STINKING_CLOUD: {
        coord cc;

        if (!already_known)
            You("have found a scroll of stinking cloud!");
        known = TRUE;
        pline("Where do you want to center the %scloud?",
              already_known ? "stinking " : "");
        cc.x = u.ux;
        cc.y = u.uy;
        getpos_sethilite(display_stinking_cloud_positions,
                         get_valid_stinking_cloud_pos);
        if (getpos(&cc, TRUE, "the desired position") < 0) {
            pline1(Never_mind);
            break;
        }
        if (!is_valid_stinking_cloud_pos(cc.x, cc.y, TRUE))
            break;
        (void) create_gas_cloud(cc.x, cc.y, 3 + bcsign(sobj),
                                8 + 4 * bcsign(sobj));
        break;
    }
    case SCR_MAGIC_DETECTION:
        if (magic_detect(sobj))
            return 1;
        known = TRUE;
        break;
    default:
        impossible("What weird effect is this? (%u)", otyp);
    }
    /* if sobj is gone, we've already called useup() above and the
       update_inventory() that it performs might have come too soon
       (before charging an item, for instance) */
    if (!sobj)
        update_inventory();
    return sobj ? 0 : 1;
}

void
drop_boulder_on_player(confused, helmet_protects, byu, skip_uswallow)
boolean confused, helmet_protects, byu, skip_uswallow;
{
    int dmg;
    struct obj *otmp2;

    /* hit monster if swallowed */
    if (u.uswallow && !skip_uswallow) {
        drop_boulder_on_monster(u.ux, u.uy, confused, byu);
        return;
    }

    otmp2 = mksobj(confused ? ROCK : BOULDER, FALSE, FALSE);
    if (!otmp2)
        return;
    otmp2->quan = confused ? rn1(5, 2) : 1;
    otmp2->owt = weight(otmp2);
    if (!amorphous(youmonst.data) && !Passes_walls
        && !noncorporeal(youmonst.data) && !unsolid(youmonst.data)) {
        You("are hit by %s!", doname(otmp2));
        dmg = dmgval(otmp2, &youmonst) * otmp2->quan;
        if (uarmh && helmet_protects) {
            if (otmp2->owt >= 400 && is_glass(uarmh)
                && break_glass_obj(uarmh)) {
                ;
            } else if (is_hard(uarmh)) {
                if (otmp2->owt >= 400) {
                    if (dmg > 2)
                        dmg -= 2;
                    Your("helmet only slightly protects you.");
                } else {
                    if (dmg > 2)
                        dmg = 2;
                    pline("Fortunately, you are wearing a hard helmet.");
                }
            } else if (flags.verbose) {
                pline("%s does not protect you.", Yname2(uarmh));
            }
        }
    } else
        dmg = 0;
    wake_nearto(u.ux, u.uy, 4 * 4);
    /* Must be before the losehp(), for bones files */
    if (!flooreffects(otmp2, u.ux, u.uy, "fall")) {
        place_object(otmp2, u.ux, u.uy);
        stackobj(otmp2);
        newsym(u.ux, u.uy);
    }
    if (dmg)
        losehp(Maybe_Half_Phys(dmg), "scroll of earth", KILLED_BY_AN);
}

boolean
drop_boulder_on_monster(x, y, confused, byu)
int x, y;
boolean confused, byu;
{
    register struct obj *otmp2;
    register struct monst *mtmp;

    /* Make the object(s) */
    otmp2 = mksobj(confused ? ROCK : BOULDER, FALSE, FALSE);
    if (!otmp2)
        return FALSE; /* Shouldn't happen */
    otmp2->quan = confused ? rn1(5, 2) : 1;
    otmp2->owt = weight(otmp2);

    /* Find the monster here (won't be player) */
    mtmp = m_at(x, y);
    if (mtmp && !amorphous(mtmp->data) && !passes_walls(mtmp->data)
        && !noncorporeal(mtmp->data) && !unsolid(mtmp->data)) {
        struct obj *helmet = which_armor(mtmp, W_ARMH);
        int mdmg;

        if (cansee(mtmp->mx, mtmp->my)) {
            pline("%s is hit by %s!", Monnam(mtmp), doname(otmp2));
            if (mtmp->minvis && !canspotmon(mtmp))
                map_invisible(mtmp->mx, mtmp->my);
        } else if (u.uswallow && mtmp == u.ustuck)
            You_hear("something hit %s %s over your %s!",
                     s_suffix(mon_nam(mtmp)), mbodypart(mtmp, STOMACH),
                     body_part(HEAD));

        mdmg = dmgval(otmp2, mtmp) * otmp2->quan;
        if (helmet) {
            if (otmp2->owt >= 400 && is_glass(helmet)
                && break_glass_obj(uarmh)) {
                ;
            } else if (is_hard(helmet)) {
                if (otmp2->owt >= 400) {
                    if (mdmg > 2)
                        mdmg -= 2;
                    pline("%s helmet is only slightly protective.",
                          s_suffix(Monnam(mtmp)));
                } else {
                    if (mdmg > 2)
                        mdmg = 2;
                    if (canspotmon(mtmp))
                        pline("Fortunately, %s is wearing a hard helmet.",
                              mon_nam(mtmp));
                    else if (!Deaf)
                        You_hear("a clanging sound.");
                }
            } else {
                if (canspotmon(mtmp))
                    pline("%s's %s does not protect %s.", Monnam(mtmp),
                          xname(helmet), mhim(mtmp));
            }
        }
        damage_mon(mtmp, mdmg, AD_PHYS);
        if (DEADMONSTER(mtmp)) {
            if (byu) {
                killed(mtmp);
            } else {
                pline("%s is killed.", Monnam(mtmp));
                mondied(mtmp);
            }
        } else {
            wakeup(mtmp, byu);
        }
        wake_nearto(x, y, 4 * 4);
    } else if (u.uswallow && mtmp == u.ustuck) {
        obfree(otmp2, (struct obj *) 0);
        /* fall through to player */
        drop_boulder_on_player(confused, TRUE, FALSE, TRUE);
        return 1;
    }
    /* Drop the rock/boulder to the floor */
    if (!flooreffects(otmp2, x, y, "fall")) {
        place_object(otmp2, x, y);
        stackobj(otmp2);
        newsym(x, y); /* map the rock */
    }
    return TRUE;
}

/* overcharging any wand or zapping/engraving cursed wand */
void
wand_explode(obj, chg)
struct obj *obj;
int chg; /* recharging */
{
    const char *expl = !chg ? "suddenly" : "vibrates violently and";
    int dmg, n, k;

    /* number of damage dice */
    if (!chg)
        chg = 2; /* zap/engrave adjustment */
    n = obj->spe + chg;
    if (n < 2)
        n = 2; /* arbitrary minimum */
    /* size of damage dice */
    switch (obj->otyp) {
    case WAN_WISHING:
        k = 12;
        break;
    case WAN_CANCELLATION:
    case WAN_DEATH:
    case WAN_POLYMORPH:
    case WAN_UNDEAD_TURNING:
        k = 10;
        break;
    case WAN_COLD:
    case WAN_FIRE:
    case WAN_LIGHTNING:
    case WAN_MAGIC_MISSILE:
        k = 8;
        break;
    case WAN_NOTHING:
        k = 4;
        break;
    default:
        k = 6;
        break;
    }
    /* inflict damage and destroy the wand */
    dmg = d(n, k);
    obj->in_use = TRUE; /* in case losehp() is fatal (or --More--^C) */
    pline("%s %s explodes!", Yname2(obj), expl);
    losehp(Maybe_Half_Phys(dmg), "exploding wand", KILLED_BY_AN);
    useup(obj);
    /* obscure side-effect */
    exercise(A_STR, FALSE);
}

void
mwand_explode(mon, obj)
register struct monst *mon;
register struct obj *obj;
{
    if (canseemon(mon))
        pline("%s %s vibrates violently and explodes!",
              s_suffix(Monnam(mon)), xname(obj));
    else if (!Deaf)
        You_hear("an explosion.");
    mon->mhp -= rnd(2 * (mon->mhpmax + 1) / 3);
    m_useup(mon, obj);
    if (mon->mhp <= 0) {
    	if (canseemon(mon))
            pline("%s is killed by the explosion!", Monnam(mon));
        mondied(mon);
    }
}

/* used to collect light haters being hit by light so that they can be
   processed after vision for the entire lit area has been brought up to date */
struct litmon {
    struct monst *mon;
    struct litmon *nxt;
};
STATIC_VAR struct litmon *light_haters = 0;

/*
 * Low-level lit-field update routine.
 */
STATIC_PTR void
set_lit(x, y, val)
int x, y;
genericptr_t val;
{
    struct monst *mtmp;
    struct litmon *target;

    if (val) {
        levl[x][y].lit = 1;
        if ((mtmp = m_at(x, y)) != 0 && hates_light(r_data(mtmp))) {
            target = (struct litmon *) alloc(sizeof *target);
            target->mon = mtmp;
            target->nxt = light_haters;
            light_haters = target;
        }
    } else {
        levl[x][y].lit = 0;
        snuff_light_source(x, y);
    }
}

void
litroom(on, mon, obj, x, y)
boolean on;      /* True: make nearby area lit; False: cursed scroll */
boolean mon;     /* True: source comes from a monster's ability; False: magical object */
struct obj *obj; /* scroll, spellbook (for spell), or wand of light */
xchar x, y;      /* coordinates for centering do_clear_area() */
{
    struct obj *otmp;
    struct monst *mtmp;
    boolean blessed_effect = (obj && obj->oclass == SCROLL_CLASS
                              && obj->blessed);
    char is_lit = 0; /* value is irrelevant but assign something anyway; its
                      * address is used as a 'not null' flag for set_lit() */
    boolean you = (x == u.ux && y == u.uy);

    /* update object lights and produce message (provided you're not blind) */
    if (!on) {
        int still_lit = 0;

        /*
         * The magic douses lamps,&c too and might curse artifact lights,
         * if the player is in the area of affect.
         */
        if (!mon || (distu(x,y) <= 25 && clear_path(x, y, u.ux, u.uy))) {
            for (otmp = invent; otmp; otmp = otmp->nobj) {
                boolean lamp = (otmp->otyp == MAGIC_LAMP && otmp->cursed);
                boolean staff = (otmp->oartifact == ART_STAFF_OF_THE_ARCHMAGI
                                 && !Upolyd && Race_if(PM_DROW));
                boolean armor = (Is_dragon_armor(otmp)
                                 && Dragon_armor_to_scales(otmp) == SHADOW_DRAGON_SCALES);

                if (otmp->lamplit) {
                    if (lamp || staff || armor) {
                        continue;
                    } else {
                        if (!artifact_light(otmp))
                            (void) snuff_lit(otmp);
                        else if (!mon)
                            /* wielded Sunsword or worn shield of light/gold dragon
                            scales; maybe lower its BUC state if not already
                            cursed */
                            impact_arti_light(otmp, TRUE, (boolean) !Blind);

                        if (otmp->lamplit)
                            ++still_lit;
                    }
                }
            }
        }

        /* scroll of light becomes discovered when not blind, so some
           message to justify that is needed */
        if (!Blind) {
            /* for the still_lit case, we don't know at this point whether
               anything currently visibly lit is going to go dark; if this
               message came after the darkening, we could count visibly
               lit squares before and after to know; we do know that being
               swallowed won't be affected--the interior is still lit */
            if (still_lit && !mon) {
                pline_The("ambient light seems dimmer.");
            } else if (u.uswallow) {
                pline("It seems even darker in here than before.");
            } else {
                if (you) {
                    You("are surrounded by darkness!");
                } else {
                    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
                        if (!DEADMONSTER(mtmp)
                            && (x == mtmp->mx) && (y == mtmp->my))
                            if (canseemon(mtmp))
                                pline("%s is surrounded by darkness!",
                                      Monnam(mtmp));
                    }
                }
            }
        }
    } else { /* on */
        if (blessed_effect) {
            /* might bless artifact lights; could put out cursed
               magic lamps (darkness) */
            for (otmp = invent; otmp; otmp = otmp->nobj) {
                boolean lamp = (otmp->otyp == MAGIC_LAMP && otmp->cursed);
                boolean staff = (otmp->oartifact == ART_STAFF_OF_THE_ARCHMAGI
                                 && !Upolyd && Race_if(PM_DROW));
                boolean armor = (Is_dragon_armor(otmp)
                                 && Dragon_armor_to_scales(otmp) == SHADOW_DRAGON_SCALES);

                if (otmp->lamplit && !mon) {
                    if (!artifact_light(otmp) && lamp) {
                        (void) snuff_lit(otmp);
                    } else {
                        /* wielded Sunsword or worn shield of light/gold dragon
                           scales; maybe raise its BUC state if not already
                           blessed */
                        if (!(staff || armor))
                            impact_arti_light(otmp, FALSE, (boolean) !Blind);
                    }
                }
            }
        }
        if (u.uswallow) {
            if (Blind)
                ; /* no feedback */
            else if (is_swallower(u.ustuck->data))
                pline("%s %s is lit.", s_suffix(Monnam(u.ustuck)),
                      mbodypart(u.ustuck, STOMACH));
            else if (is_whirly(u.ustuck->data))
                pline("%s shines briefly.", Monnam(u.ustuck));
            else
                pline("%s glistens.", Monnam(u.ustuck));
        } else if (!Blind) {
            if (you) {
                pline("A lit field surrounds you!");
            } else {
                for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
                    if (!DEADMONSTER(mtmp)
                        && (x == mtmp->mx) && (y == mtmp->my))
                        if (canseemon(mtmp))
                            pline("A lit field surrounds %s!",
                                  mon_nam(mtmp));
                }
            }
        }
    }

    /* No-op when swallowed or in water */
    if (u.uswallow || Underwater || Is_waterlevel(&u.uz))
        return;
    /*
     *  If we are darkening the room and the hero is punished but not
     *  blind, then we have to pick up and replace the ball and chain so
     *  that we don't remember them if they are out of sight.
     */
    if (Punished && !on && !Blind)
        move_bc(1, 0, uball->ox, uball->oy, uchain->ox, uchain->oy);

    if (Is_rogue_level(&u.uz)) {
        /* Can't use do_clear_area because MAX_RADIUS is too small */
        /* rogue lighting must light the entire room */
        int rnum = levl[u.ux][u.uy].roomno - ROOMOFFSET;
        int rx, ry;

        if (rnum >= 0) {
            for (rx = rooms[rnum].lx - 1; rx <= rooms[rnum].hx + 1; rx++)
                for (ry = rooms[rnum].ly - 1;
                     ry <= rooms[rnum].hy + 1; ry++)
                    set_lit(rx, ry,
                            (genericptr_t) (on ? &is_lit : (char *) 0));
            rooms[rnum].rlit = on;
        }
        /* hallways remain dark on the rogue level */
    } else
        do_clear_area(x, y, blessed_effect ? 9 : 5,
                      set_lit, (genericptr_t) (on ? &is_lit : (char *) 0));

    /*
     *  If we are not blind, then force a redraw on all positions in sight
     *  by temporarily blinding the hero.  The vision recalculation will
     *  correctly update all previously seen positions *and* correctly
     *  set the waslit bit [could be messed up from above].
     */
    if (on)
        blindingflash();

    if (!Blind) {
        vision_recalc(2);

        /* replace ball&chain */
        if (Punished && !on)
            move_bc(0, 0, uball->ox, uball->oy, uchain->ox, uchain->oy);
    }

    vision_full_recalc = 1; /* delayed vision recalculation */
    if (light_haters) {
        struct litmon *target;

        /* can't delay vision recalc after all */
        vision_recalc(0);
        /* after vision has been updated, monsters who are affected
           when hit by light can now be hit by it */
        do {
            target = light_haters;
            light_haters = target->nxt;
            light_hits_light_hater(target->mon, rnd(5));
            free((genericptr_t) target);
        } while (light_haters);
    }
    return;
}

STATIC_OVL void
do_class_genocide()
{
    int i, j, immunecnt, gonecnt, goodcnt, class, feel_dead = 0, ll_done = 0;
    int killed, candidates;
    char buf[BUFSZ] = DUMMY;
    boolean gameover = FALSE; /* true iff killed self */

    for (j = 0;; j++) {
        if (j >= 5) {
            pline1(thats_enough_tries);
            return;
        }
        do {
            getlin("What class of monsters do you wish to genocide?", buf);
            (void) mungspaces(buf);
        } while (!*buf);
        /* choosing "none" preserves genocideless conduct */
        if (*buf == '\033' || !strcmpi(buf, "none")
            || !strcmpi(buf, "nothing"))
            return;

        /* Liches and the like are immune to genocide until Vecna
           is destroyed */
        if (!u.uevent.uvecna && !strcmp(buf, "L")) {
            pline_The("voice of Vecna fills your mind:");
            verbalize("Thou shalt do no harm to my kind whilst I exist!");
            /* the dark magic causes the scroll to burn */
            pline("A dark magic catches the scroll on fire and you burn your %s.",
                  makeplural(body_part(HAND)));
            if (how_resistant(FIRE_RES) == 100) {
                shieldeff(u.ux, u.uy);
                monstseesu(M_SEEN_FIRE);
            } else {
                losehp(rnd(3), "burning scroll of genocide", KILLED_BY_AN);
            }
            return;
        }

        /* Vampires are immune to genocide until Vlad is destroyed */
        if (!u.uevent.uvlad && !strcmp(buf, "V")) {
            pline_The("voice of Vlad the Impaler fills your mind:");
            verbalize("Thou shalt do no harm to my kind whilst I exist!");
            /* the dark magic causes the scroll to burn */
            pline("A dark magic catches the scroll on fire and you burn your %s.",
                  makeplural(body_part(HAND)));
            if (how_resistant(FIRE_RES) == 100) {
                shieldeff(u.ux, u.uy);
                monstseesu(M_SEEN_FIRE);
            } else {
                losehp(rnd(3), "burning scroll of genocide", KILLED_BY_AN);
            }
            return;
        }

        class = name_to_monclass(buf, (int *) 0);
        if (class == 0 && (i = name_to_mon(buf, (int *) 0)) != NON_PM)
            class = mons[i].mlet;
        immunecnt = gonecnt = goodcnt = 0;
        for (i = LOW_PM; i < NUMMONS; i++) {
            if (mons[i].mlet == class) {
                if (!(mons[i].geno & G_GENO))
                    immunecnt++;
                else if (mvitals[i].mvflags & G_GENOD)
                    gonecnt++;
                else
                    goodcnt++;
            }
        }
        if (!goodcnt && class != mons[urole.malenum].mlet
            && class != mons[urace.malenum].mlet) {
            if (gonecnt)
                pline("All such monsters are already nonexistent.");
            else if (immunecnt || class == S_invisible)
                You("aren't permitted to genocide such monsters.");
            else if (wizard && buf[0] == '*') {
                register struct monst *mtmp, *mtmp2;

                gonecnt = 0;
                for (mtmp = fmon; mtmp; mtmp = mtmp2) {
                    mtmp2 = mtmp->nmon;
                    if (DEADMONSTER(mtmp))
                        continue;
                    mongone(mtmp);
                    gonecnt++;
                }
                pline("Eliminated %d monster%s.", gonecnt, plur(gonecnt));
                return;
            } else
                pline("That %s does not represent any monster.",
                      strlen(buf) == 1 ? "symbol" : "response");
            continue;
        }

        killed = candidates = 0;
        for (i = LOW_PM; i < NUMMONS; i++) {
            if (mons[i].mlet == class) {
                char nam[BUFSZ];

                Strcpy(nam, makeplural(mons[i].mname));
                /* Although "genus" is Latin for race, the hero benefits
                 * from both race and role; thus genocide affects either.
                 */
                candidates++;
                if (!u.uevent.uvecna && i == PM_ALHOON) {
                    if (!gameover)
                        pline("A dark magic prevents you from genociding alhoons.");
                    continue;
                } else if (!u.uevent.utalgath && i == PM_BEHOLDER) {
                    if (!gameover)
                        pline("A dark magic prevents you from genociding beholders.");
                    continue;
                } else if (Your_Own_Role(i) || Your_Own_Race(i)
                    || ((mons[i].geno & G_GENO)
                        && !(mvitals[i].mvflags & G_GENOD))) {
                    /* This check must be first since player monsters might
                     * have G_GENOD or !G_GENO. We also have to keep track of
                     * whether there are only two or fewer critters left available for us
                     * to geno in the first place; we must get them all then.
                     * finally, we have to make sure the self-geno cases always happen.
                     */
                    if (!ll_done++) {
                        if (!num_genocides())
                            livelog_printf(LL_CONDUCT | LL_GENOCIDE,
                                           "performed %s first genocide (two random monsters from class %c)",
                                           uhis(), def_monsyms[class].sym);
                        else
                            livelog_printf(LL_GENOCIDE, "genocided two random monsters from class %c",
                                           def_monsyms[class].sym);
                    }
                    if ((killed < 2 && (!rn2(goodcnt) || (killed + candidates > goodcnt - 2)))
                        || Your_Own_Role(i) || Your_Own_Race(i)) {
                        killed++;
                        mvitals[i].mvflags |= (G_GENOD | G_NOCORPSE);
                        kill_genocided_monsters();
                        update_inventory();	/* eggs & tins */
                        pline("Wiped out all %s.", nam);
                        if (Upolyd && i == u.umonnum) {
                            u.mh = -1;
                            if (Unchanging) {
                                if (!feel_dead++)
                                    You("die.");
                                /* finish genociding this class of
                                    monsters before ultimately dying */
                                gameover = TRUE;
                            } else
                                rehumanize();
                        }
                        /* Self-genocide if it matches either your race
                           or role.  Assumption:  male and female forms
                           share same monster class. */
                        if (i == urole.malenum || i == urace.malenum) {
                            u.uhp = -1;
                            if (Upolyd) {
                                if (!feel_dead++)
                                    You_feel("dead inside.");
                            } else {
                                if (!feel_dead++)
                                    You("die.");
                                gameover = TRUE;
                            }
                        }
                    }
                } else if (mvitals[i].mvflags & G_GENOD) {
                    if (!gameover)
                        pline("All %s are already nonexistent.", nam);
                } else if (!gameover) {
                    /* suppress feedback about quest beings except
                       for those applicable to our own role */
                    if ((mons[i].msound != MS_LEADER
                         || quest_info(MS_LEADER) == i)
                        && (mons[i].msound != MS_NEMESIS
                            || quest_info(MS_NEMESIS) == i)
                        && (mons[i].msound != MS_GUARDIAN
                            || quest_info(MS_GUARDIAN) == i)
                        /* non-leader/nemesis/guardian role-specific monster
                           */
                        && (i != PM_NINJA /* nuisance */
                            || Role_if(PM_SAMURAI))
                        && ((i != PM_TEMPLAR && i != PM_CHAMPION
                            && i != PM_AGENT) || Role_if(PM_INFIDEL))) {
                        boolean named, uniq;

                        named = type_is_pname(&mons[i]) ? TRUE : FALSE;
                        uniq = (mons[i].geno & G_UNIQ) ? TRUE : FALSE;
                        /* one special case */
                        if (i == PM_HIGH_PRIEST)
                            uniq = FALSE;

                        You("aren't permitted to genocide %s%s.",
                            (uniq && !named) ? "the " : "",
                            (uniq || named) ? mons[i].mname : nam);
                    }
                }
            }
        }
        if (gameover || u.uhp == -1) {
            killer.format = KILLED_BY_AN;
            Strcpy(killer.name, "scroll of genocide");
            if (gameover) {
                done(GENOCIDED);
            }
        }
        return;
    }
}

#define REALLY 1
#define PLAYER 2
#define ONTHRONE 4
void
do_genocide(how)
int how;
/* 0 = no genocide; create monsters (cursed scroll) */
/* 1 = normal genocide */
/* 3 = forced genocide of player */
/* 5 (4 | 1) = normal genocide from throne */
{
    char buf[BUFSZ] = DUMMY;
    register int i, killplayer = 0;
    register int mndx;
    register struct permonst *ptr;
    const char *which;

    if (how & PLAYER) {
        mndx = u.umonster; /* non-polymorphed mon num */
        ptr = &mons[mndx];
        Strcpy(buf, ptr->mname);
        killplayer++;
    } else {
        for (i = 0;; i++) {
            if (i >= 5) {
                /* cursed effect => no free pass (unless rndmonst() fails) */
                if (!(how & REALLY) && (ptr = rndmonst()) != 0)
                    break;

                pline1(thats_enough_tries);
                return;
            }
            getlin("What monster do you want to genocide? [type the name]",
                   buf);
            (void) mungspaces(buf);
            /* choosing "none" preserves genocideless conduct */
            if (*buf == '\033' || !strcmpi(buf, "none")
                || !strcmpi(buf, "nothing")) {
                /* ... but no free pass if cursed */
                if (!(how & REALLY) && (ptr = rndmonst()) != 0)
                    break; /* remaining checks don't apply */

                return;
            }

            mndx = name_to_mon(buf, (int *) 0);
            ptr = &mons[mndx];

            /* Liches and the like are immune to genocide until Vecna
               is destroyed */
            if (!u.uevent.uvecna
                && no_geno_vecna(ptr)) {
                pline_The("voice of Vecna fills your mind:");
                verbalize("Thou shalt do no harm to %s whilst I exist!",
                          makeplural(buf));
                if (how & ONTHRONE) { /* dark magic causes the throne to burn you */
                    pline_The("throne glows white hot!");
                    if (how_resistant(FIRE_RES) == 100) {
                        shieldeff(u.ux, u.uy);
                        monstseesu(M_SEEN_FIRE);
                    } else {
                        losehp(rnd(3), "sitting on a searing hot throne", KILLED_BY);
                    }
                } else { /* the dark magic causes the scroll to burn */
                    pline("A dark magic catches the scroll on fire and you burn your %s.",
                          makeplural(body_part(HAND)));
                    if (how_resistant(FIRE_RES) == 100) {
                        shieldeff(u.ux, u.uy);
                        monstseesu(M_SEEN_FIRE);
                    } else {
                        losehp(rnd(3), "burning scroll of genocide", KILLED_BY_AN);
                    }
                }
                return;
            }

            /* Vampires are immune to genocide until Vlad is destroyed */
            if (!u.uevent.uvlad
                && no_geno_vlad(ptr)) {
                pline_The("voice of Vlad the Impaler fills your mind:");
                verbalize("Thou shalt do no harm to %s whilst I exist!",
                          makeplural(buf));
                if (how & ONTHRONE) { /* dark magic causes the throne to burn you */
                    pline_The("throne glows white hot!");
                    if (how_resistant(FIRE_RES) == 100) {
                        shieldeff(u.ux, u.uy);
                        monstseesu(M_SEEN_FIRE);
                    } else {
                        losehp(rnd(3), "sitting on a searing hot throne", KILLED_BY);
                    }
                } else { /* the dark magic causes the scroll to burn */
                    pline("A dark magic catches the scroll on fire and you burn your %s.",
                          makeplural(body_part(HAND)));
                    if (how_resistant(FIRE_RES) == 100) {
                        shieldeff(u.ux, u.uy);
                        monstseesu(M_SEEN_FIRE);
                    } else {
                        losehp(rnd(3), "burning scroll of genocide", KILLED_BY_AN);
                    }
                }
                return;
            }

            /* Beholders are immune to genocide until Tal'Gath is killed */
            if (!u.uevent.utalgath
                && no_geno_talgath(ptr)) {
                pline_The("voice of Tal'Gath fills your mind:");
                verbalize("Thou shalt do no harm to %s whilst I exist!",
                          makeplural(buf));
                if (how & ONTHRONE) { /* dark magic causes the throne to burn you */
                    pline_The("throne glows white hot!");
                    if (how_resistant(FIRE_RES) == 100) {
                        shieldeff(u.ux, u.uy);
                        monstseesu(M_SEEN_FIRE);
                    } else {
                        losehp(rnd(3), "sitting on a searing hot throne", KILLED_BY);
                    }
                } else { /* the dark magic causes the scroll to burn */
                    pline("A dark magic catches the scroll on fire and you burn your %s.",
                          makeplural(body_part(HAND)));
                    if (how_resistant(FIRE_RES) == 100) {
                        shieldeff(u.ux, u.uy);
                        monstseesu(M_SEEN_FIRE);
                    } else {
                        losehp(rnd(3), "burning scroll of genocide", KILLED_BY_AN);
                    }
                }
                return;
            }

            if (mndx == NON_PM || (mvitals[mndx].mvflags & G_GENOD)) {
                pline("Such creatures %s exist in this world.",
                      (mndx == NON_PM) ? "do not" : "no longer");
                continue;
            }
            /* Although "genus" is Latin for race, the hero benefits
             * from both race and role; thus genocide affects either.
             */
            if (Your_Own_Role(mndx) || Your_Own_Race(mndx)) {
                killplayer++;
                break;
            }
            if (is_human(ptr) && u.ualign.type != A_NONE) {
                if (u.ualign.type == A_LAWFUL)
                    You_feel("guilty.");
                adjalign(-sgn(u.ualign.type));
            }
            if (is_demon(ptr))
                adjalign(sgn(u.ualign.type));

            if (!(ptr->geno & G_GENO)) {
                if (!Deaf) {
                    /* FIXME: unconditional "caverns" will be silly in some
                     * circumstances.  Who's speaking?  Divine pronouncements
                     * aren't supposed to be hampered by deafness....
                     */
                    if (flags.verbose)
                        pline("A thunderous voice booms through the caverns:");
                    verbalize("No, mortal!  That will not be done.");
                }
                continue;
            }
            /* KMH -- Unchanging prevents rehumanization */
            if (Unchanging && ptr == youmonst.data)
                killplayer++;
            break;
        }
        mndx = monsndx(ptr); /* needed for the 'no free pass' cases */
    }

    which = "all ";
    if (Hallucination) {
        if (Upolyd)
            Strcpy(buf, youmonst.data->mname);
        else {
            Strcpy(buf, (flags.female && urole.name.f) ? urole.name.f
                                                       : urole.name.m);
            buf[0] = lowc(buf[0]);
        }
    } else {
        Strcpy(buf, ptr->mname); /* make sure we have standard singular */
        if ((ptr->geno & G_UNIQ) && ptr != &mons[PM_HIGH_PRIEST])
            which = !type_is_pname(ptr) ? "the " : "";
    }
    if (how & REALLY) {
        if (!num_genocides())
            livelog_printf(LL_CONDUCT|LL_GENOCIDE,
                    "performed %s first genocide (%s)", uhis(), makeplural(buf));
        else
            livelog_printf(LL_GENOCIDE, "genocided %s", makeplural(buf));

        /* setting no-corpse affects wishing and random tin generation */
        mvitals[mndx].mvflags |= (G_GENOD | G_NOCORPSE);
        pline("Wiped out %s%s.", which,
              (*which != 'a') ? buf : makeplural(buf));

        if (killplayer) {
            /* might need to wipe out dual role */
            if (urole.femalenum != NON_PM && mndx == urole.malenum)
                mvitals[urole.femalenum].mvflags |= (G_GENOD | G_NOCORPSE);
            if (urole.femalenum != NON_PM && mndx == urole.femalenum)
                mvitals[urole.malenum].mvflags |= (G_GENOD | G_NOCORPSE);
            if (urace.femalenum != NON_PM && mndx == urace.malenum)
                mvitals[urace.femalenum].mvflags |= (G_GENOD | G_NOCORPSE);
            if (urace.femalenum != NON_PM && mndx == urace.femalenum)
                mvitals[urace.malenum].mvflags |= (G_GENOD | G_NOCORPSE);

            u.uhp = -1;
            if (how & PLAYER) {
                killer.format = KILLED_BY;
                Strcpy(killer.name, "genocidal confusion");
            } else if (how & ONTHRONE) {
                /* player selected while on a throne */
                killer.format = KILLED_BY_AN;
                Strcpy(killer.name, "imperious order");
            } else { /* selected player deliberately, not confused */
                killer.format = KILLED_BY_AN;
                Strcpy(killer.name, "scroll of genocide");
            }

            /* Polymorphed characters will die as soon as they're rehumanized.
             */
            /* KMH -- Unchanging prevents rehumanization */
            if (Upolyd && ptr != youmonst.data) {
                delayed_killer(POLYMORPH, killer.format, killer.name);
                You_feel("%s inside.", udeadinside());
            } else
                done(GENOCIDED);
        } else if (ptr == youmonst.data) {
            rehumanize();
        }
        kill_genocided_monsters();
        update_inventory(); /* in case identified eggs were affected */
    } else {
        int cnt = 0, census = monster_census(FALSE);

        if (!(mons[mndx].geno & G_UNIQ)
            && !(mvitals[mndx].mvflags & (G_GENOD | G_EXTINCT)))
            for (i = rn1(3, 4); i > 0; i--) {
                if (!makemon(ptr, u.ux, u.uy, NO_MINVENT))
                    break; /* couldn't make one */
                ++cnt;
                if (mvitals[mndx].mvflags & G_EXTINCT)
                    break; /* just made last one */
            }
        if (cnt) {
            /* accumulated 'cnt' doesn't take groups into account;
               assume bringing in new mon(s) didn't remove any old ones */
            cnt = monster_census(FALSE) - census;
            pline("Sent in %s%s.", (cnt > 1) ? "some " : "",
                  (cnt > 1) ? makeplural(buf) : an(buf));
        } else
            pline1(nothing_happens);
    }
}

void
punish(sobj)
struct obj *sobj;
{
    struct obj *reuse_ball = (sobj && sobj->otyp == HEAVY_IRON_BALL)
                                ? sobj : (struct obj *) 0;

    /* KMH -- Punishment is still okay when you are riding */
    if (!reuse_ball)
        You("are being punished for your misbehavior!");
    if (Punished) {
        Your("iron ball gets heavier.");
        uball->owt += IRON_BALL_W_INCR * (1 + sobj->cursed);
        return;
    }
    if (amorphous(youmonst.data) || is_whirly(youmonst.data)
        || unsolid(youmonst.data)) {
        if (!reuse_ball) {
            pline("A ball and chain appears, then falls away.");
            dropy(mkobj(BALL_CLASS, TRUE));
        } else {
            dropy(reuse_ball);
        }
        return;
    }
    setworn(mkobj(CHAIN_CLASS, TRUE), W_CHAIN);
    if (!reuse_ball)
        setworn(mkobj(BALL_CLASS, TRUE), W_BALL);
    else
        setworn(reuse_ball, W_BALL);
    uball->spe = 1; /* special ball (see save) */

    /*
     *  Place ball & chain if not swallowed.  If swallowed, the ball & chain
     *  variables will be set at the next call to placebc().
     */
    if (!u.uswallow) {
        placebc();
        if (Blind)
            set_bc(1);      /* set up ball and chain variables */
        newsym(u.ux, u.uy); /* see ball&chain if can't see self */
    }
}

/* remove the ball and chain */
void
unpunish()
{
    struct obj *savechain = uchain,
               *saveball = uball;

    /* chain goes away */
    obj_extract_self(uchain);
    maybe_unhide_at(uchain->ox, uchain->oy);
    newsym(uchain->ox, uchain->oy);
    setworn((struct obj *) 0, W_CHAIN); /* sets 'uchain' to Null */
    dealloc_obj(savechain);
    /* the chain is gone but the no longer attached ball persists */
    uball->spe = 0;
    setworn((struct obj *) 0, W_BALL); /* sets 'uball' to Null */
    if (saveball->where == OBJ_FLOOR
        && is_open_air(saveball->ox, saveball->oy)) {
        /* pick up the ball and drop it, so it can fall through the air */
        obj_extract_self(saveball);
        if (!flooreffects(saveball, saveball->ox, saveball->oy, "drop")) {
            place_object(saveball, saveball->ox, saveball->oy);
        } else {
            maybe_unhide_at(saveball->ox, saveball->oy);
            newsym(saveball->ox, saveball->oy);
        }
    }
}

/* some creatures have special data structures that only make sense in their
 * normal locations -- if the player tries to create one elsewhere, or to
 * revive one, the disoriented creature becomes a zombie
 */
boolean
cant_revive(mtype, revival, from_obj)
int *mtype;
boolean revival;
struct obj *from_obj;
{
    /* SHOPKEEPERS can be revived now */
    if (*mtype == PM_GUARD || (*mtype == PM_SHOPKEEPER && !revival)
        || *mtype == PM_HIGH_PRIEST || *mtype == PM_ALIGNED_PRIEST
        || *mtype == PM_ANGEL) {
        int typ = NON_PM;
        if (from_obj && has_omonst(from_obj) && has_erac(OMONST(from_obj)))
            typ = zombie_form(r_data(OMONST(from_obj)));
        *mtype = (typ >= LOW_PM) ? typ : PM_HUMAN_ZOMBIE;
        return TRUE;
    } else if (*mtype == PM_LONG_WORM_TAIL) { /* for create_particular() */
        *mtype = PM_LONG_WORM;
        return TRUE;
    } else if (unique_corpstat(&mons[*mtype])
               && (!from_obj || !has_omonst(from_obj))) {
        /* unique corpses (from bones or wizard mode wish) or
           statues (bones or any wish) end up as shapechangers */
        *mtype = PM_DOPPELGANGER;
        return TRUE;
    }
    return FALSE;
}

struct _create_particular_data {
    int which;
    int fem;
    int race;
    char monclass;
    boolean randmonst;
    boolean maketame, makepeaceful, makehostile;
    boolean sleeping, saddled, invisible, hidden, barded;
    boolean sick, diseased;
};

boolean
create_particular_parse(str, d)
char *str;
struct _create_particular_data *d;
{
    char *bufp = str;
    char *tmpp;
    int i;

    d->monclass = MAXMCLASSES;
    d->which = urole.malenum; /* an arbitrary index into mons[] */
    d->fem = -1; /* gender not specified */
    d->race = NON_PM; /* no race by default */
    d->randmonst = FALSE;
    d->maketame = d->makepeaceful = d->makehostile = FALSE;
    d->sleeping = d->saddled = d->invisible = d->hidden = d->barded = FALSE;
    d->sick = d->diseased = FALSE;

    if ((tmpp = strstri(bufp, "saddled ")) != 0) {
        d->saddled = TRUE;
        (void) memset(tmpp, ' ', sizeof "saddled " - 1);
    }
    if ((tmpp = strstri(bufp, "barded ")) != 0) {
        d->barded = TRUE;
        (void) memset(tmpp, ' ', sizeof "barded " - 1);
    }
    if ((tmpp = strstri(bufp, "sleeping ")) != 0) {
        d->sleeping = TRUE;
        (void) memset(tmpp, ' ', sizeof "sleeping " - 1);
    }
    if ((tmpp = strstri(bufp, "invisible ")) != 0) {
        d->invisible = TRUE;
        (void) memset(tmpp, ' ', sizeof "invisible " - 1);
    }
    if ((tmpp = strstri(bufp, "hidden ")) != 0) {
        d->hidden = TRUE;
        (void) memset(tmpp, ' ', sizeof "hidden " - 1);
    }
    /* check "female" before "male" to avoid false hit mid-word */
    if ((tmpp = strstri(bufp, "female ")) != 0) {
        d->fem = 1;
        (void) memset(tmpp, ' ', sizeof "female " - 1);
    }
    if ((tmpp = strstri(bufp, "male ")) != 0) {
        d->fem = 0;
        (void) memset(tmpp, ' ', sizeof "male " - 1);
    }
    if ((tmpp = strstri(bufp, "sick ")) != 0) {
        d->sick = TRUE;
        (void) memset(tmpp, ' ', sizeof "sick " - 1);
    }
    if ((tmpp = strstri(bufp, "diseased ")) != 0) {
        d->diseased = TRUE;
        (void) memset(tmpp, ' ', sizeof "diseased " - 1);
    }
    bufp = mungspaces(bufp); /* after potential memset(' ') */
    /* allow the initial disposition to be specified */
    if (!strncmpi(bufp, "tame ", 5)) {
        bufp += 5;
        d->maketame = TRUE;
    } else if (!strncmpi(bufp, "peaceful ", 9)) {
        bufp += 9;
        d->makepeaceful = TRUE;
    } else if (!strncmpi(bufp, "hostile ", 8)) {
        bufp += 8;
        d->makehostile = TRUE;
    }

    /* determine if a race was specified for the resulting mon */
    if (!strncmpi(bufp, "racial ", 7)) {
        bufp += 7;
        int adjlen;
        for (i = 0; races[i].adj; i++) {
            adjlen = strlen(races[i].adj);
            if (!strncmpi(bufp, races[i].adj, adjlen)
                && bufp[adjlen] == ' ') {
                bufp = bufp + adjlen + 1;
                d->race = races[i].malenum;
                break;
            }
        }
        /* if no "real" (hero-valid) race matches, try normal monster name */
        if (d->race == NON_PM) {
            int race = name_to_mon(bufp, &adjlen);
            if (race >= LOW_PM && bufp[adjlen] != '\0') {
                bufp += adjlen;
                d->race = race;
                if (*bufp != ' ')
                    bufp = index(bufp, ' ');
                ++bufp;
            }
        }
    }
    /* decide whether a valid monster was chosen */
    if (wizard && (!strcmp(bufp, "*") || !strcmp(bufp, "random"))) {
        d->randmonst = TRUE;
        return TRUE;
    }

    d->which = name_to_mon(bufp, (int *) 0);
    if (d->which >= LOW_PM)
        return TRUE; /* got one */

    d->monclass = name_to_monclass(bufp, &d->which);

    if (d->which >= LOW_PM) {
        d->monclass = MAXMCLASSES; /* matters below */
        return TRUE;
    } else if (d->monclass == S_invisible) { /* not an actual mon class */
        d->which = PM_STALKER;
        d->monclass = MAXMCLASSES;
        return TRUE;
    } else if (d->monclass == S_WORM_TAIL) { /* empty monster class */
        d->which = PM_LONG_WORM;
        d->monclass = MAXMCLASSES;
        return TRUE;
    } else if (d->monclass > 0) {
        d->which = urole.malenum; /* reset from NON_PM */
        return TRUE;
    }
    return FALSE;
}

boolean
create_particular_creation(d)
struct _create_particular_data *d;
{
    struct permonst *whichpm = NULL;
    int i, mx, my, firstchoice = NON_PM;
    struct monst *mtmp;
    boolean madeany = FALSE;

    if (!d->randmonst) {
        firstchoice = d->which;
        if (cant_revive(&d->which, FALSE, (struct obj *) 0)
            && firstchoice != PM_LONG_WORM_TAIL) {
            /* wizard mode can override handling of special monsters */
            char buf[BUFSZ];

            Sprintf(buf, "Creating %s instead; force %s?",
                    mons[d->which].mname, mons[firstchoice].mname);
            if (yn(buf) == 'y')
                d->which = firstchoice;
        }
        whichpm = &mons[d->which];
    }
    for (i = 0; i <= multi; i++) {
        if (d->monclass != MAXMCLASSES)
            whichpm = mkclass(d->monclass, 0);
        else if (d->randmonst)
            whichpm = rndmonst();
        mtmp = makemon(whichpm, u.ux, u.uy, NO_MM_FLAGS);
        if (!mtmp) {
            /* quit trying if creation failed and is going to repeat */
            if (d->monclass == MAXMCLASSES && !d->randmonst)
                break;
            /* otherwise try again */
            continue;
        }
        mx = mtmp->mx, my = mtmp->my;
        /* 'is_FOO()' ought to be called 'always_FOO()' */
        if (d->fem != -1 && !is_male(mtmp->data) && !is_female(mtmp->data))
            mtmp->female = d->fem; /* ignored for is_neuter() */
        if (d->maketame) {
            (void) tamedog(mtmp, (struct obj *) 0);
        } else if (d->makepeaceful || d->makehostile) {
            mtmp->mtame = 0; /* sanity precaution */
            mtmp->mpeaceful = d->makepeaceful ? 1 : 0;
            set_malign(mtmp);
        }
        if (d->saddled && can_saddle(mtmp) && !which_armor(mtmp, W_SADDLE)) {
            struct obj *otmp = mksobj(SADDLE, TRUE, FALSE);

            put_saddle_on_mon(otmp, mtmp);
        }
        if (d->barded && can_wear_barding(mtmp) && !which_armor(mtmp, W_BARDING)) {
            struct obj *otmp = mksobj(rn2(4) ? BARDING
                                             : rn2(3) ? SPIKED_BARDING
                                                      : BARDING_OF_REFLECTION, TRUE, FALSE);

            put_barding_on_mon(otmp, mtmp);
        }
        if (d->invisible) {
            mon_set_minvis(mtmp);
            if (does_block(mx, my, &levl[mx][my]))
                block_point(mx, my);
            else
                unblock_point(mx, my);
        }
        if (d->sick
            && !(resists_sick(mtmp->data) || defended(mtmp, AD_DISE))) {
            mtmp->msick = 1;
            mtmp->msicktime = rn1(9, 6);
        }
        if (d->diseased
            && !(resists_sick(mtmp->data) || defended(mtmp, AD_DISE))) {
            mtmp->mdiseased = 1;
            mtmp->mdiseasetime = rn1(9, 6);
        }
        if (d->hidden
            && ((is_hider(mtmp->data) && mtmp->data->mlet != S_MIMIC)
                || (hides_under(mtmp->data) && OBJ_AT(mx, my))
                || (mtmp->data->mlet == S_EEL && is_pool(mx, my))))
            mtmp->mundetected = 1;
        if (d->sleeping)
            mtmp->msleeping = 1;
        if (d->race != NON_PM) {
            /*
             * Note that this does nothing but add the race to the monster --
             * since it's happening after monster creation, their alignment
             * (if it depends on race) might be wrong, mplayer names, etc).
             * FIXME? add some way to specify race in makemon, so that it will
             * allow "properly" specifying race, along with all that implies.
             */
            apply_race(mtmp, d->race);
            newsym(mtmp->mx, mtmp->my); /* in case of racial glyph */
        }
        /* iff asking for 'hidden', show locaton of every created monster
           that can't be seen--whether that's due to successfully hiding
           or vision issues (line-of-sight, invisibility, blindness) */
        if (d->hidden && !canspotmon(mtmp)) {
            int count = couldsee(mx, my) ? 8 : 4;
            char saveviz = viz_array[my][mx];

            if (!flags.sparkle)
                count /= 2;
            viz_array[my][mx] |= (IN_SIGHT | COULD_SEE);
            flash_glyph_at(mx, my, mon_to_glyph(mtmp, newsym_rn2), count);
            viz_array[my][mx] = saveviz;
            newsym(mx, my);
        }
        madeany = TRUE;
        /* in case we got a doppelganger instead of what was asked
           for, make it start out looking like what was asked for */
        if (mtmp->cham != NON_PM && firstchoice != NON_PM
            && mtmp->cham != firstchoice)
            (void) newcham(mtmp, &mons[firstchoice], FALSE, FALSE);
    }
    return madeany;
}

/*
 * Make a new monster with the type controlled by the user.
 *
 * Note:  when creating a monster by class letter, specifying the
 * "strange object" (']') symbol produces a random monster rather
 * than a mimic.  This behavior quirk is useful so don't "fix" it
 * (use 'm'--or "mimic"--to create a random mimic).
 *
 * Used in wizard mode only (for ^G command and for scroll or spell
 * of create monster).  Once upon a time, an earlier incarnation of
 * this code was also used for the scroll/spell in explore mode.
 */
boolean
create_particular()
{
    char buf[BUFSZ] = DUMMY, *bufp;
    int  tryct = 5;
    struct _create_particular_data d;

    do {
        getlin("Create what kind of monster? [type the name or symbol]", buf);
        bufp = mungspaces(buf);
        if (*bufp == '\033')
            return FALSE;

        if (create_particular_parse(bufp, &d))
            break;

        /* no good; try again... */
        pline("I've never heard of such monsters.");
    } while (--tryct > 0);

    if (!tryct)
        pline1(thats_enough_tries);
    else
        return create_particular_creation(&d);

    return FALSE;
}

/*read.c*/
