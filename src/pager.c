/* NetHack 3.6	pager.c	$NHDT-Date: 1574722864 2019/11/25 23:01:04 $  $NHDT-Branch: NetHack-3.6 $:$NHDT-Revision: 1.162 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2018. */
/* NetHack may be freely redistributed.  See license for details. */

/* This file contains the command routines dowhatis() and dohelp() and */
/* a few other help related facilities */

#include "hack.h"
#include "dlb.h"

STATIC_DCL boolean FDECL(is_swallow_sym, (int));
STATIC_DCL int FDECL(append_str, (char *, const char *));
STATIC_DCL void FDECL(look_at_object, (char *, int, int, int));
STATIC_DCL void FDECL(look_at_monster, (char *, char *,
                                        struct monst *, int, int));
STATIC_DCL struct permonst *FDECL(lookat, (int, int, char *, char *));
STATIC_DCL char *FDECL(hallucinatory_armor, (char *));
STATIC_DCL void FDECL(add_mon_info, (winid, struct permonst *));
STATIC_DCL void FDECL(add_obj_info, (winid, SHORT_P));
STATIC_DCL void FDECL(look_all, (BOOLEAN_P,BOOLEAN_P));
STATIC_DCL void FDECL(do_supplemental_info, (char *, struct permonst *,
                                             BOOLEAN_P));
STATIC_DCL void NDECL(whatdoes_help);
STATIC_DCL void NDECL(docontact);
STATIC_DCL void NDECL(dispfile_help);
STATIC_DCL void NDECL(dispfile_shelp);
STATIC_DCL void NDECL(dispfile_optionfile);
STATIC_DCL void NDECL(dispfile_license);
STATIC_DCL void NDECL(dispfile_debughelp);
STATIC_DCL void NDECL(hmenu_doextversion);
STATIC_DCL void NDECL(hmenu_dohistory);
STATIC_DCL void NDECL(hmenu_dowhatis);
STATIC_DCL void NDECL(hmenu_dowhatdoes);
STATIC_DCL void NDECL(hmenu_doextlist);
#ifdef PORT_HELP
extern void NDECL(port_help);
#endif

/* Returns "true" for characters that could represent a monster's stomach. */
STATIC_OVL boolean
is_swallow_sym(c)
int c;
{
    int i;

    for (i = S_sw_tl; i <= S_sw_br; i++)
        if ((int) showsyms[i] == c)
            return TRUE;
    return FALSE;
}

/* Append " or "+new_str to the end of buf if new_str doesn't already exist
   as a substring of buf.  Return 1 if the string was appended, 0 otherwise.
   It is expected that buf is of size BUFSZ. */
STATIC_OVL int
append_str(buf, new_str)
char *buf;
const char *new_str;
{
    static const char sep[] = " or ";
    size_t oldlen, space_left;

    if (strstri(buf, new_str))
        return 0; /* already present */

    oldlen = strlen(buf);
    if (oldlen >= BUFSZ - 1) {
        if (oldlen > BUFSZ - 1)
            impossible("append_str: 'buf' contains %lu characters.",
                       (unsigned long) oldlen);
        return 0; /* no space available */
    }

    /* some space available, but not necessarily enough for full append */
    space_left = BUFSZ - 1 - oldlen;  /* space remaining in buf */
    (void) strncat(buf, sep, space_left);
    if (space_left > sizeof sep - 1)
        (void) strncat(buf, new_str, space_left - (sizeof sep - 1));
    return 1; /* something was appended, possibly just part of " or " */
}

/* shared by monster probing (via query_objlist!) as well as lookat() */
char *
self_lookat(outbuf)
char *outbuf;
{
    char race[QBUFSZ];

    /* include race with role unless polymorphed */
    race[0] = '\0';
    if (!Upolyd)
        Sprintf(race, "%s ", urace.adj);
    Sprintf(outbuf, "%s%s%s called %s",
            /* being blinded may hide invisibility from self */
            (Invis && (senseself() || !Blind)) ? "invisible " : "", race,
            mons[u.umonnum].mname, plname);
    if (u.usteed)
        Sprintf(eos(outbuf), ", mounted on %s", y_monnam(u.usteed));
    if (u.uundetected || (Upolyd && U_AP_TYPE))
        mhidden_description(&youmonst, FALSE, eos(outbuf));
    return outbuf;
}

/* describe a hidden monster; used for look_at during extended monster
   detection and for probing; also when looking at self */
void
mhidden_description(mon, altmon, outbuf)
struct monst *mon;
boolean altmon; /* for probing: if mimicking a monster, say so */
char *outbuf;
{
    struct obj *otmp;
    boolean fakeobj, isyou = (mon == &youmonst);
    int x = isyou ? u.ux : mon->mx, y = isyou ? u.uy : mon->my,
        glyph = (level.flags.hero_memory && !isyou) ? levl[x][y].glyph
                                                    : glyph_at(x, y);

    *outbuf = '\0';
    if (M_AP_TYPE(mon) == M_AP_FURNITURE
        || M_AP_TYPE(mon) == M_AP_OBJECT) {
        Strcpy(outbuf, ", mimicking ");
        if (M_AP_TYPE(mon) == M_AP_FURNITURE) {
            Strcat(outbuf, an(defsyms[mon->mappearance].explanation));
        } else if (M_AP_TYPE(mon) == M_AP_OBJECT
                   /* remembered glyph, not glyph_at() which is 'mon' */
                   && glyph_is_object(glyph)) {
 objfrommap:
            otmp = (struct obj *) 0;
            fakeobj = object_from_map(glyph, x, y, &otmp);
            Strcat(outbuf, (otmp && otmp->otyp != STRANGE_OBJECT)
                              ? ansimpleoname(otmp)
                              : an(obj_descr[STRANGE_OBJECT].oc_name));
            if (fakeobj) {
                otmp->where = OBJ_FREE; /* object_from_map set to OBJ_FLOOR */
                dealloc_obj(otmp);
            }
        } else {
            Strcat(outbuf, something);
        }
    } else if (M_AP_TYPE(mon) == M_AP_MONSTER) {
        if (altmon)
            Sprintf(outbuf, ", masquerading as %s",
                    an(mons[mon->mappearance].mname));
    } else if (isyou ? u.uundetected : mon->mundetected) {
        Strcpy(outbuf, ", hiding");
        if (hides_under(mon->data)) {
            Strcat(outbuf, " under ");
            int hidetyp = concealed_spot(x, y);

            if (hidetyp == 1) { /* hiding with terrain */
                Strcat(outbuf, explain_terrain(x, y));
            } else {
                /* remembered glyph, not glyph_at() which is 'mon' */
                if (glyph_is_object(glyph))
                    goto objfrommap;
                Strcat(outbuf, something);
            }
        } else if (is_hider(mon->data)) {
            Sprintf(eos(outbuf), " on the %s",
                    (is_flyer(mon->data) || mon->data->mlet == S_PIERCER)
                       ? "ceiling"
                       : surface(x, y)); /* trapper */
        } else {
            if (mon->data->mlet == S_EEL && is_pool(x, y))
                Strcat(outbuf, " in murky water");
            if (mon->data->mlet == S_EEL && is_puddle(x, y))
                Strcat(outbuf, " in a murky puddle");
            if (mon->data == &mons[PM_GIANT_LEECH] && is_sewage(x, y))
                Strcat(outbuf, " in raw sewage");
        }
    }
}

/* extracted from lookat(); also used by namefloorobj() */
boolean
object_from_map(glyph, x, y, obj_p)
int glyph, x, y;
struct obj **obj_p;
{
    boolean fakeobj = FALSE, mimic_obj = FALSE;
    struct monst *mtmp;
    struct obj *otmp;
    int glyphotyp = glyph_to_obj(glyph);

    *obj_p = (struct obj *) 0;
    /* TODO: check inside containers in case glyph came from detection */
    if ((otmp = sobj_at(glyphotyp, x, y)) == 0)
        for (otmp = level.buriedobjlist; otmp; otmp = otmp->nobj)
            if (otmp->ox == x && otmp->oy == y && otmp->otyp == glyphotyp)
                break;

    /* there might be a mimic here posing as an object */
    mtmp = m_at(x, y);
    if (mtmp && is_obj_mappear(mtmp, (unsigned) glyphotyp)) {
        otmp = 0;
        mimic_obj = TRUE;
    } else
        mtmp = 0;

    if (!otmp || otmp->otyp != glyphotyp) {
        /* this used to exclude STRANGE_OBJECT; now caller deals with it */
        otmp = mksobj(glyphotyp, FALSE, FALSE);
        set_material(otmp, objects[otmp->otyp].oc_material);
        if (!otmp)
            return FALSE;
        fakeobj = TRUE;
        if (otmp->oclass == COIN_CLASS)
            otmp->quan = 2L; /* to force pluralization */
        else if (otmp->otyp == SLIME_MOLD)
            otmp->spe = context.current_fruit; /* give it a type */
        if (mtmp && has_mcorpsenm(mtmp)) { /* mimic as corpse/statue */
            if (otmp->otyp == SLIME_MOLD)
                /* override context.current_fruit to avoid
                     look, use 'O' to make new named fruit, look again
                   giving different results when current_fruit changes */
                otmp->spe = MCORPSENM(mtmp);
            else
                otmp->corpsenm = MCORPSENM(mtmp);
        } else if (otmp->otyp == CORPSE && glyph_is_body(glyph)) {
            otmp->corpsenm = glyph - GLYPH_BODY_OFF;
        } else if (otmp->otyp == STATUE && glyph_is_statue(glyph)) {
            otmp->corpsenm = glyph - GLYPH_STATUE_OFF;
        }
        if (otmp->otyp == LEASH)
            otmp->leashmon = 0;
        /* extra fields needed for shop price with doname() formatting */
        otmp->where = OBJ_FLOOR;
        otmp->ox = x, otmp->oy = y;
        otmp->no_charge = (otmp->otyp == STRANGE_OBJECT && costly_spot(x, y));
    }
    /* if located at adjacent spot, mark it as having been seen up close
       (corpse type will be known even if dknown is 0, so we don't need a
       touch check for cockatrice corpse--we're looking without touching) */
    if (otmp && distu(x, y) <= 2 && !Blind && !Hallucination
        /* redundant: we only look for an object which matches current
           glyph among floor and buried objects; when !Blind, any buried
           object's glyph will have been replaced by whatever is present
           on the surface as soon as we moved next to its spot */
        && (fakeobj || otmp->where == OBJ_FLOOR) /* not buried */
        /* terrain mode views what's already known, doesn't learn new stuff */
        && !iflags.terrainmode) /* so don't set dknown when in terrain mode */
        otmp->dknown = 1; /* if a pile, clearly see the top item only */
    if (fakeobj && mtmp && mimic_obj &&
        (otmp->dknown || (M_AP_FLAG(mtmp) & M_AP_F_DKNOWN))) {
            mtmp->m_ap_type |= M_AP_F_DKNOWN;
            otmp->dknown = 1;
    }
    *obj_p = otmp;
    return fakeobj; /* when True, caller needs to dealloc *obj_p */
}

STATIC_OVL void
look_at_object(buf, x, y, glyph)
char *buf; /* output buffer */
int x, y, glyph;
{
    struct obj *otmp = 0;
    boolean fakeobj = object_from_map(glyph, x, y, &otmp);

    if (otmp) {
        Strcpy(buf, (otmp->otyp != STRANGE_OBJECT)
                     ? distant_name(otmp, otmp->dknown ? doname_with_price
                                                       : doname_vague_quan)
                     : obj_descr[STRANGE_OBJECT].oc_name);
        if (fakeobj) {
            otmp->where = OBJ_FREE; /* object_from_map set it to OBJ_FLOOR */
            dealloc_obj(otmp), otmp = 0;
        }
    } else
        Strcpy(buf, something); /* sanity precaution */

    if (otmp && otmp->where == OBJ_BURIED)
        Strcat(buf, " (buried)");
    else if (levl[x][y].typ == STONE || levl[x][y].typ == SCORR)
        Strcat(buf, " embedded in stone");
    else if (IS_WALL(levl[x][y].typ) || levl[x][y].typ == SDOOR)
        Strcat(buf, " embedded in a wall");
    else if (closed_door(x, y))
        Strcat(buf, " embedded in a door");
    else if (is_pool(x, y))
        Strcat(buf, " in water");
    else if (is_lava(x, y))
        Strcat(buf, " in molten lava"); /* [can this ever happen?] */
    return;
}

