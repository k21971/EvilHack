/* NetHack 3.6	save.c	$NHDT-Date: 1559994625 2019/06/08 11:50:25 $  $NHDT-Branch: NetHack-3.6 $:$NHDT-Revision: 1.121 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Michael Allison, 2009. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "lev.h"

#ifndef NO_SIGNAL
#include <signal.h>
#endif
#if !defined(LSC) && !defined(O_WRONLY) && !defined(AZTEC_C)
#include <fcntl.h>
#endif

#ifdef MFLOPPY
long bytes_counted;
static int count_only;
#endif

#ifdef MICRO
int dotcnt, dotrow; /* also used in restore */
#endif

STATIC_DCL void FDECL(savelevchn, (int, int));
STATIC_DCL void FDECL(savedamage, (int, int));
STATIC_DCL void FDECL(save_bc, (int, int));
STATIC_DCL void FDECL(saveobj, (int, struct obj *));
STATIC_DCL void FDECL(saveobjchn, (int, struct obj **, int));
STATIC_DCL void FDECL(savemon, (int, struct monst *));
STATIC_DCL void FDECL(savemonchn, (int, struct monst *, int));
STATIC_DCL void FDECL(savetrapchn, (int, struct trap *, int));
STATIC_DCL void FDECL(savegamestate, (int, int));
STATIC_OVL void FDECL(save_msghistory, (int, int));
#ifdef MFLOPPY
STATIC_DCL void FDECL(savelev0, (int, XCHAR_P, int));
STATIC_DCL boolean NDECL(swapout_oldest);
STATIC_DCL void FDECL(copyfile, (char *, char *));
#endif /* MFLOPPY */
STATIC_DCL void FDECL(savelevl, (int fd, BOOLEAN_P));
STATIC_DCL void FDECL(def_bufon, (int));
STATIC_DCL void FDECL(def_bufoff, (int));
STATIC_DCL void FDECL(def_bflush, (int));
STATIC_DCL void FDECL(def_bwrite, (int, genericptr_t, unsigned int));
#ifdef ZEROCOMP
STATIC_DCL void FDECL(zerocomp_bufon, (int));
STATIC_DCL void FDECL(zerocomp_bufoff, (int));
STATIC_DCL void FDECL(zerocomp_bflush, (int));
STATIC_DCL void FDECL(zerocomp_bwrite, (int, genericptr_t, unsigned int));
STATIC_DCL void FDECL(zerocomp_bputc, (int));
#endif

static struct save_procs {
    const char *name;
    void FDECL((*save_bufon), (int));
    void FDECL((*save_bufoff), (int));
    void FDECL((*save_bflush), (int));
    void FDECL((*save_bwrite), (int, genericptr_t, unsigned int));
    void FDECL((*save_bclose), (int));
} saveprocs = {
#if !defined(ZEROCOMP) || (defined(COMPRESS) || defined(ZLIB_COMP))
    "externalcomp", def_bufon, def_bufoff, def_bflush, def_bwrite, def_bclose,
#else
    "zerocomp",      zerocomp_bufon,  zerocomp_bufoff,
    zerocomp_bflush, zerocomp_bwrite, zerocomp_bclose,
#endif
};

#if defined(UNIX) || defined(VMS) || defined(__EMX__) || defined(WIN32)
#define HUP if (!program_state.done_hup)
#else
#define HUP
#endif

/* need to preserve these during save to avoid accessing freed memory */
static unsigned ustuck_id = 0, usteed_id = 0;
static struct obj *looseball = (struct obj *) 0;  /* track uball during save and... */
static struct obj *loosechain = (struct obj *) 0; /* track uchain since saving might free it */

int
dosave()
{
    if (iflags.debug_fuzzer)
        return 0;
    clear_nhwindow(WIN_MESSAGE);
    if (yn("Really save?") == 'n') {
        clear_nhwindow(WIN_MESSAGE);
        if (multi > 0)
            nomul(0);
    } else {
        clear_nhwindow(WIN_MESSAGE);
        pline("Saving...");
#if defined(UNIX) || defined(VMS) || defined(__EMX__)
        program_state.done_hup = 0;
#endif
        if (dosave0()) {
            u.uhp = -1; /* universal game's over indicator */
            /* make sure they see the Saving message */
            display_nhwindow(WIN_MESSAGE, TRUE);
            exit_nhwindows("Well... bye.");
            nh_terminate(EXIT_SUCCESS);
        } else
            (void) doredraw();
    }
    return 0;
}

