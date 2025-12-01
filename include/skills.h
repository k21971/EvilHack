/* NetHack 3.6	skills.h	$NHDT-Date: 1547255911 2019/01/12 01:18:31 $  $NHDT-Branch: NetHack-3.6.2-beta01 $:$NHDT-Revision: 1.15 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985-1999. */
/*-Copyright (c) Pasi Kallinen, 2017. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef SKILLS_H
#define SKILLS_H

/* Much of this code was taken from you.h.  It is now
 * in a separate file so it can be included in objects.c.
 */

enum p_skills {
    /* Code to denote that no skill is applicable */
    P_NONE = 0,

/* Weapon Skills -- Stephen White
 * Order matters and are used in macros.
 * Positive values denote hand-to-hand weapons or launchers.
 * Negative values denote ammunition or missiles.
 * Update weapon.c if you amend any skills.
 * Also used for oc_subtyp.
 */
    P_DAGGER             =  1, /* includes knives */
    P_AXE                =  2,
    P_PICK_AXE           =  3,
    P_SHORT_SWORD        =  4,
    P_BROAD_SWORD        =  5,
    P_LONG_SWORD         =  6,
    P_TWO_HANDED_SWORD   =  7,
    P_SABER              =  8, /* Curved sword, includes scimitar */
    P_CLUB               =  9, /* Heavy-shafted bludgeon */
    P_MACE               = 10, /* includes morning star, rod */
    P_FLAIL              = 11, /* Two pieces hinged or chained together */
    P_HAMMER             = 12, /* Heavy head on the end */
    P_QUARTERSTAFF       = 13, /* Long-shafted bludgeon */
    P_POLEARMS           = 14, /* attack two or three steps away */
    P_SPEAR              = 15, /* includes javelin */
    P_TRIDENT            = 16,
    P_LANCE              = 17,
    P_BOW                = 18, /* launchers */
    P_SLING              = 19,
    P_CROSSBOW           = 20,
    P_DART               = 21, /* hand-thrown missiles */
    P_SHURIKEN           = 22,
    P_BOOMERANG          = 23,
    P_WHIP               = 24, /* flexible, one-handed */
    P_UNICORN_HORN       = 25, /* last weapon, one-handed */

    /* Spell Skills added by Larry Stewart-Zerba */
    P_ATTACK_SPELL       = 26,
    P_HEALING_SPELL      = 27,
    P_DIVINATION_SPELL   = 28,
    P_ENCHANTMENT_SPELL  = 29,
    P_CLERIC_SPELL       = 30,
    P_ESCAPE_SPELL       = 31,
    P_MATTER_SPELL       = 32,
    P_EVOCATION_SPELL    = 33,

    /* Other types of combat */
    P_BARE_HANDED_COMBAT = 34, /* actually weaponless; gloves are ok */
    P_TWO_WEAPON_COMBAT  = 35, /* pair of weapons, one in each hand */
    P_SHIELD             = 36, /* How well you use a shield */
    P_RIDING             = 37, /* How well you control your steed */
    P_PET_HANDLING       = 38, /* How well you command pets */

    P_NUM_SKILLS         = 39
};

#define P_MARTIAL_ARTS P_BARE_HANDED_COMBAT /* Role distinguishes */
#define P_THIEVERY P_BARE_HANDED_COMBAT

#define P_FIRST_WEAPON P_DAGGER
#define P_LAST_WEAPON P_UNICORN_HORN

#define P_FIRST_SPELL P_ATTACK_SPELL
#define P_LAST_SPELL P_EVOCATION_SPELL

#define P_LAST_H_TO_H P_PET_HANDLING
#define P_FIRST_H_TO_H P_BARE_HANDED_COMBAT

/* These roles qualify for a martial arts bonus */
#define martial_bonus() (Role_if(PM_SAMURAI) || Role_if(PM_MONK))

/*
 * These are the standard weapon skill levels.  It is important that
 * the lowest "valid" skill be be 1.  The code calculates the
 * previous amount to practice by calling  practice_needed_to_advance()
 * with the current skill-1.  To work out for the UNSKILLED case,
 * a value of 0 needed.
 */
enum skill_levels {
    P_ISRESTRICTED = 0, /* unskilled and can't be advanced */
    P_UNSKILLED    = 1, /* unskilled so far but can be advanced */
    /* Skill levels Basic/Advanced/Expert had long been used by
       Heroes of Might and Magic (tm) and its sequels... */
    P_BASIC        = 2,
    P_SKILLED      = 3,
    P_EXPERT       = 4,
    /* when the skill system was adopted into nethack, levels beyond expert
       were unnamed and just used numbers.  Devteam coined them Master and
       Grand Master.  Sometime after that, Heroes of Might and Magic IV (tm)
       was released and had two more levels which use these same names. */
    P_MASTER       = 5, /* Unarmed combat/martial arts/thievery/shield only */
    P_GRAND_MASTER = 6  /* ditto */
};

#define practice_needed_to_advance(level) ((level) * (level) *20)

/* The hero's skill in various weapons. */
struct skills {
    xchar skill;
    xchar max_skill;
    unsigned short advance;
};

#define P_SKILL(type) (u.weapon_skills[type].skill)
#define P_MAX_SKILL(type) (u.weapon_skills[type].max_skill)
#define P_ADVANCE(type) (u.weapon_skills[type].advance)
#define P_RESTRICTED(type) (u.weapon_skills[type].skill == P_ISRESTRICTED)
#define P_UN_SKILL(type) (u.weapon_skills[type].skill == P_UNSKILLED)
#define P_BA_SKILL(type) (u.weapon_skills[type].skill == P_BASIC)
#define P_SK_SKILL(type) (u.weapon_skills[type].skill == P_SKILLED)
#define P_EX_SKILL(type) (u.weapon_skills[type].skill == P_EXPERT)

#define P_SKILL_LIMIT 60 /* Max number of skill advancements */

/* Initial skill matrix structure; used in u_init.c and weapon.c */
struct def_skill {
    xchar skill;
    xchar skmax;
};

#endif /* SKILLS_H */
