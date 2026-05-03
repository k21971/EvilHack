/* Copyright (c) Shawn Betts, Ryan Yeske, 2001                    */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * "Main" file for the lisp/emacs window-port.  This contains most of
 * the interface routines.  Please see doc/window.doc for an
 * description of the window interface.
 */

#ifdef MSDOS /* from compiler */
#define SHORT_FILENAMES
#endif

#include <stdint.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include "hack.h"
#include "winlisp.h"
#include "func_tab.h"

#include "dlb.h"
#ifdef SHORT_FILENAMES
#include "patchlev.h"
#else
#include "patchlevel.h"
#endif

/*
 * The following data structures come from the genl_ routines in
 * src/windows.c and as such are considered to be on the window-port
 * "side" of things, rather than the NetHack-core "side" of things.
 */

extern const char *status_fieldnm[MAXBLSTATS];
extern char *status_vals[MAXBLSTATS];

#define CMD_KEY 0
#define CMD_EXT 1
#define CMD_LISP 2 /* These are commands specific to the lisp port */

/* from tile.c */
extern short glyph2tile[];
extern int total_tiles_used;

typedef struct {
    anything identifier;
    char accelerator;
} lisp_menu_item_t;

/* An iterator for assigning accelerator keys. */
static char lisp_current_accelerator;

static lisp_menu_item_t *lisp_menu_item_list = NULL;
static int lisp_menu_list_num;
static int lisp_menu_list_max;

extern char *enc_stat[];
const char *hunger_stat[] = { "Satiated", "",        "Hungry", "Weak",
                              "Fainting", "Fainted", "Starved" };

extern char configfile[BUFSZ];

typedef struct {
    char *name;
    int type;
    int cmd; /* The command (a keystroke) */
} cmd_index_t;

#ifndef C
#define C(c) (0x1f & (c))
#endif

/* Taken from cmd.c */
cmd_index_t cmd_index[] = { { "gowest", CMD_KEY, 'h' },
                            { "gowestontop", CMD_KEY, 'H' },
                            { "gowestnear", CMD_KEY, C('h') },

                            { "gosouth", CMD_KEY, 'j' },
                            { "gosouthontop", CMD_KEY, 'J' },
                            { "gosouthnear", CMD_KEY, C('j') },

                            { "gonorth", CMD_KEY, 'k' },
                            { "gonorthontop", CMD_KEY, 'K' },
                            { "gonorthnear", CMD_KEY, C('k') },

                            { "goeast", CMD_KEY, 'l' },
                            { "goeastontop", CMD_KEY, 'L' },
                            { "goeastnear", CMD_KEY, C('l') },

                            { "gonorthwest", CMD_KEY, 'y' },
                            { "gonorthwestontop", CMD_KEY, 'Y' },
                            { "gonorthwestnear", CMD_KEY, C('y') },

                            { "gonortheast", CMD_KEY, 'u' },
                            { "gonortheastontop", CMD_KEY, 'U' },
                            { "gonortheastnear", CMD_KEY, C('u') },

                            { "gosouthwest", CMD_KEY, 'b' },
                            { "gosouthwestontop", CMD_KEY, 'B' },
                            { "gosouthwestnear", CMD_KEY, C('b') },

                            { "gosoutheast", CMD_KEY, 'n' },
                            { "gosoutheastontop", CMD_KEY, 'N' },
                            { "gosoutheastnear", CMD_KEY, C('n') },

                            { "travel", CMD_KEY, '_' },

                            { "idtrap", CMD_KEY, '^' },
                            { "apply", CMD_KEY, 'a' },
                            { "remarm", CMD_KEY, 'A' },
                            { "close", CMD_KEY, 'c' },
                            { "drop", CMD_KEY, 'd' },

                            { "ddrop", CMD_KEY, 'D' },
                            { "eat", CMD_KEY, 'e' },
                            { "engrave", CMD_KEY, 'E' },
                            { "fire", CMD_KEY, 'f' },
                            { "inv", CMD_KEY, 'i' },

                            { "typeinv", CMD_KEY, 'I' },
                            { "open", CMD_KEY, 'o' },
                            { "set", CMD_KEY, 'O' },
                            { "pay", CMD_KEY, 'p' },
                            { "puton", CMD_KEY, 'P' },

                            { "drink", CMD_KEY, 'q' },
                            { "wieldquiver", CMD_KEY, 'Q' },
                            { "read", CMD_KEY, 'r' },
                            { "remring", CMD_KEY, 'R' },
                            { "search", CMD_KEY, 's' },

                            { "save", CMD_KEY, 'S' },
                            { "throw", CMD_KEY, 't' },
                            { "takeoff", CMD_KEY, 'T' },
                            { "simpleversion", CMD_KEY, 'v' },
                            { "history", CMD_KEY, 'V' },

                            { "wield", CMD_KEY, 'w' },
                            { "wear", CMD_KEY, 'W' },
                            { "swapweapon", CMD_KEY, 'x' },
                            { "enter_explore_mode", CMD_KEY, 'X' },
                            { "zap", CMD_KEY, 'z' },

                            { "cast", CMD_KEY, 'Z' },
                            { "up", CMD_KEY, '<' },
                            { "down", CMD_KEY, '>' },
                            { "whatis", CMD_KEY, '/' },
                            { "help", CMD_KEY, '?' },

                            { "whatdoes", CMD_KEY, '&' },
                            { "sh", CMD_KEY, '!' },
                            { "discovered", CMD_KEY, '\\' },
                            { "null", CMD_KEY, '.' },
                            { "look", CMD_KEY, ':' },

                            { "quickwhatis", CMD_KEY, ';' },
                            { "pickup", CMD_KEY, ',' },
                            { "togglepickup", CMD_KEY, '@' },
                            { "prinuse", CMD_KEY, '*' },
                            { "countgold", CMD_KEY, '$' },
                            { "getpos_menu", CMD_KEY, '!' },

                            { "kick", CMD_KEY, C('d') },
                            { "listspells", CMD_KEY, '+' },
                            { "redraw", CMD_KEY, C('r') },
                            { "teleport", CMD_KEY, C('t') },
                            { "callmon", CMD_KEY, 'C' },
                            { "fight", CMD_KEY, 'F' },
                            { "movenear", CMD_KEY, 'g' },
                            { "move", CMD_KEY, 'G' },
                            { "movenopickuporfight", CMD_KEY, 'm' },
                            { "movenopickup", CMD_KEY, 'M' },
                            { "showweapon", CMD_KEY, ')' },
                            { "showarmor", CMD_KEY, '[' },
                            { "showrings", CMD_KEY, '=' },
                            { "showamulet", CMD_KEY, '"' },
                            { "showtool", CMD_KEY, '(' },
                            { "attributes", CMD_KEY, C('x') },
#ifdef REDO
                            { "again", CMD_KEY, DOAGAIN },
#endif /* REDO */

                            /* wizard commands */
                            { "wiz_detect", CMD_KEY, C('e') },
                            { "wiz_map", CMD_KEY, C('f') },
                            { "wiz_genesis", CMD_KEY, C('g') },
                            { "wiz_identify", CMD_KEY, C('i') },
                            { "wiz_where", CMD_KEY, C('o') },
                            { "wiz_level_tele", CMD_KEY, C('v') },
                            { "wiz_wish", CMD_KEY, C('w') },

/* wizard extended commands */
#ifdef WIZARD
                            { "light sources", CMD_EXT, 0 },
                            { "seenv", CMD_EXT, 0 },
                            { "stats", CMD_EXT, 0 },
                            { "timeout", CMD_EXT, 0 },
                            { "vision", CMD_EXT, 0 },
#ifdef DEBUG
                            { "wizdebug", CMD_EXT, 0 },
#endif /* DEBUG */
                            { "wmode", CMD_EXT, 0 },
#endif /* WIZARD */
                            { "#", CMD_KEY, '#' },

                            { "pray", CMD_EXT, 0 },
                            { "adjust", CMD_EXT, 0 },
                            { "chat", CMD_EXT, 0 },
                            { "conduct", CMD_EXT, 0 },
                            { "dip", CMD_EXT, 0 },

                            { "enhance", CMD_EXT, 0 },
                            { "force", CMD_EXT, 0 },
                            { "invoke", CMD_EXT, 0 },
                            { "jump", CMD_EXT, 0 },
                            { "loot", CMD_EXT, 0 },

                            { "monster", CMD_EXT, 0 },
                            { "name", CMD_EXT, 0 },
                            { "offer", CMD_EXT, 0 },
                            { "quit", CMD_EXT, 0 },
                            { "ride", CMD_EXT, 0 },

                            { "rub", CMD_EXT, 0 },
                            { "sit", CMD_EXT, 0 },
                            { "turn", CMD_EXT, 0 },
                            { "twoweapon", CMD_EXT, 0 },
                            { "untrap", CMD_EXT, 0 },

                            { "version", CMD_EXT, 0 },
                            { "wipe", CMD_EXT, 0 },

                            /* Lisp port specific commands  */
                            { "options", CMD_LISP, 0 },

                            { 0, CMD_KEY, '\0' } };

