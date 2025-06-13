/* NetHack 3.6	topten.c	$NHDT-Date: 1450451497 2015/12/18 15:11:37 $  $NHDT-Branch: NetHack-3.6.0 $:$NHDT-Revision: 1.44 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2012. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "dlb.h"
#ifdef SHORT_FILENAMES
#include "patchlev.h"
#else
#include "patchlevel.h"
#endif

#ifdef VMS
/* We don't want to rewrite the whole file, because that entails
   creating a new version which requires that the old one be deletable. */
#define UPDATE_RECORD_IN_PLACE
#endif

/*
 * Updating in place can leave junk at the end of the file in some
 * circumstances (if it shrinks and the O.S. doesn't have a straightforward
 * way to truncate it).  The trailing junk is harmless and the code
 * which reads the scores will ignore it.
 */
#ifdef UPDATE_RECORD_IN_PLACE
static long final_fpos;
#endif

#define done_stopprint program_state.stopprint

#define newttentry() (struct toptenentry *) alloc(sizeof (struct toptenentry))
#define dealloc_ttentry(ttent) free((genericptr_t) (ttent))
#ifndef NAMSZ
/* Changing NAMSZ can break your existing record/logfile */
#define NAMSZ PL_NSIZ
#endif
#define DTHSZ 100
#define ROLESZ 3

struct toptenentry {
    struct toptenentry *tt_next;
#ifdef UPDATE_RECORD_IN_PLACE
    long fpos;
#endif
    long points;
    int deathdnum, deathlev;
    int maxlvl, hp, maxhp, deaths;
    int ver_major, ver_minor, patchlevel;
    long deathdate, birthdate;
    int uid;
    char plrole[ROLESZ + 1];
    char plrace[ROLESZ + 1];
    char plgend[ROLESZ + 1];
    char plalign[ROLESZ + 1];
    char name[NAMSZ + 1];
    char death[DTHSZ + 1];
} * tt_head;
/* size big enough to read in all the string fields at once; includes
   room for separating space or trailing newline plus string terminator */
#define SCANBUFSZ (4 * (ROLESZ + 1) + (NAMSZ + 1) + (DTHSZ + 1) + 1)

STATIC_DCL void FDECL(topten_print, (const char *));
STATIC_DCL void FDECL(topten_print_bold, (const char *));
STATIC_DCL void NDECL(outheader);
STATIC_DCL void FDECL(outentry, (int, struct toptenentry *, BOOLEAN_P));
STATIC_DCL void FDECL(discardexcess, (FILE *));
STATIC_DCL void FDECL(readentry, (FILE *, struct toptenentry *));
STATIC_DCL void FDECL(writeentry, (FILE *, struct toptenentry *));
#ifdef XLOGFILE
STATIC_DCL void FDECL(writexlentry, (FILE *, struct toptenentry *, int));
STATIC_DCL long NDECL(encodexlogflags);
STATIC_DCL long NDECL(encodeconduct);
STATIC_DCL long NDECL(encodeachieve);
static void FDECL(add_achieveX, (char *, const char *, BOOLEAN_P));
static char *NDECL(encode_extended_achievements);
static char *NDECL(encode_extended_conducts);
#endif
STATIC_DCL void FDECL(free_ttlist, (struct toptenentry *));
STATIC_DCL int FDECL(classmon, (char *, BOOLEAN_P));
STATIC_DCL int FDECL(raceinfo, (char *, BOOLEAN_P));
STATIC_DCL boolean FDECL(name_unused_on_lvl, (const char *));
STATIC_DCL int FDECL(score_wanted, (BOOLEAN_P, int, struct toptenentry *, int,
                                    const char **, int));
#ifdef NO_SCAN_BRACK
STATIC_DCL void FDECL(nsb_mung_line, (char *));
STATIC_DCL void FDECL(nsb_unmung_line, (char *));
#endif

/* tracking of various killed unique monsters
   from UnNetHack */
static char* FDECL(killed_uniques, (void));

static winid toptenwin = WIN_ERR;

/* "killed by",&c ["an"] 'killer.name' */
void
formatkiller(buf, siz, how, incl_helpless)
char *buf;
unsigned siz;
int how;
boolean incl_helpless;
{
    static NEARDATA const char *const killed_by_prefix[] = {
        /* DIED, CHOKING, POISONING, STARVING, */
        "killed by ", "choked on ", "poisoned by ", "died of ",
        /* DROWNING, BURNING, DISSOLVED, CRUSHING, */
        "drowned in ", "burned by ", "dissolved in ", "crushed to death by ",
        /* STONING, TURNED_SLIME, DECAPITATED, */
        "petrified by ", "turned to slime by ", "decapitated by ",
        /* INCINERATED, DISINTEGRATED, GENOCIDED, */
        "incinerated by ", "disintegrated by ", "killed by ",
        /* PANICKED, TRICKED, QUIT, ESCAPED, ASCENDED */
        "", "", "", "", ""
    };
    unsigned l;
    char c, *kname = killer.name;

    buf[0] = '\0'; /* lint suppression */
    switch (killer.format) {
    default:
        impossible("bad killer format? (%d)", killer.format);
        /*FALLTHRU*/
    case NO_KILLER_PREFIX:
        break;
    case KILLED_BY_AN:
        kname = an(kname);
        /*FALLTHRU*/
    case KILLED_BY:
        (void) strncat(buf, killed_by_prefix[how], siz - 1);
        l = strlen(buf);
        buf += l, siz -= l;
        break;
    }
    /* Copy kname into buf[].
     * Object names and named fruit have already been sanitized, but
     * monsters can have "called 'arbitrary text'" attached to them,
     * so make sure that that text can't confuse field splitting when
     * record, logfile, or xlogfile is re-read at some later point.
     */
    while (--siz > 0) {
        c = *kname++;
        if (!c)
            break;
        else if (c == ',')
            c = ';';
        /* 'xlogfile' doesn't really need protection for '=', but
           fixrecord.awk for corrupted 3.6.0 'record' does (only
           if using xlogfile rather than logfile to repair record) */
        else if (c == '=')
            c = '_';
        /* tab is not possible due to use of mungspaces() when naming;
           it would disrupt xlogfile parsing if it were present */
        else if (c == '\t')
            c = ' ';
        *buf++ = c;
    }
    *buf = '\0';

    if (incl_helpless) {
        if (multi) {
            /* X <= siz: 'sizeof "string"' includes 1 for '\0' terminator */
            if (multi_reason
                && strlen(multi_reason) + sizeof ", while " <= siz)
                Sprintf(buf, ", while %s", multi_reason);
            /* either multi_reason wasn't specified or wouldn't fit */
            else if (sizeof ", while helpless" <= siz)
                Strcpy(buf, ", while helpless");
            /* else extra death info won't fit, so leave it out */
        } else if (Hidinshell
                   && sizeof ", while hiding in her shell" <= siz) {
            Sprintf(buf, ", while hiding in %s shell", uhis());
        }
    }
}

STATIC_OVL void
topten_print(x)
const char *x;
{
    if (toptenwin == WIN_ERR)
        raw_print(x);
    else
        putstr(toptenwin, ATR_NONE, x);
}

STATIC_OVL void
topten_print_bold(x)
const char *x;
{
    if (toptenwin == WIN_ERR)
        raw_print_bold(x);
    else
        putstr(toptenwin, ATR_BOLD, x);
}

