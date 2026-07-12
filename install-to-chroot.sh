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
  cp "$NETHACKBIN" "$NHBINFILE"
  ln -fs "$NHBINFILE" nethack
  LIBS="$LIBS `findlibs $NETHACKBIN`"
  cd "$HDF_CHROOT"
fi

echo "Copying NetHack playground stuff"
cp "$NETHACK_GIT/dat/nhdat" "$HDF_CHROOT/$NHSUBDIR"
chmod 644 "$HDF_CHROOT/$NHSUBDIR/nhdat"
cp "$NETHACK_GIT/dat/symbols" "$HDF_CHROOT/$NHSUBDIR"
chmod 644 "$HDF_CHROOT/$NHSUBDIR/symbols"
cp "$NETHACK_GIT/dat/license" "$HDF_CHROOT/$NHSUBDIR"
chmod 644 "$HDF_CHROOT/$NHSUBDIR/license"
cp "$NETHACK_GIT/dat/NHdump.css" "$HDF_CHROOT/$NHSUBDIR"
chmod 644 "$HDF_CHROOT/$NHSUBDIR/NHdump.css"

echo "Copying sysconf file"
SYSCF="$HDF_CHROOT/$NHSUBDIR/sysconf"
cp "$NETHACK_GIT/sys/unix/sysconf" "$SYSCF"
chmod 644 $SYSCF

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

RECOVER="$NETHACK_GIT/util/recover"

if [ -n "$RECOVER" -a -e "$RECOVER" ]; then
  echo "Copying $RECOVER"
  cp "$RECOVER" "$HDF_CHROOT/$NHSUBDIR/var"
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
		cp $lib "$HDF_CHROOT$lib"
	fi
done

echo "Finished."

