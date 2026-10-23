/* NetHack 5.0	weapon.c	$NHDT-Date: 1781973073 2026/06/20 16:31:13 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.147 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2011. */
/* NetHack may be freely redistributed.  See license for details. */

/*
 *      This module contains code for calculation of "to hit" and damage
 *      bonuses for any given weapon used, as well as weapons selection
 *      code for monsters.
 */
#include "hack.h"

staticfn void give_may_advance_msg(int);
staticfn void finish_towel_change(struct obj *obj, int) NONNULLARG1;
staticfn boolean could_advance(int);
staticfn boolean peaked_skill(int);
staticfn int slots_required(int);
staticfn void skill_advance(int);
staticfn void add_skills_to_menu(winid, boolean, boolean);

/* Categories whose names don't come from OBJ_NAME(objects[type])
 */
#define PN_BARE_HANDED (-1) /* includes martial arts and thievery */
#define PN_TWO_WEAPONS (-2)
#define PN_SHIELD (-3)
#define PN_RIDING (-4)
#define PN_POLEARMS (-5)
#define PN_SABER (-6)
#define PN_HAMMER (-7)
#define PN_WHIP (-8)
#define PN_ATTACK_SPELL (-9)
#define PN_HEALING_SPELL (-10)
#define PN_DIVINATION_SPELL (-11)
#define PN_ENCHANTMENT_SPELL (-12)
#define PN_CLERIC_SPELL (-13)
#define PN_ESCAPE_SPELL (-14)
#define PN_MATTER_SPELL (-15)
#define PN_EVOCATION_SPELL (-16)
#define PN_PET_HANDLING (-17)
#define PN_THIEVERY (-18)
#define PN_TELEKINESIS (-19)

static NEARDATA const short skill_names_indices[P_NUM_SKILLS] = {
    /* Weapon */
    0, DAGGER, AXE, PICK_AXE, SHORT_SWORD, BROADSWORD, LONG_SWORD,
    TWO_HANDED_SWORD, PN_SABER, CLUB, MACE, FLAIL, PN_HAMMER, QUARTERSTAFF,
    PN_POLEARMS, SPEAR, TRIDENT, LANCE, BOW, SLING, CROSSBOW, DART, SHURIKEN,
    BOOMERANG, PN_WHIP, UNICORN_HORN,
    /* Spell */
    PN_ATTACK_SPELL, PN_HEALING_SPELL, PN_DIVINATION_SPELL,
    PN_ENCHANTMENT_SPELL, PN_CLERIC_SPELL, PN_ESCAPE_SPELL,
    PN_MATTER_SPELL, PN_EVOCATION_SPELL,
    /* Other */
    PN_BARE_HANDED, PN_TWO_WEAPONS, PN_THIEVERY, PN_SHIELD, PN_RIDING,
    PN_PET_HANDLING, PN_TELEKINESIS
};

/* note: entry [0] isn't used */
static NEARDATA const char *const odd_skill_names[] = {
    "no skill", "bare hands", /* use barehands_or_martial[] instead */
    "two weapon combat", "shield", "riding", "polearms", "saber", "hammer",
    "whip", "attack spells", "healing spells", "divination spells",
    "enchantment spells", "clerical spells", "escape spells", "matter spells",
    "evocation spells", "pet handling", "thievery", "telekinesis",
};
/* indexed via is_martial() */
static NEARDATA const char *const barehands_or_martial[] = {
    "bare handed combat", "martial arts"
};

#define P_NAME(type)                                    \
    ((skill_names_indices[type] > 0)                    \
         ? OBJ_NAME(objects[skill_names_indices[type]]) \
         : (type == P_BARE_HANDED_COMBAT)               \
               ? barehands_or_martial[martial_bonus()]  \
               : odd_skill_names[-skill_names_indices[type]])

/* targets that provide attacker with small to-hit bonus when using a spear */
static NEARDATA const char kebabable[] = {
    S_XORN, S_DRAGON, S_JABBERWOCK, S_NAGA, S_GIANT,  '\0'
};

staticfn void
give_may_advance_msg(int skill)
{
    You_feel("more confident in your %sskills.",
             (skill == P_NONE) ? ""
                 : (skill <= P_LAST_WEAPON) ? "weapon "
                     : (skill <= P_LAST_SPELL) ? "spell casting "
                         : (skill == P_SHIELD) ? "defensive "
                             : (skill == P_RIDING) ? "riding "
                                 : (skill == P_THIEVERY) ? "thievery "
                                     : (skill == P_PET_HANDLING)
                                       ? "pet handling "
                                       : (skill == P_TELEKINESIS)
                                         ? "telekinetic "
                                         : "fighting ");
    (void) handle_tip(TIP_ENHANCE);
}

/* weapon's skill category name for use as generalized description of weapon;
   mostly used to shorten "you drop your <weapon>" messages when slippery
   fingers or polymorph causes hero to involuntarily drop wielded weapon(s) */
const char *
weapon_descr(struct obj *obj)
{
    int skill = weapon_type(obj);
    const char *descr = P_NAME(skill);

    /* assorted special cases */
    switch (skill) {
    case P_NONE:
        /* not a weapon or weptool: use item class name;
           override class name for things where it sounds strange and
           for things that aren't unexpected to find being wielded:
           corpses, tins, eggs, and globs avoid "food",
           statues and boulders avoid "large rock",
           and towels and tin openers avoid "tool" */
        descr = (obj->otyp == CORPSE || obj->otyp == TIN || obj->otyp == EGG
                 || obj->otyp == STATUE || obj->otyp == BOULDER
                 || obj->otyp == TOWEL || obj->otyp == TIN_OPENER)
                ? OBJ_NAME(objects[obj->otyp])
                : obj->globby ? "glob"
                  : def_oc_syms[(int) obj->oclass].name;
        break;
    case P_SLING:
        if (is_ammo(obj))
            descr = (obj->otyp == ROCK || is_graystone(obj))
                        ? "stone"
                        /* avoid "rock"; what about known glass? */
                        : (obj->oclass == GEM_CLASS)
                            ? "gem"
                            /* in case somebody adds odd sling ammo */
                            : def_oc_syms[(int) obj->oclass].name;
        break;
    case P_BOW:
        if (is_ammo(obj))
            descr = "arrow";
        break;
    case P_CROSSBOW:
        if (is_ammo(obj))
            descr = "bolt";
        break;
    case P_FLAIL:
        if (obj->otyp == GRAPPLING_HOOK)
            descr = "hook";
        break;
    case P_PICK_AXE:
        /* even if "dwarvish mattock" hasn't been discovered yet */
        if (obj->otyp == DWARVISH_MATTOCK)
            descr = "mattock";
        break;
    default:
        break;
    }
    return makesingular(descr);
}

/* the "to hit" bonuses of otmp that don't depend on the target */
int
base_hitbonus(struct obj *otmp)
{
    int tmp = 0;

    if (!otmp)
        return 0;

    if (otmp->oclass == WEAPON_CLASS || is_weptool(otmp))
        tmp += otmp->spe;

    /* Put weapon-specific "to hit" bonuses in below: */
    tmp += objects[otmp->otyp].oc_hitbon;
    return tmp;
}

/* base to-hit shown on the status line: the terms of find_roll_to_hit()
   that don't involve the target, since the display has no way of
   knowing what the hero intends to attack */
int
botl_hitbonus(void)
{
    int tmp, tmp2;
    int wepskill, twowepskill, useskill;
    uchar aatyp = gy.youmonst.data->mattk[0].aatyp;

    tmp = 1 + abon() + u.uhitinc
          + (sgn(Luck) * ((abs(Luck) + 2) / 3))
          + maybe_polyd(gy.youmonst.data->mlevel,
                        (u.ulevel > 20 ? 10 + (u.ulevel / 2)
                                       : u.ulevel));

    /* don't reveal the enchantment of an unidentified ring */
    if (uleft && uleft->otyp == RIN_INCREASE_ACCURACY && !uleft->known)
        tmp -= uleft->spe;
    if (uright && uright->otyp == RIN_INCREASE_ACCURACY && !uright->known)
        tmp -= uright->spe;

    if (u.ulevel == 30)
        tmp += 5;

    if (Role_if(PM_MONK) && !Upolyd) {
        if (uarm)
            tmp -= gu.urole.spelarmr + 20;
        else if (!uwep && (!uarms || is_bracer(uarms)))
            tmp += (u.ulevel / 3) + 2;
    }

    if (maybe_polyd(is_drow(gy.youmonst.data), Race_if(PM_DROW))) {
        if (spot_is_dark(u.ux, u.uy)) {
            if (!uwep || is_drow_weapon(uwep)
                || uwep->oartifact == ART_SHADOWBLADE)
                tmp += (u.ulevel / 3) + 2;
        } else {
            tmp -= 4;
        }
    }

    if (uwep && uwep->otyp == HEAVY_IRON_BALL)
        tmp += 4;

    if (!uwep && uarmg)
        tmp += uarmg->spe;

    if ((tmp2 = near_capacity()) != 0)
        tmp -= (tmp2 * 2) - 1;
    if (u.utrap)
        tmp -= 3;

    if (!uwep && druid_form)
        tmp += (u.ulevel / 3) + 5;

    if (!uwep && vampire_form)
        tmp += (u.ulevel / 3) + 5;

    if (uwep) {
        if (uwep->forged_qual == FQ_EXCEPTIONAL
            || (u.twoweap && uswapwep
                && uswapwep->forged_qual == FQ_EXCEPTIONAL))
            tmp += 1;
        if (uwep->forged_qual == FQ_INFERIOR
            || (u.twoweap && uswapwep
                && uswapwep->forged_qual == FQ_INFERIOR))
            tmp -= 2;
    }

    if (aatyp == AT_WEAP || aatyp == AT_CLAW) {
        if (uwep) {
            tmp += base_hitbonus(uwep);
            /* don't reveal an enchantment the hero hasn't learned */
            if (!uwep->known
                && (uwep->oclass == WEAPON_CLASS || is_weptool(uwep)))
                tmp -= uwep->spe;
            /* the one hitval() term that doesn't depend on the target */
            if (racial_tortle(&gy.youmonst)
                && (is_spear(uwep) || uwep->otyp == TRIDENT))
                tmp += 2;
        }
        tmp += weapon_hit_bonus(uwep);
    } else if (aatyp == AT_KICK && martial_bonus()) {
        tmp += weapon_hit_bonus((struct obj *) 0);
    }

    if (uwep && aatyp == AT_WEAP && !u.uswallow) {
        /* potions make passable improvised weapons; raise the
           unskilled/restricted to-hit ceiling from 15 to 18 */
        int hitcap = (uwep->oclass == POTION_CLASS) ? 18 : 15;

        wepskill = weapon_skill_level(uwep);
        twowepskill = P_SKILL(P_TWO_WEAPON_COMBAT);
        /* use the lesser skill of two-weapon or your primary */
        useskill = (u.twoweap && twowepskill < wepskill) ? twowepskill
                                                         : wepskill;
        if ((useskill == P_UNSKILLED || useskill == P_ISRESTRICTED)
            && tmp > hitcap)
            tmp = hitcap;
    }
    return tmp;
}

/*
 *      hitval returns an integer representing the "to hit" bonuses
 *      of "otmp" against the monster. magr is the attacker (the hero
 *      via &gy.youmonst, a monster, or (struct monst *) 0 for an
 *      anonymous source such as a trap) and may be NULL.
 */
int
hitval(struct monst *magr, struct obj *otmp, struct monst *mdef)
{
    int tmp = 0;
    struct permonst *ptr = r_data(mdef);
    boolean Is_weapon = (otmp->oclass == WEAPON_CLASS || is_weptool(otmp));

    tmp += base_hitbonus(otmp);

    /* Put weapon vs. monster type "to hit" bonuses in below: */

    /* Blessed weapons used against undead or demons */
    if (Is_weapon && otmp->blessed && mon_hates_blessings(mdef))
        tmp += 2;

    /* Cursed weapons used against angelic beings */
    if (Is_weapon && otmp->cursed
        && (is_angel(ptr) || is_aasimar(ptr)))
        tmp += 2;

    /* Infidels get a slight bonus against lawful or
       neutral monsters when using cursed weapons */
    if (magr == &gy.youmonst && Is_weapon && otmp->cursed
        && Role_if(PM_INFIDEL) && (mon_aligntyp(mdef) == A_LAWFUL
                                   || mon_aligntyp(mdef) == A_NEUTRAL))
        tmp += 1;

    if (is_spear(otmp) && strchr(kebabable, ptr->mlet))
        tmp += 2;

    /* trident is highly effective against swimmers */
    if (otmp->otyp == TRIDENT && is_swimmer(ptr)) {
        /* &youmonst has mx/my == 0 (off-map sentinel); read the real
           position from u.ux/u.uy when scoring against the player */
        coordxy dx = (mdef == &gy.youmonst) ? u.ux : mdef->mx;
        coordxy dy = (mdef == &gy.youmonst) ? u.uy : mdef->my;

        if (is_damp_terrain(dx, dy))
            tmp += 4;
        else if (ptr->mlet == S_EEL || ptr->mlet == S_SNAKE)
            tmp += 2;
    }

    /* Picks used against xorns and earth elementals */
    if (is_pick(otmp) && (passes_walls(ptr) && thick_skinned(ptr)))
        tmp += 2;

    /* Tortles receive a slight bonus to hit when using
       spears or tridents */
    if (magr && racial_tortle(magr)
        && (is_spear(otmp) || otmp->otyp == TRIDENT))
        tmp += 2;

    /* Check specially named weapon "to hit" bonuses */
    if (otmp->oartifact)
        tmp += spec_abon(otmp, mdef);

    return tmp;
}

