/* NetHack 3.6	mondata.h	$NHDT-Date: 1576626512 2019/12/17 23:48:32 $  $NHDT-Branch: NetHack-3.6 $:$NHDT-Revision: 1.39 $ */
/* Copyright (c) 1989 Mike Threepoint				  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef MONDATA_H
#define MONDATA_H

#define r_data(mon) (&mons[has_erac(mon) ? ERAC(mon)->rmnum : (mon)->mnum])

#define r_verysmall(mon) (verysmall(r_data(mon)))
#define verysmall(ptr) ((ptr)->msize < MZ_SMALL)
#define r_bigmonst(mon) (bigmonst(r_data(mon)))
#define bigmonst(ptr) ((ptr)->msize >= MZ_LARGE)
#define r_biggermonst(mon) (biggermonst(r_data(mon)))
#define biggermonst(ptr) ((ptr)->msize > (youmonst.data)->msize)
#define vs_cantflyorswim(ptr) \
    (verysmall(ptr) && !is_flyer(ptr) && !is_swimmer(ptr) && !amphibious(ptr))

#define pm_resistance(ptr, typ) (((ptr)->mresists & (typ)) != 0)

/* mresists from any source - innate, intrinsic, or extrinsic */
#define mon_resistancebits(mon) \
    ((mon)->data->mresists | (mon)->mextrinsics | (mon)->mintrinsics)
#define resists_fire(mon) ((mon_resistancebits(mon) & MR_FIRE) != 0)
#define resists_cold(mon) ((mon_resistancebits(mon) & MR_COLD) != 0)
#define resists_sleep(mon) ((mon_resistancebits(mon) & MR_SLEEP) != 0)
#define resists_disint(mon) ((mon_resistancebits(mon) & MR_DISINT) != 0)
#define resists_elec(mon) ((mon_resistancebits(mon) & MR_ELEC) != 0)
#define resists_poison(mon) ((mon_resistancebits(mon) & MR_POISON) != 0)
#define resists_acid(mon) ((mon_resistancebits(mon) & MR_ACID) != 0)
#define resists_ston(mon) ((mon_resistancebits(mon) & MR_STONE) != 0)
#define resists_psychic(mon) ((mon_resistancebits(mon) & MR_PSYCHIC) != 0)

#define has_telepathy(mon) \
    (telepathic(r_data(mon)) \
     || (mon_resistancebits(mon) & MR2_TELEPATHY) != 0)
#define can_wwalk(mon) \
    ((mon_resistancebits(mon) & MR2_WATERWALK) != 0)
#define can_jump(mon) \
    ((mon_resistancebits(mon) & MR2_JUMPING) != 0)
#define has_displacement(mon) \
    ((mon_resistancebits(mon) & MR2_DISPLACED) != 0)
#define has_reflection(mon) \
    ((mon_resistancebits(mon) & MR2_REFLECTION) != 0)
#define can_levitate(mon) \
    ((mon_resistancebits(mon) & MR2_LEVITATE) != 0)
#define has_free_action(mon) \
    ((mon_resistancebits(mon) & MR2_FREE_ACTION) != 0)

#define resists_sick(ptr) \
    ((ptr)->mlet == S_FUNGUS || nonliving(ptr)                                   \
     || is_angel(ptr) || is_demon(ptr) || is_rider(ptr)                          \
     || (ptr) == &mons[PM_BABY_GOLD_DRAGON] || (ptr) == &mons[PM_GOLD_DRAGON]    \
     || (ptr) == &mons[PM_GIANT_LEECH] || (ptr) == &mons[PM_GIANT_COCKROACH]     \
     || (ptr) == &mons[PM_LOCUST] || (ptr) == &mons[PM_KATHRYN_THE_ICE_QUEEN]    \
     || (ptr) == &mons[PM_KATHRYN_THE_ENCHANTRESS] || (ptr) == &mons[PM_CONVICT] \
     || (ptr) == &mons[PM_AIR_ELEMENTAL] || (ptr) == &mons[PM_EARTH_ELEMENTAL]   \
     || (ptr) == &mons[PM_FIRE_ELEMENTAL] || (ptr) == &mons[PM_WATER_ELEMENTAL])

/* as of 3.2.0:  gray dragons, Angels, Oracle, Yeenoghu */
#define resists_mgc(ptr) \
    (dmgtype((ptr), AD_MAGM) || (ptr) == &mons[PM_BABY_GRAY_DRAGON] \
     || (ptr) == &mons[PM_ARCHON] || (ptr) == &mons[PM_ARCHANGEL]   \
     || dmgtype((ptr), AD_RBRE)) /* Tiamat */

#define resists_drain(ptr) \
    (is_undead(ptr) || is_demon(ptr) || is_were(ptr)            \
     || (ptr) == &mons[PM_DEATH] || (ptr) == &mons[PM_CERBERUS] \
     || (ptr) == &mons[PM_BABY_SHADOW_DRAGON]                   \
     || (ptr) == &mons[PM_SHADOW_DRAGON]                        \
     || (ptr) == &mons[PM_OZZY]                                 \
     || (ptr) == &mons[PM_BOURBON]                              \
     || (ptr) == &mons[PM_KATHRYN_THE_ICE_QUEEN]                \
     || (ptr) == &mons[PM_KATHRYN_THE_ENCHANTRESS])
/* is_were() doesn't handle hero in human form */