STATIC_OVL void
look_at_monster(buf, monbuf, mtmp, x, y)
char *buf, *monbuf; /* buf: output, monbuf: optional output */
struct monst *mtmp;
int x, y;
{
    char *name, monnambuf[BUFSZ];
    boolean accurate = !Hallucination;
    char *mwounds;

    if (!canspotmon(mtmp) && has_erid(mtmp)
        && canspotmon(ERID(mtmp)->mon_steed))
        mtmp = ERID(mtmp)->mon_steed;

    name = (mtmp->data == &mons[PM_COYOTE] && accurate)
              ? coyotename(mtmp, monnambuf)
              : distant_monnam(mtmp, ARTICLE_NONE, monnambuf);
    Sprintf(buf, "%s%s%s",
            (mtmp->mx != x || mtmp->my != y)
                ? ((mtmp->isshk && accurate) ? "tail of " : "tail of a ")
                : "",
            (mtmp->mtame && accurate)
                ? "tame "
                : (mtmp->mpeaceful && accurate)
                    ? "peaceful "
                    : "",
            name);
    if (mtmp->mextra && ERID(mtmp) && ERID(mtmp)->mon_steed) {
        if (is_rider(mtmp->data) && (distu(mtmp->mx, mtmp->my) > 2)
            && !canseemon(mtmp))
            Sprintf(eos(buf), ", riding its steed.");
        else
            Sprintf(eos(buf), ", riding %s", a_monnam(ERID(mtmp)->mon_steed));
    }
    if (mtmp->ridden_by)
        Sprintf(eos(buf), ", being ridden");
    mwounds = mon_wounds(mtmp);
    if (mwounds) {
        Strcat(buf, ", ");
        Strcat(buf, mwounds);
    }
    if (u.ustuck == mtmp) {
        if (u.uswallow || iflags.save_uswallow) /* monster detection */
            Strcat(buf, is_swallower(mtmp->data)
                          ? ", swallowing you" : ", engulfing you");
        else
            Strcat(buf, (Upolyd && sticks(youmonst.data))
                          ? ", being held" : ", holding you");
    }
    if (mtmp->mleashed)
        Strcat(buf, ", leashed to you");

    if (mtmp->mentangled && cansee(mtmp->mx, mtmp->my))
        Strcat(buf, ", entangled");

    if (canseemon(mtmp) && !Blind) {
        if (accurate || program_state.gameover) {
            if (mtmp->misc_worn_check & W_ARMOR) {
                int base_ac = 0, arm_ct = 0;
                long atype;
                struct obj *otmp;

                for (atype = W_ARM; atype & W_ARMOR; atype <<= 1) {
                    if (!(otmp = which_armor(mtmp, atype)))
                        continue;
                    /* don't count armor->spe, since this represents only what
                     * the hero can see from afar -- monster with +8 gloves
                     * will still seem "lightly armored" from a distance */
                    base_ac += armor_bonus(otmp) - otmp->spe;
                    arm_ct++;
                }

                Sprintf(eos(buf), ", wearing %s%sarmor",
                        arm_ct > 4 ? "full " : arm_ct < 3 ? "some " : "",
                        base_ac > 9 ? "heavy " : base_ac < 6 ? "light " : "");
            }
            if (MON_WEP(mtmp))
                Sprintf(eos(buf), ", wielding %s",
                        ansimpleoname(MON_WEP(mtmp)));
        } else {
            if (rn2(3)) {
                char harmor[BUFSZ];
                Sprintf(eos(buf), ", wearing %s",
                        hallucinatory_armor(harmor));
            }
            if (rn2(3)) {
                struct obj *hwep = mkobj(RANDOM_CLASS, FALSE);
                Sprintf(eos(buf), ", wielding %s", ansimpleoname(hwep));
                obfree(hwep, (struct obj *) 0);
            }
        }
    }

    if (mtmp->mtrapped && cansee(mtmp->mx, mtmp->my)) {
        struct trap *t = t_at(mtmp->mx, mtmp->my);
        int tt = t ? t->ttyp : NO_TRAP;

        /* newsym lets you know of the trap, so mention it here */
        if (tt == BEAR_TRAP || is_pit(tt) || tt == WEB) {
            Sprintf(eos(buf), ", trapped in %s",
                    an(defsyms[trap_to_defsym(tt)].explanation));
            t->tseen = 1;
        }
    }

    /* we know the hero sees a monster at this location, but if it's shown
       due to persistant monster detection he might remember something else */
    if (mtmp->mundetected || M_AP_TYPE(mtmp))
        mhidden_description(mtmp, FALSE, eos(buf));

    if (monbuf) {
        unsigned how_seen = howmonseen(mtmp);

        monbuf[0] = '\0';
        if (how_seen != 0 && how_seen != MONSEEN_NORMAL) {
            if (how_seen & MONSEEN_NORMAL) {
                Strcat(monbuf, "normal vision");
                how_seen &= ~MONSEEN_NORMAL;
                /* how_seen can't be 0 yet... */
                if (how_seen)
                    Strcat(monbuf, ", ");
            }
            if (how_seen & MONSEEN_SEEINVIS) {
                Strcat(monbuf, "see invisible");
                how_seen &= ~MONSEEN_SEEINVIS;
                if (how_seen)
                    Strcat(monbuf, ", ");
            }
            if (how_seen & MONSEEN_INFRAVIS) {
                Strcat(monbuf, "infravision");
                how_seen &= ~MONSEEN_INFRAVIS;
                if (how_seen)
                    Strcat(monbuf, ", ");
            }
            if (how_seen & MONSEEN_ULTRAVIS) {
                Strcat(monbuf, "ultravision");
                how_seen &= ~MONSEEN_ULTRAVIS;
                if (how_seen)
                    Strcat(monbuf, ", ");
            }
            if (how_seen & MONSEEN_TELEPAT) {
                Strcat(monbuf, "telepathy");
                how_seen &= ~MONSEEN_TELEPAT;
                if (how_seen)
                    Strcat(monbuf, ", ");
            }
            if (how_seen & MONSEEN_XRAYVIS) {
                /* Eyes of the Overworld */
                Strcat(monbuf, "astral vision");
                how_seen &= ~MONSEEN_XRAYVIS;
                if (how_seen)
                    Strcat(monbuf, ", ");
            }
            if (how_seen & MONSEEN_DETECT) {
                Strcat(monbuf, "monster detection");
                how_seen &= ~MONSEEN_DETECT;
                if (how_seen)
                    Strcat(monbuf, ", ");
            }
            if (how_seen & MONSEEN_WARNMON) {
                if (Hallucination) {
                    Strcat(monbuf, "paranoid delusion");
                } else {
                    unsigned long mW = (context.warntype.obj
                                        | context.warntype.polyd),
                                  mh = has_erac(mtmp) ? ERAC(mtmp)->mrace
                                                      : mtmp->data->mhflags;
                    const char *whom = ((mW & MH_HUMAN & mh) ? "human"
                                        : (mW & MH_ELF & mh) ? "elf"
                                          : (mW & MH_DROW & mh) ? "drow"
                                            : (mW & MH_ORC & mh) ? "orc"
                                              : (mW & MH_UNDEAD & mh) ? "the undead"
                                                : (mW & MH_GIANT & mh) ? "giant"
                                        : (mW & MH_WERE & mh) ? "werecreature"
                                          : (mW & MH_DRAGON & mh) ? "dragon"
                                            : (mW & MH_OGRE & mh) ? "ogre"
                                              : (mW & MH_TROLL & mh) ? "troll"
                                                : (mW & MH_DEMON & mh) ? "demon"
                                                  : (mW & MH_ANGEL & mh) ? "angel"
                                                    : (mW & MH_JABBERWOCK & mh) ? "jabberwock"
                                                      : (mW & MH_WRAITH & mh) ? "wraith"
                                                        : mtmp->data->mname);

                    Sprintf(eos(monbuf), "warned of %s", makeplural(whom));
                }
                how_seen &= ~MONSEEN_WARNMON;
                if (how_seen)
                    Strcat(monbuf, ", ");
            }
            /* should have used up all the how_seen bits by now */
            if (how_seen) {
                impossible("lookat: unknown method of seeing monster");
                Sprintf(eos(monbuf), "(%u)", how_seen);
            }
        } /* seen by something other than normal vision */
    } /* monbuf is non-null */
}

/*
 * Return the name of the glyph found at (x,y).
 * If not hallucinating and the glyph is a monster, also monster data.
 */
STATIC_OVL struct permonst *
lookat(x, y, buf, monbuf)
int x, y;
char *buf, *monbuf;
{
    struct monst *mtmp = (struct monst *) 0;
    struct permonst *pm = (struct permonst *) 0;
    int glyph;
    char *releasep;

    buf[0] = monbuf[0] = '\0';
    glyph = glyph_at(x, y);
    if (u.ux == x && u.uy == y && canspotself()
        && !(iflags.save_uswallow &&
             glyph == mon_to_glyph(u.ustuck, rn2_on_display_rng))
        && (!iflags.terrainmode || (iflags.terrainmode & TER_MON) != 0)) {
        /* fill in buf[] */
        (void) self_lookat(buf);

        /* file lookup can't distinguish between "gnomish wizard" monster
           and correspondingly named player character, always picking the
           former; force it to find the general "wizard" entry instead */
        if (Role_if(PM_WIZARD) && Race_if(PM_GNOME) && !Upolyd)
            pm = &mons[PM_WIZARD];

        /* When you see yourself normally, no explanation is appended
           (even if you could also see yourself via other means).
           Sensing self while blind or swallowed is treated as if it
           were by normal vision (cf canseeself()). */
        if ((Invisible || u.uundetected) && !Blind
            && !(u.uswallow || iflags.save_uswallow)) {
            unsigned how = 0;

            if (Infravision || Ultravision)
                how |= 1;
            if (Unblind_telepat)
                how |= 2;
            if (Detect_monsters)
                how |= 4;

            if (how) {
                if (Infravision) {
                    Sprintf(eos(buf), " [seen: %s%s%s%s%s]",
                            (how & 1) ? "infravision" : "",
                            /* add comma if telep and infrav */
                            ((how & 3) > 2) ? ", " : "",
                            (how & 2) ? "telepathy" : "",
                            /* add comma if detect and (infrav or telep or both) */
                            ((how & 7) > 4) ? ", " : "",
                            (how & 4) ? "monster detection" : "");
                } else {
                    Sprintf(eos(buf), " [seen: %s%s%s%s%s]",
                            (how & 1) ? "ultravision" : "",
                            /* add comma if telep and infrav */
                            ((how & 3) > 2) ? ", " : "",
                            (how & 2) ? "telepathy" : "",
                            /* add comma if detect and (infrav or telep or both) */
                            ((how & 7) > 4) ? ", " : "",
                            (how & 4) ? "monster detection" : "");
                }
            }
        }
    } else if (u.uswallow) {
        /* when swallowed, we're only called for spots adjacent to hero,
           and blindness doesn't prevent hero from feeling what holds him */
        Sprintf(buf, "interior of %s", a_monnam(u.ustuck));
        pm = u.ustuck->data;
    } else if (glyph_is_monster(glyph)) {
        bhitpos.x = x;
        bhitpos.y = y;
        if ((mtmp = m_at(x, y)) != 0) {
            look_at_monster(buf, monbuf, mtmp, x, y);
            if (!canspotmon(mtmp) && has_erid(mtmp)
                && canspotmon(ERID(mtmp)->mon_steed))
                mtmp = ERID(mtmp)->mon_steed;
            pm = mtmp->data;
        } else if (Hallucination) {
            /* 'monster' must actually be a statue */
            Strcpy(buf, rndmonnam((char *) 0));
        }
    } else if (glyph_is_object(glyph)) {
        look_at_object(buf, x, y, glyph); /* fill in buf[] */
    } else if (glyph_is_trap(glyph)) {
        int tnum = what_trap(glyph_to_trap(glyph), rn2_on_display_rng);

        /* Trap detection displays a bear trap at locations having
         * a trapped door or trapped container or both.
         * TODO: we should create actual trap types for doors and
         * chests so that they can have their own glyphs and tiles.
         */
        if (trapped_chest_at(tnum, x, y))
            Strcpy(buf, "trapped chest"); /* might actually be a large box */
        else if (trapped_door_at(tnum, x, y))
            Strcpy(buf, "trapped door"); /* not "trap door"... */
        else
            Strcpy(buf, defsyms[trap_to_defsym(tnum)].explanation);
    } else if (glyph_is_warning(glyph)) {
        int warnindx = glyph_to_warning(glyph);

        Strcpy(buf, def_warnsyms[warnindx].explanation);
    } else if (!glyph_is_cmap(glyph)) {
        Strcpy(buf, "unexplored area");
    } else {
        int amsk;
        aligntyp algn;

        switch (glyph_to_cmap(glyph)) {
        case S_altar:
            amsk = altarmask_at(x, y);
            algn = Amask2align(amsk & AM_MASK);
            Sprintf(buf, "%s%s %saltar",
                    levl[x][y].frac_altar ? "fractured " : "",
                    /* like endgame high priests, endgame high altars
                       are only recognizable when immediately adjacent */
                    (Is_astralevel(&u.uz) && distu(x, y) > 2)
                        ? "aligned"
                        : align_str(algn),
                    ((amsk & AM_SHRINE) != 0
                     && (Is_astralevel(&u.uz) || Is_sanctum(&u.uz)))
                        ? "high "
                        : "");
            break;
        case S_ndoor:
            if (is_drawbridge_wall(x, y) >= 0)
                Strcpy(buf, "open drawbridge portcullis");
            else if ((levl[x][y].doormask & ~D_TRAPPED) == D_BROKEN)
                Strcpy(buf, "broken door");
            else
                Strcpy(buf, "doorway");
            break;
        case S_cloud:
            Strcpy(buf,
                   Is_airlevel(&u.uz) ? "cloudy area" : "fog/vapor cloud");
            break;
        case S_magic_chest:
            Strcpy(buf, releasep = doname(mchest));
            maybereleaseobuf(releasep);
            break;
        case S_stone:
            if (!levl[x][y].seenv) {
                Strcpy(buf, "unexplored");
                break;
            } else if (Underwater && !Is_waterlevel(&u.uz)) {
                /* "unknown" == previously mapped but not visible when
                   submerged; better terminology appreciated... */
                Strcpy(buf, (distu(x, y) <= 2) ? "land" : "unknown");
                break;
            } else if (levl[x][y].typ == STONE || levl[x][y].typ == SCORR) {
                Strcpy(buf, "stone");
                break;
            }
            /*FALLTHRU*/
        default:
            Strcpy(buf, defsyms[glyph_to_cmap(glyph)].explanation);
            break;
        }
    }
    return (pm && !Hallucination) ? pm : (struct permonst *) 0;
}

STATIC_OVL char *
hallucinatory_armor(buf)
char *buf;
{
#define HARMOR_FMT_HIS_HIS 0
#define HARMOR_FMT_HIS 4
    static const char *harmors[] = {
        "%s heart on %s sleeve",
        /* HARMOR_FMT_HIS_HIS */
        "out %s welcome",
        "%s birthday suit",
        "%s church clothes",
        "%s sunglasses at night",
        /* HARMOR_FMT_HIS */
        "a fursuit",
        "a rented tux",
        "cosplay",
        "a fig leaf",
        "nothing but a smile",
        "a wool coat",
        "fun socks",
        "expensive sneakers",
        "heels",
        "a little black dress",
        "some stylin' threads",
        "a smug look",
        "footie pajamas",
        "hand-me-downs",
        "a sundress",
        "a ballgown",
        "a towel",
        "a trucker hat",
        "a bald cap",
        "all black everything",
        "the emperor's new clothes",
        "rose-colored glasses",
        "blue suede shoes",
        "a raspberry beret",
        "flip flops",
        "bejeweled leather battle shorts",
        "an itsy bitsy teenie weenie yellow polka dot bikini",
        "the pants in the relationship",
        "a lei",
        "a ten gallon hat",
        "a tutu",
        "an ushanka",
        "MOPP level 4 gear",
    };

    int harmor_idx = rn2(SIZE(harmors));
    if (harmor_idx <= HARMOR_FMT_HIS_HIS) {
        const char *his = genders[rn2(3)].his;
        Sprintf(buf, harmors[harmor_idx], his, his);
    } else if (harmor_idx <= HARMOR_FMT_HIS) {
        Sprintf(buf, harmors[harmor_idx], genders[rn2(3)].his);
    } else {
        Strcpy(buf, harmors[harmor_idx]);
    }

    return buf;
}

/* Make sure the order is the same as that defined in monattk.h */
static const char * attacktypes[] = {
    "passive",
    "claw",
    "bite",
    "kick",
    "butt",
    "touch",
    "sting",
    "bearhug",
    "spit",
    "engulf",
    "breathe",
    "explode",
    "explode on death",
    "gaze",
    "tentacle",
    "scream",
    "weapon",
    "spellcast"
};

static const char * damagetypes[] = {
    "physical",
    "magic missile",
    "fire",
    "cold",
    "sleep",
    "disintegration",
    "shock",
    "strength poison",
    "acid",
    "water",
    "level drain",
    "stun",
    NULL, /* AD_SPC1 - not used */
    NULL, /* AD_SPC2 - not used */
    "blind",
    "slow",
    "paralyze",
    "energy drain",
    "wound leg",
    "petrify",
    "sticky",
    "steal gold",
    "steal item",
    "charm",
    "teleport",
    "rust",
    "confuse",
    "digest",
    "heal",
    "drown",
    "lycanthropy",
    "dexterity poison",
    "constitution poison",
    "eat brains",
    "disease",
    "decay",
    "seduce",
    "hallucination",
    "Death special",
    "Pestilence special",
    "Famine special",
    "slime",
    "disenchant",
    "corrode",
    "cancel",
    "behead",
    "affects luck",
    "psionic",
    "sonic",
    "knock-back",
    "polymorph",
    "withering",
    "create pit",
    "web",
    "fire and cold",
    "steal intrinsic",
    "clerical",
    "arcane",
    "random breath",
    "steal Amulet",
};

/* Add some information to an encyclopedia window which is printing information
 * about a monster. */
STATIC_OVL void
add_mon_info(datawin, pm)
winid datawin;
struct permonst * pm;
{
    char buf[BUFSZ];
    char buf2[BUFSZ];
    int i, gen = pm->geno;
    int freq = (gen & G_FREQ);
    int pct = max(5, (int) (pm->cwt / 90));
    int mon_align = (pm->maligntyp == -128) ? A_NONE : sgn(pm->maligntyp);
    boolean uniq = !!(gen & G_UNIQ);
    boolean hell = !!(gen & G_HELL);
    boolean nohell = !!(gen & G_NOHELL);
    boolean nogen = !!(gen & G_NOGEN);
    boolean iceq = is_iceq_only(pm);
    unsigned int mflag3 = pm->mflags3;
    unsigned int mflag4 = pm->mflags4;

#define ADDRESIST(condition, str) \
    if (condition) {              \
        if (*buf)                 \
            Strcat(buf, ", ");    \
        Strcat(buf, str);         \
    }
#define ADDMR(field, res, str)    \
    if (field & (res)) {          \
        if (*buf)                 \
            Strcat(buf, ", ");    \
        Strcat(buf, str);         \
    }
#define APPENDC(cond, str)        \
    if (cond) {                   \
        if (*buf)                 \
            Strcat(buf, ", ");    \
        Strcat(buf, str);         \
    }
#define ADDPCTRES(cond, amt, str)    \
    if (cond) {                      \
        if (*buf)                    \
            Strcat(buf, ", ");       \
        Sprintf(eos(buf), "%d%% %s", \
                amt, str);           \
    }

#define MONPUTSTR(str) putstr(datawin, ATR_NONE, str)