int
observable_depth(lev)
d_level *lev;
{
    /* if we ever randomize the order of the elemental planes, we
       must use a constant external representation in the record file */
    if (In_endgame(lev)) {
        if (Is_astralevel(lev))
            return -5;
        else if (Is_waterlevel(lev))
            return -4;
        else if (Is_firelevel(lev))
            return -3;
        else if (Is_airlevel(lev))
            return -2;
        else if (Is_earthlevel(lev))
            return -1;
        else
            return 0; /* ? */
    } else
        return depth(lev);
}

/* throw away characters until current record has been entirely consumed */
STATIC_OVL void
discardexcess(rfile)
FILE *rfile;
{
    int c;

    do {
        c = fgetc(rfile);
    } while (c != '\n' && c != EOF);
}

#define use_newfmt \
    (tt->ver_major > 0 || tt->ver_minor > 7 \
     || (tt->ver_minor == 7 && tt->patchlevel > 1))

STATIC_OVL void
readentry(rfile, tt)
FILE *rfile;
struct toptenentry *tt;
{
    char inbuf[SCANBUFSZ], s1[SCANBUFSZ], s2[SCANBUFSZ], s3[SCANBUFSZ],
        s4[SCANBUFSZ], s5[SCANBUFSZ], s6[SCANBUFSZ];

#ifdef NO_SCAN_BRACK /* Version_ Pts DgnLevs_ Hp___ Died__Born id */
    static const char fmt[] = "%d %d %d %ld %d %d %d %d %d %d %ld %ld %d%*c";
    static const char fmt32[] = "%c%c %s %s%*c";
    static const char fmt33[] = "%s %s %s %s %s %s%*c";
#else
    static const char fmt[] = "%d.%d.%d %ld %d %d %d %d %d %d %ld %ld %d ";
    static const char fmt32[] = "%c%c %[^,],%[^\n]%*c";
    static const char fmt33[] = "%s %s %s %s %[^,],%[^\n]%*c";
#endif

#ifdef UPDATE_RECORD_IN_PLACE
    /* note: input below must read the record's terminating newline */
    final_fpos = tt->fpos = ftell(rfile);
#endif
#define TTFIELDS 13
    if (fscanf(rfile, fmt, &tt->ver_major, &tt->ver_minor, &tt->patchlevel,
               &tt->points, &tt->deathdnum, &tt->deathlev, &tt->maxlvl,
               &tt->hp, &tt->maxhp, &tt->deaths, &tt->deathdate,
               &tt->birthdate, &tt->uid) != TTFIELDS) {
#undef TTFIELDS
        tt->points = 0;
        discardexcess(rfile);
    } else {
        /* load remainder of record into a local buffer;
           this imposes an implicit length limit of SCANBUFSZ
           on every string field extracted from the buffer */
        if (!fgets(inbuf, sizeof inbuf, rfile)) {
            /* sscanf will fail and tt->points will be set to 0 */
            *inbuf = '\0';
        } else if (!index(inbuf, '\n')) {
            Strcpy(&inbuf[sizeof inbuf - 2], "\n");
            discardexcess(rfile);
        }
        /* Check for backwards compatibility */
        if (!use_newfmt) {
            int i;

            if (sscanf(inbuf, fmt32, tt->plrole, tt->plgend, s1, s2) == 4) {
                tt->plrole[1] = tt->plgend[1] = '\0'; /* read via %c */
                copynchars(tt->name, s1, (int) (sizeof tt->name) - 1);
                copynchars(tt->death, s2, (int) (sizeof tt->death) - 1);
            } else
                tt->points = 0;
            tt->plrole[1] = '\0';
            if ((i = str2role(tt->plrole)) >= 0)
                Strcpy(tt->plrole, roles[i].filecode);
            Strcpy(tt->plrace, "?");
            Strcpy(tt->plgend, (tt->plgend[0] == 'M') ? "Mal" : "Fem");
            Strcpy(tt->plalign, "?");
        } else if (sscanf(inbuf, fmt33, s1, s2, s3, s4, s5, s6) == 6) {
            copynchars(tt->plrole, s1, (int) (sizeof tt->plrole) - 1);
            copynchars(tt->plrace, s2, (int) (sizeof tt->plrace) - 1);
            copynchars(tt->plgend, s3, (int) (sizeof tt->plgend) - 1);
            copynchars(tt->plalign, s4, (int) (sizeof tt->plalign) - 1);
            copynchars(tt->name, s5, (int) (sizeof tt->name) - 1);
            copynchars(tt->death, s6, (int) (sizeof tt->death) - 1);
        } else
            tt->points = 0;
#ifdef NO_SCAN_BRACK
        if (tt->points > 0) {
            nsb_unmung_line(tt->name);
            nsb_unmung_line(tt->death);
        }
#endif
    }

    /* check old score entries for Y2K problem and fix whenever found */
    if (tt->points > 0) {
        if (tt->birthdate < 19000000L)
            tt->birthdate += 19000000L;
        if (tt->deathdate < 19000000L)
            tt->deathdate += 19000000L;
    }
}

STATIC_OVL void
writeentry(rfile, tt)
FILE *rfile;
struct toptenentry *tt;
{
    static const char fmt32[] = "%c%c ";        /* role,gender */
    static const char fmt33[] = "%s %s %s %s "; /* role,race,gndr,algn */
#ifndef NO_SCAN_BRACK
    static const char fmt0[] = "%d.%d.%d %ld %d %d %d %d %d %d %ld %ld %d ";
    static const char fmtX[] = "%s,%s\n";
#else /* NO_SCAN_BRACK */
    static const char fmt0[] = "%d %d %d %ld %d %d %d %d %d %d %ld %ld %d ";
    static const char fmtX[] = "%s %s\n";

    nsb_mung_line(tt->name);
    nsb_mung_line(tt->death);
#endif

    (void) fprintf(rfile, fmt0, tt->ver_major, tt->ver_minor, tt->patchlevel,
                   tt->points, tt->deathdnum, tt->deathlev, tt->maxlvl,
                   tt->hp, tt->maxhp, tt->deaths, tt->deathdate,
                   tt->birthdate, tt->uid);
    if (!use_newfmt)
        (void) fprintf(rfile, fmt32, tt->plrole[0], tt->plgend[0]);
    else
        (void) fprintf(rfile, fmt33, tt->plrole, tt->plrace, tt->plgend,
                       tt->plalign);
    (void) fprintf(rfile, fmtX, onlyspace(tt->name) ? "_" : tt->name,
                   tt->death);

#ifdef NO_SCAN_BRACK
    nsb_unmung_line(tt->name);
    nsb_unmung_line(tt->death);
#endif
}

#undef use_newfmt

#ifdef XLOGFILE