/* is_vampshifter(mon) in handled explicitly in zap.c */
#define immune_death_magic(ptr) \
    (dmgtype((ptr), AD_DETH)                                                 \
     || nonliving(ptr) || is_demon(ptr) || is_angel(ptr)                     \
     || (ptr)->msound == MS_LEADER || (ptr) == &mons[PM_CERBERUS]            \
     || (ptr) == &mons[PM_DEATH] || (ptr) == &mons[PM_BABY_CELESTIAL_DRAGON] \
     || (ptr) == &mons[PM_CELESTIAL_DRAGON])

#define immune_poisongas(ptr) ((ptr) == &mons[PM_HEZROU])

#define is_lminion(mon) \
    (is_minion((mon)->data) && mon_aligntyp(mon) == A_LAWFUL)
#define is_jumper(ptr) \
    ((ptr) == &mons[PM_KNIGHT] || (ptr) == &mons[PM_JUMPING_SPIDER]             \
     || (ptr) == &mons[PM_GIANT_CENTIPEDE] || (ptr) == &mons[PM_ZRUTY]          \
     || (ptr) == &mons[PM_CAVE_LIZARD] || (ptr) == &mons[PM_LARGE_CAVE_LIZARD])
#define is_flyer(ptr) (((ptr)->mflags1 & M1_FLY) != 0L)
/* humanoid shape with big wings (flight blocked by most body armor) */
#define big_wings(ptr) \
    ((ptr) == &mons[PM_WINGED_GARGOYLE] || (ptr) == &mons[PM_DEMON]  \
     || (ptr) == &mons[PM_SUCCUBUS] || (ptr) == &mons[PM_INCUBUS]    \
     || (ptr) == &mons[PM_HORNED_DEVIL] || (ptr) == &mons[PM_ERINYS] \
     || (ptr) == &mons[PM_VROCK] || (ptr) == &mons[PM_PIT_FIEND]     \
     || (ptr) == &mons[PM_BALROG] || (ptr) == &mons[PM_ANGEL]        \
     || (ptr) == &mons[PM_ARCHANGEL] || (ptr) == &mons[PM_ARCHON]    \
     || (ptr) == &mons[PM_LUCIFER])
#define is_floater(ptr) \
    ((ptr)->mlet == S_EYE || (ptr)->mlet == S_LIGHT)
/* clinger: piercers, mimics, wumpus -- generally don't fall down holes */
#define is_clinger(ptr) (((ptr)->mflags1 & M1_CLING) != 0L)
#define grounded(ptr) (!is_flyer(ptr) && !is_floater(ptr) && !is_clinger(ptr))
#define is_swimmer(ptr) (((ptr)->mflags1 & M1_SWIM) != 0L)
#define breathless(ptr) (((ptr)->mflags1 & M1_BREATHLESS) != 0L)
#define amphibious(ptr) \
    (((ptr)->mflags1 & (M1_AMPHIBIOUS | M1_BREATHLESS)) != 0L)
/* monster that is in or underwater */
#define mon_underwater(mon) \
    (is_swimmer((mon)->data) && is_pool((mon)->mx, (mon)->my))
#define passes_walls(ptr) (((ptr)->mflags1 & M1_WALLWALK) != 0L)
#define amorphous(ptr) (((ptr)->mflags1 & M1_AMORPHOUS) != 0L)
#define noncorporeal(ptr) ((ptr)->mlet == S_GHOST)
#define tunnels(ptr) (((ptr)->mflags1 & M1_TUNNEL) != 0L)
#define racial_tunnels(mon) \
    ((has_erac(mon) && (ERAC(mon)->mflags1 & M1_TUNNEL)) \
     || tunnels((mon)->data))
#define needspick(ptr) (((ptr)->mflags1 & M1_NEEDPICK) != 0L)
#define racial_needspick(mon) \
    ((has_erac(mon) && (ERAC(mon)->mflags1 & M1_NEEDPICK)) \
     || needspick((mon)->data))
/* hides_under() requires an object at the location in order to hide */
#define hides_under(ptr) (((ptr)->mflags1 & M1_CONCEAL) != 0L)
/* is_hider() is True for mimics but when hiding they appear as something
   else rather than become mon->mundetected, so use is_hider() with care */
#define is_hider(ptr) (((ptr)->mflags1 & M1_HIDE) != 0L)
/* piercers cling to the ceiling; lurkers above are hiders but they fly
   so aren't classified as clingers; unfortunately mimics are classified
   as both hiders and clingers but have nothing to do with ceilings;
   wumpuses (not wumpi :-) cling but aren't hiders */
#define ceiling_hider(ptr) \
    (is_hider(ptr) && ((is_clinger(ptr) && (ptr)->mlet != S_MIMIC) \
                       || is_flyer(ptr))) /* lurker above */
#define haseyes(ptr) (((ptr)->mflags1 & M1_NOEYES) == 0L)
/* used to decide whether plural applies so no need for 'more than 2' */
#define eyecount(ptr) \
    (!haseyes(ptr) ? 0                                                     \
     : ((ptr) == &mons[PM_CYCLOPS] || (ptr) == &mons[PM_FLOATING_EYE]) ? 1 \
       : 2)
#define nohands(ptr) (((ptr)->mflags1 & M1_NOHANDS) != 0L)
#define nolimbs(ptr) (((ptr)->mflags1 & M1_NOLIMBS) == M1_NOLIMBS)
#define notake(ptr) (((ptr)->mflags1 & M1_NOTAKE) != 0L)
#define has_head(ptr) (((ptr)->mflags1 & M1_NOHEAD) == 0L)
#define has_horns(ptr) (num_horns(ptr) > 0)
#define is_whirly(ptr) \
    ((ptr)->mlet == S_VORTEX || (ptr) == &mons[PM_AIR_ELEMENTAL])
