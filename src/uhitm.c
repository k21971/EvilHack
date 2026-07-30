/* NetHack 5.0	uhitm.c	$NHDT-Date: 1781973071 2026/06/20 16:31:11 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.503 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2012. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

static const char brief_feeling[] =
    "have a %s feeling for a moment, then it passes.";

staticfn boolean mhitm_mgc_atk_negated(struct monst *, struct monst *,
                                     boolean) NONNULLPTRS;
staticfn boolean known_hitum(struct monst *, struct obj *, int *, int, int,
                           struct attack *, int) NONNULLARG13;
staticfn boolean theft_petrifies(struct obj *) NONNULLARG1;
staticfn void mhitm_really_poison(struct monst *, struct attack *,
                                  struct monst *, struct mhitm_data *);
staticfn boolean steal_slips_off(struct monst *, struct obj *) NONNULLPTRS;
staticfn void pickpocket_watch_responds(struct monst *) NONNULLARG1;
staticfn void steal_it(struct monst *, struct attack *) NONNULLARG1;
staticfn struct monst *find_steal_target(void);
/* hitum_cleave() has contradictory information. There's a comment
 * beside the 1st arg 'target' stating non-null, but later on there
 * is a test for 'target' being null */
staticfn boolean hitum_cleave(struct monst *, struct attack *) NO_NNARGS;
staticfn boolean double_punch(void);
staticfn boolean hitum(struct monst *, struct attack *) NONNULLARG1;
staticfn void hmon_hitmon_barehands(struct _hitmon_data *,
                             struct monst *) NONNULLARG12;
staticfn void hmon_hitmon_weapon_ranged(struct _hitmon_data *, struct monst *,
                             struct obj *) NONNULLARG123;
staticfn boolean backstabbable(struct monst *) NONNULLARG1;
staticfn void hmon_hitmon_weapon_melee(struct _hitmon_data *, struct monst *,
                             struct obj *) NONNULLARG123;
staticfn void hmon_hitmon_weapon(struct _hitmon_data *, struct monst *,
                             struct obj *) NONNULLARG123;
staticfn void hmon_hitmon_potion(struct _hitmon_data *, struct monst *,
                             struct obj *) NONNULLARG123;
staticfn void hmon_hitmon_misc_obj(struct _hitmon_data *, struct monst *,
                             struct obj *) NONNULLARG123;
staticfn void hmon_hitmon_do_hit(struct _hitmon_data *, struct monst *,
                             struct obj *) NONNULLARG12;
staticfn void hmon_hitmon_dmg_recalc(struct _hitmon_data *, struct obj *);
staticfn void hmon_hitmon_poison(struct _hitmon_data *, struct monst *,
                             struct obj *) NONNULLARG123;
staticfn void hmon_hitmon_jousting(struct _hitmon_data *, struct monst *,
                             struct obj *) NONNULLARG123;
staticfn void hmon_hitmon_stagger(struct _hitmon_data *, struct monst *,
                             struct obj *) NONNULLARG12;
staticfn void hmon_hitmon_pet(struct _hitmon_data *, struct monst *,
                             struct obj *) NONNULLARG12;
staticfn void hmon_hitmon_splitmon(struct _hitmon_data *, struct monst *,
                             struct obj *) NONNULLARG12;
staticfn void hmon_hitmon_msg_hit(struct _hitmon_data *, struct monst *,
                             struct obj *) NONNULLARG12;
staticfn void hmon_hitmon_msg_lightobj(struct _hitmon_data *, struct monst *,
                             struct obj *) NONNULLARG12;
staticfn boolean hmon_hitmon(struct monst *, struct obj *, int, int)
                             NONNULLARG1;
staticfn int joust(struct monst *, struct obj *) NONNULLARG12;
staticfn void demonpet(void);
staticfn boolean m_slips_free(struct monst *, struct attack *) NONNULLPTRS;
staticfn void start_engulf(struct monst *) NONNULLARG1;
staticfn void end_engulf(void);
staticfn int gulpum(struct monst *, struct attack *) NONNULLPTRS;
staticfn boolean hmonas(struct monst *, int, boolean) NONNULLARG1;
staticfn void nohandglow(struct monst *) NONNULLARG1;
staticfn void first_weapon_hit(struct obj *) NONNULLARG1;
staticfn boolean shade_aware(struct obj *) NO_NNARGS;

#define PROJECTILE(obj) ((obj) && is_ammo(obj))

staticfn boolean
mhitm_mgc_atk_negated(
    struct monst *magr, struct monst *mdef,
    boolean verbosely) /* give mesg if magical cancellation prevents damage */
{
    int armpro;
    boolean negated;

    /* mcan doesn't apply to youmonst; hero can't be cancelled */
    if (magr != &gy.youmonst && magr->mcan)
        return TRUE; /* no message if attacker has been cancelled */

    armpro = magic_negation(mdef);
    negated = !(rn2(10) >= 3 * armpro);
    if (negated) {
        /* attack has been thwarted by negation, aka magical cancellation */
        if (verbosely) {
            if (mdef == &gy.youmonst)
                You("avoid harm.");
            else if (gv.vis && canseemon(mdef))
                pline_mon(mdef, "%s avoids harm.", Monnam(mdef));
        }
        return TRUE;
    }
    return FALSE;
}

/* multi_reason is usually a literal string; here we generate one that
   has the causing monster's type included */
void
dynamic_multi_reason(struct monst *mon, const char *verb, boolean by_gaze)
{
    /* combination of noname_monnam() and m_monnam(), more or less;
       accurate regardless of visibility or hallucination (only seen
       if game ends) and without personal name (M2_PNAME excepted) */
    char *who = x_monnam(mon, ARTICLE_A, (char *) 0,
                         (SUPPRESS_IT | SUPPRESS_INVISIBLE
                          | SUPPRESS_HALLUCINATION | SUPPRESS_SADDLE
                          | SUPPRESS_BARDING | SUPPRESS_NAME),
                         FALSE),
         *p = gm.multireasonbuf;

    /* prefix info for done_in_by() */
    Sprintf(p, "%u:", mon->m_id);
    p = eos(p);
    Sprintf(p, "%s by %s%s", verb,
            !by_gaze ? who : s_suffix(who),
            !by_gaze ? "" : " gaze");
    gm.multi_reason = p;
}

void
erode_armor(struct monst *mdef, int hurt)
{
    struct obj *target;

    /* wielding Harbinger protects worn armor */
    if ((mdef == &gy.youmonst)
            ? wielding_artifact(ART_HARBINGER)
            : mon_wielding_artifact(mdef, ART_HARBINGER))
        return;

    /* What the following code does: it keeps looping until it
     * finds a target for the rust monster.
     * Head, feet, etc... not covered by metal, or covered by
     * rusty metal, are not targets.  However, your body always
     * is, no matter what covers it.
     */
    while (1) {
        switch (rn2(5)) {
        case 0:
            target = which_armor(mdef, W_ARMH);
            if (!target
                || erode_obj(target, xname(target), hurt, EF_GREASE)
                       == ER_NOTHING)
                continue;
            break;
        case 1:
            target = which_armor(mdef, W_ARMC);
            if (target) {
                (void) erode_obj(target, xname(target), hurt,
                                 EF_GREASE | EF_VERBOSE);
                break;
            }
            if ((target = which_armor(mdef, W_ARM)) != (struct obj *) 0) {
                (void) erode_obj(target, xname(target), hurt,
                                 EF_GREASE | EF_VERBOSE);
            } else if ((target = which_armor(mdef, W_ARMU))
                       != (struct obj *) 0) {
                (void) erode_obj(target, xname(target), hurt,
                                 EF_GREASE | EF_VERBOSE);
            }
            break;
        case 2:
            target = which_armor(mdef, W_ARMS);
            if (!target
                || erode_obj(target, xname(target), hurt, EF_GREASE)
                       == ER_NOTHING)
                continue;
            break;
        case 3:
            target = which_armor(mdef, W_ARMG);
            if (!target
                || erode_obj(target, xname(target), hurt, EF_GREASE)
                       == ER_NOTHING)
                continue;
            break;
        case 4:
            target = which_armor(mdef, W_ARMF);
            if (!target
                || erode_obj(target, xname(target), hurt, EF_GREASE)
                       == ER_NOTHING)
                continue;
            break;
        }
        break; /* Out of while loop */
    }
}

/* FALSE means it's OK to attack */
boolean
attack_checks(
    struct monst *mtmp, /* target */
    struct obj *wep)    /* uwep for do_attack(), null for kick_monster() */
{
    int glyph;

    /* if you're close enough to attack, alert any waiting monster */
    mtmp->mstrategy &= ~STRAT_WAITMASK;

    if (engulfing_u(mtmp))
        return FALSE;

    if (svc.context.forcefight) {
        /* Do this in the caller, after we have checked that the monster
         * didn't die from the blow.  Reason: putting the 'I' there
         * causes the hero to forget the square's contents since
         * both 'I' and remembered contents are stored in .glyph.
         * If the monster dies immediately from the blow, the 'I' will
         * not stay there, so the player will have suddenly forgotten
         * the square's contents for no apparent reason.
        if (!canspotmon(mtmp)
            && !glyph_is_invisible(levl[gb.bhitpos.x][gb.bhitpos.y].glyph))
            map_invisible(gb.bhitpos.x, gb.bhitpos.y);
         */
        return FALSE;
    }

    /* cache the shown glyph;
       cases which might change it (by placing or removing
       'remembered, unseen monster' glyph or revealing a mimic)
       always return without further reference to this */
    glyph = glyph_at(gb.bhitpos.x, gb.bhitpos.y);

    /* Put up an invisible monster marker, but with exceptions for
     * monsters that hide and monsters you've been warned about.
     * The former already prints a warning message and
     * prevents you from hitting the monster just via the hidden monster
     * code below; if we also did that here, similar behavior would be
     * happening two turns in a row.  The latter shows a glyph on
     * the screen, so you know something is there.
     */
    if (!canspotmon(mtmp)
        && !glyph_is_warning(glyph) && !glyph_is_invisible(glyph)
        && !(!Blind && mtmp->mundetected && hides_under(mtmp->data))
        /* an unseen rider on a steed you can see is not a surprise;
           the steed is drawn at the square and gives it away */
        && (!has_erid(mtmp) || !canspotmon(ERID(mtmp)->mon_steed))) {
        pline("Wait!  There's %s there you can't see!", something);
        map_invisible(gb.bhitpos.x, gb.bhitpos.y);
        /* if it was an invisible mimic, treat it as if we stumbled
         * onto a visible mimic
         */
        if (M_AP_TYPE(mtmp) && !Protection_from_shape_changers) {
            if (!u.ustuck && !mtmp->mflee && dmgtype(mtmp->data, AD_STCK)
                /* applied pole-arm attack is too far to get stuck */
                && m_next2u(mtmp))
                set_ustuck(mtmp);
        }
        /* #H7329 - if hero is on engraved "Elbereth", this will end up
         * assessing an alignment penalty and removing the engraving
         * even though no attack actually occurs.  Since it also angers
         * peacefuls, we're operating as if an attack attempt did occur
         * and the Elbereth behavior is consistent.
         */
        /* waking up is always necessary and also un-mimics mimics;
           a remembered monster glyph means the hero knew something
           was here, so the bump is a knowing attack - otherwise
           charge luck rather than alignment. Must test the glyph
           cached above; map_invisible() has already placed an 'I'
           on the map */
        if (glyph_is_monster(glyph))
            wakeup(mtmp, TRUE);
        else
            wakeup_accidental(mtmp);
        return TRUE;
    }

    if (M_AP_TYPE(mtmp) && !Protection_from_shape_changers && !sensemon(mtmp)
        && !glyph_is_warning(glyph)) {
        /* If a hidden mimic was in a square where a player remembers
         * some (probably different) unseen monster, the player is in
         * luck--he attacks it even though it's hidden.
         */
        if (glyph_is_invisible(glyph)) {
            seemimic(mtmp);
            return FALSE;
        }
        stumble_onto_mimic(mtmp);
        return TRUE;
    }

    if (mtmp->mundetected && !canseemon(mtmp)
        && !glyph_is_warning(glyph)
        && (hides_under(mtmp->data) || mtmp->data->mlet == S_EEL)) {
        mtmp->mundetected = mtmp->msleeping = 0;
        newsym(mtmp->mx, mtmp->my);
        if (glyph_is_invisible(glyph)) {
            seemimic(mtmp);
            return FALSE;
        }
        if (!tp_sensemon(mtmp) && !Detect_monsters) {
            struct obj *obj;
            char lmonbuf[BUFSZ];
            boolean notseen;

            Strcpy(lmonbuf, l_monnam(mtmp));
            /* might be unseen if invisible and hero can't see invisible */
            notseen = !strcmp(lmonbuf, "it"); /* note: not strcmpi() */
            if (!Blind && Hallucination)
                pline("A %s %s %s!", mtmp->mtame ? "tame" : "wild",
                      notseen ? "creature" : (const char *) lmonbuf,
                      notseen ? "is present" : "appears");
            else if (Blind || (is_pool(mtmp->mx, mtmp->my) && !Underwater))
                pline("Wait!  There's a hidden monster there!");
            else if ((obj = svl.level.objects[mtmp->mx][mtmp->my]) != 0)
                pline("Wait!  There's %s hiding under %s!",
                      notseen ? something : (const char *) an(lmonbuf),
                      doname(obj));
            return TRUE;
        }
    }

    /*
     * make sure to wake up a monster from the above cases if the
     * hero can sense that the monster is there.
     */
    if ((mtmp->mundetected || M_AP_TYPE(mtmp)) && sensemon(mtmp)) {
        mtmp->mundetected = 0;
        wakeup(mtmp, TRUE);
    }

    if (flags.confirm && mtmp->mpeaceful
        && !Confusion && !Hallucination && !Stunned) {
        /* Intelligent chaotic weapons (Stormbringer, The Sword of Kas)
           want blood */
        if (wep && (wielding_artifact(ART_STORMBRINGER)
                    || wielding_artifact(ART_SWORD_OF_KAS))) {
            go.override_confirmation = TRUE;
            return FALSE;
        }
        if (canspotmon(mtmp)) {
            char qbuf[QBUFSZ];

            Sprintf(qbuf, "Really attack %s?", mon_nam(mtmp));
            if (!paranoid_query(ParanoidHit, qbuf)) {
                svc.context.move = 0;
                return TRUE;
            }
        }
    }

    return FALSE;
}

/* it is unchivalrous for a knight to attack the defenseless or from behind */
void
check_caitiff(struct monst *mtmp)
{
    if (u.ualign.record <= -10)
        return;

    if (Role_if(PM_KNIGHT) && u.ualign.type == A_LAWFUL
        && !is_undead(mtmp->data)
        && (helpless(mtmp)
            || (mtmp->mflee && !mtmp->mavenge))) {
        You("caitiff!");
        adjalign(-1);
        record_abuse_event_mon(-1, ABUSE_ATTACK_HELPLESS, mtmp);
    } else if (Role_if(PM_SAMURAI) && mtmp->mpeaceful) {
        /* attacking peaceful creatures is bad for the samurai's giri */
        You("dishonorably attack the innocent!");
        adjalign(-1);
        record_abuse_event_mon(-1, ABUSE_ATTACK_PEACEFUL, mtmp);
    } else if (Role_if(PM_DRUID) && mtmp->mpeaceful
               && (is_woodland_creature(mtmp->data)
                   || is_woodland_being(mtmp->data)
                   || mtmp->data == &mons[PM_DRUID])) {
        /* attacking peaceful woodland creatures is bad for Druids */
        You("dare to attack your woodland brethren!");
        adjalign(-1);
        record_abuse_event_mon(-1, ABUSE_ATTACK_WOODLAND, mtmp);
    }
}

/* maybe unparalyze monster */
void
mon_maybe_unparalyze(struct monst *mtmp)
{
    if (!mtmp->mcanmove) {
        if (!rn2(10)) {
            mtmp->mcanmove = 1;
            mtmp->mfrozen = 0;
        }
    }
}

/* how easy it is for hero to hit a monster,
   using attack type aatyp and/or weapon.
   larger value == easier to hit */
int
find_roll_to_hit(
    struct monst *mtmp,
    uchar aatyp,        /* usually AT_WEAP or AT_KICK */
    struct obj *weapon, /* uwep or uswapwep or NULL */
    int *attk_count,
    int *role_roll_penalty)
{
    int tmp, tmp2;

    *role_roll_penalty = 0; /* default is `none' */

    tmp = 1 + abon() + find_mac(mtmp) + u.uhitinc
          + (sgn(Luck) * ((abs(Luck) + 2) / 3))
          + maybe_polyd(gy.youmonst.data->mlevel, u.ulevel);

    /* some actions should occur only once during multiple attacks */
    if (!(*attk_count)++) {
        /* knight's chivalry or samurai's giri */
        check_caitiff(mtmp);
    }

    /* adjust vs. monster state */
    if (mtmp->mstun)
        tmp += 2;
    if (mtmp->mflee)
        tmp += 2;
    if (mtmp->msleeping)
        tmp += 2;
    if (!mtmp->mcanmove)
        tmp += 4;

    /* role/race adjustments */
    if (Role_if(PM_MONK) && !Upolyd) {
        if (uarm)
            tmp -= (*role_roll_penalty = gu.urole.spelarmr);
        else if (!uwep && !uarms)
            tmp += (u.ulevel / 3) + 2;
    }
    /* elves get a slight bonus to-hit vs orcs */
    if (racial_orc(mtmp)
        && (maybe_polyd(is_elf(gy.youmonst.data), Race_if(PM_ELF))
            || maybe_polyd(is_drow(gy.youmonst.data), Race_if(PM_DROW))))
        tmp++;

    /* orcs should get the same to-hit bonus vs elves */
    if ((racial_elf(mtmp) || racial_drow(mtmp))
        && maybe_polyd(is_orc(gy.youmonst.data), Race_if(PM_ORC)))
        tmp++;

    /* drow get a to-hit bonus if surrounded by darkness
       and using their racial weaponry, otherwise no bonus,
       and they take a penalty while in the light */
    if (maybe_polyd(is_drow(gy.youmonst.data), Race_if(PM_DROW))) {
        if (spot_is_dark(u.ux, u.uy)) { /* spot is dark */
            if (!uwep || is_drow_weapon(uwep)
                || uwep->oartifact == ART_SHADOWBLADE)
                tmp += (u.ulevel / 3) + 2;
        } else {
            /* spot is lit */
            tmp -= 4;
        }
    }

    /* encumbrance: with a lot of luggage, your agility diminishes */
    if ((tmp2 = near_capacity()) != 0)
        tmp -= (tmp2 * 2) - 1;
    if (u.utrap)
        tmp -= 3;

    /* Druids that #wildshape into one of their allowed forms
       enjoy a bonus that scales as they level up */
    if (!uwep && druid_form)
        tmp += (u.ulevel / 3) + 5;

    /* same with Vampires that #shapechange */
    if (!uwep && vampire_form)
        tmp += (u.ulevel / 3) + 5;

    /* higher quality weapons have a slightly greater
       chance to hit, inferior ones not so much */
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

    /*
     * hitval applies if making a weapon attack while wielding a weapon;
     * weapon_hit_bonus applies if doing a weapon attack even bare-handed
     * or if kicking as martial artist
     */
    if (aatyp == AT_WEAP || aatyp == AT_CLAW) {
        if (weapon)
            tmp += hitval(weapon, mtmp);
        tmp += weapon_hit_bonus(weapon);
    } else if (aatyp == AT_KICK && martial_bonus()) {
        tmp += weapon_hit_bonus((struct obj *) 0);
    }

    return tmp;
}

/* temporarily override 'safepet' (by faking use of 'F' prefix) when possibly
   unintentionally attacking peaceful monsters and optionally pets */
boolean
force_attack(struct monst *mtmp, boolean pets_too)
{
    boolean attacked, save_Forcefight;

    save_Forcefight = svc.context.forcefight;
    /* always set forcefight On for hostiles and peacefuls, maybe for pets */
    if (pets_too || !mtmp->mtame)
        svc.context.forcefight = TRUE;
    attacked = do_attack(mtmp);
    svc.context.forcefight = save_Forcefight;
    return attacked;
}

/* try to attack; return False if monster evaded;
   u.dx and u.dy must be set */
boolean
do_attack(struct monst *mtmp)
{
    struct permonst *mdat = mtmp->data;

    /* This section of code provides protection against accidentally
     * hitting peaceful (like '@') and tame (like 'd') monsters.
     * Protection is provided as long as player is not: blind, confused,
     * hallucinating or stunned.
     * changes by wwp 5/16/85
     * More changes 12/90, -dkh-. if its tame and safepet, (and protected
     * 07/92) then we assume that you're not trying to attack. Instead,
     * you'll usually just swap places if this is a movement command
     */
    /* Intelligent chaotic weapons (Stormbringer, The Sword of Kas)
       want blood */
    if (is_safemon(mtmp) && !svc.context.forcefight) {
        if (!(wielding_artifact(ART_STORMBRINGER)
              || wielding_artifact(ART_SWORD_OF_KAS))) {
            /* There are some additional considerations: this won't work
             * if in a shop or Punished or you miss a random roll or
             * if you can walk thru walls and your pet cannot (KAA) or
             * if your pet is a long worm with a tail.
             * There's also a chance of displacing a "frozen" monster:
             * sleeping monsters might magically walk in their sleep.
             * This block of code used to only be called for pets; now
             * that it also applies to peacefuls, non-pets mustn't be
             * forced to flee.
             */
            boolean foo = (Punished || !rn2(7)
                           || (is_longworm(mtmp->data) && mtmp->wormno)
                           || (IS_OBSTRUCTED(levl[u.ux][u.uy].typ)
                               && !passes_walls(mtmp->data))),
                    inshop = FALSE;
            char *p;

            /* only check for in-shop if don't already have reason to stop */
            if (!foo) {
                for (p = in_rooms(mtmp->mx, mtmp->my, SHOPBASE); *p; p++)
                    if (tended_shop(&svr.rooms[*p - ROOMOFFSET])) {
                        inshop = TRUE;
                        break;
                    }
            }
            if (inshop || foo) {
                char buf[BUFSZ];

                if (!svc.context.travel && !svc.context.run)
                    if (canspotmon(mtmp) && mtmp->isshk)
                        return ECMD_TIME | dopay();

                if (mtmp->mtame) /* see 'additional considerations' above */
                    monflee(mtmp, rnd(6), FALSE, FALSE);
                Strcpy(buf, y_monnam(mtmp));
                buf[0] = highc(buf[0]);
                You("stop.  %s is in the way!", buf);
                end_running(TRUE);
                return TRUE;
            } else if (mtmp->mfrozen || helpless(mtmp)
                       || (mtmp->data->mmove == 0 && rn2(6))) {
                pline("%s doesn't seem to move!", Monnam(mtmp));
                end_running(TRUE);
                return TRUE;
            } else
                return FALSE;
        }
    }

    /* possibly set in attack_checks;
       examined in known_hitum, called via hitum or hmonas below */
    go.override_confirmation = FALSE;
    /* attack_checks() used to use <u.ux+u.dx,u.uy+u.dy> directly, now
       it uses gb.bhitpos instead; it might map an invisible monster there */
    gb.bhitpos.x = u.ux + u.dx;
    gb.bhitpos.y = u.uy + u.dy;
    gn.notonhead = (gb.bhitpos.x != mtmp->mx || gb.bhitpos.y != mtmp->my);
    if (attack_checks(mtmp, uwep))
        return TRUE;

    if (Upolyd && noattacks(gy.youmonst.data)) {
        /* certain "pacifist" monsters don't attack */
        You("have no way to attack monsters physically.");
        mtmp->mstrategy &= ~STRAT_WAITMASK;
        goto atk_done;
    }

    if (check_capacity("You cannot fight while so heavily loaded.")
        /* consume extra nutrition during combat; maybe pass out */
        || overexertion())
        goto atk_done;

    if (u.twoweap && !can_twoweapon())
        untwoweapon();

    if (gu.unweapon) {
        gu.unweapon = FALSE;
        if (flags.verbose) {
            if (uwep)
                You("begin bashing monsters with %s.", yname(uwep));
            else if (!cantwield(gy.youmonst.data))
                You("begin %s monsters with your %s %s.",
                    ing_suffix(Role_if(PM_MONK) ? "strike" : "bash"),
                    uarmg ? "gloved" : "bare", /* Del Lamb */
                    makeplural(body_part(HAND)));
        }
    }
    exercise(A_STR, TRUE); /* you're exercising muscles */
    /* andrew@orca: prevent unlimited pick-axe attacks */
    u_wipe_engr(3);

    /* Is the "it died" check actually correct? */
    if (mdat->mlet == S_LEPRECHAUN && !mtmp->mfrozen && !helpless(mtmp)
        && !mtmp->mconf && mtmp->mcansee && !rn2(7)
        && (m_move(mtmp, 0) == MMOVE_DIED /* it died */
            || mtmp->mx != u.ux + u.dx
            || mtmp->my != u.uy + u.dy)) { /* it moved */
        You("miss wildly and stumble forwards.");
        return FALSE;
    }

    if (Upolyd)
        (void) hmonas(mtmp, NON_PM, TRUE);
    else
        (void) hitum(mtmp, gy.youmonst.data->mattk);
    mtmp->mstrategy &= ~STRAT_WAITMASK;

 atk_done:
    /* see comment in attack_checks() */
    /* we only need to check for this if we did an attack_checks()
     * and it returned 0 (it's okay to attack), and the monster didn't
     * evade.
     */
    if (svc.context.forcefight && !DEADMONSTER(mtmp) && !canspotmon(mtmp)
        && !glyph_is_invisible(levl[u.ux + u.dx][u.uy + u.dy].glyph)
        && !engulfing_u(mtmp))
        map_invisible(u.ux + u.dx, u.uy + u.dy);

    return TRUE;
}

/* really hit target monster; returns TRUE if it still lives */
staticfn boolean
known_hitum(
    struct monst *mon,  /* target */
    struct obj *weapon, /* uwep or uswapwep */
    int *mhit,          /* *mhit is 1 or 0; hit (1) gets changed to miss (0)
                         * for decapitation attack against headless target */
    int rollneeded,     /* rollneeded and armorpenalty are used for monks  +*/
    int armorpenalty,   /*+ wearing suits so miss message can vary for missed
                         *  because of penalty vs would have missed anyway  */
    struct attack *uattk,
    int dieroll)
{
    boolean malive = TRUE,
            /* hmon() might destroy weapon; remember aspect for cutworm */
            slice_or_chop = (weapon && (is_blade(weapon) || is_axe(weapon)));

    if (go.override_confirmation) {
        /* this may need to be generalized if weapons other than
           Stormbringer acquire similar anti-social behavior... */
        if (flags.verbose)
            Your("bloodthirsty blade attacks!");
    }

    if (!*mhit) {
        missum(mon, rollneeded, dieroll, uattk,
               (rollneeded + armorpenalty > dieroll));
    } else {
        int oldhp = mon->mhp;
        long oldweaphit = u.uconduct.weaphit;

        /* KMH, conduct */
        if (weapon && (weapon->oclass == WEAPON_CLASS || is_weptool(weapon)))
            u.uconduct.weaphit++;

        /* we hit the monster; be careful: it might die or
           be knocked into a different location */
        gn.notonhead = (mon->mx != gb.bhitpos.x || mon->my != gb.bhitpos.y);
        malive = hmon(mon, weapon, HMON_MELEE, dieroll);
        if (malive) {
            /* monster still alive */
            if (!rn2(25) && mon->mhp < mon->mhpmax / 2
                && !engulfing_u(mon)) {
                /* maybe should regurgitate if swallowed? */
                monflee(mon, !rn2(3) ? rnd(100) : 0, FALSE, TRUE);

                if (u.ustuck == mon && !u.uswallow
                    && !sticks(gy.youmonst.data))
                    set_ustuck((struct monst *) 0);
            }
            /* Vorpal Blade hit converted to miss */
            /* could be headless monster or worm tail */
            if (mon->mhp == oldhp) {
                *mhit = 0;
                /* a miss does not break conduct */
                u.uconduct.weaphit = oldweaphit;
            }
            if (mon->wormno && *mhit)
                cutworm(mon, gb.bhitpos.x, gb.bhitpos.y, slice_or_chop);
        }
    }
    return malive;
}

/* hit the monster next to you and the monsters to the left and right of it;
   return False if the primary target is killed, True otherwise */
staticfn boolean
hitum_cleave(
    struct monst *target, /* non-Null; forcefight at nothing doesn't cleave +*/
    struct attack *uattk) /*+ but we don't enforce that here; Null works ok */
{
    /* swings will be delivered in alternate directions; with consecutive
       attacks it will simulate normal swing and backswing; when swings
       are non-consecutive, hero will sometimes start a series of attacks
       with a backswing--that doesn't impact actual play, just spoils the
       simulation attempt a bit */
    static boolean clockwise = FALSE;
    int i;
    coord save_bhitpos;
    boolean save_notonhead;
    int count, umort, x = u.ux, y = u.uy;

    /* find the direction toward primary target */
    i = xytodir(u.dx, u.dy);
    if (i == DIR_ERR) {
        impossible("hitum_cleave: unknown target direction [%d,%d,%d]?",
                   u.dx, u.dy, u.dz);
        return TRUE; /* target hasn't been killed */
    }
    /* adjust direction by two so that loop's increment (for clockwise)
       or decrement (for counter-clockwise) will point at the spot next
       to primary target */
    i = clockwise ? DIR_LEFT2(i) : DIR_RIGHT2(i);
    umort = u.umortality; /* used to detect life-saving */
    save_bhitpos = gb.bhitpos;
    save_notonhead = gn.notonhead;

    /*
     * Three attacks:  adjacent to primary, primary, adjacent on other
     * side.  Primary target must be present or we wouldn't have gotten
     * here (forcefight at thin air won't 'cleave').  However, the
     * first attack might kill it (gas spore explosion, weak long worm
     * occupying both spots) so we don't assume that it's still present
     * on the second attack.
     */
    for (count = 3; count > 0; --count) {
        struct monst *mtmp;
        int tx, ty, tmp, dieroll, mhit, attknum = 0, armorpenalty;

        /* ++i, wrap 8 to i=0 /or/ --i, wrap -1 to i=7 */
        i = clockwise ? DIR_RIGHT(i) : DIR_LEFT(i);

        tx = x + xdir[i], ty = y + ydir[i]; /* current target location */
        if (!isok(tx, ty))
            continue;
        mtmp = m_at(tx, ty);
        if (!mtmp) {
            if (glyph_is_invisible(levl[tx][ty].glyph))
                (void) unmap_invisible(tx, ty);
            continue;
        }

        tmp = find_roll_to_hit(mtmp, uattk->aatyp, uwep,
                               &attknum, &armorpenalty);
        mon_maybe_unparalyze(mtmp);
        dieroll = rnd(20);
        mhit = (tmp > dieroll);
        gb.bhitpos.x = tx, gb.bhitpos.y = ty; /* normally set by do_attack() */
        gn.notonhead = (mtmp->mx != tx || mtmp->my != ty);
        (void) known_hitum(mtmp, uwep, &mhit, tmp, armorpenalty,
                           uattk, dieroll);
        (void) passive(mtmp, uwep, mhit, !DEADMONSTER(mtmp), AT_WEAP, !uwep);

        /* stop attacking if weapon is gone or hero got paralyzed or
           killed (and then life-saved) by passive counter-attack */
        if (!uwep || gm.multi < 0 || u.umortality > umort)
            break;
    }
    /* set up for next time */
    clockwise = !clockwise; /* alternate */
    gb.bhitpos = save_bhitpos; /* in case somebody relies on bhitpos
                                * designating the primary target */
    gn.notonhead = save_notonhead;

    /* return False if primary target died, True otherwise; note: if 'target'
       was nonNull upon entry then it's still nonNull even if *target died */
    return (target && DEADMONSTER(target)) ? FALSE : TRUE;
}

/* returns True if hero is fighting without a weapon and without a shield and
   has sufficient skill in bare-handed/martial arts to attack twice */
staticfn boolean
double_punch(void)
{
    /* note: P_BARE_HANDED_COMBAT and P_MARTIAL_ARTS are equivalent */
    int skl_lvl = P_SKILL(P_BARE_HANDED_COMBAT);

    /*
     * Chance to attempt a second bare-handed or martial arts hit:
     *  restricted  (0),        [not applicable; no one is restricted]
     *  unskilled   (1) :  0%
     *  basic       (2) :  0%
     *  skilled     (3) : 20%
     *  expert      (4) : 40%
     *  master      (5) : 60%
     *  grandmaster (6) : 80%
     */
    if (!uwep && !uarms && skl_lvl > P_BASIC)
        return (skl_lvl - P_BASIC) > rn2(5);
    return FALSE;
}

/* hit target monster; returns TRUE if it still lives */
staticfn boolean
hitum(struct monst *mon, struct attack *uattk)
{
    boolean malive, wep_was_destroyed = FALSE;
    struct obj *wepbefore = uwep,
        *secondwep = u.twoweap ? uswapwep : (struct obj *) 0;
    int tmp, dieroll, mhit, armorpenalty, attknum = 0,
        x = u.ux + u.dx, y = u.uy + u.dy, oldumort = u.umortality;

    /* Cleaver attacks three spots, 'mon' and one on either side of 'mon';
       it can't be part of dual-wielding but we guard against that anyway;
       cleave return value reflects status of primary target ('mon') */
    if (is_art(uwep, ART_CLEAVER) && !u.twoweap
        && !u.uswallow && !u.ustuck && !NODIAG(u.umonnum))
        return hitum_cleave(mon, uattk);

    /* 0: single hit, 1: first of two hits; affects strength bonus and
       silver rings; known_hitum() -> hmon() -> hmon_hitmon() will copy
       gt.twohits into struct _hitmon_data hmd.twohits */
    gt.twohits = (uwep ? u.twoweap : double_punch()) ? 1 : 0;

    tmp = find_roll_to_hit(mon, uattk->aatyp, uwep, &attknum, &armorpenalty);
    mon_maybe_unparalyze(mon);
    dieroll = rnd(20);
    mhit = (tmp > dieroll || u.uswallow);
    if (tmp > dieroll)
        exercise(A_DEX, TRUE);

    /* gb.bhitpos is set up by caller */
    malive = known_hitum(mon, uwep, &mhit, tmp, armorpenalty, uattk, dieroll);
    if (wepbefore && !uwep)
        wep_was_destroyed = TRUE;
    (void) passive(mon, uwep, mhit, malive, AT_WEAP, wep_was_destroyed);

    /* second attack for two-weapon combat or skilled unarmed combat;
       won't occur if Stormbringer overrode confirmation (assumes
       Stormbringer is primary weapon), or if hero became paralyzed by
       passive counter-attack, or if hero was killed by passive
       counter-attack and got life-saved, or if monster was killed or
       knocked to different location */
    if (gt.twohits && !(go.override_confirmation
                        || gm.multi < 0 || u.umortality > oldumort
                        || !malive || m_at(x, y) != mon)) {
        gt.twohits = 2; /* second of 2 hits */
        tmp = find_roll_to_hit(mon, uattk->aatyp, uswapwep, &attknum,
                               &armorpenalty);
        mon_maybe_unparalyze(mon);
        dieroll = rnd(20);
        mhit = (tmp > dieroll || u.uswallow);
        malive = known_hitum(mon, secondwep, &mhit, tmp, armorpenalty, uattk,
                             dieroll);
        /* second passive counter-attack only occurs if second attack hits */
        if (mhit)
            (void) passive(mon, secondwep, mhit, malive, AT_WEAP,
                           secondwep && !uswapwep);
    }
    gt.twohits = 0;

    /* your race may grant extra natural attacks; illithids and draugr
       don't use their extra attack every turn, vampires only rarely,
       and centaurs are strong enough to not need their extra kick
       attack */
    if (!Upolyd && !(gm.multi < 0 || u.umortality > oldumort
                     || !malive || !isok(x, y) || m_at(x, y) != mon)) {
        int i;
        int race = gu.urace.mnum;
        struct attack *attacks = mons[race].mattk;

        if (((Race_if(PM_ILLITHID) || Race_if(PM_DRAUGR)) && rn2(4))
            || (Race_if(PM_VAMPIRE) && !rn2(3))
            || Race_if(PM_CENTAUR))
            return malive;
        for (i = 0; i < NATTK; i++) {
            if (attacks[i].aatyp != AT_WEAP
                && attacks[i].aatyp != AT_NONE) {
                malive = hmonas(mon, race, FALSE);
                break;
            }
        }
    }

    return malive;
}

/* general "damage monster" routine; return True if mon still alive */
boolean
hmon(struct monst *mon,
     struct obj *obj,
     int thrown, /* HMON_xxx (0 => hand-to-hand, other => ranged) */
     int dieroll)
{
    boolean result, anger_guards;

    anger_guards = (mon->mpeaceful
                    && (mon->ispriest || mon->isshk || is_watch(mon->data)));
    result = hmon_hitmon(mon, obj, thrown, dieroll);
    if (mon->ispriest && !rn2(2))
        ghod_hitsu(mon);
    if (anger_guards)
        (void) angry_guards(!!Deaf);
    return result;
}

/* hero hits monster bare handed */
staticfn void
hmon_hitmon_barehands(struct _hitmon_data *hmd, struct monst *mon)
{
    long spcdmgflg; /* worn mask */

    if (hmd->mdat == &mons[PM_SHADE]) {
        hmd->dmg = 0;
    } else {
        /* note: 1..2 or 1..4 can be substantially increased by
           strength bonus or skill bonus, usually both... */
        hmd->dmg = rnd(!martial_bonus() ? 2 : 4);
        hmd->use_weapon_skill = TRUE;
        hmd->train_weapon_skill = (hmd->dmg > 1);
    }

    /* Blessed gloves give bonuses when fighting 'bare-handed'. So do
       rings made of a material the target hates. Note: rings are worn
       under gloves, so you don't get both bonuses, and two hated rings
       don't give double bonus.
       When making only one hit, both rings are checked (backwards
       compatibility => playability), but when making two hits, only the
       ring on the hand making the attack is checked */
    spcdmgflg = uarmg ? W_ARMG
              : (((hmd->twohits == 0 || hmd->twohits == 1) ? W_RINGR : 0L)
                 | ((hmd->twohits == 0 || hmd->twohits == 2) ? W_RINGL : 0L));
    hmd->dmg += special_dmgval(&gy.youmonst, mon, spcdmgflg,
                               &hmd->hated_obj);
}

