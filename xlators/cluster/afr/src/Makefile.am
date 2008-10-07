xlator_LTLIBRARIES = afr.la
xlatordir = $(libdir)/glusterfs/$(PACKAGE_VERSION)/xlator/cluster

afr_la_LDFLAGS = -module -avoidversion 

afr_la_SOURCES = afr.c dir-read.c dir-write.c inode-read.c inode-write.c transaction.c self-heal.c
afr_la_LIBADD = $(top_builddir)/libglusterfs/src/libglusterfs.la

noinst_HEADERS = afr.h transaction.h inode-write.h inode-read.h dir-read.h dir-write.h

AM_CFLAGS = -fPIC -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -Wall -D$(GF_HOST_OS) \
	    -I$(top_srcdir)/libglusterfs/src -shared -nostartfiles $(GF_CFLAGS)

CLEANFILES = 

