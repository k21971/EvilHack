/* NetHack 3.6	monattk.h	$NHDT-Date: 1432512775 2015/05/25 00:12:55 $  $NHDT-Branch: master $:$NHDT-Revision: 1.11 $ */
/* NetHack may be freely redistributed.  See license for details. */
/* Copyright 1988, M. Stephenson */

#ifndef MONATTK_H
#define MONATTK_H

/*	Add new attack types below - ordering affects experience (exper.c).
 *	Attacks > AT_BUTT are worth extra experience.
 */
#define AT_ANY (-1) /* fake attack; dmgtype_fromattack wildcard */
#define AT_NONE 0   /* passive monster (ex. acid blob) */
#define AT_CLAW 1   /* claw (punch, hit, etc.) */
#define AT_BITE 2   /* bite */
#define AT_KICK 3   /* kick */
#define AT_BUTT 4   /* head butt (ex. a unicorn) */
#define AT_TUCH 5   /* touches */
#define AT_STNG 6   /* sting */
#define AT_HUGS 7   /* crushing bearhug */
#define AT_SPIT 10  /* spits substance - ranged */
#define AT_ENGL 11  /* engulf (swallow or by a cloud) */
#define AT_BREA 12  /* breath - ranged */
#define AT_EXPL 13  /* explodes - proximity */
#define AT_BOOM 14  /* explodes when killed */
#define AT_GAZE 15  /* gaze - ranged */
#define AT_TENT 16  /* tentacles */
#define AT_SCRE 17  /* scream - sonic attack */

#define AT_WEAP 254 /* uses weapon */
#define AT_MAGC 255 /* uses magic spell(s) */

/*	Add new damage types below.
 *
 *	Note that 1-10 correspond to the types of attack used in buzz().
 *	Please don't disturb the order unless you rewrite the buzz() code.
 */
#define AD_ANY (-1) /* fake damage; attacktype_fordmg wildcard */
#define AD_PHYS 0   /* ordinary physical */
#define AD_MAGM 1   /* magic missiles */
#define AD_FIRE 2   /* fire damage */
#define AD_COLD 3   /* frost damage */
#define AD_SLEE 4   /* sleep ray */
#define AD_DISN 5   /* disintegration (death ray) */
#define AD_ELEC 6   /* shock damage */
#define AD_DRST 7   /* drains str (poison) */
#define AD_WATR 8   /* water (physical) attack */
#define AD_ACID 9   /* acid damage */
#define AD_LOUD 10  /* sound damage */
#define AD_SPC1 11  /* for extension of buzz() */
#define AD_SPC2 12  /* for extension of buzz() */
#define AD_BLND 13  /* blinds (yellow light) */
#define AD_STUN 14  /* stuns */
#define AD_SLOW 15  /* slows */
#define AD_PLYS 16  /* paralyses */
#define AD_DRLI 17  /* drains life levels (Vampire) */
#define AD_DREN 18  /* drains magic energy */
#define AD_LEGS 19  /* damages legs (xan) */
#define AD_STON 20  /* petrifies (Medusa, cockatrice) */
#define AD_STCK 21  /* sticks to you (mimic) */
#define AD_SGLD 22  /* steals gold (leppie) */
#define AD_SITM 23  /* steals item (nymphs) */
#define AD_SEDU 24  /* seduces & steals multiple items */
#define AD_TLPT 25  /* teleports you (Quantum Mech.) */
#define AD_RUST 26  /* rusts armour (Rust Monster)*/
#define AD_CONF 27  /* confuses (Umber Hulk) */
#define AD_DGST 28  /* digests opponent (trapper, etc.) */
#define AD_HEAL 29  /* heals opponent's wounds (nurse) */
#define AD_WRAP 30  /* special "stick" for eels */
#define AD_WERE 31  /* confers lycanthropy */
#define AD_DRDX 32  /* drains dexterity (quasit) */
#define AD_DRCO 33  /* drains constitution */
#define AD_DRIN 34  /* drains intelligence (mind flayer) */
#define AD_DISE 35  /* confers diseases */
#define AD_DCAY 36  /* decays organics (brown Pudding) */
#define AD_SSEX 37  /* Succubus seduction (extended) */
#define AD_HALU 38  /* causes hallucination */
#define AD_DETH 39  /* for Death only */
#define AD_PEST 40  /* for Pestilence only */
#define AD_FAMN 41  /* for Famine only */
#define AD_SLIM 42  /* turns you into green slime */
#define AD_ENCH 43  /* remove enchantment (disenchanter) */
#define AD_CORR 44  /* corrode armor (black pudding) */
#define AD_CNCL 45  /* cancellation */
#define AD_BHED 46  /* beheading (vorpal jabberwock) */
#define AD_LUCK 47  /* affects luck (magical eye) */
#define AD_PSYC 48  /* psionic attack */

#define AD_CLRC 240 /* random clerical spell */
#define AD_SPEL 241 /* random magic spell */
#define AD_RBRE 242 /* random breath weapon */

#define AD_SAMU 252 /* hits, may steal Amulet (Wizard) */
#define AD_CURS 253 /* random curse (ex. gremlin) */

/*
 *  Monster to monster attacks.  When a monster attacks another (mattackm),
 *  any or all of the following can be returned.  See mattackm() for more
 *  details.
 */
#define MM_MISS 0x0     /* aggressor missed */
#define MM_HIT 0x1      /* aggressor hit defender */
#define MM_DEF_DIED 0x2 /* defender died */
#define MM_AGR_DIED 0x4 /* aggressor died */
#define MM_EXPELLED 0x8 /* defender was saved by slow digestion */

#endif /* MONATTK_H */