staticfn void
hmon_hitmon_weapon_ranged(
    struct _hitmon_data *hmd,
    struct monst *mon,
    struct obj *obj)    /* obj is not NULL */
{
    /* then do only 1-2 points of damage and don't use or
       train weapon's skill */
    if (hmd->mdat == &mons[PM_SHADE] && !shade_glare(obj))
        hmd->dmg = 0;
    else
        hmd->dmg = rnd(2);
    if (mon_hates_material(mon, obj->material)) {
        hmd->hated_obj = obj;
        /* if it will already inflict dmg, make it worse */
        hmd->dmg += rnd((hmd->dmg) ? sear_damage(obj->material)
                                   : sear_damage(obj->material) / 2);
    }
    if (!hmd->thrown && obj == uwep && obj->otyp == BOOMERANG
        && rnl(4) == 4 - 1) {
        boolean more_than_1 = (obj->quan > 1L);

        pline("As you hit %s, %s%s breaks into splinters.",
              mon_nam(mon), more_than_1 ? "one of " : "",
              yname(obj));
        if (!more_than_1)
            uwepgone(); /* set gu.unweapon */
        useup(obj);
        if (!more_than_1) {
            obj = (struct obj *) 0;
            /* obj is gone; don't let the searing message use it, and
               tell our caller, whose copy of the pointer is now stale */
            hmd->hated_obj = (struct obj *) 0;
            hmd->obj_gone = TRUE;
        }
        hmd->hittxt = TRUE;
        if (hmd->mdat != &mons[PM_SHADE])
            hmd->dmg++;
    }
}

/* can monster be stabbed in the back? */
staticfn boolean
backstabbable(struct monst *mon)
{
    return !amorphous(mon->data)
        && !is_whirly(mon->data)
        && !noncorporeal(mon->data)
        && mon->data->mlet != S_BLOB
        && mon->data->mlet != S_EYE
        && mon->data->mlet != S_FUNGUS
        && canseemon(mon)
        && (mon->mflee || helpless(mon));
}

staticfn void
hmon_hitmon_weapon_melee(
    struct _hitmon_data *hmd,
    struct monst *mon,
    struct obj *obj)    /* obj is not NULL */
{
    int wtype;
    struct obj *monwep;

    /* "normal" weapon usage */
    hmd->use_weapon_skill = TRUE;
    hmd->dmg = dmgval(obj, mon);
    /* giants are more effective with club-like weapons; roll the
       damage twice and take the better result, plus one */
    if (maybe_polyd(is_giant(gy.youmonst.data), Race_if(PM_GIANT))
        && objects[obj->otyp].oc_skill == P_CLUB
        && !noncorporeal(hmd->mdat)) {
        int dmg2 = dmgval(obj, mon);

        if (hmd->dmg < dmg2)
            hmd->dmg = dmg2;
        hmd->dmg++;
    }
    /* a minimal hit doesn't exercise proficiency */
    hmd->train_weapon_skill = (hmd->dmg > 1);

    /* Healer with anatomy knowledge */
    if (Role_if(PM_HEALER) && hmd->hand_to_hand
        && obj->oclass == WEAPON_CLASS
        && objects[obj->otyp].oc_skill == P_DAGGER)
        hmd->dmg += min(3, svm.mvitals[monsndx(mon->data)].died / 6);

    /* special attack actions */
    if (!hmd->train_weapon_skill || mon == u.ustuck || u.twoweap
        /* Cleaver can hit up to three targets at once so don't
           let it also hit from behind or shatter foes' weapons */
        || (hmd->hand_to_hand && is_art(obj, ART_CLEAVER))) {
        ; /* no special bonuses */
    } else if (Role_if(PM_ROGUE) && backstabbable(mon) && !Upolyd
               /* multi-shot throwing is too powerful here */
               && hmd->hand_to_hand) {
        You("strike %s from behind!", mon_nam(mon));
        hmd->dmg += rnd(u.ulevel);
        hmd->hittxt = TRUE;
    } else if (obj == uwep && obj->oclass == WEAPON_CLASS
               && ((hmd->dieroll == 2
                    && (bimanual(obj) || Race_if(PM_GIANT)
                        || (Role_if(PM_SAMURAI) && obj->otyp == KATANA
                            && !uarms))
                    && ((wtype = uwep_skill_type()) != P_NONE
                        && P_SKILL(wtype) >= P_SKILLED))
                   /* giants can shatter weapons on other near-best
                      hits, given enough skill */
                   || (hmd->dieroll == 3 && Race_if(PM_GIANT)
                       && ((wtype = uwep_skill_type()) != P_NONE
                           && P_SKILL(wtype) >= P_BASIC))
                   || (hmd->dieroll == 4 && !rn2(2) && Race_if(PM_GIANT)
                       && ((wtype = uwep_skill_type()) != P_NONE
                           && P_SKILL(wtype) >= P_EXPERT)))
               && ((monwep = MON_WEP(mon)) != 0
                   && !is_flimsy(monwep)
                   && !is_mithril(monwep) /* mithril is super-strong */
                   && !is_crystal(monwep) /* so is gemstone */
                   && monwep->forged_qual != FQ_EXCEPTIONAL
                   && (monwep->forged_qual != FQ_SUPERIOR && rn2(2))
                   && !obj_resists(monwep,
                                   (monwep->forged_qual == FQ_INFERIOR
                                    ? 30 : 50)
                                   + 15 * (greatest_erosion(obj)
                                           - greatest_erosion(monwep)),
                                   100))) {
        static const char from_your_blow[] = " from the force of your blow!";
        char buf[BUFSZ];
        /*
         * 2.5% chance of shattering defender's weapon when
         * using a two-handed weapon; less if uwep is rusted.
         * [dieroll == 2 is most successful non-beheading or
         * -bisecting hit, in case of special artifact damage;
         * the percentage chance is (1/20)*(50/100).]
         * If attacker's weapon is rustier than defender's,
         * the obj_resists chance is increased so the shatter
         * chance is decreased; if less rusty, then vice versa.
         */
        setmnotwielded(mon, monwep);
        mon->weapon_check = NEED_WEAPON;
        if (canseemon(mon))
            /* Yobjnam2(X,"shatter") yields "Shk's X shatters" if X is owned
               by a shop or "Mon's X shatters" if X is carried by a monster
               (or "{Your|The} X shatters" if {carried by hero|last resort})*/
            Strcpy(buf, Yobjnam2(monwep, "shatter"));
        else /* hero is blind or can't see invisible mon */
            /* construct "Its weapon shatters"; not an exact replacement
               for Yobjnam2() if an unseen mon other than the shopkeeper
               is wielding a shop-owned weapon; telepathy or extended
               monster detection will name mon but not its weapon */
            Sprintf(buf, "%s weapon%s %s", s_suffix(Monnam(mon)),
                    plur(monwep->quan), otense(monwep, "shatter"));
        buf[sizeof buf - sizeof from_your_blow] = '\0';
        pline("%s%s", buf, from_your_blow);
        m_useupall(mon, monwep);
        /* If someone just shattered MY weapon, I'd flee! */
        if (rn2(4)) {
            monflee(mon, d(2, 3), TRUE, TRUE);
        }
        hmd->hittxt = TRUE;
    }

    if ((obj->oartifact
         || ((obj->oclass == WEAPON_CLASS || is_weptool(obj)
              || is_bullet(obj)) && obj->oprops))
        && artifact_hit(&gy.youmonst, mon, obj, &hmd->dmg, hmd->dieroll)) {
        /* artifact_hit updates 'tmp' but doesn't inflict any
           damage; however, it might cause carried items to be
           destroyed and they might do so */
        if (DEADMONSTER(mon)) { /* artifact killed monster */
            hmd->doreturn = TRUE;
            hmd->retval = FALSE;
            return;
            /*return FALSE;*/
        }
        /* perhaps artifact tried to behead a headless monster */
        if (hmd->dmg == 0) {
            hmd->doreturn = TRUE;
            hmd->retval = TRUE;
            return;
            /*return TRUE;*/
        }
        hmd->hittxt = TRUE;
    }
    if (mon_hates_material(mon, obj->material)) {
        /* dmgval() already added bonus damage */
        hmd->hated_obj = obj;
    }
    if (artifact_light(obj) && obj->lamplit
        /* an Archmagi's staff radiating darkness for a drow does
           not sear light haters */
        && !(is_art(obj, ART_STAFF_OF_THE_ARCHMAGI)
             && !Upolyd && Race_if(PM_DROW))
        && mon_hates_light(mon))
        hmd->lightobj = TRUE;
    if ((u.usteed || (!Upolyd && Race_if(PM_CENTAUR)))
        && !hmd->thrown && hmd->dmg > 0
        && weapon_type(obj) == P_LANCE && mon != u.ustuck) {
        hmd->jousting = joust(mon, obj);
        /* exercise skill even for minimal damage hits */
        if (hmd->jousting)
            hmd->train_weapon_skill = TRUE;
    }
    if (hmd->thrown == HMON_THROWN
        && (is_ammo(obj) || is_missile(obj))) {
        if (ammo_and_launcher(obj, uwep)) {
            /* slings hit giants harder */
            if (uwep->otyp == SLING && racial_giant(mon))
                hmd->dmg *= 2;
            /* elves and samurai do extra damage using their own
               bows with their own arrows; they're highly trained */
            if (Role_if(PM_SAMURAI) && obj->otyp == YA
                && uwep->otyp == YUMI)
                hmd->dmg++;
            else if (Race_if(PM_ELF) && obj->otyp == ELVEN_ARROW
                     && uwep->otyp == ELVEN_BOW)
                hmd->dmg++;
            else if (Race_if(PM_DROW) && obj->otyp == DARK_ELVEN_ARROW
                     && uwep->otyp == DARK_ELVEN_BOW)
                hmd->dmg++;
            hmd->train_weapon_skill = (hmd->dmg > 0);
        }
        if (obj->opoisoned && is_poisonable(obj))
            hmd->ispoisoned = TRUE;
    }
    /* permapoisoned is non-ammo/missile, limit the poison */
    if (permapoisoned(obj) && hmd->dieroll <= 5)
        hmd->ispoisoned = TRUE;
}

staticfn void
hmon_hitmon_weapon(
    struct _hitmon_data *hmd,
    struct monst *mon,
    struct obj *obj)   /* obj is not NULL */
{
    /* is it not a melee weapon? */
    if (/* if you strike with a bow... */
        is_launcher(obj)
        /* or strike with a missile in your hand... */
        || (!hmd->thrown && (is_missile(obj) || is_ammo(obj)))
        /* or use a pole at short range and not mounted (centaurs
           count as mounted here) */
        || (!hmd->thrown && !u.usteed && !Race_if(PM_CENTAUR)
            && is_pole(obj) && !is_art(obj, ART_SNICKERSNEE))
        /* or throw a missile without the proper bow... */
        || (is_ammo(obj) && (hmd->thrown != HMON_THROWN
                             || !ammo_and_launcher(obj, uwep)))) {
        hmon_hitmon_weapon_ranged(hmd, mon, obj);
    } else {
        hmon_hitmon_weapon_melee(hmd, mon, obj);
        if (hmd->doreturn)
            return;
    }
}

staticfn void
hmon_hitmon_potion(
    struct _hitmon_data *hmd,
    struct monst *mon,
    struct obj *obj)    /* obj is not NULL */
{
    if (obj->quan > 1L)
        obj = splitobj(obj, 1L);
    else
        setuwep((struct obj *) 0);
    freeinv(obj);
    potionhit(mon, obj,
              hmd->hand_to_hand ? POTHIT_HERO_BASH : POTHIT_HERO_THROW);
    if (DEADMONSTER(mon)) {
        hmd->doreturn = TRUE;
        hmd->retval = FALSE; /* killed */
        return;
    }
    hmd->hittxt = TRUE;
    /* in case potion effect causes transformation */
    hmd->mdat = mon->data;
    hmd->dmg = (hmd->mdat == &mons[PM_SHADE]) ? 0 : 1;
}

staticfn void
hmon_hitmon_misc_obj(
    struct _hitmon_data *hmd,
    struct monst *mon,
    struct obj *obj)    /* obj is not NULL */
{
    switch (obj->otyp) {
    case BOULDER:         /* 1d20 */
    case HEAVY_IRON_BALL: /* 1d25 */
    case IRON_CHAIN:      /* 1d4+1 */
        hmd->dmg = dmgval(obj, mon);
        if (mon_hates_material(mon, obj->material)) {
            /* dmgval() already added damage, but track hated_obj */
            hmd->hated_obj = obj;
        }
        break;
    case MIRROR:
        if (breaktest(obj, u.ux, u.uy)) {
            You("break %s.  That's bad luck!", ysimple_name(obj));
            change_luck(-2);
            useup(obj);
            obj = (struct obj *) 0;
            hmd->obj_gone = TRUE;
            hmd->unarmed = FALSE; /* avoid obj==0 confusion */
            hmd->get_dmg_bonus = FALSE;
            hmd->hittxt = TRUE;
        }
        hmd->dmg = 1;
        break;
    case EXPENSIVE_CAMERA:
        You("succeed in destroying %s.  Congratulations!",
            ysimple_name(obj));
        release_camera_demon(obj, u.ux, u.uy);
        useup(obj);
        hmd->doreturn = TRUE;
        hmd->retval = TRUE;
        return;
        /*return TRUE;*/
    case CORPSE: /* fixed by polder@cs.vu.nl */
        if (touch_petrifies(&mons[obj->corpsenm])) {
            hmd->dmg = 1;
            hmd->hittxt = TRUE;
            You("hit %s with %s.", mon_nam(mon),
                corpse_xname(obj, (const char *) 0,
                             obj->dknown ? CXN_PFX_THE
                             : CXN_ARTICLE));
            observe_object(obj);
            if (!munstone(mon, TRUE))
                minstapetrify(mon, TRUE);
            if (resists_ston(mon))
                break;
            /* note: hp may be <= 0 even if munstoned==TRUE */
            hmd->doreturn = TRUE;
            hmd->retval = !DEADMONSTER(mon);
            return;
            /*return (boolean) !DEADMONSTER(mon);*/
#if 0
        } else if (touch_petrifies(mdat)) {
            ; /* maybe turn the corpse into a statue? */
#endif
        }
        hmd->dmg = (ismnum(obj->corpsenm) ? mons[obj->corpsenm].msize
                    : 0) + 1;
        break;

#define useup_eggs(o)                    \
    do {                                 \
        if (hmd->thrown)                 \
            obfree(o, (struct obj *) 0); \
        else                             \
            useupall(o);                 \
        o = (struct obj *) 0;            \
        hmd->obj_gone = TRUE;            \
    } while (0) /* now gone */
    case EGG: {
        long cnt = obj->quan;

        hmd->dmg = 1; /* nominal physical damage */
        hmd->get_dmg_bonus = FALSE;
        hmd->hittxt = TRUE; /* message always given */
        /* egg is always either used up or transformed, so next
           hand-to-hand attack should yield a "bashing" mesg */
        if (obj == uwep)
            gu.unweapon = TRUE;
        if (obj->spe && ismnum(obj->corpsenm)) {
            if (obj->quan < 5L)
                change_luck((schar) - (obj->quan));
            else
                change_luck(-5);
        }

        if (ismnum(obj->corpsenm)
            && touch_petrifies(&mons[obj->corpsenm])) {
            /*learn_egg_type(obj->corpsenm);*/
            pline("Splat!  You hit %s with %s %s egg%s!",
                  mon_nam(mon),
                  obj->known ? "the" : cnt > 1L ? "some" : "a",
                  obj->known ? mons[obj->corpsenm].pmnames[NEUTRAL]
                  : "petrifying",
                  plur(cnt));
            obj->known = 1; /* (not much point...) */
            useup_eggs(obj);
            if (!munstone(mon, TRUE))
                minstapetrify(mon, TRUE);
            if (resists_ston(mon))
                break;
            hmd->doreturn = TRUE;
            hmd->retval = !DEADMONSTER(mon);
            return;
            /*return (boolean) (!DEADMONSTER(mon));*/
        } else { /* ordinary egg(s) */
            enum monnums mnum = obj->corpsenm;
            const char *eggp =
                (ismnum(mnum) && obj->known)
                    ? the(mons[mnum].pmnames[NEUTRAL])
                    : (cnt > 1L) ? "some" : "an";

            You("hit %s with %s egg%s.", mon_nam(mon), eggp,
                plur(cnt));
            if (touch_petrifies(hmd->mdat) && !stale_egg(obj)) {
                pline_The("egg%s %s alive any more...", plur(cnt),
                          (cnt == 1L) ? "isn't" : "aren't");
                if (obj->timed)
                    obj_stop_timers(obj);
                obj->otyp = ROCK;
                obj->oclass = GEM_CLASS;
                /* rocks must be MINERAL material */
                set_material(obj, MINERAL);
                obj->oartifact = 0;
                obj->spe = 0;
                obj->known = obj->dknown = obj->bknown = 0;
                obj->owt = weight(obj);
                if (hmd->thrown)
                    place_object(obj, mon->mx, mon->my);
            } else if (obj->corpsenm == PM_PYROLISK) {
                useup_eggs(obj);
                explode(mon->mx, mon->my, BZ_M_SPELL(ZT_FIRE), d(3, 6), 0,
                        EXPL_FIERY);
                hmd->doreturn = TRUE;
                hmd->retval = !DEADMONSTER(mon);
                return;
            } else {
                pline("Splat!");
                useup_eggs(obj);
                exercise(A_WIS, FALSE);
            }
        }
        break;
#undef useup_eggs
    }
    case CLOVE_OF_GARLIC: /* no effect against demons */
        if (is_undead(hmd->mdat) || is_vampshifter(mon)) {
            monflee(mon, d(2, 4), FALSE, TRUE);
        }
        hmd->dmg = 1;
        break;
    case CREAM_PIE:
    case BLINDING_VENOM:
    case SNOWBALL:
        mon->msleeping = 0;
        if (can_blnd(&gy.youmonst, mon,
                     (uchar) ((obj->otyp == BLINDING_VENOM
                               || obj->otyp == SNOWBALL)
                              ? AT_SPIT
                              : AT_WEAP),
                     obj)) {
            if (Blind) {
                pline(obj->otyp == CREAM_PIE ? "Splat!"
                      : obj->otyp == SNOWBALL ? "Thwap!"
                                              : "Splash!");
            } else if (obj->otyp == BLINDING_VENOM) {
                pline_The("venom blinds %s%s!", mon_nam(mon),
                          mon->mcansee ? "" : " further");
            } else {
                char *whom = mon_nam(mon);
                char *what = The(xname(obj));

                if (!hmd->thrown && obj->quan > 1L)
                    what = An(singular(obj, xname));
                /* note: s_suffix returns a modifiable buffer */
                if (haseyes(hmd->mdat)
                    && hmd->mdat != &mons[PM_FLOATING_EYE])
                    whom = strcat(strcat(s_suffix(whom), " "),
                                  mbodypart(mon, FACE));
                pline("%s %s over %s!", what,
                      vtense(what, (obj->otyp == SNOWBALL) ? "spread"
                                                           : "splash"),
                      whom);
            }
            setmangry(mon, TRUE);
            mon->mcansee = 0;
            hmd->dmg = rn1(25, 21);
            if (((int) mon->mblinded + hmd->dmg) > 127)
                mon->mblinded = 127;
            else
                mon->mblinded += hmd->dmg;
        } else {
            pline(obj->otyp == CREAM_PIE ? "Splat!"
                  : obj->otyp == SNOWBALL ? "Thwap!" : "Splash!");
            setmangry(mon, TRUE);
        }
        {
            boolean more_than_1 = (obj->quan > 1L);

            if (hmd->thrown)
                obfree(obj, (struct obj *) 0);
            else
                useup(obj);

            if (!more_than_1) {
                obj = (struct obj *) 0;
                hmd->obj_gone = TRUE;
            }
        }
        hmd->hittxt = TRUE;
        hmd->get_dmg_bonus = FALSE;
        hmd->dmg = 0;
        break;
    case ACID_VENOM: /* thrown (or spit) */
        if (resists_acid(mon)) {
            Your("venom hits %s harmlessly.", mon_nam(mon));
            hmd->dmg = 0;
        } else {
            Your("venom burns %s!", mon_nam(mon));
            hmd->dmg = dmgval(obj, mon);
        }
        {
            boolean more_than_1 = (obj->quan > 1L);

            if (hmd->thrown)
                obfree(obj, (struct obj *) 0);
            else
                useup(obj);

            if (!more_than_1) {
                obj = (struct obj *) 0;
                hmd->obj_gone = TRUE;
            }
        }
        hmd->hittxt = TRUE;
        hmd->get_dmg_bonus = FALSE;
        break;
    case BARBED_NEEDLE: /* fired */
        if (resists_poison(mon) || defended(mon, AD_DRCO)) {
            Your("barbed needle hits %s harmlessly.", mon_nam(mon));
            hmd->dmg = 0;
        } else {
            Your("barbed needle stings %s!", mon_nam(mon));
            hmd->dmg = dmgval(obj, mon);
        }
        {
            boolean more_than_1 = (obj->quan > 1L);

            if (hmd->thrown)
                obfree(obj, (struct obj *) 0);
            else
                useup(obj);

            if (!more_than_1) {
                obj = (struct obj *) 0;
                hmd->obj_gone = TRUE;
            }
        }
        hmd->hittxt = TRUE;
        hmd->get_dmg_bonus = FALSE;
        break;
    case BALL_OF_WEBBING:
        /* webbing is a trap-laying tool, not a damage tool */
        hmd->dmg = 0;
        hmd->get_dmg_bonus = FALSE;
        if (!t_at(mon->mx, mon->my)) {
            struct trap *web = maketrap(mon->mx, mon->my, WEB);

            if (web) {
                (void) mintrap(mon, NO_TRAP_FLAGS);
                web_unseats_rider(mon);
            }
        }
        break;
    default:
        if ((objects[obj->otyp].oc_material == VEGGY ||
             objects[obj->otyp].oc_material == PAPER) &&
            obj->oclass != SPBOOK_CLASS) {
            /* vegetables (and similar) do no damage, because they
               aren't rigid enough; paper objects also do no damage,
               except for books */
            hmd->dmg = 0;
            hmd->get_dmg_bonus = FALSE;
            break;
        }
        /* non-weapons can damage because of their weight */
        /* (but not too much) */
        hmd->dmg = (obj->owt + 99) / 100;
        hmd->dmg = (hmd->dmg <= 1) ? 1 : rnd(hmd->dmg);
        if (hmd->dmg > 6)
            hmd->dmg = 6;
        /* wet towel has modest damage bonus beyond its weight,
           based on its wetness */
        if (is_wet_towel(obj)) {
            boolean doubld = (mon->data == &mons[PM_IRON_GOLEM]);

            /* wielded wet towel should probably use whip skill
               (but not by setting objects[TOWEL].oc_skill==P_WHIP
               because that would turn towel into a weptool);
               due to low weight, tmp always starts at 1 here, and
               due to wet towel's definition, obj->spe is 1..7 */
            hmd->dmg += obj->spe * (doubld ? 2 : 1);
            hmd->dmg = rnd(hmd->dmg); /* wet towel damage not capped at 6 */
            /* usually lose some wetness but defer doing so
               until after hit message */
            hmd->dryit = (rn2(obj->spe + 1) > 0);
        }
        /* things like wands made of a hated material can arrive here
           so we need another material check; blessed check too */
        if (mon_hates_material(mon, obj->material)) {
            hmd->dmg += rnd(sear_damage(obj->material));
            hmd->hated_obj = obj;
        }
        if (obj->blessed && mon_hates_blessings(mon))
            hmd->dmg += rnd(4);
    }
}

/* do the actual hitting monster with obj/fists */
staticfn void
hmon_hitmon_do_hit(
    struct _hitmon_data *hmd,
    struct monst *mon,
    struct obj *obj)    /* obj can be NULL */
{
    if (!obj) { /* attack with bare hands */
        hmon_hitmon_barehands(hmd, mon);
    } else {
        /* obj is not NULL here because of the !obj check in this if block,
         , so no guard is needed ahead of stone_missile(obj) */
        /* stone missile does not hurt xorn or earth elemental, but doesn't
           pass all the way through and continue on to some further target */
        if ((hmd->thrown == HMON_THROWN
             || hmd->thrown == HMON_KICKED) /* not Applied */
            && stone_missile(obj) && passes_rocks(hmd->mdat)) {
            hit(mshot_xname(obj), mon, " but does no harm.");
            wakeup(mon, TRUE);
            hmd->doreturn = TRUE;
            hmd->retval = TRUE;
            return;
        }
        /* remember obj's name since it might end up being destroyed and
           we'll want to use it after that */
        if (!(artifact_light(obj) && obj->lamplit))
            Strcpy(hmd->saved_oname, cxname(obj));
        else
            Strcpy(hmd->saved_oname, bare_artifactname(obj));

        if (obj->oclass == WEAPON_CLASS || is_weptool(obj)
            || obj->oclass == GEM_CLASS) {
            hmon_hitmon_weapon(hmd, mon, obj);
            if (hmd->doreturn)
                return;
        /* attacking with non-weapons */
        } else if (obj->oclass == POTION_CLASS) {
            hmon_hitmon_potion(hmd, mon, obj);
            if (hmd->doreturn)
                return;
        } else {
            if (hmd->mdat == &mons[PM_SHADE] && !shade_aware(obj)) {
                hmd->dmg = 0;
            } else {
                hmon_hitmon_misc_obj(hmd, mon, obj);
            }
        }
    }
}

staticfn void
hmon_hitmon_dmg_recalc(struct _hitmon_data *hmd, struct obj *obj)
{
    int dmgbonus = 0, strbonus, absbonus;

    /*
     * Potential bonus (or penalty) from worn ring of increase damage
     * (or intrinsic bonus from eating same) or from strength.  Strength
     * bonus is increased for melee with two-handed weapons and decreased
     * for dual attacks (but when both hit, the total for the two is more
     * than the bonus for a regular single hit).
     */
    if (hmd->get_dmg_bonus) {
        /* for dual attacks, udaminc applies to both, and two-handed
           weapons use it as-is */
        dmgbonus = u.udaminc;
        /* throwing using a propellor gets an increase-damage bonus
           but not a strength one; other attacks get both;
           for dual attacks, 3/4 of the strength bonus is used; when
           both attacks hit, overall bonus is 3/2 rather than doubled;
           melee hit with two-handed weapon uses 3/2 strength bonus to
           approximately match double hit with two-weapon ('approximate'
           because udaminc skews in favor of two-weapon); the 3/2 factor
           for two-handed strength does not apply to polearms unless
           hero is simply bashing with one of those and does not apply
           to jousting because lances are one-handed */
        if (hmd->thrown != HMON_THROWN
            || !obj || !uwep || !ammo_and_launcher(obj, uwep)) {
            strbonus = dbon();
            absbonus = abs(strbonus);
            if (hmd->twohits)
                strbonus = ((3 * absbonus + 2) / 4) * sgn(strbonus);
            else if (hmd->thrown == HMON_MELEE && uwep && bimanual(uwep))
                strbonus = ((3 * absbonus + 1) / 2) * sgn(strbonus);
            dmgbonus += strbonus;
        }
    }

    /*
     * Potential bonus (or penalty) from weapon skill.
     * 'use_weapon_skill' is True for hand-to-hand ordinary weapon,
     * applied or jousting polearm or lance, thrown missile (dart,
     * shuriken, boomerang), or shot ammo (arrow, bolt, rock/gem when
     * wielding corresponding launcher).
     * It is False for hand-to-hand or thrown non-weapon, hand-to-hand
     * polearm or lance when not mounted, hand-to-hand missile or ammo
     * or launcher, thrown non-missile, or thrown ammo (including rocks)
     * when not wielding corresponding launcher.
     */
    if (hmd->use_weapon_skill) {
        struct obj *skillwep = obj;

        if (PROJECTILE(obj) && ammo_and_launcher(obj, uwep))
            skillwep = uwep;
        dmgbonus += weapon_dam_bonus(skillwep);

        /* hit for more than minimal damage (before being adjusted
           for damage or skill bonus) trains the skill toward future
           enhancement */
        if (hmd->train_weapon_skill) {
            /* [this assumes that `!thrown' implies wielded...] */
            int wtype = hmd->thrown ? weapon_type(skillwep)
                                    : uwep_skill_type();

            /* train thievery in steal_it() when we know if theft
               succeeded */
            if (!(wtype == P_THIEVERY
                  && (Role_if(PM_ROGUE) || Role_if(PM_CONVICT))))
                use_skill(wtype, 1);
        }
    }

    /* apply combined damage+strength and skill bonuses */
    hmd->dmg += dmgbonus;
    /* don't let penalty, if bonus is negative, turn a hit into a miss */
    if (hmd->dmg < 1)
        hmd->dmg = 1;
}

staticfn void
hmon_hitmon_poison(
    struct _hitmon_data *hmd,
    struct monst *mon,
    struct obj *obj)    /* obj is not NULL */
{
    int nopoison = (10 - (obj->owt / 10));

    if (nopoison < 2)
        nopoison = 2;
    if (Role_if(PM_SAMURAI)) {
        You("dishonorably use a poisoned weapon!");
        adjalign(-sgn(u.ualign.type));
        record_abuse_event_dtl(-sgn(u.ualign.type), ABUSE_USE_POISON,
                               obj->otyp + 1, 0);
    } else if (u.ualign.type == A_LAWFUL && u.ualign.record > -10) {
        int pen = Role_if(PM_KNIGHT) ? -3 : -1;

        You_feel("like an evil coward for using a poisoned weapon.");
        adjalign(pen);
        record_abuse_event_dtl(pen, ABUSE_USE_POISON,
                               obj->otyp + 1, 0);
    }
    /* the venom object property never wears off */
    if (hmd->ispoisoned && !permapoisoned(obj)
        && !(obj->oprops & ITEM_VENOM) && !rn2(nopoison)) {
        /* remove poison now in case obj ends up in a bones file */
        obj->opoisoned = FALSE;
        /* defer "obj is no longer poisoned" until after hit message */
        hmd->unpoisonmsg = TRUE;
    }
    if (resists_poison(mon))
        hmd->needpoismsg = TRUE;
    else if (rn2(10))
        hmd->dmg += rnd(6);
    else
        hmd->poiskilled = TRUE;
}

staticfn void
hmon_hitmon_jousting(
    struct _hitmon_data *hmd,
    struct monst *mon, /* target */
    struct obj *obj) /* lance; obj is not NULL */
{
    hmd->dmg += d(2, (obj == uwep) ? 10 : 2); /* [was in dmgval()] */
    You("joust %s%s", mon_nam(mon), canseemon(mon) ? exclam(hmd->dmg) : ".");
    /* if this hit just broke the never-hit-with-wielded-weapon conduct
       (handled by caller...), give a livelog message for that now */
    if (u.uconduct.weaphit <= 1)
        first_weapon_hit(obj);

    if (hmd->jousting < 0) {
        /* (must be either primary or secondary weapon to get here) */
        if (u.twoweap) {
            struct obj *tmp = uswapwep;

            /* untwoweapon() is too verbose here; mirror its teardown
               so the off-hand W_SWAPWEP extrinsics are revoked instead
               of staying seated with two-weapon combat off */
            set_twoweap(FALSE); /* u.twoweap = FALSE */
            setuswapwep((struct obj *) 0);
            setuswapwep(tmp);
        }
        if (obj == uwep)
            uwepgone(); /* set gu.unweapon */
        pline("%s shatters on impact!", Yname2(obj));
        /* minor side-effect: broken lance won't split puddings */
        useup(obj);
        obj = (struct obj *) 0;
        /* obj is gone; don't let the searing message use it, and tell
           our caller, whose own copy of the pointer is now stale --
           'obj' here is a by-value parameter */
        hmd->hated_obj = (struct obj *) 0;
        hmd->obj_gone = TRUE;
    }
    /* a wielded Harbinger or Giantslayer anchors the defender against
       the knockback */
    if (!(mon_wielding_artifact(mon, ART_HARBINGER)
          || mon_wielding_artifact(mon, ART_GIANTSLAYER))
        && mhurtle_to_doom(mon, hmd->dmg, &hmd->mdat))
        hmd->already_killed = TRUE;
    hmd->hittxt = TRUE;
}

staticfn void
hmon_hitmon_stagger(
    struct _hitmon_data *hmd,
    struct monst *mon,
    struct obj *obj UNUSED)
{
    /* VERY small chance of stunning opponent if unarmed;
       giant heroes hit hard enough to manage it more often */
    if (rnd(Race_if(PM_GIANT) ? 40 : 100) < P_SKILL(P_BARE_HANDED_COMBAT)
        && !bigmonst(hmd->mdat) && !thick_skinned(hmd->mdat)) {
        if (canspotmon(mon))
            pline("%s %s from your powerful strike!", Monnam(mon),
                  makeplural(stagger(mon->data, "stagger")));
        /* a wielded Harbinger or Giantslayer anchors the defender
           against the knockback */
        if (!(mon_wielding_artifact(mon, ART_HARBINGER)
              || mon_wielding_artifact(mon, ART_GIANTSLAYER))
            && mhurtle_to_doom(mon, hmd->dmg, &hmd->mdat))
            hmd->already_killed = TRUE;
        hmd->hittxt = TRUE;
    }
}

staticfn void
hmon_hitmon_pet(
    struct _hitmon_data *hmd,
    struct monst *mon,
    struct obj *obj UNUSED)
{
    if (mon->mtame && hmd->dmg > 0) {
        /* do this even if the pet is being killed or migrating (affects revival) */
        abuse_dog(mon); /* reduces tameness */
        /* flee if still alive and still tame; if already suffering from
           untimed fleeing, no effect, otherwise increases timed fleeing */
        if (mon->mtame && !hmd->destroyed)
            monflee(mon, 10 * rnd(hmd->dmg), FALSE, FALSE);
    }
}

staticfn void
hmon_hitmon_splitmon(
    struct _hitmon_data *hmd,
    struct monst *mon,
    struct obj *obj)    /* obj can be NULL but guards are in place below */
{
    if ((hmd->mdat == &mons[PM_BLACK_PUDDING]
         || hmd->mdat == &mons[PM_BROWN_PUDDING])
        /* pudding is alive and healthy enough to split */
        && mon->mhp > 1 && !mon->mcan && !hmd->offmap
        /* iron weapon using melee or polearm hit [3.6.1: metal weapon too;
           also allow either or both weapons to cause split when twoweap] */
        && obj && (obj == uwep || (u.twoweap && obj == uswapwep))
        && ((hmd->material == IRON
             /* allow scalpel and tsurugi to split puddings */
             || hmd->material == STEEL)
            /* but not bashing with darts, arrows or ya */
            && !(is_ammo(obj) || is_missile(obj)))
        && hmd->hand_to_hand) {
        struct monst *mclone;
        char withwhat[BUFSZ];

        if ((mclone = clone_mon(mon, 0, 0)) != 0) {
            withwhat[0] = '\0';
            if (u.twoweap && flags.verbose)
                Sprintf(withwhat, " with %s", yname(obj));
            pline("%s divides as you hit it%s!", Monnam(mon), withwhat);
            hmd->hittxt = TRUE;
            (void) mintrap(mclone, NO_TRAP_FLAGS);
        }
    }
}

staticfn void
hmon_hitmon_msg_hit(
    struct _hitmon_data *hmd,
    struct monst *mon,
    struct obj *obj)   /* obj can be NULL for hand_to_hand; otherwise not */
{
    if (!hmd->hittxt /*( thrown => obj exists )*/
        && (!hmd->destroyed
            || (hmd->thrown && gm.m_shot.n > 1
                && gm.m_shot.o == obj->otyp))) {
        if (hmd->thrown)
            hit(mshot_xname(obj), mon, exclam(hmd->dmg));
        else if (!flags.verbose)
            You("hit it.");
        else /* hand_to_hand */
            You("%s %s%s",
                (obj && (is_shield(obj)
                         || obj->otyp == HEAVY_IRON_BALL)) ? "bash"
                : (obj && (objects[obj->otyp].oc_skill == P_WHIP
                           || is_wet_towel(obj))) ? "lash"
                  : Role_if(PM_BARBARIAN) ? "smite"
                    : (!obj
                       && (has_claws(gy.youmonst.data)
                           || has_claws_undead(gy.youmonst.data)
                           || (!Upolyd
                               && (Race_if(PM_DEMON)
                                   || Race_if(PM_TORTLE)
                                   || Race_if(PM_ILLITHID)
                                   || Race_if(PM_DRAUGR)
                                   || Race_if(PM_VAMPIRE))))) ? "claw"
                      : "hit",
                mon_nam(mon), canseemon(mon) ? exclam(hmd->dmg) : ".");
    }
}

staticfn void
hmon_hitmon_msg_lightobj(
    struct _hitmon_data *hmd,
    struct monst *mon,
    struct obj *obj UNUSED)
{
    const char *fmt;
    char *whom = mon_nam(mon);
    char emitlightobjbuf[BUFSZ];

    if (canspotmon(mon)) {
        if (hmd->saved_oname[0]) {
            Sprintf(emitlightobjbuf,
                    "%s radiance penetrates deep into",
                    s_suffix(hmd->saved_oname));
            Strcat(emitlightobjbuf, " %s!");
            fmt = emitlightobjbuf;
        } else
            fmt = "The light sears %s!";
    } else {
        *whom = highc(*whom); /* "it" -> "It" */
        fmt = "%s is seared!";
    }
    /* note: s_suffix returns a modifiable buffer */
    if (!noncorporeal(hmd->mdat) && !amorphous(hmd->mdat))
        whom = strcat(s_suffix(whom), " flesh");
    DISABLE_WARNING_FORMAT_NONLITERAL
    pline(fmt, whom);
    RESTORE_WARNING_FORMAT_NONLITERAL
}

