/* NetHack 3.6  artilist.h      $NHDT-Date: 1433050874 2015/05/31 05:41:14 $  $NHDT-Branch: master $:$NHDT-Revision: 1.16 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2017. */
/* NetHack may be freely redistributed.  See license for details. */

#ifdef MAKEDEFS_C
/* in makedefs.c, all we care about is the list of names */

#define A(nam, typ, s1, s2, mt, atk, dfn, cry, inv, al, cl, rac, cost, clr) nam

static const char *artifact_names[] = {
#else
/* in artifact.c, set up the actual artifact list structure */

#define A(nam, typ, s1, s2, mt, atk, dfn, cry, inv, al, cl, rac, cost, clr) \
    {                                                                       \
        typ, nam, s1, s2, mt, atk, dfn, cry, inv, al, cl, rac, cost, clr    \
    }

/* clang-format off */
#define     NO_ATTK     {0,0,0,0}               /* no attack */
#define     NO_DFNS     {0,0,0,0}               /* no defense */
#define     NO_CARY     {0,0,0,0}               /* no carry effects */
#define     DFNS(c)     {0,c,0,0}
#define     CARY(c)     {0,c,0,0}
#define     PHYS(a,b)   {0,AD_PHYS,a,b}         /* physical */
#define     DRLI(a,b)   {0,AD_DRLI,a,b}         /* life drain */
#define     COLD(a,b)   {0,AD_COLD,a,b}
#define     FIRE(a,b)   {0,AD_FIRE,a,b}
#define     ELEC(a,b)   {0,AD_ELEC,a,b}         /* electrical shock */
#define     STUN(a,b)   {0,AD_STUN,a,b}         /* magical attack */
#define     DRST(a,b)   {0,AD_DRST,a,b}         /* poison attack */
#define     ACID(a,b)   {0,AD_ACID,a,b}         /* acid attack */
/* clang-format on */

STATIC_OVL NEARDATA struct artifact artilist[] = {
#endif /* MAKEDEFS_C */

    /* Artifact cost rationale:
     * 1.  The more useful the artifact, the better its cost.
     * 2.  Quest artifacts are highly valued.
     * 3.  Chaotic artifacts are inflated due to scarcity (and balance).
     */

    /*  dummy element #0, so that all interesting indices are non-zero */
    A("", STRANGE_OBJECT, 0, 0, 0, NO_ATTK, NO_DFNS, NO_CARY, 0, A_NONE,
      NON_PM, NON_PM, 0L, NO_COLOR),

    A("Excalibur", LONG_SWORD, (SPFX_NOGEN | SPFX_RESTR | SPFX_SEEK
                                | SPFX_DEFN | SPFX_INTEL | SPFX_SEARCH),
      0, 0, PHYS(5, 10), DFNS(AD_DRLI), NO_CARY, 0, A_LAWFUL, PM_KNIGHT, NON_PM,
      4000L, NO_COLOR),
    /*
     *      Stormbringer only has a 2 because it can drain a level,
     *      providing 8 more.
     */
    A("Stormbringer", RUNESWORD,
      (SPFX_RESTR | SPFX_ATTK | SPFX_DEFN | SPFX_INTEL | SPFX_DRLI), 0, 0,
      DRLI(5, 2), DFNS(AD_DRLI), NO_CARY, 0, A_CHAOTIC, NON_PM, NON_PM, 8000L,
      NO_COLOR),
    /*
     *      Mjollnir will return to the hand of the wielder when thrown
     *      if the wielder is a Valkyrie wearing Gauntlets of Power.
     */
    A("Mjollnir", HEAVY_WAR_HAMMER, /* Mjo:llnir */
      (SPFX_RESTR | SPFX_ATTK), 0, 0, ELEC(5, 24), DFNS(AD_ELEC), NO_CARY, 0,
      A_NEUTRAL, PM_VALKYRIE, NON_PM, 5000L, NO_COLOR),

    A("Cleaver", BATTLE_AXE, SPFX_RESTR, 0, 0, PHYS(3, 6), NO_DFNS, NO_CARY,
      0, A_NEUTRAL, PM_BARBARIAN, NON_PM, 1500L, NO_COLOR),

    /*
     *      Grimtooth glows in warning when elves are present, but its
     *      damage bonus applies to all targets rather than just elves
     *      (handled as special case in spec_dbon()).
     */
    A("Grimtooth", ORCISH_DAGGER, (SPFX_RESTR | SPFX_WARN | SPFX_DFLAGH),
      0, MH_ELF, PHYS(5, 6), NO_DFNS,
      NO_CARY, 0, A_CHAOTIC, NON_PM, PM_ORC, 600L, CLR_RED),
    /*
     *      Orcrist and Sting have same alignment as elves.
     *
     *      The combination of SPFX_WARN+SPFX_DFLAGH+MH_value will trigger
     *      EWarn_of_mon for all monsters that have the MH_value flag.
     *      Sting and Orcrist will warn of MH_ORC monsters.
     */
    A("Orcrist", ELVEN_BROADSWORD, (SPFX_WARN | SPFX_DFLAGH), 0, MH_ORC,
      PHYS(5, 4), NO_DFNS, NO_CARY, 0, A_CHAOTIC, NON_PM, PM_ELF, 2000L,
      CLR_BRIGHT_BLUE), /* bright blue is actually light blue */

    A("Sting", ELVEN_DAGGER, (SPFX_WARN | SPFX_DFLAGH), 0, MH_ORC, PHYS(5, 3),
      NO_DFNS, NO_CARY, 0, A_CHAOTIC, NON_PM, PM_ELF, 800L, CLR_BRIGHT_BLUE),
    /*
     *      Magicbane is a bit different!  Its magic fanfare
     *      unbalances victims in addition to doing some damage.
     */
    A("Magicbane", ATHAME, (SPFX_RESTR | SPFX_ATTK | SPFX_DEFN), 0, 0,
      STUN(3, 4), DFNS(AD_MAGM), NO_CARY, 0, A_NEUTRAL, PM_WIZARD, NON_PM,
      3500L, NO_COLOR),

    A("Frost Brand", SHORT_SWORD, (SPFX_RESTR | SPFX_ATTK | SPFX_DEFN), 0, 0,
      COLD(5, 8), DFNS(AD_COLD), NO_CARY, 0, A_NONE, NON_PM, NON_PM, 3000L,
      NO_COLOR),

    A("Fire Brand", SHORT_SWORD, (SPFX_RESTR | SPFX_ATTK | SPFX_DEFN), 0, 0,
      FIRE(5, 8), DFNS(AD_FIRE), NO_CARY, 0, A_NONE, NON_PM, NON_PM, 3000L,
      NO_COLOR),
    /*      Let's flip the script a bit - Dragonbane is no longer a weapon,
     *      but a pair of magical gloves made from the scales of a long dead
     *      ancient dragon. The gloves afford much of the same special abilities
     *      as the weapon did, but swaps fire resistance for acid resistance.
     *      Other dragons will be able to sense the power of these gloves and
     *      will be affected accordingly.
     */
    A("Dragonbane", GLOVES,
      (SPFX_RESTR | SPFX_WARN | SPFX_DFLAGH | SPFX_REFLECT), 0, MH_DRAGON,
      NO_ATTK, DFNS(AD_ACID), NO_CARY, 0, A_NONE, NON_PM, NON_PM, 3000L,
      NO_COLOR),
    /*
     *      Demonbane from SporkHack is a silver mace with an extra property.
     *      Also the first sacrifice gift for a priest.
     */
    A("Demonbane", MACE, (SPFX_RESTR | SPFX_WARN | SPFX_DFLAGH), 0, MH_DEMON,
      PHYS(5, 4), NO_DFNS, NO_CARY, 0, A_LAWFUL, PM_PRIEST, NON_PM, 3000L,
      NO_COLOR),

    A("Werebane", SABER, (SPFX_RESTR | SPFX_WARN | SPFX_DFLAGH), 0, MH_WERE,
      PHYS(5, 4), DFNS(AD_WERE), NO_CARY, 0, A_NONE, NON_PM, NON_PM, 1500L,
      NO_COLOR),

    A("Grayswandir", SABER, (SPFX_RESTR | SPFX_HALRES), 0, 0,
      PHYS(5, 8), NO_DFNS, NO_CARY, 0, A_LAWFUL, NON_PM, NON_PM, 8000L,
      NO_COLOR),

    A("Giantslayer", LONG_SWORD, (SPFX_RESTR | SPFX_WARN | SPFX_DFLAGH), 0, MH_GIANT,
      PHYS(5, 5), NO_DFNS, NO_CARY, 0, A_NEUTRAL, NON_PM, NON_PM, 500L,
      NO_COLOR),

    A("Ogresmasher", HEAVY_WAR_HAMMER, (SPFX_RESTR | SPFX_DCLAS), 0, S_OGRE,
      PHYS(5, 5), NO_DFNS, NO_CARY, 0, A_NONE, NON_PM, NON_PM, 500L,
      NO_COLOR),

    A("Trollsbane", MORNING_STAR, (SPFX_RESTR | SPFX_DCLAS | SPFX_REGEN), 0, S_TROLL,
      PHYS(5, 5), NO_DFNS, NO_CARY, 0, A_NONE, NON_PM, NON_PM, 800L,
      NO_COLOR),
    /*
     *      Two problems:  1) doesn't let trolls regenerate heads,
     *      2) doesn't give unusual message for 2-headed monsters (but
     *      allowing those at all causes more problems than worth the effort).
     */
    A("Vorpal Blade", LONG_SWORD, (SPFX_RESTR | SPFX_BEHEAD), 0, 0,
      PHYS(5, 6), NO_DFNS, NO_CARY, 0, A_NEUTRAL, NON_PM, NON_PM, 4000L,
      NO_COLOR),
    /*
     *      Ah, never shall I forget the cry,
     *              or the shriek that shrieked he,
     *      As I gnashed my teeth, and from my sheath
     *              I drew my Snickersnee!
     *                      --Koko, Lord high executioner of Titipu
     *                        (From Sir W.S. Gilbert's "The Mikado")
     */
    A("Snickersnee", KATANA, SPFX_RESTR, 0, 0, PHYS(5, 8), NO_DFNS, NO_CARY,
      0, A_LAWFUL, PM_SAMURAI, NON_PM, 1200L, NO_COLOR),
    /*
     *      Sunsword from SporkHack was silver in nature, and also warned of nearby undead
     */
    A("Sunsword", LONG_SWORD, (SPFX_RESTR | SPFX_WARN | SPFX_DFLAGH), 0, MH_UNDEAD,
      PHYS(5, 4), DFNS(AD_BLND), NO_CARY, 0, A_LAWFUL, NON_PM, NON_PM, 2500L,
      NO_COLOR),
    /*
     *      Lifestealer from SporkHack - many of the same properties as Stormbringer
     *      Meant to be wielded by Vlad. Enjoy the buff Vlad ;)
     */
    A("Lifestealer", TWO_HANDED_SWORD,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_ATTK | SPFX_DEFN | SPFX_INTEL | SPFX_DRLI), 0, 0,
      DRLI(5, 2), DFNS(AD_DRLI), NO_CARY, 0, A_CHAOTIC, NON_PM, NON_PM, 10000L,
      NO_COLOR),
    /*
     *      Keolewa from SporkHack - a Hawaiian war club.
     *      Buffing this up a bit to give it more utility.
     */
    A("Keolewa", CLUB, (SPFX_RESTR | SPFX_DEFN), 0, 0,
      PHYS(5, 6), DFNS(AD_ELEC), NO_CARY, 0, A_NEUTRAL, PM_CAVEMAN, NON_PM,
      2000L, NO_COLOR),
    /*
     *      Dirge from SporkHack, but with a twist.
     *      This is the anti-Excalibur. A Dark Knight needs a special weapon too...
     */
    A("Dirge", LONG_SWORD,
     (SPFX_NOGEN | SPFX_RESTR | SPFX_ATTK | SPFX_DEFN | SPFX_INTEL), 0, 0,
     ACID(5, 8), DFNS(AD_ACID), NO_CARY, 0, A_CHAOTIC, NON_PM, NON_PM,
     4000L, NO_COLOR),
    /*
     *      The Sword of Bheleu - from 'The Lords of Dus' series by Lawrence Watt-Evans.
     *      If I were to add all of the powers and abilities this sword possesses from the book,
     *      it would be way over-powered. It will be a worthy two-handed sword to try to obtain however...
     */
    A("The Sword of Bheleu", TWO_HANDED_SWORD,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_ATTK | SPFX_INTEL | SPFX_DALIGN), 0, 0,
      DRST(10, 10), DFNS(AD_STON), NO_CARY, 0, A_CHAOTIC, NON_PM, NON_PM,
      12000L, NO_COLOR),
    /* Thought the Oracle just knew everything on her own? Guess again. Should
     *  anyone ever be foolhardy enough to take on the Oracle and succeed,
     *  they might discover the true source of her knowledge.
     */
    A("Magic 8-Ball", EIGHT_BALL,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_INTEL | SPFX_SPEAK), SPFX_WARN, 0,
      NO_ATTK, NO_DFNS, NO_CARY, 0, A_NONE, NON_PM, NON_PM, 5000L,
      NO_COLOR),

    /*
     *      The artifacts for the quest dungeon, all self-willed.
     */

    /*
     *      At first I was going to add the artifact shield that was made for the
     *      Archeologist quest in UnNetHack, but then decided to do something unique.
     *      Behold Xiuhcoatl, a dark wooden spear-thrower that does fire damage much like
     *      Mjollnir does electrical damage. From Aztec lore, Xiuhcoatl is an atlatl,
     *      which is an ancient device that was used to throw spears with great force
     *      and even greater distances. Xiuhcoatl will return to the throwers hand much like
     *      Mjollnir, but requires high dexterity instead of strength to handle properly.
     */
    A("Xiuhcoatl", ATLATL,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_INTEL | SPFX_ATTK | SPFX_DEFN), SPFX_ESP, 0,
      FIRE(5, 24), DFNS(AD_FIRE), NO_CARY, LEVITATION, A_LAWFUL, PM_ARCHEOLOGIST,
      NON_PM, 3500L, NO_COLOR),