    /* differentiate the two forms of werecreatures */
    Strcpy(buf2, "");
    if (is_were(pm) && pm != &mons[PM_RAT_KING])
        Sprintf(buf2, " (%s form)", pm->mlet == S_HUMAN ? "human" : "animal");

    Sprintf(buf, "Monster lookup for \"%s\"%s:", pm->mname, buf2);
    putstr(datawin, ATR_BOLD, buf);
    MONPUTSTR("");

    /* Misc */
    Sprintf(buf, "Difficulty %d, speed %d, base level %d, base AC %d.",
            pm->difficulty, pm->mmove, pm->mlevel, pm->ac);
    MONPUTSTR(buf);
    Sprintf(buf, "Magic saving throw %d, weight %d, alignment: %s.",
            pm->mr, pm->cwt,
            (mon_align == A_LAWFUL) ? "lawful"
              : (mon_align == A_NEUTRAL) ? "neutral"
                : (mon_align == A_CHAOTIC) ? "chaotic" : "unaligned (evil)");
    MONPUTSTR(buf);

    /* Generation */
    if (uniq)
        Strcpy(buf, "Unique.");
    else if (freq == 0)
	Strcpy(buf, "Not randomly generated.");
    else
        Sprintf(buf, "Normally %s%s, %s.",
                hell ? "only appears in Gehennom" :
                nohell ? "only appears outside Gehennom" :
                nogen ? "only appears specially" :
                iceq ? "only appears in the Ice Queen's realm" :
                "appears in any branch",
                (gen & G_SGROUP) ? " in groups" :
                (gen & G_LGROUP) ? " in large groups" : "",
                freq >= 5 ? "very common" :
                freq == 4 ? "common" :
                freq == 3 ? "slightly rare" :
                freq == 2 ? "rare" : "very rare");
    MONPUTSTR(buf);

    /* Resistances */
    buf[0] = '\0';
    ADDRESIST(pm_resistance(pm, MR_FIRE), "fire");
    ADDRESIST(pm_resistance(pm, MR_COLD), "cold");
    ADDRESIST(pm_resistance(pm, MR_SLEEP), "sleep");
    ADDRESIST(pm_resistance(pm, MR_DISINT), "disintegration");
    ADDRESIST(pm_resistance(pm, MR_ELEC), "shock");
    ADDRESIST(pm_resistance(pm, MR_POISON), "poison");
    ADDRESIST(pm_resistance(pm, MR_ACID), "acid");
    ADDRESIST(pm_resistance(pm, MR_STONE), "petrification");
    ADDRESIST(pm_resistance(pm, MR_PSYCHIC), "psionic attacks");
    ADDRESIST(resists_drain(pm), "life-drain");
    ADDRESIST(ptr_resists_sick(pm), "sickness");
    ADDRESIST(resists_mgc(pm), "magic");
    ADDRESIST(resists_stun(pm), "stun");
    ADDRESIST(resists_slow(pm), "slow");
    ADDRESIST(immune_death_magic(pm), "death magic");
    if (*buf) {
        Sprintf(buf2, "Resists %s.", buf);
        MONPUTSTR(buf2);
    } else {
        MONPUTSTR("Has no resistances.");
    }

    /* Corpse conveyances */
    buf[0] = '\0';
    ADDPCTRES(intrinsic_possible(FIRE_RES, pm), pct, "fire");
    ADDPCTRES(intrinsic_possible(COLD_RES, pm), pct, "cold");
    ADDPCTRES(intrinsic_possible(SHOCK_RES, pm), pct, "shock");
    ADDPCTRES(intrinsic_possible(SLEEP_RES, pm), pct, "sleep");
    ADDPCTRES(intrinsic_possible(POISON_RES, pm), pct, "poison");
    ADDPCTRES(intrinsic_possible(DISINT_RES, pm), pct, "disintegration");
    /* acid, stone, and psionic resistance aren't currently conveyable */
    if (*buf)
        Strcat(buf, " resistance");
    APPENDC(intrinsic_possible(TELEPORT, pm), "teleportation");
    APPENDC(intrinsic_possible(TELEPORT_CONTROL, pm), "teleport control");
    APPENDC(intrinsic_possible(TELEPAT, pm), "telepathy");
    /* There are a bunch of things that happen in cpostfx (levels for wraiths,
     * stunning for bats...) but only count the ones that actually behave like
     * permanent intrinsic gains.
     * If you find yourself listing multiple things here for the same effect,
     * that may indicate the property should be added to psuedo_intrinsics. */
    APPENDC(pm == &mons[PM_QUANTUM_MECHANIC], "speed or slowness");
    APPENDC(pm == &mons[PM_MIND_FLAYER] || pm == &mons[PM_MASTER_MIND_FLAYER],
            "intelligence");
    if (is_were(pm)) {
        /* Weres need a bit of special handling, since 1) you always get
         * lycanthropy so "may convey" could imply the player might not contract
         * it; 2) the animal forms are flagged as G_NOCORPSE, but still have a
         * meaningless listed corpse nutrition value which shouldn't print. */
        if (pm->mlet == S_HUMAN) {
            Sprintf(buf2, "Provides %d nutrition when eaten.", pm->cnutrit);
            MONPUTSTR(buf2);
        }
        MONPUTSTR("Corpse conveys lycanthropy.");
    } else if (pm->mlet == S_PUDDING) {
        if (*buf) {
            Sprintf(buf2, "May drop globs that convey %s.", buf);
            MONPUTSTR(buf2);
        } else {
            MONPUTSTR("May drop globs.");
        }
    } else if (!(gen & G_NOCORPSE)) {
        Sprintf(buf2, "Provides %d nutrition when eaten.", pm->cnutrit);
        MONPUTSTR(buf2);
        if (*buf) {
            Sprintf(buf2, "Corpse conveys %s.", buf);
            MONPUTSTR(buf2);
        } else
            MONPUTSTR("Corpse conveys no intrinsics.");
    } else
        MONPUTSTR("Leaves no corpse.");

    /* Flag descriptions */
    buf[0] = '\0';
    APPENDC(is_male(pm), "male");
    APPENDC(is_female(pm), "female");
    APPENDC(is_neuter(pm), "genderless");
    APPENDC(pm->msize == MZ_TINY, "tiny");
    APPENDC(pm->msize == MZ_SMALL, "small");
    APPENDC(pm->msize == MZ_LARGE, "large");
    APPENDC(pm->msize == MZ_HUGE, "huge");
    APPENDC(pm->msize == MZ_GIGANTIC, "gigantic");
    if (!(*buf)) {
        /* for nonstandard sizes */
        if (verysmall(pm)) {
            APPENDC(TRUE, "small");
        } else if (bigmonst(pm)) {
            APPENDC(TRUE, "big");
        }
    }

    /* inherent characteristics: "Monster is X." */
    APPENDC(!(gen & G_GENO), "ungenocideable");
    APPENDC(breathless(pm), "breathless");
    if (!breathless(pm))
        APPENDC(amphibious(pm), "amphibious");
    APPENDC(amorphous(pm), "amorphous");
    APPENDC(noncorporeal(pm), "incorporeal");
    if (!noncorporeal(pm))
        APPENDC(unsolid(pm), "unsolid");
    APPENDC(acidic(pm), "acidic");
    APPENDC(poisonous(pm), "poisonous");
    APPENDC(regenerates(pm), "regenerating");
    APPENDC(is_floater(pm), "floating");
    ADDRESIST(pm_resistance(pm, MR2_LEVITATE), "floating");
    APPENDC(pm_invisible(pm), "invisible");
    APPENDC(is_undead(pm), "undead");
    if (!is_undead(pm))
        APPENDC(nonliving(pm), "nonliving");
    APPENDC(mindless(pm), "mindless");
    APPENDC(telepathic(pm), "telepathic");
    ADDRESIST(pm_resistance(pm, MR2_TELEPATHY), "telepathic");
    APPENDC(is_displaced(pm), "displaced");
    ADDRESIST(pm_resistance(pm, MR2_DISPLACED), "displaced");
    APPENDC(strongmonst(pm), "strong");
    APPENDC(is_skittish(pm), "skittish");
    APPENDC(is_accurate(pm), "accurate");
    APPENDC(is_stationary(pm), "stationary");
    APPENDC(infravisible(pm), "infravisible");
    APPENDC(carnivorous(pm), "carnivorous");
    APPENDC(herbivorous(pm), "herbivorous");
    APPENDC(metallivorous(pm), "metallivorous");
    APPENDC(inediate(pm), "inediate");
    APPENDC(is_covetous(pm), "covetous");
    APPENDC(hates_material(pm, IRON), "harmed by iron")
    APPENDC(hates_material(pm, MITHRIL), "harmed by mithril")
    APPENDC(hates_material(pm, SILVER), "harmed by silver")
    APPENDC((mflag4 & M4_VULNERABLE_FIRE) != 0, "vulnerable to fire");
    APPENDC((mflag4 & M4_VULNERABLE_COLD) != 0, "vulnerable to cold");
    APPENDC((mflag4 & M4_VULNERABLE_ELEC) != 0, "vulnerable to electricity");
    APPENDC((mflag4 & M4_VULNERABLE_ACID) != 0, "vulnerable to acid");
    if (*buf) {
        Sprintf(buf2, "Is %s.", buf);
        MONPUTSTR(buf2);
        buf[0] = '\0';
    }

    /* "Monster wants X." */
    APPENDC((mflag3 & M3_WANTSAMUL) != 0, "the Amulet of Yendor");
    APPENDC((mflag3 & M3_WANTSBELL) != 0, "the Bell of Opening");
    APPENDC((mflag3 & M3_WANTSBOOK) != 0, "the Book of the Dead");
    APPENDC((mflag3 & M3_WANTSCAND) != 0, "the Candelabrum");
    APPENDC((mflag3 & M3_WANTSARTI) != 0, "quest artifacts");
    if (*buf) {
        Sprintf(buf2, "Wants %s.", buf);
        MONPUTSTR(buf2);
        buf[0] = '\0';
    }

    /* inherent abilities: "Monster can X." */
    APPENDC(perceives(pm), "see invisible");
    APPENDC(hides_under(pm), "hide under objects");
    if (!hides_under(pm))
        APPENDC(is_hider(pm), "hide");
    APPENDC(is_swimmer(pm), "swim");
    if (!is_floater(pm))
        APPENDC(is_flyer(pm), "fly");
    APPENDC(passes_walls(pm), "phase through walls");
    APPENDC(can_teleport(pm), "teleport");
    APPENDC(is_clinger(pm), "cling to the ceiling");
    APPENDC(is_jumper(pm), "jump");
    ADDRESIST(pm_resistance(pm, MR2_JUMPING), "jump");
    ADDRESIST(pm_resistance(pm, MR2_WATERWALK), "walk on water");
    APPENDC(lays_eggs(pm), "lay eggs");
    APPENDC(webmaker(pm), "spin webs");
    APPENDC(needspick(pm), "mine");
    APPENDC(is_berserker(pm), "go berserk");
    APPENDC(is_support(pm), "supports allies")
    APPENDC(can_flollop(pm), "flollop");
    APPENDC(is_reviver(pm), "revive");
    if (!needspick(pm))
        APPENDC(tunnels(pm), "dig");
    if (*buf) {
        Sprintf(buf2, "Can %s.", buf);
        MONPUTSTR(buf2);
        buf[0] = '\0';
    }

    /* "Monster can't X." */
    APPENDC(non_tameable(pm), "be tamed");
    APPENDC(no_geno_vecna(pm), "be genocided until its leader is destroyed");
    APPENDC(no_geno_vlad(pm), "be genocided until its leader is destroyed");
    APPENDC(no_geno_talgath(pm), "be genocided until its leader is killed");
    APPENDC(is_defeated(pm), "be killed");
    APPENDC(hates_light(pm), "stand the light");
    if (*buf) {
        Sprintf(buf2, "Can't %s.", buf);
        MONPUTSTR(buf2);
        buf[0] = '\0';
    }

    /* Full-line remarks */
    if (touch_petrifies(pm))
        MONPUTSTR("Petrifies by touch.");
    if (infravision(pm))
        MONPUTSTR("Has infravision.");
    if (ultravision(pm))
        MONPUTSTR("Has ultravision.");
    if (thick_skinned(pm))
        MONPUTSTR("Has a thick hide.");
    if (control_teleport(pm))
        MONPUTSTR("Has teleport control.");
    if (your_race(pm))
        MONPUTSTR("Is the same race as you.");
    if (!(gen & G_NOCORPSE)) {
        if (vegan(pm))
            MONPUTSTR("May be eaten by vegans.");
        else if (vegetarian(pm))
            MONPUTSTR("May be eaten by vegetarians.");
        else if (has_blood(pm))
            MONPUTSTR("May be fed upon by vampires.");
    }
    if (emits_light(pm)) {
        if (pm == &mons[PM_SHADOW_DRAGON]
            || pm == &mons[PM_BABY_SHADOW_DRAGON])
            MONPUTSTR("Emits darkness.");
        else
            MONPUTSTR("Emits light.");
    }
    Sprintf(buf, "Is %sa valid polymorph form.",
            polyok(pm) ? "" : "not ");
    MONPUTSTR(buf);

    /* Attacks */
    buf[0] = buf2[0] = '\0';
    for (i = 0; i < 6; i++) {
        char dicebuf[20]; /* should be a safe limit */
        struct attack * attk = &(pm->mattk[i]);
        if (attk->damn) {
            Sprintf(dicebuf, "%dd%d", attk->damn, attk->damd);
        } else if (attk->damd) {
            Sprintf(dicebuf, "(level + 1)d%d", attk->damd);
        } else {
            if (!attk->aatyp && !attk->adtyp) {
                /* no attack in this slot */
                continue;
            } else {
                /* real attack, but 0d0 damage */
                dicebuf[0] = '\0';
            }
        }
        if (attk->aatyp > LAST_AT) {
            impossible("add_to_mon: unknown attack type %d", attk->aatyp);
        } else if (attk->adtyp > LAST_AD) {
            impossible("add_to_mon: unknown damage type %d", attk->adtyp);
        /* hack to display gelatinous cubes' (and potentially shambling
           horrors') suffocation attack correctly---swimming won't save you! */
        } else if (attk->aatyp == AT_ENGL && attk->adtyp == AD_WRAP
                   && !(pm == &mons[PM_SEA_DRAGON]
                        || pm == &mons[PM_WATER_ELEMENTAL])) {
            Sprintf(buf2, "%s%s%s %s", dicebuf, ((*dicebuf) ? " " : ""),
                    attacktypes[attk->aatyp], "suffocate");
            APPENDC(TRUE, buf2);
        } else {
            Sprintf(buf2, "%s%s%s %s", dicebuf, ((*dicebuf) ? " " : ""),
                    attacktypes[attk->aatyp], damagetypes[attk->adtyp]);
            APPENDC(TRUE, buf2);
        }
    }
    if (*buf) {
        Sprintf(buf2, "Attacks: %s", buf);
        MONPUTSTR(buf2);
    } else
        MONPUTSTR("Has no attacks.");
}
#undef ADDPROP
#undef ADDMR
#undef APPENDC
#undef MONPUTSTR

extern const struct propname {
    int prop_num;
    const char* prop_name;
} propertynames[]; /* located in timeout.c */

/* Add some information to an encyclopedia window which is printing information
 * about an object. */