/*
 * These will segfault if passed a NULL obj pointer:
 *       hmon_hitmon_weapon_ranged,
 *       hmon_hitmon_weapon_melee,
 *       hmon_hitmon_weapon,
 *       hmon_hitmon_potion,
 *       hmon_hitmon_misc_obj,
 *       hmon_hitmon_poison,
 *       hmon_hitmon_jousting,
 *
 * These are equipped to handle a NULL obj pointer:
 *       hmon_hitmon_stagger,       - obj arg is unused
 *       hmon_hitmon_pet,           - obj arg is unused
 *       hmon_hitmon_msg_lightobj,  - obj arg is unused
 *       hmon_hitmon_do_hit,        - has obj and !obj code paths
 *       hmon_hitmon_splitmon,      - has !obj guards
 *       hmon_hitmon_msg_hit,       - has !obj guards exc. thrown which is ok
 */

/* guts of hmon(); returns True if 'mon' survives */
staticfn boolean
hmon_hitmon(
    struct monst *mon,
    struct obj *obj,
    int thrown, /* HMON_xxx (0 => hand-to-hand, other => ranged) */
    int dieroll)
{
    struct _hitmon_data hmd;
    boolean maybe_knockback = FALSE;

    hmd.dmg = 0;
    hmd.thrown = thrown;
    hmd.twohits = thrown ? 0 : gt.twohits;
    hmd.dieroll = dieroll;
    hmd.mdat = mon->data;
    hmd.use_weapon_skill = FALSE;
    hmd.train_weapon_skill = FALSE;
    hmd.hated_obj = (struct obj *) 0;
    hmd.lightobj = FALSE;
    hmd.material = obj ? (int) obj->material : NO_MATERIAL;
    hmd.jousting = 0;
    hmd.hittxt = FALSE;
    hmd.get_dmg_bonus = TRUE;
    hmd.unarmed = !uwep && !uarm && !uarms;
    hmd.hand_to_hand = (thrown == HMON_MELEE
                        /* not grapnels; applied implies uwep */
                        || (thrown == HMON_APPLIED && is_pole(uwep)));
    hmd.ispoisoned = FALSE;
    hmd.unpoisonmsg = FALSE;
    hmd.needpoismsg = FALSE;
    hmd.poiskilled = FALSE;
    hmd.already_killed = FALSE;
    hmd.offmap = FALSE;
    hmd.destroyed = FALSE;
    hmd.dryit = FALSE;
    hmd.doreturn = FALSE;
    hmd.retval = FALSE;
    hmd.obj_gone = FALSE;
    hmd.saved_oname[0] = '\0';

    hmon_hitmon_do_hit(&hmd, mon, obj);
    /* the hit may have used the object up; the helpers below it take obj
       by value, so our own pointer is the one left dangling */
    if (hmd.obj_gone)
        obj = (struct obj *) 0;
    if (hmd.doreturn)
        return hmd.retval;

    /*
     ***** NOTE: perhaps obj is undefined! (if !thrown && BOOMERANG)
     *      *OR* if attacking bare-handed!
     * Note too: the cases where obj might get destroyed do not
     *      set 'use_weapon_skill', bare-handed does.
     */

    if (hmd.dmg > 0)
        hmon_hitmon_dmg_recalc(&hmd, obj);

    if (hmd.ispoisoned || (obj && (obj->oprops & ITEM_VENOM)))
        hmon_hitmon_poison(&hmd, mon, obj);

    if (hmd.dmg < 1) {
        boolean mon_is_shade = (mon->data == &mons[PM_SHADE]);

        /* make sure that negative damage adjustment can't result
           in inadvertently boosting the victim's hit points */
        hmd.dmg = (hmd.get_dmg_bonus && !mon_is_shade) ? 1 : 0;
        if (mon_is_shade && !hmd.hittxt
            && thrown != HMON_THROWN && thrown != HMON_KICKED)
            /* this gives "harmlessly passes through" feedback even when
               hero doesn't see it happen; presumably sensed by touch? */
            hmd.hittxt = shade_miss(&gy.youmonst, mon, obj, FALSE, TRUE);
    }

    /* potential for gloves with an object property to do additional
       damage when fighting bare-handed */
    if (!hmd.already_killed && !rn2(3) && hmd.hand_to_hand
        && !uwep && !obj && uarmg
        && (uarmg->oprops & (ITEM_FIRE | ITEM_FROST | ITEM_SHOCK))) {
        (void) artifact_hit(&gy.youmonst, mon, uarmg, &hmd.dmg, dieroll);
        if (DEADMONSTER(mon)) {
            hmd.already_killed = TRUE;
            hmd.destroyed = TRUE;
        }
        hmd.hittxt = TRUE;
    }

    if (hmd.jousting) {
        hmon_hitmon_jousting(&hmd, mon, obj);
        /* a shattered lance was used up inside a by-value parameter, so
           our own pointer is stale from here on */
        if (hmd.obj_gone)
            obj = (struct obj *) 0;
    } else if (hmd.unarmed && hmd.dmg > 1 && !thrown && !obj && !Upolyd) {
        hmon_hitmon_stagger(&hmd, mon, obj);
    } else if (!hmd.unarmed && hmd.dmg > 1 && !thrown && !Upolyd
               && !u.twoweap && uwep) {
        maybe_knockback = TRUE;
    }

    if (!hmd.already_killed) {
        if (obj && (obj == uwep || (obj == uswapwep && u.twoweap))
            /* known_hitum 'what counts as a weapon' criteria */
            && (obj->oclass == WEAPON_CLASS || is_weptool(obj))
            && (thrown == HMON_MELEE || thrown == HMON_APPLIED)
            /* if jousting, the hit was already logged */
            && !hmd.jousting
            /* note: caller has already incremented u.uconduct.weaphit
               so we test for 1; 0 shouldn't be able to happen here... */
            && hmd.dmg > 0 && u.uconduct.weaphit <= 1)
            first_weapon_hit(obj);
        mon->mhp -= hmd.dmg;
    }
    /* adjustments might have made tmp become less than what
       a level-draining artifact has already done to max HP */
    if (mon->mhp > mon->mhpmax)
        mon->mhp = mon->mhpmax;
    if (mon->mx == 0) {
        /*
         * jousting can lead to:
         *     mhurtle_to_doom()
         *      mhurtle()
         *       mintrap()
         *        trapeffect_hole()
         *         trapeffect_level_telep()
         *          migrate_to_level()
         * Set offmap in that situation so code to follow can test for it.*/
        hmd.offmap = TRUE;
    }
    if (DEADMONSTER(mon))
        hmd.destroyed = TRUE;

    hmon_hitmon_pet(&hmd, mon, obj);

    hmon_hitmon_splitmon(&hmd, mon, obj);

    hmon_hitmon_msg_hit(&hmd, mon, obj);

    if (hmd.dryit) { /* dryit implies wet towel, so 'obj' is still intact */
        assert(obj != NULL);
        dry_a_towel(obj, -1, TRUE);
    }

    if (hmd.hated_obj)
        searmsg(&gy.youmonst, mon, hmd.hated_obj, FALSE);

    if (hmd.lightobj)
        hmon_hitmon_msg_lightobj(&hmd, mon, obj);

    /* if a "no longer poisoned" message is coming, it will be last;
       obj->opoisoned was cleared above and any message referring to
       "poisoned <obj>" has now been given; we want just "<obj>" for
       last message, so reformat while obj is still accessible */
    if (hmd.unpoisonmsg && obj) {
        Strcpy(hmd.saved_oname, cxname(obj));
    }

    /* [note: thrown obj might go away during killed()/xkilled() call
       (via 'thrownobj'; if swallowed, it gets added to engulfer's
       minvent and might merge with a stack that's already there)] */
    /* already_killed and poiskilled won't apply for Trollsbane */

    if (hmd.needpoismsg) {
        pline_The("poison doesn't seem to affect %s.", mon_nam(mon));
        if (obj && (obj->oprops & ITEM_VENOM)) {
            obj->oprops_known |= ITEM_VENOM;
            update_inventory();
        }
    }
    if (hmd.poiskilled) {
        pline_The("poison was deadly...");
        if (!hmd.already_killed)
            xkilled(mon, XKILL_NOMSG);
        hmd.destroyed = TRUE; /* return FALSE; */
        /* thrown obj might be gone (swallowed hero's missile added to
           engulfer's minvent, possibly merged) */
        if (thrown == HMON_THROWN && !gt.thrownobj)
            obj = (struct obj *) 0;
        /* melee weapon destroyed by death side effects
           (e.g. AT_BOOM explosion -> destroy_item()) */
        else if (thrown != HMON_THROWN && obj && obj != uwep
                 && !(u.twoweap && obj == uswapwep) && obj != uarms)
            obj = (struct obj *) 0;
        if (obj && (obj->oprops & ITEM_VENOM)) {
            obj->oprops_known |= ITEM_VENOM;
            update_inventory();
        }
    } else if (hmd.destroyed) {
        if (!hmd.already_killed) {
            if (troll_baned(mon, obj))
                gm.mkcorpstat_norevive = TRUE;
            killed(mon); /* takes care of most messages */
            gm.mkcorpstat_norevive = FALSE;
        }
        /* thrown obj might be gone (swallowed hero's missile added to
           engulfer's minvent, possibly merged) */
        if (thrown == HMON_THROWN && !gt.thrownobj)
            obj = (struct obj *) 0;
        /* melee weapon destroyed by death side effects
           (e.g. AT_BOOM explosion -> destroy_item()) */
        else if (thrown != HMON_THROWN && obj && obj != uwep
                 && !(u.twoweap && obj == uswapwep) && obj != uarms)
            obj = (struct obj *) 0;
    } else if (u.umconf && hmd.hand_to_hand) {
        nohandglow(mon);
        if (!mon->mconf && !resist(mon, SPBOOK_CLASS, 0, NOTELL)) {
            mon->mconf = 1;
            if (!mon->mstun && !helpless(mon) && canseemon(mon))
                pline("%s appears confused.", Monnam(mon));
        }
    }
    /* nothing to report when the weapon itself is gone; the message that
       destroyed it has already accounted for it */
    if (hmd.unpoisonmsg && !hmd.obj_gone)
        Your("%s %s no longer poisoned.", hmd.saved_oname,
             vtense(hmd.saved_oname, "are"));

    /* glass breakage from the attack. the two monster-side arms are
       skipped when the target has been jousted off the level, since its
       coordinates are gone; the hero's own weapon breaks at the hero's
       square and is unaffected. master has no offmap flag and lets the
       armor arm reach break_glass_obj's isok() failsafe instead */
    if (!hmd.offmap)
        (void) break_glass_obj(some_armor(mon));
    if (obj && (hmd.hand_to_hand
                ? break_glass_obj(obj)
                : (!hmd.offmap && hero_breaks(obj, mon->mx, mon->my, 0)))) {
        obj = (struct obj *) 0;
        hmd.obj_gone = TRUE;
    }
    /* mselftouch() inside the armor break can petrify a monster that
       wields a cockatrice corpse */
    if (DEADMONSTER(mon))
        hmd.destroyed = TRUE;

    if (!hmd.destroyed && !hmd.offmap) {
        int hitflags = M_ATTK_HIT;

        wakeup(mon, TRUE);
        if (maybe_knockback
            && mhitm_knockback(&gy.youmonst, mon, gy.youmonst.data->mattk,
                               &hitflags, TRUE)) {
            if ((hitflags & M_ATTK_DEF_DIED) != 0)
                hmd.destroyed = TRUE;
        }
    }
    return hmd.destroyed ? FALSE : TRUE;
}

/* joust or martial arts punch is knocking the target back; that might
   kill 'mon' (via trap) before known_hitum() has a chance to do so;
   return True if we kill mon, False otherwise. also used by missmu()
   for the two artifact armors that knock an attacker away */
boolean
mhurtle_to_doom(
    struct monst *mon,         /* target monster */
    int tmp,                   /* amount of pending damage */
    struct permonst **mptr)    /* caller's cached copy of mon->data */
{
    /* only hurtle if pending physical damage (tmp) isn't going to kill mon */
    if (tmp < mon->mhp) {
        mhurtle(mon, u.dx, u.dy, 1);
        /* update caller's cached mon->data in case mon was pushed into
           a polymorph trap or is a vampshifter whose current form has
           been killed by a trap so that it reverted to original form */
        *mptr = mon->data;
        if (DEADMONSTER(mon))
            return TRUE;
    }
    return FALSE; /* mon isn't dead yet */
}

/* gamelog version of "you've broken never-hit-with-wielded-weapon conduct;
   the conduct is tracked in known_hitum(); we're called by hmon_hitmon() */
staticfn void
first_weapon_hit(struct obj *weapon)
{
    char buf[BUFSZ];

    /* avoid xname() since that includes "named <foo>" and we don't want
       player-supplied <foo> in livelog */
    buf[0] = '\0';
    /* include "cursed" if known but don't bother with blessed */
    if (weapon->cursed && weapon->bknown)
        Strcat(buf, "cursed "); /* normally supplied by doname() */
    if (obj_is_pname(weapon)) {
        Strcat(buf, ONAME(weapon)); /* fully IDed artifact */
    } else {
        Strcat(buf, simpleonames(weapon));
        if (weapon->oartifact && weapon->dknown)
            Sprintf(eos(buf), " named %s", bare_artifactname(weapon));
    }

    /* when a hit breaks the never-hit-with-wielded-weapon conduct
       (handled by caller) we need to log the message about that before
       monster is possibly killed; otherwise getting log entry sequence
         N : killed for the first time
         N : hit with a wielded weapon for the first time
       reported on the same turn (N) looks "suboptimal" */
    livelog_printf(LL_CONDUCT,
                   "hit with a wielded weapon (%s) for the first time", buf);
}

staticfn boolean
shade_aware(struct obj *obj)
{
    if (!obj)
        return FALSE;
    /*
     * The things in this list either
     * 1) affect shades.
     *  OR
     * 2) are dealt with properly by other routines
     *    when it comes to shades.
     */
    if (obj->otyp == BOULDER
        || obj->otyp == HEAVY_IRON_BALL
        || obj->otyp == IRON_CHAIN      /* dmgval handles those first three */
        || obj->otyp == MIRROR          /* silver in the reflective surface */
        || obj->otyp == CLOVE_OF_GARLIC /* causes shades to flee */
        || objects[obj->otyp].oc_material == SILVER)
        return TRUE;
    return FALSE;
}

/* used for hero vs monster and monster vs monster; also handles
   monster vs hero but that won't happen because hero can't be a shade */
boolean
shade_miss(
    struct monst *magr,
    struct monst *mdef,
    struct obj *obj,
    boolean thrown,
    boolean verbose)
{
    const char *what, *whose, *target;
    boolean youagr = (magr == &gy.youmonst), youdef = (mdef == &gy.youmonst);

    /* we're using dmgval() for zero/not-zero, not for actual damage amount */
    if (mdef->data != &mons[PM_SHADE] || (obj && dmgval(obj, mdef)))
        return FALSE;

    if (verbose
        && ((youdef || cansee(mdef->mx, mdef->my) || sensemon(mdef))
            || (magr == &gy.youmonst && m_next2u(mdef)))) {
        static const char harmlessly_thru[] = " harmlessly through ";

        what = (!obj || shade_aware(obj)) ? "attack" : cxname(obj);
        target = youdef ? "you" : mon_nam(mdef);
        if (!thrown) {
            whose = youagr ? "Your" : s_suffix(Monnam(magr));
            pline("%s %s %s%s%s.", whose, what,
                  vtense(what, "pass"), harmlessly_thru, target);
        } else {
            pline("%s %s%s%s.", The(what), /* note: not pline_The() */
                  vtense(what, "pass"), harmlessly_thru, target);
        }
        if (!youdef && !canspotmon(mdef))
            map_invisible(mdef->mx, mdef->my);
    }
    if (!youdef)
        mdef->msleeping = 0;
    return TRUE;
}

/* check whether slippery clothing protects from hug or wrap attack */
/* [currently assumes that you are the attacker] */
staticfn boolean
m_slips_free(struct monst *mdef, struct attack *mattk)
{
    struct obj *obj;

    if (mattk->adtyp == AD_DRIN) {
        /* intelligence drain attacks the head */
        if (mdef->data->mlet == S_QUADRUPED || mdef->data->mlet == S_DRAGON
            || mdef->data->mlet == S_UNICORN
            || mdef->data == &mons[PM_WARG]
            || mdef->data == &mons[PM_JABBERWOCK]
            || mdef->data == &mons[PM_KI_RIN]
            || mdef->data == &mons[PM_ELDRITCH_KI_RIN])
            obj = which_armor(mdef, W_BARDING);
        else
            obj = which_armor(mdef, W_ARMH);
    } else {
        /* grabbing attacks the body */
        obj = which_armor(mdef, W_ARMC); /* cloak */
        if (!obj)
            obj = which_armor(mdef, W_ARM); /* suit */
        if (!obj)
            obj = which_armor(mdef, W_ARMU); /* shirt */
    }

    /* if monster's cloak/armor is greased, your grab slips off; this
       protection might fail (33% chance) when the armor is cursed */
    if (obj && (obj->greased || obj->otyp == OILSKIN_CLOAK
                || (obj->oprops & ITEM_OILSKIN))
        && (!obj->cursed || rn2(3))) {
        You("%s %s %s %s!",
            (mattk->adtyp == AD_WRAP) ? "slip off of"
                                      : "grab, but cannot hold onto",
            s_suffix(mon_nam(mdef)), obj->greased ? "greased" : "slippery",
            /* avoid "slippery slippery cloak"
               for undiscovered oilskin cloak */
            (obj->greased || objects[obj->otyp].oc_name_known)
                ? xname(obj)
                : cloak_simple_name(obj));

        if (obj->greased && !rn2(2)) {
            pline_The("grease wears off.");
            obj->greased = 0;
        }
        return TRUE;
    }
    return FALSE;
}

/* used when hitting a monster with a lance while mounted;
   1: joust hit; 0: ordinary hit; -1: joust but break lance */
staticfn int
joust(struct monst *mon, /* target */
      struct obj *obj)   /* weapon */
{
    struct obj *barding;
    int skill_rating, joust_dieroll;
    boolean ithilmar = (u.usteed
                        && (barding = which_armor(u.usteed, W_BARDING)) != 0
                        && barding->oartifact == ART_ITHILMAR);

    if (Fumbling || Stunned)
        return 0;
    /* sanity check; lance must be wielded in order to joust */
    if (obj != uwep && (obj != uswapwep || !u.twoweap))
        return 0;
    /* can't joust while trapped--not enough room to maneuver;
     * TODO? if the steed is trapped in a pit, perhaps the hero ought to be
     * able to joust against a monster that's in a conjoined pit */
    if (u.utrap)
        return 0;

    /* if using two weapons, use worse of lance and two-weapon skills */
    skill_rating = P_SKILL(weapon_type(obj)); /* lance skill */
    if (u.twoweap && P_SKILL(P_TWO_WEAPON_COMBAT) < skill_rating)
        skill_rating = P_SKILL(P_TWO_WEAPON_COMBAT);
    if (skill_rating == P_ISRESTRICTED)
        skill_rating = P_UNSKILLED; /* 0=>1 */

    /* odds to joust are expert:80%, skilled:60%, basic:40%, unskilled:20% */
    if ((joust_dieroll = rn2(5)) < skill_rating) {
        /* if your steed is wearing Ithilmar, there's a very good chance
           of preserving your lance while jousting */
        if (joust_dieroll == 0
            && (ithilmar ? (!rn2(3) && rnl(500) == (500 - 1))
                         : (rnl(50) == (50 - 1)))
            && !unsolid(mon->data) && !obj_resists(obj, 0, 100))
            return -1; /* hit that breaks lance */
        return 1;      /* successful joust */
    }
    return 0; /* no joust bonus; revert to ordinary attack */
}

/* send in a demon pet for the hero; exercise wisdom */
staticfn void
demonpet(void)
{
    int i;
    struct permonst *pm;
    struct monst *dtmp;

    pline("Some hell-p has arrived!");
    i = !rn2(6) ? ndemon(u.ualign.type) : NON_PM;
    pm = i != NON_PM ? &mons[i] : gy.youmonst.data;
    if ((dtmp = makemon(pm, u.ux, u.uy, NO_MM_FLAGS)) != 0)
        (void) tamedog(dtmp, (struct obj *) 0, FALSE);
    exercise(A_WIS, TRUE);
}

staticfn boolean
theft_petrifies(struct obj *otmp)
{
    if (uarmg || otmp->otyp != CORPSE
        || !touch_petrifies(&mons[otmp->corpsenm]) || Stone_resistance)
        return FALSE;

#if 0   /* no poly_when_stoned() critter has theft capability */
    if (poly_when_stoned(gy.youmonst.data) && polymon(PM_STONE_GOLEM)) {
        display_nhwindow(WIN_MESSAGE, FALSE);   /* --More-- */
        return TRUE;
    }
#endif

    /* stealing this corpse is fatal... */
    instapetrify(corpse_xname(otmp, "stolen", CXN_ARTICLE));
    /* apparently wasn't fatal after all... */
    return TRUE;
}

/* greased or otherwise slippery gear is hard to get a grip on; report
   the fumble, maybe wear the grease off, and tell the caller that this
   particular theft has failed */
staticfn boolean
steal_slips_off(struct monst *mdef, struct obj *otmp)
{
    const char *adj, *what;

    if (!otmp->greased && otmp->otyp != OILSKIN_CLOAK
        && otmp->otyp != OILSKIN_SACK
        && !is_art(otmp, ART_BAG_OF_THE_HESPERIDES)
        && !(otmp->oprops & ITEM_OILSKIN))
        return FALSE;
    /* a cursed item occasionally clings to its owner instead */
    if (otmp->cursed && !rn2(4))
        return FALSE;

    adj = otmp->greased ? "greased" : "slippery";
    what = (otmp->greased || objects[otmp->otyp].oc_name_known)
               ? xname(otmp)
               : simpleonames(otmp);
    /* an unidentified oilskin cloak is already called a slippery
       cloak, so the adjective would otherwise be said twice */
    if (!strncmpi(what, adj, strlen(adj)))
        adj = "";

    Your("%s slip off of %s %s%s%s!", makeplural(body_part(HAND)),
         s_suffix(mon_nam(mdef)), adj, *adj ? " " : "", what);
    if (otmp->greased && !rn2(2)) {
        pline_The("grease wears off.");
        otmp->greased = 0;
    }
    return TRUE;
}

/* the town watch takes a dim view of a botched pickpocketing */
staticfn void
pickpocket_watch_responds(struct monst *mdef)
{
    struct monst *mon;

    for (mon = fmon; mon; mon = mon->nmon) {
        if (DEADMONSTER(mon))
            continue;
        /* the mpeaceful test below catches this since mdef is no
           longer peaceful, but be explicit */
        if (mon == mdef)
            continue;
        if (!mon->mpeaceful || mon->msleeping)
            continue;
        if (!humanoid(mon->data) && !mon->isshk && !mon->ispriest)
            continue;
        if (is_watch(mon->data) && mon->mcansee && m_canseeu(mon)) {
            SetVoice(mon, 0, 80, 0);
            verbalize("Halt!  You're under arrest!");
            (void) angry_guards(!!Deaf);
        }
    }
}

/*
 * Player uses theft attack against monster.
 *
 * If the target is wearing body armor, take all of its possessions;
 * otherwise, take one object.  [Is this really the behavior we want?]
 */
staticfn void
steal_it(struct monst *mdef, struct attack *mattk)
{
    struct obj *otmp, *gold = 0, *ustealo, **minvent_ptr;
    long unwornmask;
    int i = rn2(10), dex_pick = 0, no_vis = 0,
        size = 0, enc = 0, other = 0, cap;
    /* seducing the target strips it bare but leaves its gold alone;
       an ordinary pickpocket takes one item and may take the coins */
    boolean as_mon = (could_seduce(&gy.youmonst, mdef, mattk) != 0);

    otmp = mdef->minvent;
    if (!otmp || (as_mon && otmp->oclass == COIN_CLASS && !otmp->nobj))
        return; /* nothing to take */

    /* look for worn body armor */
    ustealo = (struct obj *) 0;
    if (as_mon && mdef->mcanmove) {
        /* find armor, and move it to end of inventory in the process */
        minvent_ptr = &mdef->minvent;
        while ((otmp = *minvent_ptr) != 0)
            if (otmp->owornmask & W_ARM) {
                if (ustealo)
                    panic("steal_it: multiple worn suits");
                *minvent_ptr = otmp->nobj; /* take armor out of minvent */
                ustealo = otmp;
                ustealo->nobj = (struct obj *) 0;
            } else {
                minvent_ptr = &otmp->nobj;
            }
        *minvent_ptr = ustealo; /* put armor back into minvent */
    }
    /* the coins are spared whenever the theft is a seduction, even when
       the target is too incapacitated for the strip-it-bare path above */
    if (as_mon)
        gold = findgold(mdef->minvent);

    /* prevent gold from being stolen so that steal-item isn't a superset
       of steal-gold; shuffling it out of minvent before selecting next
       item, and then back in case hero or monster dies (hero touching
       stolen c'trice corpse or monster wielding one and having gloves
       stolen) is less bookkeeping than skipping it within the loop or
       taking it out once and then trying to figure out how to put it back;
       a pickpocket is not seducing anyone, so coins are fair game there */
    if (gold)
        obj_extract_self(gold);

    if (ustealo) { /* we will be taking everything */
        char heshe[20];

        /* 5.0: this uses hero's base gender rather than nymph femininity
           but was using hardcoded pronouns She/her for target monster;
           switch to dynamic pronoun */
        if (gender(mdef) == (int) u.mfemale
            && gy.youmonst.data->mlet == S_NYMPH)
            You("charm %s.  %s gladly hands over %s%s possessions.",
                mon_nam(mdef), upstart(strcpy(heshe, mhe(mdef))),
                !gold ? "" : "most of ", mhis(mdef));
        else
            You("seduce %s and %s starts to take off %s clothes.",
                mon_nam(mdef), mhe(mdef), mhis(mdef));
        otmp = mdef->minvent;
    } else {
        /* only one object is taken, so pick it at random */
        int nitems = 0;

        for (otmp = mdef->minvent; otmp; otmp = otmp->nobj)
            ++nitems;
        nitems = rnd(nitems);
        for (otmp = mdef->minvent; nitems > 1; --nitems)
            otmp = otmp->nobj;
    }

    /* greased objects are difficult to get a grip on, hence
       the odds that an attempt at stealing it may fail */
    if (otmp && steal_slips_off(mdef, otmp)) {
        if (gold)
            mpickobj(mdef, gold);
        return;
    }

    /* Rogues and Convicts can use the thievery skill;
       dexterity directly affects how successful a
       pickpocketing attempt will be */
    if (ACURR(A_DEX) <= 6)
        dex_pick += 3;
    else if (ACURR(A_DEX) <= 9)
        dex_pick += 2;
    else if (ACURR(A_DEX) <= 15)
        dex_pick -= 0;
    else if (ACURR(A_DEX) <= 18)
        dex_pick -= 1;
    else if (ACURR(A_DEX) <= 21)
        dex_pick -= 2;
    else if (ACURR(A_DEX) <= 24)
        dex_pick -= 3;
    else if (ACURR(A_DEX) == 25)
        dex_pick -= 4;

    /* varying degrees of perception */
    if (!mdef->mcansee && haseyes(mdef->data)) {
        /* target is blind */
        no_vis -= 4;
    } else if (!m_canseeu(mdef)) {
        /* thief is invisible */
        no_vis -= 3;
    } else if (mdef->mcansee && haseyes(mdef->data)
               && spot_is_dark(u.ux, u.uy)) {
        /* thief is in darkness, target can see */
        if (Race_if(PM_DROW))
            no_vis -= 3;
        else
            no_vis -= 2;
    }

    /* slight bonus if the thief is small and
       its target is much bigger */
    if ((Race_if(PM_GNOME) || Race_if(PM_HOBBIT))
        && mdef->data->msize >= MZ_LARGE)
        size -= 1;

    /* being encumbered adversely affects pickpocketing success rate,
       to the point where it becomes impossible if the level of
       encumbrance is too high */
    if ((cap = near_capacity()) > UNENCUMBERED) {
        switch (cap) {
        case SLT_ENCUMBER:
            enc += 2;
            break; /* burdened */
        case MOD_ENCUMBER:
            enc += 6;
            break; /* stressed */
        case HVY_ENCUMBER:
            enc += 10;
            break; /* strained */
        case EXT_ENCUMBER:
        case OVERLOADED:
            enc += 20;
            break; /* overtaxed/overloaded:
                      can't 'fight' when this overburdened */
        }
    }

    /* other conditions that could affect success, and these
       can stack if multiple conditions are met */
    if (has_magic_key(&gy.youmonst))
        other -= 6; /* MKoT in open inventory, meets is_magic_key() criteria */
    if (mdef->mfrozen) /* target is immobile or incapacitated */
        other -= 5;
    if (mdef->mconf || mdef->mstun) /* target can't think straight */
        other -= 3;
    if (uarms) /* wearing a shield */
        other += 2;
    if (uarm && is_heavy_metallic(uarm)) /* wearing bulky body armor */
        other += 3;
    if (Wounded_legs) /* hard to move deftly */
        other += 5;
    if (u.usteed) /* steed can hamper stealth */
        other += 5;
    if (!canspotmon(mdef)) /* can't see/sense target */
        other += 6;
    if (Glib) /* slippery fingers */
        other += 7;
    if (Fumbling) /* difficult motor control */
        other += 10;
    if (Confusion || Stunned) /* hard to do much anything if impaired */
        other += 20;
    if (mdef->data == &mons[PM_ORACLE]) {
        /* stealing from the Oracle will never succeed,
           although the attempt is allowed [see AD_MAGM in passive()] */
        other += 50;
    }

    /* failure routine */
    if (!Upolyd
        && (i + dex_pick + no_vis + size + enc + other)
               > P_SKILL(P_THIEVERY)) {
        if (Confusion || Stunned) {
            You("are in no shape to %s anything.",
                rn2(2) ? "pickpocket" : "steal");
        } else {
            Your("attempt to %s %s %s.",
                 rn2(2) ? "pickpocket" : "steal from",
                 mon_nam(mdef), rn2(2) ? "failed" : "was unsuccessful");
            if (!rn2(10) && P_SKILL(P_THIEVERY) == P_BASIC)
                You("could use more practice at pickpocketing.");
            /* there's a chance a monster being pickpocketed will notice;
               as expected, they're not too happy about it, and a good
               bit of this comes from setmangry() */
            if (mdef->mpeaceful && rnd(6) > P_SKILL(P_THIEVERY)) {
                mdef->mpeaceful = 0;
                set_malign(mdef);
                newsym(mdef->mx, mdef->my); /* clear peaceful glyph */
                if (mdef->ispriest) {
                    if (p_coaligned(mdef)) {
                        You_feel("guilty.");
                        adjalign(-5); /* very bad */
                        record_abuse_event(-5, ABUSE_PICKPOCKET);
                    } else {
                        adjalign(2);
                    }
                }
                if (couldsee(mdef->mx, mdef->my)) {
                    if (humanoid(mdef->data) || mdef->isshk || mdef->isgd)
                        pline_mon(mdef, "%s notices your pickpocketing"
                                        " attempt and gets angry!",
                                  Monnam(mdef));
                    else if (flags.verbose && !Deaf)
                        growl(mdef);
                }
                /* robbing your own quest leader will anger the guardians */
                if (!svc.context.mon_moving /* always the case here */
                    && mdef->data == &mons[quest_info(MS_LEADER)])
                    qst_guardians_respond();
                /* make the watch react */
                if (!svc.context.mon_moving)
                    pickpocket_watch_responds(mdef);
            }
        }
        if (gold)
            mpickobj(mdef, gold);
        return;
    }

    while (ustealo ? ((otmp = mdef->minvent) != 0) : (otmp != 0)) {
        if (gold) /* put 'mdef's gold back after remembering mdef->minvent */
            mpickobj(mdef, gold), gold = 0;
        /* only reachable once items have already been stolen; failing to
           steal a greased object stops further theft so that the loop
           can't spin on an item that can't be picked up */
        if (steal_slips_off(mdef, otmp))
            break;
        unwornmask = otmp->owornmask;
        /* this would take place when doname() formats the object for
           the hold_another_object() call, but we want to do it before
           otmp gets removed from mdef's inventory */
        if (otmp->oartifact && !Blind)
            find_artifact(otmp);
        /* take the object away from the monster */
        extract_from_minvent(mdef, otmp, TRUE, FALSE);
        /* special message for final item; no need to check owornmask because
         * ustealo is only set on objects with (owornmask & W_ARM) */
        if (otmp == ustealo)
            pline("%s finishes taking off %s suit.", Monnam(mdef),
                  mhis(mdef));
        /* give the object to the character */
        otmp = hold_another_object(otmp, "You snatched but dropped %s.",
                                   doname(otmp), "You steal: ");
        /* might have dropped otmp, and it might have broken or left level */
        if (!otmp || otmp->where != OBJ_INVENT) {
            if (!ustealo)
                break; /* only taking one item */
            /* the next item comes out of minvent, so take the gold
               back out before looping around to select it */
            if ((gold = findgold(mdef->minvent)) != 0)
                obj_extract_self(gold);
            continue;
        }
        if (theft_petrifies(otmp))
            break; /* stop thieving even though hero survived */
        /* more take-away handling, after theft message */
        if (unwornmask & W_WEP) { /* stole wielded weapon */
            possibly_unwield(mdef, FALSE);
        } else if (unwornmask & W_ARMG) { /* stole worn gloves */
            mselftouch(mdef, (const char *) 0, TRUE);
            if (DEADMONSTER(mdef)) /* it's now a statue */
                break; /* can't continue stealing */
        }

        if (!ustealo)
            break; /* only taking one item */
        if (!Upolyd)
            break; /* no longer have ability to steal */

        /* take gold out of minvent before making next selection; if it
           is the only thing left, the loop will terminate and it will be
           put back below */
        if ((gold = findgold(mdef->minvent)) != 0)
            obj_extract_self(gold);
    }

    /* put gold back; won't happen if either hero or 'mdef' dies because
       gold will be back in monster's inventory at either of those times
       (so will be present in mdef's minvent for bones, or in its statue
       now if it has just been turned into one) */
    if (gold)
        mpickobj(mdef, gold);

    /* only train thievery if you have the skill */
    if (Role_if(PM_ROGUE) || Role_if(PM_CONVICT)) {
        /* train the skill here instead of in hmon_hitmon() to avoid
           premature messages about feeling more dangerous when theft
           fails */
        use_skill(P_THIEVERY, 1);

        /* Training up thievery skill can be quite the chore, to the point
           where many players don't bother to use it. Randomly add an
           additional point of skill for a successful attempt to balance
           out the drudgery, without having to revamp how skill points are
           awarded for this specific skill */
        if (rn2(3)) /* 66% chance in favor of the player */
            use_skill(P_THIEVERY, 1);
    }
}

/* Find the single valid adjacent steal target if exactly one exists.
   Returns the monster if found, NULL if zero or multiple */
staticfn struct monst *
find_steal_target(void)
{
    struct monst *mtmp, *found = (struct monst *) 0;
    int dx, dy;

    for (dx = -1; dx <= 1; dx++) {
        for (dy = -1; dy <= 1; dy++) {
            if (!dx && !dy)
                continue;
            if (!isok(u.ux + dx, u.uy + dy))
                continue;
            mtmp = m_at(u.ux + dx, u.uy + dy);
            /* a long worm can occupy several adjacent spots */
            if (!mtmp || mtmp == found)
                continue;
            if (!canspotmon(mtmp))
                continue;
            /* skip targets the steal code always refuses */
            if (mtmp->mtame)
                continue;
            if (mtmp->isshk && !strcmp(shkname(mtmp), "Izchak"))
                continue;
            if (found)
                return (struct monst *) 0; /* more than one */
            found = mtmp;
        }
    }
    return found;
}

/* the #steal command -- Rogue/Convict thievery via extended command,
   replaces the old forcefight-triggered thievery mechanic */
int
dosteal(void)
{
    coord cc;
    struct monst *mtmp;

    if (!Role_if(PM_ROGUE) && !Role_if(PM_CONVICT)) {
        You("don't have any thieving skills.");
        return ECMD_OK;
    }
    if (Upolyd) {
        You_cant("steal anything in this form.");
        return ECMD_OK;
    }
    if (uwep) {
        You("need your hands free to steal.");
        return ECMD_OK;
    }
    if (u.uswallow) {
        pline("What exactly were you planning on stealing?  Its stomach?");
        return ECMD_OK;
    }

    /* if exactly one valid adjacent target exists, auto-select it */
    mtmp = find_steal_target();
    if (!mtmp) {
        cc.x = u.ux;
        cc.y = u.uy;
        if (getpos(&cc, FALSE, "the monster you want to steal from") < 0)
            return ECMD_CANCEL; /* aborted -- no turn cost */

        if (!isok(cc.x, cc.y))
            return ECMD_CANCEL;

        mtmp = m_at(cc.x, cc.y);
        if (!mtmp || !canspotmon(mtmp)) {
            pline("There's no one there to steal from.");
            return ECMD_OK;
        }
        if (distu(cc.x, cc.y) > 2) {
            pline("You're too far away to steal from %s.", mon_nam(mtmp));
            return ECMD_OK;
        }
    }

    /* Izchak is off-limits */
    if (mtmp->isshk && !strcmp(shkname(mtmp), "Izchak")) {
        You("find yourself unable to steal from %s.", mon_nam(mtmp));
        return ECMD_OK;
    }
    /* pets are off-limits */
    if (mtmp->mtame) {
        You_cant("bring yourself to steal from %s.", mon_nam(mtmp));
        return ECMD_OK;
    }

    /* wrath of the gods for attempting to steal from a peaceful Oracle */
    if (mtmp->mpeaceful && mtmp->data == &mons[PM_ORACLE]) {
        pline_The("gods notice your deception!");
        setmangry(mtmp, FALSE);
    }

    if (mtmp->minvent) {
        You("%s to %s %s.", rn2(2) ? "try" : "attempt",
            rn2(2) ? "steal from" : "pickpocket", mon_nam(mtmp));
        steal_it(mtmp, &gy.youmonst.data->mattk[0]);
    } else {
        pline("%s has nothing for you to %s.", Monnam(mtmp),
              rn2(2) ? "steal" : "pickpocket");
    }
    return ECMD_TIME;
}

