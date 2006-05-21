
gzip.%: PACKAGE_NAME=gzip-1.2.4a
gzip.%: LIVE_CONFIGURE_CMD=$(LIVE_BUILD_ENV) ASCPP="$(CPP)" ./configure
gzip.%: LIVE_BUILD_CMD=make all
gzip.%: LIVE_INSTALL_CMD=mkdir -p $1/usr; make install prefix=$1/usr

