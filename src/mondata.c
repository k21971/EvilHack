/* NetHack 3.6	mondata.c	$NHDT-Date: 1550525093 2019/02/18 21:24:53 $  $NHDT-Branch: NetHack-3.6.2-beta01 $:$NHDT-Revision: 1.72 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2011. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
/*
 *      These routines provide basic data for any type of monster.
 */

/* set up an individual monster's base type (initial creation, shapechange) */
void
set_mon_data(mon, ptr)
struct monst *mon;
struct permonst *ptr;
{
    int new_speed, old_speed = mon->data ? mon->data->mmove : 0;
    short *movement_p = (mon == &youmonst) ? &u.umovement : &mon->movement;

    mon->data = ptr;
    mon->mnum = (short) monsndx(ptr);

    if (*movement_p) { /* used to adjust poly'd hero as well as monsters */
        new_speed = ptr->mmove;
        /* prorate unused movement if new form is slower so that
           it doesn't get extra moves leftover from previous form;
           if new form is faster, leave unused movement as is */
        if (new_speed < old_speed) {
            /*
             * Some static analysis warns that this might divide by 0
               *movement_p = new_speed * *movement_p / old_speed;
             * so add a redundant test to suppress that.
             */
            *movement_p *= new_speed;
            if (old_speed > 0) /* old > new and new >= 0, so always True */
                *movement_p /= old_speed;
        }
    }
    return;
}

/* does monster-type have any attack for a specific type of damage? */
struct attack *
attacktype_fordmg(ptr, atyp, dtyp)
struct permonst *ptr;
int atyp, dtyp;
{
    struct attack *a;

    for (a = &ptr->mattk[0]; a < &ptr->mattk[NATTK]; a++)
        if (a->aatyp == atyp && (dtyp == AD_ANY || a->adtyp == dtyp))
            return a;
    return (struct attack *) 0;
}

/* does monster-type have a particular type of attack */
boolean
attacktype(ptr, atyp)
struct permonst *ptr;
int atyp;
{
    return attacktype_fordmg(ptr, atyp, AD_ANY) ? TRUE : FALSE;
}

/* returns True if monster doesn't attack, False if it does */
boolean
noattacks(ptr)
struct permonst *ptr;
{
    int i;
    struct attack *mattk = ptr->mattk;

    for (i = 0; i < NATTK; i++) {
        /* AT_BOOM "passive attack" (gas spore's explosion upon death)
           isn't an attack as far as our callers are concerned */
        if (mattk[i].aatyp == AT_BOOM)
            continue;

        if (mattk[i].aatyp)
            return FALSE;
    }
    return TRUE;
}

/* does monster-type transform into something else when petrified? */
boolean
poly_when_stoned(ptr)
struct permonst *ptr;
{
    /* non-stone golems turn into stone golems unless latter is genocided,
       same for ents <-> petrified ents */
    return (boolean) ((is_golem(ptr) && ptr != &mons[PM_STONE_GOLEM]
                       && !(mvitals[PM_STONE_GOLEM].mvflags & G_GENOD))
                      || (is_ent(ptr) && ptr != &mons[PM_PETRIFIED_ENT]
                          && !(mvitals[PM_PETRIFIED_ENT].mvflags & G_GENOD)));
    /* allow G_EXTINCT */
}

/* is 'mon' (possibly youmonst) protected against damage type 'adtype' via
   wielded weapon or worn dragon scales? [or by virtue of being a dragon?] */
boolean
defended(mon, adtyp)
struct monst *mon;
int adtyp;
{
    struct obj *o, otemp;
    int mndx;
    boolean is_you = (mon == &youmonst);

    /* is 'mon' wielding an artifact that protects against 'adtyp'? */
    o = is_you ? uwep : MON_WEP(mon);
    if (o && o->oartifact && defends(adtyp, o))
        return TRUE;

    /* if 'mon' is an adult dragon, treat it as if it was wearing scales
       so that it has the same benefit as a hero wearing dragon scales */
    mndx = monsndx(mon->data);
    if (mndx >= PM_GRAY_DRAGON && mndx <= PM_CELESTIAL_DRAGON) {
        /* a dragon is its own suit...  if mon is poly'd hero, we don't
           care about embedded scales (uskin) because being a dragon with
           embedded scales is no better than just being a dragon */
        otemp = zeroobj;
        otemp.oclass = ARMOR_CLASS;
        otemp.otyp = GRAY_DRAGON_SCALES + (mndx - PM_GRAY_DRAGON);
        /* defends() and Is_dragon_armor() only care about otyp so ignore
           the rest of otemp's fields */
        o = &otemp;
    } else {
        /* ordinary case: not an adult dragon */
        o = is_you ? uarm : which_armor(mon, W_ARM);
    }
    /* is 'mon' wearing dragon scales that protect against 'adtyp'? */
    if (o && Is_dragon_armor(o) && defends(adtyp, o))
        return TRUE;

    return FALSE;
}

/* returns True if monster is drain-life resistant */
boolean
resists_drli(mon)
struct monst *mon;
{
    struct permonst *ptr = raceptr(mon); /* handle demonic race */
    struct obj *armor;
    long slotmask;

    if (resists_drain(ptr) || is_vampshifter(mon)
        || (mon == &youmonst && u.ulycn >= LOW_PM))
        return TRUE;
    armor = (mon == &youmonst) ? invent : mon->minvent;
    slotmask = W_ARMOR | W_ACCESSORY;
    /* check for drain res object property */
    for (; armor; armor = armor->nobj) {
        if ((armor->owornmask & slotmask) != 0L
            && obj_has_prop(armor, DRAIN_RES))
            return TRUE;
    }
    return defended(mon, AD_DRLI);
}

/* True if monster is magic-missile (actually, general magic) resistant */
boolean
resists_magm(mon)
struct monst *mon;
{
    struct permonst *ptr = mon->data;
    boolean is_you = (mon == &youmonst);
    long slotmask;
    struct obj *o;

    if (resists_mgc(ptr))
        return TRUE;
    /* check for magic resistance granted by wielded weapon */
    o = is_you ? uwep : MON_WEP(mon);
    if (o && o->oartifact && defends(AD_MAGM, o))
        return TRUE;
    /* check for magic resistance granted by worn or carried items */
    o = is_you ? invent : mon->minvent;
    slotmask = W_ARMOR | W_ACCESSORY | W_BARDING;
    if (!is_you /* assumes monsters don't wield non-weapons */
        || (uwep && (uwep->oclass == WEAPON_CLASS || is_weptool(uwep))))
        slotmask |= W_WEP;
    if (is_you && u.twoweap)
        slotmask |= W_SWAPWEP;
    for (; o; o = o->nobj) {
        if (((o->owornmask & slotmask) != 0L
             && (obj_has_prop(o, ANTIMAGIC)
                 || defends(AD_MAGM, o)))
            || (o->oartifact && defends_when_carried(AD_MAGM, o)))
            return TRUE;
    }
    return FALSE;
}

/* True iff monster is resistant to light-induced blindness */
boolean
resists_blnd(mon)
struct monst *mon;
{
    struct permonst *ptr = mon->data;
    boolean is_you = (mon == &youmonst);
    long slotmask;
    struct obj *o;

