/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/
/* NetHack 3.6 cursmisc.c */
/* Copyright (c) Karl Garrison, 2010. */
/* NetHack may be freely redistributed.  See license for details. */

#include "curses.h"
#include "hack.h"
#include "wincurs.h"
#include "cursmisc.h"
#include "func_tab.h"
#include "dlb.h"

#include <ctype.h>

/* Misc. curses interface functions */

/* Private declarations */

static int curs_x = -1;
static int curs_y = -1;

static int parse_escape_sequence(void);

/* Macros for Control and Alt keys */

#ifndef M
# ifndef NHSTDC
#  define M(c)          (0x80 | (c))
# else
#  define M(c)          ((c) - 128)
# endif/* NHSTDC */
#endif
#ifndef C
# define C(c)           (0x1f & (c))
#endif

/* Wrapper for getch() that supports debug_fuzzer mode.
   Use this instead of raw getch() throughout curses code. */

int
curses_getch()
{
    if (iflags.debug_fuzzer)
        return randomkey();
    return getch();
}

/* Read a character of input from the user */

int
curses_read_char()
{
    int ch;
#if defined(ALT_0) || defined(ALT_9) || defined(ALT_A) || defined(ALT_Z)
    int tmpch;
#endif

    /* cancel message suppression; all messages have had a chance to be read */
    curses_got_input();

    ch = curses_getch();
#if defined(ALT_0) || defined(ALT_9) || defined(ALT_A) || defined(ALT_Z)
    tmpch = ch;
#endif
    ch = curses_convert_keys(ch);

    if (ch == 0) {
        ch = '\033'; /* map NUL to ESC since nethack doesn't expect NUL */
    }
#if defined(ALT_0) && defined(ALT_9)    /* PDCurses, maybe others */
    if ((ch >= ALT_0) && (ch <= ALT_9)) {
        tmpch = (ch - ALT_0) + '0';
        ch = M(tmpch);
    }
#endif

#if defined(ALT_A) && defined(ALT_Z)    /* PDCurses, maybe others */
    if ((ch >= ALT_A) && (ch <= ALT_Z)) {
        tmpch = (ch - ALT_A) + 'a';
        ch = M(tmpch);
    }
#endif

#ifdef KEY_RESIZE
    /* Handle resize events via get_nh_event, not this code */
    if (ch == KEY_RESIZE) {
        ch = C('r'); /* NetHack doesn't know what to do with KEY_RESIZE */
    }
#endif

    if (counting && !isdigit(ch)) { /* Dismiss count window if necissary */
        curses_count_window(NULL);
        curses_refresh_nethack_windows();
    }

    return ch;
}

/* TRUE when terminfo advertises the RGB capability (direct-color
   entries like xterm-direct / tmux-direct). Set in
   curses_init_nhcolors() after start_color(). On direct-color
   terms ncurses interprets the color argument to init_pair() as a
   packed 24-bit RGB value rather than a palette slot index, so
   every pair allocator in this windowport must route through
   nh_init_pair() to translate slot indices to RGB. Always FALSE
   on PDCurses (lacks tigetflag and the extended-pair API) */
boolean curses_direct_color = FALSE;

#if NH_NCURSES_EXT_COLORS
/* RGB values for the 16 base ncurses color slots, used by
   nh_init_pair() to translate COLOR_BLACK..COLOR_WHITE (+8 bright
   variants) to packed RGB on direct-color terms. Uses the IBM/VGA
   "Linux console" palette instead of the xterm-canonical one:
   it has wider separation between dim and bright variants (0xAA
   vs 0xFF instead of 0xCD vs 0xFF) and a recognisably brown slot 3
   (0xAA5500 instead of the xterm olive 0xCDCD00). Most modern
   terminals use a palette closer to VGA than to xterm-defaults */
static const int xterm_base_rgb[16] = {
    0x000000, 0xAA0000, 0x00AA00, 0xAA5500, /* 0..3 black,red,green,brown */
    0x0000AA, 0xAA00AA, 0x00AAAA, 0xAAAAAA, /* 4..7 blue,magenta,cyan,gray */
    0x555555, 0xFF5555, 0x55FF55, 0xFFFF55, /* 8..11 dgray,orange,brgrn,yel */
    0x5555FF, 0xFF55FF, 0x55FFFF, 0xFFFFFF  /* 12..15 brblu,brmag,brcyn,wht */
};

/* xterm 256-color cube layout: 0..15 base, 16..231 6x6x6 cube,
   232..255 24-step grayscale. Cube component levels are the
   standard {0, 95, 135, 175, 215, 255}. Grayscale steps run
   from RGB(8,8,8) to RGB(238,238,238) in increments of 10 */
static int
xterm_256_rgb(idx)
int idx;
{
    static const int cube_levels[6] = { 0, 95, 135, 175, 215, 255 };
    int r, g, b, v;

    if (idx < 16)
        return xterm_base_rgb[idx];
    if (idx >= 232) {
        v = 8 + (idx - 232) * 10;
        return (v << 16) | (v << 8) | v;
    }
    idx -= 16;
    r = cube_levels[(idx / 36) % 6];
    g = cube_levels[(idx / 6) % 6];
    b = cube_levels[idx % 6];
    return (r << 16) | (g << 8) | b;
}
#endif /* NH_NCURSES_EXT_COLORS */

/* Wrapper around init_pair() that handles direct-color terminfo.
   On traditional palette terms (tmux-256color, xterm-256color, ...)
   ncurses' init_pair() takes a slot index; pass through unchanged.
   On direct-color terms (RGB capability advertised, e.g.
   tmux-direct, xterm-direct) the color argument is interpreted as
   a packed 24-bit RGB value that won't fit in short, so translate
   COLOR_BLACK..COLOR_WHITE and 256-cube indices to their xterm
   canonical RGB equivalents and route through init_extended_pair()
   instead. Caller can pass -1 for default fg/bg (use_default_colors)
   unchanged on either path */
int
nh_init_pair(pair, fg, bg)
int pair;
int fg;
int bg;
{
#if NH_NCURSES_EXT_COLORS
    if (curses_direct_color) {
        if (fg >= 0 && fg < 16)
            fg = xterm_base_rgb[fg];
        else if (fg >= 16 && fg < 256)
            fg = xterm_256_rgb(fg);
        if (bg >= 0 && bg < 16)
            bg = xterm_base_rgb[bg];
        else if (bg >= 16 && bg < 256)
            bg = xterm_256_rgb(bg);
        return init_extended_pair(pair, fg, bg);
    }
#endif
    return init_pair((short) pair, (short) fg, (short) bg);
}

/* On-demand extended color pair allocation.
 * Maps xterm colors 16-255 to curses pair numbers, allocating
 * pairs lazily to fit within limited COLOR_PAIRS (e.g. PDCurses=256).
 * With ~30 unique extended colors in EvilHack, this uses ~30 of the
 * 127 pairs available after status hilite bg pairs (129-255). */
static short ext_color_pair[240]; /* color-16 -> pair, 0=not yet */
static int next_ext_pair = 0;     /* next available; 0=uninitialized */

static int
get_ext_color_pair(color)
int color;
{
    int idx = color - 16;

    if (next_ext_pair == 0) {
        /* First call: start after status hilite bg pairs */
        next_ext_pair = (COLORS >= 16) ? 129 : 65;
    }
    if (ext_color_pair[idx] == 0 && next_ext_pair < COLOR_PAIRS) {
        nh_init_pair(next_ext_pair, color, -1);
        ext_color_pair[idx] = (short) next_ext_pair++;
    }
    return ext_color_pair[idx]; /* 0 if couldn't allocate */
}

/* On-demand (extended fg, solid hilite bg) pair allocation for the map
 * highlights (object pile / hidden stairs / ridden steed). The fixed
 * pair table built in curses_init_pairs() only covers base colors 0-15;
 * an extended 256-palette foreground (EvilHack material colors) needs a
 * pair built here so the exact color survives the highlight instead of
 * overflowing the table and rendering blank. Shares the in-game
 * on-demand pool (next_ext_pair, 129+) with get_ext_color_pair() */
static short ext_hilite_pair[240][2]; /* [color-16][blue] -> pair, 0=none */

