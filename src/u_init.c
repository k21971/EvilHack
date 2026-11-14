/* $NHDT-Header: NetHack 5.0 u_init.c 2026/06/20_22:26:43 NetHack-5.0:1.129 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2017. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* struct trobj is declared in decl.h; the role kit arrays below are
   shared with makemon.c so player monsters use the same kits */

staticfn long trquan(const struct trobj *);
staticfn struct obj *ini_inv_mkobj_filter(int, boolean);
staticfn short ini_inv_obj_substitution(const struct trobj *,
                                        struct obj *) NONNULLPTRS;
staticfn boolean ini_inv_adjust_obj(const struct trobj *,
                                    struct obj *) NONNULLPTRS;
staticfn void ini_inv_use_obj(struct obj *) NONNULLARG1;
staticfn void ini_inv(const struct trobj *) NONNULLARG1;
staticfn void knows_object(int, boolean);
staticfn void knows_class(char);
staticfn void u_init_role(void);
staticfn void u_init_race(void);
staticfn void pauper_reinit(void);
staticfn const struct def_skill *skills_for_role(void);
staticfn void u_init_carry_attr_boost(void);
staticfn boolean restricted_spell_discipline(int);

#define UNDEF_TYP 0
#define UNDEF_SPE '\177'
#define UNDEF_BLESS 2
#define CURSED 3

/*
 *      Initial inventory for the various roles.
 */

