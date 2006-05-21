
sysvinit.%: PACKAGE_NAME=sysvinit-2.86
sysvinit.%: LIVE_CONFIGURE_CMD=
sysvinit.%: LIVE_BUILD_CMD=cd src; make all $(LIVE_BUILD_ENV) LCRYPT=-lcrypt
sysvinit.%: LIVE_INSTALL_CMD=cd src; make install ROOT=$1