/* This variable is set when the user has selected an extended command. */
static int extended_cmd_id = 0;

/* Interface definition, for windows.c */
struct window_procs lisp_procs = {
    "lisp",
    (WC_ALIGN_MESSAGE | WC_ALIGN_STATUS | WC_COLOR | WC_INVERSE
     | WC_HILITE_PET | WC_PERM_INVENT | WC_POPUP_DIALOG | WC_SPLASH_SCREEN),
    (WC2_DARKGRAY | WC2_HITPOINTBAR
#if defined(STATUS_HILITES)
     | WC2_HILITE_STATUS
#endif
     | WC2_FLUSH_STATUS | WC2_TERM_SIZE | WC2_STATUSLINES | WC2_WINDOWBORDERS
     | WC2_PETATTR | WC2_GUICOLOR | WC2_SUPPRESS_HIST),
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1 }, /* color availability */
    lisp_init_nhwindows,
    lisp_player_selection,
    lisp_askname,
    lisp_get_nh_event,
    lisp_exit_nhwindows,
    lisp_suspend_nhwindows,
    lisp_resume_nhwindows,
    lisp_create_nhwindow,
    lisp_clear_nhwindow,
    lisp_display_nhwindow,
    lisp_destroy_nhwindow,
    lisp_curs,
    lisp_putstr,
    genl_putmixed,
    lisp_display_file,
    lisp_start_menu,
    lisp_add_menu,
    lisp_end_menu,
    lisp_select_menu,
    genl_message_menu,
    lisp_update_inventory,
    lisp_mark_synch,
    lisp_wait_synch,
#ifdef CLIPPING
    lisp_cliparound,
#endif
#ifdef POSITIONBAR
    dummy_update_position_bar,
#endif
    lisp_print_glyph,
    lisp_raw_print,
    lisp_raw_print_bold,
    lisp_nhgetch,
    lisp_nh_poskey,
    lisp_nhbell,
    lisp_doprev_message,
    lisp_yn_function,
    lisp_getlin,
    lisp_get_ext_cmd,
    lisp_number_pad,
    lisp_delay_output,
#ifdef CHANGE_COLOR
    dummy_change_color,
#ifdef MAC /* old OS 9, not OSX */
    (void (*)(int)) 0,
    (short (*)(winid, char *)) 0,
#endif
    dummy_get_color_string,
#endif
    lisp_start_screen,
    lisp_end_screen,
    genl_outrip,
    genl_preference_update,
    genl_getmsghistory,
    genl_putmsghistory,
    genl_status_init,
    genl_status_finish,
    genl_status_enablefield,
    lisp_status_update,
    genl_can_suspend_no,
};

/* macros for printing lisp output */
#define lisp_cmd(s, body)                \
    do {                                 \
        printf("(nethack-nhapi-%s ", s); \
        body;                            \
        printf(")\n");                   \
    } while (0)
#define lisp_list(body) \
    do {                \
        printf("(");    \
        body;           \
        printf(") ");   \
    } while (0)

#define lisp_open printf("(")
#define lisp_close printf(") ")
#define lisp_quote printf("'")
#define lisp_dot printf(". ")
#define lisp_t printf("t ")
#define lisp_nil printf("nil ")
#define lisp_literal(x)   \
    do {                  \
        lisp_quote;       \
        printf("%s ", x); \
    } while (0)
#define lisp_cons(x, y) \
    do {                \
        lisp_open;      \
        x;              \
        lisp_dot;       \
        y;              \
        lisp_close;     \
    } while (0)
#define lisp_int(i) printf("%d ", i)
#define lisp_coord(c) printf("'(%d,%d) ", c.x, c.y)
#define lisp_boolean(i) printf("%s ", i ? "t" : "nil")
#define lisp_string(s)                              \
    do {                                            \
        int nhi;                                    \
        printf("\"");                               \
        if (s)                                      \
            for (nhi = 0; nhi < strlen(s); nhi++) { \
                if (s[nhi] == 34 || s[nhi] == 92)   \
                    putchar('\\');                  \
                putchar(s[nhi]);                    \
            }                                       \
        printf("\" ");                              \
    } while (0)

struct timeval start;
static void
print_timestamp()
{
    struct timeval now;
    gettimeofday(&now, NULL);

    now.tv_sec -= start.tv_sec;
    now.tv_usec -= start.tv_usec;
    if (now.tv_usec < 0) {
        now.tv_sec--;
        now.tv_usec += 1000000;
    }

    printf("; (%u %u %u 0)\n", (uint32_t) (now.tv_sec >> 16),
           (uint32_t) (now.tv_sec & 0xFFFF), (uint32_t) now.tv_usec);
}

static const char *
attr_to_string(attr)
int attr;
{
    /* Just like curses, we strip off control flags masked onto the display
       attributes (caller should have already done this...) */
    attr &= ~(ATR_URGENT | ATR_NOHISTORY);

    switch (attr) {
    case ATR_NONE:
        return "nethack-atr-none-face";
    case ATR_ULINE:
        return "nethack-atr-uline-face";
    case ATR_BOLD:
        return "nethack-atr-bold-face";
    case ATR_DIM:
        return "nethack-atr-dim-face";
    case ATR_BLINK:
        return "nethack-atr-blink-face";
    case ATR_INVERSE:
        return "nethack-atr-inverse-face";
    default:
        return "nethack-atr-none-face";
    }
}

