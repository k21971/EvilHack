/* EvilHack unicode.c - UTF-8 encoding and CP437/DEC to Unicode translation */
/* Copyright (c) Patric Mueller (UnNetHack), 2011. */
/* Adapted for EvilHack by Keith Simpson, 2026. */
/* EvilHack may be freely redistributed.  See license for details. */

#include "hack.h"

#ifdef UNIX
#include <langinfo.h>
#endif

/*
 * CP437 to Unicode mapping for bytes 0x80-0xFF.
 * Indexed by (ch - 0x80).
 * Source: Unicode Consortium
 * http://unicode.org/Public/MAPPINGS/VENDORS/MICSFT/PC/CP437.TXT
 */
static const int cp437_to_unicode[128] = {
    0x00C7, /* 0x80 LATIN CAPITAL LETTER C WITH CEDILLA */
    0x00FC, /* 0x81 LATIN SMALL LETTER U WITH DIAERESIS */
    0x00E9, /* 0x82 LATIN SMALL LETTER E WITH ACUTE */
    0x00E2, /* 0x83 LATIN SMALL LETTER A WITH CIRCUMFLEX */
    0x00E4, /* 0x84 LATIN SMALL LETTER A WITH DIAERESIS */
    0x00E0, /* 0x85 LATIN SMALL LETTER A WITH GRAVE */
    0x00E5, /* 0x86 LATIN SMALL LETTER A WITH RING ABOVE */
    0x00E7, /* 0x87 LATIN SMALL LETTER C WITH CEDILLA */
    0x00EA, /* 0x88 LATIN SMALL LETTER E WITH CIRCUMFLEX */
    0x00EB, /* 0x89 LATIN SMALL LETTER E WITH DIAERESIS */
    0x00E8, /* 0x8A LATIN SMALL LETTER E WITH GRAVE */
    0x00EF, /* 0x8B LATIN SMALL LETTER I WITH DIAERESIS */
    0x00EE, /* 0x8C LATIN SMALL LETTER I WITH CIRCUMFLEX */
    0x00EC, /* 0x8D LATIN SMALL LETTER I WITH GRAVE */
    0x00C4, /* 0x8E LATIN CAPITAL LETTER A WITH DIAERESIS */
    0x00C5, /* 0x8F LATIN CAPITAL LETTER A WITH RING ABOVE */
    0x00C9, /* 0x90 LATIN CAPITAL LETTER E WITH ACUTE */
    0x00E6, /* 0x91 LATIN SMALL LIGATURE AE */
    0x00C6, /* 0x92 LATIN CAPITAL LIGATURE AE */
    0x00F4, /* 0x93 LATIN SMALL LETTER O WITH CIRCUMFLEX */
    0x00F6, /* 0x94 LATIN SMALL LETTER O WITH DIAERESIS */
    0x00F2, /* 0x95 LATIN SMALL LETTER O WITH GRAVE */
    0x00FB, /* 0x96 LATIN SMALL LETTER U WITH CIRCUMFLEX */
    0x00F9, /* 0x97 LATIN SMALL LETTER U WITH GRAVE */
    0x00FF, /* 0x98 LATIN SMALL LETTER Y WITH DIAERESIS */
    0x00D6, /* 0x99 LATIN CAPITAL LETTER O WITH DIAERESIS */
    0x00DC, /* 0x9A LATIN CAPITAL LETTER U WITH DIAERESIS */
    0x00A2, /* 0x9B CENT SIGN */
    0x00A3, /* 0x9C POUND SIGN */
    0x00A5, /* 0x9D YEN SIGN */
    0x20A7, /* 0x9E PESETA SIGN */
    0x0192, /* 0x9F LATIN SMALL LETTER F WITH HOOK */
    0x00E1, /* 0xA0 LATIN SMALL LETTER A WITH ACUTE */
    0x00ED, /* 0xA1 LATIN SMALL LETTER I WITH ACUTE */
    0x00F3, /* 0xA2 LATIN SMALL LETTER O WITH ACUTE */
    0x00FA, /* 0xA3 LATIN SMALL LETTER U WITH ACUTE */
    0x00F1, /* 0xA4 LATIN SMALL LETTER N WITH TILDE */
    0x00D1, /* 0xA5 LATIN CAPITAL LETTER N WITH TILDE */
    0x00AA, /* 0xA6 FEMININE ORDINAL INDICATOR */
    0x00BA, /* 0xA7 MASCULINE ORDINAL INDICATOR */
    0x00BF, /* 0xA8 INVERTED QUESTION MARK */
    0x2310, /* 0xA9 REVERSED NOT SIGN */
    0x00AC, /* 0xAA NOT SIGN */
    0x00BD, /* 0xAB VULGAR FRACTION ONE HALF */
    0x00BC, /* 0xAC VULGAR FRACTION ONE QUARTER */
    0x00A1, /* 0xAD INVERTED EXCLAMATION MARK */
    0x00AB, /* 0xAE LEFT-POINTING DOUBLE ANGLE QUOTATION MARK */
    0x00BB, /* 0xAF RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */
    0x2591, /* 0xB0 LIGHT SHADE */
    0x2592, /* 0xB1 MEDIUM SHADE */
    0x2593, /* 0xB2 DARK SHADE */
    0x2502, /* 0xB3 BOX DRAWINGS LIGHT VERTICAL */
    0x2524, /* 0xB4 BOX DRAWINGS LIGHT VERTICAL AND LEFT */
    0x2561, /* 0xB5 BOX DRAWINGS VERTICAL SINGLE AND LEFT DOUBLE */
    0x2562, /* 0xB6 BOX DRAWINGS VERTICAL DOUBLE AND LEFT SINGLE */
    0x2556, /* 0xB7 BOX DRAWINGS DOWN DOUBLE AND LEFT SINGLE */
    0x2555, /* 0xB8 BOX DRAWINGS DOWN SINGLE AND LEFT DOUBLE */
    0x2563, /* 0xB9 BOX DRAWINGS DOUBLE VERTICAL AND LEFT */
    0x2551, /* 0xBA BOX DRAWINGS DOUBLE VERTICAL */
    0x2557, /* 0xBB BOX DRAWINGS DOUBLE DOWN AND LEFT */
    0x255D, /* 0xBC BOX DRAWINGS DOUBLE UP AND LEFT */
    0x255C, /* 0xBD BOX DRAWINGS UP DOUBLE AND LEFT SINGLE */
    0x255B, /* 0xBE BOX DRAWINGS UP SINGLE AND LEFT DOUBLE */
    0x2510, /* 0xBF BOX DRAWINGS LIGHT DOWN AND LEFT */
    0x2514, /* 0xC0 BOX DRAWINGS LIGHT UP AND RIGHT */
    0x2534, /* 0xC1 BOX DRAWINGS LIGHT UP AND HORIZONTAL */
    0x252C, /* 0xC2 BOX DRAWINGS LIGHT DOWN AND HORIZONTAL */
    0x251C, /* 0xC3 BOX DRAWINGS LIGHT VERTICAL AND RIGHT */
    0x2500, /* 0xC4 BOX DRAWINGS LIGHT HORIZONTAL */
    0x253C, /* 0xC5 BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL */
    0x255E, /* 0xC6 BOX DRAWINGS VERTICAL SINGLE AND RIGHT DOUBLE */
    0x255F, /* 0xC7 BOX DRAWINGS VERTICAL DOUBLE AND RIGHT SINGLE */
    0x255A, /* 0xC8 BOX DRAWINGS DOUBLE UP AND RIGHT */
    0x2554, /* 0xC9 BOX DRAWINGS DOUBLE DOWN AND RIGHT */
    0x2569, /* 0xCA BOX DRAWINGS DOUBLE UP AND HORIZONTAL */
    0x2566, /* 0xCB BOX DRAWINGS DOUBLE DOWN AND HORIZONTAL */
    0x2560, /* 0xCC BOX DRAWINGS DOUBLE VERTICAL AND RIGHT */
    0x2550, /* 0xCD BOX DRAWINGS DOUBLE HORIZONTAL */
    0x256C, /* 0xCE BOX DRAWINGS DOUBLE VERTICAL AND HORIZONTAL */
    0x2567, /* 0xCF BOX DRAWINGS UP SINGLE AND HORIZONTAL DOUBLE */
    0x2568, /* 0xD0 BOX DRAWINGS UP DOUBLE AND HORIZONTAL SINGLE */
    0x2564, /* 0xD1 BOX DRAWINGS DOWN SINGLE AND HORIZONTAL DOUBLE */
    0x2565, /* 0xD2 BOX DRAWINGS DOWN DOUBLE AND HORIZONTAL SINGLE */
    0x2559, /* 0xD3 BOX DRAWINGS UP DOUBLE AND RIGHT SINGLE */
    0x2558, /* 0xD4 BOX DRAWINGS UP SINGLE AND RIGHT DOUBLE */
    0x2552, /* 0xD5 BOX DRAWINGS DOWN SINGLE AND RIGHT DOUBLE */
    0x2553, /* 0xD6 BOX DRAWINGS DOWN DOUBLE AND RIGHT SINGLE */
    0x256B, /* 0xD7 BOX DRAWINGS VERTICAL DOUBLE AND HORIZONTAL SINGLE */
    0x256A, /* 0xD8 BOX DRAWINGS VERTICAL SINGLE AND HORIZONTAL DOUBLE */
    0x2518, /* 0xD9 BOX DRAWINGS LIGHT UP AND LEFT */
    0x250C, /* 0xDA BOX DRAWINGS LIGHT DOWN AND RIGHT */
    0x2588, /* 0xDB FULL BLOCK */
    0x2584, /* 0xDC LOWER HALF BLOCK */
    0x258C, /* 0xDD LEFT HALF BLOCK */
    0x2590, /* 0xDE RIGHT HALF BLOCK */
    0x2580, /* 0xDF UPPER HALF BLOCK */
    0x03B1, /* 0xE0 GREEK SMALL LETTER ALPHA */
    0x00DF, /* 0xE1 LATIN SMALL LETTER SHARP S */
    0x0393, /* 0xE2 GREEK CAPITAL LETTER GAMMA */
    0x03C0, /* 0xE3 GREEK SMALL LETTER PI */
    0x03A3, /* 0xE4 GREEK CAPITAL LETTER SIGMA */
    0x03C3, /* 0xE5 GREEK SMALL LETTER SIGMA */
    0x00B5, /* 0xE6 MICRO SIGN */
    0x03C4, /* 0xE7 GREEK SMALL LETTER TAU */
    0x03A6, /* 0xE8 GREEK CAPITAL LETTER PHI */
    0x0398, /* 0xE9 GREEK CAPITAL LETTER THETA */
    0x03A9, /* 0xEA GREEK CAPITAL LETTER OMEGA */
    0x03B4, /* 0xEB GREEK SMALL LETTER DELTA */
    0x221E, /* 0xEC INFINITY */
    0x03C6, /* 0xED GREEK SMALL LETTER PHI */
    0x03B5, /* 0xEE GREEK SMALL LETTER EPSILON */
    0x2229, /* 0xEF INTERSECTION */
    0x2261, /* 0xF0 IDENTICAL TO */
    0x00B1, /* 0xF1 PLUS-MINUS SIGN */
    0x2265, /* 0xF2 GREATER-THAN OR EQUAL TO */
    0x2264, /* 0xF3 LESS-THAN OR EQUAL TO */
    0x2320, /* 0xF4 TOP HALF INTEGRAL */
    0x2321, /* 0xF5 BOTTOM HALF INTEGRAL */
    0x00F7, /* 0xF6 DIVISION SIGN */
    0x2248, /* 0xF7 ALMOST EQUAL TO */
    0x00B0, /* 0xF8 DEGREE SIGN */
    0x2219, /* 0xF9 BULLET OPERATOR */
    0x00B7, /* 0xFA MIDDLE DOT */
    0x221A, /* 0xFB SQUARE ROOT */
    0x207F, /* 0xFC SUPERSCRIPT LATIN SMALL LETTER N */
    0x00B2, /* 0xFD SUPERSCRIPT TWO */
    0x25A0, /* 0xFE BLACK SQUARE */
    0x00A0, /* 0xFF NO-BREAK SPACE */
};

