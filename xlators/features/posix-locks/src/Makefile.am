xlator_LTLIBRARIES = posix-locks.la
xlatordir = $(libdir)/glusterfs/$(PACKAGE_VERSION)/xlator/features

posix_locks_la_LDFLAGS = -module -avoidversion

posix_locks_la_SOURCES = common.c posix.c internal.c
posix_locks_la_LIBADD = $(top_builddir)/libglusterfs/src/libglusterfs.la 

noinst_HEADERS = locks.h common.h

AM_CFLAGS = -fPIC -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -Wall -fno-strict-aliasing -D$(GF_HOST_OS) \
	-I$(top_srcdir)/libglusterfs/src $(GF_CFLAGS) -shared -nostartfiles

CLEANFILES = 

