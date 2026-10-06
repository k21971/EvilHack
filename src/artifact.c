/* NetHack 5.0	artifact.c	$NHDT-Date: 1781973041 2026/06/20 16:30:41 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.264 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2013. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "artifact.h"
#include "artilist.h"

#ifndef SFCTOOL

/*
 * Note:  both artilist[] and artiexist[] have a dummy element #0,
 *        so loops over them should normally start at #1.  The primary
 *        exception is the save & restore code, which doesn't care about
 *        the contents, just the total size.
 */

staticfn struct artifact *get_artifact(struct obj *) NONNULL;

/* #define get_artifact(o) \
    (((o) && ((o)->artifact > 0 && (o)->artifact < AFTER_LAST_ARTIFACT)) \
                             ? &artilist[(int) (o)->oartifact] \
                             : &artilist[ART_NONARTIFACT]) */

staticfn void fix_artifact(struct obj *) NONNULLARG1;
staticfn boolean vecna_relic_denied(struct obj *);
staticfn boolean bane_applies(const struct artifact *, struct monst *)
                                                                 NONNULLARG12;
staticfn int spec_applies(const struct artifact *, struct monst *)
                                                                 NONNULLARG12;
staticfn int invoke_ok(struct obj *);
staticfn void nothing_special(struct obj *) NONNULLARG1;
staticfn int invoke_taming(struct obj *) NONNULLARG1;
staticfn int invoke_healing(struct obj *) NONNULLARG1;
staticfn int invoke_energy_boost(struct obj *) NONNULLARG1;
staticfn int invoke_untrap(struct obj *) NONNULLARG1;
staticfn int invoke_charge_obj(struct obj *) NONNULLARG1;
staticfn int invoke_create_portal(struct obj *) NONNULLARG1;
staticfn int invoke_create_ammo(struct obj *) NONNULLARG1;
staticfn int invoke_banish(struct obj *) NONNULLARG1;
staticfn int invoke_fling_poison(struct obj *) NONNULLARG1;
staticfn int invoke_storm_spell(struct obj *) NONNULLARG1;
staticfn int invoke_blinding_ray(struct obj *) NONNULLARG1;
staticfn int invoke_shadowblade(struct obj *) NONNULLARG1;
staticfn int invoke_phasing(struct obj *) NONNULLARG1;
staticfn int invoke_channel(struct obj *) NONNULLARG1;
staticfn int invoke_death_magic(struct obj *) NONNULLARG1;
staticfn void death_magic_anger(struct monst *) NONNULLARG1;
staticfn int invoke_command_undead(struct obj *) NONNULLARG1;
staticfn int arti_invoke_cost_pw(struct obj *) NONNULLARG1;
staticfn boolean arti_invoke_cost(struct obj *) NONNULLARG1;
staticfn int arti_invoke(struct obj *);
staticfn boolean friendly_in_blast(coordxy, coordxy, struct monst *);
staticfn boolean Mb_hit(struct monst * magr, struct monst *mdef,
                      struct obj *, int *, int, boolean, char *);
staticfn unsigned long abil_to_spfx(long *) NONNULLARG1;
staticfn uchar abil_to_adtyp(long *) NONNULLARG1;
staticfn int glow_strength(int);
staticfn boolean untouchable(struct obj *, boolean);
staticfn int count_surround_traps(coordxy, coordxy);
staticfn void dispose_of_orig_obj(struct obj *);

/* The amount added to the victim's total hit points to insure that the
   victim will be killed even after damage bonus/penalty adjustments.
   Most such penalties are small, and 200 is plenty; the exception is
   half physical damage.  3.3.1 and previous versions tried to use a very
   large number to account for this case; now, we just compute the fatal
   damage by adding it to 2 times the total hit points instead of 1 time.
   Note: this will still break if they have more than about half the number
   of hit points that will fit in a 15 bit integer. */
#define FATAL_DAMAGE_MODIFIER 200
#endif /* SFCTOOL */

/* arti_info struct definition moved to artifact.h */

/* array of flags tracking which artifacts exist, indexed by ART_xx;
   ART_xx values are 1..N, element [0] isn't used; no terminator needed */
static struct arti_info artiexist[1 + NROFARTIFACTS];
/* discovery list; for N discovered artifacts, the first N entries are ART_xx
   values in discovery order, the remaining (NROFARTIFACTS-N) slots are 0 */
static xint16 artidisco[NROFARTIFACTS];
/* note: artiexist[] and artidisco[] don't need to be in struct ga; they
 * get explicitly initialized at game start so don't need to be part of
 * bulk re-init if game restart ever gets implemented.  They are saved
 * and restored but that is done through this file so they can be local.
 */

#ifndef SFCTOOL
static const struct arti_info zero_artiexist = {0}; /* all bits zero */

staticfn void hack_artifacts(void);

/* handle some special cases; must be called after u_init() */
staticfn void
hack_artifacts(void)
{
    struct artifact *art;
    int alignmnt = aligns[flags.initalign].value;

    /* Fix up the alignments of "gift" artifacts */
    for (art = artilist + 1; art->otyp; art++)
        if (art->role == Role_switch && art->alignment != A_NONE)
            art->alignment = alignmnt;

    /* Excalibur can be used by any lawful character, not just knights;
       the same applies to Dirge for chaotics */
    if (!Role_if(PM_KNIGHT)) {
        artilist[ART_EXCALIBUR].role = NON_PM;
        artilist[ART_DIRGE].role = NON_PM;
    }

    if (Role_if(PM_CLERIC)) {
        /* For crowning purposes */
        artilist[ART_MJOLLNIR].alignment = alignmnt;
        /* The Hammer of the Gods is forged from Demonbane and
           Sunsword; since a priest can reach the forge carrying
           their own retargeted Demonbane, mirror the priest's
           alignment onto the result so they are not blasted by
           the artifact they just made */
        artilist[ART_HAMMER_OF_THE_GODS].alignment = alignmnt;
    }

    /* Fix up the quest artifact */
    if (gu.urole.questarti) {
        artilist[gu.urole.questarti].alignment = alignmnt;
        artilist[gu.urole.questarti].role = Role_switch;
    }
    return;
}

/* zero out the artifact existence list */
void
init_artifacts(void)
{
    (void) memset((genericptr_t) artiexist, 0, sizeof artiexist);
    (void) memset((genericptr_t) artidisco, 0, sizeof artidisco);
    hack_artifacts();
}

void
save_artifacts(NHFILE *nhfp)
{
    int i;

    for (i = 0; i < (NROFARTIFACTS + 1); ++i)
        Sfo_arti_info(nhfp, &artiexist[i], "artiexist");

    for (i = 0; i < NROFARTIFACTS; ++i)
        Sfo_xint16(nhfp, &artidisco[i], "artidisco");
}

#endif /* SFCTOOL */

void
restore_artifacts(NHFILE *nhfp)
{
    int i;

    for (i = 0; i < (NROFARTIFACTS + 1); ++i)
        Sfi_arti_info(nhfp, &artiexist[i], "artiexist");
    for (i = 0; i < NROFARTIFACTS; ++i)
        Sfi_short(nhfp, &artidisco[i], "artidisco");
#ifndef SFCTOOL
    hack_artifacts();   /* redo non-saved special cases */
#else
    nhUse(artilist);
#endif
}

#ifndef SFCTOOL

/* some artifacts may need additional tweaking when created;
   called when the artifact is christened */
staticfn void
fix_artifact(struct obj *otmp)
{
    /* the Idol of Moloch is a figurine and must animate a fixed
       species */
    if (otmp->oartifact == ART_IDOL_OF_MOLOCH)
        set_corpsenm(otmp, PM_HORNED_DEVIL);
}

const char *
artiname(int artinum)
{
    if (artinum <= 0 || artinum > NROFARTIFACTS)
        return "";
    return artilist[artinum].name;
}

/* finds a matching SPFX for a prop; used in mondata.c */
unsigned long
arti_prop_spfx(int prop)
{
    switch (prop) {
    case SEARCHING:
        return SPFX_SEARCH;
    case WARNING:
        return SPFX_WARN;
    case TELEPAT:
        return SPFX_ESP;
    case HALLUC_RES:
        return SPFX_HALRES;
    case STEALTH:
        return SPFX_STLTH;
    case REGENERATION:
        return SPFX_REGEN;
    case TELEPORT_CONTROL:
        return SPFX_TCTRL;
    case ENERGY_REGENERATION:
        return SPFX_EREGEN;
    case HALF_SPDAM:
        return SPFX_HSPDAM;
    case HALF_PHDAM:
        return SPFX_HPHDAM;
    case REFLECTING:
        return SPFX_REFLECT;
    }
    return 0L;
}

/* return the material of a given artifact */
short
artifact_material(int artinum)
{
    if (artinum <= 0 || artinum > NROFARTIFACTS) {
        impossible("invalid artifact number %d to artifact_material", artinum);
        return 0;
    }
    return artilist[artinum].material;
}

/* Returns artifact index (1-NROFARTIFACTS) if name matches an artifact,
   or 0 if no match. For use by the in-game encyclopedia lookup */
int
name_to_arti(const char *name)
{
    const struct artifact *a;
    const char *aname;
    int i;

    if (!name || !*name)
        return 0;

    /* skip leading "the " */
    if (!strncmpi(name, "the ", 4))
        name += 4;

    for (a = artilist + 1, i = 1; a->otyp; a++, i++) {
        aname = a->name;
        if (!strncmpi(aname, "the ", 4))
            aname += 4;
        if (!strcmpi(name, aname))
            return i;
    }
    return 0;
}

/* Accessor for artilist entry by index (for pager.c / in-game
   encyclopedia) */
const struct artifact *
get_artilist_entry(int artinum)
{
    if (artinum < 1 || artinum > NROFARTIFACTS)
        return (const struct artifact *) 0;
    return &artilist[artinum];
}

/* create a specific artifact regardless of alignment or role; used when
   forging artifacts together at a forge */
struct obj *
mk_particular_artifact(int artinum)
{
    const struct artifact *a;
    struct obj *obj;

    if (artinum <= 0 || artinum > NROFARTIFACTS) {
        impossible("invalid artifact number %d to mk_particular_artifact",
                   artinum);
        return (struct obj *) 0;
    }
    a = &artilist[artinum];
    obj = mksobj((int) a->otyp, TRUE, FALSE);
    if (!artiexist[artinum].exists) {
        /* oname() sets oartifact and the artifact's material and
           marks it as existing (via artifact_exists()); the hero is
           creating it in hand so it starts out found */
        obj = oname(obj, a->name, ONAME_KNOW_ARTI);
        fix_artifact(obj);
    }
    return obj;
}

/*
   Make an artifact.  If a specific alignment is specified, then an object of
   the appropriate alignment is created from scratch, or 0 is returned if
   none is available.  (If at least one aligned artifact has already been
   given, then unaligned ones also become eligible for this.)
   If no alignment is given, then 'otmp' is converted
   into an artifact of matching type, or returned as-is if that's not
   possible.
   For the 2nd case, caller should use ``obj = mk_artifact(obj, A_NONE, 99);''
   For the 1st, ``obj = mk_artifact((struct obj *) 0, some_alignment, ...);''.
   The max_giftvalue is the value of the sacrifice, for an artifact obtained
   by sacrificing, or 99 otherwise.
 */
struct obj *
mk_artifact(
    struct obj *otmp,    /* existing object; ignored and disposed of
                          * if alignment specified */
    aligntyp alignment,  /* target alignment, or A_NONE */
    uchar max_giftvalue, /* cap on generated giftvalue */
    boolean adjust_spe)  /* whether to add spe to situational artifacts */
{
    const struct artifact *a;
    int m, n, altn;
    boolean by_align = (alignment != A_NONE);
    short o_typ = (by_align || !otmp) ? 0 : otmp->otyp;
    boolean unique = !by_align && otmp && objects[o_typ].oc_unique;
    short eligible[NROFARTIFACTS];
    xint16 skill_compatibility;

    /* don't convert objects that already have special properties */
    if (otmp && otmp->oprops)
        return otmp;
    if (otmp && otmp->oartifact)
        return otmp;

    n = altn = 0;    /* no candidates found yet */
    eligible[0] = 0; /* lint suppression */
    /* gather eligible artifacts */
    for (m = 1, a = &artilist[m]; a->otyp; a++, m++) {
        if (artiexist[m].exists)
            continue;
        if ((a->spfx & SPFX_NOGEN) || unique)
            continue;
        if (a->gift_value > max_giftvalue && !Role_if(a->role))
            continue;

        if (!by_align) {
            /* looking for a particular type of item; not producing a
               divine gift so we don't care about role's first choice */
            if (a->otyp == o_typ)
                eligible[n++] = m;
            continue; /* move on to next possibility */
        }

        /* we're looking for an alignment-specific item
           suitable for hero's role+race */
        if ((a->alignment == alignment || a->alignment == A_NONE)
            /* avoid enemies' equipment */
            && (a->race == NON_PM || !race_hostile(&mons[a->race]))
            /* handles orcs receiving Orcrist/Sting, elves/drow
               receiving Grimtooth, demons (crowned infidel) somehow
               receiving Demonbane */
            && !Hate_material(artifact_material(m))
            /* handles giants receiving Giantslayer */
            && !(Race_if(PM_GIANT) && (a->mtype & MH_GIANT))
            /* handles undead races receiving Sunsword */
            && !((Race_if(PM_DRAUGR) || Race_if(PM_VAMPIRE))
                 && (a->mtype & MH_UNDEAD))
            /* handles aasimar receiving Angelslayer */
            && !(Race_if(PM_AASIMAR) && (a->mtype & MH_AASIMAR))) {
            /* when a role-specific first choice is available, use it */
            if (Role_if(a->role)) {
                /* make this be the only possibility in the list */
                eligible[0] = m;
                n = 1;
                break; /* skip all other candidates */
            }

            /* check if this is skill-compatible */
            skill_compatibility = P_SKILLED;
            if (objects[a->otyp].oc_class == WEAPON_CLASS) {
                schar skill = objects[a->otyp].oc_skill;
                if (skill < 0)
                    skill_compatibility = P_MAX_SKILL(-skill);
                else
                    skill_compatibility = P_MAX_SKILL(skill);
            }

            /* found something to consider for random selection */
            if ((a->alignment != A_NONE || u.ugifts > 0 || !rn2(3)) &&
                (!rn2(4) || skill_compatibility >= P_SKILLED ||
                 (skill_compatibility >= P_BASIC && rn2(2)))) {
                /* right alignment, or non-aligned with at least 1
                   previous gift bestowed, makes this one viable;
                   unaligned artifacts are possible even as the first
                   gift, but less likely; if it's a bad weapon type
                   for the role that also makes it less likely */
                eligible[n++] = m;
            } else {
                /* if no candidates have been found yet, record
                   this one as a[nother] fallback possibility in
                   case all aligned candidates have been used up
                   (via wishing, naming, bones, random generation)
                   or failed the randomized compatibility checks */
                if (!n)
                    eligible[altn++] = m;
                /* [once a regular candidate is found, the list
                   is overwritten and `altn' becomes irrelevant] */
            }
        }
    }

    /* resort to fallback list if main list was empty */
    if (!n)
        n = altn;

    if (n) {
        /* found at least one candidate; pick one at random */
        m = eligible[rn2(n)]; /* [0..n-1] */
        a = &artilist[m];

        /* make an appropriate object if necessary, then christen it */
        if (by_align) {
            /* 'by_align' indicates that an alignment was passed as
             * an argument, but also that the 'otmp' argument is not
             * relevant */
            struct obj *artiobj = mksobj((int) a->otyp, TRUE, FALSE);

            /* nonnull value of 'otmp' is unexpected. Cope. */
            if (otmp) /* just in case; avoid orphaning */
                dispose_of_orig_obj(otmp);
            otmp = artiobj;
        }
        /*
         * otmp should be nonnull at this point:
         * either the passed argument (if !by_align == A_NONE), or
         * the result of mksobj() just above if by_align is an alignment. */
        assert(otmp != 0);
        /* prevent erosion from generating */
        otmp->oeroded = otmp->oeroded2 = 0;
        /* prevent 'superior/exceptional/inferior' or gem-studded
           artifacts */
        otmp->forged_qual = FQ_NORMAL;
        otmp->affixed_gem = 0;
        /* prevent object properties or erosion-protection from
           leaking onto artifacts via mksobj's random rolls */
        otmp->oprops = otmp->oprops_known = 0L;
        otmp->oerodeproof = 0;
        otmp->greased = 0;
        otmp = oname(otmp, a->name, ONAME_NO_FLAGS);
        otmp->oartifact = m;  /* probably already set by this point, but */
        /* set existence and reason for creation bits */
        artifact_origin(otmp, ONAME_RANDOM); /* 'random' is default */
        fix_artifact(otmp);
        if (adjust_spe) {
            int new_spe;

            /* Adjust artiobj->spe by a->gen_spe. (This is a no-op for
               non-weapons, which always have a gen_spe of 0, and for many
               weapons, too.) The result is clamped into the "normal" range to
               prevent an outside chance of +12 artifacts generating. */
            new_spe = (int) otmp->spe + a->gen_spe;
            if (new_spe >= -10 && new_spe < 10)
                otmp->spe = new_spe;
        }
    } else {
        /* nothing appropriate could be found; return original object */
        if (by_align && otmp) {
            /* (there shouldn't have been an original object). Deal with it.
             * The callers that passed an alignment and a NULL otmp are
             * prepared to get a potential NULL return value, so this is okay */
            dispose_of_orig_obj(otmp);
            otmp = 0;
        } /* otherwise, otmp has not changed; just fallthrough to return it */
    }
    if (otmp && permapoisoned(otmp))
        otmp->opoisoned = 1;
    return otmp;
}

staticfn void
dispose_of_orig_obj(struct obj *obj)
{
    if (!obj)
        return;

    obj_extract_self(obj);
    obfree(obj, (struct obj *) 0);
}

/* Enforce mutual exclusivity of damage properties.
   Only one of FIRE/FROST/SHOCK/VENOM/DRLI can exist on an item.
   Priority order: FROST > FIRE > SHOCK > VENOM > DRLI */
long
enforce_oprops_mutex(long props)
{
    if (props & ITEM_FROST)
        props &= ~(ITEM_DMG_PROPS & ~ITEM_FROST);
    else if (props & ITEM_FIRE)
        props &= ~(ITEM_DMG_PROPS & ~ITEM_FIRE);
    else if (props & ITEM_SHOCK)
        props &= ~(ITEM_DMG_PROPS & ~ITEM_SHOCK);
    else if (props & ITEM_VENOM)
        props &= ~(ITEM_DMG_PROPS & ~ITEM_VENOM);
    else if (props & ITEM_DRLI)
        props &= ~(ITEM_DMG_PROPS & ~ITEM_DRLI);
    return props;
}

/* Create an item with special properties, or grant the item those
 * properties.
 *
 * Object property restrictions:
 * - Weapons/weptools: No DRLI (would be Stormbringer-like, overpowered),
 *   no OILSKIN (doesn't make sense), no FUMBLING/HUNGER (handled
 *   differently)
 * - Launchers (bows): No damage properties (damage comes from ammo)
 * - Ammo/missiles: No utility properties (too transient to matter)
 * - Helmets: No ESP (helm of telepathy already exists)
 * - Barding: Limited properties (practical considerations)
 * - Non-cloth: No OILSKIN (oilskin is inherently cloth-based)
 * - Damage properties (FIRE/FROST/SHOCK/VENOM/DRLI) are mutually
 *   exclusive
 */
struct obj *
create_oprop(struct obj *obj, boolean allow_detrimental)
{
    struct obj *otmp = obj;
    int i, j;

    if (!otmp) {
        int type = 0, skill = P_NONE,
            candidates[128], ccount,
            threshold = P_EXPERT;
        /* this probably is only ever done for weapons;
           find an appropriate type of weapon */
        while (threshold > P_UNSKILLED) {
            ccount = 0;
            for (i = P_FIRST_WEAPON; i < P_LAST_WEAPON; i++) {
                if (P_MAX_SKILL(i) >= max(threshold, P_BASIC)
                    && P_SKILL(i) >= threshold)
                    candidates[ccount++] = i;
                if (ccount >= 128)
                    break;
            }
            if (ccount == 0) {
                threshold--;
                continue;
            }
            skill = candidates[rn2(ccount)];
            ccount = 0;
            for (i = ARROW; i <= CROSSBOW; i++) {
                if (abs(objects[i].oc_skill) == skill)
                    candidates[ccount++] = i;
                if (ccount == 128)
                    break;
            }
            if (!ccount) {
                impossible("found no weapons for skill %d?", skill);
                threshold--;
                continue;
            }
            type = candidates[rn2(ccount)];
            break;
        }
        /* now make one, if we can */
        if (type != 0)
            otmp = mksobj(type, TRUE, FALSE);
        else
            otmp = mkobj(WEAPON_CLASS, FALSE);
    }

    /* don't spruce up certain objects */
    if (otmp->oartifact)
        return otmp;
    else if (objects[otmp->otyp].oc_unique)
        return otmp;
    /* weapons made from monster body parts never get
       object properties */
    else if (otmp->otyp == WORM_TOOTH
             || otmp->otyp == UNICORN_HORN
             || otmp->otyp == CRYSKNIFE)
        return otmp;
    /* already magical items obtain properties
       a tenth as often */
    else if (objects[otmp->otyp].oc_magic && rn2(10))
        return otmp;
    else if (Is_dragon_armor(otmp))
        return otmp;

    /* properties only added to weapons and armor */
    if (otmp->oclass != WEAPON_CLASS && !is_bullet(otmp)
        && !is_weptool(otmp) && !is_barding(otmp)
        && otmp->oclass != ARMOR_CLASS)
        return otmp;

    /* it is possible to have an object spawn with more
       than one object property, but the odds are so low
       that if it happens, well good for you */
    while (!otmp->oprops || !rn2(100000)) {
        i = rn2(MAX_ITEM_PROPS);
        j = 1 << i; /* pick an object property */

        if (otmp->oprops & j)
            continue;

        if ((j & (ITEM_FUMBLING | ITEM_HUNGER))
            && !allow_detrimental)
            continue;

        /* check for restrictions */
        if ((otmp->oclass == WEAPON_CLASS || is_weptool(otmp))
            && (j & (ITEM_DRLI | ITEM_FUMBLING | ITEM_HUNGER
                     | ITEM_OILSKIN)))
            continue;

        if (is_launcher(otmp)
            && (j & (ITEM_FIRE | ITEM_FROST | ITEM_SHOCK
                     | ITEM_VENOM | ITEM_DRLI | ITEM_OILSKIN)))
            continue;

        if ((is_ammo(otmp) || is_missile(otmp) || is_bullet(otmp))
            && (j & (ITEM_DRLI | ITEM_OILSKIN | ITEM_ESP
                     | ITEM_EXCEL | ITEM_SEARCHING | ITEM_WARNING
                     | ITEM_FUMBLING | ITEM_HUNGER)))
            continue;

        if ((otmp->oprops & ITEM_DMG_PROPS) && (j & ITEM_DMG_PROPS))
            continue; /* damage properties are mutually exclusive */

        if (otmp->material != CLOTH && (j & ITEM_OILSKIN))
            continue;

        if (otmp->otyp == OILSKIN_CLOAK && (j & ITEM_OILSKIN))
            continue;

        /* helm of telepathy already exists */
        if (is_helmet(otmp) && (j & ITEM_ESP))
            continue;

        if (is_barding(otmp)
            && (j & (ITEM_OILSKIN | ITEM_EXCEL
                     | ITEM_SEARCHING | ITEM_WARNING
                     | ITEM_FUMBLING | ITEM_HUNGER)))
            continue;

        otmp->oprops |= j;
    }

    /* fix it up as necessary */
    if (otmp->oprops
        && (otmp->oclass == WEAPON_CLASS
            || otmp->oclass == ARMOR_CLASS
            || is_bullet(otmp) || is_barding(otmp))
        && !(otmp->oprops & (ITEM_FUMBLING | ITEM_HUNGER))) {
        if (!rn2(8)) {
            blessorcurse(otmp, 8);
            if (otmp->cursed)
                otmp->spe = -rne(3);
            else
                otmp->spe = rne(3);
        }
    }

    if (otmp->oprops & (ITEM_FUMBLING | ITEM_HUNGER)) {
        if (!otmp->cursed)
            curse(otmp);
        otmp->spe = -rne(3);
    }
    return otmp;
}

/*
 * Returns the full name (with articles and correct capitalization) of an
 * artifact named "name" if one exists, or NULL, it not.
 * The given name must be rather close to the real name for it to match.
 * The object type of the artifact is returned in otyp if the return value
 * is non-NULL.
 */
const char *
artifact_name(
    const char *name, /* string from player that might be an artifact name */
    short *otyp_p,    /* secondary output */
    boolean fuzzy)    /* whether to allow extra or omitted spaces or dashes */
{
    const struct artifact *a;
    const char *aname;

    if (!strncmpi(name, "the ", 4))
        name += 4;

    for (a = artilist + 1; a->otyp; a++) {
        aname = a->name;
        if (!strncmpi(aname, "the ", 4))
            aname += 4;
        if (!fuzzy ? !strcmpi(name, aname)
                   : fuzzymatch(name, aname, " -", TRUE)) {
            if (otyp_p)
                *otyp_p = a->otyp;
            return a->name;
        }
    }

    return (char *) 0;
}

boolean
exist_artifact(int otyp, const char *name)
{
    const struct artifact *a;
    struct arti_info *arex;

    if (otyp && *name)
        for (a = artilist + 1, arex = artiexist + 1; a->otyp; a++, arex++)
            if ((int) a->otyp == otyp && !strcmp(a->name, name))
                return arex->exists ? TRUE : FALSE;
    return FALSE;
}

/* an artifact has just been created or is being "un-created" for a chance
   to be created again later */