static const char *
wintype_to_string(type)
int type;
{
    switch (type) {
    case NHW_MAP:
        return "nhw-map";
    case NHW_MESSAGE:
        return "nhw-message";
    case NHW_STATUS:
        return "nhw-status";
    case NHW_MENU:
        return "nhw-menu";
    case NHW_TEXT:
        return "nhw-text";
    default:
        fprintf(stderr, "Invalid window code\n");
        exit(EXIT_FAILURE);
        break;
    }
}

static const char *
how_to_string(how)
int how;
{
    switch (how) {
    case PICK_NONE:
        return "pick-none";
    case PICK_ONE:
        return "pick-one";
    case PICK_ANY:
        return "pick-any";
    default:
        impossible("Invalid how value %d", how);
    }
}

static int read_int(prompt, i) const char *prompt;
int *i;
{
    char line[BUFSZ];
    int rv;
    print_timestamp();
    printf("%s> ", prompt);
    fflush(stdout);
    (void)fgets(line, BUFSZ, stdin);
    printf("\n");
    rv = sscanf(line, "%d", i);
    if (rv != 1)
        *i = -1;
    return rv;
}

static int read_string(prompt, str) const char *prompt;
char **str;
{
    char *rv;
    int len;
    int size;
    char tmp[BUFSZ];

    len = 0;
    size = BUFSZ * 2;
    *str = malloc(size);
    (*str)[0] = '\0';

    print_timestamp();
    printf("%s> ", prompt);
    fflush(stdout);
    do {
        /* Read the string */
        rv = fgets(tmp, BUFSZ, stdin);
        printf("\n");
        if (rv == NULL)
            break;

        len += strlen(tmp);
        if (len >= size - 1) {
            size *= 2;
            *str = realloc(*str, size);
            if (*str == NULL)
                panic("Memory allocation failure; cannot get %u bytes", size);
        }
        strcat(*str, tmp);
    } while (tmp[strlen(tmp) - 1] != '\n');

    /* Did we read a string or error out? */
    if (rv == NULL) {
        free(*str);
        return -1;
    } else {
        /* chop the newline */
        (*str)[strlen(*str) - 1] = '\0';
        return 0;
    }
}

static int read_command(prompt, cmd, count) const char *prompt;
char *cmd;
char *count;
{
    char *buf;
    int rv;
    cmd[0] = '\0';
    *count = 0;
    if (read_string(prompt, &buf) == -1)
        return -1;
    rv = sscanf(buf, "%255s %1023s", cmd, count);
    free(buf);
    if (rv != 2)
        *count = 0;
    return rv;
}

void bail(mesg) const char *mesg;
{
    clearlocks();
    lisp_exit_nhwindows(mesg);
    nh_terminate(EXIT_SUCCESS);
    /*NOTREACHED*/
}

void win_lisp_init(dir) int dir;
{
    /* Code to be executed on startup. */
    return;
}

