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

zlib.%: PACKAGE_NAME=zlib-1.2.3
zlib.%: LIVE_CONFIGURE_CMD=$(LIVE_BUILD_ENV) AR="$(CROSS)-ar cr" ./configure --shared
zlib.%: LIVE_BUILD_CMD=make all $(LIVE_BUILD_ENV) AR="$(CROSS)-ar cr" CFLAGS="-O -fPIC"
zlib.%: LIVE_INSTALL_CMD=make install prefix=$1/usr exec_prefix=$1/usr

zlib.%: PKG_CONFIGURE_CMD=./configure
zlib.%: PKG_BUILD_CMD=$(LIVE_BUILD_ENV) AR="$(CROSS)-ar cr" make all $(LIVE_BUILD_ENV) AR="$(CROSS)-ar cr" CFLAGS="-O -fPIC"
zlib.%: PKG_INSTALL_CMD=make install prefix=$1/opt/gluster exec_prefix=$1/opt/gluster
