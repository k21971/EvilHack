/* NetHack 3.6  artilist.h      $NHDT-Date: 1564351548 2019/07/28 22:05:48 $  $NHDT-Branch: NetHack-3.6 $:$NHDT-Revision: 1.20 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2017. */
/* NetHack may be freely redistributed.  See license for details. */

#ifdef MAKEDEFS_C
/* in makedefs.c, all we care about is the list of names */

#define A(nam, typ, s1, s2, mt, atk, dfn, cry, inv, al, cl, rac, \
          cost, clr, mat) nam

static const char *artifact_names[] = {
#else
/* in artifact.c, set up the actual artifact list structure */

#define A(nam, typ, s1, s2, mt, atk, dfn, cry, inv, al, cl, rac, \
          cost, clr, mat)                                        \
    {                                                            \
        typ, nam, s1, s2, mt, atk, dfn, cry, inv, al, cl, rac,   \
        cost, clr, mat                                           \
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
#define     DISE(a,b)   {0,AD_DISE,a,b}         /* disease attack */
#define     DREN(a,b)   {0,AD_DREN,a,b}         /* drains energy */
#define     STON(a,b)   {0,AD_STON,a,b}         /* petrification */
#define     DETH(a,b)   {0,AD_DETH,a,b}         /* special death attack */
#define     DISN(a,b)   {0,AD_DISN,a,b}         /* disintegration attack */
#define     FUSE(a,b)   {0,AD_FUSE,a,b}         /* combine fire and cold attack */

#define DEFAULT_MAT 0 /* use base object's default material */
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
      NON_PM, NON_PM, 0L, NO_COLOR, DEFAULT_MAT),

    A("Excalibur", LONG_SWORD, (SPFX_NOGEN | SPFX_RESTR | SPFX_SEEK
                                | SPFX_DEFN | SPFX_INTEL | SPFX_SEARCH),
      0, 0, PHYS(5, 10), DFNS(AD_DRLI), NO_CARY, 0, A_LAWFUL, PM_KNIGHT, NON_PM,
      4000L, NO_COLOR, DEFAULT_MAT),
    /*
     *      Stormbringer only has a 2 because it can drain a level,
     *      providing 8 more.
     */
    A("Stormbringer", RUNESWORD,
      (SPFX_RESTR | SPFX_ATTK | SPFX_DEFN | SPFX_INTEL | SPFX_DRLI), 0, 0,
      DRLI(5, 2), DFNS(AD_DRLI), NO_CARY, 0, A_CHAOTIC, NON_PM, NON_PM, 8000L,
      NO_COLOR, METAL),
    /*
     *      Mjollnir can be thrown when wielded if hero has 25 Strength
     *      (usually via gauntlets of power but possible with rings of
     *      gain strength).  If the thrower is a Valkyrie, Mjollnir will
     *      usually (99%) return and then usually (separate 99%) be caught
     *      and automatically be re-wielded.  When returning Mjollnir is
     *      not caught, there is a 50:50 chance of hitting hero for damage
     *      and its lightning shock might destroy some wands and/or rings.
     *
     *      Monsters don't throw Mjollnir regardless of strength (not even
     *      fake-player valkyries).
     */
    A("Mjollnir", HEAVY_WAR_HAMMER, /* Mjo:llnir */
      (SPFX_RESTR | SPFX_ATTK), 0, 0, ELEC(5, 24), DFNS(AD_ELEC), NO_CARY, 0,
      A_NEUTRAL, PM_VALKYRIE, NON_PM, 5000L, NO_COLOR, DEFAULT_MAT),

    A("Cleaver", BATTLE_AXE, SPFX_RESTR, 0, 0, PHYS(3, 6), NO_DFNS, NO_CARY,
      0, A_NEUTRAL, PM_BARBARIAN, NON_PM, 1500L, NO_COLOR, DEFAULT_MAT),

    /*
     *      Grimtooth glows in warning when elves are present, but its
     *      damage bonus applies to all targets rather than just elves
     *      (handled as special case in spec_dbon()).
     */
    A("Grimtooth", ORCISH_DAGGER,
      (SPFX_RESTR | SPFX_ATTK | SPFX_WARN | SPFX_DFLAGH),
      0, MH_ELF | MH_DROW, DISE(5, 6), NO_DFNS, NO_CARY, 0, A_CHAOTIC,
      NON_PM, PM_ORC, 1500L, CLR_RED, DEFAULT_MAT),
    /*
     *      Orcrist and Sting have same alignment as elves.
     *
     *      The combination of SPFX_WARN+SPFX_DFLAGH+MH_value will trigger
     *      EWarn_of_mon for all monsters that have the MH_value flag.
     *      Sting and Orcrist will warn of MH_ORC monsters.
     */
    A("Orcrist", ELVEN_BROADSWORD, (SPFX_WARN | SPFX_DFLAGH), 0, MH_ORC,
      PHYS(5, 0), NO_DFNS, NO_CARY, 0, A_CHAOTIC, NON_PM, PM_ELF, 2000L,
      CLR_BRIGHT_BLUE, MITHRIL), /* bright blue is actually light blue */

    A("Sting", ELVEN_DAGGER, (SPFX_WARN | SPFX_DFLAGH), 0,
      (MH_ORC | MH_SPIDER), PHYS(5, 0), NO_DFNS, NO_CARY, 0, A_CHAOTIC,
      NON_PM, PM_ELF, 1000L, CLR_BRIGHT_BLUE, MITHRIL),
    /*
     *      Magicbane is a bit different!  Its magic fanfare
     *      unbalances victims in addition to doing some damage.
     */
    A("Magicbane", QUARTERSTAFF, (SPFX_RESTR | SPFX_ATTK | SPFX_DEFN), 0, 0,
      STUN(3, 4), DFNS(AD_MAGM), NO_CARY, 0, A_NEUTRAL, PM_WIZARD, NON_PM,
      3500L, NO_COLOR, DEFAULT_MAT),

    A("Frost Brand", SHORT_SWORD, (SPFX_RESTR | SPFX_ATTK | SPFX_DEFN), 0, 0,
      COLD(5, 0), DFNS(AD_COLD), NO_CARY, 0, A_NONE, NON_PM, NON_PM, 3000L,
      NO_COLOR, METAL),

    A("Fire Brand", SHORT_SWORD, (SPFX_RESTR | SPFX_ATTK | SPFX_DEFN), 0, 0,
      FIRE(5, 0), DFNS(AD_FIRE), NO_CARY, 0, A_NONE, NON_PM, NON_PM, 3000L,
      NO_COLOR, METAL),
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
      NO_COLOR, DRAGON_HIDE),
    /*
     *      Demonbane from SporkHack is a silver mace with an extra property.
     *      Also the first sacrifice gift for a priest.
     */
    A("Demonbane", HEAVY_MACE, (SPFX_RESTR | SPFX_WARN | SPFX_DFLAGH), 0, MH_DEMON,
      PHYS(5, 0), NO_DFNS, NO_CARY, 0, A_LAWFUL, PM_PRIEST, NON_PM, 3000L,
      NO_COLOR, SILVER),

    A("Werebane", SABER, (SPFX_RESTR | SPFX_WARN | SPFX_DFLAGH), 0, MH_WERE,
      PHYS(5, 0), DFNS(AD_WERE), NO_CARY, 0, A_NONE, NON_PM, NON_PM, 1500L,
      NO_COLOR, SILVER),

    A("Grayswandir", SABER, (SPFX_RESTR | SPFX_HALRES), 0, 0,
      PHYS(5, 0), NO_DFNS, NO_CARY, 0, A_LAWFUL, NON_PM, NON_PM, 8000L,
      NO_COLOR, SILVER),

    A("Giantslayer", SPEAR, (SPFX_RESTR | SPFX_WARN | SPFX_DFLAGH), 0, MH_GIANT,
      PHYS(5, 0), NO_DFNS, NO_CARY, 0, A_NEUTRAL, NON_PM, NON_PM, 500L,
      NO_COLOR, DEFAULT_MAT),

    A("Ogresmasher", HEAVY_WAR_HAMMER, (SPFX_RESTR | SPFX_WARN | SPFX_DFLAGH), 0, MH_OGRE,
      PHYS(5, 0), NO_DFNS, NO_CARY, 0, A_NONE, NON_PM, NON_PM, 500L,
      NO_COLOR, DEFAULT_MAT),

    A("Trollsbane", MORNING_STAR, (SPFX_RESTR | SPFX_WARN | SPFX_DFLAGH | SPFX_REGEN), 0, MH_TROLL,
      PHYS(5, 0), NO_DFNS, NO_CARY, 0, A_NONE, NON_PM, NON_PM, 800L,
      NO_COLOR, DEFAULT_MAT),
    /*
     *      Two problems:  1) doesn't let trolls regenerate heads,
     *      2) doesn't give unusual message for 2-headed monsters (but
     *      allowing those at all causes more problems than worth the effort).
     */
    A("Vorpal Blade", LONG_SWORD, (SPFX_RESTR | SPFX_BEHEAD | SPFX_WARN | SPFX_DFLAGH),
      0, MH_JABBERWOCK, PHYS(5, 6), NO_DFNS, NO_CARY, 0, A_NEUTRAL, NON_PM, NON_PM, 5000L,
      NO_COLOR, METAL),
    /*
     *      Ah, never shall I forget the cry,
     *              or the shriek that shrieked he,
     *      As I gnashed my teeth, and from my sheath
     *              I drew my Snickersnee!
     *                      --Koko, Lord high executioner of Titipu
     *                        (From Sir W.S. Gilbert's "The Mikado")
     */
    A("Snickersnee", KATANA, (SPFX_RESTR | SPFX_DEFN), 0, 0,
      PHYS(5, 8), DFNS(AD_STUN), NO_CARY, 0, A_LAWFUL, PM_SAMURAI,
      NON_PM, 1800L, NO_COLOR, DEFAULT_MAT),
    /*
     *      Sunsword from SporkHack warned of nearby undead
     */
    A("Sunsword", LONG_SWORD, (SPFX_RESTR | SPFX_WARN | SPFX_DFLAGH),
      0, MH_UNDEAD, PHYS(5, 0), DFNS(AD_BLND), NO_CARY,
      0, A_LAWFUL, NON_PM, NON_PM, 2500L, NO_COLOR, GEMSTONE),
    /*
     *      Lifestealer from SporkHack - many of the same properties as Stormbringer
     *      Meant to be wielded by Vlad. Enjoy the buff Vlad ;)
     */
    A("Lifestealer", TWO_HANDED_SWORD,
      (SPFX_NOGEN | SPFX_NOWISH | SPFX_RESTR | SPFX_ATTK | SPFX_DEFN
       | SPFX_INTEL | SPFX_DRLI),
      0, 0, DRLI(5, 2), DFNS(AD_DRLI), NO_CARY, 0, A_CHAOTIC, NON_PM,
      NON_PM, 10000L, NO_COLOR, DEFAULT_MAT),
    /*
     *      Keolewa from SporkHack - a Hawaiian war club.
     *      Buffing this up a bit to give it more utility.
     */
    A("Keolewa", CLUB, (SPFX_RESTR | SPFX_ATTK | SPFX_DEFN),
      0, 0, ELEC(5, 8), DFNS(AD_ELEC), NO_CARY, 0, A_NEUTRAL,
      PM_CAVEMAN, NON_PM, 2000L, NO_COLOR, DEFAULT_MAT),
    /*
     *      Dirge from SporkHack, but with a twist.
     *      This is the anti-Excalibur. A Dark Knight needs a special weapon too...
     */
    A("Dirge", LONG_SWORD,
     (SPFX_NOGEN | SPFX_RESTR | SPFX_ATTK | SPFX_DEFN | SPFX_INTEL), 0, 0,
     ACID(5, 10), DFNS(AD_ACID), NO_CARY, 0, A_CHAOTIC, NON_PM, NON_PM,
     4000L, NO_COLOR, MITHRIL),
    /*
     * The Sword of Kas - the sword forged by Vecna and given to his top
     * lieutenant, Kas. This sword's specs have changed throughout ad&d
     * editions, so we'll take some creative license here while trying to
     * stay true to some of its abilities from ad&d.
     */
    A("The Sword of Kas", TWO_HANDED_SWORD,
      (SPFX_NOGEN | SPFX_NOWISH | SPFX_RESTR | SPFX_ATTK | SPFX_DEFN
       | SPFX_INTEL | SPFX_DALIGN), 0, 0,
      DRST(10, 0), DFNS(AD_STON), NO_CARY, 0, A_CHAOTIC,
      NON_PM, NON_PM, 15000L, NO_COLOR, GEMSTONE),
    /* Thought the Oracle just knew everything on her own? Guess again. Should
     * anyone ever be foolhardy enough to take on the Oracle and succeed,
     * they might discover the true source of her knowledge.
     */
    A("Magic 8-Ball", EIGHT_BALL,
      (SPFX_NOGEN | SPFX_NOWISH | SPFX_RESTR | SPFX_INTEL | SPFX_SPEAK),
      SPFX_WARN, 0, NO_ATTK, NO_DFNS, NO_CARY, 0, A_NONE,
      NON_PM, NON_PM, 5000L, NO_COLOR, DEFAULT_MAT),
    /* Convict role first artifact weapon should they altar sacrifice for one.
     * Acts like a luckstone.
     */
    A("Luck Blade", BROADSWORD, (SPFX_RESTR | SPFX_LUCK), 0, 0,
      PHYS(5, 6), NO_DFNS, NO_CARY, 0, A_CHAOTIC, PM_CONVICT, NON_PM, 3000L,
      NO_COLOR, METAL),
    /* The energy drain only works if the artifact kills its victim.
     * Also increases sacrifice value while wielded. */
    A("Secespita", KNIFE, (SPFX_RESTR | SPFX_ATTK), 0, 0,
      DREN(8, 8), DFNS(AD_DRST), NO_CARY, 0, A_CHAOTIC, PM_INFIDEL, NON_PM,
      3000L, NO_COLOR, COPPER),
    /* Bag of the Hesperides - this is the magicbal bag obtained by Perseus
     * from the Hesperides (nymphs) to contain and transport Medusa's head.
     * The bag naturally repels water, and it has greater weight reduction
     * than a regular bag of holding. Found at the end of the Ice Queen branch
     * with the captive pegasus.
     */
    A("Bag of the Hesperides", BAG_OF_HOLDING,
      (SPFX_NOGEN | SPFX_NOWISH | SPFX_RESTR), SPFX_PROTECT, 0,
      NO_ATTK, NO_DFNS, NO_CARY, 0, A_NONE, NON_PM, NON_PM,
      8000L, NO_COLOR, DRAGON_HIDE),
    /* The quasi-evil twin of Demonbane, Angelslayer is an unholy trident
     * geared towards the destruction of all angelic beings */
    A("Angelslayer", TRIDENT,
      (SPFX_RESTR | SPFX_ATTK | SPFX_SEARCH | SPFX_HSPDAM
       | SPFX_WARN | SPFX_DFLAGH), 0,
      MH_ANGEL, FIRE(5, 10), NO_DFNS, NO_CARY, 0, A_NONE,
      NON_PM, NON_PM, 5000L, NO_COLOR, DEFAULT_MAT),
    /* Yeenoghu's infamous triple-headed flail, also known as 'Butcher'.
     * A massive weapon reputed to have been created from the thighbone and
     * torn flesh of an ancient god he slew. An extremely lethal artifact */
    A("Butcher", TRIPLE_HEADED_FLAIL,
      (SPFX_NOGEN | SPFX_NOWISH | SPFX_RESTR | SPFX_ATTK | SPFX_INTEL),
      0, 0, STUN(5, 8), NO_DFNS, NO_CARY, 0, A_CHAOTIC,
      NON_PM, NON_PM, 4000L, NO_COLOR, BONE),
    /* Orcus' true 'Wand of Death', a truly terrifying weapon that can kill
     * those it strikes with one blow. In the form of an ornate mace/rod, the Wand
     * of Orcus is 'a rod of obsidian topped by a skull. This instrument causes
     * death (or annihilation) to any creature, save those of like status
     * merely by touching their flesh'. Can only be wielded by Orcus or others
     * of his ilk */
    A("Wand of Orcus", ROD,
      (SPFX_NOGEN | SPFX_NOWISH | SPFX_RESTR | SPFX_ATTK | SPFX_INTEL),
      0, 0, DETH(5, 6), NO_DFNS, NO_CARY, COMMAND_UNDEAD, A_CHAOTIC,
      NON_PM, NON_PM, 75000L, NO_COLOR, GEMSTONE),
    /* The Eye of Vecna, which Vecna will sometimes death drop
       before the rest of his body crumbles to dust */
    A("The Eye of Vecna", EYEBALL,
      (SPFX_NOGEN | SPFX_NOWISH | SPFX_RESTR | SPFX_INTEL),
      (SPFX_XRAY | SPFX_ESP | SPFX_HSPDAM), 0,
      NO_ATTK, NO_DFNS, CARY(AD_COLD), DEATH_MAGIC, A_NONE,
      NON_PM, NON_PM, 50000L, NO_COLOR, DEFAULT_MAT),
    /* The Hand of Vecna, another possible artifact that Vecna
       might drop once destroyed */
    A("The Hand of Vecna", MUMMIFIED_HAND,
      (SPFX_NOGEN | SPFX_NOWISH | SPFX_RESTR | SPFX_INTEL | SPFX_REGEN
       | SPFX_HPHDAM),
      0, 0, NO_ATTK, DFNS(AD_DISE), NO_CARY, DEATH_MAGIC, A_NONE,
      NON_PM, NON_PM, 50000L, NO_COLOR, FLESH),
    /* Dramborleg, one of the most powerful weapons ever forged from
       Lord of The Rings series. Per lore, it's unknown exactly which
       race created this axe, but it was wielded by Tuor, who used it
       to slay many powerful balrogs with ease. I like to think the
       dwarves forged it, powerful axes are their thing */
    A("Dramborleg", DWARVISH_BEARDED_AXE,
      (SPFX_RESTR | SPFX_INTEL | SPFX_PROTECT | SPFX_WARN | SPFX_DFLAGH),
      0, MH_DEMON, PHYS(8, 8), DFNS(AD_MAGM), NO_CARY, 0, A_LAWFUL,
      NON_PM, PM_DWARF, 9000L, CLR_RED, MITHRIL),
    /* The One Ring, from J.R.R Tolkien lore */
    A("The One Ring", RIN_LUSTROUS,
      (SPFX_NOGEN | SPFX_NOWISH | SPFX_RESTR | SPFX_INTEL | SPFX_STLTH
       | SPFX_SEARCH | SPFX_WARN | SPFX_DFLAGH),
      0, MH_WRAITH, NO_ATTK, NO_DFNS, NO_CARY, 0, A_NONE, NON_PM,
      NON_PM, 7500L, NO_COLOR, GOLD),

    /*
     *      Forged artifacts
     *
     *      Artifacts that can only be created in a forge
     *      by forging two existing artifacts together to
     *      create a new artifact. Due to the powerful magic
     *      involved in forging two artifacts together, the
     *      material of the final product can sometimes be
     *      completely different from the recipe object
     *      materials.
     */

    /* The Sword of Annihilation can only be created by forging the
       artifacts Angelslayer and either of the Vecna artifacts
       (eye or hand) together. Their combined magic and energy form
       to produce a sword capable of disintegrating most anything it
       hits, while protecting the one that wields it from the same type
       of attack */
    A("The Sword of Annihilation", LONG_SWORD,
      (SPFX_NOGEN | SPFX_FORGED | SPFX_NOWISH | SPFX_RESTR | SPFX_ATTK
       | SPFX_DEFN | SPFX_INTEL),
      0, 0, DISN(5, 12), DFNS(AD_DISN), NO_CARY, 0, A_NONE,
      NON_PM, NON_PM, 25000L, NO_COLOR, METAL),
    /* Glamdring, from the LotR series by J.R.R Tolkien. This was the
     * sword that was found along side Orcrist and Sting in a troll cave,
     * and was used by Gandalf thereafter. Like its brethren, this sword
     * glows blue in the presence of orcs. In EvilHack, can only come into
     * existience by forging Orcrist and Sting together */
    A("Glamdring", ELVEN_LONG_SWORD,
      (SPFX_NOGEN | SPFX_FORGED | SPFX_NOWISH | SPFX_RESTR | SPFX_WARN
       | SPFX_PROTECT | SPFX_DFLAGH),
      0, MH_ORC, PHYS(8, 0), DFNS(AD_ELEC), NO_CARY, 0, A_CHAOTIC,
      NON_PM, PM_ELF, 8000L, CLR_BRIGHT_BLUE, MITHRIL),
    /* The Staff of the Archmagi, allows the one that wields it a 50%
       casting success bonus across all spell schools (see spell.c).
       It also gives off light when wielded, and does most of the same
       type of damage Magicbane would do, minus cancellation. Can be
       created by forging Magicbane and Secespita together */
    A("The Staff of the Archmagi", ASHWOOD_STAFF,
      (SPFX_NOGEN | SPFX_FORGED | SPFX_NOWISH | SPFX_RESTR | SPFX_ATTK
       | SPFX_DEFN | SPFX_INTEL),
      SPFX_HSPDAM, 0, STUN(5, 8), DFNS(AD_MAGM), NO_CARY, PHASING, A_NEUTRAL,
      NON_PM, NON_PM, 35000L, NO_COLOR, DEFAULT_MAT),
    /* Shadowblade is a chaotic aligned athame that is created by
       forging Stormbringer and Werebane together. Inherits
       Stormbringer's drain life attack and protection without its
       penchant for attacking peaceful creatures. Can be invoked to
       cause fear or create an aura of darkness. Is made of adamantine,
       and is attuned to Drow */
    A("Shadowblade", ATHAME,
      (SPFX_NOGEN | SPFX_FORGED | SPFX_NOWISH | SPFX_RESTR | SPFX_ATTK
       | SPFX_DEFN | SPFX_INTEL | SPFX_DRLI | SPFX_SEARCH | SPFX_STLTH
       | SPFX_WARN | SPFX_DFLAGH), 0, MH_WERE,
      DRLI(8, 10), DFNS(AD_DRLI), NO_CARY, SHADOWBLADE, A_CHAOTIC,
      NON_PM, PM_DROW, 15000L, NO_COLOR, ADAMANTINE),
    /* The Gauntlets of Purity are a divine artifact that is created
       by forging Dragonbane and Grayswandir together. These gauntlets
       inherit reflection from Dragonbane, and the silver material and
       alignment of Grayswandir. While worn, they protect the wearer
       from death magic and boost power regeneration. If the wearers
       alignment matches that of the gauntlets, the wearer will be able
       to resist the charms of monsters that try to seduce them. Does
       not inhibit spellcasting while worn */
    A("Gauntlets of Purity", GAUNTLETS_OF_POWER,
      (SPFX_NOGEN | SPFX_FORGED | SPFX_NOWISH | SPFX_RESTR | SPFX_DEFN
       | SPFX_INTEL | SPFX_REFLECT | SPFX_EREGEN),
      0, 0, NO_ATTK, DFNS(AD_DETH), NO_CARY, 0, A_LAWFUL,
      NON_PM, NON_PM, 25000L, NO_COLOR, SILVER),
    /* Ashmar is a mithril dwarvish roundshield that can be created
       by forging Trollsbane and Ogresmasher together. The word 'Ashmar'
       is the neo-Khuzdul word for 'guardian', and that is what this
       shield excels at. When worn, it imparts hungerless regen (from
       Trollsbane), bumps up con to 25 (from Ogresmasher), acid
       resistance, MC1, and half physical damage. Ashmar is attuned to
       dwarvenkind, but can be used by any race, and is non-aligned.
       Ashmar also protects the wearer from being knocked back, and if
       the shield deflects an attack, there's a 1 in 4 chance of it
       knocking the attacker back */
    A("Ashmar", DWARVISH_ROUNDSHIELD,
      (SPFX_NOGEN | SPFX_FORGED | SPFX_NOWISH | SPFX_RESTR | SPFX_DEFN
       | SPFX_INTEL | SPFX_REGEN | SPFX_PROTECT | SPFX_HPHDAM),
      0, 0, NO_ATTK, DFNS(AD_ACID), NO_CARY, 0, A_NONE,
      NON_PM, PM_DWARF, 20000L, NO_COLOR, MITHRIL),
    /* The Hammer of the Gods is a silver heavy war hammer that can be
       created by forging Sunsword and Demonbane together. In a nutshell,
       has all of the same abilities and powers as Sunsword and Demonbane,
       along with imparting disease resistance when wielded */
    A("Hammer of the Gods", HEAVY_WAR_HAMMER,
      (SPFX_NOGEN | SPFX_FORGED | SPFX_NOWISH | SPFX_RESTR | SPFX_WARN
       | SPFX_INTEL | SPFX_DFLAGH), 0, (MH_DEMON | MH_UNDEAD),
      PHYS(8, 0), DFNS(AD_DISE), NO_CARY, 0, A_LAWFUL,
      NON_PM, NON_PM, 25000L, NO_COLOR, SILVER),
    /* Tempest is a halberd that can be created by forging Cleaver and
       Mjollnir together. It inherits lightning damage/protection from
       Mjollnir, and in some cases will cause area of effect lightning
       damage on a successful hit. Prevents becoming stunned when wielded,
       can be invoked for conflict */
    A("Tempest", HALBERD,
      (SPFX_NOGEN | SPFX_FORGED | SPFX_NOWISH | SPFX_RESTR | SPFX_INTEL
       | SPFX_ATTK | SPFX_DEFN), 0, 0,
      ELEC(10, 24), DFNS(AD_ELEC), NO_CARY, CONFLICT, A_NONE,
      NON_PM, NON_PM, 20000L, NO_COLOR, IRON),
    /* Ithilmar is a set of barding that can be created by forging
       Vorpal Blade and the Gauntlets of Purity together. It grants
       the steed that wears it reflection, magic resistance, and
       several other benefits that will help keep the steed alive */
    A("Ithilmar", RUNED_BARDING,
      (SPFX_NOGEN | SPFX_FORGED | SPFX_NOWISH | SPFX_RESTR | SPFX_INTEL
       | SPFX_DEFN | SPFX_REFLECT), 0, 0,
      NO_ATTK, DFNS(AD_MAGM), NO_CARY, 0, A_NONE,
      NON_PM, NON_PM, 50000L, NO_COLOR, METAL),
    /* The Armor of Retribution is crystal plate mail that is created
       by forging two forged artifacts - the Sword of Annihilation and
       Ashmar. It inherits many of the abilities of both, while having
       a few abilities that didn't exist from either, such as half spell
       damage, and a certain level of protection vs cancellation.
       Cannot be dragon-scaled */
    A("Armor of Retribution", CRYSTAL_PLATE_MAIL,
      (SPFX_NOGEN | SPFX_FORGED | SPFX_NOWISH | SPFX_RESTR | SPFX_DEFN
       | SPFX_INTEL | SPFX_PROTECT | SPFX_HPHDAM | SPFX_HSPDAM),
      0, 0, NO_ATTK, DFNS(AD_DISN), CARY(AD_ACID), 0, A_NONE,
      NON_PM, NON_PM, 60000L, NO_COLOR, DEFAULT_MAT),
    /* Dichotomy is a crystal runed broadsword that is created by forging
       Fire Brand and Frost Brand together. It inhereits the damage types
       and protection from both (fire/cold) */
    A("Dichotomy", RUNESWORD,
      (SPFX_NOGEN | SPFX_FORGED | SPFX_NOWISH | SPFX_RESTR | SPFX_ATTK
       | SPFX_DEFN), 0, 0,
      FUSE(8, 0), DFNS(AD_FUSE), NO_CARY, 0, A_NONE, NON_PM, NON_PM, 15000L,
      NO_COLOR, GEMSTONE),

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
      NON_PM, 3500L, NO_COLOR, DEFAULT_MAT),

