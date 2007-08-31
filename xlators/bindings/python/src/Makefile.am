
xlator_PROGRAMS = python.so
xlatordir = $(libdir)/glusterfs/$(PACKAGE_VERSION)/xlator/bindings

python_so_SOURCES = python.c

pythonlibs_DATA = gluster.py glusterstack.py glustertypes.py
pythonlibsdir = $(libdir)/glusterfs/$(PACKAGE_VERSION)/xlator/bindings/python/

AM_CFLAGS = -fPIC -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -Wall \
	-I$(top_srcdir)/libglusterfs/src -shared -nostartfiles \
	$(PYTHON_CFLAGS)

AM_LDFLAGS = $(PYTHON_LDFLAGS)

CLEANFILES = *~

