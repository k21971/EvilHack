/* NetHack 3.6	prop.h	$NHDT-Date: 1570566360 2019/10/08 20:26:00 $  $NHDT-Branch: NetHack-3.6 $:$NHDT-Revision: 1.21 $ */
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
    PSYCHIC_RES       =  9,
    /* note: for the first nine properties, MR_xxx == (1 << (xxx_RES - 1)) */
    DRAIN_RES         = 10,
    SICK_RES          = 11,
    DEATH_RES         = 12,
    WERE_RES          = 13,
    INVULNERABLE      = 14,
    ANTIMAGIC         = 15,
    /* Troubles */
    STUNNED           = 16,
    STUN_RES          = 17,
    CONFUSION         = 18,
    BLINDED           = 19,
    DEAF              = 20,
    SICK              = 21,
    STONED            = 22,
    STRANGLED         = 23,
    VOMITING          = 24,
    GLIB              = 25,
    SLIMED            = 26,
    HALLUC            = 27,
    HALLUC_RES        = 28,
    FUMBLING          = 29,
    WOUNDED_LEGS      = 30,
    SLEEPY            = 31,
    HUNGER            = 32,
    /* Vision and senses */
    SEE_INVIS         = 33,
    TELEPAT           = 34,
    WARNING           = 35,
    WARN_OF_MON       = 36,
    WARN_UNDEAD       = 37,
    SEARCHING         = 38,
    CLAIRVOYANT       = 39,
    INFRAVISION       = 40,
    ULTRAVISION       = 41,
    DETECT_MONSTERS   = 42,
    FOOD_SENSE        = 43,
    XRAY_VISION       = 44,
    /* Appearance and behavior */
    ADORNED           = 45,
    INVIS             = 46,
    DISPLACED         = 47,
    STEALTH           = 48,
    AGGRAVATE_MONSTER = 49,
    CONFLICT          = 50,
    /* Transportation */
    JUMPING           = 51,
    TELEPORT          = 52,
    TELEPORT_CONTROL  = 53,
    LEVITATION        = 54,
    FLYING            = 55,
    WWALKING          = 56,
    SWIMMING          = 57,
    MAGICAL_BREATHING = 58,
    PASSES_WALLS      = 59,
    PASSES_TREES      = 60,
    /* Physical attributes */
    SLOW_DIGESTION    = 61,
    HALF_SPDAM        = 62,
    HALF_PHDAM        = 63,
    REGENERATION      = 64,
    ENERGY_REGENERATION = 65,
    PROTECTION        = 66,
    PROT_FROM_SHAPE_CHANGERS = 67,
    POLYMORPH         = 68,
    POLYMORPH_CONTROL = 69,
    UNCHANGING        = 70,
    SLOW              = 71,
    FAST              = 72,
    REFLECTING        = 73,
    FREE_ACTION       = 74,
    BARKSKIN          = 75,
    STONESKIN         = 76,
    FIXED_ABIL        = 77,
    WITHERING         = 78,
    LIFESAVED         = 79,
    VULN_FIRE         = 80,
    VULN_COLD         = 81,
    VULN_ELEC         = 82,
    VULN_ACID         = 83
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
#define W_WEAPONS (W_WEP | W_SWAPWEP | W_QUIVER)
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
#define W_SADDLE 0x00100000L  /* KMH -- For riding monsters */
#define W_BARDING 0x00200000L /* Barding for steeds */
#define W_BALL 0x00400000L    /* Punishment ball */
#define W_CHAIN 0x00800000L   /* Punishment chain */

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