static int
get_ext_hilite_pair(color, blue)
int color;
boolean blue;
{
    int idx = color - 16;
    int sub = blue ? 1 : 0;

    if (next_ext_pair == 0)
        next_ext_pair = (COLORS >= 16) ? 129 : 65;
    if (ext_hilite_pair[idx][sub] == 0 && next_ext_pair < COLOR_PAIRS) {
        nh_init_pair(next_ext_pair, color, blue ? COLOR_BLUE : COLOR_RED);
        ext_hilite_pair[idx][sub] = (short) next_ext_pair++;
    }
    return ext_hilite_pair[idx][sub]; /* 0 if pool exhausted */
}

/* Resolve a map highlight to a color pair whose foreground is fg over a
 * solid blue (else red) background. Base foregrounds reuse the fixed
 * pairs; extended foregrounds allocate on demand to keep the exact color.
 * A foreground that coincides with the highlight background would be
 * invisible, so substitute white */
static int
curses_hilite_pair(fg, blue)
int fg;
boolean blue;
{
    if (IS_EXT_COLOR(fg)) {
        int pair = get_ext_hilite_pair(fg, blue);

        if (pair > 0)
            return pair;
        fg = CLR_WHITE; /* pool exhausted: readable fallback */
    }
    if ((blue && fg == CLR_BLUE) || (!blue && fg == CLR_RED))
        fg = CLR_WHITE;
    return 17 + (fg * 2) + (blue ? 1 : 0);
}

#if NH_NCURSES_EXT_COLORS
/* 24-bit RGB pair cache. Sits at the top of COLOR_PAIRS (pairs) and
 * the top of COLORS (color slots) so neither collides with init_pair
 * (1..16+bg-hilites) or get_ext_color_pair (129..). Linear scan is
 * fine: ~30 unique customcolor entries in a normal rc-file, scanning
 * 256 is sub-microsecond.
 *
 * ncurses pair indices and color slot indices live in independent
 * namespaces. A pair references a (fg_slot, bg_slot) pair of color
 * slots. We allocate both from the top end of their respective ranges
 * so the existing 256-palette allocations stay untouched.
 *
 * On cache exhaustion (>256 unique RGB customcolors in one session)
 * we return 0; the caller falls back through the 256-quantized path */
/* Sized so the #showcolors demo can allocate one bg pair per cell
   for 5 gradient ramps of CURSES_PAL_GRADIENT_W (65) cells without
   exhausting and falling back to fg+block (which would reintroduce
   line-height gaps mid-ramp). In-game customcolors typically use
   <30 distinct values, so the extra slots cost ~1.5KB of static
   memory and nothing else */
#define TRUECOLOR_CACHE_SIZE 384
static struct { unsigned long rgb; int pair; } tc_cache[TRUECOLOR_CACHE_SIZE];
static int next_tc_pair = 0;       /* next pair index; 0=uninitialized */
static int next_tc_color = 0;      /* next color slot; 0=uninitialized */

static int
get_truecolor_pair(nhcolor)
unsigned long nhcolor;
{
    int rgb = (int) COLORVAL(nhcolor);
    int r = (rgb >> 16) & 0xff;
    int g = (rgb >> 8) & 0xff;
    int b = rgb & 0xff;
    int slot, i;

    if (next_tc_pair == 0) {
        /* Pair headroom is needed on every path. The color-slot
         * headroom check only applies to traditional palette terms,
         * where we allocate slots and reference them by index; on
         * direct-color terms (RGB cap) the slot indirection is a
         * no-op and what matters is the RGB value passed directly
         * to init_extended_pair() */
        if (COLOR_PAIRS < TRUECOLOR_CACHE_SIZE + 384)
            return 0;
        if (!curses_direct_color && COLORS < 256 + TRUECOLOR_CACHE_SIZE)
            return 0;
        next_tc_pair = COLOR_PAIRS - TRUECOLOR_CACHE_SIZE;
        next_tc_color = curses_direct_color
                            ? 0
                            : COLORS - TRUECOLOR_CACHE_SIZE;
    }

    /* Linear-scan the cache for an existing pair */
    for (i = 0; i < TRUECOLOR_CACHE_SIZE; i++)
        if (tc_cache[i].pair != 0 && tc_cache[i].rgb == nhcolor)
            return tc_cache[i].pair;

    if (next_tc_pair >= COLOR_PAIRS)
        return 0; /* pair-cache exhausted */

    if (curses_direct_color) {
        /* On direct-color terms the slot index passed to
           init_extended_pair() IS the RGB value; init_extended_color
           is a no-op since there's no palette slot to update.
           tmux-direct / xterm-direct setaf has an if-else branch:
           values 0..7 are treated as ANSI palette slots, only >=8
           are emitted as direct-color SGR. RGB values < 8 (very
           dark blue-only customcolors and pure black) would render
           as ANSI 0..7 instead of direct color; bump to 8 (=
           RGB(0,0,8)) so the cell still gets the direct-color
           path, accepting a few units of blue tint at the bottom
           of the range */
        if (init_extended_pair(next_tc_pair, rgb < 8 ? 8 : rgb, -1)
            == ERR)
            return 0;
    } else {
        if (next_tc_color >= COLORS)
            return 0; /* color slots exhausted */
        /* ncurses scale is 0..1000 per channel, not 0..255 */
        if (init_extended_color(next_tc_color, r * 1000 / 255,
                                               g * 1000 / 255,
                                               b * 1000 / 255) == ERR)
            return 0;
        if (init_extended_pair(next_tc_pair, next_tc_color, -1) == ERR)
            return 0;
        next_tc_color++;
    }

    slot = next_tc_pair - (COLOR_PAIRS - TRUECOLOR_CACHE_SIZE);
    tc_cache[slot].rgb = nhcolor;
    tc_cache[slot].pair = next_tc_pair;
    return next_tc_pair++;
}
#endif /* NH_NCURSES_EXT_COLORS */

/* Bg-pair cache for #showcolors gradient rendering. The standard
   fg+block path leaves a 1-pixel line-height gap at top and bottom
   of every cell because most terminals don't fully fill the cell
   rectangle when drawing FULL BLOCK glyphs. Rendering as background
   color + space char makes the terminal paint the whole cell, so
   the gradient looks continuous. The bg cache lives in its own pair
   range below the fg ext-color cache so existing in-game pair
   allocations are unaffected. When the pair pool is exhausted
   (PDCurses' 256-pair cap) the demo falls back to fg+block */
static short ext_color_bg_pair[240]; /* color-16 -> bg pair, 0=not yet */
static int next_ext_bg_pair = 0;

static int
get_ext_color_bg_pair(color)
int color;
{
    int idx = color - 16;

    if (next_ext_bg_pair == 0) {
        /* Sit above the in-game fg ext cache (129..368) with a
           small gap */
        next_ext_bg_pair = (COLORS >= 16) ? 385 : 65 + 240;
    }
    if (ext_color_bg_pair[idx] == 0 && next_ext_bg_pair < COLOR_PAIRS) {
        nh_init_pair(next_ext_bg_pair, -1, color);
        ext_color_bg_pair[idx] = (short) next_ext_bg_pair++;
    }
    return ext_color_bg_pair[idx];
}

#if NH_NCURSES_EXT_COLORS
/* Bg-pair sibling of the truecolor fg cache. Lives in the pair
   range just below the fg cache so neither collides with the other
   nor with the base/status/ext-color allocations. Same setab < 8
   bump applies (tmux-direct setab branches palette-vs-direct at the
   same threshold as setaf) */
static struct { unsigned long rgb; int pair; } tc_bg_cache[TRUECOLOR_CACHE_SIZE];
static int next_tc_bg_pair = 0;
static int next_tc_bg_color = 0;

