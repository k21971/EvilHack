/* NetHack 3.6	pray.c	$NHDT-Date: 1573346192 2019/11/10 00:36:32 $  $NHDT-Branch: NetHack-3.6 $:$NHDT-Revision: 1.118 $ */
/* Copyright (c) Benson I. Margulies, Mike Stephenson, Steve Linhart, 1989. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "qtext.h"

static boolean mitre_inhell(void);
STATIC_PTR int NDECL(prayer_done);
STATIC_DCL struct obj *NDECL(worst_cursed_item);
STATIC_DCL int NDECL(in_trouble);
STATIC_DCL void FDECL(fix_worst_trouble, (int));
STATIC_DCL void FDECL(angrygods, (ALIGNTYP_P));
STATIC_DCL void FDECL(at_your_feet, (const char *));
STATIC_DCL void NDECL(gcrownu);
STATIC_DCL void FDECL(pleased, (ALIGNTYP_P));
STATIC_DCL void FDECL(fry_by_god, (ALIGNTYP_P, BOOLEAN_P));
STATIC_DCL void FDECL(gods_angry, (ALIGNTYP_P));
STATIC_DCL void FDECL(gods_upset, (ALIGNTYP_P));
STATIC_DCL void FDECL(consume_offering, (struct obj *));
STATIC_DCL boolean FDECL(water_prayer, (BOOLEAN_P));
STATIC_DCL boolean FDECL(blocked_boulder, (int, int));

/* simplify a few tests */
#define Cursed_obj(obj, typ) ((obj) && (obj)->otyp == (typ) && cursed(obj, TRUE))

/*
 * Logic behind deities and altars and such:
 * + prayers are made to your god if not on an altar, and to the altar's god
 *   if you are on an altar
 * + If possible, your god answers all prayers, which is why bad things happen
 *   if you try to pray on another god's altar
 * + sacrifices work basically the same way, but the other god may decide to
 *   accept your allegiance, after which they are your god.  If rejected,
 *   your god takes over with your punishment.
 * + if you're in Gehennom, all messages come from Moloch
 */

/*
 *      Moloch, who dwells in Gehennom, is the "renegade" cruel god
 *      responsible for the theft of the Amulet from Marduk, the Creator.
 *      Moloch is unaligned.
 */
static const char *Moloch = "Moloch";

static const char *godvoices[] = {
    "booms out", "thunders", "rings out", "booms",
};

/* values calculated when prayer starts, and used when completed */
static aligntyp p_aligntyp;
static int p_trouble;
static int p_type; /* (-2)-3: (-1)=really naughty, 3=really good */

#define PIOUS 20
#define DEVOUT 14
#define FERVENT 9
#define STRIDENT 4

/*
 * The actual trouble priority is determined by the order of the
 * checks performed in in_trouble() rather than by these numeric
 * values, so keep that code and these values synchronized in
 * order to have the values be meaningful.
 */

#define TROUBLE_STONED 15
#define TROUBLE_SLIMED 14
#define TROUBLE_STRANGLED 13
#define TROUBLE_LAVA 12
#define TROUBLE_SICK 11
#define TROUBLE_WITHERING 10
#define TROUBLE_STARVING 9
#define TROUBLE_REGION 8 /* stinking cloud */
#define TROUBLE_HIT 7
#define TROUBLE_LYCANTHROPE 6
#define TROUBLE_COLLAPSING 5
#define TROUBLE_STUCK_IN_WALL 4
#define TROUBLE_CURSED_LEVITATION 3
#define TROUBLE_UNUSEABLE_HANDS 2
#define TROUBLE_CURSED_BLINDFOLD 1

#define TROUBLE_PUNISHED (-1)
#define TROUBLE_FUMBLING (-2)
#define TROUBLE_CURSED_ITEMS (-3)
#define TROUBLE_SADDLE (-4)
#define TROUBLE_BLIND (-5)
#define TROUBLE_POISONED (-6)
#define TROUBLE_WOUNDED_LEGS (-7)
#define TROUBLE_HUNGRY (-8)
#define TROUBLE_STUNNED (-9)
#define TROUBLE_CONFUSED (-10)
#define TROUBLE_HALLUCINATION (-11)


#define ugod_is_angry() (u.ualign.record < 0)
#define on_altar() IS_ALTAR(levl[u.ux][u.uy].typ)
#define on_shrine() ((levl[u.ux][u.uy].altarmask & AM_SHRINE) != 0)

/* TRUE if in Gehennom and not wearing Mitre,
   which allows praying inside Gehennom */
static boolean
mitre_inhell(void)
{
    if (!Inhell)
        return FALSE;
    if (uarmh && uarmh->oartifact == ART_MITRE_OF_HOLINESS)
        return FALSE;
    return TRUE;
}

/* critically low hit points if hp <= 5 or hp <= maxhp/N for some N */
boolean
critically_low_hp(only_if_injured)
boolean only_if_injured; /* determines whether maxhp <= 5 matters */
{
    int divisor, hplim, curhp = Upolyd ? u.mh : u.uhp,
                        maxhp = Upolyd ? u.mhmax : u.uhpmax;

    if (only_if_injured && !(curhp < maxhp))
        return FALSE;
    /* if maxhp is extremely high, use lower threshold for the division test
       (golden glow cuts off at 11+5*lvl, nurse interaction at 25*lvl; this
       ought to use monster hit dice--and a smaller multiplier--rather than
       ulevel when polymorphed, but polyself doesn't maintain that) */
    hplim = 15 * u.ulevel;
    if (maxhp > hplim)
        maxhp = hplim;
    /* 7 used to be the unconditional divisor */
    switch (xlev_to_rank(u.ulevel)) { /* maps 1..30 into 0..8 */
    case 0:
    case 1:
        divisor = 5;
        break; /* explvl 1 to 5 */
    case 2:
    case 3:
        divisor = 6;
        break; /* explvl 6 to 13 */
    case 4:
    case 5:
        divisor = 7;
        break; /* explvl 14 to 21 */
    case 6:
    case 7:
        divisor = 8;
        break; /* explvl 22 to 29 */
    default:
        divisor = 9;
        break; /* explvl 30+ */
    }
    /* 5 is a magic number in TROUBLE_HIT handling below */
    return (boolean) (curhp <= 5 || curhp * divisor <= maxhp);
}

/* return True if surrounded by impassible rock, regardless of the state
   of your own location (for example, inside a doorless closet) */
boolean
stuck_in_wall()
{
    int i, j, x, y, count = 0;

    if (Passes_walls)
        return FALSE;
    for (i = -1; i <= 1; i++) {
        x = u.ux + i;
        for (j = -1; j <= 1; j++) {
            if (!i && !j)
                continue;
            y = u.uy + j;
            if (!isok(x, y)
                || (IS_ROCK(levl[x][y].typ)
                    && (levl[x][y].typ != SDOOR && levl[x][y].typ != SCORR))
                || (blocked_boulder(i, j) && !racial_throws_rocks(&youmonst)))
                ++count;
        }
    }
    return (count == 8) ? TRUE : FALSE;
}

/*
 * Return 0 if nothing particular seems wrong, positive numbers for
 * serious trouble, and negative numbers for comparative annoyances.
 * This returns the worst problem. There may be others, and the gods
 * may fix more than one.
 *
 * This could get as bizarre as noting surrounding opponents, (or
 * hostile dogs), but that's really hard.
 *
 * We could force rehumanize of polyselfed people, but we can't tell
 * unintentional shape changes from the other kind. Oh well.
 * 3.4.2: make an exception if polymorphed into a form which lacks
 * hands; that's a case where the ramifications override this doubt.
 */
STATIC_OVL int
in_trouble()
{
    struct obj *otmp;
    int i;

    /*
     * major troubles
     */
    if (Stoned)
        return TROUBLE_STONED;
    if (Slimed)
        return TROUBLE_SLIMED;
    if (Strangled)
        return TROUBLE_STRANGLED;
    if (u.utrap && u.utraptype == TT_LAVA)
        return TROUBLE_LAVA;
    if (Sick)
        return TROUBLE_SICK;
    if (HWithering && !BWithering)
        return TROUBLE_WITHERING;
    if (u.uhs >= WEAK)
        return TROUBLE_STARVING;
    if (region_danger())
        return TROUBLE_REGION;
    if (critically_low_hp(FALSE))
        return TROUBLE_HIT;
    if (u.ulycn >= LOW_PM)
        return TROUBLE_LYCANTHROPE;
    if (near_capacity() >= EXT_ENCUMBER && AMAX(A_STR) - ABASE(A_STR) > 3)
        return TROUBLE_COLLAPSING;
    if (stuck_in_wall())
        return TROUBLE_STUCK_IN_WALL;
    if (Cursed_obj(uarmf, LEVITATION_BOOTS)
        || stuck_ring(uleft, RIN_LEVITATION)
        || stuck_ring(uright, RIN_LEVITATION))
        return TROUBLE_CURSED_LEVITATION;
    if (nohands(youmonst.data) || !freehand()) {
        /* for bag/box access [cf use_container()]...
           make sure it's a case that we know how to handle;
           otherwise "fix all troubles" would get stuck in a loop */
        if (welded(uwep))
            return TROUBLE_UNUSEABLE_HANDS;
        if (Upolyd && nohands(youmonst.data)
            && (!Unchanging || ((otmp = unchanger()) != 0 && cursed(otmp, TRUE))))
            return TROUBLE_UNUSEABLE_HANDS;
    }
    if (Blindfolded && cursed(ublindf, TRUE))
        return TROUBLE_CURSED_BLINDFOLD;

    /*
     * minor troubles...
     * Cavepersons get another flavor kick: their god is
     * also a bit primitive, so they get a 10% chance of
     * their god being asleep at the switch
     */
    if (!rn2(10) && Role_if(PM_CAVEMAN))
        return 0;
    if (Punished || (u.utrap && u.utraptype == TT_BURIEDBALL))
        return TROUBLE_PUNISHED;
    if (Cursed_obj(uarmg, GAUNTLETS_OF_FUMBLING)
        || Cursed_obj(uarmf, FUMBLE_BOOTS))
        return TROUBLE_FUMBLING;
    if (worst_cursed_item())
        return TROUBLE_CURSED_ITEMS;
    if (u.usteed) { /* can't voluntarily dismount from a cursed saddle */
        otmp = which_armor(u.usteed, W_SADDLE);
        if (Cursed_obj(otmp, SADDLE))
            return TROUBLE_SADDLE;
    }

    if (Blinded > 1 && haseyes(youmonst.data)
        && (!u.uswallow
            || !attacktype_fordmg(u.ustuck->data, AT_ENGL, AD_BLND)))
        return TROUBLE_BLIND;
    /* deafness isn't it's own trouble; healing magic cures deafness
       when it cures blindness, so do the same with trouble repair */
    if ((HDeaf & TIMEOUT) > 1L)
        return TROUBLE_BLIND;

    for (i = 0; i < A_MAX; i++)
        if (ABASE(i) < AMAX(i))
            return TROUBLE_POISONED;
    if (Wounded_legs && !u.usteed)
        return TROUBLE_WOUNDED_LEGS;
    if (u.uhs >= HUNGRY)
        return TROUBLE_HUNGRY;
    if (HStun & TIMEOUT)
        return TROUBLE_STUNNED;
    if (HConfusion & TIMEOUT)
        return TROUBLE_CONFUSED;
    if (HHallucination & TIMEOUT)
        return TROUBLE_HALLUCINATION;
    return 0;
}

/* select an item for TROUBLE_CURSED_ITEMS */
STATIC_OVL struct obj *
worst_cursed_item()
{
    register struct obj *otmp;

    /* Infidels are immune to curses, but a cursed luckstone is still bad */
    if (Role_if(PM_INFIDEL)) {
        for (otmp = invent; otmp; otmp = otmp->nobj)
            if (confers_luck(otmp) && otmp->cursed)
                return otmp;
        return (struct obj *) 0;
    }

    /* if strained or worse, check for loadstone first */
    if (near_capacity() >= HVY_ENCUMBER) {
        for (otmp = invent; otmp; otmp = otmp->nobj)
            if (Cursed_obj(otmp, LOADSTONE))
                return otmp;
    }
    /* weapon takes precedence if it is interfering
       with taking off a ring or putting on a shield */
    if (welded(uwep) && (uright || bimanual(uwep))) { /* weapon */
        otmp = uwep;
    /* gloves come next, due to rings */
    } else if (uarmg && uarmg->cursed) { /* gloves */
        otmp = uarmg;
    /* then shield due to two handed weapons and spells */
    } else if (uarms && uarms->cursed) { /* shield */
        otmp = uarms;
    /* then cloak due to body armor */
    } else if (uarmc && uarmc->cursed) { /* cloak */
        otmp = uarmc;
    } else if (uarm && uarm->cursed) { /* suit */
        otmp = uarm;
    /* if worn helmet of opposite alignment is making you an adherent
       of the current god, he/she/it won't uncurse that for you */
    } else if (uarmh && uarmh->cursed /* helmet */
               && uarmh->otyp != HELM_OF_OPPOSITE_ALIGNMENT) {
        otmp = uarmh;
    } else if (uarmf && uarmf->cursed) { /* boots */
        otmp = uarmf;
    } else if (uarmu && uarmu->cursed) { /* shirt */
        otmp = uarmu;
    } else if (uamul && uamul->cursed) { /* amulet */
        otmp = uamul;
    } else if (uleft && uleft->cursed) { /* left ring */
        otmp = uleft;
    } else if (uright && uright->cursed) { /* right ring */
        otmp = uright;
    } else if (ublindf && ublindf->cursed) { /* eyewear */
        otmp = ublindf; /* must be non-blinding lenses */
    /* if weapon wasn't handled above, do it now */
    } else if (welded(uwep)) { /* weapon */
        otmp = uwep;
    /* active secondary weapon even though it isn't welded */
    } else if (uswapwep && uswapwep->cursed && u.twoweap) {
        otmp = uswapwep;
    /* all worn items ought to be handled by now */
    } else {
        for (otmp = invent; otmp; otmp = otmp->nobj) {
            if (!otmp->cursed)
                continue;
            if (otmp->otyp == LOADSTONE || confers_luck(otmp))
                break;
        }
    }
    return otmp;
}