/* Historical note: The original versions of Hack used a range of damage
 * which was similar to, but not identical to, the damage used in Advanced
 * Dungeons and Dragons.  I figured that since it was so close, I may as well
 * make it exactly the same as AD&D, adding some more weapons in the process.
 * This has the advantage that it is at least possible that the player would
 * already know the damage of at least some of the weapons.  This was circa
 * 1987 and I used the table from Unearthed Arcana until I got tired of typing
 * them in (leading to something of an imbalance towards weapons early in
 * alphabetical order).  The data structure still doesn't include fields that
 * fully allow the appropriate damage to be described (there's no way to say
 * 3d6 or 1d6+1) so we add on the extra damage in dmgval() if the weapon
 * doesn't do an exact die of damage.
 *
 * Of course new weapons were added later in the development of Nethack.  No
 * AD&D consistency was kept, but most of these don't exist in AD&D anyway.
 *
 * Second edition AD&D came out a few years later; luckily it used the same
 * table.  As of this writing (1999), third edition is in progress but not
 * released.  Let's see if the weapon table stays the same.  --KAA
 * October 2000: It didn't.  Oh, well.
 */

/*
 *      dmgval returns an integer representing the damage bonuses
 *      of "otmp" against the monster. magr is the attacker (the hero
 *      via &gy.youmonst, a monster, or (struct monst *) 0 for an
 *      anonymous source such as a trap) and may be NULL.
 */
int
dmgval(struct monst *magr, struct obj *otmp, struct monst *mdef)
{
    int tmp = 0, otyp = otmp->otyp;
    struct permonst *ptr = r_data(mdef);
    boolean Is_weapon = (otmp->oclass == WEAPON_CLASS || is_weptool(otmp));

    if (otyp == CREAM_PIE)
        return 0;

    if (bigmonst(ptr)) {
        if (objects[otyp].oc_wldam)
            tmp = rnd(objects[otyp].oc_wldam);
        switch (otyp) {
        case IRON_CHAIN:
        case CROSSBOW_BOLT:
        case DARK_ELVEN_CROSSBOW_BOLT:
        case MORNING_STAR:
        case ORCISH_MORNING_STAR:
        case PARTISAN:
        case RUNESWORD:
        case ELVEN_BROADSWORD:
        case DARK_ELVEN_BROADSWORD:
        case BROADSWORD:
            tmp++;
            break;

        case FLAIL:
        case RANSEUR:
        case VOULGE:
            tmp += rnd(4);
            break;

        case ACID_VENOM:
        case HALBERD:
        case SPETUM:
            tmp += rnd(6);
            break;

        case BATTLE_AXE:
        case BARDICHE:
        case TRIDENT:
            tmp += d(2, 4);
            break;

        case TSURUGI:
        case DWARVISH_MATTOCK:
        case TWO_HANDED_SWORD:
            tmp += d(2, 6);
            break;

        case TRIPLE_HEADED_FLAIL:
            tmp += d(3, 6);
            break;
        }
    } else {
        if (objects[otyp].oc_wsdam)
            tmp = rnd(objects[otyp].oc_wsdam);
        switch (otyp) {
        case IRON_CHAIN:
        case CROSSBOW_BOLT:
        case DARK_ELVEN_CROSSBOW_BOLT:
        case MACE:
        case DARK_ELVEN_MACE:
        case HEAVY_MACE:
        case DARK_ELVEN_HEAVY_MACE:
        case ROD:
        case WAR_HAMMER:
        case HEAVY_WAR_HAMMER:
        case FLAIL:
        case TRIPLE_HEADED_FLAIL:
        case SPETUM:
        case TRIDENT:
            tmp++;
            break;

        case BATTLE_AXE:
        case BARDICHE:
        case BILL_GUISARME:
        case GUISARME:
        case LUCERN_HAMMER:
        case MORNING_STAR:
        case ORCISH_MORNING_STAR:
        case RANSEUR:
        case BROADSWORD:
        case ELVEN_BROADSWORD:
        case DARK_ELVEN_BROADSWORD:
        case RUNESWORD:
        case VOULGE:
            tmp += rnd(4);
            break;

        case ACID_VENOM:
            tmp += rnd(6);
            break;
        }
    }
    if (Is_weapon) {
        tmp += otmp->spe;

        /* adjust for various roles */
        if (magr == &gy.youmonst && Role_if(PM_DRUID)
            && otmp->material == WOOD
            && (levl[u.ux][u.uy].typ == GRASS
                || nexttotree(u.ux, u.uy))) {
            /* if a Druid's weapon is made of wood,
               and they are standing on grass or near
               a live tree, nature grants a dmg bonus */
            tmp += 2;
        }

        /* adjust for various materials */
        if (otmp->material == GLASS || otmp->material == GEMSTONE) {
            /* glass and gemstone are extremely sharp */
            if (objects[otyp].oc_dir & (PIERCE | SLASH))
                tmp += 3;
        } else if (otmp->material == GOLD || otmp->material == PLATINUM) {
            /* heavy metals, but softer than stone */
            if (objects[otyp].oc_dir & WHACK)
                tmp += 1;
        } else if (otmp->material == MITHRIL) {
            /* light and sharp */
            if (objects[otyp].oc_dir & (PIERCE | SLASH))
                tmp += 2;
        } else if (otmp->material == ADAMANTINE) {
            /* also light and very sharp */
            if (objects[otyp].oc_dir & (PIERCE | SLASH))
                tmp += 3;
        } else if (otmp->material == MINERAL) {
            /* stone is heavy */
            if (objects[otyp].oc_dir & (SLASH | WHACK))
                tmp += 2;
        } else if (otmp->material == PLASTIC || otmp->material == PAPER) {
            /* just terrible weapons all around */
            tmp -= 2;
        } else if (otmp->material == WOOD && !is_elven_weapon(otmp)) {
            /* poor at holding an edge */
            if (is_blade(otmp))
                tmp -= 1;
        } else if (otmp->material == STEEL || otmp->material == BRONZE) {
            /* steel/bronze has roughly the same density as iron, but
               both are stronger and can have a finer edge on bladed
               weapons */
            if (objects[otyp].oc_dir & (PIERCE | SLASH)) {
                tmp += 1;
                /* technically steel can have a finer edge than bronze,
                   but we don't want to get too crazy with damage bonuses
                   here. Sometimes steel blades will get an extra bite */
                if (!rn2(3) && otmp->material == STEEL)
                    tmp += 1;
            }
        }
    }

    /* higher quality weapons can dish out a bit more damage,
       inferior ones not so much */
    if (otmp->forged_qual == FQ_SUPERIOR)
        tmp += 1;
    else if (otmp->forged_qual == FQ_EXCEPTIONAL)
        tmp += 2;
    else if (otmp->forged_qual == FQ_INFERIOR)
        tmp -= 2;

    /* negative modifiers mustn't produce negative damage */
    if (tmp < 0)
        tmp = 0;

    if (otmp->material <= LEATHER
        && (thick_skinned(ptr)
            || has_barkskin(mdef) || has_stoneskin(mdef)
            || (mdef == &gy.youmonst && (Barkskin || Stoneskin))))
        /* thick-skinned or scaled creatures don't feel it */
        tmp = 0;
    if (noncorporeal(ptr) && !shade_glare(otmp))
        tmp = 0;

    /* "very heavy iron ball"; weight increase is in increments */
    if (otyp == HEAVY_IRON_BALL && tmp > 0) {
        int wt = (int) objects[HEAVY_IRON_BALL].oc_weight;

        if ((int) otmp->owt > wt) {
            wt = ((int) otmp->owt - wt) / WT_IRON_BALL_INCR;
            tmp += rnd(4 * wt);
            if (tmp > 25)
                tmp = 25; /* objects[].oc_wldam */
        }
    }

    /* Druids that #wildshape into one of their allowed forms
       enjoy a variable damage bonus */
    if (magr == &gy.youmonst && !uwep && druid_form)
        tmp += rn1(5, 2); /* 2-6 hp of damage */

    /* same with Vampires that #shapechange */
    if (magr == &gy.youmonst && !uwep && vampire_form)
        tmp += rn1(5, 2); /* 2-6 hp of damage */

    /* Put weapon vs. monster type damage bonuses in below: */
    {
        int bonus = 0;

        if (otmp->blessed && mon_hates_blessings(mdef))
            bonus += rnd(4);
        if (otmp->cursed && (is_angel(ptr) || is_aasimar(ptr)))
            bonus += rnd(4);
        if (magr == &gy.youmonst && otmp->cursed && Role_if(PM_INFIDEL)
            && (mon_aligntyp(mdef) == A_LAWFUL
                || mon_aligntyp(mdef) == A_NEUTRAL))
            bonus += rnd(2);
        /* anatomy of the defender: an axe bites into anything woody,
           a blunt weapon barely marks it, and the reverse holds for a
           skeletal frame that a blade slips between but a club shatters */
        if (is_axe(otmp)
            && (is_wooden(ptr) || is_plant(ptr) || has_barkskin(mdef)
                || (mdef == &gy.youmonst && Barkskin)))
            bonus += rnd(4);
        if ((objects[otyp].oc_dir & WHACK)
            && (is_wooden(ptr) || is_plant(ptr) || has_barkskin(mdef)
                || (mdef == &gy.youmonst && Barkskin)))
            bonus -= rnd(3) + 3;
        if ((objects[otyp].oc_dir & (PIERCE | SLASH))
            && (is_bone_monster(ptr) || has_stoneskin(mdef)
                || (mdef == &gy.youmonst && Stoneskin)))
            bonus -= rnd(5) + 3;
        if ((objects[otyp].oc_dir & WHACK) && is_bone_monster(ptr))
            bonus += rnd(4);
        if (mon_hates_material(mdef, otmp->material)
            && !(has_barkskin(mdef) || has_stoneskin(mdef))
            && !(mdef == &gy.youmonst && (Barkskin || Stoneskin)))
            bonus += rnd(sear_damage(otmp->material));
        if (artifact_light(otmp) && otmp->lamplit
            && (hates_light(ptr)
                || (mdef == &gy.youmonst
                    && maybe_polyd(is_drow(gy.youmonst.data),
                                   Race_if(PM_DROW)))))
            bonus += rnd(8);

        /* if the weapon is going to get a double damage bonus, adjust
           this bonus so that effectively it's added after the doubling */
        if (bonus > 1 && otmp->oartifact && spec_dbon(otmp, mdef, 25) >= 25)
            bonus = (bonus + 1) / 2;

        tmp += bonus;
    }

    if (tmp > 0) {
        int mac = (mdef != &cg.zeromonst) ? find_mac(mdef) : 10;

        /* It's debatable whether a rusted blunt instrument
           should do less damage than a pristine one, since
           it will hit with essentially the same impact, but
           there ought to some penalty for using damaged gear
           so always subtract erosion even for blunt weapons. */
        tmp -= greatest_erosion(otmp);

        /* Low AC subtracts damage, just as it does with players */
        if (mac < 0)
            tmp -= rnd(-mac);

        if (tmp < 1)
            tmp = 1;
    }

    /* the anatomy penalties above can drive this negative, and callers
       add the result to a running damage total */
    if (tmp < 0)
        tmp = 0;

    return  tmp;
}

/* Find an object that magr is wearing (or even magr's body itself) that has
 * a special damaging effect on mdef, and return the amount of bonus damage
 * done. The most damaging source has precedence; each source that causes
 * special damage makes its own roll for damage, and the highest roll will
 * be applied.
 *
 * hated_obj is set only on objects that cause material hatred; if none of
 * the applicable objects are of a hated material, it will not be set */
int
special_dmgval(
    struct monst *magr, /* attacker */
    struct monst *mdef, /* defender */
    long armask,        /* armor mask of all the slots that can be
                         * touching mdef */
    struct obj **hated_obj) /* ptr to offending object, can be NULL if
                             * not wanted */
{
    boolean youattack = (magr == &gy.youmonst);
    const int magr_material = monmaterial(monsndx(magr->data));
    int i;
    int bonus = 0;
    int tmpbonus = 0;
    boolean try_body = FALSE;
    struct obj *gloves    = which_armor(magr, W_ARMG),
               *helm      = which_armor(magr, W_ARMH),
               *shield    = which_armor(magr, W_ARMS),
               *boots     = which_armor(magr, W_ARMF),
               *armor     = which_armor(magr, W_ARM),
               *cloak     = which_armor(magr, W_ARMC),
               *shirt     = which_armor(magr, W_ARMU),
               *leftring  = (youattack ? uleft : which_armor(magr, W_RINGL)),
               *rightring = (youattack ? uright : which_armor(magr, W_RINGR));

    /* The order of armor slots in this array doesn't really matter because
     * we roll for everything that applies and take the highest damage */
    struct {
        long mask;
        struct obj **obj;
    } array[9] = {
        { W_ARMG, &gloves },
        { W_ARMH, &helm   },
        { W_ARMS, &shield },
        { W_ARMF, &boots  },
        { W_ARM,  &armor  },
        { W_ARMC, &cloak  },
        { W_ARMU, &shirt  },
        { W_RINGL, &leftring },
        { W_RINGR, &rightring }
    };

    if (hated_obj)
        *hated_obj = 0;

    /* Simple exclusions where we ignore a certain type of armor because it
     * is covered by some other equipment */
    if (gloves) {
        leftring = rightring = NULL;
    }
    if (cloak) {
        armor = shirt = NULL;
    }
    if (armor) {
        shirt = NULL;
    }

    /* Cases where we want to count magr's body: the caller indicates a
     * certain slot is making contact, and magr is not wearing anything in
     * that slot, so their body must be making contact.
     * Note: in the gloves case, rings don't prevent magr's body from
     * making contact */
    if (((armask & W_ARMG) && !gloves)
        || ((armask & W_ARMF) && !boots)
        || ((armask & W_ARMH) && !helm)
        || ((armask & (W_ARMC | W_ARM | W_ARMU))
            && !cloak && !armor && !shirt)) {
        try_body = TRUE;
    }

    if (try_body && mon_hates_material(mdef, magr_material)
        && (!(has_barkskin(mdef) || has_stoneskin(mdef)))
        && !(mdef == &gy.youmonst && (Barkskin || Stoneskin))) {
        bonus = sear_damage(magr_material);
        if (hated_obj)
            *hated_obj = (struct obj *) &cg.zeroobj;
    }

    for (i = 0; i < 9; ++i) {
        if (*array[i].obj && (armask & array[i].mask)) {
            tmpbonus = dmgval(magr, *array[i].obj, mdef);
            if (tmpbonus > bonus) {
                bonus = tmpbonus;
                if (hated_obj) {
                    /* Select hated_obj based on the maximum possible
                     * amount of damage that can be caused by its material,
                     * not the actual random amount of damage.
                     * E.g. if you manage to hit a monster that hates both
                     * silver and iron with a silver and an iron item at
                     * the same time, silver has a more severe effect
                     * dealing more damage so hated_obj should always be
                     * set to the silver one, so that searmsg will get
                     * called with the most appropriate message */
                    if (mon_hates_material(mdef, (*array[i].obj)->material)
                        && (*hated_obj == NULL
                            || (sear_damage((*array[i].obj)->material)
                                > sear_damage((*hated_obj)->material)))) {
                        *hated_obj = *array[i].obj;
                    }
                }
            }
        }
    }
    return bonus;
}

