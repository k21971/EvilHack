/* NetHack 3.6	mhitu.c	$NHDT-Date: 1575245065 2019/12/02 00:04:25 $  $NHDT-Branch: NetHack-3.6 $:$NHDT-Revision: 1.168 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2012. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "artifact.h"

extern boolean notonhead;

STATIC_VAR NEARDATA struct obj *mon_currwep = (struct obj *) 0;

STATIC_DCL boolean FDECL(u_slip_free, (struct monst *, struct attack *));
STATIC_DCL int FDECL(passiveum, (struct permonst *, struct monst *,
                                 struct attack *));
STATIC_DCL void FDECL(mayberem, (struct monst *, const char *,
                                 struct obj *, const char *));
STATIC_DCL int FDECL(hitmu, (struct monst *, struct attack *));
STATIC_DCL int FDECL(gulpmu, (struct monst *, struct attack *));
STATIC_DCL int FDECL(explmu, (struct monst *, struct attack *, BOOLEAN_P));
STATIC_DCL int FDECL(screamu, (struct monst *, struct attack *));
STATIC_DCL void FDECL(missmu, (struct monst *, int, int, struct attack *));
STATIC_DCL void FDECL(mswings, (struct monst *, struct obj *));
STATIC_DCL void FDECL(wildmiss, (struct monst *, struct attack *));
STATIC_DCL void FDECL(hitmsg, (struct monst *, struct attack *));

static const char *const mwep_pierce[] = {
    "pierce", "gore", "stab", "impale", "hit"
};

static const char *const mwep_slash[] = {
    "strike", "hack", "cut", "lacerate", "hit"
};

static const char *const mwep_whack[] = {
    "smashe", "whack", "pound", "bashe", "hit"
};

static const char *const mwep_none[] = {
    "punche", "pummel", "hit"
};

/* See comment in mhitm.c.  If we use this a lot it probably should be */
/* changed to a parameter to mhitu. */
static int dieroll;

STATIC_OVL void
hitmsg(mtmp, mattk)
struct monst *mtmp;
struct attack *mattk;
{
    int compat;
    const char *pfmt = 0;
    char *Monst_name = Monnam(mtmp);

    /* Note: if opposite gender, "seductively" */
    /* If same gender, "engagingly" for nymph, normal msg for others */
    if ((compat = could_seduce(mtmp, &youmonst, mattk)) != 0
        && !mtmp->mcan && !mtmp->mspec_used) {
        pline("%s %s you %s.", Monst_name,
              !Blind ? "smiles at" : !Deaf ? "talks to" : "touches",
              (compat == 2) ? "engagingly" : "seductively");
    } else {
        switch (mattk->aatyp) {
        case AT_BITE:
            pline("%s %ss!", Monnam(mtmp), has_beak(mtmp->data) ?
                  "peck" : "bite");
            break;
        case AT_KICK:
            pline("%s %s%c", Monst_name,
                  is_quadruped(mtmp->data)
                    ? "tramples you" : "kicks",
                  (thick_skinned(youmonst.data)
                   || Barkskin || Stoneskin) ? '.' : '!');
            break;
        case AT_STNG:
            pfmt = "%s stings!";
            break;
        case AT_BUTT:
            pline("%s %s!", Monnam(mtmp),
                  has_trunk(mtmp->data) ? "gores you with its tusks"
                                        : num_horns(mtmp->data) > 1
                                          ? "gores you with its horns"
                                          : num_horns(mtmp->data) == 1
                                            ? "gores you with its horn" : "butts");
            break;
        case AT_TUCH:
            if (mtmp->data == &mons[PM_GIANT_CENTIPEDE])
                pline("%s coils its body around you!", Monnam(mtmp));
            else if (mtmp->data == &mons[PM_ASSASSIN_VINE])
                pline("%s lashes its thorny vines at you!", Monnam(mtmp));
            else
                pfmt = "%s touches you!";
            break;
        case AT_TENT:
            if (mtmp->data == &mons[PM_MEDUSA])
                pline_The("venomous snakes on %s head %s you!",
                          s_suffix(mon_nam(mtmp)),
                          rn2(2) ? "lash out at" : "bite");
            else if (mtmp->data == &mons[PM_DEMOGORGON]
                     || mtmp->data == &mons[PM_NEOTHELID])
                pline("%s tentacles lash out at you!",
                      s_suffix(Monnam(mtmp)));
            else
                pfmt = "%s tentacles suck you!";
            Monst_name = s_suffix(Monst_name);
            break;
        case AT_EXPL:
        case AT_BOOM:
            pfmt = "%s explodes!";
            break;
        case AT_WEAP:
            if (!MON_WEP(mtmp)) { /* AT_WEAP but isn't wielding anything */
                if (has_claws(r_data(mtmp)))
                    pfmt = "%s claws you!";
                else if (has_claws_undead(r_data(mtmp)))
                    pfmt = "%s scratches you!";
                else
                    pline("%s %ss you!", Monnam(mtmp),
                          mwep_none[rn2(SIZE(mwep_none))]);
            } else if (is_pierce(MON_WEP(mtmp)))
                pline("%s %ss you!", Monnam(mtmp),
                      mwep_pierce[rn2(SIZE(mwep_pierce))]);
            else if (is_slash(MON_WEP(mtmp)))
                pline("%s %ss you!", Monnam(mtmp),
                      mwep_slash[rn2(SIZE(mwep_slash))]);
            else if (is_whack(MON_WEP(mtmp)))
                pline("%s %ss you!", Monnam(mtmp),
                      mwep_whack[rn2(SIZE(mwep_whack))]);
            else
                pfmt = "%s hits you!";
            break;
        case AT_CLAW:
            if (has_claws(r_data(mtmp)))
                pfmt = "%s claws you!";
            else if (has_claws_undead(r_data(mtmp)))
                pfmt = "%s scratches you!";
            else
                pfmt = "%s hits!";
            break;
        default:
            pfmt = "%s hits!";
        }
        if (pfmt)
            pline(pfmt, Monst_name);
    }
}

/* monster missed you */
/* verbose miss descriptions taken from Slash'EM */
/* slightly edited for EvilHack */
STATIC_OVL void
missmu(mtmp, target, roll, mattk)
struct monst *mtmp;
int target;
int roll;
struct attack *mattk;
{
    struct obj *blocker = (struct obj *) 0;
    struct permonst *mdat = mtmp->data;
    boolean nearmiss = (target == roll),
            already_killed = FALSE;
    int tmp = rnd(5) + 3;
    /* 3 values for blocker
     * No blocker: (struct obj *) 0
     * Piece of armour: object
     * magical: &zeroobj
     */

    if (target < roll) {
        /* get object responsible,
           work from the closest to the skin outwards */
        if (uarmu && !uarm && !uarmc
            && target <= roll) {
            /* Try undershirt */
            target += armor_bonus(uarmu);
            if (target > roll)
                blocker = uarmu;
        }
        if (uarm && !uarmc && target <= roll) {
            /* Try body armour */
            target += armor_bonus(uarm);
            if (target > roll)
                blocker = uarm;
        }
        if (uarmg && !rn2(10)) {
            /* Try gloves */
            target += armor_bonus(uarmg);
            if (target > roll)
                blocker = uarmg;
        }
        if (uarmf && !rn2(10)) {
            /* Try boots */
            target += armor_bonus(uarmf);
            if (target > roll)
                blocker = uarmf;
        }
        if (uarmh && !rn2(5)) {
            /* Try helm */
            target += armor_bonus(uarmh);
            if (target > roll)
                blocker = uarmh;
        }
        if (uarmc && target <= roll) {
            /* Try cloak */
            target += armor_bonus(uarmc);
            if (target > roll)
                blocker = uarmc;
        }
        if (uarms && target <= roll) {
            /* Try shield */
            target += armor_bonus(uarms);
            if (target > roll)
                blocker = uarms;
        }
        if (target <= roll) {
            /* Try spell protection */
            target += u.uspellprot;
            if (target > roll)
                blocker = (struct obj *) &zeroobj;
        }
    }

    if (!canspotmon(mtmp))
        map_invisible(mtmp->mx, mtmp->my);

    if (could_seduce(mtmp, &youmonst, mattk) && !mtmp->mcan) {
        pline("%s pretends to be friendly.", Monnam(mtmp));
    } else if (!DEADMONSTER(mtmp)) {
        if (!flags.verbose || (!nearmiss && !blocker)) {
            pline("%s misses.", Monnam(mtmp));
        } else if (nearmiss || !blocker) {
            if ((thick_skinned(youmonst.data)
                 || (!Upolyd && Race_if(PM_TORTLE))
                 || Barkskin || Stoneskin)
                && rn2(2)) {
                Your("%s %s %s attack.",
                     (is_dragon(youmonst.data)
                      ? "scaly hide"
                      : (youmonst.data == &mons[PM_GIANT_TURTLE]
                         || (maybe_polyd(is_tortle(youmonst.data),
                                         Race_if(PM_TORTLE))))
                        ? "protective shell"
                        : is_bone_monster(youmonst.data)
                          ? "bony structure"
                          : (has_bark(youmonst.data) || Barkskin)
                            ? "rough bark"
                            : Stoneskin
                              ? "stony hide" : "thick hide"),
                     (rn2(2) ? "blocks" : "deflects"),
                     s_suffix(mon_nam(mtmp)));
            } else {
                if (!Upolyd && Race_if(PM_TORTLE)) {
                    pline("%s narrowly misses!", Monnam(mtmp));
                } else {
                    rn2(2) ? You("dodge %s attack!", s_suffix(mon_nam(mtmp)))
                           : rn2(2) ? You("evade %s attack!", s_suffix(mon_nam(mtmp)))
                                    : pline("%s narrowly misses!", Monnam(mtmp));
                }
            }
        } else if (blocker == &zeroobj) {
            pline("%s is stopped by your golden haze.", Monnam(mtmp));
        } else {
            /* NB: currently all artifact shields (Ashmar) are automatically dknown */
            boolean nameart = (blocker->oartifact && blocker->dknown);
            pline("%s%s %s%s %s attack.", nameart ? "Your " : "",
                 nameart ? xname(blocker) : Ysimple_name2(blocker),
                 rn2(2) ? "block" : "deflect",
                 (((blocker == uarmg) && (blocker->oartifact != ART_DRAGONBANE
                   || !nameart))
                  || (blocker == uarmf)) ? "" : "s",
                 s_suffix(mon_nam(mtmp)));
        }
        if (!blocker)
            goto end;
        /* called if attacker hates the material of the armor
           that deflected their attack */
        if (blocker
            && (!MON_WEP(mtmp) && which_armor(mtmp, W_ARMG) == 0)
            && mon_hates_material(mtmp, blocker->material)
            && (!(has_barkskin(mtmp) || has_stoneskin(mtmp)))) {
            if (DEADMONSTER(mtmp))
                already_killed = TRUE;
            if (!already_killed) {
                /* glancing blow, partial damage */
                searmsg(&youmonst, mtmp, blocker, FALSE);
                damage_mon(mtmp,
                           rnd(sear_damage(blocker->material) / 2),
                           AD_PHYS, TRUE);
                if (DEADMONSTER(mtmp))
                    killed(mtmp);
            }
        }
        /* train shield skill if the shield made a block */
        if (blocker == uarms)
            use_skill(P_SHIELD, 1);
        /* glass armor, or certain drow armor if in the presence
           of light, can potentially break if it deflects an attack */
        if (blocker
            && (is_glass(blocker) || is_adamantine(blocker)
                || blocker->forged_qual == FQ_INFERIOR))
            break_glass_obj(blocker);
        /* the artifact shield Ashmar has a chance to knockback
           the attacker if it deflects an attack. Check for
           dead monster in case the attacker kills themselves
           by some other means from the shield (material hatred) */
        if (!rn2(4) && blocker && (blocker == uarms)
            && !(u.uswallow || unsolid(mdat))
            && blocker->oartifact == ART_ASHMAR) {
            pline("%s knocks %s away from you!",
                  artiname(uarms->oartifact), mon_nam(mtmp));
            u.dx = mtmp->mx - u.ux;
            u.dy = mtmp->my - u.uy;
            if (mhurtle_to_doom(mtmp, tmp, &mdat, TRUE))
                already_killed = TRUE;
            if (!already_killed) {
                damage_mon(mtmp, tmp, AD_PHYS, TRUE);
                if (DEADMONSTER(mtmp))
                    killed(mtmp);
            }
        }
        /* the artifact Armor of Retribution can do the same
           as Ashmar, just not as often */
        if (!rn2(7) && blocker && (blocker == uarm)
            && !(u.uswallow || unsolid(mdat))
            && blocker->oartifact == ART_ARMOR_OF_RETRIBUTION) {
            pline_The("%s knocks %s away from you!",
                      artiname(uarm->oartifact), mon_nam(mtmp));
            u.dx = mtmp->mx - u.ux;
            u.dy = mtmp->my - u.uy;
            if (mhurtle_to_doom(mtmp, tmp, &mdat, TRUE))
                already_killed = TRUE;
            if (!already_killed) {
                damage_mon(mtmp, tmp, AD_PHYS, TRUE);
                if (DEADMONSTER(mtmp))
                    killed(mtmp);
            }
        }
    }
end:
    stop_occupation();
}

/* monster swings obj */
STATIC_OVL void
mswings(mtmp, otemp)
struct monst *mtmp;
struct obj *otemp;
{
    if (is_pierce(MON_WEP(mtmp))) {
        if (flags.verbose && !Blind && canspotmon(mtmp)) {
            pline("%s %s %s%s %s.", Monnam(mtmp),
                  rn2(2) ? "thrusts" : "jabs",
                  (otemp->quan > 1L) ? "one of " : "", mhis(mtmp), xname(otemp));
        }
    } else if (is_slash(MON_WEP(mtmp))) {
        if (flags.verbose && !Blind && canspotmon(mtmp)) {
            pline("%s %s %s%s %s.", Monnam(mtmp),
                  rn2(2) ? "slashes" : "swings",
                  (otemp->quan > 1L) ? "one of " : "", mhis(mtmp), xname(otemp));
        }
    } else if (is_whack(MON_WEP(mtmp))) {
        if (flags.verbose && !Blind && canspotmon(mtmp)) {
            pline("%s %s %s%s %s.", Monnam(mtmp),
                  rn2(2) ? "swings" : "swipes",
                  (otemp->quan > 1L) ? "one of " : "", mhis(mtmp), xname(otemp));
        }
    } else if (flags.verbose && !Blind && canspotmon(mtmp)) {
        pline("%s %s %s%s %s.", Monnam(mtmp), "swings",
              (otemp->quan > 1L) ? "one of " : "", mhis(mtmp), xname(otemp));
    }
}

/* return how a poison attack was delivered */
const char *
mpoisons_subj(mtmp, mattk)
struct monst *mtmp;
struct attack *mattk;
{
    if (mattk->aatyp == AT_WEAP) {
        struct obj *mwep = (mtmp == &youmonst) ? uwep : MON_WEP(mtmp);
        /* "Foo's attack was poisoned." is pretty lame, but at least
           it's better than "sting" when not a stinging attack... */
        return (!mwep || !mwep->opoisoned
                || !mwep->otainted) ? "attack" : "weapon";
    } else {
        return (mattk->aatyp == AT_TUCH) ? "contact"
                  : (mattk->aatyp == AT_GAZE) ? "gaze"
                       : (mattk->aatyp == AT_TENT) ? "snake bite" /* Medusa's hair-do */
                            : (mattk->aatyp == AT_BITE) ? "bite"
                                 : (mattk->aatyp == AT_CLAW) ? "scratch" : "sting";
    }
}

/* called when you are artifically slowed */
void
u_slow_down()
{
    if (defended(&youmonst, AD_SLOW) || resists_slow(youmonst.data)) {
        You("feel as spry as ever.");
        return;
    }
    if (!Fast && !Slow)
        You("slow down.");
    else if (!Slow)	 /* speed of some sort */
        You("feel a strange lethargy overcome you.");
    else
	Your("lethargy seems to be settling in for the long haul.");
    incr_itimeout(&HSlow,rnd(11) + 12);
    exercise(A_DEX, FALSE);
}

/* monster attacked your displaced image */
STATIC_OVL void
wildmiss(mtmp, mattk)
struct monst *mtmp;
struct attack *mattk;
{
    int compat;
    const char *Monst_name; /* Monnam(mtmp) */

    /* no map_invisible() -- no way to tell where _this_ is coming from */

    if (!flags.verbose)
        return;
    if (!cansee(mtmp->mx, mtmp->my))
        return;
    /* maybe it's attacking an image around the corner? */

    compat = ((mattk->adtyp == AD_SEDU || mattk->adtyp == AD_SSEX)
              ? could_seduce(mtmp, &youmonst, mattk) : 0);
    Monst_name = Monnam(mtmp);

    if (!mtmp->mcansee || (Invis && !mon_prop(mtmp, SEE_INVIS))) {
        const char *swings = (mattk->aatyp == AT_BITE) ? "snaps"
                             : (mattk->aatyp == AT_KICK) ? "kicks"
                               : (mattk->aatyp == AT_STNG
                                  || mattk->aatyp == AT_BUTT
                                  || nolimbs(mtmp->data)) ? "lunges"
                                 : "swings";

        if (compat)
            pline("%s tries to touch you and misses!", Monst_name);
        else
            switch (rn2(3)) {
            case 0:
                pline("%s %s wildly and misses!", Monst_name, swings);
                break;
            case 1:
                pline("%s attacks a spot beside you.", Monst_name);
                break;
            case 2:
                pline("%s strikes at %s!", Monst_name,
                      (levl[mtmp->mux][mtmp->muy].typ == WATER)
                        ? "empty water"
                        : "thin air");
                break;
            default:
                pline("%s %s wildly!", Monst_name, swings);
                break;
            }

    } else if (Displaced) {
        /* give 'displaced' message even if hero is Blind */
        if (compat)
            pline("%s smiles %s at your %sdisplaced image...", Monst_name,
                  (compat == 2) ? "engagingly" : "seductively",
                  Invis ? "invisible " : "");
        else
            pline("%s strikes at your %sdisplaced image and misses you!",
                  /* Note:  if you're both invisible and displaced, only
                   * monsters which see invisible will attack your displaced
                   * image, since the displaced image is also invisible. */
                  Monst_name, Invis ? "invisible " : "");

    } else if (Underwater) {
        /* monsters may miss especially on water level where
           bubbles shake the player here and there */
        if (compat)
            pline("%s reaches towards your distorted image.", Monst_name);
        else
            pline("%s is fooled by water reflections and misses!",
                  Monst_name);

    } else
        impossible("%s attacks you without knowing your location?",
                   Monst_name);
}

void
expels(mtmp, mdat, message)
struct monst *mtmp;
struct permonst *mdat; /* if mtmp is polymorphed, mdat != mtmp->data */
boolean message;
{
    if (message) {
        if (is_swallower(mdat)) {
            You("get regurgitated!");
        } else {
            char blast[40];
            struct attack *attk = attacktype_fordmg(mdat, AT_ENGL, AD_ANY);

            blast[0] = '\0';
            if (!attk) {
                impossible("Swallower has no engulfing attack?");
            } else {
                if (is_whirly(mdat)) {
                    switch (attk->adtyp) {
                    case AD_ELEC:
                        Strcpy(blast, " in a shower of sparks");
                        break;
                    case AD_COLD:
                        Strcpy(blast, " in a blast of frost");
                        break;
                    case AD_FIRE:
                        if (mdat == &mons[PM_STEAM_VORTEX])
                            Strcpy(blast, " in a blast of steam");
                        else
                            Strcpy(blast, " in a blast of fire");
                        break;
                    case AD_DISN:
                        Strcpy(blast, " in a haze of antiparticles");
                        break;
                    case AD_BLND:
                    case AD_PHYS:
                        Strcpy(blast, " with a woosh");
                        break;
                    }
                } else {
                    Strcpy(blast, " with a squelch");
                }
                You("get expelled from %s%s!", mon_nam(mtmp), blast);
            }
        }
    }
    unstuck(mtmp); /* ball&chain returned in unstuck() */
    mnexto(mtmp);
    newsym(u.ux, u.uy);
    /* to cover for a case where mtmp is not in a next square */
    if (um_dist(mtmp->mx, mtmp->my, 1))
        pline("Brrooaa...  You land hard at some distance.");
    spoteffects(TRUE);
}

/* select a monster's next attack, possibly substituting for its usual one */
struct attack *
getmattk(magr, mdef, indx, prev_result, alt_attk_buf)
struct monst *magr, *mdef;
int indx, prev_result[];
struct attack *alt_attk_buf;
{
    struct permonst *mptr = magr->data;
    struct attack *mattk = has_erac(magr) ? ERAC(magr)->mattk : mptr->mattk;
    struct attack *attk = &mattk[indx];
    struct obj *weap = (magr == &youmonst) ? uwep : MON_WEP(magr);

    /* honor SEDUCE=0 */
    if (!SYSOPT_SEDUCE) {
        extern const struct attack sa_no[NATTK];

        /* if the first attack is for SSEX damage, all six attacks will be
           substituted (expected succubus/incubus handling); if it isn't
           but another one is, only that other one will be substituted */
        if (mattk[0].adtyp == AD_SSEX) {
            *alt_attk_buf = sa_no[indx];
            attk = alt_attk_buf;
        } else if (attk->adtyp == AD_SSEX) {
            *alt_attk_buf = *attk;
            attk = alt_attk_buf;
            attk->adtyp = AD_DRLI;
        }
    }

    /* prevent a monster with two consecutive disease or hunger attacks
       from hitting with both of them on the same turn; if the first has
       already hit, switch to a stun attack for the second */
    if (indx > 0 && prev_result[indx - 1] > 0
        && (attk->adtyp == AD_DISE || attk->adtyp == AD_PEST
            || attk->adtyp == AD_FAMN)
        && attk->adtyp == mattk[indx - 1].adtyp) {
        *alt_attk_buf = *attk;
        attk = alt_attk_buf;
        attk->adtyp = AD_STUN;

    /* make drain-energy damage be somewhat in proportion to energy */
    } else if (attk->adtyp == AD_DREN && mdef == &youmonst) {
        int ulev = max(u.ulevel, 6);

        *alt_attk_buf = *attk;
        attk = alt_attk_buf;
        /* 3.6.0 used 4d6 but since energy drain came out of max energy
           once current energy was gone, that tended to have a severe
           effect on low energy characters; it's now 2d6 with ajustments */
        if (u.uen <= 5 * ulev && attk->damn > 1) {
            attk->damn -= 1; /* low energy: 2d6 -> 1d6 */
            if (u.uenmax <= 2 * ulev && attk->damd > 3)
                attk->damd -= 3; /* very low energy: 1d6 -> 1d3 */
        } else if (u.uen > 12 * ulev) {
            attk->damn += 1; /* high energy: 2d6 -> 3d6 */
            if (u.uenmax > 20 * ulev)
                attk->damd += 3; /* very high energy: 3d6 -> 3d9 */
            /* note: 3d9 is slightly higher than previous 4d6 */
        }

    } else if (attk->aatyp == AT_ENGL && magr->mspec_used) {
        /* can't re-engulf yet; switch to simpler attack */
        *alt_attk_buf = *attk;
        attk = alt_attk_buf;
        if (attk->adtyp == AD_ACID || attk->adtyp == AD_ELEC
            || attk->adtyp == AD_COLD || attk->adtyp == AD_FIRE
            || attk->adtyp == AD_DISE || attk->adtyp == AD_DISN) {
            attk->aatyp = AT_TUCH;
        } else {
            /* neothelids have a breath attack plus multiple tentacle
               attacks which cause mspec_used to virtually never allow
               the final engulfing attack */
            if (rn2(2) && magr->data == &mons[PM_NEOTHELID]) {
                attk->aatyp = AT_ENGL;
                attk->adtyp = AD_DGST;
            } else {
                attk->aatyp = AT_CLAW; /* attack message will be "<foo> hits" */
                attk->adtyp = AD_PHYS;
            }
        }
        attk->damn = 2; /* not-so weak: 2d8 */
        attk->damd = 8;

    /* barrow wight, Nazgul, erinys have weapon attack for non-physical
       damage; force physical damage if attacker has been cancelled or
       if weapon is sufficiently interesting; a few unique creatures
       have two weapon attacks where one does physical damage and other
       doesn't--avoid forcing physical damage for those */
    } else if (indx == 0 && magr != &youmonst
               && attk->aatyp == AT_WEAP && attk->adtyp != AD_PHYS
               && !(mattk[1].aatyp == AT_WEAP
                    && mattk[1].adtyp == AD_PHYS)
               && (magr->mcan
                   || (weap && ((weap->otyp == CORPSE
                                 && touch_petrifies(&mons[weap->corpsenm]))
                                || weap->oartifact == ART_STORMBRINGER
                                || weap->oartifact == ART_SHADOWBLADE
                                || weap->oartifact == ART_VORPAL_BLADE)))) {
        *alt_attk_buf = *attk;
        attk = alt_attk_buf;
        attk->adtyp = AD_PHYS;
    }
    return attk;
}

