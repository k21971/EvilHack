/* NetHack 3.6	hack.h	$NHDT-Date: 1561019041 2019/06/20 08:24:01 $  $NHDT-Branch: NetHack-3.6 $:$NHDT-Revision: 1.106 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Pasi Kallinen, 2017. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef HACK_H
#define HACK_H

#ifndef CONFIG_H
#include "config.h"
#endif
#include "lint.h"

#define TELL 1
#define NOTELL 0
#define ON 1
#define OFF 0
#define BOLT_LIM 8        /* from this distance ranged attacks will be made */
#define MAX_CARR_CAP 1000 /* so that boulders can be heavier */
#define DUMMY { 0 }       /* array initializer, letting [1..N-1] default */

/* symbolic names for capacity levels */
enum encumbrance_types {
    UNENCUMBERED = 0,
    SLT_ENCUMBER = 1, /* Burdened */
    MOD_ENCUMBER = 2, /* Stressed */
    HVY_ENCUMBER = 3, /* Strained */
    EXT_ENCUMBER = 4, /* Overtaxed */
    OVERLOADED   = 5  /* Overloaded */
};

/* weight increment of heavy iron ball */
#define IRON_BALL_W_INCR 160

/* number of turns it takes for vault guard to show up */
#define VAULT_GUARD_TIME 30

#define SHOP_DOOR_COST 400L /* cost of a destroyed shop door */
#define SHOP_BARS_COST 300L /* cost of iron bars */
#define SHOP_HOLE_COST 200L /* cost of making hole/trapdoor */
#define SHOP_WALL_COST 200L /* cost of destroying a wall */
#define SHOP_WALL_DMG  (10L * ACURRSTR) /* damaging a wall */
#define SHOP_PIT_COST  100L /* cost of making a pit */
#define SHOP_WEB_COST   30L /* cost of removing a web */

/* hunger states - see hu_stat in eat.c */
enum hunger_state_types {
    SATIATED   = 0,
    NOT_HUNGRY = 1,
    HUNGRY     = 2,
    WEAK       = 3,
    FRAIL      = 4, /* vampire race only */
    FAINTING   = 5,
    FAINTED    = 6,
    STARVED    = 7
};

/* Macros for how a rumor was delivered in outrumor() */
#define BY_ORACLE 0
#define BY_COOKIE 1
#define BY_PAPER 2
#define BY_OTHER 9

/* Macros for why you are no longer riding */
enum dismount_types {
    DISMOUNT_GENERIC  = 0,
    DISMOUNT_FELL     = 1,
    DISMOUNT_THROWN   = 2,
    DISMOUNT_POLY     = 3,
    DISMOUNT_ENGULFED = 4,
    DISMOUNT_BONES    = 5,
    DISMOUNT_BYCHOICE = 6
};

/* mgflags for mapglyph() */
#define MG_FLAG_NORMAL     0x00
#define MG_FLAG_NOOVERRIDE 0x01

/* Special returns from mapglyph() */
#define MG_CORPSE   0x00001
#define MG_INVIS    0x00002
#define MG_DETECT   0x00004
#define MG_PET      0x00008
#define MG_RIDDEN   0x00010
#define MG_STATUE   0x00020
#define MG_OBJPILE  0x00040  /* more than one stack of objects */
#define MG_BW_LAVA  0x00080  /* 'black & white lava': highlight lava if it
                                 can't be distringuished from water by color */
#define MG_PEACEFUL 0x00100  /* peaceful monster */
#define MG_STAIRS   0x00200  /* hidden stairs */

/* sellobj_state() states */
#define SELL_NORMAL (0)
#define SELL_DELIBERATE (1)
#define SELL_DONTSELL (2)