/* as tab is never used in eg. plname or death, no need to mangle those. */
STATIC_OVL void
writexlentry(rfile, tt, how)
FILE *rfile;
struct toptenentry *tt;
int how;
{
#define Fprintf (void) fprintf
#define XLOG_SEP '\t' /* xlogfile field separator. */
    char buf[BUFSZ], tmpbuf[DTHSZ + 1];

    Sprintf(buf, "version=%d.%d.%d", tt->ver_major, tt->ver_minor,
            tt->patchlevel);
    Sprintf(eos(buf), "%cpoints=%ld%cdeathdnum=%d%cdeathlev=%d", XLOG_SEP,
            tt->points, XLOG_SEP, tt->deathdnum, XLOG_SEP, tt->deathlev);
    Sprintf(eos(buf), "%cmaxlvl=%d%chp=%d%cmaxhp=%d", XLOG_SEP, tt->maxlvl,
            XLOG_SEP, tt->hp, XLOG_SEP, tt->maxhp);
    Sprintf(eos(buf), "%cdeaths=%d%cdeathdate=%ld%cbirthdate=%ld%cuid=%d",
            XLOG_SEP, tt->deaths, XLOG_SEP, tt->deathdate, XLOG_SEP,
            tt->birthdate, XLOG_SEP, tt->uid);
    Fprintf(rfile, "%s", buf);
    Sprintf(buf, "%crole=%s%crace=%s%cgender=%s%calign=%s", XLOG_SEP,
            tt->plrole, XLOG_SEP, tt->plrace, XLOG_SEP, tt->plgend, XLOG_SEP,
            tt->plalign);
    /* make a copy of death reason that doesn't include ", while helpless" */
    formatkiller(tmpbuf, sizeof tmpbuf, how, FALSE);
    Fprintf(rfile, "%s%cname=%s%cdeath=%s",
            buf, /* (already includes separator) */
            XLOG_SEP, plname, XLOG_SEP, tmpbuf);
    if (multi)
        Fprintf(rfile, "%cwhile=%s", XLOG_SEP,
                multi_reason ? multi_reason : "helpless");
    else if (Hidinshell)
        Fprintf(rfile, "%cwhile=hiding in %s shell", XLOG_SEP, uhis());
    Fprintf(rfile, "%cconduct=0x%lx%cturns=%ld%cachieve=0x%lx", XLOG_SEP,
            encodeconduct(), XLOG_SEP, moves, XLOG_SEP, encodeachieve());
    Fprintf(rfile, "%crealtime=%ld%cstarttime=%ld%cendtime=%ld", XLOG_SEP,
            (long) urealtime.realtime, XLOG_SEP,
            (long) ubirthday, XLOG_SEP, (long) urealtime.finish_time);
    Fprintf(rfile, "%cgender0=%s%calign0=%s%crace0=%s", XLOG_SEP,
            genders[flags.initgend].filecode, XLOG_SEP,
            aligns[u.ualignbase[A_ORIGINAL] == A_NONE
                   ? 3 : 1 - sgn(u.ualignbase[A_ORIGINAL])].filecode,
            XLOG_SEP, races[flags.initrace].filecode);
    Fprintf(rfile, "%cflags=0x%lx", XLOG_SEP, encodexlogflags());
    Fprintf(rfile, "%cgold=%ld", XLOG_SEP, money_cnt(invent) + hidden_gold());
    Fprintf(rfile, "%cwish_cnt=%ld", XLOG_SEP, u.uconduct.wishes);
    Fprintf(rfile, "%carti_wish_cnt=%ld", XLOG_SEP, u.uconduct.wisharti);

    /* unique/special monster types */
    Fprintf(rfile, "%ckilled_uniques=%s", XLOG_SEP, killed_uniques());
    Fprintf(rfile, "%ckilled_nazgul=%d", XLOG_SEP, mvitals[PM_NAZGUL].died);
    Fprintf(rfile, "%ckilled_erinyes=%d", XLOG_SEP, mvitals[PM_ERINYS].died);
    Fprintf(rfile, "%ckilled_archangels=%d", XLOG_SEP, mvitals[PM_ARCHANGEL].died);

    /* extended achievements and conducts */
    Fprintf(rfile, "%cachieveX=%s", XLOG_SEP, encode_extended_achievements());
    Fprintf(rfile, "%cconductX=%s", XLOG_SEP, encode_extended_conducts());

    Fprintf(rfile, "\n");

#undef XLOG_SEP
}

/* add the achievement or conduct comma-separated to string */
static void
add_achieveX(buf, achievement, condition)
char *buf;
const char *achievement;
boolean condition;
{
    if (condition) {
        if (buf[0] != '\0')
            Strcat(buf, ",");
        Strcat(buf, achievement);
    }
}

static char *
encode_extended_achievements()
{
    static char buf[30*40];

    buf[0] = '\0';
    /* the original 12 xlogfile achievements */
    add_achieveX(buf, "ascended", u.uevent.ascended);
    add_achieveX(buf, "entered_astral_plane", Is_astralevel(&u.uz));
    add_achieveX(buf, "entered_elemental_planes", In_endgame(&u.uz));
    add_achieveX(buf, "obtained_the_amulet_of_yendor", u.uachieve.amulet);
    add_achieveX(buf, "performed_the_invocation_ritual", u.uevent.invoked);
    add_achieveX(buf, "obtained_the_book_of_the_dead", u.uachieve.book);
    add_achieveX(buf, "obtained_the_bell_of_opening", u.uachieve.bell);
    add_achieveX(buf, "obtained_the_candelabrum_of_invocation", u.uachieve.menorah);
    add_achieveX(buf, "entered_gehennom", u.uachieve.enter_gehennom);
    add_achieveX(buf, "entered_purgatory", u.uachieve.enter_purgatory);
    add_achieveX(buf, "entered_hidden_dungeon", u.uachieve.enter_hdgn);
    add_achieveX(buf, "entered_wiztower", u.uachieve.enter_wiztower);
    add_achieveX(buf, "defeated_medusa", u.uachieve.killed_medusa);
    add_achieveX(buf, "obtained_the_luckstone_from_the_mines", u.uachieve.mines_luckstone);
    add_achieveX(buf, "obtained_the_sokoban_prize", u.uachieve.finish_sokoban);

    /* oracle */
    add_achieveX(buf, "consulted_the_oracle", (u.uevent.minor_oracle || u.uevent.major_oracle));
    add_achieveX(buf, "got_minor_oracle_consultation", u.uevent.minor_oracle);
    add_achieveX(buf, "got_major_oracle_consultation", u.uevent.major_oracle);

    /* quest */
    add_achieveX(buf, "entered_quest_portal_level", u.uevent.qcalled);
    add_achieveX(buf, "accepted_for_quest", (quest_status.got_quest || quest_status.got_thanks));
    add_achieveX(buf, "defeated_quest_nemesis", quest_status.killed_nemesis);
    add_achieveX(buf, "defeated_quest_leader", quest_status.leader_is_dead);
    add_achieveX(buf, "quest_completed", u.uevent.qcompleted);

    /* other notable achievements */
    add_achieveX(buf, "defeated_ice_queen", u.uachieve.defeat_icequeen);
    add_achieveX(buf, "defeated_cerberus", u.uachieve.killed_cerberus);
    add_achieveX(buf, "defeated_vecna", u.uachieve.killed_vecna);
    add_achieveX(buf, "defeated_vlad_impaler", u.uachieve.killed_vlad);
    add_achieveX(buf, "defeated_talgath", u.uachieve.killed_talgath);
    add_achieveX(buf, "defeated_goblin_king", u.uachieve.killed_gking);
    add_achieveX(buf, "defeated_lucifer", u.uachieve.killed_lucifer);
    add_achieveX(buf, "defeated_saint_michael", u.uachieve.killed_michael);
    add_achieveX(buf, "got_crowned", u.uevent.uhand_of_elbereth);

#if 0
    /* TODO 3.7 achievements
    add_achieveX(buf, "entered_the_gnomish_mines",
    add_achieveX(buf, "entered_mine_town",
    add_achieveX(buf, "entered_a_shop",
    add_achieveX(buf, "entered_a_temple",
    add_achieveX(buf, "entered_sokoban",
    add_achieveX(buf, "entered_bigroom",
    */
#endif

    return buf;
}

