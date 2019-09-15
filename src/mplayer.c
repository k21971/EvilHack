/* NetHack 3.6	mplayer.c	$NHDT-Date: 1550524564 2019/02/18 21:16:04 $  $NHDT-Branch: NetHack-3.6.2-beta01 $:$NHDT-Revision: 1.26 $ */
/*      Copyright (c) Izchak Miller, 1992.                        */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

STATIC_DCL const char *NDECL(dev_name);
void FDECL(get_mplname, (struct monst *, char *));
STATIC_DCL void FDECL(mk_mplayer_armor, (struct monst *, SHORT_P));

/* These are the names of those who
 * contributed to the development of NetHack 3.2/3.3/3.4/3.6.
 *
 * Keep in alphabetical order within teams.
 * Same first name is entered once within each team.
 */
static const char *developers[] = {
    /* devteam */
    "Alex",    "Dave",   "Dean",    "Derek",   "Eric",    "Izchak",
    "Janet",   "Jessie", "Ken",     "Kevin",   "Michael", "Mike",
    "Pasi",    "Pat",    "Patric",  "Paul",    "Sean",    "Steve",
    "Timo",    "Warwick",
    /* PC team */
    "Bill",    "Eric",   "Keizo",   "Ken",    "Kevin",    "Michael",
    "Mike",    "Paul",   "Stephen", "Steve",  "Timo",     "Yitzhak",
    /* Amiga team */
    "Andy",    "Gregg",  "Janne",   "Keni",   "Mike",     "Olaf",
    "Richard",
    /* Mac team */
    "Andy",    "Chris",  "Dean",    "Jon",    "Jonathan", "Kevin",
    "Wang",
    /* Atari team */
    "Eric",    "Marvin", "Warwick",
    /* NT team */
    "Alex",    "Dion",   "Michael",
    /* OS/2 team */
    "Helge",   "Ron",    "Timo",
    /* VMS team */
    "Joshua",  "Pat",    ""
};

static const char *fem_names[] = {
    "Alexandria",   "Anna",        "Ariel",      "Betty",      "Charlotte",
    "Ella",         "Gabrielle",   "Inez",       "Isabella",   "Kathryn",
    "Lillianna",    "Joanna",      "Mary",       "Nancy",      "Natasha",
    "Olivia",       "Penny",       "Rosa",       "Sally",      "Sierra",
    "Sophia",       "Tabitha",     "Veronica",   "Yvette",     "Zoey"
};

/* return a randomly chosen developer name */
STATIC_OVL const char *
dev_name()
{
    register int i, m = 0, n = SIZE(developers);
    register struct monst *mtmp;
    register boolean match;

    do {
        match = FALSE;
        i = rn2(n);
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
            if (!is_mplayer(mtmp->data))
                continue;
            if (!strncmp(developers[i], (has_mname(mtmp)) ? MNAME(mtmp) : "",
                         strlen(developers[i]))) {
                match = TRUE;
                break;
            }
        }
        m++;
    } while (match && m < 100); /* m for insurance */

    if (match)
        return (const char *) 0;
    return (developers[i]);
}

void
get_mplname(mtmp, nam)
register struct monst *mtmp;
char *nam;
{
    boolean fmlkind = is_female(mtmp->data);
    const char *devnam;

    devnam = dev_name();
    if (!devnam)
        Strcpy(nam, fmlkind ? "Eve" : "Adam");
    else if (fmlkind && !!strcmp(devnam, "Janet"))
        Sprintf(nam, "%s", fem_names[rn2(SIZE(fem_names))]);
    else
        Strcpy(nam, devnam);

    if (fmlkind || !strcmp(nam, "Janet"))
        mtmp->female = 1;
    else
        mtmp->female = 0;
    Strcat(nam, " the ");
    Strcat(nam, rank_of((int) mtmp->m_lev, monsndx(mtmp->data),
                        (boolean) mtmp->female));
}