/* alteration types--keep in synch with costly_alteration(mkobj.c) */
enum cost_alteration_types {
    COST_CANCEL  =  0, /* standard cancellation */
    COST_DRAIN   =  1, /* drain life upon an object */
    COST_UNCHRG  =  2, /* cursed charging */
    COST_UNBLSS  =  3, /* unbless (devalues holy water) */
    COST_UNCURS  =  4, /* uncurse (devalues unholy water) */
    COST_DECHNT  =  5, /* disenchant weapons or armor */
    COST_DEGRD   =  6, /* removal of rustproofing, dulling via engraving */
    COST_DILUTE  =  7, /* potion dilution */
    COST_ERASE   =  8, /* scroll or spellbook blanking */
    COST_BURN    =  9, /* dipped into flaming oil */
    COST_NUTRLZ  = 10, /* neutralized via unicorn horn */
    COST_DSTROY  = 11, /* wand breaking (bill first, useup later) */
    COST_SPLAT   = 12, /* cream pie to own face (ditto) */
    COST_BITE    = 13, /* start eating food */
    COST_OPEN    = 14, /* open tin */
    COST_BRKLCK  = 15, /* break box/chest's lock */
    COST_RUST    = 16, /* rust damage */
    COST_ROT     = 17, /* rotting attack */
    COST_CORRODE = 18, /* acid damage */
    COST_FRACTURE = 19,   /* glass damage */
    COST_DETERIORATE = 20 /* other material damage */
};

/* bitmask flags for corpse_xname();
   PFX_THE takes precedence over ARTICLE, NO_PFX takes precedence over both */
#define CXN_NORMAL 0    /* no special handling */
#define CXN_SINGULAR 1  /* override quantity if greather than 1 */
#define CXN_NO_PFX 2    /* suppress "the" from "the Unique Monst */
#define CXN_PFX_THE 4   /* prefix with "the " (unless pname) */
#define CXN_ARTICLE 8   /* include a/an/the prefix */
#define CXN_NOCORPSE 16 /* suppress " corpse" suffix */

/* getpos() return values */
enum getpos_retval {
    LOOK_TRADITIONAL = 0, /* '.' -- ask about "more info?" */
    LOOK_QUICK       = 1, /* ',' -- skip "more info?" */
    LOOK_ONCE        = 2, /* ';' -- skip and stop looping */
    LOOK_VERBOSE     = 3  /* ':' -- show more info w/o asking */
};

/* Different ways fire damage can be dealt for on_fire. */
enum on_fire_types {
    ON_FIRE         = 0, /* Standard fire damage attack. */
    ON_FIRE_HUG     = 1, /* Hugged by fire damage monster. */
    ON_FIRE_ENGULF  = 2, /* Engulfed by fire damage monster. */
    ON_FIRE_DEAD    = 3  /* Killed by fire damage. */
};

/*
 * This is the way the game ends.  If these are rearranged, the arrays
 * in end.c and topten.c will need to be changed.  Some parts of the
 * code assume that PANIC separates the deaths from the non-deaths.
 */
enum game_end_types {
    DIED          =  0,
    CHOKING       =  1,
    POISONING     =  2,
    STARVING      =  3,
    DROWNING      =  4,
    BURNING       =  5,
    DISSOLVED     =  6,
    CRUSHING      =  7,
    STONING       =  8,
    TURNED_SLIME  =  9,
    DECAPITATED   = 10,
    INCINERATED   = 11,
    DISINTEGRATED = 12,
    GENOCIDED     = 13,
    PANICKED      = 14,
    TRICKED       = 15,
    QUIT          = 16,
    ESCAPED       = 17,
    ASCENDED      = 18
};

typedef struct strbuf {
    int    len;
    char * str;
    char   buf[256];
} strbuf_t;

#include "align.h"
#include "dungeon.h"
#include "monsym.h"
#include "mkroom.h"
#include "objclass.h"
#include "youprop.h"
#include "wintype.h"
#include "context.h"
#include "decl.h"
#include "timeout.h"

NEARDATA extern coord bhitpos; /* place where throw or zap hits or stops */