void
mhitm_ad_rust(
    struct monst *magr, struct attack *mattk,
    struct monst *mdef, struct mhitm_data *mhm)
{
    struct permonst *pd = mdef->data;

    if (magr == &gy.youmonst) {
        /* uhitm */
        if (completelyrusts(pd)) { /* iron golem */
            /* note: the life-saved case is hypothetical because
               life-saving doesn't work for golems */
            pline("%s %s to pieces!", Monnam(mdef),
                  !mlifesaver(mdef) ? "falls" : "starts to fall");
            xkilled(mdef, XKILL_NOMSG);
            mhm->hitflags |= M_ATTK_DEF_DIED;
        }
        erode_armor(mdef, ERODE_RUST);
        mhm->damage = 0; /* damageum(), int tmp */
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        hitmsg(magr, mattk);
        if (magr->mcan) {
            return;
        }
        if (completelyrusts(pd)) {
            You("rust!");
            /* KMH -- this is okay with unchanging */
            rehumanize();
            return;
        }
        erode_armor(&gy.youmonst, ERODE_RUST);
    } else {
        /* mhitm */
        if (magr->mcan)
            return;
        if (completelyrusts(pd)) { /* PM_IRON_GOLEM */
            if (gv.vis && canseemon(mdef))
                pline_mon(mdef, "%s %s to pieces!", Monnam(mdef),
                      !mlifesaver(mdef) ? "falls" : "starts to fall");
            monkilled(mdef, (char *) 0, AD_RUST);
            if (!DEADMONSTER(mdef)) {
                mhm->hitflags = M_ATTK_MISS;
                mhm->done = TRUE;
                return;
            }
            mhm->hitflags = (M_ATTK_DEF_DIED | (grow_up(magr, mdef) ? 0
                                            : M_ATTK_AGR_DIED));
            mhm->done = TRUE;
            return;
        }
        erode_armor(mdef, ERODE_RUST);
        mdef->mstrategy &= ~STRAT_WAITFORU;
        mhm->damage = 0; /* mdamagem(), int tmp */
    }
}

void
mhitm_ad_corr(
    struct monst *magr, struct attack *mattk,
    struct monst *mdef, struct mhitm_data *mhm)
{
    if (magr == &gy.youmonst) {
        /* uhitm */
        erode_armor(mdef, ERODE_CORRODE);
        mhm->damage = 0;
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        hitmsg(magr, mattk);
        if (magr->mcan)
            return;
        erode_armor(mdef, ERODE_CORRODE);
    } else {
        /* mhitm */
        if (magr->mcan)
            return;
        erode_armor(mdef, ERODE_CORRODE);
        mdef->mstrategy &= ~STRAT_WAITFORU;
        mhm->damage = 0;
    }
}

void
mhitm_ad_dcay(
    struct monst *magr, struct attack *mattk,
    struct monst *mdef, struct mhitm_data *mhm)
{
    struct permonst *pd = mdef->data;

    if (magr == &gy.youmonst) {
        /* uhitm */
        if (completelyrots(pd)) { /* wood golem or leather golem */
            pline("%s %s to pieces!", Monnam(mdef),
                  !mlifesaver(mdef) ? "falls" : "starts to fall");
            xkilled(mdef, XKILL_NOMSG);
        }
        erode_armor(mdef, ERODE_ROT);
        mhm->damage = 0;
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        hitmsg(magr, mattk);
        if (magr->mcan)
            return;
        if (completelyrots(pd)) {
            You("rot!");
            /* KMH -- this is okay with unchanging */
            rehumanize();
            return;
        }
        erode_armor(mdef, ERODE_ROT);
    } else {
        /* mhitm */
        if (magr->mcan)
            return;
        if (completelyrots(pd)) { /* PM_WOOD_GOLEM || PM_LEATHER_GOLEM */
            /* note: the life-saved case is hypothetical because
               life-saving doesn't work for golems */
            if (gv.vis && canseemon(mdef))
                pline_mon(mdef, "%s %s to pieces!", Monnam(mdef),
                      !mlifesaver(mdef) ? "falls" : "starts to fall");
            monkilled(mdef, (char *) 0, AD_DCAY);
            if (!DEADMONSTER(mdef)) {
                mhm->done = TRUE;
                mhm->hitflags = M_ATTK_MISS;
                return;
            }
            mhm->done = TRUE;
            mhm->hitflags = (M_ATTK_DEF_DIED
                             | (grow_up(magr, mdef) ? 0 : M_ATTK_AGR_DIED));
            return;
        }
        erode_armor(mdef, ERODE_ROT);
        mdef->mstrategy &= ~STRAT_WAITFORU;
        mhm->damage = 0;
    }
}

void
mhitm_ad_dren(
    struct monst *magr, struct attack *mattk,
    struct monst *mdef, struct mhitm_data *mhm)
{
    boolean negated = mhitm_mgc_atk_negated(magr, mdef, FALSE);

    if (magr == &gy.youmonst) {
        /* uhitm */
        if (!negated && !rn2(4))
            xdrainenergym(mdef, TRUE);
        mhm->damage = 0;
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        hitmsg(magr, mattk);
        if (!negated && !rn2(4)) /* 25% chance */
            drain_en(mhm->damage, FALSE);
        mhm->damage = 0;
    } else {
        /* mhitm */
        if (!negated && !rn2(4))
            xdrainenergym(mdef, (boolean) (gv.vis && canspotmon(mdef)
                                           && mattk->aatyp != AT_ENGL));
        mhm->damage = 0;
    }
}

void
mhitm_ad_drli(
    struct monst *magr, struct attack *mattk,
    struct monst *mdef, struct mhitm_data *mhm)
{
    if (magr == &gy.youmonst) {
        /* uhitm */
        if (!rn2(3) && !(resists_drli(mdef) || defended(mdef, AD_DRLI))
            && !mhitm_mgc_atk_negated(magr, mdef, TRUE)) {
            if (maybe_polyd(is_vampire(gy.youmonst.data),
                            Race_if(PM_VAMPIRE))
                && mattk->aatyp == AT_BITE
                && (!has_blood(mdef->data) || u.uhunger > 1250)) {
                /* don't execute the draining effect if we were not
                   able to feed; the normal bite damage still applies.
                   Satiated is u.uhunger at 1000-1999; allow some
                   satiation, but don't go overboard */
                return;
            }
            mhm->damage = d(2, 6); /* Stormbringer uses monhp_per_lvl
                                    * (usually 1d8) */
            if (maybe_polyd(is_vampire(gy.youmonst.data),
                            Race_if(PM_VAMPIRE))
                && mattk->aatyp == AT_BITE) {
                /* For the life of a creature is in the blood */
                if (flags.verbose)
                    You("feed on its lifeblood.");
                /* Biting monsters does not count against
                   eating conducts. The draining of life is
                   considered to be primarily a non-physical effect */
                lesshungry(min(mhm->damage, mdef->mhp) * 6);
            }
            pline("%s becomes weaker!", Monnam(mdef));
            if (mdef->mhpmax - mhm->damage > (int) mdef->m_lev) {
                mdef->mhpmax -= mhm->damage;
            } else {
                /* limit floor of mhpmax reduction to current m_lev + 1;
                   avoid increasing it if somehow already less than that */
                if (mdef->mhpmax > (int) mdef->m_lev)
                    mdef->mhpmax = (int) mdef->m_lev + 1;
            }
            mdef->mhp -= mhm->damage;
            /* !m_lev: level 0 monster is killed regardless of hit points
               rather than drop to level -1; note: some non-living creatures
               (golems, vortices) are subject to life-drain */
            if (DEADMONSTER(mdef) || !mdef->m_lev) {
                pline("%s %s!", Monnam(mdef),
                      nonliving(mdef->data) ? "expires" : "dies");
                xkilled(mdef, XKILL_NOMSG);
            } else
                mdef->m_lev--;
            mhm->damage = 0; /* damage has already been inflicted */

            /* unlike hitting with Stormbringer, wounded hero doesn't
               heal any from the drained life */
        }
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        hitmsg(magr, mattk);
        if (!rn2(3) && !Drain_resistance
            && !mhitm_mgc_atk_negated(magr, mdef, TRUE)){
            losexp("life drainage");

            /* unlike hitting with Stormbringer, wounded attacker doesn't
               heal any from the drained life */
        }
    } else {
        /* mhitm */
        /* mhitm_ad_deth gets redirected here for Death's touch */
        boolean is_death = (mattk->adtyp == AD_DETH);

        if (is_death
            || (!rn2(3) && !(resists_drli(mdef) || defended(mdef, AD_DRLI))
                && !mhitm_mgc_atk_negated(magr, mdef, TRUE))) {
            if (!is_death) /* Stormbringer uses monhp_per_lvl (1d8) */
                mhm->damage = d(2, 6);
            if (gv.vis && canspotmon(mdef))
                pline_mon(mdef, "%s becomes weaker!", Monnam(mdef));
            if (mdef->mhpmax - mhm->damage > (int) mdef->m_lev) {
                mdef->mhpmax -= mhm->damage;
            } else {
                /* limit floor of mhpmax reduction to current m_lev + 1;
                   avoid increasing it if somehow already less than that */
                if (mdef->mhpmax > (int) mdef->m_lev)
                    mdef->mhpmax = (int) mdef->m_lev + 1;
            }
            if (mdef->m_lev == 0) /* automatic kill if drained past level 0 */
                mhm->damage = mdef->mhp;
            else
                mdef->m_lev--;

            /* unlike hitting with Stormbringer, wounded attacker doesn't
               heal any from the drained life */
        }
    }
}

void
mhitm_ad_fire(
    struct monst *magr, struct attack *mattk,
    struct monst *mdef, struct mhitm_data *mhm)
{
    struct permonst *pd = mdef->data;
    const int orig_dmg = mhm->damage; /* damage coming into the function */

    if (magr == &gy.youmonst) {
        /* uhitm */
        if (mhitm_mgc_atk_negated(magr, mdef, TRUE)) {
            mhm->damage = 0;
            return;
        }
        if (!Blind)
            pline("%s is %s!", Monnam(mdef), on_fire(pd, mattk));
        if (completelyburns(pd)) { /* paper golem or straw golem */
            if (!Blind)
                /* note: the life-saved case is hypothetical because
                   life-saving doesn't work for golems */
                pline("%s %s!", Monnam(mdef),
                      !mlifesaver(mdef) ? "burns completely"
                                        : "is totally engulfed in flames");
            else
                You("smell burning%s.",
                    (pd == &mons[PM_PAPER_GOLEM]) ? " paper"
                      : (pd == &mons[PM_STRAW_GOLEM]) ? " straw" : "");
            xkilled(mdef, XKILL_NOMSG | XKILL_NOCORPSE);
            mhm->damage = 0;
            return;
            /* Don't return yet; keep hp<1 and mhm.damage=0 for pet msg */
        }
        if (resists_fire(mdef) || defended(mdef, AD_FIRE)) {
            if (!Blind)
                pline_The("fire doesn't heat %s!", mon_nam(mdef));
            golemeffects(mdef, AD_FIRE, mhm->damage);
            shieldeff(mdef->mx, mdef->my);
            mhm->damage = 0;
        }
        mhm->damage += destroy_items(mdef, AD_FIRE, orig_dmg);
        ignite_items(mdef->minvent);
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        hitmsg(magr, mattk);
        if (!mhitm_mgc_atk_negated(magr, mdef, TRUE)) {
            pline("You're %s!", on_fire(pd, mattk));
            if (completelyburns(pd)) { /* paper or straw golem */
                You("go up in flames!");
                monstunseesu(M_SEEN_FIRE);
                /* KMH -- this is okay with unchanging */
                rehumanize();
                return;
            } else if (how_resistant(FIRE_RES) == 100 || Underwater) {
                if (Underwater)
                    pline_The("fire quickly fizzles out.");
                else
                    pline_The("fire doesn't feel hot!");
                monstseesu(M_SEEN_FIRE);
                mhm->damage = 0;
            } else {
                mhm->damage = resist_reduce(mhm->damage, FIRE_RES);
                monstunseesu(M_SEEN_FIRE);
            }
            if (!Underwater) {
                if ((int) magr->m_lev > rn2(20)) {
                    (void) destroy_items(&gy.youmonst, AD_FIRE, orig_dmg);
                    ignite_items(gi.invent);
                }
                burn_away_slime();
            }
        } else {
            mhm->damage = 0;
        }
    } else {
        /* mhitm */
        if (mhitm_mgc_atk_negated(magr, mdef, TRUE)) {
            mhm->damage = 0;
            return;
        }
        if (gv.vis && canseemon(mdef))
            pline_mon(mdef, "%s is %s!", Monnam(mdef), on_fire(pd, mattk));
        if (completelyburns(pd)) { /* paper golem or straw golem */
            /* note: the life-saved case is hypothetical because
               life-saving doesn't work for golems */
            if (gv.vis && canseemon(mdef))
                pline_mon(mdef, "%s %s!", Monnam(mdef),
                      !mlifesaver(mdef) ? "burns completely"
                                        : "is totally engulfed in flames");
            monkilled(mdef, (char *) 0, AD_FIRE);
            if (!DEADMONSTER(mdef)) {
                mhm->hitflags = M_ATTK_MISS;
                mhm->done = TRUE;
                return;
            }
            mhm->hitflags = (M_ATTK_DEF_DIED
                             | (grow_up(magr, mdef) ? 0 : M_ATTK_AGR_DIED));
            mhm->done = TRUE;
            return;
        }
        if (resists_fire(mdef) || defended(mdef, AD_FIRE)) {
            if (gv.vis && canseemon(mdef))
                pline_The("fire doesn't seem to burn %s!", mon_nam(mdef));
            shieldeff(mdef->mx, mdef->my);
            golemeffects(mdef, AD_FIRE, mhm->damage);
            mhm->damage = 0;
        }
        mhm->damage += destroy_items(mdef, AD_FIRE, orig_dmg);
        ignite_items(mdef->minvent);
    }
}

void
mhitm_ad_cold(
    struct monst *magr, struct attack *mattk,
    struct monst *mdef, struct mhitm_data *mhm)
{
    const int orig_dmg = mhm->damage;

    if (magr == &gy.youmonst) {
        /* uhitm */
        if (mhitm_mgc_atk_negated(magr, mdef, TRUE)) {
            mhm->damage = 0;
            return;
        }
        if (!Blind)
            pline("%s is covered in frost!", Monnam(mdef));
        if (resists_cold(mdef) || defended(mdef, AD_COLD)) {
            shieldeff(mdef->mx, mdef->my);
            if (!Blind)
                pline_The("frost doesn't chill %s!", mon_nam(mdef));
            golemeffects(mdef, AD_COLD, mhm->damage);
            mhm->damage = 0;
        }
        mhm->damage += destroy_items(mdef, AD_COLD, orig_dmg);
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        hitmsg(magr, mattk);
        if (!mhitm_mgc_atk_negated(magr, mdef, TRUE)) {
            pline("You're covered in frost!");
            if (how_resistant(COLD_RES) == 100) {
                pline_The("frost doesn't seem cold!");
                monstseesu(M_SEEN_COLD);
                mhm->damage = 0;
            } else {
                mhm->damage = resist_reduce(mhm->damage, COLD_RES);
                monstunseesu(M_SEEN_COLD);
            }
            if ((int) magr->m_lev > rn2(20))
                (void) destroy_items(&gy.youmonst, AD_COLD, orig_dmg);
        } else
            mhm->damage = 0;
    } else {
        /* mhitm */
        if (mhitm_mgc_atk_negated(magr, mdef, TRUE)) {
            mhm->damage = 0;
            return;
        }
        if (gv.vis && canseemon(mdef))
            pline_mon(mdef, "%s is covered in frost!", Monnam(mdef));
        if (resists_cold(mdef) || defended(mdef, AD_COLD)) {
            if (gv.vis && canseemon(mdef))
                pline_The("frost doesn't seem to chill %s!", mon_nam(mdef));
            shieldeff(mdef->mx, mdef->my);
            golemeffects(mdef, AD_COLD, mhm->damage);
            mhm->damage = 0;
        }
        mhm->damage += destroy_items(mdef, AD_COLD, orig_dmg);
    }
}

void
mhitm_ad_elec(
    struct monst *magr, struct attack *mattk,
    struct monst *mdef, struct mhitm_data *mhm)
{
    const int orig_dmg = mhm->damage;

    if (magr == &gy.youmonst) {
        /* uhitm */
        if (mhitm_mgc_atk_negated(magr, mdef, TRUE)) {
            mhm->damage = 0;
            return;
        }
        if (!Blind)
            pline("%s is zapped!", Monnam(mdef));
        if (resists_elec(mdef) || defended(mdef, AD_ELEC)) {
            if (!Blind)
                pline_The("zap doesn't shock %s!", mon_nam(mdef));
            golemeffects(mdef, AD_ELEC, mhm->damage);
            shieldeff(mdef->mx, mdef->my);
            mhm->damage = 0;
        }
        mhm->damage += destroy_items(mdef, AD_ELEC, orig_dmg);
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        hitmsg(magr, mattk);
        if (!mhitm_mgc_atk_negated(magr, mdef, TRUE)) {
            You("get zapped!");
            if (how_resistant(SHOCK_RES) == 100) {
                pline_The("zap doesn't shock you!");
                monstseesu(M_SEEN_ELEC);
                mhm->damage = 0;
            } else {
                mhm->damage = resist_reduce(mhm->damage, SHOCK_RES);
                monstunseesu(M_SEEN_ELEC);
            }
            if ((int) magr->m_lev > rn2(20))
                (void) destroy_items(&gy.youmonst, AD_ELEC, orig_dmg);
        } else
            mhm->damage = 0;
    } else {
        /* mhitm */
        if (mhitm_mgc_atk_negated(magr, mdef, TRUE)) {
            mhm->damage = 0;
            return;
        }
        if (gv.vis && canseemon(mdef))
            pline_mon(mdef, "%s gets zapped!", Monnam(mdef));
        if (resists_elec(mdef) || defended(mdef, AD_ELEC)) {
            if (gv.vis && canseemon(mdef))
                pline_The("zap doesn't shock %s!", mon_nam(mdef));
            shieldeff(mdef->mx, mdef->my);
            golemeffects(mdef, AD_ELEC, mhm->damage);
            mhm->damage = 0;
        }
        mhm->damage += destroy_items(mdef, AD_ELEC, orig_dmg);
    }
}

void
mhitm_ad_acid(
    struct monst *magr, struct attack *mattk,
    struct monst *mdef, struct mhitm_data *mhm)
{
    if (magr == &gy.youmonst) {
        /* uhitm */
        if (resists_acid(mdef) || defended(mdef, AD_ACID))
            mhm->damage = 0;
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        hitmsg(magr, mattk);
        if (!magr->mcan && !rn2(3))
            if (Acid_resistance) {
                pline("You're covered in %s, but it seems harmless.",
                      hliquid("acid"));
                monstseesu(M_SEEN_ACID);
                mhm->damage = 0;
            } else {
                pline("You're covered in %s!  It burns!", hliquid("acid"));
                exercise(A_STR, FALSE);
                monstunseesu(M_SEEN_ACID);
            }
        else
            mhm->damage = 0;
    } else {
        /* mhitm */
        if (magr->mcan) {
            mhm->damage = 0;
            return;
        }
        if (resists_acid(mdef) || defended(mdef, AD_ACID)) {
            if (gv.vis && canseemon(mdef))
                pline("%s is covered in %s, but it seems harmless.",
                      Monnam(mdef), hliquid("acid"));
            mhm->damage = 0;
        } else if (gv.vis && canseemon(mdef)) {
            pline_mon(mdef, "%s is covered in %s!", Monnam(mdef), hliquid("acid"));
            pline("It burns %s!", mon_nam(mdef));
        }
        if (!rn2(30))
            erode_armor(mdef, ERODE_CORRODE);
        if (!rn2(6)) {
            acid_damage(MON_WEP(mdef));
            acid_damage(MON_WEP2(mdef));
        }
    }
}

/* steal gold */
void
mhitm_ad_sgld(
    struct monst *magr, struct attack *mattk,
    struct monst *mdef, struct mhitm_data *mhm)
{
    struct permonst *pa = magr->data;
    struct permonst *pd = mdef->data;

    if (magr == &gy.youmonst) {
        /* uhitm */
        struct obj *mongold = findgold(mdef->minvent);

        if (mongold) {
            obj_extract_self(mongold);
            if (merge_choice(gi.invent, mongold)
                    || inv_cnt(FALSE) < invlet_basic) {
                addinv(mongold);
                Your("purse feels heavier.");
            } else {
                You("grab %s's gold, but find no room in your knapsack.",
                    mon_nam(mdef));
                dropy(mongold);
            }
        }
        exercise(A_DEX, TRUE);
        mhm->damage = 0;
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        hitmsg(magr, mattk);
        if (pd->mlet == pa->mlet)
            return;
        if (!(magr->mcan || Hidinshell))
            stealgold(magr);
    } else {
        /* mhitm */
        char buf[BUFSZ];

        mhm->damage = 0;
        if (magr->mcan)
            return;
        /* technically incorrect; no check for stealing gold from
         * between mdef's feet...
         */
        {
            struct obj *gold = findgold(mdef->minvent);

            if (!gold)
                return;
            obj_extract_self(gold);
            add_to_minv(magr, gold);
        }
        mdef->mstrategy &= ~STRAT_WAITFORU;
        Strcpy(buf, Monnam(magr));
        if (gv.vis && canseemon(mdef)) {
            pline("%s steals some gold from %s.", buf, mon_nam(mdef));
        }
        if (!tele_restrict(magr)) {
            boolean couldspot = canspotmon(magr);

            mhm->hitflags = M_ATTK_AGR_DONE;
            (void) rloc(magr, RLOC_NOMSG);
            /* TODO: use RLOC_MSG instead? */
            if (gv.vis && couldspot && !canspotmon(magr))
                pline("%s suddenly disappears!", buf);
        }
    }
}

