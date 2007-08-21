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
busybox.%: PACKAGE_NAME=busybox-1.00
busybox.%: LIVE_PREPARE_CMD=cp $(PATCHES_DIR)/$(PACKAGE_NAME)/config-gluster .config
busybox.%: LIVE_CONFIGURE_CMD=
busybox.%: LIVE_BUILD_CMD=make all $(LIVE_BUILD_ENV)
busybox.%: LIVE_INSTALL_CMD=make install PREFIX=$1
