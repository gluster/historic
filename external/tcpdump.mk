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

tcpdump.%: PACKAGE_NAME=tcpdump-3.9.4
tcpdump.%: LIVE_CONFIGURE_CMD=ac_cv_linux_vers=2 $(DEFAULT_LIVE_CONFIGURE_CMD) --oldincludedir=$(DESTDIR_LIVE)/usr/include
tcpdump.%: LIVE_BUILD_CMD=$(DEFAULT_LIVE_BUILD_CMD) LDFLAGS=-L$(DESTDIR_LIVE)/usr/lib
