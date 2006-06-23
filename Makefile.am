# Copyright (C) 2006 Z RESEARCH <http://www.zresearch.com>
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

GENEXT2FS_NATIVE_IF_YES = @GENEXT2FS_NATIVE@

include $(top_srcdir)/header.mk
include $(top_srcdir)/deps.mk
include $(top_srcdir)/external.mk
include $(top_srcdir)/kernel.mk
include $(top_srcdir)/ramdisk.mk
include $(top_srcdir)/boot.mk
#include $(top_srcdir)/stage2/stage2.mk
#include $(top_srcdir)/stage3/stage3.mk
include $(top_srcdir)/extensions.mk


iso: $(CDRTOOLS_NATIVE)
	mkisofs -R -J \
	-o gluster-$(PACKAGE_VERSION)-$(ARCH).iso \
	-b boot/grub/stage2_eltorito -c boot.cat -no-emul-boot \
	-boot-load-size 4 -boot-info-table -V Gluster iso_fs_$(ARCH)

build-all: boot kernel-images initrds extensions iso

all: 
	$(foreach a,$(ARCHS), \
		make build-all ARCH=$(a);)
clean:
	rm -rf iso_fs* gluster-$(PACKAGE_VERSION)-$(ARCH).iso
	rm -rf gluster-sdk-$(PACKAGE_VERSION)-$(ARCH)
	rm -rf gluster-sdk-$(PACKAGE_VERSION)-$(ARCH).tar.gz

deptest:
	make clean
	$(foreach p,$(APPS_ALL),make $(p).cross; make clean;)

SDK_DIR=$(abs_top_builddir)/gluster-sdk-$(PACKAGE_VERSION)-$(ARCH)
sdk: $(APPS_ALL:%=%.live)
	mkdir -p $(SDK_DIR)
	cd $(TOOL_BASE) && \
		(find . | cpio -puvd $(SDK_DIR)/toolchain)
	cd $(DESTDIR_LIVE)/usr && \
		(find ./include ./lib | cpio -puvd $(SDK_DIR)/toolchain/$(CROSS)/sys-root/usr)
	mkdir -p $(SDK_DIR)/sample
	tar -czf gluster-sdk-$(PACKAGE_VERSION)-$(ARCH).tar.gz \
		gluster-sdk-$(PACKAGE_VERSION)-$(ARCH)
