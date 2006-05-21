ib-libmthca.%: PACKAGE_NAME=openib-userspace-svn3640-1
ib-libmthca.%: subdir=libmthca
ib-libmthca.%: LIVE_PREPARE_CMD=cd userspace/$(subdir) \
		&& ([ -f configure ] || ./autogen.sh)
ib-libmthca.%: LIVE_CONFIGURE_CMD=cd userspace/$(subdir) \
		&& ([ -f Makefile ] || \
		./configure --prefix=/usr \
		--host=$(CROSS) --build=$(GLUSTER_BUILD))
ib-libmthca.%: LIVE_BUILD_CMD=cd userspace/$(subdir) \
		&& $(DEFAULT_LIVE_BUILD_CMD)
ib-libmthca.%: LIVE_INSTALL_CMD=cd userspace/$(subdir) \
		&& make install DESTDIR=$1


ib-libmthca.%: PKG_PREPARE_CMD=cd userspace/$(subdir) \
		&& ([ -f configure ] || ./autogen.sh)
ib-libmthca.%: PKG_CONFIGURE_CMD=cd userspace/$(subdir) \
		&& ([ -f Makefile ] || \
		./configure --prefix=/opt/gluster  \
		--host=$(CROSS) --build=$(GLUSTER_BUILD))
ib-libmthca.%: PKG_BUILD_CMD=cd userspace/$(subdir) \
		&& $(DEFAULT_PKG_BUILD_CMD)
ib-libmthca.%: PKG_INSTALL_CMD=cd userspace/$(subdir) \
		&& make install DESTDIR=$1
