/* NetHack 3.6	youprop.h	$NHDT-Date: 1568831820 2019/09/18 18:37:00 $  $NHDT-Branch: NetHack-3.6 $:$NHDT-Revision: 1.27 $ */
/* Copyright (c) 1989 Mike Threepoint				  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef YOUPROP_H
#define YOUPROP_H

#include "prop.h"
#include "permonst.h"
#include "mondata.h"
#include "pm.h"

/* KMH, intrinsics patch.
 * Reorganized and rewritten for >32-bit properties.
 * HXxx refers to intrinsic bitfields while in human form.
 * EXxx refers to extrinsic bitfields from worn objects.
 * BXxx refers to the cause of the property being blocked.
 * Xxx refers to any source, including polymorph forms.
 * [Post-3.4.3: HXxx now includes a FROMFORM bit to handle
 * intrinsic conferred by being polymorphed.]
 */

#define maybe_polyd(if_so, if_not) (Upolyd ? (if_so) : (if_not))

/*** Resistances to troubles ***/
/* With intrinsics and extrinsics */
#define HFire_resistance u.uprops[FIRE_RES].intrinsic
#define EFire_resistance u.uprops[FIRE_RES].extrinsic
#define Fire_resistance (HFire_resistance || EFire_resistance)

#define HCold_resistance u.uprops[COLD_RES].intrinsic
#define ECold_resistance u.uprops[COLD_RES].extrinsic
#define Cold_resistance (HCold_resistance || ECold_resistance)

#define HSleep_resistance u.uprops[SLEEP_RES].intrinsic
#define ESleep_resistance u.uprops[SLEEP_RES].extrinsic
#define Sleep_resistance (HSleep_resistance || ESleep_resistance)

#define HDisint_resistance u.uprops[DISINT_RES].intrinsic
#define EDisint_resistance u.uprops[DISINT_RES].extrinsic
#define Disint_resistance (HDisint_resistance || EDisint_resistance)

#define HShock_resistance u.uprops[SHOCK_RES].intrinsic
#define EShock_resistance u.uprops[SHOCK_RES].extrinsic
#define Shock_resistance (HShock_resistance || EShock_resistance)

#define HPoison_resistance u.uprops[POISON_RES].intrinsic
#define EPoison_resistance u.uprops[POISON_RES].extrinsic
#define Poison_resistance (HPoison_resistance || EPoison_resistance)

#define HDrain_resistance u.uprops[DRAIN_RES].intrinsic
#define EDrain_resistance u.uprops[DRAIN_RES].extrinsic
#define Drain_resistance (HDrain_resistance || EDrain_resistance)

#define HPsychic_resistance u.uprops[PSYCHIC_RES].intrinsic
#define EPsychic_resistance u.uprops[PSYCHIC_RES].extrinsic
#define Psychic_resistance (HPsychic_resistance || EPsychic_resistance)

#define HStun_resistance u.uprops[STUN_RES].intrinsic
#define EStun_resistance u.uprops[STUN_RES].extrinsic
#define Stun_resistance \
    (HStun_resistance || EStun_resistance                  \
     || youmonst.data == &mons[PM_SHIMMERING_DRAGON]       \
     || youmonst.data == &mons[PM_BABY_SHIMMERING_DRAGON])

#define HVulnerable_fire u.uprops[VULN_FIRE].intrinsic
#define EVulnerable_fire u.uprops[VULN_FIRE].extrinsic
#define Vulnerable_fire \
    (HVulnerable_fire || EVulnerable_fire  \
     || vulnerable_to(&youmonst, AD_FIRE))

#define HVulnerable_cold u.uprops[VULN_COLD].intrinsic
#define EVulnerable_cold u.uprops[VULN_COLD].extrinsic
#define Vulnerable_cold \
    (HVulnerable_cold || EVulnerable_cold  \
     || vulnerable_to(&youmonst, AD_COLD))

#define HVulnerable_elec u.uprops[VULN_ELEC].intrinsic
#define EVulnerable_elec u.uprops[VULN_ELEC].extrinsic
#define Vulnerable_elec \
    (HVulnerable_elec || EVulnerable_elec  \
     || vulnerable_to(&youmonst, AD_ELEC))

