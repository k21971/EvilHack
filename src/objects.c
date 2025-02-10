/* NetHack 3.6	objects.c	$NHDT-Date: 1535422421 2018/08/28 02:13:41 $  $NHDT-Branch: NetHack-3.6.2-beta01 $:$NHDT-Revision: 1.51 $ */
/* Copyright (c) Mike Threepoint, 1989.                           */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * The data in this file is processed twice, to construct two arrays.
 * On the first pass, only object name and object description matter.
 * On the second pass, all object-class fields except those two matter.
 * 2nd pass is a recursive inclusion of this file, not a 2nd compilation.
 * The name/description array is also used by makedefs and lev_comp.
 *
 * #ifndef OBJECTS_PASS_2_
 * # define OBJECT(name,desc,foo,bar,glorkum) name,desc
 * struct objdescr obj_descr[] =
 * #else
 * # define OBJECT(name,desc,foo,bar,glorkum) foo,bar,glorkum
 * struct objclass objects[] =
 * #endif
 * {
 *   { OBJECT("strange object",NULL, 1,2,3) },
 *   { OBJECT("arrow","pointy stick", 4,5,6) },
 *   ...
 *   { OBJECT(NULL,NULL, 0,0,0) }
 * };
 * #define OBJECTS_PASS_2_
 * #include "objects.c"
 */

/* *INDENT-OFF* */
/* clang-format off */

#ifndef OBJECTS_PASS_2_
/* first pass */
struct monst { struct monst *dummy; };  /* lint: struct obj's union */
#include "config.h"
#include "obj.h"
#include "objclass.h"
#include "prop.h"
#include "skills.h"

#else /* !OBJECTS_PASS_2_ */
/* second pass */
#include "color.h"
#define COLOR_FIELD(X) X,
#endif /* !OBJECTS_PASS_2_ */

/* objects have symbols: ) [ = " ( % ! ? + / $ * ` 0 _ . */

/*
 *      Note:  OBJ() and BITS() macros are used to avoid exceeding argument
 *      limits imposed by some compilers.  The ctnr field of BITS currently
 *      does not map into struct objclass, and is ignored in the expansion.
 *      The 0 in the expansion corresponds to oc_pre_discovered, which is
 *      set at run-time during role-specific character initialization.
 */

#ifndef OBJECTS_PASS_2_
/* first pass -- object descriptive text */
#define OBJ(name,desc)  name, desc
#define OBJECT(obj,bits,prp,sym,prob,dly,wt, \
               cost,sdam,ldam,oc1,oc2,nut,color)  { obj }
#define None (char *) 0 /* less visual distraction for 'no description' */

NEARDATA struct objdescr obj_descr[] =
#else
/* second pass -- object definitions */
#define BITS(nmkn,mrg,uskn,ctnr,mgc,chrg,uniq,nwsh,big,tuf,dir,sub,mtrl) \
  nmkn,mrg,uskn,0,mgc,chrg,uniq,nwsh,big,tuf,dir,mtrl,sub /*SCO cpp fodder*/
#define OBJECT(obj,bits,prp,sym,prob,dly,wt,cost,sdam,ldam,oc1,oc2,nut,color) \
  { 0, 0, (char *) 0, bits, prp, sym, dly, COLOR_FIELD(color) prob, wt, \
    cost, sdam, ldam, oc1, oc2, nut }
#ifndef lint
#define HARDGEM(n) (n >= 8)
#else
#define HARDGEM(n) (0)
#endif

NEARDATA struct objclass objects[] =
#endif
{
/* dummy object[0] -- description [2nd arg] *must* be NULL */
OBJECT(OBJ("strange object", None),
       BITS(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, P_NONE, 0),
       0, ILLOBJ_CLASS, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),

/* weapons ... */
#define WEAPON(name,desc,kn,mg,mgc,bi,prob,wt,            \
               cost,sdam,ldam,hitbon,typ,sub,metal,color) \
    OBJECT(OBJ(name,desc),                                           \
           BITS(kn, mg, 1, 0, mgc, 1, 0, 0, bi, 0, typ, sub, metal), \
           0, WEAPON_CLASS, prob, 0, wt,                             \
           cost, sdam, ldam, hitbon, 0, wt, color)
#define PROJECTILE(name,desc,kn,prob,wt,                  \
                   cost,sdam,ldam,hitbon,metal,sub,color) \
    OBJECT(OBJ(name,desc),                                          \
           BITS(kn, 1, 1, 0, 0, 1, 0, 0, 0, 0, PIERCE, sub, metal), \
           0, WEAPON_CLASS, prob, 0, wt,                            \
           cost, sdam, ldam, hitbon, 0, wt, color)
#define BOW(name,desc,kn,bi,prob,wt,cost,hitbon,metal,sub,color) \
    OBJECT(OBJ(name,desc),                                          \
           BITS(kn, 0, 1, 0, 0, 1, 0, 0, bi, 0, 0, sub, metal),     \
           0, WEAPON_CLASS, prob, 0, wt,                            \
           cost, 2, 2, hitbon, 0, wt, color)

/* Note: for weapons that don't do an even die of damage (ex. 2-7 or 3-18)
   the extra damage is added on in weapon.c, not here! */

#define P PIERCE
#define S SLASH
#define B WHACK

/* missiles; materiel reflects the arrowhead, not the shaft */
PROJECTILE("arrow", None,
           1, 67, 1, 2, 6, 6, 0,        IRON, -P_BOW, HI_METAL),
PROJECTILE("elven arrow", "runed arrow",
           0, 20, 1, 2, 7, 6, 0,        WOOD, -P_BOW, HI_WOOD),
PROJECTILE("orcish arrow", "crude arrow",
           0, 20, 1, 2, 5, 6, 0,        IRON, -P_BOW, CLR_BLACK),
PROJECTILE("dark elven arrow", "etched arrow",
           0,  3, 1, 5, 7, 6, 0,        ADAMANTINE, -P_BOW, CLR_BLACK),
PROJECTILE("ya", "long arrow",
           0, 15, 1, 4, 7, 7, 1,        WOOD, -P_BOW, HI_WOOD),
PROJECTILE("crossbow bolt", None,
           1, 55, 1, 2, 4, 6, 0,        IRON, -P_CROSSBOW, HI_METAL),
PROJECTILE("dark elven crossbow bolt", "etched crossbow bolt",
           0,  5, 1, 5, 4, 6, 0,        ADAMANTINE, -P_CROSSBOW, CLR_BLACK),

/* missiles that don't use a launcher */
WEAPON("dart", None,
       1, 1, 0, 0, 60,   1,   2,  3,  2, 0, P,   -P_DART, IRON, HI_METAL),
WEAPON("shuriken", "throwing star",
       0, 1, 0, 0, 35,   1,   5,  8,  6, 2, P,   -P_SHURIKEN, IRON, HI_METAL),
WEAPON("boomerang", None,
       1, 1, 0, 0, 15,   5,  20,  9,  9, 0, B,   -P_BOOMERANG, WOOD, HI_WOOD),

/* spears [note: javelin used to have a separate skill from spears,
   because the latter are primarily stabbing weapons rather than
   throwing ones; but for playability, they've been merged together
   under spear skill and spears can now be thrown like javelins] */
WEAPON("spear", None,
       1, 1, 0, 0, 52,  30,   3,  6,  8, 0, P,   P_SPEAR, IRON, HI_METAL),
WEAPON("elven spear", "runed spear",
       0, 1, 0, 0, 10,  11,   3,  7,  8, 0, P,   P_SPEAR, WOOD, HI_WOOD),
WEAPON("orcish spear", "crude spear",
       0, 1, 0, 0, 13,  30,   3,  5,  8, 0, P,   P_SPEAR, IRON, CLR_BLACK),
WEAPON("dwarvish spear", "stout spear",
       0, 1, 0, 0, 12,  30,   3,  8,  8, 0, P,   P_SPEAR, IRON, HI_METAL),
WEAPON("dark elven spear", "etched spear",
       0, 1, 0, 0,  5,  15,  10,  7,  8, 0, P,   P_SPEAR, ADAMANTINE, CLR_BLACK),
WEAPON("javelin", "throwing spear",
       0, 1, 0, 0, 10,  20,   3,  6,  6, 0, P,   P_SPEAR, IRON, HI_METAL),
/* Base weapon for the artifact weapon Xiuhcoatl */
WEAPON("atlatl", None,
       1, 0, 0, 0,  0,  30,  10,  8, 12, 2, P,   P_SPEAR, WOOD, CLR_BLACK),
/* --------------------------------------------- */

/* spearish; doesn't stack, not intended to be thrown */
WEAPON("trident", None,
       1, 0, 0, 0,  8,  20,   5,  6,  4, 0, P,   P_TRIDENT, IRON, HI_METAL),
        /* +1 small, +2d4 large */

/* blades; all stack */
WEAPON("dagger", None,
       1, 1, 0, 0, 33,  10,   4,  4,  3, 2, P,   P_DAGGER, IRON, HI_METAL),
WEAPON("elven dagger", "runed dagger",
       0, 1, 0, 0, 10,   4,   4,  5,  3, 2, P,   P_DAGGER, WOOD, HI_WOOD),
WEAPON("orcish dagger", "crude dagger",
       0, 1, 0, 0, 12,  10,   4,  3,  3, 2, P,   P_DAGGER, IRON, CLR_BLACK),
WEAPON("dark elven dagger", "etched dagger",
       0, 1, 0, 0,  2,   8,  10,  5,  3, 2, P,   P_DAGGER, ADAMANTINE, CLR_BLACK),
WEAPON("athame", None,
       1, 1, 0, 0,  0,  10,   4,  4,  3, 2, S,   P_DAGGER, STEEL, HI_METAL),
WEAPON("scalpel", None,
       1, 1, 0, 0,  0,   5,   6,  3,  3, 2, S,   P_DAGGER, STEEL, HI_METAL),
WEAPON("knife", None,
       1, 1, 0, 0, 20,   5,   4,  3,  2, 0, S, P_DAGGER, IRON, HI_METAL),
WEAPON("stiletto", None,
       1, 1, 0, 0,  5,   5,   4,  3,  2, 0, P, P_DAGGER, IRON, HI_METAL),
/* 3.6: worm teeth and crysknives now stack;
   when a stack of teeth is enchanted at once, they fuse into one crysknife;
   when a stack of crysknives drops, the whole stack reverts to teeth */
WEAPON("worm tooth", None,
       1, 1, 0, 0,  0,  20,   2,  2,  2, 0, P,   P_DAGGER, BONE, CLR_WHITE),
WEAPON("crysknife", None,
       1, 1, 0, 0,  0,  20, 100, 10, 10, 3, P,   P_DAGGER, BONE, CLR_WHITE),

/* axes */
WEAPON("axe", None,
       1, 0, 0, 0, 40,  60,   8,  6,  4, 0, S,   P_AXE, IRON, HI_METAL),
WEAPON("dwarvish bearded axe", "broad bearded axe",
       0, 0, 0, 0, 20,  70,  25,  8, 10, 0, S,   P_AXE, IRON, HI_METAL),
WEAPON("battle-axe", "double-headed axe",       /* "double-bitted"? */
       0, 0, 0, 1, 10, 120,  40,  8,  6, 0, S,   P_AXE, IRON, HI_METAL),

/* swords */
WEAPON("short sword", None,
       1, 0, 0, 0,  8,  30,  10,  6,  8, 0, P,   P_SHORT_SWORD, IRON, HI_METAL),
WEAPON("elven short sword", "runed short sword",
       0, 0, 0, 0,  2,  11,  10,  8,  8, 0, P,   P_SHORT_SWORD, WOOD, HI_WOOD),
WEAPON("orcish short sword", "crude short sword",
       0, 0, 0, 0,  3,  30,  10,  5,  8, 0, P,   P_SHORT_SWORD, IRON, CLR_BLACK),
WEAPON("dwarvish short sword", "broad short sword",
       0, 0, 0, 0,  2,  30,  10,  7,  8, 0, P,   P_SHORT_SWORD, IRON, HI_METAL),
WEAPON("dark elven short sword", "etched short sword",
       0, 0, 0, 0,  2,  15,  20,  8,  8, 0, P,   P_SHORT_SWORD, ADAMANTINE, CLR_BLACK),
