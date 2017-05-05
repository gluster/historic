/*
   Copyright (c) 2007, 2008 Z RESEARCH, Inc. <http://www.zresearch.com>
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

#ifndef _INODE_H
#define _INODE_H

#ifndef _CONFIG_H
#define _CONFIG_H
#include "config.h"
#endif

#include <stdint.h>
#include <sys/types.h>

struct _inode_table;
typedef struct _inode_table inode_table_t;

struct _inode;
typedef struct _inode inode_t;

struct _dentry;
typedef struct _dentry dentry_t;

#include "list.h"
#include "xlator.h"


struct _inode_table {
        pthread_mutex_t    lock;
        size_t             hashsize;    /* bucket size of inode hash and dentry hash */
        char              *name;        /* name of the inode table, just for gf_log() */
        inode_t           *root;        /* root directory inode, with number 1 */
        xlator_t          *xl;          /* xlator to be called to do purge */
        uint32_t           lru_limit;   /* maximum LRU cache size */
        struct list_head  *inode_hash;  /* buckets for inode hash table */
        struct list_head  *name_hash;   /* buckets for dentry hash table */
        struct list_head   active;      /* list of inodes currently active (in an fop) */
        uint32_t           active_size; /* count of inodes in active list */
        struct list_head   lru;         /* list of inodes recently used.
                                           lru.next most recent */
        uint32_t           lru_size;    /* count of inodes in lru list  */
        struct list_head   purge;       /* list of inodes to be purged soon */
        uint32_t           purge_size;  /* count of inodes in purge list */
};


struct _dentry {
        struct list_head   inode_list;   /* list of dentries of inode */
        struct list_head   hash;         /* hash table pointers */
        struct list_head   parent_list;  /* list of dentries under the parent  */
        inode_t           *inode;        /* inode of this directory entry */
        char              *name;         /* name of the directory entry */
        inode_t           *parent;       /* directory of the entry */
};

//#define ZR_INODE_CTX_VALUE_LEN 2
struct _inode_ctx {
	uint64_t key;
	uint64_t value;
	//uint64_t value[ZR_INODE_CTX_VALUE_LEN];
};

struct _inode {
        inode_table_t    *table;         /* the table this inode belongs to */
        gf_lock_t         lock;
        uint64_t          nlookup;
        uint64_t          generation;
        uint32_t          ref;           /* reference count on this inode */
        ino_t             ino;           /* inode number in the storage (persistent) */
        dict_t           *ctx;           /* per xlator private */
        mode_t            st_mode;       /* what kind of file */
        struct list_head  fd_list;       /* list of open files on this inode */
        struct list_head  dentry_list;   /* list of directory entries for this inode */
        struct list_head  child_list;    /* list of directory entries under this inode */
        struct list_head  hash;          /* hash table pointers */
        struct list_head  list;          /* active/lru/purge */

	struct _inode_ctx *_ctx;    /* replacement for dict_t *(inode->ctx) */
};


inode_table_t *
inode_table_new (size_t lru_limit, xlator_t *xl);

inode_t *
inode_new (inode_table_t *table);

inode_t *
inode_search (inode_table_t *table, ino_t ino, const char *name);

int
inode_link (inode_t *inode, inode_t *parent,
            const char *name, struct stat *stbuf);

void
inode_unlink (inode_t *inode,
              inode_t *parent,
              const char *name);

inode_t *
inode_parent (inode_t *inode, ino_t par, const char *name);

inode_t *
inode_ref (inode_t *inode);

inode_t *
inode_unref (inode_t *inode);

int
inode_lookup (inode_t *inode);

int
inode_forget (inode_t *inode,
	      uint64_t nlookup);

int
inode_rename (inode_table_t *table,
	      inode_t *olddir,
	      const char *oldname,
	      inode_t *newdir,
	      const char *newname,
	      inode_t *inode,
	      struct stat *stbuf);


int32_t
inode_path (inode_t *inode,
	    const char *name,
	    char **bufp);

inode_t *
inode_from_path (inode_table_t *table,
		 const char *path);

int
inode_ctx_put (inode_t *inode, xlator_t *xlator, uint64_t value);

int 
inode_ctx_get (inode_t *inode, xlator_t *xlator, uint64_t *value);

int 
inode_ctx_del (inode_t *inode, xlator_t *xlator, uint64_t *value);

#endif /* _INODE_H */
