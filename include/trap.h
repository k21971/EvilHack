/* NetHack 3.6	trap.h	$NHDT-Date: 1547255912 2019/01/12 01:18:32 $  $NHDT-Branch: NetHack-3.6.2-beta01 $:$NHDT-Revision: 1.17 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Pasi Kallinen, 2016. */
/* NetHack may be freely redistributed.  See license for details. */

/* note for 3.1.0 and later: no longer manipulated by 'makedefs' */

#ifndef TRAP_H
#define TRAP_H

union vlaunchinfo {
    short v_launch_otyp; /* type of object to be triggered */
    coord v_launch2;     /* secondary launch point (for boulders) */
    uchar v_conjoined;   /* conjoined pit locations */
    short v_tnote;       /* boards: 12 notes        */
};

struct trap {
    struct trap *ntrap;
    xchar tx, ty;
    d_level dst; /* destination for portals */
    coord launch;
    Bitfield(ttyp, 5);
    Bitfield(tseen, 1);
    Bitfield(once, 1);
    Bitfield(madeby_u, 1); /* So monsters may take offence when you trap
                              them.  Recognizing who made the trap isn't
                              completely unreasonable, everybody has
                              their own style.  This flag is also needed
                              when you untrap a monster.  It would be too
                              easy to make a monster peaceful if you could
                              set a trap for it and then untrap it. */
    struct obj* ammo; /* object associated with this trap - darts for a dart
                         trap, arrows for an arrow trap, a beartrap object for a
                         bear trap.  This object does not physically exist in
                         the game until some action creates it, such as the
                         beartrap being untrapped, or one dart being fired.
                         Not all types of traps will need this field - in fact,
                         most don't. Only those which need to store persistent
                         information about the associated object do. */
    union vlaunchinfo vl;
#define launch_otyp vl.v_launch_otyp
#define launch2 vl.v_launch2
#define conjoined vl.v_conjoined
#define tnote vl.v_tnote
};

extern struct trap *ftrap;
#define newtrap() (struct trap *) alloc(sizeof(struct trap))
#define dealloc_trap(trap) free((genericptr_t)(trap))

/* reasons for statue animation */
#define ANIMATE_NORMAL 0
#define ANIMATE_SHATTER 1
#define ANIMATE_SPELL 2

/* reasons for animate_statue's failure */
#define AS_OK 0            /* didn't fail */
#define AS_NO_MON 1        /* makemon failed */
#define AS_MON_IS_UNIQUE 2 /* statue monster is unique */

/* Note: if adding/removing a trap, adjust trap_engravings[] in mklev.c */

/* unconditional traps */
enum trap_types {
    NO_TRAP              =  0,
    ARROW_TRAP_SET       =  1,
    BOLT_TRAP_SET        =  2,
    DART_TRAP_SET        =  3,
    ROCKTRAP             =  4,
    SQKY_BOARD           =  5,
    BEAR_TRAP            =  6,
    LANDMINE             =  7,
    ROLLING_BOULDER_TRAP =  8,
    SLP_GAS_TRAP_SET     =  9,
    RUST_TRAP_SET        = 10,
    FIRE_TRAP_SET        = 11,
    ICE_TRAP_SET         = 12,
    PIT                  = 13,
    SPIKED_PIT           = 14,
    HOLE                 = 15,
    TRAPDOOR             = 16,
    TELEP_TRAP_SET       = 17,
    LEVEL_TELEP          = 18,
    MAGIC_PORTAL         = 19,
    WEB                  = 20,
    STATUE_TRAP          = 21,
    MAGIC_TRAP_SET       = 22,
    ANTI_MAGIC           = 23,
    POLY_TRAP_SET        = 24,
    SPEAR_TRAP_SET       = 25,
    MAGIC_BEAM_TRAP_SET  = 26,
    VIBRATING_SQUARE     = 27,

    TRAPNUM              = 28
};

#define is_pit(ttyp) ((ttyp) == PIT || (ttyp) == SPIKED_PIT)
#define is_hole(ttyp)  ((ttyp) == HOLE || (ttyp) == TRAPDOOR)
#define undestroyable_trap(ttyp) ((ttyp) == MAGIC_PORTAL         \
                                  || (ttyp) == VIBRATING_SQUARE)
#define is_magical_trap(ttyp) ((ttyp) == TELEP_TRAP_SET          \
                               || (ttyp) == LEVEL_TELEP          \
                               || (ttyp) == MAGIC_TRAP_SET       \
                               || (ttyp) == ANTI_MAGIC           \
                               || (ttyp) == POLY_TRAP_SET        \
                               || (ttyp) == MAGIC_BEAM_TRAP_SET)

/* Values for deltrap_with_ammo */
enum deltrap_handle_ammo {
    DELTRAP_RETURN_AMMO = 0, /* return ammo to caller; do nothing with it */
    DELTRAP_DESTROY_AMMO,    /* delete ammo */
    DELTRAP_PLACE_AMMO,      /* place ammo on ground where trap was */
    DELTRAP_BURY_AMMO,       /* bury ammo under where trap was */
    DELTRAP_TAKE_AMMO        /* put ammo into player's inventory */
};

#endif /* TRAP_H */
