/* NetHack 3.6	mklev.c	$NHDT-Date: 1562455089 2019/07/06 23:18:09 $  $NHDT-Branch: NetHack-3.6 $:$NHDT-Revision: 1.63 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Alex Smith, 2017. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "dlb.h"

/* for UNIX, Rand #def'd to (long)lrand48() or (long)random() */
/* croom->lx etc are schar (width <= int), so % arith ensures that */
/* conversion of result to int is reasonable */

STATIC_DCL void FDECL(mkfount, (int, struct mkroom *));
STATIC_DCL void FDECL(mkforge, (int, struct mkroom *));
STATIC_DCL void FDECL(mksink, (struct mkroom *));
STATIC_DCL void FDECL(mkaltar, (struct mkroom *));
STATIC_DCL void FDECL(mkgrave, (struct mkroom *));
STATIC_DCL void NDECL(makevtele);
STATIC_DCL void NDECL(clear_level_structures);
STATIC_DCL void NDECL(makelevel);
STATIC_DCL struct mkroom *FDECL(find_branch_room, (coord *));
STATIC_DCL struct mkroom *FDECL(pos_to_room, (XCHAR_P, XCHAR_P));
STATIC_DCL boolean FDECL(place_niche, (struct mkroom *, int *, int *, int *));
STATIC_DCL void FDECL(makeniche, (int));
STATIC_DCL void NDECL(make_niches);
STATIC_PTR int FDECL(CFDECLSPEC do_comp, (const genericptr,
                                          const genericptr));
STATIC_DCL void FDECL(dosdoor, (XCHAR_P, XCHAR_P, struct mkroom *, int));
STATIC_DCL void FDECL(join, (int, int, BOOLEAN_P));
STATIC_DCL void FDECL(do_room_or_subroom, (struct mkroom *, int, int,
                                           int, int, BOOLEAN_P,
                                           SCHAR_P, BOOLEAN_P, BOOLEAN_P));
STATIC_DCL void NDECL(makerooms);
STATIC_DCL void FDECL(finddpos, (coord *, XCHAR_P, XCHAR_P,
                                 XCHAR_P, XCHAR_P));
STATIC_DCL void FDECL(mkinvpos, (XCHAR_P, XCHAR_P, int));
STATIC_DCL void FDECL(mk_knox_portal, (XCHAR_P, XCHAR_P));

#define create_vault() create_room(-1, -1, 2, 2, -1, -1, VAULT, TRUE)
#define init_vault() vault_x = -1
#define do_vault() (vault_x != -1)
static xchar vault_x, vault_y;
static boolean made_branch; /* used only during level creation */

/* Args must be (const genericptr) so that qsort will always be happy. */

STATIC_PTR int CFDECLSPEC
do_comp(vx, vy)
const genericptr vx;
const genericptr vy;
{
#ifdef LINT
    /* lint complains about possible pointer alignment problems, but we know
       that vx and vy are always properly aligned. Hence, the following
       bogus definition:
    */
    return (vx == vy) ? 0 : -1;
#else
    register const struct mkroom *x, *y;

    x = (const struct mkroom *) vx;
    y = (const struct mkroom *) vy;
    if (x->lx < y->lx)
        return -1;
    return (x->lx > y->lx);
#endif /* LINT */
}

STATIC_OVL void
finddpos(cc, xl, yl, xh, yh)
coord *cc;
xchar xl, yl, xh, yh;
{
    register xchar x, y;

    x = rn1(xh - xl + 1, xl);
    y = rn1(yh - yl + 1, yl);
    if (okdoor(x, y))
        goto gotit;

    for (x = xl; x <= xh; x++)
        for (y = yl; y <= yh; y++)
            if (okdoor(x, y))
                goto gotit;

    for (x = xl; x <= xh; x++)
        for (y = yl; y <= yh; y++)
            if (IS_DOOR(levl[x][y].typ) || levl[x][y].typ == SDOOR)
                goto gotit;
    /* cannot find something reasonable -- strange */
    x = xl;
    y = yh;
 gotit:
    cc->x = x;
    cc->y = y;
    return;
}

void
sort_rooms()
{
#if defined(SYSV) || defined(DGUX)
#define CAST_nroom (unsigned) nroom
#else
#define CAST_nroom nroom /*as-is*/
#endif
    qsort((genericptr_t) rooms, CAST_nroom, sizeof (struct mkroom), do_comp);
#undef CAST_nroom
}

STATIC_OVL void
do_room_or_subroom(croom, lowx, lowy, hix, hiy, lit, rtype, special, is_room)
register struct mkroom *croom;
int lowx, lowy;
register int hix, hiy;
boolean lit;
schar rtype;
boolean special;
boolean is_room;
{
    register int x, y;
    struct rm *lev;

    /* locations might bump level edges in wall-less rooms */
    /* add/subtract 1 to allow for edge locations */
    if (!lowx)
        lowx++;
    if (!lowy)
        lowy++;
    if (hix >= COLNO - 1)
        hix = COLNO - 2;
    if (hiy >= ROWNO - 1)
        hiy = ROWNO - 2;

    if (lit) {
        for (x = lowx - 1; x <= hix + 1; x++) {
            lev = &levl[x][max(lowy - 1, 0)];
            for (y = lowy - 1; y <= hiy + 1; y++)
                lev++->lit = 1;
        }
        croom->rlit = 1;
    } else
        croom->rlit = 0;

    croom->lx = lowx;
    croom->hx = hix;
    croom->ly = lowy;
    croom->hy = hiy;
    croom->rtype = rtype;
    croom->doorct = 0;
    /* if we're not making a vault, doorindex will still be 0
     * if we are, we'll have problems adding niches to the previous room
     * unless fdoor is at least doorindex
     */
    croom->fdoor = doorindex;
    croom->irregular = FALSE;

    croom->nsubrooms = 0;
    croom->sbrooms[0] = (struct mkroom *) 0;
    if (!special) {
        for (x = lowx - 1; x <= hix + 1; x++)
            for (y = lowy - 1; y <= hiy + 1; y += (hiy - lowy + 2)) {
                levl[x][y].typ = HWALL;
                levl[x][y].horizontal = 1; /* For open/secret doors. */
            }
        for (x = lowx - 1; x <= hix + 1; x += (hix - lowx + 2))
            for (y = lowy; y <= hiy; y++) {
                levl[x][y].typ = VWALL;
                levl[x][y].horizontal = 0; /* For open/secret doors. */
            }
        for (x = lowx; x <= hix; x++) {
            lev = &levl[x][lowy];
            for (y = lowy; y <= hiy; y++)
                lev++->typ = ROOM;
        }
        if (is_room) {
            levl[lowx - 1][lowy - 1].typ = TLCORNER;
            levl[hix + 1][lowy - 1].typ = TRCORNER;
            levl[lowx - 1][hiy + 1].typ = BLCORNER;
            levl[hix + 1][hiy + 1].typ = BRCORNER;
            wallification(lowx - 1, lowy - 1, hix + 1, hiy + 1);
        } else { /* a subroom */
            wallification(lowx - 1, lowy - 1, hix + 1, hiy + 1);
        }
    }
}

void
add_room(lowx, lowy, hix, hiy, lit, rtype, special)
int lowx, lowy, hix, hiy;
boolean lit;
schar rtype;
boolean special;
{
    register struct mkroom *croom;

    croom = &rooms[nroom];
    do_room_or_subroom(croom, lowx, lowy, hix, hiy, lit, rtype, special,
                       (boolean) TRUE);
    croom++;
    croom->hx = -1;
    nroom++;
}

void
add_subroom(proom, lowx, lowy, hix, hiy, lit, rtype, special)
struct mkroom *proom;
int lowx, lowy, hix, hiy;
boolean lit;
schar rtype;
boolean special;
{
    register struct mkroom *croom;

    croom = &subrooms[nsubroom];
    do_room_or_subroom(croom, lowx, lowy, hix, hiy, lit, rtype, special,
                       (boolean) FALSE);
    proom->sbrooms[proom->nsubrooms++] = croom;
    croom++;
    croom->hx = -1;
    nsubroom++;
}

struct _rndvault {
    char *fname;
    long freq;
    long mindepth;
    struct _rndvault *next;
};

struct _rndvault_gen {
    int n_vaults;
    long total_freq;
    struct _rndvault *vaults;
};

struct _rndvault_gen *rndvault_gen = NULL;

void
rndvault_gen_load()
{
    if (!rndvault_gen) {
	dlb *fd;
	char line[BUFSZ];
	char fnamebuf[64];
	long frq, mind;
	fd = dlb_fopen("vaults.dat", "r");
        if (!fd)
            return;

	rndvault_gen = (struct _rndvault_gen *) alloc(sizeof(struct _rndvault_gen));
	if (!rndvault_gen)
            goto bailout;

	rndvault_gen->n_vaults = 0;
	rndvault_gen->total_freq = 0;
	rndvault_gen->vaults = NULL;

	while (dlb_fgets(line, sizeof line, fd)) {
	    struct _rndvault *vlt = (struct _rndvault *) alloc(sizeof(struct _rndvault));
            char *tmpch = fnamebuf;
	    fnamebuf[0] = '\0';
	    if (sscanf(line, "%ld %ld %63s", &mind, &frq, tmpch) == 3) {
		if (frq < 1)
                    frq = 1;
		vlt->freq = frq;
                vlt->mindepth = mind;
		vlt->fname = strdup(fnamebuf);
		vlt->next = rndvault_gen->vaults;
		rndvault_gen->vaults = vlt;
		rndvault_gen->n_vaults++;
		rndvault_gen->total_freq += frq;
	    }
	}

    bailout:
        (void)dlb_fclose(fd);
    }
}

