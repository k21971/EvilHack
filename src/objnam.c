/* NetHack 3.6	objnam.c	$NHDT-Date: 1674864732 2023/01/28 00:12:12 $  $NHDT-Branch: NetHack-3.6 $:$NHDT-Revision: 1.259 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2011. */
/* NetHack may be freely redistributed.  See license for details. */

/* EvilHack dev comment - I really hate objnam.c */

#include "hack.h"
#include <ctype.h>
#include <assert.h>

/* "an uncursed greased magical shimmering-scaled thoroughly rusty
    thoroughly corroded rustproof +0 exceptional [chain mail]" (107 characters) */
#define PREFIX 110
#define SCHAR_LIM 127
#define NUMOBUF 12

STATIC_DCL char *FDECL(strprepend, (char *, const char *));
STATIC_DCL short FDECL(rnd_otyp_by_wpnskill, (SCHAR_P));
STATIC_DCL short FDECL(rnd_otyp_by_namedesc, (const char *, CHAR_P, int));
STATIC_DCL boolean FDECL(wishymatch, (const char *, const char *, BOOLEAN_P));
STATIC_DCL char *NDECL(nextobuf);
STATIC_DCL void FDECL(releaseobuf, (char *));
STATIC_DCL void FDECL(xcalled, (char *, int, const char *, const char *));
STATIC_DCL char *FDECL(minimal_xname, (struct obj *));
STATIC_DCL void FDECL(add_erosion_words, (struct obj *, char *));
STATIC_DCL char *FDECL(doname_base, (struct obj *obj, unsigned));
STATIC_DCL char *FDECL(just_an, (char *str, const char *));
STATIC_DCL boolean FDECL(singplur_lookup, (char *, char *, BOOLEAN_P,
                                           const char *const *));
STATIC_DCL char *FDECL(singplur_compound, (char *));
STATIC_DCL char *FDECL(xname_flags, (struct obj *, unsigned));
STATIC_DCL boolean FDECL(badman, (const char *, BOOLEAN_P));
STATIC_DCL boolean FDECL(not_spec_material, (const char * const, int));
STATIC_DCL char *FDECL(globwt, (struct obj *, char *, boolean *));

struct Jitem {
    int item;
    const char *name;
};

#define BSTRCMPI(base, ptr, str) ((ptr) < base || strcmpi((ptr), str))
#define BSTRNCMPI(base, ptr, str, num) \
    ((ptr) < base || strncmpi((ptr), str, num))
#define Strcasecpy(dst, src) (void) strcasecpy(dst, src)

/* true for gems/rocks that should have " stone" appended to their names */
#define GemStone(typ) \
    (typ == FLINT                                                      \
     || ((objects[typ].oc_material == GEMSTONE                         \
         && (typ != DILITHIUM_CRYSTAL && typ != RUBY && typ != DIAMOND \
             && typ != SAPPHIRE && typ != BLACK_OPAL && typ != EMERALD \
             && typ != OPAL)) && objects[typ].oc_class == GEM_CLASS))

STATIC_OVL struct Jitem Japanese_items[] = { { SHORT_SWORD, "wakizashi" },
                                             { BROADSWORD, "ninja-to" },
                                             { FLAIL, "nunchaku" },
                                             { GLAIVE, "naginata" },
                                             { LOCK_PICK, "osaku" },
                                             { HARP, "koto" },
                                             { KNIFE, "shito" },
                                             { PLATE_MAIL, "tanko" },
                                             { HELMET, "kabuto" },
                                             { GLOVES, "yugake" },
                                             { GAUNTLETS, "tekko" },
                                             { FOOD_RATION, "gunyoki" },
                                             { POT_BOOZE, "sake" },
                                             { 0, "" } };

STATIC_DCL const char *FDECL(Japanese_item_name, (int i));

STATIC_OVL char *
strprepend(s, pref)
register char *s;
register const char *pref;
{
    register int i = (int) strlen(pref);

    if (i > PREFIX) {
        impossible("PREFIX too short (for %d).", i);
        return s;
    }
    s -= i;
    (void) strncpy(s, pref, i); /* do not copy trailing 0 */
    return s;
}

/* manage a pool of BUFSZ buffers, so callers don't have to */
static char NEARDATA obufs[NUMOBUF][BUFSZ];
static int obufidx = 0;

STATIC_OVL char *
nextobuf()
{
    obufidx = (obufidx + 1) % NUMOBUF;
    return obufs[obufidx];
}

/* put the most recently allocated buffer back if possible */
STATIC_OVL void
releaseobuf(bufp)
char *bufp;
{
    /* caller may not know whether bufp is the most recently allocated
       buffer; if it isn't, do nothing; note that because of the somewhat
       obscure PREFIX handling for object name formatting by xname(),
       the pointer our caller has and is passing to us might be into the
       middle of an obuf rather than the address returned by nextobuf() */
    if (bufp >= obufs[obufidx]
        && bufp < obufs[obufidx] + sizeof obufs[obufidx]) /* obufs[][BUFSZ] */
        obufidx = (obufidx - 1 + NUMOBUF) % NUMOBUF;
}

/* used by display_pickinv (invent.c, main whole-inventory routine) to
   release each successive doname() result in order to try to avoid
   clobbering all the obufs when 'perm_invent' is enabled and updated
   while one or more obufs have been allocated but not released yet */
void
maybereleaseobuf(obuffer)
char *obuffer;
{
    releaseobuf(obuffer);

    /*
     * An example from 3.6.x where all obufs got clobbered was when a
     * monster used a bullwhip to disarm the hero of a two-handed weapon:
     * "The ogre lord yanks Cleaver from your corpses!"
     |
     | hand = body_part(HAND);
     | if (use_plural)      // switches 'hand' from static buffer to an obuf
     |   hand = makeplural(hand);
      ...
     | release_worn_item(); // triggers full inventory update for perm_invent
      ...
     | pline(..., hand);    // the obuf[] for "hands" was clobbered with the
     |                      //+ partial formatting of an item from invent
     *
     * Another example was from writing a scroll without room in invent to
     * hold it after being split from a stack of blank scrolls:
     * "Oops!  food rations out of your grasp!"
     * hold_another_object() was passed 'the(aobjnam(newscroll, "slip"))'
     * as an argument and that should have yielded
     * "Oops!  The scroll of <foo> slips out of your grasp!"
     * but attempting to add the item to inventory triggered update for
     * perm_invent and the result from 'the(...)' was clobbered by partial
     * formatting of some inventory item.  [It happened in a shop and the
     * shk claimed ownership of the new scroll, but that wasn't relevant.]
     * That got fixed earlier, by delaying update_inventory() during
     * hold_another_object() rather than by avoiding using all the obufs.
     */
}

static boolean dump_prop_flag = FALSE;

/* identify object properties */
void
dump_prop_on()
{
    dump_prop_flag = TRUE;
}

void
dump_prop_off()
{
    dump_prop_flag = FALSE;
}

char *
obj_typename(otyp)
register int otyp;
{
    char *buf = nextobuf();
    struct objclass *ocl = &objects[otyp];
    const char *actualn = OBJ_NAME(*ocl);
    const char *dn = OBJ_DESCR(*ocl);
    const char *un = ocl->oc_uname;
    int nn = ocl->oc_name_known;

    if (Role_if(PM_SAMURAI) && Japanese_item_name(otyp))
        actualn = Japanese_item_name(otyp);
    buf[0] = '\0';
    switch (ocl->oc_class) {
    case COIN_CLASS:
        Strcpy(buf, "coin");
        break;
    case POTION_CLASS:
        Strcpy(buf, "potion");
        break;
    case SCROLL_CLASS:
        Strcpy(buf, "scroll");
        break;
    case WAND_CLASS:
        Strcpy(buf, "wand");
        break;
    case SPBOOK_CLASS:
        if (otyp != SPE_NOVEL) {
            Strcpy(buf, "spellbook");
        } else {
            Strcpy(buf, !nn ? "book" : "novel");
            nn = 0;
        }
        break;
    case RING_CLASS:
        Strcpy(buf, "ring");
        break;
    case AMULET_CLASS:
        if (nn)
            Strcpy(buf, actualn);
        else
            Strcpy(buf, "amulet");
        if (un)
            xcalled(buf, BUFSZ - (dn ? (int) strlen(dn) + 3 : 0), "", un);
        if (dn)
            Sprintf(eos(buf), " (%s)", dn);
        return buf;
    default:
        if (nn) {
            Strcpy(buf, actualn);
            if (GemStone(otyp))
                Strcat(buf, " stone");
            if (un) /* 3: length of " (" + ")" which will enclose 'dn' */
                xcalled(buf, BUFSZ - (dn ? (int) strlen(dn) + 3 : 0), "", un);
            if (dn)
                Sprintf(eos(buf), " (%s)", dn);
        } else {
            Strcpy(buf, dn ? dn : actualn);
            if (ocl->oc_class == GEM_CLASS)
                Strcat(buf,
                       (ocl->oc_material == MINERAL
                        || otyp == SLING_BULLET) ? " stone" : " gem");
            if (un)
                xcalled(buf, BUFSZ, "", un);
        }
        return buf;
    }
    /* here for ring/scroll/potion/wand */
    if (nn) {
        if (ocl->oc_unique)
            Strcpy(buf, actualn); /* avoid spellbook of Book of the Dead */
        else if (otyp == RIN_ANCIENT || otyp == RIN_LUSTROUS)
        /* also special-case artifact rings */
            Sprintf(buf, "%s ring", actualn);
        else
            Sprintf(eos(buf), " of %s", actualn);
    }
    if (un) /* 3: length of " (" + ")" which will enclose 'dn' */
        xcalled(buf, BUFSZ - (dn ? (int) strlen(dn) + 3 : 0), "", un);
    if (dn)
        Sprintf(eos(buf), " (%s)", dn);
    return buf;
}

/* less verbose result than obj_typename(); either the actual name
   or the description (but not both); user-assigned name is ignored */
char *
simple_typename(otyp)
int otyp;
{
    char *bufp, *pp, *save_uname = objects[otyp].oc_uname;

    objects[otyp].oc_uname = 0; /* suppress any name given by user */
    bufp = obj_typename(otyp);
    objects[otyp].oc_uname = save_uname;
    if ((pp = strstri(bufp, " (")) != 0)
        *pp = '\0'; /* strip the appended description */
    return bufp;
}

/* typename for debugging feedback where data involved might be suspect */
char *
safe_typename(otyp)
int otyp;
{
    unsigned save_nameknown;
    char *res = 0;

    if (otyp < STRANGE_OBJECT || otyp >= NUM_OBJECTS
        || !OBJ_NAME(objects[otyp])) {
        res = nextobuf();
        Sprintf(res, "glorkum[%d]", otyp);
    } else {
        /* force it to be treated as fully discovered */
        save_nameknown = objects[otyp].oc_name_known;
        objects[otyp].oc_name_known = 1;
        res = simple_typename(otyp);
        objects[otyp].oc_name_known = save_nameknown;
    }
    return res;
}

boolean
obj_is_pname(obj)
struct obj *obj;
{
    if (!obj->oartifact || !has_oname(obj))
        return FALSE;
    if (!program_state.gameover && !iflags.override_ID) {
        if (not_fully_identified(obj))
            return FALSE;
    }
    return TRUE;
}

/* used by distant_name() to pass extra information to xname_flags();
   it would be much cleaner if this were a parameter, but that would
   require all of the xname() and doname() calls to be modified */
static int distantname = 0;

/* Give the name of an object seen at a distance.  Unlike xname/doname,
 * we don't want to set dknown if it's not set already.
 */
char *
distant_name(obj, func)
struct obj *obj;
char *FDECL((*func), (OBJ_P));
{
    char *str;

    /* 3.6.1: this used to save Blind, set it, make the call, then restore
     * the saved value; but the Eyes of the Overworld override blindness
     * and let characters wearing them get dknown set for distant items.
     *
     * TODO? if the hero is wearing those Eyes, figure out whether the
     * object is within X-ray radius and only treat it as distant when
     * beyond that radius.  Logic is iffy but result might be interesting.
     */
    ++distantname;
    str = (*func)(obj);
    --distantname;
    return str;
}

/* convert player specified fruit name into corresponding fruit juice name
   ("slice of pizza" -> "pizza juice" rather than "slice of pizza juice") */
char *
fruitname(juice)
boolean juice; /* whether or not to append " juice" to the name */
{
    char *buf = nextobuf();
    const char *fruit_nam = strstri(pl_fruit, " of ");

    if (fruit_nam)
        fruit_nam += 4; /* skip past " of " */
    else
        fruit_nam = pl_fruit; /* use it as is */

    Sprintf(buf, "%s%s", makesingular(fruit_nam), juice ? " juice" : "");
    return buf;
}

/* look up a named fruit by index (1..127) */
struct fruit *
fruit_from_indx(indx)
int indx;
{
    struct fruit *f;

    for (f = ffruit; f; f = f->nextf)
        if (f->fid == indx)
            break;
    return f;
}

/* look up a named fruit by name */
struct fruit *
fruit_from_name(fname, exact, highest_fid)
const char *fname;
boolean exact; /* False => prefix or exact match, True = exact match only */
int *highest_fid; /* optional output; only valid if 'fname' isn't found */
{
    struct fruit *f, *tentativef;
    char *altfname;
    unsigned k;
    /*
     * note: named fruits are case-senstive...
     */

    if (highest_fid)
        *highest_fid = 0;
    /* first try for an exact match */
    for (f = ffruit; f; f = f->nextf)
        if (!strcmp(f->fname, fname))
            return f;
        else if (highest_fid && f->fid > *highest_fid)
            *highest_fid = f->fid;

    /* didn't match as-is; if caller is willing to accept a prefix
       match, try to find one; we want to find the longest prefix that
       matches, not the first */
    if (!exact) {
        tentativef = 0;
        for (f = ffruit; f; f = f->nextf) {
            k = strlen(f->fname);
            if (!strncmp(f->fname, fname, k)
                && (!fname[k] || fname[k] == ' ')
                && (!tentativef || k > strlen(tentativef->fname)))
                tentativef = f;
        }
        f = tentativef;
    }
    /* if we still don't have a match, try singularizing the target;
       for exact match, that's trivial, but for prefix, it's hard */
    if (!f) {
        altfname = makesingular(fname);
        for (f = ffruit; f; f = f->nextf) {
            if (!strcmp(f->fname, altfname))
                break;
        }
        releaseobuf(altfname);
    }
    if (!f && !exact) {
        char fnamebuf[BUFSZ], *p;
        unsigned fname_k = strlen(fname); /* length of assumed plural fname */

        tentativef = 0;
        for (f = ffruit; f; f = f->nextf) {
            k = strlen(f->fname);
            /* reload fnamebuf[] each iteration in case it gets modified;
               there's no need to recalculate fname_k */
            Strcpy(fnamebuf, fname);
            /* bug? if singular of fname is longer than plural,
               failing the 'fname_k > k' test could skip a viable
               candidate; unfortunately, we can't singularize until
               after stripping off trailing stuff and we can't get
               accurate fname_k until fname has been singularized;
               compromise and use 'fname_k >= k' instead of '>',
               accepting 1 char length discrepancy without risking
               false match (I hope...) */
            if (fname_k >= k && (p = index(&fnamebuf[k], ' ')) != 0) {
                *p = '\0'; /* truncate at 1st space past length of f->fname */
                altfname = makesingular(fnamebuf);
                k = strlen(altfname); /* actually revised 'fname_k' */
                if (!strcmp(f->fname, altfname)
                    && (!tentativef || k > strlen(tentativef->fname)))
                    tentativef = f;
                releaseobuf(altfname); /* avoid churning through all obufs */
            }
        }
        f = tentativef;
    }
    return f;
}

/* sort the named-fruit linked list by fruit index number */
void
reorder_fruit(forward)
boolean forward;
{
    struct fruit *f, *allfr[1 + 127];
    int i, j, k = SIZE(allfr);

    for (i = 0; i < k; ++i)
        allfr[i] = (struct fruit *) 0;
    for (f = ffruit; f; f = f->nextf) {
        /* without sanity checking, this would reduce to 'allfr[f->fid]=f' */
        j = f->fid;
        if (j < 1 || j >= k) {
            impossible("reorder_fruit: fruit index (%d) out of range", j);
            return; /* don't sort after all; should never happen... */
        } else if (allfr[j]) {
            impossible("reorder_fruit: duplicate fruit index (%d)", j);
            return;
        }
        allfr[j] = f;
    }
    ffruit = 0; /* reset linked list; we're rebuilding it from scratch */
    /* slot [0] will always be empty; must start 'i' at 1 to avoid
       [k - i] being out of bounds during first iteration */
    for (i = 1; i < k; ++i) {
        /* for forward ordering, go through indices from high to low;
           for backward ordering, go from low to high */
        j = forward ? (k - i) : i;
        if (allfr[j]) {
            allfr[j]->nextf = ffruit;
            ffruit = allfr[j];
        }
    }
}

STATIC_OVL
void
propnames(buf, props, props_known, weapon, has_of)
register char *buf;
register long props, props_known;
boolean weapon;
boolean has_of;
{
    char of[6];

    Strcpy(of, (has_of) ? " and" : " of");
    if (props & ITEM_FIRE) {
        if ((props_known & ITEM_FIRE)
            || dump_prop_flag) {
            Strcat(buf, of),
            Strcat(buf, weapon ? " fire" : " fire resistance"),
            Strcpy(of, " and");
        }
    }
    if (props & ITEM_FROST) {
        if ((props_known & ITEM_FROST)
            || dump_prop_flag) {
            Strcat(buf, of),
            Strcat(buf, weapon ? " frost" : " cold resistance"),
            Strcpy(of, " and");
        }
    }
    if (props & ITEM_DRLI) {
        if ((props_known & ITEM_DRLI)
            || dump_prop_flag) {
            Strcat(buf, of),
            Strcat(buf, weapon ? " decay" : " drain resistance"),
            Strcpy(of, " and");
        }
    }
    if (props & ITEM_SHOCK) {
        if ((props_known & ITEM_SHOCK)
            || dump_prop_flag) {
            Strcat(buf, of),
            Strcat(buf, weapon ? " lightning" : " shock resistance"),
            Strcpy(of, " and");
        }
    }
    if (props & ITEM_VENOM) {
        if ((props_known & ITEM_VENOM)
            || dump_prop_flag) {
            Strcat(buf, of),
            Strcat(buf, weapon ? " venom" : " poison resistance"),
            Strcpy(of, " and");
        }
    }
    if (props & ITEM_ESP) {
        if ((props_known & ITEM_ESP)
            || dump_prop_flag) {
            Strcat(buf, of),
            Strcat(buf, " telepathy"),
            Strcpy(of, " and");
        }
    }
    if (props & ITEM_SEARCHING) {
        if ((props_known & ITEM_SEARCHING)
            || dump_prop_flag) {
            Strcat(buf, of),
            Strcat(buf, " searching"),
            Strcpy(of, " and");
        }
    }
    if (props & ITEM_WARNING) {
        if ((props_known & ITEM_WARNING)
            || dump_prop_flag) {
            Strcat(buf, of),
            Strcat(buf, " warning"),
            Strcpy(of, " and");
        }
    }
    if (props & ITEM_FUMBLING) {
        if ((props_known & ITEM_FUMBLING)
            || dump_prop_flag) {
            Strcat(buf, of),
            Strcat(buf, " fumbling"),
            Strcpy(of, " and");
        }
    }
    if (props & ITEM_HUNGER) {
        if ((props_known & ITEM_HUNGER)
            || dump_prop_flag) {
            Strcat(buf, of),
            Strcat(buf, " hunger"),
            Strcpy(of, " and");
        }
    }
    if (props & ITEM_EXCEL) {
        if ((props_known & ITEM_EXCEL)
            || dump_prop_flag) {
            Strcat(buf, of),
            Strcat(buf, " excellence"),
            Strcpy(of, " and");
        }
    }
}

/* add "<pfx> called <sfx>" to end of buf, truncating if necessary */
STATIC_OVL void
xcalled(buf, siz, pfx, sfx)
char *buf;       /* eos(obuf) or eos(&obuf[PREFIX]) */
int siz;         /* BUFSZ or BUFSZ-PREFIX */
const char *pfx; /* usually class string, sometimes more specific */
const char *sfx; /* user assigned type name */
{
    int bufsiz = siz - 1 - (int) strlen(buf),
        pfxlen = (int) (strlen(pfx) + sizeof " called " - sizeof "");

    if (pfxlen > bufsiz)
        panic("xcalled: not enough room for prefix (%d > %d)",
              pfxlen, bufsiz);

    Sprintf(eos(buf), "%s called %.*s", pfx, bufsiz - pfxlen, sfx);
}

char *
xname(obj)
struct obj *obj;
{
    return xname_flags(obj, CXN_NORMAL);
}

/* Force rendering of materials on certain items where the object name
 * wouldn't make as much sense without a material (e.g. "leather jacket" vs
 * "jacket"), or those where the default material is non-obvious.
 * NB: GLOVES have a randomized description when not identified; "leather
 * padded gloves" would give the game away if we did not check their
 * identification status */
#define force_material_name(typ) \
    ((typ) == ARMOR || (typ) == STUDDED_ARMOR                     \
     || (typ) == JACKET || (typ) == CLOAK || (typ) == BRACERS     \
     || ((typ) == GLOVES && objects[GLOVES].oc_name_known)        \
     || ((typ) == GAUNTLETS && objects[GAUNTLETS].oc_name_known))