#define HVulnerable_acid u.uprops[VULN_ACID].intrinsic
#define EVulnerable_acid u.uprops[VULN_ACID].extrinsic
#define Vulnerable_acid	\
    (HVulnerable_acid || EVulnerable_acid  \
     || vulnerable_to(&youmonst, AD_ACID))

/* Hxxx due to FROMFORM only */
#define HAntimagic u.uprops[ANTIMAGIC].intrinsic
#define EAntimagic u.uprops[ANTIMAGIC].extrinsic
#define Antimagic (HAntimagic || EAntimagic)

#define HAcid_resistance u.uprops[ACID_RES].intrinsic
#define EAcid_resistance u.uprops[ACID_RES].extrinsic
#define Acid_resistance (HAcid_resistance || EAcid_resistance)

#define HStone_resistance u.uprops[STONE_RES].intrinsic
#define EStone_resistance u.uprops[STONE_RES].extrinsic
#define Stone_resistance (HStone_resistance || EStone_resistance)

#define HDeath_resistance u.uprops[DEATH_RES].intrinsic
#define EDeath_resistance u.uprops[DEATH_RES].extrinsic
#define Death_resistance (HDeath_resistance || EDeath_resistance)

#define HLycan_resistance u.uprops[WERE_RES].intrinsic
#define ELycan_resistance u.uprops[WERE_RES].extrinsic
#define Lycan_resistance \
    (HLycan_resistance || ELycan_resistance                                 \
     || youmonst.data->mlet == S_LICH || youmonst.data->mlet == S_ZOMBIE    \
     || youmonst.data->mlet == S_WRAITH || youmonst.data->mlet == S_VAMPIRE \
     || youmonst.data->mlet == S_GHOST || youmonst.data->mlet == S_MUMMY    \
     || youmonst.data->mlet == S_SKELETON || defended(&youmonst, AD_WERE))

/* Intrinsics only */
#define HSick_resistance u.uprops[SICK_RES].intrinsic
#define ESick_resistance u.uprops[SICK_RES].extrinsic
#define Sick_resistance \
    (HSick_resistance || ESick_resistance                                        \
     || youmonst.data->mlet == S_FUNGUS || youmonst.data->mlet == S_ENT          \
     || youmonst.data->mlet == S_PLANT || youmonst.data->mlet == S_ZOMBIE        \
     || youmonst.data->mlet == S_WRAITH || youmonst.data->mlet == S_VAMPIRE      \
     || youmonst.data->mlet == S_GHOST || youmonst.data->mlet == S_MUMMY         \
     || youmonst.data->mlet == S_LICH || youmonst.data->mlet == S_SKELETON       \
     || youmonst.data->mlet == S_ANGEL || youmonst.data->mlet == S_DEMON         \
     || youmonst.data == &mons[PM_BABY_GREEN_DRAGON]                             \
     || youmonst.data == &mons[PM_GREEN_DRAGON] || defended(&youmonst, AD_DISE))

#define Invulnerable u.uprops[INVULNERABLE].intrinsic /* [Tom] */

/*** Troubles ***/
/* Pseudo-property */
#define Punished (uball != 0)

/* Many are implemented solely as timeouts (we use just intrinsic) */
#define HStun u.uprops[STUNNED].intrinsic /* timed or FROMFORM */
#define Stunned HStun

#define HConfusion u.uprops[CONFUSION].intrinsic
#define Confusion HConfusion

#define Blinded u.uprops[BLINDED].intrinsic
#define Blindfolded \
    (ublindf && !(ublindf->otyp == LENSES || ublindf->otyp == GOGGLES))
/* ...means blind because of a cover */
#define Blind \
    ((u.uroleplay.blind || Blinded || Blindfolded \
      || !haseyes(youmonst.data) || Hidinshell)   \
     && !(ublindf && ublindf->oartifact == ART_EYES_OF_THE_OVERWORLD))
/* ...the Eyes operate even when you really are blind
    or don't have any eyes */
#define Blindfolded_only \
    (Blindfolded && ublindf->oartifact != ART_EYES_OF_THE_OVERWORLD \
     && !u.uroleplay.blind && !Blinded && haseyes(youmonst.data))
/* ...blind because of a blindfold, and *only* that */

#define Hidinshell (u.uinshell > 0)