#if 0 /* Replaced by Xiuhcoatl */
    A("The Orb of Detection", CRYSTAL_BALL,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_INTEL), (SPFX_ESP | SPFX_HSPDAM), 0,
      NO_ATTK, NO_DFNS, CARY(AD_MAGM), INVIS, A_LAWFUL, PM_ARCHEOLOGIST,
      NON_PM, 2500L, NO_COLOR, DEFAULT_MAT),
#endif

#if 0 /* Replaced by The Ring of P'hul */
    A("The Heart of Ahriman", LUCKSTONE,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_INTEL), SPFX_STLTH, 0,
      /* this stone does double damage if used as a projectile weapon */
      PHYS(5, 0), NO_DFNS, NO_CARY, LEVITATION, A_NEUTRAL, PM_BARBARIAN,
      NON_PM, 2500L, NO_COLOR, DEFAULT_MAT),
#endif

    A("The Sceptre of Might", ROD,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_INTEL | SPFX_DALIGN), 0, 0, PHYS(5, 0),
      DFNS(AD_MAGM), NO_CARY, CONFLICT, A_LAWFUL, PM_CAVEMAN, NON_PM, 2500L,
      NO_COLOR, METAL),

#if 0 /* OBSOLETE */
    A("The Palantir of Westernesse", CRYSTAL_BALL,
      (SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL),
      (SPFX_ESP|SPFX_REGEN|SPFX_HSPDAM), 0,
      NO_ATTK, NO_DFNS, NO_CARY,
      TAMING, A_CHAOTIC, NON_PM , PM_ELF, 8000L, NO_COLOR, DEFAULT_MAT),
