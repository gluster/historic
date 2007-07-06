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

pcre.%: PACKAGE_NAME=pcre-6.3
pcre.%: LIVE_CONFIGURE_CMD=./configure --prefix=/usr \
               --disable-cpp --host=$(CROSS) --build=$(GLUSTER_BUILD)

## NOTE : pcre-config is used by apps which depend on pcre for determining
##  the library and include paths of PCRE. The system's /usr/bin/pcre-config
## will confuse the cross build of grep. Below is a workaround by putting
## a dummy pcre-config into $(DESTDIR_NATIVE)/usr/bin which overrides all 
## other PATHS
## pcre.%: LIVE_INSTALL_CMD=$(DEFAULT_LIVE_INSTALL_CMD); mkdir -p $(DESTDIR_NATIVE)/usr/bin; echo > $(DESTDIR_NATIVE)/usr/bin/pcre-config; chmod +x $(DESTDIR_NATIVE)/usr/bin/pcre-config

pcre.%: LIVE_INSTALL_CMD=make install DESTDIR=$1; \
		ls pcre-config | cpio -puvd $(DESTDIR_NATIVE)/usr/bin ; \
mkdir -p $(DESTDIR_NATIVE)/usr/bin; echo > $(DESTDIR_NATIVE)/usr/bin/pcre-config; chmod +x $(DESTDIR_NATIVE)/usr/bin/pcre-config