const struct trobj Archeologist[] = {
    /* if adventure has a name...  idea from tan@uvm-gen */
    { BULLWHIP, 2, WEAPON_CLASS, 1, 1, UNDEF_BLESS },
    { JACKET, 0, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { FEDORA, 0, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { FOOD_RATION, 0, FOOD_CLASS, 3, 3, 0 },
    { PICK_AXE, 0, TOOL_CLASS, 1, 1, UNDEF_BLESS },
    { TINNING_KIT, UNDEF_SPE, TOOL_CLASS, 1, 1, UNDEF_BLESS },
    { TOUCHSTONE, 0, GEM_CLASS, 1, 1, 0 },
    { SACK, 0, TOOL_CLASS, 1, 1, 0 },
    { 0, 0, 0, 0, 0, 0 }
};
const struct trobj Barbarian_0[] = {
    { TWO_HANDED_SWORD, 0, WEAPON_CLASS, 1, 1, UNDEF_BLESS },
    { AXE, 0, WEAPON_CLASS, 1, 1, UNDEF_BLESS },
    { RING_MAIL, 0, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { FOOD_RATION, 0, FOOD_CLASS, 1, 1, 0 },
    { 0, 0, 0, 0, 0, 0 }
};
const struct trobj Barbarian_1[] = {
    { BATTLE_AXE, 0, WEAPON_CLASS, 1, 1, UNDEF_BLESS },
    { SHORT_SWORD, 0, WEAPON_CLASS, 1, 1, UNDEF_BLESS },
    { RING_MAIL, 0, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { FOOD_RATION, 0, FOOD_CLASS, 1, 1, 0 },
    { 0, 0, 0, 0, 0, 0 }
};
const struct trobj Cave_man[] = {
    { CLUB, 1, WEAPON_CLASS, 1, 1, UNDEF_BLESS },
    { SLING, 2, WEAPON_CLASS, 1, 1, UNDEF_BLESS },
    { FLINT, 0, GEM_CLASS, 10, 20, UNDEF_BLESS },
    { SLING_BULLET, 0, GEM_CLASS, 1, 1, UNDEF_BLESS },
    { ARMOR, 0, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { 0, 0, 0, 0, 0, 0 }
};
const struct trobj Convict[] = {
    { ROCK, 0, GEM_CLASS, 1, 1, 0 },
    { STRIPED_SHIRT, 0, ARMOR_CLASS, 1, 1, 0 },
    { 0, 0, 0, 0, 0, 0 }
};
/* the Druid's initial weapon (quarterstaff or scimitar) and one of
   their spellbooks are chosen in u_init_role() */
const struct trobj Druid[] = {
    { BRACERS, 1, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { CLOAK, 0, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { EUCALYPTUS_LEAF, 0, FOOD_CLASS, 1, 1, 0 },
    { MISTLETOE, 0, FOOD_CLASS, 1, 1, 0 },
    { UNDEF_TYP, UNDEF_SPE, POTION_CLASS, 2, 2, UNDEF_BLESS },
    { SPE_ENTANGLE, 0, SPBOOK_CLASS, 1, 1, 1 },
    { SPE_CREATE_GRASS, 0, SPBOOK_CLASS, 1, 1, 1 },
    { SACK, 0, TOOL_CLASS, 1, 1, 0 },
    { 0, 0, 0, 0, 0, 0 }
};
const struct trobj Healer[] = {
    { SCALPEL, 0, WEAPON_CLASS, 1, 1, UNDEF_BLESS },
    { GLOVES, 1, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { STETHOSCOPE, 0, TOOL_CLASS, 1, 1, 0 },
    { POT_HEALING, 0, POTION_CLASS, 4, 4, UNDEF_BLESS },
    { POT_EXTRA_HEALING, 0, POTION_CLASS, 4, 4, UNDEF_BLESS },
    { WAN_SLEEP, UNDEF_SPE, WAND_CLASS, 1, 1, UNDEF_BLESS },
    /* always blessed, so it's guaranteed readable */
    { SPE_HEALING, 0, SPBOOK_CLASS, 1, 1, 1 },
    { SPE_EXTRA_HEALING, 0, SPBOOK_CLASS, 1, 1, 1 },
    { SPE_STONE_TO_FLESH, 0, SPBOOK_CLASS, 1, 1, 1 },
    { APPLE, 0, FOOD_CLASS, 5, 5, 0 },
    { 0, 0, 0, 0, 0, 0 }
};
static const struct trobj Infidel[] = {
    { AMULET_OF_YENDOR, 0, AMULET_CLASS, 1, 1, 0 },
    { DAGGER, 1, WEAPON_CLASS, 1, 1, 0 },
    { JACKET, 1, ARMOR_CLASS, 1, 1, CURSED },
    { CLOAK_OF_PROTECTION, 0, ARMOR_CLASS, 1, 1, CURSED },
    { POT_WATER, 0, POTION_CLASS, 3, 3, CURSED }, /* unholy water */
    { SCR_CHARGING, 0, SCROLL_CLASS, 2, 2, 0 },
    { SPE_DRAIN_LIFE, 0, SPBOOK_CLASS, 1, 1, 0 },
    { UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 1, 0 },
    { FIRE_HORN, UNDEF_SPE, TOOL_CLASS, 1, 1, 0 },
    { OILSKIN_SACK, 0, TOOL_CLASS, 1, 1, 0 },
    { 0, 0, 0, 0, 0, 0 }
};
static const struct trobj Draugr_Infidel[] = {
    { AMULET_OF_YENDOR, 0, AMULET_CLASS, 1, 1, 0 },
    { DAGGER, 1, WEAPON_CLASS, 1, 1, 0 },
    { JACKET, 1, ARMOR_CLASS, 1, 1, CURSED },
    { CLOAK_OF_PROTECTION, 0, ARMOR_CLASS, 1, 1, CURSED },
    { POT_WATER, 0, POTION_CLASS, 3, 3, CURSED }, /* unholy water */
    { SCR_CHARGING, 0, SCROLL_CLASS, 2, 2, 0 },
    { FIRE_HORN, UNDEF_SPE, TOOL_CLASS, 1, 1, 0 },
    { OILSKIN_SACK, 0, TOOL_CLASS, 1, 1, 0 },
    { 0, 0, 0, 0, 0, 0 }
};
const struct trobj Knight[] = {
    { LONG_SWORD, 1, WEAPON_CLASS, 1, 1, UNDEF_BLESS },
    { LANCE, 1, WEAPON_CLASS, 1, 1, UNDEF_BLESS },
    { PLATE_MAIL, 0, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { HELMET, 0, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { LARGE_SHIELD, 0, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { GAUNTLETS, 0, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { APPLE, 0, FOOD_CLASS, 10, 10, 0 },
    { CARROT, 0, FOOD_CLASS, 10, 10, 0 },
    { 0, 0, 0, 0, 0, 0 }
};
const struct trobj Elven_Knight[] = {
    { LONG_SWORD, 1, WEAPON_CLASS, 1, 1, UNDEF_BLESS },
    { LANCE, 1, WEAPON_CLASS, 1, 1, UNDEF_BLESS },
    { PLATE_MAIL, 0, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { HELMET, 0, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { ELVEN_SHIELD, 0, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { GAUNTLETS, 0, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { APPLE, 0, FOOD_CLASS, 10, 10, 0 },
    { CARROT, 0, FOOD_CLASS, 10, 10, 0 },
    { 0, 0, 0, 0, 0, 0 }
};
const struct trobj Dwarvish_Knight[] = {
    { LONG_SWORD, 1, WEAPON_CLASS, 1, 1, UNDEF_BLESS },
    { LANCE, 1, WEAPON_CLASS, 1, 1, UNDEF_BLESS },
    { PLATE_MAIL, 0, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { HELMET, 0, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { DWARVISH_ROUNDSHIELD, 0, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { GAUNTLETS, 0, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { APPLE, 0, FOOD_CLASS, 10, 10, 0 },
    { CARROT, 0, FOOD_CLASS, 10, 10, 0 },
    { 0, 0, 0, 0, 0, 0 }
};
const struct trobj Orcish_Knight[] = {
    { LONG_SWORD, 1, WEAPON_CLASS, 1, 1, UNDEF_BLESS },
    { LANCE, 1, WEAPON_CLASS, 1, 1, UNDEF_BLESS },
    { PLATE_MAIL, 0, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { HELMET, 0, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { ORCISH_SHIELD, 0, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { GAUNTLETS, 0, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { APPLE, 0, FOOD_CLASS, 10, 10, 0 },
    { CARROT, 0, FOOD_CLASS, 10, 10, 0 },
    { 0, 0, 0, 0, 0, 0 }
};
const struct trobj Draugr_Knight[] = {
    { LONG_SWORD, 1, WEAPON_CLASS, 1, 1, UNDEF_BLESS },
    { LANCE, 1, WEAPON_CLASS, 1, 1, UNDEF_BLESS },
    { PLATE_MAIL, 0, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { HELMET, 0, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { LARGE_SHIELD, 0, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { GAUNTLETS, 0, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { 0, 0, 0, 0, 0, 0 }
};
const struct trobj Monk[] = {
    { GLOVES, 2, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { ROBE, 1, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 1, 1, UNDEF_BLESS },
    { POT_HEALING, 0, POTION_CLASS, 3, 3, UNDEF_BLESS },
    { FOOD_RATION, 0, FOOD_CLASS, 3, 3, 0 },
    { APPLE, 0, FOOD_CLASS, 5, 5, UNDEF_BLESS },
    { ORANGE, 0, FOOD_CLASS, 5, 5, UNDEF_BLESS },
    /* Yes, we know fortune cookies aren't really from China.  They were
       invented by George Jung in Los Angeles, California, USA in 1916. */
    { FORTUNE_COOKIE, 0, FOOD_CLASS, 3, 3, UNDEF_BLESS },
    { 0, 0, 0, 0, 0, 0 }
};
const struct trobj Draugr_Monk[] = {
    { GLOVES, 2, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { ROBE, 1, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 1, 1, UNDEF_BLESS },
    { POT_HEALING, 0, POTION_CLASS, 3, 3, UNDEF_BLESS },
    { FOOD_RATION, 0, FOOD_CLASS, 3, 3, 0 },
    { FORTUNE_COOKIE, 0, FOOD_CLASS, 3, 3, UNDEF_BLESS },
    { 0, 0, 0, 0, 0, 0 }
};
const struct trobj Vampire_Monk[] = {
    { GLOVES, 2, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { ROBE, 1, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 1, 1, UNDEF_BLESS },
    { POT_VAMPIRE_BLOOD, 0, POTION_CLASS, 1, 1, UNDEF_BLESS },
    { POT_BLOOD, 0, POTION_CLASS, 3, 3, 0 },
    { 0, 0, 0, 0, 0, 0 }
};
const struct trobj Priest[] = {
    { MACE, 1, WEAPON_CLASS, 1, 1, 1 },
    { ROBE, 0, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { SMALL_SHIELD, 0, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { POT_WATER, 0, POTION_CLASS, 4, 4, 1 }, /* holy water */
    { CLOVE_OF_GARLIC, 0, FOOD_CLASS, 1, 1, 0 },
    { SPRIG_OF_WOLFSBANE, 0, FOOD_CLASS, 1, 1, 0 },
    { UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 2, 2, UNDEF_BLESS },
    { 0, 0, 0, 0, 0, 0 }
};
const struct trobj Ranger[] = {
    { DAGGER, 1, WEAPON_CLASS, 1, 1, UNDEF_BLESS },
    { BOW, 1, WEAPON_CLASS, 1, 1, UNDEF_BLESS },
    { ARROW, 2, WEAPON_CLASS, 50, 59, UNDEF_BLESS },
    { ARROW, 0, WEAPON_CLASS, 30, 39, UNDEF_BLESS },
    { CLOAK_OF_DISPLACEMENT, 2, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { CRAM_RATION, 0, FOOD_CLASS, 4, 4, 0 },
    { 0, 0, 0, 0, 0, 0 }
};
const struct trobj Rogue[] = {
    { SHORT_SWORD, 0, WEAPON_CLASS, 1, 1, UNDEF_BLESS },
    { DAGGER, 0, WEAPON_CLASS, 6, 15, 0 },
    { ARMOR, 1, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { POT_SICKNESS, 0, POTION_CLASS, 1, 1, 0 },
    { LOCK_PICK, 0, TOOL_CLASS, 1, 1, 0 },
    { TRAP_KIT, UNDEF_SPE, TOOL_CLASS, 1, 1, UNDEF_BLESS },
    { SACK, 0, TOOL_CLASS, 1, 1, 0 },
    { 0, 0, 0, 0, 0, 0 }
};
const struct trobj Drow_Rogue[] = {
    { SHORT_SWORD, 0, WEAPON_CLASS, 1, 1, UNDEF_BLESS },
    { DARK_ELVEN_HAND_CROSSBOW, 0, WEAPON_CLASS, 1, 1, UNDEF_BLESS },
    { DARK_ELVEN_CROSSBOW_BOLT, 0, WEAPON_CLASS, 26, 45, UNDEF_BLESS },
    { DARK_ELVEN_TUNIC, 0, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { POT_DROW_POISON, 0, POTION_CLASS, 2, 2, 0 },
    { LOCK_PICK, 0, TOOL_CLASS, 1, 1, 0 },
    { TRAP_KIT, UNDEF_SPE, TOOL_CLASS, 1, 1, UNDEF_BLESS },
    { SACK, 0, TOOL_CLASS, 1, 1, 0 },
    { 0, 0, 0, 0, 0, 0 }
};
const struct trobj Samurai[] = {
    { KATANA, 0, WEAPON_CLASS, 1, 1, UNDEF_BLESS },
    { SHORT_SWORD, 0, WEAPON_CLASS, 1, 1, UNDEF_BLESS }, /* wakizashi */
    { YUMI, 0, WEAPON_CLASS, 1, 1, UNDEF_BLESS },
    { YA, 0, WEAPON_CLASS, 26, 45, UNDEF_BLESS },
    { SPLINT_MAIL, 0, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { 0, 0, 0, 0, 0, 0 }
};
const struct trobj Tourist[] = {
    { DART, 2, WEAPON_CLASS, 21, 40, UNDEF_BLESS },
    { UNDEF_TYP, UNDEF_SPE, FOOD_CLASS, 10, 10, 0 },
    { POT_EXTRA_HEALING, 0, POTION_CLASS, 2, 2, UNDEF_BLESS },
    { SCR_MAGIC_MAPPING, 0, SCROLL_CLASS, 4, 4, UNDEF_BLESS },
    { HAWAIIAN_SHIRT, 0, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { EXPENSIVE_CAMERA, UNDEF_SPE, TOOL_CLASS, 1, 1, 0 },
    { CREDIT_CARD, 0, TOOL_CLASS, 1, 1, 0 },
    { 0, 0, 0, 0, 0, 0 }
};
const struct trobj Valkyrie[] = {
    { SPEAR, 1, WEAPON_CLASS, 1, 1, UNDEF_BLESS },
    { DAGGER, 0, WEAPON_CLASS, 1, 1, UNDEF_BLESS },
    { SMALL_SHIELD, 3, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { FOOD_RATION, 0, FOOD_CLASS, 1, 1, 0 },
    { 0, 0, 0, 0, 0, 0 }
};
const struct trobj Wizard[] = {
    { CLOAK_OF_MAGIC_RESISTANCE, 0, ARMOR_CLASS, 1, 1, UNDEF_BLESS },
    { WAN_MAGIC_MISSILE, UNDEF_SPE, WAND_CLASS, 1, 1, UNDEF_BLESS },
    { UNDEF_TYP, UNDEF_SPE, WAND_CLASS, 1, 1, UNDEF_BLESS },
    { UNDEF_TYP, UNDEF_SPE, RING_CLASS, 2, 2, UNDEF_BLESS },
    { UNDEF_TYP, UNDEF_SPE, POTION_CLASS, 3, 3, UNDEF_BLESS },
    { UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 3, 3, UNDEF_BLESS },
    { SPE_FORCE_BOLT, 0, SPBOOK_CLASS, 1, 1, 1 },
    { UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 1, UNDEF_BLESS },
    { 0, 0, 0, 0, 0, 0 }
};

/*
 *      Optional extra inventory items.
 */

const struct trobj Healing_book[] =
    { { SPE_HEALING, UNDEF_SPE, SPBOOK_CLASS, 1, 1, 1 },
      { 0, 0, 0, 0, 0, 0 } };
const struct trobj Protection_book[] =
    { { SPE_PROTECTION, UNDEF_SPE, SPBOOK_CLASS, 1, 1, 1 },
      { 0, 0, 0, 0, 0, 0 } };
const struct trobj Confuse_monster_book[] =
    { { SPE_CONFUSE_MONSTER, UNDEF_SPE, SPBOOK_CLASS, 1, 1, 1 },
      { 0, 0, 0, 0, 0, 0 } };
const struct trobj Tinopener[] =
    { { TIN_OPENER, 0, TOOL_CLASS, 1, 1, 0 },
      { 0, 0, 0, 0, 0, 0 } };
const struct trobj Lamp[] =
    { { OIL_LAMP, 1, TOOL_CLASS, 1, 1, 0 },
      { 0, 0, 0, 0, 0, 0 } };
const struct trobj Blindfold[] =
    { { BLINDFOLD, 0, TOOL_CLASS, 1, 1, 0 },
      { 0, 0, 0, 0, 0, 0 } };
static const struct trobj Xtra_food[] =
    { { UNDEF_TYP, UNDEF_SPE, FOOD_CLASS, 2, 2, 0},
      { 0, 0, 0, 0, 0, 0 } };
static const struct trobj Dra_food[] =
    { { EGG, UNDEF_SPE, FOOD_CLASS, 5, 5, 0 },
      { 0, 0, 0, 0, 0, 0 } };
const struct trobj Leash[] =
    { { LEASH, 0, TOOL_CLASS, 1, 1, 0 },
      { 0, 0, 0, 0, 0, 0 } };
const struct trobj Towel[] =
    { { TOWEL, 0, TOOL_CLASS, 1, 1, 0 },
      { 0, 0, 0, 0, 0, 0 } };
static const struct trobj Wishing[] =
    { { WAN_WISHING, 3, WAND_CLASS, 1, 1, 0 },
      { 0, 0, 0, 0, 0, 0 } };
static const struct trobj Money[] =
    { { GOLD_PIECE, 0, COIN_CLASS, 1, 1, 0 },
      { 0, 0, 0, 0, 0, 0 } };
static const struct trobj Tinningkit[] =
    { { TINNING_KIT, UNDEF_SPE, TOOL_CLASS, 1, 1, 0 },
      { 0, 0, 0, 0, 0, 0 } };
static const struct trobj Pickaxe[] =
    { { PICK_AXE, 0, TOOL_CLASS, 1, 1, 0 },
      { 0, 0, 0, 0, 0, 0 } };
static const struct trobj AoMR[] =
    { { AMULET_OF_MAGIC_RESISTANCE, 0, AMULET_CLASS, 1, 1, 0 },
      { 0, 0, 0, 0, 0, 0 } };
static const struct trobj Oilskin[] =
    { { OILSKIN_SACK, 0, TOOL_CLASS, 1, 1, 0 },
      { 0, 0, 0, 0, 0, 0 } };

/* race-based substitutions for initial inventory;
   the weaker cloak for elven rangers is intentional--they shoot better */
static const struct inv_sub {
    short race_pm, item_otyp, subs_otyp;
} inv_subs[] = {
    { PM_ELF, DAGGER, ELVEN_DAGGER },
    { PM_ELF, SPEAR, ELVEN_SPEAR },
    { PM_ELF, SHORT_SWORD, ELVEN_SHORT_SWORD },
    { PM_ELF, LONG_SWORD, ELVEN_LONG_SWORD },
    { PM_ELF, BOW, ELVEN_BOW },
    { PM_ELF, ARROW, ELVEN_ARROW },
    { PM_ELF, PLATE_MAIL, ELVEN_CHAIN_MAIL },
    { PM_ELF, HELMET, ELVEN_HELM },
    { PM_ELF, GAUNTLETS, GLOVES },
    /* { PM_ELF, SMALL_SHIELD, ELVEN_SHIELD }, */
    { PM_ELF, CLOAK, ELVEN_CLOAK },
    { PM_ELF, CLOAK_OF_DISPLACEMENT, ELVEN_CLOAK },
    { PM_ELF, CRAM_RATION, LEMBAS_WAFER },
    { PM_ORC, DAGGER, ORCISH_DAGGER },
    { PM_ORC, SPEAR, ORCISH_SPEAR },
    { PM_ORC, SHORT_SWORD, ORCISH_SHORT_SWORD },
    { PM_ORC, BOW, ORCISH_BOW },
    { PM_ORC, ARROW, ORCISH_ARROW },
    { PM_ORC, HELMET, ORCISH_HELM },
    /* { PM_ORC, SMALL_SHIELD, ORCISH_SHIELD }, */
    { PM_ORC, RING_MAIL, ORCISH_RING_MAIL },
    { PM_ORC, CHAIN_MAIL, ORCISH_CHAIN_MAIL },
    { PM_ORC, CRAM_RATION, TRIPE_RATION },
    { PM_ORC, LEMBAS_WAFER, TRIPE_RATION },
    { PM_ORC, LONG_SWORD, ORCISH_LONG_SWORD },
    { PM_DWARF, SPEAR, DWARVISH_SPEAR },
    { PM_DWARF, SHORT_SWORD, DWARVISH_SHORT_SWORD },
    { PM_DWARF, HELMET, DWARVISH_HELM },
    { PM_DWARF, PLATE_MAIL, DWARVISH_CHAIN_MAIL },
    /* { PM_DWARF, SMALL_SHIELD, DWARVISH_ROUNDSHIELD }, */
    /* { PM_DWARF, PICK_AXE, DWARVISH_MATTOCK }, */
    { PM_DWARF, LEMBAS_WAFER, CRAM_RATION },
    { PM_DWARF, LONG_SWORD, DWARVISH_BEARDED_AXE },
    { PM_GNOME, BOW, CROSSBOW },
    { PM_GNOME, ARROW, CROSSBOW_BOLT },
    /* Giants have special considerations */
    { PM_GIANT, ROBE, HIGH_BOOTS },
    { PM_GIANT, RING_MAIL, HELMET },
    { PM_GIANT, ARMOR, HELMET },
    { PM_GIANT, CLOAK_OF_MAGIC_RESISTANCE, LOW_BOOTS },
    { PM_GIANT, RIN_STEALTH, RIN_SEARCHING },
    { PM_GIANT, SPLINT_MAIL, LARGE_SPLINT_MAIL },
    { PM_GIANT, JACKET, LOW_BOOTS },
    { PM_GIANT, CLOAK_OF_PROTECTION, GAUNTLETS_OF_PROTECTION },
    { PM_GIANT, CLOAK, HIGH_BOOTS },
    /* Hobbits have a thing for elven gear */
    { PM_HOBBIT, DAGGER, ELVEN_DAGGER },
    { PM_HOBBIT, SPEAR, ELVEN_SPEAR },
    { PM_HOBBIT, SHORT_SWORD, ELVEN_SHORT_SWORD },
    { PM_HOBBIT, BOW, CROSSBOW }, /* Ranger */
    { PM_HOBBIT, ARROW, CROSSBOW_BOLT }, /* Ranger */
    { PM_HOBBIT, HELMET, ELVEN_HELM },
    { PM_HOBBIT, CLOAK, ELVEN_CLOAK },
    { PM_HOBBIT, CLOAK_OF_DISPLACEMENT, ELVEN_CLOAK },
    { PM_HOBBIT, CRAM_RATION, LEMBAS_WAFER },
    /* Tortles also have special considerations */
    { PM_TORTLE, JACKET, GLOVES },
    { PM_TORTLE, RING_MAIL, TOQUE },
    { PM_TORTLE, SPLINT_MAIL, TOQUE },
    { PM_TORTLE, BATTLE_AXE, TRIDENT },
    { PM_TORTLE, TWO_HANDED_SWORD, TRIDENT },
    { PM_TORTLE, CLOAK, TOQUE },
    { PM_TORTLE, ROBE, TOQUE },
    { PM_TORTLE, HAWAIIAN_SHIRT, TOQUE },
    { PM_TORTLE, CLOAK_OF_MAGIC_RESISTANCE, GLOVES },
    { PM_TORTLE, SACK, OILSKIN_SACK },
    /* Drow have their own special gear */
    { PM_DROW, POT_SICKNESS, POT_DROW_POISON },
    { PM_DROW, DAGGER, DARK_ELVEN_DAGGER },
    { PM_DROW, SPEAR, DARK_ELVEN_SPEAR },
    { PM_DROW, SHORT_SWORD, DARK_ELVEN_SHORT_SWORD },
    { PM_DROW, LONG_SWORD, DARK_ELVEN_LONG_SWORD },
    { PM_DROW, MACE, DARK_ELVEN_MACE },
    { PM_DROW, BOW, DARK_ELVEN_BOW },
    { PM_DROW, ARROW, DARK_ELVEN_ARROW },
    { PM_DROW, CROSSBOW, DARK_ELVEN_HAND_CROSSBOW },
    { PM_DROW, CROSSBOW_BOLT, DARK_ELVEN_CROSSBOW_BOLT },
    { PM_DROW, GLOVES, DARK_ELVEN_GLOVES },
    { PM_DROW, GAUNTLETS, DARK_ELVEN_GLOVES },
    { PM_DROW, ARMOR, DARK_ELVEN_TUNIC },
    { PM_DROW, JACKET, DARK_ELVEN_TUNIC },
    { PM_DROW, PLATE_MAIL, DARK_ELVEN_CHAIN_MAIL },
    { PM_DROW, HELMET, DARK_ELVEN_HELM },
    { PM_DROW, SMALL_SHIELD, DARK_ELVEN_BRACERS },
    { PM_DROW, LARGE_SHIELD, DARK_ELVEN_BRACERS },
    { PM_DROW, CLOAK_OF_DISPLACEMENT, DARK_ELVEN_CLOAK },
    { PM_DROW, CLOAK_OF_PROTECTION, DARK_ELVEN_CLOAK },
    /* Draugr */
    { PM_DRAUGR, FOOD_RATION, EGG },
    { PM_DRAUGR, FORTUNE_COOKIE, EGG },
    /* Vampires */
    { PM_VAMPIRE, POT_SICKNESS, POT_BLOOD },
    { PM_VAMPIRE, FOOD_RATION, POT_BLOOD },
    /* end */
    { NON_PM, STRANGE_OBJECT, STRANGE_OBJECT }
};

static const struct def_skill Skill_A[] = {
    { P_DAGGER, P_BASIC },
    { P_PICK_AXE, P_EXPERT },
    { P_SHORT_SWORD, P_BASIC },
    { P_SABER, P_EXPERT },
    { P_CLUB, P_BASIC },
    { P_QUARTERSTAFF, P_SKILLED },
    { P_SPEAR, P_EXPERT },
    { P_SLING, P_SKILLED },
    { P_DART, P_BASIC },
    { P_BOOMERANG, P_EXPERT },
    { P_WHIP, P_EXPERT },
    { P_UNICORN_HORN, P_SKILLED },
    { P_ATTACK_SPELL, P_BASIC },
    { P_HEALING_SPELL, P_BASIC },
    { P_DIVINATION_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_BASIC },
    { P_RIDING, P_BASIC },
    { P_TWO_WEAPON_COMBAT, P_BASIC },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_SHIELD, P_BASIC },
    { P_PET_HANDLING, P_BASIC },
    { P_NONE, 0 }
};
static const struct def_skill Skill_B[] = {
    { P_DAGGER, P_BASIC },
    { P_AXE, P_EXPERT },
    { P_PICK_AXE, P_SKILLED },
    { P_SHORT_SWORD, P_EXPERT },
    { P_BROAD_SWORD, P_SKILLED },
    { P_LONG_SWORD, P_SKILLED },
    { P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SABER, P_SKILLED },
    { P_CLUB, P_SKILLED },
    { P_MACE, P_SKILLED },
    { P_FLAIL, P_BASIC },
    { P_HAMMER, P_EXPERT },
    { P_QUARTERSTAFF, P_BASIC },
    { P_SPEAR, P_SKILLED },
    { P_TRIDENT, P_SKILLED },
    { P_BOW, P_BASIC },
    { P_ENCHANTMENT_SPELL, P_BASIC }, /* special spell is cause fear */
    { P_RIDING, P_SKILLED },
    { P_TWO_WEAPON_COMBAT, P_SKILLED },
    { P_BARE_HANDED_COMBAT, P_MASTER },
    { P_SHIELD, P_SKILLED },
    { P_PET_HANDLING, P_BASIC },
    { P_NONE, 0 }
};
static const struct def_skill Skill_Dra_B[] = {
    { P_DAGGER, P_BASIC },
    { P_AXE, P_EXPERT },
    { P_PICK_AXE, P_SKILLED },
    { P_SHORT_SWORD, P_EXPERT },
    { P_BROAD_SWORD, P_SKILLED },
    { P_LONG_SWORD, P_SKILLED },
    { P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SABER, P_SKILLED },
    { P_CLUB, P_SKILLED },
    { P_MACE, P_SKILLED },
    { P_FLAIL, P_BASIC },
    { P_HAMMER, P_EXPERT },
    { P_QUARTERSTAFF, P_BASIC },
    { P_SPEAR, P_SKILLED },
    { P_TRIDENT, P_SKILLED },
    { P_BOW, P_BASIC },
    { P_RIDING, P_SKILLED },
    { P_TWO_WEAPON_COMBAT, P_SKILLED },
    { P_BARE_HANDED_COMBAT, P_MASTER },
    { P_SHIELD, P_SKILLED },
    { P_PET_HANDLING, P_BASIC },
    { P_NONE, 0 }
};
static const struct def_skill Skill_C[] = {
    { P_DAGGER, P_SKILLED },
    { P_AXE, P_SKILLED },
    { P_PICK_AXE, P_BASIC },
    { P_CLUB, P_EXPERT },
    { P_MACE, P_EXPERT },
    { P_FLAIL, P_SKILLED },
    { P_HAMMER, P_SKILLED },
    { P_QUARTERSTAFF, P_EXPERT },
    { P_POLEARMS, P_SKILLED },
    { P_SPEAR, P_EXPERT },
    { P_TRIDENT, P_SKILLED },
    { P_BOW, P_SKILLED },
    { P_SLING, P_EXPERT },
    { P_BOOMERANG, P_EXPERT },
    { P_UNICORN_HORN, P_BASIC },
    { P_RIDING, P_BASIC },
    { P_BARE_HANDED_COMBAT, P_MASTER },
    { P_SHIELD, P_BASIC },
    { P_PET_HANDLING, P_SKILLED },
    { P_NONE, 0 }
};
static const struct def_skill Skill_Con[] = {
    { P_DAGGER, P_EXPERT },
    { P_HAMMER, P_SKILLED },
    { P_PICK_AXE, P_EXPERT },
    { P_CLUB, P_EXPERT },
    { P_MACE, P_BASIC },
    { P_DART, P_SKILLED },
    { P_FLAIL, P_EXPERT },
    { P_SHORT_SWORD, P_BASIC },
    { P_BROAD_SWORD, P_SKILLED },
    { P_SLING, P_SKILLED },
    { P_ATTACK_SPELL, P_BASIC },
    { P_ESCAPE_SPELL, P_EXPERT },
    { P_RIDING, P_BASIC },
    { P_TWO_WEAPON_COMBAT, P_SKILLED },
    { P_BARE_HANDED_COMBAT, P_SKILLED },
    { P_THIEVERY, P_SKILLED },
    { P_SHIELD, P_BASIC },
    { P_PET_HANDLING, P_BASIC },
    { P_NONE, 0 }
};
static const struct def_skill Skill_Dra_Con[] = {
    { P_DAGGER, P_EXPERT },
    { P_HAMMER, P_SKILLED },
    { P_PICK_AXE, P_EXPERT },
    { P_CLUB, P_EXPERT },
    { P_MACE, P_BASIC },
    { P_DART, P_SKILLED },
    { P_FLAIL, P_EXPERT },
    { P_SHORT_SWORD, P_BASIC },
    { P_BROAD_SWORD, P_SKILLED },
    { P_SLING, P_SKILLED },
    { P_RIDING, P_BASIC },
    { P_TWO_WEAPON_COMBAT, P_SKILLED },
    { P_BARE_HANDED_COMBAT, P_SKILLED },
    { P_THIEVERY, P_SKILLED },
    { P_SHIELD, P_BASIC },
    { P_PET_HANDLING, P_BASIC },
    { P_NONE, 0 }
};
static const struct def_skill Skill_D[] = {
    { P_DAGGER, P_BASIC },
    { P_SABER, P_SKILLED },
    { P_CLUB, P_BASIC },
    { P_MACE, P_BASIC },
    { P_HAMMER, P_BASIC },
    { P_QUARTERSTAFF, P_SKILLED },
    { P_SPEAR, P_BASIC },
    { P_SLING, P_BASIC },
    { P_UNICORN_HORN, P_BASIC },
    { P_CLERIC_SPELL, P_SKILLED },
    { P_ENCHANTMENT_SPELL, P_BASIC },
    { P_EVOCATION_SPELL, P_EXPERT },
    { P_HEALING_SPELL, P_BASIC },
    { P_MATTER_SPELL, P_BASIC },
    { P_RIDING, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_SKILLED },
    { P_PET_HANDLING, P_MASTER },
    { P_NONE, 0 }
};
static const struct def_skill Skill_H[] = {
    { P_DAGGER, P_EXPERT },
    { P_SHORT_SWORD, P_SKILLED },
    { P_SABER, P_BASIC },
    { P_CLUB, P_SKILLED },
    { P_MACE, P_BASIC },
    { P_QUARTERSTAFF, P_EXPERT },
    { P_POLEARMS, P_BASIC },
    { P_SPEAR, P_BASIC },
    { P_TRIDENT, P_BASIC },
    { P_SLING, P_SKILLED },
    { P_DART, P_EXPERT },
    { P_SHURIKEN, P_SKILLED },
    { P_UNICORN_HORN, P_EXPERT },
    { P_HEALING_SPELL, P_EXPERT },
    { P_CLERIC_SPELL, P_SKILLED },
    { P_RIDING, P_BASIC },
    { P_BARE_HANDED_COMBAT, P_BASIC },
    { P_PET_HANDLING, P_EXPERT },
    { P_NONE, 0 }
};
static const struct def_skill Skill_Inf[] = {
    { P_DAGGER, P_EXPERT },
    { P_SHORT_SWORD, P_SKILLED },
    { P_BROAD_SWORD, P_BASIC },
    { P_SABER, P_SKILLED },
    { P_CLUB, P_BASIC },
    { P_HAMMER, P_BASIC },
    { P_QUARTERSTAFF, P_SKILLED },
    { P_POLEARMS, P_SKILLED },
    { P_SPEAR, P_BASIC },
    { P_SLING, P_BASIC },
    { P_CROSSBOW, P_SKILLED },
    { P_DART, P_BASIC },
    { P_WHIP, P_SKILLED },
    { P_ATTACK_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_SKILLED },
    { P_ENCHANTMENT_SPELL, P_BASIC },
    { P_CLERIC_SPELL, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_SKILLED },
    { P_RIDING, P_SKILLED },
    { P_PET_HANDLING, P_BASIC },
    { P_NONE, 0 }
};
static const struct def_skill Skill_Dra_Inf[] = {
    { P_DAGGER, P_EXPERT },
    { P_SHORT_SWORD, P_SKILLED },
    { P_BROAD_SWORD, P_BASIC },
    { P_SABER, P_SKILLED },
    { P_CLUB, P_BASIC },
    { P_HAMMER, P_BASIC },
    { P_QUARTERSTAFF, P_SKILLED },
    { P_POLEARMS, P_SKILLED },
    { P_SPEAR, P_BASIC },
    { P_SLING, P_BASIC },
    { P_CROSSBOW, P_SKILLED },
    { P_DART, P_BASIC },
    { P_WHIP, P_SKILLED },
    { P_BARE_HANDED_COMBAT, P_SKILLED },
    { P_RIDING, P_SKILLED },
    { P_PET_HANDLING, P_BASIC },
    { P_NONE, 0 }
};
static const struct def_skill Skill_K[] = {
    { P_DAGGER, P_BASIC },
    { P_AXE, P_EXPERT },
    { P_PICK_AXE, P_BASIC },
    { P_SHORT_SWORD, P_SKILLED },
    { P_BROAD_SWORD, P_EXPERT },
    { P_LONG_SWORD, P_EXPERT },
    { P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SABER, P_SKILLED },
    { P_CLUB, P_BASIC },
    { P_MACE, P_BASIC },
    { P_FLAIL, P_BASIC },
    { P_HAMMER, P_BASIC },
    { P_POLEARMS, P_EXPERT },
    { P_SPEAR, P_EXPERT },
    { P_TRIDENT, P_BASIC },
    { P_LANCE, P_EXPERT },
    { P_BOW, P_BASIC },
    { P_CROSSBOW, P_SKILLED },
    { P_ATTACK_SPELL, P_BASIC },
    { P_HEALING_SPELL, P_BASIC },
    { P_CLERIC_SPELL, P_SKILLED },
    { P_RIDING, P_EXPERT },
    { P_TWO_WEAPON_COMBAT, P_SKILLED },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_SHIELD, P_EXPERT },
    { P_PET_HANDLING, P_MASTER },
    { P_NONE, 0 }
};
static const struct def_skill Skill_Dra_K[] = {
    { P_DAGGER, P_BASIC },
    { P_AXE, P_EXPERT },
    { P_PICK_AXE, P_BASIC },
    { P_SHORT_SWORD, P_SKILLED },
    { P_BROAD_SWORD, P_EXPERT },
    { P_LONG_SWORD, P_EXPERT },
    { P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SABER, P_SKILLED },
    { P_CLUB, P_BASIC },
    { P_MACE, P_BASIC },
    { P_FLAIL, P_BASIC },
    { P_HAMMER, P_BASIC },
    { P_POLEARMS, P_EXPERT },
    { P_SPEAR, P_EXPERT },
    { P_TRIDENT, P_BASIC },
    { P_LANCE, P_EXPERT },
    { P_BOW, P_BASIC },
    { P_CROSSBOW, P_SKILLED },
    { P_RIDING, P_EXPERT },
    { P_TWO_WEAPON_COMBAT, P_SKILLED },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_SHIELD, P_EXPERT },
    { P_PET_HANDLING, P_MASTER },
    { P_NONE, 0 }
};
static const struct def_skill Skill_Mon[] = {
    { P_QUARTERSTAFF, P_EXPERT },
    { P_SHURIKEN, P_BASIC },
    { P_SPEAR, P_SKILLED },
    { P_TRIDENT, P_SKILLED },
    { P_BROAD_SWORD, P_BASIC },
    { P_ATTACK_SPELL, P_BASIC },
    { P_HEALING_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_BASIC },
    { P_ENCHANTMENT_SPELL, P_BASIC },
    { P_CLERIC_SPELL, P_SKILLED },
    { P_ESCAPE_SPELL, P_SKILLED },
    { P_MATTER_SPELL, P_BASIC },
    { P_EVOCATION_SPELL, P_BASIC },
    { P_RIDING, P_BASIC },
    { P_MARTIAL_ARTS, P_GRAND_MASTER },
    { P_PET_HANDLING, P_SKILLED },
    { P_NONE, 0 }
};
static const struct def_skill Skill_Dra_Mon[] = {
    { P_QUARTERSTAFF, P_EXPERT },
    { P_SHURIKEN, P_BASIC },
    { P_SPEAR, P_SKILLED },
    { P_TRIDENT, P_SKILLED },
    { P_BROAD_SWORD, P_BASIC },
    { P_RIDING, P_BASIC },
    { P_MARTIAL_ARTS, P_GRAND_MASTER },
    { P_PET_HANDLING, P_SKILLED },
    { P_NONE, 0 }
};
static const struct def_skill Skill_P[] = {
    { P_CLUB, P_EXPERT },
    { P_MACE, P_EXPERT },
    { P_HAMMER, P_EXPERT },
    { P_FLAIL, P_EXPERT },
    { P_QUARTERSTAFF, P_EXPERT },
    { P_POLEARMS, P_SKILLED },
    { P_SLING, P_BASIC },
    { P_BOOMERANG, P_BASIC },
    { P_HEALING_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_EXPERT },
    { P_CLERIC_SPELL, P_EXPERT },
    { P_EVOCATION_SPELL, P_BASIC },
    { P_RIDING, P_BASIC },
    { P_BARE_HANDED_COMBAT, P_BASIC },
    { P_SHIELD, P_BASIC },
    { P_PET_HANDLING, P_SKILLED },
    { P_NONE, 0 }
};
static const struct def_skill Skill_R[] = {
    { P_DAGGER, P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },
    { P_BROAD_SWORD, P_SKILLED },
    { P_LONG_SWORD, P_SKILLED },
    { P_TWO_HANDED_SWORD, P_BASIC },
    { P_SABER, P_SKILLED },
    { P_CLUB, P_SKILLED },
    { P_MACE, P_SKILLED },
    { P_FLAIL, P_BASIC },
    { P_HAMMER, P_BASIC },
    { P_POLEARMS, P_BASIC },
    { P_SPEAR, P_BASIC },
    { P_CROSSBOW, P_EXPERT },
    { P_DART, P_EXPERT },
    { P_SHURIKEN, P_SKILLED },
    { P_DIVINATION_SPELL, P_SKILLED },
    { P_ESCAPE_SPELL, P_SKILLED },
    { P_MATTER_SPELL, P_SKILLED },
    { P_RIDING, P_BASIC },
    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_THIEVERY, P_MASTER },
    { P_SHIELD, P_BASIC },
    { P_PET_HANDLING, P_BASIC },
    { P_NONE, 0 }
};
static const struct def_skill Skill_Dra_R[] = {
    { P_DAGGER, P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },
    { P_BROAD_SWORD, P_SKILLED },
    { P_LONG_SWORD, P_SKILLED },
    { P_TWO_HANDED_SWORD, P_BASIC },
    { P_SABER, P_SKILLED },
    { P_CLUB, P_SKILLED },
    { P_MACE, P_SKILLED },
    { P_FLAIL, P_BASIC },
    { P_HAMMER, P_BASIC },
    { P_POLEARMS, P_BASIC },
    { P_SPEAR, P_BASIC },
    { P_CROSSBOW, P_EXPERT },
    { P_DART, P_EXPERT },
    { P_SHURIKEN, P_SKILLED },
    { P_RIDING, P_BASIC },
    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_THIEVERY, P_MASTER },
    { P_SHIELD, P_BASIC },
    { P_PET_HANDLING, P_BASIC },
    { P_NONE, 0 }
};
static const struct def_skill Skill_Ran[] = {
    { P_DAGGER, P_EXPERT },
    { P_AXE, P_SKILLED },
    { P_SHORT_SWORD, P_BASIC },
    { P_BROAD_SWORD, P_SKILLED },
    { P_MACE, P_BASIC },
    { P_FLAIL, P_SKILLED },
    { P_HAMMER, P_BASIC },
    { P_QUARTERSTAFF, P_BASIC },
    { P_POLEARMS, P_BASIC },
    { P_SPEAR, P_EXPERT },
    { P_TRIDENT, P_BASIC },
    { P_BOW, P_EXPERT },
    { P_SLING, P_EXPERT },
    { P_CROSSBOW, P_EXPERT },
    { P_DART, P_EXPERT },
    { P_SHURIKEN, P_SKILLED },
    { P_BOOMERANG, P_EXPERT },
    { P_WHIP, P_BASIC },
    { P_HEALING_SPELL, P_BASIC },
    { P_DIVINATION_SPELL, P_EXPERT },
    { P_ESCAPE_SPELL, P_BASIC },
    { P_EVOCATION_SPELL, P_BASIC },
    { P_RIDING, P_BASIC },
    { P_TWO_WEAPON_COMBAT, P_SKILLED },
    { P_BARE_HANDED_COMBAT, P_BASIC },
    { P_SHIELD, P_SKILLED },
    { P_PET_HANDLING, P_EXPERT },
    { P_NONE, 0 }
};
static const struct def_skill Skill_S[] = {
    { P_DAGGER, P_SKILLED },
    { P_SHORT_SWORD, P_EXPERT },
    { P_BROAD_SWORD, P_SKILLED },
    { P_LONG_SWORD, P_EXPERT },
    { P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SABER, P_BASIC },
    { P_FLAIL, P_SKILLED },
    { P_QUARTERSTAFF, P_BASIC },
    { P_POLEARMS, P_SKILLED },
    { P_SPEAR, P_SKILLED },
    { P_LANCE, P_SKILLED },
    { P_BOW, P_EXPERT },
    { P_SHURIKEN, P_EXPERT },
    { P_DIVINATION_SPELL, P_BASIC }, /* special spell is clairvoyance */
    { P_RIDING, P_SKILLED },
    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_MARTIAL_ARTS, P_MASTER },
    { P_SHIELD, P_SKILLED },
    { P_PET_HANDLING, P_BASIC },
    { P_NONE, 0 }
};
static const struct def_skill Skill_T[] = {
    { P_DAGGER, P_EXPERT },
    { P_AXE, P_BASIC },
    { P_PICK_AXE, P_BASIC },
    { P_SHORT_SWORD, P_EXPERT },
    { P_BROAD_SWORD, P_BASIC },
    { P_LONG_SWORD, P_BASIC },
    { P_TWO_HANDED_SWORD, P_BASIC },
    { P_SABER, P_SKILLED },
    { P_MACE, P_BASIC },
    { P_FLAIL, P_BASIC },
    { P_HAMMER, P_BASIC },
    { P_QUARTERSTAFF, P_BASIC },
    { P_POLEARMS, P_BASIC },
    { P_SPEAR, P_BASIC },
    { P_TRIDENT, P_BASIC },
    { P_LANCE, P_BASIC },
    { P_BOW, P_BASIC },
    { P_SLING, P_BASIC },
    { P_CROSSBOW, P_BASIC },
    { P_DART, P_EXPERT },
    { P_SHURIKEN, P_BASIC },
    { P_BOOMERANG, P_BASIC },
    { P_WHIP, P_BASIC },
    { P_UNICORN_HORN, P_SKILLED },
    { P_DIVINATION_SPELL, P_BASIC },
    { P_ENCHANTMENT_SPELL, P_BASIC },
    { P_ESCAPE_SPELL, P_SKILLED },
    { P_RIDING, P_BASIC },
    { P_TWO_WEAPON_COMBAT, P_SKILLED },
    { P_BARE_HANDED_COMBAT, P_SKILLED },
    { P_SHIELD, P_BASIC },
    { P_PET_HANDLING, P_SKILLED },
    { P_NONE, 0 }
};
static const struct def_skill Skill_V[] = {
    { P_DAGGER, P_EXPERT },
    { P_AXE, P_EXPERT },
    { P_PICK_AXE, P_SKILLED },
    { P_SHORT_SWORD, P_SKILLED },
    { P_BROAD_SWORD, P_SKILLED },
    { P_LONG_SWORD, P_EXPERT },
    { P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SABER, P_BASIC },
    { P_HAMMER, P_EXPERT },
    { P_QUARTERSTAFF, P_BASIC },
    { P_POLEARMS, P_SKILLED },
    { P_SPEAR, P_EXPERT },
    { P_TRIDENT, P_BASIC },
    { P_LANCE, P_SKILLED },
    { P_SLING, P_BASIC },
    { P_MATTER_SPELL, P_BASIC }, /* special spell is repair armor */
    { P_RIDING, P_SKILLED },
    { P_TWO_WEAPON_COMBAT, P_SKILLED },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_SHIELD, P_MASTER },
    { P_PET_HANDLING, P_BASIC },
    { P_NONE, 0 }
};
static const struct def_skill Skill_W[] = {
    { P_DAGGER, P_BASIC },
    { P_QUARTERSTAFF, P_EXPERT },
    { P_SLING, P_BASIC },
    { P_DART, P_BASIC },
    { P_ATTACK_SPELL, P_EXPERT },
    { P_HEALING_SPELL, P_SKILLED },
    { P_DIVINATION_SPELL, P_EXPERT },
    { P_ENCHANTMENT_SPELL, P_EXPERT },
    { P_CLERIC_SPELL, P_SKILLED },
    { P_ESCAPE_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },
    { P_EVOCATION_SPELL, P_SKILLED },
    { P_RIDING, P_BASIC },
    { P_BARE_HANDED_COMBAT, P_BASIC },
    { P_PET_HANDLING, P_SKILLED },
    { P_NONE, 0 }
};

staticfn void
knows_object(int obj, boolean override_pauper)
{
    if (u.uroleplay.pauper && !override_pauper)
        return;
    /* mark as known, but not yet encountered */
    discover_object(obj, TRUE, FALSE, FALSE);
}

/* Know ordinary (non-magical) objects of a certain class,
   like all gems except the loadstone and luckstone. */
staticfn void
knows_class(char sym)
{
    struct obj odummy, *o;
    int ct;

    if (u.uroleplay.pauper)
        return;

    odummy = cg.zeroobj;
    odummy.oclass = sym;
    o = &odummy; /* for use in various obj.h macros */

    /*
     * Note:  the exceptions here can be bypassed if necessary by
     *        calling knows_object() directly.  So an elven ranger,
     *        for example, knows all elven weapons despite the bow,
     *        arrow, and spear limitation below.
     */

    for (ct = svb.bases[(uchar) sym]; ct < svb.bases[(uchar) sym + 1]; ct++) {
        /* not flagged as magic but shouldn't be pre-discovered
           (small shields look the same as two types of magical shield;
           cornuthaum / dunce cap look the same as each other) */
        if (ct == CORNUTHAUM || ct == DUNCE_CAP || ct == SMALL_SHIELD)
            continue;
        if (sym == WEAPON_CLASS) {
            odummy.otyp = ct; /* update 'o' */
            /* arbitrary: only knights and samurai recognize polearms */
            if ((!Role_if(PM_KNIGHT) && !Role_if(PM_SAMURAI)) && is_pole(o))
                continue;
            /* rangers know all launchers (bows, &c), ammo (arrows, &c),
               and spears regardless of race/species, but not other weapons */
            if (Role_if(PM_RANGER)
                && (!is_launcher(o) && !is_ammo(o) && !is_spear(o)))
                continue;
            /* rogues know daggers, regardless of racial variations */
            if (Role_if(PM_ROGUE) && (objects[o->otyp].oc_skill != P_DAGGER))
                continue;
        }

        if (objects[ct].oc_class == sym && !objects[ct].oc_magic)
            knows_object(ct, FALSE);
    }
}

/* role-specific initializations, mostly inventory

   other things may be initialised here, but the function might run more than
   once, so any non-inventory initialisations should be nonrandom and
   idempotent (i.e. doing them twice is OK) */
staticfn void
u_init_role(void)
{
    int i;

    /* the program used to check moves<=1 && invent==NULL do decide whether
       a new game has started, but due to the 'pauper' option/conduct, can't
       rely on invent becoming non-Null anymore; instead, initialize moves
       to 0 instead of 1, then set it to 1 here, where invent init occurs */
    svm.moves = 1L;

    /* various objects are universally known */
    knows_object(SCR_IDENTIFY, FALSE);
    knows_object(POT_WATER, FALSE);
    knows_object(SCR_BLANK_PAPER, FALSE);

    switch (Role_switch) {
    /* rn2(100) > 50 necessary for some choices because some
     * random number generators are bad enough to seriously
     * skew the results if we use rn2(2)...  --KAA
     */
    case PM_ARCHEOLOGIST:
        ini_inv(Archeologist);
        if (!rn2(10))
            ini_inv(Tinopener);
        else if (!rn2(2))
            ini_inv(Lamp);
        knows_object(SACK, FALSE);
        knows_object(TOUCHSTONE, FALSE); /* FALSE: don't override pauper here,
                                          * but TOUCHSTONE will be made known
                                          * in pauper_reinit() */
        break;
    case PM_BARBARIAN:
        if (rn2(100) >= 50) { /* see above comment */
            ini_inv(Barbarian_0);
        } else {
            ini_inv(Barbarian_1);
        }
        if (!rn2(6))
            ini_inv(Lamp);
        knows_class(WEAPON_CLASS); /* excluding polearms */
        knows_class(ARMOR_CLASS);
        break;
    case PM_CAVE_DWELLER:
        ini_inv(Cave_man);
        break;
    case PM_CONVICT:
        ini_inv(Convict);
        knows_object(SKELETON_KEY, FALSE);
        knows_object(GRAPPLING_HOOK, FALSE);
        u.uhunger = 200; /* on the verge of hungry */
        /* override racial alignment */
        u.ualignbase[A_CURRENT] = u.ualignbase[A_ORIGINAL]
            = u.ualign.type = A_CHAOTIC;
        break;
    case PM_DRUID: {
        static const short D_spell[] = {
            SPE_BARKSKIN, SPE_SUMMON_ANIMAL, SPE_HEALING,
            SPE_KNOCK, SPE_CURE_SICKNESS
        };
        const struct trobj D_weapon[] = {
            { (rn2(100) >= 50) ? SCIMITAR : QUARTERSTAFF, 1, WEAPON_CLASS,
              1, 1, UNDEF_BLESS },
            { 0, 0, 0, 0, 0, 0 }
        };
        const struct trobj D_book[] = {
            { D_spell[rn2(100) / 20], UNDEF_SPE, SPBOOK_CLASS, /* [0..4] */
              1, 1, UNDEF_BLESS },
            { 0, 0, 0, 0, 0, 0 }
        };

        ini_inv(D_weapon);
        ini_inv(Druid);
        ini_inv(D_book);
        if (!rn2(25))
            ini_inv(Lamp);
        knows_object(SACK, FALSE); /* FALSE: don't override pauper here,
                                    * but sack will be made known in
                                    * pauper_reinit() */
        break;
    }
    case PM_HEALER:
        u.umoney0 = rn1(1000, 1001);
        ini_inv(Healer);
        if (!rn2(25))
            ini_inv(Lamp);
        knows_object(POT_FULL_HEALING, FALSE);
        break;
    case PM_INFIDEL:
        u.umoney0 = rn1(251, 250);
        if (Race_if(PM_DRAUGR))
            ini_inv(Draugr_Infidel);
        else
            ini_inv(Infidel);
        knows_object(SCR_CHARGING, FALSE);
        break;
    case PM_KNIGHT:
        if (Race_if(PM_ELF))
            ini_inv(Elven_Knight);
        else if (Race_if(PM_DWARF))
            ini_inv(Dwarvish_Knight);
        else if (Race_if(PM_ORC))
            ini_inv(Orcish_Knight);
        else if (Race_if(PM_DRAUGR))
            ini_inv(Draugr_Knight);
        else
            ini_inv(Knight);
        knows_class(WEAPON_CLASS); /* all weapons */
        knows_class(ARMOR_CLASS);
        /* give knights chess-like mobility--idea from wooledge@..cwru.edu */
        HJumping |= FROMOUTSIDE;
        break;
    case PM_MONK: {
        static const struct trobj *M_spell[] = {
            Healing_book, Protection_book, Confuse_monster_book
        };

        if (Race_if(PM_DRAUGR))
            ini_inv(Draugr_Monk);
        else if (Race_if(PM_VAMPIRE))
            ini_inv(Vampire_Monk);
        else
            ini_inv(Monk);
        /* draugr can't cast spells, so no spellbook for them */
        if (!Race_if(PM_DRAUGR))
            ini_inv(M_spell[rn2(90) / 30]); /* [0..2] */
        if (!rn2(4) && !Race_if(PM_DROW))
            ini_inv(Lamp);
        knows_class(ARMOR_CLASS);
        /* sufficiently martial-arts oriented item to ignore language issue */
        knows_object(SHURIKEN, FALSE);
        break;
    }
    case PM_CLERIC: /* priest/priestess */
        ini_inv(Priest);
        if (!rn2(4) && !Race_if(PM_DROW))
            ini_inv(Lamp);
        knows_object(POT_WATER, TRUE); /* override pauper */
        /* KMH, conduct --
         * Some may claim that this isn't agnostic, since they
         * are literally "priests" and they have holy water.
         * But we don't count it as such.  Purists can always
         * avoid playing priests and/or confirm another player's
         * role in their YAAP.
         */
        break;
    case PM_RANGER:
        ini_inv(Ranger);
        knows_class(WEAPON_CLASS); /* bows, arrows, spears only */
        break;
    case PM_ROGUE:
        u.umoney0 = 0;
        if (Race_if(PM_DROW))
            ini_inv(Drow_Rogue);
        else
            ini_inv(Rogue);
        if (!rn2(5))
            ini_inv(Blindfold);
        knows_object(SACK, FALSE); /* FALSE: don't override pauper here,
                                    * but sack will be made known in
                                    * pauper_reinit() */
        knows_class(WEAPON_CLASS); /* daggers only */
        break;
    case PM_SAMURAI:
        ini_inv(Samurai);
        if (!rn2(5))
            ini_inv(Blindfold);
        knows_class(WEAPON_CLASS); /* all weapons */
        knows_class(ARMOR_CLASS);
        /* in order to assist non-Japanese speakers, pre-discover items
           that switch to Japanese names when playing as a Samurai */
        for (i = MAXOCLASSES; i < NUM_OBJECTS; ++i) {
            if (objects[i].oc_magic) /* skip "magic koto" */
                continue;
            if (Japanese_item_name(i, (const char *) 0))
                /* we don't override pauper here because that would give
                   samarai an advantage of knowing several items in advance */
                knows_object(i, FALSE);
        }
        break;
    case PM_TOURIST:
        u.umoney0 = rnd(1000);
        ini_inv(Tourist);
        if (!rn2(25))
            ini_inv(Tinopener);
        else if (!rn2(25))
            ini_inv(Leash);
        else if (!rn2(5))
            ini_inv(Towel);
        break;
    case PM_VALKYRIE:
        ini_inv(Valkyrie);
        if (!rn2(6))
            ini_inv(Lamp);
        knows_class(WEAPON_CLASS); /* excludes polearms */
        knows_class(ARMOR_CLASS);
        break;
    case PM_WIZARD:
        ini_inv(Wizard);
        /* giants and tortles can't wear cloaks; give them another
           source of magic resistance */
        if (Race_if(PM_GIANT) || Race_if(PM_TORTLE))
            ini_inv(AoMR);
        if (!rn2(5) && !Race_if(PM_DROW))
            ini_inv(Lamp);
        if (!rn2(5))
            ini_inv(Blindfold);
        break;

    default: /* impossible */
        break;
    }

    gn.nocreate = STRANGE_OBJECT;
    gn.nocreate2 = STRANGE_OBJECT;
    gn.nocreate3 = STRANGE_OBJECT;
    gn.nocreate4 = STRANGE_OBJECT;
}

/* race-specific initializations, same restrictions as u_init_role */
staticfn void
u_init_race(void)
{
    switch (Race_switch) {
    case PM_HUMAN:
        /* Nothing special */
        break;

    case PM_ELF:
        /*
         * Elves are people of music and song, or they are warriors.
         * Non-warriors get an instrument.  We use a kludge to
         * get only non-magic instruments.
         */
        if (Role_if(PM_CLERIC) || Role_if(PM_WIZARD) || Role_if(PM_DRUID)) {
            static const int trotyp[] =
                { FLUTE, TOOLED_HORN, HARP,
                  BELL,         BUGLE,       LEATHER_DRUM };
            const struct trobj Instrument[] =
                { { ROLL_FROM(trotyp), 0, TOOL_CLASS, 1, 1, 0 },
                  { 0, 0, 0, 0, 0, 0 } };
            ini_inv(Instrument);
        }

        /* Elves can recognize all elvish objects */
        knows_object(ELVEN_SHORT_SWORD, FALSE);
        knows_object(ELVEN_ARROW, FALSE);
        knows_object(ELVEN_BOW, FALSE);
        knows_object(ELVEN_SPEAR, FALSE);
        knows_object(ELVEN_DAGGER, FALSE);
        knows_object(ELVEN_BROADSWORD, FALSE);
        knows_object(ELVEN_LONG_SWORD, FALSE);
        knows_object(ELVEN_CHAIN_MAIL, FALSE);
        knows_object(ELVEN_HELM, FALSE);
        knows_object(ELVEN_SHIELD, FALSE);
        knows_object(ELVEN_BOOTS, FALSE);
        knows_object(ELVEN_CLOAK, FALSE);
        break;

    case PM_DWARF:
        /* Dwarves can recognize all dwarvish objects */
        knows_object(DWARVISH_SPEAR, FALSE);
        knows_object(DWARVISH_SHORT_SWORD, FALSE);
        knows_object(DWARVISH_MATTOCK, FALSE);
        knows_object(DWARVISH_HELM, FALSE);
        knows_object(DWARVISH_CHAIN_MAIL, FALSE);
        knows_object(DWARVISH_CLOAK, FALSE);
        knows_object(DWARVISH_BOOTS, FALSE);
        knows_object(DWARVISH_ROUNDSHIELD, FALSE);

        if (!Role_if(PM_ARCHEOLOGIST) && !Role_if(PM_CONVICT)) {
            /* wise dwarves bring their toy to the dungeons; it gives
               no starting skill (skill_init() skips it, so keep the
               role tests there in step with these) */
            if (!rn2(4))
                ini_inv(Pickaxe);
        }
        break;

    case PM_GNOME:
        break;

    case PM_ILLITHID:
        /* all illithids have psionic 'ability' */
        (void) force_learn_spell(SPE_PSIONIC_WAVE);
        break;

    case PM_TORTLE:
        if (!Role_if(PM_ARCHEOLOGIST) && !Role_if(PM_ROGUE)
            && !Role_if(PM_DRUID)) {
            /* in case they want to go for a swim */
            if (!rn2(4))
                ini_inv(Oilskin);
        }
        break;

    case PM_GIANT: {
        int i;

        /* giants may bring along a few valuable gems */
        while (!rn2(4)) {
            int gem = rnd_class(TOPAZ, JADE);
            const struct trobj Gem[] =
                { { gem, 0, GEM_CLASS, 1, 1, 0 },
                  { 0, 0, 0, 0, 0, 0 } };

            ini_inv(Gem);
            knows_object(gem, FALSE);
        }

        /* Giants know valuable gems from glass, and may recognize
           a few types of valuable gem */
        for (i = DILITHIUM_CRYSTAL; i <= LUCKSTONE; i++) {
            if (objects[i].oc_cost <= 1 || rn2(100) < 5 + ACURR(A_INT))
                knows_object(i, FALSE);
        }
        break;
    }

    case PM_HOBBIT:
        /* Hobbits are always hungry; you'd be hard-pressed to come
           across one that didn't have something to snack on or at least
           a means to make more food */
        if (!Role_if(PM_CONVICT))
            ini_inv(Xtra_food);
        if (!Role_if(PM_ARCHEOLOGIST) && !Role_if(PM_CONVICT))
            ini_inv(Tinningkit);

        /* If it relates to food, Hobbits know everything about it */
        knows_object(TIN, FALSE);
        knows_object(TINNING_KIT, FALSE);
        knows_object(TIN_OPENER, FALSE);

        /* Like Elves, Hobbits can recognize all elvish objects */
        knows_object(ELVEN_SHORT_SWORD, FALSE);
        knows_object(ELVEN_ARROW, FALSE);
        knows_object(ELVEN_BOW, FALSE);
        knows_object(ELVEN_SPEAR, FALSE);
        knows_object(ELVEN_DAGGER, FALSE);
        knows_object(ELVEN_BROADSWORD, FALSE);
        knows_object(ELVEN_LONG_SWORD, FALSE);
        knows_object(ELVEN_CHAIN_MAIL, FALSE);
        knows_object(ELVEN_HELM, FALSE);
        knows_object(ELVEN_SHIELD, FALSE);
        knows_object(ELVEN_BOOTS, FALSE);
        knows_object(ELVEN_CLOAK, FALSE);
        break;

    case PM_CENTAUR:
        /* Centaurs know all bow-based projectile weapons */
        knows_object(ELVEN_ARROW, FALSE);
        knows_object(ELVEN_BOW, FALSE);
        knows_object(ORCISH_ARROW, FALSE);
        knows_object(ORCISH_BOW, FALSE);
        knows_object(DARK_ELVEN_ARROW, FALSE);
        knows_object(DARK_ELVEN_BOW, FALSE);
        knows_object(ARROW, FALSE);
        knows_object(BOW, FALSE);
        knows_object(YA, FALSE);
        knows_object(YUMI, FALSE);
        knows_object(CROSSBOW_BOLT, FALSE);
        knows_object(CROSSBOW, FALSE);
        knows_object(DARK_ELVEN_CROSSBOW_BOLT, FALSE);
        knows_object(DARK_ELVEN_HAND_CROSSBOW, FALSE);
        break;

    case PM_ORC:
        /* compensate for generally inferior equipment */
        if (!Role_if(PM_WIZARD) && !Role_if(PM_CONVICT))
            ini_inv(Xtra_food);
        /* Orcs can recognize all orcish objects */
        knows_object(ORCISH_SHORT_SWORD, FALSE);
        knows_object(ORCISH_ARROW, FALSE);
        knows_object(ORCISH_BOW, FALSE);
        knows_object(ORCISH_SPEAR, FALSE);
        knows_object(ORCISH_DAGGER, FALSE);
        knows_object(ORCISH_CHAIN_MAIL, FALSE);
        knows_object(ORCISH_RING_MAIL, FALSE);
        knows_object(ORCISH_HELM, FALSE);
        knows_object(ORCISH_SHIELD, FALSE);
        knows_object(URUK_HAI_SHIELD, FALSE);
        knows_object(ORCISH_CLOAK, FALSE);
        knows_object(ORCISH_SCIMITAR, FALSE);
        knows_object(ORCISH_BOOTS, FALSE);
        knows_object(ORCISH_MORNING_STAR, FALSE);
        break;

    case PM_DROW:
        /*
         * Like their surface kin, drow are people of music and song,
         * or they are warriors. Non-warriors get an instrument.
         */
        if (Role_if(PM_CLERIC) || Role_if(PM_WIZARD)) {
            static const int trotyp[] =
                { FLUTE, TOOLED_HORN, HARP,
                  BELL,         BUGLE,       LEATHER_DRUM };
            const struct trobj Instrument[] =
                { { ROLL_FROM(trotyp), 0, TOOL_CLASS, 1, 1, 0 },
                  { 0, 0, 0, 0, 0, 0 } };
            ini_inv(Instrument);
        }

        /* Drow can recognize all dark elven objects */
        knows_object(POT_DROW_POISON, FALSE);
        knows_object(DARK_ELVEN_SHORT_SWORD, FALSE);
        knows_object(DARK_ELVEN_ARROW, FALSE);
        knows_object(DARK_ELVEN_BOW, FALSE);
        knows_object(DARK_ELVEN_CROSSBOW_BOLT, FALSE);
        knows_object(DARK_ELVEN_HAND_CROSSBOW, FALSE);
        knows_object(DARK_ELVEN_MACE, FALSE);
        knows_object(DARK_ELVEN_HEAVY_MACE, FALSE);
        knows_object(DARK_ELVEN_SPEAR, FALSE);
        knows_object(DARK_ELVEN_DAGGER, FALSE);
        knows_object(DARK_ELVEN_BROADSWORD, FALSE);
        knows_object(DARK_ELVEN_LONG_SWORD, FALSE);
        knows_object(DARK_ELVEN_HELM, FALSE);
        knows_object(DARK_ELVEN_CHAIN_MAIL, FALSE);
        knows_object(DARK_ELVEN_TUNIC, FALSE);
        knows_object(DARK_ELVEN_CLOAK, FALSE);
        knows_object(DARK_ELVEN_BRACERS, FALSE);
        knows_object(DARK_ELVEN_BOOTS, FALSE);
        knows_object(DARK_ELVEN_GLOVES, FALSE);
        break;

    case PM_DRAUGR:
        /* draugr can only eat meaty corpses and eggs,
           start them out with some provisions */
        if (!Role_if(PM_CONVICT))
            ini_inv(Dra_food);
        if (!Role_if(PM_CONVICT) && !Role_if(PM_INFIDEL)) {
            /* same starting alignment penalty as the typical Convict;
               assigned directly instead of via adjalign() so that it
               is not charged as alignment abuse and stays idempotent
               if inventory is rerolled */
            u.ualign.record = -20;
        }
        break;

    case PM_VAMPIRE:
        knows_object(POT_BLOOD, FALSE);
        knows_object(POT_VAMPIRE_BLOOD, FALSE);
        /* enable random generation of blood potions; mkobj() rolls
           against cached per-class totals, so recompute those */
        objects[POT_BLOOD].oc_prob = 20;
        objects[POT_VAMPIRE_BLOOD].oc_prob = 5;
        init_oclass_probs();
        if (!Role_if(PM_CONVICT) && !Role_if(PM_INFIDEL)) {
            /* same starting alignment penalty as draugr above */
            u.ualign.record = -20;
        }
        break;

    case PM_AASIMAR:
        break;

    default: /* impossible */
        break;
    }
}

/* for 'pauper' aka 'unprepared'; take away any skills (bare-handed combat,
   riding) that are better than unskilled; learn the book (without carrying
   it or knowing its spell yet) for some key spells */
staticfn void
pauper_reinit(void)
{
    int skill, preknown = STRANGE_OBJECT;

    if (!u.uroleplay.pauper)
        return;

    for (skill = 0; skill < P_NUM_SKILLS; skill++)
        if (P_SKILL(skill) > P_UNSKILLED) {
            P_SKILL(skill) = P_UNSKILLED;
            P_ADVANCE(skill) = 0;
        }
    /* pauper has lost out on initial skills, but provide some unspent skill
       credits to make up for that */
    u.weapon_slots = 2;

    /* paupers don't know any spells yet, but several roles will recognize
       the spellbook for a key spell (not necessarily that role's special
       spell); "supply chests" on the first few levels provide a fairly
       high chance to find the book; some other roles know a non-book item */
    switch (Role_switch) {
    case PM_HEALER:
        preknown = SPE_HEALING;
        break;
    case PM_CLERIC:
    case PM_KNIGHT:
    case PM_MONK:
        preknown = SPE_PROTECTION;
        break;
    case PM_WIZARD:
        preknown = SPE_FORCE_BOLT;
        break;
    case PM_ARCHEOLOGIST:
        preknown = TOUCHSTONE;
        break;
    case PM_CAVE_DWELLER:
        preknown = FLINT;
        break;
    case PM_DRUID:
    case PM_ROGUE:
    case PM_TOURIST:
        preknown = SACK;
        break;
    case PM_SAMURAI:
        /* food ration isn't interesting to discover, but put "gunyoki" into
           discoveries list for players who might not recognize what it is */
        preknown = FOOD_RATION;
        break;
    default:
    case PM_BARBARIAN:
    case PM_CONVICT:
    case PM_INFIDEL:
    case PM_RANGER:
    case PM_VALKYRIE:
        break;
    }
    if (preknown != STRANGE_OBJECT)
        knows_object(preknown, TRUE);
}

/* boost STR and CON until hero can carry inventory */
staticfn void
u_init_carry_attr_boost(void)
{
    /* make sure you can carry all you have - especially for Tourists */
    while (inv_weight() > 0) {
        if (adjattrib(A_STR, 1, TRUE))
            continue;
        if (adjattrib(A_CON, 1, TRUE))
            continue;
        /* only get here when didn't boost strength or constitution */
        break;
    }
}

/* initialise u, except inventory, attributes, skills and discoveries */
void
u_init_misc(void)
{
    int i;
    struct u_roleplay tmpuroleplay = u.uroleplay; /* set by rcfile options */

    flags.female = flags.initgend;
    flags.beginner = TRUE;

    /* zero u, including pointer values --
     * necessary when aborting from a failed restore */
    (void) memset((genericptr_t) &u, 0, sizeof(u));
    u.ustuck = (struct monst *) 0;
    (void) memset((genericptr_t) &ubirthday, 0, sizeof(ubirthday));
    (void) memset((genericptr_t) &urealtime, 0, sizeof(urealtime));

    u.uroleplay = tmpuroleplay; /* restore options set via rcfile */

#if 0  /* documentation of more zero values as desirable */
    u.usick_cause[0] = 0;
    u.uluck  = u.moreluck = 0;
    uarmu = 0;
    uarm = uarmc = uarmh = uarms = uarmg = uarmf = 0;
    uwep = uball = uchain = uleft = uright = 0;
    uswapwep = uquiver = 0;
    u.twoweap = FALSE; /* bypass set_twoweap() */
    u.ublessed = 0;                     /* not worthy yet */
    u.ugangr   = 0;                     /* gods not angry */
    u.ugifts   = 0;                     /* no divine gifts bestowed */
    u.uevent.uhand_of_elbereth = 0;
    u.uevent.uheard_tune = 0;
    u.uevent.uopened_dbridge = 0;
    u.uevent.udemigod = 0;              /* not a demi-god yet... */
    u.uevent.amulet_wish = 0;
    u.udg_cnt = 0;
    u.mh = u.mhmax = u.mtimedone = 0;
    u.uz.dnum = u.uz0.dnum = 0;
    u.utotype = UTOTYPE_NONE;
#endif /* 0 */

    u.uz.dlevel = 1;
    u.uz0.dlevel = 0;
    u.utolev = u.uz;

    u.umoved = FALSE;
    u.umortality = 0;
    u.ugrave_arise = NON_PM;

    u.umonnum = u.umonster = gu.urole.mnum;
    u.ulycn = NON_PM;
    set_uasmon();

    u.ulevel = 0; /* set up some of the initial attributes */
    u.uhp = u.uhpmax = u.uhppeak = newhp();
    u.uen = u.uenmax = u.uenpeak = newpw();
    u.uspellprot = 0;
    adjabil(0, 1);
    u.ulevel = u.ulevelmax = 1;

    init_uhunger();
    for (i = 0; i <= MAXSPELL; i++)
        svs.spl_book[i].sp_id = NO_SPELL;
    u.ublesscnt = 300; /* no prayers just yet */
    u.ulastprayed = 0; /* hasn't prayed yet */
    u.ualignbase[A_CURRENT] = u.ualignbase[A_ORIGINAL] = u.ualign.type =
        aligns[flags.initalign].value;

#if defined(BSD) && !defined(POSIX_TYPES)
    (void) time((long *) &ubirthday);
#else
    (void) time(&ubirthday);
#endif

    /*
     *  For now, everyone starts out with a night vision range of 1 and
     *  their xray_range disabled.
     */
    u.nv_range = 1;
    u.xray_range = -1;
    u.unblind_telepat_range = -1;

    /* OPTIONS:blind results in permanent blindness (unless overridden
       by the Eyes of the Overworld, which will clear 'u.uroleplay.blind'
       to void the conduct, but will leave the PermaBlind bit set so that
       blindness resumes when the Eyes are removed). */
    if (u.uroleplay.blind)
        HBlinded |= FROMOUTSIDE; /* set PermaBlind */

    /* roughly based on distribution in human population */
    u.uhandedness = rn2(10) ? RIGHT_HANDED : LEFT_HANDED;

    max_rank_sz(); /* set max str size for class ranks */

    return;
}

/* attack/damage structs for shambler_init() */
static const int attk_melee_types [] =
    { AT_CLAW, AT_BITE, AT_TUCH, AT_STNG, AT_WEAP };

static const int attk_spec_types [] =
    { AT_HUGS, AT_SPIT, AT_ENGL, AT_BREA, AT_GAZE,
      AT_MAGC, AT_KICK, AT_BUTT, AT_TENT
    };

static const int damg_melee_types [] =
    { AD_PHYS, AD_FIRE, AD_COLD, AD_SLEE, AD_ELEC,
      AD_DRST, AD_ACID, AD_STUN, AD_SLOW,
      AD_PLYS, AD_DRLI, AD_DREN, AD_LEGS, AD_STCK,
      AD_SGLD, AD_SITM, AD_SEDU, AD_TLPT, AD_RUST,
      AD_CONF, AD_DRDX, AD_DRCO, AD_DRIN, AD_DISE,
      AD_DCAY, AD_HALU, AD_ENCH, AD_CORR, AD_BHED,
      AD_POLY, AD_WTHR, AD_PITS, AD_WEBS
    };

static const int damg_breath_types [] =
    { AD_MAGM, AD_FIRE, AD_COLD, AD_SLEE, AD_ELEC,
      AD_DRST, AD_WATR, AD_ACID, AD_DRLI, AD_STUN
    };

static const int damg_spit_types [] =
    { AD_BLND, AD_ACID, AD_DRST };

static const int damg_gaze_types [] =
    { AD_FIRE, AD_COLD, AD_SLEE, AD_STUN, AD_SLOW,
      AD_CNCL
    };

static const int damg_engulf_types [] =
    { AD_PLYS, AD_DGST, AD_WRAP };

static const int damg_magic_types [] =
    { AD_SPEL, AD_CLRC, AD_MAGM, AD_FIRE, AD_COLD,
      AD_ACID
    };

static const int damg_kick_types [] =
    { AD_PHYS, AD_STUN, AD_LEGS, AD_ENCH, AD_CLOB };

static const int damg_butt_types [] =
    { AD_PHYS, AD_STUN, AD_CONF, AD_CLOB };

static const int damg_tent_types [] =
    { AD_PHYS, AD_DRST, AD_ACID, AD_STUN, AD_PLYS,
      AD_DRLI, AD_DREN, AD_CONF, AD_DRIN, AD_DISE,
      AD_HALU
    };

void
shambler_init(void)
{
    int i;
    struct permonst *shambler = &mons[PM_SHAMBLING_HORROR];
    struct attack *attkptr;
    int shambler_attacks;

    /* what a horrible night to have a curse...
       Assumes shambler_init() is called exactly once per process
       (allmain.c newgame()); no other writer touches the shambling
       horror's slot - monst_globals_init() copies the defaults in
       early_init() before any game starts, and restmontemplate()
       never runs in a process that then starts a new game. The `+=`
       on mlevel is only correct under that assumption; if in-process
       restart is ever added, cache base values before mutating */
    shambler->mlevel += rnd(15) - 3;    /* shuffle level */
    shambler->mmove = rn2(10) + 9;      /* slow to very fast */
    shambler->ac = rn2(31) - 20;        /* any AC */
    shambler->mr = rn2(5) * 25;         /* varying amounts of MR */
    shambler->maligntyp = rn2(21) - 10; /* varying alignment */

    shambler_attacks = rnd(4);
    for (i = 0; i < shambler_attacks; i++) {
        attkptr = &shambler->mattk[i];
        attkptr->aatyp = attk_melee_types[rn2(SIZE(attk_melee_types))];
        attkptr->adtyp = damg_melee_types[rn2(SIZE(damg_melee_types))];
        attkptr->damn = 2 + rn2(5);
        attkptr->damd = 3 + rn2(6);
    }

    shambler_attacks = shambler_attacks + (rnd(9) / 3) - 1;
    /* defensive clamp; current max is exactly NATTK, no behavior
       change but guards against future widening of either seed
       range walking us off mattk[] */
    if (shambler_attacks > NATTK)
        shambler_attacks = NATTK;
    for (; i < shambler_attacks; i++) {
        attkptr = &shambler->mattk[i];
        attkptr->aatyp = attk_spec_types[rn2(SIZE(attk_spec_types))];
        attkptr->damn = 2 + rn2(4);
        attkptr->damd = 6 + rn2(3);
        switch (attkptr->aatyp) {
        case AT_BREA:
            attkptr->adtyp = damg_breath_types[rn2(SIZE(damg_breath_types))];
            break;
        case AT_SPIT:
            attkptr->adtyp = damg_spit_types[rn2(SIZE(damg_spit_types))];
            break;
        case AT_GAZE:
            attkptr->adtyp = damg_gaze_types[rn2(SIZE(damg_gaze_types))];
            break;
        case AT_ENGL:
            attkptr->adtyp = damg_engulf_types[rn2(SIZE(damg_engulf_types))];
            break;
        case AT_MAGC:
            attkptr->adtyp = damg_magic_types[rn2(SIZE(damg_magic_types))];
            break;
        case AT_KICK:
            attkptr->adtyp = damg_kick_types[rn2(SIZE(damg_kick_types))];
            break;
        case AT_BUTT:
            attkptr->adtyp = damg_butt_types[rn2(SIZE(damg_butt_types))];
            break;
        case AT_TENT:
            attkptr->adtyp = damg_tent_types[rn2(SIZE(damg_tent_types))];
            break;
        case AT_HUGS:
            attkptr->adtyp = AD_PHYS;
            break;
        default:
            attkptr->adtyp = AD_PHYS;
            break;
        }
    }

    shambler->msize = !rn2(6) ? MZ_GIGANTIC      /* gigantic, or one of */
                              : rn2(5);          /* tiny/small/medium/large/huge */
    shambler->cwt = rnd(1000) * 4;               /* random weight (4 - 4000) */
    shambler->cnutrit = 20;                      /* moot, does not leave a corpse */
    shambler->msound = rn2(MS_HUMANOID);         /* any but the specials */
    shambler->mresists = 0;

    for (i = 0; i < rnd(6); i++)
        shambler->mresists |= (1UL << rn2(9));   /* physical resistances */
    for (i = 0; i < rnd(5); i++)
        shambler->mresists |= (0x200 << rn2(9)); /* 'different' resistances, even clumsy */
    shambler->mconveys = 0;                      /* flagged NOCORPSE */

    /*
     * now time for the random flags. this will likely produce
     * a number of complete trainwreck monsters at first, but
     * every so often something will dial up nasty stuff
     */
    shambler->mflags1 = 0;
    for (i = 0; i < rnd(17); i++)
        shambler->mflags1 |= (1UL << rn2(32));  /* rolls a random bit position 0-31; M1_ uses
                                                 * all 32 bits (bit 31 = M1_METALLIVORE). This
                                                 * is a bit-count, not the count of named M1_
                                                 * defines in include/monflag.h */
    shambler->mflags1 &= ~M1_UNSOLID;           /* no ghosts */
    shambler->mflags1 &= ~M1_WALLWALK;          /* no wall-walkers */
    shambler->mflags1 &= ~M1_ACID;              /* will never leave a corpse */
    shambler->mflags1 &= ~M1_POIS;              /* same as above */
    shambler->mflags1 &= ~M1_HIDE;              /* no hiding/mimicking */
    shambler->mflags1 &= ~M1_CONCEAL;           /* same as above */

    shambler->mflags2 = 0;
    for (i = 0; i < rnd(17); i++)
        shambler->mflags2 |= (1UL << rn2(32));  /* rolls a random bit position 0-31; M2_ has
                                                 * gaps at bits 6-9 and 15 (no-op when rolled)
                                                 * but bit 31 is M2_MAGIC so all 32 positions
                                                 * are needed. Bit-count, not named-define
                                                 * count */
    shambler->mflags2 &= ~M2_MERC;              /* no guards */
    shambler->mflags2 &= ~M2_PEACEFUL;          /* no peacefuls */
    shambler->mflags2 &= ~M2_PNAME;             /* not a proper name */
    shambler->mflags2 &= ~M2_MALE;              /* always genderless */
    shambler->mflags2 &= ~M2_FEMALE;            /* same as above */
    shambler->mflags2 &= ~M2_SHAPESHIFTER;      /* no chameleon types */
    shambler->mflags2 &= ~M2_LORD;              /* isn't royalty */
    shambler->mflags2 &= ~M2_PRINCE;            /* still isn't royalty */
    shambler->mflags2 &= ~M2_DOMESTIC;          /* no taming */
    shambler->mflags2 &= ~M2_DRUID_FORM_A;      /* prevent druids from shapechanging into a shambler */
    shambler->mflags2 &= ~M2_DRUID_FORM_B;      /* prevent druids from shapechanging into a shambler */
    shambler->mflags2 &= ~M2_DRUID_FORM_C;      /* prevent druids from shapechanging into a shambler */
    shambler->mflags2 &= ~M2_DRUID_FORM_D;      /* prevent druids from shapechanging into a shambler */
    shambler->mflags2 &= ~M2_VAMPIRE_FORM;      /* prevent vampires from shapechanging into a shambler */

    shambler->mflags2 |= M2_NOPOLY;             /* Don't let the player be one of these */
    shambler->mflags2 |= M2_HOSTILE;            /* always hostile */
    shambler->mflags2 |= M2_NEUTER;             /* always genderless */

    shambler->mflags3 = 0;
    for (i = 0; i < rnd(5); i++)
        shambler->mflags3 |= (1 << rn2(16));    /* rolls a random bit position 0-15; mflags3 is
                                                 * unsigned short so bits 16+ truncate away.
                                                 * M3_ has a gap at bit 5 (no-op when rolled).
                                                 * Bit-count, not named-define count */
    shambler->mflags3 &= ~M3_WANTSALL;
    shambler->mflags3 &= ~M3_COVETOUS;          /* no covetous behavior */
    shambler->mflags3 &= ~M3_WAITMASK;          /* no waiting either */
    shambler->mflags3 &= ~M3_STATIONARY;        /* no stuck in one spot */

    return;
}

/* the appropriate set of skills for the role */
staticfn const struct def_skill *
skills_for_role(void)
{
    const struct def_skill *skills;

    switch (Role_switch) {
    case PM_ARCHEOLOGIST:
        skills = Skill_A;
        break;
    case PM_BARBARIAN:
        skills = Race_if(PM_DRAUGR) ? Skill_Dra_B : Skill_B;
        break;
    case PM_CAVE_DWELLER:
        skills = Skill_C;
        break;
    case PM_CONVICT:
        skills = Race_if(PM_DRAUGR) ? Skill_Dra_Con : Skill_Con;
        break;
    case PM_DRUID:
        skills = Skill_D;
        break;
    case PM_HEALER:
        skills = Skill_H;
        break;
    case PM_INFIDEL:
        skills = Race_if(PM_DRAUGR) ? Skill_Dra_Inf : Skill_Inf;
        break;
    case PM_KNIGHT:
        skills = Race_if(PM_DRAUGR) ? Skill_Dra_K : Skill_K;
        break;
    case PM_MONK:
        skills = Race_if(PM_DRAUGR) ? Skill_Dra_Mon : Skill_Mon;
        break;
    case PM_CLERIC:
        skills = Skill_P;
        break;
    case PM_RANGER:
        skills = Skill_Ran;
        break;
    case PM_ROGUE:
        skills = Race_if(PM_DRAUGR) ? Skill_Dra_R : Skill_R;
        break;
    case PM_SAMURAI:
        skills = Skill_S;
        break;
    case PM_TOURIST:
        skills = Skill_T;
        break;
    case PM_VALKYRIE:
        skills = Skill_V;
        break;
    case PM_WIZARD:
        skills = Skill_W;
        break;
    default:
        panic("No skills found for role");
        break;
    }

    return skills;
}

/* skills aren't initialized, so we use the role-specific skill lists */
staticfn boolean
restricted_spell_discipline(int otyp)
{
    const struct def_skill *skills = skills_for_role();
    int this_skill = spell_skilltype(otyp);

    while (skills && skills->skill != P_NONE) {
        if (skills->skill == this_skill)
            return FALSE;
        ++skills;
    }
    return TRUE;
}

/* randomizes the quantity given a trobj description */
staticfn long
trquan(const struct trobj *trop)
{
    if (!trop->trquan_min)
        return 1;
    return trop->trquan_min + rn2(trop->trquan_max - trop->trquan_min + 1);
}

/* create random object of certain class, filtering out too powerful items */
staticfn struct obj *
ini_inv_mkobj_filter(int oclass, boolean got_level1_spellbook)
{
    struct obj *obj;
    int otyp, trycnt = 0;

    /*
     * For random objects, do not create certain overly powerful
     * items: wand of wishing, ring of levitation, or the
     * polymorph/polymorph control combination.  Specific objects,
     * i.e. the discovery wishing, are still OK.
     * Also, don't get a couple of really useless items.  (Note:
     * punishment isn't "useless".  Some players who start out with
     * one will immediately read it and use the iron ball as a
     * weapon.)
     */
    obj = mkobj(oclass, FALSE);
    otyp = obj->otyp;

    while (otyp == WAN_WISHING || otyp == gn.nocreate
           || otyp == gn.nocreate2 || otyp == gn.nocreate3
           || otyp == gn.nocreate4 || otyp == RIN_LEVITATION
           /* 'useless' items */
           || otyp == POT_HALLUCINATION
           || otyp == POT_ACID
           || otyp == SCR_AMNESIA
           || otyp == SCR_FIRE
           || otyp == SCR_BLANK_PAPER
           || otyp == SPE_BLANK_PAPER
           || otyp == RIN_AGGRAVATE_MONSTER
           || otyp == RIN_HUNGER
           || otyp == WAN_NOTHING
           /* gnomes hate eggs */
           || (otyp == EGG && Race_if(PM_GNOME))
           /* some races inherently have poison resistance */
           || (otyp == RIN_POISON_RESISTANCE
               && (Race_if(PM_ORC) || Race_if(PM_DRAUGR)
                   || Race_if(PM_VAMPIRE)))
           /* Monks don't use weapons */
           || (otyp == SCR_ENCHANT_WEAPON && Role_if(PM_MONK))
           /* wizard patch -- they already have one */
           || (otyp == SPE_FORCE_BOLT && Role_if(PM_WIZARD))
           || (otyp == WAN_MAGIC_MISSILE && Role_if(PM_WIZARD))
           /* same for druids */
           || (otyp == SPE_ENTANGLE && Role_if(PM_DRUID))
           /* and same for infidels */
           || (otyp == SPE_DRAIN_LIFE && Role_if(PM_INFIDEL))
           /* infidels already have auto-clairvoyance by having the
              Amulet of Yendor in starting inventory */
           || (otyp == SPE_CLAIRVOYANCE && Role_if(PM_INFIDEL))
           /* drow hate the light, no self-respecting dark elf
              would ever want to cast it */
           || (otyp == SPE_LIGHT && Race_if(PM_DROW))
           /* items that will be silver for vampires
              (rings/wands perhaps) that can't become iron */
           || (Race_if(PM_VAMPIRE)
               && objects[otyp].oc_material == SILVER
               && !valid_obj_material(obj, IRON))
           /* items that will be iron for elves
              (rings/wands perhaps) that can't become copper */
           || (Race_if(PM_ELF)
               && objects[otyp].oc_material == IRON
               && !valid_obj_material(obj, COPPER))
           /* items that will be iron for drow
              (rings/wands perhaps) that can't become adamantine */
           || (Race_if(PM_DROW)
               && objects[otyp].oc_material == IRON
               && !valid_obj_material(obj, ADAMANTINE))
           /* items that will be mithril for orcs
              (rings/wands perhaps) that can't become iron */
           || (Race_if(PM_ORC)
               && objects[otyp].oc_material == MITHRIL
               && !valid_obj_material(obj, IRON))
           /* powerful spells are either useless to
              low level players or unbalancing; also
              spells in restricted skill categories */
           || (obj->oclass == SPBOOK_CLASS
               && (objects[otyp].oc_level > (got_level1_spellbook ? 3 : 1)
                   || restricted_spell_discipline(otyp)))
           || otyp == SPE_NOVEL) {
        dealloc_obj(obj);
        if (++trycnt > 1000) {
            /* This lonely pancake's potential will never be realized.
             * It will exist only as a thought, of something that could have
             * been, but never will be. It will never experience maple syrup
             * oozing into its nooks, or see the delightful expression on
             * someone's face as they are about to let it dance across their
             * taste buds. */
            obj = mksobj(PANCAKE, TRUE, FALSE);
            break;
        }
        obj = mkobj(oclass, FALSE);
        otyp = obj->otyp;
    }
    return obj;
}

/* substitute object with something else based on race.
   only changes otyp, and returns it. */
staticfn short
ini_inv_obj_substitution(const struct trobj *trop, struct obj *obj)
{
    if (gu.urace.mnum != PM_HUMAN) {
        int i;

        /* substitute race-specific items; this used to be in
           the 'if (otyp != UNDEF_TYP) { }' block above, but then
           substitutions didn't occur for randomly generated items
           (particularly food) which have racial substitutes */
        for (i = 0; inv_subs[i].race_pm != NON_PM; ++i)
            if (inv_subs[i].race_pm == gu.urace.mnum
                && obj->otyp == inv_subs[i].item_otyp) {
                debugpline3("ini_inv: substituting %s for %s%s",
                            OBJ_NAME(objects[inv_subs[i].subs_otyp]),
                            (trop->trotyp == UNDEF_TYP) ? "random " : "",
                            OBJ_NAME(objects[obj->otyp]));
                obj->otyp = inv_subs[i].subs_otyp;
                /* the substitute might be in a different class
                   (vampires' food rations become potions of blood) */
                obj->oclass = objects[obj->otyp].oc_class;
                /* This might have created a bad material combination,
                   such as a dagger (which was forced to be iron
                   earlier) turning into an elven dagger, but now
                   remaining iron. Fix this up via set_material() so
                   stale oeroded/oeroded2 bits from the pre-sub
                   material are cleared */
                set_material(obj, objects[obj->otyp].oc_material);
                break;
            }
    }
    return obj->otyp;
}

/* look up the racial starting-inventory substitute for an object type
   (e.g. SPEAR -> DWARVISH_SPEAR for dwarves); returns the substituted
   otyp, or the original if no substitution applies. Exported for the
   divine gift machinery, which hands out racial gear where possible */
short
racial_inv_substitution(short otyp)
{
    int i;

    if (gu.urace.mnum == PM_HUMAN)
        return otyp;

    for (i = 0; inv_subs[i].race_pm != NON_PM; ++i) {
        if (inv_subs[i].race_pm == gu.urace.mnum
            && otyp == inv_subs[i].item_otyp)
            return inv_subs[i].subs_otyp;
    }
    return otyp;
}

/* returns: TRUE to stop generating items from this trobj,
   FALSE for normal behaviour */
staticfn boolean
ini_inv_adjust_obj(const struct trobj *trop, struct obj *obj)
{
    boolean stop = FALSE;
    if (trop->trclass == COIN_CLASS) {
        /* no "blessed" or "identified" money */
        obj->quan = u.umoney0;
    } else {
        if (objects[obj->otyp].oc_uses_known)
            obj->known = 1;
        /* not observe_object during startup, that's handled later */
        obj->dknown = obj->bknown = obj->rknown = 1;
        if (Is_container(obj) || obj->otyp == STATUE) {
            obj->cknown = obj->lknown = 1;
            obj->otrapped = 0;
        }
        obj->cursed = (trop->trbless == CURSED);
        if (obj->opoisoned && u.ualign.type > A_CHAOTIC)
            obj->opoisoned = 0;
        if (obj->otainted)
            obj->otainted = 0;
        if (obj->oclass == WEAPON_CLASS || obj->oclass == TOOL_CLASS) {
            obj->quan = trquan(trop);
            stop = TRUE;
        } else if (obj->oclass == GEM_CLASS && is_graystone(obj)
                   && obj->otyp != FLINT) {
            obj->quan = 1L;
        }
        if (Role_if(PM_INFIDEL) && obj->oclass == ARMOR_CLASS) {
            /* Infidels are used to playing with fire */
            maybe_erodeproof(obj, 1);
        }
        if (obj->otyp == STRIPED_SHIRT)
            obj->cursed = TRUE;
        if (trop->trspe != UNDEF_SPE) {
            obj->spe = trop->trspe;
        } else {
            /* Don't start with +0 or negative rings */
            if (objects[obj->otyp].oc_class == RING_CLASS
                && objects[obj->otyp].oc_charged && obj->spe <= 0)
                obj->spe = rne(3);
        }
        if (trop->trbless != UNDEF_BLESS)
            obj->blessed = (trop->trbless == 1);

    }
    /* don't allow gear with object properties to be start scummed
       for; must clear before setworn/setuwep, since those push
       oprops extrinsics into u.uprops[*].extrinsic and spfx effects
       that a later clear would not undo */
    obj->oprops = obj->oprops_known = 0L;
    /* defined after setting otyp+quan + blessedness */
    obj->owt = weight(obj);
    return stop;
}

/* initial inventory: wear, wield, learn the spell/obj */
staticfn void
ini_inv_use_obj(struct obj *obj)
{
    /* Make the type known if necessary */
    if (OBJ_DESCR(objects[obj->otyp]) && obj->known)
        discover_object(obj->otyp, TRUE, TRUE, FALSE);
    if (obj->otyp == OIL_LAMP)
        discover_object(POT_OIL, TRUE, TRUE, FALSE);

    if (obj->oclass == ARMOR_CLASS) {
        if (is_shield(obj) && !uarms && !(uwep && bimanual(uwep))
            && obj->otyp != BRACERS && obj->otyp != DARK_ELVEN_BRACERS) {
            /* Prior to 3.6.2 this used to unset uswapwep if it was set,
               but wearing a shield doesn't prevent having an alternate
               weapon ready to swap with the primary; just make sure we
               aren't two-weaponing (academic; no one starts that way) */
            set_twoweap(FALSE); /* u.twoweap = FALSE */
            setworn(obj, W_ARMS);
        } else if ((obj->otyp == BRACERS || obj->otyp == DARK_ELVEN_BRACERS)
                   && !uarms) {
            /* bracers occupy the shield slot but, unlike regular
               shields, can be worn while wielding a two-handed weapon
               (a Druid's quarterstaff, for instance) */
            setworn(obj, W_ARMS);
        } else if (is_helmet(obj) && !uarmh)
            setworn(obj, W_ARMH);
        else if (is_gloves(obj) && !uarmg)
            setworn(obj, W_ARMG);
        else if (is_shirt(obj) && !uarmu)
            setworn(obj, W_ARMU);
        else if (is_cloak(obj) && !uarmc)
            setworn(obj, W_ARMC);
        else if (is_boots(obj) && !uarmf)
            setworn(obj, W_ARMF);
        else if (is_suit(obj) && !uarm)
            setworn(obj, W_ARM);
    }

    if (obj->oclass == WEAPON_CLASS || is_weptool(obj)
        || obj->otyp == TIN_OPENER
        || obj->otyp == FLINT || obj->otyp == ROCK
        || obj->otyp == SLING_BULLET) {
        if (is_ammo(obj) || is_missile(obj)) {
            if (!uquiver)
                setuqwep(obj);
        } else if (!uwep && (!uarms || !bimanual(obj))) {
            setuwep(obj);
        } else if (!uswapwep) {
            setuswapwep(obj);
        }
    }
    if (obj->oclass == SPBOOK_CLASS && obj->otyp != SPE_BLANK_PAPER)
        initialspell(obj);
    if (obj->oclass == AMULET_CLASS)
        setworn(obj, W_AMUL);
}

staticfn void
ini_inv(const struct trobj *trop)
{
    struct obj *obj;
    int otyp;
    boolean got_sp1 = FALSE; /* got a level 1 spellbook? */
    long quan;

    if (u.uroleplay.pauper) /* pauper gets no items */
        return;

    quan = trquan(trop);
    while (trop->trclass) {
        otyp = (int) trop->trotyp;
        if (otyp != UNDEF_TYP) {
            obj = mksobj(otyp, TRUE, FALSE);
        } else { /* UNDEF_TYP */
            obj = ini_inv_mkobj_filter(trop->trclass, got_sp1);
            otyp = obj->otyp;
            /* Heavily relies on the facts that 1) we create wands
             * before rings, that 2) we create rings before
             * spellbooks, and that 3) not more than 1 object of a
             * particular symbol is to be prohibited.  (For more
             * objects, we need more nocreate variables...)
             */
            switch (otyp) {
            case WAN_POLYMORPH:
            case RIN_POLYMORPH:
            case POT_POLYMORPH:
                gn.nocreate = RIN_POLYMORPH_CONTROL;
                break;
            case RIN_POLYMORPH_CONTROL:
                gn.nocreate = RIN_POLYMORPH;
                gn.nocreate2 = SPE_POLYMORPH;
                gn.nocreate3 = POT_POLYMORPH;
            }
            /* Don't have 2 of the same ring or spellbook */
            if (obj->oclass == RING_CLASS || obj->oclass == SPBOOK_CLASS)
                gn.nocreate4 = otyp;
        }
        /* Put post-creation object adjustments that don't depend on whether
         * it was UNDEF_TYP or not after this. */

        /* Don't allow materials to be start scummed for */
        set_material(obj, objects[otyp].oc_material);

        /* Replace iron objects (e.g. Priest's mace) with copper
           for elves */
        if (Race_if(PM_ELF) && obj->material == IRON
            && valid_obj_material(obj, COPPER))
            set_material(obj, COPPER);

        /* Replace iron objects (e.g. Priest's mace) with adamantine
           for drow */
        if (Race_if(PM_DROW) && obj->material == IRON
            && valid_obj_material(obj, ADAMANTINE))
            set_material(obj, ADAMANTINE);

        /* Do the same for orcs and mithril objects.
           Currently not a concern, but may be in the future */
        if (Race_if(PM_ORC) && obj->material == MITHRIL
            && valid_obj_material(obj, IRON))
            set_material(obj, IRON);

        /* Do the same for vampires and silver objects.
           Currently not a concern, but may be in the future */
        if (Race_if(PM_VAMPIRE) && obj->material == SILVER
            && valid_obj_material(obj, IRON))
            set_material(obj, IRON);

        otyp = ini_inv_obj_substitution(trop, obj);
        nhUse(otyp);

        /* nudist gets no armor; also remove the amulet for wizards
           (giant and tortle races) in case they want to try for the
           no magic resistance conduct */
        if (u.uroleplay.nudist
            && (obj->oclass == ARMOR_CLASS
                || (Role_if(PM_WIZARD) && obj->oclass == AMULET_CLASS))) {
            dealloc_obj(obj);
            trop++;
            continue;
        }

        if (ini_inv_adjust_obj(trop, obj))
            quan = 1;
        obj = addinv(obj);

        /* First spellbook should be level 1 - did we get it? */
        if (obj->oclass == SPBOOK_CLASS && objects[obj->otyp].oc_level == 1)
            got_sp1 = TRUE;

        if (--quan)
            continue; /* make a similar object */
        trop++;
        quan = trquan(trop);
    }
}

/* initialise starting inventory and attributes

   this function can be run multiple times and will overwrite the effects of
   previous runs */
void
u_init_inventory_attrs(void)
{
    gl.lastinvnr = 51;
    while (gi.invent)
        useupall(gi.invent);

    u.umoney0 = 0;
    u_init_role();
    u_init_race();

    if (discover)
        ini_inv(Wishing);

    if (u.umoney0)
        ini_inv(Money);
    u.umoney0 += hidden_gold(TRUE); /* in case sack has gold in it */

    /* Ensure that Monks don't start with meat. (Tripe is OK, as it's
       meant as pet food.) Mainly for the races that can play the Monk
       role and start with extra food much like an Orc */
    if (Role_if(PM_MONK)) {
        struct obj *otmp;

        for (otmp = gi.invent; otmp; otmp = otmp->nobj) {
            /* spinach tins have corpsenm == NON_PM; avoid mons[-1] */
            if (otmp->otyp == TIN && otmp->corpsenm != NON_PM
                && !vegetarian(&mons[otmp->corpsenm])) {
                if (rn2(2)) {
                    otmp->spe = 1; /* replace the meat with spinach */
                    otmp->corpsenm = NON_PM;
                } else {
                    otmp->corpsenm = PM_LICHEN;
                }
            }
        }
    }

    /* Tortles that start with a trident get one that
       is rustproof (currently only barbarian role) */
    if (Role_if(PM_BARBARIAN) && Race_if(PM_TORTLE)) {
        struct obj *otmp;

        for (otmp = gi.invent; otmp; otmp = otmp->nobj) {
            if (otmp->otyp == TRIDENT) {
                maybe_erodeproof(otmp, 1);
                otmp->rknown = 1;
            }
        }
    }

    init_attr(75);    /* init attribute values */
    vary_init_attr(); /* minor variation to attrs */
    u_init_carry_attr_boost();
}

/* side effects of starting inventory (e.g. discovering it) and skills (both
   those based on role and those based on starting inventory) */
void
u_init_skills_discoveries(void)
{
    struct obj *otmp;
    for (otmp = gi.invent; otmp; otmp = otmp->nobj)
        ini_inv_use_obj(otmp);

    /* if a dwarvish monk starts with a pick-axe,
       don't start with it wielded */
    if (Role_if(PM_MONK) && uwep && uwep->otyp == PICK_AXE)
        setuwep((struct obj *) 0);

    skill_init(skills_for_role());
    if (u.uroleplay.pauper)
        pauper_reinit();

    /* If we have at least one spell, force starting Pw to be enough,
       so hero can cast the level 1 spell they should have */
    if (num_spells() && (u.uenmax < SPELL_LEV_PW(1)))
        u.uen = u.uenmax = u.uenpeak = u.ueninc[u.ulevel] = SPELL_LEV_PW(1);

    find_ac();     /* get initial ac value */
}

#undef UNDEF_TYP
#undef UNDEF_SPE
#undef UNDEF_BLESS
#undef CURSED

/*u_init.c*/
