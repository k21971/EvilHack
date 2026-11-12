/* $NHDT-Header: NetHack 5.0 objects.h 2026/06/20_22:26:43 NetHack-5.0:1.41 $ */
/* Copyright (c) Mike Threepoint, 1989.                           */
/* NetHack may be freely redistributed.  See license for details. */

/*
   The data in this file is processed multiple times by its inclusion
   in several places in the code. The results depend on the definition
   of the following:
     OBJECTS_ENUM        to construct object onames enum entries (decl.h).
     OBJECTS_DESCR_INIT to construct obj_descr[] array entries (objects.c).
     OBJECTS_INIT       to construct objects[] array entries (objects.c).
*/

#ifndef NoDes
#define NoDes (char *) 0 /* less visual distraction for 'no description' */
#endif

#ifndef lint
#define HARDGEM(n) (n >= 8)
#else
#define HARDGEM(n) (0)
#endif

/*
 * Note...
 *  OBJECTS() currently has 15 parameters; it more become needed, some
 *  will need to be combined the way BITS() is used, because compilers
 *  are allowed to impose a limit of 15.
 */

#if defined(OBJECTS_DESCR_INIT)
#define OBJ(name,desc)  name, desc
#define OBJECT(obj,bits,prp,sym,prob,dly,wt, \
               cost,sdam,ldam,oc1,oc2,nut,color,sn)  { obj }
#define MARKER(tag,sn) /*empty*/

#elif defined(OBJECTS_INIT)
/* notes: 'sub' was once a bitfield but got changed to separate schar when
   it was overloaded to hold negative weapon skill indices; the first zero
   is padding for oc_prediscovered which has variable init at run-time;
   the second zero is oc_spare1 for padding between oc_tough and oc_dir */
#define BITS(nmkn,mrg,uskn,ctnr,mgc,chrg,uniq,nwsh,big,tuf,dir,sub,mtrl) \
    nmkn,mrg,uskn,0,mgc,chrg,uniq,nwsh,big,tuf,0,dir,mtrl,sub /*cpp fodder*/
/* note: 0UL-1UL is a method of expressing the largest possible
   unsigned long value whilst working around a false-positive warning
   in Microsoft Visual C (which assumes that a negative number was
   intended despite the explicit U suffix) */
#define OBJECT(obj,bits,prp,sym,prob,dly,wt,        \
               cost,sdam,ldam,oc1,oc2,nut,color,sn) \
  { 0, 0, (char *) 0, bits, prp, sym, dly, color, prob, wt, \
    cost, sdam, ldam, oc1, oc2, nut, (0UL-1UL), 0, (0UL-1UL), 0 }
#define MARKER(tag,sn) /*empty*/

#elif defined(OBJECTS_ENUM)
#define OBJ(name,desc)
#define OBJECT(obj,bits,prp,sym,prob,dly,wt,        \
               cost,sdam,ldam,oc1,oc2,nut,color,sn) \
    sn
#define MARKER(tag,sn) tag = sn,

#elif defined(DUMP_ENUMS)
#define OBJ(name,desc)
#define OBJECT(obj,bits,prp,sym,prob,dly,wt,        \
               cost,sdam,ldam,oc1,oc2,nut,color,sn) \
  { sn, #sn }
#define MARKER(tag,sn) /*empty*/

#else
#error Unproductive inclusion of objects.h
#endif  /* OBJECTS_DESCR_INIT || OBJECTS_INIT || OBJECTS_ENUM */

#define GENERIC(desc, class, gen_enum) \
    OBJECT(OBJ("generic " desc, desc),                                  \
           BITS(0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, P_NONE, 0),            \
           0, class, 0, 0, 0, 0, 0, 0, 0, 0, 0, CLR_GRAY, gen_enum)

/* dummy object[0] -- description [2nd arg] *must* be NULL */
OBJECT(OBJ("strange object", NoDes),
       BITS(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, P_NONE, 0),
       0, ILLOBJ_CLASS, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, STRANGE_OBJECT),
/* slots [1] through [MAXOCLASSES-1] are indexed by class; some are
   used for display purposes, most aren't used; none are actual objects;
   note that 'real' strange object is in slot [0] but ILLOBJ_CLASS is 1
   so we add a dummy for it in slot [1] to simplify accessing the rest;
   there isn't any entry for RANDOM_CLASS (0) */
GENERIC("strange",    ILLOBJ_CLASS,  GENERIC_ILLOBJ),  /* [1] */
GENERIC("weapon",     WEAPON_CLASS,  GENERIC_WEAPON),  /* [2] */
GENERIC("armor",      ARMOR_CLASS,   GENERIC_ARMOR),   /* [3] */
GENERIC("ring",       RING_CLASS,    GENERIC_RING),    /* [4] */
GENERIC("amulet",     AMULET_CLASS,  GENERIC_AMULET),  /* [5] */
GENERIC("tool",       TOOL_CLASS,    GENERIC_TOOL),    /* [6] */
GENERIC("food",       FOOD_CLASS,    GENERIC_FOOD),    /* [7] */
GENERIC("potion",     POTION_CLASS,  GENERIC_POTION),  /* [8] */
GENERIC("scroll",     SCROLL_CLASS,  GENERIC_SCROLL),  /* [9] */
GENERIC("spellbook",  SPBOOK_CLASS,  GENERIC_SPBOOK),  /* [10] */
GENERIC("wand",       WAND_CLASS,    GENERIC_WAND),    /* [11] */
GENERIC("coin",       COIN_CLASS,    GENERIC_COIN),    /* [12] */
GENERIC("gem",        GEM_CLASS,     GENERIC_GEM),     /* [13] */
GENERIC("large rock", ROCK_CLASS,    GENERIC_ROCK),    /* [14] bldr+statue */
GENERIC("iron ball",  BALL_CLASS,    GENERIC_BALL),    /* [15] */
GENERIC("iron chain", CHAIN_CLASS,   GENERIC_CHAIN),   /* [16] */
GENERIC("venom",      VENOM_CLASS,   GENERIC_VENOM),   /* [17] */
#undef GENERIC
/* FIRST_OBJECT: it would be simpler just to use MARKER(FIRST_OBJECT,ARROW)
   below but that is vulnerable to neglecting to update the marker enum
   after inserting something in front of arrow */
MARKER(LAST_GENERIC, GENERIC_VENOM)
MARKER(FIRST_OBJECT, LAST_GENERIC + 1)
/* this definition of FIRST_OBJECT advances the default value for next enum;
   backtrack to fix that, otherwise ARROW and the rest would be off by 1 */
MARKER(OBJCLASS_HACK, FIRST_OBJECT - 1)

/* The object set below is EvilHack's (566 entries), bulk-converted
 * from the 3.6 tree's objects.c into 5.0's objects.h layout (entry
 * text verbatim except None -> NoDes and the appended onames enum
 * suffix); wrapper macros keep EvilHack signatures plus sn */
/* Note: for weapons that don't do an even die of damage (ex. 2-7 or 3-18)
   the extra damage is added on in weapon.c, not here! */
/* missiles; materiel reflects the arrowhead, not the shaft */

/* weapon strike mode overloads the oc_dir field */
#define P PIERCE
#define S SLASH
#define B WHACK

#define WEAPON(name,desc,kn,mg,mgc,bi,prob,wt,            \
               cost,sdam,ldam,hitbon,typ,sub,metal,color,sn) \
    OBJECT(OBJ(name,desc),                                           \
           BITS(kn, mg, 1, 0, mgc, 1, 0, 0, bi, 0, typ, sub, metal), \
           0, WEAPON_CLASS, prob, 0, wt,                             \
           cost, sdam, ldam, hitbon, 0, wt, color, sn)
#define PROJECTILE(name,desc,kn,prob,wt,                  \
                   cost,sdam,ldam,hitbon,metal,sub,color,sn) \
    OBJECT(OBJ(name,desc),                                          \
           BITS(kn, 1, 1, 0, 0, 1, 0, 0, 0, 0, PIERCE, sub, metal), \
           0, WEAPON_CLASS, prob, 0, wt,                            \
           cost, sdam, ldam, hitbon, 0, wt, color, sn)
#define BOW(name,desc,kn,bi,prob,wt,cost,hitbon,metal,sub,color,sn) \
    OBJECT(OBJ(name,desc),                                          \
           BITS(kn, 0, 1, 0, 0, 1, 0, 0, bi, 0, 0, sub, metal),     \
           0, WEAPON_CLASS, prob, 0, wt,                            \
           cost, 2, 2, hitbon, 0, wt, color, sn)
/* === PROJECTILE entries === */
PROJECTILE("arrow", NoDes,
           1, 67, 1, 2, 6, 6, 0,        IRON, -P_BOW, HI_IRON,
        ARROW),
PROJECTILE("elven arrow", "runed arrow",
           0, 20, 1, 2, 7, 6, 0,        WOOD, -P_BOW, HI_WOOD,
        ELVEN_ARROW),
PROJECTILE("orcish arrow", "crude arrow",
           0, 20, 1, 2, 5, 6, 0,        IRON, -P_BOW, CLR_BLACK,
        ORCISH_ARROW),
PROJECTILE("dark elven arrow", "etched arrow",
           0,  3, 1, 5, 7, 6, 0,        ADAMANTINE, -P_BOW, HI_ADAMANTINE,
        DARK_ELVEN_ARROW),
PROJECTILE("ya", "long arrow",
           0, 15, 1, 4, 7, 7, 1,        WOOD, -P_BOW, HI_WOOD,
        YA),
PROJECTILE("crossbow bolt", NoDes,
           1, 55, 1, 2, 4, 6, 0,        IRON, -P_CROSSBOW, HI_IRON,
        CROSSBOW_BOLT),
PROJECTILE("dark elven crossbow bolt", "etched crossbow bolt",
           0,  5, 1, 5, 4, 6, 0,        ADAMANTINE, -P_CROSSBOW, HI_ADAMANTINE,
        DARK_ELVEN_CROSSBOW_BOLT),
/* missiles that don't use a launcher */
/* === WEAPON entries === */
WEAPON("dart", NoDes,
       1, 1, 0, 0, 60,   1,   2,  3,  2, 0, P,   -P_DART, IRON, HI_IRON,
        DART),
WEAPON("shuriken", "throwing star",
       0, 1, 0, 0, 35,   1,   5,  8,  6, 2, P,   -P_SHURIKEN, IRON, HI_IRON,
        SHURIKEN),
WEAPON("boomerang", NoDes,
       1, 1, 0, 0, 15,   5,  20,  9,  9, 0, B,   -P_BOOMERANG, WOOD, HI_WOOD,
        BOOMERANG),
/* spears [note: javelin used to have a separate skill from spears,
   because the latter are primarily stabbing weapons rather than
   throwing ones; but for playability, they've been merged together
   under spear skill and spears can now be thrown like javelins] */
WEAPON("spear", NoDes,
       1, 1, 0, 0, 52,  30,   3,  6,  8, 0, P,   P_SPEAR, IRON, HI_IRON,
        SPEAR),
WEAPON("elven spear", "runed spear",
       0, 1, 0, 0, 10,  11,   3,  7,  8, 0, P,   P_SPEAR, WOOD, HI_WOOD,
        ELVEN_SPEAR),
WEAPON("orcish spear", "crude spear",
       0, 1, 0, 0, 13,  30,   3,  5,  8, 0, P,   P_SPEAR, IRON, CLR_BLACK,
        ORCISH_SPEAR),
WEAPON("dwarvish spear", "stout spear",
       0, 1, 0, 0, 12,  30,   3,  8,  8, 0, P,   P_SPEAR, IRON, HI_IRON,
        DWARVISH_SPEAR),
WEAPON("dark elven spear", "etched spear",
       0, 1, 0, 0,  5,  15,  10,  7,  8, 0, P,   P_SPEAR, ADAMANTINE, HI_ADAMANTINE,
        DARK_ELVEN_SPEAR),
WEAPON("javelin", "throwing spear",
       0, 1, 0, 0, 10,  20,   3,  6,  6, 0, P,   P_SPEAR, IRON, HI_IRON,
        JAVELIN),
/* Base weapon for the artifact weapon Xiuhcoatl */
WEAPON("atlatl", NoDes,
       1, 0, 0, 0,  0,  30,  10,  8, 12, 2, P,   P_SPEAR, WOOD, CLR_BLACK,
        ATLATL),
/* --------------------------------------------- */
/* spearish; doesn't stack, not intended to be thrown */
WEAPON("trident", NoDes,
       1, 0, 0, 0,  8,  20,   5,  6,  4, 0, P,   P_TRIDENT, IRON, HI_IRON,
        TRIDENT),
/* +1 small, +2d4 large */
/* blades; all stack */
WEAPON("dagger", NoDes,
       1, 1, 0, 0, 33,  10,   4,  4,  3, 2, P,   P_DAGGER, IRON, HI_IRON,
        DAGGER),
WEAPON("elven dagger", "runed dagger",
       0, 1, 0, 0, 10,   4,   4,  5,  3, 2, P,   P_DAGGER, WOOD, HI_WOOD,
        ELVEN_DAGGER),
WEAPON("orcish dagger", "crude dagger",
       0, 1, 0, 0, 12,  10,   4,  3,  3, 2, P,   P_DAGGER, IRON, CLR_BLACK,
        ORCISH_DAGGER),
WEAPON("dark elven dagger", "etched dagger",
       0, 1, 0, 0,  2,   8,  10,  5,  3, 2, P,   P_DAGGER, ADAMANTINE, HI_ADAMANTINE,
        DARK_ELVEN_DAGGER),
WEAPON("athame", NoDes,
       1, 1, 0, 0,  0,  10,   4,  4,  3, 2, S,   P_DAGGER, STEEL, HI_STEEL,
        ATHAME),
WEAPON("scalpel", NoDes,
       1, 1, 0, 0,  0,   5,   6,  3,  3, 2, S,   P_DAGGER, STEEL, HI_STEEL,
        SCALPEL),
WEAPON("knife", NoDes,
       1, 1, 0, 0, 20,   5,   4,  3,  2, 0, S, P_DAGGER, IRON, HI_IRON,
        KNIFE),
WEAPON("stiletto", NoDes,
       1, 1, 0, 0,  5,   5,   4,  3,  2, 0, P, P_DAGGER, IRON, HI_IRON,
        STILETTO),
/* 3.6: worm teeth and crysknives now stack;
   when a stack of teeth is enchanted at once, they fuse into one crysknife;
   when a stack of crysknives drops, the whole stack reverts to teeth */
WEAPON("worm tooth", NoDes,
       1, 1, 0, 0,  0,  20,   2,  2,  2, 0, P,   P_DAGGER, BONE, HI_BONE,
        WORM_TOOTH),
WEAPON("crysknife", NoDes,
       1, 1, 0, 0,  0,  20, 100, 10, 10, 3, P,   P_DAGGER, BONE, HI_BONE,
        CRYSKNIFE),
/* axes */
WEAPON("axe", NoDes,
       1, 0, 0, 0, 40,  60,   8,  6,  4, 0, S,   P_AXE, IRON, HI_IRON,
        AXE),
WEAPON("dwarvish bearded axe", "broad bearded axe",
       0, 0, 0, 0, 20,  70,  25,  8, 10, 0, S,   P_AXE, IRON, HI_IRON,
        DWARVISH_BEARDED_AXE),
WEAPON("battle-axe", "double-headed axe",       /* "double-bitted"? */
       0, 0, 0, 1, 10, 120,  40,  8,  6, 0, S,   P_AXE, IRON, HI_IRON,
        BATTLE_AXE),
/* swords */
WEAPON("short sword", NoDes,
       1, 0, 0, 0,  8,  30,  10,  6,  8, 0, P,   P_SHORT_SWORD, IRON, HI_IRON,
        SHORT_SWORD),
