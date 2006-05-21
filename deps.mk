
# build dependency graph

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
