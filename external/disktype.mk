
disktype.%: PACKAGE_NAME=disktype-6
disktype.%: LIVE_CONFIGURE_CMD=
disktype.%: LIVE_BUILD_CMD=make all $(LIVE_BUILD_ENV)
disktype.%: LIVE_INSTALL_CMD=ls disktype | cpio -puvd $1/usr/bin
