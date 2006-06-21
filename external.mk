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
#APPS_STAGE1 = grub syslinux elilo

APPS_RAMDISK = busybox kernel-modules termcap pcre ncurses dialog readline\
	bash hotplug udev iproute2 udpcast jed slang\
	bzip2 gzip tar zlib openssl openssh sed util-linux sysvinit atftp\
	findutils grep gawk dhcp file pcap tcpdump lsof screen\
	coreutils net-tools iputils pciutils libusb usbutils expat \
	discover discover-data nfs-utils tcp-wrappers portmap \
	module-init-tools

APPS_IPMI = libgpg-error libgcrypt gmp libtool guile freeipmi
APPS_OPENIB = ib-prepare ib-libibcommon ib-libibumad \
        ib-libibmad ib-libosmcomp ib-libosmvendor \
        ib-osm ib-osminclude ib-osmtest sysfs \
        ib-libibverbs ib-libmthca ib-libibat \
        ib-libibcm ib-libsdp ib-tools


APPS_EXTENSIONS = python dhcp-helper makebootfat reiserfsprogs device-mapper lvm2 \
	xfsprogs e2fsprogs jfsutils netcat parted shadow iptables gdb
#	$(APPS_IPMI) $(APPS_OPENIB)

APPS_PKG = mpich2 $(APPS_OPENIB) mvapich-gen2 torque slurm pdsh c3 cerebro \
	genders lam conman freetype2 libart_lgpl libpng rrdtool ganglia

APPS_BUILD = cdrtools genext2fs kernel-prepare kernel-bzimage

APPS_ALL = $(APPS_EXTENSIONS) $(APPS_RAMDISK) $(APPS_BUILD) $(APPS_PKG)

## default values, which each .mk will override
### native cmd
$(APPS_ALL:%=%.%):NATIVE_PREPARE_CMD=$(DEFAULT_NATIVE_PREPARE_CMD)
$(APPS_ALL:%=%.%):NATIVE_CONFIGURE_CMD=$(DEFAULT_NATIVE_CONFIGURE_CMD)
$(APPS_ALL:%=%.%):NATIVE_BUILD_CMD=$(DEFAULT_NATIVE_BUILD_CMD)
$(APPS_ALL:%=%.%):NATIVE_INSTALL_CMD=$(DEFAULT_NATIVE_INSTALL_CMD)
### native env
$(APPS_ALL:%=%.%):NATIVE_PREPARE_ENV=$(DEFAULT_NATIVE_PREPARE_ENV)
$(APPS_ALL:%=%.%):NATIVE_CONFIGURE_ENV=$(DEFAULT_NATIVE_CONFIGURE_ENV)
$(APPS_ALL:%=%.%):NATIVE_BUILD_ENV=$(DEFAULT_NATIVE_BUILD_ENV)
$(APPS_ALL:%=%.%):NATIVE_INSTALL_ENV=$(DEFAULT_NATIVE_INSTALL_ENV)
### cross cmd
$(APPS_ALL:%=%.%):LIVE_PREPARE_CMD=$(DEFAULT_LIVE_PREPARE_CMD)
$(APPS_ALL:%=%.%):LIVE_CONFIGURE_CMD=$(DEFAULT_LIVE_CONFIGURE_CMD)
$(APPS_ALL:%=%.%):LIVE_BUILD_CMD=$(DEFAULT_LIVE_BUILD_CMD)
$(APPS_ALL:%=%.%):LIVE_INSTALL_CMD=$(DEFAULT_LIVE_INSTALL_CMD)
### cross env
$(APPS_ALL:%=%.%):LIVE_PREPARE_ENV=$(DEFAULT_LIVE_PREPARE_ENV)
$(APPS_ALL:%=%.%):LIVE_CONFIGURE_ENV=$(DEFAULT_LIVE_CONFIGURE_ENV)
$(APPS_ALL:%=%.%):LIVE_BUILD_ENV=$(DEFAULT_LIVE_BUILD_ENV)
$(APPS_ALL:%=%.%):LIVE_INSTALL_ENV=$(DEFAULT_LIVE_INSTALL_ENV)
### pkg cmd
$(APPS_ALL:%=%.%):PKG_PREPARE_CMD=$(DEFAULT_PKG_PREPARE_CMD)
$(APPS_ALL:%=%.%):PKG_CONFIGURE_CMD=$(DEFAULT_PKG_CONFIGURE_CMD)
$(APPS_ALL:%=%.%):PKG_BUILD_CMD=$(DEFAULT_PKG_BUILD_CMD)
$(APPS_ALL:%=%.%):PKG_INSTALL_CMD=$(DEFAULT_PKG_INSTALL_CMD)
### pkg env
$(APPS_ALL:%=%.%):PKG_PREPARE_ENV=$(DEFAULT_PKG_PREPARE_ENV)
$(APPS_ALL:%=%.%):PKG_CONFIGURE_ENV=$(DEFAULT_PKG_CONFIGURE_ENV)
$(APPS_ALL:%=%.%):PKG_BUILD_ENV=$(DEFAULT_PKG_BUILD_ENV)
$(APPS_ALL:%=%.%):PKG_INSTALL_ENV=$(DEFAULT_PKG_INSTALL_ENV)