WEAPON("elven short sword", "runed short sword",
       0, 0, 0, 0,  2,  11,  10,  8,  8, 0, P,   P_SHORT_SWORD, WOOD, HI_WOOD,
        ELVEN_SHORT_SWORD),
WEAPON("orcish short sword", "crude short sword",
       0, 0, 0, 0,  3,  30,  10,  5,  8, 0, P,   P_SHORT_SWORD, IRON, CLR_BLACK,
        ORCISH_SHORT_SWORD),
WEAPON("dwarvish short sword", "broad short sword",
       0, 0, 0, 0,  2,  30,  10,  7,  8, 0, P,   P_SHORT_SWORD, IRON, HI_IRON,
        DWARVISH_SHORT_SWORD),
WEAPON("dark elven short sword", "etched short sword",
       0, 0, 0, 0,  2,  15,  20,  8,  8, 0, P,   P_SHORT_SWORD, ADAMANTINE, HI_ADAMANTINE,
        DARK_ELVEN_SHORT_SWORD),
WEAPON("scimitar", "curved sword",
       0, 0, 0, 0, 15,  40,  15,  8,  8, 0, S,   P_SABER, IRON, HI_IRON,
        SCIMITAR),
WEAPON("orcish scimitar", "crude curved sword",
       0, 0, 0, 0, 15,  40,  15,  6,  8, 0, S,   P_SABER, IRON, CLR_BLACK,
        ORCISH_SCIMITAR),
WEAPON("saber", NoDes,
       1, 0, 0, 0,  6,  40,  75,  8,  8, 0, S,   P_SABER, IRON, HI_IRON,
        SABER),
WEAPON("broadsword", NoDes,
       1, 0, 0, 0,  8,  70,  10,  4,  6, 0, S,   P_BROAD_SWORD, IRON, HI_IRON,
        BROADSWORD),
/* +d4 small, +1 large */
WEAPON("elven broadsword", "runed broadsword",
       0, 0, 0, 0,  4,  26,  10,  6,  6, 0, S,   P_BROAD_SWORD, WOOD, HI_WOOD,
        ELVEN_BROADSWORD),
/* +d4 small, +1 large */
WEAPON("dark elven broadsword", "etched broadsword",
       0, 0, 0, 0,  1,  45,  20,  6,  6, 0, S,   P_BROAD_SWORD, ADAMANTINE, HI_ADAMANTINE,
        DARK_ELVEN_BROADSWORD),
/* +d4 small, +1 large */
WEAPON("long sword", NoDes,
       1, 0, 0, 0, 50,  40,  15,  8, 12, 0, S,   P_LONG_SWORD, IRON, HI_IRON,
        LONG_SWORD),
WEAPON("elven long sword", "runed long sword",
       0, 0, 0, 0, 10,  15,  40, 10, 12, 0, S,   P_LONG_SWORD, WOOD, HI_WOOD,
        ELVEN_LONG_SWORD),
WEAPON("orcish long sword", "crude long sword",
       0, 0, 0, 0, 10,  40,  12,  8, 10, 0, S,   P_LONG_SWORD, IRON, CLR_BLACK,
        ORCISH_LONG_SWORD),
WEAPON("dark elven long sword", "etched long sword",
       0, 0, 0, 0,  2,  25,  80, 10, 12, 0, S,   P_LONG_SWORD, ADAMANTINE, HI_ADAMANTINE,
        DARK_ELVEN_LONG_SWORD),
WEAPON("two-handed sword", NoDes,
       1, 0, 0, 1, 22, 150,  50, 12,  6, 0, S,   P_TWO_HANDED_SWORD, IRON, HI_IRON,
        TWO_HANDED_SWORD),
/* +2d6 large */
WEAPON("katana", "samurai sword",
       0, 0, 0, 0,  4,  40,  80, 10, 12, 1, S,   P_LONG_SWORD, STEEL, HI_STEEL,
        KATANA),
/* special swords set up for artifacts */
WEAPON("tsurugi", "long samurai sword",
       0, 0, 0, 1,  0,  60, 500, 16,  8, 2, S,   P_TWO_HANDED_SWORD, STEEL, HI_STEEL,
        TSURUGI),
/* +2d6 large */
WEAPON("runesword", "runed broadsword",
       0, 0, 0, 0,  0,  40, 300,  4,  6, 0, S,   P_BROAD_SWORD, IRON, CLR_BLACK,
        RUNESWORD),
/* polearms */
/* spear-type */
WEAPON("partisan", "vulgar polearm",
       0, 0, 0, 1,  5,  80,  10,  6,  6, 0, P,   P_POLEARMS, IRON, HI_IRON,
        PARTISAN),
/* +1 large */
WEAPON("ranseur", "hilted polearm",
       0, 0, 0, 1,  5,  50,   6,  4,  4, 0, P,   P_POLEARMS, IRON, HI_IRON,
        RANSEUR),
/* +d4 both */
WEAPON("spetum", "forked polearm",
       0, 0, 0, 1,  5,  50,   5,  6,  6, 0, P,   P_POLEARMS, IRON, HI_IRON,
        SPETUM),
/* +1 small, +d6 large */
WEAPON("glaive", "single-edged polearm",
       0, 0, 0, 1,  8,  75,   6,  6, 10, 0, S,   P_POLEARMS, IRON, HI_IRON,
        GLAIVE),
WEAPON("lance", NoDes,
       1, 0, 0, 0,  4, 180,  10,  6,  8, 0, P,   P_LANCE, STEEL, HI_STEEL,
        LANCE),
/* +2d10 when jousting with lance as primary weapon */
/* axe-type */
WEAPON("halberd", "angled poleaxe",
       0, 0, 0, 1,  8, 150,  10, 10,  6, 0, S, P_POLEARMS, IRON, HI_IRON,
        HALBERD),
/* +1d6 large */
WEAPON("bardiche", "long poleaxe",
       0, 0, 0, 1,  4, 120,   7,  4,  4, 0, S,   P_POLEARMS, IRON, HI_IRON,
        BARDICHE),
/* +1d4 small, +2d4 large */
WEAPON("voulge", "pole cleaver",
       0, 0, 0, 1,  4, 125,   5,  4,  4, 0, S,   P_POLEARMS, IRON, HI_IRON,
        VOULGE),
/* +d4 both */
WEAPON("dwarvish mattock", "broad pick",
       0, 0, 0, 1, 13, 120,  50, 12,  8, -1, B,  P_PICK_AXE, IRON, HI_IRON,
        DWARVISH_MATTOCK),
/* curved/hooked */
WEAPON("fauchard", "pole sickle",
       0, 0, 0, 1,  6,  60,   5,  6,  8, 0, S, P_POLEARMS, IRON, HI_IRON,
        FAUCHARD),
WEAPON("guisarme", "pruning hook",
       0, 0, 0, 1,  6,  80,   5,  4,  8, 0, S,   P_POLEARMS, IRON, HI_IRON,
        GUISARME),
/* +1d4 small */
WEAPON("bill-guisarme", "hooked polearm",
       0, 0, 0, 1,  4, 120,   7,  4, 10, 0, S, P_POLEARMS, IRON, HI_IRON,
        BILL_GUISARME),
/* +1d4 small */
/* other */
WEAPON("lucern hammer", "pronged polearm",
       0, 0, 0, 1,  5, 150,   7,  4,  6, 0, B, P_POLEARMS, IRON, HI_IRON,
        LUCERN_HAMMER),
/* +1d4 small */
WEAPON("bec de corbin", "beaked polearm",
       0, 0, 0, 1,  4, 100,   8,  8,  6, 0, P, P_POLEARMS, IRON, HI_IRON,
        BEC_DE_CORBIN),
/* bludgeons */
WEAPON("mace", NoDes,
       1, 0, 0, 0, 30,  30,   5,  6,  6, 0, B,   P_MACE, IRON, HI_IRON,
        MACE),
WEAPON("dark elven mace", "etched mace",
       0, 0, 0, 0,  3,  20,  10,  6,  8, 0, B,   P_MACE, ADAMANTINE, HI_ADAMANTINE,
        DARK_ELVEN_MACE),
WEAPON("heavy mace", NoDes,
       1, 0, 0, 0, 15,  50,  10, 10, 10, 0, B,   P_MACE, IRON, HI_IRON,
        HEAVY_MACE),
WEAPON("dark elven heavy mace", "etched heavy mace",
       0, 0, 0, 0,  2,  35,  20, 10, 12, 0, B,   P_MACE, ADAMANTINE, HI_ADAMANTINE,
        DARK_ELVEN_HEAVY_MACE),
/* +1 small */
/* placeholder for certain special weapons; does not spawn randomly */
WEAPON("rod", "ornate mace",
       0, 0, 0, 0,  0,  40, 250,  6,  8, 0, B,   P_MACE, GEMSTONE, HI_GEMSTONE,
        ROD),
WEAPON("morning star", NoDes,
       1, 0, 0, 0, 12, 120,  10,  4,  6, 0, B,   P_MACE, IRON, HI_IRON,
        MORNING_STAR),
/* +d4 small, +1 large */
WEAPON("orcish morning star", "crude morning star",
       0, 0, 0, 0,  8, 120,  10,  4,  6, 0, B,   P_MACE, IRON, CLR_BLACK,
        ORCISH_MORNING_STAR),
/* +d4 small, +1 large */
WEAPON("war hammer", NoDes,
       1, 0, 0, 0, 15,  50,   5,  4,  4, 0, B,   P_HAMMER, IRON, HI_IRON,
        WAR_HAMMER),
/* +1 small */
WEAPON("heavy war hammer", NoDes,
       1, 0, 0, 0, 10,  60,  10,  8, 10, 0, B,   P_HAMMER, IRON, HI_IRON,
        HEAVY_WAR_HAMMER),
WEAPON("club", NoDes,
       1, 0, 0, 0, 12,  30,   3,  6,  3, 0, B,   P_CLUB, WOOD, HI_WOOD,
        CLUB),
WEAPON("rubber hose", NoDes,
       1, 0, 0, 0,  0,  20,   3,  4,  3, 0, B,   P_WHIP, PLASTIC, HI_PLASTIC,
        RUBBER_HOSE),
WEAPON("quarterstaff", "staff",
       0, 0, 0, 1, 11,  40,   5,  6,  6, 0, B,   P_QUARTERSTAFF, WOOD, HI_WOOD,
        QUARTERSTAFF),
WEAPON("staff of divination", "wormwood staff",
       0, 0, 1, 1,  5,  40, 400,  6,  6, 0, B,   P_QUARTERSTAFF, WOOD, HI_WOOD,
        STAFF_OF_DIVINATION),
WEAPON("staff of healing", "twisted staff",
       0, 0, 1, 1,  5,  40, 400,  6,  6, 0, B,   P_QUARTERSTAFF, WOOD, HI_WOOD,
        STAFF_OF_HEALING),
WEAPON("staff of holiness", "bone-carved staff",
       0, 0, 1, 1,  5,  40, 400,  6,  6, 0, B,   P_QUARTERSTAFF, BONE, HI_BONE,
        STAFF_OF_HOLINESS),
WEAPON("staff of matter", "etched staff",
       0, 0, 1, 1,  5,  30, 450,  6,  6, 0, B,   P_QUARTERSTAFF, ADAMANTINE, HI_ADAMANTINE,
        STAFF_OF_MATTER),
WEAPON("staff of escape", "darkwood staff",
       0, 0, 1, 1,  5,  40, 400,  6,  6, 0, B,   P_QUARTERSTAFF, WOOD, CLR_BLACK,
        STAFF_OF_ESCAPE),
WEAPON("staff of war", "ironshod staff",
       0, 0, 1, 1,  5,  50, 425,  6,  6, 2, B,   P_QUARTERSTAFF, IRON, HI_IRON,
        STAFF_OF_WAR),
WEAPON("staff of evocation", "gnarled staff",
       0, 0, 1, 1,  5,  40, 400,  6,  6, 1, B,   P_QUARTERSTAFF, WOOD, HI_WOOD,
        STAFF_OF_EVOCATION),
/* template for the Staff of the Archmagi; no shuffled description, so
   born name-known (o_init enforces that for desc-less objects) */
WEAPON("ashwood staff", NoDes,
       1, 0, 1, 1,  0,  40, 600,  8, 10, 0, B,   P_QUARTERSTAFF, WOOD, CLR_WHITE,
        ASHWOOD_STAFF),
/* two-piece */
WEAPON("aklys", "thonged club",
       0, 0, 0, 0,  8,  15,   4,  6,  3, 0, B,   P_CLUB, IRON, HI_IRON,
        AKLYS),
WEAPON("flail", NoDes,
       1, 0, 0, 0, 40,  15,   4,  6,  4, 0, B,   P_FLAIL, IRON, HI_IRON,
        FLAIL),
/* +1 small, +1d4 large */
/* template for Yeenoghu's artifact weapon; does not spawn randomly */
WEAPON("triple-headed flail", NoDes,
       1, 0, 0, 1,  0, 150,  30, 12,  6, 3, B,   P_FLAIL, BONE, HI_BONE,
        TRIPLE_HEADED_FLAIL),
/* +d4 small, +3d6 large */
/* misc */
WEAPON("bullwhip", NoDes,
       1, 0, 0, 0,  2,  20,   4,  2,  1, 0, 0,   P_WHIP, LEATHER, HI_LEATHER,
        BULLWHIP),
/* bows */
/* === BOW entries === */
BOW("bow", NoDes,                    1, 1, 24, 30,  60, 0, WOOD, P_BOW, HI_WOOD,
        BOW),
BOW("elven bow", "runed bow",       0, 1, 12, 30,  60, 0, WOOD, P_BOW, HI_WOOD,
        ELVEN_BOW),
BOW("orcish bow", "crude bow",      0, 1, 12, 30,  60, 0, WOOD, P_BOW, CLR_BLACK,
        ORCISH_BOW),
BOW("dark elven bow", "etched bow", 0, 1,  3, 40, 100, 0, ADAMANTINE, P_BOW, HI_ADAMANTINE,
        DARK_ELVEN_BOW),
BOW("yumi", "long bow",             0, 1,  0, 30,  60, 0, WOOD, P_BOW, HI_WOOD,
        YUMI),
BOW("sling", NoDes,                  1, 0, 40,  3,  20, 0, LEATHER, P_SLING, HI_LEATHER,
        SLING),
BOW("dark elven hand crossbow", "etched crossbow",
    0, 0,  5, 25,  80, 0, ADAMANTINE, P_CROSSBOW, HI_ADAMANTINE,
        DARK_ELVEN_HAND_CROSSBOW),
BOW("crossbow", NoDes,               1, 1, 45, 50,  40, 0, WOOD, P_CROSSBOW, HI_WOOD,
        CROSSBOW),
/* IRON denotes ferrous metals.
         * STEEL denotes stainless steel.
         * Only IRON weapons and armor can rust.
         * Only COPPER and BRONZE (including brass) corrodes.
         * Some creatures are vulnerable to SILVER.
         */
/* helmets */

#undef P
#undef S
#undef B

#undef WEAPON
#undef PROJECTILE
#undef BOW

#define ARMOR(name,desc,kn,mgc,blk,power,prob,delay,wt,  \
              cost,ac,can,sub,metal,c,sn)                \
    OBJECT(OBJ(name, desc),                                         \
           BITS(kn, 0, 1, 0, mgc, 1, 0, 0, blk, 0, 0, sub, metal),  \
           power, ARMOR_CLASS, prob, delay, wt,                     \
           cost, 0, 0, 10 - ac, can, wt, c, sn)