STATIC_OVL void
add_obj_info(datawin, otyp)
winid datawin;
short otyp;
{
    struct objclass oc = objects[otyp];
    char olet = oc.oc_class;
    char buf[BUFSZ];
    char buf2[BUFSZ];
    boolean weptool = (olet == TOOL_CLASS && oc.oc_skill != P_NONE);
    const char* dir = (oc.oc_dir == NODIR ? "Non-directional"
                                          : (oc.oc_dir == IMMEDIATE ? "Beam"
                                                                    : "Ray"));

#define OBJPUTSTR(str) putstr(datawin, ATR_NONE, str)
#define ADDCLASSPROP(cond, str)          \
    if (cond) {                          \
        if (*buf) { Strcat(buf, ", "); } \
        Strcat(buf, str);                \
    }

    Sprintf(buf, "Object lookup for \"%s\":", safe_typename(otyp));
    putstr(datawin, ATR_BOLD, buf);
    OBJPUTSTR("");

    /* Object classes currently with no special messages here: amulets. */
    if (olet == WEAPON_CLASS || weptool) {
        const int skill = oc.oc_skill;
        const char* dmgtyp = "blunt";
        const char* sdambon = "";
        const char* ldambon = "";

        if (skill >= 0) {
            Sprintf(buf, "%s-handed weapon%s using the %s skill.",
                    (oc.oc_bimanual ? "Two" : "Single"),
                    (weptool ? "-tool" : ""),
                    skill_name(skill));
        } else if (skill <= -P_BOW && oc.oc_skill >= -P_CROSSBOW) {
            /* Minor assumption: the skill name will be the same as the launcher
             * itself. Currently this is only bow and crossbow. */
            Sprintf(buf, "Ammunition meant to be fired from a %s.",
                    skill_name(-skill));
        } else {
            Sprintf(buf, "Thrown missile using the %s skill.",
                    skill_name(-skill));
        }
        OBJPUTSTR(buf);

        if (oc.oc_dir & PIERCE) {
            dmgtyp = "piercing";
            if (oc.oc_dir & SLASH) {
                dmgtyp = "piercing/slashing";
            }
        } else if (oc.oc_dir & SLASH) {
            dmgtyp = "slashing";
        }
        Sprintf(buf, "Deals %s damage.", dmgtyp);
        OBJPUTSTR(buf);

        /* Ugh. Can we just get rid of dmgval() and put its damage bonuses into
         * the object class? */
        switch (otyp) {
        case IRON_CHAIN:
        case CROSSBOW_BOLT:
        case DARK_ELVEN_CROSSBOW_BOLT:
        case MACE:
        case DARK_ELVEN_MACE:
        case HEAVY_MACE:
        case DARK_ELVEN_HEAVY_MACE:
        case ROD:
        case WAR_HAMMER:
        case HEAVY_WAR_HAMMER:
        case FLAIL:
        case TRIPLE_HEADED_FLAIL:
        case SPETUM:
        case TRIDENT:
            sdambon = " + 1";
            break;
        case BATTLE_AXE:
        case BARDICHE:
        case BILL_GUISARME:
        case GUISARME:
        case LUCERN_HAMMER:
        case MORNING_STAR:
        case ORCISH_MORNING_STAR:
        case RANSEUR:
        case BROADSWORD:
        case ELVEN_BROADSWORD:
        case DARK_ELVEN_BROADSWORD:
        case RUNESWORD:
        case VOULGE:
            sdambon = " + 1d4";
            break;
        }
        /* and again, because /large/ damage is entirely separate. Bleah. */
        switch (otyp) {
        case CROSSBOW_BOLT:
        case DARK_ELVEN_CROSSBOW_BOLT:
        case MORNING_STAR:
        case ORCISH_MORNING_STAR:
        case PARTISAN:
        case RUNESWORD:
        case ELVEN_BROADSWORD:
        case DARK_ELVEN_BROADSWORD:
        case BROADSWORD:
            ldambon = " + 1";
            break;
        case FLAIL:
        case RANSEUR:
        case VOULGE:
            ldambon = " + 1d4";
            break;
        case HALBERD:
        case SPETUM:
            ldambon = " + 1d6";
            break;
        case BATTLE_AXE:
        case BARDICHE:
        case TRIDENT:
            ldambon = " + 2d4";
            break;
        case TSURUGI:
        case DWARVISH_MATTOCK:
        case TWO_HANDED_SWORD:
            ldambon = " + 2d6";
            break;
        case TRIPLE_HEADED_FLAIL:
            ldambon = " + 3d6";
            break;
        }
        Sprintf(buf,
               "Damage: 1d%d%s versus small and 1d%d%s versus large monsters.",
                oc.oc_wsdam, sdambon, oc.oc_wldam, ldambon);
        OBJPUTSTR(buf);
        Sprintf(buf, "Has a %s%d %s to hit.", (oc.oc_hitbon >= 0 ? "+" : ""),
                oc.oc_hitbon, (oc.oc_hitbon >= 0 ? "bonus" : "penalty"));
        OBJPUTSTR(buf);
    }
    if (olet == ARMOR_CLASS) {
        /* Indexes here correspond to ARM_SHIELD, etc; not the W_* masks.
         * Expects ARM_SUIT = 0, all the way up to ARM_SHIRT = 6. */
        const char* armorslots[] = {
            "torso", "shield", "helm", "gloves", "boots", "cloak", "shirt"
        };
        Sprintf(buf, "%s, worn in the %s slot.",
                (oc.oc_bulky ? "Bulky armor" : "Armor"),
                armorslots[oc.oc_armcat]);

        OBJPUTSTR(buf);
        Sprintf(buf, "Base AC %d, magic cancellation %d.",
                oc.a_ac, oc.a_can);
        OBJPUTSTR(buf);
        Sprintf(buf, "Takes %d turn%s to put on or remove.",
                oc.oc_delay, (oc.oc_delay == 1 ? "" : "s"));
    }
    if (olet == FOOD_CLASS) {
        if (otyp == TIN || otyp == CORPSE) {
            OBJPUTSTR("Comestible providing varied nutrition.");
            OBJPUTSTR("Takes various amounts of turns to eat.");
            OBJPUTSTR("May or may not be vegetarian.");
        } else {
            Sprintf(buf, "Comestible providing %d nutrition.", oc.oc_nutrition);
            OBJPUTSTR(buf);
            Sprintf(buf, "Takes %d turn%s to eat.", oc.oc_delay,
                    (oc.oc_delay == 1 ? "" : "s"));
            OBJPUTSTR(buf);
            /* TODO: put special-case VEGGY foods in a list which can be
             * referenced by doeat(), so there's no second source for this. */
            if (oc.oc_material == FLESH && otyp != EGG) {
                OBJPUTSTR("Is not vegetarian.");
            } else {
                /* is either VEGGY food or egg */
                switch (otyp) {
                case PANCAKE:
                case FORTUNE_COOKIE:
                case EGG:
                case CREAM_PIE:
                case CANDY_BAR:
                case LUMP_OF_ROYAL_JELLY:
                    OBJPUTSTR("Is vegetarian but not vegan.");
                    break;
                default:
                    OBJPUTSTR("Is vegan.");
                }
            }
        }
    }
    if (olet == POTION_CLASS) {
        /* nothing special */
        OBJPUTSTR("Potion.");
    }
    if (olet == SCROLL_CLASS) {
        /* nothing special (ink is covered below) */
        OBJPUTSTR("Scroll.");
    }
    if (olet == SPBOOK_CLASS) {
        if (otyp == SPE_BLANK_PAPER) {
            OBJPUTSTR("Spellbook.");
        } else if (otyp == SPE_NOVEL || otyp == SPE_BOOK_OF_THE_DEAD) {
            OBJPUTSTR("Book.");
        } else {
            Sprintf(buf, "Level %d spellbook, in the %s school. %s spell.",
                    oc.oc_level, spelltypemnemonic(oc.oc_skill), dir);
            OBJPUTSTR(buf);
            Sprintf(buf, "Takes %d actions to read.", oc.oc_delay);
            OBJPUTSTR(buf);
        }
    }
    if (olet == WAND_CLASS) {
        Sprintf(buf, "%s wand.", dir);
        OBJPUTSTR(buf);
    }
    if (olet == RING_CLASS) {
        OBJPUTSTR(oc.oc_charged ? "Chargeable ring." : "Ring.");
        /* see material comment below; only show toughness status if this
         * particular ring is already identified... */
        if (oc.oc_tough && oc.oc_name_known) {
            OBJPUTSTR("Is made of a hard material.");
        }
    }
    if (olet == GEM_CLASS) {
        if (oc.oc_material == MINERAL) {
            OBJPUTSTR("Type of stone.");
        } else if (oc.oc_material == GLASS) {
            OBJPUTSTR("Piece of colored glass.");
        } else {
            OBJPUTSTR("Precious gem.");
        }
        /* can do unconditionally, these aren't randomized */
        if (oc.oc_tough) {
            OBJPUTSTR("Is made of a hard material.");
        }
    }
    if (olet == TOOL_CLASS && !weptool) {
        const char* subclass = "tool";
        switch (otyp) {
        case LARGE_BOX:
        case CHEST:
        case ICE_BOX:
        case IRON_SAFE:
        case CRYSTAL_CHEST:
        case HIDDEN_CHEST:
        case SACK:
        case OILSKIN_SACK:
        case BAG_OF_HOLDING:
            subclass = "container";
            break;
        case SKELETON_KEY:
        case LOCK_PICK:
        case CREDIT_CARD:
        case MAGIC_KEY:
            subclass = "unlocking tool";
            break;
        case TALLOW_CANDLE:
        case WAX_CANDLE:
        case LANTERN:
        case OIL_LAMP:
        case MAGIC_LAMP:
            subclass = "light source";
            break;
        case LAND_MINE:
        case BEARTRAP:
            subclass = "trap which can be set";
            break;
        case PEA_WHISTLE:
        case MAGIC_WHISTLE:
        case BELL:
        case LEATHER_DRUM:
        case DRUM_OF_EARTHQUAKE:
            subclass = "atonal instrument";
            break;
        case BUGLE:
        case MAGIC_FLUTE:
        case PAN_FLUTE:
        case FLUTE:
        case TOOLED_HORN:
        case FIRE_HORN:
        case FROST_HORN:
        case HARP:
        case MAGIC_HARP:
            subclass = "tonal instrument";
            break;
        case BLACKSMITH_HAMMER:
            subclass = "forging tool";
            break;
        }
        Sprintf(buf, "%s%s.", (oc.oc_charged ? "chargeable " : ""), subclass);
        /* capitalize first letter of buf */
        buf[0] -= ('a' - 'A');
        OBJPUTSTR(buf);
    }

    /* cost, wt should go next */
    Sprintf(buf, "Base cost %d, weighs %d aum.", oc.oc_cost, oc.oc_weight);
    OBJPUTSTR(buf);

    /* Scrolls or spellbooks: ink cost */
    if (olet == SCROLL_CLASS || olet == SPBOOK_CLASS) {
        if (otyp == SCR_BLANK_PAPER || otyp == SPE_BLANK_PAPER) {
            OBJPUTSTR("Can be written on.");
        } else if (otyp == SPE_NOVEL || otyp == SPE_BOOK_OF_THE_DEAD) {
            OBJPUTSTR("Cannot be written.");
        } else {
            Sprintf(buf, "Takes %d to %d ink to write.",
                    ink_cost(otyp)/2, ink_cost(otyp)-1);
            OBJPUTSTR(buf);
        }
    }

    /* power conferred */
    if (oc.oc_oprop) {
        int i;
        for (i = 0; propertynames[i].prop_name; ++i) {
            /* hack for alchemy smocks because everything about alchemy smocks
             * is a hack */
            if (propertynames[i].prop_num == ACID_RES
                && otyp == ALCHEMY_SMOCK) {
                OBJPUTSTR("Confers acid resistance.");
                continue;
            }
            if (oc.oc_oprop == propertynames[i].prop_num) {
                /* proper grammar */
                const char* confers = "Makes you";
                const char* effect = propertynames[i].prop_name;
                switch (propertynames[i].prop_num) {
                    /* special overrides because prop_name is bad */
                    case STRANGLED:
                        effect = "choke";
                        break;
                    case LIFESAVED:
                        effect = "life saving";
                        /* FALLTHRU */
                    /* for things that don't work with "Makes you" */
                    case GLIB:
                    case WOUNDED_LEGS:
                    case DETECT_MONSTERS:
                    case SEE_INVIS:
                    case HUNGER:
                    case WARNING:
                    /* don't do special warn_of_mon */
                    case SEARCHING:
                    case INFRAVISION:
                    case ULTRAVISION:
                    case AGGRAVATE_MONSTER:
                    case CONFLICT:
                    case JUMPING:
                    case TELEPORT_CONTROL:
                    case SWIMMING:
                    case SLOW_DIGESTION:
                    case HALF_SPDAM:
                    case HALF_PHDAM:
                    case REGENERATION:
                    case ENERGY_REGENERATION:
                    case PROTECTION:
                    case PROT_FROM_SHAPE_CHANGERS:
                    case POLYMORPH_CONTROL:
                    case FREE_ACTION:
                    case FIXED_ABIL:
                    case MAGICAL_BREATHING:
                    case PASSES_WALLS:
                    case PASSES_TREES:
                        confers = "Confers";
                        break;
                    default:
                        break;
                }
                if (strstri(propertynames[i].prop_name, "resistance"))
                    confers = "Confers";
                Sprintf(buf, "%s %s.", confers, effect);
                OBJPUTSTR(buf);
            }
        }
    }

    buf[0] = '\0';
    ADDCLASSPROP(oc.oc_magic, "inherently magical");
    ADDCLASSPROP(oc.oc_nowish, "not wishable");
    if (*buf) {
        Sprintf(buf2, "Is %s.", buf);
        OBJPUTSTR(buf2);
    }

    /* Material.
     * Note that we should not show the material of certain objects if they are
     * subject to description shuffling that includes materials. If the player
     * has already discovered this object, though, then it's fine to show the
     * material.
     * Object classes where this may matter: rings, wands. All randomized tools
     * share materials, and all scrolls and potions are the same material. */
    if (!(olet == RING_CLASS || olet == WAND_CLASS) || oc.oc_name_known) {
        /* char array converting materials to strings; if this is ever needed
        * anywhere else it should be externified. Corresponds exactly to the
        * materials defined in objclass.h.
        * This is very similar to materialnm[], but the slight difference is
        * that this is always the noun form whereas materialnm uses adjective
        * forms; most materials have the same noun and adjective forms but two
        * (wood/wooden, vegetable matter/organic) don't */
        const char* mat_str = materialnm[oc.oc_material];
        /* Two exceptions to materialnm, which uses adjectival forms: most of
         * these work fine as nouns but two don't. */
        if (oc.oc_material == WOOD) {
            mat_str = "wood";
        } else if (oc.oc_material == VEGGY) {
            mat_str = "vegetable matter";
        }

        Sprintf(buf, "Normally made of %s.", mat_str);
        OBJPUTSTR(buf);
    }

    /* TODO: prevent obj lookup from displaying with monster database entry
     * (e.g. scroll of light gives "light" monster database) */

    /* Full-line remarks */
    if (oc.oc_merge) {
        OBJPUTSTR("Merges with identical items.");
    }
    if (oc.oc_unique) {
        OBJPUTSTR("Unique item.");
    }

    /* forge recipes */
    const struct forge_recipe *recipe;
    boolean has_recipes = FALSE;

    for (recipe = fusions; recipe->result_typ; recipe++) {
        if (otyp == recipe->typ1 || otyp == recipe->typ2
            || otyp == recipe->result_typ) {
            if (!has_recipes) {
                OBJPUTSTR("");
                OBJPUTSTR("Forging recipes (#forge):");
                has_recipes = TRUE;
            }
            Sprintf(buf, "  %ld %s + %ld %s = %s", recipe->quan_typ1,
                    OBJ_NAME(objects[recipe->typ1]), recipe->quan_typ2,
                    OBJ_NAME(objects[recipe->typ2]),
                    OBJ_NAME(objects[recipe->result_typ]));
            OBJPUTSTR(buf);
        }
    }

    /* potion alchemy */
    const struct potion_alchemy *precipe;
    boolean has_precipes = FALSE;

    for (precipe = potion_fusions; precipe->result_typ; precipe++) {
        if (otyp == precipe->typ1 || otyp == precipe->typ2
            || otyp == precipe->result_typ) {
            if (!has_precipes) {
                OBJPUTSTR("");
                OBJPUTSTR("Potion alchemy combinations (#dip):");
                has_precipes = TRUE;
            }
            Sprintf(buf, "  %s + %s = %s%s",
                    OBJ_NAME(objects[precipe->typ1]),
                    OBJ_NAME(objects[precipe->typ2]),
                    OBJ_NAME(objects[precipe->result_typ]),
                    precipe->chance == 1 ? "" : " (1/3 chance)");
            OBJPUTSTR(buf);
        }
    }

    /* crafting traps */
    const struct trap_recipe *trecipe;
    boolean has_trecipes = FALSE;

    for (trecipe = trap_fusions; trecipe->result_typ; trecipe++) {
        if (otyp == trecipe->comp || otyp == trecipe->result_typ) {
            if (!has_trecipes) {
                OBJPUTSTR("");
                OBJPUTSTR("Trap crafting recipes (using trap kit):");
                has_trecipes = TRUE;
            }
            Sprintf(buf, "  trap kit + %ld %s%s = %s", trecipe->quan,
                    (objects[trecipe->comp].oc_class == WAND_CLASS
                     ? "wand of " : objects[trecipe->comp].oc_class == POTION_CLASS
                       ? "potion of " : ""),
                    OBJ_NAME(objects[trecipe->comp]),
                    OBJ_NAME(objects[trecipe->result_typ]));
            OBJPUTSTR(buf);
        }
    }
}

