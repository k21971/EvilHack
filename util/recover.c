/* NetHack 3.6	recover.c	$NHDT-Date: 1550103078 2019/02/14 00:11:18 $  $NHDT-Branch: NetHack-3.6.2-beta01 $:$NHDT-Revision: 1.19 $ */
/*	Copyright (c) Janet Walz, 1992.				  */
/* NetHack may be freely redistributed.  See license for details. */

/*
 *  Utility for reconstructing NetHack save file from a set of individual
 *  level files.  Requires that the `checkpoint' option be enabled at the
 *  time NetHack creates those level files.
 */
#ifdef WIN32
#include <errno.h>
#include "win32api.h"
#endif

#include "config.h"
#if !defined(O_WRONLY) && !defined(LSC) && !defined(AZTEC_C)
#include <fcntl.h>
#endif

#ifdef VMS
extern int FDECL(vms_creat, (const char *, unsigned));
extern int FDECL(vms_open, (const char *, int, unsigned));
#endif /* VMS */

int FDECL(restore_savefile, (char *, const char *));
static void FDECL(set_levelfile_name, (int));
static int FDECL(open_levelfile, (int, const char *));
static int FDECL(create_savefile, (const char *));
void FDECL(copy_bytes, (int, int));

#ifndef WIN_CE
#define Fprintf (void) fprintf
#else
#define Fprintf (void) nhce_message
static void nhce_message(FILE *, const char *, ...);
#endif

#define Close (void) close

#ifdef UNIX
#define SAVESIZE (PL_NSIZ + 13) /* save/99999player.e */
#else
#ifdef VMS
#define SAVESIZE (PL_NSIZ + 22) /* [.save]<uid>player.e;1 */
#else
#ifdef WIN32
#define SAVESIZE (PL_NSIZ + 40) /* username-player.NetHack-saved-game */
#else
#define SAVESIZE FILENAME /* from macconf.h or pcconf.h */
#endif
#endif
#endif

#if defined(EXEPATH)
char *FDECL(exepath, (char *));
#endif

#if defined(__BORLANDC__) && !defined(_WIN32)
extern unsigned _stklen = STKSIZ;
#endif
char savename[SAVESIZE]; /* holds relative path of save file from playground */

