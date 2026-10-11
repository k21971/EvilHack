/* NetHack 5.0	attrib.c	$NHDT-Date: 1781973041 2026/06/20 16:30:41 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.142 $ */
/*      Copyright 1988, 1989, 1990, 1992, M. Stephenson           */
/* NetHack may be freely redistributed.  See license for details. */

/*  attribute modification routines. */

#include "hack.h"

/* part of the output on gain or loss of attribute */
static const char
    *const plusattr[] = { "strong", "smart", "wise",
                          "agile",  "tough", "charismatic" },
    *const minusattr[] = { "weak",    "stupid",
                           "foolish", "clumsy",
                           "fragile", "repulsive" };
/* also used by enlightenment in insight.c for non-abbreviated status info */
extern const char *const attrname[6];

const char
    *const attrname[] = { "strength", "intelligence", "wisdom",
                          "dexterity", "constitution", "charisma" };

static const struct innate {
    schar ulevel;
    long *ability;
    const char *gainstr, *losestr;
} arc_abil[] = { { 1, &(HSearching), "", "" },
                 { 5, &(HStealth), "stealthy", "" },
                 { 10, &(HFast), "quick", "slow" },
                 { 0, 0, 0, 0 } },

  bar_abil[] = { { 1, &(HPoison_resistance), "", "" },
                 { 7, &(HFast), "quick", "slow" },
                 { 15, &(HStealth), "stealthy", "" },
                 { 0, 0, 0, 0 } },

  cav_abil[] = { { 7, &(HFast), "quick", "slow" },
                 { 15, &(HWarning), "sensitive", "" },
                 { 0, 0, 0, 0 } },

  con_abil[] = { { 1, &(HSick_resistance), "", "" },
                 { 7, &(HPoison_resistance), "healthy", "" },
                 { 20, &(HSearching), "perceptive", "unaware" },
                 { 0, 0, 0, 0 } },

  dru_abil[] = { { 1, &(HStealth), "", "" },
                 { 7, &(HSleep_resistance), "awake", "tired" },
                 { 10, &(HSearching), "perceptive", "unaware" },
                 { 14, &(HPasses_trees), "less solid", "more solid" },
                 { 0, 0, 0, 0 } },

  hea_abil[] = { { 3, &(HPoison_resistance), "healthy", "" },
                 { 15, &(HSick_resistance), "hale", "" },
                 { 0, 0, 0, 0 } },

  inf_abil[] = { { 1, &(HFire_resistance), "", "" },
                 { 15, &(HWarning), "sensitive", "" },
                 { 20, &(HShock_resistance), "insulated", "conductive" },
                 { 0, 0, 0, 0 } },

  kni_abil[] = { { 7, &(HFast), "quick", "slow" }, { 0, 0, 0, 0 } },

  mon_abil[] = { { 1, &(HFast), "", "" },
                 { 1, &(HSleep_resistance), "", "" },
                 { 1, &(HSee_invisible), "", "" },
                 { 3, &(HPoison_resistance), "healthy", "" },
                 { 5, &(HStealth), "stealthy", "" },
                 { 7, &(HWarning), "sensitive", "" },
                 { 9, &(HSearching), "perceptive", "unaware" },
                 { 11, &(HFire_resistance), "cool", "warmer" },
                 { 13, &(HCold_resistance), "warm", "cooler" },
                 { 15, &(HShock_resistance), "insulated", "conductive" },
                 { 17, &(HTeleport_control), "controlled", "uncontrolled" },
                 { 20, &(HTelepat), "a strange mental acuity",
                   "mentally dulled" },
                 { 23, &(HWwalking), "unsinkable", "like you might sink" },
                 { 25, &(HStone_resistance), "limber", "stiff" },
                 { 27, &(HDisint_resistance), "firm", "less firm" },
                 { 30, &(HSick_resistance), "hale", "" },
                 { 0, 0, 0, 0 } },

  pri_abil[] = { { 15, &(HWarning), "sensitive", "" },
                 { 20, &(HFire_resistance), "cool", "warmer" },
                 { 0, 0, 0, 0 } },

  ran_abil[] = { { 1, &(HSearching), "", "" },
                 { 7, &(HStealth), "stealthy", "" },
                 { 15, &(HSee_invisible), "", "" },
                 { 0, 0, 0, 0 } },

  rog_abil[] = { { 1, &(HStealth), "", "" },
                 { 10, &(HSearching), "perceptive", "unaware" },
                 { 0, 0, 0, 0 } },

  sam_abil[] = { { 1, &(HFast), "", "" },
                 { 15, &(HStealth), "stealthy", "" },
                 { 0, 0, 0, 0 } },

  tou_abil[] = { { 10, &(HSearching), "perceptive", "unaware" },
                 { 20, &(HPoison_resistance), "hardy", "" },
                 { 0, 0, 0, 0 } },

  val_abil[] = { { 1, &(HCold_resistance), "", "" },
                 { 3, &(HStealth), "stealthy", "" },
                 { 7, &(HFast), "quick", "slow" },
                 { 0, 0, 0, 0 } },

  wiz_abil[] = { { 15, &(HWarning), "sensitive", "" },
                 { 17, &(HTeleport_control), "controlled", "uncontrolled" },
                 { 0, 0, 0, 0 } },

  /* Intrinsics conferred by race */
  dwa_abil[] = { { 1, &HInfravision, "", "" },
                 { 0, 0, 0, 0 } },

  elf_abil[] = { { 1, &HInfravision, "", "" },
                 { 4, &HSleep_resistance, "awake", "tired" },
                 { 9, &HSearching, "perceptive", "unaware" },
                 { 0, 0, 0, 0 } },

  gno_abil[] = { { 1, &HInfravision, "", "" },
                 { 0, 0, 0, 0 } },

  orc_abil[] = { { 1, &HInfravision, "", "" },
                 { 1, &HPoison_resistance, "", "" },
                 { 0, 0, 0, 0 } },

  /* giants had it FROMFORM - make it FROMRACE like other races */
  gia_abil[] = { { 1, &HInfravision, "", "" },
                 { 1, &HAggravate_monster, "", "" },
                 { 12, &HRegeneration, "resilient", "less resilient" },
                 { 0, 0, 0, 0 } },

  hob_abil[] = { { 1, &HFood_sense, "", "" },
                 { 1, &HHunger, "", "" },
                 { 4, &HFast, "quick", "slow" },
                 { 7, &HSearching, "perceptive", "unaware" },
                 { 0, 0, 0, 0 } },

  cen_abil[] = { { 1, &HFast, "", "" },
                 /* use EJumping here, otherwise centaurs would only
                    be able to jump the same way as knights */
                 { 5, &EJumping, "light on your hooves", "weighted down" },
                 { 10, &HWarning, "sensitive", "" },
                 { 0, 0, 0, 0 } },

  ill_abil[] = { { 1, &HInfravision, "", "" },
                 { 1, &HTelepat, "", "" },
                 { 1, &HPsychic_resistance, "", "" },
                 { 12, &HFlying, "lighter than air", "gravity's pull" },
                 { 0, 0, 0, 0 } },

  trt_abil[] = { { 1, &HSwimming, "", "" },
                 { 5, &HWarning, "sensitive", "" },
                 { 12, &HRegeneration, "resilient", "less resilient" },
                 { 0, 0, 0, 0 } },

  dro_abil[] = { { 1, &HUltravision, "", "" },
                 { 1, &HSleep_resistance, "", "" },
                 { 5, &HPoison_resistance, "hardy", "" },
                 { 9, &HSearching, "perceptive", "unaware" },
                 { 0, 0, 0, 0 } },

  dra_abil[] = { { 1, &HInfravision, "", "" },
                 { 1, &HSick_resistance, "", "" },
                 { 1, &HCold_resistance, "", "" },
                 { 1, &HSleep_resistance, "", "" },
                 { 1, &HPoison_resistance, "", "" },
                 { 1, &HLycan_resistance, "", "" },
                 { 1, &HAggravate_monster, "", "" },
                 { 1, &HLifesaved, "", "" }, /* only a random number of times */
                 { 0, 0, 0, 0 } },

  vam_abil[] = { { 1, &HInfravision, "", "" },
                 { 1, &HSick_resistance, "", "" },
                 { 1, &HCold_resistance, "", "" },
                 { 1, &HSleep_resistance, "", "" },
                 { 1, &HPoison_resistance, "", "" },
                 { 1, &HLycan_resistance, "", "" },
                 { 12, &HFlying, "lighter than air", "gravity's pull" },
                 { 20, &HRegeneration, "resilient", "less resilient" },
                 { 0, 0, 0, 0 } },

  amr_abil[] = { { 1, &HInfravision, "", "" },
                 { 1, &HSee_invisible, "", "" },
                 { 1, &HPoison_resistance, "", "" },
                 { 4, &HAntimagic, "resistant to magic",
                   "vulnerable to magic" },
                 { 7, &HSleep_resistance, "awake", "tired" },
                 { 12, &HCold_resistance, "warm", "cooler" },
                 { 20, &HShock_resistance, "insulated", "conductive" },
                 { 0, 0, 0, 0 } },

  /* remove drain res and flying - they are FROMFORM.
     add sick res to remain consistent with previous behavior */
  dem_abil[] = { { 1, &HInfravision, "", "" },
                 { 1, &HFire_resistance, "", "" },
                 { 1, &HPoison_resistance, "", "" },
                 { 1, &HSee_invisible, "", "" },
                 { 1, &HSick_resistance, "hale", "" },
                 /* also inediate */
                 { 0, 0, 0, 0 } },

  hum_abil[] = { { 0, 0, 0, 0 } };

staticfn void exerper(void);
staticfn int rnd_attr(void);
staticfn int init_attr_role_redist(int, boolean);
staticfn void postadjabil(long *) NONNULLARG1;
staticfn const struct innate *role_abil(int);
staticfn const struct innate *check_innate_abil(long *, long);
staticfn int innately(long *);
staticfn long *aasimar_pen_prop(int);
staticfn int aasimar_pen_minlevel(int);
staticfn void aasimar_feedback(int, boolean);
staticfn void aasimar_changed(int, boolean);
staticfn boolean aasimar_penalized_now(int);
staticfn void apply_aasimar_penalty(int, boolean);
staticfn int abuse_tier(void);
staticfn boolean has_stat_drain_penalty(void);
staticfn boolean aasimar_ability_penalized(long *);
staticfn void poison_losehp(int, const char *, int, struct monst *);

/* adjust an attribute; return TRUE if change is made, FALSE otherwise */
boolean
adjattrib(
    int ndx,    /* which characteristic */
    int incr,   /* amount of change */
    int msgflg) /* positive => no message, zero => message, and */
{               /* negative => conditional (msg if change made) */
    int old_acurr, old_abase, old_amax, decr;
    boolean abonflg;
    const char *attrstr;

    if (Fixed_abil || !incr)
        return FALSE;

    if ((ndx == A_INT || ndx == A_WIS) && uarmh && uarmh->otyp == DUNCE_CAP) {
        if (msgflg == 0)
            Your("cap constricts briefly, then relaxes again.");
        return FALSE;
    }

    old_acurr = ACURR(ndx);
    old_abase = ABASE(ndx);
    old_amax = AMAX(ndx);
    ABASE(ndx) += incr; /* when incr is negative, this reduces ABASE() */
    if (incr > 0) {
        if (ABASE(ndx) > AMAX(ndx)) {
            AMAX(ndx) = ABASE(ndx);
            if (AMAX(ndx) > ATTRMAX(ndx))
                ABASE(ndx) = AMAX(ndx) = ATTRMAX(ndx);
        }
        attrstr = plusattr[ndx];
        abonflg = (ABON(ndx) < 0);
    } else { /* incr is negative */
        if (ABASE(ndx) < ATTRMIN(ndx)) {
            /*
             * If base value has dropped so low that it is trying to be
             * taken below the minimum, reduce max value (peak reached)
             * instead.  That means that restore ability and repeated
             * applications of unicorn horn will not be able to recover
             * all the lost value.  As of 3.6.2, we only take away
             * some (average half, possibly zero) of the excess from max
             * instead of all of it, but without intervening recovery, it
             * can still eventually drop to the minimum allowed.  After
             * that, it can't be recovered, only improved with new gains.
             *
             * This used to assign a new negative value to incr and then
             * add it, but that could affect messages below, possibly
             * making a large decrease be described as a small one.
             *
             * decr = rn2(-(ABASE - ATTRMIN) + 1);
             */
            decr = rn2(ATTRMIN(ndx) - ABASE(ndx) + 1);
            ABASE(ndx) = ATTRMIN(ndx);
            AMAX(ndx) -= decr;
            if (AMAX(ndx) < ATTRMIN(ndx))
                AMAX(ndx) = ATTRMIN(ndx);
        }
        attrstr = minusattr[ndx];
        abonflg = (ABON(ndx) > 0);
    }
    if (ACURR(ndx) == old_acurr) {
        if (msgflg == 0 && flags.verbose) {
            if (ABASE(ndx) == old_abase && AMAX(ndx) == old_amax) {
                pline("You're %s as %s as you can get.",
                      abonflg ? "currently" : "already", attrstr);
            } else {
                /* current stayed the same but base value changed, or
                   base is at minimum and reduction caused max to drop */
                Your("innate %s has %s.", attrname[ndx],
                     (incr > 0) ? "improved" : "declined");
            }
        }
        return FALSE;
    }

    /* Any successful change also resets abuse / exercise level */
    AEXE(ndx) = 0;

    disp.botl = TRUE;
    if (msgflg <= 0)
        You_feel("%s%s!", (incr > 1 || incr < -1) ? "very " : "", attrstr);
    if (program_state.in_moveloop && (ndx == A_STR || ndx == A_CON))
        encumber_msg();
    return TRUE;
}

