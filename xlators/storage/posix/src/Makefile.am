
xlator_PROGRAMS = posix.so
xlatordir = $(libdir)/glusterfs/$(PACKAGE_VERSION)/xlator/storage

posix_so_SOURCES = posix.c 
posix_so_LDADD = $(top_builddir)/libglusterfs/src/libglusterfs.la 

noinst_HEADERS = posix.h 

AM_CFLAGS = -fPIC -fno-strict-aliasing -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -D$(GF_HOST_OS) -Wall \
	-I$(top_srcdir)/libglusterfs/src -shared -nostartfiles \
	$(GF_CFLAGS)

CLEANFILES = 