#define Sick u.uprops[SICK].intrinsic
#define Stoned u.uprops[STONED].intrinsic
#define Strangled u.uprops[STRANGLED].intrinsic
#define Vomiting u.uprops[VOMITING].intrinsic
#define Glib u.uprops[GLIB].intrinsic
#define Slimed u.uprops[SLIMED].intrinsic /* [Tom] */

/* Hallucination is solely a timeout */
#define HHallucination u.uprops[HALLUC].intrinsic
#define HHalluc_resistance u.uprops[HALLUC_RES].intrinsic
#define EHalluc_resistance u.uprops[HALLUC_RES].extrinsic
#define Halluc_resistance (HHalluc_resistance || EHalluc_resistance)
#define Hallucination \
    ((HHallucination && !Halluc_resistance) || u.uroleplay.hallu)

/* Timeout, plus a worn mask */
#define HDeaf u.uprops[DEAF].intrinsic
#define EDeaf u.uprops[DEAF].extrinsic
#define Deaf (HDeaf || EDeaf || u.uroleplay.deaf)

#define HFumbling u.uprops[FUMBLING].intrinsic
#define EFumbling u.uprops[FUMBLING].extrinsic
#define Fumbling (HFumbling || EFumbling)

#define HWounded_legs u.uprops[WOUNDED_LEGS].intrinsic
#define EWounded_legs u.uprops[WOUNDED_LEGS].extrinsic
#define Wounded_legs (HWounded_legs || EWounded_legs)

#define HSleepy u.uprops[SLEEPY].intrinsic
#define ESleepy u.uprops[SLEEPY].extrinsic
#define Sleepy (HSleepy || ESleepy)

#define HHunger u.uprops[HUNGER].intrinsic
#define EHunger u.uprops[HUNGER].extrinsic
#define Hunger (HHunger || EHunger)

/*** Vision and senses ***/
#define HSee_invisible u.uprops[SEE_INVIS].intrinsic
#define ESee_invisible u.uprops[SEE_INVIS].extrinsic
#define See_invisible (HSee_invisible || ESee_invisible)

#define HTelepat u.uprops[TELEPAT].intrinsic
#define ETelepat u.uprops[TELEPAT].extrinsic
#define Blind_telepat (HTelepat || ETelepat)
#define Unblind_telepat (ETelepat)

#define HWarning u.uprops[WARNING].intrinsic
#define EWarning u.uprops[WARNING].extrinsic
#define Warning (HWarning || EWarning)

#define HFood_sense u.uprops[FOOD_SENSE].intrinsic
#define EFood_sense u.uprops[FOOD_SENSE].extrinsic
#define Food_sense (HFood_sense || EFood_sense)

/* (intrinsic x-ray vision prop is not currently used) */
#define HXray_vision u.uprops[XRAY_VISION].intrinsic
#define EXray_vision u.uprops[XRAY_VISION].extrinsic
#define Xray_vision (HXray_vision || EXray_vision)

/* Warning for a specific type of monster */
#define HWarn_of_mon u.uprops[WARN_OF_MON].intrinsic
#define EWarn_of_mon u.uprops[WARN_OF_MON].extrinsic
#define Warn_of_mon (HWarn_of_mon || EWarn_of_mon)

#define HUndead_warning u.uprops[WARN_UNDEAD].intrinsic
#define Undead_warning (HUndead_warning)

#define HSearching u.uprops[SEARCHING].intrinsic
#define ESearching u.uprops[SEARCHING].extrinsic
#define Searching (HSearching || ESearching)

#define HClairvoyant u.uprops[CLAIRVOYANT].intrinsic
#define EClairvoyant u.uprops[CLAIRVOYANT].extrinsic
#define BClairvoyant u.uprops[CLAIRVOYANT].blocked
#define Clairvoyant ((HClairvoyant || EClairvoyant) && !BClairvoyant)

#define HInfravision u.uprops[INFRAVISION].intrinsic
#define EInfravision u.uprops[INFRAVISION].extrinsic
#define Infravision (HInfravision || EInfravision)

#define HUltravision u.uprops[ULTRAVISION].intrinsic
#define EUltravision u.uprops[ULTRAVISION].extrinsic
#define Ultravision (HUltravision || EUltravision)

#define HDetect_monsters u.uprops[DETECT_MONSTERS].intrinsic
#define EDetect_monsters u.uprops[DETECT_MONSTERS].extrinsic
#define Detect_monsters (HDetect_monsters || EDetect_monsters)

