/* NetHack 3.6	obj.h	$NHDT-Date: 1508827590 2017/10/24 06:46:30 $  $NHDT-Branch: NetHack-3.6.0 $:$NHDT-Revision: 1.60 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Michael Allison, 2006. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef OBJ_H
#define OBJ_H

/* #define obj obj_nh */ /* uncomment for SCO UNIX, which has a conflicting
                          * typedef for "obj" in <sys/types.h> */

union vptrs {
    struct obj *v_nexthere;   /* floor location lists */
    struct obj *v_ocontainer; /* point back to container */
    struct monst *v_ocarry;   /* point back to carrying monst */
};

/****
 ***	oextra -- collection of all object extensions
 **	(see the note at the bottom of this file before adding oextra fields)
 */
struct oextra {
    char *oname;          /* ptr to name of object */
    struct monst *omonst; /* ptr to attached monst struct */
    unsigned *omid;       /* ptr to m_id */
    long *olong;          /* ptr to misc long (temporary gold object) */
    char *omailcmd;       /* response_cmd for mail deliver */
};

struct obj {
    struct obj *nobj;
    union vptrs v;
#define nexthere v.v_nexthere
#define ocontainer v.v_ocontainer
#define ocarry v.v_ocarry

    struct obj *cobj; /* contents list for containers */
    unsigned o_id;
    xchar ox, oy;
    short otyp; /* object class number */
    unsigned owt;
    long quan; /* number of items */

    schar spe; /* quality of weapon, weptool, armor or ring (+ or -);
                  number of charges for wand or charged tool ( >= -1 );
                  number of candles attached to candelabrum;
                  marks your eggs, tin variety and spinach tins;
                  Schroedinger's Box (1) or royal coffers for a court (2);
                  tells which fruit a fruit is;
                  special for uball and amulet;
                  scroll of mail (normal==0, bones or wishing==1, written==2);
                  historic and gender for statues */
#define STATUE_HISTORIC 0x01
#define STATUE_MALE 0x02
#define STATUE_FEMALE 0x04
    char oclass;    /* object class */
    char invlet;    /* designation in inventory */
    char oartifact; /* artifact array index */

    long oprops;       /* item properties */
    long oprops_known; /* known item properties */

    xchar where;        /* where the object thinks it is */
#define OBJ_FREE 0      /* object not attached to anything */
#define OBJ_FLOOR 1     /* object on floor */
#define OBJ_CONTAINED 2 /* object in a container */
#define OBJ_INVENT 3    /* object in the hero's inventory */
#define OBJ_MINVENT 4   /* object in a monster inventory */
#define OBJ_MIGRATING 5 /* object sent off to another level */
#define OBJ_BURIED 6    /* object buried */
#define OBJ_ONBILL 7    /* object on shk bill */
#define OBJ_SOMEWHERE 8 /* for magic chest */
#define NOBJ_STATES 9
    xchar timed; /* # of fuses (timers) attached to this obj */

    Bitfield(cursed, 1);
    Bitfield(blessed, 1);
    Bitfield(unpaid, 1);    /* on some bill */
    Bitfield(no_charge, 1); /* if shk shouldn't charge for this */
    Bitfield(known, 1);     /* exact nature known */
    Bitfield(dknown, 1);    /* color or text known */
    Bitfield(bknown, 1);    /* blessing or curse known */
    Bitfield(rknown, 1);    /* rustproof or not known */

    Bitfield(oeroded, 2);  /* rusted/burnt weapon/armor */
    Bitfield(oeroded2, 2); /* corroded/rotted weapon/armor */
#define greatest_erosion(otmp)                                 \
    (int)((otmp)->oeroded > (otmp)->oeroded2 ? (otmp)->oeroded \
                                             : (otmp)->oeroded2)
#define MAX_ERODE 3
#define orotten oeroded  /* rotten food */
#define odiluted oeroded /* diluted potions */
#define norevive oeroded2 /* frozen corpses */
    Bitfield(oerodeproof, 1);  /* erodeproof weapon/armor */
    Bitfield(olocked, 1);      /* object is locked */
#define sokoprize olocked      /* special flag for sokoban prize */
#define orecursive olocked     /* special flag for preventing recursive calls */
    Bitfield(obroken, 1);      /* lock has been broken */
#define degraded_horn obroken  /* unicorn horn will poly to non-magic */
    Bitfield(otrapped, 1);     /* container is trapped */
/* or accidental tripped rolling boulder trap */
#define zombie_corpse otrapped /* zombie corpse that can revive */
    Bitfield(ocoated1, 1);     /* object (weapon) is coated in some type of poison */
    Bitfield(ocoated2, 1);     /* object (weapon) is coated in some other type of poison */
#define opoisoned ocoated1     /* object (weapon) is coated with poison */
#define otainted ocoated2      /* object (weapon) is coated with drow poison */