/* give a "silver <item> sears <target>" message (or similar for other
 * material); in addition to weapon hit, this is used for rings, boots for
 * kick, gloves for punch, or helm for headbutt.
 * This also handles the case where magr is made of a material that mdef
 * hates */
void
searmsg(
    struct monst *magr,
    struct monst *mdef,
    struct obj *obj, /* the offending item, or &zeroobj if magr's body */
    boolean minimal) /* print a shorter message leaving out obj details */
{
    char onamebuf[BUFSZ];
    char whose[BUFSZ];
    int mat;
    char *whom;
    boolean youattack = (magr == &gy.youmonst);
    boolean youdefend = (mdef == &gy.youmonst);
    boolean has_flesh = is_fleshy(mdef->data);

    if (!obj) {
        impossible("searmsg: nothing searing?");
        return;
    }

    if (!youdefend && DEADMONSTER(mdef))
        return;

    if (!youdefend && !canspotmon(mdef))
        return;

    if (!youdefend
        && (has_barkskin(mdef) || has_stoneskin(mdef)))
        return;

    if (youdefend
        && (Barkskin || Stoneskin))
        return;

    if (obj == &cg.zeroobj) {
        if (youattack) {
            Strcpy(whose, "your ");
        } else if (!magr) {
            impossible("searmsg: non-weapon attack with no aggressor?");
            return;
        } else {
            Strcpy(whose, s_suffix(mon_nam(magr)));
            Strcat(whose, " ");
        }
        mat = monmaterial(monsndx(magr->data));
        Sprintf(onamebuf, "%s touch", materialnm[mat]);
    } else {
        char *cxnameobj = cxname(obj);
        const char *matname;
        boolean alreadyin;
        int lead = 0;

        mat = obj->material;
        matname = materialnm[mat];
        alreadyin = (strstri(cxnameobj, matname) != NULL);

        /* Make it explicit to the player that this effect is from the
         * material, by prepending the material, but only if the object's
         * name doesn't already contain the material string somewhere.
         * (e.g. "sword" should turn into "iron sword", but "engraved
         * silver bell" shouldn't turn into "silver engraved silver
         * bell"). A leading "pair of " or "set of " stays in front, as
         * xname() places the material ("pair of iron gauntlets") */
        if (!alreadyin) {
            if (!strncmp(cxnameobj, "pair of ", 8))
                lead = 8;
            else if (!strncmp(cxnameobj, "set of ", 7))
                lead = 7;
            Snprintf(onamebuf, sizeof onamebuf, "%.*s%s %s", lead,
                     cxnameobj, matname, cxnameobj + lead);
        } else {
            Strcpy(onamebuf, cxnameobj);
        }
        shk_your(whose, obj);
    }

    if (minimal) {
        /* instead of "foo's obj", it will be "the [touch of] <material>;
         * whose becomes "the" in both cases */
        Strcpy(whose, "the ");
        if (mat == SILVER) { /* different formatting */
            Strcpy(onamebuf, "silver");
        } else {
            Sprintf(onamebuf, "touch of %s", materialnm[mat]);
        }
    }

    /* "Extra-minimal" case where we don't know what is doing the searing.
     * Note that this can assume it will be formatting some non-player
     * entity because it only applies when the player isn't involved */
    if (!youattack && !youdefend && !canseemon(mdef) && minimal) {
        if (mat == SILVER) {
            char defender[BUFSZ];

            if (has_flesh) {
                Sprintf(defender, "%s flesh", s_suffix(Monnam(mdef)));
            } else {
                Strcpy(defender, Monnam(mdef));
            }
            pline("%s is seared!", defender);
        } else {
            pline("%s recoils!", Monnam(mdef));
        }
        return;
    }

    whom = mon_nam(mdef);
    if (youdefend)
        Strcpy(whom, "you");
    if (youattack) {
        Strcpy(whose, "your ");
    } else if (!magr) { /* thrown objects */
        Strcpy(whose, "the ");
    } else {
        Strcpy(whose, s_suffix(mon_nam(magr)));
        Strcat(whose, " ");
    }

    if (mat == SILVER) { /* more dramatic effects than other materials */
        /* note: s_suffix returns a modifiable buffer */
        if (has_flesh)
            whom = strcat(s_suffix(whom), " flesh");
        pline("%s%s %s %s!", upstart(whose), onamebuf,
              vtense(onamebuf, "sear"), whom);
    } else {
        whom = upstart(whom);
        if (rn2(2)) {
            pline("%s recoil%s from %s%s!", whom, (youdefend ? "" : "s"),
                  whose, onamebuf);
        } else {
            pline("%s %s burned by %s%s!", whom, (youdefend ? "are" : "is"),
                  whose, onamebuf);
        }
    }
}

staticfn struct obj *oselect_recurse(struct monst *, struct obj *, int,
                                     struct obj *);
staticfn struct obj *oselect(struct monst *, int);
staticfn struct obj *find_artifact_recurse(struct monst *, struct obj *,
                                           struct monst *, boolean, boolean,
                                           struct obj *);
staticfn struct obj *find_silver_recurse(struct monst *, struct obj *,
                                         struct monst *, boolean, boolean);
staticfn struct obj *find_gem_recurse(struct monst *, struct obj *);
staticfn struct monst *mon_melee_target(struct monst *);
#define Oselect(x) \
    do {                                        \
        if ((otmp = oselect(mtmp, x)) != 0)     \
            return otmp;                        \
    } while (0)

/* recursive helper to find the best weapon of type 'type', looking
   into carried containers as well as open inventory; 'obest' carries
   the running best across the recursion, pass NULL on the initial
   call */
staticfn struct obj *
oselect_recurse(struct monst *mtmp, struct obj *start, int type,
                struct obj *obest)
{
    struct obj *otmp;

    for (otmp = start; otmp; otmp = otmp->nobj) {
        /* recurse into containers */
        if (Is_container(otmp) && otmp->cobj) {
            obest = oselect_recurse(mtmp, otmp->cobj, type, obest);
            continue;
        }
        if (otmp->otyp != type)
            continue;
        /* never select non-cockatrice corpses */
        if ((type == CORPSE || type == EGG)
            && !safe_touch_petrifies(otmp->corpsenm))
            continue;
        /* covers hated materials and untouchable artifacts */
        if (!can_touch_safely(mtmp, otmp))
            continue;
        /* priests avoid weapons that draw blood */
        if (is_priest(mtmp->data)
            && (otmp->oclass == WEAPON_CLASS || is_weptool(otmp))
            && (is_pierce(otmp) || is_slash(otmp)
                || (is_launcher(otmp) && otmp->otyp != SLING)))
            continue;
        if (!obest
            || dmgval(mtmp, otmp, &gy.youmonst)
                   > dmgval(mtmp, obest, &gy.youmonst))
            obest = otmp;
    }
    return obest;
}

/* Recursive helper to find the best artifact weapon a monster can
   wield, including those nested in containers. When more than one
   candidate exists, score_artifact_weapon() ranks them by expected
   per-hit damage against 'mdef' and the highest scorer wins.
   'best' carries the running best across recursion; pass NULL on
   the initial call */
staticfn struct obj *
find_artifact_recurse(struct monst *mtmp, struct obj *start,
                      struct monst *mdef, boolean strong,
                      boolean wearing_shield, struct obj *best)
{
    struct obj *otmp;
    int otmp_score;
    int best_score = best ? score_artifact_weapon(mtmp, best, mdef) : 0;

    for (otmp = start; otmp; otmp = otmp->nobj) {
        /* recurse into containers; refresh best_score afterwards
           because best may have been replaced inside the recursion */
        if (Is_container(otmp) && otmp->cobj) {
            best = find_artifact_recurse(mtmp, otmp->cobj, mdef,
                                         strong, wearing_shield, best);
            best_score = best ? score_artifact_weapon(mtmp, best, mdef)
                              : 0;
            continue;
        }
        if (otmp->oclass == WEAPON_CLASS && otmp->oartifact
            && touch_artifact(otmp, mtmp)
            && !mon_hates_material(mtmp, otmp->material)
            && ((strong && !wearing_shield)
                || !mon_bimanual(mtmp, otmp))) {
            otmp_score = score_artifact_weapon(mtmp, otmp, mdef);
            if (!best || otmp_score > best_score) {
                best = otmp;
                best_score = otmp_score;
            }
        }
    }
    return best;
}

/* recursive helper to find a silver weapon, including in containers */
staticfn struct obj *
find_silver_recurse(struct monst *mtmp, struct obj *start,
                    struct monst *mdef, boolean strong,
                    boolean wearing_shield)
{
    struct obj *otmp, *found;

    /* mtmp is invariant across the recursion; bail once if it
       can't touch silver at all rather than retesting per item */
    if (mon_hates_material(mtmp, SILVER))
        return (struct obj *) 0;

    for (otmp = start; otmp; otmp = otmp->nobj) {
        /* recurse into containers */
        if (Is_container(otmp) && otmp->cobj) {
            found = find_silver_recurse(mtmp, otmp->cobj, mdef, strong,
                                        wearing_shield);
            if (found)
                return found;
            continue;
        }
        if (otmp->oclass == WEAPON_CLASS && is_silver(otmp)
            && !(is_ammo(otmp) || is_missile(otmp))
            && (racial_vampire(mdef) || is_demon(raceptr(mdef)))
            && ((strong && !wearing_shield)
                || !mon_bimanual(mtmp, otmp)))
            return otmp;
    }
    return (struct obj *) 0;
}

/* recursive helper to find a gem for sling ammo, including in
   containers */
staticfn struct obj *
find_gem_recurse(struct monst *mtmp, struct obj *start)
{
    struct obj *otmp, *found;

    for (otmp = start; otmp; otmp = otmp->nobj) {
        /* recurse into containers */
        if (Is_container(otmp) && otmp->cobj) {
            found = find_gem_recurse(mtmp, otmp->cobj);
            if (found)
                return found;
            continue;
        }
        if (otmp->oclass == GEM_CLASS
            && (otmp->otyp != LOADSTONE || !otmp->cursed))
            return otmp;
    }
    return (struct obj *) 0;
}

staticfn struct obj *
oselect(struct monst *mtmp, int type)
{
    return oselect_recurse(mtmp, mtmp->minvent, type, (struct obj *) 0);
}

static NEARDATA const int rwep[] = {
    DWARVISH_SPEAR, DARK_ELVEN_SPEAR, ELVEN_SPEAR, SPEAR, ORCISH_SPEAR,
    JAVELIN, SHURIKEN, YA, ELVEN_ARROW, DARK_ELVEN_ARROW, ARROW,
    ORCISH_ARROW, CROSSBOW_BOLT, DARK_ELVEN_CROSSBOW_BOLT,
    DARK_ELVEN_DAGGER, ELVEN_DAGGER, DAGGER, ORCISH_DAGGER, KNIFE,
    SLING_BULLET, FLINT, ROCK, LOADSTONE, LUCKSTONE, DART, CREAM_PIE,
};

/* polearms */
static NEARDATA const int pwep[] = { HALBERD,       BARDICHE, SPETUM,
                                     BILL_GUISARME, VOULGE,   RANSEUR,
                                     GUISARME,      GLAIVE,   LUCERN_HAMMER,
                                     BEC_DE_CORBIN, FAUCHARD, PARTISAN,
                                     LANCE };

#define AKLYS_LIM (BOLT_LIM / 2)
/* throw-and-return weapons */
static NEARDATA const struct throw_and_return_weapon arwep[] = {
    /* { BOOMERANG, 5, 0 }, */
    { AKLYS, AKLYS_LIM * AKLYS_LIM, 1 },
};

const struct throw_and_return_weapon *
autoreturn_weapon(struct obj *otmp)
{
    int i;

    for (i = 0; i < SIZE(arwep); i++) {
        if (otmp->otyp == arwep[i].otyp)
            return &arwep[i];
    }
    return (struct throw_and_return_weapon *) 0;
}

boolean
would_prefer_rwep(struct monst *mtmp, struct obj *otmp)
{
    struct obj *wep = select_rwep(mtmp);
    struct monst *mdef;
    int i = 0;

    /* score against the pet's actual hostile target when available,
       otherwise default to the player; mirrors would_prefer_hwep */
    if (mtmp->mtame) {
        mdef = mon_melee_target(mtmp);
        if (!mdef)
            mdef = &gy.youmonst;
    } else {
        mdef = &gy.youmonst;
    }

    if (wep) {
        if (wep == otmp)
            return TRUE;
        if (wep->oartifact)
            return FALSE;
        if (mtmp->data->mlet == S_KOP && wep->otyp == CREAM_PIE)
            return FALSE;
        if (mtmp->data->mlet == S_KOP && otmp->otyp == CREAM_PIE)
            return TRUE;
        if (racial_throws_rocks(mtmp) && wep->otyp == BOULDER)
            return FALSE;
        if (racial_throws_rocks(mtmp) && otmp->otyp == BOULDER)
            return TRUE;
    }

    if (strongmonst(mtmp->data) && (mtmp->misc_worn_check & W_ARMS) == 0) {
        for (i = 0; i < SIZE(pwep); i++) {
            if (wep && wep->otyp == pwep[i]
                && !(otmp->otyp == pwep[i]
                     && dmgval(mtmp, otmp, mdef) > dmgval(mtmp, wep, mdef)))
                return FALSE;
            if (otmp->otyp == pwep[i])
                return TRUE;
        }
    }

    if (is_pole(otmp))
        return FALSE; /* if we get this far,
                         we failed the polearm strength check */

    for (i = 0; i < SIZE(rwep); i++) {
        if (wep && wep->otyp == rwep[i]
            && !(otmp->otyp == rwep[i]
                 && dmgval(mtmp, otmp, mdef) > dmgval(mtmp, wep, mdef)))
            return FALSE;
        if (otmp->otyp == rwep[i])
            return TRUE;
    }
    return FALSE;
}