/*
 * DEC Special Graphic Character Set (VT100 graphics) to Unicode.
 * Only the last 32 characters (0x60-0x7F) differ from ASCII;
 * we store them indexed by (ch & 0x1F) after stripping the 0xE0 encoding
 * that NetHack uses (DECgraphics chars have 0x80 | lowercase).
 */
static const int dec_to_unicode[32] = {
    0x25C6, /* 0x60 BLACK DIAMOND */
    0x2592, /* 0x61 MEDIUM SHADE */
    0x0009, /* 0x62 CHARACTER TABULATION */
    0x000C, /* 0x63 FORM FEED */
    0x000D, /* 0x64 CARRIAGE RETURN */
    0x000A, /* 0x65 LINE FEED */
    0x00B0, /* 0x66 DEGREE SIGN */
    0x00B1, /* 0x67 PLUS-MINUS SIGN */
    0x000A, /* 0x68 NEW LINE */
    0x000B, /* 0x69 LINE TABULATION */
    0x2518, /* 0x6A BOX DRAWINGS LIGHT UP AND LEFT */
    0x2510, /* 0x6B BOX DRAWINGS LIGHT DOWN AND LEFT */
    0x250C, /* 0x6C BOX DRAWINGS LIGHT DOWN AND RIGHT */
    0x2514, /* 0x6D BOX DRAWINGS LIGHT UP AND RIGHT */
    0x253C, /* 0x6E BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL */
    0x00A0, /* 0x6F NO-BREAK SPACE */
    0x00A0, /* 0x70 NO-BREAK SPACE */
    0x2500, /* 0x71 BOX DRAWINGS LIGHT HORIZONTAL */
    0x00A0, /* 0x72 NO-BREAK SPACE */
    0x00A0, /* 0x73 NO-BREAK SPACE */
    0x251C, /* 0x74 BOX DRAWINGS LIGHT VERTICAL AND RIGHT */
    0x2524, /* 0x75 BOX DRAWINGS LIGHT VERTICAL AND LEFT */
    0x2534, /* 0x76 BOX DRAWINGS LIGHT UP AND HORIZONTAL */
    0x252C, /* 0x77 BOX DRAWINGS LIGHT DOWN AND HORIZONTAL */
    0x2502, /* 0x78 BOX DRAWINGS LIGHT VERTICAL */
    0x2264, /* 0x79 LESS-THAN OR EQUAL TO */
    0x2265, /* 0x7A GREATER-THAN OR EQUAL TO */
    0x03C0, /* 0x7B GREEK SMALL LETTER PI */
    0x2260, /* 0x7C NOT EQUAL TO */
    0x00A3, /* 0x7D POUND SIGN */
    0x00B7, /* 0x7E MIDDLE DOT */
    0x00A0, /* 0x7F NO-BREAK SPACE */
};