    if (is_you ? (Blind || Unaware)
               : (mon->mblinded || !mon->mcansee || !haseyes(ptr)
                  /* BUG: temporary sleep sets mfrozen, but since
                          paralysis does too, we can't check it */
                  || mon->msleeping))
        return TRUE;
    /* yellow light, Archon; !dust vortex, !cobra, !raven */
    if (dmgtype_fromattack(ptr, AD_BLND, AT_EXPL)
        || dmgtype_fromattack(ptr, AD_BLND, AT_GAZE))
        return TRUE;
    o = is_you ? uwep : MON_WEP(mon);
    if (o && o->oartifact && defends(AD_BLND, o))
        return TRUE;
    o = is_you ? invent : mon->minvent;
    slotmask = W_ARMOR | W_ACCESSORY;
    if (!is_you /* assumes monsters don't wield non-weapons */
        || (uwep && (uwep->oclass == WEAPON_CLASS || is_weptool(uwep))))
        slotmask |= W_WEP;
    if (is_you && u.twoweap)
        slotmask |= W_SWAPWEP;
    for (; o; o = o->nobj)
        if (((o->owornmask & slotmask) != 0L
             && objects[o->otyp].oc_oprop == BLINDED)
            || (o->oartifact && defends_when_carried(AD_BLND, o)))
            return TRUE;
    return FALSE;
}

/* True iff monster can be blinded by the given attack;
   note: may return True when mdef is blind (e.g. new cream-pie attack) */
boolean
can_blnd(magr, mdef, aatyp, obj)
struct monst *magr; /* NULL == no specific aggressor */
struct monst *mdef;
uchar aatyp;
struct obj *obj; /* aatyp == AT_WEAP, AT_SPIT */
{
    boolean is_you = (mdef == &youmonst);
    boolean check_visor = FALSE;
    struct obj *o;
    const char *s;

    /* no eyes protect against all attacks for now */
    if (!haseyes(mdef->data))
        return FALSE;

    /* if monster has been permanently blinded, the deed is already done */
    if (!is_you && mon_perma_blind(mdef))
        return FALSE;

    /* /corvus oculum corvi non eruit/
       a saying expressed in Latin rather than a zoological observation:
       "a crow will not pluck out the eye of another crow"
       so prevent ravens from blinding each other */
    if (magr && magr->data == &mons[PM_RAVEN] && mdef->data == &mons[PM_RAVEN])
        return FALSE;

    switch (aatyp) {
    case AT_EXPL:
    case AT_BOOM:
    case AT_GAZE:
    case AT_MAGC:
    case AT_BREA: /* assumed to be lightning */
        /* light-based attacks may be cancelled or resisted */
        if (magr && magr->mcan)
            return FALSE;
        return !resists_blnd(mdef);

    case AT_WEAP:
    case AT_SPIT:
    case AT_NONE:
        /* an object is used (thrown/spit/other) */
        if (obj && (obj->otyp == CREAM_PIE)) {
            if (is_you && Blindfolded)
                return FALSE;
        } else if (obj && ((obj->otyp == BLINDING_VENOM)
                           || (obj->otyp == SNOWBALL))) {
            /* all ublindf, including LENSES, protect, cream-pies too */
            if (is_you && (ublindf || u.ucreamed))
                return FALSE;
            check_visor = TRUE;
        } else if (obj && (obj->otyp == POT_BLINDNESS)) {
            return TRUE; /* no defense */
        } else
            return FALSE; /* other objects cannot cause blindness yet */
        if ((magr == &youmonst) && u.uswallow)
            return FALSE; /* can't affect eyes while inside monster */
        break;

    case AT_ENGL:
        if (is_you && (Blindfolded || Unaware || u.ucreamed))
            return FALSE;
        if (!is_you && mdef->msleeping)
            return FALSE;
        break;

    case AT_CLAW:
        /* e.g. raven: all ublindf, including LENSES, protect */
        if (is_you && ublindf)
            return FALSE;
        if ((magr == &youmonst) && u.uswallow)
            return FALSE; /* can't affect eyes while inside monster */
        check_visor = TRUE;
        break;

    case AT_TUCH:
    case AT_STNG:
        /* some physical, blind-inducing attacks can be cancelled */
        if (magr && magr->mcan)
            return FALSE;
        break;

    default:
        break;
    }

    /* check if wearing a visor (only checked if visor might help) */
    if (check_visor) {
        o = (mdef == &youmonst) ? invent : mdef->minvent;
        for (; o; o = o->nobj)
            if ((o->owornmask & W_ARMH)
                && (s = OBJ_DESCR(objects[o->otyp])) != (char *) 0
                && !strcmp(s, "visored helmet"))
                return FALSE;
    }

    return TRUE;
}

boolean
vulnerable_to(mon, element)
struct monst* mon;
int element;
{
    boolean fire_vuln = (((mon->data->mflags4 & M4_VULNERABLE_FIRE) != 0
                          || mon->vuln_fire)
                         && !resists_fire(mon));
    boolean cold_vuln = (((mon->data->mflags4 & M4_VULNERABLE_COLD) != 0
                          || mon->vuln_cold)
                         && !resists_cold(mon));
    boolean elec_vuln = (((mon->data->mflags4 & M4_VULNERABLE_ELEC) != 0
                          || mon->vuln_elec)
                         && !resists_elec(mon));
    boolean acid_vuln = (((mon->data->mflags4 & M4_VULNERABLE_ACID) != 0
                          || mon->vuln_acid)
                         && !resists_acid(mon));

    if (defended(mon, element))
        return FALSE;

    switch (element) {
    case AD_FIRE:
        return fire_vuln;
    case AD_COLD:
        return cold_vuln;
    case AD_ELEC:
        return elec_vuln;
    case AD_ACID:
        return acid_vuln;
    case AD_FUSE:
        return (fire_vuln || cold_vuln);
    default:
        break;
    }
    return FALSE;
}

/* returns True if monster can attack at range */
boolean
ranged_attk(mtmp)
struct monst *mtmp;
{
    register int i, atyp;
    long atk_mask = (1L << AT_BREA) | (1L << AT_SPIT) | (1L << AT_GAZE);
    struct attack *mattk;
    mattk = has_erac(mtmp) ? ERAC(mtmp)->mattk : mtmp->data->mattk;

    /* was: (attacktype(ptr, AT_BREA) || attacktype(ptr, AT_WEAP)
     *       || attacktype(ptr, AT_SPIT) || attacktype(ptr, AT_GAZE)
     *       || attacktype(ptr, AT_MAGC));
     * but that's too slow -dlc
     */
    for (i = 0; i < NATTK; i++) {
        atyp = mattk[i].aatyp;
        if (atyp >= AT_WEAP)
            return TRUE;
        /* assert(atyp < 32); */
        if ((atk_mask & (1L << atyp)) != 0L)
            return TRUE;
    }
    return FALSE;
}

/* True if specific monster is especially affected by weapons of the given
 * material type */
boolean
mon_hates_material(mon, material)
struct monst * mon;
int material;
{
    if (hates_material(r_data(mon), material))
        return TRUE;

    /* extra case: shapeshifted vampires still hate silver */
    if (material == SILVER && is_vampshifter(mon))
        return TRUE;

    /* extra extra case: lycanthrope player (monster lycanthropes all fall under
     * hates_material, and non-lycanthropes can't currently be infected) */
    if (mon == &youmonst && material == SILVER && u.ulycn >= LOW_PM)
        return TRUE;

    if (has_erac(mon)) {
        if (material == IRON) {
            return (racial_elf(mon) || racial_drow(mon));
        } else if (material == MITHRIL) {
            return (racial_orc(mon));
        }
    }

    return FALSE;
}

/* True if monster-type is especially affected by weapons of the given material
 * type */
boolean
hates_material(ptr, material)
register struct permonst *ptr;
int material;
{
    if (material == SILVER) {
        if (ptr->mlet == S_IMP) {
            /* impish creatures that aren't actually demonic */
            if (ptr == &mons[PM_TENGU] || ptr == &mons[PM_LEPRECHAUN])
                return FALSE;
        }
        return (is_were(ptr) || is_vampire(ptr)
                || is_demon(ptr) || ptr == &mons[PM_SHADE]
                || (ptr->mlet == S_IMP));
    } else if (material == IRON) {
        if (is_undead(ptr))
            return FALSE;
        /* cold iron: elves and drow hate it */
        return (is_elf(ptr) || is_drow(ptr));
    } else if (material == MITHRIL) {
        if (is_undead(ptr))
            return FALSE;
        /* mithril: orcs hate it */
        return (is_orc(ptr));
    }
    return FALSE;
}

