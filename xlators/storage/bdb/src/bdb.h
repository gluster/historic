/*
   Copyright (c) 2008 Z RESEARCH, Inc. <http://www.zresearch.com>
   This file is part of GlusterFS.

   GlusterFS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 3 of the License,
   or (at your option) any later version.

   GlusterFS is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see
   <http://www.gnu.org/licenses/>.
*/

#ifndef _BDB_H
#define _BDB_H

#ifndef _CONFIG_H
#define _CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

/* TODO: check for this in configure */
#include <db.h>

#ifdef linux
#ifdef __GLIBC__
#include <sys/fsuid.h>
#else
#include <unistd.h>
#endif
#endif

#ifdef HAVE_SYS_XATTR_H
#include <sys/xattr.h>
#endif

#ifdef HAVE_SYS_EXTATTR_H
#include <sys/extattr.h>
#endif

#include <pthread.h>
#include "xlator.h"
#include "inode.h"
#include "compat.h"

struct bdb_ctx {
  char *directory;
  DB *ns;
  DB *storage;
  int32_t ref;
  gf_lock_t lock;
};

struct bdb_fd {
  struct bdb_ctx *ctx;
  char *key;
  int32_t flags;
};

struct bdb_dir {
  char *key;
  DBC *nsc;
  char *path;
  struct bdb_ctx *ctx;
};

struct bdb_private {
  DB_ENV *dbenv;
  inode_table_t *itable;
  int32_t temp;
  char is_stateless;
  char *base_path;
  int32_t base_path_length;

  struct xlator_stats stats; /* Statastics, provides activity of the server */
  
  struct timeval prev_fetch_time;
  struct timeval init_time;
  int32_t max_read;            /* */
  int32_t max_write;           /* */
  int64_t interval_read;      /* Used to calculate the max_read value */
  int64_t interval_write;     /* Used to calculate the max_write value */
  int64_t read_value;    /* Total read, from init */
  int64_t write_value;   /* Total write, from init */
  dict_t *db_ctx;
};

#endif /* _BDB_H */