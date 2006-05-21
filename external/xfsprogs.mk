
xfsprogs.%: PACKAGE_NAME=xfsprogs-2.7.11
xfsprogs.%: LIVE_PREPARE_CMD=$(LIVE_CONFIGURE_ENV) $(DEFAULT_LIVE_CONFIGURE_CMD)
xfsprogs.%: LIVE_BUILD_CMD=make
xfsprogs.%: LIVE_INSTALL_CMD=make install prefix=$1/usr

