lvm2.%: PACKAGE_NAME=LVM2.2.02.06
lvm2.%: LIVE_BUILD_CMD=make -j 8 CFLAGS="-I$(DESTDIR_LIVE)/usr/include" LDFLAGS="-L$(DESTDIR_LIVE)/usr/lib -L../lib"