    Bitfield(recharged, 3); /* number of times it's been recharged */
#define on_ice recharged    /* corpse on ice */
    Bitfield(lamplit, 1);   /* a light-source -- can be lit */
    Bitfield(globby, 1);    /* combines with like types on adjacent squares */
    Bitfield(greased, 1);    /* covered with grease */
    Bitfield(nomerge, 1);    /* set temporarily to prevent merging */
    Bitfield(was_thrown, 1); /* thrown by hero since last picked up */

    Bitfield(material, 5); /* material this obj is made of */
    Bitfield(in_use, 1);   /* for magic items before useup items */
    Bitfield(bypass, 1);   /* mark this as an object to be skipped by bhito() */
    Bitfield(cknown, 1);   /* contents of container assumed to be known */

    Bitfield(lknown, 1);   /* locked/unlocked status is known */
    /* 7 free bits */

    int corpsenm;         /* type of corpse is mons[corpsenm] */
#define leashmon corpsenm /* gets m_id of attached pet */
#define fromsink corpsenm /* a potion from a sink */
#define novelidx corpsenm /* 3.6 tribute - the index of the novel title */
#define record_achieve_special corpsenm
#define dragonscales corpsenm /* dragon-scaled body armor
                               * (index into objects[], not mons[]) */
    int wep_kills;          /* tally number of kills a weapon has */
    int spestudied;         /* tally # of times a spellbook has been studied */
    int newwarncnt;         /* tally how many monsters a glow warning artifact is currently warning of */
    unsigned oeaten;        /* nutrition left in food, if partly eaten */
    unsigned lastwarncnt;   /* tally how many monsters a glow warning artifact was last warning of */
    long age;               /* creation date */
    long owornmask;
    struct oextra *oextra;  /* pointer to oextra struct */
};

#define newobj() (struct obj *) alloc(sizeof(struct obj))

/***
 **	oextra referencing and testing macros
 */

#define ONAME(o) ((o)->oextra->oname)
#define OMID(o) ((o)->oextra->omid)
#define OMONST(o) ((o)->oextra->omonst)
#define OLONG(o) ((o)->oextra->olong)
#define OMAILCMD(o) ((o)->oextra->omailcmd)

#define has_oname(o) ((o)->oextra && ONAME(o))
#define has_omid(o) ((o)->oextra && OMID(o))
#define has_omonst(o) ((o)->oextra && OMONST(o))
#define has_olong(o) ((o)->oextra && OLONG(o))
#define has_omailcmd(o) ((o)->oextra && OMAILCMD(o))

/* Weapons and weapon-tools */
/* KMH -- now based on skill categories.  Formerly:
 *	#define is_sword(otmp)	(otmp->oclass == WEAPON_CLASS && \
 *			 objects[otmp->otyp].oc_wepcat == WEP_SWORD)
 *	#define is_blade(otmp)	(otmp->oclass == WEAPON_CLASS && \
 *			 (objects[otmp->otyp].oc_wepcat == WEP_BLADE || \
 *			  objects[otmp->otyp].oc_wepcat == WEP_SWORD))
 *	#define is_weptool(o)	((o)->oclass == TOOL_CLASS && \
 *			 objects[(o)->otyp].oc_weptool)
 *	#define is_multigen(otyp) (otyp <= SHURIKEN)
 *	#define is_poisonable(otyp) (otyp <= BEC_DE_CORBIN)
 */
#define is_blade(otmp)                           \
    (otmp->oclass == WEAPON_CLASS                \
     && objects[otmp->otyp].oc_skill >= P_DAGGER \
     && objects[otmp->otyp].oc_skill <= P_SABER)
#define is_axe(otmp)                                              \
    ((otmp->oclass == WEAPON_CLASS || otmp->oclass == TOOL_CLASS) \
     && objects[otmp->otyp].oc_skill == P_AXE)