/*
 * Look in the "data" file for more info.  Called if the user typed in the
 * whole name (user_typed_name == TRUE), or we've found a possible match
 * with a character/glyph and flags.help is TRUE.
 *
 * NOTE: when (user_typed_name == FALSE), inp is considered read-only and
 *       must not be changed directly, e.g. via lcase(). We want to force
 *       lcase() for data.base lookup so that we can have a clean key.
 *       Therefore, we create a copy of inp _just_ for data.base lookup.
 */
void
checkfile(inp, pm, user_typed_name, without_asking, supplemental_name)
char *inp;
struct permonst *pm; /* only set if looking at a monster with / */
boolean user_typed_name, without_asking;
char *supplemental_name;
{
    dlb *fp;
    char buf[BUFSZ], newstr[BUFSZ], givenname[BUFSZ];
    char *ep, *dbase_str, *dbase_str_with_material;
    unsigned long txt_offset = 0L;
    winid datawin = WIN_ERR;
    short otyp, mat;
    boolean lookat_mon = (pm != (struct permonst *) 0);

    fp = dlb_fopen(DATAFILE, "r");
    if (!fp) {
        pline("Cannot open 'data' file!");
        return;
    }
    /* If someone passed us garbage, prevent fault. */
    if (!inp || strlen(inp) > (BUFSZ - 1)) {
        impossible("bad do_look buffer passed (%s)!",
                   !inp ? "null" : "too long");
        goto checkfile_done;
    }

    /* To prevent the need for entries in data.base like *ngel to account
     * for Angel and angel, make the lookup string the same for both
     * user_typed_name and picked name.
     */
    if (pm != (struct permonst *) 0 && !user_typed_name) {
        if (is_rider(pm))
            dbase_str = strcpy(newstr, "Rider");
        else
            dbase_str = strcpy(newstr, pm->mname);
    } else {
        dbase_str = strcpy(newstr, inp);
    }
    (void) lcase(dbase_str);

    /*
     * TODO:
     * The switch from xname() to doname_vague_quan() in look_at_obj()
     * had the unintendded side-effect of making names picked from
     * pointing at map objects become harder to simplify for lookup.
     * We should split the prefix and suffix handling used by wish
     * parsing and also wizmode monster generation out into separate
     * routines and use those routines here.  This currently lacks
     * erosion handling and probably lots of other bits and pieces
     * that wishing already understands and most of this duplicates
     * stuff already done for wish handling or monster generation.
     */
    if (!strncmp(dbase_str, "interior of ", 12))
        dbase_str += 12;
    if (!strncmp(dbase_str, "a ", 2))
        dbase_str += 2;
    else if (!strncmp(dbase_str, "an ", 3))
        dbase_str += 3;
    else if (!strncmp(dbase_str, "the ", 4))
        dbase_str += 4;
    else if (!strncmp(dbase_str, "some ", 5))
        dbase_str += 5;
    else if (digit(*dbase_str)) {
        /* remove count prefix ("2 ya") which can come from looking at map */
        while (digit(*dbase_str))
            ++dbase_str;
        if (*dbase_str == ' ')
            ++dbase_str;
    }
    if (!strncmp(dbase_str, "pair of ", 8))
        dbase_str += 8;
    if (!strncmp(dbase_str, "tame ", 5))
        dbase_str += 5;
    else if (!strncmp(dbase_str, "peaceful ", 9))
        dbase_str += 9;
    if (!strncmp(dbase_str, "invisible ", 10))
        dbase_str += 10;
    if (!strncmp(dbase_str, "saddled ", 8))
        dbase_str += 8;
    if (!strncmp(dbase_str, "barded ", 7))
        dbase_str += 7;
    if (!strncmp(dbase_str, "blessed ", 8))
        dbase_str += 8;
    else if (!strncmp(dbase_str, "uncursed ", 9))
        dbase_str += 9;
    else if (!strncmp(dbase_str, "cursed ", 7))
        dbase_str += 7;
    if (!strncmp(dbase_str, "empty ", 6))
        dbase_str += 6;
    if (!strncmp(dbase_str, "partly used ", 12))
        dbase_str += 12;
    else if (!strncmp(dbase_str, "partly eaten ", 13))
        dbase_str += 13;
    if (!strncmp(dbase_str, "statue of ", 10))
        dbase_str[6] = '\0';
    else if (!strncmp(dbase_str, "figurine of ", 12))
        dbase_str[8] = '\0';
    /* remove enchantment ("+0 aklys"); [for 3.6.0 and earlier, this wasn't
       needed because looking at items on the map used xname() rather than
       doname() hence known enchantment was implicitly suppressed] */
    if (*dbase_str && index("+-", dbase_str[0]) && digit(dbase_str[1])) {
        ++dbase_str; /* skip sign */
        while (digit(*dbase_str))
            ++dbase_str;
        if (*dbase_str == ' ')
            ++dbase_str;
    }
    /* "towel", "wet towel", and "moist towel" share one data.base entry;
       for "wet towel", we keep prefix so that the prompt will ask about
       "wet towel"; for "moist towel", we also want to ask about "wet towel".
       (note: strncpy() only terminates output string if the specified
       count is bigger than the length of the substring being copied) */
    if (!strncmp(dbase_str, "moist towel", 11))
        memcpy(dbase_str += 2, "wet", 3); /* skip "mo" replace "ist" */

    /* Remove material, if it exists, but store the original value in
     * dbase_str_with_material. It should be cut out of dbase_str as a prefix
     * in order for the alt handling below to function properly.
     * Note that this assumes that material is always the last thing that needs
     * to be stripped out (e.g. it will not strip things out if dbase_str is
     * "silver +0 sword").
     * This is clunky in how it adds a third possibility that needs to be
     * pmatch()'ed; a better future refactor of this code would be to collect an
     * arbitrary number of possible strings as needed, then iterate through
     * them. */
    dbase_str_with_material = dbase_str;
    for (mat = 1; mat < NUM_MATERIAL_TYPES; ++mat) {
        unsigned int len = strlen(materialnm[mat]);
        /* check for e.g. "gold " without constructing it as a string */
        if (!strncmp(dbase_str, materialnm[mat], len) && strlen(dbase_str) > len
            && dbase_str[len] == ' ') {
            dbase_str_with_material = dbase_str;
            dbase_str += len + 1;
            break;
        }
    }

    /* Make sure the name is non-empty. */
    if (*dbase_str) {
        long pass1offset = -1L;
        int chk_skip, pass = 1;
        boolean yes_to_moreinfo, found_in_file, pass1found_in_file,
                skipping_entry;
        char *sp, *ap, *alt = 0; /* alternate description */
        char *encycl_matched = 0; /* which version of the string matched
                                     (for later printing) */
        char matcher[BUFSZ];      /* the string it matched against */

        /* adjust the input to remove "named " and "called " */
        if ((ep = strstri(dbase_str, " named ")) != 0) {
            alt = ep + 7;
            if ((ap = strstri(dbase_str, " called ")) != 0 && ap < ep)
                ep = ap; /* "named" is alt but truncate at "called" */
        } else if ((ep = strstri(dbase_str, " called ")) != 0) {
            copynchars(givenname, ep + 8, BUFSZ - 1);
            alt = givenname;
            if (supplemental_name && (sp = strstri(inp, " called ")) != 0)
                copynchars(supplemental_name, sp + 8, BUFSZ - 1);
        } else
            ep = strstri(dbase_str, ", ");
        if (ep && ep > dbase_str)
            *ep = '\0';
        /* remove article from 'alt' name ("a pair of lenses named
           The Eyes of the Overworld" simplified above to "lenses named
           The Eyes of the Overworld", now reduced to "The Eyes of the
           Overworld", skip "The" as with base name processing) */
        if (alt && (!strncmpi(alt, "a ", 2)
                    || !strncmpi(alt, "an ", 3)
                    || !strncmpi(alt, "the ", 4)))
            alt = index(alt, ' ') + 1;
        /* remove charges or "(lit)" or wizmode "(N aum)" */
        if ((ep = strstri(dbase_str, " (")) != 0 && ep > dbase_str)
            *ep = '\0';
        if (alt && (ap = strstri(alt, " (")) != 0 && ap > alt)
            *ap = '\0';

        /*
         * If the object is named, then the name is the alternate description;
         * otherwise, the result of makesingular() applied to the name is.
         * This isn't strictly optimal, but named objects of interest to the
         * user will usually be found under their name, rather than under
         * their object type, so looking for a singular form is pointless.
         */
        if (!alt)
            alt = makesingular(dbase_str);

        pass1found_in_file = FALSE;
        for (pass = !strcmp(alt, dbase_str) ? 0 : 1; pass >= 0; --pass) {
            long entry_offset, fseekoffset;
            int entry_count;
            found_in_file = skipping_entry = FALSE;
            txt_offset = 0L;
            if (dlb_fseek(fp, txt_offset, SEEK_SET) < 0 ) {
                impossible("can't get to start of 'data' file");
                goto checkfile_done;
            }
            /* skip first record; read second */
            if (!dlb_fgets(buf, BUFSZ, fp) || !dlb_fgets(buf, BUFSZ, fp)) {
                impossible("can't read 'data' file");
                goto checkfile_done;
            } else if (sscanf(buf, "%8lx\n", &txt_offset) < 1
                       || txt_offset == 0L)
                goto bad_data_file;

            /* look for the appropriate entry */
            while (dlb_fgets(buf, BUFSZ, fp)) {
                if (*buf == '.')
                    break; /* we passed last entry without success */

                if (digit(*buf)) {
                    /* a number indicates the end of current entry */
                    skipping_entry = FALSE;
                } else if (!skipping_entry) {
                    if (!(ep = index(buf, '\n')))
                        goto bad_data_file;
                    (void) strip_newline((ep > buf) ? ep - 1 : ep);
                    /* if we match a key that begins with "~", skip
                       this entry */
                    chk_skip = (*buf == '~') ? 1 : 0;
                    encycl_matched = (char *) 0;
                    if (pass == 0) {
                        if (pmatch(&buf[chk_skip], dbase_str_with_material)) {
                            encycl_matched = dbase_str_with_material;
                        } else if (pmatch(&buf[chk_skip], dbase_str)) {
                            encycl_matched = dbase_str;
                        }
                    } else if (pass == 1 && alt && pmatch(&buf[chk_skip], alt)) {
                        encycl_matched = alt;
                    }
                    if (encycl_matched) {
                        if (chk_skip) {
                            skipping_entry = TRUE;
                            continue;
                        } else {
                            found_in_file = TRUE;
                            Strcpy(matcher, buf);
                            if (pass == 1)
                                pass1found_in_file = TRUE;
                            break;
                        }
                    }
                }
            }

            /* database entry should exist, now find where it is */
            if (found_in_file) {
                /* skip over other possible matches for the info */
                do {
                    if (!dlb_fgets(buf, BUFSZ, fp))
                        goto bad_data_file;
                } while (!digit(*buf));

                if (sscanf(buf, "%ld,%d\n", &entry_offset, &entry_count) < 2) {
                    goto bad_data_file;
                fseekoffset = (long) txt_offset + entry_offset;
                if (pass == 1)
                    pass1offset = fseekoffset;
                else if (fseekoffset == pass1offset)
                    goto checkfile_done;
                }
            }

            /* monster lookup: try to parse as a monster
             * use dbase_str_with_material here; if it differs from
             * dbase_str, then it's likely that dbase_str stripped off the
             * "iron" from "iron golem" or something. */
            if (!lookat_mon) {
                pm = (struct permonst *) 0; /* just to be safe */
                if (!object_not_monster(dbase_str_with_material)) {
                    int mndx = name_to_mon(dbase_str_with_material, (int *) 0);
                    if (mndx != NON_PM) {
                        pm = &mons[mndx];
                    }
                }
            }

            /* object lookup: try to parse as an object, and try the material
             * version of the string first */
            otyp = name_to_otyp(dbase_str_with_material);
            if (otyp == STRANGE_OBJECT) {
                otyp = name_to_otyp(dbase_str);
            }

            /* prompt for more info (if using whatis to navigate the map) */
            yes_to_moreinfo = FALSE;
            if (!user_typed_name && !without_asking) {
                char *entrytext = pass ? alt : dbase_str;
                char question[QBUFSZ];

                Strcpy(question, "More info about \"");
                /* +2 => length of "\"?" */
                copynchars(eos(question), entrytext,
                           (int) (sizeof question - 1
                                  - (strlen(question) + 2)));
                Strcat(question, "\"?");
                if (yn(question) == 'y')
                    yes_to_moreinfo = TRUE;
            }

            /* finally, put the appropriate information into a window */
            if (user_typed_name || without_asking || yes_to_moreinfo) {
                if (!found_in_file &&
                    ((!pm && otyp == STRANGE_OBJECT) || !flags.lookup_data)) {
                    if ((user_typed_name && pass == 0 && !pass1found_in_file)
                        || yes_to_moreinfo)
                        pline("I don't have any information on those things.");
                    /* don't print anything otherwise; we don't want it to e.g.
                     * print a database entry and then print the above message.
                     */
                } else {
                    boolean do_obj_lookup = FALSE, do_mon_lookup = FALSE;
                    if (pm) {
                        do_mon_lookup = TRUE;
                        if (!lookat_mon && otyp != STRANGE_OBJECT) {
                            /* found matches for both and player is NOT looking
                             * at a monster; ask which they want to see */
                            /* TODO: this would ideally be better generalized so
                             * that the caller could communicate that an object
                             * is being looked at, too */
                            pline("That matches both a monster and an object.");
                            if (yn("Show the monster information?") != 'y') {
                                do_obj_lookup = TRUE;
                                do_mon_lookup = FALSE;
                            }
                        }
                    } else if (otyp != STRANGE_OBJECT) {
                        do_obj_lookup = TRUE;
                    }
                    datawin = create_nhwindow(NHW_MENU);

                    if (!flags.lookup_data) {
                        ; /* do nothing, 'pokedex' is disabled */
                    } else if (do_obj_lookup) { /* object lookup info */
                        add_obj_info(datawin, otyp);
                        putstr(datawin, 0, "");
                    /* secondary to object lookup because there are some
                     * monsters whose names are substrings of objects, like
                     * "skeleton" and "skeleton key". */
                    } else if (do_mon_lookup) { /* monster lookup info */
                        if (!wizard && (pm == &mons[PM_SHAMBLING_HORROR])
                            && mvitals[PM_SHAMBLING_HORROR].died < 1)
                            ; /* no freebies until one has been killed */
                        else if (is_rider(pm) && !user_typed_name
                                 && !without_asking)
                            ; /* no stats via farlook */
                        else
                            add_mon_info(datawin, pm);
                        if (is_were(pm) && pm != &mons[PM_RAT_KING]) {
                            /* also do the alternate form */
                            putstr(datawin, 0, "");
                            add_mon_info(datawin,
                                         &mons[counter_were(monsndx(pm))]);
                        }
                        putstr(datawin, 0, "");
                    }

                    /* encyclopedia entry */
                    if (found_in_file) {
                        char titlebuf[BUFSZ];
                        int i;
                        if (dlb_fseek(fp, (long) txt_offset + entry_offset,
                                      SEEK_SET) < 0) {
                            pline("? Seek error on 'data' file!");
                            (void) dlb_fclose(fp);
                            return;
                        }

                        Sprintf(titlebuf,
                                "Encyclopedia entry for \"%s\" (matched to \"%s\"):",
                                encycl_matched, matcher);
                        putstr(datawin, ATR_BOLD, titlebuf);
                        putstr(datawin, ATR_NONE, "");

                        for (i = 0; i < entry_count; i++) {
                            /* room for 1-tab or 8-space prefix + BUFSZ-1 + \0 */
                            char tabbuf[BUFSZ + 8], *tp;

                            if (!dlb_fgets(tabbuf, BUFSZ, fp))
                                goto bad_data_file;
                            tp = tabbuf;
                            if (!index(tp, '\n'))
                                goto bad_data_file;
                            (void) strip_newline(tp);
                            /* text in this file is indented with one tab but
                               someone modifying it might use spaces instead */
                            if (*tp == '\t') {
                                ++tp;
                            } else if (*tp == ' ') {
                                /* remove up to 8 spaces (we expect 8-column
                                   tab stops but user might have them set at
                                   something else so we don't require it) */
                                do {
                                    ++tp;
                                } while (tp < &tabbuf[8] && *tp == ' ');
                            } else if (*tp) { /* empty lines are ok */
                                goto bad_data_file;
                            }
                            /* if a tab after the leading one is found,
                               convert tabs into spaces; the attributions
                               at the end of quotes typically have them */
                            if (index(tp, '\t') != 0)
                                (void) tabexpand(tp);
                            putstr(datawin, 0, tp);
                        }
                    }
                    display_nhwindow(datawin, FALSE);
                    destroy_nhwindow(datawin), datawin = WIN_ERR;
                }
            }
        }
    }
    goto checkfile_done; /* skip error feedback */

 bad_data_file:
    impossible("'data' file in wrong format or corrupted");
 checkfile_done:
    if (datawin != WIN_ERR)
        destroy_nhwindow(datawin);
    (void) dlb_fclose(fp);
    return;
}

