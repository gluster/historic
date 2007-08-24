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
mvapich.%: PACKAGE_NAME=mvapich-0.9.9-1326

ifeq ($(ARCH),x86_64)
mvapich.%: PKG_CONFIGURE_ENV=CFLAGS='-O3 -fno-strict-aliasing -g -DCH_GEN2 -DMEMORY_SCALE -D_AFFINITY_ -DCOMPAT_MODE -Wall -D_SMP_ -D_SMP_RNDV_ -DVIADEV_RPUT_SUPPORT -DEARLY_SEND_COMPLETION -DLAZY_MEM_UNREGISTER -D_EM64T_ -I$(DESTDIR_PKG)/opt/gluster/ofed/include' FFLAGS=' -L$(DESTDIR_PKG)/opt/gluster/ofed/lib -Wall' F90FLAGS=' -Wall' MPIRUN_CFLAGS=' -DPARAM_GLOBAL=\"/opt/gluster/ofed/mpi/gcc/$(PACKAGE_NAME)/etc/mvapich.conf\" -DLD_LIBRARY_PATH_MPI=\"/opt/gluster/ofed/mpi/gcc/$(PACKAGE_NAME)/lib/shared\"  -DMVAPICH_BUILDID=\"1326\"' FC=$(CROSS)-gfortran ac_cv_prog_AR=$(CROSS)-ar pac_cv_cross_compiling=yes
else
mvapich.%: PKG_CONFIGURE_ENV=CFLAGS='-O3 -fno-strict-aliasing -g -DCH_GEN2 -DMEMORY_SCALE -D_AFFINITY_ -DCOMPAT_MODE -Wall -D_SMP_ -D_SMP_RNDV_ -DVIADEV_RPUT_SUPPORT -DEARLY_SEND_COMPLETION -DLAZY_MEM_UNREGISTER -D_IA32_ -I$(DESTDIR_PKG)/opt/gluster/ofed/include' FFLAGS=' -L$(DESTDIR_PKG)/opt/gluster/ofed/lib -Wall' F90FLAGS=' -Wall' MPIRUN_CFLAGS=' -DPARAM_GLOBAL=\"/opt/gluster/ofed/mpi/gcc/$(PACKAGE_NAME)/etc/mvapich.conf\" -DLD_LIBRARY_PATH_MPI=\"/opt/gluster/ofed/mpi/gcc/$(PACKAGE_NAME)/lib/shared\"  -DMVAPICH_BUILDID=\"1326\"' FC=$(CROSS)-gfortran ac_cv_prog_AR=$(CROSS)-ar pac_cv_cross_compiling=yes
endif

mvapich.%: PKG_CONFIGURE_CMD=PKG_CONFIGURE_CMD=RSHCOMMAND="ssh" ./configure --enable-sharedlib --with-device=ch_gen2 --with-arch=LINUX --prefix=$1/opt/gluster/ofed/mpi/gcc/$(PACKAGE_NAME) --enable-f77 --enable-f90 '-lib=-L$(DESTDIR_PKG)/opt/gluster/ofed/lib -libverbs -libumad -libcommon -lpthread -Wall' --c++=$(CROSS)-g++ --disable-devdebug -cc=$(CROSS)-gcc -fc=$(CROSS)-gfortran -clinker=$(CROSS)-gcc -c++linker=$(CROSS)-g++ -with-cross

mvapich.%: PKG_BUILD_CMD=make -j 8 RANLIB=$(CROSS)-ranlib