STATIC_OVL void
fix_worst_trouble(trouble)
int trouble;
{
    int i;
    struct obj *otmp = 0;
    const char *what = (const char *) 0;
    static NEARDATA const char leftglow[] = "Your left ring softly glows",
                               rightglow[] = "Your right ring softly glows";

    switch (trouble) {
    case TROUBLE_STONED:
        make_stoned(0L, "You feel more limber.", 0, (char *) 0);
        break;
    case TROUBLE_SLIMED:
        make_slimed(0L, "The slime disappears.");
        break;
    case TROUBLE_STRANGLED:
        if (uamul && uamul->otyp == AMULET_OF_STRANGULATION) {
            Your("amulet vanishes!");
            useup(uamul);
        }
        You("can breathe again.");
        Strangled = 0L;
        context.botl = 1;
        break;
    case TROUBLE_LAVA:
        You("are back on solid ground.");
        /* teleport should always succeed, but if not, just untrap them */
        if (!safe_teleds(FALSE))
            reset_utrap(TRUE);
        break;
    case TROUBLE_STARVING:
        /* temporarily lost strength recovery now handled by init_uhunger() */
        /*FALLTHRU*/
    case TROUBLE_HUNGRY:
        Your("%s feels content.", body_part(STOMACH));
        init_uhunger();
        context.botl = 1;
        break;
    case TROUBLE_SICK:
        You_feel("better.");
        make_sick(0L, (char *) 0, FALSE, SICK_ALL);
        break;
    case TROUBLE_WITHERING:
        set_itimeout(&HWithering, (long) 0);
        if (!Withering)
            You("stop withering.");
        else
            You_feel("a little less dessicated.");
        break;
    case TROUBLE_REGION:
        /* stinking cloud, with hero vulnerable to HP loss */
        region_safety();
        break;
    case TROUBLE_HIT:
        /* "fix all troubles" will keep trying if hero has
           5 or less hit points, so make sure they're always
           boosted to be more than that */
        You_feel("much better.");
        if (Upolyd) {
            u.mhmax += rnd(5);
            if (u.mhmax <= 5)
                u.mhmax = 5 + 1;
            u.mh = u.mhmax;
        }
        if (u.uhpmax < u.ulevel * 5 + 11)
            u.uhpmax += rnd(5);
        if (u.uhpmax <= 5)
            u.uhpmax = 5 + 1;
        u.uhp = u.uhpmax;
        context.botl = 1;
        break;
    case TROUBLE_COLLAPSING:
        /* override Fixed_abil; uncurse that if feasible */
        You_feel("%sstronger.",
                 (AMAX(A_STR) - ABASE(A_STR) > 6) ? "much " : "");
        ABASE(A_STR) = AMAX(A_STR);
        context.botl = 1;
        if (Fixed_abil) {
            if ((otmp = stuck_ring(uleft, RIN_SUSTAIN_ABILITY)) != 0) {
                if (otmp == uleft)
                    what = leftglow;
            } else if ((otmp = stuck_ring(uright, RIN_SUSTAIN_ABILITY)) != 0) {
                if (otmp == uright)
                    what = rightglow;
            }
            if (otmp)
                goto decurse;
        }
        break;
    case TROUBLE_STUCK_IN_WALL:
        /* no control, but works on no-teleport levels */
        if (safe_teleds(FALSE)) {
            Your("surroundings change.");
        } else {
            /* safe_teleds() couldn't find a safe place; perhaps the
               level is completely full.  As a last resort, confer
               intrinsic wall/rock-phazing.  Hero might get stuck
               again fairly soon....
               Without something like this, fix_all_troubles can get
               stuck in an infinite loop trying to fix STUCK_IN_WALL
               and repeatedly failing. */
            set_itimeout(&HPasses_walls, (long) (d(4, 4) + 4)); /* 8..20 */
            /* how else could you move between packed rocks or among
               lattice forming "solid" rock? */
            You_feel("much slimmer.");
        }
        break;
    case TROUBLE_CURSED_LEVITATION:
        if (Cursed_obj(uarmf, LEVITATION_BOOTS)) {
            otmp = uarmf;
        } else if ((otmp = stuck_ring(uleft, RIN_LEVITATION)) != 0) {
            if (otmp == uleft)
                what = leftglow;
        } else if ((otmp = stuck_ring(uright, RIN_LEVITATION)) != 0) {
            if (otmp == uright)
                what = rightglow;
        }
        goto decurse;
    case TROUBLE_UNUSEABLE_HANDS:
        if (welded(uwep)) {
            otmp = uwep;
            goto decurse;
        }
        if (Upolyd && nohands(youmonst.data)) {
            if (!Unchanging) {
                Your("shape becomes uncertain.");
                rehumanize(); /* "You return to {normal} form." */
            } else if ((otmp = unchanger()) != 0 && cursed(otmp, TRUE)) {
                /* otmp is an amulet of unchanging */
                goto decurse;
            }
        }
        if (nohands(youmonst.data) || !freehand())
            impossible("fix_worst_trouble: couldn't cure hands.");
        break;
    case TROUBLE_CURSED_BLINDFOLD:
        otmp = ublindf;
        goto decurse;
    case TROUBLE_LYCANTHROPE:
        you_unwere(TRUE);
        break;
    /*
     */
    case TROUBLE_PUNISHED:
        Your("chain disappears.");
        if (u.utrap && u.utraptype == TT_BURIEDBALL)
            buried_ball_to_freedom();
        else
            unpunish();
        break;
    case TROUBLE_FUMBLING:
        if (Cursed_obj(uarmg, GAUNTLETS_OF_FUMBLING))
            otmp = uarmg;
        else if (Cursed_obj(uarmf, FUMBLE_BOOTS))
            otmp = uarmf;
        goto decurse;
        /*NOTREACHED*/
        break;
    case TROUBLE_CURSED_ITEMS:
        otmp = worst_cursed_item();
        if (otmp == uright)
            what = rightglow;
        else if (otmp == uleft)
            what = leftglow;
 decurse:
        if (!otmp) {
            impossible("fix_worst_trouble: nothing to uncurse.");
            return;
        }
        if (otmp == uarmg && Glib) {
            make_glib(0);
            Your("%s are no longer slippery.", gloves_simple_name(uarmg));
            if (!cursed(otmp, TRUE))
                break;
        }
        if (!Blind || (otmp == ublindf && Blindfolded_only)) {
            pline("%s %s.",
                  what ? what : (const char *) Yobjnam2(otmp, "softly glow"),
                  hcolor(NH_AMBER));
            iflags.last_msg = PLNMSG_OBJ_GLOWS;
            otmp->bknown = !Hallucination; /* ok to skip set_bknown() */
        }
        uncurse(otmp);
        update_inventory();
        break;
    case TROUBLE_POISONED:
        /* override Fixed_abil; ignore items which confer that */
        if (Hallucination)
            pline("There's a tiger in your tank.");
        else
            You_feel("in good health again.");
        for (i = 0; i < A_MAX; i++) {
            if (ABASE(i) < AMAX(i)) {
                ABASE(i) = AMAX(i);
                context.botl = 1;
            }
        }
        (void) encumber_msg();
        break;
    case TROUBLE_BLIND: { /* handles deafness as well as blindness */
        char msgbuf[BUFSZ];
        const char *eyes = body_part(EYE);
        boolean cure_deaf = (HDeaf & TIMEOUT) ? TRUE : FALSE;

        msgbuf[0] = '\0';
        if (Blinded) {
            if (eyecount(youmonst.data) != 1)
                eyes = makeplural(eyes);
            Sprintf(msgbuf, "Your %s %s better", eyes, vtense(eyes, "feel"));
            u.ucreamed = 0;
            make_blinded(0L, FALSE);
        }
        if (cure_deaf) {
            make_deaf(0L, FALSE);
            if (!Deaf)
                Sprintf(eos(msgbuf), "%s can hear again",
                        !*msgbuf ? "You" : " and you");
        }
        if (*msgbuf)
            pline("%s.", msgbuf);
        break;
    }
    case TROUBLE_WOUNDED_LEGS:
        heal_legs(0);
        break;
    case TROUBLE_STUNNED:
        make_stunned(0L, TRUE);
        break;
    case TROUBLE_CONFUSED:
        make_confused(0L, TRUE);
        break;
    case TROUBLE_HALLUCINATION:
        pline("Looks like you are back in Kansas.");
        (void) make_hallucinated(0L, FALSE, 0L);
        break;
    case TROUBLE_SADDLE:
        otmp = which_armor(u.usteed, W_SADDLE);
        if (!Blind) {
            pline("%s %s.", Yobjnam2(otmp, "softly glow"), hcolor(NH_AMBER));
            set_bknown(otmp, 1);
        }
        uncurse(otmp);
        break;
    }
}

/* "I am sometimes shocked by... the nuns who never take a bath without
 * wearing a bathrobe all the time.  When asked why, since no man can see them,
 * they reply 'Oh, but you forget the good God'.  Apparently they conceive of
 * the Deity as a Peeping Tom, whose omnipotence enables Him to see through
 * bathroom walls, but who is foiled by bathrobes." --Bertrand Russell, 1943
 * Divine wrath, dungeon walls, and armor follow the same principle.
 */
void
god_zaps_you(resp_god)
aligntyp resp_god;
{
    if (u.uswallow) {
        pline(
          "Suddenly a bolt of lightning comes down at you from the heavens!");
        pline("It strikes %s!", mon_nam(u.ustuck));
        if (!resists_elec(u.ustuck)) {
            pline("%s fries to a crisp!", Monnam(u.ustuck));
            /* Yup, you get experience.  It takes guts to successfully
             * pull off this trick on your god, anyway.
             * Other credit/blame applies (luck or alignment adjustments),
             * but not direct kill count (pacifist conduct).
             */
            xkilled(u.ustuck, XKILL_NOMSG | XKILL_NOCONDUCT);
        } else
            pline("%s seems unaffected.", Monnam(u.ustuck));
    } else {
        pline("Suddenly, a bolt of lightning strikes you!");
        if (Reflecting) {
            shieldeff(u.ux, u.uy);
            monstseesu(M_SEEN_REFL);
            if (Blind)
                pline("For some reason you're unaffected.");
            else
                (void) ureflects("%s reflects from your %s.", "It");
        } else if (how_resistant(SHOCK_RES) == 100) {
            shieldeff(u.ux, u.uy);
            monstseesu(M_SEEN_ELEC);
            pline("It seems not to affect you.");
        } else
            fry_by_god(resp_god, FALSE);
    }

    pline("%s is not deterred...", align_gname(resp_god));
    if (u.uswallow) {
        pline("A wide-angle disintegration beam aimed at you hits %s!",
              mon_nam(u.ustuck));
        if (!resists_disint(u.ustuck)) {
            pline("%s disintegrates into a pile of dust!", Monnam(u.ustuck));
            xkilled(u.ustuck, XKILL_NOMSG | XKILL_NOCORPSE | XKILL_NOCONDUCT);
        } else
            pline("%s seems unaffected.", Monnam(u.ustuck));
    } else {
        pline("A wide-angle disintegration beam hits you!");

        /* disintegrate shield and body armor before disintegrating
         * the impudent mortal, like black dragon breath -3.
         */
        if (uarms && !(EReflecting & W_ARMS)
            && !(EDisint_resistance & W_ARMS))
            (void) destroy_arm(uarms);
        if (uarmc && !(EReflecting & W_ARMC)
            && !(EDisint_resistance & W_ARMC))
            (void) destroy_arm(uarmc);
        if (uarm && !(EReflecting & W_ARM) && !(EDisint_resistance & W_ARM)
            && !uarmc)
            (void) destroy_arm(uarm);
        if (uarmu && !uarm && !uarmc)
            (void) destroy_arm(uarmu);
        if (how_resistant(DISINT_RES) < 100) {
            fry_by_god(resp_god, TRUE);
        } else {
            You("bask in its %s glow for a minute...", NH_BLACK);
            monstseesu(M_SEEN_DISINT);
            godvoice(resp_god, "I believe it not!");
        }
        if (Is_astralevel(&u.uz) || Is_sanctum(&u.uz)) {
            /* one more try for high altars */
            verbalize("Thou cannot escape my wrath, mortal!");
            (void) summon_minion(resp_god, FALSE);
            (void) summon_minion(resp_god, FALSE);
            (void) summon_minion(resp_god, FALSE);
            verbalize("Destroy %s, my servants!", uhim());
        }
    }
}

STATIC_OVL void
fry_by_god(resp_god, via_disintegration)
aligntyp resp_god;
boolean via_disintegration;
{
    You("%s!", !via_disintegration ? "fry to a crisp"
                                   : "disintegrate into a pile of dust");
    killer.format = KILLED_BY;
    Sprintf(killer.name, "the wrath of %s", align_gname(resp_god));
    done(DIED);
}

STATIC_OVL void
angrygods(resp_god)
aligntyp resp_god;
{
    int maxanger;

    if (mitre_inhell())
        resp_god = A_NONE;
    u.ublessed = 0;

    /* changed from tmp = u.ugangr + abs (u.uluck) -- rph */
    /* added test for alignment diff -dlc */
    if (resp_god != u.ualign.type)
        maxanger = u.ualign.record / 2 + (Luck > 0 ? -Luck / 3 : -Luck);
    else
        maxanger = 3 * u.ugangr + ((Luck > 0 || u.ualign.record >= STRIDENT)
                                   ? -Luck / 3
                                   : -Luck);
    if (maxanger < 1)
        maxanger = 1; /* possible if bad align & good luck */
    else if (maxanger > 15)
        maxanger = 15; /* be reasonable */

    switch (rn2(maxanger)) {
    case 0:
    case 1:
        You_feel("that %s is %s.", align_gname(resp_god),
                 Hallucination ? "bummed" : "displeased");
        break;
    case 2:
    case 3:
        godvoice(resp_god, (char *) 0);
        pline("\"Thou %s, %s.\"",
              (ugod_is_angry() && resp_god == u.ualign.type)
                  ? "hast strayed from the path"
                  : "art arrogant",
              youmonst.data->mlet == S_HUMAN ? "mortal" : "creature");
        verbalize("Thou must relearn thy lessons!");
        (void) adjattrib(A_WIS, -1, FALSE);
        losexp((char *) 0);
        break;
    case 6:
        if (!Punished) {
            gods_angry(resp_god);
            punish((struct obj *) 0);
            break;
        } /* else fall thru */
    case 4:
    case 5:
        gods_angry(resp_god);
        if (!Blind && !Antimagic)
            pline("%s glow surrounds you.", An(hcolor(NH_BLACK)));
        rndcurse();
        break;
    case 7:
    case 8:
        godvoice(resp_god, (char *) 0);
        verbalize("Thou durst %s me?",
                  (on_altar() && (a_align(u.ux, u.uy) != resp_god))
                      ? "scorn"
                      : "call upon");
        /* [why isn't this using verbalize()?] */
        pline("\"Then die, %s!\"",
              (youmonst.data->mlet == S_HUMAN) ? "mortal" : "creature");
        (void) summon_minion(resp_god, FALSE);
        break;

    default:
        gods_angry(resp_god);
        god_zaps_you(resp_god);
        break;
    }
    u.ublesscnt = rnz(300);
    return;
}