WEAPON("scimitar", "curved sword",
       0, 0, 0, 0, 15,  40,  15,  8,  8, 0, S,   P_SABER, IRON, HI_METAL),
WEAPON("orcish scimitar", "crude curved sword",
       0, 0, 0, 0, 15,  40,  15,  6,  8, 0, S,   P_SABER, IRON, CLR_BLACK),
WEAPON("saber", None,
       1, 0, 0, 0,  6,  40,  75,  8,  8, 0, S,   P_SABER, IRON, HI_METAL),
WEAPON("broadsword", None,
       1, 0, 0, 0,  8,  70,  10,  4,  6, 0, S,   P_BROAD_SWORD, IRON, HI_METAL),
        /* +d4 small, +1 large */
WEAPON("elven broadsword", "runed broadsword",
       0, 0, 0, 0,  4,  26,  10,  6,  6, 0, S,   P_BROAD_SWORD, WOOD, HI_WOOD),
        /* +d4 small, +1 large */
WEAPON("dark elven broadsword", "etched broadsword",
       0, 0, 0, 0,  1,  45,  20,  6,  6, 0, S,   P_BROAD_SWORD, ADAMANTINE, CLR_BLACK),
        /* +d4 small, +1 large */
WEAPON("long sword", None,
       1, 0, 0, 0, 50,  40,  15,  8, 12, 0, S,   P_LONG_SWORD, IRON, HI_METAL),
WEAPON("elven long sword", "runed long sword",
       0, 0, 0, 0, 10,  15,  40, 10, 12, 0, S,   P_LONG_SWORD, WOOD, HI_WOOD),
WEAPON("orcish long sword", "crude long sword",
       0, 0, 0, 0, 10,  40,  12,  8, 10, 0, S,   P_LONG_SWORD, IRON, CLR_BLACK),
WEAPON("dark elven long sword", "etched long sword",
       0, 0, 0, 0,  2,  25,  80, 10, 12, 0, S,   P_LONG_SWORD, ADAMANTINE, CLR_BLACK),
WEAPON("two-handed sword", None,
       1, 0, 0, 1, 22, 150,  50, 12,  6, 0, S,   P_TWO_HANDED_SWORD,
                                                            IRON, HI_METAL),
        /* +2d6 large */
WEAPON("katana", "samurai sword",
       0, 0, 0, 0,  4,  40,  80, 10, 12, 1, S,   P_LONG_SWORD, STEEL, HI_METAL),
/* special swords set up for artifacts */
WEAPON("tsurugi", "long samurai sword",
       0, 0, 0, 1,  0,  60, 500, 16,  8, 2, S,   P_TWO_HANDED_SWORD,
                                                            STEEL, HI_METAL),
        /* +2d6 large */
WEAPON("runesword", "runed broadsword",
       0, 0, 0, 0,  0,  40, 300,  4,  6, 0, S,   P_BROAD_SWORD, IRON, CLR_BLACK),
        /* +d4 small, +1 large; Stormbringer: +5d2 +d8 from level drain */

/* polearms */
/* spear-type */
WEAPON("partisan", "vulgar polearm",
       0, 0, 0, 1,  5,  80,  10,  6,  6, 0, P,   P_POLEARMS, IRON, HI_METAL),
        /* +1 large */
WEAPON("ranseur", "hilted polearm",
       0, 0, 0, 1,  5,  50,   6,  4,  4, 0, P,   P_POLEARMS, IRON, HI_METAL),
        /* +d4 both */
WEAPON("spetum", "forked polearm",
       0, 0, 0, 1,  5,  50,   5,  6,  6, 0, P,   P_POLEARMS, IRON, HI_METAL),
        /* +1 small, +d6 large */
WEAPON("glaive", "single-edged polearm",
       0, 0, 0, 1,  8,  75,   6,  6, 10, 0, S,   P_POLEARMS, IRON, HI_METAL),
WEAPON("lance", None,
       1, 0, 0, 0,  4, 180,  10,  6,  8, 0, P,   P_LANCE, STEEL, HI_METAL),
        /* +2d10 when jousting with lance as primary weapon */
/* axe-type */
WEAPON("halberd", "angled poleaxe",
       0, 0, 0, 1,  8, 150,  10, 10,  6, 0, S, P_POLEARMS, IRON, HI_METAL),
        /* +1d6 large */
WEAPON("bardiche", "long poleaxe",
       0, 0, 0, 1,  4, 120,   7,  4,  4, 0, S,   P_POLEARMS, IRON, HI_METAL),
        /* +1d4 small, +2d4 large */
WEAPON("voulge", "pole cleaver",
       0, 0, 0, 1,  4, 125,   5,  4,  4, 0, S,   P_POLEARMS, IRON, HI_METAL),
        /* +d4 both */
WEAPON("dwarvish mattock", "broad pick",
       0, 0, 0, 1, 13, 120,  50, 12,  8, -1, B,  P_PICK_AXE, IRON, HI_METAL),
/* curved/hooked */
WEAPON("fauchard", "pole sickle",
       0, 0, 0, 1,  6,  60,   5,  6,  8, 0, S, P_POLEARMS, IRON, HI_METAL),
WEAPON("guisarme", "pruning hook",
       0, 0, 0, 1,  6,  80,   5,  4,  8, 0, S,   P_POLEARMS, IRON, HI_METAL),
        /* +1d4 small */
WEAPON("bill-guisarme", "hooked polearm",
       0, 0, 0, 1,  4, 120,   7,  4, 10, 0, S, P_POLEARMS, IRON, HI_METAL),
        /* +1d4 small */
/* other */
WEAPON("lucern hammer", "pronged polearm",
       0, 0, 0, 1,  5, 150,   7,  4,  6, 0, B, P_POLEARMS, IRON, HI_METAL),
        /* +1d4 small */
WEAPON("bec de corbin", "beaked polearm",
       0, 0, 0, 1,  4, 100,   8,  8,  6, 0, P, P_POLEARMS, IRON, HI_METAL),

/* bludgeons */
WEAPON("mace", None,
       1, 0, 0, 0, 30,  30,   5,  6,  6, 0, B,   P_MACE, IRON, HI_METAL),
WEAPON("dark elven mace", "etched mace",
       0, 0, 0, 0,  3,  20,  10,  6,  8, 0, B,   P_MACE, ADAMANTINE, CLR_BLACK),
WEAPON("heavy mace", None,
       1, 0, 0, 0, 15,  50,  10, 10, 10, 0, B,   P_MACE, IRON, HI_METAL),
WEAPON("dark elven heavy mace", "etched heavy mace",
       0, 0, 0, 0,  2,  35,  20, 10, 12, 0, B,   P_MACE, ADAMANTINE, CLR_BLACK),
        /* +1 small */
/* placeholder for certain special weapons; does not spawn randomly */
WEAPON("rod", "ornate mace",
       0, 0, 0, 0,  0,  40, 250,  6,  8, 0, B,   P_MACE, GEMSTONE, CLR_RED),
WEAPON("morning star", None,
       1, 0, 0, 0, 12, 120,  10,  4,  6, 0, B,   P_MACE, IRON, HI_METAL),
        /* +d4 small, +1 large */
WEAPON("orcish morning star", "crude morning star",
       0, 0, 0, 0,  8, 120,  10,  4,  6, 0, B,   P_MACE, IRON, CLR_BLACK),
        /* +d4 small, +1 large */
WEAPON("war hammer", None,
       1, 0, 0, 0, 15,  50,   5,  4,  4, 0, B,   P_HAMMER, IRON, HI_METAL),
        /* +1 small */
WEAPON("heavy war hammer", None,
       1, 0, 0, 0, 10,  60,  10,  8, 10, 0, B,   P_HAMMER, IRON, HI_METAL),
WEAPON("club", None,
       1, 0, 0, 0, 12,  30,   3,  6,  3, 0, B,   P_CLUB, WOOD, HI_WOOD),
WEAPON("rubber hose", None,
       1, 0, 0, 0,  0,  20,   3,  4,  3, 0, B,   P_WHIP, PLASTIC, CLR_BROWN),
WEAPON("quarterstaff", "staff",
       0, 0, 0, 1, 11,  40,   5,  6,  6, 0, B,   P_QUARTERSTAFF, WOOD, HI_WOOD),
WEAPON("staff of divination", "wormwood staff",
       0, 0, 1, 1,  5,  40, 400,  6,  6, 0, B,   P_QUARTERSTAFF, WOOD, HI_WOOD),
WEAPON("staff of healing", "twisted staff",
       0, 0, 1, 1,  5,  40, 400,  6,  6, 0, B,   P_QUARTERSTAFF, WOOD, HI_WOOD),
WEAPON("staff of holiness", "bone-carved staff",
       0, 0, 1, 1,  5,  40, 400,  6,  6, 0, B,   P_QUARTERSTAFF, BONE, CLR_WHITE),
WEAPON("staff of matter", "etched staff",
       0, 0, 1, 1,  5,  30, 450,  6,  6, 0, B,   P_QUARTERSTAFF, ADAMANTINE, HI_SILVER),
WEAPON("staff of escape", "darkwood staff",
       0, 0, 1, 1,  5,  40, 400,  6,  6, 0, B,   P_QUARTERSTAFF, WOOD, CLR_BLACK),
WEAPON("staff of war", "ironshod staff",
       0, 0, 1, 1,  5,  50, 425,  6,  6, 2, B,   P_QUARTERSTAFF, IRON, HI_METAL),
WEAPON("staff of evocation", "gnarled staff",
       0, 0, 1, 1,  5,  40, 400,  6,  6, 1, B,   P_QUARTERSTAFF, WOOD, HI_WOOD),
/* template for the Staff of the Archmagi */
WEAPON("ashwood staff", None,
       0, 0, 1, 1,  0,  40, 600,  8, 10, 0, B,   P_QUARTERSTAFF, WOOD, CLR_WHITE),
/* two-piece */
WEAPON("aklys", "thonged club",
       0, 0, 0, 0,  8,  15,   4,  6,  3, 0, B,   P_CLUB, IRON, HI_METAL),
WEAPON("flail", None,
       1, 0, 0, 0, 40,  15,   4,  6,  4, 0, B,   P_FLAIL, IRON, HI_METAL),
        /* +1 small, +1d4 large */
/* template for Yeenoghu's artifact weapon; does not spawn randomly */
WEAPON("triple-headed flail", None,
       1, 0, 0, 1,  0, 150,  30, 12,  6, 3, B,   P_FLAIL, BONE, CLR_WHITE),
        /* +d4 small, +3d6 large */

/* misc */
WEAPON("bullwhip", None,
       1, 0, 0, 0,  2,  20,   4,  2,  1, 0, 0,   P_WHIP, LEATHER, CLR_BROWN),

/* bows */
BOW("bow", None,                    1, 1, 24, 30,  60, 0, WOOD, P_BOW, HI_WOOD),
BOW("elven bow", "runed bow",       0, 1, 12, 30,  60, 0, WOOD, P_BOW, HI_WOOD),
BOW("orcish bow", "crude bow",      0, 1, 12, 30,  60, 0, WOOD, P_BOW, CLR_BLACK),
BOW("dark elven bow", "etched bow", 0, 1,  3, 40, 100, 0, ADAMANTINE, P_BOW, CLR_BLACK),
BOW("yumi", "long bow",             0, 1,  0, 30,  60, 0, WOOD, P_BOW, HI_WOOD),
BOW("sling", None,                  1, 0, 40,  3,  20, 0, LEATHER, P_SLING, HI_LEATHER),
BOW("dark elven hand crossbow", "etched crossbow",
    0, 0,  5, 25,  80, 0, ADAMANTINE, P_CROSSBOW, CLR_BLACK),
BOW("crossbow", None,               1, 1, 45, 50,  40, 0, WOOD, P_CROSSBOW, HI_WOOD),

#undef P
#undef S
#undef B

#undef WEAPON
#undef PROJECTILE
#undef BOW

/* armor ... */
        /* IRON denotes ferrous metals.
         * STEEL denotes stainless steel.
         * Only IRON weapons and armor can rust.
         * Only COPPER and BRONZE (including brass) corrodes.
         * Some creatures are vulnerable to SILVER.
         */