void
lisp_player_selection()
{
    int i, k, n;
    char pick4u = 'n', thisch, lastch = 0;
    char pbuf[QBUFSZ], plbuf[QBUFSZ];
    winid win;
    anything any;
    menu_item *selected = 0;

    /* prevent an unnecessary prompt */
    rigid_role_checks();

    /* Should we randomly pick for the player? */
    if (!flags.randomall
        && (flags.initrole == ROLE_NONE || flags.initrace == ROLE_NONE
            || flags.initgend == ROLE_NONE || flags.initalign == ROLE_NONE)) {
        pick4u = lisp_yn_function("Shall I pick a character for you? [ynq] ",
                                  "ynq", 'y');

        if (pick4u != 'y' && pick4u != 'n') {
        give_up: /* Quit */
            if (selected)
                free((genericptr_t) selected);
            bail((char *) 0);
            /*NOTREACHED*/
            return;
        }
    }

    (void) root_plselection_prompt(plbuf, QBUFSZ - 1, flags.initrole,
                                   flags.initrace, flags.initgend,
                                   flags.initalign);

    /* Select a role, if necessary */
    /* we'll try to be compatible with pre-selected race/gender/alignment,
     * but may not succeed */
    if (flags.initrole < 0) {
        char rolenamebuf[QBUFSZ];
        /* Process the choice */
        if (pick4u == 'y' || flags.initrole == ROLE_RANDOM
            || flags.randomall) {
            /* Pick a random role */
            flags.initrole = pick_role(flags.initrace, flags.initgend,
                                       flags.initalign, PICK_RANDOM);
            if (flags.initrole < 0) {
                /*          lisp_putstr(BASE_WINDOW, 0, "Incompatible role!");
                 */
                flags.initrole = randrole(FALSE);
            }
        } else {
            /* Prompt for a role */
            win = create_nhwindow(NHW_MENU);
            start_menu(win);
            any.a_void = 0; /* zero out all bits */
            for (i = 0; roles[i].name.m; i++) {
                if (ok_role(i, flags.initrace, flags.initgend,
                            flags.initalign)) {
                    any.a_int = i + 1; /* must be non-zero */
                    thisch = lowc(roles[i].name.m[0]);
                    if (thisch == lastch)
                        thisch = highc(thisch);
                    if (flags.initgend != ROLE_NONE
                        && flags.initgend != ROLE_RANDOM) {
                        if (flags.initgend == 1 && roles[i].name.f)
                            Strcpy(rolenamebuf, roles[i].name.f);
                        else
                            Strcpy(rolenamebuf, roles[i].name.m);
                    } else {
                        if (roles[i].name.f) {
                            Strcpy(rolenamebuf, roles[i].name.m);
                            Strcat(rolenamebuf, "/");
                            Strcat(rolenamebuf, roles[i].name.f);
                        } else
                            Strcpy(rolenamebuf, roles[i].name.m);
                    }
                    add_menu(win, NO_GLYPH, &any, thisch, 0, ATR_NONE,
                             an(rolenamebuf), MENU_UNSELECTED);
                    lastch = thisch;
                }
            }
            any.a_int = pick_role(flags.initrace, flags.initgend,
                                  flags.initalign, PICK_RANDOM)
                        + 1;
            if (any.a_int == 0) /* must be non-zero */
                any.a_int = randrole(FALSE) + 1;
            add_menu(win, NO_GLYPH, &any, '*', 0, ATR_NONE, "Random",
                     MENU_UNSELECTED);
            any.a_int = i + 1; /* must be non-zero */
            add_menu(win, NO_GLYPH, &any, 'q', 0, ATR_NONE, "Quit",
                     MENU_UNSELECTED);
            Sprintf(pbuf, "Pick a role for your %.107s", plbuf);
            end_menu(win, pbuf);
            n = select_menu(win, PICK_ONE, &selected);
            destroy_nhwindow(win);

            /* Process the choice */
            if (n != 1 || selected[0].item.a_int == any.a_int)
                goto give_up; /* Selected quit */

            flags.initrole = selected[0].item.a_int - 1;
            free((genericptr_t) selected), selected = 0;
        }
        (void) root_plselection_prompt(plbuf, QBUFSZ - 1, flags.initrole,
                                       flags.initrace, flags.initgend,
                                       flags.initalign);
    }

    /* Select a race, if necessary */
    /* force compatibility with role, try for compatibility with
     * pre-selected gender/alignment */
    if (flags.initrace < 0 || !validrace(flags.initrole, flags.initrace)) {
        /* pre-selected race not valid */
        if (pick4u == 'y' || flags.initrace == ROLE_RANDOM
            || flags.randomall) {
            flags.initrace = pick_race(flags.initrole, flags.initgend,
                                       flags.initalign, PICK_RANDOM);
            if (flags.initrace < 0) {
                /*          lisp_putstr(BASE_WINDOW, 0, "Incompatible race!");
                 */
                flags.initrace = randrace(flags.initrole);
            }
        } else { /* pick4u == 'n' */
            /* Count the number of valid races */
            n = 0; /* number valid */
            k = 0; /* valid race */
            for (i = 0; races[i].noun; i++) {
                if (ok_race(flags.initrole, i, flags.initgend,
                            flags.initalign)) {
                    n++;
                    k = i;
                }
            }
            if (n == 0) {
                for (i = 0; races[i].noun; i++) {
                    if (validrace(flags.initrole, i)) {
                        n++;
                        k = i;
                    }
                }
            }

            /* Permit the user to pick, if there is more than one */
            if (n > 1) {
                win = create_nhwindow(NHW_MENU);
                start_menu(win);
                any.a_void = 0; /* zero out all bits */
                for (i = 0; races[i].noun; i++)
                    if (ok_race(flags.initrole, i, flags.initgend,
                                flags.initalign)) {
                        any.a_int = i + 1; /* must be non-zero */
                        add_menu(win, NO_GLYPH, &any, races[i].noun[0], 0,
                                 ATR_NONE, races[i].noun, MENU_UNSELECTED);
                    }
                any.a_int = pick_race(flags.initrole, flags.initgend,
                                      flags.initalign, PICK_RANDOM)
                            + 1;
                if (any.a_int == 0) /* must be non-zero */
                    any.a_int = randrace(flags.initrole) + 1;
                add_menu(win, NO_GLYPH, &any, '*', 0, ATR_NONE, "Random",
                         MENU_UNSELECTED);
                any.a_int = i + 1; /* must be non-zero */
                add_menu(win, NO_GLYPH, &any, 'q', 0, ATR_NONE, "Quit",
                         MENU_UNSELECTED);
                Sprintf(pbuf, "Pick the race of your %.106s", plbuf);
                end_menu(win, pbuf);
                n = select_menu(win, PICK_ONE, &selected);
                destroy_nhwindow(win);
                if (n != 1 || selected[0].item.a_int == any.a_int)
                    goto give_up; /* Selected quit */

                k = selected[0].item.a_int - 1;
                free((genericptr_t) selected), selected = 0;
            }
            flags.initrace = k;
        }
        (void) root_plselection_prompt(plbuf, QBUFSZ - 1, flags.initrole,
                                       flags.initrace, flags.initgend,
                                       flags.initalign);
    }

    /* Select a gender, if necessary */
    /* force compatibility with role/race, try for compatibility with
     * pre-selected alignment */
    if (flags.initgend < 0
        || !validgend(flags.initrole, flags.initrace, flags.initgend)) {
        /* pre-selected gender not valid */
        if (pick4u == 'y' || flags.initgend == ROLE_RANDOM
            || flags.randomall) {
            flags.initgend = pick_gend(flags.initrole, flags.initrace,
                                       flags.initalign, PICK_RANDOM);
            if (flags.initgend < 0) {
                /*          lisp_putstr(BASE_WINDOW, 0, "Incompatible
                 * gender!"); */
                flags.initgend = randgend(flags.initrole, flags.initrace);
            }
        } else { /* pick4u == 'n' */
            /* Count the number of valid genders */
            n = 0; /* number valid */
            k = 0; /* valid gender */
            for (i = 0; i < ROLE_GENDERS; i++) {
                if (ok_gend(flags.initrole, flags.initrace, i,
                            flags.initalign)) {
                    n++;
                    k = i;
                }
            }
            if (n == 0) {
                for (i = 0; i < ROLE_GENDERS; i++) {
                    if (validgend(flags.initrole, flags.initrace, i)) {
                        n++;
                        k = i;
                    }
                }
            }

            /* Permit the user to pick, if there is more than one */
            if (n > 1) {
                win = create_nhwindow(NHW_MENU);
                start_menu(win);
                any.a_void = 0; /* zero out all bits */
                for (i = 0; i < ROLE_GENDERS; i++)
                    if (ok_gend(flags.initrole, flags.initrace, i,
                                flags.initalign)) {
                        any.a_int = i + 1;
                        add_menu(win, NO_GLYPH, &any, genders[i].adj[0], 0,
                                 ATR_NONE, genders[i].adj, MENU_UNSELECTED);
                    }
                any.a_int = pick_gend(flags.initrole, flags.initrace,
                                      flags.initalign, PICK_RANDOM)
                            + 1;
                if (any.a_int == 0) /* must be non-zero */
                    any.a_int = randgend(flags.initrole, flags.initrace) + 1;
                add_menu(win, NO_GLYPH, &any, '*', 0, ATR_NONE, "Random",
                         MENU_UNSELECTED);
                any.a_int = i + 1; /* must be non-zero */
                add_menu(win, NO_GLYPH, &any, 'q', 0, ATR_NONE, "Quit",
                         MENU_UNSELECTED);
                Sprintf(pbuf, "Pick the gender of your %.104s", plbuf);
                end_menu(win, pbuf);
                n = select_menu(win, PICK_ONE, &selected);
                destroy_nhwindow(win);
                if (n != 1 || selected[0].item.a_int == any.a_int)
                    goto give_up; /* Selected quit */

                k = selected[0].item.a_int - 1;
                free((genericptr_t) selected), selected = 0;
            }
            flags.initgend = k;
        }
        (void) root_plselection_prompt(plbuf, QBUFSZ - 1, flags.initrole,
                                       flags.initrace, flags.initgend,
                                       flags.initalign);
    }

    /* Select an alignment, if necessary */
    /* force compatibility with role/race/gender */
    if (flags.initalign < 0
        || !validalign(flags.initrole, flags.initrace, flags.initalign)) {
        /* pre-selected alignment not valid */
        if (pick4u == 'y' || flags.initalign == ROLE_RANDOM
            || flags.randomall) {
            flags.initalign = pick_align(flags.initrole, flags.initrace,
                                         flags.initgend, PICK_RANDOM);
            if (flags.initalign < 0) {
                /*          lisp_putstr(BASE_WINDOW, 0, "Incompatible
                 * alignment!"); */
                flags.initalign = randalign(flags.initrole, flags.initrace);
            }
        } else { /* pick4u == 'n' */
            /* Count the number of valid alignments */
            n = 0; /* number valid */
            k = 0; /* valid alignment */
            for (i = 0; i < ROLE_ALIGNS; i++) {
                if (ok_align(flags.initrole, flags.initrace, flags.initgend,
                             i)) {
                    n++;
                    k = i;
                }
            }
            if (n == 0) {
                for (i = 0; i < ROLE_ALIGNS; i++) {
                    if (validalign(flags.initrole, flags.initrace, i)) {
                        n++;
                        k = i;
                    }
                }
            }

            /* Permit the user to pick, if there is more than one */
            if (n > 1) {
                win = create_nhwindow(NHW_MENU);
                start_menu(win);
                any.a_void = 0; /* zero out all bits */
                for (i = 0; i < ROLE_ALIGNS; i++)
                    if (ok_align(flags.initrole, flags.initrace,
                                 flags.initgend, i)) {
                        any.a_int = i + 1;
                        add_menu(win, NO_GLYPH, &any, aligns[i].adj[0], 0,
                                 ATR_NONE, aligns[i].adj, MENU_UNSELECTED);
                    }
                any.a_int = pick_align(flags.initrole, flags.initrace,
                                       flags.initgend, PICK_RANDOM)
                            + 1;
                if (any.a_int == 0) /* must be non-zero */
                    any.a_int = randalign(flags.initrole, flags.initrace) + 1;
                add_menu(win, NO_GLYPH, &any, '*', 0, ATR_NONE, "Random",
                         MENU_UNSELECTED);
                any.a_int = i + 1; /* must be non-zero */
                add_menu(win, NO_GLYPH, &any, 'q', 0, ATR_NONE, "Quit",
                         MENU_UNSELECTED);
                Sprintf(pbuf, "Pick the alignment of your %.101s", plbuf);
                end_menu(win, pbuf);
                n = select_menu(win, PICK_ONE, &selected);
                destroy_nhwindow(win);
                if (n != 1 || selected[0].item.a_int == any.a_int)
                    goto give_up; /* Selected quit */

                k = selected[0].item.a_int - 1;
                free((genericptr_t) selected), selected = 0;
            }
            flags.initalign = k;
        }
    }
    /* Success! */
}

