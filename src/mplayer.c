/* NetHack 3.6	mplayer.c	$NHDT-Date: 1550524564 2019/02/18 21:16:04 $  $NHDT-Branch: NetHack-3.6.2-beta01 $:$NHDT-Revision: 1.26 $ */
/*      Copyright (c) Izchak Miller, 1992.                        */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* Human male struct - These are the names of those who
 * contributed to the development of NetHack 3.2 through 3.7.
 * Janet has been included in the human female struct. This
 * struct includes every name in alphabetical order, regardless
 * of which team they worked on. */
static const char *human_male_names[] = {
    /* devteam et al */
    "Andy",         "Alex",        "Bart",       "Bill",       "Chris",
    "Dave",         "Dean",        "Derek",      "Dion",       "Eric",
    "Gregg",        "Helge",       "Izchak",     "Janne",      "Jessie",
    "Jon",          "Jonathan",    "Joshua",     "Keizo",      "Ken",
    "Keni",         "Kevin",       "Marvin",     "Michael",    "Mike",
    "Olaf",         "Pasi",        "Pat",        "Patric",     "Paul",
    "Richard",      "Ron",         "Sean",       "Stephen",    "Steve",
    "Timo",         "Wang",        "Warwick",    "Yitzhak",
    /* don't forget Carl */
    "Carl",
    0
};

static const char *human_female_names[] = {
    "Alexandria",   "Anna",        "Ariel",      "Betty",      "Charlotte",
    "Ella",         "Gabrielle",   "Inez",       "Isabella",   "Janet", /* devteam */
    "Lillianna",    "Joanna",      "Mary",       "Nancy",      "Natasha",
    "Olivia",       "Penny",       "Rosa",       "Sally",      "Sierra",
    "Sophia",       "Tabitha",     "Veronica",   "Yvette",     "Zoey",
    0
};

static const char *elf_male_names[] = {
    "Aelion",       "Aphedir",     "Caeleben",   "Eliion",     "Erwarthon",
    "Eston",        "Harnedir",    "Harnon",     "Laerchon",   "Lassendaer",
    "Meremen",      "Miluichon",   "Nimen",      "Nithor",     "Padrion",
    "Peldaer",      "Pendor",      "Silon",      "Sogrion",    "Tyalion",
    0
};

static const char *elf_female_names[] = {
    "Arthil",       "Calassel",    "Cureth",     "Dewrien",    "Eilianniel",
    "Gelinnasbes",  "Gwaerenil",   "Harnith",    "Hwiniril",   "Linnriel",
    "Liririen",     "Luthadis",    "Luthril",    "Mialel",     "Maechenebeth",
    "Raegeth",      "Remlasbes",   "Solnissien", "Theririen",  "Tolneth",
    0
};

static const char *dwarf_male_names[] = {
    "Brambin",      "Bromlar",     "Drandar",    "Dulbor",     "Dwusrun",
    "Flel",         "Flumbin",     "Frelrur",    "Frerbun",    "Glelrir",
    "Glomik",       "Glorvor",     "Grinir",     "Grundig",    "Khel",
    "Krandar",      "Tham",        "Tharlor",    "Thorak",     "Thrun",
    0
};

static const char *dwarf_female_names[] = {
    "Agahildr",     "Bryldana",    "Dourdoulin", "Erigruigit", "Glarnirgith",
    "Grordrabyrn",  "Hjara",       "Kanolsia",   "Kaldana",    "Kherebyrn",
    "Orirodeth",    "Radgrarra",   "Skafadrid",  "Thrurja",    "Umirsila",
    0
};

static const char *centaur_male_names[] = {
    "Ajalus",       "Cephagio",    "Danasius",   "Erymanus",   "Koretrius",
    "Linasio",      "Sofrasos",    "Tymysus",    "Yorgoneus",  "Zerelous",
    0
};

static const char *centaur_female_names[] = {
    "Agaraia",      "Eidone",      "Hekasia",    "Mellaste",   "Nemolea",
    "Olamna",       "Phaeraris",   "Rhodora",    "Theladina",  "Typheis",
    0
};