/* Return amount of damage a monster will take from coming into contact with a
 * material it hates. */
int
sear_damage(material)
int material;
{
    switch (material) {
    case SILVER:
        return 20;
    case IRON:
    case MITHRIL:
        return rnd(3) + 3;
    default:
        return 6;
    }
}

/* True if specific monster is especially affected by light-emitting weapons */
boolean
mon_hates_light(mon)
struct monst *mon;
{
    return (boolean) (hates_light(r_data(mon)));
}

/* True iff the type of monster pass through iron bars */
boolean
passes_bars(mptr)
struct permonst *mptr;
{
    return (boolean) (passes_walls(mptr) || amorphous(mptr) || unsolid(mptr)
                      || is_whirly(mptr) || verysmall(mptr)
                      || dmgtype(mptr, AD_CORR) || dmgtype(mptr, AD_RUST)
                      || (slithy(mptr) && !bigmonst(mptr)));
}

/* returns True if monster can blow (whistle, etc) */
boolean
can_blow(mtmp)
struct monst *mtmp;
{
    if ((is_silent(mtmp->data) || mtmp->data->msound == MS_BUZZ)
        && (breathless(mtmp->data) || r_verysmall(mtmp)
            || !has_head(mtmp->data) || mtmp->data->mlet == S_EEL))
        return FALSE;
    if ((mtmp == &youmonst) && Strangled)
        return FALSE;
    return TRUE;
}

/* for casting spells and reading scrolls while blind */
boolean
can_chant(mtmp)
struct monst *mtmp;
{
    if ((mtmp == &youmonst && Strangled)
        || racial_zombie(&youmonst)
        || is_silent(mtmp->data) || !has_head(mtmp->data)
        || mtmp->data->msound == MS_BUZZ || mtmp->data->msound == MS_BURBLE)
        return FALSE;
    return TRUE;
}

/* True if mon is vulnerable to strangulation */
boolean
can_be_strangled(mon)
struct monst *mon;
{
    struct obj *mamul;
    boolean nonbreathing, nobrainer;

    /* For amulet of strangulation support:  here we're considering
       strangulation to be loss of blood flow to the brain due to
       constriction of the arteries in the neck, so all headless
       creatures are immune (no neck) as are mindless creatures
       who don't need to breathe (brain, if any, doesn't care).
       Mindless creatures who do need to breath are vulnerable, as
       are non-breathing creatures which have higher brain function. */
    if (!has_head(mon->data))
        return FALSE;
    if (mon == &youmonst) {
        /* hero can't be mindless but poly'ing into mindless form can
           confer strangulation protection */
        nobrainer = mindless(youmonst.data);
        nonbreathing = Breathless;
    } else {
        nobrainer = mindless(mon->data);
        /* monsters don't wear amulets of magical breathing,
           so second part doesn't achieve anything useful... */
        nonbreathing = (breathless(mon->data)
                        || ((mamul = which_armor(mon, W_AMUL)) != 0
                            && (mamul->otyp == AMULET_OF_MAGICAL_BREATHING)));
    }
    return (boolean) (!nobrainer || !nonbreathing);
}

/* returns True if monster can track well */
boolean
can_track(ptr)
register struct permonst *ptr;
{
    if (wielding_artifact(ART_EXCALIBUR))
        return TRUE;
    else
        return (boolean) haseyes(ptr);
}

boolean
mon_prop(mon, prop)
register struct monst *mon;
int prop;
{
    struct obj *o;
    int adtyp = 0;
    /* First, check if prop has a corresponding monflag */
    switch (prop) {
    case REGENERATION:
        if (regenerates(mon->data))
            return TRUE;
        break;
    case SEE_INVIS:
        if (racial_perceives(mon))
            return TRUE;
        break;
    case TELEPORT:
        if (can_teleport(mon->data) && !mon->mcan)
            return TRUE;
        break;
    case TELEPORT_CONTROL:
        if (control_teleport(mon->data) || is_covetous(mon->data))
            return TRUE;
        break;
    case TELEPAT:
        if (telepathic(mon->data))
            return TRUE;
        break;
    case HALLUC_RES:
        adtyp = AD_HALU;
        break;
    case JUMPING:
        if (is_unicorn(mon->data))
            return TRUE;
        break;
    case ANTIMAGIC: /* just in case */
        return (resists_magm(mon));
    }

    /* Now check for extrinsics */
    for (o = mon->minvent; o; o = o->nobj) {
        if ((o->owornmask && objects[o->otyp].oc_oprop == prop)
            || (o->owornmask && obj_has_prop(o, prop)) /* object properties */
            || (o->oartifact && ((adtyp && protects(o, TRUE))
            || (arti_prop_spfx(prop) && spec_ability(o, arti_prop_spfx(prop)))))) {
        return TRUE;
        }
    }
    return FALSE;
}

/* creature will slide out of armor */
boolean
sliparm(mon)
struct monst *mon;
{
    struct permonst *ptr = r_data(mon);

    if (druid_form)
        return FALSE;

    if (vampire_form)
        return FALSE;

    return (boolean) (is_whirly(ptr) || ptr->msize <= MZ_SMALL
                      || noncorporeal(ptr));
}

/* creature will break out of armor */
boolean
breakarm(mon)
struct monst *mon;
{
    struct permonst *ptr = r_data(mon);
    if (sliparm(mon))
        return FALSE;

    if (racial_centaur(mon))
        return FALSE;

    if (racial_tortle(mon))
        return TRUE;

    if (ptr == &mons[PM_DRIDER])
        return FALSE;

    if (druid_form)
        return FALSE;

    if (vampire_form)
        return FALSE;

    return (boolean) (r_bigmonst(mon)
                      || (ptr->msize > MZ_SMALL && !humanoid(ptr))
                      /* special cases of humanoids that cannot wear suits */
                      || ptr == &mons[PM_MARILITH]);
}

/* creature sticks other creatures it hits */
boolean
sticks(ptr)
register struct permonst *ptr;
{
    return (boolean) (dmgtype(ptr, AD_STCK) || dmgtype(ptr, AD_WRAP)
                      || attacktype(ptr, AT_HUGS));
}

/* some monster-types can't vomit */
boolean
cantvomit(ptr)
struct permonst *ptr;
{
    /* rats and mice are incapable of vomiting;
       which other creatures have the same limitation? */
    if (ptr->mlet == S_RODENT && ptr != &mons[PM_ROCK_MOLE]
        && ptr != &mons[PM_WOODCHUCK])
        return TRUE;
    return FALSE;
}

/* number of horns this type of monster has on its head */
int
num_horns(ptr)
struct permonst *ptr;
{
    switch (monsndx(ptr)) {
    case PM_HORNED_DEVIL: /* ? "more than one" */
    case PM_MINOTAUR:
    case PM_ELDER_MINOTAUR:
    case PM_ASMODEUS:
    case PM_BAPHOMET:
    case PM_BALROG:
    case PM_SATYR:
    case PM_STAG:
        return 2;
    case PM_WHITE_UNICORN:
    case PM_GRAY_UNICORN:
    case PM_BLACK_UNICORN:
    case PM_ELDRITCH_KI_RIN:
    case PM_KI_RIN:
        return 1;
    default:
        break;
    }
    return 0;
}

/* does monster-type deal out a particular type of damage from a particular
   type of attack? */