/* types of calls to bhit() */
enum bhit_call_types {
    ZAPPED_WAND   = 0,
    THROWN_WEAPON = 1,
    THROWN_TETHERED_WEAPON = 2,
    KICKED_WEAPON = 3,
    FLASHED_LIGHT = 4,
    INVIS_BEAM    = 5
};

/* zap type definitions, formerly in zap.c */
#define ZT_MAGIC_MISSILE (AD_MAGM - 1)
#define ZT_FIRE (AD_FIRE - 1)
#define ZT_COLD (AD_COLD - 1)
#define ZT_SLEEP (AD_SLEE - 1)
#define ZT_DEATH (AD_DISN - 1) /* or disintegration */
#define ZT_LIGHTNING (AD_ELEC - 1)
#define ZT_POISON_GAS (AD_DRST - 1)
#define ZT_ACID (AD_ACID - 1)
#define ZT_WATER (AD_WATR - 1)
#define ZT_DRAIN (AD_DRLI - 1)
#define ZT_STUN (AD_STUN - 1)

#define MAX_ZT (ZT_STUN + 1)

#define BASE_ZT(x) ((x) % MAX_ZT)

#define ZT_WAND(x) (x)
#define ZT_SPELL(x) (MAX_ZT + (x))
#define ZT_BREATH(x) (MAX_ZT + MAX_ZT + (x))

/* attack mode for hmon() */
enum hmon_atkmode_types {
    HMON_MELEE   = 0, /* hand-to-hand */
    HMON_THROWN  = 1, /* normal ranged (or spitting while poly'd) */
    HMON_KICKED  = 2, /* alternate ranged */
    HMON_APPLIED = 3, /* polearm, treated as ranged */
    HMON_DRAGGED = 4  /* attached iron ball, pulled into mon */
};

/* sortloot() return type; needed before extern.h */
struct sortloot_item {
    struct obj *obj;
    char *str; /* result of loot_xname(obj) in some cases, otherwise null */
    int indx; /* signed int, because sortloot()'s qsort comparison routine
                 assumes (a->indx - b->indx) might yield a negative result */
    xchar orderclass; /* order rather than object class; 0 => not yet init'd */
    xchar subclass; /* subclass for some classes */
    xchar disco; /* discovery status */
};
typedef struct sortloot_item Loot;

#define MATCH_WARN_OF_MON(mon)                                               \
    (Warn_of_mon && ((context.warntype.obj                                   \
                      && ((!has_erac(mon) && (context.warntype.obj & (mon)->data->mhflags)) \
                          || (has_erac(mon) && (context.warntype.obj & ERAC(mon)->mrace))))      \
                     || (context.warntype.polyd                              \
                        && ((!has_erac(mon) && (context.warntype.polyd & (mon)->data->mhflags)) \
                            || (has_erac(mon) && (context.warntype.polyd & ERAC(mon)->mrace))))      \
                     || (context.warntype.species                            \
                         && (context.warntype.species == (mon)->data))))

#include "trap.h"
#include "flag.h"
#include "rm.h"
#include "vision.h"
#include "display.h"
#include "engrave.h"
#include "rect.h"
#include "region.h"

/* Symbol offsets */
#define SYM_OFF_P (0)
#define SYM_OFF_O (SYM_OFF_P + MAXPCHARS)
#define SYM_OFF_M (SYM_OFF_O + MAXOCLASSES)
#define SYM_OFF_W (SYM_OFF_M + MAXMCLASSES)
#define SYM_OFF_X (SYM_OFF_W + WARNCOUNT)
#define SYM_MAX (SYM_OFF_X + MAXOTHER)

#ifdef USE_TRAMPOLI /* this doesn't belong here, but we have little choice */
#undef NDECL
#define NDECL(f) f()
#endif

#include "extern.h"
#include "winprocs.h"
#include "sys.h"

#ifdef USE_TRAMPOLI
#include "wintty.h"
#undef WINTTY_H
#include "trampoli.h"
#undef EXTERN_H
#include "extern.h"
#endif /* USE_TRAMPOLI */

