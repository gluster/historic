
tcpdump.%: PACKAGE_NAME=tcpdump-3.9.4
tcpdump.%: LIVE_CONFIGURE_CMD=ac_cv_linux_vers=2 $(DEFAULT_LIVE_CONFIGURE_CMD) --oldincludedir=$(DESTDIR_LIVE)/usr/include
tcpdump.%: LIVE_BUILD_CMD=$(DEFAULT_LIVE_BUILD_CMD) LDFLAGS=-L$(DESTDIR_LIVE)/usr/lib
