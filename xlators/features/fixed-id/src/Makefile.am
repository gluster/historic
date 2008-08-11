xlator_PROGRAMS = fixed-id.so
xlatordir = $(libdir)/glusterfs/$(PACKAGE_VERSION)/xlator/features

fixed_id_so_SOURCES = fixed-id.c
fixed_id_so_LDADD = $(top_builddir)/libglusterfs/src/libglusterfs.la 

AM_CFLAGS = -fPIC -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -Wall -D$(GF_HOST_OS) \
	-I$(top_srcdir)/libglusterfs/src -shared -nostartfiles $(GF_CFLAGS)

CLEANFILES = 