void
mhitm_ad_tlpt(
    struct monst *magr, struct attack *mattk,
    struct monst *mdef, struct mhitm_data *mhm)
{
    if (magr == &gy.youmonst) {
        /* uhitm */
        if (mhm->damage <= 0)
            mhm->damage = 1;
        if (mhitm_mgc_atk_negated(magr, mdef, TRUE)) {
            pline("%s is not affected.", Monnam(mdef));
        } else {
            char nambuf[BUFSZ];
            boolean u_saw_mon = (canseemon(mdef) || engulfing_u(mdef));

            /* record the name before losing sight of monster */
            Strcpy(nambuf, Monnam(mdef));
            if (u_teleport_mon(mdef, FALSE) && u_saw_mon
                && !(canseemon(mdef) || engulfing_u(mdef)))
                pline("%s suddenly disappears!", nambuf);
            if (mhm->damage >= mdef->mhp) { /* see hitmu(mhitu.c) */
                if (mdef->mhp == 1)
                    ++mdef->mhp;
                mhm->damage = mdef->mhp - 1;
            }
        }
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        int tmphp;

        hitmsg(magr, mattk);
        if (mhitm_mgc_atk_negated(magr, mdef, FALSE)) {
            You("are not affected.");
        } else {
            if (flags.verbose)
                Your("position suddenly seems %suncertain!",
                     (Teleport_control && !Stunned && !unconscious()) ? ""
                     : "very ");
            tele();
            /* As of 3.6.2:  make sure damage isn't fatal; previously, it
               was possible to be teleported and then drop dead at
               the destination when QM's 1d4 damage gets applied below;
               even though that wasn't "wrong", it seemed strange,
               particularly if the teleportation had been controlled
               [applying the damage first and not teleporting if fatal
               is another alternative but it has its own complications] */
            if ((Half_physical_damage ? (mhm->damage - 1) / 2 : mhm->damage)
                >= (tmphp = (Upolyd ? u.mh : u.uhp))) {
                mhm->damage = tmphp - 1;
                if (Half_physical_damage)
                    mhm->damage *= 2; /* doesn't actually increase damage;
                                       * we only get here if half the
                                       * original damage would have
                                       * been fatal, so double reduced
                                       * damage will be less than original */
                if (mhm->damage < 1) { /* implies (tmphp <= 1) */
                    mhm->damage = 1;
                    /* this might increase current HP beyond maximum HP but it
                       will be immediately reduced by caller, so that should
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
    } else {
        /* mhitm */
        if (magr->mcan || mhm->damage >= mdef->mhp || tele_restrict(mdef)) {
            ; /* no negation message */
        } else if (mhitm_mgc_atk_negated(magr, mdef, TRUE)) {
            if (gv.vis)
                pline_mon(mdef, "%s is not affected.", Monnam(mdef));
        } else {
            char mdef_Monnam[BUFSZ];
            boolean wasseen = canspotmon(mdef);

            /* save the name before monster teleports, otherwise
               we'll get "it" in the suddenly disappears message */
            if (gv.vis && wasseen)
                Strcpy(mdef_Monnam, Monnam(mdef));
            mdef->mstrategy &= ~STRAT_WAITFORU;
            (void) rloc(mdef, RLOC_NOMSG);
            /* TODO: use RLOC_MSG instead? */
            if (gv.vis && wasseen && !canspotmon(mdef) && mdef != u.usteed)
                pline("%s suddenly disappears!", mdef_Monnam);
            if (mhm->damage >= mdef->mhp) { /* see hitmu(mhitu.c) */
                if (mdef->mhp == 1)
                    ++mdef->mhp;
                mhm->damage = mdef->mhp - 1;
            }
        }
    }
}

void
mhitm_ad_blnd(
    struct monst *magr,     /* attacker */
    struct attack *mattk,   /* magr's attack */
    struct monst *mdef,     /* defender */
    struct mhitm_data *mhm) /* optional for monster vs monster */
{
    if (magr == &gy.youmonst) {
        /* uhitm */
        if (can_blnd(magr, mdef, mattk->aatyp, (struct obj *) 0)) {
            if (!Blind && mdef->mcansee)
                pline("%s is blinded.", Monnam(mdef));
            mdef->mcansee = 0;
            mhm->damage += mdef->mblinded;
            if (mhm->damage > 127)
                mhm->damage = 127;
            mdef->mblinded = mhm->damage;
        }
        mhm->damage = 0;
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        if (can_blnd(magr, mdef, mattk->aatyp, (struct obj *) 0)) {
            if (!Blind)
                pline("%s blinds you!", Monnam(magr));
            make_blinded(BlindedTimeout + (long) mhm->damage, FALSE);
            if (!Blind) /* => Eyes of the Overworld */
                Your1(vision_clears);
        }
        mhm->damage = 0;
    } else {
        /* mhitm */
        if (can_blnd(magr, mdef, mattk->aatyp, (struct obj *) 0)) {
            char buf[BUFSZ];
            unsigned rnd_tmp;

            if (gv.vis && mdef->mcansee && canspotmon(mdef)) {
                /* feedback for becoming blinded is given if observed
                   telepathically (canspotmon suffices) but additional
                   info about archon's glow is only given if seen */
                Snprintf(buf, sizeof buf, "%s is blinded", Monnam(mdef));
                if (mdef->data == &mons[PM_ARCHON] && canseemon(mdef))
                    Snprintf(eos(buf), sizeof buf - strlen(buf),
                             " by %s radiance", s_suffix(mon_nam(magr)));
                pline("%s.", buf);
            }
            rnd_tmp = d((int) mattk->damn, (int) mattk->damd);
            if ((rnd_tmp += mdef->mblinded) > 127)
                rnd_tmp = 127;
            mdef->mblinded = rnd_tmp;
            mdef->mcansee = 0;
            mdef->mstrategy &= ~STRAT_WAITFORU;
        }
        if (mhm) {
            /* light-haters can take damage from the intense light
               (yellow light explosion), blind or not */
            if (mattk->aatyp == AT_EXPL && hates_light(r_data(mdef))) {
                if (!Deaf)
                    pline("%s cries out in pain!", Monnam(mdef));
                mhm->damage = rnd(5);
            } else {
                mhm->damage = 0;
            }
        }
    }
}

void
mhitm_ad_curs(
    struct monst *magr, struct attack *mattk,
    struct monst *mdef, struct mhitm_data *mhm)
{
    struct permonst *pa = magr->data;
    struct permonst *pd = mdef->data;

    if (magr == &gy.youmonst) {
        /* uhitm */
        if (night() && !rn2(10) && !mdef->mcan) {
            if (pd == &mons[PM_CLAY_GOLEM]) {
                if (!Blind)
                    pline("Some writing vanishes from %s head!",
                          s_suffix(mon_nam(mdef)));
                xkilled(mdef, XKILL_NOMSG);
                /* Don't return yet; keep hp<1 and mhm.damage=0 for pet msg */
            } else {
                mdef->mcan = 1;
                You("chuckle.");
            }
        }
        mhm->damage = 0;
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        hitmsg(magr, mattk);
        if (!night() && pa == &mons[PM_GREMLIN])
            return;
        if (!magr->mcan && !rn2(10)) {
            if (!Deaf) {
                Soundeffect(se_laughter, 40);
                if (Blind) {
                    You_hear("laughter.");
                } else {
                    pline_mon(magr, "%s chuckles.", Monnam(magr));
                }
            }
            if (u.umonnum == PM_CLAY_GOLEM) {
                pline("Some writing vanishes from your head!");
                /* KMH -- this is okay with unchanging */
                rehumanize();
                return;
            }
            mon_give_prop(magr, attrcurse());
        }
    } else {
        /* mhitm */
        if (!night() && (pa == &mons[PM_GREMLIN]))
            return;
        if (!magr->mcan && !rn2(10)) {
            mdef->mcan = 1; /* cancelled regardless of lifesave */
            mdef->mstrategy &= ~STRAT_WAITFORU;
            if (is_were(pd) && pd->mlet != S_HUMAN)
                were_change(mdef);
            if (pd == &mons[PM_CLAY_GOLEM]) {
                if (gv.vis && canseemon(mdef)) {
                    pline("Some writing vanishes from %s head!",
                          s_suffix(mon_nam(mdef)));
                    pline_mon(mdef, "%s is destroyed!", Monnam(mdef));
                }
                mondied(mdef);
                if (!DEADMONSTER(mdef)) {
                    mhm->hitflags = M_ATTK_MISS;
                    mhm->done = TRUE;
                    return;
                } else if (mdef->mtame && !gv.vis) {
                    You(brief_feeling, "strangely sad");
                }
                mhm->hitflags = (M_ATTK_DEF_DIED
                                 | (grow_up(magr, mdef) ? 0
                                    : M_ATTK_AGR_DIED));
                mhm->done = TRUE;
                return;
            }
            if (!Deaf) {
                if (!gv.vis)
                    You_hear("laughter.");
                else if (canseemon(magr))
                    pline_mon(magr, "%s chuckles.", Monnam(magr));
            }
        }
    }
}

/* Helper for mhitm_ad_drst(), containing some code that is also called from
 * mhitm_ad_phys (for poisoned weapons) and shouldn't be subject to magic
 * cancellation or a 1/8 chance roll.
 * In this specific case, the "mhitm" in the name ACTUALLY means just that -
 * this should be called only for monster versus monster situations. */
staticfn void
mhitm_really_poison(struct monst *magr, struct attack *mattk,
                    struct monst *mdef, struct mhitm_data *mhm)
{
    if (gv.vis && canspotmon(magr))
        pline("%s %s was poisoned!", s_suffix(Monnam(magr)),
              mpoisons_subj(magr, mattk));
    if (resists_poison(mdef)) {
        if (gv.vis && canspotmon(mdef) && canspotmon(magr))
            pline_The("poison doesn't seem to affect %s.",
                        mon_nam(mdef));
    } else {
        mhm->damage += rn1(10, 6);
        if (mhm->damage >= mdef->mhp && gv.vis && canspotmon(mdef))
            pline_The("poison was deadly...");
    }
}

void
mhitm_ad_drst(
    struct monst *magr, struct attack *mattk,
    struct monst *mdef, struct mhitm_data *mhm)
{
    boolean negated = mhitm_mgc_atk_negated(magr, mdef, FALSE);
    struct permonst *pa = magr->data;

    if (magr == &gy.youmonst) {
        /* uhitm */
        if (!negated && !rn2(8)) {
            Your("%s was poisoned!", mpoisons_subj(magr, mattk));
            if (resists_poison(mdef)) {
                pline_The("poison doesn't seem to affect %s.", mon_nam(mdef));
            } else {
                if (!rn2(10)) {
                    Your("poison was deadly...");
                    mhm->damage = mdef->mhp;
                } else
                    mhm->damage += rn1(10, 6);
            }
        }
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        int ptmp = A_STR;  /* A_STR == 0 */
        char buf[BUFSZ];

        switch (mattk->adtyp) {
        case AD_DRST: ptmp = A_STR; break;
        case AD_DRDX: ptmp = A_DEX; break;
        case AD_DRCO: ptmp = A_CON; break;
        }
        hitmsg(magr, mattk);
        if (!negated && !rn2(8)) {
            Sprintf(buf, "%s %s", s_suffix(Monnam(magr)),
                    mpoisons_subj(magr, mattk));
            poisoned(buf, ptmp, pmname(pa, Mgender(magr)), 30, FALSE);
        }
    } else {
        /* mhitm */
        if (!negated && !rn2(8)) {
            mhitm_really_poison(magr, mattk, mdef, mhm);
        }
    }
}

void
mhitm_ad_drin(
    struct monst *magr, struct attack *mattk,
    struct monst *mdef, struct mhitm_data *mhm)
{
    struct permonst *pd = mdef->data;
    struct obj *amu;
    boolean lifsav;

    /*
     * Mind flayers have multiple AD_DRIN attacks (3 for plain mind flayer,
     * 5 for master mind flayer).  If one of those kills the target, skip
     * the others (for rest of attacker's current move).  To check whether
     * hero has been killed, we check mortality counter.  For a monster,
     * we check whether it was wearing an amulet of life-saving before the
     * attack and no longer wearing any amulet after the attack.
     */

    if (magr == &gy.youmonst) {
        /* uhitm */
        struct obj *helmet;
        struct obj *barding;

        /* a zombie hero's bite infects rather than eats brains */
        if (((!Upolyd && Race_if(PM_DRAUGR)) || is_zombie(gy.youmonst.data))
            && rn2(5)) {
            if (!(resists_sick(mdef) || defended(mdef, AD_DISE))) {
                if (mdef->msicktime)
                    mdef->msicktime -= rnd(3);
                else
                    mdef->msicktime = rn1(9, 6);
                if (canseemon(mdef))
                    pline_mon(mdef, "%s looks %s.", Monnam(mdef),
                              mdef->msick ? "much worse" : "rather ill");
                mdef->msick = can_become_zombie(r_data(mdef)) ? 3 : 1;
                mdef->msickbyu = TRUE;
            }
            return; /* plus the normal damage */
        }

        if (gn.notonhead || !has_head(pd)) {
            pline("%s doesn't seem harmed.", Monnam(mdef));
            /* hero should skip remaining AT_TENT+AD_DRIN attacks
               because they'll be just as harmless as this one (and also
               to reduce verbosity) */
            gs.skipdrin = TRUE;
            mhm->damage = 0;
            if (!Unchanging && pd == &mons[PM_GREEN_SLIME]) {
                if (!Slimed) {
                    You("suck in some slime and don't feel very well.");
                    make_slimed(10L, (char *) 0);
                }
            }
            return;
        }
        if (m_slips_free(mdef, mattk))
            return;

        if ((helmet = which_armor(mdef, W_ARMH)) != 0 && rn2(8)) {
            pline("%s %s blocks your attack to %s head.",
                  s_suffix(Monnam(mdef)), helm_simple_name(helmet),
                  mhis(mdef));
            return;
        }
        if ((barding = which_armor(mdef, W_BARDING)) != 0 && rn2(8)) {
            pline("%s barding blocks your attack to %s head.",
                  s_suffix(Monnam(mdef)), mhis(mdef));
            return;
        }
        amu = which_armor(mdef, W_AMUL);
        lifsav = amu && amu->otyp == AMULET_OF_LIFE_SAVING;

        (void) eat_brains(&gy.youmonst, mdef, TRUE, &mhm->damage);

        /* skip further AD_DRIN if amulet of life-saving got used up */
        if (lifsav && !which_armor(mdef, W_AMUL))
            gs.skipdrin = TRUE;

    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        hitmsg(magr, mattk);
        if (defends(AD_DRIN, uwep) || !has_head(pd)) {
            You("don't seem harmed.");
            /* attacker should skip remaining AT_TENT+AD_DRIN attacks */
            gs.skipdrin = TRUE;
            /* Not clear what to do for green slimes */
            return;
        }
        if (u_slip_free(magr, mattk))
            return;

        /* the material of the helmet on your head determines how
           effective it will be when deflecting tentacle/bite attacks;
           harder material will do a better job than a soft cap */
        if (uarmh && is_hard(uarmh) && rn2(4)) {
            /* not body_part(HEAD) */
            Your("%s blocks the %s to your head.",
                 helm_simple_name(uarmh),
                 racial_zombie(magr) ? "bite" : "attack");
            return;
        }
        if (uarmh && !is_hard(uarmh) && rn2(2)) {
            Your("%s repels the %s to your head.",
                 helm_simple_name(uarmh),
                 racial_zombie(magr) ? "bite" : "attack");
            return;
        }
        if (maybe_polyd(is_illithid(gy.youmonst.data),
                        Race_if(PM_ILLITHID))
            && !racial_zombie(magr)) {
            Your("psionic abilities shield your brain.");
            return;
        }
        /* a zombie's bite infects rather than eats brains */
        if (racial_zombie(magr) && rn2(6)) {
            if (!mhitm_mgc_atk_negated(magr, mdef, FALSE)) {
                if (!Upolyd && Race_if(PM_DRAUGR))
                    pline_mon(magr, "%s tries to eat your brains, but can't.",
                              Monnam(magr));
                else
                    pline_mon(magr, "%s eats your brains!", Monnam(magr));
                (void) diseasemu(magr);
            }
            return;
        }
        /* negative armor class doesn't reduce this damage */
        if (Half_physical_damage)
            mhm->damage = (mhm->damage + 1) / 2;
        mdamageu(magr, mhm->damage);
        mhm->damage = 0; /* don't inflict a second dose below */

        if (!uarmh || uarmh->otyp != DUNCE_CAP) {
            int oldmort = u.umortality,
                mhitu = eat_brains(magr, mdef, TRUE, (int *) 0);

            /* skip further AD_DRIN if hero's number of deaths went up */
            if (u.umortality > oldmort)
                gs.skipdrin = TRUE;
            /* eat_brains() will miss if target is mindless (won't
               happen here--hero is considered to retain his mind
               regardless of current shape) or is noncorporeal
               (can't happen here--no one can poly into a ghost
               or shade) so this check for missing is academic */
            if (mhitu == M_ATTK_MISS)
                return;
        }
        if (maybe_polyd(is_illithid(gy.youmonst.data),
                        Race_if(PM_ILLITHID))) {
            if (!rn2(3))
                Your("psionic abilities shield your brain from memory loss.");
            return;
        }
        /* adjattrib gives dunce cap message when appropriate */
        (void) adjattrib(A_INT, -rnd(2), FALSE);
        /* mindless zombies chew on brains but don't steal memories */
        if (!racial_zombie(magr)) {
            if (!rn2(5)) {
                losespells();
                gs.skipdrin = TRUE;
            }
            if (!rn2(5)) {
                drain_weapon_skill(rnd(2));
                gs.skipdrin = TRUE;
            }
        }
    } else {
        /* mhitm */
        char buf[BUFSZ];

        if (gn.notonhead || !has_head(pd)) {
            if (gv.vis && canspotmon(mdef))
                pline_mon(mdef, "%s doesn't seem harmed.", Monnam(mdef));
            /* Not clear what to do for green slimes */
            mhm->damage = 0;
            /* don't bother with additional DRIN attacks since they wouldn't
               be able to hit target on head either */
            gs.skipdrin = TRUE; /* affects mattackm()'s attack loop */
            return;
        }
        if ((mdef->misc_worn_check & (W_ARMH | W_BARDING)) && rn2(8)) {
            if (gv.vis && canspotmon(magr) && canseemon(mdef)) {
                Strcpy(buf, s_suffix(Monnam(mdef)));
                pline("%s %s blocks %s attack to %s head.", buf,
                      which_armor(mdef, W_ARMH) ? "helmet" : "barding",
                      s_suffix(mon_nam(magr)), mhis(mdef));
            }
            return;
        }
        /* a zombie's bite infects rather than eats brains */
        if (racial_zombie(magr) && rn2(5)
            && !(resists_sick(mdef) || defended(mdef, AD_DISE))) {
            if (gv.vis && canspotmon(mdef))
                pline_mon(mdef, "%s looks %s.", Monnam(mdef),
                          mdef->msick ? "much worse" : "rather ill");
            if (mdef->msicktime)
                mdef->msicktime -= rnd(3);
            else
                mdef->msicktime = rn1(9, 6);
            mdef->msick = can_become_zombie(r_data(mdef)) ? 3 : 1;
            mdef->msickbyu = FALSE;
            return; /* plus the normal damage */
        }
        amu = which_armor(mdef, W_AMUL);
        lifsav = amu && amu->otyp == AMULET_OF_LIFE_SAVING;

        mhm->hitflags = eat_brains(magr, mdef, gv.vis, &mhm->damage);

        /* skip further AD_DRIN if amulet of life-saving got used up */
        if (lifsav && !which_armor(mdef, W_AMUL))
            gs.skipdrin = TRUE;
    }
}

void
mhitm_ad_stck(
    struct monst *magr, struct attack *mattk,
    struct monst *mdef, struct mhitm_data *mhm)
{
    boolean negated = mhitm_mgc_atk_negated(magr, mdef, FALSE);
    struct permonst *pd = mdef->data;
    boolean barbs = (magr->data == &mons[PM_BARBED_DEVIL]);

    if (magr == &gy.youmonst) {
        /* uhitm */
        if (!negated && !sticks(pd) && m_next2u(mdef)) {
            set_ustuck(mdef); /* it's now stuck to you */
            if (barbs)
                Your("barbs stick to %s!", y_monnam(mdef));
        }
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        hitmsg(magr, mattk);
        if (!negated && !u.ustuck && !sticks(pd)) {
            set_ustuck(magr);
            if (barbs)
                pline("The barbs stick to you!");
        }
    } else {
        /* mhitm */
        if (negated)
            mhm->damage = 0;
    }
}

void
mhitm_ad_wrap(
    struct monst *magr, struct attack *mattk,
    struct monst *mdef, struct mhitm_data *mhm)
{
    struct permonst *pd = mdef->data, *pa = magr->data;
    boolean coil = slithy(pa) && (pa->mlet == S_SNAKE || pa->mlet == S_NAGA);

    if (magr == &gy.youmonst) {
        /* uhitm */
        if (!sticks(pd)) {
            boolean tailmiss = !gn.notonhead;

            if (!u.ustuck && !tailmiss && !rn2(10)) {
                if (m_slips_free(mdef, mattk)) {
                    mhm->damage = 0;
                } else {
                    You("%s yourself around %s!",
                        coil ? "coil" : "swing", mon_nam(mdef));
                    set_ustuck(mdef);
                }
            } else if (u.ustuck == mdef && !tailmiss) {
                /* Monsters don't wear amulets of magical breathing */
                if (is_pool(u.ux, u.uy) && !cant_drown(pd)) {
                    You("drown %s...", mon_nam(mdef));
                    mhm->damage = mdef->mhp;
                } else if (mattk->aatyp == AT_HUGS)
                    pline("%s is being crushed.", Monnam(mdef));
            } else {
                mhm->damage = 0;
                if (flags.verbose) {
                    if (coil && !tailmiss)
                        You("brush against %s.", mon_nam(mdef));
                    else
                        You("brush against %s %s.", s_suffix(mon_nam(mdef)),
                            tailmiss ? "tail" : mbodypart(mdef, LEG));
                }
            }
        } else
            mhm->damage = 0;
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        if ((!magr->mcan || u.ustuck == magr) && !sticks(pd)) {
            if (!u.ustuck && !rn2(10)) {
                if (u_slip_free(magr, mattk)) {
                    mhm->damage = 0;
                } else {
                    set_ustuck(magr); /* before message, for botl update */
                    urgent_pline("%s %s itself around you!",
                                 Some_Monnam(magr),
                                 coil ? "coils" : "swings");
                }
            } else if (u.ustuck == magr) {
                if (is_pool(magr->mx, magr->my) && !Swimming && !Amphibious
                    && !Breathless) {
                    boolean moat = (levl[magr->mx][magr->my].typ != POOL)
                                   && !is_waterwall(magr->mx, magr->my)
                                   && !Is_medusa_level(&u.uz)
                                   && !Is_waterlevel(&u.uz);

                    urgent_pline("%s drowns you...", Monnam(magr));
                    svk.killer.format = KILLED_BY_AN;
                    Sprintf(svk.killer.name, "%s by %s",
                            moat ? "moat" : "pool of water",
                            an(pmname(magr->data, Mgender(magr))));
                    done(DROWNING);
                } else if (mattk->aatyp == AT_HUGS) {
                    You("are being crushed.");
                }
            } else {
                mhm->damage = 0;
                if (flags.verbose) {
                    if (coil)
                        pline_mon(magr, "%s brushes against you.",
                                  Monnam(magr));
                    else
                        pline_mon(magr, "%s brushes against your %s.",
                                  Monnam(magr), body_part(LEG));
                }
            }
        } else
            mhm->damage = 0;
    } else {
        /* mhitm */
        if (magr->mcan)
            mhm->damage = 0;

        if (!mhm->damage && (canseemon(magr) || canseemon(mdef))) {
            pline("%s brushes against %s.",
                  Some_Monnam(magr), some_mon_nam(mdef));
        }
    }
}

void
mhitm_ad_plys(
    struct monst *magr, struct attack *mattk,
    struct monst *mdef, struct mhitm_data *mhm)
{
    if (magr == &gy.youmonst) {
        /* uhitm */
        if (!rn2(3) && mhm->damage < mdef->mhp
            && !mhitm_mgc_atk_negated(magr, mdef, TRUE)) {
            if (!Blind)
                pline("%s is frozen by you!", Monnam(mdef));
            paralyze_monst(mdef, rnd(10));
        }
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        hitmsg(magr, mattk);
        if (gm.multi >= 0 && !rn2(3)
            && !mhitm_mgc_atk_negated(magr, mdef, TRUE)) {
            if (Free_action) {
                You("momentarily stiffen.");
            } else {
                if (Blind)
                    You("are frozen!");
                else
                    You("are frozen by %s!", mon_nam(magr));
                gn.nomovemsg = You_can_move_again;
                nomul(-rnd(10));
                /* set gm.multi_reason;
                   3.6.x used "paralyzed by a monster"; be more specific */
                dynamic_multi_reason(magr, "paralyzed", FALSE);
                exercise(A_DEX, FALSE);
            }
        }
    } else {
        /* mhitm */
        if (mdef->mcanmove && !rn2(3)
            && !mhitm_mgc_atk_negated(magr, mdef, TRUE)) {
            if (gv.vis && canspotmon(mdef)) {
                char buf[BUFSZ];

                Strcpy(buf, Monnam(mdef));
                pline("%s is frozen by %s.", buf, mon_nam(magr));
            }
            paralyze_monst(mdef, rnd(10));
        }
    }
}

void
mhitm_ad_slee(
    struct monst *magr, struct attack *mattk,
    struct monst *mdef,
    struct mhitm_data *mhm UNUSED)
{
    if (magr == &gy.youmonst) {
        /* uhitm */
        if (!mdef->msleeping && !mhitm_mgc_atk_negated(magr, mdef, FALSE)
            && sleep_monst(mdef, rnd(10), -1)) {
            if (!Blind)
                pline("%s is put to sleep by you!", Monnam(mdef));
            slept_monst(mdef);
        }
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        hitmsg(magr, mattk);
        if (gm.multi >= 0 && !rn2(5)
            && !mhitm_mgc_atk_negated(magr, mdef, TRUE)) {
            if (how_resistant(SLEEP_RES) == 100) {
                monstseesu(M_SEEN_SLEEP);
                return;
            }
            monstunseesu(M_SEEN_SLEEP);
            fall_asleep(-resist_reduce(rnd(10), SLEEP_RES), TRUE);
            if (Blind)
                You("are put to sleep!");
            else
                You("are put to sleep by %s!", mon_nam(magr));
        }
    } else {
        /* mhitm */
        if (!mdef->msleeping && sleep_monst(mdef, rnd(10), -1)
            && sleep_monst(mdef, rnd(10), -1)) {
            if (gv.vis && canspotmon(mdef)) {
                char buf[BUFSZ];

                Strcpy(buf, Monnam(mdef));
                pline("%s is put to sleep by %s.", buf, mon_nam(magr));
            }
            mdef->mstrategy &= ~STRAT_WAITFORU;
            slept_monst(mdef);
        }
    }
}

/* slime */
void
mhitm_ad_slim(
    struct monst *magr, struct attack *mattk,
    struct monst *mdef, struct mhitm_data *mhm)
{
    boolean negated = mhitm_mgc_atk_negated(magr, mdef, FALSE);
    struct permonst *pd = mdef->data;

    if (magr == &gy.youmonst) {
        /* uhitm */
        if (negated)
            return; /* physical damage only */
        if (!rn2(4) && !slimeproof(pd)) {
            if (!munslime(mdef, TRUE) && !DEADMONSTER(mdef)) {
                /* this assumes newcham() won't fail; since hero has
                   a slime attack, green slimes haven't been geno'd */
                You("turn %s into slime.", mon_nam(mdef));
                if (newcham(mdef, &mons[PM_GREEN_SLIME], NO_NC_FLAGS))
                    pd = mdef->data;
            }
            /* munslime attempt could have been fatal */
            if (DEADMONSTER(mdef)) {
                mhm->hitflags = M_ATTK_DEF_DIED; /* skip death message */
                mhm->done = TRUE;
                return;
            }
            mhm->damage = 0;
        }
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        hitmsg(magr, mattk);
        if (negated) {
            if (!magr->mcan)
                You("escape harm.");
            return;
        }
        if (flaming(pd)) {
            pline_The("slime burns away!");
            mhm->damage = 0;
        } else if (Unchanging || noncorporeal(pd)
                   || pd == &mons[PM_GREEN_SLIME]) {
            You("are unaffected.");
            mhm->damage = 0;
        } else if (!Slimed) {
            You("don't feel very well.");
            make_slimed(10L, (char *) 0);
            delayed_killer(SLIMED, KILLED_BY_AN,
                           pmname(magr->data, Mgender(magr)));
        } else
            pline("Yuck!");
    } else {
        /* mhitm */
        if (negated)
            return; /* physical damage only */
        if (!rn2(4) && !slimeproof(pd)) {
            if (!munslime(mdef, FALSE) && !DEADMONSTER(mdef)) {
                unsigned ncflags = NO_NC_FLAGS;

                if (gv.vis && canseemon(mdef))
                    ncflags |= NC_SHOW_MSG;
                if (newcham(mdef, &mons[PM_GREEN_SLIME], ncflags))
                    pd = mdef->data;
                mdef->mstrategy &= ~STRAT_WAITFORU;
                mhm->hitflags = M_ATTK_HIT;
            }
            /* munslime attempt could have been fatal,
               potentially to multiple monsters (SCR_FIRE) */
            if (DEADMONSTER(magr))
                mhm->hitflags |= M_ATTK_AGR_DIED;
            if (DEADMONSTER(mdef))
                mhm->hitflags |= M_ATTK_DEF_DIED;
            mhm->damage = 0;
        }
    }
    nhUse(pd);
}

void
mhitm_ad_ench(
    struct monst *magr, struct attack *mattk,
    struct monst *mdef,
    struct mhitm_data *mhm UNUSED)
{
    if (magr == &gy.youmonst) {
        /* uhitm */
        /* there's no msomearmor() function, so just do damage */
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        boolean negated = mhitm_mgc_atk_negated(magr, mdef, FALSE);

        hitmsg(magr, mattk);
        /* uncancelled is sufficient enough; please
           don't make this attack less frequent */
        if (!negated) {
            struct obj *obj = some_armor(mdef);

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
            if (obj && drain_item(obj, FALSE)) {
                pline("%s less effective.", Yobjnam2(obj, "seem"));
            }
        }
    } else {
        /* mhitm */
        /* there's no msomearmor() function, so just do damage */
    }
}

void
mhitm_ad_slow(
    struct monst *magr, struct attack *mattk,
    struct monst *mdef,
    struct mhitm_data *mhm UNUSED)
{
    boolean negated = mhitm_mgc_atk_negated(magr, mdef, FALSE);

    if (defended(mdef, AD_SLOW))
        return;

    if (magr == &gy.youmonst) {
        /* uhitm */
        if (!negated && mdef->mspeed != MSLOW) {
            unsigned int oldspeed = mdef->mspeed;

            mon_adjust_speed(mdef, -1, (struct obj *) 0);
            if (mdef->mspeed != oldspeed && canseemon(mdef))
                pline("%s slows down.", Monnam(mdef));
        }
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        hitmsg(magr, mattk);
        if (!negated && HFast && !rn2(4))
            u_slow_down();
    } else {
        /* mhitm */
        if (!negated && mdef->mspeed != MSLOW) {
            unsigned int oldspeed = mdef->mspeed;

            mon_adjust_speed(mdef, -1, (struct obj *) 0);
            mdef->mstrategy &= ~STRAT_WAITFORU;
            if (mdef->mspeed != oldspeed && gv.vis && canspotmon(mdef))
                pline_mon(mdef, "%s slows down.", Monnam(mdef));
        }
    }
}

void
mhitm_ad_conf(
    struct monst *magr, struct attack *mattk,
    struct monst *mdef, struct mhitm_data *mhm)
{
    if (magr == &gy.youmonst) {
        /* uhitm */
        if (!mdef->mconf) {
            if (canseemon(mdef))
                pline("%s looks confused.", Monnam(mdef));
            mdef->mconf = 1;
        }
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        hitmsg(magr, mattk);
        if (!magr->mcan && !rn2(4) && !magr->mspec_used) {
            magr->mspec_used = magr->mspec_used + (mhm->damage + rn2(6));
            if (Confusion)
                You("are getting even more confused.");
            else
                You("are getting confused.");
            make_confused(HConfusion + mhm->damage, FALSE);
        }
        mhm->damage = 0;
    } else {
        /* mhitm */
        /* Since confusing another monster doesn't have a real time
         * limit, setting spec_used would not really be right (though
         * we still should check for it).
         */
        if (!magr->mcan && !mdef->mconf && !magr->mspec_used) {
            if (gv.vis && canseemon(mdef))
                pline_mon(mdef, "%s looks confused.", Monnam(mdef));
            mdef->mconf = 1;
            mdef->mstrategy &= ~STRAT_WAITFORU;
        }
    }
}

void
mhitm_ad_poly(
    struct monst *magr, struct attack *mattk,
    struct monst *mdef, struct mhitm_data *mhm)
{
    boolean negated = (mhitm_mgc_atk_negated(magr, mdef, FALSE)
                       || magr->mspec_used);

    if (magr == &gy.youmonst) {
        /* uhitm */
        /* require weaponless attack in order to honor AD_POLY */
        if (!uwep && mhm->damage < mdef->mhp) {
            if (negated) {
                /* assume that you can tell by touch if blinded */
                pline("%s is not transformed.", Monnam(mdef));
            } else {
                mhm->damage = mon_poly(&gy.youmonst, mdef, mhm->damage);
                if (DEADMONSTER(mdef))
                    mhm->hitflags |= M_ATTK_DEF_DIED;
                mhm->hitflags |= M_ATTK_HIT;
                mhm->done = TRUE;
            }
        }
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        hitmsg(magr, mattk);
        if (Maybe_Half_Phys(mhm->damage) < (Upolyd ? u.mh : u.uhp)) {
            if (negated) {
                if (magr->mcan)
                    You("aren't transformed.");
            } else {
                mhm->damage = mon_poly(magr, &gy.youmonst, mhm->damage);
                mhm->hitflags |= M_ATTK_HIT;
                mhm->done = TRUE;
            }
        }
    } else {
        /* mhitm */
        if (mhm->damage < mdef->mhp && !negated) {
            mhm->damage = mon_poly(magr, mdef, mhm->damage);
            if (DEADMONSTER(mdef))
                mhm->hitflags |= M_ATTK_DEF_DIED;
            mhm->hitflags |= M_ATTK_HIT;
            mhm->done = TRUE;
        }
    }
}

void
mhitm_ad_famn(
    struct monst *magr,
    struct attack *mattk UNUSED,
    struct monst *mdef, struct mhitm_data *mhm)
{
    struct permonst *pd = mdef->data;

    if (magr == &gy.youmonst) {
        /* uhitm; hero can't polymorph into anything with this attack
           so this won't happen; if it could, it would be the same as
           the mhitm case except for messaging */
        goto mhitm_famn;
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        pline_mon(magr, "%s reaches out, and your body shrivels.",
                  Monnam(magr));
        exercise(A_CON, FALSE);
        if (!is_fainted())
            morehungry(rn1(40, 40));
        /* plus the normal damage */
    } else {
 mhitm_famn:
        /* mhitm; it's possible for Famine to hit another monster;
           if target is something that doesn't eat, it won't be harmed;
           otherwise, just inflict the normal damage */
        if (!(carnivorous(pd) || herbivorous(pd) || metallivorous(pd)))
            mhm->damage = 0;
    }
}

void
mhitm_ad_pest(
    struct monst *magr, struct attack *mattk UNUSED,
    struct monst *mdef, struct mhitm_data *mhm UNUSED)
{
    if (magr == &gy.youmonst) {
        /* uhitm; hero can't polymorph into anything with this attack
           so this won't happen; if it could, it would be the same as
           the mhitm case except for messaging */
        goto mhitm_pest;
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        pline_mon(magr, "%s reaches out, and you feel fever and chills.",
                  Monnam(magr));
        (void) diseasemu(magr);
        /* plus the normal damage */
    } else {
 mhitm_pest:
        /* mhitm; it's possible for Pestilence to hit another monster;
           halve its hit points and possibly leave it deathly sick */
        if (gv.vis && canseemon(mdef))
            pline_mon(mdef, "%s reaches out, and %s looks %s.",
                      Monnam(magr), mon_nam(mdef),
                      (resists_sick(mdef) || defended(mdef, AD_DISE))
                          ? "unaffected" : "rather ill");
        if (mdef->mhpmax > 3 && !resist(mdef, 0, 0, NOTELL))
            mdef->mhpmax /= 2;
        if (mdef->mhp > 2 && !resist(mdef, 0, 0, NOTELL))
            mdef->mhp /= 2;
        if (mdef->mhp > mdef->mhpmax)
            mdef->mhp = mdef->mhpmax;
        if (!(resists_sick(mdef) || defended(mdef, AD_DISE))) {
            if (mdef->msicktime)
                mdef->msicktime -= rnd(3);
            else
                mdef->msicktime = rn1(9, 6);
            mdef->msick = can_become_zombie(r_data(mdef)) ? 3 : 1;
            mdef->msickbyu = FALSE;
        }
    }
}

void
mhitm_ad_deth(
    struct monst *magr,
    struct attack *mattk,
    struct monst *mdef, struct mhitm_data *mhm)
{
    struct permonst *pd = mdef->data;

    if (magr == &gy.youmonst) {
        /* uhitm; hero can't polymorph into anything with this attack
           so this won't happen; if it could, it would be the same as
           the mhitm case except for messaging */
        goto mhitm_deth;
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        pline_mon(magr, "%s reaches out with its deadly touch.", Monnam(magr));
        if (is_undead(pd)) {
            /* still does some damage */
            mhm->damage = (mhm->damage + 1) / 2;
            pline("Was that the touch of death?");
            return;
        }
        switch (rn2(20)) {
        case 19:
        case 18:
        case 17:
            if (!Antimagic) {
                touch_of_death(magr);
                mhm->damage = 0;
                return;
            }
            FALLTHROUGH;
            /*FALLTHRU*/
        default: /* case 16: ... case 5: */
            You_feel("your life force draining away...");
            mhm->permdmg = 1; /* actual damage done by caller */
            return;
        case 4:
        case 3:
        case 2:
        case 1:
        case 0:
            if (Antimagic)
                shieldeff(u.ux, u.uy);
            pline("Lucky for you, it didn't work!");
            mhm->damage = 0;
            return;
        }
    } else {
 mhitm_deth:
        /* mhitm; it's possible for Death to hit another monster;
           if target is undead, it will take some damage but less than an
           undead hero would; otherwise, just inflict the normal damage */
        if (is_undead(pd) && mhm->damage > 1)
            mhm->damage = rnd(mhm->damage / 2);
        /* simulate Death's touch with drain life attack */
        mhitm_ad_drli(magr, mattk, mdef, mhm);
    }
}

void
mhitm_ad_halu(
    struct monst *magr,
    struct attack *mattk UNUSED,
    struct monst *mdef, struct mhitm_data *mhm)
{
    struct permonst *pd = mdef->data;

    if (magr == &gy.youmonst) {
        /* uhitm */
        mhm->damage = 0;
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        mhm->damage = 0;
    } else {
        /* mhitm */
        if (!magr->mcan && haseyes(pd) && mdef->mcansee
            && !mon_prop(mdef, HALLUC_RES)) {
            if (gv.vis && canseemon(mdef))
                pline_mon(mdef, "%s looks %sconfused.", Monnam(mdef),
                      mdef->mconf ? "more " : "");
            mdef->mconf = 1;
            mdef->mstrategy &= ~STRAT_WAITFORU;
        }
        mhm->damage = 0;
    }
}

boolean
do_stone_u(struct monst *mtmp)
{
    if (!Stoned && !Stone_resistance
        && !(poly_when_stoned(gy.youmonst.data)
             && polymon(PM_STONE_GOLEM))) {
        int kformat = KILLED_BY_AN;
        const char *kname = pmname(mtmp->data, Mgender(mtmp));

        if (mtmp->data->geno & G_UNIQ) {
            if (!type_is_pname(mtmp->data))
                kname = the(kname);
            kformat = KILLED_BY;
        }
        make_stoned(5L, (char *) 0, kformat, kname);
        return 1;
        /* done_in_by(mtmp, STONING); */
    }
    return 0;
}

void
do_stone_mon(
    struct monst *magr,
    struct attack *mattk,
    struct monst *mdef, struct mhitm_data *mhm)
{
    struct permonst *pd = mdef->data;

    /* may die from the acid if it eats a stone-curing corpse */
    if (munstone(mdef, FALSE))
        goto post_stone;
    if (poly_when_stoned(pd)) {
        mon_to_stone(mdef);
        mhm->damage = 0;
        return;
    }
    if (!resists_ston(mdef)) {
        if (gv.vis && canseemon(mdef))
            pline_mon(mdef, "%s turns to stone!", Monnam(mdef));
        /* both callers are on the monster-attacker path, so magr is
           never the hero here */
        if (magr->mtame)
            set_pet_killer(magr);
        monstone(mdef);
 post_stone:
        if (!DEADMONSTER(mdef)) {
            mhm->hitflags = M_ATTK_MISS;
            mhm->done = TRUE;
            return;
        } else if (mdef->mtame && !gv.vis) {
            You(brief_feeling, "peculiarly sad");
        }
        mhm->hitflags = (M_ATTK_DEF_DIED
                         | (grow_up(magr, mdef) ? 0 : M_ATTK_AGR_DIED));
        mhm->done = TRUE;
        return;
    }
    mhm->damage = (mattk->adtyp == AD_STON ? 0 : 1);
}

void
mhitm_ad_phys(
    struct monst *magr, struct attack *mattk,
    struct monst *mdef, struct mhitm_data *mhm)
{
    struct permonst *pa = magr->data;
    struct permonst *pd = mdef->data;

    if (magr == &gy.youmonst) {
        /* uhitm */
        if (pd == &mons[PM_SHADE]) {
            mhm->damage = 0;
            if (!mhm->specialdmg)
                impossible("bad shade attack function flow?");
        }
        mhm->damage += mhm->specialdmg;

        if (mattk->aatyp == AT_WEAP) {
            /* hmonas() uses known_hitum() to deal physical damage,
               then also damageum() for non-AD_PHYS; don't inflict
               extra physical damage for unusual damage types */
            mhm->damage = 0;
        } else if (mattk->aatyp == AT_KICK
                   || mattk->aatyp == AT_CLAW
                   || mattk->aatyp == AT_TUCH
                   || mattk->aatyp == AT_HUGS) {
            if (thick_skinned(pd))
                mhm->damage = (mattk->aatyp == AT_KICK) ? 0
                              : (mhm->damage + 1) / 2;
            /* add ring(s) of increase damage */
            if (u.udaminc > 0) {
                /* applies even if damage was 0 */
                mhm->damage += u.udaminc;
            } else if (mhm->damage > 0) {
                /* ring(s) might be negative; avoid converting
                   0 to non-0 or positive to non-positive */
                mhm->damage += u.udaminc;
                if (mhm->damage < 1)
                    mhm->damage = 1;
            }
        }
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        if (mattk->aatyp == AT_HUGS && !sticks(pd)) {
            if (!u.ustuck && rn2(2)) {
                if (u_slip_free(magr, mattk)) {
                    mhm->damage = 0;
                    mhm->hitflags |= M_ATTK_MISS;
                } else {
                    set_ustuck(magr);
                    pline_mon(magr, "%s grabs you!", Monnam(magr));
                    mhm->hitflags |= M_ATTK_HIT;
                }
            } else if (u.ustuck == magr) {
                if (Hidinshell) {
                    /* monster still has you in its grasp, but is
                       unable to cause any damage */
                    Your("protective shell prevents you from being %s!",
                         (pa == &mons[PM_ROPE_GOLEM]) ? "choked"
                                                      : "crushed");
                    mhm->damage = 0;
                } else {
                    exercise(A_STR, FALSE);
                    You("are being %s.",
                        (pa == &mons[PM_ROPE_GOLEM]) ? "choked"
                                                     : "crushed");
                }
            }
        } else { /* hand to hand weapon */
            struct obj *otmp = MON_WEP(magr);

            if (mattk->aatyp == AT_WEAP && otmp) {
                struct obj *marmg;
                int tmp;
                boolean was_poisoned = (otmp->opoisoned
                                        || permapoisoned(otmp));

                if (otmp->otyp == CORPSE
                    && touch_petrifies(&mons[otmp->corpsenm])) {
                    mhm->damage = 1;
                    pline_mon(magr, "%s hits you with the %s corpse.",
                              Monnam(magr),
                              mons[otmp->corpsenm].pmnames[NEUTRAL]);
                    if (!Stoned) {
                        if (do_stone_u(magr)) {
                            mhm->hitflags = M_ATTK_HIT;
                            mhm->done = 1;
                            return;
                        }
                    }
                }
                mhm->damage += dmgval(otmp, mdef);
                if ((marmg = which_armor(magr, W_ARMG)) != 0
                    && marmg->otyp == GAUNTLETS_OF_POWER)
                    mhm->damage += rn1(4, 3); /* 3..6 */
                if (mhm->damage <= 0)
                    mhm->damage = 1;
                if (!(otmp->oartifact
                      || ((otmp->oclass == WEAPON_CLASS || is_weptool(otmp)
                           || is_bullet(otmp)) && otmp->oprops))
                    || !artifact_hit(magr, mdef, otmp, &mhm->damage,
                                     gm.mhitu_dieroll)) {
                    hitmsg(magr, mattk);
                    mhm->hitflags |= M_ATTK_HIT;
                }
                if (!mhm->damage)
                    return;
                if (Hate_material(otmp->material)) {
                    searmsg(magr, &gy.youmonst, otmp, FALSE);
                    exercise(A_CON, FALSE);
                }
                /* dmgval() adds the extra damage for this; light-hating
                   forms and unpolymorphed drow suffer from a lit
                   artifact's radiance */
                if (artifact_light(otmp) && otmp->lamplit
                    && (hates_light(gy.youmonst.data)
                        || maybe_polyd(is_drow(gy.youmonst.data),
                                       Race_if(PM_DROW)))) {
                    if (canspotmon(magr)) {
                        char *artiname = s_suffix(bare_artifactname(otmp));

                        *artiname = highc(*artiname);
                        pline("%s radiance penetrates deep into your %s!",
                              artiname,
                              !(noncorporeal(gy.youmonst.data)
                                || amorphous(gy.youmonst.data))
                                  ? "flesh" : "form");
                    } else if (!Blind) {
                        pline("The light sears you!");
                    } else {
                        You("are seared!");
                    }
                }
                /* this redundancy necessary because you have
                   to take the damage _before_ being cloned;
                   need to have at least 2 hp left to split */
                tmp = mhm->damage;
                if (u.uac < 0)
                    tmp -= rnd(-u.uac);
                if (tmp < 1)
                    tmp = 1;
                if (Half_physical_damage)
                    tmp = (tmp + 1) / 2;

                if (u.mh - tmp > 1
                    && (otmp->material == IRON
                        /* relevant 'metal' objects are scalpel and tsurugi */
                        || otmp->material == STEEL)
                    && (u.umonnum == PM_BLACK_PUDDING
                        || u.umonnum == PM_BROWN_PUDDING)) {
                    if (tmp > 1)
                        exercise(A_STR, FALSE);
                    /* inflict damage now; we know it can't be fatal */
                    u.mh -= tmp;
                    disp.botl = TRUE;
                    mhm->damage = 0; /* don't inflict more damage below */
                    if (cloneu())
                        You("divide as %s hits you!", mon_nam(magr));
                }
                {
                    unsigned wep_id = otmp->o_id;

                    rustm(&gy.youmonst, otmp);
                    /* erode_obj() can free a weapon that was already at
                       maximum erosion; drop the local and mattacku()'s
                       cached copy so nothing below reads it */
                    if (!o_on(wep_id, magr->minvent)) {
                        mon_currwep_gone(otmp);
                        otmp = (struct obj *) 0;
                    }
                }
                if (was_poisoned && gm.mhitu_dieroll <= 5) {
                    char buf[BUFSZ];

                    /* similar to mhitm_really_poison, but we don't use the
                     * exact same values, nor do we want same 1/8 chance of
                     * poison taking (use 1/4, same as in the mhitm case). */
                    Sprintf(buf, "%s %s", s_suffix(Monnam(magr)),
                            mpoisons_subj(magr, mattk));
                    /* arbitrary, but most poison sources in the game are
                     * strength-based. With hpdamchance = 10, HP damage occurs
                     * 1/2 of the time and it will hit Str rest of the time.
                     * (This is the same as poisoned ammo.) */
                    poisoned(buf, A_STR, pmname(magr->data, Mgender(magr)),
                             10, FALSE);
                }
                /* monster attacking with a tainted (drow-poisoned)
                   weapon */
                if (otmp && otmp->otainted && is_poisonable(otmp)) {
                    int notaint;

                    pline("%s attack was tainted!", s_suffix(Monnam(magr)));

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
                                fall_asleep(-resist_reduce(rn2(3) + 2,
                                                           SLEEP_RES), TRUE);
                            }
                        }
                    }

                    notaint = ((is_drow_weapon(otmp) ? 20 : 5)
                               - otmp->owt / 10);
                    if (notaint < 2)
                        notaint = 2;
                    if (!rn2(notaint)) {
                        otmp->otainted = FALSE;
                        if (canseemon(magr))
                            pline("%s %s is no longer tainted.",
                                  s_suffix(Monnam(magr)), xname(otmp));
                    }
                }
                /* glass breakage from the attack, last so that every
                   effect of the blow has already landed - master runs
                   it after its own poison and taint blocks too, just
                   ahead of the pudding split that 5.0 moved above */
                (void) break_glass_obj(some_armor(&gy.youmonst));
                if (break_glass_obj(otmp)) {
                    /* mattacku() cached this weapon in a file static of
                       its own; clear that or passiveum() drains it */
                    mon_currwep_gone(otmp);
                    otmp = (struct obj *) 0;
                }
            } else if (mattk->aatyp != AT_TUCH || mhm->damage != 0
                       || magr != u.ustuck) {
                hitmsg(magr, mattk);
                mhm->hitflags |= M_ATTK_HIT;
            }
        }
    } else {
        /* mhitm */
        struct obj *mwep = MON_WEP(magr);
        boolean vis = canseemon(magr) && canseemon(mdef);

        if (mattk->aatyp != AT_WEAP && mattk->aatyp != AT_CLAW)
            mwep = 0;

        if (shade_miss(magr, mdef, mwep, FALSE, vis)) {
            mhm->damage = 0;
        } else if (mattk->aatyp == AT_KICK && thick_skinned(pd)) {
            /* [no 'kicking boots' check needed; monsters with kick attacks
               can't wear boots and monsters that wear boots don't kick] */
            mhm->damage = 0;
        } else if (mwep) { /* non-Null 'mwep' implies AT_WEAP || AT_CLAW */
            struct obj *marmg;

            if (mwep->otyp == CORPSE
                && touch_petrifies(&mons[mwep->corpsenm])) {
                do_stone_mon(magr, mattk, mdef, mhm);
                if (mhm->done)
                    return;
            }

            mhm->damage += dmgval(mwep, mdef);
            if ((marmg = which_armor(magr, W_ARMG)) != 0
                && marmg->otyp == GAUNTLETS_OF_POWER)
                mhm->damage += rn1(4, 3); /* 3..6 */
            if (mhm->damage < 1) /* is this necessary?  mhitu.c has it... */
                mhm->damage = 1;
            if (mwep->oartifact
                || ((mwep->oclass == WEAPON_CLASS || is_weptool(mwep)
                     || is_bullet(mwep)) && mwep->oprops)) {
                /* when magr's weapon is an artifact, caller suppressed its
                   usual 'hit' message in case artifact_hit() delivers one;
                   now we'll know and might need to deliver skipped message
                   (note: if there's no message there'll be no auxiliary
                   damage so the message here isn't coming too late) */
                if (!artifact_hit(magr, mdef, mwep, &mhm->damage,
                                  mhm->dieroll)) {
                    if (gv.vis)
                        pline_mon(magr, "%s hits %s.", Monnam(magr),
                              mon_nam_too(mdef, magr));
                    mhm->hitflags |= M_ATTK_HIT;
                }
                /* artifact_hit updates 'tmp' but doesn't inflict any
                   damage; however, it might cause carried items to be
                   destroyed and they might do so */
                if (DEADMONSTER(mdef)) {
                    mhm->hitflags = (M_ATTK_DEF_DIED
                                     | (grow_up(magr, mdef) ? 0
                                        : M_ATTK_AGR_DIED));
                    mhm->done = TRUE;
                    return;
                }
            }
            if (mhm->damage) {
                unsigned wep_id = mwep->o_id;

                rustm(mdef, mwep);
                /* same hazard as the mhitu arm above: the weapon can be
                   destroyed outright, and the poison check below reads it */
                if (!o_on(wep_id, magr->minvent))
                    mwep = (struct obj *) 0;
            }
            if (mwep && (mwep->opoisoned || permapoisoned(mwep)) && !rn2(4)) {
                /* 1/4 chance of weapon poison applying is the same as in
                 * uhitm and mhitu cases. But since we don't need to call
                 * any special functions or go through tangled hmon_hitmon
                 * code, we can just jump straight to the poisoning. */
                mhitm_really_poison(magr, mattk, mdef, mhm);
            }
        } else if (pa == &mons[PM_PURPLE_WORM] && pd == &mons[PM_SHRIEKER]) {
            /* hack to enhance mm_aggression(); we don't want purple
               worm's bite attack to kill a shrieker because then it
               won't swallow the corpse; but if the target survives,
               the subsequent engulf attack should accomplish that */
            if (mhm->damage >= mdef->mhp && mdef->mhp > 1)
                mhm->damage = mdef->mhp - 1;
        }
    }
}

