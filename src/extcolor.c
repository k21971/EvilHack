/* NetHack 3.6  extcolor.c  $NHDT-Date: 2026/02/23 $  $NHDT-Branch: master $ */
/* Copyright (c) Keith Simpson, 2026. */
/* EvilHack may be freely redistributed.  See license for details. */

/* Extended 256-color support: RGB palette, 256-to-16 fallback mapping,
   and color distance metric.  RGB values and distance algorithm are
   adapted from NetHack 3.7 src/coloratt.c (originally from UnNetHack
   via https://www.compuphase.com/cmetric.htm). */

#include "hack.h"
#include "color.h"

/* RGB values for the standard xterm-256 color palette.
   Indices 0-15:  CGA palette (matching dat/NHdump.css).
   Indices 16-231: 6x6x6 color cube.
   Indices 232-255: 24-step grayscale ramp. */
static unsigned long color256_rgb[CLR_EXT_MAX] = {
    /* 0-15: CGA base colors */
    0x555555UL, 0xAA0000UL, 0x00AA00UL, 0xAA5500UL,
    0x0000AAUL, 0xAA00AAUL, 0x00AAAAUL, 0xAAAAAAUL,
    0x555555UL, 0xFF5555UL, 0x55FF55UL, 0xFFFF55UL,
    0x5555FFUL, 0xFF55FFUL, 0x55FFFFUL, 0xFCFCFCUL,
    /* 16-231: 6x6x6 color cube */
    0x000000UL, 0x00005FUL, 0x000087UL, 0x0000AFUL,
    0x0000D7UL, 0x0000FFUL, 0x005F00UL, 0x005F5FUL,
    0x005F87UL, 0x005FAFUL, 0x005FD7UL, 0x005FFFUL,
    0x008700UL, 0x00875FUL, 0x008787UL, 0x0087AFUL,
    0x0087D7UL, 0x0087FFUL, 0x00AF00UL, 0x00AF5FUL,
    0x00AF87UL, 0x00AFAFUL, 0x00AFD7UL, 0x00AFFFUL,
    0x00D700UL, 0x00D75FUL, 0x00D787UL, 0x00D7AFUL,
    0x00D7D7UL, 0x00D7FFUL, 0x00FF00UL, 0x00FF5FUL,
    0x00FF87UL, 0x00FFAFUL, 0x00FFD7UL, 0x00FFFFUL,
    0x5F0000UL, 0x5F005FUL, 0x5F0087UL, 0x5F00AFUL,
    0x5F00D7UL, 0x5F00FFUL, 0x5F5F00UL, 0x5F5F5FUL,
    0x5F5F87UL, 0x5F5FAFUL, 0x5F5FD7UL, 0x5F5FFFUL,
    0x5F8700UL, 0x5F875FUL, 0x5F8787UL, 0x5F87AFUL,
    0x5F87D7UL, 0x5F87FFUL, 0x5FAF00UL, 0x5FAF5FUL,
    0x5FAF87UL, 0x5FAFAFUL, 0x5FAFD7UL, 0x5FAFFFUL,
    0x5FD700UL, 0x5FD75FUL, 0x5FD787UL, 0x5FD7AFUL,
    0x5FD7D7UL, 0x5FD7FFUL, 0x5FFF00UL, 0x5FFF5FUL,
    0x5FFF87UL, 0x5FFFAFUL, 0x5FFFD7UL, 0x5FFFFFUL,
    0x870000UL, 0x87005FUL, 0x870087UL, 0x8700AFUL,
    0x8700D7UL, 0x8700FFUL, 0x875F00UL, 0x875F5FUL,
    0x875F87UL, 0x875FAFUL, 0x875FD7UL, 0x875FFFUL,
    0x878700UL, 0x87875FUL, 0x878787UL, 0x8787AFUL,
    0x8787D7UL, 0x8787FFUL, 0x87AF00UL, 0x87AF5FUL,
    0x87AF87UL, 0x87AFAFUL, 0x87AFD7UL, 0x87AFFFUL,
    0x87D700UL, 0x87D75FUL, 0x87D787UL, 0x87D7AFUL,
    0x87D7D7UL, 0x87D7FFUL, 0x87FF00UL, 0x87FF5FUL,
    0x87FF87UL, 0x87FFAFUL, 0x87FFD7UL, 0x87FFFFUL,
    0xAF0000UL, 0xAF005FUL, 0xAF0087UL, 0xAF00AFUL,
    0xAF00D7UL, 0xAF00FFUL, 0xAF5F00UL, 0xAF5F5FUL,
    0xAF5F87UL, 0xAF5FAFUL, 0xAF5FD7UL, 0xAF5FFFUL,
    0xAF8700UL, 0xAF875FUL, 0xAF8787UL, 0xAF87AFUL,
    0xAF87D7UL, 0xAF87FFUL, 0xAFAF00UL, 0xAFAF5FUL,
    0xAFAF87UL, 0xAFAFAFUL, 0xAFAFD7UL, 0xAFAFFFUL,
    0xAFD700UL, 0xAFD75FUL, 0xAFD787UL, 0xAFD7AFUL,
    0xAFD7D7UL, 0xAFD7FFUL, 0xAFFF00UL, 0xAFFF5FUL,
    0xAFFF87UL, 0xAFFFAFUL, 0xAFFFD7UL, 0xAFFFFFUL,
    0xD70000UL, 0xD7005FUL, 0xD70087UL, 0xD700AFUL,
    0xD700D7UL, 0xD700FFUL, 0xD75F00UL, 0xD75F5FUL,
    0xD75F87UL, 0xD75FAFUL, 0xD75FD7UL, 0xD75FFFUL,
    0xD78700UL, 0xD7875FUL, 0xD78787UL, 0xD787AFUL,
    0xD787D7UL, 0xD787FFUL, 0xD7AF00UL, 0xD7AF5FUL,
    0xD7AF87UL, 0xD7AFAFUL, 0xD7AFD7UL, 0xD7AFFFUL,
    0xD7D700UL, 0xD7D75FUL, 0xD7D787UL, 0xD7D7AFUL,
    0xD7D7D7UL, 0xD7D7FFUL, 0xD7FF00UL, 0xD7FF5FUL,
    0xD7FF87UL, 0xD7FFAFUL, 0xD7FFD7UL, 0xD7FFFFUL,
    0xFF0000UL, 0xFF005FUL, 0xFF0087UL, 0xFF00AFUL,
    0xFF00D7UL, 0xFF00FFUL, 0xFF5F00UL, 0xFF5F5FUL,
    0xFF5F87UL, 0xFF5FAFUL, 0xFF5FD7UL, 0xFF5FFFUL,
    0xFF8700UL, 0xFF875FUL, 0xFF8787UL, 0xFF87AFUL,
    0xFF87D7UL, 0xFF87FFUL, 0xFFAF00UL, 0xFFAF5FUL,
    0xFFAF87UL, 0xFFAFAFUL, 0xFFAFD7UL, 0xFFAFFFUL,
    0xFFD700UL, 0xFFD75FUL, 0xFFD787UL, 0xFFD7AFUL,
    0xFFD7D7UL, 0xFFD7FFUL, 0xFFFF00UL, 0xFFFF5FUL,
    0xFFFF87UL, 0xFFFFAFUL, 0xFFFFD7UL, 0xFFFFFFUL,
    /* 232-255: grayscale ramp */
    0x080808UL, 0x121212UL, 0x1C1C1CUL, 0x262626UL,
    0x303030UL, 0x3A3A3AUL, 0x444444UL, 0x4E4E4EUL,
    0x585858UL, 0x626262UL, 0x6C6C6CUL, 0x767676UL,
    0x808080UL, 0x8A8A8AUL, 0x949494UL, 0x9E9E9EUL,
    0xA8A8A8UL, 0xB2B2B2UL, 0xBCBCBCUL, 0xC6C6C6UL,
    0xD0D0D0UL, 0xDADADAUL, 0xE4E4E4UL, 0xEEEEEEUL
};

