/* NetHack 3.6  glyphs.c  $NHDT-Date: 2026/04/26 $  $NHDT-Branch: master $ */
/* Copyright (c) Keith Simpson, 2026. */
/* EvilHack may be freely redistributed.  See license for details. */

/* Customcolor registry: per-glyph color overrides delivered via the
   CUSTOMCOLOR= rc-file directive. Sparse linked list so glyphs without
   overrides cost nothing. Encoding follows NetHack 3.7's uint32 scheme:
   NH_BASIC_COLOR flag selects between a 16-color index (low byte) and a
   24-bit RGB value (low 24 bits). See doc/Guidebook for syntax */

#include "hack.h"

STATIC_VAR struct customcolor_entry *customcolor_list =
    (struct customcolor_entry *) 0;

STATIC_DCL int FDECL(monidx_by_name, (const char *));
STATIC_DCL int FDECL(objidx_by_name, (const char *));
STATIC_DCL int FDECL(set_cmap_customcolors, (const char *, unsigned long));
STATIC_DCL int FDECL(parse_clr_name, (const char *));
STATIC_DCL char *FDECL(strip_ws, (char *));
STATIC_DCL int FDECL(customcolor_glyph, (int));

/* CLR_* names accepted in CUSTOMCOLOR= color values */
static const struct {
    const char *name;
    int idx;
} clr_names[] = {
    { "black",          CLR_BLACK },
    { "red",            CLR_RED },
    { "green",          CLR_GREEN },
    { "brown",          CLR_BROWN },
    { "blue",           CLR_BLUE },
    { "magenta",        CLR_MAGENTA },
    { "cyan",           CLR_CYAN },
    { "gray",           CLR_GRAY },
    { "grey",           CLR_GRAY },
    { "no-color",       NO_COLOR },
    { "orange",         CLR_ORANGE },
    { "bright-green",   CLR_BRIGHT_GREEN },
    { "yellow",         CLR_YELLOW },
    { "bright-blue",    CLR_BRIGHT_BLUE },
    { "bright-magenta", CLR_BRIGHT_MAGENTA },
    { "bright-cyan",    CLR_BRIGHT_CYAN },
    { "white",          CLR_WHITE }
};

/* Free every customcolor entry. Called at rc-file (re)load */
void
reset_customcolors()
{
    struct customcolor_entry *cur, *nxt;

    for (cur = customcolor_list; cur; cur = nxt) {
        nxt = cur->next;
        free((genericptr_t) cur);
    }
    customcolor_list = (struct customcolor_entry *) 0;
}

/* Normalize a display glyph to the canonical glyph whose color drives
   its appearance, so a CUSTOMCOLOR= entry set on a base monster or object
   resolves for every display state. Pet/peaceful/ridden/detected monsters
   and corpses take a monster's mcolor, so they map to the base monster
   glyph; statues take the STATUE object color, so they map to the statue
   object glyph. Idempotent on the canonical keys that
   parse_customcolor_line() stores */
STATIC_OVL int
customcolor_glyph(glyph)
int glyph;
{
    if (glyph_is_monster(glyph))
        return monnum_to_glyph(glyph_to_mon(glyph));
    if (glyph_is_body(glyph))
        return monnum_to_glyph(glyph - GLYPH_BODY_OFF);
    if (glyph_is_statue(glyph))
        return objnum_to_glyph(STATUE);
    return glyph;
}

/* Look up a customcolor entry for a display glyph (normalized to its base
   monster/object). Returns NULL if none */
struct customcolor_entry *
customcolor_lookup(glyph)
int glyph;
{
    struct customcolor_entry *cur;

    glyph = customcolor_glyph(glyph);
    for (cur = customcolor_list; cur; cur = cur->next)
        if (cur->glyphidx == glyph)
            return cur;
    return (struct customcolor_entry *) 0;
}