/*** Appearance and behavior ***/
#define Adornment u.uprops[ADORNED].extrinsic

#define HInvis u.uprops[INVIS].intrinsic
#define EInvis u.uprops[INVIS].extrinsic
#define BInvis u.uprops[INVIS].blocked
#define Invis ((HInvis || EInvis) && !BInvis)
#define Invisible (Invis && !See_invisible)
/* Note: invisibility also hides inventory and steed */

#define HDisplaced u.uprops[DISPLACED].intrinsic /* timed from corpse */
#define EDisplaced u.uprops[DISPLACED].extrinsic /* worn item */
#define Displaced (HDisplaced || EDisplaced || is_displaced(youmonst.data))

#define HStealth u.uprops[STEALTH].intrinsic
#define EStealth u.uprops[STEALTH].extrinsic
#define BStealth u.uprops[STEALTH].blocked
#define Stealth ((HStealth || EStealth) && !BStealth)

#define HAggravate_monster u.uprops[AGGRAVATE_MONSTER].intrinsic
#define EAggravate_monster u.uprops[AGGRAVATE_MONSTER].extrinsic
#define Aggravate_monster (HAggravate_monster || EAggravate_monster)

#define HConflict u.uprops[CONFLICT].intrinsic
#define EConflict u.uprops[CONFLICT].extrinsic
#define Conflict (HConflict || EConflict)

/*** Transportation ***/
#define HJumping u.uprops[JUMPING].intrinsic
#define EJumping u.uprops[JUMPING].extrinsic
#define Jumping (HJumping || EJumping || is_jumper(youmonst.data))

#define HTeleportation u.uprops[TELEPORT].intrinsic
#define ETeleportation u.uprops[TELEPORT].extrinsic
#define Teleportation (HTeleportation || ETeleportation)

#define HTeleport_control u.uprops[TELEPORT_CONTROL].intrinsic
#define ETeleport_control u.uprops[TELEPORT_CONTROL].extrinsic
#define Teleport_control (HTeleport_control || ETeleport_control)

/* HLevitation has I_SPECIAL set if levitating due to blessed potion
   which allows player to use the '>' command to end levitation early */
#define HLevitation u.uprops[LEVITATION].intrinsic
#define ELevitation u.uprops[LEVITATION].extrinsic
/* BLevitation has I_SPECIAL set if trapped in the floor,
   FROMOUTSIDE set if inside solid rock (or in water on Plane of Water)
   W_ARM set if in big_wings() form and wearing blocking body armor
   (the last one obviously won't block the steed from flying) */
#define BLevitation u.uprops[LEVITATION].blocked
#define Levitation ((HLevitation || ELevitation) && !BLevitation)
/* Can't touch surface, can't go under water; overrides all others */
#define Lev_at_will \
    (((HLevitation & I_SPECIAL) != 0L || (ELevitation & W_ARTI) != 0L) \
     && (HLevitation & ~(I_SPECIAL | TIMEOUT)) == 0L                   \
     && (ELevitation & ~W_ARTI) == 0L)

/* Flying is overridden by Levitation */
#define HFlying u.uprops[FLYING].intrinsic
#define EFlying u.uprops[FLYING].extrinsic
/* BFlying has I_SPECIAL set if levitating or trapped in the floor or both,
   FROMOUTSIDE set if inside solid rock (or in water on Plane of Water) */
#define BFlying u.uprops[FLYING].blocked
#define Flying \
    ((((EFlying || (HFlying & ~FROMFORM)                                    \
       || (HFlying && !big_wings(raceptr(&youmonst))))                      \
       || (u.usteed && is_flyer(u.usteed->data))) && !(BFlying & ~W_ARMOR)) \
     || (HFlying && !BFlying)) \
/* May touch surface; does not override any others */

#define HWwalking u.uprops[WWALKING].intrinsic
#define EWwalking u.uprops[WWALKING].extrinsic
#define Wwalking ((HWwalking || EWwalking) && !Is_waterlevel(&u.uz))
/* Don't get wet, can't go under water; overrides others except levitation */
/* Wwalking is meaningless on water level */

#define HSwimming u.uprops[SWIMMING].intrinsic
#define ESwimming u.uprops[SWIMMING].extrinsic /* [Tom] */
#define Swimming \
    (HSwimming || ESwimming || (u.usteed && is_swimmer(u.usteed->data)))
