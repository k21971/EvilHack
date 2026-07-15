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
  notes; commit pending). Phase 1 COMPLETE 2026-07-13 (gate PASSED)
  except roles-side follow-ups logged in its section
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
- [x] Move this tracker into doc/claude/ on the branch
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
      achievements + 31-bit xlogfile mask full - MAINTAINER
      DECISION achieve2 field needed for EvilHack's 8 extras)
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
      not record_achieve_special)
- [d] sfo_* writers OBSOLETE (Pilot D: historical whole-struct
      saves; critical_sizes[] stamps sizeof automatically; struct
      growth invalidates dev saves only - none kept)
- [ ] gX vs svX decision LOG (append every placement + why):

| Global | gX/svX | Why |
|--------|--------|-----|
| _example: forge heat state_ | sv? | survives save |
| worn_slot_exempt (poly_obj/worn_slot_sanity) | gw | transient diagnostic exemption, Null between moves, never saved; g_init_w resets it per game (better than master's bare global) |

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
      TODO-EVILHACK), wiring at mksobj + mkbox_cnts + mkobj_erosions,
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
- [ ] Object properties (oprops)
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
      saved), m_stash_items + container AI, setmnotwielded2
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
- [ ] Forging (+ FORGE nhl binding for .des->lua levels)
- [ ] Magic chests (+ MAGIC_CHEST nhl binding, crystal chest obj)
- [ ] telekinesis.c + #tk command (pilot did the file; wire cmds)
- [ ] Alignment-abuse subsystem (tiers, #conduct API - see memory
      alignment-abuse-subsystem)
- [ ] Monster AI/equipment: dual-wield, artifact ranking, weapon
      selection (see weapon-selection-architecture memory)
- [ ] Monster spellbook learning (struct emsp)
- [ ] Zombie/were/Draugr mechanics (zombify, binding ritual)
- [ ] Ice Queen branch bosses; Vecna; demon-lord behaviors
      (msummon mechanics memory)