/*
 * mattacku: monster attacks you
 *      returns MM_AGR_DIED if monster dies (e.g. "yellow light"),
 *      MM_HIT or MM_MISS otherwise
 *      Note: if you're displaced or invisible the monster might attack the
 *              wrong position...
 *      Assumption: it's attacking you or an empty square; if there's another
 *              monster which it attacks by mistake, the caller had better
 *              take care of it...
 */
int
mattacku(mtmp)
register struct monst *mtmp;
{
    struct attack *mattk, alt_attk;
    int i, j = 0, tmp, sum[NATTK];
    struct permonst *mdat = mtmp->data;
    /*
     * ranged: Is it near you?  Affects your actions.
     * ranged2: Does it think it's near you?  Affects its actions.
     * foundyou: Is it attacking you or your image?
     * youseeit: Can you observe the attack?  It might be attacking your
     *     image around the corner, or invisible, or you might be blind.
     * skipnonmagc: Are further physical attack attempts useless?  (After
     *     a wild miss--usually due to attacking displaced image.  Avoids
     *     excessively verbose miss feedback when monster can do multiple
     *     attacks and would miss the same wrong spot each time.)
     */
    boolean ranged = (distu(mtmp->mx, mtmp->my) > 3);
    boolean range2 = !monnear(mtmp, mtmp->mux, mtmp->muy);
    boolean foundyou = (mtmp->mux == u.ux && mtmp->muy == u.uy);
    boolean youseeit = canseemon(mtmp);
    boolean firstfoundyou, skipnonmagc = FALSE;

    if (!ranged)
        nomul(0);
    if (DEADMONSTER(mtmp)
        || (Underwater && !mon_underwater(mtmp)))
        return MM_MISS;

    /* If swallowed, can only be affected by u.ustuck */
    if (u.uswallow) {
        if (mtmp != u.ustuck)
            return MM_MISS;
        u.ustuck->mux = u.ux;
        u.ustuck->muy = u.uy;
        range2 = 0;
        foundyou = 1;
        if (u.uinvulnerable)
            return MM_MISS; /* stomachs can't hurt you! */
    } else if (u.usteed) {
        if (mtmp == u.usteed)
            /* Your steed won't attack you */
            return MM_MISS;
        /* Orcs like to steal and eat horses and the like */
        if (!rn2(racial_orc(mtmp) ? 2 : 4)
            && distu(mtmp->mx, mtmp->my) <= 2) {
            /* Attack your steed instead */
            i = mattackm(mtmp, u.usteed);
            if ((i & MM_AGR_DIED) != 0)
                return MM_AGR_DIED;
            /* make sure steed is still alive and within range */
            if ((i & MM_DEF_DIED) != 0 || !u.usteed
                || distu(mtmp->mx, mtmp->my) > 2)
                return MM_MISS;
            /* Let your steed retaliate */
            bhitpos.x = mtmp->mx, bhitpos.y = mtmp->my;
            notonhead = FALSE;
            return (mattackm(u.usteed, mtmp) & MM_DEF_DIED) ? MM_AGR_DIED : MM_HIT;
        }
        /* steed will attack on the players behalf without waiting
           for the player or itself to be attacked first if the steed
           is loyal or greater. if the steed has a ranged attack, it
           can utilize it apart from this routine */
        if (u.usteed->mtame >= 15
            && distu(mtmp->mx, mtmp->my) <= 2) {
            /* steed attacks without provocation,
               make sure it's actually alive */
            i = mattackm(u.usteed, mtmp);
            if ((i & MM_AGR_DIED) != 0 || !u.usteed)
                return MM_AGR_DIED;
            /* make sure monster is alive and is
               within range (melee attack) */
            if ((i & MM_DEF_DIED) != 0
                || distu(mtmp->mx, mtmp->my) > 2)
                return MM_MISS;
            /* allow steed to attack */
            bhitpos.x = mtmp->mx, bhitpos.y = mtmp->my;
            notonhead = FALSE;
            return (mattackm(u.usteed, mtmp) & MM_DEF_DIED) ? MM_AGR_DIED : MM_HIT;
        }
    }

    if (u.uundetected && !range2 && foundyou && !u.uswallow) {
        if (!canspotmon(mtmp))
            map_invisible(mtmp->mx, mtmp->my);
        u.uundetected = 0;
        if (is_hider(youmonst.data) && u.umonnum != PM_TRAPPER) {
            /* ceiling hider */
            coord cc; /* maybe we need a unexto() function? */
            struct obj *obj;

            You("fall from the %s!", ceiling(u.ux, u.uy));
            /* take monster off map now so that its location
               is eligible for placing hero; we assume that a
               removed monster remembers its old spot <mx,my> */
            remove_monster(mtmp->mx, mtmp->my);
            if (!enexto(&cc, u.ux, u.uy, youmonst.data)
                /* a fish won't voluntarily swap positions
                   when it's in water and hero is over land */
                || (mtmp->data->mlet == S_EEL
                    && (is_pool(mtmp->mx, mtmp->my)
                        || is_puddle(mtmp->mx, mtmp->my))
                    && !(is_pool(u.ux, u.uy) || is_puddle(u.ux, u.uy)))
                || (mtmp->data == &mons[PM_GIANT_LEECH]
                    && is_sewage(mtmp->mx, mtmp->my)
                    && !is_sewage(u.ux, u.uy))) {
                /* couldn't find any spot for hero; this used to
                   kill off attacker, but now we just give a "miss"
                   message and keep both mtmp and hero at their
                   original positions; hero has become unconcealed
                   so mtmp's next move will be a regular attack */
                place_monster(mtmp, mtmp->mx, mtmp->my); /* put back */
                newsym(u.ux, u.uy); /* u.uundetected was toggled */
                pline("%s draws back as you drop!", Monnam(mtmp));
                return MM_MISS;
            }

            /* put mtmp at hero's spot and move hero to <cc.x,.y> */
            newsym(mtmp->mx, mtmp->my); /* finish removal */
            place_monster(mtmp, u.ux, u.uy);
            if (mtmp->wormno) {
                worm_move(mtmp);
                /* tail hasn't grown, so if it now occupies <cc.x,.y>
                   then one of its original spots must be free */
                if (m_at(cc.x, cc.y))
                    (void) enexto(&cc, u.ux, u.uy, youmonst.data);
            }
            teleds(cc.x, cc.y, TELEDS_ALLOW_DRAG); /* move hero */
            set_apparxy(mtmp);
            newsym(u.ux, u.uy);

            if (youmonst.data->mlet != S_PIERCER)
                return MM_MISS; /* lurkers don't attack */

            obj = which_armor(mtmp, WORN_HELMET);
            if (obj && is_metallic(obj)) {
                Your("blow glances off %s %s.", s_suffix(mon_nam(mtmp)),
                     helm_simple_name(obj));
            } else {
                if (3 + find_mac(mtmp) <= rnd(20)) {
                    pline("%s is hit by a falling piercer (you)!",
                          Monnam(mtmp));
                    if (damage_mon(mtmp, d(3, 6), AD_PHYS, TRUE))
                        killed(mtmp);
                } else
                    pline("%s is almost hit by a falling piercer (you)!",
                          Monnam(mtmp));
            }
        } else {
            /* surface hider */
            if (!youseeit) {
                pline("It tries to move where you are hiding.");
            } else {
                /* Ugly kludge for eggs.  The message is phrased so as
                 * to be directed at the monster, not the player,
                 * which makes "laid by you" wrong.  For the
                 * parallelism to work, we can't rephrase it, so we
                 * zap the "laid by you" momentarily instead.
                 */
                struct obj *obj = level.objects[u.ux][u.uy];

                if (obj || concealed_spot(u.ux, u.uy)
                    || u.umonnum == PM_TRAPPER
                    || (u.umonnum == PM_GIANT_LEECH
                        && is_sewage(u.ux, u.uy))
                    || (youmonst.data->mlet == S_EEL
                        && (is_pool(u.ux, u.uy) || is_puddle(u.ux, u.uy)))) {
                    int save_spe = 0; /* suppress warning */

                    if (obj) {
                        save_spe = obj->spe;
                        if (obj->otyp == EGG)
                            obj->spe = 0;
                    }
                    /* note that m_monnam() overrides hallucination, which is
                       what we want when message is from mtmp's perspective */
                    if (youmonst.data->mlet == S_EEL
                        || u.umonnum == PM_TRAPPER
                        || u.umonnum == PM_GIANT_LEECH || !obj)
                        pline(
                             "Wait, %s!  There's a hidden %s named %s there!",
                              m_monnam(mtmp), youmonst.data->mname, plname);
                    else
                        pline(
                          "Wait, %s!  There's a %s named %s hiding under %s!",
                              m_monnam(mtmp), youmonst.data->mname, plname,
                              doname(level.objects[u.ux][u.uy]));
                    if (obj)
                        obj->spe = save_spe;
                } else
                    impossible("hiding under nothing?");
            }
            newsym(u.ux, u.uy);
        }
        return MM_MISS;
    }

    /* hero might be a mimic, concealed via #monster */
    if (youmonst.data->mlet == S_MIMIC && U_AP_TYPE && !range2
        && foundyou && !u.uswallow) {
        boolean sticky = sticks(youmonst.data);

        if (!canspotmon(mtmp))
            map_invisible(mtmp->mx, mtmp->my);
        if (sticky && !youseeit)
            pline("It gets stuck on you.");
        else /* see note about m_monnam() above */
            pline("Wait, %s!  That's a %s named %s!", m_monnam(mtmp),
                  youmonst.data->mname, plname);
        if (sticky) {
            u.ustuck = mtmp;
            /* normally this happens later in mhitu, but we return before that */
            if (mtmp->mundetected
                && (hides_under(mdat) || mdat->mlet == S_EEL
                    || mdat == &mons[PM_GIANT_LEECH])) {
                mtmp->mundetected = 0;
                if (!(Blind ? Blind_telepat : Unblind_telepat)) {
                    struct obj *obj = level.objects[mtmp->mx][mtmp->my];
                    const char *what = something;
                    int concealment = concealed_spot(mtmp->mx, mtmp->my);

                    if (concealment == 2) { /* object cover */
                        if (Blind && !obj->dknown)
                            what = something;
                        else if (is_pool(mtmp->mx, mtmp->my) && !Underwater)
                            what = "the water";
                        else if (is_puddle(mtmp->mx, mtmp->my))
                            what = "the shallow water";
                        else if (is_sewage(mtmp->mx, mtmp->my))
                            what = "the raw sewage";
                        else
                            what = doname(obj);
                    } else if (concealment == 1) { /* terrain cover, no objects */
                        what = explain_terrain(mtmp->mx, mtmp->my);
                    }
                    pline("%s was hidden under %s%s!", Amonnam(mtmp),
                          obj ? "" : "the ", what);
                    /* unhide attacking monster if hidden */
                    maybe_unhide_at(mtmp->mx, mtmp->my);
                    newsym(mtmp->mx, mtmp->my);
                }
            }
        }
        youmonst.m_ap_type = M_AP_NOTHING;
        youmonst.mappearance = 0;
        newsym(u.ux, u.uy);
        return MM_HIT;
    }

    /* non-mimic hero might be mimicking an object after eating m corpse */
    if (U_AP_TYPE == M_AP_OBJECT && !range2 && foundyou && !u.uswallow) {
        if (!canspotmon(mtmp))
            map_invisible(mtmp->mx, mtmp->my);
        if (!youseeit)
            pline("%s %s!", Something, (likes_gold(mtmp->data)
                                        && youmonst.mappearance == GOLD_PIECE)
                                           ? "tries to pick you up"
                                           : "disturbs you");
        else /* see note about m_monnam() above */
            pline("Wait, %s!  That %s is really %s named %s!", m_monnam(mtmp),
                  mimic_obj_name(&youmonst), an(mons[u.umonnum].mname),
                  plname);
        if (multi < 0) { /* this should always be the case */
            char buf[BUFSZ];

            Sprintf(buf, "You appear to be %s again.",
                    Upolyd ? (const char *) an(youmonst.data->mname)
                           : (const char *) "yourself");
            unmul(buf); /* immediately stop mimicking */
        }
        return MM_HIT;
    }

    /*  Work out the armor class differential   */
    tmp = AC_VALUE(u.uac) + 10; /* tmp ~= 0 - 20 */
    tmp += mtmp->m_lev;
    if (multi < 0)
        tmp += 4;
    if ((Invis && !mon_prop(mtmp, SEE_INVIS)) || !mtmp->mcansee)
        tmp -= 2;
    if (mtmp->mtrapped || mtmp->mentangled)
        tmp -= 2;
    if ((has_erac(mtmp) && (ERAC(mtmp)->mflags3 & M3_ACCURATE))
        || is_accurate(mdat)) /* M3_ACCURATE monsters get a to-hit bonus */
        tmp += Hidinshell ? 0 : 5;
    if (Hidinshell) /* enshelled tortles are much harder to hit */
        tmp -= 12;
    /* Drow are affected by being in both the light or the dark */
    if (racial_drow(mtmp)) {
        if (!(levl[mtmp->mx][mtmp->my].lit
              || (viz_array[mtmp->my][mtmp->mx] & TEMP_LIT))
            || (viz_array[mtmp->my][mtmp->mx] & TEMP_DARK)) {
            /* in darkness */
            tmp += (mtmp->m_lev / 3) + 2;
        } else {
            /* in the light */
            tmp -= 3;
        }
    }
    if (tmp <= 0)
        tmp = 1;

    /* find rings of increase accuracy */
    {
        struct obj *o;
	for (o = mtmp->minvent; o; o = o->nobj)
	     if (o->owornmask && o->otyp == RIN_INCREASE_ACCURACY)
	         tmp += o->spe;
    }

    /* make eels and leeches visible the moment they hit/miss us */
    if ((mdat->mlet == S_EEL || mdat == &mons[PM_GIANT_LEECH])
        && mtmp->minvis && cansee(mtmp->mx, mtmp->my)) {
        mtmp->minvis = 0;
        newsym(mtmp->mx, mtmp->my);
    }

    /*  Special demon handling code */
    if ((mtmp->cham == NON_PM) && is_demon(mdat) && !range2
        && mtmp->data != &mons[PM_BALROG] && mtmp->data != &mons[PM_SUCCUBUS]
        && mtmp->data != &mons[PM_INCUBUS] && mtmp->data != &mons[PM_DEMON])
        if (!mtmp->mcan && !rn2(13))
            (void) msummon(mtmp);

    /*  Special lycanthrope handling code */
    if ((mtmp->cham == NON_PM) && is_were(mdat) && !range2) {
        if (is_human(mdat)) {
            if (!rn2(5 - (night() * 2)) && !mtmp->mcan)
                new_were(mtmp);
        } else if (!rn2(30) && !mtmp->mcan)
            new_were(mtmp);
        mdat = mtmp->data;

        if (!rn2(10) && !mtmp->mcan) {
            int numseen, numhelp;
            char buf[BUFSZ], genericwere[BUFSZ];

            Strcpy(genericwere, "creature");
            numhelp = were_summon(mdat, FALSE, &numseen, genericwere);
            if (youseeit) {
                pline("%s summons help!", Monnam(mtmp));
                if (numhelp > 0) {
                    if (numseen == 0)
                        You_feel("hemmed in.");
                } else
                    pline("But none comes.");
            } else {
                const char *from_nowhere;

                if (!Deaf) {
                    pline("%s %s!", Something, makeplural(growl_sound(mtmp)));
                    from_nowhere = "";
                } else
                    from_nowhere = " from nowhere";
                if (numhelp > 0) {
                    if (numseen < 1)
                        You_feel("hemmed in.");
                    else {
                        if (numseen == 1)
                            Sprintf(buf, "%s appears", an(genericwere));
                        else
                            Sprintf(buf, "%s appear",
                                    makeplural(genericwere));
                        pline("%s%s!", upstart(buf), from_nowhere);
                    }
                } /* else no help came; but you didn't know it tried */
            }
        }
    }

    if (u.uinvulnerable) {
        /* monsters won't attack you */
        if (mtmp == u.ustuck) {
            pline("%s loosens its grip slightly.", Monnam(mtmp));
        } else if (!range2) {
            if (youseeit || sensemon(mtmp))
                pline("%s starts to attack you, but pulls back.",
                      Monnam(mtmp));
            else
                You_feel("%s move nearby.", something);
        }
        return MM_MISS;
    }

    /* Unlike defensive stuff, don't let them use item _and_ attack. */
    if (find_offensive(mtmp)) {
        int foo = use_offensive(mtmp);

        if (foo != 0)
            return (foo == 1) ? MM_HIT : MM_MISS;
    }

    firstfoundyou = foundyou;

    for (i = 0; i < NATTK; i++) {
        sum[i] = 0;
        /* ranged &c must be updated in case the attacker has been knocked
           back by Ashmar or the Armor of Retribution */
        ranged = (distu(mtmp->mx, mtmp->my) > 3);
        range2 = !monnear(mtmp, mtmp->mux, mtmp->muy);
        foundyou = (mtmp->mux == u.ux && mtmp->muy == u.uy);
        youseeit = canseemon(mtmp);
        if (i > 0 && firstfoundyou /* previous attack might have moved hero */
            && !foundyou)
            continue; /* fill in sum[] with 'miss' but skip other actions */
        mon_currwep = (struct obj *) 0;
        mattk = getmattk(mtmp, &youmonst, i, sum, &alt_attk);
        if ((u.uswallow && mattk->aatyp != AT_ENGL)
            || (skipnonmagc && mattk->aatyp != AT_MAGC))
            continue;

        switch (mattk->aatyp) {
        case AT_CLAW: /* "hand to hand" attacks */
        case AT_KICK:
        case AT_BITE:
        case AT_STNG:
        case AT_TUCH:
        case AT_BUTT:
        case AT_TENT:
            if (!range2 && (!MON_WEP(mtmp) || mtmp->mconf || Conflict
                            || !touch_petrifies(youmonst.data))) {
                if (foundyou) {
                    /* if our hero is sized tiny/small and unencumbered,
                       they have a decent chance of evading a zombie's
                       bite attack */
                    if (racial_zombie(mtmp) && mattk->aatyp == AT_BITE
                        && (youmonst.data)->msize <= MZ_SMALL
                        && is_animal(youmonst.data)
                        && (near_capacity() == UNENCUMBERED)
                        && !(Confusion || Stunned || Punished || multi < 0
                             || Wounded_legs || Stoned || Fumbling) && rn2(3)) {
                        You("nimbly %s %s bite!",
                            rn2(2) ? "dodge" : "evade", s_suffix(mon_nam(mtmp)));
                        return MM_MISS; /* attack stops */
                    }
                    if (mdat->msize <= MZ_LARGE && mattk->aatyp == AT_BITE
                        && Hidinshell) {
                        Your("protective shell blocks %s bite!",
                             s_suffix(mon_nam(mtmp)));
                    }
                    if (is_illithid(mdat) && mattk->aatyp == AT_TENT
                        && Hidinshell) {
                        Your("protective shell blocks %s tentacle attack!",
                             s_suffix(mon_nam(mtmp)));
                    }
                    if (mattk->aatyp == AT_STNG && Hidinshell) {
                        pline("%s stinger glances off of your protective shell!",
                              s_suffix(Monnam(mtmp)));
                    }
                    if (tmp > (j = rnd(20 + i))) {
                        if (mattk->aatyp != AT_KICK
                            || !thick_skinned(youmonst.data)
                            || !Barkskin || !Stoneskin)
                            sum[i] = hitmu(mtmp, mattk);
                        if (mtmp->data == &mons[PM_MEDUSA]
                            && mattk->aatyp == AT_BITE && !mtmp->mcan) {
                            if (!Stoned && !Stone_resistance
                                && !(poly_when_stoned(youmonst.data)
                                && (polymon(PM_STONE_GOLEM)
                                    || polymon(PM_PETRIFIED_ENT)))) {
                                int kformat = KILLED_BY_AN;
                                const char *kname = mtmp->data->mname;

                                if (mtmp->data->geno & G_UNIQ) {
                                    if (!type_is_pname(mtmp->data))
                                        kname = the(kname);
                                    kformat = KILLED_BY;
                                }
                                make_stoned(5L, (char *) 0, kformat, kname);
                                return MM_HIT;
                            }
                        }
                    } else {
                        missmu(mtmp, tmp, j, mattk);
                        if (uarms && !rn2(3))
                            use_skill(P_SHIELD, 1);
                        /* if the attacker dies from a glancing blow off
                           of a piece of the player's armor, and said armor
                           is made of a material the attacker hates, this
                           check is necessary to prevent a dmonsfree error
                           if the attacker has multiple attacks and they
                           died before their attack chain completed */
                        if (DEADMONSTER(mtmp))
                            return MM_AGR_DIED;
                    }
                } else {
                    /* note: wildmiss only expects cases where the hero is
                       displaced, invisible, or underwater. if foundyou is
                       false for other reasons, it must be dealt with above */
                    wildmiss(mtmp, mattk);
                    /* skip any remaining non-spell attacks */
                    skipnonmagc = TRUE;
                }
            }
            break;

        case AT_HUGS: /* automatic if prev two attacks succeed */
            /* unless a special case is made, any AT_HUGS attack
               will not trigger if it is 1st or 2nd in the chain
               of attacks */
            if ((!ranged && ((i >= 2 && sum[i - 1] && sum[i - 2])
                             || is_stationary(mdat)
                             || mtmp->data == &mons[PM_TREE_BLIGHT]))
                || mtmp == u.ustuck)
                sum[i] = hitmu(mtmp, mattk);
            break;

        case AT_GAZE: /* can affect you either ranged or not */
            /* Medusa gaze already operated through m_respond in
               dochug(); don't gaze more than once per round. */
            if (mdat != &mons[PM_MEDUSA])
                sum[i] = gazemu(mtmp, mattk);
            break;

        case AT_EXPL: /* automatic hit if next to, and aimed at you */
            if (!range2)
                sum[i] = explmu(mtmp, mattk, foundyou);
            break;

        case AT_ENGL:
            if (!range2) {
                if (foundyou) {
                    if (u.uswallow
                        || (!mtmp->mspec_used && tmp > (j = rnd(20 + i)))) {
                        /* force swallowing monster to be displayed
                           even when hero is moving away */
                        flush_screen(1);
                        sum[i] = gulpmu(mtmp, mattk);
                    } else {
                        missmu(mtmp, tmp, j, mattk);
                        if (uarms && !rn2(3))
                            use_skill(P_SHIELD, 1);
                    }
                } else if (is_swallower(mtmp->data)) {
                    pline("%s gulps some air!", Monnam(mtmp));
                } else {
                    if (youseeit)
                        pline("%s lunges forward and recoils!", Monnam(mtmp));
                    else
                        You_hear("a %s nearby.",
                                 is_whirly(mtmp->data) ? "rushing noise"
                                                       : "splat");
                }
            }
            break;
        case AT_BREA:
            if (range2)
                sum[i] = breamu(mtmp, mattk);
            /* Note: breamu takes care of displacement */
            break;
        case AT_SPIT:
            if (range2)
                sum[i] = spitmu(mtmp, mattk);
            /* Note: spitmu takes care of displacement */
            break;
        case AT_WEAP:
            if (range2) {
                if (!Is_rogue_level(&u.uz))
                    thrwmu(mtmp);
            } else {
                int hittmp = 0;

                /* Rare but not impossible.  Normally the monster
                 * wields when 2 spaces away, but it can be
                 * teleported or whatever....
                 */
                if ((mtmp->weapon_check == NEED_WEAPON || !MON_WEP(mtmp)
                    || (is_launcher(MON_WEP(mtmp)) && !(MON_WEP(mtmp))->cursed))) {
                    mtmp->weapon_check = NEED_HTH_WEAPON;
                    /* mon_wield_item resets weapon_check as appropriate */
                    if (mon_wield_item(mtmp) != 0)
                        break;
                }
                if (!MON_WEP(mtmp) || is_launcher(MON_WEP(mtmp))) {
                    /* implies we could not find a HTH weapon, try point blank
                     * ranged attack */
                    if (thrwmu(mtmp)) {
                        break;
                    }
                }
                if (foundyou) {
                    mon_currwep = MON_WEP(mtmp);
                    if (mon_currwep) {
                        hittmp = hitval(mon_currwep, &youmonst);
                        tmp += hittmp;
                        mswings(mtmp, mon_currwep);
                    }
                    if (tmp > (j = dieroll = rnd(20 + i))) {
                        sum[i] = hitmu(mtmp, mattk);
                    } else {
                        missmu(mtmp, tmp, j, mattk);
                        if (uarms && !rn2(3))
                            use_skill(P_SHIELD, 1);
                    }
                    /* KMH -- Don't accumulate to-hit bonuses */
                    if (mon_currwep)
                        tmp -= hittmp;
                } else {
                    wildmiss(mtmp, mattk);
                    /* skip any remaining non-spell attacks */
                    skipnonmagc = TRUE;
                }
            }
            break;
        case AT_MAGC:
            if (range2)
                sum[i] = buzzmu(mtmp, mattk);
            else
                sum[i] = castmu(mtmp, mattk, TRUE, foundyou);
            break;
	case AT_SCRE:
            if (ranged || !rn2(5)) /* sometimes right next to our hero */
                sum[i] = screamu(mtmp, mattk);
	    break;
        default: /* no attack */
            break;
        }
        if (context.botl)
            bot();
        /* give player a chance of waking up before dying -kaa */
        if (sum[i] == 1) { /* successful attack */
            if (u.usleep && u.usleep < monstermoves && !rn2(10)) {
                multi = -1;
                nomovemsg = "The combat suddenly awakens you.";
            }
        }
        if (sum[i] == 2)
            return MM_AGR_DIED; /* attacker dead */
        if (sum[i] == 3)
            break; /* attacker teleported, no more attacks */
        /* sum[i] == 0: unsuccessful attack */
    }
    return MM_HIT; /* monster did attack */
}