#define ARMOR(name,desc,kn,mgc,blk,power,prob,delay,wt,  \
              cost,ac,can,sub,metal,c)                   \
    OBJECT(OBJ(name, desc),                                         \
           BITS(kn, 0, 1, 0, mgc, 1, 0, 0, blk, 0, 0, sub, metal),  \
           power, ARMOR_CLASS, prob, delay, wt,                     \
           cost, 0, 0, 10 - ac, can, wt, c)
#define HELM(name,desc,kn,mgc,power,prob,delay,wt,cost,ac,can,metal,c)  \
    ARMOR(name, desc, kn, mgc, 0, power, prob, delay, wt,  \
          cost, ac, can, ARM_HELM, metal, c)
#define CLOAK(name,desc,kn,mgc,power,prob,delay,wt,cost,ac,can,metal,c)  \
    ARMOR(name, desc, kn, mgc, 0, power, prob, delay, wt,  \
          cost, ac, can, ARM_CLOAK, metal, c)
#define SHIELD(name,desc,kn,mgc,blk,power,prob,delay,wt,cost,ac,can,metal,c) \
    ARMOR(name, desc, kn, mgc, blk, power, prob, delay, wt, \
          cost, ac, can, ARM_SHIELD, metal, c)
#define GLOVES(name,desc,kn,mgc,power,prob,delay,wt,cost,ac,can,metal,c)  \
    ARMOR(name, desc, kn, mgc, 0, power, prob, delay, wt,  \
          cost, ac, can, ARM_GLOVES, metal, c)
#define BOOTS(name,desc,kn,mgc,power,prob,delay,wt,cost,ac,can,metal,c)  \
    ARMOR(name, desc, kn, mgc, 0, power, prob, delay, wt,  \
          cost, ac, can, ARM_BOOTS, metal, c)

/* helmets */
HELM("dunce cap", "conical hat",
     0, 1,           0,  3, 1,  4,  1, 10, 0, CLOTH, CLR_BLUE),
HELM("cornuthaum", "conical hat",
     0, 1, CLAIRVOYANT,  3, 1,  4, 80, 10, 1, CLOTH, CLR_BLUE),
        /* name coined by devteam; confers clairvoyance for wizards,
           blocks clairvoyance if worn by role other than wizard */
HELM("fedora", None,
     1, 0,           0,  0, 0,  3,  1, 10, 0, CLOTH, CLR_BROWN),
HELM("toque", None,
     1, 0,           0,  0, 0,  3,  1, 10, 0, CLOTH, CLR_ORANGE),
HELM("elven helm", "hat",
     0, 0,           0,  5, 1,  3,  8,  9, 0, LEATHER, HI_LEATHER),
HELM("dark elven helm", "spider silk hat",
     0, 0,           0,  1, 1,  2, 16,  8, 0, SPIDER_SILK, CLR_GRAY),
HELM("orcish helm", "skull cap",
     0, 0,           0,  6, 1, 30, 10,  9, 0, IRON, CLR_BLACK),
HELM("dwarvish helm", "hard hat",
     0, 0,           0,  6, 1, 40, 20,  8, 0, IRON, HI_METAL),
HELM("dented pot", None,
     1, 0,           0,  2, 0, 10,  8,  9, 0, IRON, CLR_BLACK),
HELM("helm of brilliance", "crystal helmet",
     0, 1,           0,  3, 1, 40, 50,  9, 0, GLASS, CLR_WHITE),
/* with shuffled appearances... */
HELM("helmet", "plumed helmet",
     0, 0,           0, 10, 1, 30, 10,  9, 0, IRON, HI_METAL),
HELM("helm of caution", "etched helmet",
     0, 1,     WARNING,  3, 1, 50, 50,  9, 0, IRON, CLR_GREEN),
HELM("helm of opposite alignment", "crested helmet",
     0, 1,           0,  6, 1, 50, 50,  9, 0, IRON, HI_METAL),
HELM("helm of speed", "winged helmet",
     0, 1,        FAST,  2, 1, 50, 50,  9, 0, STEEL, HI_GOLD),
HELM("helm of telepathy", "visored helmet",
     0, 1,     TELEPAT,  2, 1, 50, 50,  9, 0, IRON, HI_METAL),

/* suits of armor */
/*
 * There is code in obj.h that assumes that the order of the dragon scales is
 * the same as order of dragons defined in monst.c.
 */
#define DRGN_SCALES(name,mgc,power,cost,color)  \
    ARMOR(name, None, 1, mgc, 1, power, 0, 5, 40,  \
          cost, 7, 0, ARM_CLOAK, DRAGON_HIDE, color)
/* For now, only dragons leave these. */
/* 3.4.1: dragon scales left classified as "non-magic"; they confer magical
   properties but are produced "naturally"; affects use as polypile fodder */
DRGN_SCALES("gray dragon scales",       0, ANTIMAGIC,         700, CLR_GRAY),
DRGN_SCALES("silver dragon scales",     0, REFLECTING,        700, DRAGON_SILVER),
DRGN_SCALES("shimmering dragon scales", 0, DISPLACED,         700, CLR_CYAN),
DRGN_SCALES("red dragon scales",        0, FIRE_RES,          500, CLR_RED),
DRGN_SCALES("white dragon scales",      0, COLD_RES,          500, CLR_WHITE),
DRGN_SCALES("orange dragon scales",     0, SLEEP_RES,         500, CLR_ORANGE),
DRGN_SCALES("black dragon scales",      0, DISINT_RES,        700, CLR_BLACK),
DRGN_SCALES("blue dragon scales",       0, SHOCK_RES,         500, CLR_BLUE),
DRGN_SCALES("green dragon scales",      0, POISON_RES,        500, CLR_GREEN),
DRGN_SCALES("gold dragon scales",       0, 0,                 500, HI_GOLD),
DRGN_SCALES("sea dragon scales",        0, MAGICAL_BREATHING, 500, HI_ZAP),
DRGN_SCALES("yellow dragon scales",     0, ACID_RES,          500, CLR_YELLOW),
DRGN_SCALES("shadow dragon scales",     0, ULTRAVISION,       700, CLR_BLACK),
DRGN_SCALES("celestial dragon scales",  0, FLYING,           1000, CLR_BRIGHT_MAGENTA),
DRGN_SCALES("chromatic dragon scales",  0, ANTIMAGIC,        1500, CLR_MAGENTA),
#undef DRGN_SCALES
/* other suits */
ARMOR("plate mail", None,
      1, 0, 1,  0, 44, 5, 350, 600,  3, 2,  ARM_SUIT, IRON, HI_METAL),
ARMOR("crystal plate mail", None,
      1, 0, 1,  0,  4, 5, 150, 5000, 3, 2,  ARM_SUIT, GEMSTONE, CLR_WHITE),
ARMOR("splint mail", None,
      1, 0, 1,  0, 62, 5, 300,  80,  4, 1,  ARM_SUIT, IRON, HI_METAL),
/* special suit of armor for giant samurai only (no random spawn) */
ARMOR("large splint mail", None,
      1, 0, 1,  0,  0, 5, 500, 150,  4, 1,  ARM_SUIT, IRON, HI_METAL),
ARMOR("banded mail", None,
      1, 0, 1,  0, 72, 5, 250,  90,  4, 1,  ARM_SUIT, IRON, HI_METAL),
ARMOR("chain mail", None,
      1, 0, 0,  0, 75, 5, 200,  75,  5, 1,  ARM_SUIT, IRON, HI_METAL),
ARMOR("dwarvish chain mail", None,
      1, 0, 0,  0, 12, 1, 200,  75,  4, 2,  ARM_SUIT, IRON, HI_METAL),
ARMOR("elven chain mail", None,
      1, 0, 0,  0, 14, 1, 200,  75,  5, 2,  ARM_SUIT, COPPER, HI_COPPER),
ARMOR("dark elven chain mail", None,
      1, 0, 0,  0,  1, 1, 200, 150,  4, 2,  ARM_SUIT, ADAMANTINE, CLR_BLACK),
ARMOR("orcish chain mail", "crude chain mail",
      0, 0, 0,  0, 20, 5, 200,  75,  6, 1,  ARM_SUIT, IRON, CLR_BLACK),
ARMOR("scale mail", None,
      1, 0, 0,  0, 72, 5, 175,  45,  6, 1,  ARM_SUIT, IRON, HI_METAL),
ARMOR("studded armor", None,
      1, 0, 0,  0, 75, 3, 125,  15,  7, 1,  ARM_SUIT, LEATHER, HI_LEATHER),
ARMOR("ring mail", None,
      1, 0, 0,  0, 72, 5, 150, 100,  7, 1,  ARM_SUIT, IRON, HI_METAL),
ARMOR("orcish ring mail", "crude ring mail",
      0, 0, 0,  0, 20, 5, 150,  80,  8, 1,  ARM_SUIT, IRON, CLR_BLACK),
ARMOR("armor", None,
      1, 0, 0,  0, 82, 3,  75,   5,  8, 1,  ARM_SUIT, LEATHER, HI_LEATHER),
ARMOR("dark elven tunic", "spider silk tunic",
      0, 0, 0,  0,  1, 1,  10,  15,  7, 2,  ARM_SUIT, SPIDER_SILK, CLR_GRAY),
ARMOR("jacket", None,
      1, 0, 0,  0, 12, 0,  30,  10,  9, 0,  ARM_SUIT, LEATHER, CLR_BLACK),

/* shirts */
ARMOR("Hawaiian shirt", None,
      1, 0, 0,  0, 10, 0,   5,   3, 10, 0,  ARM_SHIRT, CLOTH, CLR_MAGENTA),
ARMOR("striped shirt", None,
      1, 0, 0,  0,  0, 0,   5,   2, 10, 0,  ARM_SHIRT, CLOTH, CLR_GRAY),
ARMOR("T-shirt", None,
      1, 0, 0,  0,  2, 0,   5,   2, 10, 0,  ARM_SHIRT, CLOTH, CLR_WHITE),

/* cloaks */
CLOAK("mummy wrapping", None,
      1, 0,          0,  0, 0,  3,   2, 10, 1,  CLOTH, CLR_GRAY),
        /* worn mummy wrapping blocks invisibility */
CLOAK("elven cloak", "faded pall",
      0, 1,    STEALTH,  7, 0, 10,  60,  9, 1,  CLOTH, CLR_BLACK),
CLOAK("dark elven cloak", "spider silk pall",
      0, 1,    STEALTH,  1, 0, 10, 120,  8, 2,  SPIDER_SILK, CLR_GRAY),
CLOAK("orcish cloak", "coarse mantelet",
      0, 0,          0,  8, 0, 10,  40, 10, 1,  CLOTH, CLR_BLACK),
CLOAK("dwarvish cloak", "hooded cloak",
      0, 0,          0,  8, 0, 10,  50, 10, 1,  CLOTH, HI_CLOTH),
CLOAK("oilskin cloak", "slippery cloak",
      0, 0,          0,  8, 0, 10,  50,  9, 2,  CLOTH, HI_CLOTH),
CLOAK("robe", None,
      1, 1,          0,  3, 0, 15,  50,  8, 2,  CLOTH, CLR_RED),
        /* robe was adopted from slash'em, where it's worn as a suit
           rather than as a cloak and there are several variations */
CLOAK("alchemy smock", "apron",
      0, 1, POISON_RES,  9, 0, 10,  50,  9, 1,  CLOTH, CLR_WHITE),
CLOAK("cloak", None,
      1, 0,          0,  8, 0, 15,  40,  9, 1,  LEATHER, CLR_BROWN),
/* with shuffled appearances... */
CLOAK("cloak of protection", "tattered cape",
      0, 1, PROTECTION,  9, 0, 10,  50,  7, 3,  CLOTH, HI_CLOTH),
CLOAK("cloak of invisibility", "opera cloak",
      0, 1,      INVIS, 10, 0, 10,  60,  9, 1,  CLOTH, CLR_BRIGHT_MAGENTA),
CLOAK("cloak of magic resistance", "ornamental cope",
      0, 1,  ANTIMAGIC,  1, 0, 10,  60,  9, 1,  CLOTH, CLR_WHITE),
        /*  'cope' is not a spelling mistake... leave it be */
