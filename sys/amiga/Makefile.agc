#	NetHack Makefile.
# Copyright (c) Kenneth Lorber, Bethesda, Maryland, 1991,1992,1993,1996.
# NetHack may be freely redistributed.  See license for details.

###
### modified for gcc by Teemu Suikki (zu@iki.fi)
###
### note: you need to use smake.. sorry
###

###
### DIRECTORY STRUCTURE
###

NH = nh:

SBIN = $(NH)sbin/
SLIB = $(NH)slib/
NETHACK = $(NH)NetHack/
HACKEXE = $(NH)HackExe/
AMI = $(NH)sys/amiga/
DAT = $(NH)dat/
DOC = $(NH)doc/
I = $(NH)include/
SHARE = $(NH)sys/share/
NHS = $(NH)src/
TTY = $(NH)win/tty/
WSHARE  = $(NH)win/share/
UTIL = $(NH)util/
O = $(NH)obj/
OO = $(NH)objo/
#	NB: O and OO MUST be different directories

###
### INVOCATION
###

MAKE = smake

# Startup makefile with:
#
#	$(MAKE) -f $(AMI)Makefile.amigcc
#	$(MAKE) -f $(AMI)Makefile.amigcc install
#
# You may use following targets on $(MAKE) command lines:
#   all		do it all (default)
#   link	just create binary from object files
#   obj		just create common object files
#   obja	just create amiga object files
#   objs	just create shared object files
#   clean	deletes the object files
#   spotless	deletes the object files, main binary, and more
#
# Note:  We do not build the Guidebook here since it needs tbl
# (See the file sys/unix/Makefile.doc for more information)

#[SAS5] [and gcc?]
#   If we were to use the precompiled header file feature in a newer version
#   of SAS/C, we would comment out these following two lines.
#   If we don't use precompiled header files, we uncomment it as well.

HDEP	= $(I)hack.h
CSYM	=

#Pathname for uudecode program:
UUDEC	= uudecode

# Flex/Bison command assignments -- Useful only if you have flex/bison
FLEX	= flex
BISON	= bison
# FBFIL and FBLIB may be used, if required by your version of flex or bison,
# to specify additional files or libraries to be linked with
FBFIL	=
FBLIB	= #lib lib:compat.lib

# If you're compiling this on a 1.3 system, you'll have to uncomment the
# following (for use with the ifchange script below).  Also useful instead of
# "protect ifchange +s"
EXECUTE = execute

# Headers we depend on
AMDEP = $(AMI)winproto.h $(AMI)winext.h $(AMI)windefs.h $(I)winami.h

# Pathname for the C compiler being used.

CC = gcc -c
ASM = as

# Compilation flags for selected C Compiler:
#   $(CFLAGS) should appear before filename arguments of $(CC) command line.

CFLAGS = -O3 -I $(I)

# Components of various link command lines:
#   $(LINK) should be the pathname of the linker being used (with any options
#   that should appear at the beginning of the command line).  The name of the
#   output file should appear immediately after $(LNSPEC).  $(LIN) should
#   appear before the list of object files in each link command.  $(LLINK)
#   should appear as the list of object files in the link command line that
#   creates the EvilHack.executable.  $(LLIB) should appear at the end of each
#   link command line.

LINK = gcc -noixemul -O3
LIN	 =
LLINK	 =
LLIB	 =
FLLIB	 =
OBJSPEC = -o
PNSPEC	= -o
LNSPEC = -o
CCLINK	= gcc -noixemul
CLFLAGS = -O3
INCLSPEC = -I
DEFSPEC = -D
IGNSPEC = -j

###
### FILE LISTS
###

# A more reasonable random number generator (recommended for the Amiga):

RANDOBJ	= $(O)random.o

.PRECIOUS:  $(I)config.h $(I)decl.h $(I)hack.h $(I)permonst.h $(I)you.h

# Almost nothing below this line should have to be changed.
# (Exceptions are marked by [SAS6], [MANX], etc.)
#
# Other things that have to be reconfigured are in config.h,
# (amiconf.h, pcconf.h), and possibly system.h, tradstdc.h.

# Object files for makedefs:

MAKEOBJS = \
	$(OO)makedefs.o $(O)monst.o $(O)objects.o

# Object files for special levels compiler:

SPLEVOBJS = \
	$(OO)lev_yacc.o $(OO)lev_lex.o	$(OO)lev_main.o \
	$(O)decl.o 	$(O)drawing.o	$(O)monst.o \
	$(O)objects.o	$(OO)panic.o

# Object files for dungeon compiler

DGNCOMPOBJS = \
	$(OO)dgn_yacc.o	$(OO)dgn_lex.o	$(OO)dgn_main.o $(O)alloc.o $(OO)panic.o

# Object files for NetHack:

COMMOBJ = \
	$(O)allmain.o	$(O)alloc.o	$(O)apply.o 	$(O)artifact.o	\
	$(O)attrib.o	$(O)ball.o	$(O)bones.o	$(O)botl.o	\
	$(O)cmd.o	$(O)dbridge.o	$(O)decl.o	$(O)detect.o	\
	$(O)dig.o	$(O)display.o	$(O)dlb.o	$(O)do.o	\
	$(O)do_name.o	$(O)do_wear.o	$(O)dog.o	$(O)dogmove.o	\
	$(O)dokick.o	$(O)dothrow.o	$(O)drawing.o	$(O)dungeon.o	\
	$(O)eat.o	$(O)end.o	$(O)engrave.o	$(O)exper.o	\
	$(O)explode.o	$(O)extralev.o	$(O)files.o 	$(O)fountain.o	\
	$(O)hack.o	$(O)hacklib.o	$(O)invent.o	$(O)light.o	\
	$(O)lock.o	$(O)mail.o	$(O)makemon.o	$(O)mapglyph.o	\
	$(O)mcastu.o	$(O)mhitm.o	$(O)mhitu.o	$(O)minion.o	\
	$(O)mklev.o	$(O)mkmap.o	$(O)mkmaze.o	$(O)mkobj.o	\
	$(O)mkroom.o	$(O)mon.o	$(O)mondata.o	$(O)monmove.o	\
	$(O)monst.o	$(O)mplayer.o	$(O)mthrowu.o	$(O)muse.o	\
	$(O)music.o	$(O)o_init.o	$(O)objects.o	$(O)objnam.o	\
	$(O)options.o	$(O)pager.o	$(O)pickup.o	$(O)pline.o	\
	$(O)polyself.o	$(O)potion.o	$(O)pray.o	$(O)priest.o	\
	$(O)quest.o	$(O)questpgr.o	$(O)read.o	$(O)rect.o	\
	$(O)region.o	$(O)restore.o	$(O)rnd.o	$(O)role.o	\
	$(O)rumors.o	$(O)save.o	$(O)shk.o	$(O)shknam.o	\
	$(O)sit.o	$(O)sounds.o	$(O)sp_lev.o	$(O)spell.o	\
	$(O)steal.o	$(O)steed.o	$(O)sys.o	$(O)teleport.o	\
	$(O)timeout.o	$(O)topten.o	$(O)track.o	$(O)trap.o	\
	$(O)u_init.o	$(O)uhitm.o	$(O)vault.o	$(O)version.o	\
	$(O)vision.o	$(O)weapon.o	$(O)were.o	$(O)wield.o	\
	$(O)windows.o	$(O)wizard.o	$(O)worm.o	$(O)worn.o	\
	$(O)write.o	$(O)zap.o

MAKEDEFOBJ = \
	$(O)monstr.o

AMIGAOBJ = \
	$(O)amidos.o	$(O)amirip.o 	$(O)amisnd.o	$(O)amistack.o	\
	$(O)amiwind.o	$(O)winami.o	$(O)winchar.o	$(O)winfuncs.o	\
	$(O)winkey.o	$(O)winmenu.o	$(O)winreq.o	$(O)winstr.o

# Objects from assembly sources (because DMake can't handle default rules)
AMIGAOBJ2 = \
#	$(O)dispmap.o

SHAREOBJ = \
	$(O)pcmain.o	$(RANDOBJ)

TTYOBJ = \
	$(O)getline.o $(O)termcap.o $(O)topl.o $(O)wintty.o $(O)amitty.o \
	$(O)rip.o

# Yuck yuck yuck.  Have to tell DMake where these are, since they're not
# all in the same place.
TTYSRC = \
	$(TTY)getline.c $(TTY)termcap.c $(TTY)topl.c $(TTY)wintty.c \
	$(AMI)amitty.c $(NHS)rip.c