#define flaming(ptr)                                                     \
    ((ptr) == &mons[PM_FIRE_VORTEX] || (ptr) == &mons[PM_FLAMING_SPHERE] \
     || (ptr) == &mons[PM_FIRE_ELEMENTAL] || (ptr) == &mons[PM_SALAMANDER])
#define is_silent(ptr) ((ptr)->msound == MS_SILENT)
#define unsolid(ptr) (((ptr)->mflags1 & M1_UNSOLID) != 0L)
#define mindless(ptr) (((ptr)->mflags1 & M1_MINDLESS) != 0L)
#define humanoid(ptr) (((ptr)->mflags1 & M1_HUMANOID) != 0L)
#define is_animal(ptr) (((ptr)->mflags1 & M1_ANIMAL) != 0L)
#define is_swallower(ptr) \
    ((is_dragon(ptr) && (ptr) != &mons[PM_SEA_DRAGON]) \
     || is_animal(ptr))
#define slithy(ptr) (((ptr)->mflags1 & M1_SLITHY) != 0L)
#define is_wooden(ptr) ((ptr) == &mons[PM_WOOD_GOLEM])
#define thick_skinned(ptr) (((ptr)->mflags1 & M1_THICK_HIDE) != 0L)
#define hug_throttles(ptr) ((ptr) == &mons[PM_ROPE_GOLEM])
#define slimeproof(ptr) \
    ((ptr) == &mons[PM_GREEN_SLIME] || flaming(ptr) || noncorporeal(ptr))
#define lays_eggs(ptr) (((ptr)->mflags1 & M1_OVIPAROUS) != 0L)
#define eggs_in_water(ptr) \
    (lays_eggs(ptr) && (ptr)->mlet == S_EEL && is_swimmer(ptr))
#define regenerates(ptr) (((ptr)->mflags1 & M1_REGEN) != 0L)
#define perceives(ptr) (((ptr)->mflags1 & M1_SEE_INVIS) != 0L)
#define racial_perceives(mon) \
    ((has_erac(mon) && (ERAC(mon)->mflags1 & M1_SEE_INVIS)) \
     || perceives(mon->data))
#define can_teleport(ptr) (((ptr)->mflags1 & M1_TPORT) != 0L)
#define control_teleport(ptr) (((ptr)->mflags1 & M1_TPORT_CNTRL) != 0L)
#define telepathic(ptr) \
    ((ptr) == &mons[PM_FLOATING_EYE] || (ptr) == &mons[PM_MIND_FLAYER]            \
     || (ptr) == &mons[PM_MASTER_MIND_FLAYER] || (ptr) == &mons[PM_GOBLIN_SHAMAN] \
     || (ptr) == &mons[PM_KOBOLD_SHAMAN] || (ptr) == &mons[PM_ORC_SHAMAN]         \
     || (ptr) == &mons[PM_HILL_GIANT_SHAMAN] || (ptr) == &mons[PM_ELVEN_WIZARD]   \
     || (ptr) == &mons[PM_GNOMISH_WIZARD] || (ptr) == &mons[PM_ALHOON]            \
     || (ptr) == &mons[PM_ILLITHID] || (ptr) == &mons[PM_GNOLL_CLERIC]            \
     || (ptr) == &mons[PM_NEOTHELID] || (ptr) == &mons[PM_TORTLE_SHAMAN]          \
     || (ptr) == &mons[PM_DROW_MAGE] || (ptr) == &mons[PM_DROW_CLERIC])
#define has_claws(ptr) \
    ((is_illithid(ptr)                                         \
      && !((ptr) == &mons[PM_MIND_FLAYER_LARVA]                \
           || (ptr) == &mons[PM_NEOTHELID])) || is_gnoll(ptr)  \
     || (ptr)->mlet == S_COCKATRICE || (ptr)->mlet == S_FELINE \
     || (ptr)->mlet == S_GREMLIN || (ptr)->mlet == S_IMP       \
     || (ptr)->mlet == S_MIMIC || (ptr)->mlet == S_SPIDER      \
     || (ptr)->mlet == S_ZRUTY || (ptr)->mlet == S_BAT         \
     || (ptr)->mlet == S_DRAGON || (ptr)->mlet == S_JABBERWOCK \
     || (ptr)->mlet == S_RUSTMONST || (ptr)->mlet == S_TROLL   \
     || (ptr)->mlet == S_UMBER || (ptr)->mlet == S_YETI        \
     || (ptr)->mlet == S_DEMON || (ptr)->mlet == S_LIZARD      \
     || (ptr)->mlet == S_DOG)
#define has_claws_undead(ptr) \
    ((ptr)->mlet == S_MUMMY || (ptr)->mlet == S_ZOMBIE          \
     || (ptr)->mlet == S_WRAITH || (ptr)->mlet == S_VAMPIRE)
#define is_armed(ptr) attacktype(ptr, AT_WEAP)
#define can_sting(ptr) attacktype(ptr, AT_STNG)
#define acidic(ptr) (((ptr)->mflags1 & M1_ACID) != 0L)
#define poisonous(ptr) (((ptr)->mflags1 & M1_POIS) != 0L)
#define carnivorous(ptr) (((ptr)->mflags1 & M1_CARNIVORE) != 0L)
#define herbivorous(ptr) (((ptr)->mflags1 & M1_HERBIVORE) != 0L)
#define metallivorous(ptr) (((ptr)->mflags1 & M1_METALLIVORE) != 0L)
#define inediate(ptr) (((ptr)->mflags1 & (M1_CARNIVORE | M1_HERBIVORE \
                                          | M1_METALLIVORE)) == 0L)
