libusb.%: PACKAGE_NAME=libusb-0.1.11
libusb.%: LIVE_CONFIGURE_CMD=./configure --prefix=/usr --host=$(CROSS) --build=$(GLUSTER_BUILD)
libusb.%: LIVE_INSTALL_CMD=make install DESTDIR=$1