# All the object files for NetHack:

HOBJ = $(COMMOBJ) $(AMIGAOBJ) $(AMIGAOBJ2) $(SHAREOBJ) $(MAKEDEFOBJ) $(TTYOBJ)

###
### DATA FILES
###

# quest files
ADFILES1= $(SLIB)Arc-fila.lev $(SLIB)Arc-filb.lev $(SLIB)Arc-loca.lev \
	$(SLIB)Arc-strt.lev
ADFILES= $(SLIB)Arc-goal.lev $(ADFILES1)

BDFILES1= $(SLIB)Bar-fila.lev $(SLIB)Bar-filb.lev $(SLIB)Bar-loca.lev \
	$(SLIB)Bar-strt.lev
BDFILES= $(SLIB)Bar-goal.lev $(BDFILES1)

CDFILES1= $(SLIB)Cav-fila.lev $(SLIB)Cav-filb.lev $(SLIB)Cav-loca.lev \
	$(SLIB)Cav-strt.lev
CDFILES= $(SLIB)Cav-goal.lev $(CDFILES1)

HDFILES1= $(SLIB)Hea-fila.lev $(SLIB)Hea-filb.lev $(SLIB)Hea-loca.lev \
	$(SLIB)Hea-strt.lev
HDFILES= $(SLIB)Hea-goal.lev $(HDFILES1)

IDFILES1= $(SLIB)Inf-fila.lev $(SLIB)Inf-filb.lev $(SLIB)Inf-loca.lev \
	$(SLIB)Inf-strt.lev
IDFILES= $(SLIB)Inf-goal.lev $(IDFILES1)

KDFILES1= $(SLIB)Kni-fila.lev $(SLIB)Kni-filb.lev $(SLIB)Kni-loca.lev \
	$(SLIB)Kni-strt.lev
KDFILES= $(SLIB)Kni-goal.lev $(KDFILES1)

MDFILES1= $(SLIB)Mon-fila.lev $(SLIB)Mon-filb.lev $(SLIB)Mon-loca.lev \
	$(SLIB)Mon-strt.lev
MDFILES= $(SLIB)Mon-goal.lev $(MDFILES1)

PDFILES1= $(SLIB)Pri-fila.lev $(SLIB)Pri-filb.lev $(SLIB)Pri-loca.lev \
	$(SLIB)Pri-strt.lev
PDFILES= $(SLIB)Pri-goal.lev $(PDFILES1)

RDFILES1= $(SLIB)Rog-fila.lev $(SLIB)Rog-filb.lev $(SLIB)Rog-loca.lev \
	$(SLIB)Rog-strt.lev
RDFILES= $(SLIB)Rog-goal.lev $(RDFILES1)

RANFILES1= $(SLIB)Ran-fila.lev $(SLIB)Ran-filb.lev $(SLIB)Ran-loca.lev \
	$(SLIB)Ran-strt.lev
RANFILES= $(SLIB)Ran-goal.lev $(RANFILES1)

SDFILES1= $(SLIB)Sam-fila.lev $(SLIB)Sam-filb.lev $(SLIB)Sam-loca.lev \
	$(SLIB)Sam-strt.lev
SDFILES= $(SLIB)Sam-goal.lev $(SDFILES1)

TDFILES1= $(SLIB)Tou-fila.lev $(SLIB)Tou-filb.lev $(SLIB)Tou-loca.lev \
	$(SLIB)Tou-strt.lev
TDFILES= $(SLIB)Tou-goal.lev $(TDFILES1)

VDFILES1= $(SLIB)Val-fila.lev $(SLIB)Val-filb.lev $(SLIB)Val-loca.lev \
	$(SLIB)Val-strt.lev
VDFILES= $(SLIB)Val-goal.lev $(VDFILES1)

WDFILES1= $(SLIB)Wiz-fila.lev $(SLIB)Wiz-filb.lev $(SLIB)Wiz-loca.lev \
	$(SLIB)Wiz-strt.lev
WDFILES= $(SLIB)Wiz-goal.lev $(WDFILES1)

XDFILES=	$(ADFILES) $(BDFILES) $(CDFILES) $(HDFILES) $(IDFILES) $(KDFILES) \
		$(MDFILES) $(PDFILES) $(RDFILES) $(RANFILES) $(SDFILES) $(TDFILES) \
		$(VDFILES) $(WDFILES)

SOUNDFILES= \
	$(SBIN)cvtsnd \
	$(SLIB)sounds \
	$(SLIB)sounds/Bell $(SLIB)sounds/Bugle \
	$(SLIB)sounds/Drum_Of_Earthquake \
	$(SLIB)sounds/Fire_Horn $(SLIB)sounds/Frost_Horn \
	$(SLIB)sounds/Leather_Drum $(SLIB)sounds/Magic_Flute \
	$(SLIB)sounds/Magic_Harp $(SLIB)sounds/Tooled_Horn \
	$(SLIB)sounds/Wooden_Flute $(SLIB)sounds/Wooden_Harp

TILEFILES= \
	$(SBIN)txt2iff \
	$(NETHACK)tiles \
	$(NETHACK)tiles/objects.iff \
	$(NETHACK)tiles/monsters.iff \
	$(NETHACK)tiles/other.iff

INSTDUNGEONFILES1= \
	$(SLIB)air.lev		$(SLIB)asmodeus.lev	$(SLIB)astral.lev \
	$(SLIB)baalz.lev	$(SLIB)bigrm-1.lev	$(SLIB)bigrm-2.lev \
	$(SLIB)bigrm-3.lev	$(SLIB)bigrm-4.lev	$(SLIB)bigrm-5.lev \
	$(SLIB)castle-1.lev	$(SLIB)dungeon		$(SLIB)earth.lev \
	$(SLIB)fakewiz1.lev	$(SLIB)fakewiz2.lev	$(SLIB)fire.lev \
	$(SLIB)juiblex.lev	$(SLIB)knox-1.lev	$(SLIB)medusa-1.lev \
	$(SLIB)medusa-2.lev	$(SLIB)minend-1.lev	$(SLIB)minend-2.lev \
	$(SLIB)minetn-1.lev	$(SLIB)minetn-2.lev	$(SLIB)minefill.lev \
	$(SLIB)options		$(SLIB)oracle.lev	$(SLIB)orcus.lev \
	$(SLIB)sanctum.lev	$(SLIB)soko1-1.lev	$(SLIB)soko1-2.lev \
	$(SLIB)soko2-1.lev	$(SLIB)soko2-2.lev	$(SLIB)soko3-1.lev \
	$(SLIB)soko3-2.lev	$(SLIB)soko4-1.lev	$(SLIB)soko4-2.lev \
	$(SLIB)tower1.lev	$(SLIB)tower2.lev	$(SLIB)tower3.lev \
	$(SLIB)valley.lev	$(SLIB)water.lev	$(SLIB)wizard1.lev \
	$(SLIB)wizard2.lev	$(SLIB)wizard3.lev      $(SLIB)castle-2.lev \
        $(SLIB)castle-3.lev     $(SLIB)knox-2.lev       $(SLIB)knox-3.lev \
        $(SLIB)hellfill.lev     $(SLIB)demogor.lev      $(SLIB)dis.lev \
        $(SLIB)yeenoghu.lev     $(SLIB)geryon.lev	$(SLIB)tower4.lev \
	$(XDFILES)

INSTDUNGEONFILES= $(NETHACK)NetHack.cnf $(INSTDUNGEONFILES1)


INSTDATAFILES= \
	$(NETHACK)license	$(NETHACK)logfile	$(NETHACK)record \
	$(NETHACK)tomb.iff	$(NETHACK)amii.hlp 	$(NETHACK)Recover.txt \
	$(NETHACK)GuideBook.txt	$(NETHACK)EvilHack.txt	$(NETHACK)Install.ami \
#	$(NETHACK)HackWB.hlp	$(NETHACK)WBDefaults.def

LIBFILES= \
	$(INSTDUNGEONFILES1) \
	$(SLIB)cmdhelp	 	$(SLIB)data		$(SLIB)dungeon \
	$(SLIB)help		$(SLIB)hh		$(SLIB)history \
	$(SLIB)opthelp		$(SLIB)oracles		$(SLIB)rumors \
	$(SLIB)quest.dat	$(SLIB)wizhelp