static int
get_truecolor_bg_pair(nhcolor)
unsigned long nhcolor;
{
    int rgb = (int) COLORVAL(nhcolor);
    int r = (rgb >> 16) & 0xff;
    int g = (rgb >> 8) & 0xff;
    int b = rgb & 0xff;
    int slot, i;

    if (next_tc_bg_pair == 0) {
        if (COLOR_PAIRS < 2 * TRUECOLOR_CACHE_SIZE + 384)
            return 0;
        if (!curses_direct_color
            && COLORS < 256 + 2 * TRUECOLOR_CACHE_SIZE)
            return 0;
        next_tc_bg_pair = COLOR_PAIRS - 2 * TRUECOLOR_CACHE_SIZE;
        next_tc_bg_color = curses_direct_color
                            ? 0
                            : COLORS - 2 * TRUECOLOR_CACHE_SIZE;
    }

    for (i = 0; i < TRUECOLOR_CACHE_SIZE; i++)
        if (tc_bg_cache[i].pair != 0 && tc_bg_cache[i].rgb == nhcolor)
            return tc_bg_cache[i].pair;

    if (next_tc_bg_pair >= COLOR_PAIRS - TRUECOLOR_CACHE_SIZE)
        return 0; /* hit the fg cache range */

    if (curses_direct_color) {
        if (init_extended_pair(next_tc_bg_pair, -1,
                               rgb < 8 ? 8 : rgb) == ERR)
            return 0;
    } else {
        if (next_tc_bg_color >= COLORS - TRUECOLOR_CACHE_SIZE)
            return 0;
        if (init_extended_color(next_tc_bg_color,
                                 r * 1000 / 255,
                                 g * 1000 / 255,
                                 b * 1000 / 255) == ERR)
            return 0;
        if (init_extended_pair(next_tc_bg_pair, -1,
                               next_tc_bg_color) == ERR)
            return 0;
        next_tc_bg_color++;
    }

    slot = next_tc_bg_pair - (COLOR_PAIRS - 2 * TRUECOLOR_CACHE_SIZE);
    tc_bg_cache[slot].rgb = nhcolor;
    tc_bg_cache[slot].pair = next_tc_bg_pair;
    return next_tc_bg_pair++;
}
#endif /* NH_NCURSES_EXT_COLORS */

/* Turn on or off the specified color and / or attribute */

void
curses_toggle_color_attr(WINDOW *win, int color, int attr, int onoff)
{
#ifdef TEXTCOLOR
    int curses_color;

    /* if color is disabled, just show attribute */
    if ((win == mapwin) ? !iflags.wc_color
                        /* statuswin is for #if STATUS_HILITES
                           but doesn't need to be conditional */
                        : !(iflags.wc2_guicolor || win == statuswin)) {
#endif
        if (attr != NONE) {
            if (onoff == ON)
                wattron(win, attr);
            else
                wattroff(win, attr);
        }
        return;
#ifdef TEXTCOLOR
    }

    /* Map hilite (object pile / hidden stairs / ridden steed): low byte
       is the foreground color, CURSES_HILITE_BLUE selects the blue (else
       red) background. Resolve to a pair that keeps the exact foreground,
       including extended 256-palette colors that the old color*2 index
       overflowed and rendered blank */
    if (color & CURSES_BG_FLAG) {
        curses_color = curses_hilite_pair(color & 0xFF,
                                          (color & CURSES_HILITE_BLUE) != 0);
        goto apply_pair;
    }

    /* Extended 256-color (on-demand pair allocation) */
    if (IS_EXT_COLOR(color)) {
        if (COLORS >= 256) {
            curses_color = get_ext_color_pair(color);
            if (curses_color > 0)
                goto apply_pair;
        }
        /* Fallback: map to nearest base-16 color */
        color = map_color_256to16(color);
        curses_color = color + 1;
        goto apply_pair;
    }

    if (color == 0) {           /* make black fg visible */
# ifdef USE_DARKGRAY
        if (iflags.wc2_darkgray) {
            if (COLORS > 16) {
                /* colorpair for black is already darkgray */
            } else {            /* Use bold for a bright black */
                wattron(win, A_BOLD);
            }
        } else
# endif/* USE_DARKGRAY */
            color = CLR_BLUE;
    }
    curses_color = color + 1;
    if (COLORS < 16) {
        if (curses_color > 8 && curses_color < 17)
            curses_color -= 8;
        else if (curses_color > (17 + 16))
            curses_color -= 16;
    }

 apply_pair:
    if (onoff == ON) {          /* Turn on color/attributes */
        if (color != NONE) {
            if ((((color > 7) && (color < 17)) ||
                 (color > 17 + 17)) && (COLORS < 16)) {
                wattron(win, A_BOLD);
            }
            /* COLOR_PAIR()'s pair index occupies only 8 bits inside
               the chtype (A_COLOR mask). Pair indices above 255 wrap
               or alias when emitted via wattron(COLOR_PAIR(...)); use
               the wcolor_set() path instead so the full short-width
               pair range stays addressable. (short) is portable -
               PDCurses takes short, ncurses' NCURSES_PAIRS_T is short
               on non-ext-colors builds and int on ext-colors builds
               where this 256-cube range still fits) */
            if (curses_color > 255)
                (void) wcolor_set(win, (short) curses_color, NULL);
            else
                wattron(win, COLOR_PAIR(curses_color));
        }

        if (attr != NONE) {
            wattron(win, attr);
        }
    } else {                    /* Turn off color/attributes */

        if (color != NONE) {
            if ((color > 7) && (COLORS < 16)) {
                wattroff(win, A_BOLD);
            }
# ifdef USE_DARKGRAY
            if ((color == 0) && (COLORS <= 16)) {
                wattroff(win, A_BOLD);
            }
# else
            if (iflags.use_inverse) {
                wattroff(win, A_REVERSE);
            }
# endif/* DARKGRAY */
            if (curses_color > 255)
                (void) wcolor_set(win, 0, NULL);
            else
                wattroff(win, COLOR_PAIR(curses_color));
        }

        if (attr != NONE) {
            wattroff(win, attr);
        }
    }
#else
    nhUse(color);
#endif /* TEXTCOLOR */
}

/* Map-glyph color/attr toggle that knows about 24-bit RGB nhcolor.
   When the windowport runtime has truecolor and nhcolor carries a raw
   RGB value (NH_BASIC_COLOR not set), allocate or reuse a pair from
   the truecolor cache and apply via wattr_set with the opts pointer.
   The opts-pointer form is required because the truecolor cache sits
   at the top of COLOR_PAIRS (e.g. 65280..65535 on tmux-direct) and
   NCURSES_PAIRS_T is typically a 16-bit short, so the inline pair
   argument truncates the high bits and silently corrupts the cell.
   The wattr_set macro reads *(int *)opts when opts is non-NULL,
   bypassing the short narrowing. Every other case falls through to
   the existing 16/256-palette code path so callers that don't carry
   RGB continue to work byte-for-byte */
void
curses_toggle_color_attr32(WINDOW *win, int color, unsigned long nhcolor,
                           int attr, int onoff)
{
#if defined(TEXTCOLOR) && NH_NCURSES_EXT_COLORS
    if (nhcolor && !(nhcolor & NH_BASIC_COLOR)
        && (windowprocs.wincap2 & WC2_TRUECOLOR)
        && ((win == mapwin) ? iflags.wc_color
                            : (iflags.wc2_guicolor || win == statuswin))) {
        int pair = get_truecolor_pair(nhcolor);

        if (pair > 0) {
            if (onoff == ON) {
                int pair_int = pair;

                wattr_set(win,
                          attr != NONE ? (attr_t) attr : A_NORMAL,
                          0, &pair_int);
            } else {
                wattr_set(win, A_NORMAL, 0, NULL);
            }
            return;
        }
        /* cache exhausted, fall through to palette path */
    }
#else
    nhUse(nhcolor);
#endif /* TEXTCOLOR && NH_NCURSES_EXT_COLORS */
    curses_toggle_color_attr(win, color, attr, onoff);
}

#ifdef TEXTCOLOR
/* Public renderer for #showcolors on the curses windowport. Mirrors
   the tty version's three-bar layout (16-color base / 256-color cube /
   24-bit gradient) but renders inside ncurses' screen model so the
   game windows can be restored cleanly afterwards.
   Uses foreground full-block rendering for every bar; per-cell bg
   pairs would overrun PDCurses' 256-pair cap. The truecolor pair
   cache (256 slots) is shared with the regular map renderer */
/* Sized to align the right edge of the gradient bars with the
   256-cube bars: 256-cube row is 72 cells starting at column 3
   (rightmost cell at column 74); gradient labels are 7 chars wide
   at column 3 (cells start at column 10), so 74 - 10 + 1 = 65 */
