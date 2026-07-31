/* NetHack 5.0	mon.c	$NHDT-Date: 1781062909 2026/06/09 19:41:49 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.634 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Derek S. Ray, 2015. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "mfndpos.h"

staticfn void pet_sanity_check(struct monst *, const char *);
staticfn void sanity_check_single_mon(struct monst *, boolean, const char *);
staticfn struct obj *make_corpse(struct monst *, unsigned);
staticfn int minliquid_core(struct monst *);
staticfn void m_calcdistress(struct monst *);
staticfn boolean monlineu(struct monst *, int, int);
staticfn long mm_2way_aggression(struct monst *, struct monst *);
staticfn long mm_displacement(struct monst *, struct monst *);
staticfn void icequeenrevive(struct monst *);
staticfn void mon_leaving_level(struct monst *);
staticfn void m_detach(struct monst *, struct permonst *, boolean);
staticfn void set_mon_min_mhpmax(struct monst *, int);
staticfn void lifesaved_monster(struct monst *);
staticfn boolean vamprises(struct monst *);
staticfn void logdeadmon(struct monst *, int, boolean, struct monst *);
staticfn void anger_quest_guardians(struct monst *);
staticfn boolean ok_to_obliterate(struct monst *);
staticfn void m_respond_shrieker(struct monst *);
staticfn void m_respond_medusa(struct monst *);
staticfn void m_respond_zombie(struct monst *);
staticfn void peacefuls_respond(struct monst *, int);
staticfn void wake_nearto_core(coordxy, coordxy, int, boolean);
staticfn void m_restartcham(struct monst *);
staticfn boolean restrap(struct monst *);
staticfn int pick_animal(void);
staticfn int pickvampshape(struct monst *);
staticfn boolean isspecmon(struct monst *);
staticfn boolean validspecmon(struct monst *, int);
staticfn int wiz_force_cham_form(struct monst *);
staticfn struct permonst *accept_newcham_form(struct monst *, int);
staticfn void kill_eggs(struct obj *) NO_NNARGS;
staticfn void pacify_guard(struct monst *);
staticfn short m_chooserace(unsigned long);

extern const struct shclass shtypes[]; /* defined in shknam.c */

/* values for ga.accidental_bump: how the hero accidentally angered
   a peaceful, for setmangry() */
#define ACCIDENT_NONE 0
#define ACCIDENT_BUMP 1      /* walked into an unseen monster */
#define ACCIDENT_COLLISION 2 /* involuntary knockback collision */

#define LEVEL_SPECIFIC_NOCORPSE(mdat) \
    (Is_rogue_level(&u.uz)            \
     || !svl.level.flags.deathdrops    \
     || (svl.level.flags.graveyard && is_undead(mdat) && rn2(3)))

/* transient kill attribution for mondead()'s livelog handling; both are
   consumed (snapshotted and cleared) on entry to mondead() and never
   saved */
static boolean player_killed = FALSE;
static struct monst *pet_killer = (struct monst *) 0;

/* set the pet responsible for a kill (call before monkilled) */
void
set_pet_killer(struct monst *mtmp)
{
    pet_killer = mtmp;
}

#if 0   /* potentially of historical interest */
/* part of the original warning code which was replaced in 3.3.1 */
const char *warnings[] = {
    "white", "pink", "red", "ruby", "purple", "black"
};
#endif /* 0 */

staticfn void
pet_sanity_check(
    struct monst *mtmp,
    const char *msgarg)
{
    if (has_edog(mtmp)) {
        struct edog *edog = EDOG(mtmp);

        if (edog->droptime > svm.moves)
            impossible("insane pet #%u has droptime (%ld)"
                       " in the future (%ld) (%s)",
                       mtmp->m_id, edog->droptime, svm.moves, msgarg);
        /* TODO: verify some of the other edog fields */
    }
}

staticfn void
sanity_check_single_mon(
    struct monst *mtmp,
    boolean chk_geno,
    const char *msg)
{
    struct permonst *mptr = mtmp->data;
    coordxy mx = mtmp->mx, my = mtmp->my;

    if (!mptr || mptr < &mons[LOW_PM] || mptr > &mons[HIGH_PM]) {
        /* most sanity checks issue warnings if they detect a problem,
           but this would be too extreme to keep going */
        panic("illegal mon data %s; mnum=%d (%s)",
              fmt_ptr((genericptr_t) mptr), mtmp->mnum, msg);
        /*NOTREACHED*/
    } else {
        int mndx = monsndx(mptr);

        if (mtmp->mnum != mndx) {
            impossible("monster mnum=%d, monsndx=%d (%s)",
                       mtmp->mnum, mndx, msg);
            mtmp->mnum = mndx;
        }
        /* check before DEADMONSTER() because dead monsters should still
           have sane mhpmax */
        if (mtmp->mhpmax < 1
            /* Gremlins don't obey the (mhpmax >= m_lev) rule so disable
             * this check, at least for the time being.  We could skip it
             * when the cloned flag is set, but the original gremlin would
             * still be an issue.
            || mtmp->mhpmax < (int) mtmp->m_lev
             */
            || mtmp->mhp > mtmp->mhpmax)
            impossible("%s: level %d %s #%u [%s] has %d cur HP, %d max HP",
                       msg, (int) mtmp->m_lev, mptr->pmnames[NEUTRAL],
                       mtmp->m_id, fmt_ptr((genericptr_t) mtmp),
                       mtmp->mhp, mtmp->mhpmax);
        if (DEADMONSTER(mtmp)) {
#if 0
            /* bad if not fmon list or if not vault guard */
            if (strcmp(msg, "fmon") || !mtmp->isgd)
                impossible("dead monster on %s; %s at <%d,%d>",
                           msg, mptr->pmnames[NEUTRAL], mx, my);
#endif
            return;
        }
        if (chk_geno && (svm.mvitals[mndx].mvflags & G_GENOD) != 0)
            impossible("genocided %s in play (%s)",
                       pmname(mptr, Mgender(mtmp)), msg);
        if (mtmp->mtame && !mtmp->mpeaceful)
            impossible("tame %s is not peaceful (%s)",
                       pmname(mptr, Mgender(mtmp)), msg);
    }
    if (mtmp->isshk && !has_eshk(mtmp))
        impossible("shk without eshk (%s)", msg);
    if (mtmp->ispriest && !has_epri(mtmp))
        impossible("priest without epri (%s)", msg);
    if (mtmp->isgd && !has_egd(mtmp))
        impossible("guard without egd (%s)", msg);
    if (mtmp->isminion && !has_emin(mtmp))
        impossible("minion without emin (%s)", msg);
    /* guardian angel on astral level is tame but has emin rather than edog */
    if (mtmp->mtame) {
        if (!has_edog(mtmp) && !mtmp->isminion)
            impossible("pet without edog (%s)", msg);
        else
            pet_sanity_check(mtmp, msg);
    }
    /* steed should be tame and saddled */
    if (mtmp == u.usteed) {
        const char *ns, *nt = !mtmp->mtame ? "not tame" : 0;

        ns = !m_carrying(mtmp, SADDLE) ? "no saddle"
             : !which_armor(mtmp, W_SADDLE) ? "saddle not worn"
               : 0;
        if (ns || nt)
            impossible("steed: %s%s%s (%s)",
                       ns ? ns : "", (ns && nt) ? ", " : "", nt ? nt : "",
                       msg);
    }

    if (mtmp->mtrapped) {
        if (mtmp->wormno) {
            ; /* TODO: how to check worm in trap? */
        } else if (!t_at(mx, my))
            impossible("trapped without a trap (%s)", msg);
    }
    /* monst->mfrozen is difficult to deal with--it's used for paralysis,
       for temporary sleep, and for being busy (usually donning armor);
       code that sets mfrozen needs to also clear mcanmove, otherwise the
       helpless() test will be unreliable */
    if (mtmp->mfrozen && mtmp->mcanmove)
        impossible("frozen monster [%s%s] is able to move (%s)",
                   mtmp->mtame ? "tame " : mtmp->mpeaceful ? "peaceful " : "",
                   pmname(mptr, Mgender(mtmp)), msg);

    /* monster is hiding? */
    if (mtmp->mundetected) {
        struct trap *t;

        if (!isok(mx, my)) /* caller will have checked this but not fixed it */
            mx = my = 0;
        if (mtmp == u.ustuck)
            impossible("hiding monster stuck to you (%s)", msg);
        if (m_at(mx, my) == mtmp && hides_under(mptr) && !OBJ_AT(mx, my))
            impossible("mon hiding under nonexistent obj (%s)", msg);
        if (mptr->mlet == S_EEL
            && !(is_pool(mx, my) && !Is_waterlevel(&u.uz)))
            impossible("eel hiding %s (%s)",
                       !Is_waterlevel(&u.uz) ? "out of water"
                                             : "on Plane of Water", msg);
        if (ceiling_hider(mptr)
            /* normally !accessible would be overridable with passes_walls,
               but not for hiding on the ceiling */
            && (!has_ceiling(&u.uz)
                || !(levl[mx][my].typ == POOL
                     || levl[mx][my].typ == MOAT
                     || levl[mx][my].typ == WATER
                     || levl[mx][my].typ == LAVAPOOL
                     || levl[mx][my].typ == LAVAWALL
                     || accessible(mx, my))))
            impossible("ceiling hider hiding %s (%s)",
                       !has_ceiling(&u.uz) ? "without ceiling"
                                           : "in solid stone",
                       msg);
        if (mtmp->mtrapped && (t = t_at(mx, my)) != 0 && !is_pit(t->ttyp))
            impossible("hiding while trapped in a non-pit (%s)", msg);
    } else if (M_AP_TYPE(mtmp) != M_AP_NOTHING) {
        boolean is_mimic = (mptr->mlet == S_MIMIC);
        const char *what = (M_AP_TYPE(mtmp) == M_AP_FURNITURE) ? "furniture"
                           : (M_AP_TYPE(mtmp) == M_AP_MONSTER) ? "a monster"
                             : (M_AP_TYPE(mtmp) == M_AP_OBJECT) ? "an object"
                               : "something strange";

        if (!strcmp(msg, "migr")) {
            if (M_AP_TYPE(mtmp) != M_AP_MONSTER)
                impossible("migrating %s mimicking %s %s",
                           is_mimic ? "mimic" : "monster", what, msg);
        } else if (Protection_from_shape_changers) {
            impossible(
                "mimic%s concealed as %s despite Prot-from-shape-changers %s",
                       is_mimic ? "" : "ker", what, msg);
        }
        /* the Wizard's clone after "double trouble" starts out mimicking
           some other monster; pet's quickmimic effect can temporarily take
           on furniture, object, or monster shape, but only until the pet
           finishes eating a mimic corpse */
        if (!(is_mimic || mtmp->meating
              || (mtmp->iswiz && M_AP_TYPE(mtmp) == M_AP_MONSTER)))
            impossible("non-mimic (%s) posing as %s (%s)",
                       mptr->pmnames[NEUTRAL], what, msg);
#if 0   /* mimics who end up in strange locations do still hide while there */
        if (!(accessible(mx, my) || passes_walls(mptr))) {
            char buf[BUFSZ];
            const char *typnam = levltyp_to_name(levl[mx][my].typ);

            if (!typnam) {
                Sprintf(buf, "[%d]", levl[mx][my].typ);
                typnam = buf;
            }
            impossible("mimic%s concealed in inaccessible location: %s (%s)",
                       is_mimic ? "" : "ker", typnam, msg);
        }
#endif
    }
    if (mtmp->mleashed) {
        if (!get_mleash(mtmp))
            impossible("monst %u: leashed but no leash for %s",
                       mtmp->m_id, mon_pmname(mtmp));
        else if (!mtmp->mtame)
            impossible("monst %u: leashed but not tame %s",
                       mtmp->m_id, mon_pmname(mtmp));
#if 0
        /* after hero moves, leashed mon won't necessarily pass 'm_next2u()'
           test; 90 is farthest observed distance with expert jumping spell
           when very slow mon is already several steps away and hero jumps in
           opposite direction (if hero teleports, leashed mon moves adjacent
           immediately; knockback has shorter range than magical jumping) */
        else if (distu(mtmp->mx, mtmp->my) > 90) /*if (!m_next2u(mtmp))*/
            impossible("monst %u: leashed but not next to you (%d)",
                       mtmp->m_id, distu(mtmp->mx, mtmp->my));
#endif
    }
}

void
mon_sanity_check(void)
{
    coordxy x, y;
    struct monst *mtmp, *m;

    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        /* dead monsters should still have sane data */
        sanity_check_single_mon(mtmp, TRUE, "fmon");
        if (DEADMONSTER(mtmp) && !mtmp->isgd)
            continue;

        x = mtmp->mx, y = mtmp->my;
        if (!isok(x, y) && !(mtmp->isgd && x == 0 && y == 0)) {
            impossible("mon (%s) claims to be at <%d,%d>?",
                       fmt_ptr((genericptr_t) mtmp), x, y);
        } else if (mtmp == u.usteed) {
            /* steed is in fmon list but not on the map; its
               <mx,my> coordinates should match hero's location */
            if (x != u.ux || y != u.uy)
                impossible("steed (%s) claims to be at <%d,%d>?",
                           fmt_ptr((genericptr_t) mtmp), x, y);
        } else if (mtmp->ridden_by) {
            /* a monster's steed is kept on the fmon list but off the map,
               with its coordinates synced to its rider's */
            continue;
        } else if ((m = svl.level.monsters[x][y]) != mtmp) {
            /* the exemption below is inert: a mounted steed is removed
               from the map, so the occupant here can never be a steed
               of mtmp; kept for master parity */
            if (!m || m->ridden_by != mtmp->m_id)
                impossible("mon (%s) at <%d,%d> is not there!",
                           fmt_ptr((genericptr_t) mtmp), x, y);
        } else if (mtmp->wormno) {
            sanity_check_worm(mtmp);

        /* some temp mstate bits can be expected for a mon on fmon, as part of
           removing it, but DEADMONSTER check above should skip those. */
        } else if (mon_offmap(mtmp)) {
            impossible("floor mon (%s) with mstate set to 0x%08lx",
                       fmt_ptr((genericptr_t) mtmp), mtmp->mstate);
        }
    }

    for (x = 1; x < COLNO; x++)
        for (y = 0; y < ROWNO; y++)
            if ((mtmp = svl.level.monsters[x][y]) != 0) {
                for (m = fmon; m; m = m->nmon)
                    if (m == mtmp)
                        break;
                if (!m)
                    impossible("map mon (%s) at <%d,%d> not in fmon list!",
                               fmt_ptr((genericptr_t) mtmp), x, y);
                else if (mtmp == u.usteed)
                    impossible("steed (%s) is on the map at <%d,%d>!",
                               fmt_ptr((genericptr_t) mtmp), x, y);
                else if ((mtmp->mx != x || mtmp->my != y)
                         && mtmp->data != &mons[PM_LONG_WORM]
                         && !mtmp->ridden_by)
                    impossible("map mon (%s) at <%d,%d> is found at <%d,%d>?",
                               fmt_ptr((genericptr_t) mtmp),
                               mtmp->mx, mtmp->my, x, y);
            }

    for (mtmp = gm.migrating_mons; mtmp; mtmp = mtmp->nmon) {
        sanity_check_single_mon(mtmp, FALSE, "migr");

        if ((mtmp->mstate
             & ~(MON_MIGRATING | MON_LIMBO | MON_ENDGAME_MIGR | MON_OFFMAP))
            != 0L
            || !(mtmp->mstate & MON_MIGRATING))
            impossible("migrating mon (%s) with mstate set to 0x%08lx",
                       fmt_ptr((genericptr_t) mtmp), mtmp->mstate);
    }

    wormno_sanity_check(); /* test for bogus worm tail */
}

/* Would monster be OK with poison gas? */
/* Does not check for actual poison gas at the location. */
/* Returns one of M_POISONGAS_foo */
int
m_poisongas_ok(struct monst *mtmp)
{
    int px, py;
    boolean is_you = (mtmp == &gy.youmonst);

    /* Non living, non breathing, immune monsters are not concerned */
    if (nonliving(mtmp->data) || is_vampshifter(mtmp)
        || breathless(mtmp->data) || immune_poisongas(mtmp->data))
        return M_POISONGAS_OK;
    /* not is_swimmer(); assume that non-fish are swimming on
       the surface and breathing the air above it periodically
       unless located at water spot on plane of water */
    px = is_you ? u.ux : mtmp->mx;
    py = is_you ? u.uy : mtmp->my;
    if ((mtmp->data->mlet == S_EEL || Is_waterlevel(&u.uz))
        && is_pool(px, py))
        return M_POISONGAS_OK;
    /* exclude monsters with poison gas breath attack:
       adult green dragon and Chromatic Dragon (and iron golem,
       but nonliving() and breathless() tests also catch that) */
    if (attacktype_fordmg(mtmp->data, AT_BREA, AD_DRST)
        || attacktype_fordmg(mtmp->data, AT_BREA, AD_RBRE))
        return M_POISONGAS_OK;
    if (is_you && (u.uinvulnerable || Breathless || Underwater))
        return M_POISONGAS_OK;
    if (is_you ? Poison_resistance : resists_poison(mtmp))
        return M_POISONGAS_MINOR;
    return M_POISONGAS_BAD;
}

/* return True if mon is capable of converting other monsters into zombies;
   true zombies by race (which covers racial overlays such as draugr
   knights) and all liches qualify; ghouls and skeletons are Z-class but
   not zombies by race, so they are excluded naturally */
boolean
zombie_maker(struct monst *mon)
{
    if (mon->mcan)
        return FALSE;

    return (racial_zombie(mon) || mon->data->mlet == S_LICH);
}

/* Return monster index of zombie monster which this monster could
   be turned into, or NON_PM if it doesn't have a direct counterpart.
   Sort of the zombie-specific inverse of undead_to_corpse. */
int
zombie_form(struct permonst *pm)
{
    switch (pm->mlet) {
    case S_KOBOLD:
        return PM_KOBOLD_ZOMBIE;
    case S_ORC:
        return PM_ORC_ZOMBIE;
    case S_GIANT:
        if (pm == &mons[PM_ETTIN])
            return PM_ETTIN_ZOMBIE;
        return PM_GIANT_ZOMBIE;
    case S_HUMAN:
    case S_KOP:
        if (is_elf(pm))
            return PM_ELF_ZOMBIE;
        if (is_drow(pm))
            return PM_DROW_ZOMBIE;
        if (is_dwarf(pm))
            return PM_DWARF_ZOMBIE;
        if (is_gnome(pm))
            return PM_GNOME_ZOMBIE;
        if (is_orc(pm))
            return PM_ORC_ZOMBIE;
        if (is_hobbit(pm))
            return PM_HOBBIT_ZOMBIE;
        if (is_giant(pm))
            return PM_GIANT_ZOMBIE;
        if (is_illithid(pm))
            return NON_PM;
        return PM_HUMAN_ZOMBIE;
    case S_HUMANOID:
        if (is_dwarf(pm))
            return PM_DWARF_ZOMBIE;
        if (is_hobbit(pm))
            return PM_HOBBIT_ZOMBIE;
        if (is_gnoll(pm))
            return PM_GNOLL_WITHERLING;
        else
            break;
    case S_GNOME:
        return PM_GNOME_ZOMBIE;
    case S_ZOMBIE:
        /* draugr arise again as draugr; other zombies stay dead */
        if (pm == &mons[PM_DRAUGR])
            return PM_DRAUGR;
    }
    return NON_PM;
}

/* convert the monster index of an undead to its living counterpart */
int
undead_to_corpse(int mndx)
{
    switch (mndx) {
    case PM_KOBOLD_ZOMBIE:
    case PM_KOBOLD_MUMMY:
        mndx = PM_KOBOLD;
        break;
    case PM_DWARF_ZOMBIE:
    case PM_DWARF_MUMMY:
        mndx = PM_DWARF;
        break;
    case PM_GNOME_ZOMBIE:
    case PM_GNOME_MUMMY:
        mndx = PM_GNOME;
        break;
    case PM_ORC_ZOMBIE:
    case PM_ORC_MUMMY:
        mndx = PM_ORC;
        break;
    case PM_ELF_ZOMBIE:
    case PM_ELF_MUMMY:
        mndx = PM_ELF;
        break;
    case PM_DROW_ZOMBIE:
    case PM_DROW_MUMMY:
        mndx = PM_DROW;
        break;
    case PM_HOBBIT_ZOMBIE:
    case PM_HOBBIT_MUMMY:
        mndx = PM_HOBBIT;
        break;
    case PM_GNOLL_WITHERLING:
        mndx = PM_GNOLL;
        break;
    case PM_VAMPIRE:
    case PM_VAMPIRE_SOVEREIGN:
    case PM_VAMPIRE_NOBLE:
    case PM_VAMPIRE_ROYAL:
    case PM_VAMPIRE_MAGE:
    case PM_HUMAN_ZOMBIE:
    case PM_HUMAN_MUMMY:
        mndx = PM_HUMAN;
        break;
    case PM_GIANT_ZOMBIE:
    case PM_GIANT_MUMMY:
        mndx = PM_GIANT;
        break;
    case PM_ETTIN_ZOMBIE:
    case PM_ETTIN_MUMMY:
        mndx = PM_ETTIN;
        break;
    default:
        break;
    }
    return mndx;
}

/* Convert the monster index of some monsters (such as quest guardians)
 * to their generic species type.
 *
 * Return associated character class monster, rather than species
 * if mode is 1.
 */
int
genus(int mndx, int mode)
{
    switch (mndx) {
    /* Quest guardians */
    case PM_STUDENT:
        mndx = mode ? PM_ARCHEOLOGIST : PM_HUMAN;
        break;
    case PM_CHIEFTAIN:
        mndx = mode ? PM_BARBARIAN : PM_HUMAN;
        break;
    case PM_NEANDERTHAL:
        mndx = mode ? PM_CAVE_DWELLER : PM_HUMAN;
        break;
    case PM_ATTENDANT:
        mndx = mode ? PM_HEALER : PM_HUMAN;
        break;
    case PM_PAGE:
        mndx = mode ? PM_KNIGHT : PM_HUMAN;
        break;
    case PM_ABBOT:
        mndx = mode ? PM_MONK : PM_HUMAN;
        break;
    case PM_ACOLYTE:
        mndx = mode ? PM_CLERIC : PM_HUMAN;
        break;
    case PM_HUNTER:
        mndx = mode ? PM_RANGER : PM_HUMAN;
        break;
    case PM_THUG:
        mndx = mode ? PM_ROGUE : PM_HUMAN;
        break;
    case PM_ROSHI:
        mndx = mode ? PM_SAMURAI : PM_HUMAN;
        break;
    case PM_GUIDE:
        mndx = mode ? PM_TOURIST : PM_HUMAN;
        break;
    case PM_APPRENTICE:
        mndx = mode ? PM_WIZARD : PM_HUMAN;
        break;
    case PM_WARRIOR:
        mndx = mode ? PM_VALKYRIE : PM_HUMAN;
        break;
    default:
        if (ismnum(mndx)) {
            struct permonst *ptr = &mons[mndx];

            if (is_human(ptr))
                mndx = PM_HUMAN;
            else if (is_elf(ptr))
                mndx = PM_ELF;
            else if (is_dwarf(ptr))
                mndx = PM_DWARF;
            else if (is_gnome(ptr))
                mndx = PM_GNOME;
            else if (is_orc(ptr))
                mndx = PM_ORC;
            else if (is_zombie(ptr))
                mndx = PM_DRAUGR;
        }
        break;
    }
    return mndx;
}

/* return monster index if chameleon, or NON_PM if not */
int
pm_to_cham(int mndx)
{
    int mcham = NON_PM;

    /*
     * As of 3.6.0 we just check M2_SHAPESHIFTER instead of having a
     * big switch statement with hardcoded shapeshifter types here.
     */
    if (ismnum(mndx) && is_shapeshifter(&mons[mndx]))
        mcham = mndx;
    return mcham;
}

/* for deciding whether corpse will carry along full monster data */
#define KEEPTRAITS(mon)                                                  \
    ((mon)->isshk || (mon)->mtame || unique_corpstat((mon)->data)        \
     || is_reviver((mon)->data)                                          \
        /* normally quest leader will be unique, */                      \
        /* but he or she might have been polymorphed  */                 \
     || (mon)->m_id == svq.quest_status.leader_m_id                       \
        /* special cancellation handling for these */                    \
     || (dmgtype((mon)->data, AD_SEDU) || dmgtype((mon)->data, AD_SSEX)))

/* Creates a monster corpse, a "special" corpse, or nothing if it doesn't
 * leave corpses.  Monsters which leave "special" corpses should have
 * G_NOCORPSE set in order to prevent wishing for one, finding tins of one,
 * etc....
 */
staticfn struct obj *
make_corpse(struct monst *mtmp, unsigned int corpseflags)
{
    struct permonst *mdat = mtmp->data;
    int num;
    struct obj *obj = (struct obj *) 0;
    struct obj *otmp = (struct obj *) 0;
    coordxy x = mtmp->mx, y = mtmp->my;
/*    int mndx = monsndx(mdat); */
    enum monnums mndx = monsndx(mdat);
    unsigned corpstatflags = corpseflags;
    boolean burythem = ((corpstatflags & CORPSTAT_BURIED) != 0);

    if (mtmp->female)
        corpstatflags |= CORPSTAT_FEMALE;
    else if (!is_neuter(mtmp->data))
        corpstatflags |= CORPSTAT_MALE;

    switch (mndx) {
    case PM_GRAY_DRAGON:
    case PM_SILVER_DRAGON:
    case PM_SHIMMERING_DRAGON:
    case PM_RED_DRAGON:
    case PM_ORANGE_DRAGON:
    case PM_WHITE_DRAGON:
    case PM_BLACK_DRAGON:
    case PM_BLUE_DRAGON:
    case PM_GREEN_DRAGON:
    case PM_GOLD_DRAGON:
    case PM_SEA_DRAGON:
    case PM_YELLOW_DRAGON:
    case PM_SHADOW_DRAGON:
    case PM_CELESTIAL_DRAGON:
        /* Make dragon scales.  This assumes that the order of the
           dragons is the same as the order of the scales.
           If the dragon is a pet, no scales generated. */
        if (!mtmp->mtame && !rn2(mtmp->mrevived ? 20 : 3)) {
            num = mndx_to_dragon_scales(monsndx(mdat));
            obj = mksobj_at(num, x, y, FALSE, FALSE);
            obj->spe = 0;
            obj->cursed = obj->blessed = FALSE;
        }
        goto default_1;
    case PM_TIAMAT:
        /* Make chromatic dragon scales */
        if (!rn2(3)) { /* Tiamat cannot be revived, no corpse */
            obj = mksobj_at(CHROMATIC_DRAGON_SCALES, x, y, FALSE, FALSE);
            obj->spe = 0;
            obj->cursed = obj->blessed = FALSE;
        }
        goto default_1;
    case PM_WHITE_UNICORN:
    case PM_GRAY_UNICORN:
    case PM_BLACK_UNICORN:
        if (mtmp->mrevived && rn2(2)) {
            if (canseemon(mtmp))
                pline_mon(mtmp,
                      "%s recently regrown horn crumbles to dust.",
                      s_suffix(Monnam(mtmp)));
        } else {
            obj = mksobj_at(UNICORN_HORN, x, y, TRUE, FALSE);
            if (obj && mtmp->mrevived)
                obj->degraded_horn = 1;
        }
        goto default_1;
    case PM_LONG_WORM:
        (void) mksobj_at(WORM_TOOTH, x, y, TRUE, FALSE);
        goto default_1;
    case PM_VAMPIRE:
    case PM_VAMPIRE_SOVEREIGN:
    case PM_VAMPIRE_NOBLE:
    case PM_VAMPIRE_ROYAL:
    case PM_VAMPIRE_MAGE:
        /* include mtmp in the mkcorpstat() call */
        num = undead_to_corpse(mndx);
        corpstatflags |= CORPSTAT_INIT;
        obj = mkcorpstat(CORPSE, mtmp, &mons[num], x, y, corpstatflags);
        obj->age -= 100; /* this is an *OLD* corpse */
        break;
    case PM_KOBOLD_ZOMBIE:
    case PM_DWARF_ZOMBIE:
    case PM_GNOME_ZOMBIE:
    case PM_ORC_ZOMBIE:
    case PM_ELF_ZOMBIE:
    case PM_DROW_ZOMBIE:
    case PM_HOBBIT_ZOMBIE:
    case PM_HUMAN_ZOMBIE:
    case PM_GIANT_ZOMBIE:
    case PM_ETTIN_ZOMBIE:
    case PM_GNOLL_WITHERLING:
    case PM_DRAUGR:
        corpstatflags |= CORPSTAT_ZOMBIE;
        FALLTHROUGH;
        /*FALLTHRU*/
    case PM_KOBOLD_MUMMY:
    case PM_DWARF_MUMMY:
    case PM_GNOME_MUMMY:
    case PM_ORC_MUMMY:
    case PM_ELF_MUMMY:
    case PM_DROW_MUMMY:
    case PM_HOBBIT_MUMMY:
    case PM_HUMAN_MUMMY:
    case PM_GIANT_MUMMY:
    case PM_ETTIN_MUMMY:
        num = undead_to_corpse(mndx);
        corpstatflags |= CORPSTAT_INIT;
        obj = mkcorpstat(CORPSE, mtmp, &mons[num], x, y, corpstatflags);
        obj->age -= 100; /* this is an *OLD* corpse */
        break;
    case PM_IRON_GOLEM:
        num = d(2, 6);
        while (num--)
            obj = mksobj_at(IRON_CHAIN, x, y, TRUE, FALSE);
        free_mgivenname(mtmp); /* don't christen obj */
        break;
    case PM_GLASS_GOLEM:
        num = d(2, 4); /* very low chance of creating all glass gems */
        while (num--)
            obj = mksobj_at(FIRST_GLASS_GEM + rn2(NUM_GLASS_GEMS),
                            x, y, TRUE, FALSE);
        free_mgivenname(mtmp);
        break;
    case PM_CLAY_GOLEM:
        obj = mksobj_at(ROCK, x, y, FALSE, FALSE);
        obj->quan = (long) (rn2(20) + 50);
        obj->owt = weight(obj);
        free_mgivenname(mtmp);
        break;
    case PM_STONE_GOLEM:
        corpstatflags &= ~CORPSTAT_INIT;
        obj = mkcorpstat(STATUE, (struct monst *) 0, mdat, x, y,
                         corpstatflags);
        break;
    case PM_WOOD_GOLEM:
        num = d(2, 4);
        while (num--) {
            obj = mksobj_at(
                            rn2(2) ? QUARTERSTAFF
                            : rn2(3) ? SMALL_SHIELD
                            : rn2(3) ? CLUB
                            : rn2(3) ? ELVEN_SPEAR : BOOMERANG,
                            x, y, TRUE, FALSE);
        }
        free_mgivenname(mtmp);
        break;
    case PM_ROPE_GOLEM:
        num = rn2(3);
        while (num-- > 0) {
            obj = mksobj_at(rn2(2) ? LEASH
                            : rn2(3) ? BULLWHIP : GRAPPLING_HOOK,
                            x, y, TRUE, FALSE);
        }
        free_mgivenname(mtmp);
        break;
    case PM_LEATHER_GOLEM:
        num = d(2, 4);
        while (num--)
            obj = mksobj_at(rn2(4) ? ARMOR
                            : rn2(3) ? CLOAK : SADDLE,
                            x, y, TRUE, FALSE);
        free_mgivenname(mtmp);
        break;
    case PM_GOLD_GOLEM:
        /* Good luck gives more coins */
        obj = mkgold((long) (200 - rnl(101)), x, y);
        free_mgivenname(mtmp);
        break;
    case PM_PAPER_GOLEM:
        num = rnd(4);
        while (num--)
            obj = mksobj_at(SCR_BLANK_PAPER, x, y, TRUE, FALSE);
        free_mgivenname(mtmp);
        break;
    /* only allow dead tree 'corpses' to form on certain
       level tile types; we don't want to blank out stairs
       or a ladder, etc */
    case PM_ENT:
    case PM_ELDER_ENT:
    case PM_TREE_BLIGHT:
    case PM_GULTHIAS_TREE:
        if (levl[x][y].typ == ROOM
            || levl[x][y].typ == GRASS
            || levl[x][y].typ == SAND) {
            levl[x][y].typ = DEADTREE;
            /* a dead tree blocks line of sight; master leaves the vision
               array stale here until the next full recalculation */
            recalc_block_point(x, y);
        }
        newsym(x, y);
        free_mgivenname(mtmp);
        break;
    /* expired puddings will congeal into a large blob;
       like dragons, relies on the order remaining consistent */
    case PM_GRAY_OOZE:
    case PM_BROWN_PUDDING:
    case PM_GREEN_SLIME:
    case PM_BLACK_PUDDING:
        /* we have to do this here because most other places
           expect there to be an object coming back; not this one */
        obj = mksobj_at(GLOB_OF_BLACK_PUDDING - (PM_BLACK_PUDDING - mndx),
                        x, y, TRUE, FALSE);

        while (obj && (otmp = obj_nexto(obj)) != (struct obj *) 0) {
            pudding_merge_message(obj, otmp);
            obj = obj_meld(&obj, &otmp);
        }
        free_mgivenname(mtmp);
        newsym(x, y);
        return obj;
    case NON_PM: case LEAVESTATUE: case NUMMONS: /* never use as index */
        break;

#if (NH_DEVEL_STATUS != NH_STATUS_RELEASED)
    case PM_GIANT_ANT: case PM_GIANT_MOSQUITO:
    case PM_KILLER_BEE: case PM_SOLDIER_ANT: case PM_FIRE_ANT:
    case PM_GIANT_COCKROACH: case PM_GIANT_BEETLE: case PM_QUEEN_BEE:
    case PM_LOCUST: case PM_QUEEN_ANT: case PM_ACID_BLOB:
    case PM_QUIVERING_BLOB: case PM_GELATINOUS_CUBE: case PM_CHICKATRICE:
    case PM_COCKATRICE: case PM_PYROLISK: case PM_BASILISK: case PM_JACKAL:
    case PM_FOX: case PM_COYOTE: case PM_WEREJACKAL: case PM_LITTLE_DOG:
    case PM_WOLF_CUB: case PM_RABID_DOG: case PM_DINGO: case PM_DOG:
    case PM_WOLF: case PM_WEREWOLF: case PM_WINTER_WOLF_CUB: case PM_LARGE_DOG:
    case PM_WARG: case PM_WINTER_WOLF: case PM_HELL_HOUND_PUP:
    case PM_HELL_HOUND: case PM_WEREDEMON: case PM_BOURBON: case PM_OZZY:
    case PM_CERBERUS: case PM_GAS_SPORE: case PM_FLOATING_EYE:
    case PM_FREEZING_SPHERE: case PM_FLAMING_SPHERE: case PM_SHOCKING_SPHERE:
    case PM_ACID_SPHERE: case PM_MAGICAL_EYE: case PM_BEHOLDER:
    case PM_TAL_GATH: case PM_KITTEN: case PM_HOUSECAT: case PM_JAGUAR:
    case PM_LYNX: case PM_PANTHER: case PM_LARGE_CAT: case PM_TIGER:
    case PM_SABER_TOOTHED_TIGER: case PM_DISPLACER_BEAST: case PM_GREMLIN:
    case PM_LAVA_GREMLIN: case PM_GARGOYLE: case PM_WINGED_GARGOYLE:
    case PM_DWARF: case PM_ILLITHID: case PM_HOBBIT: case PM_HOBBIT_PICKPOCKET:
    case PM_MOUNTAIN_DWARF: case PM_DWARF_NOBLE: case PM_GNOLL: case PM_GOLLUM:
    case PM_DWARF_ROYAL: case PM_GNOLL_HUNTER: case PM_GNOLL_CLERIC:
    case PM_MIND_FLAYER: case PM_FLIND: case PM_MASTER_MIND_FLAYER:
    case PM_ALHOON: case PM_MANES: case PM_LESSER_HOMUNCULUS:
    case PM_HOMUNCULUS: case PM_GREATER_HOMUNCULUS: case PM_IMP: case PM_LEMURE:
    case PM_QUASIT: case PM_TENGU: case PM_BLUE_JELLY: case PM_SPOTTED_JELLY:
    case PM_OCHRE_JELLY: case PM_KOBOLD: case PM_LARGE_KOBOLD:
    case PM_KOBOLD_NOBLE: case PM_KOBOLD_SHAMAN: case PM_LEPRECHAUN:
    case PM_SMALL_MIMIC: case PM_LARGE_MIMIC: case PM_GIANT_MIMIC:
    case PM_NYMPH: case PM_WOOD_NYMPH: case PM_WATER_NYMPH:
    case PM_MOUNTAIN_NYMPH: case PM_ICE_NYMPH: case PM_SATYR: case PM_APHRODITE:
    case PM_GOBLIN: case PM_HOBGOBLIN: case PM_GOBLIN_SHAMAN: case PM_ORC:
    case PM_HILL_ORC: case PM_MORDOR_ORC: case PM_BUGBEAR: case PM_URUK_HAI:
    case PM_ORC_SHAMAN: case PM_GOBLIN_OUTRIDER: case PM_ORC_CAPTAIN:
    case PM_GOBLIN_CAPTAIN: case PM_GOBLIN_KING: case PM_ROCK_PIERCER:
    case PM_IRON_PIERCER: case PM_GLASS_PIERCER: case PM_ROTHE:
    case PM_LEOCROTTA: case PM_WUMPUS: case PM_GRIZZLY_BEAR: case PM_MUMAK:
    case PM_CAVE_BEAR: case PM_TITANOTHERE: case PM_BALUCHITHERIUM:
    case PM_MASTODON: case PM_WOOLLY_MAMMOTH: case PM_SEWER_RAT:
    case PM_GIANT_RAT: case PM_RABID_RAT: case PM_WERERAT: case PM_ROCK_MOLE:
    case PM_WOODCHUCK: case PM_ENORMOUS_RAT: case PM_RODENT_OF_UNUSUAL_SIZE:
    case PM_HONEY_BADGER: case PM_WOLVERINE: case PM_DIRE_WOLVERINE:
    case PM_CAVE_SPIDER: case PM_LARGE_SPIDER: case PM_CENTIPEDE:
    case PM_JUMPING_SPIDER: case PM_GIANT_SPIDER: case PM_SCORPION:
    case PM_GARGANTUAN_SPIDER: case PM_DRIDER: case PM_GIANT_CENTIPEDE:
    case PM_LURKER_ABOVE: case PM_TRAPPER: case PM_PONY:
    case PM_LESSER_NIGHTMARE: case PM_DEER: case PM_HORSE: case PM_NIGHTMARE:
    case PM_STAG: case PM_WARHORSE: case PM_CAUCHEMAR: case PM_PEGASUS:
    case PM_GREATER_PEGASUS: case PM_PALE_HORSE: case PM_WHITE_HORSE:
    case PM_BLACK_HORSE: case PM_RED_HORSE: case PM_FOG_CLOUD:
    case PM_DUST_VORTEX: case PM_ICE_VORTEX: case PM_ENERGY_VORTEX:
    case PM_STEAM_VORTEX: case PM_FIRE_VORTEX: case PM_ANTIMATTER_VORTEX:
    case PM_BABY_LONG_WORM: case PM_GIANT_LEECH: case PM_BABY_PURPLE_WORM:
    case PM_PURPLE_WORM: case PM_NEOTHELID: case PM_GRID_BUG: case PM_XAN:
    case PM_YELLOW_LIGHT: case PM_BLACK_LIGHT: case PM_SMALL_SKELETAL_HOUND:
    case PM_SKELETAL_PONY: case PM_SKELETAL_HOUND: case PM_SKELETON:
    case PM_SKELETAL_HORSE: case PM_LARGE_SKELETAL_HOUND:
    case PM_SKELETAL_WARHORSE: case PM_SKELETON_WARRIOR:
    case PM_SKELETAL_DRAGON: case PM_COUATL: case PM_ALEAX: case PM_ANGEL:
    case PM_KI_RIN: case PM_ELDRITCH_KI_RIN: case PM_ARCHANGEL: case PM_ARCHON:
    case PM_SAINT_MICHAEL: case PM_BAT: case PM_GIANT_BAT: case PM_HAWK:
    case PM_LARGE_HAWK: case PM_RAVEN: case PM_VAMPIRE_BAT: case PM_GIANT_HAWK:
    case PM_CENTAUR: case PM_PLAINS_CENTAUR: case PM_FOREST_CENTAUR:
    case PM_MOUNTAIN_CENTAUR: case PM_PSEUDODRAGON: case PM_ELDER_PSEUDODRAGON:
    case PM_ANCIENT_PSEUDODRAGON: case PM_FELL_BEAST: case PM_BABY_GRAY_DRAGON:
    case PM_BABY_SILVER_DRAGON: case PM_BABY_SHIMMERING_DRAGON:
    case PM_BABY_RED_DRAGON: case PM_BABY_WHITE_DRAGON:
    case PM_BABY_ORANGE_DRAGON: case PM_BABY_BLACK_DRAGON:
    case PM_BABY_BLUE_DRAGON: case PM_BABY_GREEN_DRAGON:
    case PM_BABY_GOLD_DRAGON: case PM_BABY_SEA_DRAGON:
    case PM_BABY_YELLOW_DRAGON: case PM_BABY_SHADOW_DRAGON:
    case PM_BABY_CELESTIAL_DRAGON:

    case PM_CHROMATIC_DRAGON: case PM_STALKER: case PM_AIR_ELEMENTAL:
    case PM_FIRE_ELEMENTAL: case PM_EARTH_ELEMENTAL: case PM_WATER_ELEMENTAL:
    case PM_LICHEN: case PM_BROWN_MOLD: case PM_YELLOW_MOLD: case PM_GREEN_MOLD:
    case PM_RED_MOLD: case PM_SHRIEKER: case PM_VIOLET_FUNGUS:
    case PM_GRAY_FUNGUS: case PM_BLACK_FUNGUS: case PM_GNOME:
    case PM_ROCK_GNOME: case PM_GNOME_NOBLE: case PM_GNOMISH_WIZARD:
    case PM_GNOME_ROYAL: case PM_GIANT: case PM_STONE_GIANT:
    case PM_HILL_GIANT_SHAMAN: case PM_HILL_GIANT: case PM_FIRE_GIANT:
    case PM_FROST_GIANT: case PM_ETTIN: case PM_STORM_GIANT: case PM_TITAN:
    case PM_MINOTAUR: case PM_ELDER_MINOTAUR: case PM_SNARK: case PM_JABBERWOCK:
    case PM_BOOJUM: case PM_VORPAL_JABBERWOCK: case PM_KEYSTONE_KOP:
    case PM_KOP_SERGEANT: case PM_KOP_LIEUTENANT: case PM_KOP_KAPTAIN:
    case PM_LICH: case PM_DEMILICH: case PM_MASTER_LICH: case PM_ARCH_LICH:
    case PM_VECNA:
    case PM_RED_NAGA_HATCHLING: case PM_BLACK_NAGA_HATCHLING:
    case PM_GOLDEN_NAGA_HATCHLING: case PM_GUARDIAN_NAGA_HATCHLING:
    case PM_RED_NAGA: case PM_BLACK_NAGA: case PM_GOLDEN_NAGA:
    case PM_GUARDIAN_NAGA: case PM_OGRE: case PM_OGRE_NOBLE: case PM_OGRE_ROYAL:
    case PM_QUANTUM_MECHANIC: case PM_GENETIC_ENGINEER: case PM_RUST_MONSTER:
    case PM_DISENCHANTER: case PM_GARTER_SNAKE: case PM_SNAKE:
    case PM_WATER_MOCCASIN: case PM_PYTHON: case PM_PIT_VIPER: case PM_COBRA:
    case PM_GIANT_ANACONDA: case PM_TROLL: case PM_ICE_TROLL:
    case PM_ROCK_TROLL: case PM_WATER_TROLL: case PM_OLOG_HAI:
    case PM_UMBER_HULK: case PM_SHAMBLING_HORROR: case PM_KAS:
    case PM_VLAD_THE_IMPALER: case PM_BARROW_WIGHT: case PM_WRAITH:
    case PM_NAZGUL: case PM_XORN: case PM_MONKEY: case PM_BABY_OWLBEAR:
    case PM_APE: case PM_OWLBEAR: case PM_YETI: case PM_CARNIVOROUS_APE:
    case PM_SASQUATCH: case PM_ZRUTY: case PM_ABOMINABLE_SNOWMAN:
    case PM_GHOUL: case PM_REVENANT: case PM_STRAW_GOLEM:
    case PM_FLESH_GOLEM: case PM_SNOW_GOLEM: case PM_HUMAN: case PM_AASIMAR:
    case PM_MINER: case PM_PRISON_GUARD: case PM_HUMAN_WERERAT:
    case PM_HUMAN_WEREJACKAL: case PM_HUMAN_WEREWOLF: case PM_CHAMPION:
    case PM_AGENT: case PM_DROW: case PM_ELF: case PM_WOODLAND_ELF:
    case PM_GREEN_ELF: case PM_DROW_RAIDER: case PM_GREY_ELF:
    case PM_ELVEN_NOBLE: case PM_DROW_WARRIOR: case PM_ELVEN_WIZARD:
    case PM_ELVEN_ROYAL: case PM_DOPPELGANGER: case PM_DROW_MAGE:
    case PM_DROW_CLERIC: case PM_SHOPKEEPER: case PM_GUARD: case PM_PRISONER:
    case PM_ORACLE: case PM_ALIGNED_PRIEST: case PM_HIGH_PRIEST:
    case PM_SOLDIER: case PM_SERGEANT: case PM_NURSE: case PM_LIEUTENANT:
    case PM_CAPTAIN: case PM_WATCHMAN: case PM_WATCH_CAPTAIN: case PM_MEDUSA:
    case PM_WIZARD_OF_YENDOR: case PM_CROESUS: case PM_CHARON:
    case PM_RAT_KING: case PM_KATHRYN_THE_ICE_QUEEN:
    case PM_KATHRYN_THE_ENCHANTRESS: case PM_GHOST: case PM_SPECTRE:
    case PM_SHADE: case PM_WATER_DEMON: case PM_LAVA_DEMON:
    case PM_AMOROUS_DEMON: case PM_HORNED_DEVIL: case PM_ERINYS:
    case PM_BARBED_DEVIL: case PM_MARILITH: case PM_VROCK: case PM_HEZROU:
    case PM_BONE_DEVIL: case PM_ICE_DEVIL: case PM_NALFESHNEE:
    case PM_PIT_FIEND: case PM_SANDESTIN: case PM_DEMON_WEREDEMON:
    case PM_BALROG: case PM_JUIBLEX: case PM_BAPHOMET: case PM_YEENOGHU:
    case PM_LOLTH: case PM_GERYON: case PM_DISPATER: case PM_BAALZEBUB:
    case PM_MEPHISTOPHELES: case PM_ORCUS: case PM_GRAZ_ZT:
    case PM_ASMODEUS: case PM_DEMOGORGON: case PM_LUCIFER: case PM_DEATH:
    case PM_PESTILENCE: case PM_FAMINE: case PM_MAIL_DAEMON: case PM_DJINNI:
    case PM_DEMON: case PM_JELLYFISH: case PM_PIRANHA:
    case PM_MIND_FLAYER_LARVA: case PM_SHARK: case PM_GIANT_EEL:
    case PM_ELECTRIC_EEL: case PM_KRAKEN: case PM_TORTLE: case PM_NEWT:
    case PM_GECKO: case PM_IGUANA: case PM_SEA_TORTLE: case PM_BABY_CROCODILE:
    case PM_LIZARD: case PM_TORTLE_SHAMAN: case PM_CHAMELEON: case PM_CROCODILE:
    case PM_GIANT_TURTLE: case PM_CAVE_LIZARD: case PM_SALAMANDER:
    case PM_FROST_SALAMANDER: case PM_LARGE_CAVE_LIZARD:
    case PM_GIANT_CROCODILE: case PM_PETRIFIED_ENT:
    case PM_CREEPING_MOUND: case PM_TWIG_BLIGHT: case PM_NEEDLE_BLIGHT:
    case PM_ASSASSIN_VINE: case PM_ORB_OF_FROST: case PM_ORB_OF_FIRE:
    case PM_ARCHEOLOGIST: case PM_BARBARIAN: case PM_CAVE_DWELLER:
    case PM_CONVICT: case PM_DRUID: case PM_HEALER: case PM_INFIDEL:
    case PM_KNIGHT: case PM_MONK: case PM_CLERIC: case PM_RANGER: case PM_ROGUE:
    case PM_SAMURAI: case PM_TOURIST: case PM_VALKYRIE: case PM_WIZARD:
    case PM_LONG_WORM_TAIL: case PM_LORD_CARNARVON: case PM_PELIAS:
    case PM_SHAMAN_KARNOV: case PM_ROBERT_THE_LIFER: case PM_ELANEE:
    case PM_HIPPOCRATES: case PM_ARCHBISHOP_OF_MOLOCH: case PM_KING_ARTHUR:
    case PM_MORGAN_LE_FAY: case PM_MASTER_PO: case PM_ARCH_PRIEST:
    case PM_ORION: case PM_MASTER_OF_THIEVES: case PM_LORD_SATO:
    case PM_TWOFLOWER: case PM_NORN: case PM_NEFERET_THE_GREEN:
    case PM_MINION_OF_HUHETOTL: case PM_THOTH_AMON: case PM_ANNAM:
    case PM_WARDEN_ARIANNA: case PM_BABA_YAGA: case PM_CYCLOPS: case PM_PALADIN:
    case PM_IXOTH: case PM_MERLIN: case PM_MASTER_KAEN: case PM_NALZOK:
    case PM_SCORPIUS: case PM_MASTER_ASSASSIN: case PM_ASHIKAGA_TAKAUJI:
    case PM_LORD_SURTUR: case PM_DARK_ONE: case PM_STUDENT: case PM_CHIEFTAIN:
    case PM_NEANDERTHAL: case PM_INMATE: case PM_ASPIRANT: case PM_ATTENDANT:
    case PM_TEMPLAR: case PM_CULTIST: case PM_PAGE: case PM_ABBOT:
    case PM_ACOLYTE: case PM_HUNTER: case PM_THUG: case PM_NINJA: case PM_RONIN:
    case PM_ROSHI: case PM_GUIDE: case PM_WARRIOR: case PM_APPRENTICE:
    case PM_TEST_MONSTER:

    default:
#endif
 default_1:
        if (svm.mvitals[mndx].mvflags & G_NOCORPSE) {
            return (struct obj *) 0;
        } else {
            corpstatflags |= CORPSTAT_INIT;
            /* zombies by racial overlay leave a revivable corpse too */
            if (racial_zombie(mtmp))
                corpstatflags |= CORPSTAT_ZOMBIE;
            /* preserve the unique traits of some creatures */
            obj = mkcorpstat(CORPSE, KEEPTRAITS(mtmp) ? mtmp : 0,
                             mdat, x, y, corpstatflags);
            if (racial_zombie(mtmp) || racial_vampire(mtmp))
                obj->age -= 100;
            if (burythem) {
                boolean dealloc;

                (void) bury_an_obj(obj, &dealloc);
                newsym(x, y);
                return dealloc ? (struct obj *) 0 : obj;
            }
        }
        break;
    }
    /* All special cases should precede the G_NOCORPSE check */

    if (!obj)
        return (struct obj *) 0;

    /* if polymorph or undead turning has killed this monster,
       prevent the same attack beam from hitting its corpse */
    if (svc.context.bypasses)
        bypass_obj(obj);

    if (has_mgivenname(mtmp))
        obj = oname(obj, MGIVENNAME(mtmp), ONAME_NO_FLAGS);

    /*  Avoid "It was hidden under a green mold corpse!"
     *  during Blind combat. An unseen monster referred to as "it"
     *  could be killed and leave a corpse.  If a hider then hid
     *  underneath it, you could be told the corpse type of a
     *  monster that you never knew was there without this.
     *  The code in hitmu() substitutes the word "something"
     *  if the corpse's obj->dknown is 0.
     */
    if (Blind && !sensemon(mtmp))
        clear_dknown(obj); /* obj->dknown = 0; */

    stackobj(obj); /* 'obj' remains valid if stacking happens */
    newsym(x, y);
    /* in case the corpse was placed at a different spot from where
       the monster was (not expected to happen) */
    if (obj->ox != x || obj->oy != y)
        newsym(obj->ox, obj->oy);
    return obj;
}

#undef KEEPTRAITS

/* check mtmp and water/lava for compatibility, 0 (survived), 1 (died) */
int
minliquid(struct monst *mtmp)
{
    int res;

    /* set up flag for mondead() and xkilled() */
    iflags.sad_feeling = (mtmp->mtame && !canseemon(mtmp));
    res = minliquid_core(mtmp);
    /* always clear the flag */
    iflags.sad_feeling = FALSE;
    return res;
}

/* guts of minliquid() */
staticfn int
minliquid_core(struct monst *mtmp)
{
    boolean inpool, inlava, infountain, inforge;
    boolean waterwall = is_waterwall(mtmp->mx,mtmp->my);

    /* [ceiling clingers are handled below] */
    inpool = (is_pool(mtmp->mx, mtmp->my)
              && (!(is_flyer(mtmp->data) || is_floater(mtmp->data)
                    || can_levitate(mtmp) || can_fly(mtmp))
                  /* there's no "above the surface" on the plane of water */
                  || Is_waterlevel(&u.uz)));
    inlava = (is_lava(mtmp->mx, mtmp->my)
              && !(is_flyer(mtmp->data) || is_floater(mtmp->data)
                   || can_levitate(mtmp) || can_fly(mtmp)));
    infountain = IS_FOUNTAIN(levl[mtmp->mx][mtmp->my].typ);
    inforge = IS_FORGE(levl[mtmp->mx][mtmp->my].typ);

    /* Flying and levitation keeps our steed out of the liquid
       (but not water-walking or swimming; note: if hero is in a
       water location on the Plane of Water, flight and levitating
       are blocked so this (Flying || Levitation) test fails there
       and steed will be subject to water effects, as intended) */
    if (mtmp == u.usteed && (Flying || Levitation) && !waterwall)
        return 0;

    /* Gremlin multiplying won't go on forever since the hit points
     * keep going down, and when it gets to 1 hit point the clone
     * function will fail.
     */
    if (mtmp->data == &mons[PM_GREMLIN] && (inpool || infountain) && rn2(3)) {
        if (split_mon(mtmp, (struct monst *) 0))
            dryup(mtmp->mx, mtmp->my, FALSE);
        if (inpool)
            water_damage_chain(mtmp->minvent, FALSE, 0, TRUE);
        return 0;
    } else if (mtmp->data == &mons[PM_LAVA_GREMLIN]
               && (inlava || inforge) && rn2(3)) {
        (void) split_mon(mtmp, (struct monst *) 0);
        /* no lava damage chain func (yet), so minvent is spared */
        if (inforge && !rn2(3))
            blowupforge(mtmp->mx, mtmp->my);
        return 0;
    } else if (mtmp->data == &mons[PM_IRON_GOLEM] && inpool && !rn2(5)) {
        int dam = d(2, 6);

        if (cansee(mtmp->mx, mtmp->my))
            pline_mon(mtmp, "%s rusts.", Monnam(mtmp));
        mtmp->mhp -= dam;
        if (mtmp->mhpmax > dam)
            mtmp->mhpmax -= dam;
        if (DEADMONSTER(mtmp)) {
            mondied(mtmp);
            if (DEADMONSTER(mtmp))
                return 1;
        }
        water_damage_chain(mtmp->minvent, FALSE, 0, TRUE);
        return 0;
    }

    if (inlava) {
        /*
         * Lava effects much as water effects. Lava likers are able to
         * protect their stuff. Fire resistant monsters can only protect
         * themselves  --ALI
         */
        if (!is_clinger(mtmp->data) && !likes_lava(mtmp->data)) {
            /* not fair...?  hero doesn't automatically teleport away
               from lava, just from water */
            if (can_teleport(mtmp->data) && !tele_restrict(mtmp)) {
                if (rloc(mtmp, RLOC_MSG))
                    return 0;
            }
            if (!resists_fire(mtmp)) {
                if (cansee(mtmp->mx, mtmp->my)) {
                    struct attack *dummy = &mtmp->data->mattk[0];
                    const char *how = on_fire(mtmp->data, dummy);

                    pline_mon(mtmp, "%s %s.", Monnam(mtmp),
                          !strcmp(how, "boiling") ? "boils away"
                             : !strcmp(how, "melting") ? "melts away"
                                : "burns to a crisp");
                }
                /* unlike fire -> melt ice -> pool, there's no way for the
                   hero to create lava beneath a monster, so the !mon_moving
                   case is not expected to happen (and we haven't made a
                   player-against-monster variation of the message above) */
                if (svc.context.mon_moving)
                    mondead(mtmp); /* no corpse */
                else
                    xkilled(mtmp, XKILL_NOMSG);
            } else {
                mtmp->mhp -= 1;
                if (DEADMONSTER(mtmp)) {
                    if (cansee(mtmp->mx, mtmp->my))
                        pline_mon(mtmp, "%s surrenders to the fire.",
                                  Monnam(mtmp));
                    mondead(mtmp); /* no corpse */
                } else if (cansee(mtmp->mx, mtmp->my)) {
                    pline_mon(mtmp, "%s burns slightly.", Monnam(mtmp));
                }
            }
            if (!DEADMONSTER(mtmp)) {
                if (m_in_air(mtmp)) {
                    ; /* vampshifter in wolf form can revert to vampire lord
                       * and become a flyer so not need to teleport */
                } else if (likes_lava(mtmp->data)) {
                    ; /* likes_lava case is hypothetical */
                } else {
                    (void) fire_damage_chain(mtmp->minvent, FALSE, FALSE,
                                             mtmp->mx, mtmp->my);
                    if (!rloc(mtmp, RLOC_MSG))
                        deal_with_overcrowding(mtmp);
                }
                return 0;
            }
            return 1;
        }
    } else if (inpool || waterwall) {
        /* Most monsters drown in pools.  flooreffects() will take care of
         * water damage to dead monsters' inventory, but survivors need to
         * be handled here.  Swimmers are able to protect their stuff...
         */
        if ((waterwall || !is_clinger(mtmp->data))
            && !cant_drown(mtmp->data)) {
            /* like hero with teleport intrinsic or spell, teleport away
               if possible */
            if (can_teleport(mtmp->data) && !tele_restrict(mtmp)) {
                if (rloc(mtmp, RLOC_MSG))
                    return 0;
            }
            if (cansee(mtmp->mx, mtmp->my)) {
                if (svc.context.mon_moving)
                    pline_mon(mtmp, "%s drowns.", Monnam(mtmp));
                else
                    /* hero used fire to melt ice that monster was on */
                    You("drown %s.", mon_nam(mtmp));
            }
            if (engulfing_u(mtmp)) {
                /* This can happen after a purple worm plucks you off a
                   flying steed while you are over water. */
                pline("%s sinks as %s rushes in and flushes you out.",
                      Monnam(mtmp), hliquid("water"));
            }
            if (svc.context.mon_moving)
                mondied(mtmp); /* ok to leave corpse despite water */
            else
                xkilled(mtmp, XKILL_NOMSG);
            if (!DEADMONSTER(mtmp)) {
                if (m_in_air(mtmp)) {
                    ; /* vampshifter in wolf form can revert to vampire lord
                       * and become a flyer so not need to teleport */
                } else {
                    water_damage_chain(mtmp->minvent, FALSE, 0, TRUE);
                    if (!rloc(mtmp, RLOC_NOMSG))
                        deal_with_overcrowding(mtmp);
                }
                return 0;
            }
            return 1;
        }
    } else {
        /* but eels have a difficult time outside */
        if (mtmp->data->mlet == S_EEL && !Is_waterlevel(&u.uz)
            && !breathless(mtmp->data)) {
            /* as mhp gets lower, the rate of further loss slows down */
            if (mtmp->mhp > 1 && rn2(mtmp->mhp) > rn2(8))
                mtmp->mhp--;
            monflee(mtmp, 2, FALSE, FALSE);
        }
    }
    return 0;
}

/* calculate 'mon's movement for current turn; called from moveloop() */
int
mcalcmove(
    struct monst *mon,
    boolean m_moving) /* True: adjust for moving;
                       * False: just adjust for speed */
{
    int mmove = mon->data->mmove;
    int mmove_adj;

    /* Note: MSLOW's `+ 1' prevents slowed speed 1 getting reduced to 0;
     *       MFAST's `+ 2' prevents hasted speed 1 from becoming a no-op;
     *       both adjustments have negligible effect on higher speeds.
     */
    if (mon->mspeed == MSLOW) {
        /* slow-monster effects work better against faster monsters: they
           lose 1/3 of their speed below 12 but 2/3 of their speed above */
        if (mmove < NORMAL_SPEED)
            mmove = (2 * mmove + 1) / 3;
        else
            mmove = 4 + (mmove / 3);
    } else if (mon->mspeed == MFAST)
        mmove = (4 * mmove + 2) / 3;

    if (mon == u.usteed && u.ugallop && svc.context.mv) {
        /* increase movement by a factor of 1.5; also increase variance of
           movement speed (if it's naturally 24, we don't want it to always
           become 36) */
        mmove = ((rn2(2) ? 4 : 5) * mmove) / 3;
    }

    if (m_moving) {
        /* Randomly round the monster's speed to a multiple of NORMAL_SPEED.
           This makes it impossible for the player to predict when they'll
           get a free turn (thus preventing exploits like "melee kiting"),
           while retaining guarantees about shopkeepers not being outsped
           by a normal-speed player, normal-speed players being unable
           to open up a gap when fleeing a normal-speed monster, etc. */
        mmove_adj = mmove % NORMAL_SPEED;
        mmove -= mmove_adj;
        if (rn2(NORMAL_SPEED) < mmove_adj)
            mmove += NORMAL_SPEED;
    }
    return mmove;
}

/* actions that happen once per ``turn'', regardless of each
   individual monster's metabolism; some of these might need to
   be reclassified to occur more in proportion with movement rate */
void
mcalcdistress(void)
{
    iter_mons(m_calcdistress);
}

staticfn void
m_calcdistress(struct monst *mtmp)
{
    /* must check non-moving monsters once/turn in case they managed
       to end up in water or lava; note: when not in liquid they regen,
       shape-shift, timeout temporary maladies just like other monsters */
    if (mtmp->data->mmove == 0) {
        if (gv.vision_full_recalc)
            vision_recalc(0);
        if (minliquid(mtmp))
            return;
    }

    /* regenerate hit points */
    mon_regen(mtmp, FALSE);

    /* sick monsters can die from their illness */
    if (mtmp->msick && mtmp->msicktime <= 1) {
        if (resists_sick(mtmp) || defended(mtmp, AD_DISE)) {
            mtmp->msick = 0;
        } else {
            if (canseemon(mtmp))
                pline_mon(mtmp, "%s dies from %s illness.",
                          Monnam(mtmp), noit_mhis(mtmp));
            if ((mtmp->msick & 2) && !nonliving(mtmp->data)
                && can_become_zombie(r_data(mtmp))) {
                gz.zombify = (zombie_form(r_data(mtmp)) != NON_PM);
                mtmp->msick = 0;
                mtmp->mhp = -1;
                if (mtmp->msickbyu)
                    xkilled(mtmp, XKILL_NOMSG);
                else
                    mondied(mtmp);
                gz.zombify = FALSE; /* reset */
            } else {
                mtmp->msick = 0;
                mtmp->mhp = -1;
                if (mtmp->msickbyu)
                    xkilled(mtmp, XKILL_NOMSG);
                else
                    mondied(mtmp);
            }
            return;
        }
    }

    /* diseased monsters can die as well */
    if (mtmp->mdiseased && mtmp->mdiseasetime <= 1) {
        if (resists_sick(mtmp) || defended(mtmp, AD_DISE)) {
            mtmp->mdiseased = 0;
        } else {
            if (canseemon(mtmp))
                pline_mon(mtmp, "%s dies from %s infection.",
                          Monnam(mtmp), noit_mhis(mtmp));
            mtmp->mdiseased = 0;
            mtmp->mhp = -1;
            if (mtmp->mdiseabyu)
                xkilled(mtmp, XKILL_GIVEMSG);
            else
                mondied(mtmp);
        }
        return;
    }

    /* possibly polymorph shapechangers and lycanthropes */
    if (ismnum(mtmp->cham))
        decide_to_shapeshift(mtmp);
    were_change(mtmp);

    /* gradually time out temporary problems */
    if (mtmp->mblinded && !--mtmp->mblinded)
        mtmp->mcansee = 1;
    if (mtmp->mfrozen && !--mtmp->mfrozen)
        mtmp->mcanmove = 1;
    if (mtmp->mfleetim && !--mtmp->mfleetim)
        mtmp->mflee = 0;

    /* spell-conferred buffs wear off (timers tick in mon_regen) */
    if (has_reflection(mtmp) && mtmp->mreflecttime <= 1) {
        if (canseemon(mtmp))
            pline("%s shimmering globe disappears.",
                  s_suffix(Monnam(mtmp)));
        mtmp->mextrinsics &= ~(MR2_REFLECTION);
    }
    if (has_barkskin(mtmp) && mtmp->mbarkskintime <= 1) {
        if (canseemon(mtmp))
            pline("%s %s returns to normal.",
                  s_suffix(Monnam(mtmp)), mbodypart(mtmp, SKIN));
        mtmp->mextrinsics &= ~(MR2_BARKSKIN);
    }
    if (has_stoneskin(mtmp) && mtmp->mstoneskintime <= 1) {
        if (canseemon(mtmp))
            pline("%s %s returns to normal.",
                  s_suffix(Monnam(mtmp)), mbodypart(mtmp, SKIN));
        /* stone resistance also goes away */
        mtmp->mextrinsics &= ~(MR2_STONESKIN);
    }
    if (mtmp->mentangled && mtmp->mentangletime < 1) {
        mtmp->mentangled = 0;
        if (canseemon(mtmp))
            pline("%s is no longer entangled.", Monnam(mtmp));
    }
    if (can_levitate(mtmp) && mtmp->mlevitatetime
        && mtmp->mlevitatetime <= 1) {
        mtmp->mextrinsics &= ~(MR2_LEVITATE);
        mtmp->mlevitatetime = 0;
        if (canseemon(mtmp))
            pline("%s floats gently to the %s.",
                  Monnam(mtmp), surface(mtmp->mx, mtmp->my));
    }
    if (can_jump(mtmp) && mtmp->mjumptime && mtmp->mjumptime <= 1) {
        mtmp->mextrinsics &= ~(MR2_JUMPING);
        mtmp->mjumptime = 0;
        /* no message - jumping expiration is silent */
    }

    /* FIXME: mtmp->mlstmv ought to be updated here */
}

/* perform movement for a single monster.
   meant to be used with iter_mons_safe. */
boolean
movemon_singlemon(struct monst *mtmp)
{
    /* end monster movement early if hero is flagged to leave the level */
    if (u.utotype
#ifdef SAFERHANGUP
        /* or if the program has lost contact with the user */
        || program_state.done_hup
#endif
        ) {
        gs.somebody_can_move = FALSE;
        return TRUE;
    }

    /* one dead monster needs to perform a move after death: vault
       guard whose temporary corridor is still on the map; live
       guards who have led the hero back to civilization get moved
       off the map too; gd_move() decides whether the temporary
       corridor can be removed and guard discarded (via clearing
       mon->isgd flag so that dmonsfree() will get rid of mon) */
    if (PARKEDMONSTER(mtmp) && !(mtmp->mstate & MON_MIGRATING)) {
        /* parked at <0,0>; eventually isgd should get set to false */
        if (svm.moves > mtmp->mlstmv) {
            (void) gd_move(mtmp);
            mtmp->mlstmv = svm.moves;
        }
        mtmp->movement = 0;
        return FALSE;
    }
    if (DEADMONSTER(mtmp)) {
        mtmp->movement = 0;
        return FALSE;
    }

    /* monster isn't on this map anymore; a steed carrying a monster rider
       is off the map but stays on the fmon list at its rider's square and
       is still able to attack from there */
    if (mon_offmap(mtmp) && !mtmp->ridden_by) {
        mtmp->movement = 0;
        return FALSE;
    }

    m_everyturn_effect(mtmp);

    /* Find a monster that we have not treated yet. */
    if (mtmp->movement < NORMAL_SPEED)
        return FALSE;

    mtmp->movement -= NORMAL_SPEED;
    if (mtmp->movement >= NORMAL_SPEED)
        gs.somebody_can_move = TRUE;

    if (gv.vision_full_recalc)
        vision_recalc(0); /* vision! */

    /* reset obj bypasses before next monster moves */
    if (svc.context.bypasses)
        clear_bypasses();
    clear_splitobjs();
    if (minliquid(mtmp))
        return FALSE;

    /* after losing equipment, try to put on replacement;
       skip monsters parked off the map since m_dowear_type() may call
       begin_burn() which needs get_obj_location() to succeed; also
       skip a monster concentrating on reading a spellbook so it
       does not swap armor or weapons mid-read -- I_SPECIAL stays
       set and the change is deferred until the read finishes or is
       interrupted (the emsp read state is maintained by the monster
       spellbook subsystem) */
    if ((mtmp->misc_worn_check & I_SPECIAL) && mtmp->mx
        && !(has_emsp(mtmp) && EMSP(mtmp)->msp_reading != 0)) {
        long oldworn;

        /* hostiles only try to equip things if they think hero isn't
         * nearby; if they think hero is nearby, leave the flag intact so
         * that it can be checked again on subsequent moves until the hero
         * is perceived to be farther away. */
        if (mtmp->mpeaceful || mtmp->mtame
            || dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) > (3 * 3)) {
            mtmp->misc_worn_check &= ~I_SPECIAL;
            oldworn = mtmp->misc_worn_check;
            m_dowear(mtmp, FALSE);
            if (mtmp->misc_worn_check != oldworn || !mtmp->mcanmove)
                return FALSE; /* is spending this turn equipping */

            /* also reassess hand-to-hand weapon: a monster that
               acquired a better (e.g. artifact) weapon outside of
               combat would otherwise keep its old weapon until it
               next fights. Mirror the combat re-eval, but skip a
               welded weapon (can't switch) or a launcher being held
               for ranged attacks (don't disturb a ranged stance;
               combat re-derives the melee weapon when it engages) */
            if (attacktype(mtmp->data, AT_WEAP)) {
                struct obj *mw_tmp = MON_WEP(mtmp);
                struct obj *bestwep;

                if (!(mw_tmp && mwelded(mw_tmp)
                      && mtmp->data != &mons[PM_INFIDEL])
                    && !(mw_tmp && is_launcher(mw_tmp))) {
                    bestwep = select_hwep(mtmp);
                    if (bestwep && bestwep != mw_tmp
                        && (!mw_tmp || bestwep->otyp != mw_tmp->otyp)) {
                        mtmp->weapon_check = NEED_HTH_WEAPON;
                        if (mon_wield_item(mtmp) != 0)
                            return FALSE; /* spent the turn switching */
                    }
                }
            }
        }
    }

    if (is_hider(mtmp->data)) {
        /* unwatched mimics and piercers may hide again  [MRS] */
        if (restrap(mtmp))
            return FALSE;
        if (M_AP_TYPE(mtmp) == M_AP_FURNITURE
            || M_AP_TYPE(mtmp) == M_AP_OBJECT)
            return FALSE;
        if (mtmp->mundetected)
            return FALSE;
    } else if (mtmp->data->mlet == S_EEL && !mtmp->mundetected
               && (mtmp->mflee || !m_next2u(mtmp))
               && !canseemon(mtmp) && !rn2(4)) {
        /* some eels end up stuck in isolated pools, where they
           can't--or at least won't--move, so they never reach
           their post-move chance to re-hide */
        if (hideunder(mtmp))
            return FALSE;
    }

    /* a monster might climb onto a nearby saddled monster */
    if ((is_mplayer(mtmp->data) && !rn2(16)) || !rn2(300)) {
        if (mount_up(mtmp))
            return FALSE;
    }

    /* continue if the monster died fighting */
    if (Conflict && !mtmp->iswiz && m_canseeu(mtmp)) {
        /* Note:
         *  Conflict does not take effect in the first round.
         *  Therefore, A monster when stepping into the area will
         *  get to swing at you.
         *
         *  The call to fightm() must be _last_.  The monster might
         *  have died if it returns 1.
         */
        if (cansee(mtmp->mx, mtmp->my)
            && (mdistu(mtmp) <= BOLT_LIM * BOLT_LIM)
            && fightm(mtmp))
            return FALSE; /* mon might have died */
    }
    (void) dochugw(mtmp, TRUE); /* otherwise just move the monster */
    return FALSE;
}

/* counts movemon() invocations (multiple per hero turn; see
   allmain.c do/while); recorded in recent_detaches[].mm_seq so
   same-turn detaches across separate movemon() passes are
   distinguishable */
static long movemon_callctr = 0;

/* perform movement for all monsters */
int
movemon(void)
{
    gs.somebody_can_move = FALSE;

    movemon_callctr++;

    iter_mons_safe(movemon_singlemon);

    if (any_light_source())
        gv.vision_full_recalc = 1; /* in case a mon moved w/ a light source */
    /* reset obj bypasses after last monster has moved */
    if (svc.context.bypasses)
        clear_bypasses();
    clear_splitobjs();
    /* remove dead monsters; dead vault guard will be left at <0,0>
       if temporary corridor out of vault hasn't been removed yet */
    dmonsfree();

    /* a monster may have levteleported player -dlc */
    if (u.utotype) {
        deferred_goto();
        /* changed levels, so these monsters are dormant */
        gs.somebody_can_move = FALSE;
    }

    return gs.somebody_can_move;
}

/* dispose of contents of an eaten container; used for pets and other mons */
void
meatbox(struct monst *mon, struct obj *otmp)
{
    boolean engulf_contents = (mon->data == &mons[PM_GELATINOUS_CUBE]);
    int x = mon->mx, y = mon->my;
    struct obj *cobj;

    if (!Has_contents(otmp) || !isok(x, y)
        || otmp->otyp == CRYSTAL_CHEST)
        return;

    /* contents of eaten containers become engulfed or dropped onto
      the floor; this is arbitrary, but otherwise g-cubes are too
      powerful */
    if (!engulf_contents && cansee(x, y)) {
        pline("%s contents spill out onto the %s.",
              s_suffix(The(distant_name(otmp, xname))),
              surface(x, y));
    }
    while ((cobj = otmp->cobj) != 0) {
        obj_extract_self(cobj);
        if (otmp->otyp == ICE_BOX)
            removed_from_icebox(cobj);
        if (engulf_contents) {
            (void) mpickobj(mon, cobj);
        } else {
            if (!flooreffects(cobj, x, y, ""))
                place_object(cobj, x, y);
        }
    }
}

#define mstoning(obj) \
    (ofood(obj) && ismnum(obj->corpsenm)      \
     && flesh_petrifies(&mons[obj->corpsenm]))

/* Monster mtmp consumes an object.
   Monster may die, polymorph, grow up, heal, etc; meating is not changed.
   Object is extracted from any linked list and freed. */
void
m_consume_obj(struct monst *mtmp, struct obj *otmp)
{
    boolean ispet = mtmp->mtame;

    /* non-pet: Heal up to the object's weight in hp */
    if (!ispet && mtmp->mhp < mtmp->mhpmax) {
        healmon(mtmp, objects[otmp->otyp].oc_weight, 0);
    }
    if (Has_contents(otmp))
        meatbox(mtmp, otmp);
    if (otmp == uball) {
        unpunish();
        delobj(otmp);
    } else if (otmp == uchain) {
        unpunish(); /* frees uchain */
    } else {
        boolean deadmimic, slimer, unsick, pyro_egg;
        int poly, grow, heal, eyes, mstone, vis = canseemon(mtmp);
        int corpsenm = (otmp->otyp == CORPSE ? otmp->corpsenm : NON_PM);

        deadmimic = (otmp->otyp == CORPSE && (corpsenm == PM_SMALL_MIMIC
                                              || corpsenm == PM_LARGE_MIMIC
                                              || corpsenm == PM_GIANT_MIMIC));
        /* computed before delobj() frees otmp; note that corpsenm is
           NON_PM for anything that is not a corpse, so this condition
           cannot currently be true - whether a pyrolisk egg is meant to
           go off when eaten is a separate question */
        pyro_egg = (otmp->otyp == EGG && corpsenm == PM_PYROLISK);
        slimer = (otmp->otyp == GLOB_OF_GREEN_SLIME);
        unsick = (otmp->otyp == EUCALYPTUS_LEAF
                  && (mtmp->msick || mtmp->mdiseased));
        poly = polyfood(otmp);
        grow = mlevelgain(otmp);
        heal = mhealup(otmp);
        eyes = (otmp->otyp == CARROT);
        mstone = mstoning(otmp);
        delobj(otmp); /* munch */
        if (poly || slimer) {
            struct permonst *ptr = slimer ? &mons[PM_GREEN_SLIME] : 0;

            (void) newcham(mtmp, ptr, vis ? NC_SHOW_MSG : NO_NC_FLAGS);
        }
        if (grow) {
            if ((ispet && (int) mtmp->m_lev < (int) mtmp->data->mlevel + 15)
                || !ispet)
                (void) grow_up(mtmp, (struct monst *) 0);
        }
        if (mstone) {
            if (poly_when_stoned(mtmp->data)) {
                mon_to_stone(mtmp);
            } else if (!resists_ston(mtmp)) {
                if (vis)
                    pline_mon(mtmp, "%s turns to stone!",
                              Monnam(mtmp));
                monstone(mtmp);
            }
        }
        if (unsick) {
            mtmp->msick = mtmp->mdiseased = 0;
            if (vis)
                pline_mon(mtmp, "%s is no longer ill.", Monnam(mtmp));
        }
        if (heal)
            healmon(mtmp, mtmp->mhpmax, 0);
        if ((eyes || heal) && !mtmp->mcansee)
            mcureblindness(mtmp, canseemon(mtmp));
        if (ispet && deadmimic) {
            quickmimic(mtmp);
            /* a pet steed taking on the mimic shape is dismounted first,
               and that dismount can kill it */
            if (DEADMONSTER(mtmp))
                return;
        }
        if (pyro_egg)
            explode(mtmp->mx, mtmp->my, BZ_M_SPELL(ZT_FIRE), d(3, 6), 0,
                    EXPL_FIERY);
        if (corpsenm != NON_PM)
            mon_givit(mtmp, &mons[corpsenm]);
    }
}

/*
 * Maybe eat a metallic object (not just gold).
 * Return value: 0 => nothing happened, 1 => monster ate something,
 * 2 => monster died (it must have grown into a genocided form, but
 * that can't happen at present because nothing which eats objects
 * has young and old forms).
 */
int
meatmetal(struct monst *mtmp)
{
    struct obj *otmp;
    char *otmpname;
    int vis = canseemon(mtmp);

    /* If a pet, eating is handled separately, in dog.c */
    if (mtmp->mtame)
        return 0;

    /* Eats topmost metal object if it is there */
    for (otmp = svl.level.objects[mtmp->mx][mtmp->my]; otmp;
         otmp = otmp->nexthere) {
        /* Don't eat indigestible/choking/inappropriate objects */
        if ((mtmp->data == &mons[PM_RUST_MONSTER] && !is_rustprone(otmp))
            || (otmp->otyp == AMULET_OF_STRANGULATION
                || otmp->otyp == RIN_SLOW_DIGESTION)
            || (otmp->opoisoned && !resists_poison(mtmp)))
            continue;
        if (is_metallic(otmp) && !obj_resists(otmp, 5, 95)
            && touch_artifact(otmp, mtmp)) {
            if (mtmp->data == &mons[PM_RUST_MONSTER] && otmp->oerodeproof) {
                if (vis) {
                    /* call distant_name() for its side-effects even when
                       !verbose so won't be printed */
                    otmpname = distant_name(otmp, doname);
                    if (flags.verbose)
                        pline_mon(mtmp, "%s eats %s!",
                                  Monnam(mtmp), otmpname);
                }
                /* The object's rustproofing is gone now */
                otmp->oerodeproof = 0;
                mtmp->mstun = 1;
                if (vis) {
                    /* (see above; format even if it won't be printed) */
                    otmpname = distant_name(otmp, doname);
                    if (flags.verbose)
                        pline_mon(mtmp, "%s spits %s out in disgust!",
                              Monnam(mtmp), otmpname);
                }
            } else {
                if (cansee(mtmp->mx, mtmp->my)) {
                    /* (see above; format even if it won't be printed) */
                    otmpname = distant_name(otmp, doname);
                    if (flags.verbose)
                        pline_mon(mtmp, "%s eats %s!",
                                  Monnam(mtmp), otmpname);
                } else {
                    if (flags.verbose) {
                        Soundeffect(se_crunching_sound, 50);
                        You_hear("a crunching sound.");
                    }
                }
                mtmp->meating = otmp->owt / 2 + 1;
                m_consume_obj(mtmp, otmp);
                if (DEADMONSTER(mtmp))
                    return 2;
                /* Left behind a pile? */
                if (rnd(25) < 3)
                    (void) mksobj_at(ROCK, mtmp->mx, mtmp->my, TRUE, FALSE);
                newsym(mtmp->mx, mtmp->my);
                return 1;
            }
        }
    }
    return 0;
}

/* monster eats a pile of objects */
int
meatobj(struct monst *mtmp) /* for gelatinous cubes */
{
    struct obj *otmp, *otmp2;
    struct permonst *ptr, *original_ptr = mtmp->data;
    int count = 0, ecount = 0;
    char buf[BUFSZ], *otmpname;

    buf[0] = '\0';
    /* If a pet, eating is handled separately, in dog.c */
    if (mtmp->mtame)
        return 0;

    /* eat organic objects, including cloth and wood, if present;
       engulf others, except huge rocks and metal attached to player
       [despite comment at top, doesn't assume that eater is a g-cube] */
    for (otmp = svl.level.objects[mtmp->mx][mtmp->my]; otmp; otmp = otmp2) {
        otmp2 = otmp->nexthere;

        /* avoid special items; once hero picks them up, they'll cease
           being special, becoming eligible for engulf and devore if
           dropped again */
        if (is_mines_prize(otmp) || is_soko_prize(otmp))
            continue;

        /* touch sensitive items */
        if (otmp->otyp == CORPSE && is_rider(&mons[otmp->corpsenm])) {
            int ox = otmp->ox, oy = otmp->oy;
            boolean revived_it = revive_corpse(otmp);

            newsym(ox, oy);
            /* Rider corpse isn't just inedible; can't engulf it either */
            if (!revived_it)
                continue;
            /* [should check whether revival forced 'mtmp' off the level
               and return 3 in that situation (if possible...)] */
            break;

        /* untouchable (or inaccessible) items */
        } else if ((otmp->otyp == CORPSE
                    && touch_petrifies(&mons[otmp->corpsenm])
                    && !resists_ston(mtmp))
                   /* don't engulf boulders and statues or ball&chain */
                   || otmp->oclass == ROCK_CLASS
                   || otmp == uball || otmp == uchain
                   /* normally mtmp won't have stepped onto scare monster
                      scroll, but if it does, don't eat or engulf that
                      (note: scrolls inside eaten containers will still
                      become engulfed) */
                   || otmp->otyp == SCR_SCARE_MONSTER) {
            /* do nothing--neither eaten nor engulfed */
            continue;

        /* inedible items -- engulf these */
        } else if (!is_organic(otmp) || obj_resists(otmp, 5, 95)
                   || !touch_artifact(otmp, mtmp)
                   /* redundant due to non-organic composition but
                      included for emphasis */
                   || (otmp->otyp == AMULET_OF_STRANGULATION
                       || otmp->otyp == RIN_SLOW_DIGESTION)
                   || (otmp->opoisoned && !resists_poison(mtmp))
                   /* cockatrice corpses handled above; this
                      touch_petrifies() check catches eggs */
                   || (mstoning(otmp) && !resists_ston(mtmp))
                   || (otmp->otyp == GLOB_OF_GREEN_SLIME
                       && !slimeproof(mtmp->data))) {
            /* engulf */
            ++ecount;
            /* call distant_name() for its possible side-effects even if
               the result won't be printed */
            otmpname = distant_name(otmp, doname);
            if (ecount == 1)
                Sprintf(buf, "%s engulfs %s.", Monnam(mtmp), otmpname);
            else if (ecount == 2)
                Sprintf(buf, "%s engulfs several objects.", Monnam(mtmp));
            obj_extract_self(otmp);
            (void) mpickobj(mtmp, otmp); /* slurp */

        /* lastly, edible items; yum! */
        } else {
            /* devour */
            ++count;
            if (cansee(mtmp->mx, mtmp->my)) {
                /* (see above; distant_name() sometimes has side-effects */
                otmpname = distant_name(otmp, doname);
                if (flags.verbose)
                    pline_mon(mtmp, "%s eats %s!",
                              Monnam(mtmp), otmpname);
                /* give this one even if !verbose */
                if (otmp->oclass == SCROLL_CLASS
                    && objdescr_is(otmp, "YUM YUM"))
                    pline("Yum%c", otmp->blessed ? '!' : '.');
            } else {
                Soundeffect(se_slurping_sound, 30);
                if (flags.verbose)
                    You_hear("a slurping sound.");
            }
            m_consume_obj(mtmp, otmp);
            /* in case it polymorphed or died */
            ptr = mtmp->data;
            if (ptr != original_ptr)
                return !ptr ? 2 : 1;
        }

        /* Engulf & devour is instant, so don't set meating */
        if (mtmp->minvis)
            newsym(mtmp->mx, mtmp->my);
    }

    if (ecount > 0) {
        if (cansee(mtmp->mx, mtmp->my) && flags.verbose && buf[0])
            pline1(buf);
        else if (flags.verbose)
            You_hear("%s slurping sound%s.",
                     (ecount == 1) ? "a" : "several", plur(ecount));
    }
    return (count > 0 || ecount > 0) ? 1 : 0;
}

#undef mstoning

/* Monster eats a corpse off the ground.
   Return value is 0 = nothing eaten, 1 = ate a corpse, 2 = died. */
int
meatcorpse(
    struct monst *mtmp) /* for purple worms and other voracious monsters */
{
    struct obj *otmp;
    struct permonst *ptr, *original_ptr = mtmp->data, *corpsepm;
    coordxy x = mtmp->mx, y = mtmp->my;

    /* if a pet, eating is handled separately, in dog.c */
    if (mtmp->mtame)
        return 0;

    /* skips past any globs */
    for (otmp = sobj_at(CORPSE, x, y); otmp;
         /* won't get back here if otmp is split or gets used up */
         otmp = nxtobj(otmp, CORPSE, TRUE)) {

        corpsepm = &mons[otmp->corpsenm];
        /* skip some corpses */
        if (vegan(corpsepm) /* ignore veggy corpse even if omnivorous */
            /* don't eat harmful corpses */
            || (flesh_petrifies(corpsepm) && !resists_ston(mtmp)))
            continue;
        if (is_rider(corpsepm)) {
            boolean revived_it = revive_corpse(otmp);

            newsym(x, y); /* corpse is gone; mtmp might be too so do this now
                             since we're bypassing the bottom of the loop */
            if (!revived_it)
                continue; /* revival failed? if so, corpse is gone */
            /* Successful Rider revival; unlike skipped corpses, don't
               just move on to next corpse as if nothing has happened.
               [Can Rider revival bump 'mtmp' off level when it's full?
               We ought to return 3 if that happens.] */
            break;
        }

        if (otmp->quan > 1)
            otmp = splitobj(otmp, 1L);

        if (cansee(x, y) && canseemon(mtmp)) {
            /* call distant_name() for its possible side-effects even if
               the result won't be printed */
            char *otmpname = distant_name(otmp, doname);

            if (flags.verbose)
                pline_mon(mtmp, "%s eats %s!",
                          Monnam(mtmp), otmpname);
        } else {
            Soundeffect(se_masticating_sound, 50);
            if (flags.verbose)
                You_hear("a masticating sound.");
        }

        m_consume_obj(mtmp, otmp);
        /* in case it polymorphed or died */
        ptr = mtmp->data;
        if (ptr != original_ptr)
            return !ptr ? 2 : 1;

        /* Engulf & devour is instant, so don't set meating */
        if (mtmp->minvis)
            newsym(x, y);

        return 1;
    }
    return 0;
}

/* give monster property prop */
void
mon_give_prop(struct monst *mtmp, int prop)
{
    const char *msg = NULL;
    unsigned short intrinsic = 0; /* MR_* constant */

    /* Pets don't have all the fields that the hero does, so they can't get
       all the same intrinsics.  If it happens to choose strength gain or
       teleport control or whatever, ignore it. */
    switch (prop) {
    case FIRE_RES:
        msg = "%s shivers slightly.";
        break;
    case COLD_RES:
        msg = "%s looks quite warm.";
        break;
    case SLEEP_RES:
        msg = "%s looks wide awake.";
        break;
    case DISINT_RES:
        msg = "%s looks very firm.";
        break;
    case SHOCK_RES:
        msg = "%s crackles with static electricity.";
        break;
    case POISON_RES:
        msg = "%s looks healthy.";
        break;
    default:
        return; /* can't give it */
        break;
    }
    intrinsic = res_to_mr(prop);

    /* Don't give message if it already had this property intrinsically, but
       still do grant the intrinsic if it only had it from mresists.
       Do print the message if it only had this property extrinsically, which
       is why mon_resistancebits isn't used here. */
    if ((mtmp->data->mresists | mtmp->mintrinsics) & intrinsic)
        msg = (const char *) 0;

    if (intrinsic)
        mtmp->mintrinsics |= intrinsic;

    if (canseemon(mtmp) && msg) {
        DISABLE_WARNING_FORMAT_NONLITERAL
        pline_mon(mtmp, msg, Monnam(mtmp));
        RESTORE_WARNING_FORMAT_NONLITERAL
    }
}

/* Maybe give an intrinsic to monster from eating corpse that confers it. */
void
mon_givit(struct monst *mtmp, struct permonst *ptr)
{
    int prop = corpse_intrinsic(ptr);
    boolean vis = canseemon(mtmp);

    if (DEADMONSTER(mtmp))
        return;

    if (ptr == &mons[PM_STALKER]) {
        /*
         * Invisible stalker isn't flagged as conferring invisibility
         * so prop is 0.  For hero, eating a stalker corpse confers
         * temporary invisibility if hero is visible.  When already
         * invisible, if confers permanent invisibility and also
         * permanent see invisible.  For monsters, only permanent
         * invisibility is possible; temporary invisibility and see
         * invisible aren't implemented for them.
         *
         * A monster being invisible gains no benefit against other
         * monsters, and an invisible pet when hero can't see invisible
         * is a nuisance at best, so this is probably detrimental.
         * Players will just have to live with it if they want to be
         * able to have pets gain intrinsics from eating corpses.
         */
        if (!mtmp->perminvis || mtmp->invis_blkd) {
            char mtmpbuf[BUFSZ];

            Strcpy(mtmpbuf, Monnam(mtmp));
            mon_set_minvis(mtmp, FALSE);
            if (vis)
                pline_mon(mtmp, "%s %s.", mtmpbuf,
                      !canspotmon(mtmp) ? "vanishes"
                      : mtmp->invis_blkd ? "seems to flicker"
                        : "becomes invisible");
        }
        mtmp->mstun = 1; /* no timeout but will eventually wear off */
        return;
    }

    if (prop == 0)
        return; /* no intrinsic from this corpse */

    if (!should_givit(prop, ptr))
        return; /* failed die roll */

    mon_give_prop(mtmp, prop);
}

void
mpickgold(struct monst *mtmp)
{
    struct obj *gold;
    int mat_idx;

    if ((gold = g_at(mtmp->mx, mtmp->my)) != 0) {
        mat_idx = objects[gold->otyp].oc_material;
        obj_extract_self(gold);
        add_to_minv(mtmp, gold);
        if (cansee(mtmp->mx, mtmp->my)) {
            if (flags.verbose && !mtmp->isgd)
                pline_mon(mtmp, "%s picks up some %s.", Monnam(mtmp),
                         mat_idx == GOLD ? "gold" : "money");
            newsym(mtmp->mx, mtmp->my);
        }
    }
}

/* monster picks up one item stack from the map location they are at */
boolean
mpickstuff(struct monst *mtmp)
{
    struct obj *otmp, *otmp2, *otmp3;
    int carryamt = 0;

    /* TODO-EVILHACK: master lets monsters unlock and loot containers
       here (keyed on carried unlocking tools, with magic chests gated
       on MAGIC_KEY and crystal chests off limits), splices the shared
       magic chest into this walk and into m_move's goal search
       (likes_contents), and has searches_for_item() cover keys so
       monsters pick them up; port all of that together as the
       monster-container-looting slice.
       WHEN THAT LANDS it must exclude BAG_OF_TREASURE and
       BAG_OF_DEVOURING explicitly, as master's own arm does: they are
       Is_container() and merely being TOOL_CLASS is not enough to keep
       them out, and a cursed bag of treasure weighs its contents x10,
       which makes !can_carry true and so routes a dropped one straight
       into the crack-it-open path */

    /* prevent shopkeepers from leaving the door of their shop */
    if (mtmp->isshk && inhishop(mtmp))
        return FALSE;

    /* non-tame monsters normally don't go shopping */
    if (!mtmp->mtame && *in_rooms(mtmp->mx, mtmp->my, SHOPBASE) && rn2(25))
        return FALSE;

    /* item in a pool, but monster can't swim */
    if (!could_reach_item(mtmp, mtmp->mx, mtmp->my))
        return FALSE;

    for (otmp = svl.level.objects[mtmp->mx][mtmp->my]; otmp; otmp = otmp2) {
        otmp2 = otmp->nexthere;

        /* avoid special items; once hero picks them up, they'll cease
           being special, becoming eligible for normal pickup */
        if (is_mines_prize(otmp) || is_soko_prize(otmp))
            continue;

        /* Nymphs take everything.  Most monsters don't pick up corpses. */
        if (mon_would_take_item(mtmp, otmp)) {

            if (otmp->otyp == CORPSE && mtmp->data->mlet != S_NYMPH
                /* let a handful of corpse types thru to can_carry() */
                && !touch_petrifies(&mons[otmp->corpsenm])
                && otmp->corpsenm != PM_LIZARD
                && !acidic(&mons[otmp->corpsenm]))
                continue;
            if (!can_touch_safely(mtmp, otmp))
                continue;
            carryamt = can_carry(mtmp, otmp);
            if (carryamt == 0)
                continue;
            /* handle cases where the critter can only get some */
            otmp3 = otmp;
            if (carryamt != otmp->quan) {
                otmp3 = splitobj(otmp, carryamt);
            }
            if (cansee(mtmp->mx, mtmp->my)) {
                /* call distant_name() for its possible side-effects even
                   if the result won't be printed; do it before the extract
                   from floor and subsequent pickup by mtmp */
                char *otmpname = distant_name(otmp, doname);

                if (flags.verbose)
                    pline_mon(mtmp, "%s picks up %s.",
                              Monnam(mtmp), otmpname);
            }
            obj_extract_self(otmp3);      /* remove from floor */
            (void) mpickobj(mtmp, otmp3); /* may merge and free otmp3 */
            /* let them try to equip it on the next turn */
            check_gear_next_turn(mtmp);
            newsym(mtmp->mx, mtmp->my);
            return TRUE; /* pick only one object */
        }
    }
    return FALSE;
}

int
curr_mon_load(struct monst *mtmp)
{
    int curload = 0;
    struct obj *obj;

    for (obj = mtmp->minvent; obj; obj = obj->nobj) {
        if (obj->otyp != BOULDER || !racial_throws_rocks(mtmp))
            curload += obj->owt;
    }

    return curload;
}

int
max_mon_load(struct monst *mtmp)
{
    long maxload;
    long maxcarrcap = MAX_CARR_CAP;

    if (racial_giant(mtmp) || racial_centaur(mtmp))
        maxcarrcap += 400;

    if (racial_tortle(mtmp))
        maxcarrcap += 200;

    /* Base monster carrying capacity is equal to human maximum
     * carrying capacity, or half human maximum if not strong.
     * (for a polymorphed player, the value used would be the
     * non-polymorphed carrying capacity instead of max/half max).
     * This is then modified by the ratio between the monster weights
     * and human weights.  Corpseless monsters are given a capacity
     * proportional to their size instead of weight.
     */
    if (!mtmp->data->cwt)
        maxload = (maxcarrcap * (long) mtmp->data->msize) / MZ_HUMAN;
    else if (!strongmonst(mtmp->data)
             || (strongmonst(mtmp->data) && (mtmp->data->cwt > WT_HUMAN)))
        maxload = (maxcarrcap * (long) mtmp->data->cwt) / WT_HUMAN;
    else
        maxload = maxcarrcap; /*strong monsters w/cwt <= WT_HUMAN*/

    if (!strongmonst(mtmp->data))
        maxload /= 2;

    if (maxload < 1)
        maxload = 1;

    return (int) maxload;
}

/* can monster touch object safely? */
boolean
can_touch_safely(struct monst *mtmp, struct obj *otmp)
{
    int otyp = otmp->otyp;
    struct permonst *mdat = mtmp->data;

    if (otyp == CORPSE && touch_petrifies(&mons[otmp->corpsenm])
        && !(mtmp->misc_worn_check & W_ARMG) && !resists_ston(mtmp))
        return FALSE;
    if (otyp == CORPSE && is_rider(&mons[otmp->corpsenm]))
        return FALSE;
    if (mon_hates_material(mtmp, otmp->material)
        && (otyp != BELL_OF_OPENING || !is_covetous(mdat)))
        return FALSE;
    if (!touch_artifact(otmp, mtmp))
        return FALSE;
    return TRUE;
}

/* for restricting monsters' object-pickup.
 *
 * to support the new pet behavior, this now returns the max # of objects
 * that a given monster could pick up from a pile. frequently this will be
 * otmp->quan, but special cases for 'only one' now exist.
 *
 * this will probably cause very amusing behavior with pets and gold coins.
 *
 * TODO: allow picking up 2-N objects from a pile of N based on weight.
 *       Change from 'int' to 'long' to accommodate big stacks of gold.
 *       Right now we fake it by reporting a partial quantity, but the
 *       likesgold handling m_move results in picking up the whole stack.
 */
int
can_carry(struct monst *mtmp, struct obj *otmp)
{
    int iquan, otyp = otmp->otyp, newload = otmp->owt;
    struct permonst *mdat = mtmp->data;
    short nattk = 0;

    if (notake(mdat))
        return 0; /* can't carry anything */

    if (!can_touch_safely(mtmp, otmp))
        return 0;

    /* hostile monsters who like gold will pick up the whole stack;
       tame monsters with hands will pick up the partial stack */
    iquan = (otmp->quan > (long) LARGEST_INT)
               ? 20000 + rn2(LARGEST_INT - 20000 + 1)
               : (int) otmp->quan;

    /* monsters without hands can't pick up multiple objects at once
     * unless they have an engulfing attack
     *
     * ...dragons, of course, can always carry gold pieces and gems somehow
     */
    if (iquan > 1) {
        boolean glomper = FALSE;

        if (mtmp->data->mlet == S_DRAGON
            && (otmp->oclass == COIN_CLASS
                || otmp->oclass == GEM_CLASS)) {
            glomper = TRUE;
        } else {
            struct attack *mattk = has_erac(mtmp) ? ERAC(mtmp)->mattk
                                                  : mtmp->data->mattk;

            for (nattk = 0; nattk < NATTK; nattk++)
                if (mattk[nattk].aatyp == AT_ENGL) {
                    glomper = TRUE;
                    break;
                }
        }
        if ((mtmp->data->mflags1 & M1_NOHANDS) && !glomper)
            return 1;
    }

    /* steeds don't pick up stuff (to avoid shop abuse) */
    if (mtmp == u.usteed)
        return 0;
    if (mtmp->isshk)
        return iquan; /* no limit */
    if (mtmp->mpeaceful && !mtmp->mtame)
        return 0;
    /* otherwise players might find themselves obligated to violate
     * their alignment if the monster takes something they need
     */

    /* special--boulder throwers carry unlimited amounts of boulders */
    if (racial_throws_rocks(mtmp) && otyp == BOULDER)
        return iquan;

    /* gnomes hate eggs */
    if (otyp == EGG && racial_gnome(mtmp))
        return 0;

    /* nymphs deal in stolen merchandise, but not boulders or statues */
    if (mdat->mlet == S_NYMPH)
        return (otmp->oclass == ROCK_CLASS) ? 0 : iquan;

    if (curr_mon_load(mtmp) + newload > max_mon_load(mtmp))
        return 0;

    return iquan;
}

/* is <nx,ny> in direct line with where 'mon' thinks hero is? */
staticfn boolean
monlineu(struct monst *mon, int nx, int ny)
{
    return online2(nx, ny, mon->mux, mon->muy);
}

/* return flags based on monster data, for mfndpos() */
long
mon_allowflags(struct monst *mtmp)
{
    long allowflags = 0L;
    boolean can_open = !(nohands(mtmp->data) || verysmall(mtmp->data));
    boolean can_unlock = ((can_open && monhaskey(mtmp, TRUE))
                          || mtmp->iswiz || is_rider(mtmp->data));
    boolean doorbuster = racial_giant(mtmp);
    /* don't tunnel if on rogue level or if hostile and close enough
       to prefer a weapon; same criteria as in m_move() */
    boolean can_tunnel = (racial_tunnels(mtmp) && !Is_rogue_level(&u.uz));

    if (can_tunnel && racial_needspick(mtmp)
        && ((!mtmp->mpeaceful || Conflict)
            && dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) <= 8))
        can_tunnel = FALSE;

    if (mtmp->mtame)
        allowflags |= ALLOW_M | ALLOW_TRAPS | ALLOW_SANCT | ALLOW_SSM;
    else if (mtmp->mpeaceful)
        allowflags |= ALLOW_SANCT | ALLOW_SSM;
    else
        allowflags |= ALLOW_U;
    if (Conflict && !resist_conflict(mtmp))
        allowflags |= ALLOW_U;
    if (mtmp->isshk)
        allowflags |= ALLOW_SSM;
    if (mtmp->ispriest)
        allowflags |= ALLOW_SSM | ALLOW_SANCT;
    if (passes_walls(mtmp->data))
        allowflags |= (ALLOW_ROCK | ALLOW_WALL);
    if (racial_throws_rocks(mtmp) || m_can_break_boulder(mtmp))
        allowflags |= ALLOW_ROCK;
    if (can_tunnel)
        allowflags |= ALLOW_DIG;
    if (doorbuster)
        allowflags |= BUSTDOOR;
    if (can_open)
        allowflags |= OPENDOOR;
    if (can_unlock)
        allowflags |= UNLOCKDOOR;
    if (passes_bars(mtmp->data)
        /* restrict engulfer or holder who might try to pass iron bars while
           carrying hero; accept small subset for poly'd hero passes_bars() */
        && (mtmp != u.ustuck || (unsolid(gy.youmonst.data)
                                 || verysmall(gy.youmonst.data))))
        allowflags |= ALLOW_BARS;
#if 0   /* can't do this here; leave it for mfndpos() */
    if (is_displacer(mtmp->data))
        allowflags |= ALLOW_MDISP;
#endif
    if (is_minion(mtmp->data) || is_rider(mtmp->data))
        allowflags |= ALLOW_SANCT;
    /* unicorn may not be able to avoid hero on a noteleport level */
    if (is_unicorn(mtmp->data) && !noteleport_level(mtmp))
        allowflags |= NOTONL;
    if (is_human(mtmp->data) || mtmp->data == &mons[PM_MINOTAUR])
        allowflags |= ALLOW_SSM;
    if ((is_undead(mtmp->data) && mtmp->data->mlet != S_GHOST)
        || is_vampshifter(mtmp))
        allowflags |= NOGARLIC;

    return allowflags;
}

/* return TRUE if monster is up in the air/on the ceiling */
boolean
m_in_air(struct monst *mtmp)
{
    return (is_flyer(mtmp->data)
            || is_floater(mtmp->data)
            || (is_clinger(mtmp->data)
                && has_ceiling(&u.uz) && mtmp->mundetected));
}

/* return number of acceptable neighbour positions */
int
mfndpos(
    struct monst *mon,
    struct mfndposdata *data,
    long flag)
{
    struct permonst *mdat = mon->data;
    struct trap *ttmp;
    coordxy x, y, nx, ny;
    int cnt = 0;
    uchar ntyp;
    uchar nowtyp;
    boolean wantpool, poolok, lavaok, nodiag;
    boolean rockok = FALSE, treeok = FALSE, thrudoor;
    int maxx, maxy;
    boolean poisongas_ok, in_poisongas;
    NhRegion *gas_reg;
    int gas_glyph = cmap_to_glyph(S_poisoncloud);

    x = mon->mx;
    y = mon->my;
    nowtyp = levl[x][y].typ;

    (void)memset((genericptr_t) data, 0, sizeof(struct mfndposdata));

    nodiag = NODIAG(mdat - mons);
    wantpool = (mdat->mlet == S_EEL);
    poolok = ((!Is_waterlevel(&u.uz)
               && (m_in_air(mon) || can_levitate(mon) || can_fly(mon)))
              || (is_swimmer(mdat) && !wantpool));
    lavaok = (m_in_air(mon) || can_levitate(mon) || can_fly(mon)
              || likes_lava(mdat));
    if (mdat == &mons[PM_FLOATING_EYE]) /* prefers to avoid heat */
        lavaok = FALSE;
    thrudoor = ((flag & (ALLOW_WALL | BUSTDOOR)) != 0L);
    poisongas_ok = (m_poisongas_ok(mon) == M_POISONGAS_OK);
    in_poisongas = ((gas_reg = visible_region_at(x,y)) != 0
                    && gas_reg->glyph == gas_glyph);

    if (flag & ALLOW_DIG) {
        struct obj *mw_tmp;

        /* need to be specific about what can currently be dug */
        if (!racial_needspick(mon)) {
            rockok = treeok = TRUE;
        } else if ((mw_tmp = MON_WEP(mon)) && mw_tmp->cursed
                   && mon->weapon_check == NO_WEAPON_WANTED) {
            rockok = is_pick(mw_tmp);
            treeok = is_axe(mw_tmp);
        } else {
            rockok = (m_carrying(mon, PICK_AXE)
                      || (m_carrying(mon, DWARVISH_MATTOCK)
                          && !which_armor(mon, W_ARMS)));
            treeok = (m_carrying(mon, AXE) || (m_carrying(mon, BATTLE_AXE)
                                               && !which_armor(mon, W_ARMS)));
        }
        if (rockok || treeok)
            thrudoor = TRUE;
    }

 nexttry: /* eels prefer the water, but if there is no water nearby,
             they will crawl over land */
    if (mon->mconf) {
        flag |= ALLOW_ALL;
        flag &= ~NOTONL;
    }
    if (!mon->mcansee)
        flag |= ALLOW_SSM;
    maxx = min(x + 1, COLNO - 1);
    maxy = min(y + 1, ROWNO - 1);
    for (nx = max(1, x - 1); nx <= maxx; nx++)
        for (ny = max(0, y - 1); ny <= maxy; ny++) {
            if (nx == x && ny == y)
                continue;
            ntyp = levl[nx][ny].typ;
            if (IS_OBSTRUCTED(ntyp)
                && !((flag & ALLOW_WALL) && may_passwall(nx, ny))
                && (!((IS_TREES(ntyp) ? treeok : rockok) && may_dig(nx, ny))
                    /* a non-metallivore can't dig through a metal secret
                       door (which reads as rock here) */
                    || (ntyp == SDOOR && metal_door(&levl[nx][ny])
                        && !metallivorous(mdat))))
                continue;
            /* intelligent peacefuls avoid digging shop/temple walls */
            if (IS_OBSTRUCTED(ntyp) && rockok
                && !mindless(mon->data) && (mon->mpeaceful || mon->mtame)
                && (*in_rooms(nx, ny, TEMPLE) || *in_rooms(nx, ny, SHOPBASE))
                && !(*in_rooms(x, y, TEMPLE) || *in_rooms(x, y, SHOPBASE)))
                continue;
            if (IS_WATERWALL(ntyp) && !is_swimmer(mdat))
                continue;
            /* KMH -- Added iron bars */
            if (ntyp == IRONBARS
                && (!(flag & ALLOW_BARS)
                    || ((levl[nx][ny].wall_info & W_NONDIGGABLE)
                        && (dmgtype(mdat, AD_RUST)
                            || dmgtype(mdat, AD_CORR)))))
                continue;
            if (IS_DOOR(ntyp)
                /* an amorphous creature can only move under/through a
                   closed door if it doesn't currently have hero engulfed */
                && !((amorphous(mdat) || can_fog(mon)) && !engulfing_u(mon))
                && (((levl[nx][ny].doormask & D_CLOSED) && !(flag & OPENDOOR))
                    || ((levl[nx][ny].doormask & D_LOCKED)
                        && !(flag & UNLOCKDOOR)))
                /* a hard (metal or stone) door resists busting; metal
                   yields only to a metallivore, stone only to a digger,
                   and either to phasing */
                && !(thrudoor
                     && (!hard_door(&levl[nx][ny]) || (flag & ALLOW_WALL)
                         || (metal_door(&levl[nx][ny])
                             && metallivorous(mdat))
                         || (door_material(&levl[nx][ny]) == MINERAL
                             && rockok))))
                continue;
            /* avoid poison gas? */
            if (!poisongas_ok && !in_poisongas
                && (gas_reg = visible_region_at(nx,ny)) != 0
                && gas_reg->glyph == gas_glyph)
                continue;
            /* first diagonal checks (tight squeezes handled below) */
            if (nx != x && ny != y
                && (nodiag
                    || (IS_DOOR(nowtyp) && (levl[x][y].doormask & ~D_BROKEN))
                    || (IS_DOOR(ntyp) && (levl[nx][ny].doormask & ~D_BROKEN))
                    || ((IS_DOOR(nowtyp) || IS_DOOR(ntyp))
                        && Is_rogue_level(&u.uz))
                    /* mustn't pass between adjacent long worm segments,
                       but can attack that way */
                    || (m_at(x, ny) && m_at(nx, y) && worm_cross(x, y, nx, ny)
                        && !m_at(nx, ny) && (nx != u.ux || ny != u.uy))))
                continue;
            if ((!lavaok || !(flag & ALLOW_WALL)) && ntyp == LAVAWALL)
                continue;
            if ((poolok || is_pool(nx, ny) == wantpool)
                && (lavaok || !is_lava(nx, ny))) {
                int dispx, dispy;
                boolean monseeu = (mon->mcansee
                                   && (!Invis || mon_prop(mon, SEE_INVIS)));
                boolean checkobj = OBJ_AT(nx, ny);

                /* Displacement also displaces the Elbereth/scare monster,
                 * as long as you are visible.
                 */
                if (Displaced && monseeu
                    && mon->mux == nx && mon->muy == ny) {
                    dispx = u.ux;
                    dispy = u.uy;
                } else {
                    dispx = nx;
                    dispy = ny;
                }

                data->info[cnt] = 0;
                if (onscary(dispx, dispy, mon)) {
                    if (!(flag & ALLOW_SSM))
                        continue;
                    data->info[cnt] |= ALLOW_SSM;
                }
                if (u_at(nx, ny)
                    || (nx == mon->mux && ny == mon->muy)) {
                    if (u_at(nx, ny)) {
                        /* If it's right next to you, it found you,
                         * displaced or no.  We must set mux and muy
                         * right now, so when we return we can tell
                         * that the ALLOW_U means to attack _you_ and
                         * not the image.
                         */
                        mon->mux = u.ux;
                        mon->muy = u.uy;
                    }
                    if (!(flag & ALLOW_U))
                        continue;
                    data->info[cnt] |= ALLOW_U;
                } else {
                    if (MON_AT(nx, ny)) {
                        struct monst *mtmp2 = m_at(nx, ny);
                        long mmflag = flag | mm_aggression(mon, mtmp2);

                        if (mmflag & ALLOW_M) {
                            data->info[cnt] |= ALLOW_M;
                            if (mtmp2->mtame) {
                                if (!(mmflag & ALLOW_TM))
                                    continue;
                                data->info[cnt] |= ALLOW_TM;
                            }
                        } else {
                            flag &= ~ALLOW_MDISP; /* depends upon defender */
                            mmflag = flag | mm_displacement(mon, mtmp2);
                            if (!(mmflag & ALLOW_MDISP))
                                continue;
                            data->info[cnt] |= ALLOW_MDISP;
                        }
                    }
                    /* Note: ALLOW_SANCT only prevents movement, not
                       attack, into a temple. */
                    if (svl.level.flags.has_temple
                        && *in_rooms(nx, ny, TEMPLE)
                        && !*in_rooms(x, y, TEMPLE)
                        && in_your_sanctuary((struct monst *) 0, nx, ny)) {
                        if (!(flag & ALLOW_SANCT))
                            continue;
                        data->info[cnt] |= ALLOW_SANCT;
                    }
                }
                if (checkobj && sobj_at(CLOVE_OF_GARLIC, nx, ny)) {
                    if (flag & NOGARLIC)
                        continue;
                    data->info[cnt] |= NOGARLIC;
                }
                if (checkobj && sobj_at(BOULDER, nx, ny)) {
                    if (!(flag & ALLOW_ROCK))
                        continue;
                    data->info[cnt] |= ALLOW_ROCK;
                }
                if (monseeu && monlineu(mon, nx, ny)) {
                    if (flag & NOTONL)
                        continue;
                    data->info[cnt] |= NOTONL;
                }
                /* check for diagonal tight squeeze */
                if (nx != x && ny != y && bad_rock(mon, x, ny)
                    && bad_rock(mon, nx, y) && cant_squeeze_thru(mon))
                    continue;
                /* The monster avoids a particular type of trap if it's
                 * familiar with the trap type.  Pets get ALLOW_TRAPS
                 * and checking is done in dogmove.c.  In either case,
                 * "harmless" traps are neither avoided nor marked in info[].
                 */
                if ((ttmp = t_at(nx, ny)) != 0) {
                    if (ttmp->ttyp >= TRAPNUM || ttmp->ttyp == 0) {
                        impossible("A monster looked at a very strange trap"
                                   " of type %d.",
                                   ttmp->ttyp);
                            continue;
                    }
                    /* fixed-destination teleport trap, was used by hero */
                    if (fixed_tele_trap(ttmp) && hastrack(nx, ny))
                        data->info[cnt] |= ALLOW_TRAPS;
                    else if (!m_harmless_trap(mon, ttmp)) {
                        if (!(flag & ALLOW_TRAPS)) {
                            if (mon_knows_traps(mon, ttmp->ttyp))
                                continue;
                        }
                        data->info[cnt] |= ALLOW_TRAPS;
                    }
                }
                data->poss[cnt].x = nx;
                data->poss[cnt].y = ny;
                cnt++;
            }
        }
    if (!cnt && wantpool && !is_pool(x, y)) {
        wantpool = FALSE;
        goto nexttry;
    }
    data->cnt = cnt;
    return cnt;
}

/* Part of mm_aggression that represents two-way aggression.  To avoid
   having to code each case twice, this function contains those cases that
   ought to happen twice, and mm_aggression will call it twice. */
staticfn long
mm_2way_aggression(struct monst *magr, struct monst *mdef)
{
    if (On_W_tower_level(&u.uz)) {
        /* treat inside the Wizard's tower as if it were a separate level
           from outside so when hero is inside Wizard's tower, both monsters
           need to be too; when outside, the monsters need to be too */
        if (In_W_tower(u.ux, u.uy, &u.uz)
            ? (!In_W_tower(magr->mx, magr->my, &u.uz)
               || !In_W_tower(mdef->mx, mdef->my, &u.uz))
            : (In_W_tower(magr->mx, magr->my, &u.uz)
               || In_W_tower(mdef->mx, mdef->my, &u.uz)))
            return 0L;
    }
    /* liches/zombies vs things that can be zombified

       Note: avoid this on the Castle level, partly for balance reasons
       (the monster-versus-monster fights clear out significant portions of
       the Castle and make it easier than it should be), partly for flavor
       reasons (monsters who attacked other monsters to zombify them would
       have been counterattacked to death long before the hero arrived).

       Also don't include unique monsters in this, otherwise it leads to
       them waking up early (e.g. because a zombie decided to attack the
       Wizard of Yendor). */
    if (zombie_maker(magr) && zombie_form(mdef->data) != NON_PM) {
        if (magr->mgenmklev && mdef->mgenmklev)
            return 0L;
        if (!Is_stronghold(&u.uz)
            && !unique_corpstat(magr->data) && !unique_corpstat(mdef->data))
            return (ALLOW_M | ALLOW_TM);
    }

    /* elvenkind vs orcs */
    if ((racial_elf(magr) || racial_drow(magr)) && racial_orc(mdef)
        && !(nonliving(magr->data) || nonliving(mdef->data)))
        return ALLOW_M | ALLOW_TM;

    /* elves vs drow */
    if (racial_elf(magr) && racial_drow(mdef)
        && !(nonliving(magr->data) || nonliving(mdef->data)))
        return ALLOW_M | ALLOW_TM;

    /* angels vs demons */
    if (((is_angel(magr->data) && magr->data != &mons[PM_ELDRITCH_KI_RIN])
         || racial_aasimar(magr))
        && is_demon(mdef->data))
        return ALLOW_M | ALLOW_TM;

    /* Nazgul vs hobbits */
    if (magr->data == &mons[PM_NAZGUL]
        && racial_hobbit(mdef) && !is_undead(mdef->data))
        return ALLOW_M | ALLOW_TM;

    return 0L;
}

/* Monster against monster special attacks; for the specified monster
   combinations, this allows one monster to attack another adjacent one
   in the absence of Conflict.  There is no provision for targeting
   other monsters; just hand to hand fighting when they happen to be
   next to each other. */
long
mm_aggression(
    struct monst *magr, /* monster that is currently deciding where to move */
    struct monst *mdef) /* another monster which is next to it */
{
    int mndx = monsndx(magr->data);

    /* don't allow pets to fight each other */
    if (magr->mtame && mdef->mtame)
        return 0L;

    /* supposedly purple worms are attracted to shrieking because they
       like to eat shriekers, so attack the latter when feasible */
    if ((mndx == PM_PURPLE_WORM || mndx == PM_BABY_PURPLE_WORM)
        && mdef->data == &mons[PM_SHRIEKER])
        return ALLOW_M | ALLOW_TM;

    /* mini-me recently had a bad encounter with a yellowjacket;
       she now hates all things that can sting */
    if ((magr->data == &mons[PM_KATHRYN_THE_ICE_QUEEN]
         || magr->data == &mons[PM_KATHRYN_THE_ENCHANTRESS])
        && can_sting(mdef->data))
        return ALLOW_M | ALLOW_TM;

    /* pets will attack Lucifer, regardless of their level */
    if (magr->mtame && mdef->data == &mons[PM_LUCIFER])
        return ALLOW_M | ALLOW_TM;

    /* Neothelids are an abomination
       to mind flayers */
    if (is_mind_flayer(magr->data)
        && mdef->data == &mons[PM_NEOTHELID])
        return ALLOW_M | ALLOW_TM;

    /* neothelids need brains to survive and
       will attack anything not mindless */
    if (magr->data == &mons[PM_NEOTHELID]
        && !mindless(mdef->data))
        return ALLOW_M | ALLOW_TM;

    /* Various other combinations such as dog vs cat, cat vs rat, and
       elf vs orc have been suggested.  For the time being we don't
       support those. */
    return (mm_2way_aggression(magr, mdef) | mm_2way_aggression(mdef, magr));
}

/* Monster displacing another monster out of the way */
staticfn long
mm_displacement(
    struct monst *magr, /* monster that is currently deciding where to move */
    struct monst *mdef) /* another monster which is next to it */
{
    struct permonst *pa = magr->data, *pd = mdef->data;

    /* if attacker can't barge through, there's nothing to do;
       or if defender can barge through too and has a level at least
       as high as the attacker, don't let attacker do so, otherwise
       they might just end up swapping places again when defender
       gets its chance to move */
    if (is_displacer(pa) && (!is_displacer(pd) || magr->m_lev > mdef->m_lev)
        /* no displacing grid bugs diagonally */
        && !(magr->mx != mdef->mx && magr->my != mdef->my
             && NODIAG(monsndx(pd)))
        /* no displacing trapped monsters or multi-location longworms */
        && !mdef->mtrapped && !mdef->mentangled
        && (!mdef->wormno || !count_wsegs(mdef))
        /* riders can move anything; others, same size or smaller only */
        && (is_rider(pa) || pa->msize >= pd->msize))
        return ALLOW_MDISP;
    return 0L;
}

/* Is the square close enough for the monster to move or attack into? */
boolean
monnear(struct monst *mon, coordxy x, coordxy y)
{
    int distance = dist2(mon->mx, mon->my, x, y);

    if (distance == 2 && NODIAG(mon->data - mons))
        return 0;
    return (boolean) (distance < 3);
}

/* recent m_detach tracking for debugging; the oversized buffers built
   from this ring get chopped to BUFSZ-1 in the paniclog, but survive
   whole in the impossible()/panic stack frame for core-dump readers */
#define MAX_DETACH_TRACK 10
static struct {
    char mname[64]; /* longest monster name is well under this */
    coordxy x, y;
    int hp, maxhp;
    void *caller;
    long turn;
    void *mon_addr;    /* monster memory address for reuse detection */
    unsigned m_id;     /* monster id to distinguish individuals */
    long mm_seq;       /* movemon() call counter at detach time */
} recent_detaches[MAX_DETACH_TRACK];
static int detach_idx = 0;

/* the hero has freed the Ice Queen from her curse; called from
   dmonsfree() when Kathryn the Ice Queen is found dead, before the
   freeing loop can dispose of her; she revives, transformed back into
   her original form of Kathryn the Enchantress, and calls off her
   dogs */
staticfn void
icequeenrevive(struct monst *mtmp)
{
    struct monst *mon;
    struct permonst *bourbon = &mons[PM_BOURBON];
    struct permonst *ozzy = &mons[PM_OZZY];

    /* our hero has freed the Ice Queen from her curse */
    if (mtmp->data != &mons[PM_KATHRYN_THE_ICE_QUEEN])
        return;

    /* in case the player kills themselves while defeating the Ice
       Queen and isn't life-saved */
    if (u.uhp <= 0 && !Lifesaved) {
        ; /* suppress feedback */
    } else {
        Your("actions have released %s from a powerful curse!",
             mon_nam(mtmp));
        if (canspotmon(mtmp))
            You("watch as %s undergoes a transformation, back into her original form.",
                mon_nam(mtmp));
    }

    /* should be redundant (set in mondead), but in case of funky
       business */
    if (!u.uevent.defeat_icequeen)
        u.uevent.defeat_icequeen = 1;

    if (svm.mvitals[PM_KATHRYN_THE_ICE_QUEEN].died < 255)
        svm.mvitals[PM_KATHRYN_THE_ICE_QUEEN].died++;

    mtmp->mcanmove = 1;
    mtmp->mfrozen = 0;
    mtmp->mstone = 0;
    mtmp->msick = 0;
    mtmp->mdiseased = 0;
    mtmp->mwither = 0;
    mtmp->mconf = 0;
    mtmp->mstun = 0;
    mtmp->minvis = 0;

    /* make her healthy for the transformation; newcham() rescales hit
       points for the new form, so the real values are set afterwards */
    mtmp->mhp = mtmp->mhpmax = max(mtmp->mhpmax, 1);
    (void) newcham(mtmp, &mons[PM_KATHRYN_THE_ENCHANTRESS], NO_NC_FLAGS);
    /* alignment derives from her restored form, so this must follow
       the transformation */
    mtmp->mpeaceful = 1;
    set_malign(mtmp);
    if (kathryn_bday())
        mtmp->mhp = mtmp->mhpmax = 15000;
    else
        mtmp->mhp = mtmp->mhpmax = 7500;
    if (mtmp == u.ustuck) {
        if (u.uswallow)
            expels(mtmp, mtmp->data, FALSE);
        else
            uunstick();
    }
    newsym(mtmp->mx, mtmp->my);

    /* fix up Bourbon and Ozzy */
    for (mon = fmon; mon; mon = mon->nmon) {
        if (DEADMONSTER(mon))
            continue;
        /* cure any ailments the dogs may have also */
        if (mon->data == bourbon || mon->data == ozzy) {
            if (u.uhp <= 0 && !Lifesaved) {
                ; /* suppress feedback */
            } else {
                if (mon->data == bourbon && !mon->mpeaceful) {
                    if (m_cansee(mtmp, mon->mx, mon->my))
                        pline("%s motions for Bourbon to heel and stop %s attack.",
                              Monnam(mtmp), mhis(mon));
                } else if (mon->data == ozzy && !mon->mpeaceful) {
                    if (m_cansee(mtmp, mon->mx, mon->my))
                        pline("%s motions for Ozzy to heel and stop %s attack.",
                              Monnam(mtmp), mhis(mon));
                }
            }
            mon->mcanmove = 1;
            mon->mfrozen = 0;
            mon->mstone = 0;
            mon->msick = 0;
            mon->mdiseased = 0;
            mon->mwither = 0;
            mon->mconf = 0;
            mon->mstun = 0;
            mon->mpeaceful = 1;
            set_malign(mon);
            newsym(mon->mx, mon->my); /* enable peaceful glyph */
        }
    }
    /* in case the player kills themselves while defeating the Ice
       Queen and isn't life-saved */
    if (u.uhp <= 0 && !Lifesaved) {
        ; /* suppress feedback */
    } else {
        if (Role_if(PM_INFIDEL)) {
            /* the Enchantress will not tolerate those that serve
               Moloch; our Infidel has 100 turns to do what they're
               going to do and get out before the situation becomes
               dire */
            com_pager("icequeen_freed_infidel");
            mtmp->mpeaceful = 0;
            set_malign(mtmp);
            newsym(mtmp->mx, mtmp->my); /* clear peaceful glyph */
            paralyze_monst(mtmp, 100);
        } else if ((!Upolyd && Race_if(PM_DRAUGR))
                   || (!Upolyd && Race_if(PM_VAMPIRE))
                   || is_undead(gy.youmonst.data)) {
            /* as with Infidels, the Enchantress will not tolerate
               the undead */
            com_pager("icequeen_freed_undead");
            mtmp->mpeaceful = 0;
            set_malign(mtmp);
            newsym(mtmp->mx, mtmp->my); /* clear peaceful glyph */
            paralyze_monst(mtmp, 100);
        } else {
            com_pager("icequeen_freed");
        }
    }
    if (u.ualign.type == A_NONE) {
        You_feel("guilty.");
        adjalign(-2); /* doing good things as an agent of Moloch? pfft */
        record_abuse_event(-2, ABUSE_INFIDEL_GOOD);
    } else {
        adjalign(2);
    }
    change_luck(2);
    return;
}

/* really free dead monsters */
void
dmonsfree(void)
{
    struct monst **mtmp, *freetmp, *ridertmp;
    int count = 0;
    int undetached = 0;
    unsigned undetached_id = 0;
    char buf[QBUFSZ];
    char freed_list[BUFSZ] = "";
    char undetached_buf[BUFSZ] = "";

    buf[0] = '\0';
    /* diagnostic: a live steed left in fmon but off the map - its rider
       gone and ridden_by cleared, yet never re-placed - turns invisible
       while still attacking (the phantom saddled-spider bug). report it
       so a recurrence leaves a record; this only observes the state, it
       does not repair it */
    {
        static unsigned orphan_warned_id = 0;
        struct monst *o;
        boolean any = FALSE;

        for (o = fmon; o; o = o->nmon) {
            /* any non-MON_FLOOR lifecycle state is legitimately not
               placed; the target is a monster whose mstate claims it
               is an ordinary on-map resident */
            if (DEADMONSTER(o) || o == u.usteed || o->ridden_by
                || mon_offmap(o)
                || !isok(o->mx, o->my)
                || svl.level.monsters[o->mx][o->my] == o)
                continue;
            any = TRUE;
            if (o->m_id != orphan_warned_id) {
                orphan_warned_id = o->m_id;
                impossible(
                    "off-map steed %s (m_id %u) at <%d,%d> mstate 0x%lx",
                    o->data ? mon_pmname(o) : "?", o->m_id,
                    o->mx, o->my, o->mstate);
            }
        }
        if (!any)
            orphan_warned_id = 0;
    }

    for (mtmp = &fmon; *mtmp; ) {
        freetmp = *mtmp;

        if (DEADMONSTER(freetmp)
            && freetmp->data == &mons[PM_KATHRYN_THE_ICE_QUEEN]) {
            icequeenrevive(freetmp);
            /* the Ice Queen transforms to the Enchantress and is
               revived; only decrement the purge counter if she was
               actually marked for purging */
            if (freetmp->mstate & MON_DETACH) {
                iflags.purge_monsters--;
                freetmp->mstate &= ~MON_DETACH; /* clear after revival */
            }
            /* track this special case in recent_detaches with a
               negative position */
            Strcpy(recent_detaches[detach_idx].mname,
                   "IceQueen->Enchantress");
            recent_detaches[detach_idx].x = -1;
            recent_detaches[detach_idx].y = -1;
            recent_detaches[detach_idx].hp = 0;
            recent_detaches[detach_idx].maxhp = 0;
#ifdef __GNUC__
            recent_detaches[detach_idx].caller = __builtin_return_address(0);
#else
            recent_detaches[detach_idx].caller = NULL;
#endif
            recent_detaches[detach_idx].turn = svm.moves;
            recent_detaches[detach_idx].mon_addr = (void *) freetmp;
            recent_detaches[detach_idx].m_id = freetmp->m_id;
            recent_detaches[detach_idx].mm_seq = movemon_callctr;
            detach_idx = (detach_idx + 1) % MAX_DETACH_TRACK;
        }

        if (DEADMONSTER(freetmp) && !freetmp->isgd) {
            *mtmp = freetmp->nmon;
            freetmp->nmon = NULL;

            /* track what is actually freed for debugging */
            if (count < 5 && freetmp->data)
                Sprintf(eos(freed_list), " %s(%d,%d)",
                        mon_pmname(freetmp), freetmp->mx, freetmp->my);
            /* a freed monster that never went through m_detach()
               (MON_DETACH unset) reached mhp<1 outside the death
               pipeline - the count>purge producer. recent_detaches[]
               cannot see it (it tracks m_detach calls only), so record
               it here, before the struct is freed */
            if (!(freetmp->mstate & MON_DETACH)) {
                undetached++;
                if (undetached == 1) {
                    undetached_id = freetmp->m_id;
                    Sprintf(undetached_buf,
                            "%s(%d,%d) m_id=%u mstate=0x%lx mhp=%d",
                            freetmp->data ? mon_pmname(freetmp) : "?",
                            freetmp->mx, freetmp->my, freetmp->m_id,
                            freetmp->mstate, freetmp->mhp);
                }
            }
            /* a steed being freed must not leave its rider holding a
               dangling mon_steed pointer */
            if ((ridertmp = get_mon_rider(freetmp)) != 0)
                separate_steed_and_rider(ridertmp);
            dealloc_monst(freetmp);
            count++;
        } else
            mtmp = &(freetmp->nmon);
    }

    if (count != iflags.purge_monsters) {
        char msgbuf[BUFSZ * 6];
        int living_detached = 0, dead_not_freed = 0,
            dead_detached_not_freed = 0;
        int total_mons = 0;
        struct monst *mon;
        char not_freed_list[BUFSZ] = "";

        describe_level(buf, 2);

        /* add game state context */
        if (program_state.saving)
            Strcat(buf, " [SAVING]");
        if (program_state.restoring)
            Strcat(buf, " [RESTORING]");
        if (program_state.in_moveloop)
            Strcat(buf, " [MOVELOOP]");
        if (wizard && program_state.wizkit_wishing)
            Strcat(buf, " [WIZKIT_WISHING]");

        /* count problematic monsters */
        for (mon = fmon; mon; mon = mon->nmon) {
            total_mons++;
            if (!DEADMONSTER(mon) && (mon->mstate & MON_DETACH))
                living_detached++;
            if (DEADMONSTER(mon)) {
                /* dead monsters that won't be freed */
                if (mon->isgd) {
                    dead_not_freed++;
                } else if (mon->mstate & MON_DETACH) {
                    /* bad - dead with MON_DETACH but still on the list */
                    dead_detached_not_freed++;
                    if (dead_detached_not_freed <= 3) {
                        Sprintf(eos(not_freed_list), " %s(%d,%d)",
                                mon->data ? mon_pmname(mon) : "unknown",
                                mon->mx, mon->my);
                    }
                }
            }
        }

        /* build detailed error message */
        Sprintf(msgbuf, "dmonsfree: %d removed != %d pending on %s [%dmons]",
                count, iflags.purge_monsters, buf, total_mons);
        /* show what was actually freed */
        if (freed_list[0])
            Sprintf(eos(msgbuf), " Freed:%s", freed_list);
        /* name any monster freed without ever being m_detach()ed - the
           freed-not-detached (count>purge) producer that
           recent_detaches[] cannot capture */
        if (undetached > 0)
            Sprintf(eos(msgbuf), " Undetached:%d %s", undetached,
                    undetached_buf);

        /* add diagnostic counts */
        if (living_detached > 0)
            Sprintf(eos(msgbuf), " [%d living with MON_DETACH!]",
                    living_detached);
        if (dead_not_freed > 0)
            Sprintf(eos(msgbuf), " [%d dead vault guards]", dead_not_freed);
        if (dead_detached_not_freed > 0)
            Sprintf(eos(msgbuf), " [%d DEAD+DETACHED NOT FREED!%s]",
                    dead_detached_not_freed, not_freed_list);

        /* log first few problematic monsters */
        if (living_detached > 0) {
            int shown = 0;

            Strcat(msgbuf, " Living+detached:");
            for (mon = fmon; mon && shown < 3; mon = mon->nmon) {
                if (!DEADMONSTER(mon) && (mon->mstate & MON_DETACH)) {
                    Sprintf(eos(msgbuf), " %s(%d,%d)",
                            mon_pmname(mon), mon->mx, mon->my);
                    shown++;
                }
            }
        }

        /* show recent m_detach calls with m_id and turn */
        if (recent_detaches[0].mname[0]) {
            int start = (detach_idx - 1 + MAX_DETACH_TRACK)
                        % MAX_DETACH_TRACK;
            int shown = 0, i, idx;

            Strcat(msgbuf, " Recent:");
            for (i = 0; i < MAX_DETACH_TRACK && shown < 5; i++) {
                idx = (start - i + MAX_DETACH_TRACK) % MAX_DETACH_TRACK;
                if (recent_detaches[idx].mname[0]) {
                    Sprintf(eos(msgbuf),
                            " %s(%d,%d)#%u@%ld mm#%ld by %p addr=%p",
                            recent_detaches[idx].mname,
                            recent_detaches[idx].x,
                            recent_detaches[idx].y,
                            recent_detaches[idx].m_id,
                            recent_detaches[idx].turn,
                            recent_detaches[idx].mm_seq,
                            recent_detaches[idx].caller,
                            recent_detaches[idx].mon_addr);
                    shown++;
                }
            }
        }

        /* search other monster lists for the most recently
           detached monster to diagnose where it ended up */
        {
            int ridx = (detach_idx - 1 + MAX_DETACH_TRACK)
                       % MAX_DETACH_TRACK;
            unsigned target_id = recent_detaches[ridx].m_id;
            struct monst *found = (struct monst *) 0;
            const char *where = (const char *) 0;

            if (target_id) {
                for (found = gm.migrating_mons; found; found = found->nmon) {
                    if (found->m_id == target_id) {
                        where = "migrating";
                        break;
                    }
                }
                if (!found) {
                    for (found = gm.mydogs; found; found = found->nmon) {
                        if (found->m_id == target_id) {
                            where = "mydogs";
                            break;
                        }
                    }
                }
                if (!found) {
                    for (found = fmon; found; found = found->nmon) {
                        if (found->m_id == target_id) {
                            where = "fmon(missed)";
                            break;
                        }
                    }
                }
            }
            if (found)
                Sprintf(eos(msgbuf),
                        " [mid#%u FOUND on %s (%d,%d) hp=%d detached by %p"
                        " mm#%ld addr=%p]",
                        target_id, where, found->mx,
                        found->my, found->mhp,
                        recent_detaches[ridx].caller,
                        recent_detaches[ridx].mm_seq,
                        recent_detaches[ridx].mon_addr);
            else if (target_id)
                Sprintf(eos(msgbuf),
                        " [mid#%u NOT FOUND any list, detached by %p"
                        " mm#%ld addr=%p]",
                        target_id, recent_detaches[ridx].caller,
                        recent_detaches[ridx].mm_seq,
                        recent_detaches[ridx].mon_addr);
        }

        impossible("%s", msgbuf);
    }

    /* even when the counts happen to balance (a freed-not-detached
       monster offset by a detached-not-freed one in the same pass), a
       free without MON_DETACH is still anomalous; report it once per
       distinct monster. buf is empty here unless the mismatch block
       above ran, so fill it for context */
    if (undetached > 0 && count == iflags.purge_monsters) {
        static unsigned undetached_warned_id = 0;

        if (undetached_id != undetached_warned_id) {
            undetached_warned_id = undetached_id;
            if (!buf[0])
                describe_level(buf, 2);
            impossible("dmonsfree: freed undetached %s on %s",
                       undetached_buf, buf);
        }
    }
    iflags.purge_monsters = 0;
}

/* called when monster is moved to larger structure */
void
replmon(struct monst *mtmp, struct monst *mtmp2)
{
    struct obj *otmp;

    /* transfer the monster's inventory */
    for (otmp = mtmp2->minvent; otmp; otmp = otmp->nobj) {
        if (otmp->where != OBJ_MINVENT || otmp->ocarry != mtmp)
            impossible("replmon: minvent inconsistency");
        otmp->ocarry = mtmp2;
    }
    mtmp->minvent = 0;
    /* before relmon(mtmp), because it could clear polearm.hitmon */
    if (svc.context.polearm.hitmon == mtmp)
        svc.context.polearm.hitmon = mtmp2;

    /* remove the old monster from the map and from `fmon' list */
    relmon(mtmp, (struct monst **) 0);

    /* finish adding its replacement */
    /* don't place a steed onto the map */
    if (mtmp != u.usteed && !mtmp->ridden_by)
        place_monster(mtmp2, mtmp2->mx, mtmp2->my);
    if (mtmp2->wormno)      /* update level.monsters[wseg->wx][wseg->wy] */
        place_wsegs(mtmp2, mtmp); /* locations to mtmp2 not mtmp. */
    if (emits_light(mtmp2->data)) {
        /* since this is so rare, we don't have any `mon_move_light_source' */
        new_light_source(mtmp2->mx, mtmp2->my, emits_light(mtmp2->data),
                         LS_MONSTER, monst_to_any(mtmp2));
        /* here we rely on fact that `mtmp' hasn't actually been deleted */
        del_light_source(LS_MONSTER, monst_to_any(mtmp));
    }
    mtmp2->nmon = fmon;
    fmon = mtmp2;
    if (u.ustuck == mtmp)
        set_ustuck(mtmp2);
    if (u.usteed == mtmp)
        u.usteed = mtmp2;
    if (mtmp2->isshk)
        replshk(mtmp, mtmp2);

    /* discard the old monster */
    dealloc_monst(mtmp);
}

/* release mon from the display and the map's monster list,
   maybe transfer it to one of the other monster lists */
void
relmon(
    struct monst *mon,
    struct monst **monst_list) /* &gm.migrating_mons or &gm.mydogs or null */
{
    if (!fmon)
        panic("relmon: no fmon available.");

    /* take 'mon' off the map */
    mon_leaving_level(mon);

    /* remove 'mon' from the 'fmon' list */
    if (mon == fmon) {
        fmon = fmon->nmon;
    } else {
        struct monst *mtmp;

        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
            if (mtmp->nmon == mon) {
                mtmp->nmon = mon->nmon;
                break;
            }
        if (!mtmp)
            panic("relmon: mon not in list.");
    }

    if (monst_list) {
        /* insert into gm.mydogs or gm.migrating_mons */
        mon->nmon = *monst_list;
        *monst_list = mon;
    } else {
        /* orphan has no next monster */
        mon->nmon = 0;
    }
}

void
copy_mextra(struct monst *mtmp2, struct monst *mtmp1)
{
    if (!mtmp2 || !mtmp1 || !mtmp1->mextra)
        return;

    if (!mtmp2->mextra)
        mtmp2->mextra = newmextra();
    if (MGIVENNAME(mtmp1)) {
        new_mgivenname(mtmp2, (int) strlen(MGIVENNAME(mtmp1)) + 1);
        Strcpy(MGIVENNAME(mtmp2), MGIVENNAME(mtmp1));
    }
    if (EGD(mtmp1)) {
        if (!EGD(mtmp2))
            newegd(mtmp2);
        assert(has_egd(mtmp2));
        *EGD(mtmp2) = *EGD(mtmp1);
    }
    if (EPRI(mtmp1)) {
        if (!EPRI(mtmp2))
            newepri(mtmp2);
        assert(has_epri(mtmp2));
        *EPRI(mtmp2) = *EPRI(mtmp1);
    }
    if (ESHK(mtmp1)) {
        if (!ESHK(mtmp2))
            neweshk(mtmp2);
        assert(has_eshk(mtmp2));
        *ESHK(mtmp2) = *ESHK(mtmp1);
    }
    if (EMIN(mtmp1)) {
        if (!EMIN(mtmp2))
            newemin(mtmp2);
        assert(has_emin(mtmp2));
        *EMIN(mtmp2) = *EMIN(mtmp1);
    }
    if (EDOG(mtmp1)) {
        if (!EDOG(mtmp2))
            newedog(mtmp2);
        assert(has_edog(mtmp2));
        *EDOG(mtmp2) = *EDOG(mtmp1);
    }
    if (EBONES(mtmp1)) {
        if (!EBONES(mtmp2))
            newebones(mtmp2);
        assert(has_ebones(mtmp2));
        *EBONES(mtmp2) = *EBONES(mtmp1);
    }
    if (ERID(mtmp1)) {
        if (!ERID(mtmp2))
            newerid(mtmp2);
        assert(has_erid(mtmp2));
        *ERID(mtmp2) = *ERID(mtmp1);
    }
    if (ERAC(mtmp1)) {
        if (!ERAC(mtmp2))
            newerac(mtmp2);
        assert(has_erac(mtmp2));
        *ERAC(mtmp2) = *ERAC(mtmp1);
    }
    if (EMSP(mtmp1)) {
        if (!EMSP(mtmp2))
            newemsp(mtmp2);
        assert(has_emsp(mtmp2));
        *EMSP(mtmp2) = *EMSP(mtmp1);
    }
    if (has_mcorpsenm(mtmp1))
        MCORPSENM(mtmp2) = MCORPSENM(mtmp1);
}

void
dealloc_mextra(struct monst *m)
{
    struct mextra *x = m->mextra;

    if (x) {
        if (x->mgivenname)
            free((genericptr_t) x->mgivenname), x->mgivenname = 0;
        if (x->egd)
            free((genericptr_t) x->egd), x->egd = 0;
        if (x->epri)
            free((genericptr_t) x->epri), x->epri = 0;
        if (x->eshk)
            free((genericptr_t) x->eshk), x->eshk = 0;
        if (x->emin)
            free((genericptr_t) x->emin), x->emin = 0;
        if (x->edog)
            free((genericptr_t) x->edog), x->edog = 0;
        if (x->erid)
            free((genericptr_t) x->erid), x->erid = 0;
        if (x->erac)
            free((genericptr_t) x->erac), x->erac = 0;
        if (x->emsp)
            free((genericptr_t) x->emsp), x->emsp = 0;
        if (x->ebones)
            free((genericptr_t) x->ebones), x->ebones = 0;
        x->mcorpsenm = NON_PM; /* no allocation to release */

        free((genericptr_t) x);
        m->mextra = (struct mextra *) 0;
    }
}

void
dealloc_monst(struct monst *mon)
{
    char buf[QBUFSZ];

    buf[0] = '\0';
    if (mon->nmon) {
        describe_level(buf, 2);
        panic("dealloc_monst with nmon on %s", buf);
    }
    if (mon->mextra)
        dealloc_mextra(mon);
    /* clear out of date information contained in the about-to-become
       stale memory; see dealloc_obj() */
    *mon = cg.zeromonst;
    free((genericptr_t) mon);
}

/* 'mon' is being removed from level due to migration [relmon from keepdogs
   or migrate_to_level] or due to death [m_detach from mondead or mongone] */
staticfn void
mon_leaving_level(struct monst *mon)
{
    coordxy mx = mon->mx, my = mon->my;
    boolean onmap = (isok(mx, my) && svl.level.monsters[mx][my] == mon);

    /* to prevent an infinite relobj-flooreffects-hmon-killed loop */
    mon->mtrapped = 0;
    mon->mentangled = 0;
    unstuck(mon); /* mon is not swallowing or holding you nor held by you */

    /* vault guard might be at <0,0> */
    if (onmap || mon == svl.level.monsters[0][0]) {
        if (mon->wormno)
            remove_worm(mon);
        else
            remove_monster(mx, my);

#if 0   /* mustn't do this; too many places assume that the stale
         * monst->mx,my values are still valid */
        mon->mx = mon->my = 0; /* off normal map */
#endif
    }
    if (onmap) {
        mon->mundetected = 0; /* for migration; doesn't matter for death */
        /* unhide mimic in case its shape has been blocking line of sight
           or it is accompanying the hero to another level */
        if (M_AP_TYPE(mon) != M_AP_NOTHING && M_AP_TYPE(mon) != M_AP_MONSTER)
            seemimic(mon);
        /* if mon is pinned by a boulder, removing mon lets boulder drop */
        fill_pit(mx, my);
        newsym(mx, my);
    }
    /* if mon is a remembered target, forget it since it isn't here anymore */
    if (mon == svc.context.polearm.hitmon)
        svc.context.polearm.hitmon = (struct monst *) 0;
}

/* 'mtmp' is going away; remove effects of mtmp from other data structures */
staticfn void
m_detach(
    struct monst *mtmp,
    struct permonst *mptr, /* reflects mtmp->data _prior_ to mtmp's death */
    boolean due_to_death)
{
    coordxy mx = mtmp->mx, my = mtmp->my;

    /* if mtmp is the pending source of a beam, thrown object, or
       explosion, forget it since mtmp is going away */
    if (mtmp == gb.buzzer)
        gb.buzzer = 0;
    if (mtmp == gt.thrower)
        gt.thrower = 0;
    if (mtmp == ge.exploder)
        ge.exploder = 0;

    if (mtmp->mleashed)
        m_unleash(mtmp, FALSE);

    if (mx > 0 && emits_light(mptr))
        del_light_source(LS_MONSTER, monst_to_any(mtmp));

    /*
     * Take mtmp off map but not out of fmon list yet (dmonsfree does that).
     *
     * Sequencing issue:  mtmp's inventory should be dropped before taking
     * it off the map but if that includes a boulder and mtmp is at a pit
     * location, dropping minvent ought to be deferred until its corpse
     * gets placed.  We compromise and just make sure mtmp is off the map
     * before dropping its former belongings.
     */
    mon_leaving_level(mtmp);

    /* a departing rider drops its steed back onto the map; a departing
       steed leaves its rider standing where the pair used to be */
    if (has_erid(mtmp))
        separate_steed_and_rider(mtmp);
    if (mtmp->ridden_by) {
        struct monst *rider = get_mon_rider(mtmp);

        if (rider)
            free_erid(rider);
        else
            mtmp->ridden_by = 0; /* rider already gone; clear stale flag */
        newsym(mtmp->mx, mtmp->my);
    }

    mtmp->mhp = 0; /* simplify some tests: force mhp to 0 */
    /* death handling for the Wizard needs to take place even if he is
       leaving the dungeon alive rather than dying */
    if (mtmp->iswiz)
        wizdeadorgone();
    /* foodead() might give quest feedback for foo having died; skip that
       if we're called for mongone() rather than mondead(); saving bones
       or wizard mode genocide of "*" can result in special monsters going
       away without having been killed */
    if (due_to_death) {
        if (mtmp->data->msound == MS_NEMESIS) {
            nemdead();
            /* The Archeologist, Caveman, and Priest quest texts describe
               the nemesis's body creating noxious fumes/gas when killed. */
            if (stinky_nemesis(mtmp))
                nemesis_stinks(mx, my);
        }
        if (mtmp->data->msound == MS_LEADER)
            leaddead();
        /* release (drop onto map) all objects carried by mtmp; assumes that
           mtmp->mx,my contains the appropriate location */
        relobj(mtmp, 1, FALSE); /* drop mtmp->minvent, issue newsym(mx,my) */
    }

    if (mtmp->m_id == gs.stealmid)
        thiefdead(); /* reset theft-in-progress data */
    if (mtmp->isshk)
        shkgone(mtmp);
    if (mtmp->wormno)
        wormgone(mtmp);
    if (In_endgame(&u.uz))
        mtmp->mstate |= MON_ENDGAME_FREE;

    if ((mtmp->mstate & MON_DETACH) != 0) {
        /* double detach - the root cause of many dmonsfree miscounts;
           dump the recent m_detach history so the producer is named */
        int i;
        char msgbuf[BUFSZ * 12]; /* header + full detach history */
        char tmpbuf[BUFSZ];

        Sprintf(msgbuf, "m_detach: %s is already detached?\n",
                minimal_monnam(mtmp, FALSE));
        Sprintf(eos(msgbuf), "=== RECENT M_DETACH HISTORY ===\n");
        Sprintf(eos(msgbuf), "Double m_detach detected at turn %ld\n",
                svm.moves);
        Sprintf(eos(msgbuf), "Monster: %s at (%d,%d) HP:%d/%d\n",
                mtmp->data ? mon_pmname(mtmp) : "?", mx, my,
                mtmp->mhp, mtmp->mhpmax);

        for (i = 0; i < MAX_DETACH_TRACK; i++) {
            int idx = (detach_idx + i) % MAX_DETACH_TRACK;

            if (recent_detaches[idx].mname[0]) {
                Sprintf(tmpbuf,
                        "[%d] %s at (%d,%d) HP:%d/%d addr:%p id:%u by %p"
                        " turn:%ld mm:%ld",
                        i + 1, recent_detaches[idx].mname,
                        recent_detaches[idx].x, recent_detaches[idx].y,
                        recent_detaches[idx].hp, recent_detaches[idx].maxhp,
                        recent_detaches[idx].mon_addr,
                        recent_detaches[idx].m_id,
                        recent_detaches[idx].caller,
                        recent_detaches[idx].turn,
                        recent_detaches[idx].mm_seq);

                /* flag likely-related history entries */
                if (recent_detaches[idx].m_id == mtmp->m_id) {
                    Strcat(tmpbuf, " <-- SAME MONSTER ID!");
                } else if (recent_detaches[idx].mon_addr == (void *) mtmp) {
                    Strcat(tmpbuf, " <-- SAME ADDRESS (REUSED)!");
                } else if (mtmp->data
                           && !strcmp(recent_detaches[idx].mname,
                                      mon_pmname(mtmp))
                           && abs(recent_detaches[idx].x - mx) <= 1
                           && abs(recent_detaches[idx].y - my) <= 1) {
                    Strcat(tmpbuf, " <-- SIMILAR POSITION!");
                }
                Sprintf(eos(msgbuf), "%s\n", tmpbuf);
            }
        }
        Strcat(msgbuf, "=== END M_DETACH HISTORY ===");

        /* output everything in one impossible() call */
        impossible("%s", msgbuf);
    } else {
        mtmp->mstate |= MON_DETACH;
        iflags.purge_monsters++;

        /* track recent detaches for debugging */
        copynchars(recent_detaches[detach_idx].mname,
                   mtmp->data ? mon_pmname(mtmp) : "?",
                   (int) sizeof recent_detaches[0].mname - 1);
        recent_detaches[detach_idx].x = mx;
        recent_detaches[detach_idx].y = my;
        recent_detaches[detach_idx].hp = mtmp->mhp;
        recent_detaches[detach_idx].maxhp = mtmp->mhpmax;
#ifdef __GNUC__
        recent_detaches[detach_idx].caller = __builtin_return_address(0);
#else
        recent_detaches[detach_idx].caller = NULL; /* needs compiler support */
#endif
        recent_detaches[detach_idx].turn = svm.moves;
        recent_detaches[detach_idx].mon_addr = (void *) mtmp;
        recent_detaches[detach_idx].m_id = mtmp->m_id;
        recent_detaches[detach_idx].mm_seq = movemon_callctr;

        detach_idx = (detach_idx + 1) % MAX_DETACH_TRACK;
    }

    /* hero is thrown from his steed when it dies or gets genocided */
    if (mtmp == u.usteed)
        dismount_steed(DISMOUNT_GENERIC);
    return;
}

/* give a life-saved monster a reasonable mhpmax value in case it has
   been the victim of excessive life draining */
staticfn void
set_mon_min_mhpmax(
    struct monst *mon,
    int minimum_mhpmax) /* monster life-saving has traditionally used 10 */
{
    /* can't be less than m_lev+1 (if we just used m_lev itself, level 0
       monsters would end up allowing a minimum of 0); since life draining
       reduces m_lev, this usually won't give the monster much of a boost */
    if (mon->mhpmax < (int) mon->m_lev + 1)
        mon->mhpmax = (int) mon->m_lev + 1;
    /* caller can specify an alternate minimum; we'll honor it iff it is
       greater than m_lev+1; the traditional arbitrary value of 10 always
       gives level 0 and level 1 monsters a boost and has a moderate
       chance of doing so for level 2, a tiny chance for levels 3..9 */
    if (mon->mhpmax < minimum_mhpmax)
        mon->mhpmax = minimum_mhpmax;
}

/* find the worn amulet of life saving which will save a monster */
struct obj *
mlifesaver(struct monst *mon)
{
    if (!nonliving(mon->data) || is_vampshifter(mon)) {
        struct obj *otmp = which_armor(mon, W_AMUL);

        if (otmp && otmp->otyp == AMULET_OF_LIFE_SAVING)
            return otmp;
    }
    return (struct obj *) 0;
}

staticfn void
lifesaved_monster(struct monst *mtmp)
{
    boolean surviver;
    struct obj *lifesave = mlifesaver(mtmp);

    if (lifesave) {
        /* not canseemon; amulets are on the head, so you don't want
         * to show this for a long worm with only a tail visible.
         * Nor do you check invisibility, because glowing and
         * disintegrating amulets are always visible. */
        if (cansee(mtmp->mx, mtmp->my)) {
            pline("But wait...");
            pline("%s medallion begins to glow!", s_suffix(Monnam(mtmp)));
            makeknown(AMULET_OF_LIFE_SAVING);
            /* amulet is visible, but monster might not be */
            if (canseemon(mtmp)) {
                if (attacktype(mtmp->data, AT_EXPL)
                    || attacktype(mtmp->data, AT_BOOM))
                    pline("%s reconstitutes!", Monnam(mtmp));
                else
                    pline("%s looks much better!", Monnam(mtmp));
            }
            pline_The("medallion crumbles to dust!");
        }
        m_useup(mtmp, lifesave);
        /* equip replacement amulet, if any, on next move */
        check_gear_next_turn(mtmp);

        surviver = !(svm.mvitals[monsndx(mtmp->data)].mvflags & G_GENOD);
        mtmp->mcanmove = 1;
        mtmp->mfrozen = 0;
        if (mtmp->mtame && !mtmp->isminion) {
            wary_dog(mtmp, !surviver);
        }
        set_mon_min_mhpmax(mtmp, 10); /* mtmp->mhpmax=max(mtmp->m_lev+1,10) */
        mtmp->mhp = mtmp->mhpmax;

        if (!surviver) {
            /* genocided monster can't be life-saved */
            if (cansee(mtmp->mx, mtmp->my))
                pline("Unfortunately, %s is still genocided...",
                      mon_nam(mtmp));
            mtmp->mhp = 0;
        }
    }
}

/* when a shape-shifted vampire is killed, it reverts to base form instead
   of dying; returns True if mtmp successfully revives, False otherwise;
   "successfully revived" vampire might be killed by a booby trapped door */
staticfn boolean
vamprises(struct monst *mtmp)
{
    int mndx = mtmp->cham;

    /*
     * Protection from shape changers protects against this because
     * the vampire will always be in normal form instead of shifted.
     * So there's no need to check for that attribute being active.
     */
    if (ismnum(mndx) && mndx != monsndx(mtmp->data)
        && !(svm.mvitals[mndx].mvflags & G_GENOD)) {
        char action[BUFSZ];
        /* alternate message phrasing for some monster types */
        boolean spec_mon = (nonliving(mtmp->data)
                            || noncorporeal(mtmp->data)
                            || amorphous(mtmp->data)),
                spec_death = (gd.disintegested /* disintegrated/digested */
                              || noncorporeal(mtmp->data)
                              || amorphous(mtmp->data));
        coordxy x = mtmp->mx, y = mtmp->my;

        /* construct a 'before' argument to pass to pline(); this used
           to construct a dynamic format string but that's overkill */
        Snprintf(action, sizeof action, "%s%s %s%s and rises as",
                 Unaware ? "you dream that " : "",
                 x_monnam(mtmp, ARTICLE_THE,
                          spec_mon ? (char *) 0 : "seemingly dead",
                          (SUPPRESS_INVISIBLE | AUGMENT_IT), FALSE),
                 Unaware ? "" : "suddenly ",
                 spec_death ? "reconstitutes" : "transforms");
        mtmp->mcanmove = 1;
        mtmp->mfrozen = 0;
        set_mon_min_mhpmax(mtmp, 10); /* mtmp->mhpmax=max(m_lev+1,10) */
        mtmp->mhp = mtmp->mhpmax;
        /* mtmp==u.ustuck can happen if previously a fog cloud or if
           poly'd hero is hugging a vampire bat */
        if (mtmp == u.ustuck) {
            if (u.uswallow)
                expels(mtmp, mtmp->data, FALSE);
            else
                uunstick();
        }

        if (!newcham(mtmp, &mons[mndx], NO_NC_FLAGS))
            return !DEADMONSTER(mtmp);
        mtmp->cham = (mtmp->data == &mons[mndx]) ? NON_PM : mndx;

        if (canspotmon(mtmp)) {
            /* 3.6.0 used a_monnam(mtmp); that was weird if mtmp was
               named: "Dracula suddenly transforms and rises as Dracula";
               3.6.1 used mtmp->data->mname; that ignored hallucination */
            pline_mon(mtmp, "%s %s!", upstart(action),
                      x_monnam(mtmp, ARTICLE_A, (char *) 0,
                           (SUPPRESS_NAME | SUPPRESS_IT | SUPPRESS_INVISIBLE),
                               FALSE));
            gv.vamp_rise_msg = TRUE;
        }
        /* revived vampire is in normal shape, so can't be amorphous; if on
           a closed door spot, destroy the door and if trapped, blow it up */
        if (closed_door(x, y)) {
            static const char door_smashed[] = "a door being smashed";
            struct rm *door = &levl[x][y];
            boolean trapped = (door->doormask & D_TRAPPED) != 0,
                    seeit = cansee(x, y);

            set_msg_xy(x, y); /* You()/pline() will reset this */
            /* the trap's own flavor comes from mb_trapped() below, which
               knows which of the booby-trap varieties this door has */
            if (!seeit)
                You_hear("%s.", door_smashed);
            else if (!canspotmon(mtmp))
                You_see("%s.", door_smashed);
            else if (!Unaware)
                pline_The("door is smashed.");
            set_msg_xy(0, 0); /* in case none of the messages was delivered */

            door->doormask = D_NODOOR;
            recalc_block_point(x, y);
            if (trapped) {
                /* re-forming inside the door is not working its knob;
                   every booby-trap variety that can kill reports the
                   death itself, so nothing is said here */
                (void) mb_trapped(mtmp, FALSE);
            }
        }
        newsym(x, y);
        return TRUE;
    }
    return FALSE;
}

/* specific combination of x_monnam flags for livelogging; show what was
   actually killed even when unseen or hallucinated to be something else,
   and don't honor the farlook-only Rider-identity obfuscation either */
#define livelog_mon_nam(mtmp) \
    x_monnam(mtmp, ARTICLE_THE, (char *) 0, EXACT_NAME | SUPPRESS_RIDER, \
             FALSE)

/* when a mon has died, maybe record an achievement or issue livelog message;
   moved into separate routine to unclutter mondead() */
staticfn void
logdeadmon(
    struct monst *mtmp,
    int mndx,
    boolean was_player_kill,  /* death arrived via xkilled() */
    struct monst *pet_credit) /* tame monster responsible, if any */
{
    int howmany = svm.mvitals[mndx].died;

    /* Medusa and several special uniques fall into two livelog
       categories; we log one message flagged for both categories,
       but only for the first kill. Subsequent kills are not an
       achievement. Once-only gate bits are always set for game state.
       Livelogs only written for player or pet kills - other causes
       (monster vs monster, traps, etc.) are not logged */
    if (mndx == PM_MEDUSA && howmany == 1) {
        u.uevent.killed_medusa = 1;
        if (pet_credit) {
            livelog_printf(LL_ACHIEVE | LL_UMONST, "pet %s killed Medusa",
                           mon_pmname(pet_credit));
            record_achievement_quietly(ACH_MEDU);
        } else if (was_player_kill) {
            record_achievement(ACH_MEDU); /* logs "killed Medusa" */
        } else {
            /* achievement state only; the generic achievement line
               would credit the hero with a kill made by another cause */
            record_achievement_quietly(ACH_MEDU);
        }
    } else if (mtmp->iscerberus && !u.uevent.killed_cerberus) {
        u.uevent.killed_cerberus = 1;
        if (pet_credit)
            livelog_printf(LL_ACHIEVE | LL_UMONST, "pet %s killed Cerberus",
                           mon_pmname(pet_credit));
        else if (was_player_kill)
            livelog_printf(LL_ACHIEVE | LL_UMONST, "killed Cerberus");
    } else if (mtmp->isvecna && !u.uevent.killed_vecna) {
        u.uevent.killed_vecna = 1;
        if (pet_credit)
            livelog_printf(LL_ACHIEVE | LL_UMONST, "pet %s destroyed Vecna",
                           mon_pmname(pet_credit));
        else if (was_player_kill)
            livelog_printf(LL_ACHIEVE | LL_UMONST, "destroyed Vecna");
    } else if (mtmp->isvlad && !u.uevent.killed_vlad) {
        u.uevent.killed_vlad = 1;
        if (pet_credit)
            livelog_printf(LL_ACHIEVE | LL_UMONST,
                           "pet %s destroyed Vlad the Impaler",
                           mon_pmname(pet_credit));
        else if (was_player_kill)
            livelog_printf(LL_ACHIEVE | LL_UMONST,
                           "destroyed Vlad the Impaler");
    } else if (mtmp->istalgath && !u.uevent.killed_talgath) {
        u.uevent.killed_talgath = 1;
        if (pet_credit)
            livelog_printf(LL_ACHIEVE | LL_UMONST, "pet %s killed Tal'Gath",
                           mon_pmname(pet_credit));
        else if (was_player_kill)
            livelog_printf(LL_ACHIEVE | LL_UMONST, "killed Tal'Gath");
    } else if (mtmp->isgking && !u.uevent.killed_gking) {
        u.uevent.killed_gking = 1;
        if (pet_credit)
            livelog_printf(LL_ACHIEVE | LL_UMONST,
                           "pet %s killed the Goblin King",
                           mon_pmname(pet_credit));
        else if (was_player_kill)
            livelog_printf(LL_ACHIEVE | LL_UMONST,
                           "killed the Goblin King");
    } else if (mtmp->islucifer && !u.uevent.killed_lucifer) {
        u.uevent.killed_lucifer = 1;
        if (pet_credit)
            livelog_printf(LL_ACHIEVE | LL_UMONST, "pet %s killed Lucifer",
                           mon_pmname(pet_credit));
        else if (was_player_kill)
            livelog_printf(LL_ACHIEVE | LL_UMONST, "killed Lucifer");
    } else if (mtmp->ismichael && !u.uevent.killed_michael) {
        u.uevent.killed_michael = 1;
        if (pet_credit)
            livelog_printf(LL_ACHIEVE | LL_UMONST,
                           "pet %s killed Saint Michael",
                           mon_pmname(pet_credit));
        else if (was_player_kill)
            livelog_printf(LL_ACHIEVE | LL_UMONST,
                           "killed Saint Michael");
    } else if (mndx == PM_DEATH && (was_player_kill || pet_credit)) {
        switch (howmany) {
        case 1:
            if (pet_credit)
                livelog_printf(LL_ACHIEVE | LL_UMONST,
                               "pet %s put %s down for a little nap",
                               mon_pmname(pet_credit),
                               livelog_mon_nam(mtmp));
            else
                livelog_printf(LL_ACHIEVE | LL_UMONST,
                               "put %s down for a little nap",
                               livelog_mon_nam(mtmp));
            break;
        case 5:
        case 10:
        case 50:
        case 100:
        case 150:
        case 200:
        case 250:
            if (pet_credit)
                livelog_printf(LL_ACHIEVE | LL_UMONST,
                               "pet %s put %s down for a little nap"
                               " (%d times)",
                               mon_pmname(pet_credit),
                               livelog_mon_nam(mtmp), howmany);
            else
                livelog_printf(LL_ACHIEVE | LL_UMONST,
                               "put %s down for a little nap (%d times)",
                               livelog_mon_nam(mtmp), howmany);
            break;
        default:
            /* don't spam the log every time */
            break;
        }
    } else if ((unique_corpstat(mtmp->data)
                && (mndx != PM_HIGH_PRIEST || !mtmp->mrevived))
               || (mtmp->isshk && !mtmp->mrevived)) {
        char shkdetail[QBUFSZ];
        const char *mkilled;

        /*
         * livelog event; unique_corpstat() includes the Wizard and
         * any High Priest even though they aren't actually unique.
         *
         * Shopkeeper kills are logged, but only the first time per
         * shopkeeper, since their shared kill counter wouldn't work
         * for this purpose (and it wouldn't account for polymorphed
         * shopkeepers either).
         */
        shkdetail[0] = '\0';
        if (mtmp->isshk) {
            howmany = 1;
            Snprintf(shkdetail, sizeof shkdetail, ", the %s %s",
                     shtypes[ESHK(mtmp)->shoptype - SHOPBASE].name,
                     /* in case shk name doesn't include Mr or Ms honorific */
                     mtmp->female ? "proprietrix" : "proprietor");
        } else if (mndx == PM_HIGH_PRIEST) {
            /* the high priest[ess] monster is not unique; we know that
               this is the first death for this particular high priest
               (because of the !mtmp->mrevived test above) */
            howmany = 1;
        }

        /* killing a unique more than once doesn't get logged every time;
           the Wizard and the Riders can be killed more than once
           "naturally", others require deliberate player action such as
           use of undead turning to revive a corpse or petrification plus
           stone-to-flesh to create and revive a statue */
        if (howmany <= 3 || howmany == 5 || howmany == 10 || howmany == 25
            || (howmany % 50) == 0) { /* 50, 100, 150, 200, 250 */
            char xtra[40]; /* space for " (Nth time)" when N > 1 */
            long llevent_type = LL_UMONST;

            /* the first kill of any unique monster is a major event;
               all kills of the Wizard and the Riders are major when
               they're logged but they still don't get logged every time */
            if (howmany == 1 || mtmp->iswiz || is_rider(mtmp->data))
                llevent_type |= LL_ACHIEVE;
            xtra[0] = '\0';
            if (howmany > 1) /* "(2nd time)" or "(50th time)" */
                Sprintf(xtra, " (%d%s time)", howmany, ordin(howmany));

            mkilled = nonliving(mtmp->data) ? "destroyed" : "killed";
            if (pet_credit)
                livelog_printf(llevent_type, "pet %s %s %s%s%s",
                               mon_pmname(pet_credit), mkilled,
                               livelog_mon_nam(mtmp), shkdetail, xtra);
            else if (was_player_kill)
                livelog_printf(llevent_type, "%s %s%s%s",
                               mkilled,
                               livelog_mon_nam(mtmp), shkdetail, xtra);
        }
    }
}

/* anger all the quest guards on the level */
staticfn void
anger_quest_guardians(struct monst *mtmp)
{
    if (mtmp->data == &mons[gu.urole.guardnum])
        setmangry(mtmp, TRUE);
}

/* boss-death bookkeeping; these record the once-only game-state bits
   that unlock or alter later content (genocide gates, spawn rates,
   xlogfile achievements) and deliver the boss-defeat flavor text */
void
cerberusdead(void)
{
    if (!u.uevent.ucerberus)
        u.uevent.ucerberus = TRUE;
    com_pager("cerberus_defeated");
}

void
vecnadead(void)
{
    if (!u.uevent.uvecna)
        u.uevent.uvecna = TRUE;
    com_pager("vecna_defeated");
}

void
vladdead(void)
{
    if (!u.uevent.uvlad)
        u.uevent.uvlad = TRUE;
}

void
talgathdead(void)
{
    if (!u.uevent.utalgath)
        u.uevent.utalgath = TRUE;
}

void
goblinkingdead(void)
{
    if (!u.uevent.ugking)
        u.uevent.ugking = TRUE;
    com_pager("goblin_king_defeated");
}

void
luciferdead(void)
{
    if (!u.uevent.ulucifer)
        u.uevent.ulucifer = TRUE;
    com_pager("lucifer_defeated");
}

void
michaeldead(void)
{
    if (!u.uevent.umichael)
        u.uevent.umichael = TRUE;
}

/* monster 'mtmp' has died; maybe life-save, otherwise unshapeshift and
   update vanquished stats and update map */
void
mondead(struct monst *mtmp)
{
    struct permonst *mptr;
    struct monst *pet_credit, *rider;
    boolean was_player_kill;
    boolean be_sad;
    int mndx;

    /* consume the kill attribution state up front so that every path
       through here (including lifesaved early returns) clears it;
       deaths that bypass xkilled()/monkilled() can then never inherit
       a previous kill's attribution */
    was_player_kill = player_killed;
    pet_credit = (pet_killer && pet_killer->mtame) ? pet_killer
                                                   : (struct monst *) 0;
    player_killed = FALSE;
    pet_killer = (struct monst *) 0;

    /* potential pet message; always clear global flag */
    be_sad = iflags.sad_feeling;
    iflags.sad_feeling = FALSE;

    mtmp->mhp = 0; /* in case caller hasn't done this */
    lifesaved_monster(mtmp);
    if (!DEADMONSTER(mtmp))
        return;

    /* vampire in bat/fog/wolf form reverts to vampire instead of dying */
    if (is_vampshifter(mtmp) && vamprises(mtmp))
        return;

    /* the Ice Queen is defeated rather than killed; her revival as
       the Enchantress is handled by dmonsfree() at end of turn */
    if (mtmp->data == &mons[PM_KATHRYN_THE_ICE_QUEEN]) {
        if (!u.uevent.defeat_icequeen) {
            u.uevent.defeat_icequeen = 1;
            /* the defeat state is recorded regardless of cause, but
               the livelog is credit-gated like the killed_* boss
               lines; attribution was consumed on entry above */
            if (pet_credit)
                livelog_printf(LL_ACHIEVE | LL_UMONST,
                               "pet %s defeated %s",
                               mon_pmname(pet_credit),
                               livelog_mon_nam(mtmp));
            else if (was_player_kill)
                livelog_printf(LL_ACHIEVE | LL_UMONST, "defeated %s",
                               livelog_mon_nam(mtmp));
        }
        return; /* revive handled in dmonsfree */
    }
    /* our hero decided to choose poorly and attempt to kill
       Kathryn the Enchantress */
    if (mtmp->data == &mons[PM_KATHRYN_THE_ENCHANTRESS]) {
        if (canspotmon(mtmp)) {
            pline("But wait!  %s is not truly dead!", mon_nam(mtmp));
            pline("Not even death can overcome her magic!");
            gv.vamp_rise_msg = TRUE;
        }
        mtmp->mcanmove = 1;
        mtmp->mfrozen = 0;
        mtmp->mstone = 0;
        mtmp->msick = 0;
        mtmp->mdiseased = 0;
        mtmp->mwither = 0;
        mtmp->mconf = 0;
        mtmp->mstun = 0;
        mtmp->mpeaceful = 0;
        set_malign(mtmp);
        mtmp->m_lev = 100;
        mtmp->mhp = mtmp->mhpmax = 7500;
        if (mtmp == u.ustuck) {
            if (u.uswallow)
                expels(mtmp, mtmp->data, FALSE);
            else
                uunstick();
        }
        newsym(mtmp->mx, mtmp->my);
        if (u.ualign.type == A_NONE) {
            adjalign(10);
        } else {
            You_feel("very guilty.");
            adjalign(-15);
            record_abuse_event(-15, ABUSE_ATTACK_ENCHANTRESS);
        }
        change_luck(-15);
        return;
    }

    /* special handling for the Ice Queen's dogs */
    if (mtmp->data == &mons[PM_BOURBON]
        || mtmp->data == &mons[PM_OZZY]) {
        if (canspotmon(mtmp)) {
            pline("%s has been forced to submit, and %s is no longer hostile.",
                  Monnam(mtmp), mhe(mtmp));
        }
        mtmp->mcanmove = 1;
        mtmp->mfrozen = 0;
        mtmp->mstone = 0;
        mtmp->msick = 0;
        mtmp->mdiseased = 0;
        mtmp->mwither = 0;
        mtmp->mconf = 0;
        mtmp->mstun = 0;
        mtmp->mpeaceful = 1;
        set_malign(mtmp);
        newmonhp(mtmp, monsndx(mtmp->data));
        /* full health on submitting, not a random roll */
        mtmp->mhp = mtmp->mhpmax;
        if (mtmp == u.ustuck) {
            if (u.uswallow)
                expels(mtmp, mtmp->data, FALSE);
            else
                uunstick();
        }
        newsym(mtmp->mx, mtmp->my);
        gv.vamp_rise_msg = TRUE;
        return;
    }

    /* special handling for Vecna and his artifacts */
    if (mtmp->isvecna) {
        struct obj *otmp, *otmp2;

        if (!Blind) {
            if (is_lava(mtmp->mx, mtmp->my)
                || is_pool(mtmp->mx, mtmp->my))
                pline("As the remnants of %s body vanish, you notice some objects sink into the %s...",
                      s_suffix(mon_nam(mtmp)), surface(mtmp->mx, mtmp->my));
            else
                pline("As the remnants of %s body vanish, you notice some objects were left behind...",
                      s_suffix(mon_nam(mtmp)));
        }
        if (rn2(2)) {
            otmp = mksobj(EYEBALL, FALSE, FALSE);
            otmp = oname(otmp, artiname(ART_EYE_OF_VECNA), ONAME_RANDOM);
        } else {
            otmp = mksobj(MUMMIFIED_HAND, FALSE, FALSE);
            otmp = oname(otmp, artiname(ART_HAND_OF_VECNA), ONAME_RANDOM);
        }
        curse(otmp);
        place_object(otmp, mtmp->mx, mtmp->my);
        stackobj(otmp);
        /* create wand of wishing */
        otmp2 = mksobj(WAN_WISHING, TRUE, FALSE);
        (void) maybe_erodeproof(otmp2, 1);
        place_object(otmp2, mtmp->mx, mtmp->my);
        stackobj(otmp2);
    }

    if (be_sad)
        You("have a sad feeling for a moment, then it passes.");

    if (mtmp->data == &mons[PM_STEAM_VORTEX])
        create_gas_cloud(mtmp->mx, mtmp->my, rn2(10) + 5, 0, /* harmless */
                         mtmp);

    /* dead vault guard is actually kept at coordinate <0,0> until
       his temporary corridor to/from the vault has been removed;
       need to do this after life-saving and before m_detach() */
    if (mtmp->isgd && !grddead(mtmp))
        return;

    /* monster is thrown from its steed when the steed dies */
    rider = get_mon_rider(mtmp);
    if (rider)
        separate_steed_and_rider(rider);

    mptr = mtmp->data; /* save this for m_detach() */
    /* restore chameleon, lycanthropes to true form at death */
    if (ismnum(mtmp->cham)) {
        set_mon_data(mtmp, &mons[mtmp->cham]);
        mtmp->cham = NON_PM;
    } else if (mtmp->data == &mons[PM_WEREJACKAL])
        set_mon_data(mtmp, &mons[PM_HUMAN_WEREJACKAL]);
    else if (mtmp->data == &mons[PM_WEREWOLF])
        set_mon_data(mtmp, &mons[PM_HUMAN_WEREWOLF]);
    else if (mtmp->data == &mons[PM_WERERAT])
        set_mon_data(mtmp, &mons[PM_HUMAN_WERERAT]);
    else if (mtmp->data == &mons[PM_WEREDEMON])
        set_mon_data(mtmp, &mons[PM_DEMON_WEREDEMON]);

    /*
     * svm.mvitals[].died does double duty as total number of dead monsters
     * and as experience factor for the player killing more monsters.
     * this means that a dragon dying by other means reduces the
     * experience the player gets for killing a dragon directly; this
     * is probably not too bad, since the player likely finagled the
     * first dead dragon via ring of conflict or pets, and extinguishing
     * based on only player kills probably opens more avenues of abuse
     * for rings of conflict and such.
     */
    mndx = monsndx(mtmp->data);
    if (svm.mvitals[mndx].died < 255)
        svm.mvitals[mndx].died++;

    /* if it's a (possibly polymorphed) quest leader, mark him as dead */
    if (mtmp->m_id == svq.quest_status.leader_m_id)
        svq.quest_status.leader_is_dead = TRUE;
#ifdef MAIL_STRUCTURES
    /* if the mail daemon dies, no more mail delivery.  -3. */
    if (mndx == PM_MAIL_DAEMON)
        svm.mvitals[mndx].mvflags |= G_GENOD;
#endif

    if (mtmp->data->mlet == S_KOP) {
        stairway *stway = stairway_find_type_dir(FALSE, FALSE);

        /* Dead Kops may come back. */
        switch (rnd(5)) {
        case 1: /* returns near the stairs */
            if (stway) {
                (void) makemon(mtmp->data, stway->sx, stway->sy, NO_MM_FLAGS);
                break;
            }
            FALLTHROUGH;
            /* FALLTHRU */
        case 2: /* randomly */
            (void) makemon(mtmp->data, 0, 0, NO_MM_FLAGS);
            break;
        default:
            break;
        }
    }

    /* boss-death bookkeeping; set the once-only u.uevent state bits
       (the Wizard's own bookkeeping lives in m_detach since he can
       also leave the dungeon alive) */
    if (mtmp->iscerberus)
        cerberusdead();
    if (mtmp->isvecna)
        vecnadead();
    if (mtmp->isvlad)
        vladdead();
    if (mtmp->istalgath)
        talgathdead();
    if (mtmp->isgking)
        goblinkingdead();
    if (mtmp->islucifer)
        luciferdead();
    if (mtmp->ismichael)
        michaeldead();

    /* achievement and/or livelog */
    logdeadmon(mtmp, mndx, was_player_kill, pet_credit);

    if (glyph_is_invisible(levl[mtmp->mx][mtmp->my].glyph))
        unmap_object(mtmp->mx, mtmp->my);

    /* remove 'mtmp' from play; it will stay on the fmon list until end of
       current move, then dmonsfree() will get rid of it */
    m_detach(mtmp, mptr, TRUE);
    return;
}

/* TRUE if corpse might be dropped, magr may die if mon was swallowed */
boolean
corpse_chance(
    struct monst *mon,
    struct monst *magr,    /* killer, if swallowed */
    boolean was_swallowed) /* digestion */
{
    struct permonst *mdat = mon->data;
    int i, tmp;

    if (!magr && gm.mswallower && attacktype(gm.mswallower->data, AT_ENGL))
        magr = gm.mswallower, was_swallowed = TRUE; /* for gas spore boom */

    if (mdat == &mons[PM_VLAD_THE_IMPALER] || mdat->mlet == S_LICH) {
        if (cansee(mon->mx, mon->my) && !was_swallowed)
            pline_mon(mon, "%s body crumbles into dust.",
                      s_suffix(Monnam(mon)));
        return FALSE;
    }

    /* Trolls don't leave a corpse when the player is wielding Trollsbane */
    if (mdat->mlet == S_TROLL
        && wielding_artifact(ART_TROLLSBANE)
        && distu(mon->mx, mon->my) < 3) {
        if (cansee(mon->mx, mon->my))
            pline_mon(mon,
    "In the presence of Trollsbane, %s corpse flares brightly and burns to ashes.",
                      s_suffix(mon_nam(mon)));
        return FALSE;
    }

    /* Zombies don't leave a corpse when the player is wielding Sunsword */
    if (racial_zombie(mon)
        && (wielding_artifact(ART_SUNSWORD)
            || wielding_artifact(ART_HAMMER_OF_THE_GODS))
        && distu(mon->mx, mon->my) < 3) {
        if (cansee(mon->mx, mon->my)) {
            if (wielding_artifact(ART_SUNSWORD))
                pline_mon(mon,
    "In the presence of Sunsword, %s corpse dissolves into nothingness.",
                          s_suffix(mon_nam(mon)));
            else if (wielding_artifact(ART_HAMMER_OF_THE_GODS))
                pline_mon(mon,
    "In the presence of the Hammer of the Gods, %s corpse dissolves into nothingness.",
                          s_suffix(mon_nam(mon)));
        }
        return FALSE;
    }

    /* Gas spores always explode upon death */
    for (i = 0; i < NATTK; i++) {
        if (mdat->mattk[i].aatyp == AT_BOOM) {
            if (mdat->mattk[i].damn)
                tmp = d((int) mdat->mattk[i].damn, (int) mdat->mattk[i].damd);
            else if (mdat->mattk[i].damd)
                tmp = d((int) mdat->mlevel + 1, (int) mdat->mattk[i].damd);
            else
                tmp = 0;

            if (was_swallowed && magr) {
                /* mdef is a gas spore (AT_BOOM) that is exploding inside an
                   engulfer; suppress usual explosion since it's contained */
                if (magr == &gy.youmonst) {
                    There("is an explosion in your %s!", body_part(STOMACH));
                    Sprintf(svk.killer.name, "%s explosion",
                            s_suffix(pmname(mdat, Mgender(mon))));
                    losehp(Maybe_Half_Phys(tmp), svk.killer.name,
                           KILLED_BY_AN);
                } else {
                    You_hear("an explosion.");
                    magr->mhp -= tmp;
                    if (DEADMONSTER(magr)) {
                        /* swallowed pet gas spore rips its engulfer
                           open; parity with the exploder credit for
                           normal spore blasts */
                        if (mon->mtame)
                            set_pet_killer(mon);
                        mondied(magr);
                    }
                    if (DEADMONSTER(magr)) { /* maybe lifesaved */
                        if (canspotmon(magr))
                            pline_mon(magr, "%s rips open!", Monnam(magr));
                    } else if (canseemon(magr))
                        pline_mon(magr, "%s seems to have indigestion.",
                                  Monnam(magr));
                }
                return FALSE;
            }

            mon_explodes(mon, &mdat->mattk[i]);
            return FALSE;
        }
    }

    /* must duplicate this below check in xkilled() since it results in
     * creating no objects as well as no corpse
     */
    if (LEVEL_SPECIFIC_NOCORPSE(mdat))
        return FALSE;

    if (((r_bigmonst(mon) || mdat == &mons[PM_LIZARD]) && !mon->mcloned)
        || racial_zombie(mon) || is_golem(mdat) || is_mplayer(mdat)
        || is_rider(mdat) || mon->isshk)
        return TRUE;
    tmp = 2 + ((mdat->geno & G_FREQ) < 2) + r_verysmall(mon);
    return (boolean) !rn2(tmp);
}

/* drop (perhaps) a cadaver and remove monster */
void
mondied(struct monst *mdef)
{
    mondead(mdef);
    if (!DEADMONSTER(mdef))
        return; /* lifesaved */

    /* this assumes that the dead monster's map coordinates remain accurate */
    if (corpse_chance(mdef, (struct monst *) 0, FALSE)
        && (accessible(mdef->mx, mdef->my) || is_pool(mdef->mx, mdef->my)))
        (void) make_corpse(mdef, CORPSTAT_NONE);
}

/* monster disappears, not dies */
void
mongone(struct monst *mdef)
{
    struct monst *rider;

    mdef->mhp = 0; /* can skip some inventory bookkeeping */

    /* dead vault guard is actually kept at coordinate <0,0> until
       his temporary corridor to/from the vault has been removed */
    if (mdef->isgd && !grddead(mdef))
        return;
    /* monster is thrown from its steed when the steed disappears */
    rider = get_mon_rider(mdef);
    if (rider)
        separate_steed_and_rider(rider);

    /* stuck to you? release */
    unstuck(mdef);
    /* drop special items like the Amulet so that a dismissed Kop or nurse
       can't remove them from the game */
    mdrop_special_objs(mdef);
    /* release rest of monster's inventory--it is removed from game */
    discard_minvent(mdef, FALSE);
    m_detach(mdef, mdef->data, FALSE);
}

/* drop a statue or rock and remove monster */
void
monstone(struct monst *mdef)
{
    struct obj *otmp, *obj, *oldminvent;
    coordxy x = mdef->mx, y = mdef->my;
    boolean wasinside = FALSE;

    /* vampshifter reverts to vampire;
       3.6.3: also used to unshift shape-changed sandestin */
    if (!vamp_stone(mdef)) {
        /* stoning failed; drop any pending kill attribution since
           mondead() will not run to consume it */
        player_killed = FALSE;
        pet_killer = (struct monst *) 0;
        return;
    }

    /* we have to make the statue before calling mondead, to be able to
     * put inventory in it, and we have to check for lifesaving before
     * making the statue....
     */
    mdef->mhp = 0; /* in case caller hasn't done this */
    lifesaved_monster(mdef);
    if (!DEADMONSTER(mdef)) {
        player_killed = FALSE;
        pet_killer = (struct monst *) 0;
        return;
    }

    mdef->mtrapped = 0; /* (see m_detach) */
    mdef->mentangled = 0;

    if ((int) mdef->data->msize > MZ_TINY
        || !rn2(2 + ((int) (mdef->data->geno & G_FREQ) > 2))) {
        unsigned corpstatflags = CORPSTAT_NONE;

        oldminvent = 0;
        /* some objects may end up outside the statue */
        while ((obj = mdef->minvent) != 0) {
            extract_from_minvent(mdef, obj, TRUE, TRUE);
            if (obj->otyp == BOULDER
#if 0 /* monsters don't carry statues */
                ||  (obj->otyp == STATUE
                     && mons[obj->corpsenm].msize >= mdef->data->msize)
#endif
                /* invocation tools resist even with 0% resistance */
                || obj_resists(obj, 0, 0)) {
                if (flooreffects(obj, x, y, "fall"))
                    continue;
                place_object(obj, x, y);
            } else {
                if (obj->lamplit)
                    end_burn(obj, TRUE);
                obj->nobj = oldminvent;
                oldminvent = obj;
            }
        }
        /* defer statue creation until after inventory removal
           so that saved monster traits won't retain any stale
           item-conferred attributes */
        if (mdef->female)
            corpstatflags |= CORPSTAT_FEMALE;
        else if (!is_neuter(mdef->data))
            corpstatflags |= CORPSTAT_MALE;
        /* Archeologists should not break unique statues */
        if (mdef->data->geno & G_UNIQ)
            corpstatflags |= CORPSTAT_HISTORIC;
        otmp = mkcorpstat(STATUE, mdef, mdef->data, x, y, corpstatflags);
        if (has_mgivenname(mdef))
            otmp = oname(otmp, MGIVENNAME(mdef), ONAME_NO_FLAGS);
        while ((obj = oldminvent) != 0) {
            oldminvent = obj->nobj;
            obj->nobj = 0; /* avoid merged-> obfree-> dealloc_obj-> panic */
            (void) add_to_container(otmp, obj);
        }
        otmp->owt = weight(otmp);
    } else
        otmp = mksobj_at(ROCK, x, y, TRUE, FALSE);

    stackobj(otmp);
    /* mondead() already does this, but we must do it before the newsym */
    if (glyph_is_invisible(levl[x][y].glyph))
        unmap_object(x, y);
    if (cansee(x, y))
        newsym(x, y);
    /* we don't currently trap the hero in the statue in this case but we
       could */
    if (engulfing_u(mdef))
        wasinside = TRUE;
    mondead(mdef);
    if (wasinside) {
        if (digests(mdef->data))
            You("%s through an opening in the new %s.",
                u_locomotion("jump"), xname(otmp));
    }
    return;
}

/* another monster has killed the monster mdef */
void
monkilled(
    struct monst *mdef,
    const char *fltxt,
    int how)
{
    struct permonst *mptr = mdef->data;

    if (fltxt && (mdef->wormno ? worm_known(mdef)
                               : cansee(mdef->mx, mdef->my)))
        pline_mon(mdef, "%s is %s%s%s!", Monnam(mdef),
              nonliving(mptr) ? "destroyed" : "killed",
              *fltxt ? " by the " : "", fltxt);
    else
        /* sad feeling is deferred until after potential life-saving */
        iflags.sad_feeling = mdef->mtame ? TRUE : FALSE;

    /* no corpse if digested or disintegrated or flammable golem burnt up;
       no corpse for a paper golem means no scrolls; golems that rust or
       rot completely are described as "falling to pieces" so they do
       leave a corpse (which means staves for wood golem, leather armor for
       leather golem, iron chains for iron golem, not a regular corpse) */
    gd.disintegested = (how == AD_DGST || how == -AD_RBRE
                       || (how == AD_FIRE && completelyburns(mptr)));
    if (gd.disintegested)
        mondead(mdef); /* never leaves a corpse */
    else
        mondied(mdef); /* calls mondead() and maybe leaves a corpse */

    if (!DEADMONSTER(mdef))
        return; /* life-saved */
    /* extra message if pet golem is completely destroyed;
       if not visible, this will follow "you have a sad feeling" */
    if (mdef->mtame) {
        const char *rxt = (how == AD_FIRE && completelyburns(mptr)) ? "roast"
                          : (how == AD_RUST && completelyrusts(mptr)) ? "rust"
                            : (how == AD_DCAY && completelyrots(mptr)) ? "rot"
                              :  0;
        if (rxt)
            pline("May %s %s in peace.", noit_mon_nam(mdef), rxt);
    }
    return;
}

/* another monster has killed the monster mdef, but the hero takes the
   credit and the experience; used for hero-conjured helpers flagged
   with mon->uexp (summoned elemental orbs and similar) */
void
mon_xkilled(
    struct monst *mdef,
    const char *fltxt,
    int how)
{
    if (fltxt && (mdef->wormno ? worm_known(mdef)
                               : cansee(mdef->mx, mdef->my)))
        pline_mon(mdef, "%s is %s%s%s!", Monnam(mdef),
              nonliving(mdef->data) ? "destroyed" : "killed",
              *fltxt ? " by the " : "", fltxt);
    else
        /* sad feeling is deferred until after potential life-saving */
        iflags.sad_feeling = mdef->mtame ? TRUE : FALSE;

    /* no corpse if digested or disintegrated or flammable golem burnt
       up; mirrors monkilled()'s routing */
    if (how == AD_DGST || how == -AD_RBRE
        || (how == AD_FIRE && completelyburns(mdef->data)))
        xkilled(mdef, XKILL_NOMSG | XKILL_NOCORPSE | XKILL_INDIRECT);
    else
        xkilled(mdef, XKILL_NOMSG | XKILL_INDIRECT);
}

void
set_ustuck(struct monst *mtmp)
{
    if (iflags.sanity_check || iflags.debug_fuzzer) {
        if (mtmp && !m_next2u(mtmp))
            impossible("Sticking to %s at distu %d?",
                       mon_nam(mtmp), mdistu(mtmp));
    }

    disp.botl = TRUE;
    u.ustuck = mtmp;
    if (u.ustuck_mid)
        u.ustuck_mid = 0;
    if (!u.ustuck) {
        u.uswallow = 0;
        u.uswldtim = 0;
    }
}

void
unstuck(struct monst *mtmp)
{
    if (u.ustuck == mtmp) {
        struct permonst *ptr = mtmp->data;
        unsigned swallowed = u.uswallow;

        /* do this first so that docrt()'s botl update is accurate;
           clears u.uswallow as well as setting u.ustuck to Null */
        set_ustuck((struct monst *) 0);

        if (swallowed) {
            gm.mswallower = (struct monst *) 0;
            u.ux = mtmp->mx;
            u.uy = mtmp->my;
            if (Punished && uchain->where != OBJ_FLOOR)
                placebc();
            gv.vision_full_recalc = 1;
            docrt();
        }

        /* prevent holder/engulfer from immediately re-holding/re-engulfing
           [note: this call to unstuck() might be because u.ustuck has just
           changed shape and doesn't have a holding attack any more, hence
           don't set mspec_used unconditionally] */
        if (!mtmp->mspec_used && (dmgtype(ptr, AD_STCK)
                                  || attacktype(ptr, AT_ENGL)
                                  || attacktype(ptr, AT_HUGS)))
            mtmp->mspec_used = rnd(2);
    }
}

void
killed(struct monst *mtmp)
{
    xkilled(mtmp, XKILL_GIVEMSG);
}

/* the player has killed the monster mtmp */
void
xkilled(
    struct monst *mtmp,
    int xkill_flags) /* 1: suppress mesg, 2: suppress corpse, 4: pacifist */
{
    int tmp, mndx, scenario_penalty = 0;
    coordxy x = mtmp->mx, y = mtmp->my;
    struct monst museum = cg.zeromonst;
    struct permonst *mdat;
    struct obj *otmp;
    struct trap *t;
    boolean be_sad;
    boolean wasinside = engulfing_u(mtmp),
            burycorpse = FALSE,
            nomsg = (xkill_flags & XKILL_NOMSG) != 0,
            nocorpse = (xkill_flags & XKILL_NOCORPSE) != 0,
            noconduct = (xkill_flags & XKILL_NOCONDUCT) != 0,
            indirect = (xkill_flags & XKILL_INDIRECT) != 0;
    boolean not_con_inf = (!((u.ualign.type == A_NONE
                              || Role_if(PM_CONVICT))
                             && (mtmp->isgd || mtmp->isshk
                                 || is_watch(mtmp->data))));
    boolean uni_same = (is_unicorn(mtmp->data)
                        && sgn(u.ualign.type) == sgn(mtmp->data->maligntyp));

    /* potential pet message; always clear global flag */
    be_sad = iflags.sad_feeling;
    iflags.sad_feeling = FALSE;

    mtmp->mhp = 0; /* caller will usually have already done this */
    if (!noconduct) { /* KMH, conduct */
        if (!u.uconduct.killer++)
            livelog_printf(LL_CONDUCT, "killed for the first time");
    }

    /* once the Ice Queen has been defeated, she cannot be killed
       again on her way to reviving as the Enchantress */
    if (mtmp->data == &mons[PM_KATHRYN_THE_ICE_QUEEN]
        && u.uevent.defeat_icequeen)
        return;

    if (!nomsg) {
        boolean namedpet = has_mgivenname(mtmp) && !Hallucination;

        You("%s %s!",
            is_defeated(mtmp->data) ? "defeat"
              : nonliving(mtmp->data) ? "destroy" : "kill",
            !(wasinside || canspotmon(mtmp)) ? "it"
              : !mtmp->mtame ? mon_nam(mtmp)
                : x_monnam(mtmp, namedpet ? ARTICLE_NONE : ARTICLE_THE,
                           "poor",
                           namedpet ? (SUPPRESS_SADDLE | SUPPRESS_BARDING) : 0,
                           FALSE));
    }

    if (mtmp->mtrapped && (t = t_at(x, y)) != 0 && is_pit(t->ttyp)) {
        if (sobj_at(BOULDER, x, y))
            nocorpse = TRUE; /* Prevent corpses/treasure being created
                              * "on top" of boulder that is about to fall in.
                              * This is out of order, but cannot be helped
                              * unless this whole routine is rearranged. */
        if (m_carrying(mtmp, BOULDER))
            burycorpse = TRUE;
    }

    /* your pet knows who just killed it...watch out */
    if (mtmp->mtame && !mtmp->isminion)
        EDOG(mtmp)->killed_by_u = 1;

    if (wasinside && gt.thrownobj && gt.thrownobj != uball
        /* don't give to mon if missile is going to be destroyed */
        && gt.thrownobj->oclass != POTION_CLASS
        /* don't give to mon if missile is going to return to hero */
        && gt.thrownobj != (struct obj *) iflags.returning_missile) {
        /* thrown object has killed hero's engulfer; add it to mon's
           inventory now so that it will be placed with mon's other
           stuff prior to lookhere/autopickup when hero is expelled
           below (as a side-effect, this missile has immunity from
           being consumed [for this shot/throw only]) */
        mpickobj(mtmp, gt.thrownobj);
        /* let throwing code know that missile has been disposed of */
        gt.thrownobj = 0;
    }

    gv.vamp_rise_msg = FALSE; /* might get set in mondead(); checked below */
    gd.disintegested = nocorpse; /* alternate vamp_rise mesg needed if true */
    player_killed = TRUE; /* for livelog attribution in mondead() */
    /* dispose of monster and make cadaver */
    if (gs.stoned)
        monstone(mtmp);
    else
        mondead(mtmp);
    gd.disintegested = FALSE; /* reset */

    if (!DEADMONSTER(mtmp)) { /* monster lifesaved */
        /* Cannot put the non-visible lifesaving message in
         * lifesaved_monster() since the message appears only when _you_
         * kill it (as opposed to visible lifesaving which always appears).
         */
        gs.stoned = FALSE;
        if (!cansee(x, y) && !gv.vamp_rise_msg)
            pline("Maybe not...");
        return;
    }

    if (be_sad)
        You("have a sad feeling for a moment, then it passes.");

    mdat = mtmp->data; /* note: mondead can change mtmp->data */
    mndx = monsndx(mdat);

    if (gs.stoned) {
        gs.stoned = FALSE;
        goto cleanup;
    }

    if (nocorpse || (LEVEL_SPECIFIC_NOCORPSE(mdat) && !mtmp->isvecna))
        goto cleanup;

#ifdef MAIL_STRUCTURES
    if (mdat == &mons[PM_MAIL_DAEMON]) {
        stackobj(mksobj_at(SCR_MAIL, x, y, FALSE, FALSE));
    }
#endif
    if (accessible(x, y) || is_pool(x, y)) {
        struct obj *cadaver;
        int otyp;

        /* illogical but traditional "treasure drop" */
        if (!rn2(6) && !(svm.mvitals[mndx].mvflags & G_NOCORPSE)
            /* no extra item from swallower or steed */
            && (x != u.ux || y != u.uy)
            /* no extra item from kops--too easy to abuse */
            && mdat->mlet != S_KOP
            /* no items from cloned monsters */
            && !mtmp->mcloned) {
            otmp = mkobj(RANDOM_CLASS, TRUE);
            /* don't create large objects from small monsters */
            otyp = otmp->otyp;
            if (otmp->oclass == FOOD_CLASS && !(mdat->mflags2 & M2_COLLECT)
                && !otmp->oartifact) {
                /* don't drop newly created permafood from kills, unless
                   the monster collects food; it creates too much nutrition
                   in the late game and encourages grinding in the early
                   game; oartifact check is paranoia and will be redundant
                   until an artifact comestible is added */
                delobj(otmp);
            } else if (mdat->msize < MZ_HUMAN && otyp != FIGURINE
                /* oc_big is also oc_bimanual and oc_bulky */
                && (otmp->owt > 30 || objects[otyp].oc_big)) {
                if (otmp->oartifact) /* un-create */
                    artifact_exists(otmp, safe_oname(otmp), FALSE,
                                    ONAME_NO_FLAGS);
                delobj(otmp);
            } else if (!flooreffects(otmp, x, y, nomsg ? "" : "fall")) {
                place_object(otmp, x, y);
                stackobj(otmp);
            }
        }
        /* corpse--none if hero was inside the monster */
        if (!wasinside && corpse_chance(mtmp, (struct monst *) 0, FALSE)) {
            /* for kills by monsters credited to the hero (mon->uexp),
               zombify will have been set already by the caller where
               the attacking monster was known; for actual kills by
               the hero we can figure it out here */
            if (!indirect)
                gz.zombify = (!gt.thrownobj && !gs.stoned && !uwep
                             && zombie_maker(&gy.youmonst)
                             && zombie_form(r_data(mtmp)) != NON_PM);
            cadaver = make_corpse(mtmp, burycorpse ? CORPSTAT_BURIED
                                                   : CORPSTAT_NONE);
            gz.zombify = FALSE; /* reset */
            if (burycorpse && cadaver && cansee(x, y) && !mtmp->minvis
                && cadaver->where == OBJ_BURIED && !nomsg) {
                pline("%s corpse ends up buried.", s_suffix(Monnam(mtmp)));
            }
        }
    }

    if (wasinside) {
        /* spoteffects() can end up clearing level of monsters; grab a copy */
        museum = *mtmp;
        museum.nmon = 0;
        museum.minvent = 0;
        museum.mextra = 0;
        spoteffects(TRUE); /* poor man's expels() */
        mtmp = &museum; /* use the reference copy now */
    }
    /* monster is gone, corpse or other object might now be visible */
    newsym(x, y);

 cleanup:
    /* recalculate malign based on current alignment; it may be stale
       if the hero's alignment changed since the monster was created
       (e.g. helm of opposite alignment) */
    set_malign(mtmp);
    /* punish bad behaviour...
     * Change needed since there are racial shopkeepers now,
     * should also prevent triggering being a murderer on Astral
     * if you need to kill that renegade priest that's trying
     * to kill you.
     */
    if (((always_peaceful(mdat) && mtmp->malign <= 0)
        || (mtmp->isshk && !is_zombie(mdat)))) {
        if (not_con_inf)
            You_feel("guilty.");
        /* only applicable if hero is lawful or neutral */
        if (u.ualign.type > A_CHAOTIC) {
            HTelepat &= ~INTRINSIC;
            change_luck(-2);
            You("murderer!");
            if (Blind && !Blind_telepat)
                see_monsters(); /* can't sense monsters any more */
        }
    }
    if ((mtmp->mpeaceful && !rn2(2)) || mtmp->mtame)
        change_luck(-1);
    if (uni_same) {
        /* this is super bad, don't do it. adjalign() handled
           further down */
        if (u.ualign.type != A_NONE) {
            change_luck(-5);
            if (canspotmon(mtmp))
                You_feel("guilty...");
            else
                You("have a vague sense of profound guilt.");
        }
    }

    /* give experience points */
    tmp = experience(mtmp, (int) svm.mvitals[mndx].died);
    more_experienced(tmp, 0);
    newexplevel(); /* will decide if you go up */

    /* adjust alignment points */
    if (mtmp->m_id == svq.quest_status.leader_m_id) { /* REAL BAD! */
        if (u.ualign.type != A_NONE) {
            if (canspotmon(mtmp))
                You_feel("very guilty.");
            else
                You("have a vague sense of intense guilt.");
            adjalign(-(u.ualign.record + (int) ALIGNLIM / 2));
            record_abuse_event(-(u.ualign.record + (int) ALIGNLIM / 2),
                               ABUSE_KILL_LEADER);
        }
        if (u.ualign.type == A_NONE)
            ; /* Moloch's indifference */
        else
            u.ugangr += 7; /* instantly become "extremely" angry */
        change_luck(-20);
        pline("That was %sa bad idea...",
              u.uevent.qcompleted ? "probably " : "");
        if (!svc.context.mon_moving)
            iter_mons(anger_quest_guardians);
    } else if (mdat == &mons[PM_LOLTH]
               && u.ualign.type == A_CHAOTIC
               && Race_if(PM_DROW)) { /* Super bad, just killed your deity */
        if (canspotmon(mtmp))
            You_feel("very guilty.");
        else
            You("have a vague sense of intense guilt.");
        uchangealign(A_LAWFUL, 0); /* permanent alignment change */
        /* even though alignment has changed and the player is now
           bound to a different deity, they don't take kindly to one
           of their own being killed */
        adjalign(-(u.ualign.record + (int) ALIGNLIM / 2));
        record_abuse_event(-(u.ualign.record + (int) ALIGNLIM / 2),
                           ABUSE_KILL_DEITY);
        u.ugangr += 7; /* instantly become "extremely" angry */
        change_luck(-20);
        if (!Hallucination)
            pline("That was an extremely bad idea...");
        else
            pline("Umm... err...");
    } else if (mdat->msound == MS_NEMESIS) { /* Real good! */
        if (!svq.quest_status.killed_leader)
            adjalign((int) (ALIGNLIM / 4));
    } else if (mdat->msound == MS_GUARDIAN) { /* Bad */
        if (On_W_tower_level(&u.uz)) {
            ; /* nothing bad happens */
        } else {
            if (u.ualign.type != A_NONE) {
                if (canspotmon(mtmp))
                    You_feel("guilty.");
                else
                    You("have a vague sense of guilt.");
                scenario_penalty = -(int) (ALIGNLIM / 8);
                adjalign(scenario_penalty);
                record_abuse_event_mon(scenario_penalty,
                                       ABUSE_KILL_GUARDIAN, mtmp);
            }
            if (u.ualign.type == A_NONE)
                ; /* Moloch's indifference */
            else
                u.ugangr++;
            change_luck(-4);
            if (!Hallucination)
                pline("That was probably a bad idea...");
            else
                pline("Whoopsie-daisy!");
        }
    } else if (mtmp->ispriest && mtmp != &museum) {
        if (p_coaligned(mtmp))
            scenario_penalty = -2;
        adjalign((p_coaligned(mtmp)) ? -2 : 2);
        if (p_coaligned(mtmp))
            record_abuse_event_mon(-2, ABUSE_KILL_PRIEST, mtmp);
        /* cancel divine protection for killing your priest */
        if (p_coaligned(mtmp))
            u.ublessed = 0;
        else if (mdat->maligntyp == A_NONE)
            adjalign((int) (ALIGNLIM / 4)); /* BIG bonus */
        else if (u.ualign.type == A_NONE && mdat->maligntyp == A_LAWFUL)
            adjalign((int) (ALIGNLIM / 4)); /* Infidel-only BIG bonus */
    } else if (mtmp->mtame) {
        if (u.ualign.type == A_NONE) {
            if (canspotmon(mtmp))
                You_feel("a bit remorseful.");
            else
                You("have a vague sense of remorse.");
            scenario_penalty = -3;
            adjalign(-3); /* kinda bad, but it's how you roll */
            record_abuse_event_mon(-3, ABUSE_KILL_PET, mtmp);
        } else {
            if (canspotmon(mtmp))
                You_feel("very guilty.");
            else
                You("have a vague sense of intense guilt.");
            scenario_penalty = -15;
            adjalign(-15); /* bad!! */
            record_abuse_event_mon(-15, ABUSE_KILL_PET, mtmp);
        }
        /* your god is mighty displeased... */
        if (!Hallucination) {
            if (u.ualign.type == A_NONE) {
                You_hear("sinister laughter off in the distance...");
            } else {
                Soundeffect(se_distant_thunder, 40);
                You_hear("the rumble of distant thunder...");
            }
        } else {
            Soundeffect(se_applause, 40);
            You_hear("the studio audience applaud!");
        }
        if (!unique_corpstat(mdat)) {
            boolean mname = has_mgivenname(mtmp);
            livelog_printf(LL_KILLEDPET, "murdered %s%s%s faithful %s",
                           mname ? MGIVENNAME(mtmp) : "",
                           mname ? ", " : "",
                           uhis(), pmname(mdat, Mgender(mtmp)));
        }
    } else if (mtmp->mpeaceful) {
        if (u.ualign.type != A_NONE) {
            /* same alignment unicorn feedback handled before we reach
               this part of the code */
            if (!uni_same) {
                if (canspotmon(mtmp))
                    You_feel("guilty.");
                else
                    You("have a vague sense of guilt.");
            }
            scenario_penalty = u.ualign.type > A_CHAOTIC ? -5 : -1;
            adjalign(scenario_penalty);
            record_abuse_event_mon(scenario_penalty,
                                   uni_same ? ABUSE_KILL_UNICORN
                                            : ABUSE_KILL_PEACEFUL,
                                   mtmp);
        }
    }

    /* malign was already adjusted for u.ualign.type and randomization.
       This final adjalign() call is in addition to any prior adjalign()
       adjustments in this block of code. Infidels and Convicts in
       certain scenarios don't feel this kind of guilt.
       Cap the additional malign penalty proportionally: no worse than
       triple the scenario-specific penalty, and never exceeding -15 */
    if (not_con_inf) {
        int mal = mtmp->malign;

        if (scenario_penalty < 0) {
            int mal_cap = max(scenario_penalty * 3, -15);

            if (mal < mal_cap)
                mal = mal_cap;
        }
        adjalign(mal);
        if (mal < 0)
            record_abuse_event_mon(mal, ABUSE_KILL_PEACEFUL_MALIGN, mtmp);
    }

    if (has_ebones(mtmp)) {
        livelog_printf(LL_UMONST, "destroyed %s, %s former %s",
                       livelog_mon_nam(mtmp),
                       (mtmp->data == &mons[PM_GHOST]) ? "the" : "and",
                       rank_of(EBONES(mtmp)->deathlevel,
                               EBONES(mtmp)->mnum,
                               EBONES(mtmp)->female));
    }
    return;
}

#undef LEVEL_SPECIFIC_NOCORPSE
#undef livelog_mon_nam

/* changes the monster into a stone monster of the same type
   this should only be called when poly_when_stoned() is true */
void
mon_to_stone(struct monst *mtmp)
{
    if (mtmp->data->mlet == S_GOLEM) {
        /* it's a golem, and not a stone golem */
        if (canseemon(mtmp))
            pline_mon(mtmp, "%s solidifies...", Monnam(mtmp));
        if (newcham(mtmp, &mons[PM_STONE_GOLEM], NO_NC_FLAGS)) {
            if (canseemon(mtmp))
                pline("Now it's %s.", an(pmname(mtmp->data, Mgender(mtmp))));
        } else {
            if (canseemon(mtmp))
                pline("... and returns to normal.");
        }
    } else
        impossible("Can't polystone %s!", a_monnam(mtmp));
}

boolean
vamp_stone(struct monst *mtmp)
{
    if (is_vampshifter(mtmp)) {
        int mndx = mtmp->cham;
        coordxy x = mtmp->mx, y = mtmp->my;

        /* this only happens if shapeshifted */
        if (mndx >= LOW_PM && mndx != monsndx(mtmp->data)
            && !(svm.mvitals[mndx].mvflags & G_GENOD)) {
            char buf[BUFSZ];

            /* construct a format string before transformation */
            Sprintf(buf, "The lapidifying %s %s %s",
                    x_monnam(mtmp, ARTICLE_NONE, (char *) 0,
                             (SUPPRESS_SADDLE | SUPPRESS_BARDING
                              | SUPPRESS_HALLUCINATION
                              | SUPPRESS_INVISIBLE | SUPPRESS_IT), FALSE),
                    amorphous(mtmp->data) ? "coalesces on the"
                       : is_flyer(mtmp->data) ? "drops to the"
                          : "writhes on the",
                    surface(x, y));
            mtmp->mcanmove = 1;
            mtmp->mfrozen = 0;
            set_mon_min_mhpmax(mtmp, 10); /* mtmp->mhpmax=max(m_lev+1,10) */
            mtmp->mhp = mtmp->mhpmax;
            /* this can happen if previously a fog cloud */
            if (engulfing_u(mtmp))
                expels(mtmp, mtmp->data, FALSE);
            if (amorphous(mtmp->data) && closed_door(mtmp->mx, mtmp->my)) {
                coord new_xy;

                if (enexto(&new_xy, mtmp->mx, mtmp->my, &mons[mndx])) {
                    rloc_to(mtmp, new_xy.x, new_xy.y);
                }
            }
            if (canspotmon(mtmp)) {
                pline_mon(mtmp, "%s!", buf);
                display_nhwindow(WIN_MESSAGE, FALSE);
            }
            (void) newcham(mtmp, &mons[mndx], NO_NC_FLAGS);
            if (mtmp->data == &mons[mndx])
                mtmp->cham = NON_PM;
            else
                mtmp->cham = mndx;
            if (canspotmon(mtmp)) {
                pline_mon(mtmp,
                      "%s rises from the %s with renewed agility!",
                      Amonnam(mtmp), surface(mtmp->mx, mtmp->my));
            }
            newsym(mtmp->mx, mtmp->my);
            return FALSE;   /* didn't petrify */
        }
    } else if (ismnum(mtmp->cham)
               && (mons[mtmp->cham].mresists & MR_STONE)) {
        /* sandestins are stoning-immune so if hit by stoning damage
           they revert to innate shape rather than become a statue */
        mtmp->mcanmove = 1;
        mtmp->mfrozen = 0;
        set_mon_min_mhpmax(mtmp, 10); /* mtmp->mhpmax=max(mtmp->m_lev+1,10) */
        mtmp->mhp = mtmp->mhpmax;
        (void) newcham(mtmp, &mons[mtmp->cham], NC_SHOW_MSG);
        newsym(mtmp->mx, mtmp->my);
        return FALSE;   /* didn't petrify */
    }
    return TRUE;
}

/* drop monster into "limbo" - that is, migrate to the current level */
void
m_into_limbo(struct monst *mtmp)
{
    xint16 target_lev = ledger_no(&u.uz), xyloc = MIGR_APPROX_XY;

    mtmp->mstate |= MON_LIMBO;
    migrate_mon(mtmp, target_lev, xyloc);
}

void
migrate_mon(
    struct monst *mtmp,
    xint16 target_lev, /* destination level */
    xint16 xyloc)      /* MIGR_xxx flag for location within destination */
{
    /*
     * If mtmp->mx is zero, this was a failed arrival attempt from a
     * prior migration and mtmp isn't on the map.  In that situation
     * it can't be engulfing or holding the hero or held by same and
     * should have dropped any special objects during that earlier
     * migration back when it had a valid map location.  So only
     * perform some actions when mx is non-zero.
     */
    if (mtmp->mx) {
        unstuck(mtmp);
        mdrop_special_objs(mtmp);
    }
    migrate_to_level(mtmp, target_lev, xyloc, (coord *) 0);
}

staticfn boolean
ok_to_obliterate(struct monst *mtmp)
{
    /*
     * Add checks for monsters that should not be obliterated
     * here (return FALSE).
     */
    if (mtmp->data == &mons[PM_WIZARD_OF_YENDOR] || is_rider(mtmp->data)
        /* neither half of a mounted pair: obliterating one re-places
           the survivor on the shared square, which the caller's
           rloc_to() placement would then stomp */
        || has_erid(mtmp) || mtmp->ridden_by
        || has_emin(mtmp) || has_epri(mtmp) || has_eshk(mtmp)
        || mtmp == u.ustuck || mtmp == u.usteed)
        return FALSE;
    return TRUE;
}

void
elemental_clog(struct monst *mon)
{
    static long msgmv = 0L;
    int m_lev = 0;
    struct monst *mtmp, *m1, *m2, *m3, *m4, *m5, *zm;

    if (In_endgame(&u.uz)) {
        m1 = m2 = m3 = m4 = m5 = zm = (struct monst *) 0;
        if (!msgmv || (svm.moves - msgmv) > 200L) {
            if (!msgmv || rn2(2))
                You_feel("besieged.");
            msgmv = svm.moves;
        }
        /*
         * m1 an elemental from another plane.
         * m2 an elemental from this plane.
         * m3 the least powerful monst encountered in loop so far.
         * m4 some other non-tame monster.
         * m5 a pet.
         */
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
            if (DEADMONSTER(mtmp) || mtmp == mon)
                continue;
            if (mtmp->mx == 0 && mtmp->my == 0)
                continue;
            if (mon_has_amulet(mtmp) || !ok_to_obliterate(mtmp))
                continue;
            if (mtmp->data->mlet == S_ELEMENTAL) {
                if (!is_home_elemental(mtmp->data)) {
                    if (!m1)
                        m1 = mtmp;
                } else {
                    if (!m2)
                        m2 = mtmp;
                }
            } else {
                if (!mtmp->mtame) {
                    if (!m_lev || mtmp->m_lev < m_lev) {
                        m_lev = mtmp->m_lev;
                        m3 = mtmp;
                    } else if (!m4) {
                        m4 = mtmp;
                    }
                } else {
                    if (!m5)
                        m5 = mtmp;
                    break;
                }
            }
        }
        mtmp = m1 ? m1 : m2 ? m2 : m3 ? m3 : m4 ? m4 : m5 ? m5 : zm;
        if (mtmp) {
            int mx = mtmp->mx, my = mtmp->my;

            mtmp->mstate |= MON_OBLITERATE;
            mongone(mtmp);
            /* places in the code might still reference mtmp->mx, mtmp->my */
            /* mtmp->mx = mtmp->my = 0; */
            rloc_to(mon, mx, my);           /* note: mon, not mtmp */

        /* last resort - migrate mon to the next plane */
        } else if (!Is_astralevel(&u.uz)) {
            d_level dest;
            coordxy target_lev;

            dest = u.uz;
            dest.dlevel--;
            target_lev = ledger_no(&dest);
            mon->mstate |= MON_ENDGAME_MIGR;
            migrate_mon(mon, target_lev, MIGR_RANDOM);
        }
    }
}

/* make monster mtmp next to you (if possible);
   might place monst on far side of a wall or boulder */
void
mnexto(struct monst *mtmp, unsigned int rlocflags)
{
    coord mm;

    if (mtmp == u.usteed) {
        /* Keep your steed in sync with you instead */
        mtmp->mx = u.ux;
        mtmp->my = u.uy;
        return;
    }

    if (!enexto_core_mon(&mm, u.ux, u.uy, mtmp, NO_MM_FLAGS)
        || !isok(mm.x, mm.y)) {
        deal_with_overcrowding(mtmp);
        return;
    }
    /* wizard-mode player can choose destination by setting 'montelecontrol'
       option; enexto()'s value for 'mm' will be the default; 'savemm' is
       used to make sure player doesn't choose hero's location and then
       answer 'y' to the 'override invalid spot' prompt */
    if (iflags.mon_telecontrol) {
        coord savemm = mm;

        if (!control_mon_tele(mtmp, &mm, rlocflags, FALSE))
            mm = savemm;
    }

    rloc_to_flag(mtmp, mm.x, mm.y, rlocflags);
    update_monsteed(mtmp);
    return;
}

void
deal_with_overcrowding(struct monst *mtmp)
{
    if (In_endgame(&u.uz)) {
        debugpline1("overcrowding: elemental_clog on %s", m_monnam(mtmp));
        elemental_clog(mtmp);
    } else {
        debugpline1("overcrowding: sending %s into limbo", m_monnam(mtmp));
        m_into_limbo(mtmp);
    }
}

/* like mnexto() but requires destination to be directly accessible */
void
maybe_mnexto(struct monst *mtmp)
{
    coord mm;
    struct permonst *ptr = mtmp->data;
    boolean diagok = !NODIAG(ptr - mons);
    int tryct = 20;

    do {
        if (!enexto_core_mon(&mm, u.ux, u.uy, mtmp, NO_MM_FLAGS))
            return;
        if (couldsee(mm.x, mm.y)
            /* don't move grid bugs diagonally */
            && (diagok || mm.x == mtmp->mx || mm.y == mtmp->my)) {
            /* [this doesn't honor the 'montelecontrol' option] */
            rloc_to(mtmp, mm.x, mm.y);
            return;
        }
    } while (--tryct > 0);
}

/* mnearto()
 * Put monster near (or at) location if possible.
 * Returns:
 *  2 if another monster was moved out of this one's way;
 *  1 if relocation was successful (without moving another one);
 *  0 otherwise.
 * Note: if already at the target spot, result is 1 rather than 0.
 *
 * Might be called recursively if 'move_other' is True; if so, that argument
 * will be False on the nested call so there won't be any further recursion.
 */
int
mnearto(
    struct monst *mtmp,
    coordxy x,
    coordxy y,
    boolean move_other, /* make sure mtmp gets to x, y! so move m_at(x, y) */
    unsigned int rlocflags)
{
    struct monst *othermon = (struct monst *) 0;
    coordxy newx, newy;
    coord mm;
    int res = 1;

    if (mtmp->mx == x && mtmp->my == y && m_at(x, y) == mtmp)
        return res;

    if (move_other && (othermon = m_at(x, y)) != 0) {
        /* take othermon off the map; it might end up immediately returning
           but for the moment it is leaving */
        mon_leaving_level(othermon);
        othermon->mx = othermon->my = 0; /* 'othermon' is not on the map */
        othermon->mstate |= MON_OFFMAP;
    }

    newx = x;
    newy = y;
    if (!goodpos(newx, newy, mtmp, 0)) {
        /* Actually we have real problems if enexto ever fails.
         * Migrating_mons that need to be placed will cause
         * no end of trouble.
         */
        if (!enexto_core_mon(&mm, newx, newy, mtmp, NO_MM_FLAGS)
            || !isok(mm.x, mm.y)) {
            if (othermon) {
                /* othermon already had its mx, my set to 0 above
                 * and this would shortly cause a sanity check to fail
                 * if we just return 0 here. The caller only possesses
                 * awareness of mtmp, not othermon. */
                deal_with_overcrowding(othermon);
            }
            return 0;
        }
        newx = mm.x;
        newy = mm.y;
    }
    /* [this doesn't honor the 'montelecontrol' option] */
    rloc_to_flag(mtmp, newx, newy, rlocflags);

    if (move_other && othermon) {
        res = 2; /* moving another monster out of the way */
        /* 'move_other'==FALSE this time; fail rather than recurse */
        if (!mnearto(othermon, x, y, FALSE, rlocflags))
            deal_with_overcrowding(othermon);
    }

    return res;
}

/* shrieker special action: shriek, maybe summon monster, aggravate */
staticfn void
m_respond_shrieker(struct monst *mtmp)
{
    if (!Deaf) {
        pline("%s shrieks.", Monnam(mtmp));
        stop_occupation();
    }
    if (!rn2(10)) { /* 1/10 chance per shriek to create a monster */
        /* new monster has a 1/13 chance to be a purple worm, random
           otherwise; baby purple worm if adult is too difficult */
        (void) makemon(rn2(13) ? (struct permonst *) 0
                       : &mons[montoostrong(PM_PURPLE_WORM,
                                            monmax_difficulty_lev())
                               ? PM_BABY_PURPLE_WORM : PM_PURPLE_WORM],
                       0, 0, NO_MM_FLAGS);
    }
    aggravate();
}

/* medusa special action: gaze at hero */
staticfn void
m_respond_medusa(struct monst *mtmp)
{
    int i;

    for (i = 0; i < NATTK; i++)
        if (mtmp->data->mattk[i].aatyp == AT_GAZE) {
            (void) gazemu(mtmp, &mtmp->data->mattk[i]);
            break;
        }
}

/* zombie special action: moan for brains */
staticfn void
m_respond_zombie(struct monst *mtmp)
{
    if (Deaf)
        return;
    if (canseemon(mtmp))
        pline("%s %s.", Monnam(mtmp),
              !rn2(8) ? "mumbles, \"BRAAAAAAAAINS...\""
                      : !rn2(3) ? "groans" : "moans");
    else if (!rn2(4))
        You_hear("%s.",
                 !rn2(8) ? "a low voice mumble \"UUUNNNNGGHH...\""
                         : !rn2(3) ? "a low groaning" : "a low moaning");
}

/* monster responds to player action; not the same as a passive attack */
void
m_respond(struct monst *mtmp)
{
    if (mtmp->data->msound == MS_SHRIEK && !um_dist(mtmp->mx, mtmp->my, 1))
        m_respond_shrieker(mtmp);
    if (mtmp->data == &mons[PM_MEDUSA] && couldsee(mtmp->mx, mtmp->my))
        m_respond_medusa(mtmp);
    if (is_zombie(mtmp->data) && !rn2(10))
        m_respond_zombie(mtmp);
    /* Erinyes will inform surrounding monsters of your crimes */
    if (mtmp->data == &mons[PM_ERINYS] && !mtmp->mpeaceful && m_canseeu(mtmp))
        aggravate();
}

/* how quest guardians respond when you attack -- or rob -- the quest
   leader; steal_it() calls this too, so it can't be static here */
void
qst_guardians_respond(void)
{
    struct monst *mon;
    struct permonst *q_guardian = &mons[quest_info(MS_GUARDIAN)];
    int got_mad = 0;

    /* guardians will sense this attack even if they can't see it */
    for (mon = fmon; mon; mon = mon->nmon) {
        if (DEADMONSTER(mon))
            continue;
        if (mon->data == q_guardian && mon->mpeaceful) {
            mon->mpeaceful = 0;
            set_malign(mon);
            newsym(mon->mx, mon->my); /* clear peaceful glyph */
            if (canseemon(mon))
                ++got_mad;
        }
    }
    if (got_mad && !Hallucination) {
        const char *who = q_guardian->pmnames[NEUTRAL];

        if (got_mad > 1)
            who = makeplural(who);
        pline_The("%s %s to be angry too...",
                  who, vtense(who, "appear"));
    }
}

/* how other peacefuls react when you attack monster */
staticfn void
peacefuls_respond(struct monst *mtmp, int accidental)
{
    struct monst *mon;
    int mndx = monsndx(mtmp->data);

    for (mon = fmon; mon; mon = mon->nmon) {
        if (DEADMONSTER(mon))
            continue;
        if (mon == mtmp) /* the mpeaceful test catches this since mtmp */
            continue;    /* is no longer peaceful, but be explicit...  */

        if (!mindless(mon->data) && mon->mpeaceful
            && couldsee(mon->mx, mon->my) && !mon->msleeping
            && mon->mcansee && m_canseeu(mon)) {
            char buf[BUFSZ];
            boolean exclaimed = FALSE, needpunct = FALSE, alreadyfleeing;

            buf[0] = '\0';
            if (humanoid(mon->data) || mon->isshk || mon->ispriest) {
                if (is_watch(mon->data)) {
                    SetVoice(mon, 0, 80, 0);
                    verbalize("Halt!  You're under arrest!");
                    (void) angry_guards(!!Deaf);
                } else {
                    if (!Deaf && !rn2(5)) {
                        const char *gasp = maybe_gasp(mon);

                        if (gasp) {
                            if (!strncmpi(gasp, "gasp", 4)) {
                                Sprintf(buf, "%s gasps", Monnam(mon));
                                needpunct = TRUE;
                            } else {
                                Sprintf(buf, "%s exclaims \"%s\"",
                                        Monnam(mon), gasp);
                            }
                            exclaimed = TRUE;
                        }
                    }
                    /* shopkeepers and temple priests might gasp in
                       surprise, but they won't become angry here;
                       quest leader will only get angry if hero attacks
                       own quest guardians */
                    if (mon->isshk || mon->ispriest
                        || (mon->data == &mons[quest_info(MS_LEADER)]
                            && mtmp->data != &mons[gu.urole.guardnum])) {
                        if (exclaimed)
                            pline_mon(mon, "%s%s", buf, " then shrugs.");
                        continue;
                    }

                    if (mon->data->mlevel < rn2(10)
                        /* don't have quest guardians turn to flee */
                        && (mon->data != &mons[gu.urole.guardnum])) {
                        alreadyfleeing = (mon->mflee || mon->mfleetim);
                        monflee(mon, rn2(50) + 25, TRUE, !exclaimed);
                        if (exclaimed) {
                            if (flags.verbose && !alreadyfleeing) {
                                Strcat(buf, " and then turns to flee.");
                                needpunct = FALSE;
                            }
                        } else
                            exclaimed = TRUE; /* got msg from monflee() */
                    }
                    if (*buf)
                        pline_mon(mon, "%s%s", buf, needpunct ? "." : "");
                    if (mon->mtame) {
                        ; /* mustn't set mpeaceful to 0 as below;
                           * perhaps reduce tameness? */
                    } else {
                        mon->mpeaceful = 0;
                        set_malign(mon);
                        newsym(mon->mx, mon->my); /* clear peaceful glyph */
                        mon->mstrategy &= ~STRAT_WAITMASK;
                        /* bystanders still anger over an
                           accidental bump, but charge nothing */
                        if (u.ualign.type != A_NONE && !accidental) {
                            if (canspotmon(mon))
                                You_feel("guilty.");
                            else
                                You("have a vague sense of guilt.");
                            adjalign(-1);
                            record_abuse_event_mon(-1, ABUSE_ANGER_PEACEFUL,
                                                   mon);
                        }
                        if (!exclaimed)
                            pline_mon(mon, "%s gets angry!", Monnam(mon));
                    }
                }
            } else if (mon->data->mlet == mtmp->data->mlet
                       && big_little_match(mndx, monsndx(mon->data))
                       && !rn2(3)) {
                if (!rn2(4)) {
                    growl(mon);
                    exclaimed = (iflags.last_msg == PLNMSG_GROWL);
                }
                if (rn2(6)) {
                    alreadyfleeing = (mon->mflee || mon->mfleetim);
                    monflee(mon, rn2(25) + 15, TRUE, !exclaimed);
                    if (exclaimed && !alreadyfleeing)
                        /* word like a separate sentence so that we
                           don't have to poke around inside growl() */
                        pline("And then starts to flee.");
                }
            }
        }
    }
}

/* Called whenever the player attacks mtmp; also called in other situations
   where mtmp gets annoyed at the player. Handles mtmp getting annoyed at the
   attack and any ramifications that might have. Useful also in situations
   where mtmp was already hostile; it checks for situations where the player
   shouldn't be attacking and any ramifications /that/ might have. */
void
setmangry(struct monst *mtmp, boolean via_attack)
{
    int accidental = ga.accidental_bump;

    /* consume the flag immediately so early returns or nested
       calls never inherit it */
    ga.accidental_bump = ACCIDENT_NONE;

    if (via_attack && sengr_at("Elbereth", u.ux, u.uy, TRUE)
        /* only hypocritical if monster is vulnerable to Elbereth (or
           peaceful--not vulnerable but attacking it is hypocritical) */
        && (onscary(u.ux, u.uy, mtmp) || mtmp->mpeaceful)) {
        /* AIS: Yes, I know alignment penalties and bonuses aren't balanced
           at the moment. This is about correct relative to other "small"
           penalties; it should be fairly large, as attacking while standing
           on an Elbereth means that you're requesting peace and then
           violating your own request. I know 5 isn't actually large, but
           it's intentionally larger than the 1s and 2s that are normally
           given for this sort of thing */
        /* reduce to 3 (average) when alignment is already very low */
        /* blundering into an unseen monster is not hypocrisy, but
           the engraving still fades below */
        if (!accidental) {
            if (u.ualign.type != A_NONE) {
                int pen = (u.ualign.record > 5) ? -5 : -rnd(5);

                You_feel("like a hypocrite.");
                adjalign(pen);
                record_abuse_event_mon(pen, ABUSE_ATTACK_ELBERETH, mtmp);
            } else
                You_feel("clever."); /* no alignment penalty */
        }

        if (!Blind)
            pline("The engraving beneath you fades.");
        del_engr_at(u.ux, u.uy);
    }

    if (via_attack
        && (mtmp->data == &mons[PM_KATHRYN_THE_ICE_QUEEN]
            || mtmp->data == &mons[PM_KATHRYN_THE_ENCHANTRESS])) {
        struct monst *mon;
        struct permonst *bourbon = &mons[PM_BOURBON];
        struct permonst *ozzy = &mons[PM_OZZY];

        for (mon = fmon; mon; mon = mon->nmon) {
            if (DEADMONSTER(mon))
                continue;
            if (mon->data == bourbon && mon->mpeaceful) {
                mon->mstrategy &= ~STRAT_WAITMASK;
                mon->mpeaceful = 0;
                set_malign(mon);
                growl(mon);
            }
            if (mon->data == ozzy && mon->mpeaceful) {
                mon->mstrategy &= ~STRAT_WAITMASK;
                mon->mpeaceful = 0;
                set_malign(mon);
                growl(mon);
            }
            newsym(mon->mx, mon->my); /* clear peaceful glyph */
        }
    }

    /* AIS: Should this be in both places, or just in wakeup()? */
    if (!(via_attack
          && (Role_if(PM_ROGUE) && !uwep
              && svc.context.forcefight && !Upolyd))) {
        mtmp->mstrategy &= ~STRAT_WAITMASK;
        if (!mtmp->mpeaceful)
            return;
        /* [FIXME: this logic seems wrong; peaceful humanoids gasp or exclaim
           when they see you attack a peaceful monster but they just casually
           look the other way when you attack a pet?] */
        if (mtmp->mtame)
            return;
        mtmp->mpeaceful = 0;
        set_malign(mtmp);
        newsym(mtmp->mx, mtmp->my); /* clear peaceful glyph */
        /* peacefuls always catch convicts stealing. but, convicts don't
           feel guilty about it. (note there's still an alignment penalty
           for failing to steal from a priest in steal_it()) */
        if (!(via_attack
              && (Role_if(PM_CONVICT)
                  && !uwep && svc.context.forcefight && !Upolyd))) {
            if (mtmp->ispriest) {
                if (p_coaligned(mtmp)) {
                    if (accidental) {
                        /* blundering into your own deity's priest is
                           not malice, so no alignment abuse, but the
                           gods take a very dim view of it regardless */
                        u.uluck = LUCKMIN;
                        pline(accidental == ACCIDENT_COLLISION
                                  ? "That was extremely unfortunate..."
                                  : "That was extremely careless of you.");
                    } else {
                        if (canspotmon(mtmp))
                            You_feel("guilty.");
                        else
                            You("have a vague sense of guilt.");
                        adjalign(-5); /* very bad */
                        record_abuse_event_mon(-5, ABUSE_ANGER_PRIEST, mtmp);
                    }
                } else {
                    adjalign(2);
                }
            } else {
                /* Infidels and Convicts don't feel guilt from this */
                if (!(u.ualign.type == A_NONE || Role_if(PM_CONVICT))) {
                    if (accidental) {
                        /* contact with a monster the hero had no way
                           to detect or avoid is not malice; charge
                           luck instead of alignment */
                        change_luck(-2);
                        pline(accidental == ACCIDENT_COLLISION
                                  ? "That was unfortunate..."
                                  : "That was careless of you.");
                    } else {
                        int pen = u.ualign.type > A_CHAOTIC ? -5 : -1;

                        if (canspotmon(mtmp))
                            You_feel("guilty.");
                        else
                            You("have a vague sense of guilt.");
                        /* attacking or merely angering peaceful monsters
                           is bad; distinguish the two for the record */
                        adjalign(pen);
                        record_abuse_event_mon(pen,
                                               via_attack
                                                 ? ABUSE_ATTACK_PEACEFUL
                                                 : ABUSE_ANGER_PEACEFUL,
                                               mtmp);
                    }
                }
            }
        }
        if (mtmp->data == &mons[PM_ORACLE])
            mons[PM_ORACLE].mmove = 18;
        if (mtmp->data == &mons[PM_CHARON])
            mons[PM_CHARON].mmove = 18;
        if (humanoid(mtmp->data) || mtmp->isshk || mtmp->isgd) {
            if (couldsee(mtmp->mx, mtmp->my))
                pline_mon(mtmp, "%s gets angry!", Monnam(mtmp));
        } else {
            growl(mtmp);
        }

        /* attacking your own quest leader will anger his or her guardians */
        if (mtmp->data == &mons[quest_info(MS_LEADER)])
            qst_guardians_respond();

        /* make other peaceful monsters react */
        if (!svc.context.mon_moving)
            peacefuls_respond(mtmp, accidental);
    }
}

/* Indicate via message that a monster has awoken. */
void
wake_msg(struct monst *mtmp, boolean interesting)
{
    if (mtmp->msleeping && canseemon(mtmp)) {
        pline_mon(mtmp, "%s wakes up%s%s",
              Monnam(mtmp), interesting ? "!" : ".",
              mtmp->data == &mons[PM_FLESH_GOLEM] ? " It's alive!" : "");
    }
}

/* wake up a monster, possibly making it angry in the process */
void
wakeup(struct monst *mtmp, boolean via_attack)
{
    boolean was_sleeping = mtmp->msleeping;

    wake_msg(mtmp, via_attack);
    mtmp->msleeping = 0;
    if (M_AP_TYPE(mtmp) != M_AP_NOTHING) {
        /* mimics come out of hiding, but disguised Wizard doesn't
           have to lose his disguise */
        if (M_AP_TYPE(mtmp) != M_AP_MONSTER)
            seemimic(mtmp);
    } else if (svc.context.forcefight && !svc.context.mon_moving
               && mtmp->mundetected) {
        mtmp->mundetected = 0;
        newsym(mtmp->mx, mtmp->my);
    }
    finish_meating(mtmp);
    if (via_attack) {
        boolean was_peaceful = mtmp->mpeaceful;

        if (was_sleeping)
            growl(mtmp);
        setmangry(mtmp, TRUE);
        if (was_peaceful) {
            if (mtmp->ispriest && *in_rooms(mtmp->mx, mtmp->my, TEMPLE))
                ghod_hitsu(mtmp);
            if (mtmp->isshk && !*u.ushops)
                hot_pursuit(mtmp);
        }
    }
}

/* hero blundered into a monster they had no way to detect; anger it
   as an attack but charge luck rather than alignment */
void
wakeup_accidental(struct monst *mtmp)
{
    ga.accidental_bump = ACCIDENT_BUMP;
    wakeup(mtmp, TRUE);
    ga.accidental_bump = ACCIDENT_NONE; /* setmangry consumes it; be safe */
}

/* monster angered by an involuntary collision (hero knocked back,
   jumping blind, or a monster the hero sent flying); anger it but
   charge luck rather than alignment */
void
setmangry_accidental(struct monst *mtmp, boolean via_attack)
{
    ga.accidental_bump = ACCIDENT_COLLISION;
    setmangry(mtmp, via_attack);
    ga.accidental_bump = ACCIDENT_NONE; /* setmangry consumes it; be safe */
}

/* Wake up nearby monsters without angering them. */
void
wake_nearby(boolean petcall)
{
    wake_nearto_core(u.ux, u.uy, u.ulevel * 20, petcall);
}

/* Wake up monsters near some particular location. */
staticfn void
wake_nearto_core(coordxy x, coordxy y, int distance, boolean petcall)
{
    struct monst *mtmp;

    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp))
            continue;
        if (distance == 0 || dist2(mtmp->mx, mtmp->my, x, y) < distance) {
            /* sleep for N turns uses mtmp->mfrozen, but so does paralysis
               so we leave mfrozen monsters alone */
            wake_msg(mtmp, FALSE);
            mtmp->msleeping = 0; /* wake indeterminate sleep */
            if (!(mtmp->data->geno & G_UNIQ))
                mtmp->mstrategy &= ~STRAT_WAITMASK; /* wake 'meditation' */
            if (svc.context.mon_moving || !petcall)
                continue;
            if (mtmp->mtame) {
                if (!mtmp->isminion)
                    EDOG(mtmp)->whistletime = svm.moves;
                /* Fix up a pet who is stuck "fleeing" its master */
                mon_track_clear(mtmp);
            }
        }
    }
    disturb_buried_zombies(x, y);
}

void
wake_nearto(coordxy x, coordxy y, int distance)
{
    wake_nearto_core(x, y, distance, FALSE);
}

/* NOTE: we must check for mimicry before calling this routine */
void
seemimic(struct monst *mtmp)
{
    boolean is_blocker_appear = (is_lightblocker_mappear(mtmp));

    if (has_mcorpsenm(mtmp))
        freemcorpsenm(mtmp);

    mtmp->m_ap_type = M_AP_NOTHING;
    mtmp->mappearance = 0;

    /*
     *  Discovered mimics don't block light.
     */
    if (is_blocker_appear
        && !does_block(mtmp->mx, mtmp->my, &levl[mtmp->mx][mtmp->my]))
        unblock_point(mtmp->mx, mtmp->my);

    newsym(mtmp->mx, mtmp->my);
}

/* [taken out of rescham() in order to be shared by restore_cham()] */
void
normal_shape(struct monst *mon)
{
    int mcham = (int) mon->cham;

    if (ismnum(mcham)) {
        unsigned mcan = mon->mcan;

        (void) newcham(mon, &mons[mcham], NC_SHOW_MSG);
        mon->cham = NON_PM;
        /* newcham() may uncancel a polymorphing monster; override that */
        if (mcan)
            mon->mcan = 1;
        newsym(mon->mx, mon->my);
    }
    if (is_were(mon->data) && !racial_human(mon)) {
        new_were(mon);
    }
    if (M_AP_TYPE(mon) != M_AP_NOTHING) {
        /* this used to include a cansee() check but Protection_from_
           _shape_changers shouldn't be trumped by being unseen */
        if (!mon->meating) {
            /* make revealed mimic fall asleep in lieu of shape change */
            if (M_AP_TYPE(mon) != M_AP_MONSTER)
                mon->msleeping = 1;
            seemimic(mon);
        } else {
            /* quickmimic: pet is midst of eating a mimic corpse;
               this terminates the meal early */
            finish_meating(mon);
        }
    }
}

/* freed by freedynamicdata() when game ends; doesn't need to be struct g */
static struct monst **itermonarr = NULL;
static unsigned itermonsiz = 0; /* size in 'monst *' pointers */

/* manage itermonarr; it used to be allocated and freed every time the
   monster movement loop ran; now, keep it around most of the time */
void
alloc_itermonarr(unsigned count)
{
    /* if count is 0 or bigger than itermonsiz or much smaller than
       itermonsiz, release itermonarr (and reset itermonsiz to 0) */
    if (!count || count > itermonsiz || count + 40 < itermonsiz) {
        if (itermonarr)
            free((genericptr_t) itermonarr), itermonarr = NULL;
        itermonsiz = 0;
    }
    /* when count is more than itermonsiz (including when that just
       got reset to 0), allocate a new instance of itermonarr;
       implies that count is greater than 0 */
    if (count > itermonsiz) {
        /* overallocate to reduce free/alloc-again thrashing when the
           number of monsters varies from turn to turn */
        itermonsiz = count + 20;
        itermonarr = (struct monst **) alloc(
                                        itermonsiz * sizeof (struct monst *));
    }
}

/* Iterate all monsters on the level, even dead or off-map ones, calling
   bfunc() for each monster.  If bfunc() returns TRUE, stop iterating.
   If the game ends during the call to bfunc(), then freedynamicdata()
   will free 'itermonarr'.

   Safe for list deletions and insertions, and guarantees calling bfunc()
   once per monster in fmon unless it returns TRUE (or game ends). */
void
iter_mons_safe(boolean (*bfunc)(struct monst *))
{
    struct monst *mtmp;
    unsigned i, nmons;

    for (nmons = 0, mtmp = fmon; mtmp; mtmp = mtmp->nmon)
        nmons++;

    /* make sure itermonarr[] is big enough to hold nmons entries */
    alloc_itermonarr(nmons);

    if (nmons) {
        for (i = 0, mtmp = fmon; mtmp; mtmp = mtmp->nmon)
            itermonarr[i++] = mtmp;

        for (i = 0; i < nmons; i++) {
            mtmp = itermonarr[i];
            if ((*bfunc)(mtmp))
                break;
        }
    }
    return;
}

/* iterate all living monsters on current level, calling vfunc for each. */
void
iter_mons(void (*vfunc)(struct monst *))
{
    struct monst *mtmp, *mtmp2;

    for (mtmp = fmon; mtmp; mtmp = mtmp2) {
        mtmp2 = mtmp->nmon;
        /* a monster's ridden steed is off the map for the whole time it
           is ridden but must still be iterated; its coordinates mirror
           its rider's */
        if (DEADMONSTER(mtmp) || (mon_offmap(mtmp) && !mtmp->ridden_by))
            continue;
        (*vfunc)(mtmp);
    }
    return;
}

/* iterate all living monsters on current level, calling bfunc for each.
   if bfunc returns TRUE, stop and return that monster. */
struct monst *
get_iter_mons(boolean (*bfunc)(struct monst *))
{
    struct monst *mtmp, *mtmp2;

    for (mtmp = fmon; mtmp; mtmp = mtmp2) {
        mtmp2 = mtmp->nmon;
        if (DEADMONSTER(mtmp) || (mon_offmap(mtmp) && !mtmp->ridden_by))
            continue;
        if ((*bfunc)(mtmp))
            break;
    }
    return mtmp;
}

/* iterate all living monsters on current level, calling bfunc for each,
   passing x,y to the function.
   if bfunc returns TRUE, stop and return that monster. */
struct monst *
get_iter_mons_xy(
    boolean (*bfunc)(struct monst *, coordxy, coordxy),
    coordxy x, coordxy y)
{
    struct monst *mtmp, *mtmp2;

    for (mtmp = fmon; mtmp; mtmp = mtmp2) {
        mtmp2 = mtmp->nmon;
        if (DEADMONSTER(mtmp) || (mon_offmap(mtmp) && !mtmp->ridden_by))
            continue;
        if ((*bfunc)(mtmp, x, y))
            break;
    }
    return mtmp;
}

/* Heal the given monster by amt hitpoints, unless it is somehow prevented
   from healing. "overheal" is the maximum amount by which the max HP will
   increase to allow for the healing (the resulting HP caps at max HP +
   overheal, and the max HP stays the some unless it needs to increase to
   accommodate the new HP). Overhealing the player is not currently
   implemented by this method.

   This function should only be used for situations which are conceptually
   heals, rather than other situations where a monster's HP is set, so that
   "prevent healing" effects work correctly. In particular, it should not
   be used for cases where a monster's HP is restored upon revival, or when
   a monster is created. It also shouldn't be used for lifesaving, which
   overrides "cannot heal" effects.

   amt and overheal must not be negative (0 is allowed, and a very common
   amount for overheal). Returns the number of hitpoints healed. */
int
healmon(struct monst *mtmp, int amt, int overheal)
{
    if (mtmp == &gy.youmonst) {
        int oldhp = Upolyd ? u.mh : u.uhp;
        healup(amt, 0, 0, 0);
        return (Upolyd ? u.mh : u.uhp) - oldhp;
    } else {
        int oldhp = mtmp->mhp;
        if (mtmp->mhp + amt > mtmp->mhpmax + overheal) {
            mtmp->mhpmax += overheal;
            mtmp->mhp = mtmp->mhpmax;
        } else {
            mtmp->mhp += amt;
            if (mtmp->mhp > mtmp->mhpmax)
                mtmp->mhpmax = mtmp->mhp;
        }
        return mtmp->mhp - oldhp;
    }
}

/* EvilHack. Central monster-damage router: damages mon by amount of
   type and reports whether it should have died. TODO-EVILHACK: the
   vulnerability multiplier, SPFX_HPHDAM artifact halving, monster
   spellbook-reading interruption, and hero-facing damage display
   return here with their subsystems */
boolean
damage_mon(struct monst *mon, int amount, int type, boolean by_you)
{
    /* TODO-EVILHACK: vulnerability scaling, HPHDAM artifact halving,
       and damage display port with the damage_mon core slice */
    nhUse(type);
    nhUse(by_you);
    mon->mhp -= amount;

    /* interrupt spellbook reading - being attacked breaks
       concentration */
    if (has_emsp(mon) && EMSP(mon)->msp_reading != 0) {
        EMSP(mon)->msp_reading = 0;
        EMSP(mon)->msp_read_turns = 0;
    }

    return (DEADMONSTER(mon)) ? TRUE : FALSE;
}

/* Like damage_mon() but clamps mhp to 1 so mon cannot die from the
   hit; returns FALSE always. Used for "resists the spell but still
   takes a sting" branches (power word kill and similar MR-resist
   paths) so they share the central damage logic without crossing into
   a killing blow */
boolean
damage_mon_nonlethal(struct monst *mon, int amount, int type,
                     boolean by_you)
{
    nhUse(type);
    nhUse(by_you);
    mon->mhp -= amount;
    if (mon->mhp < 1)
        mon->mhp = 1;

    if (has_emsp(mon) && EMSP(mon)->msp_reading != 0) {
        EMSP(mon)->msp_reading = 0;
        EMSP(mon)->msp_read_turns = 0;
    }

    return FALSE;
}

/* force all chameleons and mimics to become themselves and werecreatures
   to revert to human form; called when Protection_from_shape_changers gets
   activated via wearing or eating ring or via #wizintrinsic */
void
rescham(void)
{
    iter_mons(normal_shape);
}

staticfn void
m_restartcham(struct monst *mtmp)
{
    if (!mtmp->mcan)
        mtmp->cham = pm_to_cham(monsndx(mtmp->data));
    if (mtmp->data->mlet == S_MIMIC && mtmp->msleeping) {
        set_mimic_sym(mtmp);
        newsym(mtmp->mx, mtmp->my);
    }
}

/* let chameleons change and mimics hide again; called when taking off
   ring of protection from shape changers */
void
restartcham(void)
{
    iter_mons(m_restartcham);
}

/* called when restoring a monster from a saved level; protection
   against shape-changing might be different now than it was at the
   time the level was saved. */
void
restore_cham(struct monst *mon)
{
    if (Protection_from_shape_changers || mon->mcan) {
        /* force chameleon or mimic to revert to its natural shape */
        normal_shape(mon);
    } else if (mon->cham == NON_PM) {
        /* chameleon doesn't change shape here, just gets allowed to do so */
        mon->cham = pm_to_cham(monsndx(mon->data));
    }
}

/* unwatched hiders may hide again; if so, returns True */
staticfn boolean
restrap(struct monst *mtmp)
{
    struct trap *t;

    if (mtmp->mcan || M_AP_TYPE(mtmp) || cansee(mtmp->mx, mtmp->my)
        || rn2(3) || mtmp == u.ustuck
        /* can't hide while trapped except in pits */
        || (mtmp->mtrapped && (t = t_at(mtmp->mx, mtmp->my)) != 0
            && !is_pit(t->ttyp))
        /* can't hide on ceiling if there isn't one */
        || (ceiling_hider(mtmp->data) && !has_ceiling(&u.uz))
        /* won't hide when adjacent to hero */
        || (sensemon(mtmp) && m_next2u(mtmp)))
        return FALSE;

    if (mtmp->data->mlet == S_MIMIC) {
        if (mtmp->msleeping || mtmp->mfrozen) {
        /*
         * The mimic needs to be awake to disguise itself
         * as something else.
         */
            return FALSE;
        }
        set_mimic_sym(mtmp);
        return TRUE;
    } else if (levl[mtmp->mx][mtmp->my].typ == ROOM) {
        mtmp->mundetected = 1;
        return TRUE;
    }

    return FALSE;
}

/* reveal a hiding monster at x,y, either under nonexistent object,
   or an eel out of water. */
void
maybe_unhide_at(coordxy x, coordxy y)
{
    struct monst *mtmp;
    boolean undetected = FALSE, trapped = FALSE;

    if ((mtmp = m_at(x, y)) != (struct monst *) 0) {
        undetected = mtmp->mundetected;
        trapped = mtmp->mtrapped;
    } else if (u_at(x, y)) {
        mtmp = &gy.youmonst;
        undetected = u.uundetected;
        trapped = u.utrap;
    } else {
        return;
    }

    if (undetected
        && ((hides_under(mtmp->data)
             && (!OBJ_AT(x, y) || trapped
                 || !can_hide_under_obj(svl.level.objects[x][y])))
            || (mtmp->data->mlet == S_EEL && !is_pool(x, y))))
        (void) hideunder(mtmp);
}

/* monster/hero tries to hide under something at the current location;
   if used by monster creation, should only happen during level
   creation, otherwise there will be message sequencing issues */
boolean
hideunder(struct monst *mtmp)
{
    struct trap *t;
    struct obj *otmp;
    const char *seenmon = (char *) 0, *seenobj = (char *) 0,
               *locomo = (char *) 0;
    int seeit = gi.in_mklev ? 0 : canseemon(mtmp);
    boolean oldundetctd, undetected = FALSE, is_u = (mtmp == &gy.youmonst);
    coordxy x = is_u ? u.ux : mtmp->mx, y = is_u ? u.uy : mtmp->my;

    if (mtmp == u.ustuck) {
        ; /* undetected==FALSE; can't hide if holding you or held by you */
    } else if ((is_u ? u.utrap : mtmp->mtrapped)
               || ((t = t_at(x, y)) != 0 && !is_pit(t->ttyp))) {
        ; /* undetected==FALSE; can't hide while trapped or on/in/under
             any non-pit trap when not trapped */
    } else if (mtmp->data->mlet == S_EEL) {
        /* aquatic creatures only hide under water, not under objects;
           they don't do so on the Plane of Water or when hero is also
           under water unless some obstacle blocks line-of-sight */
        undetected = (is_pool(x, y) && !Is_waterlevel(&u.uz)
                      && (!Underwater || !couldsee(x, y)));
        if (seeit) {
            seenobj = "the water";
            locomo = "dive";
        }
    } else if (hides_under(mtmp->data)
               /* hider-underers only hide under objects */
               && (otmp = svl.level.objects[x][y]) != 0
               /* most things can be hidden under, but not all */
               && can_hide_under_obj(otmp)
               /* pets won't hide under a cursed item or an item of any BUC
                  state that shares a pile with one or more cursed items */
               && (!mtmp->mtame || !cursed_object_at(x, y))
               /* aquatic creatures don't reach here; other swimmers
                  shouldn't hide beneath underwater objects */
               && !is_pool_or_lava(x, y)) {
        if (seeit) /*&& (!is_pool(x, y) || (Underwater && distu(x, y) <= 2))*/
            seenobj = ansimpleoname(otmp);
        /* most monsters won't hide under a cockatrice corpse but they
           can hide under a pile containing more than just such corpses */
        if (is_u ? !Stone_resistance : !resists_ston(mtmp))
            while (otmp && otmp->otyp == CORPSE
                   && touch_petrifies(&mons[otmp->corpsenm]))
                otmp = otmp->nexthere;
        if (otmp)
            undetected = TRUE;
    }

    if (is_u) {
        oldundetctd = u.uundetected != 0;
        u.uundetected = undetected ? 1 : 0;
#if 0   /* feedback handled via #monster */
        if (undetected && !oldundeteced && seenobj)
            You("hide under %s.", seenobj);
#endif
    } else {
        if (seeit)
            seenmon = y_monnam(mtmp);
        oldundetctd = mtmp->mundetected != 0;
        mtmp->mundetected = undetected ? 1 : 0;
        /* the "you see" message won't be shown for monster hiding during
           level creation because 'seeit' will be 0 so 'seenmon' and 'seenobj'
           will be Null */
        if (undetected && seenmon && seenobj) {
            if (!locomo)
                locomo = locomotion(mtmp->data, "hide");
            set_msg_xy(mtmp->mx, mtmp->my); /* pline() will reset this */
            You_see("%s %s under %s.", seenmon, locomo, seenobj);
            iflags.last_msg = PLNMSG_HIDE_UNDER;
            gl.last_hider = mtmp->m_id;
        }
    }
    if (undetected != oldundetctd)
        newsym(x, y);
    return undetected;
}

/* called when returning to a previously visited level */
void
hide_monst(struct monst *mon)
{
    boolean hider_under = hides_under(mon->data) || mon->data->mlet == S_EEL;

    if ((is_hider(mon->data) || hider_under)
        && !(mon->mundetected || M_AP_TYPE(mon))) {
        coordxy x = mon->mx, y = mon->my;
        char save_viz = gv.viz_array[y][x];

        /* override vision, forcing hero to be unable to see monster's spot */
        gv.viz_array[y][x] &= ~(IN_SIGHT | COULD_SEE);
        if (is_hider(mon->data))
            (void) restrap(mon);
        /* try again if mimic missed its 1/3 chance to hide */
        if (mon->data->mlet == S_MIMIC && !M_AP_TYPE(mon))
            (void) restrap(mon);
        gv.viz_array[y][x] = save_viz;
        if (hider_under)
            (void) hideunder(mon);
    }
}

void
mon_animal_list(boolean construct)
{
    if (construct) {
        short animal_temp[SPECIAL_PM];
        int i, n;

        /* if (animal_list) impossible("animal_list already exists"); */

        for (n = 0, i = LOW_PM; i < SPECIAL_PM; i++)
            if (is_animal(&mons[i]))
                animal_temp[n++] = i;
        /* if (n == 0) animal_temp[n++] = NON_PM; */

        ga.animal_list = (short *) alloc(n * sizeof *ga.animal_list);
        (void) memcpy((genericptr_t) ga.animal_list,
                      (genericptr_t) animal_temp,
                      n * sizeof *ga.animal_list);
        ga.animal_list_count = n;
    } else { /* release */
        if (ga.animal_list)
            free((genericptr_t) ga.animal_list), ga.animal_list = 0;
        ga.animal_list_count = 0;
    }
}

staticfn int
pick_animal(void)
{
    int res;

    if (!ga.animal_list)
        mon_animal_list(TRUE);
    assert(ga.animal_list != 0);
    res = ga.animal_list[rn2(ga.animal_list_count)];
    /* rogue level should use monsters represented by uppercase letters
       only, but since chameleons aren't generated there (not uppercase!)
       we don't perform a lot of retries */
    if (Is_rogue_level(&u.uz) && !isupper((int) monsym(&mons[res])))
        res = ga.animal_list[rn2(ga.animal_list_count)];
    return res;
}

void
decide_to_shapeshift(struct monst *mon)
{
    struct permonst *ptr = 0;
    int mndx;
    unsigned was_female = mon->female;
    boolean dochng = FALSE;

    if (!is_vampshifter(mon)) {
        /* regular shapeshifter; 'ptr' is Null */
        if (!mon->mspec_used && !rn2(6)) {
            dochng = TRUE;
            mon->mspec_used = 3 + rn2(10);
        }
    } else if (!(mon->mstrategy & STRAT_WAITFORU)) {
        /* The vampire has to be in good health (mhp) to maintain
         * its shifted form.
         *
         * If we're shifted and getting low on hp, maybe shift back, or
         * if we're a fog cloud at full hp, maybe pick a different shape.
         * If we're not already shifted and in good health, maybe shift.
         */
        if (mon->data->mlet != S_VAMPIRE) {
            if ((mon->mhp <= (mon->mhpmax + 5) / 6) && rn2(4)
                && ismnum(mon->cham)) {
                ptr = &mons[mon->cham];
                dochng = TRUE;
            } else if (mon->data == &mons[PM_FOG_CLOUD]
                       && mon->mhp == mon->mhpmax && !rn2(4)
                       && (!canseemon(mon)
                           || mdistu(mon) > BOLT_LIM * BOLT_LIM)) {
                /* if a fog cloud, maybe change to wolf or vampire bat;
                   those are more likely to take damage--at least when
                   tame--and then switch back to vampire; they'll also
                   switch to fog cloud if they encounter a closed door */
                mndx = pickvampshape(mon);
                if (ismnum(mndx)) {
                    ptr = &mons[mndx];
                    dochng = (ptr != mon->data);
                }
            }
            if (dochng && amorphous(mon->data)
                && closed_door(mon->mx, mon->my)) {
                coord new_xy;

                if (enexto(&new_xy, mon->mx, mon->my, ptr)) {
                    rloc_to(mon, new_xy.x, new_xy.y);
                }
            }
        } else {
            if (mon->mhp >= 9 * mon->mhpmax / 10 && !rn2(6)
                && (!canseemon(mon)
                    || mdistu(mon) > BOLT_LIM * BOLT_LIM))
                dochng = TRUE; /* 'ptr' stays Null */
        }
    }
    if (dochng) {
        if (newcham(mon, ptr, NC_SHOW_MSG)) {
            /* for vampshift, override the 10% chance for sex change
               (by forcing original gender in case that occurred) */
            if (is_vampshifter(mon)) {
                ptr = mon->data;
                if (!is_male(ptr) && !is_female(ptr) && !is_neuter(ptr))
                    mon->female = was_female;
            }
        }
    }
}

staticfn int
pickvampshape(struct monst *mon)
{
    int mndx = mon->cham, wolfchance = 10;
    /* avoid picking monsters with lowercase display symbols ('d' for wolf
       and 'v' for fog cloud) on rogue level*/
    boolean uppercase_only = Is_rogue_level(&u.uz);

    switch (mndx) {
    case PM_VLAD_THE_IMPALER:
        /* ensure Vlad can keep carrying the Candelabrum */
        if (mon_has_special(mon))
            break; /* leave mndx as is */
        wolfchance = 3;
        FALLTHROUGH;
    /*FALLTHRU*/
    case PM_VAMPIRE_NOBLE: /* vampire lord or Vlad can become wolf */
        if (!rn2(wolfchance) && !uppercase_only
            /* don't pick a walking form if that would lead to immediate
               drowning or immolation and reversion to vampire form */
            && !is_pool_or_lava(mon->mx, mon->my)) {
            mndx = PM_WOLF;
            break;
        }
        FALLTHROUGH;
    /*FALLTHRU*/
    case PM_VAMPIRE: /* any vampire can become fog or bat */
        mndx = (!rn2(4) && !uppercase_only) ? PM_FOG_CLOUD : PM_VAMPIRE_BAT;
        break;
    }

    /* return to base form if chosen poly target has been genocided
       or randomly if already in an alternate form (to prevent always
       switching back and forth between bat and fog) */
    if ((svm.mvitals[mndx].mvflags & G_GENOD) != 0
        || (mon->data != &mons[mon->cham] && !rn2(4)))
        return mon->cham;

    return mndx;
}

/* nonshapechangers who warrant special polymorph handling */
staticfn boolean
isspecmon(struct monst *mon)
{
    return (mon->isshk || mon->ispriest || mon->isgd
            || mon->m_id == svq.quest_status.leader_m_id);
}

/* restrict certain special monsters (shopkeepers, aligned priests,
   vault guards) to forms that allow them to behave sensibly (catching
   gold, speaking?) so that they don't need too much extra code */
staticfn boolean
validspecmon(struct monst *mon, int mndx)
{
    if (mndx == NON_PM)
        return TRUE; /* caller wants random */

    if (!accept_newcham_form(mon, mndx))
        return FALSE; /* geno'd or !polyok */

    if (isspecmon(mon)) {
        struct permonst *ptr = &mons[mndx];

        /* reject notake because object manipulation is expected
           and nohead because speech capability is expected */
        if (notake(ptr) || !has_head(ptr))
            return FALSE;
        /* [should we check ptr->msound here too?] */
    }
    return TRUE; /* potential new form is ok */
}

/* used for hero polyself handling */
boolean
valid_vampshiftform(int base, int form)
{
    if (base >= LOW_PM && is_vampire(&mons[base])) {
        if (form == PM_VAMPIRE_BAT || form == PM_FOG_CLOUD
            || (form == PM_WOLF && base != PM_VAMPIRE))
            return TRUE;
    }
    return FALSE;
}

/* prevent wizard mode user from specifying invalid vampshifter shape
   when using monpolycontrol to assign a new form to a vampshifter */
boolean
validvamp(struct monst *mon, int *mndx_p, int monclass)
{
    /* simplify caller's usage */
    if (!is_vampshifter(mon))
        return validspecmon(mon, *mndx_p);

    if (mon->cham == PM_VLAD_THE_IMPALER && mon_has_special(mon)) {
        /* Vlad with Candelabrum; override choice, then accept it */
        *mndx_p = PM_VLAD_THE_IMPALER;
        return TRUE;
    }
    if (ismnum(*mndx_p) && is_shapeshifter(&mons[*mndx_p])) {
        /* player picked some type of shapeshifter; use mon's self
           (vampire or chameleon) */
        *mndx_p = mon->cham;
        return TRUE;
    }
    /* basic vampires can't become wolves; any can become fog or bat
       (we don't enforce upper-case only for rogue level here) */
    if (*mndx_p == PM_WOLF)
        return (boolean) (mon->cham != PM_VAMPIRE);
    if (*mndx_p == PM_FOG_CLOUD || *mndx_p == PM_VAMPIRE_BAT)
        return TRUE;

    /* if we get here, specific type was no good; try by class */
    switch (monclass) {
    case S_VAMPIRE:
        *mndx_p = mon->cham;
        break;
    case S_BAT:
        *mndx_p = PM_VAMPIRE_BAT;
        break;
    case S_VORTEX:
        *mndx_p = PM_FOG_CLOUD;
        break;
    case S_DOG:
        if (mon->cham != PM_VAMPIRE) {
            *mndx_p = PM_WOLF;
            break;
        }
        FALLTHROUGH;
        /*FALLTHRU*/
    default:
        *mndx_p = NON_PM;
        break;
    }
    return (boolean) (*mndx_p != NON_PM);
}

staticfn int
wiz_force_cham_form(struct monst *mon)
{
    char pprompt[BUFSZ], parttwo[QBUFSZ], buf[BUFSZ], prevbuf[BUFSZ];
    int monclass, len, tryct, mndx = NON_PM;

    /* construct prompt in pieces */
    Sprintf(pprompt, "Change %s", noit_mon_nam(mon));
    Sprintf(parttwo, " @ %s into what?",
            coord_desc((int) mon->mx, (int) mon->my, buf,
                       (iflags.getpos_coords != GPCOORDS_NONE)
                       ? iflags.getpos_coords : GPCOORDS_MAP));
    /* combine the two parts, not exceeding QBUFSZ-1 in overall length;
       if combined length is too long it has to be due to monster's
       name so we'll chop enough of that off to fit the second part */
    if ((len = (int) strlen(pprompt) + (int) strlen(parttwo)) >= QBUFSZ)
        /* strlen(parttwo) is less than QBUFSZ/2 so strlen(pprompt) is
           more than QBUFSZ/2 and excess amount being truncated can't
           exceed pprompt's length and back up to before &pprompt[0]) */
        *(eos(pprompt) - (len - (QBUFSZ - 1))) = '\0';
    Strcat(pprompt, parttwo);

    buf[0] = prevbuf[0] = '\0'; /* clear buffer for EDIT_GETLIN */
#define TRYLIMIT 5
    tryct = TRYLIMIT;
    do {
        if (tryct == TRYLIMIT - 1) { /* first retry */
            /* change "into what?" to "into what kind of monster?" */
            if (strlen(pprompt) + sizeof " kind of monster" - 1 < QBUFSZ)
                Strcpy(eos(pprompt) - 1, " kind of monster?");
        }
#undef TRYLIMIT
        monclass = 0;
        getlin(pprompt, buf);
        mungspaces(buf);
        /* for ESC, take form selected above (might be NON_PM) */
        if (*buf == '\033')
            break;
        /* for "*", use NON_PM to pick an arbitrary shape below */
        if (!strcmp(buf, "*") || !strcmpi(buf, "random")) {
            mndx = NON_PM;
            break;
        }
        mndx = name_to_mon(buf, (int *) 0);
        if (mndx == NON_PM) {
            /* didn't get a type, so check whether it's a class
               (single letter or text match with def_monsyms[]) */
            monclass = name_to_monclass(buf, &mndx);
            if (monclass && mndx == NON_PM)
                mndx = mkclass_poly(monclass);
        }
        if (ismnum(mndx)) {
            /* got a specific type of monster; use it if we can */
            if (validvamp(mon, &mndx, monclass))
                break;
            /* can't; revert to random in case we exhaust tryct */
            mndx = NON_PM;
        }

        pline("It can't become that.");
#ifdef EDIT_GETLIN
        /* EDIT_GETLIN preloads the input buffer with the previous
           response but we shouldn't just keep repeating that if player
           leaves it unchanged; affects retry for empty input too */
        if (!strcmp(buf, prevbuf))
            Strcpy(buf, "random");
        Strcpy(prevbuf, buf);
#else
        nhUse(prevbuf);
#endif
    } while (--tryct > 0);

    if (!tryct)
        pline1(thats_enough_tries);
    if (is_vampshifter(mon) && !validvamp(mon, &mndx, monclass))
        mndx = pickvampshape(mon); /* don't resort to arbitrary */
    return mndx;
}

int
select_newcham_form(struct monst *mon)
{
    int mndx = NON_PM, tryct;

    switch (mon->cham) {
    case PM_SANDESTIN:
        if (rn2(7))
            mndx = pick_nasty(mons[PM_ARCHON].difficulty - 1);
        break;
    case PM_DOPPELGANGER:
        if (!rn2(7)) {
            mndx = pick_nasty(mons[PM_JABBERWOCK].difficulty - 1);
        } else if (rn2(3)) { /* role monsters */
            mndx = tt_doppel(mon);
        } else if (!rn2(3)) { /* quest guardians */
            mndx = rn1(PM_APPRENTICE - PM_STUDENT + 1, PM_STUDENT);
            /* avoid own role's guardian */
            if (mndx == gu.urole.guardnum)
                mndx = NON_PM;
        } else { /* general humanoids */
            tryct = 5;
            do {
                mndx = rn1(SPECIAL_PM - LOW_PM, LOW_PM);
                /* assert(ismnum(mndx)); */
                if (humanoid(&mons[mndx]) && polyok(&mons[mndx]))
                    break;
            } while (--tryct > 0);
            if (!tryct)
                mndx = NON_PM;
        }
        break;
    case PM_CHAMELEON:
        if (!rn2(3))
            mndx = pick_animal();
        break;
    case PM_VLAD_THE_IMPALER:
    case PM_VAMPIRE_NOBLE:
    case PM_VAMPIRE:
        mndx = pickvampshape(mon);
        break;
    case NON_PM: /* ordinary */
      {
        struct obj *m_armr = which_armor(mon, W_ARM);

        if (m_armr && Is_dragon_armor(m_armr))
            mndx = Dragon_armor_to_pm(m_armr);
      }
        break;
    }

    /* for debugging: allow control of polymorphed monster */
    if (wizard && iflags.mon_polycontrol)
        mndx = wiz_force_cham_form(mon);

    /* if no form was specified above, pick one at random now */
    if (mndx == NON_PM) {
        tryct = 50;
        do {
            mndx = rn1(SPECIAL_PM - LOW_PM, LOW_PM);
            /* assert(ismnum(mndx); */
        } while (--tryct > 0 && !validspecmon(mon, mndx)
                 /* try harder to select uppercase monster on rogue level */
                 && (tryct > 40 && Is_rogue_level(&u.uz)
                     && !isupper((int) monsym(&mons[mndx]))));
    }
    return mndx;
}

/* this used to be inline within newcham() but monpolycontrol needs it too */
staticfn struct permonst *
accept_newcham_form(struct monst *mon, int mndx)
{
    struct permonst *mdat;

    if (mndx == NON_PM)
        return 0;
    mdat = &mons[mndx];
    if ((svm.mvitals[mndx].mvflags & G_GENOD) != 0)
        return 0;
    if (is_placeholder(mdat))
        return 0;
    /* select_newcham_form() might deliberately pick a player
       character type (random selection never does) which
       polyok() rejects, so we need a special case here */
    if (is_mplayer(mdat))
        return mdat;
    /* shapeshifters are rejected by polyok() but allow a shapeshifter
       to take on its 'natural' form */
    if (is_shapeshifter(mdat)
        && ismnum(mon->cham) && mdat == &mons[mon->cham])
        return mdat;
    /* polyok() rules out M2_PNAME, MH_WERE, and all humans except Kops */
    return polyok(mdat) ? mdat : 0;
}

/* shapechanger might take on a shape that forces gender change */
void
mgender_from_permonst(
    struct monst *mtmp,
    struct permonst *mdat)
{
    if (is_male(mdat)) {
        mtmp->female = FALSE;
    } else if (is_female(mdat)) {
        mtmp->female = TRUE;
    } else if (!is_neuter(mdat)) {
        /* usually leave as-is; same chance to change as polymorphing hero;
           vampires use controlled shapechange (from their perspective, even
           if it is random from the player's perspective) and don't undergo
           gender change */
        if (!rn2(10) && !(is_vampire(mdat) || is_vampshifter(mtmp)))
            mtmp->female = !mtmp->female;
    }
}

/* make a chameleon take on another shape, or a polymorph target
   (possibly self-inflicted) become a different monster;
   returns 1 if it actually changes form */
int
newcham(
    struct monst *mtmp,
    struct permonst *mdat,
    unsigned ncflags)
{
    boolean polyspot = ((ncflags & NC_VIA_WAND_OR_SPELL) !=0),
            /* "The oldmon turns into a newmon!" */
            msg = ((ncflags & NC_SHOW_MSG) != 0),
            seenorsensed = canspotmon(mtmp);
    int hpn, hpd, mndx, tryct;
    struct permonst *olddata = mtmp->data;
    /* oldmhe holds a pronoun; 5.0's genders[] has a fourth "they" row
       that a hallucinating hero can draw, so three letters is not the
       most this can hold */
    char *p, oldname[BUFSZ], l_oldname[BUFSZ], oldmhe[QBUFSZ];

    /* Riders are immune to polymorph and green slime
       (but apparent Rider might actually be a doppelganger) */
    if (mtmp->cham == NON_PM) { /* not a shapechanger */
        if (is_rider(olddata))
            return 0;
        /* make Nazgul and erinyes immune too, to reduce chance of
           anomalous extinction feedback during final disclosure */
        if (mbirth_limit(monsndx(olddata)) < MAXMONNO)
            return 0;
        /* cancelled shapechangers become uncancelled prior
           to being given a new shape */
        if (mtmp->mcan && !Protection_from_shape_changers) {
            mtmp->cham = pm_to_cham(monsndx(mtmp->data));
            if (mtmp->cham != NON_PM)
                mtmp->mcan = 0;
        }
    }

    if (msg) {
        Strcpy(oldname,
               /* like YMonnam() but never mention saddle or barding */
               x_monnam(mtmp, mtmp->mtame ? ARTICLE_YOUR : ARTICLE_THE,
                        (char *) 0, (SUPPRESS_SADDLE | SUPPRESS_BARDING),
                        FALSE));
        oldname[0] = highc(oldname[0]);
    }
    /* we need this one whether msg is true or not */
    Strcpy(l_oldname, x_monnam(mtmp, ARTICLE_THE, (char *) 0,
                               has_mgivenname(mtmp)
                                 ? (SUPPRESS_SADDLE | SUPPRESS_BARDING) : 0,
                               FALSE));
    /* pronoun for the pre-transformation shape, for a rider's benefit */
    Strcpy(oldmhe, mhe(mtmp));

    /* mdat = 0 -> caller wants a random monster shape */
    if (mdat == 0) {
        /* select_newcham_form() loops when resorting to random but
           it doesn't always pick that so we still retry here too */
        tryct = 20;
        do {
            mndx = select_newcham_form(mtmp);
            mdat = accept_newcham_form(mtmp, mndx);
            /* for the first several tries we require upper-case on
               the rogue level (after that, we take whatever we get) */
            if (tryct > 15 && Is_rogue_level(&u.uz)
                && mdat && !isupper((int) monsym(mdat)))
                mdat = 0;
            if (mdat)
                break;
        } while (--tryct > 0);
        if (!tryct)
            return 0;
    } else if (svm.mvitals[monsndx(mdat)].mvflags & G_GENOD)
        return 0; /* passed in mdat is genocided */

    if (mdat == olddata)
        return 0; /* still the same monster */

    mgender_from_permonst(mtmp, mdat);
    /* Endgame mplayers start out as "Foo the Bar", but some of the
     * titles are inappropriate when polymorphed, particularly into
     * the opposite sex.  Player characters don't use ranks when
     * polymorphed, so dropping rank for mplayers seems reasonable.
     */
    if (In_endgame(&u.uz) && is_mplayer(olddata)
        && has_mgivenname(mtmp)
        && (p = strstr(MGIVENNAME(mtmp), " the ")) != 0)
        *p = '\0';

    /* ditch the old monster's race if it exists */
    if (has_erac(mtmp))
        free_erac(mtmp);

    if (mtmp->wormno) { /* throw tail away */
        coordxy mx = mtmp->mx, my = mtmp->my;

        wormgone(mtmp); /* discards tail segments, takes head off the map */
        /* put the head back; it will morph into mtmp's new form */
        place_monster(mtmp, mx, my);
    }
    if (M_AP_TYPE(mtmp) && mdat->mlet != S_MIMIC)
        seemimic(mtmp); /* revert to normal monster */

    /* (this code used to try to adjust the monster's health based on
       a normal one of its type but there are too many special cases
       which need to be handled in order to do that correctly, so just
       give the new form the same proportion of HP as its old one had) */
    hpn = mtmp->mhp;
    hpd = mtmp->mhpmax;
    /* set level and hit points */
    newmonhp(mtmp, monsndx(mdat));
    /* new hp: same fraction of max as before */
    mtmp->mhp = (int) (((long) hpn * (long) mtmp->mhp) / (long) hpd);
    /* sanity check (potential overflow) */
    if (mtmp->mhp < 0 || mtmp->mhp > mtmp->mhpmax)
        mtmp->mhp = mtmp->mhpmax;
    /* unlikely but not impossible; a 1HD creature with 1HP that changes
       into a 0HD creature will require this statement */
    if (!mtmp->mhp)
        mtmp->mhp = 1;

    /* take on the new form... */
    set_mon_data(mtmp, mdat);

    /* a skeletal dragon's breath is fixed per individual; assign it
       (and its matching self-immunity) now so the resistance is in
       place before it ever breathes */
    if (mdat == &mons[PM_SKELETAL_DRAGON])
        sd_assign_breath(mtmp);

    /* add a new random race if polymorphing into a racial monster */
    if (is_racialmon(mdat))
        apply_race(mtmp, m_randrace(mtmp->mnum));

    if (mtmp->mleashed) {
        if (!leashable(mtmp))
            m_unleash(mtmp, TRUE);
        else
            /* if leashed, persistent inventory window needs updating
               (really only when mon_nam() is going to yield "a frog"
               rather than "Kermit" but no need to micromanage here) */
            update_inventory(); /* x - leash (attached to a <mon>) */
    }

    if (emits_light(olddata) != emits_light(mtmp->data)) {
        /* used to give light, now doesn't, or vice versa,
           or light's range has changed */
        if (emits_light(olddata))
            del_light_source(LS_MONSTER, monst_to_any(mtmp));
        if (emits_light(mtmp->data))
            new_light_source(mtmp->mx, mtmp->my, emits_light(mtmp->data),
                             LS_MONSTER, monst_to_any(mtmp));
    }
    if (!mtmp->perminvis || pm_invisible(olddata))
        mtmp->perminvis = pm_invisible(mdat);
    mtmp->minvis = mtmp->invis_blkd ? 0 : mtmp->perminvis;

    /* the new shape may be unfit to ride, or unfit to be ridden */
    if (has_erid(mtmp) && !mon_can_ride(mtmp)) {
        if (canseemon(mtmp))
            pline("Transforming, %s falls from %s!", l_oldname,
                  mon_nam(ERID(mtmp)->mon_steed));
        separate_steed_and_rider(mtmp);
        /* the rider is relocated to get off its steed, and that can put
           it in a trap or hand it a turn of its own */
        if (DEADMONSTER(mtmp))
            return 1;
    }
    if (mtmp->ridden_by && !mon_can_be_ridden(mtmp) && mtmp != u.usteed) {
        struct monst *rider = get_mon_rider(mtmp);

        if (rider) {
            if (canseemon(rider))
                pline("%s falls from %s mount as %s transforms!",
                      Monnam(rider), mhis(rider), oldmhe);
            separate_steed_and_rider(rider);
        } else {
            /* the rider is gone but the steed is still flagged as ridden;
               it was taken off the map when mounted, so put it back as a
               free monster before clearing the stale flag - otherwise it
               lingers in fmon off the map, invisible yet still able to
               act, until a save/restore re-places it */
            if (m_at(mtmp->mx, mtmp->my) != mtmp) {
                coord cc;

                if (!m_at(mtmp->mx, mtmp->my) && isok(mtmp->mx, mtmp->my))
                    place_monster(mtmp, mtmp->mx, mtmp->my);
                else if (enexto_core_mon(&cc, mtmp->mx, mtmp->my, mtmp,
                                         NO_MM_FLAGS))
                    place_monster(mtmp, cc.x, cc.y);
                else
                    (void) rloc(mtmp, RLOC_ERR | RLOC_NOMSG);
                newsym(mtmp->mx, mtmp->my);
            }
            mtmp->ridden_by = 0;
        }
    }

    if (mtmp->mundetected)
        (void) hideunder(mtmp);
    if (u.ustuck == mtmp) {
        if (u.uswallow) {
            if (!attacktype(mdat, AT_ENGL)) {
                /* Does mdat care? */
                if (!noncorporeal(mdat) && !is_whirly(mdat)
                    && !(amorphous(mdat) || mdat->mlet == S_LIGHT)) {
                    char msgtrail[BUFSZ];

                    if (is_vampshifter(mtmp)) {
                        Sprintf(msgtrail, " which was a shapeshifted %s",
                                noname_monnam(mtmp, ARTICLE_NONE));
                    } else if (digests(mdat)) {
                        Strcpy(msgtrail, "'s stomach");
                    } else {
                        msgtrail[0] = '\0';
                    }
                    /* Do this even if msg is FALSE */
                    You("%s %s%s!",
                        (amorphous(olddata) || is_whirly(olddata))
                            ? "emerge from" : "break out of",
                        l_oldname, msgtrail);
                    msg = FALSE; /* message has been given */
                    mtmp->mhp = 1; /* almost dead */
                }
                expels(mtmp, olddata, FALSE);
            } else {
                /* update swallow glyphs for new monster */
                swallowed(0);
            }
        } else if ((!sticks(mdat) && !sticks(gy.youmonst.data))
                   /* sticky hero can't continue to hold mtmp if it has
                      turned into a non-solid creature; we don't use
                      uunstick() for that because its message would be
                      shown out of sequence [before 'if (msg)' below];
                      unstuck() doesn't issue any messages */
                   || unsolid(mdat)) {
            unstuck(mtmp);
        }
    }

    if (mdat == &mons[PM_LONG_WORM] && (mtmp->wormno = get_wormno()) != 0) {
        initworm(mtmp, rn2(5));
        place_worm_tail_randomly(mtmp, mtmp->mx, mtmp->my);
    }

    /* a shapeshifter that takes on a form the hero innately grudges
       (Knight vs cross-aligned dragon, aasimar vs demon, Infidel vs
       lawful angel) must turn hostile the moment it transforms, just
       as makemon() spawns such a monster hostile. Otherwise it
       lingers peaceful until the per-turn grudge pass in moveloop()
       flips it via setmangry(), charging the hero an alignment-abuse
       penalty for a transformation the hero never caused. Only
       peaceful non-tame forms incur that penalty, so only those are
       handled here; tame forms are already flipped penalty-free by
       the moveloop pass */
    if (mtmp->mpeaceful && !mtmp->mtame
        && ((Role_if(PM_KNIGHT) && is_dragon(mdat)
             && sgn(mon_aligntyp(mtmp)) != u.ualign.type)
            || (Race_if(PM_AASIMAR) && is_demon(mdat))
            || (Role_if(PM_INFIDEL) && is_angel(mdat)
                && mon_aligntyp(mtmp) > 0))) {
        mtmp->mpeaceful = 0;
        set_malign(mtmp);
    }

    mtmp->meverseen = 0; /* never seen mon in present shape; newsym() ->
                          * display_monster() may change it right back */
    newsym(mtmp->mx, mtmp->my);

    if (msg) {
        /* oldname is capitalized and might be an assigned name */
        if (!canspotmon(mtmp)) { /* can't see or sense it now */
            if (seenorsensed) /* could see or sense it before */
                pline_mon(mtmp, "%s disappears!", oldname);
            (void) usmellmon(mdat);
        } else if (!seenorsensed) { /* couldn't see/sense before, can now */
            char *mnm = x_monnam(mtmp, mtmp->mtame ? ARTICLE_YOUR : ARTICLE_A,
                                 (char *) 0, 0, FALSE);

            pline_mon(mtmp, "%s appears!", upstart(mnm));
        } else { /* saw/sensed it before, still see/sense it now */
            pline_mon(mtmp, "%s turns into %s!", oldname,
                      /* "a <monster type>" even if it has a name assigned */
                      noname_monnam(mtmp, ARTICLE_A));
        }
    }

    /* when polymorph trap/wand/potion produces a vampire, turn in into
       a full-fledged vampshifter unless shape-changing is blocked */
    if (mtmp->cham == NON_PM && mdat->mlet == S_VAMPIRE
        && !Protection_from_shape_changers)
        mtmp->cham = pm_to_cham(monsndx(mdat));

    possibly_unwield(mtmp, polyspot); /* might lose use of weapon */
    mon_break_armor(mtmp, polyspot);
    if (!(mtmp->misc_worn_check & W_ARMG))
        mselftouch(mtmp, "No longer petrify-resistant, ",
                   !svc.context.mon_moving);
    check_gear_next_turn(mtmp);

    /* This ought to re-test can_carry() on each item in the inventory
     * rather than just checking ex-giants & boulders, but that'd be
     * pretty expensive to perform.  If implemented, then perhaps
     * minvent should be sorted in order to drop heaviest items first.
     */
    /* former giants can't continue carrying boulders */
    if (mtmp->minvent && !racial_throws_rocks(mtmp)) {
        struct obj *otmp, *otmp2;

        /* DEADMONSTER(): it is possible for flooreffects() to kill mtmp;
           the rest of its inventory would be dropped making otmp2 stale */
        for (otmp = mtmp->minvent; otmp && !DEADMONSTER(mtmp); otmp = otmp2) {
            otmp2 = otmp->nobj;
            if (otmp->otyp == BOULDER) {
                /* this keeps otmp from being polymorphed in the
                   same zap that the monster that held it is polymorphed */
                if (polyspot)
                    bypass_obj(otmp);
                obj_extract_self(otmp);
                /* probably ought to give some "drop" message here */
                if (flooreffects(otmp, mtmp->mx, mtmp->my, ""))
                    continue;
                place_object(otmp, mtmp->mx, mtmp->my);
            }
        }
    }
    if (mtmp == u.usteed)
        poly_steed(mtmp, olddata);

    /* old form might not have been affected by Elbereth but perhaps the
       new form is */
    if (svc.context.mon_moving) {
        /* give 'mtmp' a new chance to pinpoint hero's location */
        if (!u_at(mtmp->mux, mtmp->muy))
            set_apparxy(mtmp);
        /* if hero is on Elbereth or scare monster, mtmp in new form might
           become scared */
        if (!mtmp->mpeaceful
            && onscary(mtmp->mux, mtmp->muy, mtmp)
            && monnear(mtmp, mtmp->mux, mtmp->muy))
            monflee(mtmp, rn1(9, 2), TRUE, TRUE); /* 2..10 turns */
    }

    return 1;
}

/* sometimes an egg will be special */
#define BREEDER_EGG (!rn2(77))

/*
 * Determine if the given monster number can be hatched from an egg.
 * Return the monster number to use as the egg's corpsenm.  Return
 * NON_PM if the given monster can't be hatched.
 */
int
can_be_hatched(int mnum)
{
    /* ranger quest nemesis has the oviparous bit set, making it
       be possible to wish for eggs of that unique monster; turn
       such into ordinary eggs rather than forbidding them outright */
    if (mnum == PM_SCORPIUS)
        mnum = PM_SCORPION;

    mnum = little_to_big(mnum);
    /*
     * Queen bees lay killer bee eggs (usually), but killer bees don't
     * grow into queen bees.  Ditto for [winged-]gargoyles.
     */
    if (mnum == PM_KILLER_BEE || mnum == PM_GARGOYLE
        || (lays_eggs(&mons[mnum])
            && (BREEDER_EGG
                || (mnum != PM_QUEEN_BEE && mnum != PM_WINGED_GARGOYLE))))
        return mnum;
    return NON_PM;
}

/* type of egg laid by #sit; usually matches parent */
int
egg_type_from_parent(
    int mnum, /* parent monster; caller must handle lays_eggs() check */
    boolean force_ordinary)
{
    if (force_ordinary || !BREEDER_EGG) {
        if (mnum == PM_QUEEN_BEE)
            mnum = PM_KILLER_BEE;
        else if (mnum == PM_WINGED_GARGOYLE)
            mnum = PM_GARGOYLE;
    }
    return mnum;
}

#undef BREEDER_EGG

/* decide whether an egg of the indicated monster type is viable;
   also used to determine whether an egg or tin can be created... */
boolean
dead_species(int m_idx, boolean egg)
{
    int alt_idx;

    /* generic eggs are unhatchable and have corpsenm of NON_PM */
    if (m_idx < LOW_PM)
        return TRUE;
    /*
     * For monsters with both baby and adult forms, genociding either
     * form kills all eggs of that monster.  Monsters with more than
     * two forms (small->large->giant mimics) are more or less ignored;
     * fortunately, none of them have eggs.  Species extinction due to
     * overpopulation does not kill eggs.
     */
    /* assert(ismnum(m_idx)); */
    alt_idx = egg ? big_to_little(m_idx) : m_idx;
    return (boolean) ((svm.mvitals[m_idx].mvflags & G_GENOD) != 0
                      || (svm.mvitals[alt_idx].mvflags & G_GENOD) != 0);
}

/* kill off any eggs of genocided monsters */
staticfn void
kill_eggs(struct obj *obj_list)
{
    struct obj *otmp;

    for (otmp = obj_list; otmp; otmp = otmp->nobj)
        if (otmp->otyp == EGG) {
            if (dead_species(otmp->corpsenm, TRUE)) {
                /*
                 * It seems we could also just catch this when
                 * it attempted to hatch, so we wouldn't have to
                 * search all of the objlists.. or stop all
                 * hatch timers based on a corpsenm.
                 */
                kill_egg(otmp);
            }
#if 0 /* not used */
        } else if (otmp->otyp == TIN) {
            if (dead_species(otmp->corpsenm, FALSE))
                otmp->corpsenm = NON_PM; /* empty tin */
        } else if (otmp->otyp == CORPSE) {
            if (dead_species(otmp->corpsenm, FALSE))
                ; /* not yet implemented... */
#endif
        } else if (Has_contents(otmp)) {
            kill_eggs(otmp->cobj);
        }
}

/* kill all members of genocided species */
void
kill_genocided_monsters(void)
{
    struct monst *mtmp, *mtmp2;
    boolean kill_cham;
    int mndx;

    /*
     * Called during genocide, and again upon level change.  The latter
     * catches up with any migrating monsters as they finally arrive at
     * their intended destinations, so possessions get deposited there.
     *
     * Chameleon handling:
     *  1) if chameleons have been genocided, destroy them
     *     regardless of current form;
     *  2) otherwise, force every chameleon which is imitating
     *     any genocided species to take on a new form.
     */
    for (mtmp = fmon; mtmp; mtmp = mtmp2) {
        mtmp2 = mtmp->nmon;
        if (DEADMONSTER(mtmp))
            continue;
        mndx = monsndx(mtmp->data);
        kill_cham = (ismnum(mtmp->cham)
                     && (svm.mvitals[mtmp->cham].mvflags & G_GENOD));
        if ((svm.mvitals[mndx].mvflags & G_GENOD) || kill_cham) {
            if (ismnum(mtmp->cham) && !kill_cham)
                (void) newcham(mtmp, (struct permonst *) 0, NC_SHOW_MSG);
            else
                mondead(mtmp);
        }
        if (mtmp->minvent)
            kill_eggs(mtmp->minvent);
    }

    kill_eggs(gi.invent);
    kill_eggs(fobj);
    kill_eggs(gm.migrating_objs);
    kill_eggs(gm.mchest);
    kill_eggs(svl.level.buriedobjlist);
}

void
golemeffects(struct monst *mon, int damtype, int dam)
{
    int heal = 0, slow = 0;

    if (mon->data == &mons[PM_FLESH_GOLEM]) {
        if (damtype == AD_ELEC)
            heal = (dam + 5) / 6;
        else if (damtype == AD_FIRE || damtype == AD_COLD)
            slow = 1;
    } else if (mon->data == &mons[PM_IRON_GOLEM]) {
        if (damtype == AD_ELEC)
            slow = 1;
        else if (damtype == AD_FIRE)
            heal = dam;
    } else {
        return;
    }
    if (slow) {
        if (mon->mspeed != MSLOW)
            mon_adjust_speed(mon, -1, (struct obj *) 0);
    }
    if (heal) {
        if (healmon(mon, heal, 0)) {
            if (cansee(mon->mx, mon->my))
                pline_mon(mon, "%s seems healthier.", Monnam(mon));
        }
    }
}

/* anger the Minetown watch */
boolean
angry_guards(boolean silent)
{
    struct monst *mtmp;
    int ct = 0, nct = 0, sct = 0, slct = 0;

    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp))
            continue;
        if (is_watch(mtmp->data) && mtmp->mpeaceful) {
            ct++;
            if (canspotmon(mtmp) && mtmp->mcanmove) {
                if (m_next2u(mtmp))
                    nct++;
                else
                    sct++;
            }
            if (mtmp->msleeping || mtmp->mfrozen) {
                slct++;
                mtmp->msleeping = mtmp->mfrozen = 0;
            }
            mtmp->mpeaceful = 0;
        }
    }
    if (ct) {
        if (!silent) { /* do we want pline msgs? */
            char buf[BUFSZ];

            if (slct) { /* sleeping guard(s) */
                Sprintf(buf, "guard%s", plur(slct));
                pline_The("%s %s up.", buf, vtense(buf, "wake"));
            }

            if (nct) { /* seen/sensed adjacent guard(s) */
                Sprintf(buf, "guard%s", plur(nct));
                pline_The("%s %s angry!", buf, vtense(buf, "get"));
            } else if (sct) { /* seen/sensed non-adjacent guard(s) */
                Sprintf(buf, "guard%s", plur(sct));
                pline("%s %s %s approaching!",
                      (sct == 1) ? "An angry" : "Angry",
                      buf, vtense(buf, "are"));
            } else {
                Strcpy(buf, (ct == 1) ? "a guard's" : "guards'");
                Soundeffect(se_shrill_whistle, 100);
                You_hear("the shrill sound of %s whistle%s.", buf, plur(ct));
            }
        }
        return TRUE;
    }
    return FALSE;
}

staticfn void
pacify_guard(struct monst *mtmp)
{
    if (is_watch(mtmp->data))
        mtmp->mpeaceful = 1;
}

void
pacify_guards(void)
{
    iter_mons(pacify_guard);
}

void
mimic_hit_msg(struct monst *mtmp, short otyp)
{
    short ap = mtmp->mappearance;

    switch (M_AP_TYPE(mtmp)) {
    case M_AP_NOTHING:
    case M_AP_FURNITURE:
    case M_AP_MONSTER:
        break;
    case M_AP_OBJECT:
        if (otyp == SPE_HEALING || otyp == SPE_EXTRA_HEALING) {
            pline_mon(mtmp, "%s seems a more vivid %s than before.",
                  The(simple_typename(ap)),
                  c_obj_colors[objects[ap].oc_color]);
        }
        break;
    }
}

boolean
usmellmon(struct permonst *mdat)
{
    int mndx;
    boolean nonspecific = FALSE;
    boolean msg_given = FALSE;

    if (mdat) {
        if (!olfaction(gy.youmonst.data))
            return FALSE;
        mndx = monsndx(mdat);
        switch (mndx) {
        case PM_ROTHE:
        case PM_MINOTAUR:
            You("notice a bovine smell.");
            msg_given = TRUE;
            break;
        case PM_CAVE_DWELLER:
        case PM_BARBARIAN:
        case PM_NEANDERTHAL:
            You("smell body odor.");
            msg_given = TRUE;
            break;
        /*
        case PM_PESTILENCE:
        case PM_FAMINE:
        case PM_DEATH:
            break;
        */
        case PM_HORNED_DEVIL:
        case PM_BALROG:
        case PM_ASMODEUS:
        case PM_DISPATER:
        case PM_YEENOGHU:
        case PM_ORCUS:
            break;
        case PM_HUMAN_WEREJACKAL:
        case PM_HUMAN_WERERAT:
        case PM_HUMAN_WEREWOLF:
        case PM_DEMON_WEREDEMON:
        case PM_WEREJACKAL:
        case PM_WERERAT:
        case PM_WEREWOLF:
        case PM_WEREDEMON:
        case PM_OWLBEAR:
            You("detect an odor reminiscent of an animal's den.");
            msg_given = TRUE;
            break;
        /*
        case PM_PURPLE_WORM:
            break;
        */
        case PM_STEAM_VORTEX:
            You("smell steam.");
            msg_given = TRUE;
            break;
        case PM_GREEN_SLIME:
            pline("%s stinks.", Something);
            msg_given = TRUE;
            break;
        case PM_VIOLET_FUNGUS:
        case PM_SHRIEKER:
            You("smell mushrooms.");
            msg_given = TRUE;
            break;
        /* These are here to avoid triggering the
           nonspecific treatment through the default case below*/
        case PM_WHITE_UNICORN:
        case PM_GRAY_UNICORN:
        case PM_BLACK_UNICORN:
        case PM_JELLYFISH:
            break;
        default:
            nonspecific = TRUE;
            break;
        }

        if (nonspecific)
            switch (mdat->mlet) {
            case S_DOG:
                You("notice a dog smell.");
                msg_given = TRUE;
                break;
            case S_DRAGON:
                You("smell a dragon!");
                msg_given = TRUE;
                break;
            case S_FUNGUS:
                pline("%s smells moldy.", Something);
                msg_given = TRUE;
                break;
            case S_UNICORN:
                You("detect a%s odor reminiscent of a stable.",
                    (mndx == PM_PONY) ? "n" : " strong");
                msg_given = TRUE;
                break;
            case S_ZOMBIE:
                You("smell rotting flesh.");
                msg_given = TRUE;
                break;
            case S_EEL:
                You("smell fish.");
                msg_given = TRUE;
                break;
            case S_ORC:
                if (maybe_polyd(is_orc(gy.youmonst.data), Race_if(PM_ORC)))
                    You("notice an attractive smell.");
                else
                    pline("A foul stench makes you feel a little nauseated.");
                msg_given = TRUE;
                break;
            default:
                break;
            }
    }
    return msg_given ? TRUE : FALSE;
}

/* setting misc_worn_check's I_SPECIAL bit flags a monster to reassess
   and potentially re-equip gear at the start of its next move;
   this hides the details of that */
void
check_gear_next_turn(struct monst *mon)
{
    mon->misc_worn_check |= I_SPECIAL;
}

/* Extract an object from a container that a monster is carrying.
 * Returns: 0 = extraction failed (locked, cursed BoH, etc.)
 *          1 = object was already in open inventory (no action needed)
 *          2 = extraction succeeded (used monster's turn)
 */
int
mon_container_extract(struct monst *mon, struct obj *obj)
{
    struct obj *container;
    char contnr_nam[BUFSZ];
    boolean vis, nearby;
    int howfar;

    if (!obj)
        return 0;

    /* object already in open inventory - no extraction needed */
    if (obj->where == OBJ_MINVENT)
        return 1;

    /* object not in a container - shouldn't happen, but handle it */
    if (obj->where != OBJ_CONTAINED)
        return 0;

    container = obj->ocontainer;

    /* prevent accessing locked containers, or containers that are
       empty or non-existent */
    if (!container || !Has_contents(container) || container->olocked)
        return 0;

    /* using a cursed bag of holding = bad */
    if (Is_mbag(container) && container->cursed)
        return 0;

    vis = cansee(mon->mx, mon->my);
    howfar = distu(mon->mx, mon->my);
    nearby = (howfar <= 7 * 7);

    /* hero no longer knows container's contents even if [attempted]
       removal is observed */
    container->cknown = 0;
    /* xname sets dknown, distant_name doesn't */
    Strcpy(contnr_nam, the(nearby ? xname(container)
                                  : distant_name(container, xname)));

    /* remove obj from container */
    obj_extract_self(obj); /* reduces container's weight */
    container->owt = weight(container);

    if (vis) {
        if (!nearby) /* not close by */
            Norep("%s rummages through %s.", Monnam(mon), contnr_nam);
        else
            pline_mon(mon, "%s removes %s from %s.", Monnam(mon),
                      ansimpleoname(obj), contnr_nam);
    } else if (!Deaf && nearby) {
        Norep("You hear something rummaging through %s.",
              ansimpleoname(container));
    }

    if (container->otyp == ICE_BOX)
        removed_from_icebox(obj); /* resume rotting for corpse */
    (void) mpickobj(mon, obj);
    check_gear_next_turn(mon);
    return 2;
}

/* make erinyes more dangerous based on your alignment abuse */
void
adj_erinys(unsigned abuse)
{
    struct permonst *pm = &mons[PM_ERINYS];

    if (abuse > 5L) {
        pm->mflags1 |= M1_SEE_INVIS;
    }
    if (abuse > 10L) {
        pm->mflags1 |= M1_AMPHIBIOUS;
    }
    if (abuse > 15L) {
        pm->mflags1 |= M1_FLY;
    }
    if (abuse > 20L) {
        /* more powerful attack */
        pm->mattk[0].damn = 3;
    }
    if (abuse > 25L) {
        pm->mflags1 |= M1_REGEN;
    }
    if (abuse > 30L) {
        pm->mflags1 |= M1_TPORT_CNTRL;
    }
    if (abuse > 35L) {
        /* second attack */
        pm->mattk[1].aatyp = AT_WEAP;
        pm->mattk[1].adtyp = AD_DRST;
        pm->mattk[1].damn = 3;
        pm->mattk[1].damd = 4;
    }
    if (abuse > 40L) {
        pm->mflags1 |= M1_TPORT;
    }
    if (abuse > 50L) {
        /* third (spellcasting) attack */
        pm->mattk[2].aatyp = AT_MAGC;
        pm->mattk[2].adtyp = AD_SPEL;
        pm->mattk[2].damn = 3;
        pm->mattk[2].damd = 4;
    }

    /* also adjust level and difficulty;
       mlevel >= 50 has a special meaning, so don't exceed 49 */
    pm->mlevel = min(7 + u.ualign.abuse, 49);
    pm->difficulty = min(10 + (u.ualign.abuse / 3), 25);
}

/* mark individual monster type as seen from close-up,
   if we haven't seen it nearby before */
void
see_monster_closeup(struct monst *mtmp, boolean photo)
{
    int mndx;

    if (Hallucination || (Blind && !Blind_telepat))
        return;

    mndx = monsndx(mtmp->data);
    if (M_AP_TYPE(mtmp) == M_AP_MONSTER && !sensemon(mtmp))
        mndx = mtmp->mappearance;
    if (mndx == PM_LONG_WORM && gn.notonhead)
        mndx = PM_LONG_WORM_TAIL;

    if (!svm.mvitals[mndx].seen_close) {
        svm.mvitals[mndx].seen_close = 1;
        svc.context.lifelist.total_seen_upclose++;
    }

    /* hallucinatory monsters don't reach here--they're not recorded;
       being able to see invisible doesn't make invisible monsters show up
       on photos; likewise, telepathy allows hero to see hidden monsters
       but doesn't cause them to appear on photos */
    if (photo && !mtmp->minvis && !mtmp->mundetected
        && (M_AP_TYPE(mtmp) == M_AP_NOTHING
            || M_AP_TYPE(mtmp) == M_AP_MONSTER)) {
        if (M_AP_TYPE(mtmp) == M_AP_MONSTER) /* cloned Wizard of Yendor */
            mndx = mtmp->mappearance;

        if (!svm.mvitals[mndx].photographed) {
            svm.mvitals[mndx].photographed = 1;
            svc.context.lifelist.total_photographed++;

            /* tourist earns points (toward EXP but not final score) for
               the first instance of each type of monster photographed;
               worm tail can be photographed but yields no EXP bonus */
            if (Role_if(PM_TOURIST)
                /* suppress extra points for photographing the pet that hero
                   started with (unless it has changed shape due to growing
                   up or being polymorphed) */
                && (mtmp->m_id != svc.context.startingpet_mid
                    || mndx != svc.context.startingpet_typ)
                /* monsndx() check covers worm tail and also disguised
                   Wizard of Yendor; experienced() won't yield a reasonable
                   value for those */
                && mndx == monsndx(mtmp->data)) {
                more_experienced(experience(mtmp, 0), 0);
                newexplevel();
            }
        }
    }
}

/* mark a monster type as seen close-up when we see it next to us */
void
see_nearby_monsters(void)
{
    struct monst *mtmp;
    int mndx;
    coordxy x, y;

    if (Hallucination || (Blind && !Blind_telepat))
        return;

    for (x = u.ux - 1; x <= u.ux + 1; x++)
        for (y = u.uy - 1; y <= u.uy + 1; y++) {
            if (!isok(x, y))
                continue;
            if (!(mtmp = m_at(x, y)))
                continue;
            mndx = monsndx(mtmp->data);
            if (M_AP_TYPE(mtmp) == M_AP_MONSTER)
                mndx = mtmp->mappearance;
            /* skip closeup handling if this mon type has already been done */
            if (svm.mvitals[mndx].seen_close)
                continue;
            /* disguised mimics pass canseemon(); undetected hiders don't */
            if (canseemon(mtmp) || (mtmp->mundetected && sensemon(mtmp))) {
                gb.bhitpos.x = x, gb.bhitpos.y = y;
                gn.notonhead = (x != mtmp->mx || y != mtmp->my);
                see_monster_closeup(mtmp, FALSE);
            }
        }
}

/* monster resists something.
   make a shield effect at monster's location and give a message */
void
shieldeff_mon(struct monst *mtmp)
{
    shieldeff(mtmp->mx, mtmp->my);
    /* does not depend on seeing the monster; the shield effect is visible */
    if (cansee(mtmp->mx, mtmp->my))
        pline_mon(mtmp, "%s resists!", Monnam(mtmp));
}

void
flash_mon(struct monst *mtmp)
{
    coordxy mx = mtmp->mx, my = mtmp->my;
    int count = couldsee(mx, my) ? 8 : 4;
    char saveviz = gv.viz_array[my][mx];

    if (!flags.sparkle)
        count /= 2;
    gv.viz_array[my][mx] |= (IN_SIGHT | COULD_SEE);
    flash_glyph_at(mx, my, mon_to_glyph(mtmp, newsym_rn2), count);
    gv.viz_array[my][mx] = saveviz;
    newsym(mx, my);
}

/* cleanup for 'onefile' processing */
#undef LEVEL_SPECIFIC_NOCORPSE
#undef KEEPTRAITS
#undef mstoning
#undef livelog_mon_nam
#undef BREEDER_EGG

/* map a monster index to an index into races[] (the playable-race
   table), or NON_PM if it is not a playable race */
int
pm_to_race(short mndx)
{
    int i;

    for (i = 0; races[i].adj; i++) {
        if (races[i].mnum == mndx)
            return i;
    }
    return NON_PM;
}

staticfn short
m_chooserace(unsigned long permitted)
{
    int i, count = 0;
    short race = NON_PM;
    static const short mraces[] = {
        PM_HUMAN, PM_ELF, PM_DWARF, PM_GNOME,
        PM_ORC, PM_GIANT, PM_HOBBIT, PM_CENTAUR,
        PM_ILLITHID, PM_TORTLE, PM_DROW, PM_DRAUGR,
        PM_VAMPIRE, PM_AASIMAR, 0
    };

    for (i = 0; mraces[i]; i++) {
        if ((permitted & mons[mraces[i]].mhflags) != 0L
            && !(svm.mvitals[mraces[i]].mvflags & G_GONE)) {
            count++;
            if (!rn2(count))
                race = mraces[i];
        }
    }
    return race;
}

/* this function is used for racial priests creation */
short
align_randrace(aligntyp algn)
{
    unsigned long permitted = MH_HUMAN;

    switch (algn) {
    case A_NONE:
        permitted |= (MH_ILLITHID | MH_ORC | MH_ELF | MH_GIANT
                      | MH_DROW);
        break;
    case A_CHAOTIC:
        permitted |=
            (MH_ILLITHID | MH_ORC | MH_ELF | MH_GIANT | MH_CENTAUR
             | MH_DROW);
        break;
    case A_LAWFUL:
        permitted |= (MH_DWARF | MH_GIANT | MH_TORTLE | MH_AASIMAR);
        break;
    case A_NEUTRAL:
        permitted |=
            (MH_ELF | MH_DWARF | MH_GIANT | MH_CENTAUR | MH_TORTLE);
        break;
    default:
        break;
    }
    return m_chooserace(permitted);
}

short
m_randrace(short mndx)
{
    unsigned long permitted = MH_HUMAN;

    switch (mndx) {
    case PM_SHOPKEEPER:
        permitted |= (MH_CENTAUR | MH_ORC | MH_DWARF | MH_ELF
                      | MH_GNOME | MH_GIANT | MH_DROW | MH_AASIMAR);
        break;
    case PM_SOLDIER:
    case PM_SERGEANT:
    case PM_LIEUTENANT:
    case PM_WATCHMAN:
    case PM_GUARD:
    case PM_PRISON_GUARD:
        permitted |= (MH_CENTAUR | MH_ORC);
        FALLTHROUGH;
        /*FALLTHRU*/
    case PM_CAPTAIN:
    case PM_WATCH_CAPTAIN:
        permitted |= (MH_DWARF | MH_ELF | MH_GNOME | MH_GIANT
                      | MH_DROW);
        break;
    case PM_ARCHEOLOGIST:
        permitted |= (MH_DWARF | MH_GNOME | MH_HOBBIT | MH_TORTLE
                      | MH_AASIMAR);
        break;
    case PM_BARBARIAN:
        permitted |= (MH_DWARF | MH_ORC | MH_GIANT | MH_CENTAUR
                      | MH_TORTLE | MH_ZOMBIE | MH_VAMPIRE);
        break;
    case PM_CAVE_DWELLER:
        permitted |= (MH_DWARF | MH_GNOME | MH_GIANT | MH_AASIMAR);
        break;
    case PM_CONVICT:
        permitted |=
            (MH_DWARF | MH_ORC | MH_GNOME | MH_HOBBIT | MH_ILLITHID
             | MH_DROW | MH_ZOMBIE | MH_VAMPIRE);
        break;
    case PM_DRUID:
        permitted |=
            (MH_ELF | MH_HOBBIT | MH_CENTAUR | MH_TORTLE | MH_GIANT);
        break;
    case PM_HEALER:
        permitted |=
            (MH_DWARF | MH_ELF | MH_GNOME | MH_HOBBIT | MH_CENTAUR
             | MH_GIANT | MH_TORTLE);
        break;
    case PM_INFIDEL:
        permitted |= (MH_ELF | MH_GIANT | MH_ORC | MH_ILLITHID
                      | MH_DROW | MH_ZOMBIE | MH_VAMPIRE);
        break;
    case PM_KNIGHT:
        permitted |= (MH_DWARF | MH_ELF | MH_ORC | MH_CENTAUR
                      | MH_DROW | MH_ZOMBIE | MH_VAMPIRE | MH_AASIMAR);
        break;
    case PM_MONK:
        permitted |= (MH_DWARF | MH_ELF | MH_GIANT | MH_CENTAUR
                      | MH_TORTLE | MH_DROW | MH_ZOMBIE | MH_VAMPIRE
                      | MH_AASIMAR);
        break;
    case PM_CLERIC:
        permitted |=
            (MH_DWARF | MH_ELF | MH_GIANT | MH_HOBBIT | MH_CENTAUR
             | MH_ORC | MH_ILLITHID | MH_TORTLE | MH_DROW | MH_AASIMAR);
        break;
    case PM_RANGER:
        permitted |= (MH_ELF | MH_GNOME | MH_HOBBIT | MH_CENTAUR
                      | MH_ORC | MH_DROW);
        break;
    case PM_ROGUE:
        permitted |= (MH_ELF | MH_HOBBIT | MH_ORC | MH_GNOME
                      | MH_DROW | MH_ZOMBIE | MH_VAMPIRE);
        break;
    case PM_SAMURAI:
        permitted |= (MH_DWARF | MH_GIANT | MH_TORTLE | MH_AASIMAR);
        break;
    case PM_TOURIST:
        permitted |= (MH_HOBBIT | MH_GNOME | MH_TORTLE);
        break;
    case PM_VALKYRIE:
        permitted |= (MH_DWARF | MH_GIANT | MH_CENTAUR | MH_AASIMAR);
        break;
    case PM_WIZARD:
        permitted |=
          (MH_DWARF | MH_ELF | MH_GIANT | MH_GNOME | MH_HOBBIT
           | MH_ORC | MH_ILLITHID | MH_TORTLE | MH_DROW | MH_VAMPIRE);
        break;
    default:
        break;
    }
    return m_chooserace(permitted);
}

/* overlay a race onto a monster via the erac mextra component; raceidx
   is a PM_ index for the race (e.g. PM_DWARF), or NON_PM to clear */
void
apply_race(struct monst *mtmp, short raceidx)
{
    struct erac *rptr;
    struct permonst *ptr, *mptr;
    boolean init = FALSE;

    if (!mtmp)
        return;

    if (raceidx == NON_PM) {
        /* apply non-race - clear race data */
        if (has_erac(mtmp))
            free_erac(mtmp);
        return;
    }

    if (svm.mvitals[raceidx].mvflags & G_GONE)
        return;

    ptr = &mons[raceidx];
    mptr = &mons[mtmp->mnum];

    if (!has_erac(mtmp)) {
        newerac(mtmp);
        init = TRUE;
    }

    rptr = ERAC(mtmp);

    if (init) {
        rptr->ralign = mptr->maligntyp;
        memcpy(rptr->mattk, mptr->mattk, sizeof (struct attack) * NATTK);
        rptr->mflags1 = mptr->mflags1;
        rptr->mflags2 = mptr->mflags2;
        rptr->mflags3 = mptr->mflags3;
    }

    if (is_female(ptr))
        mtmp->female = 1;
    else if (is_male(ptr))
        mtmp->female = 0;

    mtmp->mintrinsics |= ptr->mresists;
    rptr->r_id = pm_to_race(raceidx);
    rptr->mrace = ptr->mhflags;
    rptr->rmnum = raceidx;
    /* racial mflags are cumulative with mflags from the base monster,
     * rather than overwriting them entirely */
    rptr->mflags1 |= ptr->mflags1;
    rptr->mflags2 |= ptr->mflags2;
    rptr->mflags3 |= ptr->mflags3;

    /* various racial adjustments (special attack types, alignment, etc) */
    switch (raceidx) {
    case PM_HUMAN:
        if (mtmp->mnum == PM_CONVICT)
            rptr->ralign = -3;
        if (mtmp->mnum == PM_INFIDEL)
            rptr->ralign = -128;
        if (mtmp->mnum == PM_BARBARIAN
            || mtmp->mnum == PM_RANGER
            || mtmp->mnum == PM_ROGUE
            || mtmp->mnum == PM_WIZARD)
            rptr->ralign = rn2(2) ? 0 : -3;
        if (mtmp->mnum == PM_ARCHEOLOGIST
            || mtmp->mnum == PM_CAVE_DWELLER
            || mtmp->mnum == PM_VALKYRIE)
            rptr->ralign = rn2(2) ? 0 : 3;
        if (mtmp->mnum == PM_KNIGHT)
            rptr->ralign = rn2(2) ? 3 : -3;
        if (mtmp->mnum == PM_MONK
            || mtmp->mnum == PM_CLERIC)
            rptr->ralign = rn2(2) ? 3 : rn2(2) ? 0 : -3;
        break;
    case PM_ELF:
        if (mtmp->mnum == PM_INFIDEL)
            rptr->ralign = -128;
        if (mtmp->mnum == PM_HEALER
            || mtmp->mnum == PM_DRUID)
            rptr->ralign = 0;
        if (mtmp->mnum == PM_KNIGHT)
            rptr->ralign = -3;
        if (mtmp->mnum == PM_MONK
            || mtmp->mnum == PM_CLERIC
            || mtmp->mnum == PM_RANGER
            || mtmp->mnum == PM_ROGUE
            || mtmp->mnum == PM_WIZARD)
            rptr->ralign = rn2(3) ? -3 : 0;
        break;
    case PM_DWARF:
        if (mtmp->mnum == PM_CONVICT)
            rptr->ralign = -3;
        if (mtmp->mnum == PM_ARCHEOLOGIST
            || mtmp->mnum == PM_CAVE_DWELLER
            || mtmp->mnum == PM_MONK
            || mtmp->mnum == PM_CLERIC
            || mtmp->mnum == PM_VALKYRIE)
            rptr->ralign = rn2(3) ? 3 : 0;
        if (mtmp->mnum == PM_BARBARIAN
            || mtmp->mnum == PM_HEALER
            || mtmp->mnum == PM_WIZARD)
            rptr->ralign = 0;
        if (mtmp->mnum == PM_KNIGHT)
            rptr->ralign = 3;
        break;
    case PM_GIANT:
        if (!(mtmp->mnum == PM_WIZARD
              || mtmp->mnum == PM_MONK
              || mtmp->mnum == PM_HEALER
              || mtmp->mnum == PM_CLERIC
              || mtmp->mnum == PM_DRUID)) {
            rptr->mattk[0].aatyp = AT_WEAP;
            rptr->mattk[0].adtyp = AD_CLOB;
            rptr->mattk[0].damn = 2;
            rptr->mattk[0].damd = 8;
        }
        if (mtmp->mnum == PM_INFIDEL)
            rptr->ralign = -128;
        if (mtmp->mnum == PM_CAVE_DWELLER
            || mtmp->mnum == PM_VALKYRIE)
            rptr->ralign = rn2(2) ? 0 : 3;
        if (mtmp->mnum == PM_BARBARIAN
            || mtmp->mnum == PM_WIZARD)
            rptr->ralign = rn2(2) ? 0 : -3;
        if (mtmp->mnum == PM_HEALER
            || mtmp->mnum == PM_DRUID)
            rptr->ralign = 0;
        if (mtmp->mnum == PM_MONK
            || mtmp->mnum == PM_CLERIC)
            rptr->ralign = rn2(2) ? 3 : rn2(2) ? 0 : -3;
        break;
    case PM_GNOME:
        rptr->ralign = 0;
        if (mtmp->mnum == PM_CONVICT)
            rptr->ralign = -3;
        if (mtmp->mnum == PM_RANGER
            || mtmp->mnum == PM_ROGUE
            || mtmp->mnum == PM_WIZARD)
            rptr->ralign = rn2(3) ? 0 : -3;
        break;
    case PM_HOBBIT:
        rptr->ralign = 0;
        if (mtmp->mnum == PM_CONVICT)
            rptr->ralign = -3;
        if (mtmp->mnum == PM_ARCHEOLOGIST
            || mtmp->mnum == PM_CLERIC)
            rptr->ralign = rn2(3) ? 0 : 3;
        break;
    case PM_CENTAUR:
        rptr->mattk[2].aatyp = AT_KICK;
        rptr->mattk[2].adtyp = AD_PHYS;
        rptr->mattk[2].damn = 1;
        rptr->mattk[2].damd = 6;
        if (mtmp->mnum == PM_INFIDEL)
            rptr->ralign = -128;
        if (mtmp->mnum == PM_HEALER
            || mtmp->mnum == PM_VALKYRIE
            || mtmp->mnum == PM_DRUID)
            rptr->ralign = 0;
        if (mtmp->mnum == PM_BARBARIAN
            || mtmp->mnum == PM_MONK
            || mtmp->mnum == PM_CLERIC)
            rptr->ralign = rn2(3) ? -3 : 0;
        break;
    case PM_ORC:
        rptr->ralign = -3;
        if (mtmp->mnum == PM_INFIDEL)
            rptr->ralign = -128;
        break;
    case PM_ILLITHID:
        rptr->mattk[2].aatyp = AT_TENT;
        rptr->mattk[2].adtyp = AD_DRIN;
        rptr->mattk[2].damn = 2;
        rptr->mattk[2].damd = 1;
        rptr->ralign = -3;
        if (mtmp->mnum == PM_INFIDEL)
            rptr->ralign = -128;
        break;
    case PM_TORTLE:
        rptr->ralign = 0;
        if (mtmp->mnum == PM_ARCHEOLOGIST
            || mtmp->mnum == PM_MONK
            || mtmp->mnum == PM_CLERIC)
            rptr->ralign = rn2(4) ? 3 : 0;
        break;
    case PM_DROW:
        rptr->ralign = -3;
        if (mtmp->mnum == PM_INFIDEL)
            rptr->ralign = -128;
        break;
    case PM_DRAUGR:
        rptr->mattk[2].aatyp = AT_BITE;
        rptr->mattk[2].adtyp = AD_DRIN;
        rptr->mattk[2].damn = 2;
        rptr->mattk[2].damd = 1;
        rptr->ralign = -3;
        if (mtmp->mnum == PM_INFIDEL)
            rptr->ralign = -128;
        break;
    case PM_VAMPIRE:
        rptr->mattk[2].aatyp = AT_BITE;
        rptr->mattk[2].adtyp = AD_DRLI;
        rptr->mattk[2].damn = 4;
        rptr->mattk[2].damd = 1;
        rptr->ralign = -3;
        if (mtmp->mnum == PM_INFIDEL)
            rptr->ralign = -128;
        break;
    case PM_AASIMAR:
        rptr->ralign = 3;
        break;
    }
}

/*mon.c*/