#define polyok(ptr) (((ptr)->mflags2 & M2_NOPOLY) == 0L)
#define is_shapeshifter(ptr) (((ptr)->mflags2 & M2_SHAPESHIFTER) != 0L)
#define is_undead(ptr) (((ptr)->mhflags & MH_UNDEAD) != 0L)
#define is_were(ptr) (((ptr)->mhflags & MH_WERE) != 0L)
#define mon_has_race(mon, rflag) \
    ((has_erac(mon) ? (ERAC(mon)->mrace & (rflag)) \
                    : ((mon)->data->mhflags & (rflag))) \
     || ((mon) == &youmonst && !Upolyd && (urace.selfmask & (rflag))))
#define is_elf(ptr) (((ptr)->mhflags & MH_ELF) != 0L)
#define racial_elf(mon) mon_has_race(mon, MH_ELF)
#define is_dwarf(ptr) (((ptr)->mhflags & MH_DWARF) != 0L)
#define racial_dwarf(mon) mon_has_race(mon, MH_DWARF)
#define is_gnome(ptr) (((ptr)->mhflags & MH_GNOME) != 0L)
#define racial_gnome(mon) mon_has_race(mon, MH_GNOME)
#define is_orc(ptr) (((ptr)->mhflags & MH_ORC) != 0L)
#define racial_orc(mon) mon_has_race(mon, MH_ORC)
#define is_human(ptr) (((ptr)->mhflags & MH_HUMAN) != 0L)
#define racial_human(mon) mon_has_race(mon, MH_HUMAN)
#define is_hobbit(ptr) (((ptr)->mhflags & MH_HOBBIT) != 0L)
#define racial_hobbit(mon) mon_has_race(mon, MH_HOBBIT)
#define is_giant(ptr) (((ptr)->mhflags & MH_GIANT) != 0L)
#define racial_giant(mon) mon_has_race(mon, MH_GIANT)
#define is_centaur(ptr) (((ptr)->mhflags & MH_CENTAUR) != 0L)
#define racial_centaur(mon) mon_has_race(mon, MH_CENTAUR)
#define is_illithid(ptr) (((ptr)->mhflags & MH_ILLITHID) != 0L)
#define racial_illithid(mon) mon_has_race(mon, MH_ILLITHID)
#define is_tortle(ptr) (((ptr)->mhflags & MH_TORTLE) != 0L)
#define racial_tortle(mon) mon_has_race(mon, MH_TORTLE)
#define is_drow(ptr) (((ptr)->mhflags & MH_DROW) != 0L)
#define racial_drow(mon) mon_has_race(mon, MH_DROW)
#define your_race(ptr) (((ptr)->mhflags & urace.selfmask) != 0L)
#define racial_match(mon) mon_has_race(mon, urace.selfmask)
#define is_bat(ptr) \
    ((ptr) == &mons[PM_BAT] || (ptr) == &mons[PM_GIANT_BAT] \
     || (ptr) == &mons[PM_VAMPIRE_BAT])
#define is_bird(ptr) ((ptr)->mlet == S_BAT && !is_bat(ptr))
#define has_beak(ptr) \
    (is_bird(ptr) || (ptr) == &mons[PM_TENGU] \
     || (ptr) == &mons[PM_VROCK]              \
     || (ptr) == &mons[PM_BABY_OWLBEAR]       \
     || (ptr) == &mons[PM_OWLBEAR]            \
     || (ptr) == &mons[PM_FELL_BEAST])
#define is_rat(ptr) \
    ((ptr) == &mons[PM_SEWER_RAT] || (ptr) == &mons[PM_GIANT_RAT]       \
     || (ptr) == &mons[PM_RABID_RAT] || (ptr) == &mons[PM_ENORMOUS_RAT] \
     || (ptr) == &mons[PM_RODENT_OF_UNUSUAL_SIZE])
#define has_trunk(ptr) \
    ((ptr) == &mons[PM_MUMAK] || (ptr) == &mons[PM_MASTODON] \
     || (ptr) == &mons[PM_WOOLLY_MAMMOTH])
#define is_golem(ptr) ((ptr)->mlet == S_GOLEM)
#define is_ogre(ptr) (((ptr)->mhflags & MH_OGRE) != 0L)
#define is_troll(ptr) (((ptr)->mhflags & MH_TROLL) != 0L)
#define is_gnoll(ptr) (((ptr)->mhflags & MH_GNOLL) != 0L)
#define is_spider(ptr) (((ptr)->mhflags & MH_SPIDER) != 0L)
#define is_not_zombie(ptr) \
    ((ptr) == &mons[PM_GHOUL] || (ptr) == &mons[PM_SKELETON] \
     || (ptr) == &mons[PM_REVENANT])
#define is_zombie(ptr) ((ptr)->mlet == S_ZOMBIE && !is_not_zombie(ptr))
#define can_become_zombie(ptr) \
    ((ptr)->mlet == S_KOBOLD || (ptr)->mlet == S_GIANT   \
     || (ptr)->mlet == S_HUMAN || (ptr)->mlet == S_KOP   \
     || ((ptr)->mlet == S_HUMANOID && !is_illithid(ptr)) \
     || (ptr)->mlet == S_GNOME || (ptr)->mlet == S_ORC)
#define can_become_flayer(ptr) \
    (!nonliving(ptr) && ((ptr)->mlet == S_HUMAN || (ptr)->mlet == S_KOP      \
                         || ((ptr)->mlet == S_HUMANOID && !is_illithid(ptr)) \
                         || (ptr)->mlet == S_GNOME || (ptr)->mlet == S_ORC   \
                         || (ptr)->mlet == S_GIANT))
