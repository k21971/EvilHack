/* NetHack 3.6	mapglyph.c	$NHDT-Date: 1573943501 2019/11/16 22:31:41 $  $NHDT-Branch: NetHack-3.6 $:$NHDT-Revision: 1.51 $ */
/* Copyright (c) David Cohrs, 1991                                */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#if defined(TTY_GRAPHICS)
#include "wintty.h" /* for prototype of has_color() only */
#endif
#include "color.h"
#define HI_DOMESTIC CLR_WHITE /* monst.c */

#if !defined(TTY_GRAPHICS)
#define has_color(n) TRUE
#endif

#ifdef TEXTCOLOR
static const int explcolors[] = {
    CLR_BLACK,         /* dark    */
    CLR_BRIGHT_GREEN,  /* noxious */
    CLR_BROWN,         /* muddy   */
    CLR_BLUE,          /* wet     */
    CLR_MAGENTA,       /* magical */
    CLR_ORANGE,        /* fiery   */
    CLR_BRIGHT_CYAN,   /* frosty  */
    CLR_YELLOW,        /* acid    */
    CLR_WHITE,         /* shock   */
};

#define zap_color(n) color = iflags.use_color ? zapcolors[n] : NO_COLOR

/* Base-16 fallback colors for defsyms[] entries that use 256-color.
   NO_COLOR means the defsyms color is already base-16. */
static const int cmap_color16[] = {
/* 0  stone    */ NO_COLOR,
/* 1  vwall    */ NO_COLOR,
/* 2  hwall    */ NO_COLOR,
/* 3  tlcorn   */ NO_COLOR,
/* 4  trcorn   */ NO_COLOR,
/* 5  blcorn   */ NO_COLOR,
/* 6  brcorn   */ NO_COLOR,
/* 7  crwall   */ NO_COLOR,
/* 8  tuwall   */ NO_COLOR,
/* 9  tdwall   */ NO_COLOR,
/*10  tlwall   */ NO_COLOR,
/*11  trwall   */ NO_COLOR,
/*12  ndoor    */ NO_COLOR,
/*13  vodoor   */ CLR_BROWN,        /* HI_WOOD (94) */
/*14  hodoor   */ CLR_BROWN,        /* HI_WOOD (94) */
/*15  vcdoor   */ CLR_BROWN,        /* HI_WOOD (94) */
/*16  hcdoor   */ CLR_BROWN,        /* HI_WOOD (94) */
/*17  bars     */ NO_COLOR,         /* HI_METAL = CLR_CYAN */
/*18  tree     */ CLR_GREEN,        /* 28 */
/*19  deadtree */ NO_COLOR,
/*20  room     */ NO_COLOR,
/*21  darkroom */ NO_COLOR,
/*22  corr     */ NO_COLOR,
/*23  litcorr  */ NO_COLOR,
/*24  upstair  */ NO_COLOR,
/*25  dnstair  */ NO_COLOR,
/*26  upladder */ NO_COLOR,
/*27  dnladder */ NO_COLOR,
/*28  altar    */ NO_COLOR,
/*29  grave    */ NO_COLOR,
/*30  throne   */ CLR_YELLOW,       /* HI_GOLD (220) */
/*31  sink     */ NO_COLOR,
/*32  forge    */ CLR_ORANGE,       /* 202 */
/*33  mchest   */ NO_COLOR,
/*34  fountain */ NO_COLOR,
/*35  pool     */ CLR_BLUE,         /* 21 */
/*36  ice      */ CLR_CYAN,         /* 159 */
/*37  grass    */ CLR_BRIGHT_GREEN, /* 118 */
/*38  sand     */ CLR_YELLOW,       /* 227 */
/*39  lava     */ CLR_RED,          /* 88 */
/*40  vodbridge*/ CLR_BROWN,        /* HI_WOOD (94) */
/*41  hodbridge*/ CLR_BROWN,        /* HI_WOOD (94) */
/*42  vcdbridge*/ CLR_BROWN,        /* HI_WOOD (94) */
/*43  hcdbridge*/ CLR_BROWN,        /* HI_WOOD (94) */
/*44  air      */ NO_COLOR,
/*45  cloud    */ NO_COLOR,
/*46  puddle   */ CLR_CYAN,         /* 51 */
/*47  sewage   */ CLR_GREEN,        /* 100 */
/*48  water    */ CLR_BLUE,         /* 21 */
/* traps 49-67: all base-16, no fallback needed */
/*49*/ NO_COLOR, /*50*/ NO_COLOR, /*51*/ NO_COLOR,
/*52*/ NO_COLOR, /*53*/ NO_COLOR, /*54*/ NO_COLOR,
/*55*/ NO_COLOR, /*56*/ NO_COLOR, /*57*/ NO_COLOR,
/*58*/ NO_COLOR, /*59*/ NO_COLOR, /*60*/ NO_COLOR,
/*61*/ NO_COLOR, /*62*/ NO_COLOR, /*63*/ NO_COLOR,
/*64*/ NO_COLOR, /*65*/ NO_COLOR, /*66*/ NO_COLOR,
/*67*/ NO_COLOR,
/*68  web      */ CLR_GRAY,         /* HI_SPIDER_SILK (253) */
};