void
mhitm_ad_ston(
    struct monst *magr, struct attack *mattk,
    struct monst *mdef, struct mhitm_data *mhm)
{
    if (magr == &gy.youmonst) {
        /* uhitm */
        if (!munstone(mdef, TRUE))
            minstapetrify(mdef, TRUE);
        mhm->damage = 0;
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        hitmsg(magr, mattk);
        if (!rn2(3)) {
            if (magr->mcan) {
                if (!Deaf)
                    You_hear("a cough from %s!", mon_nam(magr));
            } else {
                if (Hallucination && !Blind) {
                    Soundeffect(se_cockatrice_hiss, 50);
                    You_hear("hissing."); /* You_hear() deals with Deaf */
                    pline("%s appears to be blowing you a kiss...",
                          Monnam(magr));
                } else if (!Deaf) {
                    You_hear("%s hissing!", s_suffix(mon_nam(magr)));
                } else if (!Blind) {
                    pline("%s seems to grimace.", Monnam(magr));
                }
                /*
                 * 5.0:  New moon is no longer overridden by carrying a
                 * lizard corpse.  Having the moon's impact on terrestrial
                 * activity be affected by carrying a dead critter felt
                 * silly.
                 *
                 * That behavior dated to when there were no corpse objects
                 * yet; "dead lizard" was a distinct item.  With a lizard
                 * corpse, hero can eat it to survive petrification and
                 * probably retain a partly eaten corpse for future use.
                 *
                 * Maintaining foodless conduct during a new moon might
                 * become a little harder.  Clearing out cockatrice nests
                 * during a new moon could become quite a bit harder.
                 */
                if (!rn2(10) || flags.moonphase == NEW_MOON) {
                    if (do_stone_u(magr)) {
                        mhm->hitflags = M_ATTK_HIT;
                        mhm->done = TRUE;
                        return;
                    }
                }
            }
        }
    } else {
        /* mhitm */
        if (magr->mcan)
            return;
        do_stone_mon(magr, mattk, mdef, mhm);
        if (mhm->done)
            return;
    }
}

void
mhitm_ad_were(
    struct monst *magr, struct attack *mattk,
    struct monst *mdef, struct mhitm_data *mhm)
{
    struct permonst *pa = magr->data;

    if (magr == &gy.youmonst) {
        /* uhitm */
        mhitm_ad_phys(magr, mattk, mdef, mhm);
        if (mhm->done)
            return;
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        hitmsg(magr, mattk);
        if (!rn2(4) && u.ulycn == NON_PM
            && !Protection_from_shape_changers && !Lycan_resistance
            && !defended(&gy.youmonst, AD_WERE)
            && !mhitm_mgc_atk_negated(magr, mdef, TRUE)) {
            urgent_pline("You feel feverish.");
            exercise(A_CON, FALSE);
            /* the rat king passes ordinary wererat lycanthropy */
            if (pa == &mons[PM_RAT_KING])
                set_ulycn(PM_WERERAT);
            else
                set_ulycn(monsndx(pa));
            retouch_equipment(2);
        }
    } else {
        /* mhitm */
        mhitm_ad_phys(magr, mattk, mdef, mhm);
        if (mhm->done)
            return;
    }
}

void
mhitm_ad_heal(
    struct monst *magr, struct attack *mattk,
    struct monst *mdef, struct mhitm_data *mhm)
{
    struct permonst *pd = mdef->data;

    if (magr == &gy.youmonst) {
        /* uhitm */
        mhitm_ad_phys(magr, mattk, mdef, mhm);
        if (mhm->done)
            return;
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        /* a cancelled nurse is just an ordinary monster,
         * nurses don't heal those that cause petrification,
         * nor will they heal the undead */
        if (magr->mcan || (Upolyd && touch_petrifies(pd))
            || is_undead(gy.youmonst.data) || Race_if(PM_DRAUGR)
            || Race_if(PM_VAMPIRE)) {
            if (is_undead(gy.youmonst.data) || Race_if(PM_DRAUGR)
                || Race_if(PM_VAMPIRE)) {
                if (!Deaf && !(svm.moves % 5)) {
                    SetVoice(magr, 0, 80, 0);
                    verbalize("I can't heal the undead... you're dead!");
                }
            }
            hitmsg(magr, mattk);
            return;
        }
        /* weapon check should match the one in sounds.c for MS_NURSE */
        if (!(uwep && (uwep->oclass == WEAPON_CLASS || is_weptool(uwep)))
            && !uarmu && !uarm && !uarmc
            && !uarms && !uarmg && !uarmf && !uarmh) {
            boolean goaway = FALSE;

            pline_mon(magr, "%s hits!  (I hope you don't mind.)",
                      Monnam(magr));
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
                    if (u.uhpmax < 5 * u.ulevel + d(2 * u.ulevel, 10)) {
                        u.uhpmax++;
                        if (u.uhpmax > u.uhppeak)
                            u.uhppeak = u.uhpmax;
                    }
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
            disp.botl = TRUE;
            if (goaway) {
                mongone(magr);
                mhm->done = TRUE;
                mhm->hitflags = M_ATTK_DEF_DIED; /* return 2??? */
                return;
            } else if (!rn2(33)) {
                if (!tele_restrict(magr))
                    (void) rloc(magr, RLOC_MSG);
                monflee(magr, d(3, 6), TRUE, FALSE);
                mhm->done = TRUE;
                mhm->hitflags = M_ATTK_HIT | M_ATTK_DEF_DIED; /* return 3??? */
                return;
            }
            mhm->damage = 0;
        } else {
            if (Role_if(PM_HEALER)) {
                if (!Deaf && !(svm.moves % 5)) {
                    SetVoice(magr, 0, 80, 0);
                    verbalize("Doc, I can't help you unless you cooperate.");
                }
                mhm->damage = 0;
            } else
                hitmsg(magr, mattk);
        }
    } else {
        /* mhitm */
        mhitm_ad_phys(magr, mattk, mdef, mhm);
        if (mhm->done)
            return;
    }
}

void
mhitm_ad_stun(
    struct monst *magr, struct attack *mattk,
    struct monst *mdef, struct mhitm_data *mhm)
{
    struct permonst *pd = mdef->data;

    if (magr == &gy.youmonst) {
        /* uhitm */
        if (mon_resists_stun(mdef) || defended(mdef, AD_STUN)
            || mon_wielding_artifact(mdef, ART_TEMPEST))
            return; /* immune to stun; plain dice damage still applies */
        if (!Blind)
            pline("%s %s for a moment.", Monnam(mdef),
                  makeplural(stagger(pd, "stagger")));
        mdef->mstun = 1;
        mhitm_ad_phys(magr, mattk, mdef, mhm);
        if (mhm->done)
            return;
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        hitmsg(magr, mattk);
        if (!magr->mcan && !rn2(4)) {
            make_stunned((HStun & TIMEOUT) + (long) mhm->damage, TRUE);
            mhm->damage /= 2;
        }
    } else {
        /* mhitm */
        if (magr->mcan)
            return;
        if (mon_resists_stun(mdef) || defended(mdef, AD_STUN)
            || mon_wielding_artifact(mdef, ART_TEMPEST))
            return; /* immune to stun; plain dice damage still applies */
        if (canseemon(mdef))
            pline_mon(mdef, "%s %s for a moment.", Monnam(mdef),
                  makeplural(stagger(pd, "stagger")));
        mdef->mstun = 1;
        mhitm_ad_phys(magr, mattk, mdef, mhm);
        if (mhm->done)
            return;
    }
}

void
mhitm_ad_legs(
    struct monst *magr, struct attack *mattk,
    struct monst *mdef, struct mhitm_data *mhm)
{
    if (magr == &gy.youmonst) {
        /* uhitm */
#if 0
        if (u.ucancelled) {
            mhm->damage = 0;
            return;
        }
#endif
        mhitm_ad_phys(magr, mattk, mdef, mhm);
        if (mhm->done)
            return;
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        long side = rn2(2) ? RIGHT_SIDE : LEFT_SIDE;
        const char *sidestr = (side == RIGHT_SIDE) ? "right" : "left",
                   *Monst_name = Monnam(magr), *leg = body_part(LEG);

        /* This case is too obvious to ignore, but Nethack is not in
         * general very good at considering height--most short monsters
         * still _can_ attack you when you're flying or mounted.
         */
        if ((u.usteed || Levitation || Flying) && !is_flyer(magr->data)) {
            pline("%s tries to reach your %s %s!", Monst_name, sidestr, leg);
            mhm->damage = 0;
        } else if (magr->mcan) {
            pline_mon(magr, "%s nuzzles against your %s %s!", Monnam(magr),
                  sidestr, leg);
            mhm->damage = 0;
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
                    mhm->damage = 0;
                    return;
                }
            } else
                pline("%s pricks your %s %s!", Monst_name, sidestr, leg);

            set_wounded_legs(side, rnd(60 - ACURR(A_DEX)));
            exercise(A_STR, FALSE);
            exercise(A_DEX, FALSE);
        }
    } else {
        /* mhitm */
        if (magr->mcan) {
            mhm->damage = 0;
            return;
        }
        mhitm_ad_phys(magr, mattk, mdef, mhm);
        if (mhm->done)
            return;
    }
}

void
mhitm_ad_dgst(
    struct monst *magr,
    struct attack *mattk UNUSED,
    struct monst *mdef, struct mhitm_data *mhm)
{
    struct permonst *pd = mdef->data;

    if (magr == &gy.youmonst) {
        /* uhitm */
        mhm->damage = 0;
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        mhm->damage = 0;
    } else {
        /* mhitm */
        int num;
        struct obj *obj;

        /* slow digestion protects against being swallowed whole */
        if (mon_prop(mdef, SLOW_DIGESTION)) {
            mhm->hitflags = (M_ATTK_HIT | M_ATTK_EXPELLED);
            mhm->done = TRUE;
            return;
        }
        /* spikes and runes are equally hard to swallow */
        if ((obj = which_armor(mdef, W_BARDING)) != 0
            && (obj->otyp == SPIKED_BARDING
                || obj->otyp == RUNED_BARDING)) {
            mhm->hitflags = (M_ATTK_HIT | M_ATTK_EXPELLED);
            mhm->done = TRUE;
            return;
        }
        /* unique monsters are too significant to be digested;
           expelled immediately with no damage, like slow digestion */
        if (unique_corpstat(pd)) {
            mhm->hitflags = (M_ATTK_HIT | M_ATTK_EXPELLED);
            mhm->done = TRUE;
            return;
        }
        /* eating a Rider or its corpse is fatal */
        if (is_rider(pd)) {
            if (gv.vis && canseemon(magr))
                pline_mon(magr, "%s %s!", Monnam(magr),
                      (pd == &mons[PM_FAMINE])
                          ? "belches feebly, shrivels up and dies"
                          : (pd == &mons[PM_PESTILENCE])
                                ? "coughs spasmodically and collapses"
                                : "vomits violently and drops dead");
            mondied(magr);
            if (!DEADMONSTER(magr)) {
                mhm->hitflags = M_ATTK_MISS; /* lifesaved */
                mhm->done = TRUE;
                return;
            } else if (magr->mtame && !gv.vis)
                You(brief_feeling, "queasy");
            mhm->hitflags = M_ATTK_AGR_DIED;
            mhm->done = TRUE;
            return;
        }
        if (flags.verbose && !Deaf) {
            /* Soundeffect? */
            SetVoice(magr, 0, 80, 0);
            verbalize("Burrrrp!");
        }
        wake_nearto(magr->mx, magr->my, 2 * 2); /* Burrrrp! */
        mhm->damage = mdef->mhp;
        /* Use up amulet of life saving */
        if ((obj = mlifesaver(mdef)) != 0)
            m_useup(mdef, obj);

        /* Is a corpse for nutrition possible?  It may kill magr */
        if (!corpse_chance(mdef, magr, TRUE) || DEADMONSTER(magr))
            return;

        /* Pets get nutrition from swallowing monster whole.
         * No nutrition from G_NOCORPSE monster, eg, undead.
         * DGST monsters don't die from undead corpses
         */
        num = monsndx(pd);
        if (magr->mtame && !magr->isminion
            && !(svm.mvitals[num].mvflags & G_NOCORPSE)) {
            struct obj *virtualcorpse = mksobj(CORPSE, FALSE, FALSE);
            int nutrit;

            set_corpsenm(virtualcorpse, num);
            nutrit = dog_nutrition(magr, virtualcorpse);
            dealloc_obj(virtualcorpse);

            /* only 50% nutrition, 25% of normal eating time */
            if (magr->meating > 1)
                magr->meating = (magr->meating + 3) / 4;
            if (nutrit > 1)
                nutrit /= 2;
            EDOG(magr)->hungrytime += nutrit;
        }
    }
}

void
mhitm_ad_samu(
    struct monst *magr, struct attack *mattk,
    struct monst *mdef, struct mhitm_data *mhm)
{
    if (magr == &gy.youmonst) {
        /* uhitm */
        mhm->damage = 0;
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        hitmsg(magr, mattk);
        /* when the Wizard or quest nemesis hits, there's a 1/20 chance
           to steal a quest artifact (any, not just the one for the hero's
           own role) or the Amulet or one of the invocation tools */
        if (Hidinshell)
            return;
        if (!rn2(20))
            stealamulet(magr);
    } else {
        /* mhitm */
        mhm->damage = 0;
    }
}

/* disease */
void
mhitm_ad_dise(
    struct monst *magr, struct attack *mattk,
    struct monst *mdef, struct mhitm_data *mhm)
{
    if (magr == &gy.youmonst) {
        /* uhitm */
        if (!(resists_sick(mdef) || defended(mdef, AD_DISE))) {
            if (mdef->mdiseasetime)
                mdef->mdiseasetime -= rnd(3);
            else
                mdef->mdiseasetime = rn1(9, 6);
            if (canseemon(mdef))
                pline_mon(mdef, "%s looks %s.", Monnam(mdef),
                          mdef->mdiseased ? "even worse" : "diseased");
            mdef->mdiseased = 1;
            mdef->mdiseabyu = TRUE;
        }
        /* plus ordinary damage */
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        hitmsg(magr, mattk);
        if (!diseasemu(magr))
            mhm->damage = 0;
    } else {
        /* mhitm */
        if (resists_sick(mdef) || defended(mdef, AD_DISE)) {
            if (gv.vis && canseemon(mdef))
                pline_mon(mdef, "%s resists infection.", Monnam(mdef));
            mhm->damage = 0;
        } else {
            if (mdef->mdiseasetime)
                mdef->mdiseasetime -= rnd(3);
            else
                mdef->mdiseasetime = rn1(9, 6);
            if (gv.vis && canseemon(mdef))
                pline_mon(mdef, "%s looks %s.", Monnam(mdef),
                          mdef->mdiseased ? "even worse" : "diseased");
            mdef->mdiseased = 1;
            mdef->mdiseabyu = FALSE;
            /* plus ordinary damage */
        }
    }
}

/* seduce and also steal item */
void
mhitm_ad_sedu(
    struct monst *magr, struct attack *mattk,
    struct monst *mdef, struct mhitm_data *mhm)
{
    struct permonst *pa = magr->data;

    if (magr == &gy.youmonst) {
        /* uhitm */
        steal_it(mdef, mattk);
        mhm->damage = 0;
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        char buf[BUFSZ];
        /* mystic Druids are one with nature and cannot be
           tempted by nymphs */
        boolean druid_mystic = (Role_if(PM_DRUID) && u.ulevel >= 14
                                && is_nymph(magr->data));

        if (is_animal(magr->data)) {
            hitmsg(magr, mattk);
            if (magr->mcan || Hidinshell)
                return;
            /* Continue below */
        } else if (dmgtype(gy.youmonst.data, AD_SEDU)
                   /* !SYSOPT_SEDUCE: when hero is attacking and AD_SSEX
                      is disabled, it would be changed to another damage
                      type, but when defending, it remains as-is */
                   || dmgtype(gy.youmonst.data, AD_SSEX)) {
            pline_mon(magr, "%s %s.", Monnam(magr),
                  Deaf ? "says something but you can't hear it"
                  : magr->minvent
                    ? "brags about the goods some dungeon explorer provided"
                  : "makes some remarks about how difficult theft is lately");
            if (!tele_restrict(magr))
                (void) rloc(magr, RLOC_MSG);
            mhm->hitflags = M_ATTK_AGR_DONE; /* return 3??? */
            mhm->done = TRUE;
            return;
        } else if (magr->mcan || Hidinshell || druid_mystic) {
            if (!Blind)
                pline("%s tries to %s you, but you seem %s.",
                      !magr->mcan ? Monnam(magr)
                                  : Adjmonnam(magr, "plain"),
                      flags.female ? "charm" : "seduce",
                      flags.female ? "unaffected" : "uninterested");
            if (rn2(3)) {
                if (!tele_restrict(magr))
                    (void) rloc(magr, RLOC_MSG);
                mhm->hitflags = M_ATTK_AGR_DONE; /* return 3??? */
                mhm->done = TRUE;
                return;
            }
            return;
        }
        buf[0] = '\0';
        switch (steal(magr, buf)) {
        case -1:
            mhm->hitflags = M_ATTK_AGR_DIED; /* return 2??? */
            mhm->done = TRUE;
            return;
        case 0:
            return;
        default:
            if (!is_animal(magr->data) && !tele_restrict(magr))
                (void) rloc(magr, RLOC_MSG);
            if (is_animal(magr->data) && *buf) {
                if (canseemon(magr))
                    pline_mon(magr, "%s tries to %s away with %s.",
                              Monnam(magr),
                              locomotion(magr->data, "run"), buf);
            }
            monflee(magr, 0, FALSE, FALSE);
            mhm->hitflags = M_ATTK_AGR_DONE; /* return 3??? */
            mhm->done = TRUE;
            return;
        }
    } else {
        /* mhitm */
        struct obj *obj;

        if (magr->mcan)
            return;
        /* find an object to steal, non-cursed if magr is tame */
        for (obj = mdef->minvent; obj; obj = obj->nobj)
            if (!magr->mtame || !obj->cursed)
                break;

        if (obj) {
            char buf[BUFSZ];
            char onambuf[BUFSZ], mdefnambuf[BUFSZ];

            /* make a special x_monnam() call that never omits
               the saddle, and save it for later messages */
            Strcpy(mdefnambuf,
                   x_monnam(mdef, ARTICLE_THE, (char *) 0, 0, FALSE));

            if (u.usteed == mdef && obj == which_armor(mdef, W_SADDLE))
                /* "You can no longer ride <steed>." */
                dismount_steed(DISMOUNT_POLY);
            extract_from_minvent(mdef, obj, TRUE, FALSE);
            /* add_to_minv() might free 'obj' [if it merges] */
            if (gv.vis)
                Strcpy(onambuf, doname(obj));
            (void) add_to_minv(magr, obj);
            Strcpy(buf, Monnam(magr));
            if (gv.vis && canseemon(mdef)) {
                pline("%s steals %s from %s!", buf, onambuf, mdefnambuf);
            }
            possibly_unwield(mdef, FALSE);
            mdef->mstrategy &= ~STRAT_WAITFORU;
            mselftouch(mdef, (const char *) 0, FALSE);
            if (DEADMONSTER(mdef)) {
                mhm->hitflags = (M_ATTK_DEF_DIED
                                 | (grow_up(magr, mdef) ? 0
                                    : M_ATTK_AGR_DIED));
                mhm->done = TRUE;
                return;
            }
            if (pa->mlet == S_NYMPH && !tele_restrict(magr)) {
                boolean couldspot = canspotmon(magr);

                mhm->hitflags = M_ATTK_AGR_DONE;
                (void) rloc(magr, RLOC_NOMSG);
                /* TODO: use RLOC_MSG instead? */
                if (gv.vis && couldspot && !canspotmon(magr))
                    pline("%s suddenly disappears!", buf);
            }
        }
        mhm->damage = 0;
    }
}

void
mhitm_ad_ssex(struct monst *magr, struct attack *mattk, struct monst *mdef,
              struct mhitm_data *mhm)
{
    if (magr == &gy.youmonst) {
        /* uhitm */
        mhitm_ad_sedu(magr, mattk, mdef, mhm);
        if (mhm->done)
            return;
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        if (SYSOPT_SEDUCE) {
            if (could_seduce(magr, mdef, mattk) == 1
                && !(magr->mcan || Hidinshell))
                if (doseduce(magr)) {
                    mhm->hitflags = M_ATTK_AGR_DONE;
                    mhm->done = TRUE;
                    return;
                }
            return;
        }
        mhitm_ad_sedu(magr, mattk, mdef, mhm);
        if (mhm->done)
            return;
    } else {
        /* mhitm */
        mhitm_ad_sedu(magr, mattk, mdef, mhm);
        if (mhm->done)
            return;
    }
}

void
mhitm_ad_webs(
    struct monst *magr, struct attack *mattk,
    struct monst *mdef, struct mhitm_data *mhm)
{
    if (magr == &gy.youmonst) {
        /* uhitm */
        if (mhitm_mgc_atk_negated(magr, mdef, FALSE))
            return;
        if (!t_at(mdef->mx, mdef->my)) {
            struct trap *web = maketrap(mdef->mx, mdef->my, WEB);

            if (web) {
                (void) mintrap(mdef, NO_TRAP_FLAGS);
                web_unseats_rider(mdef);
            }
        }
    } else if (mdef == &gy.youmonst) {
        /* mhitu */
        if (mhitm_mgc_atk_negated(magr, mdef, FALSE)) {
            mhm->damage = 0;
            return;
        }
        if (!t_at(u.ux, u.uy)) {
            struct trap *web = maketrap(u.ux, u.uy, WEB);

            if (web) {
                hitmsg(magr, mattk);
                pline("%s entangles you in a web%s", Monnam(magr),
                      (is_pool_or_lava(u.ux, u.uy)
                       || IS_AIR(levl[u.ux][u.uy].typ))
                          ? ", but it has nothing to anchor to."
                          : is_giant(gy.youmonst.data)
                              ? ", but you rip through it!"
                              : (webmaker(gy.youmonst.data)
                                 || maybe_polyd(is_drow(gy.youmonst.data),
                                                Race_if(PM_DROW)))
                                  ? ", but you easily disentangle yourself."
                                  : "!");
                dotrap(web, NOWEBMSG);
                if (u.usteed && u.utrap)
                    dismount_steed(DISMOUNT_FELL);
            }
        }
        mhm->damage = 0;
    } else {
        /* mhitm */
        if (mhitm_mgc_atk_negated(magr, mdef, FALSE))
            return;
        if (!t_at(mdef->mx, mdef->my)) {
            struct trap *web = maketrap(mdef->mx, mdef->my, WEB);

            if (web) {
                (void) mintrap(mdef, NO_TRAP_FLAGS);
                web_unseats_rider(mdef);
            }
        }
    }
}

void
mhitm_adtyping(
    struct monst *magr, struct attack *mattk,
    struct monst *mdef, struct mhitm_data *mhm)
{
    switch (mattk->adtyp) {
    case AD_STUN: mhitm_ad_stun(magr, mattk, mdef, mhm); break;
    case AD_LEGS: mhitm_ad_legs(magr, mattk, mdef, mhm); break;
    case AD_WERE: mhitm_ad_were(magr, mattk, mdef, mhm); break;
    case AD_HEAL: mhitm_ad_heal(magr, mattk, mdef, mhm); break;
    case AD_PHYS: mhitm_ad_phys(magr, mattk, mdef, mhm); break;
    case AD_FIRE: mhitm_ad_fire(magr, mattk, mdef, mhm); break;
    case AD_COLD: mhitm_ad_cold(magr, mattk, mdef, mhm); break;
    case AD_ELEC: mhitm_ad_elec(magr, mattk, mdef, mhm); break;
    case AD_ACID: mhitm_ad_acid(magr, mattk, mdef, mhm); break;
    case AD_STON: mhitm_ad_ston(magr, mattk, mdef, mhm); break;
    case AD_SSEX: mhitm_ad_ssex(magr, mattk, mdef, mhm); break;
    case AD_SITM:
    case AD_SEDU: mhitm_ad_sedu(magr, mattk, mdef, mhm); break;
    case AD_SGLD: mhitm_ad_sgld(magr, mattk, mdef, mhm); break;
    case AD_TLPT: mhitm_ad_tlpt(magr, mattk, mdef, mhm); break;
    case AD_BLND: mhitm_ad_blnd(magr, mattk, mdef, mhm); break;
    case AD_CURS: mhitm_ad_curs(magr, mattk, mdef, mhm); break;
    case AD_DRLI: mhitm_ad_drli(magr, mattk, mdef, mhm); break;
    case AD_RUST: mhitm_ad_rust(magr, mattk, mdef, mhm); break;
    case AD_CORR: mhitm_ad_corr(magr, mattk, mdef, mhm); break;
    case AD_DCAY: mhitm_ad_dcay(magr, mattk, mdef, mhm); break;
    case AD_DREN: mhitm_ad_dren(magr, mattk, mdef, mhm); break;
    case AD_DRST:
    case AD_DRDX:
    case AD_DRCO: mhitm_ad_drst(magr, mattk, mdef, mhm); break;
    case AD_DRIN: mhitm_ad_drin(magr, mattk, mdef, mhm); break;
    case AD_STCK: mhitm_ad_stck(magr, mattk, mdef, mhm); break;
    case AD_WRAP: mhitm_ad_wrap(magr, mattk, mdef, mhm); break;
    case AD_PLYS: mhitm_ad_plys(magr, mattk, mdef, mhm); break;
    case AD_SLEE: mhitm_ad_slee(magr, mattk, mdef, mhm); break;
    case AD_SLIM: mhitm_ad_slim(magr, mattk, mdef, mhm); break;
    case AD_ENCH: mhitm_ad_ench(magr, mattk, mdef, mhm); break;
    case AD_SLOW: mhitm_ad_slow(magr, mattk, mdef, mhm); break;
    case AD_CONF: mhitm_ad_conf(magr, mattk, mdef, mhm); break;
    case AD_POLY: mhitm_ad_poly(magr, mattk, mdef, mhm); break;
    case AD_DISE: mhitm_ad_dise(magr, mattk, mdef, mhm); break;
    case AD_SAMU: mhitm_ad_samu(magr, mattk, mdef, mhm); break;
    case AD_DETH: mhitm_ad_deth(magr, mattk, mdef, mhm); break;
    case AD_PEST: mhitm_ad_pest(magr, mattk, mdef, mhm); break;
    case AD_FAMN: mhitm_ad_famn(magr, mattk, mdef, mhm); break;
    case AD_DGST: mhitm_ad_dgst(magr, mattk, mdef, mhm); break;
    case AD_HALU: mhitm_ad_halu(magr, mattk, mdef, mhm); break;
    case AD_WEBS: mhitm_ad_webs(magr, mattk, mdef, mhm); break;
    default:
        mhm->damage = 0;
    }
}

int
damageum(
    struct monst *mdef,   /* target */
    struct attack *mattk, /* hero's attack */
    int specialdmg) /* blessed and/or silver bonus against various things */
{
    struct mhitm_data mhm;

    mhm.damage = d((int) mattk->damn, (int) mattk->damd);
    mhm.hitflags = M_ATTK_MISS;
    mhm.permdmg = 0;
    mhm.specialdmg = specialdmg;
    mhm.done = FALSE;

    if (is_demon(gy.youmonst.data) && !rn2(13) && !uwep
        && u.umonnum != PM_AMOROUS_DEMON && u.umonnum != PM_BALROG) {
        demonpet();
        return M_ATTK_MISS;
    }

    mhitm_adtyping(&gy.youmonst, mattk, mdef, &mhm);

    if (mhm.done)
        return mhm.hitflags;

    mdef->mstrategy &= ~STRAT_WAITFORU; /* in case player is very fast */
    mdef->mhp -= mhm.damage;
    if (DEADMONSTER(mdef)) {
        /* troll killed by Trollsbane won't auto-revive; FIXME? same when
           Trollsbane is wielded as primary and two-weaponing kills with
           secondary, which matches monster vs monster behavior but is
           different from the non-poly'd hero vs monster behavior */
        if (mattk->aatyp == AT_WEAP || mattk->aatyp == AT_CLAW)
            gm.mkcorpstat_norevive = troll_baned(mdef, uwep) ? TRUE : FALSE;
        /* (DEADMONSTER(mdef) and !mhm.damage => already killed) */
        if (mdef->mtame && !cansee(mdef->mx, mdef->my)) {
            You_feel("embarrassed for a moment.");
            if (mhm.damage)
                xkilled(mdef, XKILL_NOMSG);
        } else if (!flags.verbose) {
            You("destroy it!");
            if (mhm.damage)
                xkilled(mdef, XKILL_NOMSG);
        } else if (mhm.damage) {
            killed(mdef); /* regular "you kill <mdef>" message */
        }
        gm.mkcorpstat_norevive = FALSE;
        return M_ATTK_DEF_DIED;
    }
    return M_ATTK_HIT;
}

/* Hero, as a monster which is capable of an exploding attack mattk, is
 * exploding at a target monster mdef, or just exploding at nothing (e.g. with
 * forcefight) if mdef is null.
 */
int
explum(struct monst *mdef, struct attack *mattk)
{
    int tmp = d((int) mattk->damn, (int) mattk->damd);

    switch (mattk->adtyp) {
    case AD_BLND:
        if (mdef && !resists_blnd(mdef)) {
            pline("%s is blinded by your flash of light!", Monnam(mdef));
            mdef->mblinded = min((int) mdef->mblinded + tmp, 127);
            mdef->mcansee = 0;
        }
        /* light-haters take damage from the intense light itself */
        if (mdef && hates_light(r_data(mdef))) {
            if (!Deaf)
                pline("%s cries out in pain!", Monnam(mdef));
            if (damage_mon(mdef, rnd(5), AD_BLND, TRUE))
                xkilled(mdef, XKILL_GIVEMSG);
            if (DEADMONSTER(mdef)) {
                /* other monsters may have died too, but return this
                   if the actual target died */
                return M_ATTK_DEF_DIED;
            }
        }
        break;
    case AD_HALU:
        if (mdef && haseyes(mdef->data) && mdef->mcansee) {
            pline("%s is affected by your flash of light!", Monnam(mdef));
            mdef->mconf = 1;
        }
        break;
    case AD_COLD:
    case AD_FIRE:
    case AD_ELEC:
        /* TODO-EVILHACK: AD_ACID belongs here too (acid sphere form),
           with corrosion of the defender's wielded weapons; ports with
           the EXPL_ACID/EXPL_SHOCK explosion-glyph extension */
        /* see comment in mon_explodes() and in zap.c for an explanation
           of this encoding; here the player is causing the explosion,
           so it uses the positive hero-breath section */
        explode(u.ux, u.uy, BZ_U_BREATH(BZ_OFS_AD(mattk->adtyp)), tmp,
                MON_EXPLODE, adtyp_to_expltype(mattk->adtyp));
        if (mdef && DEADMONSTER(mdef)) {
            /* Other monsters may have died too, but return this if the actual
               target died. */
            return M_ATTK_DEF_DIED;
        }
        break;
    default:
        break;
    }
    wake_nearto(u.ux, u.uy, 7 * 7); /* same radius as exploding monster */
    return M_ATTK_HIT;
}

staticfn void
start_engulf(struct monst *mdef)
{
    boolean u_digest = digests(gy.youmonst.data),
            u_enfold = enfolds(gy.youmonst.data);

    if (!Invisible) {
        map_location(u.ux, u.uy, TRUE);
        tmp_at(DISP_ALWAYS, mon_to_glyph(&gy.youmonst, rn2_on_display_rng));
        tmp_at(mdef->mx, mdef->my);
    }
    You("%s %s%s!",
        u_digest ? "swallow" : u_enfold ? "enclose" : "engulf",
        mon_nam(mdef), u_digest ? " whole" : "");
    nh_delay_output();
    nh_delay_output();
}

staticfn void
end_engulf(void)
{
    if (!Invisible) {
        tmp_at(DISP_END, 0);
        newsym(u.ux, u.uy);
    }
}

staticfn int
gulpum(struct monst *mdef, struct attack *mattk)
{
    static char msgbuf[BUFSZ]; /* for gn.nomovemsg */
    int tmp;
    int dam = d((int) mattk->damn, (int) mattk->damd);
    boolean fatal_gulp,
            u_digest = digests(gy.youmonst.data),
            u_enfold = enfolds(gy.youmonst.data);
    struct obj *otmp;
    struct permonst *pd = mdef->data;
    const char *expel_verb = u_digest ? "regurgitate"
                             : u_enfold ? "release"
                               : "expel";

    /* Not totally the same as for real monsters.  Specifically, these
     * don't take multiple moves.  (It's just too hard, for too little
     * result, to program monsters which attack from inside you, which
     * would be necessary if done accurately.)  Instead, we arbitrarily
     * kill the monster immediately for AD_DGST and we regurgitate them
     * after exactly 1 round of attack otherwise.  -KAA
     */

    if (!engulf_target(&gy.youmonst, mdef))
        return M_ATTK_MISS;

    if (!(u_digest && u.uhunger >= 1500) && !u.uswallow) {
        if (!flaming(gy.youmonst.data)) {
            for (otmp = mdef->minvent; otmp; otmp = otmp->nobj)
                (void) snuff_lit(otmp, FALSE);
        }

        /* force vampire in bat, cloud, or wolf form to revert back to
           vampire form now instead of dealing with that when it dies */
        if (is_vampshifter(mdef)
            && newcham(mdef, &mons[mdef->cham], NO_NC_FLAGS)) {
            You("%s it, then %s it.",
                u_digest ? "swallow" : u_enfold ? "enclose" : "engulf",
                expel_verb);
            if (canspotmon(mdef)) {
                /* Avoiding a_monnam here: if the target is named, it gives us
                   a sequence like "You bite Dracula.  You swallow it, then
                   regurgitate it.  It turns into Dracula." */
                pline("It turns into %s.",
                      x_monnam(mdef, ARTICLE_A, (char *) 0,
                               (SUPPRESS_NAME | SUPPRESS_IT
                                | SUPPRESS_INVISIBLE), FALSE));
            } else {
                map_invisible(mdef->mx, mdef->my);
            }
            return M_ATTK_HIT;
        }

        /* engulfing a cockatrice or digesting a Rider or Medusa */
        fatal_gulp = (touch_petrifies(pd) && !Stone_resistance)
                     || (mattk->adtyp == AD_DGST
                         && (is_rider(pd) || (pd == &mons[PM_MEDUSA]
                                              && !Stone_resistance)));

        if (mattk->adtyp == AD_DGST && (!Slow_digestion || fatal_gulp))
            eating_conducts(pd);

        if (fatal_gulp && !is_rider(pd)) { /* petrification */
            char kbuf[BUFSZ];
            const char *mnam = pmname(pd, Mgender(mdef));

            if (!type_is_pname(pd))
                mnam = an(mnam);
            You("%s %s.", u_digest ? "englut" : "engulf", mon_nam(mdef));
            Sprintf(kbuf, "%s %s%s",
                    u_digest ? "swallowing"
                    : u_enfold ? "enclosing"
                      : "engulfing",
                    mnam, u_digest ? " whole" : "");
            instapetrify(kbuf);
        } else {
            start_engulf(mdef);
            switch (mattk->adtyp) {
            case AD_DGST: {
                struct obj *sbarding;

                /* slow digestion protects against being swallowed */
                if (mon_prop(mdef, SLOW_DIGESTION)) {
                    You("hurriedly %s the indigestible %s.",
                        expel_verb, m_monnam(mdef));
                    end_engulf();
                    return M_ATTK_HIT;
                }
                /* spikes and runes are equally hard to swallow */
                if ((sbarding = which_armor(mdef, W_BARDING)) != 0
                    && (sbarding->otyp == SPIKED_BARDING
                        || sbarding->otyp == RUNED_BARDING)) {
                    You("hurriedly %s the indigestible %s.",
                        expel_verb, m_monnam(mdef));
                    end_engulf();
                    return M_ATTK_HIT;
                }
                /* eating a Rider or its corpse is fatal */
                if (is_rider(pd)) {
                    pline("Unfortunately, digesting any of it is fatal.");
                    end_engulf();
                    Sprintf(svk.killer.name, "unwisely tried to eat %s",
                            pmname(pd, Mgender(mdef)));
                    svk.killer.format = NO_KILLER_PREFIX;
                    done(DIED);
                    return M_ATTK_MISS; /* lifesaved */
                }
                /* unique monsters are too significant to be digested;
                   expelled immediately with no damage, like slow
                   digestion */
                if (unique_corpstat(pd)) {
                    You("hurriedly %s the indigestible %s.",
                        expel_verb, m_monnam(mdef));
                    end_engulf();
                    return M_ATTK_HIT;
                }

                if (Slow_digestion) {
                    dam = 0;
                    break;
                }

                /* Use up amulet of life saving */
                if ((otmp = mlifesaver(mdef)) != 0)
                    m_useup(mdef, otmp);

                newuhs(FALSE);
                /* start_engulf() issues "you engulf <mdef>" above; this
                   used to specify XKILL_NOMSG but we need "you kill <mdef>"
                   in case we're also going to get "welcome to level N+1";
                   "you totally digest <mdef>" will be coming soon (after
                   several turns) but the level-gain message seems out of
                   order if the kill message is left implicit */
                gm.mswallower = &gy.youmonst;
                xkilled(mdef, XKILL_GIVEMSG | XKILL_NOCORPSE);
                if (!DEADMONSTER(mdef)) { /* monster lifesaved */
                    You("hurriedly regurgitate the sizzling in your %s.",
                        body_part(STOMACH));
                } else {
                    tmp = 1 + (pd->cwt >> 8);
                    if (corpse_chance(mdef, &gy.youmonst, TRUE)
                        && !(svm.mvitals[monsndx(pd)].mvflags & G_NOCORPSE)) {
                        /* nutrition only if there can be a corpse */
                        u.uhunger += (pd->cnutrit + 1) / 2;
                    } else {
                        tmp = 0;
                    }
                    Sprintf(msgbuf, "You totally digest %s.", mon_nam(mdef));
                    if (tmp != 0) {
                        /* setting afternmv = end_engulf is tempting,
                         * but will cause problems if the player is
                         * attacked (which uses his real location) or
                         * if his See_invisible wears off
                         */
                        You("digest %s.", mon_nam(mdef));
                        if (Slow_digestion)
                            tmp *= 2;
                        nomul(-tmp);
                        gm.multi_reason = "digesting something";
                        gn.nomovemsg = msgbuf;
                        /* possible intrinsic once totally digested */
                        gc.corpsenm_digested = monsndx(pd);
                        ga.afternmv = Finish_digestion;
                    } else
                        pline1(msgbuf);
                    if (pd == &mons[PM_GREEN_SLIME]) {
                        Sprintf(msgbuf, "%s isn't sitting well with you.",
                                The(pmname(pd, Mgender(mdef))));
                        if (!Unchanging) {
                            make_slimed(5L, (char *) 0);
                        }
                    } else
                        exercise(A_CON, TRUE);
                }
                gm.mswallower = (struct monst *) 0;
                end_engulf();
                return M_ATTK_DEF_DIED;
            }
            case AD_PHYS:
                if (gy.youmonst.data == &mons[PM_FOG_CLOUD]) {
                    pline("%s is laden with your moisture.", Monnam(mdef));
                    if ((breathless(pd) || amphibious(pd)) && !flaming(pd)) {
                        dam = 0;
                        pline("%s seems unharmed.", Monnam(mdef));
                    }
                } else {
                    pline("%s is %s!", Monnam(mdef),
                          enfolds(gy.youmonst.data) ? "being squashed"
                            : "pummeled with your debris");
                }
                break;
            case AD_ACID:
                pline("%s is covered with your goo!", Monnam(mdef));
                if (resists_acid(mdef)) {
                    pline("It seems harmless to %s.", mon_nam(mdef));
                    dam = 0;
                }
                break;
            case AD_BLND:
                if (can_blnd(&gy.youmonst, mdef, mattk->aatyp,
                             (struct obj *) 0)) {
                    if (mdef->mcansee)
                        pline("%s can't see in there!", Monnam(mdef));
                    mdef->mcansee = 0;
                    dam += mdef->mblinded;
                    if (dam > 127)
                        dam = 127;
                    mdef->mblinded = dam;
                }
                dam = 0;
                break;
            case AD_ELEC:
                if (rn2(2)) {
                    pline_The("air around %s crackles with electricity.",
                              mon_nam(mdef));
                    if (resists_elec(mdef)) {
                        pline("%s seems unhurt.", Monnam(mdef));
                        dam = 0;
                    }
                    golemeffects(mdef, (int) mattk->adtyp, dam);
                } else
                    dam = 0;
                break;
            case AD_COLD:
                if (rn2(2)) {
                    if (resists_cold(mdef)) {
                        pline("%s seems mildly chilly.", Monnam(mdef));
                        dam = 0;
                    } else
                        pline("%s is freezing to death!", Monnam(mdef));
                    golemeffects(mdef, (int) mattk->adtyp, dam);
                } else
                    dam = 0;
                break;
            case AD_FIRE:
                if (rn2(2)) {
                    if (resists_fire(mdef)) {
                        pline("%s seems mildly hot.", Monnam(mdef));
                        dam = 0;
                    } else
                        pline("%s is burning to a crisp!", Monnam(mdef));
                    golemeffects(mdef, (int) mattk->adtyp, dam);
                } else
                    dam = 0;
                break;
            case AD_DREN:
                if (!rn2(4))
                    xdrainenergym(mdef, TRUE);
                dam = 0;
                break;
            }
            end_engulf();
            mdef->mhp -= dam;
            if (DEADMONSTER(mdef)) {
                killed(mdef);
                if (DEADMONSTER(mdef)) /* not lifesaved */
                    return M_ATTK_DEF_DIED;
            }
            You("%s %s!", expel_verb, mon_nam(mdef));
            if ((Slow_digestion || is_animal(gy.youmonst.data)) && u_digest) {
                pline("Obviously, you didn't like %s taste.",
                      s_suffix(mon_nam(mdef)));
            }
        }
    }
    return M_ATTK_MISS;
}

