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
ib-prepare.%: PACKAGE_NAME=openib-userspace-svn3640-1
ib-prepare.%: LIVE_PREPARE_CMD=[ -d userspace ] || \
		(cp $(PATCHES_DIR)/$(PACKAGE_NAME)/*.patch . && \
		tar -xzf infiniband-backport-svn3640-userspace.tar.gz && \
		cat *-mthca.patch *gluster.patch | patch -p1 -d userspace)
ib-prepare.%: LIVE_CONFIGURE_CMD=
ib-prepare.%: LIVE_BUILD_CMD=
ib-prepare.%: LIVE_INSTALL_CMD=ls openib-userspace-udev.rules | cpio -puvd $1/etc/udev/rules.d/

ib-prepare.%: PKG_PREPARE_CMD=[ -d userspace ] || \
		(cp $(PATCHES_DIR)/$(PACKAGE_NAME)/*.patch . && \
		tar -xzf infiniband-backport-svn3640-userspace.tar.gz && \
		cat *-mthca.patch *gluster.patch | patch -p1 -d userspace)
ib-prepare.%: PKG_CONFIGURE_CMD=
ib-prepare.%: PKG_BUILD_CMD=
ib-prepare.%: PKG_INSTALL_CMD=ls openib-userspace-udev.rules | cpio -puvd $1/opt/gluster/etc/udev/rules.d/
