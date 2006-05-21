
e2fsprogs.%: PACKAGE_NAME=e2fsprogs-1.38
e2fsprogs.%: LIVE_CONFIGURE_CMD=$(DEFAULT_LIVE_CONFIGURE_CMD) \
		--enable-elf-shlibs --disable-rpath
e2fsprogs.%: LIVE_INSTALL_CMD=make install install-libs DESTDIR=$1
