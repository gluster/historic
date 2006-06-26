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
mvapich-gen2.%: PACKAGE_NAME=mvapich-gen2-1.0-106
ifeq ($(ARCH),x86_64)
mvapich-gen2.%: PKG_CONFIGURE_ENV=$(DEFAULT_PKG_CONFIGURE_ENV) CFLAGS="-D_SMP_ -DLAZY_MEM_UNREGISTER -DRDMA_FAST_PATH -D_X86_64_ -O3 -fno-strict-aliasing -I$(DESTDIR_PKG)/opt/gluster/include" CCFLAGS="$CFLAGS -Wno-deprecated" FFLAGS="-L$(DESTDIR_PKG)/opt/gluster/lib" FC=$(CROSS)-gfortran ac_cv_prog_AR=$(CROSS)-ar pac_cv_cross_compiling=yes
else
mvapich-gen2.%: PKG_CONFIGURE_ENV=$(DEFAULT_PKG_CONFIGURE_ENV) CFLAGS="-D_SMP_ -DLAZY_MEM_UNREGISTER -DRDMA_FAST_PATH -D_IA32_ -O3 -fno-strict-aliasing -I$(DESTDIR_PKG)/opt/gluster/include" CCFLAGS="$CFLAGS -Wno-deprecated" FFLAGS="-L$(DESTDIR_PKG)/opt/gluster/lib" FC=$(CROSS)-gfortran ac_cv_prog_AR=$(CROSS)-ar pac_cv_cross_compiling=yes
endif
mvapich-gen2.%: PKG_CONFIGURE_CMD=./configure --with-device=ch_p4 --with-arch=LINUX -prefix=/opt/gluster/$(PACKAGE_NAME) -lib="-L$(DESTDIR_PKG)/opt/gluster/lib -Wl,-rpath=/opt/gluster/lib -libverbs -lpthread -lsysfs" --without-mpe --enable-f77 --c++=$(CROSS)-g++ --disable-devdebug -cc=$(CROSS)-gcc -fc=$(CROSS)-gfortran -clinker=$(CROSS)-gcc -c++linker=$(CROSS)-g++ -with-cross
mvapich-gen2.%: PKG_BUILD_CMD=make RANLIB=$(CROSS)-ranlib