/* Get wet, don't go under water unless if amphibious */

#define HMagical_breathing u.uprops[MAGICAL_BREATHING].intrinsic
#define EMagical_breathing u.uprops[MAGICAL_BREATHING].extrinsic
#define Amphibious \
    (HMagical_breathing || EMagical_breathing                  \
     || amphibious(youmonst.data) || racial_tortle(&youmonst))
/* Get wet, may go under surface */

#define See_underwater \
    ((HSwimming && (HMagical_breathing || amphibious(youmonst.data)  \
                    || racial_tortle(&youmonst)))                    \
     || (ublindf && ublindf->oartifact == ART_EYES_OF_THE_OVERWORLD) \
     || (ublindf && ublindf->otyp == GOGGLES))

#define Breathless_nomagic \
    (breathless(youmonst.data) || racial_zombie(&youmonst) \
     || racial_vampire(&youmonst))

#define Breathless \
    (HMagical_breathing || EMagical_breathing || Breathless_nomagic)

#define Underwater (u.uinwater)
/* Note that Underwater and u.uinwater are both used in code.
   The latter form is for later implementation of other in-water
   states, like swimming, wading, etc. */

#define HPasses_walls u.uprops[PASSES_WALLS].intrinsic
#define EPasses_walls u.uprops[PASSES_WALLS].extrinsic
#define Passes_walls (HPasses_walls || EPasses_walls)

#define HPasses_trees u.uprops[PASSES_TREES].intrinsic
#define EPasses_trees u.uprops[PASSES_TREES].extrinsic
#define Passes_trees (HPasses_trees || EPasses_trees)

/*** Physical attributes ***/
#define HSlow_digestion u.uprops[SLOW_DIGESTION].intrinsic
#define ESlow_digestion u.uprops[SLOW_DIGESTION].extrinsic
#define Slow_digestion (HSlow_digestion || ESlow_digestion) /* KMH */

#define HHalf_spell_damage u.uprops[HALF_SPDAM].intrinsic
#define EHalf_spell_damage u.uprops[HALF_SPDAM].extrinsic
#define Half_spell_damage (HHalf_spell_damage || EHalf_spell_damage)

/*
 * Physical damage
 *
 * Damage is NOT physical damage if (in order of priority):
 * 1. it already qualifies for some other special category
 *    for which a special resistance already exists in the game
 *    including: cold, fire, shock, acid, and magic.
 *    Note that fire is extended to include all non-acid forms of
 *    burning, even boiling water since that is already dealt with
 *    by fire resistance, and in most or all cases is caused by fire.
 * 2. it doesn't leave a mark. Marks include destruction of, or
 *    damage to, an internal organ (including the brain),
 *    lacerations, bruises, crushed body parts, bleeding.
 *
 * The following were evaluated and determined _NOT_ to be
 * susceptible to Half_physical_damage protection:
 *   Being caught in a fireball                      [fire damage]
 *   Sitting in lava                                 [lava damage]
 *   Thrown potion (acid)                            [acid damage]
 *   Splattered burning oil from thrown potion       [fire damage]
 *   Mixing water and acid                           [acid damage]
 *   Molten lava (entering or being splashed)        [lava damage]
 *   boiling water from a sink                       [fire damage]
 *   Fire traps                                      [fire damage]
 *   Scrolls of fire (confused and otherwise)        [fire damage]
 *   Alchemical explosion                            [not physical]
 *   System shock                                    [shock damage]
 *   Bag of holding explosion                        [magical]
 *   Being undead-turned by your god                 [magical]
 *   Level-drain                                     [magical]
 *   Magical explosion of a magic trap               [magical]
 *   Sitting on a throne with a bad effect           [magical]
 *   Contaminated water from a sink                  [poison/sickness]
 *   Contact-poisoned spellbooks                     [poison/sickness]
 *   Eating acidic/poisonous/mildly-old corpses      [poison/sickness]
 *   Eating a poisoned weapon while polyselfed       [poison/sickness]
 *   Engulfing a zombie or mummy (AT_ENGL in hmonas) [poison/sickness]
 *   Quaffed potions of sickness, lit oil, acid      [poison/sickness]
 *   Pyrolisks' fiery gaze                           [fire damage]
 *   Any passive attack                              [most don't qualify]
 */