#define HELM(name,desc,kn,mgc,power,prob,delay,wt,cost,ac,can,metal,c,sn) \
    ARMOR(name, desc, kn, mgc, 0, power, prob, delay, wt,  \
          cost, ac, can, ARM_HELM, metal, c, sn)
#define CLOAK(name,desc,kn,mgc,power,prob,delay,wt,cost,ac,can,metal,c,sn) \
    ARMOR(name, desc, kn, mgc, 0, power, prob, delay, wt,  \
          cost, ac, can, ARM_CLOAK, metal, c, sn)
#define SHIELD(name,desc,kn,mgc,blk,power,prob,delay,wt,cost,ac,can,metal,c,sn) \
    ARMOR(name, desc, kn, mgc, blk, power, prob, delay, wt, \
          cost, ac, can, ARM_SHIELD, metal, c, sn)
#define GLOVES(name,desc,kn,mgc,power,prob,delay,wt,cost,ac,can,metal,c,sn) \
    ARMOR(name, desc, kn, mgc, 0, power, prob, delay, wt,  \
          cost, ac, can, ARM_GLOVES, metal, c, sn)
#define BOOTS(name,desc,kn,mgc,power,prob,delay,wt,cost,ac,can,metal,c,sn) \
    ARMOR(name, desc, kn, mgc, 0, power, prob, delay, wt,  \
          cost, ac, can, ARM_BOOTS, metal, c, sn)
/* === HELM entries === */
HELM("dunce cap", "conical hat",
     0, 1,           0,  3, 1,  4,  1,  10, 0, CLOTH, CLR_BLUE,
        DUNCE_CAP),
HELM("cornuthaum", "conical hat",
     0, 1, CLAIRVOYANT,  3, 1,  4, 80,  10, 1, CLOTH, CLR_BLUE,
        CORNUTHAUM),
/* name coined by devteam; confers clairvoyance for wizards,
           blocks clairvoyance if worn by role other than wizard */
HELM("fedora", NoDes,
     1, 0,           0,  0, 0,  3,   1, 10, 0, CLOTH, HI_CLOTH,
        FEDORA),
HELM("toque", NoDes,
     1, 0,           0,  0, 0,  3,   1, 10, 0, CLOTH, CLR_ORANGE,
        TOQUE),
HELM("elven helm", "hat",
     0, 0,           0,  5, 1,  3,   8,  9, 0, LEATHER, HI_LEATHER,
        ELVEN_HELM),
HELM("dark elven helm", "spider silk hat",
     0, 0,           0,  1, 1,  2,  16,  8, 0, SPIDER_SILK, HI_SPIDER_SILK,
        DARK_ELVEN_HELM),
HELM("orcish helm", "skull cap",
     0, 0,           0,  6, 1, 30,  10,  9, 0, IRON, CLR_BLACK,
        ORCISH_HELM),
HELM("dwarvish helm", "hard hat",
     0, 0,           0,  6, 1, 40,  20,  8, 0, IRON, HI_IRON,
        DWARVISH_HELM),
HELM("dented pot", NoDes,
     1, 0,           0,  2, 0, 10,   8,  9, 0, IRON, CLR_BLACK,
        DENTED_POT),
HELM("helm of brilliance", "crystal helmet",
     0, 1,           0,  3, 1, 40, 100,  9, 0, GEMSTONE, CLR_WHITE,
        HELM_OF_BRILLIANCE),
/* with shuffled appearances... */
HELM("helmet", "plumed helmet",
     0, 0,           0, 10, 1, 30,  10,  9, 0, IRON, HI_IRON,
        HELMET),
HELM("helm of caution", "etched helmet",
     0, 1,     WARNING,  3, 1, 50,  50,  9, 0, IRON, CLR_GREEN,
        HELM_OF_CAUTION),
HELM("helm of opposite alignment", "crested helmet",
     0, 1,           0,  5, 1, 50,  50,  9, 0, IRON, HI_IRON,
        HELM_OF_OPPOSITE_ALIGNMENT),
HELM("helm of speed", "winged helmet",
     0, 1,        FAST,  2, 1, 50,  50,  9, 0, STEEL, HI_GOLD,
        HELM_OF_SPEED),
HELM("helm of telepathy", "visored helmet",
     0, 1,     TELEPAT,  2, 1, 50,  50,  9, 0, IRON, HI_IRON,
        HELM_OF_TELEPATHY),
HELM("helm of telekinesis", "spiked helmet",
     0, 1, TELEKINESIS,  1, 1, 30, 100,  9, 0, ADAMANTINE, HI_ADAMANTINE,
        HELM_OF_TELEKINESIS),
/* For now, only dragons leave these. */
/* 3.4.1: dragon scales left classified as "non-magic"; they confer magical
   properties but are produced "naturally"; affects use as polypile fodder */

#define DRGN_SCALES(name,mgc,power,cost,color,sn)  \
    ARMOR(name, NoDes, 1, mgc, 1, power, 0, 5, 40,  \
          cost, 7, 0, ARM_CLOAK, DRAGON_HIDE, color, sn)
/* === DRGN_SCALES entries === */
DRGN_SCALES("gray dragon scales",       0, ANTIMAGIC,         700, CLR_GRAY,
        GRAY_DRAGON_SCALES),
DRGN_SCALES("silver dragon scales",     0, REFLECTING,        700, DRAGON_SILVER,
        SILVER_DRAGON_SCALES),
DRGN_SCALES("shimmering dragon scales", 0, DISPLACED,         700, CLR_CYAN,
        SHIMMERING_DRAGON_SCALES),
DRGN_SCALES("red dragon scales",        0, FIRE_RES,          500, CLR_RED,
        RED_DRAGON_SCALES),
DRGN_SCALES("white dragon scales",      0, COLD_RES,          500, CLR_WHITE,
        WHITE_DRAGON_SCALES),
DRGN_SCALES("orange dragon scales",     0, SLEEP_RES,         500, CLR_ORANGE,
        ORANGE_DRAGON_SCALES),
DRGN_SCALES("black dragon scales",      0, DISINT_RES,        700, CLR_BLACK,
        BLACK_DRAGON_SCALES),
DRGN_SCALES("blue dragon scales",       0, SHOCK_RES,         500, CLR_BLUE,
        BLUE_DRAGON_SCALES),
DRGN_SCALES("green dragon scales",      0, POISON_RES,        500, CLR_GREEN,
        GREEN_DRAGON_SCALES),
DRGN_SCALES("gold dragon scales",       0, 0,                 500, HI_GOLD,
        GOLD_DRAGON_SCALES),
DRGN_SCALES("sea dragon scales",        0, MAGICAL_BREATHING, 500, HI_ZAP,
        SEA_DRAGON_SCALES),
DRGN_SCALES("yellow dragon scales",     0, ACID_RES,          500, CLR_YELLOW,
        YELLOW_DRAGON_SCALES),
DRGN_SCALES("shadow dragon scales",     0, ULTRAVISION,       700, CLR_BLACK,
        SHADOW_DRAGON_SCALES),
DRGN_SCALES("celestial dragon scales",  0, FLYING,           1000, CLR_BRIGHT_MAGENTA,
        CELESTIAL_DRAGON_SCALES),
DRGN_SCALES("chromatic dragon scales",  0, ANTIMAGIC,        1500, CLR_MAGENTA,
        CHROMATIC_DRAGON_SCALES),
/* other suits */

#undef DRGN_SCALES
/* === ARMOR entries === */
ARMOR("plate mail", NoDes,
      1, 0, 1,  0, 44, 5, 350, 600,  3, 2,  ARM_SUIT, IRON, HI_IRON,
        PLATE_MAIL),
ARMOR("crystal plate mail", NoDes,
      1, 0, 1,  0,  4, 5, 150, 5000, 3, 2,  ARM_SUIT, GEMSTONE, CLR_WHITE,
        CRYSTAL_PLATE_MAIL),
ARMOR("splint mail", NoDes,
      1, 0, 1,  0, 62, 5, 300,  80,  4, 1,  ARM_SUIT, IRON, HI_IRON,
        SPLINT_MAIL),
/* special suit of armor for giant samurai only (no random spawn) */
ARMOR("large splint mail", NoDes,
      1, 0, 1,  0,  0, 5, 500, 150,  4, 1,  ARM_SUIT, IRON, HI_IRON,
        LARGE_SPLINT_MAIL),
ARMOR("banded mail", NoDes,
      1, 0, 1,  0, 72, 5, 250,  90,  4, 1,  ARM_SUIT, IRON, HI_IRON,
        BANDED_MAIL),
ARMOR("chain mail", NoDes,
      1, 0, 0,  0, 75, 5, 200,  75,  5, 1,  ARM_SUIT, IRON, HI_IRON,
        CHAIN_MAIL),
ARMOR("dwarvish chain mail", NoDes,
      1, 0, 0,  0, 12, 1, 200,  75,  4, 2,  ARM_SUIT, IRON, HI_IRON,
        DWARVISH_CHAIN_MAIL),
ARMOR("elven chain mail", NoDes,
      1, 0, 0,  0, 14, 1, 200,  75,  5, 2,  ARM_SUIT, COPPER, HI_COPPER,
        ELVEN_CHAIN_MAIL),
ARMOR("dark elven chain mail", NoDes,
      1, 0, 0,  0,  1, 1, 200, 150,  4, 2,  ARM_SUIT, ADAMANTINE, HI_ADAMANTINE,
        DARK_ELVEN_CHAIN_MAIL),
ARMOR("orcish chain mail", "crude chain mail",
      0, 0, 0,  0, 20, 5, 200,  75,  6, 1,  ARM_SUIT, IRON, CLR_BLACK,
        ORCISH_CHAIN_MAIL),
ARMOR("scale mail", NoDes,
      1, 0, 0,  0, 72, 5, 175,  45,  6, 1,  ARM_SUIT, IRON, HI_IRON,
        SCALE_MAIL),
ARMOR("studded armor", NoDes,
      1, 0, 0,  0, 75, 3, 125,  15,  7, 1,  ARM_SUIT, LEATHER, HI_LEATHER,
        STUDDED_ARMOR),
ARMOR("ring mail", NoDes,
      1, 0, 0,  0, 72, 5, 150, 100,  7, 1,  ARM_SUIT, IRON, HI_IRON,
        RING_MAIL),
ARMOR("orcish ring mail", "crude ring mail",
      0, 0, 0,  0, 20, 5, 150,  80,  8, 1,  ARM_SUIT, IRON, CLR_BLACK,
        ORCISH_RING_MAIL),
ARMOR("armor", NoDes,
      1, 0, 0,  0, 82, 3,  75,   5,  8, 1,  ARM_SUIT, LEATHER, HI_LEATHER,
        ARMOR),
ARMOR("dark elven tunic", "spider silk tunic",
      0, 0, 0,  0,  1, 1,  10,  15,  7, 2,  ARM_SUIT, SPIDER_SILK, HI_SPIDER_SILK,
        DARK_ELVEN_TUNIC),
ARMOR("jacket", NoDes,
      1, 0, 0,  0, 12, 0,  30,  10,  9, 0,  ARM_SUIT, LEATHER, CLR_BLACK,
        JACKET),
/* shirts */
ARMOR("Hawaiian shirt", NoDes,
      1, 0, 0,  0, 10, 0,   5,   3, 10, 0,  ARM_SHIRT, CLOTH, CLR_MAGENTA,
        HAWAIIAN_SHIRT),
ARMOR("striped shirt", NoDes,
      1, 0, 0,  0,  0, 0,   5,   2, 10, 0,  ARM_SHIRT, CLOTH, HI_CLOTH,
        STRIPED_SHIRT),
ARMOR("T-shirt", NoDes,
      1, 0, 0,  0,  2, 0,   5,   2, 10, 0,  ARM_SHIRT, CLOTH, CLR_WHITE,
        T_SHIRT),
/* cloaks */
/* === CLOAK entries === */
CLOAK("mummy wrapping", NoDes,
      1, 0,          0,  0, 0,  3,   2, 10, 1,  CLOTH, HI_CLOTH,
        MUMMY_WRAPPING),
/* worn mummy wrapping blocks invisibility */
CLOAK("elven cloak", "faded pall",
      0, 1,    STEALTH,  7, 0, 10,  60,  9, 1,  CLOTH, CLR_BLACK,
        ELVEN_CLOAK),
CLOAK("dark elven cloak", "spider silk pall",
      0, 1,    STEALTH,  1, 0, 10, 120,  8, 2,  SPIDER_SILK, HI_SPIDER_SILK,
        DARK_ELVEN_CLOAK),
CLOAK("orcish cloak", "coarse mantelet",
      0, 0,          0,  8, 0, 10,  40, 10, 1,  CLOTH, CLR_BLACK,
        ORCISH_CLOAK),
CLOAK("dwarvish cloak", "hooded cloak",
      0, 0,          0,  8, 0, 10,  50, 10, 1,  CLOTH, HI_CLOTH,
        DWARVISH_CLOAK),
CLOAK("oilskin cloak", "slippery cloak",
      0, 0,          0,  8, 0, 10,  50,  9, 2,  CLOTH, HI_CLOTH,
        OILSKIN_CLOAK),
CLOAK("robe", NoDes,
      1, 1,          0,  3, 0, 15,  50,  8, 2,  CLOTH, CLR_RED,
        ROBE),
/* robe was adopted from slash'em, where it's worn as a suit
           rather than as a cloak and there are several variations */
CLOAK("alchemy smock", "apron",
      0, 1, POISON_RES,  9, 0, 10,  50,  9, 1,  CLOTH, CLR_WHITE,
        ALCHEMY_SMOCK),
CLOAK("cloak", NoDes,
      1, 0,          0,  8, 0, 15,  40,  9, 1,  LEATHER, HI_LEATHER,
        CLOAK),
/* with shuffled appearances... */
CLOAK("cloak of protection", "tattered cape",
      0, 1, PROTECTION,  9, 0, 10,  50,  7, 3,  CLOTH, HI_CLOTH,
        CLOAK_OF_PROTECTION),
CLOAK("cloak of invisibility", "opera cloak",
      0, 1,      INVIS, 10, 0, 10,  60,  9, 1,  CLOTH, CLR_BRIGHT_MAGENTA,
        CLOAK_OF_INVISIBILITY),
CLOAK("cloak of magic resistance", "ornamental cope",
      0, 1,  ANTIMAGIC,  1, 0, 10,  60,  9, 1,  CLOTH, CLR_WHITE,
        CLOAK_OF_MAGIC_RESISTANCE),
/*  'cope' is not a spelling mistake... leave it be */
CLOAK("cloak of displacement", "dusty cloak",
      0, 1,  DISPLACED, 10, 0, 10,  50,  9, 1,  CLOTH, HI_CLOTH,
        CLOAK_OF_DISPLACEMENT),
/* shields */
/* === SHIELD entries === */
SHIELD("small shield", NoDes,
       1, 0, 0,           0, 4, 0,  30,   3, 9, 0,  WOOD, HI_WOOD,
        SMALL_SHIELD),
SHIELD("shield of drain resistance", "wooden shield",
       0, 1, 0, DRAIN_RES, 12, 0, 30, 50, 9, 0, WOOD, HI_WOOD,
                                                  SHIELD_OF_DRAIN_RESISTANCE),
SHIELD("shield of shock resistance", "wooden shield",
       0, 1, 0, SHOCK_RES, 12, 0, 30, 50, 9, 0, WOOD, HI_WOOD,
                                                  SHIELD_OF_SHOCK_RESISTANCE),
SHIELD("elven shield", "blue and green shield",
       0, 0, 0,           0, 2, 0,  40,   7, 8, 0,  WOOD, CLR_GREEN,
        ELVEN_SHIELD),
SHIELD("Uruk-hai shield", "white-handed shield",
       0, 0, 0,           0, 2, 0,  50,   7, 9, 0,  IRON, HI_IRON,
        URUK_HAI_SHIELD),
