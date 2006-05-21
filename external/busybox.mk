busybox.%: PACKAGE_NAME=busybox-1.00
busybox.%: LIVE_PREPARE_CMD=cp $(PATCHES_DIR)/$(PACKAGE_NAME)/config-gluster .config
busybox.%: LIVE_BUILD_CMD=make all $(LIVE_BUILD_ENV)
busybox.%: LIVE_INSTALL_CMD=make install PREFIX=$1