struct attack *
dmgtype_fromattack(ptr, dtyp, atyp)
struct permonst *ptr;
int dtyp, atyp;
{
    struct attack *a;

    for (a = &ptr->mattk[0]; a < &ptr->mattk[NATTK]; a++)
        if (a->adtyp == dtyp && (atyp == AT_ANY || a->aatyp == atyp))
            return a;
    return (struct attack *) 0;
}

/* does monster-type deal out a particular type of damage from any attack */
boolean
dmgtype(ptr, dtyp)
struct permonst *ptr;
int dtyp;
{
    return dmgtype_fromattack(ptr, dtyp, AT_ANY) ? TRUE : FALSE;
}

/* returns the maximum damage a defender can do to the attacker via
   a passive defense */
int
max_passive_dmg(mdef, magr)
register struct monst *mdef, *magr;
{
    int i, dmg = 0, multi2 = 0;
    uchar adtyp;

    /* each attack by magr can result in passive damage */
    for (i = 0; i < NATTK; i++)
        switch (magr->data->mattk[i].aatyp) {
        case AT_CLAW:
        case AT_BITE:
        case AT_KICK:
        case AT_BUTT:
        case AT_TUCH:
        case AT_STNG:
        case AT_HUGS:
        case AT_ENGL:
        case AT_TENT:
        case AT_WEAP:
            multi2++;
            break;
        default:
            break;
        }

    for (i = 0; i < NATTK; i++)
        if (mdef->data->mattk[i].aatyp == AT_NONE
            || mdef->data->mattk[i].aatyp == AT_BOOM) {
            adtyp = mdef->data->mattk[i].adtyp;
            if ((adtyp == AD_ACID && !(resists_acid(magr)
                                       || defended(magr, AD_ACID)))
                || (adtyp == AD_COLD && !(resists_cold(magr)
                                          || defended(magr, AD_COLD)))
                || (adtyp == AD_FIRE && !(resists_fire(magr)
                                          || defended(magr, AD_FIRE)))
                || (adtyp == AD_ELEC && !(resists_elec(magr)
                                          || defended(magr, AD_ELEC)))
                || adtyp == AD_DISE
                || adtyp == AD_STON
                || adtyp == AD_PHYS) {
                dmg = mdef->data->mattk[i].damn;
                if (!dmg)
                    dmg = mdef->data->mlevel + 1;
                dmg *= mdef->data->mattk[i].damd;
            } else
                dmg = 0;

            return dmg * multi2;
        }
    return 0;
}

/* determine whether two monster types are from the same species */
boolean
same_race(pm1, pm2)
struct permonst *pm1, *pm2;
{
    char let1 = pm1->mlet, let2 = pm2->mlet;

    if (pm1 == pm2)
        return TRUE; /* exact match */
    /* player races have their own predicates */
    if (is_human(pm1))
        return is_human(pm2);
    if (is_elf(pm1))
        return is_elf(pm2);
    if (is_drow(pm1))
        return is_drow(pm2);
    if (is_dwarf(pm1))
        return is_dwarf(pm2);
    if (is_gnome(pm1))
        return is_gnome(pm2);
    if (is_orc(pm1))
        return is_orc(pm2);
    if (is_hobbit(pm1))
        return is_hobbit(pm2);
    if (is_centaur(pm1))
        return is_centaur(pm2);
    if (is_tortle(pm1))
        return is_tortle(pm2);
    if (is_zombie(pm1))
        return is_zombie(pm2);
    if (is_vampire(pm1))
        return is_vampire(pm2);
    /* other creatures are less precise */
    if (is_giant(pm1))
        return is_giant(pm2); /* open to quibbling here */
    if (is_gnoll(pm1))
        return is_gnoll(pm2);
    if (is_golem(pm1))
        return is_golem(pm2); /* even moreso... */
    if (is_mind_flayer(pm1))
        return is_mind_flayer(pm2);
    if (let1 == S_KOBOLD || pm1 == &mons[PM_KOBOLD_ZOMBIE]
        || pm1 == &mons[PM_KOBOLD_MUMMY])
        return (let2 == S_KOBOLD || pm2 == &mons[PM_KOBOLD_ZOMBIE]
                || pm2 == &mons[PM_KOBOLD_MUMMY]);
    if (let1 == S_OGRE)
        return (let2 == S_OGRE);
    if (let1 == S_NYMPH)
        return (let2 == S_NYMPH);
    if (let1 == S_CENTAUR)
        return (let2 == S_CENTAUR);
    if (is_unicorn(pm1))
        return is_unicorn(pm2);
    if (let1 == S_DRAGON)
        return (let2 == S_DRAGON);
    if (let1 == S_NAGA)
        return (let2 == S_NAGA);
    /* other critters get steadily messier */
    if (is_rider(pm1))
        return is_rider(pm2); /* debatable */
    if (is_minion(pm1))
        return is_minion(pm2); /* [needs work?] */
    /* tengu don't match imps (first test handled case of both being tengu) */
    if (pm1 == &mons[PM_TENGU] || pm2 == &mons[PM_TENGU])
        return FALSE;
    if (let1 == S_IMP)
        return (let2 == S_IMP);
    /* and minor demons (imps) don't match major demons */
    else if (let2 == S_IMP)
        return FALSE;
    if (is_demon(pm1))
        return is_demon(pm2);
    if (is_undead(pm1)) {
        if (let1 == S_ZOMBIE)
            return (let2 == S_ZOMBIE);
        if (let1 == S_MUMMY)
            return (let2 == S_MUMMY);
        if (let1 == S_VAMPIRE)
            return (let2 == S_VAMPIRE);
        if (let1 == S_LICH)
            return (let2 == S_LICH);
        if (let1 == S_WRAITH)
            return (let2 == S_WRAITH);
        if (let1 == S_GHOST)
            return (let2 == S_GHOST);
    } else if (is_undead(pm2))
        return FALSE;

    /* check for monsters which grow into more mature forms */
    if (let1 == let2) {
        int m1 = monsndx(pm1), m2 = monsndx(pm2), prv, nxt;

        /* we know m1 != m2 (very first check above); test all smaller
           forms of m1 against m2, then all larger ones; don't need to
           make the corresponding tests for variants of m2 against m1 */
        for (prv = m1, nxt = big_to_little(m1); nxt != prv;
             prv = nxt, nxt = big_to_little(nxt))
            if (nxt == m2)
                return TRUE;
        for (prv = m1, nxt = little_to_big(m1); nxt != prv;
             prv = nxt, nxt = little_to_big(nxt))
            if (nxt == m2)
                return TRUE;
    }
    /* not caught by little/big handling */
    if (pm1 == &mons[PM_GARGOYLE] || pm1 == &mons[PM_WINGED_GARGOYLE])
        return (pm2 == &mons[PM_GARGOYLE]
                || pm2 == &mons[PM_WINGED_GARGOYLE]);
    if (pm1 == &mons[PM_KILLER_BEE] || pm1 == &mons[PM_QUEEN_BEE])
        return (pm2 == &mons[PM_KILLER_BEE] || pm2 == &mons[PM_QUEEN_BEE]);
    if (pm1 == &mons[PM_GIANT_ANT] || pm1 == &mons[PM_QUEEN_ANT])
        return (pm2 == &mons[PM_GIANT_ANT] || pm2 == &mons[PM_QUEEN_ANT]);

    if (is_longworm(pm1))
        return is_longworm(pm2); /* handles tail */
    /* [currently there's no reason to bother matching up
        assorted bugs and blobs with their closest variants] */
    /* didn't match */
    return FALSE;
}

/* return an index into the mons array */
int
monsndx(ptr)
struct permonst *ptr;
{
    register int i;

    i = (int) (ptr - &mons[0]);
    if (i < LOW_PM || i >= NUMMONS) {
        panic("monsndx - could not index monster (%s: %d)",
              fmt_ptr((genericptr_t) ptr), i);
        return NON_PM; /* will not get here */
    }
    return i;
}

