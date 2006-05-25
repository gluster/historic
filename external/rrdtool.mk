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

rrdtool.%: PACKAGE_NAME=rrdtool-1.2.12
rrdtool.%: PKG_CONFIGURE_ENV=CPPFLAGS="-I$(DESTDIR_PKG)/opt/gluster/include -I$(DESTDIR_PKG)/opt/gluster/include/libart-2.0 -I$(DESTDIR_PKG)/opt/gluster/include/libpng12 -I$(DESTDIR_PKG)/opt/gluster/include/freetype2" LDFLAGS="-L$(DESTDIR_PKG)/opt/gluster/lib"
rrdtool.%: PKG_CONFIGURE_CMD=rd_cv_ieee_works=yes $(DEFAULT_PKG_CONFIGURE_CMD) --disable-shared --with-pic --disable-tcl
rrdtool.%: PKG_BUILD_CMD=make all PERL_MAKE_OPTIONS="CC=$(CROSS)-gcc AR=$(CROSS)-ar FULL_AR=$(CROSS)-ar CCCDLFLAGS=-fPIC LDDLFLAGS=\"-shared -L/opt/gluster/lib\" LDFLAGS=-L/opt/gluster/lib LD=$(CROSS)-gcc"