static const char *giant_male_names[] = {
    "Agant",        "Clobos",      "Creswor",    "Cuzwar",     "Dlifur",
    "Dlimohr",      "Doxroch",     "Falwar",     "Fexsal",     "Grorlith",
    "Hefvog",       "Iziar",       "Jazbarg",    "Karog",      "Kolog",
    "Kruxfum",      "Vovog",       "Xalgi",      "Zovoq",      "Zuksag",
    0
};

static const char *giant_female_names[] = {
    "Asdius",       "Clewar",      "Clisor",     "Crerlog",    "Dithor",
    "Demhroq",      "Giwbog",      "Huwfur",     "Javar",      "Jowwor",
    "Larog",        "Lifgant",     "Riwbof",     "Talrion",    "Valas",
    "Vlinrion",     "Vrebog",      "Vrurym",     "Wenfir",     "Xenghaf",
    0
};

static const char *orc_male_names[] = {
    "Agbo",         "Auzzaf",      "Bolgorg",    "Cracbac",    "Crazza",
    "Dergi",        "Durcad",      "Gazru",      "Kozbugh",    "Oddugh",
    "Ogrogh",       "Orboth",      "Rozgi",      "Uglaudh",    "Urdi",
    0
};

static const char *orc_female_names[] = {
    "Athri",        "Auzgut",      "Azrauth",    "Becradh",    "Bridbosh",
    "Crilgi",       "Ghulgaf",     "Gihaush",    "Ohadiz",     "Rardith",
    "Sraglikh",     "Sruzdith",    "Sugdol",     "Udbe",       "Uggudh",
    0
};

static const char *gnome_male_names[] = {
    "Bengnomer",    "Clieddwess",  "Darbick",    "Frialewost", "Hembaz",
    "Hiempess",     "Knoobbrekur", "Slykwass",   "Smoofamurt", "Tivignap",
    0
};

static const char *gnome_female_names[] = {
    "Amanbin",      "Bluprell",    "Clamwal",    "Clekmit",    "Flegbibis",
    "Fobnass",      "Fyhipraal",   "Glidwi",     "Gninklidli", "Pyiqaglim",
    0
};

static const char *hobbit_male_names[] = {
    "Bilbo",        "Blutmund",    "Cyr",        "Faramond",   "Frodo",
    "Nob",          "Theodoric",   "Samwise",    "Uffo",       "Wulfram",
    0
};

static const char *hobbit_female_names[] = {
    "Athalia",      "Bave",        "Gerda",      "Guntheuc",   "Kunegund",
    "Malva",        "Menegilda",   "Merofled",   "Peony",      "Ruothilde",
    0
};

static const char *illithid_male_names[] = {
    "Druphrilt",    "Gzur",        "Gudanilz",   "Kaddudun",   "Reddin",
    "Tagangask",    "Usder",       "Vaggask",    "Vossk",      "Zuzchigoz",
    0
};

static const char *illithid_female_names[] = {
    "Cliloduzak",   "Druzzusk",    "Gusgamiyi",  "Qoss",       "Slilithindi",
    "Tazili",       "Tralq",       "Uthilith",   "Velbobar",   "Viduz",
    0
};

static const char *tortle_male_names[] = {
    "Baka",         "Damu",        "Gar",        "Jappa",      "Krull",
    "Lop",          "Nortle",      "Ploqwat",    "Queg",       "Quott",
    "Ubo",          "Xelbuk",      "Yog",
    0
};

static const char *tortle_female_names[] = {
    "Gura",         "Ini",         "Kinlek",     "Lim",        "Nukla",
    "Olo",          "Quee",        "Tibor",      "Uhok",       "Wabu",
    "Xopa",
    0
};

static const char *drow_male_names[] = {
    "Chanin",       "Dresmorlin",  "Gwydyn",     "Imaufein",   "Nadryn",
    "Ornamorlin",   "Seldszar",    "Torreldril", "Uhlsyln",    "Zaknafein",
    0
};

static const char *drow_female_names[] = {
    "Arduda",       "Balquiri",    "Drisinil",   "Ghirina",    "Jhulssysn",
    "Kiava",        "Nalin",       "Quarstin",   "Rilvra",     "Vierna",
    0
};