/*
 * Check if monster needs ammo.
 * Returns:
 *   0 - doesn't need ranged weapons (has ammo or thrown weapons)
 *   1 - needs launcher ammo (has bow/crossbow/sling but no matching ammo)
 *   2 - needs thrown weapons (no ranged options at all)
 */
int
mon_needs_ammo(struct monst *mtmp)
{
    struct obj *otmp, *launcher = (struct obj *) 0;
    boolean has_thrown = FALSE, has_matching_ammo = FALSE;

    for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj) {
        if (is_launcher(otmp) && !launcher)
            launcher = otmp;
        if (is_missile(otmp) || throwing_weapon(otmp))
            has_thrown = TRUE;
    }

    if (launcher) {
        for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj) {
            if (ammo_and_launcher(otmp, launcher)) {
                has_matching_ammo = TRUE;
                break;
            }
        }
        if (!has_matching_ammo)
            return 1; /* needs launcher ammo */
    }

    if (!has_thrown && !launcher)
        return 2; /* needs thrown weapons */

    return 0;
}

/*
 * Check if monster wants this ammo/thrown item.
 * Pets opportunistically pick up any compatible ammo for their launchers.
 * Non-pets only seek ammo when they're completely out.
 */
boolean
mon_wants_ammo(struct monst *mtmp, struct obj *obj)
{
    struct obj *launcher;
    int need;

    /* don't seek ammo made of hated materials */
    if (mon_hates_material(mtmp, obj->material))
        return FALSE;

    /* pets opportunistically pick up any compatible ammo or thrown
       weapons, but not if there's a hostile monster adjacent - combat
       takes priority */
    if (mtmp->mtame) {
        struct monst *mon;
        coordxy x, y;

        /* check for adjacent hostiles - don't seek ammo during combat */
        for (x = mtmp->mx - 1; x <= mtmp->mx + 1; x++) {
            for (y = mtmp->my - 1; y <= mtmp->my + 1; y++) {
                if (!isok(x, y) || (x == mtmp->mx && y == mtmp->my))
                    continue;
                mon = m_at(x, y);
                if (mon && !mon->mpeaceful && !mon->mtame)
                    return FALSE; /* hostile adjacent, focus on combat */
            }
        }

        /* launcher ammo - need matching launcher */
        if (is_ammo(obj)) {
            for (launcher = mtmp->minvent; launcher;
                 launcher = launcher->nobj) {
                if (is_launcher(launcher) && ammo_and_launcher(obj, launcher))
                    return TRUE;
            }
        }
        /* thrown weapons - always useful */
        if (is_missile(obj) || throwing_weapon(obj))
            return TRUE;
    }

    /* non-pets only seek ammo when they need it */
    need = mon_needs_ammo(mtmp);
    if (need) {
        /* hostile monsters shouldn't seek ammo if player is adjacent */
        if (!mtmp->mpeaceful && mdistu(mtmp) <= 2)
            return FALSE; /* player adjacent, focus on combat */

        if (need == 1) {
            /* needs launcher ammo - check if obj matches any launcher */
            for (launcher = mtmp->minvent; launcher;
                 launcher = launcher->nobj) {
                if (is_launcher(launcher) && ammo_and_launcher(obj, launcher))
                    return TRUE;
            }
        } else if (need == 2) {
            /* needs thrown weapons - accept missiles and throwing weapons */
            if (is_missile(obj) || throwing_weapon(obj))
                return TRUE;
        }
    }
    return FALSE;
}

/* select a ranged weapon for the monster */
struct obj *
select_rwep(struct monst *mtmp)
{
    struct obj *otmp;
    struct obj *mwep;
    struct obj *tmpprop = &hands_obj;
    boolean mweponly;
    int i;

    char mlet = mtmp->data->mlet;

    gp.propellor = &hands_obj;
    Oselect(EGG);      /* cockatrice egg */
    if (mlet == S_KOP) /* pies are first choice for Kops */
        Oselect(CREAM_PIE);
    if (racial_throws_rocks(mtmp)) /* ...boulders for giants */
        Oselect(BOULDER);

    /* Select polearms first; they do more damage and aren't expendable.
       But don't pick one if monster's weapon is welded, because then
       we'd never have a chance to throw non-wielding missiles. */
    /* the distance limit matches the monster polearm range limit
       (MON_POLE_DIST); selecting a polearm from any farther away
       would have the monster treat it as if it had the range of a
       bow, repeatedly wielding it without being able to use it */
    mwep = MON_WEP(mtmp);
    /* NO_WEAPON_WANTED means we already tried to wield and failed */
    mweponly = (mwelded(mwep) && mtmp->data != &mons[PM_INFIDEL]
                && mtmp->weapon_check == NO_WEAPON_WANTED);
    if (dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) <= 5
        && couldsee(mtmp->mx, mtmp->my)) {
        if (is_art(mwep, ART_SNICKERSNEE)) {
            gp.propellor = mwep;
            return mwep;
        }

        for (i = 0; i < SIZE(pwep); i++) {
            /* Only strong monsters can wield big (esp. long) weapons.
             * Big weapon is basically the same as bimanual.
             * All monsters can wield the remaining weapons.
             * Hated instance materials are rejected by oselect().
             */
            if ((strongmonst(mtmp->data)
                 && (mtmp->misc_worn_check & W_ARMS) == 0)
                || !objects[pwep[i]].oc_bimanual) {
                if ((otmp = oselect(mtmp, pwep[i])) != 0
                    && (otmp == mwep || !mweponly)) {
                    gp.propellor = otmp; /* force the monster to wield it */
                    return otmp;
                }
            }
        }
    }
    /* Next, try to select a throw-and-return weapon, since they are
     * also not as expendable. Again, don't pick one if monster's
     * weapon is welded.
     */
    for (i = 0; i < SIZE(arwep); i++) {
        const struct throw_and_return_weapon *arw = &arwep[i];
        /* while a monster aims at another monster (thrwmm() sets
           gm.mtarget), the tether's reach is measured to that target
           rather than to where the monster thinks the hero is, so a
           farther target gets the other missiles below */
        coordxy tx = gm.mtarget ? gm.mtarget->mx : mtmp->mux,
                ty = gm.mtarget ? gm.mtarget->my : mtmp->muy;

        if (!mindless(mtmp->data) && !is_animal(mtmp->data) && !mweponly
            && dist2(mtmp->mx, mtmp->my, tx, ty) <= arw->range
            && couldsee(mtmp->mx, mtmp->my)) {
            if ((((mtmp->misc_worn_check & W_ARMS) == 0)
                 || !objects[arw->otyp].oc_bimanual)
                && !mon_hates_material(mtmp,
                                       objects[arw->otyp].oc_material)) {
                /* like the rwep[] pick below, never throw a cursed
                   weapon-in-hand; the hero can't throw a welded one */
                if ((otmp = oselect(mtmp, arw->otyp)) != 0
                    && (otmp == mwep || !mweponly)
                    && !(otmp == mwep && mwelded(otmp)
                         && mtmp->data != &mons[PM_INFIDEL])) {
                    gp.propellor = otmp; /* force the monster to wield it */
                    return otmp;
                }
            }
        }
    }

    /*
     * other than the specific cases above, always select the
     * most potent ranged weapon to hand.
     */
    for (i = 0; i < SIZE(rwep); i++) {
        int prop;

        /* shooting gems from slings; this goes just before the darts */
        /* (shooting rocks is already handled via the rwep[] ordering) */
        if (rwep[i] == DART && !likes_gems(mtmp->data)
            && m_carrying(mtmp, SLING)) { /* propellor */
            otmp = find_gem_recurse(mtmp, mtmp->minvent);
            if (otmp) {
                gp.propellor = m_carrying(mtmp, SLING);
                return otmp;
            }
        }

        /* KMH -- This belongs here so darts will work */
        gp.propellor = &hands_obj;

        prop = objects[rwep[i]].oc_skill;
        if (prop < 0) {
            switch (-prop) {
            case P_BOW:
                gp.propellor = oselect(mtmp, YUMI);
                if (!gp.propellor)
                    gp.propellor = oselect(mtmp, ELVEN_BOW);
                if (!gp.propellor)
                    gp.propellor = oselect(mtmp, DARK_ELVEN_BOW);
                if (!gp.propellor)
                    gp.propellor = oselect(mtmp, BOW);
                if (!gp.propellor)
                    gp.propellor = oselect(mtmp, ORCISH_BOW);
                break;
            case P_SLING:
                gp.propellor = oselect(mtmp, SLING);
                break;
            case P_CROSSBOW:
                gp.propellor = oselect(mtmp, CROSSBOW);
                if (!gp.propellor)
                    gp.propellor = oselect(mtmp, DARK_ELVEN_HAND_CROSSBOW);
            }
            if (tmpprop == &hands_obj)
                tmpprop = gp.propellor;
            if ((otmp = MON_WEP(mtmp)) && mwelded(otmp)
                && mtmp->data != &mons[PM_INFIDEL] && otmp != gp.propellor
                && mtmp->weapon_check == NO_WEAPON_WANTED)
                gp.propellor = 0;
        }
        /* propellor = obj, propellor to use
         * propellor = &hands_obj, doesn't need a propellor
         * propellor = 0, needed one and didn't have one
         */
        if (gp.propellor != 0) {
            /* Note: cannot use m_carrying for loadstones, since it will
             * always select the first object of a type, and maybe the
             * monster is carrying two but only the first is unthrowable.
             */
            if (rwep[i] != LOADSTONE) {
                /* Don't throw a cursed weapon-in-hand or an artifact */
                if ((otmp = oselect(mtmp, rwep[i])) && !otmp->oartifact
                    && !(otmp == MON_WEP(mtmp) && mwelded(otmp)
                         && mtmp->data != &mons[PM_INFIDEL]))
                    return otmp;
            } else
                for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj) {
                    if (otmp->otyp == LOADSTONE && !otmp->cursed)
                        return otmp;
                }
        }
    }

    /* failure; remember a launcher found along the way so the monster
       can still wield it (and then go looking for ammo) */
    if (tmpprop)
        gp.propellor = tmpprop;
    return (struct obj *) 0;
}

/* is 'obj' a type of weapon that any monster knows how to throw? */
boolean
monmightthrowwep(struct obj *obj)
{
    short idx;

    for (idx = 0; idx < SIZE(rwep); ++idx)
        if (obj->otyp == rwep[idx])
            return TRUE;
    return FALSE;
}

/* Weapons in order of preference, primarily based on effective
   damage output (with some exceptions). Polearms are intentionally
   excluded - they are tracked in pwep[] and selected for ranged
   thrust via select_rwep()/thrwmu(), not for melee */
static const NEARDATA short hwep[] = {
    CORPSE, /* cockatrice corpse */
    TSURUGI, RUNESWORD, TRIPLE_HEADED_FLAIL, DWARVISH_MATTOCK,
    TWO_HANDED_SWORD, BATTLE_AXE, DARK_ELVEN_HEAVY_MACE, HEAVY_MACE,
    KATANA, DARK_ELVEN_LONG_SWORD, ELVEN_LONG_SWORD, UNICORN_HORN,
    CRYSKNIFE, TRIDENT, LONG_SWORD, ORCISH_LONG_SWORD, ATLATL,
    HEAVY_WAR_HAMMER, DWARVISH_BEARDED_AXE, DARK_ELVEN_BROADSWORD,
    ELVEN_BROADSWORD, BROADSWORD, SCIMITAR, ORCISH_SCIMITAR, SABER,
    MORNING_STAR, ORCISH_MORNING_STAR, DARK_ELVEN_SHORT_SWORD,
    ELVEN_SHORT_SWORD, DWARVISH_SHORT_SWORD, SHORT_SWORD,
    ORCISH_SHORT_SWORD, ROD, DARK_ELVEN_MACE, MACE, AXE,
    DWARVISH_SPEAR, DARK_ELVEN_SPEAR, ELVEN_SPEAR, SPEAR,
    ORCISH_SPEAR, FLAIL, BULLWHIP, QUARTERSTAFF, JAVELIN, AKLYS,
    CLUB, PICK_AXE, RUBBER_HOSE, WAR_HAMMER, DARK_ELVEN_DAGGER,
    ELVEN_DAGGER, DAGGER, ORCISH_DAGGER, ATHAME, SCALPEL, KNIFE,
    STILETTO, WORM_TOOTH
};

/* pick the most attractive monster target this monster could reach
   with a melee weapon; used to score candidate weapons against the
   pet's actual foe instead of the player */
staticfn struct monst *
mon_melee_target(struct monst *mtmp)
{
    struct monst *bestmon = (struct monst *) 0, *candidate;
    long bestscore = -1000000L, currscore;
    int dx, dy;

    for (dx = -1; dx <= 1; dx++) {
        for (dy = -1; dy <= 1; dy++) {
            if (!dx && !dy)
                continue;
            /* find_targ() traverses a straight line from (mx, my)
               toward direction (dx, dy) up to range 7, and returns
               the first monster 'candidate' that could be attacked;
               invisible monsters are skipped if the attacker lacks
               see-invisible */
            candidate = find_targ(mtmp, dx, dy, 7);
            if (!candidate)
                continue;
            /* is that candidate truly a legal melee foe? */
            if (!acceptable_pet_target(mtmp, candidate, FALSE))
                continue;
            /* score how attractive that candidate is
               (lower = less attractive) */
            currscore = score_targ(mtmp, candidate);
            if (currscore > bestscore) {
                bestscore = currscore;
                bestmon = candidate;
            }
        }
    }

    return bestmon;
}