STATIC_OVL char *
xname_flags(obj, cxn_flags)
register struct obj *obj;
unsigned cxn_flags; /* bitmask of CXN_xxx values */
{
    register char *buf;
    char *obufp;
    register int typ = obj->otyp;
    register struct objclass *ocl = &objects[typ];
    int nn = ocl->oc_name_known, omndx = obj->corpsenm;
    long orig_opknwn = obj->oprops_known;
    const char *actualn = OBJ_NAME(*ocl);
    const char *dn = OBJ_DESCR(*ocl);
    const char *un = ocl->oc_uname;
    boolean pluralize = (obj->quan != 1L) && !(cxn_flags & CXN_SINGULAR);
    boolean known, dknown, bknown;

    buf = nextobuf() + PREFIX; /* leave room for "17 -3 " */
    if (Role_if(PM_SAMURAI) && Japanese_item_name(typ))
        actualn = Japanese_item_name(typ);
    /* 3.6.2: this used to be part of 'dn's initialization, but it
       needs to come after possibly overriding 'actualn' */
    if (!dn)
        dn = actualn;

    buf[0] = '\0';
    /*
     * clean up known when it's tied to oc_name_known, eg after AD_DRIN
     * This is only required for unique objects since the article
     * printed for the object is tied to the combination of the two
     * and printing the wrong article gives away information.
     */
    if (!nn && ocl->oc_uses_known && ocl->oc_unique)
        obj->known = 0;
    if (!Blind && !distantname)
        obj->dknown = 1;
    if (is_soko_prize_flag(obj))
        obj->dknown = 1;
    if (Role_if(PM_WIZARD)
        && ((obj->oprops & ITEM_PROP_MASK)
            || (objects[obj->otyp].oc_magic && !nn)
            || obj->oartifact))
        obj->oprops_known |= ITEM_MAGICAL;
    if (Role_if(PM_PRIEST))
        obj->bknown = 1; /* actively avoid set_bknown();
                          * we mustn't call update_inventory() now because
                          * it would call xname() (via doname()) recursively
                          * and could end up clobbering all the obufs... */

    if (iflags.override_ID) {
        known = dknown = bknown = TRUE;
        obj->oprops_known = ITEM_PROP_MASK;
        nn = 1;
    } else {
        known = obj->known;
        dknown = obj->dknown;
        bknown = obj->bknown;
    }

    if (obj_is_pname(obj))
        goto nameit;

    if (dknown && (obj->oprops_known & ITEM_MAGICAL)
        && (((obj->oprops && !(obj->oprops_known & ~ITEM_MAGICAL
                               || dump_prop_flag))
                && (!objects[obj->otyp].oc_magic
                    || !objects[obj->otyp].oc_name_known))
            || (!obj->oprops && objects[obj->otyp].oc_magic
                && !objects[obj->otyp].oc_name_known)
            || (obj->oartifact && not_fully_identified(obj))))
        Strcat(buf, "magical ");

    switch (obj->oclass) {
    case AMULET_CLASS:
        if (obj->material != objects[typ].oc_material && dknown) {
            Strcat(buf, materialnm[obj->material]);
            Strcat(buf, " ");
        }
        if (!dknown)
            Strcpy(buf, "amulet");
        else if (typ == AMULET_OF_YENDOR || typ == FAKE_AMULET_OF_YENDOR)
            /* each must be identified individually */
            Strcat(buf, known ? actualn : dn);
        else if (nn && !is_soko_prize_flag(obj))
            Strcat(buf, actualn);
        else if (un && !is_soko_prize_flag(obj))
            xcalled(buf, BUFSZ - PREFIX, "amulet", un);
        else if (is_soko_prize_flag(obj))
            Strcpy(buf, "sokoban prize amulet");
        else
            Sprintf(eos(buf), "%s amulet", dn);
        break;
    case WEAPON_CLASS:
        if (is_poisonable(obj) && obj->opoisoned)
            Strcat(buf, "poisoned ");
        else if (is_poisonable(obj) && obj->otainted)
            Strcat(buf, "tainted ");
        /*FALLTHRU*/
    case VENOM_CLASS:
    case TOOL_CLASS:
        if (typ == LENSES)
            Strcat(buf, "pair of ");
        else if (typ == GOGGLES)
            Strcat(buf, "pair of ");
        else if (is_barding(obj))
            Strcat(buf, "set of ");
        else if (is_wet_towel(obj))
            Strcat(buf, (obj->spe < 3) ? "moist " : "wet ");

        if (dknown && (obj->material != objects[typ].oc_material
                       || force_material_name(typ))) {
            Strcat(buf, materialnm[obj->material]);
            Strcat(buf, " ");
        }

        if (!dknown)
            Strcat(buf, dn);
        else if (nn && !is_soko_prize_flag(obj))
            Strcat(buf, actualn);
        else if (un && !is_soko_prize_flag(obj))
            xcalled(buf, BUFSZ - PREFIX, dn, un);
        else if (is_soko_prize_flag(obj))
            Strcpy(buf, "sokoban prize tool");
        else
            Strcat(buf, dn);

        if (is_barding(obj))
            propnames(buf, obj->oprops, obj->oprops_known,
                      FALSE, FALSE);
        else
            propnames(buf, obj->oprops, obj->oprops_known,
                      TRUE, !!strstri(buf, " of "));

        if (typ == BELL_OF_OPENING && !u.uevent.qcompleted)
            Strcat(buf, " (locked)");

        if (typ == FIGURINE && omndx != NON_PM) {
            char anbuf[10]; /* [4] would be enough: 'a','n',' ','\0' */

            Sprintf(eos(buf), " of %s%s",
                    just_an(anbuf, mons[omndx].mname),
                    mons[omndx].mname);
        } else if (is_wet_towel(obj)) {
            if (wizard)
                Sprintf(eos(buf), " (%d)", obj->spe);
        }
        break;
    case ARMOR_CLASS:
        /* depends on order of the dragon scales objects */
        if (typ >= FIRST_DRAGON_SCALES && typ <= LAST_DRAGON_SCALES) {
            Strcat(buf, "set of ");
            Strcat(buf, actualn);
            break;
        }

        if (is_boots(obj)
            || is_bracer(obj)
            || (is_gloves(obj) && typ != MUMMIFIED_HAND))
            Strcat(buf, "pair of ");

        if (dknown && (obj->oprops & ITEM_OILSKIN)
            && ((obj->oprops_known & ITEM_OILSKIN)
                || dump_prop_flag))
            Strcat(buf, "oilskin ");

        if ((obj->material != objects[typ].oc_material
             || force_material_name(typ)) && dknown) {
            Strcat(buf, materialnm[obj->material]);
            Strcat(buf, " ");
        }

        if (obj->otyp >= ELVEN_SHIELD && obj->otyp <= ORCISH_SHIELD
            && !is_bracer(obj) && !dknown) {
            Strcat(buf, "shield");
            propnames(buf, obj->oprops, obj->oprops_known,
                      FALSE, FALSE);
            break;
        }
        if (is_bracer(obj) && !dknown) {
            Strcat(buf, "bracers");
            propnames(buf, obj->oprops, obj->oprops_known,
                      FALSE, FALSE);
            break;
        }
        if (obj->otyp == SHIELD_OF_REFLECTION && !dknown) {
            Strcat(buf, "smooth shield");
            propnames(buf, obj->oprops, obj->oprops_known,
                      FALSE, FALSE);
            break;
        }

        if (nn && !is_soko_prize_flag(obj)) {
            Strcat(buf, actualn);
            propnames(buf, obj->oprops, obj->oprops_known,
                      FALSE, !!strstri(actualn, " of "));
        } else if (un && !is_soko_prize_flag(obj)) {
            if (is_boots(obj))
                Strcat(buf, "boots");
            else if (is_gloves(obj))
                Strcat(buf, "gloves");
            else if (is_cloak(obj))
                Strcat(buf, "cloak");
            else if (is_helmet(obj))
                Strcat(buf, helm_simple_name(obj));
            else if (is_shield(obj))
                Strcat(buf, "shield");
            else if (is_bracer(obj))
                Strcat(buf, "bracers");
            else
                Strcat(buf, "armor");
            propnames(buf, obj->oprops, obj->oprops_known,
                      FALSE, FALSE);
            Strcat(buf, " called ");
            Strcat(buf, un);
        } else if (is_soko_prize_flag(obj)) {
            Strcpy(buf, "sokoban prize armor");
        } else {
            Strcat(buf, dn);
            propnames(buf, obj->oprops, obj->oprops_known,
                      FALSE, !!strstri(dn, " of "));
        }
        break;
    case FOOD_CLASS:
        if (typ == SLIME_MOLD) {
            struct fruit *f = fruit_from_indx(obj->spe);

            if (!f) {
                impossible("Bad fruit #%d?", obj->spe);
                Strcat(buf, "fruit");
            } else {
                Strcat(buf, f->fname);
                if (pluralize) {
                    /* ick: already pluralized fruit names are allowed--we
                       want to try to avoid adding a redundant plural suffix;
                       double ick: makesingular() and makeplural() each use
                       and return an obuf but we don't want any particular
                       xname() call to consume more than one of those
                       [note: makeXXX() will be fully evaluated and done with
                       'buf' before strcpy() touches its output buffer] */
                    Strcpy(buf, obufp = makesingular(buf));
                    releaseobuf(obufp);
                    Strcpy(buf, obufp = makeplural(buf));
                    releaseobuf(obufp);
                    pluralize = FALSE;
                }
            }
            break;
        }
        if (obj->globby) {
            Sprintf(eos(buf), "%s%s",
                    (obj->owt <= 100)
                       ? "small "
                       : (obj->owt > 500)
                          ? "very large "
                          : (obj->owt > 300)
                             ? "large "
                             : "",
                    actualn);
            break;
        }
        if (obj->otyp == MEAT_BOOTS
            || obj->otyp == MEAT_GLOVES) {
            Strcat(buf, "pair of ");
            Strcat(buf, actualn);
            break;
        }

        Strcat(buf, actualn);
        if (typ == TIN && known)
            tin_details(obj, omndx, buf);
        break;
    case COIN_CLASS:
    case CHAIN_CLASS:
        Strcpy(buf, actualn);
        break;
    case ROCK_CLASS:
        if (typ == STATUE && omndx != NON_PM) {
            char anbuf[10], monnam[BUFSZ];
            *monnam = '\0';
            if (has_omonst(obj) && has_erac(OMONST(obj)))
                (void) racial_adj(monnam, OMONST(obj));
            Strcpy(eos(monnam), mons[omndx].mname);

            Sprintf(eos(buf), "%s%s of %s%s",
                    (Role_if(PM_ARCHEOLOGIST) && (obj->spe & STATUE_HISTORIC))
                       ? "historic "
                       : "",
                    actualn,
                    type_is_pname(&mons[omndx])
                       ? ""
                       : the_unique_pm(&mons[omndx])
                          ? "the "
                          : just_an(anbuf, mons[omndx].mname),
                    monnam);
        } else
            Strcat(buf, actualn);
        break;
    case BALL_CLASS:
        Sprintf(eos(buf), "%sheavy iron ball",
                (obj->owt > ocl->oc_weight) ? "very " : "");
        break;
    case POTION_CLASS:
        if (!dknown) {
            Strcat(buf, "potion");
            break;
        }
        if (obj->odiluted)
            Strcat(buf, "diluted ");
        if (nn || un) {
            Strcat(buf, "potion");
            if (nn) {
                Strcat(buf, " of ");
                if (typ == POT_WATER && bknown
                    && (obj->blessed || obj->cursed)) {
                    Strcat(buf, obj->blessed ? "holy " : "unholy ");
                }
                Strcat(buf, actualn);
            } else {
                xcalled(buf, BUFSZ - PREFIX, "", un);
            }
        } else {
            Strcat(buf, dn);
            Strcat(buf, " potion");
        }
        break;
    case SCROLL_CLASS:
        Strcat(buf, "scroll");
        if (!dknown)
            break;
        if (nn) {
            Strcat(buf, " of ");
            Strcat(buf, actualn);
        } else if (un) {
            xcalled(buf, BUFSZ - PREFIX, "", un);
        } else if (ocl->oc_magic) {
            Strcat(buf, " labeled ");
            Strcat(buf, dn);
        } else {
            Strcpy(buf, dn);
            Strcat(buf, " scroll");
        }
        break;
    case WAND_CLASS:
        if (!dknown)
            Strcpy(buf, "wand");
        else if (nn)
            Sprintf(eos(buf), "wand of %s", actualn);
        else if (un)
            xcalled(buf, BUFSZ - PREFIX, "wand", un);
        else
            Sprintf(eos(buf), "%s wand", dn);
        break;
    case SPBOOK_CLASS:
        if (typ == SPE_NOVEL) { /* 3.6 tribute */
            if (!dknown)
                Strcpy(buf, "book");
            else if (nn)
                Strcat(buf, actualn);
            else if (un)
                xcalled(buf, BUFSZ - PREFIX, "novel", un);
            else
                Sprintf(eos(buf), "%s book", dn);
            break;
            /* end of tribute */
        } else if (!dknown) {
            Strcpy(buf, "spellbook");
        } else if (nn) {
            if (typ != SPE_BOOK_OF_THE_DEAD)
                Strcat(buf, "spellbook of ");
            Strcat(buf, actualn);
        } else if (un) {
            xcalled(buf, BUFSZ - PREFIX, "spellbook", un);
        } else
            Sprintf(eos(buf), "%s spellbook", dn);
        break;
    case RING_CLASS:
        if (!dknown)
            Strcpy(buf, "ring");
        else if (nn) {
            if (typ != RIN_ANCIENT && typ != RIN_LUSTROUS)
                Sprintf(eos(buf), "ring of %s", actualn);
            else
                Sprintf(eos(buf), "%s ring", actualn);
        }
        else if (un)
            xcalled(buf, BUFSZ - PREFIX, "ring", un);
        else
            Sprintf(eos(buf), "%s ring", dn);
        break;
    case GEM_CLASS: {
        const char *rock = (ocl->oc_material == MINERAL
                            || typ == SLING_BULLET) ? "stone" : "gem";

        if (typ == SLING_BULLET) {
            if (obj->material != objects[obj->otyp].oc_material) {
                Strcat(buf, materialnm[obj->material]);
                Strcat(buf, " ");
            }
        }

        if (!dknown) {
            Strcpy(buf, rock);
        } else if (!nn) {
            if (un)
                xcalled(buf, BUFSZ - PREFIX, rock, un);
            else
                Sprintf(eos(buf), "%s %s", dn, rock);
        } else {
            Strcat(buf, actualn);
            if (GemStone(typ))
                Strcat(buf, " stone");
            propnames(buf, obj->oprops, obj->oprops_known,
                      TRUE, !!strstri(buf, " of "));
        }
        break;
    }
    default:
        Sprintf(buf, "glorkum %d %d %d", obj->oclass, typ, obj->spe);
        break;
    }
    if (pluralize) {
        /* (see fruit name handling in case FOOD_CLASS above) */
        Strcpy(buf, obufp = makeplural(buf));
        releaseobuf(obufp);
    }

    /* additional information shown during end-of-game inventory disclosure */
    if (obj->otyp == T_SHIRT && program_state.gameover
        /* o_id will be 0 for wizard mode attribute disclosure (formatted
           via from_what() -> ysimple_name() -> minimal_xname() -> xname()
           by the enlightenment code; moot here since T-shirts don't affect
           any attributes (but for 3.7, aprons will have comparable text) */
        && obj->o_id
        /* distantname is non-0 for do_screen_description() which is used
           to produce tooltip text for HTMLDUMP (not supported by nethack) */
        && !distantname
        ) {
        char tmpbuf[BUFSZ];

        /* disclose without breaking illiterate conduct, but mainly tip off
           players who aren't aware that something readable is present */
        switch (obj->otyp) {
        case T_SHIRT:
        case ALCHEMY_SMOCK:
        case STRIPED_SHIRT:
            Sprintf(eos(buf), " with text \"%s\"",
                    (obj->otyp == T_SHIRT)
                        ? tshirt_text(obj, tmpbuf)
                        : (obj->otyp == STRIPED_SHIRT)
                            ? striped_text(obj, tmpbuf)
                            : apron_text(obj, tmpbuf));
            break;
        case HAWAIIAN_SHIRT:
            Sprintf(eos(buf), " with %s motif", an(hawaiian_motif(obj, tmpbuf)));
            break;
        default:
            break;
        }
    }

    if (has_oname(obj) && dknown) {
        Strcat(buf, " named ");
 nameit:
        (void) strncat(buf, ONAME(obj),
                       BUFSZ - 1 - PREFIX - (unsigned) strlen(buf));
    }

    if (!strncmpi(buf, "the ", 4))
        buf += 4;

    /* reset oprops_known to avoid side-effects from debug identification
     * command (^I) */
    obj->oprops_known = orig_opknwn;

    return buf;
}

/* similar to simple_typename but minimal_xname operates on a particular
   object rather than its general type; it formats the most basic info:
     potion                     -- if description not known
     brown potion               -- if oc_name_known not set
     potion of object detection -- if discovered
 */
STATIC_OVL char *
minimal_xname(obj)
struct obj *obj;
{
    char *bufp;
    struct obj bareobj;
    struct objclass saveobcls;
    int otyp = obj->otyp;

    /* suppress user-supplied name */
    saveobcls.oc_uname = objects[otyp].oc_uname;
    objects[otyp].oc_uname = 0;
    /* suppress actual name if object's description is unknown */
    saveobcls.oc_name_known = objects[otyp].oc_name_known;
    if (!obj->dknown)
        objects[otyp].oc_name_known = 0;

    /* caveat: this makes a lot of assumptions about which fields
       are required in order for xname() to yield a sensible result */
    bareobj = zeroobj;
    bareobj.otyp = otyp;
    bareobj.oclass = obj->oclass;
    bareobj.dknown = obj->dknown;
    /* suppress known except for amulets (needed for fakes and real A-of-Y) */
    bareobj.known = (obj->oclass == AMULET_CLASS)
                        ? obj->known
                        /* default is "on" for types which don't use it */
                        : !objects[otyp].oc_uses_known;
    bareobj.quan = 1L;         /* don't want plural */
    bareobj.corpsenm = NON_PM; /* suppress statue and figurine details */
    /* but suppressing fruit details leads to "bad fruit #0"
       [perhaps we should force "slime mold" rather than use xname?] */
    if (obj->otyp == SLIME_MOLD)
        bareobj.spe = obj->spe;
    /* in the interest of minimalism, don't show this specific object's
     * material, unless the material is always included in the name. */
    bareobj.material = force_material_name(obj->otyp)
                        ? obj->material : objects[obj->otyp].oc_material;

    bufp = distant_name(&bareobj, xname); /* xname(&bareobj) */
    if (!strncmp(bufp, "uncursed ", 9))
        bufp += 9; /* Role_if(PM_PRIEST) */

    objects[otyp].oc_uname = saveobcls.oc_uname;
    objects[otyp].oc_name_known = saveobcls.oc_name_known;
    return bufp;
}

/* xname() output augmented for multishot missile feedback */
char *
mshot_xname(obj)
struct obj *obj;
{
    char tmpbuf[BUFSZ];
    char *onm = xname(obj);

    if (m_shot.n > 1 && m_shot.o == obj->otyp) {
        /* "the Nth arrow"; value will eventually be passed to an() or
           The(), both of which correctly handle this "the " prefix */
        Sprintf(tmpbuf, "the %d%s ", m_shot.i, ordin(m_shot.i));
        onm = strprepend(onm, tmpbuf);
    }
    return onm;
}

/* used for naming "the unique_item" instead of "a unique_item" */
boolean
the_unique_obj(obj)
struct obj *obj;
{
    boolean known = (obj->known || iflags.override_ID);

    if (!obj->dknown && !iflags.override_ID)
        return FALSE;
    else if (obj->otyp == FAKE_AMULET_OF_YENDOR && !known)
        return TRUE; /* lie */
    else
        return (boolean) (objects[obj->otyp].oc_unique
                          && (known || obj->otyp == AMULET_OF_YENDOR));
}

/* should monster type be prefixed with "the"? (mostly used for corpses) */
boolean
the_unique_pm(ptr)
struct permonst *ptr;
{
    boolean uniq;

    /* even though monsters with personal names are unique, we want to
       describe them as "Name" rather than "the Name" */
    if (type_is_pname(ptr))
        return FALSE;

    uniq = (ptr->geno & G_UNIQ) ? TRUE : FALSE;
    /* high priest is unique if it includes "of <deity>", otherwise not
       (caller needs to handle the 1st possibility; we assume the 2nd);
       worm tail should be irrelevant but is included for completeness */
    if (ptr == &mons[PM_HIGH_PRIEST] || ptr == &mons[PM_LONG_WORM_TAIL])
        uniq = FALSE;
    /* Wizard no longer needs this; he's flagged as unique these days */
    if (ptr == &mons[PM_WIZARD_OF_YENDOR])
        uniq = TRUE;
    return uniq;
}

STATIC_OVL void
add_erosion_words(obj, prefix)
struct obj *obj;
char *prefix;
{
    boolean iscrys = (obj->otyp == CRYSKNIFE),
            rknown;

    rknown = (iflags.override_ID == 0) ? obj->rknown : TRUE;

    /* The only cases where any of these bits do double duty are for
     * rotted food and diluted potions, which are all not is_damageable().
     */
    if (obj->oeroded && !iscrys) {
        switch (obj->oeroded) {
        case 2:
            Strcat(prefix, "very ");
            break;
        case 3:
            Strcat(prefix, "thoroughly ");
            break;
        }
        Strcat(prefix, is_rustprone(obj) ? "rusty " :
               obj->oclass == FOOD_CLASS ? "decayed " : "burnt ");
    }
    if (obj->oeroded2 && !iscrys && obj->oclass != FOOD_CLASS) {
        switch (obj->oeroded2) {
        case 2:
            Strcat(prefix, "very ");
            break;
        case 3:
            Strcat(prefix, "thoroughly ");
            break;
        }
        Strcat(prefix, !is_damageable(obj) ? "deteriorated " :
               is_glass(obj) ? "fractured " :
               is_corrodeable(obj) ? "corroded " : "rotted ");
    }
    if (rknown && obj->oerodeproof) {
        if (iscrys)
            Strcat(prefix, "fixed ");
        else if (is_glass(obj))
            Strcat(prefix, "tempered ");
        else if (is_rustprone(obj))
            Strcat(prefix, "rustproof ");
        else if (is_corrodeable(obj))
            Strcat(prefix, "corrodeproof ");
        else if (obj->oclass == FOOD_CLASS)
            Strcat(prefix, "preserved ");
        else if (is_flammable(obj))
            Strcat(prefix, "fireproof ");
    }
}

/* used to prevent rust on items where rust makes no difference */
boolean
erosion_matters(obj)
struct obj *obj;
{
    switch (obj->oclass) {
    case TOOL_CLASS:
        /* it's possible for a rusty weptool to be polymorphed into some
           non-weptool iron tool, in which case the rust implicitly goes
           away, but it's also possible for it to be polymorphed into a
           non-iron tool, in which case rust also implicitly goes away,
           so there's no particular reason to try to handle the first
           instance differently [this comment belongs in poly_obj()...] */
        /* return is_weptool(obj) ? TRUE : FALSE; */
    case WEAPON_CLASS:
    case ARMOR_CLASS:
    case BALL_CLASS:
    case CHAIN_CLASS:
    case AMULET_CLASS:
    case RING_CLASS:
    case WAND_CLASS:
    case FOOD_CLASS:
        return TRUE;
    case GEM_CLASS:
        return (obj->otyp == SLING_BULLET);
    default:
        break;
    }
    return FALSE;
}

#define DONAME_WITH_PRICE 1
#define DONAME_VAGUE_QUAN 2

