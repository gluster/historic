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

mvapich2.%: PACKAGE_NAME=mvapich2-0.9.8

ifeq ($(ARCH),x86_64)
mvapich2.%: PKG_CONFIGURE_ENV=PREFIX=/opt/gluster/ofed/mpi/gcc/$(PACKAGE_NAME) CC=$(CROSS)-gcc CXX=$(CROSS)-g++ F77=$(CROSS)-gfortran F90=$(CROSS)-gfortran AR=$(CROSS)-ar RANLIB=$(CROSS)-ranlib LIBS=' -L$(DESTDIR_PKG)/opt/gluster/ofed/lib -Wl,-rpath=$(DESTDIR_PKG)/opt/gluster/ofed/lib -libcommon -lrdmacm -libverbs -libumad -lpthread' CFLAGS=' -I$(DESTDIR_PKG)/opt/gluster/include -D_EM64T_ -D_SMP_ -DUSE_HEADER_CACHING  -DONE_SIDED -DMPID_USE_SEQUENCE_NUMBERS -D_SHMEM_COLL_ -DRDMA_CM   -I$(DESTDIR_PKG)/opt/gluster/ofed/include -O2' MPI_F90FLAGS=-O2 LD_LIBRARY_PATH=$(DESTDIR_PKG)/opt/gluster/ofed/lib:$$LD_LIBRARY_PATH FFLAGS=-L$(DESTDIR_PKG)/opt/gluster/ofed/lib CROSS_SIZEOF_SHORT=2 CROSS_SIZEOF_INT=4 CROSS_SIZEOF_LONG=8 CROSS_SIZEOF_LONG_LONG=8 CROSS_SIZEOF_FLOAT=4 CROSS_SIZEOF_DOUBLE=8 CROSS_SIZEOF_LONG_DOUBLE=16 CROSS_SIZEOF_WCHAR_T=4 CROSS_SIZEOF_FLOAT_INT=8 CROSS_SIZEOF_DOUBLE_INT=16 CROSS_SIZEOF_LONG_INT=16 CROSS_SIZEOF_SHORT_INT=8 CROSS_SIZEOF_2_INT=8 CROSS_SIZEOF_LONG_DOUBLE_INT=32 CROSS_SIZEOF_VOID_P=8
endif

ifeq ($(ARCH),i686)
mvapich2.%: PKG_CONFIGURE_ENV=PREFIX=/opt/gluster/ofed/mpi/gcc/$(PACKAGE_NAME) CC=$(CROSS)-gcc CXX=$(CROSS)-g++ F77=$(CROSS)-gfortran F90=$(CROSS)-gfortran AR=$(CROSS)-ar RANLIB=$(CROSS)-ranlib LIBS=' -L$(DESTDIR_PKG)/opt/gluster/ofed/lib -Wl,-rpath=$(DESTDIR_PKG)/opt/gluster/ofed/lib -libcommon -lrdmacm -libverbs -libumad -lpthread' CFLAGS=' -I$(DESTDIR_PKG)/opt/gluster/include -D_EM64T_ -D_SMP_ -DUSE_HEADER_CACHING  -DONE_SIDED -DMPID_USE_SEQUENCE_NUMBERS -D_SHMEM_COLL_ -DRDMA_CM   -I$(DESTDIR_PKG)/opt/gluster/ofed/include -O2' MPI_F90FLAGS=-O2 LD_LIBRARY_PATH=$(DESTDIR_PKG)/opt/gluster/ofed/lib:$$LD_LIBRARY_PATH FFLAGS=-L$(DESTDIR_PKG)/opt/gluster/ofed/lib CROSS_SIZEOF_SHORT=2 CROSS_SIZEOF_INT=4 CROSS_SIZEOF_LONG=4 CROSS_SIZEOF_LONG_LONG=8 CROSS_SIZEOF_FLOAT=4 CROSS_SIZEOF_DOUBLE=8 CROSS_SIZEOF_LONG_DOUBLE=12 CROSS_SIZEOF_WCHAR_T=4 CROSS_SIZEOF_FLOAT_INT=8 CROSS_SIZEOF_DOUBLE_INT=12 CROSS_SIZEOF_LONG_INT=8 CROSS_SIZEOF_SHORT_INT=8 CROSS_SIZEOF_2_INT=8 CROSS_SIZEOF_LONG_DOUBLE_INT=16 CROSS_SIZEOF_VOID_P=4 CROSS_STRUCT_ALIGN="four"
endif

mvapich2.%: PKG_CONFIGURE_CMD=make distclean; rm -rf '*.cache' '*.log' '*.status' lib bin; $(DEFAULT_PKG_CONFIGURE_CMD) --prefix=/opt/gluster/ofed/mpi/gcc/$(PACKAGE_NAME) --with-device=osu_ch3:mrail --with-rdma=gen2 --with-pm=mpd --disable-romio --enable-sharedlibs=gcc --without-mpe

