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

static int32_t
booster_setxattr (call_frame_t *frame,
                  xlator_t *this,
                  loc_t *loc,
                  dict_t *dict,
                  int32_t flags)
{
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
	      this, FIRST_CHILD (this)->fops->open,
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
      gf_log ("booster", GF_LOG_WARNING, "unexpected fop (%d)", blk->op);
      return -1;
    }
    break;
  default:
    gf_log ("booster", GF_LOG_WARNING, "unexpected block type (%d)", blk->type);
    return -1;
  }
}

int32_t
init (xlator_t *this)
{
  transport_t *trans;

  if (!this->children || this->children->next) {
    gf_log ("booster", GF_LOG_ERROR,
	    "FATAL: booster not configured with exactly one child");
    return -1;
  }

  trans = transport_load (this->options, this, this->notify);

  return 0;
}

void
fini (xlator_t *this)
{
}

struct xlator_fops fops = {
  .open = booster_open,
  .readv = booster_readv,
  .writev = booster_writev,
  .setxattr = booster_setxattr,
};

struct xlator_mops mops = {
};