boolean
would_prefer_hwep(struct monst *mtmp, struct obj *otmp)
{
    struct obj *wep;
    struct obj *brac;
    struct monst *mdef; /* target under certain scenarios */
    int i = 0;
    boolean strong = strongmonst(mtmp->data);
    boolean wearing_shield = (mtmp->misc_worn_check & W_ARMS) != 0;

    if (!otmp)
        return FALSE;

    wep = select_hwep(mtmp);

    if (mtmp->mtame) {
        mdef = mon_melee_target(mtmp);
        if (!mdef)
            mdef = &gy.youmonst;
    } else {
        mdef = &gy.youmonst;
    }

    if (wep) {
        if (wep == otmp)
            return TRUE;
        if (wep->oartifact)
            return FALSE;
        if (racial_giant(mtmp) && wep->otyp == CLUB)
            return FALSE;
        if (racial_giant(mtmp) && otmp->otyp == CLUB)
            return TRUE;
    }

    /* bracers don't really count as shields */
    if (wearing_shield) {
        for (brac = mtmp->minvent; brac; brac = brac->nobj) {
            if (brac->owornmask && is_bracer(brac))
                wearing_shield = FALSE;
        }
    }

    for (i = 0; i < SIZE(hwep); i++) {
        if (hwep[i] == CORPSE && !(mtmp->misc_worn_check & W_ARMG)
            && !(resists_ston(mtmp) || defended(mtmp, AD_STON)))
            continue;
        if (wep && wep->otyp == hwep[i])
            break;
        if (otmp->otyp == hwep[i]
            && dmgval(mtmp, otmp, mdef) > (wep ? dmgval(mtmp, wep, mdef) : 0)
            && ((strong && !wearing_shield)
                || !objects[otmp->otyp].oc_bimanual))
            return TRUE;
    }
    return FALSE;
}

/* select a hand to hand weapon for the monster */
struct obj *
select_hwep(struct monst *mtmp)
{
    struct obj *otmp;
    struct monst *mdef; /* target under certain scenarios */
    int i;
    boolean strong = strongmonst(mtmp->data);
    boolean wearing_shield = (mtmp->misc_worn_check & W_ARMS) != 0;

    if (mtmp->mtame) {
        mdef = mon_melee_target(mtmp);
        if (!mdef)
            mdef = &gy.youmonst;
    } else {
        mdef = &gy.youmonst;
    }

    /* bracers don't really count as shields */
    if (wearing_shield) {
        for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj) {
            if (otmp->owornmask && is_bracer(otmp))
                wearing_shield = FALSE;
        }
    }

    /* prefer artifacts to everything else (including in containers);
       when several artifacts are present, pick the highest-scoring
       one against the chosen target */
    otmp = find_artifact_recurse(mtmp, mtmp->minvent, mdef, strong,
                                 wearing_shield, (struct obj *) 0);
    if (otmp)
        return otmp;

    /* prefer silver weapons when fighting demons or vampires
       (including in containers) */
    otmp = find_silver_recurse(mtmp, mtmp->minvent, mdef, strong,
                               wearing_shield);
    if (otmp)
        return otmp;

    if (racial_giant(mtmp)) /* giants just love to use clubs */
        Oselect(CLUB);
    else if (mtmp->data == &mons[PM_BALROG] && uwep)
        Oselect(BULLWHIP);

    /* only strong monsters can wield big (esp. long) weapons */
    /* big weapon is basically the same as bimanual */
    /* all monsters can wield the remaining weapons */
    for (i = 0; i < SIZE(hwep); i++) {
        if (hwep[i] == CORPSE && !(mtmp->misc_worn_check & W_ARMG)
            && !(resists_ston(mtmp) || defended(mtmp, AD_STON)))
            continue;
        if (((strong && !wearing_shield) || !objects[hwep[i]].oc_bimanual
             || is_giant(mtmp->data))
            && !mon_hates_material(mtmp, objects[hwep[i]].oc_material))
            Oselect(hwep[i]);
    }

    /* failure */
    return (struct obj *) 0;
}

/* can this monster fight with two weapons? */
boolean
can_dual_wield(struct monst *mon)
{
    struct obj *shield, *mwep;

    if (!could_twoweap(mon->data))
        return FALSE;
    if (mindless(mon->data) || is_animal(mon->data))
        return FALSE;

    mwep = MON_WEP(mon);
    if (!mwep)
        return FALSE;
    /* can't dual-wield with a two-handed primary */
    if (mon_bimanual(mon, mwep))
        return FALSE;

    /* non-bracer shields block dual-wield */
    shield = which_armor(mon, W_ARMS);
    if (shield && !is_bracer(shield))
        return FALSE;

    return TRUE;
}

/* Select a secondary hand-to-hand weapon for dual-wielding.
   Similar to select_hwep() but skips the primary weapon, requires
   one-handed, and uses simpler scoring */
struct obj *
select_offhand_hwep(struct monst *mon)
{
    struct obj *otmp, *best = (struct obj *) 0;
    struct monst *mdef;
    int best_val = 0, val;

    /* score against the pet's hostile melee target when available;
       otherwise default to the player */
    if (mon->mtame) {
        mdef = mon_melee_target(mon);
        if (!mdef)
            mdef = &gy.youmonst;
    } else {
        mdef = &gy.youmonst;
    }

    for (otmp = mon->minvent; otmp; otmp = otmp->nobj) {
        /* skip primary weapon */
        if (otmp == MON_WEP(mon))
            continue;
        /* skip worn/wielded items (but allow current mw2) */
        if (otmp->owornmask && otmp != MON_WEP2(mon))
            continue;
        /* must be a weapon or weapon-tool */
        if (otmp->oclass != WEAPON_CLASS && !is_weptool(otmp))
            continue;
        /* must be one-handed (giants one-hand non-launchers) */
        if (mon_bimanual(mon, otmp))
            continue;
        /* skip launchers (even one-handed ones) */
        if (is_launcher(otmp))
            continue;
        /* skip ammunition (arrows, bolts, sling ammo) */
        if (is_ammo(otmp))
            continue;
        /* skip thrown missiles (shuriken, darts, boomerangs); these
           are meant to be thrown, not held as a melee off-hand weapon */
        if (is_missile(otmp))
            continue;
        /* skip hated materials */
        if (mon_hates_material(mon, otmp->material))
            continue;
        /* skip artifacts that would reject the wielder */
        if (otmp->oartifact && !touch_artifact(otmp, mon))
            continue;

        /* score: artifact bonus + enchantment + base damage; pick the
           damage column that matches the defender's size, matching
           dmgval()'s r_bigmonst() selection */
        val = r_bigmonst(mdef) ? objects[otmp->otyp].oc_wldam
                               : objects[otmp->otyp].oc_wsdam;
        if (otmp->oartifact)
            val += 10;
        if (otmp->spe > 0)
            val += otmp->spe;

        if (val > best_val) {
            best_val = val;
            best = otmp;
        }
    }
    return best;
}

/* Have a monster wield a secondary weapon for dual-wielding.
   Returns 1 if the monster took time to do it, 0 if not */
int
mon_wield_offhand(struct monst *mon)
{
    struct obj *obj;

    if (!can_dual_wield(mon))
        return 0;

    obj = select_offhand_hwep(mon);
    if (!obj)
        return 0;

    /* already wielding this one */
    if (obj == MON_WEP2(mon))
        return 0;

    /* clear old secondary */
    if (MON_WEP2(mon))
        setmnotwielded2(mon, MON_WEP2(mon));

    mon->mw2 = obj;

    if (canseemon(mon))
        pline_mon(mon, "%s wields %s in %s other %s.", Monnam(mon),
                  doname(obj), mhis(mon), mbodypart(mon, HAND));

    /* handle artifact light */
    if (artifact_light(obj) && !obj->lamplit) {
        begin_burn(obj, FALSE);
        if (canseemon(mon))
            pline("%s %s in %s %s!", Tobjnam(obj, "shine"),
                  arti_light_description(obj), s_suffix(mon_nam(mon)),
                  mbodypart(mon, HAND));
        else if (cansee(mon->mx, mon->my))
            pline("Light begins shining %s.",
                  (mdistu(mon) <= 5 * 5) ? "nearby" : "in the distance");
    }

    obj->owornmask = W_SWAPWEP;
    return 1;
}

/* Called after polymorphing a monster, robbing it, etc....  Monsters
 * otherwise never unwield stuff on their own.  Might print message.
 */
void
possibly_unwield(struct monst *mon, boolean polyspot)
{
    struct obj *obj, *mw_tmp;

    if (!(mw_tmp = MON_WEP(mon)))
        goto check_mw2; /* still need to validate secondary weapon */
    for (obj = mon->minvent; obj; obj = obj->nobj)
        if (obj == mw_tmp)
            break;
    if (!obj) { /* The weapon was stolen or destroyed */
        MON_NOWEP(mon);
        mon->weapon_check = NEED_WEAPON;
        goto check_mw2;
    }
    if (!attacktype(mon->data, AT_WEAP)) {
        setmnotwielded(mon, mw_tmp);
        mon->weapon_check = NO_WEAPON_WANTED;
        /* a pet ordered not to drop items keeps the weapon
           in inventory */
        if (!pet_wont_drop(mon)) {
            /* if we're going to call distant_name(), do so before
               extract_self */
            if (cansee(mon->mx, mon->my)) {
                pline_mon(mon, "%s drops %s.", Monnam(mon),
                          distant_name(obj, doname));
                newsym(mon->mx, mon->my);
            }
            obj_extract_self(obj);
            /* might be dropping object into water or lava */
            if (!flooreffects(obj, mon->mx, mon->my, "drop")) {
                if (polyspot)
                    bypass_obj(obj);
                place_object(obj, mon->mx, mon->my);
                stackobj(obj);
            }
        }
        goto check_mw2;
    }
    /* The remaining case where there is a change is where a monster
     * is polymorphed into a stronger/weaker monster with a different
     * choice of weapons.  This has no parallel for players.  It can
     * be handled by waiting until mon_wield_item is actually called.
     * Though the monster still wields the wrong weapon until then,
     * this is OK since the player can't see it.  (FIXME: Not okay since
     * probing can reveal it.)
     * Note that if there is no change, setting the check to NEED_WEAPON
     * is harmless.
     * Possible problem: big monster with big cursed weapon gets
     * polymorphed into little monster.  But it's not quite clear how to
     * handle this anyway....
     */
    if (!(mwelded(mw_tmp) && mon->data != &mons[PM_INFIDEL]
          && mon->weapon_check == NO_WEAPON_WANTED))
        mon->weapon_check = NEED_WEAPON;

 check_mw2:
    /* also check secondary weapon */
    if ((mw_tmp = MON_WEP2(mon)) != 0) {
        for (obj = mon->minvent; obj; obj = obj->nobj)
            if (obj == mw_tmp)
                break;
        if (!obj) {
            /* secondary weapon was stolen or destroyed */
            MON_NOWEP2(mon);
        } else if (!can_dual_wield(mon)) {
            /* can no longer dual-wield (form change, etc.) */
            setmnotwielded2(mon, mw_tmp);
            /* a pet ordered not to drop items keeps the weapon
               in inventory */
            if (!pet_wont_drop(mon)) {
                if (cansee(mon->mx, mon->my)) {
                    pline_mon(mon, "%s drops %s.", Monnam(mon),
                              distant_name(obj, doname));
                    newsym(mon->mx, mon->my);
                }
                obj_extract_self(obj);
                if (!flooreffects(obj, mon->mx, mon->my, "drop")) {
                    if (polyspot)
                        bypass_obj(obj);
                    place_object(obj, mon->mx, mon->my);
                    stackobj(obj);
                }
            }
        }
    }
    return;
}

/* Let a monster try to wield a weapon, based on mon->weapon_check.
 * Returns 1 if the monster took time to do it, 0 if it did not.
 */