void
artifact_exists(
    struct obj *otmp,
    const char *name,
    boolean mod,      /* True: exists, False: being un-created */
    unsigned flgs)    /* ONAME_xyz flags; not relevant if !mod */
{
    const struct artifact *a;

    if (otmp && *name)
        for (a = artilist + 1; a->otyp; a++)
            if (a->otyp == otmp->otyp && !strcmp(a->name, name)) {
                int m = (int) (a - artilist);

                otmp->oartifact = (char) (mod ? m : 0);
                otmp->age = 0;
                if (otmp->otyp == RIN_INCREASE_DAMAGE)
                    otmp->spe = 0;
                if (mod) { /* means being created rather than un-created */
                    /* one--and only one--of these should always be set */
                    if ((flgs & (ONAME_VIA_NAMING | ONAME_WISH | ONAME_GIFT
                                 | ONAME_VIA_DIP | ONAME_LEVEL_DEF
                                 | ONAME_BONES | ONAME_RANDOM)) == 0)
                        flgs |= ONAME_RANDOM; /* the default origin */
                    /* 'exists' bit will become set (in artifact_origin();
                       there's no ONAME_ flag) and flgs might also contain
                       the know_arti bit (hero knows that artifact exists) */
                    artifact_origin(otmp, flgs);
                    fix_artifact(otmp);
                } else { /* uncreate */
                    /* clear all the flag bits */
                    artiexist[m] = zero_artiexist;
                }
                break;
            }
    return;
}

/* mark an artifact as 'found' */
void
found_artifact(int a)
{
    if (a < 1 || a > NROFARTIFACTS)
        impossible("found_artifact: invalid artifact index! (%d)", a);
    else if (!artiexist[a].exists)
        impossible("found_artifact: artifact doesn't exist yet? (%d)", a);
    else
        artiexist[a].found = 1;
}

/* if an artifact hasn't already been designated 'found', do that now
   and generate a livelog event about finding it */
void
find_artifact(struct obj *otmp)
{
    int a = otmp->oartifact;

    if (a && !artiexist[a].found) {
        const char *where;

        found_artifact(a); /* artiexist[a].found = 1 */
        /*
         * Unlike costly_spot(), inside_shop() includes the "free spot"
         * in front of the door.  And it doesn't care whether or not
         * there is a shopkeeper present.
         *
         * If hero sees a monster pick up a not-yet-found artifact, it
         * will have its dknown flag set even if far away and will be
         * described as 'found on the floor'.  Similarly for dropping
         * (possibly upon monster's death), dknown will be set and the
         * artifact will be described as 'carried by a monster'.
         * That's handled by caller:  dog_invent(), mpickstuff(), or
         * mdrop_obj() so that we get called before obj->where changes.
         */
        where = ((otmp->where == OBJ_FLOOR)
                 ?  ((inside_shop(otmp->ox, otmp->oy) != NO_ROOM)
                     ? " in a shop"
                     : " on the floor")
                 /* artifacts aren't created in containers but could be
                    inside one if it comes from a bones level */
                 : (otmp->where == OBJ_CONTAINED) ? " in a container"
                   /* perhaps probing, or seeing monster wield artifact */
                   : (otmp->where == OBJ_MINVENT) ? " carried by a monster"
                     /* catchall: probably in inventory, picked up while
                        blind but now seen; there's no previous_where to
                        figure out how it got here */
                     : "");
        livelog_printf(LL_ARTIFACT, "found %s%s",
                       bare_artifactname(otmp), where);
    }
}

int
nartifact_exist(void)
{
    int i, a = 0;

    for (i = 1; i <= NROFARTIFACTS; ++i)
        if (artiexist[i].exists)
            ++a;

    return a;
}

/* set artifact tracking flags;
   calling sequence: oname() -> artifact_exists() -> artifact_origin() or
   mksobj(),others -> mk_artifact() -> artifact_origin(random) possibly
   followed by mksobj(),others -> artifact_origin(non-random origin) */
void
artifact_origin(
    struct obj *arti, /* new artifact */
    unsigned aflags)  /* ONAME_xxx flags, shared by artifact_exists() */
{
    int ct, a = arti->oartifact;

    if (a) {
        /* start by clearing all bits; most are mutually exclusive */
        artiexist[a] = zero_artiexist;
        /* set 'exists' bit back on; not specified via flag bit in aflags */
        artiexist[a].exists = 1;
        /* 'hero knows it exists' is expected for wish, gift, viadip, or
           named and could eventually become set for any of the others */
        if ((aflags & ONAME_KNOW_ARTI) != 0)
            artiexist[a].found = 1;
        /* should be exactly one of wish, gift, via_dip, via_naming,
           level_def (quest), bones, and random (floor or monst's minvent) */
        ct = 0;
        if ((aflags & ONAME_WISH) != 0)
            artiexist[a].wish = 1, ++ct;
        if ((aflags & ONAME_GIFT) != 0)
            artiexist[a].gift = 1, ++ct;
        if ((aflags & ONAME_VIA_DIP) != 0)
            artiexist[a].viadip = 1, ++ct;
        if ((aflags & ONAME_VIA_NAMING) != 0)
            artiexist[a].named = 1, ++ct;
        if ((aflags & ONAME_LEVEL_DEF) != 0)
            artiexist[a].lvldef = 1, ++ct;
        if ((aflags & ONAME_BONES) != 0)
            artiexist[a].bones = 1, ++ct;
        if ((aflags & ONAME_RANDOM) != 0)
            artiexist[a].rndm = 1, ++ct;
        if (ct != 1)
            impossible("invalid artifact origin: %4o", aflags);
    }
}

boolean
spec_ability(struct obj *otmp, unsigned long abil)
{
    const struct artifact *arti = get_artifact(otmp);

    /* worn/wielded abilities (spfx) or carried abilities (cspfx) */
    return (boolean) (arti != &artilist[ART_NONARTIFACT]
                      && ((arti->spfx | arti->cspfx) & abil) != 0L);
}

/* used to check if an artifact can glow in warning to monsters of some
   races; if it can, return the bitmap of warned races, otherwise 0 */
int
has_glow_warning(struct obj *obj)
{
    const struct artifact *arti = get_artifact(obj);

    if (arti != &artilist[ART_NONARTIFACT]
        && (arti->spfx & SPFX_WARN) && (arti->spfx & SPFX_DFLAGH))
        return arti->mtype;
    return 0;
}

/* used so that callers don't need to known about SPFX_ codes */
boolean
confers_luck(struct obj *obj)
{
    /* might as well check for this too */
    if (obj->otyp == LUCKSTONE)
        return TRUE;

    if ((obj->oprops & ITEM_EXCEL)
        && (obj->owornmask & (W_ARMOR | W_WEP | W_SWAPWEP)))
        return TRUE;

    return (boolean) (obj->oartifact && spec_ability(obj, SPFX_LUCK));
}

/* used to check whether a monster is getting reflection from an artifact */
boolean
arti_reflects(struct obj *obj)
{
    const struct artifact *arti = get_artifact(obj);

    if (arti != &artilist[ART_NONARTIFACT]) {
        /* while being worn */
        if ((obj->owornmask & ~W_ART) && (arti->spfx & SPFX_REFLECT))
            return TRUE;
        /* just being carried */
        if (arti->cspfx & SPFX_REFLECT)
            return TRUE;
    }
    return FALSE;
}

/* check whether a monster gets a given SPFX_ effect from any artifact
   in its inventory -- generalizes the arti_reflects() pattern; pass
   &gy.youmonst to check the hero's inventory */
boolean
mon_arti_has_spfx(struct monst *mon, unsigned long spfx_flag)
{
    struct obj *obj;
    const struct artifact *arti;

    for (obj = (mon == &gy.youmonst) ? gi.invent : mon->minvent;
         obj; obj = obj->nobj) {
        if (!obj->oartifact)
            continue;
        arti = get_artifact(obj);
        if (arti == &artilist[ART_NONARTIFACT])
            continue;
        if ((obj->owornmask & ~W_ART) && (arti->spfx & spfx_flag))
            return TRUE;
        if (arti->cspfx & spfx_flag)
            return TRUE;
    }
    return FALSE;
}

/* decide whether this obj is effective when attacking against shades;
   does not consider the bonus for blessed objects versus undead */
boolean
shade_glare(struct obj *obj)
{
    const struct artifact *arti;

    /* any silver object is effective; bone too, though it gets no bonus */
    if (obj->material == SILVER || obj->material == BONE)
        return TRUE;
    /* any weapon with a +2 enchantment or better is effective */
    if ((obj->oclass == WEAPON_CLASS || is_weptool(obj)) && obj->spe > 1)
        return TRUE;
    /* non-silver artifacts with bonus against undead also are effective */
    arti = get_artifact(obj);
    if (arti != &artilist[ART_NONARTIFACT] && (arti->spfx & SPFX_DFLAGH)
                             && arti->mtype == MH_UNDEAD)
        return TRUE;
    /* [if there was anything with special bonus against noncorporeals,
       it would be effective too] */
    /* otherwise, harmless to shades */
    return FALSE;
}

/* returns 1 if name is restricted for otmp->otyp */
boolean
restrict_name(struct obj *otmp, const char *name)
{
    const struct artifact *a;
    const char *aname, *odesc, *other;
    boolean sametype[NUM_OBJECTS];
    int i, lo, hi, otyp = otmp->otyp, ocls = objects[otyp].oc_class;

    if (!*name)
        return FALSE;
    if (!strncmpi(name, "the ", 4))
        name += 4;

    /* decide what types of objects are the same as otyp;
       if it's been discovered, then only itself matches;
       otherwise, include all other undiscovered objects
       of the same class which have the same description
       or share the same pool of shuffled descriptions */
    (void) memset((genericptr_t) sametype, 0, sizeof sametype); /* FALSE */
    sametype[otyp] = TRUE;
    if (!objects[otyp].oc_name_known
        && (odesc = OBJ_DESCR(objects[otyp])) != 0) {
        obj_shuffle_range(otyp, &lo, &hi);
        for (i = svb.bases[ocls]; i < NUM_OBJECTS; i++) {
            if (objects[i].oc_class != ocls)
                break;
            if (!objects[i].oc_name_known
                && (other = OBJ_DESCR(objects[i])) != 0
                && (!strcmp(odesc, other) || (i >= lo && i <= hi)))
                sametype[i] = TRUE;
        }
    }

    /* Since almost every artifact is SPFX_RESTR, it doesn't cost
       us much to do the string comparison before the spfx check.
       Bug fix:  don't name multiple elven daggers "Sting".
     */
    for (a = artilist + 1; a->otyp; a++) {
        if (!sametype[a->otyp])
            continue;
        aname = a->name;
        if (!strncmpi(aname, "the ", 4))
            aname += 4;
        if (!strcmp(aname, name))
            return (boolean) ((a->spfx & (SPFX_NOGEN | SPFX_RESTR)) != 0
                              || otmp->quan > 1L);
    }

    return FALSE;
}

boolean
attacks(int adtyp, struct obj *otmp)
{
    const struct artifact *weap;

    if ((weap = get_artifact(otmp)) != &artilist[ART_NONARTIFACT])
        return (boolean) (weap->attk.adtyp == adtyp);

    /* not an artifact; check for object properties on weapons/armor */
    if (otmp->oprops
        && (otmp->oclass == WEAPON_CLASS
            || is_weptool(otmp) || is_bullet(otmp)
            || (uarms && otmp == uarms)
            || (uarmg && otmp == uarmg))) {
        if (adtyp == AD_FIRE && (otmp->oprops & ITEM_FIRE))
            return TRUE;
        if (adtyp == AD_COLD && (otmp->oprops & ITEM_FROST))
            return TRUE;
        if (adtyp == AD_ELEC && (otmp->oprops & ITEM_SHOCK))
            return TRUE;
        if (adtyp == AD_DRST && (otmp->oprops & ITEM_VENOM))
            return TRUE;
    }
    return FALSE;
}

boolean
defends(int adtyp, struct obj *otmp)
{
    const struct artifact *weap;

    if (!otmp)
        return FALSE;
    if ((weap = get_artifact(otmp)) != &artilist[ART_NONARTIFACT]) {
        /* a fire-and-cold fusion defense covers both elements */
        if (weap->defn.adtyp == AD_FUSE
            && (adtyp == AD_FIRE || adtyp == AD_COLD))
            return TRUE;
        return (boolean) (weap->defn.adtyp == adtyp);
    }
    if (Is_dragon_armor(otmp)) {
        /* convert dragon-scaled armor to scales to simplify testing */
        int otyp = Dragon_armor_to_scales(otmp);

        switch (adtyp) {
        case AD_MAGM: /* magic missiles => general magic resistance */
            return (otyp == GRAY_DRAGON_SCALES
                    || otyp == CHROMATIC_DRAGON_SCALES);
        case AD_DISE: /* blocks disease but not slime */
            return (otyp == GREEN_DRAGON_SCALES);
        case AD_FIRE:
            return (otyp == RED_DRAGON_SCALES); /* red but not gold */
        case AD_COLD:
            return (otyp == WHITE_DRAGON_SCALES
                    || otyp == SILVER_DRAGON_SCALES);
        case AD_DRST: /* drain strength => poison */
            return (otyp == GREEN_DRAGON_SCALES);
        case AD_SLEE: /* sleep */
            return (otyp == ORANGE_DRAGON_SCALES
                    || otyp == SHADOW_DRAGON_SCALES);
        case AD_DISN: /* disintegration */
            return (otyp == BLACK_DRAGON_SCALES);
        case AD_ELEC: /* electricity == lightning */
            return (otyp == BLUE_DRAGON_SCALES
                    || otyp == CELESTIAL_DRAGON_SCALES);
        case AD_SLOW: /* confers speed so blocks speed removal */
            return (otyp == BLUE_DRAGON_SCALES);
        case AD_ACID:
        case AD_STON: /* petrification resistance */
            return (otyp == YELLOW_DRAGON_SCALES);
        case AD_LOUD: /* sonic */
            return (otyp == CELESTIAL_DRAGON_SCALES);
        case AD_DRLI: /* drain resistance */
            return (otyp == SHADOW_DRAGON_SCALES);
        case AD_STUN: /* stun resistance */
            return (otyp == SHIMMERING_DRAGON_SCALES);
        default:
            break;
        }
    }
    return FALSE;
}

/* used for monsters */
boolean
defends_when_carried(int adtyp, struct obj *otmp)
{
    const struct artifact *weap;

    if ((weap = get_artifact(otmp)) != &artilist[ART_NONARTIFACT])
        return (boolean) (weap->cary.adtyp == adtyp);
    return FALSE;
}

/* determine whether an item confers Protection */
boolean
protects(struct obj *otmp, boolean being_worn)
{
    const struct artifact *arti;

    if (being_worn && objects[otmp->otyp].oc_oprop == PROTECTION)
        return TRUE;
    arti = get_artifact(otmp);
    if (arti == &artilist[ART_NONARTIFACT])
        return FALSE;
    return (boolean) ((arti->cspfx & SPFX_PROTECT) != 0
                      || (being_worn && (arti->spfx & SPFX_PROTECT) != 0));
}

/* an aasimar's celestial nature draws no power from Vecna's relics; the
   Eye confers intrinsics while merely carried, the Hand only while worn
   (and an aasimar cannot wear it) -- deny both */
staticfn boolean
vecna_relic_denied(struct obj *obj)
{
    return (boolean) (obj
                      && (is_art(obj, ART_EYE_OF_VECNA)
                          || is_art(obj, ART_HAND_OF_VECNA))
                      && racial_aasimar(&gy.youmonst));
}

/*
 * a potential artifact has just been worn/wielded/picked-up or
 * unworn/unwielded/dropped.  Pickup/drop only set/reset the W_ART mask.
 */
void
set_artifact_intrinsic(
    struct obj *otmp,
    boolean on,
    long wp_mask)
{
    long *mask = 0;
    long *mask2 = 0;
    const struct artifact *art, *oart = get_artifact(otmp);
    struct obj *obj;
    uchar dtyp;
    long spfx;
    unsigned long seenres_lost = M_SEEN_NOTHING;

    if (oart == &artilist[ART_NONARTIFACT] && !otmp->oprops)
        return;

    /* effects from the defn field; the non-artifact dummy entry
       yields zero here so plain property objects are unaffected */
    dtyp = (wp_mask != W_ART) ? oart->defn.adtyp : oart->cary.adtyp;

    /* an aasimar gains nothing from Vecna's relics */
    if (vecna_relic_denied(otmp))
        dtyp = 0;

    if (dtyp == AD_FIRE)
        mask = &EFire_resistance;
    else if (dtyp == AD_COLD)
        mask = &ECold_resistance;
    else if (dtyp == AD_ELEC)
        mask = &EShock_resistance;
    else if (dtyp == AD_MAGM) {
        mask = &EAntimagic;
        /* no magic resistance conduct */
        if (on)
            u.uconduct.antimagic++;
    } else if (dtyp == AD_DISN)
        mask = &EDisint_resistance;
    else if (dtyp == AD_DRST)
        mask = &EPoison_resistance;
    else if (dtyp == AD_DRLI)
        mask = &EDrain_resistance;
    else if (dtyp == AD_ACID)
        mask = &EAcid_resistance;
    else if (dtyp == AD_STON) {
        mask = &EStone_resistance;
        if (Stoned)
            make_stoned(0L, "You no longer seem to be petrifying.", 0,
                        (char *) 0);
    } else if (dtyp == AD_DISE) {
        mask = &ESick_resistance;
        if (Sick)
            make_sick(0L, (char *) 0, TRUE, SICK_ALL);
    } else if (dtyp == AD_DETH)
        mask = &EDeath_resistance;
    else if (dtyp == AD_STUN) {
        mask = &EStun_resistance;
        if (Stunned)
            make_stunned(0L, TRUE);
    } else if (dtyp == AD_FUSE) {
        /* fusion of fire and frost confers both resistances */
        mask = &EFire_resistance;
        mask2 = &ECold_resistance;
    }

    if (mask && wp_mask == W_ART && !on) {
        /* find out if some other artifact also confers this intrinsic;
           if so, leave the mask alone */
        for (obj = gi.invent; obj; obj = obj->nobj) {
            if (obj != otmp && obj->oartifact) {
                art = get_artifact(obj);
                if (art != &artilist[ART_NONARTIFACT]
                        && art->cary.adtyp == dtyp
                        && !vecna_relic_denied(obj)) {
                    mask = (long *) 0;
                    break;
                }
            }
        }
    }
    /* once the hero loses a grant this item was actually giving,
       monsters watching will try that element again, as for an item's
       base property in setworn(); an off-hand weapon is torn down on
       every reseat whether or not it was granting anything */
    if (mask) {
        if (on) {
            *mask |= wp_mask;
        } else {
            if (*mask & wp_mask)
                seenres_lost |= (dtyp == AD_FUSE)
                                    ? M_SEEN_FIRE
                                    : cvt_adtyp_to_mseenres(dtyp);
            *mask &= ~wp_mask;
        }
    }
    if (mask2) { /* second resistance for AD_FUSE */
        if (on) {
            *mask2 |= wp_mask;
        } else {
            if (*mask2 & wp_mask)
                seenres_lost |= M_SEEN_COLD;
            *mask2 &= ~wp_mask;
        }
    }

    /* intrinsics from the spfx field; there could be more than one */
    spfx = (wp_mask != W_ART) ? oart->spfx : oart->cspfx;
    if (oart == &artilist[ART_NONARTIFACT]
        && (wp_mask == W_WEP || wp_mask == W_SWAPWEP)) {
        /* object properties on a wielded weapon */
        if (otmp->oprops & ITEM_SEARCHING)
            spfx |= SPFX_SEARCH;
        if (otmp->oprops & ITEM_WARNING)
            spfx |= SPFX_WARN;
        if (otmp->oprops & ITEM_ESP)
            spfx |= SPFX_ESP;
    }

    /* an aasimar gains nothing from Vecna's relics */
    if (vecna_relic_denied(otmp))
        spfx = 0;

    if (spfx && wp_mask == W_ART && !on) {
        /* don't change any spfx also conferred by other artifacts */
        for (obj = gi.invent; obj; obj = obj->nobj)
            if (obj != otmp && obj->oartifact) {
                art = get_artifact(obj);
                if (art != &artilist[ART_NONARTIFACT]
                    && !vecna_relic_denied(obj))
                    spfx &= ~art->cspfx;
            }
    }

    if (spfx & SPFX_SEARCH) {
        if (on)
            ESearching |= wp_mask;
        else
            ESearching &= ~wp_mask;
    }
    if (spfx & SPFX_HALRES) {
        /* make_hallucinated must (re)set the mask itself to get
         * the display right */
        /* restoring needed because this is the only artifact intrinsic
         * that can print a message--need to guard against being printed
         * when restoring a game
         */
        (void) make_hallucinated((long) !on,
                                 program_state.restoring ? FALSE : TRUE,
                                 wp_mask);
    }
    if (spfx & SPFX_ESP) {
        if (on)
            ETelepat |= wp_mask;
        else
            ETelepat &= ~wp_mask;
        recalc_telepat_range();
        see_monsters();
    }
    if (spfx & SPFX_STLTH) {
        if (on)
            EStealth |= wp_mask;
        else
            EStealth &= ~wp_mask;
    }
    if (spfx & SPFX_REGEN) {
        if (on)
            ERegeneration |= wp_mask;
        else
            ERegeneration &= ~wp_mask;
    }
    if (spfx & SPFX_TCTRL) {
        if (on)
            ETeleport_control |= wp_mask;
        else
            ETeleport_control &= ~wp_mask;
    }
    if (spfx & SPFX_WARN) {
        if (spec_mh(otmp)) {
            /* since multiple artifacts can potentially warn of the
               same monster type we need to recalculate everything
               on any use or unuse */
            EWarn_of_mon = 0L;
            svc.context.warntype.obj = 0L;
            for (obj = gi.invent; obj; obj = obj->nobj) {
                if (obj == otmp && !on) {
                    glow_warning_effects(otmp);
                    obj->lastwarncnt = 0;
                } else if (((obj->owornmask & ~W_SWAPWEP)
                            || (obj->owornmask && u.twoweap))
                           && has_glow_warning(obj)) {
                    EWarn_of_mon |= obj->owornmask;
                    svc.context.warntype.obj |= spec_mh(obj);
                }
            }
        } else {
            if (on)
                EWarning |= wp_mask;
            else
                EWarning &= ~wp_mask;
        }
        see_monsters();
    }
    if (spfx & SPFX_EREGEN) {
        if (on)
            EEnergy_regeneration |= wp_mask;
        else
            EEnergy_regeneration &= ~wp_mask;
    }
    if (spfx & SPFX_HSPDAM) {
        if (on)
            EHalf_spell_damage |= wp_mask;
        else
            EHalf_spell_damage &= ~wp_mask;
    }
    if (spfx & SPFX_HPHDAM) {
        if (on)
            EHalf_physical_damage |= wp_mask;
        else
            EHalf_physical_damage &= ~wp_mask;
    }
    if (spfx & SPFX_XRAY) {
        /* this assumes that no one else is using xray_range */
        if (on)
            u.xray_range = 3;
        else
            u.xray_range = -1;
        gv.vision_full_recalc = 1;
    }
    if (spfx & SPFX_REFLECT) {
        long had_refl = EReflecting & wp_mask;

        if (is_art(otmp, ART_MAGIC_MIRROR_OF_MERLIN)) {
            if (on) {
                EReflecting |= wp_mask;
                u.uconduct.reflection++;
            } else
                EReflecting &= ~wp_mask;
        } else if ((is_art(otmp, ART_LONGBOW_OF_DIANA)
                    || is_art(otmp, ART_CROSSBOW_OF_CARL))
                   && (wp_mask & W_WEP)) {
            /* wielding various reflecting artifacts */
            if (on) {
                EReflecting |= wp_mask;
                u.uconduct.reflection++;
            } else
                EReflecting &= ~wp_mask;
        } else if (is_art(otmp, ART_DRAGONBANE)
                   && (wp_mask & W_ARMG)) {
            /* or in Dragonbane's case, wear them */
            if (on) {
                EReflecting |= wp_mask;
                u.uconduct.reflection++;
            } else
                EReflecting &= ~wp_mask;
        } else if (is_art(otmp, ART_GAUNTLETS_OF_PURITY)
                   && (wp_mask & W_ARMG)) {
            /* same */
            if (on) {
                EReflecting |= wp_mask;
                u.uconduct.reflection++;
            } else
                EReflecting &= ~wp_mask;
        }
        if (had_refl && !(EReflecting & wp_mask))
            seenres_lost |= M_SEEN_REFL;
    }
    if (spfx & SPFX_PROTECT) {
        if (on)
            EProtection |= wp_mask;
        else
            EProtection &= ~wp_mask;
    }

    if (wp_mask == W_ART && !on && oart->inv_prop) {
        /* might have to turn off invoked power too */
        if (oart->inv_prop <= LAST_PROP
            && (u.uprops[oart->inv_prop].extrinsic & W_ARTI))
            (void) arti_invoke(otmp);
    }

    if (wp_mask == W_WEP && is_art(otmp, ART_SUNSWORD)) {
        if (on)
            EBlnd_resist |= wp_mask;
        else
            EBlnd_resist &= ~wp_mask;
    }

    monstunseesu(seenres_lost);
}

/* touch_artifact()'s return value isn't sufficient to tell whether it
   dished out damage, and tracking changes to u.uhp, u.mh, Lifesaved
   when trying to avoid second wounding is too cumbersome */
static boolean touch_blasted; /* for retouch_object() */

/*
 * creature (usually hero) tries to touch (pick up or wield) an artifact obj.
 * Returns 0 if the object refuses to be touched.
 * This routine does not change any object chains.
 * Ignores such things as gauntlets, assuming the artifact is not
 * fooled by such trappings.
 */