CLOAK("cloak of displacement", "dusty cloak",
      0, 1,  DISPLACED, 10, 0, 10,  50,  9, 1,  CLOTH, HI_CLOTH),

/* shields */
SHIELD("small shield", None,
       1, 0, 0,           0, 4, 0,  30,   3, 9, 0,  WOOD, HI_WOOD),
SHIELD("elven shield", "blue and green shield",
       0, 0, 0,           0, 2, 0,  40,   7, 8, 0,  WOOD, CLR_GREEN),
SHIELD("Uruk-hai shield", "white-handed shield",
       0, 0, 0,           0, 2, 0,  50,   7, 9, 0,  IRON, HI_METAL),
SHIELD("orcish shield", "red-eyed shield",
       0, 0, 0,           0, 2, 0,  50,   7, 9, 0,  IRON, CLR_RED),
SHIELD("large shield", None,
       1, 0, 1,           0, 4, 0,  75,  10, 8, 0,  IRON, HI_METAL),
SHIELD("dwarvish roundshield", "large round shield",
       0, 0, 0,           0, 2, 0,  75,  10, 8, 0,  IRON, HI_METAL),
SHIELD("shield of reflection", "polished shield",
       0, 1, 0,  REFLECTING, 3, 0,  50,  50, 8, 0,  SILVER, HI_SILVER),
SHIELD("shield of light", "shiny shield",
       0, 1, 0,           0, 3, 0,  60, 400, 8, 0,  GOLD, CLR_YELLOW),
SHIELD("shield of mobility", "slippery shield",
       0, 1, 0, FREE_ACTION, 3, 0,  50, 300, 8, 0,  STEEL, HI_METAL),
SHIELD("bracers", None,
       1, 0, 0,           0, 4, 0,  10,  10, 9, 0,  LEATHER, CLR_BROWN),
SHIELD("dark elven bracers", "etched bracers",
       0, 0, 0,           0, 1, 0,  20,  30, 8, 0,  ADAMANTINE, CLR_BLACK),
SHIELD("runed bracers", None,
       1, 1, 0,           0, 0, 0,  15, 100, 8, 0,  WOOD, CLR_BRIGHT_GREEN),

/* gloves */
/* These have their color but not material shuffled, so the IRON must
 * stay CLR_BROWN (== HI_LEATHER) even though it's normally either
 * HI_METAL or CLR_BLACK.  All have shuffled descriptions.
 */
GLOVES("gloves", "old gloves",
       0, 0,          0, 16, 1, 10,   8, 9, 0,  LEATHER, HI_LEATHER),
GLOVES("dark elven gloves", "archer gloves",
       0, 0,          0,  1, 1,  3,  20, 8, 0,  SPIDER_SILK, CLR_BROWN),
GLOVES("gauntlets", "falconry gloves",
       0, 0,          0, 12, 1, 30,  50, 9, 0,  IRON, CLR_BROWN),
GLOVES("gauntlets of power", "riding gloves",
       0, 1,          0,  8, 1, 30,  50, 9, 0,  IRON, CLR_BROWN),
GLOVES("gauntlets of protection", "boxing gloves",
       0, 1, PROTECTION,  4, 1, 10,  60, 8, 3,  CLOTH, CLR_BROWN),
GLOVES("gauntlets of dexterity", "fencing gloves",
       0, 1,          0,  8, 1, 10,  50, 9, 0,  LEATHER, HI_LEATHER),
GLOVES("gauntlets of fumbling", "padded gloves",
       0, 1,   FUMBLING,  8, 1, 10,  50, 9, 0,  LEATHER, HI_LEATHER),
GLOVES("mummified hand", None,
       1, 0,          0,  0, 1,  5, 100, 9, 0,  FLESH, CLR_BLACK),

/* boots */
BOOTS("low boots", "walking shoes",
      0, 0,          0, 25, 2, 10,  8, 9, 0, LEATHER, HI_LEATHER),
BOOTS("dwarvish boots", "hard shoes",
      0, 0,          0,  7, 2, 50, 16, 8, 0, IRON, HI_METAL),
BOOTS("orcish boots", "crude shoes",
      0, 0,          0,  8, 2, 50, 16, 8, 0, IRON, CLR_BLACK),
BOOTS("high boots", "jackboots",
      0, 0,          0, 14, 2, 20, 12, 8, 0, LEATHER, HI_LEATHER),
/* with shuffled appearances... */
BOOTS("speed boots", "combat boots",
      0, 1,       FAST,  8, 2, 20, 50, 9, 0, LEATHER, HI_LEATHER),
BOOTS("water walking boots", "jungle boots",
      0, 1,   WWALKING, 12, 2, 15, 50, 9, 0, LEATHER, HI_LEATHER),
BOOTS("jumping boots", "hiking boots",
      0, 1,    JUMPING, 12, 2, 20, 50, 9, 0, LEATHER, HI_LEATHER),
BOOTS("elven boots", "mud boots",
      0, 1,    STEALTH, 12, 2, 15,  8, 9, 0, LEATHER, HI_LEATHER),
BOOTS("dark elven boots", "logger boots",
      0, 1,    STEALTH,  1, 2, 15, 16, 8, 0, SPIDER_SILK, CLR_GRAY),
BOOTS("kicking boots", "buckled boots",
      0, 1,          0, 12, 2, 50,  8, 9, 0, IRON, CLR_BROWN),
        /* CLR_BROWN for same reason as gauntlets of power */
BOOTS("levitation boots", "snow boots",
      0, 1, LEVITATION, 12, 2, 15, 30, 9, 0, LEATHER, HI_LEATHER),
BOOTS("fumble boots", "riding boots",
      0, 1,   FUMBLING, 12, 2, 20, 30, 9, 0, LEATHER, HI_LEATHER),
#undef HELM
#undef CLOAK
#undef SHIELD
#undef GLOVES
#undef BOOTS
#undef ARMOR

/* rings ... */
/* note that prob = 1 for all normal rings */
#define RING(name,stone,power,prob,cost,mgc,spec,mohs,metal,color) \
    OBJECT(OBJ(name, stone),                                           \
           BITS(0, 0, spec, 0, mgc, spec, 0, 0, 0,                     \
                HARDGEM(mohs), 0, P_NONE, metal),                      \
           power, RING_CLASS, prob, 0, 3, cost, 0, 0, 0, 0, 15, color)
RING("adornment", "wooden",
     ADORNED,                  1, 100, 1, 1, 2, WOOD, HI_WOOD),
RING("gain strength", "granite",
     0,                        1, 150, 1, 1, 7, MINERAL, HI_MINERAL),
RING("gain constitution", "opal",
     0,                        1, 150, 1, 1, 7, MINERAL, HI_MINERAL),
RING("increase accuracy", "clay",
     0,                        1, 150, 1, 1, 4, MINERAL, CLR_RED),
RING("increase damage", "coral",
     0,                        1, 150, 1, 1, 4, MINERAL, CLR_ORANGE),
RING("protection", "black onyx",
     PROTECTION,               1, 100, 1, 1, 7, MINERAL, CLR_BLACK),
        /* 'PROTECTION' intrinsic enhances MC from worn armor by +1,
           regardless of ring's enchantment; wearing a second ring of
           protection (or even one ring of protection combined with
           cloak of protection) doesn't give a second MC boost */
RING("regeneration", "moonstone",
     REGENERATION,             1, 200, 1, 0,  6, MINERAL, HI_MINERAL),
RING("searching", "tiger eye",
     SEARCHING,                1, 200, 1, 0,  6, GEMSTONE, CLR_BROWN),
RING("stealth", "jade",
     STEALTH,                  1, 100, 1, 0,  6, GEMSTONE, CLR_GREEN),
RING("sustain ability", "bronze",
     FIXED_ABIL,               1, 100, 1, 0,  4, COPPER, HI_COPPER),
RING("levitation", "agate",
     LEVITATION,               1, 200, 1, 0,  7, GEMSTONE, CLR_RED),
RING("hunger", "topaz",
     HUNGER,                   1, 100, 1, 0,  8, GEMSTONE, CLR_CYAN),
RING("aggravate monster", "sapphire",
     AGGRAVATE_MONSTER,        1, 150, 1, 0,  9, GEMSTONE, CLR_BLUE),
RING("conflict", "ruby",
     CONFLICT,                 1, 300, 1, 0,  9, GEMSTONE, CLR_RED),
RING("warning", "diamond",
     WARNING,                  1, 100, 1, 0, 10, GEMSTONE, CLR_WHITE),
RING("poison resistance", "pearl",
     POISON_RES,               1, 150, 1, 0,  4, BONE, CLR_WHITE),
RING("fire resistance", "iron",
     FIRE_RES,                 1, 200, 1, 0,  5, IRON, HI_METAL),
RING("cold resistance", "brass",
     COLD_RES,                 1, 150, 1, 0,  4, COPPER, HI_COPPER),
RING("shock resistance", "copper",
     SHOCK_RES,                1, 150, 1, 0,  3, COPPER, HI_COPPER),
RING("free action", "twisted",
     FREE_ACTION,              1, 200, 1, 0,  6, STEEL, HI_METAL),
RING("slow digestion", "steel",
     SLOW_DIGESTION,           1, 200, 1, 0,  8, STEEL, HI_METAL),
RING("teleportation", "silver",
     TELEPORT,                 1, 200, 1, 0,  3, SILVER, HI_SILVER),
RING("teleport control", "gold",
     TELEPORT_CONTROL,         1, 300, 1, 0,  3, GOLD, HI_GOLD),
RING("polymorph", "ivory",
     POLYMORPH,                1, 300, 1, 0,  4, BONE, CLR_WHITE),
RING("polymorph control", "emerald",
     POLYMORPH_CONTROL,        1, 300, 1, 0,  8, GEMSTONE, CLR_BRIGHT_GREEN),
RING("invisibility", "wire",
     INVIS,                    1, 150, 1, 0,  5, STEEL, HI_METAL),
RING("see invisible", "engagement",
     SEE_INVIS,                1, 150, 1, 0,  5, GOLD, HI_GOLD),
RING("protection from shape changers", "shiny",
     PROT_FROM_SHAPE_CHANGERS, 1, 100, 1, 0,  5, PLATINUM, CLR_BRIGHT_CYAN),
/* placeholders for artifact rings; will not spawn randomly */
RING("ancient", None,
     FREE_ACTION,              0, 800, 1, 0,  8, GEMSTONE, CLR_BRIGHT_GREEN),
RING("lustrous", None,
     INVIS,                    0, 600, 1, 0,  3, GOLD, HI_GOLD),
#undef RING

/* amulets ... - THE Amulet comes last because it is special */
#define AMULET(name,desc,power,prob) \
    OBJECT(OBJ(name, desc),                                            \
           BITS(0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, P_NONE, STEEL),        \
           power, AMULET_CLASS, prob, 0, 20, 150, 0, 0, 0, 0, 20, HI_METAL)
AMULET("amulet of ESP",                "circular", TELEPAT, 120),
AMULET("amulet of life saving",       "spherical", LIFESAVED, 75),
AMULET("amulet of strangulation",          "oval", STRANGLED, 115),
AMULET("amulet of restful sleep",    "triangular", SLEEPY, 115),
AMULET("amulet versus poison",        "pyramidal", POISON_RES, 115),
AMULET("amulet of change",               "square", 0, 115),
AMULET("amulet of unchanging",          "concave", UNCHANGING, 60),
AMULET("amulet of reflection",        "hexagonal", REFLECTING, 75),
AMULET("amulet of magical breathing", "octagonal", MAGICAL_BREATHING, 75),
AMULET("amulet of magic resistance",     "oblong", ANTIMAGIC, 30),
        /* +2 AC and +2 MC; +2 takes naked hero past 'warded' to 'guarded' */
AMULET("amulet of guarding",         "perforated", PROTECTION, 75),
        /* cubical: some descriptions are already three dimensional and
           parallelogrammatical (real word!) would be way over the top */
AMULET("amulet of flying",              "cubical", FLYING, 60),
/* fixed descriptions; description duplication is deliberate;
 * fake one must come before real one because selection for
 * description shuffling stops when a non-magic amulet is encountered
 */
OBJECT(OBJ("cheap plastic imitation of the Amulet of Yendor",
           "Amulet of Yendor"),
       BITS(0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, PLASTIC),
       0, AMULET_CLASS, 0, 0, 20, 0, 0, 0, 0, 0, 1, HI_METAL),
