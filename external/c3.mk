# Copyright (C) 2006 Z RESEARCH <http://www.zresearch.com>
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

c3.%: PACKAGE_NAME="c3-4.0.1"
c3.%: PKG_CONFIGURE_CMD=true
c3.%: PKG_BUILD_CMD=true
c3.%: C3_TOOLS=cexec cexecs cget ckill ckillnode cpush crm cname cnum clist cpushimage cshutdown
c3.%: C3_LIBS=c3_file_obj.py c3_com_obj.py c3_except.py c3_sock.py
c3.%: C3_PFX=/opt/gluster/c3
c3.%: PKG_INSTALL_CMD=mkdir -p $1/$(C3_PFX); cp $(C3_TOOLS) $(C3_LIBS) $1/$(C3_PFX)