/* Precomputed mapping from 256-color index to nearest base-16 color.
   Indices 0-15 map to themselves (identity). */
static int color256_to_16[CLR_EXT_MAX];

/* Weighted Euclidean color distance metric.
   Algorithm from https://www.compuphase.com/cmetric.htm,
   via NetHack 3.7 coloratt.c. */
static int
color_distance(rgb1, rgb2)
unsigned long rgb1, rgb2;
{
    int r1 = (int)((rgb1 >> 16) & 0xFF);
    int g1 = (int)((rgb1 >> 8) & 0xFF);
    int b1 = (int)(rgb1 & 0xFF);
    int r2 = (int)((rgb2 >> 16) & 0xFF);
    int g2 = (int)((rgb2 >> 8) & 0xFF);
    int b2 = (int)(rgb2 & 0xFF);

    int rmean = (r1 + r2) / 2;
    int dr = r1 - r2;
    int dg = g1 - g2;
    int db = b1 - b2;

    return ((((512 + rmean) * dr * dr) >> 8) + 4 * dg * dg
            + (((767 - rmean) * db * db) >> 8));
}

/* Populate color256_to_16[] fallback table.  Called once at startup. */
void
init_extcolors()
{
    int i, j, best, dist, d;

    /* Identity mapping for base-16 colors */
    for (i = 0; i < CLR_MAX; i++)
        color256_to_16[i] = i;

    /* For each extended color, find nearest base-16 match */
    for (i = CLR_MAX; i < CLR_EXT_MAX; i++) {
        best = 0;
        dist = color_distance(color256_rgb[i], color256_rgb[0]);
        for (j = 1; j < CLR_MAX; j++) {
            d = color_distance(color256_rgb[i], color256_rgb[j]);
            if (d < dist) {
                dist = d;
                best = j;
            }
        }
        color256_to_16[i] = best;
    }
}

/* Map a 256-color index to the nearest base-16 color.
   Returns c unchanged for c < 16; NO_COLOR for out of range. */
int
map_color_256to16(c)
int c;
{
    if (c < CLR_MAX)
        return c;
    if (c < CLR_EXT_MAX)
        return color256_to_16[c];
    return NO_COLOR;
}

/* Return the RGB value for a 256-color index (for HTML dumplog CSS).
   Returns 0 for out-of-range indices. */
unsigned long
extcolor_256_rgb(c)
int c;
{
    if (c >= 0 && c < CLR_EXT_MAX)
        return color256_rgb[c];
    return 0UL;
}

/*extcolor.c*/
