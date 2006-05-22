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
ib-libosmvendor.%: PACKAGE_NAME=openib-userspace-svn3640-1
ib-libosmvendor.%: subdir=osm/libvendor
ib-libosmvendor.%: LIVE_PREPARE_CMD=cd userspace/management/$(subdir) \
		&& ([ -f configure ] || ./autogen.sh)
ib-libosmvendor.%: LIVE_CONFIGURE_ENV=\
		CPPFLAGS=-I$(DESTDIR_LIVE)/usr/include \
		LDFLAGS="-L$(DESTDIR_LIVE)/usr/lib -libcommon"
ib-libosmvendor.%: LIVE_CONFIGURE_CMD=cd userspace/management/$(subdir) \
		&& ([ -f Makefile ] || \
		./configure --prefix=/usr \
		--host=$(CROSS) --build=$(GLUSTER_BUILD))
ib-libosmvendor.%: LIVE_BUILD_CMD=cd userspace/management/$(subdir) \
		&& $(DEFAULT_LIVE_BUILD_CMD)
ib-libosmvendor.%: LIVE_INSTALL_CMD=cd userspace/management/$(subdir) \
		&& make install  DESTDIR=$1

### hpc bundling

ib-libosmvendor.%: PKG_PREPARE_CMD=cd userspace/management/$(subdir) \
		&& ([ -f configure ] || ./autogen.sh)
ib-libosmvendor.%: PKG_CONFIGURE_ENV=\
		CPPFLAGS=-I$(DESTDIR_PKG)/opt/gluster/include \
		LDFLAGS="-L$(DESTDIR_PKG)/opt/gluster/lib" \
		LIBS="-lsysfs -libcommon -libumad"
ib-libosmvendor.%: PKG_CONFIGURE_CMD=cd userspace/management/$(subdir) \
		&& ([ -f Makefile ] || \
		./configure --prefix=/opt/gluster  \
		--host=$(CROSS) --build=$(GLUSTER_BUILD))
ib-libosmvendor.%: PKG_BUILD_CMD=cd userspace/management/$(subdir) \
		&& $(DEFAULT_PKG_BUILD_CMD)
ib-libosmvendor.%: PKG_INSTALL_CMD=cd userspace/management/$(subdir) \
		&& make install  DESTDIR=$1