static char *
encode_extended_conducts()
{
    static char buf[350]; /* XXX: Expand this when adding new conducts */

    buf[0] = '\0';
    add_achieveX(buf, "foodless",                        !u.uconduct.food);
    add_achieveX(buf, "vegan",                           !u.uconduct.unvegan);
    add_achieveX(buf, "vegetarian",                      !u.uconduct.unvegetarian);
    add_achieveX(buf, "atheist",                         !u.uconduct.gnostic);
    add_achieveX(buf, "weaponless",                      !u.uconduct.weaphit);
    add_achieveX(buf, "pacifist",                        !u.uconduct.killer);
    add_achieveX(buf, "illiterate",                      !u.uconduct.literate);
    add_achieveX(buf, "polyless",                        !u.uconduct.polypiles);
    add_achieveX(buf, "polyselfless",                    !u.uconduct.polyselfs);
    add_achieveX(buf, "wishless",                        !u.uconduct.wishes);
    add_achieveX(buf, "artiwishless",                    !u.uconduct.wisharti);
    add_achieveX(buf, "genocideless",                    !num_genocides());
    add_achieveX(buf, "never_had_a_pet",                 !u.uconduct.pets);
    add_achieveX(buf, "never_touched_an_artifact",       !u.uconduct.artitouch);
    add_achieveX(buf, "elberethless",                    !u.uconduct.elbereth);
    add_achieveX(buf, "blindfolded",                      u.uroleplay.blind);
    add_achieveX(buf, "nudist",                           u.uroleplay.nudist);
    add_achieveX(buf, "hallucinating",                    u.uroleplay.hallu);
    add_achieveX(buf, "deaf",                             u.uroleplay.deaf);
    add_achieveX(buf, "bonesless",                       !u.uroleplay.numbones);
    add_achieveX(buf, "never_died",                       u.umortality == 0);
    add_achieveX(buf, "never_abused_alignment",           u.ualign.abuse == 0);
    add_achieveX(buf, "never_forged_an_artifact",        !u.uconduct.forgedarti);
    add_achieveX(buf, "never_acquired_magic_resistance", !u.uconduct.antimagic);
    add_achieveX(buf, "never_acquired_reflection",       !u.uconduct.reflection);
    /* NB: when adding new conducts, increase the size of buf[] above based on
     * the length of the conduct string */

    return buf;
}

STATIC_OVL long
encodexlogflags()
{
    long e = 0L;

    if (wizard)
        e |= 1L << 0;
    if (discover)
        e |= 1L << 1;
    /* Reserve the 9th through 16th bits as their own number representing the
     * number of bones files loaded. */
    if (u.uroleplay.numbones > 0xFF) {
        impossible("more than 255 bones files loaded?");
        u.uroleplay.numbones = 0xFF;
    }
    e |= u.uroleplay.numbones << 8;

    return e;
}

STATIC_OVL long
encodeconduct()
{
    long e = 0L;

    if (!u.uconduct.food)
        e |= 1L << 0;
    if (!u.uconduct.unvegan)
        e |= 1L << 1;
    if (!u.uconduct.unvegetarian)
        e |= 1L << 2;
    if (!u.uconduct.gnostic)
        e |= 1L << 3;
    if (!u.uconduct.weaphit)
        e |= 1L << 4;
    if (!u.uconduct.killer)
        e |= 1L << 5;
    if (!u.uconduct.literate)
        e |= 1L << 6;
    if (!u.uconduct.polypiles)
        e |= 1L << 7;
    if (!u.uconduct.polyselfs)
        e |= 1L << 8;
    if (!u.uconduct.wishes)
        e |= 1L << 9;
    if (!u.uconduct.wisharti)
        e |= 1L << 10;
    if (!num_genocides())
        e |= 1L << 11;
    if (!u.uconduct.pets)
        e |= 1L << 12;
    if (!u.uconduct.artitouch)
        e |= 1L << 13;
    if (!u.uconduct.elbereth)
        e |= 1L << 14;
    if (u.uroleplay.blind)
        e |= 1L << 15;
    if (u.uroleplay.nudist)
        e |= 1L << 16;
    if (u.uroleplay.hallu)
        e |= 1L << 17;
    if (u.uroleplay.deaf)
        e |= 1L << 18;
    if (u.umortality == 0)
        e |= 1L << 19;
    if (u.ualign.abuse == 0)
        e |= 1L << 20;
    if (!u.uconduct.forgedarti)
        e |= 1L << 21;
    if (!u.uconduct.antimagic)
        e |= 1L << 22;
    if (!u.uconduct.reflection)
        e |= 1L << 23;

    return e;
}

STATIC_OVL long
encodeachieve()
{
    long r = 0L;

    if (u.uachieve.bell)
        r |= 1L << 0;
    if (u.uachieve.enter_gehennom)
        r |= 1L << 1;
    if (u.uachieve.menorah)
        r |= 1L << 2;
    if (u.uachieve.book)
        r |= 1L << 3;
    if (u.uevent.invoked)
        r |= 1L << 4;
    if (u.uachieve.amulet)
        r |= 1L << 5;
    if (In_endgame(&u.uz))
        r |= 1L << 6;
    if (Is_astralevel(&u.uz))
        r |= 1L << 7;
    if (u.uachieve.ascended)
        r |= 1L << 8;
    if (u.uachieve.mines_luckstone)
        r |= 1L << 9;
    if (u.uachieve.finish_sokoban)
        r |= 1L << 10;
    if (u.uachieve.killed_medusa)
        r |= 1L << 11;
    if (u.uachieve.defeat_icequeen)
        r |= 1L << 12;
    if (u.uevent.minor_oracle)
        r |= 1L << 13;
    if (u.uevent.major_oracle)
        r |= 1L << 14;
    if (u.uachieve.killed_vecna)
        r |= 1L << 15;
    if (u.uachieve.killed_cerberus)
        r |= 1L << 16;
    if (u.uachieve.killed_gking)
        r |= 1L << 17;
    if (u.uachieve.killed_lucifer)
        r |= 1L << 18;
    if (u.uachieve.enter_purgatory)
        r |= 1L << 19;
    if (u.uachieve.killed_vlad)
        r |= 1L << 20;
    if (u.uachieve.killed_michael)
        r |= 1L << 21;
    if (u.uachieve.enter_wiztower)
        r |= 1L << 22;
    if (u.uachieve.enter_hdgn)
        r |= 1L << 23;
    if (u.uachieve.killed_talgath)
        r |= 1L << 24;

    return r;
}

#endif /* XLOGFILE */

static char _killed_uniques[640];
static char*
killed_uniques(void)
{
    int i, len;
    _killed_uniques[0] = '\0';

    for (i = LOW_PM; i < NUMMONS; i++) {
	if ((mons[i].geno & G_UNIQ) && mvitals[i].died) {
    	    if (i == PM_LONG_WORM_TAIL)
                continue;
	    if (i == PM_HIGH_PRIEST)
                continue;
	    Sprintf(eos(_killed_uniques), "%s,", mons[i].mname);
	}
    }

    if ((len = strlen(_killed_uniques))) {
        _killed_uniques[len-1] = '\0';
    }

    return _killed_uniques;
}

