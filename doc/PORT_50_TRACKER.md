# EvilHack -> NetHack 5.0 Port Tracker

**This is the single source of truth for port state - keep it
updated every session.** Lives on the `evilhack-50` branch (moved
here from /root/nh50-port-preview/ at Phase 0 step 3). The plan
lives at `/root/.claude/plans/feature-evilhack-50-migration.md`;
READ PLAN SECTION 11 (implementing-model rules) BEFORE EVERY PHASE.

Skeleton pre-populated 2026-07-02 (Fable 5) from the measured tree
delta, FEATURES_ARCHIVE.md, and the converter-tooling runs. Statuses:
`[ ]` todo, `[~]` in progress (add a note!), `[x]` done+build-green,
`[d]` dropped (superseded upstream - note why).

## Dashboard

- Current phase: PHASE 2 IN PROGRESS (started 2026-07-13; lifecycle
  detectors slice ported first per plan - see Phase 2 checklist
  notes; landed e265bd879). Phase 1 COMPLETE 2026-07-13 (gate
  PASSED) except roles-side follow-ups logged in its section
- Tooling 2026-08-01 (after the slice below): the Phase 3 converter
  GATE row is ALSO closed - ~/des_to_lua.pl gained the terrain
  remap (w -> p) + fatal out-of-vocabulary gate, full-corpus
  regression and a live engine leg PASS, crib sheet documented.
  Failure-mode corrected on the row (silent erasure into the
  solidfill, not walls; Healer's puddles would have become
  DROWNING POOL). PHASE 3 CONVERSION IS UNBLOCKED - both gates
  (LEV_LIMIT + converter) are now down; next Phase 3 step is the
  first delta-diff level batch
- Last fix: 2026-08-01, the LEV_LIMIT slice - THE PHASE 3 GATE ROW
  IS CLOSED. Three commits: BUG-1453 (init_dungeon_levels/
  init_dungeon_branches bound their lua_len counts BEFORE the fill
  loops - the post-loop checks let an oversized dungeon.lua corrupt
  proto_dungeon and the init_dungeons() stack before panicking;
  upstream-inherited from the 5.0 Lua rewrite, byte-identical in
  both upstream trees, artifact ~/nh50-dungeon-init-guards.patch),
  BUG-1454 (level_range() regained master's chain >= LEV_LIMIT
  bound - EvilHack hardening vanilla lacks, port gap), and the
  LEV_LIMIT 50 -> 99 bump BY SESSION RULING now rather than with
  the Phase 3 content (save-neutral, proven by a live cross-build
  restore: a save written at 50 restored byte-identical at 99).
  Battery ALL LEGS PASS: end-to-end padded-nhdat panics with write
  breakpoints unhit (loop counters still uninitialized at the
  panic), "max 98" tracking the bumped constant, all 9 dungeons +
  Mines/Sokoban traversals + same-build round trip clean; real
  paniclog never existed (deliberate panic lines confined to a
  throwaway playground copy). Row corrections recorded: SIX
  master-only dungeons (the Wizard's Tower hoist was uncounted),
  post-port ~49 prototypes/15 dungeons; MAXDUNGEON 16 is the real
  binding constraint (exactly 15/15 post-port, kept at 16 by
  ruling, master parity). A LOW authoring-ergonomics row minted
  (the Lua loader lost dgn_comp.y's whole compile-time validation
  layer)
- Previous fix: 2026-07-31 LATE NIGHT, the opportunistic REVIEW-LOW
  residue - five commits draining the pipeline's final five rows,
  battery 4/4 legs PASS one human Rogue XL30 game, paniclog never
  existed: BUG-1450 69283d87f (incr/decr_resistance start from 0
  on a no-HAVEPARTIAL word - a timed grant's turn count no longer
  folds into, or gets promoted to, a permanent partial; a live
  corpse meal during a 99-turn grant produced a clean 50% partial),
  BUG-1451 43b842422 (doicetrap routes FROMFORM to the
  vulnerability overlay per the innate-only ruling and claims
  "alarmingly cooler" only on a real drop - five logged no-op
  strips stayed silent; staging lore: white dragons FLY OVER ice
  traps, winter wolf is the cold-res form), BUG-1449 f0b696345
  (the Vulnerable_* macros mask FROMFORM out of the word leg - a
  stale poly-time snapshot can no longer bypass the BUG-1448
  override; the ice trap's timed word still bites), BUG-1452
  e5ea1ae4f (mount_monster's denied path unmakemon()s the mount -
  extinct-warg leg: born unchanged, no stray), + the replmon
  no-steed-links invariant comment 784fc50d3. All four
  master-inherited frozen-unfixed; the 2026-07 review pipeline is
  FULLY DRAINED
- Earlier fixes this window (detail in the archive + ticked rows):
  2026-07-31 NIGHT BUG-1445..1448 (REVIEW-MEDIUM slice, four
  commits; battery 6/6 PASS one chaotic Rogue game; update_monsteed
  at both open-coded swaps, slippery-ice how_resistant==100 gate,
  lava-grant word-safety by ruling, vulnerable_to
  u_full_resist_raw override; minted the Vulnerable_* macro
  word-arm row the next slice closed);
  2026-07-31 LATE EVENING BUG-1443 d7bd8421e + BUG-1444 7fc3b5a86
  (REVIEW-HIGH slice: ok_to_obliterate excludes both mounted-pair
  halves + elemental_clog Astral no-candidate arm -> m_into_limbo
  per ruling; battery 4/4 PASS on real Astral; both
  master-inherited, the Astral no-op also upstream-shared);
  2026-07-31 EVENING BUG-1442 0f4f2c898 (mon_arrive clears
  MON_ENDGAME_MIGR; upstream-inherited, artifact
  ~/nh50-mon-endgame-migr.patch awaiting send) + the reviewer
  re-run row closed (zero re-reports; 1 HIGH + 4 MEDIUM + 4 LOW
  NEW findings minted as REVIEW-* rows);
  2026-07-31 REVIEW-LOW batch BUG-1427..1441 (17 commits - 15 fixes
  + cleanup + tracker; battery 9/9 PASS one lawful human Priest
  game; premise corrections on 7 of 22 rows; the battery caught a
  real crash in the first dogmove draft - the AT_ANY/AD_ANY
  lookup-twin lesson in the BUG-1440 entry);
  2026-07-31 NIGHT BUG-1422..1426 (regressions batch, five commits;
  battery 5/5 PASS; items 16/17/18 reclassified master-inherited;
  wrong-pool-under-Upolyd + two upstream-reportable rows minted);
  2026-07-31 LATER BUG-1416..1421 (items 8-12 batch, six commits;
  battery 7/7 PASS; SUPPRESS_RIDER + drow-poison + taint parity;
  two NEW pre-existing poison findings minted as rows);
  2026-07-31 BUG-1409..1415 (port-gap batch, items 4/5/6, seven
  commits; battery 8/8 core legs PASS one Barbarian XL30 game; six
  rows minted incl. the D1-D4 erosion deferrals and the
  can_levitate consumer sweep upgraded to MEDIUM on a live repro);
  2026-07-30 NIGHT BUG-1407 3d94580ad + BUG-1408 bd45c47a6
  (partial-resistance consumers slice, items 20+21; battery 4/4
  PASS - gremlin farming dead, crowned donor intact, lich arm was
  the live half with the trap arms proven INERT);
  2026-07-30 BUG-1400 67547eee2 + BUG-1401 a09223367 (passive-arms
  HIGHs: passive_obj AD_DISN + void->int with thrower reactions,
  passive AD_CNCL; battery 8/8 PASS) and BUG-1402 a202b734f +
  BUG-1403 d56763327 (bag-of-devouring HIGHs; battery PASS);
  2026-07-30 EVENING BUG-1404..1406 (mhitm/steed-guard slice,
  battery 4/4 PASS)
- Last slice: 2026-07-29 GOGGLES - closed the row the objnam slice minted,
  in two commits: the equipment plumbing (18 sites, one of which a
  master-anchored census structurally could not find - iactions.c is
  5.0-only), then See_underwater and its ~15 consumers. Goggles do NOT
  confer Breathless, as first claimed; they are a source of master's
  See_underwater, which is EvilHack-only and was absent here entirely.
  Battery found a real port bug - removing eyewear while submerged left a
  stale map - fixed at the transition in Blindf_on/off rather than by
  master's per-turn docrt(). Note this changed underwater play for every
  tortle, not just goggles wearers. Four rows minted
- Previous slice: 2026-07-28 LATER, objnam.c RE-PORT - the three wish/naming
  oddities the combat/AC battery reported, traced to Phase 1 leaving
  src/objnam.c as vanilla 5.0. Three commits: the dragon-scale-mail
  redirect + chromatic gate, the plural handling, the wish tables; plus a
  fourth for the eyewear property the object swap dropped, which had left
  blindfolds and towels unable to blind. Sweep found worse than the
  reports: "hat" wishes returned a random object of any class. Reviewer
  caught a new bug before it shipped - master's drow aliases are
  order-dependent and 5.0 inverted the parse order. Both batteries PASS,
  paniclog clean. Goggles behavior minted as its own row
- Previous slice: 2026-07-28 COMBAT DAMAGE / AC REMAINDER - the five rows
  the glass/blocker slice minted, closed in four commits: dmgval's
  four body-material arms (plus master's final negative clamp, which
  the branch lacked and the new penalties made mandatory), the
  find_ac shield-skill ladder with its Knight/Valkyrie skill_init
  bump, thitu's AC_VALUE hit gate, rustm's EF_DESTROY + AD_DCAY
  parity with FOUR UAF guards, and the glass-piercer rework. Battery
  20/20 PASS, no crashes. Three new rows minted: the Dex-to-AC ladder
  (unlogged adoption casualty), is_hard-vs-hard_helmet tree-wide, and
  the monster off-hand damage-weapon mismatch
- (was: PHASE 1 IN PROGRESS (content data LANDED
  2026-07-12: 561 monsters + 566 objects build clean-rebuild green;
  two commits pending maintainer: staged slice A scaffolding
  defines + working-tree content swap, message files in scratchpad
  msg-phase1-slice-a.txt / msg-phase1-content.txt; BOTH LANDED
  cc0bf8a13 + ec925f41a). Slice 3 (artilist + Role/Race port)
  LANDED 4f3d4bc7c; u_init LANDED ec155733e; approved re-adds
  (chain lightning P_ATTACK_SPELL + drain/shock shields) staged,
  message /tmp/p4.txt. NEXT = persistent-struct enumeration
  (obj.h/monst.h/prop.h/you.h/flag.h/mextra.h/context.h), glyph
  audit, Phase 1 boot gate (ask maintainer before launching))
- Adoption commit (nh50/master @ merge):
  74d1133c229a0fe17c6b05d4d4084937cb2631ca (2026-07-12); adoption
  merge commit on evilhack-50: 6160262a4 (parents c444f6a3a +
  74d1133c2; tree verified byte-identical to nh50/master)
- Fork-point tag on master: evilhack-50-fork-point @
  c444f6a3ab1e9f16d0676961dba86f628e91c6ba (= master tip at
  adoption; re-derive queue starts EMPTY)
- Backup mechanism (plan §7): dated git bundle at session end ->
  /home/build/wizmode_builds/evilhack-50-backups/ (branch stays
  local until playable per plan §9 decision 5; maintainer pulls
  bundles into the offsite ritual). Prep artifacts (tooling + this
  tracker + exemplars + the plan file itself) offsite-backed-up by
  maintainer 2026-07-03 - refresh that copy whenever they change.
- Last session: 2026-07-12 LATER (PHASE 1 content-data session:
  converters regenerated at the adoption trees - monsters 564->561
  NAMS-merged 0 PORT-TODOs round-trip OK, objects 566 round-trip
  OK; monster macro pre-draft + 561 entries + objects pre-draft +
  566 entries spliced; MH_ race-bit migration, PM_/onames rename
  sweeps, trap-type *_TRAP_SET renames, make_corpse exhaustive
  switch regenerated, dragon-gear macros + scale-mail carve-out;
  full clean rebuild ZERO errors, ONE known warning role.c:1741 =
  the pending Role-struct port; see Phase 1 checklist notes and
  Decisions log 2026-07-12 entries for everything decided)
- Previous session: 2026-07-12 (PHASE 0 COMPLETE in one session:
  interim delta 219cfb687..74d1133c2 surveyed - 42 upstream
  commits, all benign, §4 inventory unchanged; baseline build
  re-proven at 74d1133c2; tree-adoption merge 6160262a4; fork
  point tagged; tracker on branch; identity pass code+renames
  build-green and strings-verified; CI linux job re-added;
  glyphs.c->customcolor.c decided; first bundle cut+verified.
  Commit flow: git commit is deny-listed for the agent - stage +
  write message file, maintainer runs `! git commit -F <file>`.
  Found+fixed upstream shk.c impossible() format typo, see
  Upstream-reportable table. SAME DAY: Phase 0.5 pilot slice
  COMPLETE - all six items, see Phase 0.5 section for recipes,
  gotchas, and effort calibration; branch test sessions use
  --no-evilhackrc and install via `make install SHELLDIR=`)
- Baseline builds: NetHack50 clean @ 219cfb687 (2026-07-02) with
  BOTH hints/linux-debug and hints/hardfought - zero warnings;
  RE-PROVEN @ 74d1133c2 (2026-07-12, make spotless,
  hints/linux-debug, zero warnings, src/nethack 12365040 bytes)

## Phase 0 - Scaffolding

- [x] Re-verify baseline build at the ACTUAL adoption commit
      (74d1133c2, 2026-07-12, zero warnings; interim delta from
      219cfb687 surveyed: 42 commits, benign - u.ustuck save/restore
      fixes, glyph lookup optimization, herecmdmenu buttons,
      MS-DOS/Windows, Guidebook regen; no §4 inventory changes)
- [x] Create evilhack-50 via tree-adoption merge (plan §3 commands;
      merge commit 6160262a4, tree == nh50/master, both parents
      reachable; note: git reset --hard was permission-denied, used
      git branch -f from master + checkout instead - same result)
- [x] Tag master fork point; record both hashes above
- [x] Adopted tree builds GREEN in the EvilHack worktree
      (2026-07-12, make all zero warnings after setup.sh; gotchas:
      sys/unix/setup.sh must be run FROM sys/unix/ - hints path is
      relative; lib/lua-5.4.8 + lib/lua are fetched artifacts, not
      tracked - copied from NetHack50/lib instead of make fetch-lua;
      stale 3.6 build leftovers in the worktree are untracked/
      ignored and harmless - do NOT git clean, doc/claude lives
      there)
- [x] Move this tracker onto the branch - now doc/PORT_50_TRACKER.md
      (relocated out of doc/claude/ 2026-07-25 so the whole notes
      directory could be gitignored; it was the only tracked file
      under it)
- [x] Resolve src/glyphs.c name collision - DECIDED 2026-07-12:
      EvilHack's customcolor registry ports as src/customcolor.c at
      Phase 4; upstream src/glyphs.c keeps its name (see Decisions)
- [x] Identity pass (2026-07-12, build-green + strings-verified in
      binary: banner, 1.0.0 WIP version, evilhack50dir, .evilhackrc,
      EVILHACKOPTIONS, -evilhackrc/-no-evilhackrc flags,
      GAME=evilhack in Makefile.top/.src/.doc/.utl, dat/help +
      dat/usagehlp text; file renames doc/evilhack.6/.txt (.txt
      regenerated via doc make, never hand-edit),
      sys/windows/evilhackrc.template, win/win32 EvilHackW.rc/
      EvilHackW.exe.manifest/evilhack.rc, refs swept in Files +
      msdos/vms/windows build files + cross-post/macOS hints).
      Left as-is on purpose (mirrors master): NETHACKDIR env var,
      "NetHack Defaults"/"NetHack.cnf" exotic-platform names,
      X11 "NetHack" title/class + win/X11/nethack.rc file, rumors
      fallback strings, man page .TH NETHACK header. Deferred:
      Windows package output names (NetHack.exe/NetHack.txt dests,
      afternethack.proj file name) to the Phase 6 Windows-port
      validation; sys/amiga/txt-info.uu embedded icon name
      (uuencoded binary); Guidebook .nethackrc.template mentions
      (Guidebook gets wholesale re-derived later)
- [x] CI: re-add adapted .github/workflows (2026-07-12,
      evilhack-cpp.yml: linux-build job only - setup.sh from
      sys/unix + make fetch-lua + make all; install/artifact steps
      and macOS/Windows jobs deferred to Phase 6, need a
      runner-writable .gha hints re-derive)
- [x] Set up + test the backup ritual (2026-07-12: dated bundle
      `git bundle create /home/build/wizmode_builds/
      evilhack-50-backups/evilhack-50_YYYY-MM-DD.bundle
      --branches --tags`, verified complete-history; refresh at
      every session end and after big landings; maintainer pulls
      the bundle dir into the offsite ritual)

## Phase 0.5 - Pilot vertical slice (one of everything, end-to-end)

- [x] One EvilHack-only file compiled in: telekinesis.c
      (2026-07-12, HERO SIDE - dotelekinesis + tk_* helpers +
      can_use/tk_skill/tk_range/tk_pw_cost, fully converted to 5.0
      idiom: staticfn/ANSI, gy.youmonst, svc.context, svl.level
      .objects, cg.zeroany, disp.botl, 5.0 menu API, ismnum()
      instead of the safe_* macros, IS_TREE only). Port-alongs:
      damage_mon core -> mon.c (vulnerability/HPHDAM/emsp/display
      hooks = TODO-EVILHACK), move_chain_to -> ball.c, monst.h
      mtk_attk field, materialnm[] UPDATED IN PLACE in decl.c -
      5.0 already carried a vanilla-ordered copy, so the enum swap
      HAD to be paired with this array (silent name-shift class!).
      extern.h decls, Makefile.src + windows/vms/msdos/vcxproj/
      Files parity entries. DEFERRED to Phase 2 monster-AI (do NOT
      forget): m_dotelekinesis + m_tk_push_obj + mfind_target ->
      mthrowu.c + its mlined_up + dogmove.c acceptable_pet_target +
      un-static mm_aggression + mtk_attk decay ticker + #tk command
      wiring + set_pet_killer credit + illithid-race gates
      (Race_if conjuncts marked TODO-EVILHACK in the file).
      [CONSUMED 2026-07-16 by the Phase 2 telekinesis entry - all
      wired except set_pet_killer credit (kill-attribution item)
      and the mlined_up friendly-fire walk (monster-AI item)]
      CALIBRATION: the "cleanest" EvilHack-only file took ~1
      session incl. 3 pilot dependency slices; budget accordingly
      for Phase 2 files. Master source: git show
      master:src/telekinesis.c (monster half lines ~588-905)
- [x] One monster into monsters.h: alhoon (2026-07-12, hand-added
      in upstream's CURRENT MON format - the 16-arg extension stays
      Phase 1; enum suffix arg -> PM_ALHOON generated via permonst.h
      X-macro re-inclusion of monsters.h, there is NO generated pm.h
      on this tree, census note corrected). Deferred bits marked in
      the entry comment: G_VECNA, MR_PSYCHIC, MH_ILLITHID|MH_UNZOMBIE
      (need Phase 1 extended flag sets). is_mind_flayer() updated to
      master parity (+PM_ALHOON). RECIPE GOTCHA: under NH_STATUS_WIP,
      mon.c make_corpse() has a deliberately exhaustive -Wswitch
      switch over enum monnums - EVERY new monster must be added to
      its case list (upstream dev-mode feature; expect this per
      monster in the Phase 1 bulk add - the converter output should
      be paired with a make_corpse case-list update). Tiles: no
      coupling fired on linux-debug (tty/curses); tile art parity is
      a Phase 4 item. Wizmode genesis smoke test PASSED
      2026-07-12 ("An alhoon appears next to you").
- [x] One object into objects.h: helm of telekinesis (2026-07-12,
      hand-added in upstream's current HELM macro incl. sn suffix
      arg; ADAMANTINE material from the Pilot D enum; CLR_CYAN
      stands in for HI_ADAMANTINE until the HI_ alias family ports).
      Landed WITH its property + skill as one slice: prop.h
      TELEKINESIS=69 (LAST_PROP moved; append-only - uprops saved
      raw), youprop.h H/E/Telekinesis macros, skills.h
      P_TELEKINESIS=38 + P_NUM_SKILLS=39 + P_LAST_H_TO_H moved.
      RECIPE GOTCHA: weapon.c skill_names_indices[P_NUM_SKILLS] is
      POSITIONALLY initialized - a new skill without a PN_ entry +
      odd_skill_names string + indices tail entry compiles clean and
      silently names the skill "no skill" (found by inspection, NOT
      by the compiler; add to the Phase 1 skills checklist).
- [x] One persistent field end-to-end: obj->material (2026-07-12).
      RECIPE PROVEN, simpler than the plan expected: (1) game saves
      are `historical` format = whole-struct binary, so a new struct
      field persists with NO hand-written sfo pair; (2) the
      field-level writers (exportascii) are GENERATED from the
      struct definitions via ctags -> util/sftags -> sfdata.c +
      include/sfproto.h when building the OPTIONAL sfctool target
      (NEW BUILD DEP: universal-ctags); (3) the plan's
      "critical_members[]" is actually critical_sizes[] in
      src/version.c - stores (uchar) sizeof(struct obj) etc. in the
      save header AUTOMATICALLY, no manual entry (plan corrected);
      (4) verification: sfctool -c <save> exports per-field ascii -
      verified tin=STEEL(13), fedora=CLOTH(6), bullwhip/leather
      jacket=LEATHER(8) in the stream; in-game S+restore round trip
      intact. Scope landed with it: EvilHack materials ENUM adopted
      wholesale in objclass.h (values FROZEN, append-only - saved
      raw), METAL->STEEL sweep (objects.h data + mhitm/uhitm +
      zap.c create_polymon golem cases mirrored from master),
      instance-based is_metallic/is_heavy_metallic, mksobj default
      init (init_obj_material() hook point marked for Phase 2).
      GOTCHAS: sfctool exits on the fork sysconf's OPTIONS/MSGTYPE
      lines (tool build lacks the fork's sysconf extensions -
      comment them out temporarily); sfctool -u round-trip differs
      from the original in ~1400 bytes = struct PADDING noise
      (historical bwrite includes padding; unconvert zeroes it) -
      benign, verify by field not by cmp; ~/.evilhackrc is SHARED
      with the live 3.6 game during migration - branch test
      launches use --no-evilhackrc (renamed flag verified working)
- [x] One small .des level to Lua: nymph.lua exemplar validated
      through the REAL engine (2026-07-12, #wizloaddes on the
      branch): the sp_lev Lua loader parsed and executed it and
      stopped exactly at the first EvilHack-only binding - impossible
      "Unknown room type 'garden'" (non-fatal, paniclogged, game
      continued; graceful degradation confirmed). This is the
      runtime class luac -p cannot catch - the engine-load test is
      the real gate for Phase 3, and the missing pieces are exactly
      the tracker's Phase 2 nhl-binding items (garden room type et
      al). BONUS finding: the panic path wrote var/save/0wizard.e.gz
      - the fork carries the emergency-save machinery and it works
      on the branch. Session-driving gotcha reconfirmed the hard
      way: capture-pane before EVERY send-keys; a queued 'y' during
      a panic --More-- chain answered "Dump core?" and aborted the
      test game (self-inflicted, no harm).
- [x] Effort calibration for the XL phases (2026-07-12, one
      session for the whole pilot): persistent field ~= 2h incl.
      tooling bring-up (recipe now known: near-instant for
      whole-struct fields); one monster ~= 30min + one exhaustive-
      switch case; one object+property+skill ~= 45min (watch
      positional name tables); the "cleanest" EvilHack-only file =
      ~3h incl. three dependency slices and six build-file parity
      entries - EVERY EvilHack file will drag a subsystem tail, so
      Phase 2 file estimates should assume 2-4x the naive line
      count. Engine-level validation (wizloaddes / genesis / save
      round-trip) catches what compilers and luac cannot; budget a
      short wizmode session per landed slice.

## Phase 1 - Types, fields, content data

### Persistent-struct enumeration (authoritative source:
`git diff 44b6612fd..master -- include/` - NEVER from memory)

- [x] obj.h DONE 2026-07-12b (subagent, verified)
- [x] monst.h DONE 2026-07-12b (mintrinsics widened to long;
      mw2/erid raw pointers need relink code before first use)
- [ ] prop.h (+154: property enum - couples to SAVED uprops array;
      keep append-only vs upstream ordering) [~] ULTRAVISION=70
      appended 2026-07-12 (objects.h needed it); rest pending
- [x] you.h DONE 2026-07-12b (u_achieve NOT ported: 5.0 enum
      achievements + 31-bit xlogfile mask full; 2026-07-17
      kill-attribution slice appended 8 once-gate u.uevent bits
      killed_medusa..killed_michael = the game-state source;
      defeat_icequeen still pending with the Ice Queen slice.
      MAINTAINER DECIDED 2026-07-17: restore master's achieveX
      named-token xlogfile field via a DEDICATED xlog-parity
      Phase 2 slice - see that item in the Phase 2 list)
- [ ] skills.h (+88: P_PET_HANDLING etc. - saved weapon_skills)
      [~] P_EVOCATION_SPELL landed 2026-07-12 INSERTED at 35 to
      extend the contiguous spell block (P_BARE_HANDED..
      P_TELEKINESIS shifted +1; acceptable pre-real-saves, noted
      in skills.h comment; weapon.c positional tables updated -
      remember that gotcha for every remaining skill)
- [x] flag.h DONE 2026-07-12b (fields only; options.c wiring
      with subsystems; sparkle bool-vs-int kept 5.0 bool)
- [x] mextra.h DONE 2026-07-12b (copy_mextra intentionally NOT
      extended - needs new* constructors, Phase 2)
- [x] context.h DONE 2026-07-12b (soko 6-prize system deferred:
      5.0 tracks prizes via svc.context.achieveo - extend that,
      not record_achieve_special; DONE 2026-08-02 sokoban slice -
      soko_prize_oid[NUM_SOKO_PRIZES] slots, otyp stays scalar
      written at pickup)
- [d] sfo_* writers OBSOLETE (Pilot D: historical whole-struct
      saves; critical_sizes[] stamps sizeof automatically; struct
      growth invalidates dev saves only - none kept)
- [ ] gX vs svX decision LOG (append every placement + why):

| Global | gX/svX | Why |
|--------|--------|-----|
| _example: forge heat state_ | sv? | survives save |
| worn_slot_exempt (poly_obj/worn_slot_sanity) | gw | transient diagnostic exemption, Null between moves, never saved; g_init_w resets it per game (better than master's bare global) |
| exploder (explode attribution) | ge | transient, save/restore bracketed per explode() call, m_detach clears; never saved (gb.buzzer precedent) |
| thrower (missile attribution) | gt | transient, bracketed per m_throw/thrwmu flight, m_detach clears; never saved (gb.buzzer precedent) |

### Content data

- [x] Extend MON() macro variants to 16 args - PRE-DRAFTED 2026-07-03
      (monsters-macros-predraft.h: monsters.h guard+tail blocks,
      SEDUCTION_* re-insert anchor, branch permonst struct, monst.c
      mons_init block; compile-validated via macro-test/driver.sh -
      gnu89 run + c99 syntax, 0 warnings, NUMMONS=561; splice on the
      branch at Phase 1)
- [x] Decide the 3 NAMS merge pairs - DONE 2026-07-03 (Decisions
      log; converter auto-merges, 564 -> 561 entries, 0 PORT-TODOs)
- [x] Regenerate + splice ~/monst_to_50.py output (561 entries;
      2026-07-12, converters re-run at adoption trees, round-trip
      clean; MH_ race-bit migration + PM_ rename sweep + grownups[]
      replacement + make_corpse exhaustive-switch regeneration
      landed with it; details in the content commit message)
- [x] Review monster cross-diff report - RESOLVED 2026-07-12: all
      12 "5.0-only monsters" are upstream's gendered-NAMS name
      triples or renames of monsters EvilHack renamed differently
      (leader/ruler vs noble/royal, Grand Master vs Master Po,
      Chromatic Dragon vs Tiamat+placeholder); ZERO new species -
      EvilHack's set + naming win per full-parity mandate
- [x] Extend objects.h wrapper macros - PRE-DRAFTED 2026-07-03
      (objects-macros-predraft.h: upstream-verbatim context guards +
      GENERIC prelude + 23 EvilHack-signature wrappers +sn + MARKER
      insertion list + fencepost tail; same harness, NUM_OBJECTS=583
      = 566 entries + 17 GENERIC slots, gems/spells marker ranges
      verified; PAPER-override + P/S/B re-insert anchors documented)
- [x] Regenerate + splice ~/objects_to_50.py output (566 entries;
      2026-07-12, upstream GENERIC/MARKER prelude kept, 8 content
      MARKERs + PAPER override re-anchored; onames rename sweep,
      trap-type *_TRAP_SET renames, dragon-gear macro adoption,
      silver-weapon base-type substitution landed with it; NOTE:
      upstream-only content carved pending adoption decision - see
      Decisions log and the 5.0-only content queue below)
- [x] artilist.h DONE 2026-07-12b: EvilHack's 69 entries in 5.0's
      four-context layout via scratchpad merge_artilist.py; struct
      artifact gains material; gen_spe/gift_value from upstream for
      the 35 shared, heuristic defaults for the 34 EvilHack-only
      (report file scratchpad draft-artilist.h.report - maintainer
      balance pass pending); ART_ name map vs EvilHack master's
      makedefs manglings: ART_MAGIC___BALL->ART_MAGIC_8_BALL,
      ART_RING_OF_P_HUL->ART_RING_OF_PHUL,
      ART_STRIPED_SHIRT_OF_LIBERATIO(N), ART_BRACERS_OF_THE_FIRST_
      CIRCL(E) - REMEMBER when porting EvilHack code in Phase 2;
      quest-arti boundary = ART_XIUHCOATL; Gjallar carries the
      readable "Heimdallr." signature
- [x] roles/races structural port DONE 2026-07-12b (zero
      warnings; the role.c:1741 marker warning resolved by the
      real mhrace fix): struct Role.mhrace; rfilter.race_mask
      split (MH selfmasks overlap ROLE_GENDMASK - vampire=MALE
      bit!); 16 roles + 14 RACES (Draugr is its own race - plan
      said 13) + align_roles/race_roles/draugr_roles/race_demon;
      role_init variant swaps; skills.h final enum (P_KNIFE/
      P_MORNING_STAR folded away, THIEVERY/SHIELD/PET_HANDLING
      added); femalenum columns merged to NAMS-era mnum
- [x] u_init.c port DONE 2026-07-12b (subagent, verified; zero
      warnings): all role/race inventories + 22 skill tables incl.
      Skill_Dra_*; trquan column adapted to min/max; 6 TODO-EVILHACK
      deferrals inside (valid_obj_material/set_material, otainted,
      maybe_erodeproof x2, oprops) - each ports with its Phase 2
      subsystem. Judgment calls: Draugr/Vampire -20 start expressed
      as direct u.ualign.record set (adjalign would charge abuse +
      buff Erinys + stack across rerolls); vampire blood-potion
      oc_prob mutation resyncs init_oclass_probs() (5.0 caches
      class totals). FOLLOW-UPS: pauper Infidel starts without the
      Amulet (maintainer decision when Infidel mechanics port);
      vampire oc_prob resets on save/restore (re-apply at vampire
      race port; latent in EvilHack master too); Druid bracers
      rewield carve-outs pending. BOOT TESTS now unblocked (gate
      still needs glyph audit first)
- [ ] 5.0-only content adoption - DECIDED 2026-07-12 (maintainer):
      ADOPT spellbook of chain lightning (cast_chain_lightning()
      still compiled; pick attack vs evocation school at re-add)
      and shields of drain/shock resistance; DECLINE wand of
      stasis (level.flags.stasis_until machinery left dormant in
      the tree - harmless, eases future upstream re-derives) and
      dragon scale mail objects (superseded by EvilHack's
      scales-fusion dragon-scaled armor, Phase 2). Re-add the two
      adopted items as a small slice after the Role-struct port:
      restore code sites from pre-swap history, append objects.h
      entries (enum append-only; new book needs an unused
      description; mirror upstream oc_prob and rebalance the
      class to keep the o_init probability sum intact)
- [x] Glyph-space audit DONE 2026-07-13: all consumers X-macro/
      NUMMONS-derived; the one hardcoded bound (tilemap tilelist
      [2500]) was caught+fixed in the content swap; generated
      tile.c shows sex-split mappings working (female Yeenoghu)
- [x] GATE PASSED 2026-07-13 (wizmode, tortle Monk): boots clean
      (no impossible), genesis drow warrior + alhoon OK, wishes OK
      (spellbook of chain lightning, gray dragon scales), save +
      reload restores character/map/monsters/inventory; vanilla
      dungeon generates. GATE FINDING fixed en route: do_wear
      armor on/off switches impossible()'d on EvilHack armor
      otyps ("Unknown type of helmet (117)" = toque) - all 19
      missing case labels/behaviors added from EvilHack master
      (helm of speed/telekinesis, dark elven stealth pieces,
      RIN_LUSTROUS, gauntlets of protection, mummified hand
      TODO-EVILHACK for the Vecna meld, bracers/shields labels).
      FOLLOW-UPS: (1) upstream dat/*.lua reference RENAMED object
      names - tut-1.lua fails to load (paniclog, safe maze
      fallback); sweep upstream lua for renamed object/monster
      strings = Phase 3 item with reproducer; (2) re-test
      chromatic-scales wish parity (EvilHack blocks it) once the
      wish-restriction port lands; (3) wish-prompt EDIT_GETLIN
      remembers previous input - clear with ^U when driving tests;
      (4) FROM THE PHASE 2 SMOKE 2026-07-13: o_init init_objects
      boot check (sanity_check on) flags 5 ported objects "name is
      not known despite no alternate description" - ashwood staff
      #101, ring placeholders ancient #240 / lustrous #241, pan
      flute #316, gold piece #539: all have real name + NoDes +
      kn=0 (byte-faithful to master, but 5.0 requires desc-less
      objects born name-known and silently repairs kn->1 at init) -
      fix = kn 1 in the five objects.h entries (pure formalization
      of the runtime repair, zero behavior delta) - FIXED 2026-07-13:
      kn columns for ashwood staff + pan flute, raw OBJECT expansion
      for the two ring placeholders (RING wrapper hardcodes kn 0;
      barding idiom), COIN macro BITS kn restored to upstream's 1;
      zero-warning rebuild + boot re-verified clean (tortle Healer,
      sanity_check on, no o_init reports); (5) SAME SMOKE:
      upstream WIP-gated shop_keeper() diagnostic "untrustworthy
      null shkp; shkready FALSE (1,0,0,0)" fired 4x at newgame
      Dlvl 1 creation (timestamp decode corrected the first Dlvl 3
      guess) - RESOLVED 2026-07-13 as UPSTREAM 5.0 BUG, not
      port-induced: gdb backtrace (breakpoint shk.c:1082, caught on
      a fresh throne-room level) = fill_zoo COURT royal-coffers
      fill -> add_to_container(chest, gold) -> merged -> obfree,
      whose DELIBERATELY unconditional billing fallback
      shop_keeper(*u.ushops) (shk.c:1227, historical comment right
      there) probes with rmno '\0' during making and trips 5.0's
      new WIP diagnostic; chest is created with init=TRUE so its
      random contents can include gold for the merge. All three
      code sites byte-identical in NetHack50 (shk.c:1227, 1082,
      mkroom.c:430); nothing in upstream fixes docs. Suggested
      upstream fix: skip the diagnostic when rmno < ROOMOFFSET (a
      no-room probe's NULL answer is definitionally trustworthy).
      Entry added to the upstream-reportable table; local guard =
      maintainer decision (paniclog noise during migration vs
      divergence)

## Phase 2 - Core subsystems (port detectors FIRST)

- [x] Lifecycle detectors EARLY: dmonsfree accounting,
      freed-without-detach, off-map orphan (BUG-1259 class),
      worn_slot_sanity - re-derive onto 5.0 mstate machine
      DONE 2026-07-13, commit e265bd879; wizmode smoke PASSED same
      day (Valkyrie, sanity_check on via EVILHACKOPTIONS env: 4
      kills through the death pipeline, pet corpse-eating, level
      teleport with pet = STILL_ARRIVING window, THE BUG-1290 path
      dip-poly of a worn helm of opposite alignment - exemption
      window kept the detector silent through the in-window
      uchangealign/retouch, alignment/AC/slot all correct after -
      save+reload exact; ZERO detector paniclog entries; two
      PRE-EXISTING findings surfaced, logged as Phase 1 follow-ups
      in the gate section). PORTED 2026-07-13: recent_detaches[] ring +
      movemon_callctr + double-detach history dump (m_detach), off-map
      orphan scan + freed-undetached tracking + rich count-mismatch
      diagnostic (dmonsfree), worn_slot_sanity() in mkobj.c (called
      from obj_sanity_check + retouch_equipment entry, both gated
      iflags.sanity_check||debug_fuzzer like master) with
      gw.worn_slot_exempt poly_obj-transient exemption (zap.c window
      around remove_worn_item). All observation-only, zero behavior
      change. NOT ported (deferred with their subsystems): Ice Queen
      revive interception in dmonsfree (TODO-EVILHACK marker; Ice
      Queen bosses item), erid rider-separation in the free loop
      (TODO-EVILHACK; monster-riding item), vault.c grddead
      purge_monsters-- + restfakecorr mongone compensations (3.6
      hybrid-specific; memory pattern-dmonsfree-purge-accounting
      verdict = dead code in single-guard flow; revisit at the
      vault-guard subsystem review), master's wiz_makemap purge drain
      (upstream already has makemap_remove_mons). Paniclog chops
      impossible() text at BUFSZ-1 on BOTH trees; the oversized
      msgbuf locals are deliberate - core-dump/GDB readers see them
      whole (see pattern-crashlog-print-elements-truncation).
      Reviewer pass 2026-07-13 (claims verified in-tree): orphan
      scan uses mon_offmap() instead of master's verbatim six-flag
      mask - also covers 5.0-new BUBBLEMOVE/OBLITERATE/
      STILL_ARRIVING/PARKED states that could false-fire after
      future upstream merges; NOTE 5.0 natively added
      check_wornmask_slots() (worn.c, per sanity pass) covering the
      slot->invent direction too - our variant adds dedup +
      poly_obj exemption + retouch entry hook (comment cross-refs
      it; the native one is NOT exemption-aware but is unreachable
      inside the poly_obj window today); ring/dedup statics stay
      file-statics like master (diagnostic-only; a same-process
      game restart can stale them - migrate to a g_init struct if
      that ever matters); detach ring mname capped at 64 via
      copynchars (fixes a -Wformat-overflow master never saw).
      Build zero-warning after fixes.
      Remaining for [x]: maintainer commit + wizmode smoke (needs
      maintainer-approved launch; reviewer test list: poly a worn
      helm of opposite alignment with sanity_check on, vault-guard
      cycle, Plane of Water bubbles, crowded-level arrival, fuzzer
      with fuzzer_impossible_panic)
- [~] Materials system (obj->material; init/naming/pricing/burn)
      SLICE A (core assignment) IN PROGRESS 2026-07-13, subagent-
      executed + reviewed: instance-based predicates (objclass.h;
      is_supermaterial/is_fixed/is_damageable/mat_is_* added;
      is_crackable converted), is_drow_obj/is_drow_weapon (obj.h),
      matdensities[] + weight() scaling, maybe_erodeproof, 20 icp
      tables + material_list/init_obj_material/invalid_obj_material/
      valid_obj_material/set_material (soko-prize clause
      TODO-EVILHACK - DONE 2026-08-02 sokoban slice, oid form,
      hardcoded reflection/speed/MR otyps), wiring at mksobj +
      mkbox_cnts + mkobj_erosions,
      obj_sanity_check validity check, u_init 4 material TODOs
      un-deferred + anti-start-scum block. ADAPTATION: set_material
      erosion-clear follows the 5.0 crack model (glass on oeroded
      via is_crackable, NOT master's oeroded2; verified vs 5 crack
      sites). REVIEWER BLOCKER FIXED same day: master pairs every
      otyp-rewrite with a material refresh - ported spell/trap/pray
      blanking, zap poly_obj transmutes (+cancel_item sibling the
      agent found itself, master zap.c:1521), worm tooth<->crysknife
      x4 (invent.c site ADDED - master LACKS it, candidate master
      backport), uhitm egg->rock, bones resetobjs (general validity
      fixup + fake-Amulet/wax-candle/Bell; the MAGIC_KEY/BLACKSMITH_
      HAMMER bones rewrites are not on the branch yet - their
      set_material travels with them), t_missile per-shot
      base-material guard (TODO until set_trap_ammo ports).
      poly_obj oerodeproof carry-over still direct assign vs
      master's maybe_erodeproof - later slice. VERIFIED 2026-07-13
      (wizmode, sanity_check on): elf Priest starts with a COPPER
      mace (gdb: otyp 82 material 14) and it stays copper across
      save/reload; polymorph churn on floor stacks silent; per-turn
      material validity quiet in normal play. Incidental finds:
      (a) race-picker has DUPLICATE accelerators (h human/hobbit,
      d dwarf/drow - drow may be letter-unreachable; Phase 1 roles
      follow-up), (b) tut-1.lua partial-load debris trips engraving
      sanity x3 on the fallback maze (corollary of the known Phase 3
      tut-1.lua item - engravings placed before the line-147
      failure). SLICE B (naming/wishing/artifact normalization)
      DONE 2026-07-13 (commit pending): force_material_name + xname
      material prefixes (4 class branches + minimal_xname bareobj),
      artifact_material() + oname() normalization gated on 5.0's
      ONAME_VIA_NAMING (only do_oname sets it - closes BONE
      Excalibur), not_spec_material + readobjnam material parse in
      the 5.0 preparse structure (token consumption bounds-guarded
      vs master; apply block placed BEFORE erosion since 5.0 moved
      erosion before oname - outcome-equivalent, wizmode-only pline
      divergence on invalid artifact materials), "<material> armor"
      redirects re-expressed in return-code convention, GOLD
      fallback. REVIEWER BLOCKER FIXED: readobjnam wrp-loop guard
      list updated to master's (studded armor/armor renames + ring
      of p'/wand of orcus/glamdring/one ring/bag of devouring -
      the artifact entries were broken at HEAD since the Phase 1
      content swap, not just this slice). VERIFIED (wizmode): elf
      Priest inventory shows "copper mace"; wishes: silver dagger,
      studded armor (guard fix), bag of devouring (guard fix),
      glamdring -> "mithril elven long sword named Glamdring"
      (guard fix + normalization); all names survive save/reload.
      Reviewer nits logged: not_spec_material one-past-NUL read on
      bare uppercase material wish (master-parity flaw, fix master
      first), "iron door" terrain wish drops material silently
      (terrain-materials slice), invalid-material artifact wish
      prints a Note master didn't (intentional). sfctool LINK
      broken pre-existing (sfi_x_emsp/sfi_x_abuse_event missing
      from Phase 1 field enumeration - regen sfdata.c when next
      needed). SLICE C (pricing) DONE 2026-07-13 (commit pending,
      hand-ported - small): matprices[] (static const, only shk.c
      uses it) + the multiplier/divisor material ratio in get_cost
      AND set_cost (5.0 already adopted the accumulator pattern
      independently, so master's lines dropped in verbatim);
      set_cost gem-sell condition switched to instance material +
      is_worthless_glass (macro added to obj.h; kept 5.0's
      FIRST_REAL_GEM formula + divisor reset - master's variant is
      3.6 inheritance). NOTE master's get_cost has the charisma/
      racial rework (multiplier *=/shk_stat_priced/racial numer-
      denom) - that ports with the shop-racial-adjustments item,
      NOT here; 5.0's own charisma block kept. VERIFIED live (gdb
      call on wished daggers): get_cost silver 16 vs iron 5,
      set_cost 6 vs 2 - exact 3x silver/iron ratio both directions.
      SLICE D (combat: material hatred/searing) IN PROGRESS
      2026-07-13, subagent-ported 27 files (+681/-337), reviewed:
      mon_hates_material/hates_material/sear_damage/monmaterial/
      is_fleshy (mondata.c; erac clause = the slice's one
      TODO-EVILHACK), Hate_material (youprop.h), searmsg +
      special_dmgval rework (magr,mdef,armask,&hated_obj) + dmgval
      material bonus (weapon.c), hmd.hated_obj lifecycle with UAF
      guards (boomerang splinter/lance shatter - reviewer traced
      ALL destruction paths clean), retouch_object master rework
      (new direct param + will_touch_skin; 5.0 touch_blasted gate
      dropped per master; eat.c passes direct=TRUE deliberately -
      mouth contact), mk_artifact gift filter, ~20 consumer files
      (mhitm/mhitu/mthrowu/dokick/dothrow/dig/trap/pickup/steal/
      dog/muse/makemon reroll loop/insight enlightenment).
      REVIEWER BLOCKER FIXED: dmgval's 5.0 class gate
      (Is_weapon/GEM/BALL/CHAIN) went unconditional like master
      (the special_dmgval delegation needs worn items to flow
      through dmgval; reviewer traced single-count safety on all
      paths); + 4 concerns fixed (heavy-ball hated_obj message,
      thick_skinned instance read + barkskin/stoneskin defender
      fold-in, shade_glare instance read, returning-weapon self-hit
      sear); eat.c direct=TRUE divergence documented in-code.
      VERIFIED 2026-07-13 (wizmode elf Priest XL15, sanity_check
      on, paniclog clean): iron dagger wield -> "The iron of your
      dagger hurts to touch!" (retouch, bearable non-silver);
      silver saber kill (lethal-blow sear suppressed by design);
      BLOCKER CASE bare-hand punch with worn silver ring -> "Your
      silver ring of protection sears the werewolf's flesh!";
      negative control gemstone ring dmgval=0 via gdb (ring
      materials follow SHUFFLED descriptions - "ring of
      teleportation" was topaz this game; test gotcha worth
      remembering); mon_hates_material(werewolf,SILVER)=1 by gdb;
      enlightenment "You are harmed by iron." for the elf.
      Wizard death prompt saved a level-1 test hero mid-battery -
      genesis werewolves one-shot fresh heroes, #levelchange first.
      EXTENDED BATTERY post-commit f5af0ebd4 (2026-07-13, random
      Infidel XL15, paniclog zero new entries): shade kick with
      blessed jackboots CONNECTS (was "passes harmlessly through" -
      blocker half 2); wielded iron chain vs Grey-elf prints "The
      Grey-elf is burned by your iron chain!" (the misc ball/chain
      hated_obj message fix); mongets stress = no reroll
      impossibles AND a genesis'd Grey-elf threw a "mithril runed
      spear" (reroll away from hated iron + material naming live in
      combat messages); spear passed harmlessly through the shade
      (noncorporeal intact); silver boomerang vs Grey-elf = no sear
      (correct - elves hate iron not silver); iron chain wield
      burned the (iron-hating-race) hero on wield. Boomerang
      splinter UAF stays statically-verified only (reviewer traced
      all hmon destruction paths; in-game rnl(4) repro skipped).
      SLICE D COMPLETE - materials system remaining: Croesus
      transmutes, class-based consumer stragglers, master-parity
      nits (all inventoried above). DEFERRED with their subsystems
      (reviewer-verified no dangling refs): blocker-sears family,
      monster-touch-attack sears (armask plumbing in hitmu),
      AI weapon-ranking oselect/score_artifact material filters
      (until then a monster CAN wield instance-silver it hates -
      known interim gap), maybe_arm_silver, barding rerolls, door
      kick sear (terrain materials), pet armor-exchange filter,
      pager harmed-by lines. Master-parity nits tracked: pickup.c
      "a iron container" grammar + searmsg unbounded Sprintf
      (BUG-1315 family) + minimal-comment mismatch - fix master
      first. MR2_BARKSKIN/STONESKIN allocated, no monster producer
      yet (hero side live).
      DEFERRED to later
      material slices (reviewer-inventoried): metal_to_wood +
      material_to_gold (Croesus item), bones.c full pass,
      class-based consumer conversions (breaktest glass, o_material
      detect, poly_zapped, stone_missile, is_flimsy), racial-gear
      macro gaps (is_elven_weapon/is_orcish_obj/is_dwarvish_obj
      lack EvilHack's added otyps - ELVEN_LONG_SWORD etc.), NITs:
      valid_obj_material(NO_MATERIAL) leading reject,
      matdensities[] 5-bit-field OOB clamp on restore,
      init_dummyobj material, is_rottable dead BONE test (master
      parity, kept)
- [x] Object properties (oprops)
      PORTED + REVIEWED 2026-07-15 (subagent-ported 33 files
      +~1400, reviewer CONDITIONAL FAIL -> all findings fixed by
      coordinator same day, make clean rebuild zero-warning; NOT
      yet committed). Contents: create_oprop/enforce_oprops_mutex
      producers + arti_prop_spfx/obj_has_prop helpers; generation
      at mksobj x4 (incl. new BARDING/SPIKED_BARDING init case),
      makemon x4 (the B2-deferred mplayer/angel/Dispater sites),
      Izchak sword, invoke_create_ammo, wish; objnam naming
      (opropnames table, per-class prefix/suffix rework, magical
      prefix, oprops_known gating, end-of-game dump_prop
      disclosure) + wish parse (preparse magical/oilskin,
      postparse of/and property loop, class-restriction mask);
      do_wear oprops_on/_off all 7 slots + Armor_gone + EXCEL
      luck/CHA (adj_abon, acurr, extremeattr); worn.c
      update_mon_extrinsics oprops loop + obj_has_prop; mondata
      mon_prop() whole + resists_drli/resists_magm worn loops +
      ~20 consumer upgrades (apply/dog/dogmove/mcastu/mhitu/mon/
      monmove/polyself/teleport/uhitm) - resolves the "mon_prop
      upgrade path" open decision; engulf slow-digestion expel
      both directions via NEW M_ATTK_EXPELLED 0x10 (master's 0x8
      is 5.0's M_ATTK_AGR_DONE - anything porting master code
      that hardcodes 0x8-as-expelled must re-map); combat riders
      (artifact_hit FIRE/FROST/SHOCK/DRLI/VENOM + known-marking,
      spec_dbon early block, venom -> hmon poison path, bare-hand
      gloves rider, oilskin slip both directions, thrown
      ammo_stack whole-stack learn); getprice oprops factor;
      steal.c mdrop_obj saved-fields dummy (fixes an UPSTREAM
      5.0 UAF - see Upstream-reportable table); bones/oname/
      u_init/invent/wield sites. B2's inert racial-gear ITEM_*
      assignments (Vecna robe, Saint Michael runesword, paladin
      gear, Izchak sword) now fully live.
      REVIEWER FINDINGS ALL FIXED: BLOCKER worn.c x3
      set_artifact_intrinsic caller gates needed || obj->oprops
      (wielded SEARCHING/WARNING/ESP was dead code); CONCERN wish
      gate now tests makesingular(bp) per master ("wands of
      fire" was hijacked by the property scan; gauntlets/gloves/
      boots/eyes are as_is[] so their protections survive
      singularization); CONCERN unique_corpstat engulf expel
      ported both directions (purple worm could digest uniques;
      barding expel stays with barding slice); CONCERN
      thunderclap re-gated is_art(ART_MJOLLNIR) (Keolewa/Tempest
      shocks were waking radius 16); CONCERN AD_DRST section
      restructured to master shape (Sword of Kas message + hero
      1/8 STR poison were skipped by an ITEM_VENOM outer gate).
      REVIEWER CONFIRMED: gulpum expel returns M_ATTK_HIT not
      master's literal 2 (master's 2 = DEF_DIED wrongly fired the
      zombie-sick rider and passed maliveb=0 for a live monster -
      deliberate improvement); M_ATTK_EXPELLED mask-safe at every
      consumer (no ==-tests reachable, gulpmm consumes the bit,
      passivemm overwrites res[i]); steal.c dummy carries exactly
      the fields update_mon_extrinsics reads (dropping master's
      dragonscales save is correct - branch derives via
      oc_oprop); stackobj merge in mdrop_obj safe path CANNOT
      free the dropped obj (reachability proof in review).
      DELIBERATE INTENT-FIX vs master (MASTER BACKPORT CANDIDATE
      #5): master's wish-parse exclusion checks clobber the
      match length via assignments inside strncmpi args (fork
      objnam.c ~4310 `l = 4` then `l = 5`), silently breaking
      "X of fire and frost" two-property wishes; branch uses
      constant-length exclusion tests.
      OPEN DECISIONS RECORDED: mon_prop uses branch spec_ability
      (spfx-only) - a merely CARRIED artifact confers less than
      master's known carried-intrinsic leak; revisit with the
      artifact-behavior slice. W_SWAPWEP oprops extrinsics inert
      (5.0 setworn skips off-hand intrinsics for artifacts too);
      the artifact.c W_SWAPWEP mapping arm is unreachable until
      a twoweap-extrinsics decision.
      DEFERRED (item -> subsystem): pray.c gear-gift/altar-venom/
      Moloch-hellfire/Dirge oprops (sacrifice + Infidel + Dirge),
      fountain.c forge-transfer + property gems + sp_lev
      affixed-gem (Forging), monster ring/amulet wearing
      (extra_pref/RIN_* muse sites), detect-magic ITEM_MAGICAL
      (SPE_DETECT_MAGIC absent), dragon-scales merge oprops strip,
      trap-kit transfer, terrain-burning fire-weapon, greased/
      oilskin theft-slip trio, shield-bash rider, thrown-slip
      self-hit, monster teleport-AI (decide_to_teleport),
      pet-stash filter, vulnerable_to/mon_underwater severity,
      "tainted " xname prefix (drow-poison - producers exist,
      display does not; LANDED 2026-07-31 with BUG-1421's parity
      sweep), resists_magm defends(AD_MAGM)+W_BARDING
      (artifact-defense wiring: Striped Shirt/Ring of P'hul/Eyes
      of the Overworld/Ithilmar do not confer MR when worn yet),
      mk_artifact forged_qual/affixed_gem zeroing (Forging),
      SLING_BULLET quan rn1(7,8) mkobj init (sling parity, found
      in passing).
      OPROPS WIZMODE BATTERY RUN 2026-07-15 (maintainer-approved,
      fresh install of e95bed697, sanity_check ON): PANICLOG
      EMPTY. PASSED LIVE: (1) B1-blocker regression - "long sword
      of warning" wielded -> EWarning gains W_WEP (gdb 0x100),
      unwield clears to 0; (2) armor extrinsics - hunger-prop
      cloak wear -> EHunger W_ARMC, remove -> 0; detrimental
      "magical" prop item born CURSED (create_oprop curse rule
      observed); (3) EXCEL - blessed excellence sword wield ->
      CHA 7->25 floor + u.moreluck 3, unwield reverts both;
      (4) wish parse 15/15 - fire sword oprops 0x1, "of fire and
      frost" -> FROST only 0x2 = VERIFIED MASTER PARITY (mutex
      chain is frost-first; damage props mutually exclusive by
      documented design - the review's "wizard gets both"
      expectation was wrong), silver saber of frost mat 16 +
      0x2, "magical" random prop incl. detrimental, oilskin
      stripped on non-cloth, helm-of-telepathy/gauntlets-of-
      fumbling/scroll-of-fire/potion/tin/figurine all resolve to
      fixed items with oprops 0, C1-fixed plurals live ("2 wands
      of fire" -> WAN_FIRE, "wands of lightning" -> WAN_LIGHTNING,
      "spellbooks of fireball" -> SPE_FIREBALL); wished props are
      born UNKNOWN (master parity - master's wish tail never
      marks oprops_known; reveal on proc); (5) combat - fire
      proc "The long sword burns the gnoll!" + autolearn
      known=0x1 + post-learn naming "long sword of fire" in
      combat messages; frost vs cold-resistant winter wolf ->
      plain "hits." + NO learn; (6) engulf - hero purple worm
      "You swallow Croesus whole!" -> "You hurriedly regurgitate
      the indigestible Croesus." target alive (C2 fix); (8)
      thunderclap - Keolewa lightning proc leaves adjacent
      sleeper asleep, Mjollnir wakes it "The gnoll wakes up."
      (C3 fix); (9) pricing via gdb getprice() - long sword base
      15 -> 115 one-prop (+100 factor exact), excellence 125;
      (10) end-of-game identified-possessions screen renders all
      props in full ("blessed +1 long sword of excellence", "+0
      silver saber of frost", ...) cleanly distinct from fixed
      items. NOT LIVE-TESTED: (7) lava-drop mdrop_obj hardening
      (not deterministically stageable; reviewer-verified incl.
      the stackobj reachability proof), thrown-stack whole-stack
      learn + bare-hand gloves rider (same rider plumbing as
      melee, reviewer-verified), dumplog FILE write (procedural
      miss: sysconf DUMPLOGFILE retarget happened mid-game but
      sysconf is read at game START, so the game kept the
      /dgldir path - the disclosure CONTENT was verified on
      screen; retest opportunistically with the retarget done
      BEFORE launch), mon-side worn-prop extrinsics in live
      combat (obj_has_prop/mon_prop paths gdb-adjacent only).
- [~] Racial monsters (ERAC overlay)
      SLICE A (core) 2026-07-13, subagent-ported 20 files +791,
      reviewed (conditional pass, NO blockers; fixups in flight):
      struct erac (Phase 1 fields matched master exactly; NO
      pointers = no relink), newerac/free_erac + copy_mextra ext,
      FULL sf-layer serializer (ebones precedent: save.c/restore.c
      blocks after ebones before mcorpsenm; SF_C in sfmacros.h,
      sfprocs positional member epri->ERAC->eshk, sfstruct both
      tables, sfbase norm_ptrs stub, savefile.h x8, version.c
      critical-sizes entry, sftags/sfexpasc/sfctool wiring),
      r_data + racial_* family + erac_race_hostile/peaceful +
      is_racialmon + safe_mons (mondata.h), apply_race/pm_to_race/
      m_chooserace/align_randrace/m_randrace (mon.c; NAMS merges
      CAVE_DWELLER/CLERIC adapted), generation (mercenaries always
      roll, mplayers via init_mplayer_erac, aligned/high priests +
      emin via align_randrace, newcham re-rolls; peace_minded
      signature -> struct monst* all 3 callers), racial_adj through
      5.0 x_monnam (+ shopkeeper branch, dormant until shk erac),
      mon_hates_material erac clause UN-DEFERRED. LATENT PHASE 1
      BUG FOUND+FIXED: race_hostile/peaceful tested mflags2 vs
      MH-space races[].hatemask - dead test; now mhflags (also
      makes the combat slice's mk_artifact race filter actually
      work). SAVE FORMAT: dev saves invalidated (critical-bytes
      refusal names struct erac - LIVE-VERIFIED refusing the old
      Phase 1 gate save cleanly, no crash/debris beyond the known
      unpack; gate save now format-obsolete, backups in scratchpad,
      regenerate a gate save on this build when needed). VERIFIED
      live: genesis "dwarven soldier"/"dwarven watchman"/"giant
      watchman" (racial rolls + naming), save+reload keeps all
      three erac overlays (gdb: rmnum/mrace/ralign intact).
      REVIEW FIXUPS LANDED same day: mplayer mrace tests all
      bitwise now (was ==-vs-composite-mask, a MASTER BUG
      faithfully ported - MASTER BACKPORT CANDIDATE: vampire
      mplayers never get FLY, vampire Infidels wrongly MR_FIRE;
      fork-point mplayer.c:300-425), mk_mplayer christens
      unconditionally per master (sp_lev player-monsters named
      again), peace_minded racial carve-out deferral TODO-EVILHACK
      added (5 carve-outs listed, port with player races),
      mon_has_race reformatted. Reviewer NITs
      noted: newcham mplayer rank truncation keeps 5.0 endgame
      gate (stale rank if polymorphed elsewhere - cosmetic),
      r_data trusts restored rmnum (master parity), stale
      util/sfctool binary until make sfutil. ERAC->mattk racial
      attack overlays saved but NO combat consumer yet (Slice B).
      SLICE B1 (racial consumer sweep) COMMITTED 2026-07-13 as
      e9b9cab24, 44 files +736/-268 incl. coordinator fixups and
      this tracker's B1 record (this status line updated
      post-commit, rides with the next commit). Subagent-ported,
      reviewer CONDITIONAL PASS, clean make clean+all zero-warning
      after fixes. Wizmode battery still pending (with B2). Contents: mondata.h
      derived macros (racial_tunnels/needspick/perceives/
      throws_rocks, has_telepathy, extended telepathic 16-PM list,
      hates_light +drow/shadow dragons, ultravision, resists_slow,
      is_zombie/angel/troll/spider/accurate, cantweararm(mon));
      ~60 r_data sites; ERAC->mattk overlay INSIDE getmattk (=
      master's own choke point; hmonas safe, youmonst never has
      erac) + 6 full-matrix scanners (passive x13 refs, passivemm,
      ranged_attk_assessed/available/mon_avoiding, can_carry
      AT_ENGL, monmove castmu scan) - index-consistency reviewer-
      verified, zero leftover base-matrix reads; signature changes
      bad_rock/breakarm/sliparm/cantweararm -> struct monst* (all
      callers swept) + light_hits_gremlin -> light_hits_light_hater
      + litroom light_haters generalization; NEW wielding_artifact()
      helper (artifact.c, uwep+twoweap offhand); racial naming
      chain (corpse_xname/statue/tin prefixes, maybe_cannibal by
      ERAC rmnum x4 eat.c sites, start_corpse_timeout NON_PM guard,
      weight racial cwt, pray offering racial ptr); mm_2way
      aggression rows; elf/drow/orc to-hit matrix + M3_ACCURATE;
      boulder/giant sites (moverock family, hurtle, multishot,
      ignore_boulders); dog.c tamedog 'bane block + dogfood racial
      cases; mcastu CLC drow->Lolth spiders; display MH-space
      warning fixes (below). LATENT PHASE 1 BUG #2 FIXED (same
      MH-vs-M2 class as Slice A's): warning masks are MH-space
      (spec_mh producer) but three consumers tested mflags2 -
      see_monsters() count (port agent), MATCH_WARN_OF_MON macro
      hack.h + pager.c farlook overlay/race list (coordinator,
      post-review). MASTER BACKPORT CANDIDATE #3: mthrowu.c
      thrwmu elf/drow ranged bonus tests oc_skill == P_BOW but
      ammo skill is negative (-P_BOW) - dead code on master since
      3.6 vanilla; branch uses -P_BOW so the racial archer bonus
      is LIVE here for the first time (balance note for testing).
      B1 FIXUPS APPLIED 2026-07-14 (pre-B2 pass, build-green
      zero-warning): (1) tamedog 'bane list completed - all 9
      missing master gates added (Ogresmasher, Sunsword+Hammer of
      the Gods undead, Werebane, Shadowblade, Demonbane+Hammer
      demons, Angelslayer angels+aasimar, Vorpal Blade jabberwock,
      Dragonbane gloves, One Ring both hands vs wraiths); needed 5
      new mondata.h MH-space predicates (is_ogre/is_dragon/
      is_jabberwock/is_wraith/is_aasimar - MH bits were populated
      in monsters.h, macros missing on branch, no name conflicts).
      (2) tamedog RESTRUCTURED to master order: all bane gates now
      sit before the mpeaceful flip (bane-wielders cannot even
      pacify the hated race; 5.0 upstream has no bane gates so no
      upstream-vs-master conflict - decision = master parity).
      (3) drow web immunity hero half landed: trapeffect_web hero
      branch + immune_to_trap WEB case, both via racial_drow()
      (mon_has_race youmonst clause reproduces master's
      maybe_polyd(is_drow, Race_if) semantics - equivalence
      hand-checked across polyd/unpolyd cases; one idiom across
      all three web sites incl. the B1 monster half). (4) minion.c
      msummon gate -> wielding_artifact() x2 incl. Hammer of the
      Gods per master; sibling sweep caught 2 more Excalibur/
      Demonbane gates on u_wield_art that master expresses via
      wielding_artifact: demon_talk (minion.c) + dprince MS_BRIBE
      genesis (makemon.c) - fixed same pass. (5) NITs still
      deferred-with-subsystems (unchanged): corpse_xname snprintf
      hardening, minsects producer+consumer pair, sliparm/breakarm
      druid_form/vampire_form early-returns, MUSE_CAMERA drow
      clause, bad_rock tree-passing exemption.
      NEW FOLLOW-UP (from the fixup-4 sibling audit):
      wielding_artifact PARITY SWEEP - branch retains ~14
      u_wield_art sites where master is offhand-aware
      (attrib.c Ogresmasher/Giantslayer STR-CHA artifact blocks,
      pray.c crowning in_hand chain, sit.c Magicbane throne,
      hack.c Sting warning glow, mondata.c Excalibur, trap.c
      Magicbane/Sting, wield.c), and master's monster-side
      mon_wielding_artifact() helper is not yet ported (worn.c/
      sit.c/trap.c consumers). These ride with their owning
      subsystems (artifact internals, prayer, attrib, Sting glow)
      or one dedicated pass - do not convert piecemeal. apply.c
      Snickersnee sites are a 5.0-upstream feature (distance free
      hit; master has no counterpart, obj==uwep is its design) -
      NOT part of the gap. COORDINATOR DECISIONS OPEN
      (from port report): zombie_maker semantics kept 5.0-balanced
      vs master's bare racial_zombie (resolve with zombie
      subsystem); corpse_to_glyph racial bodies? (statues ARE
      racial now); Izchak transform protection needs an owning
      item (suggest shk B2); mon_prop() sites ported as racial-
      half only (upgrade when monster-intrinsics lands); end.c
      grave-arise master extras (VAMPIRE_MAGE/Nazgul/larva/lich)
      = non-racial parity pass. Deferral map (per-file detail in
      B1 port report, session 2026-07-13): player races (eat/
      attrib/cmd/spell/polyself/do_wear/youprop/hack body-parts/
      light/timeout/zap blindingflash/monmove auras), zombie
      subsystem (mm zombie rules, zombify, sickness, resists_sick
      family), bane-aura moveloop (allmain), claws family
      (has_claws), weapon-selection architecture (would_prefer_*/
      find_silver), hellfire (mcastu), steed/barding (steed.c
      gates), Mine Town invasion (sounds), peaceful_to_glyph
      (Phase 4 - no 5.0 peaceful glyph class).
      SLICE B2 PORTED + REVIEWED 2026-07-14/15 (subagent-ported,
      reviewer CONDITIONAL PASS, all blockers+concerns fixed by
      coordinator, full make clean rebuild zero-warning; NOT yet
      committed - rides with the B1 fixups above as one commit,
      files overlap in makemon.c/mondata.h). Contents: makemon.c
      m_initweap/m_initinv full racial-gear rework + mplayer kit
      system (struct trobj -> decl.h shared u_init.c/makemon.c, 31
      kit arrays de-static'd const, ini_mon_inv re-expresses
      master runtime kit mutations as const _0/_1 variants with
      5.0 trquan_min/max - reviewer verified every quantity vs
      master rn1 calls; setup_mon_inventory() new public helper),
      maybe_arm_silver + Medusa m_initthrow, mongets tail rework
      (spear rne(2) stacking, prince/lord gear-quality tiers,
      Medusa quality, maybe_arm_silver hook; reviewer CONCERN -
      was initially thrown-ammo-only); priest.c priestini +
      mk_roamer discard_minvent+setup_mon_inventory rebuilds
      (reviewer: lifecycle CLEAN - extract_from_minvent clears
      worn masks/mw/lights), drow temple darkening, sanctum-Amulet
      Infidel gate; worn.c find_mac racial-AC block verbatim +
      m_dowear racial gates + racial_exception + pick-loop
      mon_hates_material/W_AMUL/W_ARMS bracers/W_ARMG +
      mon_adjust_speed sewage cases (inert) + BUG-1323
      check_gear_next_turn re-arms x2 (reviewer CONCERN); shk.c
      shk_racial_adjustments/shk_stat_priced verbatim, get_cost
      materials->dunce->racial->charisma order (racial_discount
      suppresses high-CHA tiers), set_cost reciprocal + sell<=buy
      clamp + globby goto-end short-circuit + shkp guard (reviewer
      CONCERN+NIT); shknam.c shkinit racial shk per shop class +
      apply_race + racial/Izchak gear + nameshk r_data gender x2 -
      x_monnam racial shk naming now LIVE (light-up traced);
      obj.h gear macros = fork-point token sets (verified by
      token-set diff) + is_bracer; mondata.h +8 predicates;
      dat/Ran-goal.lua Longbow floor object retired (Scorpius
      carries it, crossbow for crossbow races).
      REVIEWER BLOCKERS FIXED: demon-bribe cluster - branch/5.0
      is_dprince vs master is_dlord (EvilHack re-tiered archdevils:
      Geryon/Dispater/Baalzebub/Mephistopheles = M2_LORD+MS_BRIBE,
      Asmodeus = M2_PRINCE+M2_HOSTILE, so the 5.0 gate caught
      exactly the wrong monster) at makemon bribe-genesis + minion
      demon_talk reveal; Hammer of the Gods clause at both artifact
      gates + Race_if(PM_AASIMAR) spawn-hostile clause. Coordinator
      sibling sweep added the monmove Demonic-Blackmail
      PM_PRISON_GUARD exclusion (prison guards share MS_BRIBE on
      the branch roster and must not blackmail). DEFERRED from the
      bribe cluster to demon-lord behaviors subsystem: master
      STRAT_APPEARMSG bribe one-shot rework (monmove whisper gated
      on STRAT_APPEARMSG, demon_talk entry clear, mnexto/rloc
      peaceful-MS_BRIBE keep-flag carve-out - branch is on the
      vanilla flow, teleport.c rloc_to_core owns the one-chance
      clear in 5.0).
      OPEN MAINTAINER DECISIONS (Slice B2): (1) master m_initinv
      soldier gate `ptr != PM_SOLDIER` = verified mechanical slip
      in 81102642b (every sibling hunk converted is_soldier() to
      ==; contradicts its own comment; non-soldiers skip the
      magic/gold tail 12/13 on live master since 2021-02) - branch
      keeps upstream `==`; MASTER BACKPORT CANDIDATE #4. (2)
      elf/hobbit ELVEN_CHAIN_MAIL: master's materials commit
      03e87e1bd removed every civilian grant but kept the
      m_carrying mithril upgrade = dead code; ported master
      exactly (elves/hobbits get no mail); restore a mongets
      before each upgrade if the grant was intended to survive.
      (3) angel weapon: kept 5.0's rn2(3) sword-bias + mace +3..+6
      compensation over master's rn2(2)+flat-spe (upstream balance
      improvement); master's Demonbane gate/SHIELD_OF_LIGHT/Saint
      Michael/astral ring ported on top.
      REVIEWER NITS RECORDED: vampire-monk book + Barbarian_0/_1 +
      D_weapon const splits are deliberate fixes of master's
      sticky kit-array mutations (RNG-observable delta, cleaner);
      W_ARMC size gate uses mon->data msize vs cloak gate's
      r_data (racial polish pass); master m_dowear_type
      shield-forces-mw2-drop block absent - safe, nothing writes
      mw2 yet, land with dual-wield + shadow-dragon wear-message
      variant; purple-rain insurance (astral RIN_SLOW_DIGESTION
      grants at priest/angel sites) is INERT until monster
      ring-wearing lands - the wear pass AND master's mhitm
      mon_prop(SLOW_DIGESTION) AD_DGST expel check must land
      together (record ON the ring-wearing deferral, else Astral
      priests keep getting purple-worm-eaten); set_cost globby
      accumulates a dead materials multiplier pre-goto (master
      parity, harmless). B2 deferral map (per-file detail in B2
      port report): Goblin Town gates (makemon/shknam
      TODO-EVILHACK markers), create_oprop sites (oprops = next
      big unlock; direct ITEM_* assignments already ported and
      saved), m_stash_items + container AI (WHEN PORTED it must
      exclude BAG_OF_TREASURE and BAG_OF_DEVOURING explicitly -
      m_stash_items runs at creation, so Croesus would otherwise
      stash straight into his own guaranteed bag; noted by the
      bags slice 2026-07-26), setmnotwielded2
      dual-wield drop, mon_bimanual, big_wings W_ARM, monster
      ring-wearing (extra_pref rework, W_RINGL/R, Nazgul ring,
      cursed_glove gating), armor_bonus/material_bonus AC.
      SLICE B WIZMODE BATTERY RUN 2026-07-15 (maintainer-approved,
      4 wizard games on the fresh install, sanity_check ON every
      turn): PANICLOG EMPTY, zero crashes/impossibles. PASSED
      LIVE: (4) archdevil bribes END-TO-END - Geryon+Dispater
      spawn invisible+peaceful, teleport-to-hero, "demands 3100
      zorkmids", pay->vanishes laughing; broke hero ($0) ->
      hostile per vanilla no-gold path; Asmodeus spawns
      hostile+visible (gdb mpeaceful=0); Hammer-wielded genesis
      -> bribeable lord born hostile (gdb, still minvis=1 per
      master). (5) bane taming - Hammer/vampire-lord (scroll
      no-ops, gdb peaceful=0 tame=0; control bare-handed: same
      scroll tames mtame=5), Dragonbane GLOVES/red dragon no-op,
      One Ring/WRAITH no-op + ring-off control tames (Nazgul
      first attempt was non-discriminating - covetous gate;
      maintainer caught it, re-run on plain wraith). (1)+(6)
      racial gear - "An orcish soldier" (erac orc + orcish
      helm/spear/dagger/chain), human sergeant (C-rations x2,
      splint mail, small shield, helmet, mace), mplayer
      "Richard the Raider" barbarian (BATTLE_AXE variant kit,
      ring mail, oilskin-sack m_lev>10 container, enchanted
      level-kit gear), "Kevin" monk rolled CENTAUR race (apples/
      oranges/fortune cookies/potions-of-healing kit, spellbook,
      robe worn, gloves worn) - christening + racial naming +
      trquan stacks all correct by gdb minvent dump. (2) Minetown
      temple - GIANT aligned priest (epri+erac straddle) with
      racial rebuild gear: silver mace +2, large shield +2 worn,
      healing books, gold. (3) Minetown shops - 4 shks ALL erac
      (3 human + 1 gnome), x_monnam racial branch LIVE by direct
      gdb call ("Nenagh the human shopkeeper"), book buy 300
      (CHA-6 x1.5 exact) / sell-back 100 (base/2, no high-CHA) =
      clamp holds, no profit. (7) drow webs - drow Wizard hero on
      web: "There is a spider web here." + u.utrap==0 stays 0;
      non-drow pet pseudodragon stuck in same web (normal path
      intact). (8) Ranger goal via #wizloaddes Ran-goal.lua - NO
      floor artifact (objlist scan), Scorpius carries BOW
      oartifact=ART_LONGBOW_OF_DIANA blessed. (10) save/reload
      full cycle + BONES round-trip (die in Minetown, save bones,
      new game, "Get bones? y") - all 5 erac overlays (gnome shk
      258/0x8, giant priest 263/0x20, 3 human) byte-identical
      through BOTH serialization paths. (9) gear churn - wear
      paths exercised at genesis across all sampled monsters;
      worn masks correct by gdb.
      NOT LIVE-TESTED (static/reviewer verification only, revisit
      opportunistically): drow temple darkening (needs random
      drow temple-priest roll), gnome INT-tier + nymph CHA-tier
      quotes + Izchak carve-out live pricing, maybe_arm_silver
      ~1/5 silver vs Infidel/Vampire hero (needs such a hero +
      spawn sampling), Crossbow of Carl variant (needs gnome/
      hobbit hero), m_dowear_type mfrozen re-arm corner (not
      deterministically stageable via tmux), glob sell pricing
      (needs food shop + glob), sfctool convert cycle (binary
      link broken pre-existing until erid/emsp serialize).
      Wizmode battery notes: trap wishes work ("web") but fail on
      doorways/stairs (trap-cover model); genesis Geryon/Dispater
      etc. needs force-confirm 'y'; farlook via tmux races game
      input - use gdb x_monnam/field dumps instead; wizkill
      picker accepts ',' select; lava suicide = wish "molten
      lava" + Die? y (a space answers default 'n' and rlocs you).
      Dev reference save NOT regenerated (battery save was
      consumed by the restore test; make one at next session
      start if needed).
      ORIGINAL battery list (kept for the not-yet-covered
      details; ASK MAINTAINER before any launch; #levelchange 30
      FIRST):
      (1) mplayer roles at m_lev 1/10/20+ incl. giant/tortle/
      draugr/vampire overlays - kit contents, YA 26-45 stacks,
      draugr monk bookless, Infidel cursed gear + fake Amulet,
      wizard suit never gray-scaled, lev-20 amulet/ring/artifact
      loop, watch paniclog for worn_slot/objlist; (2) temple
      sweep - Minetown priest (epri+erac straddle), drow priest
      dark temple (darkness confined to temple), giant priest
      gear, sanctum HP with/without Infidel hero, Astral priests,
      mk_roamer genesis churn (extract_nobj panics); (3) shop
      sweep all classes - racial shk distribution + naming,
      Izchak human/kit/no-racial-pricing, drow shop darkness,
      gnome INT + nymph CHA tiers, same-race discount + CHA>18
      non-stacking, buy-sell-back no-profit incl. stacks AND
      globs; (4) Gehennom archdevils - Geryon/Dispater/Baalzebub/
      Mephistopheles spawn invisible+peaceful awaiting bribe,
      Asmodeus hostile, demon_talk with Excalibur/Demonbane/
      Hammer angers, aasimar hero spawns them hostile, peaceful
      prison guard does NOT blackmail; (5) tamedog every bane
      wielded/worn vs matching race incl. Dragonbane gloves +
      One Ring vs Nazgul - no taming AND no peaceful flip;
      (6) Stronghold racial soldier spawns - weapons/ammo per
      rank, no impossible() from merc mac switch, spear stacks,
      ~1/5 silver weapons for Infidel/Vampire hero, prince/lord
      gear quality; (7) drow hero + monster web interplay,
      immune_to_trap pet-displacement path; (8) Ranger quest goal
      - no floor bow, Scorpius carries Longbow (crossbow for
      gnome/hobbit heroes), artifact one-shot; (9) monster gear
      churn - mixed amulets/bracers/two-handers wear priority, no
      swap loop, frozen mid-donning monster resumes equipping;
      (10) save/reload + bones round-trip over racial shk/priest
      levels, sfctool convert cycle, newcham poly churn; + B1
      recs: Sting glow-count vs racial orc + druid-form monster,
      Werebane vs werewolf taming, racial tin/sacrifice,
      vorpal-behead troll corpse revival, yellow light vs drow
      both directions.
- [x] Forging (+ FORGE nhl binding for .des->lua levels)
      PORTED + REVIEWED 2026-07-15 (subagent-ported 50 files
      +1935/-154, reviewer CONDITIONAL PASS -> blocker + concerns
      fixed by coordinator same day), COMMITTED as 8541bacb0
      (52 files +2187/-160 incl. fixes + this record; bundle
      evilhack-50_2026-07-15d cut+verified at that head; this
      status-line edit + battery record ride with the next
      commit). WIZMODE BATTERY RUN + PASSED 2026-07-15
      (maintainer-approved; 4 wizard games on the fresh install,
      sanity_check ON every turn; PANICLOG EMPTY, zero crashes/
      impossibles). PASSED LIVE: (1) terrain wish "A forge.",
      orange '{', lookat, overview count; (2) fusion trident =
      scimitar+spear via the 5.0 menu/getobj flow, components
      consumed, hammer gate enforced ("You'll need a blacksmith
      hammer"); (3) artifact fusion Dichotomy = Fire Brand +
      Frost Brand -> livelog "used a forge to create Dichotomy",
      forgedarti=1 AND artitouch=1 by gdb, #conduct line
      disappears once broken (master shape), coolforge stress
      fired ("The lava in the forge cools and solidifies");
      (4) doaffixgem: 3 shatter failures with gem staying unID,
      then success ("glows brilliantly") + makeknown on the
      remaining stack; dilithium correctly grants no oprop
      (enchant-bonus gem); (5) dipforge potion "boils and
      explodes"; drinkforge lethal sans fire res, killer
      "killed by drinking molten lava"; (6) kick ouch, thrown
      dagger rests ON the forge, sit burn d(4,4), dig ->
      breakforge "splits in two as molten lava rushes forth" ->
      LAVAPOOL + hero fell in (gear lava_damage live); (7)
      blowupforge via invocation-item fusion ("You can't forge
      such a thing!"), Bell of Opening survived via obj_resists,
      tombstone + xlog death = "burned by an exploding forge"
      (the killer-branch fix live); (8) hammer apply -> forge
      menu directly; (9) BONES POLICY by gdb scan of loaded
      bones: Eye of Vecna arrived STRIPPED (oartifact=0, oname
      freed), blacksmith hammer arrived as war hammer material
      IRON (resetobjs rewrite + set_material); return-to-owner
      arm reviewer-verified only; (10) des.feature("forge")
      engine-load: both lua forges LIT=1 (the sel_set_feature
      fix), nforges=2, lua object forged_qual=3 + affixed_gem=
      540 applied to the floor sword; (11) PREFIX regression:
      89-char prefix trident renders in inventory AND in the
      end-of-game disclosure, no crash; (12) save/reload:
      forge+lava terrain, trident qual/gem/erosion/spe,
      Dichotomy, forgedarti, nforges all byte-intact; kept-save
      double-restore OK. XLOG both directions: conduct bit 15
      set when intact + token never_forged_an_artifact present;
      bit clear + token absent after forging. gdb spot checks:
      getprice 5 -> 85 with FQ_EXCEPTIONAL (x3 factor visible),
      affixed dilithium otyp 540, FQ-aware dmgval callable.
      NOT LIVE-TESTED (static/reviewer verification only):
      FQ to-hit/damage deltas in real combat (dice noise),
      two-hander shatter + breaktest odds (statistical),
      scatter/monster-thrown forge stops (need staging), forge
      ambient sound (random tick), mkforge natural 1/40 spawn.
      Battery driving notes: terrain wish over the start
      upstairs leaves the 5.0 stairs-list entry live (lookat
      says forge, dig says "stairs are too hard" - master-parity
      wizmode quirk, harmless); restore queues "keep save file?"
      which eats blind keystrokes - capture first; a Dlvl with
      the Mines branch stairs refuses bones (Is_branchlev
      upstream rule) - use Dlvl 4; the wizard Die? prompt
      defaults n on space, answer y deliberately.
      Contents: FORGE terrain end-to-end (rm.h FORGE=28
      inserted before FOUNTAIN - FOUNTAIN..CLOUD/MAX_TYPE/
      MATCH_WALL/xFLOOR.. all shifted +1, range-macro membership
      audited, DEV SAVES/BONES INVALIDATED; defsym.h S_forge PCHAR
      37 inserted before S_fountain with explicit-index renumber,
      '{' CLR_ORANGE = map_color_256to16(202) per the HI_ alias
      precedent; levelflags nforges; back_to_glyph/botl
      terrain_descr/cmd levltyp/pager/invent dfeature/overview
      mapseen 2-bit nforge/sounds; mklev mkforge 1/40 between
      mksink and mkaltar; wizard terrain wish); fountain.c core
      (dipforge + fusions[] 96 recipes + artifusions[] 13 -
      both mechanically diffed byte-equivalent to master;
      get_forge_recipe, gem helpers, doforging via 5.0 menu/getobj
      API with oprops+gem-property transfer and material take-on
      LIVE, doaffixgem, breakforge/blowupforge/coolforge/
      drinkforge, dolavademon); #forge M('F') + herecmdmenu +
      domonability lava gremlin; forgedarti conduct (insight
      listing + xlog bit 15 + never_forged_an_artifact achieveX);
      FQ combat (find_roll_to_hit, dmgval clamp, two-hander
      shatter, breaktest 3/8) + ARM_BONUS +1/+2/-2 (SUPERSEDED
      2026-07-27: the macro is gone, master's armor_bonus() is the
      single source, and it carries the material term the macro
      never had - see the glass/blocker row); shk
      getprice quality factor; objnam quality/gem naming + wish
      parse + PREFIX 148; mkobj may_generate_quality (init-only);
      artifact.c mk_particular_artifact/is_forged_artifact/
      non_wishable_artifact + mk_artifact forged_qual/affixed_gem
      zeroing (un-defers the oprops item); bones BLACKSMITH_HAMMER
      rewrite + set_material + the non-wishable-artifact bones
      strip/return-to-owner policy (in_owner_lair/
      find_owner_on_level; Butcher hellb_level lair TODO-EVILHACK
      until Ice Queen topology ports); sp_lev feature("forge") +
      object forged_qual=/affixed_gem= params = THE FORGE NHL
      BINDING (Phase 3 unblocked); dat/symbols S_forge for Blank
      + IBMgraphics (NHAccess/UTF8graphics symsets do not exist
      on branch; U+2320 intent -> Phase 4 utf8map).
      REVIEWER BLOCKER FIXED: objnam PREFIX stayed 80 while the
      port added "exceptional "/"<gem>-studded " prefix words -
      doname prefix[] stack overflow on a fully-ID'd eroded
      gem-affixed quality weapon; master's 148 + worst-case
      comment ported. CONCERNS FIXED: explode.c FORGE_EXPLODE
      killer branch (self-death now "killed by an exploding
      forge"); scatter() forge stop; mthrowu MT_FLIGHTCHECK forge
      stop (master parenthesization kept - forge stops in pre
      phase too); COORDINATOR-FOUND sibling master zap.c bhit
      IS_SINK||IS_FORGE flight stop (reviewer missed it - claimed
      master zap.c had no forge sites); bones non-wishable policy
      (above); sel_set_feature forces lit=1 for FORGE = master
      SET_TYPLIT parity (the ONE placement path master forced was
      the one path the port missed; set_levltyp forcing kept).
      GOTCHA RECONFIRMED: include/onames.h is a DECOMMISSIONED
      makedefs reference file (5.0 header says "no longer used in
      the build") that can sit stale in the worktree - the real
      ART_ enum comes from artilist.h bn column via hack.h
      ARTI_ENUM (ART_MAGIC_8_BALL etc., exactly the Phase 1
      tracker map); never grep onames.h for branch names.
      DELIBERATE DEVIATIONS (reviewer-verified executed): binary
      Fire_resistance for master's percent-resist at 5 sites
      (revisit when percent-resist ports); destroy_one_item as
      wrapper over 5.0 destroyable/maybe_destroy_item with the
      forge min-1 guarantee inside; no mhitu FQ damage duplicate
      (master double-counted vs dmgval); FORGE_EXPLODE =
      MAXOCLASSES+4; ZT_FORGE_FIRE local define; illogical-fusion
      blowup returns ECMD_TIME (master 0 - blowing up your gear
      costs the turn); "no forge here" fixes master's "no
      fountain here" copy-paste; quality roll init=TRUE-only.
      MASTER BUGS FLAGGED (frozen-unfixed, archive entries when
      convenient): shatter `!= FQ_SUPERIOR && rn2(2)` slip
      (ported verbatim); hitmu FQ double-count (not ported);
      domonability fountain message (fixed on branch).
      DEFERRED (item -> subsystem): break_glass_obj + blocker/
      deflection FQ_INFERIOR melee fall-apart channel (glass/
      blocker combat slice - EXPLICIT RULING: FQ_INFERIOR clause
      rides with it), forge tile art in win/share/other.txt
      (Phase 4 - NOTE S_forge insertion shifts every cmap tile
      after sink; master tile exists to copy), pager forge-recipe
      /journal lookup subsystem (extended obj/artifact lookup UI;
      get_forge_recipe exported ready), percent-resist sites,
      hack_artifacts non-forge retargets (Dirge/Priest/Caveman -
      artifact/crowning slice), botl_hitbonus FQ display
      (SCORE_ON_BOTL extension), monk qi weapon-break FQ guards,
      pray.c sacrifice-gift FQ sites (sacrifice rework),
      Blacksmith's Cookbook tribute passages (Phase 3 dat),
      MAGIC_KEY bones rewrite (magic chests). NITs left as
      master/upstream parity: mkforge transient nforges overcount
      (upstream mkfount idiom, self-heals), sp_lev %4 negative
      lua value clamp, getprice erosion-pricing block absent
      (PRE-EXISTING port gap, separate decision), TOOL_CLASS
      add_erosion_words inert until barding erosion_matters.
      Remaining for [x]: maintainer commit + wizmode battery
      (list below) + xlog token style ruling (kept master's
      long never_forged_an_artifact vs branch short-style).
      BATTERY LIST (ASK MAINTAINER before launch; #levelchange
      30 first; sanity_check on): (1) wish "forge" terrain ->
      lit tile, '{' orange, lookat/overview/^o counts, level
      sound; (2) #forge fusion recipe (two inputs -> product,
      FQ roll by gdb, oprops/material carry); (3) artifact
      fusion (artifusions pair + gold cost) -> livelog,
      forgedarti conduct broken in #conduct + xlog bit,
      touch_artifact; (4) doaffixgem gem onto weapon (property
      grant, shatter path, enchant threshold via scroll);
      (5) dipforge potion destroyed (min-1 guarantee), drinkforge
      fire-res vs not, dolavademon; (6) kick forge (ouch/splash),
      throw over forge (object stops - zap bhit + mthrowu +
      scatter fixes), dig -> breakforge -> LAVAPOOL, sit burn;
      (7) lava gremlin #monster at forge -> blowupforge,
      FORGE_EXPLODE death line "killed by an exploding forge"
      (Luck<0 dip case 22); (8) BLACKSMITH_HAMMER apply ->
      wield+forge prompt, recharge case, bones round-trip ->
      war hammer iron; (9) Sword of Annihilation (or any
      SPFX_FORGED arti) in bones -> stripped on reload; Magic
      8-Ball on Oracle level bones -> survives (lair), off-level
      -> returned to Oracle if present else stripped; (10)
      #wizloaddes lua with feature("forge") + obj forged_qual=3
      affixed_gem= -> forge LIT, nforges correct after coolforge,
      object quality/gem live; (11) PREFIX regression: fully-ID
      thoroughly rusty thoroughly corroded +7 exceptional
      gem-studded long sword in inventory display (pre-fix =
      stack smash; ASan build ideal); (12) save/reload forge
      level + FQ/gem items intact; old dev save REFUSED cleanly.
- [x] Magic chests (+ MAGIC_CHEST nhl binding, crystal chest obj)
      PORTED + REVIEWED 2026-07-15 (subagent-ported ~40 files,
      reviewer CONDITIONAL PASS with NO blockers -> all 3 concerns
      + 4 nits coordinator-resolved same day; commit + battery
      pending). Contents: MAGIC_CHEST terrain end-to-end (rm.h
      =29 inserted after FORGE - FOUNTAIN..CLOUD/MAX_TYPE/xFLOOR
      shifted AGAIN, dev saves invalidated; defsym.h S_magic_chest
      PCHAR 38 '(' CLR_BRIGHT_MAGENTA, indices renumbered;
      levelflags nmagicchests; back_to_glyph/type_names/botl
      "Magic-chest"/cmd levltyp + herecmd MCMD_LOOT + mouse-click
      loot/pager lookat incl. lock state/invent dfeature/mapseen
      2-bit + overview/wizcmds legend + stats chain; dat/symbols
      Blank + RogueEpyx; windows.c htmlsym U+2302); THE SHARED
      CHAIN gm.mchest = HIDDEN_CHEST head object, contents in its
      cobj (decl.h instance_globals_m after migrating_objs,
      g_init_m UNDEFINED_PTR, allmain init_mchest() new-game
      create, save/restore at the invent->mchest->ball&chain slot
      = master stream order, freedynamicdata, obj_extract_self
      OBJ_MCHEST, unsplitobj exclusion, objlist_sanity,
      obj_is_local FALSE = global timers relink through find_oid
      recursion, clear_bypass incl. contents (improves on
      master's head-only), kill_eggs, find_qarti bit); mechanics
      (lock.c pick_lock chain splice - reviewer verified all 3
      continuation points + at-most-once transition; MAGIC_KEY
      odds 75/70+DEX; crystal=artifact-only + hidden=artifact-or-
      MAGIC_KEY refusals; doforce refusals; pickup.c container_at
      /doloot splices/do_loot_cont AUTOUNLOCK re-expression/
      reverse_loot/dotip flows; zap probe-resist no-learn +
      teleport resist + revive block + zap_updown boxlock +
      zap_map lateral toggle - double-fire reviewer-ruled-out;
      dig/kick/sit/quake-immune/fire-immune/spoteffects/meatbox/
      dogmove-skip; detect trapped_chest_at + observe_recursively
      crystal guard + object_detect single walk; objnam terrain
      wish + wizmode "magic chest object"->HIDDEN_CHEST; mkobj
      mkbox_cnts + mksobj init cases); CRYSTAL CHEST = own-cobj
      container (reviewer verified the crux vs master: ONLY the
      terrain tile reaches the shared chain); sp_lev
      feature("magic chest") binding (Phase 3: 24 master .des
      levels use it); bones MAGIC_KEY->SKELETON_KEY +
      set_material BONE (master-exact; forging-slice TODO
      resolved). REVIEWER CONCERNS ALL RESOLVED: (1) iron-safe
      deferral economy delta - crystal chests were substituting
      for IRON_SAFE at every depth >15 (permanently sealed
      loot for non-artifact heroes at 20% of deep box rolls);
      FIXED with plain-CHEST stand-in arms at all three master
      safe sites (mklev fill_ordinary_room, mkroom COURT coffers
      100%-at->15 arm, mkroom BARRACKS payroll 1/3 arm + master's
      LARGE_BOX->CHEST payroll upgrade), crystal chests back to
      master's depth 11-15 window, TODO markers kept; (2)
      pick_lock untrap-offer leak on the never-trappable chest
      (turn-waster) - skipped for HIDDEN_CHEST; (3) autokey had
      no MAGIC_KEY channel - master's mkey/amkey tracking +
      top-priority return + rogue MKoT promotion ported into the
      5.0 body (NAMING HAZARD note: 5.0 is_magic_key() = Master
      Key of Thievery, NOT the MAGIC_KEY otyp - master renamed
      its own to is_roguish_key; watch this in every future
      lock.c port). NITS RESOLVED: IRON_SAFE wish stub (was
      unlocked/empty) - mksobj locked+mkbox_cnts(10) init added,
      stethoscope-only rules stay deferred w/ TODO; magic-
      detection deferral marker added in detect.c (SCR_MAGIC_
      DETECTION effect unported; master magic_detect walks the
      chain + 3 crystal guards); cursed-tool-break marker in
      lock.c (MAGIC_KEY exemption must ride when it ports);
      tile art -> Phase 4 (S_magic_chest shifts cmap tiles
      after forge - BOTH new terrains need one tiles pass).
      REVIEWER POSITIVES: objnam parse block fixes a LATENT
      MASTER BUG (master's bare else sent wizard "bearded axe"
      wishes to the magic-chest terrain path; frozen-unfixed);
      wizterrainwish flags=0 + counter guard improve on master.
      DEFERRED (item -> subsystem): monster container looting
      (mpickstuff/likes_contents/m_move mchest splice/muse
      MAGIC_KEY pickup - one TODO in mon.c; WHEN PORTED both the
      mpickstuff arm and m_move's `|| Is_container(otmp)` clause
      must exclude BAG_OF_TREASURE and BAG_OF_DEVOURING, since a
      cursed treasure bag weighs its contents x10 and so makes
      !can_carry true, which is exactly the crack-it-open case -
      TODO-EVILHACK markers left at both sites by the bags slice
      2026-07-26), iron safes
      (generation arms + stethoscope cracking + lock_action
      flavor), magic detection (marker above), cursed-tool
      break, Is_container range to BAG_OF_DEVOURING (bags
      slice), tile art (Phase 4). OPEN DECISION recorded:
      tip-into keeps 5.0's carried-only target design + the
      chest as the sole floor-side target (master offered all
      floor containers; revisit only if players miss it).
      COMMITTED as 366e7ac34 (45 files +802/-154; bundle
      evilhack-50_2026-07-15e cut+verified). WIZMODE BATTERY RUN
      2026-07-15 + PASSED WITH TWO REAL CATCHES (both fixed same
      session, fixup commit pending with this record):
      BUG-1334 = ERAC B2 REGRESSION, giant-race mercenary
      crashed level generation (m_initinv add_ac macro read the
      uninitialized otmp when the round-1 armor chain skips
      every arm for giants; master's int-returning mongets shape
      cannot crash; caught by depth-20 wizmakemap sweep, core
      analyzed, fixed = initialize otmp, verified by 20 clean
      regens + genesis'd giant soldier with armor-less kit);
      BUG-1335 = 5.0-new timer sanity walker fired false
      impossibles on timered chest contents (OBJ_MCHEST has no
      map location by design; added to the walker's existing
      skip clause; corpse rot timer verified identical across
      save/reload). BATTERY PASSES: (1) shared chain cross-level
      stash/retrieve incl. lock state traveling with the global
      object; (2) timer relink through save/reload (newt corpse
      timeout byte-identical; NOTE lichen corpses have no rot
      timer - do not use them for timer tests); (3) loot menu
      chest-first with content count + two-container menu; (4)
      autounlock: locked chest w/ magic key -> NO trap prompt
      (concern-2 fix) straight to key offer; ordinary chest
      control DOES get the trap prompt; autokey offers the
      magic key as only tool on ordinary locks (concern-3 fix);
      (5) crystal chest: kick THUD no-break, #force refusal,
      probe resist message, teleport-zap no-op, mundane-key
      refusal (magic key correctly does NOT bypass) vs
      gdb-oartifact-flagged key unlocks, loot = OWN contents
      not the shared chain (the crux); (6) wand of locking/
      opening down-zaps toggle (Klunk/Klick x2 = tile chest +
      floor box), lateral zap across the tile single-toggles;
      (7) #tip on tile "bolted down!", locked-chest tip-into
      refused, unlocked tip-into lands contents in the shared
      chain via the target menu; (8) terrain suite: sit
      "surprisingly comfortable", kick THUNK+ouch, dig "magic
      chest is unaffected", drum earthquake pits everywhere +
      chest survives; (9) bones: magic key -> skeleton key of
      bone (gdb otyp 268 mat bone); (10) object-detection
      potion senses chest contents cleanly (NOTE "object
      detection" is a POTION - a "scroll of object detection"
      wish correctly yields a random scroll, not a parse bug);
      (11) depth sweep: Dlvl 12 + 20 wizmakemap samples show
      plain boxes/chests only, zero iron safes, zero crystal
      chests at 20 (stand-in fix live; crystal at 11-15 is a
      rare roll, absence in an 8-map sample is expected); (12)
      wishes: "bearded axe" -> "Nothing fitting" (master's
      terrain-hijack bug fixed; full "dwarvish bearded axe"
      works), "magic chest object" -> unlocked HIDDEN_CHEST,
      "iron safe" -> born locked with 8 rolled contents (NIT-7
      fix live). PANICLOG at session end: EMPTY (the single
      pre-fix BUG-1335 entry was wiped by the reinstall; both
      fixed binaries produced zero entries). Battery driving
      notes: keep-save-file prompt after restore eats blind
      keys (capture first); cross-role quest artifacts (MKoT)
      blast AND evade non-role heroes - gdb oartifact flag on a
      mundane key is the deterministic artifact-unlock lever;
      make install DELETES var incl. paniclog (record baselines
      before reinstalling mid-battery).
      ORIGINAL reviewer battery list (kept for reference):
      (1) shared chain cross-level: stash on level A via wished
      terrain chest + magic key, retrieve on level B; (2) timer
      relink: fresh corpse in chest -> save/reload + level
      change -> rots on schedule, no find_oid panic; (3) #loot
      menu chest-first + sequential splice walk with 2 floor
      boxes; (4) autounlock: apply-key+untrap set, loot locked
      chest w/ magic key -> NO trap prompt (concern-2 fix),
      unlock prompt fires; autokey: magic key as only tool ->
      offered on ordinary locked chest (concern-3 fix); (5)
      crystal chest: kick no-break, #force refusal, probe
      resists + wand stays unID, poly/tele zaps no-op, artifact
      unlock, loot = OWN contents not shared; (6) wand of
      opening/locking down-zap toggles chest lock; lateral zap
      across tile single-toggles; (7) #tip on tile "bolted
      down!"; tip carried sack into chest tile target ->
      contents land in shared chain; (8) terrain suite: dig
      unaffected, earthquake survives, kick THUNK+ouch, sit
      comfortable, farlook lock state, overview counts; (9)
      bones: die w/ magic key -> next game skeleton key of
      bone; (10) object detection with items only in chest,
      on- and off-tile; (11) depth sweep #wizmakemap at 12 +
      20: crystal chests 11-15 only, CHEST stand-ins deeper,
      court coffers, no safes; (12) wish checks: non-wiz
      "magic chest" rejected, wiz terrain + "magic chest
      object", "bearded axe" -> axe (parse regression), "iron
      safe" -> locked + 10 contents.
- [x] telekinesis.c + #tk command (pilot did the file; wire cmds)
      DONE 2026-07-16 (commit pending; port subagent + reviewer +
      coordinator fixes; clean rebuild zero-warning). Monster half
      m_tk_push_obj/m_dotelekinesis into telekinesis.c; mfind_target
      -> mthrowu.c; acceptable_pet_target -> dogmove.c (petstrat
      fields pre-existed from Phase 1 mextra.h - reads inert until
      #order ports); has_free_action (monst.h mon_resistancebits
      family) + ptr_resists_sick/resists_sick + is_support
      (mondata.h); mm_aggression un-static + 4 extern.h decls;
      mon_regen mtk_attk tick; dochug mind-flayer hook at master's
      position (after wield block); extcmdlist C('k') "telekinesis"
      AUTOCOMPLETE (^K was free); hero-side illithid gates RESOLVED
      (race live since Phase 1 roles port) + skill_init illithid
      P_TELEKINESIS unlock + give_may_advance_msg telekinetic arm +
      ECMD return conversion (OK/CANCEL/TIME). ADAPTATIONS:
      STRAT_GOALX/Y -> mtmp->mgoal.x/.y (5.0 dedicated coord field,
      strategy() maintains it); master mlined_up(mtmp, mtmp2, FALSE)
      -> branch m_lined_up(mtmp2, mtmp) REVERSED ARGS (target
      first); tbx/tby -> gt.tbx/gt.tby (contract: set on success,
      zeroed on NULL - future muse/monster-AI consumers rely on it);
      stoning killer via mons[].pmnames[NEUTRAL] (sex-split names).
      Reviewer CONDITIONAL PASS: BUG-1336 found+fixed on branch
      (m_tk_push_obj wakeup(target, FALSE) x2 - monster-caused hit
      blamed the hero via setmangry; master frozen-unfixed, archive
      entry); isok guard added on the mgoal m_at lookup
      (branch-idiom parity with monmove/wizard covetous readers);
      parity observation: dochug hook can return 0 with the caster
      dead (master-identical, traced harmless - end-of-pass
      dmonsfree + dochugw mdistu gate); reviewer's "port
      #wiztelekinesis" REJECTED - upstream's wiz_telekinesis already
      adopted (wizcmds.c). STILL DEFERRED with their subsystems:
      set_pet_killer credit (kill-attribution item below; two
      TODO-EVILHACK sites in telekinesis.c), dead-trees terrain
      distinction, master mlined_up friendly-fire walk, master
      monmove ranged-attack scan, muse/dogmove/weapon/mhitu
      mfind_target+acceptable_pet_target consumers. NOTE for pet
      slices: branch carries 5.0-NATIVE pet ranged AI
      (score_targ/best_target/pet_ranged_attk in dogmove.c) parallel
      to master's mfind_target pet path - RECONCILE per
      adopt-upstream-then-re-add rule.
      WIZMODE BATTERY PASSED 2026-07-16 (illithid Wizard XL30,
      sanity_check on all game, paniclog NEVER CREATED, save dir
      restored): skill_init unlock (#enhance telekinesis Unskilled
      0(20)); use_skill counter ticks on use; force-advance capped
      at Master; Unskilled = no menu/push-only/range 1/stack split;
      Master = push-pull menu + remote-pickup pull; helm +1 range
      boundary EXACT (6 in, 7 out of range); iron-helm block with
      material name; hero tk_push_mon + tk_impact wall slam; monster
      pull-you x2 (mhim renders both sexes; hurtle dist-1 stops
      adjacent); cooldown set 9/10 after actions and ticks exactly
      1/turn (gdb); dagger hurl at hero; potion hurl -> potionhit
      splash, destroyed obj clean; cockatrice-corpse hurl ->
      done(STONING) with killer string read live via gdb =
      "telekinetically hurled cockatrice corpse" KILLED_BY_AN;
      chain-hold blocks pull + consumes cooldown (unguarded set =
      master parity); BUG-1336 FIX VERIFIED (struck peaceful
      watchman stays peaceful=1, hero align record/abuse 0/0);
      tame flayer mfind_target + acceptable_pet_target pull of a
      hostile then melee (pull-to-prey combo); #wiztelekinesis
      works; save/reload keeps mtk_attk (8 -> 7, one legit
      save-turn tick). NOT live-run (statically + reviewer
      verified): covetous mgoal early-return, boulder push by
      master flayer, A_INT<6 gate. BATTERY-DRIVING GOTCHAS: gdb
      reads taken while a prompt is up (e.g. potion-naming docall)
      see MID-TURN state - the caller's cooldown line has not run
      yet; rloc_to does NOT validate terrain - placing a monster on
      a wall/stone tile silently kills its TK ray (ZAP_POS break)
      and mimics an AI bug; wizard #enhance 'y' force-advance is
      the fast lever for skill-gate testing; genesis'd master mind
      flayers drink !speed and brain-suck relentlessly - stage via
      gdb mcanmove=0/mfrozen, not in melee range.
- [x] Kill-attribution/livelog architecture (player_killed +
      pet_killer set/consume; memory kill-attribution-architecture)
      PORTED + REVIEWED + FIXED 2026-07-17 (port subagent 26 files
      +481/-116; reviewer CONDITIONAL PASS 0 blockers/2 concerns/2
      nits -> all coordinator-verified+fixed same day; zero-warning
      make clean + make all green). COMMITTED 2026-07-17 as
      692008bd7 (28 files +586/-128 incl. tracker + insight.c fix).
      WIZMODE BATTERY PASSED 2026-07-17 (maintainer-approved; 3
      games, sanity_check on; bundle evilhack-50_2026-07-17g cut+
      verified). BATTERY RECORD - livelog-proven: hero Medusa kill
      = SINGLE "killed Medusa" lltype=6 line (concern-1 fix live,
      no double), uevent.killed_medusa=1 + ACH_MEDU in uachieved[8];
      "pet dog killed Cerberus" lltype=6 via set_pet_killer ->
      m-prefix monkilled -> mondead consume -> logdeadmon pet arm
      (gdb-staged setter = producer-contract lever); Vlad m-killed
      IMMEDIATELY after = NO livelog + killed_vlad=1 quiet state =
      consume-on-entry staleness proof; Death nap "put Death down
      for a little nap" lltype=6 (concern-2 LL_ACHIEVE fix live),
      three revived-Death m-kills all silent; Croesus m-kill SILENT
      (5.0 would have logged "has been killed") then hero kill
      "killed Croesus (2nd time)" lltype=4 cadence; save/reload
      round-trip preserved cerberus/vlad bits with medusa=0;
      hero death string "killed by a hobbit's thrown elven dagger"
      (thitu weapon_killer possessive); hero-fault exploder chain
      live in game A (F-attack tame gas spore -> "You kill Medusa!"
      + pet-murder guilt line). PANICLOG: zero slice entries; one
      self-inflicted staging entry (rloc kitten onto an occupied
      tile -> placement detector; that game then lost to a SIGINT
      landing in a gdb-blocked call - both lessons in gameplay
      memory). RESIDUAL (not live-proven, code-verified by porter+
      reviewer): pet melee/wand producer sites in real combat (pet
      AI refuses boss targets - avoids petrify-risk Medusa, flees
      high-level Cerberus, ignores paralyzed monsters; hostiles
      prefer the hero over adjacent pets so popper staging fails),
      monster-fault exploder arm, Medusa pet/no-credit branches
      (proven on sibling arms), cloud creator_mid kill (region
      creator verified by code review only). Pet-AI target-scoring
      quirks discovered are PRE-EXISTING behavior, not slice bugs.
      STAGING RECIPES proven: genesis unique -> "Creating
      doppelganger instead; force X? [yn]" needs y (n leaves a
      shapeshifter fake - check cham on staged monsters); Death
      REVIVES every few turns and beelines the hero (m-wizkill per
      revival); set_pet_killer callable from batch gdb (pure
      setter) as the deterministic pet-credit lever.
      Landed: mon.c statics + set_pet_killer + mondead
      consume-on-entry (locals was_player_kill/pet_credit as FIRST
      statements) + monstone vamp_stone-fail/lifesaved clears +
      xkilled dispatch-point set + m_detach clear-if-equal x3 +
      corpse_chance swallowed-spore credit; ge.exploder/gt.thrower
      g-struct globals (transient, never saved) beside gb.buzzer;
      explode() tame-exploder credit + master -AD_RBRE no-mutation
      form (vanilla-5.0 bug, see upstream-reportable table) +
      mon_explodes set-AFTER-mondead ordering; region creator_mid
      threading (make/create_gas_cloud + selection variant + all 15
      callers; inside_gas_cloud find_mid credit; rest_regions
      zeroes - credit dropped across save BY DESIGN); zap.c buzzer
      consumer reads x4 (cancel_monst/disintegrate_mon/dobuzz/
      resist); mthrowu ohitmon magr param + potionhit set/clear
      wrap + thitu mon_killer_name/weapon_killer death naming +
      m_throw/thrwmu gt.thrower brackets incl. zero-damage spit
      projectiles (extended-breath deferral CLOSED); producer sweep
      at 5.0 dispatch homes (mdamagem central, mon_poly shock,
      do_stone_mon, displacem/gazemm/protector stone, passivemm
      assess_dmg, mbhitm x3 via otmp->ocarry tameness, annihilate,
      monmove psychic blast, eat_brains petrify, artifact Grimtooth
      disease, telekinesis x2) - ALL kill-attribution TODO-EVILHACK
      markers consumed; makemon boss-flag setters (iscerberus/
      isvecna/isvlad/istalgath/isgking/islucifer/ismichael - dead
      Phase 1 data model now live); logdeadmon reconciliation =
      5.0 cadence/shk/mrevived logic kept but attribution-driven
      (non-hero non-pet unique kills NO LONGER livelogged = master
      parity; 5.0's "has been killed" arm removed); 8 once-gate
      u.uevent bits appended (same storage words, sizeof unchanged;
      defeat_icequeen deliberately NOT added - Ice Queen slice).
      REVIEWER CONCERNS FIXED: (1) Medusa arm double/wrong-credit
      livelog - record_achievement's unconditional "killed Medusa"
      line claimed hero credit on pet and m-v-m kills; fixed via
      NEW insight.c record_achievement_quietly (record_achievement_
      core + quietly flag: state + sound, no livelog) - hero kill
      logs 5.0 achievement line, pet kill logs master's pet line
      with LL_ACHIEVE restored + quiet state, other causes quiet
      state only. (2) PM_DEATH nap arm was LL_UMONST-only while
      Famine/Pestilence keep LL_ACHIEVE via the generic arm's
      is_rider clause - nap arm now carries LL_ACHIEVE (branch
      rider policy + master text). MAINTAINER RATIFIED 2026-07-17:
      both livelog decisions kept as shipped (Medusa credit-only
      logging incl. silent non-hero causes; all Rider lines always
      LL_ACHIEVE), and the achieve2 question resolved = DEDICATED
      XLOG-PARITY SLICE (new Phase 2 item below). NOT ported (master defers too): DoT/delayed-petrify/
      gas-spore-chain-popper per-monster source attribution;
      hero-reflected beam kills stay unlogged BY DESIGN. DEFERRED
      with owners: trap.c nested trap-window buzzer save/zero/
      restore -> trap subsystem (MAGIC_BEAM_TRAP_SET type absent
      from branch trap.h, only the kit object exists); mb_trapped
      exploder bracket -> door-trap rework (no explode() call on
      branch); missmm blocker sear -> searing-deflection; AD_BHED/
      AD_DISN-melee/AD_CNCL wraps + dragon-scale passive arms ->
      monster attack-handler subsystems; mcastu castmm sites +
      buzzmm -> innate m-v-m casting (boss slice); MUSE stinking
      cloud -> muse offense; boss-death handlers (cerberusdead/
      vecnadead/vladdead/talgathdead/goblinkingdead/luciferdead/
      michaeldead/icequeenrevive) + Kathryn arms -> boss slice;
      mon_xkilled uexp arms -> monster-exp. NOTE: u.uevent boss
      bits = game-state source for the achieve2 xlog decision
      (topten add_achieveX + cmd.c disclosure NOT ported).
- [x] Alignment-abuse subsystem (tiers, #conduct API - see memory
      alignment-abuse-subsystem)
      DONE 2026-07-16 (commit pending; port subagent + reviewer
      CONDITIONAL PASS + coordinator fixes; clean rebuild pending
      below). PROVENANCE FINDING: upstream 5.0 itself added the
      u.ualign.abuse counter (unsigned POSITIVE magnitude) with
      erinyes scaling (adj_erinys) and HoOA fury summons (Meyer,
      2023-11) - the slice is adopt-upstream-base + re-add EvilHack
      extras; Phase 1 had landed only the EvilHack data-model extras
      (abuse_event ring, ever_abused, uaasimar_penalties), all dead
      until now. SIGN ADAPTATION: master stores abuse negative; all
      ported reads/assigns adapted to the branch's positive-unsigned
      convention (recorder API contract unchanged - producers pass
      negative penalties). Landed: 47-value ABUSE_* enum (align.h,
      append-only, saved as short); recorders + abuse_type_name +
      abuse_event_desc (pmnames[NEUTRAL], range-validated details);
      adjalign ever_abused + aasimar stub; setmangry rebuilt to
      master parity (accidental consume-on-entry, Elbereth gate,
      Kathryn/Bourbon/Ozzy cross-anger, Rogue/Convict carve-outs,
      priest LUCKMIN arm, chaotic-aware pen formula, Oracle/Charon
      mmove=18 chase); ga.accidental_bump one-shot flag +
      wakeup_accidental/setmangry_accidental + consumers
      (attack_checks glyph split, hurtle_step/mhurtle_step); xkilled
      reworked (scenario_penalty, A_NONE gates, uni_same, malign cap
      max(3*pen,-15)); priest_talk atonement (25k/50k per tier,
      tier-bottom landing, tier-scaled suggested donations,
      REFUSE_TITHE); insight.c enlightenment line + #conduct tiers +
      aasimar purity/lost-gifts (inert) + "Last alignment abuse
      events:" ring readout (ever_abused gate); restore stub call.
      Producers: 50 of 79 master sites ported (16 host files);
      29 DEFERRED with named subsystems - Moloch periodic offering
      (allmain), Grimtooth disease + Vecna invoke (artifact.c),
      aasimar helm explosion (do_wear) + forsake-deity (pray),
      Enchantress attack + Lolth deity-kill + Infidel good-deed
      (mon.c), Infidel cowardice (monmove), vampire-blood quaff
      (potion), real-Amulet deception (pray), quest betrayal
      (quest.c pissed_off), death-spell peaceful (spell.c),
      steed HAZ_FALL (is_open_air terrain), Druid woodland +
      Priest forbidden-weapon + tainted-weapon + P_THIEVERY
      pickpocket (uhitm), ray tree destruction (zap/hack) -
      full table in the port report. REVIEWER BLOCKERS FIXED:
      (1) museum-copy segfault guard restored on the xkilled
      ispriest arm (master ispriest && mtmp != &museum - a crash
      fix, not an abuse feature; two new p_coaligned calls made it
      three-deep); (2) stale-malign accounting - master's
      set_malign(mtmp) recalc at the xkilled cleanup label ported
      TOGETHER with master's murderer-block rework
      (always_peaceful/isshk gate, > A_CHAOTIC - deps
      always_peaceful/is_zombie already on branch) plus
      set_malign+newsym at the three mpeaceful->hostile flips
      (setmangry main, bystander cascade, quest guardians -
      BUG-945/981/982 pattern family). Also fixed: priest-kill
      else-if chain + Infidel lawful-priest bonus arm; steed
      water/lava BYCHOICE-vs-monkilled credit split;
      accidental_bump moved go->ga struct (first-letter
      partitioning); hack.c decl-to-block-top. FOLLOW-UPS
      (maintainer decisions / later slices): dig.c tree-cut Ranger
      arm RESOLVED (maintainer 2026-07-16): kept and raised to -2
      with an ABUSE_DESTROY_TREE record and mild-guilt message
      (Druid -15 / Elf -5 / Ranger -2; elven Rangers hit the Elf
      arm first); priest suggested-donation multiplier
      partially cancels for wealthy purses (quan divides it back
      out) - sanity-check intent; uabuse_hist_idx restore clamp
      (defense-in-depth vs corrupt saves, master shares exposure) -
      decide; atonement does not de-escalate erinyes (upstream
      adj_erinys is escalate-only by design) - confirm intended;
      re-verify restore-call ordering when the aasimar penalty
      engine lands. abuse_tier() + the aasimar penalty engine
      travel with the aasimar race slice (named in the attrib.c
      stub). ugift_history is a parallel dead Phase 1 ring - its
      display pends with the sacrifice/gift subsystem.
      WIZMODE BATTERY PASSED 2026-07-16 (commit 028cbf44a; two games,
      sanity_check on throughout, paniclog NEVER CREATED, save dir
      restored): [lawful human Valkyrie XL30] peaceful attack ->
      guilt + abuse=5 + ring entry (type ATTACK_PEACEFUL, pen stored
      +5, watchman mndx+1 detail, canspotmon-gated) + #conduct
      "moderately abused" + history line "attacking a peaceful
      watchman (-5)"; tier walk via gdb 15/30/50 -> seriously/
      severely/gravely; wizard enlightenment "alignment abuse value
      is N"; BLOCKER-2 FIX LIVE: angered ex-peaceful's malign
      resynced +15 by the setmangry set_malign, killing it = NO
      phantom KILL_PEACEFUL_MALIGN record, no murderer, abuse
      unchanged; Minetown atonement ladder EXACT (non-coaligned
      priestess of Loki, 50k/tier): 30->15, 100k->two tiers 15->1,
      50k->0 with the not-of-my-faith full-absolution verbalize;
      abuse-scaled suggested ask visibly fell 12720->9397->6637 as
      tiers dropped; ATONEMENT ring entries +15/+14/+1; post-clear
      #conduct "absolved of past alignment abuse" WITH history still
      shown (ever_abused gate); ring wraparound + eviction (6th
      event overwrote oldest, idx wrapped 4->0); Elbereth hypocrisy
      (athame-engraved; finger-dust degraded to "E|ber{tr" and
      correctly did NOT trigger): "You feel like a hypocrite." +
      engraving fades + ATTACK_ELBERETH pen 2 (-rnd(5) at record<=5)
      + ATTACK_PEACEFUL same turn; one-shot kills of peacefuls
      bypass setmangry (no hypocrite/anger) and route through
      xkilled KILL_PEACEFUL only - master parity; accidental bump
      of invisible peaceful stalker: "Wait! ... That was careless of
      you." + luck -2, abuse UNCHANGED, no ring entry,
      ga.accidental_bump read back 0 (consume-on-entry proven);
      malign tail contributed 0 on a still-peaceful gnome kill
      (recalced malign >= 0, code byte-identical to master).
      [neutral human Ranger] tree wish + axe chop -> "You cut down
      the tree. You feel a little guilty." + abuse=2 +
      DESTROY_TREE ring entry (maintainer's -2 ruling live);
      save/reload round-trips abuse/ever_abused/idx/ring
      byte-identical; post-reload #conduct "slightly abused" +
      "destroying a tree (-2)". NOT live-run: museum-copy crash
      repro (statically verified + reviewer-verified vs master fix
      96c1f08ae; the gdb staging attempt is the gotcha below),
      coaligned-priest 25k rate + LUCKMIN priest-bump arm (code
      master-identical; non-co rate proven), COLLISION accidental
      variant. BATTERY GOTCHAS: NEVER call message-generating game
      functions (newcham) from batch gdb - the nested More-prompt
      hangs the call, the Bash timeout SIGTERMs gdb, the signal
      hits the game and panictrace aborts it (killed battery game 1;
      no paniclog entry, stale lock destroyed on next launch);
      genesis "aligned priest" creates an emin MINION (ispriest=0,
      hostile cross-aligned) - real ispriest priests only via
      temples (Minetown route: C-v to 4, branch stair, 3x descend,
      altar '_' getpos-jump); rloc_to + freeze on a temple priest
      angers her on next chat ("breaks out of reverie") - gdb-flip
      peaceful back + set_malign recovers; wished axes/helms come
      cursed often (welds) - wish blessed or plan around.
- [x] Monster AI/equipment: dual-wield, artifact ranking, weapon
      selection (see weapon-selection-architecture memory)
      (+ master monmove.c dochug ranged-attack scan block using
      mfind_target - reconcile with branch's 5.0-native
      ranged_attk_available gate; + master mlined_up tame
      friendly-fire path walk vs branch m_lined_up; + muse.c
      mfind_target offensive-item consumer)
      PORTED + REVIEWED 2026-07-16 (port subagent 24 files
      +1369/-151, reviewer CONDITIONAL PASS NO blockers -> all 4
      concerns + nits coordinator-verified and fixed same day;
      clean rebuild + wizmode battery + commit pending below).
      Contents: weapon.c selection architecture (oselect_recurse
      container recursion + best-by-dmgval + priest edged gate;
      find_artifact_recurse + score_artifact_weapon in artifact.c
      mirroring BRANCH spec_dbon/artifact_hit arms only;
      find_silver_recurse/find_gem_recurse; would_prefer_rwep/
      would_prefer_hwep/mon_melee_target via un-static'd dogmove
      find_targ/score_targ; mon_needs_ammo/mon_wants_ammo with the
      searches_for_item/mon_would_take_item non-pet consumer;
      select_rwep rework incl. polearm wield range 13->5 master
      fix, Infidel weld exemptions, dark-elven launcher chains,
      tmpprop launcher latch; select_hwep rework incl. artifact-
      then-silver ranking, defended(AD_STON) corpse gate, giants
      one-hand bimanual; polearms stay pwep[]-exclusive, pwep/hwep/
      rwep disjointness verified programmatically). DUAL-WIELD:
      can_dual_wield/select_offhand_hwep/mon_wield_offhand +
      setmnotwielded2/mwep2gone, mon_wield_item rework (OBJ_
      CONTAINED -> new mon.c mon_container_extract, turn-consuming
      rummage), possibly_unwield mw2 arm, worn.c shield-forces-mw2-
      drop + extract_from_minvent W_SWAPWEP choke point (supersedes
      master per-site steal fixes - both m-v-m steal paths verified
      routing through it), mhitm/mhitu weap_attk_idx offhand attack
      loops (OFFHAND_PENALTY, getmattk erac-consistent), passive-
      acid + rust-trap + petrify mw2 arms, mkobj mon_obj_sanity mw2
      check, makemon clone_mon mw/mw2 stale-pointer clear, zap
      montraits mw2 copy + mkobj save_mtraits MON_NOWEP2
      (reviewer-found asymmetry), objnam minimal W_SWAPWEP suffix,
      restore.c mw2 relink cloned from the mw idiom (savemon whole-
      struct Sfo_monst verified - NO new sfo pair needed). AI
      CONSUMERS: monmove.c dochug m-v-m ranged scan at master's
      position (after m_dotelekinesis hook; result codes mapped by
      NAME; no double attack vs the 5.0 move-then-shoot gate -
      scan returns 0 as the turn); muse.c find_offensive
      mfind_target targeting (PETSTRAT_NOATTACK + acceptable_pet_
      target gates inert until #order) + gt.tbx/gt.tby aiming for
      offensive buzz/potion-throw/mbhit + BOTH defensive mbhit
      callers aim explicitly (master only fixed teleport - undead-
      turning stale-aim is MASTER BACKPORT CANDIDATE #6); mon.c
      I_SPECIAL weapon-side re-eval (parked-mx guard, msp_reading
      defer, welded/launcher skips); mthrowu.c m_lined_up friendly-
      fire walk (REVERSED args vs master mlined_up, breath param;
      callers: breamm TRUE, mfind_target/thrwmm/spitmm/lined_up
      FALSE); thrwmu void->boolean + master head rework.
      REVIEWER CONCERNS ALL FIXED: (1) both defensive mbhit arms
      lacked master's gb.buzzer kill-attribution bracket +
      DEADMONSTER-guarded return (offensive arms had them; sibling
      sweep confirmed no others); (2) save_mtraits nulled mw but
      not mw2 while montraits copies both back (defensive, no live
      producer today); (3) score_artifact_weapon "never calls
      rnd()" comment was FALSE both trees (spec_applies rolls
      rn2(100) vs mr for AD_MAGM/AD_STUN targets, monhp_per_lvl
      rolls a die) - comment fixed, determinism rework queued with
      the artifact-behaviors re-sync follow-up, master frozen-
      unfixed; (4) would_prefer_*/mon_needs_ammo land with zero
      branch consumers - CONFIRMED deliberate (master callers are
      dogmove only = pet order slice; architecture-first
      precedent). DELIBERATE DEVIATIONS (recorded): select_offhand_
      hwep gains a touch_artifact skip gate master lacks (master
      lets a monster offhand-wield an artifact that would blast it
      - MASTER BACKPORT CANDIDATE #7); dochug scan maps mattacku's
      1 to M_ATTK_AGR_DIED so dochug returns 1 on attacker death
      (master loses the signal - harmless there, same dmonsfree
      reasoning as telekinesis); possibly_unwield mw2 arm keeps
      branch print-before-extract ordering (fixes master's
      print-after-extract). FOLLOW-UPS QUEUED: re-sync score_
      artifact_weapon arms when artifact-behaviors slice lands
      (Dichotomy/Angelslayer/Harbinger/Orcus/Annihilation/
      vulnerable_to scaling absent by design - TODO-EVILHACK at
      the vulnerable_to site). CLONE_MON WORN-MASK FIX (maintainer
      "fold it in" 2026-07-16): clone_mon now clears
      misc_worn_check beside the mw/mw2 clears - the *m2 = *mon
      copy carried worn-slot bits with an empty inventory in ALL
      FOUR trees (branch/master/both upstream 5.0); a cloned
      gear-wearer was a candidate producer for the BUG-1273
      stale-worn-slot class (worn_slot_sanity fires on mask-
      without-item) and the gap violated the clone-mirrors-
      polymorph transient-state reset rule. Master frozen-unfixed;
      upstream-reportable (worn-mask half only - upstream also
      lacks the mw/mw2 clears, see reportable table). DEFERRED (item -> subsystem):
      pet consumers of would_prefer_*/mon_wants_ammo + is_better_
      armor mon_bimanual (pet order), mhitu steed-retaliation
      acceptable_pet_target (steed), m_stash_items + m_dowear
      container armor extraction (monster looting), worn.c ring-
      choice mw2/oprops blocks (monster ring/amulet), mon_wielding_
      artifact incl. MON_WEP2 arm (artifact behaviors), AD_ACID
      explosion arms EXPL_ACID/EXPL_SHOCK (explosion-glyph
      extension - TODO-EVILHACK markers at mhitm.c/uhitm.c sites,
      acid sphere exists on branch = silent-divergence guard),
      muse offensive-item parity (maxdmg melee gate + master-only
      MUSE_* items), munstone self stone-to-flesh (monster
      spellcasting), hitmsg pierce/slash/whack verbs (mhitu
      flavor), break_glass_obj setmnotwielded2 (glass shatter),
      pager.c probe wielding-both display, pickup.c exchange_
      objects_with_mon MON_NOWEP2 (pet item-exchange), dothrow
      glass-breakage unwear, ZT_WATER arm + mon_underwater acid
      gates (underwater parity), W_BARDING resists slotmask
      (steed), thrower attribution global (kill-attribution),
      objnam owner-aware doname rework. MASTER QUIRKS PORTED
      FAITHFULLY: select_rwep tmpprop NULL-latch (first empty
      launcher search poisons later latches - low impact);
      branch can_touch_safely rejects gloveless defended(AD_STON)
      corpse-wielders that the new selection gates accept (inert
      inconsistency, fold into a defended() sweep). BATTERY LIST
      (ASK MAINTAINER before launch; #levelchange 30 first;
      sanity_check on): (1) dual-wield lifecycle - genesis a
      dual-wielder (or gdb-arm one), verify both weapons in
      combat messages + OFFHAND_PENALTY, then steal/poly/shield-
      don m_dowear drop/petrify/save-reload with mw2 set,
      mon_obj_sanity quiet throughout; (2) artifact ranking - pet
      with two artifacts incl. one in a carried sack (rummage
      turn + wield turn), watch for wield churn (concern-3
      nondeterminism exposure); (3) m-v-m ranged - hostile with
      launcher+ammo vs another monster: dochug scan fires at
      dist2>8 from hero, no melee/launcher oscillation, no
      double attack per turn; (4) pet friendly-fire hold - tame
      breath/wand user lined up through hero or another pet =
      holds fire (m_lined_up walk); (5) muse offensive aiming -
      monster zaps striking/fire wand at a MONSTER target via
      gt.tbx (not at hero), stinking-cloud absence unchanged;
      (6) defensive undead-turning/teleport zap: beam aims at
      hero's tracked position, kill through the beam credits the
      caster (buzzer fix; pet livelog line); (7) polearm: monster
      with polearm-only inventory wields at dist2<=5, thrusts at
      2-3, no re-wield loop at 6-13; (8) mon_container_extract -
      locked chest + cursed BoH refusals (no stall), unlocked
      sack = one rummage turn then wield turn; (9) I_SPECIAL
      re-eval - hand a pet a better weapon via floor gift, next
      turn it re-wields (no combat trigger needed), frozen/
      mid-read monster defers; (10) save/reload + bones with a
      dual-wielder on-level and mid-migration: mw2 relink, no
      impossible; statue/corpse revive of a dual-wielder (stone-
      to-flesh + undead turning) with sanity_check on (save_
      mtraits fix regression guard); (11) launcher point-blank:
      adjacent launcher-wielder swaps to HTH or point-blank
      shoots per master logic, attack slot not double-consumed;
      (12) paniclog EMPTY at session end.
      COMMITTED 2026-07-16 as 89dfbd4e7 (25 files +1516/-153 incl.
      the clone_mon misc_worn_check fold-in and this record's
      earlier half; bundle evilhack-50_2026-07-16c cut+verified at
      that head; this battery record rides with the next commit).
      WIZMODE BATTERY RUN + PASSED 2026-07-16 (no-install recipe:
      src binary vs installed playground, valid because the slice
      changed no data files; one Valkyrie XL30 game, sanity_check
      on all game, T:37). PANICLOG: only 2 same-event entries from
      a coordinator STAGING mistake (gdb freeinv of the hero's
      WIELDED spear -> stale uwep; worn_slot detector caught it
      instantly = detector pipeline live-verified; repaired via
      gdb null uwep + owornmask clear). ZERO real firings.
      PASSED LIVE: (1) dual-wield core - tame marilith wields
      scimitar + "wields 5 daggers in her other claw" (stack
      offhand OK); m-v-m cycle "swings her long sword"/"swings
      her scimitar" both AT_WEAP slots; vs-hero missing-mw2
      fallback-to-primary "hits! hits again!" (master parity);
      orc-captain worn-shield can_dual_wield refusal (negative);
      (2) combat wield "wields a bronze scimitar!" after gdb
      mongets (weaponless-kit orc punches correctly before);
      (3) I_SPECIAL re-eval - planted +5 long sword + check_gear
      -> non-combat turn "The marilith wields a long sword!",
      mw2 daggers PRESERVED across primary swap, old primary
      mask zeroed; (4) silver-hate selection gate - planted +7
      silver saber: I_SPECIAL consumed, mw unchanged (demon
      mon_hates_material skip; handler precedes the pet drop);
      (5) container extraction - bag+7 sword on hostile ogre:
      "removes a long sword from the bag" (rummage turn) ->
      "wields a long sword!" (wield turn) -> melees hero with
      it; (6) offhand throw lifecycle - pet throws daggers FROM
      the wielded mw2 stack (5.0-native pet ranged), splits keep
      W_SWAPWEP quan 5->3, last-dagger throw clears mw2 clean;
      (7) dochug m-v-m ranged scan - Grey-elf (gdb-armed bow)
      "wields an elven bow!" + "shoots 3 elven arrows!" at
      dist2 9 (>8 hero-guard boundary); ogre threw a SPEAR at
      the MARILITH = pure m-v-m scan+thrwmm; ogre hurled a
      potion at the marilith = muse offensive m_throw at a
      monster target via gt.tbx; (8) launcher point-blank -
      hero closes -> elf swaps "wields a copper elven
      broadsword!"; (9) save/reload - marilith mw+mw2 relink
      exact (o_id preserved, W_SWAPWEP, quan, pointer resolves
      into own minvent); (10) 37 turns of pick/drop/throw/kill
      churn sanity-silent. NOT LIVE-TESTED (static/reviewer
      verification stands): shield-don mw2-drop (staging
      inconclusive, see follow-up), tame friendly-fire walk
      (pets exit dochug before the scan until pet slices land),
      defensive mbhit aim+buzzer (coordinator code fix +
      reviewer-verified), corpse/statue revive mw2 (no corpse
      rolled; fix is defensive), bones round-trip, polearm
      range boundary. FOLLOW-UP (ride with pet order slice):
      staged +3 shield on the dual-wielding tame marilith never
      donned; I_SPECIAL re-armed every turn by pet pick/drop
      churn (the deferred droppables would_prefer consumers) and
      an instrumented turn never reached m_dowear_type(W_ARMS) -
      churn may starve the equip pass for pets; ALSO the known
      deferral gap showed live as a pet pick-drop oscillation
      (scimitar loop). BATTERY GOTCHAS (new): gi.invent head is
      NOT the newest item (wished items append at tail); NEVER
      gdb-freeinv a wielded hero item (stale uwep, detector
      fires; repair = null uwep + clear moved obj owornmask);
      gdb -ex while-loops fail SILENTLY (use flat derefs);
      live-game breakpoints survive via background gdb +
      pkill -INT bailout (a timeout SIGTERM would kill the
      game); genesis kit rolls vary (weaponless orc-captain,
      melee-only Grey-elf - normalize via gdb mongets); mongets
      merges stackables (spears rne(2)) - distinct otyps for
      separate gifts; ^G tame prefix silently fails on marilith
      (gdb newedog+initedog recipe works).
- [x] Monster spellbook learning (struct emsp)
      PORTED + REVIEWED 2026-07-16 (port subagent 38 files
      +2530/-84, reviewer CONDITIONAL PASS 1 blocker + 5 concerns
      -> ALL coordinator-verified and fixed same day; clean
      rebuild + battery + commit pending below). Contents: emsp
      lifecycle (newemsp/free_emsp beside erac ctors, copy_mextra
      arm, LAZY allocation - first read attempt/first learn/
      restore only, so the AI slice's I_SPECIAL msp_reading guard
      stays correct); FULL sf-layer serializer on the erac
      exemplar (emin->emsp->engr position in ALL positional
      structures, critical_sizes entry, savemon/restmon blocks
      after erac before mcorpsenm - reviewer verified positional
      consistency end-to-end; dev saves invalidated) + SFCTOOL
      NOW LINKS: fixed THREE pre-existing link breakers -
      sfi_x_abuse_event (alignment-abuse slice gap: sftags.c
      never got the abuse_event entry), erid genericptr mapping
      (pure link fix; erid serializer still rides the steed
      slice), sfo/sfi_x_attack for erac->mattk (unrecorded ERAC
      slice gap; attack struct uncommented in sftags + permonst.h
      added to the ctags sweep in sys/unix/Makefile.utl SOURCE +
      regenerated util/Makefile); generated sfdata.c/sfproto.h
      diff verified limited to those additions + MAXMONSPELL/
      NATTK per-element loops. Learning path: learnable_spells[]
      whitelist/spell_is_learnable/mcan_learn_spell/
      find_spellbook_recurse/mon_wants_spellbook (dead until pet
      could_use_item - deferral note) + find_misc/use_misc
      MUSE_SPELLBOOK reading state machine (3 turns/level,
      blessed halved; blank-on-attempt semantics preserved;
      mid-read continuation by otyp, depleted twin finishes
      silently; hostile reads only at dist2>8). Casting:
      MCAST_LEARNED_SPELL/MCAST_UNTAME_PET grafted into 5.0's
      choose_monster_spell/mcast_data architecture (master's
      dual-enum rewrite NOT transplanted); mknows_spell/
      mlearn_spell/mcount/mchoose/mpick/cast_learned_spell/
      mhas_castable_offensive_spell/mhas_useful_selfbuff_spell;
      target_resists_learned_spell = single choke point,
      NULL-tolerant probes, teleport-away wand-parity gates via
      5.0 helpers (noteleport_level/mon_knows_traps/stairway_at),
      deliberately NO mon_has_amulet(target); charm monster =
      untame-unlock-only shape exact; NEW castmm (learned + untame
      only; innate m-v-m TODO-EVILHACK -> bosses; mspec cost uses
      branch 5.0 castmu formula (m_lev<8)?10-m_lev:2 not master
      4-m_lev - DELIBERATE, in-tree consistency); mage_spells
      decay + allmain hook (5000 know, -1/10 moves, silent
      forget). Defensive: MUSE_SPE_TELEPORT_AWAY(_SELF) (SELF
      inlined from branch m_tele step-for-step - m_tele's how
      param would falsely makeknown; at-hero via
      mcast_immediate_spell->mbhitm with gt.tbx + gb.buzzer);
      mcast_ray_spell (BZ_M_SPELL ray mapping, 8-ray contiguity
      re-verified vs branch objects.h)/mcast_immediate_spell/
      mcast_nodir_spell + monster_uncurse_items; new mbhitm arms
      FORCE_BOLT (death_inflicted_by attribution; knockback
      TODO -> knockback slice)/POLYMORPH/TURN_UNDEAD (resist
      class = otmp->oclass)/SLOW/DRAIN_LIFE/DISPEL_EVIL
      (set_pet_killer TODOs x4 -> kill-attribution)/ENTANGLE.
      Shared helpers: spell.c cast_reflection/barkskin/stoneskin/
      entangle (+skill param on cast_protection; GRASS arms
      TODO -> terrain slice) + hero spelleffects cases +
      zap.c bhitm/zapyourself ENTANGLE arms; TT_ENTANGLED trap
      type (append-only) + mentangled/mextrinsics/5 buff timers
      activated across ~15 files (m_calcdistress expiry home,
      mon_regen decrements, clone_mon/montraits/rloc/scatter/
      detach clears); SKIN bodypart (enum append + 20th column
      in all 12 polyself tables, reviewer-counted); new mondata.h
      predicates (can_jump/can_levitate/can_fly/mon_underwater/
      is_jumper/immune_death_magic, has_reflection/is_evil MR2).
      REVIEWER BLOCKER FIXED: nh_timeout had NO expiry cases for
      REFLECTING/BARKSKIN/STONESKIN - one stoneskin cast =
      PERMANENT stone resistance (HStone_resistance I_SPECIAL
      never cleared); master's three cases ported. CONCERNS
      FIXED: find_ac Barkskin/Stoneskin AC arms + ureflects
      HReflecting "magical shield" arm (spell was AC-inert +
      reflection ignored at mcast_lightning/floating-eye sites);
      defended(AD_MAGM) added to BOTH mbhitm force-bolt and
      polymorph resist gates (master parity; gear-protected
      monsters were polymorphable); zhitm msp_reading interrupt
      (beam family bypassed damage_mon - read-through-fireballs;
      interrupt-site census now 8:8 vs master); FIVE mentangled
      consumer sites (find_defensive stair-flee gate, find_misc
      polytrap gate, mhitu to-hit -2, mm_displacement forbid,
      pager farlook ", entangled"); mbhit force-bolt terrain
      parity (SPE_FORCE_BOLT joins WAN_STRIKING at drawbridge +
      door smash + NEW IRONBARS blow-apart arm; mid-beam
      DEADMONSTER/o_on revalidation stays deferred -> knockback
      slice, branch's cached-otyp mitigates the read half;
      temple_at_boundary arm TODO - helper has 24 master sites =
      its own temple damage-repair sweep, NEW deferred item);
      polyself SKIN cosmetics (skeleton bones, dragon scales).
      NITs recorded: entangle terrain gate is picker-only in
      MASTER TOO (announced-cast can no-op; parity kept, optional
      improvement noted); resist-then-damage double-damage in
      DRAIN_LIFE/DISPEL_EVIL arms = master parity (BUGFIXES
      candidate with damage_mon core slice); montraits clears
      buff timers but not paired mextrinsics bits (master parity,
      one spurious expiry message on revival). DEFERRALS: innate
      m-v-m casting/buzzmm/castum + hellfire + boss mspec resets
      (bosses), knockback machinery, cancel_monst buff strip
      (cancellation), damage_mon vuln/HPHDAM core, GRASS terrain,
      set_pet_killer x4, pet could_use_item consumers
      (mon_wants_spellbook, dothrow catch-deferral), entangle
      flavor consumers (sounds x14/priest/steed/dokick/dog),
      msicktime/mdiseasetime/msummoned/mbindtimer decrements
      (their slices), TEMPLE DAMAGE-REPAIR SWEEP
      (temple_at_boundary, 24 master sites, 13 files - NEW),
      HERO SPELL PARITY (17 master spelleffects cases still
      unwired: LIGHTNING/POISON_BLAST/ACID_BLAST/PSIONIC_WAVE/
      POWER_WORD_KILL/DISPEL_EVIL/CRITICAL_HEALING/BURNING_HANDS/
      SHOCKING_GRASP/REPAIR_ARMOR/CHANGE_METAL_TO_WOOD/
      CREATE_GRASS/CREATE_TREES/ORB_OF_FIRE/ORB_OF_FROST/
      SUMMON_ANIMAL/SUMMON_ELEMENTAL - default impossible();
      NEW item, owns cast_metal_to_wood etc.). MASTER-PARITY
      WARTS RECORDED: monster entangle scales off the PLAYER's
      P_SKILL (project-monster-spell-skill follow-up; two sites);
      master PWK hero arm ignores polymorph HP (u.uhp not u.mh -
      fix master first policy, frozen). FOLLOW-UPS: sfctool
      tempered sysconf parser rejects live EvilHack OPTIONS/
      MSGTYPE lines - RESOLVED 2026-07-16 post-battery: the two
      existing tolerance flags (ignore_errors_on_unmatched +
      ignore_statement_errors) initialize TRUE under the sfctool
      build only, rcfile_interface_options precedent; tool now
      starts clean against the live sysconf, game validation
      unchanged; archive entries BUG-1338..1341 written same
      pass (defensive undead-turning stale aim, offhand
      touch_artifact, scorer false-determinism, PWK polymorph
      HP); generated exportascii lossy for simple-type
      arrays generally (upstream-reportable; emsp/erac arrays
      exempted via nah loops). PROCESS NOTE: the port subagent
      ran an UNAUTHORIZED wizmode verification game (cleaned up
      fully; save dir/sysconf/processes verified clean after;
      its paniclog claim was a MISREAD of the prior slice's
      staged artifact - no new worn-slot firing); future port
      briefs must explicitly reserve game launches for the
      coordinator. Its verification data (emsp save round-trip
      byte-identical, sfctool -c emsp block, learned fireball +
      slow monster cast live, decay tick, read-at-distance state
      machine end-to-end) is folded into the battery plan.
      BATTERY LIST (ASK MAINTAINER; #levelchange 30; sanity_check
      on): (1) stoneskin expiry REGRESSION - learn+cast
      stoneskin, gdb-tick or wait out HStoneskin, verify stone
      resistance LOST + expiry message + find_ac drop (blocker
      fix); barkskin/reflection expiry messages + AC/ureflects
      arms live; (2) read-interruption - hostile mid-read zapped
      by a beam stops reading (zhitm fix; melee half via
      damage_mon already agent-verified); (3) entangle - cast at
      pet near tree: held + can-still-attack + hero swap blocked
      + farlook ", entangled" + to-hit -2 + release at timer;
      (4) charm/untame castmm - hostile caster knowing charm
      adjacent to pet: untame + set_malign resync, no direct-cast
      of charm ever; (5) defended-gate - polymorph beam at a
      gear-MR monster = shieldeff no poly; (6) force bolt vs
      door/bars - door smashes (shop damage listed), bars blow
      apart + bolt stops; (7) teleport-away defensive: wounded
      caster self-teleports (shk/priest refuse), at-hero variant
      beams via mbhitm with buzzer credit; (8) save/reload
      mid-read monster + 8-spell monster + sfctool ascii
      round-trip on that save; (9) decay: msp_know ticks -1/10
      moves; (10) paniclog EMPTY (current two lines are the
      documented prior-slice staging artifact - note before
      install wipes them).
      COMMITTED as 06310ad06 (41 files +2770/-92; bundle
      evilhack-50_2026-07-16d cut+verified at that head).
      WIZMODE BATTERY RUN + PASSED 2026-07-16 (no-install recipe,
      two Wizard games - the first retired after the battery's
      own catch, see BUG-1337; sanity_check on throughout;
      paniclog at end = the 2 documented prior-slice artifact
      lines + 3 "Unknown spell skill" impossibles from the
      PRE-FIX first game = the caught bug's own trace; ZERO
      other entries). BATTERY CATCH BUG-1337: casting ANY
      evocation-school spell aborted with "Unknown spell skill,
      33" - 5.0-native spelltypemnemonic() lacks the
      P_EVOCATION_SPELL case; latent since the Phase 1 skills
      port, unreachable until this slice wired the first
      evocation hero spells; one-line master-parity fix +
      rebuild MID-BATTERY (fixup commit rides with this record;
      weapon.c PN_ plumbing was already correct). PASSED LIVE:
      (1) BLOCKER regression - stoneskin cast "A thick layer of
      stone covers your skin.", AC 9->6 (unskilled find_ac arm),
      HStone_resistance I_SPECIAL set; gdb-shrunk timer ->
      "The stone covering your skin completely disappears." +
      AC restored + I_SPECIAL CLEARED (no more permanent stone
      resistance); (2) barkskin AC 9->8 + expiry message;
      reflection "A shimmering globe appears around you!" +
      "flickers and vanishes" expiry; (3) ureflects arm - THE
      FLOATING EYE'S GAZE "is reflected by your magical shield"
      (intrinsic reflection now honored at the passive-gaze
      site); (4) hero learn+cast of all four ported spells from
      wished books; (5) entangle - "The roots from a nearby
      tree ensnare the floating eye!" + mentangled=1/
      mentangletime=3 by gdb + timer release during play;
      (6) decay live (msp_know 5000->4999->4997 on the staged
      caster); (7) zhitm read-interruption fix - gdb-staged
      mid-read caster (msp_reading=450) hit by a hero fire ray:
      msp_reading cleared to 0, hp damage applied; (8) ambient
      regression net - prior slice visible ("The elven noble
      wields a copper runed broadsword!" wield naming), two
      wizard-mode deaths declined cleanly, heavy spawn churn
      sanity-silent. DIAGNOSED NOT-A-BUG (breakpoint-proven):
      a staged AD_SPEL+AD_MAGM caster spamming "points at you,
      then curses" is 5.0-native buzzmu m_seenres frustration
      (hero's magic resistance bounced its AD_MAGM ray once;
      buzzmu then curses forever at range) - choose_monster_
      spell was proven uninvolved via live breakpoint; ALSO
      structural observation recorded: directed learned spells
      vs the HERO fire only from castmu (adjacent), and the
      dochug scan's AT_MAGC term is gated adtyp<=AD_LOUD
      (MASTER PARITY - AD_SPEL casters enter m-v-m learned
      casting only via the find_offensive/AT_WEAP scan terms
      or adjacency; matches master, do not "fix"). NOT
      LIVE-TESTED (reviewer + port-agent verification stands):
      castmm m-v-m learned cast + MCAST_UNTAME_PET + the mbhit
      IRONBARS/door force-bolt arms (three staging attempts
      disrupted by spawn churn + the scan-gate structure above;
      the port agent's unauthorized game did verify the
      mcast_immediate/mbhitm beam family live vs hero),
      defensive MUSE_SPE_TELEPORT_AWAY, farlook ", entangled"
      text, defended-gate polymorph negative, sfctool ascii
      round-trip (port agent covered -c/-u live). BATTERY
      GOTCHAS (new): multi-line gdb while-loops DO work from
      -x SCRIPT FILES (only inline -ex breaks); monster-chain
      HEAD shifts with every spawn - re-find staged monsters
      by walking for has_emsp/name before every write (a write
      aimed at a stale head hit a lieutenant; gdb refused the
      NULL-emsp deref so no corruption - verify reads-back
      after every staged write); wished-item slot letters do
      not map to gi.invent chain order (books/wands cluster) -
      resolve otyps via an objects[] name scan from a gdb
      script file (WAN_STRIKING=518, SPE_CHARM_MONSTER=473,
      SPE_TELEPORT_AWAY=487, SPE_FORCE_BOLT=462 this build);
      pkill -INT can need TWO rounds to pop a multi-continue
      gdb (verify ps -C gdb empty before the next attach, else
      the new attach fails with "No symbol table" garbage);
      XL30 heroes attract heavy spawn churn - stage multi-step
      m-v-m scenarios EARLY in a game, not at T:150+; a fire
      ray kills your own pet in the line (check the path).
- [x] Zombie/were/Draugr mechanics (zombify, binding ritual)
      PORTED + REVIEWED + COORDINATOR-FIXED 2026-07-16, COMMITTED
      2026-07-17 as 7b19b7ba8 (36 files +1094/-181; port subagent 34
      files, reviewer CONDITIONAL PASS 2 blockers/6 concerns/6 nits ->
      all adjudicated + fixed; full make clean rebuild zero-warning).
      WIZMODE BATTERY PASSED 2026-07-17 (maintainer-approved; three
      games - human Valkyrie XL30, draugr Dark Knight XL30, aasimar
      Valkyrie; sanity_check on; PANICLOG NEVER CREATED). Verified
      live: (1) mm bite infection msick=3/msicktime paired/msickbyu=0
      by gdb + "brain is eaten"/"looks rather ill" + "dies from his
      illness" m_calcdistress death; (2) witherling rise from wished
      zombifying gnoll corpse + witherling re-infected a cured third
      party + destroyed-zombie corpses re-rose via the 1/250 arm
      (self-sustaining spread loop end to end); (3) create_particular
      sick (msick=1 flat, no zombify - master-exact) + diseased flags
      by gdb, both death messages; (4) hero SICK_ZOMBIE arc via
      witherling/zombie bites - usick_type=4 by gdb, TermIll botl, ^X
      "turning into a zombie", death "zombified by a human zombie" in
      logfile+xlogfile, bones contain the risen christened human
      zombie (found + fought when Game B loaded them, 187hp carried);
      (5) cures - prayer, unicorn horn x2, muse eucalyptus leaf on a
      staged sick watchman ("is no longer ill"), CON bounce-back
      correctly excluded (code re-read; the one apparent "recovery"
      was a blind-spaced wizard Die?-decline savelife, operator
      error); (6) gray fungus passive BOTH directions - hero punch
      "You feel deathly sick", pet melee "looks diseased"/"looks even
      worse" escalation, pet died of the infection; (7) zombie moans
      + BRAAAAAAAAINS ambience; (8) weredemon beast->demon form
      change live (gdb mnum 34->435), bite pressure vs hero (no
      lycanthropy in ~60 bites - rn2(4)+negation gates, form arms
      reviewer-verified); (9) Grimtooth "The filthy dagger infects";
      (10) death ray full-heal 26->31 by gdb; (11) binding ritual
      full cycle - chant + rattle message, mtame 10->11, exactly 5
      Pw, mbindtimer 1500 set/ticking, "not yet ready" cooldown gate
      with ZERO Pw charge, "wholly bound" cap untested (mtame 20 too
      far), mbindtimer PERSISTED across save/reload (1499->1498);
      (12) draugr hostile imposition "attempt to impose your will" +
      ignore arm; (13) draugr "tries to eat your brains, but can't"
      + helm-blocks-bite arms; draugr zombie-corpse meal "Mmmm...
      tainted meat!  Nutrition gained: 200" with NO infection (C3
      carve-out); (14) draugr Sunsword wield REFUSED ("You can't
      handle"); (15) draugr sickness death reclassified "diseased by"
      wording arm hit by the AASIMAR game instead: at Death's door
      dialogue + xlog "diseased by a human zombie", no arise; (16)
      B1 REGRESSION: draugr hero killed by a wraith, bones saved,
      reloaded in game 3 - chain scan shows ZERO christened monsters,
      plain unnamed killer wraith present, no ghost (the old
      ismnum-first order would have shipped a risen wraith named
      wizard). NOT live-tested (code-verified, owners noted):
      msickbyu->xkilled hero credit (two stagings eaten by pets; arm
      is master-verbatim), weredemon kill-in-beast-form revert +
      were_summon hell hounds (form is beast at night; dawn flip
      unreachable in-session), pet-side eucalyptus dogfood eat (pet
      died first; m_consume_obj cure is the same helper the muse
      path proved), Samurai Grimtooth abuse event + pet-wielded
      Grimtooth attribution, sleeping-gas/fountain Breathless gates
      (one-line reviewer-verified swaps; trap KITS don't apply yet -
      deferred trap-mechanics subsystem, "Sorry, I don't know how to
      use that"). FINDINGS for other slices: draugr #polyself into
      zombie form re-rolls "You feel like a new draugr" (player-races
      slice - check master polyself draugr behavior); race menu shows
      THREE 'd' letters (dwarf/drow/draugr collision - Phase 1 roles
      follow-up; workaround -r draugr CLI); spell.c cure-sickness
      draugr backfire arm is DORMANT on branch (rejectcasting
      can_chant blocks zombie-race heroes from casting at all =
      master hero-parity; master reached its zapyourself arm via
      external casts - re-home when hero spell parity or
      monster-cast-at-hero heals land); battery leftovers: draugr
      bones bonD0.1.gz + logfile/xlogfile/livelog test entries in
      the evilhack50dir playground (benign, left in place). CONTENTS: zombie-sickness system
      end to end (hero SICK_ZOMBIE arc: eatcorpse taint block,
      diseasemu, timeout death -> ugrave_arise = zombienum with 6-race
      immunity list, insight/botl/end display+death, SICK_ALL cures;
      monster msick/mdiseased arcs: AD_DISE/AD_PEST/AD_DRIN re-expressed
      in the 5.0 shared mhitm_ad_* handlers, m_calcdistress death
      blocks, mon_regen + dog.c timer decay, muse cures incl. new
      MUSE_EUCALYPTUS_LEAF + unicorn-horn sick arm, create_particular
      sick/diseased flags); zombie spread (zombie_maker HYBRID per
      Decisions log; zombie_form master species map incl.
      gnoll->witherling + draugr-rises-as-self; CORPSTAT_ZOMBIE
      re-valued 0x20 - master's 0x04 collides with 5.0
      CORPSTAT_HISTORIC/SPE_VAL; ZOMBIE_REVIVE_CHANCE -> hack.h;
      erac-aware zombify_mon/cant_revive); Draugr binding ritual
      (dochat 5 Pw / +1 mtame cap 20 / mbindtimer 1500 + hostile
      zombie/skeletal-animal taming rnl(10)<1 with master's
      fall-through-to-domonnoise on success; mon_regen decrement;
      dog.c catch-up = deliberate branch addition); WEREDEMON +
      DEMON_WEREDEMON arms across all five were.c functions +
      RAT_KING guards + wolf-cub variants; Grimtooth AD_DISE artifact
      chain (also fixes live spec_abon impossible on DISE(5,6));
      Breathless_nomagic split (8 master sites + fountain.c 5.0-only
      sibling); gray fungus passive AD_DISE both directions
      (reviewer catch - was completely inert); zombie moan ambience
      as 5.0-idiom m_respond_zombie helper; mondata.h
      can_become_zombie/is_skeletal_animal/resists_lycan.
      REVIEWER BLOCKERS fixed: (B1) bones.c savebones arise chain was
      ismnum-first so a draugr hero killed by a wraith/ghoul rose as
      that monster in bones - reordered to master (statue -> draugr ->
      vampire -> rise -> ghost) and master's VAMPIRE twin arm added
      (same bug class, race live since Phase 1); (B2) find_defensive
      selected MUSE_UNICORN_HORN for msick/mdiseased but use_defensive
      lacked master's cure arm -> impossible() spam + monster wasting
      every turn until death.
      MASTER BUGS found, frozen-unfixed archive entries BUG-1342..1345:
      eat_brains msick-without-msicktime next-turn death; AD_PEST
      resist-only message inversion; Grimtooth mdiseabyu unconditional
      hero credit; can_chant racial_zombie(&youmonst) hero-subject.
      APPROVED deviations (reviewer-adjudicated): the four fixes above
      on branch; SICK_ZOMBIE excluded from 5.0's CON bounce-back
      (terminal by design); TAINT_AGE+1 for corpse age adjusts (NOTE
      fresh racial-vampire corpse taint-on-eat ~35% master -> 0%
      branch; raise with the vampire slice); mhitm zombie bite-dodge
      gated on strike; death-ray Valley heal kept master's direct
      field writes (healmon contract cannot express the 200-point
      clamp-down); spell.c cure-sickness draugr backfire re-homed
      from master zapyourself WITH the unconditional healup cure
      (reviewer C2); draugr tainted-meat nutrition arm ported now
      (reviewer C3 - live race would terminally die to its primary
      food source).
      DEFERRED with owners: steed.c/worn.c racial riding+barding
      gates (steed slice); mbreath_attk/mbreathtyp system (NEW line
      below); hack.c racial carrcap arm (player races - 5.0
      weight_cap lacks the maxcarrcap framework); draugr death
      cosmetics + sentient_arise + same_race zombie arm + hmonas
      hero-bite gates + mon_slip_free zombie wording (player races);
      AD_WTHR wither system (BWithering pre-wired at polyself);
      disintegration-path zombify reset; AD_BHED mcan arms (no
      mhitm_ad_bhed yet); hellfire soul-sear (mcastu); become_flayer
      resets (illithid larva); boss msick resets (Ice Queen/Vecna/
      Izchak/Ozzy); vampire-blood muse + bones vampire extras +
      undead_to_corpse vampire tiers (vampire race); bane_applies
      drow/demon arms + Sword of Kas (their slices); Convict-rat
      pacify half (Convict role); salamander u_slip_free clause;
      dogfood defended() additions (defended sweep); corpse_chance
      Sunsword/Hammer zombie-corpse-dissolve (artifact behaviors -
      Trollsbane sibling also absent); zap.c bhitm cure-sickness
      monster arm (spell is self-only on branch; NOTE zap_steed
      default list mentions SPE_CURE_SICKNESS -> would impossible()
      if ever reachable, pre-existing).
      STYLE NOTE: pline "Mmmm...  tainted meat!" keeps master's
      double space byte-identical (player-facing text; the comment
      style checker flags it - false positive).
- [x] mbreath_attk breath-spit cooldown decoupling
      PORTED + REVIEWED 2026-07-17 (coordinator hand-port, small
      slice; reviewer CONDITIONAL PASS 1 concern -> coordinator
      verified + fixed same day; zero-warning build; commit +
      battery pending below). MAINTAINER DECISIONS: (1) mbreathtyp/
      skeletal-dragon half SPLIT OUT to the new extended-breath item
      below; (2) cooldown numbers keep 5.0's unified values
      (8+rn2(18); set always m-v-m, 1/3 vs hero; AD_SLEE +=rnd(20)
      surcharge) - master's 6+rn2(18)/10+rn2(20) split is 3.6
      inheritance, field swap only; (3) getmattk engulf substitute
      dice = master 2d8 with 5.0's wimpy zero-damage arm retained.
      Contents: spitmm early gate (M_ATTK_MISS = master return 0),
      breamm gate+setter field swap, mon_regen decrement (after
      mspec_used, master order), dog.c mon_catchup decay arms for
      mbreath_attk AND mtk_attk (the mtk arm was a telekinesis-slice
      gap; mbreakboulder decay deferred - field verified dead on
      branch, boulder-break still runs on mspec_used per 5.0 base),
      getmattk AT_ENGL breath-block + AD_DISE/AD_DISN AT_TUCH arms +
      neothelid rn2(2) forced-engulf, munslime/muse_unslime swaps,
      nasty() arrival-delay trio (mtk_attk also a telekinesis-slice
      gap). FOLDED IN: master 7ab677750 set_malign sibling sweep,
      dog.c hunks only (mon_catchup wilder-hostile + starvation
      flips get set_malign+newsym; wary_dog newsym in the
      killed_by_u branch + set_malign at the !mtame post-transition
      block; hunk-for-hunk reviewer-verified) - the commit's spell.c
      Word of Power + mon.c mind-flayer-larva sites do not exist on
      branch yet, travel with their subsystems. REVIEWER CONCERN
      FIXED: m_balks_at_approaching() is a 5.0-ONLY consumer (no
      master counterpart) whose `!mspec_used` readiness test would
      have turned every pure breather into a permanent kiter once
      breath stopped setting mspec_used - split per attack type
      mirroring the dochug scan gate (AT_BREA/AT_SPIT ->
      mbreath_attk, AT_MAGC/AT_GAZE -> mspec_used). Reviewer NIT
      deferred to the extended-breath item: trap.c anti-magic +
      mhitm.c xdrainenergym "seems lethargic" surcharge pads
      mspec_used on AT_BREA attackers (no mechanical effect for
      pure breathers; byte-identical quirk on master). The dochug
      m-v-m ranged scan gate went LIVE with this slice (read
      mbreath_attk inert since the monster-AI slice).
      COMMITTED 2026-07-17 as c4aa6c019 (7 files +158/-30).
      WIZMODE BATTERY PASSED same day (one human Valkyrie XL30
      game, sanity_check on, T:163, save/reload cycle):
      (1) DECOUPLING PROOF: red dragon vs hero at range 3 - after
      several breaths mbreath_attk=11 (8+rn2(18) band) while
      mspec_used stayed 0 the whole game for every breather.
      (2) COOLDOWN CYCLE: gated dragon held breath (no breath
      messages) while counter decremented exactly 1/turn via
      mon_regen (11->8->...->0), breath resumed at 0; during the
      cooldown the healthy dragon CLOSED to melee = the
      m_balks_at_approaching fix live (no permanent kiting).
      (3) M-V-M UNCONDITIONAL SETTER: red dragon breathed fire at
      a staged mammoth (mm_aggression absent -> staged via
      conflict ring + m_lev drop + frozen target on a vertical
      lane) - cooldown SET after the m-v-m breath (mspec 0).
      (4) AD_SLEE SURCHARGE: background-gdb breakpoint on the
      += rnd(20) line HIT with pre=0 target-is-hero=1 (orange
      dragon sleep breath vs unresistant hero; surcharge fires on
      the no-set path exactly as ported; sleep-induced helpless
      turns make value-sampling ambiguous - breakpoint is the
      right instrument).
      (5) SPIT GATE: cobra with forced mbreath_attk=6 never spat
      while gated (counter decrementing), spat venom at 0, and the
      field stayed 0 after (plain venom sets no cooldown - the
      AD_WEBS setter is deferred with the spit-types item).
      (6) ENGULF ARM: neothelid with forced mbreath_attk=40 and
      mspec_used=0 - breakpoint on the 2d8 dice line HIT with
      breath=38 aatyp=AT_CLAW adtyp=AD_PHYS = the new
      breath-cooldown disjunct drove the substitution.
      (7) MUNSLIME (partial): hero polyself green slime attacking
      a hell hound; munslime-entry logging breakpoint captured 3
      calls with breath=0 by_you=1 and the hound STAYED a hound =
      fire-breath self-cure branch taken repeatedly (hound is
      fire-resistant, survives its own blast). Self-cure PLINE is
      unobservable (green slime hero is eyeless = blind) and the
      1/3 rn1(10,5) setter + gate-block direction never sampled in
      the swing budget - both are one-line field swaps
      reviewer-verified, same pattern as the live-proven breamm
      setters; accepted on review.
      (8) NASTY TRIO: gdb call nasty(summoner) - all 4 spawned
      nasties carried mspec/mbreath/mtk each rnd(4) (1..4).
      (9) DOG.C CATCHUP: direct mon_catchup_elapsed_time calls -
      nmv=50: 200/150/100 -> 150/100/50 (breath/mtk/mspec subtract
      arms incl. the folded mtk gap); nmv=500: all zero-clamped.
      (10) SAVE/RELOAD: staged barbed devil breath=17 mtk=9
      restored as 16/8 = exact minus one normal post-restore
      regen tick; whole-struct persistence clean.
      PANICLOG: 8 entries, ALL one timestamp = ONE self-inflicted
      staging incident, NOT a slice defect: a surcharge-round gdb
      loop re-fetched the monlist head after an earth elemental
      had spawned AND engulfed the hero - the head had CHANGED
      IDENTITY, so rloc_to() relocated the swallowing engulfer
      over the orange dragon ("placing earth elemental over orange
      dragon", 3x "mon is not there", 2x off-map-steed detector,
      "no monster to remove"); game self-recovered, zero entries
      before or after, remainder of battery + save/reload clean.
      Driving lessons -> gameplay memory: 5.0 blocks rest/search
      next to a visible hostile without the m prefix ("Are you
      waiting to get hit?" - use m. to burn turns); chain-head
      staging loops must re-verify head IDENTITY every iteration
      (spawn mid-loop = head is a different monster - this
      battery's paniclog incident); maintainer tip: genesis 'test
      monster' (9999 HP, stationary) or assassin vine for
      stationary targets instead of freezing pets; gdb 'commands
      ... continue' logging breakpoints observe silently across
      multiple hits (munslime recipe); eyeless poly forms
      (green slime) see no monster messages - instrument with
      breakpoints, not plines.
- [x] Extended breath/spit weapon types + skeletal dragon fixed
      breath (mbreathtyp)
      COMBINED BATTERY PASSED 2026-07-17 (one Valkyrie XL30 game,
      sanity_check on, save/reload cycle, paniclog gained ZERO
      entries). LAUNCH BLOCKER found+fixed first: the game
      SEGFAULTED at startup in compose_glyph_name - src/glyphs.c
      carries a SECOND zap_texts[] copy (8 entries, implicitly
      sized) that populate_glyphname_hash_indices walks for every
      glyph at options-init; NUM_ZAP=11 indexed past it into a
      NULL "%s" (the porter grew the tilemap.c copy; porter AND
      reviewer missed this one - it is invisible to NUM_ZAP-
      consumer greps because it is a bare static array). Fixed
      +3 names; only two copies exist tree-wide (verified).
      Battery results: (1) all three beams live at range with
      correct names - "breathes water/a dark energy blast/a
      disorienting blast" + "blast of X" flash names; water
      bounce mechanics; amulet of reflection reflected a frost
      blast (generic beam reflection intact); (2) drain losexp
      end-to-end (Farvel level 30, Xp 30->29, HPmax down); (3)
      stun breath -> "You stagger..." + Stun status (central gate
      non-resist path) and hero sleep-walked randomized movement;
      (4) STUN GATES: shimmering dragon took repeated Magicbane
      spec-roll hits (scare/probe messages fired) with mstun=0
      throughout (m_id-verified reads); CONTROL = stun beam
      crossfire set mstun=1 on a plain test monster (zhitm arm,
      deterministic - Magicbane-roll controls are too slow, beam
      control is the right instrument); (5) SKELETAL DRAGON:
      creation hook assigned mbreathtyp=3/frost + mintrinsics
      MR_COLD at genesis, breathed frost per the fixed type,
      save/reload preserved both exactly; (6) HERO POLYFORMS:
      skeletal form rolled mbreathtyp=5/disint + MR_DISINT and
      #monster breath fired a real disintegration beam (the
      AD_RBRE->BZ_OFS garbage path is dead); shadow form +
      reflection self-breath -> "You feel drained..." + no losexp
      (the restored C1 Reflecting mitigation, live); shimmering
      form set STUN_RES (FROMFORM) and yellow-mold passive stun
      -> "You feel a slight itch." twice, no Stun (central
      make_stunned gate); golems are no-poly so snow golem dospit
      is untestable from hero side; (7) SPIT TRIO: snow golem
      "throws a snowball!" + hero Blind (can_blnd arm); needle
      blight "fires a barbed needle!" + "Ow, that stings! The
      barbed needle was poisoned!"; drider "shoots a ball of
      webbing!" at a hero ON STAIRS = silent no-trap (correct
      maketrap refusal path); (8) HERO DOSPIT (drider form):
      floor spit broke with "Splat!", webbing hit a test monster
      -> "caught in a spider web" (thrown-webbing hmon
      maketrap+mintrap); (9) AD_WEBS TOUCH: large spider webbed
      the hero - u.utrap TT_WEB (mhitm_ad_webs mhitu arm);
      incidental: a snow golem got caught in a leftover web trap
      (webs are real traps for monsters). BATTERY DATA POINT:
      the CAVE spider has NO webbing in EITHER tree - spider
      webbing starts at large spider (the slice inventory's
      monster list mislabeled it; diagnosed via hitmu/dispatch
      logging breakpoints when the touch never fired). Driving
      notes: drider prefers its drow archery kit over spit at
      range; snow golem breath and spit share the mbreath_attk
      cooldown so snowballs are intermittent; conditional
      breakpoints on m_id are the reliable per-monster logging
      filter (mlet is an enum, not a char).
      PORTED + REVIEWED 2026-07-17 as TWO commits: COMMIT A = 5e3874faf beam widening + skeletal dragon
      + stun predicates (port subagent 28 files +499/-119 incl.
      coordinator fixes; reviewer CONDITIONAL PASS 0 blockers,
      2 concerns + 1 nit all coordinator-fixed: drain-breath
      Reflecting mitigation restored - ubreatheu is a second zhitu
      producer reachable while Reflecting; master's CENTRAL
      make_stunned Stun_resistance gate ported ("a slight itch",
      Tempest clause deferred) so hero STUN_RES covers ALL stun
      sources; 5.0-only knockback stun gated as model sibling).
      COMMIT B = spit projectiles + AD_WEBS arms (port subagent
      7 files +240/-26; reviewer CONDITIONAL PASS 0 blockers;
      coordinator folded the C3 same-function defended() parity
      gaps in ohitmon acid/poison gates; C1 webbing thrown-obj leak
      KEPT at master parity + C2 hmon freed-obj read class =
      follow-up below). MAINTAINER DECISIONS: all THREE beams
      (water/drain/stun - sea/shadow/shimmering dragons were ALL
      broken, not one) with beam space widened to 11 via
      MAX_ZT/BASE_ZT + explicit adtyp_to_zt translation (NO AD_*
      renumbering); webbing = master WEB-trap semantics (mentangled
      spell system untouched); 10 bare stun sites gated on
      mon_resists_stun/defended, ART_TEMPEST clauses deferred with
      artifact-behaviors. KEY PORT FACTS: eight hardcoded fireball
      encodes OUTSIDE zap.c (11/-11/+10/+20/%10 literals in
      explode/read/fountain/potion/dothrow/eat/mon/muse/uhitm/
      spell/dogmove) were the real stride danger - all converted to
      symbolic BZ_/ZT_ forms, reviewer hand-verified the arithmetic
      round-trips (spell fire now 12/-12; monster wands -33..-43;
      breath stun -32); branch 5.0 has ONE mhitm dispatch
      (mhitm_adtyping) so mhitm_ad_webs = one function, three arms,
      all three master gates map onto mhitm_mgc_atk_negated;
      master's isneedle flag verified 100% dead on master = not
      ported; zhitu Reflecting arms dropped EXCEPT drain (5.0
      full-reflection model skips zhitu from dobuzz, but ubreatheu
      path reaches it); tilemap zap_texts +3 regenerated tile.c
      cleanly. NEW FOLLOW-UP (bugfix, next session candidate): hmon
      caller-reads-freed-obj class - uhitm.c hmon_hitmon_misc_obj
      frees thrown venoms but callers re-read obj (three read
      sites; byte-identical in vanilla NH 5.0 = UPSTREAM-REPORTABLE,
      add to the upstream table) + the hero-thrown BALL_OF_WEBBING
      stranded-OBJ_FREE leak (master-parity wart) - one root-cause
      fix via an hmd out-flag gives both. DEFERRALS RECORDED:
      erid steed-dismount arms on webbing hits (separate_steed_
      and_rider absent) -> steed/barding slice; shambler_init
      damg_melee_types AD_WEBS -> shambling-horror slice;
      break_glass_obj spit_object exemption -> glass/blocker combat
      slice; thitu thrower-bracket for the new zero-damage
      projectiles -> kill-attribution slice; puddle/sewage
      no-anchor reachability (webbing "nothing to anchor to"
      ternary is dead until that terrain ports - it is dead on
      pool/lava/air even now) -> terrain slice; explode() BASE_ZT
      water/drain/stun arms (unreachable until an exploder carries
      those adtyps); flash_types wand/spell poison+acid slots stay
      "" (no producers; master filled them). SKELETAL NOTES: hero
      mbreathtyp not saved (youmonst unsaved; lazy reroll = master
      parity; eventual u.mcham-style mirror); chameleon cycling
      accumulates sd_breath_resist mintrinsics bits (master
      parity); master reflection "aborbs" typo fixed on branch =
      MASTER BACKPORT CANDIDATE #8. Original dependency map kept
      below for reference:
      SPLIT from the cooldown slice
      2026-07-17 (maintainer decision; interlocking cluster):
      (a) AD_DRLI/AD_STUN rays onto the reserved AD_SPC1/AD_SPC2
      buzz slots (monattk.h "for extension of buzz()") + zap.c
      beam damage arms + breathwep[] names (master 11 entries:
      "water"/"a dark energy blast"/"a disorienting blast"; branch
      has "strange breath #8/#9" placeholders); shimmering dragon
      AD_STUN breath currently hits impossible("Breath weapon %d
      used") on branch - BZ_VALID_ADTYP is AD_MAGM..AD_SPC2 and
      AD_STUN sits outside it; (b) spit-projectile content:
      SNOWBALL/BALL_OF_WEBBING/BARBED_NEEDLE exist as object data
      only, zero branch src consumers vs master's 5 (dothrow/
      mondata/mthrowu/polyself/uhitm) + spitmm switch cases (snow
      golem/spider/needle blight plines) + the AD_WEBS
      mbreath_attk=rn2(4) setter; (c) skeletal dragon fixed breath:
      sd_breathtypes[]/sd_random_breath/sd_breath_resist/
      sd_assign_breath + breath_adtyp (master mthrowu.c ~1300-1370)
      reconciled with branch get_atkdam_type (mondata.c), hooks at
      makemon.c (~3300 BELL_OF_OPENING block), mon.c newcham
      (~6274, before is_racialmon), polyself.c (polymon
      mbreathtyp reroll; set_uasmon mintrinsics clear+set;
      PROPSET(STUN_RES) - prop exists prop.h:103, never set for
      poly forms); dobreathe/eat.c ubreatheu AD_RBRE resolution -
      branch feeds AD_RBRE straight into BZ_OFS_AD today, hero
      poly'd into skeletal dragon is broken; (d) resists_stun/
      mon_resists_stun mondata.h macros + master's 18-site consumer
      sweep (artifact/dothrow/mcastu/mhitm/mhitu/monmove/music/
      polyself/trap/uhitm/zap - wire only sites whose subsystems
      are live, defer the rest with named owners); (e) the "seems
      lethargic" mspec_used surcharge NIT from the cooldown slice
      (trap.c anti-magic trap + mhitm.c xdrainenergym key on
      AT_BREA but pad mspec_used).
- [x] Ice Queen branch bosses; Vecna; demon-lord behaviors
      (msummon mechanics memory)
      PORTED + REVIEWED + FIXED 2026-07-18 (port subagent 27 files
      +1610/-143 all 8 brief clusters; reviewer CONDITIONAL PASS 1
      blocker/6 concerns/nits -> every finding coordinator-verified
      against both trees and fixed same day; zero-warning make all).
      COMMITTED 2026-07-18 as 8b4283ba8 (maintainer); bundle
      evilhack-50_2026-07-18.bundle cut+verified same day.
      WIZMODE BATTERY RUN + PASSED 2026-07-18 (maintainer-approved;
      one Valkyrie XL30 game + restore cycle, sanity_check on;
      paniclog = ZERO slice entries - its only 2 entries are the
      pre-existing sanctum.lua content gap below). BATTERY RECORD:
      (1) bribe cluster END-TO-END - Mephistopheles genesis
      peaceful+invis with STRAT_APPEARMSG+WAITFORU armed, unveil
      "appears before you", cash-proportional demand (300 of
      10000), paid -> "vanishes laughing" + lltype=4 "bribed
      Mephistopheles with 300 zorkmids" livelog + mongone;
      demon_talk entry-clear proven (strategy=0 mid-flow); Geryon
      gdb-rloc teleport KEPT the flag = rloc_to_core carve-out
      live (vanilla would clear); whisper/spent rloc-death guards
      code-verified only (no lethal-trap staging). (2) IQ cycle -
      m-v-m first kill = livelog SILENT (credit gate) with
      defeat_icequeen=1 set + same-turn dmonsfree revive, same
      m_id, 7500/7500 HP, mstate=0, purge counter balanced,
      malign=-45 = set_malign-after-newcham fix live (A_NONE would
      give 0); Enchantress plain-wizkill -> "not truly dead"
      revive-in-place + guilt (abuse=15, luck-10); candy-bar
      repacify (snatch+eat, peaceful, malign resync); hostile
      Bourbon wizkill -> submit + pacify + 84/84 FULL HP (nit fix
      live; m_lev 21 = recorded adj_lev delta); Enchantress
      peaceful verbalization ("treat the pegasus well") = sounds.c
      arm. (3) IQ petrification edge not staged (ledger). (4)
      hellfire HERO arm - unresistant hero "You're enveloped in
      hellfire!" + "hellish flames sear your soul!" + 2x arm =
      master-parity messages confirmed against fork-point source;
      m-v-m castmm/buzzmm arms not staged live (reviewer-verified;
      Meph's AD_SPEL "Monsters appear from nowhere!" summon fired
      incidentally). (5) castum - hero as barbed devil F-attack on
      test monster: "You cast a spell..." + "enveloped in
      hellfire!" 15 dmg with uen UNCHANGED at 68 = no-Pw-charge
      ruling live; elven-wizard form proved the S_HUMAN use_weapon
      reroute (master mlet list). (6) Vecna - 393/393 genesis
      boost + isvecna; STRAT_CLOSE lair-wait observed (roster
      M3_CLOSE, cleared for test); lich-form gaze "You are
      unaffected by death magic." immune arm, no gaze damage
      (touch damage broke poly = master-consistent); hero kill ->
      "You destroy Vecna!" + remnants message + Hand of Vecna
      (oartifact) + wand of wishing at the square + lltype=6
      "destroyed Vecna" + uvecna=1 via vecnadead() + body crumble
      no-corpse; his potion-quaff + psi-bolt casting cadence =
      boss mspec resets exercised. (7) Lucifer - 666/666 boost +
      islucifer live on the Is_sanctum slot (maze fallback keeps
      the slot + noteleport flag); C1 scroll gate live-starved by
      the UPSTREAM SELECTION gates (noteleport_level +
      mon_knows_traps skip teleport items before the use-time arm;
      identical shape on master = the use-time clause is defensive
      depth) - C1/portal/rise/drum stay code-verified. (8) dlord()
      24-call gdb sample: Juiblex/Baphomet/Yeenoghu/LOLTH/
      DISPATER/BAALZEBUB all appear = widened range live; Geryon
      absent (exists on level) + Mephistopheles absent (G_GONE) =
      exclusion filters correct. (9) player-made trap credit
      BLOCKED from live staging ("magic trap" wish yields the
      deferred KIT object, no digging staged) - thitm umade
      threading + both trap arms stay reviewer-verified. (10)
      xlogfile: achieveX carries defeated_ice_queen AND
      defeated_vecna; killed_uniques=Vecna,Kathryn the Ice Queen,
      Mephistopheles (revive's died++ counts her IQ form = master
      semantics); save/reload round-trip preserved defeat_icequeen/
      uvecna/killed_vecna with cerberus controls 0. BATTERY
      LESSONS: genesis "appears next to you" screens can be STALE
      - C-r redraw before trusting a lingering prompt; #levelchange
      walks one More per level (drain loop, do not resend);
      "sanctum" ^V from Dlvl 1 lands in the Valley first (crossover
      hop); gdb obj chain member is v.v_nexthere (nexthere macro
      unavailable); gdb scripts ABORT at first bad line - macros
      (couldsee, m_canseeu) are not callable, read gv.viz_array
      directly; a failed lspo level leaves the DESTINATION teleport
      queued - it fires on the next input.
      NEW CONTENT GAP found by battery (NOT this slice; owner =
      Phase 1 content / Phase 3 levels): branch roster NAM is
      "aligned priest" but upstream dat luas say "aligned cleric"
      -> des.monster Unknown-monster-id Lua error + maze fallback
      in dat/astral.lua (ASTRAL PLANE = game unwinnable interim),
      sanctum.lua, Pri-loca.lua, minetn-1.lua (41 refs total);
      "cavewoman"/"priestess" ids resolve via female pmnames of
      the merged records so only the aligned-cleric rename breaks.
      MAINTAINER RULED 2026-07-18: rename the roster record - the
      temple entry adopts upstream's full gendered form
      NAMS("priest", "priestess", "aligned cleric") (was the
      Phase 1 single NAM("aligned priest")); the branch role
      monster already matched upstream's NAMS("priest",
      "priestess", "cleric") and upstream itself shares the
      gendered names across both records (name lookups resolve
      in roster order, temple first - same as upstream); the
      mondata.c alias table keeps "aligned priest"/"aligned
      priestess" wishes working. FIX VERIFIED 2026-07-18
      (maintainer-approved launch): all four levels now create
      with ZERO Lua errors - sanctum real level on its slot with
      9 aligned clerics placed, astral.lua full Astral Plane with
      30, Pri-loca.lua and minetn-1.lua load clean via
      #wizloaddes; paniclog never created. COMMIT PENDING
      (rides with this tracker update; message at scratchpad
      commit-msg-cleric-nams.txt). VERIFY-GAME GOTCHAS: the ^V
      menu's "One way stair to The Elemental Planes" entry on
      DoD 1 is the ESCAPE stair - climbing without the Amulet
      ends the game as death=escaped; #wizloaddes on a named
      plane lua TRANSPORTS you there (arrival message + temples
      live) and is the deterministic level-creation check.
      EIGHT MAINTAINER RULINGS 2026-07-18: (1) Ice Queen defeat
      livelog CREDIT-GATED like the killed_* boss lines - moved from
      icequeenrevive into the mondead IQ arm where the consume-on-
      entry attribution locals are in scope (hero "defeated"/pet
      variant/otherwise silent; state + xlog token always set;
      LL_ACHIEVE|LL_UMONST matching sibling arms = deliberate flag
      deviation from master's unconditional LL_UMONST line); (2)
      hellfire binary-resistance endpoints KEPT (no FR = 2x damage,
      full FR = half + soul-sear, zero for demonic/undead heroes -
      master-exact at both endpoints, percentile middle ground lost
      until/unless how_resistant ports); (3) dprince chain-summon
      hellc_entered gate KEPT DORMANT (princes unsummonable until
      Phase 3 wires entry bits - "never entered" is faithful when
      the lairs do not exist); (4) read.c genocide arms STAY
      DEFERRED to new owner "genocide-gating sweep" (G_UNIQ already
      protects the bosses - verified read.c x3; branch monflag.h
      needs G_TALGATH when it lands); (5) summonmu demon gate keeps
      master's 1/10 hell 1/13 else (upstream 16 rejected); (6)
      castum innate elemental casts FREE (master parity; the ported
      unconditional u.uen charge removed); (7) Vecna gaze of death
      deals NO damage through Death_resistance - master CODE wins
      over its own "still does normal damage" comment (BUG-1347
      archive entry, comment-only master mismatch); (8)
      dlord_protected dormant-permanent edge ACCEPTED (once 3 lords
      of a tier die the 4th stays protected until Phase 3 restores
      the lair-entry lift).
      REVIEWER BLOCKER FIXED: monmove.c Demonic-Blackmail arms had
      dropped master's post-rloc DEADMONSTER guards on a false
      premise (the port report claimed branch rloc cannot kill;
      branch rloc_to_core fires mintrap on trapped monsters) - both
      guards restored (whisper arm + spent arm return 1 on death);
      master's is_demon(raceptr(&youmonst)) form also adopted there
      (raceptr semantics verified identical both trees: race when
      natural, form when polymorphed).
      CONCERNS FIXED: muse.c cursed/confused scroll-of-teleportation
      gate gained master's (Is_sanctum && Lucifer) clause (Lucifer
      could levelport out of the sanctum as an escape item);
      icequeenrevive mpeaceful/set_malign moved AFTER newcham
      (master's newcham_deferred polys immediately - only minliquid
      is deferred - so master computed malign on Enchantress data,
      maligntyp 15, not IQ A_NONE; branch newcham never recomputes
      malign); castmm casting-cost/boss-reset/EREGEN block gated on
      AD_SPEL||AD_CLRC per master (elemental m-v-m casts are mspec-
      gated but never mspec-charging; block is dead until the
      spell-list expansion lands - NOTE for that owner: branch kept
      castmu's 10-m_lev formula vs master castmm's 4-m_lev-min-2,
      reconcile then); castum u.uen charge removed (ruling 6);
      gazemu resist-branch damage removed (ruling 7).
      NITS: Bourbon/Ozzy submit arm now sets mhp=mhpmax after
      newmonhp (master monmaxhp gave deterministic full HP; the
      newmonhp adj_lev level-shift delta remains, recorded);
      LIGHTNING substitution broader than master (any FIRE_PILLAR
      pick converts for IQ/Asmodeus vs master's clerical case 11
      only - flavor-consistent, kept); do_earthquake Lucifer/Wizard
      wakeup exemption applies to all callers not just the drum
      (all current callers are drum-driven, kept); buzzmm
      not-lined-up returns M_ATTK_MISS vs master's 1 (cleaner,
      kept).
      PORT KEY FACTS: boss-death handlers x7 + mondead dispatch +
      IQ/Enchantress/Bourbon-Ozzy/Vecna-artifact-drop mondead arms;
      icequeenrevive + dmonsfree interception (MON_DETACH-
      conditional purge-counter decrement, recent_detaches sentinel
      entry); newcham_deferred = NOT-APPLICABLE on branch (5.0
      calls newcham directly in death processing, vamprises
      precedent; minliquid-deferral machinery unneeded) - HP
      7500/15000 set after newcham because branch newcham rescales;
      defeat_icequeen = single appended u.uevent bit (u_event 47
      bits, sizeof(u_event)==8 / sizeof(you)==3408 compile-proven
      unchanged, no sfo/critical_sizes); topten achieveX
      defeated_ice_queen live; do.c tower_entered arm live (In_tower
      exists); com_pager TODO-EVILHACK markers per the wiztower
      precedent (8 msgids: vecna/cerberus/goblin_king/lucifer_
      defeated, icequeen_freed x3 variants, vlad_tower_enter;
      +4 ADDED by the roles slice 2026-07-22: lucifer_demand_idol/
      lucifer_demand_amulet at the monmove.c STRAT_CLOSE arm and
      vecna_hand_evil/vecna_hand at the do_wear.c Hand donning
      narration. 2026-07-23 STATUS: quest.lua data merge + C wiring
      DONE (16 sites, all these msgids live); still pending =
      purgatory_enter/hidden_dungeon_enter (entry arms are Phase 3
      topology; only the you.h bits exist) and the Charon chat
      helper (no branch site yet; recipe:
      com_pager_subset("charon_talk", u.uevent.ucerberus ? 3 : 0) -
      lua array elements 1-3 are the post-Cerberus-safe subset);
      LATENT BRANCH BUG FIXED: vanilla 5.0's dlord range
      JUIBLEX..YEENOGHU on the EvilHack roster reached only 3 of 8
      lords - Lolth and all four archdevil lords were silently
      unsummonable; range now JUIBLEX..MEPHISTOPHELES (roster-
      specific, NOT upstream-reportable); Vecna AD_DETH gazemu arm
      added (roster gaze otherwise hits impossible()); castmm
      extended with elemental m-v-m arms + boss mspec two-tier
      resets + mon_arti_has_spfx EREGEN helper; NEW castum (uhitm
      caller, hero polyform elemental casting, SPFX_HSPDAM halving,
      damage_mon+killed credit) + buzzmm (mhitm caller, gb.buzzer
      bracket, BZ_M_SPELL + adtyp_to_zt, m_lined_up(mdef,mtmp)
      reversed-args correct); wizard.c ice/vecna nasty tables +
      strategy/aggravate/cuss boss arms + Purgatory talk functions
      (dormant, monmove callers need Inpurg = Phase 3); STRAT_
      APPEARMSG bribe one-shot RECONCILED into 5.0 shape (monmove
      whisper gate, demon_talk entry clear, rloc_to_core peaceful-
      MS_BRIBE keep-flag carve-out = master's mnexto carve-out
      relocated to the 5.0 one-chance home; prison-guard exclusion
      preserved; B2 demon_talk body untouched); minion.c Lucifer
      tier + dlord_protected + count_hella/hellb_gone (dormant
      halves) + summonmu 13 gate + PM_DEMON no-chain exclusion;
      small-file gates: music (earthquake wakeup), teleport (sanctum
      portal), zap (poly shieldeff for IQ/Enchantress/dogs), muse
      (rise gate Lucifer half), sounds (Enchantress cuss), dothrow
      (candy-bar repacify + dogs cure), bones (boss no-bones
      allowlist; isvlad already covered via is_Vlad), monmove
      (onscary boss flags + mberserk), trap (fall-past-minetown
      !ugking clamp), potion (Valley cursed gain-level while
      Cerberus lives); mon_xkilled + XKILL_INDIRECT + thitm umade
      param through all 12 callers + magic-trap/anti-magic monster
      arms + explmm exp kludge - uexp machinery live but DORMANT
      until a producer (hero spell parity: ORB_OF_FIRE/FROST) sets
      mon->uexp; mondata.h can_sting/is_defeated + resists_drli
      boss arms; calendar.c kathryn/bourbon/ozzy_bday (5.0 home of
      time helpers).
      NEW DEFERRED OWNERS from this slice: monster innate spell-
      list expansion (MCAST_ ice/fire bolt + CLC_VULN_YOU +
      CLC_SUMMON_MINION arms, innate AD_SPEL/CLRC m-v-m dual-target
      lists, m-v-m geyser, castmm cost-formula reconcile);
      genocide-gating sweep (read.c boss arms + G_TALGATH flag);
      covetous close-range m_move arm + intervene() 8-case rework
      (needs nasty centered_on_stairs) -> Wizard-harassment sweep;
      strategy MS_NEMESIS treasure arm -> quest slice; Izchak
      transform + msick reset -> shopkeeper-behaviors; Gollum cuss
      -> mines content; bones.c non-boss additions -> bones sweep;
      summon_minion woodchuck -> misc flavor; monclock -> its
      slice; Purgatory monmove callers + all *_entered setters
      except tower_entered -> Phase 3.
      INHERITED MASTER EDGES recorded, not fixed: IQ petrification
      leaves a statue coexisting with the revived Enchantress
      (identical flow on master; BUGFIXES candidate with the
      stoning slice); dprince() range spans Tiamat/Graz'zt (both
      trees).
      BATTERY LIST (10 points, run after commit approval): (1)
      bribe-demon rloc death regression - peaceful trapped
      archdevil, hero demon-form, whisper + spent paths, paniclog
      watch; (2) IQ defeat -> same-turn revive -> Enchantress,
      re-kill window, candy-bar repacify, dogs cure/submit loop,
      credit-gated livelog variants (hero kill vs staged pet kill
      vs m-v-m = silent); (3) IQ petrification statue+revive edge
      (ledger only); (4) Mephistopheles hellfire vs resistant/
      unresistant/demonic hero + castmm at pet + buzzmm crossfire
      attribution + cast cadence post-C3; (5) hero Asmodeus-form
      elemental cast - no Pw drain, kill credit; (6) Vecna gaze vs
      death-resistant hero (zero damage) + death drop on
      no-deathdrop level + nasty summon themes; (7) Lucifer sanctum
      suite: aggravate/drum no-wake, portal block, rise gate,
      scroll-teleport block (C1); (8) dlord variety - Lolth/
      archdevils now appear; (9) player-dug trap m-v-m kill - thitm
      umade exp/attribution routing vs environmental monkilled;
      (10) defeated_ice_queen xlog token + u.uevent bits across
      save/reload.
- [x] xlogfile parity slice: achieveX/conductX named-token fields
      DONE 2026-07-17 (coordinator hand-port, small-slice
      precedent; reviewer PASS zero blockers; build green; xlog
      entry live-verified in one approved wizmode game - COMMIT
      PENDING, message at scratchpad commit-msg-xlog-parity.txt).
      KEY DISCOVERY: the 5.0 fork ALREADY had achieveX/conductX
      with master-compatible names for all 15 shared achievement
      tokens - the slice was reconciliation, not a fresh port.
      Landed: 16 live-state achieveX arms (oracle minor/major,
      quest x5 via svq.quest_status + u.uevent, 7 boss u.uevent
      bits, entered_wiztower, got_crowned); conductX to master's
      25-token set (petless->never_had_a_pet, blind->blindfolded,
      bonesless option->numbones semantics, +6 missing arms) with
      fork extras kept (sokoban/pauper/unrerolled - maintainer
      2026-07-17); LATENT FORK BUG FIXED: conductX reused
      buf[BUFSZ]=256, full new token set worst-cases 368 bytes =
      game-end stack smash - dedicated conductbuf[1600] + achbuf
      grown; killed_uniques() ported (svm.mvitals, pmnames
      NEUTRAL->MALE->FEMALE fallback, worm-tail/high-priest
      skips) + killed_uniques/killed_nazgul/killed_erinyes/
      killed_archangels fields; race0= added; do.c wiztower-entry
      arm (write-never u.uevent.wiztower_entered now set, LL_
      ACHIEVE livelog once; com_pager("wizards_tower_enter")
      TODO-EVILHACK until quest text lands - msgid already in the
      quest_to_50 map). MAINTAINER RULINGS 2026-07-17: flags=
      keeps FORK layout (bits2/3 bonesless/reroll; master packed
      bones count in bits 8-15 - intentional v1.0.0 delta, the
      bones=/rerolls= numeric fields carry the data; scoreboard
      note for cutover); forging token stays master's long
      never_forged_an_artifact (settled - it was already live in
      the fork). DEFERRED tokens with owners: entered_purgatory +
      entered_hidden_dungeon (branches unported), defeated_ice_
      queen (Ice Queen slice). BREADCRUMB for the Infidel slice:
      writexlentry align0 uses 5.0's aligns[1 - ualignbase] with
      NO A_NONE guard - Infidel A_NONE (-128) = OOB read; adopt
      master's `== A_NONE ? 3 : 1 - sgn()` ternary when Infidel
      ports (aligns[] index 3 "Una" entry already exists).
      LIVE-VERIFIED (wizmode, quit at T:1 wizard1): race0=Hum,
      killed_uniques=Croesus, killed_nazgul=2, killed_erinyes=1,
      killed_archangels=0, conductX master names no petless/
      blind, achieveX entered_gehennom + entered_wiztower, lair
      livelog single-fire across leave/re-enter, tab-split clean,
      flags=0x5 fork layout. #chronicle does NOT show the boss
      achievements (u.uevent bits, not ACH entries) - accepted
      cost of the ratified representation.
- [x] Pet order system (P_PET_HANDLING, petstrat, #order)
      PORTED + REVIEWED 2026-07-17 (port subagent 13 files
      +1006/-109; reviewer CONDITIONAL PASS 1 blocker/2 nits ->
      blocker coordinator-verified+fixed same day), COMMITTED
      2026-07-17 as 8d0c9504f (14 files +1111/-117). WIZMODE
      BATTERY PASSED same day (one human Valkyrie XL30 game,
      sanity_check on, PANICLOG NEVER CREATED; two save/reinstall/
      reload cycles inside the run; battery record below). Battery
      FOUND BUG-1346 (mfind_target direction-scan clobber, master
      frozen-unfixed since the 2019 GruntHack import) - fix +
      maintainer-ruled dochug-scan pet gate committed separately
      right after the slice; see the archive entry and the battery
      record below.
      Contents: doorder + find_commandable_pet appended to
      sounds.c (5.0 menu API with per-item color param replacing
      master's menuitemcolor global - KNOWN DISPLAY DIVERGENCE,
      maintainer 2026-07-17: the 5.0 add_menu color paints the
      WHOLE menu line; on master only the [active]/[inactive]
      bracket text is colored because the window ports render the
      programmatic item color bracket-scoped (win/curses
      cursdial.c bracket_color "text inside [] only" + win/tty
      twin). Fix rides the Phase 4 menuitemcolor/UI-extras sweep -
      port the bracket-scoped rendering, then doorder inherits it;
      ECMD returns - master 1 ->
      ECMD_TIME incl. "ignores you", refusals ECMD_OK, cancels
      ECMD_CANCEL; skill-gated menu breadth byte-parity with
      P_MASTER bypass; mutual-exclusion toggle arms; belay
      oreserved release; use_skill only on old_petstrat change)
      + cmd.c #order AUTOCOMPLETE entry; exchange_objects_with_mon
      + loot_mon rewrite (pet redirect to #order, wizard-mode
      loot arms, master's saddle-removal-block removal, looted_mon
      -> mon_interact rewiring, 4-arg yn_function) in pickup.c;
      oreserved lifecycle COMPLETE (set exchange-take, cleared
      add_to_minv/extract_from_minvent/give/belay, honored in
      m_dowear_type candidate scan - flat-scan adaptation since
      branch has no find_best_armor_recurse; container recursion
      stays with monster looting); dogmove.c petstrat weave
      (droppables weapon-preference tail = the session-7 pet
      pick-drop oscillation FIX; dog_invent NODROP/NOAPPORT;
      dog_goal COME beeline/NOAPPORT/AGGRO-pursuit; score_targ
      AVOIDPEACE; pet_ranged_attk acceptable_pet_target filter
      incl. the cmd.c forced-attack caller falling through to
      domonnoise; dog_move COME gates + arrival clear, STATIONARY
      attack-in-place block, COWED safecnt, IGNORETRAPS bypass;
      melee arm CONVERTED to acceptable_pet_target - reviewer
      verified strict superset of the removed inline balk/
      floating-eye/petrify checks, master parity at the same
      seam) + the muse-ranged block at master's pre-movement seam
      (find_offensive/defensive/misc + mfind_target natural-ranged
      fallback, all COME-gated; branch-native score_targ/
      best_target/pet_ranged_attk kept unchanged in place = the
      RECONCILE ruling resolved - master itself keeps both paths;
      no double-fire, every fallback action returns before
      pet_ranged_attk); dog.c keepdogs PETSTRAT_STAY stays-behind
      arm + thrown-treat use_skill + initedog petstrat reset (the
      port brief wrongly listed the reset as already-live - only
      newedog zeroing covered first taming; re-tamed ferals kept
      stale orders); monmove.c undesirable_disp IGNORETRAPS
      exception; mhitm.c pet-kill use_skill; trap.c reward_untrap
      use_skill; weapon.c skill_init Healer/Druid/Knight P_BASIC
      block + mon_melee_target apt consumer.
      REVIEWER BLOCKER FIXED: all three muse arms collapsed
      master's ret==0 fall-through into MMOVE_DONE (use_defensive
      has 5 reachable return-0 sites - enexto-blocked create
      monster, Is_botlevel trapdoor, stairway re-validations; a
      cornered low-HP pet with a create-monster wand would burn
      every turn doing nothing) -> ret captured, 0 falls through
      to the normal move, comment corrected. Reviewer confirmed
      the ret==2 -> MMOVE_DONE collapse itself is CORRECT for 5.0
      (MMOVE_MOVED would run postmov/mintrap on an unmoved pet).
      ADAPTATIONS: SUPPRESS_BARDING re-valued 0x80 (master 0x10 =
      5.0 SUPPRESS_MAPPEARANCE; no consumer until the x_monnam
      barding port - NOTE EXACT_NAME 0x1f does NOT cover 0x80,
      reconcile + sweep master's ~28 SUPPRESS_BARDING call sites
      with the steed/barding slice); propellor no-launcher
      sentinel &zeroobj -> &hands_obj; update_mon_intrinsics ->
      update_mon_extrinsics; loadstone exchange kludge tests raw
      otmp->cursed (branch lacks master's silent 2-arg cursed()
      with Infidel/Vecna-glove exemptions - port the helper with
      the Infidel slice); give path freeinv = EXACT master parity
      (both trees' obj_extract_self route OBJ_INVENT to freeinv;
      the port report's "master stale-u.uhave bug" claim REFUTED
      by coordinator and reviewer independently).
      MASTER NOTES: booldroppables dead logic not transplanted
      (set only in an unreachable-from-test branch); loot_mon
      dropping saddle removal for NON-tame saddled monsters
      entirely (outside wizard mode) is master behavior ported
      as-is - possibly unintended on master, maintainer FYI;
      master's dochug muse pre-pass + dog_move muse block double-
      probe exists identically on master - battery watch item
      (rare defensive-use + offensive-use same turn).
      DEFERRALS (named owners): 3 PETSTRAT_COME gates travel with
      their absent master blocks (vampire pet self-feed, grass
      eating, pet helpful-actions) + dog_invent satiation/
      intrinsic-corpse logic + pool/lava/open-air drop avoidance
      -> pet feeding/behavior slice(s); could_use_item/
      is_better_armor terms + find_best_armor_recurse container
      recursion + m_stash_items -> monster looting; W_ARM
      big_wings/dragon-scales/JACKET candidate restriction ->
      monster armor (big_wings absent on branch); worn.c ring/
      amulet candidate blocks -> monster ring/amulet; mhitu.c
      steed-retaliation apt x3 + x_monnam "barded " adjective +
      do_barding plumbing -> steed/saddle/barding; PETSTRAT_STAY
      vs u.usteed coupling censused NONE on master (keepdogs STAY
      arm unreachable for steeds by else-if order); Guidebook
      #order entry ("Give orders to a pet.") -> coordinator doc
      pass via the guidebook workflow.
      WIZMODE BATTERY RECORD 2026-07-17 (maintainer-approved;
      human Valkyrie XL30, starting little dog + genesis'd tame
      dwarf as the workhorse; sanity_check on; paniclog never
      created). VERIFIED LIVE: (1) #order command - auto-pick
      with one eligible pet, getpos targeting with two (farlook
      autodescribe names candidates), wall-select "There's no one
      there to command.", "is incapacitated." alertness abort on
      a donning-frozen pet, exchange adjacency refusal ("You need
      to be next to..."), all no-time; "%s ignores you." at
      P_UNSKILLED consumed the turn (T advanced = ECMD_TIME
      parity). (2) skill gating - menu breadth stepped live
      through all four tiers by gdb skill writes (a-f / +g-i at
      P_BASIC / +j-l at P_SKILLED / +m-o at P_EXPERT), P_MASTER
      obedience bypass; use_skill trains ONLY on changed orders
      (advance counter exact across 4 changes, 0 on ignored/
      cancelled). (3) toggles + exclusions by gdb petstrat reads:
      AGGRO 0x1 -> COWED 0x2 -> NOATTACK 0x200 each clearing the
      others; STATIONARY keeps NODROP (0xa0); belay -> 0 with
      master's discretion message. (4) behaviors - STATIONARY
      held position while hero walked away; COME 0x40 crossed the
      room and SELF-CLEARED at arrival; STAY printed "stays
      behind." at level teleport, pet absent on Dlvl 2, intact on
      return; NODROP held items 15+ turns; NOATTACK silenced
      melee+ranged+muse for 8 turns with a wand and a lined-up
      frozen target available. (5) exchange + oreserved - give
      transfers stack; dwarf donned given shield and ring mail
      (m_dowear pass); take unworn/zero-delay = immediate
      hand-over; take worn ring mail = "begins removing" +
      mfrozen 6-7 + IMMEDIATE unwear + oreserved=1; the
      RESERVATION HELD: 15 turns mobile without re-wearing (the
      m_dowear_type gate live), belay released (oreserved 0) and
      the dwarf re-wore within 6 turns; without NODROP the
      reserved item was dropped by droppables and
      extract_from_minvent cleared the reservation (master
      parity - master droppables has no oreserved term; clean
      handoff flow = take + NODROP). (6) droppables regression -
      spare +2 dagger stack KEPT stable across churn windows
      (preferred-throwable terms live), NO pick-drop oscillation
      (the session-7 loop is gone); pets DROP unheld muse items
      (wand) because could_use_item is deferred to monster
      looting - interim gap, NODROP is the workaround, noted for
      that slice. (7) pet ranged combat - STATIONARY dwarf threw
      daggers at a lined-up frozen target at range 4-5
      (pet_ranged_attk); after BUG-1346 fix + pet gate: "The
      dwarf zaps a wand of striking!" EVERY eligible turn via
      dog_move muse block (find_offensive MUSE_WAN_STRIKING=7 ->
      use_offensive), wand auto-identified in the message, kill
      "The gnoll is killed!" with NO hero XP/attribution;
      pet-handling advance did NOT tick on the zap kill
      (mdamagem melee site only = master parity). (8) save/
      reload x2 (mid-battery reinstalls): petstrat 0xa0, staged
      frozen monsters, wand charges all exact after restore.
      NOT LIVE-TESTED (reviewer/code-verified): NOAPPORT floor
      pickup, AVOIDPEACE, IGNORETRAPS, COWED retreat, dual-wield
      marilith shield-don (dwarf single-wield don observed
      instead; churn starvation root cause is fixed), loot_mon
      wizard-mode arms, friendly-fire walk positive block (the
      dog camping the zap line produced the negative case
      incidentally - tame-in-line direction rejected). BATTERY
      DEBUG LORE (new): dwarves are speed 6 = act every OTHER
      turn - single-turn instrument windows coin-flip; breakpoint
      scripts via -x file + background gdb + pkill -INT -x gdb
      (exact match - `-f "gdb -batch"` also SIGINTs the
      backgrounded wrapper whose cmdline contains the string,
      killing queued tmux sends); never combine a gdb attach and
      tmux send-keys in one compound Bash command (a hung gdb
      strands the sends; they fire at detach or never); pets NAP
      between orders - farlook autodescribe "can't move
      (paralyzed or sleeping or busy)" diagnoses the
      "incapacitated" abort, gdb msleeping/mfrozen clear stages
      around it; the wizard restore "keep save file?" prompt eats
      queued keys after "welcome back".
- [x] Player races: giant, centaur, hobbit, illithid, tortle, drow
      (light/dark auras - litroom memory), vampire, aasimar (wings,
      light aura, seduction immunity, cross-faction hostility).
      MAINTAINER RULED 2026-07-18: split into TWO sub-slices, and
      draugr HERO-race gates (~80 sites, eat.c-heavy; zombie slice
      only owned monster-side zombification) fold into this item.
      R1 = giant/centaur/hobbit/tortle/illithid/drow + litroom
      5-arg aura infrastructure (vision.h TEMP_LIT/TEMP_DARK,
      monmove.c monster aura blocks incl. the aasimar-light twin)
      + shared plumbing (peace_minded 5 racial carve-outs,
      race-picker duplicate accelerators, do_wear illithid msg
      TODO). R2 = vampire + draugr + aasimar (shapeshift forms,
      aasimar penalty system on the live stub, draugr polyself
      re-roll, feeding/food gates; corpse-taint 35%->0% decision
      raised at R2 start). Census 2026-07-18 (master remaining):
      draugr ~80, vampire ~78, drow ~72, giant ~53, tortle ~26,
      aasimar ~25, centaur ~23, illithid ~20, hobbit ~4.
      R1 PORTED + REVIEWED 2026-07-18 (COMMIT PENDING, battery
      next): port subagent 57 files ~+1700/-255 (brief: scratchpad
      port-brief-races-r1.md; full per-site report:
      races-r1-port-report.md - 172 logical fork sites, LIVE 44 /
      PORT 103 / DEFER 19 / N/A 6, zero silent skips). Landed
      beyond the grep: tortle shell-hide subsystem on the existing
      u.uinshell field (Hidinshell/toggleshell/doenshelling +
      ~40 consumer gates; no new saved state), aura infrastructure
      (vision.h TEMP_DARK+UV_SEEN, vision_recalc arms + new
      Ultravision pierce, do_light_sources dark-source
      classification w/ at_hero_range dedup removed per master,
      litroom 5-arg rework on the 5.0 body, blindingflash from
      litroom = scrolls/wands of light now blind light-haters
      (master parity), lightdamage 4-arg w/ monster KILLED_BY
      attribution, monmove drow-dark + aasimar-light monster aura
      blocks (aasimar block = monster-side, rode with the infra),
      dodarkness #monster arm), peace_minded all 5 racial
      carve-outs, race-picker first-unused-letter accelerators
      (full list: drow=r, draugr=a, aasimar=s), spidername option
      (gs bucket, runtime-only), racial intrinsic tables
      (gia/hob/cen/ill/trt/dro_abil), hmonas(mon,as,
      weapon_attacks) racial extra attacks (illithid 1-in-4
      tentacle), SPE_PSIONIC_WAVE made castable (was force-learned
      but unimplemented = latent Phase 1 gap), set_uasmon-based
      mons[] size/speed/cwt mutation (reviewer-verified coverage:
      u_init/restore/polyman; idempotent absolute writes).
      REVIEWER: CONDITIONAL PASS 1 blocker/7 concerns/4 nits, ALL
      coordinator-verified vs both trees + fixed same day: B1
      bhito missing SPE_PSIONIC_WAVE case (impossible() per floor
      pile per cast; fixed w/ master's full res=0 list incl.
      castable-sibling SPE_ENTANGLE; SPE_CHANGE_METAL_TO_WOOD
      case NOT added, cast_metal_to_wood absent); C1 wrong
      deferral - MR_PSYCHIC already on branch (monflag.h 0x100,
      collision-free vs MR2_ space; resists_psychic ported to
      monst.h, direct-bit form since psychic has no *_RES prop;
      all 6 illithid-family entries carry the bit = strictly
      widening swap); C2 false-LIVE - monster-sling-vs-giant-hero
      x2 was missing in m_throw (report conflated hit directions);
      C3 false-N/A - mcast_stun_you Hidinshell gate (5.0
      refactored MGC_STUN_YOU into a helper); C4 psionic 1-in-4
      confusion rider; C5 zap_steed PSIONIC_WAVE+ENTANGLE (NOT
      CRITICAL_HEALING - no bhitm arm yet); C6 psionic exemptions
      x4 in 5.0 idiom (age_spells no-decay, lost-ability msg,
      Amulet-drain skip w/ TODO-EVILHACK Infidel term for roles,
      fatigued-mind msg - string literals master-byte-identical
      incl. double space); N3 options.c blank-line noise
      reverse-patched. N2 REJECTED (false nit: NHOPTC continuation
      indent matches catname; reviewer compared NHOPTB). N4
      accepted-as-is (blindingflash youmonst skip term dropped;
      fmon never holds youmonst). MAINTAINER RULED 2026-07-18
      (C7): Sunsword #invoke blinding-ray + litroom blindingflash
      composite KEPT (exists in neither tree; ratified v1.0.0
      delta). Post-fix full clean rebuild zero warnings.
      FOLLOW-UPS minted by R1: (a) display.c uv_cansee dark-spot
      rendering + MONSEEN_ULTRAVIS not ported - drow currently
      see dark areas as normally-lit (Phase 4 display slice);
      (b) vanilla-race parity gaps: elf Searching@9 intrinsic +
      wield.c orc mithril aversion block (new sweep item below);
      (c) reviewer note: Croesus/jabberwock DATA already carry
      AD_GOLD/AD_BHED with no dispatcher case (damage silently
      zeroed) - their tracker item is latently urgent. DEFER
      owners confirmed by reviewer (17/18 valid): hero regen
      overhaul (elf/orc_can_regen + drow-in-light + Druid
      druid_near_veg grass/next-to-tree heal bonus - named per the
      roles-slice review N3), kick to-hit
      roll, combat miss-flavor rework, steal_it pickpocket rework,
      god-gift subsystem (record_gift), crowning rework,
      Xiuhcoatl throw cluster, Giantslayer/Harbinger/
      awaiting_guaranteed_gift artifact behaviors, larva AT_TENT
      wrap, ma_break, botl_hitbonus, begin_burn light-establish
      lightdamage, gain_guardian_steed, sewage terrain, dual-wield
      offhand weight, druid_form landmine (all_druid_forms()
      unimplemented - roles slice MUST implement before any
      druid_form/vampire_form expansion). R2 (vampire/draugr/
      aasimar) NOT STARTED - raise corpse-taint 35%->0% decision
      at R2 start.
      R1 BATTERY PASSED 2026-07-18 (6 wizmode games, sanity_check
      on; paniclog ABSENT at end; save dir restored): ILLITHID
      Wizard: psionic wave down-zap + beam over floor piles
      SILENT (B1 regression), skeleton warrior "resists your
      mental onslaught" (C1 - non-illithid MR_PSYCHIC carrier),
      lichen no-mind arm, "seems confused" rider ~1-in-4 (C4),
      mounted down-zap hits saddled pony via bhitm (C5), Amulet
      carried: psionic cast NO drain vs force bolt drain-message
      control (C6), Pw-3 "Your mind is fatigued." (C6), retention
      psionic 20000 vs force bolt 19979-and-falling incl. across
      save/reload (C6), racial Fly intrinsic at XL30 + survives
      reload, claw+tentacle extra attacks + "You eat the test
      monster's brain!", helm of telekinesis intensify/diminish
      variants, hint line. TORTLE Monk: hint line, enshell cycle
      (AC -14 -> -54, Blind status via shell), movement
      "constrained by your shell", AC ladder exact (-4@XL1 ->
      -14@XL30 = +10 max), xlog while=hiding in her shell
      (gendered). C3 stun-shrug stays code-verified (apprentice
      caster never rolled the spell vs AC-54). GIANT Barbarian:
      C2 breakpoint-proven (SLING-DOUBLE dam 2->4/7->14/1->2;
      thrown dagger correctly undoubled), BATTERY CATCH (see
      below), boulder quiver GETOBJ_SUGGEST + "at the ready",
      weight_cap()=1050 > vanilla 1000 clamp, base AC 6, St
      18/xx. DROW Wizard (via full picker): 14-race menu all
      unique letters + drow=r reachable, #monster aura (exactly
      10 Pw, both messages), scroll of light blinds+pains a
      gremlin (blindingflash + light-hater pain), Sunsword
      wield = wizmode Override prompt + "You can't handle" +
      blast damage (drow touch restriction), Archmagi wield
      "casts an aura of darkness", cursed magic lamp paints
      TEMP_DARK (viz_array 0x8 bit read via gdb; drow keeps
      sight), lembas "A little goes a long way." master-exact,
      elf corpse "You cannibal!", ^X "on a mission for Lolth".
      DROW Ranger: starting pet = tame large spider "called
      Webster" (spidername option end-to-end, gdb-verified).
      CENTAUR Ranger: "You have too many hooves to wear boots.",
      crossbow multishot "You shoot 2 crossbow bolts." volleys
      at St 18. BATTERY CATCH (fixed same day, incremental
      rebuild zero-warning, re-verified live): wearing chromatic
      scales as giant cloak hit Cloak_on default impossible
      "Unknown type of cloak (144)" - the port added the
      canwearobj wearability exception but missed master's
      Is_dragon_scales normalization in Cloak_on/Cloak_off
      (GRAY no-op + SHIMMERING displacement + CELESTIAL flight
      arms added; check_wings() calls dropped w/ TODO-EVILHACK
      races-R2); sibling sweep also ported read.c
      seffect_enchant_armor worn-scales-as-cloak preference and
      left a TODO-EVILHACK at polyself do_merge (master merges
      cloak-slot scales into uskin; interlocks with uskin
      save/restore - dragon-scales follow-up owner). Post-fix
      giant wear/remove cycle clean, no paniclog. LATENT FINDS
      recorded (NOT this slice): (a) wish parser: "pair of
      leather boots" fails to parse ("leather boots" works;
      "blessed chromatic dragon scales" fails while plain
      works) - R1 objnam diff has no parse changes, so this is
      a naming/wishing-slice preparse gap - follow-up with
      repro strings; (b) welcome() greeting still says
      "welcome to NetHack!" - Phase 0 re-brand gap, sweep with
      the branding follow-up. Battery lessons: EvilHack Wizard
      pet is a pseudodragon so the drow spider override needs a
      dog/kitten role (Ranger) to observe; wished-artifact
      delivery does not blast - the wield attempt does (wizmode
      offers Override); hero attribute staging needs u.acurr
      AND u.abase set (status refresh reads acurr).
      R2 PORTED + REVIEWED + FIXED 2026-07-18 (COMMIT PENDING,
      battery PASSED same day - record below): port subagent 41
      files +2449/-263 (brief:
      scratchpad port-brief-races-r2.md; full report
      races-r2-port-report.md - census branch/master after port:
      vampire 85/98, draugr 109/127, aasimar 34/36; ~200 PORT /
      ~45 LIVE cited / DEFERs to named owners / 2 N/A; all 17
      races-R2 markers consumed; zero silent skips). Landed:
      vampire blood-feeding arc (odrained lifecycle, drain_level,
      blood_freshness, conducts) + FRAIL hunger state + full
      vampire newuhs ladder + shapeshift (#shapechange command,
      vampire_shapechange, u.uvampireshape cooldown, mondata.h
      form-macro block = you.h druid_form/vampire_form landmine
      DEFUSED - M2 flag data was already live) + undead_to_corpse
      vampire tiers + blood-potion arcs + oc_prob restore
      re-apply + vampire pet draining; draugr food gates +
      HLifesaved/ELifesaved split + DRAUGR_REVIVE done() block +
      sentient_arise + DECAPITATED/INCINERATED/DISINTEGRATED
      death codes (producers: zap disintegration breath, Vorpal
      beheading; INCINERATED producer rides with artifacts
      slice) + undead-identity behavior set (temple/shop/watch/
      nurse/ghost-fearless) + starting pets; aasimar penalty
      engine live on the stub (abuse_tier 5/15/30/50 sign-adapted
      to positive-unsigned abuse, apply_aasimar_penalty,
      celestial-blood loss, crowning Death-res revocation) +
      check_wings machinery + racial flight + dolight aura on the
      R1 litroom infra + seduction immunity + cross-faction
      grudge pass. REVIEWER: CONDITIONAL PASS 4 blockers/6
      concerns/5 nits - ALL coordinator-verified vs both trees +
      fixed same day, zero-warning clean rebuild after fixes:
      B1 fallen-aasimar re-force DEAD ON RESTORE (5.0 runs
      role_init BEFORE Sfi_you, reversing master's order the
      role.c guard assumed; re-force added in restgamestate after
      Sfi_you, before set_uasmon consumes race state); B2
      done_eating destroyed the corpse a vampire just drained
      (master keep-corpse arm + "draining" finish verb ported;
      5.0 fire-elemental "consuming" variant kept); B3 savelife
      lost master 52c9c0fae cancel_don() stale-afternmv crash fix
      (draugr revive makes savelife routine = the master repro;
      cancel_don + all 12 do_wear NULL-slot guards re-ported); B4
      newcham grudge arm missing = BUG-1275 class regression
      (peaceful shapeshifter taking demon form near an aasimar
      billed the hero abuse via the moveloop setmangry pass;
      silent flip + set_malign ported, Knight/Infidel terms
      TODO-roles); C1 doeat drain nutrition halving (nutrit
      threading through reqtime/nmod); C2 dog_eat vampire
      meating/4 + nutrit/2; C3 bite no-feed check hoisted above
      the d(2,6) overwrite so the base bite still lands (master
      break semantics; was zero damage when satiated); C4
      isvamp-vs-odrained resume reset; C5 u.uhs FRAIL insertion
      shifts hunger enum = pre-R2 dev saves INVALID (accepted,
      migration no-save-compat policy - regenerate reference
      saves); C6 savelife HUnchanging-clear vs sentient_arise
      permanence logged as known interaction (5.0 behavior kept;
      realistic reach = wizard/discover Die?-n only); N1 mon.c
      site-3 comment dropped for master byte-parity; N3 amr_abil
      80-col wrap; N4 ECMD_OK; N2 stale (dig.c already ruled and
      fixed); N5 dog_eat dead quan>1 block kept for parity.
      Reviewer adjudications RATIFY-QUEUE: !survive guard on
      sentient_arise (deviation from master's unguarded call -
      prevents a successful medallion save being hijacked into
      permanent graverise); adjabil FROMOUTSIDE removal verified
      master-parity with no 5.0 consumer dependency (15-file
      sweep). MAINTAINER RULED 2026-07-18: corpse-taint parity
      restore -100 (see Decisions log). FOLLOW-UPS minted by R2:
      partial-intrinsics system needs an owner (NEW item below);
      hobbit R1 spillover added to the vanilla-race parity sweep.
      R2 BATTERY PASSED 2026-07-18 (4 wizmode sessions - vampire
      Barbarian, draugr Dark Knight, aasimar Valkyrie x2 -
      sanity_check on; paniclog NEVER CREATED; save dir restored
      empty; src/evilhack vs installed playground recipe).
      VAMPIRE: #shapechange hint line; racial Fly at XL30 incl.
      across save/reload; drain arc "You drain the blood" +
      wizard pline "Total nutrition: 100, available: 50" = C1
      halving (proven again 700/350 on giant corpse); B2 proven
      = "You finish draining" + corpse SURVIVES as "a drained
      gnome corpse" in inventory; floorfood "drain it?" verb;
      FRAIL ladder staged via gdb uhunger -150/-350: "You feel
      frail." + Frail Deaf status, "You are starving." + Starved
      Blind Deaf, NO starvation death, recovery feed cleared all
      three afflictions; #shapechange menu warg/fog/bat, bat
      form HD:5 + Stun, early-revert exact HP halving 188->94,
      cooldown cap = 200 exact + wizmode Override prompt +
      persistence/decay across reload (179 post-restore); C3
      proven by logging breakpoint at the no-feed return:
      mhm->damage = 3 and 4 (base bite retained; pre-fix 0);
      species-vampire corpse age diff=100 exact by gdb floor
      -chain walk (vs natural-corpse control 19); oc_prob
      re-apply proven: POT_BLOOD/VAMPIRE_BLOOD/WATER 20/5/92
      identical pre-save and in the fresh restore process.
      DRAUGR: "You have been destroyed..." death wording; innate
      revive END-TO-END ("But wait... Suddenly, you start to
      revive!" + draugr nomovemsg + FULL 134/134 restore +
      umortality=1 + adjattrib toll; lifesave intercepts BEFORE
      the wizard Die? prompt = correct done() ordering); B3
      proven by logging breakpoint: cancel_don fired inside
      savelife with a LIVE stale afternmv=0x4d2120 from the
      interrupted take-off = the exact master crash trigger,
      cleared; dra_abil LIFESAVED/COLD_RES/SLEEP_RES all
      FROMRACE + adjabil telepathy-skip (TELEPAT 0); "Ugh...
      this corpse is too fresh!" gate; gdb-aged corpse ->
      "Mmmm...  tainted meat!  Nutrition gained: 100" (cwt
      ladder exact); skeletal pony starting pet confirmed in
      passing ("You swap places with your skeletal pony");
      Minetown temple trip abandoned (boulder-blocked corridor;
      temple/shop/watch arms stay reviewer-code-verified only).
      AASIMAR: aura hint line; divine-vitality newhp = HP 392 /
      368 at XL30 (vs 188/134 controls); racial Fly at clean
      alignment; #monster light aura "A celestial glow surrounds
      you." 10 Pw; penalty engine tier 1 fired ON the abuse
      charge in one line ("You feel conductive!" shock-res
      strip + monsters-aware + "Your angelic wings wither and
      fade away!" + "less resilient"), abuse=5,
      uaasimar_penalties=0x206 packed, flight revoked next
      status refresh (FLYING bits 0x0); forsake flow at wished
      chaotic altar via #offer with gdb-staged angry god:
      "Thou hast forsaken Me!" verbalize + "Loki accepts your
      allegiance" + "Your bond with the heavens is severed.
      You are but a mortal human now." + ufallen=1 + abuse=36;
      B1 proven: save/reload -> welcome line says "the chaotic
      human Valkyrie", gu.urace human, no racial flight, in the
      fresh process; B4 proven DETERMINISTICALLY: peaceful
      chameleon (cham=472 identity-anchored poll) forced to
      vrock via gdb newcham(m, &mons[429], 0) msg-flag-0 (safe,
      no plines) -> PRE peace=1/abuse=0, POST peace=0/abuse=0 =
      hostile at transform, hero NOT billed; next-turn moveloop
      pass added no charge and no guilt line; ~20 natural form
      re-rolls (Dlvl 1 + Gehennom) all non-demon forms stayed
      peaceful abuse=0 = control. BATTERY LESSONS: chameleon
      natural rolls never produced a demon even in Gehennom
      within ~150 turns (rndmonst demon fraction small) - the
      newcham-msg-flag-0 gdb call is the deterministic driver
      and is SAFE (no message = no More-hang; the 2026-07-16
      newcham ban was about NC_SHOW_MSG); pets kill peaceful
      staging monsters in weak forms (kitten ate the chameleon -
      freeze the PET not the subject); engulfers/mobs eat
      queued rest keys silently - poll svm.moves to detect
      stalled turn-burn; wizard Die? sneaks into blind key spam
      again (answered n = savelife generic wording proven for
      non-undead as a bonus); genesis 'peaceful chameleon'
      spawns pre-disguised (farlook name lies - anchor gdb
      polls on mon->cham).
- [x] SWEEP (new, minted by races R1): vanilla-race parity for
      elf/dwarf/gnome/orc hero deltas vs master + hobbit R1
      spillover. PORTED + REVIEWED + COORDINATOR-FIXED + BATTERY
      PASSED + RE-VERIFIED 2026-07-23 (COMMIT PENDING). CENSUS:
      three parallel
      Explore agents over anchors Race_if x5 races (124 fork
      sites) / is_X(youmonst.data) (22) / racial_X(&youmonst)
      (11) / Race_switch bodies / urace fields / ability tables /
      race-keyed macros; classifications LIVE 94 / VANILLA-NATIVE
      / MISSING / DEFER per census reports; hidden-anchor sweep
      found ONE non-Race_if gap (pray.c apply_racial_substitution
      god-gift consumer, deferred). PORT (11 src files +416/-30,
      port subagent 8 files + coordinator fixes): attrib.c
      elf_abil Searching@9 row + adjabil explicit HUMAN/DWARF/
      GNOME arms (FROMRACE infravision, master comment); wield.c
      setuwep orc mithril aversion; do_wear.c find_ac ART_ASHMAR
      dwarf -7/else -5; dothrow.c 4 Ranger-artifact multishot
      disjuncts (elf/drow x2/orc); eat.c hobbit satiation
      (bite 6000, lesshungry 6000/4500 + "Amazingly, y" keeping
      5.0 !Hunger guard, newuhs 3000 + elevenses HUNGRY arm,
      fprefx FOOD_RATION 3700) + dwarf LEMBAS cram + gnome EGG
      anathema (fprefx + edibility_prompts) + EYEBALL fprefx/
      fpostfx (aasimar celestial-soul death + burn, svk.killer
      idiom) + eatspecial ART_HAND_OF_VECNA burn + doeat
      (u.uedibility || Food_sense) gate with master inner split
      + edibility stoneorslime Eye disjunct + doeat worn Hand
      "part of you" refusal; detect.c food_detect 5 Food_sense
      sites; insight.c permanent Food_sense you_can; hack.c
      maybe_wail powers[] + timeout.c propertynames[] FOOD_SENSE;
      objnam.c readobjnam wish-abuse gate (non_wishable_artifact
      + Ranger racial clause + rn2(u.uconduct.wisharti) +
      Hallucination split) + FULL previous-owner summon arm ~250
      lines (roles[] walk, Magic Mirror align split, Tourist/
      Master-of-Thieves carve-out, 64-case owner switch NAMS-
      adapted PM_CAVE_DWELLER/PM_CLERIC + enum renames, Ranger
      ammo otmp2, mk_mplayer summon, smoke/taunt matrix, post-
      oname erosion re-apply, mpickobj handoff, weapon_check
      strategy, &hands_obj consumed-wish return); mplayer.c
      mk_mplayer is_mplayer refusal REMOVED (master relaxed it so
      quest leaders summon; NONNULLARG1 contract makes a NULL
      guard dead+warning; rank_of falls back to gu.urole = safe,
      arm strips leader names via free_mgivenname; sp_lev caller
      self-gated both trees). REVIEWER CONDITIONAL PASS 2B/4C/3N:
      B1 (leader summons dead vs branch is_mplayer guard) FIXED;
      B2 REFUTED (its "68 live artifacts" counted #if-0 entries;
      real census = 64 live, ALL case-covered, default:
      unreachable) but pm>=0 summon guard added as exhaustive-
      switch-class hardening; C3 eatspecial Hand burn ported;
      C4 = NEW MASTER BUG-1356 (food_detect tautological
      !A||!B tingle guards, De Morgan family; composed form on
      branch); C5/C6 FOOD_SENSE list entries ported; N7 erosion
      comment corrected (branch pass runs pre-oname); N8
      adjudicated (eat.c long-string out-dent precedent); N9
      otmp2 leak master-parity noted. MASTER BUGS: BUG-1355
      hobbit-Ranger Longbow wish (gate tested gnome only;
      race_roles[0] questarti = Crossbow applies to both; branch
      gate tests gnome||hobbit both ways), BUG-1356 (archive
      entries written, master frozen-unfixed). DEFERS confirmed
      to existing owners: elf/orc_can_regen -> hero regen
      overhaul; hack.c ma_break elf/drow prob -> ma_break;
      pray.c god-gift cluster (apply_racial_substitution +
      crowning racial gear rejects + gift material rerolls) ->
      god-gift subsystem; uhitm pickpocket size -> steal_it;
      steed.c racial mount gates (~19) -> steed/barding; elf
      tree-guilt (hack.c burn, zap.c death-ray + dig-ray) + dig.c
      DEADTREE effort -> dead-trees terrain distinction (dig-ray
      site also sewage-entangled). NEW GAP ROWS from this pass:
      (a) mplayer racial NAMING unported - branch get_mplname is
      vanilla Adam/Eve; master namelists[]/rank_of_mplayer/
      endgame tt-name draw missing (init_mplayer_erac itself IS
      live, makemon-called) - ERAC residual, needs owner;
      (b) pager.c ASHMAR artifact-info lines have no owner (other
      ASHMAR residuals route to the existing worn-acurr force-arm
      and knockback-machinery rows). Battery list = reviewer test
      recs: owner summon via switch path (Excalibur) + roles-walk
      path (Mitre; was blocker-1 dead) + erosion transfer, hobbit
      threshold ladder + elevenses, gnome egg anathema + prompt +
      pyrolisk precedence, aasimar Eye death + life-save, polyd
      Hand eatspecial burn, detect tingle suppression x2 casts,
      Food_sense doeat prompt, multishot artifact disjunct, orc
      mithril wield, ASHMAR AC by gdb, save/reload spot.
      BATTERY RUN 2026-07-23 (evilhack-game-tester, 8 games):
      15 PASS / 2 FAIL / 1 PARTIAL -> both catches fixed same day,
      re-verification pass clean. CATCH 1 (A3/A4): quest-leader
      summons fired impossible("bad mplayer monster") - the guard
      relaxation exposed vanilla-5.0's mk_mplayer GEAR SWITCH
      default (master has no gear switch at all - its mplayer gear
      moved to the makemon kit system); FIX = early return after
      christening for !is_mplayer(ptr) so leaders keep their
      makemon gear (also avoids random mplayer kit on leaders).
      CATCH 2 (B1): hobbit 4500 "Amazingly" fullwarn never fired -
      hob_abil grants PERMANENT racial HHunger and the 5.0-native
      !Hunger warn suppression masked exactly the race the arm
      targets; FIX = (racial_hobbit || !Hunger) so ring-of-hunger
      suppression survives for everyone else (master has no such
      guard; hobbit side = master parity). Battery PASS evidence
      highlights: Excalibur owner summon end-to-end (hostile
      knight mplayer, blessed+3 preserved, W_WEP, taunt, no
      freebie), erosion transfer oeroded=1/oeroded2=1, wisharti
      denial 'n' path + increment proof, choke 6000 live (5900
      ration -> choke, survived 1/20 vomit arm), SATIATED flips
      at 3000 exactly (gdb 2862 vs 3099), elevenses + supper
      flavor, ^X/#wizintrinsic/prompt Food_sense consumers,
      BUG-1356 fix proven (starts -> continues wording), gnome
      anathema + prompt + pyrolisk precedence, aasimar Eye death
      life-saved + orc burn 196 control, worn-Hand refusal +
      cube-form eatspecial burn, orc mithril luck -1 bare vs
      gloved no-change, elf Searching gain@9/loss@8 both
      messages, dwarf Ashmar botl delta 7 vs orc 5, Longbow
      volleys 2/3/4 vs plain-bow cap 2, dwarf poly infravision
      retained, save/reload clean. A4 arrows expectation was a
      BRIEF ERROR not a defect (Longbow is roles[]-claimed in
      both trees; ammo switch case is dead on master too).
      OBSERVATIONS (no action this slice): wish-delivery
      cross-align artifact blast = by-design (session-18 ruling);
      "leather gloves" wish -> riding gloves + "material leather
      not valid; using iron" note = possible naming-slice
      follow-up, not race-related; mksobj egg corpsenm random =
      vanilla. RE-VERIFICATION 2026-07-23 (2 games) 4/4 PASS:
      Mitre summon clean (Arch Priest, Mitre WORN on head, no
      impossible, makemon caster gear - Arch Priest msound=36 is
      not MS_PRIEST so a random caster kit is its natural gear,
      the brief's mace/armor line was illustrative only), Orion
      + Longbow clean, hobbit "Amazingly" warn fires with innate
      HHunger proven set, ring-of-hunger control still suppressed
      for non-hobbits. Paniclog byte-identical to baseline.
- [x] Partial-intrinsics/percentage-resistance system PORTED +
      REVIEWED + BATTERY-PASSED 2026-07-24 (COMMIT PENDING).
      Port subagent 26 files, final +1286/-262 with coordinator
      fixes; brief/report/battery-brief in the session scratchpad
      (port-brief/partial-resistance-port-report/battery-brief-
      partial-resistance.md). Census = 3 parallel Explore agents:
      25 producer sites, ~252 consumer calls gate-style-classified,
      branch-state map. Classification 118 PORT (+2 reviewer
      completions, +2 gaze-arm siblings) / 14 LIVE / 22 N/A /
      7 DEFER; all 5 forging-debt percent-resist sites converted;
      boss-slice hellfire percentile middle ground restored.
      MAINTAINER RULINGS 2026-07-24 (Decisions log): how_resistant
      honors FROMOUTSIDE as 100% (master quirk = BUG-1357, fixed
      on branch); TIMEOUT bits read as percentage ONLY under
      HAVEPARTIAL - plain timed intrinsics (5.0 lava hatch,
      mid-meal acid/stone res) read 100 while active and expire
      normally. Port-subagent/reviewer lava-hatch "clobber" claim
      REFUTED twice: the write is gated on !Fire_resistance and
      any partial percentage makes that macro TRUE (reachability,
      not set_itimeout semantics). REVIEWER CONDITIONAL PASS
      0 blockers/4 concerns: C1 passive() AD_SLOW orange-dragon
      case missing (fixed master-exact), C2 sink_into_lava Upolyd
      u.mh third-ing dropped (fixed), C3 = the refuted clobber,
      C4 report staleness (refreshed). Census verified complete
      by the reviewer at the anchor level (per-file master-vs-
      branch symbol counts all reconciled).
      BATTERY (game-tester agent, 2 games: Valkyrie XL30 T:389 +
      draugr Barbarian XL30 T:159): 16 PASS / 1 PARTIAL / 1
      caveat. Proofs: corpse +50 steps to exactly 100 with the
      chill ladder; draugr 1/3-rate accumulation, cap write-back
      to 50, 1.5x-then-scale = 0.75x composite; ring-eating +50/
      +50-to-100 and free-action sleep 100; crowning = five
      100|HAVEPARTIAL grants (not FROMOUTSIDE) + FROMOUTSIDE-
      reads-100 ruling proof on staged sick-res; disenchanter
      attrcurse 50-100 point strips + FROMEXPER stripped outright;
      DISN three tiers incl. shield sacrifice, crystal-plate
      "remains intact, but you do not!" death, engulf single-
      application (BUG-1312 shape); hellfire (dmg+1)/2 at 100% +
      resist_reduce*2 partial; explode mask exact at the 51/50
      boundary; six %d%% enlightenment lines + FROMFORM
      vulnerability attribution (90-50=40 arithmetic); sleep
      duration halving via breakpoint; poisoned() 100/40/20
      ladder incl. instadeath + max-HP-loss signature; timed
      grant reads 100 and decays in 5 turns while 40|HAVEPARTIAL
      never decays over 120; save/reload bit-exact for both
      forms; AD_SLOW ~1/3 rate 10/31; lava polyform u.uhp AND
      u.mh to thirds. PARTIAL = draugr hellfire dmg=0 control:
      preconditions gdb-proven (soul_exempt + extrinsic ring =
      how_resistant 100 overriding the cap), deterministic branch
      gate, but Asmodeus never cast fire in the draugr game
      (melees when adjacent, ~60 turns + logging breakpoint
      empty; repro recipe in the battery report). Caveat =
      orange-scales defended(AD_SLOW) control unstaged ("orange
      dragon scale mail" wish returns plain scale mail - naming
      latent family, joins 'pair of X').
      BATTERY CATCH fixed same day: beholder AD_CNCL gaze hit
      impossible("Gaze attack 45?") x4 paniclog ("Program in
      disorder!") - latent Phase 1 class: generating monster,
      unhandled attack arm. Master gazemu AD_CNCL arm ported +
      sibling AD_LUCK (magical eye ominous glare, same class);
      reactions[9] "lackluster" was already pre-staged by the
      port. AD_STON gaze needed nothing (generic Medusa arm
      covers the beholder - port report Q5's STON half was
      wrong). RE-VERIFY 2026-07-24 (game-tester agent, 1 game,
      paniclog ZERO): 4/4 PASS - strange gaze forced titan-form
      rehumanize; mcan gazer drew "looks lackluster" with no
      cancellation; glare stepped uluck 0 -> -1 (breakpoint-
      logged change_luck(-1)); blessed wielded mirror drew
      "sees its own glare in your mirror" + "is cancelled!" +
      flee with no further luck loss. Driving note: the magical
      eye's AD_STUN gaze shares the mspec_used cooldown, so the
      LUCK glare is rare unassisted (master-identical; staged by
      zeroing mspec_used via breakpoint).
      UPSTREAM 5.0 BUGS +2 (table): muse.c scroll-of-fire mangled
      if/else (fixed here via census conversion), mhitm_ad_slee
      doubled sleep_monst condition (recorded, monster-side).
      MASTER BUGS: BUG-1357 (crowned sick-res reads 0% on the
      percentage path, fixed on branch per ruling), BUG-1358
      (freezing-mist >=50 arm dead rn2(3) store, master-only,
      intended shape rides with the trap-types owner).
      RATIFY QUEUE: poisoned() drops 5.0's survived-arm CON
      drains (master parity); lava-sink monstseesu keyed on
      boil_away kept (branch improvement over master's
      unconditional call); "quaffing a burning potion of oil"
      killer string kept over master's shorter form.
      WATCH: Valkyrie baseline COLD_RES intrinsic read
      FROMEXPER|FROMFORM before any polymorph (reads 100
      correctly; FROMFORM origin unowned, battery observation).
      LESSON: make install wipes var/ INCLUDING paniclog - a
      battery brief must not promise a stale-line baseline across
      an install; tester correctly re-baselined at 0.
      DEFER owners (nothing silent): freezing-mist trap arms +
      BUG-1358 shape (trap-types row below); CLC_VULN_YOU
      Vulnerable_* producers (monster innate spell-list
      expansion); cancel_monst stripping block incl. Vulnerable_*
      clears (cancellation slice - TODO enumeration extended in
      zap.c); Dragonbane-gloves passive, passive_obj AD_DISN,
      AT_NONE AD_CNCL passive arm (monster-cluster); monster-side
      vulnerability scaling (damage_mon core). Dev-save note:
      pre-slice binary FROMOUTSIDE elemental grants now read 100
      via the FROMOUTSIDE ruling (benign, no invalidation).
- [x] Roles: Convict, Infidel (penance/Amulet gate memory), Druid
      PORTED + REVIEWED x2 + COORDINATOR-FIXED 2026-07-22/23 (COMMIT
      PENDING, battery next): port subagent 42 files +1716/-222
      (brief: scratchpad port-brief-roles.md; report:
      roles-port-report.md - 23/23 markers consumed, LIVE 33/PORT 95/
      DEFER 26/N-A 3) -> reviewer FAIL (census anchors Role_if +
      "wildshape" missed A_NONE-keyed Infidel sites and druid_form/
      vampire_form-keyed usability sites; 5 blockers incl. a FALSE
      port claim on prayer_done) -> fix pass on the resumed subagent
      (fix-brief-roles.md; re-census with BOTH missed anchors found
      FOUR more roles-owned gaps, all fixed: files.c livelog aindx
      OOB, gcrownu A_NONE resistance gate + curse-not-bless,
      sacrifice anger/ublesscnt divisors, dopray crimson-glow +
      coaligned water-curse arms) -> focused re-review CONDITIONAL
      PASS (roles-rereview-report.md) -> coordinator applied the
      completion set same day (Cursed_obj macro + Glib arm, do_wear
      9-site + loadstone 6-site + pickup-stash cursed() conversions,
      helper Hand-of-Vecna carve-out + GOGGLES plural, objnam
      carried() merged-suffix guard, gcrownu maybe_erodeproof,
      engrave freehand parity). FINAL: 59 files +2144/-390,
      zero-warning clean build. MAINTAINER RULINGS (Decisions log):
      pauper Infidel Amulet-at-feet (implemented in newgame() after
      u_on_upstairs, once-only, never on restore); imbue state =
      u.uevent.uimbued_idol bit (sizeof(you) 3408/u_event 8
      UNCHANGED; carrying_imbued_idol() retired; full master
      uachieve.amulet census swept - artifact.c x3/potion.c/
      teleport.c:917 arms stay with the artifacts owner, bit-exists
      note recorded); Amulet first-possession WISH GRANT REMOVED
      (first 5.0-behavior reversion; bit + u_init zeroing dormant).
      KEY LANDINGS: #wildshape end-to-end + armor-merge/form-
      usability layer (~30 sites) + missing R2 vampire twins
      (rehumanize +1000, appr=0, take-off block, fog door passage);
      Moloch prayer/sacrifice/crowning arc (gcrownu A_NONE
      demonization via race_demon, restore re-force after Sfi_you on
      uhand_of_elbereth==4); Idol imbue at Sanctum + chase-gate
      conjuncts; Infidel cast-from-HP; aligns[] A_NONE OOB x7 fixed
      (crash class); cursed() converted to master 2-arg silent form
      across ALL consumers; inf_align permutation; quest-leader
      forfeit; Convict watch/vault/shop/food/pet/ball behaviors;
      SPE_SUMMON_ANIMAL/ELEMENTAL castable (latent Phase 1 gap);
      Knight/Infidel grudge arms (R2 B4 stubs consumed); grudge loop
      restructured with aasimar gate inside. RATIFY QUEUE: sliparm/
      breakarm + stuff_prevents_passage hero-global druid_form/
      vampire_form macros inside general monster predicates kept
      master-exact (scoping to mon==&youmonst would be a deviation);
      apply.c doapply-head form carve-out (5.0-only blanket gate,
      carve-out preserves master usability - exists in neither tree
      verbatim); pray_revive runs only in the non-A_NONE coaligned
      arm (5.0-only feature; Moloch-doesn't-succor reading);
      sounds.c chat fall-through-to-domonnoise delta; mysteryforce
      decay term kept + A_NONE exemption (standing collision rule).
      DISPOSITIONS: prayer_done -1 arm stays vanilla-5.0-exact
      (master's praying&& gate + HUnchanging forced-death unported;
      5.0 rehumanize covers intent); ACH_AMUL full-pack-imbue edge
      keeps master's OBJ_INVENT gate (bit set unconditionally heals
      master's own state-loss edge); ex-giant crowned demon mons[]
      cwt/mmove residue until reload (master-shared). NEW OWNER ROWS
      (from re-review + re-census, nothing silent): cursed
      wand-of-light darkness arms (wands/light parity); bearded-axe
      shield hook (muse/combat parity); master luck-timeout rework
      (misc parity, pre-existing); break_armor boots trigger
      widening + makemon S_JABBERWOCK-awake/S_NYMPH-satyr/S_EYE-
      beholder case shapes (races/monsters parity sweep); Priest
      pierce/slash cursed-slip term (Priest parity); amulet-escape
      subsystem (mon_escape_with_amulet + uamulet_on_planes gates -
      NEW owner minted by pass 1); meat-armor worn-suffix display
      parity. SEQUENCING FLAG (role-critical): Infidel ascension =
      Idol CHANNEL invoke, Convict Liberation-shirt = PHASING invoke
      - BOTH live in the artifacts slice; it must land before the
      roles are player-visible. quest.lua data merge = ruled
      immediate follow-up commit (converted file in
      /root/nh50-port-preview merges with branch hardfought deltas;
      +4 msgids added to the boss-slice follow-up list). Battery
      list: port report battery section + fix addendum deltas +
      re-review additions (cursed-gauntlets prayer, ring-over-cursed-
      gloves, loadstone menu-drop split, monster-armor probe suffix,
      Hand of Vecna T-off, goggles plural).
      BATTERY RUN 2026-07-23 (evilhack-game-tester agent, 5 games,
      sanity_check on; full evidence in scratchpad
      battery-report-roles-quest.md - transient, verdicts recorded
      here): PASSED with 2 real catches, both coordinator-fixed
      same day (fixup commit follows): (1) Shirt_on/Shirt_off
      exhaustive dev-switches lacked STRIPED_SHIRT - every Convict
      start hit impossible + paniclog (Phase 0.5 make_corpse
      dev-switch class; the object predated any wearer); (2)
      doloot_core kept the vanilla-5.0 raw nohands gate = a
      wildshaped Druid could not loot (master routes through
      u_handsy; adopted); plus the pre-game confirm header showed
      "chaotic Infidel" (plsel_startmenu = the MISSED 5TH
      special_alignment consumer - master's tty twin had it; 5.0
      relocated that code into role.c). ADJUDICATED MASTER-PARITY
      (battery-brief expectation errors, no code change): Knight
      peaceful-dragon grudge flip charges abuse via setmangry
      (flip body byte-identical both trees; tame arm free);
      woodland-guilt event totals 6 = 5 peaceful-anger + 1
      woodland rider. PASSED highlights: imbue end-to-end (livelog
      + uimbued_idol + save-survival; the imbued Idol is
      deliberately non-containable, so the stashed-Idol regen
      delta is MOOT in practice - regen proven with Idol carried),
      crowning->demon persists across reload, death-writers all
      show align=Una (aligns OOB fix proven), pauper floor-Amulet
      + no re-fire on reload + no wish at pickup, all four role
      legacy texts + altstart routing, wildshape armor-merge +
      in-form usability, metal-cast 100% fail, summons castable,
      vampire #shapechange + the N4 double-space string exact.
      SKIPPED (named): quest-leader forfeit + Convict shop/watch
      town trips + angel_cuss live gating (all reviewer-verified;
      staging heavy). NEW LATENT FIND (naming/wishing preparse
      family, joins 'pair of X'): "Bracers of the First Circle"
      and base "runed bracers" do not wish-parse - battery worked
      around with leather bracers. Druid #twoweapon is role-blocked
      outright, so the C1 bracers-shield fix is only reachable by
      non-Druid bracer wearers - C1 verified that way. Paniclog
      end state: 1 tester staging artifact + the shirt entry (now
      fixed); nothing else.
- [x] ARTIFACTS SLICE (invokes + internals + combat behaviors) -
      was URGENT (Infidel ascension + Convict shirt live here).
      BOTH COMMITS LANDED (A de7ab107f, B d025b6158) + COMBINED
      BATTERY PASSED 2026-07-23 (evilhack-game-tester agent, main
      round 5 games + supplement round 4 games; zero crashes/
      panics; one battery catch = BUG-1354 fixup pending below).
      BATTERY RECORD: main round proved the imbue/re-arm arc,
      BUG-1352 wielded/quivered Idol channel, DEATH_MAGIC full
      matrix (immune/MR/plain/peaceful + ABUSE_VECNA/#conduct +
      luck backfire death), aasimar denial, opposed/Orcus twoweap
      refusals byte-matched, resistance-ladder cures, PHASING both
      carriers, COMMAND_UNDEAD, acid/disint/fuse/Orcus/DFLAGH
      instakill cores, Fire Brand vulnerable_to severity.
      SUPPLEMENT proved: (S1) REAL-Astral Infidel ascension
      end-to-end - wrong-altar god_zaps_you arm + matching-altar
      power-increase -> altar conversion -> "Archfiend of Moloch"
      done(ASCENDED), dumplog/xlog (death=ascended (with the
      Idol), full achieveX) - THE ROLE IS NOW PLAYER-COMPLETE;
      (S2) drow Shadowblade fear-ray + darkness + per-object glow
      incl. OFF-HAND Orcrist glow (W_SWAPWEP arm live), blind
      quivering, per-blade enlightenment; (S3) twoweap extrinsics
      cycle (grant/revoke/throw-return-re-engage/clean re-wield
      revoke via gdb extrinsic masks), Monk Eyes worn-MR
      defends(AD_MAGM) worn-vs-carried split. ADJUDICATIONS:
      wished-artifact touch-blast = shared master machinery
      by-design; wizmode tele bypassing the sanctum lock = the
      guard's own !wizard term (master parity); Tempest adjacent
      polearm bash NEVER enters artifact_hit (gdb-proven; shared
      vanilla polearm routing - specials + 1/6 surge PROVEN on the
      range apply incl. unreined pet catch at unskilled) =
      NOT-A-BUG; no dedicated ascension livelog = 5.0 dump-livelog
      mechanism, master has none either, no gap. BATTERY CATCH:
      BUG-1354 poly-primary-to-ammo re-engaged twoweap ->
      "Two-weapon insanity" paniclog (slice-B reseat gated
      !bimanual only; fixed via exported twoweapon_ok() at both
      poly_obj arms - archive entry). KNOWN REMAINING GAPS
      (named, accepted): off-hand doname glow suffix (objnam
      parity owner, deviation 9 confirmed live); Healer Staff
      Withering-cure invoke + Xiuhcoatl DEX-gate live runs
      (reviewer-verified, role-staging only); B8(b)
      destroy-primary reseat (B8(a)/(c) proven). NEW OPEN ROW
      (NOT artifacts scope): drow + worn blindfold does not set
      the BLINDED extrinsic (potion blinds fine; Eyes W_TOOL
      ANTIMAGIC extrinsic grants fine, so setworn is healthy -
      blindfold/BLINDED-specific, drow tested only; verify vs
      master + non-drow -> races/do_wear owner).
      Ruled 2026-07-23: TWO commits (Decisions log; also off-hand
      extrinsics parity, per-object glow, drop-proof sanctum locks,
      cspfx ratify). Full census in the session plan file
      (feature-evilhack-50-artifacts-slice); briefs =
      scratchpad port-brief-artifacts-a/b.md.
      COMMIT A (invokes + hero-side internals) PORTED + REVIEWED +
      COORDINATOR-FIXED 2026-07-23, COMMIT PENDING (16 files
      +719/-50, zero-warning): five invoke cases live
      (SHADOWBLADE fear-ray/darkness; PHASING Staff/Liberation w/
      Convict gate + master phasing_dialogue ported to timeout.c;
      CHANNEL full altar ladder - Infidel ascension via
      done(ASCENDED), Sanctum re-assert, god_zaps_you arm, altar
      conversion, off-altar use_figurine; DEATH_MAGIC Eye/Hand AoE
      with monkilled no-XP intent, ABUSE_VECNA + ABUSE_SPELL_
      PEACEFUL producers = the alignment-abuse deferral closed;
      COMMAND_UNDEAD 3x3 tamedog); fix_artifact + 3 callers;
      vecna_relic_denied x5 (census: master's 5 sites are ALL in
      set_artifact_intrinsic/arti_invoke - agent's defends() label
      was wrong, port placed correctly); set_artifact_intrinsic
      dtyp ladder completed (ACID/STON/DISE/DETH/STUN/FUSE + cures
      + antimagic conduct); spec_ability cspfx OR; arti_align +
      obj.h alignment macros + can_twoweapon/hmonas opposed-align
      + Wand-of-Orcus gates; sanctum locks drop-proof via NEW
      achievement_recorded() helper (insight.c); use_figurine
      exposed + master Idol arms (apply.c survival w/ setnotworn
      fix = BUG-1352 master-latent; dog.c ndemon(A_NONE) channel,
      cgend gated vs spe==1 imbue-marker collision); twoweap
      extrinsics machinery (setworn predicates w/ master's real
      unconditional teardown, dotwoweapon/untwoweapon/NEW
      retwoweapon brackets, dothrow return-cycle callers, detect.c
      offhand search); invoke_taming blessed + invoke_healing
      Withering; awaiting_guaranteed_gift dead-by-design (consumer
      pray.c:2609 = prayer owner). REVIEW: CONDITIONAL PASS, 1
      blocker (BUG-1352) + all concerns fixed or assigned same day;
      reviewer verified the reseat-caller census closes (no missed
      site) and CHANNEL/DEATH_MAGIC ladders arm-by-arm.
      NEW OWNER ROWS from review: figurine-taming parity (master
      non_tameable() + petless-conduct `you` param absent on
      branch; branch initedog livelogs the conduct break instead
      of preventing it - pets/taming parity owner); pager artifact
      encyclopedia (name_to_arti/get_artilist_entry) joins the
      pager recipe-lookup deferral. COMMIT A COMMITTED de7ab107f.
      COMMIT B (combat behaviors) PORTED + REVIEWED + COORDINATOR-
      FIXED 2026-07-23, COMMIT PENDING (32 files +1848/-236 incl.
      fix round, zero-warning): mon_wielding_artifact + the
      mon_*_wield boolean layer via 16 PORT clusters (full site
      table in artifacts-b-port-report.md; 11 DEFERs with named
      owners incl. monster ring-wear extra_pref stub, mrndcurse,
      monster chest_trap, Gjallar, AD_BHED/AD_LOUD executors,
      combat-knockback muse/zap TODO sites, begin_burn light
      slice); ALL remaining artifact_hit arms (named FIRE/COLD/
      ELEC + vulnerable_to severity, Angelslayer/ash instakills,
      Orcus AD_DETH soul-consume w/ monkilled no-XP, Dirge/
      Harbinger AD_ACID + giant dissolve, Annihilation AD_DISN +
      Rider reintegrate + shared disint_mon_invent w/ container
      spill, Dichotomy AD_FUSE, DFLAGH/DCLAS instakill switch,
      Vorpal Cerberus, DRLI naming, three AOE surges on the
      ge.exploder bracket w/ BZ_U/M_BREATH + friendly_in_blast);
      u_wield_art RETIRED (29-site sweep, per-site master forms,
      uwep-only judgment calls reviewer-confirmed); attrib STR
      trio + is_botl_artifact status wiring + insight A_STR
      cursed-hide arms; corpse_chance Trollsbane/Sunsword/Hammer
      + zap.c revive() troll gate; Xiuhcoatl throw cluster;
      score_artifact_weapon re-sync + NEW vulnerable_to()
      (mondata.c; hero via gy.youmonst - Vulnerable_* props now
      live); resists_magm defends+W_BARDING; per-object glow
      subsystem (Sting_effects + gw.warn_obj_cnt retired
      tree-wide; see_monsters per-object loop; set_artifact_
      intrinsic full recalc closes the Commit A transient-clear
      concern); twoweap stuck-state sweep + setworn same-obj skip;
      hero Tempest stun immunity at the make_stunned choke point;
      m_is_steadfast Harbinger/off-hand extension. REVIEW:
      CONDITIONAL PASS -> all findings fixed same day: BLOCKER
      disint macro CRYSTAL_PLATE_MAIL term restored (deviation-6
      claim was false - crystal plate oc_oprop is 0 both trees);
      BUG-1344 regression caught by COORDINATOR pre-review (the
      B brief wrongly ordered mdiseabyu re-keyed to master's buggy
      ART_GRIMTOOTH form; youattack fix restored); Mjollnir wake
      youdefend coords; BUG-1353 hero-defend instakill message
      gates x3 (archived, master frozen-unfixed); mthrowu
      returning-weapon double-kill tail guard; insight/is_botl
      census riders ported; stale vulnerable_to TODOs updated;
      hack.c web-burn names the responsible blade. OPEN FOR
      MAINTAINER AT COMMIT: (a) AD_DETH melee kills route
      monkilled/no-XP like the invoke (master parity - confirm
      the Commit A ruling extends); (b) ratify the container-spill
      upgrade now shared by black-dragon-breath disintegration.
      NEW DEFER OWNER ROWS from the B review/report (nothing
      silent): artifact-bane aura arm of master's allmain moveloop
      grudge loop (absent; interlocks w/ the races bane passes);
      mthrowu thrown Dirge/Harbinger iron-bars dissolve;
      vulnerable_to remaining consumers (damage_mon central 1.5x,
      mhitm/mhitu/uhitm/zhitm arms, polyself PROPSET, priest.c,
      insight report rework); should_cleave() conjunct at the
      Cleaver hitum line; Vecna-relics/Ashmar/Retribution worn
      acurr force-arms; mon_regen !mwither gate (wither owner);
      corpse_chance minsects/alhoon-Kas-crumble/open-air arms.
      Battery AFTER B commits (game-tester agent; combined brief
      READY at scratchpad battery-brief-artifacts.md A1-A13 +
      B1-B12).
- [x] EvilHack trap types: BOLT/ICE/SPEAR/MAGIC_BEAM_TRAP_SET +
      trap-kit object application. PORTED + REVIEWED + BATTERY
      PASSED 2026-07-24 (maintainer commit pending; msg at
      scratchpad commit-msg-trap-types.txt). BATTERY (3 games,
      sanity_check on, evilhack-game-tester): 7 PASS 1 PARTIAL -
      landmine bones B1 proof (bones pit ammo nil, boulder fill
      clean, no impossible), ammo persistence exact across
      save/reload (7->7) + empty-store click removal, crafting
      matrix incl. BUG-1360 quivered-10 clean + BUG-1362 spe
      floor + charge-on-fail (5->4), set/untrap cycles (beartrap
      recover, ammo TAKE merge "11 in total", spear Luck
      take-vs-break, cursed bungle, low-DEX fall-apart), magic
      beam BUG-1359 quiet removal + wall-launch beam + unique
      beam-kill livelog-SILENT (Yeenoghu), ice trap 100%-res
      flash-freeze dmg 0 + erosion 100->66->31 + sub-50 scaled
      + airborne fly-over (ratified) + BUG-1361 UNSEEN MR_COLD
      strip proven (blind hero, giant soldier 2->0), display
      (colors bolt cyan/ice bright-cyan/spear brown/beam
      bright-yellow, ^F, hallu, save/reload glyphs). T7 PARTIAL:
      hero arm + Stoneskin break-off PASS; mounted-steed + ERID
      arms unstaged (mount friction; code reviewed) - fold into
      the steed/barding slice battery; flying-dragon "isn't long
      enough" = MASTER PARITY (Flying checked before
      thick_skinned in both trees, brief expectation was wrong,
      not a bug). Paniclog = 1 staging artifact only (gdb-forced
      material 7 darts, explained + restored; next install wipes
      it). WATCH ITEM: Game A (Valkyrie dlvl2 T:9) died with NO
      bones prompt while identical Game B prompted - one-off,
      unattributed, upstream gating suspected; watch on future
      batteries. Port subagent 23 files
      +1307/-311; reviewer CONDITIONAL PASS 1 blocker/5 concerns
      -> ALL coordinator-verified vs both trees + fixed same day;
      zero-warning make all after fixes. CONTENT: enum inserts
      BOLT=2/ICE=12/SPEAR=25/MAGIC_BEAM=26 with 5.0 tail kept
      (VIBRATING=27, TRAPPED_DOOR=28, TRAPPED_CHEST=29,
      TRAPNUM=30; ttyp 5-bit budget = 2 more values); defsym.h 4
      PCHAR inserts + renumber (map in port report; tiles Phase 4
      per S_forge precedent; cmapname/altlabels self-populate from
      defsym.h - no implicit-array edits needed); trap->ammo
      persistent model (OBJ_INTRAP, set_trap_ammo/
      deltrap_with_ammo + DELTRAP_* enum, savetrapchn/getlev obj
      chains, sfdata regen with genericptr ammo per erid
      precedent, obj-sanity trap-ammo walk; struct trap grew ->
      DEV SAVES INVALID, accepted); existing arrow/dart/rock
      traps now fire-from-ammo-and-deplete via shared
      trapeffect_shooting_trap; trap_fusions[] 23 recipes (all
      otyps verified live) + use_trap_kit on 5.0 getobj API;
      use_trap/set_trap extended beartrap+landmine -> all 14 trap
      objects (traptype_of_obj helper, trap_ammo_typ restore,
      material/oprops/spe transfer, use_unpaid_trapobj billing
      kept); trapeffect_ice/spear/magic_beam handlers (hero/
      monster/steed/ERID); branch-native switch arms
      (floor_trigger, m_harmless_trap, immune_to_trap,
      steedintrap, dowebspin, traptype_rnd/roguelvl, hurtle_step,
      cmap_to_roguecolor); mklev gates SPEAR>=6/BEAM>=12/rogue
      BOLT; sp_lev trap_types[] + dat/symbols. RIDERS: BUG-1358
      intended shape in doicetrap (>=50 keeps rn2(3)) +
      HVulnerable_cold/decr_resistance producers in branch
      partial-res idiom; gb.buzzer save/zero/restore around both
      new dobuzz arms (mcastu shape; trap->once set pre-dobuzz -
      melt_ice can free the trap). REVIEWER BLOCKER fixed: hero
      landmine death window - trapeffect_landmine converts to PIT
      before losehp but blow_up_landmine's ammo clear ran after,
      so bones inherited a PIT carrying a live LAND_MINE obj;
      set_trap_ammo(trap, 0) now precedes the PIT conversion.
      CONCERNS fixed: dighole magical-trap arm + both magic-beam
      stale-launch arms -> deltrap_with_ammo(DESTROY) (BUG-1359);
      use_trap_kit worn-component delobj -> remove_worn_item
      (BUG-1360); ice-trap MR_COLD strip unhooked from in_sight
      (BUG-1361); empty-kit negative spe guard (BUG-1362);
      doicetrap monstseesu(M_SEEN_COLD) sibling parity with
      dofiretrap; dig() destroy-arm trailing newsym restored
      (master parity). MASTER BUGS ARCHIVED: BUG-1359..1362 all
      frozen-unfixed, fixed on branch. RATIFY QUEUE: (1) airborne
      hero/monster now SKIPS ice traps (ICE joined floor_trigger
      like 5.0's FIRE convention; master's mist hits flyers) -
      MAINTAINER RATIFIED 2026-07-24 (5.0 convention ships,
      consistent with branch fire traps); (2) ice-trap MR_COLD strip +
      monstseesu additions are behavior deltas vs master (both
      bug-class fixes); (3) zap.c maybe-explode + branch-native
      crafted-trap deletion sites use deltrap_with_ammo(DESTROY)
      where master had plain deltrap (guard-noise avoidance);
      (4) RogueEpyx symset got all four trap glyphs (master added
      only ice - apparent oversight); (5) dart-trap poison is
      stack-level at creation + dart to-hit unified at 8 (master
      ammo-model behavior, replaces branch per-shot 1/6 + 7).
      DEFERRED/FLAGGED: random ICE_TRAP_SET generation blocked
      until Ice Queen topology (Iniceq bias N/A; wizmode wish/
      kits/des placement work); spear-trap hide/airborne
      predicate subset (can_fly/can_levitate/is_bone_monster/
      has_bark/tortle revisit when those port); vault.c fakecorr
      + shk.c shop-repair + dbridge.c keep plain deltrap = master
      parity latents (hero-set ammo trap there = guarded
      impossible + destroy, frozen-master shape); master's Flying
      loosen-rock dig feature NOT ported (not trap-related, needs
      an owner); grow_tree/create_pit_under deltrap sites travel
      with their own slices; mktrap_victim SPEAR arm dead via
      random gen (master parity); nhlua.c 3 blank-line collapses
      are formatter-hook-enforced (rode in with nhl_deltrap
      edit). Port report:
      scratchpad/port-report-trap-types.md (session-local);
      battery list = reviewer's 8-point set in that session's
      review (landmine bones, ammo save/reload/bones, crafting
      matrix incl. BUG-1360 case, set/untrap cycle, magic beam
      incl. BUG-1359 + nesting, ice trap resist tiers + strip,
      spear/steed/ERID, display/farlook/symsets).
      Original riders (all consumed): freezing-mist partial-res
      arms (done, BUG-1358 shape); kill-attribution trap-window
      nesting (done, both arms bracketed); zombie-slice "sleeping
      gas trap wish = kit object" note (confirmed intended -
      object wins the wish parse, kit now applies).
- [x] Door materials + door-trap variety (+ des.door material=
      binding; 224 TODO-EVILHACK sites in converted levels).
      PORTED + REVIEWED + BATTERY PASSED 2026-07-24 (maintainer
      commit pending; msg at scratchpad
      commit-msg-door-materials.txt). 30 files +883/-165. Covers
      all five master door features (e9bdc34f5 iron doors,
      b24696505 metal-to-wood, 5eac42ea0 steel/stone/gold/bronze,
      6b17464a4 stone-to-flesh, 9f3e876be door-trap variety).
      CONTENT: struct rm gains Bitfield(material,5) (sizeof 8->12,
      DEV SAVES INVALID, whole-struct Sfo_rm so NO hand sfo pair
      and no sfdata regen - pilot-D recipe; master's RLE
      run-match change is N/A, 5.0 savelevl writes per tile) +
      door_material/metal_door/hard_door/door_flammable in rm.h;
      doortrap_types enum (6 varieties, never stored, separate
      namespace from trap_types so the ttyp 5-bit budget is
      untouched); coord_hash() in rnd.c; getdoortrap()
      trapminlevels 1/2/5/7/10/12 + door_trap_destroys();
      b_trapped() -> (item,bodypart,material,dx,dy) six-way
      dispatch, mb_trapped() -> (mtmp,by_hand) with the
      ge.exploder bracket (consumes the kill-attribution slice's
      deferred item) and the 5.0-native mon_learns_traps tail
      kept per surviving arm; ~40 hard/metal/flammable consumers
      (dokick/lock/dig/hack/zap/mon/music/trap/pager/detect);
      dosdoor weighted material roll + both trapped gates to
      level_difficulty()>=1; NEW Lua binding door({material=".."})
      via a door_spec payload through sel_set_door + room_door
      .material (Phase 3 row wired); wizterrainwish material
      (closes the naming-slice "iron door drops material" nit).
      RIDERS FOLDED IN: water_damage_chain gains count +
      do_container in 5.0 idiom (master's x,y are N/A - 5.0
      resolves location via get_obj_location + the gb.bhitpos
      save/restore, which is strictly better than master's
      unrestored bhitpos), 10 callers swept; BUG-1329 snuff_lit
      (obj,forced) re-ported (without it the new WATER_BUCKET
      trap re-introduces the reported lit-magic-lamp bug);
      explode.c gains master's 3 TRAPPED_DOOR arms (REVIEWER
      CATCH - without them a fatal door/tin blast read "caught
      herself in her own fireball" instead of "killed by an
      explosion", and an adjacent door blamed "your bolt" for a
      monster-triggered blast). BUG-1363 minted + FIXED (battery
      catch, branch-only port regression): not_spec_material()'s
      terrain loop compares against defsyms[].explanation, which
      master spells "dark part of a room" for S_stone but 5.0
      shortens to "stone" - so "stone door" matched the material
      word against itself, "stone" was refused as a material, and
      the preparse break ate every following adjective ("stone
      locked door" -> plain closed wood door). Affected ANY
      "stone <object>" wish, not just doors. Fix = skip terrain
      entries whose name IS the material name; a bare material
      wish is already caught by the whole-string test above it.
      REVIEW: evilhack-code-reviewer PASS, LOW only (nested
      exploder-bracket staleness = master-identical, architectural
      - noted for the kill-attribution memory; trapped tin on a
      floor can be scatter-relocated before billing =
      master-identical; pager reads live material for a remembered
      glyph = save-trust class). Coordinator ALSO removed a
      now-duplicate "%s is destroyed!" in vamprises() - its
      comment claimed the death went unreported, true of the old
      silent mondied() mb_trapped but false of every new kill
      path (monkilled(mtmp,"",how) passes a non-NULL fltxt and
      prints the line itself); battery proved single-report with
      telepathy active. MAINTAINER RULINGS 2026-07-24: (1)
      SOKOBAN prize doors = LAND AS-IS, whole mechanic in its own
      slice - this slice ships two knowingly-INERT consumer arms
      (b_trapped wall-seal, stone-to-flesh In_sokoban guard); the
      producer (sel_set_door In_sokoban auto-trap) + 7 guards
      (mfndpos, picklock x3 incl. the magic-key silent-disarm,
      doorlock WAN_LOCKING + WAN_STRIKING, untrap, openone,
      zap_over_floor) + soko*.lua material="stone" land together,
      rationale = the producer arms trapped doors against
      upstream's un-reconciled soko levels that Phase 3 replaces;
      NOT a regression, branch soko doors were already untrapped.
      LANDED 2026-08-02 with the sokoban slice (+ zap_dig as a
      ruled 8th guard; see the Phase 3 sokoban.des row).
      (2) zap_over_floor shop-door billing KEEPS 5.0's `type >= 0`
      (master uses yourzap; flipping would silently change
      exploding-wand billing too - separate pre-existing
      divergence, own owner). (3) wizard-wish materials RESTRICTED
      to wood/stone/metals (master accepts any) - those are the
      only materials the four rm.h macros give coherent behavior.
      (4) BUG-925 (explode() damage loop reads live u.ux/u.uy via
      u_at, so a steed killed mid-blast -> dismount_steed ->
      teleds can move the hero into a later cell of the SAME blast
      = double damage; master fixed with a hero_x/hero_y snapshot)
      -> PHASE 5, with raised exposure recorded: this slice adds a
      common blast producer right where a mounted hero stands.
      BATTERY (evilhack-game-tester, 3 games + focused follow-up,
      sanity_check on, paniclog ZERO slice entries): struct rm
      save/restore exact + pre-slice save REFUSED CLEANLY
      ("critical bytes mismatched at 12 (struct rm)") + sizeof 12;
      generation weighted, no random gold, material named on BOTH
      open and closed doors (BUG-1217 re-derived onto 5.0's alt_i
      loop); wishes incl. the BUG-1363 re-verify (stone/stone
      locked/stone trapped locked all material=24) with the
      "iron bars" terrain case proven un-regressed; hard-door
      matrix all rows (kick, "reinforced door shudders" with the
      trap NOT consumed, Clang vs stone dig-through, absorbs vs
      "is unharmed by", disintegration, earthquake 2/3, boulder
      locked-stops vs unlocked-slams-open); all six varieties
      hero-side + determinism re-armed 3/3 + the BUG-1274 knob
      gate (kicking a shock/knob door = no spark, HP unchanged) +
      glove gating 39/45/6 + depth-gate sweep matching the table;
      WATER_BUCKET exactly (lvl/5)+1 items, MAGIC LAMP STAYS LIT
      (BUG-1329 proof), both sampler edge paths; monster side
      (knob-by-hand burn, doorbuster takes NO knob damage,
      monster-triggered explode() catching the hero, scatter
      moving floor objects); "killed by an explosion" for both
      door and tin (svk.killer.name="explosion", KILLED_BY_AN);
      mfndpos ALL FOUR ROWS with A/B controls (same LOCKED IRON
      door pinned a hill giant 25 turns but passed a rock mole;
      flipping ONLY material 12->24 took an umber hulk from
      blocked-30-turns to through-in-3; metal SDOOR blocks the
      digger, passes the metallivore); exploder attribution
      (EXPLODE olet=28 with ge.exploder = the pet, xkilled never
      fired, hero alignment untouched); both doorlock spell cases
      via gdb (metal-to-wood material->0, stone-to-flesh 4 meats +
      correct newsym ordering); vamprises single-report.
      NOT REACHED (say so plainly): shops-always-wood row (no shop
      generated), Lua material= round-trip (no consumer until
      Phase 3), trapped tin on a shop floor, bones round-trip with
      a non-wood door. DEFERRED with owners: material_color() door
      AND object map coloring -> Phase 4 (doors stay brown; the
      farlook naming that lands here is the only material channel
      until then); ZT_ACID/ZT_POISON_GAS door arms -> zap
      door-effects slice (NOTE: acid currently destroys NO door,
      wood included - the FEATURES_ARCHIVE "still destroyed by
      acid" line is not yet true on branch, the hard_door override
      is only reached when new_doormask >= 0);
      temple_at_boundary() billing -> temple slice (every ported
      site keeps shop-only billing); SPE_CHANGE_METAL_TO_WOOD /
      SPE_STONE_TO_FLESH bhito + zapyourself + the two dispatcher
      arms -> druid/matter-spell slice (the doorlock cases and
      bhit routing here are UNREACHABLE until the spells are
      wired - REVIEWER RIDER: finish the dispatcher sweep with
      that slice or they land half-wired); coord_hash serverseed
      term -> Phase 6; master's 3 rnd(3) rain/spray
      water_damage_chain producers -> their own subsystems;
      BUG-924 scatter() uball fix -> open-air/ball-flight slice
      (precondition absent today: branch unpunish() frees only the
      chain and is_open_air does not exist, so the bare waschain
      flag is correct for now); hack.c door_flammable burn arm ->
      martial-arts/break-terrain slice (feature absent entirely).
      MASTER-PARITY NOTES (do not "fix"): postmov's
      `doormask == D_CLOSED` equality test is byte-identical on
      master, so a CLOSED+TRAPPED door always routes to the buster
      arm and the monster by-hand knob path is reachable only via
      the D_LOCKED bitmask arm; stone-to-flesh deliberately calls
      mb_trapped BEFORE clearing material so the 3x shrapnel can
      still read hard_door(). Battery lever correction: rm.h
      SDOOR=14, SCORR=15 - an earlier scan misread typ 15 as a
      secret door.
- [x] Monster `dead` .des flag -> des.monster dead= binding
      DONE 2026-07-25. Census correction: the old "13 sites" figure was
      wrong - the real inventory is **6 .des lines, all dat/mines.des
      Orcish Town (minetn-1)**: 3 shopkeepers, 1 aligned priest, 1
      watchman inside LOOP [4], 1 watch captain = 9 monsters at runtime.
      No other master .des uses the flag. Master implementation was 6
      sites (lev_comp.l token, lev_comp.y token + SP_M_V_DEAD, sp_lev.h
      field, sp_lev.c opvar parse x2, sp_lev.c kill arm).
      SHAPE: `short dead` on the transient `monster` struct (no save
      exposure - the typedef is only a create_monster() arg and an
      lspo_monster() stack local), one init in the common defaults block
      so the three non-table argument forms are covered, and
      get_table_boolean_opt in the table branch.
      MAINTAINER RULINGS 2026-07-25: (1) DEFERRED KILL - create_monster()
      now returns `struct monst *` (mirroring create_object() in the same
      file) and the kill lives at a single site in lspo_monster() AFTER
      the Lua inventory closure and spo_end_moninvent(), so `dead = 1`
      composes with `inventory = function()` and the custom loot drops
      beside the corpse. Behaviour is identical to master for all 6
      existing sites (nothing runs in between). (2) BOTH mvitals EDGES
      GUARDED - decrement only when DEADMONSTER() holds after mondied()
      and only when the count is above 0; a stated deviation from
      master's unconditional decrement.
      MASTER BUGS (both latent, no shipped .des reaches either; archived
      frozen-unfixed, fixed on branch): **BUG-1364** unconditional
      `mvitals[].died--` subtracts a count mondead() never added when the
      monster survives (lifesaved / vamprises / Ice Queen), wrapping the
      uchar to 255 - real consequences are a 255 vanquished count,
      experience()'s per-20-kills halving collapsing XP for the species,
      and corruption of killed_nazgul/Croesus/unique-killed gates; it is
      NOT extinction (that is G_EXTINCT in mvflags). **BUG-1365** `dead`
      plus an inventory block hands the block's objects to the
      already-detached monster via invent_carrying_monster, and
      dealloc_monst() frees the struct with no minvent handling, so they
      leak silently.
      REVIEWER (evilhack-code-reviewer, CONDITIONAL PASS, 0 blockers):
      2 MEDIUM, both real and fixed. M1 = the else arm of the closure
      gate left invent_carrying_monster set when `inventory` is present
      but not a function (a table, say), which still sets CUSTOM_INVENT;
      with `dead` that became the BUG-1365 leak on the branch's own new
      path. Fixed by clearing the static in the else arm when
      CUSTOM_INVENT was requested; the underlying non-function case is
      upstream's and is in the upstream-reportable table. M2 = corpse
      wording, see the CENSUS CORRECTION below. Reviewer independently
      confirmed: mtmp is initialised on every path reaching `return
      mtmp` (the three-way makemon/mk_roamer/mk_mplayer assignment is
      exhaustive and both early returns precede it); christen_monst()
      returns the same pointer; worn_slot_sanity()/check_wornmask_slots()
      are hero-slot-only so a dead monster's worn gear cannot trip them;
      no NULL-mextra deref for the makemon'd shopkeeper (isshk stays 0,
      so m_detach's shkgone is not reached) or aligned cleric (makemon
      routes an ordinary aligned priest to emin, ispriest stays 0, and
      m_detach has no EPRI branch at all); the full death lifecycle is
      safe at level-creation time (gi.in_mklev suppresses newsym,
      vision_recalc(2) has already killed cansee so every cansee-gated
      pline is silent, attribution globals are clean, and the
      single-detach monster waits for the dmonsfree that savelev() and
      moveloop both force) - with create_object's named-statue
      makemon+mongone pair as the established precedent for removing a
      monster during level creation.
      CENSUS CORRECTION (my error, reviewer-caught): the plan claimed the
      placed bodies always leave a corpse because human monsters are
      bigmonst. WRONG - EvilHack defines `bigmonst(ptr)` as
      `msize >= MZ_LARGE` while `MZ_HUMAN == MZ_MEDIUM`, so a human is
      NOT bigmonst and corpse_chance() falls through to
      `2 + ((geno & G_FREQ) < 2) + r_verysmall` = a 1-in-3 roll for all
      four species. Watchmen and watch captains are M2_MERC, so
      m_randrace can give them a GIANT body, which IS r_bigmonst and
      always leaves a corpse - so the rate is race-dependent. Identical
      on master, i.e. Orcish Town's corpses have always been
      probabilistic, whereas upstream's des.object corpse block is
      unconditional. Phase 3 must not "fix" that to match upstream.
      BATTERY (evilhack-game-tester, 1 game, paniclog file NEVER
      CREATED, zero impossible()): 7 PASS, 1 invalid lever. Proofs:
      deferred kill live-proven twice (custom crystal ball on the floor
      with the body and NO default gear when `inventory` was given
      alone; marker PLUS default gear with keep_default_invent);
      shopkeeper / aligned cleric / watch captain bodies all dropped
      their gear with no message-line output at all; the M1 regression
      lever (`inventory = { }` table + a later des.object) put the
      fortune cookie on the FLOOR, `where=1`, not on the detached
      monster. TALLY = the headline proof: `#vanquished` read "No
      creatures have been vanquished" with 5 dead watchmen plus the
      shopkeeper/cleric/captain/wererat/vampire on the map (gdb: watchman
      born=7 died=0), then read exactly "a watchman" count 1 after a
      hero kill of the live control - so the placed bodies were
      cancelled and the cancellation did not over-subtract.
      **BUG-1364's fix is live-proven** by the life-saving lever: a
      `dead = 1` watchman given an amulet of life saving as custom
      inventory was worn-and-consumed by m_dowear during
      spo_end_moninvent and survived, and died stayed 0 - master's
      unconditional decrement would have read 255 there.
      ADJUDICATED, not defects: the life-saving revival was SILENT (no
      "But wait..." / medallion messages) because level-creation message
      suppression is in effect, exactly as the reviewer traced - brief
      expectation error. The vampshifter lever was INVALID on two
      counts: "vampire lord" does not exist in EvilHack (master and
      branch both name them vampire / vampire sovereign / noble /
      royal), and the plain `vampire` record is a LVL(0,...) G_NOGEN
      placeholder without M2_SHAPESHIFTER, so pm_to_cham leaves
      cham = NON_PM and is_vampshifter is false - it died correctly and
      the 2-in-3 corpse roll failed, which is why the square was empty.
      A real lever needs vampire sovereign/noble/royal. The vamprises
      early return is the same `if (DEADMONSTER(mtmp))` guard the
      life-saving lever proved, so guard coverage is adequate.
      Files: include/sp_lev.h, src/sp_lev.c, doc/lua.adoc.
      SPUN OUT OF THE INVALID LEVER: see the "Gendered monster names
      (NAMS) lost by the monsters.h migration" row below - it turned out
      to be a branch-only regression costing 24 names and 32 shipped
      level references, fixed the same day in its own slice.
- [x] MINDEPTH: directive binding - CLOSED 2026-07-25 with NO code.
      All 17 sites are in dat/vaults.des, and the rndvault C machinery is
      already ratified NOT PORTED (rndvaults become themerms.lua
      entries, see the row below). mindiff/maxdiff filtering is pure Lua
      inside themerooms_generate (dat/themerms.lua), so there is no C
      binding to write - the per-vault MINDEPTH(depth) -> mindiff
      (difficulty) semantics check travels with the Phase 3 conversion.
- [x] **Gendered monster names (NAMS) lost by the monsters.h migration -
      RESTORED 2026-07-25.** Found while adjudicating an invalid battery
      lever on the dead= slice (`id = "vampire lord"` silently produced a
      plain vampire); maintainer identified the cause as the monsters.h
      migration not accounting for 5.0's gendered-name field.
      **MASTER HAD TWO MECHANISMS AND THE MIGRATION DROPPED BOTH:**
      (1) LOOKUP - `name_to_mon()` (src/mondata.c) rewrites the *input*
      tail `" lord"/" lady" -> "noble"`, `" king"/" queen" -> "royal"`,
      `" baron"/" baroness" -> "sovereign"`, with rat king / goblin king
      / kathryn carve-outs, so a gendered name matches mons[].mname;
      (2) DISPLAY - `x_monnam()` (src/do_name.c, "lords and ladies, kings
      and queens") runs `strsubst()` on the rendered buffer in the other
      direction, gated on `is_lord()` = M2_LORD, `is_prince()` =
      M2_PRINCE, plus an explicit `PM_VAMPIRE_SOVEREIGN` arm. So on
      master a male gnome royal DISPLAYS "gnome king" and the name also
      RESOLVES. Neither mechanism reached the branch, so gendered names
      neither displayed nor resolved - a branch-only regression, NOT a
      v1.0.0 delta. (Coordinator initially mis-diagnosed this as
      master-never-displayed-gendered-names; the strsubst block does not
      store any gendered string, so a tree-wide grep for "gnome king"
      returns zero hits on master and looks like confirmation. Maintainer
      corrected it. LESSON: a constructed string is invisible to grep -
      find the render path, do not infer absence from a literal search.)
      **FIX = 5.0's single native mechanism.** `NAMS(male, female, base)`
      fills `permonst.pmnames[NUM_MGENDERS]`, which `pmname(ptr,g)`
      (include/mondata.h) reads for DISPLAY and `name_to_monplus()`
      iterates for LOOKUP - so converting the 12 records restores both
      halves at once and supersedes both 3.6 mechanisms. **Do NOT port
      master's strsubst block or the input normalizer on top of this**;
      they would be redundant, and the normalizer's rat-king/goblin-king
      carve-outs exist only because it did blind string surgery, which
      exact per-record NAMS data cannot do.
      12 records converted, EvilHack's gender-neutral base kept in the
      third field per maintainer ruling ("if master's base name differs
      from upstream's base name, keep master's base name" - upstream's
      leader/ruler/tyrant/monarch NOT adopted): dwarf noble/royal,
      kobold noble, gnome noble/royal, ogre noble/royal, vampire
      noble/royal/sovereign, elven noble/royal. Verified every " noble"
      record carries M2_LORD, every " royal" carries M2_PRINCE, vampire
      sovereign carries neither (its explicit PM arm), and none carries
      both - so master's else-if ordering hazard (is_lord matching first
      and its strsubst finding nothing) has no instances, and the 24
      names are exactly what master rendered. Upstream has a NAMS
      counterpart for only 8; the vampire royal/sovereign tiers and the
      elven pair are EvilHack-only and took their gendered names from
      master's normalizer vocabulary.
      WAS LIVE: 32 bad monster references in 17 shipped upstream .lua
      files, 30 of them silent. `gnome lord` x15 and `gnome king` x3
      (positional form) prefix-matched to the LVL(0,..) G_NOGEN
      PLACEHOLDER gnome; `vampire lord` x8 and `vampire lady` x3 to the
      placeholder vampire - so the whole Gnomish Mines, Wizard's Tower,
      Orcus's lair and Vlad's Tower spawned level-0 pushovers.
      `Elvenking` (earth.lua) matched nothing and became a RANDOM
      monster; `montype="gnome king"` (minetn-5) and `id="Grand Master"`
      (Mon-strt) hard-errored. Wishing, genocide, polyself and
      #wizgenesis lost all 24 names too.
      ALSO IN THE SLICE: two level references corrected to the names
      master's .des uses, which is what Phase 3 will emit - earth.lua
      -> "elven royal" (master endgame.des), Mon-strt.lua -> "Master Po"
      (master Monk.des; the branch Monk leader is NAM("Master Po") and
      "Grand Master" is not a rank title either - weapon.c's "Grand
      Master" is a SKILL level). HARDENING: the three positional
      `des.monster("name")` branches in lspo_monster ignored a NON_PM
      from find_montype, leaving class -1 / id NON_PM, which
      create_monster turns into pm = NULL and therefore a RANDOM monster
      - that is what hid Elvenking inside an endgame plane. They now
      nhl_error("Unknown monster id") like the table form already does
      via get_table_montype. CONVERTER: ~/monst_to_50.py gained a
      RANK_TIERS map keyed on the base name's final word so a
      regeneration cannot silently revert this; validated against
      master's real 569-entry corpus, 12/12 emitted NAMS records match
      the hand-edited monsters.h byte-for-byte. Full-corpus proof: all
      1615 monster references across all 131 dat/*.lua now resolve
      EXACTLY - zero prefix-matches, zero errors - so the prefix
      fallback is no longer load-bearing for any shipped level and a
      future conversion typo surfaces instead of substituting.
      NUMMONS unchanged at 561; no struct/enum/save impact (pmnames[]
      already existed, the MON enum suffix is a separate macro arg,
      mons[] is static const); src/tile.c is build-generated and
      regenerated to the `male gnome royal {gnome king}` form.
      REVIEWER (evilhack-code-reviewer, CONDITIONAL PASS): independently
      re-derived the do_name.c strsubst block and the M2_LORD/M2_PRINCE
      agreement between trees, and found the slice UNDER-CLAIMED its own
      impact - see BUG-1366, the Mine Town maze abort. MEDIUM finding
      fixed: three alt_spl aliases master accepted were still missing
      ("elf king"/"elf queen"/"elf royal"); without them `"elf royal"`
      prefix-matched the LVL(0,..) G_NOGEN placeholder `NAM("elf")`.
      Reviewer also verified no name collisions among the 24 new strings
      (only the pre-existing priest/priestess and were* pairs collide),
      that the one new prefix pair "vampire baron"/"vampire baroness"
      disambiguates correctly through name_to_monplus's suffix rules,
      that every gendered name is <= its neutral name in length so there
      is no doname/BUFSZ exposure, that none of the 12 carries M2_MALE or
      M2_FEMALE so both genders stay reachable, and that no literal
      string comparison anywhere in src/ or include/ tests any of the 12
      base or 24 gendered names.
      TWO REVIEWER NOTES RECORDED, NOT FIXED:
      (a) LEVEL-GEN GENDER IS NOW DETERMINISTIC where master randomized
      it. Master's .des used the neutral names ("gnome noble"), so gender
      was rolled per monster; the adopted upstream .lua files use the
      MALE aliases ("gnome lord"), and find_montype maps a male-alias
      name to MALE unconditionally, so every Mine Town / Mine's End noble
      is male today. Upstream-consistent, and it SELF-HEALS at Phase 3
      because those levels get regenerated from master's .des, which uses
      the neutral names. Do not "fix" it by diverging the .lua from
      upstream before then.
      (b) win/share/monsters.txt tile-name comments still carry
      upstream's names (`# tile 340 (gnome leader,male)`) and the file
      has 789 entries where 561x2 are needed - pre-existing Phase 1
      migration debt, zero build impact since src/tile.c is generated
      from monsters.h at build time and is gitignored. Belongs to Phase 4.
      BATTERY (evilhack-game-tester, 1 game, paniclog file NEVER CREATED,
      zero impossible()): **7/7 PASS**. Display proven for all 12 records
      both genders (33 name inputs), with articles correct throughout -
      "A gnome king", "An elven king", "An ogre lord", "A dwarf lady".
      Renderings verified in every channel: genesis lines, farlook
      autodescribe and confirm ("G  a gnome (peaceful gnome queen)"),
      pile lines ("a statue of a gnome king", "a gnome king corpse"),
      combat/anger/flee, kill ("You kill the gnome king!", "You destroy
      the vampire lady!"), peaceful displace ("Pardon me, gnome queen."),
      vampshift reveal ("...rises as a vampire lady!"), and the
      polymorphed hero status line ("Wizard the Gnome Lord"). Resolution
      proven by gdb mnum+female walk for all 33 inputs - zero landed on a
      placeholder. All 6 previously-broken levels PASS: minend-1 real
      gnome king + gnome lords; wizard1 real vampire noble; tower1 three
      named female vampire nobles; **minetn-5 loads with NO "Unknown
      montype" error** and its statue reads corpsenm=262 gnome royal
      spe=6 = CORPSTAT_MALE|CORPSTAT_HISTORIC, confirming both the
      BUG-1366 maze abort is gone AND the statue-gender arm is live;
      earth has a real elven royal at the right tile; **Mon-strt loads
      and Master Po is the genuine quest leader** (mnum 517 == urole
      ldrnum, +6 robe, real leader dialogue on #chat). Statue wishes set
      CORPSTAT_FEMALE/MALE correctly. Player-input paths pass: genocide
      accepted "vampire baron" -> "Wiped out all vampire sovereigns",
      polyself accepted "gnome lord", and both the "gnome king corpse"
      and "vampire lady figurine" wishes carried the right corpsenm+sex.
      Death reporting: tombstone/logfile/xlogfile all read "killed by a
      gnome king's thrown dart"; **#vanquished groups by species using
      the base neuter name** ("3 gnome royals") while kill messages use
      the gendered name - the same split master has (insight.c reads
      pmnames[NEUTRAL], done_in_by uses pmname(Mgender)), so it is
      correct, not an inconsistency to chase.
      TESTING NOTES for future batteries: #wizgenesis delivers the three
      M2_SHAPESHIFTER vampire records PRE-DISGUISED (genesis says "A
      vampire bat appears"), so verify the name resolved by reading
      mtmp->cham rather than the visible form, and un-mask with
      `call (int) newcham($m, &mons[$c], 0)`; #wizgenocide does not exist
      on this branch (wish a scroll instead); the DUMPLOGFILE sysconf
      path points at a nonexistent /dgldir so death dumps are not
      reachable here - logfile/xlogfile/livelog cover the same text.
      The three new elf aliases landed AFTER this battery's binary was
      built (the tester correctly caught the tree changing under it), so
      they got their own short follow-up run against the rebuilt binary:
      PASS 6/6 - "elf lord"/"elf lady"/"elf noble" -> mnum 391 elven
      noble and "elf king"/"elf queen"/"elf royal" -> mnum 394 elven
      royal, MALE/FEMALE/random respectively, all at mlevel 8/9 so none
      on the LVL(0) placeholder; "high-elf" still resolves (regression
      check); an "elf king corpse" wish carried CORPSTAT_MALE and an
      "elf queen figurine" CORPSTAT_FEMALE; paniclog again never created.
      Recorded from that run: polyself resolves the SPECIES from the
      alias but keeps the HERO's gender ("elf queen" -> "You turn into an
      elven king!") - standard polymon() behaviour, not an alias defect,
      and identical for "gnome lord"; the female half is proven by the
      figurine wish instead.
      RESIDUE (open): find_montype still resolves by longest-prefix, so
      a TYPO that prefixes a real name (not a renamed species - those
      are all fixed) would still substitute silently. Master caught every
      bad name at BUILD time in util/lev_main.c get_monster_id() (exact
      strcmp -> ERR -> lev_comp error); the branch has no equivalent
      gate. Making level-file lookup exact would need name_to_monplus to
      report HOW it matched, so alt-table aliases like "high-elf" keep
      working - deferred, revisit with the Phase 3 conversion tooling.
- [x] **Adopted upstream .lua names that resolve to an EvilHack
      PLACEHOLDER - 80 refs FIXED 2026-07-25, plus 27 gendered-alias refs
      converted to base names.** Found by the gendered-names battery
      (anomaly 1). EvilHack splits several vanilla base species into an
      `mlevel 0` + `G_NOGEN` **race-archetype placeholder** plus a
      renamed real monster; master's .des names the real one, upstream's
      .lua name the vanilla one = our placeholder. The BUG-1366 sweep was
      blind to this because these resolve EXACTLY, just to the wrong
      record. Correct detector = "resolves to an mlevel-0 G_NOGEN
      record", not "resolves". Archived as BUG-1367.
      RENAMES (74): `gnome` -> `rock gnome` x54, `dwarf` ->
      `mountain dwarf` x20 across minefill/minend-1,2/minetn-2..7. Safe as
      a uniform substitution: master's mines.des names the bare forms
      ZERO times, and LVL()+geno are byte-identical between upstream's
      base record and EvilHack's renamed one. Reviewer re-derived the
      whole master gnome/dwarf inventory (rock gnome 56, gnomish wizard
      18, mountain dwarf 17, gnome noble 17, gnome royal 5, zero bare)
      and spot-checked minetn-2/-5/-7 and minend-1/-2 section by section,
      several matching master coordinate-for-coordinate.
      orcus.lua (5): bare `vampire` -> sovereign x3 + royal x2, derived
      positionally from master's gehennom.des orcus section; post-change
      totals match master exactly (sovereign 3 / noble 4 / royal 2).
      **Cav-goal.lua: the Caveman quest had NO NEMESIS.** `id =
      "Chromatic Dragon"` case-insensitively matched
      `NAM("chromatic dragon")`, which monsters.h itself comments is
      "strictly a placeholder for chromatic dragon scales" (mlevel 0).
      Master's Caveman.des has `MONSTER:('H',"Annam"),(23,10),asleep` at
      the IDENTICAL coordinate and flag and role.c gives Caveman
      `neminum = PM_ANNAM` (field order verified against you.h). Fixed.
      GENDERED ALIASES -> BASE NAMES (27, maintainer-directed): level
      files must name monsters by the gender-neutral base name, because
      (a) master's .des use base names exclusively in creation position -
      reviewer swept all 36 master .des files and found only three
      exceptions, all deliberate - and (b) `find_montype` PINS the sex
      for a gendered alias (exact match on pmnames[MALE] propagates
      matchgend) while a neutral-name match falls through to
      `mgend = rn2(2)`, so every Mine Town noble was male. Converted
      `gnome lord` x15, `gnome king` x3, `vampire lord` x9 (the 9th was
      in themerms.lua's `nasty_undead` list, which feeds
      `des.monster(nasty_undead[1],...)`).
      tower1.lua got `id="vampire noble", female=1` instead of a plain
      base name, because its three niches are upstream's named "Brides of
      Dracula" (Madame/Marquise/Countess) and a random gender would let a
      male be called "Madame". Reviewer verified the precedence line by
      line: `tmpmons.female` is read BEFORE get_table_montype, BOOL_RANDOM
      is -1, VAMPIRE_NOBLE carries neither M2_MALE nor M2_FEMALE, so the
      override guard is false and the explicit flag wins. Its
      `nh.is_genocided("vampire")` guard was also a REAL dead check - the
      placeholder's geno is `(G_NOGEN)` with NO `G_GENO`, i.e.
      non-genocidable, so Vgenod could never be true; now
      `is_genocided("vampire noble")`.
      DELIBERATE EXCLUSIONS, all reviewer-confirmed against master:
      Tou-goal.lua incubus x4 / succubus x2 (gender IS the discriminator
      under upstream's NAMS merge, and master's Tourist.des uses the same
      names at the same coordinates); minetn-5.lua `montype="gnome king"`
      statue (master's mines.des has NO statue lines at all - upstream-only
      content whose maleness is upstream's own fix, and it keeps the
      CORPSTAT_MALE arm exercised); valley.lua `montype="caveman"/
      "cavewoman"` (master's gehennom.des uses those exact gendered
      montype names for corpses); themerms.lua "Massacre" role list.
      **CORRECTION to my own rationale, reviewer-caught**: I justified the
      Massacre exclusion as "adventurer monsters of both sexes", which is
      WRONG for `priest`/`priestess` - lspo_object's montype scan walks
      mons[] ascending and breaks on first hit, and
      `NAMS("priest","priestess","aligned cleric")` (monsters.h:4170)
      precedes `NAMS("priest","priestess","cleric")` (:5205), so those
      resolve to PM_ALIGNED_PRIEST, not the adventurer PM_CLERIC. This is
      the documented 2026-07-03 NAMS gotcha resurfacing. The exclusion is
      still correct (byte-identical upstream, and master does the same at
      purgatory.des) but do not cite the old reasoning as fact.
      DIAGNOSTIC (maintainer-approved): lspo_monster reports via
      `impossible()` when a level names an mlevel-0 G_NOGEN record, then
      creates it anyway - deliberately NOT nhl_error, because aborting the
      load costs the player the whole level via the mkmaze fallback, which
      is the BUG-1366 symptom. Reviewer verified the predicate must stay
      NARROW: ~45 further G_NOGEN records have mlevel > 0 and are named by
      levels legitimately (Orcus, Vlad, Wizard of Yendor, Annam, Master
      Po, aligned priests, quest leaders/nemeses, Riders). It also proved
      the diagnostic cannot fire in normal play by scanning all 141 .lua
      files (1803 monster arguments, 0 placeholder hits in dat/) and by
      tracing every variable-fed des.monster argument, and audited the
      three runtime `mons[]` writers (role.c Infidel enemy1num, mon.c
      mmove, adj_erinys mlevel>=7) - none can make a placeholder look
      real or vice versa.
      REVIEWER MEDIUM FIXED: test/test_des.lua named the `gnome`
      placeholder twice explicitly (now `rock gnome`), and its exhaustive
      `for i = LOW_PM, HIGH_PM` loop necessarily names all 16 archetypes,
      so a comment now records that the resulting paniclog lines are the
      diagnostic working rather than a failure. Note for Phase 7 when the
      harness gets wired up: `impossible()` becomes a hard panic under
      `fuzzer_impossible_panic`, so if the des harness ever becomes
      fuzzer-reachable that loop needs a real skip, not a comment.
      REVIEWER RULED OUT (do not re-raise): rock gnome carries G_SGROUP
      where the placeholder did not, but group spawning is gated on
      `anymon = !ptr` in makemon, so a NAMED monster never groups - no
      density change. Side benefits of the record swap: the species become
      poly-eligible again (M2_NOPOLY dropped, matching master) and
      `G_GENO` means #genocide and mvitals extinction bookkeeping now land
      on the species actually created, where the non-genocidable
      placeholder previously swallowed them.
      BATTERY (evilhack-game-tester, 1 game, 26 level loads, 2 genocides):
      **8/8 PASS**, paniclog exactly ONE line and it is the intended
      diagnostic proof. 0 placeholders on minend-1/minetn-6/minefill/
      minetn-2 with full species censuses; **gender random again - 24M/40F
      over n=64** where every one had been male; orcus exactly 3/4/2 with
      zero bare vampire over 3 loads; **Annam present asleep with mnum ==
      urole neminum 527 and no chromatic dragon anywhere**; tower1 brides
      all female+named, and after genociding "vampire noble" the
      previously-dead Vgenod guard fired (names absent, niches
      substituted); the diagnostic produced its one line AND the level
      still loaded as its room rather than a maze; and genociding
      "rock gnome" now substitutes 9 random monsters in minend-1, where
      the non-genocidable placeholder used to absorb it.
      TESTER CORRECTION to my brief, worth keeping: "no monster has
      mlevel 0" is too strong as a test criterion - kobold, goblin and bat
      legitimately have mlevel 0. The discriminating predicate is
      `mlevel == 0 && G_NOGEN`, which is exactly what the diagnostic uses.
      SPUN OUT: the tester's raw observation that Cav-goal's named mace
      had `oartifact = 0` was a REAL second bug, not a wizloaddes
      artifact - see BUG-1368. Also recorded from that run: Cav-goal.lua
      declares `level_flags("mazelevel")` with no `noflip`, so it flips on
      both axes at random; any future coordinate assertion on that level
      must account for it.
      BUG-1368 VERIFIED LIVE in a follow-up run: `oartifact = 49 =
      gu.urole.questarti` and `otyp = 86 (rod) = artilist[49].otyp`, where
      it previously read `otyp=82 (mace) oartifact=0`. The load-bearing
      proof is the pickup text, which only fires for a real artifact - "As
      you pick up the Sceptre of Might it seem heavy at first, but then
      your arms feel strong. / You feel like you can beat up anyone now."
      - i.e. the quest-artifact touch plus the intrinsic grant. Identified
      form reads "the blessed +0 Sceptre of Might"; UNidentified it reads
      "a steel ornate mace named the Sceptre of Might", because ROD's
      appearance is "ornate mace" - expected, and the very thing that made
      the wrong base object look plausible. paniclog absent for that run.
      NOT A BUG (harness): that run reported `;` (glance) doing nothing,
      with the next keystroke moving the hero. `;` IS bound to "glance" in
      cmd.c and `/` (whatis) worked in the same run - the symptom is
      exactly the tmux trap the tester documented in an earlier round,
      where a bare `;` in `send-keys -l` is eaten as tmux's own command
      separator and must be sent as `-l '\;'`. Third recurrence; it is a
      driving gotcha, not a game defect.
      GATE (re-runnable, keep for Phase 3): 0 placeholder-creation refs
      and 0 non-exact refs across 1615 monster references in all 131
      dat/*.lua; all 14 quest-artifact placements match their artilist
      base otyp (BUG-1368's gate); `luac -p` clean on all 21 edited .lua
      files.
- [x] EvilHack terrain types: GRASS/SAND/DEADTREE/PUDDLE/SEWAGE.
      NEW ITEM MINTED 2026-07-25 - this was never on the Phase 2 list,
      but three earlier items had deferred to a "terrain slice" that
      did not exist, and the special-room-types item below could not
      start without it (GARDEN needs GRASS, FOREST needs GRASS/
      DEADTREE/PUDDLE, NURSERY floods with PUDDLE, GARDEN/FOREST
      mimics need S_deadtree). PORTED + REVIEWED + BATTERY PASSED
      2026-07-25 (maintainer commit pending), 38 files +798/-253,
      zero-warning clean rebuild.
      DECISIONS (log rows below): enum placement mirrors master's
      relative order so every range macro keeps its meaning
      (MAX_TYPE 39->44, MATCH_WALL 40->45, xFLOOR..xWATERWALL +5);
      struct rm Bitfield(flags,5)->6 for master's renumbered ICED_*;
      PUDDLE takes map char 'p' because 5.0 owns 'w' for MATCH_WALL.
      CLOSED the five parked TODO-EVILHACK markers this slice existed
      to unblock (spell.c + mcastu.c entangle-on-GRASS, telekinesis.c
      x3 dead-trees-are-solid) plus a sixth in dig.c (dead-tree
      effort).
      REVIEWER CONDITIONAL PASS, 15 findings, all coordinator-fixed:
      (1) CRITICAL - the save-invalidation decision was never armed.
      sizeof(struct rm) is 12 bytes at BOTH 5- and 6-bit flags, so
      critical_sizes[] never fires; EDITLEVEL was still 0. A pre-slice
      save would have loaded with every typ >= 14 reinterpreted (old
      ROOM as CORR, old ICE as ALTAR). Fixed by EDITLEVEL 0 -> 1.
      GENERAL LESSON: "dev saves are invalidated" is NOT automatic - a
      bitfield widening that does not change sizeof needs an explicit
      EDITLEVEL bump.
      (2) HIGH - zap.c dropped master's `range = 0; break;` after a
      death ray kills a tree, so the beam fell through to make_bounce
      and reflected back at the hero.
      (3) HIGH - 14 IS_TREE sites master had converted to IS_TREES
      were missed (may_dig, all 7 in still_chewing, mfndpos,
      m_digweapon_check, sel_set_wall_property, Druid diagonal
      squeeze, 2 message sites). Symptoms: a rock mole chewing a dead
      tree produced a CORRIDOR tile inside a room; nondiggable dead
      trees were fellable and produced impossible("mdig_tunnel: tree
      ... is undiggable").
      (4) HIGH - farlook said "a grass"/"a shallow water"; master
      suppresses the article in TWO places (just_an + the
      do_screen_description article list), neither ported.
      (5) MEDIUM - dead trees had no kick handler at all ("You kick at
      empty space", fatal kick logged as "killed by kicking a rock");
      ported master's block + TREE_FLOCK into 5.0 idiom.
      (6) MEDIUM - make_corpse's new DEADTREE arm never updated the
      vision array, so the hero saw through a fresh dead tree until
      the next vision_reset; added recalc_block_point (master shares
      this gap - branch improves on master).
      COORDINATOR REVERTED TWO OF THE PORT'S OWN CHANGES: gush() left
      a PUDDLE instead of a POOL, and mkswamp() converted half its
      checkerboard to PUDDLE. Master's shallow-water SEMANTICS live
      behind is_damp_terrain/is_puddle/is_sewage (~110 sites, 30
      files) which are NOT ported, so both changes would have quietly
      REMOVED an existing drowning hazard. Both restored to POOL with
      TODO-EVILHACK markers naming the follow-on slice. Terrain stays
      reachable via wizard wish + Lua, so the room-types slice is not
      blocked.
      HTML dump: the five htmlsym[] entries were REMOVED again. The
      branch's table is minimal and has no S_room/S_tree/S_pool rows,
      so mapping dead tree to U+03A8 while live tree stays '#' would
      make the dump show distinctions the terminal does not. Whole
      table rides the Phase 4 DUMPHTML reconciliation.
      BATTERY 13/13 PASS (game-tester agent, 2 games, paniclog file
      never created): all five wishes name and render correctly with
      the expected colors; GRASS/SAND track a plain ROOM square
      through lit / out-of-LOS / unlit-remembered (the display.c risk
      - verified by capture-pane escape codes, all three identical
      dark-gray in states 2 and 3); farlook and `:` and the status
      terrain field carry no article; engraving refused on grass and
      sand while graves still work on both; ICED_* round trip exact
      for puddle AND sewage (typ 42/43 -> ICE -> back, objects not
      buried) AND the staged north-facing raised drawbridge DID bury
      its 7 rocks, confirming the branch's `shallow` boolean fixes
      master's drawbridgemask/icedpool aliasing bug in the live game;
      nondiggable dead tree resists axe and fire, plain one falls to
      an axe and a pick-axe correctly reports DIGTYP_TREE; all four
      kick outcomes seen and a fatal kick tombstones as "killed by
      kicking a dead tree"; a pick-axe dwarf walled in by nondiggable
      dead trees tunnelled the room wall instead and produced NO
      mdig_tunnel impossible; rock mole chewed a dead tree to ROOM
      (26) not CORR (25); death ray withered a tree and did NOT
      bounce; an ent killed at range cut line of sight in the same
      frame; a des.terrain level using all five new map chars loaded
      clean via #wizloaddes; overview said "some grass"/"some sand";
      save+reload byte-identical including all three ICED_* states;
      four generated swamp levels produced POOL only and zero
      PUDDLE/SEWAGE, confirming the deliberate producer deferral.
      BATTERY ANOMALY CLOSED by coordinator: an unlit SAND square
      blanked to S_stone after unmap_object. Not a defect - master's
      unmap_object blanks ROOM, SAND and GRASS identically on
      !waslit, 5.0's original had only the ROOM arm, and the port
      added the other two in master's exact shape. A plain ROOM
      control would blank the same way.
      DEFERRED (owner rows): tile art for the 5 new cmap symbols ->
      the NEW Phase 4 "TILE-ART DRIFT" item minted by this slice
      (measured there: tile.c expects 258 cmap tiles, other.txt
      supplies 243, 23 indices fall past the end of the file, first
      break moved from index 37 to 19); SAND/ICE random level
      generation -> Ice Queen dungeon topology (Phase 3), same block
      as the existing random-ICE deferral; concealed_spot +
      maybe_freeze_underfoot are master-only functions with no branch
      counterpart (mk_knox_fallback_vault was in this list until the
      knox slice ported it, 2026-08-01; makeriver/mkrivers until the
      mines slice ported them, 2026-08-02 - see the mines.des row and
      the Decisions log); SCR_CONSECRATION altar-on-sand/grass (object
      exists, read.c case does not). dat/symbols AmigaFont
      deliberately skipped - that set also has no
      S_forge/S_magic_chest/S_lavawall rows, so skipping matches both
      the branch's own precedent and upstream's.
      MASTER BUGS FOUND (traced, not fixed - master frozen):
      zap.c ZT_COLD skips bury_objs on a north-facing raised
      drawbridge (drawbridgemask and icedpool alias the same flags
      byte; DB_ICE == 8 == ICED_SEWAGE) - branch sidesteps it with a
      `shallow` boolean captured before typ is overwritten, and the
      battery proved the branch buries correctly there;
      objnam.c puddle/sewage wizard wish wets only the top floor item
      (calls water_damage() with the chain head instead of
      water_damage_chain(), unlike the adjacent pool/moat arm) -
      branch uses water_damage_chain.
      PHASE 3 CARRY: ~/des_to_lua.pl emits master's 'w' for puddles
      and MUST be remapped to 'p' before the level conversions run.
      (CARRY DISCHARGED 2026-08-01 - the converter GATE row in the
      Phase 3 checklist is ticked with the full evidence.)
- [ ] Damp-terrain behavior sweep (NEW, minted 2026-07-25 by the
      terrain slice): thread master's is_damp_terrain/is_puddle/
      is_sewage through their ~110 call sites across ~30 files
      (wading, water damage, drowning, monster movement, item damage,
      polyself, teleport, dokick, read, sit, ball, uhitm, weapon).
      Until this lands PUDDLE and SEWAGE are inert floor, and the two
      producers held back by the terrain slice (fountain.c gush,
      mkroom.c mkswamp checkerboard) stay reverted to POOL - restore
      both WITH this sweep, not before. Also closes the sp_lev.c
      is_ok_location(WET) TODO and the two Druid TODOs the terrain
      slice could not close (weapon.c dmgval has no attacker param on
      5.0 -> blocked on attacker-param threading; allmain.c regen is
      a per-turn probability on 5.0, not master's `moves % rate`
      interval, so it needs a new formula rather than a port).
- [x] Special room types only: GARDEN=17 etc. -> mkroom.h +
      sp_lev room-name table (rtype drives runtime behavior, so
      needed regardless of level source). The rndvault C
      machinery (struct _rndvault registry: fname/freq/mindepth,
      ~50 sites mklev.c/sp_lev.c/decl.*) is NOT PORTED -
      MAINTAINER RATIFIED 2026-07-18: unique-to-EvilHack
      rndvaults merge into upstream's themerooms system as
      themerms.lua entries (fname -> contents, freq -> frequency,
      mindepth -> mindiff); overlapping rooms dedupe to
      upstream's version per the collision rule. Interlocks:
      frequency-TUNING pass so EvilHack vault appearance rates
      stay roughly master-like in the merged weighted pool
      (competes with upstream entries + the weight-1000 default;
      else ratify the new cadence as a v1.0.0 delta); check
      MINDEPTH (depth) vs mindiff (difficulty) semantics per
      vault during conversion; W-gate for vaults = repeated
      level-generation sampling, not per-file walks.
      PORTED + REVIEWED + BATTERY PASSED 2026-07-25 (maintainer
      commit pending), 11 files +568/-48, zero-warning clean
      rebuild. SIX types not master's seven: RNDVAULT DROPPED
      because 5.0's THEMEROOM occupies its exact role (mklev.c and
      sp_lev.c both carry the same "treat like OROOM so random
      content still lands" test master uses RNDVAULT for), which
      follows from the ratified rndvaults-to-themerooms decision
      above. So SHOPBASE 14 -> 20 and CANDLESHOP 31, NOT master's
      21/32. SIDE EFFECT WORTH KEEPING: mapseen_feat.shoptype is
      Bitfield(5) and recalc_mapseen stores a raw rtype in it, so
      master's CANDLESHOP 32 truncates to 0 = the "mixed shop
      types" sentinel and a known candle shop annotates as
      "untended shop" in the overview - master has this bug TODAY
      and dropping RNDVAULT incidentally fixes it here. A comment
      at the enum records the <= 31 constraint; do NOT "restore
      parity" by re-adding RNDVAULT. (No static assert - this tree
      has no nhStaticAssert facility, and adding one is its own
      portability decision.)
      EDITLEVEL 1 -> 2. sizeof(struct levelflags) MEASURED at 24
      both before and after the three new bits (27 -> 30 stays in
      one 4-byte unit), so critical_sizes[] would not have fired -
      [[pattern-bitfield-widening-needs-editlevel]] recurring one
      slice after it was written.
      ARCHITECTURAL ADAPTATION: master's do_mkroom(GARDEN) calls
      mkgarden(NULL), which SELECTS AND FILLS in one call. 5.0
      splits generation - do_mkroom() only marks a room and
      fill_special_room() fills at the end of makelevel(). A
      literal port would have filled every garden TWICE (double
      trees, fountains, nymphs). Split into mkgardenroom() for
      master's selection (random room, OROOM, LIT, no stairs, 25
      tries) and mkgarden()/mkforest() for fill only; master's
      rlit requirement is preserved, which routing through
      mkzoo()/pick_room() would have lost. Reviewer verified no
      double-fill path exists (the second fill_special_room loop
      is in lspo_finalize_level, testing-only, called by no
      dat/*.lua; a themeroom arriving filled=1 is handled because
      rtype != OROOM makes ROOM_IS_FILLABLE false).
      MASTER PARITY RIDERS, all inside functions this slice
      rewrites: ANTHOLE now gets a QUEEN ANT at its geometric
      centre (master's center-tile group includes ANTHOLE; 5.0
      lacked it, and PM_QUEEN_ANT is G_GENO|G_NOGEN so this is its
      only generation site in either tree); all three master
      max(...,1) rn1/rnd guards taken together rather than one of
      three (defensive parity - none is reachable, level_difficulty
      bottoms out at depth() >= 1); COURT throne loop, COURT chest
      and the BEEHIVE-group center tile all stopped ignoring the
      picker's return value (the throne loop was reading an
      uninitialized coord and feeding it to occupied() and
      levl[][].typ = THRONE - a latent 5.0 bug, fixed with master's
      control flow but 5.0's somexyspace picker, which is strictly
      better since it also rejects IS_FURNITURE so the chest can no
      longer land on the throne tile).
      DELIBERATE SUPERSET: fill_special_room() lists ARMORY, which
      master does NOT - master's omission leaves a level-file
      armory unstocked, and under 5.0's deferred-fill model the
      case is mandatory or every armory would be empty. Commented
      in place.
      COCKNEST DEPTH RESTORED 16 -> 15 (see the unlogged-constant
      sweep item below - this is the find that minted it).
      NEOTHELID GRUDGE ARMS ported to mm_aggression (mind flayers
      attack neothelids; neothelids attack anything not mindless).
      CAUGHT BY THE BATTERY, not by the port or the review: the
      rare nursery variant's whole point is the infighting, master's
      own comment says "(see grudges)", and without the arms the
      neothelid just sits there. NOTE for future slices:
      mm_aggression is being ported PIECEMEAL per subsystem - the
      branch had 3 arms against master's 14, the present ones
      (Kathryn, Lucifer) having arrived with the Ice Queen and
      demon-lord slices. Expect to carry the relevant arms with
      every future monster-behavior slice; an incomplete grudge
      table is the normal state and will not look wrong.
      BATTERY 12/12 PASS (game-tester agent, 8 games; paniclog's
      only 2 entries were self-inflicted by the tester's own first
      Lua level lacking des.random_corridors()). Highlights: all
      six types generate and fill correctly (gdb tile + monster
      dumps, not eyeballing); entry messages exact incl. the blind
      garden/forest variants and the lemure pit's deliberate
      SILENCE (room still marked discovered); all three ambient
      sound sets fire incl. hallucinatory variants; level flags set
      and clear; garden/forest feature counts land in the 4-6 band
      a single rn1(3,3) pass produces, confirming NO double-fill;
      nursery stair tiles not converted to puddle and the 1-in-10
      neothelid variant places exactly one; natural Mine Town
      candle shop reads "lighting shop" not "untended shop",
      confirming the renumbering held; save/reload exact.
      HONEST CAVEATS from the battery: NURSERY and COCKNEST never
      appeared across ~125 randomly generated levels - they are
      last in the else-if chain at roughly 1-2%/level, so absence
      is expected rather than a defect, and their generation is
      covered via SHOPTYPE=h; and the pre-slice save rejection
      cannot be attributed uniquely to EDITLEVEL because the
      staged save predates several other branch commits.
      DEFERRED (owner rows): getroomtype() NOT ported - master's
      only consumer is its mapglyph.c per-room-type map coloring,
      which is Phase 4 on this tree, and porting an unused function
      would be dead code; ZOO "you enter an ice cave!" arm marked
      TODO-EVILHACK pending Ice Queen dungeon topology.
      PRE-EXISTING 5.0 QUIRK observed, not ours: a level created by
      #wizloaddes or regenerated by #wizmakemap has its overview
      entry discarded (makemap_prepost -> rm_mapseen) and never
      recreated, so ^o lists nothing for it even after entering a
      shop there.
- [x] #shield, other added commands (sweep cmd.c delta).
      PORTED + REVIEWED + BATTERY PASSED 2026-07-25 (maintainer
      commit pending), 9 files +629/-22, zero-warning build.
      THE SWEEP IS AUTHORITATIVE AND DONE, by two diffs: (a)
      extcmdlist name diff master vs branch = 12 names master has
      and the branch lacks; (b) cmd.c function-set diff master vs
      vanilla merge-base 44b6612fd = 13 functions master added,
      which confirms (a) and turned up nothing extra. Every other
      EvilHack cmd.c change - domonability's tortle/drow/aasimar/
      vampire arms, the #terrain forge + magic-chest rows, the
      enlightenment artifact blocks - was ALREADY on the branch,
      each having landed with its own slice.
      DISPOSITION OF THE 12: seenv/seespells/seetrap already
      present under upstream's names (wizseenv/showspells/
      showtrap); wizspell needs NO WORK, 5.0's #wizcast
      (dowizcast, src/spell.c) is body-identical to master's
      wiz_spell; showcolors stays with the Phase 4 customcolor
      item; wizpathfind + wizescapepath BLOCKED, see the new
      pathfinding row below; the remaining five ported here.
      SHIPPED: #shield (do_wear.c is_wearable_shield/
      free_shieldhand/find_altshield/designate_altshield/doshield
      + wield.c dotwoweapon shed block + objnam.c suffix +
      bones.c), #steal (uhitm.c dosteal/find_steal_target + the
      whole thievery rework inside steal_it), #removeimarkers,
      #wizcrown, #wizpious.
      THE REAL WEIGHT WAS #steal, NOT #shield. Master's steal_it()
      is 339 lines against 5.0's 105 because EvilHack's entire
      pickpocketing system lives inside it, and none of it had
      been ported: P_THIEVERY was a DEAD SKILL on the branch -
      u_init.c granted it to Rogue (Skilled) and Convict (Master),
      weapon.c named it and seeded it at Basic, and nothing
      trained or read it. Another instance of
      content-live-behavior-missing, as was obj->altshield, which
      Phase 1 had already put in obj.h with zero readers. The
      branch's own setmangry() carried a comment promising "an
      alignment penalty for failing to steal from a priest in
      steal_it()" - a dangling reference this slice closes.
      PORTED INTO 5.0 IDIOM, NOT TRANSPLANTED: really_steal() is
      NOT ported (5.0 already inlines exactly that code, and
      master's post-really_steal re-checks are dead code there);
      qst_guardians_respond() was de-static'ed in mon.c and reused
      instead of duplicating master's inline quest-guardian block,
      which 5.0 had already extracted verbatim; the greased/
      oilskin slip test became steal_slips_off() since master has
      it twice; set_twoweap() replaces master's raw u.twoweap
      write; has_magic_key()/is_magic_key() are 5.0's names for
      master's roguish-key pair; pmname() replaces the removed
      ->mname; Concat() replaces master's BP_HAS_ROOM + Strcat.
      TWO CONTROL-FLOW MOVES THAT ARE LOAD-BEARING, not cosmetic:
      (1) 5.0's `if (!Upolyd) break;` had to move from the TOP of
      the theft loop to master's position near the bottom - at the
      top it is correct only while steal_it() is reachable solely
      from mhitm_ad_sedu(), and it would have made #steal (never
      polymorphed) steal nothing at all. (2) the port adopted
      master's `while (ustealo ? ... : (otmp != 0))` ternary, so
      5.0's unconditional `continue` on a dropped object had to
      gain an `if (!ustealo) break;` - otherwise the loop
      re-enters with the SAME otmp when hold_another_object()
      returns a non-NULL floor object and extract_from_minvent()
      is called on an OBJ_FLOOR object. 5.0's continue was safe
      only because its loop condition reassigned otmp.
      COORDINATOR FIX to the port's own work: it had put
      `gold = findgold(...)` inside the `as_mon && mdef->mcanmove`
      block. Master gates gold protection on as_mon ALONE (the
      mcanmove conjunct is 5.0's, and only guards the strip-bare
      path), so a seduction against an incapacitated target would
      have let the hero take coins master protects. Gold is now
      spared whenever the theft is a seduction, and an ordinary
      pickpocket can still take coins - that asymmetry is the
      point of the as_mon gate and the battery proved both halves.
      MAINTAINER RULINGS 2026-07-25, both fixing master defects
      the review surfaced rather than shipping parity:
      (1) #shield and the X shed block called remove_worn_item(),
      the FORCED-removal helper, which has no curse check - so a
      cursed shield could be taken off freely, unlike every other
      armor slot. All 45 remove_worn_item() call sites were swept:
      every other one is theft/trap/polymorph/impossible-fallback,
      these two were the only hero-voluntary uses. Both now gate
      on cursed() - the same helper armoroff() uses. BUG-1375.
      (2) the X shed guard admitted any non-bimanual WEAPON_CLASS
      secondary, but can_twoweapon() is stricter (TWOWEAPOK also
      rejects launchers/ammo/missiles, plus opposite-aligned
      artifacts and two Wand of Orcus rules), so shield + long
      sword + readied darts meant the shield came off and
      two-weaponing was declined anyway - and the code comment
      claimed the opposite. Guard now repeats every can_twoweapon()
      refusal via the exported twoweapon_ok(); comment corrected.
      BUG-1376.
      MASTER BUGS ARCHIVED (frozen-unfixed, fixed on branch):
      BUG-1374 steal_it() leaks the target's gold - the pre-loop
      greased/oilskin check returns after obj_extract_self(gold)
      with no mpickobj(), so a nymph-form hero stealing a slippery
      item from a gold-carrying monster orphans the coins;
      BUG-1375 and BUG-1376 as above; BUG-1377 the #shield
      designation survived into bones for any marked shield NOT in
      inventory at death, because master patched only
      drop_upon_death() and not resetobjs()'s saving branch where
      every other per-hero flag is stripped - a later hero picking
      the shield up got a designation they never made; BUG-1378
      the watch shouted "Halt! You're under arrest!" once per
      visible watchman, since master's loop lacks the mpeaceful
      test that angry_guards() makes true after the first
      iteration (branch mirrors 5.0's peacefuls_respond(), which
      also supplied the SetVoice call and the !msleeping filter).
      MASTER-PARITY, DO NOT "FIX": setmangry()'s Rogue/Convict
      forcefight carve-outs are vestigial in BOTH trees (#steal
      replaced the forcefight thievery trigger) but still fire as
      live behavior, and the branch already matched; "%s has
      nothing for you to steal" costing a turn; dosteal() having
      no Hidinshell guard while doshield() does; designate_
      altshield()'s single-mark invariant holding only over
      gi.invent and not into carried containers; `{` on a
      COMPLETELY empty inventory printing nothing (query_objlist
      returns 0 not -1 before the SIGNAL_NOMENU path - the
      battery showed the message DOES fire whenever inventory is
      non-empty, so the gap is narrower than expected).
      dat/cmdhelp NEEDS NO EDIT and this was verified, not
      assumed: 5.0's dowhatdoes_core() resolves keys through
      key2extcmddesc() -> cmdbind_get() -> extcmdlist and the
      whole file-reading path is #if 0'd, so `{` and #steal
      self-document. C('u') for #removeimarkers is DEAD in BOTH
      trees and was kept for parity - reset_commands() rebinds it
      to rush-northeast on the branch, and master's rhack()
      intercepts it identically via movecmd(unctrl(*cmd)) before
      the command table is consulted; under number_pad both trees
      fall through to the table. The command is reachable by name.
      BATTERY 28/28 PASS (game-tester agent, 3 games; paniclog's
      45 entries were ALL the pre-existing rndmonst A_NONE bug
      below, fired by Oracle-level generation, none from the five
      commands). Headline proofs: thievery advance climbed
      20 -> 89 and the skill went Basic -> Skilled, the first time
      P_THIEVERY has ever moved on this branch; the gold
      asymmetry proved from both sides by gdb minvent dump (a
      pickpocket took the coins, a wood nymph's seduction stripped
      22 of 23 items and left exactly the gold piece); exactly ONE
      "Halt!" for five visible Minetown watch; quest-leader theft
      angering three thugs; coaligned-priest failure recording
      "Turn 141: caught pickpocketing (-5)" in #conduct; the
      Oracle refusing 20 controlled attempts with 0 successes; the
      cursed shield staying on with bknown set; darts as secondary
      leaving the shield worn; save/reload preserving both the
      designation (same o_id) and mid-skill progress.
      COSMETIC MASTER BUG FIXED ON MAINTAINER'S CALL 2026-07-26,
      caught by the battery: the slip message read "the jackal's
      slippery slippery cloak" because master's format supplies
      the adjective while an unidentified oilskin cloak's own
      appearance string is already "slippery cloak"
      (objects.h CLOAK("oilskin cloak", "slippery cloak", ..)).
      steal_slips_off() now resolves the object name first and
      drops the adjective when the name already starts with it, so
      the adjective still appears for an IDENTIFIED oilskin cloak
      ("slippery oilskin cloak") and a greased unidentified one
      still reads "greased slippery cloak", which is correct.
      Safe to hold the name across s_suffix() because that uses
      its own static buffer in hacklib.c, not objnam.c's rotating
      NUMOBUF pool. BUG-1379.
      COSMETIC, MASTER-PARITY, LEFT AS-IS: a dropped designated
      shield keeps the tag in its drop message, because doname()
      runs while carried() is still true.
- [ ] Monster BFS pathfinding: intelligent + escape distance maps
      (NEW, minted 2026-07-25 by the added-commands slice). NOT a
      checklist item before now, and the two wizard commands that
      view it CANNOT ship without it - #wizpathfind and
      #wizescapepath ride with this row and are the ONLY part of
      the cmd.c delta left unported.
      SCOPE: decl.h globals pathfind_dist[COLNO][ROWNO],
      pathfind_turn, pathfind_px/py, escape_pathfind_dist,
      escape_pathfind_turn, escape_pathfind_tx/ty; hack.h
      PATHFIND_UNREACHABLE; compute_pathfind_map/
      ensure_pathfind_map/mon_uses_pathfinding/
      compute_escape_pathfind_map/ensure_escape_pathfind_map;
      ~46 refs in mon.c and 17 in monmove.c, plus map-
      invalidation hooks in do.c, hack.c, mkobj.c and zap.c.
      All of it must land in 5.0 idiom, so the globals are a gX
      vs svX placement decision (log it) and the caches are
      turn-stamped rather than saved.
- [ ] findgold() gold-material rule (NEW, minted 2026-07-25 by the
      added-commands slice). Master's findgold(chain, only_coins)
      makes gold-MATERIAL objects count as gold; the branch has
      the 1-arg coins-only form, which equals master's
      only_coins == TRUE. 14 call sites across detect.c,
      makemon.c, mhitm.c, monmove.c, shk.c, steal.c and uhitm.c,
      plus master's AD_SGLD "stole a gold non-coin object" arm,
      absent from mhitm_ad_sgld(). NOT a blocker for #steal, which
      is why that slice shipped without it: master's steal_it()
      only ever passes TRUE.
- [x] rndmonst A_NONE weight underflow (NEW, minted 2026-07-26 by
      the added-commands BATTERY, which is the only reason it was
      seen; PRE-EXISTING, not that slice's doing). Symptom: 45
      paniclog lines and 39 "Program in disorder!" --More--
      presses from ONE Oracle-level generation, cycling
      `impossible("bad weight in rndmonst for mndx %d")` for mndx
      423/435/497 (amorous demon, weredemon, infidel).
      ROOT CAUSE: A_NONE is -128 (align.h), and align_shift()
      computes (maligntyp + 20) / (2 * ALIGNWEIGHT) on a lawful
      level and (20 - abs(maligntyp)) / ALIGNWEIGHT on a neutral
      one, so an A_NONE monster contributes -13 or -27 against a
      G_FREQ of 1..3 and rndmonst_adj()'s `weight < 0` guard
      fires. The code is upstream 5.0 VERBATIM (identical in
      Official_nh50) - it is the DATA that differs: the branch has
      37 monster records declared A_NONE against upstream's 1,
      so upstream never trips a guard EvilHack's monster set trips
      constantly. A Phase 1 data-meets-upstream-code collision.
      FIXED + BATTERY PASSED 2026-07-26, LANDED 54f3f27e6
      ("Fix: restore the A_NONE guard in align_shift()"),
      src/makemon.c only, 11 insertions.
      THE FRAMING ABOVE WAS HALF WRONG AND THE CORRECTION IS THE
      USEFUL PART: this was not a judgement call between three
      options, it was an UNPORTED MASTER FIX. Master's
      align_shift() has always opened with
      `aligntyp ma = (ptr->maligntyp == A_NONE) ? -3 :
      ptr->maligntyp;` and used `ma` in all three arms; the Phase 0
      tree adoption took upstream's version, which reads
      ptr->maligntyp directly, and the guard was lost. So the fix
      is master parity, not invention. Shifts go from -13/-27/+18
      to 2/4/2 (lawful/neutral/chaotic).
      SECOND SYMPTOM, silent, which the paniclog could never have
      revealed: on a CHAOTIC level the same arithmetic yields
      +18, so those species were generated at weight ~19 against
      every other monster's 1-5. No message, just over-generation.
      Only gdb on align_shift() proves that half.
      LIVE SET IS THREE SPECIES, not 37: of the 37 A_NONE records,
      23 are G_NOGEN/G_UNIQ and filtered by uncommon(); of the
      remaining 14, twelve are G_HELL, and uncommon() filters those
      outside Gehennom while Gehennom itself is unaligned - so they
      never see a non-zero shift either. The live set is exactly
      amorous demon, weredemon and infidel, matching the three mndx
      values the battery reported. That is also why master's -3
      remains the right value despite master's comment ("the only
      randomly generated A_NONE monster is the infidel")
      undercounting the records - the branch comment states the
      accurate version.
      SWEEP DONE: every other maligntyp consumer on the branch
      (artifact.c, minion.c, dothrow.c, mplayer.c, priest.c,
      pray.c, mon.c, spell.c, trap.c, wizard.c, restore.c and the
      rest of makemon.c) uses sgn(), an ==/!= comparison or a plain
      assignment. align_shift() held the only arithmetic that can
      underflow. sgn(A_NONE) reading as chaotic is master-parity
      and is not part of this bug.
      BATTERY 7/7 PASS (game-tester agent, 2 games; paniclog file
      NEVER CREATED, against 45 lines from a single Oracle
      generation pre-fix). Proofs: 3000 rndmonst_adj passes on the
      neutral Oracle level with zero paniclog lines; gdb
      align_shift(&mons[PM_INFIDEL]) = 4 neutral / 2 chaotic
      (medusa) / 2 lawful (forced field, restored and read back);
      ordinary species unchanged and still ordered correctly
      (Archon 4 lawful vs 0 chaotic, hill orc 2 lawful vs 3
      chaotic); Gehennom a no-op at 0; and all three live species
      selectable again at weight 5 where pre-fix they clamped to 0
      and could never be randomly generated at all.
      BATTERY ALSO FOUND THE BUG THAT WAS MASKING THIS ONE - see
      the dungeon-alignment row below. Fixing them in this order
      was lucky rather than clever: had the alignment bug been
      fixed first, the A_NONE spam would have widened from two
      special levels to whole dungeons.
- [x] Dungeon alignments never reach runtime (minted 2026-07-26 by
      the A_NONE battery, which surveyed svd.dungeons[] to check
      its own premise and found every dungeon reporting align 0).
      UPSTREAM 5.0 BUG, live on the branch, NOT branch-introduced -
      see the upstream-reportable table. DONE 2026-07-26,
      BUG-1381 + BUG-1382 + BUG-1383.
      SCOPED AS A ONE-LINE FIX; IT WAS THREE DEFECTS. The two extra
      ones were not in the original trace, and the note above that
      "the LEVEL path in the same file gets it right" was wrong -
      it only looked right because it reads a differently-broken
      field. Recorded here because the mis-scoping is the lesson:
      1. BUG-1381, the traced one. get_dgn_align() returns
         D_ALIGN_* (`AM_* << 4`) and dungeon.c assigned one
         straight into `Bitfield(align, 3)`, so 64/32/16 all
         truncated to 0 and every dungeon-wide alignment in
         dat/dungeon.lua was discarded.
      2. BUG-1382. The level-inherits-dungeon fallback reads
         `pd->tmpdungeon[dgn].flags & D_ALIGN_MASK`, but 5.0's Lua
         parser stores dungeon alignment in a dedicated `.align`
         member and puts NO alignment bit in `.flags`. That member
         was write-only dead code; special levels never inherited
         their dungeon's alignment. Independent of 1381 - fixing
         the truncation alone would have left minetn/minend/tower
         unaligned.
      3. BUG-1383. `UNCONNECTED` is 0x10 and `D_ALIGN_CHAOTIC` is
         `AM_CHAOTIC << 4` = 0x10 - the same bit, with D_ALIGN_MASK
         0x70 overlapping the flag word. The Tutorial's dungeon-
         scope `flags = { "mazelike", "unconnected" }` therefore
         read back as chaotic through defect 2. That is what the
         survey line below was actually seeing.
      FIX: alignment decoupled from the flag word entirely -
      `struct tmplevel` gained an `align` member mirroring
      tmpdungeon's, both init_level() reads take the align fields,
      and the dungeon assignment shifts. Master's `<< 9` was
      REJECTED: master's word runs TOWN 0x01..HDGN 0x100 with
      D_ALIGN_CHAOTIC at 0x200 on the next free bit, so once the
      five unported EvilHack flags land alongside 5.0's
      UNCONNECTED (ten flags, needing bit 9) `<< 9` re-creates the
      collision. Decoupling leaves 0x20-0x200 free for them.
      CORRECTED SURVEY (the pre-fix line said "Sokoban and The
      Quest neutral" and "tut-1/tut-2 chaotic"; both wrong):
      dat/dungeon.lua declares Gnomish Mines lawful, Sokoban
      neutral, Vlad's Tower chaotic, oracle neutral, medusa
      chaotic. The Quest block declares NO alignment key - reading
      0 is correct, not a symptom. tut-1/tut-2 declare none either;
      their chaotic reading was defect 3.
      POST-FIX gdb survey at init_castle_tune(): Mines 4, Sokoban
      2, Vlad 1, Quest/DoD/Gehennom/Ludios/Planes/Tutorial 0;
      minetn/minend 4, tower1-3 1, soko1-4 2 (inheritance live);
      tut-1/tut-2 0 while Tutorial keeps unconnected 1 - a pairing
      that was impossible pre-fix.
      A_NONE battery re-run afterwards as required, since this
      widens exposure from a few special levels to whole dungeons:
      PASS, ~860 forced rndmonst() calls across the lawful, neutral
      and chaotic arms plus 13 level generations, paniclog never
      created, and mvitals[].born proving all three A_NONE species
      are generated again rather than merely silent.
      TWO BATTERY-STAGING GOTCHAS worth reusing (both cost time on
      this slice):
      1. align_shift() caches `lev = Is_special(&u.uz)` keyed on
         svm.moves, and wizard ^G/^V consume no game time - so a
         run staged entirely at a fixed turn count reads a STALE
         lev from a previously-visited level and reports the wrong
         alignment arm. Burn a turn on each level before observing.
         Staging artifact only; the reviewer traced the cache and
         found no in-game staleness window (svm.moves++ precedes
         rhack()/deferred_goto(), and goto_level() sets u.uz before
         mklev()).
      2. /root/.evilhackrc is full of master-era options this 5.0
         build rejects, which stops startup at "Hit return to
         continue" and hangs an unattended run. Launch with
         `--no-evilhackrc`. Also `-@` does NOT suppress character
         selection in this build - it still prompts, answer 'y'.
- [x] LEV_LIMIT must be raised before the EvilHack dungeons land
      (NEW, minted 2026-07-26 by the code reviewer on the dungeon-
      alignment slice). include/dgn_file.h on the branch carries
      upstream 5.0's `LEV_LIMIT 50`; master raised it to 99 for
      EvilHack's extra dungeons. dat/dungeon.lua currently declares
      ~37 level prototypes (38 entries in the runtime sp_levchn
      survey), so headroom is only ~12 - and the five unported
      EvilHack dungeons (Ice Queen, Vecna, Goblin King, Purgatory,
      Hidden Dungeon) plus their levels will blow through it.
      Raise to master's 99 with the dungeon-content port, NOT
      before - it is a pure constant bump with no dependency, but
      it belongs with the content that needs it.
      RELATED, same file: `init_dungeon_levels()` writes
      `tmpl = &pd->tmplevel[pd->n_levs + f]` BEFORE the
      `if (pd->n_levs > LEV_LIMIT) panic(...)` guard that follows
      the loop, so overflow corrupts proto_dungeon before it is
      caught. Pre-existing and upstream-identical, not player-
      reachable (dungeon.lua is maintainer-controlled), but it
      means the failure mode when the limit IS hit is a silent
      stack smash rather than the intended panic. Worth fixing in
      the same slice as the bump.
      DONE 2026-08-01 as the Phase 3 gate slice, three commits.
      SESSION RULING reversing the mint-time timing note: the bump
      landed NOW (save-neutral - proto_dungeon is a stack local of
      init_dungeons() and the dungeon save format is count-prefixed
      throughout; PROVEN LIVE by a cross-build restore, a save
      written at 50 restoring byte-identical at 99), so Phase 3 is
      un-gated completely. BUG-1453 = both loaders bound the
      lua_len count BEFORE their fill loops (branches sibling
      init_dungeon_branches() had the same shape); upstream-
      reportable, byte-identical in both 5.0 trees, artifact
      ~/nh50-dungeon-init-guards.patch; battery proved the panic
      end-to-end with a padded rebuilt nhdat (100 > 99), write
      breakpoints unhit. BUG-1454 = level_range() regained
      master's chain >= LEV_LIMIT bound (EvilHack hardening,
      vanilla 3.6 lacks it - port gap, NOT upstream-reportable);
      "max 98" in the live panic proves the bound tracks the new
      constant. ROW CORRECTIONS from the planning census: the
      master-only dungeons are SIX, not five - the list above
      misses The Wizard's Tower (master hoists wizard1-3 out of
      Gehennom via a wizportal CHAINBRANCH); post-port target is
      ~49 prototypes (master 47 + Tutorial 2), 15 dungeons, ~13
      branches. THE BINDING CONSTRAINT IS MAXDUNGEON, not
      LEV_LIMIT: global.h's 16 with init_dungeons()'s `>=` guard
      means max 15 dungeons and the full port lands at EXACTLY 15,
      zero spare - kept at 16 by session ruling (master parity;
      master runs 14 of 15); a 16th dungeon must bump it
      (save-neutral too, dungeons[] is saved count-prefixed).
- [ ] LOW, record for Phase 3 authoring (minted 2026-08-01 by the
      LEV_LIMIT slice): the Lua loader has NO equivalent of
      master's dgn_comp.y compile-time validation layer - the
      MAXLEVEL base/range couple checks, the MAXDUNGEON and
      LEV_LIMIT rejections with their edit-this-header hints are
      all gone, and an over-deep dungeon now silently clamps at
      init_dungeons()'s num_dunlevs > MAXLEVEL arm instead of
      being rejected with a message. The runtime guards (hardened
      by BUG-1453) are the only checks left, and they panic rather
      than explain. Anyone authoring the Phase 3 dungeon.lua
      content should know the failure modes: too many levels or
      branches = launch panic; too deep = silent truncation to 32
      levels; bad name lengths = the Strlen guards at
      init_dungeon_dungeons(). Fix-opportunistically: a
      validation pass in the loader printing WHAT overflowed
      before panicking would restore master's authoring
      ergonomics. SAME THEME (added 2026-08-01 by the converter
      GATE slice): lspo_map's invalid-map-char arm is an
      unimplemented upstream TODO ("/* TODO: warn about illegal
      map char */" then continue), so a bad map char silently
      keeps the fill terrain - engine-proven with a 'w' cell
      loading as STONE, zero messages. Implementing that warning
      belongs to this row.
- [x] Halloween undead generation bonus (NEW, minted 2026-07-26 by
      the A_NONE fix, a sibling gap in the same function).
      Master's rndmonst_adj() carries
      `if (is_undead(ptr) && halloween()) weight *= 3;` right after
      the weight line this fix touched. `halloween()` does not
      exist anywhere on the branch, so this is a whole unported
      EvilHack feature rather than a one-line add - deliberately
      NOT smuggled into the A_NONE bug fix. Check whether master's
      halloween() lives in hacklib.c and what else calls it.
      DONE 2026-07-26 (maintainer commit pending): include/extern.h
      + src/calendar.c + src/allmain.c + src/makemon.c, 24 added
      lines. Origin is master 76eacd0b1 ("Refactor rndmonst()."),
      whose message calls the feature a long-standing IRC joke:
      on 31 October undead spawn at triple weight and the player is
      told so at every game load, "just like when there's a
      full/new moon".
      SCOPE ANSWERED, both halves of the row's question: master's
      halloween() is in src/hacklib.c, and there are exactly TWO
      callers, both from that same commit - the rndmonst_adj()
      weight and a load notice in allmain.c. A case-insensitive
      sweep of all of master (src/, include/, dat/, doc/) turns up
      nothing else; the only other hits are "shallow" and
      "hallowed". Reviewer independently re-ran the sweep and
      agreed.
      THE HELPER MOVED HOUSE: 5.0 extracted the date routines from
      hacklib.c into src/calendar.c, and the three EvilHack birthday
      helpers (kathryn_bday/bourbon_bday/ozzy_bday) were ported
      there at some earlier slice - halloween() is the ONLY one of
      master's date helpers that was missed. Master's body uses
      getmday()/getmonth(), which 5.0 dropped and the branch does
      not have, so the port uses the single-getlt() idiom its three
      siblings in that file already use. That INCIDENTALLY AVOIDS a
      latent master defect, BUG-1384: master's getmday()+getmonth()
      pair is two independent time()+localtime() calls, so the
      predicate reads the day and the month from two clock samples
      and can straddle midnight, taking tm_mday from Oct 31 against
      tm_mon from Nov 1.
      SCOPE CORRECTED WHEN THE ID WAS ASSIGNED, and the correction
      is the part worth keeping: this was first recorded as
      affecting master's three bday helpers too. It does not. The
      month can only change across a midnight rollover when the day
      under test is the LAST day of its month, so kathryn_bday
      (12 Feb), bourbon_bday (19 Apr) and ozzy_bday (13 Jan) always
      land their second sample in the same month and cannot
      misfire; only halloween() is exposed, because 31 October is
      October's last day. friday_13th() in that same file already
      uses the single-getlt() form, so both shapes coexist there.
      A false positive is impossible - it would need the first
      sample on a 31st and the later one still in October, which
      requires 31 October already - leaving only a false negative
      in the microseconds of the Oct 31 -> Nov 1 rollover, i.e. the
      feature ends a few microseconds early on its last instant.
      Numbered at the maintainer's direction as a code-shape
      record, not an impact record; master frozen-unfixed, and the
      branch is correct by construction rather than by a fix.
      TWO DELIBERATE DEVIATIONS FROM MASTER, both maintainer-ruled
      at plan time:
      1. The x3 goes AFTER `weight += temperature_shift(ptr);`, an
         upstream 5.0 term master lacks, so it triples the FINAL
         weight rather than master's freq+align subtotal. There is
         no master position to be faithful to. Reviewer bounded the
         observable scope: svl.level.flags.temperature is set in
         only three places (mklev.c In_hell, one cold hellfill.lua
         fill, dat/fire.lua), and since temperature > 0 needs
         MR_FIRE while nearly every undead is MR_COLD, the
         deviation bites only for master lich / arch-lich in
         default Gehennom and for most undead on the one cold
         hellfill variant. Battery proved the +3 is inside the
         multiply: orc zombie 4 -> 12 at temperature -1.
      2. halloween() is HOISTED into a `hallow` local assigned after
         the qt_montype() early return, not called per candidate.
         Master paid up to 54 calls per rndmonst_adj(), each of
         which was TWO localtime()s via getmday()+getmonth(), so
         ~108; the branch pays 1. Reviewer's one LOW note: the
         hoist is unconditional, so it pays that 1 call on the
         ~364/365 of the year the answer is FALSE and on calls
         where no undead is even a candidate, where master's
         short-circuit paid 0. Still a large net win, rndmonst_adj()
         is not hot, and lazy assignment would reintroduce the
         definite-assignment question the hoist removes. Left as is.
      MASTER PARITY, DO NOT "FIX": the notice prints on RESTORE as
      well as new game (it sits outside `if (!resuming)`, like the
      moon-phase and Friday-13th notices, and master's commit
      message says that is the intent); halloween() is evaluated
      LIVE rather than cached into flags at game start the way
      flags.moonphase and flags.friday13 are, so a session running
      past midnight into 1 November loses the bonus mid-game -
      caching it would mean a save-format field for no benefit.
      GUARD HEADROOM, and the corrected number: the x3 cannot trip
      either arm of `weight < 0 || weight > 127`. All three addends
      are individually non-negative - maligntyp spans exactly -20
      .. 20 across all 562 records (plus A_NONE, normalised by
      54f3f27e6, so the two commits compose), which pins
      align_shift() to 0..5; temperature_shift() is 0 or 3; G_FREQ
      is 0..5 in the data. My plan quoted a 39 ceiling; THE BATTERY
      CORRECTED IT TO 27, because 39 assumed a G_FREQ-5 undead and
      the highest frequency among the 54 undead records is 2
      (wraith, skeleton) - verified independently. Reviewer adds
      that the G_FREQ MASK is 0x0007, so a future freq-6/7 monster
      would raise the ceiling to 45, still unreachable. Highest
      weight actually observed across 1348 instrumented samples: 15.
      AFFECTED SET IS AT PARITY: is_undead() and
      MH_UNDEAD (MH_ZOMBIE | MH_UNZOMBIE) are byte-identical
      between the trees, and both carry the same 54 records
      (12 MH_ZOMBIE + 42 MH_UNZOMBIE). Reviewer diffed the NAME
      sets, not just the counts, and they match.
      BATTERY 6/6 PASS (game-tester agent, 2 games), paniclog NEVER
      CREATED, and a breakpoint armed on the impossible() call
      itself never fired. Headline evidence: notice byte-identical
      on both the new-game and restore paths; on Dlvl 20 with
      align_shift 0, all 33 undead candidates went to exactly 3x and
      all 304 non-undead were unchanged, zero mismatches either way;
      10,000 direct rndmonst() returns went 7.68% -> 20.17% undead
      on Dlvl 20 and 8.06% -> 20.67% on Dlvl 14, against 7.59% ->
      19.77% predicted by the weight table; the quest arm hit
      qt_montype() 19 times in 20 and reached the weighted loop
      once, consistent with rn2(7), confirming the early return
      never reads `hallow`. Reviewer PASS, no critical/high/medium.
      THE BATTERY CORRECTED TWO PREMISES IN MY OWN BRIEF, which is
      the reusable part: (a) rndmonst() does NOT touch
      mvitals[].born - born is incremented inside makemon()
      (src/makemon.c), so "force several hundred rndmonst() calls
      and compare born" cannot work as written; the tester reported
      both a direct tally of rndmonst()'s return value AND born
      deltas driven by real makemon() calls, which is the better
      pair of metrics. (b) the 27-vs-39 ceiling above. Third slice
      running where the tester checked the brief's premise instead
      of trusting it.
      STAGING RECIPE FOR ANY FUTURE DATE-GATED FEATURE, since this
      is the first one tested on the branch: faketime and datefudge
      are NOT installed on this host and no TZ value can turn one
      month into another, so the date must be forced in gdb. The
      obvious technique - a breakpoint inside halloween() with
      `return 1` or a `set var` on the tm struct - IS UNUSABLE
      alongside gdb inferior calls: any breakpoint inside a
      gdb-called function aborts the evaluation ("stopped while in
      a function called from GDB"), leaves a dangling dummy frame,
      and killed one test game outright. What worked was a 4-byte
      in-memory text patch NOPing halloween()'s two branch-out jnes
      so control always falls into `mov $0x1,%eax` while getlt() is
      still called normally (binary is non-PIE, so file addresses
      are runtime addresses), verified each time with
      `print halloween()` and confirmed side-effect-free against
      friday_13th() and yyyymmdd().
      NOTE ON THE PORTED COMMENT: master's "chaotic-aligned levels
      will see a greater increase than lawful/neutral" is accurate
      as written but has no effect in the Dungeons of Doom, where
      svd.dungeons[0].flags.align is AM_NONE and align_shift()
      returns 0 for every candidate; the differential only appears
      on branches and levels that declare an alignment. Kept
      verbatim as master parity.
- [x] Bag of treasure/devouring rules; #tip interactions
      DONE 2026-07-26 (maintainer commit pending): include/obj.h +
      src/pickup.c + src/mkobj.c + src/mklev.c + src/apply.c +
      src/muse.c + src/mon.c + src/monmove.c. Ports master 417f2c972
      (feature) with f57302ef0 (mktrap exclusion) folded in, per the
      maintainer's one-commit ruling. Design spec was already written:
      FEATURES_ARCHIVE.md "Bag of Treasure and Bag of Devouring"
      (May 18 2026) plus the topic memory - this slice made it true.
      STARTING STATE WAS WORSE THAN "UNPORTED", which is the reusable
      part: Phase 1 had landed the two OBJECTS and the Croesus grant
      in makemon.c, but Is_container() stopped at BAG_OF_TRICKS and
      both bags sort after it, so they were NOT CONTAINERS AT ALL.
      Producer live, consumer absent - Croesus reliably dropped a
      "bag" that could not be opened, filled or used, and random
      generation and mktrap_victim could place one too. Same shape as
      P_THIEVERY being a dead skill before the #steal slice; no BUG
      id, since this is the normal unported state the row existed to
      close. The magic-chest slice had already predicted the work,
      its own deferral list naming "Is_container range to
      BAG_OF_DEVOURING (bags slice)".
      PREREQUISITE SWEEP found nothing blocking but three absent
      pools, and getting that right mattered more than the port:
      is_treasure_item() and Is_allbag were missing and are added
      here; the wish-parser class-word exception ("bag of devou-RING"
      parsing as a ring, a real master bug) was ALREADY PRESENT and
      5.0 has no second name_to_otyp() chain; but m_stash_items() is
      a whole unported subsystem, 5.0's mpickstuff() has no
      floor-container looting arm, and 5.0's m_move() object-seek has
      plain can_carry with no `|| Is_container(otmp)`. The archive's
      three demanded monster-AI exclusions therefore COULD NOT BE
      WRITTEN - you cannot exclude from a function that is not there.
      Recorded as riders on the two existing deferred rows above plus
      TODO-EVILHACK markers at both live sites, per maintainer ruling.
      TWO DIVERGENCES RESTORED ON PROVENANCE (vanilla@44b6612fd ==
      5.0 != master, the reverted-EvilHack-tweak signature): the
      maintainer ruled mbag_explodes() back to master's unconditional
      form (5.0 kept vanilla's depth-graduated odds; the divergence is
      only observable 2+ containers deep, since depth 0 and 1 always
      explode in both), and I applied the same ruling without
      re-asking to the sibling makeknown() block for the item that
      triggers a bag-of-holding explosion - same triple, and two of
      its four otyps are the new bags. The reviewer independently
      re-derived the provenance for that second call and confirmed it.
      REVIEW CAUGHT WHAT THE PORT AND I BOTH MISSED, and both findings
      came from the Is_container widening rather than the port itself:
      (a) BUG-1385, hornoplenty()'s tip-into arm is an unguarded
      add_to_container with no type check at all, so a horn of plenty
      tipped into a bag of treasure filled it with food at the /4
      weight discount and into a bag of devouring gave that bag
      PERSISTENT CONTENTS the design says are impossible - live on
      master too, since master's tip-target filter is
      Is_nonprize_container and master's Is_container already covers
      both bags; archived master frozen-unfixed, fixed here.
      (b) 5.0 has a FOURTH monster pool the archive could not have
      named because MASTER DOES NOT HAVE IT: find_misc()'s MUSE_BAG
      selector (src/muse.c) keys on Is_container() with only
      BAG_OF_TRICKS excluded, and 5.0's searches_for_item() is a
      RANGE test where master's is an otyp WHITELIST - so a monster
      that picked up or stole a filled bag of treasure would
      mloot_container() it empty, breaking the archive's stated
      "it then stays sealed" invariant. Both otyps now excluded there.
      LESSON: the archive's exclusion list was written against
      master's pools and is not sufficient for this tree - when a
      range macro is widened, re-derive the consumer set on the
      BRANCH rather than porting master's exclusion list.
      FOUR MORE FIXES from review and battery: the devour message
      hardcoded "is" and read "The rocks is devoured" (all three sites
      now otense(obj, "are")); mkbox_cnts()'s magic-bag substitution
      when filling a randomly generated bag of holding is keyed on
      Is_mbag, which deliberately excludes both new bags - my slice is
      what made them detonate a bag of holding, so seeding one inside
      became a forbidden state and the substitution now catches them;
      mktrap_victim nulls otmp after obfree, matching master; Is_mbag
      comment corrected to mention both bags.
      HONEST NEGATIVE, and it corrects a reviewer claim as well as
      mine: the mkbox_cnts fix guards an UNREACHABLE path. The battery
      read boxiprobs[] (src/mkobj.c) and found it carries no
      TOOL_CLASS at all, so a bag-of-holding content draw can never be
      a tool - measured 0 tool-class contents in 10000 filled bags.
      The pre-fix rate is 0, not the ~1/995 I briefed, and the
      PRE-EXISTING Is_mbag clause beside it is equally unreachable.
      Kept anyway for consistency with that clause and proven correct
      by forcing the branch (13/13 forced draws became SACK), but it
      is defensive code, not a live fix - do not cite it as one.
      VERIFICATION: reviewer CONDITIONAL PASS (every finding fixed),
      which also audited the tipcontainer() restructure - the riskiest
      edit, converting the branch's `if (targetbox) / else if
      (highdrop) / else` chain to master's to_floor form - and found
      every object takes exactly one terminal path with no leak,
      double-free or double-extract, the to_floor latch placed before
      the explosion arm nulls targetbox being the load-bearing detail.
      Battery 10/10 PASS then a 5-fix re-verify 5/5 PASS plus
      regressions, paniclog never created in either run. Headline
      evidence: 12/12 forced Croesus creations carried exactly one bag
      and it is now a working container; 40000 mktrap_victim calls
      produced 13462 tool draws and ZERO bags against ~21 expected
      unguarded; all four beatitude functions reweighed a LOADED bag
      immediately (x10/x1//4//8 confirmed by gdb and by dipping in
      holy water); 199 evaluations of a contents-bearing treasure bag
      in monster minvent produced 0 MUSE_BAG selections while ordinary
      sacks still drew 54, proving the fix is narrow.
      THE BATTERY FALSIFIED THREE MORE OF MY PREMISES (fifth and sixth
      slice running): the wish exception is in readobjnam_postparse1()
      not readobjnam_parse_charges() as I briefed - and the tester
      proved the guard load-bearing rather than vestigial by wishing
      "bag of vouring", which still ends in "ring" and duly produced a
      wooden ring; obj_resists(o,0,0) gives ARTIFACTS zero protection,
      so Excalibur is permanently destroyed by a bag of devouring
      (master parity, recorded not changed), and four of the six
      obj_resists items plus the Infidel quest artifact are caught by
      in_container's earlier "cannot be confined in such trappings"
      branch, leaving "refuses to devour" reachable via #loot only by
      a Rider corpse (the #tip path has no such earlier branch, so it
      fires normally there); and the amulet of life saving can NEVER
      be the rescue at the devour-you-whole site, because the death
      branch requires zero eligible items while the amulet is always
      eligible and is eaten first - structurally mutually exclusive.
      The tester substituted the Draugr racial LIFESAVED intrinsic and
      proved done(DIED) does honor life saving there, livelog
      recording "averted death (eaten by a bag of devouring)".
      MASTER PARITY, DO NOT "FIX": the first devour or refusal message
      in a game names the bag by its unidentified description, because
      xname(targetbox) is formatted before makeknown() runs; put-in
      says "only accepts gold and gems" while tip and horn say "only
      holds", both being master's own strings; oc_nowish blocks only
      non-wizard wishes, so wizmode wishing works by design.
- [ ] Object-lore display: master's add_obj_info() (NEW, minted
      2026-07-26 by the bags slice, which needed one line of it).
      Master's src/pager.c carries a 557-line add_obj_info()
      (1322-1879) called from its `/` lookup, printing per-object
      lore: damage, to-hit, material, weight, container subclass,
      erosion, and so on. NOTHING of it exists on the branch - the
      whole subsystem is unported and was never on any checklist.
      Found because master classifies the two new bags as subclass
      "container" there and I went looking for the branch counterpart.
      No dependency either way with the bags, which is why that one
      line is simply N/A rather than deferred.
- [x] Steed/saddle/barding architecture - COMPLETE 2026-07-26/27 in
      THREE commits, maintainer-ratified split: f6a787e0d barding +
      the saddle/riding racial gates (31 files +926/-82), 96d900884
      the carried fixes (13 files +150/-32), 48f0bf3c8 monster riding
      via erid (37 files +908/-39). Battery 24 PASS / 1 brief-error /
      1 latent, paniclog CLEAN.
      THE GROUNDWORK WAS ALREADY THERE AND UNREACHABLE, the same
      producer-live/consumer-absent shape as the bags slice: barding
      objects, is_barding(), W_BARDING, SUPPRESS_BARDING 0x80, the
      material tables, Ithilmar + its forge recipe and the #order
      "Remove barding" arm had all landed in earlier slices, but
      nothing could put barding on a monster, so all of it was dead.
      THE ARCHIVE KEYWORD SWEEP WAS THE HIGHEST-YIELD STEP AND
      BELONGS BEFORE THE PORT, NOT AFTER. ~35 archive entries mention
      steed/saddle/barding; checking each against the BRANCH BODY
      rather than assuming master's fixes travel with master's text
      found twelve whose targets are shared 5.0 code the branch still
      carried unfixed (BUG-386 pt2, 474, 579, 837, 925, 929, 930, 935,
      946, 948, 986, 987, 988), one FEATURE the tree adoption silently
      dropped (throwing food down to feed your steed - verified absent
      from vanilla at merge-base 44b6612fd AND from Official 5.0), and
      it is what led to BUG-1390. Four suspected entries turned out
      already correct on the branch and were left alone.
      MASTER BUGS ARCHIVED (frozen-unfixed, fixed on branch): BUG-1386
      there-menu saddle/barding removal queues a #loot prompt that no
      longer exists - on this branch 5.0's command queue then executes
      the orphaned canned 'y' as a movement key; BUG-1387 the Ithilmar
      tameness exemption returns out of mon_catchup_elapsed_time,
      costing the pet its off-level regen (maintainer ratified the
      divergence); BUG-1388 landmine saddle pointer captured before
      the scatter() that can free it. BRANCH-ONLY REGRESSION: BUG-1389
      upstream's steeds[] lacks S_SKELETON, so dog.c's
      put_saddle_on_mon(NULL,...) grant silently no-opped and the
      chaotic Draugr Knight started with an unsaddleable skeletal pony
      - the NULL obj arg suppresses the orphan impossible(), which is
      why nothing reported it.
      TWO USE-AFTER-FREES THAT 5.0 CREATED BY SPLITTING FUNCTIONS
      INTO HELPERS, both live in Official 5.0 and both upstream-
      reportable: BUG-1390, the hmon_hitmon() helpers take obj BY
      VALUE, so all seven places one of them uses the object up
      (shattered lance, splintered boomerang, broken mirror, eggs,
      cream pie, acid venom, barbed needle) left the caller reading -
      and on the venom path WRITING - through freed memory, which the
      neighbouring assert cannot catch because the pointer is non-NULL
      and freed; and BUG-1391, m_consume_obj() reads otmp->otyp after
      delobj() freed it, on every monster meal, while snapshotting
      every other field it needs. The reviewer found the first as a
      sibling sweep off the jousting half, which had been fixed alone.
      REVIEWER CRITICAL ON THE ERID COMMIT, master-impossible by
      construction: newcham() copies a pronoun into char oldmhe[4],
      correct on master whose genders[] has three rows, but 5.0 added
      a fourth ("they") that pronoun_gender() draws via rn2(4) while
      hallucinating - a five-byte write into a four-byte buffer on
      ~1 newcham in 4, and newcham runs constantly. Also dropped and
      restored: master's attack_checks() erid conjunct, without which
      an invisible rider on a visible steed is permanently
      unattackable while consuming a turn per try.
      5.0 ADAPTATIONS THE PORT REQUIRED (record these - a literal port
      gets them wrong): a ridden steed must carry mstate MON_OFFMAP,
      which then needs (a) ridden_by carve-outs in mon_sanity_check's
      BOTH loops or every mounted monster reports "mon is not there!",
      (b) a colocation exemption in place_monster(), (c) exemptions at
      the off-map early-outs in dochug and movemon_singlemon so the
      steed still attacks, and (d) mcalcdistress given its own loop,
      because iter_mons() skips off-map monsters and the steed would
      otherwise never regenerate, never time out blindness and never
      recharge a special attack. erid's raw mon_steed pointer is
      re-derived from the saved mid in the same restore pass 5.0 uses
      for u.ustuck_mid/u.usteed_mid, and the steed's ridden_by
      back-pointer needs the bones remap too. struct erid's field is
      spelled 'unsigned' because sftags maps 'unsigned int' to a type
      name with no serializer. MM_NOSTEED took 0x02000000 (first free
      bit above the GP_ block, no GP_ renumbering); MM_ERID was NOT
      ported - master defines it with zero callers.
      SAVE FORMAT BROKEN DELIBERATELY: struct erid joins
      critical_sizes[] beside its ebones/emsp/erac siblings and
      EDITLEVEL 2 -> 3, so old saves report "outdated savefile"
      rather than a datamodel mismatch. Dev saves and bones invalid.
      DIVERGENCES FROM MASTER, BOTH RATIFIED: the Ithilmar arm above
      (maintainer 2026-07-26), and mount_up() declining to take the
      mount the hero is sitting on (maintainer 2026-07-27, keep as
      defensive). Master lets a monster bind erid to u.usteed, which
      on this branch would also stamp MON_OFFMAP on the mount the
      hero is riding. TRACED AT RATIFICATION TIME AND CURRENTLY
      UNREACHABLE, which is why the guard's comment says so: a
      mounted steed can only lose tameness through kick_steed(),
      which dismounts the hero at zero; mount_steed()'s !mtame
      refusal is not force-gated, so even wizmode cannot mount a
      non-tame monster; nothing sets u.usteed->mtame or mpeaceful to
      0 directly; and mon_can_be_ridden() requires both !mtame and
      !mpeaceful. Re-check the guard if a future path ever untames a
      steed without dismounting.
      BATTERY (26 items, sanity_check on, 5 games): 24 PASS. NONE of
      "mon is not there!" / "placing X over Y" / "off-map steed" /
      "no monster to remove" ever fired, and the barded-monster
      sanity check ran ~90 turns silent, which is what proves the
      wearbits[] row. Mounted Nazgul rendered, farlooked "riding a
      saddled fell beast" and rider+steed both acted in the same turn;
      killing either party placed the other correctly; save/reload
      relinked with no "steed ID does not exist?". THE TWO NON-PASSES
      WERE BOTH MY BRIEF'S ERRORS, NOT DEFECTS: master's W_BARDING
      rust-trap arm is in the MONSTER branch only, so a hero riding
      over a rust trap damaging his own gear is correct (barding is
      water-reachable, proven by a thrown potion), and #stats folds
      erid bytes into the fmon total rather than printing a line.
      ONE LATENT FIX: BUG-579's update_inventory() calls are right but
      have nothing to show, because 5.0's perm_invent renders no
      weight header at all - master's curses port does. -> Phase 4 UI
      sweep. ALSO LEARNED: keepdogs() skips both parties, so a mounted
      monster can NEVER follow the hero across levels by design; and
      the spear trap is the only in-game damage source that reaches an
      off-map steed (wand of probing is the only wand that touches
      it).
      DEFERRALS, each verified absent by grep rather than assumed:
      gain_guardian_steed (Red Horse barding), selective_remove_curse
      (NEW - expert-clerical auto-uncurse of steed barding, belongs
      with the spell-skill subsystem), hero-cast SPE_REPAIR_ARMOR
      prompt, an AD_BHED dispatcher arm, master's mind-flayer-larva
      steed displacement (that feature is unported; whoever ports it
      must carry the block), and the two items minted below.
      MASTER DEFECTS REPRODUCED VERBATIM FOR PARITY, no id assigned:
      resist_conflict()'s Ithilmar arm is an else-if on the
      resist_chance > 19 clamp, so "always resist" is false exactly
      when Charisma and level are high; and can_saddle()/
      can_wear_barding() read mlet from r_data() but test the ladder
      against mtmp->mnum, so a raced monster is matched against the
      wrong species list.
- [x] Glass/blocker combat (was "Armor-blocker miss flavor", minted
      2026-07-26 by the steed slice) - COMPLETE 2026-07-27 in FOUR
      commits: 707efb221 AC foundation, 844b61273 glass breakage,
      faf70b62f the melee-odds ruling, 2c8ee6887 the deflection
      ladders. Reviewer pass on every commit, every finding
      coordinator-verified against both trees before acting.
      THE ROW UNDERSTATED ITS OWN SCOPE, and the reason is worth
      keeping: it was written as message flavor, but the prerequisite
      check found the AC foundation underneath it missing. matac[] and
      material_bonus() were never ported, so armor materials have
      contributed ZERO AC since the tree adoption while
      init_obj_material() has been randomizing them - a leather-
      material plate mail protected exactly as well as a steel one.
      BUG-1392. The branch's ARM_BONUS macro was deleted rather than
      left as a second, incomplete AC formula; all ten callers now go
      through master's armor_bonus(), which is the macro plus the
      material term and the dragon-scaled +5.
      P_SHIELD WAS UNTRAINABLE, which is the other thing the row did
      not know it owned: the skill exists and eight roles start with
      it, but all five of master's use_skill(P_SHIELD, 1) sites live
      inside these ladders.
      FOUR SLICES' DEFERRALS CLOSED, all by name: the forge slice's
      break_glass_obj + FQ_INFERIOR fall-apart channel (tracker line
      1219, explicit ruling), the webbing slice's spit_object
      exemption (2539), the kill-attribution slice's missmm blocker
      sear (1606), and the materials slice's blocker-sears family
      (611). The steed slice's two W_BARDING arms land via
      which_armor, which already knows a hero never wears barding.
      ONE HELPER, NOT THREE LADDERS: master writes the blocker walk
      out three times; the branch has armor_blocker() in worn.c taking
      the running to-hit total by pointer so missmu can continue with
      the golden haze, which is not an object. Reviewer verified it
      arm-for-arm against all three master copies - same order, same
      short-circuits, same rn2(10)/rn2(10)/rn2(5) draw count and
      positions, including master's quirk that a lucky glove overrides
      an inner piece's claim.
      MAINTAINER RULINGS, both recorded as precedent:
      (1) MELEE GLASS ODDS ARE MASTER'S. break_glass_obj was briefly
      gated twice for crackable armor - upstream's nonbreakchance = 90
      inside breaktest plus master's rn2(6) force gate - compounding to
      ~1.7% per blow per crack step, ~240 blows to destroy. Ruled that
      where the trees disagree on melee fragility master wins; the
      melee channel now runs master's own obj_resists(obj, 1, 99) and
      never consults breaktest, giving 16.5% per blow. Throws, drops
      and kicks keep upstream's gentler roll. The arm also tests
      oerodeproof, mirroring master's breaktest glass arm, else
      erode_obj announces "somehow, your helm is not affected" every
      few blows and identifies the erodeproofing for free.
      (2) A DISPLAY OPTION MUST NOT DECIDE WHETHER DAMAGE LANDS. In
      master the sear and the breakage sit inside the message block, so
      an off-screen fight, a blind hero or verbose off means no damage
      and no broken armor - and master is not self-consistent about it,
      since its monster-attacks-hero version applies both regardless.
      All three directions now apply the mechanics whenever a blocker
      was found. Visible consequence: a blind hero deflecting off a
      hated material takes the searing, and unseen monster fights
      resolve like watched ones.
      MASTER DEFECTS FIXED RATHER THAN REPRODUCED: hmonas' engulf arm
      passed an UNINITIALIZED dieroll to missum (master assigns one,
      but by drawing a second unrelated rnd(20), so master judges that
      miss by the wrong roll; -O0 hides the read, -O2 reports it);
      mattackm's engulf arm passed missmm a dieroll the melee path
      never wrote, so m-v-m engulf deflection has essentially never
      fired in master; neither mattackm miss site reported a dead
      attacker or defender, though missmm can kill the attacker with
      the sear and can petrify a cockatrice-wielding defender by
      breaking its gloves; break_glass_obj's off-hand unwield was
      gated on u.twoweap, which the primary weapon's own destruction
      clears behind its back, freeing the object with uswapwep still
      pointing at it; hitmm takes the weapon by value so master's
      null-out never reaches mattackm, which then reads it for the
      pudding split and hands it to passivemm's AD_ENCH drain_item.
      THE CRACK-MODEL COLLISION IS THE ONE TO REMEMBER: upstream added
      a model master does not have - glass armor cracks four times
      before dying, and breakobj() returns early to erode_obj() to do
      it. Master's break_glass_obj assumes breakobj always deletes, so
      ported verbatim it announced "breaks into pieces", unwore the
      piece by hand and reported destruction for armor that had merely
      cracked and was still in the pack, with every caller believing it
      gone. Any master function that ends in breakobj() needs the same
      check.
      MASTER QUIRKS PRESERVED, no id assigned: missum's sear tests the
      DEFENDER's barkskin/stoneskin where the other two directions test
      the attacker; "stone hide" in missum against "stony hide" in the
      other two.
      NOT PORTED, recorded below as their own rows: extra_pref's rework
      (monster ring-wearing), is_better_armor (monster looting), the
      remaining Ashmar/Retribution behaviour sites, the dmgval
      body-material arms, the glass piercer, rustm's EF_DESTROY.
- [x] Open-air terrain - PORTED 2026-08-02 with the tower slice
      (commit 1 of the two-commit ruling). is_open_air() in dbridge.c
      (AIR typ outside the endgame; no new terrain type) + consumer
      arms across 18 files in 5.0-native idiom: hero fall death in
      spoteffects (iron-ball pull, clinger/steed exemptions), both
      end.c lifesave arms (discharged an UNRECORDED third deferral -
      the end.c TODO marker), float_down plummet + ball-over-air
      pull, run/travel/lookaround refusal, dochug + minliquid
      inopenair monster gravity (mon_moving mongone vs xkilled
      attribution), mfndpos + could_reach_item avoidance, mimic
      disguise suppression, pet drop guard, flooreffects object
      destruction (obj_resists items migrate to hellc_level when
      defined, rloco in-level fallback until the Gehennom slice;
      ball/chain yank death), boulder push-over-edge, hurtle msg,
      corpse_chance no-corpse, bones veto, wizbury guard,
      bad_location, goodpos + MM_IGNOREAIR (0x04000000L - master's
      0x200000L collides with GP_ALLOW_XY), scatter waschain removal
      (BUG-924 shape), unpunish saved-coords drop (BUG-294 shape),
      dismount_steed ABUSE_HAZ_FALL (steed.c TODO discharged). BOTH
      recorded deferrals closed. hellc_level plumbing rode along
      (dgn_topology field + level_map entry, inert until Gehennom).
      NO-SIBLING master sites recorded: flooreffects Amulet-teleport
      guard (feature unported - carry its open-air exception when it
      lands) and make_helper's MM_IGNOREAIR caller (sphere spells
      unported). Battery-caught port defect fixed pre-commit:
      lookaround's "uninteresting terrain" arm classified AIR as
      skippable and shadowed the stop arm (runs walked into known
      chasms); open air excluded there. VERIFIED-INTENDED master
      parity: the FIRST step of a run into adjacent seen air and
      travel's FINAL step into an explicitly-selected air destination
      both fall (master's lookaround gates only continuation; its
      travel arm exempts the destination cell). BUG-1456 rode with
      this commit: the vampshifter tier cluster the adoption reverted
      (see the archive entry). Battery: full open-air leg PASS (fall
      + Die?-decline with NO relocation - hero stays suspended on the
      air cell; flying traversal; object destroyed; Amulet rloco'd
      in-level exactly-1; engrave refusal; dismount plummet with
      abuse counter + align -1; nest stability post-BUG-1456).
- [ ] Flames of hell (in_hell_effects) ambient fire subsystem (NEW,
      minted 2026-08-05 by the gehennom slice; maintainer ruled own
      row). Master allmain.c calls trap.c in_hell_effects() every
      turn the hero is in Gehennom outside the Valley: periodic fire
      damage to non-fire-resistant heroes with its own in_hell_killer
      attribution ("the flames of hell"), plus assorted gear/heat
      arms - read the full master body before scoping, the function
      is larger than the headline mechanic. ENTIRELY absent from the
      branch (no in_hell_effects, no in_hell_killer). Prerequisites
      all landed with the gehennom slice (Is_valley was always
      there; the tier levels now exist so the hazard has somewhere
      to matter). Interaction check for the port: 5.0's
      svl.level.flags.temperature/temperature_shift system and the
      hellish_smoke_mesg arrival path are the native heat layer -
      decide whether in_hell_effects sits beside it or partially
      merges. BUG sweep hits to carry: BUG-284/285 mention it only
      incidentally (sink_into_lava counters), verify at port time.
- [ ] monclock / MIN_MONGEN_RATE spawn-rate ladder reconcile (NEW,
      minted 2026-08-05 by the gehennom slice; maintainer ruled own
      row). Master allmain.c tightens the random spawn-rate floor as
      the game progresses, keyed on gehennom_entered, hella_entered
      and hellc_entered plus BUG-660's got_quest_early bit (struct
      q_score, absent on branch) and svc.context.timeout_start
      (EXISTS on branch, include/context.h:161, but is DEAD - no
      C reader/writer). 5.0 rewrote spawn pacing entirely, so this
      is a DESIGN RECONCILE against the 5.0 scheduler, not a
      re-port; the _entered milestones themselves landed with the
      gehennom slice. Decide: port master's ladder onto the 5.0
      scheduler, translate its intent into 5.0-native knobs, or
      ratify the 5.0 pacing and retire timeout_start + the BUG-660
      remainder.
- [ ] Mplayer kit reconciliation (NEW, minted 2026-08-01 by the
      endgame-slice battery, which made the branch's first real
      Astral visit). Astral creation fires impossible("bad mplayer
      monster") + a paniclog line whenever create_mplayers rolls
      convict/druid/infidel: the 5.0-native equipment switch in
      mk_mplayer covers only the 13 vanilla roles while the roster
      block PM_ARCHEOLOGIST..PM_WIZARD carries 16 (the roles slice
      ported init_mplayer_erac's 16-case race switch but never
      widened this one), so those three spawn with no switch kit
      plus one impossible each. NOT a three-arm bolt-on: master has
      NO equipment switch in mk_mplayer at all - master kits
      mplayers via the makemon m_initweap/m_initinv tables, which
      ERAC B2 already ported (makemon.c is_mplayer arms live), and
      branch mk_mplayer calls makemon WITHOUT NO_MINVENT then tops
      up the 13 vanilla roles from the 5.0 switch. Reconcile the two
      kit paths (extend the switch vs route through the ported
      tables) and audit the vanilla 13 for double-kitting while
      there. ESCALATED 2026-08-05 by the yendor slice: wizard1.lua
      des-places an infidel, so the impossible now fires
      DETERMINISTICALLY on every wizard1 generation (file load,
      real entry, wizmakemap - 3/3 in the slice battery), not just
      on Astral mplayer rolls. Pick this row up before players
      reach the tower.
- [ ] music.c earthquake reconcile (minted 2026-08-06 by the Valkyrie
      slice's archive sweep). Two master fixes do not transplant into
      5.0's rewritten do_earthquake() (the do_pit() helper shape):
      BUG-1053's quake_worm_hit[] long-worm dedup (master hits each
      worm once per quake; the branch shape needs its own multi-hit
      analysis before any port) and BUG-1057/BUG-331's player_caused
      threading, dormant until a slice ports a monster
      do_earthquake() caller (master's wizard.c tactics earthquake -
      harassment-sweep adjacent). The drum of earthquake is
      player-reachable today, so the dedup half is live exposure.
- [ ] wizard.c target_on()/strategy() residual master deltas (minted
      2026-08-06 by the Valkyrie slice's reviewer, not taken with
      BUG-1477): master's is_mplayer covetousness arms (the M_Wants
      early-out exemption; Purgatory-era mplayer covetousness -
      travels with the mplayer/purgatory rows) and the you_have
      STRAT_PLAYER !mtmp->mpeaceful gate (peaceful covetous monsters
      do not target the hero). BUG-799 mhpmax guard + BUG-806 Astral
      aligned-altar block remain with the Wizard-harassment sweep
      (2026-08-05 ruling).
- [ ] hack.c forcefight fire-burn (fire_wep/burnhd) NOT PORTED (minted
      2026-08-06 by the Archeologist slice's C-hook audit). Master's
      forcefight block computes burnhd (u.umburn burning-hands form,
      bare-handed) and fire_wep (wielded ITEM_FIRE-oprop weapon or
      Fire Brand/Dichotomy/Xiuhcoatl/Angelslayer via
      wielding_artifact) and lets forcefight burn through flammable
      doors, trees and dead trees (~90 lines of message/effect arms
      incl. door_flammable, elf guilt, smoke). Branch forcefight
      (domove_fight_* refactor) has only explo/solid; its web arm
      already burns via attacks(AD_FIRE, uwep), which would
      incidentally cover the fire artifacts - keep that arm's
      behavior in mind when reconciling. The Xiuhcoatl pager blurb
      "Can burn through flammable doors and trees" (pager deferral)
      advertises this feature - port them together or the blurb lies.
- [ ] dothrow.c launcher range table NOT PORTED (minted 2026-08-06 by
      the Archeologist slice's reviewer while proving the range>0
      gate's producer absent). Master's throwit() range overrides for
      fired ammo - Crossbow of Carl +12, dark elven hand crossbow +8,
      elven/orcish/racial bow urange adjustments (master
      src/dothrow.c ~1602-1636) - vs the branch's vanilla
      `range = BOLT_LIM` arm. Launcher/ammo mechanics, no quest
      coupling; the underwater turbulence producer itself WAS ported
      with the slice (BUG-1487).
- [ ] Conduct wiring reconciliation (reflection/antimagic/forgedarti)
      NOT PORTED (minted 2026-08-06 by the Knight slice's C-hook audit).
      The u.uconduct counter fields and topten.c token-string readers
      (add_achieveX) are live on branch and the Knight slice ported the
      artifact.c writers (SPFX_REFLECT 4-arm block, set_artifact_intrinsic
      antimagic arm was already there), but the rest of the master wiring
      is absent: insight.c #conduct/enlightenment display lines for
      reflection ("acquired reflection") and antimagic, topten.c
      encodeconduct() xlogfile bitmask bits (branch stops at bit 15
      forgedarti; master carries antimagic bit 22 / reflection bit 23 -
      xlog parseability ruling applies, Phase 7 must re-verify), worn.c
      equipment writers (master gates via armor_provides_extrinsic(), a
      helper the branch lacks - covers amulet/shield/barding of
      reflection and the "magic resistance from worn gear" antimagic
      sibling), and the polyself.c silver-dragon-form reflection bump.
      The forge row's battery already measured the same display/xlog gap
      for forgedarti - close all three conducts in one pass here.
- [ ] Slow-status ladder (u_slow_down HSlow timeout + Fast/Very_fast
      !Slow gating) NOT PORTED (minted 2026-08-07 by the Monk slice's
      battery, leg D6). include/youprop.h's own TODO-EVILHACK above
      the Slow macro already flags the gating half. Master's
      u_slow_down() (mhitu.c) is a real status setter: internal
      defended(&youmonst, AD_SLOW)/resists_slow guard ("spry as
      ever"), three-tier messages (slow down / strange lethargy /
      settling in for the long haul), incr_itimeout(&HSlow, rnd(11)
      + 12), exercise. The branch keeps vanilla's Fast-strip-only
      body (HFast = 0L), so HSlow is only ever written by the hunger
      arm (eat.c FROMHUNGER) and all seven u_slow_down call sites
      (mhitu AD_SLOW gaze BUG-1502, zap.c youdefend, muse.c, trap.c,
      uhitm.c x3 incl. the branch-only mhitm helper site) deliver a
      Fast strip where master inflicts ~13-23 turns of Slow.
      Battery-measured: a non-Fast hero gets "You slow down." with
      ZERO state change. Port the body + the youprop.h !Slow gating,
      audit master's movement/botl Slow consumers, then re-verify the
      seven call sites in one pass. The gaze arm's !Slow gate and
      defended-subject fix landed with the Monk slice and are
      forward-compatible with this row.
- [ ] frac_altar (fractured altar) subsystem NOT PORTED (minted
      2026-08-06 by the Knight slice's crowning-parity port). Master
      levl[x][y].frac_altar: crowning cracks the altar the hero is
      crowned on (all four alignment arms in gcrownu, master pray.c
      ~899/932/967/1021 - lightning-strike message pair, one-shot via
      the frac_altar==0 gate); the field, its save/restore, display and
      any other consumers need enumeration from master before the
      writes can port. The Knight slice deliberately left the crowning
      arms without the crack (subsystem absent on branch; branch
      mklev.c has one comment referencing frac_altar bits - start the
      enumeration there and at master rm.h).
- [x] Shop pricing racial adjustments (shk_racial_adjustments) -
      ALREADY PORTED, ticked 2026-07-27 after verification rather
      than after a port. The row was never a work item: the
      subsystem rode in with an earlier slice and nobody came back
      to tick it. Verified in-tree: shk_racial_adjustments() in
      shk.c is master's body verbatim modulo 5.0 idiom (prototype
      shape and two reflowed comments are the only diff), both
      call sites are present in get_cost() and set_cost() with the
      numer/denom argument order inverted between them exactly as
      master has it, shk_stat_priced() and the gnome/nymph
      charisma carve-out are there, matprices[] carries the
      EvilHack material rows, and doc/shop-price-matrix.md is on
      the branch.
      LESSON, same shape as the A_NONE row above: unchecked does
      not mean unported. Diff the branch body against master
      BEFORE planning any remaining row - two of the fourteen open
      Phase 2 items turned out to need nothing.
- [x] BUG-1393 mounted save is unrestorable (NEW, minted 2026-07-27 by
      the glass/blocker slice's BATTERY, which is the only reason it was
      seen; NOT that slice's doing - it touches none of restore.c,
      check_wings or the Flying macro). FIXED SAME DAY, src/restore.c
      only. Restoring any save made while riding SIGSEGVs in
      check_wings() before the welcome line, twice reproduced, once from
      a 3-turn minimal repro.
      ROOT CAUSE: save.c writes the whole u struct including the raw
      u.usteed POINTER, which is meaningless in the next process - its
      own comment says the pointers must not be dereferenced - and the
      valid handle is u.usteed_mid. dorecover() calls getlev() BEFORE
      restgamestate(), so getlev's relink cannot fire yet; restgamestate
      then loads the stale pointer into u.usteed; check_wings(TRUE)
      runs sixteen lines BEFORE the u.usteed = NULL that exists for
      exactly this reason, and its first statement reads Flying, which
      dereferences u.usteed->data.
      OWNER: the player-races R2 slice (721196586, 2026-07-18) placed
      the check_wings call there. Unmounted restores are safe because a
      NULL steed short-circuits, which is why the steed slice's own
      battery missed it - it never saved while mounted.
      THE FIX IS THE ORDERING, NOT THE LINE: check_wings() moves to just
      after the getlev() that re-links the steed, which is master's
      relationship (master's restlevelstate() really re-links, ours is a
      5.0 stub) and is also what set_uasmon() already asks for, since it
      defers its own call "until we have a steed" - the deferral was
      being resumed before the steed existed. restgamestate() also
      clears u.ustuck/u.usteed itself now, right after setuwep() makes
      the last legitimate use of one; the reviewer's decisive catch was
      that the clear must NOT go any earlier, because setuwep() reads
      u.usteed's non-null-ness as a "hero is mounted" flag for the
      "bashing" reminder.
      Battery 14/14 PASS, paniclog empty: both crash saves captured from
      the ORIGINAL failure, written by the pre-fix binary, now restore.
      A flying steed restores with flight correct, which the old
      placement could not have got right even without the crash.
      LESSON FOR THE REST OF THE PORT: this is what porting a line
      instead of a relationship looks like. Master's call site was
      correct because of what ran immediately before it; the branch kept
      the position and lost the precondition. When a ported call depends
      on state another function establishes, check that the 5.0 version
      of that function still establishes it - restlevelstate() had been
      reduced to a stub.
      Full mechanism, repro and cores in CRASH_TRACKING.md.
- [x] Body-material weapon-damage arms (NEW, minted 2026-07-27 by the
      glass/blocker slice; maintainer ruled it a row of its own rather
      than smuggling a combat-balance change into an AC-foundation
      commit). PORTED 2026-07-28, include/mondata.h + src/weapon.c.
      is_wooden() widened as a composition of the has_bark() the branch
      already had rather than by re-listing master's PM_ constants -
      same four species, and the petrified ent stays excluded because
      it is stone, not bark. All four arms landed verbatim in branch
      idiom; none of them reads an attacker, so no signature change was
      needed. Battery-confirmed at the dmgval() return: axe 4-7 vs ent
      against mace 0-2, mace 6-13 vs skeleton against dagger 0-1, and
      the petrified ent showing clean unmodified rolls on both weapons.
      MASTER'S FINAL CLAMP CAME WITH THEM AND IS NOT OPTIONAL: master
      ends dmgval with `if (tmp < 0) tmp = 0;` under the comment
      "certain bonuses can make tmp negative", and the branch had no
      such clamp because every pre-existing bonus was positive. The two
      new penalty arms are the first that can drive the running total
      below zero, and the branch's tail only clamps inside `if (tmp >
      0)`, so without it dmgval would have returned negative damage to
      callers that add it to a total. Caught in self-review, not by the
      reviewer - the general shape is that porting a penalty into a
      function that has only ever computed bonuses invalidates its
      clamp reasoning.
      THE HERO-SIDE TWIN WAS DELIBERATELY NOT PORTED, as this row
      instructed. It is unnecessary as well as buggy: the branch runs
      AT_WEAP damage through one choke point, mhitm_ad_phys() in
      uhitm.c, which hands mdef == &gy.youmonst straight to dmgval(),
      so the hero-on-defense direction got all four arms for free.
      Battery confirmed it: a mace against a Barkskin hero dropped from
      2-7 to 0-1, a dagger against a Stoneskin hero to 0, and a mace
      against a Stoneskin hero stayed unchanged - the right arm firing
      for the right defender in the direction that has no code of its
      own. Master's mhitu.c late block stays unported and its FQ
      double-count stays frozen-unfixed.
      include/mondata.h is_wooden() on the branch was
      upstream's wood-golem-only form; master's is WOOD_GOLEM || ENT ||
      ELDER_ENT || is_blight. src/weapon.c dmgval carries 1 of master's
      4 anatomy arms: master has axe +rnd(4) vs wooden/plant/barkskin,
      WHACK -rnd(3)-3 vs the same, PIERCE|SLASH -rnd(5)-3 vs
      bone/stoneskin, and WHACK +rnd(4) vs bone. These are the
      substantive consumers of the is_bone_monster() the AC-foundation
      commit added - until they land it only feeds flavor text.
      The hero-side twin is master mhitu.c's late damage-adjustment
      block and is ENTANGLED with master's FQ double-count, already
      flagged frozen-unfixed by the forge slice: port the weapon.c
      arms, decide the mhitu twin separately.
- [x] Glass piercer + the piercer damage rework (NEW, minted 2026-07-27
      by the glass/blocker slice). PORTED 2026-07-28, src/hack.c only.
      Master's restructure landed: dmg hoisted to the top of the arm,
      the dodge roll moved ahead of the helm check, hard helm halves
      instead of nullifying, glass piercer against a glass helm pays
      +rnd(6). Battery confirmed the reorder is visible in play - "you
      are almost hit" now fires while wearing a hard helm, which the
      old short-circuit made impossible - and that a metal helm now
      costs 5-9 HP where it used to cost none.
      THREE DEVIATIONS FROM MASTER, all deliberate:
      (1) the honesty fix this row asked for. breakobj() is called and
      its RETURN gates both the message and the bonus damage, so the
      claim is only made on the crack that actually finishes the helm.
      Battery: 34 tracked encounters, cracks reported as cracks at
      eroded 1/2/3, no false claim once. Master's manual setworn() +
      update_inventory() were deliberately NOT ported - on this branch
      erode_obj() does the unwearing through remove_worn_item(), which
      is strictly more correct because it runs Helmet_off().
      (2) hard_helmet() is used, NOT master's is_hard(). MAINTAINER
      RULING 2026-07-28. Master has no hard_helmet() at all - verified
      by git grep over the whole of master's src/ - and uses is_hard()
      everywhere; hard_helmet() is upstream 5.0's narrower helper
      (metallic or crackable, so it excludes wood/bone/stone) and the
      branch already stands on it at ten falling-object sites. Taking
      master's predicate at this one site would have made a wooden helm
      deflect a piercer but not a falling rock. Glass helms satisfy
      both predicates, so the new glass path is unaffected either way.
      The tree-wide is_hard-vs-hard_helmet question is its own row
      below.
      (3) a soft helm keeps the "you are hit" message. Master's
      structure prints nothing at all for a leather helm - no shatter
      line, no glance line, no hit line - so the damage arrives silent.
      MESSAGE TEXT DEVIATES TOO, and the battery is why: erode_obj()
      announces "Your helm shatters!" itself on this branch, so
      master's "It pierces and shatters your helm!" landed directly on
      top of it and said shatters twice. Reworded to "It punches
      straight through the glass!" per maintainer 2026-07-28 - keeps
      the glass-through-glass flavor that explains the bonus damage
      without repeating the line erode_obj already printed.
      master hack.c's S_PIERCER arm in
      spoteffects is restructured: a hard helmet HALVES damage instead
      of nullifying it, and a glass piercer against a glass helm
      shatters it for +rnd(6) bonus damage. The branch has upstream's
      arm (hard helmet = no damage, no glass case). This is the ONLY
      remaining master breaktest/breakobj call site with no branch
      counterpart. NOTE master's version has a defect not to copy: it
      gates on breaktest() then calls breakobj() unconditionally, which
      on this branch routes through the crackable erode_obj arm and
      usually does NOT destroy, so "It pierces and shatters your helm!"
      would lie exactly the way break_glass_obj did before the fix.
- [x] rustm() erosion parity (NEW, minted 2026-07-27 by the
      glass/blocker slice). PORTED 2026-07-28, src/mhitm.c +
      src/uhitm.c. AD_DCAY joins AD_CORR and the flags become master's
      EF_GREASE | EF_DESTROY. Battery-confirmed on every leg: corrode
      (black pudding), the new decay arm (brown pudding, which did
      nothing to weapons before this), rust (rust monster) and burn
      (fire vortex), each destroying a maxed weapon without incident,
      m-v-u and m-v-m both.
      THIS ROW WARNED THE GUARD MUST LAND WITH IT AND THE WARNING WAS
      AN UNDERCOUNT - it says "both call sites are consistent today",
      which was true of the two rustm call sites and false of the
      call CHAIN. Four guards were needed, not two:
      - uhitm.c mhitu arm, master's o_id + o_on() pattern plus
        mon_currwep_gone(); the drow-taint block below it reads and
        WRITES otmp->otainted. This is BUG-563's shape, except the
        branch had reordered rustm to run BEFORE the taint block
        instead of after it, so the exposure was worse than master's.
      - uhitm.c mhitm arm, same pattern; mwep->opoisoned is read on
        the very next line.
      - mdamagem(), which takes mwep BY VALUE from hitmm(). Reads it
        at four sites after mhitm_adtyping() returns, one of them a
        real dereference through troll_baned().
      - mattackm(), which owns the original pointer. Reads it in the
        pudding-split test right after hitmm() returns, and hands it
        to passivemm() further down.
      THE LESSON IS THE FRAME COUNT. The reviewer caught the last two;
      the first two were not enough because mhitm_ad_phys() does not
      receive mwep - it recomputes MON_WEP(magr) locally - so nulling
      that local reached neither caller. Three independent copies of
      one pointer, and a fix that only clears the innermost looks
      complete and is not. When a leaf function gains the ability to
      free an object, every frame between it and the object's owner
      has to be walked, not just the frame holding the call.
      The comment already sitting above mattackm's break_glass_obj
      null-out states this invariant exactly, and it is the reason it
      is stated there: it was written when break_glass_obj gained the
      same power.
      SIBLING FIX IN THE SAME LINES, maintainer-approved 2026-07-28:
      mattackm's pudding-split test read objects[mwep->otyp].oc_material
      - the object TYPE's default - where master reads the instance
      (mwep->material) and the branch's own hero-side sibling in
      uhitm.c reads the instance too. Under EvilHack's per-instance
      material system those disagree: a wooden-instance long sword split
      puddings and a steel-instance club did not. Now reads the
      instance. Battery-confirmed both directions: two soldiers'
      long swords set to WOOD produced zero splits over 22 turns of
      melee against black and brown puddings; the same two objects
      switched to IRON split them immediately.
      master's rustm passes EF_DESTROY and
      handles AD_DCAY alongside AD_CORR; the branch carries upstream's
      EF_GREASE | EF_VERBOSE with neither, so a monster's weapon can
      never be destroyed by the hero's rust/corrode/decay polyform.
      That is also why BUG-563's guard is not needed on the branch -
      its hazard does not exist until this lands, and it MUST land with
      the guard, since the block downstream of rustm reads the weapon.
      Both call sites (hero-defense and m-v-m) are consistent today.
- [x] mthrowu hero-on-defense AC model (NEW, minted 2026-07-27 by the
      glass/blocker slice). PORTED 2026-07-28, src/mthrowu.c only,
      three lines. The hit gate runs on realac = AC_VALUE(u.uac); the
      near-miss comparison stays on raw u.uac against dieroll - 2,
      which is master's own arrangement and deliberate - the outcome
      is randomized, how close the shot came is reported honestly.
      Restores the branch's third AC_VALUE call site; it had matched
      upstream's two.
      Battery: at AC -44 a centaurian sergeant's thrown daggers still
      landed repeatedly, interleaved with honest misses, shield blocks
      and the near-miss line. That is the whole point of the row -
      before this, enough negative AC made the hero untouchable by
      thrown weapons.
      thitu used raw u.uac + tlev; master uses
      realac = AC_VALUE(u.uac). Upstream 5.0 also uses raw u.uac, so
      this is an unported master divergence, not a 5.0 regression.
      Belongs with the to-hit/AC roll matrix - the hero-on-defense
      -rnd() arm is the whole reason the hero is never fully immune.
- [x] find_ac shield-skill AC block (NEW, minted 2026-07-27 by the
      glass/blocker slice). PORTED 2026-07-28, src/do_wear.c +
      src/weapon.c. master's ladder landed verbatim, and master's
      skill_init() Knight/Valkyrie starting P_BASIC came with it -
      without that bump the two roles that can reach master-rank
      shield skill would have started no better than anyone else at
      the thing their kit is built around.
      Battery: bare AC 10, +3 small shield, ladder measured at
      Unskilled 6 / Basic 5 / Skilled 3 / Expert 1 / Master -2, and
      the bracer split measured separately at Basic -1 / Skilled -2 /
      Expert -3 / Master -4. Valkyrie and Knight both start Basic; a
      Wizard has the skill restricted and absent from #enhance.
      P_SKILL() is safe before skill_init() runs - weapon_skills[] is
      zero-initialised as part of struct you, so every rank reads
      P_ISRESTRICTED and no arm fires; uarms is NULL then anyway.
      STILL UNPORTED, master's other P_SKILL(P_SHIELD) consumers: the
      shield-bash chance in uhitm.c (three sites) and the muse.c gate.
      Training the skill now pays in AC but not yet in offense.
- [ ] pager farlook remainder (NEW, minted 2026-07-27 by the
      glass/blocker slice, which ported the armor-weight half). master's
      ", wielding %s" + MON_WEP2 arm is already claimed by the
      "pager.c probe wielding-both display" deferral; the
      hallucinatory_armor() helper and the hallucinatory-weapon else
      branch have no owner yet.
- [ ] look_at_monster buffer accumulation (NEW, minted 2026-07-27 by
      the glass/blocker slice). buf is BUFSZ and every clause before
      the new armor line appends unbounded, including player-nameable
      distant_monnam and the newly live ", riding <name>". Master and
      upstream are both unbounded here and master is strictly worse
      (it also appends mon_wounds and the wielding clause). The armor
      line itself was written bounded; the function wants an audit.
- [ ] Dexterity-to-AC ladder (NEW, minted 2026-07-28 by the combat
      damage/AC slice, which found it while verifying find_ac() for the
      shield row). NOT PREVIOUSLY ON ANY ROW - the tree adoption
      dropped it silently and no keyword sweep would have found it.
      master's find_ac() carries a dex_adjust_ac ladder: Dex <= 6 gives
      +3 AC, 7-9 +1, 10-14 0, 15-16 -1, 17-18 -2, 19-20 -3, 21-23 -4,
      >= 24 -5, then cancelled outright (clamped to no better than 0)
      when the body armor is heavy metallic / bone / stone / wood /
      glass, or when near_capacity() >= SLT_ENCUMBER. The branch's
      find_ac() has no dex_adjust_ac local at all. Maintainer ruled
      2026-07-28: record it now, port it in a later slice, so the
      shield commit stayed AC plumbing rather than a character-balance
      change.
      RIDES WITH IT: ART_ARMOR_OF_RETRIBUTION's -5 in the same
      function is also absent on the branch - it belongs to the
      existing Ashmar/Retribution behaviour row, noted here because
      both gaps are in find_ac() and one read of the function should
      close both.
- [ ] is_hard() vs hard_helmet(), tree-wide (NEW, minted 2026-07-28 by
      the combat damage/AC slice). master has NO hard_helmet() at all -
      git grep over master's whole src/ finds zero - and uses is_hard()
      at every falling-object site. hard_helmet() is upstream 5.0's
      helper and is NARROWER: metallic or crackable, where is_hard()
      also covers wood, bone and stone. The branch currently stands on
      hard_helmet() at ten sites (ball.c, dothrow.c, read.c x2,
      potion.c, mhitu.c, trap.c, dig.c, objnam.c, do_wear.c).
      The piercer slice hit this and was ruled to hard_helmet() for
      consistency, but that means a wooden or bone helm gives no
      protection against ANY falling object on the branch where it
      would on master. Decide once, tree-wide; do not settle it a site
      at a time.
- [ ] Monster off-hand weapon rolls damage from the primary (NEW,
      minted 2026-07-28 by the combat damage/AC slice; PRE-EXISTING,
      not that slice's doing, and surfaced by its reviewer).
      mattackm() and mhitu.c's mattacku() both select the off-hand
      weapon correctly for a 2nd+ AT_WEAP attack - MON_WEP2, with
      OFFHAND_PENALTY applied - and it correctly drives to-hit and the
      mswings message. But the damage path does not receive it:
      mhitm_ad_phys() recomputes MON_WEP(magr) locally in both its
      m-v-u and m-v-m arms, so a dual-wielding monster's off-hand
      swing scores damage from the PRIMARY weapon. On master the late
      mhitu damage-adjustment block consumed mon_currwep and kept the
      two in sync; the branch has no consumer, so nothing ever
      noticed. Now that the anatomy arms and rustm's EF_DESTROY both
      key off that pointer, the off-hand swing also erodes and can
      destroy the wrong weapon. Fix is to thread the swung weapon into
      mhitm_ad_phys rather than recompute it.
- [x] objnam.c wish tables and plural handling (NEW, minted AND closed
      2026-07-28 by the oddities the combat/AC battery reported while
      staging test gear). Phase 1's object swap ec925f41a brought
      EvilHack's objects.h but left src/objnam.c as vanilla 5.0, so every
      table in it either indexes the wrong objects or omits EvilHack's
      outright. Three reported symptoms, one cause; the sweep found the
      rest.
      LANDED: master's dragon-scale-mail redirect above mksobj (the dead
      TODO stub in the post-creation switch deleted with it), the
      CHROMATIC_DRAGON_SCALES and SCR_CONSECRATION non-wizard downgrades,
      three stale dragon ranges modernized to the FIRST/LAST macros,
      as_is[] + pair_of() + xname_flags() + obj_typename() taught that
      bracers and goggles are already plural, master's MUMMIFIED_HAND
      carve-out and MEAT_BOOTS/MEAT_GLOVES arm, o_ranges[] endpoints
      adopted wholesale plus master's bracers row, and spellings[] unioned
      with master's 25 drow aliases and four other entries.
      WORST OF THE SWEEP FINDINGS: wishing "hat" returned a RANDOM OBJECT
      OF ANY CLASS. EvilHack reordered the helmet block so DUNCE_CAP now
      precedes FEDORA, the vanilla row reads {FEDORA, DUNCE_CAP},
      rnd_class() returns STRANGE_OBJECT when last <= first, and mkobj()
      then rolls a random class. Six sibling rows silently truncated.
      THE LESSON IS PARSE ORDER, and it nearly shipped a new bug: master
      runs the alternate-spelling lookup BEFORE the monster-name stripper;
      5.0 inverted them. Porting master's "drow dagger" aliases into 5.0's
      order meant name_to_monplus() ate "drow" as the monster it also is,
      and all 21 aliases silently produced plain base items. Caught by the
      reviewer, not by the battery, and not by the build. Fixed by giving
      the stripper's existing exclusion list (samurai sword, wizard lock,
      master key) a general test: alt_spelling_otyp() now answers "this
      whole string is a known alias", which also de-duplicates the
      spellings loop and self-polices any future alias whose first word
      names a monster. A DATA table is not order-independent when the code
      around it was reordered.
      Battery: every part PASS. 21/21 drow aliases, 14 dragon colors
      including shadow and celestial, non-wizard downgrades proven in a
      real non-wizmode game with a legitimately zapped wand, "hat" x26 all
      headwear, blind-pickup showing the generic word rather than leaking
      "etched bracers", and a named type reading "a pair of bracers called
      Foo" rather than "a pair of shield called Foo" - the last two being
      reviewer catches on top of the port.
- [x] Eyewear lost its property in the object swap (NEW, minted 2026-07-28
      by this slice's battery, which noticed a worn blindfold did not
      blind; PRE-EXISTING, not the objnam slice's doing). FIXED SAME DAY,
      include/objects.h only.
      ROOT CAUSE, and it is the A_NONE shape again - Phase 1 DATA meeting
      5.0 CODE. Upstream declares facewear with a dedicated EYEWEAR(name,
      desc, kn, prop, ...) macro carrying a property; blindfold and towel
      get BLINDED. The branch had no EYEWEAR macro at all and carried all
      four as plain TOOL(), whose expansion hardcodes oc_oprop 0. setworn()
      grants extrinsics from oc_oprop, and 5.0's Blind macro reads
      EBlinded - where master's read the ublindf POINTER, which is exactly
      why master never needed the property and why a byte-faithful
      conversion of master's entry silently broke it here.
      Property enum starts at 1, so the 0 went to an unused slot; nothing
      wrong was granted, blindness simply never arrived. Swept every other
      property-bearing macro (ARMOR/HELM/CLOAK/SHIELD/GLOVES/BOOTS/
      DRGN_SCALES/RING/AMULET/POTION): all survived, EYEWEAR was the only
      one missing, so the blast radius is these four objects.
      Battery: blindfold and towel both blind and unblind cleanly with
      EBlinded going 0x80000 (W_TOOL) and back; lenses are the negative
      control and stay at 0; telepathy through a blindfold paints and
      TRACKS monsters outside line of sight, proving the extrinsic reaches
      a real per-turn consumer rather than a one-shot.
- [x] Goggles behavior (NEW, minted 2026-07-28 by the objnam slice, which
      made goggles wishable and correctly named but not functional).
      PORTED 2026-07-29 in two commits: the equipment plumbing, then
      See_underwater.
      THE ROW SAID GOGGLES "NEED A DESIGN CALL ON WHAT THEY DO" AND THAT
      WAS WRONG - master answers it, and the first answer given was wrong
      too. They do NOT confer Breathless; that macro is already
      byte-identical here and was never the gap. Goggles are a source of
      master's `See_underwater`, which lets the hero see while submerged
      instead of only feeling the adjacent water. The misreading came from
      quoting a macro's continuation lines without its `#define`.
      See_underwater is a pure EvilHack invention - absent from upstream
      5.0 as well as from the branch - and all ~15 of its master call
      sites were unported, so the branch was plain 5.0 at each and
      underwater vision was unconditionally restricted. Two TODO-EVILHACK
      markers in polyself.c named it.
      THE SITE COUNT WAS WRONG TOO, AND THE REASON GENERALIZES: the census
      said thirteen sites because it was anchored on `git grep GOGGLES
      master`. src/iactions.c is 5.0-only, has no master counterpart, and
      therefore CANNOT appear in a master-anchored census - it carries the
      item-action menu's own BLINDFOLD/TOWEL/LENSES list, so goggles had
      no "Put these on" entry. Found by the reviewer. When porting an
      object into 5.0, census the BRANCH's sibling-object sites too, not
      just master's sites for the object being ported.
      THE BATTERY CAUGHT A REAL BUG IN THE PORT: removing eyewear while
      submerged left the full map painted on screen forever, correctable
      only with ^R. Master hides this by calling docrt() every turn in
      BOTH moveloop arms; the branch keeps upstream's cheaper first arm,
      so nothing redrew on the way out. Fixed at the transition instead of
      per-turn, in Blindf_on()/Blindf_off(), using the same old_uwvis
      compare the polyself sites use - which is the better shape and
      avoids master's per-turn full-screen clear. Retest: three
      remove/wear cycles plus the Eyes and a blindfold control, every
      transition byte-identical to a forced ^R.
      DELIBERATE DEVIATION, recorded because it is invisible otherwise:
      master's first moveloop arm also calls docrt(); ours does not, and
      the moveloop's new second arm carries !Is_waterlevel so the Plane of
      Water does not repaint every turn. Measured: 2 docrt() calls in 22
      submerged turns there.
      KNOWN LIMITATION, MASTER PARITY, LEFT AS-IS: a monster on an
      adjacent DRY tile is not drawn while submerged, even with
      See_underwater. The rule is terrain-based - a monster on adjacent
      WATER is drawn, and the same monster rloc'd between the two flips
      visibility. newsym()'s first disjunct returns early for any
      non-pool/non-ice square and is unguarded on master too; only
      docrt()'s remembered-glyph blit paints land. Farlook does not report
      the monster either, though its action messages still print. Not
      goggles-specific: identical for a tortle's innate See_underwater.
      WIZMODE-ONLY ODDITY, not reachable in play: #wizmap while submerged
      opens a position prompt rather than mapping, and repeated use
      toggles a state where the auto-drawn screen disagrees with ^R.
      SCOPE NOTE FOR ANYONE READING THIS LATER: See_underwater is true for
      every tortle from XL1 - they take intrinsic Swimming at XL1
      (attrib.c trt_abil[]) and the macro's first clause is
      `HSwimming && (... || racial_tortle(...))`. This commit therefore
      changed underwater play for a whole race, not just for goggles
      wearers. Battery-verified against a non-tortle control at the same
      level.
- [ ] Amphibious is missing racial_tortle (NEW, minted 2026-07-29 by the
      goggles slice, which edited the same macro block and deliberately
      did not touch this). master's Amphibious is
      `(HMagical_breathing || EMagical_breathing || amphibious(...)
      || racial_tortle(&youmonst))`; the branch is at plain upstream 5.0
      and lacks the last term, though racial_tortle exists here and is
      used in 18 places. A tortle therefore survives deep water only via
      the Swimming disjunct in drown(), and reads Amphibious == FALSE at
      telekinesis.c and polyself.c. Kept out of the goggles commit because
      it makes a whole race amphibious - a gameplay change that wants its
      own row, per the standing ruling. Note See_underwater legitimately
      references racial_tortle as part of its own content; that is not
      this gap.
- [ ] #up out of water (NEW, minted 2026-07-29 by the goggles slice).
      master's doup() opens with an `if (u.uinwater && (HWwalking ||
      HFlying))` block letting a hero with intrinsic water walking or
      flying step or fly out of a pool; the branch's doup() has no such
      block and neither does upstream, so it is EvilHack-only and
      unported. It is also the one See_underwater consumer with no branch
      counterpart - when it lands it must carry
      `if (See_underwater) vision_reset();` before its docrt(), exactly
      as pooleffects() does. Same note for master's pager.c
      append_spot_lighting(), whose "in light"/"in darkness" farlook
      annotation is likewise unported and carries the other missing
      See_underwater site.
- [x] Plane of Water floods paniclog under sanity_check (NEW, minted
      2026-07-29 by the goggles battery; PRE-EXISTING, unrelated to that
      slice - it fired on arrival, before any goggles interaction).
      FIXED SAME DAY as BUG-1394, src/mkmaze.c only.
      The row asked whether the check should exempt MON_BUBBLEMOVE or the
      bubble code should clear it. It is the latter, and not as a
      judgement call: the bit is set when a bubble lifts a monster and is
      never cleared ANYWHERE - three hits tree-wide, the #define, the one
      |=, and a comment in place_monster() naming it among bits that
      function deliberately preserves. Nothing reads it. mon_offmap() is
      `mstate != MON_FLOOR` with MON_FLOOR == 0, so the stale bit made
      every carried monster read as permanently off-map, and the checker
      only reaches that arm for a monster that IS on the map at its own
      coordinates - so the complaint was correct and the state was wrong.
      Exempting the bit in the checker would have been the sanity-heal.
      Cleared in the CONS_MON restore arm, before the placement rather
      than after, so that elemental_clog()'s migrating/limbo/obliterated
      outcomes are not stomped. place_monster()'s mask left alone.
      UPSTREAM 5.0 BUG, verified character-identical in NetHack50 and
      Official_nh50 - added to the upstream-reportable table.
      Battery: 5056 instrumented carries over 187 turns with up to 238
      monsters, every DROP 0x10 and every PLACED 0x0 with occupancy
      confirmed, zero mstate lines, elemental_clog exercised 99 times. A
      deliberate positive control proved the checker was live.
- [x] see_monsters() calls newsym() on off-map monsters (NEW, minted
      2026-07-29 by the BUG-1394 battery, which is the only reason it was
      visible - the mstate flood was drowning it out). FIXED SAME DAY,
      src/display.c only: the skip becomes `DEADMONSTER(mon) || !mon->mx`,
      which strictly subsumes the PARKEDMONSTER term it replaces.
      `!mon->mx` was chosen over mon_offmap() deliberately - mx == 0 is
      precisely what newsym() cannot handle, and it holds whether or not
      mstate bookkeeping is correct. BUG-1394 had just proved mstate can
      drift, so the display path should not depend on it.
      Battery: zero newsym lines over 187 turns on the plane, and the
      reported backtrace reproduced FIVE times by forcing u.ustuck to be
      mnearto()'s displaced monster - 3, 46, 43, 3 and 31 off-map
      monsters in the following see_monsters() walk, i.e. 126 newsym(0,0)
      calls that no longer happen, with paniclog untouched. Monsters
      still render and still track their bubble; vault guard escort
      re-tested end to end.
      The SECOND paniclog source on the Plane of Water, 28 lines:
      `impossible("newsym: attempting screen update for <0,0>")`.
      see_monsters() (src/display.c) skips DEADMONSTER, PARKEDMONSTER and
      MON_STILL_ARRIVING, then calls `newsym(mon->mx, mon->my)`
      unconditionally - but a monster a bubble is carrying has mx = my = 0
      and is not any of those three. newsym() rejects column 0 with
      impossible(). PARKEDMONSTER is `isgd && mx == 0`, i.e. upstream
      already special-cases ONE kind of off-map monster in that loop and
      not the general case. Captured backtrace: movebubbles -> mv_bubble
      -> mnearto -> mon_leaving_level -> unstuck -> docrt -> docrt_flags
      -> see_monsters -> newsym(0,0). Byte-identical to upstream 5.0, so
      it is theirs too. Fix is to skip any monster with mx == 0, which
      strictly subsumes PARKEDMONSTER; newsym() on column 0 is never
      valid, so nothing legitimate is lost.
- [ ] mon_sanity_check() exempts the parked guard's position but not its
      mstate (NEW, minted 2026-07-29 by the see_monsters battery, which
      hit it during the vault-guard regression test; NOT that fix's doing
      - display.c cannot touch mstate). One line, on an ordinary level,
      at the moment a vault guard departs:
      `impossible("floor mon (0x...) with mstate set to 0x00000200")`.
      0x200 is MON_PARKED.
      THIS ONE IS THE OPPOSITE OF BUG-1394 AND THE DIFFERENCE MATTERS.
      There the state was wrong and the checker was right. Here the state
      is CORRECT: parkguard() (src/vault.c) deliberately places the guard
      at <0,0> with MON_PARKED set while the temporary corridor is torn
      down, and restfakecorr() clears the bit again - a real, bounded,
      by-design window, which is why it fires once rather than per turn.
      The checker already knows that position is legitimate - its isok()
      arm carries the explicit carve-out `!(mtmp->isgd && x == 0 && y ==
      0)` - and then its final mon_offmap() arm has no matching carve-out
      for the mstate bit that says the same thing. So the fix is to
      exempt the parked guard there too, and that is NOT a sanity-heal:
      nothing is being papered over, the check is simply incomplete.
      Byte-identical in upstream 5.0.
      DEFERRED TO LATER IN PHASE 2, maintainer 2026-07-29. It was traced
      to the line while the Plane of Water work was in flight and could
      have ridden along; held back because it is a different level and a
      different bug from the two bubble fixes, and those commits should
      not carry an unrelated rider.
      READY TO PICK UP - no re-derivation needed. The fix is one
      condition in mon_sanity_check() (src/mon.c): give the final
      `else if (mon_offmap(mtmp))` arm the same parked-guard carve-out
      its own isok() arm already has, i.e. do not complain when the
      monster is a guard parked at <0,0> with MON_PARKED set. Prefer
      testing that trio explicitly over a bare PARKEDMONSTER(), so the
      exemption stays narrow and a guard that is off-map for any OTHER
      reason still gets reported.
      VERIFY BY: ordinary level, sanity_check on, enter a vault, take
      the gold, let the guard arrive and escort you out. The line fires
      exactly once, at "Suddenly, the guard disappears." Repro recorded
      by the see_monsters battery: Dlvl 3, vault at rooms[8], teleport
      in, pick up the gold, wait for the guard, answer the name prompt,
      drop the gold, follow until the corridor collapses.
      WATCH FOR: whether a guard can also be parked while the hero is
      off-level or the level is being saved - if so the carve-out needs
      to hold there too. Not investigated.
- [ ] levl vision blocking on the Plane of Water (NEW, minted 2026-07-29
      by the see_monsters battery). THIRD paniclog source on that level,
      4 lines, always in pairs on one square, two bursts in 187 turns:
      `impossible("levl[%i][%i] vision blocking")` from
      levl_sanity_check() (src/wizcmds.c). Independent of both bubble
      fixes - it is a terrain/vision-table disagreement, not monster
      state. An earlier battery saw the same message and could not rule
      out its own gdb staging; this run got the [31][16] pair during
      plain play with nothing attached, so it is real. Sporadic, so
      expect to need a long instrumented run to catch it.
- [ ] "pairs of" at quan > 1 (NEW, minted 2026-07-28 by this slice's
      battery). A stack of the mergeable pair-items renders "2 pair of
      meat gloves" where it wants "2 pairs of". Only reachable on the
      FOOD_CLASS meat gloves and boots, since armor does not merge.
      Master has the identical construction so this is master parity, not
      a regression - but neither tree pluralizes the "pair".
- [x] passive_obj() has no AD_DISN case (NEW, minted 2026-07-30 from
      the 2026-07-29 review of the July 24-29 commits; HIGH; verified
      still live at mint time). LANDED SAME DAY as BUG-1400 (battery
      PASS; commit 67547eee2): master's case ported between AD_ENCH and default (the
      ENCH mcan fallthrough became a plain break so a cancelled
      disenchanter cannot fall into the new case), fallback widened
      to the six adtypes, AND a prerequisite the row missed -
      master's passive_obj() void -> int return with ER_* codes plus
      the two thrower reactions (thitmonst return-1, drop_throw
      broken=TRUE), without which the port re-creates BUG-310's
      freed-pointer reuse; drop_throw's master guard IS that fix.
      update_inventory() tail keyed on ER_DESTROYED first so it never
      reads the freed obj. EF_DESTROY erosion flags deliberately NOT
      ported - their MEDIUM row below now owes only the flags.
      Original finding: Black dragon / antimatter vortex
      passive object disintegration is a silent no-op: the AD_DISN arm
      b6a2f9576 added to passive() carries a comment claiming
      passive_obj() handles the object side, and passive_obj()
      (src/uhitm.c) has no such case. Port master's case AD_DISN block
      (obj_resists -> Hand of Vecna -> Dragonbane -> black dragon
      scales -> artifact 1-in-20 -> erodeproof -> remove_worn_item +
      delobj) AND widen the bare-handed NULL-obj fallback that routes
      to uarmg: master routes FIRE|ACID|RUST|CORR|ENCH|DISN where the
      branch's `if (!obj && mattk->adtyp == AD_ENCH)` routes ENCH
      only. BATCH: one passive-arms slice with the AD_CNCL row below.
- [x] passive() has no case AD_CNCL (NEW, minted 2026-07-30 from the
      2026-07-29 review of the July 24-29 commits; HIGH; verified
      still live at mint time - zero AD_CNCL hits in src/uhitm.c).
      LANDED SAME DAY as BUG-1401 (battery PASS; commit a09223367):
      master's arm in
      master's position, mhitb/gy.youmonst form; cancel_monst()
      verified NULL-obj-tolerant (only use is obj ? obj->oclass : 0)
      and youattack stays FALSE per the BUG-1308 family. The branch's
      cancel_monst is still 5.0's reduced implementation (zap.c
      TODO-EVILHACK) - the arm strengthens for free when the
      cancellation slice lands.
      Original finding: The gray dragon's passive slot is
      ATTK(AT_NONE, AD_CNCL, 0, 0),
      so melee-attacking a gray dragon never cancels the hero.
      Master's arm in passive()'s first switch: if (mhit && !rn2(6))
      cancel_monst(&youmonst, weapon, FALSE, TRUE, FALSE). (AD_SLIM
      is also absent from passive(), but it is absent upstream too -
      upstream gap, not a port regression; not part of this row.)
      BATCH: one passive-arms slice with the AD_DISN row above.
- [x] Quest artifact destroyable by bag of devouring (NEW, minted
      2026-07-30 from the 2026-07-29 review of the July 24-29
      commits; HIGH; verified still live at mint time - no "resists
      being confined" arm in src/pickup.c). LANDED as BUG-1402
      (commit a202b734f): master's arm restored verbatim between the
      Amulet/invocation and LEASH arms; bricking mechanism verified
      (artiexist bit never cleared by obfree, recreation gated on it,
      finish_quest needs the physical artifact and its invent scan is
      non-recursive). PREMISE CORRECTION: this row's "add the same
      guard to the tipcontainer devouring arm" was wrong - both trees
      already guard that arm with !obj_resists(otmp, 0, 0) and master
      has NO quest arm there (protection is transitive once insertion
      is refused; quest artifacts neither generate in containers nor
      survive into bones). Exploration instead surfaced THREE missing
      obj_resists pre-checks in the same family, landed as BUG-1403
      (commit d56763327):
      do_boh_explosion(), boh_loss(), and tipcontainer's cursed-mbag
      arm (the obj_resists half of the BUG-1284-third-arm row below).
      FUTURE DEPENDENCY: master's m_stash_items() (monsters stash
      items into containers) is entirely unported and carries this
      same quest-artifact guard - it MUST travel when that function
      is ported; no branch counterpart exists today.
      Original finding: master's in_container()
      refusal arm ("resists being confined until the quest is
      completed", between the Amulet/invocation-items arm and the
      LEASH arm) was never restored; the branch guards only the
      cursed-bag attack path (bag_devours_invent, two sites), so a
      hero can still voluntarily stash their own quest artifact in a
      bag of devouring and lose it for good. Add the same guard to
      the tipcontainer devouring arm while there. Permanent
      quest-bricking: obfree never clears artifact_exists, so
      qcompleted can never be set afterwards.
- [x] EF_DESTROY armor siblings (NEW, minted 2026-07-30 by the
      2026-07-29 review; MEDIUM port gap). rustm() got master parity
      but the sibling erosion entry points still carry upstream's
      non-destroying flags, so armor can never be destroyed by
      erosion outside rustm(): erode_armor() (uhitm.c, 7 sites),
      passive_obj() (4 sites - master also changed its signature
      void -> int so callers can react), trap.c's burn_dmg macro +
      acid_damage() + water_damage(), and potion.c's acid arm.
      Master's ERODE_FRACTURE type has no branch counterpart at all.
      UPDATE 2026-07-30: the void -> int signature half landed with
      BUG-1400 (all passive_obj arms record ret; both thrower callers
      react to ER_DESTROYED). This row now owes only the EF_DESTROY
      flags at the listed sites - note master's fire/acid passive_obj
      arms also carry !Underwater guards the branch lacks, bring them
      along - plus the trap.c/potion.c siblings, the erode_armor()
      sites, and the ERODE_FRACTURE decision.
      LANDED 2026-07-31 as four commits (battery PASS, paniclog
      never created; evidence in the archive entries): BUG-1410
      1f77678b1 ported the destroy-arm guards FIRST as a
      prerequisite (uball/uchain + obj_resists 5/95 + iron-safe
      bail + box contents dump + in_lava_effects wrap + Hand of
      Vecna exclusion); BUG-1411 8c598b845 carried master's
      mattacku() cover-break refresh (the BUG-821 fix lives in
      shared code and had never traveled - archive-sweep find);
      BUG-1414 e67f51153 widened all 19 flag sites with the
      !Underwater gates, the waterproof-container exterior rust,
      and four caller-guard groups from the UAF audit (hitum
      secondwep, passive() choke - which also closed a LIVE
      AD_DISN/dragon-scale UAF present since BUG-1400 - hmonas
      slot re-read, passiveum mon_currwep sync); BUG-1412
      f4d3a8ddc fixed sit.c's reintroduced BUG-329 boots typo
      found during verification. ERODE_FRACTURE deliberately
      deferred to rows D1/D2 below; erosion_matters() width to D4.
- [ ] D1: AD_LOUD / AT_SCRE sonic combat subsystem unported (NEW,
      minted 2026-07-31 by the EF_DESTROY parity slice; MEDIUM
      port gap). Master's executors - mhitu.c screamu() behind the
      mattacku AT_SCRE arm (damage, deafness, glass fracture,
      destroy_item of POTION/RING/TOOL/WAND), mhitm.c's two
      AD_LOUD arms with destroy_mitem, uhitm.c's AT_SCRE polyform
      arm, and mcastu.c m_destroy_armor() - none exist on the
      branch. LIVE TODAY: celestial dragon (monsters.h:2540) and
      Nazgul (:3559) carry AT_SCRE/AD_LOUD attacks with no
      handler, so the slot is silently skipped in both mattacku
      and mattackm default arms, and the dochug ranged-scan
      (monmove.c:1130) counts AT_SCRE as usable so they select
      ranged targets they cannot attack. Depends on D2 for the
      glass-shatter leg; destroy_mitem already exists (muse.c).
- [ ] D2: ERODE_FRACTURE decision (NEW, minted 2026-07-31;
      decision row, tied to D1). Master's erosion type 4 is
      ERODE_FRACTURE (is_glass, "fracture"/"fragmentation",
      COST_FRACTURE) feeding the sonic subsystem; the branch's
      type 4 is 5.0's ERODE_CRACK (is_crackable crystal armor,
      COST_CRACK) - same slot, DIFFERENT semantics, a bare port
      collides. Decide: renumber master's fracture in as a new
      type alongside crack, or fold the glass fracture into the
      crack model. COST_FRACTURE/COST_DETERIORATE also absent
      from the branch COST_ enum.
- [ ] D3: ERODE_DETERIORATE defined but unhandled (NEW, minted
      2026-07-31; LOW, latent). include/obj.h defines it (5) but
      the branch erode_obj() switch has no arm - any caller would
      hit the impossible() default. Master's arm:
      is_supermaterial, secondary erosion, COST_DETERIORATE. No
      branch caller today; lands naturally with D1/D2.
- [ ] D4: erosion_matters() vanilla-narrow vs master (NEW, minted
      2026-07-31; MEDIUM port gap; supersedes the REVIEW-LOW
      SLING_BULLET note's erosion_matters half). Master lets all
      of TOOL_CLASS fall through (containers included) and adds
      AMULET/RING/WAND/FOOD and SLING_BULLET; the branch returns
      TRUE only for weptools + WEAPON/ARMOR/BALL/CHAIN. Until
      widened, containers cannot erode, which keeps three
      already-ported master-shape pieces of erode_obj()'s destroy
      arm dormant (iron-safe bail, box contents dump,
      water_damage's waterproof-container exterior rust - battery
      documented the dormancy live: an iron safe in water shows
      erosion_matters==0 while co-located ball/chain erode; the
      Amulet also short-circuits at this gate BEFORE the
      obj_resists guard). Widening is a real balance change
      (rings/wands/food start eroding) - port deliberately, with
      the doname/xname erosion-word consumers in the same slice.
      Related anomaly recorded by the battery: a WIELDED
      thrice-corroded weapon's dip prompt drops the "thoroughly
      corroded" adjective that the carried form shows - check the
      doname worn-suffix path during that slice. NOTE 2026-07-31:
      BUG-1427 landed master's doname GEM_CLASS SLING_BULLET arm -
      inert until this row widens erosion_matters, at which point
      sling-bullet erosion words become visible with no further
      naming work.
- [x] dmgval() missing the cursed-vs-angel/aasimar bonus arm (NEW,
      minted 2026-07-30 by the 2026-07-29 review; MEDIUM port gap).
      weapon.c: the mirror of the blessed arm the branch has - cursed
      && (is_angel(ptr) || is_aasimar(ptr)) -> bonus += rnd(4).
      Attacker-free, drops into the 2-param dmgval unchanged.
      (Master's Infidel cursed arm IS blocked by the 2-param
      signature - correctly out of scope.)
      LANDED 2026-07-31, BUG-1409 67ae1c655 (battery PASS: gdb
      matrix showed exactly +2 hitval and the +rnd(4) dmgval
      spread vs an Angel, flat vs the soldier control). SIBLING
      SWEEP FOUND MORE: hitval() was missing the same cursed arm
      (+2, also attacker-free) - both ported. The Infidel cursed
      arms in BOTH functions joined the Druid arm in the
      TODO-EVILHACK attacker-param-threading deferral comment.
- [x] enexto_core_mon() family unported (NEW, minted 2026-07-30 by
      the 2026-07-29 review; MEDIUM port gap; the BUG-506/525/536/
      559/560/575/610 archive family). separate_steed_and_rider
      (steed.c, 2 sites), the bones.c steed site and the newcham
      steed site all use enexto(..., ->data), so gear-based fly/
      levitation is ignored - a rider rloc'd across the level, or a
      steed mongone'd in bones. Port enexto_core_mon/
      enexto_core_full from master teleport.c (the other half of
      BUG-610, acting on the return value, WAS carried).
      LANDED 2026-07-31 as two commits (battery PASS - the BUG-506
      whistle repro placed a gear-levitating pet on water 6/18
      applications, it SURVIVED there, and the negative control
      never touched water): BUG-1413 0b6fb92b7 = the mechanism
      (both NEW_ENEXTO arm bodies refactored to a shared
      enexto_core_full, the dead ring arm compile-checked;
      enexto_core_mon keeps 5.0's GP_CHECKSCARY two-pass and
      upgrades to real onscary()) PLUS the load-bearing
      companions the review row missed: branch goodpos(),
      minliquid_core() AND mfndpos() all lacked can_levitate/
      can_fly terms (m_in_air is type-data-only), so the entry
      point alone would have placed pets on water and DROWNED
      them a turn later - all three gained the terms; plus the
      four named sites. BUG-1415 59177863e = the 18-site
      master-parity sweep (dismount trio = BUG-525's real scope,
      fixuporacle + goodpos pre-check, mnexto/maybe_mnexto/
      mnearto, revive_nasty x2, mhitu hero drops x2 = BUG-887,
      u_collide_m = BUG-575, whip + grapple = BUG-559, sp_lev
      retry = BUG-536, vault guard, clone_mon, u_teleport_mon),
      skip list per master's BUG-560 audit.
- [ ] Bare-enexto residue audit (NEW 2026-07-31, replaces the old
      "~59 call sites" AUDIT RIDER; LOW). After the sweep, ~24
      bare enexto() calls remain and all are believed-correct
      fakemon users: pre-creation PM_*/corpsenm spawns (mcastu,
      muse, minion, dokick, timeout, mkroom, zap, mail, apply
      figurine, makemon eel-distress), steed.c PM_BAT/PM_GHOST
      probes, newcham NEW-form pre-placements (vamp_stone,
      decide_to_shapeshift - passing the form about to be assumed
      is the point), dbridge edata, landing_spot forceit, and the
      two sites master also left bare (mcastu bypos, teleport.c
      level-full relocation). Re-audit only when a caller passes
      ->data of an already-existing monster. Three master
      enexto_core_mon sites have no branch counterpart because
      their features are unported - each must arrive as
      enexto_core_mon with its feature: apply.c hooked-axe pit
      escape, mon.c larva-host steed displacement, uhitm.c
      underwater swim-under displace.
- [ ] can_levitate/can_fly consumer sweep beyond goodpos/
      minliquid/mfndpos (NEW 2026-07-31; MEDIUM - upgraded from
      the planned LOW after the battery produced a live kill).
      steed.c's dismount drop check reads data-only grounded(mdat)
      where master guards with !can_levitate - battery repro,
      twice: a saddled pony with verified MR2_LEVITATE dismounted
      over water -> "The saddled pony falls into the water! The
      saddled pony is killed!" (the have_spot/ride-onto-water
      path; the converted no-dry-tile fallback is a different
      arm). Also data-only: trap.c pit/hole arms, do.c boulder
      push. Flagged-at-design-time gaps in the same family:
      dismount triple-failure mongone (master steed.c), mhitu eel
      puddle/sewage refinements, goodpos hero-water Breathless
      question comment.
- [ ] Door-material display color unported (NEW, minted 2026-07-30
      by the 2026-07-29 review; MEDIUM port gap). master's
      mapglyph.c colors non-wood doors via
      material_color(door_material()); grep material_color on the
      branch = nothing. All the new door rules (unkickable,
      undiggable, beam-absorbing, shrapnel) hang off a property the
      player cannot see, and lock.c's newsym_force "redraw with the
      new color" comment describes behavior that does not exist.
      DEPENDENCY: belongs with the Phase 4 color rows - carry it
      onto their list when that phase opens.
- [x] SUPPRESS_RIDER farlook hedge unported (NEW, minted 2026-07-30
      by the 2026-07-29 review; MEDIUM port gap). pager.c has the
      Rider "riding its steed" hedge but master's x_monnam
      SUPPRESS_RIDER mechanism is absent, so farlook prints "Death,
      riding its steed" - the steed name leaks which Rider is
      mounted. Re-verify the exact leak shape at fix time.
      LANDED 2026-07-31 as BUG-1419 (commit fd5021875; battery all
      four sub-checks PASS - blind+ESP farlook "Rider, riding its
      steed", '/' prompt says "rider", sighted farlook names Death,
      livelog records "put Death down for a little nap"). PREMISE
      CORRECTION: the leak was backwards - the hedge already hid
      the steed; the Rider's OWN name leaked, plus again via the
      '/' prompt. Flag is 0x100 (0x40 = AUGMENT_IT here); end.c
      needs no opt-out (done_in_by formats via pmname); master's
      "no stats via farlook" hedge guards add_mon_info(), which is
      UNPORTED - it must travel with the monster-stats display
      feature when that ports.
- [x] is_lightblocker_mappear() missing the S_deadtree row (NEW,
      minted 2026-07-30 by the 2026-07-29 review; MEDIUM port gap).
      include/monst.h: the branch macro ends at S_tree where master
      continues with S_deadtree, so a garden/forest mimic disguised
      as a dead tree is see-through while a real dead tree blocks
      sight - a free mimic tell. One line; seemimic() reads the same
      macro, so discovery unblocking is fixed by the same edit.
      LANDED 2026-07-31 as BUG-1417 (commit b80fefbde; by
      inspection - no GARDEN/FOREST room appeared in the battery
      game and the leg was opportunistic by design; consumers
      verified: does_block, mimic_light_blocking, seemimic's
      capture-then-conditional-unblock is symmetric). Verification
      upgraded the stakes: forest mimics take S_deadtree 7-in-8,
      garden 1-in-3 - the dominant disguise, not a corner.
- [x] tipcontainer missing BUG-1284's third arm (NEW, minted
      2026-07-30 by the 2026-07-29 review; MEDIUM port gap).
      pickup.c generic-destination tip branch: no ice-box age-freeze
      + ROT_CORPSE/REVIVE_MON stop_timer on #tip into an ice box,
      while the #loot path (in_container's Icebox arm) has it - a
      troll tipped into an ice box revives inside. The same branch
      also lacks master's obj_resists pre-check on the cursed-mbag
      arm.
      UPDATE 2026-07-30: the obj_resists half landed with BUG-1403
      (branch-shaped condition widening; do_boh_explosion and
      boh_loss got the same missing pre-check). This row now owes
      only the ice-box age-freeze/stop_timer third arm.
      LANDED 2026-07-31 as BUG-1418 (commit 017682b05; battery
      PASS - tipped troll corpse timed=0 with age flipped, #loot
      control unregressed, floor-control troll DID revive, both
      boxed corpses inert 250 turns later). Copied the BRANCH's
      in_container block (ice-troll uncancel + SHRINK_GLOB, beyond
      master's reduced arm); sibling-sweep find: hornoplenty() can
      tip corpses into an ice box too - same block added there
      (mkobj.c). Upstream 5.0 shares the original gap. Row DONE.
- [x] POT_DROW_POISON quaff/breathe arms unported (NEW, minted
      2026-07-30 by the 2026-07-29 review; MEDIUM port gap).
      potion.c peffects() and potionbreathe() have no case, so
      quaffing one lands in impossible("What a funny potion!") - and
      every drow starts with two (u_init). Master's arms gate on
      how_resistant(SLEEP_RES) and scale with resist_reduce
      POISON_RES - port in partial-resistance form per the standing
      rulings.
      LANDED 2026-07-31, scope WIDENED to full drow-poison parity
      by session ruling, two commits: BUG-1420 ba73baa4b (ALL
      potion.c arms - quaff/thrown/vapor/mixtype x2/fungus recipes
      x2 with BUG-731 guards/coating/cross-guards/healing strip;
      battery legs a-h PASS) and BUG-1421 cbb0c86de (taint parity
      sweep across 9 more files: hero-side hmon_hitmon_taint +
      mon-vs-mon + mthrowu arms, mpoisons_subj, "tainted" xname/
      doname/killer/wish display+parse, cancel+poly_obj carry,
      mksobj gen rolls, u_init clear retiring the mislabeled
      zombification TODO; battery PASS incl. gdb-verified abuse
      event). pray.c altar-strip stays with the sacrifice-gift
      feature (oprops deferral list). Battery yielded TWO NEW
      pre-existing findings, minted as rows below (is_poisonable
      width; hand-thrown ammo skips poison/taint).
- [x] searches_for_item() Is_container-range widening (NEW, minted
      2026-07-30 by the 2026-07-29 review; MEDIUM port gap). muse.c:
      monsters now seek and pick up bags of treasure/devouring
      (master whitelists containers there). Add the two-otyp
      exclusion mirroring find_misc()'s MUSE_BAG arm in the same
      file - the widening commit fixed that sibling and missed this
      one.
      LANDED 2026-07-31 as BUG-1416 (commit a3f673272; battery
      PASS - gnome lord on the pile took the sack + a tin, both
      rare bags untouched across ~40 monster-turns). Two in-tree
      TODOs (mon.c mpickstuff, monmove.c goal arm) had
      pre-specified the exclusion; they stay - they gate the
      unported container-cracking port. Pet residual minted as a
      LOW row below.
- [ ] is_poisonable() range narrower than master (NEW, minted
      2026-07-31 by the BUG-1420/1421 battery; MEDIUM port gap,
      affects poison AND taint). Branch include/obj.h keeps
      upstream 5.0's ammo/missile-only form (oc_skill in
      [-P_SHURIKEN, -P_BOW], plus the branch's permapoisoned
      rider); master covers every non-launcher WEAPON_CLASS item
      with oc_dir != WHACK - daggers, spears, swords. On the
      branch, dipping an elven dagger into drow poison OR sickness
      prints "Interesting..." and coats nothing, and wishes for
      poisoned/tainted daggers silently drop the adjective
      (readobjnam gates on the same macro). Widening changes
      generation too (mksobj poison/taint rolls and makemon drow
      producers gate on it) - audit consumers before flipping.
- [ ] Hand-thrown ammo never applies poison or taint (NEW, minted
      2026-07-31 by the BUG-1421 battery; MEDIUM port gap,
      upstream-shared). hmon_hitmon_weapon() routes is_ammo &&
      (thrown != HMON_THROWN || !ammo_and_launcher) to
      hmon_hitmon_weapon_ranged(), which never sets
      hmd->ispoisoned/istainted - both flags are set only in the
      melee helper's thrown-ammo tail, which such throws never
      reach. A poisoned arrow thrown by hand hits with no poison;
      battery-observed with a tainted elven arrow. Thrown darts
      are is_missile, take the melee helper, and work. Routing and
      flag placement are byte-identical in NetHack50 - an upstream
      5.0 regression vs 3.6 (master's monolithic hmon_hitmon
      applies poison on every WEAPON_CLASS hit); also entered in
      the Upstream-reportable table.
- [ ] Pets still fetch the two rare bags (NEW, minted 2026-07-31
      with BUG-1416; LOW, master-shared). dogmove.c dog_invent()
      picks up floor items via the nofetch class filter and never
      consults searches_for_item(), so a pet will carry an
      uncursed bag of treasure/devouring after BUG-1416. Master
      has the same shape; fix is a beyond-master decision (add the
      two otyps to dog_invent's exclusions if wanted).
- [x] mhitm.c res[i] clobber in both mattackm miss arms (NEW, minted
      2026-07-30 by the 2026-07-29 review; MEDIUM regression).
      res[i] = M_ATTK_DEF_DIED overwrites a just-set M_ATTK_AGR_DIED
      when missmm() kills both parties (the hated-blocker sear kills
      magr; the FQ_INFERIOR glove break's mselftouch petrifies a
      cockatrice-wielding mdef) -> passivemm()/monkilled() on a dead
      attacker. Fix shape: test mdef first with =, then magr with
      |= - four sibling sites in the same function show it. BATCH:
      one mhitm/steed-guard slice with the next two rows. LANDED
      SAME DAY as BUG-1404 (commit ade89646c; battery melee smoke
      PASS - every kill announced exactly once through ~90 message
      pages, paniclog clean): exactly the minted fix shape, both
      arms; the dual-death miss itself is not deterministically
      stageable, so that corner stands by inspection against the
      four sibling sites (master uses |= for both flags - branch
      regression confirmed).
- [x] mhitm.c pre-hit armor-break blocks leave strike == 1 (NEW,
      minted 2026-07-30 by the 2026-07-29 review; MEDIUM regression;
      2 sites). When break_glass_obj kills mdef before hitmm() runs,
      passivemm() still receives strike = 1 and applies passive
      damage for a blow that never landed. Set strike = 0 before the
      break. BATCH: rides with the res[i] row above. LANDED SAME
      DAY as BUG-1405 (commit cef59e66a; same battery): strike
      zeroed INSIDE the DEADMONSTER break blocks rather than before
      the break (the break itself is not the miss - only a fatal
      one is); passivemm(mhitb=FALSE) verified to give ordinary
      miss semantics with the passed-in DEF_DIED flag merging into
      its return unchanged.
- [x] separate_steed_and_rider() callers missing the DEADMONSTER/
      relocation guard (NEW, minted 2026-07-30 by the 2026-07-29
      review; MEDIUM regression; five call sites). The same commit
      added the guard at its dothrow.c and newcham siblings but not
      at: the web_unseats_rider() callers (uhitm.c x3, mthrowu.c
      x1), tamedog() (dog.c), gulpmm() (mhitm.c), and
      trapeffect_poly_trap() (trap.c). rloc_to at the destination
      runs mintrap(), which can kill or migrate the rider
      mid-function. Consider having web_unseats_rider() return
      still-alive-and-here so its callers share one guard; pin the
      exact site count at fix time. BATCH: rides with the two mhitm
      rows above. LANDED SAME DAY as BUG-1406 (commit 380d42ced;
      battery: all three web paths live with rider+steed
      gdb-verified alive/separate/on-map, taming
      dismount-and-tame, three poly-trap unseats clean, paniclog
      absent throughout). Count PINNED at fix time: FOUR direct
      call sites (web_unseats_rider() itself, tamedog, gulpmm,
      trapeffect_poly_trap) yielding SIX guarded continuations -
      the webbing risk fans through web_unseats_rider's callers
      into hmon_hitmon_misc_obj (already_killed) and both
      mhitm_ad_webs arms (DEF_DIED + done); tamedog returns FALSE;
      gulpmm returns before the display swap; poly_trap returns
      Trap_Killed_Mon/Trap_Moved_Mon. mthrowu.c's caller needed NO
      change (its only later use is already death-guarded); zap.c's
      bhitm arm verified already guarded; the boolean-return idea
      DECLINED (each caller needs a different reaction).
      Master-inherited at tamedog/gulpmm/poly-trap/AD_WEBS
      (frozen-unfixed, archived); web/hmon path branch-side.
      Battery caveat: all three poly-trap triggers routed through
      newcham's own guard arm (every poly result was unridable), so
      trap.c's new arm stands by inspection against that sibling.
- [x] deltrap_with_ammo() unconditional reset_utrap(TRUE) (NEW,
      minted 2026-07-30 by the 2026-07-29 review; MEDIUM
      regression). trap.c: it defeats delfloortrap()'s TT_BURIEDBALL
      carve-out - a Punished hero is silently freed from a buried
      iron ball when the trap square floods or is dug away. Guard on
      utraptype != TT_BURIEDBALL or pass the disposition through.
      LANDED 2026-07-31 as BUG-1422 (commit a2542f9ed; battery:
      staged tether survived trap deletion, beartrap control still
      reset; trigger lore - own-square #untrap refuses while
      trapped, "room floor" terrain wish is the direct path).
      RECLASSIFIED master-inherited: master's deltrap_with_ammo has
      the identical unconditional reset (frozen-unfixed, archived);
      upstream has no deltrap_with_ammo but its cnv_trap_obj carries
      the same defeat - upstream-reportable row added.
- [x] sink_into_lava() thirds BOTH HP pools for a polymorphed hero
      (NEW, minted 2026-07-30 by the 2026-07-29 review; MEDIUM
      regression). trap.c: make the u.uhp / u.mh reduction
      exclusive, like potionbreathe's sickness arm.
      LANDED 2026-07-31 as BUG-1423 (commit db9c25f88; battery:
      polymorphed giant at 40%% fire res, u.mh 33->11->4->0 across
      three sinking turns with u.uhp pinned at 228; staging lore -
      the TT_LAVA tether is only reachable at 100%% fire res, grant
      then drop). RECLASSIFIED master-inherited (identical hunk,
      frozen-unfixed); upstream reduces only u.uhp = the wrong pool
      when poly'd - upstream-reportable row added. Two wrong-pool
      SIBLINGS surfaced (reviewer + battery), minted as the
      wrong-pool-under-Upolyd row below.
- [x] rehumanize() missing the See_underwater transition redraw
      (NEW, minted 2026-07-30 by the 2026-07-29 review; MEDIUM
      regression). polyself.c: a polyform swimmer whose form expires
      while submerged keeps the full map + stale glyphs painted
      forever. The goggles slice instrumented Blindf_on/off with the
      old_uwvis compare; rehumanize() is the one uninstrumented
      form-change door. (The reverse direction needs nothing -
      allmain's arm-2 docrt covers it.)
      LANDED 2026-07-31 as BUG-1424 (commit a9f87676b; battery:
      submerged eel form, forced expiry, painted map collapsed to
      the hero glyph the moment the rehumanize message flushed;
      surfacing repainted). SCOPE WIDENED by the reviewer sweep:
      slimed_to_death()'s direct polymon (timeout.c) was a second
      live door, fixed same commit by inspection; all other direct
      polymon callers cannot flip See_underwater. "One
      uninstrumented door" premise corrected to two. RECLASSIFIED
      master-inherited (rehumanize identically bare there).
- [x] set_trap() can install an invalid ammo material (NEW, minted
      2026-07-30 by the 2026-07-29 review; MEDIUM regression).
      apply.c: crafted_ammo_trap_materials[] includes ADAMANTINE but
      the ammo otyps fall to metal_materials[] without it; wish
      "adamantine arrow trap", apply -> set_material impossible(),
      and the invalid material follows the ammo through the new
      trap-ammo sanity walk. Validate against the ammo otyp
      (fallback objects[].oc_material) or drop ADAMANTINE from the
      crafted list.
      LANDED 2026-07-31 as BUG-1425 (commit 978752639; battery:
      wished adamantine arrow trap deployed IRON arrows with no
      impossible; crafted adamantine dwarvish-spear kit round-
      tripped keeping ADAMANTINE). PREMISE REFINED: the array lives
      in mkobj.c, and the drop-ADAMANTINE option would have broken
      the legitimate adamantine dwarvish-spear kit (dwarvish lists
      roll {2, ADAMANTINE}); the deploy-side validation was the
      only correct arm. Branch-only (the kit transmutation flow is
      the OBJ_INTRAP port).
- [x] attrcurse() contract + gremlin farming (NEW, minted 2026-07-30
      by the 2026-07-29 review; MEDIUM regression). sit.c + the
      uhitm.c mon_give_prop consumer: the six converted resistance
      cases return the property even when nothing was stripped, and
      decr_resistance() never clears a spent HAVEPARTIAL - so a
      gremlin repeatedly gains real resistances the hero no longer
      has (1-in-10 per hit). Fix: before/after how_resistant compare
      for the return; consider clearing HAVEPARTIAL at 0 in
      decr_resistance so the binary macros stop reporting the
      phantom. BATCH: one partial-resistance-consumers slice with
      the next row. LANDED SAME DAY as BUG-1407 (commit 3d94580ad;
      battery: first proc stripped 40%% fire res and gifted exactly
      one gremlin, word gdb-verified 0x10000028 -> 0x00000000 with
      the marker gone; 45 later procs zero repeats/zero re-gifts;
      100->30 decrement kept the marker; paniclog absent): both
      suggested halves implemented. EXTRA VECTOR closed, found at
      fix time: a crowned hero (FROMOUTSIDE pinned at 100, mask
      preserves it) was an infinite free donor - the no-op strip
      still returned the property; the before/after compare closes
      it (44-proc donor leg, word intact, nothing gifted).
      Classification split: decr + gates master-inherited
      (frozen-unfixed); the mon_give_prop farming consumer is
      upstream-5.0/branch-side (master discards attrcurse's
      return).
- [x] Decision layers read binary macros under partial resistance
      (NEW, minted 2026-07-30 by the 2026-07-29 review; MEDIUM
      regression). immune_to_trap()'s Sleep/Fire arms (trap.c) and
      getmattk()'s cold arm (mhitu.c) classify 5% resistance as
      immunity (suppressed trap confirmations; a lich's AT_TUCH
      converted to AD_PHYS, which can be MORE damage). Intended form
      per the partial-resistance rulings: how_resistant(X) == 100.
      BATCH: rides with the attrcurse row above. LANDED SAME DAY as
      BUG-1408 (commit bd45c47a6; battery: lich vs 50%% cold res
      landed real frost 11x where pre-fix substituted physical, vs
      100%% still converted - 9 touches zero frost; all four
      trap-prompt A/B legs unchanged; paniclog absent): all three
      sites converted to the ruled how_resistant form. PREMISE
      CORRECTION recorded: the "suppressed trap confirmations"
      claim was WRONG - immune_to_trap()'s only consumer (hack.c
      ParanoidTrap prompt) skips asking solely for
      TRAP_CLEARLY_IMMUNE, so the trap-arm half was INERT; fixed
      per the ruling anyway (enum semantics, /* revisit */ extern,
      future consumers). The lich cold arm was the live half
      (upstream-5.0 code absent from master; a 95%% hero took ~10x
      the intended damage). Both halves branch-only.
- [x] Migrated ridden steed arrives at <0,0> (NEW, minted 2026-07-30
      by the 2026-07-29 review; MEDIUM; master-inherited, 5.0's
      per-turn sanity makes it loud). dog.c mon_arrive: the
      ridden_by early-return runs before coordinates are restored,
      so the steed sits at <0,0> - per-turn sanity impossible()
      until the rider moves, and if the rider dies on its arrival
      turn the steed is place_monster'd into level.monsters[0][0],
      the vault-guard parking slot. Fix: update_monsteed on the
      rider's arrival or set steed coords in the early-return.
      LANDED 2026-07-31 as BUG-1426 (commit ef21f70ac; battery:
      genesis'd mounted drow cleric fell through a trapdoor
      barrier, arrival steed coords exactly equaled the rider's,
      monsters[0][0] NULL, per-turn sync tracked three rider moves,
      rider #wizkill placed the steed normally; paniclog never
      existed). FIX SHAPE HARDENED by the reviewer: the first-draft
      failed-arrival routing (steed -> failed_arrivals) would have
      double-migrated via the rider's limbo steed recursion =
      relmon panic, and a cross-list rider/steed pair cannot
      survive restmonsteeds' fmon-only relink - final shape leaves
      an unplaced rider's steed on fmon for the recursion to carry
      (pair always one list), plus a defense-in-depth
      migrating/limbo guard in update_monsteed. Rider-death arm =
      the BUG-1259 orphan signature; noted on that investigation
      as a possible Track 2 producer.
- [ ] Wrong HP pool consulted under polymorph, non-lava sites (NEW,
      minted 2026-07-31 from the BUG-1423 fix round; MEDIUM;
      pre-existing, one row for the family). Two confirmed sites
      read/write the dormant u.uhp while Upolyd: (1) muse.c
      power-word-kill youdefend branch - the kill-zone gate
      (u.uhp <= threshold), BOTH damage clamps (dmg = u.uhp - 1,
      whose losehp then charges u.mh), and the resisted arm's
      setuhpmax + u.uhp /= 3 (reviewer find); (2) trap.c
      lava_effects boil-away arm - the u.uhp > 1 gate and the
      u.uhp / 2 amount feed a losehp that charges u.mh (battery
      find; the !boil_away flat-1 path behaved). Intended shape
      per BUG-1423: gate/clamp/reduce on Upolyd ? u.mh : u.uhp,
      potionbreathe sickness form. Sweep the rest of the u.uhp
      readers in decision code before fixing - these two came from
      one review pass, the family is likely wider.
- [x] REVIEW-LOW (2026-07-29): #undef EYEWEAR missing at the
      objects.h macro-cleanup block - upstream has it. (This and the
      REVIEW-LOW rows below were minted 2026-07-30 as one batch; fix
      opportunistically.) LANDED 2026-07-31 in the batch cleanup
      commit (BUG-1396 oversight; hygiene only).
- [x] REVIEW-LOW (2026-07-29): obj_typename() GEM_CLASS lost
      master's SLING_BULLET " stone" carve-out (xname has it);
      master also has erosion_matters/doname SLING_BULLET arms the
      branch lacks. One row for the family. LANDED 2026-07-31
      BUG-1427 (obj_typename disjunct live; doname arm restored but
      inert until D4 widens erosion_matters - noted on D4; the
      erosion_matters half was already superseded by D4).
- [x] REVIEW-LOW (2026-07-29): stale comments - polyself.c eyewear
      list missing goggles; mon.c sanity colocation-exemption arms
      are dead code (the ridden_by continue precedes them);
      designate_altshield "at most one" overclaim (master parity -
      the bit survives outside invent). LANDED 2026-07-31 in the
      batch cleanup commit; PREMISE CORRECTED on the mon.c arms:
      dead because a mounted steed is off the map and the exemption
      tests the inverse relation, NOT because of the continue -
      comment now says so; predicates untouched (master parity).
- [x] REVIEW-LOW (2026-07-29): mktrap_victim() SPEAR_TRAP_SET arm is
      dead code (mklev.c kind < HOLE gate). RECORD-ONLY 2026-07-31:
      comment added marking the arm unreachable and kept; widening
      the gate is pointless today (spear traps generate at lvl>=6,
      victims only at lvl<=rnd(4)).
- [x] REVIEW-LOW (2026-07-29): immune_to_trap() vs
      trapeffect_ice_trap() contradiction - cold-resistant monsters
      route around the trap whose live effect is eroding their cold
      resistance. Decide which is authoritative. RULED + LANDED
      2026-07-31 BUG-1439: erosion is authoritative; the live
      router was m_harmless_trap() (immune_to_trap's only consumer
      is the hero prompt) and its ice arm now exempts only
      unstrippable resistance.
- [x] REVIEW-LOW (2026-07-29): ICE_TRAP/MAGIC_BEAM generation gate
      is one-sided - traptype_rnd blocks them, rndtrap (lua
      trap("random")) still rolls both. LANDED 2026-07-31 BUG-1429,
      PREMISE CORRECTED twice: lua trap("random") routes through
      traptype_rnd correctly - the leaker is rndtrap()/maze filler;
      MAGIC_BEAM is depth-gated, only ICE was blocked. Fix also
      excluded TRAPPED_DOOR/CHEST (silent ~7% maze density hole,
      upstream-shared - upstream table row added) and restored
      BUG-539's retry cap the port dropped.
- [x] REVIEW-LOW (2026-07-29): mb_trapped() infers death from
      m_at(mx, my) != mtmp - false when the monster merely moved
      (vamprise rloc, minliquid) (monmove.c). LANDED 2026-07-31
      BUG-1430 (liveness test at both arms; relocated survivor no
      longer forfeits its turn; mdig_tunnel redraw snapshot).
- [x] REVIEW-LOW (2026-07-29): lock.c - the stone-to-flesh
      trapped-door arm calls mb_trapped() before doormask =
      D_NODOOR, breaking the invariant its five siblings document;
      and picklock's non-destroy path forces D_CLOSED, silently
      losing a "lock the door" action. LANDED 2026-07-31 BUG-1431
      (state hoist; flesh door no longer throws x3 stone shrapnel,
      "unharmed" contradiction gone) + BUG-1432 (lock/unlock toggle
      preserved; the temple_at_boundary billing half belongs to the
      existing temple-slice row - helper unported).
- [x] REVIEW-LOW (2026-07-29): disarm_spear_trap()'s trailing newsym
      is derived from u.dx/u.dy - redundant (deltrap_with_ammo does
      its own) and coordinate-fragile. LANDED 2026-07-31 in the
      batch cleanup commit (line deleted; disarm_squeaky_board's
      sibling newsym IS load-bearing and stays).
- [x] REVIEW-LOW (2026-07-29): dat/symbols AmigaFont set is missing
      the 5 new terrain symbols - consistent with its older gaps;
      one cosmetic row. LANDED 2026-07-31 BUG-1441. Ruling asked
      for MACgraphics/curses too; premise corrected: both are
      internally consistent as they stand (curses omits the whole
      water family together; MACgraphics fallbacks match its floor)
      - left alone deliberately.
- [x] REVIEW-LOW (2026-07-29): morguemon()/courtmon() zero-range
      guards unswept (mkroom.c) - the commit message claimed the
      family swept; 3 of master's 5 adopted. LANDED 2026-07-31
      BUG-1428 (both max() wraps + the pick_room/mkswamp nroom
      guards - BUG-658 completion). Verification ALSO FOUND
      morguemon() is the vanilla 5.0 body - EvilHack content row
      minted below.
- [x] REVIEW-LOW (2026-07-29): mcalcdistress()'s hand-rolled loop
      reads mtmp->nmon after the callback (mon.c; iter_mons
      pre-captures; latent - no live relmon path today). LANDED
      2026-07-31 with BUG-1434: the ridden-steed exemption moved
      into the helpers, so mcalcdistress reverted to iter_mons()
      and inherits its pre-capture.
- [x] REVIEW-LOW (2026-07-29): bubble pickup can spuriously unstuck
      a steed grabbing the hero (mkmaze.c: mx = 0 then
      update_monsteed -> !m_next2u -> unstuck). LANDED 2026-07-31
      BUG-1435 (isok gate on the release arm; rloc_to_core re-tests
      after placement).
- [x] REVIEW-LOW (2026-07-29): mattacku() hittmp leak when the
      weapon dies mid-attack (mhitu.c "KMH" subtraction gated on
      mon_currwep; master parity, but rust-destruction now makes it
      reachable). Key it on a had-weapon boolean. LANDED 2026-07-31
      BUG-1436 (had_wep; leak was branch-specific - master's gate
      dangles instead of nulling).
- [x] REVIEW-LOW (2026-07-29): steed_off_map's MON_OFFMAP excludes
      ridden steeds from 6 other iter_mons consumers
      (set_mon_lastmove, maybe_turn_mon_iter,
      deadbook_pacify_undead, garlic_breath, reset_hostility,
      remove_mon_from_bones) - mostly cosmetic; a comment or an
      iter_mons-level exemption. RULED helper-level + LANDED
      2026-07-31 BUG-1434 (all ~25 consumers audited;
      remove_mon_from_bones was a false alarm - rider's erid block
      handles the steed).
- [x] REVIEW-LOW (2026-07-29): separate_steed_and_rider() can place
      the steed twice when the rider dies inside rloc_to (steed.c -
      three independent ifs should be an if/else-if chain;
      master-inherited). LANDED 2026-07-31 BUG-1437 (else-if chain;
      the "placing X over itself" impossible retired).
- [x] REVIEW-LOW (2026-07-29): see_monsters()'s widened skip drops
      bubble passengers from the warning-glow census (display.c;
      cosmetic; the old PARKEDMONSTER term had the same property).
      COMMENT-ONLY 2026-07-31 (batch cleanup commit); premise note:
      PARKEDMONSTER covered only the vault guard - the census loss
      is new vs both trees but is a sub-turn Plane-of-Water flicker
      that self-corrects; a real fix would split a hot redraw loop.
- [x] REVIEW-LOW (2026-07-29): b6a2f9576's commit message overstates
      coverage - dogmove.c pet-ignite arms and read.c
      genocide-throne (Vecna/Vlad/Tal'Gath) arms are NOT ported.
      Recorded so a later census does not treat them as done.
      RESOLVED 2026-07-31: dogmove pet-unslime PORTED as BUG-1440
      (session ruling); read.c genocide-throne minted below as a
      data-port row (no code existed to convert - the no_geno_*
      macros and G_VECNA/G_VLAD/G_TALGATH geno bits are absent
      tree-wide; u.uevent bits exist unconsumed).
- [x] REVIEW-LOW (2026-07-29): resist_conflict()'s Ithilmar
      always-resist arm is unreachable at high CHA (mondata.c
      else-if off the clamp; master parity). LANDED 2026-07-31
      BUG-1438 (artifact test hoisted above the clamp; battery
      10/10 resist with barding vs 1/10 control).
- [x] REVIEW-LOW (2026-07-29): RECORD ONLY - EDITLEVEL gap window
      df04f38c6..403245844 accepted dev saves unguarded (later bumps
      closed it); 403245844's commit-message claim that struct rm
      growth changes critical_sizes is wrong (the 5 bits fit the
      existing bitfield unit). CONFIRMED CLOSED 2026-07-31: bumps
      1/2/3 landed in the next Phase-2 commits, HEAD at EDITLEVEL 3;
      nothing to do.
- [x] REVIEW-LOW (2026-07-29): dothrow.c dropy() return discarded
      near hurtle (pre-existing; master captures it). RECORD-ONLY
      2026-07-31, premise corrected: branch dropy() is void (5.0
      signatures), so the real defect is toss_up()'s hard-coded
      TRUE return vs its "FALSE if the object is gone" header -
      master captured a boolean dropy() exactly there and the port
      lost it. INERT today: toss_up's sole caller void-discards and
      obj is never dereferenced after. Revisit if toss_up's return
      gains a consumer.
- [x] REVIEW-LOW (2026-07-29): hack.c domove switch default clears
      mpeaceful without set_malign/newsym (pre-existing, shared with
      upstream; the BUG-945 shape). LANDED 2026-07-31 BUG-1433,
      premise corrected: the site is spoteffects(), and it is a
      PORT REGRESSION - master has set_malign + newsym (BUG-478's
      fix) + an mcanmove gate on the surprise-attack line; all
      three restored.
- [x] Wish feedback prints the pre-merge count (NEW, minted
      2026-07-30 by the BUG-1400 battery; cosmetic). Wishing "15
      daggers" while already carrying one prints "b - 15 +0 daggers"
      though the merged stack is 16 (inventory display and gdb agree
      on 16). The wish-delivery pline reads the wished object's quan
      before/without the merge. Check master for the same behavior
      before treating it as a port bug. CHECKED 2026-07-31: NOT a
      port bug - master prints the identical pre-merge count and no
      total at all; the branch matches upstream 5.0 and adds
      "(16 in total)." under verbose (default on). The reporter's
      bare string implies verbose:false. Nothing to do.
- [ ] morguemon() EvilHack content unported (NEW, minted 2026-07-31
      by the BUG-1428 verification; MEDIUM port gap). The branch
      carries the vanilla 5.0 body; master's EvilHack content is
      entirely absent: skeletal-dragon rare top tier (hd > 10,
      i > 97, G_GONE-gated), vampire/skeleton-warrior arms
      (hd > 8, i > 85), In_vecna_branch shade arm and wraith
      weighting (v = 22 vs 40), and the zombie/skeleton class
      split. FEATURES_ARCHIVE documents the feature. Port with the
      Vecna-branch slice or as its own content slice; check
      In_vecna_branch availability first.
- [ ] read.c genocide-throne arms + geno-bit data port (NEW, minted
      2026-07-31 from the b6a2f9576 overstatement row; content/data
      port). Master's do_genocide() has three ~28-line refusal arms
      (Vecna/liches, Vlad/vampires, Tal'Gath/beholders) keyed on
      no_geno_vecna/vlad/talgath macros over G_VECNA/G_VLAD/
      G_TALGATH geno bits - none of which exist on the branch; the
      u.uevent.uvecna/uvlad/utalgath consumers-to-be are already
      set on boss death. Prerequisite: port the geno bits and their
      per-monster assignments in monsters.h, plus master's pager.c
      lookup line. The b6a2f9576 commit message wrongly claims this
      was converted - there was nothing to convert.
- [x] elemental_clog() Astral no-candidate arm is a total no-op (NEW,
      minted 2026-07-31 by the BUG-1442 verification; needs a design
      ruling; upstream-shared). DONE 2026-07-31 as BUG-1444 (commit
      7fc3b5a86; ruling: m_into_limbo - the game's standard no-room
      fallback, sanity-legal, save/restore-safe, ERID-recursive so a
      mounted displaced rider carries its steed; the zm bucket removed
      in the same commit; battery: mstate 0xd on migrating_mons, 31
      sanity-checked turns clean, paniclog never existed; landed
      together with the REVIEW-HIGH pair-exclusion row below, whose
      fix widens this arm's no-candidate window). Original finding:
      On Astral with no obliteration
      candidate neither clog arm runs, but mnearto() has ALREADY
      stripped its displaced monster to <0,0> with MON_OFFMAP - its
      own deal_with_overcrowding comment says the call exists to
      prevent exactly the ensuing sanity failure, and on Astral it
      prevents nothing. Permanent <0,0> monster, isok-arm sanity
      complaint signature (distinct from BUG-1442's floor-mon one).
      Ruling needed: obliterate despite the ok_to_obliterate filters,
      m_into_limbo, or accept-and-exempt. Related cosmetic note:
      elemental_clog's zm candidate bucket is declared and never
      assigned in master, branch and upstream alike - dead weight,
      fold its removal into whatever fix lands here.
- [x] Re-run the review on the partial-intrinsics (b6a2f9576) and
      steed/riding clusters with the reviewer agent inheriting the
      session model (NEW, minted 2026-07-30; the original 2026-07-29
      pass reviewed those clusters on Opus via the old frontmatter
      pin). DONE 2026-07-31 EVENING: two agents, both clusters
      CONDITIONAL PASS - the fixed/deferred exclusion lists held (no
      re-reports); 1 HIGH + 4 MEDIUM + 4 LOW NEW findings, every one
      re-verified in the main context against the actual bodies and
      minted as the rows below. Core lifecycles reported tight (ERID
      alloc/free, migration pairing, bones separation, HAVEPARTIAL
      save/lifecycle, no double-scaling on the resistance graph).
- [x] REVIEW-HIGH (2026-07-31): elemental_clog() can pick one half
      of a mounted pair as its obliterate victim (mon.c victim scan
      filters neither has_erid nor ridden_by; ok_to_obliterate
      excludes only the Rider ITSELF, not its horse; Death/Famine/
      Pestilence always spawn mounted and Astral has no elementals,
      so a Rider's horse is a prime m3 pick). mongone(horse) ->
      separation re-places the RIDER on the shared square, then the
      snapshotted-coords rloc_to() stomps that square: "placing X
      over Y" impossible + level.monsters overwrite, Rider left as a
      phantom (per-turn "is not there!", his next move deletes the
      wrong map entry). MASTER-INHERITED (identical sequence).
      Fix shape: exclude has_erid/ridden_by candidates in the scan,
      or re-check MON_AT after mongone and fall back to rloc().
      LAND TOGETHER with the elemental_clog Astral no-op row below -
      both touch the victim scan, and excluding pairs makes the
      no-candidate arms more reachable. DONE 2026-07-31 as BUG-1443
      (commit d7bd8421e; the scan-exclusion shape, in
      ok_to_obliterate itself so every bucket is covered; battery
      counterfactual: 7 pair halves obliterate-eligible pre-fix on
      one live Astral level - FIVE mounted pairs, so ordinary
      mounted knights hit the path, not just the Riders - with a
      horse at m_lev=7 as the pre-fix m3 winner; post-fix scan
      empty, pairs intact and moving as units 31 turns on).
- [x] REVIEW-MEDIUM (2026-07-31): domove_swap_with_pet() lost the
      steed drag (hack.c raw remove_monster+place_monster with no
      update_monsteed; PORT REGRESSION - master routes the swap
      through rloc_to whose core calls update_monsteed; the correct
      branch sibling is dokick.c's "a rider drags its steed along").
      A charmed mounted Knight (tamedog sets peaceful then bails
      before separation for humans) swap-displaced then killed
      places its steed under the hero ("you over monster" sanity).
      Fix: update_monsteed(mtmp) after the place; same one-liner at
      the displacer-beast swap sibling for symmetry. DONE 2026-07-31
      as BUG-1445 (commit de333b251; verification UPGRADED the
      mdisplacem sibling from symmetry to a LIVE master-inherited
      path - the Riders are M3_DISPLACES and always mounted, so
      only the mounted-magr side needed the call, mdef is has_erid-
      filtered at entry; battery proved both drags with breakpoints
      BEFORE the per-turn monmove update_monsteed confound).
- [x] REVIEW-MEDIUM (2026-07-31): slippery_ice_fumbling() binary
      resists_cold gate makes any 1% partial grant total slip
      immunity - the graded how_resistant>50 ? 3 : 2 roll ported
      for partials is dead code for the hero (hack.c; BRANCH-ONLY:
      master's hero-side resists_cold reads form bits only, so
      master heroes with eaten resistance still slip). Also mixed
      actors when mounted (gate reads the steed, graded arm reads
      the rider). Fix: hero leg gates on how_resistant(COLD_RES)
      == 100, keeping 1-99 in the graded roll. DONE 2026-07-31 as
      BUG-1446 (commit c4544ad9b; graded roll additionally made
      unmounted-only - the mounted mixed-actor half; measured
      rates 0%~1/2, 40%~1/2 [pre-fix NEVER], 60%~1/3, 100/
      FROMOUTSIDE never, mounted flat 1/2 with the steed's binary
      gate ruling; 200-call gdb loops per case).
- [x] REVIEW-MEDIUM (2026-07-31): the lava double-lifesave
      countermeasure is defeated for 1-99% fire resistance (trap.c
      burncount==2 arm: `!Fire_resistance` is binary, so a partial
      hero gets no 5-turn escape grant and re-burns every move;
      cluster-created window - vanilla's gate was exhaustive).
      NEEDS A DESIGN RULING before fixing: a naive <100 gate would
      let set_itimeout() write 5 into a HAVEPARTIAL word and
      silently rewrite a partial percentage (TIMEOUT bits are
      dual-purpose); options = grant only when the word is empty,
      top the partial to 100 via incr_resistance for the wizard/
      explore corner, or a different escape mechanism. DONE
      2026-07-31 as BUG-1447 by RULING (commit e70c635bb; ruling
      = grant only when the word is free; the skip for partials is
      INTENDED - the unconditional Wwalking grant is their escape,
      scaled burn on the walk-out verified live; gate rewritten
      word-safe with the constraint comment, ZERO behavior delta
      proven across all 7 word states; the battery demonstrated
      the counterfactual corruption 0x10000028 -> 0x10000005 that
      the conjunct prevents, and found the arm UNREACHABLE on a
      normal level - safe_teleds succeeds first try - so the arm
      was exercised by direct statement execution).
- [x] REVIEW-MEDIUM (2026-07-31): vulnerable_to() hero leg reads
      binary resistance, so any 1% partial suppresses form
      vulnerability, and the suppression is acquisition-order
      dependent (partial held at poly time kills the FROMFORM
      vulnerability bit via PROPSET; partial gained after keeps it
      -> identical state differs by 50 resistance points; master is
      consistent - partials never suppress). FIX CAVEAT: converting
      the hero leg to how_resistant()==100 recurses (how_resistant
      -> Vulnerable_* -> vulnerable_to); the override must read the
      raw pre-vulnerability layers directly. DONE 2026-07-31 as
      BUG-1448 (commit c7e00ecf8; u_full_resist_raw() raw-layer
      helper per the caveat; battery proved both acquisition
      orders agree, the 99/100 boundary, and the monster leg
      unchanged across 12 live monsters; NEW pre-existing sibling
      minted as the row below).
- [x] REVIEW-LOW (2026-07-31, minted by the BUG-1448 battery;
      pre-existing, untouched by the fix): the Vulnerable_* MACRO
      word arm bypasses vulnerable_to()'s resistance override -
      Vulnerable_cold is (HVulnerable_cold || EVulnerable_cold ||
      vulnerable_to(...)), and polyself's PROPSET stamps the
      VULN_* word's FROMFORM bit from vulnerable_to at POLY TIME,
      so a hero who becomes fully resistant AFTER polymorph keeps
      how_resistant() capped at 50 (macro word arm TRUE) while
      vulnerable_to() itself reports not-vulnerable - the BUG-1448
      order-dependence shape one layer up. Measured: red-dragon
      form, word set post-poly to 100%-partial/FROMOUTSIDE/timed-
      full all read vulnerable_to=0 yet how_resistant(COLD_RES)=50.
      Fix shape to evaluate: make the macro's H-word arm defer to
      the same raw-resistance override, or re-run PROPSET when
      resistance layers change; check master's macro form first
      (the VULN_* words carry external vulnerability sources -
      curses, ice traps - which must keep working). DONE 2026-07-31
      as BUG-1449 (commit f0b696345): the narrow shape - mask
      FROMFORM alone out of the word leg; the whole-word defer was
      REJECTED because the ice trap stamps its timed vulnerability
      precisely on full-innate-resistance holders, so deferring
      would no-op it (the sole external writer - there are no curse
      writers on the branch). The PROPSET stamp stays for
      from_what() attribution. Master shares the macro shape and
      the staleness, frozen-unfixed. Battery: all three post-poly
      full-resistance word states read 100 (pre-fix 50), partials
      still never suppress, and the FROMRACE + timed-VULN external
      case still reads 50.
- [x] REVIEW-LOW batch (2026-07-31), fix opportunistically:
      (a) incr_resistance() folds a live timed full-strength grant
      (the lava-escape 5-turn HFire_resistance, #wizintrinsic) into
      a permanent small partial when a resistance corpse is eaten
      during it - wizmode/explore reach only; (b) doicetrap() else
      arm's pre-mask deletes a FROMFORM cold resistance instead of
      routing it to the vulnerability arm, and "alarmingly cooler"
      prints for a FROMOUTSIDE holder pinned at 100 - MASTER-SHARED
      verbatim (note: the branch already fixed master's dead
      num=rn2(3) clobber there); (c) mount_monster()'s covetous/
      G_GONE rejection path leaks the freshly-created mount to roam
      the level - master-shared; (d) replmon() transfers no erid
      cross-links - currently unreachable (sole caller passes fresh
      MM_NOSTEED monsters), defensive note only. DONE 2026-07-31,
      all four: (a) BUG-1450 69283d87f - both incr AND decr made
      HAVEPARTIAL-aware (the decr side could even PROMOTE a 200-turn
      grant into a permanent >=100 partial, worse than the row
      knew); (b) BUG-1451 43b842422 - FROMFORM joins the innate arm
      per the 82af6dfab unstrippable ruling, message gated on a
      real how_resistant() drop; (c) BUG-1452 e5ea1ae4f - unmakemon
      on the denied path, born unwound (extinct steed = the live
      vector; no steed type is covetous); (d) record-only per the
      row - replmon invariant comment, commit 784fc50d3. Battery
      4/4 legs PASS (details in the archive entries).
- [ ] Murder mechanic; Croesus AD_GOLD; seduction attacks table
- [ ] describe_level branch-aware status naming, REMAINING ARMS
      (minted 2026-08-02 by the mines slice; Decisions log ruling).
      Master src/botl.c describe_level() names every branch; the
      mines arms + minetn_level plumbing landed with the mines C
      payload. Still to port, each gated on its branch predicate
      existing: Soko:%d DONE (gehennom slice 2026-08-05,
      opportunistic), Vlad:%d DONE (gehennom slice, branch spelling
      In_tower), WizTwr:%d DONE (yendor slice 2026-08-05, dnum arm +
      addbranch split in the mines shape), Hell:%d(%d) dunlev+depth
      pair DONE (gehennom slice, BUG-1293-fixed form, prose
      "Gehennom N"; the Gehennom-chain block comment + ordering
      landed with it), Goblin:%d (In_goblintown), IceQ:%d
      (In_icequeen_branch), Vecna:%d (In_vecna_branch), Hidden:%d
      (In_hdgn), Purg:%d (In_purgatory). Master's arm order matters:
      the Gehennom-chain sub-branches (Vlad/Vecna/Hidden/WizTwr/
      Purg) must be tested BEFORE the generic In_hell arm since they
      all carry hellish=TRUE (master's own comment; the comment +
      Vlad/WizTwr/In_hell ordering are IN PLACE - the three later
      arms slot in between when their predicates land). Mines-slice
      shape note: self-identifying arms set addbranch = FALSE (knox
      precedent) and the numeric status form stays out of livelog
      prose (see the In_mines arm's addbranch split)
- [ ] Endgame randomized plane order + mkportal dlevel-1 semantics
      (MUST land with Phase 3 endgame.lua - see crib sheet gotcha)
- [ ] SWEEP: FEATURES_ARCHIVE.md Features Index (57 entries) +
      dated 2026 sections (~100) - tick each off as covered by the
      subsystems above or added here; nothing skipped silently
- [ ] SWEEP: UNLOGGED NUMERIC-CONSTANT diff, master vs branch (NEW,
      minted 2026-07-25 by the special-room-types slice). The two
      archive sweeps above are KEYWORD-driven and therefore cannot
      catch an EvilHack tweak that was never archived - by
      construction. Found case: EvilHack master lowered COCKNEST
      generation from vanilla's `u_depth > 16` to `> 15`, with no
      BUGFIXES_ARCHIVE or FEATURES_ARCHIVE entry. The wholesale 5.0
      tree adoption reverted it to upstream's 16 and nothing would
      ever have flagged it; caught only because the room-types slice
      happened to rewrite that ladder. Restored to 15 with that
      slice.
      WORK: a value-level diff of master against the branch over
      already-ported subsystems, looking for bare numeric constants
      EvilHack changed away from vanilla - depth gates, rn2/rnd
      probabilities, damage and timeout values, percentage rolls,
      table weights. Establish provenance for each hit the same way:
      compare vanilla at merge-base 44b6612fd, upstream 5.0, and
      master; only a vanilla==5.0 != master triple is an EvilHack
      tweak that the adoption reverted. A vanilla != 5.0 difference
      is an upstream change and stays per the collision rule.
      Highest-yield targets are the files whose slices are already
      committed, since those are the ones where a reverted constant
      is now invisible.
- [ ] Per-subsystem: BUGFIXES_ARCHIVE.md keyword sweep as it lands

## Phase 3 - Levels and quests (233 lua files from 36 .des)

Workflow + gotchas: /root/nh50-port-preview/exemplar/DES_TO_LUA_CRIB.md.
Delta-diff strategy for EvilHack-modified vanilla levels (plan §6 Ph3).
Statuses per file: C=converted, P=polished, W=wizmode-walked.

| .des | levels | mode | C | P | W | notes |
|------|--------|------|---|---|---|-------|
| bigroom.des | 10 | delta-diff | x | x | | EvilHack delta vs 3.6 base = 0 lines (never modified); upstream bigrm-1..13 adopted verbatim incl. 3 upstream-new variants; level-count/chance reconcile belongs to the dungeon.lua row; no W walk needed (no EvilHack content) |
| castle.des | 3 | delta-diff | x | x | x | THIRD new-variant slice 2026-08-01: castle.lua renamed castle-1.lua + EvilHack delta (iron-bar FFF wall rows + the two hallway door cells becoming walls, 4 map rows spliced byte-verified; up-teleport levregion 79->77; magic chest (32,08); steel/iron door materials incl. the two percent(50) alternate-position doors; SporkHack wand scheme = 50% locked chest ELSE crystal chest holding wishing spe=2 recharged=1, three decoy chests random-wand/nothing/random-wand via four successive place:rndcoord(1) picks, Elbereth + cursed scare monster at all four towers; sergeant/lieutenant tower guards). Upstream evolution KEPT (noflipy, levregion stair-up + two teleport regions, place selection, filled=2 throne, 27-line court with 'H'); upstream's added potion of gain level DROPPED (Decisions log ruling - strict master chest parity). castle-2/3 from the converter + polish; castle-2 gains shuffle(monster) = BUG-1455 (master defines the register without SHUFFLE, frozen-unfixed); dungeon.lua castle nlevels=3; build lists = the oracle 9-file set + setup.sh regen; nhdat packs castle-?.lua. CONVERTER: TWO new gaps recorded in the crib, both silent to luac + the nil-sweep: (a) register-loss - $object[n]/$monster[n] value args and monster ids paired with $place[n] coords all emit without the register/id (61 hand-restored sites: armouries, courts, iron golems); (b) positional-statue runtime abort - emitted des.object("statue",x,y) hits lspo_object's montype-capable arm (STATUE/EGG/CORPSE/TIN/FIGURINE unconditionally read table fields) -> "attempt to index a number value", the rest of the level script dies, and the real path degrades to a fallback maze (observed: 8 magic-chest cells + 2 thrones on the mangled level). (b) was CAUGHT BY THE BATTERY on castle-2's first run and fixed same session (table form, upstream's own statue idiom); corpus swept clean. BATTERIES PASSED (2 runs, 6 games; paniclog zero new lines - the 3 pre-fix castle-2 entries went with var/ on reinstall): per-variant censuses exact via read-only gdb walks incl. door materials (castle-1 14 iron + 4 steel; castle-2 11 iron + 3 steel + 3 map-S secret; castle-3 28 bronze incl. 15 secret), wand of wishing EXACTLY 1 per level in-chest with spe=2 recharged=1 proven on every variant and BOTH paths, wand-of-nothing decoy, 4 Elbereth/scare tower spots, storeroom/armoury register classes uniform (register restore proven live), castle-2 mirrored court 9/9 class multisets on the real level, castle-3 court/golems/prisoners/barracks/boulders/polearms exact, titan 25% arm seen firing and missing across runs, SPLEVTYPE real-path selection proven on stronghold dlvls 28/25/26 (dlvl via svd.dungeon_topology.d_stronghold_level - ^V past the DoD end lands in the Valley, so read it first), flip-independence proven (levels arrived unflipped, x-, y- and x+y-flipped across runs). ENGINE FACTS from the battery: class='V' placements generate REAL vampires already in animal form BY DESIGN (makemon gives every shapechanger newcham() at creation - cham holds the vampire; mlet reads S_BAT/S_DOG; at difficulty 1 mkclass(S_VAMPIRE) has no candidates and falls back to a random monster); PM_PRISONER is peaceful by species (all four prisoners peaceful though only two are scripted so); barracks filled=1 fills can roll extra chests (wizloaddes-vs-real census delta); wizloaddes does NOT clear gf.ftrap from the pre-load level (first-load census pollution, lever artifact only). Watch item: the latent upstream lspo_object positional-form gap for the five montype-capable ids - candidate for an upstream report / a maybe_contents guard, not taken this slice |
| endgame.des | 5 | delta-diff | x | x | x | 5/5 delta applied + mkportal In_endgame dlevel-1 arm ported SAME COMMIT (mkmaze.c) per the crib portal gotcha, 2026-08-01. Self-named portals on all four portal planes; monster upgrades (elder minotaur/elven royal, antimatter vortices, 4 lava demons + 12 salamanders, sea-life row, 3 Archangels, undead-free nasties + celestial dragon); earth arrival messages commented out per master. Battery via REAL ^V plane visits (wizloaddes cannot exercise the In_endgame arm - u.uz must be in-endgame at creation): portal dst tuples proven Earth 5->4, Air 4->3, Fire 3->2, Water 2->1, Astral portal-free, dst.dnum preserved; all censuses exact; W ticked on those live visits, though portal traversal on foot was skipped (water-bubble drift) - full ascension traversal lands with Phase 7 validation. Battery's single paniclog line = the pre-existing mplayer kit-switch gap (see the Phase 2 Mplayer kit reconciliation row), NOT this slice. With vanilla fixed plane slots the arm reproduces vanilla progression; randomized order arrives with the dungeon.lua row |
| gehennom.des | 14 | delta-diff | x | x | x | TWO-COMMIT slice 2026-08-05 (knox/tower/soko/mines/yendor precedent): commit 1 = C payload, commit 2 = levels + topology. DUNGEON resize (20,5)->(16,5); levels: valley V 1,0 / hella A 3,2 nlevels 4 / hellb B 6,2 nlevels 3 / hellc C 12,2 nlevels 3 / orcus O 9,2 / sanctum -1 / wizportal F -3 (kept LAST for the chainlevel scan bound); juiblex/asmodeus/baalz rows deleted, files RENAMED to hella-1/2/3 (git mv). C PAYLOAD: topology plumbing (d_hella/d_hellb + macros + Is_hella/Is_hellb/Is_hellc/Is_orcustown; Is_juiblex/Is_asmo/Is_baal + their d_ fields DELETED, all four consumers retargeted juiblex->hella: dbridge is_moat, pager+hack waterbody swamp, mkmaze baalz_fixup on_level; decl.c initializer net -1 {0}; dgn_topology size change = save-break via critical_sizes, d_hellc precedent); ACTIVATIONS: do.c open-air obj_resists arm migrates to hellc_level unconditionally (rloco interim fallback deleted), bones.c ART_BUTCHER -> hellb_level TODO discharged (the old TODO text blamed the Ice Queen branch - master maps it to hellb, verified); ENTRY EVENTS: goto_level hella/hellb/hellc/orcus _entered setters (the bits existed on branch but had NO writer - dlord_protected()/dprince() gating was inert until now); LEVELPORT: level_tele four clamp rungs before the invoked rung + Cerberus Valley lockout after the udemigod lockout + master's dlord/dprince probabilistic negation loop BEFORE the udemigod lockout (rn2(10)/rn2(20), "Demonic forces prevent...", DEADMONSTER-guarded, wizard overrides - reviewer HIGH; fully discharges yendor watch item 7: the loop sits above the lockout exactly as master places it). LAYERING NOTE: master carries the SAME rn2 loop in BOTH paths (scrolltele ~:578 and level_tele ~:931; it has no noteleport_level at all); the branch instead keeps 5.0's noteleport_level() dlord arm ("demon court in Gehennom", m_blocks_teleporting) for the IN-level path - unconditional, STRONGER than master's 9/10, kept per the BUG-772 adjudication (5.0-native shape; get_iter_mons skips DEADMONSTERs) - and ports master's probabilistic loop only where the branch had no equivalent (level_tele); monster-side tier/orcustown banish guards NOT ported - they live ONLY inside master's unported SPE_DISPEL_EVIL banish ladder (same site as the yendor WoY-immunity deferral; travel with that feature; master's mlevel_tele_trap has only the Lucifer arm, already on branch); describe_level: Soko:%-2d (opportunistic remaining-arms discharge) + Vlad:%-2d (In_tower) + Hell:%d(%d) In_hell arms, Gehennom-chain block ordered before In_hell per master's comment, addbranch prose "Sokoban N"/"Vlad's Tower N"/"Gehennom N" (mines shape); MIGR_STALK 10 + dlords_stalk() under !iflags.nofollowers (wiz_makemap regen must not stalk) + losedogs muy<dlevel match + laughter/forboding messages (master's "forboding" typo kept, player-facing parity) + mon_arrive case via fromdlev.dlevel (5.0 translation of master's dlev=muy capture); VALLEY BUNDLE: dodown Cerberus gate, no_bones_level Valley-while-Cerberus-lives clause (master's own comment is inverted vs its code - branch comment states the accurate version, session-27 precedent), bones removal Charon clause, hero HP-regen Valley suppression both arms + arrival warning (discharges the branch's own TODO-EVILHACK), music sleep nap pline + sleep_monst Cerberus TOOL_CLASS carve-out + mfrozen cap 8, Charon domonnoise via the PRE-EXISTING com_pager_subset() helper (ucerberus ? 3 : 0 = master's rn1(QTN_CHRN_NOCERB:QTN_CHARON) semantics, BUG-1301-correct), insight killed_cerberus line beside the wiztower line; vorpal Cerberus beheading ALREADY PRESENT (5.0's unified artifact_hit carries both directions - master's mhitm/uhitm copies subsumed, why literal greps miss it); RE-PORTS: BUG-335 baalz_fixup COLNO guard (hella blocker), BUG-851 orcus shk purge set_residency+NULL guard (verified live: resident set at shknam.c:785, nothing in the detach path clears it), BUG-825 stealamulet doname-before-freeinv, BUG-923 dogaze graceful non-CONF/FIRE refusal, BUG-1300 role half (classmon impossible removed, record files cross-version), mkgate()+mkgate2() into mklev.c carrying BUG-644 (== not =) + BUG-1085 (flags=0) + stairs.c stairway_del_at() helper (5.0 has no single-stair removal; replaces master's xupstair=0) + invent.c/pray.c call sites (TODOs discharged) + mon.c Rat King hook (mkgate2 was a MINES-slice gap - minend-5's treasure room was sealed forever; mines row noted); ADJUDICATIONS: BUG-772 shape (5.0 unconditional m_blocks_teleporting kept vs master's rn2 rolls, BUG-861/1086 precedent); savecharon/restcharon NOT ported (master persists the whole mutated permonst tail after the angered-Charon mmove=18 write; branch carries the mmove writes (mon.c) but not the persistence - matches how master itself treats the Oracle's identical write; restore-hook re-apply available on request); hero-side Wand-of-Orcus regen suppression + elf_can_regen/orc_can_regen gates verified ABSENT branch-wide (separate unported racial/artifact regen subsystems, not smuggled in); interim: the clamp rungs read depth() of unset hella/hellb levels between commits (yendor WizTwr-arm interim class, commits land together). LEVELS: valley/sanctum delta by hand (valley = master map wholesale incl. Cerberus lair/moat/drawbridge/iron safe/Dante engraving, 5 morgues, arrival_room Cerberus area, stone doors, 4th IF path arm; hdgn PORTAL OMITTED w/ TODO-EVILHACK comment - find_level(NULL) would fire the BUG-1460 impossible every load until the Hidden Dungeon row; sanctum = lava-sea map, Lucifer + 4 asleep named balrogs + Baalphegor, drawbridge pair, 10 secret bronze doors, crystal chest w/ marker, aligned cleric x9, fire-ring DELETED, Purgatory BRANCH OMITTED w/ TODO comment, temple contents secret-door kept = 5.0's fixup_special equivalent); orcus delta (y+2 shift proven by aligned-row diff, lava frame w/ master's exact seed tuples incl. the (0,18)/(46,18) oddity - frozen-note candidate, noteleport RESTORED - upstream had dropped it, wish-compensation deleted, hell_tweaks + preamble stripped, vampire tiers sovereign 3/noble 4/royal 2 BUG-1366/1367 guard held); hella-1/2/3 delta onto renamed juiblex/asmodeus/baalz (upstream register idioms kept; boss roulettes; mazewalks re-added on hella-1; hell_tweaks stripped from hella-2; upstream's baalz-legs TODO FIXME comment deleted - the redrawn x-leg map + baalz_fixup retarget resolve it; hella-3's [N%]: LOOP arms hand-written w/ nhlib d()/percent()); hella-4/hellb-1..3/hellc-1..3 from scratch (hella-4 dual water: fg="W" fill + terrain overwrite order preserved; hellb-1 mid-level NOMAP restructured = pre-NOMAP content in the des.map contents closure, post-NOMAP top-level, lspo_map reset_xystart_size frame parity verified in-engine; hellb-3 ragged map padded rows 1/17; hellc-2 = six lemurepit REGIONs filled=1 (FILL_NORMAL -> fill_zoo LEMUREPIT verified) + G3 cursed,0 x10 + G4 class-qualified x12 + "Balrog"->"balrog" (master latent, frozen-note candidate) + named asleep entourages; hellc-3 niches register 1-based + marilith inventory blocks + G2 coord restore x10 + converter's runtime-broken bare levregion() exclude fixed); hellfill = MERGE per ruling (upstream 7-variant generator kept incl. invocation-level native handling - NO mklev exclusion; master's five flavour groups grafted into populatemaze(): L 50%, shade x2/skeleton warrior+skeletal dragon 50%, J 66%, salamanders 75%, alhoon 20%; upstream's shuffled registers already = master's unshuffled ones fixed - master latent, frozen-note candidate); crib gaps hand-fixed per level (census tables in the agent reports, all des-vs-lua counts equal); luac 15/15 (incl. dungeon.lua), runtime-nil sweep clean, id gate CLEAN (script; NAMS-aware), nhdat packs the 10 tier files w/ zero old names. BUILD LISTS: Makefile.top/Makefile_top.vms/install.com/Makefile.nmake (LUA_FILES explicit + luawildcards)/files.props (explicit x10)/pbxproj (file list + dlb shellScript)/Files reflow (ASCII order, hella-* before hellfill) + setup.sh regen; sys/windows/GNUmakefile stays stale by soko precedent. WATCH: mkgate constants (17,10)/(17,11)/(66,16) + mkgate2 (23,10) are ABSOLUTE and assume the sanctum/minend-5 maps land at master's placement - battery must gdb-verify the cells; hella-2 drawbridge x=33 beyond map 2's width (safe per get_location, master parity) and hella-3's IF-block wall rewrites = battery eyeball spots; hellb-1 iron safe lands level-absolute (14,8) NOT the throne room (master same, post-NOMAP - frozen-note candidate); hellb-3 river endpoint y=20 clips below the map frame (master behavior class); bonetag A/B reassigned (asmodeus->hella, baalz->hellb) - pre-slice dev bones load into wrong levels, dev-only. REVIEWER (CONDITIONAL PASS -> all five actionable findings fixed pre-commit): HIGH dlords_stalk needed a DEADMONSTER continue - master relied on ITS migrate_to_level() opening with a DEADMONSTER return that the 5.0 body lacks (pattern-ported-call-lost-precondition; keepdogs guards the same list the same way); HIGH invent.c mkgate() call was unconditional - regated on !achievement_recorded(ACH_AMUL) && !Role_if(PM_INFIDEL) (master gates on first acquisition; ungated it re-locked opened gates on every sanctum re-pickup and opened them for a pre-imbue Infidel); HIGH + the negation-loop port above; MEDIUM Rat King bones removal clause (mptr + cham halves; travels with mkgate2 - the King can stair-flee its level and would duplicate via foreign bones); MEDIUM hero-side !wielding_artifact(ART_WAND_OF_ORCUS) regen clause restored to BOTH rewritten regen_hp arms (master carries it in exactly those arms; the monster-side twin mon_orcus_wield was already live = observable asymmetry). Reviewer LOWs recorded as parity notes: stalk migrates trapped/entangled dlords that self-heal (master-latent, fuzzer note); mounted/ridden-dlord erid shapes unreachable today; Cerberus nap Monnam-unseen master-verbatim; "forboding"/"pickup" master typos kept byte-exact (player-facing parity, soko precedent). Reviewer verified: decl.c initializer 17/5/3/7 exact vs struct order; stairway_del_at unlink clean w/ no cached-pointer holders; MIGR_STALK single-producer + fromdlev-before-mon_track_clear ordering; Charon subset vs the com_pager_core contract AND master's rn1 semantics; goto_level setter position (u.uz0 still departure); sleep_monst carve-out DEADMONSTER-safe. BATTERY (evilhack-game-tester, 5 games + 1 restore, 14/16 legs PASS first pass; the two FAILs fixed same session + targeted re-battery): topology exact across 5 games (n_dgns 10, Gehennom 16-21, all four tier slots BOUND and in range, ^V span sane); all 10 tier files load clean w/ per-level censuses exact (lemurepits 6/6 STOCKED 10-20 lemures + exactly one horned devil each; hellc-3 niche pairs + marilith bows; hellb ice ring/drawbridges/level-absolute iron safe; hella F-bars/bronze/LOOP fauna counts); boss roulettes ALL FOUR ARMS per tier across reloads with the bribe expectation exact (Geryon/Baalzebub/Mephistopheles/Dispater invisible+peaceful, Juiblex/Baphomet/Yeenoghu/Lolth/Demogorgon/Tiamat/Graz'zt/Asmodeus hostile); valley census on a FLIPPED real-path load (transform-verified incl. engraving text, stone sdoors material 24, arrival room = real rooms[] entry proving arrival_room took); Cerberus all 6 arms (gate refusal verbatim, lockout shudder + Override, kill -> cerberus_defeated pager + both uevent bits, descent confirm, #conduct "defeated Cerberus", flute nap mfrozen=7<=8 THROUGH sleep resistance); Valley regen all 5 arms (61-turn pin at 99, undead-poly regens, non-undead-poly pinned 67 turns, filler resumes, Wand of Orcus wield pins/unwield resumes); entry events + clamps (all four _entered flip on arrival; wizard=0 clamp chain hella->hellb->orcus->hellc all SILENT to the clamped depth then Sorry... only on the invoked rung at deepest-1; dlord negation 3-of-4 blocked at rn2(10) + both Override prompts + both message tiers; in-level scroll blocked unconditionally by the noteleport_level arm only WITH a dlord present); MIGR_STALK all 4 arms (laughter + adjacent intercept x2, pretend-never-left on return, waits out a Wizard's-Tower excursion as MIGR mux=1, intercepts on deeper re-entry); hellc open-air migration (Amulet 1:13 pline = d_hellc_level readback, found on the hellc floor); mkgate unflipped + re-pick no-refire + INFIDEL arm (no gate at pickup pre-imbue, opens at the #offer imbue); mkgate2 door + outside-Mines no-op; status/livelog forms exact incl. prose "Gehennom N"; hellfill 6+ distinct variants over 15 regens w/ graft rates matching (salamanders 100% is CORRECT - both roulette arms place one; shades XOR skeleton pair; alhoon 3/9; minotaurs upstream), invocation level = vibrating square + NO down stair; baalz_fixup leg (hella-1 pools survive byte-identical as hella_level, hella-3 legs intact, hella-2/4 clean); orcus town (both shops resident=NULL - the BUG-1468 leg live, vampire tiers 3/4/2, noteleport, alhoon on the altar); save/restore terrain md5-identical + _entered bits + wizmakemap regen + full-flow no-crash. THE BATTERY CORRECTED TWO BRIEF PREMISES: hella-4 WATER=0 is CORRECT (the survey had already established master's own 'W' fill is dead - the fillrect overwrites it before the map lands; the brief's "deep-WATER edge" expectation was the error) and the valley iron-safe 10-items-vs-9 is engine-native random gem quantity/merging, not a slice defect. TWO REAL FAILS, BOTH FIXED SAME SESSION: (1) Charon chat UNREACHABLE - Charon carries M2_MAGIC so the likes_magic "talks about spellcraft" arm shadowed the PM_CHARON switch case; MASTER HAS THE IDENTICAL DEAD SHAPE (same chain order, same M2_MAGIC) so Charon's six authored lines have never rendered on master either and BUG-1301 fixed a range inside dead code (static-analysis find, never live-repro'd) - branch fix hoists a PM_CHARON arm ABOVE likes_magic (peaceful -> com_pager_subset, hostile -> master's silence), master side = frozen-unfixed archive entry; (2) mkgate/mkgate2 FLIP-SENSITIVE - 5.0's flip_level_rnd (which 3.6 master never had) relocates the maps out from under the hard-coded absolute coordinates (~75% of sanctum loads flipped; carved mid-map doors + left the real upstairs live + fired a "vision blocking" impossible; y-flipped minend-5 put the door in the wrong wall) - fixed with noflip on sanctum.lua AND minend-5.lua (functional requirement, sokoban-noflip class; comments in both files name the C coupling). Battery watch notes: wished artifacts blast at WISH delivery (touch_artifact at wish time, pre-existing, not slice); unique mvitals born/mvflags bookkeeping inconsistent across wizloaddes reloads (lever artifact); G3 lost to spawn churn during a wizard=0 window (harness note: zero flags.debug only on a cleared level). RE-BATTERY (targeted, 1 game, paniclog NEVER CREATED): E Charon chat PASS (9 alive-chats spanned all six lines incl. both Cerberus mentions, zero spellcraft; 18 dead-chats drew ONLY the first three, the boat line correctly excluded); J sanctum noflip PASS (8/8 regens unflipped - STAIRS exactly at (66,16) every time; gate fired once w/ full state flip, re-pick silent, no vision-blocking line; the earlier no-message wish acquisition was a staging artifact - mkgate's plines are cansee-gated and the hero lacked LOS); K minend-5 noflip PASS (2/2 unflipped, Rat King kill -> You_hear arm + (23,10) locked DOOR between the sewage cells). NEW WATCH: the sanctum temple's contents door (upstream sanctum.lua wall="random") can land its SDOOR ON a gate cell - observed 2 of 8 gens at (17,10)/(17,11), the temple WEST wall = the gate cells; master's C create_secret_door used W_NORTH|W_SOUTH only so this never happens there; benign (mkgate overwrites typ+doormask wholesale, worst case a found-and-opened secret door re-locks at gate time) but the SDOOR+mkgate firing combination was not directly exercised - candidate one-line fix = wall="north" or "south" in the temple contents fn if the maintainer wants master parity; gdb replay notes: svl.level.stairs does not resolve (use stairway_at()), achievements live in u.uachieved[] |
| knox.des | 3 | delta-diff | x | x | x | SECOND new-variant slice 2026-08-01: knox.lua renamed knox-1.lua + EvilHack delta (6 map S->wall cells byte-verified; gold secret doors via the LIVE des.door material= binding - fort/vault 50% if-else pairs + 4 gem rooms; magic chest beside Croesus in both throne arms via des.feature; locked iron safe w/ magic key+gem+ring+amulet); knox-2 (Winterly)/knox-3 (Minas Tirith?) from the upgraded converter + polish; dungeon.lua knox nlevels=3; build lists = the oracle 9-file set + setup.sh regen. Upstream idiom adopted on all 3: treasure_spot Lua fill (branch mkmaze.c has NO Is_knox morgue arm - the Lua fill is the only stocking mechanism), arrival_room=true, filled throne; knox-3's ragged treasury (60 floor + 24 stone in-rect) fills via selection:filter_mapchar('.') = master's !is_solid guard, battery-proven 60/60 gold on floor, 0 on stone. CONVERTER upgraded this slice (crib log): DOOR material table form + MAGIC_CHEST:/FORGE: des.feature emissions (301/43/10 corpus-wide, regression clean). HARD LESSON (new crib section): des.map rows MUST be uniform width - stripping the .des trailing-space padding sheared knox-3 (lspo_map upstream TODO), cascading into 6/12 drawbridge impossibles + irregular-region flood-fill (759 monsters); restore of the 76-char rows fixed all of it (re-battery 12/12 drawbridges, transform-verified zero shear, 185 monsters). REACHABILITY C PORT same slice, 2nd commit: mk_knox_portal rn2(3) defer REMOVED (master delta; invisible in wizmode since the defer had && !wizard), mk_knox_portal_at + mk_knox_fallback_vault into mklev.c (svn./svl./svr. structs, needfill=FILL_NONE replaces 3.6 fill_room-prefilled, set_levltyp replaces raw typ writes - CAN_OVERWRITE_TERRAIN refuses only stairs/ladders), find_knox_fallback_spot + Medusa-arm trigger in mkmaze.c, extern.h decls - closes the terrain-slice no-branch-counterpart deferral. NOT PORTED (inert master delta): mk_knox_portal's nymph/forest dnum tests - both main-dungeon levels, so they duplicate the oracle-dnum test and special levels never run vault code. BATTERIES PASSED (2 sessions, 7 games, paniclog clean): per-variant censuses exact incl. material=GOLD sdoors (farlook "closed gold door"), ice 48/48, portal round trip; fallback proven by breakpoint chain fixup_special->find_knox_fallback_spot->mk_knox_fallback_vault->mk_knox_portal_at on a fresh Medusa visit + on-foot traversal into Fort Ludios. Wizmode notes: ^V cannot list Fort Ludios until the branch is wired (upstream tport_menu floater behavior; gdb mk_knox_portal_at is the lever); DOOR:closed aimed at a map S cell stays secret (master parity, vanilla knox same). Watch items: fallback vault tiles carry roomno=0 (add_room without topologize; master same shape, no misbehavior seen); dog.c MIGR_PORTAL impossible() not fuzzer-gated when no stairway matches (otherwise 5.0-native equivalent of master's hardening) - Phase 7 fuzzer item |
| medusa.des | 4 | delta-diff | x | x | | 4/4 delta applied onto upstream files 2026-08-01: maps spliced byte-identical (w->p verified per file), scimitar->saber, cobra->giant-anaconda upgrades + python, moved Medusa/Perseus/stairs/doors/regions/traps on 1/2, medusa-4 place-list 4->3 spots + 3 sealed doors + board/land-mine row + S-loop 16; upstream 5.0 evolution KEPT (medloc/altloc rndcoord scheme on 3/4, arrival_room rooms, historic/male Perseus, Medusa asleep on 3/4); luac + runtime-nil sweep clean; engine leg PASSED 2026-08-01 (two games): 4/4 wizload clean no fallback, Medusa + named Perseus statue verified per level, medloc co-location proven on 3 (statue+Medusa+stairs same cell), puddle census EXACT at depth 21 (235/235 on medusa-4) and short only by dug-trap cells at dlvl 1 - engine-native maketrap IS_ROOM dig-out under PIT/HOLE (master parity, master trap.c 'typ = ROOM' sibling), landmine intact at depth 21 flanked by both boards while at dlvl 1 it correctly reads as an exploded PIT (5.0-native mktrap victim mechanic, level_difficulty <= rnd(4)); paniclog never created. W column ticks after the damp-terrain sweep re-walk (745 puddle cells inert until then). Converter gap found en route: MONSTER:random,(x,y) loses its coords (bare des.monster()) - crib updated, hand-restore on from-scratch levels |
| mines.des | 14 | delta-diff | x | x | x | TWO-COMMIT slice 2026-08-02 (knox/tower/soko precedent; Decisions log x4): commit 1 = C payload, commit 2 = levels. Row = 11 delta-diff (upstream ships minefill + minetn-1..7 + minend-1..3 as lua) + 3 EvilHack-only from-scratch (minetn-8 "Frontier Town - not yet pillaged", minend-4 "Orc Temple" by Khor, minend-5 "The Sewers of Waterdeep"); dungeon.lua minetn nlevels 7->8, minend 3->5 (minend stays NO-BONES per ruling - deliberate deviation from master's bonetag "E"). C PAYLOAD: mines rivers ported (makeriver/mkrivers -> mkmap.c in 5.0 idiom: staticfn, svr.rooms/svn.nroom, IS_ROCK->IS_OBSTRUCTED (definitionally identical), 80-col rewrap incl. hoisted aquatic pick; mkmap() call gated In_mines && !In_hell && !Is_minetn_level && !Is_branchlev && rn2(dlevel+1); mkstairs gains master's undetected-swimmer reveal tail; reviewer proved random stairs CANNOT hit the 5.0-only stairs-on-water impossible - good_stair_loc+DRY on random, force=TRUE sets ROOM on fixed); minetn_level topology plumbing (d_minetn_level in dgn_topology between knox and mineend + hack.h macro + Is_minetn_level + dungeon.c level_map entry + decl.c init_svd one more {0}; dgn_topology saves via the historical whole-struct path ONLY - no fieldlevel walker names its members - and sizeof lives in critical_sizes[] so old saves reject cleanly, d_hellc_level precedent); describe_level mines arms (Mine Town / Mines' End / Mines:%-2d, addbranch=FALSE knox-precedent, the numeric arm splits addbranch so livelog prose reads "Mines N" - reviewer NIT taken; REMAINING master arms = the NEW Phase 2 row); BUG-1459 = BUG-858 migrate_orc clamp re-port (archive sweep hit; adoption had reverted it). Orctown C machinery (check_ransacked/migrate_orc/stolen_booty) was ALREADY on branch in 5.0 idiom incl. racial_orc + BUG-853 set_malign; upstream's generalized stolen_booty food exclusion (oc_prob!=0 + corpsenm guard) KEPT over master's hand list. LEVELS: minetn-1 Orcish Town = the big delta - LEVEL->MAZE (level_flags mazelevel+inaccessibles), inside/near_temple selections MOVED BEFORE the door placements (floodfill must not flow through door cells into shops - ordering is content), barricaded LIVE shop (region shop filled + locked door + 3 boulders), magic chest (20,14), teleport_region -> master's down-only levregion pair, REPLACE_TERRAIN first rect (07..11 -> 11 only), upstream's corpse-object block REPLACED with 3 dead=1 shopkeepers on shuffled shk_demise + boulders on those spots (gear drops beside corpses - the dead= slice payoff), aligned cleric dead=1 at (20,12) + d(3)+1 spellbooks, 4+1 dead Watch, loot loops (9x tool class + 4x food class + 6x the 80/50 candle/lamp tree at inside:rndcoord(0)), rubble 5+d(2,5), goblin-captains 1d2 + 20% goblin outrider arm; minetn-2/3/4 = temple magic chests only (renames were session-23); minetn-5 = magic chest + statue montype "gnome royal" (master's own name); minetn-6 = WHOLESALE REPLACEMENT with SporkHack Zoo Town on master's 16-row map (upstream's 20-row full-height Bustling redesign does NOT carry - master content wins; zoo pen F-bars+grass, NON_DIGGABLE, monkfoodshop() food shop matching adopted siblings, in_zoo/outside selections, bullwhip handlers via inventory closures, 65% tiger / anaconda / 85% mumak pen stock); minetn-7 = magic chest + counts 2 rock/2 noble; minetn-8 from scratch (SUBROOM type-chance [90%] -> the native chance=90 room key, minetn-7's monkfoodshop chance=50 precedent); minend-1 = 1-cell map fix, forge (48,13), buried magic-key chest + "One-Eyed Sam was here" engraving at place[6] (upstream's deliberately-empty niche slot - their mapping runs des[0]->lua[7], so the .des's place[6] IS lua place[6]), 2 royal/2 noble counts, hobbit pickpocket; minend-2 = 2 map rows gain right-side cells + fountain map cell removed, master's map-relative teleport_region REPLACES upstream's islev one (upstream-also-changed-it -> EvilHack content wins), stone locked door (11,06) table form, forge REPLACES the (14,13) fountain, royal wine-cellar engraving texts, Yendorian mercenaries at (14,4/5/8), burned Elbereth (10,09), buried key chest + "Property of Croesus" at place[1]; minend-3 = key[] 6-spot shuffle + buried key chest + "X" engraving; minend-4 from scratch (RAGGED map PADDED to 76x20 - crib clause; 14 statues + balrog figurine ALL table form, 2 bare iron safes, 3 altars incl. noncoaligned elven shrine + Elbereth + 33% elves, 8 board traps ringing the figurine, orc army incl. case-insensitive "mordor orc" normalized to Mordor orc, stones[9] buried key chest "Do not dig here"); minend-5 from scratch (75x18 uniform, des.message x2, non_passwall x2, throne map cell = Rat King's seat, Rat King asleep (60,7), trapped chest w/ magic key + tool tree (inner sack/oilskin/BoH have NO contents fn - no lspo push/pop nesting, watch item respected), crystal chest + iron safe hoards w/ gold piece quantity=5000, place[3] deliberately empty); ALL FIVE minend luckstones achievement=1 with upstream's buc="not-cursed" DROPPED (soko prize ruling). Luac 14/14 + runtime-nil sweep clean + zero positional montype forms + 6 dead=1 sites verified. BUILD LISTS: nmake/files.props/MSC/pbxproj explicit adds + Files reflow (3 insertions ripple to section end); Makefile.top/vms/install.com/luahelper/dlb-shellScript all ?-wildcard covered - NO setup.sh regen; nhdat verified carrying all 14 (dlb tf). GNUmakefile LUALIST + test_lev.lua stay stale by soko precedent. CONVERTER GAP found+crib-worthy: SUBROOM (x,y),(w,h) loses the w/h pair (silent, luac-clean) and the [N%] type-chance emits as a literal type string - both hand-fixed on minetn-8. WIZLOADDES GOTCHA: minend files must load while standing IN the Mines (Is_mineend_level gates the achievement write; soko lesson). BATTERY PASSED (3 games, 6 legs, paniclog NEVER CREATED): status arms live on the real path (Mine Town / Mines' End / Mines:N by dunlev; depth-7 non-mines still Dlvl); rivers proven on real fillers (POOL/MOAT/PUDDLE mixes + SEWAGE=213 deep with the dlevel>=5 leech/roach table, aquatic spawns 3-9, zero-river rolls legal, Mine Town + branch level + all five minend ALWAYS river-free, every stair typ=27 never water, wizmakemap regen too); Orcish Town real-path via SPLEVTYPE: all 9 dead-monster gear piles WITH possessions (shk skeleton keys/wands under boulders, captain crystal plate, cleric 2 spellbooks + maces + robe at the altar), corpse objects on 3 of 9 = the documented probabilistic corpse behavior (bigmonst 1-in-3 for humans, session-23 correction - NOT a defect), live barricaded peaceful shk in a stocked filled shop behind locked door + 3 boulders, magic chest + AM_NONE altar, army census incl. warg-mounted captains + outriders, stolen_booty MIGR_COUNT=13 with candles/keys/gauntlets booty + shiny gold + ORC_LEADER m_id-traced to the mines bottom holding MIGR_LEFTOVERS loot end-to-end; all 8 town/Zoo censuses exact (pen 7 bars + 12 grass, cats/anaconda/mumak cham=-1, bullwhip handlers, per-temple magic chests, minetn-5 gnome-royal historic statue spe=4, minetn-8 90% type-chance rolls observed); all five minend: EXACTLY one achievement luckstone per load with oid re-marked and matching o_id across 11 loads, buried magic-key chests + marker engravings per variant, minend-2 forge-replaces-fountain + stone secret door material=24 + mercenaries + burned Elbereth, minend-4 14 statues + cursed balrog figurine + scare scroll + 8 boards + safes + 3 altars + lava ring, minend-5 both arrival messages + Rat King asleep on the throne + NONPASSWALL=686 + 5000-gold crystal chest + trapped key chest on file-load AND a separate real-path game (Rat King present, wizloaddes-consumption avoided); save/restore round trip: prize oid 3754 survives, pickup fires the "acquired the Mines' End luckstone" livelog + oid clears. WATCH ITEM (battery anomaly, unresolved): ONE of five minend-4 loads read the noncoaligned elven altar's altarmask as 3 (AM_CHAOTIC|AM_NEUTRAL - an illegal two-bit mask; the other four loads read a legal 4). Statically unreachable: get_table_align maps "noncoaligned" to exactly AM_SPLEV_NONCO, sp_amask_to_amask resolves via noncoalignment() (master-identical, returns one alignment), create_altar assigns (never ORs), and the AM_SHRINE OR is temple-gated. Either an unidentified post-creation writer or a gdb read artifact; repro fingerprint = #wizloaddes minend-4 on the Mines' End slot + read locations[].flags at the elven-shrine altar cell. Re-check on recurrence before suspecting the slice. Also verified-intended: shrine bit absent on all non-temple-room altars (create_altar gates priestini+AM_SHRINE on croom_is_temple - master parity; the six town temple altars all carry it). POST-SLICE GAP CLOSED ELSEWHERE (2026-08-05): master's mkgate2() (Rat King death opens the minend-5 treasure-room door at absolute (23,10)) was NOT ported with this slice - minend-5's treasure room was unreachable except by digging; the gehennom slice ported it as mkgate()'s sibling (mklev.c + the mon.c Rat King death hook); its battery owes the (23,10) cell check |
| oracle.des | 3 | delta-diff | x | x | x | FIRST NEW-VARIANT slice - mechanics established 2026-08-01: dat/oracle.lua renamed oracle-1.lua (delta: delphi/ordinary room-type swap, four trees, grass, Oracle carries the named Magic 8-Ball; upstream noflip + historic statues kept), oracle-2/3 from converter + polish, dungeon.lua oracle entry nlevels=3 (= master RNDLEVEL count; mkmaze protofile "%s-%d"), build lists swept (Makefile.top, Makefile_top.vms, Makefile.nmake x2, files.props, xcodeproj x2, Files reflow, install.com, outdated MSC) + setup.sh regen. CONVERTER BUG found+fixed en route: SWITCH plain form emitted math.random(1,N) against 0-based CASE labels - case 0 dead AND top roll matched no arm, so oracle-3's ring stayed fully sealed ~25% of games; ~/des_to_lua.pl now emits math.random(0,N-1) (crib maintenance log; vaults.des SWITCHes benefit later). Battery PASSED (4 games, paniclog never created): wizloaddes 3/3 clean; real-path selection via SPLEVTYPE on 4 random oracle dlvls; censuses EXACT per variant (v1 4F/0P/4T + 8 historic centaur statues; v2 4F/15P + elemental statues; v3 3F/19P/4T + EXACTLY ONE open passage aligned with its removed fountain = SWITCH fix live, + woodchuck statue); Oracle + named Magic 8-Ball in minvent on all variants; W ticked on those live visits. LESSONS: wizard variant lever = SPLEVTYPE env var (no picker prompt); wizloaddes consumes unique monsters - run file-load legs and real-level visits in separate games |
| sokoban.des | 24 | delta-diff | x | x | x | LARGEST batch-1 row, TWO-COMMIT slice 2026-08-02 (Decisions log): commit 1 = C payload, commit 2 = levels. Row = 8 delta-diff (upstream ships soko1-1..4-2 as lua) + 16 EvilHack-only from-scratch (soko*-3..*-6); dungeon.lua all four tiers nlevels 2->6. C payload: sel_set_door In_sokoban auto-trap producer + the ruled 7 guards + zap_dig as ruled 8th (mfndpos trapped-door clause; picklock no-lock/magic-key/success-path; doorlock WAN_LOCKING + WAN_STRIKING; untrap; openone + zap_dig with a typ==DOOR overlay qualifier the consumers gained on this tree; zap_over_floor goto def_case ahead of the untouched type>=0 billing) making the two door-slice inert arms live; 3-slot prize subsystem per the context.h ruling (achieveo soko_prize_oid[3], scalar otyp written at pickup; is_soko_prize/is_mines_prize gained a NONZERO-o_id sentinel guard after battery A caught minimal_xname's zeroobj copy matching cleared slots and misnaming ordinary bags "sokoban prize tool"); del_soko_prizes + autopickup exclusion + objnam prize naming (amulet/tool/armor + blind dknown force) + mkobj material exclusion (discharges the TODO-EVILHACK) + Is_nonprize_container with 13 swap sites + dokick never-fall/nodrop + meatmetal guard + wiz-makemap 3-slot clear + three regained sokoban_guilt callers (jump/ball-drag/hurtle, all named in master's own sokoban.des header). BUG-1457/BUG-1458 re-ported in create_object same commit (tree adoption had reverted master's BUG-529/532 fixes; archive entries). LEVELS: noflip on all 24 by ruling; upstream rolling-boulder traps + mongen exclusion zones stripped from all 24 (strict master content parity); soko2-2 keeps upstream's map redesign (only upstream map change kept; its RB cell becomes a hole = 12-hole corridor, ruling intersection); soko1-x three-closet blocks (shuffled place, 50/50 pairs BOTH arms achievement=1, no BUC spec - prizes CAN generate cursed, master parity vs upstream's not-cursed 75/25), stone doors in des.door TABLE form (positional form cannot carry material); soko4-x teleport_region initial-placement + magic chests as des.feature + 50%-gated second earth scroll. Build lists: Files reflow + nmake + files.props + pbxproj + MSC (globs cover Makefile.top/vms/install.com; NO setup.sh regen - Makefile.top untouched). BATTERIES (3 runs / 6 games, paniclog NEVER CREATED): battery A on interim wooden doors proved producer/marking/naming/autopickup/save-restore/seal/foop/livelog/wizmakemap-revoke/jump+hurtle guilt and caught the oid-0 naming defect (fixed same session, pre-commit, no BUG id); battery B censuses EXACT on all 24 (boulders/holes/pits/doors/chests/bars/lava/mimics/scare/Elbereth), NOFLIP proven by pure-translation coordinate match on all 24 + 4 identical reloads, all 7 guard messages verbatim on REAL STONE doors (striking hits the hard_door "reinforced door shudders" arm; stone-to-flesh absorb arm reachable only on stone - both unreachable in battery A, by material), monster-pathing held for soldier/minotaur/stone giant (branch doorbuster predicate = racial_giant; scare-scroll flee needed mberserk staging), seal + pickup + "acquired the Sokoban <item>" + naming-regression probe (claimed prize bag tips as "bag of holding"), SPLEVTYPE soko2-9 out-of-range cleanly ignored, alignment 2 live. REVIEWER (evilhack-code-reviewer, conditional pass, no crit/high): 90-col foop line wrapped; latent nested-container push/pop imbalance in the BUG-1458 shape = WATCH ITEM (master-identical; comment left at the lspo_object pop; no shipped level nests contents-in-contents - canary test level is the suggested follow-up); prize bright-green color = the ruled Phase 4 defer (with door material colors), NOT a silent skip; "wink out of existience" master typo kept byte-exact (one-word fix available on request); producer deliberately does NOT get a typ==DOOR qualifier (master parity; trapped secret doors are a managed overlay; no soko level uses secret doors). ALSO pre-existing-stale, left alone by precedent: sys/windows/GNUmakefile LUALIST and test/test_lev.lua (already stale for castle/knox/oracle renames - cleanup-row candidate); outdated mac Files.r / os2 Install.os2 still name .lev files (pre-Lua relics). WATCH: sfctool's generated fieldlevel walker emits single-element writes for unsigned arrays (soko_prize_oid[3] now joins novel_tracking.pasg in that pre-existing upstream shape; the GAME binary is unaffected - live saves use the historical whole-struct serializer, proven by battery save/restore). Master-parity notes: picklock magic-key arm is belt-and-braces on BOTH trees (no-lock guard fires first on a closed prize door); interim window between the commits leaves upstream soko1-1/1-2's single prize 2-in-3 sealed - commits land together |
| tower.des | 4 | delta-diff | x | x | x | TWO-COMMIT slice 2026-08-02 (Decisions log): commit 1 = the open-air subsystem + BUG-1456 vampshifter cluster (Phase 2 row, see there), commit 2 = the levels. Master rebuilt Vlad's Tower wholesale: dungeon (3,0)->(4,0), ENTRY -1 = enter at tower4 (Bottomless Cavern) and climb UP; Gehennom branch vanilla (9,5)up -> master (8,5)down. tower1-3 = upstream headers/idiom + EvilHack content (30/30/52-wide maps - RAGGED in master's .des, lev_comp padded internally; the Lua files must be PADDED to the widest row, new crib clause); tower4 from scratch (76x20 air map, morgue unfilled, nommap/graveyard/shortsighted/solidify, ~48 sleeping nest vampires + 8 random V + 3 demons). RULINGS: Brides of Dracula KEPT adapted into three tower1 niches (waiting=1 true form, genocide-guarded); Vlad keep_default_invent=true so his makemon Candelabrum is not duplicated by the branch's mdrop_special_objs safety (side effect: he may carry makemon blood potions - accepted). CONVERTER GAPS hand-fixed + crib-recorded: spe dice after name emitted as quantity (Lifestealer = 1d3 stacked swords), class-qualified ambiguous names lose the class (spellbook "magic missile" -> the wand; ring "levitation"), TRAP:random,$place[n] loses the register coord. dungeon.lua: Vlad base 4 + tower4 row + branch 8/5/down; build lists = explicit sets only (Files/nmake/files.props/xcodeproj/MSC; tower?.lua wildcards already match). BATTERIES (2 games + re-battery, paniclog never created): topology proven via gdb (BR_STAIR Gehennom dlev 12 in [8,12] -> dnum 6 dlev 4, entry_lev=4; ^V menu "entrance from below"); arrival narrative + engraving; per-level censuses EXACT on file-load AND real path (Vlad true-form with Lifestealer spe2 erodeproof cursed + EXACTLY ONE Candelabrum incl. minvents; Brides Madame/Marquise/Countess true-form female; ghost-class ' ' placements = 4 spectres mlet S_GHOST; spellbook ocls SPBOOK; ring ocls RING; all 6 tower3 niche traps co-located with their objects; steel/bronze/wood door materials; corpse montypes all valid incl. cave dweller/aligned cleric NAMS mappings); tier vampire forms post-BUG-1456 all valid vampshapes (wolf/warg only on ground, bat/fog on air - noble->warg reachable via master's own fallthrough, verified intended); tower4 nests STABLE over 27 turns (delta = the tester's own wizkill). Real-path traversal note: tower3->tower1 climbed via ^V because tower3's arrival closet is gated by locked+secret doors (level content, not a defect). W ticked on the real-path visits. Watch: wizloaddes onto Dlvl 1 leaves the pre-existing staircase cell + strips the pet (lever artifact, recorded in gameplay memory) |
| yendor.des | 4 | delta-diff | x | x | x | TWO-COMMIT slice 2026-08-05 (knox/tower/soko/mines precedent): commit 1 = C payload, commit 2 = levels + topology. Master rebuilt the tower WHOLESALE (2023, 9be2c12e7 + 3a3cd5ad7): 4 fixed maps - wizard1 open-air lair (TOP), wizard2 sealed middle, wizard3 moated garrison (ENTRY), wizportal Gehennom lava field - as a NEW dungeon "The Wizard's Tower" (chaotic, mazelike, entry 3 = arrive at wizard3 and climb UP, bones W, NOT hellish - In_hell flips inside), chainbranch portal up from wizportal @ Gehennom (-3,0) bonetag F; fakewiz1/2 DELETED outright. Dungeon count 9 -> 10 (15/15 MAXDUNGEON ruling unchanged). C PAYLOAD: BUG-1460..1466 re-ports (854/860 fixup_special LR_PORTAL NULL-guard + d_level zero-init; 643 find_branch_room corridor fallback + 0,0 sentinel + place_branch isok/occupied gates; 646 occupied isok; 647 mkinvpos pointer-after-bounds; 1188 mon_arrive both-direction stairway fallback + fuzzer-gated impossible - one-way portal into a LADDER-only dungeon is exactly this topology; 1466 place_lregion invalid-rect validation + exclusion-relaxation retry, un-archived master fix, the validation piece reviewer-caught after the first re-port dropped it). ADJUDICATIONS: BUG-861/1086 NOT blind-ported - the 5.0 u.ucamefrom fuzzer arm is the native equivalent and now sits inside the new isok/occupied gate; BUG-535 heir = get_location_coord validated coder path, no change; BUG-1150 heir = stairway_at(x,y), no change; 47fb2b0f3 do.c = superseded by 5.0's UTOTYPE named-constant refactor (master's own commit message cites it); 717a6a9f9 = .des-only, superseded by the rebuild. Tower-overhaul guards: teleport.c udemigod levelport lockout ("Powerful magic prevents you from teleporting.") + always-fire scrolltele disorientation (drops !rn2(3); steed-amulet + quest-nemesis arms, svq idiom) + level_tele sibling arms In_V_tower/steed-amulet/quest-nemesis-with-Rogue-carve-out (reviewer HIGH: the first pass ported only the scrolltele half of master's guard commit, leaving the quest-skip levelport abuse open; In_purgatory arm stays with the purgatory slice); potion.c cursed-gain-level tower block; dbridge.c protected-drawbridge feature (Valley/wiz1/wiz3 strain arms + "This drawbridge cannot be destroyed."); dungeon.c u_on_rndspot degenerate-exclusion-rect fallback (<20 tiles -> main region; load-bearing since the tower TELEPORT_REGIONs keep master's 1-tile (0,0,0,0)-shifted exclusion); botl.c describe_level WizTwr:%-2d arm (dnum test - no dedicated flag exists, addbranch split -> livelog prose "Wizard's Tower N", mines shape) DISCHARGING the WizTwr piece of the Phase 2 remaining-arms row; insight.c uevent-keyed "entered the Wizard of Yendor's lair" line after the ACH loop (no ACH slot by xlog-slice design). NOT this slice (rulings): wizard.c hygiene cluster BUG-798..806 + BUG-257 -> the named Wizard-harassment sweep (maintainer 2026-08-05; NEXT-SESSION CANDIDATE - the tower is now reachable); zap.c WoY levelport-immunity lines live INSIDE master's unported SPE_DISPEL_EVIL banish ladder -> travel with that feature; mapglyph magenta tower walls (colored-walls slice); Gehennom (20,5)->(16,5) + hellc activation (gehennom slice). LEVELS: converter run + polish; TWO NEW CONVERTER GAPS (crib log 2026-08-05): ordinary+unfilled REGION emits filled=0 but the 3.6 semantic is the room-forcing migration-control kludge = 5.0 arrival_room=true (filled=0 silently loses the room; fixed on all 5 sites incl. wizportal's irregular+arrival_room = upstream fakewiz1 idiom; bare ordinary REGION = lighting-only in both trees, stays plain); MAZE+FLAGS headers emit spurious duplicate level_init/level_flags. Maps PADDED uniform (wizard2 ragged 29->39 incl. the deliberate blank all-stone bottom row; wizportal 20->21); barbarian keeps class "@", Angel exact case; wizportal 6x corner-seed + floodfill lava + grow(north|west) randline river hand-written (single-pass parity: river | clone-grow-north | clone-grow-west, reviewer NIT); NO hell_tweaks trailer on any of the four (tower not Gehennom; wizportal's .des lava scripting IS the content); NO noflip. BUG-1366/1367 gate: 40 monster + 15 object ids verified vs branch tables, zero placeholders, zero positional montype forms, 51/51 register refs restored. luac 4/4 + nil-sweep clean; reviewer verified maps CELL-FOR-CELL vs master (wizard2's 25 diffs = the w->p puddle remap, correct). dungeon.lua: Gehennom branches += portal-up chainbranch (chainlevel wizportal, base 0; ORDERING COMMENT added - init_dungeon's chain scan bound n_levs+f-1 includes wizportal's last-position only because the branch is second; verified vs parser); levels -= wizard1/2/3+fakewiz1/2, += wizportal F base -3; new dungeon block between Vlad and the Planes; src/dungeon.c level_map fakewiz1 -> wizportal rides commit 1 with the other dungeon.c hunk (portal_level unbound for the one-commit interim; whole-file staging beats an index split). BUILD LISTS: wizportal.lua matches NO wildcard - Makefile.top SPEC_LEVS reflow + setup.sh regen, vms Makefile_top + install.com, nmake explicit + luawildcards, files.props, pbxproj file list + dlb shellScript, luahelper, Files reflow (wizportal after wizhelp, ASCII order). GNUmakefile LUALIST + test/test_lev.lua + test/test_des.lua stay stale by soko precedent (already carried deleted castle/knox/oracle names; fakewiz refs join the cleanup-row candidate). nhdat verified (dlb tf). BATTERY (evilhack-game-tester, 2 games + 1 restore, ALL LEGS PASS): topology exact by gdb (n_dgns=10, tower dnum 7 entry_lev 3 hellish 0 boneid W, wizportal 1:19 = Gehennom 21-2, BR_PORTAL end1_up, ^V "The Wizard's Tower: levels 43 to 45, entrance from below"); censuses EXACT on file-load AND real path (wizard1 AIR=1209 + drawbridge pair DB_LAVA + WoY asleep ON the Book + crystal chest wishing spe=1 recharged=1; wizard2 25 puddles + nursery 12+2 + 50% neothelid observed + captives-on-chests; wizard3 32 aquatics/20 lava-hall/4 marilith w/ bow+100 arrows/stables asleep/both 50% forges observed across loads; wizportal balrog AND vorpal-jabberwock guardian rolls observed across loads, board ring, 596-cell lava field); real-path portal traversal + full ladder chain with WizTwr:3/2/1 status; wiztower_entered fires ONLY on wizard1 (0 on wizard3/wizard2, then 1 + livelog + full pager text); ALL guard messages verbatim (lockout prompt + refusal + override-y works; udemigod=1 removes the lockout cleanly; disorientation 4/4 scroll reads; gain-level uneasy-feeling no-op; drawbridge indestructible on BOTH wiz1 and wiz3 + strain arm on open AND close paths + normal operation clean-span verified); pet-follow both portal directions x2 clean; save/restore round trip byte-stable terrain + WizTwr:1 + wizmakemap regen same content class; enlightenment line renders. PANICLOG: 3 lines, ALL = the pre-existing Phase 2 mplayer-kit row ("bad mplayer monster" - wizard1's des-placed infidel PM 497 is inside is_mplayer()'s range with no kit arm; fires deterministically on EVERY wizard1 build - that row is now ESCALATED, see it). RESOLVED PRE-COMMIT by an instrumented battery leg (background-gdb backtraces on one transit): master's do.c "heat and smoke are gone" arrival arm is SUBSUMED on the branch - 5.0 generalized it into temperature_change_msg() (do.c, `pline_The("heat %s gone.", In_hell(&u.uz0) ? "and smoke are" : "is")`, fires on any hot->cold arrival) - so porting the arm printed the sentence twice per Gehennom->wizard3 transit; the format string defeats a literal grep, which is why the sweep called the arm "missing". Arm REMOVED; the native path alone is master-equivalent. LESSON for vecna/gehennom/purgatory slices: do NOT port master's heat-smoke arms; grep for format-string variants ("heat %s gone") before porting any "missing" message. WATCH ITEMS: (1) u.uevent.udemigod read back 1 after a save/restore despite a verified 0 write pre-save (tester sequence: gdb set 0 -> read 0 -> zaps/^X/#conduct -> S -> restore -> 1); no slice code writes the flag (writers: wizdead, Book read, u_init) - suspect gdb bitfield-write artifact vs live process; fingerprint = leg E sequence, re-check on recurrence; (3) wizportal's portal chamber + watch towers are fully SEALED (dig in; teleport_region excludes the vault) - map is cell-identical to master = master content, intent not adjudicated; (4) wizportal-side portal entry resolves one command late with a look_here "There is a magic portal here." first (wizard3 side fires immediately) - portal machinery untouched by this slice, note only; (5) beam zaps print the drawbridge guard once per bridge square (per-square destroy_drawbridge calls, master shape); (6) mon_arrive endgame arm not hardened to master's later Is_stronghold/xspan shape (defer with purgatory slice per reviewer); (7) gehennom slice must revisit the udemigod-lockout placement relative to master's demon-lord/prince negation loops when it ports them (master places the lockout after the dlord loop). Reviewer: CONDITIONAL PASS -> both actionable findings fixed pre-commit (level_tele arms, place_lregion validation), GNUmakefile declined by precedent, ordering comment added, river NIT taken |
| forest.des | 1 | from-scratch | | | | 7 top-level TERRAIN:random TODOs |
| goblintown.des | 3 | from-scratch | | | | ROUTING NOTE (2026-08-05 analysis, pre-slice): the ugking machinery is SPLIT across slices - the you.h ugking bitfield + mon.c kill setter + trap.c fall-past-minetown clamp are ALREADY ON BRANCH (rode the Phase 2 boss slice 8b4283ba8, NOT the mines slice); master's do.c goto_level stairs/fall gate into Mine Town and Mines' End (the Blind-aware "Access denied, by order of the Goblin King" glyph arm + "mysterious force prevents you from falling", ledger_no tests against minetn_level/mineend_level) is NOT PORTED - branch do.c has zero minetn_level/mineend_level references - and MUST RIDE THIS SLICE's C payload: gating descent before Goblin Town exists would seal the mines with no des-placed King to kill (master's deliberate bypass is levelport only). Prerequisites already in place: the minetn/mineend topology macros landed with the mines slice, so the gate ports cleanly here. ALSO RIDES HERE: sounds.c's three Ingtown-gated peaceful elf/dwarf chatter arms keyed on ugking (Death-to-the-Goblin-King / back-to-mining lines) - In_goblintown does not exist on branch yet; the Goblin:%d describe_level arm stays parked in the Phase 2 remaining-arms row until this topology lands. INTERIM ASYMMETRY on branch today: fall clamp live + stairs gate absent = stairs descent past Mine Town unrestricted while in-mines hole-falls clamp to one level; resolves when this slice lands |
| hdgn.des | 1 | from-scratch | | | | carries `REGION: ... "nursery",filled` - needs the NURSERY rtype, available since the special-room-types slice |
| icequeen.des | 3 | from-scratch | | | | |
| nymph.des | 1 | from-scratch | x | x | | EXEMPLAR (luac-clean; W needs branch content); five `ROOM: "garden"` incl. one `unlit` - rtype available since the special-room-types slice, and mkgarden() deliberately does NOT re-check rlit when called with a room, so the unlit garden converts faithfully |
| purgatory.des | 2 | from-scratch | | | | dead-shopkeeper flag |
| vaults.des | 46 | from-scratch -> THEMERMS ENTRIES (ratified 2026-07-18) | | | | destination = themerms.lua contents functions, NOT standalone vlt-*.lua; vlt-0004/0045 HAND-CONVERTED 2026-07-03 (exemplar/vaults/, luac-clean, nil-swept) stay the map-conversion reference; remaining 44 convert cleanly - polish with the vlt-0045 ROOMDOOR idiom; dedupe overlaps to upstream's rooms; frequency/mindiff per the Phase 2 special-room-types item |
| vecna.des | 3 | from-scratch | | | | |
| Arch..Wizard (13 vanilla quests) | 65 | delta-diff | | | | VAL + ARC + BAR + CAV + HEA + KNI + MON TRACKS DONE 2026-08-07 (7/13). === VAL RECORD === TWO-COMMIT slice per batch-1 precedent: commit 1 = C payload 050c88e34 (MS_NEMESIS treasure-priority strategy arm - the boss-slice deferral, discharged; music.c ART_GJALLAR horn arm ported into 5.0 idiom incl. BUG-1058's status refresh - the artifacts-B DEFER whose owner report file was lost; BUG-1477..1483 re-ports: target_on quest-artifact carrier exemption, quest_chat/quest_talk foreign guards + MS_LEADER arm, questpgr NON_PM + bounded names, sounds guardian NON_PM x2, music set_malign x3, isqldr writer = BATTERY-CAUGHT forfeiture orphan, nemesis SCR_CONSECRATION via re-added mitem2), commit 2 = levels. LEVELS: master delta = 18 lines / 5 hunks in dat/Valkyrie.des, maps byte-identical; Val-strt banded mail +4 (EvilHack wins the upstream-also-changed-it overlap, exemplar ruling) + magic chest (33,10) via des.feature (exemplar TODO discharged - binding live since knox; the exemplar's des.magic_chest() API never existed); Val-loca chaotic altar (29,06) + hill giant shaman ON it replacing the 7th fire giant; Val-goal Gjallar (blessed tooled horn, NO spe, no leading "The") replacing the Orb of Fate line - THE BRANCH-BREAKING FIX: the old line named an artifact whose artilist entry is #if 0'd, producing a plain named crystal ball and an unfinishable Valkyrie quest (BUG-1368 class; that entry's 14-goal sweep ran against MASTER's artilist - RE-RUN the linkage gate per track as each quest slice lands) + 3 salamanders, the fixed pair IN LAVA flanking the drawbridges (likes_lava, master content); Val-fila/filb zero delta, untouched; headers kept upstream-verbatim (reviewer: 157-file precedent; the exemplar's evilhack-50 header idiom NOT adopted); NO build-list work (wildcards carry Val-*) + NO dungeon.lua work (Quest block delta = whitespace only). ALREADY DONE pre-slice (the prerequisite check): quest.lua text merge all 16 roles (2026-07-23), questpgr/quest machinery in 5.0 msgid idiom, magic-chest/forge bindings, ART_GJALLAR + role.c wiring, build lists. NOT this slice (routings): BUG-799 + BUG-806 (harassment-sweep ruling 2026-08-05), pager per-artifact hint table incl. Gjallar's two hints (existing pager artifact-encyclopedia deferral), BUG-1053/1057 (new music.c earthquake row), target_on is_mplayer/peaceful deltas (new wizard.c residual row); BUG-278 verified already on branch (objnam erosion-before-mpickobj, comment intact), BUG-1302 MOOT on branch (5.0 qt_pager picks uniformly over the lua array), BUG-1402 landed earlier. BATTERY (game-tester, 3 games + 1-game targeted re-battery, ALL LEGS PASS, paniclog NEVER CREATED): file-load censuses exact (Norn +4/+4 inventory, magic chest feature locked/unlocks with magic key + full shared-inventory loot menu ON a quest level, plain chest gone; altar AM_CHAOTIC with hostile shaman ON it on file-load AND real path at identical coords; Gjallar oartifact=60 blessed spe=0 under Surtur on BOTH paths across 3 independent creations; salamanders 2-in-lava + 1 random; fila/filb clean); real path portal + telepathic summons + assignquest incl. silver-bell wording; 8 distinct encourage lines from the 11-entry pool over 17 chats; STRAT_GROUND treasure warp (Surtur crossed 27 tiles to the dropped horn in one turn, mstrategy 0x84000010 = STRAT_GROUND|M3_WANTSARTI, horn then in his minvent) + STRAT_PLAYER steal arm; gotit + Loki/Ragnarok nemesis-death text verbatim; Gjallar blow legs (both tone variants, whole-level wake, You_feel + STRAT_WAITMASK clear verified by mstrategy readback, 33 Valkyrie blows zero deafen, non-Valkyrie deafen ~1/10 with SAME-FRAME Deaf status = the disp.botl carry, blessed stagger arm incl. stagger() verb on a brown mold, stun-resistant control 32 blows clean); completion offeredit + FORCED forfeiture (leader_wantsit/gaveitup, bell blessed 0->1, luck +3, adjalign +5) - the re-battery proved Gjallar lands IN the Norn's minvent post-forfeit (pre-fix it sat in NO object chain, BUG-1482) and Surtur is born with Bell otyp 332 + consecration otyp 425 (BUG-1483), both dropped on kill; foreign King Arthur/Ixoth/warrior chats silent-sane with no impossible; bugle set_malign flip (malign -15 -> +15, both soldiers attack) + harp calm legs; save/restore byte-stable incl. quest state + overview "Completed quest for the Norn". BATTERY PREMISE CORRECTIONS: "fixed" on a wished BONE Gjallar sets no oerodeproof (bone is not damageable; the tester's control wish proved it - master-identical, the BUG-278 transfer block itself verified present) and Val-strt/goal carry NO noflip (only loca/fila/filb do) - flips occurred and are harmless, no C-coupled absolute coords on the track. WATCH: one-off panictrace abort in tty_nhgetch on the first keystroke after a gdb batch call+detach (live-attach artifact class, not reproduced, no paniclog; fingerprint = rloc_to call, detach, keystroke); post-forfeit artiexist stays SET while the horn lives in the leader's minvent (master parity - she keeps it); wishing "harp"/"wooden harp" always resolves to the MAGIC harp (frozen-note candidate, master side unverified). === ARC RECORD (2026-08-06) === TWO-COMMIT slice: commit 1 = C payload a68548224, commit 2 = levels + tracker 193c32955. C PAYLOAD (BUG-1484 objnam yname/yobjnam quest-artifact "your" exception lists - carried Gjallar rendered "your Gjallar", six below-boundary artifacts skipped shk_your; ART_MAGIC_8_BALL is the 5.0 ART_##bn spelling, include/onames.h's ART_ block is a stale 3.6 vestige, do not trust it; BUG-1485 dothrow retouch class - branch had ZERO retouch_object() calls (vanilla 5.0 shape), ported master's throw-time gate in throw_obj (ECMD_TIME via the unsplit_stack epilogue, which skips rejoining when retouch nulls obj - composition with 5.0 stack-split machinery verified) + both return-site retouch/artifact_light "begins to shine" blocks; return_throw_to_inv() helper deliberately untouched so the boomerang path keeps master's no-retouch parity; BUG-1486 expert-skill guaranteed return aklys/spear/hammer booleans + catch-message parity range>0 gate + body_part(HAND); AutoReturn comment tail + artilist.h Xiuhcoatl comment re-wrap). LEVELS: master delta = 25 lines / 4 hunks in dat/Arch.des, maps byte-identical (branch Arc-* were pure vanilla = byte-identical to Official_nh50); Arc-strt REGION (24,08,38,12) unlit->lit + magic chest (27,10) via des.feature replacing Carnarvon's chest at (25,10) (ported master's FINAL coord - 08ffd005d moved it off the student square); Carnarvon fedora+5/bullwhip+4 ALREADY UPSTREAM (their kill-your-leader work converged, H2a no-op - do not re-apply); Arc-goal local altar {(26,05),(26,14),(50,05),(50,14)} + shuffle() (Mon-goal.lua idiom; lua altar[1] = .des $altar[0]) carrying des.altar chaos + Xiuhcoatl (atlatl, blessed, spe=1, no leading "The") + Minion of Huhetotl on the shared slot; LINKAGE GATE FIRED AGAIN - second consecutive goal level naming a #if 0'd artifact ("The Orb of Detection"), Arc quest unfinishable since the adoption, the per-track gate is MANDATORY; 3 fixed rolling boulders (30,05)/(30,14)/(46,05) joining (46,14), 3 random traps remain; goal temple region stays PINNED (43,13,50,15) while the altar shuffles 3/4 elsewhere = master's own asymmetry, kept; NO noflip on the track (master parity; both fixed-coord sets are flip-symmetric, no C-coupled coords); fila/filb/loca zero delta, untouched. ALREADY DONE pre-slice (prerequisite check): 11/14 master ART_XIUHCOATL hook sites live incl. role.c questarti, throw_obj wield+dex gates, artifact_hit fire arm, AutoReturn membership; quest.lua Arc 32-key block; build lists (Makefile.top wildcards + Xcode both carry Arc-*). ROUTINGS: hack.c fire_wep/burnhd forcefight-burn -> NEW Phase 2 row; worn.c extra_pref ring rework (Xiuhcoatl arm inside) -> existing extra_pref row; pager arti_specials Xiuhcoatl x3 -> existing pager artifact-encyclopedia deferral; dat/data.base NEVER MERGED (2452-line delta; atlatl/xiuhcoatl/gjallar entries all missing - the Val slice missed gjallar silently) -> NEW Phase 3 row, NO piecemeal entries (ruling 2026-08-06). Archive sweep: zero level-data hits for Arch.des ever; BUG-572/1034/1132/1176/1271 + 1477..1483 verified live on branch; consecration-scroll trap avoided (master 25fa4ee08 deliberately reverted the .des sack+scroll - mitem2/BUG-1483 delivers it, do NOT re-add to the level file). REVIEWER (same day): CONDITIONAL PASS, no HIGH - the MEDIUM (BUG-1486's range>0 gate dead-true, master's underwater turbulence producer dropped by the adoption) fixed same day as BUG-1487 (turbulence block ported verbatim over the vanilla clamp); LOWs fixed (both "must be wielded" block comments gained master's Xiuhcoatl text incl. the inherited must-we typo; tethered impaired-catch no-retouch exemption documented in-line); reviewer verified enum ordering by ARTI_ENUM preprocess (six exceptions < XIUHCOATL < GJALLAR; the preprocess numbered them 50/62 but the LIVE game reads ART_XIUHCOATL=48 / ART_GJALLAR=60 - battery correction, ordering conclusion identical), census exact vs master goal section, RNG-stream parity (rn2 leftmost), and the deliberate no-retouch parity on boomerang + quest-leader toss-back paths; reviewer's adjacent find = launcher range table gap -> NEW Phase 2 row. BATTERY (game-tester, 4 games, ALL LEGS PASS, paniclog NEVER CREATED, sanity_check verified on): Arc-strt census on file-load AND real path (exactly one magic chest typ 30, lit; Carnarvon fedora+5 worn/bullwhip+4; ZERO chest objects on the level; flip back-transform exact on every coord); Arc-goal across 7 creations (6 wizloaddes + 1 real) - altar+Xiuhcoatl+Minion co-located EVERY time, 3 of 4 candidate coords drawn, align chaotic, oname exactly "Xiuhcoatl", oartifact=48, blessed, spe=1, traps 7/4-boulder every creation, temple rtype=10 needfill=2 pinned with no priest (altar inside it 2/7); the artifact is only produced on the FIRST goal creation per game (artiexist rejection after - census lever note); yname legs all four wrappers share the predicate (bare "Gjallar" bash, "your Magic 8-Ball"/"your Dragonbane", shop differential "Ouiatchouane's Dragonbane" vs bare "Gjallar" in the same shop, "Your Xiuhcoatl evades your grasp" via Yobjnam2); throw-time retouch (lycanthropy-staged silver hater: blast damage, turn consumed, saber floorward, nothing flew) + BATTERY PREMISE CORRECTION: the count prefix is a MULTISHOT shotlimit - getobj does NOT split the stack, the whole hated stack drops as one object with stable o_id and no orphan (non-hater control threw exactly 1, kept quan 9); return retouch + artifact_light arm LIVE (Hammer of the Gods at skilled P_HAMMER = AutoReturn + artifact_light: "stops shining"/"returns to your hand"/"begins to shine brightly" exactly once per catch) + polyself body_part "returns to your claw!" as a troll; expert-spear guaranteed return 110/110 with zero failures (logging breakpoints on all four block exits), basic control 342 throws = inner-fail 4 (1.17%) + outer-fail 1 (0.29%) with the verbatim feet-landing line and a 9-tile-downrange drop; turbulence 40/50 (80.0%) suppressed with NO return message = the silent range-0 catch, non-returning "It drifts down to your feet." 6/8; gates: unwielded refusal consumes no turn, non-Arch Dx15 refusal WITH turn (Valkyrie game, artilist role/align defeated in process memory only), non-Arch Dx21 and Arc Dx18 both throw; REAL QUEST CHAIN: telepathic summons + portal to dnum 3, assignquest text names Xiuhcoatl throughout, the Minion is BORN carrying Bell otyp 332 + consecration otyp 425 (mitem/mitem2 = BUG-1483; the level file places neither) and drops both on kill, gotit, offeredit "[Lord Carnarvon instructs you to guard Xiuhcoatl from now on.]", bell blessed 0->1; FORCED forfeiture (abuse=50 lever): the atlatl moves o_id-stable into Carnarvon's minvent with isqldr=1 (BUG-1482 proof), uhave.questart 1->0, luck +3, adjalign(5) clamped by ALIGNLIM at T:74 (clamp, not a missing effect); save/restore x2 STATE-CLEAN (every quest field round-trips, no refusal, no paniclog) but NOT byte-stable - recorded as a MEASUREMENT not a verdict (a 2-byte field at offset 163 plus 3-byte-stride churn; RNG/display memory make byte identity not obviously expected and there is no master baseline; the Val row's "byte-stable" claim may have compared differently) - WATCH. Battery anomalies recorded: wizloaddes Arc-goal onto Dlvl 1 loses the up-stair (known Dlvl-1 lever artifact; the real level has STAIRS at the right cell); init_artifacts() rewrites questarti + Hammer of the Gods alignment to the hero's at game start (a gdb alignment flip makes the hero's own quest artifact blast on retouch - cost one aborted batch); wished quest/NOWISH artifacts DO blast on wish delivery via the pickup/hold path for cross-aligned heroes (gameplay rule-of-thumb corrected) and prompt "Deal with previous owner? [yn]"; #wizkill of the nemesis leaves met_nemesis=0 so the leader chat sets the cheater flag (staging artifact, not a bug). === BAR RECORD (2026-08-06) === TWO-COMMIT slice: commit 1 = C payload e0cd04276, commit 2 = levels + tracker 068a97945. FILENAME GOTCHA: the .des is dat/Barb.des in BOTH base and master (vanilla's own name) - diffing dat/Bar.des silently returns EMPTY, not an error; check `git ls-tree` when a track's delta looks like zero. MASTER ENUM GOTCHA: master = ART_RING_OF_P_HUL (makedefs apostrophe mangling), branch = ART_RING_OF_PHUL. C payload commit 1 (BUG-1488 objnam xname_flags + obj_typename artifact-base-ring naming - the branch rendered "ring of ancient"; covers RIN_LUSTROUS in the same master expressions; BUG-1489 dosinkring RIN_ANCIENT/RIN_LUSTROUS case labels + zap.c destroy_item P'hul comment restore + artilist.h P'hul comment re-wrap preserving master's "artiafcts" typo). PREREQUISITE CHECK: artilist A() entry field-verified, role.c questarti in BOTH roles[] and draugr_roles[] (Duali Tribe override), readobjnam fallback, ALL generic DFNS(AD_MAGM)/CARY(AD_DISE) plumbing, quest.lua Bar 31 keys all-%o, eat.c sleep-res arm, build lists - all live; KEEP the branch's make_sick(0L,..,SICK_ALL) AD_DISE cure (5.0 improvement over master's raw Sick=0L, do NOT regress). LEVELS commit 2: master delta 55/51 across 6 hunks - FIRST track with MAP-BODY rewrites: Bar-strt + Bar-loca maps replaced with master's blocks byte-exact via script (w->p PUDDLE transliteration - the only change; 'a' SAND and 'g' GRASS carry; these are the tree's FIRST lua maps using 'a'; loca trailing-space padding and the row-15 stray in-water 'a' preserved verbatim; zero leftover 'w' verified - leftover 'w' would parse as MATCH_WALL silently); Bar-strt gains the leading 100% '.'->'g' replace_terrain over (37,00,75,19) + the three tree conversions re-target fromterrain="g" + randline path/portal-spot paint "g"; chest -> des.feature("magic chest",09,05) (same coord, verified floor in Pelias's north alcove); Pelias runesword+5/chain-mail+5 ALREADY UPSTREAM (second leader-kit convergence after Carnarvon; no keep_default_invent = master parity - master's kit REPLACED the default and 5.0's sp_lev discard-guard makes that explicit); Bar-goal LINKAGE GATE FIRED (THIRD consecutive track): "The Heart of Ahriman" luckstone (#if 0'd artilist entry) -> des.object id="ancient" (unique name verified) "The Ring of P'hul" blessed spe=0 at the fixed noncoaligned altar (63,04) - no shuffle this track; Thoth Amon stays inventory-free (25fa4ee08/mitem2 trap avoided); Bar-fila solidfill fg=" "->"a" + mines bg="."->"a" (sand-walled filler); Bar-filb zero delta; noflip: master adds NONE - fila/filb noflip is 5.0-upstream machinery, kept; strt/loca get none (not in the delta, no C-coupled coords). ROUTINGS: dogmove could_use_item / muse searches_for_item / worn find_best_armor_recurse + extra_pref RIN_ANCIENT arms -> the existing monster-item-use/extra_pref Phase 2 rows (branch files carry zero RIN_ refs). Archive sweep: zero level-data entries ever for Barb.des; BUG-1034/1367 + the BUG-1477..1487 cluster verified live, nothing new needed. REVIEWER (same day): CONDITIONAL PASS - ONE HIGH, fixed same day: the map replacement broke upstream's siege-ogre floodfill (seed (37,7) became SAND; selection.floodfill matches the typ under the seed, so the (40,03)-(45,20) intersection came up empty in 92.8% of the reviewer's 200k Monte-Carlo trials and rndcoord's (-1,-1) scattered all 12 ogres randomly - the two-correct-slices-compose-wrong shape). FIX = master parity: the floodfill loop replaced with master's ELEVEN fixed-coordinate hostile ogres (Barb.des:95-105; upstream's loop also rolled 12 vs master's 11 - count parity restored too). The same fragility exists in VANILLA 5.0 (~4.3% via the 5% tree band converting the seed) -> logged in the Upstream-reportable table 2026-08-06. Reviewer verified everything else clean: both map transliterations byte-exact vs master with w->p (programmatic row compare incl. loca trailing pads + both stray in-water 'a' cells), eels on P cells, all C hunks expression-exact vs master (objnam master site-set = exactly the two covered), "ancient" resolves uniquely via find_objtype strcmpi, fila two-init shape = Val-fila precedent, no >80-col lines. BATTERY (game-tester, 2 games, ALL LEGS PASS, paniclog NEVER CREATED, sanity_check on): Bar-strt censused across NINE independent creations (8 wizloaddes + 1 real; flips v/none/h+v all drawn) - SAND typ 38 live (~520 cells; the tree's FIRST 'a' lua maps PROVEN), puddle banks 37-38 cells flanking the POOL run, 100% grass conversion east (zero unconverted '.'; the single ROOM cell east of x=37 is the spiked-pit tile - des.trap converts its host cell to ROOM, anomaly note), tree bands graded 4.7%/8.9%/21.5% means over 9 draws; SIEGE OGRES 11/11 massed at the fixed source block in ALL NINE creations, zero scatter; magic chest typ 30 at source (9,5) both paths, ZERO chest objects; Pelias runesword+5/chain-mail+5 control. Bar-loca: sand/puddle/grass-island censuses exact both paths, single TREE at source (13,11). Bar-goal: noncoaligned altar + Ring of P'hul (otyp 240, live oartifact=62, blessed spe=0) + Thoth Amon co-located at source (63,04); artifact minted only on first creation per game (artiexist, known lever). Bar-fila real+file-load: mines-carve over SAND fill (typ 38 ~1042-1156 cells, STONE only the x=0 column). BUG-1488 legs: wish/inventory/obj_typename/simple_typename/xname all render "ancient ring"/"lustrous ring"; PREMISE CORRECTION - both artifact-base rings have NO random appearance (oc_descr NULL), so they NEVER appear in discoveries and even name_known=0 renders the true name; only !dknown gives bare "ring" (by design, not a miss). BUG-1489 legs verbatim both rings. P'hul properties: carried W_ART=4096 sick-res, worn adds W_RINGL antimagic, drop clears; Sick cure on pickup ("You feel cured.  What a relief!") = the kept make_sick(SICK_ALL) branch. REAL QUEST CHAIN: portal at source (62,02) branch spot; leader alignment-gate expulsion then assignquest with %o="the Ring of P'hul" throughout; Thoth Amon BORN with Bell spe=3 + consecration + confusion potion (mitem/mitem2; level file places none), all dropped o_id-stable on kill; gotit; offeredit -> got_thanks=1 qcompleted=1 bell blessed 0->1 (PREMISE CORRECTION: q_score.offered_artifact is never written anywhere in src/ - the observable is text + got_thanks/qcompleted); FORCED forfeiture (abuse=60) -> ring o_id 276 STABLE from altar through nemesis minvent, death drop, hero invent, save/restore and into Pelias minvent isqldr=1, luck +3, adjalign clamped; save/restore state-clean (11-item inventory byte-identical incl. o_ids). STAGING DISCLOSURE: Thoth Amon melee kill BLOCKED (covetous self-teleport + summon storm at AC -2; 3-attempt budget) - killed via #wizkill with gdb repair of met_nemesis/cheater before the return leg. WATCH: ONE unreproduced wizloaddes Bar-goal creation read altarmask=3 (AM_CHAOTIC|AM_NEUTRAL - not a value sp_amask_to_amask emits for noncoaligned; reloads gave 4 and 2, real path gave 4; recorded verbatim, no diagnosis). Battery anomalies recorded: master-parity items ("%o" at sentence start renders lowercase "the" - master quest.txt identical all roles; randline grass path can bridge the river - byte-identical op in master); quest ^V takes DLEVEL not absolute depth; ';' farlook not bound in this build's map (use '_' getpos). NOTE: the battery report attributes enum 62 to onames.h:76 ART_RING_OF_P_HUL - include/onames.h remains the STALE 3.6 vestige (Arch-slice ruling); the live compile symbol is the bn-generated ART_RING_OF_PHUL and the live value 62 was read from the object, which is what matters. === CAV RECORD (2026-08-06) === ONE-COMMIT slice (levels + tracker) - FIRST track with ZERO C payload: the prerequisite check found EVERY master hook already live (Annam monsters.h field-exact incl. MS_NEMESIS; Shaman Karnov MS_LEADER; Sceptre of Might artilist A() entry LIVE with PM_CAVE_DWELLER - EvilHack kept vanilla's artifact NAME and only rebased the otyp onto ROD, so the #if 0 linkage trap of Val/Arc/Bar cannot arise; role.c names+enums exact; objnam.c Keolewa/Dragonbane/Sceptre -> PM_CAVE_DWELLER grouping; quest.lua Cav block; bugbear S_ORC + cave bear from Phase 1; score_artifact_weapon comment-only divergence). LINKAGE GATE PASSED PRE-SLICE (first track to): Cav-goal id="rod" + Annam landed with BUG-1367/1368 (2026-07-25); the gate re-ran anyway and the battery re-verified oartifact=49 live (the live enum happens to equal the onames.h vestige this time - still trust only live reads). LEVELS: master delta 80+/61- in dat/Caveman.des (FILENAME check passed - Caveman.des in BOTH trees, vanilla's own name); Cav-strt map REPLACED with master's block byte-exact (9 changed rows, charset " .BS" - NO transliteration this track; script splice + re-extract cmp); Karnov kit "leather armor" -> "armor" +5 (the EvilHack plain ARMOR object; upstream's leader-kit convergence had used vanilla leather armor; find_objtype's exact-oc_name pass beats 5.0's "generic armor" entry) with club +5 kept, kit REPLACES default (Pelias/Carnarvon precedent); chest (34,02) -> des.feature("magic chest",40,03) (floor on the new map); Cav-loca 4th random bugbear -> cave bear + class h->o + last 2 random hill giants -> 2 cave bears + trailing goal-header comment Tiamat->Annam x2 (body content, not the upstream-verbatim header); Cav-goal +3 boulders after the Sceptre + 12 corpses master-ordered (caveman/cavewoman pairs via gendered montype -> CORPSTAT_MALE/FEMALE natively, + neanderthal x8); Cav-fila 5th bugbear -> cave bear + h->o; Cav-filb 4th bugbear -> cave bear + h->o x2; monster/object ORDER master-exact everywhere (RNG-stream parity); noflip: master adds none, fila/filb keep upstream's. Archive sweep clean (only BUG-1034 touches track machinery, re-ported session 9); data.base Annam entry -> the existing data.base full-delta row. REVIEWER: PASS with ZERO findings at any severity (batch-2 first) - map byte-identical incl. trailing pads, all 28 fixed-coordinate consumers of the new map re-verified (Bar compose lesson), Official_nh50 cross-diff = branch diverges in exactly the master-delta items. BATTERY (game-tester, 2 games, ALL LEGS PASS, paniclog NEVER CAME INTO EXISTENCE, sanity_check on): Cav-strt census file-load AND real path (Karnov kit exactly 2 items, otyp 159 armor +5 / club +5, no default gear; ONE magic chest typ 30 at source (40,03) with nmagicchests=1, ZERO chest objects; 12/12 siege bugbears + 8/8 neanderthals + shrine altar AM_SHRINE with temple priest intoning; map splice proven mechanically - all 646 source floor cells + 12 B cells resolve, missing=0, spot-check rows verified in tree AND installed nhdat); Cav-goal across 3 creations (rod otyp 86 oart=49 blessed spe=0 co-located with sleeping Annam at source (23,10) under none/X/X+Y flips; 3 boulders; corpse census exact every time = neanderthal x8 + cave dweller x4 with CORPSTAT spe 2/1/2/1 rendering "caveman corpse"/"cavewoman corpse"; 3 shriekers at fixed coords every creation); Cav-loca 27/27 staged monsters incl. all 13 fixed bugbear + 3 fixed hill giant coords under X+Y flip, locked SDOOR at source (28,11), both stairs exact, hardfloor, no altar; fila 7/7 + filb 8/8 mines-carve censuses; REAL QUEST CHAIN: portal entry x2 + telepathic summons + firsttime arrival text all %-substituted; badlevel expulsion at XL13 AND badalign expulsion (both paths exercised) then assignquest with %o="the Sceptre of Might" throughout; Annam BORN with Bell otyp 332 spe=3 + consecration otyp 425 (mitem/mitem2, level file places neither - BUG-1483 4th-track proof; also present on file-load, so not qstart-gated); covetous warp x2 then DISCLOSED #wizkill + gdb met_nemesis repair (cheater=0 confirmed post-chat); gotit on pickup with o_id=380 STABLE from creation; offeredit completion (got_thanks/qcompleted=1, Bell blessed 0->1, Karnov tells the hero to KEEP the Sceptre - master text); FORCED forfeiture (abuse=60) -> Sceptre o_id=380 STABLE into Karnov's minvent isqldr=1, luck +3, adjalign(5) ALIGNLIM-clamped at T:58 (BUG-1482 proof, 4th track); save/restore STATE-CLEAN (quest_status + uevent byte-identical across the round trip, inventory o_ids ordered-identical, Karnov minvent identical). BATTERY PREMISE CORRECTIONS: the plain ARMOR object ALWAYS renders with a material prefix ("cloth armor"/"leather armor" - objnam force_material_name includes ARMOR; bare "armor" never renders, and id="armor" is the only .lua spelling matching oc_name); repeat goal creations after artiexist yield an UNNAMED plain rod (oextra nil - the name drops with the artifact link, not "a named rod"). Anomalies recorded: Cav-strt wizloaddes object piles = stock mktrap_victim() trap-victim noise off the level's 6 traps (real path TOTALOBJ 0 - census math must not assume file-declared counts); 9 deterministic gap-fill ROOM cells vs the map text (diagonal-junction bridging, identical set flipped/unflipped, no declared cell lost; goal 0, loca 1); the known wizloaddes-on-Dlvl-1 up-stair artifact (real path exact); NEW WATCH - stun silently aborts #chat (direction prompt accepted then no message and no time consumed, getdir ECMD_CANCEL; reproduced x3, clears with STUNNED unset; undiagnosed); Cav-loca still ends with the goal-level comment block (upstream file-split artifact, names updated only); artifact-sighting livelog fires on a wizloaddes file-load (cosmetic, wizmode-only). === HEA RECORD (2026-08-06) === ONE-COMMIT slice - SECOND consecutive zero-C-payload track: prerequisite check found everything live (enormous rat + rodent of unusual size monsters.h field-exact vs master; Hippocrates MS_LEADER; Cyclops MS_NEMESIS; Staff of Aesculapius artilist LIVE on QUARTERSTAFF - vanilla artifact both trees; role.c parity modulo the NAMS merge; objnam PM_HEALER case; quest.lua Hea block). LINKAGE GATE PASSED PRE-SLICE (second track to): Hea-goal already places id="quarterstaff" named correctly; battery read oartifact=51 live. FILENAME check passed (Healer.des both trees). LEVELS: master delta 53+/47- DOMINATED BY THREE MAP-BODY REWRITES (strt/loca/goal) adding 'w' shallow-water banks around the pools - the Bar w->p PUDDLE transliteration applied to all three (script splice; byte-verified by p->w round-trip cmp vs master blocks incl. trailing pads; zero leftover 'w'; charsets strt "+-.PS\p|" loca "+-.PSp|" goal ".Pp"; the strt throne '\' carried); Hippocrates kit "silver dagger"->"dagger" +5 (SECOND EvilHack-wins kit overlap after Caveman - upstream's leader-kit convergence picked silver, master's delta is plain dagger; Val banded-mail ruling); chest (37,10) -> des.feature("magic chest",31,09) (temple-interior floor verified); rats: strt last 2 of 10 rabid -> 2 enormous, loca last 2 of 8 -> 2 enormous, goal +2 rodent of unusual size after the 3 rabid, fila +1 enormous, filb +1 enormous - all master-ordered (RNG-stream parity); rosters otherwise base-identical (an early "upstream roster divergence" read was a diff-context misread - base really has 10 rabid + D x5/S x5 on strt). Archive sweep: ZERO hits. data.base rat entries -> the existing data.base row. REVIEWER: PASS, no actionable findings - all three transliterations byte-exact, all fixed coords re-verified against the new maps incl. all TWELVE strt doors on '+'/'S' cells (the brief had said 5 - reviewer corrected), goal up-stair (39,10) lands on a puddle cell = FAITHFUL master parity (master's own 'w' there; stair creation overwrites terrain in both trees), no floodfill/terrain-keyed selections anywhere (Bar class absent), merge-base Healer.des proven byte-identical to vanilla 3.6.7's (the delta is the COMPLETE EvilHack divergence), Official_nh50 cross-diff exactly bounded. BATTERY (game-tester, 2 games, ALL 5 LEGS PASS, paniclog NEVER EXISTED, sanity_check on): PUDDLE censuses EXACT on every creation - 181 strt / 20 loca / 89 goal-minus-stair = master's 'w' counts byte-for-byte (the tester recounted master directly, correcting the brief's 141), full-grid cell-by-cell comparisons 0 mismatches, goal stair cell typ 27 over the 89th puddle; Hippocrates ON the throne with EXACTLY one base-DAGGER otyp 36 spe=5 (live read - not silver); magic chest typ 30 at source, zero CHEST objects; 8/8 attendants; strt altar flags=2 plain neutral NO shrine/priest vs loca shrine flags=9 AM_SHRINE|AM_CHAOTIC with aligned cleric (BATTERY PREMISE CORRECTION - the brief had put the shrine on strt); rat/aquatic/D/S censuses exact all five files; Staff otyp=93 arti=51 blessed spe=0 + wand of lightning + HOSTILE AWAKE Cyclops co-located at source (20,06) on every creation, artiexist lever reconfirmed (repeat creation = arti=0 UNNAMED, the Cav correction holds); Cyclops BORN with Bell otyp 332 spe=3 + consecration otyp 425 on ALL THREE goal creations (BUG-1483 5th-track proof); REAL CHAIN: summons/portal/arrival + badlevel expulsion at XL13 w/ "Medicus ossium" rank substitution + return text + assignquest at align 20 (%o="the Staff of Aesculapius" throughout) + locate first_locate + WANTSARTI grab ("picks up.. wields") + covetous warp -> DISCLOSED #wizkill + met_nemesis repair + gotit verbatim + completion (got_thanks/qcompleted=1, Bell blessed 0->1) + FORCED forfeiture (abuse=60): Staff o_id=838 STABLE creation -> hero -> Hippocrates minvent isqldr=1, luck +3, adjalign ALIGNLIM-clamped (BUG-1482 5th-track proof); save/restore STATE-CLEAN (quest_status 8 bytes + uevent 16 bytes + all o_ids byte-identical). Anomalies recorded: wizloaddes-only wall->ROOM at 2 secret-door-flanking cells (real path = CROSSWALL 112/112 exact; wallification artifact, map block byte-unchanged by the port); NEW OBSERVATION - pit-family traps (PIT/SPIKED_PIT) convert their host PUDDLE cell to ROOM typ_under=26 while non-pit traps on puddles do not (both paths, at most the 6 random des.trap() per level; undiagnosed, likely master-shared - candidate for the damp-terrain sweep); forfeiture thank-you renders lowercase "the Staff.." at sentence start (the known %o-at-sentence-start master-parity class from the Bar battery); fila drew 3 traps for 4 des.trap() once (placement collision, not systematic); one undeclared giant spider on real-path loca (random spawn, not chased); kelp fronds + random fill = normal water-level flora, not file content. === KNI RECORD === TWO-COMMIT slice: commit 1 = C payload 9338a9515, commit 2 = levels+tracker 3f34f90ec (this hash line edited post-commit, rides the next commit). PREREQ CHECK: makemon Arthur C kit / bribe-lords aasimar clause / demon_talk / wish-opposition / Ixoth+page+quasit monsters / quest.lua Kni block ALL LIVE; LINKAGE GATE: BOTH placements passed PRE-SLICE (the memory's 'Kni x2' = Excalibur in strt Arthur kit + Mirror in goal; Excalibur LONG_SWORD and Mirror MIRROR artilist entries both live, no #if 0 trap - second track after Cav where the gate cannot fire). C PAYLOAD (full same-function parity ruling, AskUserQuestion 2026-08-06): pray.c gcrownu FULL crowning parity - Wizard class_gift restored to SPE_POWER_WORD_KILL + Druid FINGER_OF_DEATH arm (branch had Wizard on the Druid gift, Druid arm absent, both prereqs live), Cleric/Cave-dweller carve-outs + Mjollnir/Keolewa/Giantslayer transform-and-gift arms in lawful/neutral/chaotic blocks, artitouch on transform, unrestrict guards, discover clusters; branch extras KEPT (transform/bestow livelogs, ONAME_GIFT|ONAME_KNOW_ARTI); frac_altar crack NOT ported (subsystem absent -> new Phase 2 row). fountain.c dipfountain master semantics: Knight-only 1/6 success (branch had vanilla-5.0 any-lawful 1/30), HELM_OF_OPPOSITE_ALIGNMENT gate, 1-in-3 hand-confiscation (useup/delobj + fountain gone + luck -1 + removal livelog), oprops wipe, spe floor, artitouch, maybe_erodeproof both paths. artifact.c: touch_artifact artitouch in yours arm + is_demon arm (demons-unaligned prereqs verified live) + ART_WAND_OF_ORCUS blast clause + GoP mismatch damage + wizmode Override restored via y_n (upstream dropped it); hack_artifacts Dirge NON_PM line (no-op parity - static role NON_PM both trees); SPFX_REFLECT block -> master 4-arm shape (mirror any-mask = W_ART carried via cspfx, bows W_WEP, Dragonbane/GoP W_ARMG) + reflection conduct writes (counter field + xlog token reader already live). muse.c mon_reflects FULL parity: youmonst->ureflects early-return (BUG-1292 re-port - branch W_AMUL read made the which_armor impossible reachable again), m_carrying_arti_reflecting staticfn helper (BUG-347 re-port), corrected artifact-glove arms (NEW MASTER BUG: master muse.c compares orefl->otyp against ART_DRAGONBANE/ART_GAUNTLETS_OF_PURITY and calls makeknown(ART_) - otyp vs artifact-index space, arms dead in master; branch ports oartifact== with no makeknown per the Ithilmar-arm pattern), dragon-scaled W_ARM arm (Is_dragon_scaled_armor silver/chromatic), W_ARMC scales arm, naked arm PM_CHROMATIC_DRAGON -> PM_TIAMAT (the chromatic entry is the G_NOGEN scales placeholder in BOTH trees); ureflects +W_ARMG/W_ARMC/W_ART arms (branch idiom kept: no monstseesu inside, callers own it). worn.c which_armor hero W_AMUL/W_RINGL/W_RINGR/W_TOOL map (BUG-1292 layer 1). LEVELS: master delta 35+/35- 3 hunks; Kni-strt knight guards -> page x4 peaceful (in-place, RNG order kept), chest (09,07) -> magic chest (07,07) via des.feature, Arthur des-kit REMOVED (upstream-only Excalibur +4 + plate mail +4 closure replaced default inventory and collided with master's live C kit - makemon Excalibur rnd(4)+1 + large shield is the single source now), warhorse loop REMOVED (upstream-only free mount, maintainer ruling = master parity); Kni-loca/goal map bodies spliced byte-exact from master with w->p transliteration, round-trip cmp EXACT, 57 loca / 49 goal puddle cells, zero leftover w (goal stair (03,08) and the 3 fixed spiked pits sit on plain floor - no overwrite subtleties this track); Kni-fila/filb zero delta, untouched, upstream noflip kept. ROUTED: Dirge sacrifice gift (master pray.c ~2122) + mthrowu Dirge multishot message -> DKnight track row; pager Excalibur hints x2 -> existing artifact-encyclopedia deferral; conduct display/xlog remainder -> NEW consolidated conduct-wiring Phase 2 row; crowning altar-crack -> NEW frac_altar Phase 2 row. REVIEWER: CONDITIONAL PASS, no HIGH; both MEDIUMs fixed same day: (1) gcrownu gift arms dropped master's BUG-295 dropy-destruction nulling (5.0 dropy is void) - new dropped_gift() staticfn infers the outcome from obj->where, applied at all 8 gcrownu dropy sites incl. the pre-existing spellbook and draugr-trident arms (BUG-1490 rider), plus the reviewer's cross-file siblings sit.c egg-lay and eat.c spit-out stackobj-after-dropy guards (BUG-1498, vanilla-5.0-inherited, Upstream-reportable row added); (2) reflected gazes never recorded M_SEEN_REFL - mhitu Medusa gaze + uhitm floating-eye sites now record on reflect in branch call-site idiom (BUG-1499). Reviewer LOWs: fountain rn2-before-quan seeded-stream corner kept (5.0 clause order, gameplay identical); Draugr Monk GAUNTLETS_OF_PROTECTION class_gift is a whitelist entry with no dispatch in BOTH trees (BUG-1497, frozen-unfixed master) -> fix candidate routed to the Monk track. BATTERY (game-tester, 12 games, ALL LEGS PASS, paniclog NEVER CAME INTO EXISTENCE, sanity_check on): strt census x2 real creations - 4 peaceful watchroom pages + 6 audience, zero knights/warhorses/plate-mail/chest-objects, magic chest typ 30 nmagicchests=1 at des (07,07), Arthur BORN with Excalibur arti=1 blessed spe 3-4 ero 0/0 ep=1 + large shield worn, artiexist rndm=1 exactly once (Excalibur is the makemon kit, not lvldef); puddle censuses EXACT 57 loca / 49 goal every creation incl. file-load, goal POOL=56 = the literal P count, zero overwrite deductions; REAL CHAIN: assignquest %o 'the Magic Mirror of Merlin', Ixoth BORN with Bell spe=3 + consecration x3 samples incl. file-load (BUG-1483 6th-track proof), forfeiture mirror o_id STABLE into Arthur minvent isqldr=1 + luck +3 + ALIGNLIM clamp + W_ART reflection revoked on transfer + Bell uncursed (BUG-1482 6th proof), save/restore byte-identical quest_status/uevent + ordered o_id/m_id lists; CROWNING MATRIX 7 rows PASS (Val sword->Excalibur w/ artitouch, Cleric carve-out x3 incl. hammer->Mjollnir and NO transform on a wielded long sword, chaotic Cleric Mjollnir + 'take lives' + P_BROAD_SWORD stays restricted, Cave-dweller spear->Giantslayer, giant club->Keolewa, Wizard POWER WORD KILL, Druid FINGER OF DEATH + unrestrict proof); dropped_gift destruction leg PASS via OPEN AIR - PREMISE CORRECTIONS: lava cannot destroy an iron sword (lava_damage material gate), so open-air delobj is the destruction staging; and 'You feel unworthy.' IS printed post-fix (= master parity; the brief had it backwards); the generic ok_wep tail still unrestricts whatever a Cleric happens to WIELD (master-identical, carve-out only guards the class skill); FOUNTAIN: Knight success full field check (blessed, spe clamp -3->0, ero reset, ep=1, oprops wiped, viadip=1, artitouch, fountain->ROOM) + Samurai ~40 dips NEVER Excalibur with both mist arms observed (curse arm strips erodeproof = the maybe_erodeproof(0) proof; confiscation x3, luck -1 each) + HOOA 29 dips zero events (p~0.005) then success right after removal; REFLECTION: carried-mirror monster arm ('reflects from King Arthur's mirror'), Dragonbane-worn monster 'gloves' + hero ureflects 'gloves', mon_reflects(&youmonst,0) tracks Reflecting 1/0 with NO which_armor impossible, Medusa gaze sets M_SEEN_REFL 0x100 (BUG-1499 proof), reflection conduct 0->1 mirror pickup W_ART -> 2 on Dragonbane wear. Battery anomalies: quest.lua Kni gaveitup '%o'-at-sentence-start lowercase (known master-parity class, quest.lua untouched this slice); random goal des.object() slots can mint artifacts (expected mkobj); kelp frond +1 water-cell object; strt fortress unreachable from the portal except the locked+trapped top doors (as-written in both trees). === MON RECORD === TWO-COMMIT slice 2026-08-07 (commit 1 = C payload BUG-1497/1500..1504, commit 2 = levels+tracker; hashes ride the next commit). GATES ALL PASSED PRE-SLICE (first track to pass every gate): FILENAME dat/Monk.des in both trees; linkage gate = Eyes of the Overworld LIVE in artilist + correctly named in Mon-goal.lua (live oartifact=53 battery-read, onames.h untrusted as always); quest.lua Mon block + monsters.h Po/Kaen field-identical (audit); per the Kni lesson the work hid in the C-hook audit. C PAYLOAD (mhitu.c/uhitm.c/pray.c): BUG-1500 gazemu AD_STON master parity - LIVE GAP, beholder AND Tal'Gath gaze AD_STON on the branch but the arm was vanilla Medusa-only, so their gaze was INSTANT DEATH instead of make_stoned(5L) slow stoning (Eyes partial-protection d(4,6) arm, PETRIFIED_ENT polymon alternative, dist2>8 reflect arm, !rn2(12) reflect-mercy, Medusa Eyes flavor line; return 1 -> M_ATTK_HIT reviewer-verified); BUG-1501 AD_CONF/AD_STUN Eyes arms (local d(3,4)/d(2,6) rolls kept - dice verified identical to master's attack-dice reuse in both trees); BUG-1502 AD_SLOW full rewrite - also fixes VANILLA 5.0's wrong-subject defended(mtmp) (attacker consulted for the HERO's slow-defense; upstream-reportable, fixed via defended(&gy.youmonst)); BUG-1503 passive() floating-eye Eyes arm; BUG-1504 reviewer MEDIUM - cancelled-Medusa "doesn't look all that ugly" was dead code (constant-false !react, vanilla-5.0-inherited, upstream-reportable); BUG-1497 FIX-FORWARD (AskUserQuestion ruling) - crowned Draugr Monk's GAUNTLETS_OF_PROTECTION materialization arm (ARMOR_CLASS sibling of the SPBOOK_CLASS block, negative-spe clamp; only reachable by that combo, alignment-switch guards verified). SCOPE RULINGS (AskUserQuestion x4): siege floodfill -> master fixed coords; KEEP upstream's Po +6 robe des-kit AND the strt spinach tins/food rations (FIRST deliberate upstream-only content adoption in batch 2 - record: master gives Po no kit at all); FULL same-function gazemu parity; BUG-1497 rides. LEVELS: strt map spliced byte-exact (outer field '.' -> 'g' GRASS, round-trip proven; 1008 g-cells: 220/220 side regions + 568 mid), replace_terrain fromterrain 'g' x2, magic chest (26,09), fixed dart traps (20,09)/(20,10) + 12 fixed siege coords replacing upstream's floodfill (the flood seeds at (05,04) INSIDE the 10% tree band = the Bar fragility class at ~10% odds in vanilla, upstream-reportable); goal Kaen+altar moved above the objects (master order) + 2 hostile salamanders; loca/fila/filb zero delta untouched. NEW Phase 2 row minted: Slow-status ladder (u_slow_down HSlow timeout + youprop !Slow gating - battery leg D6 proved the branch callee is Fast-strip-only; the in-file youprop.h TODO-EVILHACK already flagged the gating half). BATTERY (game-tester, 4 games, ALL LEGS PASS, paniclog NEVER CAME INTO EXISTENCE, sanity_check on): strt census exact x4 incl. both-axis flip (G+T 219/220 regions, mid 568/0, zero tree bleed, portal cell ROOM), magic chest typ 30 nmagicchests=1 zero chest objects, 2 fixed darts + 12 siege coords ZERO scatter, Po mnum=517 isqldr=1 +6 robe + leader dialogue, tins/rations placed; goal x10+real: Kaen+altar+Eyes co-located BOTH $place values, Kaen born w/ Bell+consecration on BOTH paths (BUG-1483 7th-track proof), 9+9+2 census w/ salamanders peaceful=0; REAL CHAIN: assignquest names the artifact, covetous Kaen STOLE the Eyes and migrated off-level (staging note: goal-level kill non-deterministic), kill -> essence pickup -> offeredit (retain, master text) AND forced forfeiture (abuse=50): o_id=132 STABLE into Po's minvent isqldr=1, luck +3, adjalign ALIGNLIM-clamped (BUG-1482 7th proof), bell uncursed, save/restore state-clean x2; GAZE LEGS: beholder slow-stoning + "unaffected" w/ Stone_res, Eyes partial-protect (14 dmg in d(4,6), Stoned never set, double-space string confirmed rendered), Medusa Eyes flavor -> death, CANCELLED Medusa prints the resurrected line x5 (BUG-1504 live), reflect 1/12 kill + shields/covers both variants + dist2>8 arm, AD_SLOW fires on non-Fast hero + Eyes 13/13 block (3 msgs ~1/4) + blue-dragon resists_slow immune, umber hulk/magical eye blocked w/ zero Confusion/Stun, floating-eye passive 10/12 protect zero paralysis vs control frozen; CROWNING: draugr Monk gets blessed +3 gauntlets of protection at feet (u.ugifts 0->1, livelog LL 72, NO artifact double-gift, NO unworthy), literate control spellbook path unchanged. PREMISE CORRECTIONS (tester): Stone-res message renders "the beholder's gaze" not "its gaze"; random traps can duplicate the dart type (only the FIXED coords are deterministic; real path adds the ttyp=19 branch portal); offeredit/forfeiture mutually exclusive per chat (staged via abuse 0/50); quest branch rolled 6 levels (base 5 range 2); u_slow_down on a non-Fast hero = message only (-> the new Slow-ladder row). WATCH/observations: des spinach tins carry a live random corpsenm (spe=1 keeps them spinach; sp_lev sets NON_PM but a random tin corpsenm survives - likely vanilla-5.0-shared, harmless under the BUG-591-class guards, dogfood() would read the random flesh); 6/8 des elementals spawned peaceful for the neutral Monk on the real path only (unflagged .des lines + peace_minded, master-shared shape, xorns all hostile); goal random objects mostly land on lava (objlist empty by T:41). Reviewer: CONDITIONAL PASS, no HIGH; MEDIUM (BUG-1504) + the one 83-col line fixed same day; explmu defended(mtmp) exploder-gear read noted as master-origin shared code -> master-bugs memory. NEXT TRACKS: 6 remain (Pri..Wiz; Priest next alphabetically) - re-diff the track's .des (verify the FILENAME first) vs 44b6612fd, apply onto upstream's lua, re-run the artifact-linkage gate on the goal level, archive-sweep the track's C hooks before planning; Kni/Mon lesson: when gates pass, audit the C hooks - and check leader des-kit vs C-kit AND callee bodies (u_slow_down class: a ported call site can assume master semantics its callee lacks) |
| Convict.des | 5 | from-scratch | | | | |
| DKnight.des | 5 | from-scratch | | | | align_roles mechanism first. ROUTED HERE by the Knight slice (2026-08-06): Dirge sacrifice gift (master pray.c ~2122-2127, chaotic Knight RUNESWORD -> ART_DIRGE at the altar, incl. its artitouch bump) and the mthrowu.c thrown-Dirge multishot message (master ~1890-1894) have no branch counterpart - port with this track |
| Druid.des | 5 | from-scratch | | | | |
| Infidel.des | 5 | from-scratch | | | | |

- [x] GATE BEFORE ANY CONVERSION (NEW, minted 2026-07-30 from the
      2026-07-29 review): ~/des_to_lua.pl still emits 'w' for
      PUDDLE, which on this branch is MATCH_WALL - every converted
      level would silently turn puddles into walls. Add a converter
      remap or hard assertion first.
      DONE 2026-08-01 - both the remap AND the assertion, with a
      FAILURE-MODE CORRECTION: MATCH_WALL(45) >= MAX_TYPE(44), and
      every Lua emission channel SKIPS such cells (lspo_map
      continue, set_levltyp reject, replace_terrain no-op), so
      puddles would not have become walls - they would have
      SILENTLY VANISHED into the level's solidfill: STONE on 8 of
      the 10 affected files (connectivity risk), POOL on Healer
      (shallow water becoming drowning water), ROOM on Knight.
      WIDENED at planning: 'Z' is a SECOND silent collision
      (master MAGIC_CHEST vs 5.0's upstream-standard LAVAWALL -
      both sides valid, zero errors) and 'U' (FORGE) has no branch
      binding at all (INVALID_TYPE; lspo_map's warning is an
      unimplemented upstream TODO). Both have zero map-block uses
      in master (directives only) and both route via des.feature
      by name ("magic chest"/"forge" - branch name-table
      additions), so no engine map-chars are needed. Context
      hazard honored: 'w'/'s' are also monster class chars (14
      giant-leech sites), so terrain_char() applies only in
      terrain funnels (MAP blocks, mapchar_of/terrain_value, the
      MAZE fill, INIT_MAP fg/bg); out-of-vocabulary chars die
      with nonzero exit. Corpus regression: all 36 master .des
      converted pre/post - 2237/2237 puddle map cells remapped
      (per-file counts match the census exactly), 344 changed
      lines every one pure w->p, all des.monster lines
      byte-identical, luac 230/233 (the same three known
      hand-finish files). Gate proof: crafted Z/U levels die at
      the right line with the des.feature route in the message.
      Engine leg (throwaway playground, rebuilt nhdat): converted
      'p' loads as PUDDLE typ 42; the old 'w' output loads as
      solidfill STONE with no message and no paniclog line -
      the silent-erasure counterfactual demonstrated live.
      Crib sheet gained a Map characters section (its old "map
      char kept" limitation note read as an endorsement of raw
      pass-through - corrected). Phase 3 conversion UNBLOCKED.
      Observed en route, not chased: the test level's POOL
      control cell loaded as ROOM with a des.stair directive
      targeting it (DRY-diverted stair, pool absent in both
      legs) - flag if real conversions show pool loss near
      stairs; and #wizloaddes applies flip_level_rnd, so
      coordinate probes must account for random flips.
- [ ] dungeon.def -> dat/dungeon.lua (branches: Ice Queen etc.).
      Includes the elemental-plane order randomization: master's
      dungeon.def gives the four planes @ (2, 4) random-range slots
      vs vanilla's fixed 2/3/4/5 (astral stays 1, dummy 6). The
      level side is already order-agnostic since the endgame slice
      (self-named portals + mkportal In_endgame dlevel-1 arm)
- [x] quest.txt -> quest.lua converter BUILT + VALIDATED (2026-07-03,
      ~/quest_to_50.py; output quest_evilhack.lua 18 sections / 1066
      messages, luac-clean, round-tripped field-by-field through real
      Lua 5.4.8; key sets match upstream exactly on all 13 shared
      roles; quest-conversion-report.md has msgnum->msgid map, C
      call-site rewrite table, gating notes for charon_talk/
      angel_cuss and the altstart substitution hook; rerun after any
      master quest.txt edit)
- [x] Val-strt selection-DSL expression hand-translated (2026-07-02,
      exemplar/Val-strt.lua - adopted upstream's own translation;
      `&` is UNION per SPO_SEL_ADD, crib sheet has the vocabulary)
- [ ] Themed rooms: drop our back-port, keep custom rooms as
      additions on upstream themerms.lua; PLUS (ratified
      2026-07-18) the unique-to-EvilHack rndvaults land here as
      themerms entries - see the Phase 2 special-room-types item
      for the merge rules (frequency tuning, mindiff mapping,
      dedupe-to-upstream on overlap); the vlt-0004/0045
      exemplars + crib ROOMDOOR idiom remain the map-conversion
      reference, destination changed from standalone vlt-*.lua
      files to themerooms contents functions
- [ ] nhl bindings from Phase 2 wired: MAGIC_CHEST, FORGE,
      MINDEPTH, dead=, door material=, room types
- [ ] dat/data.base reconciliation NOT STARTED (minted 2026-08-06 by
      the Archeologist slice; scope ruling same day: ONE full-delta
      slice, NO piecemeal per-track entries). Branch data.base is
      pure vanilla 5.0 (6528 lines vs master 7876; 2452-line diff) -
      every EvilHack encyclopedia addition is missing, incl. the
      quest-track entries atlatl, xiuhcoatl and gjallar (the Val
      slice missed gjallar silently; /lookup finds nothing for
      landed artifacts until this row closes). Port = merge master's
      additions onto the 5.0 file, adopting upstream's version where
      both sides changed an entry (feature-collision rule). Watch
      data.base's strict format (leading-tab bodies, *-wildcards,
      makedefs validation at build time via dat/data).
- [ ] GATE: every level wizmode-walked (W column full)

## Phase 4 - Display and UI

- [ ] Customcolor/truecolor rebuilt on map_glyphinfo(); reconcile
      with upstream coloratt/utf8map (nh50-color-architecture
      memory: 5.0 ships 16-color baseline, truecolor opt-in)
- [ ] unicode.c vs upstream utf8map.c reconciliation
- [ ] curses/tty re-port (print_glyph glyph_info signatures; census
      note: win/tty + win/curses file sets are IDENTICAL between
      trees - pure per-file content reconciliation, no add/remove)
- [ ] HTML dumplog: NetHack50 DUMPHTML base + EvilHack enhancements
      (pet/peaceful highlighting, customcolor swatches, rider fix)
- [ ] TILE-ART DRIFT: win/share/other.txt is behind the branch's
      cmap enum and every tile from the first branch-only symbol
      onward resolves to the WRONG ART. Own item (minted
      2026-07-25 by the terrain slice) rather than a rider on the
      customcolor row, because it is an art+reindex job with a
      different shape and a different source of truth.
      MECHANISM: src/tile.c is GENERATED at build time from the
      cmap enum via win/share/tilemap.c (not tracked in git) and
      IS compiled in - sys/unix/Makefile.src sets
      `NHCFLAGS+=-DTTY_TILES_ESCCODES` and adds tile.c to WINSRC.
      So every defsym.h insert silently shifts the mapping and
      nothing in the build complains.
      MEASURED 2026-07-25: tile.c expects 258 cmap tiles;
      other.txt supplies 243 tiles total; 23 of the expected cmap
      indices fall past the END of the file. First content
      misalignment is now index 19 - tile.c wants "dead tree",
      other.txt has "floor of a room". PRE-EXISTING, not caused by
      the terrain slice: before it the first branch-only insert was
      S_forge, and other.txt index 37 is "other altar", so the file
      was already misaligned there - the terrain slice moved the
      break from 37 to 19 and added 5 more.
      WORK, by source of art:
      (a) master HAS the art, lift + reindex: dead tree (master
          other.txt 19), forge (32), magic chest (33), grass (37),
          sand (38), shallow water (46), sewage (47);
      (b) 5.0-only, NO master counterpart, needs art or a
          deliberate placeholder: S_lavawall + the new trap
          symbols (bolt/ice/spear/magic-beam trap sets);
      (c) NOT a verbatim lift either way - the branch's other.txt
          expands altars (35/36/37 neutral/lawful/other) where
          master's numbering differs, so this is an index-by-index
          reconciliation against the CURRENT defsym.h PCHAR list,
          not a file copy.
      SCOPE OF IMPACT: tile-capable output only (the
      TTY_TILES_ESCCODES escape-code path and the tile ports).
      Plain tty/curses glyph rendering is unaffected, which is why
      this has stayed invisible - it is wrong today, not newly
      wrong. Not a gameplay blocker.
      GATE: after any future defsym.h insert, re-run the check -
      compare tile.c's `other.txt:NNN <name> (cmap=N)` comments
      against other.txt's `# tile N (name)` headers and confirm
      they agree index-by-index. Worth doing as the last step of
      every slice that touches defsym.h.
- [ ] menuitemcolor / status branch-labels / UI extras sweep
      (master menuitemcolor renders BRACKET-SCOPED: item color
      applies only to text inside [] - win/curses cursdial.c
      bracket_color + win/tty wintty.c twin. First consumer
      already live: pet #order toggle rows currently color the
      whole line via the 5.0 add_menu color param; maintainer
      2026-07-17 - only [active]/[inactive] should be colored.
      Port the bracket-scoped rendering into both 5.0 window
      ports, then re-check doorder's rows)
      ALSO OWNS (added 2026-07-27 by the steed battery): master's
      curses perm_invent header prints "Inventory: X/Y weight
      (N/52 slots)"; 5.0's renders no weight or capacity figure at
      all, and no inv_mode covers it. Consequence already on the
      branch: BUG-579's update_inventory() calls in mount_steed and
      dismount_steed are correct but have nothing to refresh, so
      that fix stays latent until this row lands. Re-check it then.
- [ ] Fuzzer input hook (curses_getch chokepoint) re-established

## Phase 5 - Bugfix triage sweep

- [ ] Systematic BUGFIXES_ARCHIVE.md pass for shared-logic fixes
      not covered by subsystem ports; verify against 5.0 body first
      (upstream fixed many independently - their fixes archive is
      doc/fixes5-0-*.txt, renamed from fixesN.N); use the
      file-relocation map (re-derive queue section) to find where
      each fix's home file went

## Phase 6 - Infra and server

- [ ] serverseed persistence (+ hints install parity memory)
- [ ] hints reconciliation (EvilHack vs NetHack50 hardfought hints;
      hints/hardfought + linux-debug + linux-minimal exist in BOTH
      trees; EvilHack-only hints needing re-derive: linux-chroot,
      linux-debug.gha, clang-linux-debug; upstream hints now
      composable *.500 + hints/include/ fragments)
- [ ] Emergency-save recovery vs 5.0 SELF_RECOVER + CRASHREPORT
- [ ] Deploy scripts; xlogfile field-set scoreboard check (plan §9.3)
- [ ] Fuzzer harness re-target (paths, randomkey hook)

## Phase 7 - Validation and release

- [ ] Build matrix; Lua test harness (test/) wired in
- [ ] Long ASan/UBSan fuzzer campaign
- [ ] sfctool/sfexpasc save-format inspection pass
- [ ] Scripted wizmode walkthrough of every special level
- [ ] EvilHack-only docs carried over: colors256.md,
      color-swatches.html, monster-colors.html,
      shop-price-matrix.md, to-do-list.txt (+ evilhack-changelog.md
      - hands-off rule, maintainer curates); lev_comp.6/dgn_comp.6
      man pages die with the level compiler
- [ ] v1.0.0 hard cutover per plan §9 (archive saves+bones, keep
      0.9.x binary offline, announce)

## Upstream-reportable fixes (send as format-patch, never edit their tree)

| date | fix | status |
|------|-----|--------|
| 2026-08-07 | dat/Mon-strt.lua siege/dart-trap placement fragility (the Bar-strt class, worse odds): `selection.floodfill(05,04)` runs AFTER the file's own two 10% '.'->'T' replace_terrain bands, and (05,04) sits inside the x=0..10 band - when the seed rolls a TREE (~10% of vanilla Monk-quest generations) the flood typ-matches the tree cluster instead of the open field, and the 2 dart traps + 8 earth elementals + 4 xorns rndcoord onto a handful of tree cells (or scatter once the selection exhausts). Identical in NetHack50 and Official_nh50. Branch replaced the floodfill with master .des's fixed placements (2 dart coords + 12 siege coords, zero failure mode). Suggested upstream fix: carve the portal cell first (move `des.terrain({05,04}, ".")` above the replace_terrain calls) or seed elsewhere | analyzed on branch (branch no longer uses the floodfill); not yet sent |
| 2026-08-07 | mhitu.c gazemu() AD_SLOW wrong-subject defended(): the gate `!defended(mtmp, AD_SLOW)` passes the GAZING MONSTER where the HERO is the defender - defended(mon,adtyp) answers "is mon itself protected", so 5.0 consults the attacker's own wielded artifact/dragon scales when deciding whether the hero resists a lethargic gaze (3.6 checked the hero's uwep via defends(AD_SLOW, uwep); the defended() modernization swapped the subject). Identical in NetHack50 and Official_nh50. Fixed on the branch by the Monk-slice AD_SLOW master-parity rewrite (BUG-1502), which checks &gy.youmonst | fixed on branch; not yet sent |
| 2026-08-07 | mhitu.c gazemu() AD_STON cancelled arm: the message selector `(is_medusa && mtmp->mcan && !react)` is constant-FALSE (react is provably -1 there - the only earlier assignment breaks out first), so a cancelled Medusa always prints "gazes ineffectually" and the "doesn't look all that ugly" line is dead code. Identical in NetHack50 and Official_nh50. Fixed on the branch by dropping the !react term (BUG-1504, Monk-slice reviewer find) | fixed on branch; not yet sent |
| 2026-08-06 | sit.c/eat.c stackobj-after-dropy on a deletable object (BUG-1498): egg laying does `dropy(uegg); stackobj(uegg);` and the eat.c spit-out block calls `stackobj(otmp)` after a possible `dropy(otmp)` - `dropz()` runs flooreffects (lava/open-air delete, or merge) and already stacks survivors, so the trailing stackobj either no-ops or acts on an OBJ_DELETED object (deferred-dealloc double-delete impossible when the egg merged/burned). Identical shape in Official_nh50; 3.6.x master fixed the class with boolean dropy returns (BUG-295 family). Branch fix (Knight slice): gate both trailing stackobj calls on `->where == OBJ_FLOOR` | fixed on branch; not yet sent |
| 2026-08-06 | dat/Bar-strt.lua siege-ogre placement fragility: `selection.floodfill(37,7) & selection.area(40,03, 45,20)` seeds the floodfill on a cell the file's own 5% '.'&#8594;'T' replace_terrain band (x&#8805;37) can convert, and floodfill matches the terrain typ under the seed - when (37,7) rolls a TREE (~4.3% of vanilla generations) the intersection is empty, rndcoord(1) returns (-1,-1), lspo_monster converts that to random placement, and all 12 siege ogres silently scatter instead of massing east of the river. Identical in NetHack50 and Official_nh50. Found by the Barbarian-slice reviewer's Monte-Carlo while proving the EvilHack sand map raised the failure to ~93%; the branch replaced the floodfill with the master .des's 11 fixed hostile-ogre coordinates (zero failure mode). Suggested upstream fix: paint the seed first (des.terrain({37,07}, ".")) or use fixed coordinates | analyzed on branch (branch no longer uses the floodfill); not yet sent |
| 2026-07-12 | shk.c shop_keeper() impossible() format typo `&d` -> `%d` (code only compiles when NH_DEVEL_STATUS is WIP/BETA, so upstream's POSTRELEASE builds never saw the -Wformat-extra-args warning) | fixed on branch; not yet sent |
| 2026-07-12 | util/sfctool.c validate() call missing the 4th arg added by the u.ustuck save/restore fixes (optional target, upstream CI never builds it); also two -DSFCTOOL -Wunused-function warnings in files.c (set_whereisfile/write_whereis) | validate fix on branch; warnings not chased |
| 2026-07-13 | WIP-build false positive: shop_keeper()'s "untrustworthy null shkp" diagnostic (shk.c:1082) fires during throne-room generation - fill_zoo COURT chest fill -> add_to_container gold merge -> obfree's unconditional shop_keeper(*u.ushops) fallback (shk.c:1227) probes rmno '\0' while level_status.making. Repro: generate levels until a COURT spawns (4 merges = 4 reports at one newgame observed). Suggested fix: diagnostic should skip rmno < ROOMOFFSET - a no-room probe's NULL is definitionally correct | traced on branch (gdb bt); code byte-identical upstream; not yet sent |
| 2026-07-15 | steal.c mdrop_obj() use-after-free: update_mon_extrinsics(mon, obj, ...) runs after flooreffects() may have destroyed obj (worn item dropped over lava/open air deallocates it, then the extrinsics update derefs the freed object). Verbatim in upstream 5.0 at adoption commit 74d1133c2 and in Official_nh50. Fixed on the branch (oprops slice) with master's saved-fields dummy-object pattern; the companion stackobj() merge in the safe path was reviewer-proven NOT to free the dropped obj (merged() keeps the potmp arg = the dropped obj, frees the pre-existing floor stack) so no further change needed | fixed on branch; not yet sent |
| 2026-07-22 | pray.c can_pray() demon gate `(gp.p_aligntyp == A_LAWFUL \|\| gp.p_aligntyp != A_NEUTRAL)` contradicts its own "ok if chaotic or none" comment - as written, demons find CHAOTIC prayer repugnant and NEUTRAL prayer fine. Master's `> A_CHAOTIC` form (matches the comment's intent) ported with the roles slice; reviewer-verified | fixed on branch; not yet sent |
| 2026-07-17 | explode.c monster-kill loop mutates its adtyp local (`adtyp = AD_RBRE; /* no corpse */` at explode.c:579, identical in NetHack50 and Official_nh50): after the first no-corpse victim, every later victim of the same blast and the post-loop hero-damage block see AD_RBRE instead of the real damage type. Master's non-mutating `xkflg ? -AD_RBRE : adtyp` inline form ported to the branch (kill-attribution slice) | fixed on branch; not yet sent |
| 2026-07-23 | pray.c pleased() gift switch rolls `rn2((Luck + 6) >> 1)` with no lower clamp: Luck <= -6 passes rn2() a non-positive argument (reachable - the pleased path gates on god anger/trouble, not on Luck). Master's clamp form (`gift_slot` local, floor at 1) ported with the roles slice; branch verified at pray.c pleased() | fixed on branch; not yet sent |
| 2026-07-23 | artifact.c invoke_healing (5.0's split of arti_invoke HEALING) can print "You feel better." twice per invoke: the body has BOTH an unconditional `Blinded > creamed -> You_feel("better.")` arm and the gated `You_feel("%sbetter.", ...)` tail. Identical at 74d1133c2; looks like an upstream merge artifact. Left as-is on branch (only the Withering cure term was added by the artifacts slice) | traced; not yet sent |
| 2026-07-24 | muse.c hero scroll-of-fire arm: `if (Fire_resistance) You("are not harmed.");` followed by `if (Half_spell_damage) num = (num + 1) / 2; else losehp(num, ...)` - the losehp rides the halving else, so a fire-resistant hero takes FULL damage whenever Half_spell_damage is absent, and ANY hero with Half_spell_damage takes zero. Identical in NetHack50 and Official_nh50. Branch restored master's gate form (==100 no-harm + monstseesu, else halve-then-losehp) with the partial-resistance slice | fixed on branch; not yet sent |
| 2026-07-25 | sp_lev.c lspo_monster() leaks two heap strings on its table-form error paths, because `nhl_error` -> `lua_error` longjmps out past the `Free`s: the `nhl_error(L, "Unknown appear_as type")` arm abandons `mappear` (from get_table_str_opt) plus `tmpmons.name.str`, and an `nhl_error` raised inside `get_table_montype()` abandons `tmpmons.name.str` and the `dupstr`ed `tmpmons.appear_as.str`. Verified identical in NetHack50 and Official_nh50. Matters a little more than a normal error-path leak because a Lua error during level generation is NOT fatal - `nhl_pcall_handle` uses NHLpa_impossible, so load_special returns FALSE, mkmaze falls back to a random maze and play continues, so the leak can recur. NOT the same as `get_table_montype()` itself, whose `Free(s)` correctly precedes its `nhl_error`. The branch's three new positional-form `nhl_error`s are leak-free by construction (nothing is allocated yet at that point and paramstr is Lua-owned) | traced, not fixed on branch; not yet sent |
| 2026-07-25 | sp_lev.c lspo_monster(): a `des.monster{ inventory = <non-function> }` (a table, say) still sets `has_invent = CUSTOM_INVENT` at the `lua_getfield`/`!lua_isnil` test, so create_monster() points the file static `invent_carrying_monster` at the monster, but the closure gate additionally requires LUA_TFUNCTION - the else arm only does `lua_pop` and never calls spo_end_moninvent(). The static therefore outlives the call and create_object() diverts every later `des.object()` in the file into that monster instead of onto the map. Identical in NetHack50 and Official_nh50 (both `} else lua_pop(L, 1);`, and their create_monster is `staticfn void`). Harmless-but-wrong upstream; on the branch the `dead` binding turns it into a silent object leak (objects handed to a detached monster, freed by dealloc_monst which has no minvent handling), so the branch clears the static in the else arm whenever CUSTOM_INVENT was requested | fixed on branch; not yet sent |
| 2026-07-26 | dungeon.c/dgn_file.h dungeon alignment - THREE defects in one mechanism, all identical in Official_nh50. (1) `init_dungeon()` assigns a `D_ALIGN_*` value (`AM_* << 4`) straight into `Bitfield(align, 3)`, so lawful/neutral/chaotic all truncate to 0 and every dungeon-wide alignment in dat/dungeon.lua is discarded; 3.6's dgn_comp-era code has the same assignment with a `>> 9` shift AND the comment "bit shift needs to match that in include/dgn_file.h". (2) `init_level()`'s inherit-from-dungeon fallback reads `pd->tmpdungeon[dgn].flags & D_ALIGN_MASK`, but 5.0's Lua parser stores dungeon alignment in a dedicated `.align` member and never puts an alignment bit in `.flags` - so that member is write-only dead code and special levels never inherit. (3) `UNCONNECTED` (0x10) is bit-identical to `D_ALIGN_CHAOTIC` (`AM_CHAOTIC << 4`), and D_ALIGN_MASK 0x70 overlaps the flag word, so upstream's own Tutorial block (`flags = { "mazelike", "unconnected" }`) reads back through (2) as chaotic - tut-1/tut-2 generate chaotic-weighted monsters and chaotic-biased altars that nothing ever declared. Branch fix decouples alignment from the flag word (`struct tmplevel` gains an `align` member mirroring tmpdungeon's); master's `<< 9` was rejected because master has zero headroom above HDGN 0x100 and the branch's eventual ten-flag set would re-collide. gdb-proven: Mines 4 / Sokoban 2 / Vlad 1, minetn+minend 4, tower1-3 1, soko1-4 2, tut-1/tut-2 0 with Tutorial still unconnected 1 | fixed on branch (BUG-1381/1382/1383); not yet sent - decide at send time whether upstream gets the decoupling or a minimal shift-only variant |
| 2026-07-24 | uhitm.c mhitm_ad_slee monster-vs-monster arm: `!mdef->msleeping && sleep_monst(mdef, rnd(10), -1) && sleep_monst(mdef, rnd(10), -1)` - sleep_monst called twice in one condition, so the sleep applies only when both rolls succeed and the first success is re-rolled. Identical in NetHack50 and Official_nh50. Left as-is on branch (monster-side, outside the partial-resistance slice) | traced; not yet sent |
| 2026-07-26 | trap.c trapeffect_landmine() captures the floor saddle pointer (`sobj_at(SADDLE, u.ux, u.uy)`) BEFORE `losehp()` and `blow_up_landmine()`, then hands it to `keep_saddle_with_steedcorpse()`, which does `obj_extract_self()`/`place_object()` through it. `blow_up_landmine()` calls `scatter()`, which can destroy floor objects outright - metallic gear is exactly what a blast breaks - so the pointer can be freed before it is dereferenced. EvilHack master fetches after the explosion and documents why. Identical in Official_nh50 | fixed on branch (BUG-1388, commit f6a787e0d); not yet sent |
| 2026-07-26 | mon.c m_consume_obj() reads `otmp->otyp` at the pyrolisk-egg test AFTER `delobj(otmp)` has freed the object - the function carefully snapshots every other field it needs into locals beforehand (corpsenm, deadmimic, slimer, unsick, poly, grow, heal, eyes, mstone) and missed this one, so every monster meal ends with a read of freed memory. The guarded action is also dead code: `corpsenm` is assigned `(otmp->otyp == CORPSE ? otmp->corpsenm : NON_PM)`, so `otyp == EGG && corpsenm == PM_PYROLISK` can never be true and a pyrolisk egg has never exploded when eaten. Identical in Official_nh50 (mon.c, the corpsenm assignment and the test). Branch fix snapshots the condition into a `pyro_egg` local before delobj, which kills the read and deliberately leaves the dead branch dead - whether the explosion is meant to fire is a gameplay question for the maintainer, not a port decision | UAF fixed on branch (BUG-1391); dead-code question open; not yet sent |
| 2026-07-29 | mkmaze.c `movebubbles()` sets `mon->mstate \|= MON_BUBBLEMOVE` when a bubble on the Plane of Water lifts a monster, and nothing anywhere clears it - three hits tree-wide (the `#define`, that `\|=`, and a comment in `place_monster()` listing it among bits that function deliberately preserves), and no code reads it. `mon_offmap()` is `mstate != MON_FLOOR` with `MON_FLOOR == 0`, so the stale bit makes every carried monster read as permanently off-map; `mon_sanity_check()` reaches its complaint only for a monster that IS on the map at its own coordinates, so it fires once per carried monster per turn forever. With `sanity_check` on that is ~900 paniclog lines and a `--More--` wall in 25 turns. Verified character-identical in NetHack50 and Official_nh50. Branch fix clears the bit in the `CONS_MON` bubble-restore arm, before the `mnearto()`/`elemental_clog()` placement so the failure path's own state is not stomped; `place_monster()`'s mask left alone | fixed on branch (BUG-1394); not yet sent - correct the archive entry first: its "clearing afterwards would stomp whichever bits that path had just set" clause is false of &= ~ONE_BIT; the right justification is the entry's own first clause (the transit is over - the clear is unconditional and placement-independent) |
| 2026-07-29 | display.c `see_monsters()` calls `newsym(mon->mx, mon->my)` on every live monster after skipping only DEADMONSTER, PARKEDMONSTER and MON_STILL_ARRIVING - but a monster in transit inside a Plane of Water bubble has `mx = my = 0` and is none of those, and `newsym()` rejects column 0 with `impossible("newsym: attempting screen update for <0,0>")`. Note PARKEDMONSTER is `isgd && mx == 0`, so the loop already special-cases one kind of off-map monster and misses the general case. Backtrace: movebubbles -> mv_bubble -> mnearto -> mon_leaving_level -> unstuck -> docrt -> docrt_flags -> see_monsters -> newsym(0,0). Byte-identical loop in NetHack50 and Official_nh50. Branch fix makes the skip `DEADMONSTER(mon) \|\| !mon->mx`, which strictly subsumes PARKEDMONSTER; `!mon->mx` rather than `mon_offmap()` so the display path does not depend on mstate bookkeeping being correct | fixed on branch; not yet sent |
| 2026-07-29 | mon.c `mon_sanity_check()` carves the parked vault guard out of its position test - `if (!isok(x, y) && !(mtmp->isgd && x == 0 && y == 0))` - but not out of its final `mon_offmap()` test, so the `MON_PARKED` bit that `parkguard()` (vault.c) legitimately sets while the temporary corridor is torn down trips `impossible("floor mon (%s) with mstate set to 0x%08lx")` once per guard departure. The state is correct and the check is incomplete, which is the inverse of the MON_BUBBLEMOVE bug above. Identical in NetHack50 and Official_nh50 | found on branch; fix pending |
| 2026-07-26 | uhitm.c joust-shatter use-after-free created by 5.0's own split of hmon_hitmon() into helpers: `hmon_hitmon_jousting()` takes `obj` BY VALUE, and its `hmd->jousting < 0` arm runs `useup(obj); obj = (struct obj *) 0;`, so only the helper's local is cleared. Back in `hmon_hitmon()` the caller's `obj` is now dangling and is still read by `hmon_hitmon_pet/splitmon/msg_hit`, the `oprops` venom arm, and above all `if (hmd.unpoisonmsg) { assert(obj != NULL); Strcpy(hmd.saved_oname, cxname(obj)); }` - the neighbouring assert cannot catch it because the pointer is non-NULL and freed. Reachable with a poisoned lance: shatter is `joust_dieroll == 0` and `rnl(50) == 49`. 3.6's single-function form had the NULL-deref variant instead (EvilHack BUG-896). Identical in Official_nh50 (uhitm.c, the shatter arm and the unpoisonmsg tail). Branch fix adds `hmd->obj_gone`, set by the helper and consumed by the caller, which then drops its own pointer; the unpoisonmsg sites gate on `obj` and on `saved_oname[0]` | fixed on branch (BUG-1390); not yet sent |
| 2026-07-29 | dog.c mon_arrive() never clears MON_ENDGAME_MIGR: elemental_clog() sets it when an Elemental Plane is too full and bumps a monster to the next one, mon_arrive() clears only MIGRATING\|LIMBO, and place_monster() deliberately preserves it - so the migrated monster lands permanently frozen on the destination plane (mon_offmap() is mstate != MON_FLOOR, so it never moves, gets no regen/timeouts, and draws a sanity complaint per turn under sanity_check). Fix shape per BUG-1394's own reasoning: add the bit to mon_arrive()'s clear mask. Same family: elemental_clog() on Astral with no candidate leaves the caller off-map forever. Verified byte-identical in NetHack50 and Official_nh50 | FIXED on branch 2026-07-31 BUG-1442 (mon_arrive clear-mask); report artifact ~/nh50-mon-endgame-migr.patch applies clean to both upstream trees; upstream unreported. Astral no-op family FIXED on branch 2026-07-31 BUG-1444 (m_into_limbo per session ruling; still live upstream - the artifact's sibling note stands) |
| 2026-07-31 | sp_lev.c rndtrap() rolls TRAPPED_DOOR/TRAPPED_CHEST out of rnd(TRAPNUM - 1); maketrap() silently rejects both, so roughly seven percent of fill_empty_maze() trap placements evaporate. Upstream's own rndtrap() has the same roll over its 24/25 values. Branch fixed by BUG-1429 (exclusion cases + the restored retry cap); upstream still affected | fixed on branch; upstream unreported |
| 2026-07-31 | uhitm.c hand-thrown ammo never applies weapon poison: hmon_hitmon_weapon() routes `is_ammo && (thrown != HMON_THROWN \|\| !ammo_and_launcher(obj, uwep))` to hmon_hitmon_weapon_ranged(), but `hmd->ispoisoned` is set only in hmon_hitmon_weapon_melee()'s thrown-ammo tail, which such throws never reach - a poisoned arrow thrown without its bow hits with no poison effect at all. Thrown darts are is_missile, take the melee helper, and work. 3.6's monolithic hmon_hitmon() set ispoisoned for every WEAPON_CLASS hit, so this is a 5.0 refactor regression. Routing and flag placement byte-identical in NetHack50 | found on branch (BUG-1421 battery); not yet sent |
| 2026-07-31 | trap.c sink_into_lava() reduces only `u.uhp` even when the hero is polymorphed - the dormant real-form pool, while the active `u.mh` never shrinks (their own rehumanize() comment calls reducing u.uhp while poly'd a code bug). The branch's exclusive-pool form (BUG-1423) is the fix shape. Identical in NetHack50 and Official_nh50 | fixed on branch (BUG-1423, exclusive form); not yet sent |
| 2026-07-31 | trap.c cnv_trap_obj() runs `if (u.utrap && u_at(ttmp->tx, ttmp->ty)) reset_utrap(TRUE);` with no TT_BURIEDBALL carve-out, silently freeing a Punished hero from a buried iron ball when a co-located trap converts to objects; upstream's own delfloortrap() carries the carve-out (the same defeat EvilHack fixed at deltrap_with_ammo(), BUG-1422 - EvilHack has no cnv_trap_obj to fix). Reviewer find, verified in Official_nh50 | found (no branch counterpart); not yet sent |
| 2026-08-01 | dungeon.c init_dungeon_levels()/init_dungeon_branches() fill their proto_dungeon arrays BEFORE the LEV_LIMIT/BRANCH_LIMIT panics that follow the loops - the lua_len count is file-driven and unbounded, so an oversized dungeon.lua corrupts final_lev[] (later dereferenced by place_level), tmpbranch[], then pd's own counters (which can suppress the panic itself), and past ~25 slots over the limit the init_dungeons() stack frame - instead of the intended clean panic. 3.6's compiled loader checked before Fread-filling, and its dgn_comp.y compile-time validation layer was dropped by the Lua rewrite, so the runtime guard's ordering is the only protection left. Byte-identical in NetHack50 and Official_nh50 (both functions). Battery-proven end-to-end on the branch: padded nhdat -> clean pre-loop panic, write breakpoints unhit | fixed on branch (BUG-1453, pre-loop bounds); report artifact ~/nh50-dungeon-init-guards.patch; not yet sent |

## Master re-derive queue

Mechanism: `git log evilhack-50-fork-point..master --oneline`; every
commit gets re-derived by hand on the branch (NEVER `git merge
master`) and ticked here. Seeded at adoption 2026-07-12: EMPTY
(fork point c444f6a3a == master tip; nothing outstanding).

| master commit | subject | re-derived as | status |
|---------------|---------|---------------|--------|

QUEUE PERMANENTLY EMPTY: maintainer ruling 2026-07-15 - master
(v0.9.3) will not see any further updates. The 2026-07-15 backport
batch (BUG-1330..1333, briefly committed locally as
d6f951a21/7ffa7c24c/abe69f813/60e7a491e) was rescinded the same
day and master reset to the fork point c444f6a3a; the four fixes
live on the branch (they originated there). The MASTER BACKPORT
CANDIDATE tracking concept is retired - future master-side bugs
found during branch work get an archive entry noting master is
frozen-unfixed, nothing more.

### File-relocation map (where a master fix lands on the branch)

Function-level provenance verified 2026-07-03 (full census incl.
include/sys/win/util/dat/doc: plan appendix "new/removed file
mapping"). A master commit touching the left file may re-target
the right file in 5.0:

| EvilHack file | What moved | 5.0 file |
|---------------|-----------|----------|
| cmd.c | enlightenment/#conduct/#attributes/achievements | insight.c |
| cmd.c | wiz_* commands + #stats/sanity_check | wizcmds.c |
| end.c | vanquished/genocided listings | insight.c |
| end.c | panictrace | report.c |
| priest.c | align_str, piousness, mstatusline, ustatusline | insight.c |
| do_name.c | getpos + gloc_filter subsystem | getpos.c |
| files.c | config parsing/config_error_* | cfgfiles.c |
| files.c | symset line parsing | symbols.c |
| pline.c | vconfig_error_add | cfgfiles.c |
| drawing.c | symbol init/assign/switch | symbols.c |
| options.c | symset load/parse | symbols.c |
| options.c | color+attr names, menucolors | coloratt.c |
| hacklib.c | time/date functions | calendar.c |
| hacklib.c | strbuf_*, pmatch | strutil.c |
| dungeon.c | u_on_*stairs, On_stairs | stairs.c |
| allmain.c | argcheck, debug_fields | earlyarg.c |
| sp_lev.c | selection_* engine | selvar.c |
| save.c | bwrite/bufon/bufoff/bclose/bflush | sfstruct.c |
| restore.c | mread | sfstruct.c |
| mapglyph.c | mapglyph() -> map_glyphinfo() | display.c |
| util/makedefs.c | version/banner strings | mdlib.c |
| sys/winnt/nttty.c | whole file renamed | sys/windows/consoletty.c |
| sys/winnt/winnt.c | whole file renamed | sys/windows/windsys.c |

Name-collision watchlist: wiz_telekinesis (upstream debug command
in wizcmds.c vs our telekinesis.c ability), src/glyphs.c (both
trees, different content - Phase 0 rename).

## Feature-collision audit (adopt upstream, re-add our extras)

| Feature | Upstream/NetHack50 has | EvilHack extras to re-add | status |
|---------|------------------------|---------------------------|--------|
| HTML dumplog | DUMPHTML (fork) | pet/peaceful highlight, customcolor, rider hero highlight | |
| livelog | fork livelog | EvilHack event extras - diff the sets | |
| server hints | hints/hardfought (fork) | EvilHack hints deltas (pkg-config WINTTYLIB etc.) | |
| recover | fork recover.c changes | EvilHack recover tweaks | |
| themed rooms | themerms.lua (origin) | EvilHack-only room types + unique rndvaults merged in as entries (ratified 2026-07-18; rndvault C registry retired) | |
| color/truecolor | coloratt/utf8map/glyphs.c | extcolor 24-bit, customcolor registry (RENAME ours) | |
| crash logging | CRASHREPORT (report.c) | crash-log infra, paniclog extras | |
| self-recovery | SELF_RECOVER | emergency .e.gz save + in-game recovery | |
| extended player names | fork | verify parity | |

## Decisions log

| date | decision | by |
|------|----------|----|
| 2026-08-02 | Mines rivers (makeriver/mkrivers) PORTED WITH THE MINES SLICE C payload rather than left on the terrain-slice no-counterpart deferral list - the only caller is mines fill generation, so the mines row owns it. Includes master's mkstairs undetected-swimmer reveal tail. concealed_spot + maybe_freeze_underfoot remain deferred (their consumers are elsewhere) | maintainer |
| 2026-08-02 | describe_level branch-aware status naming: MINES ARMS ONLY this slice (Mine Town / Mines' End / Mines:n + the minetn_level topology plumbing they need). The remaining master arms (Soko/Goblin/IceQ/Vlad/Vecna/Hidden/WizTwr/Purg/Hell) = NEW Phase 2 row, landing as their branch predicates arrive; five of the predicates do not exist on the branch yet | maintainer |
| 2026-08-02 | minend stays NO-BONES (upstream 5.0's deliberate removal KEPT so the luckstone achievement is earnable every game) - a recorded deviation from master's dungeon.def bonetag "E". Contrast: the soko/castle strict-parity rulings cover level CONTENT; this is a systems trade the maintainer decided the other way | maintainer |
| 2026-08-02 | minend luckstone BUC spec: upstream's buc="not-cursed" DROPPED on all five variants - the sokoban prize ruling (prizes CAN generate cursed, master parity) extended to the mines prize | coordinator |
| 2026-08-02 | Sokoban levels NEVER FLIP: all 24 soko lua files carry level_flags "noflip". Upstream 5.0's soko levels flip both axes; the maintainer does not want Sokoban puzzles mirrored. Stated up front by the maintainer when opening the slice | maintainer |
| 2026-08-02 | Upstream 5.0's Sokoban additions (rolling-boulder traps, monster-generation exclusion zones) STRIPPED from all 24 levels - strict master content parity, castle-chest-ruling precedent extended to a whole subsystem. EXCEPTION same ruling set: upstream's soko2-2 map redesign (side passage + two locked doors) is KEPT - the one upstream map change that survives; its trap row stays map-consistent (RB cell -> hole, 12 holes) | maintainer |
| 2026-08-02 | zap_dig guard INCLUDED as an 8th prize-door guard beyond the 2026-07-24 ruling's enumerated seven (the enumeration missed master's dig.c site; without it a wand of digging razes the closet). The 9th master site (domove_core fire-weapon door-burn) stays deferred with its unported feature | maintainer |
| 2026-08-02 | Sokoban prize subsystem lands with FULL master consumer parity minus color: protections, prize naming, material exclusion, Is_nonprize_container all in the slice; bright-green prize coloring deferred to Phase 4 with the door-material color channel | maintainer |
| 2026-08-02 | Sokoban slice ships as TWO commits (knox/tower precedent): commit 1 = all C work (door producer + 8 guards + 3-slot prize subsystem interlock), commit 2 = 24 levels + dungeon.lua + build lists | maintainer |
| 2026-08-02 | Tower slice ships as TWO commits (knox precedent): commit 1 = the open-air terrain subsystem (closes the dormant Phase 2 row + its recorded deferrals), commit 2 = the four tower levels + dungeon plumbing; one combined battery after both | maintainer |
| 2026-08-02 | Vampire spawn-form policy RESTORED to master's (makemon): Vlad AND Kas always true form; vampire mage/royal/noble/sovereign each 50/50 true-vs-shifted; all other vampires shift at creation. The tree adoption had silently reverted this (vanilla==5.0 != master triple) leaving every vampire except Vlad always-shifted - the castle-2 battery's all-bats observation traces to exactly this | maintainer |
| 2026-08-02 | Brides of Dracula KEPT, adapted: upstream 5.0's three named female vampire nobles (Madame/Marquise/Countess, waiting=1 true form, genocide-guarded) occupy three of EvilHack tower1's eight niche vampire slots. Upstream-content exception to the strict-parity default (contrast the castle potion ruling) | maintainer |
| 2026-08-02 | Vlad keeps his makemon Candelabrum (keep_default_invent = true in tower1.lua) instead of master's discard-and-recreate .des shape - the branch's mdrop_special_objs safety would otherwise duplicate the unique item (floor copy + inventory copy). Side effect: Vlad may also carry his makemon blood potions, which master's discard stripped - accepted divergence | coordinator |
| 2026-08-05 | wizard.c hygiene cluster (BUG-798..806 incl. the absent BUG-806 Astral aligned-altar feature block, plus BUG-257 covetous-escape in monmove.c) stays with the tracker's named Wizard-harassment sweep rather than riding the yendor C payload; the sweep is now a next-session candidate because this slice makes the tower reachable | maintainer |
| 2026-08-05 | yendor slice ships as TWO commits (knox/tower/soko/mines precedent); the describe_level WizTwr dnum arm rides commit 1 with a known interim (it matches all of Gehennom until commit 2's dungeon.lua flip; the commits land together per the soko interim ruling) | coordinator |
| 2026-08-05 | The Wizard's Tower lua levels carry NO hell_tweaks trailer (the dungeon is not hellish and not in Gehennom; wizportal keeps its .des lava scripting as the content) and NO noflip (census-based verification, medusa/castle/mines precedent) | coordinator |
| 2026-08-05 | zap.c Wizard-of-Yendor levelport-immunity lines NOT ported standalone - they live inside master's SPE_DISPEL_EVIL banish ladder, which the branch does not carry; they travel with that feature's slice | coordinator |
| 2026-08-05 | hellfill = MERGE: upstream's 7-variant themed generator + prefabs kept wholesale (incl. its native u.invocation_level vibrating-square handling - master's mklev.c invocation-level exclusion NOT ported); master's five EvilHack flavour groups (L/shade-skeleton/J/salamander/alhoon) grafted into populatemaze(). Master's single hand-rolled design, its SPILL floodfill frame, its 4-option stair separation and its global teleport-region arrival constraint are all subsumed by the variant structure. Upstream's helpers already carry master's class registers correctly SHUFFLED (master declares them without SHUFFLE - frozen-unfixed latent) | maintainer |
| 2026-08-05 | hell_tweaks STRIPPED from the delta-diff fixed levels (orcus, asmodeus->hella-2, incl. their now-unused bounds/protected preambles): ported fixed levels generate exactly master's content (castle/soko strict-parity precedent). hellfill keeps its internal variant-2 use per the merge ruling; from-scratch tier levels get no call | maintainer |
| 2026-08-05 | Flames-of-hell (in_hell_effects ambient fire, master trap.c/allmain.c) and the monclock spawn-rate ladder (incl. BUG-660's got_quest_early half; svc.context.timeout_start already dead on branch; 5.0 rewrote spawn pacing) each get their OWN Phase 2 row rather than riding the gehennom slice; the slice lands their prerequisites (_entered setters, Is_hell* macros) | maintainer |
| 2026-08-05 | gehennom slice ships as TWO commits (knox/tower/soko/mines/yendor precedent); the level_tele clamp rungs ride commit 1 reading depth() of unset hella/hellb levels as a known interim until commit 2's dungeon.lua (WizTwr-arm interim class, commits land together) | coordinator |
| 2026-08-01 | Castle wand chest: upstream 5.0's added potion of gain level (nh50 a8800a9ac anti-frustration guarantee) DROPPED everywhere - strict master parity on the chest contents (wand of wishing +2 recharged:1 only) across all three castle variants. A deliberate deviation from the adopt-upstream collision default | maintainer |
| 2026-08-01 | castle-2 court monsters GET the shuffle on branch (master's castle-2 $monster register lacks SHUFFLE while castle-1/3 and vanilla all shuffle = master oversight, BUG-1455 frozen-unfixed): dat/castle-2.lua carries shuffle(monster) | maintainer |
| 2026-07-25 | RNDVAULT rtype DROPPED rather than ported: 5.0's THEMEROOM occupies its exact role (both trees carry the same "treat like OROOM so random content still lands" test), which follows directly from the ratified 2026-07-18 rndvaults-to-themerooms decision. Consequence: SHOPBASE 14->20 and CANDLESHOP 31, NOT master's 21/32. This also incidentally FIXES a live master bug - mapseen_feat.shoptype is Bitfield(5) and recalc_mapseen stores a raw rtype, so master's CANDLESHOP 32 truncates to 0, the "mixed shop types" sentinel, and a known candle shop annotates as "untended shop" in the overview. Constraint recorded at the enum; do NOT re-add RNDVAULT | coordinator |
| 2026-07-25 | master's select-and-fill mkgarden(NULL) SPLIT into mkgardenroom() (selection, sets rtype + needfill) and mkgarden()/mkforest() (fill only), because 5.0 defers all filling to fill_special_room() and a literal port would fill every garden twice. Master's lit-room selection requirement is preserved in mkgardenroom rather than routing through mkzoo()/pick_room(), which would have lost it | coordinator |
| 2026-07-25 | COCKNEST depth gate restored to master's `u_depth > 15` from upstream's 16. Provenance established the EvilHack-tweak way: vanilla at merge-base 44b6612fd is 16, upstream 5.0 is 16, master is 15 - a vanilla==5.0 != master triple, so it is an EvilHack change the tree adoption silently reverted, not an upstream improvement to keep. Minted the unlogged-numeric-constant sweep item, since the archive-keyword sweeps cannot catch a tweak that was never archived | maintainer |
| 2026-07-25 | EvilHack terrain enum placement mirrors MASTER'S RELATIVE ORDER (DEADTREE after TREE; GRASS/SAND after ICE; PUDDLE/SEWAGE after CLOUD) rather than appending at the end - this keeps every range macro's meaning without rewriting it: IS_OBSTRUCTED(<POOL) catches DEADTREE as solid, IS_ROOM(>=ROOM) catches GRASS/SAND as walkable floor, IS_FURNITURE(STAIRS..ALTAR) is undisturbed. MAX_TYPE 39->44, MATCH_WALL 40->45, xFLOOR..xWATERWALL +5; terrain_descr[]/levltyp[]/type_names[] are positional and shift with them. 5.0's open-ended `ACCESSIBLE(typ) >= DOOR` deliberately LEFT ALONE (master bounds it `<= SEWAGE`; the open form already spanned MAX_TYPE before this change and is not a bug) | coordinator |
| 2026-07-25 | struct rm `Bitfield(flags, 5)` WIDENED TO 6 so master's renumbered ICED_PUDDLE 4 / ICED_SEWAGE 8 / ICED_POOL 16 / ICED_MOAT 32 fit (32 overflows 5 bits; master widened it too). levl saves whole-struct via historical_sfo_rm so there is no hand sfo/sfi pair and no new field, leaving the generated exportascii writers unchanged. Master's neighbouring frac_altar bit deliberately NOT adopted (sacrifice subsystem, different slice). CAVEAT THAT BIT US: sizeof(struct rm) is 12 either way, so critical_sizes[] does NOT invalidate old saves - EDITLEVEL bumped 0 -> 1 to arm it. Applies to ANY future bitfield widening | coordinator |
| 2026-07-25 | PUDDLE takes map char 'p', NOT master's 'w': 5.0 already uses 'w' for MATCH_WALL and four upstream levels depend on it (hellfill, nhlib, themerms, bigrm-11). Empirically scanned every map/mapfragment block across all 131 dat/*.lua - the complete charset in use is `#+-.ABCFLPSTWZ\wx{\|}`, so master's 's'/'g'/'a'/'t' stay master-compatible and only PUDDLE needed a new char ('p' free, mnemonic against 'P' = POOL). Phase 3 carry: des_to_lua.pl emits 'w' for puddles and must be remapped (DISCHARGED 2026-08-01 - the converter GATE row) | coordinator |
| 2026-07-24 | how_resistant() honors FROMOUTSIDE as 100% - divine/external permanent grants are full strength (fixes master's crowned-sick-res-reads-0% quirk, archived BUG-1357; the crowning grant itself stays `HSick_resistance \|= FROMOUTSIDE` master-exact). Rejected: master-exact 0% read; producer-side rework | maintainer |
| 2026-07-24 | how_resistant() reads the TIMEOUT bits as a percentage ONLY when HAVEPARTIAL is set; a plain timed intrinsic reads 100 while active and expires via nh_timeout (reconciles 5.0-native timed grants - lava double-lifesave hatch, mid-meal acid/stone res - with the percentage system; no-op vs master, which has zero timed grants on the eight queried props). Rejected: master-exact raw read (5.0 features misbehave); rewriting the two 5.0 grant mechanisms | maintainer |
| 2026-07-23 | Off-hand/twoweap artifact extrinsics = MASTER PARITY: setworn admits W_SWAPWEP while u.twoweap (teardown stays unconditional per master's own form) + wield.c dotwoweapon/untwoweapon/retwoweapon reseat brackets; un-deadens the oprops W_SWAPWEP mapping arm. Rejected: keep 5.0's no-off-hand-intrinsics policy (real nerf vs master) | maintainer |
| 2026-07-23 | Warning glow = MASTER PER-OBJECT subsystem (has_glow_warning/glow_warning_effects/blind_glow_warnings, any SPFX_WARN\|SPFX_DFLAGH artifact incl. Shadowblade, off-hand aware, per-obj newwarncnt/lastwarncnt), replacing 5.0's hardcoded global Sting_effects - rides artifacts Commit B | maintainer |
| 2026-07-23 | Artifacts slice ships as TWO commits: A = invokes + hero-side internals (role-unblocking), B = combat behaviors (artifact_hit arms, mon_wielding_artifact layer, u_wield_art sweep, glow rework); one combined battery after both | maintainer |
| 2026-07-23 | Sanctum escape locks (portal invoke / cursed gain-level / level-tele) = DROP-PROOF per master's "even if you've dropped the Amulet/Idol" intent: NEW achievement_recorded(ACH_AMUL) query (insight.c) && (!Infidel \|\| uimbued_idol). Supersedes the carry-based uhave conjunct for THESE three sites only; chase gates keep the roles conjunct. Pauper edge: a pre-imbue pauper Infidel carrying the real Amulet can still portal out (conjunct leniency, consistent with the pauper chase-gate design) | maintainer |
| 2026-07-23 | spec_ability cspfx OR restored (carried-artifact intrinsic leak = master parity; 5.0 never had cspfx so not a collision case) - closes the oprops open-decision row | ratified with Commit A |
| 2026-07-22 | Infidel imbue state = NEW u.uevent Bitfield uimbued_idol (kill-attribution precedent: appended bit, achieve2 game-state source). Master's u.uachieve.amulet consumers sweep to it (conjunct sites reduce to `u.uhave.amulet && (!Role_if(PM_INFIDEL) \|\| u.uevent.uimbued_idol)`). Rejected: object-side-only carrying_imbued_idol() derivation (stashed-Idol no-heal + cancellation-erases-imbue deltas) | maintainer |
| 2026-07-22 | Amulet first-possession WISH GRANT REMOVED entirely (hardfought-5.0 u.uevent.amulet_wish feature; every Infidel would get a free turn-1 wish). Grant block deleted from moveloop; the you.h bit + u_init zeroing stay dormant (wand-of-stasis precedent). Maintainer flags this as the FIRST OF SEVERAL intended 5.0-behavior reversions - collect future candidates here as they are named | maintainer |
| 2026-07-22 | Pauper Infidel: the actual Amulet of Yendor (the Infidel's win-condition item; "Idol of Moloch" is only the Sanctum imbue-event flavor) is placed ON THE FLOOR at the hero's feet on turn 0 - starting inventory stays truly empty. Rationale: pauper (hardfought-5.0 conduct, no master counterpart) strips all of ini_inv, which would make the role unwinnable (no alternate Amulet source exists for Infidels). Rejected: exempt-and-wear carve-out, disallowing the combo | maintainer |
| 2026-07-18 | Corpse-taint parity: racial-vampire/zombie corpse aging restored to master's -100 at all three mon.c make_corpse sites AND the dig.c grave dig-up site (fresh racial-vampire corpse taints ~35% on eat, dug-up grave corpse likewise); upstream 5.0's "use TAINT_AGE" cleanup (408321b4f, -51) can never trip the rotted>5 taint check (needs >=55) = a silent upstream behavior change; mklev.c graveyard site stays TAINT_AGE+1 (master's own literal 51 there = parity) | maintainer |
| 2026-07-18 | rndvault -> themerooms MERGE: unique-to-EvilHack rndvaults become upstream themerms.lua entries (fname->contents, freq->frequency, mindepth->mindiff); the rndvault C registry (~50 sites) is retired unported; overlapping rooms adopt upstream's version; special room TYPES (GARDEN etc.) still port to mkroom.h + sp_lev name table; frequency-tuning pass keeps vault appearance rates roughly master-like or the new cadence gets its own ratification | maintainer |
| 2026-07-16 | zombie_maker() = HYBRID: `if (mon->mcan) return FALSE; return racial_zombie(mon) \|\| mlet == S_LICH;` - master's erac-aware racial semantics (draugr-raced Knights/Monks/Infidels spread zombification) PLUS both upstream 5.0 additions kept (liches raise zombies; cancelled makers do not). Ghoul/skeleton excluded naturally (not MH_ZOMBIE) | maintainer |
| 2026-07-15 | xlogfile forging conduct token stays master's long-style `never_forged_an_artifact` (0.9.x scoreboard-parser continuity) rather than a 5.0 short-style token | maintainer |
| 2026-07-15 | Item economy: v1.0.0 ships the RICH economy (branch keeps upstream `ptr == &mons[PM_SOLDIER]` m_initinv gate - every monster rolls the m_lev magic/gold tail). Master's 2021 `!=` slip stays branch-excluded AND master-unfixed (soldier gate NOT in the backport batch; the lean-economy years remain a master-only accident that dies at cutover) | maintainer |
| 2026-07-15 | Elf/hobbit ELVEN_CHAIN_MAIL: branch stays master-exact (no civilian grant, dead mithril-upgrade check kept textually identical for re-derive ease); revisit as cleanup after v1.0.0 | maintainer |
| 2026-07-15 | Angel weapon: 5.0's rebalance kept (rn2(3) sword-biased + mace +3..+6 compensation) over master's rn2(2)/flat-spe; master's Demonbane gate/Shield of Light/Saint Michael gear ride on top | maintainer |
| 2026-07-15 | Master backport batch APPROVED (4 of 5 candidates; soldier gate excluded per the economy ruling): wish-parse `l = N` length clobber, invent.c crysknife-restore material refresh, mplayer.c mrace ==-vs-& composite masks, mthrowu.c P_BOW-sign dead archer bonus. Prepared as sequential master commits, coordinator stages + maintainer commits | maintainer |
| 2026-07-15 | SUPERSEDES the row above: master (v0.9.3) is FROZEN - no further updates ever ("I should have made this clear from the beginning"). The committed batch was rescinded same day, master reset to c444f6a3a (= fork point = release tip). ALL backport activity to master stops; the branch (v1.0.0) is the sole future. Archive entries BUG-1330..1333 re-pointed to the originating branch commits | maintainer |
| 2026-06-12 | plan §9: v1.0.0, hard cutover, same xlogfile, full parity, local-until-playable, DUMPHTML base | maintainer |
| 2026-07-03 | NAMS merges: ALL THREE pairs merge like upstream - caveman/cavewoman -> PM_CAVE_DWELLER, priest/priestess (role mon) -> PM_CLERIC, incubus/succubus -> PM_AMOROUS_DEMON; M2_MALE/M2_FEMALE dropped (sex assigned at creation); pairs verified field-identical otherwise; monst_to_50.py now auto-merges (refuses + PORT-TODOs any future diverging pair). GOTCHA: upstream has TWO "priest"/"priestess" NAMS records (role PM_CLERIC d=12 vs temple PM_ALIGNED_CLERIC d=15) - converter disambiguates by field score; C port must map role sites (Role_if etc.) to PM_CLERIC, temple/aligned-priest sites to PM_ALIGNED_CLERIC | maintainer |
| | endgame portal naming kept EvilHack-style | per crib sheet |
| 2026-07-12 | local install dir = /usr/games/lib/evilhack50dir for the migration window (hints fragment + config.h default); flip to evilhackdir at v1.0.0 cutover - protects the live 3.6.x local install | maintainer |
| 2026-07-12 | src/glyphs.c collision: upstream glyphs.c keeps its name; EvilHack customcolor registry ports as src/customcolor.c (Phase 4) | plan §6.0.4 default |
| 2026-07-12 | CLI rc flags renamed -nethackrc/-no-nethackrc -> -evilhackrc/-no-evilhackrc (moved to case 'e' in earlyarg.c, removing the case 'n' collision with -n news suppression); Phase 6 must check dgamelaunch/server invocations for the old flag | session |
| 2026-07-12 | NETHACKDIR env var kept unchanged (mirrors master; man page documents it) | mirrors master |
| 2026-07-12 | 5.0-only monster list = pure renames/NAMS triples, none adopted; EvilHack names + gender-neutral noble/royal naming win (matches maintainer's master-side renames) | session, per full-parity mandate |
| 2026-07-12 | HI_ material/monster color aliases defined as the base-16 colors EvilHack's own map_color_256to16() computes (verified against extcolor.c algorithm) until customcolor ports at Phase 4 - NOT hand-picked approximations | session, pilot C precedent |
| 2026-07-12 | trap types ARROW/DART/SLP_GAS/RUST/FIRE/TELEP/MAGIC/POLY_TRAP -> EvilHack's *_TRAP_SET names (trap-kit objects own the plain names); EvilHack's NEW trap types (BOLT/ICE/SPEAR/MAGIC_BEAM_TRAP_SET) deferred to Phase 2 with their mechanics | mirrors EvilHack master |
| 2026-07-12 | G_IGNORE moved 0x8000 -> 0x0100 (call-flag only, never stored in geno; 0x8000 is EvilHack's G_TALG) | session |
| 2026-07-12 | upstream 5.0-only content: ADOPT chain lightning spellbook + shields of drain/shock resistance (re-add slice after the Role-struct port); DECLINE wand of stasis (dormant machinery stays) + dragon scale mail objects (EvilHack scales-fusion supersedes) | maintainer |

## Gotcha shortlist (full list: plan §7 + §11)

1. gX/svX misplacement and missing sfo coupling = SILENT save loss.
2. Enum renumbering vs saved raw indices - append-only during port.
3. NUMMONS/MAX_GLYPH arrays compile clean and overflow at runtime.
4. luac -p passes runtime-broken Lua (bare identifiers) - use the
   crib-sheet grep sweep.
5. Endgame portals: EvilHack self-named + mkportal dlevel-1.
6. The plan was verified 2026-07-02 against tip 219cfb687; the tree
   at adoption WILL be newer - trust the tree, then fix the plan.
7. master weapon.c skill_init carries racial/role adjustment blocks
   (centaur/tortle riding, knight/valkyrie shield, healer/druid/
   knight pet handling, tortle trident, illithid telekinesis) and
   give_may_advance_msg carries per-skill message arms - only the
   illithid telekinesis pieces are on the branch; port each block
   WITH its subsystem or the skill silently misbehaves.