###
### Getting down to business:
###

all:  $(COMPACT_HEADERS) $(SBIN)lev_comp $(SBIN)dgn_comp $(SBIN)NetHack \
	$(SBIN)dlb $(NETHACK)recover #$(NETHACK)HackCli $(SBIN)splitter \
#	$(NETHACK)HackWB 

install: inst-data inst-dungeon inst-fonts inst-sounds inst-tiles \
	 $(NETHACK)recover $(NETHACK)NetHack $(NETHACK)nhdat
	#$(NETHACK)NetHack.dir inst-icons

$(SBIN)NetHack:  link

$(NETHACK)NetHack: $(SBIN)NetHack
	copy $(SBIN)NetHack $(NETHACK)NetHack

## uuh this is messy.. smake has weird command line length limit
link: $(HOBJ)
	list to t:link lformat="$(O)%s" $(O)\#?.o QUICK NOHEAD
	echo "\#sh" to t:cc
	echo "$(LINK) $(LNSPEC) $(SBIN)NetHack $(LIN) $(LLIB) $(LLINK) " >>t:cc noline
	fmt -u -w 2500 t:link >>t:cc
	sh t:cc
	delete t:cc t:link


## dlb support
$(OO)dlb_main.o:	$(UTIL)dlb_main.c $(HDEP) $(I)dlb.h $(I)date.h
	$(CC) $(CFLAGS) $(OBJSPEC)$(OO)dlb_main.o $(UTIL)dlb_main.c

$(SBIN)dlb:	$(OO)dlb_main.o $(O)dlb.o $(O)alloc.o $(OO)panic.o
	$(LINK) $(PNSPEC) $(SBIN)dlb $(LIN) $(OO)dlb_main.o $(O)dlb.o \
	  $(O)alloc.o $(OO)panic.o $(LLIB)

obj:  $(HOBJ)

obja:  $(AMIGAOBJ)

objs:  $(SHAREOBJ)


SUFFIXES = .lev .des
.des.lev:
	$(SBIN)lev_comp $<

# The default method for creating object files:

#$(O)%.o: $(NHS)%.c
.c.o:
	$(CC) $(CFLAGS) $(CSYM) $(OBJSPEC)$@ $<

clean:
	-delete $(O)\#?.o $(OO)\#?.o

spotless:  clean
	-delete $(SBIN)NetHack $(SBIN)lev_comp $(SBIN)makedefs $(SBIN)dgn_comp
	-delete $(SBIN)cvtsnd $(SBIN)dlb $(SBIN)txt2iff $(SBIN)splitter
	-delete $(SBIN)tilemap
	-delete $(SLIB)data $(SLIB)rumors
	-delete $(SLIB)\#?.lev
	-delete $(SLIB)dungeon
	-delete $(SLIB)cmdhelp $(SLIB)help $(SLIB)hh $(SLIB)history
	-delete $(SLIB)opthelp $(SLIB)options $(SLIB)oracles
	-delete $(SLIB)quest.dat $(SLIB)wizhelp
#	-delete $(SLIB)earth.lev $(SLIB)air.lev $(SLIB)fire.lev
#	-delete $(SLIB)water.lev $(SLIB)astral.lev
#	-delete $(SLIB)tower1.lev $(SLIB)tower2.lev $(SLIB)tower3.lev $(SLIB)tower4.lev
#	-delete $(SLIB)fakewiz1.lev $(SLIB)fakewiz2.lev
#	-delete $(SLIB)medusa-1.lev $(SLIB)medusa-2.lev
#	-delete $(SLIB)oracle.lev $(SLIB)wizard1.lev $(SLIB)wizard2.lev
#	-delete $(SLIB)wizard3.lev $(DAT)dungeon.pdf $(SLIB)valley.lev
#	-delete $(SLIB)minefill.lev
#	-delete $(SLIB)minetn-1 $(SLIB)minetn-2 $(SLIB)minend-1 $(SLIB)minend-2
#	-delete	$(SLIB)soko1-1.lev $(SLIB)soko1-2.lev $(SLIB)soko2-1.lev
#	-delete $(SLIB)soko2-2.lev $(SLIB)soko3-1.lev $(SLIB)soko3-2.lev
#	-delete $(SLIB)soko4-1.lev $(SLIB)soko4-2.lev
#	-delete $(ADFILES)
#	-delete $(BDFILES)
#	-delete $(CDFILES)
#	-delete $(HDFILES)
#	-delete $(KDFILES)
#	-delete $(MDFILES)
#	-delete $(PDFILES)
#	-delete $(RDFILES)
#	-delete $(RANFILES)
#	-delete $(SDFILES)
#	-delete $(TDFILES)
#	-delete $(VDFILES)
#	-delete $(WDFILES)
	-delete $(I)onames.h $(I)pm.h $(I)date.h
	-delete $(NHS)tile.c $(NHS)monstr.c
	-delete $(I)tile.h 
#	-echo to $(I)onames.h "" noline
#	-c:wait 2
#	-echo to $(I)pm.h "" noline
#	-c:wait 2
#	-setdate $(UTIL)makedefs.c
#	-c:wait 2

# Creating precompiled version of $(I)hack.h to save disk I/O.

#
#	Please note:	The dependency lines for the modules here are
#			deliberately incorrect.  Including "hack.h" in
#			the dependency list would cause a dependency
#			loop.
#

$(SBIN)makedefs: $(MAKEOBJS)
	$(LINK) $(LNSPEC) $(SBIN)makedefs $(LIN) $(MAKEOBJS) $(LLIB)

$(OO)makedefs.o:  $(UTIL)makedefs.c $(I)config.h $(I)permonst.h $(I)monsym.h \
		$(I)objclass.h  $(I)patchlevel.h $(I)qtext.h $(I)artilist.h
	$(CC) $(DEFSPEC)MAKEDEFS_C $(CFLAGS) $(OBJSPEC)$@ $(UTIL)makedefs.c

$(SBIN)lev_comp:  $(SPLEVOBJS)
	$(LINK) $(LNSPEC) $(SBIN)lev_comp $(LIN) $(SPLEVOBJS) $(FBFIL) $(FLLIB)

$(SBIN)dgn_comp:  $(DGNCOMPOBJS)
	$(LINK) $(LNSPEC) $(SBIN)dgn_comp $(LIN) $(DGNCOMPOBJS) $(FBFIL) $(FLLIB)

$(OO)lev_yacc.o:  $(UTIL)lev_yacc.c $(HDEP) $(I)sp_lev.h $(I)pm.h $(I)onames.h
#	setdate $(UTIL)lev_yacc.c
	$(CC) $(DEFSPEC)LEV_LEX_C $(DEFSPEC)PREFIX="NH:slib/" $(CFLAGS) \
		$(DEFSPEC)alloca=malloc $(OBJSPEC)$@ $(UTIL)lev_yacc.c

$(OO)lev_lex.o: $(UTIL)lev_lex.c $(HDEP) $(I)lev_comp.h $(I)sp_lev.h
	$(CC) $(DEFSPEC)LEV_LEX_C $(CFLAGS) $(OBJSPEC)$@ $(UTIL)lev_lex.c

$(OO)lev_main.o:  $(UTIL)lev_main.c $(HDEP) $(I)pm.h $(I)onames.h $(I)date.h
	$(CC) $(DEFSPEC)LEV_LEX_C $(DEFSPEC)AMIGA $(CFLAGS) $(OBJSPEC)$@ \
		$(UTIL)lev_main.c

$(OO)dgn_yacc.o:  $(UTIL)dgn_yacc.c $(HDEP) $(I)dgn_file.h $(I)patchlevel.h
	$(CC) $(DEFSPEC)LEV_LEX_C $(CFLAGS) $(DEFSPEC)alloca=malloc \
		$(OBJSPEC)$@ $(UTIL)dgn_yacc.c

$(OO)dgn_lex.o: $(UTIL)dgn_lex.c $(I)config.h $(I)dgn_comp.h $(I)dgn_file.h
	$(CC) $(DEFSPEC)LEV_LEX_C $(CFLAGS) $(OBJSPEC)$@ $(UTIL)dgn_lex.c

$(OO)dgn_main.o: $(UTIL)dgn_main.c $(I)config.h $(I)date.h
	$(CC) $(DEFSPEC)LEV_LEX_C $(DEFSPEC)AMIGA $(CFLAGS) $(OBJSPEC)$@ \
		$(UTIL)dgn_main.c

