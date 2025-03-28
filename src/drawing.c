/* NetHack 3.6	drawing.c	$NHDT-Date: 1573943500 2019/11/16 22:31:40 $  $NHDT-Branch: NetHack-3.6 $:$NHDT-Revision: 1.64 $ */
/* Copyright (c) NetHack Development Team 1992.                   */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "tcap.h"

/* Relevant header information in rm.h, objclass.h, and monsym.h. */

#ifdef C
#undef C
#endif

#ifdef TEXTCOLOR
#define C(n) n
#else
#define C(n)
#endif

struct symsetentry symset[NUM_GRAPHICS];

int currentgraphics = 0;

nhsym showsyms[SYM_MAX] = DUMMY; /* symbols to be displayed */
nhsym primary_syms[SYM_MAX] = DUMMY;   /* primary symbols          */
nhsym rogue_syms[SYM_MAX] = DUMMY;   /* rogue symbols           */
nhsym ov_primary_syms[SYM_MAX] = DUMMY;   /* overides via config SYMBOL */
nhsym ov_rogue_syms[SYM_MAX] = DUMMY;   /* overides via config ROGUESYMBOL */
nhsym warnsyms[WARNCOUNT] = DUMMY; /* the current warning display symbols */
const char invisexplain[] = "remembered, unseen, creature",
           altinvisexplain[] = "unseen creature"; /* for clairvoyance */

/* Default object class symbols.  See objclass.h.
 * {symbol, name, explain}
 *     name:    used in object_detect().
 *     explain: used in do_look().
 */
const struct class_sym def_oc_syms[MAXOCLASSES] = {
    { '\0', "", "" }, /* placeholder for the "random class" */
    { ILLOBJ_SYM, "illegal objects", "strange object" },
    { WEAPON_SYM, "weapons", "weapon" },
    { ARMOR_SYM, "armor", "suit or piece of armor" },
    { RING_SYM, "rings", "ring" },
    { AMULET_SYM, "amulets", "amulet" },
    { TOOL_SYM, "tools", "useful item (pick-axe, key, lamp...)" },
    { FOOD_SYM, "food", "piece of food" },
    { POTION_SYM, "potions", "potion" },
    { SCROLL_SYM, "scrolls", "scroll" },
    { SPBOOK_SYM, "spellbooks", "spellbook" },
    { WAND_SYM, "wands", "wand" },
    { GOLD_SYM, "coins", "pile of coins" },
    { GEM_SYM, "rocks", "gem or rock" },
    { ROCK_SYM, "large stones", "boulder or statue" },
    { BALL_SYM, "iron balls", "iron ball" },
    { CHAIN_SYM, "chains", "iron chain" },
    { VENOM_SYM, "venoms", "splash of venom" }
};

/* Default monster class symbols.  See monsym.h. */
const struct class_sym def_monsyms[MAXMCLASSES] = {
    { '\0', "", "" },
    { DEF_ANT, "", "ant or other insect" },
    { DEF_BLOB, "", "blob" },
    { DEF_COCKATRICE, "", "cockatrice" },
    { DEF_DOG, "", "dog or other canine" },
    { DEF_EYE, "", "eye or sphere" },
    { DEF_FELINE, "", "cat or other feline" },
    { DEF_GREMLIN, "", "gremlin" },
    { DEF_HUMANOID, "", "humanoid" },
    { DEF_IMP, "", "imp or minor demon" },
    { DEF_JELLY, "", "jelly" },
    { DEF_KOBOLD, "", "kobold" },
    { DEF_LEPRECHAUN, "", "leprechaun" },
    { DEF_MIMIC, "", "mimic" },
    { DEF_NYMPH, "", "nymph" },
    { DEF_ORC, "", "orc" },
    { DEF_PIERCER, "", "piercer" },
    { DEF_QUADRUPED, "", "quadruped" },
    { DEF_RODENT, "", "rodent" },
    { DEF_SPIDER, "", "arachnid or centipede" },
    { DEF_TRAPPER, "", "trapper or lurker above" },
    { DEF_UNICORN, "", "unicorn or horse" },
    { DEF_VORTEX, "", "vortex" },
    { DEF_WORM, "", "worm" },
    { DEF_XAN, "", "xan or other mythical/fantastic insect" },
    { DEF_LIGHT, "", "light" },
    { DEF_SKELETON, "", "skeleton" },
    { DEF_ANGEL, "", "angelic being" },
    { DEF_BAT, "", "bat or bird" },
    { DEF_CENTAUR, "", "centaur" },
    { DEF_DRAGON, "", "dragon" },
    { DEF_ELEMENTAL, "", "elemental" },
    { DEF_FUNGUS, "", "fungus or mold" },
    { DEF_GNOME, "", "gnome" },
    { DEF_GIANT, "", "giant humanoid" },
    { '\0', "", "invisible monster" },
    { DEF_JABBERWOCK, "", "jabberwock" },
    { DEF_KOP, "", "Keystone Kop" },
    { DEF_LICH, "", "lich" },
    { DEF_MUMMY, "", "mummy" },
    { DEF_NAGA, "", "naga" },
    { DEF_OGRE, "", "ogre" },
    { DEF_PUDDING, "", "pudding or ooze" },
    { DEF_QUANTMECH, "", "quantum mechanic" },
    { DEF_RUSTMONST, "", "rust monster or disenchanter" },
    { DEF_SNAKE, "", "snake" },
    { DEF_TROLL, "", "troll" },
    { DEF_UMBER, "", "umber hulk or shambling horror" },
    { DEF_VAMPIRE, "", "vampire" },
    { DEF_WRAITH, "", "wraith" },
    { DEF_XORN, "", "xorn" },
    { DEF_YETI, "", "apelike creature" },
    { DEF_ZOMBIE, "", "zombie" },
    { DEF_HUMAN, "", "human or other humanoid" },
    { DEF_GHOST, "", "ghost" },
    { DEF_GOLEM, "", "golem" },
    { DEF_DEMON, "", "major demon" },
    { DEF_EEL, "", "sea monster" },
    { DEF_LIZARD, "", "lizard" },
    { DEF_ENT, "", "tree-like creature" },
    { DEF_PLANT, "", "plant-like creature" },
    { DEF_WORM_TAIL, "", "long worm tail" },
    { DEF_MIMIC_DEF, "", "mimic" },
};