boolean
diseasemu(mtmp)
struct monst *mtmp;
{
    if (Sick_resistance) {
        You_feel("a slight illness.");
        return FALSE;
    } else {
        make_sick(Sick ? Sick / 3L + 1L
                       : (long) rn1(ACURR(A_CON), 20),
                  mtmp->data->mname, TRUE,
                  racial_zombie(mtmp) ? SICK_ZOMBIE : SICK_NONVOMITABLE);
        return TRUE;
    }
}

/* check whether slippery clothing protects from hug or wrap attack */
STATIC_OVL boolean
u_slip_free(mtmp, mattk)
struct monst *mtmp;
struct attack *mattk;
{
    struct obj *obj = (uarmc ? uarmc : uarm);

    if (!obj)
        obj = uarmu;
    if (mattk->adtyp == AD_DRIN)
        obj = uarmh;
    if (mtmp->data == &mons[PM_MIND_FLAYER_LARVA]
        && mattk->aatyp == AT_TENT && mattk->adtyp == AD_WRAP)
        obj = uarmh;

    /* if your cloak/armor is greased, monster slips off; this
       protection might fail (33% chance) when the armor is cursed */
    if (obj && (obj->greased || obj->otyp == OILSKIN_CLOAK
        || (obj->oprops & ITEM_OILSKIN))
        && (!obj->cursed || rn2(3))) {
        pline("%s %s your %s %s!", Monnam(mtmp),
              (mattk->adtyp == AD_WRAP
               && mtmp->data != &mons[PM_SALAMANDER])
                   ? "slips off of"
                   : (mattk->adtyp == AD_DRIN && racial_zombie(mtmp))
                       ? "bites you, but slips off of"
                       : "grabs you, but cannot hold onto",
              obj->greased ? "greased" : "slippery",
              /* avoid "slippery slippery cloak"
                 for undiscovered oilskin cloak */
              (obj->greased || objects[obj->otyp].oc_name_known)
                  ? xname(obj)
                  : simpleonames(obj));

        if (obj->greased && !rn2(2)) {
            pline_The("grease wears off.");
            obj->greased = 0;
            update_inventory();
        }
        return TRUE;
    }
    return FALSE;
}

/* armor that sufficiently covers the body might be able to block magic */
int
magic_negation(mon)
struct monst *mon;
{
    struct obj *o;
    long wearmask;
    int armpro, mc = 0;
    boolean is_you = (mon == &youmonst),
            via_amul = FALSE,
            gotprot = is_you ? (EProtection != 0L)
                             /* high priests have innate protection */
                             : (mon->data == &mons[PM_HIGH_PRIEST]);

    for (o = is_you ? invent : mon->minvent; o; o = o->nobj) {
        /* a_can field is only applicable for armor (which must be worn) */
        if ((o->owornmask & W_ARMOR) != 0L) {
            armpro = objects[o->otyp].a_can;
            /* mithril armor grants a level of MC due to its magical origins,
             * plus extra for elves*/
            if (((o->owornmask & (W_ARM | W_ARMC)) != 0)
                && o->material == MITHRIL) {
                armpro = max(armpro, ((Race_if(PM_ELF) && !Upolyd) ? 3 : 2));
            }
            /* bone or stone armor grants MC to Orcs */
            else if (((o->owornmask & (W_ARM | W_ARMC)) != 0)
                && (o->material == BONE || o->material == MINERAL)
                && Race_if(PM_ORC) && !Upolyd && armpro < 3) {
                armpro = 3;
            }
            if (armpro > mc)
                mc = armpro;
        } else if ((o->owornmask & W_AMUL) != 0L) {
            via_amul = (o->otyp == AMULET_OF_GUARDING);
        }
        /* if we've already confirmed Protection, skip additional checks */
        if (is_you || gotprot)
            continue;

        /* omit W_SWAPWEP+W_QUIVER; W_ART+W_ARTI handled by protects() */
        wearmask = W_ARMOR | W_ACCESSORY;
        if (o->oclass == WEAPON_CLASS || is_weptool(o))
            wearmask |= W_WEP;
        if (protects(o, ((o->owornmask & wearmask) != 0L) ? TRUE : FALSE))
            gotprot = TRUE;
    }

    if (gotprot) {
        /* extrinsic Protection increases mc by 1 (2 for amulet of guarding);
           multiple sources don't provide multiple increments */
        mc += via_amul ? 2 : 1;
        if (mc > 3)
            mc = 3;
    } else if (mc < 1) {
        /* intrinsic Protection is weaker (play balance; obtaining divine
           protection is too easy); it confers minimum mc 1 instead of 0 */
        if ((is_you && ((HProtection && u.ublessed > 0) || u.uspellprot))
            /* aligned priests and angels have innate intrinsic Protection */
            || (mon->data == &mons[PM_ALIGNED_PRIEST] || is_minion(mon->data)))
            mc = 1;
    }
    return mc;
}

/*
 * hitmu: monster hits you
 *        returns 2 if monster dies (e.g. "yellow light"), 1 otherwise
 *        3 if the monster lives but teleported/paralyzed, so it can't keep
 *             attacking you
 */
