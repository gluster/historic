xlator_PROGRAMS = io-threads.so
xlatordir = $(libdir)/glusterfs/$(PACKAGE_VERSION)/xlator/performance

io_threads_so_SOURCES = io-threads.c
io_threads_so_LDADD = $(top_builddir)/libglusterfs/src/libglusterfs.la

noinst_HEADERS = io-threads.h

AM_CFLAGS = -fPIC -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -Wall -D$(GF_HOST_OS)\
	-I$(top_srcdir)/libglusterfs/src -shared -nostartfiles $(GF_CFLAGS)

CLEANFILES = 