- [ ] Pet order system (P_PET_HANDLING, petstrat, #order)
- [ ] Player races: giant, centaur, hobbit, illithid, tortle, drow
      (light/dark auras - litroom memory), vampire, aasimar (wings,
      light aura, seduction immunity, cross-faction hostility)
- [ ] Roles: Convict, Infidel (penance/Amulet gate memory), Druid
- [ ] Door materials + door-trap variety (+ des.door material=
      binding; 224 TODO-EVILHACK sites in converted levels)
- [ ] Monster `dead` .des flag -> des.monster dead= binding
      (must call mondied(); 13 sites)
- [ ] MINDEPTH: directive binding (17 sites)
- [ ] rndvault room system + "garden"/other room types (mkroom.h
      GARDEN=17 etc. -> sp_lev room-name table)
- [ ] #shield, other added commands (sweep cmd.c delta)
- [ ] Bag of treasure/devouring rules; #tip interactions
- [ ] Steed/saddle/barding architecture (memory file)
- [ ] Shop pricing racial adjustments (shk_racial_adjustments)
- [ ] Murder mechanic; Croesus AD_GOLD; seduction attacks table
- [ ] Endgame randomized plane order + mkportal dlevel-1 semantics
      (MUST land with Phase 3 endgame.lua - see crib sheet gotcha)
- [ ] SWEEP: FEATURES_ARCHIVE.md Features Index (57 entries) +
      dated 2026 sections (~100) - tick each off as covered by the
      subsystems above or added here; nothing skipped silently
- [ ] Per-subsystem: BUGFIXES_ARCHIVE.md keyword sweep as it lands

## Phase 3 - Levels and quests (233 lua files from 36 .des)

Workflow + gotchas: /root/nh50-port-preview/exemplar/DES_TO_LUA_CRIB.md.
Delta-diff strategy for EvilHack-modified vanilla levels (plan §6 Ph3).
Statuses per file: C=converted, P=polished, W=wizmode-walked.

| .des | levels | mode | C | P | W | notes |
|------|--------|------|---|---|---|-------|
| bigroom.des | 10 | delta-diff | | | | |
| castle.des | 3 | delta-diff | | | | wand recharged:1 now converts |
| endgame.des | 5 | delta-diff | | | | PORTAL self-name + mkportal semantics! |
| gehennom.des | 14 | delta-diff | | | | |
| knox.des | 3 | delta-diff | | | | |
| medusa.des | 4 | delta-diff | | | | |
| mines.des | 14 | delta-diff | | | | |
| oracle.des | 3 | delta-diff | | | | |
| sokoban.des | 24 | delta-diff | | | | |
| tower.des | 4 | delta-diff | | | | |
| yendor.des | 4 | delta-diff | | | | |
| forest.des | 1 | from-scratch | | | | 7 top-level TERRAIN:random TODOs |
| goblintown.des | 3 | from-scratch | | | | |
| hdgn.des | 1 | from-scratch | | | | |
| icequeen.des | 3 | from-scratch | | | | |
| nymph.des | 1 | from-scratch | x | x | | EXEMPLAR (luac-clean; W needs branch content) |
| purgatory.des | 2 | from-scratch | | | | dead-shopkeeper flag |
| vaults.des | 46 | from-scratch | | | | rndvault; vlt-0004/0045 HAND-CONVERTED 2026-07-03 (exemplar/vaults/, luac-clean, nil-swept; rndvault type/flag/MINDEPTH = TODO-EVILHACK until Phase 2); remaining 44 convert cleanly - polish with the vlt-0045 ROOMDOOR idiom |
| vecna.des | 3 | from-scratch | | | | |
| Arch..Wizard (13 vanilla quests) | 65 | delta-diff | | | | Val-strt done as the delta-diff EXEMPLAR (exemplar/Val-strt.lua; Norn inv reconcile + magic-chest TODO) |
| Convict.des | 5 | from-scratch | | | | |
| DKnight.des | 5 | from-scratch | | | | align_roles mechanism first |
| Druid.des | 5 | from-scratch | | | | |
| Infidel.des | 5 | from-scratch | | | | |

- [ ] dungeon.def -> dat/dungeon.lua (branches: Ice Queen etc.)
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
      additions on upstream themerms.lua
- [ ] nhl bindings from Phase 2 wired: MAGIC_CHEST, FORGE,
      MINDEPTH, dead=, door material=, room types
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
- [ ] menuitemcolor / status branch-labels / UI extras sweep
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
| 2026-07-12 | shk.c shop_keeper() impossible() format typo `&d` -> `%d` (code only compiles when NH_DEVEL_STATUS is WIP/BETA, so upstream's POSTRELEASE builds never saw the -Wformat-extra-args warning) | fixed on branch; not yet sent |
| 2026-07-12 | util/sfctool.c validate() call missing the 4th arg added by the u.ustuck save/restore fixes (optional target, upstream CI never builds it); also two -DSFCTOOL -Wunused-function warnings in files.c (set_whereisfile/write_whereis) | validate fix on branch; warnings not chased |
| 2026-07-13 | WIP-build false positive: shop_keeper()'s "untrustworthy null shkp" diagnostic (shk.c:1082) fires during throne-room generation - fill_zoo COURT chest fill -> add_to_container gold merge -> obfree's unconditional shop_keeper(*u.ushops) fallback (shk.c:1227) probes rmno '\0' while level_status.making. Repro: generate levels until a COURT spawns (4 merges = 4 reports at one newgame observed). Suggested fix: diagnostic should skip rmno < ROOMOFFSET - a no-room probe's NULL is definitionally correct | traced on branch (gdb bt); code byte-identical upstream; not yet sent |

## Master re-derive queue

Mechanism: `git log evilhack-50-fork-point..master --oneline`; every
commit gets re-derived by hand on the branch (NEVER `git merge
master`) and ticked here. Seeded at adoption 2026-07-12: EMPTY
(fork point c444f6a3a == master tip; nothing outstanding).

| master commit | subject | re-derived as | status |
|---------------|---------|---------------|--------|

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
| themed rooms | themerms.lua (origin) | EvilHack-only room types | |
| color/truecolor | coloratt/utf8map/glyphs.c | extcolor 24-bit, customcolor registry (RENAME ours) | |
| crash logging | CRASHREPORT (report.c) | crash-log infra, paniclog extras | |
| self-recovery | SELF_RECOVER | emergency .e.gz save + in-game recovery | |
| extended player names | fork | verify parity | |

## Decisions log

| date | decision | by |
|------|----------|----|
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
