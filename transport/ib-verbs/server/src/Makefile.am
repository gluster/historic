
transport_PROGRAMS = server.so
transportdir = $(libdir)/glusterfs/$(PACKAGE_VERSION)/transport/ib-verbs

server_so_SOURCES = ib-verbs-server.c $(top_srcdir)/transport/ib-verbs/ib-verbs.c
server_so_LDADD = -libverbs

AM_CFLAGS = -fPIC -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -Wall -D$(GF_HOST_OS)\
	-I$(top_srcdir)/libglusterfs/src -I$(top_srcdir)/transport/ib-verbs -shared -nostartfiles

CLEANFILES = *~

