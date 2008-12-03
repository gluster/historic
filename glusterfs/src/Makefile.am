sbin_PROGRAMS = glusterfs

glusterfs_SOURCES = glusterfs.c fetch-spec.c
glusterfs_LDADD = $(top_builddir)/libglusterfs/src/libglusterfs.la $(GF_LDADD)
glusterfs_LDFLAGS = $(GF_LDFLAGS) $(GF_GLUSTERFS_LDFLAGS)
noinst_HEADERS = glusterfsd.h

AM_CFLAGS = -fPIC -Wall -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -D$(GF_HOST_OS)\
	-I$(top_srcdir)/libglusterfs/src -DDATADIR=\"$(localstatedir)\" \
	-DCONFDIR=\"$(sysconfdir)/glusterfs\" $(GF_GLUSTERFS_CFLAGS)

CLEANFILES = 

$(top_builddir)/libglusterfs/src/libglusterfs.la:
	make -C $(top_builddir)/libglusterfs/src/ all

uninstall-local:
	rm -f $(DESTDIR)$(sbindir)/glusterfsd

install-data-local:
	$(INSTALL) -d -m 755 $(DESTDIR)$(localstatedir)/run
	$(INSTALL) -d -m 755 $(DESTDIR)$(localstatedir)/log/glusterfs
	rm -f $(DESTDIR)$(sbindir)/glusterfsd
	ln -s glusterfs $(DESTDIR)$(sbindir)/glusterfsd
