ib-libibverbs.%: PACKAGE_NAME=openib-userspace-svn3640-1
ib-libibverbs.%: subdir=libibverbs
ib-libibverbs.%: LIVE_PREPARE_CMD=cd userspace/$(subdir) \
		&& ([ -f configure ] || ./autogen.sh)
ib-libibverbs.%: LIVE_CONFIGURE_CMD=cd userspace/$(subdir) \
		&& ([ -f Makefile ] || \
		./configure --prefix=/usr  \
		--host=$(CROSS) --build=$(GLUSTER_BUILD))
ib-libibverbs.%: LIVE_BUILD_CMD=cd userspace/$(subdir) \
		&& $(DEFAULT_LIVE_BUILD_CMD)
ib-libibverbs.%: LIVE_INSTALL_CMD=cd userspace/$(subdir) \
		&& make install  DESTDIR=$1

### hpc bundling

ib-libibverbs.%: PKG_PREPARE_CMD=cd userspace/$(subdir) \
		&& ([ -f configure ] || ./autogen.sh)
ib-libibverbs.%: PKG_CONFIGURE_CMD=cd userspace/$(subdir) \
		&& ([ -f Makefile ] || \
		./configure --prefix=/opt/gluster   \
		--host=$(CROSS) --build=$(GLUSTER_BUILD))
ib-libibverbs.%: PKG_BUILD_CMD=cd userspace/$(subdir) \
		&& $(DEFAULT_PKG_BUILD_CMD)
ib-libibverbs.%: PKG_INSTALL_CMD=cd userspace/$(subdir) \
		&& make install  DESTDIR=$1