/* returns 1 if save successful */
int
dosave0()
{
    const char *fq_save;
    register int fd, ofd;
    xchar ltmp;
    d_level uz_save;
    char whynot[BUFSZ];
    int res = 0;

#ifdef WHEREIS_FILE
    delete_whereis();
#endif

    program_state.saving++; /* inhibit status and perm_invent updates */
    /* we may get here via hangup signal, in which case we want to fix up
       a few of things before saving so that they won't be restored in
       an improper state; these will be no-ops for normal save sequence */
    u.uinvulnerable = 0;
    if (iflags.save_uswallow)
        u.uswallow = 1, iflags.save_uswallow = 0;
    if (iflags.save_uinwater)
        u.uinwater = 1, iflags.save_uinwater = 0;
    if (iflags.save_uburied)
        u.uburied = 1, iflags.save_uburied = 0;
    /* extra handling for hangup save or panic save; without this,
       a thrown light source might trigger an "obj_is_local" panic;
       if a thrown or kicked object is in transit, put it on the map;
       when punished, make sure ball and chain are placed too */
    done_object_cleanup(); /* maybe force some items onto map */

    if (!program_state.something_worth_saving || !SAVEF[0])
        goto done;

    fq_save = fqname(SAVEF, SAVEPREFIX, 1); /* level files take 0 */

#if defined(UNIX) || defined(VMS)
    sethanguphandler((void FDECL((*), (int) )) SIG_IGN);
#endif
#ifndef NO_SIGNAL
    (void) signal(SIGINT, SIG_IGN);
#endif

#if defined(MICRO) && defined(MFLOPPY)
    if (!saveDiskPrompt(0))
        return 0;
#endif

    HUP if (iflags.window_inited) {
        nh_uncompress(fq_save);
        fd = open_savefile();
        if (fd > 0) {
            (void) nhclose(fd);
            clear_nhwindow(WIN_MESSAGE);
            There("seems to be an old save file.");
            if (yn("Overwrite the old file?") == 'n') {
                nh_compress(fq_save);
                goto done;
            }
        }
    }

    HUP mark_synch(); /* flush any buffered screen output */

    fd = create_savefile();
    if (fd < 0) {
        HUP pline("Cannot open save file.");
        (void) delete_savefile(); /* ab@unido */
        goto done;
    }

    vision_recalc(2); /* shut down vision to prevent problems
                         in the event of an impossible() call */

    /* undo date-dependent luck adjustments made at startup time */
    if (flags.moonphase == FULL_MOON) /* ut-sally!fletcher */
        change_luck(-1);              /* and unido!ab */
    if (flags.friday13)
        change_luck(1);
    if (iflags.window_inited)
        HUP clear_nhwindow(WIN_MESSAGE);

#ifdef MICRO
    dotcnt = 0;
    dotrow = 2;
    curs(WIN_MAP, 1, 1);
    if (!WINDOWPORT("X11"))
        putstr(WIN_MAP, 0, "Saving:");
#endif
#ifdef MFLOPPY
    /* make sure there is enough disk space */
    if (iflags.checkspace) {
        long fds, needed;

        savelev(fd, ledger_no(&u.uz), COUNT_SAVE);
        savegamestate(fd, COUNT_SAVE);
        needed = bytes_counted;

        for (ltmp = 1; ltmp <= maxledgerno(); ltmp++)
            if (ltmp != ledger_no(&u.uz) && level_info[ltmp].where)
                needed += level_info[ltmp].size + (sizeof ltmp);
        fds = freediskspace(fq_save);
        if (needed > fds) {
            HUP
            {
                There("is insufficient space on SAVE disk.");
                pline("Require %ld bytes but only have %ld.", needed, fds);
            }
            flushout();
            (void) nhclose(fd);
            (void) delete_savefile();
            return 0;
        }

        co_false();
    }
#endif /* MFLOPPY */

    store_version(fd);
    store_savefileinfo(fd);
    store_plname_in_file(fd);
    ustuck_id = (u.ustuck ? u.ustuck->m_id : 0);
    usteed_id = (u.usteed ? u.usteed->m_id : 0);
    /* savelev() might save uball and uchain, releasing their memory if
       FREEING, so we need to check their status now; if hero is swallowed,
       uball and uchain will persist beyond saving map floor and inventory
       so these copies of their pointers will be valid and savegamestate()
       will know to save them separately (from floor and invent); when not
       swallowed, uchain will be stale by then, and uball will be too if
       ball is on the floor rather than carried */
    looseball = BALL_IN_MON ? uball : 0;
    loosechain = CHAIN_IN_MON ? uchain : 0;
    savelev(fd, ledger_no(&u.uz), WRITE_SAVE | FREE_SAVE);
    savegamestate(fd, WRITE_SAVE | FREE_SAVE);

    /* While copying level files around, zero out u.uz to keep
     * parts of the restore code from completely initializing all
     * in-core data structures, since all we're doing is copying.
     * This also avoids at least one nasty core dump.
     */
    uz_save = u.uz;
    u.uz.dnum = u.uz.dlevel = 0;
    /* these pointers are no longer valid, and at least u.usteed
     * may mislead place_monster() on other levels
     */
    u.ustuck = (struct monst *) 0;
    u.usteed = (struct monst *) 0;

    for (ltmp = (xchar) 1; ltmp <= maxledgerno(); ltmp++) {
        if (ltmp == ledger_no(&uz_save))
            continue;
        if (!(level_info[ltmp].flags & LFILE_EXISTS))
            continue;
#ifdef MICRO
        curs(WIN_MAP, 1 + dotcnt++, dotrow);
        if (dotcnt >= (COLNO - 1)) {
            dotrow++;
            dotcnt = 0;
        }
        if (!WINDOWPORT("X11")) {
            putstr(WIN_MAP, 0, ".");
        }
        mark_synch();
#endif
        ofd = open_levelfile(ltmp, whynot);
        if (ofd < 0) {
            HUP pline1(whynot);
            (void) nhclose(fd);
            (void) delete_savefile();
            HUP Strcpy(killer.name, whynot);
            HUP done(TRICKED);
            goto done;
        }
        minit(); /* ZEROCOMP */
        getlev(ofd, hackpid, ltmp, FALSE);
        (void) nhclose(ofd);
        bwrite(fd, (genericptr_t) &ltmp, sizeof ltmp); /* level number*/
        savelev(fd, ltmp, WRITE_SAVE | FREE_SAVE);     /* actual level*/
        delete_levelfile(ltmp);
    }
    bclose(fd);

    u.uz = uz_save;

    /* get rid of current level --jgm */
    delete_levelfile(ledger_no(&u.uz));
    delete_levelfile(0);
    nh_compress(fq_save);
    /* this should probably come sooner... */
    program_state.something_worth_saving = 0;
    res = 1;

 done:
    program_state.saving--;
    return res;
}

STATIC_OVL void
savegamestate(fd, mode)
register int fd, mode;
{
    unsigned long uid;

    program_state.saving++; /* caller should/did already set this... */
#ifdef MFLOPPY
    count_only = (mode & COUNT_SAVE);
#endif
    uid = (unsigned long) getuid();
    bwrite(fd, (genericptr_t) &uid, sizeof uid);
    bwrite(fd, (genericptr_t) &context, sizeof context);
    bwrite(fd, (genericptr_t) &flags, sizeof flags);
#ifdef SYSFLAGS
    bwrite(fd, (genericptr_t) &sysflags, sysflags);
#endif
    urealtime.finish_time = getnow();
    urealtime.realtime += (long) (urealtime.finish_time
                                  - urealtime.start_timing);
    bwrite(fd, (genericptr_t) &u, sizeof u);
    bwrite(fd, yyyymmddhhmmss(ubirthday), 14);
    bwrite(fd, (genericptr_t) &urealtime.realtime, sizeof urealtime.realtime);
    bwrite(fd, yyyymmddhhmmss(urealtime.start_timing), 14);  /** Why? **/
    /* this is the value to use for the next update of urealtime.realtime */
    urealtime.start_timing = urealtime.finish_time;
    save_killers(fd, mode);

    /* must come before migrating_objs and migrating_mons are freed */
    save_timers(fd, mode, RANGE_GLOBAL);
    save_light_sources(fd, mode, RANGE_GLOBAL);

    /* when FREEING, deletes objects in invent and sets invent to Null;
       pointers into invent (uwep, uarmg, uamul, &c) are set to Null too */
    saveobjchn(fd, &invent, mode);

    /* it's just the one object, but saveobj doesn't do everything needed */
    saveobjchn(fd, &mchest, mode);

    /* save ball and chain if they happen to be in an unusal state */
    save_bc(fd, mode);

    saveobjchn(fd, &migrating_objs, mode); /* frees objs and sets to Null */
    savemonchn(fd, migrating_mons, mode);
    if (release_data(mode))
        migrating_mons = (struct monst *) 0;
    bwrite(fd, (genericptr_t) mvitals, sizeof mvitals);

    save_dungeon(fd, (boolean) !!perform_bwrite(mode),
                 (boolean) !!release_data(mode));
    savelevchn(fd, mode);
    bwrite(fd, (genericptr_t) &moves, sizeof moves);
    bwrite(fd, (genericptr_t) &monstermoves, sizeof monstermoves);
    bwrite(fd, (genericptr_t) &quest_status, sizeof quest_status);
    bwrite(fd, (genericptr_t) spl_book,
           sizeof(struct spell) * (MAXSPELL + 1));
    save_artifacts(fd);
    save_oracles(fd, mode);
    if (ustuck_id)
        bwrite(fd, (genericptr_t) &ustuck_id, sizeof ustuck_id);
    if (usteed_id)
        bwrite(fd, (genericptr_t) &usteed_id, sizeof usteed_id);
    bwrite(fd, (genericptr_t) pl_character, sizeof pl_character);
    bwrite(fd, (genericptr_t) pl_fruit, sizeof pl_fruit);
    savefruitchn(fd, mode);
    saveshambler(fd, mode);
    saveoracle(fd, mode);
    savecharon(fd, mode);
    savenames(fd, mode);
    save_waterlevel(fd, mode);
    save_msghistory(fd, mode);
    bflush(fd);
    program_state.saving--;
    return;
}