#define is_domestic(ptr) (((ptr)->mflags2 & M2_DOMESTIC) != 0L)
#define is_demon(ptr) (((ptr)->mhflags & MH_DEMON) != 0L)
#define is_dragon(ptr) (((ptr)->mhflags & MH_DRAGON) != 0L)
#define is_pseudodragon(ptr) \
    ((ptr) == &mons[PM_PSEUDODRAGON] \
     || (ptr) == &mons[PM_ELDER_PSEUDODRAGON] || (ptr) == &mons[PM_ANCIENT_PSEUDODRAGON])
#define is_angel(ptr) (((ptr)->mhflags & MH_ANGEL) != 0L)
#define is_mercenary(ptr) (((ptr)->mflags2 & M2_MERC) != 0L)
#define is_rogue(ptr) \
    ((ptr) == &mons[PM_ROGUE] \
     || (ptr) == &mons[PM_HOBBIT_PICKPOCKET] || (ptr) == &mons[PM_GOLLUM])
#define is_male(ptr) (((ptr)->mflags2 & M2_MALE) != 0L)
#define is_female(ptr) (((ptr)->mflags2 & M2_FEMALE) != 0L)
#define is_neuter(ptr) (((ptr)->mflags2 & M2_NEUTER) != 0L)
#define is_wanderer(ptr) (((ptr)->mflags2 & M2_WANDER) != 0L)
#define always_hostile(ptr) (((ptr)->mflags2 & M2_HOSTILE) != 0L)
#define always_peaceful(ptr) (((ptr)->mflags2 & M2_PEACEFUL) != 0L)
#define race_hostile(ptr) (((ptr)->mhflags & urace.hatemask) != 0L)
#define race_peaceful(ptr) (((ptr)->mhflags & urace.lovemask) != 0L)
#define erac_race_hostile(mon) \
    (((has_erac(mon) ? ERAC(mon)->mrace : (mon)->data->mhflags) \
     & urace.hatemask) != 0L)
#define erac_race_peaceful(mon) \
    (((has_erac(mon) ? ERAC(mon)->mrace : (mon)->data->mhflags) \
     & urace.lovemask) != 0L)
#define extra_nasty(ptr) (((ptr)->mflags2 & M2_NASTY) != 0L)
#define strongmonst(ptr) (((ptr)->mflags2 & M2_STRONG) != 0L)
#define can_breathe(ptr) attacktype(ptr, AT_BREA)
#define cantwield(ptr) (nohands(ptr) || verysmall(ptr) || Hidinshell)
/* Does this type of monster have multiple weapon attacks?  If so,
   hero poly'd into this form can use two-weapon combat.  It used
   to just check mattk[1] and assume mattk[0], which was suitable
   for mons[] at the time but somewhat fragile.  This is more robust
   without going to the extreme of checking all six slots. */
#define could_twoweap(ptr) \
    ((  ((ptr)->mattk[0].aatyp == AT_WEAP)              \
      + ((ptr)->mattk[1].aatyp == AT_WEAP)              \
      + ((ptr)->mattk[2].aatyp == AT_WEAP)  ) > 1)
#define cantweararm(mon) (breakarm(mon) || sliparm(mon))
#define throws_rocks(ptr) \
    ((((ptr)->mflags2 & M2_ROCKTHROW) != 0L))
#define racial_throws_rocks(mon) \
    ((has_erac(mon) && ERAC(mon)->mflags2 & M2_ROCKTHROW) \
     || throws_rocks((mon)->data) \
     || ((mon) == &youmonst && !Upolyd && Race_if(PM_GIANT)))
#define type_is_pname(ptr) (((ptr)->mflags2 & M2_PNAME) != 0L)
#define is_lord(ptr) (((ptr)->mflags2 & M2_LORD) != 0L)
#define is_prince(ptr) (((ptr)->mflags2 & M2_PRINCE) != 0L)
#define is_ndemon(ptr) \
    (is_demon(ptr) && (((ptr)->mflags2 & (M2_LORD | M2_PRINCE)) == 0L))
#define is_dlord(ptr) (is_demon(ptr) && is_lord(ptr))
#define is_dprince(ptr) (is_demon(ptr) && is_prince(ptr))
#define is_minion(ptr) (((ptr)->mflags2 & M2_MINION) != 0L)
#define likes_gold(ptr) (((ptr)->mflags2 & M2_GREEDY) != 0L)
#define likes_gems(ptr) (((ptr)->mflags2 & M2_JEWELS) != 0L)
#define likes_objs(ptr) (((ptr)->mflags2 & M2_COLLECT) != 0L || is_armed(ptr))
#define likes_magic(ptr) (((ptr)->mflags2 & M2_MAGIC) != 0L)
#define webmaker(ptr) (is_spider(ptr) || is_drow(ptr))
#define is_unicorn(ptr) ((ptr)->mlet == S_UNICORN && likes_gems(ptr))
#define is_cavelizard(ptr) \
    ((ptr) == &mons[PM_CAVE_LIZARD] || (ptr) == &mons[PM_LARGE_CAVE_LIZARD])
#define is_longworm(ptr) \
    (((ptr) == &mons[PM_BABY_LONG_WORM]) || ((ptr) == &mons[PM_LONG_WORM]) \
     || ((ptr) == &mons[PM_LONG_WORM_TAIL]))