/* Insert or replace a customcolor entry. Returns 1 on success */
int
set_customcolor(glyphidx, nhcolor)
int glyphidx;
unsigned long nhcolor;
{
    struct customcolor_entry *ce;
    unsigned long quant;
    int idx;

    ce = customcolor_lookup(glyphidx);
    if (!ce) {
        ce = (struct customcolor_entry *) alloc(
            (unsigned) sizeof (struct customcolor_entry));
        ce->glyphidx = glyphidx;
        ce->next = customcolor_list;
        customcolor_list = ce;
    }
    ce->nhcolor = nhcolor;
    /* Pre-compute the 256-palette fallback so windowports that need a
       quantized index (curses, 16-color terminals) don't redo the work
       on every cell. Basic colors keep the field valid too (the CLR_*
       index is its own quantized fallback) even though consumers read
       color256idx only on the RGB path */
    if (nhcolor & NH_BASIC_COLOR) {
        ce->color256idx = (int) (nhcolor & 0xFFUL);
    } else if (closest_color(COLORVAL(nhcolor), &quant, &idx)) {
        ce->color256idx = idx;
    } else {
        ce->color256idx = 0;
    }
    return 1;
}

/* Strip leading/trailing whitespace; modifies buf in place; returns
   pointer to first non-whitespace character */
STATIC_OVL char *
strip_ws(buf)
char *buf;
{
    char *cp;

    while (*buf == ' ' || *buf == '\t')
        buf++;
    cp = eos(buf);
    while (cp > buf && (cp[-1] == ' ' || cp[-1] == '\t')) {
        cp--;
        *cp = '\0';
    }
    return buf;
}

/* Look up a CLR_* index by lowercased name. Returns -1 on miss */
STATIC_OVL int
parse_clr_name(name)
const char *name;
{
    unsigned i;

    for (i = 0; i < SIZE(clr_names); i++)
        if (!strcmp(name, clr_names[i].name))
            return clr_names[i].idx;
    return -1;
}

/* Convert a color string ("#RRGGBB", "R-G-B", or a CLR_* name) into
   the 32-bit nhcolor encoding. Returns -1L on parse failure */
long
rgbstr_to_int32(s)
const char *s;
{
    char buf[BUFSZ];
    char *p;
    int r, g, b;
    int idx;
    char tail;

    if (!s || !*s)
        return -1L;
    /* "#RRGGBB" hex form */
    if (s[0] == '#') {
        unsigned long v = 0;
        const char *q = s + 1;
        int n = 0, d;

        while (*q && n < 6) {
            if (*q >= '0' && *q <= '9')
                d = *q - '0';
            else if (*q >= 'a' && *q <= 'f')
                d = 10 + (*q - 'a');
            else if (*q >= 'A' && *q <= 'F')
                d = 10 + (*q - 'A');
            else
                return -1L;
            v = (v << 4) | (unsigned long) d;
            q++;
            n++;
        }
        if (*q || n != 6)
            return -1L;
        return (long) v;
    }
    /* "R-G-B" decimal form */
    r = g = b = -1;
    if (sscanf(s, "%d-%d-%d%c", &r, &g, &b, &tail) == 3) {
        if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255)
            return -1L;
        return ((long) r << 16) | ((long) g << 8) | (long) b;
    }
    /* CLR_* color name (lowercased) */
    (void) strncpy(buf, s, sizeof buf - 1);
    buf[sizeof buf - 1] = '\0';
    for (p = buf; *p; p++)
        *p = lowc(*p);
    idx = parse_clr_name(buf);
    if (idx >= 0)
        return (long) (NH_BASIC_COLOR | (unsigned long) idx);
    return -1L;
}

/* Find a monster's glyph index by name (case-insensitive, via the
   existing name_to_mon helper). Returns -1 on miss */
STATIC_OVL int
monidx_by_name(name)
const char *name;
{
    int matchlen = 0;
    int mn = name_to_mon(name, &matchlen);

    if (mn == NON_PM)
        return -1;
    return mn + GLYPH_MON_OFF;
}

/* Find an object's glyph index by canonical name (case-insensitive,
   exact match against obj_descr[oc_name_idx].oc_name). Returns -1 */