long curr_vault_depth = -1;
long curr_total_freq = -1;

char *
rndvault_getname()
{
    if (!rndvault_gen) rndvault_gen_load();
    if (rndvault_gen) {
        struct _rndvault *tmp = rndvault_gen->vaults;
        long cdepth = depth(&u.uz);
        long frq;
        if (curr_total_freq == -1 || curr_vault_depth != cdepth) {
            curr_total_freq = 0;
            while (tmp) {
                if (cdepth >= tmp->mindepth)
                    curr_total_freq += tmp->freq;
                tmp = tmp->next;
            }
            curr_vault_depth = cdepth;
        }
        frq = rn2(curr_total_freq);
        tmp = rndvault_gen->vaults;
        while (tmp) {
            if (cdepth >= tmp->mindepth)
                frq -= tmp->freq;
            if (frq <= 0)
                return tmp->fname ? tmp->fname : NULL;
            tmp = tmp->next;
        }

    }
    return NULL;
}

STATIC_OVL void
makerooms()
{
    boolean tried_vault = FALSE;

    /* make rooms until satisfied */
    /* rnd_rect() will returns 0 if no more rects are available... */
    while (nroom < MAXNROFROOMS && rnd_rect()) {
        if (nroom >= (MAXNROFROOMS / 6) && rn2(2) && !tried_vault) {
            tried_vault = TRUE;
            if (create_vault()) {
                vault_x = rooms[nroom].lx;
                vault_y = rooms[nroom].ly;
                rooms[nroom].hx = -1;
            }
        } else {
            char protofile[64];
	    char *fnam = rndvault_getname();
	    if (fnam) {
		Sprintf(protofile, "%s", fnam);
		Strcat(protofile, LEV_EXT);
		in_mk_rndvault = TRUE;
		rndvault_failed = FALSE;
		(void) load_special(protofile);
		in_mk_rndvault = FALSE;
		if (rndvault_failed)
                    return;
	    } else {
		if (!create_room(-1, -1, -1, -1, -1, -1, OROOM, -1))
		    return;
	    }
	}
    }
    return;
}

STATIC_OVL void
join(a, b, nxcor)
register int a, b;
boolean nxcor;
{
    coord cc, tt, org, dest;
    register xchar tx, ty, xx, yy;
    register struct mkroom *croom, *troom;
    register int dx, dy;

    croom = &rooms[a];
    troom = &rooms[b];

    /* find positions cc and tt for doors in croom and troom
       and direction for a corridor between them */

    if (troom->hx < 0 || croom->hx < 0 || doorindex >= DOORMAX)
        return;
    if (troom->lx > croom->hx) {
        dx = 1;
        dy = 0;
        xx = croom->hx + 1;
        tx = troom->lx - 1;
        finddpos(&cc, xx, croom->ly, xx, croom->hy);
        finddpos(&tt, tx, troom->ly, tx, troom->hy);
    } else if (troom->hy < croom->ly) {
        dy = -1;
        dx = 0;
        yy = croom->ly - 1;
        finddpos(&cc, croom->lx, yy, croom->hx, yy);
        ty = troom->hy + 1;
        finddpos(&tt, troom->lx, ty, troom->hx, ty);
    } else if (troom->hx < croom->lx) {
        dx = -1;
        dy = 0;
        xx = croom->lx - 1;
        tx = troom->hx + 1;
        finddpos(&cc, xx, croom->ly, xx, croom->hy);
        finddpos(&tt, tx, troom->ly, tx, troom->hy);
    } else {
        dy = 1;
        dx = 0;
        yy = croom->hy + 1;
        ty = troom->ly - 1;
        finddpos(&cc, croom->lx, yy, croom->hx, yy);
        finddpos(&tt, troom->lx, ty, troom->hx, ty);
    }
    xx = cc.x;
    yy = cc.y;
    tx = tt.x - dx;
    ty = tt.y - dy;
    if (nxcor && levl[xx + dx][yy + dy].typ)
        return;
    if (okdoor(xx, yy) || !nxcor)
        dodoor(xx, yy, croom);

    org.x = xx + dx;
    org.y = yy + dy;
    dest.x = tx;
    dest.y = ty;

    if (!dig_corridor(&org, &dest, nxcor, level.flags.arboreal ? ROOM : CORR,
                      STONE))
        return;

    /* we succeeded in digging the corridor */
    if (okdoor(tt.x, tt.y) || !nxcor)
        dodoor(tt.x, tt.y, troom);

    if (smeq[a] < smeq[b])
        smeq[b] = smeq[a];
    else
        smeq[a] = smeq[b];
}

void
makecorridors()
{
    int a, b, i;
    boolean any = TRUE;

    for (a = 0; a < nroom - 1; a++) {
        join(a, a + 1, FALSE);
        if (!rn2(50))
            break; /* allow some randomness */
    }
    for (a = 0; a < nroom - 2; a++)
        if (smeq[a] != smeq[a + 2])
            join(a, a + 2, FALSE);
    for (a = 0; any && a < nroom; a++) {
        any = FALSE;
        for (b = 0; b < nroom; b++)
            if (smeq[a] != smeq[b]) {
                join(a, b, FALSE);
                any = TRUE;
            }
    }
    if (nroom > 2)
        for (i = rn2(nroom) + 4; i; i--) {
            a = rn2(nroom);
            b = rn2(nroom - 2);
            if (b >= a)
                b += 2;
            join(a, b, TRUE);
        }
}

void
add_door(x, y, aroom)
register int x, y;
register struct mkroom *aroom;
{
    register struct mkroom *broom;
    register int tmp;
    int i;

    if (aroom->doorct == 0)
        aroom->fdoor = doorindex;

    aroom->doorct++;

    for (tmp = doorindex; tmp > aroom->fdoor; tmp--)
        doors[tmp] = doors[tmp - 1];

    for (i = 0; i < nroom; i++) {
        broom = &rooms[i];
        if (broom != aroom && broom->doorct && broom->fdoor >= aroom->fdoor)
            broom->fdoor++;
    }
    for (i = 0; i < nsubroom; i++) {
        broom = &subrooms[i];
        if (broom != aroom && broom->doorct && broom->fdoor >= aroom->fdoor)
            broom->fdoor++;
    }

    doorindex++;
    doors[aroom->fdoor].x = x;
    doors[aroom->fdoor].y = y;
}

STATIC_OVL void
dosdoor(x, y, aroom, type)
register xchar x, y;
struct mkroom *aroom;
int type;
{
    boolean shdoor = *in_rooms(x, y, SHOPBASE) ? TRUE : FALSE;

    if (!IS_WALL(levl[x][y].typ)) /* avoid SDOORs on already made doors */
        type = DOOR;
    levl[x][y].typ = type;
    if (type == DOOR) {
        if (!rn2(3)) { /* is it a locked door, closed, or a doorway? */
            if (!rn2(5))
                levl[x][y].doormask = D_ISOPEN;
            else if (!rn2(6))
                levl[x][y].doormask = D_LOCKED;
            else
                levl[x][y].doormask = D_CLOSED;

            if (levl[x][y].doormask != D_ISOPEN && !shdoor
                && level_difficulty() >= 10 && !rn2(25))
                levl[x][y].doormask |= D_TRAPPED;
        } else {
#ifdef STUPID
            if (shdoor)
                levl[x][y].doormask = D_ISOPEN;
            else
                levl[x][y].doormask = D_NODOOR;
#else
            levl[x][y].doormask = (shdoor ? D_ISOPEN : D_NODOOR);
#endif
        }

        /* also done in roguecorr(); doing it here first prevents
           making mimics in place of trapped doors on rogue level */
        if (Is_rogue_level(&u.uz))
            levl[x][y].doormask = D_NODOOR;

        if (levl[x][y].doormask & D_TRAPPED) {
            struct monst *mtmp;

            if (level_difficulty() >= 9 && !rn2(5)
                && !((mvitals[PM_SMALL_MIMIC].mvflags & G_GONE)
                     && (mvitals[PM_LARGE_MIMIC].mvflags & G_GONE)
                     && (mvitals[PM_GIANT_MIMIC].mvflags & G_GONE))) {
                /* make a mimic instead */
                levl[x][y].doormask = D_NODOOR;
                mtmp = makemon(mkclass(S_MIMIC, 0), x, y, NO_MM_FLAGS);
                if (mtmp)
                    set_mimic_sym(mtmp);
            }
        }
        /* newsym(x,y); */
    } else { /* SDOOR */
        if (shdoor || !rn2(5))
            levl[x][y].doormask = D_LOCKED;
        else
            levl[x][y].doormask = D_CLOSED;

        if (!shdoor && level_difficulty() >= 10 && !rn2(20))
            levl[x][y].doormask |= D_TRAPPED;
    }

    add_door(x, y, aroom);
}

STATIC_OVL boolean
place_niche(aroom, dy, xx, yy)
register struct mkroom *aroom;
int *dy, *xx, *yy;
{
    coord dd;

    if (rn2(2)) {
        *dy = 1;
        finddpos(&dd, aroom->lx, aroom->hy + 1, aroom->hx, aroom->hy + 1);
    } else {
        *dy = -1;
        finddpos(&dd, aroom->lx, aroom->ly - 1, aroom->hx, aroom->ly - 1);
    }
    *xx = dd.x;
    *yy = dd.y;
    return (boolean) ((isok(*xx, *yy + *dy)
                       && levl[*xx][*yy + *dy].typ == STONE)
                      && (isok(*xx, *yy - *dy)
                          && !IS_POOL(levl[*xx][*yy - *dy].typ)
                          && !IS_FURNITURE(levl[*xx][*yy - *dy].typ)));
}