static const char *draugr_male_names[] = {
    "Bjorn",        "Erik",        "Gorm",       "Halfdan",    "Leif",
    "Njal",         "Sten",        "Svend",      "Torsten",    "Ulf",
    0
};

static const char *draugr_female_names[] = {
    "Astrid",       "Frida",       "Gunhild",    "Helga",      "Sigrid",
    "Revna",        "Thyra",       "Thurid",     "Yrsa",       "Ulfhild",
    0
};

static const char *vampire_male_names[] = {
    "Antoine",      "Armand",      "Avicus",     "Azim",       "Daniel",
    "David",        "Enkil",       "Eric",       "Khayman",    "Lestat",
    "Louis",        "Mael",        "Magnus",     "Marius",     "Nicolas",
    "Rhoshamandes", "Santiago"     "Santino",    "Seth",       "Teskhamen",
    "Thorne",
    0
};

static const char *vampire_female_names[] = {
    "Akasha",       "Bianca",      "Claudia",    "Eudoxia",    "Gabrielle",
    "Jessica",      "Madeleine",   "Maharet",    "Mekare",     "Merrick",
    "Pandora",
    0
};

struct mfnames {
    const char **male;
    const char **female;
};

/* indexing must match races[] in role.c */
static const struct mfnames namelists[] = {
    { human_male_names, human_female_names },
    { elf_male_names, elf_female_names },
    { dwarf_male_names, dwarf_female_names },
    { gnome_male_names, gnome_female_names },
    { orc_male_names, orc_female_names },
    { giant_male_names, giant_female_names },
    { hobbit_male_names, hobbit_female_names },
    { centaur_male_names, centaur_female_names },
    { illithid_male_names, illithid_female_names },
    { tortle_male_names, tortle_female_names },
    { drow_male_names, drow_female_names },
    { draugr_male_names, draugr_female_names },
    { vampire_male_names, vampire_female_names }
};

void
get_mplname(mtmp, nam)
struct monst *mtmp;
char *nam;
{
    char* ttname = get_rnd_tt_name(TRUE); /* record file */
    const char **mp_names;
    int r_id = 0, ncnt;

    if (has_erac(mtmp) && ERAC(mtmp)->r_id >= 0)
        r_id = ERAC(mtmp)->r_id;

    mp_names = mtmp->female ? namelists[r_id].female : namelists[r_id].male;

    for (ncnt = 0; mp_names[ncnt]; ncnt++)
        ; /* count the number of names in the list */

    Strcpy(nam, (In_endgame(&u.uz) && ttname != 0)
           ? ttname : mp_names[rn2(ncnt)]);
    Strcat(nam, " the ");
    Strcat(nam, rank_of_mplayer((int) mtmp->m_lev, mtmp,
                                (boolean) mtmp->female));
}

void
init_mplayer_erac(mtmp)
struct monst *mtmp;
{
    char nam[PL_PSIZ];
    int race;
    struct erac *rptr;
    int mndx = mtmp->mnum;
    struct permonst *ptr = &mons[mndx];

    newerac(mtmp);
    rptr = ERAC(mtmp);
    rptr->mrace = ptr->mhflags;
    rptr->ralign = ptr->maligntyp;
    memcpy(rptr->mattk, ptr->mattk, sizeof(struct attack) * NATTK);

    /* default player monster attacks.
       [2] slot used for various special
       racial attacks (see mon.c) */
    rptr->mattk[0].aatyp = AT_WEAP;
    rptr->mattk[0].adtyp = AD_PHYS;
    rptr->mattk[0].damn = 1;
    rptr->mattk[0].damd = 6;
    rptr->mattk[1].aatyp = AT_WEAP;
    rptr->mattk[1].adtyp = AD_SAMU;
    rptr->mattk[1].damn = 1;
    rptr->mattk[1].damd = 6;

    race = m_randrace(mndx);
    apply_race(mtmp, race);

