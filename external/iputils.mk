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
iputils.%: PACKAGE_NAME=iputils_20020927
iputils.%: LIVE_BUILD_CMD=make all KERNEL_INCLUDE=$(abs_top_builddir)/live_$(ARCH)_kernel_include LIBC_INCLUDE=$(shell [ -d $(TOOL_BASE)/$(CROSS)/sys-root/usr/include ] && echo $(TOOL_BASE)/$(CROSS)/sys-root/usr/include || echo $(TOOL_BASE)/$(CROSS)/include ) CC=$(CROSS)-gcc
iputils.%: LIVE_INSTALL_CMD=ls arping clockdiff ping ping6 rarpd rdisc tftpd tracepath tracepath6 traceroute6 | cpio -puvd $1/sbin