/* strength gain; msgflg as for adjattrib() */
boolean
gainstr(struct obj *otmp, int incr, int msgflg)
{
    int num = incr;

    /* inherently strong races treat any non-cursed strength gain as
       a full gain roll; giants gain extra and top out at 18/100 */
    if (!num
        || ((Race_if(PM_GIANT) || Race_if(PM_CENTAUR)
             || Race_if(PM_TORTLE) || Race_if(PM_DRAUGR)
             || Race_if(PM_VAMPIRE) || Race_if(PM_AASIMAR))
            && !(otmp && otmp->cursed))) {
        if (ABASE(A_STR) < 18)
            num = (rn2(4) ? 1 : rnd(6));
        else if (ABASE(A_STR) < STR18(85))
            num = rnd(10);
        else
            num = 1;
        if (Race_if(PM_GIANT) && ABASE(A_STR) < STR18(100))
            num += d(2, 2);
        if (ABASE(A_STR) < STR18(100) && ABASE(A_STR) + num > STR18(100))
            num = STR18(100) - ABASE(A_STR);
    }
    return adjattrib(A_STR, (otmp && otmp->cursed) ? -num : num, msgflg);
}

/* strength loss, may kill you; cause may be poison or monster like 'a' */
void
losestr(int num, const char *knam, schar k_format)
{
    int uhpmin = minuhpmax(1), olduhpmax = u.uhpmax;
    int ustr = ABASE(A_STR) - num, amt, dmg;
    boolean waspolyd = Upolyd;

    if (num <= 0 || ABASE(A_STR) < ATTRMIN(A_STR)) {
        impossible("losestr: %d - %d", ABASE(A_STR), num);
        return;
    }
    dmg = 0;
    while (ustr < ATTRMIN(A_STR)) {
        ++ustr;
        --num;
        amt = rn1(4, 3); /* (0..(4-1))+3 => 3..6; used to use flat 6 here */
        dmg += amt;
    }
    if (dmg) {
        /* in case damage is fatal and caller didn't supply killer reason */
        if (!knam || !*knam) {
            knam = "terminal frailty";
            k_format = KILLED_BY;
        }
        losehp(dmg, knam, k_format);

        if (Upolyd) {
            /* when still poly'd, reduce you-as-monst maxHP; never below 1 */
            setuhpmax(max(u.mhmax - dmg, 1), FALSE); /* acts as setmhmax() */
        } else if (!waspolyd) {
            /* not polymorphed now and didn't rehumanize when taking damage;
               reduce max HP, but not below uhpmin */
            if (u.uhpmax > uhpmin)
                setuhpmax(max(u.uhpmax - dmg, uhpmin), FALSE);
        }
        disp.botl = TRUE;
    }
#if 0   /* only possible if uhpmax was already less than uhpmin */
    if (!Upolyd && u.uhpmax < uhpmin) {
        setuhpmax(min(olduhpmax, uhpmin), FALSE);
        if (!Drain_resistance)
            losexp(NULL); /* won't be fatal when no 'drainer' is supplied */
    }
#else
    nhUse(olduhpmax);
#endif
    /* 'num' could have been reduced to 0 in the minimum strength loop;
       '(Upolyd || !waspolyd)' is True unless damage caused rehumanization */
    if (num > 0 && (Upolyd || !waspolyd))
        (void) adjattrib(A_STR, -num, 1);
}

/* combined strength loss and damage from some poisons */
void
poison_strdmg(int strloss, int dmg, const char *knam, schar k_format)
{
    losestr(strloss, knam, k_format);
    losehp(dmg, knam, k_format);
}

static const struct poison_effect_message {
    void (*delivery_func)(const char *, ...);
    const char *effect_msg;
} poiseff[] = {
    { You_feel, "weaker" },             /* A_STR */
    { Your, "brain is on fire" },       /* A_INT */
    { Your, "judgement is impaired" },  /* A_WIS */
    { Your, "muscles won't obey you" }, /* A_DEX */
    { You_feel, "very sick" },          /* A_CON */
    { You, "break out in hives" }       /* A_CHA */
};

/* feedback for attribute loss due to poisoning */
void
poisontell(int typ,         /* which attribute */
           boolean exclaim) /* emphasis */
{
    void (*func)(const char *, ...) = poiseff[typ].delivery_func;
    const char *msg_txt = poiseff[typ].effect_msg;

    /*
     * "You feel weaker" or "you feel very sick" aren't appropriate when
     * wearing or wielding something (gauntlets of power, Ogresmasher)
     * which forces the attribute to maintain its maximum value.
     * Phrasing for other attributes which might have fixed values
     * (dunce cap) is such that we don't need message fixups for them.
     */
    if (typ == A_STR && ACURR(A_STR) == STR19(25))
        msg_txt = "innately weaker";
    else if (typ == A_CON && ACURR(A_CON) == 25)
        msg_txt = "sick inside";

    (*func)("%s%c", msg_txt, exclaim ? '!' : '.');
}

/* hit point loss from poison; a lethal dose delivered by a monster's
   attack is that monster's kill, as its melee damage would be */
staticfn void
poison_losehp(
    int loss,
    const char *pkiller,
    int kprefix,
    struct monst *poisoner)
{
    if (poisoner && !Upolyd && u.uhp - loss < 1) {
        end_running(TRUE); /* as losehp() would */
        mdamageu(poisoner, loss);
    } else {
        losehp(loss, pkiller, kprefix);
    }
}

/* called when an attack or trap has poisoned hero (used to be in mon.c) */
void
poisoned(
    const char *reason,     /* controls what messages we display */
    int typ,
    const char *pkiller,    /* for score+log file if fatal */
    int fatal,              /* if fatal is 0, limit damage to adjattrib */
    boolean thrown_weapon,  /* thrown weapons are less deadly */
    struct monst *poisoner) /* monster whose melee attack delivered the
                             * poison, or Null; a fatal dose is its kill */
{
    int i, loss, kprefix = KILLED_BY_AN;
    boolean blast = !strcmp(reason, "blast");

    /* inform player about being poisoned unless that's already been done;
       "blast" has given a "blast of poison gas" message; "poison arrow",
       "poison dart", etc have implicitly given poison messages too... */
    if (!blast && !strstri(reason, "poison")) {
        boolean plural = (reason[strlen(reason) - 1] == 's') ? 1 : 0;

        /* avoid "The" Orcus's sting was poisoned... */
        pline("%s%s %s poisoned!",
              isupper((uchar) *reason) ? "" : "The ", reason,
              plural ? "were" : "was");
    }
    if (how_resistant(POISON_RES) == 100) {
        if (blast)
            shieldeff(u.ux, u.uy);
        pline_The("poison doesn't seem to affect you.");
        return;
    }

    /* suppress killer prefix if it already has one */
    i = name_to_mon(pkiller, (int *) 0);
    if (ismnum(i) && (mons[i].geno & G_UNIQ)) {
        kprefix = KILLED_BY;
        if (!type_is_pname(&mons[i]))
            pkiller = the(pkiller);
    } else if (!strncmpi(pkiller, "the ", 4) || !strncmpi(pkiller, "an ", 3)
               || !strncmpi(pkiller, "a ", 2)) {
        /*[ does this need a plural check too? ]*/
        kprefix = KILLED_BY;
    }

    /*
     * FIXME:
     *  this operates on u.uhp[max] even when hero is polymorphed....
     */

    i = !fatal ? 1 : rn2(fatal + (thrown_weapon ? 20 : 0));
    if (i == 0 && typ != A_CHA) {
        if (how_resistant(POISON_RES) >= 35) {
            /* partial resistance turns the instant kill into a
               severe reaction */
            /* adjuhploss() compares this with u.mh when polymorphed */
            int olduhp = Upolyd ? u.mh : u.uhp, newuhpmax;

            pline_The("poison was extremely toxic!");
            loss = resist_reduce(d(4, 6), POISON_RES);
            newuhpmax = u.uhpmax - (loss / 2);
            /* the floor never raises a max that is already below it */
            newuhpmax = min(u.uhpmax, max(newuhpmax, minuhpmax(3)));
            setuhpmax(newuhpmax, TRUE); /*True: see FIXME*/
            loss = adjuhploss(loss, olduhp);
            poison_losehp(loss, pkiller, kprefix, poisoner);
        } else {
            /* instant kill */
            u.uhp = -1;
            disp.botl = TRUE;
            pline_The("poison was deadly...");
        }
    } else if (i > 5) {
        boolean cloud = !strcmp(reason, "gas cloud");

        /* HP damage; more likely--but less severe--with missiles */
        loss = resist_reduce(thrown_weapon ? rnd(6) : rn1(10, 6), POISON_RES);
        if ((blast || cloud) && Half_gas_damage) /* worn towel */
            loss = (loss + 1) / 2;
        poison_losehp(loss, pkiller, kprefix, poisoner);
    } else {
        /* attribute loss; if typ is A_STR, reduction in current and
           maximum HP will occur once strength has dropped down to 3 */
        loss = (thrown_weapon || !fatal)
                   ? 1 : resist_reduce(d(2, 2), POISON_RES); /* was rn1(3,3) */
        /* check that a stat change was made */
        if (adjattrib(typ, -loss, 1))
            poisontell(typ, TRUE);
    }

    if (u.uhp < 1) {
        if (poisoner) {
            /* named, and risen from the grave, as its melee kill */
            done_in_by(poisoner, POISONING);
        } else {
            svk.killer.format = kprefix;
            Strcpy(svk.killer.name, pkiller);
            /* "Poisoned by a poisoned ___" is redundant */
            done(strstri(pkiller, "poison") ? DIED : POISONING);
        }
    }
    encumber_msg();
}

void
change_luck(schar n)
{
    u.uluck += n;
    if (u.uluck < 0 && u.uluck < LUCKMIN)
        u.uluck = LUCKMIN;
    if (u.uluck > 0 && u.uluck > LUCKMAX)
        u.uluck = LUCKMAX;
}

/* decide whether there are more blessed luckstones (plus luck-conferring
   artifacts) than cursed ones; optionally combine uncursed with blessed */
int
stone_luck(boolean include_uncursed)
{
    struct obj *otmp;
    long bonchance = 0;

    for (otmp = gi.invent; otmp; otmp = otmp->nobj)
        if (confers_luck(otmp)) {
            if (otmp->cursed)
                bonchance -= otmp->quan;
            else if (otmp->blessed || include_uncursed)
                bonchance += otmp->quan;
        }

    return sgn((int) bonchance);
}

/* is the hero carrying anything that confers luck, whatever its BUC? */
boolean
has_luckitem(void)
{
    struct obj *otmp;

    for (otmp = gi.invent; otmp; otmp = otmp->nobj)
        if (confers_luck(otmp))
            return TRUE;
    return FALSE;
}

/* there has just been an inventory change affecting a luck-granting item */
void
set_moreluck(void)
{
    if (!has_luckitem())
        u.moreluck = 0;
    else if (stone_luck(TRUE) >= 0)
        u.moreluck = LUCKADD;
    else
        u.moreluck = -LUCKADD;
}