#define is_pick(otmp)                                             \
    ((otmp->oclass == WEAPON_CLASS || otmp->oclass == TOOL_CLASS) \
     && objects[otmp->otyp].oc_skill == P_PICK_AXE)
#define is_sword(otmp)                                \
    (otmp->oclass == WEAPON_CLASS                     \
     && objects[otmp->otyp].oc_skill >= P_SHORT_SWORD \
     && objects[otmp->otyp].oc_skill <= P_SABER)
#define is_hammer(otmp)                                           \
    ((otmp->oclass == WEAPON_CLASS || otmp->oclass == TOOL_CLASS) \
     && objects[otmp->otyp].oc_skill == P_HAMMER)
#define is_pole(otmp)                                             \
    ((otmp->oclass == WEAPON_CLASS || otmp->oclass == TOOL_CLASS) \
     && (objects[otmp->otyp].oc_skill == P_POLEARMS               \
         || objects[otmp->otyp].oc_skill == P_LANCE))
#define is_spear(otmp) \
    (otmp->oclass == WEAPON_CLASS && objects[otmp->otyp].oc_skill == P_SPEAR)
#define is_launcher(otmp) \
    (otmp->oclass == WEAPON_CLASS && objects[otmp->otyp].oc_skill >= P_BOW \
     && objects[otmp->otyp].oc_skill <= P_CROSSBOW)
#define is_2h_launcher(otmp) \
    (otmp->oclass == WEAPON_CLASS                         \
     && (objects[otmp->otyp].oc_skill == P_BOW            \
         || (objects[otmp->otyp].oc_skill == P_CROSSBOW   \
             && otmp->otyp != DARK_ELVEN_HAND_CROSSBOW)))
#define is_ammo(otmp)                                            \
    ((otmp->oclass == WEAPON_CLASS || otmp->oclass == GEM_CLASS) \
     && objects[otmp->otyp].oc_skill >= -P_CROSSBOW              \
     && objects[otmp->otyp].oc_skill <= -P_BOW)
#define matching_launcher(a, l) \
    ((l) && objects[(a)->otyp].oc_skill == -objects[(l)->otyp].oc_skill)
#define ammo_and_launcher(a, l) (is_ammo(a) && matching_launcher(a, l))
#define is_missile(otmp)                                          \
    ((otmp->oclass == WEAPON_CLASS || otmp->oclass == TOOL_CLASS) \
     && objects[otmp->otyp].oc_skill >= -P_BOOMERANG              \
     && objects[otmp->otyp].oc_skill <= -P_DART)
#define is_bullet(otmp) (otmp->otyp == SLING_BULLET)
#define is_weptool(o) \
    ((o)->oclass == TOOL_CLASS && objects[(o)->otyp].oc_skill != P_NONE)
        /* towel is not a weptool:  spe isn't an enchantment, cursed towel
           doesn't weld to hand, and twoweapon won't work with one */
#define is_barding(otmp) \
    (otmp->otyp == BARDING                   \
     || otmp->otyp == RUNED_BARDING          \
     || otmp->otyp == SPIKED_BARDING         \
     || otmp->otyp == BARDING_OF_REFLECTION)

#define is_lawful_artifact(otmp) \
    (otmp->oartifact && arti_align(otmp->oartifact) == A_LAWFUL)

#define is_chaotic_artifact(otmp) \
    (otmp->oartifact && arti_align(otmp->oartifact) == A_CHAOTIC)

#define is_magical_staff(otmp) \
    (otmp->otyp == STAFF_OF_DIVINATION || otmp->otyp == STAFF_OF_HEALING \
     || otmp->otyp == STAFF_OF_HOLINESS || otmp->otyp == STAFF_OF_MATTER \
     || otmp->otyp == STAFF_OF_ESCAPE || otmp->otyp == STAFF_OF_WAR      \
     || otmp->otyp == STAFF_OF_EVOCATION)

#define is_wet_towel(o) ((o)->otyp == TOWEL && (o)->spe > 0)
#define bimanual(otmp) \
    ((!(maybe_polyd(is_giant(youmonst.data), Race_if(PM_GIANT)))      \
      || is_2h_launcher(otmp))                                        \
     && (otmp->oclass == WEAPON_CLASS || otmp->oclass == TOOL_CLASS)  \
     && objects[otmp->otyp].oc_bimanual)