/* there should be one of these per trap, in the same order as trap.h */
static NEARDATA const char *trap_engravings[TRAPNUM] = {
    (char *) 0,      (char *) 0,    (char *) 0,    (char *) 0, (char *) 0,
    (char *) 0,      (char *) 0,    (char *) 0,    (char *) 0, (char *) 0,
    (char *) 0,      (char *) 0,    (char *) 0,    (char *) 0, (char *) 0,
    /* 16..18: trap door, teleport, level-teleport */
    (char *) 0, "Vlad was here", "ad aerarium", "ad aerarium", (char *) 0,
    (char *) 0,      (char *) 0,    (char *) 0,    (char *) 0, (char *) 0,
    (char *) 0,      (char *) 0,    (char *) 0,
};

STATIC_OVL void
makeniche(trap_type)
int trap_type;
{
    register struct mkroom *aroom;
    struct rm *rm;
    int vct = 8;
    int dy, xx, yy;
    struct trap *ttmp;

    if (doorindex < DOORMAX) {
        while (vct--) {
            aroom = &rooms[rn2(nroom)];
            if (aroom->rtype != OROOM)
                continue; /* not an ordinary room */
            if (aroom->doorct == 1 && rn2(5))
                continue;
            if (!place_niche(aroom, &dy, &xx, &yy))
                continue;

            rm = &levl[xx][yy + dy];
            if (trap_type || !rn2(4)) {
                rm->typ = SCORR;
                if (trap_type) {
                    if (is_hole(trap_type) && !Can_fall_thru(&u.uz))
                        trap_type = ROCKTRAP;
                    ttmp = maketrap(xx, yy + dy, trap_type);
                    if (ttmp) {
                        if (trap_type != ROCKTRAP)
                            ttmp->once = 1;
                        if (trap_engravings[trap_type]) {
                            make_engr_at(xx, yy - dy,
                                         trap_engravings[trap_type], 0L,
                                         DUST);
                            wipe_engr_at(xx, yy - dy, 5,
                                         FALSE); /* age it a little */
                        }
                    }
                }
                dosdoor(xx, yy, aroom, SDOOR);
            } else {
                rm->typ = CORR;
                if (rn2(7))
                    dosdoor(xx, yy, aroom, rn2(5) ? SDOOR : DOOR);
                else {
                    /* inaccessible niches occasionally have iron bars */
                    if (!rn2(5) && IS_WALL(levl[xx][yy].typ)) {
                        levl[xx][yy].typ = IRONBARS;
                        if (rn2(3))
                            (void) mkcorpstat(CORPSE, (struct monst *) 0,
                                              mkclass(S_HUMAN, 0), xx,
                                              yy + dy, TRUE);
                    }
                    if (!level.flags.noteleport)
                        (void) mksobj_at(SCR_TELEPORTATION, xx, yy + dy, TRUE,
                                         FALSE);
                    if (!rn2(3))
                        (void) mkobj_at(0, xx, yy + dy, TRUE);
                }
            }
            return;
        }
    }
}

/* replaces horiz/vert walls with iron bars,
   if there's no door next to the place, and
   there's space on the other side of the wall */
void
make_ironbarwalls(chance)
int chance;
{
    xchar x, y;

    if (chance < 1)
        return;

    for (x = 1; x < COLNO-1; x++) {
	for (y = 1; y < ROWNO-1; y++) {
	    schar typ = levl[x][y].typ;
	    if (typ == HWALL) {
		if ((IS_WALL(levl[x - 1][y].typ) || levl[x - 1][y].typ == IRONBARS)
		    && (IS_WALL(levl[x + 1][y].typ) || levl[x + 1][y].typ == IRONBARS)
		    && SPACE_POS(levl[x][y - 1].typ) && SPACE_POS(levl[x][y + 1].typ)
		    && rn2(100) < chance)
		    levl[x][y].typ = IRONBARS;
	    } else if (typ == VWALL) {
		if ((IS_WALL(levl[x][y - 1].typ) || levl[x][y - 1].typ == IRONBARS)
		    && (IS_WALL(levl[x][y + 1].typ) || levl[x][y + 1].typ == IRONBARS)
		    && SPACE_POS(levl[x - 1][y].typ) && SPACE_POS(levl[x + 1][y].typ)
		    && rn2(100) < chance)
		    levl[x][y].typ = IRONBARS;
	    }
	}
    }
}

STATIC_OVL void
make_niches()
{
    int ct = rnd((nroom >> 1) + 1), dep = depth(&u.uz);
    boolean ltptr = (!level.flags.noteleport && dep > 15),
            vamp = (dep > 5 && dep < 25);

    while (ct--) {
        if (ltptr && !rn2(6)) {
            ltptr = FALSE;
            makeniche(LEVEL_TELEP);
        } else if (vamp && !rn2(6)) {
            vamp = FALSE;
            makeniche(TRAPDOOR);
        } else
            makeniche(NO_TRAP);
    }
}

STATIC_OVL void
makevtele()
{
    makeniche(TELEP_TRAP_SET);
}

/* clear out various globals that keep information on the current level.
 * some of this is only necessary for some types of levels (maze, normal,
 * special) but it's easier to put it all in one place than make sure
 * each type initializes what it needs to separately.
 */
STATIC_OVL void
clear_level_structures()
{
    static struct rm zerorm = { cmap_to_glyph(S_stone),
                                0, 0, 0, 0, 0, 0, 0, 0, 0 };
    register int x, y;
    register struct rm *lev;

    /* note:  normally we'd start at x=1 because map column #0 isn't used
       (except for placing vault guard at <0,0> when removed from the map
       but not from the level); explicitly reset column #0 along with the
       rest so that we start the new level with a completely clean slate */
    for (x = 0; x < COLNO; x++) {
        lev = &levl[x][0];
        for (y = 0; y < ROWNO; y++) {
            *lev++ = zerorm;
            /*
             * These used to be '#if MICROPORT_BUG',
             * with use of memset(0) for '#if !MICROPORT_BUG' below,
             * but memset is not appropriate for initializing pointers,
             * so do these level.objects[][] and level.monsters[][]
             * initializations unconditionally.
             */
            level.objects[x][y] = (struct obj *) 0;
            level.monsters[x][y] = (struct monst *) 0;
        }
    }
    level.objlist = (struct obj *) 0;
    level.buriedobjlist = (struct obj *) 0;
    level.monlist = (struct monst *) 0;
    level.damagelist = (struct damage *) 0;
    level.bonesinfo = (struct cemetery *) 0;

    level.flags.nforges = 0;
    level.flags.nmagicchests = 0;
    level.flags.nfountains = 0;
    level.flags.nsinks = 0;
    level.flags.has_shop = 0;
    level.flags.has_vault = 0;
    level.flags.has_zoo = 0;
    level.flags.has_court = 0;
    level.flags.has_morgue = level.flags.graveyard = 0;
    level.flags.has_lemurepit = 0;
    level.flags.has_beehive = 0;
    level.flags.has_barracks = 0;
    level.flags.has_temple = 0;
    level.flags.has_swamp = 0;
    level.flags.has_garden = 0;
    level.flags.has_forest = 0;
    level.flags.noteleport = 0;
    level.flags.hardfloor = 0;
    level.flags.nommap = 0;
    level.flags.hero_memory = 1;
    level.flags.shortsighted = 0;
    level.flags.sokoban_rules = 0;
    level.flags.is_maze_lev = 0;
    level.flags.is_cavernous_lev = 0;
    level.flags.arboreal = 0;
    level.flags.wizard_bones = 0;
    level.flags.corrmaze = 0;

    nroom = 0;
    rooms[0].hx = -1;
    nsubroom = 0;
    subrooms[0].hx = -1;
    doorindex = 0;
    init_rect();
    init_vault();
    xdnstair = ydnstair = xupstair = yupstair = 0;
    sstairs.sx = sstairs.sy = 0;
    xdnladder = ydnladder = xupladder = yupladder = 0;
    dnstairs_room = upstairs_room = sstairs_room = (struct mkroom *) 0;
    made_branch = FALSE;
    clear_regions();
}

