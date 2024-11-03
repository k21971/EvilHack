/* NetHack 3.6	vision.h	$NHDT-Date: 1559994624 2019/06/08 11:50:24 $  $NHDT-Branch: NetHack-3.6 $:$NHDT-Revision: 1.10 $ */
/* Copyright (c) Dean Luick, with acknowledgements to Dave Cohrs, 1990. */
/* NetHack may be freely redistributed.  See license for details.	*/

#ifndef VISION_H
#define VISION_H

#if 0 /* (moved to decl.h) */
extern boolean vision_full_recalc;	/* TRUE if need vision recalc */
extern char **viz_array;		/* could see/in sight row pointers */
extern char *viz_rmin;			/* min could see indices */
extern char *viz_rmax;			/* max could see indices */
#endif
#define COULD_SEE 0x01 /* location could be seen, if it were lit */
#define IN_SIGHT  0x02 /* location can be seen */
#define TEMP_LIT  0x04 /* location is temporarily lit */
#define TEMP_DARK 0x08 /* location is temporarily darkened */
#define UV_SEEN   0x10 /* location visible with ultravision */

/*
 * Light source sources
 */
#define LS_OBJECT 0
#define LS_MONSTER 1

/*
 *  cansee()	- Returns true if the hero can see the location.
 *
 *  couldsee()	- Returns true if the hero has a clear line of sight to
 *		  the location.
 */
#define cansee(x, y) (viz_array[y][x] & IN_SIGHT)
#define couldsee(x, y) (viz_array[y][x] & COULD_SEE)
#define uv_cansee(x, y) (viz_array[y][x] & UV_SEEN)
#define templit(x, y) \
    ((viz_array[y][x] & TEMP_LIT) && !(viz_array[y][x] & TEMP_DARK))
#define spot_is_dark(x, y) \
    (!(levl[x][y].lit || (viz_array[y][x] & TEMP_LIT)) \
     || (viz_array[y][x] & TEMP_DARK) \
     || (x == u.ux && y == u.uy && u.uswallow))

/*
 *  The following assume the monster is not blind.
 *
 *  m_cansee()	- Returns true if the monster can see the given location.
 *
 *  m_canseeu() - Returns true if the monster could see the hero.  Assumes
 *		  that if the hero has a clear line of sight to the monster's
 *		  location and the hero is visible, then monster can see the
 *		  hero.
 */
#define m_cansee(mtmp, x2, y2) clear_path((mtmp)->mx, (mtmp)->my, (x2), (y2))

#if 0
#define m_canseeu(m) \
    ((!Invis || perceives((m)->data))                      \
     && !(u.uburied || (m)->mburied)                       \
     && couldsee((m)->mx, (m)->my))
#else   /* without 'uburied' and 'mburied' */
#define m_canseeu(m) \
    ((!Invis || racial_perceives(m))                       \
     && couldsee((m)->mx, (m)->my))
#endif

/*
 *  Circle information
 */
#define MAX_RADIUS 15 /* this is in points from the source */

/* Use this macro to get a list of distances of the edges (see vision.c). */
#define circle_ptr(z) (&circle_data[(int) circle_start[z]])

/* howmonseen() bitmask values */
#define MONSEEN_NORMAL   0x0001 /* normal vision */
#define MONSEEN_SEEINVIS 0x0002 /* seeing invisible */
#define MONSEEN_INFRAVIS 0x0004 /* via infravision */
#define MONSEEN_ULTRAVIS 0x0008 /* via ultravision */
#define MONSEEN_TELEPAT  0x0010 /* via telepathy */
#define MONSEEN_XRAYVIS  0x0020 /* via Xray vision */
#define MONSEEN_DETECT   0x0040 /* via extended monster detection */
#define MONSEEN_WARNMON  0x0080 /* via type-specific warning */

#endif /* VISION_H */