STATIC_OVL int
objidx_by_name(name)
const char *name;
{
    int i, len = (int) strlen(name);

    for (i = 0; i < NUM_OBJECTS; i++) {
        const char *on = OBJ_NAME(objects[i]);

        if (on && (int) strlen(on) == len
            && !strncmpi(on, name, len))
            return i + GLYPH_OBJ_OFF;
    }
    return -1;
}

/* Set the customcolor on every cmap glyph whose explanation matches
   name (case-insensitive, exact). Terrain features commonly share an
   explanation, so a single cmap: directive colors the whole family.
   When no explanation matches, fall back to the map symbol name from
   loadsyms[] (the names used by SYMBOLS=), so the short names players
   know ("corr", "lava", ...) work too; that path targets a single
   glyph. Returns the count of glyphs set */
STATIC_OVL int
set_cmap_customcolors(name, nhcolor)
const char *name;
unsigned long nhcolor;
{
    int i, len = (int) strlen(name), count = 0;

    for (i = 0; i < MAXPCHARS; i++) {
        if (defsyms[i].explanation
            && (int) strlen(defsyms[i].explanation) == len
            && !strncmpi(defsyms[i].explanation, name, len)) {
            (void) set_customcolor(i + GLYPH_CMAP_OFF, nhcolor);
            count++;
        }
    }
    if (count == 0) {
        extern struct symparse loadsyms[]; /* drawing.c */
        struct symparse *sp;
        const char *sn;

        for (sp = loadsyms; sp->range; sp++) {
            if (sp->range != SYM_PCHAR)
                continue;
            /* accept the bare name without the "S_" prefix as well */
            sn = sp->name;
            if ((sn[0] == 'S' || sn[0] == 's') && sn[1] == '_')
                sn += 2;
            if (((int) strlen(sn) == len && !strncmpi(sn, name, len))
                || ((int) strlen(sp->name) == len
                    && !strncmpi(sp->name, name, len))) {
                (void) set_customcolor(sp->idx + GLYPH_CMAP_OFF, nhcolor);
                count++;
                break; /* symbol names are unique */
            }
        }
    }
    return count;
}

/* Parse a CUSTOMCOLOR= rc value of the form
       PREFIX:NAME/COLOR
   where PREFIX is one of mon|obj|cmap and COLOR is "#RRGGBB", an
   "R-G-B" decimal triple, or a CLR_* name. Returns 1 on success,
   0 on parse failure (caller emits a config error) */
int
parse_customcolor_line(str)
const char *str;
{
    char buf[BUFSZ];
    char *cp, *prefix, *target, *colorstr;
    int glyphidx = -1;
    long nhcolor;

    if (!str)
        return 0;
    (void) strncpy(buf, str, sizeof buf - 1);
    buf[sizeof buf - 1] = '\0';
    /* Split target/color on the '/' */
    cp = strchr(buf, '/');
    if (!cp)
        return 0;
    *cp++ = '\0';
    colorstr = strip_ws(cp);
    /* Split prefix:target on the ':' */
    cp = strchr(buf, ':');
    if (!cp)
        return 0;
    *cp++ = '\0';
    prefix = strip_ws(buf);
    target = strip_ws(cp);
    if (!*prefix || !*target || !*colorstr)
        return 0;
    /* Parse the color first so cmap: can apply it to every match */
    nhcolor = rgbstr_to_int32(colorstr);
    if (nhcolor < 0L)
        return 0;
    /* Resolve the prefix + target name and store the override */
    if (!strcmp(prefix, "mon")) {
        glyphidx = monidx_by_name(target);
    } else if (!strcmp(prefix, "obj")) {
        glyphidx = objidx_by_name(target);
        if (glyphidx >= 0 && glyphidx - GLYPH_OBJ_OFF == CORPSE) {
            config_error_add(
                "CUSTOMCOLOR obj:corpse has no effect; use mon:<name>");
            return 0;
        }
    } else if (!strcmp(prefix, "cmap")) {
        return set_cmap_customcolors(target, (unsigned long) nhcolor) > 0;
    } else {
        return 0;
    }
    if (glyphidx < 0)
        return 0;
    return set_customcolor(glyphidx, (unsigned long) nhcolor);
}

/*glyphs.c*/