STATIC_OVL void
makelevel()
{
    register struct mkroom *croom, *troom;
    register int tryct;
    register int i;
    struct monst *tmonst; /* always put a web with a spider */
    branch *branchp;
    int room_threshold, boxtype;
    coord pos;

    if (wiz1_level.dlevel == 0)
        init_dungeons();
    oinit(); /* assign level dependent obj probabilities */
    clear_level_structures();

    {
        register s_level *slev = Is_special(&u.uz);

        /* check for special levels */
        if (slev && !Is_rogue_level(&u.uz)) {
            makemaz(slev->proto);
            return;
        } else if (dungeons[u.uz.dnum].proto[0]) {
            makemaz("");
            return;
        } else if (In_mines(&u.uz)) {
            makemaz("minefill");
            return;
        } else if (In_quest(&u.uz)) {
            char fillname[9];
            s_level *loc_lev;

            Sprintf(fillname, "%s-loca", urole.filecode);
            loc_lev = find_level(fillname);

            Sprintf(fillname, "%s-fil", urole.filecode);
            Strcat(fillname,
                   (u.uz.dlevel < loc_lev->dlevel.dlevel) ? "a" : "b");
            makemaz(fillname);
            return;
        } else if (u.uz.dnum == medusa_level.dnum
                   && depth(&u.uz) > depth(&medusa_level)) {
            /* TODO: currently, only create normal rooms and corridors
               between Medusa's level and the castle (this will fall
               through to makerooms() below). In the future, maybe
               create a special filler level type specifically for
               these levels, like what UnNetHack does */
            ;
        } else if (In_hell(&u.uz)) {
            /* The vibrating square code is hardcoded into mkmaze --
             * rather than fiddle around trying to port it to a 'generalist'
             * sort of level, just go ahead and let the VS level be a maze */
            if (!Invocation_lev(&u.uz)) {
                makemaz("hellfill");
            } else {
            /* TODO: the invocation level is the last guaranteed maze
               level in the game. In the future, maybe create a small
               number of special end maps unique to this level, with
               its own distinct challenges */
                makemaz("");
            }
            return;
        }
    }

    /* otherwise, fall through - it's a "regular" level. */

    if (Is_rogue_level(&u.uz)) {
        makeroguerooms();
        makerogueghost();
    } else
        makerooms();
    /* sort_rooms(); */ /* this screws with roomno order */

    /* construct stairs (up and down in different rooms if possible) */
    tryct = 0;
    do {
        croom = &rooms[rn2(nroom)];
    } while (!croom->needjoining && ++tryct < 500);
    if (!Is_botlevel(&u.uz)) {
	if (!somexyspace(croom, &pos, 0)) {
            if (!is_damp_terrain(pos.x, pos.y)) {
                pos.x = somex(croom);
                pos.y = somey(croom);
            }
	}
        mkstairs(pos.x, pos.y, 0, croom); /* down */
    }
    if (nroom > 1) {
        troom = croom;
        tryct = 0;
        do {
            croom = &rooms[rn2(nroom - 1)];
        } while ((!croom->needjoining || (croom == troom)) && ++tryct < 500);
    }

    if (u.uz.dlevel != 1) {
	if (!somexyspace(croom, &pos, 0)) {
            if (!somexy(croom, &pos)) {
                if (!is_damp_terrain(pos.x, pos.y)) {
                    pos.x = somex(croom);
                    pos.y = somey(croom);
                }
            }
        }
	mkstairs(pos.x, pos.y, 1, croom); /* up */
    }

    branchp = Is_branchlev(&u.uz);    /* possible dungeon branch */
    room_threshold = branchp ? 4 : 3; /* minimum number of rooms needed
                                         to allow a random special room */
    if (Is_rogue_level(&u.uz))
        goto skip0;
    makecorridors();
    make_niches();

    if (!rn2(5))
        make_ironbarwalls(rn2(20) ? rn2(20) : rn2(50));

    /* make a secret treasure vault, not connected to the rest */
    if (do_vault()) {
        xchar w, h;

        debugpline0("trying to make a vault...");
        w = 1;
        h = 1;
        if (check_room(&vault_x, &w, &vault_y, &h, TRUE)) {
 fill_vault:
            add_room(vault_x, vault_y, vault_x + w, vault_y + h,
                     TRUE, VAULT, FALSE);
            level.flags.has_vault = 1;
            ++room_threshold;
            fill_room(&rooms[nroom - 1], FALSE);
            mk_knox_portal(vault_x + w, vault_y + h);
            if (!level.flags.noteleport && !rn2(3))
                makevtele();
        } else if (rnd_rect() && create_vault()) {
            vault_x = rooms[nroom].lx;
            vault_y = rooms[nroom].ly;
            if (check_room(&vault_x, &w, &vault_y, &h, TRUE))
                goto fill_vault;
            else
                rooms[nroom].hx = -1;
        }
    }

    {
        register int u_depth = depth(&u.uz);

        if (wizard && nh_getenv("SHOPTYPE"))
            mkroom(SHOPBASE);
        else if (u_depth > 1 && u_depth < depth(&medusa_level)
                 && nroom >= room_threshold && rn2(u_depth) < 3)
            mkroom(SHOPBASE);
        else if (u_depth > 4 && !rn2(6))
            mkroom(COURT);
        else if (u_depth > 5 && !rn2(8)
                 && !(mvitals[PM_LEPRECHAUN].mvflags & G_GONE))
            mkroom(LEPREHALL);
        else if (u_depth > 6 && !rn2(7))
            mkroom(ZOO);
        else if (u_depth > 7 && !rn2(6))
            mkroom(GARDEN);
        else if (u_depth > 7 && !rn2(7)
                 && !(mvitals[PM_RUST_MONSTER].mvflags & G_GONE))
            mkroom(ARMORY);
        else if (u_depth > 8 && !rn2(5))
            mkroom(TEMPLE);
        else if (u_depth > 9 && !rn2(5)
                 && !(mvitals[PM_KILLER_BEE].mvflags & G_GONE))
            mkroom(BEEHIVE);
        else if (u_depth > 10 && !rn2(7)
                 && !(mvitals[PM_BABY_OWLBEAR].mvflags & G_GONE))
            mkroom(OWLBNEST);
        else if (u_depth > 11 && !rn2(6))
            mkroom(FOREST);
        else if (u_depth > 11 && !rn2(6))
            mkroom(MORGUE);
        else if (u_depth > 12 && !rn2(8) && antholemon())
            mkroom(ANTHOLE);
        else if (u_depth > 14 && !rn2(4)
                 && !(mvitals[PM_SOLDIER].mvflags & G_GONE))
            mkroom(BARRACKS);
        else if (u_depth > 15 && !rn2(6))
            mkroom(SWAMP);
        else if (u_depth > 15 && !rn2(8)
                 && !(mvitals[PM_COCKATRICE].mvflags & G_GONE))
            mkroom(COCKNEST);
        else if (u_depth > 16 && !rn2(6)
                 && !(mvitals[PM_MIND_FLAYER_LARVA].mvflags & G_GONE))
            mkroom(NURSERY);
    }

 skip0:
    /* Place multi-dungeon branch. */
    place_branch(branchp, 0, 0);

    /* for each room: put things inside */
    for (croom = rooms; croom->hx > 0; croom++) {
	if (croom->rtype != OROOM && croom->rtype != RNDVAULT)
            continue;
	if (!croom->needfill)
            continue;

        /* put a sleeping monster inside */
        /* Note: monster may be on the stairs. This cannot be
           avoided: maybe the player fell through a trap door
           while a monster was on the stairs. Conclusion:
           we have to check for monsters on the stairs anyway. */

        if (u.uhave.amulet || !rn2(3)) {
	    if (somexyspace(croom, &pos, 0)) {
                tmonst = makemon((struct permonst *) 0, pos.x, pos.y,
                                 MM_NOGRP | MM_MPLAYEROK);
                if (tmonst && tmonst->data == &mons[PM_GIANT_SPIDER]
                    && !occupied(pos.x, pos.y))
                    (void) maketrap(pos.x, pos.y, WEB);
            }
        }

        if (level.flags.has_beehive == 1) {
            if (!occupied(pos.x, pos.y) && rn2(5))
                (void) makemon(&mons[PM_HONEY_BADGER], pos.x, pos.y, NO_MM_FLAGS);
        }

        /* put traps and mimics inside */
        i = 8 - (level_difficulty() / 6);
        if (i <= 1)
            i = 2;
        while (!rn2(i))
            mktrap(0, 0, croom, (coord *) 0);
	if (!rn2(3)) {
	    if (somexyspace(croom, &pos, 0))
		(void) mkgold(0L, pos.x, pos.y);
	}
        if (Is_rogue_level(&u.uz))
            goto skip_nonrogue;
        /* greater chance of puddles if a water source is nearby */
        if (!rn2(10))
            mkfount(0, croom);
        if (!rn2(60))
            mksink(croom);
        if (!rn2(40))
            mkforge(0, croom);
        if (depth(&u.uz) > 2 && !rn2(60))
            mkaltar(croom);
        i = 80 - (depth(&u.uz) * 2);
        if (i < 2)
            i = 2;
        if (!rn2(i))
            mkgrave(croom);

        /* put statues inside */
	if (!rn2(20)) {
	    if (somexyspace(croom, &pos, 0))
		(void) mkcorpstat(STATUE, (struct monst *) 0,
				  (struct permonst *) 0,
				  pos.x, pos.y, CORPSTAT_INIT);
	}
        /* put box/chest/safe inside;
         *  40% chance for at least 1 box, regardless of number
         *  of rooms; about 5 - 7.5% for 2 boxes, least likely
         *  when few rooms; chance for 3 or more is negligible.
         *
         * A safe will only show up below level 15 since they're
         *  not unlockable.
         */
	if (!rn2(nroom * 5 / 2)) {
	    i = rn2(5);
            if (!i && depth(&u.uz) > 15) {
                boxtype = IRON_SAFE;
            } else if (!i && depth(&u.uz) > 10) {
                boxtype = CRYSTAL_CHEST;
	    } else if (i > 2) {
	        boxtype = CHEST;
	    } else {
	        boxtype = LARGE_BOX;
	    }
	    if (somexyspace(croom, &pos, 0))
		(void) mksobj_at(boxtype, pos.x, pos.y, TRUE, FALSE);
	}
        /* maybe make some graffiti */
        if (!rn2(27 + 3 * abs(depth(&u.uz)))) {
            char buf[BUFSZ];
            const char *mesg = random_engraving(buf);

            if (mesg) {
		if (somexyspace(croom, &pos, 1))
                    make_engr_at(pos.x, pos.y, mesg, 0L, MARK);
            }
        }

 skip_nonrogue:
        if (!rn2(3)) {
	    if (somexyspace(croom, &pos, 0))
		(void) mkobj_at(0, pos.x, pos.y, TRUE);
            tryct = 0;
            while (!rn2(5)) {
                if (++tryct > 100) {
                    impossible("tryct overflow4");
                    break;
                }
		if (somexyspace(croom, &pos, 0))
		    (void) mkobj_at(0, pos.x, pos.y, TRUE);
            }
        }
    }
}