/* flags to control makemon(); goodpos() uses some plus has some of its own */
#define NO_MM_FLAGS 0x000000L /* use this rather than plain 0 */
#define NO_MINVENT  0x000001L /* suppress minvent when creating mon */
#define MM_NOWAIT   0x000002L /* don't set STRAT_WAITMASK flags */
#define MM_NOCOUNTBIRTH 0x000004L /* don't increment born count (for revival) */
#define MM_IGNOREWATER  0x000008L /* ignore water when positioning */
#define MM_ADJACENTOK   0x000010L /* acceptable to use adjacent coordinates */
#define MM_ANGRY    0x000020L /* monster is created angry */
#define MM_NONAME   0x000040L /* monster is not christened */
#define MM_EGD      0x000100L /* add egd structure */
#define MM_EPRI     0x000200L /* add epri structure */
#define MM_ESHK     0x000400L /* add eshk structure */
#define MM_EMIN     0x000800L /* add emin structure */
#define MM_EDOG     0x001000L /* add edog structure */
#define MM_ASLEEP   0x002000L /* monsters should be generated asleep */
#define MM_NOGRP    0x004000L /* suppress creation of monster groups */
#define MM_ERID     0x008000L /* add erid structure */
#define MM_REVIVE   0x010000L /* no riding */
#define MM_MPLAYEROK 0x020000L /* allow mplayer creation */
/* if more MM_ flag masks are added, skip or renumber the GP_ one(s) */
#define GP_ALLOW_XY 0x040000L /* [actually used by enexto() to decide whether
                             * to make an extra call to goodpos()]          */
#define GP_ALLOW_U  0x080000L /* don't reject hero's location */
#define MM_IGNORELAVA 0x100000L /* ignore lava when positioning */
#define MM_IGNOREAIR  0x200000L /* ignore air when positioning */

/* flags for make_corpse() and mkcorpstat() */
#define CORPSTAT_NONE 0x00
#define CORPSTAT_INIT 0x01   /* pass init flag to mkcorpstat */
#define CORPSTAT_BURIED 0x02 /* bury the corpse or statue */
#define CORPSTAT_ZOMBIE 0x04 /* zombie corpse can revive */

/* flags for decide_to_shift() */
#define SHIFT_SEENMSG 0x01 /* put out a message if in sight */
#define SHIFT_MSG 0x02     /* always put out a message */

/* m_poisongas_ok() return values */
#define M_POISONGAS_BAD   0 /* poison gas is bad */
#define M_POISONGAS_MINOR 1 /* poison gas is ok, maybe causes coughing */
#define M_POISONGAS_OK    2 /* ignores poison gas completely */

/* flags for deliver_obj_to_mon */
#define DF_NONE     0x00
#define DF_RANDOM   0x01
#define DF_ALL      0x04

/* special mhpmax value when loading bones monster to flag as extinct or
 * genocided */
#define DEFUNCT_MONSTER (-100)

/* macro form of adjustments of physical damage based on Half_physical_damage.
 * Can be used on-the-fly with the 1st parameter to losehp() if you don't
 * need to retain the dmg value beyond that call scope.
 * Take care to ensure it doesn't get used more than once in other instances.
 */
#define Maybe_Half_Phys(dmg) \
    ((Half_physical_damage) ? (((dmg) + 1) / 2) : (dmg))

/* flags for special ggetobj status returns */
#define ALL_FINISHED 0x01 /* called routine already finished the job */

/* flags to control query_objlist() */
#define BY_NEXTHERE     0x01   /* follow objlist by nexthere field */
#define AUTOSELECT_SINGLE 0x02 /* if only 1 object, don't ask */
#define USE_INVLET      0x04   /* use object's invlet */
#define INVORDER_SORT   0x08   /* sort objects by packorder */
#define SIGNAL_NOMENU   0x10   /* return -1 rather than 0 if none allowed */
#define SIGNAL_ESCAPE   0x20   /* return -2 rather than 0 for ESC */
#define FEEL_COCKATRICE 0x40   /* engage cockatrice checks and react */
#define INCLUDE_HERO    0x80   /* show hero among engulfer's inventory */