OBJECT(OBJ("Amulet of Yendor", /* note: description == name */
           "Amulet of Yendor"),
       BITS(0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, GEMSTONE),
       0, AMULET_CLASS, 0, 0, 20, 30000, 0, 0, 0, 0, 20, HI_METAL),
#undef AMULET

/* tools ... */
/* tools with weapon characteristics come last */
#define TOOL(name,desc,kn,mrg,mgc,chg,prob,wt,cost,mat,color) \
    OBJECT(OBJ(name, desc),                                             \
           BITS(kn, mrg, chg, 0, mgc, chg, 0, 0, 0, 0, 0, P_NONE, mat), \
           0, TOOL_CLASS, prob, 0, wt, cost, 0, 0, 0, 0, wt, color)
#define CONTAINER(name,desc,kn,mgc,chg,prob,wt,cost,mat,color) \
    OBJECT(OBJ(name, desc),                                             \
           BITS(kn, 0, chg, 1, mgc, chg, 0, 0, 0, 0, 0, P_NONE, mat),   \
           0, TOOL_CLASS, prob, 0, wt, cost, 0, 0, 0, 0, wt, color)
#define WEPTOOL(name,desc,kn,mgc,bi,prob,wt,cost,sdam,ldam,hitbon,typ,sub,mat,clr) \
    OBJECT(OBJ(name, desc),                                             \
           BITS(kn, 0, 1, 0, mgc, 1, 0, 0, bi, 0, typ, sub, mat),    \
           0, TOOL_CLASS, prob, 0, wt, cost, sdam, ldam, hitbon, 0, wt, clr)
/* containers */
CONTAINER("large box",        None, 1, 0, 0, 40, 350,   8, WOOD, HI_WOOD),
CONTAINER("chest",            None, 1, 0, 0, 25, 600,  16, WOOD, HI_WOOD),
CONTAINER("iron safe",        None, 1, 0, 0, 10, 900,  50, IRON, HI_METAL),
CONTAINER("crystal chest",    None, 1, 1, 0,  1, 500,  20, GEMSTONE, CLR_WHITE),
CONTAINER("ice box",          None, 1, 0, 0,  5, 900,  42, PLASTIC, CLR_WHITE),
/* define as object so it's unwishable. base type is "hidden chest" to avoid conflict
   with terrain type name, but description is "magic chest" so player gets correct
   feedback when they loot it. it should always remain unidentified. */
OBJECT(OBJ("hidden chest", "magic chest"),                             \
       BITS(0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, P_NONE, GEMSTONE),        \
       0, TOOL_CLASS, 0, 0, 5000, 999, 0, 0, 0, 0, 5000, CLR_MAGENTA),
CONTAINER("sack",            "bag", 0, 0, 0, 35,  15,   2, CLOTH, HI_CLOTH),
CONTAINER("oilskin sack",    "bag", 0, 0, 0,  5,  15, 100, CLOTH, HI_CLOTH),
CONTAINER("bag of holding",  "bag", 0, 1, 0, 20,  15, 100, CLOTH, HI_CLOTH),
CONTAINER("bag of tricks",   "bag", 0, 1, 1, 20,  15, 100, CLOTH, HI_CLOTH),
#undef CONTAINER

/* lock opening tools */
TOOL("skeleton key", "key",
     0, 0, 0, 0,  80,   3,   10, BONE, CLR_WHITE),
TOOL("lock pick", None,
     1, 0, 0, 0,  60,   4,   20, STEEL, HI_METAL),
TOOL("credit card", None,
     1, 0, 0, 0,  15,   1,   10, PLASTIC, CLR_WHITE),
TOOL("magic key", "ornate key",
     0, 0, 1, 0,   0,   5, 1000, GEMSTONE, CLR_RED),
/* light sources */
TOOL("tallow candle", "candle",
     0, 1, 0, 0,  20,   2,  10, WAX, CLR_WHITE),
TOOL("wax candle", "candle",
     0, 1, 0, 0,   5,   2,  20, WAX, CLR_WHITE),
TOOL("lantern", None,
     1, 0, 0, 0,  30,  30,  12, COPPER, CLR_YELLOW),
TOOL("oil lamp", "lamp",
     0, 0, 0, 0,  45,  20,  10, COPPER, CLR_YELLOW),
TOOL("magic lamp", "lamp",
     0, 0, 1, 0,  15,  20, 500, COPPER, CLR_YELLOW),
/* other tools */
TOOL("expensive camera", None,
     1, 0, 0, 1,  15,  12, 200, PLASTIC, CLR_BLACK),
TOOL("mirror", "looking glass",
     0, 0, 0, 0,  45,  13,  10, GLASS, HI_SILVER),
TOOL("crystal ball", "glass orb",
     0, 0, 1, 1,  15, 150,  60, GLASS, HI_GLASS),
TOOL("eight ball", "plastic orb",
     0, 0, 0, 0,   0,  20,  30, PLASTIC, CLR_BLACK),
TOOL("lenses", None,
     1, 0, 0, 0,   4,   3,  80, GLASS, HI_GLASS),
TOOL("goggles", None,
     1, 0, 0, 0,   1,   3,  50, PLASTIC, CLR_BLACK),
TOOL("blindfold", None,
     1, 0, 0, 0,  50,   2,  20, CLOTH, CLR_BLACK),
TOOL("towel", None,
     1, 0, 0, 0,  50,   2,  50, CLOTH, CLR_MAGENTA),
TOOL("saddle", None,
     1, 0, 0, 0,   5, 200, 150, LEATHER, HI_LEATHER),
OBJECT(OBJ("barding", None),
        BITS(1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, P_NONE, IRON),
        0, TOOL_CLASS, 4, 0, 250, 300, 0, 0, 3, 0, 250, HI_METAL),
OBJECT(OBJ("spiked barding", None),
        BITS(1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, P_NONE, IRON),
        0, TOOL_CLASS, 2, 0, 275, 300, 0, 0, 3, 0, 275, HI_METAL),
OBJECT(OBJ("barding of reflection", "polished barding"),
        BITS(0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, P_NONE, SILVER),
        0, TOOL_CLASS, 1, 0, 275, 600, 0, 0, 3, 0, 275, HI_SILVER),
OBJECT(OBJ("runed barding", None),
        BITS(1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, P_NONE, STEEL),
        0, TOOL_CLASS, 0, 0, 125, 900, 0, 0, 6, 0, 125, HI_METAL),
TOOL("leash", None,
     1, 0, 0, 0,  65,  12,  20, LEATHER, HI_LEATHER),
TOOL("stethoscope", None,
     1, 0, 0, 0,  25,   4,  75, IRON, HI_METAL),
TOOL("tinning kit", None,
     1, 0, 0, 1,  20, 100,  30, IRON, HI_METAL),
TOOL("tin opener", None,
     1, 0, 0, 0,  35,   4,  30, IRON, HI_METAL),
TOOL("can of grease", None,
     1, 0, 0, 1,  10,  15,  20, IRON, HI_METAL),
TOOL("figurine", None, /* monster type specified by obj->corpsenm */
     1, 0, 1, 0,  25,  50,  80, MINERAL, HI_MINERAL),
OBJECT(OBJ("blacksmith hammer", "runed hammer"),
       BITS(0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, P_NONE, IRON),
       0, TOOL_CLASS, 0, 0, 30, 300, 0, 0, 0, 0, 30, HI_METAL),
TOOL("magic marker", "marker",
     0, 0, 1, 1,   0,   2,  50, PLASTIC, CLR_RED),
/* traps */
TOOL("land mine", None,
     1, 0, 0, 0,   0, 300, 180, IRON, CLR_RED),
TOOL("beartrap", None,
     1, 0, 0, 0,   0, 200,  60, IRON, HI_METAL),
/* instruments */
TOOL("pea whistle", "whistle",
     0, 0, 0, 0, 100,   3,  10, STEEL, HI_METAL),
TOOL("magic whistle", "whistle",
     0, 0, 1, 0,  30,   3,  10, STEEL, HI_METAL),
TOOL("flute", "flute",
     0, 0, 0, 0,   4,   5,  12, WOOD, HI_WOOD),
TOOL("pan flute", None,
     0, 0, 0, 0,   0,   6,  24, WOOD, HI_WOOD),
TOOL("magic flute", "flute",
     0, 0, 1, 1,   2,   5,  36, WOOD, HI_WOOD),
TOOL("tooled horn", "horn",
     0, 0, 0, 0,   5,  18,  15, BONE, CLR_WHITE),
TOOL("frost horn", "horn",
     0, 0, 1, 1,   2,  18,  50, BONE, CLR_WHITE),
TOOL("fire horn", "horn",
     0, 0, 1, 1,   2,  18,  50, BONE, CLR_WHITE),
TOOL("horn of plenty", "horn", /* horn, but not an instrument */
     0, 0, 1, 1,   2,  18,  50, BONE, CLR_WHITE),
TOOL("harp", "harp",
     0, 0, 0, 0,   4,  30,  50, WOOD, HI_WOOD),
TOOL("magic harp", "harp",
     0, 0, 1, 1,   2,  30,  50, WOOD, HI_WOOD),
TOOL("bell", None,
     1, 0, 0, 0,   2,  30,  50, COPPER, HI_COPPER),
TOOL("bugle", None,
     1, 0, 0, 0,   4,  10,  15, COPPER, HI_COPPER),
TOOL("leather drum", "drum",
     0, 0, 0, 0,   4,  25,  25, LEATHER, HI_LEATHER),
TOOL("drum of earthquake", "drum",
     0, 0, 1, 1,   2,  25,  25, LEATHER, HI_LEATHER),
/* tools useful as weapons */
WEPTOOL("pick-axe", None,
        1, 0, 0, 20, 100,  50,  6,  3, 0, WHACK,  P_PICK_AXE, IRON, HI_METAL),
WEPTOOL("grappling hook", "iron hook",
        0, 0, 0,  5,  30,  50,  2,  6, 0, WHACK,  P_FLAIL,    IRON, HI_METAL),
WEPTOOL("unicorn horn", None,
        1, 1, 0,  0,  20, 100,  8, 10, 1, PIERCE, P_UNICORN_HORN, BONE, CLR_WHITE),
        /* 3.4.1: unicorn horn left classified as "magic" */
/* two unique tools;
 * not artifacts, despite the comment which used to be here
 */
OBJECT(OBJ("Candelabrum of Invocation", "candelabrum"),
       BITS(0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, P_NONE, GOLD),
       0, TOOL_CLASS, 0, 0, 10, 5000, 0, 0, 0, 0, 200, HI_GOLD),
OBJECT(OBJ("Bell of Opening", "engraved silver bell"),
       BITS(0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, P_NONE, SILVER),
       0, TOOL_CLASS, 0, 0, 10, 5000, 0, 0, 0, 0, 50, HI_SILVER),
#undef TOOL
#undef WEPTOOL

/* Comestibles ... */
#define FOOD(name, prob, delay, wt, unk, tin, nutrition, color)         \
    OBJECT(OBJ(name, None),                                       \
           BITS(1, 1, unk, 0, 0, 0, 0, 0, 0, 0, 0, P_NONE, tin), 0,     \
           FOOD_CLASS, prob, delay, wt, nutrition / 20 + 5, 0, 0, 0, 0, \
           nutrition, color)
/* All types of food (except tins & corpses) must have a delay of at least 1.
 * Delay on corpses is computed and is weight dependant.
 * Domestic pets prefer tripe rations above all others.
 * Fortune cookies can be read, using them up without ingesting them.
 * Carrots improve your vision.
 * +0 tins contain monster meat.
 * +1 tins (of spinach) make you stronger (like Popeye).
 * Meatballs/sticks/rings are only created from objects via stone to flesh.
 */
