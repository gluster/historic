#!/bin/sh
# Copyright (C) 2006 Z RESEARCH Inc. <http://www.zresearch.com>
#  
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#  
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#  
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#  

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
