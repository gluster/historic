device-mapper.%: PACKAGE_NAME=device-mapper.1.02.07
device-mapper.%: LIVE_CONFIGURE_CMD=$(DEFAULT_LIVE_CONFIGURE_CMD) --disable-selinux
device-mapper.%: LIVE_BUILD_CMD=make DESTDIR=$(DESTDIR_LIVE)
