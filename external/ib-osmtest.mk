ib-osmtest.%: PACKAGE_NAME=openib-userspace-svn3640-1
ib-osmtest.%: subdir=osm/osmtest
ib-osmtest.%: LIVE_PREPARE_CMD=cd userspace/management/$(subdir) \
		&& ([ -f configure ] || ./autogen.sh)
ib-osmtest.%: LIVE_CONFIGURE_ENV=LDFLAGS="-L$(DESTDIR_LIVE)/usr/lib -libcommon" \
		CPPFLAGS="-I$(DESTDIR_LIVE)/usr/include"
ib-osmtest.%: LIVE_CONFIGURE_CMD=cd userspace/management/$(subdir) \
		&& ([ -f Makefile ] || \
		./configure --prefix=/usr \
		--host=$(CROSS) --build=$(GLUSTER_BUILD))
ib-osmtest.%: LIVE_BUILD_CMD=cd userspace/management/$(subdir) \
		&& $(DEFAULT_LIVE_BUILD_CMD)
ib-osmtest.%: LIVE_INSTALL_CMD=cd userspace/management/$(subdir) \
		&& make install DESTDIR=$1

### hpc bundle

ib-osmtest.%: PKG_PREPARE_CMD=cd userspace/management/$(subdir) \
		&& ([ -f configure ] || ./autogen.sh)
ib-osmtest.%: PKG_CONFIGURE_ENV=LDFLAGS="-L$(DESTDIR_PKG)/opt/gluster/lib" \
		LIBS="-libcommon" \
		CPPFLAGS="-I$(DESTDIR_PKG)/opt/gluster/include"
ib-osmtest.%: PKG_CONFIGURE_CMD=cd userspace/management/$(subdir) \
		&& ([ -f Makefile ] || \
		./configure --prefix=/opt/gluster  \
		--host=$(CROSS) --build=$(GLUSTER_BUILD))
ib-osmtest.%: PKG_BUILD_CMD=cd userspace/management/$(subdir) \
		&& $(DEFAULT_PKG_BUILD_CMD)
ib-osmtest.%: PKG_INSTALL_CMD=cd userspace/management/$(subdir) \
		&& make install DESTDIR=$1