/* helper to print "str appears at your feet", or appropriate */
static void
at_your_feet(str)
const char *str;
{
    if (Blind)
        str = Something;
    if (u.uswallow) {
        /* barrier between you and the floor */
        pline("%s %s into %s %s.", str, vtense(str, "drop"),
              s_suffix(mon_nam(u.ustuck)), mbodypart(u.ustuck, STOMACH));
    } else {
        pline("%s %s %s your %s!", str,
              Blind ? "lands" : vtense(str, "appear"),
              Levitation ? "beneath" : "at", makeplural(body_part(FOOT)));
    }
}

int
wiz_crown()
{
    pline("Your crown, my %s.", flags.female ? "queen" : "king");
    gcrownu();
    return 0;
}

STATIC_OVL void
gcrownu()
{
    struct obj *obj;
    boolean already_exists, in_hand;
    short class_gift;
    xchar maxint, maxwis;
#define ok_wep(o) ((o) && ((o)->oclass == WEAPON_CLASS || is_weptool(o)))

    /* Moloch-worshippers get intrinsics from becoming a demon */
    if (u.ualign.type != A_NONE) {
        incr_resistance(&HFire_resistance, 100);
        incr_resistance(&HCold_resistance, 100);
        incr_resistance(&HShock_resistance, 100);
        incr_resistance(&HSleep_resistance, 100);
        incr_resistance(&HPoison_resistance, 100);
        HSee_invisible |= FROMOUTSIDE;
        /* small chance to obtain sick resistance, but not
        this way if infidel (see below) */
        if (!rn2(10))
            HSick_resistance |= FROMOUTSIDE;

        monstseesu(M_SEEN_FIRE | M_SEEN_COLD | M_SEEN_ELEC
                   | M_SEEN_SLEEP | M_SEEN_POISON);
    }

    godvoice(u.ualign.type, (char *) 0);

    class_gift = STRANGE_OBJECT;
    /* 3.3.[01] had this in the A_NEUTRAL case,
       preventing chaotic wizards from receiving a spellbook */
    if (Role_if(PM_WIZARD)
        && (!uwep || (uwep->oartifact != ART_VORPAL_BLADE
                      && uwep->oartifact != ART_STORMBRINGER))
        && !carrying(SPE_FINGER_OF_DEATH)) {
        class_gift = SPE_FINGER_OF_DEATH;
    } else if (Role_if(PM_MONK) && (!uwep || !uwep->oartifact)
               && !carrying(SPE_RESTORE_ABILITY)) {
        /* monks rarely wield a weapon */
        class_gift = SPE_RESTORE_ABILITY;
    }

    obj = ok_wep(uwep) ? uwep : 0;
    already_exists = in_hand = FALSE; /* lint suppression */
    switch (u.ualign.type) {
    case A_LAWFUL:
        u.uevent.uhand_of_elbereth = 1;
        verbalize("I crown thee...  The Hand of Elbereth!");
        learn_elbereth();
        livelog_printf(LL_DIVINEGIFT,
                "was crowned \"The Hand of Elbereth\" by %s", u_gname());
        break;
    case A_NEUTRAL:
        u.uevent.uhand_of_elbereth = 2;
        /* priests aren't supposed to use edged weapons */
        if (Role_if(PM_PRIEST)) {
            in_hand = wielding_artifact(ART_MJOLLNIR);
            already_exists =
                exist_artifact(HEAVY_WAR_HAMMER, artiname(ART_MJOLLNIR));
        } else {
            in_hand = wielding_artifact(ART_VORPAL_BLADE);
            already_exists =
                exist_artifact(LONG_SWORD, artiname(ART_VORPAL_BLADE));
        }
        verbalize("Thou shalt be my Envoy of Balance!");
        livelog_printf(LL_DIVINEGIFT, "became %s Envoy of Balance",
                s_suffix(u_gname()));
        break;
    case A_CHAOTIC:
        u.uevent.uhand_of_elbereth = 3;
        /* priests aren't supposed to use edged weapons */
        if (Role_if(PM_PRIEST)) {
            in_hand = wielding_artifact(ART_MJOLLNIR);
            already_exists =
                exist_artifact(HEAVY_WAR_HAMMER, artiname(ART_MJOLLNIR));
        } else {
            in_hand = wielding_artifact(ART_STORMBRINGER);
            already_exists =
                exist_artifact(RUNESWORD, artiname(ART_STORMBRINGER));
        }
        if (Role_if(PM_PRIEST)) {
            verbalize("Thou art chosen to take lives for My Glory!");
            livelog_printf(LL_DIVINEGIFT, "was chosen to take lives for the Glory of %s",
                           u_gname());

        } else {
            verbalize("Thou art chosen to %s for My Glory!",
                      ((already_exists && !in_hand)
                       || class_gift != STRANGE_OBJECT) ? "take lives"
                      : "steal souls");
            livelog_printf(LL_DIVINEGIFT, "was chosen to %s for the Glory of %s",
                           ((already_exists && !in_hand) || class_gift != STRANGE_OBJECT)
                           ? "take lives" : "steal souls", u_gname());
        }
        break;
    case A_NONE:
        u.uevent.uhand_of_elbereth = 4;
        if (Role_if(PM_INFIDEL)) {
            verbalize("Thou shalt be my vassal of suffering and terror!");
            livelog_printf(LL_DIVINEGIFT, "became the Emissary of Moloch");
            class_gift = SPE_FIREBALL; /* no special weapon */
            unrestrict_weapon_skill(P_TRIDENT);
            P_MAX_SKILL(P_TRIDENT) = P_EXPERT;
            if (Upolyd)
                rehumanize(); /* return to original form -- not a demon yet */
            /* lose ALL old racial abilities */
            adjabil(u.ulevel, 0);
            maxint = urace.attrmax[A_INT];
            maxwis = urace.attrmax[A_WIS];
            urace = race_demon;
            /* mental faculties are not changed by demonization */
            urace.attrmax[A_INT] = maxint;
            urace.attrmax[A_WIS] = maxwis;
            youmonst.data->msize = MZ_HUMAN; /* in case we started out as a giant */
            /* gain demonic resistances */
            adjabil(0, u.ulevel);
            /* resistances - not shock res because that can be gained by leveling
               up, and not cold res because demons and cold typically don't mix */
            incr_resistance(&HSleep_resistance, 100);
            /* move this line so adjabil doesn't flash e.g. warning on and off */
            pline1("Wings sprout from your back and you grow a barbed tail!");
            set_uasmon();
            newsym(u.ux, u.uy);
            retouch_equipment(2); /* silver */
            monstseesu(M_SEEN_FIRE | M_SEEN_POISON | M_SEEN_SLEEP);
            break;
        }
    }

    if (objects[class_gift].oc_class == SPBOOK_CLASS) {
        obj = mksobj(class_gift, TRUE, FALSE);
        if (!u.uconduct.literate && !known_spell(obj->otyp)) {
            if (force_learn_spell(obj->otyp))
                pline("Divine knowledge of %s fills your mind!",
                      OBJ_NAME(objects[obj->otyp]));
            obfree(obj, (struct obj *) 0);
        } else {
            bless(obj);
            obj->bknown = 1; /* ok to skip set_bknown() */
            at_your_feet("A spellbook");
            place_object(obj, u.ux, u.uy);
            newsym(u.ux, u.uy);
        }
        u.ugifts++;
        /* when getting a new book for known spell, enhance
           currently wielded weapon rather than the book */
        if (known_spell(class_gift) && ok_wep(uwep))
            obj = uwep; /* to be blessed,&c */
    }

    switch (u.ualign.type) {
    case A_LAWFUL:
        if (class_gift != STRANGE_OBJECT) {
            ; /* already got bonus above */
        } else if (obj && obj->otyp == LONG_SWORD && !obj->oartifact
                   && !Role_if(PM_PRIEST)) {
            if (!Blind)
                Your("sword shines brightly for a moment.");
            obj = oname(obj, artiname(ART_EXCALIBUR));
            if (obj && obj->oartifact == ART_EXCALIBUR) {
                u.ugifts++;
                u.uconduct.artitouch++;
            }
        } else if (obj && obj->otyp == HEAVY_WAR_HAMMER && !obj->oartifact
                   && Role_if(PM_PRIEST)) {
            if (!Blind)
                Your("hammer shines brightly for a moment.");
            obj = oname(obj, artiname(ART_MJOLLNIR));
            if (obj && obj->oartifact == ART_MJOLLNIR) {
                u.ugifts++;
                u.uconduct.artitouch++;
            }
        }
        /* acquire Excalibur's skill regardless of weapon or gift
           (non-priests only) */
        if (!Role_if(PM_PRIEST))
            unrestrict_weapon_skill(P_LONG_SWORD);
        if (obj && obj->oartifact == ART_EXCALIBUR)
            discover_artifact(ART_EXCALIBUR);
        if (obj && obj->oartifact == ART_MJOLLNIR)
            discover_artifact(ART_MJOLLNIR);
        break;
    case A_NEUTRAL:
        if (class_gift != STRANGE_OBJECT) {
            ; /* already got bonus above */
        } else if (obj && in_hand) {
            if (!Role_if(PM_PRIEST))
                Your("%s goes snicker-snack!", xname(obj));
            obj->dknown = TRUE;
        } else if (!already_exists) {
            if (Role_if(PM_PRIEST)) {
                obj = mksobj(HEAVY_WAR_HAMMER, FALSE, FALSE);
                obj = oname(obj, artiname(ART_MJOLLNIR));
                obj->spe = 1;
                at_your_feet("A hammer");
            } else {
                obj = mksobj(LONG_SWORD, FALSE, FALSE);
                obj = oname(obj, artiname(ART_VORPAL_BLADE));
                obj->spe = 1;
                at_your_feet("A sword");
            }
            dropy(obj);
            u.ugifts++;
        }
        /* acquire Vorpal Blade's skill regardless of weapon or gift */
        if (!Role_if(PM_PRIEST))
            unrestrict_weapon_skill(P_LONG_SWORD);
        if (obj && obj->oartifact == ART_VORPAL_BLADE)
            discover_artifact(ART_VORPAL_BLADE);
        if (obj && obj->oartifact == ART_MJOLLNIR)
            discover_artifact(ART_MJOLLNIR);
        break;
    case A_CHAOTIC: {
        char swordbuf[BUFSZ];

        Sprintf(swordbuf, "%s sword", hcolor(NH_BLACK));
        if (class_gift != STRANGE_OBJECT) {
            ; /* already got bonus above */
        } else if (obj && in_hand) {
            if (!Role_if(PM_PRIEST))
                Your("%s hums ominously!", swordbuf);
            obj->dknown = TRUE;
        } else if (!already_exists) {
            if (Role_if(PM_PRIEST)) {
                obj = mksobj(HEAVY_WAR_HAMMER, FALSE, FALSE);
                obj = oname(obj, artiname(ART_MJOLLNIR));
                obj->spe = 1;
                at_your_feet("A hammer");
            } else {
                obj = mksobj(RUNESWORD, FALSE, FALSE);
                obj = oname(obj, artiname(ART_STORMBRINGER));
                obj->spe = 1;
                at_your_feet(An(swordbuf));
            }
            dropy(obj);
            u.ugifts++;
        }
        /* acquire Stormbringer's skill regardless of weapon or gift */
        if (!Role_if(PM_PRIEST))
            unrestrict_weapon_skill(P_BROAD_SWORD);
        if (obj && obj->oartifact == ART_STORMBRINGER)
            discover_artifact(ART_STORMBRINGER);
        if (obj && obj->oartifact == ART_MJOLLNIR)
            discover_artifact(ART_MJOLLNIR);
        break;
    }
    case A_NONE:
        /* OK, we don't get an artifact, but surely Moloch
         * can at least offer His own blessing? */
        obj = uwep;
        if (ok_wep(obj) && !obj->oartifact
            && obj->quan == 1 && (obj->oprops & ITEM_PROP_MASK) == 0L) {
            Your("%s is wreathed in hellfire!", simple_typename(obj->otyp));
            obj->oprops |= ITEM_FIRE;
            obj->oprops_known |= ITEM_FIRE;
        }
        break;
    default:
        obj = 0; /* lint */
        break;
    }

    /* enhance weapon regardless of alignment or artifact status */
    if (ok_wep(obj)) {
        if (u.ualign.type == A_NONE)
            curse(obj);
        else
            bless(obj);
        obj->oeroded = obj->oeroded2 = 0;
        maybe_erodeproof(obj, 1);
        obj->bknown = obj->rknown = 1; /* ok to skip set_bknown() */
        if (obj->spe < 1)
            obj->spe = 1;
        /* acquire skill in this weapon */
        unrestrict_weapon_skill(weapon_type(obj));
    } else if (class_gift == STRANGE_OBJECT) {
        /* opportunity knocked, but there was nobody home... */
        You_feel("unworthy.");
    }
    update_inventory();

    /* lastly, confer an extra skill slot/credit beyond the
       up-to-29 you can get from gaining experience levels */
    add_weapon_skill(1);
    return;
}

