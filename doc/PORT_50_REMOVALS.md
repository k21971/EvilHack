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

### 1.6 [ ] `nasties[]` -> master's list

- **Branch today:** 5.0's 44-entry list with EvilHack renames. On
  EvilHack alignments it is 19 neutral, 20 chaotic or unaligned and 5
  lawful, so a lawful caster stops on only 24 of 44 picks and
  summons more than the table intends.
- **Master:** a curated 50-entry list (EvilHack-authored: the shadow
  dragon 9e0097194, the devils out with de169194a), 22 neutral, 15
  chaotic, 13 lawful.
- **Ruled 2026-10-18:** master's list as one unit, with `nasty()`'s
  comment recomputed; the tracker's nasties[] content reconcile row is
  the slice. 5.0's picking rules (`pick_nasty()`'s difficulty cap and
  lesser-form substitution) are a separate difference outside it.

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

### 2.3 Combat and magic rules -> master's

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

## 4. Open

None. The last two were ruled 2026-10-18: `nasties[]` became 1.6,
and `poisoned()`'s survivor drains were ratified (listed under 2.3).

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
(BUG-2490).