/* Reads from standard in, the player's name. */
void
lisp_askname()
{
    char *line;
    lisp_cmd("askname", );
    for (int i = 0; i < 10; i++)
        if (read_string("string", &line) != -1)
            goto done;
    bail("Giving up after 10 tries.\n");
done:
    strncpy(plname, line, PL_NSIZ);
    plname[PL_NSIZ - 1] = '\0';
    free(line);
}

/* This is a noop for tty and X, so should it be a noop for us too? */
void
lisp_get_nh_event()
{
    return;
}

/* Global Functions */
void lisp_raw_print(str) const char *str;
{
    lisp_cmd("raw-print", lisp_string(str));
}

void lisp_raw_print_bold(str) const char *str;
{
    lisp_cmd("raw-print-bold", lisp_string(str));
}

void lisp_curs(window, x, y) winid window;
int x, y;
{
    if (window == WIN_MAP)
        lisp_cmd("curs", lisp_int(x); lisp_int(y));
    else if (window == WIN_STATUS) {
        /* do nothing */
    } else
        lisp_cmd("error", lisp_string("lisp_curs bad window");
                 lisp_int(window));
}

/* Send the options to the lisp process */
static void
get_options()
{
    lisp_cmd(
        "options",
        lisp_boolean(iflags.cbreak); /* in cbreak mode, rogue format */
        /* lisp_boolean(iflags.DECgraphics);    /* use DEC VT-xxx extended
           character set */
        lisp_boolean(iflags.echo); /* 1 to echo characters */
        /* lisp_boolean(iflags.IBMgraphics);    /* use IBM extended character
           set */
        lisp_int(iflags.msg_history); /* hint: # of top lines to save */
        lisp_boolean(iflags.num_pad); /* use numbers for movement commands */
        lisp_boolean(iflags.news);    /* print news */
        lisp_boolean(
            iflags.window_inited); /* true if init_nhwindows() completed */
        lisp_boolean(iflags.vision_inited); /* true if vision is ready */
        lisp_boolean(iflags.menu_tab_sep); /* Use tabs to separate option menu
                                              fields */
        lisp_boolean(
            iflags.menu_requested); /* Flag for overloaded use of 'm' prefix
                                     * on some non-move commands */
        lisp_int(iflags.num_pad_mode); lisp_int(
            iflags
                .purge_monsters); /* # of dead monsters still on fmon list */
        /*      lisp_int(*iflags.opt_booldup);  /\* for duplication of boolean
           opts in config file *\/ */
        /*      lisp_int(*iflags.opt_compdup);  /\* for duplication of
           compound opts in config file *\/ */
        lisp_int(iflags.bouldersym); /* symbol for boulder display */
        lisp_coord(iflags.travelcc); /* coordinates for travel_cache */
#ifdef WIZARD
        lisp_boolean(iflags.sanity_check); /* run sanity checks */
        lisp_boolean(
            iflags.mon_polycontrol); /* debug: control monster polymorphs */
#endif
    );
}

/* call once for each field, then call with BL_FLUSH to output the result */

/* Note that a bunch of the configuration for colors doesn't live here, but
   instead resides in the Lisp half.  This allows us to some Lisp-based
   overrides of faces, as well as putting configuration in a custom-set
   variable instead of .nethacrc. */

void lisp_status_update(idx, ptr, chg, percent, color_and_attr,
                        colormasks) int idx,
    chg UNUSED, percent, color_and_attr UNUSED;
genericptr_t ptr;
unsigned long *colormasks UNUSED;
{
    long cond, *condptr = (long *) ptr;
    char *nb, *text = (char *) ptr;
    if (idx != BL_FLUSH) {
        if (idx < 0 || idx >= MAXBLSTATS)
            return; /* Should be a panic of some kind, but not sure */
        if (idx == BL_CONDITION) {
            cond = condptr ? *condptr : 0L;
            nb = status_vals[idx];
            *nb = '\0';
            if (cond & BL_MASK_STONE)
                Strcpy(nb = eos(nb), " Stone");
            if (cond & BL_MASK_SLIME)
                Strcpy(nb = eos(nb), " Slime");
            if (cond & BL_MASK_STRNGL)
                Strcpy(nb = eos(nb), " Strngl");
            if (cond & BL_MASK_FOODPOIS)
                Strcpy(nb = eos(nb), " FoodPois");
            if (cond & BL_MASK_TERMILL)
                Strcpy(nb = eos(nb), " TermIll");
            if (cond & BL_MASK_BLIND)
                Strcpy(nb = eos(nb), " Blind");
            if (cond & BL_MASK_DEAF)
                Strcpy(nb = eos(nb), " Deaf");
            if (cond & BL_MASK_STUN)
                Strcpy(nb = eos(nb), " Stun");
            if (cond & BL_MASK_CONF)
                Strcpy(nb = eos(nb), " Conf");
            if (cond & BL_MASK_HALLU)
                Strcpy(nb = eos(nb), " Hallu");
            if (cond & BL_MASK_LEV)
                Strcpy(nb = eos(nb), " Lev");
            if (cond & BL_MASK_FLY)
                Strcpy(nb = eos(nb), " Fly");
            if (cond & BL_MASK_RIDE)
                Strcpy(nb = eos(nb), " Ride");
            /* We've broken the loop a little, so this should be updated
             * independently */
            lisp_cmd("status-condition-update", lisp_quote;
                     lisp_list(status_vals[idx]););
        } else {
            if (idx == BL_GOLD) {
                /* decode once instead of every time it's displayed */
                status_vals[BL_GOLD][0] = ' ';
                text = decode_mixed(&status_vals[BL_GOLD][1], text);
                /* I'm too lazy to figure out how to cut out the “$:” prefix
                   here, so its handled by the Lisp side when gold is updated.
                   */
            }
            lisp_cmd("status-update", lisp_string(status_fieldnm[idx]);
                     lisp_string(text); lisp_int(percent););
        }
    } else { /* BL_FLUSH */
        lisp_cmd("print-status", );
    }
}