/* Monster race or type name mapping from MH_ values in monflag.h, used for glow warnings. */
const char *
mon_race_name(mhflag)
unsigned mhflag;
{
    static const char* mrnames[] = {
        "human", "elf", "dwarf", "gnome",
        "orc", "giant", "hobbit", "centaur",
        "illithid", "tortle", "drow", "zombie",
        "vampire", "demon",
        "undead",
        "lycanthrope",
        "dragon",
        "angelic being",
        "ogre",
        "troll",
        "gnoll",
        "spider",
        "jabberwock",
        "wraith"
    };
    return mrnames[mhflag];
}

/* for handling alternate spellings */
struct alt_spl {
    const char *name;
    short pm_val;
};

/* figure out what type of monster a user-supplied string is specifying */
int
name_to_mon(in_str, matchlen)
const char *in_str;
int *matchlen;
{
    /* Be careful.  We must check the entire string in case it was
     * something such as "ettin zombie corpse".  The calling routine
     * doesn't know about the "corpse" until the monster name has
     * already been taken off the front, so we have to be able to
     * read the name with extraneous stuff such as "corpse" stuck on
     * the end.
     * This causes a problem for names which prefix other names such
     * as "ettin" on "ettin zombie".  In this case we want the _longest_
     * name which exists.
     * This also permits plurals created by adding suffixes such as 's'
     * or 'es'.  Other plurals must still be handled explicitly.
     */
    register int i;
    register int mntmp = NON_PM;
    register char *s, *str, *term;
    char buf[BUFSZ];
    int len, slen;

    str = strcpy(buf, in_str);

    if (!strncmp(str, "a ", 2))
        str += 2;
    else if (!strncmp(str, "an ", 3))
        str += 3;
    else if (!strncmp(str, "the ", 4))
        str += 4;

    slen = strlen(str);
    term = str + slen;

    if ((s = strstri(str, "vortices")) != 0)
        Strcpy(s + 4, "ex");
    /* nobles and royalty */
    if (slen > 5 && (s = strstri(term - 5, " lady")) != 0)
        Strcpy(term - 5, "noble");
    else if (slen > 5 && (s = strstri(term - 5, " lord")) != 0)
        Strcpy(term - 5, "noble");
    else if (slen > 6 && (s = strstri(term - 6, " queen")) != 0
             && strncmpi(str, "kathryn ", 8))
        Strcpy(term - 5, "royal");
    else if (slen > 5 && (s = strstri(term - 5, " king")) != 0
             && strncmpi(str, "rat ", 4) && strncmpi(str, "goblin ", 7))
        Strcpy(term - 5, "royal");
    else if (slen > 9 && (s = strstri(term - 9, " baroness")) != 0)
        Strcpy(term - 9, "sovereign");
    else if (slen > 6 && (s = strstri(term - 6, " baron")) != 0)
        Strcpy(term - 9, "sovereign");
    /* be careful with "ies"; "priest", "zombies" */
    else if (slen > 3 && !strcmpi(term - 3, "ies")
             && (slen < 7 || strcmpi(term - 7, "zombies")))
        Strcpy(term - 3, "y");
    /* luckily no monster names end in fe or ve with ves plurals */
    else if (slen > 3 && !strcmpi(term - 3, "ves"))
        Strcpy(term - 3, "f");

    slen = strlen(str); /* length possibly needs recomputing */

    {
        static const struct alt_spl names[] = {
            /* Alternate spellings */
            { "grey dragon", PM_GRAY_DRAGON },
            { "baby grey dragon", PM_BABY_GRAY_DRAGON },
            { "grey unicorn", PM_GRAY_UNICORN },
            { "grey ooze", PM_GRAY_OOZE },
            { "gray-elf", PM_GREY_ELF },
            { "mindflayer", PM_MIND_FLAYER },
            { "master mindflayer", PM_MASTER_MIND_FLAYER },
            /* More alternates; priest and priestess are separate monster
               types but that isn't the case for {aligned,high} priests */
            { "aligned priestess", PM_ALIGNED_PRIEST },
            { "high priestess", PM_HIGH_PRIEST },
            /* Inappropriate singularization by -ves check above */
            { "master of thief", PM_MASTER_OF_THIEVES },
            /* Potential misspellings where we want to avoid falling back
               to the rank title prefix (input has been singularized) */
            { "master thief", PM_MASTER_OF_THIEVES },
            { "master of assassin", PM_MASTER_ASSASSIN },
            /* Outdated names */
            { "invisible stalker", PM_STALKER },
            { "high-elf", PM_ELVEN_ROYAL }, /* PM_HIGH_ELF is obsolete */
            /* other misspellings or incorrect words */
            { "wood-elf", PM_WOODLAND_ELF },
            { "wood elf", PM_WOODLAND_ELF },
            { "woodland nymph", PM_WOOD_NYMPH },
            { "halfling", PM_HOBBIT },    /* potential guess for polyself */
            { "genie", PM_DJINNI }, /* potential guess for ^G/#wizgenesis */
            /* Hyphenated names -- it would be nice to handle these via
               fuzzymatch() but it isn't able to ignore trailing stuff */
            { "ki rin", PM_KI_RIN },
            { "eldritch ki rin", PM_ELDRITCH_KI_RIN },
            { "uruk hai", PM_URUK_HAI },
            { "orc captain", PM_ORC_CAPTAIN },
            { "goblin captain", PM_GOBLIN_CAPTAIN },
            { "woodland elf", PM_WOODLAND_ELF },
            { "green elf", PM_GREEN_ELF },
            { "grey elf", PM_GREY_ELF },
            { "gray elf", PM_GREY_ELF },
            { "olog hai", PM_OLOG_HAI },
            { "arch lich", PM_ARCH_LICH },
            /* Some irregular plurals */
            { "incubi", PM_INCUBUS },
            { "succubi", PM_SUCCUBUS },
            { "violet fungi", PM_VIOLET_FUNGUS },
            { "homunculi", PM_HOMUNCULUS },
            { "baluchitheria", PM_BALUCHITHERIUM },
            { "lurkers above", PM_LURKER_ABOVE },
            { "cavemen", PM_CAVEMAN },
            { "cavewomen", PM_CAVEWOMAN },
            { "watchmen", PM_WATCHMAN },
            { "djinn", PM_DJINNI },
            { "mumakil", PM_MUMAK },
            { "erinyes", PM_ERINYS },
            /* end of list */
            { 0, NON_PM }
        };
        register const struct alt_spl *namep;

        for (namep = names; namep->name; namep++)
            if (!strncmpi(str, namep->name, (int) strlen(namep->name)))
                return namep->pm_val;
    }

    for (len = 0, i = LOW_PM; i < NUMMONS; i++) {
        register int m_i_len = (int) strlen(mons[i].mname);

        if (m_i_len > len && !strncmpi(mons[i].mname, str, m_i_len)) {
            if (m_i_len == slen) {
                mntmp = i;
                if (matchlen)
                    *matchlen = m_i_len;
                break; /* exact match */
            } else if (slen > m_i_len
                       && (str[m_i_len] == ' '
                           || !strcmpi(&str[m_i_len], "s")
                           || !strncmpi(&str[m_i_len], "s ", 2)
                           || !strcmpi(&str[m_i_len], "'")
                           || !strncmpi(&str[m_i_len], "' ", 2)
                           || !strcmpi(&str[m_i_len], "'s")
                           || !strncmpi(&str[m_i_len], "'s ", 3)
                           || !strcmpi(&str[m_i_len], "es")
                           || !strncmpi(&str[m_i_len], "es ", 3))) {
                mntmp = i;
                if (matchlen)
                    *matchlen = m_i_len;
                len = m_i_len;
            }
        }
    }
    if (mntmp == NON_PM)
        mntmp = title_to_mon(str, (int *) 0, (int *) 0);
    return mntmp;
}