#if 0 /* Replaced by Xiuhcoatl */
    A("The Orb of Detection", CRYSTAL_BALL,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_INTEL), (SPFX_ESP | SPFX_HSPDAM), 0,
      NO_ATTK, NO_DFNS, CARY(AD_MAGM), INVIS, A_LAWFUL, PM_ARCHEOLOGIST,
      NON_PM, 2500L, NO_COLOR),
#endif

#if 0 /* Replaced by The Ring of P'hul */
    A("The Heart of Ahriman", LUCKSTONE,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_INTEL), SPFX_STLTH, 0,
      /* this stone does double damage if used as a projectile weapon */
      PHYS(5, 0), NO_DFNS, NO_CARY, LEVITATION, A_NEUTRAL, PM_BARBARIAN,
      NON_PM, 2500L, NO_COLOR),
#endif

    A("The Sceptre of Might", MACE,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_INTEL | SPFX_DALIGN), 0, 0, PHYS(5, 2),
      DFNS(AD_MAGM), NO_CARY, CONFLICT, A_LAWFUL, PM_CAVEMAN, NON_PM, 2500L,
      NO_COLOR),

#if 0 /* OBSOLETE */
    A("The Palantir of Westernesse", CRYSTAL_BALL,
      (SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL),
      (SPFX_ESP|SPFX_REGEN|SPFX_HSPDAM), 0,
      NO_ATTK, NO_DFNS, NO_CARY,
      TAMING, A_CHAOTIC, NON_PM , PM_ELF, 8000L, NO_COLOR ),