$(OO)panic.o: $(UTIL)panic.c $(HDEP)

$(OO)recover.o: $(UTIL)recover.c $(I)config.h $(I)date.h
	$(CC) $(DEFSPEC)LEV_LEX_C $(DEFSPEC)AMIGA $(CFLAGS) $(OBJSPEC)$@ \
		$(UTIL)recover.c

$(NETHACK)recover: $(OO)recover.o
	$(LINK) $(LNSPEC) $(NETHACK)recover $(LIN) $(OO)recover.o $(LLIB)

# [OPTION] -- If you have flex/bison, leave these uncommented.  Otherwise,
# comment them out and be careful! (You're not guaranteed to have the most
# up to date *_comp.c, *_comp.h and *_lex.c)

$(I)lev_comp.h: $(UTIL)lev_yacc.c $(I)patchlevel.h

$(UTIL)lev_yacc.c:  $(UTIL)lev_comp.y $(I)patchlevel.h
	$(BISON) -d $(UTIL)lev_comp.y
#	copy y.tab.c $(UTIL)lev_yacc.c
#	copy y.tab.h $(I)lev_comp.h
	copy $(UTIL)lev_comp.tab.c $(UTIL)lev_yacc.c
	copy $(UTIL)lev_comp.tab.h $(I)lev_comp.h
#	delete y.tab.c
#	delete y.tab.h
	delete $(UTIL)lev_comp.tab.c
	delete $(UTIL)lev_comp.tab.h

$(UTIL)lev_lex.c:  $(UTIL)lev_comp.l $(I)patchlevel.h
	$(FLEX) $(UTIL)lev_comp.l
	copy lex.yy.c $(UTIL)lev_lex.c
	delete lex.yy.c

$(I)dgn_comp.h: $(UTIL)dgn_yacc.c $(I)patchlevel.h

$(UTIL)dgn_yacc.c:  $(UTIL)dgn_comp.y $(I)patchlevel.h
	$(BISON) -d $(UTIL)dgn_comp.y
#	copy y.tab.c $(UTIL)dgn_yacc.c
#	copy y.tab.h $(I)dgn_comp.h
	copy $(UTIL)dgn_comp.tab.c $(UTIL)dgn_yacc.c
	copy $(UTIL)dgn_comp.tab.h $(I)dgn_comp.h
#	delete y.tab.c
#	delete y.tab.h
	delete $(UTIL)dgn_comp.tab.c
	delete $(UTIL)dgn_comp.tab.h

$(UTIL)dgn_lex.c:  $(UTIL)dgn_comp.l $(I)patchlevel.h
	$(FLEX) $(UTIL)dgn_comp.l
	copy lex.yy.c $(UTIL)dgn_lex.c
	delete lex.yy.c

#
#	The following include files depend on makedefs to be created.
#	As a result, they are not defined in HACKINCL, instead, their
#	dependencies are explicitly outlined here.
#

#
#	date.h should be remade any time any of the source or include code
#	is modified.  Unfortunately, this would make the contents of this
#	file far more complex.	Since "hack.h" depends on most of the include
#	files, we kludge around this by making date.h dependent on hack.h,
#	even though it doesn't include this file.
#

$(I)date.h $(DAT)options:  $(HDEP) $(SBIN)makedefs $(AMIGAOBJ) $(I)patchlevel.h
	$(SBIN)makedefs -v
	$(EXECUTE) ifchange MOVE $(I)t.date.h $(I)date.h
	-c:wait 2

$(I)onames.h:  $(SBIN)makedefs
	$(SBIN)makedefs -o
	$(EXECUTE) ifchange TOUCH $(I)t.onames.h $(I)onames.h $(I)decl.h
	$(EXECUTE) ifchange MOVE $(I)t.onames.h $(I)onames.h
	-c:wait 2

$(I)pm.h:  $(SBIN)makedefs
	$(SBIN)makedefs -p
	$(EXECUTE) ifchange TOUCH $(I)t.pm.h $(I)pm.h $(I)decl.h $(I)youprop.h
	$(EXECUTE) ifchange MOVE $(I)t.pm.h $(I)pm.h
	-c:wait 2

$(SLIB)quest.dat:	$(DAT)quest.txt $(SBIN)makedefs
	$(SBIN)makedefs -q

$(NHS)monstr.c:  $(HDEP) $(SBIN)makedefs
	$(SBIN)makedefs -m
	-c:wait 2

$(SLIB)oracles:	$(DAT)oracles.txt $(SBIN)makedefs
	$(SBIN)makedefs -h
	-c:wait 2

#
#	The following programs vary depending on what OS you are using.
#	As a result, they are not defined in HACKSRC and their dependencies
#	are explicitly outlined here.
#

$(O)amidos.o:  $(AMI)amidos.c $(HDEP)

$(O)amirip.o:  $(AMI)amirip.c $(HDEP)

$(O)aglue.o:  $(AMI)aglue.a
	$(ASM) $(AFLAGS) $(AOBJSPEC)$(O)aglue.o $(AMI)aglue.a

$(O)amisnd.o:	$(AMI)amisnd.c $(HDEP)

$(O)winchar.o:	$(AMI)winchar.c $(NHS)tile.c $(HDEP)

$(NHS)tile.c:	$(WSHARE)tilemap.c
	$(CCLINK) $(CFLAGS) $(PNSPEC) $(SBIN)tilemap $(WSHARE)tilemap.c
	$(SBIN)tilemap

$(O)winstr.o:	$(AMI)winstr.c $(HDEP) $(AMDEP)

$(O)winreq.o:	$(AMI)winreq.c $(HDEP) $(AMDEP) $(AMI)colorwin.c $(AMI)clipwin.c

$(O)winfuncs.o:	$(AMI)winfuncs.c $(HDEP) $(AMDEP) $(I)patchlevel.h

$(O)winkey.o:	$(AMI)winkey.c $(HDEP) $(AMDEP)

$(O)winmenu.o:	$(AMI)winmenu.c $(HDEP) $(AMDEP)

$(O)winami.o:	$(AMI)winami.c $(HDEP) $(AMDEP) #$(AMI)char.c $(AMI)randwin.c

#$(O)amilib.o:	$(AMI)amilib.c $(HDEP) $(AMDEP)

$(O)amiwind.o:  $(AMI)amiwind.c $(AMI)amimenu.c $(HDEP) $(AMDEP)

$(O)amiwbench.o:  $(AMI)amiwbench.c $(HDEP)

$(O)random.o:  $(SHARE)random.c

$(O)pcmain.o:  $(SHARE)pcmain.c $(HDEP) $(I)dlb.h

$(O)dispmap.o:	$(AMI)dispmap.s
	$(ASM) $(AFLAGS) $(AOBJSPEC)$@ $<

# Stuff to build the front ends
$(NETHACK)HackWB: $(OO)wb.o $(OO)wbx.o $(OO)loader.o $(OO)multi.o
	$(LINK) $(LNSPEC) $(NETHACK)HackWB $(LIN) $(OO)wb.o $(OO)wbx.o \
		$(OO)loader.o $(OO)multi.o $(LLIB)

$(NETHACK)HackCli: $(OO)cli.o $(OO)loader.o $(OO)multi.o
	$(LINK) $(LNSPEC) $(NETHACK)HackCli $(LIN) $(OO)cli.o $(OO)loader.o \
		$(OO)multi.o $(LLIB)

# This needs to exist to eliminate the HackWB startup message
$(NETHACK)WBDefaults.def:
	echo to $(NETHACK)WBDefaults.def

WBH	= $(AMI)wbdefs.h $(AMI)wbstruct.h $(AMI)wbprotos.h
ASP	= $(AMI)splitter
$(OO)wb.o: $(WBH) $(AMI)wb.c $(AMI)wbwin.c $(AMI)wbdata.c $(AMI)wbgads.c \
		$(I)patchlevel.h
	$(CC) $(WBCFLAGS) $(SPLFLAGS) $(OBJSPEC)$(OO)wb.o $(AMI)wb.c

$(OO)wbx.o: $(WBH) $(AMI)wbcli.c $(AMI)wbwin.c $(AMI)wbdata.c \
		$(I)patchlevel.h $(I)date.h
	$(CC) $(WBCFLAGS) $(SPLFLAGS) $(OBJSPEC)$(OO)wbx.o $(AMI)wbcli.c