STATIC_OVL char *
doname_base(obj, doname_flags)
struct obj *obj;
unsigned doname_flags;
{
    boolean ispoisoned = FALSE, istainted = FALSE,
            with_price = (doname_flags & DONAME_WITH_PRICE) != 0,
            vague_quan = (doname_flags & DONAME_VAGUE_QUAN) != 0,
            weightshown = FALSE;
    boolean known, dknown, cknown, bknown, lknown;
    long orig_opknwn = obj->oprops_known;
    int omndx = obj->corpsenm;
    char prefix[PREFIX], globbuf[QBUFSZ];
    char tmpbuf[PREFIX + 1]; /* for when we have to add something at
                                the start of prefix instead of the
                                end (Strcat is used on the end) */
    register char *bp = xname(obj);
    /* in practice, if something is worn or wielded by a hand, it is
       probably yours if it isn't specifically a monster's. there are
       some cases where an object is temporarily freed from inventory
       while still wielded, for instance */
    struct monst *owner =  mcarried(obj) ? obj->ocarry : &youmonst;

    if (iflags.override_ID) {
        known = dknown = cknown = bknown = lknown = TRUE;
        obj->oprops_known = ITEM_PROP_MASK;
    } else {
        known = obj->known;
        dknown = obj->dknown;
        cknown = obj->cknown;
        bknown = obj->bknown;
        lknown = obj->lknown;
    }

    /* When using xname, we want "poisoned arrow", and when using
     * doname, we want "poisoned +0 arrow".  This kludge is about the only
     * way to do it, at least until someone overhauls xname() and doname(),
     * combining both into one function taking a parameter. Known artifacts
     * bypass the poisoned/tainted/etc additions, so it may be no characters
     * have to be eaten.
     */
    /* must check opoisoned--someone can have a weirdly-named fruit */
    if (obj->opoisoned) {
        if (!strncmp(bp, "poisoned ", 9))
            bp += 9;
        ispoisoned = TRUE;
    }

    /* same check for otainted */
    if (obj->otainted) {
        if (!strncmp(bp, "tainted ", 8))
            bp += 8;
        istainted = TRUE;
    }

    if (obj->quan != 1L) {
        if (dknown || !vague_quan)
            Sprintf(prefix, "%ld ", obj->quan);
        else
            Strcpy(prefix, "some ");
    } else if (obj->otyp == CORPSE) {
        /* skip article prefix for corpses [else corpse_xname()
           would have to be taught how to strip it off again] */
        *prefix = '\0';
    } else if (obj_is_pname(obj) || the_unique_obj(obj)) {
        if (!strncmpi(bp, "the ", 4))
            bp += 4;
        Strcpy(prefix, "the ");
    } else {
        Strcpy(prefix, "a ");
    }

    /* "empty" goes at the beginning, but item count goes at the end */
    if (cknown
        /* bag of tricks: include "empty" prefix if it's known to
           be empty but its precise number of charges isn't known
           (when that is known, suffix of "(n:0)" will be appended,
           making the prefix be redundant; note that 'known' flag
           isn't set when emptiness gets discovered because then
           charging magic would yield known number of new charges);
           horn of plenty isn't a container but is close enough */
        && ((obj->otyp == BAG_OF_TRICKS || obj->otyp == HORN_OF_PLENTY)
             ? (obj->spe == 0 && !known)
             /* not a bag of tricks or horn of plenty: it's empty if
                it is a container that has no contents */
             : ((Is_container(obj) || obj->otyp == STATUE)
                && !Has_contents(obj))))
        Strcat(prefix, "empty ");

    if (bknown && obj->oclass != COIN_CLASS
        && (obj->otyp != POT_WATER || !objects[POT_WATER].oc_name_known
            || (!obj->cursed && !obj->blessed) || !dknown)) {
        /* allow 'blessed clear potion' if we don't know it's holy water;
         * always allow "uncursed potion of water;"
         * always show "blessed/cursed potion" if potion type is unknown.
         */
        if (obj->cursed)
            Strcat(prefix, "cursed ");
        else if (obj->blessed)
            Strcat(prefix, "blessed ");
        else if (!iflags.implicit_uncursed
            /* For most items with charges or +/-, if you know how many
             * charges are left or what the +/- is, then you must have
             * totally identified the item, so "uncursed" is unnecessary,
             * because an identified object not described as "blessed" or
             * "cursed" must be uncursed.
             *
             * If the charges or +/- is not known, "uncursed" must be
             * printed to avoid ambiguity between an item whose curse
             * status is unknown, and an item known to be uncursed.
             */
                 || ((!known || !objects[obj->otyp].oc_charged
                      || obj->oclass == ARMOR_CLASS
                      || obj->oclass == RING_CLASS)
#ifdef MAIL
                     && obj->otyp != SCR_MAIL
#endif
                     && obj->otyp != FAKE_AMULET_OF_YENDOR
                     && obj->otyp != AMULET_OF_YENDOR
                     && !Role_if(PM_PRIEST)))
            Strcat(prefix, "uncursed ");
    }

    if (lknown && Is_box(obj)) {
        if (obj->obroken)
            /* 3.6.0 used "unlockable" here but that could be misunderstood
               to mean "capable of being unlocked" rather than the intended
               "not capable of being locked" */
            Strcat(prefix, "broken ");
        else if (obj->olocked)
            Strcat(prefix, "locked ");
        else
            Strcat(prefix, "unlocked ");
    }

    if (obj->greased)
        Strcat(prefix, "greased ");

    if (cknown && Has_contents(obj)) {
        /* we count the number of separate stacks, which corresponds
           to the number of inventory slots needed to be able to take
           everything out if no merges occur */
        long itemcount = count_contents(obj, FALSE, FALSE, TRUE, FALSE);

        Sprintf(eos(bp), " containing %ld item%s", itemcount,
                plur(itemcount));
    }

    switch (is_weptool(obj) ? WEAPON_CLASS : obj->oclass) {
    case AMULET_CLASS:
        add_erosion_words(obj, prefix);
        if (obj->owornmask & W_AMUL)
            Strcat(bp, " (being worn)");
        break;
    case ARMOR_CLASS:
        if (obj->owornmask & W_ARMOR) {
            Strcat(bp, (obj == uskin) ? " (embedded in your skin)"
                       /* in case of perm_invent update while Wear/Takeoff
                          is in progress; check doffing() before donning()
                          because donning() returns True for both cases */
                       : doffing(obj) ? " (being doffed)"
                         : donning(obj) ? " (being donned)"
                           : ((druid_form || vampire_form)
                              && (owner == &youmonst))
                               ? " (merged to your form)"
                             : obj->otyp == MUMMIFIED_HAND ? " "
                               : " (being worn)");
            /* slippery fingers is an intrinsic condition of the hero
               rather than extrinsic condition of objects, but gloves
               are described as slippery when hero has slippery fingers */
            if (obj == uarmg
                && obj->otyp != MUMMIFIED_HAND && Glib) {
                /* just appended "(something)",
                 * change to "(something; slippery)" */
                Strcpy(rindex(bp, ')'), "; slippery)");
            } else if (obj->otyp == MUMMIFIED_HAND) {
                /* monsters don't wear these, so assuming it's yours is fine */
                if (Glib)
                    Sprintf(rindex(bp, ' '), " (merged to your left %s; slippery)",
                            body_part(ARM));
                else
                    Sprintf(rindex(bp, ' '), " (merged to your left %s)",
                            body_part(ARM));
            }
        }
        if (Is_dragon_scaled_armor(obj)) {
            char scalebuf[30], *colorstr = dragon_scales_color(obj);
            Sprintf(scalebuf, "%s-scaled ", colorstr);
            Strcat(prefix, scalebuf);
            releaseobuf(colorstr); /* don't consume an extra obuf */
        }
        /*FALLTHRU*/
    case WEAPON_CLASS:
        if (ispoisoned)
            Strcat(prefix, "poisoned ");
        if (istainted)
            Strcat(prefix, "tainted ");
        add_erosion_words(obj, prefix);
        if (known) {
            Strcat(prefix, sitoa(obj->spe));
            Strcat(prefix, " ");
        }
        if (known
            || Role_if(PM_SAMURAI) || Role_if(PM_KNIGHT)) {
            if (obj->forged_qual == FQ_SUPERIOR)
                Strcat(prefix, "superior ");
            else if (obj->forged_qual == FQ_EXCEPTIONAL)
                Strcat(prefix, "exceptional ");
            else if (obj->forged_qual == FQ_INFERIOR)
                Strcat(prefix, "inferior ");
        }
        break;
    case TOOL_CLASS:
        add_erosion_words(obj, prefix);
        if (is_barding(obj) && known) {
            Strcat(prefix, sitoa(obj->spe));
            Strcat(prefix, " ");
        }
        if (is_barding(obj)
            && (known || Role_if(PM_SAMURAI)
                || Role_if(PM_KNIGHT))) {
            if (obj->forged_qual == FQ_SUPERIOR)
                Strcat(prefix, "superior ");
            else if (obj->forged_qual == FQ_EXCEPTIONAL)
                Strcat(prefix, "exceptional ");
            else if (obj->forged_qual == FQ_INFERIOR)
                Strcat(prefix, "inferior ");
        }
        if (obj->owornmask & (W_TOOL | W_SADDLE | W_BARDING)) { /* blindfold */
            Strcat(bp, " (being worn)");
            break;
        }
        if (obj->otyp == LEASH && obj->leashmon != 0) {
            struct monst *mlsh = find_mid(obj->leashmon, FM_FMON);

            if (!mlsh) {
                impossible("leashed monster not on this level");
                obj->leashmon = 0;
            } else {
                Sprintf(eos(bp), " (attached to %s)",
                        noit_mon_nam(mlsh));
            }
            break;
        }
        if (obj->otyp == CANDELABRUM_OF_INVOCATION) {
            if (!obj->spe)
                Strcpy(tmpbuf, "no");
            else
                Sprintf(tmpbuf, "%d", obj->spe);
            Sprintf(eos(bp), " (%s candle%s%s)", tmpbuf, plur(obj->spe),
                    !obj->lamplit ? " attached" : ", lit");
            break;
        } else if (obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP
                   || obj->otyp == LANTERN || Is_candle(obj)) {
            if (Is_candle(obj)
                && obj->age < 20L * (long) objects[obj->otyp].oc_cost)
                Strcat(prefix, "partly used ");
            if (obj->lamplit)
                Strcat(bp, " (lit)");
            break;
        }
        if (objects[obj->otyp].oc_charged)
            goto charges;
        break;
    case WAND_CLASS:
        add_erosion_words(obj, prefix);
 charges:
        if (known)
            Sprintf(eos(bp), " (%d:%d)", (int) obj->recharged, obj->spe);
        break;
    case POTION_CLASS:
        if (obj->otyp == POT_OIL && obj->lamplit)
            Strcat(bp, " (lit)");
        break;
    case RING_CLASS:
        add_erosion_words(obj, prefix);
 ring:
        if (obj->owornmask & W_RINGR)
            Strcat(bp, " (on right ");
        if (obj->owornmask & W_RINGL)
            Strcat(bp, " (on left ");
        if (obj->owornmask & W_RING) {
            Strcat(bp, mbodypart(owner, HAND));
            Strcat(bp, ")");
        }
        if (known && objects[obj->otyp].oc_charged) {
            Strcat(prefix, sitoa(obj->spe));
            Strcat(prefix, " ");
        }
        break;
    case FOOD_CLASS:
        if (obj->otyp == CORPSE && obj->odrained) {
            if (wizard && obj->oeaten < (unsigned) drain_level(obj)) {
                Strcat(prefix, "over-drained ");
            } else if (obj->oeaten > (unsigned) drain_level(obj)) {
                Strcat(prefix, "partly drained ");
            } else {
                Strcat(prefix, "drained ");
            }
        } else if (obj->oeaten) {
            Strcat(prefix, "partly eaten ");
        }
        /* draugr automatically know how rotted a corpse is */
        if (obj->otyp == CORPSE
            && (!(obj->corpsenm == PM_LICHEN
                  || obj->corpsenm == PM_LIZARD
                  || obj->corpsenm == PM_DEATH
                  || obj->corpsenm == PM_FAMINE
                  || obj->corpsenm == PM_PESTILENCE))
            && (!Upolyd && Race_if(PM_DRAUGR))) {
            if (rot_amount(obj) > 5L)
                Strcat(prefix, " very rancid ");
            else if (rot_amount(obj) > 3L)
                Strcat(prefix, " rancid ");
        }
        add_erosion_words(obj, prefix);
        if (obj->otyp == CORPSE) {
            /* (quan == 1) => want corpse_xname() to supply article,
               (quan != 1) => already have count or "some" as prefix;
               "corpse" is already in the buffer returned by xname() */
            unsigned cxarg = (((obj->quan != 1L) ? 0 : CXN_ARTICLE)
                              | CXN_NOCORPSE);
            char *cxstr = corpse_xname(obj, prefix, cxarg);

            Sprintf(prefix, "%s ", cxstr);
            /* avoid having doname(corpse) consume an extra obuf */
            releaseobuf(cxstr);
        } else if (obj->otyp == EGG) {
#if 0 /* corpses don't tell if they're stale either */
            if (known && stale_egg(obj))
                Strcat(prefix, "stale ");
#endif
            if (omndx >= LOW_PM
                && (known || (mvitals[omndx].mvflags & MV_KNOWS_EGG))) {
                Strcat(prefix, mons[omndx].mname);
                Strcat(prefix, " ");
                if (obj->spe)
                    Strcat(bp, " (laid by you)");
            }
        }
        if (obj->otyp == MEAT_RING)
            goto ring;
        if ((obj == uarm && obj->otyp == MEAT_SUIT)
            || (obj == uarmh && obj->otyp == MEAT_HELMET)
            || (obj == uarms && obj->otyp == MEAT_SHIELD)
            || (obj == uarmg && obj->otyp == MEAT_GLOVES)
            || (obj == uarmf && obj->otyp == MEAT_BOOTS)) {
            Strcat(bp, (obj == uskin) ? " (embedded in your skin)"
                       : doffing(obj) ? " (being doffed)"
                         : donning(obj) ? " (being donned)"
                           : (druid_form || vampire_form) ? " (merged to your form)"
                             : " (being worn)");
        }
        break;
    case BALL_CLASS:
    case CHAIN_CLASS:
        add_erosion_words(obj, prefix);
        if (obj->owornmask & W_BALL)
            Strcat(bp, " (chained to you)");
        break;
    case GEM_CLASS:
        if (obj->otyp == SLING_BULLET)
            add_erosion_words(obj, prefix);
        break;
    }

    if ((obj->owornmask & W_WEP) && !mrg_to_wielded) {
        if (obj->quan != 1L) {
            Strcat(bp, " (wielded)");
        } else {
            const char *hand_s = mbodypart(owner, HAND);

            if (bimanual(obj))
                hand_s = makeplural(hand_s);
            /* note: Sting's glow message, if added, will insert text
               in front of "(weapon in hand)"'s closing paren */
            Sprintf(eos(bp), " (%sweapon in %s)",
                    (obj->otyp == AKLYS
                     || (obj->oartifact == ART_HAMMER_OF_THE_GODS
                         && P_SKILL(P_HAMMER) >= P_SKILLED)) ? "tethered "
                                                             : "", hand_s);
        }
    }
    if (obj->owornmask & W_SWAPWEP) {
        if (u.twoweap) {
            Sprintf(eos(bp), " (wielded in other %s)", mbodypart(owner, HAND));
        } else {
            Strcat(bp, " (alternate weapon; not wielded)");
        }
    }

    /* Various in-use light sources; overwrite trailing ')'. */
    if (!Blind
        && ((obj->owornmask & (W_ARMOR | W_ACCESSORY | W_WEP))
        || (u.twoweap && (obj->owornmask & W_SWAPWEP)))) {
        /* Warning glow from in-use artifacts. */
        if (obj->lastwarncnt
            && strcmp(glow_color(obj->oartifact), "no color")) {
            Sprintf(eos(bp) - 1, ", %s %s)",
                    glow_verb(obj->lastwarncnt, TRUE),
                    glow_color(obj->oartifact));

        /* Light/dark from always-lit objects */
        } else if (!Blind && obj->lamplit && artifact_light(obj)) {
            if ((obj->oartifact == ART_STAFF_OF_THE_ARCHMAGI
                 && !Upolyd && Race_if(PM_DROW))
                || (Is_dragon_armor(obj)
                    && (Dragon_armor_to_scales(obj) == SHADOW_DRAGON_SCALES)))
                Sprintf(eos(bp) - 1, ", aura of darkness)");
            else
                Sprintf(eos(bp) - 1, ", %s lit)",
                        arti_light_description(obj));
        }
    }

    if (obj->owornmask & W_QUIVER && !iflags.suppress_worn) {
        switch (obj->oclass) {
        case WEAPON_CLASS:
            if (is_ammo(obj)) {
                if (objects[obj->otyp].oc_skill == -P_BOW) {
                    /* Ammo for a bow */
                    Strcat(bp, " (in quiver)");
                    break;
                } else {
                    /* Ammo not for a bow */
                    Strcat(bp, " (in quiver pouch)");
                    break;
                }
            } else {
                /* Weapons not considered ammo */
                Strcat(bp, " (at the ready)");
                break;
            }
        /* Small things and ammo not for a bow */
        case RING_CLASS:
        case AMULET_CLASS:
        case WAND_CLASS:
        case COIN_CLASS:
        case GEM_CLASS:
            Strcat(bp, " (in quiver pouch)");
            break;
        default: /* odd things */
            Strcat(bp, " (at the ready)");
        }
    }
    /* treat 'restoring' like suppress_price because shopkeeper and
       bill might not be available yet while restore is in progress
       (objects won't normally be formatted during that time, but if
       'perm_invent' is enabled then they might be [not any more...]) */
    if (iflags.suppress_price || program_state.restoring) {
        ; /* don't attempt to obtain any stop pricing, even if 'with_price' */
    } else if (is_unpaid(obj)) { /* in inventory or in container in invent */
        long quotedprice = unpaid_cost(obj, TRUE);

        Sprintf(eos(bp), " (%s, %s%ld %s)",
                obj->unpaid ? "unpaid" : "contents",
                globwt(obj, globbuf, &weightshown),
                quotedprice, currency(quotedprice));
    } else if (with_price) { /* on floor or in container on floor */
        int nochrg = 0;
        long price = get_cost_of_shop_item(obj, &nochrg);

        if (price > 0L)
            Sprintf(eos(bp), " (%s, %s%ld %s)",
                    nochrg ? "contents" : "for sale",
                    globwt(obj, globbuf, &weightshown),
                    price, currency(price));
        else if (nochrg > 0)
            Sprintf(eos(bp), " (%sno charge)",
                    globwt(obj, globbuf, &weightshown));
    }
    if (!strncmp(prefix, "a ", 2)) {
        /* save current prefix, without "a "; might be empty */
        Strcpy(tmpbuf, prefix + 2);
        /* set prefix[] to "", "a ", or "an " */
        (void) just_an(prefix, *tmpbuf ? tmpbuf : bp);
        /* append remainder of original prefix */
        Strcat(prefix, tmpbuf);
    }

    /* show weight for items (debug tourist info);
       "aum" is stolen from Crawl's "Arbitrary Unit of Measure" */
    if (iflags.invweight && (obj->where == OBJ_INVENT || wizard)) {
        /* wizard mode user has asked to see object weights;
           globs with shop pricing attached already include it */
        if (!weightshown)
            Sprintf(eos(bp), " (%u aum)", obj->owt);
    }

    bp = strprepend(bp, prefix);

    obj->oprops_known = orig_opknwn;

    return bp;
}

char *
doname(obj)
struct obj *obj;
{
    return doname_base(obj, (unsigned) 0);
}

/* Name of object including price. */
char *
doname_with_price(obj)
struct obj *obj;
{
    return doname_base(obj, DONAME_WITH_PRICE);
}

/* "some" instead of precise quantity if obj->dknown not set */
char *
doname_vague_quan(obj)
struct obj *obj;
{
    /* Used by farlook.
     * If it hasn't been seen up close and quantity is more than one,
     * use "some" instead of the quantity: "some gold pieces" rather
     * than "25 gold pieces".  This is suboptimal, to put it mildly,
     * because lookhere and pickup report the precise amount.
     * Picking the item up while blind also shows the precise amount
     * for inventory display, then dropping it while still blind leaves
     * obj->dknown unset so the count reverts to "some" for farlook.
     *
     * TODO: add obj->qknown flag for 'quantity known' on stackable
     * items; it could overlay obj->cknown since no containers stack.
     */
    return doname_base(obj, DONAME_VAGUE_QUAN);
}

/* used from invent.c */
boolean
not_fully_identified(otmp)
struct obj *otmp;
{
    /* gold doesn't have any interesting attributes [yet?] */
    if (otmp->oclass == COIN_CLASS)
        return FALSE; /* always fully ID'd */
    /* check fundamental ID hallmarks first */
    if (!otmp->known || !otmp->dknown
#ifdef MAIL
        || (!otmp->bknown && otmp->otyp != SCR_MAIL)
#else
        || !otmp->bknown
#endif
        || !objects[otmp->otyp].oc_name_known)
        return TRUE;
    if ((!otmp->cknown && (Is_container(otmp) || otmp->otyp == STATUE))
        || (!otmp->lknown && Is_box(otmp)))
        return TRUE;
    if (((otmp->oprops_known & otmp->oprops) & ITEM_PROP_MASK) != otmp->oprops)
        return TRUE;
    if (otmp->oartifact && undiscovered_artifact(otmp->oartifact))
        return TRUE;
    /* otmp->rknown is the only item of interest if we reach here */
    /*
     *  Note:  if a revision ever expands fooproofing to scrolls, spellbooks,
     *  or potions, this checking will need to be revised.
     *  `rknown' ID only matters if xname() will provide the info about it.
     */
    if (otmp->rknown
        || otmp->oclass == SCROLL_CLASS || otmp->oclass == SPBOOK_CLASS
        || otmp->oclass == POTION_CLASS)
        return FALSE;
    else /* lack of `rknown' only matters for vulnerable objects */
        return (boolean) (is_damageable(otmp));
}

/* format a corpse name (xname() omits monster type; doname() calls us);
   eatcorpse() also uses us for death reason when eating tainted glob */
char *
corpse_xname(otmp, adjective, cxn_flags)
struct obj *otmp;
const char *adjective;
unsigned cxn_flags; /* bitmask of CXN_xxx values */
{
    /* some callers [aobjnam()] rely on prefix area that xname() sets aside */
    char *nambuf = nextobuf() + PREFIX;
    int omndx = otmp->corpsenm;
    boolean ignore_quan = (cxn_flags & CXN_SINGULAR) != 0,
            /* suppress "the" from "the unique monster corpse" */
        no_prefix = (cxn_flags & CXN_NO_PFX) != 0,
            /* include "the" for "the woodchuck corpse */
        the_prefix = (cxn_flags & CXN_PFX_THE) != 0,
            /* include "an" for "an ogre corpse */
        any_prefix = (cxn_flags & CXN_ARTICLE) != 0,
            /* leave off suffix (do_name() appends "corpse" itself) */
        omit_corpse = (cxn_flags & CXN_NOCORPSE) != 0,
        possessive = FALSE,
        glob = (otmp->otyp != CORPSE && otmp->globby);

    const char *mname;
    char mrace[BUFSZ];

    *mrace = '\0';
    if (has_omonst(otmp)) {
        (void) racial_adj(mrace, OMONST(otmp));
    }

    if (glob) {
        mname = OBJ_NAME(objects[otmp->otyp]); /* "glob of <monster>" */
    } else if (omndx == NON_PM) { /* paranoia */
        mname = "thing";
        /* [Possible enhancement:  check whether corpse has monster traits
            attached in order to use priestname() for priests and minions.] */
    } else if (omndx == PM_ALIGNED_PRIEST) {
        /* avoid "aligned priest"; it just exposes internal details */
        mname = "priest";
    } else {
        mname = mons[omndx].mname;
        if (the_unique_pm(&mons[omndx]) || type_is_pname(&mons[omndx])) {
            mname = s_suffix(mname);
            possessive = TRUE;
            /* don't precede personal name like "Medusa" with an article */
            if (type_is_pname(&mons[omndx]))
                no_prefix = TRUE;
            /* always precede non-personal unique monster name like
               "Oracle" with "the" unless explicitly overridden */
            else if (the_unique_pm(&mons[omndx]) && !no_prefix)
                the_prefix = TRUE;
        }
    }
    if (no_prefix)
        the_prefix = any_prefix = FALSE;
    else if (the_prefix)
        any_prefix = FALSE; /* mutually exclusive */

    *nambuf = '\0';
    /* can't use the() the way we use an() below because any capitalized
       Name causes it to assume a personal name and return Name as-is;
       that's usually the behavior wanted, but here we need to force "the"
       to precede capitalized unique monsters (pnames are handled above) */
    if (the_prefix)
        Strcat(nambuf, "the ");

    if (!adjective || !*adjective) {
        /* normal case:  newt corpse */
        Strcat(nambuf, mrace);
        Strcat(nambuf, mname);
    } else {
        /* adjective positioning depends upon format of monster name */
        if (possessive) /* Medusa's cursed partly eaten corpse */
            Sprintf(eos(nambuf), "%s%s %s", mrace, mname, adjective);
        else /* cursed partly eaten troll corpse */
            Sprintf(eos(nambuf), "%s %s%s", adjective, mrace, mname);
        /* in case adjective has a trailing space, squeeze it out */
        mungspaces(nambuf);
        /* doname() might include a count in the adjective argument;
           if so, don't prepend an article */
        if (digit(*adjective))
            any_prefix = FALSE;
    }

    if (glob) {
        ; /* omit_corpse doesn't apply; quantity is always 1 */
    } else if (!omit_corpse) {
        Strcat(nambuf, " corpse");
        /* makeplural(nambuf) => append "s" to "corpse" */
        if (otmp->quan > 1L && !ignore_quan) {
            Strcat(nambuf, "s");
            any_prefix = FALSE; /* avoid "a newt corpses" */
        }
    }

    /* it's safe to overwrite our nambuf[] after an() has copied its
       old value into another buffer; and once _that_ has been copied,
       the obuf[] returned by an() can be made available for re-use */
    if (any_prefix) {
        char *obufp;

        Strcpy(nambuf, obufp = an(nambuf));
        releaseobuf(obufp);
    }
    return nambuf;
}

/* xname doesn't include monster type for "corpse"; cxname does */
char *
cxname(obj)
struct obj *obj;
{
    if (obj->otyp == CORPSE)
        return corpse_xname(obj, (const char *) 0, CXN_NORMAL);
    return xname(obj);
}

/* like cxname, but ignores quantity */
char *
cxname_singular(obj)
struct obj *obj;
{
    if (obj->otyp == CORPSE)
        return corpse_xname(obj, (const char *) 0, CXN_SINGULAR);
    return xname_flags(obj, CXN_SINGULAR);
}

/* treat an object as fully ID'd when it might be used as reason for death */
char *
killer_xname(obj)
struct obj *obj;
{
    struct obj save_obj;
    unsigned save_ocknown;
    char *buf, *save_ocuname, *save_oname = (char *) 0;

    /* bypass object twiddling for artifacts */
    if (obj->oartifact)
        return bare_artifactname(obj);

    /* remember original settings for core of the object;
       oextra structs other than oname don't matter here--since they
       aren't modified they don't need to be saved and restored */
    save_obj = *obj;
    if (has_oname(obj))
        save_oname = ONAME(obj);

    /* killer name should be more specific than general xname; however, exact
       info like blessed/cursed and rustproof makes things be too verbose */
    obj->known = obj->dknown = 1;
    obj->bknown = obj->rknown = obj->greased = 0;
    /* if character is a priest[ess], bknown will get toggled back on */
    if (obj->otyp != POT_WATER)
        obj->blessed = obj->cursed = 0;
    else
        obj->bknown = 1; /* describe holy/unholy water as such */
    /* "killed by poisoned <obj>" would be misleading when poison is
       not the cause of death and "poisoned by poisoned <obj>" would
       be redundant when it is, so suppress "poisoned" prefix */
    obj->opoisoned = 0;
    /* same for "killed by tainted <obj>" */
    obj->otainted = 0;
    /* strip user-supplied name; artifacts keep theirs */
    if (!obj->oartifact && save_oname)
        ONAME(obj) = (char *) 0;
    /* temporarily identify the type of object */
    save_ocknown = objects[obj->otyp].oc_name_known;
    objects[obj->otyp].oc_name_known = 1;
    save_ocuname = objects[obj->otyp].oc_uname;
    objects[obj->otyp].oc_uname = 0; /* avoid "foo called bar" */

    /* format the object */
    if (obj->otyp == CORPSE) {
        buf = corpse_xname(obj, (const char *) 0, CXN_NORMAL);
    } else if (obj->otyp == SLIME_MOLD) {
        /* concession to "most unique deaths competition" in the annual
           devnull tournament, suppress player supplied fruit names because
           those can be used to fake other objects and dungeon features */
        buf = nextobuf();
        Sprintf(buf, "deadly slime mold%s", plur(obj->quan));
    } else {
        buf = xname(obj);
    }
    /* apply an article if appropriate; caller should always use KILLED_BY */
    if (obj->quan == 1L && !strstri(buf, "'s ") && !strstri(buf, "s' "))
        buf = (obj_is_pname(obj) || the_unique_obj(obj)) ? the(buf) : an(buf);

    objects[obj->otyp].oc_name_known = save_ocknown;
    objects[obj->otyp].oc_uname = save_ocuname;
    *obj = save_obj; /* restore object's core settings */
    if (!obj->oartifact && save_oname)
        ONAME(obj) = save_oname;

    return buf;
}

/* xname,doname,&c with long results reformatted to omit some stuff */
char *
short_oname(obj, func, altfunc, lenlimit)
struct obj *obj;
char *FDECL((*func), (OBJ_P)),    /* main formatting routine */
     *FDECL((*altfunc), (OBJ_P)); /* alternate for shortest result */
unsigned lenlimit;
{
    struct obj save_obj;
    char unamebuf[12], onamebuf[12], *save_oname, *save_uname, *outbuf;

    outbuf = (*func)(obj);
    if ((unsigned) strlen(outbuf) <= lenlimit)
        return outbuf;

    /* shorten called string to fairly small amount */
    save_uname = objects[obj->otyp].oc_uname;
    if (save_uname && strlen(save_uname) >= sizeof unamebuf) {
        (void) strncpy(unamebuf, save_uname, sizeof unamebuf - 4);
        Strcpy(unamebuf + sizeof unamebuf - 4, "...");
        objects[obj->otyp].oc_uname = unamebuf;
        releaseobuf(outbuf);
        outbuf = (*func)(obj);
        objects[obj->otyp].oc_uname = save_uname; /* restore called string */
        if ((unsigned) strlen(outbuf) <= lenlimit)
            return outbuf;
    }

    /* shorten named string to fairly small amount */
    save_oname = has_oname(obj) ? ONAME(obj) : 0;
    if (save_oname && strlen(save_oname) >= sizeof onamebuf) {
        (void) strncpy(onamebuf, save_oname, sizeof onamebuf - 4);
        Strcpy(onamebuf + sizeof onamebuf - 4, "...");
        ONAME(obj) = onamebuf;
        releaseobuf(outbuf);
        outbuf = (*func)(obj);
        ONAME(obj) = save_oname; /* restore named string */
        if ((unsigned) strlen(outbuf) <= lenlimit)
            return outbuf;
    }

    /* shorten both called and named strings;
       unamebuf and onamebuf have both already been populated */
    if (save_uname && strlen(save_uname) >= sizeof unamebuf && save_oname
        && strlen(save_oname) >= sizeof onamebuf) {
        objects[obj->otyp].oc_uname = unamebuf;
        ONAME(obj) = onamebuf;
        releaseobuf(outbuf);
        outbuf = (*func)(obj);
        if ((unsigned) strlen(outbuf) <= lenlimit) {
            objects[obj->otyp].oc_uname = save_uname;
            ONAME(obj) = save_oname;
            return outbuf;
        }
    }

    /* still long; strip several name-lengthening attributes;
       called and named strings are still in truncated form */
    save_obj = *obj;
    obj->bknown = obj->rknown = obj->greased = 0;
    obj->oeroded = obj->oeroded2 = 0;
    releaseobuf(outbuf);
    outbuf = (*func)(obj);
    if (altfunc && (unsigned) strlen(outbuf) > lenlimit) {
        /* still long; use the alternate function (usually one of
           the jackets around minimal_xname()) */
        releaseobuf(outbuf);
        outbuf = (*altfunc)(obj);
    }
    /* restore the object */
    *obj = save_obj;
    if (save_oname)
        ONAME(obj) = save_oname;
    if (save_uname)
        objects[obj->otyp].oc_uname = save_uname;

    /* use whatever we've got, whether it's too long or not */
    return outbuf;
}

/*
 * Used if only one of a collection of objects is named (e.g. in eat.c).
 */
const char *
singular(otmp, func)
register struct obj *otmp;
char *FDECL((*func), (OBJ_P));
{
    long savequan;
    char *nam;

    /* using xname for corpses does not give the monster type */
    if (otmp->otyp == CORPSE && func == xname)
        func = cxname;

    savequan = otmp->quan;
    otmp->quan = 1L;
    nam = (*func)(otmp);
    otmp->quan = savequan;
    return nam;
}

/* pick "", "a ", or "an " as article for 'str'; used by an() and doname() */
STATIC_OVL char *
just_an(outbuf, str)
char *outbuf;
const char *str;
{
    char c0;

    *outbuf = '\0';
    c0 = lowc(*str);
    if (!str[1]) {
        /* single letter; might be used for named fruit */
        Strcpy(outbuf, index("aefhilmnosx", c0) ? "an " : "a ");
    } else if (!strncmpi(str, "the ", 4) || !strcmpi(str, "molten lava")
               || !strcmpi(str, "iron bars") || !strcmpi(str, "ice")
               || !strcmpi(str, "grass") || !strcmpi(str, "sand")) {
        ; /* no article */
    } else {
        /* normal case is "an <vowel>" or "a <consonant>" */
        if ((index(vowels, c0) /* some exceptions warranting "a <vowel>" */
             /* 'wun' initial sound */
             && (strncmpi(str, "one", 3) || (str[3] && !index("-_ ", str[3])))
             /* long 'u' initial sound */
             && strncmpi(str, "eu", 2) /* "eucalyptus leaf" */
             && strncmpi(str, "uke", 3) && strncmpi(str, "ukulele", 7)
             && strncmpi(str, "unicorn", 7) && strncmpi(str, "uranium", 7)
             && strncmpi(str, "useful", 6)) /* "useful tool" */
            || (c0 == 'x' && !index(vowels, lowc(str[1]))))
            Strcpy(outbuf, "an ");
        else
            Strcpy(outbuf, "a ");
    }
    return outbuf;
}