/* monster class from user input; used for genocide and controlled polymorph;
   returns 0 rather than MAXMCLASSES if no match is found */
int
name_to_monclass(in_str, mndx_p)
const char *in_str;
int *mndx_p;
{
    /* Single letters are matched against def_monsyms[].sym; words
       or phrases are first matched against def_monsyms[].explain
       to check class description; if not found there, then against
       mons[].mname to test individual monster types.  Input can be a
       substring of the full description or mname, but to be accepted,
       such partial matches must start at beginning of a word.  Some
       class descriptions include "foo or bar" and "foo or other foo"
       so we don't want to accept "or", "other", "or other" there. */
    static NEARDATA const char *const falsematch[] = {
        /* multiple-letter input which matches any of these gets rejected */
        "an", "the", "or", "other", "or other", 0
    };
    /* positive pm_val => specific monster; negative => class */
    static NEARDATA const struct alt_spl truematch[] = {
        /* "long worm" won't match "worm" class but would accidentally match
           "long worm tail" class before the comparison with monster types */
        { "long worm", PM_LONG_WORM },
        /* matches wrong--or at least suboptimal--class */
        { "demon", -S_DEMON }, /* hits "imp or minor demon" */
        /* matches specific monster (overly restrictive) */
        { "devil", -S_DEMON }, /* always "horned devil" */
        /* some plausible guesses which need help */
        { "bug", -S_XAN },  /* would match bugbear... */
        { "fish", -S_EEL }, /* wouldn't match anything */
        /* end of list */
        { 0, NON_PM }
    };
    const char *p, *x;
    int i, len;

    if (mndx_p)
        *mndx_p = NON_PM; /* haven't [yet] matched a specific type */

    if (!in_str || !in_str[0]) {
        /* empty input */
        return 0;
    } else if (!in_str[1]) {
        /* single character */
        i = def_char_to_monclass(*in_str);
        if (i == S_MIMIC_DEF) { /* ']' -> 'm' */
            i = S_MIMIC;
        } else if (i == S_WORM_TAIL) { /* '~' -> 'w' */
            i = S_WORM;
            if (mndx_p)
                *mndx_p = PM_LONG_WORM;
        } else if (i == MAXMCLASSES) /* maybe 'I' */
            i = (*in_str == DEF_INVISIBLE) ? S_invisible : 0;
        return i;
    } else {
        /* multiple characters */
        if (!strcmpi(in_str, "long")) /* not enough to match "long worm" */
            return 0; /* avoid false whole-word match with "long worm tail" */
        in_str = makesingular(in_str);
        /* check for special cases */
        for (i = 0; falsematch[i]; i++)
            if (!strcmpi(in_str, falsematch[i]))
                return 0;
        for (i = 0; truematch[i].name; i++)
            if (!strcmpi(in_str, truematch[i].name)) {
                i = truematch[i].pm_val;
                if (i < 0)
                    return -i; /* class */
                if (mndx_p)
                    *mndx_p = i; /* monster */
                return mons[i].mlet;
            }
        /* check monster class descriptions */
        len = (int) strlen(in_str);
        for (i = 1; i < MAXMCLASSES; i++) {
            x = def_monsyms[i].explain;
            if ((p = strstri(x, in_str)) != 0 && (p == x || *(p - 1) == ' ')
                && ((int) strlen(p) >= len
                    && (p[len] == '\0' || p[len] == ' ')))
                return i;
        }
        /* check individual species names */
        i = name_to_mon(in_str, (int *) 0);
        if (i != NON_PM) {
            if (mndx_p)
                *mndx_p = i;
            return mons[i].mlet;
        }
    }
    return 0;
}

/* returns 3 values (0=male, 1=female, 2=none) */
int
gender(mtmp)
register struct monst *mtmp;
{
    if (is_neuter(mtmp->data))
        return 2;
    return mtmp->female;
}

/* Like gender(), but lower animals and such are still "it".
   This is the one we want to use when printing messages. */
int
pronoun_gender(mtmp, override_vis)
register struct monst *mtmp;
boolean override_vis; /* if True then 'no it' unless neuter */
{
    if (!override_vis && !canspotmon(mtmp))
        return 2;
    if (is_neuter(mtmp->data))
        return 2;
    return (humanoid(mtmp->data) || (mtmp->data->geno & G_UNIQ)
            || type_is_pname(mtmp->data)) ? (int) mtmp->female : 2;
}

/* used for nearby monsters when you go to another level */
boolean
levl_follower(mtmp)
struct monst *mtmp;
{
    if (mtmp == u.usteed || mtmp->ridden_by)
        return TRUE;

    /* Wizard with Amulet won't bother trying to follow across levels */
    if (mtmp->iswiz && mon_has_amulet(mtmp))
        return FALSE;
    /* some monsters will follow even while intending to flee from you */
    if (mtmp->mtame || mtmp->iswiz || is_fshk(mtmp))
        return TRUE;
    /* stalking types follow, but won't when fleeing unless you hold
       the Amulet */
    return (boolean) ((mtmp->data->mflags2 & M2_STALK)
                      && (!mtmp->mflee || u.uhave.amulet));
}