int
touch_artifact(struct obj *obj, struct monst *mon)
{
    const struct artifact *oart = get_artifact(obj);
    boolean badclass, badalign, self_willed, yours;

    touch_blasted = FALSE;
    if (oart == &artilist[ART_NONARTIFACT])
        return 1;

    yours = (mon == &gy.youmonst);
    /* all quest artifacts are self-willed; if this ever changes, `badclass'
       will have to be extended to explicitly include quest artifacts */
    self_willed = ((oart->spfx & SPFX_INTEL) != 0);
    if (yours) {
        u.uconduct.artitouch++;
        badclass = self_willed
                   && ((oart->role != NON_PM && !Role_if(oart->role))
                       || (oart->race != NON_PM && !Race_if(oart->race)));
        badalign = ((oart->spfx & SPFX_RESTR) != 0
                    && oart->alignment != A_NONE
                    && (oart->alignment != u.ualign.type
                        || u.ualign.record < 0));
    } else if (!(is_covetous(mon->data)
                 || is_mplayer(mon->data) || is_demon(mon->data))) {
        badclass = self_willed && oart->role != NON_PM
                   && oart != &artilist[ART_EXCALIBUR];
        badalign = (oart->spfx & SPFX_RESTR) && oart->alignment != A_NONE
                   && (oart->alignment != mon_aligntyp(mon));
    } else if (is_demon(mon->data)) {
        /* special case for demons as they are now all unaligned (evil).
           Lawful/neutral artifacts are off-limits */
        badclass = self_willed && oart->role != NON_PM
                   && oart != &artilist[ART_EXCALIBUR];
        badalign = (oart->spfx & SPFX_RESTR) && oart->alignment >= A_NEUTRAL
                   && mon_aligntyp(mon) == A_NONE;
    } else { /* an M3_WANTSxxx monster or a fake player */
        /* special monsters trying to take the Amulet, invocation tools or
           quest item can touch anything except `spec_applies' artifacts */
        badclass = badalign = FALSE;
    }
    /* weapons which attack specific categories of monsters are
       bad for them even if their alignments happen to match */
    if (!badalign)
        badalign = bane_applies(oart, mon);

    if (((badclass || badalign) && self_willed)
        || (badalign && (!yours || !rn2(4)))
        || (yours && oart == &artilist[ART_WAND_OF_ORCUS])) {
        int dmg;
        char buf[BUFSZ];
        unsigned oid = obj->o_id;
        boolean in_invent = (obj->where == OBJ_INVENT);

        if (!yours)
            return 0;
        You("are blasted by %s power!", s_suffix(the(xname(obj))));
        touch_blasted = TRUE;
        dmg = d((Antimagic ? 2 : 4), (self_willed ? 10 : 4));
        /* wearing the gauntlets of purity can harm the player
           even more if their alignment doesn't match */
        if (oart == &artilist[ART_GAUNTLETS_OF_PURITY]
            && oart->alignment != u.ualign.type)
            dmg += d((u.ualign.record > 50 ? 6 : 8), 10);
        Sprintf(buf, "touching %s", oart->name);
        losehp(dmg, buf, KILLED_BY); /* magic damage, not physical */
        /* returning to a natural form can make break_armor() drop the
           worn artifact, even into lava */
        if (in_invent && o_on(oid, gi.invent) != obj)
            return 0;
        exercise(A_WIS, FALSE);
    }

    /* can pick it up unless you're totally non-synch'd with the artifact */
    if (badclass && badalign && self_willed) {
        if (yours) {
            if (!carried(obj))
                pline("%s your grasp!", Tobjnam(obj, "evade"));
            else
                pline("%s beyond your control!", Tobjnam(obj, "are"));
        }
        if (!wizard || y_n("Override?") != 'y')
            return 0;
    }

    if (oart == &artilist[ART_CROSSBOW_OF_CARL]) {
        if (yours ? (Role_if(PM_RANGER) && (Race_if(PM_GNOME)
                                            || Race_if(PM_HOBBIT)))
                  : (racial_gnome(mon) || racial_hobbit(mon)))
            obj->owt = 24; /* magically lightened, same weight as
                              the Longbow of Diana */
    }

    return 1;
}

/* decide whether an artifact itself is vulnerable to a particular type
   of erosion damage, independent of the properties of its bearer */
boolean
arti_immune(struct obj *obj, int dtyp)
{
    const struct artifact *weap = get_artifact(obj);

    if (weap == &artilist[ART_NONARTIFACT])
        return FALSE;
    if (dtyp == AD_PHYS)
        return FALSE; /* nothing is immune to phys dmg */
    return (boolean) (weap->attk.adtyp == dtyp
                      || weap->defn.adtyp == dtyp
                      || weap->cary.adtyp == dtyp);
}

staticfn boolean
bane_applies(const struct artifact *oart, struct monst *mon)
{
    struct artifact atmp;
    boolean yours = (mon == &gy.youmonst);

    if (oart != &artilist[ART_NONARTIFACT]
            && (oart->spfx & SPFX_DBONUS) != 0) {
        /* special case so non-chaotics can wield it */
        if (oart == &artilist[ART_SWORD_OF_KAS])
            return FALSE;

        atmp = *oart;
        atmp.spfx &= SPFX_DBONUS; /* clear other spfx fields */
        if (spec_applies(&atmp, mon))
            return TRUE;
    }

    /* drow and the undead may not handle sacred artifacts */
    if (yours) {
        if (maybe_polyd(is_drow(gy.youmonst.data), Race_if(PM_DROW))
            && (oart == &artilist[ART_SUNSWORD]
                || oart == &artilist[ART_HAMMER_OF_THE_GODS])) {
            if (!wizard || y_n("Override?") != 'y')
                return TRUE;
        } else if ((maybe_polyd(is_undead(gy.youmonst.data),
                                Race_if(PM_DRAUGR))
                    || maybe_polyd(is_undead(gy.youmonst.data),
                                   Race_if(PM_VAMPIRE)))
            && (oart == &artilist[ART_SUNSWORD]
                || oart == &artilist[ART_HAMMER_OF_THE_GODS]
                || oart == &artilist[ART_MITRE_OF_HOLINESS])) {
            if (!wizard || y_n("Override?") != 'y')
                return TRUE;
        } else if (is_demon(raceptr(&gy.youmonst))
            && (oart == &artilist[ART_DEMONBANE]
                || oart == &artilist[ART_HAMMER_OF_THE_GODS])) {
            if (!wizard || y_n("Override?") != 'y')
                return TRUE;
        }
    } else {
        if (racial_drow(mon)
            && (oart == &artilist[ART_SUNSWORD]
                || oart == &artilist[ART_HAMMER_OF_THE_GODS]
                /* SotA does not accommodate monster drow */
                || oart == &artilist[ART_STAFF_OF_THE_ARCHMAGI])) {
            return TRUE;
        } else if ((racial_zombie(mon) || racial_vampire(mon))
            && (oart == &artilist[ART_SUNSWORD]
                || oart == &artilist[ART_HAMMER_OF_THE_GODS]
                || oart == &artilist[ART_MITRE_OF_HOLINESS])) {
            return TRUE;
        }
    }

    return FALSE;
}

/* decide whether an artifact's special attacks apply against mtmp */
staticfn int
spec_applies(const struct artifact *weap, struct monst *mtmp)
{
    struct permonst *ptr;
    boolean yours;

    if (!(weap->spfx & (SPFX_DBONUS | SPFX_ATTK)))
        return (weap->attk.adtyp == AD_PHYS);

    yours = (mtmp == &gy.youmonst);
    ptr = mtmp->data;

    if (weap->spfx & SPFX_DMONS) {
        return (ptr == &mons[(int) weap->mtype]);
    } else if (weap->spfx & SPFX_DCLAS) {
        return (weap->mtype == (unsigned long) ptr->mlet
                || (has_erac(mtmp)
                    && weap->mtype
                       == (unsigned long) safe_mons(ERAC(mtmp)->rmnum).mlet));
    } else if (weap->spfx & SPFX_DFLAG1) {
        return ((has_erac(mtmp) && (ERAC(mtmp)->mflags1 & weap->mtype))
                || (ptr->mflags1 & weap->mtype) != 0L);
    } else if (weap->spfx & SPFX_DFLAGH) {
        return ((has_erac(mtmp) && (ERAC(mtmp)->mrace & weap->mtype))
                || (!has_erac(mtmp) && (ptr->mhflags & weap->mtype))
                || (yours
                    && ((!Upolyd && (gu.urace.selfmask & weap->mtype))
                        || ((weap->mtype & MH_WERE) && ismnum(u.ulycn)))));
    } else if (weap->spfx & SPFX_DALIGN) {
        return yours ? (u.ualign.type != weap->alignment)
                     : (ptr->maligntyp == A_NONE
                        || sgn(ptr->maligntyp) != weap->alignment);
    } else if (weap->spfx & SPFX_ATTK) {
        if (defended(mtmp, (int) weap->attk.adtyp))
            return FALSE;

        switch (weap->attk.adtyp) {
        case AD_FIRE:
            return !(yours ? ((how_resistant(FIRE_RES) > 99) || Underwater)
                           : (resists_fire(mtmp) || mon_underwater(mtmp)));
        case AD_COLD:
            return !(yours ? (how_resistant(COLD_RES) > 99)
                           : resists_cold(mtmp));
        case AD_ELEC:
            return !(yours ? (how_resistant(SHOCK_RES) > 99)
                           : resists_elec(mtmp));
        case AD_MAGM:
        case AD_STUN:
            return !(yours ? Antimagic : (rn2(100) < ptr->mr));
        case AD_DRST:
            return !(yours ? (how_resistant(POISON_RES) > 99)
                           : resists_poison(mtmp));
        case AD_DRLI:
            return !(yours ? Drain_resistance : resists_drli(mtmp));
        case AD_DREN:
            return !nonliving(ptr);
        case AD_STON:
            return !(yours ? Stone_resistance : resists_ston(mtmp));
        case AD_ACID:
            return !(yours ? (Acid_resistance || Underwater)
                           : (resists_acid(mtmp) || mon_underwater(mtmp)));
        case AD_DISE:
            return !(yours ? Sick_resistance : resists_sick(mtmp));
        case AD_DETH:
            return !(yours ? Death_resistance : immune_death_magic(ptr));
        case AD_DISN:
            return !(yours ? Disint_resistance : resists_disint(mtmp));
        case AD_FUSE:
            return !(yours ? (((how_resistant(FIRE_RES) > 99) || Underwater)
                              && (how_resistant(COLD_RES) > 99))
                           : ((resists_fire(mtmp) || mon_underwater(mtmp)
                               || defended(mtmp, AD_FIRE))
                              && (resists_cold(mtmp)
                                  || defended(mtmp, AD_COLD))));
        default:
            impossible("Weird weapon special attack.");
        }
    }
    return 0;
}

/* return the MH flags of monster that an artifact's special attacks apply
 * against */
long
spec_mh(struct obj *otmp)
{
    const struct artifact *artifact = get_artifact(otmp);

    if (artifact != &artilist[ART_NONARTIFACT])
        return artifact->mtype;
    return 0L;
}

/* special attack bonus */
int
spec_abon(struct obj *otmp, struct monst *mon)
{
    const struct artifact *weap = get_artifact(otmp);

    /* no need for an extra check for `NO_ATTK' because this will
       always return 0 for any artifact which has that attribute */

    if (weap != &artilist[ART_NONARTIFACT]
            && weap->attk.damn && spec_applies(weap, mon))
        return rnd((int) weap->attk.damn);
    return 0;
}

/* special damage bonus */
int
spec_dbon(struct obj *otmp, struct monst *mon, int tmp)
{
    const struct artifact *weap = get_artifact(otmp);
    boolean yours = (mon == &gy.youmonst);
    int dbon, adtype = 0;

    if (weap == &artilist[ART_NONARTIFACT] && otmp->oprops
        && (otmp->oclass == WEAPON_CLASS
            || is_weptool(otmp) || is_bullet(otmp)
            || (uarms && otmp == uarms)
            || (uarmg && otmp == uarmg))) {
        gs.spec_dbon_applies = FALSE;
        if ((attacks(adtype = AD_FIRE, otmp)
             && (yours ? !(Fire_resistance || Underwater)
                       : !(resists_fire(mon) || defended(mon, AD_FIRE)
                           || mon_underwater(mon))))
            || (attacks(adtype = AD_COLD, otmp)
                && (yours ? !Cold_resistance
                          : !(resists_cold(mon)
                              || defended(mon, AD_COLD))))
            || (attacks(adtype = AD_ELEC, otmp)
                && (yours ? !Shock_resistance
                          : !(resists_elec(mon)
                              || defended(mon, AD_ELEC))))
            || (attacks(adtype = AD_DRST, otmp)
                && (yours ? !Poison_resistance
                          : !(resists_poison(mon)
                              || defended(mon, AD_DRST))))
            || (attacks(adtype = AD_ACID, otmp)
                && (yours ? !(Acid_resistance || Underwater)
                          : !(resists_acid(mon) || defended(mon, AD_ACID)
                              || mon_underwater(mon))))
            || (attacks(adtype = AD_DISE, otmp)
                && (yours ? !Sick_resistance
                          : !(resists_sick(mon)
                              || defended(mon, AD_DISE))))) {
            gs.spec_dbon_applies = TRUE;
            /* the majority of ITEM_VENOM damage is handled in
               the poisoned-hit code */
            if (otmp->oprops & ITEM_VENOM)
                return rnd(2);
            dbon = rnd(5) + 3;
            if (vulnerable_to(mon, adtype))
                dbon = ((3 * dbon) + 1) / 2;
            return dbon;
        }
        if ((otmp->oprops & ITEM_DRLI)
            && (yours ? !Drain_resistance
                      : !(resists_drli(mon) || defended(mon, AD_DRLI)))) {
            gs.spec_dbon_applies = TRUE;
            return 0;
        }
        return 0;
    }

    if ((weap == &artilist[ART_NONARTIFACT])
        || (weap->attk.adtyp == AD_PHYS /* check for `NO_ATTK' */
                  && weap->attk.damn == 0 && weap->attk.damd == 0))
        gs.spec_dbon_applies = FALSE;
    else if ((is_art(otmp, ART_GRIMTOOTH)
              && !(yours ? Sick_resistance
                         : (resists_sick(mon) || defended(mon, AD_DISE))))
             || is_art(otmp, ART_VORPAL_BLADE)
             || (is_art(otmp, ART_ANGELSLAYER)
                 && !(yours ? is_demon(raceptr(&gy.youmonst))
                            : is_demon(mon->data)))
             || (is_art(otmp, ART_HARBINGER)
                 && !(yours ? Acid_resistance
                            : (resists_acid(mon) || defended(mon, AD_ACID))))
             || is_art(otmp, ART_DRAMBORLEG))
        /* Grimtooth, Vorpal Blade, Angelslayer, Harbinger, and
           Dramborleg have SPFX settings to warn against elves,
           jabberwocks, angels, giants, and demons respectively,
           but we want their damage bonuses to apply to more targets,
           so bypass spec_applies() */
        gs.spec_dbon_applies = TRUE;
    else
        gs.spec_dbon_applies = spec_applies(weap, mon);

    if (gs.spec_dbon_applies) {
        dbon = weap->attk.damd ? rnd((int) weap->attk.damd) : max(tmp, 1);
        /* we want to possibly increase dbon, not the whole attack's
           damage, since only dbon is elemental */
        if (vulnerable_to(mon, weap->attk.adtyp))
            dbon = ((3 * dbon) + 1) / 2;
        /* damage bonus doubles if Dichotomy hits a target that resists
           neither fire nor cold */
        if (is_art(otmp, ART_DICHOTOMY)
            && (yours ? (!(Fire_resistance || Underwater)
                         && !Cold_resistance)
                      : (!(resists_fire(mon) || defended(mon, AD_FIRE)
                           || mon_underwater(mon))
                         && !(resists_cold(mon)
                              || defended(mon, AD_COLD)))))
            dbon = (2 * dbon) + 1;
        /* hellfire is mitigated by fire resistance */
        if (is_art(otmp, ART_ANGELSLAYER)
            && (yours ? Fire_resistance
                      : (resists_fire(mon) || defended(mon, AD_FIRE))))
            dbon = (dbon + 1) / 2;
        return dbon;
    }
    return 0;
}

/* Estimate of expected per-hit damage for an artifact weapon
   against a target. Used by find_artifact_recurse() in weapon.c to
   rank multiple artifacts in a monster's inventory. Direct rnd(N)
   damage terms are replaced by the closed-form mean (N+1)/2, but
   the estimate is not fully deterministic: spec_applies() rolls
   against magic resistance for some attack types and
   monhp_per_lvl() rolls a die, so repeat evaluations of the same
   weapon can differ. Mirrors spec_dbon() gating without side
   effects (gs.spec_dbon_applies is intentionally not touched);
   keep the special cases here in sync with spec_dbon() and
   artifact_hit() */
int
score_artifact_weapon(
    struct monst *magr UNUSED, /* attacker, reserved for future use */
    struct obj *otmp,
    struct monst *mdef)
{
    const struct artifact *weap;
    int score = 0, base_dmg, dbon, hp;
    boolean yours, applies = FALSE;

    if (!otmp)
        return 0;
    if (!mdef)
        mdef = &gy.youmonst;
    yours = (mdef == &gy.youmonst);

    /* base 1d(weapon damage), expected value (N+1)/2;
       r_bigmonst() picks oc_wldam vs oc_wsdam to match dmgval() */
    base_dmg = r_bigmonst(mdef)
                   ? (objects[otmp->otyp].oc_wldam + 1) / 2
                   : (objects[otmp->otyp].oc_wsdam + 1) / 2;
    score += base_dmg;

    /* enchantment, signed */
    score += otmp->spe;

    weap = get_artifact(otmp);
    if (weap == &artilist[ART_NONARTIFACT])
        return score;

    /* mirror spec_dbon() gating without rnd() or side effects */
    if (weap->attk.adtyp == AD_PHYS
        && weap->attk.damn == 0 && weap->attk.damd == 0)
        applies = FALSE;
    else if (is_art(otmp, ART_GRIMTOOTH))
        /* spec_dbon() bypasses spec_applies() for Grimtooth */
        applies = !(yours ? Sick_resistance
                          : (resists_sick(mdef)
                             || defended(mdef, AD_DISE)));
    else if (is_art(otmp, ART_VORPAL_BLADE) || is_art(otmp, ART_DRAMBORLEG))
        applies = TRUE;
    else if (is_art(otmp, ART_ANGELSLAYER))
        applies = !(yours ? is_demon(raceptr(&gy.youmonst))
                          : is_demon(mdef->data));
    else if (is_art(otmp, ART_HARBINGER))
        applies = !(yours ? Acid_resistance
                          : (resists_acid(mdef)
                             || defended(mdef, AD_ACID)));
    else
        applies = (spec_applies(weap, mdef) != 0);

    if (applies) {
        /* when damd is 0, spec_dbon() returns max(tmp, 1) where tmp
           is the base weapon damage roll - i.e. it doubles the base
           hit; mirror that closed form using the average base damage */
        dbon = weap->attk.damd
                   ? (weap->attk.damd + 1) / 2
                   : (base_dmg > 0 ? base_dmg : 1);
        if (vulnerable_to(mdef, weap->attk.adtyp))
            dbon = ((3 * dbon) + 1) / 2;
        /* Dichotomy doubles vs targets that resist neither fire
           nor cold */
        if (is_art(otmp, ART_DICHOTOMY)
            && (yours ? (!(Fire_resistance || Underwater)
                         && !Cold_resistance)
                      : (!(resists_fire(mdef) || defended(mdef, AD_FIRE)
                           || mon_underwater(mdef))
                         && !(resists_cold(mdef)
                              || defended(mdef, AD_COLD)))))
            dbon = (2 * dbon) + 1;
        /* Angelslayer's hellfire is halved by fire resistance */
        if (is_art(otmp, ART_ANGELSLAYER)
            && (yours ? Fire_resistance
                      : (resists_fire(mdef) || defended(mdef, AD_FIRE))))
            dbon = (dbon + 1) / 2;
        score += dbon;

        /* per-artifact extras that aren't part of spec_dbon() but
           fire whenever spec_dbon() would apply; Mb_hit adds probe
           (always +1d4), plus 50% stun (+1d4) and rare scare/cancel
           (+1d4 each), and Tempest has a 1/6 chance of a d(6,6) AOE
           lightning surge - each roughly +4 expected per swing */
        switch (otmp->oartifact) {
        case ART_MAGICBANE:
        case ART_BUTCHER:
        case ART_STAFF_OF_THE_ARCHMAGI:
            score += 4;
            break;
        case ART_TEMPEST:
            score += 4; /* (1/6) * d(6,6) avg ~3.5 */
            break;
        default:
            break;
        }
    }

    /* save-or-die expected value, scaled by target HP */
    hp = yours ? (Upolyd ? u.mhmax : u.uhpmax) : mdef->mhpmax;
    if (hp < 1)
        hp = 1;
    switch (otmp->oartifact) {
    case ART_VORPAL_BLADE:
        if (has_head(mdef->data) && !noncorporeal(mdef->data)
            && !amorphous(mdef->data))
            score += hp / 20;
        break;
    case ART_TSURUGI_OF_MURAMASA:
        score += hp / 20;
        break;
    case ART_WAND_OF_ORCUS:
        if (!immune_death_magic(mdef->data))
            score += (hp * 3) / 20;
        break;
    case ART_GIANTSLAYER:
    case ART_HARBINGER:
        if (is_giant(mdef->data))
            score += hp / 10;
        break;
    case ART_ANGELSLAYER:
        if (is_angel(mdef->data) || is_aasimar(mdef->data))
            score += hp / 10;
        break;
    case ART_SWORD_OF_ANNIHILATION:
        /* artifact_hit() instakills with 1/12 probability against
           non-resistant non-uniques and 1/100 against uniques */
        if (!(yours ? Disint_resistance
                    : (resists_disint(mdef)
                       || defended(mdef, AD_DISN))))
            score += hp / ((!yours && (mdef->data->geno & G_UNIQ))
                               ? 100 : 12);
        break;
    default:
        break;
    }

    /* expected level-drain damage for AD_DRLI artifacts */
    if ((attacks(AD_DRLI, otmp) || spec_ability(otmp, SPFX_DRLI))
        && !(yours ? Drain_resistance
                   : (resists_drli(mdef)
                      || defended(mdef, AD_DRLI)
                      || nonliving(mdef->data))))
        score += monhp_per_lvl(mdef);

    /* expected searing damage when the target hates this weapon's
       material (silver vs demons and vampires, iron vs elves, and
       so on); mirrors the dmgval() bonus in weapon.c using the
       closed-form mean of rnd(sear_damage(material)) */
    if (mon_hates_material(mdef, otmp->material)
        && !(has_barkskin(mdef) || has_stoneskin(mdef))
        && !(yours && (Barkskin || Stoneskin)))
        score += (otmp->material == SILVER) ? 10 : 3;

    return score;
}

/* add identified artifact to discoveries list */
void
discover_artifact(xint16 m)
{
    int i;

    /* look for this artifact in the discoveries list;
       if we hit an empty slot then it's not present, so add it */
    for (i = 0; i < NROFARTIFACTS; i++)
        if (artidisco[i] == 0 || artidisco[i] == m) {
            artidisco[i] = m;
            return;
        }
    /* there is one slot per artifact, so we should never reach the
       end without either finding the artifact or an empty slot... */
    impossible("couldn't discover artifact (%d)", (int) m);
}

/* used to decide whether an artifact has been fully identified */
boolean
undiscovered_artifact(xint16 m)
{
    int i;

    /* look for this artifact in the discoveries list;
       if we hit an empty slot then it's undiscovered */
    for (i = 0; i < NROFARTIFACTS; i++)
        if (artidisco[i] == m)
            return FALSE;
        else if (artidisco[i] == 0)
            break;
    return TRUE;
}

/* display a list of discovered artifacts; return their count */
int
disp_artifact_discoveries(
    winid tmpwin) /* supplied by dodiscover(); type is NHW_TEXT */
{
    int i, m, otyp;
    const char *algnstr;
    char buf[BUFSZ];

    for (i = 0; i < NROFARTIFACTS; i++) {
        if (artidisco[i] == 0)
            break; /* empty slot implies end of list */
        if (tmpwin == WIN_ERR)
            continue; /* for WIN_ERR, we just count */

        if (i == 0)
            putstr(tmpwin, iflags.menu_headings.attr, "Artifacts");
        m = artidisco[i];
        otyp = artilist[m].otyp;
        algnstr = align_str(artilist[m].alignment);
        if (!strcmp(algnstr, "unaligned"))
            algnstr = "non-aligned";

        Sprintf(buf, "  %s [%s %s]", artiname(m),
                algnstr, simple_typename(otyp));
        putstr(tmpwin, 0, buf);
    }
    return i;
}

/* (wizard mode only) show all artifacts and their flags */
void
dump_artifact_info(winid tmpwin)
{
    int m;
    char buf[BUFSZ], buf2[BUFSZ];

    /* not a menu, but header uses same bold or whatever attribute as such */
    putstr(tmpwin, iflags.menu_headings.attr, "Artifacts");
    for (m = 1; m <= NROFARTIFACTS; ++m) {
        Snprintf(buf2, sizeof buf2,
                "[%s%s%s%s%s%s%s%s%s]", /* 9 bits overall */
                artiexist[m].exists ? "exists;" : "",
                artiexist[m].found  ? " hero knows;" : "",
                /* .exists and .found have different punctuation because
                   they're expected to be combined with one of these */
                artiexist[m].gift   ? " gift"   : "",
                artiexist[m].wish   ? " wish"   : "",
                artiexist[m].named  ? " named"  : "",
                artiexist[m].viadip ? " viadip" : "",
                artiexist[m].lvldef ? " lvldef" : "",
                artiexist[m].bones  ? " bones"  : "",
                artiexist[m].rndm   ? " random" : "");
#if 0   /* 'tmpwin' here is a text window, not a menu */
        if (iflags.menu_tab_sep)
            Sprintf(buf, "  %s\t%s", artiname(m), buf2);
        else
#else
            /* "The Platinum Yendorian Express Card" is 35 characters */
            Snprintf(buf, sizeof buf, "  %-36.36s%s", artiname(m), buf2);
#endif
        putstr(tmpwin, 0, buf);
    }
    return;
}

/*
 * Magicbane's intrinsic magic is incompatible with normal
 * enchantment magic.  Thus, its effects have a negative
 * dependence on spe.  Against low mr victims, it typically
 * does "double athame" damage, 2d4.  Occasionally, it will
 * cast unbalancing magic which effectively averages out to
 * 4d4 damage (3d4 against high mr victims), for spe = 0.
 *
 * Prior to 3.4.1, the cancel (aka purge) effect always
 * included the scare effect too; now it's one or the other.
 * Likewise, the stun effect won't be combined with either
 * of those two; it will be chosen separately or possibly
 * used as a fallback when scare or cancel fails.
 *
 * [Historical note: a change to artifact_hit() for 3.4.0
 * unintentionally made all of Magicbane's special effects
 * be blocked if the defender successfully saved against a
 * stun attack.  As of 3.4.1, those effects can occur but
 * will be slightly less likely than they were in 3.3.x.]
 */

