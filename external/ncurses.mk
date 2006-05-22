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
ncurses.%: PACKAGE_NAME=ncurses-5.4
ncurses.%: LIVE_CONFIGURE_CMD=$(DEFAULT_LIVE_CONFIGURE_CMD) --with-shared --without-ada --with-build-cc=gcc; sed -i '/cd man/d' Makefile
ncurses.%: NATIVE_CONFIGURE_CMD=$(DEFAULT_NATIVE_CONFIGURE_CMD) --with-shared --without-ada; sed -i '/cd man/d' Makefile
ncurses.%: PKG_CONFIGURE_CMD=$(DEFAULT_PKG_CONFIGURE_CMD) --with-shared -\-without-ada; sed -i '/cd man/d' Makefile
ncurses.%: PKG_INSTALL_CMD=$(DEFAULT_PKG_INSTALL_CMD); cd $1/opt/gluster/include && ln -sf ncurses/ncurses.h && ln -sf ncurses/curses.h