#define is_multigen(otmp)                           \
    (otmp->oclass == WEAPON_CLASS                   \
     && objects[otmp->otyp].oc_skill >= -P_SHURIKEN \
     && objects[otmp->otyp].oc_skill <= -P_BOW)
#define is_poisonable(otmp)       \
    (otmp->oclass == WEAPON_CLASS \
     && !is_launcher(otmp)        \
     && objects[otmp->otyp].oc_dir != WHACK)
#define uslinging() (uwep && objects[uwep->otyp].oc_skill == P_SLING)
/* 'is_quest_artifact()' only applies to the current role's artifact */
#define any_quest_artifact(o) ((o)->oartifact >= ART_XIUHCOATL)

/* Armor */
#define is_shield(otmp)          \
    (otmp->oclass == ARMOR_CLASS \
     && objects[otmp->otyp].oc_armcat == ARM_SHIELD)
#define is_helmet(otmp) \
    (otmp->oclass == ARMOR_CLASS && objects[otmp->otyp].oc_armcat == ARM_HELM)
#define is_boots(otmp)           \
    (otmp->oclass == ARMOR_CLASS \
     && objects[otmp->otyp].oc_armcat == ARM_BOOTS)
#define is_gloves(otmp)          \
    (otmp->oclass == ARMOR_CLASS \
     && objects[otmp->otyp].oc_armcat == ARM_GLOVES)
#define is_cloak(otmp)           \
    (otmp->oclass == ARMOR_CLASS \
     && objects[otmp->otyp].oc_armcat == ARM_CLOAK)
#define is_shirt(otmp)           \
    (otmp->oclass == ARMOR_CLASS \
     && objects[otmp->otyp].oc_armcat == ARM_SHIRT)
#define is_suit(otmp) \
    (otmp->oclass == ARMOR_CLASS && objects[otmp->otyp].oc_armcat == ARM_SUIT)
#define is_bracer(otmp) \
    ((otmp)->otyp == BRACERS                                                 \
     || (otmp)->otyp == DARK_ELVEN_BRACERS || (otmp)->otyp == RUNED_BRACERS)
#define is_elven_armor(otmp) \
    ((otmp)->otyp == ELVEN_HELM                                         \
     || (otmp)->otyp == ELVEN_CHAIN_MAIL || (otmp)->otyp == ELVEN_CLOAK \
     || (otmp)->otyp == ELVEN_SHIELD || (otmp)->otyp == ELVEN_BOOTS)
#define is_drow_armor(otmp) \
    ((otmp)->otyp == DARK_ELVEN_HELM || (otmp)->otyp == DARK_ELVEN_BOOTS          \
     || (otmp)->otyp == DARK_ELVEN_CHAIN_MAIL || (otmp)->otyp == DARK_ELVEN_TUNIC \
     || (otmp)->otyp == DARK_ELVEN_CLOAK || (otmp)->otyp == DARK_ELVEN_BRACERS    \
     || (otmp)->otyp == DARK_ELVEN_GLOVES)
#define is_orcish_armor(otmp) \
    ((otmp)->otyp == ORCISH_HELM || (otmp)->otyp == ORCISH_CHAIN_MAIL    \
     || (otmp)->otyp == ORCISH_RING_MAIL || (otmp)->otyp == ORCISH_CLOAK \
     || (otmp)->otyp == URUK_HAI_SHIELD || (otmp)->otyp == ORCISH_SHIELD \
     || (otmp)->otyp == ORCISH_BOOTS)
#define is_dwarvish_armor(otmp) \
    ((otmp)->otyp == DWARVISH_HELM            \
     || (otmp)->otyp == DWARVISH_CHAIN_MAIL   \
     || (otmp)->otyp == DWARVISH_CLOAK        \
     || (otmp)->otyp == DWARVISH_BOOTS        \
     || (otmp)->otyp == DWARVISH_ROUNDSHIELD)
#define is_gnomish_armor(otmp) (FALSE)

#define spit_object(otmp) \
    ((otmp)->otyp == ACID_VENOM || (otmp)->otyp == BLINDING_VENOM   \
     || (otmp)->otyp == SNOWBALL || (otmp)->otyp == BALL_OF_WEBBING \
     || (otmp)->otyp == BARBED_NEEDLE)