/* Base-16 fallback for extended 256-color HI_* material colors used
   in objects[].oc_color.  When the terminal can't render 256-color
   (use_256color off), this returns the original intended game color
   instead of the generic map_color_256to16() RGB-distance result. */
int
obj_color_fallback(color)
int color;
{
    switch (color) {
    case HI_WAX:
        return CLR_WHITE;
    case HI_VEGGY:
        return CLR_GREEN;
    case HI_FLESH:
    case HI_CLOTH:
    case HI_WOOD:
        return CLR_BROWN;
    case HI_SPIDER_SILK:
    case HI_SILVER:
    case HI_MITHRIL:
    case HI_MINERAL:
        return CLR_GRAY;
    case HI_BONE:
    case HI_PLATINUM:
        return CLR_WHITE;
    case HI_DRAGON_HIDE:
    case HI_ADAMANTINE:
        return CLR_BLACK;
    case HI_STEEL:
        return CLR_CYAN;
    case HI_COPPER:
    case HI_BRONZE:
        return CLR_ORANGE;
    case HI_GOLD:
        return CLR_YELLOW;
    case HI_PLASTIC:
    case HI_GLASS:
        return CLR_BRIGHT_CYAN;
    case HI_GEMSTONE:
        return CLR_RED;
    default:
        return map_color_256to16(color);
    }
}

/* Base-16 fallback for extended 256-color values used in mons[].mcolor.
   Handles monster-specific colors (HI_PURPLE, HI_ORANGE, etc.) and
   material colors (HI_CLOTH, HI_MINERAL, etc.) reused for monsters.
   Some monsters use material colors whose obj fallback doesn't match
   the monster's original base-16 color; those are overridden by
   monster index before falling through to the color-based switch. */
static int
mon_color_fallback(mndx, color)
int mndx;
int color;
{
    /* Per-monster overrides for material colors whose obj fallback
       doesn't match the monster's original base-16 color */
    switch (mndx) {
    case PM_HAWK:
    case PM_LARGE_HAWK:
    case PM_GIANT_HAWK:
        return CLR_YELLOW;      /* HI_CLOTH obj fallback is CLR_BROWN */
    case PM_WEREJACKAL:
    case PM_CLAY_GOLEM:
        return CLR_BROWN;       /* HI_COPPER obj fallback is CLR_ORANGE */
    case PM_FLESH_GOLEM:
        return CLR_RED;         /* HI_FLESH obj fallback is CLR_BROWN */
    case PM_GLASS_PIERCER:
        return CLR_WHITE;       /* HI_GLASS obj fallback is CLR_BRIGHT_CYAN */
    case PM_GLASS_GOLEM:
        return CLR_CYAN;        /* HI_GLASS obj fallback is CLR_BRIGHT_CYAN */
    case PM_BONE_DEVIL:
        return CLR_GRAY;        /* HI_BONE obj fallback is CLR_WHITE */
    default:
        break;
    }

    /* Color-based fallback for monster-specific and material colors */
    switch (color) {
    case HI_MPLAYER:
        return CLR_YELLOW;
    case HI_ORANGE:
        return CLR_ORANGE;
    case HI_PURPLE:
        return CLR_MAGENTA;
    case HI_TREE:
    case HI_PLANT:
        return CLR_BRIGHT_GREEN;
    case HI_WATER:
        return CLR_BLUE;
    default:
        /* Material colors (HI_CLOTH, HI_MINERAL, etc.) */
        return obj_color_fallback(color);
    }
}