STATIC_OVL void
free_ttlist(tt)
struct toptenentry *tt;
{
    struct toptenentry *ttnext;

    while (tt->points > 0) {
        ttnext = tt->tt_next;
        dealloc_ttentry(tt);
        tt = ttnext;
    }
    dealloc_ttentry(tt);
}

void
topten(how, when)
int how;
time_t when;
{
    int uid = getuid();
    int rank, rank0 = -1, rank1 = 0;
    int occ_cnt = sysopt.persmax;
    register struct toptenentry *t0, *tprev;
    struct toptenentry *t1;
    FILE *rfile;
    register int flg = 0;
    boolean t0_used;
#ifdef LOGFILE
    FILE *lfile;
#endif /* LOGFILE */
#ifdef XLOGFILE
    FILE *xlfile;
#endif /* XLOGFILE */

#ifdef _DCC
    /* Under DICE 3.0, this crashes the system consistently, apparently due to
     * corruption of *rfile somewhere.  Until I figure this out, just cut out
     * topten support entirely - at least then the game exits cleanly.  --AC
     */
    return;
#endif

    /* If we are in the midst of a panic, cut out topten entirely.
     * topten uses alloc() several times, which will lead to
     * problems if the panic was the result of an alloc() failure.
     */
    if (program_state.panicking)
        return;

    if (iflags.toptenwin) {
        toptenwin = create_nhwindow(NHW_TEXT);
    }

#if defined(UNIX) || defined(VMS) || defined(__EMX__)
#define HUP if (!program_state.done_hup)
#else
#define HUP
#endif

#ifdef TOS
    restore_colors(); /* make sure the screen is black on white */
#endif
    /* create a new 'topten' entry */
    t0_used = FALSE;
    t0 = newttentry();
    t0->ver_major = VERSION_MAJOR;
    t0->ver_minor = VERSION_MINOR;
    t0->patchlevel = PATCHLEVEL;
    t0->points = u.urexp;
    t0->deathdnum = u.uz.dnum;
    /* deepest_lev_reached() is in terms of depth(), and reporting the
     * deepest level reached in the dungeon death occurred in doesn't
     * seem right, so we have to report the death level in depth() terms
     * as well (which also seems reasonable since that's all the player
     * sees on the screen anyway)
     */
    t0->deathlev = observable_depth(&u.uz);
    t0->maxlvl = deepest_lev_reached(TRUE);
    t0->hp = u.uhp;
    t0->maxhp = u.uhpmax;
    t0->deaths = u.umortality;
    t0->uid = uid;
    copynchars(t0->plrole, urole.filecode, ROLESZ);
    copynchars(t0->plrace, urace.filecode, ROLESZ);
    copynchars(t0->plgend, genders[flags.female].filecode, ROLESZ);
    copynchars(t0->plalign,
               aligns[u.ualign.type == A_NONE ? 3 : 1 - u.ualign.type].filecode, ROLESZ);
    copynchars(t0->name, plname, NAMSZ);
    formatkiller(t0->death, sizeof t0->death, how, TRUE);
    t0->birthdate = yyyymmdd(ubirthday);
    t0->deathdate = yyyymmdd(when);
    t0->tt_next = 0;
#ifdef UPDATE_RECORD_IN_PLACE
    t0->fpos = -1L;
#endif

#ifdef LOGFILE /* used for debugging (who dies of what, where) */
    if (lock_file(LOGFILE, SCOREPREFIX, 10)) {
        if (!(lfile = fopen_datafile(LOGFILE, "a", SCOREPREFIX))) {
            HUP raw_print("Cannot open log file!");
        } else {
            writeentry(lfile, t0);
            (void) fclose(lfile);
        }
        unlock_file(LOGFILE);
    }
#endif /* LOGFILE */
#ifdef XLOGFILE
    if (lock_file(XLOGFILE, SCOREPREFIX, 10)) {
        if (!(xlfile = fopen_datafile(XLOGFILE, "a", SCOREPREFIX))) {
            HUP raw_print("Cannot open extended log file!");
        } else {
            writexlentry(xlfile, t0, how);
            (void) fclose(xlfile);
        }
        unlock_file(XLOGFILE);
    }
#endif /* XLOGFILE */

    if (wizard || discover) {
        if (how != PANICKED)
            HUP {
                char pbuf[BUFSZ];

                topten_print("");
                Sprintf(pbuf,
             "Since you were in %s mode, the score list will not be checked.",
                        wizard ? "wizard" : "discover");
                topten_print(pbuf);
            }
        goto showwin;
    }

    if (!lock_file(RECORD, SCOREPREFIX, 60))
        goto destroywin;

#ifdef UPDATE_RECORD_IN_PLACE
    rfile = fopen_datafile(RECORD, "r+", SCOREPREFIX);
#else
    rfile = fopen_datafile(RECORD, "r", SCOREPREFIX);
#endif

    if (!rfile) {
        HUP raw_print("Cannot open record file!");
        unlock_file(RECORD);
        goto destroywin;
    }

    HUP topten_print("");

    /* assure minimum number of points */
    if (t0->points < sysopt.pointsmin)
        t0->points = 0;

    t1 = tt_head = newttentry();
    tprev = 0;
    /* rank0: -1 undefined, 0 not_on_list, n n_th on list */
    for (rank = 1;;) {
        readentry(rfile, t1);
        if (t1->points < sysopt.pointsmin)
            t1->points = 0;
        if (rank0 < 0 && t1->points < t0->points) {
            rank0 = rank++;
            if (tprev == 0)
                tt_head = t0;
            else
                tprev->tt_next = t0;
            t0->tt_next = t1;
#ifdef UPDATE_RECORD_IN_PLACE
            t0->fpos = t1->fpos; /* insert here */
#endif
            t0_used = TRUE;
            occ_cnt--;
            flg++; /* ask for a rewrite */
        } else
            tprev = t1;

        if (t1->points == 0)
            break;
        if ((sysopt.pers_is_uid ? t1->uid == t0->uid
                                : strncmp(t1->name, t0->name, NAMSZ) == 0)
            && !strncmp(t1->plrole, t0->plrole, ROLESZ) && --occ_cnt <= 0) {
            if (rank0 < 0) {
                rank0 = 0;
                rank1 = rank;
                HUP {
                    char pbuf[BUFSZ];

                    Sprintf(pbuf,
                        "You didn't beat your previous score of %ld points.",
                            t1->points);
                    topten_print(pbuf);
                    topten_print("");
                }
            }
            if (occ_cnt < 0) {
                flg++;
                continue;
            }
        }
        if (rank <= sysopt.entrymax) {
            t1->tt_next = newttentry();
            t1 = t1->tt_next;
            rank++;
        }
        if (rank > sysopt.entrymax) {
            t1->points = 0;
            break;
        }
    }
    if (flg) { /* rewrite record file */
#ifdef UPDATE_RECORD_IN_PLACE
        (void) fseek(rfile, (t0->fpos >= 0 ? t0->fpos : final_fpos),
                     SEEK_SET);
#else
        (void) fclose(rfile);
        if (!(rfile = fopen_datafile(RECORD, "w", SCOREPREFIX))) {
            HUP raw_print("Cannot write record file");
            unlock_file(RECORD);
            free_ttlist(tt_head);
            goto destroywin;
        }
#endif /* UPDATE_RECORD_IN_PLACE */
        if (!done_stopprint)
            if (rank0 > 0) {
                if (rank0 <= 10) {
                    topten_print("You made the top ten list!");
                } else {
                    char pbuf[BUFSZ];

                    Sprintf(pbuf,
                            "You reached the %d%s place on the top %d list.",
                            rank0, ordin(rank0), sysopt.entrymax);
                    topten_print(pbuf);
                }
                topten_print("");
            }
    }
    if (rank0 == 0)
        rank0 = rank1;
    if (rank0 <= 0)
        rank0 = rank;
    if (!done_stopprint)
        outheader();
    t1 = tt_head;
    for (rank = 1; t1->points != 0; rank++, t1 = t1->tt_next) {
        if (flg
#ifdef UPDATE_RECORD_IN_PLACE
            && rank >= rank0
#endif
            )
            writeentry(rfile, t1);
        if (done_stopprint)
            continue;
        if (rank > flags.end_top && (rank < rank0 - flags.end_around
                                     || rank > rank0 + flags.end_around)
            && (!flags.end_own
                || (sysopt.pers_is_uid
                        ? t1->uid == t0->uid
                        : strncmp(t1->name, t0->name, NAMSZ) == 0)))
            continue;
        if (rank == rank0 - flags.end_around
            && rank0 > flags.end_top + flags.end_around + 1 && !flags.end_own)
            topten_print("");
        if (rank != rank0)
            outentry(rank, t1, FALSE);
        else if (!rank1)
            outentry(rank, t1, TRUE);
        else {
            outentry(rank, t1, TRUE);
            outentry(0, t0, TRUE);
        }
    }
    if (rank0 >= rank)
        if (!done_stopprint)
            outentry(0, t0, TRUE);
#ifdef UPDATE_RECORD_IN_PLACE
    if (flg) {
#ifdef TRUNCATE_FILE
        /* if a reasonable way to truncate a file exists, use it */
        truncate_file(rfile);
#else
        /* use sentinel record rather than relying on truncation */
        t1->points = 0L; /* terminates file when read back in */
        t1->ver_major = t1->ver_minor = t1->patchlevel = 0;
        t1->uid = t1->deathdnum = t1->deathlev = 0;
        t1->maxlvl = t1->hp = t1->maxhp = t1->deaths = 0;
        t1->plrole[0] = t1->plrace[0] = t1->plgend[0] = t1->plalign[0] = '-';
        t1->plrole[1] = t1->plrace[1] = t1->plgend[1] = t1->plalign[1] = 0;
        t1->birthdate = t1->deathdate = yyyymmdd((time_t) 0L);
        Strcpy(t1->name, "@");
        Strcpy(t1->death, "<eod>\n");
        writeentry(rfile, t1);
        (void) fflush(rfile);
#endif /* TRUNCATE_FILE */
    }
#endif /* UPDATE_RECORD_IN_PLACE */
    (void) fclose(rfile);
    unlock_file(RECORD);
    free_ttlist(tt_head);

showwin:
    if (iflags.toptenwin && !done_stopprint)
        display_nhwindow(toptenwin, 1);
destroywin:
    if (!t0_used)
        dealloc_ttentry(t0);
    if (iflags.toptenwin) {
        destroy_nhwindow(toptenwin);
        toptenwin = WIN_ERR;
    }
}

