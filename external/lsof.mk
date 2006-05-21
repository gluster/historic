
lsof.%: PACKAGE_NAME=lsof-4.76
lsof.%: LIVE_CONFIGURE_CMD=touch .neverInv; touch .neverCust; LSOF_ARCH=linux LSOF_CC=$(CC) LSOF_AR="$(AR) cr" LSOF_LD=$(LD) LSOF_RANLIB=$(RANLIB) ./Configure linux
lsof.%: LIVE_INSTALL_CMD=ls lsof | cpio -puvd $1/usr/bin