/* meat */
FOOD("tripe ration",        140,  2, 10, 0, FLESH, 200, CLR_BROWN),
FOOD("corpse",                0,  1,  0, 0, FLESH,   0, CLR_BROWN),
/* body parts (currently only for base object: the Eye of Vecna) */
FOOD("eyeball",               0,  1,  1, 0, FLESH,   5, CLR_WHITE),
FOOD("egg",                  85,  1,  1, 1, FLESH,  80, CLR_WHITE),
FOOD("meatball",              0,  1,  1, 0, FLESH,   5, CLR_BROWN),
FOOD("meat stick",            0,  1,  1, 0, FLESH,   5, CLR_BROWN),
FOOD("strip of bacon",        0,  1,  1, 0, FLESH,   5, CLR_BROWN),
FOOD("huge chunk of meat",    0, 20,400, 0, FLESH,2000, CLR_BROWN),
/* special cases because they aren't mergable */
OBJECT(OBJ("meat ring", None),
       BITS(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, FLESH),
       0, FOOD_CLASS, 0, 1, 5, 1, 0, 0, 0, 0, 5, CLR_BROWN),
OBJECT(OBJ("meat suit", None),
       BITS(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, FLESH),
       0, FOOD_CLASS, 0, 3, 50, 1, 0, 0, 0, 0, 200, CLR_BROWN),
OBJECT(OBJ("meat helmet", None),
       BITS(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, FLESH),
       0, FOOD_CLASS, 0, 1, 10, 1, 0, 0, 0, 0, 20, CLR_BROWN),
OBJECT(OBJ("meat shield", None),
       BITS(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, FLESH),
       0, FOOD_CLASS, 0, 1, 15, 1, 0, 0, 0, 0, 30, CLR_BROWN),
OBJECT(OBJ("meat gloves", None),
       BITS(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, FLESH),
       0, FOOD_CLASS, 0, 1, 10, 1, 0, 0, 0, 0, 20, CLR_BROWN),
OBJECT(OBJ("meat boots", None),
       BITS(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, FLESH),
       0, FOOD_CLASS, 0, 1, 10, 1, 0, 0, 0, 0, 20, CLR_BROWN),
/* pudding 'corpses' will turn into these and combine;
   must be in same order as the pudding monsters */
FOOD("glob of gray ooze",     0,  2, 20, 0, FLESH,  20, CLR_GRAY),
FOOD("glob of brown pudding", 0,  2, 20, 0, FLESH,  20, CLR_BROWN),
FOOD("glob of green slime",   0,  2, 20, 0, FLESH,  20, CLR_GREEN),
FOOD("glob of black pudding", 0,  2, 20, 0, FLESH,  20, CLR_BLACK),

/* fruits & veggies */
FOOD("kelp frond",            0,  1,  1, 0, VEGGY,  30, CLR_GREEN),
FOOD("eucalyptus leaf",      10,  1,  1, 0, VEGGY,  30, CLR_GREEN),
FOOD("mistletoe",             3,  1,  1, 0, VEGGY, 100, CLR_GREEN),
FOOD("apple",                10,  1,  2, 0, VEGGY,  50, CLR_RED),
FOOD("orange",               10,  1,  2, 0, VEGGY,  80, CLR_ORANGE),
FOOD("pear",                 10,  1,  2, 0, VEGGY,  50, CLR_BRIGHT_GREEN),
FOOD("melon",                10,  1,  5, 0, VEGGY, 100, CLR_BRIGHT_GREEN),
FOOD("banana",               10,  1,  2, 0, VEGGY,  80, CLR_YELLOW),
FOOD("carrot",               12,  1,  2, 0, VEGGY,  50, CLR_ORANGE),
FOOD("sprig of wolfsbane",    7,  1,  1, 0, VEGGY,  40, CLR_GREEN),
FOOD("clove of garlic",       7,  1,  1, 0, VEGGY,  40, CLR_WHITE),
/* name of slime mold is changed based on player's OPTION=fruit:something
   and bones data might have differently named ones from prior games */
FOOD("slime mold",           75,  1,  5, 0, VEGGY, 250, HI_ORGANIC),

/* people food */
FOOD("lump of royal jelly",   0,  1,  2, 0, VEGGY, 200, CLR_MAGENTA),
FOOD("cream pie",            25,  1, 10, 0, VEGGY, 100, CLR_WHITE),
FOOD("candy bar",            10,  1,  2, 0, VEGGY, 100, CLR_BROWN),
FOOD("fortune cookie",       55,  1,  1, 0, VEGGY,  40, CLR_YELLOW),
FOOD("pancake",              25,  2,  2, 0, VEGGY, 200, CLR_YELLOW),
FOOD("lembas wafer",         20,  2,  5, 0, VEGGY, 800, CLR_WHITE),
FOOD("cram ration",          20,  3, 15, 0, VEGGY, 600, HI_ORGANIC),
FOOD("food ration",         380,  5, 20, 0, VEGGY, 800, HI_ORGANIC),
FOOD("K-ration",              0,  1, 10, 0, VEGGY, 400, HI_ORGANIC),
FOOD("C-ration",              0,  1, 10, 0, VEGGY, 300, HI_ORGANIC),
/* tins have type specified by obj->spe (+1 for spinach, other implies
   flesh; negative specifies preparation method {homemade,boiled,&c})
   and by obj->corpsenm (type of monster flesh) */
FOOD("tin",                  75,  0, 10, 1, STEEL,   0, HI_METAL),
#undef FOOD

/* potions ... */
#define POTION(name,desc,mgc,power,prob,cost,color) \
    OBJECT(OBJ(name, desc),                                             \
           BITS(0, 1, 0, 0, mgc, 0, 0, 0, 0, 0, 0, P_NONE, GLASS),      \
           power, POTION_CLASS, prob, 0, 20, cost, 0, 0, 0, 0, 10, color)
POTION("gain ability",           "ruby",  1, 0, 40, 300, CLR_RED),
POTION("restore ability",        "pink",  1, 0, 40, 100, CLR_BRIGHT_MAGENTA),
POTION("confusion",            "orange",  1, CONFUSION, 42, 100, CLR_ORANGE),
POTION("blindness",            "yellow",  1, BLINDED, 40, 150, CLR_YELLOW),
POTION("paralysis",           "emerald",  1, 0, 40, 300, CLR_BRIGHT_GREEN),
POTION("speed",                 "fizzy",  1, FAST, 40, 200, CLR_CYAN),
POTION("levitation",             "cyan",  1, LEVITATION, 42, 200, CLR_CYAN),
POTION("hallucination",      "sky blue",  1, HALLUC, 40, 100, CLR_CYAN),
POTION("invisibility", "brilliant blue",  1, INVIS, 40, 150, CLR_BRIGHT_BLUE),
POTION("see invisible",       "magenta",  1, SEE_INVIS, 42, 50, CLR_MAGENTA),
POTION("healing",          "purple-red",  1, 0, 57, 100, CLR_MAGENTA),
POTION("extra healing",          "puce",  1, 0, 47, 100, CLR_RED),
POTION("gain level",            "milky",  1, 0, 20, 300, CLR_WHITE),
POTION("enlightenment",        "swirly",  1, 0, 20, 200, CLR_BROWN),
POTION("monster detection",    "bubbly",  1, 0, 40, 150, CLR_WHITE),
POTION("object detection",      "smoky",  1, 0, 42, 150, CLR_GRAY),
POTION("gain energy",          "cloudy",  1, 0, 40, 150, CLR_WHITE),
POTION("sleeping",       "effervescent",  1, 0, 42, 100, CLR_GRAY),
POTION("full healing",          "black",  1, 0, 10, 200, CLR_BLACK),
POTION("polymorph",            "golden",  1, 0, 10, 200, CLR_YELLOW),
POTION("booze",                 "brown",  0, 0, 42,  50, CLR_BROWN),
POTION("fruit juice",            "dark",  0, 0, 42,  50, CLR_BLACK),
POTION("acid",                  "white",  0, 0, 10, 250, CLR_WHITE),
POTION("oil",                   "murky",  0, 0, 30, 250, CLR_BROWN),
/* fixed descriptions */
POTION("sickness",         "dark green",  0, 0, 42,  50, CLR_GREEN),
POTION("drow poison",            "inky",  0, 0,  8, 300, CLR_BLACK),
POTION("water",                 "clear",  0, 0, 92, 100, CLR_CYAN),
#undef POTION

/* scrolls ... */
#define SCROLL(name,text,mgc,prob,cost) \
    OBJECT(OBJ(name, text),                                           \
           BITS(0, 1, 0, 0, mgc, 0, 0, 0, 0, 0, 0, P_NONE, PAPER),    \
           0, SCROLL_CLASS, prob, 0, 5, cost, 0, 0, 0, 0, 6, HI_PAPER)
SCROLL("enchant armor",              "ZELGO MER",  1,  65,  80),
SCROLL("destroy armor",         "JUYED AWK YACC",  1,  45, 100),
SCROLL("confuse monster",                 "NR 9",  1,  53, 100),
SCROLL("scare monster",   "XIXAXA XOXAXA XUXAXA",  1,  35, 100),
SCROLL("remove curse",             "PRATYAVAYAH",  1,  65,  80),
SCROLL("enchant weapon",         "DAIYEN FOOELS",  1,  80,  60),
SCROLL("create monster",       "LEP GEX VEN ZEA",  1,  45, 200),
SCROLL("taming",                   "PRIRUTSENIE",  1,  15, 200),
SCROLL("genocide",                  "ELBIB YLOH",  1,  15, 300),
SCROLL("light",                 "VERR YED HORRE",  1,  90,  50),
SCROLL("teleportation",        "VENZAR BORGAVVE",  1,  55, 100),
SCROLL("gold detection",                 "THARR",  1,  34, 100),
SCROLL("food detection",               "YUM YUM",  1,  25, 100),
SCROLL("identify",                  "KERNOD WEL",  1, 180,  20),
SCROLL("magic mapping",              "ELAM EBOW",  1,  45, 100),
SCROLL("amnesia",                   "DUAM XNAHT",  1,  35, 200),
SCROLL("fire",                  "ANDOVA BEGARIN",  1,  30, 100),
SCROLL("earth",                          "KIRJE",  1,  18, 200),
SCROLL("punishment",            "VE FORBRYDERNE",  1,  15, 300),
SCROLL("charging",                "HACKEM MUCHE",  1,  15, 300),
SCROLL("stinking cloud",             "VELOX NEB",  1,  15, 300),
SCROLL("magic detection",        "FOOBIE BLETCH",  1,  25, 300),
    /* Extra descriptions, shuffled into use at start of new game.
     * Code in win/share/tilemap.c depends on SCR_STINKING_CLOUD preceding
     * these and on how many of them there are.  If a real scroll gets added
     * after stinking cloud or the number of extra descriptions changes,
     * tilemap.c must be modified to match.
     */
SCROLL(None,              "TEMOV",  1,   0, 100),
SCROLL(None,         "GARVEN DEH",  1,   0, 100),
SCROLL(None,            "READ ME",  1,   0, 100),
SCROLL(None,      "ETAOIN SHRDLU",  1,   0, 100),
SCROLL(None,        "LOREM IPSUM",  1,   0, 100),
SCROLL(None,              "FNORD",  1,   0, 100), /* Illuminati */
SCROLL(None,            "KO BATE",  1,   0, 100), /* Kurd Lasswitz */
SCROLL(None,      "ABRA KA DABRA",  1,   0, 100), /* traditional incantation */
SCROLL(None,       "ASHPD SODALG",  1,   0, 100), /* Portal */
SCROLL(None,            "ZLORFIK",  1,   0, 100), /* Zak McKracken */
SCROLL(None,      "GNIK SISI VLE",  1,   0, 100), /* Zak McKracken */
SCROLL(None,    "HAPAX LEGOMENON",  1,   0, 100),
SCROLL(None,  "EIRIS SAZUN IDISI",  1,   0, 100), /* Merseburg Incantations */
SCROLL(None,    "PHOL ENDE WODAN",  1,   0, 100), /* Merseburg Incantations */
SCROLL(None,              "GHOTI",  1,   0, 100), /* pronounced as 'fish',
                                                        George Bernard Shaw */
SCROLL(None, "MAPIRO MAHAMA DIROMAT", 1, 0, 100), /* Wizardry */
SCROLL(None,  "VAS CORP BET MANI",  1,   0, 100), /* Ultima */
SCROLL(None,            "XOR OTA",  1,   0, 100), /* Aarne Haapakoski */
SCROLL(None, "STRC PRST SKRZ KRK",  1,   0, 100), /* Czech and Slovak
                                                        tongue-twister */
    /* These must come last because they have special fixed descriptions.
     */