const struct symdef def_warnsyms[WARNCOUNT] = {
    /* white warning  */
    { '0', "unknown creature causing you worry",    C(CLR_WHITE) },
    /* pink warning   */
    { '1', "unknown creature causing you concern",  C(CLR_RED) },
    /* red warning    */
    { '2', "unknown creature causing you anxiety",  C(CLR_RED) },
    /* ruby warning   */
    { '3', "unknown creature causing you disquiet", C(CLR_RED) },
    /* purple warning */
    { '4', "unknown creature causing you alarm",    C(CLR_MAGENTA) },
    /* black warning  */
    { '5', "unknown creature causing you dread",    C(CLR_BRIGHT_MAGENTA) },
};

/*
 *  Default screen symbols with explanations and colors.
 */
const struct symdef defsyms[MAXPCHARS] = {
/* 0*/ { ' ', "dark part of a room", C(NO_COLOR) },    /* stone */
       { '|', "wall", C(CLR_GRAY) },                   /* vwall */
       { '-', "wall", C(CLR_GRAY) },                   /* hwall */
       { '-', "wall", C(CLR_GRAY) },                   /* tlcorn */
       { '-', "wall", C(CLR_GRAY) },                   /* trcorn */
       { '-', "wall", C(CLR_GRAY) },                   /* blcorn */
       { '-', "wall", C(CLR_GRAY) },                   /* brcorn */
       { '-', "wall", C(CLR_GRAY) },                   /* crwall */
       { '-', "wall", C(CLR_GRAY) },                   /* tuwall */
       { '-', "wall", C(CLR_GRAY) },                   /* tdwall */
/*10*/ { '|', "wall", C(CLR_GRAY) },                   /* tlwall */
       { '|', "wall", C(CLR_GRAY) },                   /* trwall */
       { '.', "doorway", C(CLR_GRAY) },                /* ndoor */
       { '-', "open door", C(CLR_BROWN) },             /* vodoor */
       { '|', "open door", C(CLR_BROWN) },             /* hodoor */
       { '+', "closed door", C(CLR_BROWN) },           /* vcdoor */
       { '+', "closed door", C(CLR_BROWN) },           /* hcdoor */
       { '#', "iron bars", C(HI_METAL) },              /* bars */
       { '#', "tree", C(CLR_GREEN) },                  /* tree */
       { '#', "dead tree", C(CLR_BLACK) },             /* dead tree */
/*20*/ { '.', "floor of a room", C(CLR_GRAY) },        /* room */
       { '.', "dark part of a room", C(CLR_BLACK) },   /* dark room */
       { '#', "corridor", C(CLR_BLACK) },              /* dark corr */
       { '#', "lit corridor", C(CLR_GRAY) },   /* lit corr (see mapglyph.c) */
       { '<', "staircase up", C(CLR_GRAY) },           /* upstair */
       { '>', "staircase down", C(CLR_GRAY) },         /* dnstair */
       { '<', "ladder up", C(CLR_BROWN) },             /* upladder */
       { '>', "ladder down", C(CLR_BROWN) },           /* dnladder */
       { '_', "altar", C(CLR_GRAY) },                  /* altar */
       { '|', "grave", C(CLR_WHITE) },                 /* grave */
/*30*/ { '\\', "opulent throne", C(HI_GOLD) },         /* throne */
       { '{', "sink", C(CLR_WHITE) },                  /* sink */
       { '{', "forge", C(CLR_ORANGE) },                /* forge */
       { '(', "magic chest", C(CLR_BRIGHT_MAGENTA) },  /* magic chest */
       { '{', "fountain", C(CLR_BRIGHT_BLUE) },        /* fountain */
       { '}', "water", C(CLR_BLUE) },                  /* pool */
       { '.', "ice", C(CLR_CYAN) },                    /* ice */
       { '.', "grass", C(CLR_BRIGHT_GREEN) },          /* grass */
       { '.', "sand", C(CLR_YELLOW) },                 /* sand */
       { '}', "molten lava", C(CLR_RED) },             /* lava */
/*40*/ { '.', "lowered drawbridge", C(CLR_BROWN) },    /* vodbridge */
       { '.', "lowered drawbridge", C(CLR_BROWN) },    /* hodbridge */
       { '#', "raised drawbridge", C(CLR_BROWN) },     /* vcdbridge */
       { '#', "raised drawbridge", C(CLR_BROWN) },     /* hcdbridge */
       { ' ', "air", C(CLR_CYAN) },                    /* open air */
       { '#', "cloud", C(CLR_GRAY) },                  /* [part of] a cloud */
       { '}', "shallow water", C(CLR_CYAN) },          /* puddle */
       { '}', "sewage", C(CLR_GREEN) },                /* sewage */
       { '}', "water", C(CLR_BLUE) },                  /* under water */
       { '^', "arrow trap", C(HI_METAL) },             /* trap */
/*50*/ { '^', "bolt trap", C(HI_METAL) },              /* trap */
       { '^', "dart trap", C(HI_METAL) },              /* trap */
       { '^', "falling rock trap", C(CLR_GRAY) },      /* trap */
       { '^', "squeaky board", C(CLR_BROWN) },         /* trap */
       { '^', "bear trap", C(HI_METAL) },              /* trap */
       { '^', "land mine", C(CLR_RED) },               /* trap */
       { '^', "rolling boulder trap", C(CLR_GRAY) },   /* trap */
       { '^', "sleeping gas trap", C(HI_ZAP) },        /* trap */
       { '^', "rust trap", C(CLR_BLUE) },              /* trap */
       { '^', "fire trap", C(CLR_ORANGE) },            /* trap */
/*60*/ { '^', "ice trap", C(CLR_BRIGHT_CYAN) },        /* trap */
       { '^', "pit", C(CLR_BLACK) },                   /* trap */
       { '^', "spiked pit", C(CLR_BLACK) },            /* trap */
       { '^', "hole", C(CLR_BROWN) },                  /* trap */
       { '^', "trap door", C(CLR_BROWN) },             /* trap */
       { '^', "teleportation trap", C(CLR_MAGENTA) },  /* trap */
       { '^', "level teleporter", C(CLR_MAGENTA) },    /* trap */
       { '^', "magic portal", C(CLR_BRIGHT_MAGENTA) }, /* trap */
       { '"', "web", C(CLR_GRAY) },                    /* web */
       { '^', "statue trap", C(CLR_GRAY) },            /* trap */
/*70*/ { '^', "magic trap", C(HI_ZAP) },               /* trap */
       { '^', "anti-magic field", C(HI_ZAP) },         /* trap */
       { '^', "polymorph trap", C(CLR_BRIGHT_GREEN) }, /* trap */
       { '^', "spear trap", C(CLR_BROWN) },            /* trap */
       { '^', "magic beam trap", C(CLR_YELLOW) },      /* trap */
       { '~', "vibrating square", C(CLR_MAGENTA) },    /* "trap" */
       /* zap colors are changed by mapglyph() to match type of beam */
       { '|', "", C(CLR_GRAY) },                /* vbeam */
       { '-', "", C(CLR_GRAY) },                /* hbeam */
       { '\\', "", C(CLR_GRAY) },               /* lslant */
       { '/', "", C(CLR_GRAY) },                /* rslant */
/*80*/ { '*', "", C(CLR_WHITE) },               /* dig beam */
       { '!', "", C(CLR_WHITE) },               /* camera flash beam */
       { ')', "", C(HI_WOOD) },                 /* boomerang open left */
       { '(', "", C(HI_WOOD) },                 /* boomerang open right */
       { '0', "", C(HI_ZAP) },                  /* 4 magic shield symbols */
       { '#', "", C(HI_ZAP) },
       { '@', "", C(HI_ZAP) },
       { '*', "", C(HI_ZAP) },
       { '#', "poison cloud", C(CLR_BRIGHT_GREEN) },   /* part of a cloud */
       { '?', "valid position", C(CLR_BRIGHT_GREEN) }, /*  target position */
       /* swallow colors are changed by mapglyph() to match engulfing monst */
/*90*/ { '/', "", C(CLR_GREEN) },         /* swallow top left      */
       { '-', "", C(CLR_GREEN) },         /* swallow top center    */
       { '\\', "", C(CLR_GREEN) },        /* swallow top right     */
       { '|', "", C(CLR_GREEN) },         /* swallow middle left   */
       { '|', "", C(CLR_GREEN) },         /* swallow middle right  */
       { '\\', "", C(CLR_GREEN) },        /* swallow bottom left   */
       { '-', "", C(CLR_GREEN) },         /* swallow bottom center */
       { '/', "", C(CLR_GREEN) },         /* swallow bottom right  */
       /* explosion colors are changed by mapglyph() to match type of expl. */
       { '/', "", C(CLR_ORANGE) },        /* explosion top left     */
       { '-', "", C(CLR_ORANGE) },        /* explosion top center   */
/*100*/{ '\\', "", C(CLR_ORANGE) },       /* explosion top right    */
       { '|', "", C(CLR_ORANGE) },        /* explosion middle left  */
       { ' ', "", C(CLR_ORANGE) },        /* explosion middle center*/
       { '|', "", C(CLR_ORANGE) },        /* explosion middle right */
       { '\\', "", C(CLR_ORANGE) },       /* explosion bottom left  */
       { '-', "", C(CLR_ORANGE) },        /* explosion bottom center*/
       { '/', "", C(CLR_ORANGE) },        /* explosion bottom right */
};

