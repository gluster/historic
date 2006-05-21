iputils.%: PACKAGE_NAME=iputils_20020927
iputils.%: LIVE_BUILD_CMD=make all KERNEL_INCLUDE=$(abs_top_builddir)/live_kernel_include LIBC_INCLUDE=$(shell [ -d $(TOOL_BASE)/$(CROSS)/sys-root/usr/include ] && echo $(TOOL_BASE)/$(CROSS)/sys-root/usr/include || echo $(TOOL_BASE)/$(CROSS)/include ) CC=$(CROSS)-gcc
iputils.%: LIVE_INSTALL_CMD=ls arping clockdiff ping ping6 rarpd rdisc tftpd tracepath tracepath6 traceroute6 | cpio -puvd $1/sbin