STATIC_OVL void
outheader()
{
    char linebuf[BUFSZ];
    register char *bp;

    Strcpy(linebuf, " No  Points     Name");
    bp = eos(linebuf);
    while (bp < linebuf + COLNO - 9)
        *bp++ = ' ';
    Strcpy(bp, "Hp [max]");
    topten_print(linebuf);
}

/* so>0: standout line; so=0: ordinary line */
STATIC_OVL void
outentry(rank, t1, so)
struct toptenentry *t1;
int rank;
boolean so;
{
    boolean second_line = TRUE;
    char linebuf[BUFSZ];
    char *bp, hpbuf[24], linebuf3[BUFSZ];
    int hppos, lngr;

    linebuf[0] = '\0';
    if (rank)
        Sprintf(eos(linebuf), "%3d", rank);
    else
        Strcat(linebuf, "   ");

    Sprintf(eos(linebuf), " %10ld  %.16s", t1->points ? t1->points : u.urexp,
            t1->name);
    Sprintf(eos(linebuf), "-%s", t1->plrole);
    if (t1->plrace[0] != '?')
        Sprintf(eos(linebuf), "-%s", t1->plrace);
    /* Printing of gender and alignment is intentional.  It has been
     * part of the NetHack Geek Code, and illustrates a proper way to
     * specify a character from the command line.
     */
    Sprintf(eos(linebuf), "-%s", t1->plgend);
    if (t1->plalign[0] != '?')
        Sprintf(eos(linebuf), "-%s ", t1->plalign);
    else
        Strcat(linebuf, " ");
    if (!strncmp("escaped", t1->death, 7)) {
        Sprintf(eos(linebuf), "escaped the dungeon %s[max level %d]",
                !strncmp(" (", t1->death + 7, 2) ? t1->death + 7 + 2 : "",
                t1->maxlvl);
        /* fixup for closing paren in "escaped... with...Amulet)[max..." */
        if ((bp = index(linebuf, ')')) != 0)
            *bp = (t1->deathdnum == astral_level.dnum) ? '\0' : ' ';
        second_line = FALSE;
    } else if (!strncmp("ascended", t1->death, 8)) {
        Sprintf(eos(linebuf), "ascended to demigod%s-hood",
                (t1->plgend[0] == 'F') ? "dess" : "");
        second_line = FALSE;
    } else {
        if (!strncmp(t1->death, "quit", 4)) {
            Strcat(linebuf, "quit");
            second_line = FALSE;
        } else if (!strncmp(t1->death, "died of st", 10)) {
            Strcat(linebuf, "starved to death");
            second_line = FALSE;
        } else if (!strncmp(t1->death, "choked", 6)) {
            Sprintf(eos(linebuf), "choked on h%s food",
                    (t1->plgend[0] == 'F') ? "er" : "is");
        } else if (!strncmp(t1->death, "poisoned", 8)) {
            Strcat(linebuf, "was poisoned");
        } else if (!strncmp(t1->death, "crushed", 7)) {
            Strcat(linebuf, "was crushed to death");
        } else if (!strncmp(t1->death, "petrified by ", 13)) {
            Strcat(linebuf, "turned to stone");
        } else if (!strncmp(t1->death, "decapitated by ", 15)) {
            Strcat(linebuf, "decapitated");
        } else if (!strncmp(t1->death, "incinerated by ", 15)) {
            Strcat(linebuf, "incinerated");
        } else if (!strncmp(t1->death, "disintegrated by ", 17)) {
            Strcat(linebuf, "disintegrated");
        } else
            Strcat(linebuf, "died");

        if (t1->deathdnum == astral_level.dnum) {
            const char *arg, *fmt = " on the Plane of %s";

            switch (t1->deathlev) {
            case -5:
                fmt = " on the %s Plane";
                arg = "Astral";
                break;
            case -4:
                arg = "Water";
                break;
            case -3:
                arg = "Fire";
                break;
            case -2:
                arg = "Air";
                break;
            case -1:
                arg = "Earth";
                break;
            default:
                arg = "Void";
                break;
            }
            Sprintf(eos(linebuf), fmt, arg);
        } else {
            Sprintf(eos(linebuf), " in %s", dungeons[t1->deathdnum].dname);
            if (t1->deathdnum != knox_level.dnum)
                Sprintf(eos(linebuf), " on level %d", t1->deathlev);
            if (t1->deathlev != t1->maxlvl)
                Sprintf(eos(linebuf), " [max %d]", t1->maxlvl);
        }

        /* kludge for "quit while already on Charon's boat" */
        if (!strncmp(t1->death, "quit ", 5))
            Strcat(linebuf, t1->death + 4);
    }
    Strcat(linebuf, ".");

    /* Quit, starved, ascended, and escaped contain no second line */
    if (second_line)
        Sprintf(eos(linebuf), "  %c%s.", highc(*(t1->death)), t1->death + 1);

    lngr = (int) strlen(linebuf);
    if (t1->hp <= 0)
        hpbuf[0] = '-', hpbuf[1] = '\0';
    else
        Sprintf(hpbuf, "%d", t1->hp);
    /* beginning of hp column after padding (not actually padded yet) */
    hppos = COLNO - (sizeof("  Hp [max]") - 1); /* sizeof(str) includes \0 */
    while (lngr >= hppos) {
        for (bp = eos(linebuf); !(*bp == ' ' && (bp - linebuf < hppos)); bp--)
            ;
        /* special case: word is too long, wrap in the middle */
        if (linebuf + 15 >= bp)
            bp = linebuf + hppos - 1;
        /* special case: if about to wrap in the middle of maximum
           dungeon depth reached, wrap in front of it instead */
        if (bp > linebuf + 5 && !strncmp(bp - 5, " [max", 5))
            bp -= 5;
        if (*bp != ' ')
            Strcpy(linebuf3, bp);
        else
            Strcpy(linebuf3, bp + 1);
        *bp = 0;
        if (so) {
            while (bp < linebuf + (COLNO - 1))
                *bp++ = ' ';
            *bp = 0;
            topten_print_bold(linebuf);
        } else
            topten_print(linebuf);
        Sprintf(linebuf, "%15s %s", "", linebuf3);
        lngr = strlen(linebuf);
    }
    /* beginning of hp column not including padding */
    hppos = COLNO - 7 - (int) strlen(hpbuf);
    bp = eos(linebuf);

    if (bp <= linebuf + hppos) {
        /* pad any necessary blanks to the hit point entry */
        while (bp < linebuf + hppos)
            *bp++ = ' ';
        Strcpy(bp, hpbuf);
        Sprintf(eos(bp), " %s[%d]",
                (t1->maxhp < 10) ? "  " : (t1->maxhp < 100) ? " " : "",
                t1->maxhp);
    }

    if (so) {
        bp = eos(linebuf);
        if (so >= COLNO)
            so = COLNO - 1;
        while (bp < linebuf + so)
            *bp++ = ' ';
        *bp = 0;
        topten_print_bold(linebuf);
    } else
        topten_print(linebuf);
}

