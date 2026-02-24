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
#define CLR_EXT_MAX     256
#define IS_EXT_COLOR(c) ((c) >= CLR_MAX && (c) < CLR_EXT_MAX)

/* The "half-way" point for tty based color systems.  This is used in */
/* the tty color setup code.  (IMHO, it should be removed - dean).    */
#define BRIGHT 8

/* these can be configured */
#define HI_OBJ CLR_MAGENTA
#define HI_METAL CLR_CYAN
#define HI_ORGANIC CLR_BROWN
#define HI_PAPER CLR_WHITE
#define DRAGON_SILVER CLR_BRIGHT_CYAN
#define HI_ZAP CLR_BRIGHT_BLUE

/* Per-material 256-color macros.
   On 16-color terminals, mapglyph() falls back via map_color_256to16(). */
#define HI_LIQUID      28  /* forest green */
#define HI_WAX         229 /* lemon chiffon */
#define HI_VEGGY       64  /* olive green */
#define HI_FLESH       167 /* indian red */
#define HI_CLOTH       137 /* burlywood */
#define HI_SPIDER_SILK 251 /* light gray */
#define HI_LEATHER     130 /* dark orange */
#define HI_WOOD        94  /* dark goldenrod */
#define HI_BONE        187 /* warm cream */
#define HI_DRAGON_HIDE 101 /* dark khaki */
#define HI_IRON        67  /* steel blue */
#define HI_STEEL       246 /* dark silver */
#define HI_COPPER      173 /* sandy brown */
#define HI_BRONZE      136 /* dark goldenrod */
#define HI_SILVER      249 /* silver */
#define HI_GOLD        220 /* gold */
#define HI_PLATINUM    255 /* near white */
#define HI_MITHRIL     110 /* light steel blue */
#define HI_ADAMANTINE  60  /* slate gray */
#define HI_PLASTIC     253 /* very light gray */
#define HI_GLASS       117 /* light blue */
#define HI_GEMSTONE    162 /* deep pink */
#define HI_MINERAL     245 /* gray */

struct menucoloring {
    struct nhregex *match;
    char *origstr;
    int color, attr;
    struct menucoloring *next;
};

#endif /* COLOR_H */