ifeq ($(ARCH),x86_64)
mvapich2.%: PKG_BUILD_ENV=PREFIX=/opt/gluster/ofed/mpi/gcc/$(PACKAGE_NAME) CC=$(CROSS)-gcc CXX=$(CROSS)-g++ F77=$(CROSS)-gfortran F90=$(CROSS)-gfortran AR=$(CROSS)-ar RANLIB=$(CROSS)-ranlib LIBS=' -L$(DESTDIR_PKG)/opt/gluster/ofed/lib -Wl,-rpath=$(DESTDIR_PKG)/opt/gluster/ofed/lib -libcommon -lrdmacm -libverbs -libumad -lpthread' CFLAGS=' -I$(DESTDIR_PKG)/opt/gluster/include -D_EM64T_ -D_SMP_ -DUSE_HEADER_CACHING  -DONE_SIDED -DMPID_USE_SEQUENCE_NUMBERS -D_SHMEM_COLL_ -DRDMA_CM   -I$(DESTDIR_PKG)/opt/gluster/ofed/include -O2' MPI_F90FLAGS=-O2 LD_LIBRARY_PATH=$(DESTDIR_PKG)/opt/gluster/ofed/lib:$$LD_LIBRARY_PATH FFLAGS=-L$(DESTDIR_PKG)/opt/gluster/ofed/lib CROSS_SIZEOF_SHORT=2 CROSS_SIZEOF_INT=4 CROSS_SIZEOF_LONG=8 CROSS_SIZEOF_LONG_LONG=8 CROSS_SIZEOF_FLOAT=4 CROSS_SIZEOF_DOUBLE=8 CROSS_SIZEOF_LONG_DOUBLE=16 CROSS_SIZEOF_WCHAR_T=4 CROSS_SIZEOF_FLOAT_INT=8 CROSS_SIZEOF_DOUBLE_INT=16 CROSS_SIZEOF_LONG_INT=16 CROSS_SIZEOF_SHORT_INT=8 CROSS_SIZEOF_2_INT=8 CROSS_SIZEOF_LONG_DOUBLE_INT=32 CROSS_SIZEOF_VOID_P=8
endif

ifeq ($(ARCH),i686)
mvapich2.%: PKG_BUILD_ENV=PREFIX=/opt/gluster/ofed/mpi/gcc/$(PACKAGE_NAME) CC=$(CROSS)-gcc CXX=$(CROSS)-g++ F77=$(CROSS)-gfortran F90=$(CROSS)-gfortran AR=$(CROSS)-ar RANLIB=$(CROSS)-ranlib LIBS=' -L$(DESTDIR_PKG)/opt/gluster/ofed/lib -Wl,-rpath=$(DESTDIR_PKG)/opt/gluster/ofed/lib -libcommon -lrdmacm -libverbs -libumad -lpthread' CFLAGS=' -I$(DESTDIR_PKG)/opt/gluster/include -D_EM64T_ -D_SMP_ -DUSE_HEADER_CACHING  -DONE_SIDED -DMPID_USE_SEQUENCE_NUMBERS -D_SHMEM_COLL_ -DRDMA_CM   -I$(DESTDIR_PKG)/opt/gluster/ofed/include -O2' MPI_F90FLAGS=-O2 LD_LIBRARY_PATH=$(DESTDIR_PKG)/opt/gluster/ofed/lib:$$LD_LIBRARY_PATH FFLAGS=-L$(DESTDIR_PKG)/opt/gluster/ofed/lib CROSS_SIZEOF_SHORT=2 CROSS_SIZEOF_INT=4 CROSS_SIZEOF_LONG=4 CROSS_SIZEOF_LONG_LONG=8 CROSS_SIZEOF_FLOAT=4 CROSS_SIZEOF_DOUBLE=8 CROSS_SIZEOF_LONG_DOUBLE=12 CROSS_SIZEOF_WCHAR_T=4 CROSS_SIZEOF_FLOAT_INT=8 CROSS_SIZEOF_DOUBLE_INT=12 CROSS_SIZEOF_LONG_INT=8 CROSS_SIZEOF_SHORT_INT=8 CROSS_SIZEOF_2_INT=8 CROSS_SIZEOF_LONG_DOUBLE_INT=16 CROSS_SIZEOF_VOID_P=4 CROSS_STRUCT_ALIGN="four"
endif

mvapich2.%: PKG_BUILD_CMD=make

mvapich2.%: PKG_INSTALL_CMD=make install DESTDIR=$1; cd $1/opt/gluster/ofed/mpi/gcc/$(PACKAGE_NAME); for f in bin/mpicc bin/mpich2version bin/mpicxx bin/mpif77 bin/mpif90 etc/mpicxx.conf etc/mpif77.conf etc/mpif90.conf etc/mpicc.conf; do if [ -f $$f ]; then sed -i "s^prefix=$(DESTDIR_PKG)^prefix=^g" $$f; fi; done; for f in bin/mpicc bin/mpicxx bin/mpif77 bin/mpif90 etc/mpicc.conf etc/mpicxx.conf etc/mpif77.conf etc/mpif90.conf; do if [ -f $$f ]; then sed -i -e "s^-L$(DESTDIR_PKG)^-L^g" -e "s^-I$(DESTDIR_PKG)^-I^g" $$f; fi; done