static const short grownups[][2] = {
    { PM_CHICKATRICE, PM_COCKATRICE },
    { PM_LITTLE_DOG, PM_DOG },
    { PM_DOG, PM_LARGE_DOG },
    { PM_SMALL_SKELETAL_HOUND, PM_SKELETAL_HOUND },
    { PM_SKELETAL_HOUND, PM_LARGE_SKELETAL_HOUND },
    { PM_HELL_HOUND_PUP, PM_HELL_HOUND },
    { PM_WOLF_CUB, PM_WOLF },
    { PM_WINTER_WOLF_CUB, PM_WINTER_WOLF },
    { PM_KITTEN, PM_HOUSECAT },
    { PM_HOUSECAT, PM_LARGE_CAT },
    { PM_PONY, PM_HORSE },
    { PM_HORSE, PM_WARHORSE },
    { PM_SKELETAL_PONY, PM_SKELETAL_HORSE },
    { PM_SKELETAL_HORSE, PM_SKELETAL_WARHORSE },
    { PM_LESSER_NIGHTMARE, PM_NIGHTMARE },
    { PM_NIGHTMARE, PM_CAUCHEMAR },
    { PM_PEGASUS, PM_GREATER_PEGASUS },
    { PM_KOBOLD, PM_LARGE_KOBOLD },
    { PM_LARGE_KOBOLD, PM_KOBOLD_NOBLE },
    { PM_ROCK_GNOME, PM_GNOME_NOBLE },
    { PM_GNOME_NOBLE, PM_GNOME_ROYAL },
    { PM_MOUNTAIN_DWARF, PM_DWARF_NOBLE },
    { PM_DWARF_NOBLE, PM_DWARF_ROYAL },
    { PM_GNOLL, PM_GNOLL_HUNTER },
    { PM_GNOLL_HUNTER, PM_FLIND },
    { PM_MIND_FLAYER, PM_MASTER_MIND_FLAYER },
    { PM_GOBLIN, PM_HOBGOBLIN },
    { PM_HILL_ORC, PM_URUK_HAI },
    { PM_MORDOR_ORC, PM_URUK_HAI },
    { PM_URUK_HAI, PM_ORC_CAPTAIN },
    { PM_SEWER_RAT, PM_GIANT_RAT },
    { PM_GIANT_RAT, PM_ENORMOUS_RAT },
    { PM_ENORMOUS_RAT, PM_RODENT_OF_UNUSUAL_SIZE },
    { PM_LESSER_HOMUNCULUS, PM_HOMUNCULUS },
    { PM_HOMUNCULUS, PM_GREATER_HOMUNCULUS },
    { PM_CAVE_SPIDER, PM_LARGE_SPIDER },
    { PM_LARGE_SPIDER, PM_GIANT_SPIDER },
    { PM_GIANT_SPIDER, PM_GARGANTUAN_SPIDER },
    { PM_OGRE, PM_OGRE_NOBLE },
    { PM_OGRE_NOBLE, PM_OGRE_ROYAL },
    { PM_DROW_RAIDER, PM_DROW_WARRIOR },
    { PM_WOODLAND_ELF, PM_GREY_ELF },
    { PM_GREEN_ELF, PM_GREY_ELF },
    { PM_GREY_ELF, PM_ELVEN_NOBLE },
    { PM_ELVEN_NOBLE, PM_ELVEN_ROYAL },
    { PM_LICH, PM_DEMILICH },
    { PM_DEMILICH, PM_MASTER_LICH },
    { PM_MASTER_LICH, PM_ARCH_LICH },
    { PM_VAMPIRE_SOVEREIGN, PM_VAMPIRE_NOBLE },
    { PM_VAMPIRE_NOBLE, PM_VAMPIRE_ROYAL },
    { PM_BAT, PM_GIANT_BAT },
    { PM_HAWK, PM_LARGE_HAWK },
    { PM_LARGE_HAWK, PM_GIANT_HAWK },
    { PM_BABY_GRAY_DRAGON, PM_GRAY_DRAGON },
    { PM_BABY_SILVER_DRAGON, PM_SILVER_DRAGON },
    { PM_BABY_SHIMMERING_DRAGON, PM_SHIMMERING_DRAGON },
    { PM_BABY_RED_DRAGON, PM_RED_DRAGON },
    { PM_BABY_WHITE_DRAGON, PM_WHITE_DRAGON },
    { PM_BABY_ORANGE_DRAGON, PM_ORANGE_DRAGON },
    { PM_BABY_BLACK_DRAGON, PM_BLACK_DRAGON },
    { PM_BABY_BLUE_DRAGON, PM_BLUE_DRAGON },
    { PM_BABY_GREEN_DRAGON, PM_GREEN_DRAGON },
    { PM_BABY_GOLD_DRAGON, PM_GOLD_DRAGON },
    { PM_BABY_SEA_DRAGON, PM_SEA_DRAGON },
    { PM_BABY_YELLOW_DRAGON, PM_YELLOW_DRAGON },
    { PM_BABY_SHADOW_DRAGON, PM_SHADOW_DRAGON },
    { PM_BABY_CELESTIAL_DRAGON, PM_CELESTIAL_DRAGON },
    { PM_PSEUDODRAGON, PM_ELDER_PSEUDODRAGON },
    { PM_ELDER_PSEUDODRAGON, PM_ANCIENT_PSEUDODRAGON },
    { PM_RED_NAGA_HATCHLING, PM_RED_NAGA },
    { PM_BLACK_NAGA_HATCHLING, PM_BLACK_NAGA },
    { PM_GOLDEN_NAGA_HATCHLING, PM_GOLDEN_NAGA },
    { PM_GUARDIAN_NAGA_HATCHLING, PM_GUARDIAN_NAGA },
    { PM_SMALL_MIMIC, PM_LARGE_MIMIC },
    { PM_LARGE_MIMIC, PM_GIANT_MIMIC },
    { PM_BABY_LONG_WORM, PM_LONG_WORM },
    { PM_BABY_PURPLE_WORM, PM_PURPLE_WORM },
    { PM_BABY_CROCODILE, PM_CROCODILE },
    { PM_CROCODILE, PM_GIANT_CROCODILE },
    { PM_CAVE_LIZARD, PM_LARGE_CAVE_LIZARD },
    { PM_SOLDIER, PM_SERGEANT },
    { PM_SERGEANT, PM_LIEUTENANT },
    { PM_LIEUTENANT, PM_CAPTAIN },
    { PM_WATCHMAN, PM_WATCH_CAPTAIN },
    { PM_ALIGNED_PRIEST, PM_HIGH_PRIEST },
    { PM_STUDENT, PM_ARCHEOLOGIST },
    { PM_ASPIRANT, PM_DRUID },
    { PM_ATTENDANT, PM_HEALER },
    { PM_PAGE, PM_KNIGHT },
    { PM_ACOLYTE, PM_PRIESTESS },
    { PM_APPRENTICE, PM_WIZARD },
    { PM_MANES, PM_LEMURE },
    { PM_KEYSTONE_KOP, PM_KOP_SERGEANT },
    { PM_KOP_SERGEANT, PM_KOP_LIEUTENANT },
    { PM_KOP_LIEUTENANT, PM_KOP_KAPTAIN },
    { PM_BABY_OWLBEAR, PM_OWLBEAR },
    { PM_ENT, PM_ELDER_ENT },
    { PM_TWIG_BLIGHT, PM_NEEDLE_BLIGHT },
    { PM_GRIZZLY_BEAR, PM_CAVE_BEAR },
    { PM_WOLVERINE, PM_DIRE_WOLVERINE },
    { NON_PM, NON_PM }
};

int
little_to_big(montype)
int montype;
{
    register int i;

    for (i = 0; grownups[i][0] >= LOW_PM; i++)
        if (montype == grownups[i][0]) {
            montype = grownups[i][1];
            break;
        }
    return montype;
}

int
big_to_little(montype)
int montype;
{
    register int i;

    /* Exceptions, such as a monster type that is
       oviporous that spans more than two classes
       of monsters */
    if (montype == PM_GIANT_CROCODILE)
        return PM_BABY_CROCODILE;

    for (i = 0; grownups[i][0] >= LOW_PM; i++)
        if (montype == grownups[i][1]) {
            montype = grownups[i][0];
            break;
        }
    return montype;
}

/* determine whether two permonst indices are part of the same progression;
   existence of progressions with more than one step makes it a bit tricky */
boolean
big_little_match(montyp1, montyp2)
int montyp1, montyp2;
{
    int l, b;

    /* simplest case: both are same pm */
    if (montyp1 == montyp2)
        return TRUE;
    /* assume it isn't possible to grow from one class letter to another */
    if (mons[montyp1].mlet != mons[montyp2].mlet)
        return FALSE;
    /* check whether montyp1 can grow up into montyp2 */
    for (l = montyp1; (b = little_to_big(l)) != l; l = b)
        if (b == montyp2)
            return TRUE;
    /* check whether montyp2 can grow up into montyp1 */
    for (l = montyp2; (b = little_to_big(l)) != l; l = b)
        if (b == montyp1)
            return TRUE;
    /* neither grows up to become the other; no match */
    return FALSE;
}

/*
 * Return the permonst ptr for the race of the monster.
 * Returns correct pointer for non-polymorphed and polymorphed
 * player.  It does not return a pointer to player role character.
 */
struct permonst *
raceptr(mtmp)
struct monst *mtmp;
{
    if (mtmp == &youmonst && !Upolyd)
        return &mons[urace.malenum];
    else
        return mtmp->data;
}

static const char *levitate[4] = { "float", "Float", "wobble", "Wobble" };
static const char *flys[4] = { "fly", "Fly", "flutter", "Flutter" };
static const char *flyl[4] = { "fly", "Fly", "stagger", "Stagger" };
static const char *slither[4] = { "slither", "Slither", "falter", "Falter" };
static const char *ooze[4] = { "ooze", "Ooze", "tremble", "Tremble" };
static const char *immobile[4] = { "wiggle", "Wiggle", "pulsate", "Pulsate" };
static const char *crawl[4] = { "crawl", "Crawl", "falter", "Falter" };