/* Flags to control query_category() */
/* BY_NEXTHERE used by query_category() too, so skip 0x01 */
#define UNPAID_TYPES 0x002
#define GOLD_TYPES   0x004
#define WORN_TYPES   0x008
#define ALL_TYPES    0x010
#define BILLED_TYPES 0x020
#define CHOOSE_ALL   0x040
#define BUC_BLESSED  0x080
#define BUC_CURSED   0x100
#define BUC_UNCURSED 0x200
#define BUC_UNKNOWN  0x400
#define UNIDED_TYPES 0x800
#define BUC_ALLBKNOWN (BUC_BLESSED | BUC_CURSED | BUC_UNCURSED)
#define BUCX_TYPES (BUC_ALLBKNOWN | BUC_UNKNOWN)
#define ALL_TYPES_SELECTED -2

/* Flags to control find_mid() */
#define FM_FMON 0x01    /* search the fmon chain */
#define FM_MIGRATE 0x02 /* search the migrating monster chain */
#define FM_MYDOGS 0x04  /* search mydogs */
#define FM_EVERYWHERE (FM_FMON | FM_MIGRATE | FM_MYDOGS)

/* Flags to control pick_[race,role,gend,align] routines in role.c */
#define PICK_RANDOM 0
#define PICK_RIGID 1

/* Flags to control dotrap() in trap.c */
#define FORCETRAP 0x01     /* triggering not left to chance */
#define NOWEBMSG 0x02      /* suppress stumble into web message */
#define FORCEBUNGLE 0x04   /* adjustments appropriate for bungling */
#define RECURSIVETRAP 0x08 /* trap changed into another type this same turn */
#define TOOKPLUNGE 0x10    /* used '>' to enter pit below you */
#define VIASITTING 0x20    /* #sit while at trap location (affects message) */
#define FAILEDUNTRAP 0x40  /* trap activated by failed untrap attempt */

/* Flags to control test_move in hack.c */
#define DO_MOVE 0   /* really doing the move */
#define TEST_MOVE 1 /* test a normal move (move there next) */
#define TEST_TRAV 2 /* test a future travel location */
#define TEST_TRAP 3 /* check if a future travel loc is a trap */

/*** some utility macros ***/
#define yn(query) yn_function(query, ynchars, 'n')
#define ynq(query) yn_function(query, ynqchars, 'q')
#define ynaq(query) yn_function(query, ynaqchars, 'y')
#define nyaq(query) yn_function(query, ynaqchars, 'n')
#define nyNaq(query) yn_function(query, ynNaqchars, 'n')
#define ynNaq(query) yn_function(query, ynNaqchars, 'y')

/* Macros for scatter */
#define VIS_EFFECTS 0x01 /* display visual effects */
#define MAY_HITMON 0x02  /* objects may hit monsters */
#define MAY_HITYOU 0x04  /* objects may hit you */
#define MAY_HIT (MAY_HITMON | MAY_HITYOU)
#define MAY_DESTROY 0x08  /* objects may be destroyed at random */
#define MAY_FRACTURE 0x10 /* boulders & statues may fracture */

/* Macros for launching objects */
#define ROLL 0x01          /* the object is rolling */
#define FLING 0x02         /* the object is flying thru the air */
#define LAUNCH_UNSEEN 0x40 /* hero neither caused nor saw it */
#define LAUNCH_KNOWN 0x80  /* the hero caused this by explicit action */

/* Macros for explosion types */
enum explosion_types {
    EXPL_DARK    = 0,
    EXPL_NOXIOUS = 1,
    EXPL_MUDDY   = 2,
    EXPL_WET     = 3,
    EXPL_MAGICAL = 4,
    EXPL_FIERY   = 5,
    EXPL_FROSTY  = 6,
    EXPL_ACID    = 7,
    EXPL_SHOCK   = 8,
    EXPL_MAX     = 9
};