enum mb_effect_indices {
    MB_INDEX_PROBE = 0,
    MB_INDEX_STUN,
    MB_INDEX_SCARE,
    MB_INDEX_CANCEL,

    NUM_MB_INDICES
};

#define MB_MAX_DIEROLL 8 /* rolls above this aren't magical */
static const char *const mb_verb[2][NUM_MB_INDICES] = {
    { "probe", "stun", "scare", "cancel" },
    { "prod", "amaze", "tickle", "purge" },
};

/* called when someone is being hit by Magicbane */
staticfn boolean
Mb_hit(struct monst *magr, /* attacker */
       struct monst *mdef, /* defender */
       struct obj *mb,     /* Magicbane */
       int *dmgptr,        /* extra damage target will suffer */
       int dieroll,        /* d20 that has already scored a hit */
       boolean vis,        /* whether the action can be seen */
       char *hittee)       /* target's name: "you" or mon_nam(mdef) */
{
    struct permonst *old_mdat;
    const char *verb;
    boolean youattack = (magr == &gy.youmonst),
            youdefend = (mdef == &gy.youmonst),
            resisted = FALSE, do_stun, do_confuse, result;
    int attack_indx, fakeidx, scare_dieroll = MB_MAX_DIEROLL / 2;

    result = FALSE; /* no message given yet */
    /* the most severe effects are less likely at higher enchantment */
    if (mb->spe >= 3)
        scare_dieroll /= (1 << (mb->spe / 3));
    /* if target successfully resisted the artifact damage bonus,
       reduce overall likelihood of the assorted special effects */
    if (!gs.spec_dbon_applies)
        dieroll += 1;

    /* might stun even when attempting a more severe effect, but
       in that case it will only happen if the other effect fails;
       extra damage will apply regardless; 3.4.1: sometimes might
       just probe even when it hasn't been enchanted */
    do_stun = (max(mb->spe, 0) < rn2(gs.spec_dbon_applies ? 11 : 7));

    /* the special effects also boost physical damage; increments are
       generally cumulative, but since the stun effect is based on a
       different criterium its damage might not be included; the base
       damage is either 1d4 (athame) or 2d4 (athame+spec_dbon) depending
       on target's resistance check against AD_STUN (handled by caller)
       [note that a successful save against AD_STUN doesn't actually
       prevent the target from ending up stunned] */
    attack_indx = MB_INDEX_PROBE;
    *dmgptr += rnd(4); /* (2..3)d4 */
    if (do_stun) {
        attack_indx = MB_INDEX_STUN;
        *dmgptr += rnd(4); /* (3..4)d4 */
    }
    /* mindless and unique monsters can't be scared */
    if (dieroll <= scare_dieroll
        && !mindless(mdef->data) && !unique_corpstat(mdef->data)) {
        attack_indx = MB_INDEX_SCARE;
        *dmgptr += rnd(4); /* (3..5)d4 */
    }
    if (dieroll <= (scare_dieroll / 2)) {
        attack_indx = MB_INDEX_CANCEL;
        *dmgptr += rnd(4); /* (4..6)d4 */
    }

    /* give the hit message prior to inflicting the effects */
    verb = mb_verb[!!Hallucination][attack_indx];
    if (youattack || youdefend || vis) {
        result = TRUE;
        pline_The("magic-absorbing blade %s %s!",
                  vtense((const char *) 0, verb), hittee);
        /* assume probing has some sort of noticeable feedback
           even if it is being done by one monster to another */
        if (attack_indx == MB_INDEX_PROBE && !canspotmon(mdef))
            map_invisible(mdef->mx, mdef->my);
    }

    /* now perform special effects */
    switch (attack_indx) {
    case MB_INDEX_CANCEL:
        old_mdat = youdefend ? gy.youmonst.data : mdef->data;
        /* No mdef->mcan check: even a cancelled monster can be polymorphed
         * into a golem, and the "cancel" effect acts as if some magical
         * energy remains in spellcasting defenders to be absorbed later.
         */
        if (!cancel_monst(mdef, mb, youattack, FALSE, FALSE)) {
            resisted = TRUE;
        } else {
            do_stun = FALSE;
            if (youdefend) {
                if (gy.youmonst.data != old_mdat)
                    *dmgptr = 0; /* rehumanized, so no more damage */
                if (u.uenmax > 0) {
                    u.uenmax--;
                    if (u.uen > 0)
                        u.uen--;
                    disp.botl = TRUE;
                    You("lose magical energy!");
                }
            } else if (!DEADMONSTER(mdef)) {
                /* the cancel itself can prove fatal (a forced form
                   change into a handless polymorph sheds cockatrice
                   gloves and mselftouch petrifies); skip post-cancel
                   state writes through the dead struct.
                   A canceled shapeshifter/vamp may have changed
                   forms, so update its name if necessary */
                if (mdef->data != old_mdat)
                    Strcpy(hittee, mon_nam(mdef));
                if (mdef->data == &mons[PM_CLAY_GOLEM])
                    mdef->mhp = 1; /* cancelled clay golems will die */
                if (youattack && attacktype(mdef->data, AT_MAGC)) {
                    u.uenmax++;
                    if (u.uenmax > u.uenpeak)
                        u.uenpeak = u.uenmax;
                    u.uen++;
                    disp.botl = TRUE;
                    You("absorb magical energy!");
                }
            }
        }
        break;

    case MB_INDEX_SCARE:
        if (youdefend) {
            if (Antimagic) {
                resisted = TRUE;
            } else {
                nomul(-3);
                gm.multi_reason = "being scared stiff";
                gn.nomovemsg = "";
                if (magr && magr == u.ustuck && sticks(gy.youmonst.data)) {
                    set_ustuck((struct monst *) 0);
                    You("release %s!", mon_nam(magr));
                }
            }
        } else {
            if (rn2(2) && resist(mdef, WEAPON_CLASS, 0, NOTELL))
                resisted = TRUE;
            else
                monflee(mdef, 3, FALSE, (mdef->mhp > *dmgptr));
        }
        if (!resisted)
            do_stun = FALSE;
        break;

    case MB_INDEX_STUN:
        do_stun = TRUE; /* (this is redundant...) */
        break;

    case MB_INDEX_PROBE:
        if (youattack && (mb->spe == 0 || !rn2(3 * abs(mb->spe)))) {
            pline_The("%s is insightful.", verb);
            /* pre-damage status */
            probe_monster(mdef);
        }
        break;
    }
    /* stun if that was selected and a worse effect didn't occur */
    if (do_stun) {
        if (youdefend) {
            make_stunned(((HStun & TIMEOUT) + 3L), FALSE);
        } else if (!DEADMONSTER(mdef)) {
            if (!(mon_resists_stun(mdef) || defended(mdef, AD_STUN)
                  || mon_wielding_artifact(mdef, ART_TEMPEST)))
                mdef->mstun = 1;
        }
        /* avoid extra stun message below if we used mb_verb["stun"] above */
        if (attack_indx == MB_INDEX_STUN)
            do_stun = FALSE;
    }
    /* lastly, all this magic can be confusing... */
    do_confuse = !rn2(12);
    if (do_confuse) {
        if (youdefend)
            make_confused((HConfusion & TIMEOUT) + 4L, FALSE);
        else
            mdef->mconf = 1;
    }

    /* now give message(s) describing side-effects; Use fakename
       so vtense() won't be fooled by assigned name ending in 's' */
    fakeidx = youdefend ? 1 : 0;
    if (youattack || youdefend || vis) {
        (void) upstart(hittee); /* capitalize */
        if (resisted) {
            pline("%s %s!", hittee, vtense(fakename[fakeidx], "resist"));
            shieldeff(youdefend ? u.ux : mdef->mx,
                      youdefend ? u.uy : mdef->my);
        }
        if ((do_stun || do_confuse) && flags.verbose) {
            char buf[BUFSZ];

            buf[0] = '\0';
            if (do_stun)
                Strcat(buf, "stunned");
            if (do_stun && do_confuse)
                Strcat(buf, " and ");
            if (do_confuse)
                Strcat(buf, "confused");
            pline("%s %s %s%c", hittee, vtense(fakename[fakeidx], "are"), buf,
                  (do_stun && do_confuse) ? '!' : '.');
        }
    }

    return result;
}

DISABLE_WARNING_FORMAT_NONLITERAL

/* TRUE if a peaceful or tame monster (other than 'skip') - or a tame
   steed the hero is sitting on - stands within the 3x3 area explode()
   would affect when centered on <x,y>. The Tempest / Harbinger /
   Dichotomy surge routines use this to rein in the bonus blast around
   friendlies when the hero attacks with a blessed artifact and is at
   least skilled in its base weapon, so a lucky proc on a hostile target
   can't anger, untame, or kill an innocent bystander */
staticfn boolean
friendly_in_blast(coordxy x, coordxy y, struct monst *skip)
{
    coordxy dx, dy;
    struct monst *mtmp;

    for (dx = x - 1; dx < x + 2; ++dx) {
        for (dy = y - 1; dy < y + 2; ++dy) {
            if (!isok(dx, dy))
                continue;
            mtmp = m_at(dx, dy);
            if (!mtmp && u_at(dx, dy))
                mtmp = u.usteed; /* mirror explode()'s steed pick */
            if (!mtmp || mtmp == skip || DEADMONSTER(mtmp))
                continue;
            if (mtmp->mpeaceful || mtmp->mtame)
                return TRUE;
        }
    }
    return FALSE;
}

/* Function used when someone attacks someone else with an artifact
 * weapon.  Only adds the special (artifact) damage, and returns a 1 if it
 * did something special (in which case the caller won't print the normal
 * hit message).  This should be called once upon every artifact attack;
 * dmgval() no longer takes artifact bonuses into account.  Possible
 * extension: change the killer so that when an orc kills you with
 * Stormbringer it's "killed by Stormbringer" instead of "killed by an orc".
 */