int
do_screen_description(cc, looked, sym, out_str, firstmatch, for_supplement)
coord cc;
boolean looked;
int sym;
char *out_str;
const char **firstmatch;
struct permonst **for_supplement;
{
    static const char mon_interior[] = "the interior of a monster",
                      unreconnoitered[] = "unreconnoitered";
    static char look_buf[BUFSZ];
    char prefix[BUFSZ];
    int i, alt_i, j, glyph = NO_GLYPH,
        skipped_venom = 0, found = 0; /* count of matching syms found */
    boolean hit_trap, need_to_look = FALSE,
            submerged = (Underwater && !Is_waterlevel(&u.uz)
                         && !See_underwater);
    const char *x_str;
    nhsym tmpsym;

    if (looked) {
        int oc;
        unsigned os;

        glyph = glyph_at(cc.x, cc.y);
        /* Convert glyph at selected position to a symbol for use below. */
        (void) mapglyph(glyph, &sym, &oc, &os, cc.x, cc.y, 0);

        Sprintf(prefix, "%s        ", encglyph(glyph));
    } else
        Sprintf(prefix, "%c        ", sym);

    /*
     * Check all the possibilities, saving all explanations in a buffer.
     * When all have been checked then the string is printed.
     */

    /*
     * Handle restricted vision range (limited to adjacent spots when
     * swallowed or underwater) cases first.
     *
     * 3.6.0 listed anywhere on map, other than self, as "interior
     * of a monster" when swallowed, and non-adjacent water or
     * non-water anywhere as "dark part of a room" when underwater.
     * "unreconnoitered" is an attempt to convey "even if you knew
     * what was there earlier, you don't know what is there in the
     * current circumstance".
     *
     * (Note: 'self' will always be visible when swallowed so we don't
     * need special swallow handling for <ux,uy>.
     * Another note: for '#terrain' without monsters, u.uswallow and
     * submerged will always both be False and skip this code.)
     */
    x_str = 0;
    if (!looked) {
        ; /* skip special handling */
    } else if (((u.uswallow || submerged) && distu(cc.x, cc.y) > 2)
               /* detection showing some category, so mostly background */
               || ((iflags.terrainmode & (TER_DETECT | TER_MAP)) == TER_DETECT
                   && glyph == cmap_to_glyph(S_stone))) {
        x_str = unreconnoitered;
        need_to_look = FALSE;
    } else if (is_swallow_sym(sym)) {
        x_str = mon_interior;
        need_to_look = TRUE; /* for specific monster type */
    }
    if (x_str) {
        /* we know 'found' is zero here, but guard against some other
           special case being inserted ahead of us someday */
        if (!found) {
            Sprintf(out_str, "%s%s", prefix, x_str);
            *firstmatch = x_str;
            found++;
        } else {
            found += append_str(out_str, x_str); /* not 'an(x_str)' */
        }
        /* for is_swallow_sym(), we want to list the current symbol's
           other possibilities (wand for '/', throne for '\\', &c) so
           don't jump to the end for the x_str==mon_interior case */
        if (x_str == unreconnoitered)
            goto didlook;
    }
 check_monsters:
    /* Check for monsters */
    if (!iflags.terrainmode || (iflags.terrainmode & TER_MON) != 0) {
        for (i = 1; i < MAXMCLASSES; i++) {
            if (sym == (looked ? showsyms[i + SYM_OFF_M] : def_monsyms[i].sym)
                && def_monsyms[i].explain && *def_monsyms[i].explain) {
                need_to_look = TRUE;
                if (!found) {
                    Sprintf(out_str, "%s%s",
                            prefix, an(def_monsyms[i].explain));
                    *firstmatch = def_monsyms[i].explain;
                    found++;
                } else {
                    found += append_str(out_str, an(def_monsyms[i].explain));
                }
            }
        }
        /* handle '@' as a special case if it refers to you and you're
           playing a character which isn't normally displayed by that
           symbol; firstmatch is assumed to already be set for '@' */
        if ((looked ? (sym == showsyms[S_HUMAN + SYM_OFF_M]
                       && cc.x == u.ux && cc.y == u.uy)
                    : (sym == def_monsyms[S_HUMAN].sym && !flags.showrace))
            && !(Race_if(PM_HUMAN) || Race_if(PM_ELF)
                 || Race_if(PM_DROW)) && !Upolyd)
            found += append_str(out_str, "you"); /* tack on "or you" */
    }

    /* Now check for objects */
    if (!iflags.terrainmode || (iflags.terrainmode & TER_OBJ) != 0) {
        for (i = 1; i < MAXOCLASSES; i++) {
            if (sym == (looked ? showsyms[i + SYM_OFF_O]
                               : def_oc_syms[i].sym)
                || (looked && i == ROCK_CLASS && glyph_is_statue(glyph))) {
                need_to_look = TRUE;
                if (looked && i == VENOM_CLASS) {
                    skipped_venom++;
                    continue;
                }
                if (!found) {
                    Sprintf(out_str, "%s%s",
                            prefix, an(def_oc_syms[i].explain));
                    *firstmatch = def_oc_syms[i].explain;
                    found++;
                } else {
                    found += append_str(out_str, an(def_oc_syms[i].explain));
                }
            }
        }
    }

    if (sym == DEF_INVISIBLE) {
        extern const char altinvisexplain[]; /* drawing.c */
        /* for active clairvoyance, use alternate "unseen creature" */
        boolean usealt = (EDetect_monsters & I_SPECIAL) != 0L;
        const char *unseen_explain = !usealt ? invisexplain : altinvisexplain;

        if (!found) {
            Sprintf(out_str, "%s%s", prefix, an(unseen_explain));
            *firstmatch = unseen_explain;
            found++;
        } else {
            found += append_str(out_str, an(unseen_explain));
        }
    }

    /* Now check for graphics symbols */
    alt_i = (sym == (looked ? showsyms[0] : defsyms[0].sym)) ? 0 : (2 + 1);
    for (hit_trap = FALSE, i = 0; i < MAXPCHARS; i++) {
        /* when sym is the default background character, we process
           i == 0 three times: unexplored, stone, dark part of a room */
        if (alt_i < 2) {
            x_str = !alt_i++ ? "unexplored" : submerged ? "unknown" : "stone";
            i = 0; /* for second iteration, undo loop increment */
            /* alt_i is now 1 or 2 */
        } else {
            if (alt_i++ == 2)
                i = 0; /* undo loop increment */
            x_str = defsyms[i].explanation;
            if (i == S_magic_chest)
                continue; /* don't mention it when asking what '(' is */
            if (submerged && !strcmp(x_str, defsyms[0].explanation))
                x_str = "land"; /* replace "dark part of a room" */
            /* alt_i is now 3 or more and no longer of interest */
        }
        if (sym == (looked ? showsyms[i] : defsyms[i].sym) && *x_str) {
            /* avoid "an unexplored", "an stone", "an air", "a water",
               "a floor of a room", "a dark part of a room";
               article==2 => "the", 1 => "an", 0 => (none) */
            int article = strstri(x_str, " of a room") ? 2
                          : !(alt_i <= 2
                              || strcmp(x_str, "air") == 0
                              || strcmp(x_str, "land") == 0
                              || strcmp(x_str, "grass") == 0
                              || strcmp(x_str, "sand") == 0
                              || strcmp(x_str, "shallow water") == 0
                              || strcmp(x_str, "sewage") == 0
                              || strcmp(x_str, "water") == 0);

            if (!found) {
                if (is_cmap_trap(i)) {
                    Sprintf(out_str, "%sa trap", prefix);
                    hit_trap = TRUE;
                } else {
                    Sprintf(out_str, "%s%s", prefix,
                            article == 2 ? the(x_str)
                            : article == 1 ? an(x_str) : x_str);
                }
                *firstmatch = x_str;
                found++;
            } else if (!(hit_trap && is_cmap_trap(i))
                       && !(found >= 3 && is_cmap_drawbridge(i))
                       /* don't mention vibrating square outside of Gehennom
                          unless this happens to be one (hallucination?) */
                       && (i != S_vibrating_square || Inhell
                           || (looked && glyph_is_trap(glyph)
                               && glyph_to_trap(glyph) == VIBRATING_SQUARE))) {
                found += append_str(out_str, (article == 2) ? the(x_str)
                                             : (article == 1) ? an(x_str)
                                               : x_str);
                if (is_cmap_trap(i))
                    hit_trap = TRUE;
            }

            if (i == S_altar || is_cmap_trap(i))
                need_to_look = TRUE;
        }
    }

    /* Now check for warning symbols */
    for (i = 1; i < WARNCOUNT; i++) {
        x_str = def_warnsyms[i].explanation;
        if (sym == (looked ? warnsyms[i] : def_warnsyms[i].sym)) {
            if (!found) {
                Sprintf(out_str, "%s%s", prefix, def_warnsyms[i].explanation);
                *firstmatch = def_warnsyms[i].explanation;
                found++;
            } else {
                found += append_str(out_str, def_warnsyms[i].explanation);
            }
            /* Kludge: warning trumps boulders on the display.
               Reveal the boulder too or player can get confused */
            if (looked && sobj_at(BOULDER, cc.x, cc.y))
                Strcat(out_str, " co-located with a boulder");
            break; /* out of for loop*/
        }
    }

    /* if we ignored venom and list turned out to be short, put it back */
    if (skipped_venom && found < 2) {
        x_str = def_oc_syms[VENOM_CLASS].explain;
        if (!found) {
            Sprintf(out_str, "%s%s", prefix, an(x_str));
            *firstmatch = x_str;
            found++;
        } else {
            found += append_str(out_str, an(x_str));
        }
    }

    /* Finally, handle some optional overriding symbols */
    for (j = SYM_OFF_X; j < SYM_MAX; ++j) {
        if (j == (SYM_INVISIBLE + SYM_OFF_X))
            continue;       /* already handled above */
        tmpsym = Is_rogue_level(&u.uz) ? ov_rogue_syms[j]
                                       : ov_primary_syms[j];
        if (tmpsym && sym == tmpsym) {
            switch (j) {
            case SYM_BOULDER + SYM_OFF_X:
                if (!found) {
                    *firstmatch = "boulder";
                    Sprintf(out_str, "%s%s", prefix, an(*firstmatch));
                    found++;
                } else {
                    found += append_str(out_str, "boulder");
                }
                break;
            case SYM_PET_OVERRIDE + SYM_OFF_X:
                if (looked) {
                    int oc = 0;
                    unsigned os = 0;

                    /* convert to symbol without override in effect */
                    (void) mapglyph(glyph, &sym, &oc, &os,
                                    cc.x, cc.y, MG_FLAG_NOOVERRIDE);
                    goto check_monsters;
                }
                break;
            case SYM_HERO_OVERRIDE + SYM_OFF_X:
                sym = showsyms[S_HUMAN + SYM_OFF_M];
                goto check_monsters;
            }
        }
    }
#if 0
    /* handle optional boulder symbol as a special case */
    if (o_syms[SYM_BOULDER + SYM_OFF_X]
        && sym == o_syms[SYM_BOULDER + SYM_OFF_X]) {
        if (!found) {
            *firstmatch = "boulder";
            Sprintf(out_str, "%s%s", prefix, an(*firstmatch));
            found++;
        } else {
            found += append_str(out_str, "boulder");
        }
    }
#endif

    /*
     * If we are looking at the screen, follow multiple possibilities or
     * an ambiguous explanation by something more detailed.
     */

    if (found > 4)
        /* 3.6.3: this used to be "That can be many things" (without prefix)
           which turned it into a sentence that lacked its terminating period;
           we could add one below but reinstating the prefix here is better */
        Sprintf(out_str, "%scan be many things", prefix);

 didlook:
    if (looked) {
        struct permonst *pm = (struct permonst *)0;

        if (found > 1 || need_to_look) {
            char monbuf[BUFSZ];
            char temp_buf[BUFSZ];

            pm = lookat(cc.x, cc.y, look_buf, monbuf);
            if (pm && for_supplement)
                *for_supplement = pm;
            *firstmatch = look_buf;
            if (*(*firstmatch)) {
                Sprintf(temp_buf, " (%s)", *firstmatch);
                (void) strncat(out_str, temp_buf,
                               BUFSZ - strlen(out_str) - 1);
                found = 1; /* we have something to look up */
            }
            if (monbuf[0]) {
                Sprintf(temp_buf, " [seen: %s]", monbuf);
                (void) strncat(out_str, temp_buf,
                               BUFSZ - strlen(out_str) - 1);
            }
        }
    }

    return found;
}

/* also used by getpos hack in do_name.c */
const char what_is_an_unknown_object[] = "an unknown object";