#endif

    A("Bracers of the First Circle", RUNED_BRACERS,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_INTEL | SPFX_DEFN | SPFX_ESP), 0, 0,
      NO_ATTK, DFNS(AD_STON), NO_CARY, TAMING, A_NEUTRAL,
      PM_DRUID, NON_PM, 5000L, NO_COLOR, DEFAULT_MAT),

    A("The Staff of Aesculapius", QUARTERSTAFF,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_ATTK | SPFX_INTEL | SPFX_DRLI
       | SPFX_REGEN),
      0, 0, DRLI(5, 0), DFNS(AD_DRLI), NO_CARY, HEALING, A_NEUTRAL, PM_HEALER,
      NON_PM, 5000L, NO_COLOR, DEFAULT_MAT),

    A("The Magic Mirror of Merlin", MIRROR,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_INTEL | SPFX_SPEAK),
      (SPFX_REFLECT | SPFX_ESP | SPFX_HSPDAM), 0,
      NO_ATTK, NO_DFNS, NO_CARY, 0, A_LAWFUL, PM_KNIGHT, NON_PM, 1500L,
      NO_COLOR, DEFAULT_MAT),

    A("The Eyes of the Overworld", LENSES,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_INTEL | SPFX_XRAY), 0, 0, NO_ATTK,
      DFNS(AD_MAGM), NO_CARY, ENLIGHTENING, A_NEUTRAL, PM_MONK, NON_PM,
      2500L, NO_COLOR, DEFAULT_MAT),

    A("The Mitre of Holiness", HELM_OF_BRILLIANCE,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_DFLAGH | SPFX_INTEL | SPFX_PROTECT),
      0, MH_UNDEAD, NO_ATTK, NO_DFNS, CARY(AD_FIRE),
      ENERGY_BOOST, A_LAWFUL, PM_PRIEST, NON_PM, 2000L, NO_COLOR, METAL),

    /* If playing a gnomish ranger, the player receives the 'Crossbow of Carl',
       otherwise rangers will receive the Longbow of Diana. Exact same properties
       between the two artifacts */
    A("The Longbow of Diana", BOW,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_INTEL | SPFX_REFLECT), SPFX_ESP, 0,
      PHYS(5, 0), NO_DFNS, NO_CARY, CREATE_AMMO, A_CHAOTIC, PM_RANGER, NON_PM,
      4000L, NO_COLOR, DEFAULT_MAT),

    A("The Crossbow of Carl", CROSSBOW,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_INTEL | SPFX_REFLECT), SPFX_ESP, 0,
      PHYS(5, 0), NO_DFNS, NO_CARY, CREATE_AMMO, A_CHAOTIC, PM_RANGER, NON_PM,
      4000L, NO_COLOR, DEFAULT_MAT),

    /* MKoT has an additional carry property if the Key is not cursed (for
       rogues) or blessed (for non-rogues):  #untrap of doors and chests
       will always find any traps and disarming those will always succeed */
    A("The Master Key of Thievery", SKELETON_KEY,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_INTEL | SPFX_SPEAK),
      (SPFX_WARN | SPFX_TCTRL | SPFX_HPHDAM), 0, NO_ATTK, NO_DFNS, NO_CARY,
      UNTRAP, A_CHAOTIC, PM_ROGUE, NON_PM, 3500L, NO_COLOR, DEFAULT_MAT),

    A("The Tsurugi of Muramasa", TSURUGI,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_INTEL | SPFX_BEHEAD | SPFX_LUCK
       | SPFX_PROTECT | SPFX_HPHDAM), 0, 0,
      PHYS(3, 8), NO_DFNS, NO_CARY, 0, A_LAWFUL, PM_SAMURAI, NON_PM,
      6000L, NO_COLOR, DEFAULT_MAT),

    A("The Platinum Yendorian Express Card", CREDIT_CARD,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_INTEL | SPFX_DEFN),
      (SPFX_ESP | SPFX_HSPDAM), 0, NO_ATTK, NO_DFNS, CARY(AD_MAGM),
      CHARGE_OBJ, A_NEUTRAL, PM_TOURIST, NON_PM, 7000L, NO_COLOR, PLATINUM),

