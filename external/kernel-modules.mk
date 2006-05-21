kernel-modules.%: PACKAGE_NAME=linux-2.6.15
kernel-modules.%: LIVE_CONFIGURE_CMD=
kernel-modules.%: LIVE_BUILD_CMD=make modules CROSS_COMPILE=$(CROSS)- ARCH=$(KERNEL_ARCH)
kernel-modules.%: LIVE_INSTALL_CMD=make modules_install INSTALL_MOD_PATH=$1; find $1/lib/modules -name '*.ko' -exec gzip -9vf {} \;

