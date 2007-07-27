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
openmpi.%: PACKAGE_NAME=openmpi-1.2.2-1

openmpi.%: PKG_CONFIGURE_ENV=CFLAGS='-O2 -g -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fexceptions -fstack-protector --param=ssp-buffer-size=4 -m64 -mtune=generic' CXXFLAGS='-O2 -g -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fexceptions -fstack-protector --param=ssp-buffer-size=4 -m64 -mtune=generic' FCFLAGS='-O2 -g -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fexceptions -fstack-protector --param=ssp-buffer-size=4 -m64 -mtune=generic' FFLAGS='-O2 -g -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fexceptions -fstack-protector --param=ssp-buffer-size=4 -m64 -mtune=generic'

openmpi.%: PKG_CONFIGURE_CMD=for i in `find . -name config.guess -o -name config.sub`; do \
   (cd `dirname $i` && aclocal; autoconf; automake --add-missing --copy --foreign) \
done \
$(DEFAULT_PKG_CONFIGURE_CMD) --prefix=/opt/gluster/ofed/mpi/gcc/$(PACKAGE_NAME) --sysconfdir=/opt/gluster/ofed/mpi/gcc/$(PACKAGE_NAME)/etc --datadir=/opt/gluster/ofed/mpi/gcc/$(PACKAGE_NAME)/share --includedir=/opt/gluster/ofed/mpi/gcc/$(PACKAGE_NAME)/include --libdir=/opt/gluster/ofed/mpi/gcc/$(PACKAGE_NAME)/lib --libexecdir=/opt/gluster/ofed/mpi/gcc/$(PACKAGE_NAME)/libexec --localstatedir=/opt/gluster/ofed/var --sharedstatedir=/opt/gluster/ofed/mpi/gcc/$(PACKAGE_NAME)/com --mandir=/opt/gluster/ofed/mpi/gcc/$(PACKAGE_NAME)/share/man --infodir=/opt/gluster/ofed/info 'LDFLAGS=-Wl,-R/opt/gluster/ofed/lib -Wl,-rpath-link,/var/tmp/OFED//opt/gluster/ofed/lib' --with-openib=$(DESTDIR_PKG)/opt/gluster/ofed --with-openib-libdir=$(DESTDIR_PKG)/opt/gluster/ofed/lib CC=$(CROSS)-gcc CXX=$(CROSS)-g++ F77=$(CROSS)-gfortran FC=$(CROSS)-gfortran '--with-wrapper-ldflags=-Wl,-rpath -Wl,/opt/gluster/ofed/mpi/gcc/$(PACKAGE_NAME)/lib' --enable-mpirun-prefix-by-default

openmpi.%: PKG_BUILD_CMD=make -j 4
