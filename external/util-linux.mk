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

util-linux.%: PACKAGE_NAME=util-linux-2.13-pre4
util-linux.%: LIVE_CONFIGURE_CMD=$(DEFAULT_LIVE_CONFIGURE_CMD) --disable-schedutils --disable-use-tty-group --enable-partx --disable-cramfs
util-linux.%: LIVE_INSTALL_CMD=DESTDIR= make install prefix=$1 datadir=$1/usr/share