int
do_look(mode, click_cc)
int mode;
coord *click_cc;
{
    boolean quick = (mode == 1); /* use cursor; don't search for "more info" */
    boolean clicklook = (mode == 2); /* right mouse-click method */
    char out_str[BUFSZ] = DUMMY;
    const char *firstmatch = 0;
    struct permonst *pm = 0, *supplemental_pm = 0;
    int i = '\0', ans = 0;
    int sym;              /* typed symbol or converted glyph */
    int found;            /* count of matching syms found */
    coord cc;             /* screen pos of unknown glyph */
    boolean save_verbose; /* saved value of flags.verbose */
    boolean from_screen;  /* question from the screen */

    cc.x = 0;
    cc.y = 0;

    if (!clicklook) {
        if (quick) {
            from_screen = TRUE; /* yes, we want to use the cursor */
            i = 'y';
        } else {
            menu_item *pick_list = (menu_item *) 0;
            winid win;
            anything any;

            any = zeroany;
            win = create_nhwindow(NHW_MENU);
            start_menu(win);
            any.a_char = '/';
            /* 'y' and 'n' to keep backwards compatibility with previous
               versions: "Specify unknown object by cursor?" */
            add_menu(win, NO_GLYPH, &any,
                     flags.lootabc ? 0 : any.a_char, 'y', ATR_NONE,
                     "something on the map", MENU_UNSELECTED);
            any.a_char = 'i';
            add_menu(win, NO_GLYPH, &any,
                     flags.lootabc ? 0 : any.a_char, 0, ATR_NONE,
                     "something you're carrying", MENU_UNSELECTED);
            any.a_char = '?';
            add_menu(win, NO_GLYPH, &any,
                     flags.lootabc ? 0 : any.a_char, 'n', ATR_NONE,
                     "something else (by symbol or name)", MENU_UNSELECTED);
            if (!u.uswallow && !Hallucination) {
                any = zeroany;
                add_menu(win, NO_GLYPH, &any, 0, 0, ATR_NONE,
                         "", MENU_UNSELECTED);
                /* these options work sensibly for the swallowed case,
                   but there's no reason for the player to use them then;
                   objects work fine when hallucinating, but screen
                   symbol/monster class letter doesn't match up with
                   bogus monster type, so suppress when hallucinating */
                any.a_char = 'm';
                add_menu(win, NO_GLYPH, &any,
                         flags.lootabc ? 0 : any.a_char, 0, ATR_NONE,
                         "nearby monsters", MENU_UNSELECTED);
                any.a_char = 'M';
                add_menu(win, NO_GLYPH, &any,
                         flags.lootabc ? 0 : any.a_char, 0, ATR_NONE,
                         "all monsters shown on map", MENU_UNSELECTED);
                any.a_char = 'o';
                add_menu(win, NO_GLYPH, &any,
                         flags.lootabc ? 0 : any.a_char, 0, ATR_NONE,
                         "nearby objects", MENU_UNSELECTED);
                any.a_char = 'O';
                add_menu(win, NO_GLYPH, &any,
                         flags.lootabc ? 0 : any.a_char, 0, ATR_NONE,
                         "all objects shown on map", MENU_UNSELECTED);
            }
            end_menu(win, "What do you want to look at:");
            if (select_menu(win, PICK_ONE, &pick_list) > 0) {
                i = pick_list->item.a_char;
                free((genericptr_t) pick_list);
            }
            destroy_nhwindow(win);
        }

        switch (i) {
        default:
        case 'q':
            return 0;
        case 'y':
        case '/':
            from_screen = TRUE;
            sym = 0;
            cc.x = u.ux;
            cc.y = u.uy;
            break;
        case 'i':
          {
            char invlet;
            struct obj *invobj;

            invlet = display_inventory((const char *) 0, TRUE);
            if (!invlet || invlet == '\033')
                return 0;
            *out_str = '\0';
            for (invobj = invent; invobj; invobj = invobj->nobj)
                if (invobj->invlet == invlet) {
                    strcpy(out_str, singular(invobj, xname));
                    break;
                }
            if (*out_str)
                checkfile(out_str, (struct permonst *) 0, TRUE, TRUE,
                          (char *) 0);
            return 0;
          }
        case '?':
            from_screen = FALSE;
            getlin("Specify what? (type the word)", out_str);
            if (strcmp(out_str, " ")) /* keep single space as-is */
                /* remove leading and trailing whitespace and
                   condense consecutive internal whitespace */
                mungspaces(out_str);
            if (out_str[0] == '\0' || out_str[0] == '\033')
                return 0;

            if (out_str[1]) { /* user typed in a complete string */
                checkfile(out_str, pm, TRUE, TRUE, (char *) 0);
                return 0;
            }
            sym = out_str[0];
            break;
        case 'm':
            look_all(TRUE, TRUE); /* list nearby monsters */
            return 0;
        case 'M':
            look_all(FALSE, TRUE); /* list all monsters */
            return 0;
        case 'o':
            look_all(TRUE, FALSE); /* list nearby objects */
            return 0;
        case 'O':
            look_all(FALSE, FALSE); /* list all objects */
            return 0;
        }
    } else { /* clicklook */
        cc.x = click_cc->x;
        cc.y = click_cc->y;
        sym = 0;
        from_screen = FALSE;
    }

    /* Save the verbose flag, we change it later. */
    save_verbose = flags.verbose;
    flags.verbose = flags.verbose && !quick;
    /*
     * The user typed one letter, or we're identifying from the screen.
     */
    do {
        /* Reset some variables. */
        pm = (struct permonst *) 0;
        found = 0;
        out_str[0] = '\0';

        if (from_screen || clicklook) {
            if (from_screen) {
                if (flags.verbose)
                    pline("Please move the cursor to %s.",
                          what_is_an_unknown_object);
                else
                    pline("Pick an object.");

                ans = getpos(&cc, quick, what_is_an_unknown_object);
                if (ans < 0 || cc.x < 0)
                    break; /* done */
                flags.verbose = FALSE; /* only print long question once */
            }
        }

        found = do_screen_description(cc, (from_screen || clicklook), sym,
                                      out_str, &firstmatch, &supplemental_pm);

        /* Finally, print out our explanation. */
        if (found) {
            /* use putmixed() because there may be an encoded glyph present */
            putmixed(WIN_MESSAGE, 0, out_str);
#if defined(DUMPLOG) || defined (DUMPHTML)
            {
                char dmpbuf[BUFSZ];

                /* putmixed() bypasses pline() so doesn't write to DUMPLOG;
                   tty puts it into ^P recall, so it ought to be there;
                   DUMPLOG is plain text, so override graphics character;
                   at present, force space, but we ought to use defsyms[]
                   value for the glyph the graphics character came from */
                (void) decode_mixed(dmpbuf, out_str);
                if (dmpbuf[0] < ' ' || dmpbuf[0] >= 127) /* ASCII isprint() */
                    dmpbuf[0] = ' ';
                dumplogmsg(dmpbuf);
            }
#endif

            /* check the data file for information about this thing */
            if (found == 1 && ans != LOOK_QUICK && ans != LOOK_ONCE
                && (ans == LOOK_VERBOSE || (flags.help && !quick))
                && !clicklook) {
                char temp_buf[BUFSZ], supplemental_name[BUFSZ];

                supplemental_name[0] = '\0';
                Strcpy(temp_buf, firstmatch);
                checkfile(temp_buf, supplemental_pm, FALSE,
                          (boolean) (ans == LOOK_VERBOSE), supplemental_name);
                if (supplemental_pm)
                    do_supplemental_info(supplemental_name, supplemental_pm,
                                         (boolean) (ans == LOOK_VERBOSE));
            }
        } else {
            pline("I've never heard of such things.");
        }
    } while (from_screen && !quick && ans != LOOK_ONCE && !clicklook);

    flags.verbose = save_verbose;
    return 0;
}

STATIC_OVL void
look_all(nearby, do_mons)
boolean nearby; /* True => within BOLTLIM, False => entire map */
boolean do_mons; /* True => monsters, False => objects */
{
    winid win;
    int x, y, lo_x, lo_y, hi_x, hi_y, glyph, count = 0;
    char lookbuf[BUFSZ], outbuf[BUFSZ];

    win = create_nhwindow(NHW_TEXT);
    lo_y = nearby ? max(u.uy - BOLT_LIM, 0) : 0;
    lo_x = nearby ? max(u.ux - BOLT_LIM, 1) : 1;
    hi_y = nearby ? min(u.uy + BOLT_LIM, ROWNO - 1) : ROWNO - 1;
    hi_x = nearby ? min(u.ux + BOLT_LIM, COLNO - 1) : COLNO - 1;
    for (y = lo_y; y <= hi_y; y++) {
        for (x = lo_x; x <= hi_x; x++) {
            lookbuf[0] = '\0';
            glyph = glyph_at(x, y);
            if (do_mons) {
                if (glyph_is_monster(glyph)) {
                    struct monst *mtmp;

                    bhitpos.x = x; /* [is this actually necessary?] */
                    bhitpos.y = y;
                    if (x == u.ux && y == u.uy && canspotself()) {
                        (void) self_lookat(lookbuf);
                        ++count;
                    } else if ((mtmp = m_at(x, y)) != 0) {
                        look_at_monster(lookbuf, (char *) 0, mtmp, x, y);
                        ++count;
                    }
                } else if (glyph_is_invisible(glyph)) {
                    /* remembered, unseen, creature */
                    Strcpy(lookbuf, invisexplain);
                    ++count;
                } else if (glyph_is_warning(glyph)) {
                    int warnindx = glyph_to_warning(glyph);

                    Strcpy(lookbuf, def_warnsyms[warnindx].explanation);
                    ++count;
                }
            } else { /* !do_mons */
                if (glyph_is_object(glyph)) {
                    look_at_object(lookbuf, x, y, glyph);
                    ++count;
                }
            }
            if (*lookbuf) {
                char coordbuf[20], which[12], cmode;

                cmode = (iflags.getpos_coords != GPCOORDS_NONE)
                           ? iflags.getpos_coords : GPCOORDS_MAP;
                if (count == 1) {
                    Strcpy(which, do_mons ? "monsters" : "objects");
                    if (nearby)
                        Sprintf(outbuf, "%s currently shown near %s:",
                                upstart(which),
                                (cmode != GPCOORDS_COMPASS)
                                  ? coord_desc(u.ux, u.uy, coordbuf, cmode)
                                  : !canspotself() ? "your position" : "you");
                    else
                        Sprintf(outbuf, "All %s currently shown on the map:",
                                which);
                    putstr(win, 0, outbuf);
                    putstr(win, 0, "");
                }
                /* prefix: "coords  C  " where 'C' is mon or obj symbol */
                Sprintf(outbuf, (cmode == GPCOORDS_SCREEN) ? "%s  "
                                  : (cmode == GPCOORDS_MAP) ? "%8s  "
                                      : "%12s  ",
                        coord_desc(x, y, coordbuf, cmode));
                Sprintf(eos(outbuf), "%s  ", encglyph(glyph));
                /* guard against potential overflow */
                lookbuf[sizeof lookbuf - 1 - strlen(outbuf)] = '\0';
                Strcat(outbuf, lookbuf);
                putmixed(win, 0, outbuf);
            }
        }
    }
    if (count)
        display_nhwindow(win, TRUE);
    else
        pline("No %s are currently shown %s.",
              do_mons ? "monsters" : "objects",
              nearby ? "nearby" : "on the map");
    destroy_nhwindow(win);
}

static const char *suptext1[] = {
    "%s is a member of a marauding horde of orcs",
    "rumored to have brutally attacked and plundered",
    "the ordinarily sheltered town that is located ",
    "deep within The Gnomish Mines.",
    "",
    "The members of that vicious horde proudly and ",
    "defiantly acclaim their allegiance to their",
    "leader %s in their names.",
    (char *) 0,
};

static const char *suptext2[] = {
    "\"%s\" is the common dungeon name of",
    "a nefarious orc who is known to acquire property",
    "from thieves and sell it off for profit.",
    "",
    "The perpetrator was last seen hanging around the",
    "stairs leading to the Gnomish Mines.",
    (char *) 0,
};

STATIC_OVL void
do_supplemental_info(name, pm, without_asking)
char *name;
struct permonst *pm;
boolean without_asking;
{
    const char **textp;
    winid datawin = WIN_ERR;
    char *entrytext = name, *bp = (char *) 0, *bp2 = (char *) 0;
    char question[QBUFSZ];
    boolean yes_to_moreinfo = FALSE;
    boolean is_marauder = (name && pm && is_orc(pm));

    /*
     * Provide some info on some specific things
     * meant to support in-game mythology, and not
     * available from data.base or other sources.
     */
    if (is_marauder && (strlen(name) < (BUFSZ - 1))) {
        char fullname[BUFSZ];

        bp = strstri(name, " of ");
        bp2 = strstri(name, " the Fence");

        if (bp || bp2) {
            Strcpy(fullname, name);
            if (!without_asking) {
                Strcpy(question, "More info about \"");
                /* +2 => length of "\"?" */
                copynchars(eos(question), entrytext,
                    (int) (sizeof question - 1 - (strlen(question) + 2)));
                Strcat(question, "\"?");
                if (yn(question) == 'y')
                yes_to_moreinfo = TRUE;
            }
            if (yes_to_moreinfo) {
                int i, subs = 0;
                const char *gang = (char *) 0;

                if (bp) {
                    textp = suptext1;
                    gang = bp + 4;
                    *bp = '\0';
                } else {
                    textp = suptext2;
                    gang = "";
                }
                datawin = create_nhwindow(NHW_MENU);
                for (i = 0; textp[i]; i++) {
                    char buf[BUFSZ];
                    const char *txt;

                    if (strstri(textp[i], "%s") != 0) {
                        Sprintf(buf, textp[i], subs++ ? gang : fullname);
                        txt = buf;
                    } else
                        txt = textp[i];
                    putstr(datawin, 0, txt);
                }
                display_nhwindow(datawin, FALSE);
                destroy_nhwindow(datawin), datawin = WIN_ERR;
            }
        }
    }
}

/* the '/' command */
int
dowhatis()
{
    return do_look(0, (coord *) 0);
}

/* the ';' command */
int
doquickwhatis()
{
    return do_look(1, (coord *) 0);
}

/* the '^' command */
int
doidtrap()
{
    register struct trap *trap;
    int x, y, tt, glyph;

    if (!getdir("^"))
        return 0;
    x = u.ux + u.dx;
    y = u.uy + u.dy;

    /* check fake bear trap from confused gold detection */
    glyph = glyph_at(x, y);
    if (glyph_is_trap(glyph) && (tt = glyph_to_trap(glyph)) == BEAR_TRAP) {
        boolean chesttrap = trapped_chest_at(tt, x, y);

        if (chesttrap || trapped_door_at(tt, x, y)) {
            pline("That is a trapped %s.", chesttrap ? "chest" : "door");
            return 0; /* trap ID'd, but no time elapses */
        }
    }

    for (trap = ftrap; trap; trap = trap->ntrap)
        if (trap->tx == x && trap->ty == y) {
            if (!trap->tseen)
                break;
            tt = trap->ttyp;
            if (u.dz) {
                if (u.dz < 0 ? is_hole(tt) : tt == ROCKTRAP)
                    break;
            }
            tt = what_trap(tt, rn2_on_display_rng);
            pline("That is %s%s%s.",
                  an(defsyms[trap_to_defsym(tt)].explanation),
                  !trap->madeby_u
                     ? ""
                     : (tt == WEB)
                        ? " woven"
                        /* trap doors & spiked pits can't be made by
                           player, and should be considered at least
                           as much "set" as "dug" anyway */
                        : (tt == HOLE || tt == PIT)
                           ? " dug"
                           : " set",
                  !trap->madeby_u ? "" : " by you");
            return 0;
        }
    pline("I can't see a trap there.");
    return 0;
}

