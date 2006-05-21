ib-prepare.%: PACKAGE_NAME=openib-userspace-svn3640-1
ib-prepare.%: LIVE_PREPARE_CMD=[ -d userspace ] || \
		(cp $(PATCHES_DIR)/$(PACKAGE_NAME)/*.patch . && \
		tar -xzf infiniband-backport-svn3640-userspace.tar.gz && \
		cat *-mthca.patch *gluster.patch | patch -p1 -d userspace)
ib-prepare.%: LIVE_CONFIGURE_CMD=
ib-prepare.%: LIVE_BUILD_CMD=
ib-prepare.%: LIVE_INSTALL_CMD=ls openib-userspace-udev.rules | cpio -puvd $1/etc/udev/rules.d/

ib-prepare.%: PKG_PREPARE_CMD=[ -d userspace ] || \
		(cp $(PATCHES_DIR)/$(PACKAGE_NAME)/*.patch . && \
		tar -xzf infiniband-backport-svn3640-userspace.tar.gz && \
		cat *-mthca.patch *gluster.patch | patch -p1 -d userspace)
ib-prepare.%: PKG_CONFIGURE_CMD=
ib-prepare.%: PKG_BUILD_CMD=
ib-prepare.%: PKG_INSTALL_CMD=ls openib-userspace-udev.rules | cpio -puvd $1/opt/gluster/etc/udev/rules.d/
