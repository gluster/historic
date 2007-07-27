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
mvapich2.%: PACKAGE_NAME=mvapich2-0.9.8-13

mvapich2.%: PKG_CONFIGURE_CMD=

mvapich2.%: PKG_BUILD_ENV=OPEN_IB_HOME=$(DESTDIR_PKG)/opt/gluster/ofed RDMA_CM_SUPPORT=yes PREFIX=$(DESTDIR_PKG)/opt/gluster/ofed/mpi/gcc/$(PACKAGE_NAME) ROMIO=yes SHARED_LIBS=yes CC=$(CROSS)-gcc CXX=$(CROSS)-g++ F77=$(CROSS)-gfortran F90=$(CROSS)-gfortran 

mvapich2.%: PKG_BUILD_CMD=./make.mvapich2.ofa

mvapich2.%: PKG_INSTALL_CMD=cd $1/opt/gluster/ofed/mpi/gcc/$(PACKAGE_NAME) \
   for f in bin/mpicc bin/mpich2version bin/mpicxx bin/mpif77 bin/mpif90 etc/mpicxx.conf etc/mpif77.conf etc/mpif90.conf etc/mpicc.conf; do \
      if [ -f $f ]; then \
         sed -i "s^prefix=$(DESTDIR_PKG)^prefix=^g" $f \
      fi \
   done \
   for f in bin/mpicc bin/mpicxx bin/mpif77 bin/mpif90 etc/mpicc.conf etc/mpicxx.conf etc/mpif77.conf etc/mpif90.conf; do \
      if [ -f $f ]; then \
         sed -i -e "s^-L$(DESTDIR_PKG)^-L^g" -e "s^-I$(DESTDIR_PKG)^-I^g" $f \
      fi \
   done