boolean
artifact_hit(
    struct monst *magr, /* attacker; might be Null if 'mdef' is youmonst */
    struct monst *mdef, /* defender */
    struct obj *otmp,   /* artifact weapon */
    int *dmgptr,        /* output */
    int dieroll)        /* needed for Magicbane and vorpal blades */
{
    boolean youattack = (magr == &gy.youmonst);
    boolean youdefend = (mdef == &gy.youmonst);
    boolean vis = (!youattack && magr && cansee(magr->mx, magr->my))
                  || (!youdefend && cansee(mdef->mx, mdef->my))
                  || (youattack && engulfing_u(mdef) && !Blind);
    /* used to determine if an instakill message should be shown;
       currently consistent with behavior of other instakill weapons,
       but not realizes_damage */
    boolean show_instakill = (youattack || youdefend || vis);
    boolean realizes_damage; /* player can see or feel the hit;
                              * gates messages only */
    boolean def_underwater;
    const char *wepdesc;
    const struct artifact *atmp;
    static const char you[] = "you";
    char hittee[BUFSZ];
    int hurtle_distance;

    Strcpy(hittee, youdefend ? you : mon_nam(mdef));

    /* The following takes care of most of the damage, but not all--
     * the exception being for level draining, which is specially
     * handled.  Messages are done in this function, however.
     */
    *dmgptr += spec_dbon(otmp, mdef, *dmgptr);

    if (youattack && youdefend) {
        impossible("attacking yourself with weapon?");
        return FALSE;
    }

    /* Ogresmasher has a damage bonus against ogres, but it can also
     * hurtle light-weight monsters, so the spec_dbon_applies check
     * is insufficient for its effects. Hurtle distance is used as an
     * additional check for its knockback behavior for MZ_MEDIUM and
     * smaller targets. Knockback rate is set here as well in rn2(rate).
     * Knockback will not trigger if Ogresmasher is cursed.
     */
    hurtle_distance = 0;

    if (is_art(otmp, ART_OGRESMASHER) && !otmp->cursed
        && mdef->data->msize < MZ_LARGE && !rn2(5))
        hurtle_distance = MZ_LARGE - mdef->data->msize;

    /* incorporeal monsters are immune to various types of damage */
    if (noncorporeal(mdef->data)
        && !(shade_glare(otmp) || gs.spec_dbon_applies))
        return FALSE;

    realizes_damage = (youdefend || vis
                       /* feel the effect even if not seen */
                       || (youattack && mdef == u.ustuck));

    def_underwater = youdefend ? Underwater : mon_underwater(mdef);

    /* the basic attacks: fire, cold, shock, missiles, and so on */
    if (attacks(AD_FIRE, otmp)) {
        boolean angel, aasimar;

        if (realizes_damage) {
            if (is_art(otmp, ART_FIRE_BRAND)) {
                pline_The("fiery blade %s %s%c",
                          !gs.spec_dbon_applies
                              ? "hits"
                              : can_vaporize(mdef->data)
                                  ? "vaporizes part of"
                                  : def_underwater
                                      ? "hits"
                                      : vulnerable_to(mdef, AD_FIRE)
                                          ? "severely burns" : "burns",
                          hittee, !gs.spec_dbon_applies ? '.' : '!');
            } else if (is_art(otmp, ART_XIUHCOATL)) {
                pline_The("flaming spear %s %s%c",
                          !gs.spec_dbon_applies
                              ? "hits"
                              : can_vaporize(mdef->data)
                                  ? "vaporizes part of"
                                  : def_underwater
                                      ? "hits"
                                      : vulnerable_to(mdef, AD_FIRE)
                                          ? "severely burns" : "burns",
                          hittee, !gs.spec_dbon_applies ? '.' : '!');
            } else if (is_art(otmp, ART_ANGELSLAYER)) {
                pline_The("infernal trident %s %s%c",
                          !gs.spec_dbon_applies
                              ? "hits"
                              : can_vaporize(mdef->data)
                                  ? "vaporizes part of"
                                  : vulnerable_to(mdef, AD_FIRE)
                                      ? "severely burns" : "burns",
                          hittee, !gs.spec_dbon_applies ? '.' : '!');
            } else if ((otmp->oclass == WEAPON_CLASS || is_weptool(otmp)
                        || is_bullet(otmp) || otmp == uarms
                        || otmp == uarmg)
                       && (otmp->oprops & ITEM_FIRE)) {
                pline_The("%s %s %s%c",
                          otmp->oclass == WEAPON_CLASS
                              ? distant_name(otmp, xname)
                              : simpleonames(otmp),
                          !gs.spec_dbon_applies
                              ? "hits"
                              : can_vaporize(mdef->data)
                                  ? "vaporizes part of"
                                  : def_underwater
                                      ? "hits"
                                      : vulnerable_to(mdef, AD_FIRE)
                                          ? "severely burns" : "burns",
                          hittee, !gs.spec_dbon_applies ? '.' : '!');
            }
        }
        if ((otmp->oprops & ITEM_FIRE) && gs.spec_dbon_applies) {
            otmp->oprops_known |= ITEM_FIRE;
            update_inventory();
        }
        if (!def_underwater || is_art(otmp, ART_ANGELSLAYER)) {
            if (!rn2(4)) {
                int itemdmg = destroy_items(mdef, AD_FIRE, *dmgptr);

                if (!youdefend)
                    *dmgptr += itemdmg; /* item destruction dmg */
                ignite_items(mdef->minvent);
            }
            if (youdefend && Slimed)
                burn_away_slime();
            /* NB: if youdefend, then mdef == &gy.youmonst */
            if (completelyburns(mdef->data)
                || mdef->data == &mons[PM_WOOD_GOLEM]
                || mdef->data == &mons[PM_GREEN_SLIME]
                || (is_blight(mdef->data) && !rn2(10))
                || (mdef->data == &mons[PM_ENT] && !rn2(10))
                || (mdef->data == &mons[PM_ELDER_ENT] && !rn2(12))) {
                if (youdefend) {
                    int loss = (Upolyd ? u.mh : u.uhp) + 1;

                    You("ignite and turn to ash!");
                    /* the wielder's kill, with nothing left to rise */
                    set_monster_killer_hp(magr, loss, FALSE);
                    losehp(loss, "immolation", NO_KILLER_PREFIX);
                } else {
                    /* guard against mdef being killed twice should
                       it die before we reach this point */
                    if (!DEADMONSTER(mdef)) {
                        if (show_instakill)
                            pline("%s ignites and turns to ash!",
                                  Monnam(mdef));
                        if (youattack) {
                            xkilled(mdef, XKILL_NOMSG | XKILL_NOCORPSE);
                        } else {
                            if (magr && magr->mtame)
                                set_pet_killer(magr);
                            monkilled(mdef, (char *) 0, AD_FIRE);
                        }
                    }
                }
                return TRUE;
            }
        }

        angel = youdefend ? is_angel(gy.youmonst.data)
                          : is_angel(mdef->data);
        aasimar = youdefend ? maybe_polyd(is_aasimar(gy.youmonst.data),
                                          Race_if(PM_AASIMAR))
                            : racial_aasimar(mdef);

        if (is_art(otmp, ART_ANGELSLAYER) && !rn2(10)
            && (angel || aasimar)) {
            if (youdefend) {
                int loss = (Upolyd ? u.mh : u.uhp) + 1;

                pline("Angelslayer's eldritch flame consumes %s!", hittee);
                /* the wielder's kill, with nothing left to rise */
                set_monster_killer_hp(magr, loss, FALSE);
                losehp(loss, "incinerated by Angelslayer", NO_KILLER_PREFIX);
            } else {
                /* guard against mdef being killed twice should
                   it die before we reach this point */
                if (!DEADMONSTER(mdef)) {
                    if (show_instakill)
                        pline_The(
                            "infernal trident's eldritch flame consumes %s!",
                                  hittee);
                    if (youattack) {
                        xkilled(mdef, XKILL_NOMSG | XKILL_NOCORPSE);
                    } else {
                        if (magr && magr->mtame)
                            set_pet_killer(magr);
                        monkilled(mdef, (char *) 0, AD_FIRE);
                    }
                }
            }
            return TRUE;
        }
        return realizes_damage;
    }
    if (attacks(AD_COLD, otmp)) {
        if (realizes_damage) {
            if (is_art(otmp, ART_FROST_BRAND)) {
                pline_The("ice-cold blade %s %s%c",
                          !gs.spec_dbon_applies
                              ? "hits"
                              : can_freeze(mdef->data)
                                  ? "freezes part of"
                                  : vulnerable_to(mdef, AD_COLD)
                                      ? "severely freezes" : "freezes",
                          hittee, !gs.spec_dbon_applies ? '.' : '!');
            } else if ((otmp->oclass == WEAPON_CLASS || is_weptool(otmp)
                        || is_bullet(otmp) || otmp == uarms
                        || otmp == uarmg)
                       && (otmp->oprops & ITEM_FROST)) {
                pline_The("%s %s %s%c",
                          otmp->oclass == WEAPON_CLASS
                              ? distant_name(otmp, xname)
                              : simpleonames(otmp),
                          !gs.spec_dbon_applies
                              ? "hits"
                              : can_freeze(mdef->data)
                                  ? "freezes part of"
                                  : vulnerable_to(mdef, AD_COLD)
                                      ? "severely freezes" : "freezes",
                          hittee, !gs.spec_dbon_applies ? '.' : '!');
            }
        }
        if ((otmp->oprops & ITEM_FROST) && gs.spec_dbon_applies) {
            otmp->oprops_known |= ITEM_FROST;
            update_inventory();
        }
        if (!rn2(4)) {
            int itemdmg = destroy_items(mdef, AD_COLD, *dmgptr);

            if (!youdefend)
                *dmgptr += itemdmg; /* item destruction dmg */
        }
        return realizes_damage;
    }
    if (attacks(AD_ELEC, otmp)) {
        if (realizes_damage) {
            if (is_art(otmp, ART_MJOLLNIR)) {
                pline_The("massive hammer hits%s %s%c",
                          !gs.spec_dbon_applies ? "" : "!  Lightning strikes",
                          hittee, !gs.spec_dbon_applies ? '.' : '!');
            } else if (is_art(otmp, ART_KEOLEWA)) {
                pline_The("war club hits%s %s%c",
                          !gs.spec_dbon_applies ? "" : "!  Lightning strikes",
                          hittee, !gs.spec_dbon_applies ? '.' : '!');
            } else if (is_art(otmp, ART_TEMPEST)) {
                pline_The("tumultuous polearm hits%s %s%c",
                          !gs.spec_dbon_applies ? "" : "!  Lightning strikes",
                          hittee, !gs.spec_dbon_applies ? '.' : '!');
            } else if ((otmp->oclass == WEAPON_CLASS || is_weptool(otmp)
                        || is_bullet(otmp) || otmp == uarms
                        || otmp == uarmg)
                       && (otmp->oprops & ITEM_SHOCK)) {
                pline_The("%s %s %s%c",
                          otmp->oclass == WEAPON_CLASS
                              ? distant_name(otmp, xname)
                              : simpleonames(otmp),
                          !gs.spec_dbon_applies
                              ? "hits"
                              : vulnerable_to(mdef, AD_ELEC)
                                  ? "severely shocks"
                                  : rn2(2) ? "jolts" : "shocks",
                          hittee, !gs.spec_dbon_applies ? '.' : '!');
            }
        }
        if ((otmp->oprops & ITEM_SHOCK) && gs.spec_dbon_applies) {
            otmp->oprops_known |= ITEM_SHOCK;
            update_inventory();
        }
        /* only Mjollnir's thunderclap wakes the neighborhood; other
           shock artifacts and object-property weapons stay quiet */
        if (gs.spec_dbon_applies && is_art(otmp, ART_MJOLLNIR))
            wake_nearto(youdefend ? u.ux : mdef->mx,
                        youdefend ? u.uy : mdef->my, 4 * 4);

        /* Tempest has a chance of a power surge on a successful hit,
           dealing extra electrical damage (area of effect); a blessed
           one in the hero's hands, wielded with at least skilled
           proficiency in its base weapon, reins the surge in rather
           than catch a peaceful or tame creature next to the target --
           an uncursed, cursed, or clumsily wielded one, or any
           monster's, blasts regardless of who is standing in range */
        if (!rn2(6)
            && gs.spec_dbon_applies && is_art(otmp, ART_TEMPEST)
            && (youdefend || !DEADMONSTER(mdef))) {
            if (youattack && otmp->blessed
                && P_SKILL(weapon_type(otmp)) >= P_SKILLED
                && friendly_in_blast(mdef->mx, mdef->my, mdef)) {
                pline("A surge of energy builds in the %s, then ebbs.",
                      simpleonames(otmp));
                /* fall through to the normal AD_ELEC follow-up */
            } else {
                struct monst *save_exploder = ge.exploder;

                pline("A massive surge of energy courses through the %s!",
                      simpleonames(otmp));
                ge.exploder = magr;
                explode(youdefend ? u.ux : mdef->mx,
                        youdefend ? u.uy : mdef->my,
                        (youattack ? BZ_U_BREATH(ZT_LIGHTNING)
                                   : BZ_M_BREATH(ZT_LIGHTNING)), d(6, 6),
                        0, EXPL_SHOCK);
                ge.exploder = save_exploder;
                return TRUE;
            }
        }

        if (!rn2(5)) {
            int itemdmg = destroy_items(mdef, AD_ELEC, *dmgptr);

            if (!youdefend)
                *dmgptr += itemdmg; /* item destruction dmg */
        }
        return realizes_damage;
    }
    if (attacks(AD_DRST, otmp)) {
        if (realizes_damage) {
            if (is_art(otmp, ART_SWORD_OF_KAS)) {
                pline_The("gigantic blade %s %s%c",
                          (resists_poison(mdef) || defended(mdef, AD_DRST))
                              ? "hits"
                              : rn2(2) ? "poisons" : "eviscerates",
                          hittee, !gs.spec_dbon_applies ? '.' : '!');
            } else if ((otmp->oclass == WEAPON_CLASS || is_weptool(otmp)
                        || is_bullet(otmp))
                       && (otmp->oprops & ITEM_VENOM)) {
                pline_The("%s %s %s%c",
                          distant_name(otmp, xname),
                          (resists_poison(mdef) || defended(mdef, AD_DRST))
                              ? "hits"
                              : rn2(2) ? "infects" : "poisons",
                          hittee, !gs.spec_dbon_applies ? '.' : '!');
            }
        }
        if ((otmp->oprops & ITEM_VENOM) && gs.spec_dbon_applies) {
            otmp->oprops_known |= ITEM_VENOM;
            update_inventory();
        }
        /* magr is Null for a missile or the hero's own blade, which keep
           the blade as the cause; a monster's melee blow is its kill */
        if (youdefend && gs.spec_dbon_applies && !rn2(8))
            poisoned("blade", A_STR, "poisoned blade", 30, FALSE, magr);
        return realizes_damage;
    }
    /* disease */
    if (attacks(AD_DISE, otmp)) {
        boolean elf = youdefend ? maybe_polyd(is_elf(gy.youmonst.data),
                                              Race_if(PM_ELF))
                                : racial_elf(mdef);
        boolean drow = youdefend ? maybe_polyd(is_drow(gy.youmonst.data),
                                               Race_if(PM_DROW))
                                 : racial_drow(mdef);
        boolean no_sick = youdefend ? Sick_resistance
                                    : (resists_sick(mdef)
                                       || defended(mdef, AD_DISE));

        if (youattack) {
            if (Role_if(PM_SAMURAI)) {
                You("dishonorably use a diseased weapon!");
                adjalign(-sgn(u.ualign.type));
                record_abuse_event(-sgn(u.ualign.type), ABUSE_USE_DISEASE);
            } else if (u.ualign.type == A_LAWFUL && u.ualign.record > -10) {
                int penalty = Role_if(PM_KNIGHT) ? -10 : -1;

                You_feel("like an evil coward for using a diseased weapon.");
                adjalign(penalty);
                record_abuse_event(penalty, ABUSE_USE_DISEASE);
            }
        }
        if (realizes_damage) {
            /* currently the only object that uses this is Grimtooth */
            pline_The("filthy dagger %s %s%c",
                      no_sick ? "hits"
                              : rn2(2) ? "contaminates" : "infects",
                      hittee, !gs.spec_dbon_applies ? '.' : '!');
        }

        if (!rn2(10) && (elf || drow)) {
            if ((youdefend || !DEADMONSTER(mdef)) && show_instakill)
                pline_The(
                     "cruel blade penetrates %s soft flesh, disemboweling %s!",
                          youdefend ? "your" : s_suffix(mon_nam(mdef)),
                          youdefend ? "you" : noit_mhim(mdef));
            if (youdefend) {
                int loss = (Upolyd ? u.mh : u.uhp) + 1;

                /* the wielder's kill, rising as after its melee kill */
                set_monster_killer_hp(magr, loss, TRUE);
                losehp(loss, "disemboweled by Grimtooth", NO_KILLER_PREFIX);
            } else {
                /* guard against mdef being killed twice should
                   it die before we reach this point */
                if (!DEADMONSTER(mdef)) {
                    if (youattack) {
                        xkilled(mdef, XKILL_NOMSG);
                    } else {
                        if (magr && magr->mtame)
                            set_pet_killer(magr);
                        monkilled(mdef, (char *) 0, AD_DISE);
                    }
                }
            }
            return TRUE;
        }

        if (youdefend && magr && !rn2(5)) {
            (void) diseasemu(magr);
        } else if (!youdefend) {
            mdef->mdiseasetime = rnd(10) + 5;
            if (!no_sick && !rn2(5)) {
                if (canseemon(mdef))
                    pline_mon(mdef, "%s looks %s.", Monnam(mdef),
                              mdef->mdiseased ? "even worse" : "diseased");
                mdef->mdiseased = 1;
                /* only a hero-swung weapon credits the hero when the
                   disease kills; a monster-wielded Grimtooth must not */
                mdef->mdiseabyu = youattack ? TRUE : FALSE;
            }
        }
        return realizes_damage;
    }
    if (attacks(AD_MAGM, otmp)) {
        if (realizes_damage)
            pline_The("imaginary widget hits%s %s%c",
                      !gs.spec_dbon_applies
                          ? ""
                          : "!  A hail of magic missiles strikes",
                      hittee, !gs.spec_dbon_applies ? '.' : '!');
        return realizes_damage;
    }
    /* death magic */
    if (attacks(AD_DETH, otmp)) {
        int permdmg = 0;

        if (realizes_damage) {
            /* currently the only object that uses this
               is the Wand of Orcus artifact */
            pline_The("ornate mace %s %s%c",
                      (immune_death_magic(mdef->data)
                       || defended(mdef, AD_DETH))
                          ? "hits"
                          : rn2(2) ? "bashes" : "bludgeons",
                      hittee, !gs.spec_dbon_applies ? '.' : '!');
        }
        if (youdefend && !Death_resistance) {
            switch (rn2(20)) {
            case 19:
            case 18:
            case 17:
                if (!Antimagic) {
                    pline_The("%s consumes your soul!",
                              distant_name(otmp, xname));
                    svk.killer.format = KILLED_BY;
                    Strcpy(svk.killer.name, "the Wand of Orcus");
                    /* the wielder's kill (none for a thrown one) */
                    set_monster_killer(magr, TRUE);
                    done(DIED);
                    return TRUE;
                }
                FALLTHROUGH;
                /*FALLTHRU*/
            default: /* case 16: ... case 0: */
                You_feel("your life force draining away...");
                permdmg = 1; /* actual damage done below */
                break;
            }
            if (permdmg) {
                int lowerlimit, *hpmax_p;
                /*
                 * Apply some of the damage to permanent hit points:
                 *  polymorphed         100% against poly'd hpmax
                 *  hpmax > 25*lvl  50..100% against normal hpmax
                 *  hpmax > 10*lvl  25..75%
                 *  hpmax >  5*lvl  20..60%
                 *  otherwise        0..50%
                 * Never reduces hpmax below 1 hit point per level.
                 */
                permdmg = rn2(*dmgptr / 2 + 1);
                if (Upolyd || u.uhpmax > 25 * u.ulevel)
                    permdmg = *dmgptr / 2;
                else if (u.uhpmax > 10 * u.ulevel)
                    permdmg += *dmgptr / 4;
                else if (u.uhpmax > 5 * u.ulevel)
                    permdmg += *dmgptr / 5;

                /* cap maximum permdmg to 15 hit points per hit */
                if (permdmg > 15)
                    permdmg = 15;

                if (Upolyd) {
                    hpmax_p = &u.mhmax;
                    /* [can't use gy.youmonst.m_lev] */
                    lowerlimit = min((int) gy.youmonst.data->mlevel,
                                     u.ulevel);
                    if (lowerlimit <= 0)
                        lowerlimit = 1;
                } else {
                    hpmax_p = &u.uhpmax;
                    lowerlimit = u.ulevel;
                }
                if (*hpmax_p - permdmg > lowerlimit)
                    *hpmax_p -= permdmg;
                else if (*hpmax_p > lowerlimit)
                    *hpmax_p = lowerlimit;
                /* else unlikely...
                 * already at or below minimum threshold; do nothing */

                if (!Upolyd) {
                    if (u.uhp > *hpmax_p)
                        u.uhp = *hpmax_p;
                } else {
                    if (u.mh > *hpmax_p)
                        u.mh = *hpmax_p;
                }
                disp.botl = TRUE;
            }
        } else if (!youdefend && !immune_death_magic(mdef->data)) {
            switch (rn2(20)) {
            case 19:
            case 18:
            case 17:
                if (!(resists_magm(mdef) || defended(mdef, AD_MAGM))
                    && !resist(mdef, 0, 0, 0)) {
                    pline_The("%s consumes %s life force!",
                              distant_name(otmp, xname),
                              s_suffix(mon_nam(mdef)));
                    mdef->mhp = 0;
                    if (!youattack && magr && magr->mtame)
                        set_pet_killer(magr);
                    monkilled(mdef, (char *) 0, AD_DETH);
                    if (!DEADMONSTER(mdef))
                        return FALSE;
                    return TRUE;
                }
                break;
            default: /* case 16 through case 0 */
                if (vis)
                    pline("%s looks weaker!", Monnam(mdef));
                /* mhp will then still be less than this value */
                mdef->mhpmax -= (rn2(*dmgptr / 2 + 1) + (*dmgptr / 4));
                if (mdef->mhpmax <= 0) /* protect against invalid value */
                    mdef->mhpmax = 1;
                break;
            }
        }
        return realizes_damage;
    }
    /* acid */
    if (attacks(AD_ACID, otmp)) {
        boolean giant;

        if (realizes_damage) {
            if (is_art(otmp, ART_DIRGE)) {
                pline_The("acidic blade %s %s%c",
                          !gs.spec_dbon_applies
                              ? "hits"
                              : can_corrode(mdef->data)
                                  ? "eats away part of"
                                  : def_underwater
                                      ? "hits"
                                      : vulnerable_to(mdef, AD_ACID)
                                          ? "severely burns" : "burns",
                          hittee, !gs.spec_dbon_applies ? '.' : '!');
            } else if (is_art(otmp, ART_HARBINGER)) {
                pline_The("acidic %s %s %s%c",
                          simpleonames(otmp),
                          !gs.spec_dbon_applies
                              ? "hits"
                              : can_corrode(mdef->data)
                                  ? "eats away part of"
                                  : def_underwater
                                      ? "hits"
                                      : vulnerable_to(mdef, AD_ACID)
                                          ? "severely burns" : "burns",
                          hittee, !gs.spec_dbon_applies ? '.' : '!');
            }
        }

        /* Harbinger has a chance of an explosive surge on a successful
           hit, dealing extra acidic damage (area of effect); a blessed
           one in the hero's hands, wielded with at least skilled
           proficiency in its base weapon, reins the surge in rather
           than catch a peaceful or tame creature next to the target --
           an uncursed, cursed, or clumsily wielded one, or any
           monster's, blasts regardless of who is standing in range */
        if (!rn2(6)
            && gs.spec_dbon_applies && is_art(otmp, ART_HARBINGER)
            && (youdefend || !DEADMONSTER(mdef))) {
            if (youattack && otmp->blessed
                && P_SKILL(weapon_type(otmp)) >= P_SKILLED
                && friendly_in_blast(mdef->mx, mdef->my, mdef)) {
                pline("A surge of acid wells up in the %s, then ebbs.",
                      simpleonames(otmp));
                /* fall through to the normal AD_ACID follow-up */
            } else {
                struct monst *save_exploder = ge.exploder;

                pline("A surge of acid flows through the %s!",
                      simpleonames(otmp));
                ge.exploder = magr;
                explode(youdefend ? u.ux : mdef->mx,
                        youdefend ? u.uy : mdef->my,
                        (youattack ? BZ_U_BREATH(ZT_ACID)
                                   : BZ_M_BREATH(ZT_ACID)), d(6, 6),
                        0, EXPL_ACID);
                ge.exploder = save_exploder;
                return TRUE;
            }
        }

        if (!def_underwater) {
            if (!rn2(5))
                erode_armor(mdef, ERODE_CORRODE);
        }

        /* Harbinger inherits Giantslayer's chance to instakill
           giants with a single blow */
        giant = youdefend ? maybe_polyd(is_giant(gy.youmonst.data),
                                        Race_if(PM_GIANT))
                          : racial_giant(mdef);

        if (!rn2(10) && giant && is_art(otmp, ART_HARBINGER)) {
            if (youdefend || !DEADMONSTER(mdef)) {
                if (show_instakill)
                    pline(
     "A massive surge of acid engulfs %s body, completely dissolving %s!",
                          youdefend ? "your" : s_suffix(mon_nam(mdef)),
                          youdefend ? "you" : noit_mhim(mdef));
            }
            if (youdefend) {
                int loss = (Upolyd ? u.mh : u.uhp) + 1;

                /* the wielder's kill, with nothing left to rise */
                set_monster_killer_hp(magr, loss, FALSE);
                losehp(loss, "dissolved by Harbinger", NO_KILLER_PREFIX);
            } else { /* you or mon hit monster */
                /* guard against mdef being killed twice should
                   it die before we reach this point */
                if (!DEADMONSTER(mdef)) {
                    if (youattack) {
                        xkilled(mdef, XKILL_NOMSG);
                    } else {
                        if (magr && magr->mtame)
                            set_pet_killer(magr);
                        monkilled(mdef, (char *) 0, AD_DISE);
                    }
                }
            }
            return TRUE;
        }
        return realizes_damage;
    }
    /* disintegration */
    if (attacks(AD_DISN, otmp)) {
        boolean resistant;
        struct obj *target;

        resistant = youdefend ? how_resistant(DISINT_RES) >= 50
                              : (resists_disint(mdef)
                                 || defended(mdef, AD_DISN));
        if (!resistant
            && !rn2((!youdefend && (mdef->data->geno & G_UNIQ))
                    ? 100 : 12)) {
            /* instant disintegration */
            if (youdefend || !DEADMONSTER(mdef)) {
                if (show_instakill)
                    pline_The("deadly blade disintegrates %s!", hittee);
            }
            if (youdefend) {
                u.ugrave_arise = -3; /* no corpse */
                svk.killer.format = NO_KILLER_PREFIX;
                Snprintf(svk.killer.name, sizeof svk.killer.name,
                         "disintegrated by %s", the(xname(otmp)));
                /* the wielder's kill, with nothing left to rise */
                set_monster_killer(magr, FALSE);
                done(DISINTEGRATED);
            } else { /* you or mon hit monster */
                disint_mon_invent(mdef);
                if (youattack) {
                    if (is_rider(mdef->data)) {
                        if (vis) {
                            pline("%s body reintegrates before your %s!",
                                  s_suffix(Monnam(mdef)),
                                  (eyecount(gy.youmonst.data) == 1)
                                     ? body_part(EYE)
                                     : makeplural(body_part(EYE)));
                            pline("%s resurrects!", Monnam(mdef));
                        }
                        mdef->mhp = mdef->mhpmax;
                    } else {
                        if (!DEADMONSTER(mdef))
                            xkilled(mdef, XKILL_NOMSG | XKILL_NOCORPSE);
                    }
                } else {
                    if (is_rider(mdef->data)) {
                        if (vis) {
                            pline("%s body reintegrates!",
                                  s_suffix(Monnam(mdef)));
                            pline("%s resurrects!", Monnam(mdef));
                        }
                        mdef->mhp = mdef->mhpmax;
                    } else {
                        if (!DEADMONSTER(mdef)) {
                            if (magr && magr->mtame)
                                set_pet_killer(magr);
                            monkilled(mdef, (char *) 0, AD_DISN);
                        }
                    }
                }
            }
            return TRUE;
        }

        if (realizes_damage) {
            /* currently the only object that uses this
               is the Sword of Annihilation artifact */
            pline_The("deadly blade %s %s!",
                      resistant ? "hits" : "partially disintegrates",
                      hittee);
        }

        /* maybe disintegrate some armor */
        target = (struct obj *) 0;

        if (youdefend) { /* hero's armor is targeted */
            if (uarms && !rn2(3)) {
                target = uarms;
            } else if (uarmc && !rn2(4)) {
                target = uarmc;
            } else if (uarm && !rn2(6)) {
                target = uarm;
            }

            if (target) {
                if (target->otyp == BLACK_DRAGON_SCALES
                    || (Is_dragon_scaled_armor(target)
                        && Dragon_armor_to_scales(target)
                           == BLACK_DRAGON_SCALES)) {
                    /* nothing happens */
                    Your("%s vibrates unexpectedly, but remains intact.",
                         xname(target));
                } else {
                    (void) disintegrate_arm(target);
                }
            }
        } else { /* monster's armor is targeted */
            if ((mdef->misc_worn_check & W_ARMS) && !rn2(3)) {
                target = which_armor(mdef, W_ARMS);
            } else if ((mdef->misc_worn_check & W_ARMC) && !rn2(4)) {
                target = which_armor(mdef, W_ARMC);
            } else if ((mdef->misc_worn_check & W_ARM) && !rn2(6)) {
                target = which_armor(mdef, W_ARM);
            }

            if (target) {
                if (target->otyp == BLACK_DRAGON_SCALES
                    || (Is_dragon_scaled_armor(target)
                        && Dragon_armor_to_scales(target)
                           == BLACK_DRAGON_SCALES)) {
                    /* nothing happens */
                    if (canseemon(mdef))
                        pline(
                         "%s %s vibrates unexpectedly, but remains intact.",
                              s_suffix(Monnam(mdef)), xname(target));
                } else {
                    if (canseemon(mdef))
                        pline("%s %s is disintegrated!",
                              s_suffix(Monnam(mdef)), xname(target));
                    m_useup(mdef, target);
                }
            }
        }
        return realizes_damage;
    }
    /* combines fire and cold together */
    if (attacks(AD_FUSE, otmp)) {
        boolean no_burn, no_freeze;

        no_burn = youdefend ? (Fire_resistance || Underwater)
                            : (resists_fire(mdef)
                               || defended(mdef, AD_FIRE)
                               || mon_underwater(mdef));
        no_freeze = youdefend ? Cold_resistance
                              : (resists_cold(mdef)
                                 || defended(mdef, AD_COLD));
        if (realizes_damage) {
            if (no_freeze) {
                pline_The("scintillating blade %s %s%c",
                          !gs.spec_dbon_applies
                              ? "hits"
                              : can_vaporize(mdef->data)
                                  ? "vaporizes part of"
                                  : def_underwater ? "hits" : "burns",
                          hittee, !gs.spec_dbon_applies ? '.' : '!');
            } else if (no_burn) {
                pline_The("scintillating blade %s %s%c",
                          !gs.spec_dbon_applies
                              ? "hits"
                              : can_freeze(mdef->data)
                                  ? "freezes part of"
                                  : "freezes",
                          hittee, !gs.spec_dbon_applies ? '.' : '!');
            } else {
                pline_The("scintillating blade %s %s%c",
                          !gs.spec_dbon_applies
                              ? "hits" : "sears",
                          hittee, !gs.spec_dbon_applies ? '.' : '!');
            }
        }
        /* Dichotomy has a chance of exploding either a ball of fire or
           cold on a successful hit, dealing extra elemental damage
           (area of effect); a blessed one in the hero's hands, wielded
           with at least skilled proficiency in its base weapon, reins
           the surge in rather than catch a peaceful or tame creature
           next to the target -- an uncursed, cursed, or clumsily
           wielded one, or any monster's, blasts regardless of who is
           standing in range */
        if (!rn2(6)
            && gs.spec_dbon_applies && is_art(otmp, ART_DICHOTOMY)
            && (youdefend || !DEADMONSTER(mdef))) {
            if (youattack && otmp->blessed
                && P_SKILL(weapon_type(otmp)) >= P_SKILLED
                && friendly_in_blast(mdef->mx, mdef->my, mdef)) {
                pline("Power surges through the blade, then ebbs.");
                /* fall through to the normal follow-up effects */
            } else if (rn2(2)) {
                if (!(resists_fire(mdef) || defended(mdef, AD_FIRE))) {
                    struct monst *save_exploder = ge.exploder;

                    pline("A surge of flame flows through the blade!");
                    ge.exploder = magr;
                    explode(youdefend ? u.ux : mdef->mx,
                            youdefend ? u.uy : mdef->my,
                            (youattack ? BZ_U_BREATH(ZT_FIRE)
                                       : BZ_M_BREATH(ZT_FIRE)), d(4, 6),
                            0, EXPL_FIERY);
                    ge.exploder = save_exploder;
                    return TRUE;
                }
            } else {
                if (!(resists_cold(mdef) || defended(mdef, AD_COLD))) {
                    struct monst *save_exploder = ge.exploder;

                    pline("A surge of frost flows through the blade!");
                    ge.exploder = magr;
                    explode(youdefend ? u.ux : mdef->mx,
                            youdefend ? u.uy : mdef->my,
                            (youattack ? BZ_U_BREATH(ZT_COLD)
                                       : BZ_M_BREATH(ZT_COLD)), d(4, 6),
                            0, EXPL_FROSTY);
                    ge.exploder = save_exploder;
                    return TRUE;
                }
            }
        }
        if (!rn2(4) && !no_freeze) {
            int itemdmg = destroy_items(mdef, AD_COLD, *dmgptr);

            if (!youdefend)
                *dmgptr += itemdmg; /* item destruction dmg */
        }
        if (!def_underwater && !no_burn) {
            if (!rn2(4)) {
                int itemdmg = destroy_items(mdef, AD_FIRE, *dmgptr);

                if (!youdefend)
                    *dmgptr += itemdmg; /* item destruction dmg */
                ignite_items(mdef->minvent);
            }
            if (youdefend && Slimed)
                burn_away_slime();
            /* NB: if youdefend, then mdef == &gy.youmonst */
            if (completelyburns(mdef->data)
                || mdef->data == &mons[PM_WOOD_GOLEM]
                || mdef->data == &mons[PM_GREEN_SLIME]
                || (is_blight(mdef->data) && !rn2(10))
                || (mdef->data == &mons[PM_ENT] && !rn2(10))
                || (mdef->data == &mons[PM_ELDER_ENT] && !rn2(12))) {
                if (youdefend) {
                    int loss = (Upolyd ? u.mh : u.uhp) + 1;

                    You("ignite and turn to ash!");
                    /* the wielder's kill, with nothing left to rise */
                    set_monster_killer_hp(magr, loss, FALSE);
                    losehp(loss, "immolation", NO_KILLER_PREFIX);
                } else {
                    /* guard against mdef being killed twice should
                       it die before we reach this point */
                    if (!DEADMONSTER(mdef)) {
                        if (show_instakill)
                            pline("%s ignites and turns to ash!",
                                  Monnam(mdef));
                        if (youattack) {
                            xkilled(mdef, XKILL_NOMSG | XKILL_NOCORPSE);
                        } else {
                            if (magr && magr->mtame)
                                set_pet_killer(magr);
                            monkilled(mdef, (char *) 0, AD_FIRE);
                        }
                    }
                }
                return TRUE;
            }
        }
        return realizes_damage;
    }

    if (attacks(AD_STUN, otmp) && dieroll <= MB_MAX_DIEROLL) {
        /* Magicbane's special attacks (possibly modifies hittee[]) */
        return Mb_hit(magr, mdef, otmp, dmgptr, dieroll, vis, hittee);
    }

    if (!gs.spec_dbon_applies && !hurtle_distance) {
        /* since damage bonus didn't apply, nothing more to do;
           no further attacks have side-effects on inventory */
        return FALSE;
    }

    /* Some neat and unique effects from various artifact weapons.
     * Pulled this from SporkHack - by default, the odds of an instakill
     * were 50/50. Because we can dual-wield artifact weapons in EvilHack,
     * we need to tone this down. Significantly.
     * I've added a few extra artifact weapons here that could use some
     * love. Just be warned, this can be used against the player depending
     * on the race they choose...
     *
     * If an artifact already has another type of attack (e.g. Angelslayer
     * has SPFX_DFLAGH vs MH_ANGEL but also uses AD_FIRE attack type), they
     * can't be included in this switch statement.
     */
    atmp = get_artifact(otmp);

    if (atmp->spfx & (SPFX_DFLAGH | SPFX_DCLAS)) {
        int j = !rn2(10), /* 10% chance of instakill for some artifacts */
            k = !rn2(20), /* 5% chance if same weapon is used against
                             the player */
            l = !rn2(5);  /* special chance of incineration
                             (Draugr/Vampire) */

        switch (otmp->oartifact) {
        case ART_WEREBANE:
        case ART_SHADOWBLADE:
            if (youattack && is_were(mdef->data) && j) {
                You("severely burn %s with your %s blade!", mon_nam(mdef),
                    (is_art(otmp, ART_WEREBANE) ? "silver" : "shadowy"));
                *dmgptr = (2 * mdef->mhp + FATAL_DAMAGE_MODIFIER);
            } else if (!youattack && !youdefend
                       && magr && is_were(mdef->data) && j) {
                if (show_instakill)
                    pline("%s severely burns %s with its %s blade!",
                          Monnam(magr), mon_nam(mdef),
                          (is_art(otmp, ART_WEREBANE) ? "silver"
                                                      : "shadowy"));
                *dmgptr = (2 * mdef->mhp + FATAL_DAMAGE_MODIFIER);
            } else if (youdefend && is_were(gy.youmonst.data) && k) {
                pline_The("%s blade gravely burns you!",
                          (is_art(otmp, ART_WEREBANE) ? "silver"
                                                      : "shadowy"));
                *dmgptr = (2 * (Upolyd ? u.mh : u.uhp)
                           + FATAL_DAMAGE_MODIFIER);
                /* player returns to their original form */
            } else if (!(youdefend ? is_were(gy.youmonst.data)
                                   : is_were(mdef->data))
                       && is_art(otmp, ART_SHADOWBLADE)) {
                /* Shadowblade still has a DRLI attack */
                break;
            } else {
                return FALSE;
            }
            return TRUE;
        case ART_GIANTSLAYER:
            if (youattack && racial_giant(mdef) && j) {
                You("eviscerate %s with a fatal stab!", mon_nam(mdef));
                *dmgptr = (2 * mdef->mhp + FATAL_DAMAGE_MODIFIER);
            } else if (!youattack && !youdefend
                       && magr && racial_giant(mdef) && j) {
                if (show_instakill)
                    pline("%s eviscerates %s with a fatal stab!",
                          Monnam(magr), mon_nam(mdef));
                *dmgptr = (2 * mdef->mhp + FATAL_DAMAGE_MODIFIER);
            } else if (youdefend && maybe_polyd(is_giant(gy.youmonst.data),
                                                Race_if(PM_GIANT)) && k) {
                pline_The("magical spear eviscerates you!");
                *dmgptr = (2 * (Upolyd ? u.mh : u.uhp)
                           + FATAL_DAMAGE_MODIFIER);
                /* player returns to their original form if poly'd */
            } else
                return FALSE;
            return TRUE;
        case ART_OGRESMASHER:
            if (youattack && is_ogre(mdef->data) && j) {
                You("crush %s skull!", s_suffix(mon_nam(mdef)));
                *dmgptr = (2 * mdef->mhp + FATAL_DAMAGE_MODIFIER);
            } else if (!youattack && !youdefend
                       && magr && is_ogre(mdef->data) && j) {
                if (show_instakill)
                    pline("%s crushes %s skull!",
                          Monnam(magr), s_suffix(mon_nam(mdef)));
                *dmgptr = (2 * mdef->mhp + FATAL_DAMAGE_MODIFIER);
            } else if (youdefend && is_ogre(gy.youmonst.data) && k) {
                pline_The("monstrous hammer crushes your skull!");
                *dmgptr = (2 * (Upolyd ? u.mh : u.uhp)
                           + FATAL_DAMAGE_MODIFIER);
                /* player returns to their original form */
            } else if (hurtle_distance && !unsolid(mdef->data)) {
                if (youattack && !u.uswallow) {
                    You("smash %s backwards%s", mon_nam(mdef),
                        canseemon(mdef) ? exclam(4 * hurtle_distance)
                                        : ".");
                    mhurtle(mdef, u.dx, u.dy, hurtle_distance);
                } else if (!youattack && !youdefend) {
                    /* not an instakill, but currently behaves like one */
                    int kx, ky;

                    if (magr) {
                        kx = mdef->mx - magr->mx;
                        ky = mdef->my - magr->my;
                    } else {
                        /* thrown projectile has no attacker; derive
                           knockback direction from projectile's path */
                        kx = mdef->mx - gb.bhitpos.x;
                        ky = mdef->my - gb.bhitpos.y;
                        if (!kx && !ky)
                            return FALSE;
                    }
                    if (show_instakill) {
                        if (magr)
                            pline("%s smashes %s backwards!",
                                  Monnam(magr), mon_nam(mdef));
                        else
                            pline("%s is smashed backwards!",
                                  Monnam(mdef));
                    }
                    mhurtle(mdef, kx, ky, hurtle_distance);
                } else if (!u.uswallow) {
                    int kx, ky;

                    if (magr) {
                        kx = u.ux - magr->mx;
                        ky = u.uy - magr->my;
                    } else {
                        kx = u.ux - gb.bhitpos.x;
                        ky = u.uy - gb.bhitpos.y;
                        if (!kx && !ky)
                            return FALSE;
                    }
                    You("are smashed backwards!");
                    hurtle(kx, ky, hurtle_distance, FALSE);
                    /* hurtle's traps can fire dobuzz that kills magr
                       on the beam path */
                    if (magr && !DEADMONSTER(magr)) {
                        magr->mux = u.ux;
                        magr->muy = u.uy;
                    }
                    return TRUE;
                }
            } else
                return FALSE;
            return TRUE;
        case ART_TROLLSBANE:
            if (youattack && is_troll(mdef->data) && j) {
                pline("As you strike %s, it bursts into flame!",
                      mon_nam(mdef));
                xkilled(mdef, XKILL_NOMSG | XKILL_NOCORPSE);
            } else if (!youattack && !youdefend
                       && magr && is_troll(mdef->data) && j) {
                if (show_instakill)
                    pline("As %s strikes %s, it bursts into flame!",
                          mon_nam(magr), mon_nam(mdef));
                if (magr->mtame)
                    set_pet_killer(magr);
                monkilled(mdef, (char *) 0, AD_DISN);
            } else if (youdefend && is_troll(gy.youmonst.data) && k) {
                You("burst into flame as you are hit!");
                *dmgptr = (2 * (Upolyd ? u.mh : u.uhp)
                           + FATAL_DAMAGE_MODIFIER);
                /* player returns to their original form */
            } else
                return FALSE;
            return TRUE;
        case ART_ORCRIST:
            if (youattack && racial_orc(mdef) && j) {
                You("slice open %s throat!", s_suffix(mon_nam(mdef)));
                *dmgptr = (2 * mdef->mhp + FATAL_DAMAGE_MODIFIER);
            } else if (!youattack && !youdefend
                       && magr && racial_orc(mdef) && j) {
                if (show_instakill)
                    pline("%s slices open %s throat!",
                          Monnam(magr), s_suffix(mon_nam(mdef)));
                *dmgptr = (2 * mdef->mhp + FATAL_DAMAGE_MODIFIER);
            } else if (youdefend && maybe_polyd(is_orc(gy.youmonst.data),
                                                Race_if(PM_ORC)) && k) {
                You("feel Orcrist slice deep across your neck!");
                *dmgptr = (2 * (Upolyd ? u.mh : u.uhp)
                           + FATAL_DAMAGE_MODIFIER);
                /* player returns to their original form if poly'd */
            } else
                return FALSE;
            return TRUE;
        case ART_STING:
            if (youattack && racial_orc(mdef) && j) {
                You("stab deep into %s heart!", s_suffix(mon_nam(mdef)));
                *dmgptr = (2 * mdef->mhp + FATAL_DAMAGE_MODIFIER);
            } else if (!youattack && !youdefend
                       && magr && racial_orc(mdef) && j) {
                if (show_instakill)
                    pline("%s stabs deep into %s heart!",
                          Monnam(magr), s_suffix(mon_nam(mdef)));
                *dmgptr = (2 * mdef->mhp + FATAL_DAMAGE_MODIFIER);
            } else if (youdefend && maybe_polyd(is_orc(gy.youmonst.data),
                                                Race_if(PM_ORC)) && k) {
                You("feel Sting stab deep into your heart!");
                *dmgptr = (2 * (Upolyd ? u.mh : u.uhp)
                           + FATAL_DAMAGE_MODIFIER);
                /* player returns to their original form if poly'd */
            } else
                return FALSE;
            return TRUE;
        case ART_SUNSWORD:
            if (youattack && is_undead(mdef->data) && j) {
                if (mdef->isvecna || mdef->isvlad) {
                    pline_The(
             "consecrated blade flares brightly, severely wounding %s!",
                              mon_nam(mdef));
                    *dmgptr *= 3;
                    return TRUE;
                } else {
                    pline_The(
             "consecrated blade flares brightly as it incinerates %s!",
                              mon_nam(mdef));
                    xkilled(mdef, XKILL_NOMSG | XKILL_NOCORPSE);
                }
            } else if (!youattack && !youdefend
                       && magr && is_undead(mdef->data) && j) {
                if (mdef->isvecna || mdef->isvlad) {
                    if (show_instakill)
                        pline_The(
             "consecrated blade flares brightly, severely wounding %s!",
                                  mon_nam(mdef));
                    *dmgptr *= 3;
                    return TRUE;
                } else {
                    if (show_instakill)
                        pline_The(
             "consecrated blade flares brightly as it incinerates %s!",
                                  mon_nam(mdef));
                    if (magr->mtame)
                        set_pet_killer(magr);
                    monkilled(mdef, (char *) 0, AD_DISN);
                }
            } else if (youdefend && l
                       && (maybe_polyd(is_undead(gy.youmonst.data),
                                       Race_if(PM_DRAUGR))
                           || maybe_polyd(is_undead(gy.youmonst.data),
                                          Race_if(PM_VAMPIRE)))) {
                pline_The(
                    "holy power of Sunsword incinerates your undead flesh!");
                if (Upolyd) {
                    /* player returns to their original form */
                    *dmgptr = ((2 * u.mh) + FATAL_DAMAGE_MODIFIER);
                } else {
                    svk.killer.format = NO_KILLER_PREFIX;
                    /* specifically name Sunsword */
                    Sprintf(svk.killer.name, "incinerated by Sunsword");
                    /* the wielder's kill, with nothing left to rise */
                    set_monster_killer(magr, FALSE);
                    done(INCINERATED);
                }
            } else
                return FALSE;
            return TRUE;
        case ART_DRAMBORLEG:
            /* Dramborleg will one-shot kill a balrog every time */
            if (youattack && mdef->data == &mons[PM_BALROG]) {
                You("obliterate %s with a powerful strike!", mon_nam(mdef));
                *dmgptr = (2 * mdef->mhp + FATAL_DAMAGE_MODIFIER);
            } else if (!youattack && !youdefend
                       && magr && mdef->data == &mons[PM_BALROG]) {
                if (show_instakill)
                    pline("%s obliterates %s with a powerful strike!",
                          Monnam(magr), mon_nam(mdef));
                *dmgptr = (2 * mdef->mhp + FATAL_DAMAGE_MODIFIER);
            } else if (youdefend && gy.youmonst.data == &mons[PM_BALROG]) {
                pline_The("magical axe obliterates you!");
                *dmgptr = (2 * (Upolyd ? u.mh : u.uhp)
                           + FATAL_DAMAGE_MODIFIER);
                /* player returns to their original form */
            } else
                return FALSE;
            return TRUE;
        case ART_DEMONBANE:
            if (youattack && is_demon(mdef->data) && j) {
                if (!is_ndemon(mdef->data)) {
                    pline_The("holy mace gravely wounds %s!",
                              mon_nam(mdef));
                    *dmgptr *= 3;
                    return TRUE;
                } else {
                    pline_The("holy mace shines brilliantly, destroying %s!",
                              mon_nam(mdef));
                    *dmgptr = (2 * mdef->mhp + FATAL_DAMAGE_MODIFIER);
                }
            } else if (!youattack && !youdefend
                       && magr && is_demon(mdef->data) && j) {
                if (!is_ndemon(mdef->data)) {
                    if (show_instakill)
                        pline_The("holy mace gravely wounds %s!",
                                  mon_nam(mdef));
                    *dmgptr *= 3;
                    return TRUE;
                } else {
                    if (show_instakill)
                        pline_The(
                            "holy mace shines brilliantly, destroying %s!",
                                  mon_nam(mdef));
                    *dmgptr = (2 * mdef->mhp + FATAL_DAMAGE_MODIFIER);
                }
            } else if (youdefend && k
                       && maybe_polyd(is_demon(gy.youmonst.data),
                                      Race_if(PM_DEMON))) {
                pline("Demonbane shines brilliantly, destroying you!");
                *dmgptr = (2 * (Upolyd ? u.mh : u.uhp)
                           + FATAL_DAMAGE_MODIFIER);
                /* player returns to their original form if poly'd */
            } else
                return FALSE;
            return TRUE;
        case ART_HAMMER_OF_THE_GODS:
            if (youattack && is_demon(mdef->data) && j) {
                if (!is_ndemon(mdef->data)) {
                    pline_The("divine hammer gravely wounds %s!",
                              mon_nam(mdef));
                    *dmgptr *= 3;
                    return TRUE;
                } else {
                    pline_The(
                        "divine hammer shines brilliantly, destroying %s!",
                              mon_nam(mdef));
                    *dmgptr = (2 * mdef->mhp + FATAL_DAMAGE_MODIFIER);
                }
            } else if (youattack && is_undead(mdef->data) && j) {
                if (mdef->isvecna || mdef->isvlad) {
                    pline_The(
                 "divine hammer flares brightly, severely wounding %s!",
                              mon_nam(mdef));
                    *dmgptr *= 3;
                    return TRUE;
                } else {
                    pline_The(
                 "divine hammer flares brightly as it incinerates %s!",
                              mon_nam(mdef));
                    xkilled(mdef, XKILL_NOMSG | XKILL_NOCORPSE);
                }
            } else if (!youattack && !youdefend
                       && magr && is_demon(mdef->data) && j) {
                if (!is_ndemon(mdef->data)) {
                    if (show_instakill)
                        pline_The("divine hammer gravely wounds %s!",
                                  mon_nam(mdef));
                    *dmgptr *= 3;
                    return TRUE;
                } else {
                    if (show_instakill)
                        pline_The(
                        "divine hammer shines brilliantly, destroying %s!",
                                  mon_nam(mdef));
                    *dmgptr = (2 * mdef->mhp + FATAL_DAMAGE_MODIFIER);
                }
            } else if (!youattack && !youdefend
                       && magr && is_undead(mdef->data) && j) {
                if (mdef->isvecna || mdef->isvlad) {
                    if (show_instakill)
                        pline_The(
                 "divine hammer flares brightly, severely wounding %s!",
                                  mon_nam(mdef));
                    *dmgptr *= 3;
                    return TRUE;
                } else {
                    if (show_instakill)
                        pline_The(
                 "divine hammer flares brightly as it incinerates %s!",
                                  mon_nam(mdef));
                    if (magr->mtame)
                        set_pet_killer(magr);
                    monkilled(mdef, (char *) 0, AD_DISN);
                }
            } else if (youdefend && k
                       && maybe_polyd(is_demon(gy.youmonst.data),
                                      Race_if(PM_DEMON))) {
                pline_The(
                    "Hammer of the Gods shines brilliantly, destroying you!");
                *dmgptr = (2 * (Upolyd ? u.mh : u.uhp)
                           + FATAL_DAMAGE_MODIFIER);
                /* player returns to their original form if poly'd */
            } else if (youdefend && l
                       && (maybe_polyd(is_undead(gy.youmonst.data),
                                       Race_if(PM_DRAUGR))
                           || maybe_polyd(is_undead(gy.youmonst.data),
                                          Race_if(PM_VAMPIRE)))) {
                pline_The(
                    "Hammer of the Gods incinerates your undead flesh!");
                if (Upolyd) {
                    /* player returns to their original form */
                    *dmgptr = ((2 * u.mh) + FATAL_DAMAGE_MODIFIER);
                } else {
                    svk.killer.format = NO_KILLER_PREFIX;
                    /* specifically name the Hammer of the Gods */
                    Sprintf(svk.killer.name,
                            "incinerated by the Hammer of the Gods");
                    /* the wielder's kill, with nothing left to rise */
                    set_monster_killer(magr, FALSE);
                    done(INCINERATED);
                }
            } else
                return FALSE;
            return TRUE;
        default:
            break;
        }
    }
    /* We really want "on a natural 20" but Nethack does it in */
    /* reverse from AD&D. */
    if (spec_ability(otmp, SPFX_BEHEAD)) {
        if (is_art(otmp, ART_TSURUGI_OF_MURAMASA) && dieroll == 1) {
            wepdesc = "The razor-sharp blade";
            /* not really beheading, but so close, why add another SPFX */
            if (youattack && engulfing_u(mdef)) {
                You("slice %s wide open!", mon_nam(mdef));
                *dmgptr = 2 * mdef->mhp + FATAL_DAMAGE_MODIFIER;
                return TRUE;
            }
            if (!youdefend) {
                /* allow normal cutworm() call to add extra damage */
                if (gn.notonhead)
                    return FALSE;

                if (bigmonst(mdef->data)) {
                    if (youattack)
                        You("slice deeply into %s!", mon_nam(mdef));
                    else if (vis)
                        pline("%s cuts deeply into %s!", Monnam(magr),
                              hittee);
                    *dmgptr *= 2;
                    return TRUE;
                }
                *dmgptr = 2 * mdef->mhp + FATAL_DAMAGE_MODIFIER;
                pline("%s cuts %s in half!", wepdesc, mon_nam(mdef));
                observe_object(otmp);
                return TRUE;
            } else {
                if (bigmonst(gy.youmonst.data)) {
                    pline("%s cuts deeply into you!",
                          magr ? Monnam(magr) : wepdesc);
                    *dmgptr *= 2;
                    return TRUE;
                }

                /* Players with negative AC's take less damage instead
                 * of just not getting hit.  We must add a large enough
                 * value to the damage so that this reduction in
                 * damage does not prevent death.
                 */
                *dmgptr = 2 * (Upolyd ? u.mh : u.uhp) + FATAL_DAMAGE_MODIFIER;
                pline("%s cuts you in half!", wepdesc);
                observe_object(otmp);
                return TRUE;
            }
        } else if (is_art(otmp, ART_VORPAL_BLADE)
                   && (dieroll == 1 || is_jabberwock(mdef->data))) {
            static const char *const behead_msg[2] = { "%s beheads %s!",
                                                       "%s decapitates %s!" };

            if (youattack && engulfing_u(mdef))
                return FALSE;
            wepdesc = artilist[ART_VORPAL_BLADE].name;
            if (!youdefend) {
                if (!has_head(mdef->data) || gn.notonhead || u.uswallow) {
                    if (youattack)
                        pline("Somehow, you miss %s wildly.", mon_nam(mdef));
                    else if (vis)
                        pline("Somehow, %s misses wildly.", mon_nam(magr));
                    *dmgptr = 0;
                    return (boolean) (youattack || vis);
                }
                if (noncorporeal(mdef->data) || amorphous(mdef->data)) {
                    pline("%s slices through %s %s.", wepdesc,
                          s_suffix(mon_nam(mdef)), mbodypart(mdef, NECK));
                    return TRUE;
                }
                if (mdef->data == &mons[PM_CERBERUS]) {
                    pline("%s removes one of %s heads!", wepdesc,
                          s_suffix(mon_nam(mdef)));
                    *dmgptr = rn2(15) + 10;
                    if (!DEADMONSTER(mdef)) {
                        if (canseemon(mdef))
                            You("watch in horror as it quickly grows back.");
                    }
                    return TRUE;
                }
                *dmgptr = 2 * mdef->mhp + FATAL_DAMAGE_MODIFIER;
                pline(ROLL_FROM(behead_msg), wepdesc,
                      mon_nam(mdef));
                if (Hallucination && !flags.female)
                    pline("Good job Henry, but that wasn't Anne.");
                /* kinda hard to revive if you've lost your head */
                if (racial_zombie(mdef)
                    || (is_troll(mdef->data) && !mlifesaver(mdef)))
                    mdef->mcan = 1;
                observe_object(otmp);
                return TRUE;
            } else {
                if (!has_head(gy.youmonst.data)) {
                    pline("Somehow, %s misses you wildly.",
                          magr ? mon_nam(magr) : wepdesc);
                    *dmgptr = 0;
                    return TRUE;
                }
                if (noncorporeal(gy.youmonst.data)
                    || amorphous(gy.youmonst.data)) {
                    pline("%s slices through your %s.", wepdesc,
                          body_part(NECK));
                    return TRUE;
                }
                if (Hidinshell) {
                    pline("%s glances harmlessly off of your protective %s.",
                          wepdesc, "shell");
                    return TRUE;
                }
                pline(ROLL_FROM(behead_msg), wepdesc, "you");
                observe_object(otmp);
                if (Upolyd) {
                    /* player returns to their original form */
                    *dmgptr = 2 * u.mh + FATAL_DAMAGE_MODIFIER;
                } else {
                    svk.killer.format = NO_KILLER_PREFIX;
                    Snprintf(svk.killer.name, sizeof svk.killer.name,
                             "decapitated by %s", xname(otmp));
                    /* the wielder's kill, but a headless body doesn't
                       rise */
                    set_monster_killer(magr, FALSE);
                    done(DECAPITATED);
                }
                /* Should amulets fall off? */
                return TRUE;
            }
        }
    }
    if (attacks(AD_DRLI, otmp) || spec_ability(otmp, SPFX_DRLI)) {
        /* some non-living creatures (golems, vortices) are vulnerable to
           life drain effects so can get "<Arti> draws the <life>" feedback */
        const char *life = nonliving(mdef->data) ? "animating force" : "life";

        if (!youdefend) {
            int m_lev = (int) mdef->m_lev, /* will be 0 for 1d4 mon */
                mhpmax = mdef->mhpmax,
                drain = monhp_per_lvl(mdef); /* usually 1d8 */
                /* note: DRLI attack uses 2d6, attacker doesn't get healed */

            /* stop draining HP if it drops too low (still drains level;
               also caller still inflicts regular weapon damage) */
            if (mhpmax - drain <= m_lev)
                drain = (mhpmax > m_lev) ? (mhpmax - (m_lev + 1)) : 0;

            if (vis) {
                /* call distant_name() for possible side-effects even if
                   the result won't be printed */
                char *otmpname = distant_name(otmp, xname);

                if (is_art(otmp, ART_STORMBRINGER))
                    pline_The("%s blade draws the %s from %s!",
                              hcolor(NH_BLACK), life, mon_nam(mdef));
                else if (is_art(otmp, ART_LIFESTEALER))
                    pline_The("massive sword draws the %s from %s!",
                              life, mon_nam(mdef));
                else if (is_art(otmp, ART_SHADOWBLADE))
                    pline_The("shadowy blade draws the %s from %s!",
                              life, mon_nam(mdef));
                /* thirsty weapons currently are not allowed but we
                   cover that base here in case they are added someday */
                else if (otmp->oclass == WEAPON_CLASS
                         && (otmp->oprops & ITEM_DRLI))
                    pline_The("deadly %s draws the %s from %s!",
                              otmpname, life, mon_nam(mdef));
                /* the Staff of Aesculapius */
                else
                    pline("%s draws the %s from %s!",
                          The(otmpname), life, mon_nam(mdef));
                if (otmp->oprops & ITEM_DRLI) {
                    otmp->oprops_known |= ITEM_DRLI;
                    update_inventory();
                }
            }
            if (mdef->m_lev == 0) {
                /* losing a level when at 0 is fatal */
                *dmgptr = 2 * mdef->mhp + FATAL_DAMAGE_MODIFIER;
            } else {
                *dmgptr += drain;
                mdef->mhpmax -= drain;
                mdef->m_lev--;
            }

            if (drain > 0) {
                /* drain: was target's damage, now heal attacker by half */
                drain = (drain + 1) / 2; /* drain/2 rounded up */
                if (youattack) {
                    healup(drain, 0, FALSE, FALSE);
                } else {
                    assert(magr != 0);
                    healmon(magr, drain, 0);
                }
            }
            return vis;
        } else { /* youdefend */
            int oldhpmax = u.uhpmax;

            if (Blind) {
                You_feel("an %s drain your %s!",
                         (is_art(otmp, ART_STORMBRINGER)
                          || is_art(otmp, ART_LIFESTEALER))
                            ? "unholy blade"
                            : "object",
                         life);
            } else {
                /* call distant_name() for possible side-effects even if
                   the result won't be printed */
                char *otmpname = distant_name(otmp, xname);

                if (is_art(otmp, ART_STORMBRINGER))
                    pline_The("%s blade drains your %s!",
                              hcolor(NH_BLACK), life);
                else if (is_art(otmp, ART_LIFESTEALER))
                    pline_The("massive sword drains your %s!", life);
                else if (is_art(otmp, ART_SHADOWBLADE))
                    pline_The("shadowy blade drains your %s!", life);
                else if (otmp->oclass == WEAPON_CLASS
                         && (otmp->oprops & ITEM_DRLI))
                    pline_The("deadly %s drains your %s!", otmpname, life);
                /* the Staff of Aesculapius */
                else
                    pline("%s drains your %s!", The(otmpname), life);
            }
            if (otmp->oprops & ITEM_DRLI) {
                otmp->oprops_known |= ITEM_DRLI;
                update_inventory();
            }
            mon_drains_level(magr, "life drainage");
            if (magr && magr->mhp < magr->mhpmax) {
                healmon(magr, (abs(oldhpmax - u.uhpmax) + 1) / 2, 0);
            }
            return TRUE;
        }
    }
    return FALSE;
}