/* Eggs and other food */
#define MAX_EGG_HATCH_TIME 200 /* longest an egg can remain unhatched */
#define stale_egg(egg) \
    ((monstermoves - (egg)->age) > (2 * MAX_EGG_HATCH_TIME))
#define ofood(o) ((o)->otyp == CORPSE || (o)->otyp == EGG || (o)->otyp == TIN)
#define polyfodder(obj) \
    (ofood(obj) && (pm_to_cham((obj)->corpsenm) != NON_PM         \
                    || dmgtype(&mons[(obj)->corpsenm], AD_POLY)))
#define mlevelgain(obj) (ofood(obj) && (obj)->corpsenm == PM_WRAITH)
#define mhealup(obj) (ofood(obj) && (obj)->corpsenm == PM_NURSE)
#define is_royaljelly(o) ((o)->otyp == LUMP_OF_ROYAL_JELLY)
#define Is_pudding(o) \
    (o->otyp == GLOB_OF_GRAY_OOZE || o->otyp == GLOB_OF_BROWN_PUDDING \
     || o->otyp == GLOB_OF_GREEN_SLIME || o->otyp == GLOB_OF_BLACK_PUDDING)
#define is_gollumfood(obj) \
    (ofood(obj) && ((obj)->corpsenm == PM_PIRANHA        \
                    || (obj)->corpsenm == PM_GOBLIN      \
                    || (obj)->corpsenm == PM_HOBGOBLIN   \
                    || (obj)->corpsenm == PM_BAT         \
                    || (obj)->corpsenm == PM_GIANT_BAT))
#define all_food(o) \
    ((ofood(o) || (o)->oclass == FOOD_CLASS) && (o)->otyp != TIN)

/* Containers */
#define carried(o) ((o)->where == OBJ_INVENT)
#define mcarried(o) ((o)->where == OBJ_MINVENT)
#define Has_contents(o)                                \
    (/* (Is_container(o) || (o)->otyp == STATUE) && */ \
     (o)->cobj != (struct obj *) 0)
#define Is_container(o) ((o)->otyp >= LARGE_BOX && (o)->otyp <= BAG_OF_TRICKS)
#define Is_nonprize_container(o) (Is_container(o) && !is_soko_prize_flag(o))
#define Is_box(o) ((o)->otyp >= LARGE_BOX && (o)->otyp <= HIDDEN_CHEST)
#define Is_mbag(o) ((o)->otyp == BAG_OF_HOLDING || (o)->otyp == BAG_OF_TRICKS)
#define Is_allbag(o) ((o)->otyp >= SACK && (o)->otyp <= BAG_OF_TRICKS)
#define SchroedingersBox(o) ((o)->otyp == LARGE_BOX && (o)->spe == 1)
/* usually waterproof; random chance to be subjected to leakage if cursed;
   excludes statues, which aren't vulernable to water even when cursed */
#define Waterproof_container(o) \
    ((o)->otyp == OILSKIN_SACK || (o)->otyp == ICE_BOX || Is_box(o) \
     || (o)->oartifact == ART_BAG_OF_THE_HESPERIDES)

/* dragon gear
 * NOTE: this assumes that gray dragons come first and yellow last, as detailed
 * in monst.c. */
#define FIRST_DRAGON        PM_GRAY_DRAGON
#define LAST_DRAGON         PM_CHROMATIC_DRAGON
#define FIRST_DRAGON_SCALES GRAY_DRAGON_SCALES
#define LAST_DRAGON_SCALES  CHROMATIC_DRAGON_SCALES
#define Is_dragon_scales(obj) \
    ((obj)->otyp >= FIRST_DRAGON_SCALES && (obj)->otyp <= LAST_DRAGON_SCALES)
/* Note: dragonscales is corpsenm, and corpsenm is usually initialized to
 * NON_PM, which is -1. Thus, check for > 0 rather than just nonzero. */
#define Is_dragon_scaled_armor(obj) \
    (is_suit(obj) && (obj)->dragonscales > 0)
#define Is_dragon_armor(obj) \
    (Is_dragon_scales(obj) || Is_dragon_scaled_armor(obj))
/* any dragon armor -> FOO_DRAGON_SCALES object */
#define Dragon_armor_to_scales(obj) \
    (Is_dragon_scales(obj) ? (obj)->otyp : (obj)->dragonscales)