# external includes to override
include $(APPS_ALL:%=$(top_srcdir)/external/%.mk)


define do-build
$(MAKE) $(APP_DIR) PACKAGE_NAME=$(PACKAGE_NAME)

cd $(APP_DIR) ; \
	export $(PREPARE_ENV) ; \
	$(call PREPARE_CMD)

cd $(APP_DIR) ; \
	export $(CONFIGURE_ENV) ; \
	[ -f Makefile ] || FOOBAR=FUBAR $(call CONFIGURE_CMD)

cd $(APP_DIR) ; \
        touch libtool ; sed -i -r $$(for line in `cat /etc/ld.so.conf | grep -v '\^[ \t]*#' ; echo /lib /usr/lib /lib64 /usr/lib64` ; do echo ' -e' 's,([^a-zA-Z0-9_\.])'$$line'([^a-zA-Z0-9_\.]),\1\2,' ; done) `find . -name libtool`

cd $(APP_DIR) ; \
	export $(BUILD_ENV) ; \
	$(call BUILD_CMD)

cd $(APP_DIR) ; \
	export $(INSTALL_ENV) ; \
	$(call INSTALL_CMD,$(INSTALL_DIR))

find $(INSTALL_DIR) -name '*.la' -exec rm -f {} \;
endef 

$(APPS_ALL:%=%.native): $(DESTDIR_NATIVE)
$(APPS_ALL:%=%.native): $(BUILD_NATIVE)
$(APPS_ALL:%=%.native): APP_DIR=$(BUILD_NATIVE)/$(PACKAGE_NAME)
$(APPS_ALL:%=%.native): PREPARE_ENV=$(NATIVE_PREPARE_ENV)
$(APPS_ALL:%=%.native): PREPARE_CMD=$(NATIVE_PREPARE_CMD)
$(APPS_ALL:%=%.native): CONFIGURE_ENV=$(NATIVE_CONFIGURE_ENV)
$(APPS_ALL:%=%.native): CONFIGURE_CMD=$(NATIVE_CONFIGURE_CMD)
$(APPS_ALL:%=%.native): BUILD_ENV=$(NATIVE_BUILD_ENV)
$(APPS_ALL:%=%.native): BUILD_CMD=$(NATIVE_BUILD_CMD)
$(APPS_ALL:%=%.native): INSTALL_ENV=$(NATIVE_INSTALL_ENV)
$(APPS_ALL:%=%.native): INSTALL_CMD=$(NATIVE_INSTALL_CMD)
$(APPS_ALL:%=%.native): INSTALL_DIR=$(DESTDIR_NATIVE)
$(APPS_ALL:%=%.native):
	$(do-build)

