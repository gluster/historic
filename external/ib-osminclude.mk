ib-osminclude.%: PACKAGE_NAME=openib-userspace-svn3640-1
ib-osminclude.%: subdir=osm/include
ib-osminclude.%: LIVE_PREPARE_CMD=cd userspace/management/$(subdir) \
		&& ([ -f configure ] || ./autogen.sh)
ib-osminclude.%: LIVE_CONFIGURE_CMD=cd userspace/management/$(subdir) \
		&& ([ -f Makefile ] || \
		./configure --prefix=/usr \
		--host=$(CROSS) --build=$(GLUSTER_BUILD))
ib-osminclude.%: LIVE_BUILD_CMD=cd userspace/management/$(subdir) \
		&& $(DEFAULT_LIVE_BUILD_CMD)
ib-osminclude.%: LIVE_INSTALL_CMD=cd userspace/management/$(subdir) \
		&& make install DESTDIR=$1

### hpc

ib-osminclude.%: PKG_PREPARE_CMD=cd userspace/management/$(subdir) \
		&& ([ -f configure ] || ./autogen.sh)
ib-osminclude.%: PKG_CONFIGURE_CMD=cd userspace/management/$(subdir) \
		&& ([ -f Makefile ] || \
		./configure --prefix=/opt/gluster  \
		--host=$(CROSS) --build=$(GLUSTER_BUILD))
ib-osminclude.%: PKG_BUILD_CMD=cd userspace/management/$(subdir) \
		&& $(DEFAULT_PKG_BUILD_CMD)
ib-osminclude.%: PKG_INSTALL_CMD=cd userspace/management/$(subdir) \
		&& make install DESTDIR=$1