$(OO)loader.o: $(ASP)/loader.c $(ASP)/split.h $(ASP)/amiout.h $(ASP)/multi.h
	$(CC) $(WBCFLAGS) $(SPLFLAGS) $(OBJSPEC)$(OO)loader.o $(ASP)/loader.c

$(OO)multi.o: $(ASP)/multi.c $(ASP)/multi.h
	$(CC) $(WBCFLAGS) $(SPLFLAGS) $(OBJSPEC)$(OO)multi.o $(ASP)/multi.c

$(OO)cli.o: $(WBH) $(AMI)wbcli.c $(I)patchlevel.h $(I)date.h
	$(CC) $(WBCFLAGS) $(WBC2FLAGS) $(SPLFLAGS) $(OBJSPEC)$(OO)cli.o \
		$(AMI)wbcli.c

####
# splitter support
$(SBIN)splitter:	$(OO)splitter.o $(OO)arg.o
	$(LINK) $(LNSPEC) $(SBIN)splitter $(LIN) $(OO)splitter.o $(OO)arg.o \
		$(LLIB)

$(NETHACK)NetHack.dir:	$(SBIN)splitter $(SBIN)NetHack
	$(SBIN)splitter $(SBIN)NetHack

$(OO)splitter.o:	$(ASP)/splitter.c $(ASP)/split.h $(ASP)/amiout.h $(ASP)/arg.h
	$(CC) $(WBCFLAGS) $(SPLFLAGS) $(OBJSPEC)$(OO)splitter.o \
		$(ASP)/splitter.c

$(OO)arg.o:	$(ASP)/arg.c $(ASP)/arg.h
	$(CC) $(WBCFLAGS) $(SPLFLAGS) $(OBJSPEC)$(OO)arg.o $(ASP)/arg.c 

# Create/copy other stuff into NetHack: directory:

$(NETHACK)tomb.iff:	$(SBIN)xpm2iff $(AMI)grave16.xpm
	$(SBIN)xpm2iff $(AMI)grave16.xpm $(NETHACK)tomb.iff

$(OO)xpm2iff.o:	$(AMI)xpm2iff.c
	$(CC) $(CFLAGS) $(INCLSPEC)$(WSHARE) $(OBJSPEC)$@ $(AMI)xpm2iff.c

$(SBIN)xpm2iff:	$(OO)xpm2iff.o
	$(LINK) $(LNSPEC) $@ $(LIN) $(OO)xpm2iff.o $(FLLIB)

# Tile installation for the tile version of the game
inst-tiles: $(TILEFILES)

$(NETHACK)tiles:
	-makedir $(NETHACK)tiles

$(OO)txt2iff.o:	$(AMI)txt2iff.c
	$(CC) $(CFLAGS) $(CSYM) $(INCLSPEC)$(WSHARE) $(OBJSPEC)$@ \
		$(AMI)txt2iff.c

$(OO)ppmwrite.o: $(WSHARE)ppmwrite.c
	$(CC) $(CFLAGS) $(CSYM) $(INCLSPEC)$(WSHARE) $(OBJSPEC)$@ $(WSHARE)ppmwrite.c

$(OO)tiletext.o:	$(WSHARE)tiletext.c $(I)config.h $(WSHARE)tile.h
	$(CC) $(CFLAGS) $(CSYM) $(INCLSPEC)$(WSHARE) $(OBJSPEC)$@ $(WSHARE)tiletext.c

$(OO)tiletxt.o:	$(WSHARE)tilemap.c $(I)hack.h
	$(CC) $(CFLAGS) $(CSYM) $(DEFSPEC)TILETEXT $(INCLSPEC)$(WSHARE) $(OBJSPEC)$@ $(WSHARE)tilemap.c

NAMEOBJS = $(O)drawing.o $(O)decl.o $(O)monst.o $(O)objects.o

$(SBIN)txt2ppm:	$(OO)ppmwrite.o $(NAMEOBJS) $(O)alloc.o $(OO)panic.o $(OO)tiletext.o $(OO)tiletxt.o
	$(LINK) $(LNSPEC) $@ $(LIN) $(OO)ppmwrite.o $(NAMEOBJS) $(OO)tiletext.o $(OO)tiletxt.o $(O)alloc.o $(OO)panic.o $(FLLIB)

$(SBIN)txt2iff: $(OO)txt2iff.o $(NAMEOBJS) $(OO)tiletext.o $(OO)tiletxt.o
	$(LINK) $(LNSPEC) $@ $(LIN) $(OO)txt2iff.o $(NAMEOBJS) $(OO)tiletext.o \
		$(OO)tiletxt.o  $(FLLIB)

$(NETHACK)tiles/objects.iff: $(WSHARE)objects.txt $(SBIN)txt2iff
	$(SBIN)txt2iff $(WSHARE)objects.txt $(NETHACK)tiles/objects.iff

$(NETHACK)tiles/monsters.iff: $(WSHARE)monsters.txt $(SBIN)txt2iff
	$(SBIN)txt2iff $(WSHARE)monsters.txt $(NETHACK)tiles/monsters.iff

$(NETHACK)tiles/other.iff: $(WSHARE)other.txt $(SBIN)txt2iff
	$(SBIN)txt2iff $(WSHARE)other.txt $(NETHACK)tiles/other.iff

# Sound installation rules.
inst-sounds: $(SOUNDFILES)
	list to T:nhsdat.lst $(SLIB)sounds QUICK NOHEAD
	echo  >T:make-nhsdat $(SBIN)dlb cCfI $(SLIB)sounds $(NETHACK)nhsdat T:nhsdat.lst
	echo >>T:make-nhsdat if not exists $(NETHACK)nhsdat
	echo >>T:make-nhsdat copy $(SLIB)sounds/\#? $(NETHACK)sounds
	echo >>T:make-nhsdat endif
	execute T:make-nhsdat
	-delete T:make-nhsdat

$(SLIB)sounds:
	-makedir $(SLIB)sounds

$(SBIN)cvtsnd: $(OO)cvtsnd.o
	$(LINK) $(LNSPEC) $@ $(LIN) $(OO)cvtsnd.o $(FLLIB)

$(OO)cvtsnd.o: $(AMI)cvtsnd.c

$(SLIB)sounds/Bell: $(SHARE)sounds/bell.uu
	$(UUDEC) $(SHARE)sounds/bell.uu
	$(SBIN)cvtsnd Bell $(SLIB)sounds/Bell
	-delete Bell

$(SLIB)sounds/Bugle: $(SHARE)sounds/bugle.uu
	$(UUDEC) $(SHARE)sounds/bugle.uu
	$(SBIN)cvtsnd Bugle $(SLIB)sounds/Bugle
	-delete Bugle

$(SLIB)sounds/Drum_Of_Earthquake: $(SHARE)sounds/erthdrum.uu
	$(UUDEC) $(SHARE)sounds/erthdrum.uu
	$(SBIN)cvtsnd Drum_Of_Earthquake $(SLIB)sounds/Drum_Of_Earthquake
	-delete Drum_Of_Earthquake

$(SLIB)sounds/Fire_Horn: $(SHARE)sounds/firehorn.uu
	$(UUDEC) $(SHARE)sounds/firehorn.uu
	$(SBIN)cvtsnd Fire_Horn $(SLIB)sounds/Fire_Horn
	-delete Fire_Horn

$(SLIB)sounds/Frost_Horn: $(SHARE)sounds/frsthorn.uu
	$(UUDEC) $(SHARE)sounds/frsthorn.uu
	$(SBIN)cvtsnd Frost_Horn $(SLIB)sounds/Frost_Horn
	-delete Frost_Horn

$(SLIB)sounds/Leather_Drum: $(SHARE)sounds/lethdrum.uu
	$(UUDEC) $(SHARE)sounds/lethdrum.uu
	$(SBIN)cvtsnd Leather_Drum $(SLIB)sounds/Leather_Drum
	-delete Leather_Drum

$(SLIB)sounds/Magic_Flute: $(SHARE)sounds/mgcflute.uu
	$(UUDEC) $(SHARE)sounds/mgcflute.uu
	$(SBIN)cvtsnd Magic_Flute $(SLIB)sounds/Magic_Flute
	-delete Magic_Flute

$(SLIB)sounds/Magic_Harp: $(SHARE)sounds/mgcharp.uu
	$(UUDEC) $(SHARE)sounds/mgcharp.uu
	$(SBIN)cvtsnd Magic_Harp $(SLIB)sounds/Magic_Harp
	-delete Magic_Harp