RESTORE_WARNING_FORMAT_NONLITERAL

/* getobj callback for object to be invoked */
staticfn int
invoke_ok(struct obj *obj)
{
    if (!obj)
        return GETOBJ_EXCLUDE;

    /* artifacts and other special items */
    if (obj->oartifact || objects[obj->otyp].oc_unique
        || (obj->otyp == FAKE_AMULET_OF_YENDOR && !obj->known))
        return GETOBJ_SUGGEST;

    /* synonym for apply, though actually invoking it will do different things
     * depending if it's a regular crystal ball, an artifact one that has an
     * invoke power, and a (theoretical) artifact one that doesn't have an
     * invoke power */
    if (obj->otyp == CRYSTAL_BALL)
        return GETOBJ_SUGGEST;

    return GETOBJ_EXCLUDE;
}

/* the #invoke command */
int
doinvoke(void)
{
    struct obj *obj;

    obj = getobj("invoke", invoke_ok, GETOBJ_PROMPT);
    if (!obj)
        return ECMD_CANCEL;
    if (Hidinshell) {
        You_cant("invoke %s while hiding in your shell.",
                 the(distant_name(obj, xname)));
        return ECMD_OK;
    }
    if (!retouch_object(&obj, FALSE, FALSE))
        return ECMD_TIME;
    return arti_invoke(obj);
}

staticfn void
nothing_special(struct obj *obj)
{
    if (carried(obj))
        You_feel("a surge of power, but nothing seems to happen.");
}

staticfn int
invoke_taming(struct obj *obj UNUSED)
{
    struct obj pseudo;

    pseudo = cg.zeroobj; /* zero oextra too */
    pseudo.otyp = SCR_TAMING;
    pseudo.blessed = 1; /* acts as a blessed scroll of taming */
    (void) seffects(&pseudo);
    return ECMD_TIME;
}

staticfn int
invoke_healing(struct obj *obj)
{
    int healamt = (u.uhpmax + 1 - u.uhp) / 2;
    long creamed = (long) u.ucreamed;

    if (Upolyd)
        healamt = (u.mhmax + 1 - u.mh) / 2;
    if (healamt || Sick || Slimed || Blinded > creamed)
        You_feel("better.");
    if (healamt || Sick || Slimed || Withering || BlindedTimeout > creamed)
        You_feel("%sbetter.",
                 (!healamt && !Sick && !Slimed
                  /* when healing temporary blindness (aside from
                     goop covering face), might still be blind
                     due to PermaBlind or eyeless polymorph;
                     vary the message in that situation */
                  && (HBlinded & ~TIMEOUT) != 0L) ? "slightly " : "");
    else {
        nothing_special(obj);
        return ECMD_TIME;
    }
    if (healamt > 0) {
        if (Upolyd)
            u.mh += healamt;
        else
            u.uhp += healamt;
    }
    if (Sick)
        make_sick(0L, (char *) 0, FALSE, SICK_ALL);
    if (Slimed)
        make_slimed(0L, (char *) 0);
    if (Withering)
        set_itimeout(&HWithering, 0L);
    if (BlindedTimeout > creamed)
        make_blinded(creamed, FALSE);
    disp.botl = TRUE;
    return ECMD_TIME;
}

staticfn int
invoke_energy_boost(struct obj *obj)
{
    int epboost = (u.uenmax + 1 - u.uen) / 2;

    if (epboost > 120)
        epboost = 120; /* arbitrary */
    else if (epboost < 12)
        epboost = u.uenmax - u.uen;
    if (epboost) {
        u.uen += epboost;
        disp.botl = TRUE;
        You_feel("re-energized.");
    } else {
        nothing_special(obj);
        return ECMD_TIME;
    }
    return ECMD_TIME;
}

staticfn int
invoke_untrap(struct obj *obj)
{
    if (!untrap(TRUE, 0, 0, (struct obj *) 0)) {
        obj->age = 0; /* don't charge for changing their mind */
        return ECMD_CANCEL;
    }
    return ECMD_TIME;
}

staticfn int
invoke_charge_obj(struct obj *obj)
{
    const struct artifact *oart = get_artifact(obj);
    struct obj *otmp = getobj("charge", charge_ok,
                              GETOBJ_PROMPT | GETOBJ_ALLOWCNT);
    boolean b_effect;

    if (!otmp) {
        obj->age = 0;
        return ECMD_CANCEL;
    }
    b_effect = (obj->blessed && (oart->role == Role_switch
                                 || oart->role == NON_PM));
    recharge(otmp, b_effect ? 1 : obj->cursed ? -1 : 0);
    update_inventory();
    return ECMD_TIME;
}