$(APPS_ALL:%=%.live): $(DESTDIR_LIVE)
$(APPS_ALL:%=%.live): $(BUILD_LIVE)
$(APPS_ALL:%=%.live): APP_DIR=$(BUILD_LIVE)/$(PACKAGE_NAME)
$(APPS_ALL:%=%.live): PREPARE_ENV=$(LIVE_PREPARE_ENV)
$(APPS_ALL:%=%.live): PREPARE_CMD=$(LIVE_PREPARE_CMD)
$(APPS_ALL:%=%.live): CONFIGURE_ENV=$(LIVE_CONFIGURE_ENV)
$(APPS_ALL:%=%.live): CONFIGURE_CMD=$(LIVE_CONFIGURE_CMD)
$(APPS_ALL:%=%.live): BUILD_ENV=$(LIVE_BUILD_ENV)
$(APPS_ALL:%=%.live): BUILD_CMD=$(LIVE_BUILD_CMD)
$(APPS_ALL:%=%.live): INSTALL_ENV=$(LIVE_INSTALL_ENV)
$(APPS_ALL:%=%.live): INSTALL_CMD=$(LIVE_INSTALL_CMD)
$(APPS_ALL:%=%.live): INSTALL_DIR=$(DESTDIR_LIVE)
$(APPS_ALL:%=%.live):
	$(do-build)

$(APPS_ALL:%=%.pkg): $(DESTDIR_PKG) $(DESTDIR_DIST)
$(APPS_ALL:%=%.pkg): $(BUILD_PKG)
$(APPS_ALL:%=%.pkg): APP_DIR=$(BUILD_PKG)/$(PACKAGE_NAME)
$(APPS_ALL:%=%.pkg): PREPARE_ENV=$(PKG_PREPARE_ENV)
$(APPS_ALL:%=%.pkg): PREPARE_CMD=$(PKG_PREPARE_CMD)
$(APPS_ALL:%=%.pkg): CONFIGURE_ENV=$(PKG_CONFIGURE_ENV)
$(APPS_ALL:%=%.pkg): CONFIGURE_CMD=$(PKG_CONFIGURE_CMD)
$(APPS_ALL:%=%.pkg): BUILD_ENV=$(PKG_BUILD_ENV)
$(APPS_ALL:%=%.pkg): BUILD_CMD=$(PKG_BUILD_CMD)
$(APPS_ALL:%=%.pkg): INSTALL_ENV=$(PKG_INSTALL_ENV)
$(APPS_ALL:%=%.pkg): INSTALL_CMD=$(PKG_INSTALL_CMD)
$(APPS_ALL:%=%.pkg): INSTALL_DIR=$(DESTDIR_PKG)
$(APPS_ALL:%=%.pkg):
	$(do-build)

%.dist: %.pkg
	cd $(BUILD_PKG)/$(PACKAGE_NAME) ; \
		$(call PKG_INSTALL_CMD,$(DESTDIR_DIST)/$(PACKAGE_NAME))
	tar -czf $(DESTDIR_DIST)/$(PACKAGE_NAME).tgz -C \
		$(DESTDIR_DIST)/$(PACKAGE_NAME) .
	mkdir -p $(DESTDIR_STAGE3)/$(EXTENSION)
	cp $(DESTDIR_DIST)/$(PACKAGE_NAME).tgz $(DESTDIR_STAGE3)/$(EXTENSION)

$(BUILD_NATIVE)/% $(BUILD_LIVE)/% $(BUILD_PKG)/%: $(TARBALLS_DIR)/%.tar.gz
	tar -C $(shell dirname $@) -xzvf $^
	$(call PATCH_CMD,$(PACKAGE_NAME),$@)
	@touch $@

$(DESTDIR_NATIVE) $(DESTDIR_LIVE) $(DESTDIR_PKG) $(DESTDIR_DIST):
	mkdir -p $@

$(BUILD_NATIVE) $(BUILD_LIVE) $(BUILD_PKG):
	mkdir -p $@
	@touch $@

.PRECIOUS: $(TARBALLS_DIR)/%.tar.gz
$(TARBALLS_DIR)/%.tar.gz:
	wget -P$(TARBALLS_DIR) \
		$(TARBALLS_URL)/$(@:$(TARBALLS_DIR)/%=%)

clean.build:
	rm -rf $(DESTDIR_LIVE) $(DESTDIR_NATIVE) $(DESTDIR_PKG) $(DESTDIR_DIST)
	rm -rf $(BUILD_LIVE) $(BUILD_NATIVE) $(BUILD_PKG)

clean: clean.build
