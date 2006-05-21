ib-osm.%: PACKAGE_NAME=openib-userspace-svn3640-1
ib-osm.%: subdir=osm/opensm
ib-osm.%: LIVE_PREPARE_CMD=cd userspace/management/$(subdir) \
		&& ([ -f configure ] || ./autogen.sh)
ib-osm.%: LIVE_CONFIGURE_CMD=cd userspace/management/$(subdir) \
		&& ([ -f Makefile ] || \
		./configure --prefix=/usr \
		--host=$(CROSS) --build=$(GLUSTER_BUILD))
ib-osm.%: LIVE_CONFIGURE_ENV=LDFLAGS="-L$(DESTDIR_LIVE)/usr/lib -libcommon" \
		CPPFLAGS="-I$(DESTDIR_LIVE)/usr/include"
ib-osm.%: LIVE_BUILD_CMD=cd userspace/management/$(subdir) \
		&& $(DEFAULT_LIVE_BUILD_CMD)
ib-osm.%: LIVE_INSTALL_CMD=cd userspace/management/$(subdir) \
		&& make install libdir=/usr/lib DESTDIR=$1

#### hpc bundling

ib-osm.%: PKG_PREPARE_CMD=cd userspace/management/$(subdir) \
		&& ([ -f configure ] || ./autogen.sh)
ib-osm.%: PKG_CONFIGURE_CMD=cd userspace/management/$(subdir) \
		&& ([ -f Makefile ] || \
		./configure --prefix=/opt/gluster  \
		--host=$(CROSS) --build=$(GLUSTER_BUILD))
ib-osm.%: PKG_CONFIGURE_ENV=LDFLAGS="-L$(DESTDIR_PKG)/opt/gluster/lib" \
		LIBS="-libcommon -lsysfs -libumad" \
		CPPFLAGS="-I$(DESTDIR_PKG)/opt/gluster/include"
ib-osm.%: PKG_BUILD_CMD=cd userspace/management/$(subdir) \
		&& $(DEFAULT_PKG_BUILD_CMD)
ib-osm.%: PKG_INSTALL_CMD=cd userspace/management/$(subdir) \
		&& make install libdir=/opt/gluster/lib DESTDIR=$1