STATIC_OVL int
hitmu(mtmp, mattk)
register struct monst *mtmp;
register struct attack *mattk;
{
    struct permonst *mdat = mtmp->data;
    int uncancelled, ptmp;
    int dmg, armpro, permdmg, tmphp;
    char buf[BUFSZ];
    struct permonst *olduasmon = youmonst.data;
    int res;
    long armask = attack_contact_slots(mtmp, mattk->aatyp);
    struct obj* hated_obj;
    boolean lightobj = FALSE, ispoisoned = FALSE, istainted = FALSE;
    boolean vorpal_wield = ((uwep && uwep->oartifact == ART_VORPAL_BLADE)
                            || (u.twoweap && uswapwep->oartifact == ART_VORPAL_BLADE));

    if (!canspotmon(mtmp) && mdat != &mons[PM_GHOST]) {
        /* Ghosts have an exception because if the hero can't spot it, their
         * attack does absolutely nothing, and there's no indication of a
         * monster being around. */
        map_invisible(mtmp->mx, mtmp->my);
    }

    /* Awaken nearby monsters */
    if (!(is_silent(mdat) && multi < 0) && rn2(10)) {
        wake_nearto(u.ux, u.uy, combat_noise(mtmp->data));
    }

    /*  If the monster is undetected & hits you, you should know where
     *  the attack came from.
     */
    if (mtmp->mundetected && (hides_under(mdat) || mdat->mlet == S_EEL
                              || mdat == &mons[PM_GIANT_LEECH])) {
        mtmp->mundetected = 0;
        if (!(Blind ? Blind_telepat : Unblind_telepat)) {
            struct obj *obj = level.objects[mtmp->mx][mtmp->my];
            const char *what = something;
            int concealment = concealed_spot(mtmp->mx, mtmp->my);

            if (concealment == 2) { /* object cover */
                if (Blind && !obj->dknown)
                    what = something;
                else if (is_pool(mtmp->mx, mtmp->my) && !Underwater)
                    what = "the water";
                else if (is_puddle(mtmp->mx, mtmp->my))
                    what = "the shallow water";
                else if (is_sewage(mtmp->mx, mtmp->my))
                    what = "the raw sewage";
                else
                    what = doname(obj);
            } else if (concealment == 1) { /* terrain cover, no objects */
                what = explain_terrain(mtmp->mx, mtmp->my);
            }
            pline("%s was hidden under %s%s!", Amonnam(mtmp),
                  obj ? "" : "the ", what);
            newsym(mtmp->mx, mtmp->my);
        }
    }

    /*  First determine the base damage done */
    dmg = d((int) mattk->damn, (int) mattk->damd);
    if ((is_undead(mdat) || is_vampshifter(mtmp)) && midnight())
        dmg += d((int) mattk->damn, (int) mattk->damd); /* extra damage */

    /* find rings of increase damage */
    {
        struct obj *o, *nextobj;

        for (o = mtmp->minvent; o; o = nextobj) {
            nextobj = o->nobj;
            if (o->owornmask && o->otyp == RIN_INCREASE_DAMAGE)
                dmg += o->spe;
        }
    }

    /* elementals on their home plane hit very hard */
    if (is_home_elemental(mdat)) {
        /* air elementals hit hard enough already */
	if (mtmp->mnum != PM_AIR_ELEMENTAL)
            dmg += d((int) mattk->damn, (int) mattk->damd);
    }

    /*  Next a cancellation factor.
     *  Use uncancelled when cancellation factor takes into account certain
     *  armor's special magic protection.  Otherwise just use !mtmp->mcan.
     */
    armpro = magic_negation(&youmonst);
    uncancelled = !mtmp->mcan && (rn2(10) >= 3 * armpro);

    permdmg = 0;
    /*  Now, adjust damages via resistances or specific attacks */
    switch (mattk->adtyp) {
    case AD_CLOB:
    case AD_PHYS:
        if ((mattk->aatyp == AT_HUGS
            || (mattk->aatyp == AT_TENT
                && mtmp->data == &mons[PM_NEOTHELID]))
            && !sticks(youmonst.data)) {
            if (!u.ustuck && rn2(2)) {
                if (u_slip_free(mtmp, mattk)) {
                    dmg = 0;
                } else {
                    u.ustuck = mtmp;
                    if (mtmp->mundetected) {
                        mtmp->mundetected = 0;
                        newsym(mtmp->mx, mtmp->my);
                    }
                    if (has_trunk(mtmp->data))
                        pline("%s grasps you with its trunk!", Monnam(mtmp));
                    else if (mtmp->data == &mons[PM_NEOTHELID])
                        pline("%s ensnares you with its tentacles!", Monnam(mtmp));
                    else
                        pline("%s grabs you!", Monnam(mtmp));
                }
            } else if (u.ustuck == mtmp) {
                if (Hidinshell) {
                    /* monster still has you in its grasp, but is unable
                       to cause any damage */
                    Your("protective shell prevents you from being crushed!");
                    dmg = 0;
                } else {
                    exercise(A_STR, FALSE);
                    You("are being %s.", (mtmp->data == &mons[PM_ROPE_GOLEM])
                                             ? "choked"
                                             : "crushed");
                }
            } else {
                hitmsg(mtmp, mattk);
            }
        } else { /* hand to hand weapon */
            struct obj *otmp = mon_currwep;

            if (mtmp->mberserk && !rn2(3))
                dmg += d((int) mattk->damn, (int) mattk->damd);

            if (mattk->aatyp == AT_WEAP && otmp) {
                struct obj *marmg;
                int tmp;
                int wepmaterial = otmp->material;

                if (otmp->otyp == CORPSE
                    && touch_petrifies(&mons[otmp->corpsenm])) {
                    dmg = 1;
                    pline("%s hits you with the %s corpse.", Monnam(mtmp),
                          mons[otmp->corpsenm].mname);
                    if (!Stoned)
                        goto do_stone;
                }

                if (artifact_light(otmp) && otmp->lamplit
                    && (hates_light(youmonst.data)
                        || maybe_polyd(is_drow(youmonst.data),
                                               Race_if(PM_DROW))))
                    lightobj = TRUE;

                if (lightobj) {
                    if (canspotmon(mtmp)) {
                        char *artiname = s_suffix(bare_artifactname(otmp));
                        *artiname = highc(*artiname);
                        pline("%s radiance penetrates deep into your %s!",
                              artiname,
                              (!(noncorporeal(youmonst.data)
                                 || amorphous(youmonst.data))) ? "flesh" : "form");
                    } else if (!Blind) {
                        pline("The light sears you!");
                    } else {
                        You("are seared!");
                    }
                }

                dmg += dmgval(otmp, &youmonst);
                if ((marmg = which_armor(mtmp, W_ARMG)) != 0
                    && marmg->otyp == GAUNTLETS_OF_POWER)
                    dmg += rn1(4, 3); /* 3..6 */
                if (dmg <= 0)
                    dmg = 1;
                if (!((((otmp->oclass == WEAPON_CLASS
                         || is_weptool(otmp) || is_bullet(otmp))
                        && otmp->oprops) || otmp->oartifact)
                      && artifact_hit(mtmp, &youmonst, otmp, &dmg, dieroll)))
                    hitmsg(mtmp, mattk);

                if (otmp->opoisoned && is_poisonable(otmp))
                    ispoisoned = TRUE;

                if (otmp->otainted && is_poisonable(otmp))
                    istainted = TRUE;

                if (!dmg)
                    break;
                if (Hate_material(wepmaterial)) {
                    /* dmgval() already added extra damage */
                    searmsg(mtmp, &youmonst, otmp, FALSE);
                    exercise(A_CON, FALSE);
                }
                /* monster attacking with a poisoned weapon */
                if (ispoisoned) {
                    int nopoison;

                    Sprintf(buf, "%s %s", s_suffix(Monnam(mtmp)),
                            mpoisons_subj(mtmp, mattk));
                    poisoned(buf, A_STR, mdat->mname, 30, FALSE);

                    nopoison = (10 - otmp->owt / 10);
                    if (nopoison < 2)
                        nopoison = 2;
                    if (otmp && !rn2(nopoison)) {
                        otmp->opoisoned = FALSE;
                        if (canseemon(mtmp))
                            pline("%s %s is no longer poisoned.",
                                  s_suffix(Monnam(mtmp)), xname(otmp));
                    }
                }
                /* monster attacking with a tainted (drow-poisoned) weapon */
                if (istainted) {
                    int notaint;

                    Sprintf(buf, "%s %s", s_suffix(Monnam(mtmp)),
                            mpoisons_subj(mtmp, mattk));
                    pline("%s attack was tainted!", s_suffix(Monnam(mtmp)));

                    if (how_resistant(SLEEP_RES) == 100) {
                        monstseesu(M_SEEN_SLEEP);
                        pline_The("drow poison doesn't seem to affect you.");
                    } else {
                        if (!rn2(3)) {
                            losehp(resist_reduce(rnd(2), POISON_RES),
                                   "tainted weapon", KILLED_BY_AN);
                        } else {
                            if (u.usleep) { /* don't let sleep effect stack */
                                losehp(resist_reduce(rnd(2), POISON_RES),
                                       "tainted weapon", KILLED_BY_AN);
                            } else {
                                You("lose consciousness.");
                                losehp(resist_reduce(rnd(2), POISON_RES),
                                       "tainted weapon", KILLED_BY_AN);
                                fall_asleep(-resist_reduce(rn2(3) + 2, SLEEP_RES), TRUE);
                            }
                        }
                    }

                    notaint = ((is_drow_weapon(otmp) ? 20 : 5) - otmp->owt / 10);
                    if (notaint < 2)
                        notaint = 2;
                    if (otmp && !rn2(notaint)) {
                        otmp->otainted = FALSE;
                        if (canseemon(mtmp))
                            pline("%s %s is no longer tainted.",
                                  s_suffix(Monnam(mtmp)), xname(otmp));
                    }
                }

                /* glass breakage from the attack */
                break_glass_obj(some_armor(&youmonst));
                if (break_glass_obj(MON_WEP(mtmp))) {
                    otmp = NULL;
                    mon_currwep = NULL;
                }

                /* this redundancy necessary because you have
                   to take the damage _before_ being cloned;
                   need to have at least 2 hp left to split */
                tmp = dmg;
                if (u.uac < 0)
                    tmp -= rnd(-u.uac);
                if (tmp < 1)
                    tmp = 1;
                if (u.mh - tmp > 1
                    && (wepmaterial == IRON || wepmaterial == STEEL)
                        /* relevant 'metal' objects are scalpel and tsurugi */
                    && (u.umonnum == PM_BLACK_PUDDING
                        || u.umonnum == PM_BROWN_PUDDING)) {
                    if (tmp > 1)
                        exercise(A_STR, FALSE);
                    /* inflict damage now; we know it can't be fatal */
                    u.mh -= tmp;
                    context.botl = 1;
                    dmg = 0; /* don't inflict more damage below */
                    if (cloneu())
                        You("divide as %s hits you!", mon_nam(mtmp));
                }
                rustm(&youmonst, otmp); /* safe if otmp is NULL */
            } else if (mattk->aatyp != AT_TUCH || dmg != 0
                       || mtmp != u.ustuck) {
                hitmsg(mtmp, mattk);
            }
            if (mattk->adtyp == AD_CLOB && dmg != 0
                && !wielding_artifact(ART_GIANTSLAYER)
                && !wielding_artifact(ART_HARBINGER)
                && !(uarms && uarms->oartifact == ART_ASHMAR)
                && !(uarm && uarm->oartifact == ART_ARMOR_OF_RETRIBUTION)
                && (youmonst.data)->msize < MZ_HUGE
                && !unsolid(youmonst.data) && !u.ustuck && !rn2(6)) {
                pline("%s knocks you %s with a %s %s!", Monnam(mtmp),
                      u.usteed ? "out of your saddle" : "back",
                      rn2(2) ? "forceful" : "powerful", rn2(2) ? "blow" : "strike");
                hurtle(u.ux - mtmp->mx, u.uy - mtmp->my, rnd(2), FALSE);
                /* Update monster's knowledge of your position */
                mtmp->mux = u.ux;
                mtmp->muy = u.uy;
                if (!rn2(4))
                    make_stunned((HStun & TIMEOUT) + (long) rnd(2) + 1, TRUE);
            }
            if (mtmp->data == &mons[PM_WATER_ELEMENTAL]
                || mtmp->data == &mons[PM_BABY_SEA_DRAGON]
                || mtmp->data == &mons[PM_SEA_DRAGON])
                goto do_rust;
        }
        break;
    case AD_DISE:
        hitmsg(mtmp, mattk);
        if (!diseasemu(mtmp))
            dmg = 0;
        break;
    case AD_FIRE:
        hitmsg(mtmp, mattk);
        if (uncancelled) {
            pline("You're %s!",
                  on_fire(&youmonst, mattk->aatyp == AT_HUGS ? ON_FIRE_HUG
                                                             : ON_FIRE));
            if (completelyburns(youmonst.data)) { /* paper or straw golem */
                You("go up in flames!");
                /* KMH -- this is okay with unchanging */
                rehumanize();
                break;
            } else if (how_resistant(FIRE_RES) == 100 || Underwater) {
                if (Underwater)
                    pline_The("fire quickly fizzles out.");
                else
                    pline_The("fire doesn't feel hot!");
                monstseesu(M_SEEN_FIRE);
                dmg = 0;
            } else {
                dmg = resist_reduce(dmg, FIRE_RES);
            }
            if (!Underwater) {
                if ((int) mtmp->m_lev > rn2(20))
                    destroy_item(SCROLL_CLASS, AD_FIRE);
                if ((int) mtmp->m_lev > rn2(20))
                    destroy_item(POTION_CLASS, AD_FIRE);
                if ((int) mtmp->m_lev > rn2(25))
                    destroy_item(SPBOOK_CLASS, AD_FIRE);
                burn_away_slime();
            }
        } else
            dmg = 0;
        break;
    case AD_COLD:
        hitmsg(mtmp, mattk);
        if (uncancelled) {
            pline("You're covered in frost!");
            if (how_resistant(COLD_RES) == 100) {
                pline_The("frost doesn't seem cold!");
                monstseesu(M_SEEN_COLD);
                dmg = 0;
            } else {
                dmg = resist_reduce(dmg, COLD_RES);
            }
            if ((int) mtmp->m_lev > rn2(20))
                destroy_item(POTION_CLASS, AD_COLD);
        } else
            dmg = 0;
        break;
    case AD_ELEC:
        hitmsg(mtmp, mattk);
        if (uncancelled) {
            You("get zapped!");
            if (how_resistant(SHOCK_RES) == 100) {
                pline_The("zap doesn't shock you!");
                monstseesu(M_SEEN_ELEC);
                dmg = 0;
            } else {
                dmg = resist_reduce(dmg, SHOCK_RES);
            }
            if ((int) mtmp->m_lev > rn2(20))
                destroy_item(WAND_CLASS, AD_ELEC);
            if ((int) mtmp->m_lev > rn2(20))
                destroy_item(RING_CLASS, AD_ELEC);
        } else
            dmg = 0;
        break;
    case AD_SLEE:
        hitmsg(mtmp, mattk);
        if (uncancelled && multi >= 0 && !rn2(5)) {
            if (how_resistant(SLEEP_RES) == 100) {
                monstseesu(M_SEEN_SLEEP);
                break;
            }
            fall_asleep(-resist_reduce(rnd(10), SLEEP_RES), TRUE);
            if (Blind)
                You("are put to sleep!");
            else
                You("are put to sleep by %s!", mon_nam(mtmp));
        }
        break;
    case AD_BLND:
        if (can_blnd(mtmp, &youmonst, mattk->aatyp, (struct obj *) 0)) {
            if (!Blind)
                pline("%s blinds you!", Monnam(mtmp));
            make_blinded(Blinded + (long) dmg, FALSE);
            if (!Blind)
                Your1(vision_clears);
        }
        dmg = 0;
        break;
    case AD_DRST:
        ptmp = A_STR;
        goto dopois;
    case AD_DRDX:
        ptmp = A_DEX;
        goto dopois;
    case AD_DRCO:
        ptmp = A_CON;
 dopois:
        hitmsg(mtmp, mattk);
        if (uncancelled && !rn2(8)) {
            Sprintf(buf, "%s %s", s_suffix(Monnam(mtmp)),
                    mpoisons_subj(mtmp, mattk));
            poisoned(buf, ptmp, mdat->mname, 30, FALSE);
        }
        break;
    case AD_DRIN:
        hitmsg(mtmp, mattk);
        if (defends(AD_DRIN, uwep) || !has_head(youmonst.data)) {
            You("don't seem harmed.");
            /* Not clear what to do for green slimes */
            break;
        }

        if (u_slip_free(mtmp, mattk))
            break;

        /* The material of the helmet on your head determines how effective
         * it will be when deflecting tentacle/bite attacks. Harder material
         * will do a better job than a soft cap. */
        if (uarmh && is_hard(uarmh) && rn2(4)) {
            /* not body_part(HEAD) */
            Your("%s blocks the %s to your head.",
                 helm_simple_name(uarmh),
                 racial_zombie(mtmp) ? "bite" : "attack");
            break;
        }

        if (uarmh && !is_hard(uarmh) && rn2(2)) {
            Your("%s repels the %s to your head.",
                 helm_simple_name(uarmh),
                 racial_zombie(mtmp) ? "bite" : "attack");
            break;
        }

        if (maybe_polyd(is_illithid(youmonst.data), Race_if(PM_ILLITHID))
            && !racial_zombie(mtmp)) {
            Your("psionic abilities shield your brain.");
            break;
        }

        if (racial_zombie(mtmp) && rn2(6)) {
            if (uncancelled) {
                if (!Upolyd && Race_if(PM_DRAUGR))
                    pline("%s tries to eat your brains, but can't.",
                          Monnam(mtmp));
                else
                    pline("%s eats your brains!", Monnam(mtmp));
                diseasemu(mtmp);
            }
            break;
        }
        /* negative armor class doesn't reduce this damage */
        if (Half_physical_damage)
            dmg = (dmg + 1) / 2;
        mdamageu(mtmp, dmg);
        dmg = 0; /* don't inflict a second dose below */

        if (!uarmh || uarmh->otyp != DUNCE_CAP) {
            /* eat_brains() will miss if target is mindless (won't
               happen here; hero is considered to retain his mind
               regardless of current shape) or is noncorporeal
               (can't happen here; no one can poly into a ghost
               or shade) so this check for missing is academic */
            if (eat_brains(mtmp, &youmonst, TRUE, (int *) 0) == MM_MISS)
                break;
        }
        /* adjattrib gives dunce cap message when appropriate */
        if (maybe_polyd(is_illithid(youmonst.data), Race_if(PM_ILLITHID))) {
            if (!rn2(3))
                Your("psionic abilities shield your brain from memory loss.");
            break;
        } else {
            (void) adjattrib(A_INT, -rnd(2), FALSE);
            if (!racial_zombie(mtmp)) {
                forget_traps();
                if (rn2(2))
                    forget(rnd(u.uluck <= 0 ? 4 : 2));
            }
            break;
        }
    case AD_PLYS:
        hitmsg(mtmp, mattk);
        /* From xNetHack:
         * Ghosts don't have a "paralyzing touch"; this is simply the most
         * convenient place to put this code. What they actually do is try to
         * pop up out of nowhere right next to you, frightening you to death
         * (which of course paralyzes you). */
        if (mtmp->data == &mons[PM_GHOST]) {
            boolean couldspot = canspotmon(mtmp);
            if (multi < 0) /* already paralyzed; ghost won't appear */
                break;
            if (mtmp->minvis) {
                mtmp->minvis = 0;
                mtmp->mspec_used = d(2, 8);
                /* canseemon rather than canspotmon; if you can spot but not see
                 * the ghost, you won't notice that it became visible */
                if (canseemon(mtmp) && !Unaware) {
                    newsym(mtmp->mx, mtmp->my);
                    if (!couldspot) {
                        /* only works if you didn't know it was there before it
                        * turned visible */
                        if (Hallucination)
                            verbalize("Boo!");
                        else
                            pline("A ghost appears out of %s!",
                                  rn2(2) ? "the shadows" : "nowhere");
                        scary_ghost(mtmp);
                    } else {
                        if (Blind) {
                            pline("%s seems easier to spot.", Monnam(mtmp));
                        } else {
                            pline("%s becomes visible!", Monnam(mtmp));
                            if (!rn2(3))
                                You("aren't %s.", rn2(2) ? "afraid" : "scared");
                        }
                    }
                }
            }
            break;
        }
        if (uncancelled && multi >= 0 && !rn2(3)) {
            if (Free_action) {
                You("momentarily stiffen.");
            } else {
                if (Blind)
                    You("are frozen!");
                else
                    You("are frozen by %s!", mon_nam(mtmp));
                nomovemsg = You_can_move_again;
                nomul(-rnd(10));
                multi_reason = "paralyzed by a monster";
                exercise(A_DEX, FALSE);
            }
        }
        break;
    case AD_DRLI:
        hitmsg(mtmp, mattk);
        if (uncancelled && !rn2(3) && !Drain_resistance) {
            /* if vampire biting (and also a pet) */
            if (racial_vampire(mtmp) && mattk->aatyp == AT_BITE
                && has_blood(youmonst.data)) {
                Your("blood is being drained!");
                /* get 1/20th of full corpse value,
                   therefore 4 bites == 1 drink */
                if (mtmp->mtame && !mtmp->isminion)
                    EDOG(mtmp)->hungrytime += ((int) ((youmonst.data)->cnutrit / 20) + 1);
            }
            losexp("life drainage");
        }
        break;
    case AD_LEGS: {
        long side = rn2(2) ? RIGHT_SIDE : LEFT_SIDE;
        const char *sidestr = (side == RIGHT_SIDE) ? "right" : "left",
                   *Monst_name = Monnam(mtmp), *leg = body_part(LEG);

        /* This case is too obvious to ignore, but Nethack is not in
         * general very good at considering height--most short monsters
         * still _can_ attack you when you're flying or mounted.
         */
        if ((u.usteed || Levitation || Flying) && !is_flyer(mtmp->data)) {
            pline("%s tries to reach your %s %s!", Monst_name, sidestr, leg);
            dmg = 0;
        } else if (mtmp->mcan) {
            pline("%s nuzzles against your %s %s!", Monnam(mtmp),
                  sidestr, leg);
            dmg = 0;
        } else {
            if (uarmf) {
                if (rn2(2) && (uarmf->otyp == LOW_BOOTS
                               || uarmf->otyp == DWARVISH_BOOTS)) {
                    pline("%s pricks the exposed part of your %s %s!",
                          Monst_name, sidestr, leg);
                } else if (!rn2(5)) {
                    pline("%s pricks through your %s boot!", Monst_name,
                          sidestr);
                } else {
                    pline("%s scratches your %s boot!", Monst_name,
                          sidestr);
                    dmg = 0;
                    break;
                }
            } else
                pline("%s pricks your %s %s!", Monst_name, sidestr, leg);

            set_wounded_legs(side, rnd(60 - ACURR(A_DEX)));
            exercise(A_STR, FALSE);
            exercise(A_DEX, FALSE);
        }
        break;
    }
    case AD_STON: /* cockatrice */
        hitmsg(mtmp, mattk);
        if (!rn2(3)) {
            if (mtmp->mcan) {
                if (!Deaf)
                    You_hear("a cough from %s!", mon_nam(mtmp));
            } else {
                if (!Deaf)
                    You_hear("%s hissing!", s_suffix(mon_nam(mtmp)));
                if (!rn2(6)
                    || (flags.moonphase == NEW_MOON && !have_lizard())) {
 do_stone:
                    if (!Stoned && !Stone_resistance
                        && !(poly_when_stoned(youmonst.data)
                             && (polymon(PM_STONE_GOLEM)
                                 || polymon(PM_PETRIFIED_ENT)))) {
                        int kformat = KILLED_BY_AN;
                        const char *kname = mtmp->data->mname;

                        if (mtmp->data->geno & G_UNIQ) {
                            if (!type_is_pname(mtmp->data))
                                kname = the(kname);
                            kformat = KILLED_BY;
                        }
                        make_stoned(5L, (char *) 0, kformat, kname);
                        return 1;
                    }
                }
            }
        }
        break;
    case AD_STCK:
        hitmsg(mtmp, mattk);
        if (uncancelled && !u.ustuck && !sticks(youmonst.data)) {
            u.ustuck = mtmp;
            if (mtmp->mundetected) {
                mtmp->mundetected = 0;
                newsym(mtmp->mx, mtmp->my);
            }
        }
        break;
    case AD_WRAP:
        if ((!mtmp->mcan || u.ustuck == mtmp) && !sticks(youmonst.data)) {
            /* salamanders miss less ofen than most other monsters,
               assassin vines rarely miss at all */
            if (!u.ustuck
                && mtmp->data == &mons[PM_ASSASSIN_VINE]
                   ? rn2(5) : mtmp->data == &mons[PM_SALAMANDER]
                       ? !rn2(6) : !rn2(8)) {
                if (u_slip_free(mtmp, mattk)) {
                    dmg = 0;
                } else {
                    if (mtmp->data == &mons[PM_MIND_FLAYER_LARVA])
                        pline("%s wraps its tentacles around your %s, attaching itself to your %s!",
                              Monnam(mtmp), body_part(HEAD), body_part(FACE));
                    else
                        pline("%s %s around you!", Monnam(mtmp),
                              mtmp->data == &mons[PM_GIANT_CENTIPEDE]
                              ? "coils its body"
                                  : mtmp->data == &mons[PM_SALAMANDER]
                                      ? "wraps its arms"
                                          : mtmp->data == &mons[PM_ASSASSIN_VINE]
                                              ? "winds itself" : "swings itself");
                    stop_occupation();
                    u.ustuck = mtmp;
                    if (mtmp->mundetected) {
                        mtmp->mundetected = 0;
                        newsym(mtmp->mx, mtmp->my);
                    }
                }
            } else if (u.ustuck == mtmp) {
                boolean freeze = has_cold_feet(&youmonst);
                if (is_pool(mtmp->mx, mtmp->my) && !Swimming
                    && !Amphibious && !Breathless && !freeze
                    && mtmp->data != &mons[PM_MIND_FLAYER_LARVA]) {
                    boolean moat = (levl[mtmp->mx][mtmp->my].typ != POOL)
                                   && (levl[mtmp->mx][mtmp->my].typ != WATER)
                                   && !Is_medusa_level(&u.uz)
                                   && !Is_waterlevel(&u.uz);

                    pline("%s drowns you...", Monnam(mtmp));
                    killer.format = KILLED_BY_AN;
                    Sprintf(killer.name, "%s by %s",
                            moat ? "moat" : "pool of water",
                            an(mtmp->data->mname));
                    done(DROWNING);
                } else if (is_lava(mtmp->mx, mtmp->my) && !freeze) {
                    pline("%s pulls you into the lava...", Monnam(mtmp));
                    killer.format = NO_KILLER_PREFIX;
                    Sprintf(killer.name, "incinerated in molten lava by %s",
                            an(mtmp->data->mname));
                    done(DIED);
                } else if (mattk->aatyp == AT_HUGS) {
                    You("are being crushed.");
                } else if (mattk->aatyp == AT_TENT) {
                    if (Race_if(PM_ILLITHID) || is_mind_flayer(youmonst.data)) {
                        pline("%s senses you are kin, and releases its grip.",
                              Monnam(mtmp));
                        u.ustuck = 0;
                        dmg = 0;
                    } else {
                        /* A dying larva can't complete transformation */
                        if (mtmp->mstate & MON_DETACH) {
                            pline("%s shudders and loses its grip!", Monnam(mtmp));
                            u.ustuck = 0;
                            dmg = 0;
                        } else {
                            pline("%s burrows itself into your brain!",
                                  Monnam(mtmp));
                            Your("last thoughts fade away as your begin your transformation...");
                            mongone(mtmp); /* mind flayer larva just took over your body */
                            killer.format = NO_KILLER_PREFIX;
                            Sprintf(killer.name, "became a parasitic host to %s",
                                    an(mtmp->data->mname));
                            done(DIED);
                        }
                    }
                }
            } else {
                dmg = 0;
                if (flags.verbose) {
                    if (mtmp->data == &mons[PM_SALAMANDER])
                        pline("%s tries to grab you!",
                               Monnam(mtmp));
                    else if (mtmp->data == &mons[PM_MIND_FLAYER_LARVA])
                        pline("%s tries to attach itself to your %s!",
                              Monnam(mtmp), body_part(FACE));
                    else
                        pline("%s brushes against your %s.", Monnam(mtmp),
                              mtmp->data == &mons[PM_GIANT_CENTIPEDE]
                              ? "body" : body_part(LEG));
                }
            }
        } else
            dmg = 0;
        break;
    case AD_WERE:
        hitmsg(mtmp, mattk);
        if (uncancelled && !rn2(4) && u.ulycn == NON_PM
            && !Protection_from_shape_changers
            && !Lycan_resistance && !defended(&youmonst, AD_WERE)) {
            You_feel("feverish.");
            exercise(A_CON, FALSE);
            if (mdat == &mons[PM_RAT_KING])
                set_ulycn(PM_WERERAT);
            else
                set_ulycn(monsndx(mdat));
            retouch_equipment(2);
        }
        break;
    case AD_SGLD:
        hitmsg(mtmp, mattk);
        if (youmonst.data->mlet == mdat->mlet)
            break;
        if (!(mtmp->mcan || Hidinshell))
            stealgold(mtmp);
        break;

    case AD_SSEX:
        if (SYSOPT_SEDUCE) {
            if (could_seduce(mtmp, &youmonst, mattk) == 1
                && !(mtmp->mcan || Hidinshell))
                if (doseduce(mtmp))
                    return 3;
            break;
        }
        /*FALLTHRU*/
    case AD_SITM: /* for now these are the same */
    case AD_SEDU: {
        int is_robber = (is_animal(mtmp->data) || is_rogue(mtmp->data));
        boolean purity = (u.ualign.type == A_LAWFUL && uarmg
                          && uarmg->oartifact == ART_GAUNTLETS_OF_PURITY);
        boolean druid_mystic = (Role_if(PM_DRUID) && u.ulevel >= 14
                                && is_nymph(mtmp->data));

        if (is_robber) {
            hitmsg(mtmp, mattk);
            if (mtmp->mcan || Hidinshell)
                break;
            /* Continue below */
        } else if (dmgtype(youmonst.data, AD_SEDU)
                   /* !SYSOPT_SEDUCE: when hero is attacking and AD_SSEX
                      is disabled, it would be changed to another damage
                      type, but when defending, it remains as-is */
                   || dmgtype(youmonst.data, AD_SSEX)) {
            pline("%s %s.", Monnam(mtmp),
                  Deaf ? "says something but you can't hear it"
                       : mtmp->minvent
                      ? "brags about the goods some dungeon explorer provided"
                  : "makes some remarks about how difficult theft is lately");
            if (!tele_restrict(mtmp))
                (void) rloc(mtmp, TRUE);
            return 3;
        } else if (mtmp->mcan || Hidinshell
                   || purity || druid_mystic) {
            if (!Blind)
                pline("%s tries to %s you, but you seem %s.",
                      !mtmp->mcan ? Monnam(mtmp) : Adjmonnam(mtmp, "plain"),
                      flags.female ? "charm" : "seduce",
                      flags.female ? "unaffected" : "uninterested");
            if (rn2(3)) {
                if (!tele_restrict(mtmp))
                    (void) rloc(mtmp, TRUE);
                return 3;
            }
            break;
        }
        buf[0] = '\0';
        switch (steal(mtmp, buf)) {
        case -1:
            return 2;
        case 0:
            break;
        default:
            if (!is_robber && !tele_restrict(mtmp))
                (void) rloc(mtmp, TRUE);
            if (is_robber && *buf) {
                if (canseemon(mtmp))
                    pline("%s tries to %s away with %s.", Monnam(mtmp),
                          locomotion(mtmp->data, "run"), buf);
            }
            monflee(mtmp, 0, FALSE, FALSE);
            return 3;
        }
        break;
    }
    case AD_SAMU:
        hitmsg(mtmp, mattk);
        /* when the Wizard or quest nemesis hits, there's a 1/20 chance
           to steal a quest artifact (any but the one for the hero's
           own role) or the Amulet or one of the invocation tools */
        if (Hidinshell)
            break;
        if (!rn2(20)) {
            stealamulet(mtmp);
            if (In_endgame(&u.uz) && mon_has_amulet(mtmp)) {
                monflee(mtmp, rnd(100) + 100, FALSE, TRUE);
            }
        }
        break;

    case AD_TLPT:
        hitmsg(mtmp, mattk);
	if (uncancelled || mtmp->mnum == PM_BOOJUM) {
	    /* "But oh, beamish nephew, beware of the day
	     * if your Snark be a Boojum!  For then
	     * You will softly and suddenly vanish away,
	     * And never be met with again!" */
	    if (mtmp->mnum == PM_BOOJUM) {
		/* depending on what we are or if we can't teleport,
		 * display appropriate messages */
		if (!level.flags.noteleport) {
		    You("suddenly vanish!");
                } else {
                    if (!Invis) {
                        You("suddenly %s!", See_invisible ? "become transparent" : "vanish");
                    }
                }
		incr_itimeout(&HInvis, d(6, 100));	  /* In multiple senses of 'vanish' :) */
	    } else {
                if (flags.verbose)
                    Your("position suddenly seems %suncertain!",
                         (Teleport_control && !Stunned && !unconscious()) ? ""
                         : "very ");
            }
            tele();
            /* As of 3.6.2:  make sure damage isn't fatal; previously, it
               was possible to be teleported and then drop dead at
               the destination when QM's 1d4 damage gets applied below;
               even though that wasn't "wrong", it seemed strange,
               particularly if the teleportation had been controlled
               [applying the damage first and not teleporting if fatal
               is another alternative but it has its own complications] */
            if ((Half_physical_damage ? (dmg - 1) / 2 : dmg)
                >= (tmphp = (Upolyd ? u.mh : u.uhp))) {
                dmg = tmphp - 1;
                if (Half_physical_damage)
                    dmg *= 2; /* doesn't actually increase damage; we only
                               * get here if half the original damage would
                               * would have been fatal, so double reduced
                               * damage will be less than original damage */
                if (dmg < 1) { /* implies (tmphp <= 1) */
                    dmg = 1;
                    /* this might increase current HP beyond maximum HP but
                       it will be immediately reduced below, so that should
                       be indistinguishable from zero damage; we don't drop
                       damage all the way to zero because that inhibits any
                       passive counterattack if poly'd hero has one */
                    if (Upolyd && u.mh == 1)
                        ++u.mh;
                    else if (!Upolyd && u.uhp == 1)
                        ++u.uhp;
                    /* [don't set context.botl here] */
                }
            }
        }
        break;
    case AD_RUST:
        hitmsg(mtmp, mattk);
do_rust:
        if (mtmp->mcan)
            break;
        if (u.umonnum == PM_IRON_GOLEM) {
            You("rust!");
            /* KMH -- this is okay with unchanging */
            rehumanize();
            break;
        }
        erode_armor(&youmonst, ERODE_RUST);
        break;
    case AD_CORR:
        hitmsg(mtmp, mattk);
        if (mtmp->mcan)
            break;
        erode_armor(&youmonst, ERODE_CORRODE);
        break;
    case AD_DCAY:
        hitmsg(mtmp, mattk);
        if (mtmp->mcan)
            break;
        if (u.umonnum == PM_WOOD_GOLEM || u.umonnum == PM_LEATHER_GOLEM) {
            You("rot!");
            /* KMH -- this is okay with unchanging */
            rehumanize();
            break;
        }
        erode_armor(&youmonst, ERODE_ROT);
        break;
    case AD_HEAL:
        /* a cancelled nurse is just an ordinary monster,
         * nurses don't heal those that cause petrification,
         * nor will they heal the undead */
        if (mtmp->mcan || (Upolyd && touch_petrifies(youmonst.data))
            || is_undead(youmonst.data) || Race_if(PM_DRAUGR)
            || Race_if(PM_VAMPIRE)) {
            if (is_undead(youmonst.data) || Race_if(PM_DRAUGR)
                || Race_if(PM_VAMPIRE)) {
                if (!Deaf && !(moves % 5))
                    verbalize("I can't heal the undead... you're dead!");
            }
            hitmsg(mtmp, mattk);
            break;
        }
        if (!uwep && !uarmu && !uarm && !uarmc
            && !uarms && !uarmf && !uarmh
            && (!uarmg || (uarmg && uarmg->oartifact == ART_HAND_OF_VECNA))) {
            boolean goaway = FALSE;

            pline("%s touches you!  (I hope you don't mind.)", Monnam(mtmp));
            if (Upolyd) {
                u.mh += rnd(7);
                if (!rn2(7)) {
                    /* no upper limit necessary; effect is temporary */
                    u.mhmax++;
                    if (!rn2(13))
                        goaway = TRUE;
                }
                if (u.mh > u.mhmax)
                    u.mh = u.mhmax;
            } else {
                u.uhp += rnd(7);
                if (!rn2(7)) {
                    /* hard upper limit via nurse care: 25 * ulevel */
                    if (u.uhpmax < 5 * u.ulevel + d(2 * u.ulevel, 10))
                        u.uhpmax++;
                    if (!rn2(13))
                        goaway = TRUE;
                }
                if (u.uhp > u.uhpmax)
                    u.uhp = u.uhpmax;
            }
            if (!rn2(3))
                exercise(A_STR, TRUE);
            if (!rn2(3))
                exercise(A_CON, TRUE);
            if (Sick)
                make_sick(0L, (char *) 0, FALSE, SICK_ALL);
            context.botl = 1;
            if (goaway) {
                mongone(mtmp);
                return 2;
            } else if (!rn2(33)) {
                if (!tele_restrict(mtmp))
                    (void) rloc(mtmp, TRUE);
                monflee(mtmp, d(3, 6), TRUE, FALSE);
                return 3;
            }
            dmg = 0;
        } else {
            if (Role_if(PM_HEALER)) {
                if (!Deaf && !(moves % 5))
                    verbalize("Doc, I can't help you unless you cooperate.");
                dmg = 0;
            } else
                hitmsg(mtmp, mattk);
        }
        break;
    case AD_CURS:
        hitmsg(mtmp, mattk);
        if (!night() && mdat == &mons[PM_GREMLIN])
            break;
        if (night() && mdat == &mons[PM_LAVA_GREMLIN])
            break;
        if (!mtmp->mcan && !rn2(10)) {
            if (!Deaf) {
                if (Blind)
                    You_hear("laughter.");
                else
                    pline("%s chuckles.", Monnam(mtmp));
            }
            if (u.umonnum == PM_CLAY_GOLEM) {
                pline("Some writing vanishes from your head!");
                /* KMH -- this is okay with unchanging */
                rehumanize();
                break;
            }
            attrcurse();
        }
        break;
    case AD_STUN:
        hitmsg(mtmp, mattk);
        if (!mtmp->mcan && !rn2(4)) {
            make_stunned((HStun & TIMEOUT) + (long) dmg, TRUE);
            dmg /= 2;
        }
        break;
    case AD_ACID:
        hitmsg(mtmp, mattk);
        if (!mtmp->mcan && !rn2(3))
            if (Acid_resistance || Underwater) {
                pline("You're covered in %s, but it seems harmless.",
                      hliquid("acid"));
                monstseesu(M_SEEN_ACID);
                dmg = 0;
            } else {
                pline("You're covered in %s!  It burns!", hliquid("acid"));
                exercise(A_STR, FALSE);
            }
        else
            dmg = 0;
        break;
    case AD_SLOW:
        hitmsg(mtmp, mattk);
        if (uncancelled && !Slow && !defended(&youmonst, AD_SLOW)
            && !resists_slow(youmonst.data) && !rn2(3))
            u_slow_down();
        stop_occupation();
        break;
    case AD_DREN:
        hitmsg(mtmp, mattk);
        if (uncancelled && !rn2(4)) /* 25% chance */
            drain_en(dmg);
        dmg = 0;
        break;
    case AD_CONF:
        hitmsg(mtmp, mattk);
        if (!mtmp->mcan && !rn2(4) && !mtmp->mspec_used) {
            mtmp->mspec_used = mtmp->mspec_used + (dmg + rn2(6));
            if (Confusion)
                You("are getting even more confused.");
            else
                You("are getting confused.");
            make_confused(HConfusion + dmg, FALSE);
        }
        dmg = 0;
        break;
    case AD_DETH:
        if (mtmp && mdat == &mons[PM_DEATH])
            pline("%s reaches out with its deadly touch.", Monnam(mtmp));
        if (Death_resistance || immune_death_magic(youmonst.data)) {
            /* Still does normal damage */
            shieldeff(u.ux, u.uy);
            monstseesu(M_SEEN_DEATH);
            You("are unaffected by death magic.");
            break;
        }
        switch (rn2(20)) {
        case 19:
        case 18:
        case 17:
            if (!Antimagic) {
                killer.format = KILLED_BY_AN;
                Strcpy(killer.name, "touch of death");
                ukiller = mtmp;
                done(DIED);
                dmg = 0;
                break;
            }
            /*FALLTHRU*/
        default: /* case 16: ... case 5: */
            You_feel("your life force draining away...");
            permdmg = 1; /* actual damage done below */
            break;
        case 4:
        case 3:
        case 2:
        case 1:
        case 0:
            if (Antimagic) {
                shieldeff(u.ux, u.uy);
                monstseesu(M_SEEN_MAGR);
            }
            pline("Lucky for you, it didn't work!");
            dmg = 0;
            break;
        }
        break;
    case AD_PEST:
        pline("%s reaches out, and you feel fever and chills.", Monnam(mtmp));
        (void) diseasemu(mtmp); /* plus the normal damage */
        break;
    case AD_FAMN:
        pline("%s reaches out, and your body shrivels.", Monnam(mtmp));
        exercise(A_CON, FALSE);
        if (!is_fainted())
            morehungry(rn1(40, 40));
        /* plus the normal damage */
        break;
    case AD_SLIM:
        hitmsg(mtmp, mattk);
        if (!uncancelled)
            break;
        if (flaming(youmonst.data)) {
            pline_The("slime burns away!");
            dmg = 0;
        } else if (Unchanging || noncorporeal(youmonst.data)
                   || youmonst.data == &mons[PM_GREEN_SLIME]) {
            You("are unaffected.");
            dmg = 0;
        } else if (!Slimed) {
            You("don't feel very well.");
            make_slimed(10L, (char *) 0);
            delayed_killer(SLIMED, KILLED_BY_AN, mtmp->data->mname);
        } else
            pline("Yuck!");
        break;
    case AD_ENCH: /* KMH -- remove enchantment (disenchanter) */
        hitmsg(mtmp, mattk);
        /* uncancelled is sufficient enough; please
           don't make this attack less frequent */
        if (uncancelled) {
            struct obj *obj = some_armor(&youmonst);

            if (!obj) {
                /* some rings are susceptible;
                   amulets and blindfolds aren't (at present) */
                switch (rn2(5)) {
                case 0:
                    break;
                case 1:
                    obj = uright;
                    break;
                case 2:
                    obj = uleft;
                    break;
                case 3:
                    obj = uamul;
                    break;
                case 4:
                    obj = ublindf;
                    break;
                }
            }
            if (drain_item(obj, FALSE)) {
                pline("%s less effective.", Yobjnam2(obj, "seem"));
            }
        }
        break;
    case AD_BHED:
        if ((!rn2(15) || is_jabberwock(youmonst.data))
            && !mtmp->mcan) {
            if (!has_head(youmonst.data) || vorpal_wield) {
                pline("Somehow, %s misses you wildly.", mon_nam(mtmp));
                dmg = 0;
                break;
            }
            if (noncorporeal(youmonst.data) || amorphous(youmonst.data)) {
                /* still take regular damage */
                pline("%s slices through your %s.",
                      Monnam(mtmp), body_part(NECK));
                break;
            }
            if (Hidinshell) {
                pline("%s attack glances harmlessly off of your protective shell.",
                      s_suffix(Monnam(mtmp)));
                dmg = 0;
                break;
            }
            pline("%s %ss you!", Monnam(mtmp),
                  rn2(2) ? "behead" : "decapitate");
            if (Upolyd) {
                rehumanize();
            } else {
                killer.format = NO_KILLER_PREFIX;
                Sprintf(killer.name, "decapitated by %s",
                        an(l_monnam(mtmp)));
                done(DECAPITATED);
            }
            dmg = 0;
        } else
            hitmsg(mtmp, mattk);
        break;
    case AD_POLY:
        hitmsg(mtmp, mattk);
        if (uncancelled && Maybe_Half_Phys(dmg) < (Upolyd ? u.mh : u.uhp))
            dmg = mon_poly(mtmp, &youmonst, dmg);
        break;
    case AD_WTHR: {
        uchar withertime = max(2, dmg);
        boolean no_effect =
            (nonliving(youmonst.data)
             || racial_vampire(&youmonst)
             || racial_zombie(&youmonst) || !uncancelled);
        boolean lose_maxhp = (withertime >= 8 && !BWithering); /* if already withering */
        dmg = 0; /* doesn't deal immediate damage */

        hitmsg(mtmp, mattk);
        if (!no_effect) {
            if (Withering)
                Your("withering speeds up!");
            else if (!BWithering)
                You("begin to wither away!");
            else
                You_feel("dessicated for a moment.");
            incr_itimeout(&HWithering, withertime);

            if (lose_maxhp) {
                if (Upolyd && u.mhmax > 1) {
                    u.mhmax--;
                    u.mh = min(u.mh, u.mhmax);
                } else if (u.uhpmax > 1) {
                    u.uhpmax--;
                    u.uhp = min(u.uhp, u.uhpmax);
                }
            }
        }
        break;
    }
    case AD_PITS:
        /* For some reason, the uhitm code calls this for any AT_HUGS attack,
         * but the mhitu code doesn't. */
        if (rn2(2)) {
            if (mattk->aatyp == AT_HUGS) {
                u.ustuck = mtmp;
                if (mtmp->mundetected) {
                    mtmp->mundetected = 0;
                    newsym(mtmp->mx, mtmp->my);
                }
            }
            if (!mtmp->mcan) {
                if (!create_pit_under(&youmonst, mtmp))
                    dmg = 0;
            }
        }
        break;
    case AD_WEBS:
        if (!uncancelled)
            break;
        if (!t_at(u.ux, u.uy)) {
            struct trap *web = maketrap(u.ux, u.uy, WEB);
            if (web) {
                hitmsg(mtmp, mattk);
                pline("%s entangles you in a web%s",
                      Monnam(mtmp),
                      (is_pool_or_lava(u.ux, u.uy)
                       || is_puddle(u.ux, u.uy)
                       || is_sewage(u.ux, u.uy)
                       || IS_AIR(levl[u.ux][u.uy].typ))
                          ? ", but it has nothing to anchor to."
                          : is_giant(youmonst.data)
                              ? ", but you rip through it!"
                              : (webmaker(youmonst.data)
                                 || maybe_polyd(is_drow(youmonst.data),
                                                Race_if(PM_DROW)))
                                  ? ", but you easily disentangle yourself."
                                  : "!");
                dotrap(web, NOWEBMSG);
                if (u.usteed && u.utrap)
                    dismount_steed(DISMOUNT_FELL);
            }
        }
        break;
    default:
        dmg = 0;
        break;
    }

    /* player monster monks can sometimes stun with their kick attack */
    if (mattk->aatyp == AT_KICK && mdat == &mons[PM_MONK]
        && !rn2(10) && youmonst.data->msize < MZ_HUGE) {
        if (!(Stun_resistance || wielding_artifact(ART_TEMPEST)))
            You("reel from %s powerful kick!", s_suffix(mon_nam(mtmp)));
        make_stunned((HStun & TIMEOUT) + (long) dmg, TRUE);
        dmg /= 2;
    }

    if ((Upolyd ? u.mh : u.uhp) < 1) {
        /* already dead? call rehumanize() or done_in_by() as appropriate */
        mdamageu(mtmp, 1);
        dmg = 0;
    }

    /*  Negative armor class reduces damage done instead of fully protecting
     *  against hits.
     */
    if (dmg && u.uac < 0) {
        dmg -= rnd(-u.uac);
        if (dmg < 1)
            dmg = 1;
    }

    /* handle body/equipment made out of harmful materials for touch attacks */
    /* should come after AC damage reduction */
    dmg += special_dmgval(mtmp, &youmonst, armask, &hated_obj);
    if (hated_obj) {
        searmsg(mtmp, &youmonst, hated_obj, FALSE);
        exercise(A_CON, FALSE);
    }

    if (dmg) {
        if (Half_physical_damage
            /* Mitre of Holiness */
            || (Role_if(PM_PRIEST) && uarmh && is_quest_artifact(uarmh)
                && (is_undead(mtmp->data) || is_demon(mtmp->data)
                    || is_vampshifter(mtmp))))
            dmg = (dmg + 1) / 2;

        if (permdmg) { /* Death's life force drain */
            int lowerlimit, *hpmax_p;
            /*
             * Apply some of the damage to permanent hit points:
             *  polymorphed         100% against poly'd hpmax
             *  hpmax > 25*lvl      100% against normal hpmax
             *  hpmax > 10*lvl  50..100%
             *  hpmax >  5*lvl  25..75%
             *  otherwise        0..50%
             * Never reduces hpmax below 1 hit point per level.
             */
            permdmg = rn2(dmg / 2 + 1);
            if (Upolyd || u.uhpmax > 25 * u.ulevel)
                permdmg = dmg;
            else if (u.uhpmax > 10 * u.ulevel)
                permdmg += dmg / 2;
            else if (u.uhpmax > 5 * u.ulevel)
                permdmg += dmg / 4;

            if (Upolyd) {
                hpmax_p = &u.mhmax;
                /* [can't use youmonst.m_lev] */
                lowerlimit = min((int) youmonst.data->mlevel, u.ulevel);
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
            context.botl = 1;
        }

        /* adjust for various effects/conditions */
        if (mattk->aatyp == AT_WEAP) {
            struct obj *mwep, *nextobj;

            for (mwep = mtmp->minvent; mwep; mwep = nextobj) {
                nextobj = mwep->nobj;
                if (MON_WEP(mtmp) && is_axe(mwep)
                    && (is_wooden(youmonst.data)
                        || is_plant(youmonst.data) || Barkskin)) {
                    dmg += rnd(4);
                } else if (MON_WEP(mtmp)
                           && objects[mwep->otyp].oc_dir & WHACK
                           && (is_wooden(youmonst.data)
                               || is_plant(youmonst.data) || Barkskin)) {
                    dmg -= rnd(3) + 3;
                } else if (MON_WEP(mtmp)
                           && objects[mwep->otyp].oc_dir & (PIERCE | SLASH)
                           && (is_bone_monster(youmonst.data) || Stoneskin)) {
                    dmg -= rnd(5) + 3;
                } else if (MON_WEP(mtmp)
                           && objects[mwep->otyp].oc_dir & WHACK
                           && is_bone_monster(youmonst.data)) {
                    dmg += rnd(4);
                } else if (MON_WEP(mtmp) && mwep->forged_qual == FQ_SUPERIOR) {
                    dmg += 1;
                } else if (MON_WEP(mtmp) && mwep->forged_qual == FQ_EXCEPTIONAL) {
                    dmg += 2;
                } else if (MON_WEP(mtmp) && mwep->forged_qual == FQ_INFERIOR) {
                    dmg -= 2;
                }
                if (dmg < 1)
                    dmg = 1;
            }
        }

        mdamageu(mtmp, dmg);
    }

    /* If monster was marked for removal during attack processing
       (e.g., mind flayer larva dying before completing transformation),
       it's already dead - don't process passive defense */
    if (mtmp->mstate & MON_DETACH)
        return 2; /* attacker is dead */

    if (dmg)
        res = passiveum(olduasmon, mtmp, mattk);
    else
        res = 1;
    stop_occupation();
    return res;
}

/* An interface for use when taking a blindfold off, for example,
 * to see if an engulfing attack should immediately take affect, like
 * a passive attack. TRUE if engulfing blindness occurred */
boolean
gulp_blnd_check()
{
    struct attack *mattk;

    if (!Blinded && u.uswallow
        && (mattk = attacktype_fordmg(u.ustuck->data, AT_ENGL, AD_BLND))
        && can_blnd(u.ustuck, &youmonst, mattk->aatyp, (struct obj *) 0)) {
        ++u.uswldtim; /* compensate for gulpmu change */
        (void) gulpmu(u.ustuck, mattk);
        return TRUE;
    }
    return FALSE;
}

/* monster swallows you, or damage if u.uswallow */
STATIC_OVL int
gulpmu(mtmp, mattk)
struct monst *mtmp;
struct attack *mattk;
{
    struct trap *t = t_at(u.ux, u.uy);
    int tmp = d((int) mattk->damn, (int) mattk->damd);
    int tim_tmp;
    struct obj *otmp2, *nextobj;
    int i;
    boolean physical_damage = FALSE;
    /* for tracking if this is the first engulf */
    boolean old_uswallow = u.uswallow;

    if (!u.uswallow) { /* swallows you */
        int omx = mtmp->mx, omy = mtmp->my;

        if (!engulf_target(mtmp, &youmonst))
            return 0;
        if ((t && is_pit(t->ttyp)) && sobj_at(BOULDER, u.ux, u.uy))
            return 0;

        if (!goodpos(u.ux, u.uy, mtmp, GP_ALLOW_U))
            return 0;

        if (Punished)
            unplacebc(); /* ball&chain go away */
        remove_monster(omx, omy);
        mtmp->mtrapped = 0; /* no longer on old trap */
        mtmp->mentangled = 0; /* no longer entangled */
        place_monster(mtmp, u.ux, u.uy);
        u.ustuck = mtmp;
        if (mtmp->mundetected) {
            mtmp->mundetected = 0;
        }
        newsym(mtmp->mx, mtmp->my);
        if (u.usteed) {
            char buf[BUFSZ];

            Strcpy(buf, mon_nam(u.usteed));
            pline("%s %s forward and plucks you off %s!",
                  Monnam(mtmp),
                  is_animal(mtmp->data) ? "lunges"
                    : amorphous(mtmp->data) ? "oozes"
                      : "surges",
                  buf);
            dismount_steed(DISMOUNT_ENGULFED);
        } else
            pline("%s engulfs you!", Monnam(mtmp));
        stop_occupation();
        reset_occupations(); /* behave as if you had moved */

        if (u.utrap) {
            You("are released from the %s!",
                u.utraptype == TT_WEB ? "web" : "trap");
            reset_utrap(FALSE);
        }

        i = number_leashed();
        if (i > 0) {
            const char *s = (i > 1) ? "leashes" : "leash";

            pline_The("%s %s loose.", s, vtense(s, "snap"));
            unleash_all();
        }

        if (touch_petrifies(youmonst.data)
            && !(resists_ston(mtmp) || defended(mtmp, AD_STON))) {
            /* put the attacker back where it started;
               the resulting statue will end up there
               [note: if poly'd hero could ride or non-poly'd hero could
               acquire touch_petrifies() capability somehow, this code
               would need to deal with possibility of steed having taken
               engulfer's previous spot when hero was forcibly dismounted] */
            remove_monster(mtmp->mx, mtmp->my); /* u.ux,u.uy */
            place_monster(mtmp, omx, omy);
            minstapetrify(mtmp, TRUE);
            /* normally unstuck() would do this, but we're not
               fully swallowed yet so that won't work here */
            if (Punished)
                placebc();
            u.ustuck = 0;
            return (!DEADMONSTER(mtmp)) ? 0 : 2;
        }

        display_nhwindow(WIN_MESSAGE, FALSE);
        vision_recalc(2); /* hero can't see anything */
        u.uswallow = 1;
        /* for digestion, shorter time is more dangerous;
           for other swallowings, longer time means more
           chances for the swallower to attack */
        if (mattk->adtyp == AD_DGST) {
            tim_tmp = 25 - (int) mtmp->m_lev;
            if (tim_tmp > 0)
                tim_tmp = rnd(tim_tmp) / 2;
            else if (tim_tmp < 0)
                tim_tmp = -(rnd(-tim_tmp) / 2);
            /* having good armor & high constitution makes
               it take longer for you to be digested, but
               you'll end up trapped inside for longer too */
            tim_tmp += -u.uac + 10 + (ACURR(A_CON) / 3 - 1);
        } else {
            /* higher level attacker takes longer to eject hero */
            tim_tmp = rnd((int) mtmp->m_lev + 10 / 2);
        }
        /* u.uswldtim always set > 1 */
        u.uswldtim = (unsigned) ((tim_tmp < 2) ? 2 : tim_tmp);
        swallowed(1);
        for (otmp2 = invent; otmp2; otmp2 = nextobj) {
            nextobj = otmp2->nobj;
            (void) snuff_lit(otmp2);
        }
    }

    if (mtmp != u.ustuck)
        return 0;
    if (Punished) {
        /* ball&chain are in limbo while swallowed; update their internal
           location to be at swallower's spot */
        if (uchain->where == OBJ_FREE)
            uchain->ox = mtmp->mx, uchain->oy = mtmp->my;
        if (uball->where == OBJ_FREE)
            uball->ox = mtmp->mx, uball->oy = mtmp->my;
    }
    if (u.uswldtim > 0)
        u.uswldtim -= 1;

    switch (mattk->adtyp) {
    case AD_DGST:
        physical_damage = TRUE;
        if (Slow_digestion) {
            /* Messages are handled below */
            u.uswldtim = 0;
            tmp = 0;
        } else if (u.uswldtim == 0) {
            pline("%s totally digests you!", Monnam(mtmp));
            tmp = u.uhp;
            if (Half_physical_damage)
                tmp *= 2; /* sorry */
            killer.format = NO_KILLER_PREFIX;
            Sprintf(killer.name, "digested by %s",
                    an(l_monnam(u.ustuck)));
            done(DIED);
        } else {
            pline("%s%s digests you!", Monnam(mtmp),
                  (u.uswldtim == 2) ? " thoroughly"
                                    : (u.uswldtim == 1) ? " utterly" : "");
            exercise(A_STR, FALSE);
        }
        break;
    case AD_PHYS:
        physical_damage = TRUE;
        if (mtmp->data == &mons[PM_FOG_CLOUD]) {
            You("are laden with moisture and %s",
                flaming(youmonst.data)
                    ? "are smoldering out!"
                    : Breathless ? "find it mildly uncomfortable."
                                 : Amphibious
                                       ? "feel comforted."
                                       : "can barely breathe!");
            if ((Amphibious || Breathless)
                && !flaming(youmonst.data))
                tmp = 0;
        } else {
            You("are pummeled with debris!");
            exercise(A_STR, FALSE);
        }
        break;
    case AD_WRAP:
        /* Initially pulled from GruntHack, and then improved upon by
         * aosdict for xNetHack (see git commit ee808b): AD_WRAP is used because
         * there's no specific suffocation attack, but it's used for other
         * suffocation-y things like drowning attacks.
         * Generally, only give a message if this is the first engulf, not a
         * subsequent attack when already engulfed. */
        if (Breathless
            || (Amphibious && (mtmp->data == &mons[PM_WATER_ELEMENTAL]
                               || mtmp->data == &mons[PM_SEA_DRAGON]))) {
            if (!old_uswallow) {
                if (!(HMagical_breathing || EMagical_breathing)) {
                    /* test this one first, in case breathless and also wearing
                     * magical breathing */
                    if (Amphibious)
                        You("can still breathe, though.");
                    else
                        You("can't breathe, but you don't need to.");
                } else {
                    You("can still breathe, though.");
                    if (uamul && uamul->otyp == AMULET_OF_MAGICAL_BREATHING)
                        makeknown(AMULET_OF_MAGICAL_BREATHING);
                }
            }
            tmp = 0;
        } else if (!Strangled) {
            if (!old_uswallow)
                pline("It's impossible to breathe in here!");
            Strangled = 4L; /* xNetHack sets this timer for 5, GruntHack had it for 3.
                             * we'll meet in the middle. */
            tmp = 0;
            /* Immediate timeout message: "You find it hard to breathe." */
        }
        if (mtmp->data == &mons[PM_WATER_ELEMENTAL]
            || mtmp->data == &mons[PM_SEA_DRAGON])
            water_damage_chain(invent, FALSE, rnd(3), FALSE, u.ux, u.uy);
        break;
    case AD_ACID:
        if (Acid_resistance) {
            You("are covered with a seemingly harmless goo.");
            monstseesu(M_SEEN_ACID);
            tmp = 0;
        } else {
            if (Hallucination)
                pline("Ouch!  You've been slimed!");
            else
                You("are covered in slime!  It burns!");
            exercise(A_STR, FALSE);
        }
        break;
    case AD_BLND:
        if (can_blnd(mtmp, &youmonst, mattk->aatyp, (struct obj *) 0)) {
            if (!Blind) {
                long was_blinded = Blinded;

                if (!Blinded)
                    You_cant("see in here!");
                make_blinded((long) tmp, FALSE);
                if (!was_blinded && !Blind)
                    Your1(vision_clears);
            } else
                /* keep him blind until disgorged */
                make_blinded(Blinded + 1, FALSE);
        }
        tmp = 0;
        break;
    case AD_ELEC:
        if (!mtmp->mcan && rn2(2)) {
            pline_The("air around you crackles with electricity.");
            if (how_resistant(SHOCK_RES) == 100) {
                shieldeff(u.ux, u.uy);
                You("seem unhurt.");
                monstseesu(M_SEEN_ELEC);
                ugolemeffects(AD_ELEC, tmp);
                tmp = 0;
	    } else {
		tmp = resist_reduce(tmp, SHOCK_RES);
	    }
        } else
            tmp = 0;
        break;
    case AD_COLD:
        if (!mtmp->mcan && rn2(2)) {
            if (how_resistant(COLD_RES) == 100) {
                shieldeff(u.ux, u.uy);
                You_feel("mildly chilly.");
                monstseesu(M_SEEN_COLD);
                ugolemeffects(AD_COLD, tmp);
                tmp = 0;
            } else {
                You("are freezing to death!");
                tmp = resist_reduce(tmp, COLD_RES);
            }
        } else
            tmp = 0;
        break;
    case AD_FIRE:
        if (!mtmp->mcan && rn2(2)) {
            if (how_resistant(FIRE_RES) == 100) {
                shieldeff(u.ux, u.uy);
                You_feel("mildly hot.");
                monstseesu(M_SEEN_FIRE);
                ugolemeffects(AD_FIRE, tmp);
                tmp = 0;
            } else {
                You("are %s!", on_fire(&youmonst, ON_FIRE_ENGULF));
                tmp = resist_reduce(tmp, FIRE_RES);
            }
            burn_away_slime();
        } else
            tmp = 0;
        break;
    case AD_DISE:
        if (!diseasemu(mtmp))
            tmp = 0;
        break;
    case AD_DREN:
        /* AC magic cancellation doesn't help when engulfed */
        if (!mtmp->mcan && rn2(4)) /* 75% chance */
            drain_en(tmp);
        tmp = 0;
        break;
    case AD_DISN:
        if (!mtmp->mcan && rn2(2) && u.uswldtim < 2) {
            pline_The("air around you shimmers with antiparticles.");
            if (how_resistant(DISINT_RES) == 100) {
                shieldeff(u.ux, u.uy);
                You_feel("mildly tickled.");
                tmp = 0;
                break;
            } else if (how_resistant(DISINT_RES) >= 50) {
                You("aren't disintegrated, but that hurts!");
                tmp = resist_reduce(tmp, DISINT_RES);
                if (tmp)
                    mdamageu(mtmp, tmp);
                break;
            } else if (how_resistant(DISINT_RES) < 50) {
                tmp = resist_reduce(tmp, DISINT_RES);
                if (tmp)
                    mdamageu(mtmp, tmp);
                if (uarms) {
                    /* destroy shield; other possessions are safe */
                    (void) destroy_arm(uarms);
                    break;
                } else if (uarm) {
                    /* destroy suit; if present, cloak goes too */
                    if (uarmc)
                        (void) destroy_arm(uarmc);
                    (void) destroy_arm(uarm);
                    break;
                }
                /* fall through. not having enough disintegration
                   resistance can still get you disintegrated */
            }
            /* no shield or suit, you're dead; wipe out cloak
               and/or shirt in case of life-saving or bones */
            if (uarmc)
                (void) destroy_arm(uarmc);
            if (uarmu)
                (void) destroy_arm(uarmu);

            You("are disintegrated!");
            /* when killed by disintegration, don't leave a corpse */
            u.ugrave_arise = -3;
            killer.format = NO_KILLER_PREFIX;
            Sprintf(killer.name, "disintegrated by %s",
                    an(mtmp->data->mname));
            done(DISINTEGRATED);
        } else {
            tmp = 0;
        }
        break;
    default:
        physical_damage = TRUE;
        tmp = 0;
        break;
    }

    if (physical_damage)
        tmp = Maybe_Half_Phys(tmp);

    mdamageu(mtmp, tmp);
    if (tmp)
        stop_occupation();

    if (!u.uswallow) {
        ; /* life-saving has already expelled swallowed hero */
    } else if (touch_petrifies(youmonst.data)
               && !(resists_ston(mtmp) || defended(mtmp, AD_STON))) {
        pline("%s very hurriedly %s you!", Monnam(mtmp),
              is_swallower(mtmp->data) ? "regurgitates" : "expels");
        expels(mtmp, mtmp->data, FALSE);
    } else if (Passes_walls) {
        /* enabling phasing (artifact) or poly'd into a monster that
           can naturally phase allows our hero to escape being engulfed */
        expels(mtmp, mtmp->data, FALSE);
        You("exit %s, phasing right through %s %s!", mon_nam(mtmp),
            mhis(mtmp), mbodypart(mtmp, STOMACH));
    } else if (!u.uswldtim || youmonst.data->msize >= MZ_HUGE) {
        /* As of 3.6.2: u.uswldtim used to be set to 0 by life-saving but it
           expels now so the !u.uswldtim case is no longer possible;
           however, polymorphing into a huge form while already
           swallowed is still possible */
        expels(mtmp, mtmp->data, TRUE);
        if (flags.verbose
            && (is_swallower(mtmp->data)
                || (dmgtype(mtmp->data, AD_DGST) && Slow_digestion)))
            pline("Obviously %s doesn't like your taste.", mon_nam(mtmp));
    }
    return 1;
}

/* monster explodes in your face */
STATIC_OVL int
explmu(mtmp, mattk, ufound)
struct monst *mtmp;
struct attack *mattk;
boolean ufound;
{
    boolean kill_agr = TRUE;
    boolean not_affected;
    int tmp;

    if (mtmp->mcan)
        return 0;

    tmp = d((int) mattk->damn, (int) mattk->damd);
    not_affected = defended(mtmp, (int) mattk->adtyp);

    if (!ufound) {
        pline("%s explodes at a spot in %s!",
              canseemon(mtmp) ? Monnam(mtmp) : "It",
              levl[mtmp->mux][mtmp->muy].typ == WATER ? "empty water"
                                                      : "thin air");
    } else {
        hitmsg(mtmp, mattk);
    }

    switch (mattk->adtyp) {
    case AD_COLD:
    case AD_FIRE:
    case AD_ELEC:
    case AD_ACID:
        mon_explodes(mtmp, mattk);
        if (!DEADMONSTER(mtmp)) {
            kill_agr = FALSE; /* lifesaving? */
        }
        break;
    case AD_BLND:
        not_affected = resists_blnd(&youmonst);
        if (ufound && !not_affected) {
            /* sometimes you're affected even if it's invisible */
            if (mon_visible(mtmp) || (rnd(tmp /= 2) > u.ulevel)) {
                You("are blinded by a blast of light!");
                make_blinded((long) tmp, FALSE);
                if (!Blind)
                    Your1(vision_clears);
            } else if (flags.verbose) {
                You("get the impression it was not terribly bright.");
            }
        }
        /* exploding light can damage light haters whether
           they are blind or not */
        if (ufound
            && (hates_light(youmonst.data)
                || maybe_polyd(is_drow(youmonst.data),
                               Race_if(PM_DROW)))) {
            pline("Ow, that light hurts!");
            not_affected = FALSE;
            tmp = rnd(5);
            mdamageu(mtmp, tmp);
        }
        break;

    case AD_HALU:
        not_affected |= Blind || (u.umonnum == PM_BLACK_LIGHT
                                  || u.umonnum == PM_VIOLET_FUNGUS
                                  || dmgtype(youmonst.data, AD_STUN));
        if (ufound && !not_affected) {
            boolean chg;
            if (!Hallucination)
                You("are caught in a blast of kaleidoscopic light!");
            /* avoid hallucinating the black light as it dies */
            mondead(mtmp);    /* remove it from map now */
            kill_agr = FALSE; /* already killed (maybe lifesaved) */
            chg =
                make_hallucinated(HHallucination + (long) tmp, FALSE, 0L);
            You("%s.", chg ? "are freaked out" : "seem unaffected");
        }
        break;

    default:
        impossible("unknown exploder damage type %d", mattk->adtyp);
        break;
    }
    if (not_affected) {
        You("seem unaffected by it.");
        ugolemeffects((int) mattk->adtyp, tmp);
    }
    if (kill_agr && !DEADMONSTER(mtmp))
        mondead(mtmp);
    wake_nearto(mtmp->mx, mtmp->my, 7 * 7);
    return (!DEADMONSTER(mtmp)) ? 0 : 2;
}

/* monster uses a sonic-based attack against you */
STATIC_OVL int
screamu(mtmp, mattk)
struct monst *mtmp;
struct attack *mattk;
{
    int dmg = d((int) mattk->damn, (int) mattk->damd);

    switch (mattk->adtyp) {
    case AD_LOUD:
        /* Assumes that the hero has to hear the monster's
         * scream in order to be affected.
         * Only screams when a certain distance from our hero,
         * can see them, and has the available mspec.
         */
        if (distu(mtmp->mx, mtmp->my) > 128
            || !m_canseeu(mtmp) || mtmp->mspec_used)
            return FALSE;

        if (!mtmp->mcan && canseemon(mtmp) && Deaf) {
            pline("It looks as if %s is yelling at you.",
                  mon_nam(mtmp));
        } else if (!mtmp->mcan
                   && !canseemon(mtmp) && Deaf) {
            You("sense a disturbing vibration in the air.");
        } else if (mtmp->mcan
                   && canseemon(mtmp) && !Deaf) {
            pline("%s croaks hoarsely.", Monnam(mtmp));
        } else if (mtmp->mcan && !canseemon(mtmp) && !Deaf) {
            You_hear("a hoarse croak nearby.");
        }

        /* Set mspec_used */
        mtmp->mspec_used = mtmp->mspec_used + (rn2(6) + 5);

        if (mtmp->mcan || Deaf)
            return FALSE;

        if (m_canseeu(mtmp))
            pline("%s lets out a %s!", Monnam(mtmp),
                  mtmp->data == &mons[PM_NAZGUL] ? "bloodcurdling scream"
                                                 : "deafening roar");
        else if (u.usleep && m_canseeu(mtmp) && !Deaf)
                 unmul("You are frightened awake!");

        if (!Deaf && uarmh && uarmh->otyp == TOQUE) {
            pline("Your %s protects your ears from the sonic onslaught.",
                  helm_simple_name(uarmh));
            break;
        } else if (!Deaf && uarm
                   && Dragon_armor_to_scales(uarm) == CELESTIAL_DRAGON_SCALES) {
            pline("Your armor negates the lethal sonic assault.");
            break;
        } else if (Stun_resistance
                   || wielding_artifact(ART_TEMPEST)) {
            /* make_stunned() handles having stun resistance,
               but for feedback purposes, we specifically
               handle it here */
            You("are unaffected by %s scream.",
                s_suffix(mon_nam(mtmp)));
            break;
        } else {
            if (!Stunned)
                Your("mind reels from the noise!");
            else
                You("struggle to keep your balance.");
            make_stunned((HStun & TIMEOUT) + (long) dmg, TRUE);
            stop_occupation();
            mdamageu(mtmp, dmg);
        }

        /* being deaf won't protect objects in inventory,
           or being made of glass */
        if (!rn2(6))
            erode_armor(&youmonst, ERODE_FRACTURE);
        if (!rn2(5))
            erode_obj(uwep, (char *) 0, ERODE_FRACTURE, EF_DESTROY);
        if (!rn2(6))
            erode_obj(uswapwep, (char *) 0, ERODE_FRACTURE, EF_DESTROY);
        if (rn2(2))
            destroy_item(POTION_CLASS, AD_LOUD);
        if (!rn2(4))
            destroy_item(RING_CLASS, AD_LOUD);
        if (!rn2(4))
            destroy_item(TOOL_CLASS, AD_LOUD);
        if (!rn2(3))
            destroy_item(WAND_CLASS, AD_LOUD);

        if (u.umonnum == PM_GLASS_GOLEM) {
            You("shatter into a million pieces!");
            rehumanize();
            break;
        }
        break;
    default:
        dmg = 0;
        break;
    }

    return TRUE;
}

/* monster gazes at you */
int
gazemu(mtmp, mattk)
struct monst *mtmp;
struct attack *mattk;
{
    static const char *const reactions[] = {
        "confused",              /* [0] */
        "stunned",               /* [1] */
        "puzzled",   "dazzled",  /* [2,3] */
        "irritated", "inflamed", /* [4,5] */
        "tired",                 /* [6] */
        "dulled",                /* [7] */
        "chilly",                /* [8] */
        "lackluster",            /* [9] */
    };
    int react = -1;
    boolean cancelled = (mtmp->mcan != 0), already = FALSE;
    int dmg = d((int) mattk->damn, (int) mattk->damd);

    /* assumes that hero has to see monster's gaze in order to be
       affected, rather than monster just having to look at hero;
       when hallucinating, hero's brain doesn't register what
       it's seeing correctly so the gaze is usually ineffective
       [this could be taken a lot farther and select a gaze effect
       appropriate to what's currently being displayed, giving
       ordinary monsters a gaze attack when hero thinks he or she
       is facing a gazing creature, but let's not go that far...] */
    if (Hallucination && rn2(4))
        cancelled = TRUE;

    switch (mattk->adtyp) {
    case AD_STON:
        if (cancelled || !mtmp->mcansee) {
            if (!canseemon(mtmp))
                break; /* silently */
            pline("%s %s.", Monnam(mtmp),
                  (mtmp->data == &mons[PM_MEDUSA] && mtmp->mcan)
                      ? "doesn't look all that ugly"
                      : "gazes ineffectually");
            break;
        }
        if (Reflecting && couldsee(mtmp->mx, mtmp->my)
            && mtmp->data == &mons[PM_MEDUSA]) {
            /* hero has line of sight to Medusa and she's not blind */
            boolean useeit = canseemon(mtmp);

            if (useeit)
                (void) ureflects("%s gaze is reflected by your %s.",
                                 s_suffix(Monnam(mtmp)));
            if (dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) > 8) {
                if (useeit)
                    pline("%s reflection is too far away for %s to notice.",
                          s_suffix(Monnam(mtmp)), mhis(mtmp));
                break;
            }
            if (mon_reflects(
                    mtmp, !useeit ? (char *) 0
                                  : "The gaze is reflected away by %s %s!"))
                break;
            if (!m_canseeu(mtmp)) { /* probably you're invisible */
                if (useeit)
                    pline(
                      "%s doesn't seem to notice that %s gaze was reflected.",
                          Monnam(mtmp), mhis(mtmp));
                break;
            }
            if (!rn2(12)) {
                if (useeit)
                    pline("%s is turned to stone!", Monnam(mtmp));
                stoned = TRUE;
                killed(mtmp);
            } else {
                if (useeit)
                    pline("%s %s %s eyes from %s reflected gaze just in time!",
                          Monnam(mtmp), rn2(2) ? "shields" : "covers", mhis(mtmp), mhis(mtmp));
                break;
            }

            if (!DEADMONSTER(mtmp))
                break;
            return 2;
        }
        if (canseemon(mtmp)
            && couldsee(mtmp->mx, mtmp->my) && !rn2(3)) {
            You("meet %s petrifying gaze!", s_suffix(mon_nam(mtmp)));
            stop_occupation();
            if (Stone_resistance
                || (poly_when_stoned(youmonst.data)
                    && (polymon(PM_STONE_GOLEM)
                        || polymon(PM_PETRIFIED_ENT)))) {
                You("are unaffected by %s gaze.", s_suffix(mon_nam(mtmp)));
                break;
            }
            if (mtmp->data == &mons[PM_BEHOLDER]
                || mtmp->data == &mons[PM_TAL_GATH]) {
                /* The EotO can afford the player some protection when worn */
                if (ublindf
                    && ublindf->oartifact == ART_EYES_OF_THE_OVERWORLD) {
                    pline("%s partially protect you from %s petrifying gaze.  That hurts!",
                          An(bare_artifactname(ublindf)), s_suffix(mon_nam(mtmp)));
                    dmg = d(4, 6);
                    if (dmg)
                        mdamageu(mtmp, dmg);
                } else if (!Stoned) {
                    int kformat = KILLED_BY_AN;
                    const char *kname = mtmp->data->mname;

                    if (mtmp->data->geno & G_UNIQ) {
                        if (!type_is_pname(mtmp->data))
                            kname = the(kname);
                        kformat = KILLED_BY;
                    }
                    make_stoned(5L, (char *) 0, kformat, kname);
                    return 1;
                }
            } else if (mtmp->data == &mons[PM_MEDUSA]) {
                if (ublindf
                       && ublindf->oartifact == ART_EYES_OF_THE_OVERWORLD)
                    pline("%s gaze is too powerful for %s to resist!",
                          s_suffix(Monnam(mtmp)), bare_artifactname(ublindf));
                You("turn to stone...");
                killer.format = KILLED_BY;
                Strcpy(killer.name, mtmp->data->mname);
                done(STONING);
            }
        }
        break;
    case AD_CONF:
        if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my) && mtmp->mcansee
            && !mtmp->mspec_used && rn2(5)) {
            if (cancelled) {
                react = 0; /* "confused" */
                already = (mtmp->mconf != 0);
            } else if (ublindf
                       && ublindf->oartifact == ART_EYES_OF_THE_OVERWORLD) {
                if (!rn2(4))
                    pline("%s protect you from %s confusing gaze.",
                          An(bare_artifactname(ublindf)), s_suffix(mon_nam(mtmp)));
                break;
            } else {
                mtmp->mspec_used = mtmp->mspec_used + (dmg + rn2(6));
                if (!Confusion)
                    pline("%s gaze confuses you!", s_suffix(Monnam(mtmp)));
                else
                    You("are getting more and more confused.");
                make_confused(HConfusion + dmg, FALSE);
                stop_occupation();
            }
        }
        break;
    case AD_STUN:
        if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my) && mtmp->mcansee
            && !mtmp->mspec_used && rn2(5)) {
            if (cancelled) {
                react = 1; /* "stunned" */
                already = (mtmp->mstun != 0);
            } else if (ublindf
                       && ublindf->oartifact == ART_EYES_OF_THE_OVERWORLD) {
                if (!rn2(4))
                    pline("%s protect you from %s stunning gaze.",
                          An(bare_artifactname(ublindf)), s_suffix(mon_nam(mtmp)));
                break;
            } else {
                mtmp->mspec_used = mtmp->mspec_used + (dmg + rn2(6));
                pline("%s stares piercingly at you!", Monnam(mtmp));
                make_stunned((HStun & TIMEOUT) + (long) dmg, TRUE);
                stop_occupation();
            }
        }
        break;
    case AD_BLND:
        if (canseemon(mtmp) && !resists_blnd(&youmonst)
            && distu(mtmp->mx, mtmp->my) <= BOLT_LIM * BOLT_LIM) {
            if (cancelled) {
                react = rn1(2, 2); /* "puzzled" || "dazzled" */
                already = (mtmp->mcansee == 0);
                /* Archons gaze every round; we don't want cancelled ones
                   giving the "seems puzzled/dazzled" message that often */
                if (mtmp->mcan && mtmp->data == &mons[PM_ARCHON] && rn2(5))
                    react = -1;
            } else {
                You("are blinded by %s radiance!", s_suffix(mon_nam(mtmp)));
                make_blinded((long) dmg, FALSE);
                stop_occupation();
                /* not blind at this point implies you're wearing
                   the Eyes of the Overworld; make them block this
                   particular stun attack too */
                if (!Blind) {
                    Your1(vision_clears);
                } else {
                    long oldstun = (HStun & TIMEOUT), newstun = (long) rnd(3);

                    /* we don't want to increment stun duration every time
                       or sighted hero will become incapacitated */
                    make_stunned(max(oldstun, newstun), TRUE);
                }
            }
        }
        break;
    case AD_FIRE:
        if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my) && mtmp->mcansee
            && !mtmp->mspec_used && rn2(5)) {
            if (cancelled) {
                react = rn1(2, 4); /* "irritated" || "inflamed" */
            } else {
                int lev = (int) mtmp->m_lev;

                pline("%s attacks you with a fiery gaze!", Monnam(mtmp));
                stop_occupation();
                dmg = resist_reduce(dmg, FIRE_RES);
                if (how_resistant(FIRE_RES) == 100
                    || Underwater) {
                    shieldeff(u.ux, u.uy);
                    if (Underwater)
                        pline_The("fire quickly fizzles out.");
                    else
                        pline_The("fire feels mildly hot.");
                    monstseesu(M_SEEN_FIRE);
                    ugolemeffects(AD_FIRE, d(12, 6));
                    dmg = 0;
                }
                if (!Underwater) {
                    burn_away_slime();
                    if (lev > rn2(20))
                        (void) burnarmor(&youmonst);
                    if (lev > rn2(20))
                        destroy_item(SCROLL_CLASS, AD_FIRE);
                    if (lev > rn2(20))
                        destroy_item(POTION_CLASS, AD_FIRE);
                    if (lev > rn2(25))
                        destroy_item(SPBOOK_CLASS, AD_FIRE);
                }
                if (dmg)
                    mdamageu(mtmp, dmg);
            }
        }
        break;
    case AD_COLD:
        if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my) && mtmp->mcansee
            && !mtmp->mspec_used && rn2(5)) {
            if (cancelled) {
                react = 8; /* "chilly" */
            } else {
                int lev = (int) mtmp->m_lev;

                pline("%s attacks you with a chilling gaze!", Monnam(mtmp));
                stop_occupation();
                dmg = resist_reduce(dmg, COLD_RES);
                if (how_resistant(COLD_RES) == 100) {
                    shieldeff(u.ux, u.uy);
                    pline_The("chilling gaze feels mildly cool.");
                    monstseesu(M_SEEN_COLD);
                    ugolemeffects(AD_COLD, d(12, 6));
                    dmg = 0;
                }
                if (lev > rn2(20))
                    destroy_item(POTION_CLASS, AD_COLD);
                if (dmg)
                    mdamageu(mtmp, dmg);
            }
        }
        break;
    case AD_LUCK:
        if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my) && mtmp->mcansee
            && !mtmp->mspec_used && rn2(4) && !cancelled) {
            pline("%s glares ominously at you!", Monnam(mtmp));
            mtmp->mspec_used = mtmp->mspec_used + 3 + rn2(8);

            if (uwep && uwep->otyp == MIRROR && uwep->blessed) {
                pline("%s sees its own glare in your mirror.", Monnam(mtmp));
                pline("%s is cancelled!", Monnam(mtmp));
                mtmp->mcan = 1;
                monflee(mtmp, 0, FALSE, TRUE);
            } else {
                change_luck(-1);
                pline("You don't feel as lucky as before.");
            }
            stop_occupation();
        }
        break;
    /* Comment out the PM_BEHOLDER indef here so the below attack types function */
    case AD_SLEE:
        if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my)
            && mtmp->mcansee && multi >= 0 && !rn2(5)) {
            if (!cancelled)
                You("meet %s slumbering gaze.",
                    s_suffix(mon_nam(mtmp)));
            if (cancelled) {
                react = 6;                      /* "tired" */
                already = (mtmp->mfrozen != 0); /* can't happen... */
                break;
            } else if (how_resistant(SLEEP_RES) == 100) {
                You("yawn.");
                monstseesu(M_SEEN_SLEEP);
                break;
            } else if (ublindf
                       && ublindf->oartifact == ART_EYES_OF_THE_OVERWORLD) {
                if (!rn2(4))
                    pline("%s protect you from %s slumbering gaze.",
                          An(bare_artifactname(ublindf)), s_suffix(mon_nam(mtmp)));
                break;
            } else if (how_resistant(SLEEP_RES) < 100) {
                fall_asleep(-resist_reduce(dmg, SLEEP_RES), TRUE);
                pline("%s gaze makes you very sleepy...",
                      s_suffix(Monnam(mtmp)));
                break;
            }
        }
        break;
    case AD_SLOW:
        if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my)
            && mtmp->mcansee && !rn2(4)) {
            if (!cancelled)
                You("meet %s lethargic gaze.",
                    s_suffix(mon_nam(mtmp)));
            if (cancelled) {
                react = 7; /* "dulled" */
                already = (mtmp->mspeed == MSLOW);
                break;
            /* The EotO can afford the player some protection when worn */
            } else if (ublindf
                       && ublindf->oartifact == ART_EYES_OF_THE_OVERWORLD) {
                if (!rn2(4))
                    pline("%s protect you from %s lethargic gaze.",
                          An(bare_artifactname(ublindf)), s_suffix(mon_nam(mtmp)));
                break;
            } else if (!Slow && !(defended(&youmonst, AD_SLOW)
                                  || resists_slow(youmonst.data))) {
                u_slow_down();
                stop_occupation();
                break;
            }
        }
        break;
    /* Adding the parts here for disintegration and cancellation. The devteam probably
     * never bothered to add these, even though the Beholder has these two attacks.
     * Why you may ask? Because the Beholder was never enabled.
     */
    case AD_DISN:
        if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my)
            && mtmp->mcansee && multi >= 0 && !rn2(5)) {
            pline("%s turns %s towards you%s", Monnam(mtmp),
                  cancelled ? "an impotent leer" : "a destructive gaze",
                  cancelled ? "." : "!");
            if (cancelled) {
                break;
            } else if (how_resistant(DISINT_RES) == 100) {
                pline("You bask in its %s aura.", hcolor(NH_BLACK));
                monstseesu(M_SEEN_DISINT);
                stop_occupation();
                break;
            } else if (how_resistant(DISINT_RES) >= 50) {
                You("aren't disintegrated, but that really hurts!");
                dmg = resist_reduce(dmg, DISINT_RES);
                if (ublindf
                    && ublindf->oartifact == ART_EYES_OF_THE_OVERWORLD)
                    dmg /= 2;
                if (dmg)
                    mdamageu(mtmp, dmg);
                break;
            /* The EotO can afford the player some protection when worn */
            } else if (ublindf
                       && ublindf->oartifact == ART_EYES_OF_THE_OVERWORLD) {
                pline("%s partially protect you.  That stings!",
                      An(bare_artifactname(ublindf)));
                dmg /= 2;
                if (dmg)
                    mdamageu(mtmp, dmg);
                break;
            } else if (uarms) {
                /* destroy shield; other possessions are safe */
                (void) destroy_arm(uarms);
                break;
            } else if (uarm) {
                /* destroy suit; if present, cloak goes too */
                if (uarmc)
                    (void) destroy_arm(uarmc);
                (void) destroy_arm(uarm);
                break;
            } else {
                /* no shield or suit, you're dead; wipe out cloak
                 * and/or shirt in case of life-saving or bones */
                if (uarmc)
                    (void) destroy_arm(uarmc);
                if (uarmu)
                    (void) destroy_arm(uarmu);
                /* when killed by a disintegration beam, don't leave a corpse */
                u.ugrave_arise = -3;
                killer.format = NO_KILLER_PREFIX;
                Sprintf(killer.name, "disintegrated by %s",
                        an(mtmp->data->mname));
                done(DISINTEGRATED);
            }
        }
        break;
    case AD_CNCL:
        if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my)
            && mtmp->mcansee && !rn2(3)) {
            if (!cancelled)
                You("meet %s strange gaze.", s_suffix(mon_nam(mtmp)));
            if (cancelled) {
                react = 9; /* "lackluster" */
                already = (mtmp->mcan != 0);
                break;
            /* The EotO can afford the player some protection when worn */
            } else if (ublindf
                && ublindf->oartifact == ART_EYES_OF_THE_OVERWORLD) {
                pline("%s partially protect you from %s strange gaze.  Ouch!",
                      An(bare_artifactname(ublindf)), s_suffix(mon_nam(mtmp)));
                dmg = (dmg + 1) / 2;
                if (dmg)
                    mdamageu(mtmp, dmg);
                break;
            } else {
                (void) cancel_monst(&youmonst, (struct obj *) 0, FALSE, TRUE, FALSE);
                if (dmg)
                    mdamageu(mtmp, dmg);
                break;
            }
        }
        break;
    case AD_DETH:
        if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my)
            && mtmp->mcansee && rn2(4)) {
            int permdmg = 0;
            /* currently only Vecna has the gaze of death */
            if (mtmp && mtmp->data == &mons[PM_VECNA])
                You("meet %s deadly gaze!", s_suffix(mon_nam(mtmp)));
            if (Death_resistance || immune_death_magic(youmonst.data)) {
                /* Still does normal damage */
                shieldeff(u.ux, u.uy);
                monstseesu(M_SEEN_DEATH);
                You("are unaffected by death magic.");
                break;
            }
            switch (rn2(20)) {
            case 19:
            case 18:
            case 17:
                if (!Antimagic) {
                    killer.format = KILLED_BY_AN;
                    Strcpy(killer.name, "gaze of death");
                    ukiller = mtmp;
                    done(DIED);
                    dmg = 0;
                    break;
                }
                /*FALLTHRU*/
            default: /* case 16: ... case 2: */
                You_feel("your life force draining away...");
                permdmg = 1; /* actual damage done below */
                break;
            case 1:
            case 0:
                if (Antimagic) {
                    shieldeff(u.ux, u.uy);
                    monstseesu(M_SEEN_MAGR);
                }
                pline("Lucky for you, it didn't work!");
                dmg = 0;
                break;
            }

            if (permdmg) { /* Vecna's life force drain */
                int lowerlimit, *hpmax_p;
                /*
                 * Apply some of the damage to permanent hit points:
                 *  polymorphed         100% against poly'd hpmax
                 *  hpmax > 25*lvl      100% against normal hpmax
                 *  hpmax > 10*lvl  50..100%
                 *  hpmax >  5*lvl  25..75%
                 *  otherwise        0..50%
                 * Never reduces hpmax below 1 hit point per level.
                 */
                permdmg = rn2(dmg / 2 + 1);
                if (Upolyd || u.uhpmax > 25 * u.ulevel)
                    permdmg = dmg;
                else if (u.uhpmax > 10 * u.ulevel)
                    permdmg += dmg / 2;
                else if (u.uhpmax > 5 * u.ulevel)
                    permdmg += dmg / 4;

                if (Upolyd) {
                    hpmax_p = &u.mhmax;
                    /* [can't use youmonst.m_lev] */
                    lowerlimit = min((int) youmonst.data->mlevel, u.ulevel);
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
                context.botl = 1;
            }
            mdamageu(mtmp, dmg);
        }
        break;
    default:
        impossible("Gaze attack %d?", mattk->adtyp);
        break;
    }
    if (react >= 0) {
        if (Hallucination && rn2(3))
            react = rn2(SIZE(reactions));
        /* cancelled/hallucinatory feedback; monster might look "confused",
           "stunned",&c but we don't actually set corresponding attribute */
        pline("%s looks %s%s.", Monnam(mtmp),
              !rn2(3) ? "" : already ? "quite "
                                     : (!rn2(2) ? "a bit " : "somewhat "),
              reactions[react]);
    }
    return 0;
}

