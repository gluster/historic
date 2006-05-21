jove.%: PACKAGE_NAME=jove4.16.0.61
jove.%: LIVE_CONFIGURE_CMD=
jove.%: LIVE_BUILD_CMD=make $(LIVE_BUILD_ENV) $(LIVE_CONFIGURE_ENV) LOCALCC=gcc
jove.%: LIVE_INSTALL_CMD=make install JOVEHOME=$1/usr; ln -sf jove $1/usr/bin/emacs
