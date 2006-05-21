kernel-bzimage.%: PACKAGE_NAME=linux-2.6.15
kernel-bzimage.%: LIVE_BUILD_CMD=make bzImage CROSS_COMPILE=$(CROSS)- ARCH=$(KERNEL_ARCH)
kernel-bzimage.%: LIVE_INSTALL_CMD=mkdir -p $1/boot; cp arch/$(KERNEL_ARCH)/boot/bzImage $1/boot/vmlinuz