int
mon_wield_item(struct monst *mon)
{
    struct obj *obj;
    boolean exclaim = TRUE; /* assume mon is planning to attack */

    /* This case actually should never happen */
    if (mon->weapon_check == NO_WEAPON_WANTED)
        return 0;
    switch (mon->weapon_check) {
    case NEED_HTH_WEAPON:
        obj = select_hwep(mon);
        break;
    case NEED_RANGED_WEAPON:
        (void) select_rwep(mon);
        obj = gp.propellor;
        break;
    case NEED_PICK_AXE:
        obj = m_carrying(mon, PICK_AXE);
        /* KMH -- allow other picks */
        if (!obj && !which_armor(mon, W_ARMS))
            obj = m_carrying(mon, DWARVISH_MATTOCK);
        exclaim = FALSE; /* mon is just planning to dig */
        break;
    case NEED_AXE:
        /* currently, only 2 types of axe */
        obj = m_carrying(mon, BATTLE_AXE);
        if (!obj || which_armor(mon, W_ARMS))
            obj = m_carrying(mon, AXE);
        exclaim = FALSE;
        break;
    case NEED_PICK_OR_AXE:
        /* prefer pick for fewer switches on most levels */
        obj = m_carrying(mon, DWARVISH_MATTOCK);
        if (!obj)
            obj = m_carrying(mon, BATTLE_AXE);
        if (!obj || which_armor(mon, W_ARMS)) {
            obj = m_carrying(mon, PICK_AXE);
            if (!obj)
                obj = m_carrying(mon, AXE);
        }
        exclaim = FALSE;
        break;
    default:
        impossible("weapon_check %d for %s?", mon->weapon_check,
                   mon_nam(mon));
        return 0;
    }
    if (obj && obj != &hands_obj) {
        struct obj *mw_tmp = MON_WEP(mon);

        /* if weapon is in a container, extract it first (uses turn) */
        if (obj->where == OBJ_CONTAINED) {
            int result = mon_container_extract(mon, obj);

            if (result != 1) { /* 0=failed, 2=used turn for extraction */
                /* reset to NEED_WEAPON so combat code re-evaluates
                   next turn */
                mon->weapon_check = NEED_WEAPON;
                return (result == 2) ? 1 : 0;
            }
            /* result==1 means already in inventory (shouldn't happen) */
        }

        if (mw_tmp && mw_tmp->otyp == obj->otyp) {
            /* already wielding it */
            mon->weapon_check = NEED_WEAPON;
            /* still try to wield secondary weapon if capable */
            if (can_dual_wield(mon) && !MON_WEP2(mon))
                return mon_wield_offhand(mon);
            return 0;
        }
        /* Actually, this isn't necessary--as soon as the monster
         * wields the weapon, the weapon welds itself, so the monster
         * can know it's cursed and needn't even bother trying.
         * Still....
         */
        if (mw_tmp && mwelded(mw_tmp) && mon->data != &mons[PM_INFIDEL]) {
            if (canseemon(mon)) {
                char welded_buf[BUFSZ];
                const char *mon_hand = mbodypart(mon, HAND);

                if (mon_bimanual(mon, mw_tmp))
                    mon_hand = makeplural(mon_hand);
                Sprintf(welded_buf, "%s welded to %s %s",
                        otense(mw_tmp, "are"), mhis(mon), mon_hand);

                if (obj->otyp == PICK_AXE) {
                    pline("Since %s weapon%s %s,", s_suffix(mon_nam(mon)),
                          plur(mw_tmp->quan), welded_buf);
                    pline("%s cannot wield that %s.", mon_nam(mon),
                          xname(obj));
                } else {
                    pline_mon(mon, "%s tries to wield %s.", Monnam(mon), doname(obj));
                    pline("%s %s!", Yname2(mw_tmp), welded_buf);
                }
                mw_tmp->bknown = 1;
            }
            mon->weapon_check = NO_WEAPON_WANTED;
            return 1;
        }
        mon->mw = obj; /* wield obj */
        setmnotwielded(mon, mw_tmp);
        /* if promoting the secondary weapon to primary, clear mw2 in
           place; skip setmnotwielded2 because its end_burn would
           extinguish a lit artifact only for the begin_burn below to
           immediately re-fire it, producing a duplicate "stops
           shining"/"shines" message pair. The W_SWAPWEP bit is
           cleared by the owornmask = W_WEP assignment further down */
        if (obj == MON_WEP2(mon))
            MON_NOWEP2(mon);
        mon->weapon_check = NEED_WEAPON;
        if (canseemon(mon)) {
            boolean newly_welded;
            const struct throw_and_return_weapon *arw;

            pline_mon(mon, "%s wields %s%c",
                      Monnam(mon), doname(obj),
                      exclaim ? '!' : '.');
            if ((arw = autoreturn_weapon(obj)) != 0 && arw->tethered != 0)
                pline_mon(mon, "%s secures the tether on %s.", Monnam(mon),
                          the(xname(obj)));

            /* 3.6.3: mwelded() predicate expects the object to have its
               W_WEP bit set in owormmask, but the pline here and for
               artifact_light don't want that because they'd have '(weapon
               in hand/claw)' appended; so we set it for the mwelded test
               and then clear it, until finally setting it for good below */
            obj->owornmask |= W_WEP;
            newly_welded = (mwelded(obj) && mon->data != &mons[PM_INFIDEL]);
            obj->owornmask &= ~W_WEP;
            if (newly_welded) {
                const char *mon_hand = mbodypart(mon, HAND);

                if (mon_bimanual(mon, obj))
                    mon_hand = makeplural(mon_hand);
                pline("%s %s to %s %s!", Tobjnam(obj, "weld"),
                      is_plural(obj) ? "themselves" : "itself",
                      s_suffix(mon_nam(mon)), mon_hand);
                obj->bknown = 1;
            }
        }
        if (artifact_light(obj) && !obj->lamplit) {
            begin_burn(obj, FALSE);
            if (canseemon(mon))
                pline("%s %s in %s %s!", Tobjnam(obj, "shine"),
                      arti_light_description(obj), s_suffix(mon_nam(mon)),
                      mbodypart(mon, HAND));
            /* 3.6.3: artifact might be getting wielded by invisible monst */
            else if (cansee(mon->mx, mon->my))
                pline("Light begins shining %s.",
                      (mdistu(mon) <= 5 * 5) ? "nearby" : "in the distance");
        }
        obj->owornmask = W_WEP;

        /* if primary is two-handed, clear any secondary */
        if (mon_bimanual(mon, obj) && MON_WEP2(mon))
            setmnotwielded2(mon, MON_WEP2(mon));
        /* try to wield secondary weapon if capable */
        if (can_dual_wield(mon) && !MON_WEP2(mon))
            (void) mon_wield_offhand(mon);

        return 1;
    }
    mon->weapon_check = NEED_WEAPON;
    return 0;
}

/* force monster to stop wielding current weapon, if any */
void
mwepgone(struct monst *mon)
{
    struct obj *mwep = MON_WEP(mon);

    if (mwep) {
        setmnotwielded(mon, mwep);
        mon->weapon_check = NEED_WEAPON;
    }
}

/* force monster to stop wielding secondary weapon, if any */
void
mwep2gone(struct monst *mon)
{
    struct obj *mwep2 = MON_WEP2(mon);

    if (mwep2) {
        setmnotwielded2(mon, mwep2);
        mon->weapon_check = NEED_WEAPON;
    }
}

/* attack bonus for strength & dexterity */
int
abon(void)
{
    int sbon;
    int str = ACURR(A_STR), dex = ACURR(A_DEX);

    if (Upolyd)
        return (adj_lev(&mons[u.umonnum]) - 3);

    /* this used to be '<= 18/50' for bonus of 1 but got changed to '< 18/50'
       so that '18/50' gives a bonus of 2; gnome and orc player characters
       have max Str of 18/50 and giving an extra bonus at that break point
       provides an incentive for them to max out that characteristic */
    if (str < 6)
        sbon = -2;
    else if (str < 8)
        sbon = -1;
    else if (str < 17)
        sbon = 0;
    else if (str < STR18(50))
        sbon = 1; /* up to 18/49 */
    else if (str < STR18(100))
        sbon = 2;
    else
        sbon = 3;

    /* Game tuning kludge: make it a bit easier for a low level character to
     * hit */
    sbon += (u.ulevel < 3) ? 1 : 0;

    if (dex < 4)
        return (sbon - 3);
    else if (dex < 6)
        return (sbon - 2);
    else if (dex < 8)
        return (sbon - 1);
    else if (dex < 14)
        return sbon;
    else
        return (sbon + dex - 14);
}

/* damage bonus for strength; hitobj is the object the blow is struck
   with, or NULL when bare-handed */
int
dbon(struct obj *hitobj)
{
    int str = ACURR(A_STR);
    int bonus = 0;

    if (Upolyd)
        return 0;

    if (str < 6)
        bonus = -2;
    else if (str < 9)
        bonus = -1;
    else if (str < 16)
        bonus = 0;
    else if (str <= 18)
        bonus = 1;
    else if (str <= STR18(50))
        bonus = 2; /* up to 18/50 */
    else if (str <= STR18(75))
        bonus = 3; /* up to 18/75 */
    else if (str < STR18(100))
        bonus = 4; /* up to 18/99 */
    else if (str == STR18(100))
        bonus = 5; /* 18/00 only */
    else
        bonus = 6;

    /* damage bonus is multiplied 1.5 times for a blow struck with a
       weapon made for two hands, even by a giant one-handing it
       (they're heavy); the rounding leaves a penalty as it is. A
       thrown or kicked object or a shield bash is not swung with it
       and gets the plain bonus. No u.twoweap test: the primary
       weapon's destruction can clear the flag before hitum() swings
       the off-hand it captured earlier */
    if (hitobj && (hitobj == uwep || hitobj == uswapwep)
        && (hitobj->oclass == WEAPON_CLASS
            || hitobj->oclass == TOOL_CLASS)
        && objects[hitobj->otyp].oc_bimanual)
        bonus = ((bonus * 3) + 1) / 2;

    return bonus;
}

/* called when wet_a_towel() or dry_a_towel() is changing a towel's wetness */
staticfn void
finish_towel_change(struct obj *obj, int newspe)
{
    /* towel wetness is always between 0 (dry) and 7, inclusive */
    newspe = min(newspe, 7);
    obj->spe = max(newspe, 0);

    /* if hero is wielding this towel, don't give "you begin bashing with
       your [wet] towel" message if it's wet, do give one if it's dry */
    if (obj == uwep)
        gu.unweapon = !is_wet_towel(obj);

    /* description might change: "towel" vs "moist towel" vs "wet towel" */
    if (carried(obj))
        update_inventory();
}

/* increase a towel's wetness */
void
wet_a_towel(
    struct obj *obj,
    int amt, /* positive: new val; negative: increment by -amt; zero: no-op */
    boolean verbose)
{
    int newspe = (amt <= 0) ? obj->spe - amt : amt;

    /* new state is only reported if it's an increase */
    if (newspe > obj->spe) {
        if (verbose) {
            const char *wetness = (newspe < 3)
                                     ? (!obj->spe ? "damp" : "damper")
                                     : (!obj->spe ? "wet" : "wetter");

            if (carried(obj))
                pline("%s gets %s.", Yobjnam2(obj, (const char *) 0),
                      wetness);
            else if (mcarried(obj) && canseemon(obj->ocarry))
                pline("%s %s gets %s.", s_suffix(Monnam(obj->ocarry)),
                      xname(obj), wetness);
        }
    }

    if (newspe != obj->spe)
        finish_towel_change(obj, newspe);
}

/* decrease a towel's wetness; unlike when wetting, 0 is not a no-op */
void
dry_a_towel(
    struct obj *obj,
    int amt, /* positive or zero: new value; negative: decrement by abs(amt) */
    boolean verbose)
{
    int newspe = (amt < 0) ? obj->spe + amt : amt;

    /* new state is only reported if it's a decrease */
    if (newspe < obj->spe) {
        if (verbose) {
            if (carried(obj))
                pline("%s dries%s.", Yobjnam2(obj, (const char *) 0),
                      !newspe ? " out" : "");
            else if (mcarried(obj) && canseemon(obj->ocarry))
                pline("%s %s dries%s.", s_suffix(Monnam(obj->ocarry)),
                      xname(obj), !newspe ? " out" : "");
        }
    }

    if (newspe != obj->spe)
        finish_towel_change(obj, newspe);
}

/* copy the skill level name into the given buffer */
char *
skill_level_name(int skill, char *buf)
{
    const char *ptr;

    switch (P_SKILL(skill)) {
    case P_UNSKILLED:
        ptr = "Unskilled";
        break;
    case P_BASIC:
        ptr = "Basic";
        break;
    case P_SKILLED:
        ptr = "Skilled";
        break;
    case P_EXPERT:
        ptr = "Expert";
        break;
    /* these are for unarmed combat/martial arts only */
    case P_MASTER:
        ptr = "Master";
        break;
    case P_GRAND_MASTER:
        ptr = "Grand Master";
        break;
    default:
        ptr = "Unknown";
        break;
    }
    Strcpy(buf, ptr);
    return buf;
}

const char *
skill_name(int skill)
{
    return P_NAME(skill);
}

/* return the # of slots required to advance the skill */
staticfn int
slots_required(int skill)
{
    int tmp = P_SKILL(skill);

    /* The more difficult the training, the more slots it takes.
     *  unskilled -> basic      1
     *  basic -> skilled        2
     *  skilled -> expert       3
     */
    if (skill <= P_LAST_WEAPON || skill == P_TWO_WEAPON_COMBAT)
        return tmp;

    /* Fewer slots used up for unarmed or martial.
     *  unskilled -> basic      1
     *  basic -> skilled        1
     *  skilled -> expert       2
     *  expert -> master        2
     *  master -> grand master  3
     */
    return (tmp + 1) / 2;
}

/* return true if this skill can be advanced */
boolean
can_advance(int skill, boolean speedy)
{
    if (P_RESTRICTED(skill)
        || P_SKILL(skill) >= P_MAX_SKILL(skill)
        || u.skills_advanced >= P_SKILL_LIMIT)
        return FALSE;

    if (wizard && speedy)
        return TRUE;

    return (boolean) ((int) P_ADVANCE(skill)
                      >= practice_needed_to_advance(P_SKILL(skill))
                      && u.weapon_slots >= slots_required(skill));
}

/* return true if this skill could be advanced if more slots were available */
staticfn boolean
could_advance(int skill)
{
    if (P_RESTRICTED(skill)
        || P_SKILL(skill) >= P_MAX_SKILL(skill)
        || u.skills_advanced >= P_SKILL_LIMIT)
        return FALSE;

    return (boolean) ((int) P_ADVANCE(skill)
                      >= practice_needed_to_advance(P_SKILL(skill)));
}

/* return true if this skill has reached its maximum and there's been enough
   practice to become eligible for the next step if that had been possible */
staticfn boolean
peaked_skill(int skill)
{
    if (P_RESTRICTED(skill))
        return FALSE;

    return (boolean) (P_SKILL(skill) >= P_MAX_SKILL(skill)
                      && ((int) P_ADVANCE(skill)
                          >= practice_needed_to_advance(P_SKILL(skill))));
}

staticfn void
skill_advance(int skill)
{
    u.weapon_slots -= slots_required(skill);
    P_SKILL(skill)++;
    u.skill_record[u.skills_advanced++] = skill;
    /* subtly change the advance message to indicate no more advancement */
    You("are now %s skilled in %s.",
        P_SKILL(skill) >= P_MAX_SKILL(skill) ? "most" : "more",
        P_NAME(skill));

    /* wizards discover spellbook IDs depending on spell 'school' skill limits;
       this allows them to successfully write books for unknown spells without
       the Luck bias they used to have over other roles */
    if (skill >= P_FIRST_SPELL && skill <= P_LAST_SPELL)
        skill_based_spellbook_id();
}

static const struct skill_range {
    short first, last;
    const char *name;
} skill_ranges[] = {
    { P_FIRST_H_TO_H, P_LAST_H_TO_H, "Fighting Skills" },
    { P_FIRST_WEAPON, P_LAST_WEAPON, "Weapon Skills" },
    { P_FIRST_SPELL, P_LAST_SPELL, "Spellcasting Skills" },
};

/* write a list of skills onto the given menu

   if selectable is set, give selection letters for skills that can be
   advanced and leave room for them on skills that can't be advanced */