STATIC_OVL void
pleased(g_align)
aligntyp g_align;
{
    /* don't use p_trouble, worst trouble may get fixed while praying */
    int trouble = in_trouble(); /* what's your worst difficulty? */
    int pat_on_head = 0, kick_on_butt;

    You_feel("that %s is %s.", align_gname(g_align),
             (u.ualign.record >= DEVOUT)
                 ? Hallucination ? "pleased as punch" : "well-pleased"
                 : (u.ualign.record >= STRIDENT)
                       ? Hallucination ? "ticklish" : "pleased"
                       : Hallucination ? "full" : "satisfied");

    /* not your deity */
    if (on_altar() && p_aligntyp != u.ualign.type) {
        You_feel("guilty.");
        adjalign(-1);
        return;
    } else if (u.ualign.record < 2 && trouble <= 0)
        adjalign(1);

    /*
     * Depending on your luck & align level, the god you prayed to will:
     *  - fix your worst problem if it's major;
     *  - fix all your major problems;
     *  - fix your worst problem if it's minor;
     *  - fix all of your problems;
     *  - do you a gratuitous favor.
     *
     * If you make it to the last category, you roll randomly again
     * to see what they do for you.
     *
     * If your luck is at least 0, then you are guaranteed rescued from
     * your worst major problem.
     */
    if (!trouble && u.ualign.record >= DEVOUT) {
        /* if hero was in trouble, but got better, no special favor */
        if (p_trouble == 0)
            pat_on_head = 1;
    } else {
        int action, prayer_luck;
        int tryct = 0;

        /* Negative luck is normally impossible here (can_pray() forces
           prayer failure in that situation), but it's possible for
           Luck to drop during the period of prayer occupation and
           become negative by the time we get here.  [Reported case
           was lawful character whose stinking cloud caused a delayed
           killing of a peaceful human, triggering the "murderer"
           penalty while successful prayer was in progress.  It could
           also happen due to inconvenient timing on Friday 13th, but
           the magnitude there (-1) isn't big enough to cause trouble.]
           We don't bother remembering start-of-prayer luck, just make
           sure it's at least -1 so that Luck+2 is big enough to avoid
           a divide by zero crash when generating a random number.  */
        prayer_luck = max(Luck, -1); /* => (prayer_luck + 2 > 0) */
        action = rn1(prayer_luck + (on_altar() ? 3 + on_shrine() : 2), 1);
        if (!on_altar())
            action = min(action, 3);
        if (u.ualign.record < STRIDENT)
            action = (u.ualign.record > 0 || !rnl(2)) ? 1 : 0;

        switch (min(action, 5)) {
        case 5:
            pat_on_head = 1;
            /*FALLTHRU*/
        case 4:
            do
                fix_worst_trouble(trouble);
            while ((trouble = in_trouble()) != 0);
            break;

        case 3:
            fix_worst_trouble(trouble);
        case 2:
            /* arbitrary number of tries */
            while ((trouble = in_trouble()) > 0 && (++tryct < 10))
                fix_worst_trouble(trouble);
            break;

        case 1:
            if (trouble > 0)
                fix_worst_trouble(trouble);
        case 0:
            break; /* your god blows you off, too bad */
        }
    }

    /* note: can't get pat_on_head unless all troubles have just been
       fixed or there were no troubles to begin with; hallucination
       won't be in effect so special handling for it is superfluous.
       Cavepersons are sometimes ignored by their god */
    if (pat_on_head && (Role_if(PM_CAVEMAN) ? rn2(10) : 1))
        switch (rn2((Luck + 6) >> 1)) {
        case 0:
            break;
        case 1:
            if (uwep && (welded(uwep) || uwep->oclass == WEAPON_CLASS
                         || is_weptool(uwep))) {
                char repair_buf[BUFSZ];

                *repair_buf = '\0';
                if (uwep->oeroded || uwep->oeroded2)
                    Sprintf(repair_buf, " and %s now as good as new",
                            otense(uwep, "are"));

                if (uwep->cursed) {
                    if (!Blind) {
                        pline("%s %s%s.", Yobjnam2(uwep, "softly glow"),
                              (g_align == A_NONE ? hcolor(NH_BLACK)
                                                 : hcolor(NH_AMBER)), repair_buf);
                        iflags.last_msg = PLNMSG_OBJ_GLOWS;
                    } else
                        You_feel("the power of %s over %s.", u_gname(),
                                 yname(uwep));
                    if (g_align == A_NONE)
                        ; /* already cursed */
                    else
                        uncurse(uwep);
                    uwep->bknown = 1; /* ok to bypass set_bknown() */
                    *repair_buf = '\0';
                } else if (!uwep->blessed) {
                    if (!Blind) {
                        pline("%s with %s aura%s.",
                              Yobjnam2(uwep, "softly glow"),
                              (g_align == A_NONE ? an(hcolor(NH_BLACK))
                                                 : an(hcolor(NH_LIGHT_BLUE))), repair_buf);
                        iflags.last_msg = PLNMSG_OBJ_GLOWS;
                    } else
                        You_feel("the %s of %s over %s.",
                                 (g_align == A_NONE ? "power" : "blessing"), u_gname(),
                                 yname(uwep));
                    if (g_align == A_NONE)
                        curse(uwep);
                    else
                        bless(uwep);
                    uwep->bknown = 1; /* ok to bypass set_bknown() */
                    *repair_buf = '\0';
                }

                /* fix any rust/burn/rot damage, but don't protect
                   against future damage */
                if (uwep->oeroded || uwep->oeroded2) {
                    uwep->oeroded = uwep->oeroded2 = 0;
                    /* only give this message if we didn't just bless
                       or uncurse (which has already given a message) */
                    if (*repair_buf)
                        pline("%s as good as new!",
                              Yobjnam2(uwep, Blind ? "feel" : "look"));
                }
                update_inventory();
            }
            break;
        case 3:
            /* takes 2 hints to get the music to enter the stronghold;
               skip if you've solved it via mastermind or destroyed the
               drawbridge (both set uopened_dbridge) or if you've already
               travelled past the Valley of the Dead (gehennom_entered) */
            if (!u.uevent.uopened_dbridge && !u.uevent.gehennom_entered) {
                if (u.uevent.uheard_tune < 1) {
                    godvoice(g_align, (char *) 0);
                    verbalize("Hark, %s!", youmonst.data->mlet == S_HUMAN
                                               ? "mortal"
                                               : "creature");
                    verbalize(
                       "To enter the castle, thou must play the right tune!");
                    u.uevent.uheard_tune++;
                    break;
                } else if (u.uevent.uheard_tune < 2) {
                    You_hear("a divine music...");
                    pline("It sounds like:  \"%s\".", tune);
                    u.uevent.uheard_tune++;
                    break;
                }
            }
            /*FALLTHRU*/
        case 2:
            if (!Blind)
                You("are surrounded by %s %s.",
                    (g_align == A_NONE ? an(hcolor(NH_BLACK)) : an(hcolor(NH_GOLDEN))),
                    (g_align == A_NONE ? "mist" : "glow"));
            /* if any levels have been lost (and not yet regained),
               treat this effect like blessed full healing */
            if (u.ulevel < u.ulevelmax) {
                u.ulevelmax -= 1; /* see potion.c */
                pluslvl(FALSE);
            } else {
                u.uhpmax += 5;
                if (Upolyd)
                    u.mhmax += 5;
            }
            u.uhp = u.uhpmax;
            if (Upolyd)
                u.mh = u.mhmax;
            if (ABASE(A_STR) < AMAX(A_STR)) {
                ABASE(A_STR) = AMAX(A_STR);
                context.botl = 1; /* before potential message */
                (void) encumber_msg();
            }
            if (u.uhunger < 900)
                init_uhunger();
            /* luck couldn't have been negative at start of prayer because
               the prayer would have failed, but might have been decremented
               due to a timed event (delayed death of peaceful monster hit
               by hero-created stinking cloud) during the praying interval */
            if (u.uluck < 0)
                u.uluck = 0;
            /* superfluous; if hero was blinded we'd be handling trouble
               rather than issuing a pat-on-head */
            u.ucreamed = 0;
            make_blinded(0L, TRUE);
            context.botl = 1;
            break;
        case 4: {
            register struct obj *otmp;
            int any = 0;

            if (Blind)
                You_feel("the power of %s.", u_gname());
            else
                You("are surrounded by %s aura.",
                    (g_align == A_NONE ? an(hcolor(NH_BLACK)) : an(hcolor(NH_LIGHT_BLUE))));
            for (otmp = invent; otmp; otmp = otmp->nobj) {
                if (otmp->cursed && g_align != A_NONE
                    && (otmp != uarmh /* [see worst_cursed_item()] */
                        || uarmh->otyp != HELM_OF_OPPOSITE_ALIGNMENT)) {
                    if (!Blind) {
                        pline("%s %s.", Yobjnam2(otmp, "softly glow"),
                              hcolor(NH_AMBER));
                        iflags.last_msg = PLNMSG_OBJ_GLOWS;
                        otmp->bknown = 1; /* ok to bypass set_bknown() */
                        ++any;
                    }
                    uncurse(otmp);
                /* Moloch will curse any blessed/uncursed piece of armor or weapon */
                } else if (!otmp->cursed && g_align == A_NONE
                    && (otmp->oclass == ARMOR_CLASS || otmp->oclass == WEAPON_CLASS)) {
                    if (!Blind) {
                        pline("%s %s.", Yobjnam2(otmp, "softly glow"),
                              hcolor(NH_BLACK));
                        iflags.last_msg = PLNMSG_OBJ_GLOWS;
                        otmp->bknown = 1; /* ok to bypass set_bknown() */
                        ++any;
                    }
                    curse(otmp);
                }
            }
            if (any)
                update_inventory();
            break;
        }
        case 5: {
            static NEARDATA const char msg[] =
                "\"and thus I grant thee the gift of %s!\"";

            godvoice(u.ualign.type,
                     "Thou hast pleased me with thy progress,");
            if (!(HTelepat & INTRINSIC)) {
                HTelepat |= FROMOUTSIDE;
                pline(msg, "Telepathy");
                if (Blind)
                    see_monsters();
            } else if (!(HFast & INTRINSIC)) {
                HFast |= FROMOUTSIDE;
                pline(msg, "Speed");
            } else if (!(HStealth & INTRINSIC)) {
                HStealth |= FROMOUTSIDE;
                pline(msg, "Stealth");
            } else {
                if (!(HProtection & INTRINSIC)) {
                    HProtection |= FROMOUTSIDE;
                    if (!u.ublessed)
                        u.ublessed = rn1(3, 2);
                } else
                    u.ublessed++;
                pline(msg, "my protection");
            }
            verbalize("Use it wisely in my name!");
            break;
        }
        case 7:
        case 8:
            if (u.ualign.record >= PIOUS && !u.uevent.uhand_of_elbereth) {
                gcrownu();
                break;
            }
            /*FALLTHRU*/
        case 6: {
            struct obj *otmp;
            int trycnt = u.ulevel + 1;

            /* cavepersons don't mess around with spells, so do nothing */
            if (Role_if(PM_CAVEMAN)) {
                break;
            } else {
                /* not yet known spells given preference over already known ones.
                   Also, try to grant a spell for which there is a skill slot */
                otmp = mkobj(SPBOOK_CLASS, TRUE);
                while (--trycnt > 0) {
                    if (otmp->otyp != SPE_BLANK_PAPER) {
                        if (!known_spell(otmp->otyp)
                            && !P_RESTRICTED(spell_skilltype(otmp->otyp)))
                            break; /* usable, but not yet known */
                    } else {
                        if ((!objects[SPE_BLANK_PAPER].oc_name_known
                             || carrying(MAGIC_MARKER)) && u.uconduct.literate)
                            break;
                    }
                    otmp->otyp = rnd_class(bases[SPBOOK_CLASS], SPE_FREEZE_SPHERE);
                    set_material(otmp, objects[otmp->otyp].oc_material);
                }
                if (!u.uconduct.literate && (otmp->otyp != SPE_BLANK_PAPER)
                    && !known_spell(otmp->otyp)) {
                    if (force_learn_spell(otmp->otyp))
                        pline("Divine knowledge of %s fills your mind!",
                              OBJ_NAME(objects[otmp->otyp]));
                    obfree(otmp, (struct obj *) 0);
                } else {
                    bless(otmp);
                    otmp->oeroded = otmp->oeroded2 = 0;
                    at_your_feet("A spellbook");
                    place_object(otmp, u.ux, u.uy);
                    newsym(u.ux, u.uy);
                }
            }
            break;
        }
        default:
            impossible("Confused deity!");
            break;
        } else if (pat_on_head) {
            You_feel("that %s is not entirely paying attention.",
                     align_gname(g_align));
        }

    u.ublesscnt = rnz(350);
    kick_on_butt = u.uevent.udemigod ? 1 : 0;
    if (u.uevent.uhand_of_elbereth)
        kick_on_butt++;
    if (kick_on_butt)
        u.ublesscnt += kick_on_butt * rnz(1000);

    /* Avoid games that go into infinite loops of copy-pasted commands with no
       human interaction; this is a DoS vector against the computer running
       NetHack. Once the turn counter is over 100000, every additional 100 turns
       increases the prayer timeout by 1, thus eventually nutrition prayers will
       fail and some other source of nutrition will be required. */
    if (moves > 100000L)
        u.ublesscnt += (moves - 100000L) / 100;

    return;
}

/* either blesses or curses water on the altar,
 * returns true if it found any water here.
 */
STATIC_OVL boolean
water_prayer(bless_water)
boolean bless_water;
{
    register struct obj *otmp;
    register long changed = 0;
    boolean other = FALSE, bc_known = !(Blind || Hallucination);

    for (otmp = level.objects[u.ux][u.uy]; otmp; otmp = otmp->nexthere) {
        /* turn water into (un)holy water */
        if (otmp->otyp == POT_WATER
            && (bless_water ? !otmp->blessed : !otmp->cursed)) {
            otmp->blessed = bless_water;
            otmp->cursed = !bless_water;
            otmp->bknown = bc_known; /* ok to bypass set_bknown() */
            changed += otmp->quan;
        } else if (otmp->oclass == POTION_CLASS)
            other = TRUE;
    }
    if (!Blind && changed) {
        pline("%s potion%s on the altar glow%s %s for a moment.",
              ((other && changed > 1L) ? "Some of the"
                                       : (other ? "One of the" : "The")),
              ((other || changed > 1L) ? "s" : ""), (changed > 1L ? "" : "s"),
              (bless_water ? hcolor(NH_LIGHT_BLUE) : hcolor(NH_BLACK)));
    }
    return (boolean) (changed > 0L);
}

int
check_malign(mtmp)
register struct monst *mtmp;
{
    aligntyp mon_align = has_erac(mtmp) ? ERAC(mtmp)->ralign
                                        : mtmp->data->maligntyp;
    if (mon_align < 0) {
        return A_CHAOTIC;
    } else if (mon_align > 0) {
        return A_LAWFUL;
    } else {
        return A_NEUTRAL;
    }
}

boolean
moffer(mtmp)
register struct monst *mtmp;
{
    register struct obj *otmp;
    /* loop based on select_hwep */
    for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj) {
        if (otmp->otyp == AMULET_OF_YENDOR && In_endgame(&u.uz)
            && a_align(mtmp->mx, mtmp->my) == check_malign(mtmp)) {
        pline("%s raises the Amulet of Yendor high above the altar!",
              Monnam(mtmp));
        /* game is now unwinnable... oops */
        m_useup(mtmp, otmp);
        livelog_printf(LL_ARTIFACT, "failed their quest! %s sacrificed the Amulet of Yendor, ascending!",
                       Monnam(mtmp));
        if (is_demon(mtmp->data) || mtmp->iswiz) {
            pline("%s gains ultimate power, laughs fiendishly, and erases you from existence.",
                  Monnam(mtmp));
            Sprintf(killer.name, "%s wrath", s_suffix(mon_nam(mtmp)));
            killer.format = KILLED_BY;
            done(DIED);
        } else {
            pline("%s accepts the Amulet and gains dominion over the gods, and %s ascends to demigodhood!",
                  a_gname_at(mtmp->mx, mtmp->my), mon_nam(mtmp));
            if (Luck >= 10) {
                pline("Luckily for you, %s does not smite you with their newfound power, and you are allowed to live.",
                      mon_nam(mtmp));
                pline("However, your quest ends here...");
                done(ESCAPED);
            } else {
                pline("The Demigod of %s looks down upon you, and squashes you like the ant that you are.",
                      a_gname_at(mtmp->mx, mtmp->my));
                Sprintf(killer.name, "%s indifference", s_suffix(mon_nam(mtmp)));
                killer.format = KILLED_BY;
                done(DIED);
            }
        }
        return 3;
        }
    }
    return 0;
}