/* (not used) */
void
restore_attrib(void)
{
    int i, equilibrium;;

    /*
     * Note:  this used to get called by moveloop() on every turn but
     * ATIME() is never set to non-zero anywhere so didn't do anything.
     * Presumably it once supported something like potion of heroism
     * which conferred temporary characteristics boost(s).
     *
     * ATEMP() is used for strength loss from hunger, which doesn't
     * time out, and for dexterity loss from wounded legs, which has
     * its own timeout routine.
     */

    for (i = 0; i < A_MAX; i++) { /* all temporary losses/gains */
        equilibrium = ((i == A_STR && u.uhs >= WEAK)
                       || (i == A_DEX && Wounded_legs)) ? -1 : 0;
        if (ATEMP(i) != equilibrium && ATIME(i) != 0) {
            if (!(--(ATIME(i)))) { /* countdown for change */
                ATEMP(i) += (ATEMP(i) > 0) ? -1 : 1;
                disp.botl = TRUE;
                if (ATEMP(i)) /* reset timer */
                    ATIME(i) = 100 / ACURR(A_CON);
            }
        }
    }
    if (disp.botl)
        encumber_msg();
}

#define AVAL 50 /* tune value for exercise gains */

void
exercise(int i, boolean inc_or_dec)
{
    debugpline0("Exercise:");
    if (i == A_CHA)
        return; /* can't exercise charisma */

    /* no physical exercise while polymorphed; the body's temporary,
       but the mind isn't */
    if (Upolyd && !(i == A_WIS || i == A_INT))
        return;

    if (abs(AEXE(i)) < AVAL) {
        /*
         *      Law of diminishing returns (Part I):
         *
         *      Gain is harder at higher attribute values.
         *      79% at "3" --> 0% at "18"
         *      Loss is even at all levels (50%).
         *
         *      Note: *YES* ACURR is the right one to use.
         */
        AEXE(i) += (inc_or_dec) ? (rn2(19) > ACURR(i)) : -rn2(2);
        debugpline3("%s, %s AEXE = %d",
                    (i == A_STR) ? "Str"
                    : (i == A_INT) ? "Int"
                      : (i == A_WIS) ? "Wis"
                        : (i == A_DEX) ? "Dex"
                          : "Con",
                    (inc_or_dec) ? "inc" : "dec", AEXE(i));
    }
    if (svm.moves > 0 && (i == A_STR || i == A_CON))
        encumber_msg();
}

staticfn void
exerper(void)
{
    if (!(svm.moves % 10)) {
        /* Hunger Checks; hobbits eat often and are rarely sated;
           vampire heroes can push into negative satiation */
        int hs;

        if (!racial_vampire(&gy.youmonst))
            hs = (u.uhunger > (Race_if(PM_HOBBIT) ? 3000 : 1000)) ? SATIATED
                 : (u.uhunger > 150) ? NOT_HUNGRY
                   : (u.uhunger > 50) ? HUNGRY
                     : (u.uhunger > 0) ? WEAK
                       : FAINTING;
        else
            hs = (u.uhunger > 1000) ? SATIATED
                 : (u.uhunger > 150) ? NOT_HUNGRY
                   : (u.uhunger > 50) ? HUNGRY
                     : (u.uhunger > -100) ? WEAK
                       : (u.uhunger > -300) ? FRAIL
                         : STARVED;

        debugpline0("exerper: Hunger checks");
        switch (hs) {
        case SATIATED:
            if (!(maybe_polyd(is_zombie(gy.youmonst.data),
                              Race_if(PM_DRAUGR))
                  || maybe_polyd(is_vampire(gy.youmonst.data),
                                 Race_if(PM_VAMPIRE))))
                exercise(A_DEX, FALSE);
            if (Role_if(PM_MONK))
                exercise(A_WIS, FALSE);
            break;
        case NOT_HUNGRY:
            exercise(A_CON, TRUE);
            break;
        case WEAK:
            exercise(A_STR, FALSE);
            if (Role_if(PM_MONK)) /* fasting */
                exercise(A_WIS, TRUE);
            break;
        case FAINTING:
        case FAINTED:
            exercise(A_CON, FALSE);
            break;
        case FRAIL:
        case STARVED:
            exercise(A_STR, FALSE);
            exercise(A_CON, FALSE);
            break;
        }

        /* Encumbrance Checks */
        debugpline0("exerper: Encumber checks");
        switch (near_capacity()) {
        case MOD_ENCUMBER:
            exercise(A_STR, TRUE);
            break;
        case HVY_ENCUMBER:
            exercise(A_STR, TRUE);
            exercise(A_DEX, FALSE);
            break;
        case EXT_ENCUMBER:
            exercise(A_DEX, FALSE);
            exercise(A_CON, FALSE);
            break;
        }
    }

    /* status checks */
    if (!(svm.moves % 5)) {
        debugpline0("exerper: Status checks");
        if ((HClairvoyant & (INTRINSIC | TIMEOUT)) && !BClairvoyant)
            exercise(A_WIS, TRUE);
        if (HRegeneration)
            exercise(A_STR, TRUE);

        if (Sick || Vomiting)
            exercise(A_CON, FALSE);
        if (Confusion || Hallucination)
            exercise(A_WIS, FALSE);
        if ((Wounded_legs && !u.usteed) || Fumbling || HStun)
            exercise(A_DEX, FALSE);
    }
}

/* exercise/abuse text (must be in attribute order, not botl order);
   phrased as "You must have been [][0]." or "You haven't been [][1]." */
static NEARDATA const char *const exertext[A_MAX][2] = {
    { "exercising diligently", "exercising properly" },           /* Str */
    { "studying hard", "focusing properly" },                     /* Int */
    { "very observant", "paying attention" },                     /* Wis */
    { "working on your reflexes", "working on reflexes lately" }, /* Dex */
    { "leading a healthy life-style", "watching your health" },   /* Con */
    { 0, 0 },                                                     /* Cha */
};

void
exerchk(void)
{
    int i, ax, mod_val, lolim, hilim;

    /*  Check out the periodic accumulations */
    exerper();

    if (svm.moves >= svc.context.next_attrib_check) {
        debugpline1("exerchk: ready to test. multi = %ld.", gm.multi);
    }
    /*  Are we ready for a test? */
    if (svm.moves >= svc.context.next_attrib_check && !gm.multi) {
        debugpline0("exerchk: testing.");
        /*
         *      Law of diminishing returns (Part II):
         *
         *      The effects of "exercise" and "abuse" wear
         *      off over time.  Even if you *don't* get an
         *      increase/decrease, you lose some of the
         *      accumulated effects.
         */
        for (i = 0; i < A_MAX; ++i) {
            ax = AEXE(i);
            /* nothing to do here if no exercise or abuse has occurred
               (Cha always falls into this category) */
            if (!ax)
                continue; /* ok to skip nextattrib */

            mod_val = sgn(ax); /* +1 or -1; used below */
            /* no further effect for exercise if at max or abuse if at min;
               can't exceed 18 via exercise even if actual max is higher */
            lolim = ATTRMIN(i); /* usually 3; might be higher */
            hilim = ATTRMAX(i); /* usually 18; maybe lower or higher */
            if (hilim > 18)
                hilim = 18;
            if ((ax < 0) ? (ABASE(i) <= lolim) : (ABASE(i) >= hilim))
                goto nextattrib;
            /* can't exercise the body (Str, Dex, Con) while polymorphed;
               previous exercise/abuse gradually wears off without impact
               then */
            if (Upolyd && !(i == A_WIS || i == A_INT))
                goto nextattrib;

            debugpline2("exerchk: testing %s (%d).",
                        (i == A_STR) ? "Str"
                        : (i == A_INT) ? "Int"
                          : (i == A_WIS) ? "Wis"
                            : (i == A_DEX) ? "Dex"
                              : (i == A_CON) ? "Con"
                                : (i == A_CHA) ? "Cha?"
                                  : "???",
                        ax);
            /*
             *  Law of diminishing returns (Part III):
             *
             *  You don't *always* gain by exercising.
             *  [MRS 92/10/28 - Treat Wisdom specially for balance.]
             */
            if (rn2(AVAL) > ((i != A_WIS) ? (abs(ax) * 2 / 3) : abs(ax)))
                goto nextattrib;

            debugpline1("exerchk: changing %d.", i);
            if (adjattrib(i, mod_val, -1)) {
                debugpline1("exerchk: changed %d.", i);
                /* if you actually changed an attrib - zero accumulation */
                AEXE(i) = ax = 0;
                /* then print an explanation */
                You("%s %s.",
                    (mod_val > 0) ? "must have been" : "haven't been",
                    exertext[i][(mod_val > 0) ? 0 : 1]);
            }
 nextattrib:
            /* this used to be ``AEXE(i) /= 2'' but that would produce
               platform-dependent rounding/truncation for negative vals */
            AEXE(i) = (abs(ax) / 2) * mod_val;
        }
        svc.context.next_attrib_check += rn1(200, 800);
        debugpline1("exerchk: next check at %ld.",
                    svc.context.next_attrib_check);
    }
}

/* return random hero attribute (by role's attr distribution).
   returns A_MAX if failed. */
staticfn int
rnd_attr(void)
{
    int i, x = rn2(100);

    /* 5.0: the x -= ... calculation used to have an off by 1 error that
       resulted in the values being biased toward Str and away from Cha */
    for (i = 0; i < A_MAX; ++i)
        if ((x -= gu.urole.attrdist[i]) < 0)
            break;
    return i;
}

/* add or subtract np points from random attributes,
   adjusting the base and maximum values of the attributes.
   if subtracting, np must be negative.
   returns the left over points. */
staticfn int
init_attr_role_redist(int np, boolean addition)
{
    int tryct = 0;
    int adj = addition ? 1 : -1;

    while ((addition ? (np > 0) : (np < 0)) && tryct < 100) {
        int i = rnd_attr();

        if (i >= A_MAX
            || (addition ? (ABASE(i) >= ATTRMAX(i))
                         : (ABASE(i) <= ATTRMIN(i)))) {
            tryct++;
            continue;
        }
        tryct = 0;
        ABASE(i) += adj;
        AMAX(i) += adj;
        np -= adj;
    }
    return np;
}

/* allocate hero's initial characteristics */
void
init_attr(int np)
{
    int i;

    for (i = 0; i < A_MAX; i++) {
        ABASE(i) = AMAX(i) = gu.urole.attrbase[i];
        /* a role's minimum can reach or exceed the race's maximum
           (giant Barbarian: Dex 15 against 14); drop it one below
           the cap on purpose, so the redistribution below can land
           it on 13 or 14 rather than always 14 */
        while (ABASE(i) >= ATTRMAX(i)) {
            ABASE(i)--;
            AMAX(i)--;
        }
        ATEMP(i) = ATIME(i) = 0;
        np -= ABASE(i);
    }

    /* distribute leftover points */
    np = init_attr_role_redist(np, TRUE);
    /* if we went over, remove points */
    np = init_attr_role_redist(np, FALSE);
}

void
redist_attr(void)
{
    int i, tmp;

    for (i = 0; i < A_MAX; i++) {
        if (i == A_INT || i == A_WIS)
            continue;
        /* Polymorphing doesn't change your mind */
        tmp = AMAX(i);
        AMAX(i) += (rn2(5) - 2);
        if (AMAX(i) > ATTRMAX(i))
            AMAX(i) = ATTRMAX(i);
        if (AMAX(i) < ATTRMIN(i))
            AMAX(i) = ATTRMIN(i);
        ABASE(i) = ABASE(i) * AMAX(i) / tmp;
        /* ABASE(i) > ATTRMAX(i) is impossible */
        if (ABASE(i) < ATTRMIN(i))
            ABASE(i) = ATTRMIN(i);
    }
    /* encumber_msg(); -- caller needs to do this */
}

/* apply minor variation to attributes */
void
vary_init_attr(void)
{
    int i;

    for (i = 0; i < A_MAX; i++)
        if (!rn2(20)) {
            int xd = rn2(7) - 2; /* biased variation */

            (void) adjattrib(i, xd, TRUE);
            if (ABASE(i) < AMAX(i))
                AMAX(i) = ABASE(i);
        }
}

staticfn
void
postadjabil(long *ability)
{
    if (!u.ulevel) /* initializing hero; don't attempt screen update yet */
        return;
    /* telepathic monsters can be sensed even when not blind */
    if (ability == &(HWarning) || ability == &(HSee_invisible)
        || ability == &(HTelepat))
        see_monsters();
}