SHIELD("orcish shield", "red-eyed shield",
       0, 0, 0,           0, 2, 0,  50,   7, 9, 0,  IRON, CLR_RED,
        ORCISH_SHIELD),
SHIELD("large shield", NoDes,
       1, 0, 1,           0, 4, 0,  75,  10, 8, 0,  IRON, HI_IRON,
        LARGE_SHIELD),
SHIELD("dwarvish roundshield", "large round shield",
       0, 0, 0,           0, 2, 0,  75,  10, 8, 0,  IRON, HI_IRON,
        DWARVISH_ROUNDSHIELD),
SHIELD("shield of reflection", "polished shield",
       0, 1, 0,  REFLECTING, 3, 0,  50,  50, 8, 0,  SILVER, HI_SILVER,
        SHIELD_OF_REFLECTION),
SHIELD("shield of light", "shiny shield",
       0, 1, 0,           0, 3, 0,  60, 400, 8, 0,  GOLD, HI_GOLD,
        SHIELD_OF_LIGHT),
SHIELD("shield of mobility", "slippery shield",
       0, 1, 0, FREE_ACTION, 3, 0,  50, 300, 8, 0,  STEEL, HI_STEEL,
        SHIELD_OF_MOBILITY),
SHIELD("bracers", NoDes,
       1, 0, 0,           0, 4, 0,  10,  10, 9, 0,  LEATHER, HI_LEATHER,
        BRACERS),
SHIELD("dark elven bracers", "etched bracers",
       0, 0, 0,           0, 1, 0,  20,  30, 8, 0,  ADAMANTINE, HI_ADAMANTINE,
        DARK_ELVEN_BRACERS),
SHIELD("runed bracers", NoDes,
       1, 1, 0,           0, 0, 0,  15, 100, 8, 0,  WOOD, CLR_BRIGHT_GREEN,
        RUNED_BRACERS),
/* gloves */
/* These have their color but not material shuffled, so the IRON must
 * stay CLR_BROWN (== HI_LEATHER) even though it's normally either
 * HI_IRON or CLR_BLACK.  All have shuffled descriptions.
 */
/* === GLOVES entries === */
GLOVES("gloves", "old gloves",
       0, 0,          0, 16, 1, 10,   8, 9, 0,  LEATHER, HI_LEATHER,
        GLOVES),
GLOVES("dark elven gloves", "archer gloves",
       0, 0,          0,  1, 1,  3,  20, 8, 0,  SPIDER_SILK, HI_SPIDER_SILK,
        DARK_ELVEN_GLOVES),
GLOVES("gauntlets", "falconry gloves",
       0, 0,          0, 12, 1, 30,  50, 9, 0,  IRON, CLR_BROWN,
        GAUNTLETS),
GLOVES("gauntlets of power", "riding gloves",
       0, 1,          0,  8, 1, 30,  50, 9, 0,  IRON, CLR_BROWN,
        GAUNTLETS_OF_POWER),
GLOVES("gauntlets of protection", "boxing gloves",
       0, 1, PROTECTION,  4, 1, 10,  60, 8, 3,  CLOTH, CLR_BROWN,
        GAUNTLETS_OF_PROTECTION),
GLOVES("gauntlets of dexterity", "fencing gloves",
       0, 1,          0,  8, 1, 10,  50, 9, 0,  LEATHER, HI_LEATHER,
        GAUNTLETS_OF_DEXTERITY),
GLOVES("gauntlets of fumbling", "padded gloves",
       0, 1,   FUMBLING,  8, 1, 10,  50, 9, 0,  LEATHER, HI_LEATHER,
        GAUNTLETS_OF_FUMBLING),
GLOVES("mummified hand", NoDes,
       1, 0,          0,  0, 1,  5, 100, 9, 0,  FLESH, CLR_BLACK,
        MUMMIFIED_HAND),
/* boots */
/* === BOOTS entries === */
BOOTS("low boots", "walking shoes",
      0, 0,          0, 25, 2, 10,  8, 9, 0, LEATHER, HI_LEATHER,
        LOW_BOOTS),
BOOTS("dwarvish boots", "hard shoes",
      0, 0,          0,  7, 2, 50, 16, 8, 0, IRON, HI_IRON,
        DWARVISH_BOOTS),
BOOTS("orcish boots", "crude shoes",
      0, 0,          0,  8, 2, 50, 16, 8, 0, IRON, CLR_BLACK,
        ORCISH_BOOTS),
BOOTS("high boots", "jackboots",
      0, 0,          0, 14, 2, 20, 12, 8, 0, LEATHER, HI_LEATHER,
        HIGH_BOOTS),
/* with shuffled appearances... */
BOOTS("speed boots", "combat boots",
      0, 1,       FAST,  8, 2, 20, 50, 9, 0, LEATHER, HI_LEATHER,
        SPEED_BOOTS),
BOOTS("water walking boots", "jungle boots",
      0, 1,   WWALKING, 12, 2, 15, 50, 9, 0, LEATHER, HI_LEATHER,
        WATER_WALKING_BOOTS),
BOOTS("jumping boots", "hiking boots",
      0, 1,    JUMPING, 12, 2, 20, 50, 9, 0, LEATHER, HI_LEATHER,
        JUMPING_BOOTS),
BOOTS("elven boots", "mud boots",
      0, 1,    STEALTH, 12, 2, 15,  8, 9, 0, LEATHER, HI_LEATHER,
        ELVEN_BOOTS),
BOOTS("dark elven boots", "logger boots",
      0, 1,    STEALTH,  1, 2, 15, 16, 8, 0, SPIDER_SILK, HI_SPIDER_SILK,
        DARK_ELVEN_BOOTS),
BOOTS("kicking boots", "buckled boots",
      0, 1,          0, 12, 2, 50,  8, 9, 0, IRON, CLR_BROWN,
        KICKING_BOOTS),
/* CLR_BROWN for same reason as gauntlets of power */
BOOTS("levitation boots", "snow boots",
      0, 1, LEVITATION, 12, 2, 15, 30, 9, 0, LEATHER, HI_LEATHER,
        LEVITATION_BOOTS),
BOOTS("fumble boots", "riding boots",
      0, 1,   FUMBLING, 12, 2, 20, 30, 9, 0, LEATHER, HI_LEATHER,
        FUMBLE_BOOTS),
/* rings ... */
/* note that prob = 1 for all normal rings */

#undef HELM
#undef CLOAK
#undef SHIELD
#undef GLOVES
#undef BOOTS
#undef ARMOR

#define RING(name,stone,power,prob,cost,mgc,spec,mohs,metal,color,sn) \
    OBJECT(OBJ(name, stone),                                           \
           BITS(0, 0, spec, 0, mgc, spec, 0, 0, 0,                     \
                HARDGEM(mohs), 0, P_NONE, metal),                      \
           power, RING_CLASS, prob, 0, 3, cost, 0, 0, 0, 0, 15, color, sn)
/* === RING entries === */
RING("adornment", "wooden",
     ADORNED,                  1, 100, 1, 1, 2, WOOD, HI_WOOD,
        RIN_ADORNMENT),
RING("gain strength", "granite",
     0,                        1, 150, 1, 1, 7, MINERAL, HI_MINERAL,
        RIN_GAIN_STRENGTH),
RING("gain constitution", "opal",
     0,                        1, 150, 1, 1, 7, MINERAL, HI_MINERAL,
        RIN_GAIN_CONSTITUTION),
RING("increase accuracy", "clay",
     0,                        1, 150, 1, 1, 4, MINERAL, CLR_RED,
        RIN_INCREASE_ACCURACY),
RING("increase damage", "coral",
     0,                        1, 150, 1, 1, 4, MINERAL, CLR_ORANGE,
        RIN_INCREASE_DAMAGE),
RING("protection", "black onyx",
     PROTECTION,               1, 100, 1, 1, 7, MINERAL, CLR_BLACK,
        RIN_PROTECTION),
/* 'PROTECTION' intrinsic enhances MC from worn armor by +1,
           regardless of ring's enchantment; wearing a second ring of
           protection (or even one ring of protection combined with
           cloak of protection) doesn't give a second MC boost */
RING("regeneration", "moonstone",
     REGENERATION,             1, 200, 1, 0,  6, MINERAL, HI_MINERAL,
        RIN_REGENERATION),
RING("searching", "tiger eye",
     SEARCHING,                1, 200, 1, 0,  6, GEMSTONE, CLR_YELLOW,
        RIN_SEARCHING),
RING("stealth", "jade",
     STEALTH,                  1, 100, 1, 0,  6, GEMSTONE, CLR_GREEN,
        RIN_STEALTH),
RING("sustain ability", "bronze",
     FIXED_ABIL,               1, 100, 1, 0,  4, BRONZE, HI_BRONZE,
        RIN_SUSTAIN_ABILITY),
RING("levitation", "agate",
     LEVITATION,               1, 200, 1, 0,  7, GEMSTONE, CLR_RED,
        RIN_LEVITATION),
RING("hunger", "topaz",
     HUNGER,                   1, 100, 1, 0,  8, GEMSTONE, CLR_CYAN,
        RIN_HUNGER),
RING("aggravate monster", "sapphire",
     AGGRAVATE_MONSTER,        1, 150, 1, 0,  9, GEMSTONE, CLR_BLUE,
        RIN_AGGRAVATE_MONSTER),
RING("conflict", "ruby",
     CONFLICT,                 1, 300, 1, 0,  9, GEMSTONE, HI_GEMSTONE,
        RIN_CONFLICT),
RING("warning", "diamond",
     WARNING,                  1, 100, 1, 0, 10, GEMSTONE, CLR_WHITE,
        RIN_WARNING),
RING("poison resistance", "pearl",
     POISON_RES,               1, 150, 1, 0,  4, BONE, HI_BONE,
        RIN_POISON_RESISTANCE),
RING("fire resistance", "iron",
     FIRE_RES,                 1, 200, 1, 0,  5, IRON, HI_IRON,
        RIN_FIRE_RESISTANCE),
RING("cold resistance", "brass",
     COLD_RES,                 1, 150, 1, 0,  4, COPPER, HI_COPPER,
        RIN_COLD_RESISTANCE),
RING("shock resistance", "copper",
     SHOCK_RES,                1, 150, 1, 0,  3, COPPER, HI_COPPER,
        RIN_SHOCK_RESISTANCE),
RING("free action", "twisted",
     FREE_ACTION,              1, 200, 1, 0,  6, STEEL, HI_STEEL,
        RIN_FREE_ACTION),
RING("slow digestion", "steel",
     SLOW_DIGESTION,           1, 200, 1, 0,  8, STEEL, HI_STEEL,
        RIN_SLOW_DIGESTION),
RING("teleportation", "silver",
     TELEPORT,                 1, 200, 1, 0,  3, SILVER, HI_SILVER,
        RIN_TELEPORTATION),
RING("teleport control", "gold",
     TELEPORT_CONTROL,         1, 300, 1, 0,  3, GOLD, HI_GOLD,
        RIN_TELEPORT_CONTROL),
RING("polymorph", "ivory",
     POLYMORPH,                1, 300, 1, 0,  4, BONE, HI_BONE,
        RIN_POLYMORPH),
RING("polymorph control", "emerald",
     POLYMORPH_CONTROL,        1, 300, 1, 0,  8, GEMSTONE, CLR_BRIGHT_GREEN,
        RIN_POLYMORPH_CONTROL),
RING("invisibility", "wire",
     INVIS,                    1, 150, 1, 0,  5, STEEL, HI_STEEL,
        RIN_INVISIBILITY),
RING("see invisible", "engagement",
     SEE_INVIS,                1, 150, 1, 0,  5, GOLD, HI_GOLD,
        RIN_SEE_INVISIBLE),
RING("protection from shape changers", "shiny",
     PROT_FROM_SHAPE_CHANGERS, 1, 100, 1, 0,  5, PLATINUM, HI_PLATINUM,
        RIN_PROTECTION_FROM_SHAPE_CHAN),
/* placeholders for artifact rings; will not spawn randomly. raw OBJECT
   entries because they have no shuffled description and so must be born
   name-known (kn 1), which the RING wrapper cannot express */
/* === OBJECT entries === */
OBJECT(OBJ("ancient", NoDes),
        BITS(1, 0, 0, 0, 1, 0, 0, 0, 0, HARDGEM(8), 0, P_NONE, GEMSTONE),
        FREE_ACTION, RING_CLASS, 0, 0, 3, 800, 0, 0, 0, 0, 15,
        CLR_BRIGHT_GREEN, RIN_ANCIENT),
OBJECT(OBJ("lustrous", NoDes),
        BITS(1, 0, 0, 0, 1, 0, 0, 0, 0, HARDGEM(3), 0, P_NONE, GOLD),
        INVIS, RING_CLASS, 0, 0, 3, 600, 0, 0, 0, 0, 15,
        HI_GOLD, RIN_LUSTROUS),
/* amulets ... - THE Amulet comes last because it is special */

#undef RING

#define AMULET(name,desc,power,prob,sn) \
    OBJECT(OBJ(name, desc),                                            \
           BITS(0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, P_NONE, STEEL),       \
           power, AMULET_CLASS, prob, 0, 20, 150, 0, 0, 0, 0, 20,      \
           HI_STEEL, sn)
/* === AMULET entries === */
AMULET("amulet of ESP",                "circular", TELEPAT, 120,
        AMULET_OF_ESP),
MARKER(FIRST_AMULET, AMULET_OF_ESP)
AMULET("amulet of life saving",       "spherical", LIFESAVED, 75,
        AMULET_OF_LIFE_SAVING),
AMULET("amulet of strangulation",          "oval", STRANGLED, 115,
        AMULET_OF_STRANGULATION),
AMULET("amulet of restful sleep",    "triangular", SLEEPY, 115,
        AMULET_OF_RESTFUL_SLEEP),
AMULET("amulet versus poison",        "pyramidal", POISON_RES, 115,
        AMULET_VERSUS_POISON),
AMULET("amulet of change",               "square", 0, 115,
        AMULET_OF_CHANGE),
AMULET("amulet of unchanging",          "concave", UNCHANGING, 60,
        AMULET_OF_UNCHANGING),
AMULET("amulet of reflection",        "hexagonal", REFLECTING, 75,
        AMULET_OF_REFLECTION),
AMULET("amulet of magical breathing", "octagonal", MAGICAL_BREATHING, 75,
        AMULET_OF_MAGICAL_BREATHING),
AMULET("amulet of magic resistance",     "oblong", ANTIMAGIC, 30,
        AMULET_OF_MAGIC_RESISTANCE),
/* +2 AC and +2 MC; +2 takes naked hero past 'warded' to 'guarded' */
AMULET("amulet of guarding",         "perforated", PROTECTION, 75,
        AMULET_OF_GUARDING),
/* cubical: some descriptions are already three dimensional and
           parallelogrammatical (real word!) would be way over the top */
AMULET("amulet of flying",              "cubical", FLYING, 60,
        AMULET_OF_FLYING),
/* fixed descriptions; description duplication is deliberate;
 * fake one must come before real one because selection for
 * description shuffling stops when a non-magic amulet is encountered
 */

#undef AMULET
/* === OBJECT entries === */
OBJECT(OBJ("cheap plastic imitation of the Amulet of Yendor",
           "Amulet of Yendor"),
       BITS(0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, PLASTIC),
       0, AMULET_CLASS, 0, 0, 20, 0, 0, 0, 0, 0, 1, HI_METAL,
        FAKE_AMULET_OF_YENDOR),
OBJECT(OBJ("Amulet of Yendor", /* note: description == name */
           "Amulet of Yendor"),
       BITS(0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, GEMSTONE),
       0, AMULET_CLASS, 0, 0, 20, 30000, 0, 0, 0, 0, 20, HI_METAL,
        AMULET_OF_YENDOR),
