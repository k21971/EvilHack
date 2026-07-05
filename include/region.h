/* NetHack 3.6	region.h	$NHDT-Date: 1432512779 2015/05/25 00:12:59 $  $NHDT-Branch: master $:$NHDT-Revision: 1.13 $ */
/* Copyright (c) 1996 by Jean-Christophe Collet			  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef REGION_H
#define REGION_H

/* generic callback function */

typedef boolean FDECL((*callback_proc), (genericptr_t, genericptr_t));

/*
 * player_flags
 */
#define REG_HERO_INSIDE 0x01
#define REG_NOT_HEROS 0x02
#define hero_inside(r) ((r)->player_flags & REG_HERO_INSIDE)
#define heros_fault(r) (!((r)->player_flags & REG_NOT_HEROS))
#define set_hero_inside(r) ((r)->player_flags |= REG_HERO_INSIDE)
#define clear_hero_inside(r) ((r)->player_flags &= ~REG_HERO_INSIDE)
#define set_heros_fault(r) ((r)->player_flags &= ~REG_NOT_HEROS)
#define clear_heros_fault(r) ((r)->player_flags |= REG_NOT_HEROS)

/*
 * Note: if you change the size/type of any of the fields below,
 *       or add any/remove any fields, you must update the
 *       bwrite() calls in save_regions(), and the
 *       mread() calls in rest_regions() in src/region.c
 *       to reflect the changes.
 */

typedef struct {
    NhRect bounding_box;        /* Bounding box of the region */
    NhRect *rects;              /* Rectangles composing the region */
    short nrects;               /* Number of rectangles  */
    boolean attach_2_u;         /* Region attached to player ? */
    unsigned int attach_2_m;    /* Region attached to monster ? */
    /*struct obj *attach_2_o;*/ /* Region attached to object ? UNUSED YET */
    const char *enter_msg;      /* Message when entering */
    const char *leave_msg;      /* Message when leaving */
    long ttl;                   /* Time to live. -1 is forever */
    short expire_f;            /* Function to call when region's ttl expire */
    short can_enter_f;         /* Function to call to check whether the player
                                  can, or can not, enter the region */
    short enter_f;             /* Function to call when the player enters*/
    short can_leave_f;         /* Function to call to check whether the player
                                  can, or can not, leave the region */
    short leave_f;             /* Function to call when the player leaves */
    short inside_f;            /* Function to call every turn if player's
                                  inside */
    unsigned int player_flags; /* (see above) */
    unsigned int *monsters;    /* Monsters currently inside this region */
    short n_monst;             /* Number of monsters inside this region */
    short max_monst;           /* Maximum number of monsters that can be
                                  listed without having to grow the array */
#define MONST_INC 5

    /* Should probably do the same thing about objects */

    boolean visible; /* Is the region visible ? */
    int glyph;       /* Which glyph to use if visible */
    anything arg;    /* Optional user argument (Ex: strength of
                        force field, damage of a fire zone, ...*/
    unsigned int creator_mid; /* m_id of the monster that created this
                                 region (0 = hero or environment), for
                                 pet kill attribution; deliberately NOT
                                 saved/restored (both create_region and
                                 rest_regions zero the struct), so
                                 attribution is dropped across reload */
} NhRegion;

/* Size caps used by rest_regions() to reject corrupt save prefixes */
#define MAX_REGIONS         100   /* generous; gas clouds rarely co-exist */
#define MAX_REGION_RECTS     64   /* radii > 8 are unheard of */
#define MAX_REGION_MONST    256   /* worm body cells can inflate this */
#define MAX_REGION_MSGLEN  BUFSZ  /* enter_msg/leave_msg cap */

/* Callback dispatch sentinels. NUM_CALLBACKS must match the size of
   the callbacks[] array in region.c; a compile-time check there
   enforces this */
#define NO_CALLBACK   (-1)
#define NUM_CALLBACKS  2
#define VALID_CALLBACK(idx) \
    ((idx) == NO_CALLBACK || ((idx) >= 0 && (idx) < NUM_CALLBACKS))

#endif /* REGION_H */