void
godvoice(g_align, words)
aligntyp g_align;
const char *words;
{
    const char *quot = "";

    if (words)
        quot = "\"";
    else
        words = "";

    pline_The("voice of %s %s: %s%s%s", align_gname(g_align),
              godvoices[rn2(SIZE(godvoices))], quot, words, quot);
}

STATIC_OVL void
gods_angry(g_align)
aligntyp g_align;
{
    godvoice(g_align, "Thou hast angered me.");
}

/* The g_align god is upset with you. */
STATIC_OVL void
gods_upset(g_align)
aligntyp g_align;
{
    if (g_align == u.ualign.type)
        u.ugangr++;
    else if (u.ugangr)
        u.ugangr--;
    angrygods(g_align);
}

STATIC_OVL void
consume_offering(otmp)
register struct obj *otmp;
{
    if (Hallucination)
        switch (rn2(3)) {
        case 0:
            Your("sacrifice sprouts wings and a propeller and roars away!");
            break;
        case 1:
            Your("sacrifice puffs up, swelling bigger and bigger, and pops!");
            break;
        case 2:
            Your(
     "sacrifice collapses into a cloud of dancing particles and fades away!");
            break;
        }
    else if (Blind && u.ualign.type == A_LAWFUL)
        Your("sacrifice disappears!");
    else
        Your("sacrifice is consumed in a %s!",
             u.ualign.type == A_LAWFUL
                 ? "flash of light"
                 : u.ualign.type == A_NEUTRAL
                     ? "cloud of smoke"
                     : u.ualign.type == A_NONE
                         ? "torrent of hellfire"
                         : "burst of flame");
    if (carried(otmp))
        useup(otmp);
    else
        useupf(otmp, 1L);
    exercise(A_WIS, TRUE);
}

struct inv_sub { short race_pm, item_otyp, subs_otyp; };
extern struct inv_sub inv_subs[];

