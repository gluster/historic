dhcp-helper.%: PACKAGE_NAME=dhcp-helper-0.2
dhcp-helper.%: LIVE_BUILD_CMD=make all $(LIVE_BUILD_ENV)
dhcp-helper.%: LIVE_INSTALL_CMD=make install PREFIX=/usr DESTDIR=$1 $(LIVE_BUILD_ENV)