MARKER(LAST_AMULET, AMULET_OF_YENDOR)
/* tools with weapon characteristics come last */
/* containers */

#define TOOL(name,desc,kn,mrg,mgc,chg,prob,wt,cost,mat,color,sn) \
    OBJECT(OBJ(name, desc),                                             \
           BITS(kn, mrg, chg, 0, mgc, chg, 0, 0, 0, 0, 0, P_NONE, mat), \
           0, TOOL_CLASS, prob, 0, wt, cost, 0, 0, 0, 0, wt, color, sn)
#define CONTAINER(name,desc,kn,mgc,chg,prob,wt,cost,mat,color,sn) \
    OBJECT(OBJ(name, desc),                                             \
           BITS(kn, 0, chg, 1, mgc, chg, 0, 0, 0, 0, 0, P_NONE, mat),   \
           0, TOOL_CLASS, prob, 0, wt, cost, 0, 0, 0, 0, wt, color, sn)
/* facewear; unlike TOOL this carries a property, which is how a worn
   blindfold or towel confers blindness through setworn() */
#define EYEWEAR(name,desc,kn,prop,prob,wt,cost,mat,color,sn) \
    OBJECT(OBJ(name, desc),                                             \
           BITS(kn, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, P_NONE, mat),         \
           prop, TOOL_CLASS, prob, 0, wt, cost, 0, 0, 0, 0, wt, color, sn)
#define WEPTOOL(name,desc,kn,mgc,bi,prob,wt,cost,sdam,ldam,hitbon,typ,sub,mat,clr,sn) \
    OBJECT(OBJ(name, desc),                                             \
           BITS(kn, 0, 1, 0, mgc, 1, 0, 0, bi, 0, typ, sub, mat),       \
           0, TOOL_CLASS, prob, 0, wt, cost, sdam, ldam, hitbon, 0, wt, \
           clr, sn)
/* === CONTAINER entries === */
CONTAINER("large box",        NoDes, 1, 0, 0, 40, 350,   8, WOOD, HI_WOOD,
        LARGE_BOX),
CONTAINER("chest",            NoDes, 1, 0, 0, 25, 600,  16, WOOD, HI_WOOD,
        CHEST),
CONTAINER("iron safe",        NoDes, 1, 0, 0, 10, 900,  50, IRON, HI_IRON,
        IRON_SAFE),
CONTAINER("crystal chest",    NoDes, 1, 1, 0,  1, 500,  20, GEMSTONE, CLR_WHITE,
        CRYSTAL_CHEST),
CONTAINER("ice box",          NoDes, 1, 0, 0,  5, 900,  42, PLASTIC, HI_PLASTIC,
        ICE_BOX),
/* define as object so it's unwishable. base type is "hidden chest" to
   avoid conflict with terrain type name, but description is "magic
   chest" so player gets correct feedback when they loot it. it should
   always remain unidentified */
/* === OBJECT entries === */
OBJECT(OBJ("hidden chest", "magic chest"),                             \
       BITS(0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, P_NONE, GEMSTONE),        \
       0, TOOL_CLASS, 0, 0, 5000, 999, 0, 0, 0, 0, 5000, CLR_MAGENTA,
        HIDDEN_CHEST),
/* === CONTAINER entries === */
CONTAINER("sack",            "bag", 0, 0, 0, 35,  15,   2, CLOTH, HI_CLOTH,
        SACK),
CONTAINER("oilskin sack",    "bag", 0, 0, 0,  5,  15, 100, CLOTH, HI_CLOTH,
        OILSKIN_SACK),
CONTAINER("bag of holding",  "bag", 0, 1, 0, 20,  15, 100, CLOTH, HI_CLOTH,
        BAG_OF_HOLDING),
CONTAINER("bag of tricks",   "bag", 0, 1, 1, 20,  15, 100, CLOTH, HI_CLOTH,
        BAG_OF_TRICKS),
/* same as a magic chest, define as objects so these are unwishable */
/* === OBJECT entries === */
OBJECT(OBJ("bag of treasure", "bag"),
       BITS(0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, P_NONE, CLOTH),
       0, TOOL_CLASS, 1, 0, 15, 100, 0, 0, 0, 0, 15, HI_CLOTH,
        BAG_OF_TREASURE),
OBJECT(OBJ("bag of devouring", "bag"),
       BITS(0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, P_NONE, CLOTH),
       0, TOOL_CLASS, 1, 0, 15, 100, 0, 0, 0, 0, 15, HI_CLOTH,
        BAG_OF_DEVOURING),
/* lock opening tools */
/* === TOOL entries === */
TOOL("skeleton key", "key",
     0, 0, 0, 0,  80,   3,   10, BONE, HI_BONE,
        SKELETON_KEY),
TOOL("lock pick", NoDes,
     1, 0, 0, 0,  60,   4,   20, STEEL, HI_STEEL,
        LOCK_PICK),
TOOL("credit card", NoDes,
     1, 0, 0, 0,  15,   1,   10, PLASTIC, HI_PLASTIC,
        CREDIT_CARD),
TOOL("magic key", "ornate key",
     0, 0, 1, 0,   0,   5, 1000, GEMSTONE, HI_GEMSTONE,
        MAGIC_KEY),
/* light sources */
TOOL("tallow candle", "candle",
     0, 1, 0, 0,  20,   2,  10, WAX, HI_WAX,
        TALLOW_CANDLE),
TOOL("wax candle", "candle",
     0, 1, 0, 0,   5,   2,  20, WAX, HI_WAX,
        WAX_CANDLE),
TOOL("lantern", NoDes,
     1, 0, 0, 0,  30,  30,  12, COPPER, CLR_YELLOW,
        LANTERN),
TOOL("oil lamp", "lamp",
     0, 0, 0, 0,  45,  20,  10, COPPER, CLR_YELLOW,
        OIL_LAMP),
TOOL("magic lamp", "lamp",
     0, 0, 1, 0,  15,  20, 500, COPPER, CLR_YELLOW,
        MAGIC_LAMP),
/* other tools */
TOOL("expensive camera", NoDes,
     1, 0, 0, 1,  15,  12, 200, PLASTIC, CLR_BLACK,
        EXPENSIVE_CAMERA),
TOOL("mirror", "looking glass",
     0, 0, 0, 0,  45,  13,  10, GLASS, HI_SILVER,
        MIRROR),
TOOL("crystal ball", "glass orb",
     0, 0, 1, 1,  15, 150,  60, GLASS, HI_GLASS,
        CRYSTAL_BALL),
TOOL("eight ball", "plastic orb",
     0, 0, 0, 0,   0,  20,  30, PLASTIC, CLR_BLACK,
        EIGHT_BALL),
EYEWEAR("lenses", NoDes,
     1,       0,   3,   3,  80, GLASS, HI_GLASS,
        LENSES),
EYEWEAR("goggles", NoDes,
     1,       0,   1,   3,  50, PLASTIC, HI_PLASTIC,
        GOGGLES),
EYEWEAR("blindfold", NoDes,
     1, BLINDED,  50,   2,  20, CLOTH, CLR_BLACK,
        BLINDFOLD),
EYEWEAR("towel", NoDes,
     1, BLINDED,  50,   2,  50, CLOTH, CLR_MAGENTA,
        TOWEL),
TOOL("saddle", NoDes,
     1, 0, 0, 0,   5, 200, 150, LEATHER, HI_LEATHER,
        SADDLE),
/* === OBJECT entries === */
OBJECT(OBJ("barding", NoDes),
        BITS(1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, P_NONE, IRON),
        0, TOOL_CLASS, 4, 0, 250, 300, 0, 0, 3, 0, 250, HI_IRON,
        BARDING),
OBJECT(OBJ("spiked barding", NoDes),
        BITS(1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, P_NONE, IRON),
        0, TOOL_CLASS, 2, 0, 275, 300, 0, 0, 3, 0, 275, HI_IRON,
        SPIKED_BARDING),
OBJECT(OBJ("barding of reflection", "polished barding"),
        BITS(0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, P_NONE, SILVER),
        0, TOOL_CLASS, 1, 0, 275, 600, 0, 0, 3, 0, 275, HI_SILVER,
        BARDING_OF_REFLECTION),
OBJECT(OBJ("runed barding", NoDes),
        BITS(1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, P_NONE, STEEL),
        0, TOOL_CLASS, 0, 0, 125, 900, 0, 0, 6, 0, 125, HI_STEEL,
        RUNED_BARDING),
/* === TOOL entries === */
TOOL("leash", NoDes,
     1, 0, 0, 0,  65,  12,  20, LEATHER, HI_LEATHER,
        LEASH),
TOOL("stethoscope", NoDes,
     1, 0, 0, 0,  25,   4,  75, IRON, HI_IRON,
        STETHOSCOPE),
TOOL("tinning kit", NoDes,
     1, 0, 0, 1,  20, 100,  30, IRON, HI_IRON,
        TINNING_KIT),
TOOL("tin opener", NoDes,
     1, 0, 0, 0,  35,   4,  30, IRON, HI_IRON,
        TIN_OPENER),
TOOL("can of grease", NoDes,
     1, 0, 0, 1,  10,  15,  20, IRON, HI_IRON,
        CAN_OF_GREASE),
TOOL("trap kit", NoDes,
     1, 0, 0, 1,   1,  25, 100, LEATHER, HI_LEATHER,
        TRAP_KIT),
TOOL("figurine", NoDes, /* monster type specified by obj->corpsenm */
     1, 0, 1, 0,  25,  50,  80, MINERAL, HI_MINERAL,
        FIGURINE),
/* === OBJECT entries === */
OBJECT(OBJ("blacksmith hammer", "runed hammer"),
       BITS(0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, P_NONE, IRON),
       0, TOOL_CLASS, 0, 0, 30, 300, 0, 0, 0, 0, 30, HI_IRON,
        BLACKSMITH_HAMMER),
/* === TOOL entries === */
TOOL("magic marker", "marker",
     0, 0, 1, 1,   0,   2,  50, PLASTIC, CLR_RED,
        MAGIC_MARKER),
/* traps; compose_glyph_name() in glyphs.c prefixes "unset " to the
   glyph names of ARROW_TRAP through MAGIC_BEAM_TRAP, so keep that run
   unbroken */
TOOL("land mine", NoDes,
     1, 0, 0, 0,   0, 300, 180, IRON, CLR_RED,
        LAND_MINE),
TOOL("beartrap", NoDes,
     1, 0, 0, 0,   0, 200,  60, IRON, HI_IRON,
        BEARTRAP),
TOOL("arrow trap", NoDes,
     1, 0, 0, 0,   0,  50,  20, IRON, HI_IRON,
        ARROW_TRAP),
TOOL("bolt trap", NoDes,
     1, 0, 0, 0,   0,  50,  20, IRON, HI_IRON,
        BOLT_TRAP),
TOOL("dart trap", NoDes,
     1, 0, 0, 0,   0,  50,  20, IRON, HI_IRON,
        DART_TRAP),
TOOL("spear trap", NoDes,
     1, 0, 0, 0,   0,  50,  40, IRON, HI_WOOD,
        SPEAR_TRAP),
TOOL("sleeping gas trap", NoDes,
     1, 0, 0, 0,   0,  50,  75, IRON, HI_ZAP,
        SLEEPING_GAS_TRAP),
TOOL("fire trap", NoDes,
     1, 0, 0, 0,   0,  50,  75, IRON, CLR_ORANGE,
        FIRE_TRAP),
TOOL("ice trap", NoDes,
     1, 0, 0, 0,   0,  50,  75, IRON, CLR_BRIGHT_CYAN,
        ICE_TRAP),
TOOL("teleportation trap", NoDes,
     1, 0, 0, 0,   0,  50, 100, IRON, CLR_MAGENTA,
        TELEPORTATION_TRAP),
TOOL("magic trap", NoDes,
     1, 0, 0, 0,   0,  50, 250, IRON, HI_ZAP,
        MAGIC_TRAP),
TOOL("anti-magic trap", NoDes,
     1, 0, 0, 0,   0,  50, 200, IRON, HI_ZAP,
        ANTI_MAGIC_TRAP),
TOOL("polymorph trap", NoDes,
     1, 0, 0, 0,   0,  50, 300, IRON, CLR_BRIGHT_GREEN,
        POLYMORPH_TRAP),
TOOL("magic beam trap", NoDes,
     1, 0, 0, 0,   0,  50,  75, IRON, CLR_YELLOW,
        MAGIC_BEAM_TRAP),
/* instruments */
TOOL("pea whistle", "whistle",
     0, 0, 0, 0, 100,   3,  10, STEEL, HI_STEEL,
        PEA_WHISTLE),
TOOL("magic whistle", "whistle",
     0, 0, 1, 0,  30,   3,  10, STEEL, HI_STEEL,
        MAGIC_WHISTLE),
TOOL("flute", "flute",
     0, 0, 0, 0,   4,   5,  12, WOOD, HI_WOOD,
        FLUTE),
TOOL("pan flute", NoDes,
     1, 0, 0, 0,   0,   6,  24, WOOD, HI_WOOD,
        PAN_FLUTE),
TOOL("magic flute", "flute",
     0, 0, 1, 1,   2,   5,  36, WOOD, HI_WOOD,
        MAGIC_FLUTE),
TOOL("tooled horn", "horn",
     0, 0, 0, 0,   5,  18,  15, BONE, HI_BONE,
        TOOLED_HORN),
TOOL("frost horn", "horn",
     0, 0, 1, 1,   2,  18,  50, BONE, HI_BONE,
        FROST_HORN),
TOOL("fire horn", "horn",
     0, 0, 1, 1,   2,  18,  50, BONE, HI_BONE,
        FIRE_HORN),
TOOL("horn of plenty", "horn", /* horn, but not an instrument */
     0, 0, 1, 1,   2,  18,  50, BONE, HI_BONE,
        HORN_OF_PLENTY),
TOOL("harp", "harp",
     0, 0, 0, 0,   4,  30,  50, WOOD, HI_WOOD,
        HARP),
TOOL("magic harp", "harp",
     0, 0, 1, 1,   2,  30,  50, WOOD, HI_WOOD,
        MAGIC_HARP),
TOOL("bell", NoDes,
     1, 0, 0, 0,   2,  30,  50, COPPER, HI_COPPER,
        BELL),
TOOL("bugle", NoDes,
     1, 0, 0, 0,   4,  10,  15, COPPER, HI_COPPER,
        BUGLE),
TOOL("leather drum", "drum",
     0, 0, 0, 0,   4,  25,  25, LEATHER, HI_LEATHER,
        LEATHER_DRUM),
TOOL("drum of earthquake", "drum",
     0, 0, 1, 1,   2,  25,  25, LEATHER, HI_LEATHER,
        DRUM_OF_EARTHQUAKE),
/* tools useful as weapons */
/* === WEPTOOL entries === */
WEPTOOL("pick-axe", NoDes,
        1, 0, 0, 20, 100,  50,  6,  3, 0, WHACK,  P_PICK_AXE, IRON, HI_IRON,
        PICK_AXE),
WEPTOOL("grappling hook", "iron hook",
        0, 0, 0,  5,  30,  50,  2,  6, 0, WHACK,  P_FLAIL,    IRON, HI_IRON,
        GRAPPLING_HOOK),
WEPTOOL("unicorn horn", NoDes,
        1, 1, 0,  0,  20, 100,  8, 10, 1, PIERCE, P_UNICORN_HORN, BONE, HI_BONE,
        UNICORN_HORN),
/* 3.4.1: unicorn horn left classified as "magic" */
/* two unique tools;
 * not artifacts, despite the comment which used to be here
 */

