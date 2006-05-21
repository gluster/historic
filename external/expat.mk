expat.%: PACKAGE_NAME=expat-1.95.8
expat.%: LIVE_CONFIGURE_CMD=$(DEFAULT_LIVE_CONFIGURE_CMD) --with-shared
expat.%: LIVE_INSTALL_CMD=make install prefix=$1/usr
expat.%: NATIVE_CONFIGURE_CMD=./configure --prefix=/usr --with-shared
