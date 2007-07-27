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

# build dependency graph

readline.live: ncurses.live
file.live: file.native # `file' is run during build to compile `magic' file
python.live: python.native readline.live ncurses.live openssl.live bzip2.live gzip.live # dependencies to provide bindings
bash.live: ncurses.live readline.live # dependency
grep.live: pcre.live # dependency
dialog.live: ncurses.live # dependency
atftp.live: pcre.live readline.live ncurses.live # depenency
openssh.live: zlib.live openssl.live # dependency
wget.live: openssl.live # for https
kernel-modules.live: kernel-prepare.live # to extract and put .config
kernel-bzimage.live: kernel-prepare.live # -- ditto --
ncurses.live: ncurses.native # for `tic', terminfo compiler to run and compile terminfo files during build
iputils.live: kernel-prepare.live # for KERNEL_INCLUDE
jove.live: ncurses.live # dependency
udev.live: kernel-prepare.live # for KERNEL_DIR
usbutils.live: libusb.live
util-linux.live: ncurses.live # for term.h in more.c
e2fsprogs.live: util-linux.live # for libblkid problem
xfsprogs.live: e2fsprogs.live # for uuid.h
reiserfsprogs.live: e2fsprogs.live # for uuid.h
jfsutils.live: e2fsprogs.live # uuid.h
discover.live: expat.live # dependency
discover.live: discover-data.live # for xml sepcfications
screen.live: ncurses.live # for tgetent
guile.live: ncurses.live libtool.live gmp.live termcap.live readline.live# libguile needs termcap ncurser and readline. and guile normal support gmp and libtool
freeipmi.live: guile.live libgcrypt.live guile.native # guile dependency for freeipmi
libgcrypt.live: libgpg-error.live # libgpg-error dependency for libgcrypt
libgcrypt.live: libgpg-error.native # for gpg-error-config during ./configure
parted.live: e2fsprogs.live reiserfsprogs.live jfsutils.live xfsprogs.live readline.live # support 
portmap.live: tcp-wrappers.live # for LIB_WRAP
gdb.live: termcap.live ncurses.live
jed.live: slang.live
lvm2.live: device-mapper.live

# packaging

readline.pkg: ncurses.pkg
torque.pkg: ncurses.pkg
slurm.pkg: openssl.pkg readline.pkg
pdsh.pkg: slurm.pkg readline.pkg
cerebro.pkg: slurm.pkg genders.pkg
libpng.pkg: zlib.pkg
rrdtool.pkg: freetype2.pkg libart_lgpl.pkg libpng.pkg zlib.pkg
ganglia.pkg: rrdtool.pkg