    switch (mndx) {
    case PM_ARCHEOLOGIST:
        /* flags for all archeologists regardless of race */
        rptr->mflags1 |= (M1_TUNNEL | M1_NEEDPICK);
        break;
    case PM_BARBARIAN:
        /* flags for all barbarians regardless of race */
        rptr->mflags3 |= M3_BERSERK;
        mtmp->mintrinsics |= MR_POISON;
        /* vampire only */
        if (rptr->mrace == MH_VAMPIRE)
            rptr->mflags1 |= M1_FLY;
        break;
    case PM_CAVEMAN:
    case PM_CAVEWOMAN:
        /* flags for all cavepersons regardless of race */
        /* cavepeople only have a single attack, but it does 2d4 */
        rptr->mattk[0].damn = 2;
        rptr->mattk[0].damd = 4;
        rptr->mattk[0].adtyp = AD_SAMU;
        rptr->mattk[1].aatyp = rptr->mattk[1].adtyp = 0;
        rptr->mattk[1].damn = rptr->mattk[1].damd = 0;
        break;
    case PM_CONVICT:
        /* flags for all convicts regardless of race */
        mtmp->mintrinsics |= MR_POISON;
        /* illithid/vampire only */
        if (rptr->mrace == MH_ILLITHID
            || rptr->mrace == MH_VAMPIRE)
            rptr->mflags1 |= M1_FLY;
        break;
    case PM_DRUID:
        /* flags for all druids regardless of race */
        rptr->mattk[0].adtyp = AD_SAMU;
        rptr->mattk[1].aatyp = AT_MAGC;
        rptr->mattk[1].adtyp = AD_CLRC;
        mtmp->mintrinsics |= MR_SLEEP;
        break;
    case PM_HEALER:
        /* flags for all healers regardless of race */
        rptr->mattk[0].adtyp = AD_SAMU;
        rptr->mattk[1].aatyp = AT_MAGC;
        rptr->mattk[1].adtyp = AD_CLRC;
        mtmp->mintrinsics |= MR_POISON;
        break;
    case PM_INFIDEL:
        /* flags for all infidels regardless of race */
        rptr->mattk[0].adtyp = AD_SAMU;
        if (rptr->mrace == MH_ZOMBIE) {
            rptr->mattk[1].aatyp = AT_CLAW;
            rptr->mattk[1].adtyp = AD_DRCO;
        } else {
            rptr->mattk[1].aatyp = AT_MAGC;
            rptr->mattk[1].adtyp = AD_SPEL;
        }
        /* illithid/vampire only */
        if (rptr->mrace == MH_ILLITHID
            || rptr->mrace == MH_VAMPIRE)
            rptr->mflags1 |= M1_FLY;
        /* all except draugr/vampire */
        if (rptr->mrace != MH_ZOMBIE
            || rptr->mrace != MH_VAMPIRE)
            mtmp->mintrinsics |= MR_FIRE;
        break;
    case PM_KNIGHT:
        if (rptr->mrace == MH_VAMPIRE)
            rptr->mflags1 |= M1_FLY;
        break;
    case PM_MONK:
        /* flags for all monks regardless of race */
        rptr->mattk[0].adtyp = AD_SAMU;
        rptr->mattk[1].aatyp = AT_KICK;
        rptr->mattk[1].adtyp = AD_CLOB;
        /* monks do 1d8 instead of 1d6 */
        rptr->mattk[0].damn = rptr->mattk[1].damn = 1;
        rptr->mattk[0].damd = rptr->mattk[1].damd = 8;
        rptr->mflags1 |= M1_SEE_INVIS;
        mtmp->mintrinsics |= (MR_POISON | MR_SLEEP);
        /* vampire only */
        if (rptr->mrace == MH_VAMPIRE)
            rptr->mflags1 |= M1_FLY;
        break;
    case PM_PRIEST:
    case PM_PRIESTESS:
        /* flags for all priests regardless of race */
        rptr->mattk[0].adtyp = AD_SAMU;
        rptr->mattk[1].aatyp = AT_MAGC;
        rptr->mattk[1].adtyp = AD_CLRC;
        /* illithid only */
        if (rptr->mrace == MH_ILLITHID)
            rptr->mflags1 |= M1_FLY;
        break;
    case PM_RANGER:
        /* flags for all rangers regardless of race */
        rptr->mflags3 |= M3_ACCURATE;
        break;
    case PM_ROGUE:
        /* flags for all rogues regardless of race */
        rptr->mattk[0].adtyp = AD_SAMU;
        rptr->mattk[1].aatyp = AT_CLAW;
        rptr->mattk[1].adtyp = AD_SITM;
        rptr->mattk[1].damn = rptr->mattk[1].damd = 0;
        rptr->mflags3 |= M3_ACCURATE;
        /* vampire only */
        if (rptr->mrace == MH_VAMPIRE)
            rptr->mflags1 |= M1_FLY;
        break;
    case PM_SAMURAI:
        /* flags for all samurai regardless of race */
        /* samurai do 1d8 instead of 1d6 */
        rptr->mattk[0].damn = rptr->mattk[1].damn = 1;
        rptr->mattk[0].damd = rptr->mattk[1].damd = 8;
        break;
    case PM_TOURIST:
        /* nothing special based on role */
        break;
    case PM_VALKYRIE:
        /* flags for all valkyrie regardless of race */
        /* valkyries do 1d8 instead of 1d6 */
        rptr->mattk[0].damn = rptr->mattk[1].damn = 1;
        rptr->mattk[0].damd = rptr->mattk[1].damd = 8;
        mtmp->mintrinsics |= MR_COLD;
        break;
    case PM_WIZARD:
        /* flags for all wizards regardless of race */
        rptr->mattk[0].adtyp = AD_SAMU;
        rptr->mattk[1].aatyp = AT_MAGC;
        rptr->mattk[1].adtyp = AD_SPEL;
        /* illithid/vampire only */
        if (rptr->mrace == MH_ILLITHID
            || rptr->mrace == MH_VAMPIRE)
            rptr->mflags1 |= M1_FLY;
        break;
    default:
        break;
    }

