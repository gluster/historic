ib-libibcommon.%: PACKAGE_NAME=openib-userspace-svn3640-1
ib-libibcommon.%: LIVE_PREPARE_CMD=cd userspace/management/libibcommon \
		&& ([ -f configure ] || ./autogen.sh)
ib-libibcommon.%: LIVE_CONFIGURE_CMD=cd userspace/management/libibcommon \
		&& ([ -f Makefile ] || \
		./configure --prefix=/usr \
		--host=$(CROSS) --build=$(GLUSTER_BUILD))
ib-libibcommon.%: LIVE_BUILD_CMD=cd userspace/management/libibcommon \
		&& $(DEFAULT_LIVE_BUILD_CMD)
ib-libibcommon.%: LIVE_INSTALL_CMD=cd userspace/management/libibcommon \
		&& make install DESTDIR=$1

### for hpc bundling

ib-libibcommon.%: PKG_PREPARE_CMD=cd userspace/management/libibcommon \
		&& ([ -f configure ] || ./autogen.sh)
ib-libibcommon.%: PKG_CONFIGURE_CMD=cd userspace/management/libibcommon \
		&& ([ -f Makefile ] || \
		./configure --prefix=/opt/gluster \
		--host=$(CROSS) --build=$(GLUSTER_BUILD))
ib-libibcommon.%: PKG_BUILD_CMD=cd userspace/management/libibcommon \
		&& $(DEFAULT_PKG_BUILD_CMD)
ib-libibcommon.%: PKG_INSTALL_CMD=cd userspace/management/libibcommon \
		&& make install DESTDIR=$1