/* any dragon armor -> associated dragon PM_ constant */
#define Dragon_armor_to_pm(obj) \
    (FIRST_DRAGON \
     + (Is_dragon_scales(obj) ? (obj)->otyp : (obj)->dragonscales) \
     - FIRST_DRAGON_SCALES)
/* dragon PM_ constant -> dragon scales */
#define mndx_to_dragon_scales(mndx) \
    (FIRST_DRAGON_SCALES + (mndx - FIRST_DRAGON))

/* Elven gear */
#define is_elven_weapon(otmp) \
    ((otmp)->otyp == ELVEN_ARROW || (otmp)->otyp == ELVEN_SPEAR           \
     || (otmp)->otyp == ELVEN_DAGGER || (otmp)->otyp == ELVEN_SHORT_SWORD \
     || (otmp)->otyp == ELVEN_BROADSWORD || (otmp)->otyp == ELVEN_BOW     \
     || (otmp)->otyp == ELVEN_LONG_SWORD)
#define is_elven_obj(otmp) (is_elven_armor(otmp) || is_elven_weapon(otmp))

/* Drow gear */
#define is_drow_weapon(otmp) \
    ((otmp)->otyp == DARK_ELVEN_ARROW || (otmp)->otyp == DARK_ELVEN_CROSSBOW_BOLT    \
     || (otmp)->otyp == DARK_ELVEN_HAND_CROSSBOW || (otmp)->otyp == DARK_ELVEN_SPEAR \
     || (otmp)->otyp == DARK_ELVEN_DAGGER || (otmp)->otyp == DARK_ELVEN_SHORT_SWORD  \
     || (otmp)->otyp == DARK_ELVEN_BROADSWORD || (otmp)->otyp == DARK_ELVEN_BOW      \
     || (otmp)->otyp == DARK_ELVEN_LONG_SWORD || (otmp)->otyp == DARK_ELVEN_MACE     \
     || (otmp)->otyp == DARK_ELVEN_HEAVY_MACE)
#define is_drow_obj(otmp) (is_drow_armor(otmp) || is_drow_weapon(otmp))

/* Orcish gear */
#define is_orcish_obj(otmp) \
    (is_orcish_armor(otmp) || (otmp)->otyp == ORCISH_ARROW                     \
     || (otmp)->otyp == ORCISH_SPEAR || (otmp)->otyp == ORCISH_DAGGER          \
     || (otmp)->otyp == ORCISH_SHORT_SWORD || (otmp)->otyp == ORCISH_BOW       \
     || (otmp)->otyp == ORCISH_MORNING_STAR || (otmp)->otyp == ORCISH_SCIMITAR \
     || (otmp)->otyp == ORCISH_LONG_SWORD)

/* Dwarvish gear */
#define is_dwarvish_obj(otmp) \
    (is_dwarvish_armor(otmp) || (otmp)->otyp == DWARVISH_SPEAR \
     || (otmp)->otyp == DWARVISH_SHORT_SWORD                   \
     || (otmp)->otyp == DWARVISH_MATTOCK                       \
     || (otmp)->otyp == DWARVISH_BEARDED_AXE)

/* Gnomish gear */
#define is_gnomish_obj(otmp) (is_gnomish_armor(otmp))

/* Light sources */
#define Is_candle(otmp) \
    (otmp->otyp == TALLOW_CANDLE || otmp->otyp == WAX_CANDLE)
#define MAX_OIL_IN_FLASK 400 /* maximum amount of oil in a potion of oil */

/* MAGIC_LAMP intentionally excluded below */
/* age field of this is relative age rather than absolute */
#define age_is_relative(otmp) \
    ((otmp)->otyp == LANTERN || (otmp)->otyp == OIL_LAMP            \
     || (otmp)->otyp == CANDELABRUM_OF_INVOCATION                   \
     || (otmp)->otyp == TALLOW_CANDLE || (otmp)->otyp == WAX_CANDLE \
     || (otmp)->otyp == POT_OIL)
/* object can be ignited */
#define ignitable(otmp) \
    ((otmp)->otyp == LANTERN || (otmp)->otyp == OIL_LAMP            \
     || (otmp)->otyp == CANDELABRUM_OF_INVOCATION                   \
     || (otmp)->otyp == TALLOW_CANDLE || (otmp)->otyp == WAX_CANDLE \
     || (otmp)->otyp == POT_OIL)

