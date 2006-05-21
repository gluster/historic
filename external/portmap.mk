portmap.%: PACKAGE_NAME=portmap_4
portmap.%: LIVE_INSTALL_CMD=make install $(LIVE_BUILD_ENV) DESTDIR=$1