int
dosacrifice()
{
    static NEARDATA const char cloud_of_smoke[] =
        "A cloud of %s smoke surrounds you...";
    register struct obj *otmp;
    int value = 0, pm;
    boolean highaltar;
    aligntyp altaralign = a_align(u.ux, u.uy);

    if (!on_altar() || u.uswallow) {
        You("are not standing on an altar.");
        return 0;
    }

    if (Hidinshell) {
        You_cant("offer a sacrifice while hiding in your shell.");
        return 0;
    }

    highaltar = ((Is_astralevel(&u.uz) || Is_sanctum(&u.uz))
                 && (levl[u.ux][u.uy].altarmask & AM_SHRINE));

    otmp = floorfood("sacrifice", 1);
    if (!otmp)
        return 0;
    /*
     * Was based on nutritional value and aging behavior (< 50 moves).
     * Sacrificing a food ration got you max luck instantly, making the
     * gods as easy to please as an angry dog!
     *
     * Now only accepts corpses, based on the game's evaluation of their
     * toughness.  Human and pet sacrifice, as well as sacrificing unicorns
     * of your alignment, is strongly discouraged.
     */
#define MAXVALUE 24 /* Highest corpse value (besides Wiz) */

    /* sacrificing the Eye of Vecna is a special case */
    if (otmp->oartifact == ART_EYE_OF_VECNA) {
        You("offer this evil abomination to %s...", a_gname());
        value = MAXVALUE; /* woop */
        /* KMH, conduct */
        if (!u.uconduct.gnostic++)
            livelog_printf(LL_CONDUCT,
                    "rejected atheism by offering %s on an altar of %s",
                    The(xname(otmp)), a_gname());
    }

    if (otmp->otyp == CORPSE) {
        struct permonst *ptr;
        struct monst *mtmp;
        boolean to_other_god;

        if (has_omonst(otmp) && has_erac(OMONST(otmp))) {
            ptr = &mons[ERAC(OMONST(otmp))->rmnum];
        } else {
            ptr = &mons[otmp->corpsenm];
        }
        /* is this a conversion attempt? */
        to_other_god = (ugod_is_angry() && !your_race(ptr)
                        && u.ualign.type != altaralign);

        /* KMH, conduct */
        if (!u.uconduct.gnostic++)
            livelog_printf(LL_CONDUCT,
                    "rejected atheism by offering %s on an altar of %s",
                    corpse_xname(otmp, (const char *) 0, CXN_ARTICLE),
                    a_gname());

        /* you're handling this corpse, even if it was killed upon the altar
         */
        feel_cockatrice(otmp, TRUE);
        if (rider_corpse_revival(otmp, FALSE))
            return 1;

        if (otmp->corpsenm == PM_ACID_BLOB || your_race(ptr)
            || (monstermoves <= peek_at_iced_corpse_age(otmp) + 50)) {
            value = mons[otmp->corpsenm].difficulty + 1;
            /* Not demons--no demon corpses */
            if (is_undead(ptr) && u.ualign.type > A_CHAOTIC)
                value += 1;
            if (is_unicorn(ptr))
                value += 3;
            if (wielding_artifact(ART_SECESPITA))
                value += value / 2;
            if (otmp->oeaten)
                value = eaten_stat(value, otmp);
            /* even cross-aligned sacrifices will count,
             * as long as they're ultimately to Moloch */
            if (u.ualign.type == A_NONE && !to_other_god) {
                long new_timeout = moves + value * 500;
                if (context.next_moloch_offering < new_timeout)
                    context.next_moloch_offering = new_timeout;
            }
        }

        if (your_race(ptr)
            || (Race_if(PM_ELF) && is_drow(ptr))
            || (Race_if(PM_DROW) && is_elf(ptr))) {
            if (is_demon(raceptr(&youmonst))) {
                You("find the idea very satisfying.");
                exercise(A_WIS, TRUE);
            } else if (u.ualign.type > A_CHAOTIC) {
                pline("You'll regret this infamous offense!");
                exercise(A_WIS, FALSE);
            }

            if (highaltar
                && (altaralign != A_CHAOTIC || u.ualign.type != A_CHAOTIC)
                && (altaralign != A_NONE || u.ualign.type != A_NONE)) {
                goto desecrate_high_altar;
            } else if (altaralign != A_CHAOTIC && altaralign != A_NONE) {
                /* curse the lawful/neutral altar */
                pline_The("altar is stained with %s blood.",
                          (Race_if(PM_DROW) && is_elf(ptr))
                            ? "elven" : (Race_if(PM_ELF) && is_drow(ptr))
                              ? "dark elven" : urace.adj);
                levl[u.ux][u.uy].altarmask = (u.ualign.type == A_NONE)
                                              ? AM_NONE : AM_CHAOTIC;
                newsym(u.ux, u.uy); /* in case Invisible to self */
                angry_priest();
                if (!canspotself())
                    /* with colored altars, regular newsym() doesn't cut it -
                     * it will see that the actual glyph is still the same, so
                     * the color won't be updated. This code must be added
                     * anywhere an altar mask could change. */
                    newsym_force(u.ux, u.uy);
            } else {
                struct monst *dmon;
                const char *demonless_msg;

                /* Human sacrifice on a chaotic or unaligned altar */
                /* is equivalent to demon summoning */
                if (altaralign == A_CHAOTIC && u.ualign.type != A_CHAOTIC) {
                    pline(
                    "The blood floods the altar, which vanishes in %s cloud!",
                          an(hcolor(NH_BLACK)));
                    levl[u.ux][u.uy].typ = ROOM;
                    levl[u.ux][u.uy].altarmask = 0;
                    newsym(u.ux, u.uy);
                    angry_priest();
                    demonless_msg = "cloud dissipates";
                    if (!canspotself())
                        newsym_force(u.ux, u.uy);
                } else {
                    /* either you're chaotic or altar is Moloch's or both */
                    pline_The("blood covers the altar!");
                    change_luck(altaralign == u.ualign.type ? 2 : -2);
                    demonless_msg = "blood coagulates";
                }
                if ((pm = ndemon(altaralign)) != NON_PM
                    && (dmon = makemon(&mons[pm], u.ux, u.uy, NO_MM_FLAGS))
                           != 0) {
                    char dbuf[BUFSZ];

                    Strcpy(dbuf, a_monnam(dmon));
                    if (!strcmpi(dbuf, "it"))
                        Strcpy(dbuf, "something dreadful");
                    else
                        dmon->mstrategy &= ~STRAT_APPEARMSG;
                    You("have summoned %s!", dbuf);
                    if (sgn(u.ualign.type) == sgn(check_malign(dmon))) {
                        if (rn2(5))
                            dmon->mpeaceful = TRUE;
                        else
                            verbalize("Who dares summon me?");
                    }
                    You("are terrified, and unable to move.");
                    nomul(-3);
                    multi_reason = "being terrified of a demon";
                    nomovemsg = 0;
                } else
                    pline_The("%s.", demonless_msg);
            }

            if (u.ualign.type > A_CHAOTIC) {
                You_feel("guilty.");
                adjalign(-5);
                u.ugangr += 3;
                (void) adjattrib(A_WIS, -1, TRUE);
                if (!mitre_inhell())
                    angrygods(u.ualign.type);
                change_luck(-5);
            } else
                adjalign(5);
            if (carried(otmp))
                useup(otmp);
            else
                useupf(otmp, 1L);

            /* create Dirge from player's longsword here if possible */
            if (u.ualign.type == A_CHAOTIC && Role_if(PM_KNIGHT)
                && !u.ugangr && u.ualign.record > 0
                && uwep && (uwep->otyp == LONG_SWORD
                            || uwep->otyp == ELVEN_LONG_SWORD
                            || uwep->otyp == ORCISH_LONG_SWORD
                            || uwep->otyp == DARK_ELVEN_LONG_SWORD)
                && !uwep->oartifact && !(uarmh && uarmh->otyp == HELM_OF_OPPOSITE_ALIGNMENT)
                && !exist_artifact(LONG_SWORD, artiname(ART_DIRGE))) {
                pline("Your sword melts in your hand and transforms into something new!");
                uwep->oprops = uwep->oprops_known = 0L;
                uwep->otyp = LONG_SWORD;
                uwep = oname(uwep, artiname(ART_DIRGE));
                discover_artifact(ART_DIRGE);
                bless(uwep);
                if (uwep->spe < 0)
                    uwep->spe = 0;
                uwep->oeroded = uwep->oeroded2 = 0;
                maybe_erodeproof(uwep, 1);
                exercise(A_WIS, TRUE);
                u.uconduct.artitouch++;
                livelog_printf(LL_DIVINEGIFT | LL_ARTIFACT,
                               "had Dirge gifted to %s by the grace of %s",
                               uhim(), align_gname(u.ualign.type));
                update_inventory();
            }
            return 1;
        } else if (has_omonst(otmp)
                   && (mtmp = get_mtraits(otmp, FALSE)) != 0
                   && mtmp->mtame
                   /* Moloch is OK with sacrificing pets,
                    * but make sure we're offering to him */
                   && (u.ualign.type != A_NONE || to_other_god)) {
                /* mtmp is a temporary pointer to a tame monster's attributes,
                 * not a real monster */
            pline("So this is how you repay loyalty?");
            adjalign(-3);
            value = -1;
            HAggravate_monster |= FROMOUTSIDE;
        } else if (is_unicorn(ptr) && value /* fresh */) {
            int unicalign = sgn(ptr->maligntyp);

            if (unicalign == altaralign) {
                /* When same as altar, always a very bad action.
                 */
                pline("Such an action is an insult to %s!",
                      (unicalign == A_CHAOTIC) ? "chaos"
                         : unicalign ? "law" : "balance");
                (void) adjattrib(A_WIS, -1, TRUE);
                value = -5;
            } else if (u.ualign.type == altaralign) {
                /* When different from altar, and altar is same as yours,
                 * it's a very good action.
                 */
                if (u.ualign.record < ALIGNLIM)
                    You_feel("appropriately %s.", align_str(u.ualign.type));
                else
                    You_feel("you are thoroughly on the right path.");
                adjalign(5);
                /* value += 3; -- now applied above */
            } else if (unicalign == u.ualign.type) {
                /* When sacrificing unicorn of your alignment to altar not of
                 * your alignment, your god gets angry and it's a conversion.
                 */
                u.ualign.record = -1;
                value = 1;
            } else {
                /* Otherwise, unicorn's alignment is different from yours
                 * and different from the altar's.  It's an ordinary (well,
                 * with a bonus) sacrifice on a cross-aligned altar.
                 */
                /* value += 3; -- now applied above */
            }
        }
    } /* corpse */

    if (otmp->otyp == AMULET_OF_YENDOR) {
        if (!highaltar) {
 too_soon:
            if (altaralign == A_NONE && u.ualign.type != A_NONE && Inhell)
                /* hero has left Moloch's Sanctum so is in the process
                   of getting away with the Amulet (outside of Gehennom,
                   fall through to the "ashamed" feedback) */
                gods_upset(A_NONE);
            else
                You_feel("%s.",
                         Hallucination
                            ? "homesick"
                            /* if on track, give a big hint */
                            : (altaralign == u.ualign.type)
                               ? (Role_if(PM_INFIDEL)
                                  ? "an urge to descend deeper"
                                  : "an urge to return to the surface")
                               /* else headed towards celestial disgrace */
                               : "ashamed");
            return 1;
        } else if (!rn2(3) && Is_astralevel(&u.uz)
                   && uarmh && uarmh->otyp == HELM_OF_OPPOSITE_ALIGNMENT) {
            /* just got caught trying to be sneaky */
            You("offer the Amulet of Yendor to %s... but %s has noticed your deception!",
                a_gname(), align_gname(u.ualignbase[A_CURRENT]));
            /* your god isn't thrilled about it */
            godvoice(u.ualignbase[A_CURRENT], "Thou shall pay for thy trickery, mortal!");
            godvoice(u.ualignbase[A_CURRENT], "Bring unto me what is rightfully mine!");
            /* say goodbye to the HoOA */
            destroy_arm(uarmh);
            change_luck(-5);
            adjalign(-5);
            /* and forget about ever being able to pray.
             * at least the hero is able to continue and
             * perhaps redeem themselves */
            gods_upset(u.ualignbase[A_CURRENT]);
            return 1;
        } else {
            /* The final Test.  Did you win? */
            if (uamul == otmp)
                Amulet_off();
            u.uevent.ascended = 1;
            if (carried(otmp))
                useup(otmp); /* well, it's gone now */
            else
                useupf(otmp, 1L);
            You("offer the Amulet of Yendor to %s...", a_gname());
            if (altaralign == A_NONE) {
                /* Moloch's high altar */
                if (Role_if(PM_INFIDEL)) {
                    /* Infidels still have an ascension run,
                     * they just carry a different McGuffin */
                    u.uevent.ascended = 0;
                    otmp = find_quest_artifact(1 << OBJ_INVENT);
                    godvoice(A_NONE, (char *) 0);
                    if (!otmp)
                        qt_pager(QT_MOLOCH_1);
                    else {
                        qt_pager(QT_MOLOCH_2);
                        if (otmp->where == OBJ_CONTAINED) {
                            /* the Idol cannot be contained now,
                             * so we have to remove it */
                            obj_extract_self(otmp);
                            (void) hold_another_object(otmp, "Oops!",
                                                       (const char *) 0,
                                                       (const char *) 0);
                        }
                        You_feel("strange energies envelop %s.",
                                 the(xname(otmp)));
                        otmp->spe = 1;
                        if (otmp->where == OBJ_INVENT) {
                            u.uhave.amulet = 1;
                            u.uachieve.amulet = 1;
                            mkgate();
                        }
                        livelog_write_string(LL_ACHIEVE, "imbued the Idol of Moloch");
                    }
                    return 1;
                }
                if (u.ualign.record > -99)
                    u.ualign.record = -99;
                /*[apparently shrug/snarl can be sensed without being seen]*/
                pline("%s shrugs and retains dominion over %s,", Moloch,
                      u_gname());
                pline("then mercilessly snuffs out your life.");
                Sprintf(killer.name, "%s indifference", s_suffix(Moloch));
                killer.format = KILLED_BY;
                done(DIED);
                /* life-saved (or declined to die in wizard/explore mode) */
                pline("%s snarls and tries again...", Moloch);
                fry_by_god(A_NONE, TRUE); /* wrath of Moloch */
                /* declined to die in wizard or explore mode */
                pline(cloud_of_smoke, hcolor(NH_BLACK));
                done(ESCAPED);
            } else if (u.ualign.type != altaralign) {
                /* And the opposing team picks you up and
                   carries you off on their shoulders */
                adjalign(-99);
                pline("%s accepts your gift, and gains dominion over %s...",
                      a_gname(), u_gname());
                pline("%s is enraged...", u_gname());
                pline("Fortunately, %s permits you to live...", a_gname());
                pline(cloud_of_smoke, hcolor(NH_ORANGE));
                done(ESCAPED);
            } else { /* super big win */
                adjalign(10);
                u.uachieve.ascended = 1;
                pline(
               "An invisible choir sings, and you are bathed in radiance...");
                godvoice(altaralign, "Mortal, thou hast done well!");
                display_nhwindow(WIN_MESSAGE, FALSE);
                verbalize(
          "In return for thy service, I grant thee the gift of Immortality!");
                You("ascend to the status of Demigod%s...",
                    flags.female ? "dess" : "");
                done(ASCENDED);
            }
        }
    } /* real Amulet */

    if (otmp->otyp == FAKE_AMULET_OF_YENDOR) {
        if (!highaltar && !otmp->known)
            goto too_soon;
        You_hear("a nearby thunderclap.");
        if (!otmp->known) {
            You("realize you have made a %s.",
                Hallucination ? "boo-boo" : "mistake");
            otmp->known = TRUE;
            change_luck(-1);
            return 1;
        } else {
            /* don't you dare try to fool the gods */
            if (Deaf)
                pline("Oh, no."); /* didn't hear thunderclap */
            change_luck(-3);
            if (u.ualign.type != A_NONE) {
                adjalign(-1);
                u.ugangr += 3;
            }
            value = -3;
        }
    } /* fake Amulet */

    if (value == 0) {
        pline1(nothing_happens);
        return 1;
    }

    if (altaralign != u.ualign.type && highaltar) {
 desecrate_high_altar:
        /*
         * REAL BAD NEWS!!! High altars cannot be converted.  Even an attempt
         * gets the god who owns it truly pissed off.
         */
        You_feel("the air around you grow charged...");
        pline("Suddenly, you realize that %s has noticed you...", a_gname());
        godvoice(altaralign,
                 "So, mortal!  You dare desecrate my High Temple!");
        /* Throw everything we have at the player */
        god_zaps_you(altaralign);
    } else if (value
               < 0) { /* I don't think the gods are gonna like this... */
        gods_upset(altaralign);
    } else {
        int saved_anger = u.ugangr;
        int saved_cnt = u.ublesscnt;
        int saved_luck = u.uluck;

        /* Sacrificing at an altar of a different alignment */
        if (u.ualign.type != altaralign) {
            /* Is this a conversion ? */
            /* An unaligned altar in Gehennom will always elicit rejection. */
            /* Infidels will also never be accepted. */
            if (ugod_is_angry() || (altaralign == A_NONE && Inhell)) {
                if (u.ualignbase[A_CURRENT] == u.ualignbase[A_ORIGINAL]
                    && altaralign != A_NONE && !Role_if(PM_INFIDEL)) {
                    You("have a strong feeling that %s is angry...",
                        u_gname());
                    consume_offering(otmp);
                    pline("%s accepts your allegiance.", a_gname());

                    uchangealign(altaralign, 0);
                    /* Beware, Conversion is costly */
                    change_luck(-3);
                    u.ublesscnt += 300;
                } else {
                    u.ugangr += 3;
                    adjalign(-5);
                    pline("%s rejects your sacrifice!", a_gname());
                    godvoice(altaralign, "Suffer, infidel!");
                    change_luck(-5);
                    (void) adjattrib(A_WIS, -2, TRUE);
                    if (!Inhell)
                        angrygods(u.ualign.type);
                }
                return 1;
            } else {
                consume_offering(otmp);
                You("sense a conflict between %s and %s.", u_gname(),
                    a_gname());
                if (rn2(8 + u.ulevel) > 5
                    /* Infidels have difficulty converting altars. */
                    && !(u.ualign.type == A_NONE
                         && !(Role_if(PM_INFIDEL) && u.uhave.questart)
                         && depth(&u.uz) < depth(&valley_level) && rn2(5))) {
                    struct monst *pri;
                    boolean shrine;

                    You_feel("the power of %s increase.", u_gname());
                    exercise(A_WIS, TRUE);
                    change_luck(1);
                    shrine = on_shrine();
                    levl[u.ux][u.uy].altarmask = Align2amask(u.ualign.type);
                    if (shrine)
                        levl[u.ux][u.uy].altarmask |= AM_SHRINE;
                    newsym(u.ux, u.uy); /* in case Invisible to self */
                    if (!Blind)
                        pline_The("altar glows %s.",
                                  hcolor((u.ualign.type == A_LAWFUL)
                                            ? NH_WHITE
                                            : u.ualign.type == A_NONE
                                                ? NH_RED
                                                : u.ualign.type
                                                    ? NH_BLACK
                                                    : (const char *) "gray"));

                    if (!canspotself())
                        newsym_force(u.ux, u.uy);

                    if (u.ualign.record > 0
                        && rnd(u.ualign.record) >
                        (3 * ALIGNLIM) / (temple_occupied(u.urooms)
                                          ? 12 : u.ulevel)) {
                        (void) summon_minion(altaralign, TRUE);
                    }
                    /* anger priest; test handles bones files */
                    if ((pri = findpriest(temple_occupied(u.urooms)))
                        && !p_coaligned(pri))
                        angry_priest();
                } else {
                    pline("Unluckily, you feel the power of %s decrease.",
                          u_gname());
                    change_luck(-1);
                    exercise(A_WIS, FALSE);
                    if (rnl(u.ulevel) > 6 && u.ualign.record > 0
                        && rnd(u.ualign.record) > (7 * ALIGNLIM) / 8)
                        (void) summon_minion(altaralign, TRUE);
                }
                return 1;
            }
        }

        consume_offering(otmp);
        /* OK, you get brownie points. */
        if (u.ugangr) {
            u.ugangr -= ((value * (u.ualign.type == A_NONE ? 3
                                   : u.ualign.type == A_CHAOTIC ? 4 : 6))
                         / (MAXVALUE * 2));
            if (u.ugangr < 0)
                u.ugangr = 0;
            if (u.ugangr != saved_anger) {
                if (u.ugangr) {
                    pline("%s seems %s.", u_gname(),
                          Hallucination ? "groovy" : "slightly mollified");

                    if ((int) u.uluck < 0)
                        change_luck(1);
                } else {
                    pline("%s seems %s.", u_gname(),
                          Hallucination ? "cosmic (not a new fact)"
                                        : "mollified");

                    if ((int) u.uluck < 0)
                        u.uluck = 0;
                }
            } else { /* not satisfied yet */
                if (Hallucination)
                    pline_The("gods seem tall.");
                else
                    You("have a feeling of inadequacy.");
            }
        } else if (ugod_is_angry()) {
            if (value > MAXVALUE)
                value = MAXVALUE;
            if (value > -u.ualign.record)
                value = -u.ualign.record;
            adjalign(value);
            You_feel("partially absolved.");
        } else if (u.ublesscnt > 0) {
            u.ublesscnt -= ((value * (u.ualign.type <= A_CHAOTIC ? 500 : 300))
                            / MAXVALUE);
            if (u.ublesscnt < 0)
                u.ublesscnt = 0;
            if (u.ublesscnt != saved_cnt) {
                if (u.ublesscnt) {
                    if (Hallucination)
                        You("realize that the gods are not like you and I.");
                    else
                        You("have a hopeful feeling.");
                    if ((int) u.uluck < 0)
                        change_luck(1);
                } else {
                    if (Hallucination)
                        pline("Overall, there is a smell of fried onions.");
                    else
                        You("have a feeling of reconciliation.");
                    if ((int) u.uluck < 0)
                        u.uluck = 0;
                }
            }
        } else {
            int nchance = u.ulevel + 6;
            /* having never abused your alignment slightly increases
               the odds of receiving a gift from your deity.
               the more artifacts the player wishes for, the lower
               the chances of receiving an artifact gift via sacrifice */
            int reg_gift_odds  = ((u.ualign.abuse == 0) ? 5 : 6) + (2 * u.ugifts);
            int arti_gift_odds = ((u.ualign.abuse == 0) ? 9 : 10) + (2 * u.ugifts) + (2 * u.uconduct.wisharti);
            boolean primary_casters, primary_casters_priest;

            /* Primary casting roles */
            primary_casters = Role_if(PM_HEALER)
                                      || Role_if(PM_WIZARD) || Role_if(PM_INFIDEL);
            primary_casters_priest = Role_if(PM_PRIEST);

            /* you were already in pretty good standing
             *
             * The player can gain an artifact;
             * The chance goes down as the number of artifacts goes up.
             *
             * From SporkHack (heavily modified):
             * The player can also get handed just a plain old hunk of
             * weaponry or piece of armor, but it will be blessed, +3 to +5,
             * fire/rustproof, and if it's a weapon, it'll be in one of the
             * player's available skill slots. The lower level you are, the
             * more likely it is that you'll get a hunk of ordinary junk
             * rather than an artifact.
             *
             * Note that no artifact is guaranteed; it's still subject to the
             * chances of generating one of those in the first place. These
             * are just the chances that an artifact will even be considered
             * as a gift.
             *
             * If your role has a guaranteed first sacrifice gift, you will
             * not receive a non-artifact item as a gift until you've gotten
             * your guaranteed artifact.
             *
             * level  4: 10% chance level  9: 20% chance level 12: 30% chance
             * level 14: 40% chance level 17: 50% chance level 19: 60% chance
             * level 21: 70% chance level 23: 80% chance level 24: 90% chance
             * level 26 or greater: 100% chance
             */
            if ((!awaiting_guaranteed_gift() || u.ulevel <= 2)
                && rn2(10) >= (int) ((nchance * nchance) / 100)) {
                if (u.uluck >= 0 && !rn2(reg_gift_odds)) {
                    int typ, ncount = 0;
                    if (rn2(2)) { /* Making a weapon */
                        do {
                            /* Don't give unicorn horns or anything the player's restricted in
                             * Lets also try to dish out suitable gear based on the player's role */
                            if (primary_casters) {
                                typ = rn2(2) ? rnd_class(DAGGER, ATHAME) : rnd_class(MACE, FLAIL);
                            } else if (primary_casters_priest) {
                                typ = rnd_class(MACE, FLAIL);
                            } else if (Role_if(PM_MONK)) {
                                if (!u.uconduct.weaphit)
                                    typ = SHURIKEN;
                                else
                                    typ = rn2(4) ? rnd_class(QUARTERSTAFF, STAFF_OF_WAR)
                                                 : BROADSWORD;
                            } else {
                                typ = rnd_class(SPEAR, KATANA);
                            }

                            /* apply starting inventory subs - so we'll get racial gear if possible */
                            if (urace.malenum != PM_HUMAN) {
                                int i;
                                for (i = 0; inv_subs[i].race_pm != NON_PM; ++i) {
                                    if (inv_subs[i].race_pm == urace.malenum
                                        && typ == inv_subs[i].item_otyp) {
                                        typ = inv_subs[i].subs_otyp;
                                        break;
                                    }
                                }
                            }

                            /* The issue here is it blocks elves from getting basically
                             * anything, since most (non-elven) weapons are base mat iron...
                             * if we have the WRONG race, then let's not do that
                             */
                            otmp = mksobj(typ, FALSE, FALSE);
                            if (is_elven_obj(otmp) && !Race_if(PM_ELF))
                                typ = 0;
                            else if (is_drow_obj(otmp) && !Race_if(PM_DROW))
                                typ = 0;
                            else if (is_orcish_obj(otmp) && !Race_if(PM_ORC))
                                typ = 0;

                            obfree(otmp, (struct obj *) 0);
                            otmp = (struct obj *) 0;

                            if (typ && !P_RESTRICTED(objects[typ].oc_skill))
                                break;
                        } while (ncount++ < 1000);
                    } else if ((primary_casters || primary_casters_priest) && !rn2(3)) {
                        /* Making a spellbook */
                        int trycnt = u.ulevel + 1;

                        /* not yet known spells given preference over already known ones.
                           Also, try to grant a spell for which there is a skill slot */
                        otmp = mkobj(SPBOOK_CLASS, TRUE);

                        if (!otmp)
                            return 1;

                        while (--trycnt > 0) {
                            if (otmp->otyp != SPE_BLANK_PAPER) {
                                if (!known_spell(otmp->otyp)
                                    && !P_RESTRICTED(spell_skilltype(otmp->otyp)))
                                    break; /* usable, but not yet known */
                            } else {
                                if ((!objects[SPE_BLANK_PAPER].oc_name_known
                                     || carrying(MAGIC_MARKER)) && u.uconduct.literate)
                                    break;
                            }
                            otmp->otyp = rnd_class(bases[SPBOOK_CLASS], SPE_FREEZE_SPHERE);
                            set_material(otmp, objects[otmp->otyp].oc_material);
                        }

                        if (!u.uconduct.literate && (otmp->otyp != SPE_BLANK_PAPER)
                            && !known_spell(otmp->otyp)) {
                            if (force_learn_spell(otmp->otyp))
                                pline("Divine knowledge of %s fills your mind!",
                                      OBJ_NAME(objects[otmp->otyp]));
                            obfree(otmp, (struct obj *) 0);
                        } else {
                            bless(otmp);
                            otmp->oeroded = otmp->oeroded2 = 0;
                            at_your_feet("A spellbook");
                            place_object(otmp, u.ux, u.uy);
                            newsym(u.ux, u.uy);
                        }
                        godvoice(u.ualign.type, "Use this gift skillfully!");
                        if (!otmp || is_magic(otmp))
                            u.ugifts++;
                        u.ublesscnt = rnz(300 + (50 * u.ugifts));
                        exercise(A_WIS, TRUE);
                        if (!Hallucination && !Blind) {
                            otmp->dknown = 1;
                            makeknown(otmp->otyp);
                        }
                        livelog_printf(LL_DIVINEGIFT | LL_ARTIFACT,
                                       "had %s given to %s by %s",
                                       an(xname(otmp)), uhim(), u_gname());
                        return 1;
                    } else { /* Making armor */
                        do {
                            /* even chance for each slot
                               giants and tortles are evenly distributed among armor
                               they can wear. monks and centaurs end up more likely
                               to receive certain kinds, but them's the breaks */
                            switch (Race_if(PM_GIANT) ? rn1(4, 2)
                                                      : Race_if(PM_TORTLE) ? rn1(3, 3)
                                                                           : rn2(6)) {
                            case 0:
                                /* body armor (inc. shirts) */
                                if (primary_casters || primary_casters_priest) {
                                    typ = rn2(2) ? rnd_class(ARMOR, JACKET)
                                                 : rn2(6) ? typ == STUDDED_ARMOR
                                                          : typ == CRYSTAL_PLATE_MAIL;
                                } else {
                                    typ = rnd_class(PLATE_MAIL, T_SHIRT);
                                }
                                if (!Role_if(PM_MONK)
                                    || (typ == T_SHIRT || typ == HAWAIIAN_SHIRT)) {
                                    break; /* monks only can have shirts */
                                } /* monks have (almost) double chance for cloaks */
                                /* FALLTHRU */
                            case 1:
                                /* cloak */
                                typ = rnd_class(MUMMY_WRAPPING, CLOAK_OF_DISPLACEMENT);
                                break;
                            case 2:
                                /* boots */
                                if (primary_casters || primary_casters_priest) {
                                    typ = !rn2(3) ? typ == LOW_BOOTS
                                                  : rnd_class(HIGH_BOOTS, LEVITATION_BOOTS);
                                } else {
                                    typ = rnd_class(LOW_BOOTS, LEVITATION_BOOTS);
                                }
                                if (!Race_if(PM_CENTAUR)) {
                                    break;
                                } /* centaurs have double chances to get a shield */
                                /* FALLTHRU */
                            case 3:
                                /* shield */
                                if (primary_casters || primary_casters_priest) {
                                    if (Race_if(PM_DROW)) {
                                        typ = rn2(8) ? typ == DARK_ELVEN_BRACERS
                                                     : rn2(2) ? typ == SHIELD_OF_REFLECTION
                                                              : typ == SHIELD_OF_MOBILITY;
                                    } else {
                                        typ = rn2(8) ? typ == SMALL_SHIELD
                                                     : rnd_class(SHIELD_OF_REFLECTION, SHIELD_OF_MOBILITY);
                                    }
                                } else {
                                    if (Race_if(PM_DROW))
                                        typ = rnd_class(SMALL_SHIELD, SHIELD_OF_REFLECTION);
                                    else
                                        typ = rnd_class(SMALL_SHIELD, SHIELD_OF_MOBILITY);
                                }
                                if (!Role_if(PM_MONK)) {
                                    break;
                                } /* monks have double chances to get gloves */
                                /* FALLTHRU */
                            case 4:
                                /* gloves */
                                if ((primary_casters || primary_casters_priest)) {
                                    typ = rn2(3) ? typ == GLOVES
                                                 : rnd_class(GAUNTLETS_OF_POWER,
                                                             GAUNTLETS_OF_DEXTERITY);
                                } else {
                                    typ = rnd_class(GLOVES, GAUNTLETS_OF_DEXTERITY);
                                }
                                break;
                            case 5:
                                /* helm */
                                if ((primary_casters || primary_casters_priest)) {
                                    if (Role_if(PM_WIZARD)) {
                                        typ = rn2(2) ? rnd_class(CORNUTHAUM, DARK_ELVEN_HELM)
                                                     : rnd_class(HELM_OF_BRILLIANCE,
                                                                 HELM_OF_TELEPATHY);
                                    } else {
                                        typ = rn2(2) ? rnd_class(FEDORA, ELVEN_HELM)
                                                     : rnd_class(HELM_OF_BRILLIANCE,
                                                                 HELM_OF_TELEPATHY);
                                    }
                                } else {
                                    typ = rnd_class(ELVEN_HELM, HELM_OF_TELEPATHY);
                                }
                                break;
                            default:
                                typ = HAWAIIAN_SHIRT; /* Ace Ventura approved. Alrighty then. */
                                break;
                            }

                            /* Same as weapons, but not as badly obviously
                             * apply starting inventory subs - so we'll get
                             * racial gear if possible
                             */
                            if (urace.malenum != PM_HUMAN) {
                                int i;
                                for (i = 0; inv_subs[i].race_pm != NON_PM; ++i) {
                                    if (inv_subs[i].race_pm == urace.malenum
                                        && typ == inv_subs[i].item_otyp) {
                                        typ = inv_subs[i].subs_otyp;
                                        break;
                                    }
                                }
                            }

                            /* if we have the WRONG object, then let's not do that */
                            otmp = mksobj(typ, FALSE, FALSE);
                            if (is_elven_armor(otmp) && !Race_if(PM_ELF))
                                typ = 0;
                            else if (is_drow_armor(otmp) && !Race_if(PM_DROW))
                                typ = 0;
                            else if (is_orcish_armor(otmp) && !Race_if(PM_ORC))
                                typ = 0;
                            else if (is_dwarvish_armor(otmp) && !Race_if(PM_DWARF))
                                typ = 0;
                            else if (otmp->otyp == LARGE_SPLINT_MAIL && !Race_if(PM_GIANT)
                                     && !Role_if(PM_SAMURAI))
                                typ = 0;
                            else if (otmp->otyp == HELM_OF_OPPOSITE_ALIGNMENT)
                                typ = 0;

                            obfree(otmp, (struct obj *) 0);
                            otmp = (struct obj *) 0;

                        } while (ncount++ < 1000 && !typ);
                    }

                    if (typ) {
                        ncount = 0;
                        otmp = mksobj(typ, FALSE, FALSE);
                        while ((((Race_if(PM_ELF) || Race_if(PM_DROW))
                                 && otmp->material == IRON)
                                || (Race_if(PM_ORC)
                                    && otmp->material == MITHRIL)
                                || (Race_if(PM_ILLITHID)
                                    && is_helmet(otmp)
                                    && is_heavy_metallic(otmp))
                                || (Role_if(PM_INFIDEL)
                                    && otmp->material == SILVER))
                               && ncount++ < 500) {
                            obfree(otmp, (struct obj *) 0);
                            otmp = mksobj(typ, FALSE, FALSE);
                        }

                        if (otmp) {
                            if (otmp->otyp == SHURIKEN)
                                otmp->quan = (long) rn1(7, 14); /* 14-20 count */
                            if (!rn2(8))
                                otmp = create_oprop(otmp, FALSE);
                            if (altaralign == A_NONE)
                                curse(otmp);
                            else
                                bless(otmp);
                            otmp->spe = rn2(3) + 3; /* +3 to +5 */
                            maybe_erodeproof(otmp, 1);
                            otmp->oeroded = otmp->oeroded2 = 0;
                            if (altaralign > A_CHAOTIC) /* lawful or neutral altar */
                                otmp->opoisoned = otmp->otainted = 0;
                            otmp->owt = weight(otmp);
                            at_your_feet(otmp->quan > 1L ? "Some objects"
                                                         : "An object");
                            place_object(otmp, u.ux, u.uy);
                            newsym(u.ux, u.uy);
                            if (altaralign == A_NONE)
                                godvoice(u.ualign.type,
                                         "Use this gift ominously!");
                            else
                                godvoice(u.ualign.type,
                                         "Use this gift valorously!");
                            if (is_magic(otmp))
                                u.ugifts++;
                            u.ublesscnt = rnz(300 + (50 * u.ugifts));
                            exercise(A_WIS, TRUE);
                            if (!Hallucination && !Blind) {
                                otmp->dknown = 1;
                                makeknown(otmp->otyp);
                            }
                            livelog_printf(LL_DIVINEGIFT | LL_ARTIFACT,
                                           "had %s entrusted to %s by %s",
                                           doname(otmp), uhim(), u_gname());
                            return 1;
                        }
                    }
                }
            } else if (u.uluck >= 0 && !rn2(arti_gift_odds)) {
                otmp = mk_artifact((struct obj *) 0, a_align(u.ux, u.uy));
                if (otmp) {
                    if (otmp->spe < 0)
                        otmp->spe = 0;
                    if (altaralign == A_NONE)
                        curse(otmp);
                    else
                        bless(otmp);
                    maybe_erodeproof(otmp, 1);
                    otmp->oeroded = otmp->oeroded2 = 0;
                    if (altaralign > A_CHAOTIC) /* lawful or neutral altar */
                        otmp->opoisoned = otmp->otainted = 0;
                    at_your_feet("An object");
                    place_object(otmp, u.ux, u.uy);
                    newsym(u.ux, u.uy);
                    godvoice(u.ualign.type, "Use my gift wisely!");
                    u.ugifts++;
                    u.ublesscnt = rnz(300 + (50 * u.ugifts));
                    exercise(A_WIS, TRUE);

                    /* make sure we can use this weapon */
                    unrestrict_weapon_skill(weapon_type(otmp));
                    if (!Hallucination && !Blind) {
                        otmp->dknown = 1;
                        makeknown(otmp->otyp);
                        discover_artifact(otmp->oartifact);
                    }
                    livelog_printf(LL_DIVINEGIFT | LL_ARTIFACT,
                                   "had %s bestowed upon %s by %s",
                                   otmp->oartifact
                                        ? artiname(otmp->oartifact)
                                        : an(xname(otmp)),
                                   uhim(), align_gname(u.ualign.type));
                    return 1;
                }
            }

            change_luck((value * LUCKMAX) / (MAXVALUE * 2));

            if ((int) u.uluck < 0)
                u.uluck = 0;

            if (u.uluck != saved_luck) {
                if (Blind)
                    You("think %s brushed your %s.", something,
                        body_part(FOOT));
                else
                    You(Hallucination
                        ? "see crabgrass at your %s.  A funny thing in a dungeon."
                        : "glimpse a four-leaf clover at your %s.",
                        makeplural(body_part(FOOT)));
            }
        }
    }
    return 1;
}

