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

kernel-images: kernel-bzimage.live
	mkdir $(top_builddir)/iso_fs/boot -p
	cp $(DESTDIR_LIVE)/boot/vmlinuz \
		$(top_builddir)/iso_fs_$(ARCH)/boot/linux.master.$(ARCH)
	cp $(DESTDIR_LIVE)/boot/vmlinuz \
		$(top_builddir)/iso_fs_$(ARCH)/boot/linux.slave.$(ARCH)