#define cmap_color(n)                                                  \
    do {                                                               \
        if (!iflags.use_color) {                                       \
            color = NO_COLOR;                                          \
        } else {                                                       \
            int _cc = defsyms[n].color;                                \
            if (IS_EXT_COLOR(_cc) && !has_color(_cc)                   \
                    && (n) < SIZE(cmap_color16)                        \
                    && cmap_color16[n] != NO_COLOR)                    \
                color = cmap_color16[n];                               \
            else                                                       \
                color = _cc;                                           \
        }                                                              \
    } while (0)
#define obj_color(n) color = iflags.use_color ? objects[n].oc_color : NO_COLOR
#if 0
/* used to allow racial glyphs to retain the color of their base type,
 * e.g. make orcish priests white instead of red */
#define rmon_color(n, x, y) \
{                                               \
  struct monst *mtmp = m_at(x, y);              \
  if (mtmp && has_erac(mtmp) && !Hallucination) \
      mon_color(monsndx(mtmp->data));           \
  else                                          \
      mon_color(n);                             \
}
#endif
#define mon_color(n)                                               \
    do {                                                               \
        color = iflags.use_color ? mons[n].mcolor : NO_COLOR;         \
        if (IS_EXT_COLOR(color) && !has_color(color))                  \
            color = mon_color_fallback(n, color);                      \
    } while (0)
#define invis_color(n) color = NO_COLOR
#define pet_color(n)                                                   \
    do {                                                               \
        color = iflags.use_color ? mons[n].mcolor : NO_COLOR;         \
        if (IS_EXT_COLOR(color) && !has_color(color))                  \
            color = mon_color_fallback(n, color);                      \
    } while (0)
#define warn_color(n) \
    color = iflags.use_color ? def_warnsyms[n].color : NO_COLOR
#define explode_color(n) color = iflags.use_color ? explcolors[n] : NO_COLOR
#define sokoban_prize_color() color = iflags.use_color ? CLR_BRIGHT_GREEN : NO_COLOR

#else /* no text color */

#define zap_color(n)
#define cmap_color(n)
#define obj_color(n)
/* #define rmon_color(n, x, y) */
#define mon_color(n)
#define invis_color(n)
#define pet_color(c)
#define warn_color(n)
#define explode_color(n)
#define sokoban_prize_color()
#endif

#if defined(USE_TILES) && defined(MSDOS)
#define HAS_ROGUE_IBM_GRAPHICS \
    (currentgraphics == ROGUESET && SYMHANDLING(H_IBM) && !iflags.grmode)
#else
#define HAS_ROGUE_IBM_GRAPHICS \
    (currentgraphics == ROGUESET && SYMHANDLING(H_IBM))
#endif

#define is_objpile(x,y) (!Hallucination && level.objects[(x)][(y)] \
                         && level.objects[(x)][(y)]->nexthere)