staticfn const struct innate *
role_abil(int r)
{
    const struct {
        short role;
        const struct innate *abil;
    } roleabils[] = {
        { PM_ARCHEOLOGIST, arc_abil },
        { PM_BARBARIAN, bar_abil },
        { PM_CAVE_DWELLER, cav_abil },
        { PM_CONVICT, con_abil },
        { PM_DRUID, dru_abil },
        { PM_HEALER, hea_abil },
        { PM_INFIDEL, inf_abil },
        { PM_KNIGHT, kni_abil },
        { PM_MONK, mon_abil },
        { PM_CLERIC, pri_abil },
        { PM_RANGER, ran_abil },
        { PM_ROGUE, rog_abil },
        { PM_SAMURAI, sam_abil },
        { PM_TOURIST, tou_abil },
        { PM_VALKYRIE, val_abil },
        { PM_WIZARD, wiz_abil },
        { 0, 0 }
    };
    int i;

    for (i = 0; roleabils[i].abil && roleabils[i].role != r; i++)
        continue;
    return roleabils[i].abil;
}

staticfn const struct innate *
check_innate_abil(long *ability, long frommask)
{
    const struct innate *abil = 0;

    if (frommask == FROMEXPER)
        abil = role_abil(Role_switch);
    else if (frommask == FROMRACE)
        switch (Race_switch) {
        case PM_DWARF:
            abil = dwa_abil;
            break;
        case PM_ELF:
            abil = elf_abil;
            break;
        case PM_GNOME:
            abil = gno_abil;
            break;
        case PM_ORC:
            abil = orc_abil;
            break;
        case PM_GIANT:
            abil = gia_abil;
            break;
        case PM_HOBBIT:
            abil = hob_abil;
            break;
        case PM_CENTAUR:
            abil = cen_abil;
            break;
        case PM_ILLITHID:
            abil = ill_abil;
            break;
        case PM_DEMON:
            abil = dem_abil;
            break;
        case PM_TORTLE:
            abil = trt_abil;
            break;
        case PM_DROW:
            abil = dro_abil;
            break;
        case PM_DRAUGR:
            abil = dra_abil;
            break;
        case PM_VAMPIRE:
            abil = vam_abil;
            break;
        case PM_AASIMAR:
            abil = amr_abil;
            break;
        case PM_HUMAN:
            abil = hum_abil;
            break;
        default:
            break;
        }

    while (abil && abil->ability) {
        if ((abil->ability == ability) && (u.ulevel >= abil->ulevel))
            return abil;
        abil++;
    }
    return (struct innate *) 0;
}

/* reasons for innate ability */
#define FROM_NONE 0
#define FROM_ROLE 1 /* from experience at level 1 */
#define FROM_RACE 2
#define FROM_INTR 3 /* intrinsically (eating some corpse or prayer reward) */
#define FROM_EXP  4 /* from experience for some level > 1 */
#define FROM_FORM 5
#define FROM_LYCN 6

/* check whether particular ability has been obtained via innate attribute */
staticfn int
innately(long *ability)
{
    const struct innate *iptr;

    if ((iptr = check_innate_abil(ability, FROMEXPER)) != 0)
        return (iptr->ulevel == 1) ? FROM_ROLE : FROM_EXP;
    if ((iptr = check_innate_abil(ability, FROMRACE)) != 0)
        return FROM_RACE;
    if ((*ability & FROMOUTSIDE) != 0L)
        return FROM_INTR;
    if ((*ability & FROMFORM) != 0L)
        return FROM_FORM;
    return FROM_NONE;
}

int
is_innate(int propidx)
{
    int innateness;

    /* innately() would report FROM_FORM for this; caller wants specificity */
    if (propidx == DRAIN_RES && ismnum(u.ulycn))
        return FROM_LYCN;
    if (propidx == FAST && Very_fast)
        return FROM_NONE; /* can't become very fast innately */
    if (propidx == FLYING && (BFlying & W_ARMOR))
        return FROM_NONE; /* not from form, as that is blocked */
    if ((innateness = innately(&u.uprops[propidx].intrinsic)) != FROM_NONE)
        return innateness;
    if (propidx == JUMPING && Role_if(PM_KNIGHT)
        /* knight has intrinsic jumping, but extrinsic is more versatile so
           ignore innateness if equipment is going to claim responsibility */
        && !u.uprops[propidx].extrinsic)
        return FROM_ROLE;
    if ((propidx == BLINDED && !haseyes(gy.youmonst.data))
        || (propidx == BLND_RES && (HBlnd_resist & FROMFORM) != 0))
        return FROM_FORM;
    return FROM_NONE;
}

DISABLE_WARNING_FORMAT_NONLITERAL

char *
from_what(
    int propidx) /* special cases can have negative values */
{
    static char buf[BUFSZ];

    buf[0] = '\0';
    /*
     * Restrict the source of the attributes just to debug mode for now
     */
    if (wizard) {
        static NEARDATA const char because_of[] = " because of %s";

        if (propidx >= 0) {
            char *p;
            struct obj *obj = (struct obj *) 0;
            int innateness = is_innate(propidx);

            /*
             * Properties can be obtained from multiple sources and we
             * try to pick the most significant one.  Classification
             * priority is not set in stone; current precedence is:
             * "from the start" (from role or race at level 1),
             * "from outside" (eating corpse, divine reward, blessed potion),
             * "from experience" (from role or race at level 2+),
             * "from current form" (while polymorphed),
             * "from timed effect" (potion or spell),
             * "from worn/wielded equipment" (Firebrand, elven boots, &c),
             * "from carried equipment" (mainly quest artifacts).
             * There are exceptions.  Versatile jumping from spell or boots
             * takes priority over knight's innate but limited jumping.
             */
            if ((propidx == BLINDED && u.uroleplay.blind)
                || (propidx == DEAF && u.uroleplay.deaf))
                Sprintf(buf, " from birth");
            else if (innateness == FROM_ROLE || innateness == FROM_RACE)
                Strcpy(buf, " innately");
            else if (innateness == FROM_INTR) /* [].intrinsic & FROMOUTSIDE */
                Strcpy(buf, " intrinsically");
            else if (innateness == FROM_EXP)
                Strcpy(buf, " because of your experience");
            else if (innateness == FROM_LYCN)
                Strcpy(buf, " due to your lycanthropy");
            else if (innateness == FROM_FORM)
                Strcpy(buf, " from your creature form");
            else if (propidx == FAST && Very_fast)
                Sprintf(buf, because_of,
                        ((HFast & TIMEOUT) != 0L) ? "a potion or spell"
                          : ((EFast & W_ARMF) != 0L && uarmf->dknown
                             && objects[uarmf->otyp].oc_name_known)
                              ? ysimple_name(uarmf) /* speed boots */
                                : EFast ? "worn equipment"
                                  : something);
            else if (wizard
                     && (obj = what_gives(&u.uprops[propidx].extrinsic)) != 0)
                Sprintf(buf, because_of, obj->oartifact
                                             ? bare_artifactname(obj)
                                             : ysimple_name(obj));
            else if (propidx == BLINDED && Blindfolded_only)
                Sprintf(buf, because_of, ysimple_name(ublindf));
            else if (propidx == BLINDED && u.ucreamed
                     && BlindedTimeout == (long) u.ucreamed
                     && !EBlinded && !(HBlinded & ~TIMEOUT))
                Sprintf(buf, "due to goop covering your %s",
                        body_part(FACE));

            /* remove some verbosity and/or redundancy */
            if ((p = strstri(buf, " pair of ")) != 0)
                copynchars(p + 1, p + 9, BUFSZ); /* overlapping buffers ok */
            else if (propidx == STRANGLED
                     && (p = strstri(buf, " of strangulation")) != 0)
                *p = '\0';

        } else { /* negative property index */
            /* if more blocking capabilities get implemented we'll need to
               replace this with what_blocks() comparable to what_gives() */
            switch (-propidx) {
            case BLINDED:
                /* wearing the Eyes of the Overworld overrides blindness */
                if (BBlinded && is_art(ublindf, ART_EYES_OF_THE_OVERWORLD))
                    Sprintf(buf, because_of, bare_artifactname(ublindf));
                break;
            case INVIS:
                if (u.uprops[INVIS].blocked & W_ARMC)
                    Sprintf(buf, because_of,
                            ysimple_name(uarmc)); /* mummy wrapping */
                break;
            case CLAIRVOYANT:
                if (wizard && (u.uprops[CLAIRVOYANT].blocked & W_ARMH))
                    Sprintf(buf, because_of,
                            ysimple_name(uarmh)); /* cornuthaum */
                break;
            }
        }

    } /*wizard*/
    return buf;
}

RESTORE_WARNING_FORMAT_NONLITERAL

void
adjabil(int oldlevel, int newlevel)
{
    const struct innate *abil, *rabil;
    long prevabil, mask = FROMEXPER;
    /* don't give resistance messages when demon crowning; they aren't
       given in normal crowning, and you can e.g. lose and regain
       warning, so you don't want messages about that */
    boolean verbose = !(oldlevel == 0 || newlevel == 0);
    boolean told, was_flying = !!Flying;

    abil = role_abil(Role_switch);

    switch (Race_switch) {
    case PM_ELF:
        rabil = elf_abil;
        break;
    case PM_ORC:
        rabil = orc_abil;
        break;
    case PM_GIANT:
        rabil = gia_abil;
        break;
    case PM_HOBBIT:
        rabil = hob_abil;
        break;
    case PM_CENTAUR:
        rabil = cen_abil;
        break;
    case PM_ILLITHID:
        rabil = ill_abil;
        break;
    case PM_TORTLE:
        rabil = trt_abil;
        break;
    case PM_DROW:
        rabil = dro_abil;
        break;
    case PM_DRAUGR:
        rabil = dra_abil;
        break;
    case PM_VAMPIRE:
        rabil = vam_abil;
        break;
    case PM_DEMON:
        rabil = dem_abil;
        break;
    case PM_AASIMAR:
        rabil = amr_abil;
        break;
    case PM_HUMAN:
        rabil = hum_abil;
        break;
    /* explicitly write these out or else infravision will be FROMFORM
       (though ^x will show it as innate) */
    case PM_DWARF:
        rabil = dwa_abil;
        break;
    case PM_GNOME:
        rabil = gno_abil;
        break;
    default:
        rabil = 0;
        break;
    }

    while (abil || rabil) {
        /* Have we finished with the intrinsics list? */
        if (!abil || !abil->ability) {
            /* Try the race intrinsics */
            if (!rabil || !rabil->ability)
                break;
            abil = rabil;
            rabil = 0;
            mask = FROMRACE;
        }
        prevabil = *(abil->ability);
        told = FALSE;
        /* giants are too big and loud to acquire stealth from their
           role; the undead races never warm up to fire resistance;
           mindless draugr never develop telepathy */
        if ((Race_if(PM_GIANT) && abil->ability == &HStealth)
            || (Race_if(PM_VAMPIRE) && abil->ability == &HFire_resistance)
            || (Race_if(PM_DRAUGR) && abil->ability == &HFire_resistance)
            || (Race_if(PM_DRAUGR) && abil->ability == &HTelepat)) {
            abil++;
            continue;
        }
        if (oldlevel < abil->ulevel && newlevel >= abil->ulevel) {
            /* Abilities gained at level 1 can never be lost
             * via level loss, but can be lost by race change via
             * infidel crowning. So, track separately from corpses.
             */
            /* Don't grant if an aasimar abuse penalty is
               stripping this intrinsic */
            if (mask == FROMRACE && Race_if(PM_AASIMAR)
                && aasimar_ability_penalized(abil->ability)) {
                abil++;
                continue;
            }
            *(abil->ability) |= mask;
            /* no magic resistance conduct */
            if (mask == FROMRACE && abil->ability == &HAntimagic)
                u.uconduct.antimagic++;
            if (!(*(abil->ability) & INTRINSIC & ~HAVEPARTIAL & ~mask)
                && (!(*(abil->ability) & HAVEPARTIAL & ~mask)
                    || (*(abil->ability) & TIMEOUT) < 100)
                && verbose) {
                if (*(abil->gainstr)) {
                    You_feel("%s!", abil->gainstr);
                    told = TRUE;
                }
            }
        } else if (oldlevel >= abil->ulevel && newlevel < abil->ulevel) {
            *(abil->ability) &= ~mask;
            if (prevabil != *(abil->ability)
                && !(*(abil->ability) & INTRINSIC & ~HAVEPARTIAL)
                && (!(*(abil->ability) & HAVEPARTIAL)
                    || (*(abil->ability) & TIMEOUT) < 100)
                && verbose) {
                if (*(abil->losestr)) {
                    You_feel("%s!", abil->losestr);
                    told = TRUE;
                } else if (*(abil->gainstr)) {
                    You_feel("less %s!", abil->gainstr);
                    told = TRUE;
                }
            }
        }
        if (prevabil != *(abil->ability)) { /* it changed */
            /* a polymorphed aasimar's untold change to it is moot once
               this says so; a silent flip leaves it to be told */
            if (told)
                aasimar_change_told(abil->ability);
            postadjabil(abil->ability);
        }
        abil++;
    }

    if (Role_if(PM_MONK) && newlevel >= 25 && Stoned)
        make_stoned(0L, "You no longer seem to be petrifying.",
                    0, (char *) 0);

    /* don't lose infidel skill slots when crowning; probably good to have
       the symmetry regardless (newlevel == 0 should never happen elsewhere,
       but if it does we probably don't want lost skill slots there either) */
    if (oldlevel > 0 && newlevel > 0) {
        if (newlevel > oldlevel)
            add_weapon_skill(newlevel - oldlevel);
        else
            lose_weapon_skill(oldlevel - newlevel);
    }

    /* Re-evaluate aasimar abuse penalties after level change.
       A newly gained intrinsic may need to be stripped if abused */
    aasimar_check_abuse();

    /* racial flight lost to a level drain (or stripped by crowning, whose
       demonic wings may yet take over); losexp() runs inside attacks, so
       let moveloop land the hero */
    if (was_flying && !Flying)
        defer_landing(LANDING_FLY);
}