/* mtmp hits you for n points damage */
void
mdamageu(mtmp, n)
struct monst *mtmp;
int n;
{
    context.botl = 1;
    if (Upolyd) {
        u.mh -= n;
        showdamage(n);
        if (u.mh < 1)
            rehumanize();
    } else {
        u.uhp -= n;
        showdamage(n);
        if (u.uhp < 1)
            done_in_by(mtmp, DIED);
    }
}

/* returns 0 if seduction impossible,
 *         1 if fine,
 *         2 if wrong gender for nymph
 */
int
could_seduce(magr, mdef, mattk)
struct monst *magr, *mdef;
struct attack *mattk; /* non-Null: current attack; Null: general capability */
{
    struct permonst *pagr;
    boolean agrinvis, defperc;
    xchar genagr, gendef;
    int adtyp;

    if (is_animal(magr->data))
        return 0;
    if (magr == &youmonst) {
        pagr = youmonst.data;
        agrinvis = (Invis != 0);
        genagr = poly_gender();
    } else {
        pagr = magr->data;
        agrinvis = magr->minvis;
        genagr = gender(magr);
    }
    if (mdef == &youmonst) {
        defperc = (See_invisible != 0);
        gendef = poly_gender();
    } else {
        defperc = mon_prop(mdef, SEE_INVIS);
        gendef = gender(mdef);
    }