$(SLIB)sounds/Tooled_Horn: $(SHARE)sounds/toolhorn.uu
	$(UUDEC) $(SHARE)sounds/toolhorn.uu
	$(SBIN)cvtsnd Tooled_Horn $(SLIB)sounds/Tooled_Horn
	-delete Tooled_Horn

$(SLIB)sounds/Wooden_Flute: $(SHARE)sounds/wdnflute.uu
	$(UUDEC) $(SHARE)sounds/wdnflute.uu
	$(SBIN)cvtsnd Wooden_Flute $(SLIB)sounds/Wooden_Flute
	-delete Wooden_Flute

$(SLIB)sounds/Wooden_Harp: $(SHARE)sounds/wdnharp.uu
	$(UUDEC) $(SHARE)sounds/wdnharp.uu
	$(SBIN)cvtsnd Wooden_Harp $(SLIB)sounds/Wooden_Harp
	-delete Wooden_Harp

inst-dungeon: $(INSTDUNGEONFILES)

$(NETHACK)options : $(DAT)options
	copy $(DAT)options $@

# Create compiled dungeon files
BGM= $(SLIB)bigrm-2.lev $(SLIB)bigrm-3.lev $(SLIB)bigrm-4.lev $(SLIB)bigrm-5.lev
$(BGM):	$(SLIB)bigrm-1.lev

$(SLIB)bigrm-1.lev: $(DAT)bigroom.des $(SBIN)lev_comp

$(SLIB)castle.lev:  $(DAT)castle.des $(SBIN)lev_comp

ENDGAME1= $(SLIB)air.lev $(SLIB)earth.lev $(SLIB)fire.lev $(SLIB)water.lev
$(ENDGAME1):	$(SLIB)astral.lev

$(SLIB)astral.lev:	$(DAT)endgame.des $(SBIN)lev_comp

GEHENNOM1= $(SLIB)asmodeus.lev $(SLIB)baalz.lev $(SLIB)juiblex.lev \
  $(SLIB)orcus.lev $(SLIB)sanctum.lev
$(GEHENNOM1):	$(SLIB)valley.lev

$(SLIB)valley.lev:	$(DAT)gehennom.des $(SBIN)lev_comp

$(SLIB)knox.lev: $(DAT)knox.des $(SBIN)lev_comp

MINES1= $(SLIB)minend-1.lev $(SLIB)minend-2.lev $(SLIB)minetn-1.lev $(SLIB)minetn-2.lev
$(MINES1): $(SLIB)minefill.lev

$(SLIB)minefill.lev: $(DAT)mines.des $(SBIN)lev_comp

$(SLIB)oracle.lev: $(DAT)oracle.des $(SBIN)lev_comp

TOWER1= $(SLIB)tower1.lev $(SLIB)tower2.lev $(SLIB)tower3.lev
$(TOWER1): $(SLIB)tower4.lev

$(SLIB)tower4.lev: $(DAT)tower.des $(SBIN)lev_comp

WIZARD1= $(SLIB)wizard1.lev $(SLIB)wizard2.lev $(SLIB)wizard3.lev \
	$(SLIB)fakewiz1.lev
$(WIZARD1):  $(SLIB)fakewiz2.lev

$(SLIB)fakewiz2.lev:  $(DAT)yendor.des $(SBIN)lev_comp

MEDUSA1= $(SLIB)medusa-1.lev
$(MEDUSA1): $(SLIB)medusa-2.lev

$(SLIB)medusa-2.lev:	$(DAT)medusa.des $(SBIN)lev_comp

SOKOBAN1= $(SLIB)soko1-1.lev $(SLIB)soko1-2.lev $(SLIB)soko2-1.lev \
	$(SLIB)soko2-2.lev $(SLIB)soko3-1.lev $(SLIB)soko3-2.lev \
	$(SLIB)soko4-1.lev
$(SOKOBAN1): $(SLIB)soko4-2.lev

$(SLIB)soko4-2.lev: $(DAT)sokoban.des $(SBIN)lev_comp

$(ADFILES1):	$(SLIB)Arc-goal.lev

$(SLIB)Arc-goal.lev:	$(DAT)Arch.des $(SBIN)lev_comp

$(BDFILES1):	$(SLIB)Bar-goal.lev

$(SLIB)Bar-goal.lev:	$(DAT)Barb.des $(SBIN)lev_comp

$(CDFILES1):	$(SLIB)Cav-goal.lev

$(SLIB)Cav-goal.lev:	$(DAT)Caveman.des $(SBIN)lev_comp

$(HDFILES1):	$(SLIB)Hea-goal.lev

$(SLIB)Hea-goal.lev:	$(DAT)Healer.des $(SBIN)lev_comp

$(IDFILES1):   $(SLIB)Inf-goal.lev

$(SLIB)Inf-goal.lev:   $(DAT)Infidel.des $(SBIN)lev_comp

$(KDFILES1):	$(SLIB)Kni-goal.lev

$(SLIB)Kni-goal.lev:	$(DAT)Knight.des $(SBIN)lev_comp

$(MDFILES1):	$(SLIB)Mon-goal.lev

$(SLIB)Mon-goal.lev:	$(DAT)Monk.des $(SBIN)lev_comp

$(PDFILES1):	$(SLIB)Pri-goal.lev

$(SLIB)Pri-goal.lev:	$(DAT)Priest.des $(SBIN)lev_comp

$(RDFILES1):	$(SLIB)Rog-goal.lev

$(SLIB)Rog-goal.lev:	$(DAT)Rogue.des $(SBIN)lev_comp

$(RANFILES1):	$(SLIB)Ran-goal.lev

$(SLIB)Ran-goal.lev:	$(DAT)Ranger.des $(SBIN)lev_comp

$(SDFILES1):	$(SLIB)Sam-goal.lev

$(SLIB)Sam-goal.lev:	$(DAT)Samurai.des $(SBIN)lev_comp

$(TDFILES1):	$(SLIB)Tou-goal.lev

$(SLIB)Tou-goal.lev:	$(DAT)Tourist.des $(SBIN)lev_comp

$(VDFILES1):	$(SLIB)Val-goal.lev

$(SLIB)Val-goal.lev:	$(DAT)Valkyrie.des $(SBIN)lev_comp

$(WDFILES1):	$(SLIB)Wiz-goal.lev

$(SLIB)Wiz-goal.lev:	$(DAT)Wizard.des $(SBIN)lev_comp

$(SLIB)dungeon:  $(DAT)dungeon.def $(SBIN)makedefs $(SBIN)dgn_comp
	$(SBIN)makedefs -e
	$(SBIN)dgn_comp $(DAT)dungeon.pdf
	copy $(DAT)dungeon $(SLIB)dungeon
	delete $(DAT)dungeon

inst-data: $(INSTDATAFILES)

$(NETHACK)amii.hlp: $(AMI)amii.hlp
	copy $(AMI)amii.hlp $@

#$(NETHACK)data:  $(DAT)data
#	copy $(DAT)data $@

$(SLIB)data:  $(DAT)data.base $(I)config.h $(SBIN)makedefs
	$(SBIN)makedefs -d

#$(NETHACK)rumors:  $(DAT)rumors
#	copy $(DAT)rumors $@

$(SLIB)rumors:  $(DAT)rumors.tru $(DAT)rumors.fal $(SBIN)makedefs
	$(SBIN)makedefs -r

$(SLIB)cmdhelp:  $(DAT)cmdhelp
	copy $(DAT)cmdhelp $@

$(SLIB)help:  $(DAT)help
	copy $(DAT)help $@

$(SLIB)hh:  $(DAT)hh
	copy $(DAT)hh $@

$(NETHACK)HackWB.hlp: $(AMI)HackWB.hlp
	copy $(AMI)HackWB.hlp $@

$(SLIB)history:  $(DAT)history
	copy $(DAT)history $@

$(NETHACK)license:  $(DAT)license
	copy $(DAT)license $@

$(SLIB)opthelp:  $(DAT)opthelp
	copy $(DAT)opthelp $@

$(NETHACK)Recover.txt: $(DOC)Recover.txt
	copy $(DOC)Recover.txt $@

$(NETHACK)GuideBook.txt: $(DOC)GuideBook.txt
	copy $(DOC)GuideBook.txt $@

$(NETHACK)EvilHack.txt: $(DOC)EvilHack.txt
	copy $(DOC)EvilHack.txt $@