/* called when gaining a level (before u.ulevel gets incremented);
   also called with u.ulevel==0 during hero initialization or for
   re-init if hero turns into a "new man/woman/elf/&c" */
int
newhp(void)
{
    int hp, conplus;

    if (u.ulevel == 0) {
        /* Initialize hit points */
        hp = gu.urole.hpadv.infix + gu.urace.hpadv.infix;
        if (gu.urole.hpadv.inrnd > 0)
            hp += rnd(gu.urole.hpadv.inrnd);
        if (gu.urace.hpadv.inrnd > 0)
            hp += rnd(gu.urace.hpadv.inrnd);
        if (svm.moves == 0) { /* initial hero; skip for polyself to new man */
            /* Initialize alignment stuff */
            u.ualign.type = aligns[flags.initalign].value;
            u.ualign.record = gu.urole.initrecord;
        }
        /* no Con adjustment for initial hit points */
    } else {
        if (u.ulevel < gu.urole.xlev) {
            hp = gu.urole.hpadv.lofix + gu.urace.hpadv.lofix;
            if (gu.urole.hpadv.lornd > 0)
                hp += rnd(gu.urole.hpadv.lornd);
            if (gu.urace.hpadv.lornd > 0)
                hp += rnd(gu.urace.hpadv.lornd);
        } else {
            hp = gu.urole.hpadv.hifix + gu.urace.hpadv.hifix;
            if (gu.urole.hpadv.hirnd > 0)
                hp += rnd(gu.urole.hpadv.hirnd);
            if (gu.urace.hpadv.hirnd > 0)
                hp += rnd(gu.urace.hpadv.hirnd);
        }

        /* Cavepersons who have kept illiterate conduct get a random
           hit point boost, bigger at higher ranks, and a little
           wisdom exercise */
        if (Role_if(PM_CAVE_DWELLER) && !u.uconduct.literate) {
            if (u.ulevel < 3)
                hp += rn1(3, 2); /* 1st rank: 2..4 */
            else if (u.ulevel < 10)
                hp += rn1(4, 3); /* 2nd and 3rd ranks: 3..6 */
            else if (u.ulevel < 18)
                hp += rn1(6, 3); /* 4th and 5th ranks: 3..8 */
            else
                hp += rn1(8, 5); /* 6th through 9th ranks: 5..12 */
            exercise(A_WIS, TRUE);
        }

        /* Aasimar divine vitality: bonus HP per level if
           alignment is completely unblemished or fully atoned */
        if (Race_if(PM_AASIMAR) && u.ualign.abuse == 0)
            hp += rn1(4, 3);

        if (ACURR(A_CON) <= 3)
            conplus = -2;
        else if (ACURR(A_CON) <= 6)
            conplus = -1;
        else if (ACURR(A_CON) <= 14)
            conplus = 0;
        else if (ACURR(A_CON) <= 16)
            conplus = 1;
        else if (ACURR(A_CON) == 17)
            conplus = 2;
        else if (ACURR(A_CON) == 18)
            conplus = 3;
        else
            conplus = 4;
        hp += conplus;
    }
    if (hp <= 0)
        hp = 1;
    if (u.ulevel < MAXULEV) {
        /* remember increment; future level drain could take it away again */
        u.uhpinc[u.ulevel] = (xint16) hp;
    } else {
        /* after level 30, throttle hit point gains from extra experience;
           once max reaches 1200, further increments will be just 1 more */
        char lim = 5 - u.uhpmax / 300;

        lim = max(lim, 1);
        if (hp > lim)
            hp = lim;
    }
    return hp;
}

/* minimum value for uhpmax is ulevel but for life-saving it is always at
   least 10 if ulevel is less than that */
int
minuhpmax(int altmin)
{
    if (altmin < 1)
        altmin = 1;
    return max(u.ulevel, altmin);
}

/* update u.uhpmax or u.mhmax and values of other things that depend upon
   whichever of them is relevant */
void
setuhpmax(int newmax, boolean even_when_polyd)
{
    if (!Upolyd || even_when_polyd) {
        if (newmax != u.uhpmax) {
            u.uhpmax = newmax;
            if (u.uhpmax > u.uhppeak)
                u.uhppeak = u.uhpmax;
            disp.botl = TRUE;
        }
        if (u.uhp > u.uhpmax)
            u.uhp = u.uhpmax, disp.botl = TRUE;
    } else { /* Upolyd */
        if (newmax != u.mhmax) {
            u.mhmax = newmax;
            disp.botl = TRUE;
        }
        if (u.mh > u.mhmax)
            u.mh = u.mhmax, disp.botl = TRUE;
    }
}

/* permanent loss of base maximum hit points from a partly resisted
   death ray, drain blast or touch of death, taken even while
   polymorphed; never below 1. In normal form the damage that follows
   goes through losehp(), which mends a maximum left below current hit
   points (master's arithmetic, so that is left alone); while
   polymorphed losehp() only touches u.mh, so clamp here */
void
drain_uhpmax(int amt)
{
    u.uhpmax -= amt;
    if (u.uhpmax < 1)
        u.uhpmax = 1;
    if (Upolyd && u.uhp > u.uhpmax)
        u.uhp = u.uhpmax;
    disp.botl = TRUE;
}

/* called after setuhpmax() when damage is pending;
   if uhpmax (or mhmax) has been reduced, it might have caused uhp (or mh)
   to be reduced too; if so, recalculate pending loss to account for that */
int
adjuhploss(
    int loss, /* pending hp loss */
    int olduhp) /* does double duty as oldmh when Upolyd */
{
    if (!Upolyd) {
        if (u.uhp < olduhp)
            loss -= (olduhp - u.uhp);
    } else {
        if (u.mh < olduhp)
            loss -= (olduhp - u.mh);
    }
    return max(loss, 1);
}

/* return the current effective value of a specific characteristic
   (the 'a' in 'acurr()' comes from outdated use of "attribute" for the
   six Str/Dex/&c characteristics; likewise for u.abon, u.atemp, u.acurr) */
schar
acurr(int chridx)
{
    int tmp, result = 0; /* 'result' will always be reset to positive value */

    assert(chridx >= 0 && chridx < A_MAX);
    tmp = u.abon.a[chridx] + u.atemp.a[chridx] + u.acurr.a[chridx];

    /* for Strength:  3 <= result <= 125;
       for all others:  3 <= result <= 25 */
    if (chridx == A_STR) {
        /* strength value is encoded:  3..18 normal, 19..118 for 18/xx (with
           1 <= xx <= 100), and 119..125 for other characteristics' 19..25;
           STR18(x) yields 18 + x (intended for 0 <= x <= 100; not used here);
           STR19(y) yields 100 + y (intended for 19 <= y <= 25) */
        if (tmp >= STR19(25) || (uarmg && uarmg->otyp == GAUNTLETS_OF_POWER)
            || is_art(uarmg, ART_HAND_OF_VECNA)
            || wielding_artifact(ART_GIANTSLAYER)
            || wielding_artifact(ART_HARBINGER)
            || wielding_artifact(ART_SWORD_OF_KAS))
            result = STR19(25); /* 125 */
        else
            /* need non-zero here to avoid 'if(result==0)' below because
               that doesn't deal with Str encoding; the cap of 25 applied
               there would limit Str to 18/07 [18 + 7] */
            result = max(tmp, 3);
    } else if (chridx == A_CHA) {
        if (tmp < 18 && (gy.youmonst.data->mlet == S_NYMPH
                         || u.umonnum == PM_AMOROUS_DEMON))
            result = 18;
        else if ((uwep && (uwep->oprops & ITEM_EXCEL))
                 || (u.twoweap && uswapwep
                     && (uswapwep->oprops & ITEM_EXCEL))) {
            /* a wielded weapon of excellence forces charisma toward
               an extreme based on its beatitude */
            if (tmp > 6
                && ((uwep && uwep->cursed)
                    || (u.twoweap && uswapwep->cursed)))
                result = 6;
            else if (tmp < 18
                     && ((uwep && !uwep->blessed)
                         || (u.twoweap && !uswapwep->blessed)))
                result = 18;
            else if ((uwep && uwep->blessed)
                     || (u.twoweap && uswapwep->blessed))
                result = 25;
        }
    } else if (chridx == A_CON) {
        if (wielding_artifact(ART_OGRESMASHER))
            result = 25;
    } else if (chridx == A_INT || chridx == A_WIS) {
        /* Yes, this may raise Int and/or Wis if hero is sufficiently
           stupid.  There are lower levels of cognition than "dunce". */
        if (uarmh && uarmh->otyp == DUNCE_CAP)
            result = 6;
    } else if (chridx == A_DEX) {
        ; /* there aren't any special cases for dexterity */
    }

    if (result == 0) /* none of the special cases applied */
        result = (tmp >= 25) ? 25 : (tmp <= 3) ? 3 : tmp;

    return (schar) result;
}

/* condense clumsy ACURR(A_STR) value into value that fits into formulas */
schar
acurrstr(void)
{
    int str = ACURR(A_STR), /* 3..125 after massaging by acurr() */
        result; /* 3..25 */

    if (str <= STR18(0)) /* <= 18; max(,3) here is redundant */
        result = max(str, 3); /* 3..18 */
    else if (str <= STR19(21)) /* <= 121 */
        /* this converts
           18/01..18/31 into 19,
           18/32..18/81 into 20,
           18/82..18/100 and 19..21 into 21 */
        result = 19 + str / 50; /* map to 19..21 */
    else /* convert 122..125; min(,125) here is redundant */
        result = min(str, 125) - 100; /* 22..25 */

    return (schar) result;
}

/* when wearing (or taking off) an unID'd item, this routine is used
   to distinguish between observable +0 result and no-visible-effect
   due to an attribute not being able to exceed maximum or minimum */
boolean
extremeattr(
    int attrindx) /* does attrindx's value match its max or min? */
{
    /* Fixed_abil and racial MINATTR/MAXATTR aren't relevant here */
    int lolimit = 3, hilimit = 25, curval = ACURR(attrindx);

    /* upper limit for Str is 25 but its value is encoded differently */
    if (attrindx == A_STR) {
        hilimit = STR19(25); /* 125 */
        /* lower limit for Str can also be 25 */
        if ((uarmg && uarmg->otyp == GAUNTLETS_OF_POWER)
            || is_art(uarmg, ART_HAND_OF_VECNA)
            || wielding_artifact(ART_GIANTSLAYER)
            || wielding_artifact(ART_HARBINGER)
            || wielding_artifact(ART_SWORD_OF_KAS))
            lolimit = hilimit;
    } else if (attrindx == A_CON) {
        if (wielding_artifact(ART_OGRESMASHER))
            lolimit = hilimit;
    } else if (attrindx == A_CHA) {
        /* mirror acurr()'s A_CHA forced floor and ceiling so a +0 item
           put on while charisma is forced doesn't reveal its
           enchantment via a no-visible-change deduction */
        int tmp = u.abon.a[A_CHA] + u.atemp.a[A_CHA] + u.acurr.a[A_CHA];

        if ((gy.youmonst.data->mlet == S_NYMPH
             || u.umonnum == PM_AMOROUS_DEMON) && tmp < 18) {
            lolimit = hilimit = 18;
        } else if ((uwep && (uwep->oprops & ITEM_EXCEL))
                   || (u.twoweap && uswapwep
                       && (uswapwep->oprops & ITEM_EXCEL))) {
            if (tmp > 6
                && ((uwep && uwep->cursed)
                    || (u.twoweap && uswapwep->cursed)))
                lolimit = hilimit = 6;
            else if (tmp < 18
                     && ((uwep && !uwep->blessed)
                         || (u.twoweap && !uswapwep->blessed)))
                lolimit = hilimit = 18;
            else if ((uwep && uwep->blessed)
                     || (u.twoweap && uswapwep->blessed))
                lolimit = hilimit = 25;
        }
    }
    /* this exception is hypothetical; the only other worn item affecting
       Int or Wis is another helmet so can't be in use at the same time */
    if (attrindx == A_INT || attrindx == A_WIS) {
        if (uarmh && uarmh->otyp == DUNCE_CAP)
            hilimit = lolimit = 6;
    }

    /* are we currently at either limit? */
    return (curval == lolimit || curval == hilimit) ? TRUE : FALSE;
}