STATIC_OVL void
mk_mplayer_armor(mon, typ)
struct monst *mon;
short typ;
{
    struct obj *obj;

    if (typ == STRANGE_OBJECT)
        return;
    obj = mksobj(typ, FALSE, FALSE);
    if (!rn2(3))
        obj->oerodeproof = 1;
    if (!rn2(3))
        curse(obj);
    if (!rn2(3))
        bless(obj);
    /* Most players who get to the endgame who have cursed equipment
     * have it because the wizard or other monsters cursed it, so its
     * chances of having plusses is the same as usual....
     */
    obj->spe = rn2(10) ? (rn2(3) ? rn2(5) : rn1(4, 4)) : -rnd(3);
    (void) mpickobj(mon, obj);
}

struct monst *
mk_mplayer(ptr, x, y, special, obj)
register struct permonst *ptr;
xchar x, y;
register boolean special;
struct obj *obj;
{
    register struct monst *mtmp;
    register boolean ascending = special && (In_endgame(&u.uz) || u.uhave.amulet);

    char nam[PL_NSIZ];

    if (MON_AT(x, y))
        (void) rloc(m_at(x, y), FALSE); /* insurance */

    if ((mtmp = makemon(ptr, x, y, NO_MM_FLAGS)) != 0) {
        short weapon, armor, cloak, helm, shield;
        int quan;
        struct obj *otmp;

        mtmp->m_lev = (special ? (ascending ? rn1(16, 15) : min(30, u.ulevel + rn1(4, 4))) : rnd(16));
        mtmp->mhp = mtmp->mhpmax = d((int)mtmp->m_lev, 10) +
                    (ascending ? (30 + rnd(30)) : 30);
        if (ascending) {
            /* that's why they are "stuck" in the endgame :-) */
            (void) mongets(mtmp, FAKE_AMULET_OF_YENDOR);
        }
        get_mplname(mtmp, nam);
        mtmp = christen_monst(mtmp, nam);
        mtmp->mpeaceful = 0;
        set_malign(mtmp); /* peaceful may have changed again */

        /* default equipment; much of it will be overridden below */
        weapon = !rn2(2) ? LONG_SWORD : rnd_class(SPEAR, BULLWHIP);
        armor  = rnd_class(GRAY_DRAGON_SCALE_MAIL, YELLOW_DRAGON_SCALE_MAIL);
        cloak  = !rn2(8) ? STRANGE_OBJECT
                         : rnd_class(OILSKIN_CLOAK, CLOAK_OF_DISPLACEMENT);
        helm   = !rn2(8) ? STRANGE_OBJECT
                         : rnd_class(ELVEN_HELM, HELM_OF_TELEPATHY);
        shield = !rn2(8) ? STRANGE_OBJECT
                         : rnd_class(ELVEN_SHIELD, SHIELD_OF_REFLECTION);

        switch (monsndx(ptr)) {
        case PM_HUMAN_ARCHEOLOGIST:
            if (rn2(2))
                weapon = BULLWHIP;
            break;
        case PM_HUMAN_BARBARIAN:
            if (rn2(2)) {
                weapon = rn2(2) ? TWO_HANDED_SWORD : BATTLE_AXE;
                shield = STRANGE_OBJECT;
            }
            if (rn2(2))
                armor = rnd_class(PLATE_MAIL, CHAIN_MAIL);
            if (helm == HELM_OF_BRILLIANCE)
                helm = STRANGE_OBJECT;
            break;
        case PM_HUMAN_CAVEWOMAN:
            if (rn2(4))
                weapon = MACE;
            else if (rn2(2))
                weapon = CLUB;
            if (helm == HELM_OF_BRILLIANCE)
                helm = STRANGE_OBJECT;
            break;
        case PM_HUMAN_HEALER:
            if (rn2(4))
                weapon = QUARTERSTAFF;
            else if (rn2(2))
                weapon = rn2(2) ? UNICORN_HORN : SCALPEL;
            if (rn2(4))
                helm = rn2(2) ? HELM_OF_BRILLIANCE : HELM_OF_TELEPATHY;
            if (rn2(2))
                shield = STRANGE_OBJECT;
            break;
        case PM_HUMAN_KNIGHT:
            if (rn2(4))
                weapon = LONG_SWORD;
            if (rn2(2))
                armor = rnd_class(PLATE_MAIL, CHAIN_MAIL);
            break;
        case PM_HUMAN_MONK:
            weapon = !rn2(3) ? SHURIKEN : STRANGE_OBJECT;
            armor = STRANGE_OBJECT;
            cloak = ROBE;
            if (rn2(2))
                shield = STRANGE_OBJECT;
            break;
        case PM_HUMAN_PRIESTESS:
            if (rn2(2))
                weapon = MACE;
            if (rn2(2))
                armor = rnd_class(PLATE_MAIL, CHAIN_MAIL);
            if (rn2(4))
                cloak = ROBE;
            if (rn2(4))
                helm = rn2(2) ? HELM_OF_BRILLIANCE : HELM_OF_TELEPATHY;
            if (rn2(2))
                shield = STRANGE_OBJECT;
            break;
        case PM_HUMAN_RANGER:
            if (rn2(2))
                weapon = ELVEN_DAGGER;
            break;
        case PM_HUMAN_ROGUE:
            if (rn2(2))
                weapon = rn2(2) ? SHORT_SWORD : ORCISH_DAGGER;
            break;
        case PM_HUMAN_SAMURAI:
            if (rn2(2))
                weapon = KATANA;
            break;
        case PM_HUMAN_TOURIST:
            (void) mongets(mtmp, EXPENSIVE_CAMERA);
            break;
        case PM_HUMAN_VALKYRIE:
            if (rn2(2))
                weapon = WAR_HAMMER;
            if (rn2(2))
                armor = rnd_class(PLATE_MAIL, CHAIN_MAIL);
            break;
        case PM_HUMAN_WIZARD:
            if (rn2(4))
                weapon = rn2(2) ? QUARTERSTAFF : ATHAME;
            if (rn2(2)) {
                armor = rn2(2) ? BLACK_DRAGON_SCALE_MAIL
                               : SILVER_DRAGON_SCALE_MAIL;
                cloak = CLOAK_OF_MAGIC_RESISTANCE;
            }
            if (rn2(4))
                helm = HELM_OF_BRILLIANCE;
            shield = STRANGE_OBJECT;
            break;
        default:
            weapon = STRANGE_OBJECT;
            break;
        }

        if (obj) {
            if (obj->oclass == WEAPON_CLASS)
                weapon = STRANGE_OBJECT;
            if (is_shield(obj))
                shield = STRANGE_OBJECT;
        }

        if (weapon != STRANGE_OBJECT) {
            otmp = mksobj(weapon, TRUE, FALSE);
            otmp->spe = (ascending ? rn1(5, 4) : rn2(4));
            if (!rn2(3))
                otmp->oerodeproof = 1;
            else if (!rn2(2))
                otmp->greased = 1;
            if (special && rn2(2)) {
                if (!obj) {
                    if (!rn2(5))
                        otmp = mk_artifact(otmp, A_NONE);
                    else
                        otmp = create_oprop(otmp, FALSE);
                }
            }
            /* usually increase stack size if stackable weapon */
            if (objects[otmp->otyp].oc_merge && !otmp->oartifact
                && monmightthrowwep(otmp))
                otmp->quan += (long) rn2(is_spear(otmp) ? 4 : 8);
            /* mplayers knew better than to overenchant Magicbane */
            if (otmp->oartifact == ART_MAGICBANE)
                otmp->spe = rnd(4);
            (void) mpickobj(mtmp, otmp);
        }

        if (ascending) {
            if (!rn2(10))
                (void) mongets(mtmp, rn2(3) ? LUCKSTONE : LOADSTONE);
            mk_mplayer_armor(mtmp, armor);
            mk_mplayer_armor(mtmp, cloak);
            mk_mplayer_armor(mtmp, helm);
            mk_mplayer_armor(mtmp, shield);
            if (weapon == WAR_HAMMER) /* valkyrie: wimpy weapon or Mjollnir */
                mk_mplayer_armor(mtmp, GAUNTLETS_OF_POWER);
            else if (rn2(8))
                mk_mplayer_armor(mtmp, rnd_class(GLOVES,
                                                 GAUNTLETS_OF_DEXTERITY));
            if (rn2(8))
                mk_mplayer_armor(mtmp, rnd_class(LOW_BOOTS,
                                                 LEVITATION_BOOTS));
            m_dowear(mtmp, TRUE);
            mon_wield_item(mtmp);

	    /* done after wearing any dragon mail so the resists checks work */
	    if (rn2(8) || monsndx(ptr) == PM_HUMAN_WIZARD) {
		int i, ring;
		for (i=0; i<2 && (rn2(2) || monsndx(ptr) == PM_HUMAN_WIZARD); i++) {
		     do ring = !rn2(9) ? RIN_INVISIBILITY :
			       !rn2(8) ? RIN_TELEPORT_CONTROL :
			       !rn2(7) ? RIN_FIRE_RESISTANCE :
			       !rn2(6) ? RIN_COLD_RESISTANCE :
		               !rn2(5) ? RIN_SHOCK_RESISTANCE :
			       !rn2(4) ? RIN_POISON_RESISTANCE :
			       !rn2(3) ? RIN_INCREASE_ACCURACY :
			       !rn2(2) ? RIN_INCREASE_DAMAGE :
					 RIN_PROTECTION;
		     while ((resists_poison(mtmp) && ring == RIN_POISON_RESISTANCE)
			    || (resists_elec(mtmp) && ring == RIN_SHOCK_RESISTANCE)
			    || (resists_fire(mtmp) && ring == RIN_FIRE_RESISTANCE)
			    || (resists_cold(mtmp) && ring == RIN_COLD_RESISTANCE)
			    || (mtmp->minvis && ring == RIN_INVISIBILITY));
			    mk_mplayer_armor(mtmp, ring);
		 }
	    }

            quan = rn2(3) ? rn2(3) : rn2(16);
            while (quan--)
                (void) mongets(mtmp, rnd_class(DILITHIUM_CRYSTAL, JADE));
            /* To get the gold "right" would mean a player can double his
               gold supply by killing one mplayer.  Not good. */
            mkmonmoney(mtmp, rn2(1000));
            quan = rn2(10);
            while (quan--)
                (void) mpickobj(mtmp, mkobj(RANDOM_CLASS, FALSE));
        }

        quan = rnd(3);
        while (quan--)
            (void) mongets(mtmp, rnd_offensive_item(mtmp));
        quan = rnd(3);
        while (quan--)
            (void) mongets(mtmp, rnd_defensive_item(mtmp));
        quan = rnd(3);
        while (quan--)
            (void) mongets(mtmp, rnd_misc_item(mtmp));
    }