/* default rogue level symbols */
static const uchar def_r_oc_syms[MAXOCLASSES] = {
/* 0*/ '\0', ILLOBJ_SYM, WEAPON_SYM, ']', /* armor */
       RING_SYM,
/* 5*/ ',',                     /* amulet */
       TOOL_SYM, ':',           /* food */
       POTION_SYM, SCROLL_SYM,
/*10*/ SPBOOK_SYM, WAND_SYM,
       GEM_SYM,                /* gold -- yes it's the same as gems */
       GEM_SYM, ROCK_SYM,
/*15*/ BALL_SYM, CHAIN_SYM, VENOM_SYM
};

#undef C

#if defined(TERMLIB) || defined(CURSES_GRAPHICS)
void NDECL((*decgraphics_mode_callback)) = 0; /* set in tty_start_screen() */
#endif /* TERMLIB || CURSES */

#ifdef PC9800
void NDECL((*ibmgraphics_mode_callback)) = 0; /* set in tty_start_screen() */
void NDECL((*ascgraphics_mode_callback)) = 0; /* set in tty_start_screen() */
#endif

#ifdef CURSES_GRAPHICS
void NDECL((*cursesgraphics_mode_callback)) = 0;
#endif

/*
 * Convert the given character to an object class.  If the character is not
 * recognized, then MAXOCLASSES is returned.  Used in detect.c, invent.c,
 * objnam.c, options.c, pickup.c, sp_lev.c, lev_main.c, and tilemap.c.
 */