/*ARGSUSED*/
int
mapglyph(glyph, ochar, ocolor, ospecial, x, y, mgflags)
int glyph, *ocolor, x, y;
int *ochar;
unsigned *ospecial;
unsigned mgflags;
{
    int offset, idx;
    int color = NO_COLOR;
    nhsym ch;
    unsigned special = 0;
    /* condense multiple tests in macro version down to single */
    boolean has_rogue_ibm_graphics = HAS_ROGUE_IBM_GRAPHICS,
            is_you = (x == u.ux && y == u.uy),
            has_rogue_color = (has_rogue_ibm_graphics
                               && symset[currentgraphics].nocolor == 0);

    /*
     *  Map the glyph back to a character and color.
     *
     *  Warning:  For speed, this makes an assumption on the order of
     *            offsets.  The order is set in display.h.
     */
    if ((offset = (glyph - GLYPH_STATUE_OFF)) >= 0) { /* a statue */
        idx = mons[offset].mlet + SYM_OFF_M;
        if (has_rogue_color)
            color = CLR_RED;
        else
            obj_color(STATUE);
        special |= MG_STATUE;
        if (is_objpile(x,y))
            special |= MG_OBJPILE;
        if (On_stairs(x,y))
            special |= MG_STAIRS;
    } else if ((offset = (glyph - GLYPH_WARNING_OFF)) >= 0) { /* warn flash */
        idx = offset + SYM_OFF_W;
        if (has_rogue_color)
            color = NO_COLOR;
        else
            warn_color(offset);
    } else if ((offset = (glyph - GLYPH_SWALLOW_OFF)) >= 0) { /* swallow */
        /* see swallow_to_glyph() in display.c */
        idx = (S_sw_tl + (offset & 0x7)) + SYM_OFF_P;
        if (has_rogue_color && iflags.use_color)
            color = NO_COLOR;
        else
            mon_color(offset >> 3);
    } else if ((offset = (glyph - GLYPH_ZAP_OFF)) >= 0) { /* zap beam */
        /* see zapdir_to_glyph() in display.c */
        idx = (S_vbeam + (offset & 0x3)) + SYM_OFF_P;
        if (has_rogue_color && iflags.use_color)
            color = NO_COLOR;
        else
            zap_color((offset >> 2));
    } else if ((offset = (glyph - GLYPH_EXPLODE_OFF)) >= 0) { /* explosion */
        idx = ((offset % MAXEXPCHARS) + S_explode1) + SYM_OFF_P;
        explode_color(offset / MAXEXPCHARS);
    } else if ((offset = (glyph - GLYPH_CMAP_OFF)) >= 0) { /* cmap */
        idx = offset + SYM_OFF_P;
        if (has_rogue_color && iflags.use_color) {
            if (offset >= S_vwall && offset <= S_hcdoor)
                color = CLR_BROWN;
            else if (offset >= S_arrow_trap && offset <= S_magic_beam_trap)
                color = CLR_MAGENTA;
            else if (offset == S_corr || offset == S_litcorr)
                color = CLR_GRAY;
            else if (offset >= S_room && offset <= S_water
                     && offset != S_darkroom)
                color = CLR_GREEN;
            else
                color = NO_COLOR;
#ifdef TEXTCOLOR
        /* provide a visible difference if normal and lit corridor
           use the same symbol */
        } else if (iflags.use_color && offset == S_litcorr
                   && showsyms[idx] == showsyms[S_corr + SYM_OFF_P]) {
            color = CLR_WHITE;
        /* show branch stairs in a different color */
        } else if (iflags.use_color
                   && (offset == S_upstair || offset == S_dnstair)
                   && (x == sstairs.sx && y == sstairs.sy)) {
            color = CLR_YELLOW;
        /* Colored Walls and Floors Patch */
        } else if (iflags.use_color && offset >= S_vwall && offset <= S_trwall) {
            if (In_W_tower(x, y, &u.uz))
                color = CLR_MAGENTA;
            else if (In_tower(&u.uz)) /* Vlad's */
                color = has_color(166) ? 166 : CLR_ORANGE; /* rust */
            else if (In_sokoban(&u.uz))
                color = CLR_CYAN;
            else if (Is_valley(&u.uz))
                color = CLR_BLACK;
            else if (In_hell(&u.uz))
                color = has_color(52) ? 52 : CLR_RED; /* dark maroon */
            else if (In_mines(&u.uz)) /* no in_rooms check */
                color = CLR_BROWN;
            else if (Is_astralevel(&u.uz))
                color = CLR_WHITE;
            else if (getroomtype(x, y) == DELPHI)
                color = CLR_BRIGHT_BLUE;
            else if (getroomtype(x, y) == BEEHIVE)
                color = CLR_YELLOW;
            else if (getroomtype(x, y) == GARDEN)
                color = CLR_GREEN;
            else if (getroomtype(x, y) == FOREST)
                color = CLR_BLACK;
            else if (getroomtype(x, y) == COCKNEST)
                color = CLR_GREEN;
            else if (getroomtype(x, y) == ANTHOLE)
                color = CLR_BROWN;
            else if (getroomtype(x, y) == SWAMP)
                color = CLR_GREEN;
            else if (getroomtype(x, y) == LEPREHALL)
                color = CLR_BRIGHT_GREEN;
            else if (getroomtype(x, y) == VAULT)
                color = HI_METAL;
            else if (getroomtype(x, y) == OWLBNEST)
                color = CLR_BLACK;
            else if (getroomtype(x, y) == ARMORY)
                color = HI_METAL;
            else if (getroomtype(x, y) == LEMUREPIT)
                color = CLR_RED;
            else
                cmap_color(offset);

        } else if (iflags.use_color && (offset == S_room)) {
            if (In_hell(&u.uz) && !In_W_tower(x, y, &u.uz) && !Is_valley(&u.uz))
                color = (Is_hella_level(&u.uz))
                         ? CLR_GREEN : CLR_ORANGE;
            else
                cmap_color(offset);

#endif
        /* try to provide a visible difference between water and lava
           if they use the same symbol and color is disabled */
        } else if (!iflags.use_color && offset == S_lava
                   && (showsyms[idx] == showsyms[S_pool + SYM_OFF_P]
                       || showsyms[idx]
                          == showsyms[S_water + SYM_OFF_P])) {
            special |= MG_BW_LAVA;
        } else if (offset == S_altar && iflags.use_color) {
            int amsk = altarmask_at(x, y); /* might be a mimic */

            if ((Is_astralevel(&u.uz) || Is_sanctum(&u.uz))
                && (amsk & AM_SHRINE) != 0) {
                /* high altar */
                color = CLR_BRIGHT_MAGENTA;
            } else {
                switch (amsk & AM_MASK) {
        /*
         * On OSX with TERM=xterm-color256 these render as
         *  white -> tty: gray, curses: ok
         *  gray  -> both tty and curses: black
         *  black -> both tty and curses: blue
         *  red   -> both tty and curses: ok.
         * Since the colors have specific associations (with the
         * unicorns matched with each alignment), we shouldn't use
         * scrambled colors and we don't have sufficient information
         * to handle platform-specific color variations.
         */
                case AM_LAWFUL:  /* 4 */
                    color = CLR_WHITE;
                    break;
                case AM_NEUTRAL: /* 2 */
                    color = CLR_GRAY;
                    break;
                case AM_CHAOTIC: /* 1 */
                    color = CLR_BLACK;
                    break;
#if 0
                case AM_LAWFUL:  /* 4 */
                case AM_NEUTRAL: /* 2 */
                case AM_CHAOTIC: /* 1 */
                    cmap_color(S_altar); /* gray */
                    break;
#endif /* 0 */
                case AM_NONE:    /* 0 */
                    color = CLR_RED;
                    break;
                default: /* 3, 5..7 -- shouldn't happen but 3 was possible
                          * prior to 3.6.3 (due to faulty sink polymorph) */
                    color = NO_COLOR;
                    break;
                }
            }
        } else {
            cmap_color(offset);
        }
    } else if ((offset = (glyph - GLYPH_OBJ_OFF)) >= 0) { /* object */
        struct obj* otmp = vobj_at(x, y);

        /* If no object at this location with matching type, check transient
           objects. Items in flight (thrown/kicked) aren't at the display
           location yet, but we need their material for proper coloring */
        if (!otmp || otmp->otyp != offset) {
            if (thrownobj && thrownobj->otyp == offset)
                otmp = thrownobj;
            else if (kickedobj && kickedobj->otyp == offset)
                otmp = kickedobj;
        }

        idx = objects[offset].oc_class + SYM_OFF_O;
        if (offset == BOULDER)
            idx = SYM_BOULDER + SYM_OFF_X;
        if (has_rogue_color && iflags.use_color) {
            switch (objects[offset].oc_class) {
            case COIN_CLASS:
                color = CLR_YELLOW;
                break;
            case FOOD_CLASS:
                color = CLR_RED;
                break;
            default:
                color = CLR_BRIGHT_BLUE;
                break;
            }
        } else
#ifdef TEXTCOLOR
        if (iflags.use_color && otmp && otmp->otyp == offset
            && otmp->material != objects[offset].oc_material) {
            color = material_color(otmp->material);
        } else
#endif
        {
            obj_color(offset);
            if (IS_EXT_COLOR(color) && !has_color(color))
                color = obj_color_fallback(color);
        }
        if (offset != BOULDER && is_objpile(x,y))
            special |= MG_OBJPILE;
        if (On_stairs(x,y))
            special |= MG_STAIRS;
	if (level.objects[x][y] && is_soko_prize_flag(level.objects[x][y])) {
	    sokoban_prize_color();
	}
    } else if ((offset = (glyph - GLYPH_RIDDEN_OFF)) >= 0) { /* mon ridden */
        idx = mons[offset].mlet + SYM_OFF_M;
        if (has_rogue_color)
            /* This currently implies that the hero is here -- monsters */
            /* don't ride (yet...).  Should we set it to yellow like in */
            /* the monster case below?  There is no equivalent in rogue. */
            color = CLR_RED; /* no need to check iflags.use_color */
        else
            mon_color(offset);
        special |= MG_RIDDEN;
    } else if ((offset = (glyph - GLYPH_BODY_OFF)) >= 0) { /* a corpse */
        idx = objects[CORPSE].oc_class + SYM_OFF_O;
        if (has_rogue_color && iflags.use_color)
            color = CLR_RED;
        else
            mon_color(offset);
        special |= MG_CORPSE;
        if (is_objpile(x,y))
            special |= MG_OBJPILE;
        if (On_stairs(x,y))
            special |= MG_STAIRS;
    } else if ((offset = (glyph - GLYPH_DETECT_OFF)) >= 0) { /* mon detect */
        idx = mons[offset].mlet + SYM_OFF_M;
        if (has_rogue_color)
            color = NO_COLOR; /* no need to check iflags.use_color */
        else
            mon_color(offset);
        /* Disabled for now; anyone want to get reverse video to work? */
        /* is_reverse = TRUE; */
        special |= MG_DETECT;
    } else if ((offset = (glyph - GLYPH_INVIS_OFF)) >= 0) { /* invisible */
        idx = SYM_INVISIBLE + SYM_OFF_X;
        if (has_rogue_color)
            color = NO_COLOR; /* no need to check iflags.use_color */
        else
            invis_color(offset);
        special |= MG_INVIS;
    } else if ((offset = (glyph - GLYPH_PEACEFUL_OFF)) >= 0) { /* peaceful */
        idx = mons[offset].mlet + SYM_OFF_M;
        if (has_rogue_color)
            color = NO_COLOR; /* no need to check iflags.use_color */
        else
            mon_color(offset);
        special |= MG_PEACEFUL;
    } else if ((offset = (glyph - GLYPH_PET_OFF)) >= 0) { /* a pet */
        idx = mons[offset].mlet + SYM_OFF_M;
        if (has_rogue_color)
            color = NO_COLOR; /* no need to check iflags.use_color */
        else
            pet_color(offset);
        special |= MG_PET;
    } else { /* a monster */
        idx = mons[glyph].mlet + SYM_OFF_M;
        if (has_rogue_color && iflags.use_color) {
            if (is_you)
                /* actually player should be yellow-on-gray if in corridor */
                color = CLR_YELLOW;
            else
                color = NO_COLOR;
        } else {
            mon_color(glyph);
#ifdef TEXTCOLOR
            /* special case for the hero in their normal form */
            if (iflags.use_color && is_you && !Upolyd)
                color = HI_DOMESTIC;
#endif
        }
    }

    /* These were requested by a blind player to enhance screen reader use */
    if (sysopt.accessibility == 1 && !(mgflags & MG_FLAG_NOOVERRIDE)) {
        int ovidx;

        if ((special & MG_PET) != 0) {
            ovidx = SYM_PET_OVERRIDE + SYM_OFF_X;
            if (Is_rogue_level(&u.uz) ? ov_rogue_syms[ovidx]
                                      : ov_primary_syms[ovidx])
                idx = ovidx;
        }
        if (is_you) {
            ovidx = SYM_HERO_OVERRIDE + SYM_OFF_X;
            if (Is_rogue_level(&u.uz) ? ov_rogue_syms[ovidx]
                                      : ov_primary_syms[ovidx])
                idx = ovidx;
        }
    }

    ch = showsyms[idx];
#ifdef TEXTCOLOR
    /* Customcolor override: per-glyph color from the CUSTOMCOLOR= rc
       directive. Applied before the has_color fallback so quantization
       still works for non-truecolor terminals. When the windowport
       advertises 24-bit truecolor, emit NH_CUSTOMCOLOR_SENTINEL so the
       windowport reads the entry and picks the best escape sequence */
    if (iflags.customcolors) {
        struct customcolor_entry *ce = customcolor_lookup(glyph);

        if (ce) {
            if ((ce->nhcolor & NH_BASIC_COLOR) != 0)
                color = (int) (ce->nhcolor & 0xFFL);
            else if (has_truecolor())
                color = NH_CUSTOMCOLOR_SENTINEL;
            else
                color = ce->color256idx;
        }
    }
    /* Turn off color if no color defined, or rogue level w/o PC graphics.
       For extended 256-colors, fall back to nearest base-16 color.
       NH_CUSTOMCOLOR_SENTINEL bypasses the check; the windowport
       resolves the entry via customcolor_lookup */
    if (Is_rogue_level(&u.uz) && !has_rogue_color)
        color = NO_COLOR;
    else if (color != NH_CUSTOMCOLOR_SENTINEL && !has_color(color))
        color = IS_EXT_COLOR(color) ? map_color_256to16(color) : NO_COLOR;
#else
    color = NO_COLOR;
#endif
    *ochar = (int) ch;
    *ospecial = special;
    *ocolor = color;
    return idx;
}