$(NETHACK)Install.ami: $(AMI)Install.ami
	copy $(AMI)Install.ami $@

$(NETHACK)logfile:
	echo to $@

$(NETHACK)record:
	echo to $@

$(SLIB)wizhelp: $(DAT)wizhelp
	copy $(DAT)wizhelp $@

# Create the directories here because NetHack.cnf puts them there by default
$(NETHACK)NetHack.cnf:  $(AMI)NetHack.cnf
	copy $(AMI)NetHack.cnf $@
	-makedir $(NETHACK)save
	-makedir $(NETHACK)levels

# Unpack and install fonts

INSTFONTFILES= 	$(NETHACK)hack.font $(NETHACK)hack $(NETHACK)hack/8

inst-fonts: $(INSTFONTFILES)

$(NETHACK)hack/8:  $(AMI)amifont8.uu $(NETHACK)hack
	$(UUDEC) $(AMI)amifont8.uu
	copy 8 $(NETHACK)hack/8
	delete 8

$(NETHACK)hack.font:  $(AMI)amifont.uu
	$(UUDEC) $(AMI)amifont.uu
	copy hack.font $(NETHACK)hack.font
	delete hack.font

$(NETHACK)hack:
	-makedir $@

INSTICONFILES= \
	$(NETHACK)default.icon $(NETHACK)NetHack.info $(NETHACK)NewGame.info \
	$(NETHACK)HackWB.info

inst-icons: $(INSTICONFILES)

# Unpack the icons into place

$(NETHACK)default.icon:  $(AMI)dflticon.uu
	$(UUDEC) $(AMI)dflticon.uu
#	copy default.icon $(NETHACK)default.icon
#	delete default.icon

$(NETHACK)NetHack.info:  $(AMI)NHinfo.uu
	$(UUDEC) $(AMI)NHinfo.uu
#	copy NetHack.info $(NETHACK)NetHack.info
#	delete NetHack.info

$(NETHACK)NewGame.info:  $(AMI)NewGame.uu
	$(UUDEC) $(AMI)NewGame.uu
#	copy NewGame.info $(NETHACK)NewGame.info
#	delete NewGame.info

$(NETHACK)HackWB.info:  $(AMI)HackWB.uu
	$(UUDEC) $(AMI)HackWB.uu
#	copy HackWB.info $(NETHACK)HackWB.info
#	delete HackWB.info

# If DLB is defined, create the nhdat library file in the playground
# directory.  If not, move all the data files there.
$(NETHACK)nhdat:  $(LIBFILES)
	list to T:nhdat.lst $(SLIB) QUICK NOHEAD FILES
	echo  >T:make-nhdat $(SBIN)dlb cCfI $(SLIB) $(NETHACK)nhdat T:nhdat.lst
	echo >>T:make-nhdat if not exists $(NETHACK)nhdat
	echo >>T:make-nhdat copy $(SLIB)\#? $(NETHACK)
	echo >>T:make-nhdat endif
	execute T:make-nhdat
	-delete T:make-nhdat

# DO NOT DELETE THIS LINE

$(O)allmain.o:  $(NHS)allmain.c $(HDEP)

$(O)alloc.o:  $(NHS)alloc.c $(I)config.h

$(O)apply.o:  $(NHS)apply.c $(HDEP)
	$(CC) $(CFLAGS) $(CFLAGS2) $(OBJSPEC)$@ $(NHS)apply.c

$(O)artifact.o:  $(NHS)artifact.c $(HDEP) $(I)artifact.h $(I)artilist.h

$(O)attrib.o:  $(NHS)attrib.c $(HDEP) $(I)artifact.h

$(O)ball.o: $(NHS)ball.c $(HDEP)

$(O)bones.o:  $(NHS)bones.c $(HDEP) $(I)lev.h

$(O)botl.o:	$(NHS)botl.c $(HDEP)

$(O)cmd.o:  $(NHS)cmd.c $(HDEP) $(I)func_tab.h

$(O)dbridge.o:  $(NHS)dbridge.c $(HDEP)

$(O)decl.o:  $(NHS)decl.c $(HDEP) $(I)quest.h

$(O)detect.o: $(NHS)detect.c $(HDEP) $(I)artifact.h

$(O)dig.o: $(NHS)dig.c $(HDEP)

$(O)display.o:  $(NHS)display.c $(HDEP)

$(O)dlb.o: $(NHS)dlb.c $(HDEP) $(I)dlb.h

$(O)do.o:  $(NHS)do.c $(HDEP) $(I)lev.h

$(O)do_name.o:  $(NHS)do_name.c $(HDEP)

$(O)do_wear.o:  $(NHS)do_wear.c $(HDEP)

$(O)dog.o:  $(NHS)dog.c $(HDEP)

$(O)dogmove.o:  $(NHS)dogmove.c $(HDEP) $(I)mfndpos.h

$(O)dokick.o:  $(NHS)dokick.c $(HDEP)

$(O)dothrow.o:  $(NHS)dothrow.c $(HDEP)

$(O)drawing.o:  $(NHS)drawing.c $(HDEP) $(I)tcap.h
	$(CC) $(CFLAGS) $(CFLAGS2) $(OBJSPEC)$@ $(NHS)drawing.c

$(O)dungeon.o:  $(NHS)dungeon.c $(HDEP) $(I)dgn_file.h $(I)dlb.h

$(O)eat.o:  $(NHS)eat.c $(HDEP)

$(O)end.o:  $(NHS)end.c $(HDEP) $(I)lev.h $(I)dlb.h

$(O)engrave.o:  $(NHS)engrave.c $(HDEP) $(I)lev.h

$(O)exper.o:  $(NHS)exper.c $(HDEP)

$(O)explode.o:  $(NHS)explode.c $(HDEP)

$(O)extralev.o:  $(NHS)extralev.c $(HDEP)

$(O)files.o:  $(NHS)files.c $(HDEP) $(I)dlb.h $(I)date.h

$(O)fountain.o:  $(NHS)fountain.c $(HDEP)

$(O)hack.o:  $(NHS)hack.c $(HDEP)

$(O)hacklib.o:  $(NHS)hacklib.c $(HDEP)

$(O)invent.o:  $(NHS)invent.c $(HDEP) $(I)artifact.h

$(O)light.o:  $(NHS)light.c $(HDEP) $(I)lev.h

$(O)lock.o:  $(NHS)lock.c $(HDEP)

$(O)mail.o:  $(NHS)mail.c $(HDEP) $(I)mail.h

$(O)makemon.o:  $(NHS)makemon.c $(HDEP)

$(O)mapglyph.o:  $(NHS)mapglyph.c $(HDEP)

$(O)mcastu.o:  $(NHS)mcastu.c $(HDEP)

$(O)mhitm.o:  $(NHS)mhitm.c $(HDEP) $(I)artifact.h

$(O)mhitu.o:  $(NHS)mhitu.c $(HDEP) $(I)artifact.h
	$(CC) $(CFLAGS) $(CFLAGS2) $(OBJSPEC)$@ $(NHS)mhitu.c

$(O)minion.o:  $(NHS)minion.c $(HDEP)

$(O)mklev.o:  $(NHS)mklev.c $(HDEP)

$(O)mkmap.o:  $(NHS)mkmap.c $(HDEP) $(I)sp_lev.h

$(O)mkmaze.o:  $(NHS)mkmaze.c $(HDEP) $(I)sp_lev.h $(I)lev.h

$(O)mkobj.o:  $(NHS)mkobj.c $(HDEP) $(I)artifact.h $(I)prop.h

$(O)mkroom.o:  $(NHS)mkroom.c $(HDEP)

$(O)mon.o:  $(NHS)mon.c $(HDEP) $(I)mfndpos.h

$(O)mondata.o:  $(NHS)mondata.c $(HDEP)

$(O)monmove.o:  $(NHS)monmove.c $(HDEP) $(I)mfndpos.h $(I)artifact.h

$(O)monst.o:  $(NHS)monst.c $(I)config.h $(I)permonst.h $(I)monsym.h \
		$(I)color.h

$(O)monstr.o:  $(NHS)monstr.c $(HDEP)

$(O)mplayer.o:	$(NHS)mplayer.c $(HDEP)

$(O)mthrowu.o:  $(NHS)mthrowu.c $(HDEP)

