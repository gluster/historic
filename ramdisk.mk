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

%.ramdisk: %.live
	cd $(BUILD_LIVE)/$(PACKAGE_NAME) ; \
	export $(LIVE_INSTALL_ENV) ; \
	$(call LIVE_INSTALL_CMD,$(DESTDIR_RAMDISK))

ramdisk-template: $(APPS_RAMDISK:%=%.ramdisk)
	mkdir -p $(top_builddir)/$(BUILD_LIVE)/ramdisk
	cd $(top_builddir)/$(BUILD_LIVE)/ramdisk && \
		$(abs_top_srcdir)/ramdisk/configure --prefix=/usr \
		--host=$(CROSS) --build=$(GLUSTER_BUILD)
	$(MAKE) -C $(abs_top_builddir)/$(BUILD_LIVE)/ramdisk all
	$(MAKE) -C $(abs_top_builddir)/$(BUILD_LIVE)/ramdisk \
		install DESTDIR=$(DESTDIR_RAMDISK)

	-if [ -d $(TOOL_BASE)/$(CROSS)/sys-root ] ;\
	then \
		cd $(TOOL_BASE)/$(CROSS)/sys-root && \
		(find . | grep -v '/usr/include' \
			| grep -v '/lib.*\.a$$' \
			| grep -v '/lib.*\.old$$' \
			| grep -v '/man/'  \
			| grep -v '/usr/share/' \
			| grep -v '/usr/info/' \
		| cpio -puvd $(DESTDIR_RAMDISK)); \
	fi
	-cp -a $(TOOL_BASE)/$(CROSS)/lib/{*.so,*.so.*} \
		$(DESTDIR_RAMDISK)/lib;
	-cp -a $(TOOL_BASE)/$(CROSS)/lib64/{*.so,*.so.*} \
			$(DESTDIR_RAMDISK)/lib64;

	cp $(abs_top_srcdir)/ramdisk/linuxrc $(DESTDIR_RAMDISK)
	$(abs_top_srcdir)/cleanup.sh $(DESTDIR_RAMDISK) STRIP=$(CROSS)-strip

initrd.slave: $(GENEXT2FS_NATIVE_IF_YES) ramdisk-template
	rm -rf $(top_builddir)/ramdisk.slave.$(ARCH)
	cp -af $(DESTDIR_RAMDISK) $(top_builddir)/ramdisk.slave.$(ARCH)
	cd $(abs_top_srcdir)/ramdisk/skel-slave && \
		(find . | grep -Ev 'CVS|~|.arch-ids' | \
		cpio -puvd $(abs_top_builddir)/ramdisk.slave.$(ARCH))
	genext2fs -b 65536 -e 0 -D $(abs_top_srcdir)/ramdisk/devices.txt \
		-d $(top_builddir)/ramdisk.slave.$(ARCH) \
		$(top_builddir)/initrd.slave.$(ARCH).img
	mkdir -p $(top_builddir)/iso_fs_$(ARCH)/boot
	gzip -9 < $(top_builddir)/initrd.slave.$(ARCH).img > \
		$(top_builddir)/iso_fs_$(ARCH)/boot/initrd.slave.$(ARCH).gz

initrd.master: $(GENEXT2FS_NATIVE_IF_YES) ramdisk-template
	mkdir -p $(top_builddir)/ramdisk.master.$(ARCH)
	cp -af $(DESTDIR_RAMDISK)/* $(top_builddir)/ramdisk.master.$(ARCH)
	cd $(abs_top_srcdir)/ramdisk/skel-master && \
		(find . | grep -Ev 'CVS|~|.arch-ids' | \
		cpio -puvd $(abs_top_builddir)/ramdisk.master.$(ARCH))
	genext2fs -b 65536 -e 0 -D $(abs_top_srcdir)/ramdisk/devices.txt \
		-d $(top_builddir)/ramdisk.master.$(ARCH) \
		$(top_builddir)/initrd.master.$(ARCH).img
	mkdir -p $(top_builddir)/iso_fs_$(ARCH)/boot
	gzip -9 < $(top_builddir)/initrd.master.$(ARCH).img > \
		$(top_builddir)/iso_fs/boot/initrd.master.$(ARCH).gz


initrds: initrd.slave initrd.master

clean.ramdisk:
	rm -rf $(top_builddir)/{initrd.slave.*,initrd.master.*,ramdisk.slave.*,ramdisk.master.*}
	rm -rf $(DESTDIR_RAMDISK)

clean: clean.ramdisk