/*
 * Aasimar alignment abuse penalties.
 *
 * Each abuse tier imposes one random penalty: either strip
 * a positive racial intrinsic (FROMRACE) or impose a negative
 * one. Penalties persist until atonement reduces abuse below
 * that tier. If the pool is exhausted, stat max values are
 * reduced by 3 (one STAT_DRAIN max per character).
 *
 * Penalty IDs packed 6 bits each in u.uaasimar_penalties:
 * bits 0-5 = Slight, 6-11 = Moderate, 12-17 = Serious,
 * 18-23 = Severe, 24-29 = Grave.
 */
#define AASIMAR_PEN_NONE       0
#define AASIMAR_PEN_SEE_INVIS  1
#define AASIMAR_PEN_ANTIMAGIC  2
#define AASIMAR_PEN_COLD_RES   3
#define AASIMAR_PEN_SLEEP_RES  4
#define AASIMAR_PEN_POISON_RES 5
#define AASIMAR_PEN_SHOCK_RES  6
#define AASIMAR_PEN_HUNGER     7
#define AASIMAR_PEN_AGGRAVATE  8
#define AASIMAR_PEN_STAT_DRAIN 9
#define NUM_AASIMAR_PENALTIES  10
#define NUM_ABUSE_TIERS        5

/* Feedback channels: the penalty IDs above, then the clean-alignment
   grants of aasimar_check_abuse(). Changes made while polymorphed are
   told on return to normal form: u.uaasimar_pending holds a pending bit
   per channel and, from the first silent change on, whether the hero
   was penalized before it, so only a net change still in force is told
   (adjabil() can flip the same FROMRACE bit with its own message in
   between). Highest bit used is 28 */
#define AASIMAR_CH_DEATH_RES   10
#define AASIMAR_CH_WINGS       11
#define AASIMAR_CH_REGEN       12
#define NUM_AASIMAR_CHANNELS   13
#define AASIMAR_PENDING(ch)    (1L << (ch))
#define AASIMAR_WAS_PEN(ch)    (1L << ((ch) + 16))

#define AASIMAR_PEN_TIER(tier) \
    ((int)((u.uaasimar_penalties \
            >> ((tier) * 6)) & 0x3FL))
#define SET_AASIMAR_PEN_TIER(tier, val) \
    (u.uaasimar_penalties = \
        (u.uaasimar_penalties \
         & ~(0x3FL << ((tier) * 6))) \
        | ((long)(val) << ((tier) * 6)))

/* TRUE if this is a positive intrinsic penalty */
#define PEN_IS_POSITIVE(id) \
    ((id) >= AASIMAR_PEN_SEE_INVIS \
     && (id) <= AASIMAR_PEN_SHOCK_RES)

/* Map penalty ID to the property pointer it affects.
   Returns NULL for STAT_DRAIN (handled separately) */
staticfn long *
aasimar_pen_prop(int pen_id)
{
    switch (pen_id) {
    case AASIMAR_PEN_SEE_INVIS:
        return &HSee_invisible;
    case AASIMAR_PEN_ANTIMAGIC:
        return &HAntimagic;
    case AASIMAR_PEN_COLD_RES:
        return &HCold_resistance;
    case AASIMAR_PEN_SLEEP_RES:
        return &HSleep_resistance;
    case AASIMAR_PEN_POISON_RES:
        return &HPoison_resistance;
    case AASIMAR_PEN_SHOCK_RES:
        return &HShock_resistance;
    case AASIMAR_PEN_HUNGER:
        return &HHunger;
    case AASIMAR_PEN_AGGRAVATE:
        return &HAggravate_monster;
    default:
        return (long *) 0;
    }
}

/* Minimum player level for each positive intrinsic.
   Negative intrinsics have no level requirement */
staticfn int
aasimar_pen_minlevel(int pen_id)
{
    switch (pen_id) {
    case AASIMAR_PEN_SEE_INVIS:
        return 1;
    case AASIMAR_PEN_POISON_RES:
        return 1;
    case AASIMAR_PEN_ANTIMAGIC:
        return 4;
    case AASIMAR_PEN_SLEEP_RES:
        return 7;
    case AASIMAR_PEN_COLD_RES:
        return 12;
    case AASIMAR_PEN_SHOCK_RES:
        return 20;
    default:
        return 0;
    }
}

/* Message for a change on channel chan; penalized is TRUE when the hero
   is now worse off (positive intrinsic stripped, negative one imposed,
   stats drained, grant revoked) */
staticfn void
aasimar_feedback(int chan, boolean penalized)
{
    switch (chan) {
    case AASIMAR_PEN_SEE_INVIS:
        if (penalized)
            Your("eyesight feels diminished.");
        else
            You_feel("an odd sensation.");
        break;
    case AASIMAR_PEN_ANTIMAGIC:
        if (penalized)
            You_feel("vulnerable to magic!");
        else
            You_feel("resistant to magic!");
        break;
    case AASIMAR_PEN_COLD_RES:
        if (penalized)
            You_feel("cooler!");
        else
            You_feel("warm!");
        break;
    case AASIMAR_PEN_SLEEP_RES:
        if (penalized)
            You_feel("tired!");
        else
            You_feel("awake!");
        break;
    case AASIMAR_PEN_POISON_RES:
        if (penalized)
            You_feel("less hardy!");
        else
            You_feel("hardy!");
        break;
    case AASIMAR_PEN_SHOCK_RES:
        if (penalized)
            You_feel("conductive!");
        else
            You_feel("insulated!");
        break;
    case AASIMAR_PEN_HUNGER:
        if (penalized)
            You_feel("ravenously hungry!");
        else
            You("no longer feel quite so hungry.");
        break;
    case AASIMAR_PEN_AGGRAVATE:
        if (penalized)
            You_feel("that monsters are aware of your presence.");
        else
            You_feel(
                  "that monsters are no longer aware of your presence.");
        break;
    case AASIMAR_PEN_STAT_DRAIN:
        if (penalized)
            You_feel("your divine potential diminishing!");
        else
            You_feel("your divine potential restored!");
        break;
    case AASIMAR_CH_DEATH_RES:
        if (penalized)
            You_feel("the gift of immortality slipping away!");
        else
            You_feel("the gift of immortality returning to you!");
        break;
    case AASIMAR_CH_WINGS:
        if (penalized) {
            /* below level 15 lost levels took them, else the abuse */
            if (u.ulevel < 15)
                Your(
              "angelic wings fold inward and disappear beneath your skin.");
            else
                Your("angelic wings wither and fade away!");
            check_wings(TRUE);
        } else {
            pline("Angelic wings sprout from your back!");
            check_wings(FALSE);
        }
        break;
    case AASIMAR_CH_REGEN:
        if (penalized)
            You_feel("less resilient!");
        else
            You_feel("resilient!");
        break;
    }
}

/* A change on channel chan just happened: say so now, or while
   polymorphed remember it for aasimar_announce_pending(); restore's
   re-application of the stat drain is never remembered */
staticfn void
aasimar_changed(int chan, boolean penalized)
{
    /* the racial wings came or went, and when not polymorphed "Fly" may
       have with them; check_wings() only notices what its own armor
       block changes */
    if (chan == AASIMAR_CH_WINGS)
        disp.botl = TRUE;
    if (!Upolyd) {
        aasimar_feedback(chan, penalized);
    } else if (!program_state.restoring
               && !(u.uaasimar_pending & AASIMAR_PENDING(chan))) {
        u.uaasimar_pending |= AASIMAR_PENDING(chan);
        /* this change lifted it, so it was in force before */
        if (!penalized)
            u.uaasimar_pending |= AASIMAR_WAS_PEN(chan);
    }
}

/* Is the hero on the penalized side of channel chan right now? */
staticfn boolean
aasimar_penalized_now(int chan)
{
    long *prop;
    int i;

    switch (chan) {
    case AASIMAR_PEN_STAT_DRAIN:
        for (i = 0; i < A_MAX; i++)
            if (gu.urace.attrmax[i] != races[flags.initrace].attrmax[i])
                return TRUE;
        return FALSE;
    case AASIMAR_CH_DEATH_RES:
        return !(HDeath_resistance & FROMRACE);
    case AASIMAR_CH_WINGS:
        return !(HFlying & FROMRACE);
    case AASIMAR_CH_REGEN:
        return !(HRegeneration & FROMRACE);
    default:
        prop = aasimar_pen_prop(chan);
        if (!prop)
            return FALSE;
        return PEN_IS_POSITIVE(chan) ? !(*prop & FROMRACE)
                                     : ((*prop & FROMRACE) != 0L);
    }
}

/* Apply or remove one aasimar penalty.
   apply=TRUE imposes, apply=FALSE reverses */
staticfn void
apply_aasimar_penalty(int pen_id, boolean apply)
{
    long *prop;
    long prevval;
    int i;

    /* Stat drain -- no property pointer.
       Use absolute values from race definition
       so apply/reverse are idempotent */
    if (pen_id == AASIMAR_PEN_STAT_DRAIN) {
        boolean changed = FALSE;

        if (apply) {
            for (i = 0; i < A_MAX; i++) {
                int orig = races[flags.initrace].attrmax[i];
                int drained = orig - 3;

                if (drained < 3)
                    drained = 3;
                if (gu.urace.attrmax[i] != drained)
                    changed = TRUE;
                gu.urace.attrmax[i] = drained;
                if (ABASE(i) > ATTRMAX(i))
                    ABASE(i) = ATTRMAX(i);
                if (AMAX(i) > ATTRMAX(i))
                    AMAX(i) = ATTRMAX(i);
                /* while polymorphed those are the form's stats; the
                   normal-form ones polyman() restores must meet the
                   drained cap too */
                if (Upolyd) {
                    if (MCURR(i) > drained)
                        MCURR(i) = drained;
                    if (MMAX(i) > drained)
                        MMAX(i) = drained;
                }
            }
            if (changed && !program_state.restoring)
                aasimar_changed(AASIMAR_PEN_STAT_DRAIN, TRUE);
        } else {
            for (i = 0; i < A_MAX; i++) {
                if (gu.urace.attrmax[i]
                    != races[flags.initrace].attrmax[i])
                    changed = TRUE;
                gu.urace.attrmax[i] =
                    races[flags.initrace].attrmax[i];
            }
            if (changed)
                aasimar_changed(AASIMAR_PEN_STAT_DRAIN, FALSE);
        }
        return;
    }

    prop = aasimar_pen_prop(pen_id);
    if (!prop)
        return;

    prevval = *prop;

    if (PEN_IS_POSITIVE(pen_id)) {
        /* Strip FROMRACE to punish, restore to forgive */
        if (u.ulevel < aasimar_pen_minlevel(pen_id))
            return;
        if (apply)
            *prop &= ~FROMRACE;
        else
            *prop |= FROMRACE;
    } else {
        /* Negative: set FROMRACE to punish */
        if (apply)
            *prop |= FROMRACE;
        else
            *prop &= ~FROMRACE;
    }

    if (prevval != *prop) {
        aasimar_changed(pen_id, apply);
        postadjabil(prop);
    }
}

/* Compute the abuse tier (0-4) from current abuse value.
   Returns -1 if no abuse (all tiers reversed) */