STATIC_OVL int
score_wanted(current_ver, rank, t1, playerct, players, uid)
boolean current_ver;
int rank;
struct toptenentry *t1;
int playerct;
const char **players;
int uid;
{
    int i;

    if (current_ver
        && (t1->ver_major != VERSION_MAJOR || t1->ver_minor != VERSION_MINOR
            || t1->patchlevel != PATCHLEVEL))
        return 0;

    if (sysopt.pers_is_uid && !playerct && t1->uid == uid)
        return 1;

    for (i = 0; i < playerct; i++) {
        if (players[i][0] == '-' && index("pr", players[i][1])
            && players[i][2] == 0 && i + 1 < playerct) {
            const char *arg = players[i + 1];
            if ((players[i][1] == 'p'
                 && str2role(arg) == str2role(t1->plrole))
                || (players[i][1] == 'r'
                    && str2race(arg) == str2race(t1->plrace)))
                return 1;
            i++;
        } else if (strcmp(players[i], "all") == 0
                   || strncmp(t1->name, players[i], NAMSZ) == 0
                   || (players[i][0] == '-' && players[i][1] == t1->plrole[0]
                       && players[i][2] == 0)
                   || (digit(players[i][0]) && rank <= atoi(players[i])))
            return 1;
    }
    return 0;
}

/*
 * print selected parts of score list.
 * argc >= 2, with argv[0] untrustworthy (directory names, et al.),
 * and argv[1] starting with "-s".
 * caveat: some shells might allow argv elements to be arbitrarily long.
 */
void
prscore(argc, argv)
int argc;
char **argv;
{
    const char **players;
    int playerct, rank;
    boolean current_ver = TRUE, init_done = FALSE;
    register struct toptenentry *t1;
    FILE *rfile;
    boolean match_found = FALSE;
    register int i;
    char pbuf[BUFSZ];
    int uid = -1;
    const char *player0;

    if (argc < 2 || strncmp(argv[1], "-s", 2)) {
        raw_printf("prscore: bad arguments (%d)", argc);
        return;
    }

    rfile = fopen_datafile(RECORD, "r", SCOREPREFIX);
    if (!rfile) {
        raw_print("Cannot open record file!");
        return;
    }

#ifdef AMIGA
    {
        extern winid amii_rawprwin;

        init_nhwindows(&argc, argv);
        amii_rawprwin = create_nhwindow(NHW_TEXT);
    }
#endif

    /* If the score list isn't after a game, we never went through
     * initialization. */
    if (wiz1_level.dlevel == 0) {
        dlb_init();
        init_dungeons();
        init_done = TRUE;
    }

    if (!argv[1][2]) { /* plain "-s" */
        argc--;
        argv++;
    } else
        argv[1] += 2;

    if (argc > 1 && !strcmp(argv[1], "-v")) {
        current_ver = FALSE;
        argc--;
        argv++;
    }

    if (argc <= 1) {
        if (sysopt.pers_is_uid) {
            uid = getuid();
            playerct = 0;
            players = (const char **) 0;
        } else {
            player0 = plname;
            if (!*player0)
#ifdef AMIGA
                player0 = "all"; /* single user system */
#else
                player0 = "hackplayer";
#endif
            playerct = 1;
            players = &player0;
        }
    } else {
        playerct = --argc;
        players = (const char **) ++argv;
    }
    raw_print("");

    t1 = tt_head = newttentry();
    for (rank = 1;; rank++) {
        readentry(rfile, t1);
        if (t1->points == 0)
            break;
        if (!match_found
            && score_wanted(current_ver, rank, t1, playerct, players, uid))
            match_found = TRUE;
        t1->tt_next = newttentry();
        t1 = t1->tt_next;
    }

    (void) fclose(rfile);
    if (init_done) {
        free_dungeons();
        dlb_cleanup();
    }

    if (match_found) {
        outheader();
        t1 = tt_head;
        for (rank = 1; t1->points != 0; rank++, t1 = t1->tt_next) {
            if (score_wanted(current_ver, rank, t1, playerct, players, uid))
                (void) outentry(rank, t1, FALSE);
        }
    } else {
        Sprintf(pbuf, "Cannot find any %sentries for ",
                current_ver ? "current " : "");
        if (playerct < 1)
            Strcat(pbuf, "you.");
        else {
            if (playerct > 1)
                Strcat(pbuf, "any of ");
            for (i = 0; i < playerct; i++) {
                /* stop printing players if there are too many to fit */
                if (strlen(pbuf) + strlen(players[i]) + 2 >= BUFSZ) {
                    if (strlen(pbuf) < BUFSZ - 4)
                        Strcat(pbuf, "...");
                    else
                        Strcpy(pbuf + strlen(pbuf) - 4, "...");
                    break;
                }
                Strcat(pbuf, players[i]);
                if (i < playerct - 1) {
                    if (players[i][0] == '-' && index("pr", players[i][1])
                        && players[i][2] == 0)
                        Strcat(pbuf, " ");
                    else
                        Strcat(pbuf, ":");
                }
            }
        }
        raw_print(pbuf);
        raw_printf("Usage: %s -s [-v] <playertypes> [maxrank] [playernames]",

                   hname);
        raw_printf("Player types are: [-p role] [-r race]");
    }
    free_ttlist(tt_head);
#ifdef AMIGA
    {
        extern winid amii_rawprwin;

        display_nhwindow(amii_rawprwin, 1);
        destroy_nhwindow(amii_rawprwin);
        amii_rawprwin = WIN_ERR;
    }
#endif
}