#define is_jabberwock(ptr) (((ptr)->mhflags & MH_JABBERWOCK) != 0L)
#define is_covetous(ptr) (((ptr)->mflags3 & M3_COVETOUS))
#define is_skittish(ptr) (((ptr)->mflags3 & M3_SKITTISH))
#define is_accurate(ptr) \
    (((ptr)->mflags3 & M3_ACCURATE) \
     || ((ptr) == youmonst.data && !Upolyd && Race_if(PM_CENTAUR)))
#define is_berserker(ptr) (((ptr)->mflags3 & M3_BERSERK))
#define infravision(ptr) (((ptr)->mflags3 & M3_INFRAVISION))
#define infravisible(ptr) (((ptr)->mflags3 & M3_INFRAVISIBLE))
#define ultravision(ptr) (((ptr)->mflags3 & M3_ULTRAVISION))
#define is_displacer(ptr) (((ptr)->mflags3 & M3_DISPLACES) != 0L)
#define is_displaced(ptr) \
    ((ptr) == &mons[PM_SHIMMERING_DRAGON]         \
     || (ptr) == &mons[PM_BABY_SHIMMERING_DRAGON] \
     || (ptr) == &mons[PM_DISPLACER_BEAST])
#define is_mplayer(ptr) \
    (((ptr) >= &mons[PM_ARCHEOLOGIST]) && ((ptr) <= &mons[PM_WIZARD]))
#define is_watch(ptr) \
    ((ptr) == &mons[PM_WATCHMAN] || (ptr) == &mons[PM_WATCH_CAPTAIN])
#define is_rider(ptr) \
    ((ptr) == &mons[PM_DEATH] || (ptr) == &mons[PM_FAMINE] \
     || (ptr) == &mons[PM_PESTILENCE])
#define is_placeholder(ptr) \
    ((ptr) == &mons[PM_ORC] || (ptr) == &mons[PM_GIANT]         \
     || (ptr) == &mons[PM_ELF] || (ptr) == &mons[PM_HUMAN]      \
     || (ptr) == &mons[PM_CENTAUR] || (ptr) == &mons[PM_DEMON]  \
     || (ptr) == &mons[PM_DWARF] || (ptr) == &mons[PM_GNOME]    \
     || (ptr) == &mons[PM_ILLITHID] || (ptr) == &mons[PM_NYMPH] \
     || (ptr) == &mons[PM_DROW])

/* Ice Queen branch defines */
#define is_iceq_only(ptr) \
    ((ptr) == &mons[PM_SNOW_GOLEM] || (ptr) == &mons[PM_WOOLLY_MAMMOTH]        \
     || (ptr) == &mons[PM_SABER_TOOTHED_TIGER] || (ptr) == &mons[PM_ICE_NYMPH] \
     || (ptr) == &mons[PM_FROST_SALAMANDER] || (ptr) == &mons[PM_REVENANT])
#define freeze_step(ptr) \
    ((ptr) == &mons[PM_SNOW_GOLEM] || (ptr) == &mons[PM_FROST_SALAMANDER] \
     || (ptr) == &mons[PM_ABOMINABLE_SNOWMAN])
#define likes_iceq(ptr) \
    ((ptr) == &mons[PM_SNOW_GOLEM] || (ptr) == &mons[PM_OWLBEAR]                  \
     || (ptr) == &mons[PM_WOLF] || (ptr) == &mons[PM_WEREWOLF]                    \
     || (ptr) == &mons[PM_WINTER_WOLF_CUB] || (ptr) == &mons[PM_WINTER_WOLF]      \
     || (ptr) == &mons[PM_WARG] || (ptr) == &mons[PM_FREEZING_SPHERE]             \
     || (ptr) == &mons[PM_LYNX] || (ptr) == &mons[PM_BLUE_JELLY]                  \
     || (ptr) == &mons[PM_GOBLIN_OUTRIDER] || (ptr) == &mons[PM_GOBLIN_CAPTAIN]   \
     || (ptr) == &mons[PM_MASTODON] || (ptr) == &mons[PM_WOOLLY_MAMMOTH]          \
     || (ptr) == &mons[PM_ICE_VORTEX] || (ptr) == &mons[PM_MOUNTAIN_CENTAUR]      \
     || (ptr) == &mons[PM_BABY_WHITE_DRAGON] || (ptr) == &mons[PM_WHITE_DRAGON]   \
     || (ptr) == &mons[PM_BABY_SILVER_DRAGON] || (ptr) == &mons[PM_SILVER_DRAGON] \
     || (ptr) == &mons[PM_BROWN_MOLD] || (ptr) == &mons[PM_FROST_GIANT]           \
     || (ptr) == &mons[PM_ICE_TROLL] || (ptr) == &mons[PM_YETI]                   \
     || (ptr) == &mons[PM_SASQUATCH] || (ptr) == &mons[PM_SABER_TOOTHED_TIGER]    \
     || (ptr) == &mons[PM_FROST_SALAMANDER] || (ptr) == &mons[PM_ICE_NYMPH]       \
     || (ptr) == &mons[PM_REVENANT] || (ptr) == &mons[PM_BABY_OWLBEAR]            \
     || (ptr) == &mons[PM_HUMAN_ZOMBIE] || (ptr) == &mons[PM_GIANT_ZOMBIE]        \
     || (ptr) == &mons[PM_LICH])
/* Goblin Town branch defines */
#define likes_gtown(ptr) \
    ((ptr)->mlet == S_ORC || (ptr)->mlet == S_KOBOLD || is_rat(ptr))