char *
an(str)
const char *str;
{
    char *buf = nextobuf();

    if (!str || !*str) {
        impossible("Alphabet soup: 'an(%s)'.", str ? "\"\"" : "<null>");
        return strcpy(buf, "an []");
    }
    (void) just_an(buf, str);
    return strncat(buf, str, BUFSZ - 1 - (unsigned) strlen(buf));
}

char *
An(str)
const char *str;
{
    char *tmp = an(str);

    *tmp = highc(*tmp);
    return tmp;
}

/*
 * Prepend "the" if necessary; assumes str is a subject derived from xname.
 * Use type_is_pname() for monster names, not the().  the() is idempotent.
 */
char *
the(str)
const char *str;
{
    char *buf = nextobuf();
    boolean insert_the = FALSE;

    if (!str || !*str) {
        impossible("Alphabet soup: 'the(%s)'.", str ? "\"\"" : "<null>");
        return strcpy(buf, "the []");
    }
    if (!strncmpi(str, "the ", 4)) {
        buf[0] = lowc(*str);
        Strcpy(&buf[1], str + 1);
        return buf;
    } else if (*str < 'A' || *str > 'Z'
               /* treat named fruit as not a proper name, even if player
                  has assigned a capitalized proper name as his/her fruit */
               || fruit_from_name(str, TRUE, (int *) 0)) {
        /* not a proper name, needs an article */
        insert_the = TRUE;
    } else {
        /* Probably a proper name, might not need an article */
        register char *tmp, *named, *called;
        int l;

        /* some objects have capitalized adjectives in their names */
        if (((tmp = rindex(str, ' ')) != 0 || (tmp = rindex(str, '-')) != 0)
            && (tmp[1] < 'A' || tmp[1] > 'Z')) {
            insert_the = TRUE;
        } else if (tmp && index(str, ' ') < tmp) { /* has spaces */
            /* it needs an article if the name contains "of" */
            tmp = strstri(str, " of ");
            named = strstri(str, " named ");
            called = strstri(str, " called ");
            if (called && (!named || called < named))
                named = called;

            if (tmp && (!named || tmp < named)) /* found an "of" */
                insert_the = TRUE;
            /* stupid special case: lacks "of" but needs "the" */
            else if (!named && (l = strlen(str)) >= 31
                     && !strcmp(&str[l - 31],
                                "Platinum Yendorian Express Card"))
                insert_the = TRUE;
        }
    }
    if (insert_the)
        Strcpy(buf, "the ");
    else
        buf[0] = '\0';
    return strncat(buf, str, BUFSZ - 1 - (unsigned) strlen(buf));
}

char *
The(str)
const char *str;
{
    char *tmp = the(str);

    *tmp = highc(*tmp);
    return tmp;
}

/* returns "count cxname(otmp)" or just cxname(otmp) if count == 1 */
char *
aobjnam(otmp, verb)
struct obj *otmp;
const char *verb;
{
    char prefix[PREFIX];
    char *bp = cxname(otmp);

    if (otmp->quan != 1L) {
        Sprintf(prefix, "%ld ", otmp->quan);
        bp = strprepend(bp, prefix);
    }
    if (verb) {
        Strcat(bp, " ");
        Strcat(bp, otense(otmp, verb));
    }
    return bp;
}

/* combine yname and aobjnam eg "your count cxname(otmp)" */
char *
yobjnam(obj, verb)
struct obj *obj;
const char *verb;
{
    char *s = aobjnam(obj, verb);

    /* leave off "your" for most of your artifacts, but prepend
     * "your" for unique objects and "foo of bar" quest artifacts */
    if (!carried(obj) || !obj_is_pname(obj)
        || (obj->oartifact >= ART_XIUHCOATL
            && obj->oartifact != ART_GJALLAR)
        || obj->oartifact == ART_DRAGONBANE
        || obj->oartifact == ART_MAGIC___BALL
        || obj->oartifact == ART_WAND_OF_ORCUS
        || obj->oartifact == ART_BAG_OF_THE_HESPERIDES
        || obj->oartifact == ART_GAUNTLETS_OF_PURITY
        || obj->oartifact == ART_HAMMER_OF_THE_GODS) {
        char *outbuf = shk_your(nextobuf(), obj);
        int space_left = BUFSZ - 1 - strlen(outbuf);

        s = strncat(outbuf, s, space_left);
    }
    return s;
}

/* combine Yname2 and aobjnam eg "Your count cxname(otmp)" */
char *
Yobjnam2(obj, verb)
struct obj *obj;
const char *verb;
{
    register char *s = yobjnam(obj, verb);

    *s = highc(*s);
    return s;
}

/* like aobjnam, but prepend "The", not count, and use xname */
char *
Tobjnam(otmp, verb)
struct obj *otmp;
const char *verb;
{
    char *bp = The(xname(otmp));

    if (verb) {
        Strcat(bp, " ");
        Strcat(bp, otense(otmp, verb));
    }
    return bp;
}

/* capitalized variant of doname() */
char *
Doname2(obj)
struct obj *obj;
{
    char *s = doname(obj);

    *s = highc(*s);
    return s;
}

#if 0 /* stalled-out work in progress */
/* Doname2() for itemized buying of 'obj' from a shop */
char *
payDoname(obj)
struct obj *obj;
{
    static const char and_contents[] = " and its contents";
    char *p = doname(obj);

    if (Is_container(obj) && !obj->cknown) {
        if (obj->unpaid) {
            if ((int) strlen(p) + sizeof and_contents - 1 < BUFSZ - PREFIX)
                Strcat(p, and_contents);
            *p = highc(*p);
        } else {
            p = strprepend(p, "Contents of ");
        }
    } else {
        *p = highc(*p);
    }
    return p;
}
#endif /*0*/

/* returns "[your ]xname(obj)" or "Foobar's xname(obj)" or "the xname(obj)" */
char *
yname(obj)
struct obj *obj;
{
    char *s = cxname(obj);

    /* leave off "your" for most of your artifacts, but prepend
     * "your" for unique objects and "foo of bar" quest artifacts */
    if (!carried(obj) || !obj_is_pname(obj)
        || (obj->oartifact >= ART_XIUHCOATL
            && obj->oartifact != ART_GJALLAR)
        || obj->oartifact == ART_DRAGONBANE
        || obj->oartifact == ART_MAGIC___BALL
        || obj->oartifact == ART_WAND_OF_ORCUS
        || obj->oartifact == ART_BAG_OF_THE_HESPERIDES
        || obj->oartifact == ART_GAUNTLETS_OF_PURITY
        || obj->oartifact == ART_HAMMER_OF_THE_GODS) {
        char *outbuf = shk_your(nextobuf(), obj);
        int space_left = BUFSZ - 1 - strlen(outbuf);

        s = strncat(outbuf, s, space_left);
    }

    return s;
}

/* capitalized variant of yname() */
char *
Yname2(obj)
struct obj *obj;
{
    char *s = yname(obj);

    *s = highc(*s);
    return s;
}

/* returns "your minimal_xname(obj)"
 * or "Foobar's minimal_xname(obj)"
 * or "the minimal_xname(obj)"
 */
char *
ysimple_name(obj)
struct obj *obj;
{
    char *outbuf = nextobuf();
    char *s = shk_your(outbuf, obj); /* assert( s == outbuf ); */
    int space_left = BUFSZ - 1 - strlen(s);

    return strncat(s, minimal_xname(obj), space_left);
}

/* capitalized variant of ysimple_name() */
char *
Ysimple_name2(obj)
struct obj *obj;
{
    char *s = ysimple_name(obj);

    *s = highc(*s);
    return s;
}

/* "scroll" or "scrolls" */
char *
simpleonames(obj)
struct obj *obj;
{
    char *obufp, *simpleoname = minimal_xname(obj);

    if (obj->quan != 1L) {
        simpleoname = makeplural(obufp = simpleoname);
        releaseobuf(obufp);
    }
    return simpleoname;
}

/* "a scroll" or "scrolls"; "a silver bell" or "the Bell of Opening" */
char *
ansimpleoname(obj)
struct obj *obj;
{
    char *obufp, *simpleoname = simpleonames(obj);
    int otyp = obj->otyp;

    /* prefix with "the" if a unique item, or a fake one imitating same,
       has been formatted with its actual name (we let typename() handle
       any `known' and `dknown' checking necessary) */
    if (otyp == FAKE_AMULET_OF_YENDOR)
        otyp = AMULET_OF_YENDOR;
    if (objects[otyp].oc_unique && OBJ_NAME(objects[otyp])
        && !strcmp(simpleoname, OBJ_NAME(objects[otyp]))) {
        /* the() will allocate another obuf[]; we want to avoid using two */
        Strcpy(simpleoname, obufp = the(simpleoname));
        releaseobuf(obufp);
    } else if (obj->quan == 1L) {
        /* simpleoname[] is singular if quan==1, plural otherwise;
           an() will allocate another obuf[]; we want to avoid using two */
        Strcpy(simpleoname, obufp = an(simpleoname));
        releaseobuf(obufp);
    }
    return simpleoname;
}

/* "the scroll" or "the scrolls" */
char *
thesimpleoname(obj)
struct obj *obj;
{
    char *obufp, *simpleoname = simpleonames(obj);

    /* the() will allocate another obuf[]; we want to avoid using two */
    Strcpy(simpleoname, obufp = the(simpleoname));
    releaseobuf(obufp);
    return simpleoname;
}

/* artifact's name without any object type or known/dknown/&c feedback */
char *
bare_artifactname(obj)
struct obj *obj;
{
    char *outbuf;

    if (obj->oartifact) {
        outbuf = nextobuf();
        Strcpy(outbuf, artiname(obj->oartifact));
        if (!strncmp(outbuf, "The ", 4))
            outbuf[0] = lowc(outbuf[0]);
    } else {
        outbuf = xname(obj);
    }
    return outbuf;
}

static const char *wrp[] = {
    "wand",   "ring",      "potion",     "scroll", "gem",
    "amulet", "spellbook", "spell book",
    /* for non-specific wishes */
    "weapon", "armor",     "tool",       "food",   "comestible",
};
static const char wrpsym[] = { WAND_CLASS,   RING_CLASS,   POTION_CLASS,
                               SCROLL_CLASS, GEM_CLASS,    AMULET_CLASS,
                               SPBOOK_CLASS, SPBOOK_CLASS, WEAPON_CLASS,
                               ARMOR_CLASS,  TOOL_CLASS,   FOOD_CLASS,
                               FOOD_CLASS };

/* return form of the verb (input plural) if xname(otmp) were the subject */
char *
otense(otmp, verb)
struct obj *otmp;
const char *verb;
{
    char *buf;

    /*
     * verb is given in plural (without trailing s).  Return as input
     * if the result of xname(otmp) would be plural.  Don't bother
     * recomputing xname(otmp) at this time.
     */
    if (!is_plural(otmp))
        return vtense((char *) 0, verb);

    buf = nextobuf();
    Strcpy(buf, verb);
    return buf;
}

/* various singular words that vtense would otherwise categorize as plural;
   also used by makesingular() to catch some special cases */
static const char *const special_subjs[] = {
    "erinys",  "manes", /* this one is ambiguous */
    "Cyclops", "Hippocrates",     "Pelias",    "Kas",  "aklys",
    "amnesia", "detect monsters", "paralysis", "shape changers",
    "nemesis", "Mephistopheles", 0
    /* note: "detect monsters" and "shape changers" are normally
       caught via "<something>(s) of <whatever>", but they can be
       wished for using the shorter form, so we include them here
       to accommodate usage by makesingular during wishing */
};

/* return form of the verb (input plural) for present tense 3rd person subj */
char *
vtense(subj, verb)
register const char *subj;
register const char *verb;
{
    char *buf = nextobuf(), *bspot;
    int len, ltmp;
    const char *sp, *spot;
    const char *const *spec;

    /*
     * verb is given in plural (without trailing s).  Return as input
     * if subj appears to be plural.  Add special cases as necessary.
     * Many hard cases can already be handled by using otense() instead.
     * If this gets much bigger, consider decomposing makeplural.
     * Note: monster names are not expected here (except before corpse).
     *
     * Special case: allow null sobj to get the singular 3rd person
     * present tense form so we don't duplicate this code elsewhere.
     */
    if (subj) {
        if (!strncmpi(subj, "a ", 2) || !strncmpi(subj, "an ", 3))
            goto sing;
        spot = (const char *) 0;
        for (sp = subj; (sp = index(sp, ' ')) != 0; ++sp) {
            if (!strncmpi(sp, " of ", 4) || !strncmpi(sp, " from ", 6)
                || !strncmpi(sp, " called ", 8) || !strncmpi(sp, " named ", 7)
                || !strncmpi(sp, " labeled ", 9)) {
                if (sp != subj)
                    spot = sp - 1;
                break;
            }
        }
        len = (int) strlen(subj);
        if (!spot)
            spot = subj + len - 1;

        /*
         * plural: anything that ends in 's', but not '*us' or '*ss'.
         * Guess at a few other special cases that makeplural creates.
         */
        if ((lowc(*spot) == 's' && spot != subj
             && !index("us", lowc(*(spot - 1))))
            || !BSTRNCMPI(subj, spot - 3, "eeth", 4)
            || !BSTRNCMPI(subj, spot - 3, "feet", 4)
            || !BSTRNCMPI(subj, spot - 1, "ia", 2)
            || !BSTRNCMPI(subj, spot - 1, "ae", 2)) {
            /* check for special cases to avoid false matches */
            len = (int) (spot - subj) + 1;
            for (spec = special_subjs; *spec; spec++) {
                ltmp = strlen(*spec);
                if (len == ltmp && !strncmpi(*spec, subj, len))
                    goto sing;
                /* also check for <prefix><space><special_subj>
                   to catch things like "the invisible erinys" */
                if (len > ltmp && *(spot - ltmp) == ' '
                    && !strncmpi(*spec, spot - ltmp + 1, ltmp))
                    goto sing;
            }

            return strcpy(buf, verb);
        }
        /*
         * 3rd person plural doesn't end in telltale 's';
         * 2nd person singular behaves as if plural.
         */
        if (!strcmpi(subj, "they") || !strcmpi(subj, "you"))
            return strcpy(buf, verb);
    }

 sing:
    Strcpy(buf, verb);
    len = (int) strlen(buf);
    bspot = buf + len - 1;

    if (!strcmpi(buf, "are")) {
        Strcasecpy(buf, "is");
    } else if (!strcmpi(buf, "have")) {
        Strcasecpy(bspot - 1, "s");
    } else if (index("zxs", lowc(*bspot))
               || (len >= 2 && lowc(*bspot) == 'h'
                   && index("cs", lowc(*(bspot - 1))))
               || (len == 2 && lowc(*bspot) == 'o')) {
        /* Ends in z, x, s, ch, sh; add an "es" */
        Strcasecpy(bspot + 1, "es");
    } else if (lowc(*bspot) == 'y' && !index(vowels, lowc(*(bspot - 1)))) {
        /* like "y" case in makeplural */
        Strcasecpy(bspot, "ies");
    } else {
        Strcasecpy(bspot + 1, "s");
    }

    return buf;
}

struct sing_plur {
    const char *sing, *plur;
};

/* word pairs that don't fit into formula-based transformations;
   also some suffices which have very few--often one--matches or
   which aren't systematically reversible (knives, staves) */
static struct sing_plur one_off[] = {
    { "child",
      "children" },      /* (for wise guys who give their food funny names) */
    { "cubus", "cubi" }, /* in-/suc-cubus */
    { "culus", "culi" }, /* homunculus */
    { "djinni", "djinn" },
    { "erinys", "erinyes" },
    { "foot", "feet" },
    { "fungus", "fungi" },
    { "goose", "geese" },
    { "knife", "knives" },
    { "labrum", "labra" }, /* candelabrum */
    { "louse", "lice" },
    { "mouse", "mice" },
    { "mumak", "mumakil" },
    { "nemesis", "nemeses" },
    { "ovum", "ova" },
    { "ox", "oxen" },
    { "passerby", "passersby" },
    { "rtex", "rtices" }, /* vortex */
    { "serum", "sera" },
    { "staff", "staves" },
    { "tooth", "teeth" },
    { 0, 0 }
};

static const char *const as_is[] = {
    /* makesingular() leaves these plural due to how they're used */
    "boots",   "shoes",     "gloves",    "lenses",   "scales",
    "eyes",    "gauntlets", "iron bars", "goggles",  "bracers",
    /* both singular and plural are spelled the same */
    "bison",   "deer",      "elk",       "fish",      "fowl",
    "tuna",    "yaki",      "-hai",      "krill",     "manes",
    "moose",   "ninja",     "sheep",     "ronin",     "roshi",
    "shito",   "tengu",     "ki-rin",    "Nazgul",    "gunyoki",
    "piranha", "samurai",   "shuriken",  "eldritch ki-rin",
    "drow",    "undead",    "mistletoe", 0,
    /* Note:  "fish" and "piranha" are collective plurals, suitable
       for "wiped out all <foo>".  For "3 <foo>", they should be
       "fishes" and "piranhas" instead.  We settle for collective
       variant instead of attempting to support both. */
};

/* singularize/pluralize decisions common to both makesingular & makeplural */
STATIC_OVL boolean
singplur_lookup(basestr, endstring, to_plural, alt_as_is)
char *basestr, *endstring;    /* base string, pointer to eos(string) */
boolean to_plural;            /* true => makeplural, false => makesingular */
const char *const *alt_as_is; /* another set like as_is[] */
{
    const struct sing_plur *sp;
    const char *same, *other, *const *as;
    int al;
    int baselen = strlen(basestr);

    for (as = as_is; *as; ++as) {
        al = (int) strlen(*as);
        if (!BSTRCMPI(basestr, endstring - al, *as))
            return TRUE;
    }
    if (alt_as_is) {
        for (as = alt_as_is; *as; ++as) {
            al = (int) strlen(*as);
            if (!BSTRCMPI(basestr, endstring - al, *as))
                return TRUE;
        }
    }

   /* Leave "craft" as a suffix as-is (aircraft, hovercraft);
      "craft" itself is (arguably) not included in our likely context */
   if ((baselen > 5) && (!BSTRCMPI(basestr, endstring - 5, "craft")))
       return TRUE;
   /* avoid false hit on one_off[].plur == "lice" or .sing == "goose";
      if more of these turn up, one_off[] entries will need to flagged
      as to which are whole words and which are matchable as suffices
      then matching in the loop below will end up becoming more complex */
    if (!strcmpi(basestr, "slice")
        || !strcmpi(basestr, "mongoose")) {
        if (to_plural)
            Strcasecpy(endstring, "s");
        return TRUE;
    }
    /* skip "ox" -> "oxen" entry when pluralizing "<something>ox"
       unless it is muskox */
    if (to_plural && baselen > 2 && !strcmpi(endstring - 2, "ox")
        && !(baselen > 5 && !strcmpi(endstring - 6, "muskox"))) {
        /* "fox" -> "foxes" */
        Strcasecpy(endstring, "es");
        return TRUE;
    }
    if (to_plural) {
        if (baselen > 2 && !strcmpi(endstring - 3, "man")
            && badman(basestr, to_plural)) {
            Strcasecpy(endstring, "s");
            return TRUE;
        }
    } else {
        if (baselen > 2 && !strcmpi(endstring - 3, "men")
            && badman(basestr, to_plural))
            return TRUE;
    }
    for (sp = one_off; sp->sing; sp++) {
        /* check whether endstring already matches */
        same = to_plural ? sp->plur : sp->sing;
        al = (int) strlen(same);
        if (!BSTRCMPI(basestr, endstring - al, same))
            return TRUE; /* use as-is */
        /* check whether it matches the inverse; if so, transform it */
        other = to_plural ? sp->sing : sp->plur;
        al = (int) strlen(other);
        if (!BSTRCMPI(basestr, endstring - al, other)) {
            Strcasecpy(endstring - al, same);
            return TRUE; /* one_off[] transformation */
        }
    }
    return FALSE;
}

/* searches for common compounds, ex. lump of royal jelly */
STATIC_OVL char *
singplur_compound(str)
char *str;
{
    /* if new entries are added, be sure to keep compound_start[] in sync */
    static const char *const compounds[] =
        {
          " of ",     " labeled ", " called ",
          " named ",  " above", /* lurkers above */
          " versus ", " from ",    " in ",
          " on ",     " a la ",    " with", /* " with "? */
          " de ",     " d'",       " du ",
          "-in-",     "-at-",      0
        }, /* list of first characters for all compounds[] entries */
        compound_start[] = " -";

    const char *const *cmpd;
    char *p;

    for (p = str; *p; ++p) {
        /* substring starting at p can only match if *p is found
           within compound_start[] */
        if (!index(compound_start, *p))
            continue;

        /* check current substring against all words in the compound[] list */
        for (cmpd = compounds; *cmpd; ++cmpd)
            if (!strncmpi(p, *cmpd, (int) strlen(*cmpd)))
                return p;
    }
    /* wasn't recognized as a compound phrase */
    return 0;
}

/* Plural routine; once upon a time it may have been chiefly used for
 * user-defined fruits, but it is now used extensively throughout the
 * program.
 *
 * For fruit, we have to try to account for everything reasonable the
 * player has; something unreasonable can still break the code.
 * However, it's still a lot more accurate than "just add an 's' at the
 * end", which Rogue uses...
 *
 * Also used for plural monster names ("Wiped out all homunculi." or the
 * vanquished monsters list) and body parts.  A lot of unique monsters have
 * names which get mangled by makeplural and/or makesingular.  They're not
 * genocidable, and vanquished-mon handling does its own special casing
 * (for uniques who've been revived and re-killed), so we don't bother
 * trying to get those right here.
 *
 * Also misused by muse.c to convert 1st person present verbs to 2nd person.
 * 3.6.0: made case-insensitive.
 */
char *
makeplural(oldstr)
const char *oldstr;
{
    register char *spot;
    char lo_c, *str = nextobuf();
    const char *excess = (char *) 0;
    int len;

    if (oldstr)
        while (*oldstr == ' ')
            oldstr++;
    if (!oldstr || !*oldstr) {
        impossible("plural of null?");
        Strcpy(str, "s");
        return str;
    }
    Strcpy(str, oldstr);

    /*
     * Skip changing "pair of" to "pairs of".  According to Webster, usual
     * English usage is use pairs for humans, e.g. 3 pairs of dancers,
     * and pair for objects and non-humans, e.g. 3 pair of boots.  We don't
     * refer to pairs of humans in this game so just skip to the bottom.
     */
    if (!strncmpi(str, "pair of ", 8))
        goto bottom;

    /* look for "foo of bar" so that we can focus on "foo" */
    if ((spot = singplur_compound(str)) != 0) {
        excess = oldstr + (int) (spot - str);
        *spot = '\0';
    } else
        spot = eos(str);

    spot--;
    while (spot > str && *spot == ' ')
        spot--; /* Strip blanks from end */
    *(spot + 1) = '\0';
    /* Now spot is the last character of the string */

    len = strlen(str);

    /* Single letters */
    if (len == 1 || !letter(*spot)) {
        Strcpy(spot + 1, "'s");
        goto bottom;
    }

    /* dispense with some words which don't need pluralization */
    {
        static const char *const already_plural[] = {
            "ae",  /* algae, larvae, &c */
            "matzot", 0,
        };

        /* spot+1: synch up with makesingular's usage */
        if (singplur_lookup(str, spot + 1, TRUE, already_plural))
            goto bottom;

        /* more of same, but not suitable for blanket loop checking */
        if ((len == 2 && !strcmpi(str, "ya"))
            || (len >= 3 && !strcmpi(spot - 2, " ya")))
            goto bottom;
    }

    /* man/men ("Wiped out all cavemen.") */
    if (len >= 3 && !strcmpi(spot - 2, "man")
        /* exclude shamans and humans etc */
        && !badman(str, TRUE)) {
        Strcasecpy(spot - 1, "en");
        goto bottom;
    }
    if (lowc(*spot) == 'f') { /* (staff handled via one_off[]) */
        lo_c = lowc(*(spot - 1));
        if (len >= 3 && !strcmpi(spot - 2, "erf")) {
            /* avoid "nerf" -> "nerves", "serf" -> "serves" */
            ; /* fall through to default (append 's') */
        } else if (index("lr", lo_c) || index(vowels, lo_c)) {
            /* [aeioulr]f to [aeioulr]ves */
            Strcasecpy(spot, "ves");
            goto bottom;
        }
    }
    /* ium/ia (mycelia, baluchitheria) */
    if (len >= 3 && !strcmpi(spot - 2, "ium")) {
        Strcasecpy(spot - 2, "ia");
        goto bottom;
    }
    /* algae, larvae, hyphae (another fungus part) */
    if ((len >= 4 && !strcmpi(spot - 3, "alga"))
        || (len >= 5
            && (!strcmpi(spot - 4, "hypha") || !strcmpi(spot - 4, "larva")))
        || (len >= 6 && !strcmpi(spot - 5, "amoeba"))
        || (len >= 8 && (!strcmpi(spot - 7, "vertebra")))) {
        /* a to ae */
        Strcasecpy(spot + 1, "e");
        goto bottom;
    }
    /* fungus/fungi, homunculus/homunculi, but buses, lotuses, wumpuses */
    if (len > 3 && !strcmpi(spot - 1, "us")
        && !((len >= 5 && !strcmpi(spot - 4, "lotus"))
             || (len >= 6 && !strcmpi(spot - 5, "wumpus")))) {
        Strcasecpy(spot - 1, "i");
        goto bottom;
    }
    /* sis/ses (nemesis) */
    if (len >= 3 && !strcmpi(spot - 2, "sis")) {
        Strcasecpy(spot - 1, "es");
        goto bottom;
    }
    /* matzoh/matzot, possible food name */
    if (len >= 6
        && (!strcmpi(spot - 5, "matzoh") || !strcmpi(spot - 5, "matzah"))) {
        Strcasecpy(spot - 1, "ot"); /* oh/ah -> ot */
        goto bottom;
    }
    if (len >= 5
        && (!strcmpi(spot - 4, "matzo") || !strcmpi(spot - 4, "matza"))) {
        Strcasecpy(spot, "ot"); /* o/a -> ot */
        goto bottom;
    }

    /* note: -eau/-eaux (gateau, bordeau...) */
    /* note: ox/oxen, VAX/VAXen, goose/geese */

    lo_c = lowc(*spot);

    /* codex/spadix/neocortex and the like */
    if (len >= 5
        && (!strcmpi(spot - 2, "dex")
            ||!strcmpi(spot - 2, "dix")
            ||!strcmpi(spot - 2, "tex"))
           /* indices would have been ok too, but stick with indexes */
        && (strcmpi(spot - 4,"index") != 0)) {
        Strcasecpy(spot - 1, "ices"); /* ex|ix -> ices */
        goto bottom;
    }
    /* Ends in z, x, s, ch, sh; add an "es" */
    if (index("zxs", lo_c)
        || (len >= 2 && lo_c == 'h' && index("cs", lowc(*(spot - 1)))
            /* 21st century k-sound */
            && !(len >= 4 &&
                 ((lowc(*(spot - 2)) == 'e'
                    && index("mt", lowc(*(spot - 3)))) ||
                  (lowc(*(spot - 2)) == 'o'
                    && index("lp", lowc(*(spot - 3)))))))
        /* Kludge to get "tomatoes" and "potatoes" right */
        || (len >= 4 && !strcmpi(spot - 2, "ato"))
        || (len >= 5 && !strcmpi(spot - 4, "dingo"))) {
        Strcasecpy(spot + 1, "es"); /* append es */
        goto bottom;
    }
    /* Ends in y preceded by consonant (note: also "qu") change to "ies" */
    if (lo_c == 'y' && !index(vowels, lowc(*(spot - 1)))) {
        Strcasecpy(spot, "ies"); /* y -> ies */
        goto bottom;
    }
    /* Default: append an 's' */
    Strcasecpy(spot + 1, "s");

 bottom:
    if (excess)
        Strcat(str, excess);
    return str;
}