STATIC_OVL int
classmon(plch, fem)
char *plch;
boolean fem;
{
    int i;

    /* Look for this role in the role table */
    for (i = 0; roles[i].name.m; i++)
        if (!strncmp(plch, roles[i].filecode, ROLESZ)) {
            if (fem && roles[i].femalenum != NON_PM)
                return roles[i].femalenum;
            else if (roles[i].malenum != NON_PM)
                return roles[i].malenum;
            else
                return PM_HUMAN;
        }
    /* this might be from a 3.2.x score for former Elf class */
    if (!strcmp(plch, "E"))
        return PM_RANGER;

    impossible("What weird role is this? (%s)", plch);
    return  PM_HUMAN_MUMMY;
}

STATIC_OVL int
raceinfo(plrac, fem)
char *plrac;
boolean fem;
{
    int i;

    if (!strcmp(plrac, "?"))
        return NON_PM;

    if (!strncmp(plrac, race_demon.filecode, ROLESZ)) {
        if (fem && race_demon.femalenum != NON_PM)
            return race_demon.femalenum;
        else if (race_demon.malenum != NON_PM)
            return race_demon.malenum;
        else
            return NON_PM;
    }

    for (i = 0; races[i].noun; i++) {
        if (!strncmp(plrac, races[i].filecode, ROLESZ)) {
            if (fem && races[i].femalenum != NON_PM)
                return races[i].femalenum;
            else if (races[i].malenum != NON_PM)
                return races[i].malenum;
            else
                return NON_PM;
        }
    }

    impossible("What weird race is this? (%s)", plrac);
    return NON_PM;
}

/*
 * Get a random player name and class from the high score list,
 */
struct toptenentry *
get_rnd_toptenentry()
{
    int rank, i;
    FILE *rfile;
    register struct toptenentry *tt;
    static struct toptenentry tt_buf;

    rfile = fopen_datafile(RECORD, "r", SCOREPREFIX);
    if (!rfile) {
        raw_print("Cannot open record file!");
        return NULL;
    }

    tt = &tt_buf;
    rank = rnd(sysopt.tt_oname_maxrank);
pickentry:
    for (i = rank; i; i--) {
        readentry(rfile, tt);
        if (tt->points == 0)
            break;
    }

    if (tt->points == 0) {
        if (rank > 1) {
            rank = 1;
            rewind(rfile);
            goto pickentry;
        }
        tt = NULL;
    }

    (void) fclose(rfile);
    return tt;
}

/* does a monster on the level already has this name?  used to ensure player
 * monster names from the high score list aren't reused on Astral */
STATIC_OVL boolean
name_unused_on_lvl(name)
const char *name;
{
    int namlen = strlen(name);
    char plus_space[NAMSZ + 2];
    struct monst *mtmp;

    Strcpy(plus_space, name);
    Strcat(plus_space, " ");

    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        const char *mnam;
        if (!has_mname(mtmp))
            continue;
        mnam = MNAME(mtmp);
        /* normal mplayer will typically be named like "<name> the <title>",
         * so check for starting with "<name> " in addition to a full match */
        if (!strncmpi(plus_space, mnam, namlen + 1) || !strcmpi(name, mnam)) {
            return FALSE;
        }
    }
    return TRUE;
}

/* Return a random player name from the high score list as a string. */
char *
get_rnd_tt_name(unique)
boolean unique; /* don't accept a name that's already in use on the level */
{
    int rank, i, tries;
    FILE *rfile;
    struct toptenentry *tt;
    static struct toptenentry tt_buf;

    rfile = fopen_datafile(RECORD, "r", SCOREPREFIX);
    if (!rfile) {
        raw_print("get_rnd_tt_name: Cannot open record file!");
        return (char *) 0;
    }

    tt = &tt_buf;
    rank = rnd(sysopt.tt_oname_maxrank);
pickentry:
    for (i = rank; i; i--) {
        readentry(rfile, tt);
        if (tt->points == 0)
            break;
    }

    tries = min(20, sysopt.tt_oname_maxrank - rank);
    if (unique && tries > 0 && tt->points > 0) {
        /* continue reading entries until we hit one that's unique (or we run
         * out of tries, hit the sysopt max rank, or run out of entries) */
        while (!name_unused_on_lvl(tt->name)) {
            readentry(rfile, tt);
            if (tt->points == 0)
                break;
            if (--tries == 0)
                break;
        }
    }

    if (tries == 0 || tt->points == 0) {
        if (rank > 1) {
            rank = 1;
            rewind(rfile);
            goto pickentry;
        }
        tt = NULL;
    }

    (void) fclose(rfile);
    return tt ? tt->name : (char *) 0;
}

/*
 * Attach random player name and class from high score list
 * to an object (for statues or morgue corpses).
 */
struct obj *
tt_oname(otmp)
struct obj *otmp;
{
    struct toptenentry *tt;
    struct monst *mtmp;
    int classndx, racendx;
    if (!otmp)
        return (struct obj *) 0;

    tt = get_rnd_toptenentry();

    if (!tt)
        return (struct obj *) 0;

    /* set name */
    otmp = oname(otmp, tt->name);

    /* set monster type */
    classndx = classmon(tt->plrole, (tt->plgend[0] == 'F'));
    set_corpsenm(otmp, classndx);

    /* set race */
    racendx = raceinfo(tt->plrace, (tt->plgend[0] == 'F'));

    /* don't make corpses from races that don't
       technically leave a corpse (crowned infidels) */
    if (otmp->otyp == CORPSE && racendx >= LOW_PM && no_corpse(&mons[racendx]))
        return (struct obj *) 0;

    if (racendx > NON_PM) {
        mtmp = makemon(&mons[classndx], 0, 0, MM_NOCOUNTBIRTH);
        apply_race(mtmp, racendx);
        otmp = save_mtraits(otmp, mtmp);
        mongone(mtmp);
    }

    return otmp;
}

#ifdef NO_SCAN_BRACK
/* Lattice scanf isn't up to reading the scorefile.  What */
/* follows deals with that; I admit it's ugly. (KL) */
/* Now generally available (KL) */
STATIC_OVL void
nsb_mung_line(p)
char *p;
{
    while ((p = index(p, ' ')) != 0)
        *p = '|';
}

STATIC_OVL void
nsb_unmung_line(p)
char *p;
{
    while ((p = index(p, '|')) != 0)
        *p = ' ';
}
#endif /* NO_SCAN_BRACK */

/*topten.c*/