#define CURSES_PAL_GRADIENT_W 65
void
curses_show_color_palette()
{
    WINDOW *win = stdscr;
    const char *block = SYMHANDLING(H_UTF8) ? "\xe2\x96\x88" : "#";
    const char *envterm = nh_getenv("TERM");
    const char *cterm = nh_getenv("COLORTERM");
    const char *envstr;
    char buf[BUFSZ];
    int y = 0, x, i, idx, v, row, env_depth;
    boolean truecolor_active;
    unsigned long rgb;

    /* Show the env-advertised depth only; the curses runtime depth is
       implied by which of the three bars actually renders below
       (16-color base / 256-color cube / 24-bit gradient). Dropping the
       "Active" line keeps the whole demo inside 24 rows when the
       truecolor section is present */
    env_depth = detect_env_color_depth(COLORS);
    truecolor_active = has_truecolor();
    if (env_depth >= 16777216)
        envstr = "24-bit truecolor";
    else if (env_depth >= 256)
        envstr = "256-color";
    else
        envstr = "16-color";

    erase();

    /* Header */
    Sprintf(buf, "  TERM = %s  COLORTERM = %s",
            envterm ? envterm : "(unset)",
            cterm ? cterm : "(unset)");
    mvwaddstr(win, y++, 0, buf);
    Sprintf(buf, "  Detected = %s", envstr);
    mvwaddstr(win, y++, 0, buf);
    y++;

    /* Bar 1: 16-color base palette, two rows of 8 + labels.
       Uses init_pair pairs 1..16 set up in curses_init_nhcolors */
    mvwaddstr(win, y++, 0,
              "  16-color base palette (CLR_BLACK..CLR_WHITE):");
    for (row = 0; row < 2; row++) {
        wmove(win, y++, 3);
        for (x = 0; x < 8; x++) {
            i = row * 8 + x;
            curses_toggle_color_attr(win, i, A_NORMAL, ON);
            for (idx = 0; idx < 7; idx++)
                waddstr(win, block);
            curses_toggle_color_attr(win, i, A_NORMAL, OFF);
            waddstr(win, " ");
        }
        wmove(win, y++, 3);
        for (x = 0; x < 8; x++) {
            i = row * 8 + x;
            Sprintf(buf, "%-7.7s ", clrlabels[i]);
            waddstr(win, buf);
        }
    }
    Sprintf(buf, "   ('black' = dark gray with use_darkgray, blue"
                 " without; current: %s)",
            iflags.wc2_darkgray ? "ON" : "OFF");
    mvwaddstr(win, y++, 0, buf);
    y++;

    /* Bar 2: 256-color extended palette. fg-block rendering means
       a thin line-height gap between the 3 cube rows; acceptable.
       Gated on COLORS (ncurses runtime), not env_depth, because
       ncurses can only allocate 256-color pairs when terminfo has
       the slots */
    if (COLORS >= 256) {
        mvwaddstr(win, y++, 0,
                  "  256-color extended palette (16..231 RGB,"
                  " 232..255 grayscale):");
        for (row = 0; row < 3; row++) {
            wmove(win, y++, 3);
            for (x = 0; x < 72; x++) {
                int bg_pair = 0;

                idx = 16 + row * 72 + x;
                if (iflags.wc2_guicolor)
                    bg_pair = get_ext_color_bg_pair(idx);
                if (bg_pair > 0) {
                    int p = bg_pair;

                    wattr_set(win, A_NORMAL, 0, &p);
                    waddstr(win, " ");
                    wattr_set(win, A_NORMAL, 0, NULL);
                } else {
                    curses_toggle_color_attr(win, idx, A_NORMAL, ON);
                    waddstr(win, block);
                    curses_toggle_color_attr(win, idx, A_NORMAL, OFF);
                }
            }
        }
        wmove(win, y++, 3);
        for (i = 232; i < 256; i++) {
            int bg_pair = 0;

            if (iflags.wc2_guicolor)
                bg_pair = get_ext_color_bg_pair(i);
            if (bg_pair > 0) {
                int p = bg_pair;

                wattr_set(win, A_NORMAL, 0, &p);
                waddstr(win, "  ");
                wattr_set(win, A_NORMAL, 0, NULL);
            } else {
                curses_toggle_color_attr(win, i, A_NORMAL, ON);
                waddstr(win, block);
                waddstr(win, block);
                curses_toggle_color_attr(win, i, A_NORMAL, OFF);
            }
        }
        y++;
    }

    /* Bar 3: 24-bit truecolor gradients */
    if (truecolor_active) {
        static const char *const grad_labels[5] = {
            "Hue:   ", "Gray:  ", "Red:   ", "Green: ", "Blue:  "
        };
        mvwaddstr(win, y++, 0,
                  "  24-bit truecolor demo"
                  " (active -- gradients should be smooth):");
        for (row = 0; row < 5; row++) {
            mvwaddstr(win, y, 3, grad_labels[row]);
            for (x = 0; x < CURSES_PAL_GRADIENT_W; x++) {
                /* Single-channel ramps (rows 2-4) start at v=8
                   instead of v=0 so the rgb value sits above the
                   ANSI-vs-direct-color threshold in setaf (values
                   0-7 are interpreted as ANSI palette slots, only
                   >=8 emit direct-color SGR). The gray ramp uses
                   the same start to keep the bottom of the demo
                   visually consistent. Hue ramp is unaffected
                   because every point on the wheel saturates at
                   least one channel at 255 */
                switch (row) {
                case 0:
                    rgb = hue_to_rgb(x * 360 / CURSES_PAL_GRADIENT_W);
                    break;
                case 1:
                    v = 8 + x * (255 - 8) / (CURSES_PAL_GRADIENT_W - 1);
                    rgb = ((unsigned long) v << 16)
                          | ((unsigned long) v << 8)
                          | (unsigned long) v;
                    break;
                case 2:
                    v = 8 + x * (255 - 8) / (CURSES_PAL_GRADIENT_W - 1);
                    rgb = (unsigned long) v << 16;
                    break;
                case 3:
                    v = 8 + x * (255 - 8) / (CURSES_PAL_GRADIENT_W - 1);
                    rgb = (unsigned long) v << 8;
                    break;
                default:
                    v = 8 + x * (255 - 8) / (CURSES_PAL_GRADIENT_W - 1);
                    rgb = (unsigned long) v;
                    break;
                }
#if NH_NCURSES_EXT_COLORS
                {
                    int bg_pair = 0;

                    if (iflags.wc2_guicolor)
                        bg_pair = get_truecolor_bg_pair(rgb);
                    if (bg_pair > 0) {
                        int p = bg_pair;

                        wattr_set(win, A_NORMAL, 0, &p);
                        waddstr(win, " ");
                        wattr_set(win, A_NORMAL, 0, NULL);
                        continue;
                    }
                }
#endif
                curses_toggle_color_attr32(win, NO_COLOR, rgb,
                                           A_NORMAL, ON);
                waddstr(win, block);
                curses_toggle_color_attr32(win, NO_COLOR, rgb,
                                           A_NORMAL, OFF);
            }
            y++;
        }
    } else if (env_depth >= 16777216 && COLORS <= 256) {
        mvwaddstr(win, y++, 0,
                  "  (env advertises truecolor but terminfo has"
                  " only 256 color slots; switch TERM to a -direct");
        mvwaddstr(win, y++, 0,
                  "   variant -- e.g. tmux-direct or xterm-direct"
                  " -- to enable ncurses truecolor)");
    } else if (env_depth >= 16777216) {
        mvwaddstr(win, y++, 0,
                  "  (24-bit truecolor available but disabled;"
                  " set OPTIONS=truecolor in your rc-file)");
    } else {
        mvwaddstr(win, y++, 0,
                  "  (24-bit truecolor not detected; set TERM to"
                  " a -direct variant or");
        mvwaddstr(win, y++, 0,
                  "   export COLORTERM=truecolor before launching)");
    }
    y++;

    mvwaddstr(win, y, 0, "  --Press any key to return--");
    wrefresh(win);
    (void) curses_getch();

    /* Force a full redraw of the normal game windows */
    erase();
    clearok(curscr, TRUE);
    wrefresh(win);
    curses_refresh_nethack_windows();
}
#endif /* TEXTCOLOR */

/* call curses_toggle_color_attr() with 'menucolors' instead of 'guicolor'
   as the control flag */

void
curses_menu_color_attr(WINDOW *win, int color, int attr, int onoff)
{
    boolean save_guicolor = iflags.wc2_guicolor;

    /* curses_toggle_color_attr() uses 'guicolor' to decide whether to
       honor specified color, but menu windows have their own
       more-specific control, 'menucolors', so override with that here */
    iflags.wc2_guicolor = iflags.use_menu_color;
    curses_toggle_color_attr(win, color, attr, onoff);
    iflags.wc2_guicolor = save_guicolor;
}

/* clean up and quit - taken from tty port */

void
curses_bail(const char *mesg)
{
    clearlocks();
    curses_exit_nhwindows(mesg);
    nh_terminate(EXIT_SUCCESS);
}

