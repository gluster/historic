xlator_PROGRAMS = filter.so
xlatordir = $(libdir)/glusterfs/$(PACKAGE_VERSION)/xlator/features

filter_so_SOURCES = filter.c
filter_so_LDADD = $(top_builddir)/libglusterfs/src/libglusterfs.la

noinst_HEADERS = filter.h

AM_CFLAGS = -fPIC -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -Wall -D$(GF_HOST_OS)\
	-I$(top_srcdir)/libglusterfs/src -shared -nostartfiles $(GF_CFLAGS)

CLEANFILES = 