/* potentially called from goto_level(do.c) as well as savestateinlock() */
boolean
tricked_fileremoved(fd, whynot)
int fd;
char *whynot;
{
    if (fd < 0) {
        pline1(whynot);
        pline("Probably someone removed it.");
        Strcpy(killer.name, whynot);
        done(TRICKED);
        return TRUE;
    }
    return FALSE;
}

#ifdef INSURANCE
void
savestateinlock()
{
    int fd, hpid;
    static boolean havestate = TRUE;
    char whynot[BUFSZ];

    program_state.saving++; /* inhibit status and perm_invent updates */
    /* When checkpointing is on, the full state needs to be written
     * on each checkpoint.  When checkpointing is off, only the pid
     * needs to be in the level.0 file, so it does not need to be
     * constantly rewritten.  When checkpointing is turned off during
     * a game, however, the file has to be rewritten once to truncate
     * it and avoid restoring from outdated information.
     *
     * Restricting havestate to this routine means that an additional
     * noop pid rewriting will take place on the first "checkpoint" after
     * the game is started or restored, if checkpointing is off.
     */
    if (flags.ins_chkpt || havestate) {
        /* save the rest of the current game state in the lock file,
         * following the original int pid, the current level number,
         * and the current savefile name, which should not be subject
         * to any internal compression schemes since they must be
         * readable by an external utility
         */
        fd = open_levelfile(0, whynot);
        if (tricked_fileremoved(fd, whynot)) {
            program_state.saving--;
            return;
        }

        mread(fd, (genericptr_t) &hpid, sizeof hpid);
        if (hackpid != hpid) {
            Sprintf(whynot, "Level #0 pid (%d) doesn't match ours (%d)!",
                    hpid, hackpid);
            goto giveup;
        }
        (void) nhclose(fd);

        fd = create_levelfile(0, whynot);
        if (fd < 0) {
            pline1(whynot);
 giveup:
            Strcpy(killer.name, whynot);
            /* done(TRICKED) will return when running in wizard mode;
               clear the display-update-suppression flag before rather
               than after so that screen updating behaves normally;
               game data shouldn't be inconsistent yet, unlike it would
               become midway through saving */
            program_state.saving--;
            done(TRICKED);
            return;
        }
        bwrite(fd, (genericptr_t) &hackpid, sizeof hackpid);
        if (flags.ins_chkpt) {
            int currlev = ledger_no(&u.uz);

            bwrite(fd, (genericptr_t) &currlev, sizeof currlev);
            save_savefile_name(fd);
            store_version(fd);
            store_savefileinfo(fd);
            store_plname_in_file(fd);

            ustuck_id = (u.ustuck ? u.ustuck->m_id : 0);
            usteed_id = (u.usteed ? u.usteed->m_id : 0);
            /* if ball and/or chain aren't on floor or in invent, keep a copy
               of their pointers; not valid when on floor or in invent */
            looseball = BALL_IN_MON ? uball : 0;
            loosechain = CHAIN_IN_MON ? uchain : 0;
            savegamestate(fd, WRITE_SAVE);
        }
        bclose(fd);
    }
    program_state.saving--;
    havestate = flags.ins_chkpt;
    return;
}
#endif

#ifdef MFLOPPY
boolean
savelev(fd, lev, mode)
int fd;
xchar lev;
int mode;
{
    if (mode & COUNT_SAVE) {
        bytes_counted = 0;
        savelev0(fd, lev, COUNT_SAVE);
        /* probably bytes_counted will be filled in again by an
         * immediately following WRITE_SAVE anyway, but we'll
         * leave it out of checkspace just in case */
        if (iflags.checkspace) {
            while (bytes_counted > freediskspace(levels))
                if (!swapout_oldest())
                    return FALSE;
        }
    }
    if (mode & (WRITE_SAVE | FREE_SAVE)) {
        bytes_counted = 0;
        savelev0(fd, lev, mode);
    }
    if (mode != FREE_SAVE) {
        level_info[lev].where = ACTIVE;
        level_info[lev].time = moves;
        level_info[lev].size = bytes_counted;
    }
    return TRUE;
}

STATIC_OVL void
savelev0(fd, lev, mode)
#else
void
savelev(fd, lev, mode)
#endif
int fd;
xchar lev;
int mode;
{
#ifdef TOS
    short tlev;
#endif

    program_state.saving++; /* even if current mode is FREEING */
    /*
     *  Level file contents:
     *    version info (handled by caller);
     *    save file info (compression type; also by caller);
     *    process ID;
     *    internal level number (ledger number);
     *    bones info;
     *    actual level data.
     *
     *  If we're tearing down the current level without saving anything
     *  (which happens at end of game or upon entrance to endgame or
     *  after an aborted restore attempt) then we don't want to do any
     *  actual I/O.  So when only freeing, we skip to the bones info
     *  portion (which has some freeing to do), then jump quite a bit
     *  further ahead to the middle of the 'actual level data' portion.
     */
    if (mode != FREE_SAVE) {
        /* WRITE_SAVE (probably ORed with FREE_SAVE), or COUNT_SAVE */

        /* purge any dead monsters (necessary if we're starting
           a panic save rather than a normal one, or sometimes
           when changing levels without taking time -- e.g.
           create statue trap then immediately level teleport) */
        if (iflags.purge_monsters)
            dmonsfree();

        if (fd < 0)
            panic("Save on bad file!"); /* impossible */
#ifdef MFLOPPY
        count_only = (mode & COUNT_SAVE);
#endif
        if (lev >= 0 && lev <= maxledgerno())
            level_info[lev].flags |= VISITED;
        bwrite(fd, (genericptr_t) &hackpid, sizeof hackpid);
#ifdef TOS
        tlev = lev;
        tlev &= 0x00ff;
        bwrite(fd, (genericptr_t) &tlev, sizeof tlev);
#else
        bwrite(fd, (genericptr_t) &lev, sizeof lev);
#endif
    }

    /* bones info comes before level data; the intent is for an external
       program ('hearse') to be able to match a bones file with the
       corresponding log file entry--or perhaps just skip that?--without
       the guessing that was needed in 3.4.3 and without having to
       interpret level data to find where to start; unfortunately it
       still needs to handle all the data compression schemes */
    savecemetery(fd, mode, &level.bonesinfo);
    if (mode == FREE_SAVE) /* see above */
        goto skip_lots;

    savelevl(fd, ((sfsaveinfo.sfi1 & SFI1_RLECOMP) == SFI1_RLECOMP));
    bwrite(fd, (genericptr_t) lastseentyp, sizeof lastseentyp);
    bwrite(fd, (genericptr_t) &monstermoves, sizeof monstermoves);
    bwrite(fd, (genericptr_t) &upstair, sizeof (stairway));
    bwrite(fd, (genericptr_t) &dnstair, sizeof (stairway));
    bwrite(fd, (genericptr_t) &upladder, sizeof (stairway));
    bwrite(fd, (genericptr_t) &dnladder, sizeof (stairway));
    bwrite(fd, (genericptr_t) &sstairs, sizeof (stairway));
    bwrite(fd, (genericptr_t) &updest, sizeof (dest_area));
    bwrite(fd, (genericptr_t) &dndest, sizeof (dest_area));
    bwrite(fd, (genericptr_t) &level.flags, sizeof level.flags);
    bwrite(fd, (genericptr_t) doors, sizeof doors);
    save_rooms(fd); /* no dynamic memory to reclaim */

    /* from here on out, saving also involves allocated memory cleanup */
 skip_lots:
    /* timers and lights must be saved before monsters and objects */
    save_timers(fd, mode, RANGE_LEVEL);
    save_light_sources(fd, mode, RANGE_LEVEL);

    savemonchn(fd, fmon, mode);
    save_worm(fd, mode); /* save worm information */
    savetrapchn(fd, ftrap, mode);
    saveobjchn(fd, &fobj, mode);
    saveobjchn(fd, &level.buriedobjlist, mode);
    saveobjchn(fd, &billobjs, mode);
    if (release_data(mode)) {
        int x,y;

        for (y = 0; y < ROWNO; y++)
            for (x = 0; x < COLNO; x++)
                level.monsters[x][y] = 0;
        fmon = 0;
        ftrap = 0;
        fobj = level.buriedobjlist = billobjs = 0;
        (void) memset(rooms, 0, sizeof(rooms));
        /* level.bonesinfo = 0; -- handled by savecemetery() */
    }
    save_engravings(fd, mode);
    savedamage(fd, mode); /* pending shop wall and/or floor repair */
    save_regions(fd, mode);
    if (mode != FREE_SAVE)
        bflush(fd);
    program_state.saving--;
    return;
}