/*
 *      Place deposits of minerals (gold and misc gems) in the stone
 *      surrounding the rooms on the map.
 *      Also place kelp in water.
 *      mineralize(-1, -1, -1, -1, FALSE); => "default" behaviour
 */
void
mineralize(kelp_pool, kelp_moat, goldprob, gemprob, skip_lvl_checks)
int kelp_pool, kelp_moat, goldprob, gemprob;
boolean skip_lvl_checks;
{
    s_level *sp;
    struct obj *otmp;
    int x, y, cnt;

    if (kelp_pool < 0)
        kelp_pool = 10;
    if (kelp_moat < 0)
        kelp_moat = 30;

    /* Place kelp, except on the plane of water */
    if (!skip_lvl_checks && In_endgame(&u.uz))
        return;
    for (x = 2; x < (COLNO - 2); x++)
        for (y = 1; y < (ROWNO - 1); y++)
            if ((kelp_pool && levl[x][y].typ == POOL && !rn2(kelp_pool))
                || (kelp_moat && levl[x][y].typ == MOAT && !rn2(kelp_moat)))
                (void) mksobj_at(KELP_FROND, x, y, TRUE, FALSE);

    /* determine if it is even allowed;
       almost all special levels are excluded */
    if (!skip_lvl_checks
        && (In_hell(&u.uz) || In_V_tower(&u.uz) || Is_rogue_level(&u.uz)
            || level.flags.arboreal
            || ((sp = Is_special(&u.uz)) != 0 && !Is_oracle_level(&u.uz)
                && (!In_mines(&u.uz) || sp->flags.town))))
        return;

    /* basic level-related probabilities */
    if (goldprob < 0)
        goldprob = 20 + depth(&u.uz) / 3;
    if (gemprob < 0)
        gemprob = goldprob / 4;

    /* mines have ***MORE*** goodies - otherwise why mine? */
    if (!skip_lvl_checks) {
        if (In_mines(&u.uz)) {
            goldprob *= 2;
            gemprob *= 3;
        } else if (In_quest(&u.uz)) {
            goldprob /= 4;
            gemprob /= 6;
        }
    }

    /*
     * Seed rock areas with gold and/or gems.
     * We use fairly low level object handling to avoid unnecessary
     * overhead from placing things in the floor chain prior to burial.
     */
    for (x = 2; x < (COLNO - 2); x++)
        for (y = 1; y < (ROWNO - 1); y++)
            if (levl[x][y + 1].typ != STONE) { /* <x,y> spot not eligible */
                y += 2; /* next two spots aren't eligible either */
            } else if (levl[x][y].typ != STONE) { /* this spot not eligible */
                y += 1; /* next spot isn't eligible either */
            } else if (!(levl[x][y].wall_info & W_NONDIGGABLE)
                       && levl[x][y - 1].typ == STONE
                       && levl[x + 1][y - 1].typ == STONE
                       && levl[x - 1][y - 1].typ == STONE
                       && levl[x + 1][y].typ == STONE
                       && levl[x - 1][y].typ == STONE
                       && levl[x + 1][y + 1].typ == STONE
                       && levl[x - 1][y + 1].typ == STONE) {
                if (rn2(1000) < goldprob) {
                    if ((otmp = mksobj(GOLD_PIECE, FALSE, FALSE)) != 0) {
                        otmp->ox = x, otmp->oy = y;
                        otmp->quan = 1L + rnd(goldprob * 3);
                        otmp->owt = weight(otmp);
                        if (!rn2(3))
                            add_to_buried(otmp);
                        else
                            place_object(otmp, x, y);
                    }
                }
                if (rn2(1000) < gemprob) {
                    for (cnt = rnd(2 + dunlev(&u.uz) / 3); cnt > 0; cnt--) {
                        if ((otmp = mkobj(GEM_CLASS, FALSE)) != 0) {
                            if (otmp->otyp == ROCK
                                || otmp->otyp == SLING_BULLET) {
                                dealloc_obj(otmp); /* discard it */
                            } else {
                                otmp->ox = x, otmp->oy = y;
                                if (!rn2(3))
                                    add_to_buried(otmp);
                                else
                                    place_object(otmp, x, y);
                            }
                        }
                    }
                }
            }
}

void
mklev()
{
    struct mkroom *croom;
    int ridx;

    reseed_random(rn2);
    reseed_random(rn2_on_display_rng);

    init_mapseen(&u.uz);
    if (getbones())
        return;

    in_mklev = TRUE;
    makelevel();
    bound_digging();
    mineralize(-1, -1, -1, -1, FALSE);
    in_mklev = FALSE;
    /* has_morgue gets cleared once morgue is entered; graveyard stays
       set (graveyard might already be set even when has_morgue is clear
       [see fixup_special()], so don't update it unconditionally) */
    if (level.flags.has_morgue)
        level.flags.graveyard = 1;
    if (!level.flags.is_maze_lev) {
        for (croom = &rooms[0]; croom != &rooms[nroom]; croom++)
#ifdef SPECIALIZATION
            topologize(croom, FALSE);
#else
            topologize(croom);
#endif
    }
    set_wall_state();
    /* for many room types, rooms[].rtype is zeroed once the room has been
       entered; rooms[].orig_rtype always retains original rtype value */
    for (ridx = 0; ridx < SIZE(rooms); ridx++)
        rooms[ridx].orig_rtype = rooms[ridx].rtype;

    /* something like this usually belongs in clear_level_structures()
       but these aren't saved and restored so might not retain their
       values for the life of the current level; reset them to default
       now so that they never do and no one will be tempted to introduce
       a new use of them for anything on this level */
    dnstairs_room = upstairs_room = sstairs_room = (struct mkroom *) 0;

    reseed_random(rn2);
    reseed_random(rn2_on_display_rng);
}

void
#ifdef SPECIALIZATION
topologize(croom, do_ordinary)
struct mkroom *croom;
boolean do_ordinary;
#else
topologize(croom)
struct mkroom *croom;
#endif
{
    register int x, y, roomno = (int) ((croom - rooms) + ROOMOFFSET);
    int lowx = croom->lx, lowy = croom->ly;
    int hix = croom->hx, hiy = croom->hy;
#ifdef SPECIALIZATION
    schar rtype = croom->rtype;
#endif
    int subindex, nsubrooms = croom->nsubrooms;

    /* skip the room if already done; i.e. a shop handled out of order */
    /* also skip if this is non-rectangular (it _must_ be done already) */
    if ((int) levl[lowx][lowy].roomno == roomno || croom->irregular)
        return;
#ifdef SPECIALIZATION
    if (Is_rogue_level(&u.uz))
        do_ordinary = TRUE; /* vision routine helper */
    if ((rtype != OROOM) || do_ordinary)
#endif
        {
        /* do innards first */
        for (x = lowx; x <= hix; x++)
            for (y = lowy; y <= hiy; y++)
#ifdef SPECIALIZATION
                if (rtype == OROOM)
                    levl[x][y].roomno = NO_ROOM;
                else
#endif
                    levl[x][y].roomno = roomno;
        /* top and bottom edges */
        for (x = lowx - 1; x <= hix + 1; x++)
            for (y = lowy - 1; y <= hiy + 1; y += (hiy - lowy + 2)) {
                levl[x][y].edge = 1;
                if (levl[x][y].roomno)
                    levl[x][y].roomno = SHARED;
                else
                    levl[x][y].roomno = roomno;
            }
        /* sides */
        for (x = lowx - 1; x <= hix + 1; x += (hix - lowx + 2))
            for (y = lowy; y <= hiy; y++) {
                levl[x][y].edge = 1;
                if (levl[x][y].roomno)
                    levl[x][y].roomno = SHARED;
                else
                    levl[x][y].roomno = roomno;
            }
    }
    /* subrooms */
    for (subindex = 0; subindex < nsubrooms; subindex++)
#ifdef SPECIALIZATION
        topologize(croom->sbrooms[subindex], (boolean) (rtype != OROOM));
#else
        topologize(croom->sbrooms[subindex]);
#endif
}

/* Find an unused room for a branch location. */
STATIC_OVL struct mkroom *
find_branch_room(mp)
coord *mp;
{
    struct mkroom *croom = 0;

    if (nroom == 0) {
        mazexy(mp); /* already verifies location */
    } else {
        /* not perfect - there may be only one stairway */
        if (nroom > 2) {
            int tryct = 0;

            do
                croom = &rooms[rn2(nroom)];
            while ((croom == dnstairs_room || croom == upstairs_room
                   || croom->rtype != OROOM) && (++tryct < 500));
        } else
            croom = &rooms[rn2(nroom)];

        if (!somexyspace(croom, mp, 2)) {
            /* if the chosen room really has no free tile,
               fallback to maze/corridor tile (such as the
               branch portal to Vecna's domain in Gehennom
               [no rooms at all], or if the level with the
               portal to the quest is somehow filled) */
            if (!somexyspace(croom, mp, 0)) {
                mazexy(mp);
                /* signal "no room" so caller knows it's a
                   corridor placement */
                croom = (struct mkroom *) 0;
            }
        }
    }
    /* sanity‐check: after both room‐and‐corridor attempts,
       mp must land on either a room floor or a corridor,
       otherwise call impossible */
    if (!isok(mp->x, mp->y)
        || (levl[mp->x][mp->y].typ != CORR
            && !IS_ROOM(levl[mp->x][mp->y].typ)))
        impossible("Can't place branch!");

    return croom;
}