/* determine prayer results in advance; also used for enlightenment */
boolean
can_pray(praying)
boolean praying; /* false means no messages should be given */
{
    int alignment;

    p_aligntyp = on_altar() ? a_align(u.ux, u.uy) : u.ualign.type;
    p_trouble = in_trouble();

    if (is_demon(raceptr(&youmonst)) && (p_aligntyp > A_CHAOTIC)) {
        if (praying)
            pline_The("very idea of praying to a %s god is repugnant to you.",
                      p_aligntyp ? "lawful" : "neutral");
        return FALSE;
    }

    if (praying)
        You("begin praying to %s.", align_gname(p_aligntyp));

    if (u.ualign.type && u.ualign.type == -p_aligntyp)
        alignment = -u.ualign.record; /* Opposite alignment altar */
    else if (u.ualign.type != p_aligntyp)
        alignment = u.ualign.record / 2; /* Different alignment altar */
    else
        alignment = u.ualign.record;

    if ((p_trouble > 0) ? (u.ublesscnt > 200)      /* big trouble */
           : (p_trouble < 0) ? (u.ublesscnt > 100) /* minor difficulties */
              : (u.ublesscnt > 0))                 /* not in trouble */
        p_type = 0;                     /* too soon... */
    else if ((int) Luck < 0 || u.ugangr || alignment < 0)
        p_type = 1; /* too naughty... */
    else /* alignment >= 0 */ {
        if (on_altar() && u.ualign.type != p_aligntyp)
            p_type = 2;
        else
            p_type = 3;
    }

    if (is_undead(youmonst.data) && !mitre_inhell()
        && (p_aligntyp == A_LAWFUL || (p_aligntyp == A_NEUTRAL && praying
                                       && !rn2(10))))
        p_type = -1;
    if (p_aligntyp == A_NONE && !on_altar()
        && depth(&u.uz) < depth(&valley_level)
        && !(Role_if(PM_INFIDEL) && u.uhave.questart) && praying && rn2(5))
        p_type = -2; /* Moloch can't hear you */

    return praying || (p_type == 3 && (!mitre_inhell() || u.ualign.type == A_NONE));
}

/* #pray commmand */
int
dopray()
{
    /* Confirm accidental slips of Alt-P */
    if (ParanoidPray && yn("Are you sure you want to pray?") != 'y')
        return 0;

    if (!u.uconduct.gnostic++)
        /* breaking conduct should probably occur in can_pray() at
         * "You begin praying to %s", as demons who find praying repugnant
         * should not break conduct.  Also we can add more detail to the
         * livelog message as p_aligntyp will be known.
         */
        livelog_write_string(LL_CONDUCT, "rejected atheism with a prayer");

    /* set up p_type and p_alignment */
    if (!can_pray(TRUE))
        return 0;

    if (wizard && p_type >= 0) {
        if (yn("Force the gods to be pleased?") == 'y') {
            u.ublesscnt = 0;
            if (u.uluck < 0)
                u.uluck = 0;
            if (u.ualign.record <= 0)
                u.ualign.record = 1;
            u.ugangr = 0;
            if (p_type < 2)
                p_type = 3;
        }
    }
    nomul(-3);
    multi_reason = "praying";
    nomovemsg = "You finish your prayer.";
    afternmv = prayer_done;

    if (p_type == 3 && (!mitre_inhell() || u.ualign.type == A_NONE)) {
        /* if you've been true to your god you can't die while you pray */
        if (!Blind) {
            if (u.ualign.type == A_NONE)
                You("are surrounded by an ominous crimson glow.");
            else if (u.ualign.type != A_NONE && Race_if(PM_DROW))
                You("are surrounded by swirling shadows.");
            else
                You("are surrounded by a shimmering light.");
        }
        u.uinvulnerable = TRUE;
    }

    return 1;
}

