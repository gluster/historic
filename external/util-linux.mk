
util-linux.%: PACKAGE_NAME=util-linux-2.13-pre4
util-linux.%: LIVE_CONFIGURE_CMD=$(DEFAULT_LIVE_CONFIGURE_CMD) --disable-schedutils --disable-use-tty-group --enable-partx --disable-cramfs
util-linux.%: LIVE_INSTALL_CMD=DESTDIR= make install prefix=$1 datadir=$1/usr/share