/* enlightenment control flags */
#define BASICENLIGHTENMENT 1 /* show mundane stuff */
#define MAGICENLIGHTENMENT 2 /* show intrinsics and such */
#define ENL_GAMEINPROGRESS 0
#define ENL_GAMEOVERALIVE  1 /* ascension, escape, quit, trickery */
#define ENL_GAMEOVERDEAD   2

/* control flags for sortloot() */
#define SORTLOOT_PACK   0x01
#define SORTLOOT_INVLET 0x02
#define SORTLOOT_LOOT   0x04
#define SORTLOOT_PETRIFY 0x20 /* override filter func for c-trice corpses */

/* flags for xkilled() [note: meaning of first bit used to be reversed,
   1 to give message and 0 to suppress] */
#define XKILL_GIVEMSG   0x0
#define XKILL_NOMSG     0x1
#define XKILL_NOCORPSE  0x2
#define XKILL_NOCONDUCT 0x4
#define XKILL_INDIRECT  0x8 /* from exploding summoned sphere */

/* pline_flags; mask values for custompline()'s first argument */
/* #define PLINE_ORDINARY 0 */
#define PLINE_NOREPEAT   1
#define OVERRIDE_MSGTYPE 2
#define SUPPRESS_HISTORY 4
#define URGENT_MESSAGE   8

/* Macros for messages referring to hands, eyes, feet, etc... */
enum bodypart_types {
    ARM       =  0,
    EYE       =  1,
    FACE      =  2,
    FINGER    =  3,
    FINGERTIP =  4,
    FOOT      =  5,
    HAND      =  6,
    HANDED    =  7,
    HEAD      =  8,
    LEG       =  9,
    LIGHT_HEADED = 10,
    NECK      = 11,
    SPINE     = 12,
    TOE       = 13,
    HAIR      = 14,
    BLOOD     = 15,
    LUNG      = 16,
    NOSE      = 17,
    STOMACH   = 18,
    SKIN      = 19
};

struct forge_recipe {
    short result_typ;
    short typ1;
    short typ2;
    long quan_typ1;
    long quan_typ2;
};

extern const struct forge_recipe fusions[]; /* array of forge recipes */

struct potion_alchemy {
    short result_typ;
    short typ1;
    short typ2;
    int chance;
};

extern const struct potion_alchemy potion_fusions[]; /* array of potion alchemy types */

struct trap_recipe {
    short result_typ;
    short comp;
    long quan;
};

extern const struct trap_recipe trap_fusions[]; /* array of trap recipes */

/* indices for some special tin types */
#define ROTTEN_TIN 0
#define HOMEMADE_TIN 1
#define SPINACH_TIN (-1)
#define RANDOM_TIN (-2)
#define HEALTHY_TIN (-3)

/* Some misc definitions */
#define POTION_OCCUPANT_CHANCE(n) (13 + 2 * (n))
#define WAND_BACKFIRE_CHANCE 30
#define BALL_IN_MON (u.uswallow && uball && uball->where == OBJ_FREE)
#define CHAIN_IN_MON (u.uswallow && uchain && uchain->where == OBJ_FREE)
#define NODIAG(monnum) ((monnum) == PM_GRID_BUG)

/* Flags to control menus */
#define MENUTYPELEN sizeof("traditional ")
#define MENU_TRADITIONAL 0
#define MENU_COMBINATION 1
#define MENU_FULL 2
#define MENU_PARTIAL 3

#define MENU_SELECTED TRUE
#define MENU_UNSELECTED FALSE

/* flags to control teleds() */
#define TELEDS_NO_FLAGS 0
#define TELEDS_ALLOW_DRAG 1
#define TELEDS_TELEPORT 2

