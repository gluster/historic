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

ofa_user.%: PACKAGE_NAME="ofa_user-1.2"
#ofa_user.%: PKG_CONFIGURE_ENV=CPPFLAGS=-I$(DESTDIR_PKG)/opt/gluster/include
ofa_user.%: PKG_CONFIGURE_CMD=./configure --prefix=/opt/gluster/ofed --libdir=/opt/gluster/ofed/lib --with-dapl --with-ipoibtools --with-libcxgb3 --with-libibcm --with-libibcommon --with-libibmad --with-libibumad --with-libibverbs --with-libipathverbs --with-libmthca --with-opensm --with-librdmacm --with-libsdp --with-openib-diags --with-sdpnetstat --with-srptools --with-perftest --sysconfdir=/opt/gluster/ofed/etc --mandir=/opt/gluster/ofed/man
ofa_user.%: PKG_BUILD_CMD=make -j 8 user
ofa_user.%: PKG_INSTALL_CMD=make DESTDIR=$1 install_user