/*
 * Singularize a string the user typed in; this helps reduce the complexity
 * of readobjnam, and is also used in pager.c to singularize the string
 * for which help is sought.
 *
 * "Manes" is ambiguous: monster type (keep s), or horse body part (drop s)?
 * Its inclusion in as_is[]/special_subj[] makes it get treated as the former.
 *
 * A lot of unique monsters have names ending in s; plural, or singular
 * from plural, doesn't make much sense for them so we don't bother trying.
 * 3.6.0: made case-insensitive.
 */
char *
makesingular(oldstr)
const char *oldstr;
{
    register char *p, *bp;
    const char *excess = 0;
    char *str = nextobuf();

    if (oldstr)
        while (*oldstr == ' ')
            oldstr++;
    if (!oldstr || !*oldstr) {
        impossible("singular of null?");
        str[0] = '\0';
        return str;
    }

    bp = strcpy(str, oldstr);

    /* check for "foo of bar" so that we can focus on "foo" */
    if ((p = singplur_compound(bp)) != 0) {
        excess = oldstr + (int) (p - bp);
        *p = '\0';
    } else
        p = eos(bp);

    /* dispense with some words which don't need singularization */
    if (singplur_lookup(bp, p, FALSE, special_subjs))
        goto bottom;

    /* remove -s or -es (boxes) or -ies (rubies) */
    if (p >= bp + 1 && lowc(p[-1]) == 's') {
        if (p >= bp + 2 && lowc(p[-2]) == 'e') {
            if (p >= bp + 3 && lowc(p[-3]) == 'i') { /* "ies" */
                if (!BSTRCMPI(bp, p - 7, "cookies")
                    || (!BSTRCMPI(bp, p - 4, "pies")
                        /* avoid false match for "harpies" */
                        && (p - 4 == bp || p[-5] == ' '))
                    /* alternate djinni/djinn spelling; not really needed */
                    || (!BSTRCMPI(bp, p - 6, "genies")
                        /* avoid false match for "progenies" */
                        && (p - 6 == bp || p[-7] == ' '))
                    || !BSTRCMPI(bp, p - 5, "mbies") /* zombie */
                    || !BSTRCMPI(bp, p - 5, "yries")) /* valkyrie */
                    goto mins;
                Strcasecpy(p - 3, "y"); /* ies -> y */
                goto bottom;
            }
            /* wolves, but f to ves isn't fully reversible */
            if (p - 4 >= bp && (index("lr", lowc(*(p - 4)))
                                || index(vowels, lowc(*(p - 4))))
                && !BSTRCMPI(bp, p - 3, "ves")) {
                if (!BSTRCMPI(bp, p - 6, "cloves")
                    || !BSTRCMPI(bp, p - 6, "nerves"))
                    goto mins;
                Strcasecpy(p - 3, "f"); /* ves -> f */
                goto bottom;
            }
            /* note: nurses, axes but boxes, wumpuses */
            if (!BSTRCMPI(bp, p - 4, "eses")
                || !BSTRCMPI(bp, p - 4, "oxes") /* boxes, foxes */
                || !BSTRCMPI(bp, p - 4, "nxes") /* lynxes */
                || !BSTRCMPI(bp, p - 4, "ches")
                || !BSTRCMPI(bp, p - 4, "uses") /* lotuses */
                || !BSTRCMPI(bp, p - 4, "sses") /* priestesses */
                || !BSTRCMPI(bp, p - 5, "atoes") /* tomatoes */
                || !BSTRCMPI(bp, p - 7, "dingoes")
                || !BSTRCMPI(bp, p - 7, "Aleaxes")) {
                *(p - 2) = '\0'; /* drop es */
                goto bottom;
            } /* else fall through to mins */

            /* ends in 's' but not 'es' */
        } else if (!BSTRCMPI(bp, p - 2, "us")) { /* lotus, fungus... */
            if (BSTRCMPI(bp, p - 6, "tengus") /* but not these... */
                && BSTRCMPI(bp, p - 7, "hezrous"))
                goto bottom;
        } else if (!BSTRCMPI(bp, p - 2, "ss")
                   || !BSTRCMPI(bp, p - 5, " lens")
                   || (p - 4 == bp && !strcmpi(p - 4, "lens"))) {
            goto bottom;
        }
 mins:
        *(p - 1) = '\0'; /* drop s */

    } else { /* input doesn't end in 's' */

        if (!BSTRCMPI(bp, p - 3, "men")
            && !badman(bp, FALSE)) {
            Strcasecpy(p - 2, "an");
            goto bottom;
        }
        /* matzot -> matzo, algae -> alga */
        if (!BSTRCMPI(bp, p - 6, "matzot") || !BSTRCMPI(bp, p - 2, "ae")) {
            *(p - 1) = '\0'; /* drop t/e */
            goto bottom;
        }
        /* balactheria -> balactherium */
        if (p - 4 >= bp && !strcmpi(p - 2, "ia")
            && index("lr", lowc(*(p - 3))) && lowc(*(p - 4)) == 'e') {
            Strcasecpy(p - 1, "um"); /* a -> um */
        }

        /* here we cannot find the plural suffix */
    }

 bottom:
    /* if we stripped off a suffix (" of bar" from "foo of bar"),
       put it back now [strcat() isn't actually 100% safe here...] */
    if (excess)
        Strcat(bp, excess);

    return bp;
}

STATIC_OVL boolean
badman(basestr, to_plural)
const char *basestr;
boolean to_plural;            /* true => makeplural, false => makesingular */
{
    /* these are all the prefixes for *man that don't have a *men plural */
    static const char *no_men[] = {
        "albu", "antihu", "anti", "ata", "auto", "bildungsro", "cai", "cay",
        "ceru", "corner", "decu", "des", "dura", "fir", "hanu", "het",
        "infrahu", "inhu", "nonhu", "otto", "out", "prehu", "protohu",
        "subhu", "superhu", "talis", "unhu", "sha",
        "hu", "un", "le", "re", "so", "to", "at", "a",
    };
    /* these are all the prefixes for *men that don't have a *man singular */
    static const char *no_man[] = {
        "abdo", "acu", "agno", "ceru", "cogno", "cycla", "fleh", "grava",
        "hegu", "preno", "sonar", "speci", "dai", "exa", "fla", "sta", "teg",
        "tegu", "vela", "da", "hy", "lu", "no", "nu", "ra", "ru", "se", "vi",
        "ya", "o", "a",
    };
    int i, al;
    const char *endstr, *spot;

    if (!basestr || strlen(basestr) < 4)
        return FALSE;

    endstr = eos((char *) basestr);

    if (to_plural) {
        for (i = 0; i < SIZE(no_men); i++) {
            al = (int) strlen(no_men[i]);
            spot = endstr - (al + 3);
            if (!BSTRNCMPI(basestr, spot, no_men[i], al)
                && (spot == basestr || *(spot - 1) == ' '))
                return TRUE;
        }
    } else {
        for (i = 0; i < SIZE(no_man); i++) {
            al = (int) strlen(no_man[i]);
            spot = endstr - (al + 3);
            if (!BSTRNCMPI(basestr, spot, no_man[i], al)
                && (spot == basestr || *(spot - 1) == ' '))
                return TRUE;
        }
    }
    return FALSE;
}

/* compare user string against object name string using fuzzy matching */
STATIC_OVL boolean
wishymatch(u_str, o_str, retry_inverted)
const char *u_str;      /* from user, so might be variant spelling */
const char *o_str;      /* from objects[], so is in canonical form */
boolean retry_inverted; /* optional extra "of" handling */
{
    static NEARDATA const char detect_SP[] = "detect ",
                               SP_detection[] = " detection";
    char *p, buf[BUFSZ];

    /* ignore spaces & hyphens and upper/lower case when comparing */
    if (fuzzymatch(u_str, o_str, " -", TRUE))
        return TRUE;

    if (retry_inverted) {
        const char *u_of, *o_of;

        /* when just one of the strings is in the form "foo of bar",
           convert it into "bar foo" and perform another comparison */
        u_of = strstri(u_str, " of ");
        o_of = strstri(o_str, " of ");
        if (u_of && !o_of) {
            Strcpy(buf, u_of + 4);
            p = eos(strcat(buf, " "));
            while (u_str < u_of)
                *p++ = *u_str++;
            *p = '\0';
            return fuzzymatch(buf, o_str, " -", TRUE);
        } else if (o_of && !u_of) {
            Strcpy(buf, o_of + 4);
            p = eos(strcat(buf, " "));
            while (o_str < o_of)
                *p++ = *o_str++;
            *p = '\0';
            return fuzzymatch(u_str, buf, " -", TRUE);
        }
    }

    /* [note: if something like "elven speed boots" ever gets added, these
       special cases should be changed to call wishymatch() recursively in
       order to get the "of" inversion handling] */
    if (!strncmp(o_str, "dwarvish ", 9)) {
        if (!strncmpi(u_str, "dwarven ", 8))
            return fuzzymatch(u_str + 8, o_str + 9, " -", TRUE);
    } else if (!strncmp(o_str, "elven ", 6)) {
        if (!strncmpi(u_str, "elvish ", 7))
            return fuzzymatch(u_str + 7, o_str + 6, " -", TRUE);
        else if (!strncmpi(u_str, "elfin ", 6))
            return fuzzymatch(u_str + 6, o_str + 6, " -", TRUE);
    } else if (!strncmp(o_str, detect_SP, sizeof detect_SP - 1)) {
        /* check for "detect <foo>" vs "<foo> detection" */
        if ((p = strstri(u_str, SP_detection)) != 0
            && !*(p + sizeof SP_detection - 1)) {
            /* convert "<foo> detection" into "detect <foo>" */
            *p = '\0';
            Strcat(strcpy(buf, detect_SP), u_str);
            /* "detect monster" -> "detect monsters" */
            if (!strcmpi(u_str, "monster"))
                Strcat(buf, "s");
            *p = ' ';
            return fuzzymatch(buf, o_str, " -", TRUE);
        }
    } else if (strstri(o_str, SP_detection)) {
        /* and the inverse, "<foo> detection" vs "detect <foo>" */
        if (!strncmpi(u_str, detect_SP, sizeof detect_SP - 1)) {
            /* convert "detect <foo>s" into "<foo> detection" */
            p = makesingular(u_str + sizeof detect_SP - 1);
            Strcat(strcpy(buf, p), SP_detection);
            /* caller may be looping through objects[], so avoid
               churning through all the obufs */
            releaseobuf(p);
            return fuzzymatch(buf, o_str, " -", TRUE);
        }
    } else if (strstri(o_str, "ability")) {
        /* when presented with "foo of bar", makesingular() used to
           singularize both foo & bar, but now only does so for foo */
        /* catch "{potion(s),ring} of {gain,restore,sustain} abilities" */
        if ((p = strstri(u_str, "abilities")) != 0
            && !*(p + sizeof "abilities" - 1)) {
            (void) strncpy(buf, u_str, (unsigned) (p - u_str));
            Strcpy(buf + (p - u_str), "ability");
            return fuzzymatch(buf, o_str, " -", TRUE);
        }
    } else if (!strcmp(o_str, "aluminum")) {
        /* this special case doesn't really fit anywhere else... */
        /* (note that " wand" will have been stripped off by now) */
        if (!strcmpi(u_str, "aluminium"))
            return fuzzymatch(u_str + 9, o_str + 8, " -", TRUE);
    }

    return FALSE;
}

struct o_range {
    const char *name, oclass;
    int f_o_range, l_o_range;
};

/* wishable subranges of objects */
STATIC_OVL NEARDATA const struct o_range o_ranges[] = {
    { "bag", TOOL_CLASS, SACK, BAG_OF_TRICKS },
    { "lamp", TOOL_CLASS, OIL_LAMP, MAGIC_LAMP },
    { "candle", TOOL_CLASS, TALLOW_CANDLE, WAX_CANDLE },
    { "horn", TOOL_CLASS, TOOLED_HORN, HORN_OF_PLENTY },
    { "shield", ARMOR_CLASS, SMALL_SHIELD, SHIELD_OF_MOBILITY },
    { "bracers", ARMOR_CLASS, BRACERS, RUNED_BRACERS },
    { "hat", ARMOR_CLASS, DUNCE_CAP, FEDORA },
    { "helm", ARMOR_CLASS, DUNCE_CAP, HELM_OF_TELEPATHY },
    { "gloves", ARMOR_CLASS, GLOVES, MUMMIFIED_HAND },
    { "gauntlets", ARMOR_CLASS, GAUNTLETS, GAUNTLETS_OF_POWER },
    { "boots", ARMOR_CLASS, LOW_BOOTS, FUMBLE_BOOTS },
    { "shoes", ARMOR_CLASS, LOW_BOOTS, ORCISH_BOOTS },
    { "cloak", ARMOR_CLASS, MUMMY_WRAPPING, CLOAK_OF_DISPLACEMENT },
    { "shirt", ARMOR_CLASS, HAWAIIAN_SHIRT, T_SHIRT },
    { "dragon scales", ARMOR_CLASS, FIRST_DRAGON_SCALES,
      LAST_DRAGON_SCALES },
    { "sword", WEAPON_CLASS, SHORT_SWORD, KATANA },
    { "venom", VENOM_CLASS, BLINDING_VENOM, BARBED_NEEDLE },
    { "gray stone", GEM_CLASS, LUCKSTONE, FLINT },
    { "grey stone", GEM_CLASS, LUCKSTONE, FLINT },
};

/* alternate spellings; if the difference is only the presence or
   absence of spaces and/or hyphens (such as "pickaxe" vs "pick axe"
   vs "pick-axe") then there is no need for inclusion in this list;
   likewise for ``"of" inversions'' ("boots of speed" vs "speed boots") */
static const struct alt_spellings {
    const char *sp;
    int ob;
} spellings[] = {
    { "pickax", PICK_AXE },
    { "whip", BULLWHIP },
    { "sabre", SABER },
    { "smooth shield", SHIELD_OF_REFLECTION },
    { "grey dragon scales", GRAY_DRAGON_SCALES },
    { "iron ball", HEAVY_IRON_BALL },
    { "lantern", LANTERN },
    { "mattock", DWARVISH_MATTOCK },
    { "bearded axe", DWARVISH_BEARDED_AXE },
    { "amulet of poison resistance", AMULET_VERSUS_POISON },
    { "amulet of protection", AMULET_OF_GUARDING },
    { "potion of sleep", POT_SLEEPING },
    { "stone", ROCK },
    { "camera", EXPENSIVE_CAMERA },
    { "tee shirt", T_SHIRT },
    { "can", TIN },
    { "can opener", TIN_OPENER },
    { "kelp", KELP_FROND },
    { "eucalyptus", EUCALYPTUS_LEAF },
    { "royal jelly", LUMP_OF_ROYAL_JELLY },
    { "lembas", LEMBAS_WAFER },
    { "cookie", FORTUNE_COOKIE },
    { "pie", CREAM_PIE },
    { "marker", MAGIC_MARKER },
    { "hook", GRAPPLING_HOOK },
    { "grappling iron", GRAPPLING_HOOK },
    { "grapnel", GRAPPLING_HOOK },
    { "grapple", GRAPPLING_HOOK },
    { "protection from shape shifters", RIN_PROTECTION_FROM_SHAPE_CHAN },
    /* if we ever add other sizes, move this to o_ranges[] with "bag" */
    { "box", LARGE_BOX },
    /* normally we wouldn't have to worry about unnecessary <space>, but
       " stone" will get stripped off, preventing a wishymatch; that actually
       lets "flint stone" be a match, so we also accept bogus "flintstone" */
    { "luck stone", LUCKSTONE },
    { "load stone", LOADSTONE },
    { "touch stone", TOUCHSTONE },
    { "flintstone", FLINT },
    { "shiny stone", SLING_BULLET },
    /* Drow objects */
    { "drow arrow", DARK_ELVEN_ARROW },
    { "drow crossbow bolt", DARK_ELVEN_CROSSBOW_BOLT },
    { "drow spear", DARK_ELVEN_SPEAR },
    { "drow dagger", DARK_ELVEN_DAGGER },
    { "drow short sword", DARK_ELVEN_SHORT_SWORD },
    { "drow broadsword", DARK_ELVEN_BROADSWORD },
    { "drow long sword", DARK_ELVEN_LONG_SWORD },
    { "drow mace", DARK_ELVEN_MACE },
    { "drow heavy mace", DARK_ELVEN_HEAVY_MACE },
    { "drow bow", DARK_ELVEN_BOW },
    { "drow crossbow", DARK_ELVEN_HAND_CROSSBOW },
    { "dark elven crossbow", DARK_ELVEN_HAND_CROSSBOW },
    { "drow hand crossbow", DARK_ELVEN_HAND_CROSSBOW },
    { "drow helm", DARK_ELVEN_HELM },
    { "drow armor", DARK_ELVEN_CHAIN_MAIL },
    { "dark elven armor", DARK_ELVEN_CHAIN_MAIL },
    { "drow chain mail", DARK_ELVEN_CHAIN_MAIL },
    { "drow tunic", DARK_ELVEN_TUNIC },
    { "drow cloak", DARK_ELVEN_CLOAK },
    { "drow bracers", DARK_ELVEN_BRACERS },
    { "dark elven bracers", DARK_ELVEN_BRACERS },
    { "drow shield", DARK_ELVEN_BRACERS },
    { "dark elven shield", DARK_ELVEN_BRACERS },
    { "drow gloves", DARK_ELVEN_GLOVES },
    { "drow boots", DARK_ELVEN_BOOTS },
    /* armor */
    { "gloves", GLOVES },
    { "bracers", BRACERS },
    { "runed bracers", RUNED_BRACERS },
    { (const char *) 0, 0 },
};

STATIC_OVL short
rnd_otyp_by_wpnskill(skill)
schar skill;
{
    int i, n = 0;
    short otyp = STRANGE_OBJECT;

    for (i = bases[WEAPON_CLASS];
         i < NUM_OBJECTS && objects[i].oc_class == WEAPON_CLASS; i++)
        if (objects[i].oc_skill == skill) {
            n++;
            otyp = i;
        }
    if (n > 0) {
        n = rn2(n);
        for (i = bases[WEAPON_CLASS];
             i < NUM_OBJECTS && objects[i].oc_class == WEAPON_CLASS; i++)
            if (objects[i].oc_skill == skill)
                if (--n < 0)
                    return i;
    }
    return otyp;
}

STATIC_OVL short
rnd_otyp_by_namedesc(name, oclass, xtra_prob)
const char *name;
char oclass;
int xtra_prob; /* to force 0% random generation items to also be considered */
{
    int i, n = 0;
    short validobjs[NUM_OBJECTS];
    register const char *zn;
    int prob, maxprob = 0;

    if (!name || !*name)
        return STRANGE_OBJECT;

    memset((genericptr_t) validobjs, 0, sizeof validobjs);

    /* FIXME:
     * When this spans classes (the !oclass case), the item
     * probabilities are not very useful because they don't take
     * the class generation probability into account.  [If 10%
     * of spellbooks were blank and 1% of scrolls were blank,
     * "blank" would have 10/11 chance to yield a book even though
     * scrolls are supposed to be much more common than books.]
     */
    for (i = oclass ? bases[(int) oclass] : STRANGE_OBJECT + 1;
         i < NUM_OBJECTS && (!oclass || objects[i].oc_class == oclass);
         ++i) {
        /* don't match extra descriptions (w/o real name) */
        if ((zn = OBJ_NAME(objects[i])) == 0)
            continue;
        if (wishymatch(name, zn, TRUE)
            || ((zn = OBJ_DESCR(objects[i])) != 0
                && wishymatch(name, zn, FALSE))
            || ((zn = objects[i].oc_uname) != 0
                && wishymatch(name, zn, FALSE))) {
            validobjs[n++] = (short) i;
            maxprob += (objects[i].oc_prob + xtra_prob);
        }
    }

    if (n > 0 && maxprob) {
        prob = rn2(maxprob);
        for (i = 0; i < n - 1; i++)
            if ((prob -= (objects[validobjs[i]].oc_prob + xtra_prob)) < 0)
                break;
        return validobjs[i];
    }
    return STRANGE_OBJECT;
}

int
shiny_obj(oclass)
char oclass;
{
    return (int) rnd_otyp_by_namedesc("shiny", oclass, 0);
}

/*
 * Given a user-supplied string, try to match it to an object type.
 * Very similar to rnd_otyp_by_namedesc, except it doesn't fall back on picking
 * a random object if it can't find an appropriate one. Intended to be the
 * object counterpart to name_to_mon.
 * Only works on exact object names, since allowing it to work on randomized
 * descriptions or user-called names would leak information to object lookup.
 */
short
name_to_otyp(in_str)
const char * in_str;
{
    struct Jitem *ji;
    short otyp;
    int i;
    char oclass = 0;
    const struct alt_spellings *as;

    /* Search for class names: XXXXX potion, scroll of XXXXX.  Avoid */
    /* false hits on, e.g., rings for "ring mail".
     * This is lifted from readobjnam, and should probably be refactored into
     * its own function but the existing logic in there is too tied up with
     * readobjnam variables at the moment. */
    if (strncmpi(in_str, "enchant ", 8)
        && strncmpi(in_str, "destroy ", 8)
        && strncmpi(in_str, "detect food", 11)
        && strncmpi(in_str, "food detection", 14)
        && strncmpi(in_str, "ring mail", 9)
        && strncmpi(in_str, "studded armor", 13)
        && strncmpi(in_str, "armor", 5)
        && strncmpi(in_str, "tooled horn", 11)
        && strncmpi(in_str, "ring of p'", 10)
        && strncmpi(in_str, "wand of orcus", 13)
        && strncmpi(in_str, "food ration", 11)
        && strncmpi(in_str, "meat ring", 9)
        && strncmpi(in_str, "glamdring", 9)
        && strncmpi(in_str, "one ring", 8)) {
        for (i = 0; i < (int) (sizeof wrpsym); i++) {
            int j = strlen(wrp[i]);
            if (!strncmpi(in_str, wrp[i], j)) {
                oclass = wrpsym[i];
                if (oclass != AMULET_CLASS) {
                    /* amulets don't consistently use "amulet of" */
                    in_str += j;
                    if (!strncmpi(in_str, " of ", 4))
                        in_str += 4;
                }
                break;
            }
        }
    }
    /* if the player asked only for "ring", etc, that's not going to resolve to
     * anything in this function, so safe to say the string matches no otyp. */
    if (!(*in_str))
        return STRANGE_OBJECT;

    for (otyp = STRANGE_OBJECT + 1; otyp < NUM_OBJECTS; ++otyp) {
        if (!OBJ_NAME(objects[otyp])) {
            /* obj is nonexistent in this game */
            continue;
        } else if (oclass && objects[otyp].oc_class != oclass) {
            /* name might match, but the class is wrong, e.g. "scroll of light"
             * becomes "light" which matches "wand of light" */
            continue;
        }
        if (wishymatch(in_str, OBJ_NAME(objects[otyp]), TRUE)) {
            return otyp;
        }
    }
    /* try alternate spellings */
    for (as = spellings; as->sp != 0; as++) {
        if (!strcmpi(in_str, as->sp)) {
            return as->ob;
        }
    }
    /* try Japanese names */
    for (ji = Japanese_items; ji->item != 0; ji++) {
        if (!strcmpi(in_str, ji->name)) {
            return ji->item;
        }
    }
    /* try fruits */
    if (fruit_from_name(in_str, FALSE, NULL))
        return SLIME_MOLD;
    return STRANGE_OBJECT;
}

/* Return true if the input string clearly represents an object, not a monster.
 */
boolean
object_not_monster(str)
const char *str;
{
    const char *non_monster_strs[] = {
        "samurai sword", /* not the "samurai" monster! */
        "wizard lock",   /* not the "wizard" monster! */
        "drow poison",   /* not the "drow" monster! */
        "spider silk",   /* not "spider" monsters! */
        "death wand",    /* 'of inversion', not Rider */
        "master key",    /* not the "master" rank */
        "ninja-to",      /* not the "ninja" rank */
        "magenta",       /* not the "mage" rank */
        "vampire blood", /* not the "vampire" monster! */
    };
    int i;
    for (i = 0; i < SIZE(non_monster_strs); ++i) {
        if (!strncmpi(str, non_monster_strs[i], strlen(non_monster_strs[i]))) {
            return TRUE;
        }
    }
    return FALSE;
}

/* Return true if the input string appears to be specifying something that
   happens to start with a material name and is not actually specifying a
   material */
