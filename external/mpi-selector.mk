##
## Copyright (C) 2007 Z RESEARCH <http://www.zresearch.com>
##  
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##  
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##  
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
##  
##

mpi-selector.%: PACKAGE_NAME="mpi-selector-1.0.0"
mpi-selector.%: PKG_CONFIGURE_CMD=$(DEFAULT_PKG_CONFIGURE_CMD) --prefix=/opt/gluster/ofed --sysconfdir=/opt/gluster/ofed/etc --datadir=/opt/gluster/ofed/share --includedir=/opt/gluster/ofed/include --libdir=/opt/gluster/ofed/lib --libexecdir=/opt/gluster/ofed/libexec --localstatedir=/opt/gluster/ofed/var --sharedstatedir=/opt/gluster/ofed/com --mandir=/opt/gluster/ofed/man --infodir=/opt/gluster/ofed/info --with-shell-startup-dir=/etc/profile.d
mpi-selector.%: PKG_BUILD_CMD=make