staticfn int
invoke_create_portal(struct obj *obj)
{
    int i, num_ok_dungeons, last_ok_dungeon = 0;
    d_level newlev;
    winid tmpwin;
    anything any;
    int clr = NO_COLOR;

    /* once the Amulet of Yendor has been obtained (or the Idol of
       Moloch imbued for Infidels), portaling out of the sanctum is
       prohibited, even after dropping it */
    if (Is_sanctum(&u.uz) && achievement_recorded(ACH_AMUL)
        && (!Role_if(PM_INFIDEL) || u.uevent.uimbued_idol)) {
        nothing_special(obj);
        return ECMD_TIME;
    }

    tmpwin = create_nhwindow(NHW_MENU);
    any = cg.zeroany; /* set all bits to zero */
    start_menu(tmpwin, MENU_BEHAVE_STANDARD);
    /* use index+1 (can't use 0) as identifier */
    for (i = num_ok_dungeons = 0; i < svn.n_dgns; i++) {
        if (!svd.dungeons[i].dunlev_ureached)
            continue;
        if (i == tutorial_dnum) /* can't portal into tutorial */
            continue;
        any.a_int = i + 1;
        add_menu(tmpwin, &nul_glyphinfo, &any, 0, 0,
                 ATR_NONE, clr,
                 svd.dungeons[i].dname, MENU_ITEMFLAGS_NONE);
        num_ok_dungeons++;
        last_ok_dungeon = i;
    }
    end_menu(tmpwin, "Open a portal to which dungeon?");
    if (num_ok_dungeons > 1) {
        /* more than one entry; display menu for choices */
        menu_item *selected;
        int n;

        n = select_menu(tmpwin, PICK_ONE, &selected);
        if (n <= 0) {
            destroy_nhwindow(tmpwin);
            nothing_special(obj);
            return ECMD_TIME;
        }
        i = selected[0].item.a_int - 1;
        free((genericptr_t) selected);
    } else
        i = last_ok_dungeon; /* also first & only OK dungeon */
    destroy_nhwindow(tmpwin);

    /*
     * i is now index into dungeon structure for the new dungeon.
     * Find the closest level in the given dungeon, open
     * a use-once portal to that dungeon and go there.
     * The closest level is either the entry or dunlev_ureached.
     */
    newlev.dnum = i;
    if (svd.dungeons[i].depth_start >= depth(&u.uz))
        newlev.dlevel = svd.dungeons[i].entry_lev;
    else
        newlev.dlevel = svd.dungeons[i].dunlev_ureached;

    if (u.uhave.amulet || In_endgame(&u.uz) || In_endgame(&newlev)
        || newlev.dnum == u.uz.dnum || !next_to_u()) {
        You_feel("very disoriented for a moment.");
    } else {
        if (!Blind)
            You("are surrounded by a shimmering sphere!");
        else
            You_feel("weightless for a moment.");
        /* update hero location and the display before leaving the
           level; goto_level() releases any non-engulfing hold */
        if (u.uswallow)
            unstuck(u.ustuck);
        goto_level(&newlev, FALSE, FALSE, FALSE);
    }
    return ECMD_TIME;
}

staticfn int
invoke_create_ammo(struct obj *obj)
{
    struct obj *otmp =
        mksobj((obj->otyp == CROSSBOW
                || obj->otyp == DARK_ELVEN_HAND_CROSSBOW)
                   ? (Race_if(PM_DROW) ? DARK_ELVEN_CROSSBOW_BOLT
                                       : CROSSBOW_BOLT)
                   : (Race_if(PM_ELF) ? ELVEN_ARROW
                      : Race_if(PM_DROW) ? DARK_ELVEN_ARROW
                        : Race_if(PM_ORC) ? ORCISH_ARROW
                          : ARROW), TRUE, FALSE);

    if (!otmp) {
        nothing_special(obj);
        return ECMD_TIME;
    }
    otmp->blessed = obj->blessed;
    otmp->cursed = obj->cursed;
    otmp->bknown = obj->bknown;
    otmp->oeroded = otmp->oeroded2 = 0;
    if (obj->blessed) {
        if (otmp->spe < 0)
            otmp->spe = 0;
        otmp->quan += rnd(10);
        if (!rn2(5))
            (void) create_oprop(otmp, FALSE);
    } else if (obj->cursed) {
        if (otmp->spe > 0)
            otmp->spe = 0;
    } else
        otmp->quan += rnd(5);
    otmp->owt = weight(otmp);
    otmp->oprops_known |= obj->oprops_known;
    otmp = hold_another_object(otmp, "Suddenly %s out.",
                               aobjnam(otmp, "fall"), (char *) 0);
    nhUse(otmp);
    return ECMD_TIME;
}

staticfn int
invoke_banish(struct obj *obj UNUSED)
{
    int nvanished = 0, nstayed = 0;
    struct monst *mtmp, *mtmp2;
    d_level dest;

    find_hell(&dest);

    for (mtmp = fmon; mtmp; mtmp = mtmp2) {
        int chance = 1;

        mtmp2 = mtmp->nmon;
        if (DEADMONSTER(mtmp) || !isok(mtmp->mx, mtmp->my))
            continue;
        if (!is_demon(mtmp->data) && mtmp->data->mlet != S_IMP)
            continue;
        if (!couldsee(mtmp->mx, mtmp->my))
            continue;
        if (mtmp->data->msound == MS_NEMESIS)
            continue;

        if (In_quest(&u.uz) && !svq.quest_status.killed_nemesis)
            chance += 10;
        if (is_dprince(mtmp->data))
            chance += 2;
        if (is_dlord(mtmp->data))
            chance++;

        mtmp->msleeping = mtmp->mtame = mtmp->mpeaceful = 0;
        if (chance <= 1 || !rn2(chance)) {
            if (!Inhell) {
                nvanished++;
                /* banish to a random level in Gehennom */
                dest.dlevel = rn2(dunlevs_in_dungeon(&dest));
                migrate_mon(mtmp, ledger_no(&dest), MIGR_RANDOM);
            } else {
                u_teleport_mon(mtmp, FALSE);
            }
        } else {
            nstayed++;
        }
    }

    if (nvanished) {
        char subject[] = "demons";

        if (nvanished == 1)
            *(eos(subject) - 1) = '\0'; /* remove 's' */
        pline("%s %s %s in a cloud of brimstone!",
              nstayed ? ((nvanished > nstayed)
                         ? "Most of the"
                         : "Some of the")
              : "The",
              subject, vtense(subject, "disappear"));
    }
    return ECMD_TIME;
}

staticfn int
invoke_fling_poison(struct obj *obj)
{
    if (getdir((char *) 0)) {
        int venom = rn2(2) ? BLINDING_VENOM : ACID_VENOM;
        struct obj *otmp = mksobj(venom, TRUE, FALSE);

        otmp->spe = 1; /* the poison is yours */
        throwit(otmp, 0L, FALSE, (struct obj *) 0);
    } else {
        /* no direction picked */
        pline("%s", Never_mind);
        obj->age = svm.moves;
        return ECMD_CANCEL;
    }
    return ECMD_TIME;
}

staticfn int
invoke_storm_spell(struct obj *obj)
{
    const struct artifact *oart = get_artifact(obj);
    int storm = oart->inv_prop == SNOWSTORM ? SPE_CONE_OF_COLD : SPE_FIREBALL;
    int skill = spell_skilltype(storm);
    int expertise = P_SKILL(skill);

    P_SKILL(skill) = P_EXPERT;
    (void) spelleffects(storm, FALSE, TRUE);
    P_SKILL(skill) = expertise;
    return ECMD_TIME;
}

staticfn int
invoke_blinding_ray(struct obj *obj)
{
    if (getdir((char *) 0)) {
        if (u.dx || u.dy) {
            do_blinding_ray(obj);
        } else if (u.dz) {
            /* up or down => light this map spot; litroom() uses
               radius 0 for Sunsword, except on Rogue level where
               whole room gets lit and corridor spots remain unlit */
            litroom(TRUE, FALSE, obj, u.ux, u.uy);
            pline("%s", ((!Blind && levl[u.ux][u.uy].lit
                          && !levl[u.ux][u.uy].waslit)
                         ? "It is lit here now."
                         : nothing_seems_to_happen));
        } else { /* zapyourself() */
            boolean vulnerable = (u.umonnum == PM_GREMLIN);
            int damg = obj->blessed ? 15 : !obj->cursed ? 10 : 5;

            if (vulnerable) /* could be fatal if Unchanging */
                (void) lightdamage(obj, (struct monst *) 0, TRUE, 2 * damg);

            if (!flashburn((long) (damg + rnd(damg)), FALSE)
                && !vulnerable)
                pline("%s", nothing_seems_to_happen);
        }
    } else {
        /* no direction picked */
        pline("%s", Never_mind);
        obj->age = svm.moves;
        return ECMD_CANCEL;
    }
    return ECMD_TIME;
}

staticfn int
invoke_shadowblade(struct obj *obj)
{
    struct monst *mtmp;
    int ct = 0;

    if (y_n("Do you want to cast an aura of darkness?") == 'n') {
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
            if (DEADMONSTER(mtmp))
                continue;
            if (cansee(mtmp->mx, mtmp->my)) {
                if (!resist(mtmp, WEAPON_CLASS, 0, NOTELL)
                    /* some monsters are immune */
                    && onscary(0, 0, mtmp))
                    monflee(mtmp, 0, FALSE, TRUE);
                if (!mtmp->mtame)
                    ct++; /* pets don't laugh at you */
            }
        }
        if (!Deaf && ct)
            You_hear("%s laughter coming from %s!",
                     rn2(2) ? "fiendish" : "maniacal", xname(obj));
    } else {
        litroom(FALSE, FALSE, obj, u.ux, u.uy);
    }
    return ECMD_TIME;
}

/* walk through walls and stone like a xorn */
staticfn int
invoke_phasing(struct obj *obj)
{
    if (is_art(obj, ART_STRIPED_SHIRT_OF_LIBERATION)
        && !Role_if(PM_CONVICT) && rn2(5)) {
        You_feel("that %s %s purposely ignoring you.",
                 the(xname(obj)), otense(obj, "are"));
        return ECMD_TIME;
    }
    if (Passes_walls) {
        nothing_special(obj);
        return ECMD_TIME;
    }
    if (!Hallucination)
        Your("body begins to feel less solid.");
    else
        You_feel("one with the spirit world.");
    incr_itimeout(&HPasses_walls, 50 + rnd(100));
    obj->age += HPasses_walls; /* time begins after phasing ends */
    if (u.uswallow) {
        struct monst *mon = u.ustuck;

        expels(mon, mon->data, FALSE);
        You("exit %s, phasing right through %s %s!", mon_nam(mon),
            mhis(mon), mbodypart(mon, STOMACH));
    }
    return ECMD_TIME;
}

staticfn int
invoke_channel(struct obj *obj)
{
    /* should this break atheist conduct? currently it doesn't, under
       the excuse of being necessary to ascend, even though this is
       channeling a god's power */
    if (!IS_ALTAR(levl[u.ux][u.uy].typ)) {
        obj->age = 0; /* figurine activation manages the invoke cost */
        return use_figurine(&obj);
    } else {
        aligntyp altar_align = Amask2align(levl[u.ux][u.uy].altarmask
                                           & AM_MASK);
        boolean high_altar = ((Is_astralevel(&u.uz) || Is_sanctum(&u.uz))
                              && (levl[u.ux][u.uy].altarmask & AM_SANCTUM));

        if (!Blind)
            pline("Tendrils of %s mist seep out of %s "
                  "and into the altar below...",
                  hcolor("crimson"), the(xname(obj)));
        else
            You_feel("something flow from %s.", the(xname(obj)));
        if (altar_align == A_NONE) {
            if (high_altar && Role_if(PM_INFIDEL)
                && u.uevent.uimbued_idol) {
                godvoice(A_NONE, (char *) 0);
                qt_pager("moloch_offer_idol");
                You_feel("strange energies envelop %s.", the(xname(obj)));
                u.uhave.amulet = 1;
                u.uamulet_on_planes = 0;
                return ECMD_TIME;
            }
            if (!Blind)
                pline_The("altar glows for a moment.");
            /* nothing happens */
            return ECMD_TIME;
        }
        if (high_altar) {
            /* messages yoinked from pray.c */
            You("sense a conflict between %s and %s.",
                align_gname(A_NONE), a_gname());
            if (u.uevent.uimbued_idol && altar_align == inf_align(1)) {
                You_feel("the power of %s increase.", align_gname(A_NONE));
            } else {
                pline("%s feel the power of %s decrease.",
                      u.ualign.type == A_NONE ? "Unluckily, you" : "You",
                      align_gname(A_NONE));
                godvoice(altar_align,
                         "So, mortal!  You dare desecrate my High Temple!");
                god_zaps_you(altar_align);
                return ECMD_TIME;
            }
        }
        levl[u.ux][u.uy].altarmask &= (AM_SHRINE | AM_SANCTUM);
        levl[u.ux][u.uy].altarmask |= AM_NONE;
        if (!Blind)
            pline_The("altar glows %s.", hcolor(NH_RED));
        if (!high_altar) {
            /* the Idol does all the work for you, so you don't get a
               luck increase; but you don't get a hostile minion,
               either */
            struct monst *pri = findpriest(temple_occupied(u.urooms));

            if (pri && mon_aligntyp(pri) != A_NONE)
                angry_priest();
        } else {
            /* at this point the player must be an Infidel; should we
               still check for opposite alignment? currently Moloch
               doesn't care */
            adjalign(10);
            u.uevent.ascended = 1;
            pline1("A sinister laughter echoes through the temple, "
                   "and you're bathed in darkness...");
            godvoice(A_NONE, "My pawn, thou hast done well!");
            display_nhwindow(WIN_MESSAGE, FALSE);
            SetVoice((struct monst *) 0, 0, 80, voice_deity);
            verbalize("In return for thy service, "
                      "I grant thee a part of My domain!");
            You("ascend, becoming the Archfiend of Moloch...");
            done(ASCENDED);
            /*NOTREACHED*/
        }
    }
    return ECMD_TIME;
}

/* a peaceful or tame monster has survived the invoked wrath of the Eye
   or Hand of Vecna; it will not stay friendly toward the invoker */
staticfn void
death_magic_anger(struct monst *mtmp)
{
    if (mtmp->mpeaceful || mtmp->mtame) {
        mtmp->mpeaceful = mtmp->mtame = 0;
        if (mtmp->mleashed)
            m_unleash(mtmp, TRUE);
        set_malign(mtmp);
        newsym(mtmp->mx, mtmp->my);
        if (u.ualign.type != A_NONE) {
            You_feel("distraught.");
            adjalign(-3);
            record_abuse_event_mon(-3, ABUSE_SPELL_PEACEFUL, mtmp);
        }
    }
}

staticfn int
invoke_death_magic(struct obj *obj)
{
    struct monst *mtmp, *nextmon;

    if (u.uluck < -9) {
        /* being immune to death magic doesn't help in this instance */
        pline("%s turns on you!", The(xname(obj)));
        u.uhp = 0;
        svk.killer.format = KILLED_BY;
        if (is_art(obj, ART_EYE_OF_VECNA))
            Strcpy(svk.killer.name, "the Eye of Vecna");
        else
            Strcpy(svk.killer.name, "the Hand of Vecna");
        done(DIED);
    } else {
        if (is_art(obj, ART_EYE_OF_VECNA)) {
            pline("%s looks around with its icy gaze!", The(xname(obj)));
        /* the Hand of Vecna needs to be 'worn' for it to be invoked */
        } else if (uarmg && is_art(uarmg, ART_HAND_OF_VECNA)) {
            You("hold %s up high above your %s!",
                the(xname(obj)), body_part(HEAD));
        } else if (is_art(obj, ART_HAND_OF_VECNA)) {
            You("hold %s up high above your %s, but nothing happens.",
                the(xname(obj)), body_part(HEAD));
            return ECMD_TIME;
        }

        for (mtmp = fmon; mtmp; mtmp = nextmon) {
            nextmon = mtmp->nmon;
            if (DEADMONSTER(mtmp))
                continue;
            /* the Eye is never blind */
            if (is_art(obj, ART_EYE_OF_VECNA)
                    ? !couldsee(mtmp->mx, mtmp->my)
                    : !canspotmon(mtmp))
                continue;
            if (immune_death_magic(mtmp->data)) {
                shieldeff(mtmp->mx, mtmp->my);
                pline("%s is not affected by %s.", Monnam(mtmp),
                      the(distant_name(obj, xname)));
                continue;
            }
            switch (rn2(20)) {
            case 19:
            case 18:
            case 17:
            case 16:
            case 15:
                if (!(resists_magm(mtmp) || defended(mtmp, AD_MAGM))) {
                    pline_The("%s %s %s!", distant_name(obj, xname),
                              rn2(2) ? "annihilates" : "obliterates",
                              mon_nam(mtmp));
                    mtmp->mhp = 0;
                    /* the artifact does the killing, not the hero;
                       no experience or murder penalty (the alignment
                       and luck costs below are the price) */
                    monkilled(mtmp, (char *) 0, AD_DETH);
                    if (!DEADMONSTER(mtmp)) /* lifesaved */
                        continue;
                } else { /* MR or resists */
                    shieldeff(mtmp->mx, mtmp->my);
                    pline("%s %s in pain, but resists %s deadly %s.",
                          Monnam(mtmp), makeplural(growl_sound(mtmp)),
                          the(s_suffix(xname(obj))),
                          is_art(obj, ART_EYE_OF_VECNA) ? "gaze" : "aura");
                    (void) damage_mon_nonlethal(mtmp, d(6, 4), AD_DETH,
                                                TRUE);
                    death_magic_anger(mtmp);
                }
                break;
            default: /* case 14 through case 0 */
                if (!(resists_magm(mtmp) || defended(mtmp, AD_MAGM))) {
                    if (!Deaf)
                        pline("%s %s in %s!", Monnam(mtmp),
                              makeplural(growl_sound(mtmp)),
                              rn2(2) ? "agony" : "pain");
                    else if (cansee(mtmp->mx, mtmp->my))
                        pline("%s trembles in %s!", Monnam(mtmp),
                              rn2(2) ? "agony" : "pain");
                    /* mhp will then still be less than this value */
                    mtmp->mhpmax -= rn1(9, 4);
                    if (mtmp->mhpmax < 1) /* protect against invalid value */
                        mtmp->mhpmax = 1;
                    mtmp->mhp /= 2;
                    if (mtmp->mhp < 1)
                        mtmp->mhp = 1;
                    death_magic_anger(mtmp);
                } else { /* MR or resists */
                    shieldeff(mtmp->mx, mtmp->my);
                    pline("%s %s in pain, but resists %s deadly %s.",
                          Monnam(mtmp), makeplural(growl_sound(mtmp)),
                          the(s_suffix(xname(obj))),
                          is_art(obj, ART_EYE_OF_VECNA) ? "gaze" : "aura");
                    (void) damage_mon_nonlethal(mtmp, d(4, 4), AD_DETH,
                                                TRUE);
                    death_magic_anger(mtmp);
                }
                break;
            }
        }
    }
    /* use at your own risk */
    if (u.ualign.type != A_NONE) {
        if (u.ualign.type == A_LAWFUL) {
            You_feel("very guilty.");
            adjalign(-7);
            record_abuse_event(-7, ABUSE_VECNA);
        } else {
            You_feel("guilty.");
            adjalign(-3);
            record_abuse_event(-3, ABUSE_VECNA);
        }
    }
    /* Infidels get a bit more leeway from Moloch */
    change_luck((u.ualign.type == A_NONE) ? -1 : -3);
    exercise(A_WIS, FALSE);
    return ECMD_TIME;
}

staticfn int
invoke_command_undead(struct obj *obj UNUSED)
{
    struct monst *mtmp;

    if (u.uswallow) {
        if (is_undead(u.ustuck->data))
            (void) tamedog(u.ustuck, (struct obj *) 0, FALSE);
    } else {
        int i, j, bd = 1;

        for (i = -bd; i <= bd; i++) {
            for (j = -bd; j <= bd; j++) {
                if (!isok(u.ux + i, u.uy + j))
                    continue;
                if ((mtmp = m_at(u.ux + i, u.uy + j)) != 0
                    && is_undead(mtmp->data))
                    (void) tamedog(mtmp, (struct obj *) 0, FALSE);
            }
        }
    }
    return ECMD_TIME;
}

/* return the amount of Pw invoking an object costs.
   return a negative value, if obj invoking cannot be paid with Pw */
staticfn int
arti_invoke_cost_pw(struct obj *obj)
{
    const struct artifact *oart = get_artifact(obj);

    if (oart->inv_prop == FLING_POISON
        || oart->inv_prop == BLINDING_RAY) {
        /* pretend it's a level 5 spell */
        return SPELL_LEV_PW(5);
    }

    return -1;
}

/* return TRUE if artifact object's invoke cost can be paid (and pay it) */
staticfn boolean
arti_invoke_cost(struct obj *obj)
{
    if (obj->age > svm.moves) {
        int pw_cost = arti_invoke_cost_pw(obj);

        if (pw_cost < 0 || u.uen < pw_cost) {
            /* the artifact is tired :-) */
            You_feel("that %s %s ignoring you.", the(xname(obj)),
                     otense(obj, "are"));
            /* and just got more so; patience is essential... */
            obj->age += (long) d(3, 10);
            return FALSE;
        } else {
            /* you pay invoke cost with your own magic */
            You_feel("drained...");
            u.uen -= pw_cost;
            disp.botl = TRUE;
        }
    } else {
        obj->age = svm.moves + rnz(100);
    }
    return TRUE;
}

staticfn int
arti_invoke(struct obj *obj)
{
    const struct artifact *oart;
    int res = ECMD_OK;

    if (!obj) {
        impossible("arti_invoke without obj");
        return ECMD_OK;
    }
    oart = get_artifact(obj);
    if (oart == &artilist[ART_NONARTIFACT] || !oart->inv_prop) {
        if (obj->otyp == CRYSTAL_BALL)
            use_crystal_ball(&obj);
        else
            pline1(nothing_happens);
        return ECMD_TIME;
    }

    /* an aasimar's celestial nature recoils from Vecna's unholy relics */
    if (vecna_relic_denied(obj)) {
        You("recoil from %s; you cannot bring yourself to channel"
            " such unholy power.", the(xname(obj)));
        return ECMD_TIME;
    }

    /* It's a special power, not "just" a property */
    if (oart->inv_prop > LAST_PROP) {
        if (!arti_invoke_cost(obj))
            return ECMD_TIME;

        switch (oart->inv_prop) {
        case TAMING: res = invoke_taming(obj); break;
        case HEALING: res = invoke_healing(obj); break;
        case ENERGY_BOOST: res = invoke_energy_boost(obj); break;
        case UNTRAP: res = invoke_untrap(obj); break;
        case CHARGE_OBJ: res = invoke_charge_obj(obj); break;
        case LEV_TELE: level_tele(); res = ECMD_TIME; break;
        case CREATE_PORTAL: res = invoke_create_portal(obj); break;
        case ENLIGHTENING:
            enlightenment(MAGICENLIGHTENMENT, ENL_GAMEINPROGRESS);
            res = ECMD_TIME;
            break;
        case CREATE_AMMO: res = invoke_create_ammo(obj); break;
        case BANISH: res = invoke_banish(obj); break;
        case FLING_POISON: res = invoke_fling_poison(obj); break;
        case SNOWSTORM:
            /*FALLTHRU*/
        case FIRESTORM: res = invoke_storm_spell(obj); break;
        case BLINDING_RAY: res = invoke_blinding_ray(obj); break;
        case SHADOWBLADE: res = invoke_shadowblade(obj); break;
        case PHASING: res = invoke_phasing(obj); break;
        case CHANNEL: res = invoke_channel(obj); break;
        case DEATH_MAGIC: res = invoke_death_magic(obj); break;
        case COMMAND_UNDEAD: res = invoke_command_undead(obj); break;
        default:
            impossible("Unknown invoke power %d.", oart->inv_prop);
            break;
        }
        return res;
    } else {
        long eprop = (u.uprops[oart->inv_prop].extrinsic ^= W_ARTI),
             iprop = u.uprops[oart->inv_prop].intrinsic;
        boolean on = (eprop & W_ARTI) != 0; /* true if prop just set */

        if (on && obj->age > svm.moves) {
            /* the artifact is tired :-) */
            u.uprops[oart->inv_prop].extrinsic ^= W_ARTI;
            You_feel("that %s %s ignoring you.", the(xname(obj)),
                     otense(obj, "are"));
            /* can't just keep repeatedly trying */
            obj->age += (long) d(3, 10);
            return ECMD_TIME;
        } else if (!on) {
            /* when turning off property, determine downtime */
            /* arbitrary for now until we can tune this -dlc */
            obj->age = svm.moves + rnz(100);
        }

        if ((eprop & ~W_ARTI) || iprop) {
            /* you had the property from some other source too */
            nothing_special(obj);
            return ECMD_TIME;
        }
        switch (oart->inv_prop) {
        case CONFLICT:
            if (on)
                You_feel("like a rabble-rouser.");
            else
                You_feel("the tension decrease around you.");
            break;
        case LEVITATION:
            if (on) {
                float_up();
                spoteffects(FALSE);
            } else if (iflags.in_lava_effects) {
                /* erode_obj() is destroying the artifact (or lava_effects()
                   burning it off a hero already down in the lava, whose
                   levitation was blocked); float_down() can't run under
                   either, so do what Boots_off() does there */
                boolean was_levitating = !BLevitation;

                float_vs_flight();
                disp.botl = TRUE;
                if (was_levitating) {
                    if (Flying)
                        You("have stopped levitating and are now flying.");
                    else
                        defer_landing(LANDING_FLOAT);
                }
            } else {
                (void) float_down(I_SPECIAL | TIMEOUT, W_ARTI);
            }
            break;
        case INVIS:
            if (BInvis || Blind) {
                nothing_special(obj);
                return ECMD_TIME;
            }
            newsym(u.ux, u.uy);
            if (on)
                Your("body takes on a %s transparency...",
                     Hallucination ? "normal" : "strange");
            else
                Your("body seems to unfade...");
            break;
        }
    }

    return ECMD_TIME;
}

/* will freeing this object from inventory cause levitation to end? */
boolean
finesse_ahriman(struct obj *obj)
{
    const struct artifact *oart;
    struct prop save_Lev;
    boolean result;

    /* if we aren't levitating or this isn't an artifact which confers
       levitation via #invoke then freeinv() won't toggle levitation */
    if (!Levitation
        || (oart = get_artifact(obj)) == &artilist[ART_NONARTIFACT]
        || oart->inv_prop != LEVITATION || !(ELevitation & W_ARTI))
        return FALSE;

    /* arti_invoke(off) -> float_down() clears I_SPECIAL|TIMEOUT & W_ARTI;
       probe ahead to see whether that actually results in floating down;
       (this assumes that there aren't two simultaneously invoked artifacts
       both conferring levitation--safe, since if there were two of them,
       invoking the 2nd would negate the 1st rather than stack with it) */
    save_Lev = u.uprops[LEVITATION];
    HLevitation &= ~(I_SPECIAL | TIMEOUT);
    ELevitation &= ~W_ARTI;
    result = (boolean) !Levitation;
    u.uprops[LEVITATION] = save_Lev;
    return result;
}