    adtyp = mattk ? mattk->adtyp
            : dmgtype(pagr, AD_SSEX) ? AD_SSEX
              : dmgtype(pagr, AD_SEDU) ? AD_SEDU
                : AD_PHYS;
    if (adtyp == AD_SSEX && !SYSOPT_SEDUCE)
        adtyp = AD_SEDU;

    if (agrinvis && !defperc && adtyp == AD_SEDU)
        return 0;

    /* nymphs have two attacks, one for steal-item damage and the other
       for seduction, both pass the could_seduce() test;
       incubi/succubi have three attacks, their claw attacks for damage
       don't pass the test */
    if ((pagr->mlet != S_NYMPH
         && pagr != &mons[PM_INCUBUS] && pagr != &mons[PM_SUCCUBUS])
        || (adtyp != AD_SEDU && adtyp != AD_SSEX && adtyp != AD_SITM))
        return 0;

    return (genagr == 1 - gendef) ? 1 : (pagr->mlet == S_NYMPH) ? 2 : 0;
}

/* returns 1 if monster teleported (or hero leaves monster's vicinity) */
int
doseduce(mon)
struct monst *mon;
{
    struct obj *ring, *nring;
    boolean fem = (mon->data == &mons[PM_SUCCUBUS]); /* otherwise incubus */
    boolean seewho, naked; /* True iff no armor */
    boolean purity = (u.ualign.type == A_LAWFUL && uarmg
                      && uarmg->oartifact == ART_GAUNTLETS_OF_PURITY);
    int attr_tot, tried_gloves = 0;
    char qbuf[QBUFSZ], Who[QBUFSZ];