int
def_char_to_objclass(ch)
char ch;
{
    int i;
    for (i = 1; i < MAXOCLASSES; i++)
        if (ch == def_oc_syms[i].sym)
            break;
    return i;
}

/*
 * Convert a character into a monster class.  This returns the _first_
 * match made.  If there are are no matches, return MAXMCLASSES.
 * Used in detect.c, options.c, read.c, sp_lev.c, and lev_main.c
 */
int
def_char_to_monclass(ch)
char ch;
{
    int i;
    for (i = 1; i < MAXMCLASSES; i++)
        if (ch == def_monsyms[i].sym)
            break;
    return i;
}

/*
 * Explanations of the functions found below:
 *
 * init_symbols()
 *                     Sets the current display symbols, the
 *                     loadable symbols to the default NetHack
 *                     symbols, including the rogue_syms rogue level
 *                     symbols. This would typically be done
 *                     immediately after execution begins. Any
 *                     previously loaded external symbol sets are
 *                     discarded.
 *
 * switch_symbols(arg)
 *                     Called to swap in new current display symbols
 *                     (showsyms) from either the default symbols,
 *                     or from the loaded symbols.
 *
 *                     If (arg == 0) then showsyms are taken from
 *                     defsyms, def_oc_syms, and def_monsyms.
 *
 *                     If (arg != 0), which is the normal expected
 *                     usage, then showsyms are taken from the
 *                     adjustable display symbols found in primary_syms.
 *                     primary_syms may have been loaded from an external
 *                     symbol file by config file options or interactively
 *                     in the Options menu.
 *
 * assign_graphics(arg)
 *
 *                     This is used in the game core to toggle in and
 *                     out of other {rogue} level display modes.
 *
 *                     If arg is ROGUESET, this places the rogue level
 *                     symbols from rogue_syms into showsyms.
 *
 *                     If arg is PRIMARY, this places the symbols
 *                     from l_monsyms into showsyms.
 *
 * update_primary_symset()
 *                     Update a member of the primary(primary_*) symbol set.
 *
 * update_rogue_symset()
 *                     Update a member of the rogue (rogue_*) symbol set.
 *
 * update_ov_primary_symset()
 *                     Update a member of the overrides for primary symbol set.
 *
 * update_ov_rogue_symset()
 *                     Update a member of the overrides for rogue symbol set.
 *
 */

void
init_symbols()
{
    init_ov_primary_symbols();
    init_ov_rogue_symbols();
    init_primary_symbols();
    init_showsyms();
    init_rogue_symbols();
}

void
init_showsyms()
{
    register int i;

    for (i = 0; i < MAXPCHARS; i++)
        showsyms[i + SYM_OFF_P] = defsyms[i].sym;
    for (i = 0; i < MAXOCLASSES; i++)
        showsyms[i + SYM_OFF_O] = def_oc_syms[i].sym;
    for (i = 0; i < MAXMCLASSES; i++)
        showsyms[i + SYM_OFF_M] = def_monsyms[i].sym;
    for (i = 0; i < WARNCOUNT; i++)
        showsyms[i + SYM_OFF_W] = def_warnsyms[i].sym;
    for (i = 0; i < MAXOTHER; i++)
        showsyms[i + SYM_OFF_X] = get_othersym(i, PRIMARY);
}

/* initialize defaults for the overrides to the rogue symset */
void
init_ov_rogue_symbols()
{
    register int i;

    for (i = 0; i < SYM_MAX; i++)
        ov_rogue_syms[i] = (nhsym) 0;
}
/* initialize defaults for the overrides to the primary symset */
void
init_ov_primary_symbols()
{
    register int i;

    for (i = 0; i < SYM_MAX; i++)
        ov_primary_syms[i] = (nhsym) 0;
}

nhsym
get_othersym(idx, which_set)
int idx, which_set;
{
    nhsym sym = (nhsym) 0;
    int oidx = idx + SYM_OFF_X;

    if (which_set == ROGUESET)
        sym = ov_rogue_syms[oidx] ? ov_rogue_syms[oidx]
                                  : rogue_syms[oidx];
    else
        sym = ov_primary_syms[oidx] ? ov_primary_syms[oidx]
                                  : primary_syms[oidx];
    if (!sym) {
        switch(idx) {
            case SYM_BOULDER:
                sym = def_oc_syms[ROCK_CLASS].sym;
                break;
            case SYM_INVISIBLE:
                sym = DEF_INVISIBLE;
                break;
        }
    }
    return sym;
}

