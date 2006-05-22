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
ib-libibumad.%: PACKAGE_NAME=openib-userspace-svn3640-1
ib-libibumad.%: LIVE_PREPARE_CMD=cd userspace/management/libibumad \
		&& ([ -f configure ] || ./autogen.sh)
ib-libibumad.%: LIVE_CONFIGURE_ENV=LDFLAGS="-L$(DESTDIR_LIVE)/usr/lib -libcommon" \
		CPPFLAGS="-I$(DESTDIR_LIVE)/usr/include"
ib-libibumad.%: LIVE_CONFIGURE_CMD=cd userspace/management/libibumad \
		&& ([ -f Makefile ] || \
		./configure --prefix=/usr \
		--host=$(CROSS) --build=$(GLUSTER_BUILD))
ib-libibumad.%: LIVE_BUILD_CMD=cd userspace/management/libibumad \
		&& $(DEFAULT_LIVE_BUILD_CMD)
ib-libibumad.%: LIVE_INSTALL_CMD=cd userspace/management/libibumad \
		&& make install DESTDIR=$1

#### 

ib-libibumad.%: PKG_PREPARE_CMD=cd userspace/management/libibumad \
		&& ([ -f configure ] || ./autogen.sh)
ib-libibumad.%: PKG_CONFIGURE_ENV=LDFLAGS="-L$(DESTDIR_PKG)/opt/gluster/lib" \
		LIBS="-libcommon -lsysfs" \
		CPPFLAGS="-I$(DESTDIR_PKG)/opt/gluster/include"
ib-libibumad.%: PKG_CONFIGURE_CMD=cd userspace/management/libibumad \
		&& ([ -f Makefile ] || \
		./configure --prefix=/opt/gluster  \
		--host=$(CROSS) --build=$(GLUSTER_BUILD))
ib-libibumad.%: PKG_BUILD_CMD=cd userspace/management/libibumad \
		&& $(DEFAULT_PKG_BUILD_CMD)
ib-libibumad.%: PKG_INSTALL_CMD=cd userspace/management/libibumad \
		&& make install DESTDIR=$1
