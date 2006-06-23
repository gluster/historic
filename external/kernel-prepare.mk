# Copyright (C) 2006 Z RESEARCH Inc. <http://www.zresearch.com>
#  
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#  
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#  
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#  
kernel-prepare.%: PACKAGE_NAME=linux-2.6.15
kernel-prepare.%: LIVE_PREPARE_CMD=cp $(PATCHES_DIR)/$(PACKAGE_NAME)/config-$(KERNEL_ARCH) .config
kernel-prepare.%: LIVE_BUILD_CMD=make prepare-all modules_prepare CROSS_COMPILE=$(CROSS)- ARCH=$(KERNEL_ARCH)
kernel-prepare.%: LIVE_INSTALL_CMD=ln -sf build_live/$(PACKAGE_NAME)/include $(abs_top_builddir)/live_$(ARCH)_kernel_include; ln -sf build_live/$(PACKAGE_NAME) $(abs_top_builddir)/live_$(ARCH)_kernel_src

clean.kernel-prepare:
	rm -f $(top_builddir)/live_$(ARCH)_kernel_include
	rm -f $(top_builddir)/live_$(ARCH)_kernel_src

clean: clean.kernel-prepare