/* things that can be read */
#define is_readable(otmp) \
    ((otmp)->otyp == FORTUNE_COOKIE || (otmp)->otyp == T_SHIRT             \
     || (otmp)->otyp == ALCHEMY_SMOCK || (otmp)->otyp == HAWAIIAN_SHIRT    \
     || (otmp)->otyp == CREDIT_CARD || (otmp)->otyp == CAN_OF_GREASE       \
     || (otmp)->otyp == MAGIC_MARKER || (otmp)->oclass == COIN_CLASS       \
     || (otmp)->oartifact == ART_GJALLAR || (otmp)->otyp == STRIPED_SHIRT  \
     || (otmp)->oartifact == ART_MAGIC___BALL || (otmp)->otyp == CANDY_BAR \
     || (otmp)->oartifact == ART_ONE_RING)

/* special stones */
#define is_graystone(obj) \
    ((obj)->otyp == LUCKSTONE || (obj)->otyp == LOADSTONE \
     || (obj)->otyp == FLINT || (obj)->otyp == TOUCHSTONE)

/* worthless glass -- assumes all GLASS * are worthless glass */
#define is_worthless_glass(obj) \
    ((obj)->oclass == GEM_CLASS && obj->material == GLASS)

/* misc helpers, simple enough to be macros */
#define is_flimsy(otmp) \
    (otmp->material <= LEATHER || otmp->material == DRAGON_HIDE \
     || otmp->otyp == RUBBER_HOSE)
#define is_plural(o) \
    ((o)->quan != 1L                                                    \
     /* "the Eyes of the Overworld" are plural, but                     \
        "a pair of lenses named the Eyes of the Overworld" is not */    \
     || ((o)->oartifact == ART_EYES_OF_THE_OVERWORLD                    \
         && !undiscovered_artifact(ART_EYES_OF_THE_OVERWORLD)))
#define pair_of(o) \
    ((o)->otyp == LENSES || (o)->otyp == GOGGLES \
     || is_gloves(o) || is_boots(o) || is_bracer(o))

#define bypass_forging_rules(obj) \
    ((obj)->otyp == SADDLE || (obj)->otyp == ROCK \
     || (obj)->oartifact == ART_MAGICBANE         \
     || (obj)->oartifact == ART_DRAGONBANE        \
     || (obj)->oartifact == ART_EYE_OF_VECNA      \
     || (obj)->oartifact == ART_HAND_OF_VECNA     \
     || (obj)->oartifact == ART_KEOLEWA)
#define is_magic(obj) \
    (((obj)->oprops & ITEM_PROP_MASK)   \
     || (objects[(obj)->otyp]).oc_magic \
     || (obj)->oartifact)

/* 'PRIZE' values override obj->corpsenm so prizes mustn't be object types
   which use that field for monster type (or other overloaded purpose) */
#define MINES_PRIZE 1
#define SOKO_PRIZE1 2
#define SOKO_PRIZE2 3
#define SOKO_PRIZE3 4
#define SOKO_PRIZE4 5
#define SOKO_PRIZE5 6
#define SOKO_PRIZE6 7
#define is_mines_prize(o) \
    ((o)->otyp == iflags.mines_prize_type \
     && (o)->record_achieve_special == MINES_PRIZE)
#define is_soko_prize(o) \
    (((o)->otyp == iflags.soko_prize_type1                            \
      && (o)->record_achieve_special == SOKO_PRIZE1)                  \
     || ((o)->otyp == iflags.soko_prize_type2                         \
         && (o)->record_achieve_special == SOKO_PRIZE2)               \
        || ((o)->otyp == iflags.soko_prize_type3                      \
            && (o)->record_achieve_special == SOKO_PRIZE3)            \
           || ((o)->otyp == iflags.soko_prize_type4                   \
               && (o)->record_achieve_special == SOKO_PRIZE4)         \
              || ((o)->otyp == iflags.soko_prize_type5                \
                  && (o)->record_achieve_special == SOKO_PRIZE5)      \
                 || ((o)->otyp == iflags.soko_prize_type6             \
                     && (o)->record_achieve_special == SOKO_PRIZE6))
#define is_soko_prize_flag(otmp) \
    ((otmp)->sokoprize && (!Is_box(otmp)))

/* Flags for get_obj_location(). */
#define CONTAINED_TOO 0x1
#define BURIED_TOO 0x2

