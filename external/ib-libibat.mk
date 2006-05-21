ib-libibat.%: PACKAGE_NAME=openib-userspace-svn3640-1
ib-libibat.%: subdir=libibat
ib-libibat.%: LIVE_PREPARE_CMD=cd userspace/$(subdir) \
		&& ([ -f configure ] || ./autogen.sh)
ib-libibat.%: LIVE_CONFIGURE_CMD=cd userspace/$(subdir) \
		&& ([ -f Makefile ] || \
		./configure --prefix=/usr \
		--host=$(CROSS) --build=$(GLUSTER_BUILD))
ib-libibat.%: LIVE_BUILD_CMD=cd userspace/$(subdir) \
		&& $(DEFAULT_LIVE_BUILD_CMD)
ib-libibat.%: LIVE_INSTALL_CMD=cd userspace/$(subdir) \
		&& make install DESTDIR=$1

## for bundling

ib-libibat.%: PKG_PREPARE_CMD=cd userspace/$(subdir) \
		&& ([ -f configure ] || ./autogen.sh)
ib-libibat.%: PKG_CONFIGURE_CMD=cd userspace/$(subdir) \
		&& ([ -f Makefile ] || \
		./configure --prefix=/opt/gluster \
		--host=$(CROSS) --build=$(GLUSTER_BUILD))
ib-libibat.%: PKG_BUILD_CMD=cd userspace/$(subdir) \
		&& $(DEFAULT_PKG_BUILD_CMD)
ib-libibat.%: PKG_INSTALL_CMD=cd userspace/$(subdir) \
		&& make install DESTDIR=$1