/* Find the room for (x,y).  Return null if not in a room. */
STATIC_OVL struct mkroom *
pos_to_room(x, y)
xchar x, y;
{
    int i;
    struct mkroom *curr;

    for (curr = rooms, i = 0; i < nroom; curr++, i++)
        if (inside_room(curr, x, y))
            return curr;
    ;
    return (struct mkroom *) 0;
}

/* If given a branch, randomly place a special stair or portal. */
void
place_branch(br, x, y)
branch *br; /* branch to place */
xchar x, y; /* location */
{
    coord m;
    d_level *dest;
    boolean make_stairs;
    struct mkroom *br_room;

    /*
     * Return immediately if there is no branch to make or we have
     * already made one.  This routine can be called twice when
     * a special level is loaded that specifies an SSTAIR location
     * as a favored spot for a branch.
     */
    if (!br || made_branch)
        return;

    if (!x) { /* find random coordinates for branch */
        br_room = find_branch_room(&m);
        x = m.x;
        y = m.y;
    } else {
        br_room = pos_to_room(x, y);
    }

    if (on_level(&br->end1, &u.uz)) {
        /* we're on end1 */
        make_stairs = br->type != BR_NO_END1;
        dest = &br->end2;
    } else {
        /* we're on end2 */
        make_stairs = br->type != BR_NO_END2;
        dest = &br->end1;
    }

    if (br->type == BR_PORTAL) {
        if (!occupied(x, y))
            mkportal(x, y, dest->dnum, dest->dlevel);
    } else if (make_stairs) {
        sstairs.sx = x;
        sstairs.sy = y;
        sstairs.up =
            (char) on_level(&br->end1, &u.uz) ? br->end1_up : !br->end1_up;
        assign_level(&sstairs.tolev, dest);
        sstairs_room = br_room;

        levl[x][y].ladder = sstairs.up ? LA_UP : LA_DOWN;
        levl[x][y].typ = STAIRS;
    }
    /*
     * Set made_branch to TRUE even if we didn't make a stairwell (i.e.
     * make_stairs is false) since there is currently only one branch
     * per level, if we failed once, we're going to fail again on the
     * next call.
     */
    made_branch = TRUE;
}

boolean
bydoor(x, y)
register xchar x, y;
{
    register int typ;

    if (isok(x + 1, y)) {
        typ = levl[x + 1][y].typ;
        if (IS_DOOR(typ) || typ == SDOOR)
            return TRUE;
    }
    if (isok(x - 1, y)) {
        typ = levl[x - 1][y].typ;
        if (IS_DOOR(typ) || typ == SDOOR)
            return TRUE;
    }
    if (isok(x, y + 1)) {
        typ = levl[x][y + 1].typ;
        if (IS_DOOR(typ) || typ == SDOOR)
            return TRUE;
    }
    if (isok(x, y - 1)) {
        typ = levl[x][y - 1].typ;
        if (IS_DOOR(typ) || typ == SDOOR)
            return TRUE;
    }
    return FALSE;
}

/* see whether it is allowable to create a door at [x,y] */
int
okdoor(x, y)
xchar x, y;
{
    boolean near_door = bydoor(x, y);

    return ((levl[x][y].typ == HWALL || levl[x][y].typ == VWALL)
            && doorindex < DOORMAX && !near_door);
}

void
dodoor(x, y, aroom)
int x, y;
struct mkroom *aroom;
{
    if (doorindex >= DOORMAX) {
        impossible("DOORMAX exceeded?");
        return;
    }

    dosdoor(x, y, aroom, rn2(8) ? DOOR : SDOOR);
}

boolean
occupied(x, y)
register xchar x, y;
{
    return (boolean) (t_at(x, y) || IS_FURNITURE(levl[x][y].typ)
                      || is_lava(x, y) || is_pool(x, y)
                      || invocation_pos(x, y));
}

/* make a trap somewhere (in croom if mazeflag = 0 && !tm) */
/* if tm != null, make trap at that location */
void
mktrap(num, mazeflag, croom, tm)
int num, mazeflag;
struct mkroom *croom;
coord *tm;
{
    register int i, kind;
    struct trap *t;
    unsigned lvl = level_difficulty();
    coord m;

    /* no traps in pools */
    if (tm && is_pool(tm->x, tm->y))
        return;

    if (num > 0 && num < TRAPNUM) {
        kind = num;
    } else if (Is_rogue_level(&u.uz)) {
        switch (rn2(8)) {
        default:
            kind = BEAR_TRAP;
            break; /* 0 */
        case 1:
            kind = ARROW_TRAP_SET;
            break;
        case 2:
            kind = BOLT_TRAP_SET;
            break;
        case 3:
            kind = DART_TRAP_SET;
            break;
        case 4:
            kind = TRAPDOOR;
            break;
        case 5:
            kind = PIT;
            break;
        case 6:
            kind = SLP_GAS_TRAP_SET;
            break;
        case 7:
            kind = RUST_TRAP_SET;
            break;
        }
    } else if (Inhell && !Iniceq && !rn2(5)) {
        /* bias the frequency of fire traps in Gehennom */
        kind = FIRE_TRAP_SET;
    } else if (Iniceq && !rn2(3)) {
        /* same for ice traps in the Ice Queen branch */
        kind = ICE_TRAP_SET;
    } else {
        do {
            kind = rnd(TRAPNUM - 1);
            /* reject "too hard" traps */
            switch (kind) {
            case MAGIC_PORTAL:
            case VIBRATING_SQUARE:
                kind = NO_TRAP;
                break;
            case ROLLING_BOULDER_TRAP:
            case SLP_GAS_TRAP_SET:
                if (lvl < 2)
                    kind = NO_TRAP;
                break;
            case LEVEL_TELEP:
                if (lvl < 5 || level.flags.noteleport)
                    kind = NO_TRAP;
                break;
            case SPIKED_PIT:
                if (lvl < 5)
                    kind = NO_TRAP;
                if (tm && (is_puddle(tm->x, tm->y)
                           || is_sewage(tm->x, tm->y)))
                    kind = NO_TRAP;
                break;
            case LANDMINE:
                if (lvl < 6)
                    kind = NO_TRAP;
                if (tm && (is_puddle(tm->x, tm->y)
                           || is_sewage(tm->x, tm->y)))
                    kind = NO_TRAP;
                break;
            case SPEAR_TRAP_SET:
                if (lvl < 6)
                    kind = NO_TRAP;
                break;
            case WEB:
                if (lvl < 7)
                    kind = NO_TRAP;
                if (tm && (is_puddle(tm->x, tm->y)
                           || is_sewage(tm->x, tm->y)))
                    kind = NO_TRAP;
                break;
            case STATUE_TRAP:
            case POLY_TRAP_SET:
                if (lvl < 8)
                    kind = NO_TRAP;
                break;
            case MAGIC_BEAM_TRAP_SET:
                if (lvl < 12)
                    kind = NO_TRAP;
                if (tm && (is_puddle(tm->x, tm->y)
                           || is_sewage(tm->x, tm->y)))
                    kind = NO_TRAP;
                break;
            case FIRE_TRAP_SET:
                if (!Inhell)
                    kind = NO_TRAP;
                break;
            case ICE_TRAP_SET:
                if (!Iniceq)
                    kind = NO_TRAP;
                break;
            case TELEP_TRAP_SET:
                if (level.flags.noteleport)
                    kind = NO_TRAP;
                break;
            case HOLE:
                /* make these much less often than other traps */
                if (rn2(7))
                    kind = NO_TRAP;
                if (tm && (is_puddle(tm->x, tm->y)
                           || is_sewage(tm->x, tm->y)))
                    kind = NO_TRAP;
                break;
            case PIT:
            case TRAPDOOR:
                if (tm && (is_puddle(tm->x, tm->y)
                           || is_sewage(tm->x, tm->y)))
                    kind = NO_TRAP;
                break;
            }
        } while (kind == NO_TRAP);
    }

    if (is_hole(kind) && !Can_fall_thru(&u.uz))
        kind = ROCKTRAP;

    if (tm) {
        m = *tm;
    } else {
        register int tryct = 0;
        boolean avoid_boulder = (is_pit(kind) || is_hole(kind));

        if (mazeflag)
            do {
                if (++tryct > 200)
                    return;
                mazexy(&m);
            } while (occupied(m.x, m.y)
                     || (avoid_boulder && sobj_at(BOULDER, m.x, m.y)));

	else if (!somexyspace(croom, &m, (avoid_boulder ? 4 : 0)))
	    return;
    }

    t = maketrap(m.x, m.y, kind);
    /* we should always get type of trap we're asking for (occupied() test
       should prevent cases where that might not happen) but be paranoid */
    kind = t ? t->ttyp : NO_TRAP;

    if (kind == WEB) {
        i = rn2(4);
        switch (i) {
        case 0:
            (void) makemon(&mons[PM_JUMPING_SPIDER], m.x, m.y, NO_MM_FLAGS);
            break;
        case 1:
            (void) makemon(&mons[PM_CAVE_SPIDER], m.x, m.y, NO_MM_FLAGS);
            break;
        case 2:
            (void) makemon(&mons[PM_LARGE_SPIDER], m.x, m.y, NO_MM_FLAGS);
            break;
        default:
            (void) makemon(&mons[PM_GIANT_SPIDER], m.x, m.y, NO_MM_FLAGS);
            break;
        }
    }

    /* The hero isn't the only person who's entered the dungeon in
       search of treasure. On the very shallowest levels, there's a
       chance that a created trap will have killed something already
       (and this is guaranteed on the first level).

       This isn't meant to give any meaningful treasure (in fact, any
       items we drop here are typically cursed, other than ammo fired
       by the trap). Rather, it's mostly just for flavour and to give
       players on very early levels a sufficient chance to avoid traps
       that may end up killing them before they have a fair chance to
       build max HP. Including cursed items gives the same fair chance
       to the starting pet, and fits the rule that possessions of the
       dead are normally cursed.

       Some types of traps are excluded because they're entirely
       nonlethal, even indirectly. We also exclude all of the
       later/fancier traps because they tend to have special
       considerations (e.g. webs, portals), often are indirectly
       lethal, and tend not to generate on shallower levels anyway.
       Finally, pits are excluded because it's weird to see an item
       in a pit and yet not be able to identify that the pit is there. */
    if (kind != NO_TRAP && lvl <= (unsigned) rnd(4)
        && kind != SQKY_BOARD && kind != RUST_TRAP_SET
        /* rolling boulder trap might not have a boulder if there was no
           viable path (such as when placed in the corner of a room), in
           which case tx,ty==launch.x,y; no boulder => no dead predecessor */
        && !(kind == ROLLING_BOULDER_TRAP
             && t->launch.x == t->tx && t->launch.y == t->ty)
        && !is_pit(kind) && kind < HOLE) {
        /* Object generated by the trap; initially NULL, stays NULL if
           we fail to generate an object or if the trap doesn't
           generate objects. */
        struct obj *otmp = NULL;
        int victim_mnum; /* race of the victim */
        int quan = rnd(4); /* amount of ammo to dump */

        /* Not all trap types have special handling here; only the ones
           that kill in a specific way that's obvious after the fact. */
        switch (kind) {
        case ARROW_TRAP_SET:
        case BOLT_TRAP_SET:
        case DART_TRAP_SET:
        case SPEAR_TRAP_SET:
        case ROCKTRAP:
            if (t->ammo) {
                if (t->ammo->quan <= quan)
                    t->ammo->quan = quan + 1;

                otmp = splitobj(t->ammo, quan); /* this handles weights */
                if (otmp) {
                    extract_nobj(otmp, &t->ammo);
                    place_object(otmp, m.x, m.y);
                }
            } else {
                impossible("fresh trap %d without ammo?", t->ttyp);
            }
        default:
            /* no item dropped by the trap */
            break;
        }

        /* now otmp is reused for other items we're placing */

        /* Place a random possession. This could be a weapon, tool,
           food, or gem, i.e. the item classes that are typically
           nonmagical and not worthless. */
        do {
            int poss_class = RANDOM_CLASS; /* init => lint suppression */

            switch (rn2(4)) {
            case 0:
                poss_class = WEAPON_CLASS;
                break;
            case 1:
                poss_class = TOOL_CLASS;
                break;
            case 2:
                poss_class = FOOD_CLASS;
                break;
            case 3:
                poss_class = GEM_CLASS;
                break;
            }

            otmp = mkobj(poss_class, FALSE);
            /* these items are always cursed, both for flavour (owned
               by a dead adventurer, bones-pile-style) and for balance
               (less useful to use, and encourage pets to avoid the trap) */
            if (otmp) {
                otmp->blessed = 0;
                otmp->cursed = 1;
                otmp->owt = weight(otmp);
                place_object(otmp, m.x, m.y);
            }

            /* 20% chance of placing an additional item, recursively */
        } while (!rn2(5));

        /* Place a corpse. */
        switch (rn2(15)) {
        case 0:
            /* elf corpses are the rarest as they're the most useful */
            victim_mnum = PM_ELF;
            /* elven adventurers get sleep resistance early; so don't
               generate elf corpses on sleeping gas traps unless a)
               we're on dlvl 2 (1 is impossible) and b) we pass a coin
               flip */
            if (kind == SLP_GAS_TRAP_SET && !(lvl <= 2 && rn2(2)))
                victim_mnum = PM_HUMAN;
            break;
        case 1: case 2:
            victim_mnum = PM_DWARF;
            break;
        case 3: case 4: case 5:
            victim_mnum = PM_ORC;
            break;
        case 6: case 7: case 8: case 9:
            /* more common as they could have come from the Mines */
            victim_mnum = PM_GNOME;
            /* 10% chance of a candle too */
            if (!rn2(10)) {
                otmp = mksobj(rn2(4) ? TALLOW_CANDLE : WAX_CANDLE,
                              TRUE, FALSE);
                otmp->quan = 1;
                otmp->blessed = 0;
                otmp->cursed = 1;
                otmp->owt = weight(otmp);
                place_object(otmp, m.x, m.y);
            }
            break;
        default:
            /* the most common race */
            victim_mnum = PM_HUMAN;
            break;
        }
        otmp = mkcorpstat(CORPSE, NULL, &mons[victim_mnum], m.x, m.y,
                          CORPSTAT_INIT);
        if (otmp)
            otmp->age -= 51; /* died too long ago to eat */
    }
}