#undef CONTAINER
#undef TOOL
#undef WEPTOOL
#undef EYEWEAR
/* === OBJECT entries === */
OBJECT(OBJ("Candelabrum of Invocation", "candelabrum"),
       BITS(0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, P_NONE, GOLD),
       0, TOOL_CLASS, 0, 0, 10, 5000, 0, 0, 0, 0, 200, HI_GOLD,
        CANDELABRUM_OF_INVOCATION),
OBJECT(OBJ("Bell of Opening", "engraved silver bell"),
       BITS(0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, P_NONE, SILVER),
       0, TOOL_CLASS, 0, 0, 10, 5000, 0, 0, 0, 0, 50, HI_SILVER,
        BELL_OF_OPENING),
/* meat */

#define FOOD(name, prob, delay, wt, unk, tin, nutrition, color, sn)     \
    OBJECT(OBJ(name, NoDes),                                            \
           BITS(1, 1, unk, 0, 0, 0, 0, 0, 0, 0, 0, P_NONE, tin), 0,     \
           FOOD_CLASS, prob, delay, wt, nutrition / 20 + 5, 0, 0, 0, 0, \
           nutrition, color, sn)
/* === FOOD entries === */
FOOD("tripe ration",        140,  2, 10, 0, FLESH, 200, CLR_BROWN,
        TRIPE_RATION),
FOOD("corpse",                0,  1,  0, 0, FLESH,   0, CLR_BROWN,
        CORPSE),
/* body parts (currently only for base object: the Eye of Vecna) */
FOOD("eyeball",               0,  1,  1, 0, FLESH,   5, CLR_WHITE,
        EYEBALL),
FOOD("egg",                  85,  1,  1, 1, FLESH,  80, HI_BONE,
        EGG),
FOOD("meatball",              0,  1,  1, 0, FLESH,   5, HI_FLESH,
        MEATBALL),
FOOD("meat stick",            0,  1,  1, 0, FLESH,   5, HI_FLESH,
        MEAT_STICK),
FOOD("strip of bacon",        0,  1,  1, 0, FLESH,   5, HI_FLESH,
        STRIP_OF_BACON),
FOOD("enormous meatball",     0, 20,400, 0, FLESH,2000, HI_FLESH,
        ENORMOUS_MEATBALL),
/* special cases because they aren't mergable */
/* === OBJECT entries === */
OBJECT(OBJ("meat ring", NoDes),
       BITS(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, FLESH),
       0, FOOD_CLASS, 0, 1, 5, 1, 0, 0, 0, 0, 5, HI_FLESH,
        MEAT_RING),
OBJECT(OBJ("meat suit", NoDes),
       BITS(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, FLESH),
       0, FOOD_CLASS, 0, 3, 50, 1, 0, 0, 0, 0, 200, HI_FLESH,
        MEAT_SUIT),
OBJECT(OBJ("meat helmet", NoDes),
       BITS(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, FLESH),
       0, FOOD_CLASS, 0, 1, 10, 1, 0, 0, 0, 0, 20, HI_FLESH,
        MEAT_HELMET),
OBJECT(OBJ("meat shield", NoDes),
       BITS(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, FLESH),
       0, FOOD_CLASS, 0, 1, 15, 1, 0, 0, 0, 0, 30, HI_FLESH,
        MEAT_SHIELD),
OBJECT(OBJ("meat gloves", NoDes),
       BITS(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, FLESH),
       0, FOOD_CLASS, 0, 1, 10, 1, 0, 0, 0, 0, 20, HI_FLESH,
        MEAT_GLOVES),
OBJECT(OBJ("meat boots", NoDes),
       BITS(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, FLESH),
       0, FOOD_CLASS, 0, 1, 10, 1, 0, 0, 0, 0, 20, HI_FLESH,
        MEAT_BOOTS),
/* pudding 'corpses' will turn into these and combine;
   must be in same order as the pudding monsters */
/* === FOOD entries === */
FOOD("glob of gray ooze",     0,  2, 20, 0, FLESH,  20, CLR_GRAY,
        GLOB_OF_GRAY_OOZE),
FOOD("glob of brown pudding", 0,  2, 20, 0, FLESH,  20, CLR_BROWN,
        GLOB_OF_BROWN_PUDDING),
FOOD("glob of green slime",   0,  2, 20, 0, FLESH,  20, CLR_GREEN,
        GLOB_OF_GREEN_SLIME),
FOOD("glob of black pudding", 0,  2, 20, 0, FLESH,  20, CLR_BLACK,
        GLOB_OF_BLACK_PUDDING),
/* fruits & veggies */
FOOD("kelp frond",            0,  1,  1, 0, VEGGY,  30, HI_VEGGY,
        KELP_FROND),
FOOD("eucalyptus leaf",      10,  1,  1, 0, VEGGY,  30, HI_VEGGY,
        EUCALYPTUS_LEAF),
FOOD("mistletoe",             3,  1,  1, 0, VEGGY, 100, HI_VEGGY,
        MISTLETOE),
FOOD("apple",                10,  1,  2, 0, VEGGY,  50, CLR_RED,
        APPLE),
FOOD("orange",               10,  1,  2, 0, VEGGY,  80, HI_ORANGE,
        ORANGE),
FOOD("pear",                 10,  1,  2, 0, VEGGY,  50, CLR_BRIGHT_GREEN,
        PEAR),
FOOD("melon",                10,  1,  5, 0, VEGGY, 100, CLR_BRIGHT_GREEN,
        MELON),
FOOD("banana",               10,  1,  2, 0, VEGGY,  80, CLR_YELLOW,
        BANANA),
FOOD("carrot",               12,  1,  2, 0, VEGGY,  50, CLR_ORANGE,
        CARROT),
FOOD("sprig of wolfsbane",    7,  1,  1, 0, VEGGY,  40, HI_VEGGY,
        SPRIG_OF_WOLFSBANE),
FOOD("clove of garlic",       7,  1,  1, 0, VEGGY,  40, HI_WAX,
        CLOVE_OF_GARLIC),
/* name of slime mold is changed based on player's OPTION=fruit:something
   and bones data might have differently named ones from prior games */
FOOD("slime mold",           75,  1,  5, 0, VEGGY, 250, HI_ORGANIC,
        SLIME_MOLD),
/* people food */
FOOD("lump of royal jelly",   0,  1,  2, 0, VEGGY, 200, CLR_MAGENTA,
        LUMP_OF_ROYAL_JELLY),
FOOD("cream pie",            25,  1, 10, 0, VEGGY, 100, CLR_WHITE,
        CREAM_PIE),
FOOD("candy bar",            10,  1,  2, 0, VEGGY, 100, CLR_BROWN,
        CANDY_BAR),
FOOD("fortune cookie",       55,  1,  1, 0, VEGGY,  40, CLR_YELLOW,
        FORTUNE_COOKIE),
FOOD("pancake",              25,  2,  2, 0, VEGGY, 200, CLR_YELLOW,
        PANCAKE),
FOOD("lembas wafer",         20,  2,  5, 0, VEGGY, 800, CLR_WHITE,
        LEMBAS_WAFER),
FOOD("cram ration",          20,  3, 15, 0, VEGGY, 600, HI_ORGANIC,
        CRAM_RATION),
FOOD("food ration",         380,  5, 20, 0, VEGGY, 800, HI_ORGANIC,
        FOOD_RATION),
FOOD("K-ration",              0,  1, 10, 0, VEGGY, 400, HI_ORGANIC,
        K_RATION),
FOOD("C-ration",              0,  1, 10, 0, VEGGY, 300, HI_ORGANIC,
        C_RATION),
/* tins have type specified by obj->spe (+1 for spinach, other implies
   flesh; negative specifies preparation method {homemade,boiled,&c})
   and by obj->corpsenm (type of monster flesh) */
FOOD("tin",                  75,  0, 10, 1, STEEL,   0, HI_STEEL,
        TIN),
/* potions ... */

#undef FOOD

#define POTION(name,desc,mgc,power,prob,cost,color,sn) \
    OBJECT(OBJ(name, desc),                                             \
           BITS(0, 1, 0, 0, mgc, 0, 0, 0, 0, 0, 0, P_NONE, GLASS),      \
           power, POTION_CLASS, prob, 0, 20, cost, 0, 0, 0, 0, 10,      \
           color, sn)
/* === POTION entries === */
POTION("gain ability",           "ruby",  1, 0, 40, 300, CLR_RED,
        POT_GAIN_ABILITY),
POTION("restore ability",        "pink",  1, 0, 40, 100, CLR_BRIGHT_MAGENTA,
        POT_RESTORE_ABILITY),
POTION("confusion",            "orange",  1, CONFUSION, 42, 100, HI_ORANGE,
        POT_CONFUSION),
POTION("blindness",            "yellow",  1, BLINDED, 40, 150, CLR_YELLOW,
        POT_BLINDNESS),
POTION("paralysis",           "emerald",  1, 0, 40, 300, CLR_BRIGHT_GREEN,
        POT_PARALYSIS),
POTION("speed",                 "fizzy",  1, FAST, 40, 200, HI_STEEL,
        POT_SPEED),
POTION("levitation",             "cyan",  1, LEVITATION, 42, 200, HI_IRON,
        POT_LEVITATION),
POTION("hallucination",      "sky blue",  1, HALLUC, 40, 100, HI_MITHRIL,
        POT_HALLUCINATION),
POTION("invisibility", "brilliant blue",  1, INVIS, 40, 150, CLR_BRIGHT_BLUE,
        POT_INVISIBILITY),
POTION("see invisible",       "magenta",  1, SEE_INVIS, 42, 50, CLR_MAGENTA,
        POT_SEE_INVISIBLE),
POTION("healing",          "purple-red",  1, 0, 57, 100, HI_PURPLE,
        POT_HEALING),
POTION("extra healing",          "puce",  1, 0, 47, 100, HI_GEMSTONE,
        POT_EXTRA_HEALING),
POTION("gain level",            "milky",  1, 0, 20, 300, HI_BONE,
        POT_GAIN_LEVEL),
POTION("enlightenment",        "swirly",  1, 0, 20, 200, HI_BRONZE,
        POT_ENLIGHTENMENT),
POTION("monster detection",    "bubbly",  1, 0, 40, 150, HI_WAX,
        POT_MONSTER_DETECTION),
POTION("object detection",      "smoky",  1, 0, 42, 150, CLR_GRAY,
        POT_OBJECT_DETECTION),
POTION("gain energy",          "cloudy",  1, 0, 40, 150, HI_CLOTH,
        POT_GAIN_ENERGY),
POTION("sleeping",       "effervescent",  1, 0, 42, 100, HI_PLASTIC,
        POT_SLEEPING),
POTION("full healing",          "black",  1, 0, 10, 200, CLR_BLACK,
        POT_FULL_HEALING),
POTION("polymorph",            "golden",  1, 0, 10, 200, HI_GOLD,
        POT_POLYMORPH),
POTION("booze",                 "brown",  0, 0, 42,  50, CLR_BROWN,
        POT_BOOZE),
POTION("fruit juice",            "dark",  0, 0, 42,  50, CLR_BLACK,
        POT_FRUIT_JUICE),
POTION("acid",                  "white",  0, 0, 10, 250, CLR_WHITE,
        POT_ACID),
POTION("oil",                   "murky",  0, 0, 30, 250, CLR_ORANGE,
        POT_OIL),
/* fixed descriptions */
POTION("sickness",         "dark green",  0, 0, 42,  50, HI_VEGGY,
        POT_SICKNESS),
POTION("drow poison",            "inky",  0, 0,  8, 300, HI_ADAMANTINE,
        POT_DROW_POISON),
POTION("blood",             "blood-red",  0, 0,  0,  50, HI_DRAGON_HIDE,
        POT_BLOOD),
POTION("vampire blood",     "blood-red",  1, 0,  0, 300, HI_DRAGON_HIDE,
        POT_VAMPIRE_BLOOD),
POTION("water",                 "clear",  0, 0, 92, 100, CLR_CYAN,
        POT_WATER),
/* scrolls ... */

#undef POTION

#define SCROLL(name,text,mgc,prob,cost,sn) \
    OBJECT(OBJ(name, text),                                           \
           BITS(0, 1, 0, 0, mgc, 0, 0, 0, 0, 0, 0, P_NONE, PAPER),    \
           0, SCROLL_CLASS, prob, 0, 5, cost, 0, 0, 0, 0, 6,          \
           HI_PAPER, sn)
/* === SCROLL entries === */
SCROLL("enchant armor",              "ZELGO MER",  1,  65,  80,
        SCR_ENCHANT_ARMOR),
SCROLL("destroy armor",         "JUYED AWK YACC",  1,  45, 100,
        SCR_DESTROY_ARMOR),
SCROLL("confuse monster",                 "NR 9",  1,  53, 100,
        SCR_CONFUSE_MONSTER),
SCROLL("scare monster",   "XIXAXA XOXAXA XUXAXA",  1,  35, 100,
        SCR_SCARE_MONSTER),
SCROLL("remove curse",             "PRATYAVAYAH",  1,  65,  80,
        SCR_REMOVE_CURSE),
SCROLL("enchant weapon",         "DAIYEN FOOELS",  1,  80,  60,
        SCR_ENCHANT_WEAPON),
SCROLL("create monster",       "LEP GEX VEN ZEA",  1,  45, 200,
        SCR_CREATE_MONSTER),
SCROLL("taming",                   "PRIRUTSENIE",  1,  15, 200,
        SCR_TAMING),
SCROLL("genocide",                  "ELBIB YLOH",  1,  15, 300,
        SCR_GENOCIDE),
SCROLL("light",                 "VERR YED HORRE",  1,  90,  50,
        SCR_LIGHT),
SCROLL("teleportation",        "VENZAR BORGAVVE",  1,  55, 100,
        SCR_TELEPORTATION),
SCROLL("gold detection",                 "THARR",  1,  34, 100,
        SCR_GOLD_DETECTION),
SCROLL("food detection",               "YUM YUM",  1,  25, 100,
        SCR_FOOD_DETECTION),
SCROLL("identify",                  "KERNOD WEL",  1, 180,  20,
        SCR_IDENTIFY),
SCROLL("magic mapping",              "ELAM EBOW",  1,  45, 100,
        SCR_MAGIC_MAPPING),
SCROLL("amnesia",                   "DUAM XNAHT",  1,  35, 200,
        SCR_AMNESIA),
SCROLL("fire",                  "ANDOVA BEGARIN",  1,  30, 100,
        SCR_FIRE),
SCROLL("earth",                          "KIRJE",  1,  18, 200,
        SCR_EARTH),
SCROLL("punishment",            "VE FORBRYDERNE",  1,  15, 300,
        SCR_PUNISHMENT),
SCROLL("charging",                "HACKEM MUCHE",  1,  15, 300,
        SCR_CHARGING),
SCROLL("stinking cloud",             "VELOX NEB",  1,  15, 300,
        SCR_STINKING_CLOUD),
SCROLL("magic detection",        "FOOBIE BLETCH",  1,  25, 300,
        SCR_MAGIC_DETECTION),
/* === OBJECT entries === */
OBJECT(OBJ("consecration", "GILCHEX FRONUTS"),
       BITS(0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, P_NONE, PAPER),
       0, SCROLL_CLASS, 0, 0, 5, 5000, 0, 0, 0, 0, 6, HI_PAPER,
        SCR_CONSECRATION),
/* Extra descriptions, shuffled into use at start of new game.
     * compose_glyph_name() in glyphs.c depends on SCR_CONSECRATION being
     * the last real scroll before these, and win/share/tilemap.c depends
     * on how many of them there are. If a real scroll gets added after
     * consecration or the number of extra descriptions changes, both
     * must be modified to match
     */
