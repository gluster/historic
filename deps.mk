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
ib-libibcommon.live: ib-prepare.live # extract the openib tarball and apply the patch
ib-libibumad.live: ib-libibcommon.live # ib libraries have dependency on previously build libraries
ib-libibmad.live: ib-libibumad.live # ib libraries have dependency on previously build libraries
ib-libosmvendor.live: ib-libibumad.live
ib-osm.live: ib-libosmvendor.live ib-libosmcomp.live
ib-osminclude.live: ib-osm.live
ib-osmtest.live: ib-osm.live
ib-libibverbs.live: ib-osmtest.live sysfs.live
ib-libmthca.live: ib-libibverbs.live
ib-libibat.live: ib-libibverbs.live
ib-libibcm.live: ib-libibat.live
ib-libsdp.live: ib-libibverbs.live
ib-tools.live: pciutils.live zlib.live ib-libmthca.live # These tools use the already existing library
ib-libosmcomp.live: ib-libibumad.live
screen.live: ncurses.live # for tgetent
guile.live: ncurses.live libtool.live gmp.live termcap.live readline.live# libguile needs termcap ncurser and readline. and guile normal support gmp and libtool
freeipmi.live: guile.live libgcrypt.live guile.native # guile dependency for freeipmi
libgcrypt.live: libgpg-error.live # libgpg-error dependency for libgcrypt
libgcrypt.live: libgpg-error.native # for gpg-error-config during ./configure
parted.live: e2fsprogs.live reiserfsprogs.live jfsutils.live xfsprogs.live # support 
portmap.live: tcp-wrappers.live # for LIB_WRAP
gdb.live: termcap.live ncurses.live

# packaging

ib-libibcommon.pkg: ib-prepare.pkg # extract the openib tarball and apply the patch
ib-libibumad.pkg: ib-libibcommon.pkg sysfs.pkg# ib libraries have dependency on previously build libraries
ib-libibmad.pkg: ib-libibumad.pkg # ib libraries have dependency on previously build libraries
ib-libosmvendor.pkg: ib-libibumad.pkg
ib-osm.pkg: ib-libosmvendor.pkg ib-libosmcomp.pkg
ib-osminclude.pkg: ib-osm.pkg
ib-osmtest.pkg: ib-osm.pkg
ib-libibverbs.pkg: ib-osmtest.pkg sysfs.pkg
ib-libmthca.pkg: ib-libibverbs.pkg
ib-libibat.pkg: ib-libibverbs.pkg
ib-libibcm.pkg: ib-libibat.pkg
ib-libsdp.pkg: ib-libibverbs.pkg
ib-tools.pkg: pciutils.pkg zlib.pkg ib-libmthca.pkg # These tools use the already existing library
ib-libosmcomp.pkg: ib-libibumad.pkg
mvapich-gen2.pkg: $(APPS_OPENIB:%=%.pkg)
readline.pkg: ncurses.pkg
torque.pkg: ncurses.pkg
slurm.pkg: openssl.pkg readline.pkg
pdsh.pkg: slurm.pkg readline.pkg
cerebro.pkg: slurm.pkg genders.pkg
rrdtool.pkg: freetype2.pkg libart_lgpl.pkg libpng.pkg
ganglia.pkg: rrdtool.pkg
