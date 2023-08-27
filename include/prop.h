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
    INVULNERABLE      = 13,
    ANTIMAGIC         = 14,
    /* Troubles */
    STUNNED           = 15,
    STUN_RES          = 16,
    CONFUSION         = 17,
    BLINDED           = 18,
    DEAF              = 19,
    SICK              = 20,
    STONED            = 21,
    STRANGLED         = 22,
    VOMITING          = 23,
    GLIB              = 24,
    SLIMED            = 25,
    HALLUC            = 26,
    HALLUC_RES        = 27,
    FUMBLING          = 28,
    WOUNDED_LEGS      = 29,
    SLEEPY            = 30,
    HUNGER            = 31,
    /* Vision and senses */
    SEE_INVIS         = 32,
    TELEPAT           = 33,
    WARNING           = 34,
    WARN_OF_MON       = 35,
    WARN_UNDEAD       = 36,
    SEARCHING         = 37,
    CLAIRVOYANT       = 38,
    INFRAVISION       = 39,
    ULTRAVISION       = 40,
    DETECT_MONSTERS   = 41,
    FOOD_SENSE        = 42,
    XRAY_VISION       = 43,
    /* Appearance and behavior */
    ADORNED           = 44,
    INVIS             = 45,
    DISPLACED         = 46,
    STEALTH           = 47,
    AGGRAVATE_MONSTER = 48,
    CONFLICT          = 49,
    /* Transportation */
    JUMPING           = 50,
    TELEPORT          = 51,
    TELEPORT_CONTROL  = 52,
    LEVITATION        = 53,
    FLYING            = 54,
    WWALKING          = 55,
    SWIMMING          = 56,
    MAGICAL_BREATHING = 57,
    PASSES_WALLS      = 58,
    /* Physical attributes */
    SLOW_DIGESTION    = 59,
    HALF_SPDAM        = 60,
    HALF_PHDAM        = 61,
    REGENERATION      = 62,
    ENERGY_REGENERATION = 63,
    PROTECTION        = 64,
    PROT_FROM_SHAPE_CHANGERS = 65,
    POLYMORPH         = 66,
    POLYMORPH_CONTROL = 67,
    UNCHANGING        = 68,
    SLOW              = 69,
    FAST              = 70,
    REFLECTING        = 71,
    FREE_ACTION       = 72,
    FIXED_ABIL        = 73,
    WITHERING         = 74,
    LIFESAVED         = 75,
    VULN_FIRE         = 76,
    VULN_COLD         = 77,
    VULN_ELEC         = 78,
    VULN_ACID         = 79
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
