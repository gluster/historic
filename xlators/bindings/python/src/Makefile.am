
xlator_PROGRAMS = python.so

xlatordir = $(libdir)/glusterfs/$(PACKAGE_VERSION)/xlator/bindings

scripts_DATA = gluster.py glustertypes.py glusterstack.py

scriptsdir = $(xlatordir)/python

python_so_SOURCES = python.c

AM_CFLAGS = -fPIC -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -Wall \
	-I$(top_srcdir)/libglusterfs/src -shared -nostartfiles \
	$(PYTHON_CFLAGS) -DGLUSTER_PYTHON_PATH=\"$(scriptsdir)\"

AM_LDFLAGS = $(PYTHON_LDFLAGS)

CLEANFILES = *~