/* initialize defaults for the primary symset */
void
init_primary_symbols()
{
    register int i;

    for (i = 0; i < MAXPCHARS; i++)
        primary_syms[i + SYM_OFF_P] = defsyms[i].sym;
    for (i = 0; i < MAXOCLASSES; i++)
        primary_syms[i + SYM_OFF_O] = def_oc_syms[i].sym;
    for (i = 0; i < MAXMCLASSES; i++)
        primary_syms[i + SYM_OFF_M] = def_monsyms[i].sym;
    for (i = 0; i < WARNCOUNT; i++)
        primary_syms[i + SYM_OFF_W] = def_warnsyms[i].sym;
    for (i = 0; i < MAXOTHER; i++)
        primary_syms[i + SYM_OFF_X] = get_othersym(i, PRIMARY);

    clear_symsetentry(PRIMARY, FALSE);
}

/* initialize defaults for the rogue symset */
void
init_rogue_symbols()
{
    register int i;

    /* These are defaults that can get overwritten
       later by the roguesymbols option */

    for (i = 0; i < MAXPCHARS; i++)
        rogue_syms[i + SYM_OFF_P] = defsyms[i].sym;
    rogue_syms[S_vodoor] = rogue_syms[S_hodoor] = rogue_syms[S_ndoor] = '+';
    rogue_syms[S_upstair] = rogue_syms[S_dnstair] = '%';

    for (i = 0; i < MAXOCLASSES; i++)
        rogue_syms[i + SYM_OFF_O] = def_r_oc_syms[i];
    for (i = 0; i < MAXMCLASSES; i++)
        rogue_syms[i + SYM_OFF_M] = def_monsyms[i].sym;
    for (i = 0; i < WARNCOUNT; i++)
        rogue_syms[i + SYM_OFF_W] = def_warnsyms[i].sym;
    for (i = 0; i < MAXOTHER; i++)
        rogue_syms[i + SYM_OFF_X] = get_othersym(i, ROGUESET);

    clear_symsetentry(ROGUESET, FALSE);
    /* default on Rogue level is no color
     * but some symbol sets can override that
     */
    symset[ROGUESET].nocolor = 1;
}

void
assign_graphics(whichset)
int whichset;
{
    register int i;

    switch (whichset) {
    case ROGUESET:
        /* Adjust graphics display characters on Rogue levels */

        for (i = 0; i < SYM_MAX; i++)
            showsyms[i] = ov_rogue_syms[i] ? ov_rogue_syms[i]
                                           : rogue_syms[i];

#if defined(MSDOS) && defined(USE_TILES)
        if (iflags.grmode)
            tileview(FALSE);
#endif
        currentgraphics = ROGUESET;
        break;

    case PRIMARY:
    default:
        for (i = 0; i < SYM_MAX; i++)
            showsyms[i] = ov_primary_syms[i] ? ov_primary_syms[i]
                                             : primary_syms[i];

#if defined(MSDOS) && defined(USE_TILES)
        if (iflags.grmode)
            tileview(TRUE);
#endif
        currentgraphics = PRIMARY;
        break;
    }
}

void
switch_symbols(nondefault)
int nondefault;
{
    register int i;

    if (nondefault) {
        for (i = 0; i < SYM_MAX; i++)
            showsyms[i] = ov_primary_syms[i] ? ov_primary_syms[i]
                                             : primary_syms[i];
#ifdef PC9800
        if (SYMHANDLING(H_IBM) && ibmgraphics_mode_callback)
            (*ibmgraphics_mode_callback)();
        else if (SYMHANDLING(H_UNK) && ascgraphics_mode_callback)
            (*ascgraphics_mode_callback)();
#endif
#if defined(TERMLIB) || defined(CURSES_GRAPHICS)
        /* curses doesn't assign any routine to dec..._callback but
           probably does the expected initialization under the hood
           for terminals capable of rendering DECgraphics */
        if (SYMHANDLING(H_DEC) && decgraphics_mode_callback)
            (*decgraphics_mode_callback)();
# ifdef CURSES_GRAPHICS
        /* there aren't any symbol sets with CURS handling, and the
           curses interface never assigns a routine to curses..._callback */
        if (SYMHANDLING(H_CURS) && cursesgraphics_mode_callback)
            (*cursesgraphics_mode_callback)();
# endif
#endif
    } else {
        init_primary_symbols();
        init_showsyms();
    }
}

void
update_ov_primary_symset(symp, val)
struct symparse *symp;
int val;
{
    ov_primary_syms[symp->idx] = val;
}

void
update_ov_rogue_symset(symp, val)
struct symparse *symp;
int val;
{
    ov_rogue_syms[symp->idx] = val;
}

void
update_primary_symset(symp, val)
struct symparse *symp;
int val;
{
    primary_syms[symp->idx] = val;
}

void
update_rogue_symset(symp, val)
struct symparse *symp;
int val;
{
    rogue_syms[symp->idx] = val;
}

void
clear_symsetentry(which_set, name_too)
int which_set;
boolean name_too;
{
    if (symset[which_set].desc)
        free((genericptr_t) symset[which_set].desc);
    symset[which_set].desc = (char *) 0;

    symset[which_set].handling = H_UNK;
    symset[which_set].nocolor = 0;
    /* initialize restriction bits */
    symset[which_set].primary = 0;
    symset[which_set].rogue = 0;

    if (name_too) {
        if (symset[which_set].name)
            free((genericptr_t) symset[which_set].name);
        symset[which_set].name = (char *) 0;
    }
}

/*
 * If you are adding code somewhere to be able to recognize
 * particular types of symset "handling", define a
 * H_XXX macro in include/rm.h and add the name
 * to this array at the matching offset.
 */
const char *known_handling[] = {
    "UNKNOWN", /* H_UNK  */
    "IBM",     /* H_IBM  */
    "DEC",     /* H_DEC  */
    "CURS",    /* H_CURS */
    "MAC",     /* H_MAC  -- pre-OSX MACgraphics */
    (const char *) 0,
};