/* WAC return TRUE if artifact is always lit */
boolean
artifact_light(struct obj *obj)
{
    /* not artifacts but treat them as if they were because they emit
       light without burning; shadow dragon scales emit darkness */
    if (obj && Is_dragon_armor(obj)
        && (Dragon_armor_to_scales(obj) == GOLD_DRAGON_SCALES
            || Dragon_armor_to_scales(obj) == SHADOW_DRAGON_SCALES
            || Dragon_armor_to_scales(obj) == CHROMATIC_DRAGON_SCALES)
        && (obj->owornmask & (W_ARM | W_ARMC)) != 0L)
        return TRUE;

    if (obj && obj->otyp == SHIELD_OF_LIGHT
        && (obj->owornmask & W_ARMS) != 0L)
        return TRUE;

    return (boolean) ((get_artifact(obj) != &artilist[ART_NONARTIFACT])
                      && (is_art(obj, ART_SUNSWORD)
                          || is_art(obj, ART_STAFF_OF_THE_ARCHMAGI)
                          || is_art(obj, ART_HAMMER_OF_THE_GODS)));
}

/* KMH -- Talking artifacts are finally implemented */
int
arti_speak(struct obj *obj)
{
    const struct artifact *oart = get_artifact(obj);
    const char *line;
    char buf[BUFSZ];

    /* Is this a speaking artifact? */
    if (oart == &artilist[ART_NONARTIFACT] || !(oart->spfx & SPFX_SPEAK))
        return ECMD_OK; /* nothing happened */

    line = getrumor(bcsign(obj), buf, TRUE);
    if (!*line)
        line = "NetHack rumors file closed for renovation.";
    pline("%s:", Tobjnam(obj, "whisper"));
    SetVoice((struct monst *) 0, 0, 80, voice_talking_artifact);
    verbalize1(line);
    return ECMD_TIME;
}

boolean
artifact_has_invprop(struct obj *otmp, uchar inv_prop)
{
    const struct artifact *arti = get_artifact(otmp);

    return (boolean) ((arti != &artilist[ART_NONARTIFACT])
                      && (arti->inv_prop == inv_prop));
}

/* Return the price sold to the hero of a given artifact or unique item */
long
arti_cost(struct obj *otmp)
{
    if (!otmp->oartifact)
        return (long) objects[otmp->otyp].oc_cost;
    else if (artilist[(int) otmp->oartifact].cost)
        return artilist[(int) otmp->oartifact].cost;
    else
        return (100L * (long) objects[otmp->otyp].oc_cost);
}

staticfn uchar
abil_to_adtyp(long *abil)
{
    struct abil2adtyp_tag {
        long *abil;
        uchar adtyp;
    } abil2adtyp[] = {
        { &EFire_resistance, AD_FIRE },
        { &ECold_resistance, AD_COLD },
        { &EShock_resistance, AD_ELEC },
        { &EAntimagic, AD_MAGM },
        { &EDisint_resistance, AD_DISN },
        { &EPoison_resistance, AD_DRST },
        { &EDrain_resistance, AD_DRLI },
    };
    int k;

    for (k = 0; k < SIZE(abil2adtyp); k++) {
        if (abil2adtyp[k].abil == abil)
            return abil2adtyp[k].adtyp;
    }
    return 0;
}

staticfn unsigned long
abil_to_spfx(long *abil)
{
    static const struct abil2spfx_tag {
        long *abil;
        unsigned long spfx;
    } abil2spfx[] = {
        { &ESearching, SPFX_SEARCH },
        { &EHalluc_resistance, SPFX_HALRES },
        { &ETelepat, SPFX_ESP },
        { &EStealth, SPFX_STLTH },
        { &ERegeneration, SPFX_REGEN },
        { &ETeleport_control, SPFX_TCTRL },
        { &EWarn_of_mon, SPFX_WARN },
        { &EWarning, SPFX_WARN },
        { &EEnergy_regeneration, SPFX_EREGEN },
        { &EHalf_spell_damage, SPFX_HSPDAM },
        { &EHalf_physical_damage, SPFX_HPHDAM },
        { &EReflecting, SPFX_REFLECT },
    };
    int k;

    for (k = 0; k < SIZE(abil2spfx); k++) {
        if (abil2spfx[k].abil == abil)
            return abil2spfx[k].spfx;
    }
    return 0L;
}

/*
 * Return the first item that is conveying a particular intrinsic.
 */
struct obj *
what_gives(long *abil)
{
    struct obj *obj;
    uchar dtyp;
    unsigned long spfx;
    long wornbits;
    long wornmask = (W_ARM | W_ARMC | W_ARMH | W_ARMS
                     | W_ARMG | W_ARMF | W_ARMU
                     | W_AMUL | W_RINGL | W_RINGR | W_TOOL
                     | W_ART | W_ARTI);

    if (u.twoweap)
        wornmask |= W_SWAPWEP;
    dtyp = abil_to_adtyp(abil);
    spfx = abil_to_spfx(abil);
    wornbits = (wornmask & *abil);

    for (obj = gi.invent; obj; obj = obj->nobj) {
        if (obj->oartifact
            && (abil != &EWarn_of_mon || svc.context.warntype.obj)) {
            const struct artifact *art = get_artifact(obj);

            if (art != &artilist[ART_NONARTIFACT]) {
                if (dtyp) {
                    if (art->cary.adtyp == dtyp /* carried */
                        || (art->defn.adtyp == dtyp /* defends while worn */
                            && (obj->owornmask & ~(W_ART | W_ARTI))))
                        return obj;
                }
                if (spfx) {
                    /* property conferred when carried */
                    if ((art->cspfx & spfx) == spfx)
                        return obj;
                    /* property conferred when wielded or worn */
                    if ((art->spfx & spfx) == spfx && obj->owornmask)
                        return obj;
                }
                if (obj == uwep && abil == &EBlnd_resist
                    && (*abil & W_WEP) != 0L) {
                    return obj; /* Sunsword */
                }
            }
        } else {
            if (wornbits && wornbits == (wornmask & obj->owornmask))
                return obj;
        }
    }
    return (struct obj *) 0;
}

const char *
glow_color(int arti_indx)
{
    int colornum = artilist[arti_indx].acolor;
    const char *colorstr = clr2colorname(colornum);

    return hcolor(colorstr);
}

/* glow verb; [0] holds the value used when blind */
static const char *const glow_verbs[] = {
    "quiver", "flicker", "glimmer", "gleam"
};

/* relative strength that Sting is glowing (0..3), to select verb */
staticfn int
glow_strength(int count)
{
    /* glow strength should also be proportional to proximity and
       probably difficulty, but we don't have that information and
       gathering it is more trouble than this would be worth */
    return (count > 12) ? 3 : (count > 4) ? 2 : (count > 0);
}

const char *
glow_verb(int count, /* 0 means blind rather than no applicable creatures */
          boolean ingsfx)
{
    static char resbuf[20];

    Strcpy(resbuf, glow_verbs[glow_strength(count)]);
    /* ing_suffix() will double the last consonant for all the words
       we're using and none of them should have that, so bypass it */
    if (ingsfx)
        Strcat(resbuf, "ing");
    return resbuf;
}

/* TRUE if the hero is wielding the given artifact, either as the
   primary weapon or as the off-hand weapon while two-weaponing */
boolean
wielding_artifact(int art)
{
    if (!art)
        return FALSE;

    return (is_art(uwep, art)
            || (u.twoweap && uswapwep && is_art(uswapwep, art)));
}

/* monster equivalent of wielding_artifact() - checks both the primary
   and the off-hand weapon */
boolean
mon_wielding_artifact(struct monst *mon, int art)
{
    if (!art || !mon)
        return FALSE;

    return (is_art(MON_WEP(mon), art) || is_art(MON_WEP2(mon), art));
}

/* TRUE if the hero's role still has an ungenerated artifact that
   mk_artifact() would hand out as a guaranteed first-choice gift */
boolean
awaiting_guaranteed_gift(void)
{
    int m;
    const struct artifact *a;

    for (m = 1, a = &artilist[m]; a->otyp; a++, m++) {
        if (artiexist[m].exists)
            continue;
        if (a->spfx & SPFX_NOGEN)
            continue;

        /* uses the same criteria as mk_artifact() */
        if (Role_if(a->role)
            && (a->alignment == u.ualign.type || a->alignment == A_NONE)
            && (a->race == NON_PM || !race_hostile(&mons[a->race]))
            && !Hate_material(artifact_material(m))
            && !(Race_if(PM_GIANT) && (a->mtype & MH_GIANT))
            && !((Race_if(PM_DRAUGR) || Race_if(PM_VAMPIRE))
                 && (a->mtype & MH_UNDEAD))
            && !(Race_if(PM_AASIMAR) && (a->mtype & MH_AASIMAR)))
            return TRUE;
    }
    return FALSE;
}

/* the intrinsic alignment of an artifact */
int
arti_align(int oartifact)
{
    if (oartifact <= 0 || oartifact > NROFARTIFACTS)
        return A_NONE;
    return artilist[oartifact].alignment;
}

/* warning glow from in-use race-detecting artifacts like Sting,
   Demonbane, and so on */
void
glow_warning_effects(struct obj *glower)
{
    if (glower && glower->oartifact
        && artilist[(int) glower->oartifact].acolor != NO_COLOR) {
        int oldstr = glow_strength((int) glower->lastwarncnt),
            newstr = glow_strength(glower->newwarncnt);

        if (newstr > 0 && newstr != oldstr) {
            /* goto_level() -> docrt() -> see_monsters() -> here;
               if "you materialize on a different level" is pending, give
               it now so that start-glowing message comes after it */
            maybe_lvltport_feedback(); /* usually called by goto_level() */

            /* 'start' message */
            if (!Blind)
                pline("%s %s %s%c", bare_artifactname(glower),
                      otense(glower, glow_verb(glower->newwarncnt, FALSE)),
                      glow_color(glower->oartifact),
                      (newstr > oldstr) ? '!' : '.');
            else if (oldstr == 0) /* quivers */
                pline("%s %s slightly.", bare_artifactname(glower),
                      otense(glower, glow_verb(0, FALSE)));
        } else if (glower->newwarncnt == 0 && glower->lastwarncnt > 0) {
            /* 'stop' message */
            pline("%s stops %s.", bare_artifactname(glower),
                  glow_verb(Blind ? 0 : (int) glower->lastwarncnt, TRUE));
        }
    }
}

/* blinding suppresses perception of artifacts like Sting that glow to
   warn of certain monsters; give a 'continue' message when blindness
   is toggled so that the eventual 'stop' message will match it */
void
blind_glow_warnings(void)
{
    struct obj *otmp;

    for (otmp = gi.invent; otmp; otmp = otmp->nobj) {
        if (((otmp->owornmask & (W_ARMOR | W_ACCESSORY | W_WEP))
             || (u.twoweap && (otmp->owornmask & W_SWAPWEP)))
            && has_glow_warning(otmp)
            && otmp->lastwarncnt > 0) {
            pline("%s is %s.", bare_artifactname(otmp),
                  glow_verb(Blind ? 0 : (int) otmp->lastwarncnt, TRUE));
        }
    }
}

/* called when hero is wielding/applying/invoking a carried item, or
   after undergoing a transformation (alignment change, lycanthropy,
   polymorph) which might affect item access */
int
retouch_object(
    struct obj **objp, /* might be destroyed or unintentionally dropped */
    boolean direct,    /* whether player lacks physical protection */
    boolean loseit)    /* whether to drop it if hero can longer touch it */
{
    struct obj *obj = *objp;
    unsigned oid = obj->o_id;
    /* a steed's saddle comes here from the steed's inventory */
    boolean in_invent = (obj->where == OBJ_INVENT), touched;

    /* allow hero (specifically, vampires or a crowned Infidel)
       in silver-hating form to try to perform invocation ritual */
    if (obj->otyp == BELL_OF_OPENING
        && invocation_pos(u.ux, u.uy) && !On_stairs(u.ux, u.uy)) {
        return 1;
    }

    touched = touch_artifact(obj, &gy.youmonst);
    /* the blast can return the hero to a natural form whose
       break_armor() drops what the polymorphed one wore */
    if (in_invent && o_on(oid, gi.invent) != obj) {
        *objp = 0;
        return 0;
    }
    if (touched) {
        char buf[BUFSZ];
        int dmg = 0;
        boolean hatemat = Hate_material(obj->material),
                bane = bane_applies(get_artifact(obj), &gy.youmonst);

        /* nothing else to do if hero can successfully handle this object */
        if (!hatemat && !bane)
            return 1;

        /* skin changing to bark or stone allows for safe handling,
           similar to wearing gloves. demons and vampires will always
           get blasted by silver, though */
        if ((Barkskin || Stoneskin)
            && !bane && !(is_demon(raceptr(&gy.youmonst))
                          || is_vampire(raceptr(&gy.youmonst))))
            return 1;

        /* another case where nothing should happen: hero is wearing gloves
           which protect them from directly touching a weapon of a material
           they hate or wearing boots that prevent them touching a kicked
           object. demons and vampires will always get blasted, though */
        if (!bane && !(direct || is_demon(raceptr(&gy.youmonst))
                       || is_vampire(raceptr(&gy.youmonst))))
            return 1;

        /* hero can't handle this object, but didn't get touch_artifact()'s
           "<obj> evades your grasp|control" message; give an alternate one */
        if (!bane && !(hatemat && obj->material == SILVER)) {
            pline("The %s of %s %s!", materialnm[obj->material],
                  yname(obj), rn2(2) ? "hurts to touch" : "burns your skin");
        } else {
            You_cant("handle %s%s!", yname(obj),
                     obj->owornmask ? " anymore" : "");
        }
        /* damage is somewhat arbitrary: 1d10 magical for <foo>bane,
           half of the usual damage for materials */
        if (hatemat && (direct || is_demon(raceptr(&gy.youmonst))
                        || is_vampire(raceptr(&gy.youmonst))))
            dmg += rnd(sear_damage(obj->material) / 2);
        if (bane)
            dmg += rnd(10);
        if (bane)
            Snprintf(buf, sizeof buf, "handling %s", killer_xname(obj));
        else
            Snprintf(buf, sizeof buf, "handling %s that was made of %s",
                     killer_xname(obj), materialnm[obj->material]);
        losehp(dmg, buf, KILLED_BY);
        /* returning to a natural form can make break_armor() drop or
           destroy the worn item */
        if (in_invent && o_on(oid, gi.invent) != obj) {
            *objp = 0;
            return 0;
        }
        exercise(A_CON, FALSE);
        /* concession to those wishing to use gear made of an adverse
         * material: don't make them totally unable to use them. In fact,
         * they can touch them just fine as long as they're willing to.
         * In keeping with the flavor of searing vs just pain implemented
         * everywhere else, only silver is actually unbearable -- other
         * hated non-silver materials can be used too */
        if (!bane && !(hatemat && obj->material == SILVER))
            return 1;
    }

    /* removing a worn item might result in loss of levitation,
       dropping the hero onto a polymorph trap or into water or
       lava and potentially dropping or destroying the item */
    if (obj->owornmask) {
        struct obj *otmp;

        remove_worn_item(obj, FALSE);
        for (otmp = gi.invent; otmp; otmp = otmp->nobj)
            if (otmp == obj)
                break;
        if (!otmp)
            *objp = obj = 0;
    }

    /* if we still have it and caller wants us to drop it, do so now */
    if (loseit && obj) {
        if (Levitation) {
            freeinv(obj);
            hitfloor(obj, TRUE);
        } else {
            /* dropx gives a message if a dropped item lands on an altar;
               we provide one for other terrain */
            if (!IS_ALTAR(levl[u.ux][u.uy].typ))
                pline("%s to the %s.", Tobjnam(obj, "fall"),
                      surface(u.ux, u.uy));
            dropx(obj);
        }
        *objp = obj = 0; /* no longer in inventory */
    }
    return 0;
}

/* hero has changed form or alignment; an item which is worn/wielded
   or an artifact which conveys something via being carried or which
   has an #invoke effect currently in operation undergoes a touch test;
   if it fails, it will be unworn/unwielded and maybe dropped */
staticfn boolean
untouchable(
    struct obj *obj, /* object to test; in invent or is steed's saddle */
    boolean drop_untouchable) /* whether to drop it if it can't be touched */
{
    struct artifact *art;
    boolean beingworn, carryeffect, invoked;
    long wearmask = ~(W_QUIVER | (u.twoweap ? 0L : W_SWAPWEP) | W_BALL);

    beingworn = (obj /* never Null; this pacifies static analysis when
                      * the get_artifact() macro tests 'obj' for Null */
                 && ((obj->owornmask & wearmask) != 0L
                     /* some items in use don't have any wornmask setting */
                     || (obj->oclass == TOOL_CLASS
                         && (obj->lamplit
                             || (obj->otyp == LEASH && obj->leashmon)
                             || (Is_container(obj) && Has_contents(obj))))));

    if ((art = get_artifact(obj)) != &artilist[ART_NONARTIFACT]) {
        carryeffect = (art->cary.adtyp || art->cspfx);
        invoked = (art->inv_prop > 0 && art->inv_prop <= LAST_PROP
                   && (u.uprops[art->inv_prop].extrinsic & W_ARTI) != 0L);
    } else {
        carryeffect = invoked = FALSE;
    }

    if (beingworn || carryeffect || invoked) {
        boolean direct = beingworn && will_touch_skin(obj->owornmask);

        if (!retouch_object(&obj, direct, drop_untouchable)) {
            /* "<artifact> is beyond your control" or "you can't handle
               <object>" has been given and it is now unworn/unwielded
               and possibly dropped (depending upon caller); if dropped,
               carried effect was turned off, else we leave that alone;
               we turn off invocation property here if still carried */
            if (invoked && obj)
                (void) arti_invoke(obj); /* reverse #invoke */
            return TRUE;
        }
    }
    return FALSE;
}

/* check all items currently in use (mostly worn) for touchability */
void
retouch_equipment(
    int dropflag) /* 0==don't drop, 1==drop all, 2==drop weapon */
{
    static int nesting = 0; /* recursion control */
    struct obj *obj;
    boolean dropit, had_gloves = (uarmg != 0);
    int had_rings = (!!uleft + !!uright);

    /*
     * We can potentially be called recursively if losing/unwearing
     * an item causes worn helm of opposite alignment to come off or
     * be destroyed.
     *
     * BUG: if the initial call was due to putting on a helm of
     * opposite alignment and it does come off to trigger recursion,
     * after the inner call executes, the outer call will finish
     * using the non-helm alignment rather than the helm alignment
     * which triggered this in the first place.
     */
    if (!nesting++)
        clear_bypasses(); /* init upon initial entry */

    /* catch a worn slot already pointing outside invent on entry, before
       a forced drop turns it into a freeinv() "object lost" panic */
    if (iflags.sanity_check || iflags.debug_fuzzer)
        worn_slot_sanity("retouch_equipment entry");

    dropit = (dropflag > 0); /* drop all or drop weapon */
    /* check secondary weapon first, before possibly unwielding primary */
    if (u.twoweap) {
        bypass_obj(uswapwep); /* so loop below won't process it again */
        (void) untouchable(uswapwep, dropit);
    }
    /* check primary weapon next so that they're handled together */
    if (uwep) {
        bypass_obj(uwep); /* so loop below won't process it again */
        (void) untouchable(uwep, dropit);
    }

    /* in case someone is daft enough to add artifact or silver saddle */
    if (u.usteed && (obj = which_armor(u.usteed, W_SADDLE)) != 0) {
        /* untouchable() calls retouch_object() which expects an object in
           hero's inventory, but remove_worn_item() will be harmless for
           saddle and we're suppressing drop, so this works as intended */
        if (untouchable(obj, FALSE))
            dismount_steed(DISMOUNT_THROWN);
    }
    /*
     * TODO?  Force off gloves if either or both rings are going to
     * become unworn; force off cloak [suit] before suit [shirt].
     * The torso handling is hypothetical; the case for gloves is
     * not, due to the possibility of unwearing silver rings.
     */

    dropit = (dropflag == 1); /* all untouchable items */
    /* loss of levitation (silver ring, or Heart of Ahriman invocation)
       might cause hero to lose inventory items (by dropping into lava,
       for instance), so inventory traversal needs to rescan the whole
       gi.invent chain each time it moves on to another object; we use bypass
       handling to keep track of which items have already been processed */
    while ((obj = nxt_unbypassed_obj(gi.invent)) != 0)
        (void) untouchable(obj, dropit);

    if (had_rings != (!!uleft + !!uright) && uarmg && uarmg->cursed)
        uncurse(uarmg); /* temporary? hack for ring removal plausibility */
    if (had_gloves && !uarmg)
        selftouch("After losing your gloves, you");

    if (!--nesting)
        clear_bypasses(); /* reset upon final exit */
}

staticfn int
count_surround_traps(coordxy x, coordxy y)
{
    struct rm *levp;
    struct obj *o;
    coordxy dx, dy;
    int glyph, ret = 0;

    for (dx = x - 1; dx < x + 2; ++dx)
        for (dy = y - 1; dy < y + 2; ++dy) {
            if (!isok(dx, dy))
                continue;
            /* If a trap is shown here, don't count it; the hero
             * should be expecting it.  But if there is a trap here
             * that's not shown, either undiscovered or covered by
             * something, do count it.
             */
            glyph = glyph_at(dx, dy);
            if (glyph_is_trap(glyph))
                continue;
            if (t_at(dx, dy)) {
                ++ret;
                continue;
            }
            levp = &levl[dx][dy];
            if (IS_DOOR(levp->typ) && (levp->doormask & D_TRAPPED) != 0) {
                ++ret;
                continue;
            }
            for (o = svl.level.objects[dx][dy]; o; o = o->nexthere)
                if (Is_container(o) && o->otrapped) {
                    ++ret; /* we're counting locations, so just */
                    break; /* count the first one in a pile     */
                }
        }
    /*
     * [Shouldn't we also check inventory for a trapped container?
     * Even if its trap has already been found, there's no 'tknown'
     * flag to help hero remember that so we have nothing comparable
     * to a shown glyph to justify skipping it.]
     */
    return ret;
}

/* sense adjacent traps if wielding MKoT without wearing gloves */
void
mkot_trap_warn(void)
{
    static const char *const heat[7] = {
        "cool", "slightly warm", "warm", "very warm",
        "hot", "very hot", "like fire"
    };

    if (!uarmg && is_art(uwep, ART_MASTER_KEY_OF_THIEVERY)) {
        int idx, ntraps = count_surround_traps(u.ux, u.uy);

        if (ntraps != gm.mkot_trap_warn_count) {
            idx = min(ntraps, SIZE(heat) - 1);
            pline_The("Key feels %s%c", heat[idx], (ntraps > 3) ? '!' : '.');
        }
        gm.mkot_trap_warn_count = ntraps;
    } else
        gm.mkot_trap_warn_count = 0;
}

/* Master Key is magic key if its bless/curse state meets our criteria:
   not cursed for rogues or blessed for non-rogues */
boolean
is_magic_key(struct monst *mon, /* if null, non-rogue is assumed */
             struct obj *obj)
{
    if (is_art(obj, ART_MASTER_KEY_OF_THIEVERY)) {
        if ((mon == &gy.youmonst) ? Role_if(PM_ROGUE)
                                 : (mon && mon->data == &mons[PM_ROGUE]))
            return !obj->cursed; /* a rogue; non-cursed suffices for magic */
        /* not a rogue; key must be blessed to behave as a magic one */
        return obj->blessed;
    }
    return FALSE;
}

/* figure out whether 'mon' (usually youmonst) is carrying the magic key */
struct obj *
has_magic_key(struct monst *mon) /* if null, hero assumed */
{
    struct obj *o;
    short key = artilist[ART_MASTER_KEY_OF_THIEVERY].otyp;

    if (!mon)
        mon = &gy.youmonst;
    for (o = ((mon == &gy.youmonst) ? gi.invent : mon->minvent); o;
         o = nxtobj(o, key, FALSE)) {
        if (is_magic_key(mon, o))
            return o;
    }
    return (struct obj *) 0;
}

/* #define is_art(o,art) ((o) && (o)->oartifact == (art)) */

boolean
is_art(struct obj *obj, int art)
{
    if (obj && obj->oartifact == art)
        return TRUE;
    return FALSE;
}

/* #define get_artifact(o) \
    (((o) && ((o)->artifact > 0 && (o)->artifact < AFTER_LAST_ARTIFACT)) \
                             ? &artilist[(int) (o)->oartifact] \
                             : &artilist[ART_NONARTIFACT]) */

staticfn struct artifact *
get_artifact(struct obj *obj)
{
    if (obj) {
        int artidx = (int) obj->oartifact;

        /* skip 0, 1st artifact at 1 */
        /* SIZE(artilist) would include the terminator,
           so use AFTER_LAST_ARTIFACT instead */
        if (artidx > 0 && artidx < AFTER_LAST_ARTIFACT)
            return &artilist[artidx];
    }
    return &artilist[ART_NONARTIFACT];
}

/* is object permanently poisoned? (currently only Grimtooth) */
boolean
permapoisoned(struct obj *obj)
{
    return (obj && is_art(obj, ART_GRIMTOOTH));
}

/* is this artifact one that can never be wished for? */
boolean
non_wishable_artifact(struct obj *otmp)
{
    int art = otmp->oartifact;

    if (!art)
        return FALSE;
    return ((artilist[art].spfx & SPFX_NOWISH) != 0L);
}

/* is this artifact one that can only be created via forging? */
boolean
is_forged_artifact(struct obj *otmp)
{
    int art = otmp->oartifact;

    if (!art)
        return FALSE;
    return ((artilist[art].spfx & SPFX_FORGED) != 0L);
}
#endif /* SFCTOOL */

/*artifact.c*/
