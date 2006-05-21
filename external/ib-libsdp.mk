ib-libsdp.%: PACKAGE_NAME=openib-userspace-svn3640-1
ib-libsdp.%: subdir=libsdp
ib-libsdp.%: LIVE_PREPARE_CMD=cd userspace/$(subdir) \
		&& ([ -f configure ] || ./autogen.sh)
ib-libsdp.%: LIVE_CONFIGURE_CMD=cd userspace/$(subdir) \
		&& ([ -f Makefile ] || \
		./configure --prefix=/usr \
		--host=$(CROSS) --build=$(GLUSTER_BUILD))
ib-libsdp.%: LIVE_BUILD_CMD=cd userspace/$(subdir) \
		&& $(DEFAULT_LIVE_BUILD_CMD)
ib-libsdp.%: LIVE_INSTALL_CMD=cd userspace/$(subdir) \
		&& make install DESTDIR=$1


## hpc

ib-libsdp.%: PKG_PREPARE_CMD=cd userspace/$(subdir) \
		&& ([ -f configure ] || ./autogen.sh)
ib-libsdp.%: PKG_CONFIGURE_CMD=cd userspace/$(subdir) \
		&& ([ -f Makefile ] || \
		./configure --prefix=/opt/gluster  \
		--host=$(CROSS) --build=$(GLUSTER_BUILD))
ib-libsdp.%: PKG_BUILD_CMD=cd userspace/$(subdir) \
		&& $(DEFAULT_PKG_BUILD_CMD)
ib-libsdp.%: PKG_INSTALL_CMD=cd userspace/$(subdir) \
		&& make install DESTDIR=$1
