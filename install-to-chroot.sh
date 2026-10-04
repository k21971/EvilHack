#!/bin/bash
#
# Ideas and some parts from the original dgl-create-chroot (by joshk@triplehelix.org, modifications by jilles@stack.nl)
# More by <paxed@alt.org>
# More by Michael Andrew Streib <dtype@dtype.org>
# Licensed under the MIT License
# https://opensource.org/licenses/MIT

# autonamed chroot directory. Can rename.
DATESTAMP=`date +%Y%m%d-%H%M%S`
HDF_CHROOT="/opt/nethack/chroot"
NETHACK_GIT="/home/build/NetHack50"
# the user & group from dgamelaunch config file.
USRGRP="games:games"
# COMPRESS from include/config.h; the compression binary to copy. leave blank to skip.
COMPRESSBIN="/bin/gzip"
# fixed data to copy (leave blank to skip)
NH_GIT="/home/build/NetHack50"
# HACKDIR from include/config.h; aka nethack subdir inside chroot
NHSUBDIR="nh500.0-hdf"
# VAR_PLAYGROUND from include/unixconf.h
NH_VAR_PLAYGROUND="/nh500.0-hdf/var/"
# END OF CONFIG
##############################################################################

errorexit()
{
    echo "Error: $@" >&2
    exit 1
}

findlibs()
{
  for i in "$@"; do
      if [ -z "`ldd "$i" | grep 'not a dynamic executable'`" ]; then
         echo $(ldd "$i" | awk '{ print $3 }' | egrep -v ^'\(' | grep lib)
         echo $(ldd "$i" | grep 'ld-linux' | awk '{ print $1 }')
      fi
  done
}

# install_atomic SRC DEST MODE [OWNER]
#
# 🔴 NEVER `cp` over a file in a live chroot. `cp` opens the destination
# O_TRUNC, so an existing file is briefly zero-length WHILE games hold it open:
#   - every running nethack holds nh500.0-hdf/nhdat open for the whole session
#     and reads level/lua data from it, so a truncating overwrite hands a live
#     game a short read mid-play;
#   - for anything mmap'd, a demand fault past the shortened EOF is SIGBUS, and
#     nethack traps only SIGHUP and SIGXCPU -- so that is a LOST CHARACTER, not
#     a caught error.
#
# Stage alongside the target, then rename(2). A running process keeps its old
# inode as `(deleted)` and finishes on it; the next exec picks up the new file.
# Staging in the DESTINATION DIRECTORY is load-bearing -- rename is only atomic
# within a single filesystem, and a cross-device `mv` silently degrades to
# copy+unlink, which reintroduces exactly the window this exists to close.
# Mode and owner are set on the staged copy, so the file is never visible at
# the target path with the wrong permissions.
install_atomic()
{
  _src="$1"; _dst="$2"; _mode="$3"; _own="${4:-}"
  _tmp="$_dst.new.$$"

  # Don't touch what hasn't changed. A routine rebuild is almost always a
  # source-only change, so dat/nhdat, dat/symbols, dat/license, dat/NHdump.css
  # and sysconf come out byte-identical and re-installing them is pure risk for
  # zero benefit -- every write into a live chroot is a chance to get it wrong.
  # Skipping on content also makes the install idempotent and makes its output
  # an honest report of what actually moved.
  #
  # Deliberately NOT "install the binary only": nhdat is version-locked to the
  # binary (its level/lua data must match what the executable expects), so a
  # rule that never copies it would pair a new binary with a stale nhdat the
  # first time dat/ really does change. Compare-then-install keeps them in step.
  if cmp -s "$_src" "$_dst" 2>/dev/null; then
    # Content is already right, but assert mode/owner anyway -- chmod/chown
    # modify the inode in place and never truncate, so they are safe on a file
    # a live game holds open, and this is the only thing that would repair a
    # drifted mode without a content change.
    chmod "$_mode" "$_dst" || errorexit "chmod $_mode on unchanged $_dst failed"
    if [ -n "$_own" ]; then
      chown "$_own" "$_dst" || errorexit "chown $_own on unchanged $_dst failed"
    fi
    echo "  `basename "$_dst"` unchanged - not copying"
    return 0
  fi
  echo "  `basename "$_dst"` CHANGED - staging + atomic rename"
  cp "$_src" "$_tmp"    || errorexit "staging $_dst failed"
  chmod "$_mode" "$_tmp" || errorexit "chmod $_mode on staged $_dst failed"
  if [ -n "$_own" ]; then
    chown "$_own" "$_tmp" || errorexit "chown $_own on staged $_dst failed"
  fi
  # -T so a directory at $_dst can never turn this into a move INTO it.
  mv -T "$_tmp" "$_dst"  || errorexit "atomic rename into $_dst failed"
}

set -e

umask 022

echo "Creating inprogress and extrainfo directories"
mkdir -p "$HDF_CHROOT/dgldir/inprogress-nh500.0-hdf"
chown "$USRGRP" "$HDF_CHROOT/dgldir/inprogress-nh500.0-hdf"
mkdir -p "$HDF_CHROOT/dgldir/extrainfo-nh500"
chown "$USRGRP" "$HDF_CHROOT/dgldir/extrainfo-nh500"

echo "Making $HDF_CHROOT/$NHSUBDIR"
mkdir -p "$HDF_CHROOT/$NHSUBDIR"

NETHACKBIN="$NETHACK_GIT/src/nethack"
if [ -n "$NETHACKBIN" -a ! -e "$NETHACKBIN" ]; then
  errorexit "Cannot find NetHack binary $NETHACKBIN"
fi

