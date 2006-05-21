discover-data.%: PACKAGE_NAME=discover-data-2.2005.02.13.bin
discover-data.%: LIVE_CONFIGURE_CMD=
discover-data.%: LIVE_BUILD_CMD=
discover-data.%: LIVE_INSTALL_CMD=find . | cpio -puvd $1
discover-data.%: NATIVE_CONFIGURE_CMD=
discover-data.%: NATIVE_BUILD_CMD=
discover-data.%: NATIVE_INSTALL_CMD=find . | cpio -puvd $1
