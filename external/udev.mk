
udev.%: PACKAGE_NAME=udev-050
udev.%: LIVE_PREPARE_CMD=rm -f ./etc/udev/udev.rules; cp ./etc/udev/udev.rules.devfs ./etc/udev/udev.rules
udev.%: LIVE_CONFIGURE_CMD=
udev.%: LIVE_BUILD_CMD=$(DEFAULT_LIVE_BUILD_CMD) $(LIVE_BUILD_ENV) LD=$(CROSS)-gcc KERNEL_DIR=$(abs_top_builddir)/live_kernel_src
udev.%: LIVE_INSTALL_CMD=$(DEFAULT_LIVE_INSTALL_CMD); cp ./extras/*.{sh,conf} $1/etc/udev/; chmod +x $1/etc/udev/*.sh
