/* NetHack 3.6	align.h	$NHDT-Date: 1432512779 2015/05/25 00:12:59 $  $NHDT-Branch: master $:$NHDT-Revision: 1.8 $ */
/* Copyright (c) Mike Stephenson, Izchak Miller  1991.		  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef ALIGN_H
#define ALIGN_H

typedef schar aligntyp; /* basic alignment type */

/* Alignment abuse event types for tracking recent transgressions */
enum abuse_type {
    ABUSE_NONE = 0,
    /* Combat */
    ABUSE_ATTACK_HELPLESS,      /* Knight attacking helpless/sleeping */
    ABUSE_ATTACK_PEACEFUL,      /* attacking peaceful creature */
    ABUSE_ATTACK_WOODLAND,      /* Druid attacking woodland */
    ABUSE_ATTACK_ELBERETH,      /* attacking while on Elbereth */
    /* Killing */
    ABUSE_KILL_PEACEFUL,        /* killing peaceful */
    ABUSE_KILL_PEACEFUL_MALIGN, /* additional malign penalty for killing peaceful */
    ABUSE_KILL_PET,             /* murdering pet */
    ABUSE_KILL_PRIEST,          /* killing coaligned priest */
    ABUSE_KILL_GUARDIAN,        /* killing quest guardian */
    ABUSE_KILL_LEADER,          /* killing quest leader */
    ABUSE_KILL_UNICORN,         /* killing same-aligned unicorn */
    ABUSE_KILL_DEITY,           /* killing your deity */
    /* Dishonorable weapons */
    ABUSE_USE_POISON,           /* poison weapon */
    ABUSE_USE_DISEASE,          /* disease weapon */
    /* Theft */
    ABUSE_LYING,                /* lying to a guard */
    ABUSE_SHOPLIFTING,          /* stealing from a shop */
    ABUSE_VANDALISM,            /* damaging a shop */
    ABUSE_PICKPOCKET,           /* caught pickpocketing */
    /* Sacrifice */
    ABUSE_SAC_PET,              /* sacrificing pet */
    ABUSE_SAC_SAME_RACE,        /* same-race sacrifice */
    /* Environmental */
    ABUSE_GRAVE_ROB,            /* grave robbing */
    ABUSE_DESTROY_TREE,         /* destroying a tree */
    /* Conduct */
    ABUSE_GLUTTONY,             /* Knight gluttony */
    ABUSE_CANNIBALISM,          /* eating own kind */
    ABUSE_GENOCIDE_HUMAN,       /* genociding humans */
    /* Religious */
    ABUSE_QUEST_BETRAYAL,       /* betraying quest leader */
    ABUSE_COWARDICE,            /* Infidel Elbereth */
    /* Mounting */
    ABUSE_STEED_DEATH,          /* steed killed by hazard */
    ABUSE_PET_PUSH,             /* pushing pet into hazard */
    /* Spells */
    ABUSE_SPELL_PEACEFUL,       /* spell turns peaceful hostile */
    /* Artifact */
    ABUSE_VECNA,                /* invoking Vecna artifact */
    /* Role-specific */
    ABUSE_FORBIDDEN_WEAPON,     /* Monk/Priest using wrong weapon */
    ABUSE_INFIDEL_GOOD,         /* Infidel doing good deeds */
    /* Prayer/Sacrifice */
    ABUSE_WRONG_ALTAR,          /* praying at wrong altar */
    ABUSE_DECEPTION,            /* trickery/fake Amulet */
    ABUSE_AMULET_BETRAYAL,      /* offering Amulet to wrong deity */
    ABUSE_SAC_REJECTED,         /* Amulet sacrifice rejected (Infidel) */
    ABUSE_NEGLECT_OFFERING,     /* Infidel neglecting sacrifice to Moloch */
    ABUSE_REFUSE_TITHE,         /* refusing temple contribution */
    /* Archeologist */
    ABUSE_HISTORIC_STATUE,      /* damaging historic statue */
    /* Special NPCs */
    ABUSE_ATTACK_ENCHANTRESS,   /* attacking Kathryn the Enchantress */
    /* Conduct (Monk) */
    ABUSE_VEGETARIAN,           /* Monk eating meat */
    /* Atonement (not abuse, but tracked in same history) */
    ABUSE_ATONEMENT             /* atoned for past abuse via temple donation */
};

typedef struct align { /* alignment & record */
    aligntyp type;
    int record;
    int abuse;
} align;

/* bounds for "record" -- respect initial alignments of 10 */
#define ALIGNLIM (10L + (moves / 200L))

#define A_NONE (-128) /* the value range of type */

#define A_CHAOTIC (-1)
#define A_NEUTRAL 0
#define A_LAWFUL 1

#define A_COALIGNED 1
#define A_OPALIGNED (-1)

#define AM_NONE 0
#define AM_CHAOTIC 1
#define AM_NEUTRAL 2
#define AM_LAWFUL 4

#define AM_MASK 7

#define AM_SPLEV_CO 3
#define AM_SPLEV_NONCO 7

#define Amask2align(x)                                          \
    ((aligntyp)((!(x)) ? A_NONE : ((x) == AM_LAWFUL) ? A_LAWFUL \
                                                     : ((int) x) - 2))
#define Align2amask(x) \
    (((x) == A_NONE) ? AM_NONE : ((x) == A_LAWFUL) ? AM_LAWFUL : (x) + 2)

/* for altars (see pray.c and wizard.c) */
#define a_align(x, y) ((aligntyp) Amask2align(levl[x][y].altarmask & AM_MASK))

#endif /* ALIGN_H */
