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
nfs-utils.%: PACKAGE_NAME=nfs-utils-1.0.8-rc4
#nfs-utils.%: LIVE_PREPARE_CMD=[ -f .prep ] || (sh autogen.sh && touch .prep)
nfs-utils.%: LIVE_CONFIGURE_CMD=knfsd_cv_bsd_signals=yes $(DEFAULT_LIVE_CONFIGURE_CMD) --disable-nfsv4 --disable-gss
