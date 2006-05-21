#!/bin/sh

# This shell script is used to cleanup a given CHROOT tree
# by removing unwanted stuff in the live mode
destdir=$1

# remove .h files
find $destdir -name '*.h' -type f -exec rm -vf {} \;

# remove .a files
find $destdir -name 'lib*.a' -type f -exec rm -vf {} \;

# remove .old files
find $destdir -name '*.old' -type f -exec rm -vf {} \;

# remove /usr/share from toolchain
# remove info files

find $destdir -type d -name man | xargs rm -rvf
find $destdir -type d -name info | xargs rm -rvf
find $destdir -type d -name doc | xargs rm -rvf

# remove locale's
find $destdir -type d -name locale | xargs rm -rvf
find $destdir -type d -name gconv | xargs rm -rvf

# strip ELF files
find $destdir -type f -exec /usr/bin/file {} \; | grep ELF | cut -f1 -d: | while read f; do strip --strip-debug --strip-unneeded $f; done