#ifdef MAIL
SCROLL("mail",          "stamped",  0,   0,   0),
#endif
SCROLL("blank paper", "unlabeled",  0,  28,  60),
#undef SCROLL

/* spellbooks ... */
    /* Expanding beyond 52 spells would require changes in spellcasting
     * or imposition of a limit on number of spells hero can know because
     * they are currently assigned successive letters, a-zA-Z, when learned.
     * [The existing spell sorting capability could conceivably be extended
     * to enable moving spells from beyond Z to within it, bumping others
     * out in the process, allowing more than 52 spells be known but keeping
     * only 52 be castable at any given time.]
     */
#define SPELL(name,desc,sub,prob,delay,level,mgc,dir,color)  \
    OBJECT(OBJ(name, desc),                                             \
           BITS(0, 0, 0, 0, mgc, 0, 0, 0, 0, 0, dir, sub, PAPER),       \
           0, SPBOOK_CLASS, prob, delay, level * 5 + 30,                \
           level * 100, 0, 0, 0, level, 20, color)
/* Spellbook description normally refers to book covers (primarily color).
   Parchment and vellum would never be used for such, but rather than
   eliminate those, finagle their definitions to refer to the pages
   rather than the cover.  They are made from animal skin (typically of
   a goat or sheep) and books using them for pages generally need heavy
   covers with straps or clamps to tightly close the book in order to
   keep the pages flat.  (However, a wooden cover might itself be covered
   by a sheet of parchment, making this become less of an exception.  Also,
   changing the internal composition from paper to leather makes eating a
   parchment or vellum spellbook break vegetarian conduct, as it should.) */
#define PAPER LEATHER /* override enum for use in SPELL() expansion */
SPELL("dig",              "parchment",
      P_MATTER_SPELL,      20,  6, 3, 1, RAY, HI_LEATHER),
SPELL("magic missile",    "vellum",
      P_ATTACK_SPELL,      25,  2, 2, 1, RAY, HI_LEATHER),
#undef PAPER /* revert to normal material */
SPELL("fireball",         "ragged",
      P_ATTACK_SPELL,      20,  4, 4, 1, RAY, HI_PAPER),
SPELL("cone of cold",     "dog eared",
      P_ATTACK_SPELL,      10,  7, 4, 1, RAY, HI_PAPER),
SPELL("sleep",            "mottled",
      P_ENCHANTMENT_SPELL, 20,  1, 2, 1, RAY, HI_PAPER),
SPELL("finger of death",  "stained",
      P_EVOCATION_SPELL,    5, 10, 7, 1, RAY, HI_PAPER),
SPELL("lightning",        "electric blue",
      P_ATTACK_SPELL,      10,  8, 4, 1, RAY, CLR_BRIGHT_BLUE),
SPELL("poison blast",     "olive green",
      P_ATTACK_SPELL,      10,  9, 5, 1, RAY, CLR_GREEN),
SPELL("acid blast",       "acid green",
      P_ATTACK_SPELL,       5,  9, 6, 1, RAY, CLR_BRIGHT_GREEN),
SPELL("psionic wave",     "worn",
      P_ATTACK_SPELL,       0,  1, 1, 1, IMMEDIATE, CLR_MAGENTA),
SPELL("power word kill",  "ominous",
      P_ATTACK_SPELL,       5, 10, 7, 1, NODIR, CLR_ORANGE),
SPELL("light",            "cloth",
      P_DIVINATION_SPELL,  25,  1, 1, 1, NODIR, HI_CLOTH),
SPELL("detect monsters",  "leathery",
      P_DIVINATION_SPELL,  20,  1, 2, 1, NODIR, HI_LEATHER),
SPELL("healing",          "white",
      P_HEALING_SPELL,     25,  2, 1, 1, IMMEDIATE, CLR_WHITE),
SPELL("knock",            "pink",
      P_MATTER_SPELL,      25,  1, 1, 1, IMMEDIATE, CLR_BRIGHT_MAGENTA),
SPELL("force bolt",       "red",
      P_ATTACK_SPELL,      30,  2, 1, 1, IMMEDIATE, CLR_RED),
SPELL("confuse monster",  "orange",
      P_ENCHANTMENT_SPELL, 20,  2, 1, 1, IMMEDIATE, CLR_ORANGE),
SPELL("cure blindness",   "yellow",
      P_HEALING_SPELL,     20,  2, 2, 1, IMMEDIATE, CLR_YELLOW),
SPELL("drain life",       "velvet",
      P_ATTACK_SPELL,      10,  2, 2, 1, IMMEDIATE, CLR_MAGENTA),
SPELL("slow monster",     "light green",
      P_ENCHANTMENT_SPELL, 20,  2, 2, 1, IMMEDIATE, CLR_BRIGHT_GREEN),
SPELL("wizard lock",      "dark green",
      P_MATTER_SPELL,      20,  3, 2, 1, IMMEDIATE, CLR_GREEN),
SPELL("create monster",   "turquoise",
      P_CLERIC_SPELL,      20,  3, 2, 1, NODIR, CLR_BRIGHT_CYAN),
SPELL("detect food",      "cyan",
      P_DIVINATION_SPELL,  20,  3, 1, 1, NODIR, CLR_CYAN),
SPELL("cause fear",       "light blue",
      P_ENCHANTMENT_SPELL, 20,  3, 3, 1, NODIR, CLR_BRIGHT_BLUE),
SPELL("clairvoyance",     "dark blue",
      P_DIVINATION_SPELL,  15,  3, 3, 1, NODIR, CLR_BLUE),
SPELL("cure sickness",    "indigo",
      P_HEALING_SPELL,     25,  3, 3, 1, IMMEDIATE, CLR_BLUE),
SPELL("charm monster",    "magenta",
      P_ENCHANTMENT_SPELL, 15,  3, 5, 1, IMMEDIATE, CLR_MAGENTA),
SPELL("haste self",       "purple",
      P_ESCAPE_SPELL,      25,  4, 3, 1, NODIR, CLR_MAGENTA),
SPELL("detect unseen",    "violet",
      P_DIVINATION_SPELL,  20,  4, 2, 1, NODIR, CLR_MAGENTA),
SPELL("levitation",       "tan",
      P_ESCAPE_SPELL,      20,  4, 4, 1, NODIR, CLR_BROWN),
SPELL("extra healing",    "plaid",
      P_HEALING_SPELL,     25,  5, 3, 1, IMMEDIATE, CLR_GREEN),
SPELL("restore ability",  "light brown",
      P_HEALING_SPELL,     25,  5, 2, 1, IMMEDIATE, CLR_BROWN),
SPELL("invisibility",     "dark brown",
      P_ESCAPE_SPELL,      20,  5, 3, 1, NODIR, CLR_BROWN),
SPELL("detect treasure",  "gray",
      P_DIVINATION_SPELL,  20,  5, 4, 1, NODIR, CLR_GRAY),
SPELL("remove curse",     "wrinkled",
      P_CLERIC_SPELL,      25,  5, 3, 1, NODIR, HI_PAPER),
SPELL("magic mapping",    "dusty",
      P_DIVINATION_SPELL,  15,  7, 5, 1, NODIR, HI_PAPER),
SPELL("identify",         "bronze",
      P_DIVINATION_SPELL,  20,  6, 3, 1, NODIR, HI_COPPER),
SPELL("turn undead",      "copper",
      P_CLERIC_SPELL,      15,  8, 6, 1, IMMEDIATE, HI_COPPER),
SPELL("polymorph",        "silver",
      P_MATTER_SPELL,      10,  8, 6, 1, IMMEDIATE, HI_SILVER),
SPELL("teleport away",    "gold",
      P_ESCAPE_SPELL,      15,  6, 6, 1, IMMEDIATE, HI_GOLD),
SPELL("create familiar",  "glittering",
      P_CLERIC_SPELL,      10,  7, 6, 1, NODIR, CLR_WHITE),
SPELL("cancellation",     "shining",
      P_MATTER_SPELL,       5,  8, 7, 1, IMMEDIATE, CLR_WHITE),
SPELL("protection",       "dull",
      P_CLERIC_SPELL,      15,  3, 1, 1, NODIR, HI_PAPER),
SPELL("jumping",          "thin",
      P_ESCAPE_SPELL,      20,  3, 2, 1, IMMEDIATE, HI_PAPER),
SPELL("stone to flesh",   "thick",
      P_HEALING_SPELL,     15,  1, 3, 1, IMMEDIATE, HI_PAPER),
SPELL("repair armor",     "platinum",
      P_MATTER_SPELL,      20,  6, 3, 1, NODIR, HI_PAPER),
SPELL("reflection",       "decrepit",
      P_MATTER_SPELL,      10,  3, 5, 1, NODIR, CLR_BROWN),
SPELL("critical healing", "antique",
      P_HEALING_SPELL,     10,  8, 6, 1, IMMEDIATE, CLR_BRIGHT_GREEN),
SPELL("burning hands",    "crinkled",
      P_ENCHANTMENT_SPELL, 20,  2, 1, 1, NODIR, CLR_RED),
SPELL("shocking grasp",   "warped",
      P_ENCHANTMENT_SPELL, 15,  2, 2, 1, NODIR, CLR_BRIGHT_BLUE),
/* from slash'em, create a tame critter which explodes when attacking,
   damaging adjacent creatures--friend or foe--and dying in the process */
SPELL("flame sphere",     "canvas",
      P_MATTER_SPELL,      15,  2, 2, 1, NODIR, CLR_BROWN),
SPELL("freeze sphere",    "hardcover",
      P_MATTER_SPELL,      15,  2, 2, 1, NODIR, CLR_BROWN),
/* Druid spells, sans 'Finger of Death' (ray-based spells
   need to stay grouped together) */
SPELL("entangle",         "singed",
      P_EVOCATION_SPELL,   20,  1, 1, 1, IMMEDIATE, CLR_BRIGHT_BLUE),
SPELL("barkskin",         "torn",
      P_EVOCATION_SPELL,   20,  2, 2, 1, NODIR, CLR_GREEN),
SPELL("create grass",     "moldy",
      P_EVOCATION_SPELL,   20,  3, 3, 1, NODIR, CLR_BLUE),
SPELL("summon animal",    "blood-stained",
      P_EVOCATION_SPELL,   15,  4, 3, 1, NODIR, CLR_RED),
SPELL("stoneskin",        "moth-eaten",
      P_EVOCATION_SPELL,   15,  3, 4, 1, NODIR, CLR_MAGENTA),
SPELL("change metal to wood", "faded",
      P_EVOCATION_SPELL,   10,  6, 5, 1, IMMEDIATE, CLR_WHITE),
SPELL("create trees",     "smudged",
      P_EVOCATION_SPELL,   10,  7, 5, 1, NODIR, CLR_BRIGHT_GREEN),
SPELL("summon elemental", "mysterious",
      P_EVOCATION_SPELL,    5,  8, 6, 1, NODIR, CLR_CYAN),
/* books with fixed descriptions
 */
SPELL("blank paper", "plain", P_NONE, 15, 0, 0, 0, 0, HI_PAPER),
/* tribute book for 3.6 */
OBJECT(OBJ("novel", "paperback"),
       BITS(0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, P_NONE, PAPER),
       0, SPBOOK_CLASS, 0, 0, 0, 20, 0, 0, 0, 1, 20, CLR_BRIGHT_BLUE),
/* a special, one of a kind, spellbook */
OBJECT(OBJ("Book of the Dead", "papyrus"),
       BITS(0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, P_NONE, PAPER),
       0, SPBOOK_CLASS, 0, 0, 20, 10000, 0, 0, 0, 7, 20, HI_PAPER),
#undef SPELL

/* wands ... */
/* sum of all probabilty values equals 3000 - wands
   of wishing are much less likely to randomly spawn
   versus all other kinds */
#define WAND(name,typ,prob,cost,mgc,dir,metal,color) \
    OBJECT(OBJ(name, typ),                                              \
           BITS(0, 0, 1, 0, mgc, 1, 0, 0, 0, 0, dir, P_NONE, metal),    \
           0, WAND_CLASS, prob, 0, 7, cost, 0, 0, 0, 0, 30, color)
