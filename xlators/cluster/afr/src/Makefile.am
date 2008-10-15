xlator_LTLIBRARIES = afr.la
xlatordir = $(libdir)/glusterfs/$(PACKAGE_VERSION)/xlator/cluster

afr_la_LDFLAGS = -module -avoidversion 

afr_la_SOURCES = afr.c afr-dir-read.c afr-dir-write.c afr-inode-read.c afr-inode-write.c afr-transaction.c afr-self-heal.c 
afr_la_LIBADD = $(top_builddir)/libglusterfs/src/libglusterfs.la

noinst_HEADERS = afr.h afr-transaction.h afr-inode-write.h afr-inode-read.h afr-dir-read.h afr-dir-write.h afr-self-heal.h

AM_CFLAGS = -fPIC -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -Wall -D$(GF_HOST_OS) \
	    -I$(top_srcdir)/libglusterfs/src -shared -nostartfiles $(GF_CFLAGS)

CLEANFILES = 