/* Return a winid for a new window of the given type */

winid
curses_get_wid(int type)
{
    static winid menu_wid = 20; /* Always even */
    static winid text_wid = 21; /* Always odd */
    winid ret;

    switch (type) {
    case NHW_MESSAGE:
        return MESSAGE_WIN;
    case NHW_MAP:
        return MAP_WIN;
    case NHW_STATUS:
        return STATUS_WIN;
    case NHW_MENU:
        ret = menu_wid;
        break;
    case NHW_TEXT:
        ret = text_wid;
        break;
    default:
        impossible("curses_get_wid: unsupported window type");
        ret = -1;
    }

    while (curses_window_exists(ret)) {
        ret += 2;
        if ((ret + 2) > 10000) {        /* Avoid "wid2k" problem */
            ret -= 9900;
        }
    }

    if (type == NHW_MENU) {
        menu_wid += 2;
    } else {
        text_wid += 2;
    }

    return ret;
}

/*
 * Allocate a copy of the given string.  If null, return a string of
 * zero length.
 *
 * This is taken from copy_of() in tty/wintty.c.
 */

char *
curses_copy_of(const char *s)
{
    if (!s)
        s = "";
    return dupstr(s);
}

/* Determine the number of lines needed for a string for a dialog window
   of the given width */

int
curses_num_lines(const char *str, int width)
{
    int last_space, count;
    int curline = 1;
    char substr[BUFSZ];
    char tmpstr[BUFSZ];

    strncpy(substr, str, BUFSZ-1);
    substr[BUFSZ-1] = '\0';

    while (strlen(substr) > (size_t) width) {
        last_space = 0;

        for (count = 0; count <= width; count++) {
            if (substr[count] == ' ')
                last_space = count;

        }
        if (last_space == 0) {  /* No spaces found */
            last_space = count - 1;
        }
        for (count = (last_space + 1); count < (int) strlen(substr); count++) {
            tmpstr[count - (last_space + 1)] = substr[count];
        }
        tmpstr[count - (last_space + 1)] = '\0';
        /* Use memmove for explicit bounds safety */
        memmove(substr, tmpstr, strlen(tmpstr) + 1);
        curline++;
    }

    return curline;
}

/* Break string into smaller lines to fit into a dialog window of the
given width */

char *
curses_break_str(const char *str, int width, int line_num)
{
    int last_space, count;
    char *retstr;
    int curline = 0;
    int strsize = (int) strlen(str) + 1;
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
    char substr[strsize];
    char curstr[strsize];
    char tmpstr[strsize];

    strcpy(substr, str);
#else
#ifndef BUFSZ
#define BUFSZ 256
#endif
    char substr[BUFSZ * 2];
    char curstr[BUFSZ * 2];
    char tmpstr[BUFSZ * 2];

    if (strsize > (BUFSZ * 2) - 1) {
        paniclog("curses", "curses_break_str() string too long.");
        strncpy(substr, str, (BUFSZ * 2) - 2);
        substr[(BUFSZ * 2) - 1] = '\0';
    } else
        strcpy(substr, str);
#endif

    while (curline < line_num) {
        if (strlen(substr) == 0) {
            break;
        }
        curline++;
        last_space = 0;
        for (count = 0; count <= width; count++) {
            if (substr[count] == ' ') {
                last_space = count;
            } else if (substr[count] == '\0') {
                last_space = count;
                break;
            }
        }
        if (last_space == 0) {  /* No spaces found */
            last_space = count - 1;
        }
        for (count = 0; count < last_space; count++) {
            curstr[count] = substr[count];
        }
        curstr[count] = '\0';
        if (substr[count] == '\0') {
            break;
        }
        for (count = (last_space + 1); count < (int) strlen(substr); count++) {
            tmpstr[count - (last_space + 1)] = substr[count];
        }
        tmpstr[count - (last_space + 1)] = '\0';
        /* Use memmove for explicit bounds safety */
        memmove(substr, tmpstr, strlen(tmpstr) + 1);
    }

    if (curline < line_num) {
        return NULL;
    }

    retstr = curses_copy_of(curstr);

    return retstr;
}

/* Return the remaining portion of a string after hacking-off line_num lines */

char *
curses_str_remainder(const char *str, int width, int line_num)
{
    int last_space, count;
    char *retstr;
    int curline = 0;
    int strsize = strlen(str) + 1;
#if __STDC_VERSION__ >= 199901L
    char substr[strsize];
    char tmpstr[strsize];

    strcpy(substr, str);
#else
#ifndef BUFSZ
#define BUFSZ 256
#endif
    char substr[BUFSZ * 2];
    char tmpstr[BUFSZ * 2];

    if (strsize > (BUFSZ * 2) - 1) {
        paniclog("curses", "curses_str_remainder() string too long.");
        strncpy(substr, str, (BUFSZ * 2) - 2);
        substr[(BUFSZ * 2) - 1] = '\0';
    } else
        strcpy(substr, str);
#endif

    while (curline < line_num) {
        if (strlen(substr) == 0) {
            break;
        }
        curline++;
        last_space = 0;
        for (count = 0; count <= width; count++) {
            if (substr[count] == ' ') {
                last_space = count;
            } else if (substr[count] == '\0') {
                last_space = count;
                break;
            }
        }
        if (last_space == 0) {  /* No spaces found */
            last_space = count - 1;
        }
        if (substr[last_space] == '\0') {
            break;
        }
        for (count = (last_space + 1); count < (int) strlen(substr); count++) {
            tmpstr[count - (last_space + 1)] = substr[count];
        }
        tmpstr[count - (last_space + 1)] = '\0';
        /* Use memmove for explicit bounds safety */
        memmove(substr, tmpstr, strlen(tmpstr) + 1);
    }

    if (curline < line_num) {
        return NULL;
    }

    retstr = curses_copy_of(substr);

    return retstr;
}

/* Determine if the given NetHack winid is a menu window */

boolean
curses_is_menu(winid wid)
{
    if ((wid > 19) && !(wid % 2)) {     /* Even number */
        return TRUE;
    } else {
        return FALSE;
    }
}

/* Determine if the given NetHack winid is a text window */

boolean
curses_is_text(winid wid)
{
    if ((wid > 19) && (wid % 2)) {      /* Odd number */
        return TRUE;
    } else {
        return FALSE;
    }
}