    get_mplname(mtmp, nam);
    mtmp = christen_monst(mtmp, nam);
}

struct monst *
mk_mplayer(ptr, x, y, special)
struct permonst *ptr;
xchar x, y;
boolean special;
{
    struct monst *mtmp;
    boolean ascending = special && (In_endgame(&u.uz) || u.uhave.amulet);
    char nam[PL_PSIZ];

    if (!ptr)
        return ((struct monst *) 0);

    if (MON_AT(x, y))
        (void) rloc(m_at(x, y), FALSE); /* insurance */

    if ((mtmp = makemon(ptr, x, y, MM_MPLAYEROK)) != 0) {
        mtmp->m_lev = (special ? (ascending ? rn1(16, 15)
                                            : min(30, u.ulevel + rn1(4, 4)))
                               : rnd(16));
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

        if (on_level(&purgstart_level, &u.uz) && rn2(2)) {
            (void) mongets(mtmp, RIN_TELEPORT_CONTROL);
            m_dowear(mtmp, FALSE);
        }

        if (ascending) {
            if (!rn2(10))
                (void) mongets(mtmp, rn2(3) ? LUCKSTONE : LOADSTONE);

            /* extra ring */
            if (rn2(8) || monsndx(ptr) == PM_WIZARD) {
                int i, ring;

                for (i = 0; i < 2 && (rn2(2) || monsndx(ptr) == PM_WIZARD); i++) {
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
                    (void) mongets(mtmp, ring);
                }
                m_dowear(mtmp, TRUE);
            }
        }
    }
    return mtmp;
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
int num;
boolean special;
{
    int pm, x, y;
    struct monst fakemon;

    fakemon = zeromonst;
    while (num) {
        int tryct = 0;

        /* roll for character class */
        pm = rn1(PM_WIZARD - PM_ARCHEOLOGIST + 1, PM_ARCHEOLOGIST);
        set_mon_data(&fakemon, &mons[pm]);

        /* roll for an available location */
        do {
            x = rn1(COLNO - 4, 2);
            y = rnd(ROWNO - 2);
        } while (!goodpos(x, y, &fakemon, 0L) && tryct++ <= 500);

        /* if pos not found in 500 tries, don't bother to continue */
        if (tryct > 500)
            return;

        (void) mk_mplayer(&mons[pm], (xchar) x, (xchar) y, special);
        num--;
    }
}

void
mplayer_talk(mtmp)
struct monst *mtmp;
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
