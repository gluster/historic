/*
  Copyright (c) 2006,2007 Z RESEARCH, Inc. <http://www.zresearch.com>
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

#include "glusterfs.h"
#include "xlator.h"
#include "dict.h"
#include "logging.h"
#include "transport.h"
#include "protocol.h"

#define BOOSTER_LISTEN_PATH  "/tmp/glusterfs-booster-server"

static int32_t
booster_getxattr_cbk (call_frame_t *frame,
		      void *cookie,
		      xlator_t *this,
		      int32_t op_ret,
		      int32_t op_errno,
		      dict_t *dict)
{
  dict_t *options = get_new_dict ();
  int len;
  char *buf;
  loc_t *loc = (loc_t *)cookie;

  gf_log (this->name, GF_LOG_DEBUG, "setting path to %s", BOOSTER_LISTEN_PATH);
  dict_set (options, "transport-type", str_to_data ("unix/client"));
  dict_set (options, "connect-path", str_to_data (BOOSTER_LISTEN_PATH));

  len = dict_serialized_length (options);
  buf = calloc (1, len);
  dict_serialize (options, buf);

  dict_set (dict, "user.glusterfs-booster-transport-options", 
	    data_from_dynptr (buf, len));
  dict_set (dict, "user.glusterfs-booster-handle", bin_to_data (loc->inode, 8));

  if (op_ret < 0)
    op_ret = 2;
  else
    op_ret += 2;

  STACK_UNWIND (frame, op_ret, op_errno, dict);
  return 0;
}

static int32_t
booster_getxattr (call_frame_t *frame,
		  xlator_t *this,
		  loc_t *loc)
{
  _STACK_WIND (frame, booster_getxattr_cbk,
	       loc, FIRST_CHILD (this), FIRST_CHILD (this)->fops->getxattr,
	       loc);
  return 0;
}

static int32_t
booster_open_cbk (call_frame_t *frame,
                  void *cookie,
                  xlator_t *this,
                  int32_t op_ret,
                  int32_t op_errno,
                  fd_t *fd)
{
  STACK_UNWIND (frame, op_ret, op_errno, fd);
  return 0;
}

static int32_t
booster_open (call_frame_t *frame,
              xlator_t *this,
              loc_t *loc,
              int32_t flags,
              fd_t *fd)
{
  STACK_WIND (frame, booster_open_cbk,
	      FIRST_CHILD (this), FIRST_CHILD (this)->fops->open,
	      loc, flags, fd);
  return 0;
}

static int32_t
booster_readv_reply_cbk (call_frame_t *frame, 
			 void *cookie,
			 xlator_t *this,
			 int32_t op_ret,
			 int32_t op_errno,
			 struct iovec *vector,
			 int32_t count,
			 struct stat *stbuf)
{
}

int32_t
booster_readv_req (xlator_t *this, 
		   dict_t *params)
{
}

static int32_t
booster_writev_reply_cbk (call_frame_t *frame,
			  void *cookie,
			  xlator_t *this,
			  int32_t op_ret,
			  int32_t op_errno,
			  struct stat *stbuf)
{
}

int32_t
booster_writev_req (xlator_t *this, 
		    dict_t *params)
{
}

int32_t
notify (xlator_t *this,
        int32_t event,
        void *data,
        ...)
{
  switch (event) {
  case GF_EVENT_POLLIN:
    printf ("some input happening\n");
    break;
  case GF_EVENT_POLLERR:
    transport_disconnect (data);
    break;
  }
}

int32_t
booster_interpret (transport_t *trans, gf_block_t *blk)
{
  dict_t *params = blk->dict;
  xlator_t *this = (xlator_t *)trans->private;

  switch (blk->type) {
  case GF_OP_TYPE_FOP_REQUEST:
    if (blk->op == GF_FOP_READ) {
      booster_readv_req (this, params);
    }
    else if (blk->op == GF_FOP_WRITE) {
      booster_writev_req (this, params);
    }
    else {
      gf_log (trans->xl->name, GF_LOG_WARNING, "unexpected fop (%d)", blk->op);
      return -1;
    }
    break;
  default:
    gf_log (trans->xl->name, GF_LOG_WARNING, "unexpected block type (%d)", blk->type);
    return -1;
  }
}

int32_t
init (xlator_t *this)
{
  transport_t *trans;
  dict_t *options = get_new_dict ();

  if (!this->children || this->children->next) {
    gf_log (this->name, GF_LOG_ERROR,
	    "FATAL: booster not configured with exactly one child");
    return -1;
  }

  dict_set (options, "transport-type", str_to_data ("unix/server"));
  dict_set (options, "listen-path", str_to_data (BOOSTER_LISTEN_PATH));

  trans = transport_load (options, this, this->notify);

  return 0;
}

void
fini (xlator_t *this)
{
}

struct xlator_fops fops = {
  .open = booster_open,
  .getxattr = booster_getxattr,
};

struct xlator_mops mops = {
};
