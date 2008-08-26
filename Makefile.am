EXTRA_DIST = autogen.sh COPYING INSTALL README AUTHORS THANKS NEWS glusterfs.spec

SUBDIRS = argp-standalone libglusterfs $(LIBGLUSTERFSCLIENT_SUBDIR) xlators scheduler transport auth glusterfs $(MOD_GLUSTERFS_SUBDIR) doc extras

CLEANFILES = 

tlaclean: distclean
	find . -name Makefile.in -exec rm -f {} \;
	find . -name Makefile -exec rm -f {} \;
	find . -name mount.glusterfs -exec rm -f {} \;
	rm -fr autom4te.cache
	rm -f missing aclocal.m4 config.h.in config.guess config.sub ltmain.sh install-sh configure depcomp