STATIC_OVL void
savelevl(fd, rlecomp)
int fd;
boolean rlecomp;
{
#ifdef RLECOMP
    struct rm *prm, *rgrm;
    int x, y;
    uchar match;

    if (rlecomp) {
        /* perform run-length encoding of rm structs */

        rgrm = &levl[0][0]; /* start matching at first rm */
        match = 0;

        for (y = 0; y < ROWNO; y++) {
            for (x = 0; x < COLNO; x++) {
                prm = &levl[x][y];
                if (prm->glyph == rgrm->glyph && prm->typ == rgrm->typ
                    && prm->seenv == rgrm->seenv
                    && prm->horizontal == rgrm->horizontal
                    && prm->flags == rgrm->flags && prm->lit == rgrm->lit
                    && prm->waslit == rgrm->waslit
                    && prm->roomno == rgrm->roomno && prm->edge == rgrm->edge
                    && prm->candig == rgrm->candig) {
                    match++;
                    if (match > 254) {
                        match = 254; /* undo this match */
                        goto writeout;
                    }
                } else {
                    /* run has been broken, write out run-length encoding */
 writeout:
                    bwrite(fd, (genericptr_t) &match, sizeof (uchar));
                    bwrite(fd, (genericptr_t) rgrm, sizeof (struct rm));
                    /* start encoding again. we have at least 1 rm
                       in the next run, viz. this one. */
                    match = 1;
                    rgrm = prm;
                }
            }
        }
        if (match > 0) {
            bwrite(fd, (genericptr_t) &match, sizeof (uchar));
            bwrite(fd, (genericptr_t) rgrm, sizeof (struct rm));
        }
        return;
    }
#else /* !RLECOMP */
    nhUse(rlecomp);
#endif /* ?RLECOMP */
    bwrite(fd, (genericptr_t) levl, sizeof levl);
    return;
}

/*ARGSUSED*/
void
bufon(fd)
int fd;
{
    (*saveprocs.save_bufon)(fd);
    return;
}

/*ARGSUSED*/
void
bufoff(fd)
int fd;
{
    (*saveprocs.save_bufoff)(fd);
    return;
}

/* flush run and buffer */
void
bflush(fd)
register int fd;
{
    (*saveprocs.save_bflush)(fd);
    return;
}

void
bwrite(fd, loc, num)
int fd;
genericptr_t loc;
register unsigned num;
{
    (*saveprocs.save_bwrite)(fd, loc, num);
    return;
}

void
bclose(fd)
int fd;
{
    (*saveprocs.save_bclose)(fd);
    return;
}

static int bw_fd = -1;
static FILE *bw_FILE = 0;
static boolean buffering = FALSE;

STATIC_OVL void
def_bufon(fd)
int fd;
{
#ifdef UNIX
    if (bw_fd != fd) {
        if (bw_fd >= 0)
            panic("double buffering unexpected");
        bw_fd = fd;
        if ((bw_FILE = fdopen(fd, "w")) == 0)
            panic("buffering of file %d failed", fd);
    }
#endif
    buffering = TRUE;
}

STATIC_OVL void
def_bufoff(fd)
int fd;
{
    def_bflush(fd);
    buffering = FALSE;
}

STATIC_OVL void
def_bflush(fd)
int fd;
{
#ifdef UNIX
    if (fd == bw_fd) {
        if (fflush(bw_FILE) == EOF)
            panic("flush of savefile failed!");
    }
#endif
    return;
}

STATIC_OVL void
def_bwrite(fd, loc, num)
register int fd;
register genericptr_t loc;
register unsigned num;
{
    boolean failed;

#ifdef MFLOPPY
    bytes_counted += num;
    if (count_only)
        return;
#endif

#ifdef UNIX
    if (buffering) {
        if (fd != bw_fd)
            panic("unbuffered write to fd %d (!= %d)", fd, bw_fd);

        failed = (fwrite(loc, (int) num, 1, bw_FILE) != 1);
    } else
#endif /* UNIX */
    {
        /* lint wants 3rd arg of write to be an int; lint -p an unsigned */
#if defined(BSD) || defined(ULTRIX) || defined(WIN32) || defined(_MSC_VER)
        failed = ((long) write(fd, loc, (int) num) != (long) num);
#else /* e.g. SYSV, __TURBOC__ */
        failed = ((long) write(fd, loc, num) != (long) num);
#endif
    }

    if (failed) {
#if defined(UNIX) || defined(VMS) || defined(__EMX__)
        if (program_state.done_hup)
            nh_terminate(EXIT_FAILURE);
        else
#endif
            panic("cannot write %u bytes to file #%d", num, fd);
    }
}

void
def_bclose(fd)
int fd;
{
    bufoff(fd);
#ifdef UNIX
    if (fd == bw_fd) {
        (void) fclose(bw_FILE);
        bw_fd = -1;
        bw_FILE = 0;
    } else
#endif
        (void) nhclose(fd);
    return;
}