/* === SCROLL entries === */
SCROLL(NoDes,              "TEMOV",  1,   0, 100,
        SC01),
SCROLL(NoDes,         "GARVEN DEH",  1,   0, 100,
        SC02),
SCROLL(NoDes,            "READ ME",  1,   0, 100,
        SC03),
SCROLL(NoDes,      "ETAOIN SHRDLU",  1,   0, 100,
        SC04),
SCROLL(NoDes,        "LOREM IPSUM",  1,   0, 100,
        SC05),
SCROLL(NoDes,              "FNORD",  1,   0, 100,
        SC06),
/* Illuminati */
SCROLL(NoDes,            "KO BATE",  1,   0, 100,
        SC07),
/* Kurd Lasswitz */
SCROLL(NoDes,      "ABRA KA DABRA",  1,   0, 100,
        SC08),
/* traditional incantation */
SCROLL(NoDes,       "ASHPD SODALG",  1,   0, 100,
        SC09),
/* Portal */
SCROLL(NoDes,            "ZLORFIK",  1,   0, 100,
        SC10),
/* Zak McKracken */
SCROLL(NoDes,      "GNIK SISI VLE",  1,   0, 100,
        SC11),
/* Zak McKracken */
SCROLL(NoDes,    "HAPAX LEGOMENON",  1,   0, 100,
        SC12),
SCROLL(NoDes,  "EIRIS SAZUN IDISI",  1,   0, 100,
        SC13),
/* Merseburg Incantations */
SCROLL(NoDes,    "PHOL ENDE WODAN",  1,   0, 100,
        SC14),
/* Merseburg Incantations */
SCROLL(NoDes,              "GHOTI",  1,   0, 100,
        SC15),
/* pronounced as 'fish',
                                                        George Bernard Shaw */
SCROLL(NoDes, "MAPIRO MAHAMA DIROMAT", 1, 0, 100,
        SC16),
/* Wizardry */
SCROLL(NoDes,  "VAS CORP BET MANI",  1,   0, 100,
        SC17),
/* Ultima */
SCROLL(NoDes,            "XOR OTA",  1,   0, 100,
        SC18),
/* Aarne Haapakoski */
SCROLL(NoDes, "STRC PRST SKRZ KRK",  1,   0, 100,
        SC19),
/* Czech and Slovak
                                                        tongue-twister */
/* These must come last because they have special fixed descriptions.
     */
SCROLL("mail",          "stamped",  0,   0,   0,
        SCR_MAIL),
SCROLL("blank paper", "unlabeled",  0,  28,  60,
        SCR_BLANK_PAPER),
/* override enum for use in SPELL() expansion */

#undef SCROLL

#define SPELL(name,desc,sub,prob,delay,level,mgc,dir,color,sn)  \
    OBJECT(OBJ(name, desc),                                             \
           BITS(0, 0, 0, 0, mgc, 0, 0, 0, 0, 0, dir, sub, PAPER),       \
           0, SPBOOK_CLASS, prob, delay, level * 5 + 30,                \
           level * 100, 0, 0, 0, level, 20, color, sn)
/* === SPELL entries === */
#define PAPER LEATHER /* override enum for use in SPELL() expansion */
SPELL("dig",              "parchment",
      P_MATTER_SPELL,      20,  6, 3, 1, RAY, HI_LEATHER,
        SPE_DIG),
MARKER(FIRST_SPELL, SPE_DIG)
SPELL("magic missile",    "vellum",
      P_ATTACK_SPELL,      25,  2, 2, 1, RAY, HI_LEATHER,
        SPE_MAGIC_MISSILE),
#undef PAPER /* revert to normal material */
/* revert to normal material */
SPELL("fireball",         "ragged",
      P_ATTACK_SPELL,      20,  4, 4, 1, RAY, HI_PAPER,
        SPE_FIREBALL),
SPELL("cone of cold",     "dog eared",
      P_ATTACK_SPELL,      10,  7, 4, 1, RAY, HI_CLOTH,
        SPE_CONE_OF_COLD),
SPELL("sleep",            "mottled",
      P_ENCHANTMENT_SPELL, 20,  1, 2, 1, RAY, HI_GLASS,
        SPE_SLEEP),
SPELL("finger of death",  "stained",
      P_EVOCATION_SPELL,    5, 10, 7, 1, RAY, HI_MINERAL,
        SPE_FINGER_OF_DEATH),
SPELL("lightning",        "electric blue",
      P_ATTACK_SPELL,      10,  8, 4, 1, RAY, CLR_BRIGHT_BLUE,
        SPE_LIGHTNING),
SPELL("poison blast",     "olive green",
      P_ATTACK_SPELL,      10,  9, 5, 1, RAY, HI_VEGGY,
        SPE_POISON_BLAST),
SPELL("acid blast",       "acid green",
      P_ATTACK_SPELL,       5,  9, 6, 1, RAY, CLR_BRIGHT_GREEN,
        SPE_ACID_BLAST),
SPELL("psionic wave",     "worn",
      P_ATTACK_SPELL,       0,  1, 1, 1, IMMEDIATE, CLR_MAGENTA,
        SPE_PSIONIC_WAVE),
SPELL("power word kill",  "ominous",
      P_ATTACK_SPELL,       5, 10, 7, 1, NODIR, HI_DRAGON_HIDE,
        SPE_POWER_WORD_KILL),
SPELL("light",            "cloth",
      P_DIVINATION_SPELL,  25,  1, 1, 1, NODIR, HI_CLOTH,
        SPE_LIGHT),
SPELL("detect monsters",  "leathery",
      P_DIVINATION_SPELL,  20,  1, 2, 1, NODIR, HI_LEATHER,
        SPE_DETECT_MONSTERS),
SPELL("healing",          "white",
      P_HEALING_SPELL,     25,  2, 1, 1, IMMEDIATE, CLR_WHITE,
        SPE_HEALING),
SPELL("knock",            "pink",
      P_MATTER_SPELL,      25,  1, 1, 1, IMMEDIATE, CLR_BRIGHT_MAGENTA,
        SPE_KNOCK),
SPELL("force bolt",       "red",
      P_ATTACK_SPELL,      30,  2, 1, 1, IMMEDIATE, HI_GEMSTONE,
        SPE_FORCE_BOLT),
SPELL("confuse monster",  "orange",
      P_ENCHANTMENT_SPELL, 20,  2, 1, 1, IMMEDIATE, CLR_ORANGE,
        SPE_CONFUSE_MONSTER),
SPELL("cure blindness",   "yellow",
      P_HEALING_SPELL,     20,  2, 2, 1, IMMEDIATE, CLR_YELLOW,
        SPE_CURE_BLINDNESS),
SPELL("drain life",       "velvet",
      P_ATTACK_SPELL,      10,  2, 2, 1, IMMEDIATE, CLR_MAGENTA,
        SPE_DRAIN_LIFE),
SPELL("slow monster",     "light green",
      P_ENCHANTMENT_SPELL, 20,  2, 2, 1, IMMEDIATE, CLR_BRIGHT_GREEN,
        SPE_SLOW_MONSTER),
SPELL("wizard lock",      "dark green",
      P_MATTER_SPELL,      20,  3, 2, 1, IMMEDIATE, CLR_GREEN,
        SPE_WIZARD_LOCK),
SPELL("create monster",   "turquoise",
      P_CLERIC_SPELL,      20,  3, 2, 1, NODIR, CLR_BRIGHT_CYAN,
        SPE_CREATE_MONSTER),
SPELL("detect food",      "cyan",
      P_DIVINATION_SPELL,  20,  3, 1, 1, NODIR, CLR_CYAN,
        SPE_DETECT_FOOD),
SPELL("cause fear",       "light blue",
      P_ENCHANTMENT_SPELL, 20,  3, 3, 1, NODIR, CLR_BRIGHT_BLUE,
        SPE_CAUSE_FEAR),
SPELL("clairvoyance",     "dark blue",
      P_DIVINATION_SPELL,  15,  3, 3, 1, NODIR, CLR_BLUE,
        SPE_CLAIRVOYANCE),
SPELL("cure sickness",    "indigo",
      P_HEALING_SPELL,     25,  3, 3, 1, IMMEDIATE, CLR_BLUE,
        SPE_CURE_SICKNESS),
SPELL("charm monster",    "magenta",
      P_ENCHANTMENT_SPELL, 15,  3, 5, 1, IMMEDIATE, HI_PLASTIC,
        SPE_CHARM_MONSTER),
SPELL("haste self",       "purple",
      P_ESCAPE_SPELL,      25,  4, 3, 1, NODIR, HI_PURPLE,
        SPE_HASTE_SELF),
SPELL("detect unseen",    "violet",
      P_DIVINATION_SPELL,  20,  4, 2, 1, NODIR, HI_PLASTIC,
        SPE_DETECT_UNSEEN),
SPELL("levitation",       "tan",
      P_ESCAPE_SPELL,      20,  4, 4, 1, NODIR, CLR_BROWN,
        SPE_LEVITATION),
SPELL("extra healing",    "plaid",
      P_HEALING_SPELL,     25,  5, 3, 1, IMMEDIATE, CLR_GREEN,
        SPE_EXTRA_HEALING),
SPELL("restore ability",  "light brown",
      P_HEALING_SPELL,     25,  5, 2, 1, IMMEDIATE, HI_WOOD,
        SPE_RESTORE_ABILITY),
SPELL("invisibility",     "dark brown",
      P_ESCAPE_SPELL,      20,  5, 3, 1, NODIR, CLR_BROWN,
        SPE_INVISIBILITY),
SPELL("detect treasure",  "gray",
      P_DIVINATION_SPELL,  20,  5, 4, 1, NODIR, CLR_GRAY,
        SPE_DETECT_TREASURE),
SPELL("remove curse",     "wrinkled",
      P_CLERIC_SPELL,      20,  5, 3, 1, NODIR, HI_PAPER,
        SPE_REMOVE_CURSE),
SPELL("magic mapping",    "dusty",
      P_DIVINATION_SPELL,  15,  7, 5, 1, NODIR, HI_ADAMANTINE,
        SPE_MAGIC_MAPPING),
SPELL("identify",         "bronze",
      P_DIVINATION_SPELL,  20,  6, 3, 1, NODIR, HI_BRONZE,
        SPE_IDENTIFY),
SPELL("turn undead",      "copper",
      P_CLERIC_SPELL,      10,  5, 4, 1, IMMEDIATE, HI_COPPER,
        SPE_TURN_UNDEAD),
SPELL("dispel evil",      "archaic",
      P_CLERIC_SPELL,      10,  8, 5, 1, IMMEDIATE, HI_GOLD,
        SPE_DISPEL_EVIL),
SPELL("polymorph",        "silver",
      P_MATTER_SPELL,      10,  8, 6, 1, IMMEDIATE, HI_SILVER,
        SPE_POLYMORPH),
SPELL("teleport away",    "gold",
      P_ESCAPE_SPELL,      15,  6, 6, 1, IMMEDIATE, HI_GOLD,
        SPE_TELEPORT_AWAY),
SPELL("create familiar",  "glittering",
      P_CLERIC_SPELL,      10,  4, 3, 1, NODIR, HI_MITHRIL,
        SPE_CREATE_FAMILIAR),
SPELL("cancellation",     "shining",
      P_MATTER_SPELL,       5,  8, 7, 1, IMMEDIATE, HI_BONE,
        SPE_CANCELLATION),
SPELL("protection",       "dull",
      P_CLERIC_SPELL,      15,  3, 1, 1, NODIR, HI_FLESH,
        SPE_PROTECTION),
SPELL("jumping",          "thin",
      P_ESCAPE_SPELL,      20,  3, 2, 1, IMMEDIATE, HI_PAPER,
        SPE_JUMPING),
SPELL("stone to flesh",   "thick",
      P_HEALING_SPELL,     15,  1, 3, 1, IMMEDIATE, HI_WAX,
        SPE_STONE_TO_FLESH),
SPELL("chain lightning", "checkered",
      P_ATTACK_SPELL, 25, 4, 2, 1, NODIR, CLR_GRAY,
                                                        SPE_CHAIN_LIGHTNING),
SPELL("repair armor",     "platinum",
      P_MATTER_SPELL,      20,  6, 3, 1, NODIR, HI_PLATINUM,
        SPE_REPAIR_ARMOR),
SPELL("reflection",       "decrepit",
      P_MATTER_SPELL,      10,  3, 5, 1, NODIR, HI_ORANGE,
        SPE_REFLECTION),
SPELL("critical healing", "antique",
      P_HEALING_SPELL,     10,  8, 6, 1, IMMEDIATE, CLR_BRIGHT_GREEN,
        SPE_CRITICAL_HEALING),
SPELL("burning hands",    "crinkled",
      P_ENCHANTMENT_SPELL, 20,  2, 1, 1, NODIR, CLR_RED,
        SPE_BURNING_HANDS),
SPELL("shocking grasp",   "warped",
      P_ENCHANTMENT_SPELL, 15,  2, 2, 1, NODIR, CLR_BRIGHT_BLUE,
        SPE_SHOCKING_GRASP),
/* from slash'em (modified), create a tame critter which explodes when
   attacking, damaging adjacent creatures and dying in the process */
SPELL("orb of fire",      "canvas",
      P_MATTER_SPELL,      15,  2, 2, 1, NODIR, CLR_BROWN,
        SPE_ORB_OF_FIRE),
SPELL("orb of frost",     "hardcover",
      P_MATTER_SPELL,      15,  2, 2, 1, NODIR, CLR_BROWN,
        SPE_ORB_OF_FROST),
/* Druid spells, sans 'Finger of Death' (ray-based spells
   need to stay grouped together) */
SPELL("entangle",         "singed",
      P_EVOCATION_SPELL,   20,  1, 1, 1, IMMEDIATE, CLR_BRIGHT_BLUE,
        SPE_ENTANGLE),
SPELL("barkskin",         "torn",
      P_EVOCATION_SPELL,   20,  2, 2, 1, NODIR, CLR_GREEN,
        SPE_BARKSKIN),
SPELL("create grass",     "moldy",
      P_EVOCATION_SPELL,   20,  3, 2, 1, NODIR, CLR_BLUE,
        SPE_CREATE_GRASS),
SPELL("summon animal",    "blood-stained",
      P_EVOCATION_SPELL,   15,  4, 3, 1, NODIR, CLR_RED,
        SPE_SUMMON_ANIMAL),
SPELL("create trees",     "smudged",
      P_EVOCATION_SPELL,   10,  5, 4, 1, NODIR, CLR_BRIGHT_GREEN,
        SPE_CREATE_TREES),
SPELL("stoneskin",        "moth-eaten",
      P_EVOCATION_SPELL,   15,  3, 4, 1, NODIR, CLR_MAGENTA,
        SPE_STONESKIN),
SPELL("change metal to wood", "faded",
      P_EVOCATION_SPELL,   10,  6, 5, 1, IMMEDIATE, HI_SPIDER_SILK,
        SPE_CHANGE_METAL_TO_WOOD),
SPELL("summon elemental", "mysterious",
      P_EVOCATION_SPELL,    5,  8, 6, 1, NODIR, HI_STEEL,
        SPE_SUMMON_ELEMENTAL),
/* books with fixed descriptions
 */
SPELL("blank paper", "plain", P_NONE, 15, 0, 0, 0, 0, HI_PAPER,
        SPE_BLANK_PAPER),
MARKER(LAST_SPELL, SPE_BLANK_PAPER)
/* tribute book for 3.6 */

#undef SPELL
/* === OBJECT entries === */
OBJECT(OBJ("novel", "paperback"),
       BITS(0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, P_NONE, PAPER),
       0, SPBOOK_CLASS, 0, 0, 0, 20, 0, 0, 0, 1, 20, CLR_BRIGHT_BLUE,
        SPE_NOVEL),