/*
 * Translate a legacy character to a Unicode codepoint.
 * For IBM (CP437) graphics: chars 0x80-0xFF are looked up in cp437 table.
 * For DEC graphics: chars with high bit set (0xE0-0xFF encoding) are
 *   translated via the DEC table.
 * For UTF8 handling or plain ASCII: returned as-is.
 */
int
get_unicode_codepoint(ch)
int ch;
{
    if (SYMHANDLING(H_IBM) && ch >= 0x80 && ch <= 0xFF) {
        return cp437_to_unicode[ch - 0x80];
    } else if (SYMHANDLING(H_DEC) && ch >= 0xE0 && ch <= 0xFF) {
        return dec_to_unicode[ch - 0xE0];
    }
    return ch;
}

/*
 * Encode a Unicode codepoint as a UTF-8 byte sequence via putchar().
 * Originally from Ray Chason's Unicode proof of concept patch
 * via UnNetHack.
 */
void
pututf8char(c)
int c;
{
    if (c < 0x80) {
        (void) putchar(c);
    } else if (c < 0x800) {
        (void) putchar(0xC0 | (c >> 6));
        (void) putchar(0x80 | (c & 0x3F));
    } else if (c < 0x10000) {
        (void) putchar(0xE0 | (c >> 12));
        (void) putchar(0x80 | ((c >> 6) & 0x3F));
        (void) putchar(0x80 | (c & 0x3F));
    } else if (c < 0x200000) {
        (void) putchar(0xF0 | (c >> 18));
        (void) putchar(0x80 | ((c >> 12) & 0x3F));
        (void) putchar(0x80 | ((c >> 6) & 0x3F));
        (void) putchar(0x80 | (c & 0x3F));
    }
}