$(O)muse.o:	$(NHS)muse.c $(HDEP)
	$(CC) $(CFLAGS) $(CFLAGS2) $(OBJSPEC)$@ $(NHS)muse.c

$(O)music.o:  $(NHS)music.c $(HDEP) #interp.c

$(O)o_init.o:  $(NHS)o_init.c $(HDEP) $(I)lev.h

$(O)objects.o:  $(NHS)objects.c $(I)config.h $(I)obj.h $(I)objclass.h \
		$(I)prop.h $(I)skills.h $(I)color.h
	$(CC) $(CFLAGS) $(INCLSPEC)$(NHS) $(OBJSPEC)$@ $(NHS)objects.c

$(O)objnam.o:  $(NHS)objnam.c $(HDEP)

$(O)options.o:  $(NHS)options.c $(HDEP) $(I)tcap.h $(I)config.h \
		$(I)objclass.h $(I)flag.h

$(O)pager.o:  $(NHS)pager.c $(HDEP) $(I)dlb.h

$(O)pickup.o:  $(NHS)pickup.c $(HDEP)

$(O)pline.o:	$(NHS)pline.c $(HDEP)

$(O)polyself.o:  $(NHS)polyself.c $(HDEP)

$(O)potion.o:  $(NHS)potion.c $(HDEP)

$(O)pray.o:  $(NHS)pray.c $(HDEP)

$(O)priest.o:  $(NHS)priest.c $(HDEP) $(I)mfndpos.h

$(O)quest.o:	$(NHS)quest.c $(HDEP) $(I)quest.h $(I)qtext.h

$(O)questpgr.o: $(NHS)questpgr.c $(HDEP) $(I)qtext.h $(I)dlb.h

$(O)read.o:  $(NHS)read.c $(HDEP)

$(O)rect.o:	$(NHS)rect.c $(HDEP)

$(O)region.o:	$(NHS)region.c $(HDEP)

$(O)restore.o:  $(NHS)restore.c $(HDEP) $(I)lev.h $(I)tcap.h $(I)quest.h

$(O)rnd.o:  $(NHS)rnd.c $(HDEP)

$(O)role.o:	$(NHS)role.c $(HDEP)

$(O)rumors.o:  $(NHS)rumors.c $(HDEP) $(I)dlb.h

$(O)save.o:  $(NHS)save.c $(HDEP) $(I)lev.h $(I)quest.h

$(O)shk.o:  $(NHS)shk.c $(HDEP)
	$(CC) $(CFLAGS) $(CFLAGS2) $(OBJSPEC)$@ $(NHS)shk.c

$(O)shknam.o:  $(NHS)shknam.c $(HDEP)

$(O)sit.o:  $(NHS)sit.c $(HDEP) $(I)artifact.h

$(O)sounds.o:  $(NHS)sounds.c $(HDEP)

$(O)sp_lev.o:  $(NHS)sp_lev.c $(HDEP) $(I)sp_lev.h $(I)rect.h $(I)dlb.h

$(O)spell.o:  $(NHS)spell.c $(HDEP)

$(O)steal.o:  $(NHS)steal.c $(HDEP)

$(O)steed.o:	$(NHS)steed.c $(HDEP)

$(O)sys.o:	$(NHS)sys.c $(HDEP)

$(O)teleport.o:	$(NHS)teleport.c $(HDEP)

$(O)timeout.o:  $(NHS)timeout.c $(HDEP) $(I)lev.h

$(O)topten.o:  $(NHS)topten.c $(HDEP) $(I)dlb.h

$(O)track.o:  $(NHS)track.c $(HDEP)

$(O)trap.o:  $(NHS)trap.c $(HDEP)
	$(CC) $(CFLAGS) $(CFLAGS2) $(OBJSPEC)$@ $(NHS)trap.c

$(O)u_init.o:  $(NHS)u_init.c $(HDEP)

$(O)uhitm.o:  $(NHS)uhitm.c $(HDEP)
	$(CC) $(CFLAGS) $(CFLAGS2) $(OBJSPEC)$@ $(NHS)uhitm.c

$(O)vault.o:  $(NHS)vault.c $(HDEP)

$(O)version.o:  $(NHS)version.c $(HDEP) $(I)date.h $(I)patchlevel.h

$(O)vision.o:  $(NHS)vision.c $(HDEP) #$(I)vis_tab.h

$(O)weapon.o:  $(NHS)weapon.c $(HDEP)

$(O)were.o:  $(NHS)were.c $(HDEP)

$(O)wield.o:  $(NHS)wield.c $(HDEP)

$(O)windows.o:  $(NHS)windows.c $(HDEP) $(I)wintty.h

$(O)wizard.o:  $(NHS)wizard.c $(HDEP) $(I)qtext.h

$(O)worm.o:  $(NHS)worm.c $(HDEP) $(I)lev.h

$(O)worn.o:  $(NHS)worn.c $(HDEP)

$(O)write.o:  $(NHS)write.c $(HDEP)

$(O)zap.o:  $(NHS)zap.c $(HDEP)
	$(CC) $(CFLAGS) $(CFLAGS2) $(OBJSPEC)$@ $(NHS)zap.c

$(O)getline.o:	$(TTY)getline.c $(HDEP) $(I)wintty.h

$(O)termcap.o:	$(TTY)termcap.c $(HDEP) $(I)wintty.h $(I)tcap.h

$(O)topl.o:	$(TTY)topl.c $(HDEP) $(I)wintty.h $(I)tcap.h

$(O)wintty.o:	$(TTY)wintty.c $(HDEP) $(I)wintty.h $(I)tcap.h \
		$(I)date.h $(I)patchlevel.h

$(O)amitty.o:	$(AMI)amitty.c $(HDEP)

$(O)amistack.o:	$(AMI)amistack.c
	$(CC) $(CFLAGS3) $(CSYM) $(OBJSPEC)$@ $(AMI)amistack.c

$(O)rip.o:	$(NHS)rip.c $(HDEP)


$(I)config.h:  $(I)config1.h $(I)tradstdc.h $(I)global.h
	-setdate $(I)config.h
	-c:wait 2

# onames.h handled at onames.h target, pm.h

$(I)decl.h:  $(I)quest.h $(I)spell.h $(I)color.h $(I)obj.h $(I)you.h
	-setdate $(I)decl.h
	-c:wait 2

$(I)global.h:  $(I)coord.h $(I)pcconf.h $(I)amiconf.h
	-setdate $(I)global.h
	-c:wait 2

$(I)hack.h:  $(I)config.h $(I)context.h $(I)trap.h $(I)decl.h $(I)dungeon.h 
		$(I)monsym.h $(I)mkroom.h $(I)objclass.h $(I)flag.h $(I)rm.h 
		$(I)vision.h $(I)display.h $(I)wintype.h $(I)engrave.h 
		$(I)rect.h $(I)region.h $(I)trampoli.h $(I)sys.h
	-setdate $(I)hack.h
	-c:wait 2

$(I)permonst.h:  $(I)monattk.h $(I)monflag.h $(I)align.h
	-setdate $(I)permonst.h
	-c:wait 2

$(I)you.h:  $(I)align.h $(I)attrib.h $(I)monst.h $(I)mextra.h $(I)youprop.h $(I)skills.h
	-setdate $(I)you.h
	-c:wait 2

# pm.h handled at target

$(I)youprop.h:  $(I)prop.h $(I)permonst.h $(I)mondata.h
	-setdate $(I)youprop.h
	-c:wait 2

$(I)display.h: $(I)vision.h $(I)mondata.h
	-setdate $(I)display.h
	-c:wait 2

$(I)dungeon.h: $(I)align.h
	-setdate $(I)dungeon.h
	-c:wait 2

$(I)engrave.h: $(I)trampoli.h $(I)rect.h
	-setdate $(I)engrave.h
	-c:wait 2

$(I)mextra.h: $(I)align.h
	-setdate $(I)mextra.h
	-c:wait 2

$(I)mondata.h: $(I)align.h
	-setdate $(I)mondata.h
	-c:wait 2

$(I)monst.h: $(I)align.h $(I)mextra.h
	-setdate $(I)monst.h
	-c:wait 2

$(I)pcconf.h: $(I)micro.h $(I)system.h
	-setdate $(I)pcconf.h
	-c:wait 2

$(I)rm.h: $(I)align.h
	-setdate $(I)rm.h
	-c:wait 2


#notes
#  install keeps doing re-install because it keeps rebuilding lev_comp???
#  fixed(?) - deleted setdate