/* a special, one of a kind, spellbook */
OBJECT(OBJ("Book of the Dead", "papyrus"),
       BITS(0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, P_NONE, PAPER),
       0, SPBOOK_CLASS, 0, 0, 20, 10000, 0, 0, 0, 7, 20, HI_PAPER,
        SPE_BOOK_OF_THE_DEAD),
/* wands ... */
/* sum of all probabilty values equals 3000 - wands
   of wishing are much less likely to randomly spawn
   versus all other kinds */

#define WAND(name,typ,prob,cost,mgc,dir,metal,color,sn) \
    OBJECT(OBJ(name, typ),                                              \
           BITS(0, 0, 1, 0, mgc, 1, 0, 0, 0, 0, dir, P_NONE, metal),    \
           0, WAND_CLASS, prob, 0, 7, cost, 0, 0, 0, 0, 30, color, sn)
/* === WAND entries === */
WAND("light",           "glass", 285, 100, 1, NODIR, GLASS, HI_GLASS,
        WAN_LIGHT),
WAND("secret door detection",
                        "balsa", 150, 150, 1, NODIR, WOOD, HI_WOOD,
        WAN_SECRET_DOOR_DETECTION),
WAND("enlightenment", "crystal",  58, 150, 1, NODIR, GLASS, HI_GLASS,
        WAN_ENLIGHTENMENT),
WAND("create monster",  "maple", 135, 200, 1, NODIR, WOOD, HI_WOOD,
        WAN_CREATE_MONSTER),
WAND("wishing",          "pine",   1, 500, 1, NODIR, WOOD, HI_WOOD,
        WAN_WISHING),
WAND("nothing",           "oak",  75, 500, 0, IMMEDIATE, WOOD, HI_WOOD,
        WAN_NOTHING),
WAND("striking",        "ebony", 225, 150, 1, IMMEDIATE, WOOD, HI_WOOD,
        WAN_STRIKING),
WAND("make invisible", "marble", 135, 150, 1, IMMEDIATE, MINERAL, HI_MINERAL,
        WAN_MAKE_INVISIBLE),
WAND("slow monster",      "tin", 150, 150, 1, IMMEDIATE, STEEL, HI_STEEL,
        WAN_SLOW_MONSTER),
WAND("speed monster",  "bronze", 150, 150, 1, IMMEDIATE, BRONZE, HI_BRONZE,
        WAN_SPEED_MONSTER),
WAND("undead turning", "copper", 150, 150, 1, IMMEDIATE, COPPER, HI_COPPER,
        WAN_UNDEAD_TURNING),
WAND("polymorph",      "silver", 135, 200, 1, IMMEDIATE, SILVER, HI_SILVER,
        WAN_POLYMORPH),
WAND("cancellation", "platinum", 135, 200, 1, IMMEDIATE, PLATINUM, HI_PLATINUM,
        WAN_CANCELLATION),
WAND("teleportation", "iridium", 135, 200, 1, IMMEDIATE, STEEL, HI_STEEL,
        WAN_TELEPORTATION),
WAND("opening",          "zinc",  75, 150, 1, IMMEDIATE, STEEL, HI_STEEL,
        WAN_OPENING),
WAND("locking",      "aluminum",  75, 150, 1, IMMEDIATE, STEEL, HI_STEEL,
        WAN_LOCKING),
WAND("probing",       "uranium",  90, 150, 1, IMMEDIATE, STEEL, HI_STEEL,
        WAN_PROBING),
WAND("digging",          "iron", 165, 150, 1, RAY, IRON,  HI_IRON,
        WAN_DIGGING),
WAND("magic missile",   "steel", 150, 150, 1, RAY, STEEL, HI_STEEL,
        WAN_MAGIC_MISSILE),
WAND("fire",        "hexagonal", 120, 175, 1, RAY, IRON,  HI_IRON,
        WAN_FIRE),
WAND("cold",            "short", 120, 175, 1, RAY, IRON,  HI_IRON,
        WAN_COLD),
WAND("sleep",           "runed", 150, 175, 1, RAY, IRON,  HI_IRON,
        WAN_SLEEP),
WAND("death",            "long",  16, 500, 1, RAY, IRON,  HI_IRON,
        WAN_DEATH),
WAND("lightning",      "curved", 120, 175, 1, RAY, IRON,  HI_IRON,
        WAN_LIGHTNING),
/* extra descriptions, shuffled into use at start of new game */
WAND(NoDes,             "forked",  0, 150, 1, 0, WOOD, HI_WOOD,
        WAN1),
WAND(NoDes,             "spiked",  0, 150, 1, 0, IRON, HI_IRON,
        WAN2),
WAND(NoDes,            "jeweled",  0, 150, 1, 0, IRON, HI_MINERAL,
        WAN3),
/* coins ... - so far, gold is all there is */

#undef WAND

#define COIN(name,prob,metal,worth,sn) \
    OBJECT(OBJ(name, NoDes),                                        \
           BITS(1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, P_NONE, metal),    \
           0, COIN_CLASS, prob, 0, 1, worth, 0, 0, 0, 0, 0,         \
           HI_GOLD, sn)
/* === COIN entries === */
COIN("gold piece", 1000, GOLD, 1,
        GOLD_PIECE),
/* gems ... - includes stones and rocks but not boulders */

#undef COIN

#define GEM(name,desc,prob,wt,gval,nutr,mohs,glass,color,sn) \
    OBJECT(OBJ(name, desc),                                             \
           BITS(0, 1, 0, 0, 0, 0, 0, 0, 0,                              \
                HARDGEM(mohs), 0, -P_SLING, glass),                     \
           0, GEM_CLASS, prob, 0, 1, gval, 3, 3, 0, 0, nutr, color, sn)
#define ROCK(name,desc,kn,prob,wt,gval,sdam,ldam,mgc,nutr,mohs,glass,color,sn) \
    OBJECT(OBJ(name, desc),                                             \
           BITS(kn, 1, 0, 0, mgc, 0, 0, 0, 0,                           \
                HARDGEM(mohs), 0, -P_SLING, glass),                     \
           0, GEM_CLASS, prob, 0, wt, gval, sdam, ldam, 0, 0, nutr,     \
           color, sn)
/* === GEM entries === */
GEM("dilithium crystal", "white",  2, 1, 4500, 15,  5, GEMSTONE, CLR_WHITE,
        DILITHIUM_CRYSTAL),
MARKER(FIRST_REAL_GEM, DILITHIUM_CRYSTAL)
GEM("diamond",           "white",  3, 1, 4000, 15, 10, GEMSTONE, CLR_WHITE,
        DIAMOND),
GEM("ruby",                "red",  4, 1, 3500, 15,  9, GEMSTONE, CLR_RED,
        RUBY),
GEM("jacinth",          "orange",  3, 1, 3250, 15,  9, GEMSTONE, CLR_ORANGE,
        JACINTH),
GEM("sapphire",           "blue",  4, 1, 3000, 15,  9, GEMSTONE, CLR_BLUE,
        SAPPHIRE),
GEM("black opal",        "black",  3, 1, 2500, 15,  8, GEMSTONE, CLR_BLACK,
        BLACK_OPAL),
GEM("emerald",           "green",  5, 1, 2500, 15,  8, GEMSTONE, CLR_GREEN,
        EMERALD),
GEM("turquoise",         "green",  6, 1, 2000, 15,  6, GEMSTONE, CLR_GREEN,
        TURQUOISE),
GEM("citrine",          "yellow",  4, 1, 1500, 15,  6, GEMSTONE, CLR_YELLOW,
        CITRINE),
GEM("aquamarine",        "green",  6, 1, 1500, 15,  8, GEMSTONE, CLR_GREEN,
        AQUAMARINE),
GEM("amber",   "yellowish brown",  8, 1, 1000, 15,  2, GEMSTONE, CLR_BROWN,
        AMBER),
GEM("topaz",   "yellowish brown", 10, 1,  900, 15,  8, GEMSTONE, CLR_BROWN,
        TOPAZ),
GEM("jet",               "black",  6, 1,  850, 15,  7, GEMSTONE, CLR_BLACK,
        JET),
GEM("opal",              "white", 12, 1,  800, 15,  6, GEMSTONE, CLR_WHITE,
        OPAL),
GEM("chrysoberyl",      "yellow",  8, 1,  700, 15,  5, GEMSTONE, CLR_YELLOW,
        CHRYSOBERYL),
GEM("garnet",              "red", 12, 1,  700, 15,  7, GEMSTONE, CLR_RED,
        GARNET),
GEM("amethyst",         "violet", 14, 1,  600, 15,  7, GEMSTONE, CLR_MAGENTA,
        AMETHYST),
GEM("jasper",              "red", 15, 1,  500, 15,  7, GEMSTONE, CLR_RED,
        JASPER),
GEM("fluorite",         "violet", 15, 1,  400, 15,  4, GEMSTONE, CLR_MAGENTA,
        FLUORITE),
GEM("obsidian",          "black",  9, 1,  200, 15,  6, GEMSTONE, CLR_BLACK,
        OBSIDIAN),
GEM("agate",            "orange", 12, 1,  200, 15,  6, GEMSTONE, CLR_ORANGE,
        AGATE),
GEM("jade",              "green", 10, 1,  300, 15,  6, GEMSTONE, CLR_GREEN,
        JADE),
MARKER(LAST_REAL_GEM, JADE)
GEM("worthless piece of white glass", "white",
    74, 1, 0, 6, 5, GLASS, CLR_WHITE,
        WORTHLESS_WHITE_GLASS),
MARKER(FIRST_GLASS_GEM, WORTHLESS_WHITE_GLASS)
GEM("worthless piece of blue glass", "blue",
    74, 1, 0, 6, 5, GLASS, CLR_BLUE,
        WORTHLESS_BLUE_GLASS),
GEM("worthless piece of red glass", "red",
    73, 1, 0, 6, 5, GLASS, CLR_RED,
        WORTHLESS_RED_GLASS),
GEM("worthless piece of yellowish brown glass", "yellowish brown",
    73, 1, 0, 6, 5, GLASS, CLR_BROWN,
        WORTHLESS_YELLOWBROWN_GLASS),
GEM("worthless piece of orange glass", "orange",
    73, 1, 0, 6, 5, GLASS, CLR_ORANGE,
        WORTHLESS_ORANGE_GLASS),
GEM("worthless piece of yellow glass", "yellow",
    73, 1, 0, 6, 5, GLASS, CLR_YELLOW,
        WORTHLESS_YELLOW_GLASS),
GEM("worthless piece of black glass", "black",
    73, 1, 0, 6, 5, GLASS, CLR_BLACK,
        WORTHLESS_BLACK_GLASS),
GEM("worthless piece of green glass", "green",
    74, 1, 0, 6, 5, GLASS, CLR_GREEN,
        WORTHLESS_GREEN_GLASS),
GEM("worthless piece of violet glass", "violet",
    74, 1, 0, 6, 5, GLASS, CLR_MAGENTA,
        WORTHLESS_VIOLET_GLASS),
MARKER(LAST_GLASS_GEM, WORTHLESS_VIOLET_GLASS)
/* Placement note: there is a wishable subrange for
 * "gray stones" in the o_ranges[] array in objnam.c
 * that is currently everything between luckstones and flint
 * (inclusive).
 */
/* === ROCK entries === */
ROCK("luckstone", "gray",     0,  10,  10, 60, 3, 3, 1, 10, 7, MINERAL, HI_MINERAL,
        LUCKSTONE),
ROCK("loadstone", "gray",     0,  10, 500,  1, 3, 3, 1, 10, 6, MINERAL, HI_MINERAL,
        LOADSTONE),
ROCK("touchstone", "gray",    0,   8,  10, 45, 3, 3, 1, 10, 6, MINERAL, HI_MINERAL,
        TOUCHSTONE),
ROCK("flint", "gray",         0,  10,   2,  1, 6, 6, 0, 10, 7, MINERAL, HI_MINERAL,
        FLINT),
ROCK("sling bullet", "shiny", 0,  30,   6, 10, 6, 8, 0, 10, 7, IRON, HI_IRON,
        SLING_BULLET),
ROCK("rock", NoDes,            1, 100,  10,  0, 3, 3, 0, 10, 7, MINERAL, HI_MINERAL,
        ROCK),
/* miscellaneous ... */
/* Note: boulders and rocks are not normally created at random; the
 * probabilities only come into effect when you try to polymorph them.
 * Boulders weigh more than MAX_CARR_CAP; statues use corpsenm to take
 * on a specific type and may act as containers (both affect weight).
 */

#undef GEM
#undef ROCK
/* === OBJECT entries === */
OBJECT(OBJ("boulder", NoDes),
       BITS(1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, P_NONE, MINERAL), 0,
       ROCK_CLASS, 100, 0, 6000, 0, 20, 20, 0, 0, 2000, HI_MINERAL,
        BOULDER),
OBJECT(OBJ("statue", NoDes),
       BITS(1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, P_NONE, MINERAL), 0,
       ROCK_CLASS, 900, 0, 2500, 0, 20, 20, 0, 0, 2500, CLR_WHITE,
        STATUE),
OBJECT(OBJ("heavy iron ball", NoDes),
       BITS(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, WHACK, P_FLAIL, IRON), 0,
       BALL_CLASS, 1000, 0, 480, 10, 25, 25, 0, 0, 200, HI_IRON,
        HEAVY_IRON_BALL),
/* +d4 when "very heavy" */
OBJECT(OBJ("iron chain", NoDes),
       BITS(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, WHACK, P_NONE, IRON), 0,
       CHAIN_CLASS, 1000, 0, 120, 0, 4, 4, 0, 0, 200, HI_IRON,
        IRON_CHAIN),
/* +1 both l & s */
/* Venom is normally a transitory missile (spit by various creatures)
 * but can be wished for in wizard mode so could occur in bones data.
 */
OBJECT(OBJ("blinding venom", "splash of venom"),
       BITS(0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, P_NONE, LIQUID), 0,
       VENOM_CLASS, 500, 0, 1, 0, 0, 0, 0, 0, 0, HI_VEGGY,
        BLINDING_VENOM),
OBJECT(OBJ("acid venom", "splash of venom"),
       BITS(0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, P_NONE, LIQUID), 0,
       VENOM_CLASS, 500, 0, 1, 0, 6, 6, 0, 0, 0, HI_ORGANIC,
        ACID_VENOM),
OBJECT(OBJ("snowball", NoDes),
       BITS(1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, P_NONE, LIQUID), 0,
       VENOM_CLASS, 500, 0, 1, 0, 6, 6, 0, 0, 0, HI_GLASS,
        SNOWBALL),
OBJECT(OBJ("ball of webbing", NoDes),
       BITS(1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, P_NONE, LIQUID), 0,
       VENOM_CLASS, 500, 0, 1, 0, 0, 0, 0, 0, 0, HI_SPIDER_SILK,
        BALL_OF_WEBBING),
OBJECT(OBJ("barbed needle", NoDes),
       BITS(1, 1, 0, 0, 0, 0, 0, 1, 0, 0, PIERCE, P_NONE, WOOD), 0,
       VENOM_CLASS, 500, 0, 1, 0, 6, 6, 0, 0, 0, HI_WOOD,
        BARBED_NEEDLE),

#if defined(OBJECTS_DESCR_INIT) || defined(OBJECTS_INIT)
/* fencepost, the deadly Array Terminator -- name [1st arg] *must* be NULL */
OBJECT(OBJ(NoDes, NoDes),
       BITS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, P_NONE, 0), 0,
       ILLOBJ_CLASS, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#undef BITS
#endif

#undef OBJ
#undef OBJECT
#undef MARKER
#undef HARDGEM
#undef NoDes

/*objects.h*/