staticfn int
abuse_tier(void)
{
    int val = (int) u.ualign.abuse;

    if (val <= 0)
        return -1;
    if (val < 5)
        return 0;  /* slight */
    if (val < 15)
        return 1;  /* moderate */
    if (val < 30)
        return 2;  /* serious */
    if (val < 50)
        return 3;  /* severe */
    return 4;       /* grave */
}

/* TRUE if any tier already has STAT_DRAIN assigned */
staticfn boolean
has_stat_drain_penalty(void)
{
    int t;

    for (t = 0; t < NUM_ABUSE_TIERS; t++)
        if (AASIMAR_PEN_TIER(t) == AASIMAR_PEN_STAT_DRAIN)
            return TRUE;
    return FALSE;
}

/* TRUE if the given property has an active aasimar
   abuse penalty stripping it */
staticfn boolean
aasimar_ability_penalized(long *prop)
{
    int t, pen_id;

    if (!Race_if(PM_AASIMAR))
        return FALSE;

    for (t = 0; t < NUM_ABUSE_TIERS; t++) {
        pen_id = AASIMAR_PEN_TIER(t);
        if (pen_id != AASIMAR_PEN_NONE
            && PEN_IS_POSITIVE(pen_id)
            && aasimar_pen_prop(pen_id) == prop)
            return TRUE;
    }
    return FALSE;
}

/* Check aasimar abuse penalties. Called when abuse
   changes (adjalign), atonement (priest.c), level
   change (adjabil), and save restore (restore.c) */
void
aasimar_check_abuse(void)
{
    int current_tier, t, pen_id;
    int eligible[AASIMAR_PEN_STAT_DRAIN]; /* IDs 1-8 */
    int neligible, i, j;
    boolean already_used, was_flying;

    if (!Race_if(PM_AASIMAR))
        return;

    was_flying = !!Flying;
    current_tier = abuse_tier();

    for (t = 0; t < NUM_ABUSE_TIERS; t++) {
        if (t <= current_tier) {
            if (AASIMAR_PEN_TIER(t)
                == AASIMAR_PEN_NONE) {
                /* New tier -- pick random penalty */
                neligible = 0;
                for (i = 1; i < AASIMAR_PEN_STAT_DRAIN; i++) {
                    already_used = FALSE;
                    for (j = 0; j < NUM_ABUSE_TIERS; j++) {
                        if (j != t && AASIMAR_PEN_TIER(j) == i) {
                            already_used = TRUE;
                            break;
                        }
                    }
                    if (already_used)
                        continue;
                    if (PEN_IS_POSITIVE(i)
                        && u.ulevel < aasimar_pen_minlevel(i))
                        continue;
                    eligible[neligible++] = i;
                }
                if (neligible > 0) {
                    pen_id = eligible[rn2(neligible)];
                } else if (!has_stat_drain_penalty()) {
                    pen_id = AASIMAR_PEN_STAT_DRAIN;
                } else {
                    /* all positive intrinsics, both negative
                       intrinsics, and stat drain are taken;
                       nothing left to assign -- defensively
                       leave the tier empty rather than
                       double-assign a HUNGER/AGGRAVATE that
                       the eligibility loop already rejected */
                    pen_id = AASIMAR_PEN_NONE;
                }
                if (pen_id != AASIMAR_PEN_NONE) {
                    SET_AASIMAR_PEN_TIER(t, pen_id);
                    apply_aasimar_penalty(pen_id, TRUE);
                }
            } else {
                /* Re-enforce existing penalty;
                   adjabil or restore may have
                   reset FROMRACE flags */
                apply_aasimar_penalty(AASIMAR_PEN_TIER(t), TRUE);
            }
        } else {
            /* Abuse below this tier -- reverse */
            pen_id = AASIMAR_PEN_TIER(t);
            if (pen_id != AASIMAR_PEN_NONE) {
                apply_aasimar_penalty(pen_id, FALSE);
                SET_AASIMAR_PEN_TIER(t, AASIMAR_PEN_NONE);
            }
        }
    }

    /* Death resistance from crowning: revoked on
       abuse, restored on full atonement */
    if (u.uevent.uhand_of_elbereth) {
        if (u.ualign.abuse > 0
            && (HDeath_resistance & FROMRACE)) {
            HDeath_resistance &= ~FROMRACE;
            aasimar_changed(AASIMAR_CH_DEATH_RES, TRUE);
        } else if (u.ualign.abuse == 0
                   && !(HDeath_resistance & FROMRACE)) {
            HDeath_resistance |= FROMRACE;
            aasimar_changed(AASIMAR_CH_DEATH_RES, FALSE);
        }
    }

    /* Flying from clean alignment: level 15+.
       Aasimar grow angelic wings as a body part; armor interaction
       matches crowned-Infidel demonic wings via check_wings() */
    if (u.ulevel >= 15) {
        if (u.ualign.abuse > 0
            && (HFlying & FROMRACE)) {
            HFlying &= ~FROMRACE;
            aasimar_changed(AASIMAR_CH_WINGS, TRUE);
        } else if (u.ualign.abuse == 0
                   && !(HFlying & FROMRACE)) {
            HFlying |= FROMRACE;
            aasimar_changed(AASIMAR_CH_WINGS, FALSE);
        }
    } else if (HFlying & FROMRACE) {
        /* Lost enough levels: revoke */
        HFlying &= ~FROMRACE;
        aasimar_changed(AASIMAR_CH_WINGS, TRUE);
    }

    /* Hungerless regeneration from clean alignment:
       level 18+ (FROMRACE regen has no hunger cost) */
    if (u.ulevel >= 18) {
        if (u.ualign.abuse > 0
            && (HRegeneration & FROMRACE)) {
            HRegeneration &= ~FROMRACE;
            aasimar_changed(AASIMAR_CH_REGEN, TRUE);
        } else if (u.ualign.abuse == 0
                   && !(HRegeneration & FROMRACE)) {
            HRegeneration |= FROMRACE;
            aasimar_changed(AASIMAR_CH_REGEN, FALSE);
        }
    } else if (HRegeneration & FROMRACE) {
        /* Lost enough levels: revoke */
        HRegeneration &= ~FROMRACE;
        aasimar_changed(AASIMAR_CH_REGEN, TRUE);
    }

    /* the angelic wings went; adjalign() runs this from deep inside
       combat (a murder in xkilled()), so let moveloop land the hero */
    if (was_flying && !Flying)
        defer_landing(LANDING_FLY);
}

/* Back in normal form (polyman()): tell the penalty and grant changes
   made while polymorphed whose net effect is still in force */
void
aasimar_announce_pending(void)
{
    long pending = u.uaasimar_pending;
    boolean now;
    int chan;

    u.uaasimar_pending = 0L;
    if (!pending || !Race_if(PM_AASIMAR))
        return;
    for (chan = AASIMAR_PEN_SEE_INVIS; chan < NUM_AASIMAR_CHANNELS;
         chan++) {
        if (!(pending & AASIMAR_PENDING(chan)))
            continue;
        now = aasimar_penalized_now(chan);
        if (now != ((pending & AASIMAR_WAS_PEN(chan)) != 0L))
            aasimar_feedback(chan, now);
    }
}

/* Will aasimar_announce_pending() tell of angelic wings sprouting? Asked
   just before it (which zeroes the record), after set_uasmon() has
   granted whatever racial flight the normal form has */
boolean
aasimar_wings_pending(void)
{
    long pending = u.uaasimar_pending;

    return (Race_if(PM_AASIMAR)
            && (pending & AASIMAR_PENDING(AASIMAR_CH_WINGS)) != 0L
            && (pending & AASIMAR_WAS_PEN(AASIMAR_CH_WINGS)) != 0L
            && (HFlying & FROMRACE) != 0L);
}

/* Another source (adjabil()'s level change, attrcurse()) just changed
   intrinsic prop with a message of its own; the player knows its state
   now, so a penalty change to it still waiting to be told is moot */
void
aasimar_change_told(long *prop)
{
    int chan;

    if (!u.uaasimar_pending)
        return;
    for (chan = AASIMAR_PEN_SEE_INVIS; chan < AASIMAR_PEN_STAT_DRAIN;
         chan++)
        if (aasimar_pen_prop(chan) == prop)
            u.uaasimar_pending &= ~(AASIMAR_PENDING(chan)
                                    | AASIMAR_WAS_PEN(chan));
}

/* An aasimar who deliberately permaconverts their alignment at a
   cross-aligned altar betrays their celestial heritage. The spurned
   deity strips their celestial blood: the race becomes human (one-way)
   and every aasimar power is lost. This mirrors the Infidel crowning
   that turns the hero into a demon (pray.c). Called from the altar
   conversion path after uchangealign(), the chokepoint for permanent
   alignment change */
void
aasimar_lose_celestial_blood(void)
{
    const struct innate *abil;
    int i;
    boolean was_flying = !!Flying;

    /* changes made in a form go untold, since every power is stripped
       below */
    u.uaasimar_pending = 0L;

    /* Become human before any set_uasmon()/check_wings(): those re-grant
       celestial abilities while Race_if(PM_AASIMAR) holds. Do not use
       adjabil() to strip the racial intrinsics either -- once the race
       is human its ability table is empty (so it cannot strip aasimar
       bits) and its trailing aasimar_check_abuse() would re-grant the
       clean-alignment intrinsics. races[0] is the human entry */
    gu.urace = races[0];

    /* Strip the FROMRACE intrinsic layer. The table-driven racial
       abilities come from amr_abil[]; the alignment-gated grants and
       abuse penalties imposed by aasimar_check_abuse() and
       apply_aasimar_penalty() live outside the table and are cleared
       explicitly. set_uasmon() below handles the separate FROMFORM
       layer for the new human form */
    for (abil = amr_abil; abil->ability; abil++)
        *(abil->ability) &= ~FROMRACE;
    HDeath_resistance &= ~FROMRACE;  /* crowning gift */
    HFlying &= ~FROMRACE;            /* angelic wings, level 15+ */
    HRegeneration &= ~FROMRACE;      /* hungerless regen, level 18+ */
    HHunger &= ~FROMRACE;            /* abuse penalty */
    HAggravate_monster &= ~FROMRACE; /* abuse penalty */
    u.uaasimar_penalties = 0L;

    /* a temporary polymorph ends as a human, so the angelic wings don't
       come back only to be lost ("You return to human form!") */
    if (Upolyd)
        rehumanize();

    /* Drop attributes to human ceilings (urace.attrmax is now human's),
       same clamp the stat-drain penalty uses; after the revert, which
       restores the normal form's attributes */
    for (i = 0; i < A_MAX; i++) {
        if (ABASE(i) > ATTRMAX(i))
            ABASE(i) = ATTRMAX(i);
        if (AMAX(i) > ATTRMAX(i))
            AMAX(i) = ATTRMAX(i);
    }

    /* Recompute the FROMFORM layer for the human form and refresh
       wings/flight; the race is human so no celestial grant returns */
    set_uasmon();
    newsym(u.ux, u.uy);
    disp.botl = TRUE;

    /* Permanent: re-forced as human on every restore by role_init() */
    u.uevent.ufallen_aasimar = 1;

    /* the angelic wings are gone; come down onto the altar */
    if (was_flying && !Flying)
        defer_landing(LANDING_FLY);
}

/* avoid possible problems with alignment overflow, and provide a centralized
   location for any future alignment limits */
void
adjalign(int n)
{
    int newalign = u.ualign.record + n;

    if (n < 0) {
        unsigned newabuse = u.ualign.abuse - n;

        if (newalign < u.ualign.record)
            u.ualign.record = newalign;
        if (newabuse > u.ualign.abuse) {
            u.ualign.abuse = newabuse;
            u.ever_abused = TRUE; /* for conduct tracking */
            adj_erinys(newabuse);
            aasimar_check_abuse();
        }
    } else if (newalign > u.ualign.record) {
        u.ualign.record = newalign;
        if (u.ualign.record > ALIGNLIM)
            u.ualign.record = (int)ALIGNLIM;
    }
}

/* Record alignment abuse event in circular buffer.
   For abuse events, penalty is negative. For atonement,
   penalty is positive (abuse points reduced) */
void
record_abuse_event(int penalty, int atype)
{
    record_abuse_event_dtl(penalty, atype, 0, 0);
}

/* Core abuse event recorder. The detail fields carry optional
   context such as a monster or object index (interpreted per event
   type by abuse_event_desc) and are always stored so that reused
   ring buffer slots never keep stale details */