char *
encglyph(glyph)
int glyph;
{
    static char encbuf[20]; /* 10+1 would suffice */

    Sprintf(encbuf, "\\G%04X%04X", context.rndencode, glyph);
    return encbuf;
}

char *
decode_mixed(buf, str)
char *buf;
const char *str;
{
    static const char hex[] = "00112233445566778899aAbBcCdDeEfF";
    char *put = buf;

    if (!str)
        return strcpy(buf, "");

    while (*str) {
        if (*str == '\\') {
            int rndchk, dcount, so, gv, ch = 0, oc = 0;
            unsigned os = 0;
            const char *dp, *save_str;

            save_str = str++;
            switch (*str) {
            case 'G': /* glyph value \GXXXXNNNN*/
                rndchk = dcount = 0;
                for (++str; *str && ++dcount <= 4; ++str)
                    if ((dp = index(hex, *str)) != 0)
                        rndchk = (rndchk * 16) + ((int) (dp - hex) / 2);
                    else
                        break;
                if (rndchk == context.rndencode) {
                    gv = dcount = 0;
                    for (; *str && ++dcount <= 4; ++str)
                        if ((dp = index(hex, *str)) != 0)
                            gv = (gv * 16) + ((int) (dp - hex) / 2);
                        else
                            break;
                    so = mapglyph(gv, &ch, &oc, &os, 0, 0, 0);
                    if (iflags.supports_utf8 && showsyms[so] > 0x7F) {
                        int uc = get_unicode_codepoint(showsyms[so]);

                        put += utf8str_from_codepoint(uc, put);
                    } else {
                        *put++ = showsyms[so];
                    }
                    /* 'str' is ready for the next loop iteration and '*str'
                       should not be copied at the end of this iteration */
                    continue;
                } else {
                    /* possible forgery - leave it the way it is */
                    str = save_str;
                }
                break;
#if 0
            case 'S': /* symbol offset */
                so = rndchk = dcount = 0;
                for (++str; *str && ++dcount <= 4; ++str)
                    if ((dp = index(hex, *str)) != 0)
                        rndchk = (rndchk * 16) + ((int) (dp - hex) / 2);
                    else
                        break;
                if (rndchk == context.rndencode) {
                    dcount = 0;
                    for (; *str && ++dcount <= 2; ++str)
                        if ((dp = index(hex, *str)) != 0)
                            so = (so * 16) + ((int) (dp - hex) / 2);
                        else
                            break;
                }
                *put++ = showsyms[so];
                break;
#endif
            case '\\':
                break;
            case '\0':
                /* String ended with '\\'.  This can happen when someone
                   names an object with a name ending with '\\', drops the
                   named object on the floor nearby and does a look at all
                   nearby objects. */
                /* brh - should we perhaps not allow things to have names
                   that contain '\\' */
                str = save_str;
                break;
            }
        }
        *put++ = *str++;
    }
    *put = '\0';
    return buf;
}