    if (mon->mcan || mon->mspec_used) {
        pline("%s acts as though %s has got a %sheadache.", Monnam(mon),
              mhe(mon), mon->mcan ? "severe " : "");
        return 0;
    }
    if (unconscious() || purity) {
        pline("%s seems dismayed at your lack of response.", Monnam(mon));
        return 0;
    }
    seewho = canseemon(mon);
    if (!seewho)
        pline("Someone caresses you...");
    else
        You_feel("very attracted to %s.", mon_nam(mon));
    /* cache the seducer's name in a local buffer */
    Strcpy(Who, (!seewho ? (fem ? "She" : "He") : Monnam(mon)));

    /* if in the process of putting armor on or taking armor off,
       interrupt that activity now */
    (void) stop_donning((struct obj *) 0);
    /* don't try to take off gloves if cursed weapon blocks them */
    if (welded(uwep))
        tried_gloves = 1;

    for (ring = invent; ring; ring = nring) {
        nring = ring->nobj;
        if (ring->otyp != RIN_ADORNMENT)
            continue;
        if (fem) {
            if (ring->owornmask && uarmg) {
                /* don't take off worn ring if gloves are in the way */
                if (!tried_gloves++)
                    mayberem(mon, Who, uarmg, "gloves");
                if (uarmg)
                    continue; /* next ring might not be worn */
            }
            /* confirmation prompt when charisma is high bypassed if deaf */
            if (!Deaf && rn2(20) < ACURR(A_CHA)) {
                (void) safe_qbuf(qbuf, "\"That ",
                                 " looks pretty.  May I have it?\"", ring,
                                 xname, simpleonames, "ring");
                makeknown(RIN_ADORNMENT);
                if (yn(qbuf) == 'n')
                    continue;
            } else
                pline("%s decides she'd like %s, and takes it.",
                      Who, yname(ring));
            makeknown(RIN_ADORNMENT);
            /* might be in left or right ring slot or weapon/alt-wep/quiver */
            if (ring->owornmask)
                remove_worn_item(ring, FALSE);
            freeinv(ring);
            (void) mpickobj(mon, ring);
        } else {
            if (uleft && uright && uleft->otyp == RIN_ADORNMENT
                && uright->otyp == RIN_ADORNMENT)
                break;
            if (ring == uleft || ring == uright)
                continue;
            if (uarmg) {
                /* don't put on ring if gloves are in the way */
                if (!tried_gloves++)
                    mayberem(mon, Who, uarmg, "gloves");
                if (uarmg)
                    break; /* no point trying further rings */
            }
            /* confirmation prompt when charisma is high bypassed if deaf */
            if (!Deaf && rn2(20) < ACURR(A_CHA)) {
                (void) safe_qbuf(qbuf, "\"That ",
                                 " looks pretty.  Would you wear it for me?\"",
                                 ring, xname, simpleonames, "ring");
                makeknown(RIN_ADORNMENT);
                if (yn(qbuf) == 'n')
                    continue;
            } else {
                pline("%s decides you'd look prettier wearing %s,",
                      Who, yname(ring));
                pline("and puts it on your finger.");
            }
            makeknown(RIN_ADORNMENT);
            if (!uright) {
                pline("%s puts %s on your right %s.",
                      Who, the(xname(ring)), body_part(HAND));
                setworn(ring, RIGHT_RING);
            } else if (!uleft) {
                pline("%s puts %s on your left %s.",
                      Who, the(xname(ring)), body_part(HAND));
                setworn(ring, LEFT_RING);
            } else if (uright && uright->otyp != RIN_ADORNMENT) {
                /* note: the "replaces" message might be inaccurate if
                   hero's location changes and the process gets interrupted,
                   but trying to figure that out in advance in order to use
                   alternate wording is not worth the effort */
                pline("%s replaces %s with %s.",
                      Who, yname(uright), yname(ring));
                Ring_gone(uright);
                /* ring removal might cause loss of levitation which could
                   drop hero onto trap that transports hero somewhere else */
                if (u.utotype || distu(mon->mx, mon->my) > 2)
                    return 1;
                setworn(ring, RIGHT_RING);
            } else if (uleft && uleft->otyp != RIN_ADORNMENT) {
                /* see "replaces" note above */
                pline("%s replaces %s with %s.",
                      Who, yname(uleft), yname(ring));
                Ring_gone(uleft);
                if (u.utotype || distu(mon->mx, mon->my) > 2)
                    return 1;
                setworn(ring, LEFT_RING);
            } else
                impossible("ring replacement");
            Ring_on(ring);
            prinv((char *) 0, ring, 0L);
        }
    }

    naked = (!uarmc && !uarmf && (!uarmg || uarmg->oartifact == ART_HAND_OF_VECNA)
             && !uarms && !uarmh && !uarmu);
    pline("%s %s%s.", Who,
          Deaf ? "seems to murmur into your ear"
               : naked ? "murmurs sweet nothings into your ear"
                       : "murmurs in your ear",
          naked ? "" : ", while helping you undress");
    mayberem(mon, Who, uarmc, cloak_simple_name(uarmc));
    if (!uarmc)
        mayberem(mon, Who, uarm, suit_simple_name(uarm));
    mayberem(mon, Who, uarmf, "boots");
    if (!tried_gloves)
        mayberem(mon, Who, uarmg, "gloves");
    mayberem(mon, Who, uarms,
             uarms && is_bracer(uarms) ? "bracers" : "shield");
    mayberem(mon, Who, uarmh, helm_simple_name(uarmh));
    if (!uarmc && !uarm)
        mayberem(mon, Who, uarmu, "shirt");

    /* removing armor (levitation boots, or levitation ring to make
       room for adornment ring with incubus case) might result in the
       hero falling through a trap door or landing on a teleport trap
       and changing location, so hero might not be adjacent to seducer
       any more (mayberem() has its own adjacency test so we don't need
       to check after each potential removal) */
    if (u.utotype || distu(mon->mx, mon->my) > 2)
        return 1;

    if (uarm || uarmc) {
        if (!Deaf)
            verbalize("You're such a %s; I wish...",
                      flags.female ? "sweet lady" : "nice guy");
        else if (seewho)
            pline("%s appears to sigh.", Monnam(mon));
        /* else no regret message if can't see or hear seducer */

        if (!tele_restrict(mon))
            (void) rloc(mon, TRUE);
        return 1;
    }
    if (u.ualign.type == A_CHAOTIC)
        adjalign(1);

    /* by this point you have discovered mon's identity, blind or not... */
    pline("Time stands still while you and %s lie in each other's arms...",
          noit_mon_nam(mon));
    /* 3.6.1: a combined total for charisma plus intelligence of 35-1
       used to guarantee successful outcome; now total maxes out at 32
       as far as deciding what will happen; chance for bad outcome when
       Cha+Int is 32 or more is 2/35, a bit over 5.7% */
    attr_tot = ACURR(A_CHA) + ACURR(A_INT);
    if (rn2(35) > min(attr_tot, 32)) {
        /* Don't bother with mspec_used here... it didn't get tired! */
        pline("%s seems to have enjoyed it more than you...",
              noit_Monnam(mon));
        switch (rn2(5)) {
        case 0:
            You_feel("drained of energy.");
            u.uen = 0;
            u.uenmax -= rnd(Half_physical_damage ? 5 : 10);
            exercise(A_CON, FALSE);
            if (u.uenmax < 0)
                u.uenmax = 0;
            break;
        case 1:
            You("are down in the dumps.");
            (void) adjattrib(A_CON, -1, TRUE);
            exercise(A_CON, FALSE);
            context.botl = 1;
            break;
        case 2:
            Your("senses are dulled.");
            (void) adjattrib(A_WIS, -1, TRUE);
            exercise(A_WIS, FALSE);
            context.botl = 1;
            break;
        case 3:
            if (!resists_drli(&youmonst)) {
                You_feel("out of shape.");
                losexp("overexertion");
            } else {
                You("have a curious feeling...");
            }
            exercise(A_CON, FALSE);
            exercise(A_DEX, FALSE);
            exercise(A_WIS, FALSE);
            break;
        case 4: {
            int tmp;

            You_feel("exhausted.");
            exercise(A_STR, FALSE);
            tmp = rn1(10, 6);
            losehp(Maybe_Half_Phys(tmp), "exhaustion", KILLED_BY);
            break;
        } /* case 4 */
        } /* switch */
    } else {
        mon->mspec_used = rnd(100); /* monster is worn out */
        You("seem to have enjoyed it more than %s...", noit_mon_nam(mon));
        switch (rn2(5)) {
        case 0:
            You_feel("raised to your full potential.");
            exercise(A_CON, TRUE);
            u.uen = (u.uenmax += rnd(5));
            break;
        case 1:
            You_feel("good enough to do it again.");
            (void) adjattrib(A_CON, 1, TRUE);
            exercise(A_CON, TRUE);
            context.botl = 1;
            break;
        case 2:
            You("will always remember %s...", noit_mon_nam(mon));
            (void) adjattrib(A_WIS, 1, TRUE);
            exercise(A_WIS, TRUE);
            context.botl = 1;
            break;
        case 3:
            pline("That was a very educational experience.");
            pluslvl(FALSE);
            exercise(A_WIS, TRUE);
            break;
        case 4:
            You_feel("restored to health!");
            u.uhp = u.uhpmax;
            if (Upolyd)
                u.mh = u.mhmax;
            exercise(A_STR, TRUE);
            context.botl = 1;
            break;
        }
    }

    if (mon->mtame) { /* don't charge */
        ;
    } else if (rn2(20) < ACURR(A_CHA)) {
        pline("%s demands that you pay %s, but you refuse...",
              noit_Monnam(mon), noit_mhim(mon));
    } else if (u.umonnum == PM_LEPRECHAUN) {
        pline("%s tries to take your money, but fails...", noit_Monnam(mon));
    } else {
        long cost;
        long umoney = money_cnt(invent);

        if (umoney > (long) LARGEST_INT - 10L)
            cost = (long) rnd(LARGEST_INT) + 500L;
        else
            cost = (long) rnd((int) umoney + 10) + 500L;
        if (mon->mpeaceful) {
            cost /= 5L;
            if (!cost)
                cost = 1L;
        }
        if (cost > umoney)
            cost = umoney;
        if (!cost) {
            verbalize("It's on the house!");
        } else {
            pline("%s takes %ld %s for services rendered!", noit_Monnam(mon),
                  cost, currency(cost));
            money2mon(mon, cost);
            context.botl = 1;
        }
    }
    if (!rn2(25))
        mon->mcan = 1; /* monster is worn out */
    if (!tele_restrict(mon))
        (void) rloc(mon, TRUE);
    return 1;
}

STATIC_OVL void
mayberem(mon, seducer, obj, str)
struct monst *mon;
const char *seducer; /* only used for alternate message */
struct obj *obj;
const char *str;
{
    char qbuf[QBUFSZ];

    if (!obj || !obj->owornmask)
        return;
    /* removal of a previous item might have sent the hero elsewhere
       (loss of levitation that leads to landing on a transport trap) */
    if (u.utotype || distu(mon->mx, mon->my) > 2)
        return;
    /* the Hand of Vecna cannot be stolen, as it has 'merged' with
       the wearer */
    if (obj && obj->oartifact == ART_HAND_OF_VECNA)
        return;

    /* being deaf overrides confirmation prompt for high charisma */
    if (Deaf) {
        pline("%s takes off your %s.", seducer, str);
    } else if (rn2(20) < ACURR(A_CHA)) {
        Sprintf(qbuf, "\"Shall I remove your %s, %s?\"", str,
                (!rn2(2) ? "lover" : !rn2(2) ? "dear" : "sweetheart"));
        if (yn(qbuf) == 'n')
            return;
    } else {
        char hairbuf[BUFSZ];

        Sprintf(hairbuf, "let me run my fingers through your %s",
                body_part(HAIR));
        verbalize("Take off your %s; %s.", str,
                  (obj == uarm)
                     ? "let's get a little closer"
                     : (obj == uarmc || obj == uarms)
                        ? "it's in the way"
                        : (obj == uarmf)
                           ? "let me rub your feet"
                           : (obj == uarmg)
                              ? "they're too clumsy"
                              : (obj == uarmu)
                                 ? "let me massage you"
                                 /* obj == uarmh */
                                 : hairbuf);
    }
    remove_worn_item(obj, TRUE);
}

/* FIXME:
 *  sequencing issue:  a monster's attack might cause poly'd hero
 *  to revert to normal form.  The messages for passive counterattack
 *  would look better if they came before reverting form, but we need
 *  to know whether hero reverted in order to decide whether passive
 *  damage applies.
 */
STATIC_OVL int
passiveum(olduasmon, mtmp, mattk)
struct permonst *olduasmon;
struct monst *mtmp;
struct attack *mattk;
{
    int i, tmp;
    struct attack *oldu_mattk = 0;
    boolean mon_tempest_wield = (MON_WEP(mtmp)
                                 && MON_WEP(mtmp)->oartifact == ART_TEMPEST);