/* convert nethack's DECgraphics encoding into curses' ACS encoding */
int
curses_convert_glyph(int ch, int glyph)
{
    /* The DEC line drawing characters use 0x5f through 0x7e instead
       of the much more straightforward 0x60 through 0x7f, possibly
       because 0x7f is effectively a control character (Rubout);
       nethack ORs 0x80 to flag line drawing--that's stripped below */
    static int decchars[33]; /* for chars 0x5f through 0x7f (95..127) */

    ch &= 0xff; /* 0..255 only */
    if (!(ch & 0x80))
        return ch; /* no conversion needed */

    /* this conversion routine is only called for SYMHANDLING(H_DEC) and
       we decline to support special graphics symbols on the rogue level */
    if (Is_rogue_level(&u.uz)) {
        /* attempting to use line drawing characters will end up being
           rendered as lowercase gibberish */
        ch &= ~0x80;
        return ch;
    }

    /*
     * Curses has complete access to all characters that DECgraphics uses.
     * However, their character value isn't consistent between terminals
     * and implementations.  For actual DEC terminals and faithful emulators,
     * line-drawing characters are specified as lowercase letters (mostly)
     * and a control code is sent to the terminal telling it to switch
     * character sets (that's how the tty interface handles them).
     * Curses remaps the characters instead.
     */

    /* one-time initialization; some ACS_x aren't compile-time constant */
    if (!decchars[0]) {
        /* [0] is non-breakable space; irrelevant to nethack */
        decchars[0x5f - 0x5f] = ' '; /* NBSP */
        decchars[0x60 - 0x5f] = ACS_DIAMOND; /* [1] solid diamond */
        decchars[0x61 - 0x5f] = ACS_CKBOARD; /* [2] checkerboard */
        /* several "line drawing" characters are two-letter glyphs
           which could be substituted for invisible control codes;
           nethack's DECgraphics doesn't use any of them so we're
           satisfied with conversion to a simple letter;
           [3] "HT" as one char, with small raised upper case H over
           and/or preceding small lowered upper case T */
        decchars[0x62 - 0x5f] = 'H'; /* "HT" (horizontal tab) */
        decchars[0x63 - 0x5f] = 'F'; /* "FF" as one char (form feed) */
        decchars[0x64 - 0x5f] = 'C'; /* "CR" as one (carriage return) */
        decchars[0x65 - 0x5f] = 'L'; /* [6] "LF" as one (line feed) */
        decchars[0x66 - 0x5f] = ACS_DEGREE; /* small raised circle */
        /* [8] plus or minus sign, '+' with horizontal line below */
        decchars[0x67 - 0x5f] = ACS_PLMINUS;
        decchars[0x68 - 0x5f] = 'N'; /* [9] "NL" as one char (new line) */
        decchars[0x69 - 0x5f] = 'V'; /* [10] "VT" as one (vertical tab) */
        decchars[0x6a - 0x5f] = ACS_LRCORNER; /* lower right corner */
        decchars[0x6b - 0x5f] = ACS_URCORNER; /* upper right corner, 7-ish */
        decchars[0x6c - 0x5f] = ACS_ULCORNER; /* upper left corner */
        decchars[0x6d - 0x5f] = ACS_LLCORNER; /* lower left corner, 'L' */
        /* [15] center cross, like big '+' sign */
        decchars[0x6e - 0x5f] = ACS_PLUS;
        decchars[0x6f - 0x5f] = ACS_S1; /* very high horizontal line */
        decchars[0x70 - 0x5f] = ACS_S3; /* medium high horizontal line */
        decchars[0x71 - 0x5f] = ACS_HLINE; /* centered horizontal line */
        decchars[0x72 - 0x5f] = ACS_S7; /* medium low horizontal line */
        decchars[0x73 - 0x5f] = ACS_S9; /* very low horizontal line */
        /* [21] left tee, 'H' with right-hand vertical stroke removed;
           note on left vs right:  the ACS name (also DEC's terminal
           documentation) refers to vertical bar rather than cross stroke,
           nethack's left/right refers to direction of the cross stroke */
        decchars[0x74 - 0x5f] = ACS_LTEE; /* ACS left tee, NH right tee */
        /* [22] right tee, 'H' with left-hand vertical stroke removed */
        decchars[0x75 - 0x5f] = ACS_RTEE; /* ACS right tee, NH left tee */
        /* [23] bottom tee, '+' with lower half of vertical stroke
           removed and remaining stroke pointed up (unside-down 'T');
           nethack is inconsistent here--unlike with left/right, its
           bottom/top directions agree with ACS */
        decchars[0x76 - 0x5f] = ACS_BTEE; /* bottom tee, stroke up */
        /* [24] top tee, '+' with upper half of vertical stroke removed */
        decchars[0x77 - 0x5f] = ACS_TTEE; /* top tee, stroke down, 'T' */
        decchars[0x78 - 0x5f] = ACS_VLINE; /* centered vertical line */
        decchars[0x79 - 0x5f] = ACS_LEQUAL; /* less than or equal to */
        /* [27] greater than or equal to, '>' with underscore */
        decchars[0x7a - 0x5f] = ACS_GEQUAL;
        /* [28] Greek pi ('n'-like; case is ambiguous: small size
           suggests lower case but flat top suggests upper case) */
        decchars[0x7b - 0x5f] = ACS_PI;
        /* [29] not equal sign, combination of '=' and '/' */
        decchars[0x7c - 0x5f] = ACS_NEQUAL;
        /* [30] British pound sign (curly 'L' with embellishments) */
        decchars[0x7d - 0x5f] = ACS_STERLING;
        decchars[0x7e - 0x5f] = ACS_BULLET; /* [31] centered dot */
        /* [32] is not used for DEC line drawing but is a potential
           value for someone who assumes that 0x60..0x7f is the valid
           range, so we're prepared to accept--and sanitize--it */
        decchars[0x7f - 0x5f] = '?';
    }

    /* high bit set means special handling */
    if (ch & 0x80) {
        int convindx, symbol;

        ch &= ~0x80; /* force plain ASCII for last resort */
        convindx = ch - 0x5f;
        /* if it's in the lower case block of ASCII (which includes
           a few punctuation characters), use the conversion table */
        if (convindx >= 0 && convindx < SIZE(decchars)) {
            ch = decchars[convindx];
            /* in case ACS_foo maps to 0 when current terminal is unable
               to handle a particular character; if so, revert to default
               rather than using DECgr value with high bit stripped */
            if (!ch) {
                symbol = glyph_to_cmap(glyph);
                /* defsyms[] is only sized for cmap glyphs; a non-cmap
                   glyph (monster/object/etc. routed here via mapglyph)
                   would index defsyms[NO_GLYPH] out of bounds */
                if (symbol != NO_GLYPH)
                    ch = (int) defsyms[symbol].sym;
                else
                    ch = convindx + 0x5f; /* restore the stripped byte */
            }
        }
    }

    return ch;
}

/* convert nethack's IBMgraphics (CP437) encoding into curses' ACS
   encoding. Companion to curses_convert_glyph() above; called instead
   of it when SYMHANDLING(H_IBM) is active.

   Under ncursesw with a UTF-8 locale, mvwaddch() on a raw high byte
   (e.g. 0xC4) treats it as an incomplete UTF-8 lead and renders the
   "M-D" meta notation instead of the intended glyph. Translating the
   CP437 byte to an ACS_x macro routes the character through the
   terminal's alternate-character-set machinery, which is independent
   of UTF-8 byte rendering -- the same path DEC takes, and the reason
   the DECgraphics and 'curses' symsets render correctly on CP437
   terminals while raw IBMgraphics did not */
int
curses_convert_ibm_glyph(int ch, int glyph)
{
    /* CP437 high-byte (0x80..0xFF) to ACS_ mapping; indexed by
       (ch - 0x80). Lazy-initialized because ACS_x are runtime lookups
       into acs_map[], not compile-time constants. A static boolean
       sentinel is used because index 0 (CP437 0x80 = LATIN CAPITAL C
       WITH CEDILLA) has no ACS counterpart and so cannot double as
       the "initialized" flag the way curses_convert_glyph() uses
       decchars[0] for the DEC table */
    static int cp437_acs[128];
    static boolean cp437_acs_inited = FALSE;

    ch &= 0xff; /* 0..255 only */
    if (!(ch & 0x80))
        return ch; /* ASCII, no conversion needed */

    /* match the DEC handler's policy: rogue levels render line drawing
       as plain ASCII (line-drawing chars come out as lowercase
       gibberish there). RogueIBM's double-line wall fidelity is lost
       too -- acceptable; this mirrors DEC */
    if (Is_rogue_level(&u.uz)) {
        ch &= ~0x80;
        return ch;
    }

    if (!cp437_acs_inited) {
        /* Box drawing -- single-line, from primary IBMgraphics */
        cp437_acs[0xB3 - 0x80] = ACS_VLINE;     /* vertical */
        cp437_acs[0xB4 - 0x80] = ACS_RTEE;      /* right tee */
        cp437_acs[0xBF - 0x80] = ACS_URCORNER;  /* top right */
        cp437_acs[0xC0 - 0x80] = ACS_LLCORNER;  /* bottom left */
        cp437_acs[0xC1 - 0x80] = ACS_BTEE;      /* bottom tee */
        cp437_acs[0xC2 - 0x80] = ACS_TTEE;      /* top tee */
        cp437_acs[0xC3 - 0x80] = ACS_LTEE;      /* left tee */
        cp437_acs[0xC4 - 0x80] = ACS_HLINE;     /* horizontal */
        cp437_acs[0xC5 - 0x80] = ACS_PLUS;      /* cross */
        cp437_acs[0xD9 - 0x80] = ACS_LRCORNER;  /* bottom right */
        cp437_acs[0xDA - 0x80] = ACS_ULCORNER;  /* top left */

        /* Box drawing -- double-line variants from RogueIBM/RogueEpyx
           collapse to single-line ACS (ncurses has no double-line
           macros). Players who need double-line fidelity can use
           UTF8graphics on a UTF-8 terminal */
        cp437_acs[0xB9 - 0x80] = ACS_RTEE;
        cp437_acs[0xBA - 0x80] = ACS_VLINE;
        cp437_acs[0xBB - 0x80] = ACS_URCORNER;
        cp437_acs[0xBC - 0x80] = ACS_LRCORNER;
        cp437_acs[0xC8 - 0x80] = ACS_LLCORNER;
        cp437_acs[0xC9 - 0x80] = ACS_ULCORNER;
        cp437_acs[0xCA - 0x80] = ACS_BTEE;
        cp437_acs[0xCB - 0x80] = ACS_TTEE;
        cp437_acs[0xCC - 0x80] = ACS_LTEE;
        cp437_acs[0xCD - 0x80] = ACS_HLINE;
        cp437_acs[0xCE - 0x80] = ACS_PLUS;

        /* Shading / dots / scattered symbols actually used by the
           IBMgraphics family in dat/symbols */
        cp437_acs[0xB0 - 0x80] = ACS_CKBOARD;   /* light shade */
        cp437_acs[0xB1 - 0x80] = ACS_CKBOARD;   /* medium shade */
        cp437_acs[0xB2 - 0x80] = ACS_CKBOARD;   /* dark shade */
        cp437_acs[0xDB - 0x80] = ACS_BLOCK;     /* full block */
        cp437_acs[0xF9 - 0x80] = ACS_BULLET;    /* centered dot */
        cp437_acs[0xFA - 0x80] = ACS_BULLET;    /* centered dot */
        cp437_acs[0xF1 - 0x80] = ACS_PLMINUS;   /* plus or minus */
        cp437_acs[0xF8 - 0x80] = ACS_DEGREE;    /* degree sign */
        cp437_acs[0x9C - 0x80] = ACS_STERLING;  /* pound sterling */
        /* 0x04 is below 0x80 and would never reach this branch; it
           reaches the ASCII early-return above. RogueEpyx's diamond
           trap symbol thus renders as raw 0x04, which most curses
           builds treat as a control char. Acceptable: RogueEpyx is
           rarely used and a real fix would need ACS_DIAMOND mapped
           via a different mechanism */

        cp437_acs_inited = TRUE;
    }

    /* high bit guaranteed set here */
    {
        int convindx = ch - 0x80;
        int acsval = cp437_acs[convindx];

        if (acsval)
            return acsval;
        /* no ACS counterpart; fall back to the glyph's ASCII default
           symbol -- matches the fallback strategy in
           curses_convert_glyph(). Bytes that hit this path:
           0xF0 (equivalence), 0xF4 (integral upper), 0xF6 (division),
           0xF5 (section), 0xF7 (approximately equal), 0xFE (small
           square), 0xAD (inverted exclamation), 0xE7 (Greek tau), and
           any CP437 byte a user has assigned without an ACS_ match */
        {
            int symbol = glyph_to_cmap(glyph);
            /* defsyms[] is only sized for cmap glyphs; tmp_at() routes
               object/monster glyphs through here for projectile
               animation (frost-giant boulder throw repro) and
               defsyms[NO_GLYPH] would be far out of bounds */
            if (symbol != NO_GLYPH)
                ch = (int) defsyms[symbol].sym;
            else
                ch &= ~0x80; /* non-cmap glyph: degrade to plain ASCII */
        }
    }

    return ch;
}

