##
## Copyright (C) 2007 Z RESEARCH <http://www.zresearch.com>
##  
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##  
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##  
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
##  
##

rds-tools.%: PACKAGE_NAME="rds-tools-1.0-20"
rds-tools.%: PKG_CONFIGURE_CMD=
rds-tools.%: PKG_BUILD_CMD=make
rds-tools.%: PKG_INSTALL_CMD=mkdir -p $1/opt/gluster/ofed/bin $1/opt/gluster/ofed/man/man1; install -m 755 rds-info rds-gen rds-sink rds-stress $1/opt/gluster/ofed/bin; install -m 755 rds-info.1 rds-gen.1 rds-sink.1 rds-stress.1 $1/opt/gluster/ofed/man/man1