/*
    Implements a rudimentary if/elif/else/endif interpretor and use
    conditionals in dat/cmdhelp to describe what command each keystroke
    currently invokes, so that there isn't a lot of "(debug mode only)"
    and "(if number_pad is off)" cluttering the feedback that the user
    sees.  (The conditionals add quite a bit of clutter to the raw data
    but users don't see that.  number_pad produces a lot of conditional
    commands:  basic letters vs digits, 'g' vs 'G' for '5', phone
    keypad vs normal layout of digits, and QWERTZ keyboard swap between
    y/Y/^Y/M-y/M-Y/M-^Y and z/Z/^Z/M-z/M-Z/M-^Z.)

    The interpretor understands
     '&#' for comment,
     '&? option' for 'if' (also '&? !option'
                           or '&? option=value[,value2,...]'
                           or '&? !option=value[,value2,...]'),
     '&: option' for 'elif' (with argument variations same as 'if';
                             any number of instances for each 'if'),
     '&:' for 'else' (also '&: #comment';
                      0 or 1 instance for a given 'if'), and
     '&.' for 'endif' (also '&. #comment'; required for each 'if').

    The option handling is a bit of a mess, with no generality for
    which options to deal with and only a comma separated list of
    integer values for the '=value' part.  number_pad is the only
    supported option that has a value; the few others (wizard/debug,
    rest_on_space, #if SHELL, #if SUSPEND) are booleans.
*/

STATIC_DCL void
whatdoes_help()
{
    dlb *fp;
    char *p, buf[BUFSZ];
    winid tmpwin;

    fp = dlb_fopen(KEYHELP, "r");
    if (!fp) {
        pline("Cannot open \"%s\" data file!", KEYHELP);
        display_nhwindow(WIN_MESSAGE, TRUE);
        return;
    }
    tmpwin = create_nhwindow(NHW_TEXT);
    while (dlb_fgets(buf, (int) sizeof buf, fp)) {
        if (*buf == '#')
            continue;
        for (p = buf; *p; p++)
            if (*p != ' ' && *p != '\t')
                break;
        putstr(tmpwin, 0, p);
    }
    (void) dlb_fclose(fp);
    display_nhwindow(tmpwin, TRUE);
    destroy_nhwindow(tmpwin);
}

#if 0
#define WD_STACKLIMIT 5
struct wd_stack_frame {
    Bitfield(active, 1);
    Bitfield(been_true, 1);
    Bitfield(else_seen, 1);
};

STATIC_DCL boolean FDECL(whatdoes_cond, (char *, struct wd_stack_frame *,
                                         int *, int));

STATIC_OVL boolean
whatdoes_cond(buf, stack, depth, lnum)
char *buf;
struct wd_stack_frame *stack;
int *depth, lnum;
{
    const char badstackfmt[] = "cmdhlp: too many &%c directives at line %d.";
    boolean newcond, neg, gotopt;
    char *p, *q, act = buf[1];
    int np = 0;

    newcond = (act == '?' || !stack[*depth].been_true);
    buf += 2;
    mungspaces(buf);
    if (act == '#' || *buf == '#' || !*buf || !newcond) {
        gotopt = (*buf && *buf != '#');
        *buf = '\0';
        neg = FALSE; /* lint suppression */
        p = q = (char *) 0;
    } else {
        gotopt = TRUE;
        if ((neg = (*buf == '!')) != 0)
            if (*++buf == ' ')
                ++buf;
        p = index(buf, '='), q = index(buf, ':');
        if (!p || (q && q < p))
            p = q;
        if (p) { /* we have a value specified */
            /* handle a space before or after (or both) '=' (or ':') */
            if (p > buf && p[-1] == ' ')
                p[-1] = '\0'; /* end of keyword in buf[] */
            *p++ = '\0'; /* terminate keyword, advance to start of value */
            if (*p == ' ')
                p++;
        }
    }
    if (*buf && (act == '?' || act == ':')) {
        if (!strcmpi(buf, "number_pad")) {
            if (!p) {
                newcond = iflags.num_pad;
            } else {
                /* convert internal encoding (separate yes/no and 0..3)
                   back to user-visible one (-1..4) */
                np = iflags.num_pad ? (1 + iflags.num_pad_mode) /* 1..4 */
                                    : (-1 * iflags.num_pad_mode); /* -1..0 */
                newcond = FALSE;
                for (; p; p = q) {
                    q = index(p, ',');
                    if (q)
                        *q++ = '\0';
                    if (atoi(p) == np) {
                        newcond = TRUE;
                        break;
                    }
                }
            }
        } else if (!strcmpi(buf, "rest_on_space")) {
            newcond = flags.rest_on_space;
        } else if (!strcmpi(buf, "debug") || !strcmpi(buf, "wizard")) {
            newcond = flags.debug; /* == wizard */
        } else if (!strcmpi(buf, "shell")) {
#ifdef SHELL
            /* should we also check sysopt.shellers? */
            newcond = TRUE;
#else
            newcond = FALSE;
#endif
        } else if (!strcmpi(buf, "suspend")) {
#ifdef SUSPEND
            /* sysopt.shellers is also used for dosuspend()... */
            newcond = TRUE;
#else
            newcond = FALSE;
#endif
        } else {
            impossible(
                "cmdhelp: unrecognized &%c conditional at line %d: \"%.20s\"",
                       act, lnum, buf);
            neg = FALSE;
        }
        /* this works for number_pad too: &? !number_pad:-1,0
           would be true for 1..4 after negation */
        if (neg)
            newcond = !newcond;
    }
    switch (act) {
    default:
    case '#': /* comment */
        break;
    case '.': /* endif */
        if (--*depth < 0) {
            impossible(badstackfmt, '.', lnum);
            *depth = 0;
        }
        break;
    case ':': /* else or elif */
        if (*depth == 0 || stack[*depth].else_seen) {
            impossible(badstackfmt, ':', lnum);
            *depth = 1; /* so that stack[*depth - 1] is a valid access */
        }
        if (stack[*depth].active || stack[*depth].been_true
            || !stack[*depth - 1].active)
            stack[*depth].active = 0;
        else if (newcond)
            stack[*depth].active = stack[*depth].been_true = 1;
        if (!gotopt)
            stack[*depth].else_seen = 1;
        break;
    case '?': /* if */
        if (++*depth >= WD_STACKLIMIT) {
            impossible(badstackfmt, '?', lnum);
            *depth = WD_STACKLIMIT - 1;
        }
        stack[*depth].active = (newcond && stack[*depth - 1].active) ? 1 : 0;
        stack[*depth].been_true = stack[*depth].active;
        stack[*depth].else_seen = 0;
        break;
    }
    return stack[*depth].active ? TRUE : FALSE;
}
#endif /* 0 */

char *
dowhatdoes_core(q, cbuf)
char q;
char *cbuf;
{
    char buf[BUFSZ];
#if 0
    dlb *fp;
    struct wd_stack_frame stack[WD_STACKLIMIT];
    boolean cond;
    int ctrl, meta, depth = 0, lnum = 0;
#endif /* 0 */
    const char *ec_desc;

    if ((ec_desc = key2extcmddesc(q)) != NULL) {
        char keybuf[QBUFSZ];

        Sprintf(buf, "%-8s%s.", key2txt(q, keybuf), ec_desc);
        Strcpy(cbuf, buf);
        return cbuf;
    }
    return 0;
#if 0
    fp = dlb_fopen(CMDHELPFILE, "r");
    if (!fp) {
        pline("Cannot open \"%s\" data file!", CMDHELPFILE);
        return 0;
    }

    meta = (0x80 & (uchar) q) != 0;
    if (meta)
        q &= 0x7f;
    ctrl = (0x1f & (uchar) q) == (uchar) q;
    if (ctrl)
        q |= 0x40; /* NUL -> '@', ^A -> 'A', ... ^Z -> 'Z', ^[ -> '[', ... */
    else if (q == 0x7f)
        ctrl = 1, q = '?';

    (void) memset((genericptr_t) stack, 0, sizeof stack);
    cond = stack[0].active = 1;
    while (dlb_fgets(buf, sizeof buf, fp)) {
        ++lnum;
        if (buf[0] == '&' && buf[1] && index("?:.#", buf[1])) {
            cond = whatdoes_cond(buf, stack, &depth, lnum);
            continue;
        }
        if (!cond)
            continue;
        if (meta ? (buf[0] == 'M' && buf[1] == '-'
                    && (ctrl ? buf[2] == '^' && highc(buf[3]) == q
                             : buf[2] == q))
                 : (ctrl ? buf[0] == '^' && highc(buf[1]) == q
                         : buf[0] == q)) {
            (void) strip_newline(buf);
            if (index(buf, '\t'))
                (void) tabexpand(buf);
            if (meta && ctrl && buf[4] == ' ') {
                (void) strncpy(buf, "M-^?    ", 8);
                buf[3] = q;
            } else if (meta && buf[3] == ' ') {
                (void) strncpy(buf, "M-?     ", 8);
                buf[2] = q;
            } else if (ctrl && buf[2] == ' ') {
                (void) strncpy(buf, "^?      ", 8);
                buf[1] = q;
            } else if (buf[1] == ' ') {
                (void) strncpy(buf, "?       ", 8);
                buf[0] = q;
            }
            (void) dlb_fclose(fp);
            Strcpy(cbuf, buf);
            return cbuf;
        }
    }
    (void) dlb_fclose(fp);
    if (depth != 0)
        impossible("cmdhelp: mismatched &? &: &. conditionals.");
    return (char *) 0;
#endif /* 0 */
}

int
dowhatdoes()
{
    static boolean once = FALSE;
    char bufr[BUFSZ];
    char q, *reslt;

    if (!once) {
        pline("Ask about '&' or '?' to get more info.%s",
#ifdef ALTMETA
              iflags.altmeta ? "  (For ESC, type it twice.)" :
#endif
              "");
        once = TRUE;
    }
#if defined(UNIX) || defined(VMS)
    introff(); /* disables ^C but not ^\ */
#endif
    q = yn_function("What command?", (char *) 0, '\0');
#ifdef ALTMETA
    if (q == '\033' && iflags.altmeta) {
        /* in an ideal world, we would know whether another keystroke
           was already pending, but this is not an ideal world...
           if user typed ESC, we'll essentially hang until another
           character is typed */
        q = yn_function("]", (char *) 0, '\0');
        if (q != '\033')
            q = (char) ((uchar) q | 0200);
    }
#endif /*ALTMETA*/
#if defined(UNIX) || defined(VMS)
    intron(); /* reenables ^C */
#endif
    reslt = dowhatdoes_core(q, bufr);
    if (reslt) {
        if (q == '&' || q == '?')
            whatdoes_help();
        pline("%s", reslt);
    } else {
        pline("No such command '%s', char code %d (0%03o or 0x%02x).",
              visctrl(q), (uchar) q, (uchar) q, (uchar) q);
    }
    return 0;
}

STATIC_OVL void
docontact(VOID_ARGS)
{
    winid cwin = create_nhwindow(NHW_TEXT);
    char buf[BUFSZ];

    if (sysopt.support) {
        /*XXX overflow possibilities*/
        Sprintf(buf, "To contact local support, %s", sysopt.support);
        putstr(cwin, 0, buf);
        putstr(cwin, 0, "");
    } else if (sysopt.fmtd_wizard_list) { /* formatted SYSCF WIZARDS */
        Sprintf(buf, "To contact local support, contact %s.",
                sysopt.fmtd_wizard_list);
        putstr(cwin, 0, buf);
        putstr(cwin, 0, "");
    }
    putstr(cwin, 0, "To contact the EvilHack development team directly,");
    /*XXX overflow possibilities*/
    Sprintf(buf, "visit #evilhack or #hardfought on Libera Chat IRC, or email <%s>.",
            DEVTEAM_EMAIL);
    putstr(cwin, 0, buf);
    putstr(cwin, 0, "");
    putstr(cwin, 0, "For more information on EvilHack, or to report a bug,");
    Sprintf(buf, "visit our website \"%s\".", DEVTEAM_URL);
    putstr(cwin, 0, buf);
    display_nhwindow(cwin, FALSE);
    destroy_nhwindow(cwin);
}

STATIC_OVL void
dispfile_help(VOID_ARGS)
{
    display_file(HELP, TRUE);
}

STATIC_OVL void
dispfile_shelp(VOID_ARGS)
{
    display_file(SHELP, TRUE);
}

STATIC_OVL void
dispfile_optionfile(VOID_ARGS)
{
    display_file(OPTIONFILE, TRUE);
}

STATIC_OVL void
dispfile_license(VOID_ARGS)
{
    display_file(LICENSE, TRUE);
}

STATIC_OVL void
dispfile_debughelp(VOID_ARGS)
{
    display_file(DEBUGHELP, TRUE);
}

STATIC_OVL void
hmenu_doextversion(VOID_ARGS)
{
    (void) doextversion();
}

STATIC_OVL void
hmenu_dohistory(VOID_ARGS)
{
    (void) dohistory();
}

STATIC_OVL void
hmenu_dowhatis(VOID_ARGS)
{
    (void) dowhatis();
}

STATIC_OVL void
hmenu_dowhatdoes(VOID_ARGS)
{
    (void) dowhatdoes();
}

STATIC_OVL void
hmenu_doextlist(VOID_ARGS)
{
    (void) doextlist();
}

void
domenucontrols(VOID_ARGS)
{
    winid cwin = create_nhwindow(NHW_TEXT);
    show_menu_controls(cwin, FALSE);
    display_nhwindow(cwin, FALSE);
    destroy_nhwindow(cwin);
}

/* data for dohelp() */
static struct {
    void NDECL((*f));
    const char *text;
} help_menu_items[] = {
    { hmenu_doextversion, "About EvilHack (version information)." },
    { dispfile_help, "Long description of the game and commands." },
    { dispfile_shelp, "List of game commands." },
    { hmenu_dohistory, "Concise history of EvilHack." },
    { hmenu_dowhatis, "Info on a character in the game display." },
    { hmenu_dowhatdoes, "Info on what a given key does." },
    { option_help, "List of game options." },
    { dispfile_optionfile, "Longer explanation of game options." },
    { dokeylist, "Full list of keyboard commands" },
    { hmenu_doextlist, "List of extended commands." },
    { domenucontrols, "List menu control keys" },
    { dispfile_license, "The EvilHack license." },
    { docontact, "Support information." },
#ifdef PORT_HELP
    { port_help, "%s-specific help and commands." },
#endif
    { dispfile_debughelp, "List of wizard-mode commands." },
    { (void NDECL((*))) 0, (char *) 0 }
};

/* the '?' command */
int
dohelp()
{
    winid tmpwin = create_nhwindow(NHW_MENU);
    char helpbuf[QBUFSZ];
    int i, n;
    menu_item *selected;
    anything any;
    int sel;

    any = zeroany; /* zero all bits */
    start_menu(tmpwin);

    for (i = 0; help_menu_items[i].text; i++) {
        if (!wizard && help_menu_items[i].f == dispfile_debughelp)
            continue;
        if (help_menu_items[i].text[0] == '%') {
            Sprintf(helpbuf, help_menu_items[i].text, PORT_ID);
        } else {
            Strcpy(helpbuf, help_menu_items[i].text);
        }
        any.a_int = i + 1;
        add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE,
                 helpbuf, MENU_UNSELECTED);
    }
    end_menu(tmpwin, "Select one item:");
    n = select_menu(tmpwin, PICK_ONE, &selected);
    destroy_nhwindow(tmpwin);
    if (n > 0) {
        sel = selected[0].item.a_int - 1;
        free((genericptr_t) selected);
        (void) (*help_menu_items[sel].f)();
    }
    return 0;
}

/* the 'V' command; also a choice for '?' */
int
dohistory()
{
    display_file(HISTORY, TRUE);
    return 0;
}

/*pager.c*/
