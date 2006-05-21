
zlib.%: PACKAGE_NAME=zlib-1.2.3
zlib.%: LIVE_CONFIGURE_CMD=./configure --shared
zlib.%: LIVE_BUILD_CMD=make all $(LIVE_BUILD_ENV) AR="$(CROSS)-ar cr" CFLAGS="-O -fPIC"
zlib.%: LIVE_INSTALL_CMD=make install prefix=$1/usr exec_prefix=$1/usr

zlib.%: PKG_CONFIGURE_CMD=./configure
zlib.%: PKG_BUILD_CMD=make all $(LIVE_BUILD_ENV) AR="$(CROSS)-ar cr" CFLAGS="-O -fPIC"
zlib.%: PKG_INSTALL_CMD=make install prefix=$1/opt/gluster exec_prefix=$1/opt/gluster