void lisp_putstr(window, attr, str) winid window;
int attr;
const char *str;
{
    int mesgflags;
    mesgflags = attr & (ATR_URGENT | ATR_NOHISTORY);
    attr &= ~mesgflags;

    if (window == WIN_STATUS) {
        /* generate_status_line(); */
        lisp_cmd("print-status", );
    } else if (window == WIN_MESSAGE && (mesgflags & ATR_NOHISTORY) != 0)
        lisp_cmd("message-nohistory", lisp_literal(attr_to_string(attr));
                 lisp_string(str));
    else if (window == WIN_MESSAGE)
        lisp_cmd("message", lisp_literal(attr_to_string(attr));
                 lisp_string(str));
    else
        lisp_cmd("menu-putstr", lisp_int(window);
                 lisp_literal(attr_to_string(attr)); lisp_string(str));
}

void lisp_start_menu(window) winid window;
{
    if (lisp_menu_item_list) {
        free((genericptr_t) lisp_menu_item_list);
        lisp_menu_item_list = NULL;
    }
    lisp_menu_list_num = 0;
    lisp_menu_list_max = 0;
    lisp_current_accelerator = 'a';
    lisp_cmd("start-menu", lisp_int(window));
}

void lisp_add_menu(window, glyph, identifier, ch, gch, attr, str, preselected)
    winid window;           /* window to use, must be of type NHW_MENU */
int glyph;                  /* glyph to display with item (unused) */
const anything *identifier; /* what to return if selected */
char ch;                    /* keyboard accelerator (0 = pick our own) */
char gch;                   /* group accelerator (0 = no group) */
int attr;                   /* attribute for string (like tty_putstr()) */
const char *str;            /* menu string */
boolean preselected;        /* item is marked as selected */
{
    if (identifier->a_void) {
        if (lisp_menu_list_num >= lisp_menu_list_max) {
            lisp_menu_list_max =
                lisp_menu_list_max ? lisp_menu_list_max * 2 : 16;
            lisp_menu_item_list = (lisp_menu_item_t *) realloc(
                lisp_menu_item_list,
                lisp_menu_list_max * sizeof(lisp_menu_item_t));
            if (lisp_menu_item_list == NULL)
                panic(
                    "Memory allocation failure; cannot grow menu item list");
        }
        lisp_menu_item_list[lisp_menu_list_num].identifier = *identifier;
        if (ch == 0) {
            ch = lisp_menu_item_list[lisp_menu_list_num].accelerator =
                lisp_current_accelerator;
            if (lisp_current_accelerator == 'z')
                lisp_current_accelerator = 'A';
            else if (lisp_current_accelerator == 'Z')
                lisp_current_accelerator = 'a';
            else
                lisp_current_accelerator++;
        } else
            lisp_menu_item_list[lisp_menu_list_num].accelerator = ch;

        lisp_menu_list_num++;
    } else
        ch = -1;

    lisp_cmd("add-menu", lisp_int(window); lisp_int(glyph);
             lisp_int((glyph == NO_GLYPH) ? -1 : glyph2tile[glyph]);
             lisp_int(ch); lisp_int(gch); lisp_literal(attr_to_string(attr));
             lisp_string(str); preselected ? lisp_t : lisp_nil);
}

void lisp_end_menu(window, prompt) winid window; /* menu to use */
const char *prompt;                              /* prompt to for menu */
{
    lisp_cmd("end-menu", lisp_int(window); lisp_string(prompt));
}

static int
lisp_get_menu_identifier(page, ch, identifier)
unsigned page;
char ch;
anything *identifier;
{
    int i;

    for (i = 0; i < lisp_menu_list_num; i++) {
        // this is obviously a dumb way to do implement "pages", but I
        // don't think we can just do page*(26*2)... or can we?
        if (lisp_menu_item_list[i].accelerator == ch && (page-- == 0)) {
            *identifier = lisp_menu_item_list[i].identifier;
            return 1;
        }
    }

    return 0;
}

int
lisp_select_menu(window, how, menu_list)
winid window;
int how;
menu_item **menu_list;
{
    const char *delim = "() \n";
    char *list;
    char *token;
    unsigned page;
    int size = 0;
    int toggle;

redo:
    lisp_cmd("select-menu", lisp_int(window);
             lisp_literal(how_to_string(how)));
    if (read_string("menu", &list) == -1)
        return size;

    /* The client should submit a structure like this:

     ((page ch count) (page ch count) (page ch count) ...)

     where page is menu_item_idx//(26*2+6), ch is the accelerator for
     the menu item and count is the number of them to select.

     We strtok it so we just get id count id count id count. */

    token = strtok(list, delim);

    /* Start with some memory so realloc doesn't fail. */
    *menu_list = malloc(sizeof(menu_item));
    if (*menu_list == NULL) {
        panic("Memory allocation failure; cannot get %lu bytes",
              sizeof(menu_item));
    }
    size = 0;

    while (token != NULL) {
        /* Make more room in the array for the new item */
        size++;
        if ((*menu_list = realloc(*menu_list, size * sizeof(menu_item)))
            == NULL) {
            panic("Memory allocation failure; cannot get %lu bytes",
                  size * sizeof(menu_item));
        }

        page = atoi(token);
        token = strtok(NULL, delim);
        if (token == NULL)
            break;
        /* assign the item ID */
        if (!lisp_get_menu_identifier(page, atoi(token),
                                      &(*menu_list)[size - 1].item)) {
            free(*menu_list);
            free(list);
            goto redo;
        }

        /* Read the item count */
        token = strtok(NULL, delim);
        if (token == NULL)
            break;
        (*menu_list)[size - 1].count = atoi(token);

        /* read the next item ID */
        token = strtok(NULL, delim);
    }

    free(list);

    return size;
}

/* This is a tty-specific hack. Do we need it? */
char
lisp_message_menu(let, how, mesg)
char let;
int how;
const char *mesg;
{
    lisp_cmd("message-menu", lisp_int(let); lisp_literal(how_to_string(how));
             lisp_string(mesg));
    return '\0';
}

static int lisp_get_cmd(str) const char *str;
{
    int i;

    for (i = 0; cmd_index[i].name != (char *) 0; i++) {
        if (!strcmp(str, cmd_index[i].name))
            return i;
    }

    return -1;
}