#ifdef ZEROCOMP
/* The runs of zero-run compression are flushed after the game state or a
 * level is written out.  This adds a couple bytes to a save file, where
 * the runs could be mashed together, but it allows gluing together game
 * state and level files to form a save file, and it means the flushing
 * does not need to be specifically called for every other time a level
 * file is written out.
 */

#define RLESC '\0' /* Leading character for run of LRESC's */
#define flushoutrun(ln) (zerocomp_bputc(RLESC), zerocomp_bputc(ln), ln = -1)

#ifndef ZEROCOMP_BUFSIZ
#define ZEROCOMP_BUFSIZ BUFSZ
#endif
static NEARDATA unsigned char outbuf[ZEROCOMP_BUFSIZ];
static NEARDATA unsigned short outbufp = 0;
static NEARDATA short outrunlength = -1;
static NEARDATA int bwritefd;
static NEARDATA boolean compressing = FALSE;

/*dbg()
{
    HUP printf("outbufp %d outrunlength %d\n", outbufp,outrunlength);
}*/

STATIC_OVL void
zerocomp_bputc(c)
int c;
{
#ifdef MFLOPPY
    bytes_counted++;
    if (count_only)
        return;
#endif
    if (outbufp >= sizeof outbuf) {
        (void) write(bwritefd, outbuf, sizeof outbuf);
        outbufp = 0;
    }
    outbuf[outbufp++] = (unsigned char) c;
}

/*ARGSUSED*/
void STATIC_OVL
zerocomp_bufon(fd)
int fd;
{
    compressing = TRUE;
    return;
}

/*ARGSUSED*/
STATIC_OVL void
zerocomp_bufoff(fd)
int fd;
{
    if (outbufp) {
        outbufp = 0;
        panic("closing file with buffered data still unwritten");
    }
    outrunlength = -1;
    compressing = FALSE;
    return;
}

/* flush run and buffer */
STATIC_OVL void
zerocomp_bflush(fd)
register int fd;
{
    bwritefd = fd;
    if (outrunlength >= 0) { /* flush run */
        flushoutrun(outrunlength);
    }
#ifdef MFLOPPY
    if (count_only)
        outbufp = 0;
#endif

    if (outbufp) {
        if (write(fd, outbuf, outbufp) != outbufp) {
#if defined(UNIX) || defined(VMS) || defined(__EMX__)
            if (program_state.done_hup)
                nh_terminate(EXIT_FAILURE);
            else
#endif
                zerocomp_bclose(fd); /* panic (outbufp != 0) */
        }
        outbufp = 0;
    }
}

STATIC_OVL void
zerocomp_bwrite(fd, loc, num)
int fd;
genericptr_t loc;
register unsigned num;
{
    register unsigned char *bp = (unsigned char *) loc;

    if (!compressing) {
#ifdef MFLOPPY
        bytes_counted += num;
        if (count_only)
            return;
#endif
        if ((unsigned) write(fd, loc, num) != num) {
#if defined(UNIX) || defined(VMS) || defined(__EMX__)
            if (program_state.done_hup)
                nh_terminate(EXIT_FAILURE);
            else
#endif
                panic("cannot write %u bytes to file #%d", num, fd);
        }
    } else {
        bwritefd = fd;
        for (; num; num--, bp++) {
            if (*bp == RLESC) { /* One more char in run */
                if (++outrunlength == 0xFF) {
                    flushoutrun(outrunlength);
                }
            } else {                     /* end of run */
                if (outrunlength >= 0) { /* flush run */
                    flushoutrun(outrunlength);
                }
                zerocomp_bputc(*bp);
            }
        }
    }
}

void
zerocomp_bclose(fd)
int fd;
{
    zerocomp_bufoff(fd);
    (void) nhclose(fd);
    return;
}
#endif /* ZEROCOMP */

STATIC_OVL void
savelevchn(fd, mode)
register int fd, mode;
{
    s_level *tmplev, *tmplev2;
    int cnt = 0;

    for (tmplev = sp_levchn; tmplev; tmplev = tmplev->next)
        cnt++;
    if (perform_bwrite(mode))
        bwrite(fd, (genericptr_t) &cnt, sizeof cnt);

    for (tmplev = sp_levchn; tmplev; tmplev = tmplev2) {
        tmplev2 = tmplev->next;
        if (perform_bwrite(mode))
            bwrite(fd, (genericptr_t) tmplev, sizeof *tmplev);
        if (release_data(mode))
            free((genericptr_t) tmplev);
    }
    if (release_data(mode))
        sp_levchn = 0;
}

/* used when saving a level and also when saving dungeon overview data */
void
savecemetery(fd, mode, cemeteryaddr)
int fd;
int mode;
struct cemetery **cemeteryaddr;
{
    struct cemetery *thisbones, *nextbones;
    int flag;

    flag = *cemeteryaddr ? 0 : -1;
    if (perform_bwrite(mode))
        bwrite(fd, (genericptr_t) &flag, sizeof flag);
    nextbones = *cemeteryaddr;
    while ((thisbones = nextbones) != 0) {
        nextbones = thisbones->next;
        if (perform_bwrite(mode))
            bwrite(fd, (genericptr_t) thisbones, sizeof *thisbones);
        if (release_data(mode))
            free((genericptr_t) thisbones);
    }
    if (release_data(mode))
        *cemeteryaddr = 0;
}

STATIC_OVL void
savedamage(fd, mode)
register int fd, mode;
{
    struct damage *damageptr, *tmp_dam;
    unsigned int xl = 0;

    damageptr = level.damagelist;
    for (tmp_dam = damageptr; tmp_dam; tmp_dam = tmp_dam->next)
        xl++;
    if (perform_bwrite(mode))
        bwrite(fd, (genericptr_t) &xl, sizeof xl);

    while (xl--) {
        if (perform_bwrite(mode))
            bwrite(fd, (genericptr_t) damageptr, sizeof *damageptr);
        tmp_dam = damageptr;
        damageptr = damageptr->next;
        if (release_data(mode))
            free((genericptr_t) tmp_dam);
    }
    if (release_data(mode))
        level.damagelist = 0;
}

STATIC_OVL void
save_bc(fd, mode)
int fd, mode;
{
    struct obj *bc_objs = 0;

    /* save ball and chain if they are currently dangling free (i.e. not
       on floor or in inventory); 'looseball' and 'loosechain' have been
       set up in caller because ball and chain will be gone by now if on
       floor, or ball gone if carried */
    if (loosechain) {
        loosechain->nobj = bc_objs; /* uchain */
        bc_objs = loosechain;
        if (mode & FREE_SAVE) {
            setworn((struct obj *) 0, W_CHAIN); /* sets 'uchain' to Null */
            loosechain = (struct obj *) 0;
        }
    }
    if (looseball) {
        looseball->nobj = bc_objs;
        bc_objs = looseball;
        if (mode & FREE_SAVE) {
            setworn((struct obj *) 0, W_BALL); /* sets 'uball' to Null */
            looseball = (struct obj *) 0;
        }
    }
    saveobjchn(fd, &bc_objs, mode); /* frees objs in list, sets bc_objs to Null */
}

STATIC_OVL void
saveobj(fd, otmp)
int fd;
struct obj *otmp;
{
    int buflen, zerobuf = 0;

