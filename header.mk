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

abs_top_srcdir = $(shell cd $(top_srcdir) && pwd)
abs_top_builddir = $(shell cd $(top_builddir) && pwd)


GLUSTER_BUILD=$(shell gcc -dumpmachine)
ARCH ?= $(shell uname -m)
CROSS = $(ARCH)-gluster-linux-gnu
KERNEL_ARCH=$(shell echo $(ARCH) | sed -e s/i.86/i386/ -e s/sun4u/sparc64/ \
                                  -e s/arm.*/arm/ -e s/sa110/arm/ \
                                  -e s/s390x/s390/ -e s/parisc64/parisc/ )

TOOL_BASE=$(shell dirname `which $(CROSS)-gcc 2>/dev/null` 2>/dev/null)/..

DESTDIR_LIVE=$(abs_top_builddir)/destdir_live_$(ARCH)
DESTDIR_NATIVE=$(abs_top_builddir)/destdir_native
DESTDIR_PKG=$(abs_top_builddir)/destdir_pkg_$(ARCH)

#DESTDIR_STAGE2=$(abs_top_builddir)/destdir_stage2
DESTDIR_STAGE3=$(abs_top_builddir)/destdir_stage3
DESTDIR_RAMDISK=$(abs_top_builddir)/destdir_ramdisk_$(ARCH)
DESTDIR_DIST=$(abs_top_builddir)/destdir_dist_$(ARCH)

BUILD_NATIVE=$(abs_top_builddir)/build_native
BUILD_LIVE=$(abs_top_builddir)/build_live_$(ARCH)
BUILD_PKG=$(abs_top_builddir)/build_pkg_$(ARCH)

TARBALLS_DIR = @TARBALLS_DIR@
PATCHES_DIR=$(abs_top_srcdir)/patches

PATH := $(TOOL_BASE)/bin:$(DESTDIR_NATIVE)/usr/bin:$(DESTDIR_NATIVE)/bin:$(DESTDIR_NATIVE)/usr/sbin:$(DESTDIR_NATIVE)/sbin:$(PATH)
LD_LIBRARY_PATH := $(DESTDIR_NATIVE)/lib:$(DESTDIR_NATIVE)/usr/lib:$(LD_LIBRARY_PATH)

export PATH 
export DESTDIR_LIVE
export LD_LIBRARY_PATH

### native

DEFAULT_NATIVE_PREPARE_ENV=FOOBAR=FUBAR
DEFAULT_NATIVE_CONFIGURE_ENV=CPPFLAGS=-I$(DESTDIR_NATIVE)/usr/include \
				LDFLAGS=-L$(DESTDIR_NATIVE)/usr/lib
DEFAULT_NATIVE_BUILD_ENV=FOOBAR=FUBAR
DEFAULT_NATIVE_INSTALL_ENV=

DEFAULT_NATIVE_PREPARE_CMD=FOOBAR=FUBAR
DEFAULT_NATIVE_CONFIGURE_CMD=./configure --prefix=$(DESTDIR_NATIVE)/usr
DEFAULT_NATIVE_BUILD_CMD=make all
DEFAULT_NATIVE_INSTALL_CMD=make install


### cross

DEFAULT_LIVE_PREPARE_ENV=FOOBAR=FUBAR
DEFAULT_LIVE_CONFIGURE_ENV=CPPFLAGS=-I$(DESTDIR_LIVE)/usr/include\
				LDFLAGS=-L$(DESTDIR_LIVE)/usr/lib\
				CC_FOR_BUILD=cc
DEFAULT_LIVE_BUILD_ENV=CC=$(CROSS)-gcc LD=$(CROSS)-ld NM=$(CROSS)-nm \
			AS=$(CROSS)-as AR=$(CROSS)-ar RANLIB=$(CROSS)-ranlib \
			OBJCOPY=$(CROSS)-objcopy CPP=$(CROSS)-cpp \
			CXX=$(CROSS)-g++ CC_FOR_BUILD=cc
DEFAULT_LIVE_INSTALL_ENV=DESTDIR=$(DESTDIR_LIVE)

DEFAULT_LIVE_PREPARE_CMD=FOOBAR=FUBAR
DEFAULT_LIVE_CONFIGURE_CMD=./configure --prefix=/usr --host=$(CROSS) --build=$(GLUSTER_BUILD)
DEFAULT_LIVE_BUILD_CMD=make all
DEFAULT_LIVE_INSTALL_CMD=make install DESTDIR=$1

## pkg

DEFAULT_PKG_PREPARE_ENV=FUBAR=FOOBAR
DEFAULT_PKG_CONFIGURE_ENV=CPPFLAGS=-I$(DESTDIR_PKG)/opt/gluster/include \
				LDFLAGS="-L$(DESTDIR_PKG)/opt/gluster/lib" \
				CC_FOR_BUILD=cc
DEFAULT_PKG_BUILD_ENV=CC=$(CROSS)-gcc LD=$(CROSS)-ld NM=$(CROSS)-nm \
			AS=$(CROSS)-as AR=$(CROSS)-ar RANLIB=$(CROSS)-ranlib \
			OBJCOPY=$(CROSS)-objcopy CPP=$(CROSS)-cpp \
			CXX=$(CROSS)-g++ CC_FOR_BUILD=cc
DEFAULT_PKG_INSTALL_ENV=DESTDIR=$(DESTDIR_PKG)

DEFAULT_PKG_PREPARE_CMD=FUBAR=FOOBAR
DEFAULT_PKG_CONFIGURE_CMD=./configure --prefix=/opt/gluster --host=$(CROSS) --build=$(GLUSTER_BUILD)
DEFAULT_PKG_BUILD_CMD=make all
DEFAULT_PKG_INSTALL_CMD=make install DESTDIR=$1



PATCH_CMD=$(foreach d,$(shell ls $(PATCHES_DIR)/$1/*.diff 2>/dev/null),\
	cat $d | patch --verbose -p1 -d $2; )

