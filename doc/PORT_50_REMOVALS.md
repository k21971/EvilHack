# EvilHack -> NetHack 5.0: Removed Upstream Behavior

Companion to `doc/PORT_50_TRACKER.md`. This file lists the NetHack 5.0
features and behaviors that the `evilhack-50` branch has removed (or
replaced with master's shape), plus removals the maintainer has asked
for but that have not been implemented yet. The tracker stays the
source of truth for each slice's evidence; this file is the index.

Compiled 2026-10-17 from a full sweep of the tracker (Dashboard,
Phases 0-7, Decisions log) and the rulings memory. Headline items
spot-checked in source the same day: `STASIS`, `PARANOID_TRAP`,
`mwandexp` / `buzz_force_miss` and `special_throne_effect` are absent
from `src/` and `include/`; the mysterious force sits under `#if 0`
in `goto_level()`; the wish livelog has no `, got "<object>"` tail.

Maintenance: when a slice removes a 5.0 behavior, add a line under
section 2; when a planned item lands, move it from section 1 to
section 2 with its BUG ids and commit. Statuses: `[ ]` planned (no
code yet), `[x]` removed (landed), `[r]` reversed (5.0 kept after
all), `[?]` open ruling.

## 1. Planned removals (maintainer requests, 2026-10-17)

No code has changed for any of these. Each needs its own plan and
slice. Scopes below were ruled via AskUserQuestion on 2026-10-17;
1.4, 1.6 and 1.3's food death-drops on 2026-10-18.

### 1.1 [ ] Generic-object display (true colors only up close)

- **5.0 source:** 85c908cb0 (PatR 2023-01-10, "displaying generic
  objects") + 52a44b325 (github #967, "nearby objects").
- **What it does:** a potion, a real or glass gem, or a spellbook
  (blank included; not the novel or the Book of the Dead) whose
  `dknown` is unset maps to a per-class generic glyph with no
  description color. It becomes specific only when in sight within
  `neardist` (`r * r * 2 - r`, `r = max(xray_range, 2)`, about two
  squares).
- **Branch sites:** `obj_is_generic()`, `generic_obj_to_glyph()` and
  the `glyph_is_*generic*` family in include/display.h;
  `map_object()` and `see_nearby_objects()` in display.c; the
  "generic object" lookup text in display.c; the generic glyph range
  in glyphs.c and the tile mapping; `display_monster()`'s
  `M_AP_OBJECT` arm and the branch's `M_AP_F_DKNOWN` mimic bit
  (Decisions log 2026-09-18), which exists only to serve this
  mechanism.
- **Master:** no generic glyphs; objects show their own glyph and
  color at any distance. `distant_name()` (the "a potion" vs "a ruby
  potion" naming at range) is in master/3.6 too, so it stays.
- **Scope ruled:** the whole mechanism. The maintainer also mentioned
  terrain and monsters; the trace found no terrain or real-monster
  equivalent, only mimics disguised as these items, which follow the
  object rule automatically. Re-check if a terrain or monster case
  turns up in play.

### 1.2 [ ] Iron footwear vs traps (polymorph trap boot swap)

- **5.0 source:** b6df17f34 (2026-03-17, "Iron shoes protect /
  partially protect against certain trap types") + 89473216a
  (2026-04-14, "Iron footwear and traps balance fixes").
  `wearing_iron_shoes()` in trap.c tests the base `oc_material`.
- **Branch behaviors, all six to go:**
  1. `trapeffect_poly_trap()`: a hero or monster in iron boots does
     not polymorph; the trap is used up, "<boots> warps strangely."
     and the boots `poly_obj()` between DWARVISH_BOOTS and
     KICKING_BOOTS (the branch's adaptation of 5.0's IRON_SHOES).
  2. `trapeffect_bear_trap()`: "<boots> protects your leg.", no
     wound; a trapped monster skips its d(2,4).
  3. `trapeffect_pit()`: spiked-pit spikes blocked, hero and monster.
  4. `trapeffect_anti_magic()`: positively enchanted iron boots lose
     enchantment instead of the wearer being drained.
  5. `trapeffect_landmine()`: damage `(rnd(16) + 3) / 4`.
  6. `find_misc()` (muse.c): a monster in iron boots will not pick
     MUSE_POLY_TRAP.
- **Master:** none of it. A polymorph trap polymorphs you (magic
  resistance and Unchanging excepted) and boots give no trap
  protection.
- **Scope ruled:** the whole package, not just the polymorph trap
  (see memory feedback-evilhack-not-easyhack).

### 1.3 [ ] Early-game supply chests and Mines-level food

- **5.0 source:** ae3e5d281 (Alex Smith 2023-12-02, "Add bonus items
  to some early-game Dungeons levels"), part of the same early-game
  series as 62fb87593 (healing potions much more common).
- **Branch sites:** `fill_ordinary_room()`'s `bonus_items` arm and
  `makelevel()`'s `bonus_item_room_countdown` room pick (mklev.c).
  - Mines branch level: a guaranteed food ration (3 in 5), cram
    ration or lembas wafer.
  - Dungeons levels above the Oracle: 2 in 3 chance of a supply
    chest (chest 2 in 3, else large box; locked 5 in 6). Half the
    time it holds a potion of healing (half of those quantity 2);
    otherwise one of extra healing, speed, gain energy, enchant
    weapon, enchant armor, confuse monster, scare monster, wand of
    digging or spellbook of healing, rerolled to noncursed, plus
    maybe one random extra item.
- **Already master's:** potion probabilities and price. 62fb87593's
  healing 115 / base price 20 never landed; the branch uses master's
  objects table (healing 57 / price 100, extra healing 47).
- **Scope ruled:** remove the supply chests AND the Mines food.
- **Ruled 2026-10-18 into this removal:** 39a7dc1c5 (same series):
  most monsters no longer death-drop newly created food in `xkilled()`
  (only M2_COLLECT monsters keep it, which in EvilHack covers most
  humanoids, so beasts and dragons drop none). Live on the branch;
  master death-drops food. Remove the delete-food arm with the
  chests: keeping it without them leaves less food than either tree.
  Census the rest of the series at slice time: 87f3b4809 (Monk
  quest home extra rations), 9d910773d (starting magic markers),
  e8ca614e5 (multi-step alchemy yields), 319dfbdaa (Wizards learn
  spellbooks with skill), 6acf5a778 (potions destroyed on very hot
  ground), 4a00b66b3 (oil boiling point).

### 1.4 [ ] HP regeneration -> master's formula

- **5.0 source:** f3408b87b (Alex Smith 2023-12-11, "A new HP
  regeneration formula"): (XL + Con)% chance of 1 HP each turn;
  Regeneration adds 1 every turn unconditionally; restful sleep adds
  1 (91c46ab54, 2020). Stated intent: no mid-fight regeneration,
  potions needed in the mid and late game.
- **Master (3.6):** XL 1-9: 1 HP every `42 / (XL + 2) + 1` turns.
  XL 10+: every 3 turns, 1 HP if Con <= 12, else `rnd(Con)` capped at
  XL - 9. Regeneration only fills a turn that healed nothing.
- **Branch today:** 5.0's model, in `regen_hp()` (allmain.c), with
  master's terms layered on top: drow in light at a third (the
  `rn2(300)` die, ruled 2026-09-18), Druid near vegetation x1.5,
  tortle shell, elf / orc metal-contact gates (BUG-1696, 6bd028b4b),
  Infidel without the Amulet, the Valley, the Wand of Orcus,
  Withering.
- **Rates** (HP per turn, unencumbered, no Regeneration):

  | XL | Con | master | 5.0 / branch | 5.0 vs master |
  |----|-----|--------|--------------|---------------|
  | 1  | 16  | 0.07   | 0.17         | 2.5x faster   |
  | 5  | 16  | 0.14   | 0.21         | 1.5x faster   |
  | 9  | 16  | 0.25   | 0.25         | even          |
  | 10 | 16  | 0.33   | 0.26         | 0.8x          |
  | 14 | 16  | 1.46   | 0.30         | 5x slower     |
  | 20 | 18  | 2.65   | 0.38         | 7x slower     |
  | 30 | 18  | 3.17   | 0.48         | 6.6x slower   |
  | 20 | 10  | 0.33   | 0.30         | about even    |

  The gap is master's `rnd(Con)` jump at XL 10 for Con 13+.
- **Coupled terms to name in any ruling:** Regeneration's +1
  semantics, restful sleep, the polymorphed `u.mh` arm, and the
  layered EvilHack terms above, which assume 5.0's die.
- **Scope ruled 2026-10-18:** master's formula exactly, rejecting 5.0's
  roll and a slower-of-the-two hybrid. 5.0 paired its roll with
  commoner healing potions (never landed) and the supply chests
  (1.3), so the branch was 5-7x slower than 0.9.3 from XL 14 with
  Con 13+. Regeneration goes back to "at least 1 HP a turn". The
  drow-in-light (interval x3) and Druid (interval - 1) terms go back
  to master's forms, which retires the 2026-09-18 `rn2(300)`
  adaptation. Restful sleep healing (91c46ab54) is dropped too:
  `U_CAN_REGEN()`'s sleep term and its +1, in both the `u.uhp` and
  the polymorphed `u.mh` arms. The polymorphed arm is otherwise
  master's already.

### 1.5 [ ] Hitpointbar blink and dashes at critical HP

- **5.0 source:** 7da370d74 (PatR 2024-04-11, "experimental
  hitpointbar change").
- **What it does:** when `critically_low_hp(TRUE)` holds, the tty and
  curses hitpointbar blinks and `repad_with_dashes()` turns the
  title's trailing space pairs into " -". Curses also stopped honoring
  the HP highlight's attribute for the bar and forces inverse, as tty
  does.
- **Branch sites:** wintty.c's `hpbar_crit_hp` (set in the BL_HP arm
  of the status update, read in the hitpointbar render, which also
  blinks the second part); cursstat.c's `hpbar_crit_hp` and its
  `HL_BLINK` / forced `HL_INVERSE` in `hpbar_color`, read in
  `curs_HPbar()`; `repad_with_dashes()` in botl.c and its extern.h
  prototype (no other caller). Qt and the Windows GUI do not have the
  blink. Guidebook.mn / .tex: the "For tty and curses, blink will also
  be used..." sentence, and the "video attributes ... ignored in favor
  of inverse" sentence, which no longer holds for curses.
- **Master:** tty draws the filled part in inverse plus the HP color
  and the rest plain; curses uses the HP highlight's color and
  attribute (`hpbar_color = color_and_attr`). No blink, no dashes.
- **Scope ruled 2026-10-17:** the bar goes back to master's shape in
  both tty and curses. KEEP 5.0's opt-in `criticalhp` status-hilite
  rule (df2799faf, not on master; e.g.
  `hilite_status:hitpoints/criticalhp/red&bold`). It only acts when a
  player configures it, and the bar still takes its color.

## 2. Removed so far (landed)

All landed unless noted. BUG ids and commits as recorded in the
tracker; sNNN = port session.

### 2.1 Whole features, items and options

- [x] **Mysterious force**: the hero's Gehennom climb and the
  carrier's MUSE_UPSTAIRS trip, plus 5.0's decaying `mysteryforce`
  and its Infidel exemption. An Amulet carrier's own teleports are
  always refused (5.0: 2 in 3) (s140, BUG-1924..1929, f37a4fc10;
  master ad34e781a / f6063dcd8).
- [x] **Wand of stasis**: declined 2026-07-12. The `stasis_until`
  machinery is left dormant. Its side effect, covetous monsters exempt
  from no-teleport levels via `noteleport_level()`, was dropped too
  (s153, BUG-1985/1986, 0f66f9596; upstream 07fc4904c).
- [x] **paranoid_confirmation:trap**: both questions (trap and gas
  cloud), the 'trap' / 'move-trap' values, PARANOID_TRAP and
  `immune_to_trap()` (s237, BUG-2410, ac60731fc).
- [x] **First attack-wand zap always misses**: mwandexp,
  buzz_force_miss, `dobuzz()` forcemiss (s223, BUG-2330, 8d1bbbaa4;
  upstream 4d043a6f9).
- [x] **Worn-gear inventory shield**: 99% / 90% item protection from
  worn resistance sources, its ^X lines and the disn_prot arm (s147,
  BUG-1950, 6ace2e519; upstream 9942b65df + 7ca995199).
- [x] **Vlad's throne `special_throne_effect()`** (s161, BUG-2038,
  8bfa2121b; upstream 3fd30c4ec).
- [x] **ESC or empty answer at the wish prompt giving a random
  object**: now "Really forfeit this wish?" (s161, BUG-2033,
  8bfa2121b).
- [x] **Dragon scale mail objects**: declined 2026-07-12, superseded
  by EvilHack scale fusion; wishes redirect to scales.
- [x] **Monster bag rummaging** (MUSE_BAG / `mloot_container()`)
  (2026-08-27, 0ed4ae627).
- [x] **Plain-quarterstaff spellcasting bonus** (s165, BUG-2058,
  5f0fcc31a; upstream 780d30eca).
- [x] **Cloaks surviving an armor-bursting polymorph**: upstream
  ceaffcbd4 declined (s157, BUG-1997, dc3b7a129).
- [x] **Paranoid swim on by default with a silent refusal**: master's
  "Really enter...?" with a decline penalty, off by default (ruled
  s184, landed s234, BUG-2398, b6310f0e4; master 2c50b94ab).
- [x] **Paranoid quit on a plain 'y'**: master's typed "yes" default
  (s237, BUG-2411, ac60731fc; master 367af1633).
- [x] **Permanent invisibility / see invisible** from potion, wand,
  fountain, throne and stalker corpse; 5.0's permchance; the cursed
  potion's strip; 5.0's magic-trap fates -> master's `domagictrap()`
  (s195, BUG-2203..2213, 911c03a78).
- [x] **Promptless area fireball / cone of cold**: master's "Cast
  advanced spell?" prompt with a decline refund (BUG-1638, c29302d7f).
- [x] **Catching thrown items** narrowed: only a Skilled+ Monk or
  Samurai with both hands empty; 5.0's any-free-hand catch and the
  Rogue bonus are gone (s172, BUG-2100, c1e2b1189).
- [x] **Queen ant eggs**: queen ants no longer lay eggs (beyond
  master, maintainer request) (s162, BUG-2040..2049, 4715c6077).
- [x] **First-touch Amulet wish** (`u.uevent.amulet_wish`; a Hardfought
  server patch, not vanilla 5.0): deleted, the bit left dormant
  (2026-07-22).
- [x] **wizweight**, wizard-only, with its ", N aum)" merge into the
  last parenthesis: master's invweight for every player, the weight
  always its own " (N aum)"; 5.0's removal of glob weights from shop
  prices (296548852) reverted (s249, BUG-2484..2486; master
  90c12021c).
- [x] **Small purses weightless for encumbrance**: 5.0's 90aded364
  gave 1..49 coins 1 aum in weight() only; inv_weight() and pickup's
  lift math now count it too (ruled s250, BUG-2493, 34b404ad1;
  beyond master, which counts 0 everywhere).
- [x] **Traditional identify re-asking on ESC**: ESC at "What kinds
  of thing do you want to identify?" now goes to the identify menu,
  so a hangup can no longer spin there (s250, BUG-2499, 34b404ad1;
  master 8966b35a7).
- [x] **A lone 'A' rejected in the drop and container category
  menus**: accepted when `autoall_menu` is on, where it leads to the
  item menu with everything selected; 5.0's "No relevant items
  selected." stays with the option off (ruled s329, BUG-3108,
  c9b8b4087).

### 2.2 Monster AI

- [x] Keep-away arms: ranged hang-back (BUG-1947), aklys preferred
  range (BUG-1948), polearm stand-off (BUG-1949) (s146, 654aaea2d /
  500e50988).
- [x] Resistance memory gates: gazers stop after a seen resist
  (BUG-1945; upstream 0a3d02b9d); `dochug()` ranged memory (BUG-1946;
  upstream 9a42d7090) (s145, a8988bd19); breath and spell casters
  stop after a seen reflection or resist (s136).
- [x] Covetous: the Gehennom demon court freezing covetous warps;
  5.0's distance harass (idling 4 turns in 5) -> master's pursuit
  (ruled s161, landed s169, BUG-2081..2087, 0547e1f4e).
- [x] Tengu-only random blink -> master's TELEPORT-property blink and
  `decide_to_teleport()` (s139, BUG-1920..1923, 349aa59b5).
- [x] Amulet carriers standing to fight and the HP gate on their
  escapes -> master 72134de80 (BUG-1934..1940, ff2ee33d4 / 69e3c6f3f).
- [x] Ungated pick-up-in-passing in postmov (s144, BUG-1942..1944,
  cb16d5cfe; upstream 6c9700ab2).
- [x] Jabberwocks generated asleep (s171, BUG-2098, 34ed5ca2a).
- [x] Vampire-bat form permanently stunned (s135, BUG-1895, 408713be5).
- [x] Monster-on-monster theft taking the first eligible item; now
  random (s159, BUG-2024, 0612d2a42).
- [x] Any swimmer attacking a submerged hero from dry land -> master's
  rule: only a monster that is in the water attacks one who is under
  it (s329, BUG-3097, 8bc94ee80; master 442a2ac22).
- [x] Salamanders roaming off lava -> master's `wantlava`: one beside
  lava goes in and stays, as an eel does with water; the frost
  salamander likewise with ice (s329, BUG-3107, dbbbbc605;
  master c839aa4a1, 113aae435).
- [x] A giant-, centaur- or tortle-race monster sized by its base
  form (a soldier is human-sized) -> by its race: as a pet it "won't
  fit through" a tight diagonal swap between rock or wall (s330,
  BUG-3112, commit pending; master 1e835fae5), and it does not walk
  that diagonal either (ruled s330, BUG-3118, commit pending; beyond
  master, whose walking test reads the base form).
- [x] A zombie moaning 1 time in 10 when the hero's zap or boomerang
  reaches it (5.0 moved dochug()'s roll into m_respond()) -> every
  time; 1 in 10 on its own turns (s330, BUG-3116, commit pending;
  master's m_respond()).
- [x] Warning: the weakest zombies (base level 0-3) never shown, a
  beholder shown by its level -> every hostile zombie in range warns
  as at least a 1, a beholder as a 5 (s330, BUG-3114, 3115, commit
  pending; master 785716ffd, f059a60fd).

### 2.3 Combat and magic rules -> master's

- [x] The Tsurugi of Muramasa's natural 1 cutting a giant-, centaur-
  or tortle-race monster in half -> "You slice deeply into" it for
  double damage, as for any big monster (s330, BUG-3111, commit
  pending; master 1e835fae5).
- [x] Knockback: 5.0's size-gated knockback, Ogresmasher's 1 in 2, the
  hero knocking monsters back, the post-hurtle stun. Only AD_CLOB
  knocks back (s109, BUG-1819..1824).
- [x] Reflection / magic resistance (s136, BUG-1900..1911): full
  reflection -> partial (half dice); MR vs death ray ("You aren't
  affected.") -> drain; touch of death's escape roll and 50+8d6 drain;
  MR blocking magic missiles -> halving; the gas deferral in
  `dobuzz()`.
- [x] Luckstone stopping the luck timeout -> slowing it (s137,
  BUG-1912/1913, 7583fc0dd; the lucky fedora was KEPT).
- [x] Lava burn: 5.0's damage-capped `destroy_items()` that spared a
  fire-resistant hero -> master's `fire_damage()` chain (s149,
  BUG-1961..1971, dfedd36aa).
- [x] Amnesia and traps: 5.0 04c59fff0's no-trap-forgetting reverted
  (BUG-2051); mind flayer AD_DRIN odds (d6dd5c743) -> master's
  (BUG-2052) (s163, 2e994ba6e).
- [x] Throne effect 6 wish -> master's 1 in 5 with the throne vanishing
  (BUG-2036/2037); water demon wish may dry the fountain (BUG-2039)
  (s161, 8bfa2121b).
- [x] `onscary()`'s magical-only gate (`magical_scare`) (s131,
  BUG-1876/1877).
- [x] Wrap attacks: 5.0's flat 1 in 10 grab -> master's odds and texts
  (s158, BUG-2012..2020, 77a47c48e).
- [x] Thrown potion of paralysis on monsters: `rnd(25)` -> master's
  dilution-aware dice (s110, BUG-1826; master 32af2161a).
- [x] Bag of holding explosion: master's odds, d(8,10) damage, scatter
  and fracture (BUG-2388..2391, ac28cb13e).
- [x] Random artifact odds -> master's (BUG-2387, a6e658347).
- [x] #turn undead ladder -> master's (s204, BUG-2247, 70b24f8e9).
- [x] Alchemy: enlightenment + levitation -> gain level 1 in 3
  (s196, BUG-2215, cd0d26a56).
- [x] Dragon-scale secondary properties -> EvilHack's table
  (BUG-1772, BUG-1782..1794).
- [x] Medusa vs an applied mirror from any distance -> master's
  distance and averting guards (s124, BUG-1854, 9bce89466).
- [x] Excalibur dip: any lawful 1 in 30 -> Knight-only 1 in 6 with
  confiscation (2026-08-07, 9338a9515).
- [x] Teleport scroll with the Amulet or in the Wizard's Tower: 5.0's
  2 in 3 success dropped; always fails with disorientation.
- [x] #jump into iron bars: "You crash into some iron bars" -> a
  refusal (s138, BUG-1918, a8f62f78c).
- [x] `poisoned()`: 5.0's survivable deadly poison (survive with more
  HP than 6 + 4d6, at -1 Con and -3 to the poison's stat) -> master's
  instant death below 35% poison resistance, the toxic arm draining no
  stat (partial-intrinsics port; ratified 2026-10-18).
- [x] Strength damage bonus on two-handed and dual hits: 5.0's 3/2 of
  the bonus for melee with a two-hander held in both hands (no giants,
  no applied polearms), and 3/4 on each hit of a two-weapon attack or
  double punch -> master's 1.5x for any blow struck with a two-handed
  weapon, and the full bonus on each hit (s252, BUG-2520 / 2521,
  8814d900a).
- [x] Wish spreading, the wand half: 5.0's (0:1) random wand of
  wishing and its one-charge recharge ("glows feebly") -> master's
  (1:1..3), already recharged outside explore mode, so charging one
  blows it up; a never-recharged one recharges to 1..3 (ruled s285,
  BUG-2796, 059962eb1; upstream 615edb1c7; master 80ab6d9dc). The other
  wish-spreading parts (Vlad's throne, the Amulet wish) went earlier.
- [x] Monster hit points: 5.0's flat d8 per level and its adult-dragon
  formula (4 per level + d(level,4), 8 per level in the endgame) ->
  master's dice by size, Rider and demon lord bonuses (s315,
  BUG-2952..2957). Kept from 5.0: the raised golem values and the
  2 HP floor (see section 3).
- [x] Force bolt: 5.0's flat 2d12, to-hit base 10 and 5 Pw ->
  master's attack-spell-skill scale: 2d6 / 2d12 / 2d16 / 2d20, base
  8 / 10 / 20 / 30, and a Wizard pays 1 / 1 / 2 / 3 Pw (s316,
  BUG-2958/2959, c1bec4a3c; listed by the 2026-11-11 audit). An
  unskilled caster hits for less than on 5.0.
- [x] Bare-handed stagger: 5.0's `!bigmonst` gate -> master's proc
  on any size, half the time a "knocks ... senseless" confusion in
  place of the stagger (s316, BUG-2965, c1bec4a3c; listed by the
  2026-11-11 audit). `mhurtle()` still refuses to move a huge
  monster.
- [x] Iron bars: 5.0's water elemental "eats through the iron bars"
  (by its rust attack) and so does its xorn (as a metal eater) ->
  both go through and the bars stay standing, monster or hero
  form; the water bodies pass undiggable bars too (s321,
  BUG-2995..2997, e18f68c13; master 6529613d9 for the monster
  water bodies, rulings for the rest). A rock mole's bar eating is
  kept (see section 3).
- [x] Passive counter-effects: vanilla's "only when a weapon, glove
  or boot object was passed in" test on the fire, acid, rust and
  corrosion arms -> master's: a bare-handed hit reaches the worn
  gloves (s321, BUG-2992, e18f68c13; master 501e23bec).
- [x] Ray to-hit: vanilla's `3 - chance < AC + bonus` roll, with
  no bonus at all for a wand -> master's `-chance` roll, so a ray
  lands up to 15 points more often against AC 2 or better, the
  ones aimed at the hero included (AC 0: 80% -> 95%, AC -20: 30%
  -> 43%; no change at AC 3 or worse); the hero's ray spells add
  their level up to 15; and by ruling the hero's own wand rays and
  horn blasts get that level term, the Dex adjustment and +3 with
  empty hands, the arm master wrote but never reached for a wand
  (s322, BUG-3000..3002, 4542b9892; master cd5c574bb).
- [x] Engulfing over a hazard: any walking engulfer swallowed a hero
  who was above water, lava or open air and died there ("sinks as
  water rushes in and flushes you out") -> master's test that the
  engulfer can stand on the hero's square; by ruling only a stunned
  or confused engulfer still lunges, and its lava death, silent in
  5.0, has a line (s323, BUG-3004 / 3019, 34b506fea; master
  3a0948837 + 23a2594bf).
- [x] A phasing hero stayed inside an engulfer -> master's exit, on
  the engulfer's turn or the hero's own
  (s323, BUG-3005..3007, 34b506fea; master b45cbbcad +
  4c52be0cd).
- [x] A rock-thrower in Sokoban stepped over a stuck boulder and took
  the Luck penalty unasked -> master's "could maneuver over it" and
  "Do so?" (s323, BUG-3010, 34b506fea; master f03ad32ba).
- [x] A disturbed grave's zombie or mummy rolled for peacefulness ->
  always hostile (s323, BUG-3011, 34b506fea; master c1cd71c2f).
- [x] A slowing blow slowed on every hit -> master's one time in
  three, gazes excepted, for the hero's blows and between monsters
  (s323, BUG-3017, 34b506fea; master 641e9bd79).
- [x] Invoked ammo copied only the launcher's bless knowledge ->
  master's known / dknown / rknown too
  (s323, BUG-3015, 34b506fea; master 042a0c296).
- [x] A blessed scroll of genocide wiped every genocidable species
  of the class -> two species plus the hero's own role or race.
  Master's limit; by ruling the two are picked at even odds, not by
  master's loop, which favored the end of the class and could never
  pick some species (s324, BUG-3042, f4d0860b9; master's
  SporkHack-derived do_class_genocide()).
- [x] Digging down turned an ordinary trap into a pit with no word,
  and blew up a magical one with no "Boom!" -> master's rule on
  every level: the trap is destroyed first ("You destroy the dart
  trap with your pick-axe."), a falling rock trap excepted
  (s324, BUG-3033, f4d0860b9; master 5666a8ef3 + 7ba42c5be +
  7f328d740).
- [x] A hard helmet stopped a scroll of earth's boulder at 2 damage
  and the falling iron ball at 3 -> master's 2 off ("Your helmet
  only slightly protects you."); rocks keep the cap. A helmet was
  ignored under a cursed scroll -> it counts
  (s324, BUG-3029..3031, f4d0860b9; master eb73649b9 +
  b9fe36fbc).
- [x] Joust bonus a flat 2d10 -> master's 2d(5 + level / 3); a
  ghost or vortex could be jousted and knocked back -> an ordinary
  hit (s324, BUG-3024 / 3025, f4d0860b9; master 6667cac6c +
  daf50114f).
- [x] A magic whistle was identified only when a pet was seen to
  answer -> identified when blown, like the pea whistle, with "This
  must be a magic whistle." (s324, BUG-3037, f4d0860b9; master
  65cd648f1).
- [x] A magical eye corpse gave 200 turns of hallucination (its stun
  gaze) -> master's Luck shift and nothing else
  (s324, BUG-3027, f4d0860b9; master ae8bf25d6 + 7f3663163).
- [x] Jumping cost a Knight rnd(25) nutrition like everyone else ->
  master's rnd(10) (s324, BUG-3021, f4d0860b9; master
  0085e9669).
- [x] A corpse about to rise as a zombie eaten as five points more
  rotten, and refused by every pet (upstream acfb7ea97, arrived with
  catch-up 3) -> EvilHack's own rule, harsher than both trees by
  ruling: the corpse of a zombie's kill carries the zombie sickness
  for as long as it has its rising timer, as a real zombie's corpse
  does; pets refuse it unless they resist sickness; a vampire is told
  "The blood in this corpse is tainted!"; it does not stack, so the
  timer cannot be shed that way (an ice box still stops it)
  (s325, BUG-3050 / 3055, f4ba98734).
- [x] A monster was turned to stone at once by another monster's
  stoning touch, bite or corpse weapon, by the hero's cockatrice
  corpse or egg, by a thrown egg and by a petrifying meal -> master's
  five-turn countdown with its message ladder, cures and hero credit.
  Medusa's gaze, a polymorphed hero's own touch and a monster
  touching a petrifier stay instant. By ruling "<Monster> is slowing
  down." shows once, at the hit
  (s326, BUG-3056..3061, 3f3dbf7ed; master 2f58ace93 and the
  commits that shaped it since).
- [x] A beholder's or Tal'Gath's gaze turned a monster to stone on
  every gaze -> master's 1 time in 3, and gradual
  (s326, BUG-3058, 3f3dbf7ed; master 903bff9ee + 1c704a9a7).
- [x] A monster's wielded petrifying corpse also did weapon damage,
  with the gauntlet, artifact, rust and poison riders -> master's
  flat 1 (s326, BUG-3057, 3f3dbf7ed).
- [x] A submerged hero fighting across the waterline -> master's
  rule: no attack on a monster on dry land (the hero climbs out
  toward it), one above the water is swum under, one over lava cannot
  be reached (the last by ruling; master steps into the lava)
  (s329, BUG-3098, 3099, 8bc94ee80; master 442a2ac22 and the
  commits that shaped it since).
- [x] "Wait!  There's something there you can't see!" on every move
  of a submerged hero at an unseen monster on the bank -> once; the
  second move climbs out (ruled s329, BUG-3102, 8bc94ee80;
  beyond master, which repeats it too).
- [x] A kick under water landing half the time -> never
  (s329, BUG-3100, 8bc94ee80; master 95ed3591f).

### 2.4 Text and logs

- [x] Wish livelog `, got "<object>"` tail (s231, BUG-2385, 5e9136d4a;
  5.0's "declined to make a wish" line KEPT and tagged).
- [x] "<unique> has been killed" livelog for kills by neither hero
  nor pet (692008bd7).
- [x] "invisible " before a unique in death reasons (BUG-1857,
  428abc0f0; master 113aae435).
- [x] Unseen explosion death reason: bare "explosion" -> master's full
  name (s177, BUG-2118).
- [x] Melee verbs: the delayed m-v-m "hits" fallback; master's verb
  tables (s132, BUG-1879/1880).
- [x] Boots refusal "You have too many hooves..." -> master's "not
  shaped correctly" (BUG-1874, 87e1b5dea).
- [x] Player-monster rank title printed twice in death messages;
  vanilla Adam/Eve names -> master's racial lists (BUG-1555,
  41970ffcd).
- [x] Vlad rumor "near the mazes" -> master's "in hell"; oracle
  dragon-scale passage -> master's (s123, 29528f0f9).
- [x] Blind confused enchant weapon "Your weapon feels warm" ->
  names the item (s222, BUG-2325/2326, add5e9b91).
- [x] xlogfile conduct tokens -> master's names (2026-07-17).
- [x] -nethackrc / NETHACKOPTIONS / .nethackrc -> EvilHack names
  (2026-07-12).
- [x] "Boom!" before every explosion the hero sees (s258, BUG-2560,
  81f2eaeef; master never printed it; an unseen blast still reads
  "You hear a blast.").
- [x] "You die..." on total digestion: a hero digested in their own
  form gets the engulfer's belch and the death reason "digested by
  <monster>", as master's direct done(); other engulf deaths keep the
  line (s319, BUG-2985; master 848be16a0).
- [x] Record file version test: 5.0's "older than 3.3" layout test ->
  master's EvilHack-version test, so entries keep role, race, gender
  and alignment codes (s319, BUG-2982; master 8817d4e8a).
- [x] `not_fully_identified()`: 5.0's "only armor, weapons and
  weapon-tools need rknown" -> master's "everything damageable but
  scrolls, spellbooks and potions" (s319, BUG-2979; master bfefc9a34).
- [x] An angry shopkeeper's fixed "scum" in a price quote -> master's
  eight-way insult, also used by a shopkeeper whose race the hero's
  race hates or who faces an Infidel (s318, BUG-2974, 4f949ab6a;
  listed by the 2026-11-11 audit).
- [x] "There doesn't seem to be any sort of lock here." -> master's
  "...any sort of pickable lock here." (s320, BUG-2987, da9110cab;
  master 7acedb22e; listed by the 2026-11-11 audit).
- [x] "<Golem> falls to pieces!" from the hero's rust or rot hit,
  printed even for a golem the hero cannot see ("It falls to
  pieces!") -> shown only when the hero sees it (s321, BUG-2999,
  e18f68c13; master ba46f54c8).
- [x] Class genocide's "genocided class <c>" livelog and its one
  refusal line per species -> master's livelog naming the species
  and three grouped lines after the wipes
  (s324, BUG-3042, f4d0860b9; master 979fac14c + 6e7694b0e).
  5.0's prompt, `?` list and
  "declined to perform class genocide" line KEPT.
- [x] "<Demon> appears in a cloud of smoke!" -> "cloud of
  brimstone", and master's stench line for a blind hero. By ruling
  5.0's frame stays: one line per summon, keyed on the arriving
  monster, flash of light / vapor / flame for the others
  (s324, BUG-3040, f4d0860b9; master e197133c0).
- [x] Monster class texts "umber hulk" and "human or elf" -> master's
  "umber hulk or shambling horror" and "human or other humanoid",
  Guidebook table included (s324, BUG-3047, f4d0860b9).
- [x] "You have turned to stone." as the fourth line of the hero's
  stoning countdown -> master's "You have almost completely turned to
  stone." (s324, BUG-3043, f4d0860b9; master dacf253d8).
- [x] "You die from your illness." for food poisoning -> master's
  "sickness" (s324, BUG-3044, f4d0860b9; master 65ed594dd).
- [x] "<Monster> is slowing down." just before "<Monster> turns to
  stone." on an instant petrification, and again when a monster ate
  or drank its cure -> gone, as on master; the line belongs to the
  start of the countdown
  (s326, BUG-3062 / 3068, 3f3dbf7ed; master 17488cfcc).

### 2.5 Levels and content

- [x] Sokoban: rolling-boulder traps and spawn-exclusion zones
  stripped, all 24 levels no-flip, the prizes' "not-cursed" dropped
  (2026-08-02).
- [x] Castle wand chest: the extra potion of gain level (2026-08-01;
  upstream a8800a9ac).
- [x] Gehennom (2026-08-05): `hell_tweaks` stripped from Orcus and
  hella-2; Valley, hella-1/2/3, Sanctum and Orcus Town to master's
  shape; dungeon (20,5) -> (16,5).
- [x] Vlad's Tower: 5.0's 3 levels -> master's 4-level climb
  (2026-08-02).
- [x] Wizard's Tower: in-Gehennom levels -> master's portal dungeon;
  fakewiz1/2 deleted (2026-08-05).
- [x] Elemental planes order randomized again (s47).
- [x] Mines: minend luckstones can be cursed; minetn-6 Bustling Town
  -> Zoo Town; Orcish Town and minend-2 regions (2026-08-02).
- [x] Ludios portal's 2 in 3 deferral (2026-08-01).
- [x] Quest kits: Arthur's Excalibur / plate / warhorse and
  Twoflower's upstream kit; Barbarian and Monk siege floodfills ->
  master's fixed coordinates.
- [x] 5.0-only monster names and rank tiers not adopted; Caveman
  nemesis Chromatic Dragon -> Annam (BUG-1367).
- [x] Player-monster gear switch (including dragon scale mail) ->
  master's kit (BUG-1552..1557).
- [x] Fountain gush / swamp make PUDDLE, not 5.0's POOL.
- [x] Summon nasties: 5.0's 44-entry `nasties[]` -> master's curated
  50 (ruled 2026-10-18 as planned item 1.6), with the Double Trouble
  clone's `wizapp[]` disguises -> master's 12, the other half of
  master 04518bab7 (s259, BUG-2569 / 2570, 8b03f5d2c).
- [x] Monk locate level: the lava formation drawn as water three
  games in four (upstream acb0988fd, arrived with catch-up 3) ->
  always lava. The second formation and the random pick KEPT
  (s325, ruled, f4ba98734).

## 3. Reversed (5.0 kept after a removal was ruled or proposed)

- [r] Charged-ring shock recharge: dropped (BUG-1951), then kept and
  made symmetric for monsters (BUG-1956, 5e2c0a3f2).
- [r] `losexp()` max HP floor: master's 1 ruled, then reversed; 5.0's
  max(XL, 10) and the fire-trap coupling (d53c1a7a6) kept (s198).
- [r] Human-form lycanthrope drain resistance kept (BUG-1899; master's
  loss was a regression).
- Ended up removed after a first keep: polearm stand-off (kept, then
  BUG-1949), paranoid trap (kept s184, removed s237), the mysterious
  force decay term (kept 2026-07-23, removed s140).
- [r] Gold weight minimum: 1..49 coins weigh 1 aum (5.0 90aded364)
  kept over master's 0 (ruled 2026-10-21, s249); extended to
  encumbrance in s250 (2.1).
- [r] Strength to-hit at exactly 18/50: abon()'s +2 (5.0, so gnomes and
  orcs gain a point at their cap) kept over master's / vanilla 3.6's +1
  (ruled 2026-10-23, s252).
- [r] Gazes on a helpless hero: 5.0's gazemu() cancels every gaze
  while the hero is asleep, fainted or unconscious; kept over
  master's form, where a sleeper is stoned or burned (ruled
  2026-11-09, s314: closed eyes protect as blindness does).
- [r] Golem hit points and the 2 HP floor: the maintainer kept 5.0's
  raised golem values (gold 60, clay 70, stone 100, glass 80, iron
  120; master 40 / 50 / 60 / 60 / 80) and 5.0's "every die rolled a 1
  gets +1" floor when master's hit dice were ported (s315,
  2026-11-09).
