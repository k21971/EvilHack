
#define MCF_NONE     0x0000
#define MCF_INDIRECT 0x0001 /* untargeted/indirect spell */
#define MCF_SIGHT    0x0002 /* monster needs to see hero */
#define MCF_HOSTILE  0x0004 /* cast by hostile monsters only */
#define MCF_MELEE    0x0008 /* monster casters need the target adjacent */

#if defined(MCASTU_ENUM)
#define MONSPELL(def, lvl, flags) MCAST_##def
#elif defined(MCASTU_INIT)
#define MONSPELL(def, lvl, flags) { lvl, flags }
#elif defined(DUMP_MCASTU_ENUM1)
#define MONSPELL(def, lvl, flags) MCAST_DUMPENUM_##def
#elif defined(DUMP_MCASTU_ENUM2)
#define MONSPELL(def, lvl, flags) { MCAST_DUMPENUM_##def, #def }
#endif

/* new entries must be appended at the end of this list */
MONSPELL(PSI_BOLT,     0, MCF_HOSTILE|MCF_SIGHT),
MONSPELL(OPEN_WOUNDS,  0, MCF_HOSTILE|MCF_SIGHT),
MONSPELL(CURE_SELF,    1, MCF_INDIRECT),
MONSPELL(HASTE_SELF,   2, MCF_INDIRECT),
MONSPELL(CONFUSE_YOU,  2, MCF_HOSTILE|MCF_SIGHT|MCF_MELEE),
MONSPELL(STUN_YOU,     3, MCF_HOSTILE|MCF_SIGHT|MCF_MELEE),
MONSPELL(DISAPPEAR,    4, MCF_INDIRECT),
MONSPELL(PARALYZE,     4, MCF_HOSTILE|MCF_SIGHT|MCF_MELEE),
MONSPELL(BLIND_YOU,    6, MCF_HOSTILE|MCF_SIGHT|MCF_MELEE),
MONSPELL(WEAKEN_YOU,   6, MCF_HOSTILE|MCF_SIGHT|MCF_MELEE),
MONSPELL(DESTRY_ARMR,  8, MCF_HOSTILE|MCF_SIGHT|MCF_MELEE),
MONSPELL(INSECTS,      8, MCF_HOSTILE|MCF_INDIRECT|MCF_SIGHT),
MONSPELL(CURSE_ITEMS, 10, MCF_HOSTILE|MCF_SIGHT|MCF_MELEE),
MONSPELL(LIGHTNING,   11, MCF_HOSTILE|MCF_SIGHT),
MONSPELL(FIRE_PILLAR, 12, MCF_HOSTILE|MCF_SIGHT),
MONSPELL(GEYSER,      13, MCF_HOSTILE|MCF_SIGHT),
MONSPELL(AGGRAVATION, 13, MCF_INDIRECT|MCF_HOSTILE|MCF_SIGHT|MCF_MELEE),
MONSPELL(SUMMON_MONS, 15, MCF_HOSTILE|MCF_INDIRECT|MCF_SIGHT|MCF_MELEE),
MONSPELL(CLONE_WIZ,   18, MCF_HOSTILE|MCF_INDIRECT|MCF_SIGHT|MCF_MELEE),
MONSPELL(DEATH_TOUCH, 20, MCF_HOSTILE|MCF_SIGHT|MCF_MELEE),
/* spell learned from a spellbook; selection and targeting gates are
   handled dynamically in mcastu.c, not via the flags here */
MONSPELL(LEARNED_SPELL, 0, MCF_NONE),
/* charm-monster special vs tame monsters; castmm() only */
MONSPELL(UNTAME_PET,    0, MCF_NONE),
/* druidic self-wards; choose_monster_spell() substitutes them for
   the druid caster ring in place of same-level clerical picks */
MONSPELL(BARKSKIN,      2, MCF_INDIRECT),
MONSPELL(STONESKIN,     6, MCF_INDIRECT),
/* area acid blast; wizard-ladder pick for casters of level 15 up */
MONSPELL(ACID_BLAST,   14, MCF_HOSTILE|MCF_INDIRECT|MCF_SIGHT),
/* area fire and ice blasts; the wizard ladder carries a single bolt
   rung (fire) and choose_bolt_spell() rolls the actual element, both
   there and on the ladder's psi-bolt floor */
MONSPELL(FIRE_BOLT,   23, MCF_HOSTILE|MCF_INDIRECT|MCF_SIGHT),
MONSPELL(ICE_BOLT,    23, MCF_HOSTILE|MCF_INDIRECT|MCF_SIGHT),
/* elemental vulnerability debuff; clerical-ladder pick */
MONSPELL(VULN_YOU,     5, MCF_HOSTILE|MCF_INDIRECT|MCF_SIGHT),
/* summon an aligned servant; clerical-ladder pick, melee range only */
MONSPELL(SUMMON_MINION, 14, MCF_HOSTILE|MCF_INDIRECT|MCF_SIGHT|MCF_MELEE),
/* reflection self-ward; the wizard ladder's aggravation rung rolls
   between the two */
MONSPELL(REFLECTION,   13, MCF_INDIRECT),
/* protection self-ward; clerical-ladder pick sharing its rung with
   the confusion spell */
MONSPELL(PROTECTION,    2, MCF_INDIRECT),
/* cancel the target's magic; reached through the wizard ladder's
   summoning rung roll. A directed spell, unlike the other late
   wizard picks */
MONSPELL(CANCELLATION, 15, MCF_HOSTILE|MCF_SIGHT),
/* summon undead at the caster; clerical-ladder pick, melee range,
   undead/demonic/evil-priest casters only */
MONSPELL(CALL_UNDEAD,   8, MCF_HOSTILE|MCF_INDIRECT|MCF_SIGHT|MCF_MELEE),

#undef MONSPELL
