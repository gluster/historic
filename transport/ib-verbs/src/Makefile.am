# Copyright (c) 2006, 2007, 2008 Z RESEARCH, Inc. <http://www.zresearch.com>
# This file is part of GlusterFS.
#
# GlusterFS is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# GlusterFS is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

noinst_HEADERS = ib-verbs.h name.h

transport_LTLIBRARIES = ib-verbs.la
transportdir = $(libdir)/glusterfs/$(PACKAGE_VERSION)/transport/

ib_verbs_la_LDFLAGS = -module -avoidversion

ib_verbs_la_SOURCES = ib-verbs.c name.c
ib_verbs_la_LIBADD = -libverbs $(top_builddir)/libglusterfs/src/libglusterfs.la 

AM_CFLAGS = -fPIC -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -Wall -D$(GF_HOST_OS)\
	-I$(top_srcdir)/libglusterfs/src -I$(top_srcdir)/transport/ib-verbs \
	-shared -nostartfiles

CLEANFILES = *~