- [r] Rock moles eat iron bars: 5.0's metal-eater rule kept for the
  rock mole, monster and hero form, over master's "bars stop a rock
  mole"; undiggable bars now stop it (they did not on 5.0). A
  xorn-form hero's deliberate bar meal (the eat command while
  standing on bars) is kept too (ruled 2026-11-11, s321).
- [r] Bare-handed training on a minimum roll: 5.0 trains nothing
  when a bare-handed or martial arts hit rolls its minimum (one hit
  in four for martial arts, one in two bare-handed); kept over
  master's every-hit training, the damage bonus being given either
  way (ruled 2026-11-11, s323).
- [r] A dazed engulfer over a hazard: 5.0 lets any walking engulfer
  swallow a hero above water, lava or open air and die there; kept
  for a stunned or confused engulfer only, master's refusal for the
  rest (ruled 2026-11-11, s323; see 2.3).
- [r] Surviving fatal food poisoning: 5.0's Con-in-100 save
  (63d3d7b68) kept, though master has none and removal was
  recommended; its cost raised from Con -1 to Con -3 by the same
  ruling (2026-11-11, s324, BUG-3044).
- [r] The whip flicked down over water or lava: 5.0's "You cause a
  small splash." kept over master's grab from the liquid and the
  whip's lava damage (ruled s329, against the recommendation).