#if 0 /* Replaced by Gjallar */
    A("The Orb of Fate", CRYSTAL_BALL,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_INTEL | SPFX_LUCK),
      (SPFX_WARN | SPFX_HPHDAM), 0, NO_ATTK, NO_DFNS, NO_CARY,
      LEV_TELE, A_NEUTRAL, PM_VALKYRIE, NON_PM, 3500L, NO_COLOR, DEFAULT_MAT),
#endif

    A("Gjallar", TOOLED_HORN,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_INTEL | SPFX_LUCK),
      (SPFX_WARN | SPFX_HPHDAM), 0, NO_ATTK, NO_DFNS, NO_CARY,
      LEV_TELE, A_NEUTRAL, PM_VALKYRIE, NON_PM, 5000L, NO_COLOR, BONE),

    A("The Eye of the Aethiopica", AMULET_OF_ESP,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_INTEL), (SPFX_EREGEN | SPFX_HSPDAM), 0,
      NO_ATTK, DFNS(AD_MAGM), NO_CARY, CREATE_PORTAL, A_NEUTRAL, PM_WIZARD,
      NON_PM, 4000L, NO_COLOR, DEFAULT_MAT),
    /*
     *      Based loosely off of the Ring of P'hul - from 'The Lords of Dus' series
     *      by Lawrence Watt-Evans. This is another one of those artifacts that would
     *      just be ridiculous if its full power were realized in-game. In the series,
     *      it deals out death and disease. Here it will protect the wearer from a
     *      good portion of that. Making this the quest artifact for the Barbarian role.
     *      This artifact also corrects an oversight from vanilla, that no chaotic-based
     *      artiafcts conferred magic resistance, a problem that was compounded if our
     *      hero is in a form that can't wear body armor or cloaks. So, we make the
     *      Barbarian artifact chaotic (why it was neutral before is a bit confusing
     *      to me as most vanilla Barbarian race/role combinations are chaotic).
     */
    A("The Ring of P\'hul", RIN_ANCIENT,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_INTEL | SPFX_DEFN), 0, 0,
      NO_ATTK, DFNS(AD_MAGM), CARY(AD_DISE), 0, A_CHAOTIC, PM_BARBARIAN,
      NON_PM, 5000L, NO_COLOR, GEMSTONE),

    /* Convict role quest artifact. Provides magic resistance when worn,
     * invoke to phase through walls like a xorn.
     */
    A("The Striped Shirt of Liberation", STRIPED_SHIRT,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_INTEL | SPFX_STLTH
       | SPFX_SEARCH | SPFX_WARN), 0, 0,
      NO_ATTK, DFNS(AD_MAGM), NO_CARY, PHASING,
      A_CHAOTIC, PM_CONVICT, NON_PM, 10000L, NO_COLOR, DEFAULT_MAT),

    /* Infidel role quest artifact. Confers energy regeneration,
     * but only to those in good standing with Moloch. */
    A("The Idol of Moloch", FIGURINE,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_INTEL), SPFX_HSPDAM, 0,
      NO_ATTK, NO_DFNS, CARY(AD_MAGM), CHANNEL, A_CHAOTIC, PM_INFIDEL, NON_PM,
      4000L, NO_COLOR, DEFAULT_MAT),

    /*
     *  terminator; otyp must be zero
     */
    A(0, 0, 0, 0, 0, NO_ATTK, NO_DFNS, NO_CARY, 0, A_NONE, NON_PM, NON_PM, 0L,
      0, 0) /* 0 is CLR_BLACK rather than NO_COLOR but it doesn't matter here */

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
#undef DREN
#undef STON
#undef DETH
#undef DISN
#undef FUSE
#endif

/*artilist.h*/