void
mkstairs(x, y, up, croom)
xchar x, y;
char up;
struct mkroom *croom;
{
    struct monst *mtmp;

    if (!x) {
        impossible("mkstairs:  bogus stair attempt at <%d,%d>", x, y);
        return;
    }

    /*
     * We can't make a regular stair off an end of the dungeon.  This
     * attempt can happen when a special level is placed at an end and
     * has an up or down stair specified in its description file.
     */
    if ((dunlev(&u.uz) == 1 && up)
        || (dunlev(&u.uz) == dunlevs_in_dungeon(&u.uz) && !up))
        return;

    if (up) {
        xupstair = x;
        yupstair = y;
        upstairs_room = croom;
    } else {
        xdnstair = x;
        ydnstair = y;
        dnstairs_room = croom;
    }

    if (levl[x][y].typ == ICE)
        spot_stop_timers(x, y, MELT_ICE_AWAY);

    levl[x][y].typ = STAIRS;
    levl[x][y].ladder = up ? LA_UP : LA_DOWN;

    /* added because makeriver() runs before mkstairs()
       in the gnomish mines */
    if ((mtmp = m_at(x, y)) != 0) {
        if (is_swimmer(mtmp->data) && mtmp->mundetected)
            mtmp->mundetected = 0;
    }
}

STATIC_OVL void
mkfount(mazeflag, croom)
int mazeflag;
struct mkroom *croom;
{
    coord m;

    if (mazeflag)
        (void) somexyspace(NULL, &m, 16);
    else if (!somexyspace(croom, &m, 8))
        return;

    /* Put a fountain at m.x, m.y */
    levl[m.x][m.y].typ = FOUNTAIN;
    /* Is it a "blessed" fountain? (affects drinking from fountain) */
    if (!rn2(7))
        levl[m.x][m.y].blessedftn = 1;

    level.flags.nfountains++;
}

STATIC_OVL void
mkforge(mazeflag, croom)
int mazeflag;
struct mkroom *croom;
{
    coord m;

    if (mazeflag)
        (void) somexyspace(NULL, &m, 16);
    else if (!somexyspace(croom, &m, 8))
        return;

    /* Put a forge at m.x, m.y */
    levl[m.x][m.y].typ = FORGE;

    level.flags.nforges++;
}

STATIC_OVL void
mksink(croom)
struct mkroom *croom;
{
    coord m;

    if (!somexyspace(croom, &m, 8))
        return;

    /* Put a sink at m.x, m.y */
    levl[m.x][m.y].typ = SINK;

    level.flags.nsinks++;
}

STATIC_OVL void
mkaltar(croom)
struct mkroom *croom;
{
    coord m;
    aligntyp al;

    if (croom->rtype != OROOM)
        return;

    if (!somexyspace(croom, &m, 8))
        return;

    /* Put an altar at m.x, m.y */
    levl[m.x][m.y].typ = ALTAR;

    /* -1 - A_CHAOTIC, 0 - A_NEUTRAL, 1 - A_LAWFUL */
    al = rn2((int) A_LAWFUL + 2) - 1;
    levl[m.x][m.y].altarmask = Align2amask(al);
    levl[m.x][m.y].frac_altar = 0;
}

static void
mkgrave(croom)
struct mkroom *croom;
{
    coord m;
    register int tryct = 0;
    register struct obj *otmp;
    boolean dobell = !rn2(10);

    if (croom->rtype != OROOM)
        return;

    if (!somexyspace(croom, &m, 8))
        return;

    /* Put a grave at <m.x,m.y> */
    make_grave(m.x, m.y, dobell ? "Saved by the bell!" : (char *) 0);

    /* Possibly fill it with objects */
    if (!rn2(3)) {
        /* this used to use mkgold(), which puts a stack of gold on
           the ground (or merges it with an existing one there if
           present), and didn't bother burying it; now we create a
           loose, easily buriable, stack but we make no attempt to
           replicate mkgold()'s level-based formula for the amount */
        struct obj *gold = mksobj(GOLD_PIECE, TRUE, FALSE);

        gold->quan = (long) (rnd(20) + level_difficulty() * rnd(5));
        gold->owt = weight(gold);
        gold->ox = m.x, gold->oy = m.y;
        add_to_buried(gold);
    }
    for (tryct = rn2(5); tryct; tryct--) {
        otmp = mkobj(RANDOM_CLASS, TRUE);
        if (!otmp)
            return;
        curse(otmp);
        otmp->ox = m.x;
        otmp->oy = m.y;
        add_to_buried(otmp);
    }

