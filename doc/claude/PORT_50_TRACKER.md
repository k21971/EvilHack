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
      not record_achieve_special)
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
      display does not), resists_magm defends(AD_MAGM)+W_BARDING
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
      shatter, breaktest 3/8) + ARM_BONUS +1/+2/-2 (replaces
      master's armor_bonus - covers hero AC + find_mac); shk
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
      MAGIC_KEY pickup - one TODO in mon.c), iron safes
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
      defeated, icequeen_freed x3 variants, vlad_tower_enter);
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
      (+ master mhitu.c steed-retaliation acceptable_pet_target
      sites; + master skill_init centaur/tortle riding + tortle
      trident racial blocks)
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
      (master menuitemcolor renders BRACKET-SCOPED: item color
      applies only to text inside [] - win/curses cursdial.c
      bracket_color + win/tty wintty.c twin. First consumer
      already live: pet #order toggle rows currently color the
      whole line via the 5.0 add_menu color param; maintainer
      2026-07-17 - only [active]/[inactive] should be colored.
      Port the bracket-scoped rendering into both 5.0 window
      ports, then re-check doorder's rows)
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
| 2026-07-15 | steal.c mdrop_obj() use-after-free: update_mon_extrinsics(mon, obj, ...) runs after flooreffects() may have destroyed obj (worn item dropped over lava/open air deallocates it, then the extrinsics update derefs the freed object). Verbatim in upstream 5.0 at adoption commit 74d1133c2 and in Official_nh50. Fixed on the branch (oprops slice) with master's saved-fields dummy-object pattern; the companion stackobj() merge in the safe path was reviewer-proven NOT to free the dropped obj (merged() keeps the potmp arg = the dropped obj, frees the pre-existing floor stack) so no further change needed | fixed on branch; not yet sent |
| 2026-07-17 | explode.c monster-kill loop mutates its adtyp local (`adtyp = AD_RBRE; /* no corpse */` at explode.c:579, identical in NetHack50 and Official_nh50): after the first no-corpse victim, every later victim of the same blast and the post-loop hero-damage block see AD_RBRE instead of the real damage type. Master's non-mutating `xkflg ? -AD_RBRE : adtyp` inline form ported to the branch (kill-attribution slice) | fixed on branch; not yet sent |

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
| themed rooms | themerms.lua (origin) | EvilHack-only room types | |
| color/truecolor | coloratt/utf8map/glyphs.c | extcolor 24-bit, customcolor registry (RENAME ours) | |
| crash logging | CRASHREPORT (report.c) | crash-log infra, paniclog extras | |
| self-recovery | SELF_RECOVER | emergency .e.gz save + in-game recovery | |
| extended player names | fork | verify parity | |

## Decisions log

| date | decision | by |
|------|----------|----|
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