    buflen = (int) sizeof (struct obj);
    bwrite(fd, (genericptr_t) &buflen, sizeof buflen);
    bwrite(fd, (genericptr_t) otmp, buflen);
    if (otmp->oextra) {
        buflen = ONAME(otmp) ? (int) strlen(ONAME(otmp)) + 1 : 0;
        bwrite(fd, (genericptr_t) &buflen, sizeof buflen);
        if (buflen > 0)
            bwrite(fd, (genericptr_t) ONAME(otmp), buflen);

        /* defer to savemon() for this one */
        if (OMONST(otmp))
            savemon(fd, OMONST(otmp));
        else
            bwrite(fd, (genericptr_t) &zerobuf, sizeof zerobuf);

        buflen = OMID(otmp) ? (int) sizeof (unsigned) : 0;
        bwrite(fd, (genericptr_t) &buflen, sizeof buflen);
        if (buflen > 0)
            bwrite(fd, (genericptr_t) OMID(otmp), buflen);

        /* TODO: post 3.6.x, get rid of this */
        buflen = OLONG(otmp) ? (int) sizeof (long) : 0;
        bwrite(fd, (genericptr_t) &buflen, sizeof buflen);
        if (buflen > 0)
            bwrite(fd, (genericptr_t) OLONG(otmp), buflen);

        buflen = OMAILCMD(otmp) ? (int) strlen(OMAILCMD(otmp)) + 1 : 0;
        bwrite(fd, (genericptr_t) &buflen, sizeof buflen);
        if (buflen > 0)
            bwrite(fd, (genericptr_t) OMAILCMD(otmp), buflen);
    }
}

/* save an object chain; sets head of list to Null when done;
   handles release_data() for each object in the list */
STATIC_OVL void
saveobjchn(fd, obj_p, mode)
register int fd, mode;
struct obj **obj_p;
{
    struct obj *otmp = *obj_p;
    struct obj *otmp2;
    boolean is_invent = (otmp && otmp == invent);
    int minusone = -1;

    while (otmp) {
        otmp2 = otmp->nobj;
        if (perform_bwrite(mode)) {
            saveobj(fd, otmp);
        }
        if (Has_contents(otmp))
            saveobjchn(fd, &otmp->cobj, mode);
        if (release_data(mode)) {
            /*
             * If these are on the floor, the discarding could be
             * due to game save, or we could just be changing levels.
             * Always invalidate the pointer, but ensure that we have
             * the o_id in order to restore the pointer on reload.
             */
            if (otmp == context.victual.piece) {
                context.victual.o_id = otmp->o_id;
                context.victual.piece = (struct obj *) 0;
            }
            if (otmp == context.tin.tin) {
                context.tin.o_id = otmp->o_id;
                context.tin.tin = (struct obj *) 0;
            }
            if (otmp == context.spbook.book) {
                context.spbook.o_id = otmp->o_id;
                context.spbook.book = (struct obj *) 0;
            }
            otmp->where = OBJ_FREE; /* set to free so dealloc will work */
            otmp->nobj = NULL;      /* nobj saved into otmp2 */
            otmp->cobj = NULL;      /* contents handled above */
            otmp->timed = 0;        /* not timed any more */
            otmp->lamplit = 0;      /* caller handled lights */
            otmp->leashmon = 0;     /* mon->mleashed could still be set;
                                     * unfortunately, we can't clear that
                                     * until after the monster is saved */
            /* clear 'uball' and 'uchain' pointers if resetting their mask;
               could also do same for other worn items but don't need to */
            if ((otmp->owornmask & (W_BALL | W_CHAIN)) != 0)
                setworn((struct obj *) 0,
                        otmp->owornmask & (W_BALL | W_CHAIN));
            otmp->owornmask = 0L;   /* no longer care */
            dealloc_obj(otmp);
        }
        otmp = otmp2;
    }
    if (perform_bwrite(mode))
        bwrite(fd, (genericptr_t) &minusone, sizeof (int));
    if (release_data(mode)) {
        if (is_invent)
            allunworn(); /* clear uwep, uarm, uball, &c pointers */
        *obj_p = (struct obj *) 0;
    }
}

STATIC_OVL void
savemon(fd, mtmp)
int fd;
struct monst *mtmp;
{
    int buflen;

    mtmp->mtemplit = 0; /* normally clear; if set here then a panic save
                         * is being written while bhit() was executing */
    buflen = (int) sizeof (struct monst);
    bwrite(fd, (genericptr_t) &buflen, sizeof buflen);
    bwrite(fd, (genericptr_t) mtmp, buflen);
    if (mtmp->mextra) {
        buflen = MNAME(mtmp) ? (int) strlen(MNAME(mtmp)) + 1 : 0;
        bwrite(fd, (genericptr_t) &buflen, sizeof buflen);
        if (buflen > 0)
            bwrite(fd, (genericptr_t) MNAME(mtmp), buflen);
        buflen = EGD(mtmp) ? (int) sizeof (struct egd) : 0;
        bwrite(fd, (genericptr_t) &buflen, sizeof buflen);
        if (buflen > 0)
            bwrite(fd, (genericptr_t) EGD(mtmp), buflen);
        buflen = EPRI(mtmp) ? (int) sizeof (struct epri) : 0;
        bwrite(fd, (genericptr_t) &buflen, sizeof buflen);
        if (buflen > 0)
            bwrite(fd, (genericptr_t) EPRI(mtmp), buflen);
        buflen = ESHK(mtmp) ? (int) sizeof (struct eshk) : 0;
        bwrite(fd, (genericptr_t) &buflen, sizeof(int));
        if (buflen > 0)
            bwrite(fd, (genericptr_t) ESHK(mtmp), buflen);
        buflen = EMIN(mtmp) ? (int) sizeof (struct emin) : 0;
        bwrite(fd, (genericptr_t) &buflen, sizeof(int));
        if (buflen > 0)
            bwrite(fd, (genericptr_t) EMIN(mtmp), buflen);
        buflen = EDOG(mtmp) ? (int) sizeof (struct edog) : 0;
        bwrite(fd, (genericptr_t) &buflen, sizeof(int));
        if (buflen > 0)
            bwrite(fd, (genericptr_t) EDOG(mtmp), buflen);
        if (ERID(mtmp))
            buflen = sizeof(struct erid);
        else
            buflen = 0;
        bwrite(fd, (genericptr_t) &buflen, sizeof(int));
        if (buflen > 0)
            bwrite(fd, (genericptr_t) ERID(mtmp), buflen);
        buflen = ERAC(mtmp) ? (int) sizeof (struct erac) : 0;
        bwrite(fd, (genericptr_t) &buflen, sizeof(int));
        if (buflen > 0)
            bwrite(fd, (genericptr_t) ERAC(mtmp), buflen);


        /* mcorpsenm is inline int rather than pointer to something,
           so doesn't need to be preceded by a length field */
        bwrite(fd, (genericptr_t) &MCORPSENM(mtmp), sizeof MCORPSENM(mtmp));
    }
}