STATIC_OVL boolean
not_spec_material(str, material)
const char * const str;
int material;
{
    int i;
    const char *matstr = materialnm[material];
    int matlen = strlen(matstr);
    /* is this the entire string? e.g. "gold" is actually a wish for zorkmids.
       The effect of this is that you can't just wish for a material and get a
       random item, made of that material, from the set of items that can be
       made of that material, but this was never a feature anyway; it would give
       a totally random object even if the material-object combo was invalid */
    if (!strcmp(str, matstr)) {
        return TRUE;
    }
    /* does it match some object? */
    for (i = STRANGE_OBJECT + 1; i < NUM_OBJECTS; ++i) {
        /* match in the object name e.g. "gold detection", "wax candle" */
        const char *oc_name = OBJ_NAME(objects[i]),
                   *oc_descr = OBJ_DESCR(objects[i]);
        if (oc_name && !strncmpi(str, oc_name, strlen(oc_name))) {
            return TRUE;
        }
        /* match in the object description, followed by object class name e.g.
           "silver ring"
           Note: this assumes that the only types of objects that can have
           material strings as descriptions are not eligible to have object
           materials vary on them. It also does not account for the fact that,
           for instance, there being more wand descriptions than wands could
           mean that "iron wand" doesn't actually exist in this game... */
        if (oc_descr && !strcmp(matstr, oc_descr)) {
            /* this is a bit complicated... the only 3 object classes that ought
               to behave this way are rings, wands and spellbooks, all of which
               have the singular object class string in def_oc_syms[].explain.
               If e.g. "silver armor" was a real randomized armor description this
               would break because the explain is "suit or piece of armor" */
            const char *aftermat = str + matlen + 1; /* advance past material */
            oc_descr = def_oc_syms[(int) objects[i].oc_class].explain;
            if (!strncmpi(aftermat, oc_descr, strlen(oc_descr))) {
                return TRUE;
            }
        }
    }
    /* does it match some artifact? e.g. "platinum yendorian express card" */
    short otyp;
    if (artifact_name(str, &otyp)) {
        return TRUE;
    }
    /* does it match some terrain or a trap? e.g. "iron bars" */
    for (i = 0; i < MAXPCHARS; ++i) {
        const char *terr_name = defsyms[i].explanation;
        if (terr_name && *terr_name
            && !strncmpi(str, terr_name, strlen(terr_name))) {
            return TRUE;
        }
    }
    return FALSE;
}

/*
 * Return something wished for.  Specifying a null pointer for
 * the user request string results in a random object.  Otherwise,
 * if asking explicitly for "nothing" (or "nil") return no_wish;
 * if not an object return &zeroobj; if an error (no matching object),
 * return null.
 */
struct obj *
readobjnam(bp, no_wish)
register char *bp;
struct obj *no_wish;
{
    register char *p;
    register int i;
    register struct obj *otmp;
    int cnt, spe, spesgn, typ, very, rechrg;
    int blessed, uncursed, iscursed, ispoisoned, istainted, isgreased;
    int magical, isforged0, isforged1, isforged2, isdrained;
    int eroded, eroded2, erodeproof, locked, unlocked, broken;
    int halfeaten, halfdrained, mntmp, contents;
    int islit, unlabeled, ishistoric, isdiluted, trapped;
    int material;
    int tmp, tinv, tvariety;
    int wetness, gsize = 0;
    boolean zombo;
    struct fruit *f;
    int ftype = context.current_fruit;
    char fruitbuf[BUFSZ], globbuf[BUFSZ];
    /* Fruits may not mess up the ability to wish for real objects (since
     * you can leave a fruit in a bones file and it will be added to
     * another person's game), so they must be checked for last, after
     * stripping all the possible prefixes and seeing if there's a real
     * name in there.  So we have to save the full original name.  However,
     * it's still possible to do things like "uncursed burnt Alaska",
     * or worse yet, "2 burned 5 course meals", so we need to loop to
     * strip off the prefixes again, this time stripping only the ones
     * possible on food.
     * We could get even more detailed so as to allow food names with
     * prefixes that _are_ possible on food, so you could wish for
     * "2 3 alarm chilis".  Currently this isn't allowed; options.c
     * automatically sticks 'candied' in front of such names.
     */
    char oclass;
    char *un, *dn, *actualn, *origbp = bp;
    const char *name = 0;

    long objprops = 0;
    long objpropcount = 0;

    cnt = spe = spesgn = typ = 0;
    very = rechrg = blessed = uncursed = iscursed = magical =
        ispoisoned = istainted = isgreased = eroded = eroded2 =
        erodeproof = halfeaten = islit = unlabeled = ishistoric =
        isdiluted = trapped = locked = unlocked = broken =
        isforged0 = isforged1 = isforged2 = isdrained = halfdrained = 0;
    tvariety = RANDOM_TIN;
    mntmp = NON_PM;
#define UNDEFINED 0
#define EMPTY 1
#define SPINACH 2
    material = 0;
    contents = UNDEFINED;
    oclass = 0;
    actualn = dn = un = 0;
    wetness = 0;
    zombo = FALSE;

    if (!bp)
        goto any;
    /* first, remove extra whitespace they may have typed */
    (void) mungspaces(bp);
    /* allow wishing for "nothing" to preserve wishless conduct...
       [now requires "wand of nothing" if that's what was really wanted] */
    if (!strcmpi(bp, "nothing") || !strcmpi(bp, "nil")
        || !strcmpi(bp, "none"))
        return no_wish;
    /* save the [nearly] unmodified choice string */
    Strcpy(fruitbuf, bp);

    for (;;) {
        register int l;

        if (!bp || !*bp)
            goto any;
        if (!strncmpi(bp, "an ", l = 3) || !strncmpi(bp, "a ", l = 2)) {
            cnt = 1;
        } else if (!strncmpi(bp, "the ", l = 4)) {
            ; /* just increment `bp' by `l' below */
        } else if (!cnt && digit(*bp) && strcmp(bp, "0")) {
            cnt = atoi(bp);
            while (digit(*bp))
                bp++;
            while (*bp == ' ')
                bp++;
            l = 0;
        } else if (*bp == '+' || *bp == '-') {
            spesgn = (*bp++ == '+') ? 1 : -1;
            spe = atoi(bp);
            while (digit(*bp))
                bp++;
            while (*bp == ' ')
                bp++;
            l = 0;
        } else if (!strncmpi(bp, "blessed ", l = 8)
                   || !strncmpi(bp, "holy ", l = 5)) {
            blessed = 1;
        } else if (!strncmpi(bp, "moist ", l = 6)
                   || !strncmpi(bp, "wet ", l = 4)) {
            if (!strncmpi(bp, "wet ", 4))
                wetness = rn2(3) + 3;
            else
                wetness = rnd(2);
        } else if (!strncmpi(bp, "cursed ", l = 7)
                   || !strncmpi(bp, "unholy ", l = 7)) {
            iscursed = 1;
        } else if (!strncmpi(bp, "uncursed ", l = 9)) {
            uncursed = 1;
        } else if (!strncmpi(bp, "magical ", l = 8)) {
            magical = 1;
        } else if (!strncmpi(bp, "rustproof ", l = 10)
                   || !strncmpi(bp, "erodeproof ", l = 11)
                   || !strncmpi(bp, "corrodeproof ", l = 13)
                   || !strncmpi(bp, "fixed ", l = 6)
                   || !strncmpi(bp, "preserved ", l = 10)
                   || !strncmpi(bp, "fireproof ", l = 10)
                   || !strncmpi(bp, "tempered ", l = 9)) {
            erodeproof = 1;
        } else if (!strncmpi(bp, "lit ", l = 4)
                   || !strncmpi(bp, "burning ", l = 8)) {
            islit = 1;
        } else if (!strncmpi(bp, "unlit ", l = 6)
                   || !strncmpi(bp, "extinguished ", l = 13)) {
            islit = 0;
            /* "unlabeled" and "blank" are synonymous */
        } else if (!strncmpi(bp, "unlabeled ", l = 10)
                   || !strncmpi(bp, "unlabelled ", l = 11)
                   || !strncmpi(bp, "blank ", l = 6)) {
            unlabeled = 1;
        } else if (!strncmpi(bp, "inferior ", l = 9)) {
            isforged0 = 1;
        } else if (!strncmpi(bp, "superior ", l = 9)) {
            isforged1 = 1;
        } else if (!strncmpi(bp, "exceptional ", l = 12)) {
            isforged2 = 1;
        } else if (!strncmpi(bp, "poisoned ", l = 9)) {
            ispoisoned = 1;
        } else if (!strncmpi(bp, "tainted ", l = 8)) {
            istainted = 1;
            /* "trapped" recognized but not honored outside wizard mode */
        } else if (!strncmpi(bp, "trapped ", l = 8)) {
            trapped = 0; /* undo any previous "untrapped" */
            if (wizard)
                trapped = 1;
        } else if (!strncmpi(bp, "untrapped ", l = 10)) {
            trapped = 2; /* not trapped */
        /* locked, unlocked, broken: box/chest lock states */
        } else if (!strncmpi(bp, "locked ", l = 7)) {
            locked = 1, unlocked = broken = 0;
        } else if (!strncmpi(bp, "unlocked ", l = 9)) {
            unlocked = 1, locked = broken = 0;
        } else if (!strncmpi(bp, "broken ", l = 7)) {
            broken = 1, locked = unlocked = 0;
        } else if (!strncmpi(bp, "greased ", l = 8)) {
            isgreased = 1;
        } else if (!strncmpi(bp, "zombifying ", l = 11)) {
            zombo = TRUE;
        } else if (!strncmpi(bp, "very ", l = 5)) {
            /* very rusted very heavy iron ball */
            very = 1;
        } else if (!strncmpi(bp, "thoroughly ", l = 11)) {
            very = 2;
        } else if (!strncmpi(bp, "rusty ", l = 6)
                   || !strncmpi(bp, "rusted ", l = 7)
                   || !strncmpi(bp, "decayed ", l = 8)
                   || !strncmpi(bp, "burnt ", l = 6)
                   || !strncmpi(bp, "burned ", l = 7)) {
            eroded = 1 + very;
            very = 0;
        } else if (!strncmpi(bp, "corroded ", l = 9)
                   || !strncmpi(bp, "rotted ", l = 7)
                   || !strncmpi(bp, "fractured ", l = 10)
                   || !strncmpi(bp, "deteriorated ", l = 13)) {
            eroded2 = 1 + very;
            very = 0;
        } else if (!strncmpi(bp, "partly drained ", l = 15)
                   || !strncmpi(bp, "partially drained ", l = 18)) {
            isdrained = 1;
            halfdrained = 1;
        } else if (!strncmpi(bp, "drained ", l = 8)) {
            isdrained = 1;
            halfdrained = 0;
        } else if (!strncmpi(bp, "partly eaten ", l = 13)
                   || !strncmpi(bp, "partially eaten ", l = 16)) {
            halfeaten = 1;
        } else if (!strncmpi(bp, "historic ", l = 9)) {
            ishistoric = 1;
        } else if (!strncmpi(bp, "diluted ", l = 8)) {
            isdiluted = 1;
        } else if (!strncmpi(bp, "empty ", l = 6)) {
            contents = EMPTY;
        } else if (!strncmpi(bp, "small ", l = 6)) { /* glob sizes */
            /* "small" might be part of monster name (mimic, if wishing
               for its corpse) rather than prefix for glob size; when
               used for globs, it might be either "small glob of <foo>" or
               "small <foo> glob" and user might add 's' even though plural
               doesn't accomplish anything because globs don't stack */
            if (strncmpi(bp + l, "glob", 4) && !strstri(bp + l, " glob"))
                break;
            gsize = 1;
        } else if (!strncmpi(bp, "medium ", l = 7)) {
            /* xname() doesn't display "medium" but without this
               there'd be no way to ask for the intermediate size
               ("glob" without size prefix yields smallest one) */
            gsize = 2;
        } else if (!strncmpi(bp, "large ", l = 6)) {
            /* "large" might be part of monster name (dog, cat, koboold,
               mimic) or object name (box, round shield) rather than
               prefix for glob size */
            if (strncmpi(bp + l, "glob", 4) && !strstri(bp + l, " glob"))
                break;
            /* "very large " had "very " peeled off on previous iteration */
            gsize = (very != 1) ? 3 : 4;
        } else if (!strncmpi(bp, "oilskin ", l = 8)
                   && !!strncmpi(bp, "oilskin cloak", 13)
                   && !!strncmpi(bp, "oilskin sack", 12)) {
            if (!objpropcount || wizard)
                objprops |= ITEM_OILSKIN;
            objpropcount++;
        } else {
            /* check for materials */
            if (!strncmpi(bp, "silver dragon", l = 13)
                || !strncmpi(bp, "gold dragon", l = 11)) {
                /* hack so that gold/silver dragon scales doesn't get
                 * interpreted as silver, or a wish for just "gold" doesn't get
                 * interpreted as gold */
                break;
            }
            /* doesn't currently catch "wood" for wooden */
            for (i = 1; i < NUM_MATERIAL_TYPES; i++) {
                l = strlen(materialnm[i]);
                if (l > 0 && !strncmpi(bp, materialnm[i], l)
                    /* it LOOKS like a wish for a material...
                       but need to ensure that it's not just a wish for
                       something else that happens to have a prefix of a
                       material */
                    && !not_spec_material(bp, i))
                {
                    material = i;
                    l++;
                    break; /* from the for loop */
                }
            }
            if (i == NUM_MATERIAL_TYPES)
                /* no matching materials so no match for anything in this whole
                 * if chain */
                break;
        }
        bp += l;
    }
    if (!cnt)
        cnt = 1; /* will be changed to 2 if makesingular() changes string */
    if (strlen(bp) > 1 && (p = rindex(bp, '(')) != 0) {
        boolean keeptrailingchars = TRUE;

        p[(p > bp && p[-1] == ' ') ? -1 : 0] = '\0'; /*terminate bp */
        ++p; /* advance past '(' */
        if (!strncmpi(p, "lit)", 4)) {
            islit = 1;
            p += 4 - 1; /* point at ')' */
        } else {
            spe = atoi(p);
            while (digit(*p))
                p++;
            if (*p == ':') {
                p++;
                rechrg = spe;
                spe = atoi(p);
                while (digit(*p))
                    p++;
            }
            if (*p != ')') {
                spe = rechrg = 0;
                /* mis-matched parentheses; rest of string will be ignored
                 * [probably we should restore everything back to '('
                 * instead since it might be part of "named ..."]
                 */
                keeptrailingchars = FALSE;
            } else {
                spesgn = 1;
            }
        }
        if (keeptrailingchars) {
            char *pp = eos(bp);

            /* 'pp' points at 'pb's terminating '\0',
               'p' points at ')' and will be incremented past it */
            do {
                *pp++ = *++p;
            } while (*p);
        }
    }
    /*
     * otmp->spe is type schar, so we don't want spe to be any bigger or
     * smaller.  Also, spe should always be positive --some cheaters may
     * try to confuse atoi().
     */
    if (spe < 0) {
        spesgn = -1; /* cheaters get what they deserve */
        spe = abs(spe);
    }
    if (spe > SCHAR_LIM)
        spe = SCHAR_LIM;
    if (rechrg < 0 || rechrg > 7)
        rechrg = 7; /* recharge_limit */

    if (!bp || !*bp)
        goto any;

    /* now we have the actual name, as delivered by xname, say
     *  green potions called whisky
     *  scrolls labeled "QWERTY"
     *  egg
     *  fortune cookies
     *  very heavy iron ball named hoei
     *  wand of wishing
     *  elven cloak
     */
    if ((p = strstri(bp, " named ")) != 0) {
        *p = 0;
        /* note: if 'name' is too long, oname() will truncate it */
        name = p + 7;
    }
    if ((p = strstri(bp, " called ")) != 0) {
        *p = 0;
        /* note: if 'un' is too long, obj lookup just won't match anything */
        un = p + 8;
        /* "helmet called telepathy" is not "helmet" (a specific type)
         * "shield called reflection" is not "shield" (a general type)
         */
        for (i = 0; i < SIZE(o_ranges); i++)
            if (!strcmpi(bp, o_ranges[i].name)) {
                oclass = o_ranges[i].oclass;
                goto srch;
            }
    }
    if ((p = strstri(bp, " labeled ")) != 0) {
        *p = 0;
        dn = p + 9;
    } else if ((p = strstri(bp, " labelled ")) != 0) {
        *p = 0;
        dn = p + 10;
    }
    if ((p = strstri(bp, " of spinach")) != 0) {
        *p = 0;
        contents = SPINACH;
    }

    /*
     * Skip over "pair of ", "pairs of", "set of" and "sets of".
     *
     * Accept "3 pair of boots" as well as "3 pairs of boots".  It is
     * valid English either way.  See makeplural() for more on pair/pairs.
     *
     * We should only double count if the object in question is not
     * referred to as a "pair of".  E.g. We should double if the player
     * types "pair of spears", but not if the player types "pair of
     * lenses".  Luckily (?) all objects that are referred to as pairs
     * -- boots, gloves, and lenses -- are also not mergable, so cnt is
     * ignored anyway.
     */
    if (!strncmpi(bp, "pair of ", 8)) {
        bp += 8;
        cnt *= 2;
    } else if (!strncmpi(bp, "pairs of ", 9)) {
        bp += 9;
        if (cnt > 1)
            cnt *= 2;
    } else if (!strncmpi(bp, "set of ", 7)) {
        bp += 7;
    } else if (!strncmpi(bp, "sets of ", 8)) {
        bp += 8;
    }

    /* intercept pudding globs here; they're a valid wish target,
     * but we need them to not get treated like a corpse.
     *
     * also don't let player wish for multiple globs.
     */
    i = (int) strlen(bp);
    p = (char *) 0;
    /* check for "glob", "<foo> glob", and "glob of <foo>" */
    if (!strcmpi(bp, "glob") || !BSTRCMPI(bp, bp + i - 5, " glob")
        || !strcmpi(bp, "globs") || !BSTRCMPI(bp, bp + i - 6, " globs")
        || (p = strstri(bp, "glob of ")) != 0
        || (p = strstri(bp, "globs of ")) != 0) {
        mntmp = name_to_mon(!p ? bp : (strstri(p, " of ") + 4), (int *) 0);
        /* if we didn't recognize monster type, pick a valid one at random */
        if (mntmp == NON_PM)
            mntmp = rn1(PM_BLACK_PUDDING - PM_GRAY_OOZE, PM_GRAY_OOZE);
        /* construct canonical spelling in case name_to_mon() recognized a
           variant (grey ooze) or player used inverted syntax (<foo> glob);
           if player has given a valid monster type but not valid glob type,
           object name lookup won't find it and wish attempt will fail */
        Sprintf(globbuf, "glob of %s", mons[mntmp].mname);
        bp = globbuf;
        mntmp = NON_PM; /* not useful for "glob of <foo>" object lookup */
        cnt = 0; /* globs don't stack */
        oclass = FOOD_CLASS;
        actualn = bp, dn = 0;
        goto srch;
    } else {
        char *sng = makesingular(bp);
        /*
         * Find corpse type using "of" (figurine of an orc, tin of orc meat)
         * Don't check if it's a wand or spellbook.
         * (avoid "wand/finger of death" confusion).
         * (also avoid "sword of kas" or "eye/hand of vecna" issues)
         */
        if (!strstri(sng, "wand ") && !strstri(sng, "spellbook ")
            && !strstri(sng, "potion ") && !strstri(sng, "finger ")
            && !strstri(sng, "eye ") && !strstri(sng, "hand ")
            && !strstri(sng, "sword of kas")) {
            int l = 0, of = 4;
            char *tmpp;

            if ((p = strstri(bp, "tin of ")) != 0) {
                if (!strcmpi(p + 7, "spinach")) {
                    contents = SPINACH;
                    mntmp = NON_PM;
                } else {
                    tmp = tin_variety_txt(p + 7, &tinv);
                    tvariety = tinv;
                    mntmp = name_to_mon(p + 7 + tmp, (int *) 0);
                }
                typ = TIN;
                goto typfnd;
            }

            /* object properties */
            p = bp;
            while (p != 0) {
                tmpp = strstri(p, " of ");
                if (tmpp) {
                    if ((tmpp - 6 >= bp && !strncmpi(tmpp - 6, "amulet", 6))
                        || (tmpp - 6 >= bp && !strncmpi(tmpp - 6, "potion", 6))
                        || (tmpp - 4 >= bp && !strncmpi(tmpp - 4, "ring", 4))) {
                        p = tmpp + 4;
                        continue;
                    }
                    of = 4;
                    p = tmpp;
                } else {
                    tmpp = strstri(p, " and ");
                    if (tmpp) {
                        of = 5, p = tmpp;
                    } else {
                       p = 0;
                       break;
                    }
                }

                l = 0;

                if ((mntmp = name_to_mon(p + of, (int *) 0)) >= LOW_PM) {
                    *p = 0;
                    p = 0;
                } else if (!strncmpi((p + of), "fire", l = 4)
                           && strncmpi(bp, "scroll", l = 5)) {
                    if (!objpropcount || wizard)
                        objprops |= ITEM_FIRE;
                    objpropcount++;
                } else if ((!strncmpi((p + of), "frost", l = 5)
                           || !strncmpi((p + of), "cold", l = 4))) {
                    if (!objpropcount || wizard)
                        objprops |= ITEM_FROST;
                    objpropcount++;
                } else if (!strncmpi((p + of), "drain", l = 5)
                           || !strncmpi((p + of), "decay", l = 5)) {
                    if (!objpropcount || wizard)
                        objprops |= ITEM_DRLI;
                    objpropcount++;
                } else if (!strncmpi((p + of), "shock", l = 5)
                           || !strncmpi((p + of), "lightning", l = 9)) {
                    if (!objpropcount || wizard)
                        objprops |= ITEM_SHOCK;
                    objpropcount++;
                } else if (!strncmpi((p + of), "poison", l = 6)
                           || !strncmpi((p + of), "venom", l = 5)) {
                    if (!objpropcount || wizard)
                        objprops |= ITEM_VENOM;
                    objpropcount++;
                } else if ((!strncmpi((p + of), "telepathy", l = 9)
                            && strncmpi(bp, "helm", l = 4))
                           || !strncmpi((p + 4), "ESP", l = 3)) {
                    if (!objpropcount || wizard)
                        objprops |= ITEM_ESP;
                    objpropcount++;
                } else if (!strncmpi((p + of), "searching", l = 9)) {
                    if (!objpropcount || wizard)
                        objprops |= ITEM_SEARCHING;
                    objpropcount++;
                } else if (!strncmpi((p + of), "warning", l = 7)) {
                    if (!objpropcount || wizard)
                        objprops |= ITEM_WARNING;
                    objpropcount++;
                } else if (!strncmpi((p + of), "fumbling", l = 8)
                           && strncmpi(bp, "gauntlets", l = 9)) {
                    /* do not need to account for 'boots' here,
                       as they don't exist, you'd wish for 'fumble
                       boots' instead */
                    if (!objpropcount || wizard)
                        objprops |= ITEM_FUMBLING;
                    objpropcount++;
                } else if (!strncmpi((p + of), "hunger", l = 6)) {
                    if (!objpropcount || wizard)
                        objprops |= ITEM_HUNGER;
                    objpropcount++;
                } else if (!strncmpi((p + of), "excellence", l = 10)) {
                    if (!objpropcount || wizard)
                        objprops |= ITEM_EXCEL;
                    objpropcount++;
                } else
                    l = 0;

                if (l > 0)
                    *p = 0;

                if (p)
                    p += (of + l);
            }
        }
    }

    /* Alternate spellings (pick-ax, silver sabre, &c) */
    {
        const struct alt_spellings *as = spellings;

        while (as->sp) {
            if (fuzzymatch(bp, as->sp, " -", TRUE)) {
                typ = as->ob;
                goto typfnd;
            }
            as++;
        }
        /* can't use spellings list for this one due to shuffling */
        if (!strncmpi(bp, "grey spell", 10))
            *(bp + 2) = 'a';

        if ((p = strstri(bp, "armour")) != 0) {
            /* skip past "armo", then copy remainder beyond "u" */
            p += 4;
            while ((*p = *(p + 1)) != '\0')
                ++p; /* self terminating */
        }
    }

    /* Find corpse type w/o "of" (red dragon scales, yeti corpse) */
    if (strncmpi(bp, "samurai sword", 13)    /* not the "samurai" monster! */
        && strncmpi(bp, "wizard lock", 11)   /* not the "wizard" monster! */
        && strncmpi(bp, "ninja-to", 8)       /* not the "ninja" rank */
        && strncmpi(bp, "master key", 10)    /* not the "master" rank */
        && strncmpi(bp, "vampire blood", 13) /* not the "vampire" monster! */
        && strncmpi(bp, "magenta", 7)) {     /* not the "mage" rank */
        if (mntmp < LOW_PM && strlen(bp) > 2
            && (mntmp = name_to_mon(bp, (int *) 0)) >= LOW_PM) {
            int mntmptoo, mntmplen; /* double check for rank title */
            char *obp = bp;

            mntmptoo = title_to_mon(bp, (int *) 0, &mntmplen);
            bp += (mntmp != mntmptoo) ? (int) strlen(mons[mntmp].mname)
                                      : mntmplen;
            if (*bp == ' ') {
                bp++;
            } else if (!strncmpi(bp, "s ", 2)) {
                bp += 2;
            } else if (!strncmpi(bp, "es ", 3)) {
                bp += 3;
            } else if (!*bp && !actualn && !dn && !un && !oclass) {
                /* no referent; they don't really mean a monster type */
                bp = obp;
                mntmp = NON_PM;
            }
        }
    }

    /* first change to singular if necessary */
    if (*bp) {
        char *sng = makesingular(bp);

        if (strcmp(bp, sng)) {
            if (cnt == 1)
                cnt = 2;
            Strcpy(bp, sng);
        }
    }

    /* dragon scales - assumes order of dragons */
    if (!strcmpi(bp, "scales") && mntmp >= FIRST_DRAGON
        && mntmp <= LAST_DRAGON) {
        typ = mndx_to_dragon_scales(mntmp);
        mntmp = NON_PM; /* no monster */
        goto typfnd;
    }

    p = eos(bp);
    if (!BSTRCMPI(bp, p - 10, "holy water")) {
        typ = POT_WATER;
        if ((p - bp) >= 12 && *(p - 12) == 'u')
            iscursed = 1; /* unholy water */
        else
            blessed = 1;
        goto typfnd;
    }
    if (unlabeled && !BSTRCMPI(bp, p - 6, "scroll")) {
        typ = SCR_BLANK_PAPER;
        goto typfnd;
    }
    if (unlabeled && !BSTRCMPI(bp, p - 9, "spellbook")) {
        typ = SPE_BLANK_PAPER;
        goto typfnd;
    }
    /* specific food rather than color of gem/potion/spellbook[/scales] */
    if (!BSTRCMPI(bp, p - 6, "orange") && mntmp == NON_PM) {
        typ = ORANGE;
        goto typfnd;
    }
    /*
     * NOTE: Gold pieces are handled as objects nowadays, and therefore
     * this section should probably be reconsidered as well as the entire
     * gold/money concept.  Maybe we want to add other monetary units as
     * well in the future. (TH)
     */
    if (!BSTRCMPI(bp, p - 10, "gold piece")
        || !BSTRCMPI(bp, p - 7, "zorkmid")
        || !strcmpi(bp, "gold") || !strcmpi(bp, "money")
        || !strcmpi(bp, "coin") || *bp == GOLD_SYM) {
        if (cnt > 5000 && !wizard)
            cnt = 5000;
        else if (cnt < 1)
            cnt = 1;
        otmp = mksobj(GOLD_PIECE, FALSE, FALSE);
        otmp->quan = (long) cnt;
        otmp->owt = weight(otmp);
        context.botl = 1;
        return otmp;
    }