/* Purgatory defines */
#define likes_purg(ptr) \
    ((ptr)->mlet == S_DRAGON || (ptr)->mlet == S_NYMPH                 \
     || (ptr)->mlet == S_UNICORN || (ptr)->mlet == S_CENTAUR           \
     || (ptr)->mlet == S_JABBERWOCK || (ptr)->mlet == S_NYMPH          \
     || ((ptr) >= &mons[PM_ARCHEOLOGIST] && (ptr) <= &mons[PM_WIZARD]) \
     || (ptr) == &mons[PM_SPECTRE] || (ptr) == &mons[PM_GHOST])

/* macros for various monsters affected by specific types of damage */
#define can_vaporize(ptr) \
    ((ptr) == &mons[PM_WATER_ELEMENTAL] || (ptr) == &mons[PM_ICE_VORTEX]     \
     || (ptr) == &mons[PM_BABY_SEA_DRAGON] || (ptr) == &mons[PM_SEA_DRAGON])

#define can_freeze(ptr) \
    ((ptr) == &mons[PM_WATER_ELEMENTAL]                                      \
     || (ptr) == &mons[PM_BABY_SEA_DRAGON] || (ptr) == &mons[PM_SEA_DRAGON])

#define can_corrode(ptr) \
    ((ptr) == &mons[PM_IRON_GOLEM] || (ptr) == &mons[PM_IRON_PIERCER])

/* various monsters move faster underwater vs on land */
#define is_fast_underwater(ptr) \
    (is_tortle(ptr) || (ptr) == &mons[PM_WATER_TROLL]                        \
     || (ptr) == &mons[PM_GIANT_TURTLE] || (ptr) == &mons[PM_BABY_CROCODILE] \
     || (ptr) == &mons[PM_CROCODILE] || (ptr) == &mons[PM_GIANT_CROCODILE]   \
     || (ptr) == &mons[PM_BABY_SEA_DRAGON] || (ptr) == &mons[PM_SEA_DRAGON])

/* return TRUE if the monster tends to revive */
#define is_reviver(ptr) (is_rider(ptr) || (ptr)->mlet == S_TROLL \
                         || is_zombie(ptr))
/* monsters whose corpses and statues need special handling;
   note that high priests and the Wizard of Yendor are flagged
   as unique even though they really aren't; that's ok here */
#define unique_corpstat(ptr) (((ptr)->geno & G_UNIQ) != 0)

/* never leaves a corpse */
#define no_corpse(ptr) (((ptr)->geno & G_NOCORPSE) != 0)

/* monsters that cannot be genocided until Vecna has been destroyed */
#define no_geno_vecna(ptr) (((ptr)->geno & G_VECNA) != 0)

/* monsters that cannot be genocided until Vlad the Impaler has been destroyed */
#define no_geno_vlad(ptr) (((ptr)->geno & G_VLAD) != 0)

/* this returns the light's range, or 0 if none; if we add more light emitting
   monsters, we'll likely have to add a new light range field to mons[].
   shadow dragon types emit darkness instead of light, see do_light_sources()
   in light.c */
#define emits_light(ptr) \
    (((ptr)->mlet == S_LIGHT                          \
      || (ptr) == &mons[PM_FLAMING_SPHERE]            \
      || (ptr) == &mons[PM_SHOCKING_SPHERE]           \
      || (ptr) == &mons[PM_BABY_GOLD_DRAGON]          \
      || (ptr) == &mons[PM_FIRE_VORTEX])              \
         ? 1                                          \
         : ((ptr) == &mons[PM_FIRE_ELEMENTAL]         \
            || (ptr) == &mons[PM_BABY_GOLD_DRAGON]    \
            || (ptr) == &mons[PM_BABY_SHADOW_DRAGON]) \
           ? 2                                        \
           : ((ptr) == &mons[PM_LUCIFER]              \
              || (ptr) == &mons[PM_GOLD_DRAGON]       \
              || (ptr) == &mons[PM_SHADOW_DRAGON]     \
              || (ptr) == &mons[PM_TIAMAT])           \
             ? 3 : 0)
    /* [Note: the light ranges above were reduced to 1 for performance,
     *  otherwise screen updating on the plane of fire slowed to a crawl.
     *  Note too: that was with 1990s hardware and before fumarole smoke
     *  blocking line of sight was added, so might no longer be necessary.] */
#define likes_lava(ptr) \
    ((ptr) == &mons[PM_FIRE_ELEMENTAL] || (ptr) == &mons[PM_SALAMANDER]   \
     || (ptr) == &mons[PM_LAVA_DEMON] || (ptr) == &mons[PM_LAVA_GREMLIN])

#define pm_invisible(ptr) \
    ((ptr) == &mons[PM_STALKER] || (ptr) == &mons[PM_BLACK_LIGHT])

/* could probably add more */
#define likes_fire(ptr) \
    ((ptr) == &mons[PM_FIRE_VORTEX] || (ptr) == &mons[PM_FLAMING_SPHERE] \
     || likes_lava(ptr))

#define likes_ice(ptr) \
    ((ptr) == &mons[PM_FROST_SALAMANDER])

#define touch_petrifies(ptr) \
    ((ptr) == &mons[PM_COCKATRICE] || (ptr) == &mons[PM_CHICKATRICE] \
     || (ptr) == &mons[PM_BASILISK])

#define is_mind_flayer(ptr) \
    ((ptr) == &mons[PM_MIND_FLAYER] || (ptr) == &mons[PM_MASTER_MIND_FLAYER] \
     || (ptr) == &mons[PM_ALHOON])

#define is_vampire(ptr) ((ptr)->mlet == S_VAMPIRE)

#define hates_light(ptr) \
    ((ptr) == &mons[PM_GREMLIN] || is_drow(ptr) \
     || (ptr) == &mons[PM_BABY_SHADOW_DRAGON]   \
     || (ptr) == &mons[PM_SHADOW_DRAGON])

