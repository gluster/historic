ib-libibcm.%: PACKAGE_NAME=openib-userspace-svn3640-1
ib-libibcm.%: subdir=libibcm
ib-libibcm.%: LIVE_PREPARE_CMD=cd userspace/$(subdir) \
		&& ([ -f configure ] || ./autogen.sh)
ib-libibcm.%: LIVE_CONFIGURE_ENV=CPPFLAGS=-I$(DESTDIR_LIVE)/usr/include \
		LDFLAGS="-L$(DESTDIR_LIVE)/usr/lib -lsysfs"
ib-libibcm.%: LIVE_CONFIGURE_CMD=cd userspace/$(subdir) \
		&& ([ -f Makefile ] || \
		./configure --prefix=/usr \
		--host=$(CROSS) --build=$(GLUSTER_BUILD))
ib-libibcm.%: LIVE_BUILD_CMD=cd userspace/$(subdir) \
		&& $(DEFAULT_LIVE_BUILD_CMD)
ib-libibcm.%: LIVE_INSTALL_CMD=cd userspace/$(subdir) \
		&& make install DESTDIR=$1

ib-libibcm.%: PKG_PREPARE_CMD=cd userspace/$(subdir) \
		&& ([ -f configure ] || ./autogen.sh)
ib-libibcm.%: PKG_CONFIGURE_ENV=CPPFLAGS=-I$(DESTDIR_PKG)/opt/gluster/include \
		LDFLAGS="-L$(DESTDIR_PKG)/opt/gluster/lib"\
		LIBS="-lsysfs"
ib-libibcm.%: PKG_CONFIGURE_CMD=cd userspace/$(subdir) \
		&& ([ -f Makefile ] || \
		./configure --prefix=/opt/gluster \
		--host=$(CROSS) --build=$(GLUSTER_BUILD))
ib-libibcm.%: PKG_BUILD_CMD=cd userspace/$(subdir) \
		&& $(DEFAULT_PKG_BUILD_CMD)
ib-libibcm.%: PKG_INSTALL_CMD=cd userspace/$(subdir) \
		&& make install DESTDIR=$1
