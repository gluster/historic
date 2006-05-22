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
ib-libibmad.%: PACKAGE_NAME=openib-userspace-svn3640-1
ib-libibmad.%: LIVE_PREPARE_CMD=cd userspace/management/libibmad \
		&& ([ -f Makefile.in ] || ./autogen.sh)
ib-libibmad.%: LIVE_CONFIGURE_CMD=cd userspace/management/libibmad \
		&& ([ -f Makefile ] || \
		./configure --prefix=/usr \
		--host=$(CROSS) --build=$(GLUSTER_BUILD))
ib-libibmad.%: LIVE_BUILD_CMD=cd userspace/management/libibmad \
		&& $(DEFAULT_LIVE_BUILD_CMD)
ib-libibmad.%: LIVE_INSTALL_CMD=cd userspace/management/libibmad \
		&& make install DESTDIR=$1

## bundling

ib-libibmad.%: PKG_PREPARE_CMD=cd userspace/management/libibmad \
		&& ([ -f Makefile.in ] || ./autogen.sh)
ib-libibmad.%: PKG_CONFIGURE_CMD=cd userspace/management/libibmad \
		&& ([ -f Makefile ] || \
		./configure --prefix=/opt/gluster  \
		--host=$(CROSS) --build=$(GLUSTER_BUILD))
ib-libibmad.%: PKG_BUILD_CMD=cd userspace/management/libibmad \
		&& $(DEFAULT_PKG_BUILD_CMD)
ib-libibmad.%: PKG_INSTALL_CMD=cd userspace/management/libibmad \
		&& make install DESTDIR=$1
