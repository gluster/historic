/*
   Copyright (c) 2006 Z RESEARCH, Inc. <http://www.zresearch.com>
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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "glusterfs.h"
#include "dict.h"
#include "xlator.h"

#include "meta.h"

int32_t meta_lookup_cbk (call_frame_t *frame, void *cookie,
			 xlator_t *this, int32_t op_ret, int32_t op_errno, 
			 inode_t *inode, struct stat *buf)
{
  STACK_UNWIND (frame, op_ret, op_errno, inode, buf);
  return 0;
}

int32_t meta_lookup (call_frame_t *frame, xlator_t *this,
		     loc_t *loc)
{
  STACK_WIND (frame, meta_lookup_cbk,
	      FIRST_CHILD (this), FIRST_CHILD (this)->fops->lookup, loc);
  return 0;
}

int32_t
meta_opendir (call_frame_t *frame,
	      xlator_t *this,
	      const char *path)
{
  meta_private_t *priv = (meta_private_t *) this->private;
  meta_dirent_t *root = priv->tree;
  meta_dirent_t *dir = lookup_meta_entry (root, path, NULL);
  
  if (dir) {
    dict_t *ctx = get_new_dict ();
    dict_set (ctx, this->name, str_to_data (strdup (path)));
    STACK_UNWIND (frame, 0, 0, ctx);
    return 0;
  }
  else {  
    STACK_WIND (frame, meta_opendir_cbk,
		FIRST_CHILD(this), FIRST_CHILD(this)->fops->opendir,
		path);
    return 0;
  }
}

int32_t
init (xlator_t *this)
{
/*   if (this->parent != NULL) { */
/*     gf_log ("meta", GF_LOG_ERROR, "FATAL: meta should be the root of the xlator tree"); */
/*     return -1; */
/*   } */
  
/*   meta_private_t *priv = calloc (1, sizeof (meta_private_t)); */
  
/*   data_t *directory = dict_get (this->options, "directory"); */
/*   if (directory) { */
/*     priv->directory = strdup (data_to_str (directory)); */
/*   } */
/*   else { */
/*     priv->directory = ".meta"; */
/*   } */
  
/*   this->private = priv; */
/*   build_meta_tree (this); */

  return 0;
}

int32_t
fini (xlator_t *this)
{
  return 0;
}

struct xlator_fops fops = {
  //  .opendir     = meta_opendir,
  //  .readdir     = meta_readdir,
  //  .closedir    = meta_closedir,
  .lookup      = meta_lookup,
};

struct xlator_mops mops = {
};