## 4. Open

None. The last two were ruled 2026-10-18: `nasties[]` became 1.6
(landed s259, now under 2.5), and `poisoned()`'s survivor drains were
ratified (listed under 2.3).

## 5. Not counted: vanilla rules replaced by EvilHack's own systems

These remove vanilla behavior too, but they are master's systems
coming back, not 5.0 additions taken out. Examples: weapon-skill and
hero to-hit tables (BUG-1660..1671), spawn pacing (BUG-1691..1696),
finite trap ammo, erosion destroying armor (BUG-1410/1414), hard
helmets (BUG-1684), monster poisoned weapons (BUG-1680/1681), launcher
ranges (BUG-1685), the XP curve and other numeric constants
(BUG-1714, ea8a75559), monster spell cooldowns (BUG-1642), Wizard's
`intervene()` (BUG-1558..1561), role intrinsic tables (BUG-1760; Arc
and Val keep 5.0's by ruling), life-saving HP (a maintainer-designed
Con curve, s198), magic fountains and restore ability (BUG-2228..2244),
the Astral Red Horse in place of the guardian Angel and its Conflict
Angels (BUG-2448), master's random-saddle odds (BUG-2451), the
sparkle frame count (BUG-2487), the inventory weight / slots header
(BUG-2490), the strength damage table (BUG-2520).
