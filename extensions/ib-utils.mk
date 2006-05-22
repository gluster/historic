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
ib-utils.gex: EXTENSION=ib-utils
ib-utils.gex: ib-prepare.stage3 ib-libibcommon.stage3 ib-libibumad.stage3 \
	ib-libibmad.stage3 ib-libosmcomp.stage3 ib-libosmvendor.stage3 \
	ib-osm.stage3 ib-osminclude.stage3 ib-osmtest.stage3 sysfs.stage3 \
	ib-libibverbs.stage3 ib-libmthca.stage3 ib-libibat.stage3 \
	ib-libibcm.stage3 ib-libsdp.stage3 ib-tools.stage3 
