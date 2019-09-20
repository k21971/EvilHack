/* NetHack 3.6	prop.h	$NHDT-Date: 1547514641 2019/01/15 01:10:41 $  $NHDT-Branch: NetHack-3.6.2-beta01 $:$NHDT-Revision: 1.20 $ */
/* Copyright (c) 1989 Mike Threepoint				  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef PROP_H
#define PROP_H

/*** What the properties are ***
 *
 * note:  propertynames[] array in timeout.c has string values for these.
 *        Property #0 is not used.
 */
/* Resistances to troubles */
enum prop_types {
    FIRE_RES          =  1,
    COLD_RES          =  2,
    SLEEP_RES         =  3,
    DISINT_RES        =  4,
    SHOCK_RES         =  5,
    POISON_RES        =  6,
    ACID_RES          =  7,
    STONE_RES         =  8,
    /* note: for the first eight properties, MR_xxx == (1 << (xxx_RES - 1)) */
    PSYCHIC_RES       =  9,
    DRAIN_RES         = 10,
    SICK_RES          = 11,
    INVULNERABLE      = 12,
    ANTIMAGIC         = 13,
    /* Troubles */
    STUNNED           = 14,
    CONFUSION         = 15,
    BLINDED           = 16,
    DEAF              = 17,
    SICK              = 18,
    STONED            = 19,
    STRANGLED         = 20,
    VOMITING          = 21,
    GLIB              = 22,
    SLIMED            = 23,
    HALLUC            = 24,
    HALLUC_RES        = 25,
    FUMBLING          = 26,
    WOUNDED_LEGS      = 27,
    SLEEPY            = 28,
    HUNGER            = 29,
    /* Vision and senses */
    SEE_INVIS         = 30,
    TELEPAT           = 31,
    WARNING           = 32,
    WARN_OF_MON       = 33,
    WARN_UNDEAD       = 34,
    SEARCHING         = 35,
    CLAIRVOYANT       = 36,
    INFRAVISION       = 37,
    DETECT_MONSTERS   = 38,
    FOOD_SENSE        = 39,
    /* Appearance and behavior */
    ADORNED           = 40,
    INVIS             = 41,
    DISPLACED         = 42,
    STEALTH           = 43,
    AGGRAVATE_MONSTER = 44,
    CONFLICT          = 45,
    /* Transportation */
    JUMPING           = 46,
    TELEPORT          = 47,
    TELEPORT_CONTROL  = 48,
    LEVITATION        = 49,
    FLYING            = 50,
    WWALKING          = 51,
    SWIMMING          = 52,
    MAGICAL_BREATHING = 53,
    PASSES_WALLS      = 54,
    /* Physical attributes */
    SLOW_DIGESTION    = 55,
    HALF_SPDAM        = 56,
    HALF_PHDAM        = 57,
    REGENERATION      = 58,
    ENERGY_REGENERATION = 59,
    PROTECTION        = 60,
    PROT_FROM_SHAPE_CHANGERS = 61,
    POLYMORPH         = 62,
    POLYMORPH_CONTROL = 63,
    UNCHANGING        = 64,
    SLOW              = 65,
    FAST              = 66,
    REFLECTING        = 67,
    FREE_ACTION       = 68,
    FIXED_ABIL        = 69,
    LIFESAVED         = 70,
    VULN_FIRE         = 71,
    VULN_COLD         = 72,
    VULN_ELEC         = 73,
    VULN_ACID         = 74
};
#define LAST_PROP (VULN_ACID)

/*** Where the properties come from ***/
/* Definitions were moved here from obj.h and you.h */
struct prop {
    /*** Properties conveyed by objects ***/
    long extrinsic;
/* Armor */
#define W_ARM 0x00000001L  /* Body armor */
#define W_ARMC 0x00000002L /* Cloak */
#define W_ARMH 0x00000004L /* Helmet/hat */
#define W_ARMS 0x00000008L /* Shield */
#define W_ARMG 0x00000010L /* Gloves/gauntlets */
#define W_ARMF 0x00000020L /* Footwear */
#define W_ARMU 0x00000040L /* Undershirt */
#define W_ARMOR (W_ARM | W_ARMC | W_ARMH | W_ARMS | W_ARMG | W_ARMF | W_ARMU)
/* Weapons and artifacts */
#define W_WEP 0x00000100L     /* Wielded weapon */
#define W_QUIVER 0x00000200L  /* Quiver for (f)iring ammo */
#define W_SWAPWEP 0x00000400L /* Secondary weapon */
#define W_WEAPON (W_WEP | W_SWAPWEP | W_QUIVER)
#define W_ART 0x00001000L     /* Carrying artifact (not really worn) */
#define W_ARTI 0x00002000L    /* Invoked artifact  (not really worn) */
/* Amulets, rings, tools, and other items */
#define W_AMUL 0x00010000L    /* Amulet */
#define W_RINGL 0x00020000L   /* Left ring */
#define W_RINGR 0x00040000L   /* Right ring */
#define W_RING (W_RINGL | W_RINGR)
#define W_TOOL 0x00080000L   /* Eyewear */
#define W_ACCESSORY (W_RING | W_AMUL | W_TOOL)
    /* historical note: originally in slash'em, 'worn' saddle stayed in
       hero's inventory; in nethack, it's kept in the steed's inventory */
#define W_SADDLE 0x00100000L /* KMH -- For riding monsters */
#define W_BALL 0x00200000L   /* Punishment ball */
#define W_CHAIN 0x00400000L  /* Punishment chain */

    /*** Property is blocked by an object ***/
    long blocked; /* Same assignments as extrinsic */

    /*** Timeouts, permanent properties, and other flags ***/
    long intrinsic;
/* Timed properties */
#define TIMEOUT 0x00ffffffL     /* Up to 16 million turns */
                                /* Permanent properties */
#define FROMEXPER 0x01000000L   /* Gain/lose with experience, for role */
#define FROMRACE 0x02000000L    /* Gain/lose with experience, for race */
#define FROMOUTSIDE 0x04000000L /* By corpses, prayer, thrones, etc. */
#define HAVEPARTIAL 0x08000000L /* This is no longer a timeout, but a partial resistance */
#define INTRINSIC (FROMOUTSIDE | FROMRACE | FROMEXPER | HAVEPARTIAL)
/* Control flags */
#define FROMFORM 0x10000000L  /* Polyd; conferred by monster form */
#define I_SPECIAL 0x20000000L /* Property is controllable */
};

/*** Definitions for backwards compatibility ***/
#define LEFT_RING W_RINGL
#define RIGHT_RING W_RINGR
#define LEFT_SIDE LEFT_RING
#define RIGHT_SIDE RIGHT_RING
#define BOTH_SIDES (LEFT_SIDE | RIGHT_SIDE)
#define WORN_ARMOR W_ARM
#define WORN_CLOAK W_ARMC
#define WORN_HELMET W_ARMH
#define WORN_SHIELD W_ARMS
#define WORN_GLOVES W_ARMG
#define WORN_BOOTS W_ARMF
#define WORN_AMUL W_AMUL
#define WORN_BLINDF W_TOOL
#define WORN_SHIRT W_ARMU

#endif /* PROP_H */