/* used to vary a few messages */
#define weirdnonliving(ptr) (is_golem(ptr) || (ptr)->mlet == S_VORTEX)
#define nonliving(ptr) \
    (is_undead(ptr) || (ptr) == &mons[PM_MANES] || weirdnonliving(ptr))

/* no corpse (ie, blank scrolls) if killed by fire */
#define completelyburns(ptr) \
    ((ptr) == &mons[PM_PAPER_GOLEM] || (ptr) == &mons[PM_STRAW_GOLEM])

/* monster can mount and ride other monsters */
#define mon_can_ride(mon) \
    (!(mon)->mtame && (mon) != u.ustuck && !(mon)->mpeaceful                 \
     && !(mon)->mtrapped && humanoid((mon)->data) && !is_zombie((mon)->data) \
     && !r_bigmonst(mon) && !is_animal((mon)->data) && !is_were((mon)->data) \
     && (mon)->data->mlet != S_MUMMY && (mon)->data->mlet != S_LIZARD        \
     && !r_verysmall(mon) && !is_shapeshifter((mon)->data)                   \
     && (mon)->mcanmove && !(mon)->msleeping && (mon)->cham == NON_PM        \
     && !unsolid((mon)->data) && !((mon)->mstrategy & STRAT_WAITFORU)        \
     && !is_covetous((mon)->data))
/* monster can be ridden by other monsters */
#define mon_can_be_ridden(mon) \
    (can_saddle(mon) && !DEADMONSTER(mon) && !is_covetous((mon)->data)       \
     && !(mon)->mtame && (mon) != u.ustuck && !(mon)->mpeaceful              \
     && !(mon)->mtrapped && (mon)->mcanmove && !(mon)->msleeping             \
     && !is_shapeshifter((mon)->data) && !is_were((mon)->data)               \
     && !(mon)->isshk && (mon)->data->mlet != S_DOG && (mon)->cham == NON_PM \
     && !((mon)->mstrategy & STRAT_WAITFORU))

/* Used for conduct with corpses, tins, and digestion attacks */
/* G_NOCORPSE monsters might still be swallowed as a purple worm */
/* Maybe someday this could be in mflags... */
#define vegan(ptr) \
    ((ptr)->mlet == S_BLOB || (ptr)->mlet == S_JELLY                \
     || (ptr)->mlet == S_FUNGUS || (ptr)->mlet == S_VORTEX          \
     || (ptr)->mlet == S_LIGHT                                      \
     || ((ptr)->mlet == S_ELEMENTAL && (ptr) != &mons[PM_STALKER])  \
     || ((ptr)->mlet == S_GOLEM && (ptr) != &mons[PM_FLESH_GOLEM]   \
         && (ptr) != &mons[PM_LEATHER_GOLEM]) || noncorporeal(ptr))
#define vegetarian(ptr) \
    (vegan(ptr)         \
     || ((ptr)->mlet == S_PUDDING && (ptr) != &mons[PM_BLACK_PUDDING]))
/* jello-like creatures */
#define can_flollop(ptr) \
    ((ptr)->mlet == S_BLOB || (ptr)->mlet == S_JELLY \
     || (ptr)->mlet == S_PUDDING)

/* monkeys are tameable via bananas but not pacifiable via food,
   otherwise their theft attack could be nullified too easily;
   dogs and cats can be tamed by anything they like to eat and are
   pacified by any other food;
   horses can be tamed by always-veggy food or lichen corpses but
   not tamed or pacified by other corpses or tins of veggy critters */
#define befriend_with_obj(ptr, obj) \
    (((ptr) == &mons[PM_MONKEY] || (ptr) == &mons[PM_APE])                   \
     ? (obj)->otyp == BANANA                                                 \
     : ((is_domestic(ptr)                                                    \
         || (is_rat(ptr) && Role_if(PM_CONVICT))                             \
         || (((is_spider(ptr) && (ptr) != &mons[PM_DRIDER])                  \
             || is_cavelizard(ptr)) && Race_if(PM_DROW))                     \
         || ((ptr) == &mons[PM_WARG] && Race_if(PM_ORC))                     \
         || ((ptr) == &mons[PM_SABER_TOOTHED_TIGER] && Role_if(PM_CAVEMAN))) \
        && (obj)->oclass == FOOD_CLASS                                       \
        && ((ptr)->mlet != S_UNICORN                                         \
            || obj->material == VEGGY                                        \
            || ((obj)->otyp == CORPSE && (obj)->corpsenm == PM_LICHEN))))

/* Noise that a monster makes when engaged in combat. Assume that vocalizations
 * account for some noise, so monsters capable of vocalizing make more.
 * This gets used as an argument to wake_nearto, which expects a squared value,
 * so we square the result. */
#define combat_noise(ptr) \
    ((ptr)->msound ? ((ptr)->msize * 2 + 1) * ((ptr)->msize * 2 + 1) \
                   : ((ptr)->msize + 1) * ((ptr)->msize + 1))

/* monsters not technically killed, but defeated instead */
#define is_defeated(ptr) \
    ((ptr) == &mons[PM_KATHRYN_THE_ICE_QUEEN]                  \
     || (ptr) == &mons[PM_BOURBON] || (ptr) == &mons[PM_OZZY])

#define is_racialmon(ptr) (is_mplayer(ptr) || is_mercenary(ptr))

#define M_IN_WATER(ptr) \
    ((ptr)->mlet == S_EEL || amphibious(ptr) || is_swimmer(ptr))

#endif /* MONDATA_H */