void
add_skills_to_menu(winid win, boolean selectable, boolean speedy)
{
    int pass, i, len, longest;
    anything any;
    char buf[BUFSZ], sklnambuf[BUFSZ];
    const char *prefix;
    int clr = NO_COLOR;
    boolean dumping = program_state.gameover;

    /* Find the longest skill name. */
    for (longest = 0, i = 0; i < P_NUM_SKILLS; i++) {
        if (P_RESTRICTED(i))
            continue;
        if ((len = Strlen(P_NAME(i))) > longest)
            longest = len;
    }

    /* List the skills, making ones that could be advanced selectable if
       selectable is set.  List the miscellaneous skills first.  Possible
       future enhancement: list spell skills before weapon skills for
       spellcaster roles. */
    for (pass = 0; pass < SIZE(skill_ranges); pass++)
        for (i = skill_ranges[pass].first; i <= skill_ranges[pass].last;
             i++) {
            /* Print headings for skill types */
            any = cg.zeroany;
            if (i == skill_ranges[pass].first) {
                if (dumping)
                    /* html dumplogs: add_menu_heading does not permit
                       formatting */
                    putstr(win, ATR_SUBHEAD, skill_ranges[pass].name);
                else
                    add_menu_heading(win, skill_ranges[pass].name);
            }

            if (P_RESTRICTED(i))
                continue;
            /*
             * Sigh, this assumes a monospaced font unless
             * iflags.menu_tab_sep is set in which case it puts
             * tabs between columns.
             * The 12 is the longest skill level name.
             * The "    " is room for a selection letter and dash, "a - ".
             */
            if (!selectable)
                prefix = "";
            else if (can_advance(i, speedy))
                prefix = ""; /* will be preceded by menu choice */
            else if (could_advance(i))
                prefix = "  * ";
            else if (peaked_skill(i))
                prefix = "  # ";
            else
                prefix = "    ";
            (void) skill_level_name(i, sklnambuf);
            if (wizard) {
                if (!iflags.menu_tab_sep)
                    Snprintf(buf, sizeof buf,
                             " %s%-*s %-12s %5d(%4d)", prefix,
                             longest, P_NAME(i), sklnambuf, P_ADVANCE(i),
                             practice_needed_to_advance(P_SKILL(i)));
                else
                    Snprintf(buf, sizeof buf,
                             " %s%s\t%s\t%5d(%4d)", prefix, P_NAME(i),
                             sklnambuf, P_ADVANCE(i),
                             practice_needed_to_advance(P_SKILL(i)));
            } else {
                if (!iflags.menu_tab_sep)
                    Snprintf(buf, sizeof buf,
                             " %s %-*s [%s]", prefix, longest,
                             P_NAME(i), sklnambuf);
                else
                    Snprintf(buf, sizeof buf,
                             " %s%s\t[%s]", prefix, P_NAME(i),
                             sklnambuf);
            }
            any.a_int = selectable && can_advance(i, speedy) ? i + 1 : 0;
            add_menu(win, &nul_glyphinfo, &any, 0, 0,
                     ATR_NONE, clr, buf, MENU_ITEMFLAGS_NONE);
        }
}

/* Displays a skill list for dumplog purposes. */
void
show_skills(void)
{
    winid win;
    menu_item *selected;

    putstr(0, ATR_HEADING, "Skills:");
    win = create_nhwindow(NHW_MENU);
    start_menu(win, MENU_BEHAVE_STANDARD);
    add_skills_to_menu(win, FALSE, FALSE);
    end_menu(win, "");
    nhUse(select_menu(win, PICK_NONE, &selected));
    destroy_nhwindow(win);
}

/*
 * The `#enhance' extended command.  What we _really_ would like is
 * to keep being able to pick things to advance until we couldn't any
 * more.  This is currently not possible -- the menu code has no way
 * to call us back for instant action.  Even if it did, we would also need
 * to be able to update the menu since selecting one item could make
 * others unselectable.
 */
int
enhance_weapon_skill(void)
{
    int i, n, to_advance, eventually_advance, maxxed_cnt;
    char buf[BUFSZ];
    menu_item *selected;
    winid win;
    boolean speedy = FALSE;

    /* player knows about #enhance, don't show tip anymore */
    svc.context.tips |= (1 << TIP_ENHANCE);

    if (wizard && y_n("Advance skills without practice?") == 'y')
        speedy = TRUE;

    do {
        /* count advanceable skills */
        to_advance = eventually_advance = maxxed_cnt = 0;
        for (i = 0; i < P_NUM_SKILLS; i++) {
            if (P_RESTRICTED(i))
                continue;
            if (can_advance(i, speedy))
                to_advance++;
            else if (could_advance(i))
                eventually_advance++;
            else if (peaked_skill(i))
                maxxed_cnt++;
        }

        win = create_nhwindow(NHW_MENU);
        start_menu(win, MENU_BEHAVE_STANDARD);

        /* start with a legend if any entries will be annotated
           with "*" or "#" below */
        if (eventually_advance > 0 || maxxed_cnt > 0) {
            if (eventually_advance > 0) {
                Sprintf(buf, "(Skill%s flagged by \"*\" may be enhanced %s.)",
                        plur(eventually_advance),
                        (u.ulevel < MAXULEV)
                            ? "when you're more experienced"
                            : "if skill slots become available");
                add_menu_str(win, buf);
            }
            if (maxxed_cnt > 0) {
                Sprintf(buf,
                 "(Skill%s flagged by \"#\" cannot be enhanced any further.)",
                        plur(maxxed_cnt));
                add_menu_str(win, buf);
            }
            add_menu_str(win, "");
        }

        add_skills_to_menu(
            win, to_advance + eventually_advance + maxxed_cnt > 0, speedy);

        Strcpy(buf, (to_advance > 0) ? "Pick a skill to advance:"
                                     : "Current skills:");
        if (wizard && !speedy)
            Sprintf(eos(buf), "  (%d slot%s available)", u.weapon_slots,
                    plur(u.weapon_slots));
        end_menu(win, buf);
        n = select_menu(win, to_advance ? PICK_ONE : PICK_NONE, &selected);
        destroy_nhwindow(win);
        if (n > 0) {
            n = selected[0].item.a_int - 1; /* get item selected */
            free((genericptr_t) selected);
            skill_advance(n);
            /* check for more skills able to advance; if so, then... */
            for (n = i = 0; i < P_NUM_SKILLS; i++) {
                if (can_advance(i, speedy)) {
                    if (!speedy)
                        You_feel("you could be more dangerous!");
                    n++;
                    break;
                }
            }
        }
    } while (speedy && n > 0);
    return ECMD_OK;
}

/*
 * Change from restricted to unrestricted, allowing P_BASIC as max.  This
 * function may be called with with P_NONE.  Used in pray.c as well as below.
 */
void
unrestrict_weapon_skill(int skill)
{
    /* From SporkHack (modified): Cavepersons are good at what they
       know how to use, but not much on advanced fencing or combat
       tactics, and clerics shouldn't have edged weapons at all.
       Neither role gets a restricted weapon skill opened unless it
       is a blunt one (club, mace, flail, hammer, quarterstaff) */
    if (Role_if(PM_CAVE_DWELLER) || Role_if(PM_CLERIC)) {
        if (skill >= P_DAGGER && skill <= P_SABER)
            return;
        if (skill >= P_POLEARMS && skill <= P_UNICORN_HORN)
            return;
    }

    if (skill < P_NUM_SKILLS && P_RESTRICTED(skill)) {
        P_SKILL(skill) = P_UNSKILLED;
        P_MAX_SKILL(skill) = P_BASIC;
        P_ADVANCE(skill) = 0;
    }
}

void
use_skill(int skill, int degree)
{
    boolean advance_before;

    if (skill != P_NONE && !P_RESTRICTED(skill)) {
        advance_before = can_advance(skill, FALSE);
        P_ADVANCE(skill) += degree;
        if (!advance_before && can_advance(skill, FALSE))
            give_may_advance_msg(skill);
    }
}

void
add_weapon_skill(int n) /* number of slots to gain; normally one */
{
    int i, before, after;

    for (i = 0, before = 0; i < P_NUM_SKILLS; i++)
        if (can_advance(i, FALSE))
            before++;
    u.weapon_slots += n;
    for (i = 0, after = 0; i < P_NUM_SKILLS; i++)
        if (can_advance(i, FALSE))
            after++;
    if (before < after)
        give_may_advance_msg(P_NONE);
}

void
lose_weapon_skill(int n) /* number of slots to lose; normally one */
{
    int skill;

    while (--n >= 0) {
        /* deduct first from unused slots then from last placed one, if any */
        if (u.weapon_slots) {
            u.weapon_slots--;
        } else if (u.skills_advanced) {
            skill = u.skill_record[--u.skills_advanced];
            if (P_SKILL(skill) <= P_UNSKILLED)
                panic("lose_weapon_skill (%d)", skill);
            P_SKILL(skill)--; /* drop skill one level */
            /* Lost skill might have taken more than one slot; refund rest. */
            u.weapon_slots = slots_required(skill) - 1;
            /* It might now be possible to advance some other pending
               skill by using the refunded slots, but giving a message
               to that effect would seem pretty confusing.... */
        }
    }
}

void
drain_weapon_skill(int n) /* number of skills to drain */
{
    int skill;
    int i;
    int curradv;
    int prevadv;
    int tmpskills[P_NUM_SKILLS];

    (void) memset((genericptr_t) tmpskills, 0, sizeof(tmpskills));

    while (--n >= 0) {
        if (u.skills_advanced) {
            /* Pick a random skill, deleting it from the list. */
            i = rn2(u.skills_advanced);
            skill = u.skill_record[i];
            tmpskills[skill] = 1;
            for (; i < u.skills_advanced - 1; i++) {
                u.skill_record[i] = u.skill_record[i + 1];
            }
            u.skills_advanced--;
            if (P_SKILL(skill) <= P_UNSKILLED)
                panic("drain_weapon_skill (%d)", skill);
            P_SKILL(skill)--;   /* drop skill one level */
            /* refund slots used for skill */
            u.weapon_slots += slots_required(skill);
            /* drain skill training to a value appropriate for new level */
            curradv = practice_needed_to_advance(P_SKILL(skill));
            prevadv = practice_needed_to_advance(P_SKILL(skill) - 1);
            if (P_ADVANCE(skill) >= curradv)
                P_ADVANCE(skill) = prevadv + rn2(curradv - prevadv);
        }
    }

    for (skill = 0; skill < P_NUM_SKILLS; skill++)
        if (tmpskills[skill]) {
            You("forget %syour training in %s.",
                P_SKILL(skill) >= P_BASIC ? "some of " : "", P_NAME(skill));
        }
}

/* blessed scroll of amnesia: undo every recorded skill advance and refund
   its slots; practice (P_ADVANCE) is kept so the hero can re-#enhance */
void
reset_weapon_skills(void)
{
    int skill;

    while (u.skills_advanced) {
        skill = u.skill_record[--u.skills_advanced];
        if (P_SKILL(skill) <= P_UNSKILLED)
            panic("reset_weapon_skills (%d)", skill);
        P_SKILL(skill)--; /* drop skill one level */
        /* refund the slots that advance used */
        u.weapon_slots += slots_required(skill);
    }
}

int
weapon_type(struct obj *obj)
{
    /* KMH -- now uses the object table */
    int type;

    if (!obj)
        return P_BARE_HANDED_COMBAT; /* Not using a weapon */
    /* a Convict can wield their punishment ball to some effect */
    if (obj->otyp == HEAVY_IRON_BALL && Role_if(PM_CONVICT))
        return objects[obj->otyp].oc_skill;
    if (obj->oclass != WEAPON_CLASS && obj->oclass != TOOL_CLASS
        && obj->oclass != GEM_CLASS)
        return P_NONE; /* Not a weapon, weapon-tool, or ammo */
    type = objects[obj->otyp].oc_skill;
    return (type < 0) ? -type : type;
}

int
uwep_skill_type(void)
{
    if (u.twoweap)
        return P_TWO_WEAPON_COMBAT;
    return weapon_type(uwep);
}

/*
 * Effective skill level for a weapon, accounting for artifacts that
 * confer skill (Glamdring reads as at least Expert in long sword for
 * a Wizard wielding it). Callers that need the raw skill for
 * training or advancement keep using P_SKILL() directly
 */
int
weapon_skill_level(struct obj *weapon)
{
    int type = weapon_type(weapon);
    int skill;

    if (type == P_NONE || type == P_BARE_HANDED_COMBAT
        || type == P_TWO_WEAPON_COMBAT)
        return P_SKILL(type);
    skill = P_SKILL(type);
    if (weapon && weapon == uwep && weapon->oartifact == ART_GLAMDRING
        && type == P_LONG_SWORD && Role_if(PM_WIZARD)
        && skill < P_EXPERT)
        skill = P_EXPERT;
    return skill;
}

/*
 * Return hit bonus/penalty based on skill of weapon.
 * weapon can be null, meaning bare-handed combat.
 * Treat restricted weapons as unskilled.
 */
