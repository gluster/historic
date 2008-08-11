sched_PROGRAMS = nufa.so
scheddir = $(libdir)/glusterfs/$(PACKAGE_VERSION)/scheduler

nufa_so_SOURCES = nufa.c
nufa_so_LDADD = $(top_builddir)/libglusterfs/src/libglusterfs.la

noinst_HEADERS = nufa.h

AM_CFLAGS = -fPIC -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -Wall -D$(GF_HOST_OS)\
	-I$(top_srcdir)/libglusterfs/src -shared -nostartfiles $(GF_CFLAGS)

CLEANFILES = 