#ifndef NO_MAIN
int
main(argc, argv)
int argc;
char *argv[];
{
    int argno;
    const char *dir = (char *) 0;
#ifdef AMIGA
    char *startdir = (char *) 0;
#endif

    if (!dir)
        dir = getenv("NETHACKDIR");
    if (!dir)
        dir = getenv("HACKDIR");
#if defined(EXEPATH)
    if (!dir)
        dir = exepath(argv[0]);
#endif
    if (argc == 1 || (argc == 2 && !strcmp(argv[1], "-"))) {
        Fprintf(stderr, "Usage: %s [ -d directory ] base1 [ base2 ... ]\n",
                argv[0]);
#if defined(WIN32) || defined(MSDOS)
        if (dir) {
            Fprintf(
                stderr,
                "\t(Unless you override it with -d, recover will look \n");
            Fprintf(stderr, "\t in the %s directory on your system)\n", dir);
        }
#endif
        exit(EXIT_FAILURE);
    }

    argno = 1;
    if (!strncmp(argv[argno], "-d", 2)) {
        dir = argv[argno] + 2;
        if (*dir == '=' || *dir == ':')
            dir++;
        if (!*dir && argc > argno) {
            argno++;
            dir = argv[argno];
        }
        if (!*dir) {
            Fprintf(stderr,
                    "%s: flag -d must be followed by a directory name.\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
        argno++;
    }
#if defined(SECURE) && !defined(VMS)
    if (dir
#ifdef HACKDIR
        && strcmp(dir, HACKDIR)
#endif
            ) {
        if (setgid(getgid()) == -1 || setuid(getuid()) == -1) {
            perror("Failed to drop privileges");
            exit(EXIT_FAILURE);
        }
    }
#endif /* SECURE && !VMS */

#ifdef HACKDIR
    if (!dir)
        dir = HACKDIR;
#endif

#ifdef AMIGA
    startdir = getcwd(0, 255);
#endif
    if (dir && chdir((char *) dir) < 0) {
        Fprintf(stderr, "%s: cannot chdir to %s.\n", argv[0], dir);
        exit(EXIT_FAILURE);
    }

    while (argc > argno) {
        if (restore_savefile(argv[argno], dir) == 0)
            Fprintf(stderr, "recovered \"%s\" to %s\n", argv[argno],
                    savename);
        argno++;
    }
#ifdef AMIGA
    if (startdir)
        (void) chdir(startdir);
#endif
    exit(EXIT_SUCCESS);
    /*NOTREACHED*/
    return 0;
}
#endif /* !NO_MAIN */

static char lock[256];

static void
set_levelfile_name(lev)
int lev;
{
    char *tf;

    tf = rindex(lock, '.');
    if (!tf)
        tf = lock + strlen(lock);
    (void) sprintf(tf, ".%d", lev);
#ifdef VMS
    (void) strcat(tf, ";1");
#endif
}

static int
open_levelfile(lev, directory)
int lev;
const char *directory;
{
    int fd;
    char levelfile[BUFSIZ];

    set_levelfile_name(lev);
    if (directory) {
        snprintf(levelfile, BUFSIZ, "%s/%s", directory, lock);
    } else {
        strcpy(levelfile, lock);
    }
#if defined(MICRO) || defined(WIN32) || defined(MSDOS)
    fd = open(levelfile, O_RDONLY | O_BINARY);
#else
    fd = open(levelfile, O_RDONLY, 0);
#endif
    return fd;
}

static int
create_savefile(directory)
const char *directory;
{
    int fd;
    char savefile[BUFSIZ];

    if (directory) {
        sprintf(savefile, "%s/%s", directory, savename);
    } else {
        strcpy(savefile, savename);
    }
#if defined(MICRO) || defined(WIN32) || defined(MSDOS)
    fd = open(savefile, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, FCMASK);
#else
    fd = creat(savefile, FCMASK);
#endif
    return fd;
}

void
copy_bytes(ifd, ofd)
int ifd, ofd;
{
    char buf[BUFSIZ];
    int nfrom, nto;

    do {
        nfrom = read(ifd, buf, BUFSIZ);
        nto = write(ofd, buf, nfrom);
        if (nto != nfrom) {
            Fprintf(stderr, "file copy failed!\n");
            exit(EXIT_FAILURE);
        }
    } while (nfrom == BUFSIZ);
}

int
restore_savefile(basename, directory)
char *basename;
const char *directory;
{
    int gfd, lfd, sfd;
    int res = 0, lev, savelev, hpid, pltmpsiz;
    xchar levc;
    struct version_info version_data;
    struct savefile_info sfi;
    char plbuf[PL_NSIZ];

    /* level 0 file contains:
     *  pid of creating process (ignored here)
     *  level number for current level of save file
     *  name of save file nethack would have created
     *  savefile info
     *  player name
     *  and game state
     */
    (void) strcpy(lock, basename);
    gfd = open_levelfile(0, directory);
    if (gfd < 0) {
#if defined(WIN32) && !defined(WIN_CE)
        if (errno == EACCES) {
            Fprintf(
                stderr,
                "\nThere are files from a game in progress under your name.");
            Fprintf(stderr, "\nThe files are locked or inaccessible.");
            Fprintf(stderr, "\nPerhaps the other game is still running?\n");
        } else
            Fprintf(stderr, "\nTrouble accessing level 0 (errno = %d).\n",
                    errno);
#endif
        Fprintf(stderr, "Cannot open level 0 for %s.\n", basename);
        return -1;
    }
    if (read(gfd, (genericptr_t) &hpid, sizeof hpid) != sizeof hpid) {
        Fprintf(
            stderr, "%s\n%s%s%s\n",
            "Checkpoint data incompletely written or subsequently clobbered;",
            "recovery for \"", basename, "\" impossible.");
        Close(gfd);
        return -1;
    }
    if (read(gfd, (genericptr_t) &savelev, sizeof(savelev))
        != sizeof(savelev)) {
        Fprintf(stderr, "Checkpointing was not in effect for %s -- recovery "
                        "impossible.\n",
                basename);
        Close(gfd);
        return -1;
    }
    if ((read(gfd, (genericptr_t) savename, sizeof savename)
         != sizeof savename)
        || (read(gfd, (genericptr_t) &version_data, sizeof version_data)
            != sizeof version_data)
        || (read(gfd, (genericptr_t) &sfi, sizeof sfi) != sizeof sfi)
        || (read(gfd, (genericptr_t) &pltmpsiz, sizeof pltmpsiz)
            != sizeof pltmpsiz) || (pltmpsiz > PL_NSIZ)
        || (read(gfd, (genericptr_t) plbuf, pltmpsiz) != pltmpsiz)) {
        Fprintf(stderr, "Error reading %s -- can't recover.\n", lock);
        Close(gfd);
        return -1;
    }

    /* save file should contain:
     *  version info
     *  savefile info
     *  player name
     *  current level (including pets)
     *  (non-level-based) game state
     *  other levels
     */
    sfd = create_savefile(directory);
    if (sfd < 0) {
        Fprintf(stderr, "Cannot create savefile %s.\n", savename);
        Close(gfd);
        return -1;
    }

    lfd = open_levelfile(savelev, directory);
    if (lfd < 0) {
        Fprintf(stderr, "Cannot open level of save for %s.\n", basename);
        Close(gfd);
        Close(sfd);
        return -1;
    }

    if (write(sfd, (genericptr_t) &version_data, sizeof version_data)
        != sizeof version_data) {
        Fprintf(stderr, "Error writing %s; recovery failed.\n", savename);
        Close(gfd);
        Close(sfd);
        Close(lfd);
        return -1;
    }

    if (write(sfd, (genericptr_t) &sfi, sizeof sfi) != sizeof sfi) {
        Fprintf(stderr,
                "Error writing %s; recovery failed (savefile_info).\n",
                savename);
        Close(gfd);
        Close(sfd);
        Close(lfd);
        return -1;
    }

    if (write(sfd, (genericptr_t) &pltmpsiz, sizeof pltmpsiz)
        != sizeof pltmpsiz) {
        Fprintf(stderr,
                "Error writing %s; recovery failed (player name size).\n",
                savename);
        Close(gfd);
        Close(sfd);
        Close(lfd);
        return -1;
    }

    if (write(sfd, (genericptr_t) plbuf, pltmpsiz) != pltmpsiz) {
        Fprintf(stderr, "Error writing %s; recovery failed (player name).\n",
                savename);
        Close(gfd);
        Close(sfd);
        Close(lfd);
        return -1;
    }

    copy_bytes(lfd, sfd);
    Close(lfd);
    (void) unlink(lock);

    copy_bytes(gfd, sfd);
    Close(gfd);
    set_levelfile_name(0);
    (void) unlink(lock);

    for (lev = 1; lev < 256 && res == 0; lev++) {
        /* level numbers are kept in xchars in save.c, so the
         * maximum level number (for the endlevel) must be < 256
         */
        if (lev != savelev) {
            lfd = open_levelfile(lev, directory);
            if (lfd >= 0) {
                /* any or all of these may not exist */
                levc = (xchar) lev;
                if (write(sfd, (genericptr_t) &levc, sizeof levc)
                    != sizeof levc)
                    res = -1;
                else
                    copy_bytes(lfd, sfd);
                Close(lfd);
                (void) unlink(lock);
            }
        }
    }

    Close(sfd);

#if 0 /* OBSOLETE, HackWB is no longer in use */
#ifdef AMIGA
    if (res == 0) {
        /* we need to create an icon for the saved game
         * or HackWB won't notice the file.
         */
        char iconfile[FILENAME];
        int in, out;

        (void) sprintf(iconfile, "%s.info", savename);
        in = open("NetHack:default.icon", O_RDONLY);
        out = open(iconfile, O_WRONLY | O_TRUNC | O_CREAT);
        if (in > -1 && out > -1) {
            copy_bytes(in, out);
        }
        if (in > -1)
            close(in);
        if (out > -1)
            close(out);
    }
#endif /*AMIGA*/
#endif
    return res;
}

#ifdef EXEPATH
#ifdef __DJGPP__
#define PATH_SEPARATOR '/'
#else
#define PATH_SEPARATOR '\\'
#endif

#define EXEPATHBUFSZ 256
char exepathbuf[EXEPATHBUFSZ];

char *
exepath(str)
char *str;
{
    char *tmp, *tmp2;
    int bsize;

    if (!str)
        return (char *) 0;
    bsize = EXEPATHBUFSZ;
    tmp = exepathbuf;
#if !defined(WIN32)
    strcpy(tmp, str);
#else
#if defined(WIN_CE)
    {
        TCHAR wbuf[EXEPATHBUFSZ];
        GetModuleFileName((HANDLE) 0, wbuf, EXEPATHBUFSZ);
        NH_W2A(wbuf, tmp, bsize);
    }
#else
    *(tmp + GetModuleFileName((HANDLE) 0, tmp, bsize)) = '\0';
#endif
#endif
    tmp2 = strrchr(tmp, PATH_SEPARATOR);
    if (tmp2)
        *tmp2 = '\0';
    return tmp;
}
#endif /* EXEPATH */

#ifdef AMIGA
#include "date.h"
const char amiga_version_string[] = AMIGA_VERSION_STRING;
#endif

#ifdef WIN_CE
void
nhce_message(FILE *f, const char *str, ...)
{
    va_list ap;
    TCHAR wbuf[NHSTR_BUFSIZE];
    char buf[NHSTR_BUFSIZE];

    va_start(ap, str);
    vsprintf(buf, str, ap);
    va_end(ap);

    MessageBox(NULL, NH_A2W(buf, wbuf, NHSTR_BUFSIZE), TEXT("Recover"),
               MB_OK);
}
#endif

/*recover.c*/