    /* check for single character object class code ("/" for wand, &c) */
    if (strlen(bp) == 1 && (i = def_char_to_objclass(*bp)) < MAXOCLASSES
        && i > ILLOBJ_CLASS && (i != VENOM_CLASS || wizard)) {
        oclass = i;
        goto any;
    }

    /* Search for class names: XXXXX potion, scroll of XXXXX.  Avoid */
    /* false hits on, e.g., rings for "ring mail". */
    if (strncmpi(bp, "enchant ", 8)
        && strncmpi(bp, "destroy ", 8)
        && strncmpi(bp, "detect food", 11)
        && strncmpi(bp, "food detection", 14)
        && strncmpi(bp, "ring mail", 9)
        && strncmpi(bp, "studded armor", 13)
        && strncmpi(bp, "armor", 5)
        && strncmpi(bp, "tooled horn", 11)
        && strncmpi(bp, "ring of p'", 10)
        && strncmpi(bp, "wand of orcus", 13)
        && strncmpi(bp, "food ration", 11)
        && strncmpi(bp, "meat ring", 9)
        && strncmpi(bp, "glamdring", 9)
        && strncmpi(bp, "one ring", 8))
        for (i = 0; i < (int) (sizeof wrpsym); i++) {
            register int j = strlen(wrp[i]);

            /* check for "<class> [ of ] something" */
            if (!strncmpi(bp, wrp[i], j)) {
                oclass = wrpsym[i];
                if (oclass != AMULET_CLASS) {
                    bp += j;
                    if (!strncmpi(bp, " of ", 4))
                        actualn = bp + 4;
                    /* else if(*bp) ?? */
                } else
                    actualn = bp;
                if (oclass != ARMOR_CLASS || !material)
                    goto srch;
                else {
                    oclass = 0;
                    typ = ARMOR;
                    goto typfnd;
                }
            }
            /* check for "something <class>" */
            if (!BSTRCMPI(bp, p - j, wrp[i])) {
                oclass = wrpsym[i];
                /* for "foo amulet", leave the class name so that
                   wishymatch() can do "of inversion" to try matching
                   "amulet of foo"; other classes don't include their
                   class name in their full object names (where
                   "potion of healing" is just "healing", for instance) */
                if (oclass != AMULET_CLASS) {
                    p -= j;
                    *p = '\0';
                    if (p > bp && p[-1] == ' ')
                        p[-1] = '\0';
                } else {
                    /* amulet without "of"; convoluted wording but better a
                       special case that's handled than one that's missing */
                    if (!strncmpi(bp, "versus poison ", 14)) {
                        typ = AMULET_VERSUS_POISON;
                        goto typfnd;
                    }
                }
                actualn = dn = bp;
                if (oclass != ARMOR_CLASS || !material)
                    goto srch;
                else {
                    oclass = 0;
                    typ = ARMOR;
                    goto typfnd;
                }
            }
        }

    /* Wishing in wizard mode can create traps and furniture.
     * Part I:  distinguish between trap and object for the two
     * types of traps which have corresponding objects:  bear trap
     * and land mine.  "beartrap" (object) and "bear trap" (trap)
     * have a difference in spelling which we used to exploit by
     * adding a special case in wishymatch(), but "land mine" is
     * spelled the same either way so needs different handing.
     * Since we need something else for land mine, we've dropped
     * the bear trap hack so that both are handled exactly the
     * same.  To get an armed trap instead of a disarmed object,
     * the player can prefix either the object name or the trap
     * name with "trapped " (which ordinarily applies to chests
     * and tins), or append something--anything at all except for
     * " object", but " trap" is suggested--to either the trap
     * name or the object name.
     * Similarly, wishing for a "magic chest" will give the terrain
     * unless specifically wishing for a "magic chest object."
     */
    if (wizard && (!strncmpi(bp, "bear", 4) || !strncmpi(bp, "land", 4)
        || !strncmpi (bp, "magic chest", 11))) {
        boolean beartrap = (lowc(*bp) == 'b');
        boolean landmine = (lowc(*bp) == 'l');
        boolean four = (beartrap || landmine);
        char *zp = bp + (four ? 4 : 11); /* skip "bear"/"land" */

        if (*zp == ' ')
            ++zp; /* embedded space is optional */
        if (four && !strncmpi(zp, beartrap ? "trap" : "mine", 4)) {
            zp += 4;
            if (trapped == 2 || !strcmpi(zp, " object")) {
                /* "untrapped <foo>" or "<foo> object" */
                typ = beartrap ? BEARTRAP : LAND_MINE;
                goto typfnd;
            } else if (trapped == 1 || *zp != '\0') {
                /* "trapped <foo>" or "<foo> trap" (actually "<foo>*") */
                int idx = trap_to_defsym(beartrap ? BEAR_TRAP : LANDMINE);

                /* use canonical trap spelling, skip object matching */
                Strcpy(bp, defsyms[idx].explanation);
                goto wiztrap;
            }
            /* [no prefix or suffix; we're going to end up matching
               the object name and getting a disarmed trap object] */
        } else {
            if (!strcmpi(zp, "object")) {
                typ = HIDDEN_CHEST;
                goto typfnd;
            } else {
                Strcpy(bp, "magic chest");
                goto wiztrap;
            }
        }
    }

 retry:
    /* "grey stone" check must be before general "stone" */
    for (i = 0; i < SIZE(o_ranges); i++)
        if (!strcmpi(bp, o_ranges[i].name)) {
            typ = rnd_class(o_ranges[i].f_o_range, o_ranges[i].l_o_range);
            goto typfnd;
        }

    if (!BSTRCMPI(bp, p - 6, " stone") || !BSTRCMPI(bp, p - 4, " gem")) {
        p[!strcmpi(p - 4, " gem") ? -4 : -6] = '\0';
        oclass = GEM_CLASS;
        dn = actualn = bp;
        goto srch;
    } else if (!strcmpi(bp, "looking glass")) {
        ; /* avoid false hit on "* glass" */
    } else if (!BSTRCMPI(bp, p - 6, " glass") || !strcmpi(bp, "glass")) {
        register char *g = bp;

        /* treat "broken glass" as a non-existent item; since "broken" is
           also a chest/box prefix it might have been stripped off above */
        if (broken || strstri(g, "broken"))
            return (struct obj *) 0;
        if (!strncmpi(g, "worthless ", 10))
            g += 10;
        if (!strncmpi(g, "piece of ", 9))
            g += 9;
        if (!strncmpi(g, "colored ", 8))
            g += 8;
        else if (!strncmpi(g, "coloured ", 9))
            g += 9;
        if (!strcmpi(g, "glass")) { /* choose random color */
            /* 9 different kinds */
            typ = LAST_GEM + rnd(9);
            if (objects[typ].oc_class == GEM_CLASS)
                goto typfnd;
            else
                typ = 0; /* somebody changed objects[]? punt */
        } else { /* try to construct canonical form */
            char tbuf[BUFSZ];

            Strcpy(tbuf, "worthless piece of ");
            Strcat(tbuf, g); /* assume it starts with the color */
            Strcpy(bp, tbuf);
        }
    }

    actualn = bp;
    if (!dn)
        dn = actualn; /* ex. "skull cap" */
 srch:
    /* check real names of gems first */
    if (!oclass && actualn) {
        for (i = bases[GEM_CLASS]; i <= LAST_GEM; i++) {
            register const char *zn;

            if ((zn = OBJ_NAME(objects[i])) != 0 && !strcmpi(actualn, zn)) {
                typ = i;
                goto typfnd;
            }
        }
        /* "tin of foo" would be caught above, but plain "tin" has
           a random chance of yielding "tin wand" unless we do this */
        if (!strcmpi(actualn, "tin")) {
            typ = TIN;
            goto typfnd;
        }
    }

    if (((typ = rnd_otyp_by_namedesc(actualn, oclass, 1)) != STRANGE_OBJECT)
        || ((typ = rnd_otyp_by_namedesc(dn, oclass, 1)) != STRANGE_OBJECT)
        || ((typ = rnd_otyp_by_namedesc(un, oclass, 1)) != STRANGE_OBJECT)
        || ((typ = rnd_otyp_by_namedesc(origbp, oclass, 1)) != STRANGE_OBJECT))
        goto typfnd;
    typ = 0;

    if (actualn) {
        struct Jitem *j = Japanese_items;

        while (j->item) {
            if (actualn && !strcmpi(actualn, j->name)) {
                typ = j->item;
                goto typfnd;
            }
            j++;
        }
    }
    /* if we've stripped off "armor" and failed to match anything
       in objects[], append "mail" and try again to catch misnamed
       requests like "plate armor" and "yellow dragon scale armor" */
    if (oclass == ARMOR_CLASS && !strstri(bp, "mail")) {
        /* modifying bp's string is ok; we're about to resort
           to random armor if this also fails to match anything */
        Strcat(bp, " mail");
        goto retry;
    }
    if (!strcmpi(bp, "spinach")) {
        contents = SPINACH;
        typ = TIN;
        goto typfnd;
    }
    /* Note: not strcmpi.  2 fruits, one capital, one not, are possible.
       Also not strncmp.  We used to ignore trailing text with it, but
       that resulted in "grapefruit" matching "grape" if the latter came
       earlier than the former in the fruit list. */
    {
        char *fp;
        int l, cntf;
        int blessedf, iscursedf, uncursedf, halfeatenf;

        blessedf = iscursedf = uncursedf = halfeatenf = 0;
        cntf = 0;

        fp = fruitbuf;
        for (;;) {
            if (!fp || !*fp)
                break;
            if (!strncmpi(fp, "an ", l = 3) || !strncmpi(fp, "a ", l = 2)) {
                cntf = 1;
            } else if (!cntf && digit(*fp)) {
                cntf = atoi(fp);
                while (digit(*fp))
                    fp++;
                while (*fp == ' ')
                    fp++;
                l = 0;
            } else if (!strncmpi(fp, "blessed ", l = 8)) {
                blessedf = 1;
            } else if (!strncmpi(fp, "cursed ", l = 7)) {
                iscursedf = 1;
            } else if (!strncmpi(fp, "uncursed ", l = 9)) {
                uncursedf = 1;
            } else if (!strncmpi(fp, "partly eaten ", l = 13)
                       || !strncmpi(fp, "partially eaten ", l = 16)) {
                halfeatenf = 1;
            } else
                break;
            fp += l;
        }

        for (f = ffruit; f; f = f->nextf) {
            /* match type: 0=none, 1=exact, 2=singular, 3=plural */
            int ftyp = 0;

            if (!strcmp(fp, f->fname))
                ftyp = 1;
            else if (!strcmp(fp, makesingular(f->fname)))
                ftyp = 2;
            else if (!strcmp(fp, makeplural(f->fname)))
                ftyp = 3;
            if (ftyp) {
                typ = SLIME_MOLD;
                blessed = blessedf;
                iscursed = iscursedf;
                uncursed = uncursedf;
                halfeaten = halfeatenf;
                /* adjust count if user explicitly asked for
                   singular amount (can't happen unless fruit
                   has been given an already pluralized name)
                   or for plural amount */
                if (ftyp == 2 && !cntf)
                    cntf = 1;
                else if (ftyp == 3 && !cntf)
                    cntf = 2;
                cnt = cntf;
                ftype = f->fid;
                goto typfnd;
            }
        }
    }

    if (!oclass && actualn) {
        short objtyp;

        /* Perhaps it's an artifact specified by name, not type */
        name = artifact_name(actualn, &objtyp);
        if (name) {
            typ = objtyp;
            goto typfnd;
        }
    }

    /*
     * Let wizards wish for traps and furniture.
     * Must come after objects check so wizards can still wish for
     * trap objects like beartraps.
     * Disallow such topology tweaks for WIZKIT startup wishes.
     */
 wiztrap:
    if (wizard && !program_state.wizkit_wishing) {
        struct rm *lev;
        boolean madeterrain = FALSE;
        int trap, x = u.ux, y = u.uy;

        for (trap = NO_TRAP + 1; trap < TRAPNUM; trap++) {
            struct trap *t;
            const char *tname;

            tname = defsyms[trap_to_defsym(trap)].explanation;
            if (strncmpi(tname, bp, strlen(tname)))
                continue;
            /* found it; avoid stupid mistakes */
            if (is_hole(trap) && !Can_fall_thru(&u.uz))
                trap = ROCKTRAP;
            if ((t = maketrap(x, y, trap)) != 0) {
                trap = t->ttyp;
                tname = defsyms[trap_to_defsym(trap)].explanation;
                pline("%s%s.", An(tname),
                      (trap != MAGIC_PORTAL) ? "" : " to nowhere");
            } else
                pline("Creation of %s failed.", an(tname));
            return (struct obj *) &zeroobj;
        }

        /* furniture and terrain (use at your own risk; can clobber stairs
           or place furniture on existing traps which shouldn't be allowed) */
        lev = &levl[x][y];
        p = eos(bp);
        if (!BSTRCMPI(bp, p - 8, "fountain")) {
            lev->typ = FOUNTAIN;
            level.flags.nfountains++;
            if (!strncmpi(bp, "magic ", 6))
                lev->blessedftn = 1;
            pline("A %sfountain.", lev->blessedftn ? "magic " : "");
            madeterrain = TRUE;
        } else if (!BSTRCMPI(bp, p - 6, "throne")) {
            lev->typ = THRONE;
            pline("A throne.");
            madeterrain = TRUE;
        } else if (!BSTRCMPI(bp, p - 4, "sink")) {
            lev->typ = SINK;
            level.flags.nsinks++;
            pline("A sink.");
            madeterrain = TRUE;
        } else if (!BSTRCMPI(bp, p - 11, "magic chest")) {
            lev->typ = MAGIC_CHEST;
            level.flags.nmagicchests++;
            pline("A magic chest.");
            madeterrain = TRUE;
        } else if (!BSTRCMPI(bp, p - 5, "forge")) {
            lev->typ = FORGE;
            level.flags.nforges++;
            pline("A forge.");
            madeterrain = TRUE;

        /* ("water" matches "potion of water" rather than terrain) */
        } else if (!BSTRCMPI(bp, p - 4, "pool")
                   || !BSTRCMPI(bp, p - 4, "moat")) {
            lev->typ = !BSTRCMPI(bp, p - 4, "pool") ? POOL : MOAT;
            del_engr_at(x, y);
            pline("A %s.", (lev->typ == POOL) ? "pool" : "moat");
            /* Must manually make kelp! */
            water_damage_chain(level.objects[x][y], TRUE, 0, TRUE, x, y);
            madeterrain = TRUE;
        } else if (!BSTRCMPI(bp, p - 13, "shallow water")
                   || !BSTRCMPI(bp, p - 6, "puddle")) {
            lev->typ = PUDDLE;
            del_engr_at(x, y);
            pline("Shallow water.");
            water_damage(level.objects[x][y], 0, TRUE, x, y);
            madeterrain = TRUE;
        } else if (!BSTRCMPI(bp, p - 6, "sewage")) {
            lev->typ = SEWAGE;
            del_engr_at(x, y);
            pline("Sewage.");
            water_damage(level.objects[x][y], 0, TRUE, x, y);
            madeterrain = TRUE;

        /* also matches "molten lava" */
        } else if (!BSTRCMPI(bp, p - 4, "lava")) {
            lev->typ = LAVAPOOL;
            del_engr_at(x, y);
            pline("A pool of molten lava.");
            if (!(Levitation || Flying))
                pooleffects(FALSE);
            madeterrain = TRUE;
        } else if (!BSTRCMPI(bp, p - 5, "altar")) {
            aligntyp al;

            lev->typ = ALTAR;
            if (!strncmpi(bp, "chaotic ", 8))
                al = A_CHAOTIC;
            else if (!strncmpi(bp, "neutral ", 8))
                al = A_NEUTRAL;
            else if (!strncmpi(bp, "lawful ", 7))
                al = A_LAWFUL;
            else if (!strncmpi(bp, "unaligned ", 10))
                al = A_NONE;
            else /* -1 - A_CHAOTIC, 0 - A_NEUTRAL, 1 - A_LAWFUL */
                al = !rn2(6) ? A_NONE : (rn2((int) A_LAWFUL + 2) - 1);
            lev->altarmask = Align2amask(al);
            /* TODO: allow fractured altars to be wished for
               in wizmode. I really hate objnam.c */
            if (lev->frac_altar)
                pline("A fractured %s altar.", align_str(al));
            else
                pline("%s altar.", An(align_str(al)));
            madeterrain = TRUE;
        } else if (!BSTRCMPI(bp, p - 5, "grave")
                   || !BSTRCMPI(bp, p - 9, "headstone")) {
            make_grave(x, y, (char *) 0);
            pline("%s.", IS_GRAVE(lev->typ) ? "A grave"
                                            : "Can't place a grave here");
            madeterrain = TRUE;
        } else if (!BSTRCMPI(bp, p - 9, "dead tree")) {
            lev->typ = DEADTREE;
            pline("A dead tree.");
            block_point(x, y);
            madeterrain = TRUE;
        } else if (!BSTRCMPI(bp, p - 4, "tree")) {
            lev->typ = TREE;
            pline("A tree.");
            block_point(x, y);
            madeterrain = TRUE;
        } else if (!BSTRCMPI(bp, p - 5, "grass")) {
            lev->typ = GRASS;
            pline("Some grass.");
            madeterrain = TRUE;
        } else if (!BSTRCMPI(bp, p - 4, "sand")) {
            lev->typ = SAND;
            pline("Some sand.");
            madeterrain = TRUE;
        } else if (!BSTRCMPI(bp, p - 4, "bars")) {
            lev->typ = IRONBARS;
            pline("Iron bars.");
            madeterrain = TRUE;
        }

        if (madeterrain) {
            feel_newsym(x, y); /* map the spot where the wish occurred */
            /* hero started at <x,y> but might not be there anymore (create
               lava, decline to die, and get teleported away to safety) */
            if (u.uinwater && !is_pool(u.ux, u.uy)) {
                u.uinwater = 0; /* leave the water */
                docrt();
                vision_full_recalc = 1;
            } else if (u.utrap && u.utraptype == TT_LAVA
                       && !is_lava(u.ux, u.uy)) {
                reset_utrap(FALSE);
            }
            /* cast 'const' away; caller won't modify this */
            return (struct obj *) &zeroobj;
        }
    } /* end of wizard mode traps and terrain */

    if (!oclass && !typ) {
        if (!strncmpi(bp, "polearm", 7)) {
            typ = rnd_otyp_by_wpnskill(P_POLEARMS);
            goto typfnd;
        } else if (!strncmpi(bp, "hammer", 6)) {
            typ = rnd_otyp_by_wpnskill(P_HAMMER);
            goto typfnd;
        }
    }

    if (!oclass && material == GOLD) {
        /* things like "5000 gold" */
        oclass = COIN_CLASS;
        typ = GOLD_PIECE;
    }
    if (!oclass)
        return ((struct obj *) 0);
 any:
    if (!oclass)
        oclass = wrpsym[rn2((int) sizeof wrpsym)];
 typfnd:
    if (typ)
        oclass = objects[typ].oc_class;

    /* handle some objects that are only allowed in wizard mode */
    if (typ && !wizard) {
        switch (typ) {
        case AMULET_OF_YENDOR:
            typ = FAKE_AMULET_OF_YENDOR;
            break;
        case CANDELABRUM_OF_INVOCATION:
            typ = rnd_class(TALLOW_CANDLE, WAX_CANDLE);
            break;
        case BELL_OF_OPENING:
            typ = BELL;
            break;
        case SPE_BOOK_OF_THE_DEAD:
            typ = SPE_BLANK_PAPER;
            break;
        case CHROMATIC_DRAGON_SCALES:
            typ = rnd_class(FIRST_DRAGON_SCALES, LAST_DRAGON_SCALES - 1);
            break;
        case BLACKSMITH_HAMMER:
            typ = WAR_HAMMER;
            break;
        case SCR_CONSECRATION:
            typ = SCR_BLANK_PAPER;
            break;
        default:
            /* catch any other non-wishable objects (venom) */
            if (objects[typ].oc_nowish)
                return (struct obj *) 0;
            break;
        }
    }

    /* players are likely to wish for "foo dragon scale mail" by reflex, which
     * no longer exists and would now ignore the dragon type and give a plain
     * scale mail; don't screw over players who aren't aware of the dtsund-DSM
     * changes - produce a set of scales instead of nothing */
    if (typ == SCALE_MAIL && mntmp >= FIRST_DRAGON
        && mntmp <= PM_CELESTIAL_DRAGON) { /* chromatic dragon scales are off-limits */
        typ = mndx_to_dragon_scales(mntmp);
        mntmp = NON_PM; /* no monster */
    }

    /*
     * Create the object, then fine-tune it.
     */
    otmp = typ ? mksobj(typ, TRUE, FALSE) : mkobj(oclass, FALSE);
    typ = otmp->otyp, oclass = otmp->oclass; /* what we actually got */

    if (islit && (typ == OIL_LAMP || typ == MAGIC_LAMP || typ == LANTERN
                  || Is_candle(otmp) || typ == POT_OIL)) {
        place_object(otmp, u.ux, u.uy); /* make it viable light source */
        begin_burn(otmp, FALSE);
        obj_extract_self(otmp); /* now release it for caller's use */
    }

    /* if player specified a reasonable count, maybe honor it */
    if (cnt > 1 && objects[typ].oc_merge
        && (wizard || cnt < rnd(6) || (cnt <= 7 && Is_candle(otmp))
            || (cnt <= 20 && ((oclass == WEAPON_CLASS && is_ammo(otmp))
                              || typ == ROCK || is_missile(otmp)))))
        otmp->quan = (long) cnt;

    if (oclass == VENOM_CLASS)
        otmp->spe = 1;

    if (spesgn == 0) {
        spe = otmp->spe;
    } else if (wizard) {
        ; /* no alteration to spe */
    } else if (oclass == ARMOR_CLASS || oclass == WEAPON_CLASS
               || is_weptool(otmp) || is_barding(otmp)
               || (oclass == RING_CLASS && objects[typ].oc_charged)) {
        if (spe > rnd(5) && spe > otmp->spe)
            spe = 0;
        if (spe > 2 && Luck < 0)
            spesgn = -1;
    } else {
        if (oclass == WAND_CLASS) {
            if (spe > 1 && spesgn == -1)
                spe = 1;
        } else {
            if (spe > 0 && spesgn == -1)
                spe = 0;
        }
        if (spe > otmp->spe)
            spe = otmp->spe;
    }

    if (spesgn == -1)
        spe = -spe;

    /* set otmp->spe.  This may, or may not, use spe... */
    switch (typ) {
    case TIN:
        if (contents == EMPTY) {
            otmp->corpsenm = NON_PM;
            otmp->spe = 0;
        } else if (contents == SPINACH) {
            otmp->corpsenm = NON_PM;
            otmp->spe = 1;
        }
        break;
    case TOWEL:
        if (wetness)
            otmp->spe = wetness;
        break;
    case SLIME_MOLD:
        otmp->spe = ftype;
    /* Fall through */
    case SKELETON_KEY:
    case CHEST:
    case LARGE_BOX:
    case IRON_SAFE:
    case CRYSTAL_CHEST:
    case HEAVY_IRON_BALL:
    case IRON_CHAIN:
    case STATUE:
        /* otmp->cobj already done in mksobj() */
        break;
#ifdef MAIL
    case SCR_MAIL:
        /* 0: delivered in-game via external event (or randomly for fake mail);
           1: from bones or wishing; 2: written with marker */
        otmp->spe = 1;
        break;
#endif
    case WAN_WISHING:
        if (!wizard) {
            otmp->spe = (rn2(10) ? -1 : 0);
            break;
        }
        /*FALLTHRU*/
    default:
        otmp->spe = spe;
    }

    /* set otmp->corpsenm */
    if (mntmp >= LOW_PM) {
        if (mntmp == PM_LONG_WORM_TAIL)
            mntmp = PM_LONG_WORM;

        switch (typ) {
        case TIN:
            otmp->spe = 0; /* No spinach */
            if (dead_species(mntmp, FALSE)) {
                otmp->corpsenm = NON_PM; /* it's empty */
            } else if ((!(mons[mntmp].geno & G_UNIQ) || wizard)
                       && !(mvitals[mntmp].mvflags & G_NOCORPSE)
                       && mons[mntmp].cnutrit != 0) {
                otmp->corpsenm = mntmp;
            }
            break;
        case CORPSE:
            if ((!(mons[mntmp].geno & G_UNIQ) || wizard)
                && !(mvitals[mntmp].mvflags & G_NOCORPSE)) {
                if (mons[mntmp].msound == MS_GUARDIAN)
                    mntmp = genus(mntmp, 1);
                set_corpsenm(otmp, mntmp);
            }
            if (zombo && zombie_form(&mons[mntmp])) {
                (void) start_timer(rn1(5, 10), TIMER_OBJECT,
                                   ZOMBIFY_MON, obj_to_any(otmp));
            }
            break;
        case EGG:
            mntmp = can_be_hatched(mntmp);
            /* this also sets hatch timer if appropriate */
            set_corpsenm(otmp, mntmp);
            break;
        case FIGURINE:
            if (!(mons[mntmp].geno & G_UNIQ) && !is_human(&mons[mntmp])
#ifdef MAIL
                && mntmp != PM_MAIL_DAEMON
#endif
                )
                otmp->corpsenm = mntmp;
            break;
        case STATUE:
            otmp->corpsenm = mntmp;
            if (Has_contents(otmp) && verysmall(&mons[mntmp]))
                delete_contents(otmp); /* no spellbook */
            otmp->spe = ishistoric ? STATUE_HISTORIC : 0;
            break;
        }
    }

    /* set blessed/cursed -- setting the fields directly is safe
     * since weight() is called below and addinv() will take care
     * of luck */
    if (iscursed) {
        curse(otmp);
    } else if (uncursed) {
        otmp->blessed = 0;
        otmp->cursed = (Luck < 0 && !wizard);
    } else if (blessed) {
        otmp->blessed = (Luck >= 0 || wizard);
        otmp->cursed = (Luck < 0 && !wizard);
    } else if (spesgn < 0) {
        curse(otmp);
    }
    otmp->oprops = 0;
    if (magical && (!objpropcount || wizard)) {
        create_oprop(otmp, TRUE);
    }

    /* set otmp->recharged */
    if (oclass == WAND_CLASS) {
        /* prevent wishing abuse */
        if (otmp->otyp == WAN_WISHING && !wizard)
            rechrg = 1;
        otmp->recharged = (unsigned) rechrg;
    }