STATIC_OVL void
savemonchn(fd, mtmp, mode)
register int fd, mode;
struct monst *mtmp;
{
    struct monst *mtmp2;
    int minusone = -1;
    struct permonst *monbegin = &mons[0];

    if (perform_bwrite(mode))
        bwrite(fd, (genericptr_t) &monbegin, sizeof(monbegin));

    while (mtmp) {
        mtmp2 = mtmp->nmon;
        #if 0
        /* this simply eliminates the mount entirely */
        if (mtmp->mextra && ERID(mtmp) && ERID(mtmp)->mon_steed) {
            ERID(mtmp)->mon_steed = (struct monst *) 0;
        }
        #endif
        if (perform_bwrite(mode)) {
            mtmp->mnum = monsndx(mtmp->data);
            if (mtmp->ispriest)
                forget_temple_entry(mtmp); /* EPRI() */
            savemon(fd, mtmp);
        }
        if (mtmp->minvent)
            saveobjchn(fd, &mtmp->minvent, mode);
        if (release_data(mode)) {
            if (mtmp == context.polearm.hitmon) {
                context.polearm.m_id = mtmp->m_id;
                context.polearm.hitmon = NULL;
            }
            mtmp->nmon = NULL;  /* nmon saved into mtmp2 */
            dealloc_monst(mtmp);
        }
        mtmp = mtmp2;
    }
    if (perform_bwrite(mode))
        bwrite(fd, (genericptr_t) &minusone, sizeof (int));
}

/* save traps; ftrap is the only trap chain so the 2nd arg is superfluous */
STATIC_OVL void
savetrapchn(fd, trap, mode)
int fd;
struct trap *trap;
int mode;
{
    static struct trap zerotrap;
    struct trap *trap2;

    while (trap) {
        trap2 = trap->ntrap;
        if (perform_bwrite(mode))
            bwrite(fd, (genericptr_t) trap, sizeof *trap);
        if (trap->ammo)
            /* if perform_bwrite, this will save the ammo after the
               trap; if release_data, this will free the ammo before
               freeing the trap */
            saveobjchn(fd, &trap->ammo, mode);
        if (release_data(mode))
            dealloc_trap(trap);
        trap = trap2;
    }
    if (perform_bwrite(mode))
        bwrite(fd, (genericptr_t) &zerotrap, sizeof zerotrap);
}

/* save all the fruit names and ID's; this is used only in saving whole games
 * (not levels) and in saving bones levels.  When saving a bones level,
 * we only want to save the fruits which exist on the bones level; the bones
 * level routine marks nonexistent fruits by making the fid negative.
 */
void
savefruitchn(fd, mode)
int fd, mode;
{
    static struct fruit zerofruit;
    struct fruit *f2, *f1;

    f1 = ffruit;
    while (f1) {
        f2 = f1->nextf;
        if (f1->fid >= 0 && perform_bwrite(mode))
            bwrite(fd, (genericptr_t) f1, sizeof *f1);
        if (release_data(mode))
            dealloc_fruit(f1);
        f1 = f2;
    }
    if (perform_bwrite(mode))
        bwrite(fd, (genericptr_t) &zerofruit, sizeof zerofruit);
    if (release_data(mode))
        ffruit = 0;
}

void
saveshambler(fd, mode)
int fd, mode;
{
    int namesize = sizeof(mons[0].mname);
    /* Preserve our delectable abberation so the player can
       experience the same joy upon reload */
    if (perform_bwrite(mode))
        bwrite(fd, (genericptr_t) ((char *) &mons[PM_SHAMBLING_HORROR]
                                   + namesize),
               sizeof(struct permonst) - namesize);
}

void
saveoracle(fd, mode)
int fd, mode;
{
    int namesize = sizeof(mons[0].mname);
    if (perform_bwrite(mode))
        bwrite(fd, (genericptr_t) ((char *) &mons[PM_ORACLE]
                                   + namesize),
               sizeof(struct permonst) - namesize);
}

void
savecharon(fd, mode)
int fd, mode;
{
    int namesize = sizeof(mons[0].mname);
    if (perform_bwrite(mode))
        bwrite(fd, (genericptr_t) ((char *) &mons[PM_CHARON]
                                   + namesize),
               sizeof(struct permonst) - namesize);
}

void
store_plname_in_file(fd)
int fd;
{
    int plsiztmp = PL_NSIZ;

    bufoff(fd);
    /* bwrite() before bufon() uses plain write() */
    bwrite(fd, (genericptr_t) &plsiztmp, sizeof plsiztmp);
    bwrite(fd, (genericptr_t) plname, plsiztmp);
    bufon(fd);
    return;
}

STATIC_OVL void
save_msghistory(fd, mode)
int fd, mode;
{
    char *msg;
    int msgcount = 0, msglen;
    int minusone = -1;
    boolean init = TRUE;

    if (perform_bwrite(mode)) {
        /* ask window port for each message in sequence */
        while ((msg = getmsghistory(init)) != 0) {
            init = FALSE;
            msglen = strlen(msg);
            if (msglen < 1)
                continue;
            /* sanity: truncate if necessary (shouldn't happen);
               no need to modify msg[] since terminator isn't written */
            if (msglen > BUFSZ - 1)
                msglen = BUFSZ - 1;
            bwrite(fd, (genericptr_t) &msglen, sizeof msglen);
            bwrite(fd, (genericptr_t) msg, msglen);
            ++msgcount;
        }
        bwrite(fd, (genericptr_t) &minusone, sizeof (int));
    }
    debugpline1("Stored %d messages into savefile.", msgcount);
    /* note: we don't attempt to handle release_data() here */
}

void
store_savefileinfo(fd)
int fd;
{
    /* sfcap (decl.c) describes the savefile feature capabilities
     * that are supported by this port/platform build.
     *
     * sfsaveinfo (decl.c) describes the savefile info that actually
     * gets written into the savefile, and is used to determine the
     * save file being written.
     *
     * sfrestinfo (decl.c) describes the savefile info that is
     * being used to read the information from an existing savefile.
     */

    bufoff(fd);
    /* bwrite() before bufon() uses plain write() */
    bwrite(fd, (genericptr_t) &sfsaveinfo, (unsigned) sizeof sfsaveinfo);
    bufon(fd);
    return;
}

void
set_savepref(suitename)
const char *suitename;
{
    if (!strcmpi(suitename, "externalcomp")) {
        saveprocs.name = "externalcomp";
        saveprocs.save_bufon = def_bufon;
        saveprocs.save_bufoff = def_bufoff;
        saveprocs.save_bflush = def_bflush;
        saveprocs.save_bwrite = def_bwrite;
        saveprocs.save_bclose = def_bclose;
        sfsaveinfo.sfi1 |= SFI1_EXTERNALCOMP;
        sfsaveinfo.sfi1 &= ~SFI1_ZEROCOMP;
    }
    if (!strcmpi(suitename, "!rlecomp")) {
        sfsaveinfo.sfi1 &= ~SFI1_RLECOMP;
    }
#ifdef ZEROCOMP
    if (!strcmpi(suitename, "zerocomp")) {
        saveprocs.name = "zerocomp";
        saveprocs.save_bufon = zerocomp_bufon;
        saveprocs.save_bufoff = zerocomp_bufoff;
        saveprocs.save_bflush = zerocomp_bflush;
        saveprocs.save_bwrite = zerocomp_bwrite;
        saveprocs.save_bclose = zerocomp_bclose;
        sfsaveinfo.sfi1 |= SFI1_ZEROCOMP;
        sfsaveinfo.sfi1 &= ~SFI1_EXTERNALCOMP;
    }
#endif
#ifdef RLECOMP
    if (!strcmpi(suitename, "rlecomp")) {
        sfsaveinfo.sfi1 |= SFI1_RLECOMP;
    }
#endif
}