void
missum(
    struct monst *mdef,
    int target,           /* hero's to-hit total */
    int roll,             /* what the hero rolled against it */
    struct attack *mattk,
    boolean wouldhavehit)
{
    struct obj *blocker;
    boolean nearmiss = (target == roll);

    blocker = armor_blocker(mdef, &target, roll);

    if (wouldhavehit) /* monk is missing due to penalty for wearing suit */
        Your("armor is rather cumbersome...");

    if (could_seduce(&gy.youmonst, mdef, mattk)) {
        You("pretend to be friendly to %s.", mon_nam(mdef));
    } else if (canspotmon(mdef) && flags.verbose) {
        if (nearmiss || !blocker) {
            if ((thick_skinned(mdef->data)
                 || has_barkskin(mdef) || has_stoneskin(mdef))
                && !rn2(10))
                pline_mon(mdef, "%s %s %s your attack.",
                          s_suffix(Monnam(mdef)),
                          is_dragon(mdef->data)
                            ? "scaly hide"
                            : (mdef->data == &mons[PM_GIANT_TURTLE]
                               || is_tortle(mdef->data))
                              ? "protective shell"
                              : is_bone_monster(mdef->data)
                                ? "bony structure"
                                : (has_bark(mdef->data)
                                   || has_barkskin(mdef))
                                  ? "rough bark"
                                  : has_stoneskin(mdef)
                                    ? "stone hide" : "thick hide",
                          rn2(2) ? "blocks" : "deflects");
            else
                You("%smiss %s.", nearmiss ? "just " : "", mon_nam(mdef));
        } else {
            pline_mon(mdef, "%s %s %s your attack.", s_suffix(Monnam(mdef)),
                      aobjnam(blocker, (char *) 0),
                      rn2(2) ? "blocks" : "deflects");
        }
    } else {
        You("%smiss it.", (flags.verbose && nearmiss) ? "just " : "");
    }

    /* What a deflection does is not conditional on the hero watching it
       happen. Master nests both of these inside the message arm above,
       so a blind hero takes no searing and unseen armor never breaks;
       maintainer ruling is that a display option must not decide
       whether damage lands, in any of the three combat directions */
    if (blocker) {
        /* the hero grazes a material they hate on the way past */
        if (!uwep && !uarmg
            && Hate_material(blocker->material)
            && !(has_barkskin(mdef) || has_stoneskin(mdef))) {
            searmsg(mdef, &gy.youmonst, blocker, FALSE);
            /* glancing blow */
            losehp(rnd(sear_damage(blocker->material) / 2),
                   "touching a hated material", KILLED_BY);
        }
        /* glass armor, or certain drow armor in the presence of light,
           can break when it deflects an attack */
        if (is_glass(blocker) || is_adamantine(blocker)
            || blocker->forged_qual == FQ_INFERIOR)
            (void) break_glass_obj(blocker);
    }
    /* breaking the blocker can petrify a cockatrice-wielding defender */
    if (!DEADMONSTER(mdef) && !helpless(mdef))
        wakeup(mdef, TRUE);
}

/* check whether equipment protects against knockback */
boolean
m_is_steadfast(struct monst *mtmp)
{
    boolean is_u = (mtmp == &gy.youmonst);

    /* must be on the ground (or in water) */
    if ((is_u ? (Flying || Levitation)
              : (is_flyer(mtmp->data) || is_floater(mtmp->data)))
        || Is_airlevel(&u.uz) /* air or cloud */
        || (Is_waterlevel(&u.uz) && !is_pool(u.ux, u.uy))) /* air bubble */
        return FALSE;

    /* Giantslayer and Harbinger both anchor their wielder,
       in either hand */
    if (is_u ? (wielding_artifact(ART_GIANTSLAYER)
                || wielding_artifact(ART_HARBINGER))
             : (mon_wielding_artifact(mtmp, ART_GIANTSLAYER)
                || mon_wielding_artifact(mtmp, ART_HARBINGER)))
        return TRUE;

    /* steadfast if carrying any loadstone (and not floating or flying);
       'is_u' test not needed here; m_carrying() is 'youmonst' aware */
    if (m_carrying(mtmp, LOADSTONE))
        return TRUE;
    /* when mounted and steed is target of knockback, check the rider for
       a loadstone too (Giantslayer's protection doesn't extend to steed) */
    if (u.usteed && mtmp == u.usteed && carrying(LOADSTONE))
        return TRUE;

    return FALSE;
}

/* monster hits another monster hard enough to knock it back? */
boolean
mhitm_knockback(
    struct monst *magr,   /* attacker; might be hero */
    struct monst *mdef,   /* defender; might be hero (only if magr isn't)  */
    struct attack *mattk, /* attack type and damage info */
    int *hitflags,        /* modified if magr or mdef dies */
    boolean weapon_used)  /* True: via weapon hit */
{
    char magrbuf[BUFSZ], mdefbuf[BUFSZ];
    struct obj *otmp;
    const char *knockedhow;
    coordxy dx, dy, defx, defy;
    int knockdistance = rn2(3) ? 1 : 2; /* 67%: 1 step, 33%: 2 steps */
    int chance = 6; /* 1/6 chance of attack knocking back a monster */
    boolean u_agr = (magr == &gy.youmonst);
    boolean u_def = (mdef == &gy.youmonst);
    boolean was_u = FALSE, dismount = FALSE;
    struct obj *wep = weapon_used ? (u_agr ? uwep : MON_WEP(magr))
                                  : (struct obj *) 0;

    if (wep && is_art(wep, ART_OGRESMASHER))
        chance = 2;

    if (rn2(chance))
        return FALSE;

    /* only certain attacks qualify for knockback */
    if (!((mattk->adtyp == AD_PHYS)
          && (mattk->aatyp == AT_CLAW
              || mattk->aatyp == AT_KICK
              || mattk->aatyp == AT_BUTT
              || mattk->aatyp == AT_WEAP)))
        return FALSE;

    /* don't knockback if attacker also wants to grab or engulf */
    if (attacktype(magr->data, AT_ENGL)
        || attacktype(magr->data, AT_HUGS)
        || sticks(magr->data))
        return FALSE;

    /* decide where the first step will place the target; not accurate
       for being knocked out of saddle but doesn't need to be; used for
       test_move() and for message before actual hurtle */
    defx = u_def ? u.ux : mdef->mx;
    defy = u_def ? u.uy : mdef->my;
    dx = sgn(defx - (u_agr ? u.ux : magr->mx));
    dy = sgn(defy - (u_agr ? u.uy : magr->my));

    /* can't move most targets into or out of a doorway diagonally */
    if (u_def) {
        if (!test_move(defx, defy, dx, dy, TEST_MOVE))
            return FALSE;
    } else {
        /* subset of test_move() */
        if (!isok(defx + dx, defy + dy))
            return FALSE;
        if (IS_DOOR(levl[defx][defy].typ)
            && (defx - magr->mx && defy - magr->my)
            && !doorless_door(defx, defy))
            return FALSE;
    }

    /* if hero is stuck to a cursed saddle, knock the steed back */
    if (u_def && u.usteed) {
        if ((otmp = which_armor(u.usteed, W_SADDLE)) != 0 && otmp->cursed) {
            mdef = u.usteed;
            was_u = TRUE;
            u_def = FALSE;
        } else {
            dismount = TRUE; /* saddle is not cursed; knock hero out of it */
        }
    }

    /* monsters must be alive */
    if ((!u_agr && DEADMONSTER(magr))
        || (!u_def && DEADMONSTER(mdef)))
        return FALSE;

    /* attacker must be much larger than defender */
    if (!(magr->data->msize > (mdef->data->msize + 1)))
        return FALSE;

    /* no knockback with a flimsy or non-blunt weapon */
    if (wep && (is_flimsy(wep) || !is_blunt_weapon(wep)))
        return FALSE;

    /* needs a solid physical hit */
    if (unsolid(magr->data))
        return FALSE;

    /* the attack must have hit */
    /* mon-vs-mon code path doesn't set up hitflags */
    if ((u_agr || u_def) && !(*hitflags & M_ATTK_HIT))
        return FALSE;

    /* steadfast defender cannot be pushed around */
    if (m_is_steadfast(mdef)) {
        if (u_def || (u.usteed && mdef == u.usteed)) {
            mdefbuf[0] = '\0';
            if (u.usteed)
                Snprintf(mdefbuf, sizeof mdefbuf, "and %s ",
                         y_monnam(u.usteed));
            You("%sdon't budge.", mdefbuf);
        } else if (canseemon(mdef)) {
            pline("%s doesn't budge.", Monnam(mdef));
        }
        return FALSE;
    }

    /* subtly vary the message text if monster won't actually move */
    knockedhow = dismount ? "out of your saddle"
                 : will_hurtle(mdef, defx + dx, defy + dy) ? "backward"
                   : "back";

    /* give the message */
    if (u_def || canseemon(mdef)) {
        Strcpy(magrbuf, u_agr ? "You" : Monnam(magr));
        Strcpy(mdefbuf, (u_def || was_u) ? "you" : y_monnam(mdef));
        if (was_u)
            Snprintf(eos(mdefbuf), sizeof mdefbuf - strlen(mdefbuf),
                     " and %s", y_monnam(u.usteed));
        /*
         * uhitm: You knock the gnome back with a powerful blow!
         * mhitu: The red dragon knocks you back with a forceful blow!
         * mhitm: The fire giant knocks the gnome back with a forceful strike!
         */
        pline("%s %s %s %s with a %s %s!",
              magrbuf, vtense(magrbuf, "knock"), mdefbuf, knockedhow,
              rn2(2) ? "forceful" : "powerful", rn2(2) ? "blow" : "strike");
    } else if (u_agr) {
        /* hero knocks unseen foe back; noticed by touch */
        You_feel("%s be knocked %s!", some_mon_nam(mdef), knockedhow);
    }

    if (u.ustuck && (u_def || u_agr))
        unstuck(u.ustuck);

    /* do the actual knockback effect */
    if (u_def) {
        if (dismount) {
            /* normally u.dx,u.dy indicates the direction hero is throwing,
               zapping, &c but here it is used to pass preferred direction
               for dismount to dismount_steed (for DISMOUNT_KNOCKED only) */
            u.dx = dx;
            u.dy = dy;
            dismount_steed(DISMOUNT_KNOCKED);
        } else {
            hurtle(dx, dy, knockdistance, FALSE);
            *hitflags |= M_ATTK_HIT;
        }
        set_apparxy(magr); /* update magr's idea of where you are */
        if (!Stunned && !rn2(4))
            make_stunned((long) (knockdistance + 1), TRUE); /* 2 or 3 */
    } else {
        mhurtle(mdef, dx, dy, knockdistance);
        if (!u_agr)
            *hitflags |= M_ATTK_HIT;
        if (DEADMONSTER(mdef)) {
            if (!was_u)
                *hitflags |= M_ATTK_DEF_DIED;
        } else if (!rn2(4)) {
            if (!(mon_resists_stun(mdef) || defended(mdef, AD_STUN)))
                mdef->mstun = 1;
            /* if steed and hero were knocked back, update attacker's idea
               of where hero is */
            if (mdef == u.usteed)
                set_apparxy(magr);
        }
    }
    if (!u_agr) {
        if (DEADMONSTER(magr))
            *hitflags |= M_ATTK_AGR_DIED;
    }

    return TRUE;
}

/* attack monster as a monster; returns True if mon survives;
   'as' picks whose attack list to use: NON_PM means the hero's
   current form, otherwise the given monster index (used for the
   extra natural attacks of unpolymorphed racial heroes) */
staticfn boolean
hmonas(struct monst *mon, int as, boolean weapon_attacks)
{
    struct attack *mattk, alt_attk;
    struct obj *weapon, **originalweapon;
    boolean altwep = FALSE, weapon_used = !weapon_attacks,
            odd_claw = TRUE;
    int i, tmp, dieroll, armorpenalty, sum[NATTK],
        dhit = 0, attknum = 0, multi_claw = 0, multi_weap = 0;
    boolean monster_survived;

    /* not used here but umpteen mhitm_ad_xxxx() need this */
    gv.vis = (canseemon(mon) || m_next2u(mon));

    /* with just one touch/claw/weapon attack, both rings matter;
       with more than one, alternate right and left when checking
       whether silver ring causes successful hit */
    for (i = 0; i < NATTK; i++) {
        sum[i] = M_ATTK_MISS;
        mattk = (as != NON_PM) ? &mons[as].mattk[i]
                : getmattk(&gy.youmonst, mon, i, sum, &alt_attk);
        if (mattk->aatyp == AT_WEAP)
            ++multi_weap;
        if (mattk->aatyp == AT_WEAP
            || mattk->aatyp == AT_CLAW || mattk->aatyp == AT_TUCH)
            ++multi_claw;
    }
    multi_claw = (multi_claw > 1); /* switch from count to yes/no */
    gt.twohits = 0;

    gs.skipdrin = FALSE; /* [see mattackm(mhitm.c)] */

    for (i = 0; i < NATTK; i++) {
        /* sum[i] = M_ATTK_MISS; -- now done above */

        /* target might have been knocked back so no longer in range, or an
           engulfing vampshifted fog cloud killed and reverted to vampire
           that's placed at another spot (hero occupies mon's first spot) */
        if (i > 0 && (m_at(gb.bhitpos.x, gb.bhitpos.y) != mon
                      || DEADMONSTER(mon)))
            continue;

        mattk = (as != NON_PM) ? &mons[as].mattk[i]
                : getmattk(&gy.youmonst, mon, i, sum, &alt_attk);
        if (gs.skipdrin && mattk->aatyp == AT_TENT && mattk->adtyp == AD_DRIN)
            continue;
        weapon = 0;
        switch (mattk->aatyp) {
        case AT_WEAP:
            /* if (!uwep) goto weaponless; */
            if (!weapon_attacks)
                continue;
 use_weapon:
            odd_claw = !odd_claw; /* see case AT_CLAW,AT_TUCH below */
            /* if we've already hit with a two-handed weapon, we don't
               get to make another weapon attack (note:  monsters who
               use weapons do not have this restriction, but they also
               never have the opportunity to use two weapons) */
            if (weapon_used && (sum[i - 1] > M_ATTK_MISS)
                && uwep && bimanual(uwep))
                continue;
            /* Certain monsters don't use weapons when encountered as enemies,
             * but players who polymorph into them have hands or claws and
             * thus should be able to use weapons.  This shouldn't prohibit
             * the use of most special abilities, either.
             * If monster has multiple claw attacks, only one can use weapon.
             */
            weapon_used = TRUE;
            /* Potential problem: if the monster gets multiple weapon attacks,
             * we currently allow the player to get each of these as a weapon
             * attack.  Is this really desirable?
             */
            /* approximate two-weapon mode; known_hitum() -> hmon() -> &c
               might destroy the weapon argument, but it might also already
               be Null, and we want to track that for passive() */
            originalweapon = (altwep && uswapwep) ? &uswapwep : &uwep;
            if (uswapwep /* set up 'altwep' flag for next iteration */
                /* only consider secondary when wielding one-handed primary */
                && uwep && (uwep->oclass == WEAPON_CLASS || is_weptool(uwep))
                && !bimanual(uwep)
                /* only switch if not wearing shield;
                   shield limitation is iffy since still get extra swings
                   if polyform has them, but it matches twoweap behavior */
                && !uarms
                /* opposite-aligned artifacts can't be twoweaponed */
                && !((is_lawful_artifact(uwep)
                      && is_chaotic_artifact(uswapwep))
                     || (is_chaotic_artifact(uwep)
                         && is_lawful_artifact(uswapwep)))
                /* the Wand of Orcus can't be twoweaponed with any
                   other artifact */
                && !(is_art(uwep, ART_WAND_OF_ORCUS)
                     && uswapwep->oartifact)
                /* the Wand of Orcus won't be held second to anything */
                && !(is_art(uswapwep, ART_WAND_OF_ORCUS)
                     && (is_weptool(uwep) || uwep->oclass == WEAPON_CLASS))
                /* only switch to uswapwep if it's a weapon */
                && (uswapwep->oclass == WEAPON_CLASS || is_weptool(uswapwep))
                /* only switch if uswapwep is not bow, arrows, or darts */
                && !(is_launcher(uswapwep) || is_ammo(uswapwep)
                     || is_missile(uswapwep)) /* dart, shuriken, boomerang */
                /* and not two-handed and not incapable of being wielded */
                && !bimanual(uswapwep)
                && !Hate_material(uswapwep->material))
                altwep = !altwep; /* toggle for next attack */
            weapon = *originalweapon;
            if (!weapon) /* no need to go beyond no-gloves to rings; not ...*/
                originalweapon = &uarmg; /*... subject to erosion damage */

            tmp = find_roll_to_hit(mon, AT_WEAP, weapon, &attknum,
                                   &armorpenalty);
            mon_maybe_unparalyze(mon);
            dieroll = rnd(20);
            dhit = (tmp > dieroll || u.uswallow);
            if (multi_weap > 1)
                ++gt.twohits;
            /* caller must set gb.bhitpos */
            monster_survived = known_hitum(mon, weapon, &dhit, tmp,
                                           armorpenalty, mattk, dieroll);
            /* originalweapon points to an equipment slot which might
               now be empty if the weapon was destroyed during the hit;
               passive(,weapon,...) won't call passive_obj() in that case */
            weapon = *originalweapon; /* might receive passive erosion */
            if (!monster_survived) {
                /* enemy dead, before any special abilities used */
                sum[i] = M_ATTK_DEF_DIED;
                break;
            } else
                sum[i] = dhit ? M_ATTK_HIT : M_ATTK_MISS;
            /* might be a worm that gets cut in half; if so, early return */
            if (m_at(u.ux + u.dx, u.uy + u.dy) != mon) {
                i = NATTK; /* skip additional attacks */
                /* proceed with uswapwep->cursed check, then exit loop */
                goto passivedone;
            }
            /* Do not print "You hit" message; known_hitum already did it. */
            if (dhit && mattk->adtyp != AD_SPEL && mattk->adtyp != AD_PHYS)
                sum[i] = damageum(mon, mattk, 0);
            break;
        case AT_CLAW:
            if (uwep && !cantwield(gy.youmonst.data) && !weapon_used)
                goto use_weapon;
            FALLTHROUGH;
            /*FALLTHRU*/
        case AT_TUCH:
            if (uwep && gy.youmonst.data->mlet == S_LICH && !weapon_used)
                goto use_weapon;
            FALLTHROUGH;
            /*FALLTHRU*/
        case AT_KICK:
            if (mattk->aatyp == AT_KICK && mtrapped_in_pit(&gy.youmonst))
                continue;
            FALLTHROUGH;
            /*FALLTHRU*/
        case AT_BITE:
        case AT_STNG:
        case AT_BUTT:
        case AT_TENT:
        /*weaponless:*/
            /* don't eat brains as a racial illithid or as a druid in
               wild shape when it would be particularly detrimental */
            if ((druid_form
                 || maybe_polyd(is_illithid(gy.youmonst.data),
                                Race_if(PM_ILLITHID)))
                && mattk->aatyp == AT_TENT
                && (((touch_petrifies(mon->data)
                      || mon->data == &mons[PM_MEDUSA])
                     && !Stone_resistance)
                    || is_rider(mon->data)
                    || noncorporeal(mon->data)
                    || mon->data == &mons[PM_GREEN_SLIME]
                    /* illithid-on-illithid cannibalism is not a worry
                       since their minds are psionically shielded */
                    || (ismnum(u.ulycn)
                        && were_beastie(monsndx(mon->data)) == u.ulycn
                        && !Role_if(PM_CAVE_DWELLER))
                    || (how_resistant(DISINT_RES) < 50
                        && (mon->data == &mons[PM_BLACK_DRAGON]
                            || mon->data == &mons[PM_ANTIMATTER_VORTEX]))))
                break;
            /* never eat brains while engulfed */
            if (maybe_polyd(is_illithid(gy.youmonst.data),
                            Race_if(PM_ILLITHID))
                && mattk->aatyp == AT_TENT && u.uswallow)
                break;
            /* don't bite as one of the undead races or as a druid in
               wild shape when it would be particularly detrimental */
            if ((druid_form || vampire_form
                 || maybe_polyd(is_zombie(gy.youmonst.data),
                                Race_if(PM_DRAUGR))
                 || maybe_polyd(is_vampire(gy.youmonst.data),
                                Race_if(PM_VAMPIRE)))
                && mattk->aatyp == AT_BITE
                && (((touch_petrifies(mon->data)
                      || mon->data == &mons[PM_MEDUSA])
                     && !Stone_resistance)
                    || is_rider(mon->data)
                    || noncorporeal(mon->data)
                    || mon->data == &mons[PM_GREEN_SLIME]
                    || (how_resistant(DISINT_RES) < 50
                        && (mon->data == &mons[PM_BLACK_DRAGON]
                            || mon->data == &mons[PM_ANTIMATTER_VORTEX]))))
                break;
            /* never eat brains while engulfed */
            if (maybe_polyd(is_zombie(gy.youmonst.data),
                            Race_if(PM_DRAUGR))
                && mattk->aatyp == AT_BITE && u.uswallow)
                break;
            /* small, unimpaired animals can evade a zombie's bite */
            if (maybe_polyd(is_zombie(gy.youmonst.data),
                            Race_if(PM_DRAUGR))
                && mattk->aatyp == AT_BITE
                && mon->data->msize <= MZ_SMALL
                && is_animal(mon->data)
                && !(mon->mfrozen || mon->mstone
                     || mon->mconf || mon->mstun) && rn2(3)) {
                pline("%s nimbly %s your bite!", Monnam(mon),
                      rn2(2) ? "dodges" : "evades");
                break;
            }
            /* don't sting as a druid in wild shape or as a demon-race
               hero when it would be particularly detrimental */
            if ((druid_form || (!Upolyd && Race_if(PM_DEMON)))
                && mattk->aatyp == AT_STNG
                && ((touch_petrifies(mon->data) && !Stone_resistance)
                    || noncorporeal(mon->data)
                    || (how_resistant(DISINT_RES) < 50
                        && (mon->data == &mons[PM_BLACK_DRAGON]
                            || mon->data == &mons[PM_ANTIMATTER_VORTEX]))))
                break;
            tmp = find_roll_to_hit(mon, mattk->aatyp, (struct obj *) 0,
                                   &attknum, &armorpenalty);
            mon_maybe_unparalyze(mon);
            dieroll = rnd(20);
            dhit = (tmp > dieroll || u.uswallow);
            if (dhit) {
                int compat, specialdmg;
                struct obj *hated_obj = (struct obj *) 0;
                const char *verb = 0; /* verb or body part */

                if (!u.uswallow
                    && (compat = could_seduce(&gy.youmonst, mon, mattk))
                       != 0) {
                    You("%s %s %s.",
                        (mon->mcansee && haseyes(mon->data)) ? "smile at"
                                                             : "talk to",
                        mon_nam(mon),
                        (compat == 2) ? "engagingly" : "seductively");
                    /* doesn't anger it; no wakeup() */
                    sum[i] = damageum(mon, mattk, 0);
                    break;
                }
                wakeup(mon, TRUE);

                specialdmg = 0; /* blessed and/or silver bonus */
                switch (mattk->aatyp) {
                case AT_CLAW:
                case AT_TUCH:
                    /* verb=="claws" may be overridden below */
                    verb = (mattk->aatyp == AT_TUCH) ? "touch" : "claws";
                    /* decide if material-hater will be hit by ring(s);
                       for 'multi_claw' where attacks alternate right/left,
                       assume 'even' claw or touch attacks use dominant hand
                       or paw, 'odd' ones use non-dominant hand for ring
                       interaction; even vs odd is based on actual attacks
                       rather than on index into mon->dat->mattk[] so that
                       {bite,claw,claw} instead of {claw,claw,bite} doesn't
                       make poly'd hero mysteriously switch handedness */
                    odd_claw = !odd_claw;
                    specialdmg = special_dmgval(&gy.youmonst, mon,
                                                W_ARMG
                                                | ((odd_claw || !multi_claw)
                                                   ? W_RINGL : 0L)
                                                | ((!odd_claw || !multi_claw)
                                                   ? W_RINGR : 0L),
                                                &hated_obj);
                    break;
                case AT_TENT:
                    /* assumes mind flayer's tentacles-on-head rather
                       than sea monster's tentacle-as-arm */
                    verb = "tentacles";
                    break;
                case AT_KICK:
                    verb = "kick";
                    specialdmg = special_dmgval(&gy.youmonst, mon, W_ARMF,
                                                &hated_obj);
                    break;
                case AT_BUTT:
                    verb = "head butt"; /* mbodypart(mon,HEAD)=="head" */
                    /* hypothetical; if any form with a head-butt attack
                       could wear a helmet, it would hit shades when
                       wearing a blessed (or silver) one */
                    specialdmg = special_dmgval(&gy.youmonst, mon, W_ARMH,
                                                &hated_obj);
                    break;
                case AT_BITE:
                    verb = "bite";
                    break;
                case AT_STNG:
                    verb = "sting";
                    break;
                default:
                    verb = "hit";
                    break;
                }
                if (mon->data == &mons[PM_SHADE] && !specialdmg) {
                    if (!strcmp(verb, "hit")
                        || (mattk->aatyp == AT_CLAW && humanoid(mon->data)))
                        verb = "attack";
                    Your("%s %s harmlessly through %s.",
                         verb, vtense(verb, "pass"), mon_nam(mon));
                } else {
                    /* either not a shade or no special silver/blessed damage,
                       other unsolid monsters are immune to AT_TUCH+AD_WRAP */
                    if (failed_grab(&gy.youmonst, mon, mattk))
                        break; /* miss; message already given */

                    if (mattk->aatyp == AT_TENT) {
                        Your("tentacles suck %s.", mon_nam(mon));
                    } else {
                        if (mattk->aatyp == AT_CLAW)
                            verb = "hit"; /* not "claws" */
                        You("%s %s.", verb, mon_nam(mon));
                        if (hated_obj && flags.verbose)
                            searmsg(&gy.youmonst, mon, hated_obj, FALSE);
                    }
                    sum[i] = damageum(mon, mattk, specialdmg);
                }
            } else { /* !dhit */
                missum(mon, tmp, dieroll, mattk,
                       (tmp + armorpenalty > dieroll));
            }
            break;

        case AT_HUGS: {
            int specialdmg;
            struct obj *hated_obj = (struct obj *) 0;
            boolean byhand = hug_throttles(&mons[u.umonnum]), /* rope golem */
                    unconcerned = (byhand && !can_be_strangled(mon));

            if (sticks(mon->data) || u.uswallow || gn.notonhead
                || (byhand && (uwep || !has_head(mon->data)))) {
                /* can't hold a holder due to subsequent ambiguity over
                   who is holding whom; can't hug engulfer from inside;
                   can't hug a worm tail (would immobilize entire worm!);
                   byhand: can't choke something that lacks a head;
                   not allowed to make a choking hug if wielding a weapon
                   (but might have grabbed w/o weapon, then wielded one,
                   and may even be attacking a different monster now) */
                if (byhand && uwep && u.ustuck
                    && !(sticks(u.ustuck->data) || u.uswallow))
                    uunstick();
                continue; /* not 'break'; bypass passive counter-attack */
            }
            /* automatic if prev two attacks succeed, or if
               already grabbed in a previous attack */
            dhit = 1;
            wakeup(mon, TRUE);
            /* choking hug/throttling grab uses hands (gloves or rings);
               normal hug uses outermost of cloak/suit/shirt */
            specialdmg = special_dmgval(&gy.youmonst, mon,
                                        byhand ? (W_ARMG | W_RINGL | W_RINGR)
                                               : (W_ARMC | W_ARM | W_ARMU),
                                        &hated_obj);
            if (unconcerned) {
                /* strangling something which can't be strangled */
                if (mattk != &alt_attk) {
                    alt_attk = *mattk;
                    mattk = &alt_attk;
                }
                /* change damage to 1d1; not strangling but still
                   doing [minimal] physical damage to victim's body */
                mattk->damn = mattk->damd = 1;
                /* don't give 'unconcerned' feedback if there is extra damage
                   or if it is nearly destroyed or if creature doesn't have
                   the mental ability to be concerned in the first place */
                if (specialdmg || mindless(mon->data)
                    || mon->mhp <= 1 + max(u.udaminc, 1))
                    unconcerned = FALSE;
            }
            if (mon->data == &mons[PM_SHADE]) {
                const char *verb = byhand ? "grasp" : "hug";

                /* hugging a shade; successful if blessed outermost armor
                   for normal hug, or blessed gloves or silver ring(s) for
                   choking hug; deals damage but never grabs hold */
                if (specialdmg) {
                    You("%s %s%s", verb, mon_nam(mon), exclam(specialdmg));
                    if (hated_obj && flags.verbose)
                        searmsg(&gy.youmonst, mon, hated_obj, FALSE);
                    sum[i] = damageum(mon, mattk, specialdmg);
                } else {
                    Your("%s passes harmlessly through %s.",
                         verb, mon_nam(mon));
                }
                break;
            }
            /* can't grab unsolid creatures (checked after shade handling) */
            if (failed_grab(&gy.youmonst, mon, mattk))
                break;
            /* hug attack against ordinary foe */
            if (mon == u.ustuck) {
                pline("%s is being %s%s.", Monnam(mon),
                      byhand ? "throttled" : "crushed",
                      /* extra feedback for non-breather being choked */
                      unconcerned ? " but doesn't seem concerned" : "");
                if (hated_obj && flags.verbose)
                    searmsg(&gy.youmonst, mon, hated_obj, FALSE);
                sum[i] = damageum(mon, mattk, specialdmg);
            } else if (i >= 2 && (sum[i - 1] > M_ATTK_MISS)
                       && (sum[i - 2] > M_ATTK_MISS)) {
                /* in case we're hugging a new target while already
                   holding something else; yields feedback
                   "<u.ustuck> is no longer in your clutches" */
                if (u.ustuck && u.ustuck != mon)
                    uunstick();
                You("grab %s!", mon_nam(mon));
                set_ustuck(mon);
                if (hated_obj && flags.verbose)
                    searmsg(&gy.youmonst, mon, hated_obj, FALSE);
                sum[i] = damageum(mon, mattk, specialdmg);
            }
            break; /* AT_HUGS */
        }

        case AT_EXPL: /* automatic hit if next to */
            dhit = -1;
            wakeup(mon, TRUE);
            You("explode!");
            sum[i] = explum(mon, mattk);
            break;

        case AT_ENGL:
            tmp = find_roll_to_hit(mon, mattk->aatyp, (struct obj *) 0,
                                   &attknum, &armorpenalty);
            mon_maybe_unparalyze(mon);
            /* the roll is captured rather than left inline because the
               miss path now needs it, and this arm never wrote dieroll.
               master draws a second, unrelated rnd(20) for the same
               purpose, so its ladder judges the miss by the wrong roll */
            if ((dhit = (tmp > (dieroll = rnd(20 + i))))) {
                wakeup(mon, TRUE);
                /* can't engulf unsolid creatures */
                if (mon->data == &mons[PM_SHADE]) {
                    /* no specialdmg check needed */
                    Your("attempt to surround %s is harmless.", mon_nam(mon));
                } else if (failed_grab(&gy.youmonst, mon, mattk)) {
                    ; /* non-shade miss; message already given */
                } else {
                    sum[i] = gulpum(mon, mattk);
                    if (sum[i] == M_ATTK_DEF_DIED
                        && (mon->data->mlet == S_ZOMBIE
                            || mon->data->mlet == S_MUMMY)
                        && rn2(5) && !Sick_resistance) {
                        You_feel("%ssick.", (Sick) ? "very " : "");
                        mdamageu(mon, rnd(8));
                    }
                }
            } else {
                missum(mon, tmp, dieroll, mattk, FALSE);
            }
            break;

        case AT_MAGC:
            /* No check for uwep; if wielding nothing we want to
             * do the normal 1-2 points bare hand damage...
             */
            if ((gy.youmonst.data->mlet == S_KOBOLD
                 || gy.youmonst.data->mlet == S_ORC
                 || gy.youmonst.data->mlet == S_HUMAN
                 || gy.youmonst.data->mlet == S_GIANT
                 || gy.youmonst.data->mlet == S_VAMPIRE
                 || gy.youmonst.data->mlet == S_GNOME) && !weapon_used)
                goto use_weapon;
            /* elemental casts (hellfire and kin) work when the hero
               is polymorphed into such a caster; the innate
               AD_SPEL/AD_CLRC monster spell lists have no
               hero-as-caster implementation yet */
            if (mattk->adtyp == AD_FIRE || mattk->adtyp == AD_COLD
                || mattk->adtyp == AD_ACID || mattk->adtyp == AD_MAGM) {
                sum[i] = castum(mon, mattk);
                continue;
            }
            FALLTHROUGH;
            /*FALLTHRU*/

        case AT_NONE:
        case AT_BOOM:
            continue;
        /* Not break--avoid passive attacks from enemy */

        case AT_BREA:
        case AT_SPIT:
        case AT_GAZE: /* all done using #monster command */
            dhit = 0;
            break;

        default: /* Strange... */
            impossible("strange attack of yours (%d)", mattk->aatyp);
        }
        if (dhit == -1) {
            u.mh = -1; /* dead in the current form */
            rehumanize();
        }
        if (sum[i] == M_ATTK_DEF_DIED) {
            /* defender dead */
            (void) passive(mon, weapon, 1, 0, mattk->aatyp, FALSE);
        } else {
            (void) passive(mon, weapon, (sum[i] != M_ATTK_MISS), 1,
                           mattk->aatyp, FALSE);
        }

        if (mhitm_knockback(&gy.youmonst, mon, mattk, &sum[i], weapon_used))
            break;

        /* don't use sum[i] beyond this point;
           'i' will be out of bounds if we get here via 'goto' */
 passivedone:
        /* when using dual weapons, cursed secondary weapon doesn't weld,
           it gets dropped; do the same when multiple AT_WEAP attacks
           simulate twoweap */
        if (uswapwep && weapon == uswapwep && weapon->cursed) {
            drop_uswapwep();
            break; /* don't proceed with additional attacks */
        }
        /* stop attacking if defender has died;
           needed to defer this until after uswapwep->cursed check */
        if (DEADMONSTER(mon))
            break;
        if (!Upolyd)
            break; /* No extra attacks if no longer a monster */
        if (gm.multi < 0)
            break; /* If paralyzed while attacking, i.e. floating eye */
    }

    gv.vis = FALSE; /* reset */
    gt.twohits = 0;
    /* return value isn't used, but make it match hitum()'s */
    return !DEADMONSTER(mon);
}