/*
 * Encode a Unicode codepoint as a UTF-8 byte sequence into buf[].
 * Returns the number of bytes written (1-4).
 * buf must have room for at least 5 bytes (4 + NUL).
 */
int
utf8str_from_codepoint(c, buf)
int c;
char *buf;
{
    if (c < 0x80) {
        buf[0] = (char) c;
        buf[1] = '\0';
        return 1;
    } else if (c < 0x800) {
        buf[0] = (char) (0xC0 | (c >> 6));
        buf[1] = (char) (0x80 | (c & 0x3F));
        buf[2] = '\0';
        return 2;
    } else if (c < 0x10000) {
        buf[0] = (char) (0xE0 | (c >> 12));
        buf[1] = (char) (0x80 | ((c >> 6) & 0x3F));
        buf[2] = (char) (0x80 | (c & 0x3F));
        buf[3] = '\0';
        return 3;
    } else if (c < 0x200000) {
        buf[0] = (char) (0xF0 | (c >> 18));
        buf[1] = (char) (0x80 | ((c >> 12) & 0x3F));
        buf[2] = (char) (0x80 | ((c >> 6) & 0x3F));
        buf[3] = (char) (0x80 | (c & 0x3F));
        buf[4] = '\0';
        return 4;
    }
    buf[0] = '?';
    buf[1] = '\0';
    return 1;
}

/*
 * Detect whether the terminal supports UTF-8 encoding.
 * Uses nl_langinfo(CODESET) on Unix systems.
 */
boolean
detect_utf8_terminal()
{
#ifdef UNIX
    const char *charset = nl_langinfo(CODESET);

    if (charset
        && (!strcmpi(charset, "UTF-8") || !strcmpi(charset, "UTF8")))
        return TRUE;
#endif
    return FALSE;
}

/*unicode.c*/