int
weapon_hit_bonus(struct obj *weapon)
{
    int type, wep_type, skill, bonus = 0;
    unsigned maxweight = 0;
    static const char bad_skill[] = "weapon_hit_bonus: bad skill %d";

    wep_type = weapon_type(weapon);
    /* use two weapon skill only if attacking with one of the wielded weapons
     */
    type = (u.twoweap && (weapon == uwep || weapon == uswapwep))
               ? P_TWO_WEAPON_COMBAT
               : wep_type;
    if (type == P_NONE) {
        bonus = 0;
    } else if (type <= P_LAST_WEAPON) {
        switch (weapon_skill_level(weapon)) {
        default:
            impossible(bad_skill, P_SKILL(type));
            FALLTHROUGH;
            /* FALLTHRU */
        case P_ISRESTRICTED:
        case P_UNSKILLED:
            bonus = -4;
            break;
        case P_BASIC:
            bonus = 0;
            break;
        case P_SKILLED:
            bonus = 3;
            break;
        case P_EXPERT:
            bonus = 5;
            break;
        }
    } else if (type == P_TWO_WEAPON_COMBAT) {
        skill = P_SKILL(P_TWO_WEAPON_COMBAT);
        if (P_SKILL(wep_type) < skill)
            skill = P_SKILL(wep_type);
        switch (skill) {
        default:
            impossible(bad_skill, skill);
            FALLTHROUGH;
            /* FALLTHRU */
        case P_ISRESTRICTED:
        case P_UNSKILLED:
            bonus = -9;
            break;
        case P_BASIC:
            bonus = -5;
            break;
        case P_SKILLED:
            bonus = -3;
            break;
        case P_EXPERT:
            bonus = 0; /* if you're an expert, there shouldn't be a penalty */
            break;
        }
        /* Heavy things are hard to use in your offhand unless you're
           very good at what you're doing, or are very strong (see below).
           do_attack() carries a twin of this ladder for its feedback;
           keep the two in sync */
        switch (P_SKILL(P_TWO_WEAPON_COMBAT)) {
        default:
            impossible(bad_skill, P_SKILL(P_TWO_WEAPON_COMBAT));
            FALLTHROUGH;
            /* FALLTHRU */
        case P_ISRESTRICTED:
        case P_UNSKILLED:
            maxweight = 20; /* can use tridents/javelins, crysknives,
                               unicorn horns or anything lighter */
            break;
        case P_BASIC:
            maxweight = 30; /* can use short swords/spears or a mace */
            break;
        case P_SKILLED:
            maxweight = 40; /* can use sabers/long swords */
            break;
        case P_EXPERT:
            maxweight = 70; /* expert level can offhand any one-handed
                               weapon */
            break;
        }

        /* basically no restrictions if you're a giant, or have giant
           strength */
        if (ACURR(A_STR) == STR19(25)
            || maybe_polyd(is_giant(gy.youmonst.data), Race_if(PM_GIANT)))
            maxweight = 200;

        if (uswapwep && uswapwep->owt > maxweight) {
            /* feedback handled in do_attack() */
            bonus = -30;
        }
    } else if (type == P_BARE_HANDED_COMBAT) {
        /*
         *        b.h. m.a. giant b.h. m.a.
         * unskl:  +1  n/a         +3  n/a
         * basic:  +1   +3         +3   +4
         * skild:  +2   +4         +4   +5
         * exprt:  +2   +5         +4   +6
         * mastr:  +3   +8         +5   +9
         * grand:  +3  +11         +5  +12
         */
        bonus = P_SKILL(type);
        bonus = max(bonus, P_UNSKILLED) - 1; /* unskilled => 0 */
        bonus = ((bonus + 2) * (martial_bonus() ? 2 : 1)) / 2;
        /* extra boost for master/GM */
        if (bonus > 5)
            bonus += ((bonus - 5) * 2);

        /* giants have naturally powerful fists */
        if (Race_if(PM_GIANT))
            bonus += martial_bonus() ? 1 : 2;
    }

    /* KMH -- It's harder to hit while you are riding */
    if (u.usteed) {
        switch (P_SKILL(P_RIDING)) {
        case P_ISRESTRICTED:
        case P_UNSKILLED:
            bonus -= 2;
            break;
        case P_BASIC:
            bonus -= 1;
            break;
        case P_SKILLED:
            break;
        case P_EXPERT:
            break;
        }
        if (u.twoweap)
            bonus -= 2;
    }

    /* Priests using edged weapons is frowned upon by their deity */
    if (uwep && Role_if(PM_CLERIC)
        && (uwep->oclass == WEAPON_CLASS || is_weptool(uwep))
        && (is_pierce(uwep) || is_slash(uwep) || is_ammo(uwep))) {
        /* feedback handled in do_attack() */
        bonus = -40;
    }
    return bonus;
}

/*
 * Return damage bonus/penalty based on skill of weapon.
 * weapon can be null, meaning bare-handed combat.
 * Treat restricted weapons as unskilled.
 */
int
weapon_dam_bonus(struct obj *weapon)
{
    int type, wep_type, skill, bonus = 0;

    wep_type = weapon_type(weapon);
    /* use two weapon skill only if attacking with one of the wielded weapons
     */
    type = (u.twoweap && (weapon == uwep || weapon == uswapwep))
               ? P_TWO_WEAPON_COMBAT
               : wep_type;
    if (type == P_NONE) {
        bonus = 0;
    } else if (type <= P_LAST_WEAPON) {
        switch (weapon_skill_level(weapon)) {
        default:
            impossible("weapon_dam_bonus: bad skill %d", P_SKILL(type));
            FALLTHROUGH;
        /* FALLTHRU */
        case P_ISRESTRICTED:
        case P_UNSKILLED:
            bonus = -2;
            break;
        case P_BASIC:
            bonus = 0;
            break;
        case P_SKILLED:
            bonus = 1;
            break;
        case P_EXPERT:
            bonus = 2;
            break;
        }
    } else if (type == P_TWO_WEAPON_COMBAT) {
        skill = P_SKILL(P_TWO_WEAPON_COMBAT);
        if (P_SKILL(wep_type) < skill)
            skill = P_SKILL(wep_type);
        switch (skill) {
        default:
        case P_ISRESTRICTED:
        case P_UNSKILLED:
            bonus = -3;
            break;
        case P_BASIC:
            bonus = -1;
            break;
        case P_SKILLED:
            bonus = 0;
            break;
        case P_EXPERT:
            bonus = 1;
            break;
        }
    } else if (type == P_BARE_HANDED_COMBAT) {
        /*
         *        b.h. m.a.
         * unskl:   0  n/a
         * basic:  +1   +3
         * skild:  +1   +4
         * exprt:  +2   +6
         * mastr:  +2   +7
         * grand:  +3   +9
         */
        bonus = P_SKILL(type);
        bonus = max(bonus, P_UNSKILLED) - 1; /* unskilled => 0 */
        bonus = ((bonus + 1) * (martial_bonus() ? 3 : 1)) / 2;
    }

    /* KMH -- Riding or being a centaur gives some thrusting damage.
       The centaur half is inert: skill_init() restricts riding for
       centaurs and tortles, so they can only ever reach the no-bonus
       arms of the switch */
    if ((u.usteed || Race_if(PM_CENTAUR)) && type != P_TWO_WEAPON_COMBAT) {
        switch (P_SKILL(P_RIDING)) {
        case P_ISRESTRICTED:
        case P_UNSKILLED:
            break;
        case P_BASIC:
            break;
        case P_SKILLED:
            bonus += 1;
            break;
        case P_EXPERT:
            bonus += 2;
            break;
        }
    }

    return bonus;
}

/*
 * Initialize weapon skill array for the game.  Start by setting all
 * skills to restricted, then set the skill for every weapon the
 * hero is holding, finally reading the given array that sets
 * maximums.
 */
void
skill_init(const struct def_skill *class_skill)
{
    struct obj *obj;
    int skmax, skill;

    /* initialize skill array; by default, everything is restricted */
    for (skill = 0; skill < P_NUM_SKILLS; skill++) {
        P_SKILL(skill) = P_ISRESTRICTED;
        P_MAX_SKILL(skill) = P_ISRESTRICTED;
        P_ADVANCE(skill) = 0;
    }

    /* Set skill for all weapons in inventory to be basic */
    for (obj = gi.invent; obj; obj = obj->nobj) {
        /* don't give skill just because of carried ammo, wait until
           we see the relevant launcher (prevents an archeologist's
           touchstone from inadvertently providing skill in sling) */
        if (is_ammo(obj))
            continue;
        /* a dwarf's racial pick-axe from u_init_race() is not part of
           the role's kit and gives no starting skill; the Archeologist's
           own pick-axe still counts */
        if (obj->otyp == PICK_AXE && Race_if(PM_DWARF)
            && !Role_if(PM_ARCHEOLOGIST))
            continue;

        skill = weapon_type(obj);
        if (skill != P_NONE)
            P_SKILL(skill) = P_BASIC;
    }

    /* Wizards no longer start with a weapon in inventory,
       so make sure quarterstaff is set to basic skill */
    if (Role_if(PM_WIZARD))
        P_SKILL(P_QUARTERSTAFF) = P_BASIC;

    /* set skills for magic; draugr cannot cast, so give their
       spellcasting roles no starting spell skill */
    if (Role_if(PM_HEALER)
        || (Role_if(PM_MONK) && !Race_if(PM_DRAUGR))) {
        P_SKILL(P_HEALING_SPELL) = P_BASIC;
    } else if (Role_if(PM_DRUID)) {
        P_SKILL(P_EVOCATION_SPELL) = P_BASIC;
    } else if (Role_if(PM_INFIDEL) && !Race_if(PM_DRAUGR)) {
        P_SKILL(P_ATTACK_SPELL) = P_BASIC;
    } else if (Role_if(PM_CLERIC)) {
        P_SKILL(P_CLERIC_SPELL) = P_BASIC;
    } else if (Role_if(PM_WIZARD)) {
        P_SKILL(P_ATTACK_SPELL) = P_BASIC;
        P_SKILL(P_ENCHANTMENT_SPELL) = P_BASIC;
    }

    /* set skill for thievery */
    if (Role_if(PM_ROGUE) || Role_if(PM_CONVICT))
        P_SKILL(P_THIEVERY) = P_BASIC;

    /* walk through array to set skill maximums */
    for (; class_skill->skill != P_NONE; class_skill++) {
        skmax = class_skill->skmax;
        skill = class_skill->skill;

        P_MAX_SKILL(skill) = skmax;
        if (P_SKILL(skill) == P_ISRESTRICTED) /* skill pre-set */
            P_SKILL(skill) = P_UNSKILLED;
    }

    /* High potential fighters already know how to use their hands. */
    if (P_MAX_SKILL(P_BARE_HANDED_COMBAT) > P_EXPERT)
        P_SKILL(P_BARE_HANDED_COMBAT) = P_BASIC;

    /* Roles that start with a horse know how to ride it */
    if (gu.urole.petnum == PM_PONY
        || gu.urole.petnum == PM_SKELETAL_PONY)
        P_SKILL(P_RIDING) = P_BASIC;

    /* Centaurs/tortles can never ride anything */
    if (Race_if(PM_CENTAUR) || Race_if(PM_TORTLE))
        P_SKILL(P_RIDING) = P_NONE;

    /* Roles that can reach expert or master in shield skill already
       have a basic understanding of how to use them */
    if (Role_if(PM_KNIGHT) || Role_if(PM_VALKYRIE))
        P_SKILL(P_SHIELD) = P_BASIC;

    /* Healers, Druids and Knights start with basic pet handling
       skill */
    if (Role_if(PM_HEALER) || Role_if(PM_DRUID)
        || Role_if(PM_KNIGHT))
        P_SKILL(P_PET_HANDLING) = P_BASIC;

    /* As a Tortle, if their role lists trident as a trainable skill,
       raise the max proficiency level by one. If non-cleric, add
       trident skill, and allow training up to basic */
    if (Race_if(PM_TORTLE)) {
        if (Role_if(PM_BARBARIAN) || Role_if(PM_MONK)) {
            P_MAX_SKILL(P_TRIDENT) = P_EXPERT;
        } else if (Role_if(PM_HEALER) || Role_if(PM_TOURIST)) {
            P_MAX_SKILL(P_TRIDENT) = P_SKILLED;
        } else if (!Role_if(PM_CLERIC)) {
            unrestrict_weapon_skill(P_TRIDENT);
            P_MAX_SKILL(P_TRIDENT) = P_BASIC;
        }
    }

    /* Illithids have innate telekinesis - start untrained, must
       practice */
    if (Race_if(PM_ILLITHID)) {
        unrestrict_weapon_skill(P_TELEKINESIS);
        P_SKILL(P_TELEKINESIS) = P_UNSKILLED;
        P_MAX_SKILL(P_TELEKINESIS) = P_MASTER;
    }

    /*
     * Make sure we haven't missed setting the max on a skill
     * & set advance
     */
    for (skill = 0; skill < P_NUM_SKILLS; skill++) {
        if (!P_RESTRICTED(skill)) {
            if (P_MAX_SKILL(skill) < P_SKILL(skill)) {
                impossible("skill_init: curr > max: %s", P_NAME(skill));
                P_MAX_SKILL(skill) = P_SKILL(skill);
            }
            P_ADVANCE(skill) = practice_needed_to_advance(P_SKILL(skill) - 1);
        }
    }

    /* each role has a special spell; allow at least basic for its type
       (despite the function name, this works for spell skills too) */
    unrestrict_weapon_skill(spell_skilltype(gu.urole.spelspec));

    if (!u.uroleplay.pauper) /* paupers lack advanced access to books */
        skill_based_spellbook_id();
}

void
setmnotwielded(struct monst *mon, struct obj *obj)
{
    if (!obj)
        return;
    if (artifact_light(obj) && obj->lamplit) {
        end_burn(obj, FALSE);
        if (canseemon(mon))
            pline("%s in %s %s %s shining.", The(xname(obj)),
                  s_suffix(mon_nam(mon)), mbodypart(mon, HAND),
                  otense(obj, "stop"));
    }
    if (MON_WEP(mon) == obj)
        MON_NOWEP(mon);
    obj->owornmask &= ~W_WEP;
}

/* off-hand counterpart of setmnotwielded() */
void
setmnotwielded2(struct monst *mon, struct obj *obj)
{
    if (!obj)
        return;
    if (artifact_light(obj) && obj->lamplit) {
        end_burn(obj, FALSE);
        if (canseemon(mon))
            pline("%s in %s %s %s shining.", The(xname(obj)),
                  s_suffix(mon_nam(mon)), mbodypart(mon, HAND),
                  otense(obj, "stop"));
    }
    if (MON_WEP2(mon) == obj)
        MON_NOWEP2(mon);
    obj->owornmask &= ~W_SWAPWEP;
}

#undef PN_BARE_HANDED
#undef PN_RIDING
#undef PN_POLEARMS
#undef PN_SABER
#undef PN_HAMMER
#undef PN_WHIP
#undef PN_ATTACK_SPELL
#undef PN_HEALING_SPELL
#undef PN_DIVINATION_SPELL
#undef PN_ENCHANTMENT_SPELL
#undef PN_CLERIC_SPELL
#undef PN_ESCAPE_SPELL
#undef PN_MATTER_SPELL
#undef PN_TELEKINESIS
#undef AKLYS_LIM

/*weapon.c*/
