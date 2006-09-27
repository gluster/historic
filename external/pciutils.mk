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
pciutils.%: PACKAGE_NAME=pciutils-2.1.11
pciutils.%: LIVE_BUILD_CMD=make all $(LIVE_BUILD_ENV) PREFIX=/usr
pciutils.%: LIVE_INSTALL_CMD=make install PREFIX=$1/usr; mkdir -p $1/usr/include/pci && cp lib/pci.h lib/header.h lib/config.h $1/usr/include/pci ; mkdir -p $1/usr/lib && cp lib/libpci.a $1/usr/lib

pciutils.%: PKG_BUILD_CMD=make all $(PKG_BUILD_ENV) PREFIX=/opt/gluster
pciutils.%: PKG_INSTALL_CMD=mkdir -p $1/opt/gluster/include/pci && cp lib/pci.h lib/header.h lib/config.h $1/opt/gluster/include/pci ; mkdir -p $1/opt/gluster/lib && cp lib/libpci.a $1/opt/gluster/lib
