ib-libosmcomp.%: PACKAGE_NAME=openib-userspace-svn3640-1
ib-libosmcomp.%: subdir=osm/complib
ib-libosmcomp.%: LIVE_PREPARE_CMD=cd userspace/management/$(subdir) \
		&& ([ -f configure ] || ./autogen.sh)
ib-libosmcomp.%: LIVE_CONFIGURE_CMD=cd userspace/management/$(subdir) \
		&& ([ -f Makefile ] || \
		./configure --prefix=/usr \
		--host=$(CROSS) --build=$(GLUSTER_BUILD))
ib-libosmcomp.%: LIVE_BUILD_CMD=cd userspace/management/$(subdir) \
		&& $(DEFAULT_LIVE_BUILD_CMD)
ib-libosmcomp.%: LIVE_INSTALL_CMD=cd userspace/management/$(subdir) \
		&& make install DESTDIR=$1

### hpc bundling

ib-libosmcomp.%: PKG_PREPARE_CMD=cd userspace/management/$(subdir) \
		&& ([ -f configure ] || ./autogen.sh)
ib-libosmcomp.%: PKG_CONFIGURE_CMD=cd userspace/management/$(subdir) \
		&& ([ -f Makefile ] || \
		./configure --prefix=/opt/gluster  \
		--host=$(CROSS) --build=$(GLUSTER_BUILD))
ib-libosmcomp.%: PKG_BUILD_CMD=cd userspace/management/$(subdir) \
		&& $(DEFAULT_PKG_BUILD_CMD)
ib-libosmcomp.%: PKG_INSTALL_CMD=cd userspace/management/$(subdir) \
		&& make install DESTDIR=$1