/*
 * Accepted keywords for symset restrictions.
 * These can be virtually anything that you want to
 * be able to test in the code someplace.
 * Be sure to:
 *    - add a corresponding Bitfield to the symsetentry struct in rm.h
 *    - initialize the field to zero in parse_sym_line in the SYM_CONTROL
 *      case 0 section of the idx switch. The location is prefaced with
 *      with a comment stating "initialize restriction bits".
 *    - set the value appropriately based on the index of your keyword
 *      under the case 5 sections of the same SYM_CONTROL idx switches.
 *    - add the field to clear_symsetentry()
 */
const char *known_restrictions[] = {
    "primary", "rogue", (const char *) 0,
};

struct symparse loadsyms[] = {
    { SYM_CONTROL, 0, "start" },
    { SYM_CONTROL, 0, "begin" },
    { SYM_CONTROL, 1, "finish" },
    { SYM_CONTROL, 2, "handling" },
    { SYM_CONTROL, 3, "description" },
    { SYM_CONTROL, 4, "color" },
    { SYM_CONTROL, 4, "colour" },
    { SYM_CONTROL, 5, "restrictions" },
    { SYM_PCHAR, S_stone, "S_stone" },
    { SYM_PCHAR, S_vwall, "S_vwall" },
    { SYM_PCHAR, S_hwall, "S_hwall" },
    { SYM_PCHAR, S_tlcorn, "S_tlcorn" },
    { SYM_PCHAR, S_trcorn, "S_trcorn" },
    { SYM_PCHAR, S_blcorn, "S_blcorn" },
    { SYM_PCHAR, S_brcorn, "S_brcorn" },
    { SYM_PCHAR, S_crwall, "S_crwall" },
    { SYM_PCHAR, S_tuwall, "S_tuwall" },
    { SYM_PCHAR, S_tdwall, "S_tdwall" },
    { SYM_PCHAR, S_tlwall, "S_tlwall" },
    { SYM_PCHAR, S_trwall, "S_trwall" },
    { SYM_PCHAR, S_ndoor, "S_ndoor" },
    { SYM_PCHAR, S_vodoor, "S_vodoor" },
    { SYM_PCHAR, S_hodoor, "S_hodoor" },
    { SYM_PCHAR, S_vcdoor, "S_vcdoor" },
    { SYM_PCHAR, S_hcdoor, "S_hcdoor" },
    { SYM_PCHAR, S_bars, "S_bars" },
    { SYM_PCHAR, S_tree, "S_tree" },
    { SYM_PCHAR, S_deadtree, "S_deadtree" },
    { SYM_PCHAR, S_room, "S_room" },
    { SYM_PCHAR, S_darkroom, "S_darkroom" },
    { SYM_PCHAR, S_corr, "S_corr" },
    { SYM_PCHAR, S_litcorr, "S_litcorr" },
    { SYM_PCHAR, S_upstair, "S_upstair" },
    { SYM_PCHAR, S_dnstair, "S_dnstair" },
    { SYM_PCHAR, S_upladder, "S_upladder" },
    { SYM_PCHAR, S_dnladder, "S_dnladder" },
    { SYM_PCHAR, S_altar, "S_altar" },
    { SYM_PCHAR, S_grave, "S_grave" },
    { SYM_PCHAR, S_throne, "S_throne" },
    { SYM_PCHAR, S_sink, "S_sink" },
    { SYM_PCHAR, S_forge, "S_forge" },
    { SYM_PCHAR, S_magic_chest, "S_magic_chest" },
    { SYM_PCHAR, S_fountain, "S_fountain" },
    { SYM_PCHAR, S_pool, "S_pool" },
    { SYM_PCHAR, S_ice, "S_ice" },
    { SYM_PCHAR, S_grass, "S_grass" },
    { SYM_PCHAR, S_sand, "S_sand" },
    { SYM_PCHAR, S_lava, "S_lava" },
    { SYM_PCHAR, S_vodbridge, "S_vodbridge" },
    { SYM_PCHAR, S_hodbridge, "S_hodbridge" },
    { SYM_PCHAR, S_vcdbridge, "S_vcdbridge" },
    { SYM_PCHAR, S_hcdbridge, "S_hcdbridge" },
    { SYM_PCHAR, S_air, "S_air" },
    { SYM_PCHAR, S_cloud, "S_cloud" },
    { SYM_PCHAR, S_poisoncloud, "S_poisoncloud" },
    { SYM_PCHAR, S_puddle, "S_puddle" },
    { SYM_PCHAR, S_sewage, "S_sewage" },
    { SYM_PCHAR, S_water, "S_water" },
    { SYM_PCHAR, S_arrow_trap, "S_arrow_trap" },
    { SYM_PCHAR, S_bolt_trap, "S_bolt_trap" },
    { SYM_PCHAR, S_dart_trap, "S_dart_trap" },
    { SYM_PCHAR, S_falling_rock_trap, "S_falling_rock_trap" },
    { SYM_PCHAR, S_squeaky_board, "S_squeaky_board" },
    { SYM_PCHAR, S_bear_trap, "S_bear_trap" },
    { SYM_PCHAR, S_land_mine, "S_land_mine" },
    { SYM_PCHAR, S_rolling_boulder_trap, "S_rolling_boulder_trap" },
    { SYM_PCHAR, S_sleeping_gas_trap, "S_sleeping_gas_trap" },
    { SYM_PCHAR, S_rust_trap, "S_rust_trap" },
    { SYM_PCHAR, S_fire_trap, "S_fire_trap" },
    { SYM_PCHAR, S_ice_trap, "S_ice_trap" },
    { SYM_PCHAR, S_pit, "S_pit" },
    { SYM_PCHAR, S_spiked_pit, "S_spiked_pit" },
    { SYM_PCHAR, S_hole, "S_hole" },
    { SYM_PCHAR, S_trap_door, "S_trap_door" },
    { SYM_PCHAR, S_teleportation_trap, "S_teleportation_trap" },
    { SYM_PCHAR, S_level_teleporter, "S_level_teleporter" },
    { SYM_PCHAR, S_magic_portal, "S_magic_portal" },
    { SYM_PCHAR, S_web, "S_web" },
    { SYM_PCHAR, S_statue_trap, "S_statue_trap" },
    { SYM_PCHAR, S_magic_trap, "S_magic_trap" },
    { SYM_PCHAR, S_anti_magic_trap, "S_anti_magic_trap" },
    { SYM_PCHAR, S_polymorph_trap, "S_polymorph_trap" },
    { SYM_PCHAR, S_spear_trap, "S_spear_trap" },
    { SYM_PCHAR, S_magic_beam_trap, "S_magic_beam_trap" },
    { SYM_PCHAR, S_vibrating_square, "S_vibrating_square" },
    { SYM_PCHAR, S_vbeam, "S_vbeam" },
    { SYM_PCHAR, S_hbeam, "S_hbeam" },
    { SYM_PCHAR, S_lslant, "S_lslant" },
    { SYM_PCHAR, S_rslant, "S_rslant" },
    { SYM_PCHAR, S_digbeam, "S_digbeam" },
    { SYM_PCHAR, S_flashbeam, "S_flashbeam" },
    { SYM_PCHAR, S_boomleft, "S_boomleft" },
    { SYM_PCHAR, S_boomright, "S_boomright" },
    { SYM_PCHAR, S_goodpos, "S_goodpos" },
    { SYM_PCHAR, S_ss1, "S_ss1" },
    { SYM_PCHAR, S_ss2, "S_ss2" },
    { SYM_PCHAR, S_ss3, "S_ss3" },
    { SYM_PCHAR, S_ss4, "S_ss4" },
    { SYM_PCHAR, S_sw_tl, "S_sw_tl" },
    { SYM_PCHAR, S_sw_tc, "S_sw_tc" },
    { SYM_PCHAR, S_sw_tr, "S_sw_tr" },
    { SYM_PCHAR, S_sw_ml, "S_sw_ml" },
    { SYM_PCHAR, S_sw_mr, "S_sw_mr" },
    { SYM_PCHAR, S_sw_bl, "S_sw_bl" },
    { SYM_PCHAR, S_sw_bc, "S_sw_bc" },
    { SYM_PCHAR, S_sw_br, "S_sw_br" },
    { SYM_PCHAR, S_explode1, "S_explode1" },
    { SYM_PCHAR, S_explode2, "S_explode2" },
    { SYM_PCHAR, S_explode3, "S_explode3" },
    { SYM_PCHAR, S_explode4, "S_explode4" },
    { SYM_PCHAR, S_explode5, "S_explode5" },
    { SYM_PCHAR, S_explode6, "S_explode6" },
    { SYM_PCHAR, S_explode7, "S_explode7" },
    { SYM_PCHAR, S_explode8, "S_explode8" },
    { SYM_PCHAR, S_explode9, "S_explode9" },
    { SYM_OC, ILLOBJ_CLASS + SYM_OFF_O, "S_strange_obj" },
    { SYM_OC, WEAPON_CLASS + SYM_OFF_O, "S_weapon" },
    { SYM_OC, ARMOR_CLASS + SYM_OFF_O, "S_armor" },
    { SYM_OC, ARMOR_CLASS + SYM_OFF_O, "S_armour" },
    { SYM_OC, RING_CLASS + SYM_OFF_O, "S_ring" },
    { SYM_OC, AMULET_CLASS + SYM_OFF_O, "S_amulet" },
    { SYM_OC, TOOL_CLASS + SYM_OFF_O, "S_tool" },
    { SYM_OC, FOOD_CLASS + SYM_OFF_O, "S_food" },
    { SYM_OC, POTION_CLASS + SYM_OFF_O, "S_potion" },
    { SYM_OC, SCROLL_CLASS + SYM_OFF_O, "S_scroll" },
    { SYM_OC, SPBOOK_CLASS + SYM_OFF_O, "S_book" },
    { SYM_OC, WAND_CLASS + SYM_OFF_O, "S_wand" },
    { SYM_OC, COIN_CLASS + SYM_OFF_O, "S_coin" },
    { SYM_OC, GEM_CLASS + SYM_OFF_O, "S_gem" },
    { SYM_OC, ROCK_CLASS + SYM_OFF_O, "S_rock" },
    { SYM_OC, BALL_CLASS + SYM_OFF_O, "S_ball" },
    { SYM_OC, CHAIN_CLASS + SYM_OFF_O, "S_chain" },
    { SYM_OC, VENOM_CLASS + SYM_OFF_O, "S_venom" },
    { SYM_MON, S_ANT + SYM_OFF_M, "S_ant" },
    { SYM_MON, S_BLOB + SYM_OFF_M, "S_blob" },
    { SYM_MON, S_COCKATRICE + SYM_OFF_M, "S_cockatrice" },
    { SYM_MON, S_DOG + SYM_OFF_M, "S_dog" },
    { SYM_MON, S_EYE + SYM_OFF_M, "S_eye" },
    { SYM_MON, S_FELINE + SYM_OFF_M, "S_feline" },
    { SYM_MON, S_GREMLIN + SYM_OFF_M, "S_gremlin" },
    { SYM_MON, S_HUMANOID + SYM_OFF_M, "S_humanoid" },
    { SYM_MON, S_IMP + SYM_OFF_M, "S_imp" },
    { SYM_MON, S_JELLY + SYM_OFF_M, "S_jelly" },
    { SYM_MON, S_KOBOLD + SYM_OFF_M, "S_kobold" },
    { SYM_MON, S_LEPRECHAUN + SYM_OFF_M, "S_leprechaun" },
    { SYM_MON, S_MIMIC + SYM_OFF_M, "S_mimic" },
    { SYM_MON, S_NYMPH + SYM_OFF_M, "S_nymph" },
    { SYM_MON, S_ORC + SYM_OFF_M, "S_orc" },
    { SYM_MON, S_PIERCER + SYM_OFF_M, "S_piercer" },
    { SYM_MON, S_QUADRUPED + SYM_OFF_M, "S_quadruped" },
    { SYM_MON, S_RODENT + SYM_OFF_M, "S_rodent" },
    { SYM_MON, S_SPIDER + SYM_OFF_M, "S_spider" },
    { SYM_MON, S_TRAPPER + SYM_OFF_M, "S_trapper" },
    { SYM_MON, S_UNICORN + SYM_OFF_M, "S_unicorn" },
    { SYM_MON, S_VORTEX + SYM_OFF_M, "S_vortex" },
    { SYM_MON, S_WORM + SYM_OFF_M, "S_worm" },
    { SYM_MON, S_XAN + SYM_OFF_M, "S_xan" },
    { SYM_MON, S_LIGHT + SYM_OFF_M, "S_light" },
    { SYM_MON, S_SKELETON + SYM_OFF_M, "S_skeleton" },
    { SYM_MON, S_ANGEL + SYM_OFF_M, "S_angel" },
    { SYM_MON, S_BAT + SYM_OFF_M, "S_bat" },
    { SYM_MON, S_CENTAUR + SYM_OFF_M, "S_centaur" },
    { SYM_MON, S_DRAGON + SYM_OFF_M, "S_dragon" },
    { SYM_MON, S_ELEMENTAL + SYM_OFF_M, "S_elemental" },
    { SYM_MON, S_FUNGUS + SYM_OFF_M, "S_fungus" },
    { SYM_MON, S_GNOME + SYM_OFF_M, "S_gnome" },
    { SYM_MON, S_GIANT + SYM_OFF_M, "S_giant" },
    { SYM_MON, S_JABBERWOCK + SYM_OFF_M, "S_jabberwock" },
    { SYM_MON, S_KOP + SYM_OFF_M, "S_kop" },
    { SYM_MON, S_LICH + SYM_OFF_M, "S_lich" },
    { SYM_MON, S_MUMMY + SYM_OFF_M, "S_mummy" },
    { SYM_MON, S_NAGA + SYM_OFF_M, "S_naga" },
    { SYM_MON, S_OGRE + SYM_OFF_M, "S_ogre" },
    { SYM_MON, S_PUDDING + SYM_OFF_M, "S_pudding" },
    { SYM_MON, S_QUANTMECH + SYM_OFF_M, "S_quantmech" },
    { SYM_MON, S_RUSTMONST + SYM_OFF_M, "S_rustmonst" },
    { SYM_MON, S_SNAKE + SYM_OFF_M, "S_snake" },
    { SYM_MON, S_TROLL + SYM_OFF_M, "S_troll" },
    { SYM_MON, S_UMBER + SYM_OFF_M, "S_umber" },
    { SYM_MON, S_VAMPIRE + SYM_OFF_M, "S_vampire" },
    { SYM_MON, S_WRAITH + SYM_OFF_M, "S_wraith" },
    { SYM_MON, S_XORN + SYM_OFF_M, "S_xorn" },
    { SYM_MON, S_YETI + SYM_OFF_M, "S_yeti" },
    { SYM_MON, S_ZOMBIE + SYM_OFF_M, "S_zombie" },
    { SYM_MON, S_HUMAN + SYM_OFF_M, "S_human" },
    { SYM_MON, S_GHOST + SYM_OFF_M, "S_ghost" },
    { SYM_MON, S_GOLEM + SYM_OFF_M, "S_golem" },
    { SYM_MON, S_DEMON + SYM_OFF_M, "S_demon" },
    { SYM_MON, S_EEL + SYM_OFF_M, "S_eel" },
    { SYM_MON, S_LIZARD + SYM_OFF_M, "S_lizard" },
    { SYM_MON, S_ENT + SYM_OFF_M, "S_ent" },
    { SYM_MON, S_PLANT + SYM_OFF_M, "S_plant" },
    { SYM_MON, S_WORM_TAIL + SYM_OFF_M, "S_worm_tail" },
    { SYM_MON, S_MIMIC_DEF + SYM_OFF_M, "S_mimic_def" },
    { SYM_OTH, SYM_BOULDER + SYM_OFF_X, "S_boulder" },
    { SYM_OTH, SYM_INVISIBLE + SYM_OFF_X, "S_invisible" },
    { SYM_OTH, SYM_PET_OVERRIDE + SYM_OFF_X, "S_pet_override" },
    { SYM_OTH, SYM_HERO_OVERRIDE + SYM_OFF_X, "S_hero_override" },
    { 0, 0, (const char *) 0 } /* fence post */
};

/*drawing.c*/