WAND("light",           "glass", 285, 100, 1, NODIR, GLASS, HI_GLASS),
WAND("secret door detection",
                        "balsa", 150, 150, 1, NODIR, WOOD, HI_WOOD),
WAND("enlightenment", "crystal",  58, 150, 1, NODIR, GLASS, HI_GLASS),
WAND("create monster",  "maple", 135, 200, 1, NODIR, WOOD, HI_WOOD),
WAND("wishing",          "pine",   1, 500, 1, NODIR, WOOD, HI_WOOD),
WAND("nothing",           "oak",  75, 500, 0, IMMEDIATE, WOOD, HI_WOOD),
WAND("striking",        "ebony", 225, 150, 1, IMMEDIATE, WOOD, HI_WOOD),
WAND("make invisible", "marble", 135, 150, 1, IMMEDIATE, MINERAL, HI_MINERAL),
WAND("slow monster",      "tin", 150, 150, 1, IMMEDIATE, STEEL, HI_METAL),
WAND("speed monster",  "bronze", 150, 150, 1, IMMEDIATE, BRONZE, HI_COPPER),
WAND("undead turning", "copper", 150, 150, 1, IMMEDIATE, COPPER, HI_COPPER),
WAND("polymorph",      "silver", 135, 200, 1, IMMEDIATE, SILVER, HI_SILVER),
WAND("cancellation", "platinum", 135, 200, 1, IMMEDIATE, PLATINUM, CLR_WHITE),
WAND("teleportation", "iridium", 135, 200, 1, IMMEDIATE, STEEL,
                                                             CLR_BRIGHT_CYAN),
WAND("opening",          "zinc",  75, 150, 1, IMMEDIATE, STEEL, HI_METAL),
WAND("locking",      "aluminum",  75, 150, 1, IMMEDIATE, STEEL, HI_METAL),
WAND("probing",       "uranium",  90, 150, 1, IMMEDIATE, STEEL, HI_METAL),
WAND("digging",          "iron", 165, 150, 1, RAY, IRON,  HI_METAL),
WAND("magic missile",   "steel", 150, 150, 1, RAY, STEEL, HI_METAL),
WAND("fire",        "hexagonal", 120, 175, 1, RAY, IRON,  HI_METAL),
WAND("cold",            "short", 120, 175, 1, RAY, IRON,  HI_METAL),
WAND("sleep",           "runed", 150, 175, 1, RAY, IRON,  HI_METAL),
WAND("death",            "long",  16, 500, 1, RAY, IRON,  HI_METAL),
WAND("lightning",      "curved", 120, 175, 1, RAY, IRON,  HI_METAL),
/* extra descriptions, shuffled into use at start of new game */
WAND(None,             "forked",  0, 150, 1, 0, WOOD, HI_WOOD),
WAND(None,             "spiked",  0, 150, 1, 0, IRON, HI_METAL),
WAND(None,            "jeweled",  0, 150, 1, 0, IRON, HI_MINERAL),
#undef WAND

/* coins ... - so far, gold is all there is */
#define COIN(name,prob,metal,worth) \
    OBJECT(OBJ(name, None),                                        \
           BITS(0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, P_NONE, metal),    \
           0, COIN_CLASS, prob, 0, 1, worth, 0, 0, 0, 0, 0, HI_GOLD)
COIN("gold piece", 1000, GOLD, 1),
#undef COIN

/* gems ... - includes stones and rocks but not boulders */
#define GEM(name,desc,prob,wt,gval,nutr,mohs,glass,color) \
    OBJECT(OBJ(name, desc),                                             \
           BITS(0, 1, 0, 0, 0, 0, 0, 0, 0,                              \
                HARDGEM(mohs), 0, -P_SLING, glass),                     \
           0, GEM_CLASS, prob, 0, 1, gval, 3, 3, 0, 0, nutr, color)
#define ROCK(name,desc,kn,prob,wt,gval,sdam,ldam,mgc,nutr,mohs,glass,color) \
    OBJECT(OBJ(name, desc),                                             \
           BITS(kn, 1, 0, 0, mgc, 0, 0, 0, 0,                           \
                HARDGEM(mohs), 0, -P_SLING, glass),                     \
           0, GEM_CLASS, prob, 0, wt, gval, sdam, ldam, 0, 0, nutr, color)
GEM("dilithium crystal", "white",  2, 1, 4500, 15,  5, GEMSTONE, CLR_WHITE),
GEM("diamond",           "white",  3, 1, 4000, 15, 10, GEMSTONE, CLR_WHITE),
GEM("ruby",                "red",  4, 1, 3500, 15,  9, GEMSTONE, CLR_RED),
GEM("jacinth",          "orange",  3, 1, 3250, 15,  9, GEMSTONE, CLR_ORANGE),
GEM("sapphire",           "blue",  4, 1, 3000, 15,  9, GEMSTONE, CLR_BLUE),
GEM("black opal",        "black",  3, 1, 2500, 15,  8, GEMSTONE, CLR_BLACK),
GEM("emerald",           "green",  5, 1, 2500, 15,  8, GEMSTONE, CLR_GREEN),
GEM("turquoise",         "green",  6, 1, 2000, 15,  6, GEMSTONE, CLR_GREEN),
GEM("citrine",          "yellow",  4, 1, 1500, 15,  6, GEMSTONE, CLR_YELLOW),
GEM("aquamarine",        "green",  6, 1, 1500, 15,  8, GEMSTONE, CLR_GREEN),
GEM("amber",   "yellowish brown",  8, 1, 1000, 15,  2, GEMSTONE, CLR_BROWN),
GEM("topaz",   "yellowish brown", 10, 1,  900, 15,  8, GEMSTONE, CLR_BROWN),
GEM("jet",               "black",  6, 1,  850, 15,  7, GEMSTONE, CLR_BLACK),
GEM("opal",              "white", 12, 1,  800, 15,  6, GEMSTONE, CLR_WHITE),
GEM("chrysoberyl",      "yellow",  8, 1,  700, 15,  5, GEMSTONE, CLR_YELLOW),
GEM("garnet",              "red", 12, 1,  700, 15,  7, GEMSTONE, CLR_RED),
GEM("amethyst",         "violet", 14, 1,  600, 15,  7, GEMSTONE, CLR_MAGENTA),
GEM("jasper",              "red", 15, 1,  500, 15,  7, GEMSTONE, CLR_RED),
GEM("fluorite",         "violet", 15, 1,  400, 15,  4, GEMSTONE, CLR_MAGENTA),
GEM("obsidian",          "black",  9, 1,  200, 15,  6, GEMSTONE, CLR_BLACK),
GEM("agate",            "orange", 12, 1,  200, 15,  6, GEMSTONE, CLR_ORANGE),
GEM("jade",              "green", 10, 1,  300, 15,  6, GEMSTONE, CLR_GREEN),
GEM("worthless piece of white glass", "white",
    74, 1, 0, 6, 5, GLASS, CLR_WHITE),
GEM("worthless piece of blue glass", "blue",
    74, 1, 0, 6, 5, GLASS, CLR_BLUE),
GEM("worthless piece of red glass", "red",
    73, 1, 0, 6, 5, GLASS, CLR_RED),
GEM("worthless piece of yellowish brown glass", "yellowish brown",
    73, 1, 0, 6, 5, GLASS, CLR_BROWN),
GEM("worthless piece of orange glass", "orange",
    73, 1, 0, 6, 5, GLASS, CLR_ORANGE),
GEM("worthless piece of yellow glass", "yellow",
    73, 1, 0, 6, 5, GLASS, CLR_YELLOW),
GEM("worthless piece of black glass", "black",
    73, 1, 0, 6, 5, GLASS, CLR_BLACK),
GEM("worthless piece of green glass", "green",
    74, 1, 0, 6, 5, GLASS, CLR_GREEN),
GEM("worthless piece of violet glass", "violet",
    74, 1, 0, 6, 5, GLASS, CLR_MAGENTA),

/* Placement note: there is a wishable subrange for
 * "gray stones" in the o_ranges[] array in objnam.c
 * that is currently everything between luckstones and flint
 * (inclusive).
 */
ROCK("luckstone", "gray",     0,  10,  10, 60, 3, 3, 1, 10, 7, MINERAL, CLR_GRAY),
ROCK("loadstone", "gray",     0,  10, 500,  1, 3, 3, 1, 10, 6, MINERAL, CLR_GRAY),
ROCK("touchstone", "gray",    0,   8,  10, 45, 3, 3, 1, 10, 6, MINERAL, CLR_GRAY),
ROCK("flint", "gray",         0,  10,   2,  1, 6, 6, 0, 10, 7, MINERAL, CLR_GRAY),
ROCK("sling bullet", "shiny", 0,  30,   6, 10, 6, 8, 0, 10, 7, IRON, HI_METAL),
ROCK("rock", None,            1, 100,  10,  0, 3, 3, 0, 10, 7, MINERAL, CLR_GRAY),
#undef GEM
#undef ROCK

/* miscellaneous ... */
/* Note: boulders and rocks are not normally created at random; the
 * probabilities only come into effect when you try to polymorph them.
 * Boulders weigh more than MAX_CARR_CAP; statues use corpsenm to take
 * on a specific type and may act as containers (both affect weight).
 */
OBJECT(OBJ("boulder", None),
       BITS(1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, P_NONE, MINERAL), 0,
       ROCK_CLASS, 100, 0, 6000, 0, 20, 20, 0, 0, 2000, HI_MINERAL),
OBJECT(OBJ("statue", None),
       BITS(1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, P_NONE, MINERAL), 0,
       ROCK_CLASS, 900, 0, 2500, 0, 20, 20, 0, 0, 2500, CLR_WHITE),

OBJECT(OBJ("heavy iron ball", None),
       BITS(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, WHACK, P_FLAIL, IRON), 0,
       BALL_CLASS, 1000, 0, 480, 10, 25, 25, 0, 0, 200, HI_METAL),
        /* +d4 when "very heavy" */
OBJECT(OBJ("iron chain", None),
       BITS(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, WHACK, P_NONE, IRON), 0,
       CHAIN_CLASS, 1000, 0, 120, 0, 4, 4, 0, 0, 200, HI_METAL),
        /* +1 both l & s */

/* Venom is normally a transitory missile (spit by various creatures)
 * but can be wished for in wizard mode so could occur in bones data.
 */
OBJECT(OBJ("blinding venom", "splash of venom"),
       BITS(0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, P_NONE, LIQUID), 0,
       VENOM_CLASS, 500, 0, 1, 0, 0, 0, 0, 0, 0, HI_ORGANIC),
OBJECT(OBJ("acid venom", "splash of venom"),
       BITS(0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, P_NONE, LIQUID), 0,
       VENOM_CLASS, 500, 0, 1, 0, 6, 6, 0, 0, 0, HI_ORGANIC),
OBJECT(OBJ("snowball", None),
       BITS(1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, P_NONE, LIQUID), 0,
       VENOM_CLASS, 500, 0, 1, 0, 6, 6, 0, 0, 0, HI_GLASS),
OBJECT(OBJ("ball of webbing", None),
       BITS(1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, P_NONE, LIQUID), 0,
       VENOM_CLASS, 500, 0, 1, 0, 0, 0, 0, 0, 0, CLR_WHITE),
OBJECT(OBJ("barbed needle", None),
       BITS(1, 1, 0, 0, 0, 0, 0, 1, 0, 0, PIERCE, P_NONE, WOOD), 0,
       VENOM_CLASS, 500, 0, 1, 0, 6, 6, 0, 0, 0, HI_WOOD),
        /* +d6 small or large */

/* fencepost, the deadly Array Terminator -- name [1st arg] *must* be NULL */
OBJECT(OBJ(None, None),
       BITS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, P_NONE, 0), 0,
       ILLOBJ_CLASS, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
}; /* objects[] */

#ifndef OBJECTS_PASS_2_

/* perform recursive compilation for second structure */
#undef OBJ
#undef OBJECT
#define OBJECTS_PASS_2_
#include "objects.c"

/* clang-format on */
/* *INDENT-ON* */

void NDECL(objects_init);

/* dummy routine used to force linkage */
void
objects_init()
{
    return;
}

#endif /* !OBJECTS_PASS_2_ */

/*objects.c*/
