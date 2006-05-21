
bzip2.%: PACKAGE_NAME=bzip2-1.0.3
bzip2.%: LIVE_CONFIGURE_CMD=
bzip2.%: LIVE_BUILD_CMD=make -f Makefile-libbz2_so all $(LIVE_BUILD_ENV) ; make bzip2 bzip2recover $(LIVE_BUILD_ENV)
bzip2.%: LIVE_INSTALL_CMD=make install PREFIX=$1/usr; cp -a libbz2.so.* $1/usr/lib
