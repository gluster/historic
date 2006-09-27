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
guile.%: PACKAGE_NAME=guile-1.8.0
guile.%: LIVE_PREPARE_CMD=[ -f .prep ] || ( aclocal && autoconf && automake && touch .prep )
guile.%: LIVE_CONFIGURE_ENV=CC_FOR_BUILD=gcc $(DEFAULT_LIVE_CONFIGURE_ENV) LIBS=-lm
guile.%: LIVE_CONFIGURE_CMD=$(DEFAULT_LIVE_CONFIGURE_CMD) --with-threads=pthreads --disable-error-on-warning
guile.%: NATIVE_CONFIGURE_ENV=LIBS=-lm
guile.%: NATIVE_CONFIGURE_CMD=./configure --prefix=$(DESTDIR_NATIVE)/usr
guile.%: NATIVE_INSTALL_CMD=make install DESTDIR=