    /* Leave a bell, in case we accidentally buried someone alive */
    if (dobell)
        (void) mksobj_at(BELL, m.x, m.y, TRUE, FALSE);
    return;
}

/* maze levels have slightly different constraints from normal levels */
#define x_maze_min 2
#define y_maze_min 2

/*
 * Major level transmutation:  add a set of stairs (to the Sanctum) after
 * an earthquake that leaves behind a new topology, centered at inv_pos.
 * Assumes there are no rooms within the invocation area and that inv_pos
 * is not too close to the edge of the map.  Also assume the hero can see,
 * which is guaranteed for normal play due to the fact that sight is needed
 * to read the Book of the Dead.  [That assumption is not valid; it is
 * possible that "the Book reads the hero" rather than vice versa if
 * attempted while blind (in order to make blind-from-birth conduct viable).]
 */
void
mkinvokearea()
{
    int dist;
    xchar xmin = inv_pos.x, xmax = inv_pos.x,
          ymin = inv_pos.y, ymax = inv_pos.y;
    register xchar i;

    /* slightly odd if levitating, but not wrong */
    pline_The("floor shakes violently under you!");
    /*
     * TODO:
     *  Suppress this message if player has dug out all the walls
     *  that would otherwise be affected.
     */
    pline_The("walls around you begin to bend and crumble!");
    display_nhwindow(WIN_MESSAGE, TRUE);

    /* any trap hero is stuck in will be going away now */
    if (u.utrap) {
        if (u.utraptype == TT_BURIEDBALL)
            buried_ball_to_punishment();
        reset_utrap(FALSE);
    }
    mkinvpos(xmin, ymin, 0); /* middle, before placing stairs */

    for (dist = 1; dist < 7; dist++) {
        xmin--;
        xmax++;

        /* top and bottom */
        if (dist != 3) { /* the area is wider that it is high */
            ymin--;
            ymax++;
            for (i = xmin + 1; i < xmax; i++) {
                mkinvpos(i, ymin, dist);
                mkinvpos(i, ymax, dist);
            }
        }

        /* left and right */
        for (i = ymin; i <= ymax; i++) {
            mkinvpos(xmin, i, dist);
            mkinvpos(xmax, i, dist);
        }

        flush_screen(1); /* make sure the new glyphs shows up */
        delay_output();
    }

    You("are standing at the top of a stairwell leading down!");
    mkstairs(u.ux, u.uy, 0, (struct mkroom *) 0); /* down */
    newsym(u.ux, u.uy);
    vision_full_recalc = 1; /* everything changed */
    livelog_write_string(LL_ACHIEVE, "performed the invocation");
}

void
mkgate()
{
    const int a = 66, b = 16,
              x = 17, y1 = 10, y2 = 11;

    /* don't do this if we're not in the sanctum */
    if (!Is_sanctum(&u.uz))
        return;

    /* create the gates leading from the inner sanctum
       to Lucifer and the portal to Purgatory */
    levl[x][y1].typ = DOOR;
    levl[x][y2].typ = DOOR;
    levl[x][y1].doormask = D_LOCKED;
    levl[x][y2].doormask = D_LOCKED;
    if (cansee(x, y1))
        newsym(x, y1);
    if (cansee(x, y2))
        newsym(x, y2);

    if (cansee (x, y1) || cansee(x, y2)) {
        pline("As you %s, the far wall of the sanctum warps.",
              Role_if(PM_INFIDEL) ? "imbue the Idol of Moloch"
                                  : "pickup the Amulet of Yendor");
        pline("A gate appears where the wall used to be.");
    }

    /* once u.uachieve.amulet is triggered (touching the
       Amulet of Yendor or, as an Infidel, imbuing the Idol
       of Moloch for the first time), stair access leading
       out of the sanctum is removed - the only way out is
       forward, through Purgatory */

    /* we know the exact location of the stairs leading
       back up, no need to setup a for loop to find them */
    xupstair = 0; /* remove stairs mask */
    if ((levl[a][b].typ = STAIRS))
        levl[a][b].typ = ROOM;
    if (cansee(a, b))
        newsym(a, b);

    vision_full_recalc = 1;
}

void
mkgate2()
{
    const int x = 23, y = 10;

    /* don't do this if we're not at Mines' End */
    if (!Is_mineend_level(&u.uz))
        return;

    /* create a locked door leading into the treasure room */
    levl[x][y].typ = DOOR;
    levl[x][y].doormask = D_LOCKED;
    if (cansee(x, y))
        newsym(x, y);

    /* odds are the player won't be anywhere near this location
       once they defeat the Rat King, but it is possible */
    if (cansee (x, y))
        pline("A door appears where the wall used to be.");
    else if (!Deaf)
        You_hear("the walls of the dungeon shift in a peculiar way.");

    vision_full_recalc = 1;
}

/* Change level topology.  Boulders in the vicinity are eliminated.
 * Temporarily overrides vision in the name of a nice effect.
 */
STATIC_OVL void
mkinvpos(x, y, dist)
xchar x, y;
int dist;
{
    struct trap *ttmp;
    struct obj *otmp;
    boolean make_rocks;
    register struct rm *lev = &levl[x][y];
    struct monst *mon;

    /* clip at existing map borders if necessary */
    if (!within_bounded_area(x, y, x_maze_min + 1, y_maze_min + 1,
                             x_maze_max - 1, y_maze_max - 1)) {
        /* outermost 2 columns and/or rows may be truncated due to edge */
        if (dist < (7 - 2))
            panic("mkinvpos: <%d,%d> (%d) off map edge!", x, y, dist);
        return;
    }

    /* clear traps */
    if ((ttmp = t_at(x, y)) != 0)
        deltrap_with_ammo(ttmp, DELTRAP_DESTROY_AMMO);

    /* clear boulders; leave some rocks for non-{moat|trap} locations */
    make_rocks = (dist != 1 && dist != 4 && dist != 5) ? TRUE : FALSE;
    while ((otmp = sobj_at(BOULDER, x, y)) != 0) {
        if (make_rocks) {
            fracture_rock(otmp);
            make_rocks = FALSE; /* don't bother with more rocks */
        } else {
            obj_extract_self(otmp);
            obfree(otmp, (struct obj *) 0);
        }
    }

    /* fake out saved state */
    lev->seenv = 0;
    lev->doormask = 0;
    if (dist < 6)
        lev->lit = TRUE;
    lev->waslit = TRUE;
    lev->horizontal = FALSE;
    /* short-circuit vision recalc */
    viz_array[y][x] = (dist < 6) ? (IN_SIGHT | COULD_SEE) : COULD_SEE;

    switch (dist) {
    case 1: /* fire traps */
        if (is_pool(x, y))
            break;
        lev->typ = ROOM;
        ttmp = maketrap(x, y, FIRE_TRAP_SET);
        if (ttmp)
            ttmp->tseen = TRUE;
        break;
    case 0: /* lit room locations */
    case 2:
    case 3:
    case 6: /* unlit room locations */
        lev->typ = ROOM;
        break;
    case 4: /* pools (aka a wide moat) */
    case 5:
        lev->typ = MOAT;
        /* No kelp! */
        break;
    default:
        impossible("mkinvpos called with dist %d", dist);
        break;
    }

    if ((mon = m_at(x, y)) != 0) {
        /* wake up mimics, don't want to deal with them blocking vision */
        if (mon->m_ap_type)
            seemimic(mon);

        if ((ttmp = t_at(x, y)) != 0)
            (void) mintrap(mon);
        else
            (void) minliquid(mon);
    }

    if (!does_block(x, y, lev))
        unblock_point(x, y); /* make sure vision knows this location is open */

    /* display new value of position; could have a monster/object on it */
    newsym(x, y);
}

/*
 * The portal to Ludios is special.  The entrance can only occur within a
 * vault in the main dungeon at a depth greater than 10.  The Ludios branch
 * structure reflects this by having a bogus "source" dungeon:  the value
 * of n_dgns (thus, Is_branchlev() will never find it).
 *
 * Ludios will remain isolated until the branch is corrected by this function.
 */
STATIC_OVL void
mk_knox_portal(x, y)
xchar x, y;
{
    extern int n_dgns; /* from dungeon.c */
    d_level *source;
    branch *br;
    schar u_depth;

    br = dungeon_branch("Fort Ludios");
    if (on_level(&knox_level, &br->end1)) {
        source = &br->end2;
    } else {
        /* disallow Knox branch on a level with one branch already */
        if (Is_branchlev(&u.uz))
            return;
        source = &br->end1;
    }

    /* Already set */
    if (source->dnum < n_dgns)
        return;

    if (!(u.uz.dnum == oracle_level.dnum      /* in main dungeon */
          && u.uz.dnum == nymph_level.dnum    /* Aphrodite's garden */
          && u.uz.dnum == forest_level.dnum   /* Desolate Forest */
          && !at_dgn_entrance("The Quest")    /* but not Quest's entry */
          && (u_depth = depth(&u.uz)) > 10    /* beneath 10 */
          && u_depth < depth(&medusa_level))) /* and above Medusa */
        return;

    /* Adjust source to be current level and re-insert branch. */
    *source = u.uz;
    insert_branch(br, TRUE);

    debugpline0("Made knox portal.");
    place_branch(br, x, y);
}

/*mklev.c*/
