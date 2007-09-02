/*
   Copyright (c) 2006, 2007 Z RESEARCH, Inc. <http://www.zresearch.com>
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


#include <stdint.h>
#include <signal.h>
#include <pthread.h>
#include <stdio.h>

#include "glusterfs.h"
#include "logging.h"
#include "xlator.h"
#include "glusterfs.h"
#include "transport.h"
#include "defaults.h"
#include "common-utils.h"
#include "booster.h"


static glusterfs_ctx_t ctx;

int32_t
glusterfs_booster_bridge_notify (xlator_t *this, int32_t event,
				 void *data, ...)
{
  switch (event) {
  case GF_EVENT_POLLERR:
    transport_disconnect (event);
    break;
  }
  return 0;
}

glusterfs_ctx_t *
glusterfs_booster_bridge_init ()
{
  ctx.logfile = "/dev/stderr";
  ctx.loglevel = GF_LOG_DEBUG;
  ctx.poll_type = SYS_POLL_TYPE_MAX;

  gf_log_init ("/dev/stderr");
  gf_log_set_loglevel (GF_LOG_DEBUG);

  return &ctx;
}

void *
glusterfs_booster_bridge_open (glusterfs_ctx_t *ctx, char *options, int size,
			       char *handle)
{
  xlator_t *xl;
  transport_t *trans;
  data_t *transport_data;
  struct file *filep;
  int ret;

  xl = calloc (1, sizeof (xlator_t));
  xl->name = "booster";
  xl->type = "performance/booster\n";
  xl->next = xl->prev = xl;
  xl->ctx = &ctx;
  xl->notify = glusterfs_booster_bridge_notify;

  xl->options = get_new_dict ();
  if (dict_unserialize (options, size, &xl->options) == NULL) {
    gf_log ("booster", GF_LOG_ERROR,
	    "could not unserialize dictionary");
    free (xl);
    return NULL;
  }

  if (dict_get (xl->options, "transport-type") == NULL) {
    gf_log ("booster", GF_LOG_ERROR,
	    "missing option transport-type");
    free (xl);
    return NULL;
  }

  trans = transport_load (xl->options, xl,
			  glusterfs_booster_bridge_notify);

  if (!trans) {
    gf_log ("booster", GF_LOG_ERROR,
	    "disabling booster for this fd");
    free (xl);
    return NULL;
  }

  ret = transport_connect (trans);

  if (ret != 0) {
    gf_log ("booster", GF_LOG_ERROR, "could not connect to translator");
    free (xl);
    free (trans);
    return NULL;
  }

  xl->private = transport_ref (trans);

  filep = calloc (1, sizeof (*filep));
  filep->transport = trans;
  memcpy (filep->handle, handle, 8);

  return filep;
}


int
glusterfs_booster_bridge_preadv (struct file *filep, struct iovec *vector,
				 int count, off_t offset)
{
  int ret;

  printf ("reading\n");
}


int
glusterfs_booster_bridge_pwritev (struct file *filep, struct iovec *vector,
				  int count, off_t offset)
{
  int ret;

  printf ("writing\n");
}
