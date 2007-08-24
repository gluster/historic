
bin_PROGRAMS = glusterfs-booster.so

AM_CFLAGS = -fPIC -Wall -pthread

AM_CPPFLAGS = -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26 -D_GNU_SOURCE \
	-I$(top_srcdir)/libglusterfs/src -DDATADIR=\"$(localstatedir)\" \
	-DCONFDIR=\"$(sysconfdir)/glusterfs\"

CLEANFILES = *~

glusterfs_booster_so_SOURCES = entry.c bridge.c
glusterfs_booster_so_CFLAGS = -D_GNU_SOURCE 
glusterfs_booster_so_LDFLAGS = -shared -nostartfiles
#glusterfs_booster_so_LDADD = -L../../libglusterfs/src -lglusterfs