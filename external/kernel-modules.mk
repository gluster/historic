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
kernel-modules.%: PACKAGE_NAME=linux-2.6.21.4
kernel-modules.%: LIVE_CONFIGURE_CMD=
kernel-modules.%: LIVE_BUILD_CMD=make -j 8 modules CROSS_COMPILE=$(CROSS)- ARCH=$(KERNEL_ARCH)
kernel-modules.%: LIVE_INSTALL_CMD=make modules_install INSTALL_MOD_PATH=$1 ARCH=$(KERNEL_ARCH); find $1/lib/modules -name '*.ko' -exec gzip -9vf {} \;