STATIC_PTR int
prayer_done() /* M. Stephenson (1.0.3b) */
{
    aligntyp alignment = p_aligntyp;

    u.uinvulnerable = FALSE;
    if (p_type == -1) {
        const char* residual = "residual undead turning effect";
        godvoice(alignment,
                 (alignment == A_LAWFUL)
                    ? "Vile creature, thou durst call upon me?"
                    : "Walk no more, perversion of nature!");
        You_feel("like you are falling apart.");
        /* KMH -- Gods have mastery over unchanging
         * aos -- ...unless you've been sentient_arise()'d or are polyinitted */
        if (HUnchanging) {
            u.mh = 0;
            /* set killer things here because rehumanize will call done() if you
             * have unchanging and will impossible if killer is unset */
            Strcpy(killer.name, residual);
            killer.format = KILLED_BY_AN;
        }
        rehumanize();
        /* no Half_physical_damage adjustment here */
        if (!HUnchanging) {
            losehp(rnd(20), "residual undead turning effect", KILLED_BY_AN);
            exercise(A_CON, FALSE);
        }
        return 1;
    }
    if (p_type == -2) {
        pline("Unfortunately, this close to the surface %s can't hear you.",
              align_gname(alignment));
        /* no further effects */
        return 0;
    }
    if (mitre_inhell() && u.ualign.type != A_NONE) {
        pline("Since you are in Gehennom, %s can't help you.",
              align_gname(alignment));
        /* haltingly aligned is least likely to anger */
        if (u.ualign.record <= 0 || rnl(u.ualign.record))
            angrygods(u.ualign.type);
        return 0;
    }

    if (p_type == 0) {
        if (on_altar() && u.ualign.type != alignment)
            (void) water_prayer(FALSE);
        u.ublesscnt += rnz(250);
        change_luck(-3);
        gods_upset(u.ualign.type);
    } else if (p_type == 1) {
        if (on_altar() && u.ualign.type != alignment)
            (void) water_prayer(FALSE);
        angrygods(u.ualign.type); /* naughty */
    } else if (p_type == 2) {
        if (water_prayer(FALSE)) {
            /* attempted water prayer on a non-coaligned altar */
            u.ublesscnt += rnz(250);
            change_luck(-3);
            gods_upset(u.ualign.type);
        } else
            pleased(alignment);
    } else {
        /* coaligned */
        if (alignment == A_NONE) {
            if (on_altar())
                (void) water_prayer(FALSE);
        } else {
            if (on_altar())
                (void) water_prayer(TRUE);
        }
        pleased(alignment); /* nice */
    }
    return 1;
}

/* #turn command */
int
doturn()
{
    /* Knights & Priest(esse)s only please */
    struct monst *mtmp, *mtmp2;
    const char *Gname;
    int once, range, xlev;

    if (!Role_if(PM_PRIEST) && !Role_if(PM_KNIGHT)) {
        /* Try to use the "turn undead" spell. */
        if (known_spell(SPE_TURN_UNDEAD))
            return spelleffects(spell_idx(SPE_TURN_UNDEAD), FALSE, FALSE);
        You("don't know how to turn undead!");
        return 0;
    }
    if (Hidinshell) {
        You_cant("turn undead while hiding in your shell!");
        return 0;
    }
    if (!u.uconduct.gnostic++)
        livelog_write_string(LL_CONDUCT, "rejected atheism by turning undead");

    u.uconduct.gnostic++;
    Gname = halu_gname(u.ualign.type);

    /* [What about needing free hands (does #turn involve any gesturing)?] */
    if (!can_chant(&youmonst)) {
        /* "evilness": "demons and undead" is too verbose and too precise */
        You("are %s upon %s to turn aside evilness.",
            Strangled ? "not able to call" : "incapable of calling", Gname);
        /* violates agnosticism due to intent; conduct tracking is not
           supposed to affect play but we make an exception here:  use a
           move if this is the first time agnostic conduct has been broken */
        return (u.uconduct.gnostic == 1);
    }
    if ((u.ualign.type != A_CHAOTIC
         && (is_demon(raceptr(&youmonst)) || is_undead(youmonst.data)))
        || u.ugangr > 6) { /* "Die, mortal!" */
        pline("For some reason, %s seems to ignore you.", Gname);
        aggravate();
        exercise(A_WIS, FALSE);
        return 1;
    }
    if (mitre_inhell()) {
        pline("Since you are in Gehennom, %s %s help you.",
              /* not actually calling upon Moloch but use alternate
                 phrasing anyway if hallucinatory feedback says it's him */
              Gname, !strcmp(Gname, Moloch) ? "won't" : "can't");
        aggravate();
        return 1;
    }
    pline("Calling upon %s, you chant an arcane formula.", Gname);
    exercise(A_WIS, TRUE);

    /* note: does not perform unturn_dead() on victims' inventories */
    range = BOLT_LIM + (u.ulevel / 5); /* 8 to 14 */
    range *= range;
    once = 0;
    for (mtmp = fmon; mtmp; mtmp = mtmp2) {
        mtmp2 = mtmp->nmon;
        if (DEADMONSTER(mtmp))
            continue;
        /* 3.6.3: used to use cansee() here but the purpose is to prevent
           #turn operating through walls, not to require that the hero be
           able to see the target location */
        if (!couldsee(mtmp->mx, mtmp->my)
            || distu(mtmp->mx, mtmp->my) > range)
            continue;

        if (!mtmp->mpeaceful
            && (is_undead(mtmp->data) || is_vampshifter(mtmp)
                || (is_demon(mtmp->data) && (u.ulevel > (MAXULEV / 2))))) {
            mtmp->msleeping = 0;
            if (Confusion) {
                if (!once++)
                    pline("Unfortunately, your voice falters.");
                mtmp->mflee = 0;
                mtmp->mfrozen = 0;
		if (!mtmp->mstone || mtmp->mstone > 2)
		    mtmp->mcanmove = 1;
            } else if (!resist(mtmp, '\0', 0, TELL)) {
                xlev = 6;
                switch (mtmp->data->mlet) {
                /* this is intentional, lichs are tougher
                   than zombies. */
                case S_LICH:
                case PM_ALHOON:
                    xlev += 2;
                    /*FALLTHRU*/
                case S_GHOST:
                    xlev += 2;
                    /*FALLTHRU*/
                case S_VAMPIRE:
                    xlev += 2;
                    /*FALLTHRU*/
                case S_WRAITH:
                    xlev += 2;
                    /*FALLTHRU*/
                case S_MUMMY:
                    xlev += 2;
                    /*FALLTHRU*/
                case S_ZOMBIE:
                    if (u.ulevel >= xlev && !resist(mtmp, '\0', 0, NOTELL)) {
                        if (u.ualign.type == A_CHAOTIC) {
                            mtmp->mpeaceful = 1;
                            set_malign(mtmp);
                        } else { /* damn them */
                            killed(mtmp);
                        }
                        break;
                    } /* else flee */
                /*FALLTHRU*/
                default:
                    monflee(mtmp, 0, FALSE, TRUE);
                    break;
                }
            }
        }
    }

    /*
     *  There is no detrimental effect on self for successful #turn
     *  while in demon or undead form.  That can only be done while
     *  chaotic oneself (see "For some reason" above) and chaotic
     *  turning only makes targets peaceful.
     *
     *  Paralysis duration probably ought to be based on the strengh
     *  of turned creatures rather than on turner's level.
     *  Why doesn't this honor Free_action?  [Because being able to
     *  repeat #turn every turn would be too powerful.  Maybe instead
     *  of nomul(-N) we should add the equivalent of mon->mspec_used
     *  for the hero and refuse to #turn when it's non-zero?  Or have
     *  both and u.uspec_used only matters when Free_action prevents
     *  the brief paralysis?]
     */
    nomul(-(5 - ((u.ulevel - 1) / 6))); /* -5 .. -1 */
    multi_reason = "trying to turn the monsters";
    nomovemsg = You_can_move_again;
    return 1;
}

int
altarmask_at(x, y)
int x, y;
{
    int res = 0;

    if (isok(x, y)) {
        struct monst *mon = m_at(x, y);

        if (mon && M_AP_TYPE(mon) == M_AP_FURNITURE
            && mon->mappearance == S_altar)
            res = has_mcorpsenm(mon) ? MCORPSENM(mon) : 0;
        else if (IS_ALTAR(levl[x][y].typ))
            res = levl[x][y].altarmask;
    }
    return res;
}

const char *
a_gname()
{
    return a_gname_at(u.ux, u.uy);
}

/* returns the name of an altar's deity */
const char *
a_gname_at(x, y)
xchar x, y;
{
    if (!IS_ALTAR(levl[x][y].typ))
        return (char *) 0;

    return align_gname(a_align(x, y));
}

/* returns the name of the hero's deity */
const char *
u_gname()
{
    return align_gname(u.ualign.type);
}

const char *
align_gname(alignment)
aligntyp alignment;
{
    const char *gnam;

    switch (alignment) {
    case A_NONE:
        gnam = Moloch;
        break;
    case A_LAWFUL:
        gnam = urole.lgod;
        break;
    case A_NEUTRAL:
        gnam = urole.ngod;
        break;
    case A_CHAOTIC:
        gnam = urole.cgod;
        break;
    default:
        impossible("unknown alignment.");
        gnam = "someone";
        break;
    }
    if (*gnam == '_')
        ++gnam;
    return gnam;
}

static const char *hallu_gods[] = {
    "the Flying Spaghetti Monster", /* Church of the FSM */
    "Eris",                         /* Discordianism */
    "the Martians",                 /* every science fiction ever */
    "Xom",                          /* Crawl */
    "AnDoR dRaKoN",                 /* ADOM */
    "the Central Bank of Yendor",   /* economics */
    "Tooth Fairy",                  /* real world(?) */
    "Om",                           /* Discworld */
    "Yawgmoth",                     /* Magic: the Gathering */
    "Morgoth",                      /* LoTR */
    "Cthulhu",                      /* Lovecraft */
    "the Ori",                      /* Stargate */
    "destiny",                      /* why not? */
    "your Friend the Computer",     /* Paranoia */
};

/* hallucination handling for priest/minion names: select a random god
   iff character is hallucinating */
const char *
halu_gname(alignment)
aligntyp alignment;
{
    const char *gnam = NULL;
    int which;

    if (!Hallucination)
        return align_gname(alignment);

    /* Some roles (Priest) don't have a pantheon unless we're playing as
       that role, so keep trying until we get a role which does have one.
       [If playing a Priest, the current pantheon will be twice as likely
       to get picked as any of the others.  That's not significant enough
       to bother dealing with.] */
    do
        which = randrole(TRUE);
    while (!roles[which].lgod);

    switch (rn2_on_display_rng(9)) {
    case 0:
    case 1:
        gnam = roles[which].lgod;
        break;
    case 2:
    case 3:
        gnam = roles[which].ngod;
        break;
    case 4:
    case 5:
        gnam = roles[which].cgod;
        break;
    case 6:
    case 7:
        gnam = hallu_gods[rn2_on_display_rng(SIZE(hallu_gods))];
        break;
    case 8:
        gnam = Moloch;
        break;
    default:
        impossible("rn2 broken in halu_gname?!?");
    }
    if (!gnam) {
        impossible("No random god name?");
        gnam = "your Friend the Computer"; /* Paranoia */
    }
    if (*gnam == '_')
        ++gnam;
    return gnam;
}

/* deity's title */
const char *
align_gtitle(alignment)
aligntyp alignment;
{
    const char *gnam, *result = "god";

    switch (alignment) {
    case A_NONE:
        gnam = Moloch;
        break;
    case A_LAWFUL:
        gnam = urole.lgod;
        break;
    case A_NEUTRAL:
        gnam = urole.ngod;
        break;
    case A_CHAOTIC:
        gnam = urole.cgod;
        break;
    default:
        gnam = 0;
        break;
    }
    if (gnam && *gnam == '_')
        result = "goddess";
    return result;
}

/* return an alignment from a (lawful, neutral, chaotic) permutation
 * randomly chosen at game start; only relevant to Infidels */
aligntyp
inf_align(num)
int num; /* 1..3 */
{
    aligntyp first, other;
    first = context.inf_aligns % 3 - 1;
    if (num == 1)
        return first;
    other = (context.inf_aligns + num) % 2;
    if (other <= first)
        other--;
    return other;
}

void
altar_wrath(x, y)
register int x, y;
{
    aligntyp altaralign = a_align(x, y);

    if (u.ualign.type == altaralign && u.ualign.record > -rn2(4)) {
        godvoice(altaralign, "How darest thou desecrate my altar!");
        (void) adjattrib(A_WIS, -1, FALSE);
        u.ualign.record--;
    } else {
        pline("%s %s%s:",
              !Deaf ? "A voice (could it be"
                    : "Despite your deafness, you seem to hear",
              align_gname(altaralign),
              !Deaf ? "?) whispers" : " say");
        verbalize("Thou shalt pay, infidel!");
        /* higher luck is more likely to be reduced; as it approaches -5
           the chance to lose another point drops down, eventually to 0 */
        if (Luck > -5 && rn2(Luck + 6))
            change_luck(rn2(20) ? -1 : -2);
    }
}

/* assumes isok() at one space away, but not necessarily at two */
STATIC_OVL boolean
blocked_boulder(dx, dy)
int dx, dy;
{
    register struct obj *otmp;
    int nx, ny;
    long count = 0L;

    for (otmp = level.objects[u.ux + dx][u.uy + dy]; otmp;
         otmp = otmp->nexthere) {
        if (otmp->otyp == BOULDER)
            count += otmp->quan;
    }

    nx = u.ux + 2 * dx, ny = u.uy + 2 * dy; /* next spot beyond boulder(s) */
    switch (count) {
    case 0:
        /* no boulders--not blocked */
        return FALSE;
    case 1:
        /* possibly blocked depending on if it's pushable */
        break;
    case 2:
        /* this is only approximate since multiple boulders might sink */
        if (is_pool_or_lava(nx, ny)) /* does its own isok() check */
            break; /* still need Sokoban check below */
        /*FALLTHRU*/
    default:
        /* more than one boulder--blocked after they push the top one;
           don't force them to push it first to find out */
        return TRUE;
    }

    if (dx && dy && Sokoban) /* can't push boulder diagonally in Sokoban */
        return TRUE;
    if (!isok(nx, ny))
        return TRUE;
    if (IS_ROCK(levl[nx][ny].typ))
        return TRUE;
    if (sobj_at(BOULDER, nx, ny))
        return TRUE;

    return FALSE;
}

/*pray.c*/
