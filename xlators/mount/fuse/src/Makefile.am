
noinst_HEADERS = fuse-extra.h 

xlator_PROGRAMS = fuse.so
xlatordir = $(libdir)/glusterfs/$(PACKAGE_VERSION)/xlator/mount
fuse_so_SOURCES = fuse-bridge.c fuse-extra.c

AM_CFLAGS = -fPIC -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -D$(GF_HOST_OS) -Wall \
	-I$(top_srcdir)/libglusterfs/src $(GF_DARWIN_BUNDLE_CFLAGS) -DFUSE_USE_VERSION=26

AM_LDFLAGS = -shared -nostartfiles -lfuse $(GF_DARWIN_FUSE_LDADD)

CLEANFILES = 