    return (mtmp);
}

/* create the indicated number (num) of monster-players,
 * randomly chosen, and in randomly chosen (free) locations
 * on the level.  If "special", the size of num should not
 * be bigger than the number of _non-repeated_ names in the
 * developers array, otherwise a bunch of Adams and Eves will
 * fill up the overflow.
 */
void
create_mplayers(num, special)
register int num;
boolean special;
{
    int pm, x, y;
    struct monst fakemon;

    fakemon = zeromonst;
    while (num) {
        int tryct = 0;

        /* roll for character class */
        pm = rn1(PM_HUMAN_WIZARD - PM_HUMAN_ARCHEOLOGIST + 1, PM_HUMAN_ARCHEOLOGIST);
        set_mon_data(&fakemon, &mons[pm]);

        /* roll for an available location */
        do {
            x = rn1(COLNO - 4, 2);
            y = rnd(ROWNO - 2);
        } while (!goodpos(x, y, &fakemon, 0) && tryct++ <= 500);

        /* if pos not found in 500 tries, don't bother to continue */
        if (tryct > 500)
            return;

        (void) mk_mplayer(&mons[pm], (xchar) x, (xchar) y, special, NULL);
        num--;
    }
}

void
mplayer_talk(mtmp)
register struct monst *mtmp;
{
    static const char
        *same_class_msg[3] = {
            "I can't win, and neither will you!",
            "You don't deserve to win!",
            "Mine should be the honor, not yours!",
        },
        *other_class_msg[3] = {
            "The low-life wants to talk, eh?",
            "Fight, scum!",
            "Here is what I have to say!",
        };

    if (mtmp->mpeaceful)
        return; /* will drop to humanoid talk */

    pline("Talk? -- %s", (mtmp->data == &mons[urole.malenum]
                          || mtmp->data == &mons[urole.femalenum])
                             ? same_class_msg[rn2(3)]
                             : other_class_msg[rn2(3)]);
}

/*mplayer.c*/