    /* set poisoned */
    if (ispoisoned) {
        if (is_poisonable(otmp))
            otmp->opoisoned = (Luck >= 0);
        else if (oclass == FOOD_CLASS)
            /* try to taint by making it as old as possible */
            otmp->age = 1L;
    }
    /* set tainted */
    if (istainted) {
        if (is_poisonable(otmp))
            otmp->otainted = (Luck >= 0);
    }
    /* set forge quality */
    if (isforged0)
        otmp->forged_qual = FQ_INFERIOR;
    if (isforged1)
        otmp->forged_qual = FQ_SUPERIOR;
    if (isforged2)
        otmp->forged_qual = FQ_EXCEPTIONAL;
    /* and [un]trapped */
    if (trapped) {
        if (Is_box(otmp) || typ == TIN)
            otmp->otrapped = (trapped == 1);
    }
    /* empty for containers rather than for tins */
    if (contents == EMPTY) {
        if (otmp->otyp == BAG_OF_TRICKS || otmp->otyp == HORN_OF_PLENTY) {
            if (otmp->spe > 0)
                otmp->spe = 0;
        } else if (Has_contents(otmp)) {
            /* this assumes that artifacts can't be randomly generated
               inside containers */
            delete_contents(otmp);
            otmp->owt = weight(otmp);
        }
    }
    /* set locked/unlocked/broken */
    if (Is_box(otmp)) {
        if (locked) {
            otmp->olocked = 1, otmp->obroken = 0;
        } else if (unlocked) {
            otmp->olocked = 0, otmp->obroken = 0;
        } else if (broken) {
            otmp->olocked = 0, otmp->obroken = 1;
        }
    }

    if (isgreased)
        otmp->greased = 1;

    if (isdiluted && otmp->oclass == POTION_CLASS && otmp->otyp != POT_WATER)
        otmp->odiluted = 1;

    /* set tin variety */
    if (otmp->otyp == TIN && tvariety >= 0 && (rn2(4) || wizard))
        set_tin_variety(otmp, tvariety);

    if (name) {
        const char *aname;
        short objtyp;

        /* an artifact name might need capitalization fixing */
        aname = artifact_name(name, &objtyp);
        if (aname && objtyp == otmp->otyp)
            name = aname;

        /* 3.6 tribute - fix up novel */
        if (otmp->otyp == SPE_NOVEL) {
            const char *novelname;

            novelname = lookup_novel(name, &otmp->novelidx);
            if (novelname)
                name = novelname;
        }

        otmp = oname(otmp, name);
        /* name==aname => wished for artifact (otmp->oartifact => got it) */
        if (otmp->oartifact || name == aname) {
            /* prevent artifacts from being wished for with
               quality bit applied, adjust this if we want
               certain artifacts to have a forged quality bit */
            otmp->forged_qual = FQ_NORMAL;
            otmp->quan = 1L;
            u.uconduct.wisharti++; /* KMH, conduct */
        }
    }

    /* more wishing abuse: don't allow wishing for the quest artifact
     * and make them pay; charge them for the wish anyway!
     * otherwise an increasing probability that the artifact returns
     * with its previous owner
     */
    if ((is_quest_artifact(otmp)
        || non_wishable_artifact(otmp)
        || (Role_if(PM_RANGER)
            && ((Race_if(PM_GNOME) && otmp->oartifact == ART_LONGBOW_OF_DIANA)
                || (!Race_if(PM_GNOME) && otmp->oartifact == ART_CROSSBOW_OF_CARL)))
        || (otmp->oartifact && rn2(u.uconduct.wisharti) > 1)) && !wizard) {
        artifact_exists(otmp, ONAME(otmp), FALSE);
        obfree(otmp, (struct obj *) 0);
        otmp = (struct obj *) &zeroobj;

        if (Hallucination)
            pline("Wish in one hand...");
        else
            pline("For a moment, you feel %s in your %s, but it disappears!",
                  something, makeplural(body_part(HAND)));
        return otmp;
    } else if (otmp->oartifact
               && (rn2(u.uconduct.wisharti)
                   || any_quest_artifact(otmp))
               && !(wizard && (program_state.wizkit_wishing
                               || yn("Deal with previous owner?") == 'n'))) {
        int pm = -1;
        int strategy = NEED_HTH_WEAPON;
        struct monst *mtmp;
        struct obj *otmp2 = (struct obj *) 0;
        /* You can use otmp2 to give the owner some other item you want to.
           Used here to give ammunition for the Ranger artifacts. */
        const char *aname = artiname(otmp->oartifact);

        /* Wishing for a quest artifact may summon its rightful owner */
        if (any_quest_artifact(otmp)) {
            const struct Role *role = roles;

            while ((role->name.m) && (role->questarti != otmp->oartifact))
                role++;
            if (role->name.m) {
                /* Don't bring the Rogue's leader to fight a Tourist when he is also
                 * the Tourist's quest nemesis.
                 */
                if (!((role->ldrnum == PM_MASTER_OF_THIEVES)
                      && Role_if(PM_TOURIST))) {
                    struct permonst* ldr;

                    pm = role->ldrnum;
                    /* remove flags that tag quest leaders as
                       peaceful or spawn them mediating */
                    ldr = &mons[pm];
                    ldr->mflags2 &= ~(M2_PEACEFUL);
                    ldr->mflags3 &= ~(M3_WAITMASK);
                }
            }
        }

        if (pm < 0) {
            switch(otmp->oartifact) {
            case ART_XIUHCOATL:
            case ART_SUNSWORD:
            case ART_GRAYSWANDIR:
                pm = PM_ARCHEOLOGIST;
                break;
            case ART_RING_OF_P_HUL:
            case ART_CLEAVER:
            case ART_STORMBRINGER:
            case ART_OGRESMASHER:
                pm = PM_BARBARIAN;
                break;
            case ART_KEOLEWA:
            case ART_DRAGONBANE:
            case ART_SCEPTRE_OF_MIGHT:
                pm = PM_CAVEMAN;
                break;
            case ART_LUCK_BLADE:
            case ART_STRIPED_SHIRT_OF_LIBERATIO:
                pm = PM_CONVICT;
                break;
            case ART_BRACERS_OF_THE_FIRST_CIRCL:
                pm = PM_DRUID;
                break;
            case ART_STAFF_OF_AESCULAPIUS:
                pm = PM_HEALER;
                break;
            case ART_IDOL_OF_MOLOCH:
            case ART_SECESPITA:
            case ART_ANGELSLAYER:
                pm = PM_INFIDEL;
                break;
            case ART_MAGIC_MIRROR_OF_MERLIN:
            case ART_EXCALIBUR:
            case ART_DIRGE:
                pm = PM_KNIGHT;
                break;
            case ART_EYES_OF_THE_OVERWORLD:
                pm = PM_MONK;
                break;
            case ART_MITRE_OF_HOLINESS:
            case ART_TROLLSBANE:
                pm = PM_PRIEST;
                break;
            case ART_LONGBOW_OF_DIANA:
                otmp2 = mksobj(ARROW, TRUE, FALSE);
                otmp2->quan = (long) rn1(20, 10);
                otmp2->owt = weight(otmp2);
                otmp2->blessed = otmp2->cursed = 0;
                otmp2->spe = rn2(3);
                strategy = NEED_RANGED_WEAPON;
                pm = PM_RANGER;
                break;
            case ART_CROSSBOW_OF_CARL:
                otmp2 = mksobj(CROSSBOW_BOLT, TRUE, FALSE);
                otmp2->quan = (long) rn1(20, 10);
                otmp2->owt = weight(otmp2);
                otmp2->blessed = otmp2->cursed = 0;
                otmp2->spe = rn2(3);
                strategy = NEED_RANGED_WEAPON;
                pm = PM_RANGER;
                break;
            case ART_ORCRIST:
                pm = PM_RANGER;
                break;
            case ART_MASTER_KEY_OF_THIEVERY:
            case ART_STING:
            case ART_GRIMTOOTH:
                pm = PM_ROGUE;
                break;
            case ART_TSURUGI_OF_MURAMASA:
            case ART_SNICKERSNEE:
                pm = PM_SAMURAI;
                break;
            case ART_YENDORIAN_EXPRESS_CARD:
            case ART_VORPAL_BLADE:
                pm = PM_TOURIST;
                break;
            case ART_GJALLAR:
            case ART_GIANTSLAYER:
            case ART_MJOLLNIR:
            case ART_DRAMBORLEG:
                pm = PM_VALKYRIE;
                break;
            case ART_EYE_OF_THE_AETHIOPICA:
            case ART_MAGICBANE:
                pm = PM_WIZARD;
                break;
            case ART_FROST_BRAND:
                if (u.ualign.type == A_NEUTRAL)
                    pm = PM_TOURIST;
                else
                    pm = PM_KNIGHT;
                break;
            case ART_FIRE_BRAND:
                if (u.ualign.type == A_CHAOTIC)
                    pm = PM_BARBARIAN;
                else
                    pm = PM_ARCHEOLOGIST;
                break;
            case ART_DEMONBANE:
                if (u.ualign.type == A_NEUTRAL)
                    pm = PM_HEALER;
                else
                    pm = PM_PRIEST;
                break;
            case ART_WEREBANE:
                if (u.ualign.type == A_CHAOTIC)
                    pm = PM_BARBARIAN;
                else
                    pm = PM_DRUID;
                break;
            /* non-wishable artifacts (wizmode only) */
            case ART_MAGIC___BALL:
            case ART_LIFESTEALER:
            case ART_BAG_OF_THE_HESPERIDES:
            case ART_BUTCHER:
            case ART_WAND_OF_ORCUS:
            case ART_EYE_OF_VECNA:
            case ART_HAND_OF_VECNA:
            case ART_SWORD_OF_KAS:
            case ART_ONE_RING:
            case ART_SWORD_OF_ANNIHILATION:
            case ART_GLAMDRING:
            case ART_STAFF_OF_THE_ARCHMAGI:
            case ART_SHADOWBLADE:
            case ART_GAUNTLETS_OF_PURITY:
            case ART_ASHMAR:
            case ART_HAMMER_OF_THE_GODS:
            case ART_TEMPEST:
            case ART_ITHILMAR:
            case ART_ARMOR_OF_RETRIBUTION:
            case ART_DICHOTOMY:
            case ART_HARBINGER:
                pm = PM_SAMURAI;
                break;
            default:
                impossible("Unknown artifact!");
                break;
            }
            if (pm == PM_CAVEMAN && rn2(2))
                pm = PM_CAVEWOMAN;
            if (pm == PM_PRIEST && rn2(2))
                pm = PM_PRIESTESS;
        }

        mtmp = mk_mplayer(&mons[pm], u.ux, u.uy, TRUE);
        if (mtmp && !is_mplayer(mtmp->data))
            free_mname(mtmp);
        if (mtmp) {
            if (Blind) {
                if (Hallucination)
                    pline("Smells like teen spirit...");
                else
                    You("%ssmell smoke.",
                        Deaf ? "" : "hear a small explosion and ");
            } else {
                if (Hallucination)
                    pline("Nice colors, but %s.",
                          Deaf ? "funky smell"
                               : "the sound could have been more mellow");
                else
                    pline("There is a puff of smoke and a figure appears!");
            }
            if (!Deaf) {
                pline("%s says:", Blind ? "Someone" : Monnam(mtmp));
                verbalize("Did you think that I would %s %s %s?",
                          rn2(2) ? "relinquish"
                                 : (rn2(2) ? "hand over" : "give you"),
                          aname, rn2(2) ? "so easily" : "without a fight");
            }
            (void) mpickobj(mtmp, otmp);
            if (otmp2)
                (void) mpickobj(mtmp, otmp2);
            otmp = (struct obj *) &zeroobj;
            if (is_mplayer(mtmp->data))
                mtmp->m_lev = rn1(8, 23);
            else
                mtmp->m_lev = rn1(3, 20);
            mtmp->mpeaceful = mtmp->msleeping = 0;
            newsym(mtmp->mx, mtmp->my);
            m_dowear(mtmp, TRUE);
            mtmp->weapon_check = strategy;
            mon_wield_item(mtmp);
            return otmp;
        }
    }

    if (material > 0 && !otmp->oartifact
        && ((wizard && !iflags.debug_fuzzer)
            || valid_obj_material(otmp, material))) {
        if (!valid_obj_material(otmp, material)) {
            pline("Note: material %s is not normally valid for this object.",
                  materialnm[material]);
        }
        set_material(otmp, material);
    } else if (otmp->oartifact) {
        /* oname() handles the assignment of a specific material for any
         * possible artifact. Do nothing here. */
    } else {
        /* for now, material in wishes will always be base; this is to prevent
         * problems like wishing for arrows and getting glass arrows which will
         * shatter. */
        set_material(otmp, objects[otmp->otyp].oc_material);
    }

    /* set eroded and erodeproof */
    if (erosion_matters(otmp)) {
        /* wished-for item shouldn't be eroded unless specified */
        otmp->oeroded = otmp->oeroded2 = 0;
        if (eroded && (is_flammable(otmp) || is_rustprone(otmp)))
            otmp->oeroded = eroded;
        if (eroded2 && (is_corrodeable(otmp) || is_rottable(otmp)
                        || is_glass(otmp) || is_supermaterial(otmp)))
            otmp->oeroded2 = eroded2;
        /*
         * 3.6.1: earlier versions included `&& !eroded && !eroded2' here,
         * but damageproof combined with damaged is feasible (eroded
         * armor modified by confused reading of cursed destroy armor)
         * so don't prevent player from wishing for such a combination.
         *
         * Note on glass objects: this cannot be used to wish for shatterproof
         * non-base-glass objects like daggers, but it can be used to e.g. get a
         * shatterproof crystal plate mail.
         */
        if (erodeproof && (is_damageable(otmp) || otmp->otyp == CRYSKNIFE
                           || objects[otmp->otyp].oc_material == GLASS))
            maybe_erodeproof(otmp, (Luck >= 0 || wizard));
    }

    /* set quality */
    if (otmp->forged_qual) {
        /* only armor and weapons */
        if (!(otmp->oclass == ARMOR_CLASS
              || otmp->oclass == WEAPON_CLASS || is_barding(otmp)))
            return 0;
        /* part of a monster's body and produced when it dies */
        if (otmp->otyp == WORM_TOOTH || otmp->otyp == UNICORN_HORN)
            return 0;
        /* artifacts cannot be generated with a quality bit */
        if (otmp->oartifact)
            return 0;
        /* neither can magic items */
        if (objects[otmp->otyp].oc_magic)
            return 0;
        /* only objects that can be forged */
        if (!(is_metallic(otmp) || is_crystal(otmp)))
            return 0;
    }

    /* object property restrictions */
    if (otmp->oclass == WEAPON_CLASS || is_weptool(otmp)
        || is_bullet(otmp) || is_barding(otmp)
        || otmp->oclass == ARMOR_CLASS) {
        if (objprops & ITEM_FROST)
            objprops &= ~(ITEM_FIRE | ITEM_DRLI
                          | ITEM_SHOCK | ITEM_VENOM);
        else if (objprops & ITEM_FIRE)
            objprops &= ~(ITEM_FROST | ITEM_DRLI
                          | ITEM_SHOCK | ITEM_VENOM);
        else if (objprops & ITEM_DRLI)
            objprops &= ~(ITEM_FIRE | ITEM_FROST
                          | ITEM_SHOCK | ITEM_VENOM);
        else if (objprops & ITEM_SHOCK)
            objprops &= ~(ITEM_FIRE | ITEM_FROST
                          | ITEM_DRLI | ITEM_VENOM);
        else if (objprops & ITEM_VENOM)
            objprops &= ~(ITEM_FIRE | ITEM_FROST
                          | ITEM_DRLI | ITEM_SHOCK);

        if (objects[otmp->otyp].oc_unique || otmp->oartifact
            || Is_dragon_armor(otmp))
            objprops &= ~ITEM_PROP_MASK;

        if (objects[otmp->otyp].oc_magic && !wizard)
            objprops &= ~ITEM_PROP_MASK;

        if (otmp->oclass == WEAPON_CLASS || is_weptool(otmp))
            objprops &= ~(ITEM_DRLI | ITEM_FUMBLING | ITEM_HUNGER
                          | ITEM_OILSKIN);

        if (is_launcher(otmp))
            objprops &= ~(ITEM_FIRE | ITEM_FROST | ITEM_DRLI
                          | ITEM_SHOCK | ITEM_VENOM | ITEM_OILSKIN);

        if (is_ammo(otmp) || is_missile(otmp) || is_bullet(otmp))
            objprops &= ~(ITEM_DRLI | ITEM_OILSKIN | ITEM_ESP
                          | ITEM_SEARCHING | ITEM_WARNING | ITEM_FUMBLING
                          | ITEM_HUNGER | ITEM_EXCEL);

        if (otmp->material != CLOTH)
            objprops &= ~ITEM_OILSKIN;

        if (otmp->otyp == OILSKIN_CLOAK)
            objprops &= ~ITEM_OILSKIN;

        if (is_helmet(otmp))
            objprops &= ~ITEM_ESP;

        if (is_barding(otmp))
            objprops &= ~(ITEM_OILSKIN | ITEM_EXCEL
                          | ITEM_SEARCHING | ITEM_WARNING
                          | ITEM_FUMBLING | ITEM_HUNGER);

        otmp->oprops |= objprops;
    }

    if (halfeaten && otmp->oclass == FOOD_CLASS) {
        if (otmp->otyp == CORPSE)
            otmp->oeaten = mons[otmp->corpsenm].cnutrit;
        else
            otmp->oeaten = objects[otmp->otyp].oc_nutrition;
        /* (do this adjustment before setting up object's weight) */
        consume_oeaten(otmp, 1);
    }
    if (isdrained && otmp->otyp == CORPSE
        && mons[otmp->corpsenm].cnutrit) {
        int amt;

        otmp->odrained = 1;
        amt = mons[otmp->corpsenm].cnutrit - drain_level(otmp);
        if (halfdrained) {
            amt /= 2;
            if (amt == 0)
                amt++;
        }
        /* (do this adjustment before setting up object's weight) */
        consume_oeaten(otmp, -amt);
    }
    otmp->owt = weight(otmp);
    if (very && otmp->otyp == HEAVY_IRON_BALL)
        otmp->owt += IRON_BALL_W_INCR;
    else if (gsize > 1 && otmp->globby)
        /* 0: unspecified => small; 1: small => keep default owt of 20;
           2: medium => 120; 3: large => 320; 4: very large => 520 */
        otmp->owt += 100 + (gsize - 2) * 200;

    return otmp;
}

int
rnd_class(first, last)
int first, last;
{
    int i, x, sum = 0;

    if (first == last)
        return first;
    for (i = first; i <= last; i++)
        sum += objects[i].oc_prob;
    if (!sum) /* all zero */
        return first + rn2(last - first + 1);
    x = rnd(sum);
    for (i = first; i <= last; i++)
        if (objects[i].oc_prob && (x -= objects[i].oc_prob) <= 0)
            return i;
    return 0;
}

STATIC_OVL const char *
Japanese_item_name(i)
int i;
{
    struct Jitem *j = Japanese_items;

    while (j->item) {
        if (i == j->item)
            return j->name;
        j++;
    }
    return (const char *) 0;
}

const char *
suit_simple_name(suit)
struct obj *suit;
{
    const char *suitnm, *esuitp;

    if (suit) {
        suitnm = OBJ_NAME(objects[suit->otyp]);
        esuitp = eos((char *) suitnm);
        if (strlen(suitnm) > 5 && !strcmp(esuitp - 5, " mail"))
            return "mail"; /* most suits fall into this category */
        else if (strlen(suitnm) > 7 && !strcmp(esuitp - 7, " jacket"))
            return "jacket"; /* leather jacket */
    }
    /* "suit" is lame but "armor" is ambiguous and "body armor" is absurd */
    return "suit";
}

const char *
cloak_simple_name(cloak)
struct obj *cloak;
{
    if (cloak) {
        if (Is_dragon_scales(cloak))
            return "set of dragon scales";
        switch (cloak->otyp) {
        case ROBE:
            return "robe";
        case MUMMY_WRAPPING:
            return "wrapping";
        case ALCHEMY_SMOCK:
            return (objects[cloak->otyp].oc_name_known && cloak->dknown)
                       ? "smock"
                       : "apron";
        default:
            break;
        }
    }
    return "cloak";
}

/* helm vs hat for messages */
const char *
helm_simple_name(helmet)
struct obj *helmet;
{
    /*
     *  There is some wiggle room here; the result has been chosen
     *  for consistency with the "protected by hard helmet" messages
     *  given for various bonks on the head:  headgear that provides
     *  such protection is a "helm", that which doesn't is a "hat".
     *
     *      elven helm / hat                    -> hat
     *      dwarvish helm / hard hat            -> helm
     *  The rest are completely straightforward:
     *      fedora, cornuthaum, dunce cap       -> hat
     *      all other types of helmets          -> helm
     */
    return (helmet && !is_hard(helmet)) ? "hat" : "helm";
}

/* gloves vs gauntlets; depends upon discovery state */
const char *
gloves_simple_name(gloves)
struct obj *gloves;
{
    static const char gauntlets[] = "gauntlets";

    if (gloves && gloves->dknown) {
        int otyp = gloves->otyp;
        struct objclass *ocl = &objects[otyp];
        const char *actualn = OBJ_NAME(*ocl),
                   *descrpn = OBJ_DESCR(*ocl);

        if (strstri(objects[otyp].oc_name_known ? actualn : descrpn,
                    gauntlets))
            return gauntlets;
    }
    return "gloves";
}

/* Return the dragon color associated with a piece of dragon armor.
 * E.g. red-scaled chain mail => "red"; blue dragon scales => "blue". */
char *
dragon_scales_color(obj)
struct obj *obj;
{
    char *buf = nextobuf();
    const struct permonst *pm;
    const char* endp;
    int colorlen;

    if (!obj) {
        impossible("dragon_scales_color: null obj");
        return NULL;
    }
    if (!Is_dragon_armor(obj)) {
        impossible("getting color of non-dragon-armor (otyp=%d, scales=%d)",
                   obj->otyp, obj->dragonscales);
        Strcpy(buf, "bugged color");
        return buf;
    }
    pm = &mons[Dragon_armor_to_pm(obj)];
    endp = strstri(pm->mname, " dragon");
    if (!endp) {
        impossible("dragon_scales_color found non-dragon monster (%s)",
                   pm->mname);
        Strcpy(buf, "bugged color");
        return buf;
    }
    colorlen = (int) (endp - pm->mname);
    strncpy(buf, pm->mname, colorlen);
    buf[colorlen] = '\0';
    return buf;
}

const char *
mimic_obj_name(mtmp)
struct monst *mtmp;
{
    if (M_AP_TYPE(mtmp) == M_AP_OBJECT) {
        if (mtmp->mappearance == GOLD_PIECE)
            return "gold";
        if (mtmp->mappearance != STRANGE_OBJECT)
            return simple_typename(mtmp->mappearance);
    }
    return "whatcha-may-callit";
}

/*
 * Construct a query prompt string, based around an object name, which is
 * guaranteed to fit within [QBUFSZ].  Takes an optional prefix, three
 * choices for filling in the middle (two object formatting functions and a
 * last resort literal which should be very short), and an optional suffix.
 */
char *
safe_qbuf(qbuf, qprefix, qsuffix, obj, func, altfunc, lastR)
char *qbuf; /* output buffer */
const char *qprefix, *qsuffix;
struct obj *obj;
char *FDECL((*func), (OBJ_P)), *FDECL((*altfunc), (OBJ_P));
const char *lastR;
{
    char *bufp, *endp;
    /* convert size_t (or int for ancient systems) to ordinary unsigned */
    unsigned len, lenlimit,
        len_qpfx = (unsigned) (qprefix ? strlen(qprefix) : 0),
        len_qsfx = (unsigned) (qsuffix ? strlen(qsuffix) : 0),
        len_lastR = (unsigned) strlen(lastR);

    lenlimit = QBUFSZ - 1;
    endp = qbuf + lenlimit;
    /* sanity check, aimed mainly at paniclog (it's conceivable for
       the result of short_oname() to be shorter than the length of
       the last resort string, but we ignore that possibility here) */
    if (len_qpfx > lenlimit)
        impossible("safe_qbuf: prefix too long (%u characters).", len_qpfx);
    else if (len_qpfx + len_qsfx > lenlimit)
        impossible("safe_qbuf: suffix too long (%u + %u characters).",
                   len_qpfx, len_qsfx);
    else if (len_qpfx + len_lastR + len_qsfx > lenlimit)
        impossible("safe_qbuf: filler too long (%u + %u + %u characters).",
                   len_qpfx, len_lastR, len_qsfx);

    /* the output buffer might be the same as the prefix if caller
       has already partially filled it */
    if (qbuf == qprefix) {
        /* prefix is already in the buffer */
        *endp = '\0';
    } else if (qprefix) {
        /* put prefix into the buffer */
        (void) strncpy(qbuf, qprefix, lenlimit);
        *endp = '\0';
    } else {
        /* no prefix; output buffer starts out empty */
        qbuf[0] = '\0';
    }
    len = (unsigned) strlen(qbuf);

    if (len + len_lastR + len_qsfx > lenlimit) {
        /* too long; skip formatting, last resort output is truncated */
        if (len < lenlimit) {
            (void) strncpy(&qbuf[len], lastR, lenlimit - len);
            *endp = '\0';
            len = (unsigned) strlen(qbuf);
            if (qsuffix && len < lenlimit) {
                (void) strncpy(&qbuf[len], qsuffix, lenlimit - len);
                *endp = '\0';
                /* len = (unsigned) strlen(qbuf); */
            }
        }
    } else {
        /* suffix and last resort are guaranteed to fit */
        len += len_qsfx; /* include the pending suffix */
        /* format the object */
        bufp = short_oname(obj, func, altfunc, lenlimit - len);
        if (len + strlen(bufp) <= lenlimit)
            Strcat(qbuf, bufp); /* formatted name fits */
        else
            Strcat(qbuf, lastR); /* use last resort */
        releaseobuf(bufp);

        if (qsuffix)
            Strcat(qbuf, qsuffix);
    }
    /* assert( strlen(qbuf) < QBUFSZ ); */
    return qbuf;
}

STATIC_OVL char *
globwt(otmp, buf, weightformatted_p)
struct obj *otmp;
char *buf;
boolean *weightformatted_p;
{
    *buf = '\0';
    if (otmp->globby) {
        Sprintf(buf, "%u aum, ", otmp->owt);
        *weightformatted_p = TRUE;
    } else {
        *weightformatted_p = FALSE;
    }
    return buf;
}

/*objnam.c*/