void
record_abuse_event_dtl(int penalty, int atype, int detail1, int detail2)
{
    struct abuse_event *evt;

    /* Only record actual penalties, or atonement */
    if (penalty >= 0 && atype != ABUSE_ATONEMENT)
        return;

    evt = &u.uabuse_history[u.uabuse_hist_idx];
    evt->turn = svm.moves;
    evt->abuse_type = (short) atype;
    if (atype == ABUSE_ATONEMENT)
        evt->penalty = (short) penalty; /* store as-is for atonement */
    else
        evt->penalty = (short) (-penalty); /* store as positive for display */
    evt->detail1 = (short) detail1;
    evt->detail2 = (short) detail2;
    u.uabuse_hist_idx = (u.uabuse_hist_idx + 1) % MAX_ABUSE_HISTORY;
}

/* Record an abuse event involving a particular monster. The species
   is stored only if the hero can spot the monster, mirroring the
   "You feel guilty" versus "vague sense of guilt" message gating */
void
record_abuse_event_mon(int penalty, int atype, struct monst *mtmp)
{
    int detail1 = 0, detail2 = 0;

    if (mtmp && canspotmon(mtmp)) {
        detail1 = monsndx(mtmp->data) + 1;
        if ((atype == ABUSE_KILL_PRIEST || atype == ABUSE_ANGER_PRIEST)
            && mtmp->ispriest && has_epri(mtmp))
            detail2 = (Align2amask(EPRI(mtmp)->shralign) + 1)
                      | (mtmp->female ? 8 : 0);
    }
    record_abuse_event_dtl(penalty, atype, detail1, detail2);
}

/* Get human-readable name for abuse type */
const char *
abuse_type_name(int atype)
{
    switch (atype) {
    case ABUSE_ATTACK_HELPLESS:
        return "attacking a helpless creature";
    case ABUSE_ATTACK_PEACEFUL:
        return "attacking a peaceful creature";
    case ABUSE_ANGER_PEACEFUL:
        return "angering a peaceful creature";
    case ABUSE_ATTACK_WOODLAND:
        return "attacking a woodland creature";
    case ABUSE_ATTACK_ELBERETH:
        return "violating Elbereth truce";
    case ABUSE_KILL_PEACEFUL:
        return "killing a peaceful creature";
    case ABUSE_KILL_PEACEFUL_MALIGN:
        return "add'l penalty, killing a peaceful creature";
    case ABUSE_KILL_PET:
        return "murdering your pet";
    case ABUSE_KILL_PRIEST:
        return "killing a coaligned priest";
    case ABUSE_KILL_GUARDIAN:
        return "killing a quest guardian";
    case ABUSE_KILL_LEADER:
        return "killing your quest leader";
    case ABUSE_KILL_UNICORN:
        return "killing a same-aligned unicorn";
    case ABUSE_KILL_DEITY:
        return "killing your deity";
    case ABUSE_USE_POISON:
        return "using a poisoned weapon";
    case ABUSE_USE_DISEASE:
        return "using a diseased weapon";
    case ABUSE_LYING:
        return "lying to a guard";
    case ABUSE_SHOPLIFTING:
        return "shoplifting";
    case ABUSE_VANDALISM:
        return "vandalizing a shop";
    case ABUSE_PICKPOCKET:
        return "caught pickpocketing";
    case ABUSE_SAC_PET:
        return "sacrificing your pet";
    case ABUSE_SAC_SAME_RACE:
        return "same-race sacrifice";
    case ABUSE_GRAVE_ROB:
        return "grave robbing";
    case ABUSE_DESTROY_TREE:
        return "destroying a tree";
    case ABUSE_GLUTTONY:
        return "gluttony";
    case ABUSE_CANNIBALISM:
        return "drinking vampire blood";
    case ABUSE_GENOCIDE_HUMAN:
        return "genociding humans";
    case ABUSE_QUEST_BETRAYAL:
        return "betraying your quest leader";
    case ABUSE_COWARDICE:
        return "cowardice before Moloch";
    case ABUSE_STEED_DEATH:
        return "steed dying from hazard";
    case ABUSE_PET_PUSH:
        return "pushing pet into hazard";
    case ABUSE_SPELL_PEACEFUL:
        return "hostile spell on peaceful";
    case ABUSE_VECNA:
        return "invoking Vecna's power";
    case ABUSE_FORBIDDEN_WEAPON:
        return "using a forbidden weapon";
    case ABUSE_INFIDEL_GOOD:
        return "releasing the Ice Queen from her curse";
    case ABUSE_WRONG_ALTAR:
        return "praying at wrong altar";
    case ABUSE_ALIGNMENT_HELM:
        return "donning a helm of opposite alignment";
    case ABUSE_DECEPTION:
        return "attempted deception";
    case ABUSE_AMULET_BETRAYAL:
        return "offering Amulet to wrong deity";
    case ABUSE_SAC_REJECTED:
        return "rejected Amulet sacrifice";
    case ABUSE_NEGLECT_OFFERING:
        return "neglecting sacrifice to Moloch";
    case ABUSE_REFUSE_TITHE:
        return "refusing temple contribution";
    case ABUSE_HISTORIC_STATUE:
        return "damaging a historic statue";
    case ABUSE_ATTACK_ENCHANTRESS:
        return "attacking the Enchantress";
    case ABUSE_VEGETARIAN:
        return "violating vegetarian vow";
    case ABUSE_ATONEMENT:
        return "atonement";
    case ABUSE_FORSAKE_DEITY:
        return "forsaking your deity";
    case ABUSE_ANGER_PRIEST:
        return "angering a coaligned priest";
    case ABUSE_USE_TAINTED:
        return "using a tainted weapon";
    default:
        return "transgression";
    }
}

DISABLE_WARNING_FORMAT_NONLITERAL

/* Compose the most specific available description of an abuse event
   into buf, which must hold at least BUFSZ characters. Falls back to
   the generic abuse_type_name() text when no valid detail was
   recorded, which is always the case for entries from older saves */
void
abuse_event_desc(struct abuse_event *evt, char *buf)
{
    int atype = (int) evt->abuse_type;
    int d1 = (int) evt->detail1;
    int d2 = (int) evt->detail2;
    const char *generic = abuse_type_name(atype);
    const char *fmt = (const char *) 0;
    boolean parens = FALSE;
    struct permonst *ptr;

    Strcpy(buf, generic);

    switch (atype) {
    case ABUSE_ATTACK_HELPLESS:
        fmt = "attacking a helpless %s";
        break;
    case ABUSE_ATTACK_PEACEFUL:
        fmt = "attacking a peaceful %s";
        break;
    case ABUSE_ANGER_PEACEFUL:
        fmt = "angering a peaceful %s";
        break;
    case ABUSE_ATTACK_WOODLAND:
        fmt = "attacking a woodland %s";
        break;
    case ABUSE_KILL_PEACEFUL:
        fmt = "killing a peaceful %s";
        break;
    case ABUSE_KILL_PEACEFUL_MALIGN:
        fmt = "add'l penalty, killing a peaceful %s";
        break;
    case ABUSE_KILL_PET:
        fmt = "murdering your pet %s";
        break;
    case ABUSE_KILL_UNICORN:
        fmt = "killing a same-aligned %s";
        break;
    case ABUSE_SPELL_PEACEFUL:
        fmt = "hostile spell on a peaceful %s";
        break;
    case ABUSE_SAC_PET:
        fmt = "sacrificing your pet %s";
        break;
    case ABUSE_PET_PUSH:
        fmt = "pushing your pet %s into hazard";
        break;
    case ABUSE_ATTACK_ELBERETH:
    case ABUSE_KILL_GUARDIAN:
    case ABUSE_HISTORIC_STATUE:
        parens = TRUE;
        break;
    case ABUSE_STEED_DEATH:
        if (d1 > 0 && d1 <= NUMMONS) {
            const char *mname = mons[d1 - 1].pmnames[NEUTRAL];

            if (d2 == ABUSE_HAZ_WATER)
                Sprintf(buf, "your %s drowning", mname);
            else if (d2 == ABUSE_HAZ_LAVA)
                Sprintf(buf, "your %s dying in lava", mname);
            else if (d2 == ABUSE_HAZ_FALL)
                Sprintf(buf, "your %s falling to its death", mname);
            else
                Sprintf(buf, "your %s dying from hazard", mname);
        }
        return;
    case ABUSE_KILL_PRIEST:
    case ABUSE_ANGER_PRIEST:
        if ((d2 & 7) >= 1 && (d2 & 7) <= 5) {
            const char *gname = align_gname(Amask2align((d2 & 7) - 1));
            const char *job = (d1 - 1 == PM_HIGH_PRIEST)
                                  ? ((d2 & 8) ? "high priestess"
                                              : "high priest")
                                  : ((d2 & 8) ? "priestess" : "priest");

            Sprintf(buf, "%s a %s of %s",
                    (atype == ABUSE_KILL_PRIEST) ? "killing" : "angering",
                    job, gname);
        }
        return;
    case ABUSE_GENOCIDE_HUMAN:
        if (d1 > 0 && d1 <= NUMMONS)
            Sprintf(buf, "genociding %s",
                    makeplural(mons[d1 - 1].pmnames[NEUTRAL]));
        return;
    case ABUSE_FORBIDDEN_WEAPON:
    case ABUSE_USE_POISON:
    case ABUSE_USE_TAINTED:
        if (d1 > 0 && d1 <= NUM_OBJECTS)
            Sprintf(buf, "%s (%s)", generic, simple_typename(d1 - 1));
        return;
    case ABUSE_WRONG_ALTAR:
        if (d1 >= 1 && d1 <= 5)
            Sprintf(buf, "%s (%s)", generic,
                    align_gname(Amask2align((d1 - 1))));
        return;
    default:
        return;
    }

    /* the remaining cases carry a monster species index in detail1 */
    if (d1 < 1 || d1 > NUMMONS)
        return;
    ptr = &mons[d1 - 1];
    if (type_is_pname(ptr))
        Sprintf(buf, "%s (%s)", generic, ptr->pmnames[NEUTRAL]);
    else if (the_unique_pm(ptr))
        Sprintf(buf, "%s (the %s)", generic, ptr->pmnames[NEUTRAL]);
    else if (parens)
        Sprintf(buf, "%s (%s)", generic, ptr->pmnames[NEUTRAL]);
    else if (fmt)
        Sprintf(buf, fmt, ptr->pmnames[NEUTRAL]);
}

RESTORE_WARNING_FORMAT_NONLITERAL

/* change hero's alignment type, possibly losing use of artifacts */
void
uchangealign(
    int newalign,
    int reason) /* A_CG_CONVERT, A_CG_HELM_ON, or A_CG_HELM_OFF */
{
    aligntyp oldalign = u.ualign.type;

    u.ublessed = 0; /* lose divine protection */
    /* You/Your/pline message with call flush_screen(), triggering bot(),
       so the actual data change needs to come before the message */
    disp.botl = TRUE; /* status line needs updating */
    if (reason == A_CG_CONVERT) {
        /* conversion via altar */
        livelog_printf(LL_ALIGNMENT, "permanently converted to %s",
                       aligns[1 - newalign].adj);
        u.ualignbase[A_CURRENT] = (aligntyp) newalign;
        /* worn helm of opposite alignment might block change */
        if (!uarmh || uarmh->otyp != HELM_OF_OPPOSITE_ALIGNMENT)
            u.ualign.type = u.ualignbase[A_CURRENT];
        You("have a %ssense of a new direction.",
            (u.ualign.type != oldalign) ? "sudden " : "");
    } else {
        /* putting on or taking off a helm of opposite alignment */
        u.ualign.type = (aligntyp) newalign;
        if (reason == A_CG_HELM_ON) {
            adjalign(-7); /* for abuse -- record will be cleared shortly */
            Your("mind oscillates %s.", Hallucination ? "wildly" : "briefly");
            make_confused(rn1(2, 3), FALSE);
            if (Is_astralevel(&u.uz) || ((unsigned) rn2(50) < u.ualign.abuse))
                summon_furies(Is_astralevel(&u.uz) ? 0 : 1);
            /* don't livelog taking it back off */
            livelog_printf(LL_ALIGNMENT, "used a helm to turn %s",
                           aligns[1 - newalign].adj);
        } else if (reason == A_CG_HELM_OFF) {
            Your("mind is %s.", Hallucination
                                    ? "much of a muchness"
                                    : "back in sync with your body");
        }
    }
    if (u.ualign.type != oldalign) {
        u.ualign.record = 0; /* slate is wiped clean */
        retouch_equipment(0);
    }
}

/*attrib.c*/
