ib-libibmad.%: PACKAGE_NAME=openib-userspace-svn3640-1
ib-libibmad.%: LIVE_PREPARE_CMD=cd userspace/management/libibmad \
		&& ([ -f Makefile.in ] || ./autogen.sh)
ib-libibmad.%: LIVE_CONFIGURE_CMD=cd userspace/management/libibmad \
		&& ([ -f Makefile ] || \
		./configure --prefix=/usr \
		--host=$(CROSS) --build=$(GLUSTER_BUILD))
ib-libibmad.%: LIVE_BUILD_CMD=cd userspace/management/libibmad \
		&& $(DEFAULT_LIVE_BUILD_CMD)
ib-libibmad.%: LIVE_INSTALL_CMD=cd userspace/management/libibmad \
		&& make install DESTDIR=$1

## bundling

ib-libibmad.%: PKG_PREPARE_CMD=cd userspace/management/libibmad \
		&& ([ -f Makefile.in ] || ./autogen.sh)
ib-libibmad.%: PKG_CONFIGURE_CMD=cd userspace/management/libibmad \
		&& ([ -f Makefile ] || \
		./configure --prefix=/opt/gluster  \
		--host=$(CROSS) --build=$(GLUSTER_BUILD))
ib-libibmad.%: PKG_BUILD_CMD=cd userspace/management/libibmad \
		&& $(DEFAULT_PKG_BUILD_CMD)
ib-libibmad.%: PKG_INSTALL_CMD=cd userspace/management/libibmad \
		&& make install DESTDIR=$1
