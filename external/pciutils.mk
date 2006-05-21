pciutils.%: PACKAGE_NAME=pciutils-2.1.11
pciutils.%: LIVE_BUILD_CMD=make all $(LIVE_BUILD_ENV) PREFIX=/usr
pciutils.%: LIVE_INSTALL_CMD=make install PREFIX=$1/usr; mkdir -p $1/usr/include/pci && cp lib/pci.h lib/header.h lib/config.h $1/usr/include/pci ; mkdir -p $1/usr/lib && cp lib/libpci.a $1/usr/lib

pciutils.%: PKG_BUILD_CMD=make all $(PKG_BUILD_ENV) PREFIX=/opt/gluster
pciutils.%: PKG_INSTALL_CMD=mkdir -p $1/opt/gluster/include/pci && cp lib/pci.h lib/header.h lib/config.h $1/opt/gluster/include/pci ; mkdir -p $1/opt/gluster/lib && cp lib/libpci.a $1/opt/gluster/lib
