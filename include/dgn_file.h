/* NetHack 5.0	dgn_file.h	$NHDT-Date: 1781973079 2026/06/20 16:31:19 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.15 $ */
/* Copyright (c) 1989 by M. Stephenson                            */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef DGN_FILE_H
#define DGN_FILE_H

#ifndef ALIGN_H
#include "align.h"
#endif

/*
 * Structures manipulated by the dungeon loader & compiler
 */

struct couple {
    short base, rand;
};

struct tmpdungeon {
    char *name, *protoname;
    struct couple lev;
    int flags, chance, levels, branches,
        entry_lev; /* entry level for this dungeon */
    char boneschar;
    int align;
};

struct tmplevel {
    char *name;
    char *chainlvl;
    struct couple lev;
    int chance, rndlevs, chain, flags;
    char boneschar;
    int align;
};

struct tmpbranch {
    char *name; /* destination dungeon name */
    struct couple lev;
    int chain; /* index into tmplevel array (chained branch)*/
    int type;  /* branch type (see below) */
    int up;    /* branch is up or down */
};

/*
 *    Values for type in tmpbranch structure.
 */
#define TBR_STAIR 0   /* connection with both ends having a staircase */
#define TBR_NO_UP 1   /* connection with no up staircase */
#define TBR_NO_DOWN 2 /* connection with no down staircase */
#define TBR_PORTAL 3  /* portal connection */

/*
 *    Flags that map into the dungeon flags bitfields.
 */
#define TOWN        0x01 /* levels only */
#define HELLISH     0x02
#define MAZELIKE    0x04
#define ROGUELIKE   0x08
#define UNCONNECTED 0x10
#define GTOWN       0x20

/*
 *    Dungeon and level alignment. These are carried in the dedicated
 *    'align' field of struct tmpdungeon and struct tmplevel and must
 *    never be OR'd into 'flags': the values overlap the flag bits above
 *    (D_ALIGN_CHAOTIC and UNCONNECTED are both 0x10), so packing them
 *    into one word makes an unconnected dungeon read as chaotic.
 *    dungeon.c unpacks them with D_ALIGN_SHIFT into a 3-bit field, so
 *    everything below derives from that one shift
 */
#define D_ALIGN_SHIFT 4
#define D_ALIGN_NONE 0
#define D_ALIGN_CHAOTIC (AM_CHAOTIC << D_ALIGN_SHIFT)
#define D_ALIGN_NEUTRAL (AM_NEUTRAL << D_ALIGN_SHIFT)
#define D_ALIGN_LAWFUL (AM_LAWFUL << D_ALIGN_SHIFT)

#define D_ALIGN_MASK (AM_MASK << D_ALIGN_SHIFT)

/*
 *    Max number of prototype levels and branches.
 */
#define LEV_LIMIT 99
#define BRANCH_LIMIT 32

#endif /* DGN_FILE_H */