/* object erosion types */
#define ERODE_BURN 0
#define ERODE_RUST 1
#define ERODE_ROT 2
#define ERODE_CORRODE 3
#define ERODE_FRACTURE 4
#define ERODE_DETERIORATE 5

/* erosion flags for erode_obj() */
#define EF_NONE 0
#define EF_GREASE 0x1  /* check for a greased object */
#define EF_DESTROY 0x2 /* potentially destroy the object */
#define EF_VERBOSE 0x4 /* print extra messages */
#define EF_PAY 0x8     /* it's the player's fault */

/* erosion return values for erode_obj(), water_damage() */
#define ER_NOTHING 0   /* nothing happened */
#define ER_GREASED 1   /* protected by grease */
#define ER_DAMAGED 2   /* object was damaged in some way */
#define ER_DESTROYED 3 /* object was destroyed */

/* propeller method for potionhit() */
#define POTHIT_HERO_BASH   0 /* wielded by hero */
#define POTHIT_HERO_THROW  1 /* thrown by hero */
#define POTHIT_MONST_THROW 2 /* thrown by a monster */
#define POTHIT_OTHER_THROW 3 /* propelled by some other means [scatter()] */

/* object properties */
#define ITEM_FIRE      0x00000001L /* fire damage or resistance */
#define ITEM_FROST     0x00000002L /* frost damage or resistance */
#define ITEM_SHOCK     0x00000004L /* shock damage or resistance */
#define ITEM_VENOM     0x00000008L /* poison damage or resistance */
#define ITEM_DRLI      0x00000010L /* drains life or resists it */
#define ITEM_OILSKIN   0x00000020L /* permanently greased */
#define ITEM_ESP       0x00000040L /* extrinsic telepathy */
#define ITEM_SEARCHING 0x00000080L /* extrinsic searching */
#define ITEM_WARNING   0x00000100L /* extrinsic warning */
#define ITEM_EXCEL     0x00000200L /* confers luck, charisma boost */
#define ITEM_FUMBLING  0x00000400L /* extrinsic fumbling */
#define ITEM_HUNGER    0x00000800L /* extrinsic hunger */

#define ITEM_MAGICAL   0x80000000L /* known to have magical properties */

#define ITEM_PROP_MASK 0x00000FFFL /* all current properties */
#define MAX_ITEM_PROPS 12

/*
 *  Notes for adding new oextra structures:
 *
 *	 1. Add the structure definition and any required macros in an
 *          appropriate header file that precedes this one.
 *	 2. Add a pointer to your new struct to oextra struct in this file.
 *	 3. Add a referencing macro to this file after the newobj macro above
 *	    (see ONAME, OMONST, OMIN, OLONG, or OMAILCMD for examples).
 *	 4. Add a testing macro after the set of referencing macros
 *	    (see has_oname(), has_omonst(), has_omin(), has_olong(),
 *	    has_omailcmd() for examples).
 *	 5. Create newXX(otmp) function and possibly free_XX(otmp) function
 *	    in an appropriate new or existing source file and add a prototype
 *	    for it to include/extern.h.  The majority of these are currently
 *	    located in mkobj.c for convenience.
 *
 *		void FDECL(newXX, (struct obj *));
 *	        void FDECL(free_XX, (struct obj *));
 *
 *	          void
 *	          newxx(otmp)
 *	          struct obj *otmp;
 *	          {
 *	              if (!otmp->oextra) otmp->oextra = newoextra();
 *	              if (!XX(otmp)) {
 *	                  XX(otmp) = (struct XX *)alloc(sizeof(struct xx));
 *	                  (void) memset((genericptr_t) XX(otmp),
 *	                             0, sizeof(struct xx));
 *	              }
 *	          }
 *
 *	 6. Adjust size_obj() in src/cmd.c appropriately.
 *	 7. Adjust dealloc_oextra() in src/mkobj.c to clean up
 *	    properly during obj deallocation.
 *	 8. Adjust copy_oextra() in src/mkobj.c to make duplicate
 *	    copies of your struct or data onto another obj struct.
 *	 9. Adjust restobj() in src/restore.c to deal with your
 *	    struct or data during a restore.
 *	10. Adjust saveobj() in src/save.c to deal with your
 *	    struct or data during a save.
 */

#endif /* OBJ_H */