/* Move text cursor to specified coordinates in the given NetHack window */

void
curses_move_cursor(winid wid, int x, int y)
{
    int sx, sy, ex, ey;
    int xadj = 0;
    int yadj = 0;

#ifndef PDCURSES
    WINDOW *win = curses_get_nhwin(MAP_WIN);
#endif

    if (wid != MAP_WIN) {
        return;
    }
#ifdef PDCURSES
    /* PDCurses seems to not handle wmove correctly, so we use move and
       physical screen coordinates instead */
    curses_get_window_xy(wid, &xadj, &yadj);
#endif
    curs_x = x + xadj;
    curs_y = y + yadj;
    curses_map_borders(&sx, &sy, &ex, &ey, x, y);

    if (curses_window_has_border(wid)) {
        curs_x++;
        curs_y++;
    }

    if (x >= sx && x <= ex && y >= sy && y <= ey) {
        /* map column #0 isn't used; shift column #1 to first screen column */
        curs_x -= (sx + 1);
        curs_y -= sy;
#ifdef PDCURSES
        move(curs_y, curs_x);
#else
        wmove(win, curs_y, curs_x);
#endif
    }
}

/* update the ncurses stdscr cursor to where the cursor in our map is */
void
curses_update_stdscr_cursor()
{
#ifndef PDCURSES
    int xadj = 0, yadj = 0;

    curses_get_window_xy(MAP_WIN, &xadj, &yadj);
    move(curs_y + yadj, curs_x + xadj);
#endif
}

/* Perform actions that should be done every turn before nhgetch() */

void
curses_prehousekeeping()
{
#ifndef PDCURSES
    WINDOW *win = curses_get_nhwin(MAP_WIN);
#endif /* PDCURSES */

    if ((curs_x > -1) && (curs_y > -1)) {
        curs_set(1);
#ifdef PDCURSES
        /* PDCurses seems to not handle wmove correctly, so we use move
           and physical screen coordinates instead */
        move(curs_y, curs_x);
#else
        wmove(win, curs_y, curs_x);
#endif /* PDCURSES */
        curses_refresh_nhwin(MAP_WIN);
    }
}

/* Perform actions that should be done every turn after nhgetch() */

void
curses_posthousekeeping()
{
    curs_set(0);
    /* curses_decrement_highlights(FALSE); */
    curses_clear_unhighlight_message_window();
}

void
curses_view_file(const char *filename, boolean must_exist)
{
    winid wid;
    anything Id;
    char buf[BUFSZ];
    menu_item *selected = NULL;
    dlb *fp = dlb_fopen(filename, "r");

    if (fp == NULL) {
        if (must_exist)
            pline("Cannot open \"%s\" for reading!", filename);
        return;
    }

    wid = curses_get_wid(NHW_MENU);
    curses_create_nhmenu(wid);
    Id = zeroany;

    while (dlb_fgets(buf, BUFSZ, fp) != NULL) {
        curses_add_menu(wid, NO_GLYPH, &Id, 0, 0, A_NORMAL, buf, FALSE);
    }

    dlb_fclose(fp);
    curses_end_menu(wid, "");
    curses_select_menu(wid, PICK_NONE, &selected);
    curses_del_wid(wid);
}

void
curses_rtrim(char *str)
{
    char *s;

    for (s = str; *s != '\0'; ++s);
    if (s > str) {
        for (--s; isspace(*s) && s > str; --s);
    }
    if (s == str)
        *s = '\0';
    else
        *(++s) = '\0';
}

/* Read numbers until non-digit is encountered, and return number
in int form. */

int
curses_get_count(int first_digit)
{
    long current_count = first_digit;
    int current_char;

    current_char = curses_read_char();

    while (isdigit(current_char)) {
        current_count = (current_count * 10) + (current_char - '0');
        if (current_count > LARGEST_INT) {
            current_count = LARGEST_INT;
        }

        custompline(SUPPRESS_HISTORY, "Count: %ld", current_count);
        current_char = curses_read_char();
    }

    ungetch(current_char);

    if (current_char == '\033') {     /* Cancelled with escape */
        current_count = -1;
    }

    return current_count;
}

/* Convert the given NetHack text attributes into the format curses
   understands, and return that format mask. */

int
curses_convert_attr(int attr)
{
    int curses_attr;

    /* first, strip off control flags masked onto the display attributes
       (caller should have already done this...) */
    attr &= ~(ATR_URGENT | ATR_NOHISTORY);

    switch (attr) {
    case ATR_NONE:
        curses_attr = A_NORMAL;
        break;
    case ATR_ULINE:
        curses_attr = A_UNDERLINE;
        break;
    case ATR_BOLD:
        curses_attr = A_BOLD;
        break;
    case ATR_DIM:
        curses_attr = A_DIM;
        break;
    case ATR_BLINK:
        curses_attr = A_BLINK;
        break;
    case ATR_INVERSE:
        curses_attr = A_REVERSE;
        break;
    default:
        curses_attr = A_NORMAL;
    }

    return curses_attr;
}

/* Map letter attributes from a string to bitmask.  Return mask on
   success (might be 0), or -1 if not found. */