#define HHalf_physical_damage u.uprops[HALF_PHDAM].intrinsic
#define EHalf_physical_damage u.uprops[HALF_PHDAM].extrinsic
#define Half_physical_damage (HHalf_physical_damage || EHalf_physical_damage)

#define HRegeneration u.uprops[REGENERATION].intrinsic
#define ERegeneration u.uprops[REGENERATION].extrinsic
#define Regeneration (HRegeneration || ERegeneration)

#define HEnergy_regeneration u.uprops[ENERGY_REGENERATION].intrinsic
#define EEnergy_regeneration u.uprops[ENERGY_REGENERATION].extrinsic
#define Energy_regeneration (HEnergy_regeneration || EEnergy_regeneration)

#define HProtection u.uprops[PROTECTION].intrinsic
#define EProtection u.uprops[PROTECTION].extrinsic
#define Protection (HProtection || EProtection)

#define HProtection_from_shape_changers \
    u.uprops[PROT_FROM_SHAPE_CHANGERS].intrinsic
#define EProtection_from_shape_changers \
    u.uprops[PROT_FROM_SHAPE_CHANGERS].extrinsic
#define Protection_from_shape_changers \
    (HProtection_from_shape_changers || EProtection_from_shape_changers)

#define HPolymorph u.uprops[POLYMORPH].intrinsic
#define EPolymorph u.uprops[POLYMORPH].extrinsic
#define Polymorph (HPolymorph || EPolymorph)

#define HPolymorph_control u.uprops[POLYMORPH_CONTROL].intrinsic
#define EPolymorph_control u.uprops[POLYMORPH_CONTROL].extrinsic
#define Polymorph_control (HPolymorph_control || EPolymorph_control)

#define HUnchanging u.uprops[UNCHANGING].intrinsic
#define EUnchanging u.uprops[UNCHANGING].extrinsic
#define Unchanging (HUnchanging || EUnchanging) /* KMH */

#define HFast u.uprops[FAST].intrinsic
#define EFast u.uprops[FAST].extrinsic
#define SFast (Underwater && is_fast_underwater(youmonst.data))
#define Fast ((HFast || EFast || SFast) && !Slow)
#define Very_fast (((HFast & ~INTRINSIC) || EFast || SFast) && !Slow)

#define HSlow u.uprops[SLOW].intrinsic
#define ESlow u.uprops[SLOW].extrinsic
#define Slow (HSlow || ESlow)

#define HReflecting u.uprops[REFLECTING].intrinsic
#define EReflecting u.uprops[REFLECTING].extrinsic
#define Reflecting \
    (HReflecting || EReflecting                     \
     || (youmonst.data == &mons[PM_SILVER_DRAGON]))

#define HWithering u.uprops[WITHERING].intrinsic
#define EWithering u.uprops[WITHERING].extrinsic
#define BWithering u.uprops[WITHERING].blocked
#define Withering ((HWithering || EWithering) && !BWithering)

#define Free_action u.uprops[FREE_ACTION].extrinsic /* [Tom] */

#define HBarkskin u.uprops[BARKSKIN].intrinsic
#define EBarkskin u.uprops[BARKSKIN].extrinsic
#define Barkskin (HBarkskin || EBarkskin)

#define HStoneskin u.uprops[STONESKIN].intrinsic
#define EStoneskin u.uprops[STONESKIN].extrinsic
#define Stoneskin (HStoneskin || EStoneskin)

#define Fixed_abil u.uprops[FIXED_ABIL].extrinsic /* KMH */

#define HLifesaved u.uprops[LIFESAVED].intrinsic /* Draugr race */
#define ELifesaved u.uprops[LIFESAVED].extrinsic
#define Lifesaved (HLifesaved || ELifesaved)

/*
 * Some pseudo-properties.
 */

/* unconscious() includes u.usleep but not is_fainted(); the multi test is
   redundant but allows the function calls to be skipped most of the time */
#define Unaware (multi < 0 && (unconscious() || is_fainted()))

/* Whether the hero is in a form that dislikes a certain material */
#define Hate_material(material) \
    (hates_material(raceptr(&youmonst), material) \
     || mon_hates_material(&youmonst, material))

#endif /* YOUPROP_H */
