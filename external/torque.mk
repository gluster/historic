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
torque.%: PACKAGE_NAME=torque-2.0.0p8
torque.%: PKG_CONFIGURE_CMD=$(PKG_BUILD_ENV) CFLAGS=-I$(DESTDIR_PKG)/opt/gluster/include LDFLAGS=-L/opt/gluster/lib ./configure --prefix=/opt/gluster --set-default-server=master-node --with-scp
torque.%: PKG_BUILD_CMD=make all $(PKG_CONFIGURE_ENV)
torque.%: PKG_INSTALL_CMD=make install prefix=$1/opt/gluster; ls torque.setup | cpio -puvd $1/opt/gluster/setup
