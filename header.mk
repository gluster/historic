
abs_top_srcdir = $(shell cd $(top_srcdir) && pwd)
abs_top_builddir = $(shell cd $(top_builddir) && pwd)

CROSS=$(host_alias)
GLUSTER_BUILD=$(build_alias)
ARCH=$(shell echo $(host_alias) | sed 's/-.*//g')
KERNEL_ARCH=$(shell echo $(ARCH) | sed -e s/i.86/i386/ -e s/sun4u/sparc64/ \
                                  -e s/arm.*/arm/ -e s/sa110/arm/ \
                                  -e s/s390x/s390/ -e s/parisc64/parisc/ )

TOOL_BASE=$(shell dirname `which $(CC)`)/..

DESTDIR_LIVE=$(abs_top_builddir)/destdir_live
DESTDIR_NATIVE=$(abs_top_builddir)/destdir_native
DESTDIR_PKG=$(abs_top_builddir)/destdir_pkg

#DESTDIR_STAGE2=$(abs_top_builddir)/destdir_stage2
DESTDIR_STAGE3=$(abs_top_builddir)/destdir_stage3
DESTDIR_RAMDISK=$(abs_top_builddir)/destdir_ramdisk
DESTDIR_DIST=$(abs_top_builddir)/destdir_dist

BUILD_NATIVE=$(abs_top_builddir)/build_native
BUILD_LIVE=$(abs_top_builddir)/build_live
BUILD_PKG=$(abs_top_builddir)/build_pkg

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
DEFAULT_NATIVE_INSTALL_ENV=DESTDIR=$(DESTDIR_NATIVE)

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

