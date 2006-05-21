


cdrtools.%: PACKAGE_NAME=cdrtools-2.01
cdrtools.%: LIVE_CONFIGURE_CMD=
cdrtools.%: LIVE_INSTALL_CMD=make install INS_BASE=$1/usr
cdrtools.%: LIVE_BUILD_CMD=make all $(LIVE_BUILD_ENV)
cdrtools.%: NATIVE_CONFIGURE_CMD=
cdrtools.%: NATIVE_INSTALL_CMD=make install INS_BASE=$1/usr