/*      Special (passive) attacks on you by monsters done here.
 */
int
passive(
    struct monst *mon,
    struct obj *weapon, /* uwep or uswapwep or uarmg or uarmf or Null */
    boolean mhitb,
    boolean maliveb,
    uchar aatyp,
    boolean wep_was_destroyed)
{
    struct permonst *ptr = mon->data;
    /* passive attacks come from the racial overlay's matrix */
    struct attack *mattk = has_erac(mon) ? ERAC(mon)->mattk : ptr->mattk;
    struct obj *passive_armor;
    int i, t, tmp;
    int mhit = mhitb ? M_ATTK_HIT : M_ATTK_MISS;
    int malive = maliveb ? M_ATTK_HIT : M_ATTK_MISS;

    /* a vampire that just bit gets the side effects of tasting the
       victim's flesh or blood */
    if (maybe_polyd(is_vampire(gy.youmonst.data), Race_if(PM_VAMPIRE))
        && aatyp == AT_BITE && mhitb) {
        if (bite_monster(mon))
            return M_ATTK_AGR_DIED; /* lifesaved */
    }

    for (i = 0;; i++) {
        if (i >= NATTK)
            return (malive | mhit); /* no passive attacks */
        if (mattk[i].aatyp == AT_NONE)
            break; /* try this one */
    }
    /* Note: tmp not always used */
    if (mattk[i].damn)
        tmp = d((int) mattk[i].damn, (int) mattk[i].damd);
    else if (mattk[i].damd)
        tmp = d((int) mon->m_lev + 1, (int) mattk[i].damd);
    else
        tmp = 0;

    /*  These affect you even if they just died.
     */
    switch (mattk[i].adtyp) {
    case AD_FIRE:
        if (mhitb && !mon->mcan && weapon) {
            if (aatyp == AT_KICK) {
                if (uarmf && !rn2(6))
                    (void) erode_obj(uarmf, xname(uarmf), ERODE_BURN,
                                     EF_GREASE | EF_VERBOSE);
            } else if (aatyp == AT_WEAP || aatyp == AT_CLAW
                       || aatyp == AT_MAGC || aatyp == AT_TUCH)
                (void) passive_obj(mon, weapon, &(mattk[i]));
        }
        break;
    case AD_ACID:
        if (mhitb && rn2(2)) {
            if (Blind || !flags.verbose)
                You("are splashed!");
            else
                You("are splashed by %s %s!", s_suffix(mon_nam(mon)),
                    hliquid("acid"));

            if (!Acid_resistance) {
                mdamageu(mon, tmp);
                monstunseesu(M_SEEN_ACID);
            } else {
                monstseesu(M_SEEN_ACID);
            }
            if (!rn2(30))
                erode_armor(&gy.youmonst, ERODE_CORRODE);
        }
        if (mhitb && weapon) {
            if (aatyp == AT_KICK) {
                if (uarmf && !rn2(6))
                    (void) erode_obj(uarmf, xname(uarmf), ERODE_CORRODE,
                                     EF_GREASE | EF_VERBOSE);
            } else if (aatyp == AT_WEAP || aatyp == AT_CLAW
                       || aatyp == AT_MAGC || aatyp == AT_TUCH)
                (void) passive_obj(mon, weapon, &(mattk[i]));
        }
        exercise(A_STR, FALSE);
        break;
    case AD_STON:
        if (mhitb) { /* successful attack */
            long protector = attk_protection((int) aatyp);

            /* hero using monsters' AT_MAGC attack is hitting hand to
               hand rather than casting a spell */
            if (aatyp == AT_MAGC)
                protector = W_ARMG;

            if (protector == 0L /* no protection */
                || (protector == W_ARMG && !uarmg
                    && !uwep && !wep_was_destroyed)
                || (protector == W_ARMF && !uarmf)
                || (protector == W_ARMH && !uarmh)
                || (protector == (W_ARMC | W_ARMG) && (!uarmc || !uarmg))) {
                if (!Stone_resistance
                    && !(poly_when_stoned(gy.youmonst.data)
                         && polymon(PM_STONE_GOLEM))) {
                    done_in_by(mon, STONING); /* "You turn to stone..." */
                    return M_ATTK_DEF_DIED;
                }
            }
        }
        break;
    case AD_RUST:
        if (mhitb && !mon->mcan && weapon) {
            if (aatyp == AT_KICK) {
                if (uarmf)
                    (void) erode_obj(uarmf, xname(uarmf), ERODE_RUST,
                                     EF_GREASE | EF_VERBOSE);
            } else if (aatyp == AT_WEAP || aatyp == AT_CLAW
                       || aatyp == AT_MAGC || aatyp == AT_TUCH)
                (void) passive_obj(mon, weapon, &(mattk[i]));
        }
        break;
    case AD_CORR:
        if (mhitb && !mon->mcan && weapon) {
            if (aatyp == AT_KICK) {
                if (uarmf)
                    (void) erode_obj(uarmf, xname(uarmf), ERODE_CORRODE,
                                     EF_GREASE | EF_VERBOSE);
            } else if (aatyp == AT_WEAP || aatyp == AT_CLAW
                       || aatyp == AT_MAGC || aatyp == AT_TUCH)
                (void) passive_obj(mon, weapon, &(mattk[i]));
        }
        break;
    case AD_DISN:
        if (mhitb && !mon->mcan) { /* successful attack */
            long protector = attk_protection((int) aatyp);
            int chance = (mon->data == &mons[PM_ANTIMATTER_VORTEX])
                             ? rn2(10) : rn2(20);

            tmp = rnd(8) + 1;
            /* hero using monsters' AT_MAGC attack is hitting hand to
               hand rather than casting a spell */
            if (aatyp == AT_MAGC)
                protector = W_ARMG;

            if (protector == 0L /* no protection */
                || (protector == W_ARMG && !uarmg
                    && !uwep && !wep_was_destroyed)
                || (protector == W_ARMF && !uarmf)
                || (protector == W_ARMH && !uarmh)
                || (protector == (W_ARMC | W_ARMG) && (!uarmc || !uarmg))) {
                if (how_resistant(DISINT_RES) == 100) {
                    if (!rn2(3))
                        You("are unaffected by %s %s.",
                            s_suffix(mon_nam(mon)),
                            mon->data == &mons[PM_ANTIMATTER_VORTEX]
                                ? "form" : "hide");
                    monstseesu(M_SEEN_DISINT);
                    stop_occupation();
                } else {
                    if (chance) {
                        You("partially disintegrate!");
                        tmp = resist_reduce(tmp, DISINT_RES);
                        mdamageu(mon, tmp);
                    } else {
                        if (how_resistant(DISINT_RES) < 50) {
                            pline("%s deadly %s disintegrates you!",
                                  s_suffix(Monnam(mon)),
                                  mon->data == &mons[PM_ANTIMATTER_VORTEX]
                                      ? "form" : "hide");
                            u.ugrave_arise = -3;
                            svk.killer.format = NO_KILLER_PREFIX;
                            Sprintf(svk.killer.name, "disintegrated by %s",
                                    an(l_monnam(mon)));
                            done(DISINTEGRATED);
                            return M_ATTK_DEF_DIED;
                        } else {
                            You("partially disintegrate!");
                            tmp = resist_reduce(tmp, DISINT_RES);
                            mdamageu(mon, tmp);
                        }
                    }
                }
            }
            /* odds of obj disintegrating handled in passive_obj()
               except for attack type AT_KICK */
            if (aatyp == AT_KICK) {
                if (uarmf
                    && (mon->data == &mons[PM_ANTIMATTER_VORTEX]
                            ? !rn2(3) : !rn2(6))) {
                    if (rn2(2)
                        && (uarmf->oerodeproof || is_supermaterial(uarmf)))
                        pline("%s being disintegrated!",
                              Yobjnam2(uarmf, "resist"));
                    else
                        (void) disintegrate_arm(uarmf);
                }
            } else if (aatyp == AT_WEAP || aatyp == AT_CLAW
                       || aatyp == AT_MAGC || aatyp == AT_TUCH) {
                (void) passive_obj(mon, weapon, &(mattk[i]));
            }
        }
        break;
    case AD_MAGM:
        /* wrath of gods for attacking Oracle */
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
            monstseesu(M_SEEN_MAGR);
            pline("A hail of magic missiles narrowly misses you!");
        } else {
            You("are hit by magic missiles appearing from thin air!");
            mdamageu(mon, tmp);
            monstunseesu(M_SEEN_MAGR);
        }
        break;
    case AD_ENCH: /* KMH -- remove enchantment (disenchanter) */
        if (mhitb) {
            if (aatyp == AT_KICK) {
                if (!weapon)
                    break;
            } else if (aatyp == AT_BITE || aatyp == AT_BUTT
                       || (aatyp >= AT_STNG && aatyp < AT_WEAP)) {
                break; /* no object involved */
            } else {
                /*
                 * TODO:  #H2668 - if hitting with a ring that has a
                 * positive enchantment, it ought to be subject to
                 * having that enchantment reduced.  But we don't have
                 * sufficient information here to know which hand/ring
                 * has delivered a weaponless blow.
                 */
                ;
            }
            (void) passive_obj(mon, weapon, &(mattk[i]));
        }
        break;
    case AD_CNCL:
        if (mhitb && !rn2(6)) {
            (void) cancel_monst(&gy.youmonst, weapon, FALSE, TRUE, FALSE);
        }
        break;
    default:
        break;
    }

    /*  These only affect you if they still live.
     */
    if (malive && !mon->mcan && rn2(3)) {
        switch (mattk[i].adtyp) {
        case AD_PLYS:
            if (ptr == &mons[PM_FLOATING_EYE]) {
                if (!canseemon(mon)) {
                    break;
                }
                if (mon->mcansee) {
                    if (ureflects("%s gaze is reflected by your %s.",
                                  s_suffix(Monnam(mon)))) {
                        ;
                    } else if (Hallucination && rn2(4)) {
                        /* [it's the hero who should be getting paralyzed
                           and isn't; this message describes the monster's
                           reaction rather than the hero's escape] */
                        pline("%s looks %s%s.", Monnam(mon),
                              !rn2(2) ? "" : "rather ",
                              !rn2(2) ? "numb" : "stupefied");
                    } else if (Free_action) {
                        You("momentarily stiffen under %s gaze!",
                            s_suffix(mon_nam(mon)));
                    } else {
                        You("are frozen by %s gaze!", s_suffix(mon_nam(mon)));
                        nomul((ACURR(A_WIS) > 12 || rn2(4)) ? -tmp : -127);
                        /* set gm.multi_reason;
                           3.6.x used "frozen by a monster's gaze" */
                        dynamic_multi_reason(mon, "frozen", TRUE);
                        gn.nomovemsg = 0;
                    }
                } else {
                    pline("%s cannot defend itself.",
                          Adjmonnam(mon, "blind"));
                    if (!rn2(500))
                        change_luck(-1);
                }
            } else if (Free_action) {
                You("momentarily stiffen.");
            } else { /* gelatinous cube */
                You("are frozen by %s!", mon_nam(mon));
                gn.nomovemsg = You_can_move_again;
                nomul(-tmp);
                /* set gm.multi_reason;
                   3.6.x used "frozen by a monster"; be more specific */
                dynamic_multi_reason(mon, "frozen", FALSE);
                exercise(A_DEX, FALSE);
            }
            break;
        case AD_COLD: /* brown mold or blue jelly */
            if (monnear(mon, u.ux, u.uy)) {
                if (how_resistant(COLD_RES) == 100) {
                    shieldeff(u.ux, u.uy);
                    You_feel("a mild chill.");
                    monstseesu(M_SEEN_COLD);
                    ugolemeffects(AD_COLD, tmp);
                    break;
                }
                monstunseesu(M_SEEN_COLD);
                You("are suddenly very cold!");
                tmp = resist_reduce(tmp, COLD_RES);
                mdamageu(mon, tmp);
                /* monster gets stronger with your heat! */
                healmon(mon, (tmp + rn2(2)) / 2, (tmp + 1) / 2);
                /* at a certain point, the monster will reproduce! */
                if (mon->mhpmax > (((int) mon->m_lev) + 1) * 8)
                    (void) split_mon(mon, &gy.youmonst);
            }
            break;
        case AD_STUN: /* specifically yellow mold */
            if (!Stunned)
                make_stunned((long) tmp, TRUE);
            break;
        case AD_FIRE:
            if (monnear(mon, u.ux, u.uy)) {
                if (how_resistant(FIRE_RES) == 100 || Underwater) {
                    shieldeff(u.ux, u.uy);
                    You_feel("mildly warm.");
                    monstseesu(M_SEEN_FIRE);
                    ugolemeffects(AD_FIRE, tmp);
                    break;
                }
                monstunseesu(M_SEEN_FIRE);
                You("are suddenly very hot!");
                tmp = resist_reduce(tmp, FIRE_RES);
                mdamageu(mon, tmp); /* fire damage */
            }
            break;
        case AD_ELEC:
            if (how_resistant(SHOCK_RES) == 100) {
                shieldeff(u.ux, u.uy);
                You_feel("a mild tingle.");
                monstseesu(M_SEEN_ELEC);
                ugolemeffects(AD_ELEC, tmp);
                break;
            }
            monstunseesu(M_SEEN_ELEC);
            You("are jolted with electricity!");
            tmp = resist_reduce(tmp, SHOCK_RES);
            mdamageu(mon, tmp);
            break;
        case AD_DISE: /* specifically gray fungus */
            (void) diseasemu(mon);
            break;
        case AD_DRST: /* specifically green dragons */
            if (how_resistant(POISON_RES) == 100) {
                if (!rn2(5))
                    You("are immune to %s poisonous hide.",
                        s_suffix(mon_nam(mon)));
                monstseesu(M_SEEN_POISON);
            } else {
                i = rn2(20);
                if (i) {
                    You("have been poisoned!");
                    tmp = resist_reduce(tmp, POISON_RES);
                    mdamageu(mon, tmp);
                } else {
                    if (how_resistant(POISON_RES) <= 34) {
                        pline("%s poisonous hide was deadly...",
                              s_suffix(Monnam(mon)));
                        done_in_by(mon, DIED);
                        return M_ATTK_DEF_DIED;
                    }
                }
            }
            break;
        case AD_SLOW: /* specifically orange dragons */
            if (!Slow && !defended(&gy.youmonst, AD_SLOW)
                && !resists_slow(gy.youmonst.data) && !rn2(3))
                u_slow_down();
            break;
        case AD_SLEE: /* specifically black fungus */
            if (how_resistant(SLEEP_RES) < 100) {
                You("lose consciousness.");
                losehp(resist_reduce(rnd(6), POISON_RES),
                       "black fungus", KILLED_BY_AN);
                fall_asleep(-resist_reduce(rn2(3) + 8, SLEEP_RES), TRUE);
            }
            break;
        default:
            break;
        }
    }

    /* humanoid monsters wearing various dragon-scaled armor */
    if ((passive_armor = which_armor(mon, W_ARM)) != 0) {
        t = rnd(6) + 1;
        if (mhitb && !rn2(3)
            && Is_dragon_scaled_armor(passive_armor)) {
            int otyp = Dragon_armor_to_scales(passive_armor);

            switch (otyp) {
            case GREEN_DRAGON_SCALES:
                if (how_resistant(POISON_RES) == 100) {
                    You("are immune to %s poisonous armor.",
                        s_suffix(mon_nam(mon)));
                    monstseesu(M_SEEN_POISON);
                    break;
                } else {
                    if (rn2(20)) {
                        You("have been poisoned!");
                        t = resist_reduce(t, POISON_RES);
                        mdamageu(mon, t);
                    } else {
                        if (how_resistant(POISON_RES) <= 34) {
                            pline("%s poisonous armor was deadly...",
                                  s_suffix(Monnam(mon)));
                            done_in_by(mon, DIED);
                            return M_ATTK_DEF_DIED;
                        }
                    }
                }
                break;
            case BLACK_DRAGON_SCALES:
                {
                    long protector = attk_protection((int) aatyp);

                    /* hero using monsters' AT_MAGC attack is hitting
                       hand to hand rather than casting a spell */
                    if (aatyp == AT_MAGC)
                        protector = W_ARMG;

                    if (protector == 0L /* no protection */
                        || (protector == W_ARMG && !uarmg
                            && !uwep && !wep_was_destroyed)
                        || (protector == W_ARMF && !uarmf)
                        || (protector == W_ARMH && !uarmh)
                        || (protector == (W_ARMC | W_ARMG)
                            && (!uarmc || !uarmg))) {
                        if (how_resistant(DISINT_RES) == 100) {
                            if (!rn2(3))
                                You("are unaffected by %s deadly armor.",
                                    s_suffix(mon_nam(mon)));
                            monstseesu(M_SEEN_DISINT);
                            break;
                        } else {
                            if (rn2(40)) {
                                You("partially disintegrate!");
                                t = resist_reduce(t, DISINT_RES);
                                mdamageu(mon, t);
                            } else {
                                if (how_resistant(DISINT_RES) < 50) {
                                    pline(
                                      "%s deadly armor disintegrates you!",
                                          s_suffix(Monnam(mon)));
                                    u.ugrave_arise = -3;
                                    done_in_by(mon, DISINTEGRATED);
                                    return M_ATTK_DEF_DIED;
                                } else {
                                    You("partially disintegrate!");
                                    t = resist_reduce(t, DISINT_RES);
                                    mdamageu(mon, t);
                                }
                            }
                        }
                    }
                }
                if (!rn2(12)) {
                    if (!(uwep || (u.twoweap && uswapwep))
                        && !wep_was_destroyed
                        && (aatyp == AT_WEAP || aatyp == AT_CLAW
                            || aatyp == AT_MAGC || aatyp == AT_TUCH)) {
                        if (uarmg) {
                            if (uarmg->oartifact == ART_DRAGONBANE)
                                pline("%s %s, but remains %s.",
                                      The(xname(uarmg)),
                                      rn2(2) ? "shudders violently"
                                             : "vibrates unexpectedly",
                                      rn2(2) ? "whole" : "intact");
                            else if (rn2(2)
                                     && (uarmg->oerodeproof
                                         || is_supermaterial(uarmg)))
                                pline("%s being disintegrated!",
                                      Yobjnam2(uarmg, "resist"));
                            else
                                (void) disintegrate_arm(uarmg);
                        }
                    }
                    break;
                }
                if (weapon && !rn2(12)) {
                    if (aatyp == AT_KICK) {
                        if (uarmf) {
                            if (rn2(2)
                                && (uarmf->oerodeproof
                                    || is_supermaterial(uarmf)))
                                pline("%s being disintegrated!",
                                      Yobjnam2(uarmf, "resist"));
                            else
                                (void) disintegrate_arm(uarmf);
                        }
                        break;
                    } else if (aatyp == AT_WEAP || aatyp == AT_CLAW
                               || aatyp == AT_MAGC || aatyp == AT_TUCH) {
                        if (obj_resists(weapon, 0, 0)) {
                            pline_The("%s %s and cannot be disintegrated.",
                                      xname(weapon),
                                      rn2(2) ? "resists completely"
                                             : "defies physics");
                            break;
                        } else if (weapon->otyp == BLACK_DRAGON_SCALES
                                   || (Is_dragon_scaled_armor(weapon)
                                       && Dragon_armor_to_scales(weapon)
                                          == BLACK_DRAGON_SCALES)) {
                            pline("%s disintegration-proof and %s intact.",
                                  Yobjnam2(weapon, "are"),
                                  otense(weapon, "remain"));
                            break;
                        } else if (weapon->oartifact && rn2(20)) {
                            pline("%s %s, but remains %s.", Yname2(weapon),
                                  rn2(2) ? "shudders violently"
                                         : "vibrates unexpectedly",
                                  rn2(2) ? "whole" : "intact");
                            break;
                        } else if (rn2(2)
                                   && (weapon->oerodeproof
                                       || is_supermaterial(weapon))) {
                            pline("%s being disintegrated!",
                                  Yobjnam2(weapon, "resist"));
                            break;
                        } else {
                            if (cansee(mon->mx, mon->my))
                                pline("%s disintegrates!", Yname2(weapon));
                            if (weapon == uball || weapon == uchain)
                                unpunish();
                            /* make sure any properties given by worn or
                               wielded objects are removed */
                            remove_worn_item(weapon, FALSE);
                            setnotworn(weapon);
                            delobj(weapon);
                            break;
                        }
                    }
                }
                update_inventory();
                break;
            case ORANGE_DRAGON_SCALES:
                if (!Slow && !defended(&gy.youmonst, AD_SLOW)
                    && !resists_slow(gy.youmonst.data) && !rn2(3)) {
                    u_slow_down();
                }
                break;
            case WHITE_DRAGON_SCALES:
                if (how_resistant(COLD_RES) == 100) {
                    shieldeff(u.ux, u.uy);
                    monstseesu(M_SEEN_COLD);
                    You_feel("a mild chill from %s armor.",
                             s_suffix(mon_nam(mon)));
                    ugolemeffects(AD_COLD, t);
                    break;
                } else {
                    if (rn2(20)) {
                        You("are suddenly very cold!");
                        t = resist_reduce(t, COLD_RES);
                        mdamageu(mon, t);
                    } else {
                        pline("%s chilly armor freezes you!",
                              s_suffix(Monnam(mon)));
                        t = resist_reduce(t, COLD_RES);
                        mdamageu(mon, d(3, 6) + t);
                    }
                }
                break;
            case RED_DRAGON_SCALES:
                if (how_resistant(FIRE_RES) == 100
                    || Underwater) {
                    shieldeff(u.ux, u.uy);
                    monstseesu(M_SEEN_FIRE);
                    You_feel("mildly warm from %s armor.",
                             s_suffix(mon_nam(mon)));
                    ugolemeffects(AD_FIRE, t);
                    break;
                } else {
                    if (rn2(20)) {
                        You("are suddenly very hot!");
                        t = resist_reduce(t, FIRE_RES);
                        mdamageu(mon, t);
                    } else {
                        pline("%s fiery armor severely burns you!",
                              s_suffix(Monnam(mon)));
                        t = resist_reduce(t, FIRE_RES);
                        mdamageu(mon, d(3, 6) + t);
                    }
                }
                break;
            case YELLOW_DRAGON_SCALES:
                if (how_resistant(ACID_RES) == 100
                    || Underwater) {
                    shieldeff(u.ux, u.uy);
                    monstseesu(M_SEEN_ACID);
                    You_feel("a mild sting from %s armor.",
                             s_suffix(mon_nam(mon)));
                    ugolemeffects(AD_ACID, t);
                    break;
                } else {
                    if (rn2(20)) {
                        You_feel("a searing sensation!");
                        t = resist_reduce(t, ACID_RES);
                        mdamageu(mon, t);
                    } else {
                        pline("%s acidic armor critically sears you!",
                              s_suffix(Monnam(mon)));
                        t = resist_reduce(t, ACID_RES);
                        mdamageu(mon, d(3, 6) + t);
                    }
                }
                if (rn2(u.twoweap ? 2 : 3))
                    acid_damage(uwep);
                if (u.twoweap && rn2(2))
                    acid_damage(uswapwep);
                break;
            case GRAY_DRAGON_SCALES:
                if (!rn2(6)) {
                    (void) cancel_monst(&gy.youmonst,
                                        (struct obj *) 0, FALSE, TRUE,
                                        FALSE);
                }
                break;
            default: /* all other types of armor, just pass on through */
                break;
            }
        }
    }
    return (malive | mhit);
}

/*
 * Special (passive) attacks on an attacking object by monsters done here.
 * Assumes the attack was successful.
 */
int
passive_obj(
    struct monst *mon,
    struct obj *obj,      /* null means pick uwep, uswapwep or uarmg */
    struct attack *mattk) /* null means we find one internally */
{
    struct permonst *ptr = mon->data;
    int i, ret = ER_NOTHING;

    /* if caller hasn't specified an object, use uwep, uswapwep or uarmg;
       passive() does this for a bare-handed hit */
    if (!obj) {
        obj = (u.twoweap && uswapwep && !rn2(2)) ? uswapwep : uwep;
        if (!obj && (mattk->adtyp == AD_FIRE || mattk->adtyp == AD_ACID
                     || mattk->adtyp == AD_RUST || mattk->adtyp == AD_CORR
                     || mattk->adtyp == AD_ENCH || mattk->adtyp == AD_DISN))
            obj = uarmg; /* no weapon? then must be gloves */
        if (!obj)
            return ER_NOTHING; /* no object to affect */
    }

    /* if caller hasn't specified an attack, find one */
    if (!mattk) {
        for (i = 0;; i++) {
            if (i >= NATTK)
                return ER_NOTHING; /* no passive attacks */
            if (ptr->mattk[i].aatyp == AT_NONE)
                break; /* try this one */
        }
        mattk = &(ptr->mattk[i]);
    }

    switch (mattk->adtyp) {
    case AD_FIRE:
        if (!rn2(6) && !mon->mcan
            /* steam vortex: fire resist applies, fire damage doesn't */
            && mon->data != &mons[PM_STEAM_VORTEX]) {
            ret = erode_obj(obj, NULL, ERODE_BURN, EF_NONE);
        }
        break;
    case AD_ACID:
        if (!rn2(6)) {
            ret = erode_obj(obj, NULL, ERODE_CORRODE, EF_GREASE);
        }
        break;
    case AD_RUST:
        if (!mon->mcan) {
            ret = erode_obj(obj, (char *) 0, ERODE_RUST, EF_GREASE);
        }
        break;
    case AD_CORR:
        if (!mon->mcan) {
            ret = erode_obj(obj, (char *) 0, ERODE_CORRODE, EF_GREASE);
        }
        break;
    case AD_ENCH:
        if (!mon->mcan) {
            if (drain_item(obj, TRUE) && carried(obj)
                && (obj->known || obj->oclass == ARMOR_CLASS)) {
                pline("%s less effective.", Yobjnam2(obj, "seem"));
                ret = ER_DAMAGED;
            }
            break;
        }
        break;
    case AD_DISN: {
        int chance = (mon->data == &mons[PM_ANTIMATTER_VORTEX]) ? !rn2(3)
                                                                : !rn2(6);

        if (chance && !mon->mcan) {
            /* lets not make the game unwinnable */
            if (obj_resists(obj, 0, 0)) {
                pline_The("%s %s and cannot be disintegrated.", xname(obj),
                          rn2(2) ? "resists completely" : "defies physics");
                break;
            } else if (obj->oartifact == ART_HAND_OF_VECNA) {
                ; /* no feedback, as we're pretending that the Hand
                     isn't worn, but a part of your body */
                break;
            } else if (obj->oartifact == ART_DRAGONBANE
                       && mon->data != &mons[PM_ANTIMATTER_VORTEX]) {
                pline("%s %s, but remains %s.", The(xname(obj)),
                      rn2(2) ? "shudders violently" : "vibrates unexpectedly",
                      rn2(2) ? "whole" : "intact");
                break;
            } else if (obj->otyp == BLACK_DRAGON_SCALES
                       || (Is_dragon_scaled_armor(obj)
                           && Dragon_armor_to_scales(obj)
                              == BLACK_DRAGON_SCALES)) {
                pline("%s disintegration-proof and %s intact.",
                      Yobjnam2(obj, "are"), otense(obj, "remain"));
                break;
            } else if (obj->oartifact && rn2(20)) {
                pline("%s %s, but remains %s.", Yname2(obj),
                      rn2(2) ? "shudders violently" : "vibrates unexpectedly",
                      rn2(2) ? "whole" : "intact");
                break;
            } else if (rn2(2)
                       && (obj->oerodeproof || is_supermaterial(obj))) {
                pline("%s being disintegrated!", Yobjnam2(obj, "resist"));
                break;
            } else {
                if ((u.uswallow && mon == u.ustuck && !Blind)
                    || cansee(mon->mx, mon->my))
                    pline("%s disintegrates!", Yname2(obj));
                if (obj == uchain || obj == uball)
                    unpunish();
                /* make sure any properties given by worn/wielded
                   objects are removed */
                remove_worn_item(obj, FALSE);
                setnotworn(obj);
                delobj(obj);
                ret = ER_DESTROYED;
            }
            break;
        }
        break;
    }
    default:
        break;
    }

    /* obj is freed when destroyed - it cannot feed carried() then */
    if (ret == ER_DESTROYED || carried(obj))
        update_inventory();
    return ret;
}

DISABLE_WARNING_FORMAT_NONLITERAL

/* used by stumble_onto_mimic() and bhitm() cases WAN_LOCKING, WAN_OPENING */
void
that_is_a_mimic(
    struct monst *mtmp, /* a hidden mimic (nonnull) */
    unsigned mimic_flags) /* 0, MIM_REVEAL, MIM_OMIT_WAIT, REVEAL+OMIT */
{
    static char generic[] = "a monster";
    char fmtbuf[BUFSZ];
    const char *what = NULL;
    boolean reveal_it = (mimic_flags & MIM_REVEAL) != 0,
            omit_wait = (mimic_flags & MIM_OMIT_WAIT) != 0;

    Strcpy(fmtbuf, "Wait!  That's %s!");
    if (Blind) {
        if (!Blind_telepat)
            what = generic; /* with default fmt */
        else if (M_AP_TYPE(mtmp) == M_AP_MONSTER)
            what = a_monnam(mtmp); /* differs from what was sensed */
    } else {
        coordxy x = mtmp->mx, y = mtmp->my;
        int glyph = glyph_at(x, y);

        if (glyph_is_cmap(glyph)) {
            int sym = glyph_to_cmap(glyph);

            if (M_AP_TYPE(mtmp) == M_AP_FURNITURE
                || (M_AP_TYPE(mtmp) == M_AP_OBJECT && sym == S_trapped_chest))
                Snprintf(fmtbuf, sizeof fmtbuf, "That %s actually is %%s!",
                         defsyms[sym].explanation);
        } else if (glyph_is_object(glyph)) {
            boolean fakeobj;
            const char *otmp_name;
            struct obj *otmp = NULL;

            fakeobj = object_from_map(glyph, x, y, &otmp);
            otmp_name = (otmp && otmp->otyp != STRANGE_OBJECT)
                        ? simpleonames(otmp) : "strange object";
            Snprintf(fmtbuf, sizeof fmtbuf, "%s %s %s %%s!",
                     (otmp && is_plural(otmp)) ? "Those" : "That",
                     otmp_name, otmp ? otense(otmp, "are") : "is");
            if (fakeobj && otmp) {
                otmp->where = OBJ_FREE; /* object_from_map set to OBJ_FLOOR */
                dealloc_obj(otmp);
            }
        } else if (glyph_is_monster(glyph)) {
            const char *mtmp_name;
            int mndx = glyph_to_mon(glyph);

            assert(mndx >= LOW_PM && mndx <= HIGH_PM);
            mtmp_name = pmname(&mons[mndx], Mgender(mtmp));
            Snprintf(fmtbuf, sizeof fmtbuf,
                     "Wait!  That %s is really %%s!", mtmp_name);
        }

        /* cloned Wiz starts out mimicking some other monster and
           might make himself invisible before being revealed */
        if (mtmp->minvis && !See_invisible)
            what = generic;
        else if (M_AP_TYPE(mtmp) == M_AP_MONSTER)
            what = x_monnam(mtmp, ARTICLE_A, (char *) NULL, EXACT_NAME, TRUE);
        else if (mtmp->data->mlet == S_MIMIC
                 && (M_AP_TYPE(mtmp) == M_AP_OBJECT
                     || M_AP_TYPE(mtmp) == M_AP_FURNITURE)
                 && (mtmp->msleeping || mtmp->mfrozen))
            /* BUG: this will misclassify a paralyzed mimic as sleeping */
            what = x_monnam(mtmp, ARTICLE_A, "sleeping", 0, FALSE);
        else
            what = a_monnam(mtmp);
    }

    if (what) {
        int i = (omit_wait && !strncmp(fmtbuf, "Wait!  ", 7)) ? 7 : 0;

        pline(&fmtbuf[i], what);
    }
    if (reveal_it)
        seemimic(mtmp);
}

RESTORE_WARNING_FORMAT_NONLITERAL

/* Note: caller must ascertain mtmp is mimicking... */
void
stumble_onto_mimic(struct monst *mtmp)
{
    that_is_a_mimic(mtmp, MIM_REVEAL);

    if (!u.ustuck && !mtmp->mflee && dmgtype(mtmp->data, AD_STCK)
        /* must be adjacent; attack via polearm could be from farther away */
        && m_next2u(mtmp))
        set_ustuck(mtmp);

    wakeup(mtmp, FALSE); /* clears mimicking */
    /* if hero is blind, wakeup() won't display the monster even though
       it's no longer concealed */
    if (!canspotmon(mtmp)
        && !glyph_is_invisible(levl[mtmp->mx][mtmp->my].glyph))
        map_invisible(mtmp->mx, mtmp->my);
}

boolean
disguised_as_non_mon(struct monst *mtmp)
{
    return (!sensemon(mtmp)
            && M_AP_TYPE(mtmp)
            && M_AP_TYPE(mtmp) != M_AP_MONSTER);
}

boolean
disguised_as_mon(struct monst *mtmp)
{
    return (M_AP_TYPE(mtmp)
            && M_AP_TYPE(mtmp) == M_AP_MONSTER);
}

staticfn void
nohandglow(struct monst *mon)
{
    char *hands;
    boolean altfeedback;

    if (!u.umconf || mon->mconf)
        return;

    hands = makeplural(body_part(HAND));
    altfeedback = (Blind || Invisible); /* Invisible == Invis && !See_invis */
    if (u.umconf == 1) {
        if (altfeedback)
            Your("%s stop tingling.", hands);
        else
            Your("%s stop glowing %s.", hands, hcolor(NH_RED));
    } else {
        if (altfeedback)
            pline_The("tingling in your %s lessens.", hands);
        else
            Your("%s no longer glow so brightly %s.", hands, hcolor(NH_RED));
    }
    u.umconf--;
}

/* returns 1 if light flash has noticeable effect on 'mtmp', 0 otherwise */
int
flash_hits_mon(
    struct monst *mtmp,
    struct obj *otmp) /* source of flash */
{
    struct rm *lev;
    coordxy mx = mtmp->mx, my = mtmp->my;
    int tmp, amt, useeit, res = 0;

    if (gn.notonhead)
        return 0;
    lev = &levl[mx][my];
    useeit = canseemon(mtmp);

    if (M_AP_TYPE(mtmp) != M_AP_NOTHING) {
        char whatbuf[BUFSZ];
        int oldglyph = glyph_at(mx, my);

        /* 'altmon' probably doesn't matter here because 'whatbuf' will
           only be shown if the glyph changes and wakeup() doesn't call
           seemimic() for M_AP_MONSTER */
        mhidden_description(mtmp, MHID_ALTMON, whatbuf);

        wakeup(mtmp, FALSE); /* -> seemimic() -> newsym(); also calls
                              * finish_meating() to end quickmimic */

        /* if glyph has changed then hero saw something happen */
        if (glyph_at(mx, my) != oldglyph) {
            pline("That %s is really %s%c", whatbuf,
                  /* y_monnam()+a_monnam() */
                  x_monnam(mtmp, mtmp->mtame ? ARTICLE_YOUR : ARTICLE_A,
                           (char *) 0, 0, FALSE),
                  mtmp->mtame ? '.' : '!');
            res = 1;
        }
    }

    if (mtmp->msleeping && haseyes(mtmp->data)) {
        mtmp->msleeping = 0;
        if (useeit) {
            pline_The("flash awakens %s.", mon_nam(mtmp));
            res = 1;
        }
    } else if (mtmp->data->mlet != S_LIGHT) {
        if (!resists_blnd(mtmp)) {
            tmp = dist2(otmp->ox, otmp->oy, mx, my);
            if (useeit) {
                pline("%s is blinded by the flash!", Monnam(mtmp));
                res = 1;
            }
            if (hates_light(r_data(mtmp))) {
                /* Rule #1: Keep them out of the light. */
                amt = (otmp->otyp == WAN_LIGHT) ? d(1 + otmp->spe, 4)
                                                : rnd(min(mtmp->mhp, 4));
                light_hits_light_hater(mtmp, amt);
            }
            if (!DEADMONSTER(mtmp)) {
                if (!svc.context.mon_moving)
                    setmangry(mtmp, TRUE);
                if (tmp < 9 && !mtmp->isshk && rn2(4))
                    monflee(mtmp, rn2(4) ? rnd(100) : 0, FALSE, TRUE);
                mtmp->mcansee = 0;
                mtmp->mblinded = (tmp < 3) ? 0 : rnd(1 + 50 / tmp);
            }
        } else if (useeit) {
            if (resists_blnd_by_arti(mtmp))
                shieldeff(mx, my);
            if (flags.verbose) {
                if (lev->lit)
                    pline("The flash of light shines on %s.", mon_nam(mtmp));
                else
                    pline("%s is illuminated.", Monnam(mtmp));
                res = 2; /* 'message has been given' temporary value */
            }
        }
    }
    if (res) {
        if (!lev->lit)
            display_nhwindow(WIN_MESSAGE, TRUE);
        res &= 1; /* change temporary 2 back to 0 */
    }
    return res;
}

void
light_hits_light_hater(struct monst *mon, int dmg)
{
    if (!Deaf && mdistu(mon) <= 90) {
        /* cry of pain can be heard somewhat farther than the waking radius */
        pline_mon(mon, "%s %s!", Monnam(mon),
                  (dmg > mon->mhp / 2) ? "wails in agony"
                                       : "cries out in pain");
    } else if (canseemon(mon)) {
        pline_mon(mon, "%s recoils from the light!", Monnam(mon));
    }
    mon->mhp -= dmg;
    wake_nearto(mon->mx, mon->my, 30);
    if (DEADMONSTER(mon)) {
        if (svc.context.mon_moving)
            monkilled(mon, (char *) 0, AD_BLND);
        else
            killed(mon);
    } else if (cansee(mon->mx, mon->my) && !canspotmon(mon)) {
        map_invisible(mon->mx, mon->my);
    }
}

/*uhitm.c*/