/* also called by prscore(); this probably belongs in dungeon.c... */
void
free_dungeons()
{
#ifdef FREE_ALL_MEMORY
    savelevchn(0, FREE_SAVE);
    save_dungeon(0, FALSE, TRUE);
#endif
    return;
}

/* free a lot of allocated memory which is ordinarily freed during save */
void
freedynamicdata()
{
#if defined(UNIX) && defined(MAIL)
    free_maildata();
#endif
    unload_qtlist();
    free_menu_coloring();
    free_invbuf();           /* let_to_name (invent.c) */
    free_youbuf();           /* You_buf,&c (pline.c) */
    msgtype_free();
    tmp_at(DISP_FREEMEM, 0); /* temporary display effects */
#ifdef FREE_ALL_MEMORY
#define free_current_level() savelev(-1, -1, FREE_SAVE)
#define freeobjchn(X) (saveobjchn(0, &X, FREE_SAVE), X = 0)
#define freemonchn(X) (savemonchn(0, X, FREE_SAVE), X = 0)
#define freefruitchn() savefruitchn(0, FREE_SAVE)
#define freenames() savenames(0, FREE_SAVE)
#define free_killers() save_killers(0, FREE_SAVE)
#define free_oracles() save_oracles(0, FREE_SAVE)
#define free_waterlevel() save_waterlevel(0, FREE_SAVE)
#define free_timers(R) save_timers(0, FREE_SAVE, R)
#define free_light_sources(R) save_light_sources(0, FREE_SAVE, R)
#define free_animals() mon_animal_list(FALSE)

    /* move-specific data */
    dmonsfree(); /* release dead monsters */

    /* level-specific data */
    done_object_cleanup(); /* maybe force some OBJ_FREE items onto map */
    free_current_level();

    /* game-state data [ought to reorganize savegamestate() to handle this] */
    free_killers();
    free_timers(RANGE_GLOBAL);
    free_light_sources(RANGE_GLOBAL);
    freeobjchn(invent);
    freeobjchn(mchest);
    freeobjchn(migrating_objs);
    freemonchn(migrating_mons);
    freemonchn(mydogs); /* ascension or dungeon escape */
    /* freelevchn();  --  [folded into free_dungeons()] */
    free_animals();
    free_oracles();
    freefruitchn();
    freenames();
    free_waterlevel();
    free_dungeons();

    /* some pointers in iflags */
    if (iflags.wc_font_map)
        free((genericptr_t) iflags.wc_font_map), iflags.wc_font_map = 0;
    if (iflags.wc_font_message)
        free((genericptr_t) iflags.wc_font_message),
        iflags.wc_font_message = 0;
    if (iflags.wc_font_text)
        free((genericptr_t) iflags.wc_font_text), iflags.wc_font_text = 0;
    if (iflags.wc_font_menu)
        free((genericptr_t) iflags.wc_font_menu), iflags.wc_font_menu = 0;
    if (iflags.wc_font_status)
        free((genericptr_t) iflags.wc_font_status), iflags.wc_font_status = 0;
    if (iflags.wc_tile_file)
        free((genericptr_t) iflags.wc_tile_file), iflags.wc_tile_file = 0;
    free_autopickup_exceptions();

    /* miscellaneous */
    /* free_pickinv_cache();  --  now done from really_done()... */
    free_symsets();
#endif /* FREE_ALL_MEMORY */
    if (VIA_WINDOWPORT())
        status_finish();
#if defined(DUMPLOG) || defined(DUMPHTML)
    dumplogfreemessages();
#endif

    /* last, because it frees data that might be used by panic() to provide
       feedback to the user; conceivably other freeing might trigger panic */
    sysopt_release(); /* SYSCF strings */
    return;
}

#ifdef MFLOPPY
boolean
swapin_file(lev)
int lev;
{
    char to[PATHLEN], from[PATHLEN];

    Sprintf(from, "%s%s", permbones, alllevels);
    Sprintf(to, "%s%s", levels, alllevels);
    set_levelfile_name(from, lev);
    set_levelfile_name(to, lev);
    if (iflags.checkspace) {
        while (level_info[lev].size > freediskspace(to))
            if (!swapout_oldest())
                return FALSE;
    }
    if (wizard) {
        pline("Swapping in `%s'.", from);
        wait_synch();
    }
    copyfile(from, to);
    (void) unlink(from);
    level_info[lev].where = ACTIVE;
    return TRUE;
}

STATIC_OVL boolean
swapout_oldest()
{
    char to[PATHLEN], from[PATHLEN];
    int i, oldest;
    long oldtime;

    if (!ramdisk)
        return FALSE;
    for (i = 1, oldtime = 0, oldest = 0; i <= maxledgerno(); i++)
        if (level_info[i].where == ACTIVE
            && (!oldtime || level_info[i].time < oldtime)) {
            oldest = i;
            oldtime = level_info[i].time;
        }
    if (!oldest)
        return FALSE;
    Sprintf(from, "%s%s", levels, alllevels);
    Sprintf(to, "%s%s", permbones, alllevels);
    set_levelfile_name(from, oldest);
    set_levelfile_name(to, oldest);
    if (wizard) {
        pline("Swapping out `%s'.", from);
        wait_synch();
    }
    copyfile(from, to);
    (void) unlink(from);
    level_info[oldest].where = SWAPPED;
    return TRUE;
}

STATIC_OVL void
copyfile(from, to)
char *from, *to;
{
#ifdef TOS
    if (_copyfile(from, to))
        panic("Can't copy %s to %s", from, to);
#else
    char buf[BUFSIZ]; /* this is system interaction, therefore
                       * BUFSIZ instead of NetHack's BUFSZ */
    int nfrom, nto, fdfrom, fdto;

    if ((fdfrom = open(from, O_RDONLY | O_BINARY, FCMASK)) < 0)
        panic("Can't copy from %s !?", from);
    if ((fdto = open(to, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, FCMASK)) < 0)
        panic("Can't copy to %s", to);
    do {
        nfrom = read(fdfrom, buf, BUFSIZ);
        nto = write(fdto, buf, nfrom);
        if (nto != nfrom)
            panic("Copyfile failed!");
    } while (nfrom == BUFSIZ);
    (void) nhclose(fdfrom);
    (void) nhclose(fdto);
#endif /* TOS */
}

/* see comment in bones.c */
void
co_false()
{
    count_only = FALSE;
    return;
}

#endif /* MFLOPPY */

/*save.c*/