#endif

    A("The Staff of Aesculapius", QUARTERSTAFF,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_ATTK | SPFX_INTEL | SPFX_DRLI
       | SPFX_REGEN),
      0, 0, DRLI(5, 2), DFNS(AD_DRLI), NO_CARY, HEALING, A_NEUTRAL, PM_HEALER,
      NON_PM, 5000L, NO_COLOR),

    A("The Magic Mirror of Merlin", MIRROR,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_INTEL | SPFX_SPEAK | SPFX_REFLECT),
      (SPFX_REFLECT | SPFX_ESP | SPFX_HSPDAM), 0,
      NO_ATTK, NO_DFNS, NO_CARY, 0, A_LAWFUL, PM_KNIGHT, NON_PM, 1500L,
      NO_COLOR),

    A("The Eyes of the Overworld", LENSES,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_INTEL | SPFX_XRAY), 0, 0, NO_ATTK,
      DFNS(AD_MAGM), NO_CARY, ENLIGHTENING, A_NEUTRAL, PM_MONK, NON_PM,
      2500L, NO_COLOR),

    A("The Mitre of Holiness", HELM_OF_BRILLIANCE,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_DFLAGH | SPFX_INTEL | SPFX_PROTECT), 0,
      MH_UNDEAD, NO_ATTK, NO_DFNS, CARY(AD_FIRE), ENERGY_BOOST, A_LAWFUL,
      PM_PRIEST, NON_PM, 2000L, NO_COLOR),

    A("The Longbow of Diana", BOW,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_INTEL | SPFX_REFLECT), SPFX_ESP, 0,
      PHYS(5, 0), NO_DFNS, NO_CARY, CREATE_AMMO, A_CHAOTIC, PM_RANGER, NON_PM,
      4000L, NO_COLOR),

    /* MKoT has an additional carry property if the Key is not cursed (for
       rogues) or blessed (for non-rogues):  #untrap of doors and chests
       will always find any traps and disarming those will always succeed */
    A("The Master Key of Thievery", SKELETON_KEY,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_INTEL | SPFX_SPEAK),
      (SPFX_WARN | SPFX_TCTRL | SPFX_HPHDAM), 0, NO_ATTK, NO_DFNS, NO_CARY,
      UNTRAP, A_CHAOTIC, PM_ROGUE, NON_PM, 3500L, NO_COLOR),

    A("The Tsurugi of Muramasa", TSURUGI,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_INTEL | SPFX_BEHEAD | SPFX_LUCK
       | SPFX_PROTECT),
      0, 0, PHYS(3, 8), NO_DFNS, NO_CARY, 0, A_LAWFUL, PM_SAMURAI, NON_PM,
      4500L, NO_COLOR),

    A("The Platinum Yendorian Express Card", CREDIT_CARD,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_INTEL | SPFX_DEFN),
      (SPFX_ESP | SPFX_HSPDAM), 0, NO_ATTK, NO_DFNS, CARY(AD_MAGM),
      CHARGE_OBJ, A_NEUTRAL, PM_TOURIST, NON_PM, 7000L, NO_COLOR),

    A("The Orb of Fate", CRYSTAL_BALL,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_INTEL | SPFX_LUCK),
      (SPFX_WARN | SPFX_HPHDAM), 0, NO_ATTK, NO_DFNS, NO_CARY,
      LEV_TELE, A_NEUTRAL, PM_VALKYRIE, NON_PM, 3500L, NO_COLOR),

    A("The Eye of the Aethiopica", AMULET_OF_ESP,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_INTEL), (SPFX_EREGEN | SPFX_HSPDAM), 0,
      NO_ATTK, DFNS(AD_MAGM), NO_CARY, CREATE_PORTAL, A_NEUTRAL, PM_WIZARD,
      NON_PM, 4000L, NO_COLOR),
    /*
     *      Based loosely off of the Ring of P'hul - from 'The Lords of Dus' series
     *      by Lawrence Watt-Evans. This is another one of those artifacts that would
     *      just be ridiculous if its full power were realized in-game. In the series,
     *      it deals out death and disease. Here it will protect the wearer from a
     *      good portion of that. Making this the quest artifact for the Barbarian role,
     *      keeping at least one of the special abilities from The Heart of Ahriman.
     *      This artifact also corrects an oversight from vanilla, that no chaotic-based
     *      artiafcts conferred magic resistance, a problem that was compounded if our
     *      hero is in a form that can't wear body armor or cloaks. So, we make the
     *      Barbarian artifact chaotic (why it was neutral before is a bit confusing
     *      to me as most vanilla Barbarian race/role combinations are chaotic).
     */
    A("The Ring of P\'hul", RIN_STEALTH,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_INTEL | SPFX_DEFN), 0, 0,
      NO_ATTK, DFNS(AD_MAGM), CARY(AD_DISE), 0, A_CHAOTIC, PM_BARBARIAN,
      NON_PM, 5000L, NO_COLOR),

    /*
     *  terminator; otyp must be zero
     */
    A(0, 0, 0, 0, 0, NO_ATTK, NO_DFNS, NO_CARY, 0, A_NONE, NON_PM, NON_PM, 0L,
      0) /* 0 is CLR_BLACK rather than NO_COLOR but it doesn't matter here */

}; /* artilist[] (or artifact_names[]) */

#undef A

#ifndef MAKEDEFS_C
#undef NO_ATTK
#undef NO_DFNS
#undef DFNS
#undef PHYS
#undef DRLI
#undef COLD
#undef FIRE
#undef ELEC
#undef STUN
#undef DRST
#undef ACID
#undef DISE
#endif

/*artilist.h*/