static int lisp_get_ext_cmd_id(str) const char *str;
{
    int i;

    for (i = 0; extcmdlist[i].ef_txt != (char *) 0; i++) {
        if (!strcmp(str, extcmdlist[i].ef_txt))
            return i;
    }

    return -1;
}

int
lisp_nhgetch()
{
    /* multi is not 0 if this  */
    static char count_buf[BUFSIZ] = "";
    static char *count_pos = count_buf;
    static int count_cmd = -1;
    int cmd;

    if (*count_pos) {
        char *tmp = count_pos;
        count_pos++;
        return *tmp;
    }

    if (count_cmd >= 0) {
        cmd = count_cmd;
        count_cmd = -1;
    } else {
        char cmdstr[BUFSZ];
        int nh_cmd = 0;

        while (!nh_cmd) {
            if (read_command("command", cmdstr, count_buf) == -1)
                return '\033';

            count_pos = count_buf;
            cmd = lisp_get_cmd(cmdstr);
            if (cmd == -1) {
                printf("(nethack-nhapi-message 'nethack-atr-none-face "
                       "\"undefined-command %s\")\n",
                       cmdstr);
            } else if (cmd_index[cmd].type == CMD_LISP) {
                /* We have to handle Lisp commands in this inner loop, because
                   they don't interact with the nethack layer. */
                /* FIXME: Maybe this should go in an array? */
                if (!strcmp(cmd_index[cmd].name, "options")) {
                    get_options();
                }
            } else {
                /* We have a nh command. */
                nh_cmd = 1;
            }
        }

        if (atoi(count_pos) > 1) {
            char *tmp = count_pos;
            count_pos++;
            count_cmd = cmd;
            return *tmp;
        } else {
            /* Since the count is 1, zero out the string. */
            *count_pos = 0;
        }
    }

    if (cmd_index[cmd].type == CMD_KEY) {
        return cmd_index[cmd].cmd;
    } else if (cmd_index[cmd].type == CMD_EXT) {
        if ((extended_cmd_id = lisp_get_ext_cmd_id(cmd_index[cmd].name))
            == -1) {
            /* Can never happen. */
            printf("%s:%d: Bad extended command name\n", __FILE__, __LINE__);
        }
        return '#';
    } else {
        impossible("Impossible command type: %d", cmd_index[cmd].type);
    }
}

int
lisp_nh_poskey(x, y, mod)
int *x, *y, *mod;
{
    return lisp_nhgetch();
}

static boolean inven_win_created = FALSE;

/* These globals are used to keep track of window IDs. */
static winid *winid_list = NULL;
static int winid_list_len = 0;
static int winid_list_max = 0;

/* returns index into winid_list that can be used. */
static int
find_empty_cell()
{
    int i;

    /* Check for a vacant spot in the list. */
    for (i = 0; i < winid_list_len; i++) {
        if (winid_list[i] == -1)
            return i;
    }

    /* no vacant ones, so grow the array. */
    if (winid_list_len >= winid_list_max) {
        winid_list_max *= 2;
        winid_list = realloc(winid_list, sizeof(int) * winid_list_max);
        if (winid_list == NULL)
            bail("Out of memory\n");
    }
    winid_list_len++;

    return winid_list_len - 1;
}

static int
winid_is_taken(winid n)
{
    int i;

    for (i = 0; i < winid_list_len; i++)
        if (winid_list[i] == n)
            return 1;

    return 0;
}

static int
add_winid(winid n)
{
    if (winid_is_taken(n))
        return 0; /* failed. */

    winid_list[find_empty_cell()] = n;
    return 1; /* success! */
}

static winid
get_unique_winid()
{
    winid i;

    /* look for a unique number, and add it to the list of taken
       numbers. */
    i = 0;
    while (!add_winid(i))
        i++;

    return i;
}

/* When a window is destroyed, it gives back its window number with
   this function. */
static void
return_winid(winid n)
{
    int i;

    for (i = 0; i < winid_list_len; i++) {
        if (winid_list[i] == n) {
            winid_list[i] = -1;
            return;
        }
    }
}

static void
init_winid_list()
{
    winid_list_max = 10;
    winid_list_len = 0;

    winid_list = malloc(winid_list_max * sizeof(int));
}

/* Prints a create_nhwindow function and expects from stdin the id of
   this new window as a number. */
winid
lisp_create_nhwindow(type)
int type;
{
    winid id = get_unique_winid();

    switch (type) {
    case NHW_MESSAGE:
        lisp_cmd("create-message-window", );
        break;
    case NHW_MAP:
        lisp_cmd("create-map-window", );
        break;
    case NHW_STATUS:
        lisp_cmd("create-status-window", );
        break;
    case NHW_TEXT:
        lisp_cmd("create-text-window", lisp_int(id));
        break;
    case NHW_MENU:
        if (!inven_win_created) {
            lisp_cmd("create-inventory-window", lisp_int(id));
            inven_win_created = TRUE;
        } else
            lisp_cmd("create-menu-window", lisp_int(id));
        break;
    default:
        impossible("Unknown window type: %d", type);
    };

    return id;
}

void lisp_clear_nhwindow(window) winid window;
{
    if (window == WIN_MESSAGE)
        lisp_cmd("clear-message", );
    else if (window == WIN_MAP)
        lisp_cmd("clear-map", );
    else
        /* are other window types ever cleared? */
        lisp_cmd("error", lisp_string("clearing unknown winid"));
}

void lisp_display_nhwindow(window, blocking) winid window;
boolean blocking;
{
    /* don't send display messages for anything but menus */
    char *dummy;
    if (window != WIN_MESSAGE && window != WIN_STATUS && window != WIN_MAP) {
        lisp_cmd("display-menu", lisp_int(window));
        if (read_string("dummy", &dummy) != -1)
            free(dummy);
    } else if (blocking) {
        if (window == WIN_MESSAGE && program_state.gameover) {
            lisp_cmd("end", );
        } else {
            lisp_cmd("block", );
            if (read_string("dummy", &dummy) != -1)
                free(dummy);
        }
    } else if (window == WIN_STATUS) {
        /* initial window setup hack here :) */
        lisp_cmd("restore-window-configuration", );
    }
}

void lisp_destroy_nhwindow(window) winid window;
{
    if ((window != WIN_STATUS) && (window != WIN_MESSAGE)
        && (window != WIN_MAP)) {
        lisp_cmd("destroy-menu", lisp_int(window));
        return_winid(window);
    }
}

void
lisp_update_inventory()
{
    lisp_cmd("update-inventory", );
    display_inventory(NULL, FALSE);
}

int
lisp_doprev_message()
{
    lisp_cmd("doprev-message", );
    return 0;
}

void
lisp_nhbell()
{
    lisp_cmd("nhbell", );
}

/* Can be an empty call says window.doc. */
void
lisp_mark_synch()
{
    return;
}

void
lisp_wait_synch()
{
    lisp_cmd("wait-synch", );
}

/* Since nethack will never be suspended, we need not worry about this
   function. */
void
lisp_resume_nhwindows()
{
    return;
}

/* Since nethack will never be suspended, we need not worry about this
   function. */
void lisp_suspend_nhwindows(str) const char *str;
{
    return;
}

