kernel-prepare.%: PACKAGE_NAME=linux-2.6.15
kernel-prepare.%: LIVE_PREPARE_CMD=cp $(PATCHES_DIR)/$(PACKAGE_NAME)/config-$(KERNEL_ARCH) .config
kernel-prepare.%: LIVE_BUILD_CMD=make prepare-all modules_prepare CROSS_COMPILE=$(CROSS)- ARCH=$(KERNEL_ARCH)
kernel-prepare.%: LIVE_INSTALL_CMD=ln -sf build_live/$(PACKAGE_NAME)/include $(abs_top_builddir)/live_kernel_include; ln -sf build_live/$(PACKAGE_NAME) $(abs_top_builddir)/live_kernel_src

clean.kernel-prepare:
	rm -f $(top_builddir)/live_kernel_include
	rm -f $(top_builddir)/live_kernel_src

clean: clean.kernel-prepare