/*
 * This differs from putstr() because the str parameter can
 * contain a sequence of characters representing:
 *        \GXXXXNNNN    a glyph value, encoded by encglyph().
 *
 * For window ports that haven't yet written their own
 * XXX_putmixed() routine, this general one can be used.
 * It replaces the encoded glyph sequence with a single
 * showsyms[] char, then just passes that string onto
 * putstr().
 */

void
genl_putmixed(window, attr, str)
winid window;
int attr;
const char *str;
{
    char buf[BUFSZ];

    /* now send it to the normal putstr */
    putstr(window, attr, decode_mixed(buf, str));
}

/* Return the display color for a given material type. Used by menu
   display code and map glyph rendering to show objects in
   material-appropriate colors. On 256-color terminals, returns
   extended palette colors; falls back to base-16 otherwise.
   Returns NO_COLOR for invalid materials or if color is disabled */
int
material_color(mat)
int mat;
{
#ifdef TEXTCOLOR
    /* Base-16 fallback colors. Must match order of enum
       obj_material_types in objclass.h: 0=unused, 1=LIQUID..24=MINERAL */
    static const int materialclr[] = {
        CLR_BLACK,       /*  0: unused       */
        CLR_BROWN,       /*  1: LIQUID       */
        CLR_WHITE,       /*  2: WAX          */
        CLR_BROWN,       /*  3: VEGGY        */
        CLR_RED,         /*  4: FLESH        */
        CLR_WHITE,       /*  5: PAPER        */
        CLR_BROWN,       /*  6: CLOTH        */
        CLR_GRAY,        /*  7: SPIDER_SILK  */
        CLR_BROWN,       /*  8: LEATHER      */
        CLR_BROWN,       /*  9: WOOD         */
        CLR_WHITE,       /* 10: BONE         */
        CLR_BLACK,       /* 11: DRAGON_HIDE  */
        CLR_CYAN,        /* 12: IRON         */
        CLR_CYAN,        /* 13: STEEL        */
        CLR_ORANGE,      /* 14: COPPER       */
        CLR_ORANGE,      /* 15: BRONZE       */
        CLR_GRAY,        /* 16: SILVER       */
        CLR_YELLOW,      /* 17: GOLD         */
        CLR_WHITE,       /* 18: PLATINUM     */
        CLR_GRAY,        /* 19: MITHRIL      */
        CLR_BLACK,       /* 20: ADAMANTINE   */
        CLR_WHITE,       /* 21: PLASTIC      */
        CLR_BRIGHT_CYAN, /* 22: GLASS        */
        CLR_RED,         /* 23: GEMSTONE     */
        CLR_GRAY,        /* 24: MINERAL      */
    };
    /* Extended 256-color alternatives. NO_COLOR = use base-16 above */
    static const int materialclr_ext[] = {
        NO_COLOR,       /*  0: unused       */
        NO_COLOR,       /*  1: LIQUID       */
        HI_WAX,         /*  2: WAX          */
        HI_VEGGY,       /*  3: VEGGY        */
        HI_FLESH,       /*  4: FLESH        */
        NO_COLOR,       /*  5: PAPER        */
        HI_CLOTH,       /*  6: CLOTH        */
        HI_SPIDER_SILK, /*  7: SPIDER_SILK  */
        NO_COLOR,       /*  8: LEATHER      */
        HI_WOOD,        /*  9: WOOD         */
        HI_BONE,        /* 10: BONE         */
        HI_DRAGON_HIDE, /* 11: DRAGON_HIDE  */
        NO_COLOR,       /* 12: IRON         */
        HI_STEEL,       /* 13: STEEL        */
        HI_COPPER,      /* 14: COPPER       */
        HI_BRONZE,      /* 15: BRONZE       */
        HI_SILVER,      /* 16: SILVER       */
        HI_GOLD,        /* 17: GOLD         */
        HI_PLATINUM,    /* 18: PLATINUM     */
        HI_MITHRIL,     /* 19: MITHRIL      */
        HI_ADAMANTINE,  /* 20: ADAMANTINE   */
        HI_PLASTIC,     /* 21: PLASTIC      */
        HI_GLASS,       /* 22: GLASS        */
        HI_GEMSTONE,    /* 23: GEMSTONE     */
        HI_MINERAL,     /* 24: MINERAL      */
    };
    if (iflags.use_color && mat >= 0 && mat < SIZE(materialclr)) {
        int ext = materialclr_ext[mat];
        if (ext != NO_COLOR && has_color(ext))
            return ext;
        return materialclr[mat];
    }
#else
    nhUse(mat);
#endif
    return NO_COLOR;
}

/*mapglyph.c*/
