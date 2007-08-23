xlator_PROGRAMS = booster.so
xlatordir = $(libdir)/glusterfs/$(PACKAGE_VERSION)/xlator/performance

booster_so_SOURCES = booster.c

noinst_HEADERS = booster.h

AM_CFLAGS = -fPIC -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -Wall \
	-I$(top_srcdir)/libglusterfs/src -shared -nostartfiles

CLEANFILES = *~