/* All keys are defined in emacs, so number_pad makes no sense. */
void lisp_number_pad(state) int state;
{
    return;
}

void lisp_init_nhwindows(argcp, argv) int *argcp;
char **argv;
{
    char verbuf[BUFSZ];
    char *need_options_file_p;
    int i;

    gettimeofday(&start, NULL);

    printf("\n;; START LISP\n");

    lisp_cmd("need-options-file", );

    if (read_string("string", &need_options_file_p) != -1) {
        if (!strcmp(need_options_file_p, "t")) {
            char buf[BUFSZ];
            FILE *fp = fopen(configfile, "r");

            if (fp == NULL) {
                Sprintf(buf, "cannot open options file %s", configfile);
                lisp_cmd("receive-file", lisp_string(buf);
                         lisp_string("error"););
            } else {
                while (fgets(buf, BUFSZ, fp) != NULL) {
                    lisp_cmd("receive-file", lisp_string(buf););
                }

                fclose(fp);
                lisp_cmd("receive-file", lisp_string(""); lisp_t;);
            }
        }
        free(need_options_file_p);
    }

    /* Print each command-line option, constructing a list of strings */
    lisp_cmd("init-nhwindows", lisp_string(getversionstring(verbuf));
             for (i = 0; i < *argcp; i++) lisp_string(argv[i]));

    /* FIXME: doesn't remove the arguments parsed, as specified in the
       api doc. */

    /* Setup certain flags lisp clients need */
    iflags.num_pad = FALSE;
#ifdef EXP_ON_BOTL /* we are going to lose if Nethack is \
              compiled without this option -rcy */
    flags.showexp = TRUE;
#endif
    flags.time = TRUE;

    /* inform nethack that the windows have been initialized. */
    iflags.window_inited = TRUE;

    init_winid_list();
}

void lisp_exit_nhwindows(str) const char *str;
{
    lisp_cmd("exit-nhwindows ", lisp_string(str));
}

void
lisp_delay_output()
{
    char *dummy;
    lisp_cmd("delay-output", );
    if (read_string("dummy", &dummy) != -1)
        free(dummy);
}

void lisp_getlin(question, input) const char *question;
char *input;
{
    char *tmp;
    lisp_cmd("getlin", lisp_string(question));
    if (read_string("string", &tmp) == -1) {
        strcpy(input, "\033");
    } else {
        strncpy(input, tmp, BUFSZ - 1);
        free(tmp);
    }
}

int
lisp_get_ext_cmd()
{
    int cmd;
    if (extended_cmd_id != 0) {
        cmd = extended_cmd_id;
        extended_cmd_id = 0;
    } else {
        int i;

        printf("(nethack-nhapi-get-ext-cmd '(");

        for (i = 0; extcmdlist[i].ef_txt != (char *) 0; i++) {
            printf("(\"%s\" . %d)", extcmdlist[i].ef_txt, i);
        }
        printf("))\n");

        read_int("number", &cmd);
    }

    return cmd;
}

void lisp_display_file(str, complain) const char *str;
boolean complain;
{
    char *success;
    lisp_cmd("display-file", lisp_string(str); complain ? lisp_t : lisp_nil);
    ;

    if (!read_string("string", &success)) {
        if (strcmp(success, "t")) {
            char buf[BUFSZ];
            dlb *fp = dlb_fopen(str, "r");

            if (fp == NULL) {
                if (complain) {
                    Sprintf(buf, "cannot open file %s", str);
                    lisp_cmd("receive-file", lisp_string(buf);
                             lisp_string("error"););
                    ;
                }
                return;
            }

            while (dlb_fgets(buf, BUFSZ, fp) != NULL) {
                lisp_cmd("receive-file", lisp_string(buf););
                ;
            }

            dlb_fclose(fp);
            lisp_cmd("receive-file", lisp_string(""); lisp_t;);
            ;
        }
        free(success);
    }
}

char lisp_yn_function(ques, choices, def) const char *ques;
const char *choices;
char def;
{
    int answer;

    /* Some questions have special functions. */
    if (!strncmp(ques, "In what direction", 17)
        || !strncmp(ques, "Talk to whom? (in what direction)", 33)) {
        char *dir;
        lisp_cmd("ask-direction", lisp_string(ques));
        if (read_string("direction", &dir) == -1)
            return '\033';
        if (!strcmp(dir, "n"))
            answer = 'k';
        else if (!strcmp(dir, "s"))
            answer = 'j';
        else if (!strcmp(dir, "e"))
            answer = 'l';
        else if (!strcmp(dir, "w"))
            answer = 'h';
        else if (!strcmp(dir, "ne"))
            answer = 'u';
        else if (!strcmp(dir, "nw"))
            answer = 'y';
        else if (!strcmp(dir, "se"))
            answer = 'n';
        else if (!strcmp(dir, "sw"))
            answer = 'b';
        else if (!strcmp(dir, "up"))
            answer = '<';
        else if (!strcmp(dir, "down"))
            answer = '>';
        else if (!strcmp(dir, "self"))
            answer = '.';
        else {
            if (def == '\0')
                answer = 0x20; /* space */
            else
                answer = def;
        }

        free(dir);
    } else {
        lisp_cmd("yn-function", lisp_string(ques); lisp_string(choices);
                 lisp_int(def));
        read_int("number", &answer);
        answer = (answer == -1) ? '\033' : answer;
    }

    return (char) answer;
}

#ifdef POSITIONBAR
void lisp_update_positionbar(features) char *features;
{
    lisp_cmd("update-positionbar", lisp_string(features));
}
#endif

#define zap_color(n) zapcolors[n]
#define cmap_color(n) defsyms[n].color
#define obj_color(n) objects[n].oc_color
#define mon_color(n) mons[n].mcolor
#define invis_color(n) NO_COLOR
#define pet_color(n) mons[n].mcolor
#define warn_color(n) def_warnsyms[n].color

void lisp_print_glyph(window, x, y, glyph, bkglyph) winid window;
xchar x, y;
int glyph;
int bkglyph UNUSED;
{
    int ch;
    boolean reverse_on = FALSE;
    int color;
    unsigned special;
    int attr = -1;

    /* map glyph to character and color */
    (void) mapglyph(glyph, &ch, &color, &special, x, y, 0);

    if ((special & MG_PET) && iflags.hilite_pet) {
        attr = ATR_INVERSE;
    } else if ((special & (MG_DETECT | MG_BW_LAVA)) && iflags.use_inverse)
        attr = ATR_INVERSE;

    if (window == WIN_MAP) {
        lisp_cmd("print-glyph", lisp_int(x); lisp_int(y); lisp_int(color);
                 lisp_int(glyph); lisp_int(glyph2tile[glyph]); lisp_int(ch);
                 if (attr != -1) lisp_literal(attr_to_string(attr)););
    } else
        lisp_cmd("error", lisp_string("lisp_print_glyph bad window");
                 lisp_int(window));
}

#ifdef CLIPPING
void lisp_cliparound(x, y) int x;
int y;
{
    return;
}
#endif

void
lisp_start_screen()
{
    return;
} /* called from setftty() in unixtty.c */
void
lisp_end_screen()
{
    return;
} /* called from settty() in unixtty.c */