/* constant passed to explode() for gas spores because gas spores are weird
 * Specifically, this is an exception to the whole "explode() uses dobuzz types"
 * system (the range -1 to -9 isn't used by it, for some reason), where this is
 * effectively an extra dobuzz type, and some zap.c code needs to be aware of
 * it.  */
#define PHYS_EXPL_TYPE -1

/* flags for hero_breaks() and hits_bars(); BRK_KNOWN* let callers who have
   already called breaktest() prevent it from being called again since it
   has a random factor which makes it be non-deterministic */
#define BRK_BY_HERO        1
#define BRK_FROM_INV       2
#define BRK_KNOWN2BREAK    4
#define BRK_KNOWN2NOTBREAK 8
#define BRK_KNOWN_OUTCOME  (BRK_KNOWN2BREAK | BRK_KNOWN2NOTBREAK)

/*
 * Option flags
 * Each higher number includes the characteristics of the numbers
 * below it.
 */
/* XXX This should be replaced with a bitmap. */
#define SET_IN_SYS 0   /* system config file option only */
#define SET_IN_FILE 1  /* config file option only */
#define SET_VIA_PROG 2 /* may be set via extern program, not seen in game */
#define DISP_IN_GAME 3 /* may be set via extern program, displayed in game \
                          */
#define SET_IN_GAME 4  /* may be set via extern program or set in the game */
#define SET_IN_WIZGAME 5  /* may be set set in the game if wizmode */
#define SET__IS_VALUE_VALID(s) ((s < SET_IN_SYS) || (s > SET_IN_WIZGAME))

#define FEATURE_NOTICE_VER(major, minor, patch)                    \
    (((unsigned long) major << 24) | ((unsigned long) minor << 16) \
     | ((unsigned long) patch << 8) | ((unsigned long) 0))

#define FEATURE_NOTICE_VER_MAJ (flags.suppress_alert >> 24)
#define FEATURE_NOTICE_VER_MIN \
    (((unsigned long) (0x0000000000FF0000L & flags.suppress_alert)) >> 16)
#define FEATURE_NOTICE_VER_PATCH \
    (((unsigned long) (0x000000000000FF00L & flags.suppress_alert)) >> 8)

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef min
#define min(x, y) ((x) < (y) ? (x) : (y))
#endif
#define plur(x) (((x) == 1) ? "" : "s")

#define makeknown(x) discover_object((x), TRUE, TRUE)
#define distu(xx, yy) dist2((int)(xx), (int)(yy), (int) u.ux, (int) u.uy)
#define onlineu(xx, yy) online2((int)(xx), (int)(yy), (int) u.ux, (int) u.uy)

#define rn1(x, y) (rn2(x) + (y))

/* negative armor class is randomly weakened to prevent invulnerability */
#define AC_VALUE(AC) ((AC) >= 0 ? (AC) : -rnd(-(AC)))

#if defined(MICRO) && !defined(__DJGPP__)
#define getuid() 1
#define getlogin() ((char *) 0)
#endif /* MICRO */

#if defined(OVERLAY)
#define USE_OVLx
#define STATIC_DCL extern
#define STATIC_OVL
#define STATIC_VAR

#else /* !OVERLAY */

#define STATIC_DCL static
#define STATIC_OVL static
#define STATIC_VAR static

#endif /* OVERLAY */

/* Macro for a few items that are only static if we're not overlaid.... */
#if defined(USE_TRAMPOLI) || defined(USE_OVLx)
#define STATIC_PTR
#else
#define STATIC_PTR static
#endif

/* The function argument to qsort() requires a particular
 * calling convention under WINCE which is not the default
 * in that environment.
 */
#if defined(WIN_CE)
#define CFDECLSPEC __cdecl
#else
#define CFDECLSPEC
#endif

#define DEVTEAM_EMAIL "admin@hardfought.org"
#define DEVTEAM_URL "https://github.com/k21971/EvilHack"

#endif /* HACK_H */