if [ -n "$NETHACKBIN" -a -e "$NETHACKBIN" ]; then
  echo "Copying $NETHACKBIN"
  cd "$HDF_CHROOT/$NHSUBDIR"
  NHBINFILE="`basename $NETHACKBIN`-$DATESTAMP"
  # The binary itself is safe to plain-cp: $NHBINFILE is a NEW datestamped name,
  # so no inode a running game has mapped is touched. It is the SYMLINK swap
  # that has to be atomic -- `ln -fs` is unlink+symlink, and any login landing
  # in that window finds no `nethack` at all.
  # Same "don't touch what hasn't changed" rule as install_atomic, and here it
  # also stops a no-op rebuild dropping another ~12M dated copy every run.
  # NB the binary embeds NETHACK_GIT_SHA, so two builds from the same commit
  # compare equal while builds from different commits never do.
  _cur="`readlink nethack 2>/dev/null || true`"
  if [ -n "$_cur" ] && [ -e "$_cur" ] && cmp -s "$NETHACKBIN" "$_cur"; then
    echo "  nethack unchanged ($_cur) - not installing a new dated binary"
  else
    cp "$NETHACKBIN" "$NHBINFILE"
    chown root:root "$NHBINFILE"
    chmod 755 "$NHBINFILE"
    ln -sfn "$NHBINFILE" "nethack.new.$$"
    mv -T "nethack.new.$$" nethack
    echo "  nethack -> $NHBINFILE"
  fi
  LIBS="$LIBS `findlibs $NETHACKBIN`"
  cd "$HDF_CHROOT"
fi

echo "Copying NetHack playground stuff"
# nhdat above all: every live game holds it open for the whole session.
install_atomic "$NETHACK_GIT/dat/nhdat"      "$HDF_CHROOT/$NHSUBDIR/nhdat"      644
install_atomic "$NETHACK_GIT/dat/symbols"    "$HDF_CHROOT/$NHSUBDIR/symbols"    644
install_atomic "$NETHACK_GIT/dat/license"    "$HDF_CHROOT/$NHSUBDIR/license"    644
install_atomic "$NETHACK_GIT/dat/NHdump.css" "$HDF_CHROOT/$NHSUBDIR/NHdump.css" 644

echo "Copying sysconf file"
SYSCF="$HDF_CHROOT/$NHSUBDIR/sysconf"
install_atomic "$NETHACK_GIT/sys/unix/sysconf" "$SYSCF" 644

echo "Creating NetHack variable dir stuff."
mkdir -p "$HDF_CHROOT/$NHSUBDIR/var"
chown -R "$USRGRP" "$HDF_CHROOT/$NHSUBDIR/var"
mkdir -p "$HDF_CHROOT/$NHSUBDIR/var/save"
chown -R "$USRGRP" "$HDF_CHROOT/$NHSUBDIR/var/save"
mkdir -p "$HDF_CHROOT/$NHSUBDIR/var/save/backup"
chown -R "$USRGRP" "$HDF_CHROOT/$NHSUBDIR/var/save/backup"
mkdir -p "$HDF_CHROOT/$NHSUBDIR/var/whereis"
chown -R "$USRGRP" "$HDF_CHROOT/$NHSUBDIR/var/whereis"

touch "$HDF_CHROOT/$NHSUBDIR/var/logfile"
chown -R "$USRGRP" "$HDF_CHROOT/$NHSUBDIR/var/logfile"
touch "$HDF_CHROOT/$NHSUBDIR/var/perm"
chown -R "$USRGRP" "$HDF_CHROOT/$NHSUBDIR/var/perm"
touch "$HDF_CHROOT/$NHSUBDIR/var/record"
chown -R "$USRGRP" "$HDF_CHROOT/$NHSUBDIR/var/record"
touch "$HDF_CHROOT/$NHSUBDIR/var/xlogfile"
chown -R "$USRGRP" "$HDF_CHROOT/$NHSUBDIR/var/xlogfile"
touch "$HDF_CHROOT/$NHSUBDIR/var/livelog"
chown -R "$USRGRP" "$HDF_CHROOT/$NHSUBDIR/var/livelog"
touch "$HDF_CHROOT/$NHSUBDIR/var/wishtracker"
chown -R "$USRGRP" "$HDF_CHROOT/$NHSUBDIR/var/wishtracker"

RECOVER="$NETHACK_GIT/util/recover"

if [ -n "$RECOVER" -a -e "$RECOVER" ]; then
  echo "Copying $RECOVER"
  # recover is CODE, so it belongs next to the game binary, not in var/.
  # var/ is game DATA and is writable by the 'games' user the game drops to; an
  # executable in there can be replaced by a compromised game process and is then
  # run by an admin as root -- persistence + privesc. Same reason the game binary
  # itself is root:root 755.
  # Atomic too: an admin can be mid-`recover` on a crashed game while this runs.
  install_atomic "$RECOVER" "$HDF_CHROOT/$NHSUBDIR/recover" 755 root:root
  rm -f "$HDF_CHROOT/$NHSUBDIR/var/recover"   # clean up the old location
  LIBS="$LIBS `findlibs $RECOVER`"
  cd "$HDF_CHROOT"
fi

LIBS=`for lib in $LIBS; do echo $lib; done | sort | uniq`
echo "Copying libraries:" $LIBS
for lib in $LIBS; do
        mkdir -p "$HDF_CHROOT`dirname $lib`"
        if [ -f "$HDF_CHROOT$lib" ]
	then
		echo "$HDF_CHROOT$lib already exists - skipping."
	else
		# Deliberately a plain cp, NOT install_atomic: this branch only
		# runs when the destination does not exist, so there is no inode
		# for a running game to have mapped and nothing to truncate.
		# Refreshing an EXISTING lib is a different job needing a
		# stage-and-rename pass over every file; do not "fix" this into
		# an overwrite.
		cp $lib "$HDF_CHROOT$lib"
	fi
done

echo "Finished."