const char *
locomotion(ptr, def)
const struct permonst *ptr;
const char *def;
{
    int capitalize = (*def == highc(*def));

    return (is_floater(ptr) ? levitate[capitalize]
            : (is_flyer(ptr) && ptr->msize <= MZ_SMALL) ? flys[capitalize]
              : (is_flyer(ptr) && ptr->msize > MZ_SMALL) ? flyl[capitalize]
                : slithy(ptr) ? slither[capitalize]
                  : amorphous(ptr) ? ooze[capitalize]
                    : !ptr->mmove ? immobile[capitalize]
                      : nolimbs(ptr) ? crawl[capitalize]
                        : def);
}

const char *
stagger(ptr, def)
const struct permonst *ptr;
const char *def;
{
    int capitalize = 2 + (*def == highc(*def));

    return (is_floater(ptr) ? levitate[capitalize]
            : (is_flyer(ptr) && ptr->msize <= MZ_SMALL) ? flys[capitalize]
              : (is_flyer(ptr) && ptr->msize > MZ_SMALL) ? flyl[capitalize]
                : slithy(ptr) ? slither[capitalize]
                  : amorphous(ptr) ? ooze[capitalize]
                    : !ptr->mmove ? immobile[capitalize]
                      : nolimbs(ptr) ? crawl[capitalize]
                        : def);
}

/* return phrase describing the effect of fire attack on a type of monster */
const char *
on_fire(mptr, attktype)
struct monst *mptr;
enum on_fire_types attktype;
{
    const char *what;

    switch (monsndx(mptr->data)) {
    case PM_FLAMING_SPHERE:
    case PM_ORB_OF_FIRE:
    case PM_FIRE_VORTEX:
    case PM_FIRE_ELEMENTAL:
    case PM_SALAMANDER:
        what = "already on fire";
        break;
    case PM_WATER_ELEMENTAL:
    case PM_FOG_CLOUD:
    case PM_STEAM_VORTEX:
    case PM_WATER_TROLL:
    case PM_BABY_SEA_DRAGON:
    case PM_SEA_DRAGON:
    case PM_ACID_SPHERE:
        switch (attktype) {
        case ON_FIRE_DEAD:
            what = (mptr == &youmonst) ? "boil away" : "boils away";
            break;
        case ON_FIRE_ENGULF:
            what = "boiling away";
            break;
        default:
            what = "boiling";
            break;
        }
        break;
    case PM_ICE_VORTEX:
    case PM_SNOW_GOLEM:
    case PM_ABOMINABLE_SNOWMAN:
    case PM_FREEZING_SPHERE:
    case PM_ORB_OF_FROST:
        /* Melts and then boils away or evaporates. */
        switch (attktype) {
        case ON_FIRE_DEAD:
            what = (mptr == &youmonst) ? "melt away" : "melts away";
            break;
        case ON_FIRE_ENGULF:
            what = "melting away";
            break;
        default:
            what = "melting";
            break;
        }
        break;
    case PM_GLASS_GOLEM:
    case PM_GOLD_GOLEM:
        /* Melts into a puddle. */
        switch (attktype) {
        case ON_FIRE_DEAD:
            what = (mptr == &youmonst) ? "fully melt" : "fully melts";
            break;
        case ON_FIRE_ENGULF:
            what = "melting away";
            break;
        default:
            what = "melting";
            break;
        }
        break;
    case PM_STONE_GOLEM:
    case PM_CLAY_GOLEM:
    case PM_EARTH_ELEMENTAL:
        switch (attktype) {
        case ON_FIRE_DEAD:
            what = (mptr == &youmonst) ? "burn to a crisp" : "burns to a crisp";
            break;
        case ON_FIRE_ENGULF:
            what = "burning to a crisp";
            break;
        default:
            what = "heating up";
            break;
        }
        break;
    case PM_AIR_ELEMENTAL:
    case PM_DUST_VORTEX:
    case PM_ENERGY_VORTEX:
    case PM_SHOCKING_SPHERE:
    case PM_ANTIMATTER_VORTEX:
        /* Gas or plasma that gets cooked off. */
        switch (attktype) {
        case ON_FIRE_DEAD:
            what = (mptr == &youmonst) ? "burn away" : "burns away";
            break;
        case ON_FIRE_ENGULF:
            what = "burning away";
            break;
        default:
            what = "heating up";
            break;
        }
        break;
    default:
        switch (attktype) {
        case ON_FIRE_DEAD:
            what = (mptr == &youmonst) ? "burn to a crisp" : "burns to a crisp";
            break;
        case ON_FIRE_ENGULF:
            what = "burning to a crisp";
            break;
        case ON_FIRE_HUG:
            what = "being roasted";
            break;
        default:
            what = "on fire";
            break;
        }
        break;
    }
    return what;
}

/*
 * Returns:
 *      True if monster is presumed to have a sense of smell.
 *      False if monster definitely does not have a sense of smell.
 *
 * Do not base this on presence of a head or nose, since many
 * creatures sense smells other ways (feelers, forked-tongues, etc.)
 * We're assuming all insects can smell at a distance too.
 */
boolean
olfaction(mdat)
struct permonst *mdat;
{
    if (is_golem(mdat)
        || mdat->mlet == S_EYE /* spheres  */
        || mdat->mlet == S_JELLY || mdat->mlet == S_PUDDING
        || mdat->mlet == S_BLOB || mdat->mlet == S_VORTEX
        || mdat->mlet == S_ELEMENTAL
        || mdat->mlet == S_FUNGUS /* mushrooms and fungi */
        || mdat->mlet == S_LIGHT)
        return FALSE;
    return TRUE;
}

/* Return the material a monster is composed of. */
int
monmaterial(mndx)
int mndx;
{
    switch (mndx) {
    case PM_GARGOYLE:
    case PM_WINGED_GARGOYLE:
    case PM_EARTH_ELEMENTAL:
        return MINERAL;
    case PM_BONE_DEVIL:
    case PM_SKELETON:
    case PM_SKELETON_WARRIOR:
    case PM_SMALL_SKELETAL_HOUND:
    case PM_SKELETAL_PONY:
    case PM_SKELETAL_HOUND:
    case PM_SKELETAL_HORSE:
    case PM_LARGE_SKELETAL_HOUND:
    case PM_SKELETAL_WARHORSE:
        return BONE;
    case PM_PAPER_GOLEM:
        return PAPER;
    case PM_GOLD_GOLEM:
        return GOLD;
    case PM_LEATHER_GOLEM:
        return LEATHER;
    case PM_WOOD_GOLEM:
    case PM_ENT:
    case PM_ELDER_ENT:
    case PM_TWIG_BLIGHT:
    case PM_TREE_BLIGHT:
    case PM_NEEDLE_BLIGHT:
    case PM_GULTHIAS_TREE:
        return WOOD;
    case PM_CLAY_GOLEM:
    case PM_STONE_GOLEM:
    case PM_PETRIFIED_ENT:
        return MINERAL;
    case PM_GLASS_GOLEM:
        return GLASS;
    case PM_IRON_GOLEM:
        return IRON;
    case PM_ASSASSIN_VINE:
    case PM_CREEPING_MOUND:
        return VEGGY;
    default:
        return 0;
    }
}

/* Return TRUE if the monster has flesh. */
boolean
is_fleshy(ptr)
const struct permonst *ptr;
{
    if (vegetarian(ptr)) {
        /* vegetarian monsters generally don't have flesh */
        return FALSE;
    }
    if (noncorporeal(ptr)) {
        /* these certainly don't have flesh */
        return FALSE;
    }
    if (!nonliving(ptr)) {
        /* nonvegetarian, alive monsters generally do */
        return TRUE;
    }
    if (ptr->mlet == S_MUMMY || ptr->mlet == S_VAMPIRE
        || ptr->mlet == S_ZOMBIE || ptr == &mons[PM_FLESH_GOLEM]) {
        /* Exceptions: non-living monsters that do have flesh */
        return TRUE;
    }
    return FALSE;
}

/*mondata.c*/
