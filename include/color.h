/* NetHack 3.6	color.h	$NHDT-Date: 1432512776 2015/05/25 00:12:56 $  $NHDT-Branch: master $:$NHDT-Revision: 1.13 $ */
/* Copyright (c) Steve Linhart, Eric Raymond, 1989. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef COLOR_H
#define COLOR_H

/*
 * The color scheme used is tailored for an IBM PC.  It consists of the
 * standard 8 colors, followed by their bright counterparts.  There are
 * exceptions, these are listed below.	Bright black doesn't mean very
 * much, so it is used as the "default" foreground color of the screen.
 */
#define CLR_BLACK 0
#define CLR_RED 1
#define CLR_GREEN 2
#define CLR_BROWN 3 /* on IBM, low-intensity yellow is brown */
#define CLR_BLUE 4
#define CLR_MAGENTA 5
#define CLR_CYAN 6
#define CLR_GRAY 7 /* low-intensity white */
#define NO_COLOR 8
#define CLR_ORANGE 9
#define CLR_BRIGHT_GREEN 10
#define CLR_YELLOW 11
#define CLR_BRIGHT_BLUE 12
#define CLR_BRIGHT_MAGENTA 13
#define CLR_BRIGHT_CYAN 14
#define CLR_WHITE 15
#define CLR_MAX 16

/* Extended 256-color support */
#define CLR_EXT_MAX 256
#define IS_EXT_COLOR(c) ((c) >= CLR_MAX && (c) < CLR_EXT_MAX)

/* The "half-way" point for tty based color systems.  This is used in */
/* the tty color setup code.  (IMHO, it should be removed - dean).    */
#define BRIGHT 8

/* these can be configured */
#define HI_OBJ        CLR_MAGENTA
#define HI_METAL      CLR_CYAN
#define HI_ORGANIC    CLR_BROWN
#define HI_LIQUID     CLR_BROWN
#define HI_PAPER      CLR_WHITE
#define HI_LEATHER    CLR_BROWN
#define HI_IRON       CLR_CYAN
#define DRAGON_SILVER CLR_BRIGHT_CYAN
#define HI_ZAP        CLR_BRIGHT_BLUE

/* Per-material 256-color macros.
   On 16-color terminals, mapglyph() falls back via map_color_256to16(). */
#define HI_WAX         229 /* lemon chiffon */
#define HI_VEGGY       64  /* olive green */
#define HI_FLESH       167 /* indian red */
#define HI_CLOTH       180 /* medium burlywood */
#define HI_SPIDER_SILK 253 /* very light gray */
#define HI_WOOD        94  /* dark goldenrod */
#define HI_BONE        230 /* cornsilk */
#define HI_DRAGON_HIDE 52  /* dark maroon */
#define HI_STEEL       67  /* steel blue */
#define HI_COPPER      166 /* rust */
#define HI_BRONZE      172 /* dark orange */
#define HI_SILVER      249 /* silver */
#define HI_GOLD        220 /* gold */
#define HI_PLATINUM    254 /* near white */
#define HI_MITHRIL     110 /* light steel blue */
#define HI_ADAMANTINE  236 /* dark gray */
#define HI_PLASTIC     156 /* pale green */
#define HI_GLASS       195 /* azure */
#define HI_GEMSTONE    196 /* bright red */
#define HI_MINERAL     244 /* gray */

/* Monster-specific 256-color macros (used in monst.c mcolor field).
   On 16-color terminals, mapglyph() falls back via mon_color_fallback(). */
#define HI_MPLAYER  228 /* player monsters */
#define HI_ORANGE   208 /* orange-furred/scaled */
#define HI_PURPLE   93  /* purple creatures */
#define HI_TREE     76  /* tree creatures */
#define HI_PLANT    106 /* plants */
#define HI_WATER    27  /* aquatic/blue */

/* 3.7-compatible 32-bit color encoding (additive overlay).
 * Lower 24 bits hold either a CLR_* index (when NH_BASIC_COLOR is set
 * in the high byte) or a 0xRRGGBB value (when no flag is set).
 * NH_ALTPALETTE is reserved for an alternate-palette future feature */
#define NH_BASIC_COLOR 0x1000000UL
#define NH_ALTPALETTE  0x2000000UL
#define COLORVAL(x)    ((x) & 0xFFFFFFUL)

/* Sentinel returned by mapglyph() when a glyph has a customcolor entry
 * that the windowport should resolve via customcolor_lookup().
 * Distinct from NO_COLOR (8), CLR_BLACK..CLR_WHITE (0..15), and the
 * IS_EXT_COLOR range (16..255). Negative so it can never collide */
#define NH_CUSTOMCOLOR_SENTINEL (-2)

enum nhcolortype { no_color, nh_color, rgb_color };

struct nethack_color {
    enum nhcolortype colortyp;
    int tableindex;
    int rgbindex;
    const char *name;
    long r, g, b;
};

/* Customcolor registry entry: per-glyph color override loaded from the
   CUSTOMCOLOR= rc directive (see src/glyphs.c) */
struct customcolor_entry {
    int glyphidx;
    unsigned long nhcolor;
    int color256idx;
    struct customcolor_entry *next;
};

struct menucoloring {
    struct nhregex *match;
    char *origstr;
    int color, attr;
    struct menucoloring *next;
};

#endif /* COLOR_H */