    if (uarm && Is_dragon_scaled_armor(uarm) && !rn2(3)) {
        int otyp = Dragon_armor_to_scales(uarm);

        switch (otyp) {
        case GREEN_DRAGON_SCALES:
            if (resists_poison(mtmp) || defended(mtmp, AD_DRST))
                break;
            if (rn2(20)) {
                /* Regular poison damage (95% chance) */
                if (!rn2(3)) {
                    if (canseemon(mtmp))
                        pline("%s staggers from the poison!", Monnam(mtmp));
                    if (damage_mon(mtmp, rnd(4), AD_DRST, TRUE)) {
                        /* damage_mon() killed the monster */
                        if (canseemon(mtmp))
                            pline("%s dies!", Monnam(mtmp));
                        xkilled(mtmp, XKILL_NOMSG);
                        if (!DEADMONSTER(mtmp))
                            return 1;
                        return 2;
                    }
                }
            } else {
                /* Fatal poison (5% chance) */
                if (canseemon(mtmp))
                    pline("%s is fatally poisoned!", Monnam(mtmp));
                xkilled(mtmp, XKILL_NOMSG);
                if (!DEADMONSTER(mtmp))
                    return 1;
                return 2;
            }
            break;
        case BLACK_DRAGON_SCALES:
            if (resists_disint(mtmp) || defended(mtmp, AD_DISN)) {
                if (canseemon(mtmp) && !rn2(3)) {
                    shieldeff(mtmp->mx, mtmp->my);
                    Your("armor does not appear to affect %s.",
                         mon_nam(mtmp));
                }
                break;
            } else if (mattk->aatyp == AT_WEAP || mattk->aatyp == AT_CLAW
                       || mattk->aatyp == AT_TUCH || mattk->aatyp == AT_KICK
                       || mattk->aatyp == AT_BITE || mattk->aatyp == AT_HUGS
                       || mattk->aatyp == AT_BUTT || mattk->aatyp == AT_STNG
                       || mattk->aatyp == AT_TENT) {
                /* if mtmp is wielding a weapon, that disintegrates first before
                   the actual monster. Same if mtmp is wearing gloves or boots */
                if (MON_WEP(mtmp) && !rn2(12)) {
                    if (canseemon(mtmp))
                        pline("%s %s is disintegrated!",
                              s_suffix(Monnam(mtmp)), xname(MON_WEP(mtmp)));
                    m_useup(mtmp, MON_WEP(mtmp));
                } else if ((mtmp->misc_worn_check & W_ARMF)
                           && mattk->aatyp == AT_KICK && !rn2(12)) {
                    if (canseemon(mtmp))
                        pline("%s %s are disintegrated!",
                              s_suffix(Monnam(mtmp)), xname(which_armor(mtmp, W_ARMF)));
                    m_useup(mtmp, which_armor(mtmp, W_ARMF));
                } else if ((mtmp->misc_worn_check & W_ARMG)
                           && (mattk->aatyp == AT_WEAP || mattk->aatyp == AT_CLAW
                               || mattk->aatyp == AT_TUCH)
                           && !MON_WEP(mtmp) && !rn2(12)
                           && !((which_armor(mtmp, W_ARMG))->oartifact == ART_DRAGONBANE)) {
                    if (canseemon(mtmp))
                        pline("%s %s are disintegrated!",
                              s_suffix(Monnam(mtmp)), xname(which_armor(mtmp, W_ARMG)));
                    m_useup(mtmp, which_armor(mtmp, W_ARMG));
                } else {
                    if (rn2(40)) {
                        if (canseemon(mtmp))
                            pline("%s partially disintegrates!", Monnam(mtmp));
                        if (damage_mon(mtmp, rnd(4), AD_DISN, TRUE)) {
                            /* damage_mon() killed the monster */
                            if (canseemon(mtmp))
                                pline("%s dies!", Monnam(mtmp));
                            xkilled(mtmp, XKILL_NOMSG);
                            if (!DEADMONSTER(mtmp))
                                return 1;
                            return 2;
                        }
                    } else {
                        if (canseemon(mtmp))
                            pline("%s is disintegrated completely!", Monnam(mtmp));
                        disint_mon_invent(mtmp);
                        if (is_rider(mtmp->data)) {
                            if (canseemon(mtmp)) {
                                pline("%s body reintegrates before your %s!",
                                      s_suffix(Monnam(mtmp)),
                                      (eyecount(youmonst.data) == 1)
                                         ? body_part(EYE)
                                         : makeplural(body_part(EYE)));
                                pline("%s resurrects!", Monnam(mtmp));
                            }
                            mtmp->mhp = mtmp->mhpmax;
                        } else {
                            xkilled(mtmp, XKILL_NOMSG | XKILL_NOCORPSE);
                        }
                        if (!DEADMONSTER(mtmp))
                            return 1;
                        return 2;
                    }
                }
            }
            break;
        case ORANGE_DRAGON_SCALES:
            if (!rn2(3)) {
                mon_adjust_speed(mtmp, -1, (struct obj *) 0);
            }
            break;
        case WHITE_DRAGON_SCALES:
            if (resists_cold(mtmp) || defended(mtmp, AD_COLD))
                break;
            if (rn2(20)) {
                if (!rn2(3)) {
                    if (canseemon(mtmp))
                        pline("%s flinches from the cold!", Monnam(mtmp));
                    if (damage_mon(mtmp, rnd(4), AD_COLD, TRUE)) {
                        /* damage_mon() killed the monster */
                        if (canseemon(mtmp))
                            pline("%s dies!", Monnam(mtmp));
                        xkilled(mtmp, XKILL_NOMSG);
                        if (!DEADMONSTER(mtmp))
                            return 1;
                        return 2;
                    }
                }
            } else {
                if (canseemon(mtmp))
                    pline("%s is frozen solid!", Monnam(mtmp));
                if (damage_mon(mtmp, d(6, 6), AD_COLD, TRUE)) {
                    /* damage_mon() killed the monster */
                    if (canseemon(mtmp))
                        pline("%s dies!", Monnam(mtmp));
                    xkilled(mtmp, XKILL_NOMSG);
                    if (!DEADMONSTER(mtmp))
                        return 1;
                    return 2;
                }
            }
            break;
        case RED_DRAGON_SCALES:
            if (resists_fire(mtmp) || defended(mtmp, AD_FIRE)
                || mon_underwater(mtmp))
                break;
            if (rn2(20)) {
                if (!rn2(3)) {
                    if (canseemon(mtmp))
                        pline("%s is burned!", Monnam(mtmp));
                    if (damage_mon(mtmp, rnd(4), AD_FIRE, TRUE)) {
                        /* damage_mon() killed the monster */
                        if (canseemon(mtmp))
                            pline("%s dies!", Monnam(mtmp));
                        xkilled(mtmp, XKILL_NOMSG);
                        if (!DEADMONSTER(mtmp))
                            return 1;
                        return 2;
                    }
                }
            } else {
                if (canseemon(mtmp))
                    pline("%s is severely burned!", Monnam(mtmp));
                if (damage_mon(mtmp, d(6, 6), AD_FIRE, TRUE)) {
                    /* damage_mon() killed the monster */
                    if (canseemon(mtmp))
                        pline("%s dies!", Monnam(mtmp));
                    xkilled(mtmp, XKILL_NOMSG);
                    if (!DEADMONSTER(mtmp))
                        return 1;
                    return 2;
                }
            }
            break;
        case YELLOW_DRAGON_SCALES:
            if (resists_acid(mtmp) || defended(mtmp, AD_ACID)
                || mon_underwater(mtmp))
                break;
            if (rn2(20)) {
                if (!rn2(3)) {
                    if (canseemon(mtmp))
                        pline("%s is seared!", Monnam(mtmp));
                    if (damage_mon(mtmp, rnd(4), AD_ACID, TRUE)) {
                        /* damage_mon() killed the monster */
                        if (canseemon(mtmp))
                            pline("%s dies!", Monnam(mtmp));
                        xkilled(mtmp, XKILL_NOMSG);
                        if (!DEADMONSTER(mtmp))
                            return 1;
                        return 2;
                    }
                }
            } else {
                if (canseemon(mtmp))
                    pline("%s is critically seared!", Monnam(mtmp));
                if (damage_mon(mtmp, d(6, 6), AD_ACID, TRUE)) {
                    /* damage_mon() killed the monster */
                    if (canseemon(mtmp))
                        pline("%s dies!", Monnam(mtmp));
                    xkilled(mtmp, XKILL_NOMSG);
                    if (!DEADMONSTER(mtmp))
                        return 1;
                    return 2;
                }
            }
            break;
        case GRAY_DRAGON_SCALES:
            if (!rn2(6))
                (void) cancel_monst(mtmp, (struct obj *) 0, TRUE, TRUE, FALSE);
            break;
        default: /* all other types of armor, just pass on through */
            break;
        }
    }

    if (uarmg) {
        switch (uarmg->otyp) {
        case GLOVES:
            if (!is_dragon(mtmp->data))
                break;
            if (!rn2(3) && uarmg->oartifact == ART_DRAGONBANE) {
                if (canseemon(mtmp))
                    pline("Dragonbane sears %s scaly hide!", s_suffix(mon_nam(mtmp)));
                if (damage_mon(mtmp, rnd(6) + 2, AD_PHYS, TRUE)) {
                    /* damage_mon() killed the monster */
                    if (canseemon(mtmp))
                        pline("Dragonbane's power overwhelms %s!", mon_nam(mtmp));
                    pline("%s dies!", Monnam(mtmp));
                    xkilled(mtmp, XKILL_NOMSG);
                    if (!DEADMONSTER(mtmp))
                        return 1;
                    return 2;
                }
            }
            break;
        default: /* all other types of armor, just pass on through */
            break;
        }
    }

    /*
     * mattk      == mtmp's attack that hit you;
     * oldu_mattk == your passive counterattack (even if mtmp's attack
     *               has already caused you to revert to normal form).
     */
    for (i = 0; !oldu_mattk; i++) {
        if (i >= NATTK)
            return 1;

        if (olduasmon->mattk[i].aatyp == AT_NONE
            || olduasmon->mattk[i].aatyp == AT_BOOM)
            oldu_mattk = &olduasmon->mattk[i];
    }
    if (oldu_mattk->damn)
        tmp = d((int) oldu_mattk->damn, (int) oldu_mattk->damd);
    else if (oldu_mattk->damd) {
        if (is_dragon(olduasmon))
            tmp = d((int) olduasmon->mlevel + 1, (int) oldu_mattk->damd) / 3;
        else
            tmp = d((int) olduasmon->mlevel + 1, (int) oldu_mattk->damd);
    } else
        tmp = 0;

    /* These affect the enemy even if you were "killed" (rehumanized) */
    switch (oldu_mattk->adtyp) {
    case AD_ACID:
        if (!rn2(2)) {
            if (youmonst.data == &mons[PM_YELLOW_DRAGON]) {
                pline("%s is seared by your acidic hide!", Monnam(mtmp));
            } else {
                pline("%s is splashed by %s%s!", Monnam(mtmp),
                      /* temporary? hack for sequencing issue:  "your acid"
                         looks strange coming immediately after player has
                         been told that hero has reverted to normal form */
                      !Upolyd ? "" : "your ", hliquid("acid"));
            }
            if (resists_acid(mtmp) || defended(mtmp, AD_ACID)
                || mon_underwater(mtmp)) {
                pline("%s is not affected.", Monnam(mtmp));
                tmp = 0;
            }
        } else
            tmp = 0;
        if (!Underwater) {
            if (!rn2(30))
                erode_armor(mtmp, ERODE_CORRODE);
            if (!rn2(6))
                acid_damage(MON_WEP(mtmp));
        }
        goto assess_dmg;
    case AD_DISN: {
        int chance = (youmonst.data == &mons[PM_ANTIMATTER_VORTEX] ? !rn2(3) : !rn2(6));
        if (resists_disint(mtmp) || defended(mtmp, AD_DISN)) {
            if (canseemon(mtmp) && !rn2(3)) {
                shieldeff(mtmp->mx, mtmp->my);
                Your("deadly %s does not appear to affect %s.",
                     youmonst.data == &mons[PM_ANTIMATTER_VORTEX]
                         ? "form" : "hide", mon_nam(mtmp));
            }
        } else if (mattk->aatyp == AT_WEAP || mattk->aatyp == AT_CLAW
                   || mattk->aatyp == AT_TUCH || mattk->aatyp == AT_KICK
                   || mattk->aatyp == AT_BITE || mattk->aatyp == AT_HUGS
                   || mattk->aatyp == AT_BUTT || mattk->aatyp == AT_STNG
                   || mattk->aatyp == AT_TENT) {
            /* if mtmp is wielding a weapon, that disintegrates first before
               the actual monster. Same if mtmp is wearing gloves or boots */
            if (MON_WEP(mtmp) && chance) {
                if (canseemon(mtmp))
                    pline("%s %s is disintegrated!",
                          s_suffix(Monnam(mtmp)), xname(MON_WEP(mtmp)));
                m_useup(mtmp, MON_WEP(mtmp));
            } else if ((mtmp->misc_worn_check & W_ARMF)
                       && mattk->aatyp == AT_KICK && chance) {
                if (canseemon(mtmp))
                    pline("%s %s are disintegrated!",
                          s_suffix(Monnam(mtmp)), xname(which_armor(mtmp, W_ARMF)));
                m_useup(mtmp, which_armor(mtmp, W_ARMF));
            } else if ((mtmp->misc_worn_check & W_ARMG)
                       && (mattk->aatyp == AT_WEAP || mattk->aatyp == AT_CLAW
                           || mattk->aatyp == AT_TUCH)
                       && !MON_WEP(mtmp) && chance) {
                if (canseemon(mtmp))
                    pline("%s %s are disintegrated!",
                          s_suffix(Monnam(mtmp)), xname(which_armor(mtmp, W_ARMG)));
                m_useup(mtmp, which_armor(mtmp, W_ARMG));
            } else {
                if (youmonst.data == &mons[PM_ANTIMATTER_VORTEX] ? rn2(10) : rn2(20)) {
                    if (canseemon(mtmp))
                        Your("%s partially disintegrates %s!",
                             youmonst.data == &mons[PM_ANTIMATTER_VORTEX]
                                 ? "form" : "hide", mon_nam(mtmp));
                    tmp = rn2(6) + 1;
                    goto assess_dmg;
                } else {
                    if (canseemon(mtmp)) {
                        Your("deadly %s disintegrates %s!",
                             youmonst.data == &mons[PM_ANTIMATTER_VORTEX]
                                 ? "form" : "hide", mon_nam(mtmp));
                        disint_mon_invent(mtmp);
                        if (is_rider(mtmp->data)) {
                            if (canseemon(mtmp)) {
                                pline("%s body reintegrates before your %s!",
                                      s_suffix(Monnam(mtmp)),
                                      (eyecount(youmonst.data) == 1)
                                         ? body_part(EYE)
                                         : makeplural(body_part(EYE)));
                                pline("%s resurrects!", Monnam(mtmp));
                            }
                            mtmp->mhp = mtmp->mhpmax;
                        } else {
                            xkilled(mtmp, XKILL_NOMSG | XKILL_NOCORPSE);
                        }
                        if (!DEADMONSTER(mtmp))
                            return 1;
                        return 2;
                    }
                }
            }
        }
        break;
    }
    case AD_STON: { /* cockatrice */
        long protector = attk_protection((int) mattk->aatyp),
             wornitems = mtmp->misc_worn_check;

        /* wielded weapon gives same protection as gloves here */
        if (MON_WEP(mtmp) != 0)
            wornitems |= W_ARMG;

        if (!(resists_ston(mtmp) || defended(mtmp, AD_STON))
            && (protector == 0L
                || (protector != ~0L
                    && (wornitems & protector) != protector))) {
            if (poly_when_stoned(mtmp->data)) {
                mon_to_stone(mtmp);
                return 1;
            }
            pline("%s turns to stone!", Monnam(mtmp));
            stoned = 1;
            mtmp->mstone = 0; /* end any lingering timer */
            xkilled(mtmp, XKILL_NOMSG);
            if (!DEADMONSTER(mtmp))
                return 1;
            return 2;
        }
        return 1;
    }
    case AD_ENCH: /* KMH -- remove enchantment (disenchanter) */
        if (mon_currwep) {
            /* by_you==True: passive counterattack to hero's action
               is hero's fault */
            (void) drain_item(mon_currwep, TRUE);
            /* No message */
        }
        return 1;
    case AD_CNCL:
        if (!rn2(6)) {
            (void) cancel_monst(mtmp, (struct obj *) 0, TRUE, TRUE, FALSE);
        }
        return 1;
    case AD_SLIM:
        if (!rn2(3)) {
            Your("slime splashes onto %s!", mon_nam(mtmp));
            if (flaming(mtmp->data)) {
                pline_The("slime burns away!");
            } else if (slimeproof(mtmp->data)) {
                pline("%s is unaffected.", Monnam(mtmp));
            } else if (!rn2(4) && !slimeproof(mtmp->data)) {
                if (!munslime(mtmp, FALSE) && !DEADMONSTER(mtmp)) {
                    if (newcham(mtmp, &mons[PM_GREEN_SLIME], FALSE,
                                (boolean) (canseemon(mtmp))))
                    mtmp->mstrategy &= ~STRAT_WAITFORU;
                }
            }
        }
        return 1;
        break;
    default:
        break;
    }
    if (!Upolyd)
        return 1;

    /* These affect the enemy only if you are still a monster */
    if (rn2(3))
        switch (oldu_mattk->adtyp) {
        case AD_PHYS:
            if (oldu_mattk->aatyp == AT_BOOM) {
                You("explode!");
                /* KMH, balance patch -- this is okay with unchanging */
                rehumanize();
                goto assess_dmg;
            }
            break;
        case AD_PLYS: /* Floating eye */
            if (tmp > 127)
                tmp = 127;
            if (u.umonnum == PM_FLOATING_EYE) {
                if (!rn2(4))
                    tmp = 127;
                if (mtmp->mcansee && haseyes(mtmp->data) && rn2(3)
                    && (mon_prop(mtmp, SEE_INVIS) || !Invis)) {
                    if (Blind)
                        pline("As a blind %s, you cannot defend yourself.",
                              youmonst.data->mname);
                    else {
                        if (mon_reflects(mtmp,
                                         "Your gaze is reflected by %s %s."))
                            return 1;
                        if (has_free_action(mtmp)) {
                            pline("%s stiffens momentarily.", Monnam(mtmp));
                            return 1;
                        } else {
                            pline("%s is frozen by your gaze!", Monnam(mtmp));
                            paralyze_monst(mtmp, tmp);
                        }
                        return 3;
                    }
                }
            } else { /* gelatinous cube */
                if (has_free_action(mtmp)) {
                    pline("%s stiffens momentarily.", Monnam(mtmp));
                    return 1;
                } else {
                    pline("%s is frozen by you.", Monnam(mtmp));
                    paralyze_monst(mtmp, tmp);
                }
                return 3;
            }
            return 1;
        case AD_COLD: /* Brown mold or blue jelly */
            if (resists_cold(mtmp) || defended(mtmp, AD_COLD)) {
                shieldeff(mtmp->mx, mtmp->my);
                pline("%s is mildly chilly.", Monnam(mtmp));
                golemeffects(mtmp, AD_COLD, tmp);
                tmp = 0;
                break;
            }
            pline("%s is suddenly very cold!", Monnam(mtmp));
            u.mh += tmp / 2;
            if (u.mhmax < u.mh)
                u.mhmax = u.mh;
            if (u.mhmax > ((youmonst.data->mlevel + 1) * 8)
                && (youmonst.data->mlet == S_JELLY
                    || youmonst.data->mlet == S_FUNGUS))
                (void) split_mon(&youmonst, mtmp);
            break;
        case AD_STUN: /* Yellow mold */
            if (resists_stun(mtmp->data)
                || defended(mtmp, AD_STUN) || mon_tempest_wield) {
                ; /* immune */
                break;
            }
            if (!mtmp->mstun) {
                mtmp->mstun = 1;
                pline("%s %s.", Monnam(mtmp),
                      makeplural(stagger(mtmp->data, "stagger")));
            }
            tmp = 0;
            break;
        case AD_FIRE: /* Red mold */
            if (resists_fire(mtmp) || defended(mtmp, AD_FIRE)
                || mon_underwater(mtmp)) {
                shieldeff(mtmp->mx, mtmp->my);
                pline("%s is mildly warm.", Monnam(mtmp));
                golemeffects(mtmp, AD_FIRE, tmp);
                tmp = 0;
                break;
            }
            pline("%s is suddenly very hot!", Monnam(mtmp));
            break;
        case AD_ELEC:
            if (resists_elec(mtmp) || defended(mtmp, AD_ELEC)) {
                shieldeff(mtmp->mx, mtmp->my);
                pline("%s is slightly tingled.", Monnam(mtmp));
                golemeffects(mtmp, AD_ELEC, tmp);
                tmp = 0;
                break;
            }
            pline("%s is jolted with your electricity!", Monnam(mtmp));
            break;
        case AD_DRST:
            if (resists_poison(mtmp) || defended(mtmp, AD_DRST)) {
                if (canseemon(mtmp) && !rn2(5))
                    pline("%s is unaffected by your poisonous hide.", Monnam(mtmp));
                tmp = 0;
                break;
            }
            /* Poison effect happens regardless of visibility */
            if (rn2(20)) {
                /* Regular poison damage */
                if (canseemon(mtmp))
                    pline("%s is poisoned!", Monnam(mtmp));
                /* Let damage_mon() handle the actual damage at assess_dmg */
            } else {
                /* Deadly poison - instant death */
                if (canseemon(mtmp))
                    Your("poisonous hide was deadly...");
                xkilled(mtmp, XKILL_NOMSG);
                if (!DEADMONSTER(mtmp))
                    return 1;
                return 2;
            }
            break;
        case AD_SLOW:
            if (!rn2(3)) {
                mon_adjust_speed(mtmp, -1, (struct obj *) 0);
            }
            tmp = 0;
            break;
        case AD_DISE: /* gray fungus */
            if (resists_sick(mtmp) || defended(mtmp, AD_DISE)) {
                if (canseemon(mtmp))
                    pline("%s resists infection.", Monnam(mtmp));
                tmp = 0;
                break;
            } else {
                if (mtmp->mdiseasetime)
                    mtmp->mdiseasetime -= rnd(3);
                else
                    mtmp->mdiseasetime = rn1(9, 6);
                if (canseemon(mtmp))
                    pline("%s looks %s.", Monnam(mtmp),
                          mtmp->mdiseased ? "even worse" : "diseased");
                mtmp->mdiseased = 1;
                mtmp->mdiseabyu = TRUE;
            }
            break;
        case AD_SLEE: /* black fungus */
            if (sleep_monst(mtmp, rn2(3) + 8, -1)) {
                if (canseemon(mtmp))
                    pline("%s loses consciousness.", Monnam(mtmp));
                slept_monst(mtmp);
            }
            break;
        default:
            tmp = 0;
            break;
        }
    else
        tmp = 0;

 assess_dmg:
    if (damage_mon(mtmp, tmp, youmonst.data->mattk[i].adtyp, TRUE)) {
        pline("%s dies!", Monnam(mtmp));
        xkilled(mtmp, XKILL_NOMSG);
        if (!DEADMONSTER(mtmp))
            return 1;
        return 2;
    }
    return 1;
}

struct monst *
cloneu()
{
    struct monst *mon;
    int mndx = monsndx(youmonst.data);

    if (u.mh <= 1)
        return (struct monst *) 0;
    if (mvitals[mndx].mvflags & G_EXTINCT)
        return (struct monst *) 0;
    mon = makemon(youmonst.data, u.ux, u.uy, NO_MINVENT | MM_EDOG);
    if (!mon)
        return NULL;
    mon->mcloned = 1;
    mon = christen_monst(mon, plname);
    initedog(mon, TRUE);
    mon->m_lev = youmonst.data->mlevel;
    mon->mhpmax = u.mhmax;
    mon->mhp = u.mh / 2;
    u.mh -= mon->mhp;
    context.botl = 1;
    return mon;
}

/* Given an attacking monster and the attack type it's currently attacking with,
 * return a bitmask of W_ARM* values representing the gear slots that might be
 * coming in contact with the defender.
 * Intended to return worn items. Will not return W_WEP.
 * Does not check to see whether slots are ineligible due to being covered by
 * some other piece of gear. Usually special_dmgval() will handle that.
 */
long
attack_contact_slots(magr, aatyp)
struct monst *magr;
int aatyp;
{
    struct obj* mwep = (magr == &youmonst ? uwep : magr->mw);
    if (aatyp == AT_CLAW || aatyp == AT_TUCH || (aatyp == AT_WEAP && !mwep)
        || (aatyp == AT_HUGS && hug_throttles(magr->data))) {
        /* attack with hands; gloves and rings might touch */
        return W_ARMG | W_RINGL | W_RINGR;
    }
    if (aatyp == AT_HUGS && !hug_throttles(magr->data)) {
        /* bear hug which is not a strangling attack; gloves and rings might
         * touch, but also all torso slots */
        return W_ARMG | W_RINGL | W_RINGR | W_ARMC | W_ARM | W_ARMU;
    }
    if (aatyp == AT_KICK) {
        return W_ARMF;
    }
    if (aatyp == AT_BUTT) {
        return W_ARMH;
    }
    return 0;
}

/*mhitu.c*/
