tcp-wrappers.%: PACKAGE_NAME=tcp_wrappers_7.6
tcp-wrappers.%: LIVE_INSTALL_CMD=make install INSTPREFIX=$(DESTDIR_LIVE) $(LIVE_BUILD_ENV)