int
curses_read_attrs(const char *attrs)
{
    int retattr = 0;

    if (!attrs || !*attrs)
        return A_NORMAL;

    if (strchr(attrs, 'b') || strchr(attrs, 'B'))
        retattr |= A_BOLD;
    if (strchr(attrs, 'i') || strchr(attrs, 'I')) /* inverse */
        retattr |= A_REVERSE;
    if (strchr(attrs, 'u') || strchr(attrs, 'U'))
        retattr |= A_UNDERLINE;
    if (strchr(attrs, 'k') || strchr(attrs, 'K'))
        retattr |= A_BLINK;
    if (strchr(attrs, 'd') || strchr(attrs, 'D'))
        retattr |= A_DIM;
#ifdef A_ITALIC
    if (strchr(attrs, 't') || strchr(attrs, 'T'))
        retattr |= A_ITALIC;
#endif
#ifdef A_LEFTLINE
    if (strchr(attrs, 'l') || strchr(attrs, 'L'))
        retattr |= A_LEFTLINE;
#endif
#ifdef A_RIGHTLINE
    if (strchr(attrs, 'r') || strchr(attrs, 'R'))
        retattr |= A_RIGHTLINE;
#endif
    if (retattr == 0) {
        /* still default; check for none/normal */
        if (strchr(attrs, 'n') || strchr(attrs, 'N'))
            retattr = A_NORMAL;
        else
            retattr = -1; /* error */
    }
    return retattr;
}

/* format iflags.wc2_petattr into "+a+b..." for set bits a, b, ...
   (used by core's 'O' command; return value points past leading '+') */
char *
curses_fmt_attrs(outbuf)
char *outbuf;
{
    int attr = iflags.wc2_petattr;

    outbuf[0] = '\0';
    if (attr == A_NORMAL) {
        Strcpy(outbuf, "+N(None)");
    } else {
        if (attr & A_BOLD)
            Strcat(outbuf, "+B(Bold)");
        if (attr & A_REVERSE)
            Strcat(outbuf, "+I(Inverse)");
        if (attr & A_UNDERLINE)
            Strcat(outbuf, "+U(Underline)");
        if (attr & A_BLINK)
            Strcat(outbuf, "+K(blinK)");
        if (attr & A_DIM)
            Strcat(outbuf, "+D(Dim)");
#ifdef A_ITALIC
        if (attr & A_ITALIC)
            Strcat(outbuf, "+T(iTalic)");
#endif
#ifdef A_LEFTLINE
        if (attr & A_LEFTLINE)
            Strcat(outbuf, "+L(Left line)");
#endif
#ifdef A_RIGHTLINE
        if (attr & A_RIGHTLINE)
            Strcat(outbuf, "+R(Right line)");
#endif
    }
    if (!*outbuf)
        Sprintf(outbuf, "+unknown [%d]", attr);
    return &outbuf[1];
}

/* Convert special keys into values that NetHack can understand.
Currently this is limited to arrow keys, but this may be expanded. */

int
curses_convert_keys(int key)
{
    int ret = key;

    if (ret == '\033') {
        ret = parse_escape_sequence();
    }

    /* Handle arrow keys */
    switch (key) {
    case KEY_BACKSPACE:
        /* we can't distinguish between a separate backspace key and
           explicit Ctrl+H intended to rush to the left; without this,
           a value for ^H greater than 255 is passed back to core's
           readchar() and stripping the value down to 0..255 yields ^G! */
        ret = C('H');
        break;
#ifdef KEY_B1
    case KEY_B1:
#endif
    case KEY_LEFT:
        if (iflags.num_pad) {
            ret = '4';
        } else {
            ret = 'h';
        }
        break;
#ifdef KEY_B3
    case KEY_B3:
#endif
    case KEY_RIGHT:
        if (iflags.num_pad) {
            ret = '6';
        } else {
            ret = 'l';
        }
        break;
#ifdef KEY_A2
    case KEY_A2:
#endif
    case KEY_UP:
        if (iflags.num_pad) {
            ret = '8';
        } else {
            ret = 'k';
        }
        break;
#ifdef KEY_C2
    case KEY_C2:
#endif
    case KEY_DOWN:
        if (iflags.num_pad) {
            ret = '2';
        } else {
            ret = 'j';
        }
        break;
#ifdef KEY_A1
    case KEY_A1:
#endif
    case KEY_HOME:
        if (iflags.num_pad) {
            ret = '7';
        } else {
            ret = !Cmd.swap_yz ? 'y' : 'z';
        }
        break;
#ifdef KEY_A3
    case KEY_A3:
#endif
    case KEY_PPAGE:
        if (iflags.num_pad) {
            ret = '9';
        } else {
            ret = 'u';
        }
        break;
#ifdef KEY_C1
    case KEY_C1:
#endif
    case KEY_END:
        if (iflags.num_pad) {
            ret = '1';
        } else {
            ret = 'b';
        }
        break;
#ifdef KEY_C3
    case KEY_C3:
#endif
    case KEY_NPAGE:
        if (iflags.num_pad) {
            ret = '3';
        } else {
            ret = 'n';
        }
        break;
#ifdef KEY_B2
    case KEY_B2:
        if (iflags.num_pad) {
            ret = '5';
        } else {
            ret = 'g';
        }
        break;
#endif /* KEY_B2 */
    }

    return ret;
}

/*
 * We treat buttons 2 and 3 as equivalent so that it doesn't matter which
 * one is for right-click and which for middle-click.  The core uses CLICK_2
 * for right-click ("not left"-click) even though 2 might be middle button.
 *
 * BUTTON_CTRL was enabled at one point but was not working as intended.
 * Ctrl+left_click was generating pairs of duplicated events with Ctrl and
 * Report_mouse_position bits set (even though Report_mouse_position wasn't
 * enabled) but no button click bit set.  (It sort of worked because Ctrl+
 * Report_mouse_position wasn't a left click so passed along CLICK_2, but
 * the duplication made that too annoying to use.  Attempting to immediately
 * drain the second one wasn't working as intended either.)
 */
#define MOUSEBUTTONS (BUTTON1_CLICKED | BUTTON2_CLICKED | BUTTON3_CLICKED)

/* Process mouse events.  Mouse movement is processed until no further
mouse movement events are available.  Returns 0 for a mouse click
event, or the first non-mouse key event in the case of mouse
movement. */

int
curses_get_mouse(int *mousex, int *mousey, int *mod)
{
    int key = '\033';

#ifdef NCURSES_MOUSE_VERSION
    MEVENT event;

    if (getmouse(&event) == OK) { /* True if user has clicked */
        if ((event.bstate & MOUSEBUTTONS) != 0) {
        /*
         * The ncurses man page documents wmouse_trafo() incorrectly.
         * It says that last argument 'TRUE' translates from screen
         * to window and 'FALSE' translates from window to screen,
         * but those are backwards.  The mouse_trafo() macro calls
         * last argument 'to_screen', suggesting that the backwards
         * implementation is the intended behavior and the man page
         * is describing it wrong.
         */
            /* See if coords are in map window & convert coords */
            if (wmouse_trafo(mapwin, &event.y, &event.x, FALSE)) {
                key = '\0'; /* core uses this to detect a mouse click */
                *mousex = event.x + 1; /* +1: screen 0..78 is map 1..79 */
                *mousey = event.y;

                if (curses_window_has_border(MAP_WIN)) {
                    (*mousex)--;
                    (*mousey)--;
                }

                *mod = ((event.bstate & (BUTTON1_CLICKED | BUTTON_CTRL))
                        == BUTTON1_CLICKED) ? CLICK_1 : CLICK_2;
            }
        }
    }
#endif /* NCURSES_MOUSE_VERSION */

    return key;
}

void
curses_mouse_support(mode)
int mode; /* 0: off, 1: on, 2: alternate on */
{
#ifdef NCURSES_MOUSE_VERSION
    mmask_t result, oldmask, newmask;

    if (!mode)
        newmask = 0;
    else
        newmask = MOUSEBUTTONS; /* buttons 1, 2, and 3 */

    result = mousemask(newmask, &oldmask);
    nhUse(result);
#else
    nhUse(mode);
#endif
}

static int
parse_escape_sequence(void)
{
#ifndef PDCURSES
    int ret;

    timeout(10);

    ret = curses_getch();

    if (ret != ERR) {           /* Likely an escape sequence */
        if (((ret >= 'a') && (ret <= 'z')) || ((ret >= '0') && (ret <= '9'))) {
            ret |= 0x80;        /* Meta key support for most terminals */
        } else if (ret == 'O') {        /* Numeric keypad */
            ret = curses_getch();
            if ((ret != ERR) && (ret >= 112) && (ret <= 121)) {
                ret = ret - 112 + '0';  /* Convert to number */
            } else {
                ret = '\033';   /* Escape */
            }
        }
    } else {
        ret = '\033';           /* Just an escape character */
    }

    timeout(-1);

    return ret;
#else
    return '\033';
#endif /* !PDCURSES */
}

