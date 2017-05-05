/*
  Copyright (c) 2006, 2007, 2008, 2009 Z RESEARCH, Inc. <http://www.zresearch.com>
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

#ifndef _CONFIG_H
#define _CONFIG_H
#include "config.h"
#endif
#include <inttypes.h>


#include "glusterfs.h"
#include "client-protocol.h"
#include "compat.h"
#include "dict.h"
#include "protocol.h"
#include "transport.h"
#include "xlator.h"
#include "logging.h"
#include "timer.h"
#include "defaults.h"
#include "compat.h"
#include "compat-errno.h"

#include <sys/resource.h>
#include <inttypes.h>

/* for default_*_cbk functions */
#include "defaults.c"
#include "saved-frames.h"


int protocol_client_cleanup (transport_t *trans);
int protocol_client_interpret (xlator_t *this, transport_t *trans,
                               char *hdr_p, size_t hdrlen,
                               char *buf_p, size_t buflen);
int
protocol_client_xfer (call_frame_t *frame, xlator_t *this, transport_t *trans,
                      int type, int op,
                      gf_hdr_common_t *hdr, size_t hdrlen,
                      struct iovec *vector, int count,
                      dict_t *refs);

static gf_op_t gf_fops[];
static gf_op_t gf_mops[];
static gf_op_t gf_cbks[];


static ino_t
this_ino_get_from_inode (inode_t *inode, xlator_t *this)
{
	ino_t   ino = 0;
	int32_t ret = 0;

	GF_VALIDATE_OR_GOTO ("client", this, out);
	GF_VALIDATE_OR_GOTO (this->name, inode, out);

	if (inode->ino == 1) {
		ino = 1;
		goto out;
	}

	ret = inode_ctx_get (inode, this, &ino);

	if (inode->ino && ret < 0) {
		gf_log (this->name, GF_LOG_ERROR,
			"(%"PRId64"): failed to get remote inode number",
			inode->ino);
	}

out:
	return ino;
}


static ino_t
this_ino_get (loc_t *loc, xlator_t *this, int32_t which)
{
	ino_t    ino = 0;
	int32_t  ret = 0;
	inode_t *inode = NULL;

	GF_VALIDATE_OR_GOTO ("client", this, out);
	
	if (which == GF_CLIENT_INODE_SELF) {
		inode = loc->inode;
	} else if (which == GF_CLIENT_INODE_PARENT) {
		inode = loc->parent;
	}
	GF_VALIDATE_OR_GOTO (this->name, inode, out);

	if (inode->ino == 1) {
		ino = 1;
		goto out;
	}

	ret = inode_ctx_get (inode, this, &ino);

	if (inode->ino && ret < 0) {
		gf_log (this->name, GF_LOG_ERROR,
			"%s(%s - %"PRId64") failed to get remote inode number",
			loc->path, 
			(which == GF_CLIENT_INODE_SELF? "self" : "parent"), 
			inode->ino);
	}

out:
	return ino;
}


static void
this_ino_set (loc_t *loc, xlator_t *this, ino_t ino)
{
	ino_t    old_ino = 0;
	int32_t  ret = -1;
	inode_t *inode = NULL;

	GF_VALIDATE_OR_GOTO ("client", this, out);

	inode = loc->inode;
	GF_VALIDATE_OR_GOTO (this->name, inode, out);

	ret = inode_ctx_get (inode, this, &old_ino);

	if (old_ino != ino) {
		if (old_ino)
			gf_log (this->name, GF_LOG_DEBUG,
				"%s: inode number changed from %"PRId64" "
				"to %"PRId64,
				loc->path, old_ino, ino);

		ret = inode_ctx_put (inode, this, ino);
		if (ret < 0) {
			gf_log (this->name, GF_LOG_ERROR,
				"%s (%"PRId64"): failed to set remote "
				"inode number to inode ctx",
				loc->path, ino);
		}
	}
out:
	return;
}


static int
this_fd_get (fd_t *file, xlator_t *this, int64_t *remote_fd)
{
	int ret = 0;
	int dict_ret = -1;
	uint64_t tmp_fd = 0;

	GF_VALIDATE_OR_GOTO ("client", this, out);
	GF_VALIDATE_OR_GOTO (this->name, file, out);
	GF_VALIDATE_OR_GOTO (this->name, remote_fd, out);

	dict_ret = fd_ctx_get (file, this, &tmp_fd);

	if (dict_ret < 0) {
		ret = -1;
	}
	*remote_fd = (int64_t)tmp_fd;
out:
	return ret;
}


static void
this_fd_set (fd_t *file, xlator_t *this, loc_t *loc, int64_t fd)
{
	uint64_t old_fd = 0;
	int32_t ret = -1;

	GF_VALIDATE_OR_GOTO ("client", this, out);
	GF_VALIDATE_OR_GOTO (this->name, file, out);

	ret = fd_ctx_get (file, this, &old_fd);
	if (ret >= 0) {
		gf_log (this->name, GF_LOG_WARNING,
			"%s (%"PRId64"): trying duplicate remote fd set. "
			"%"PRId64" over-rides %"PRId64,
			loc->path, loc->inode->ino, fd, old_fd);
	}

	ret = fd_ctx_set (file, this, (uint64_t)fd);
	if (ret < 0) {
		gf_log (this->name, GF_LOG_ERROR,
			"%s (%"PRId64"): failed to set remote fd",
			loc->path, loc->inode->ino);
	}
out:
	return;
}


static int 
client_local_wipe (client_local_t *local)
{
	if (local) {
		loc_wipe (&local->loc);

		if (local->fd)
			fd_unref (local->fd);

		free (local);
	} 
	
	return 0;
}

/*
 * lookup_frame - lookup call frame corresponding to a given callid
 * @trans: transport object
 * @callid: call id of the frame
 *
 * not for external reference
 */

static call_frame_t *
lookup_frame (transport_t *trans, int32_t op, int8_t type, int64_t callid)
{
	client_connection_t *conn = NULL;
	call_frame_t        *frame = NULL;

	conn = trans->xl_private;

	pthread_mutex_lock (&conn->lock);
	{
		frame = saved_frames_get (conn->saved_frames,
					  op, type, callid);
	}
	pthread_mutex_unlock (&conn->lock);

	return frame;
}


static void
call_bail (void *data)
{
	client_connection_t *conn = NULL;
	struct timeval       current;
	int32_t              bail_out = 0;
	transport_t         *trans = NULL;

	GF_VALIDATE_OR_GOTO("client", data, out);
	trans = data;

	conn = trans->xl_private;

	gettimeofday (&current, NULL);
	pthread_mutex_lock (&conn->lock);
	{
		/* Chaining to get call-always functionality from 
		   call-once timer */
		if (conn->timer) {
			struct timeval timeout = {0,};
			gf_timer_cbk_t timer_cbk = conn->timer->cbk;

			timeout.tv_sec = 10;
			timeout.tv_usec = 0;

			gf_timer_call_cancel (trans->xl->ctx, conn->timer);
			conn->timer = gf_timer_call_after (trans->xl->ctx,
							       timeout,
							       timer_cbk,
							       trans);
			if (conn->timer == NULL) {
				gf_log (trans->xl->name, GF_LOG_DEBUG,
					"Cannot create bailout timer");
			}
		}

		if (((conn->saved_frames->count > 0) &&
		     (RECEIVE_TIMEOUT(conn, current)) && 
		     (SEND_TIMEOUT(conn, current)))) {

			struct tm last_sent_tm, last_received_tm;
			char last_sent[32] = {0,}, last_received[32] = {0,};

			bail_out = 1;
			
			localtime_r (&conn->last_sent.tv_sec, 
				     &last_sent_tm);
			localtime_r (&conn->last_received.tv_sec, 
				     &last_received_tm);
			
			strftime (last_sent, 32, 
				  "%Y-%m-%d %H:%M:%S", &last_sent_tm);
			strftime (last_received, 32, 
				  "%Y-%m-%d %H:%M:%S", &last_received_tm);
			
			gf_log (trans->xl->name, GF_LOG_ERROR,
				"activating bail-out. pending frames = %d. "
				"last sent = %s. last received = %s. "
				"transport-timeout = %d",
				(int32_t) conn->saved_frames->count,
				last_sent, last_received,
				conn->transport_timeout);
		}
	}

	if (bail_out) {
		conn->ping_started = 0;
	}

	pthread_mutex_unlock (&conn->lock);

	if (bail_out) {
		gf_log (trans->xl->name, GF_LOG_CRITICAL,
			"bailing transport");
		transport_disconnect (trans);
	}
out:
	return;
}


void
save_frame (transport_t *trans, call_frame_t *frame,
	    int32_t op, int8_t type, uint64_t callid)
{
	client_connection_t *conn = NULL;
	struct timeval       timeout = {0, };


	conn = trans->xl_private;

	saved_frames_put (conn->saved_frames, frame, op, type, callid);

	if (conn->timer == NULL) {
		timeout.tv_sec  = 10;
		timeout.tv_usec = 0;
		conn->timer = gf_timer_call_after (trans->xl->ctx, timeout,
						   call_bail, (void *) trans);
       }
}


int
client_get_forgets (xlator_t *this, client_forget_t *forget) 
{
	call_frame_t        *fr = NULL;
	gf_hdr_common_t     *hdr = NULL;
	size_t               hdrlen = 0;
	gf_cbk_forget_req_t *req = NULL;
	int                  ret = -1;
	client_conf_t       *conf = NULL;
	int                  count = 0;
	int                  index = 0;

	conf = this->private;

	if (conf->forget.count > 0) {
		count = conf->forget.count;
		
		hdrlen = gf_hdr_len (req, (count * sizeof (int64_t)));
		hdr    = gf_hdr_new (req, (count * sizeof (int64_t)));
		GF_VALIDATE_OR_GOTO (this->name, hdr, out);
			
		req    = gf_param (hdr);
		
		req->count = hton32 (count);
		for (index = 0; index < count; index++) {
			req->ino_array[index] = 
				hton64 (conf->forget.ino_array[index]);
		}
		
		fr = create_frame (this, this->ctx->pool);
		GF_VALIDATE_OR_GOTO (this->name, fr, out);

		conf->forget.frames_in_transit++;

		forget->frame = fr;
		forget->hdr   = hdr;
		forget->hdrlen = hdrlen;
		
		ret = count;

		conf->forget.count = 0;
	}
 out:
	return ret;
}


void 
client_ping_timer_expired (void *data)
{
	xlator_t            *this = NULL;
	transport_t         *trans = NULL;
	client_conf_t       *conf = NULL;
	client_connection_t *conn = NULL;

	trans = data;
	this  = trans->xl;
	conf  = this->private;
	conn  = trans->xl_private;

	gf_log (this->name, GF_LOG_ERROR, 
		"ping timer expired! bailing transport");

	pthread_mutex_lock (&conn->lock);
	{
		if (conn->ping_timer)
			gf_timer_call_cancel (trans->xl->ctx, 
					      conn->ping_timer);

		conn->ping_started = 0;
		conn->ping_timer = NULL;
	}
	pthread_mutex_unlock (&conn->lock);
	transport_disconnect (trans);
}


void
client_start_ping (void *data)
{
	xlator_t            *this = NULL;
	transport_t         *trans = NULL;
	client_conf_t       *conf = NULL;
	client_connection_t *conn = NULL;
	int32_t              ret = -1;
	gf_hdr_common_t     *hdr = NULL;
	struct timeval       timeout = {0, };
	call_frame_t        *dummy_frame = NULL;
	size_t               hdrlen = -1;
	gf_mop_ping_req_t   *req = NULL;


	trans = data;
	this  = trans->xl;
	conf  = this->private;
	conn  = trans->xl_private;

	pthread_mutex_lock (&conn->lock);
	{
		if ((conn->saved_frames->count == 0) || 
		    !conn->connected) {
			/* using goto looked ugly here, 
			 * hence getting out this way */
			if (conn->ping_timer)
				gf_timer_call_cancel (trans->xl->ctx, 
						      conn->ping_timer);
			conn->ping_timer = NULL;
			conn->ping_started = 0;
			/* unlock */
			pthread_mutex_unlock (&conn->lock);
			return;
		}

		if (conn->saved_frames->count < 0) {
			gf_log (this->name, GF_LOG_ERROR,
				"saved_frames->count is %"PRId64, 
				conn->saved_frames->count);
			conn->saved_frames->count = 0;
		}
		timeout.tv_sec = conn->ping_timeout;
		timeout.tv_usec = 0;
		
		conn->ping_timer = 
			gf_timer_call_after (trans->xl->ctx, timeout,
					     client_ping_timer_expired,
					     (void *) trans);

		if (conn->ping_timer == NULL) {
			gf_log (this->name, GF_LOG_ERROR,
				"unable to setup timer");
		} else
			conn->ping_started = 1;
	}
	pthread_mutex_unlock (&conn->lock);

	hdrlen = gf_hdr_len (req, 0);
	hdr    = gf_hdr_new (req, 0);

	dummy_frame = create_frame (this, this->ctx->pool);
	dummy_frame->local = trans;

	ret = protocol_client_xfer (dummy_frame, this, trans,
				    GF_OP_TYPE_MOP_REQUEST, GF_MOP_PING,
				    hdr, hdrlen, NULL, 0, NULL);
}


int
client_ping_cbk (call_frame_t *frame, gf_hdr_common_t *hdr, size_t hdrlen,
		 char *buf, size_t buflen)
{
	xlator_t            *this = NULL;
	transport_t         *trans = NULL;
	client_conf_t       *conf = NULL;
	client_connection_t *conn = NULL;
	struct timeval       timeout = {0, };
	int                  op_ret = 0;

	trans  = frame->local; frame->local = NULL;
	this   = trans->xl;
	conf   = this->private;
	conn   = trans->xl_private;

	op_ret = ntoh32 (hdr->rsp.op_ret);

	if (op_ret == -1) {
		/* timer expired and transport bailed out */
		gf_log (this->name, GF_LOG_ERROR, "timer must have expired");
		goto out;
	}

	pthread_mutex_lock (&conn->lock);
	{
		timeout.tv_sec  = conn->ping_timeout;
		timeout.tv_usec = 0;

		gf_timer_call_cancel (trans->xl->ctx, 
				      conn->ping_timer);
	
		conn->ping_timer = 
			gf_timer_call_after (trans->xl->ctx, timeout,
					     client_start_ping, (void *)trans);
		if (conn->ping_timer == NULL)
			gf_log (this->name, GF_LOG_ERROR,
				"gf_timer_call_after() returned NULL");
	}
	pthread_mutex_unlock (&conn->lock);
out:
	STACK_DESTROY (frame->root);
	return 0;
}


int
protocol_client_xfer (call_frame_t *frame, xlator_t *this, transport_t *trans,
                      int type, int op,
                      gf_hdr_common_t *hdr, size_t hdrlen,
                      struct iovec *vector, int count,
                      dict_t *refs)
{
	client_conf_t        *conf = NULL;
	client_connection_t  *conn = NULL;
	uint64_t              callid = 0;
	int32_t               ret = -1;
	int                   start_ping = 0;
	gf_hdr_common_t       rsphdr = {0, };
	client_forget_t       forget = {0, };
	uint8_t               send_forget = 0;


	conf  = this->private;

	if (!trans) {
		/* default to bulk op since it is 'safer' */
		trans = conf->transport[CHANNEL_BULK];
	}
	conn  = trans->xl_private;

	if (!((type == GF_OP_TYPE_CBK_REQUEST) && 
	      (op == GF_CBK_FORGET))) 
	{
		LOCK (&conf->forget.lock);
		{
			ret = client_get_forgets (this, &forget);
			if (ret <= 0)
				send_forget = 0;
			else
				send_forget = 1;
		}
		UNLOCK (&conf->forget.lock);

		if (send_forget) {
			ret = protocol_client_xfer (forget.frame, this, NULL,
						    GF_OP_TYPE_CBK_REQUEST, 
						    GF_CBK_FORGET,
						    forget.hdr, forget.hdrlen, 
						    NULL, 0, NULL);
		}
	}

	pthread_mutex_lock (&conn->lock);
	{
		callid = ++conn->callid;

		hdr->callid = hton64 (callid);
		hdr->op     = hton32 (op);
		hdr->type   = hton32 (type);

		if (frame) {
			hdr->req.uid = hton32 (frame->root->uid);
			hdr->req.gid = hton32 (frame->root->gid);
			hdr->req.pid = hton32 (frame->root->pid);
		}

		if (conn->connected == 0)
			transport_connect (trans);

		ret = -1;

		if (conn->connected ||
		    ((type == GF_OP_TYPE_MOP_REQUEST) &&
		     (op == GF_MOP_SETVOLUME))) {
			ret = transport_submit (trans, (char *)hdr, hdrlen,
						vector, count, refs);
		}
		
		if ((ret >= 0) && frame) {
			/* TODO: check this logic */
			gettimeofday (&conn->last_sent, NULL);
			save_frame (trans, frame, op, type, callid);
		}

		if (!conn->ping_started && (ret >= 0)) {
			start_ping = 1;
		}
	}
	pthread_mutex_unlock (&conn->lock);

	if (start_ping)
		client_start_ping ((void *) trans);

	if (frame && (ret < 0)) {
		rsphdr.op = op;
		rsphdr.rsp.op_ret   = hton32 (-1);
		rsphdr.rsp.op_errno = hton32 (ENOTCONN);

		if (type == GF_OP_TYPE_FOP_REQUEST) {
			rsphdr.type = GF_OP_TYPE_FOP_REPLY;
			gf_fops[op] (frame, &rsphdr, sizeof (rsphdr), NULL, 0);
		} else if (type == GF_OP_TYPE_MOP_REQUEST) {
			rsphdr.type = GF_OP_TYPE_MOP_REPLY;
			gf_mops[op] (frame, &rsphdr, sizeof (rsphdr), NULL, 0);
		} else {
			rsphdr.type = GF_OP_TYPE_CBK_REPLY;
			gf_cbks[op] (frame, &rsphdr, sizeof (rsphdr), NULL, 0);
		}
	}

	return ret;
}



/**
 * client_create - create function for client protocol
 * @frame: call frame
 * @this: this translator structure
 * @path: complete path to file
 * @flags: create flags
 * @mode: create mode
 *
 * external reference through client_protocol_xlator->fops->create
 */

int
client_create (call_frame_t *frame, xlator_t *this,
               loc_t *loc, int32_t flags,
               mode_t mode, fd_t *fd)
{
	gf_hdr_common_t     *hdr = NULL;
	gf_fop_create_req_t *req = NULL;
	size_t               hdrlen = 0;
	size_t               pathlen = 0;
	size_t               baselen = 0;
	int32_t              ret = -1;
	ino_t                par = 0;
	client_conf_t       *conf = NULL;
	client_local_t      *local = NULL;


	conf = this->private;

	if (conf->child) {
		STACK_WIND (frame, default_create_cbk,
			    conf->child,
			    conf->child->fops->create,
			    loc, flags, mode, fd);
		return 0;
	}

	local = calloc (1, sizeof (*local));
	GF_VALIDATE_OR_GOTO(this->name, local, unwind);

	local->fd = fd_ref (fd);
	loc_copy (&local->loc, loc);
	
	frame->local = local;

	pathlen = STRLEN_0(loc->path);
	baselen = STRLEN_0(loc->name);
	par = this_ino_get (loc, this, GF_CLIENT_INODE_PARENT);

	hdrlen = gf_hdr_len (req, pathlen + baselen);
	hdr    = gf_hdr_new (req, pathlen + baselen);
	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->flags   = hton32 (flags);
	req->mode    = hton32 (mode);
	req->par     = hton64 (par);
	strcpy (req->path, loc->path);
	strcpy (req->bname + pathlen, loc->name);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_LOWLAT),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_CREATE,
				    hdr, hdrlen, NULL, 0, NULL);
	return ret;
unwind:
	if (hdr)
		free (hdr);
	STACK_UNWIND(frame, -1, EINVAL, fd, NULL, NULL);
	return 0;

}

/**
 * client_open - open function for client protocol
 * @frame: call frame
 * @this: this translator structure
 * @loc: location of file
 * @flags: open flags
 * @mode: open modes
 *
 * external reference through client_protocol_xlator->fops->open
 */
int
client_open (call_frame_t *frame, xlator_t *this,
             loc_t *loc, int32_t flags, fd_t *fd)
{
	int                 ret = -1;
	gf_hdr_common_t    *hdr = NULL;
	size_t              hdrlen = 0;
	gf_fop_open_req_t  *req = NULL;
	size_t              pathlen = 0;
	ino_t               ino = 0;
	client_conf_t      *conf = NULL;
	client_local_t     *local = NULL;

	conf = this->private;
	if (conf->child) {
		/* */
		STACK_WIND (frame, default_open_cbk,
			    conf->child,
			    conf->child->fops->open,
			    loc, flags, fd);
		
		return 0;
	}

	local = calloc (1, sizeof (*local));
	GF_VALIDATE_OR_GOTO(this->name, local, unwind);

	local->fd = fd_ref (fd);
	loc_copy (&local->loc, loc);	

	frame->local = local;

	pathlen = STRLEN_0(loc->path);
	ino = this_ino_get (loc, this, GF_CLIENT_INODE_SELF);

	hdrlen = gf_hdr_len (req, pathlen);
	hdr    = gf_hdr_new (req, pathlen);
	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->ino   = hton64 (ino);
	req->flags = hton32 (flags);
	strcpy (req->path, loc->path);
	
	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_LOWLAT),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_OPEN,
				    hdr, hdrlen, NULL, 0, NULL);

	return ret;
unwind:
	if (hdr)
		free (hdr);
	STACK_UNWIND(frame, -1, EINVAL, fd);
	return 0;

}


/**
 * client_stat - stat function for client protocol
 * @frame: call frame
 * @this: this translator structure
 * @loc: location
 *
 * external reference through client_protocol_xlator->fops->stat
 */
int32_t
client_stat (call_frame_t *frame,
             xlator_t *this,
             loc_t *loc)
{
	gf_hdr_common_t   *hdr = NULL;
	gf_fop_stat_req_t *req = NULL;
	size_t hdrlen = -1;
	int32_t ret = -1;
	size_t  pathlen = 0;
	ino_t   ino = 0;
	client_conf_t *conf = this->private;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_stat_cbk,
			    conf->child,
			    conf->child->fops->stat,
			    loc);
		
		return 0;
	}

	pathlen = STRLEN_0(loc->path);
	ino = this_ino_get (loc, this, GF_CLIENT_INODE_SELF);

	hdrlen = gf_hdr_len (req, pathlen);
	hdr    = gf_hdr_new (req, pathlen);
	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->ino  = hton64 (ino);
	strcpy (req->path, loc->path);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_LOWLAT),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_STAT,
				    hdr, hdrlen, NULL, 0, NULL);

	return ret;
unwind:
	if (hdr)
		free (hdr);
	STACK_UNWIND(frame, -1, EINVAL, NULL);
	return 0;

}


/**
 * client_readlink - readlink function for client protocol
 * @frame: call frame
 * @this: this translator structure
 * @loc: location
 * @size:
 *
 * external reference through client_protocol_xlator->fops->readlink
 */
int32_t
client_readlink (call_frame_t *frame,
                 xlator_t *this,
                 loc_t *loc,
                 size_t size)
{
	gf_hdr_common_t       *hdr = NULL;
	gf_fop_readlink_req_t *req = NULL;
	size_t hdrlen = -1;
	int    ret = -1;
	size_t pathlen = 0;
	ino_t  ino = 0;
	client_conf_t *conf = this->private;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_readlink_cbk,
			    conf->child,
			    conf->child->fops->readlink,
			    loc,
			    size);
		
		return 0;
	}

	pathlen = STRLEN_0(loc->path);
	ino = this_ino_get (loc, this, GF_CLIENT_INODE_SELF);

	hdrlen = gf_hdr_len (req, pathlen);
	hdr    = gf_hdr_new (req, pathlen);
	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->ino  = hton64 (ino);
	req->size = hton32 (size);
	strcpy (req->path, loc->path);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_LOWLAT),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_READLINK,
				    hdr, hdrlen, NULL, 0, NULL);

	return ret;
unwind:
	if (hdr)
		free (hdr);
	STACK_UNWIND(frame, -1, EINVAL, NULL);
	return 0;

}


/**
 * client_mknod - mknod function for client protocol
 * @frame: call frame
 * @this: this translator structure
 * @path: pathname of node
 * @mode:
 * @dev:
 *
 * external reference through client_protocol_xlator->fops->mknod
 */
int32_t
client_mknod (call_frame_t *frame,
              xlator_t *this,
              loc_t *loc,
              mode_t mode,
              dev_t dev)
{
	gf_hdr_common_t    *hdr = NULL;
	gf_fop_mknod_req_t *req = NULL;
	size_t hdrlen = -1;
	int    ret = -1;
	size_t pathlen = 0;
	size_t baselen = 0;
	ino_t  par = 0;
	client_conf_t *conf = this->private;
	client_local_t *local = NULL;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_mknod_cbk,
			    conf->child,
			    conf->child->fops->mknod,
			    loc, mode, dev);

		return 0;
	}

	local = calloc (1, sizeof (*local));
	GF_VALIDATE_OR_GOTO(this->name, local, unwind);

	loc_copy (&local->loc, loc);	

	frame->local = local;

	pathlen = STRLEN_0(loc->path);
	baselen = STRLEN_0(loc->name);
	par = this_ino_get (loc, this, GF_CLIENT_INODE_PARENT);

	hdrlen = gf_hdr_len (req, pathlen + baselen);
	hdr    = gf_hdr_new (req, pathlen + baselen);
	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->par  = hton64 (par);
	req->mode = hton32 (mode);
	req->dev  = hton64 (dev);
	strcpy (req->path, loc->path);
	strcpy (req->bname + pathlen, loc->name);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_LOWLAT),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_MKNOD,
				    hdr, hdrlen, NULL, 0, NULL);

	return ret;
unwind:
	if (hdr)
		free (hdr);
	STACK_UNWIND(frame, -1, EINVAL, loc->inode, NULL);
	return 0;

}


/**
 * client_mkdir - mkdir function for client protocol
 * @frame: call frame
 * @this: this translator structure
 * @path: pathname of directory
 * @mode:
 *
 * external reference through client_protocol_xlator->fops->mkdir
 */
int32_t
client_mkdir (call_frame_t *frame,
              xlator_t *this,
              loc_t *loc,
              mode_t mode)
{
	gf_hdr_common_t    *hdr = NULL;
	gf_fop_mkdir_req_t *req = NULL;
	size_t hdrlen = -1;
	int    ret = -1;
	size_t pathlen = 0;
	size_t baselen = 0;
	ino_t  par = 0;
	client_conf_t *conf = this->private;
	client_local_t *local = NULL;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_mkdir_cbk,
			    conf->child,
			    conf->child->fops->mkdir,
			    loc, mode);
		
		return 0;
	}

	local = calloc (1, sizeof (*local));
	GF_VALIDATE_OR_GOTO(this->name, local, unwind);
	
	loc_copy (&local->loc, loc);

	frame->local = local;

	pathlen = STRLEN_0(loc->path);
	baselen = STRLEN_0(loc->name);
	par = this_ino_get (loc, this, GF_CLIENT_INODE_PARENT);

	hdrlen = gf_hdr_len (req, pathlen + baselen);
	hdr    = gf_hdr_new (req, pathlen + baselen);
	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->par  = hton64 (par);
	req->mode = hton32 (mode);
	strcpy (req->path, loc->path);
	strcpy (req->bname + pathlen, loc->name);
	
	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_LOWLAT),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_MKDIR,
				    hdr, hdrlen, NULL, 0, NULL);

	return ret;
unwind:
	if (hdr)
		free (hdr);
	STACK_UNWIND(frame, -1, EINVAL, loc->inode, NULL);
	return 0;

}



/**
 * client_unlink - unlink function for client protocol
 * @frame: call frame
 * @this: this translator structure
 * @loc: location of file
 *
 * external reference through client_protocol_xlator->fops->unlink
 */
int32_t
client_unlink (call_frame_t *frame,
               xlator_t *this,
               loc_t *loc)
{
	gf_hdr_common_t     *hdr = NULL;
	gf_fop_unlink_req_t *req = NULL;
	size_t hdrlen = -1;
	int    ret = -1;
	size_t pathlen = 0;
	size_t baselen = 0;
	ino_t  par = 0;
	client_conf_t *conf = this->private;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_unlink_cbk,
			    conf->child,
			    conf->child->fops->unlink,
			    loc);
		
		return 0;
	}

	pathlen = STRLEN_0(loc->path);
	baselen = STRLEN_0(loc->name);
	par = this_ino_get (loc, this, GF_CLIENT_INODE_PARENT);

	hdrlen = gf_hdr_len (req, pathlen + baselen);
	hdr    = gf_hdr_new (req, pathlen + baselen);
	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->par  = hton64 (par);
	strcpy (req->path, loc->path);
	strcpy (req->bname + pathlen, loc->name);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_BULK),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_UNLINK,
				    hdr, hdrlen, NULL, 0, NULL);

	return ret;
unwind:
	if (hdr)
		free (hdr);
	STACK_UNWIND(frame, -1, EINVAL);
	return 0;

}

/**
 * client_rmdir - rmdir function for client protocol
 * @frame: call frame
 * @this: this translator structure
 * @loc: location
 *
 * external reference through client_protocol_xlator->fops->rmdir
 */
int32_t
client_rmdir (call_frame_t *frame,
              xlator_t *this,
              loc_t *loc)
{
	gf_hdr_common_t    *hdr = NULL;
	gf_fop_rmdir_req_t *req = NULL;
	size_t hdrlen = -1;
	int    ret = -1;
	size_t pathlen = 0;
	size_t baselen = 0;
	ino_t  par = 0;
	client_conf_t *conf = this->private;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_rmdir_cbk,
			    conf->child,
			    conf->child->fops->rmdir,
			    loc);
		
		return 0;
	}

	pathlen = STRLEN_0(loc->path);
	baselen = STRLEN_0(loc->name);
	par = this_ino_get (loc, this, GF_CLIENT_INODE_PARENT);

	hdrlen = gf_hdr_len (req, pathlen + baselen);
	hdr    = gf_hdr_new (req, pathlen + baselen);
	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->par  = hton64 (par);
	strcpy (req->path, loc->path);
	strcpy (req->bname + pathlen, loc->name);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_BULK),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_RMDIR,
				    hdr, hdrlen, NULL, 0, NULL);

	return ret;
unwind:
	if (hdr)
		free (hdr);
	STACK_UNWIND(frame, -1, EINVAL);
	return 0;

}



/**
 * client_symlink - symlink function for client protocol
 * @frame: call frame
 * @this: this translator structure
 * @oldpath: pathname of target
 * @newpath: pathname of symlink
 *
 * external reference through client_protocol_xlator->fops->symlink
 */
int32_t
client_symlink (call_frame_t *frame,
                xlator_t *this,
                const char *linkname,
                loc_t *loc)
{
	int ret = -1;
	gf_hdr_common_t      *hdr = NULL;
	gf_fop_symlink_req_t *req = NULL;
	size_t hdrlen  = 0;
	size_t pathlen = 0;
	size_t newlen  = 0;
	size_t baselen = 0;
	ino_t par = 0;
	client_conf_t *conf = this->private;
	client_local_t *local = NULL;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_symlink_cbk,
			    conf->child,
			    conf->child->fops->symlink,
			    linkname, loc);
		
		return 0;
	}

	local = calloc (1, sizeof (*local));
	GF_VALIDATE_OR_GOTO(this->name, local, unwind);
	
	loc_copy (&local->loc, loc);

	frame->local = local;

	pathlen = STRLEN_0 (loc->path);
	baselen = STRLEN_0 (loc->name);
	newlen = STRLEN_0 (linkname);
	par = this_ino_get (loc, this, GF_CLIENT_INODE_PARENT);

	hdrlen = gf_hdr_len (req, pathlen + baselen + newlen);
	hdr    = gf_hdr_new (req, pathlen + baselen + newlen);
	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->par =  hton64 (par);
	strcpy (req->path, loc->path);
	strcpy (req->bname + pathlen, loc->name);
	strcpy (req->linkname + pathlen + baselen, linkname);
	
	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_LOWLAT),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_SYMLINK,
				    hdr, hdrlen, NULL, 0, NULL);
	return ret;
unwind:
	if (hdr)
		free (hdr);
	STACK_UNWIND(frame, -1, EINVAL, loc->inode, NULL);
	return 0;

}


/**
 * client_rename - rename function for client protocol
 * @frame: call frame
 * @this: this translator structure
 * @oldloc: location of old pathname
 * @newloc: location of new pathname
 *
 * external reference through client_protocol_xlator->fops->rename
 */
int32_t
client_rename (call_frame_t *frame,
               xlator_t *this,
               loc_t *oldloc,
               loc_t *newloc)
{
	int ret = -1;
	gf_hdr_common_t *hdr = NULL;
	gf_fop_rename_req_t *req = NULL;
	size_t hdrlen = 0;
	size_t oldpathlen = 0;
	size_t oldbaselen = 0;
	size_t newpathlen = 0;
	size_t newbaselen = 0;
	ino_t  oldpar = 0;
	ino_t  newpar = 0;
	client_conf_t *conf = this->private;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_rename_cbk,
			    conf->child,
			    conf->child->fops->rename,
			    oldloc, newloc);
		
		return 0;
	}

	oldpathlen = STRLEN_0(oldloc->path);
	oldbaselen = STRLEN_0(oldloc->name);
	newpathlen = STRLEN_0(newloc->path);
	newbaselen = STRLEN_0(newloc->name);
	oldpar = this_ino_get (oldloc, this, GF_CLIENT_INODE_PARENT);
	newpar = this_ino_get (newloc, this, GF_CLIENT_INODE_PARENT);

	hdrlen = gf_hdr_len (req, (oldpathlen + oldbaselen + 
				   newpathlen + newbaselen));
	hdr    = gf_hdr_new (req, (oldpathlen + oldbaselen + 
				   newpathlen + newbaselen));

	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->oldpar = hton64 (oldpar);
	req->newpar = hton64 (newpar);

	strcpy (req->oldpath, oldloc->path);
	strcpy (req->oldbname + oldpathlen, oldloc->name);
	strcpy (req->newpath  + oldpathlen + oldbaselen, newloc->path);
	strcpy (req->newbname + oldpathlen + oldbaselen + newpathlen, 
		newloc->name);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_LOWLAT),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_RENAME,
				    hdr, hdrlen, NULL, 0, NULL);
	return ret;
unwind:
	if (hdr)
		free (hdr);
	STACK_UNWIND(frame, -1, EINVAL, NULL);
	return 0;

}



/**
 * client_link - link function for client protocol
 * @frame: call frame
 * @this: this translator structure
 * @oldloc: location of old pathname
 * @newpath: new pathname
 *
 * external reference through client_protocol_xlator->fops->link
 */

int32_t
client_link (call_frame_t *frame,
             xlator_t *this,
             loc_t *oldloc,
             loc_t *newloc)
{
	int ret = -1;
	gf_hdr_common_t *hdr = NULL;
	gf_fop_link_req_t *req = NULL;
	size_t hdrlen = 0;
	size_t oldpathlen = 0;
	size_t newpathlen = 0;
	size_t newbaselen = 0;
	ino_t  oldino = 0;
	ino_t  newpar = 0;
	client_conf_t *conf = this->private;
	client_local_t *local = NULL;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_link_cbk,
			    conf->child,
			    conf->child->fops->link,
			    oldloc, newloc);
		
		return 0;
	}

	local = calloc (1, sizeof (*local));
	GF_VALIDATE_OR_GOTO(this->name, local, unwind);

	loc_copy (&local->loc, oldloc);

	frame->local = local;

	oldpathlen = STRLEN_0(oldloc->path);
	newpathlen = STRLEN_0(newloc->path);
	newbaselen = STRLEN_0(newloc->name);
	oldino = this_ino_get (oldloc, this, GF_CLIENT_INODE_SELF);
	newpar = this_ino_get (newloc, this, GF_CLIENT_INODE_PARENT);

	hdrlen = gf_hdr_len (req, oldpathlen + newpathlen + newbaselen);
	hdr    = gf_hdr_new (req, oldpathlen + newpathlen + newbaselen);
	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req    = gf_param (hdr);

	strcpy (req->oldpath, oldloc->path);
	strcpy (req->newpath  + oldpathlen, newloc->path);
	strcpy (req->newbname + oldpathlen + newpathlen, newloc->name);

	req->oldino = hton64 (oldino);
	req->newpar = hton64 (newpar);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_LOWLAT),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_LINK,
				    hdr, hdrlen, NULL, 0, NULL);
	return ret;
unwind:
	if (hdr)
		free (hdr);
	STACK_UNWIND(frame, -1, EINVAL, oldloc->inode, NULL);
	return 0;
}



/**
 * client_chmod - chmod function for client protocol
 * @frame: call frame
 * @this: this translator structure
 * @loc: location
 * @mode:
 *
 * external reference through client_protocol_xlator->fops->chmod
 */
int32_t
client_chmod (call_frame_t *frame,
              xlator_t *this,
              loc_t *loc,
              mode_t mode)
{
	gf_hdr_common_t    *hdr = NULL;
	gf_fop_chmod_req_t *req = NULL;
	size_t hdrlen = -1;
	int    ret = -1;
	size_t pathlen = 0;
	ino_t  ino = 0;
	client_conf_t *conf = this->private;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_chmod_cbk,
			    conf->child,
			    conf->child->fops->chmod,
			    loc,
			    mode);
		
		return 0;
	}

	pathlen = STRLEN_0(loc->path);
	ino = this_ino_get (loc, this, GF_CLIENT_INODE_SELF);

	hdrlen = gf_hdr_len (req, pathlen);
	hdr    = gf_hdr_new (req, pathlen);
	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->ino     = hton64 (ino);
	req->mode    = hton32 (mode);
	strcpy (req->path, loc->path);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_LOWLAT),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_CHMOD,
				    hdr, hdrlen, NULL, 0, NULL);

	return ret;
unwind:
	if (hdr)
		free (hdr);
	STACK_UNWIND(frame, -1, EINVAL, NULL);
	return 0;

}


/**
 * client_chown - chown function for client protocol
 * @frame: call frame
 * @this: this translator structure
 * @loc: location
 * @uid: uid of new owner
 * @gid: gid of new owner group
 *
 * external reference through client_protocol_xlator->fops->chown
 */
int32_t
client_chown (call_frame_t *frame,
              xlator_t *this,
              loc_t *loc,
              uid_t uid,
              gid_t gid)
{
	gf_hdr_common_t    *hdr = NULL;
	gf_fop_chown_req_t *req = NULL;
	size_t hdrlen = -1;
	int    ret = -1;
	size_t pathlen = 0;
	ino_t  ino = 0;
	client_conf_t *conf = this->private;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_chown_cbk,
			    conf->child,
			    conf->child->fops->chown,
			    loc,
			    uid,
			    gid);
		
		return 0;
	}

	pathlen = STRLEN_0(loc->path);
	ino = this_ino_get (loc, this, GF_CLIENT_INODE_SELF);

	hdrlen = gf_hdr_len (req, pathlen);
	hdr    = gf_hdr_new (req, pathlen);
	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->ino = hton64 (ino);
	req->uid = hton32 (uid);
	req->gid = hton32 (gid);
	strcpy (req->path, loc->path);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_LOWLAT),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_CHOWN,
				    hdr, hdrlen, NULL, 0, NULL);

	return ret;
unwind:
	if (hdr)
		free (hdr);
	STACK_UNWIND(frame, -1, EINVAL, NULL);
	return 0;

}

/**
 * client_truncate - truncate function for client protocol
 * @frame: call frame
 * @this: this translator structure
 * @loc: location
 * @offset:
 *
 * external reference through client_protocol_xlator->fops->truncate
 */
int32_t
client_truncate (call_frame_t *frame,
                 xlator_t *this,
                 loc_t *loc,
                 off_t offset)
{
	gf_hdr_common_t       *hdr = NULL;
	gf_fop_truncate_req_t *req = NULL;
	size_t hdrlen = -1;
	int    ret = -1;
	size_t pathlen = 0;
	ino_t  ino = 0;
	client_conf_t *conf = this->private;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_truncate_cbk,
			    conf->child,
			    conf->child->fops->truncate,
			    loc,
			    offset);
		
		return 0;
	}

	pathlen = STRLEN_0(loc->path);
	ino = this_ino_get (loc, this, GF_CLIENT_INODE_SELF);

	hdrlen = gf_hdr_len (req, pathlen);
	hdr    = gf_hdr_new (req, pathlen);
	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->ino    = hton64 (ino);
	req->offset = hton64 (offset);
	strcpy (req->path, loc->path);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_BULK),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_TRUNCATE,
				    hdr, hdrlen, NULL, 0, NULL);

	return ret;
unwind:
	if (hdr)
		free (hdr);
	STACK_UNWIND(frame, -1, EINVAL, NULL);
	return 0;

}



/**
 * client_utimes - utimes function for client protocol
 * @frame: call frame
 * @this: this translator structure
 * @loc: location
 * @tvp:
 *
 * external reference through client_protocol_xlator->fops->utimes
 */
int32_t
client_utimens (call_frame_t *frame,
                xlator_t *this,
                loc_t *loc,
                struct timespec *tvp)
{
	gf_hdr_common_t      *hdr = NULL;
	gf_fop_utimens_req_t *req = NULL;
	size_t hdrlen = -1;
	int    ret = -1;
	size_t pathlen = 0;
	ino_t  ino = 0;
	client_conf_t *conf = this->private;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_utimens_cbk,
			    conf->child,
			    conf->child->fops->utimens,
			    loc,
			    tvp);
		
		return 0;
	}

	pathlen = STRLEN_0(loc->path);
	ino = this_ino_get (loc, this, GF_CLIENT_INODE_SELF);

	hdrlen = gf_hdr_len (req, pathlen);
	hdr    = gf_hdr_new (req, pathlen);
	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->ino = hton64 (ino);
	gf_timespec_from_timespec (req->tv, tvp);
	strcpy (req->path, loc->path);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_LOWLAT),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_UTIMENS,
				    hdr, hdrlen, NULL, 0, NULL);

	return ret;
unwind:
	if (hdr)
		free (hdr);
	STACK_UNWIND(frame, -1, EINVAL, NULL);
	return 0;

}



/**
 * client_readv - readv function for client protocol
 * @frame: call frame
 * @this: this translator structure
 * @fd: file descriptor structure
 * @size:
 * @offset:
 *
 * external reference through client_protocol_xlator->fops->readv
 */
int32_t
client_readv (call_frame_t *frame,
              xlator_t *this,
              fd_t *fd,
              size_t size,
              off_t offset)
{
	gf_hdr_common_t *hdr = NULL;
	gf_fop_read_req_t *req = NULL;
	size_t hdrlen = 0;
	int64_t remote_fd = -1;
	int ret = -1;
	client_conf_t *conf = this->private;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_readv_cbk,
			    conf->child,
			    conf->child->fops->readv,
			    fd,
			    size,
			    offset);
		
		return 0;
	}

	ret = this_fd_get (fd, this, &remote_fd);
	if (ret == -1) {
		gf_log (this->name, GF_LOG_DEBUG,
			"(%"PRId64"): failed to get remote fd, returning EBADFD",
			fd->inode->ino);
		STACK_UNWIND (frame, -1, EBADFD, NULL, 0, NULL);
		return 0;
	}

	hdrlen = gf_hdr_len (req, 0);
	hdr    = gf_hdr_new (req, 0);
	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->fd     = hton64 (remote_fd);
	req->size   = hton32 (size);
	req->offset = hton64 (offset);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_BULK),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_READ,
				    hdr, hdrlen, NULL, 0, NULL);

	return 0;
unwind:
	if (hdr)
		free (hdr);
	STACK_UNWIND(frame, -1, EINVAL, NULL, 0, NULL);
	return 0;

}


/**
 * client_writev - writev function for client protocol
 * @frame: call frame
 * @this: this translator structure
 * @fd: file descriptor structure
 * @vector:
 * @count:
 * @offset:
 *
 * external reference through client_protocol_xlator->fops->writev
 */
int32_t
client_writev (call_frame_t *frame,
               xlator_t *this,
               fd_t *fd,
               struct iovec *vector,
               int32_t count,
               off_t offset)
{
	gf_hdr_common_t    *hdr = NULL;
	gf_fop_write_req_t *req = NULL;
	size_t  hdrlen = 0;
	int64_t remote_fd = -1;
	int     ret = -1;
	client_conf_t *conf = this->private;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_writev_cbk,
			    conf->child,
			    conf->child->fops->writev,
			    fd,
			    vector,
			    count,
			    offset);
		
		return 0;
	}

	ret = this_fd_get (fd, this, &remote_fd);
	if (ret == -1) {
		gf_log (this->name, GF_LOG_DEBUG,
			"(%"PRId64"): failed to get remote fd. returning EBADFD",
			fd->inode->ino);
		STACK_UNWIND (frame, -1, EBADFD, NULL);
		return 0;
	}

	hdrlen = gf_hdr_len (req, 0);
	hdr    = gf_hdr_new (req, 0);
	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->fd     = hton64 (remote_fd);
	req->size   = hton32 (iov_length (vector, count));
	req->offset = hton64 (offset);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_BULK),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_WRITE,
				    hdr, hdrlen, vector, count,
				    frame->root->req_refs);

	return ret;
unwind:
	if (hdr)
		free (hdr);
	STACK_UNWIND(frame, -1, EINVAL, NULL);
	return 0;

}


/**
 * client_statfs - statfs function for client protocol
 * @frame: call frame
 * @this: this translator structure
 * @loc: location
 *
 * external reference through client_protocol_xlator->fops->statfs
 */
int32_t
client_statfs (call_frame_t *frame,
               xlator_t *this,
               loc_t *loc)
{
	gf_hdr_common_t     *hdr = NULL;
	gf_fop_statfs_req_t *req = NULL;
	size_t hdrlen = -1;
	int    ret = -1;
	size_t pathlen = 0;
	ino_t  ino = 0;
	client_conf_t *conf = this->private;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_statfs_cbk,
			    conf->child,
			    conf->child->fops->statfs,
			    loc);
		
		return 0;
	}

	pathlen = STRLEN_0(loc->path);
	ino = this_ino_get (loc, this, GF_CLIENT_INODE_SELF);

	hdrlen = gf_hdr_len (req, pathlen);
	hdr    = gf_hdr_new (req, pathlen);
	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->ino = hton64 (ino);
	strcpy (req->path, loc->path);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_LOWLAT),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_STATFS,
				    hdr, hdrlen, NULL, 0, NULL);

	return ret;
unwind:
	if (hdr)
		free (hdr);
	STACK_UNWIND(frame, -1, EINVAL, NULL);
	return 0;

}


/**
 * client_flush - flush function for client protocol
 * @frame: call frame
 * @this: this translator structure
 * @fd: file descriptor structure
 *
 * external reference through client_protocol_xlator->fops->flush
 */

int32_t
client_flush (call_frame_t *frame,
              xlator_t *this,
              fd_t *fd)
{
	gf_hdr_common_t *hdr = NULL;
	gf_fop_flush_req_t *req = NULL;
	size_t hdrlen = 0;
	int64_t remote_fd = -1;
	int ret = -1;
	client_conf_t *conf = this->private;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_flush_cbk,
			    conf->child,
			    conf->child->fops->flush,
			    fd);
		
		return 0;
	}

	ret = this_fd_get (fd, this, &remote_fd);
	if (ret == -1) {
		gf_log (this->name, GF_LOG_DEBUG,
			"(%"PRId64"): failed to get remote fd. returning EBADFD",
			fd->inode->ino);
		STACK_UNWIND (frame, -1, EBADFD);
		return 0;
	}

	hdrlen = gf_hdr_len (req, 0);
	hdr    = gf_hdr_new (req, 0);
	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->fd = hton64 (remote_fd);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_BULK),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_FLUSH,
				    hdr, hdrlen, NULL, 0, NULL);

	return 0;
unwind:
	if (hdr)
		free (hdr);
	STACK_UNWIND(frame, -1, EINVAL);
	return 0;

}




/**
 * client_fsync - fsync function for client protocol
 * @frame: call frame
 * @this: this translator structure
 * @fd: file descriptor structure
 * @flags:
 *
 * external reference through client_protocol_xlator->fops->fsync
 */

int32_t
client_fsync (call_frame_t *frame,
              xlator_t *this,
              fd_t *fd,
              int32_t flags)
{
	gf_hdr_common_t *hdr = NULL;
	gf_fop_fsync_req_t *req = NULL;
	size_t hdrlen = 0;
	int64_t remote_fd = -1;
	int32_t ret = -1;
	client_conf_t *conf = this->private;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_fsync_cbk,
			    conf->child,
			    conf->child->fops->fsync,
			    fd,
			    flags);
		
		return 0;
	}

	ret = this_fd_get (fd, this, &remote_fd);
	if (ret == -1) {
		gf_log (this->name, GF_LOG_DEBUG,
			"(%"PRId64"): failed to get remote fd. returning EBADFD", 
			fd->inode->ino);
		STACK_UNWIND(frame, -1, EBADFD);
		return 0;
	}

	hdrlen = gf_hdr_len (req, 0);
	hdr    = gf_hdr_new (req, 0);
	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->fd   = hton64 (remote_fd);
	req->data = hton32 (flags);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_BULK),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_FSYNC,
				    hdr, hdrlen, NULL, 0, NULL);

	return ret;
unwind:
	if (hdr)
		free (hdr);
	STACK_UNWIND(frame, -1, EINVAL);
	return 0;

}

int32_t
client_xattrop (call_frame_t *frame,
		xlator_t *this,
		loc_t *loc,
		gf_xattrop_flags_t flags,
		dict_t *dict)
{
	gf_hdr_common_t      *hdr = NULL;
	gf_fop_xattrop_req_t *req = NULL;
	size_t  hdrlen = 0;
	size_t  dict_len = 0;
	int32_t ret = -1;
	size_t  pathlen = 0;
	ino_t   ino = 0;
	client_conf_t *conf = NULL;

	GF_VALIDATE_OR_GOTO("client", this, unwind);

	conf = this->private;
	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_xattrop_cbk,
			    conf->child,
			    conf->child->fops->xattrop,
			    loc,
			    flags,
			    dict);
		
		return 0;
	}

	GF_VALIDATE_OR_GOTO(this->name, loc, unwind);

	if (dict) {
		dict_len = dict_serialized_length (dict);
		if (dict_len < 0) {
			gf_log (this->name, GF_LOG_ERROR,
				"failed to get serialized length of dict(%p)",
				dict);
			goto unwind;
		}
	}

	pathlen = STRLEN_0(loc->path);
	ino = this_ino_get (loc, this, GF_CLIENT_INODE_SELF);

	hdrlen = gf_hdr_len (req, dict_len + pathlen);
	hdr    = gf_hdr_new (req, dict_len + pathlen);
	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->flags = hton32 (flags);
	req->dict_len = hton32 (dict_len);
	if (dict) {
		ret = dict_serialize (dict, req->dict);
		if (ret < 0) {
			gf_log (this->name, GF_LOG_ERROR,
				"failed to serialize dictionary(%p)",
				dict);
			goto unwind;
		}
	}
	req->ino = hton64 (ino);
	strcpy (req->path + dict_len, loc->path);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_BULK),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_XATTROP,
				    hdr, hdrlen, NULL, 0, NULL);
	return ret;
unwind:
	if (hdr)
		free (hdr);

	STACK_UNWIND(frame, -1, EINVAL, NULL);
	return 0;
}


int32_t
client_fxattrop (call_frame_t *frame,
		 xlator_t *this,
		 fd_t *fd,
		 gf_xattrop_flags_t flags,
		 dict_t *dict)
{
	gf_hdr_common_t      *hdr = NULL;
	gf_fop_fxattrop_req_t *req = NULL;
	size_t  hdrlen = 0;
	size_t  dict_len = 0;
	int64_t remote_fd = -1;
	int32_t ret = -1;
	ino_t   ino = 0;
	client_conf_t *conf = this->private;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_fxattrop_cbk,
			    conf->child,
			    conf->child->fops->fxattrop,
			    fd,
			    flags,
			    dict);
		
		return 0;
	}

	if (dict) {
		dict_len = dict_serialized_length (dict);
		if (dict_len < 0) {
			gf_log (this->name, GF_LOG_ERROR,
				"failed to get serialized length of dict(%p)",
				dict);
			goto unwind;
		}
	}

	if (fd) {
		ret = this_fd_get (fd, this, &remote_fd);
		if (ret == -1) {
			gf_log (this->name, GF_LOG_DEBUG,
				"(%"PRId64"): failed to get remote fd. returning EBADFD", 
				fd->inode->ino);
			goto unwind;
		}
		ino = fd->inode->ino;
	}

	hdrlen = gf_hdr_len (req, dict_len);
	hdr    = gf_hdr_new (req, dict_len);
	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->flags = hton32 (flags);
	req->dict_len = hton32 (dict_len);
	if (dict) {
		ret = dict_serialize (dict, req->dict);
		if (ret < 0) {
			gf_log (this->name, GF_LOG_ERROR,
				"failed to serialize dictionary(%p)",
				dict);
			goto unwind;
		}
	}
	req->fd = hton64 (remote_fd);
	req->ino = hton64 (ino);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_BULK),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_FXATTROP,
				    hdr, hdrlen, NULL, 0, NULL);
	return ret;
unwind:
	if (hdr)
		free (hdr);

	STACK_UNWIND (frame, -1, EBADFD, NULL);
	return 0;

}


/**
 * client_setxattr - setxattr function for client protocol
 * @frame: call frame
 * @this: this translator structure
 * @loc: location
 * @dict: dictionary which contains key:value to be set.
 * @flags:
 *
 * external reference through client_protocol_xlator->fops->setxattr
 */
int32_t
client_setxattr (call_frame_t *frame,
                 xlator_t *this,
                 loc_t *loc,
                 dict_t *dict,
                 int32_t flags)
{
	gf_hdr_common_t       *hdr = NULL;
	gf_fop_setxattr_req_t *req = NULL;
	size_t hdrlen = 0;
	size_t dict_len = 0;
	int    ret = -1;
	size_t pathlen = 0;
	ino_t  ino = 0;
	client_conf_t *conf = this->private;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_setxattr_cbk,
			    conf->child,
			    conf->child->fops->setxattr,
			    loc,
			    dict,
			    flags);
		
		return 0;
	}

	dict_len = dict_serialized_length (dict);
	if (dict_len < 0) {
		gf_log (this->name, GF_LOG_ERROR,
			"failed to get serialized length of dict(%p)",
			dict);
		goto unwind;
	}

	pathlen = STRLEN_0(loc->path);
	ino = this_ino_get (loc, this, GF_CLIENT_INODE_SELF);

	hdrlen = gf_hdr_len (req, dict_len + pathlen);
	hdr    = gf_hdr_new (req, dict_len + pathlen);
	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->ino   = hton64 (ino);
	req->flags = hton32 (flags);
	req->dict_len = hton32 (dict_len);

	ret = dict_serialize (dict, req->dict);
	if (ret < 0) {
		gf_log (this->name, GF_LOG_ERROR,
			"failed to serialize dictionary(%p)",
			dict);
		goto unwind;
	}

	strcpy (req->path + dict_len, loc->path);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_BULK),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_SETXATTR,
				    hdr, hdrlen, NULL, 0, NULL);
	return ret;
unwind:
	if (hdr)
		free (hdr);

	STACK_UNWIND(frame, -1, EINVAL);
	return 0;
}

/**
 * client_getxattr - getxattr function for client protocol
 * @frame: call frame
 * @this: this translator structure
 * @loc: location structure
 *
 * external reference through client_protocol_xlator->fops->getxattr
 */
int32_t
client_getxattr (call_frame_t *frame,
                 xlator_t *this,
                 loc_t *loc,
                 const char *name)
{
	int ret = -1;
	gf_hdr_common_t *hdr = NULL;
	gf_fop_getxattr_req_t *req = NULL;
	size_t hdrlen = 0;
	size_t pathlen = 0;
	size_t namelen = 0;
	ino_t  ino = 0;
	client_conf_t *conf = this->private;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_getxattr_cbk,
			    conf->child,
			    conf->child->fops->getxattr,
			    loc,
			    name);
		
		return 0;
	}

	pathlen = STRLEN_0(loc->path);
	if (name)
		namelen = STRLEN_0(name);

	ino = this_ino_get (loc, this, GF_CLIENT_INODE_SELF);

	hdrlen = gf_hdr_len (req, pathlen + namelen);
	hdr    = gf_hdr_new (req, pathlen + namelen);
	GF_VALIDATE_OR_GOTO(frame->this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->ino   = hton64 (ino);
	req->namelen = hton32 (namelen);
	strcpy (req->path, loc->path);
	if (name)
		strcpy (req->name + pathlen, name);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_LOWLAT),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_GETXATTR,
				    hdr, hdrlen, NULL, 0, NULL);
	return ret;
unwind:
	if (hdr)
		free (hdr);

	STACK_UNWIND(frame, -1, EINVAL, NULL);
	return 0;
}

/**
 * client_removexattr - removexattr function for client protocol
 * @frame: call frame
 * @this: this translator structure
 * @loc: location structure
 * @name:
 *
 * external reference through client_protocol_xlator->fops->removexattr
 */
int32_t
client_removexattr (call_frame_t *frame,
                    xlator_t *this,
                    loc_t *loc,
                    const char *name)
{
	int ret = -1;
	gf_hdr_common_t *hdr = NULL;
	gf_fop_removexattr_req_t *req = NULL;
	size_t hdrlen = 0;
	size_t namelen = 0;
	size_t pathlen = 0;
	ino_t  ino = 0;
	client_conf_t *conf = this->private;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_removexattr_cbk,
			    conf->child,
			    conf->child->fops->removexattr,
			    loc,
			    name);
		
		return 0;
	}

	pathlen = STRLEN_0(loc->path);
	namelen = STRLEN_0(name);
	ino = this_ino_get (loc, this, GF_CLIENT_INODE_SELF);

	hdrlen = gf_hdr_len (req, pathlen + namelen);
	hdr    = gf_hdr_new (req, pathlen + namelen);
	GF_VALIDATE_OR_GOTO(frame->this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->ino   = hton64 (ino);
	strcpy (req->path, loc->path);
	strcpy (req->name + pathlen, name);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_LOWLAT),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_REMOVEXATTR,
				    hdr, hdrlen, NULL, 0, NULL);
	return ret;
unwind:
	if (hdr)
		free (hdr);
	STACK_UNWIND(frame, -1, EINVAL);
	return 0;
}


/**
 * client_opendir - opendir function for client protocol
 * @frame: call frame
 * @this: this translator structure
 * @loc: location structure
 *
 * external reference through client_protocol_xlator->fops->opendir
 */
int32_t
client_opendir (call_frame_t *frame,
                xlator_t *this,
                loc_t *loc,
                fd_t *fd)
{
	gf_fop_opendir_req_t *req = NULL;
	gf_hdr_common_t      *hdr = NULL;
	size_t hdrlen = 0;
	int    ret = -1;
	ino_t  ino = 0;
	size_t pathlen = 0;
	client_conf_t *conf = this->private;
	client_local_t *local = NULL;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_opendir_cbk,
			    conf->child,
			    conf->child->fops->opendir,
			    loc, fd);
		
		return 0;
	}

	local = calloc (1, sizeof (*local));
	GF_VALIDATE_OR_GOTO(this->name, local, unwind);

	loc_copy (&local->loc, loc);
	local->fd = fd_ref (fd);

	frame->local = local;
	
	ino = this_ino_get (loc, this, GF_CLIENT_INODE_SELF);
	pathlen = STRLEN_0(loc->path);

	hdrlen = gf_hdr_len (req, pathlen);
	hdr    = gf_hdr_new (req, pathlen);
	GF_VALIDATE_OR_GOTO(frame->this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->ino = hton64 (ino);
	strcpy (req->path, loc->path);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_LOWLAT),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_OPENDIR,
				    hdr, hdrlen, NULL, 0, NULL);

	return ret;
unwind:
	if (hdr)
		free (hdr);
	STACK_UNWIND(frame, -1, EINVAL, fd);
	return 0;

}


/**
 * client_readdir - readdir function for client protocol
 * @frame: call frame
 * @this: this translator structure
 *
 * external reference through client_protocol_xlator->fops->readdir
 */

int32_t
client_getdents (call_frame_t *frame,
                 xlator_t *this,
                 fd_t *fd,
                 size_t size,
                 off_t offset,
                 int32_t flag)
{
	gf_hdr_common_t *hdr = NULL;
	gf_fop_getdents_req_t *req = NULL;
	size_t hdrlen = 0;
	int64_t remote_fd = -1;
	int ret = -1;
	client_conf_t *conf = this->private;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_getdents_cbk,
			    conf->child,
			    conf->child->fops->getdents,
			    fd,
			    size,
			    offset,
			    flag);
		
		return 0;
	}

	ret = this_fd_get (fd, this, &remote_fd);
	if (ret == -1) {
		gf_log (this->name, GF_LOG_DEBUG,
			"(%"PRId64"): failed to get remote fd. returning EBADFD", 
			fd->inode->ino);
		STACK_UNWIND (frame, -1, EBADFD, NULL);
		return 0;
	}

	hdrlen = gf_hdr_len (req, 0);
	hdr    = gf_hdr_new (req, 0);
	GF_VALIDATE_OR_GOTO(frame->this->name, hdr, unwind);

	req    = gf_param (hdr);
	GF_VALIDATE_OR_GOTO(frame->this->name, hdr, unwind);

	req->fd     = hton64 (remote_fd);
	req->size   = hton32 (size);
	req->offset = hton64 (offset);
	req->flags  = hton32 (flag);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_BULK),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_GETDENTS,
				    hdr, hdrlen, NULL, 0, NULL);

	return 0;
unwind:
	STACK_UNWIND(frame, -1, EINVAL, NULL, 0);
	return 0;
}

/**
 * client_readdir - readdir function for client protocol
 * @frame: call frame
 * @this: this translator structure
 *
 * external reference through client_protocol_xlator->fops->readdir
 */

int32_t
client_readdir (call_frame_t *frame,
		xlator_t *this,
		fd_t *fd,
		size_t size,
		off_t offset)
{
	gf_hdr_common_t *hdr = NULL;
	gf_fop_readdir_req_t *req = NULL;
	size_t hdrlen = 0;
	int64_t remote_fd = -1;
	int ret = -1;
	client_conf_t *conf = this->private;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_readdir_cbk,
			    conf->child,
			    conf->child->fops->readdir,
			    fd, size, offset);
		
		return 0;
	}

	ret = this_fd_get (fd, this, &remote_fd);
	if (ret == -1) {
		gf_log (this->name, GF_LOG_DEBUG,
			"(%"PRId64"): failed to get remote fd. returning EBADFD", 
			fd->inode->ino);
		goto unwind;
	}

	hdrlen = gf_hdr_len (req, 0);
	hdr    = gf_hdr_new (req, 0);
	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req    = gf_param (hdr);
	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req->fd     = hton64 (remote_fd);
	req->size   = hton32 (size);
	req->offset = hton64 (offset);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_LOWLAT),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_READDIR,
				    hdr, hdrlen, NULL, 0, NULL);

	return 0;
unwind:
	if (hdr)
		free (hdr);
	STACK_UNWIND (frame, -1, EBADFD, NULL);
	return 0;

}



/**
 * client_fsyncdir - fsyncdir function for client protocol
 * @frame: call frame
 * @this: this translator structure
 * @fd: file descriptor structure
 * @flags:
 *
 * external reference through client_protocol_xlator->fops->fsyncdir
 */

int32_t
client_fsyncdir (call_frame_t *frame,
                 xlator_t *this,
                 fd_t *fd,
                 int32_t flags)
{
	gf_hdr_common_t *hdr = NULL;
	gf_fop_fsyncdir_req_t *req = NULL;
	size_t hdrlen = 0;
	int64_t remote_fd = -1;
	int32_t ret = -1;
	client_conf_t *conf = this->private;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_fsyncdir_cbk,
			    conf->child,
			    conf->child->fops->fsyncdir,
			    fd,
			    flags);
		
		return 0;
	}

	ret = this_fd_get (fd, this, &remote_fd);
	if (ret == -1) {
		gf_log (this->name, GF_LOG_DEBUG,
			"(%"PRId64"): failed to get remote fd. returning EBADFD", 
			fd->inode->ino);
		goto unwind;
	}

	hdrlen = gf_hdr_len (req, 0);
	hdr    = gf_hdr_new (req, 0);
	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->data = hton32 (flags);
	req->fd   = hton64 (remote_fd);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_BULK),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_FSYNCDIR,
				    hdr, hdrlen, NULL, 0, NULL);

	return ret;
unwind:
	STACK_UNWIND (frame, -1, EBADFD);
	return 0;
}


/**
 * client_access - access function for client protocol
 * @frame: call frame
 * @this: this translator structure
 * @loc: location structure
 * @mode:
 *
 * external reference through client_protocol_xlator->fops->access
 */
int32_t
client_access (call_frame_t *frame,
               xlator_t *this,
               loc_t *loc,
               int32_t mask)
{
	gf_hdr_common_t     *hdr = NULL;
	gf_fop_access_req_t *req = NULL;
	size_t hdrlen = -1;
	int    ret = -1;
	ino_t  ino = 0;
	size_t pathlen = 0;
	client_conf_t *conf = this->private;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_access_cbk,
			    conf->child,
			    conf->child->fops->access,
			    loc,
			    mask);
		
		return 0;
	}

	ino = this_ino_get (loc, this, GF_CLIENT_INODE_SELF);
	pathlen = STRLEN_0(loc->path);

	hdrlen = gf_hdr_len (req, pathlen);
	hdr    = gf_hdr_new (req, pathlen);
	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->ino  = hton64 (ino);
	req->mask = hton32 (mask);
	strcpy (req->path, loc->path);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_LOWLAT),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_ACCESS,
				    hdr, hdrlen, NULL, 0, NULL);

	return ret;
unwind:
	if (hdr)
		free (hdr);

	STACK_UNWIND(frame, -1, EINVAL);
	return 0;

}


/**
 * client_ftrucate - ftruncate function for client protocol
 * @frame: call frame
 * @this: this translator structure
 * @fd: file descriptor structure
 * @offset: offset to truncate to
 *
 * external reference through client_protocol_xlator->fops->ftruncate
 */

int32_t
client_ftruncate (call_frame_t *frame,
                  xlator_t *this,
                  fd_t *fd,
                  off_t offset)
{
	gf_hdr_common_t *hdr = NULL;
	gf_fop_ftruncate_req_t *req = NULL;
	int64_t remote_fd = -1;
	size_t hdrlen = -1;
	int ret = -1;
	client_conf_t *conf = this->private;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_ftruncate_cbk,
			    conf->child,
			    conf->child->fops->ftruncate,
			    fd,
			    offset);
		
		return 0;
	}

	ret = this_fd_get (fd, this, &remote_fd);
	if (ret == -1) {
		gf_log (this->name, GF_LOG_DEBUG,
			"(%"PRId64"): failed to get remote fd. returning EBADFD", 
			fd->inode->ino);
		STACK_UNWIND (frame, -1, EBADFD, NULL);
		return 0;
	}

	hdrlen = gf_hdr_len (req, 0);
	hdr    = gf_hdr_new (req, 0);
	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->fd     = hton64 (remote_fd);
	req->offset = hton64 (offset);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_BULK),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_FTRUNCATE,
				    hdr, hdrlen, NULL, 0, NULL);

	return ret;
unwind:
	if (hdr)
		free (hdr);

	STACK_UNWIND(frame, -1, EINVAL, NULL);
	return 0;

}


/**
 * client_fstat - fstat function for client protocol
 * @frame: call frame
 * @this: this translator structure
 * @fd: file descriptor structure
 *
 * external reference through client_protocol_xlator->fops->fstat
 */

int32_t
client_fstat (call_frame_t *frame,
              xlator_t *this,
              fd_t *fd)
{
	gf_hdr_common_t *hdr = NULL;
	gf_fop_fstat_req_t *req = NULL;
	int64_t remote_fd = -1;
	size_t hdrlen = -1;
	int ret = -1;
	client_conf_t *conf = this->private;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_fstat_cbk,
			    conf->child,
			    conf->child->fops->fstat,
			    fd);
		
		return 0;
	}

	ret = this_fd_get (fd, this, &remote_fd);
	if (ret == -1) {
		gf_log (this->name, GF_LOG_DEBUG,
			"(%"PRId64"): failed to get remote fd. returning EBADFD",
			fd->inode->ino);
		STACK_UNWIND (frame, -1, EBADFD, NULL);
		return 0;
	}

	hdrlen = gf_hdr_len (req, 0);
	hdr    = gf_hdr_new (req, 0);
	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->fd = hton64 (remote_fd);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_BULK),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_FSTAT,
				    hdr, hdrlen, NULL, 0, NULL);

	return ret;
unwind:
	if (hdr)
		free (hdr);

	STACK_UNWIND(frame, -1, EINVAL, NULL);
	return 0;

}


/**
 * client_lk - lk function for client protocol
 * @frame: call frame
 * @this: this translator structure
 * @fd: file descriptor structure
 * @cmd: lock command
 * @lock:
 *
 * external reference through client_protocol_xlator->fops->lk
 */
int32_t
client_lk (call_frame_t *frame,
           xlator_t *this,
           fd_t *fd,
           int32_t cmd,
           struct flock *flock)
{
	int ret = -1;
	gf_hdr_common_t *hdr = NULL;
	gf_fop_lk_req_t *req = NULL;
	size_t hdrlen = 0;
	int64_t remote_fd = -1;
	int32_t gf_cmd = 0;
	int32_t gf_type = 0;
	client_conf_t *conf = this->private;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_lk_cbk,
			    conf->child,
			    conf->child->fops->lk,
			    fd,
			    cmd,
			    flock);
		
		return 0;
	}

	ret = this_fd_get (fd, this, &remote_fd);
	if (ret == -1) {
		gf_log (this->name, GF_LOG_DEBUG,
			"(%"PRId64"): failed to get remote fd. returning EBADFD",
			fd->inode->ino);
		STACK_UNWIND(frame, -1, EBADFD, NULL);
		return 0;
	}

	if (cmd == F_GETLK || cmd == F_GETLK64)
		gf_cmd = GF_LK_GETLK;
	else if (cmd == F_SETLK || cmd == F_SETLK64)
		gf_cmd = GF_LK_SETLK;
	else if (cmd == F_SETLKW || cmd == F_SETLKW64)
		gf_cmd = GF_LK_SETLKW;
	else {
		gf_log (this->name, GF_LOG_ERROR,
			"Unknown cmd (%d)!", gf_cmd);
		goto unwind;
	}

	switch (flock->l_type) {
	case F_RDLCK:
		gf_type = GF_LK_F_RDLCK;
		break;
	case F_WRLCK:
		gf_type = GF_LK_F_WRLCK;
		break;
	case F_UNLCK:
		gf_type = GF_LK_F_UNLCK;
		break;
	}

	hdrlen = gf_hdr_len (req, 0);
	hdr    = gf_hdr_new (req, 0);
	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->fd   = hton64 (remote_fd);
	req->cmd  = hton32 (gf_cmd);
	req->type = hton32 (gf_type);
	gf_flock_from_flock (&req->flock, flock);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_BULK),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_LK,
				    hdr, hdrlen, NULL, 0, NULL);
	return ret;
unwind:
	if (hdr)
		free (hdr);

	STACK_UNWIND(frame, -1, EINVAL, NULL);
	return 0;
}


/**
 * client_inodelk - inodelk function for client protocol
 * @frame: call frame
 * @this: this translator structure
 * @inode: inode structure
 * @cmd: lock command
 * @lock: flock struct
 *
 * external reference through client_protocol_xlator->fops->inodelk
 */
int32_t
client_inodelk (call_frame_t *frame,
		xlator_t *this,
		loc_t *loc,
		int32_t cmd,
		struct flock *flock)
{
	int ret = -1;
	gf_hdr_common_t *hdr = NULL;
	gf_fop_inodelk_req_t *req = NULL;
	size_t hdrlen = 0;
	int32_t gf_cmd = 0;
	int32_t gf_type = 0;
	ino_t   ino  = 0;
	size_t  pathlen = 0;
	client_conf_t *conf = this->private;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_inodelk_cbk,
			    conf->child,
			    conf->child->fops->inodelk,
			    loc, cmd, flock);
		
		return 0;
	}

	pathlen = STRLEN_0(loc->path);
	ino = this_ino_get (loc, this, GF_CLIENT_INODE_SELF);

	if (cmd == F_GETLK || cmd == F_GETLK64)
		gf_cmd = GF_LK_GETLK;
	else if (cmd == F_SETLK || cmd == F_SETLK64)
		gf_cmd = GF_LK_SETLK;
	else if (cmd == F_SETLKW || cmd == F_SETLKW64)
		gf_cmd = GF_LK_SETLKW;
	else {
		gf_log (this->name, GF_LOG_ERROR,
			"Unknown cmd (%d)!", gf_cmd);
		goto unwind;
	}

	switch (flock->l_type) {
	case F_RDLCK:
		gf_type = GF_LK_F_RDLCK;
		break;
	case F_WRLCK:
		gf_type = GF_LK_F_WRLCK;
		break;
	case F_UNLCK:
		gf_type = GF_LK_F_UNLCK;
		break;
	}

	hdrlen = gf_hdr_len (req, pathlen);
	hdr    = gf_hdr_new (req, pathlen);
	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req    = gf_param (hdr);

	strcpy (req->path, loc->path);

	req->ino  = hton64 (ino);

	req->cmd  = hton32 (gf_cmd);
	req->type = hton32 (gf_type);
	gf_flock_from_flock (&req->flock, flock);


	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_BULK),
				    GF_OP_TYPE_FOP_REQUEST,
				    GF_FOP_INODELK,
				    hdr, hdrlen, NULL, 0, NULL);
	return ret;
unwind:
	if (hdr)
		free (hdr);

	STACK_UNWIND(frame, -1, EINVAL);
	return 0;

}


/**
 * client_finodelk - finodelk function for client protocol
 * @frame: call frame
 * @this: this translator structure
 * @inode: inode structure
 * @cmd: lock command
 * @lock: flock struct
 *
 * external reference through client_protocol_xlator->fops->finodelk
 */
int32_t
client_finodelk (call_frame_t *frame,
		 xlator_t *this,
		 fd_t *fd,
		 int32_t cmd,
		 struct flock *flock)
{
	int ret = -1;
	gf_hdr_common_t *hdr = NULL;
	gf_fop_finodelk_req_t *req = NULL;
	size_t hdrlen = 0;
	int32_t gf_cmd = 0;
	int32_t gf_type = 0;
	int64_t remote_fd = -1;
	client_conf_t *conf = this->private;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_finodelk_cbk,
			    conf->child,
			    conf->child->fops->finodelk,
			    fd, cmd, flock);
		
		return 0;
	}

	ret = this_fd_get (fd, this, &remote_fd);
	if (ret == -1) {
		gf_log (this->name, GF_LOG_DEBUG,
			"(%"PRId64"): failed to get remote fd. returning EBADFD", 
			fd->inode->ino);
		STACK_UNWIND(frame, -1, EBADFD);
		return 0;
	}

	if (cmd == F_GETLK || cmd == F_GETLK64)
		gf_cmd = GF_LK_GETLK;
	else if (cmd == F_SETLK || cmd == F_SETLK64)
		gf_cmd = GF_LK_SETLK;
	else if (cmd == F_SETLKW || cmd == F_SETLKW64)
		gf_cmd = GF_LK_SETLKW;
	else {
		gf_log (this->name, GF_LOG_ERROR,
			"Unknown cmd (%d)!", gf_cmd);
		goto unwind;
	}

	switch (flock->l_type) {
	case F_RDLCK:
		gf_type = GF_LK_F_RDLCK;
		break;
	case F_WRLCK:
		gf_type = GF_LK_F_WRLCK;
		break;
	case F_UNLCK:
		gf_type = GF_LK_F_UNLCK;
		break;
	}

	hdrlen = gf_hdr_len (req, 0);
	hdr    = gf_hdr_new (req, 0);
	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->fd = hton64 (remote_fd);

	req->cmd  = hton32 (gf_cmd);
	req->type = hton32 (gf_type);
	gf_flock_from_flock (&req->flock, flock);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_BULK),
				    GF_OP_TYPE_FOP_REQUEST,
				    GF_FOP_FINODELK,
				    hdr, hdrlen, NULL, 0, NULL);
	return ret;
unwind:
	if (hdr)
		free (hdr);

	STACK_UNWIND(frame, -1, EINVAL);
	return 0;
}


int32_t
client_entrylk (call_frame_t *frame,
		xlator_t *this,
		loc_t *loc,
		const char *name,
		entrylk_cmd cmd,
		entrylk_type type)
{
	gf_hdr_common_t *hdr = NULL;
	gf_fop_entrylk_req_t *req = NULL;
	size_t pathlen = 0;
	size_t hdrlen = -1;
	int ret = -1;
	ino_t ino = 0;
	size_t namelen = 0;
	client_conf_t *conf = this->private;

	if (conf->child) {
		/* */
		STACK_WIND (frame, default_entrylk_cbk,
			    conf->child,
			    conf->child->fops->entrylk,
			    loc, name, cmd, type);
		
		return 0;
	}

	pathlen = STRLEN_0(loc->path);
	if (name)
		namelen = STRLEN_0(name);

	ino = this_ino_get (loc, this, GF_CLIENT_INODE_SELF);

	hdrlen = gf_hdr_len (req, pathlen + namelen);
	hdr    = gf_hdr_new (req, pathlen + namelen);
	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->ino  = hton64 (ino);
	req->namelen = hton64 (namelen);

	strcpy (req->path, loc->path);
	if (name)
		strcpy (req->name + pathlen, name);

	req->cmd  = hton32 (cmd);
	req->type = hton32 (type);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_LOWLAT),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_ENTRYLK,
				    hdr, hdrlen, NULL, 0, NULL);

	return ret;
unwind:
	if (hdr)
		free (hdr);

	STACK_UNWIND(frame, -1, EINVAL);
	return 0;

}


int32_t
client_fentrylk (call_frame_t *frame,
		 xlator_t *this,
		 fd_t *fd,
		 const char *name,
		 entrylk_cmd cmd,
		 entrylk_type type)
{
	gf_hdr_common_t *hdr = NULL;
	gf_fop_fentrylk_req_t *req = NULL;
	int64_t remote_fd = -1;
	size_t namelen = 0;
	size_t hdrlen = -1;
	int ret = -1;
	client_conf_t *conf = this->private;

	if (conf->child) {
		/* */
		STACK_WIND (frame, default_fentrylk_cbk,
			    conf->child,
			    conf->child->fops->fentrylk,
			    fd, name, cmd, type);
		
		return 0;
	}

	if (name)
		namelen = STRLEN_0(name);

	ret = this_fd_get (fd, this, &remote_fd);
	if (ret == -1) {
		gf_log (this->name, GF_LOG_DEBUG,
			"(%"PRId64"): failed to get remote fd. returning EBADFD", 
			fd->inode->ino);
		STACK_UNWIND(frame, -1, EBADFD);
		return 0;
	}

	hdrlen = gf_hdr_len (req, namelen);
	hdr    = gf_hdr_new (req, namelen);
	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->fd = hton64 (remote_fd);
	req->namelen = hton64 (namelen);

	if (name)
		strcpy (req->name, name);

	req->cmd  = hton32 (cmd);
	req->type = hton32 (type);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_LOWLAT),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_FENTRYLK,
				    hdr, hdrlen, NULL, 0, NULL);

	return ret;
unwind:
	if (hdr)
		free (hdr);

	STACK_UNWIND(frame, -1, EINVAL);
	return 0;
}


/*
 * client_lookup - lookup function for client protocol
 * @frame: call frame
 * @this:
 * @loc: location
 *
 * not for external reference
 */
int32_t
client_lookup (call_frame_t *frame,
               xlator_t *this,
               loc_t *loc,
               dict_t *xattr_req)
{
	gf_hdr_common_t     *hdr = NULL;
	gf_fop_lookup_req_t *req = NULL;
	size_t hdrlen = -1;
	int    ret = -1;
	ino_t  ino = 0;
	ino_t  par = 0;
	size_t dictlen = 0;
	size_t pathlen = 0;
	size_t baselen = 0;
	int32_t op_ret = -1;
	int32_t op_errno = EINVAL;
	client_conf_t *conf = this->private;
	client_local_t *local = NULL;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_lookup_cbk,
			    conf->child,
			    conf->child->fops->lookup,
			    loc,
			    xattr_req);
		
		return 0;
	}

	local = calloc (1, sizeof (*local));
	GF_VALIDATE_OR_GOTO(this->name, local, unwind);

	loc_copy (&local->loc, loc);

	frame->local = local;

	GF_VALIDATE_OR_GOTO (this->name, loc, unwind);
	GF_VALIDATE_OR_GOTO (this->name, loc->path, unwind);

	if (loc->ino != 1) {
		par = this_ino_get (loc, this, GF_CLIENT_INODE_PARENT);
		GF_VALIDATE_OR_GOTO (this->name, loc->name, unwind);
		baselen = STRLEN_0(loc->name);
	} else {
		ino = 1;
	}

	pathlen = STRLEN_0(loc->path);
	
	if (xattr_req) {
		dictlen = dict_serialized_length (xattr_req);
		if (dictlen < 0) {
			gf_log (this->name, GF_LOG_ERROR,
				"failed to get serialized length of dict(%p)",
				xattr_req);
			ret = dictlen;
			goto unwind;
		}
	}

	hdrlen = gf_hdr_len (req, pathlen + baselen + dictlen);
	hdr    = gf_hdr_new (req, pathlen + baselen + dictlen);
	GF_VALIDATE_OR_GOTO (this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->ino   = hton64 (ino);
	req->par   = hton64 (par);
	strcpy (req->path, loc->path);
	if (baselen)
		strcpy (req->path + pathlen, loc->name);
	
	if (dictlen) {
		ret = dict_serialize (xattr_req, req->dict + baselen + pathlen);
		if (ret < 0) {
			gf_log (this->name, GF_LOG_ERROR,
				"failed to serialize dictionary(%p)",
				xattr_req);
			goto unwind;
		}
	}

	req->dictlen = hton32 (dictlen);
	
	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_LOWLAT),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_LOOKUP,
				    hdr, hdrlen, NULL, 0, NULL);
	return ret;

unwind:
	STACK_UNWIND (frame, op_ret, op_errno, loc->inode, NULL, NULL);
	return ret;
}



/*
 * client_fchmod
 *
 */
int32_t
client_fchmod (call_frame_t *frame,
               xlator_t *this,
               fd_t *fd,
               mode_t mode)
{
	gf_hdr_common_t *hdr = NULL;
	gf_fop_fchmod_req_t *req = NULL;
	int64_t remote_fd = -1;
	size_t hdrlen = -1;
	int ret = -1;
	int32_t op_errno = EINVAL;
	int32_t op_ret   = -1;
	client_conf_t *conf = this->private;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_fchmod_cbk,
			    conf->child,
			    conf->child->fops->fchmod,
			    fd,
			    mode);
		
		return 0;
	}

	GF_VALIDATE_OR_GOTO (this->name, fd, unwind);

	ret = this_fd_get (fd, this, &remote_fd);
	if (ret == -1) {
		op_errno = EBADFD;
		gf_log (this->name, GF_LOG_DEBUG,
			"(%"PRId64"): failed to get remote fd. returning EBADFD", 
			fd->inode->ino);
		goto unwind;
	}

	hdrlen = gf_hdr_len (req, 0);
	hdr    = gf_hdr_new (req, 0);
	GF_VALIDATE_OR_GOTO (this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->fd   = hton64 (remote_fd);
	req->mode = hton32 (mode);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_LOWLAT),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_FCHMOD,
				    hdr, hdrlen, NULL, 0, NULL);

	return 0;

unwind:
	STACK_UNWIND (frame, op_ret, op_errno, NULL);
	return 0;
}


/*
 * client_fchown -
 *
 * @frame:
 * @this:
 * @fd:
 * @uid:
 * @gid:
 *
 */
int32_t
client_fchown (call_frame_t *frame,
               xlator_t *this,
               fd_t *fd,
               uid_t uid,
               gid_t gid)
{
	gf_hdr_common_t     *hdr = NULL;
	gf_fop_fchown_req_t *req = NULL;
	int64_t remote_fd = 0;
	size_t  hdrlen   = -1;
	int32_t op_ret   = -1;
	int32_t op_errno = EINVAL;
	int32_t ret      = -1;
	client_conf_t *conf = this->private;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_fchown_cbk,
			    conf->child,
			    conf->child->fops->fchown,
			    fd,
			    uid,
			    gid);
		
		return 0;
	}

	GF_VALIDATE_OR_GOTO (this->name, fd, unwind);

	ret = this_fd_get (fd, this, &remote_fd);
	if (ret == -1) {
		op_errno = EBADFD;
		gf_log (this->name, GF_LOG_DEBUG,
			"(%"PRId64"): failed to get remote fd. returning EBADFD", 
			fd->inode->ino);
		goto unwind;
	}

	hdrlen = gf_hdr_len (req, 0);
	hdr    = gf_hdr_new (req, 0);
	GF_VALIDATE_OR_GOTO (this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->fd  = hton64 (remote_fd);
	req->uid = hton32 (uid);
	req->gid = hton32 (gid);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_LOWLAT),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_FCHOWN,
				    hdr, hdrlen, NULL, 0, NULL);

	return 0;

unwind:
	STACK_UNWIND (frame, op_ret, op_errno, NULL);
	return 0;

}

/**
 * client_setdents -
 */
int32_t
client_setdents (call_frame_t *frame,
                 xlator_t *this,
                 fd_t *fd,
                 int32_t flags,
                 dir_entry_t *entries,
                 int32_t count)
{
	gf_hdr_common_t       *hdr = NULL;
	gf_fop_setdents_req_t *req = NULL;
	int64_t remote_fd = 0;
	char   *buffer = NULL;
	char *ptr = NULL;
	data_t *buf_data = NULL;
	dict_t *reply_dict = NULL;
	dir_entry_t *trav = NULL;
	uint32_t len = 0;
	int32_t  buf_len = 0;
	int32_t  ret = -1;
	int32_t  op_ret = -1;
	int32_t  op_errno = EINVAL;
	int32_t  vec_count = 0;
	size_t   hdrlen = -1;
	struct iovec vector[1];
	client_conf_t *conf = this->private;

	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_setdents_cbk,
			    conf->child,
			    conf->child->fops->setdents,
			    fd,
			    flags,
			    entries,
			    count);
		
		return 0;
	}

	GF_VALIDATE_OR_GOTO (this->name, fd, unwind);

	ret = this_fd_get (fd, this, &remote_fd);
	if (ret == -1) {
		gf_log (this->name, GF_LOG_DEBUG,
			"(%"PRId64"): failed to get remote fd. returning EBADFD", 
			fd->inode->ino);
		op_errno = EBADFD;
		goto unwind;
	}

	GF_VALIDATE_OR_GOTO (this->name, entries, unwind);
	GF_VALIDATE_OR_GOTO (this->name, count, unwind);

	trav = entries->next;
	while (trav) {
		len += strlen (trav->name);
		len += 1;
		len += strlen (trav->link);
		len += 1;
		len += 256; // max possible for statbuf;
		trav = trav->next;
	}
	buffer = CALLOC (1, len);
	GF_VALIDATE_OR_GOTO (this->name, buffer, unwind);

	ptr = buffer;

	trav = entries->next;
	while (trav) {
		int32_t this_len = 0;
		char *tmp_buf = NULL;
		struct stat *stbuf = &trav->buf;
		{
			/* Convert the stat buf to string */
			uint64_t dev = stbuf->st_dev;
			uint64_t ino = stbuf->st_ino;
			uint32_t mode = stbuf->st_mode;
			uint32_t nlink = stbuf->st_nlink;
			uint32_t uid = stbuf->st_uid;
			uint32_t gid = stbuf->st_gid;
			uint64_t rdev = stbuf->st_rdev;
			uint64_t size = stbuf->st_size;
			uint32_t blksize = stbuf->st_blksize;
			uint64_t blocks = stbuf->st_blocks;

			uint32_t atime = stbuf->st_atime;
			uint32_t mtime = stbuf->st_mtime;
			uint32_t ctime = stbuf->st_ctime;

			uint32_t atime_nsec = ST_ATIM_NSEC(stbuf);
			uint32_t mtime_nsec = ST_MTIM_NSEC(stbuf);
			uint32_t ctime_nsec = ST_CTIM_NSEC(stbuf);

			asprintf (&tmp_buf,
				  GF_STAT_PRINT_FMT_STR,
				  dev,
				  ino,
				  mode,
				  nlink,
				  uid,
				  gid,
				  rdev,
				  size,
				  blksize,
				  blocks,
				  atime,
				  atime_nsec,
				  mtime,
				  mtime_nsec,
				  ctime,
				  ctime_nsec);
		}
		this_len = sprintf (ptr, "%s/%s%s\n",
				    trav->name,
				    tmp_buf,
				    trav->link);

		FREE (tmp_buf);
		trav = trav->next;
		ptr += this_len;
	}
	buf_len = strlen (buffer);

	hdrlen = gf_hdr_len (req, 0);
	hdr    = gf_hdr_new (req, 0);
	GF_VALIDATE_OR_GOTO (this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->fd    = hton64 (remote_fd);
	req->flags = hton32 (flags);
	req->count = hton32 (count);

	{
		buf_data = get_new_data ();
		GF_VALIDATE_OR_GOTO (this->name, buf_data, unwind);
		reply_dict = get_new_dict();
		GF_VALIDATE_OR_GOTO (this->name, reply_dict, unwind);

		buf_data->data = buffer;
		buf_data->len = buf_len;
		dict_set (reply_dict, NULL, buf_data);
		frame->root->rsp_refs = dict_ref (reply_dict);
		vector[0].iov_base = buffer;
		vector[0].iov_len = buf_len;
		vec_count = 1;
	}

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_BULK),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_SETDENTS,
				    hdr, hdrlen, vector, vec_count, 
				    frame->root->rsp_refs);

	return ret;
unwind:
	STACK_UNWIND (frame, op_ret, op_errno);
	return 0;
}


/*
 * CBKs
 */
/*
 * client_forget - forget function for client protocol
 * @this:
 * @inode:
 *
 * not for external reference
 */
int32_t
client_forget (xlator_t *this,
               inode_t *inode)
{
	ino_t            ino = 0;
	client_conf_t *conf = NULL;
	client_forget_t  forget = {0,};
	uint8_t          send_forget = 0;
	int32_t          ret = -1;

	GF_VALIDATE_OR_GOTO ("client", this, out);
	conf = this->private;

	if (conf->child) {
		/* */
		/* Yenu beda */
		return 0;
	}

	GF_VALIDATE_OR_GOTO (this->name, inode, out);
	ino = this_ino_get_from_inode (inode, this);

	LOCK (&conf->forget.lock);
	{
		conf->forget.ino_array[conf->forget.count++] = ino;

		if ((!conf->forget.frames_in_transit) || 
		    (conf->forget.count >= CLIENT_PROTO_FORGET_LIMIT)) {
			ret = client_get_forgets (this, &forget);
			if (ret <= 0)
				send_forget = 0;
			else
				send_forget = 1;
		}
	}
	UNLOCK (&conf->forget.lock);
	
	if (send_forget) {
		ret = protocol_client_xfer (forget.frame, this,
					    CLIENT_CHANNEL (this,CHANNEL_BULK),
					    GF_OP_TYPE_CBK_REQUEST, 
					    GF_CBK_FORGET,
					    forget.hdr, forget.hdrlen, 
					    NULL, 0, NULL);
	}
out:
	return 0;
}

/**
 * client_releasedir - releasedir function for client protocol
 * @this: this translator structure
 * @fd: file descriptor structure
 *
 * external reference through client_protocol_xlator->cbks->releasedir
 */

int32_t
client_releasedir (xlator_t *this, fd_t *fd)
{
	call_frame_t            *fr = NULL;
	int32_t                  ret = -1;
	int64_t                  remote_fd = 0;
	char                     key[32] = {0,};
	gf_hdr_common_t         *hdr = NULL;
	size_t                   hdrlen = 0;
	gf_cbk_releasedir_req_t *req  = NULL;
	client_conf_t           *conf = NULL;


	GF_VALIDATE_OR_GOTO ("client", this, out);
	GF_VALIDATE_OR_GOTO (this->name, fd, out);

	conf = this->private;
	if (conf->child) {
		return 0;
	}

	ret = this_fd_get (fd, this, &remote_fd);
	if (ret == -1){
		gf_log (this->name, GF_LOG_DEBUG,
			"(%"PRId64"): failed to get remote fd.", 
			fd->inode->ino);
		goto out;
	}

	hdrlen = gf_hdr_len (req, 0);
	hdr    = gf_hdr_new (req, 0);
	GF_VALIDATE_OR_GOTO (this->name, hdr, out);

	req    = gf_param (hdr);

	req->fd = hton64 (remote_fd);

	{
		sprintf (key, "%p", fd);

		pthread_mutex_lock (&conf->mutex);
		{
			dict_del (conf->saved_fds, key);
		}
		pthread_mutex_unlock (&conf->mutex);
	}

	fr = create_frame (this, this->ctx->pool);
	GF_VALIDATE_OR_GOTO (this->name, fr, out);

	ret = protocol_client_xfer (fr, this,
				    CLIENT_CHANNEL (this, CHANNEL_BULK),
				    GF_OP_TYPE_CBK_REQUEST, GF_CBK_RELEASEDIR,
				    hdr, hdrlen, NULL, 0, NULL);
out:
	return ret;
}


/**
 * client_release - release function for client protocol
 * @this: this translator structure
 * @fd: file descriptor structure
 *
 * external reference through client_protocol_xlator->cbks->release
 *
 */
int
client_release (xlator_t *this, fd_t *fd)
{
	call_frame_t          *fr = NULL;
	int32_t                ret = -1;
	int64_t                remote_fd = 0;
	char                   key[32] = {0,};
	gf_hdr_common_t       *hdr = NULL;
	size_t                 hdrlen = 0;
	gf_cbk_release_req_t  *req = NULL;
	client_conf_t         *conf = NULL;

	GF_VALIDATE_OR_GOTO ("client", this, out);
	GF_VALIDATE_OR_GOTO (this->name, fd, out);

	conf = this->private;

	if (conf->child) {
		return 0;
	}

	ret = this_fd_get (fd, this, &remote_fd);
	if (ret == -1) {
		gf_log (this->name, GF_LOG_DEBUG,
			"(%"PRId64"): failed to get remote fd.", 
			fd->inode->ino);
		goto out;
	}

	hdrlen = gf_hdr_len (req, 0);
	hdr    = gf_hdr_new (req, 0);
	GF_VALIDATE_OR_GOTO (this->name, hdr, out);
	req    = gf_param (hdr);

	req->fd = hton64 (remote_fd);

	{
		sprintf (key, "%p", fd);

		pthread_mutex_lock (&conf->mutex);
		{
			dict_del (conf->saved_fds, key);
		}
		pthread_mutex_unlock (&conf->mutex);
	}

	fr = create_frame (this, this->ctx->pool);
	GF_VALIDATE_OR_GOTO (this->name, fr, out);

	ret = protocol_client_xfer (fr, this,
				    CLIENT_CHANNEL (this, CHANNEL_BULK),
				    GF_OP_TYPE_CBK_REQUEST, GF_CBK_RELEASE,
				    hdr, hdrlen, NULL, 0, NULL);
out:
	return ret;
}

/*
 * MGMT_OPS
 */

/**
 * client_stats - stats function for client protocol
 * @frame: call frame
 * @this: this translator structure
 * @flags:
 *
 * external reference through client_protocol_xlator->mops->stats
 */

int32_t
client_stats (call_frame_t *frame,
              xlator_t *this,
              int32_t flags)
{
	gf_hdr_common_t *hdr = NULL;
	gf_mop_stats_req_t *req = NULL;
	size_t hdrlen = -1;
	int ret = -1;
	client_conf_t *conf = NULL;

	GF_VALIDATE_OR_GOTO ("client", this, unwind);

	conf = this->private;
	if (conf->child) {
		/* */
		STACK_WIND (frame,
			    default_stats_cbk,
			    conf->child,
			    conf->child->mops->stats,
			    flags);
		
		return 0;
	}


	hdrlen = gf_hdr_len (req, 0);
	hdr    = gf_hdr_new (req, 0);
	GF_VALIDATE_OR_GOTO (this->name, hdr, unwind);

	req    = gf_param (hdr);

	req->flags = hton32 (flags);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_BULK),
				    GF_OP_TYPE_MOP_REQUEST, GF_MOP_STATS,
				    hdr, hdrlen, NULL, 0, NULL);

	return ret;
unwind:
	STACK_UNWIND (frame, -1, EINVAL, NULL);
	return 0;
}


/* Callbacks */

int32_t
client_fxattrop_cbk (call_frame_t *frame,
		     gf_hdr_common_t *hdr, size_t hdrlen,
		     char *buf, size_t buflen)
{
	gf_fop_xattrop_rsp_t *rsp = NULL;
	int32_t op_ret   = 0;
	int32_t gf_errno = 0;
	int32_t op_errno = 0;
	int32_t dict_len = 0;
	dict_t *dict = NULL;
	int32_t ret = -1;
	char *dictbuf = NULL;

	rsp = gf_param (hdr);
	GF_VALIDATE_OR_GOTO(frame->this->name, rsp, fail);

	op_ret   = ntoh32 (hdr->rsp.op_ret);

	if (op_ret >= 0) {
		op_ret = -1;
		dict_len = ntoh32 (rsp->dict_len);

		if (dict_len > 0) {
			dictbuf = memdup (rsp->dict, dict_len);
			GF_VALIDATE_OR_GOTO(frame->this->name, dictbuf, fail);

			dict = dict_new();
			GF_VALIDATE_OR_GOTO(frame->this->name, dict, fail);

			ret = dict_unserialize (dictbuf, dict_len, &dict);
			if (ret < 0) {
				gf_log (frame->this->name, GF_LOG_ERROR,
					"failed to serialize dictionary(%p)",
					dict);
				op_errno = -ret;
				goto fail;
			} else {
				dict->extra_free = dictbuf;
				dictbuf = NULL;
			}
		}
		op_ret = 0;
	}
	gf_errno = ntoh32 (hdr->rsp.op_errno);
	op_errno = gf_error_to_errno (gf_errno);

fail:
	STACK_UNWIND (frame, op_ret, op_errno, dict);
	
	if (dictbuf)
		free (dictbuf);

	if (dict)
		dict_unref (dict);

	return 0;
}

int32_t
client_xattrop_cbk (call_frame_t *frame,
		    gf_hdr_common_t *hdr, size_t hdrlen,
		    char *buf, size_t buflen)
{
	gf_fop_xattrop_rsp_t *rsp = NULL;
	int32_t op_ret   = -1;
	int32_t gf_errno = EINVAL;
	int32_t op_errno = 0;
	int32_t dict_len = 0;
	dict_t *dict = NULL;
	int32_t ret = -1;
	char *dictbuf = NULL;

	rsp = gf_param (hdr);
	GF_VALIDATE_OR_GOTO(frame->this->name, rsp, fail);

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	if (op_ret >= 0) {
		op_ret = -1;
		dict_len = ntoh32 (rsp->dict_len);

		if (dict_len > 0) {
			dictbuf = memdup (rsp->dict, dict_len);
			GF_VALIDATE_OR_GOTO(frame->this->name, dictbuf, fail);

			dict = get_new_dict();
			GF_VALIDATE_OR_GOTO(frame->this->name, dict, fail);
			dict_ref (dict);
                                               
			ret = dict_unserialize (dictbuf, dict_len, &dict);
			if (ret < 0) {
				gf_log (frame->this->name, GF_LOG_ERROR,
					"failed to serialize dictionary(%p)",
					dict);
				goto fail;
			} else {
				dict->extra_free = dictbuf;
				dictbuf = NULL;
			}
		}
		op_ret = 0;
	}
	gf_errno = ntoh32 (hdr->rsp.op_errno);
	op_errno = gf_error_to_errno (gf_errno);


fail:
	STACK_UNWIND (frame, op_ret, op_errno, dict);
	
	if (dictbuf)
		free (dictbuf);
	if (dict)
		dict_unref (dict);

	return 0;
}

/*
 * client_chown_cbk -
 *
 * @frame:
 * @args:
 *
 * not for external reference
 */
int32_t
client_fchown_cbk (call_frame_t *frame,
                   gf_hdr_common_t *hdr, size_t hdrlen,
                   char *buf, size_t buflen)
{
	struct stat stbuf = {0, };
	gf_fop_fchown_rsp_t *rsp = NULL;
	int32_t op_ret = 0;
	int32_t op_errno = 0;

	rsp = gf_param (hdr);

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	if (op_ret == 0) {
		gf_stat_to_stat (&rsp->stat, &stbuf);
	}

	STACK_UNWIND (frame, op_ret, op_errno, &stbuf);

	return 0;
}


/*
 * client_fchmod_cbk
 *
 * @frame:
 * @args:
 *
 * not for external reference
 */
int32_t
client_fchmod_cbk (call_frame_t *frame,
                   gf_hdr_common_t *hdr, size_t hdrlen,
                   char *buf, size_t buflen)
{
	struct stat stbuf = {0, };
	gf_fop_fchmod_rsp_t *rsp = NULL;
	int32_t op_ret = 0;
	int32_t op_errno = 0;

	rsp = gf_param (hdr);

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	if (op_ret == 0) {
		gf_stat_to_stat (&rsp->stat, &stbuf);
	}

	STACK_UNWIND (frame, op_ret, op_errno, &stbuf);

	return 0;
}


/*
 * client_create_cbk - create callback function for client protocol
 * @frame: call frame
 * @args: arguments in dictionary
 *
 * not for external reference
 */

int
client_create_cbk (call_frame_t *frame,
                   gf_hdr_common_t *hdr, size_t hdrlen,
                   char *buf, size_t buflen)
{
	gf_fop_create_rsp_t  *rsp = NULL;
	int32_t               op_ret = 0;
	int32_t               op_errno = 0;
	fd_t                 *fd = NULL;
	inode_t              *inode = NULL;
	struct stat           stbuf = {0, };
	int64_t               remote_fd = 0;
	char                  key[32] = {0, };
	int32_t               ret = -1;
	client_local_t       *local = NULL;
	client_conf_t        *conf = NULL;


	local = frame->local; frame->local = NULL;
	conf  = frame->this->private;
	fd    = local->fd;
	inode = local->loc.inode;

	rsp = gf_param (hdr);

	op_ret    = ntoh32 (hdr->rsp.op_ret);
	op_errno  = ntoh32 (hdr->rsp.op_errno);

	if (op_ret >= 0) {
		remote_fd = ntoh64 (rsp->fd);
		gf_stat_to_stat (&rsp->stat, &stbuf);
	}

	if (op_ret >= 0) {
		this_ino_set (&local->loc, frame->this, stbuf.st_ino);
		this_fd_set (fd, frame->this, &local->loc, remote_fd);

		sprintf (key, "%p", fd);

		pthread_mutex_lock (&conf->mutex);
		{
			ret = dict_set_str (conf->saved_fds, key, "");
		}
		pthread_mutex_unlock (&conf->mutex);

		if (ret < 0) {
			free (key);
			gf_log (frame->this->name, GF_LOG_ERROR,
				"%s (%"PRId64"): failed to save remote fd", 
				local->loc.path, stbuf.st_ino);
		}
	}

	STACK_UNWIND (frame, op_ret, op_errno, fd, inode, &stbuf);
	
	client_local_wipe (local);

	return 0;
}


/*
 * client_open_cbk - open callback for client protocol
 * @frame: call frame
 * @args: argument dictionary
 *
 * not for external reference
 */
int32_t
client_open_cbk (call_frame_t *frame,
                 gf_hdr_common_t *hdr, size_t hdrlen,
                 char *buf, size_t buflen)
{
	int32_t              op_ret = -1;
	int32_t              op_errno = ENOTCONN;
	fd_t                *fd = NULL;
	int64_t              remote_fd = 0;
	gf_fop_open_rsp_t   *rsp = NULL;
	char                 key[32] = {0,};
	int32_t              ret = -1;
	client_local_t      *local = NULL;
	client_conf_t       *conf = NULL;
	

	local = frame->local; frame->local = NULL;
	conf  = frame->this->private;
	fd    = local->fd;

	rsp = gf_param (hdr);

	op_ret    = ntoh32 (hdr->rsp.op_ret);
	op_errno  = ntoh32 (hdr->rsp.op_errno);

	if (op_ret >= 0) {
		remote_fd = ntoh64 (rsp->fd);
	}

	if (op_ret >= 0) {
		this_fd_set (fd, frame->this, &local->loc, remote_fd);

		sprintf (key, "%p", fd);

		pthread_mutex_lock (&conf->mutex);
		{
			ret = dict_set_str (conf->saved_fds, key, "");
		}
		pthread_mutex_unlock (&conf->mutex);

		if (ret < 0) {
			gf_log (frame->this->name, GF_LOG_ERROR,
				"%s (%"PRId64"): failed to save remote fd", 
				local->loc.path, local->loc.inode->ino);
			free (key);
		}

	}

	STACK_UNWIND (frame, op_ret, op_errno, fd);
	
	client_local_wipe (local);

	return 0;
}

/*
 * client_stat_cbk - stat callback for client protocol
 * @frame: call frame
 * @args: arguments dictionary
 *
 * not for external reference
 */
int
client_stat_cbk (call_frame_t *frame,
                 gf_hdr_common_t *hdr, size_t hdrlen,
                 char *buf, size_t buflen)
{
	struct stat        stbuf = {0, };
	gf_fop_stat_rsp_t *rsp = NULL;
	int32_t            op_ret = 0;
	int32_t            op_errno = 0;

	rsp = gf_param (hdr);

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	if (op_ret == 0) {
		gf_stat_to_stat (&rsp->stat, &stbuf);
	}

	STACK_UNWIND (frame, op_ret, op_errno, &stbuf);

	return 0;
}

/*
 * client_utimens_cbk - utimens callback for client protocol
 * @frame: call frame
 * @args: argument dictionary
 *
 * not for external reference
 */

int32_t
client_utimens_cbk (call_frame_t *frame,
                    gf_hdr_common_t *hdr, size_t hdrlen,
                    char *buf, size_t buflen)
{
	struct stat stbuf = {0, };
	gf_fop_utimens_rsp_t *rsp = NULL;
	int32_t op_ret = 0;
	int32_t op_errno = 0;

	rsp = gf_param (hdr);

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	if (op_ret == 0) {
		gf_stat_to_stat (&rsp->stat, &stbuf);
	}

	STACK_UNWIND (frame, op_ret, op_errno, &stbuf);

	return 0;
}

/*
 * client_chmod_cbk - chmod for client protocol
 * @frame: call frame
 * @args: argument dictionary
 *
 * not for external reference
 */
int32_t
client_chmod_cbk (call_frame_t *frame,
                  gf_hdr_common_t *hdr, size_t hdrlen,
                  char *buf, size_t buflen)
{
	struct stat stbuf = {0, };
	gf_fop_chmod_rsp_t *rsp = NULL;
	int32_t op_ret = 0;
	int32_t op_errno = 0;

	rsp = gf_param (hdr);

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	if (op_ret == 0) {
		gf_stat_to_stat (&rsp->stat, &stbuf);
	}

	STACK_UNWIND (frame, op_ret, op_errno, &stbuf);

	return 0;
}

/*
 * client_chown_cbk - chown for client protocol
 * @frame: call frame
 * @args: argument dictionary
 *
 * not for external reference
 */
int32_t
client_chown_cbk (call_frame_t *frame,
                  gf_hdr_common_t *hdr, size_t hdrlen,
                  char *buf, size_t buflen)
{
	struct stat stbuf = {0, };
	gf_fop_chown_rsp_t *rsp = NULL;
	int32_t op_ret = 0;
	int32_t op_errno = 0;

	rsp = gf_param (hdr);

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	if (op_ret == 0) {
		gf_stat_to_stat (&rsp->stat, &stbuf);
	}

	STACK_UNWIND (frame, op_ret, op_errno, &stbuf);

	return 0;
}

/*
 * client_mknod_cbk - mknod callback for client protocol
 * @frame: call frame
 * @args: argument dictionary
 *
 * not for external reference
 */
int32_t
client_mknod_cbk (call_frame_t *frame,
                  gf_hdr_common_t *hdr, size_t hdrlen,
                  char *buf, size_t buflen)
{
	gf_fop_mknod_rsp_t *rsp = NULL;
	int32_t op_ret = 0;
	int32_t op_errno = 0;
	struct stat stbuf = {0, };
	inode_t *inode = NULL;
	client_local_t *local = NULL;

	local = frame->local;
	frame->local = NULL;
	inode = local->loc.inode;

	rsp = gf_param (hdr);

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	if (op_ret >= 0) {
		gf_stat_to_stat (&rsp->stat, &stbuf);
		this_ino_set (&local->loc, frame->this, stbuf.st_ino);
	}

	STACK_UNWIND (frame, op_ret, op_errno, inode, &stbuf);
	
	client_local_wipe (local);

	return 0;
}

/*
 * client_symlink_cbk - symlink callback for client protocol
 * @frame: call frame
 * @args: argument dictionary
 *
 * not for external reference
 */
int32_t
client_symlink_cbk (call_frame_t *frame,
                    gf_hdr_common_t *hdr, size_t hdrlen,
                    char *buf, size_t buflen)
{
	gf_fop_symlink_rsp_t *rsp = NULL;
	int32_t op_ret = 0;
	int32_t op_errno = 0;
	struct stat stbuf = {0, };
	inode_t *inode = NULL;
	client_local_t *local = NULL;

	local = frame->local;
	frame->local = NULL;
	inode = local->loc.inode;

	rsp = gf_param (hdr);

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	if (op_ret >= 0) {
		gf_stat_to_stat (&rsp->stat, &stbuf);
		this_ino_set (&local->loc, frame->this, stbuf.st_ino);
	}

	STACK_UNWIND (frame, op_ret, op_errno, inode, &stbuf);
	
	client_local_wipe (local);

	return 0;
}

/*
 * client_link_cbk - link callback for client protocol
 * @frame: call frame
 * @args: argument dictionary
 *
 * not for external reference
 */
int32_t
client_link_cbk (call_frame_t *frame,
                 gf_hdr_common_t *hdr, size_t hdrlen,
                 char *buf, size_t buflen)
{
	gf_fop_link_rsp_t *rsp = NULL;
	int32_t op_ret = 0;
	int32_t op_errno = 0;
	struct stat stbuf = {0, };
	inode_t *inode = NULL;
	client_local_t *local = NULL;

	local = frame->local;
	frame->local = NULL;
	inode = local->loc.inode;

	rsp = gf_param (hdr);

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	if (op_ret >= 0) {
		gf_stat_to_stat (&rsp->stat, &stbuf);
	}

	STACK_UNWIND (frame, op_ret, op_errno, inode, &stbuf);
	
	client_local_wipe (local);

	return 0;
}

/*
 * client_truncate_cbk - truncate callback for client protocol
 * @frame: call frame
 * @args: argument dictionary
 *
 * not for external reference
 */

int32_t
client_truncate_cbk (call_frame_t *frame,
                     gf_hdr_common_t *hdr, size_t hdrlen,
                     char *buf, size_t buflen)
{
	struct stat stbuf = {0, };
	gf_fop_truncate_rsp_t *rsp = NULL;
	int32_t op_ret = 0;
	int32_t op_errno = 0;

	rsp = gf_param (hdr);

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	if (op_ret == 0) {
		gf_stat_to_stat (&rsp->stat, &stbuf);
	}

	STACK_UNWIND (frame, op_ret, op_errno, &stbuf);

	return 0;
}

/* client_fstat_cbk - fstat callback for client protocol
 * @frame: call frame
 * @args: argument dictionary
 *
 * not for external reference
 */

int32_t
client_fstat_cbk (call_frame_t *frame,
                  gf_hdr_common_t *hdr, size_t hdrlen,
                  char *buf, size_t buflen)
{
	struct stat stbuf = {0, };
	gf_fop_fstat_rsp_t *rsp = NULL;
	int32_t op_ret = 0;
	int32_t op_errno = 0;

	rsp = gf_param (hdr);

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	if (op_ret == 0) {
		gf_stat_to_stat (&rsp->stat, &stbuf);
	}

	STACK_UNWIND (frame, op_ret, op_errno, &stbuf);

	return 0;
}

/*
 * client_ftruncate_cbk - ftruncate callback for client protocol
 * @frame: call frame
 * @args: argument dictionary
 *
 * not for external reference
 */
int32_t
client_ftruncate_cbk (call_frame_t *frame,
                      gf_hdr_common_t *hdr, size_t hdrlen,
                      char *buf, size_t buflen)
{
	struct stat stbuf = {0, };
	gf_fop_ftruncate_rsp_t *rsp = NULL;
	int32_t op_ret = 0;
	int32_t op_errno = 0;

	rsp = gf_param (hdr);

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	if (op_ret == 0) {
		gf_stat_to_stat (&rsp->stat, &stbuf);
	}

	STACK_UNWIND (frame, op_ret, op_errno, &stbuf);

	return 0;
}

/* client_readv_cbk - readv callback for client protocol
 * @frame: call frame
 * @args: argument dictionary
 *
 * not for external referece
 */

int32_t
client_readv_cbk (call_frame_t *frame,
                  gf_hdr_common_t *hdr, size_t hdrlen,
                  char *buf, size_t buflen)
{
	gf_fop_read_rsp_t *rsp = NULL;
	int32_t op_ret = 0;
	int32_t op_errno = 0;
	struct iovec vector = {0, };
	struct stat stbuf = {0, };
	dict_t *refs = NULL;

	rsp = gf_param (hdr);

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	if (op_ret != -1) {
		gf_stat_to_stat (&rsp->stat, &stbuf);
		vector.iov_base = buf;
		vector.iov_len  = buflen;

		refs = get_new_dict ();
		dict_set (refs, NULL, data_from_dynptr (buf, 0));
		frame->root->rsp_refs = dict_ref (refs);
	}

	STACK_UNWIND (frame, op_ret, op_errno, &vector, 1, &stbuf);

	if (refs)
		dict_unref (refs);

	return 0;
}

/*
 * client_write_cbk - write callback for client protocol
 * @frame: cal frame
 * @args: argument dictionary
 *
 * not for external reference
 */

int32_t
client_write_cbk (call_frame_t *frame,
                  gf_hdr_common_t *hdr, size_t hdrlen,
                  char *buf, size_t buflen)
{
	gf_fop_write_rsp_t *rsp = NULL;
	int32_t op_ret = 0;
	int32_t op_errno = 0;
	struct stat stbuf = {0, };

	rsp = gf_param (hdr);

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	if (op_ret >= 0)
		gf_stat_to_stat (&rsp->stat, &stbuf);

	STACK_UNWIND (frame, op_ret, op_errno, &stbuf);

	return 0;
}


int32_t
client_readdir_cbk (call_frame_t *frame,
                    gf_hdr_common_t *hdr, size_t hdrlen,
                    char *buf, size_t buflen)
{
	gf_fop_readdir_rsp_t *rsp = NULL;
	int32_t op_ret = 0;
	int32_t op_errno = 0;
	uint32_t buf_size = 0;
	gf_dirent_t entries;

	rsp = gf_param (hdr);

	op_ret    = ntoh32 (hdr->rsp.op_ret);
	op_errno  = ntoh32 (hdr->rsp.op_errno);

	INIT_LIST_HEAD (&entries.list);
	if (op_ret > 0) {
		buf_size = ntoh32 (rsp->size);
		gf_dirent_unserialize (&entries, rsp->buf, buf_size);
	}

	STACK_UNWIND (frame, op_ret, op_errno, &entries);

	gf_dirent_free (&entries);

	return 0;
}

/*
 * client_fsync_cbk - fsync callback for client protocol
 *
 * @frame: call frame
 * @args: argument dictionary
 *
 * not for external reference
 */
int32_t
client_fsync_cbk (call_frame_t *frame,
                  gf_hdr_common_t *hdr, size_t hdrlen,
                  char *buf, size_t buflen)
{
	struct stat stbuf = {0, };
	gf_fop_fsync_rsp_t *rsp = NULL;
	int32_t op_ret = 0;
	int32_t op_errno = 0;

	rsp = gf_param (hdr);

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	STACK_UNWIND (frame, op_ret, op_errno, &stbuf);

	return 0;
}


/*
 * client_unlink_cbk - unlink callback for client protocol
 * @frame: call frame
 * @args: argument dictionary
 *
 * not for external reference
 */
int32_t
client_unlink_cbk (call_frame_t *frame,
                   gf_hdr_common_t *hdr, size_t hdrlen,
                   char *buf, size_t buflen)
{
	gf_fop_unlink_rsp_t *rsp = NULL;
	int32_t op_ret = 0;
	int32_t op_errno = 0;

	rsp = gf_param (hdr);

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	STACK_UNWIND (frame, op_ret, op_errno);

	return 0;
}

/*
 * client_rename_cbk - rename callback for client protocol
 * @frame: call frame
 * @args: argument dictionary
 *
 * not for external reference
 */
int32_t
client_rename_cbk (call_frame_t *frame,
                   gf_hdr_common_t *hdr, size_t hdrlen,
                   char *buf, size_t buflen)
{
	struct stat stbuf = {0, };
	gf_fop_rename_rsp_t *rsp = NULL;
	int32_t op_ret = 0;
	int32_t op_errno = 0;

	rsp = gf_param (hdr);

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	if (op_ret == 0) {
		gf_stat_to_stat (&rsp->stat, &stbuf);
	}

	STACK_UNWIND (frame, op_ret, op_errno, &stbuf);

	return 0;
}


/*
 * client_readlink_cbk - readlink callback for client protocol
 *
 * @frame: call frame
 * @args: argument dictionary
 *
 * not for external reference
 */
int32_t
client_readlink_cbk (call_frame_t *frame,
                     gf_hdr_common_t *hdr, size_t hdrlen,
                     char *buf, size_t buflen)
{
	gf_fop_readlink_rsp_t *rsp = NULL;
	int32_t op_ret = 0;
	int32_t op_errno = 0;
	char *link = NULL;

	rsp = gf_param (hdr);

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	if (op_ret > 0) {
		link = rsp->path;
	}

	STACK_UNWIND (frame, op_ret, op_errno, link);
	return 0;
}

/*
 * client_mkdir_cbk - mkdir callback for client protocol
 * @frame: call frame
 * @args: argument dictionary
 *
 * not for external reference
 */
int32_t
client_mkdir_cbk (call_frame_t *frame,
                  gf_hdr_common_t *hdr, size_t hdrlen,
                  char *buf, size_t buflen)
{
	gf_fop_mkdir_rsp_t *rsp = NULL;
	int32_t op_ret = 0;
	int32_t op_errno = 0;
	struct stat stbuf = {0, };
	inode_t *inode = NULL;
	client_local_t *local = NULL;

	local = frame->local;
	inode = local->loc.inode;
	frame->local = NULL;

	rsp = gf_param (hdr);

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	if (op_ret >= 0) {
		gf_stat_to_stat (&rsp->stat, &stbuf);
		this_ino_set (&local->loc, frame->this, stbuf.st_ino);
	}

	STACK_UNWIND (frame, op_ret, op_errno, inode, &stbuf);
	
	client_local_wipe (local);

	return 0;
}

/*
 * client_flush_cbk - flush callback for client protocol
 *
 * @frame: call frame
 * @args: argument dictionary
 *
 * not for external reference
 */

int32_t
client_flush_cbk (call_frame_t *frame,
                  gf_hdr_common_t *hdr, size_t hdrlen,
                  char *buf, size_t buflen)
{
	int32_t op_ret = 0;
	int32_t op_errno = 0;

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	STACK_UNWIND (frame, op_ret, op_errno);

	return 0;
}


/*
 * client_opendir_cbk - opendir callback for client protocol
 * @frame: call frame
 * @args: argument dictionary
 *
 * not for external reference
 */
int
client_opendir_cbk (call_frame_t *frame,
                    gf_hdr_common_t *hdr, size_t hdrlen,
                    char *buf, size_t buflen)
{
	int32_t               op_ret   = -1;
	int32_t               op_errno = ENOTCONN;
	fd_t                 *fd       = NULL;
	int64_t               remote_fd = 0;
	gf_fop_opendir_rsp_t *rsp       = NULL;
	char                  key[32] = {0,};
	int32_t               ret     = -1;
	client_local_t       *local = NULL;
	client_conf_t        *conf = NULL;


	local = frame->local; frame->local = NULL;
	conf  = frame->this->private;
	fd    = local->fd;

	rsp = gf_param (hdr);

	op_ret    = ntoh32 (hdr->rsp.op_ret);
	op_errno  = ntoh32 (hdr->rsp.op_errno);

	if (op_ret >= 0) {
		remote_fd = ntoh64 (rsp->fd);
	}

	if (op_ret >= 0) {
		this_fd_set (fd, frame->this, &local->loc, remote_fd);

		sprintf (key, "%p", fd);

		pthread_mutex_lock (&conf->mutex);
		{
			ret = dict_set_str (conf->saved_fds, key, "");
		}
		pthread_mutex_unlock (&conf->mutex);

		if (ret < 0) {
			free (key);
			gf_log (frame->this->name, GF_LOG_ERROR,
				"%s (%"PRId64"): failed to save remote fd", 
				local->loc.path, local->loc.inode->ino);
		}
	}

	STACK_UNWIND (frame, op_ret, op_errno, fd);
	
	client_local_wipe (local);

	return 0;
}


/*
 * client_rmdir_cbk - rmdir callback for client protocol
 * @frame: call frame
 * @args: argument dictionary
 *
 * not for external reference
 */

int
client_rmdir_cbk (call_frame_t *frame,
                  gf_hdr_common_t *hdr, size_t hdrlen,
                  char *buf, size_t buflen)
{
	gf_fop_rmdir_rsp_t *rsp = NULL;
	int32_t op_ret = 0;
	int32_t op_errno = 0;

	rsp = gf_param (hdr);

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	STACK_UNWIND (frame, op_ret, op_errno);

	return 0;
}

/*
 * client_access_cbk - access callback for client protocol
 * @frame: call frame
 * @args: argument dictionary
 *
 * not for external reference
 */
int32_t
client_access_cbk (call_frame_t *frame,
                   gf_hdr_common_t *hdr, size_t hdrlen,
                   char *buf, size_t buflen)
{
	gf_fop_access_rsp_t *rsp = NULL;
	int32_t op_ret = 0;
	int32_t op_errno = 0;

	rsp = gf_param (hdr);

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	STACK_UNWIND (frame, op_ret, op_errno);

	return 0;
}



/*
 * client_lookup_cbk - lookup callback for client protocol
 *
 * @frame: call frame
 * @args: arguments dictionary
 *
 * not for external reference
 */
int32_t
client_lookup_cbk (call_frame_t *frame,
                   gf_hdr_common_t *hdr, size_t hdrlen,
                   char *buf, size_t buflen)
{
	struct stat stbuf = {0, };
	inode_t *inode = NULL;
	dict_t *xattr = NULL;
	gf_fop_lookup_rsp_t *rsp = NULL;
	int32_t op_ret = 0;
	int32_t op_errno = 0;
	size_t dict_len = 0;
	char *dictbuf = NULL;
	int32_t ret = -1;
	int32_t gf_errno = 0;
	client_local_t *local = NULL;

	local = frame->local; 
	inode = local->loc.inode;
	frame->local = NULL;

	rsp = gf_param (hdr);

	op_ret   = ntoh32 (hdr->rsp.op_ret);

	if (op_ret == 0) {
		op_ret = -1;
		gf_stat_to_stat (&rsp->stat, &stbuf);
		this_ino_set (&local->loc, frame->this, stbuf.st_ino);

		dict_len = ntoh32 (rsp->dict_len);

		if (dict_len > 0) {
			dictbuf = memdup (rsp->dict, dict_len);
			GF_VALIDATE_OR_GOTO(frame->this->name, dictbuf, fail);
			
			xattr = dict_new();
			GF_VALIDATE_OR_GOTO(frame->this->name, xattr, fail);

			ret = dict_unserialize (dictbuf, dict_len, &xattr);
			if (ret < 0) {
				gf_log (frame->this->name, GF_LOG_ERROR,
					"%s (%"PRId64"): failed to unserialize dictionary",
					local->loc.path, inode->ino);
				goto fail;
			} else {
				xattr->extra_free = dictbuf;
				dictbuf = NULL;
			}
		}
		op_ret = 0;
	}
	gf_errno = ntoh32 (hdr->rsp.op_errno);
	op_errno = gf_error_to_errno (gf_errno);

fail:
	STACK_UNWIND (frame, op_ret, op_errno, inode, &stbuf, xattr);
	
	client_local_wipe (local);

	if (dictbuf)
		free (dictbuf);

	if (xattr)
		dict_unref (xattr);

	return 0;
}

static dir_entry_t *
gf_bin_to_direntry (char *buf, size_t count)
{
	int32_t idx = 0, bread = 0;
	size_t rcount = 0;
	char *ender = NULL, *buffer = NULL;
	char tmp_buf[512] = {0,};
	dir_entry_t *trav = NULL, *prev = NULL;
	dir_entry_t *thead = NULL, *head = NULL;

	thead = CALLOC (1, sizeof (dir_entry_t));
	GF_VALIDATE_OR_GOTO("client-protocol", thead, fail);

	buffer = buf;
	prev = thead;

	for (idx = 0; idx < count ; idx++) {
		bread = 0;
		trav = CALLOC (1, sizeof (dir_entry_t));
		GF_VALIDATE_OR_GOTO("client-protocol", trav, fail);

		ender = strchr (buffer, '/');
		if (!ender)
			break;
		rcount = ender - buffer;
		trav->name = CALLOC (1, rcount + 2);
		GF_VALIDATE_OR_GOTO("client-protocol", trav->name, fail);

		strncpy (trav->name, buffer, rcount);
		bread = rcount + 1;
		buffer += bread;

		ender = strchr (buffer, '\n');
		if (!ender)
			break;
		rcount = ender - buffer;
		strncpy (tmp_buf, buffer, rcount);
		bread = rcount + 1;
		buffer += bread;
			
		gf_string_to_stat (tmp_buf, &trav->buf);

		ender = strchr (buffer, '\n');
		if (!ender)
			break;
		rcount = ender - buffer;
		*ender = '\0';
		if (S_ISLNK (trav->buf.st_mode))
			trav->link = strdup (buffer);
		else
			trav->link = "";

		bread = rcount + 1;
		buffer += bread;

		prev->next = trav;
		prev = trav;
	}
	
	head = thead;
fail:
	return head;
}

int32_t
gf_free_direntry(dir_entry_t *head)
{
	dir_entry_t *prev = NULL, *trav = NULL;

	prev = head;
	GF_VALIDATE_OR_GOTO("client-protocol", prev, fail);

	trav = head->next;
	while (trav) {
		prev->next = trav->next;
		FREE (trav->name);
		if (S_ISLNK (trav->buf.st_mode))
			FREE (trav->link);
		FREE (trav);
		trav = prev->next;
	}
	FREE (head);
fail:
	return 0;
}
/*
 * client_getdents_cbk - readdir callback for client protocol
 * @frame: call frame
 * @args: argument dictionary
 *
 * not for external reference
 */
int32_t
client_getdents_cbk (call_frame_t *frame,
                     gf_hdr_common_t *hdr, size_t hdrlen,
                     char *buf, size_t buflen)
{
	gf_fop_getdents_rsp_t *rsp = NULL;
	int32_t op_ret = 0;
	int32_t op_errno = 0;
	int32_t gf_errno = 0;
	int32_t nr_count = 0;
	dir_entry_t *entry = NULL;

	rsp = gf_param (hdr);

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	gf_errno = ntoh32 (hdr->rsp.op_errno);
	op_errno = gf_error_to_errno (gf_errno);

	if (op_ret >= 0) {
		nr_count = ntoh32 (rsp->count);		
		entry = gf_bin_to_direntry(buf, nr_count);
		if (entry == NULL) {
			op_ret = -1;
			op_errno = EINVAL;
		}
	}

	STACK_UNWIND (frame, op_ret, op_errno, entry, nr_count);

	if (op_ret >= 0) {
		/* Free the buffer */
		FREE (buf);
		gf_free_direntry(entry);
      	}

	return 0;
}

/*
 * client_statfs_cbk - statfs callback for client protocol
 * @frame: call frame
 * @args: argument dictionary
 *
 * not for external reference
 */
int32_t
client_statfs_cbk (call_frame_t *frame,
                   gf_hdr_common_t *hdr, size_t hdrlen,
                   char *buf, size_t buflen)
{
	struct statvfs stbuf = {0, };
	gf_fop_statfs_rsp_t *rsp = NULL;
	int32_t op_ret = 0;
	int32_t op_errno = 0;

	rsp = gf_param (hdr);

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	if (op_ret == 0)
	{
		gf_statfs_to_statfs (&rsp->statfs, &stbuf);
	}

	STACK_UNWIND (frame, op_ret, op_errno, &stbuf);

	return 0;
}

/*
 * client_fsyncdir_cbk - fsyncdir callback for client protocol
 * @frame: call frame
 * @args: argument dictionary
 *
 * not for external reference
 */
int32_t
client_fsyncdir_cbk (call_frame_t *frame,
                     gf_hdr_common_t *hdr, size_t hdrlen,
                     char *buf, size_t buflen)
{
	int32_t op_ret = 0;
	int32_t op_errno = 0;

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	STACK_UNWIND (frame, op_ret, op_errno);

	return 0;
}

/*
 * client_setxattr_cbk - setxattr callback for client protocol
 * @frame: call frame
 * @args: argument dictionary
 *
 * not for external reference
 */
int32_t
client_setxattr_cbk (call_frame_t *frame,
                     gf_hdr_common_t *hdr, size_t hdrlen,
                     char *buf, size_t buflen)
{
	gf_fop_setxattr_rsp_t *rsp = NULL;
	int32_t op_ret = 0;
	int32_t op_errno = 0;

	rsp = gf_param (hdr);

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	STACK_UNWIND (frame, op_ret, op_errno);

	return 0;
}

/*
 * client_getxattr_cbk - getxattr callback for client protocol
 * @frame: call frame
 * @args: argument dictionary
 *
 * not for external reference
 */
int32_t
client_getxattr_cbk (call_frame_t *frame,
                     gf_hdr_common_t *hdr, size_t hdrlen,
                     char *buf, size_t buflen)
{
	gf_fop_getxattr_rsp_t *rsp = NULL;
	int32_t op_ret   = 0;
	int32_t gf_errno = 0;
	int32_t op_errno = 0;
	int32_t dict_len = 0;
	dict_t *dict = NULL;
	int32_t ret = -1;
	char *dictbuf = NULL;
	client_local_t *local = NULL;
	
	local = frame->local;
	frame->local = NULL;

	rsp = gf_param (hdr);
	GF_VALIDATE_OR_GOTO(frame->this->name, rsp, fail);

	op_ret   = ntoh32 (hdr->rsp.op_ret);

	if (op_ret >= 0) {
		op_ret = -1;
		dict_len = ntoh32 (rsp->dict_len);

		if (dict_len > 0) {
			dictbuf = memdup (rsp->dict, dict_len);
			GF_VALIDATE_OR_GOTO(frame->this->name, dictbuf, fail);

			dict = dict_new();
			GF_VALIDATE_OR_GOTO(frame->this->name, dict, fail);

			ret = dict_unserialize (dictbuf, dict_len, &dict);
			if (ret < 0) {
				gf_log (frame->this->name, GF_LOG_ERROR,
					"%s (%"PRId64"): failed to "
					"unserialize xattr dictionary", 
					local->loc.path, local->loc.inode->ino);
				goto fail;
			} else {
				dict->extra_free = dictbuf;
				dictbuf = NULL;
			}
		}
		op_ret = 0;
	}
	gf_errno = ntoh32 (hdr->rsp.op_errno);
	op_errno = gf_error_to_errno (gf_errno);
fail:
	STACK_UNWIND (frame, op_ret, op_errno, dict);
	
	client_local_wipe (local);

	if (dictbuf)
		free (dictbuf);

	if (dict)
		dict_unref (dict);

	return 0;
}

/*
 * client_removexattr_cbk - removexattr callback for client protocol
 * @frame: call frame
 * @args: argument dictionary
 *
 * not for external reference
 */
int32_t
client_removexattr_cbk (call_frame_t *frame,
                        gf_hdr_common_t *hdr, size_t hdrlen,
                        char *buf, size_t buflen)
{
	int32_t op_ret = 0;
	int32_t op_errno = 0;

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	STACK_UNWIND (frame, op_ret, op_errno);

	return 0;
}

/*
 * client_lk_cbk - lk callback for client protocol
 * @frame: call frame
 * @args: argument dictionary
 *
 * not for external reference
 */
int32_t
client_lk_common_cbk (call_frame_t *frame,
		      gf_hdr_common_t *hdr, size_t hdrlen,
		      char *buf, size_t buflen)
{
	struct flock lock = {0,};
	gf_fop_lk_rsp_t *rsp = NULL;
	int32_t op_ret = 0;
	int32_t op_errno = 0;

	rsp = gf_param (hdr);

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	if (op_ret >= 0) {
		gf_flock_to_flock (&rsp->flock, &lock);
	}

	STACK_UNWIND (frame, op_ret, op_errno, &lock);
	return 0;
}


/*
 * client_gf_file_lk_cbk - gf_file_lk callback for client protocol
 * @frame: call frame
 * @args: argument dictionary
 *
 * not for external reference
 */
int32_t
client_inodelk_cbk (call_frame_t *frame,
		    gf_hdr_common_t *hdr, size_t hdrlen,
		    char *buf, size_t buflen)
{
	gf_fop_inodelk_rsp_t *rsp = NULL;
	int32_t op_ret = 0;
	int32_t op_errno = 0;

	rsp = gf_param (hdr);

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	STACK_UNWIND (frame, op_ret, op_errno);
	return 0;
}


int32_t
client_finodelk_cbk (call_frame_t *frame,
		     gf_hdr_common_t *hdr, size_t hdrlen,
		     char *buf, size_t buflen)
{
	gf_fop_finodelk_rsp_t *rsp = NULL;
	int32_t op_ret = 0;
	int32_t op_errno = 0;

	rsp = gf_param (hdr);

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	STACK_UNWIND (frame, op_ret, op_errno);
	return 0;
}


/*
 * client_entrylk_cbk - entrylk callback for client protocol
 * @frame: call frame
 * @args: argument dictionary
 *
 * not for external reference
 */
int32_t
client_entrylk_cbk (call_frame_t *frame,
		    gf_hdr_common_t *hdr, size_t hdrlen,
		    char *buf, size_t buflen)
{
	gf_fop_entrylk_rsp_t *rsp = NULL;
	int32_t op_ret = 0;
	int32_t op_errno = 0;

	rsp = gf_param (hdr);

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	STACK_UNWIND (frame, op_ret, op_errno);
	return 0;
}

int32_t
client_fentrylk_cbk (call_frame_t *frame,
		     gf_hdr_common_t *hdr, size_t hdrlen,
		     char *buf, size_t buflen)
{
	gf_fop_fentrylk_rsp_t *rsp = NULL;
	int32_t op_ret = 0;
	int32_t op_errno = 0;

	rsp = gf_param (hdr);

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	STACK_UNWIND (frame, op_ret, op_errno);
	return 0;
}


/**
 * client_writedir_cbk -
 *
 * @frame:
 * @args:
 *
 * not for external reference
 */
int32_t
client_setdents_cbk (call_frame_t *frame,
                     gf_hdr_common_t *hdr, size_t hdrlen,
                     char *buf, size_t buflen)
{
	int32_t op_ret = 0;
	int32_t op_errno = 0;

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	STACK_UNWIND (frame, op_ret, op_errno);

	return 0;
}



/*
 * client_stats_cbk - stats callback for client protocol
 *
 * @frame: call frame
 * @args: argument dictionary
 *
 * not for external reference
 */

int32_t
client_stats_cbk (call_frame_t *frame,
                  gf_hdr_common_t *hdr, size_t hdrlen,
                  char *buf, size_t buflen)
{
	struct xlator_stats stats = {0,};
	gf_mop_stats_rsp_t *rsp = NULL;
	char *buffer = NULL;
	int32_t op_ret = 0;
	int32_t op_errno = 0;

	rsp = gf_param (hdr);

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	if (op_ret >= 0)
	{
		buffer = rsp->buf;

		sscanf (buffer, "%"SCNx64",%"SCNx64",%"SCNx64",%"SCNx64
			",%"SCNx64",%"SCNx64",%"SCNx64",%"SCNx64"\n",
			&stats.nr_files,
			&stats.disk_usage,
			&stats.free_disk,
			&stats.total_disk_size,
			&stats.read_usage,
			&stats.write_usage,
			&stats.disk_speed,
			&stats.nr_clients);
	}

	STACK_UNWIND (frame, op_ret, op_errno, &stats);
	return 0;
}

/*
 * client_getspec - getspec function for client protocol
 * @frame: call frame
 * @this: client protocol xlator structure
 * @flag:
 *
 * external reference through client_protocol_xlator->fops->getspec
 */
int32_t
client_getspec (call_frame_t *frame,
                xlator_t *this,
		const char *key,
                int32_t flag)
{
	gf_hdr_common_t *hdr = NULL;
	gf_mop_getspec_req_t *req = NULL;
	size_t hdrlen = -1;
	int keylen = 0;
	int ret = -1;

	if (key)
		keylen = STRLEN_0(key);

	hdrlen = gf_hdr_len (req, keylen);
	hdr    = gf_hdr_new (req, keylen);
	GF_VALIDATE_OR_GOTO(this->name, hdr, unwind);

	req        = gf_param (hdr);
	req->flags = hton32 (flag);
	req->keylen = hton32 (keylen);
	if (keylen)
		strcpy (req->key, key);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_BULK),
				    GF_OP_TYPE_MOP_REQUEST, GF_MOP_GETSPEC,
				    hdr, hdrlen, NULL, 0, NULL);

	return ret;
unwind:
	if (hdr)
		free (hdr);
	STACK_UNWIND(frame, -1, EINVAL, NULL);
	return 0;
}


/*
 * client_getspec_cbk - getspec callback for client protocol
 *
 * @frame: call frame
 * @args: argument dictionary
 *
 * not for external reference
 */

int32_t
client_getspec_cbk (call_frame_t *frame,
                    gf_hdr_common_t *hdr, size_t hdrlen,
                    char *buf, size_t buflen)
{
	gf_mop_getspec_rsp_t *rsp = NULL;
	char *spec_data = NULL;
	int32_t op_ret = 0;
	int32_t op_errno = 0;
	int32_t gf_errno = 0;

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	gf_errno = ntoh32 (hdr->rsp.op_errno);
	op_errno = gf_error_to_errno (gf_errno);
	rsp = gf_param (hdr);

	if (op_ret >= 0) {
		spec_data = rsp->spec;
	}

	STACK_UNWIND (frame, op_ret, op_errno, spec_data);
	return 0;
}

int32_t
client_checksum (call_frame_t *frame,
                 xlator_t *this,
                 loc_t *loc,
                 int32_t flag)
{
	gf_hdr_common_t *hdr = NULL;
	gf_fop_checksum_req_t *req = NULL;
	size_t hdrlen = -1;
	int ret = -1;
	client_conf_t *conf = this->private;
	ino_t ino = 0;

	if (conf->child) {
		STACK_WIND (frame,
			    default_checksum_cbk,
			    conf->child,
			    conf->child->fops->checksum,
			    loc,
			    flag);

		return 0;
	}

	hdrlen = gf_hdr_len (req, strlen (loc->path) + 1);
	hdr    = gf_hdr_new (req, strlen (loc->path) + 1);
	req    = gf_param (hdr);
	
	ino = this_ino_get (loc, this, GF_CLIENT_INODE_SELF);
	req->ino  = hton64 (ino);
	req->flag = hton32 (flag);
	strcpy (req->path, loc->path);

	ret = protocol_client_xfer (frame, this,
				    CLIENT_CHANNEL (this, CHANNEL_BULK),
				    GF_OP_TYPE_FOP_REQUEST, GF_FOP_CHECKSUM,
				    hdr, hdrlen, NULL, 0, NULL);

	return ret;
}

int32_t
client_checksum_cbk (call_frame_t *frame,
                     gf_hdr_common_t *hdr, size_t hdrlen,
                     char *buf, size_t buflen)
{
	gf_fop_checksum_rsp_t *rsp = NULL;
	int32_t op_ret = 0;
	int32_t op_errno = 0;
	int32_t gf_errno = 0;
	unsigned char *fchecksum = NULL;
	unsigned char *dchecksum = NULL;

	rsp = gf_param (hdr);

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	gf_errno = ntoh32 (hdr->rsp.op_errno);
	op_errno = gf_error_to_errno (gf_errno);

	if (op_ret >= 0) {
		fchecksum = rsp->fchecksum;
		dchecksum = rsp->dchecksum + ZR_FILENAME_MAX;
	}

	STACK_UNWIND (frame, op_ret, op_errno, fchecksum, dchecksum);
	return 0;
}


/*
 * client_setspec_cbk - setspec callback for client protocol
 * @frame: call frame
 * @args: argument dictionary
 *
 * not for external reference
 */

int32_t
client_setspec_cbk (call_frame_t *frame,
                    gf_hdr_common_t *hdr, size_t hdrlen,
                    char *buf, size_t buflen)
{
	int32_t op_ret = 0;
	int32_t op_errno = 0;

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	STACK_UNWIND (frame, op_ret, op_errno);

	return 0;
}

/*
 * client_setvolume_cbk - setvolume callback for client protocol
 * @frame:  call frame
 * @args: argument dictionary
 *
 * not for external reference
 */
int
client_setvolume_cbk (call_frame_t *frame,
                      gf_hdr_common_t *hdr, size_t hdrlen,
                      char *buf, size_t buflen)
{
	gf_mop_setvolume_rsp_t *rsp = NULL;
	client_connection_t    *conn = NULL;
	client_conf_t          *conf = NULL;
	glusterfs_ctx_t        *ctx = NULL; 
	xlator_t               *this = NULL;
	xlator_list_t          *parent = NULL;
	transport_t            *trans = NULL;
	dict_t                 *reply = NULL;
	char                   *remote_subvol = NULL;
	char                   *remote_error = NULL;
	char                   *process_uuid = NULL;
	int32_t                 ret = -1;
	int32_t                 op_ret   = -1;
	int32_t                 op_errno = EINVAL;
	int32_t                 dict_len = 0;


	trans = frame->local; frame->local = NULL;
	this  = frame->this;
	conf  = this->private;
	conn  = trans->xl_private;

	rsp = gf_param (hdr);

	op_ret   = ntoh32 (hdr->rsp.op_ret);
	op_errno = gf_error_to_errno (ntoh32 (hdr->rsp.op_errno));

	if (op_ret < 0 && op_errno == ENOTCONN) {
		gf_log (this->name, GF_LOG_ERROR,
			"setvolume failed (%s)",
			strerror (op_errno));
		goto out;
	}

	reply = dict_new ();
	GF_VALIDATE_OR_GOTO(this->name, reply, out);

	dict_len = ntoh32 (rsp->dict_len);
	ret = dict_unserialize (rsp->buf, dict_len, &reply);
	if (ret < 0) {
		gf_log (frame->this->name, GF_LOG_ERROR,
			"failed to unserialize buffer(%p) to dictionary",
			rsp->buf);
		goto out;
	}
	
	ret = dict_get_str (reply, "ERROR", &remote_error);
	if (ret < 0) {
		gf_log (this->name, GF_LOG_ERROR,
			"failed to get ERROR string from reply dictionary");
	}

	ret = dict_get_str (reply, "process-uuid", &process_uuid);
	if (ret < 0) {
		gf_log (this->name, GF_LOG_DEBUG,
			"failed to get 'process-uuid' from reply dictionary");
	}

	if (op_ret < 0) {
		gf_log (trans->xl->name, GF_LOG_ERROR,
			"SETVOLUME on remote-host failed: %s",
			remote_error ? remote_error : strerror (op_errno));
		errno = op_errno;
		if (op_errno == ENOTCONN)
			goto out;
	} else {
		ctx = get_global_ctx_ptr ();
		if (process_uuid && !strcmp (ctx->process_uuid,process_uuid)) {
			ret = dict_get_str (this->options, "remote-subvolume",
					    &remote_subvol);
			if (!remote_subvol) 
				goto out;
			
			gf_log (this->name, GF_LOG_WARNING, 
				"attaching to the local volume '%s'",
				remote_subvol);

			/* TODO: */
			conf->child = xlator_search_by_name (this, 
							     remote_subvol);
		}
		gf_log (trans->xl->name, GF_LOG_INFO,
			"connection and handshake succeeded");

		pthread_mutex_lock (&(conn->lock));
		{
			conn->connected = 1;
		}
		pthread_mutex_unlock (&(conn->lock));

		parent = trans->xl->parents;
		while (parent) {
			parent->xlator->notify (parent->xlator,
						GF_EVENT_CHILD_UP,
						trans->xl);
			parent = parent->next;
		}
	}

out:
	STACK_DESTROY (frame->root);

	if (reply)
		dict_unref (reply);

	return op_ret;
}

/*
 * client_enosys_cbk -
 * @frame: call frame
 *
 * not for external reference
 */
int
client_enosys_cbk (call_frame_t *frame,
                   gf_hdr_common_t *hdr, size_t hdrlen,
                   char *buf, size_t buflen)
{
	STACK_DESTROY (frame->root);
	return 0;
}


void
client_protocol_reconnect (void *trans_ptr)
{
	transport_t         *trans = NULL;
	client_connection_t *conn = NULL;
	struct timeval       tv = {0, 0};

	trans = trans_ptr;
	conn  = trans->xl_private;
	pthread_mutex_lock (&conn->lock);
	{
		if (conn->reconnect)
			gf_timer_call_cancel (trans->xl->ctx, 
					      conn->reconnect);
		conn->reconnect = 0;

		if (conn->connected == 0) {
			tv.tv_sec = 10;

			gf_log (trans->xl->name, GF_LOG_DEBUG,
				"attempting reconnect");
			transport_connect (trans);

			conn->reconnect = 
				gf_timer_call_after (trans->xl->ctx, tv,
						     client_protocol_reconnect,
						     trans);
		} else {
			gf_log (trans->xl->name, GF_LOG_DEBUG, 
				"breaking reconnect chain");
		}
	}
	pthread_mutex_unlock (&conn->lock);
}

/*
 * client_protocol_cleanup - cleanup function
 * @trans: transport object
 *
 */
int
protocol_client_cleanup (transport_t *trans)
{
	client_connection_t    *conn = NULL;
	struct saved_frames    *saved_frames = NULL;

	conn = trans->xl_private;
			
	gf_log (trans->xl->name, GF_LOG_DEBUG,
		"cleaning up state in transport object %p", trans);

	pthread_mutex_lock (&conn->lock);
	{
		saved_frames = conn->saved_frames;
		conn->saved_frames = saved_frames_new ();

/*
		trav = conn->saved_fds->members_list;
		this = trans->xl;

		while (trav) {
			fd_t *fd_tmp = (fd_t *)(long) strtoul (trav->key, 
							       NULL, 0);
			fd_ctx_del (fd_tmp, this, NULL);
			trav = trav->next;
		}

		dict_destroy (conn->saved_fds);

		conn->saved_fds = get_new_dict_full (64);
*/
		/* bailout logic cleanup */
		memset (&(conn->last_sent), 0, 
			sizeof (conn->last_sent));

		memset (&(conn->last_received), 0, 
			sizeof (conn->last_received));

		if (conn->timer) {
			gf_timer_call_cancel (trans->xl->ctx, conn->timer);
			conn->timer = NULL;
		}

		if (conn->reconnect == NULL) {
			/* :O This part is empty.. any thing missing? */
		}
	}
	pthread_mutex_unlock (&conn->lock);

	saved_frames_destroy (trans->xl, saved_frames,
			      gf_fops, gf_mops, gf_cbks);

	return 0;
}


/* cbk callbacks */
int
client_releasedir_cbk (call_frame_t *frame,
		       gf_hdr_common_t *hdr, size_t hdrlen,
		       char *buf, size_t buflen)
{
	STACK_DESTROY (frame->root);
	return 0;
}


int
client_release_cbk (call_frame_t *frame,
		    gf_hdr_common_t *hdr, size_t hdrlen,
		    char *buf, size_t buflen)
{
	STACK_DESTROY (frame->root);
	return 0;
}


int
client_forget_cbk (call_frame_t *frame,
                   gf_hdr_common_t *hdr, size_t hdrlen,
                   char *buf, size_t buflen)
{
	client_conf_t   *conf = NULL;
	client_forget_t  forget = {0, };
	uint8_t          send_forget = 0;
	int32_t          ret = -1;


	conf = frame->this->private;
	LOCK (&conf->forget.lock);
	{
		conf->forget.frames_in_transit--;

		ret = client_get_forgets (frame->this, &forget);
		if (ret <= 0)
			send_forget = 0;
		else
			send_forget = 1;
	}
	UNLOCK (&conf->forget.lock);

	if (send_forget) {
		ret = protocol_client_xfer (forget.frame, frame->this,
					    CLIENT_CHANNEL (frame->this,
							    CHANNEL_BULK),
					    GF_OP_TYPE_CBK_REQUEST, 
					    GF_CBK_FORGET,
					    forget.hdr, forget.hdrlen, 
					    NULL, 0, NULL);
	}
	
	STACK_DESTROY (frame->root);
	return 0;
}


static gf_op_t gf_fops[] = {
	[GF_FOP_STAT]           =  client_stat_cbk,
	[GF_FOP_READLINK]       =  client_readlink_cbk,
	[GF_FOP_MKNOD]          =  client_mknod_cbk,
	[GF_FOP_MKDIR]          =  client_mkdir_cbk,
	[GF_FOP_UNLINK]         =  client_unlink_cbk,
	[GF_FOP_RMDIR]          =  client_rmdir_cbk,
	[GF_FOP_SYMLINK]        =  client_symlink_cbk,
	[GF_FOP_RENAME]         =  client_rename_cbk,
	[GF_FOP_LINK]           =  client_link_cbk,
	[GF_FOP_CHMOD]          =  client_chmod_cbk,
	[GF_FOP_CHOWN]          =  client_chown_cbk,
	[GF_FOP_TRUNCATE]       =  client_truncate_cbk,
	[GF_FOP_OPEN]           =  client_open_cbk,
	[GF_FOP_READ]           =  client_readv_cbk,
	[GF_FOP_WRITE]          =  client_write_cbk,
	[GF_FOP_STATFS]         =  client_statfs_cbk,
	[GF_FOP_FLUSH]          =  client_flush_cbk,
	[GF_FOP_FSYNC]          =  client_fsync_cbk,
	[GF_FOP_SETXATTR]       =  client_setxattr_cbk,
	[GF_FOP_GETXATTR]       =  client_getxattr_cbk,
	[GF_FOP_REMOVEXATTR]    =  client_removexattr_cbk,
	[GF_FOP_OPENDIR]        =  client_opendir_cbk,
	[GF_FOP_GETDENTS]       =  client_getdents_cbk,
	[GF_FOP_FSYNCDIR]       =  client_fsyncdir_cbk,
	[GF_FOP_ACCESS]         =  client_access_cbk,
	[GF_FOP_CREATE]         =  client_create_cbk,
	[GF_FOP_FTRUNCATE]      =  client_ftruncate_cbk,
	[GF_FOP_FSTAT]          =  client_fstat_cbk,
	[GF_FOP_LK]             =  client_lk_common_cbk,
	[GF_FOP_UTIMENS]        =  client_utimens_cbk,
	[GF_FOP_FCHMOD]         =  client_fchmod_cbk,
	[GF_FOP_FCHOWN]         =  client_fchown_cbk,
	[GF_FOP_LOOKUP]         =  client_lookup_cbk,
	[GF_FOP_SETDENTS]       =  client_setdents_cbk,
	[GF_FOP_READDIR]        =  client_readdir_cbk,
	[GF_FOP_INODELK]        =  client_inodelk_cbk,
	[GF_FOP_FINODELK]       =  client_finodelk_cbk,
	[GF_FOP_ENTRYLK]        =  client_entrylk_cbk,
	[GF_FOP_FENTRYLK]       =  client_fentrylk_cbk,
	[GF_FOP_CHECKSUM]       =  client_checksum_cbk,
	[GF_FOP_XATTROP]        =  client_xattrop_cbk,
	[GF_FOP_FXATTROP]       =  client_fxattrop_cbk,
};

static gf_op_t gf_mops[] = {
	[GF_MOP_SETVOLUME]        =  client_setvolume_cbk,
	[GF_MOP_GETVOLUME]        =  client_enosys_cbk,
	[GF_MOP_STATS]            =  client_stats_cbk,
	[GF_MOP_SETSPEC]          =  client_setspec_cbk,
	[GF_MOP_GETSPEC]          =  client_getspec_cbk,
	[GF_MOP_PING]             =  client_ping_cbk,
};

static gf_op_t gf_cbks[] = {
	[GF_CBK_FORGET]           = client_forget_cbk,
	[GF_CBK_RELEASE]          = client_release_cbk,
	[GF_CBK_RELEASEDIR]       = client_releasedir_cbk
};

/*
 * client_protocol_interpret - protocol interpreter
 * @trans: transport object
 * @blk: data block
 *
 */
int
protocol_client_interpret (xlator_t *this, transport_t *trans,
                           char *hdr_p, size_t hdrlen,
                           char *buf_p, size_t buflen)
{
	int ret = -1;
	call_frame_t *frame = NULL;
	gf_hdr_common_t *hdr = NULL;
	uint64_t callid = 0;
	int type = -1;
	int op = -1;


	hdr  = (gf_hdr_common_t *)hdr_p;

	type   = ntoh32 (hdr->type);
	op     = ntoh32 (hdr->op);
	callid = ntoh64 (hdr->callid);

	frame  = lookup_frame (trans, op, type, callid);
	if (frame == NULL) {
		gf_log (this->name, GF_LOG_ERROR,
			"no frame for callid=%"PRId64" type=%d op=%d",
			callid, type, op);
		return 0;
	}

	switch (type) {
	case GF_OP_TYPE_FOP_REPLY:
		if ((op > GF_FOP_MAXVALUE) || 
		    (op < 0)) {
			gf_log (trans->xl->name, GF_LOG_WARNING,
				"invalid fop '%d'", op);
		} else {
			ret = gf_fops[op] (frame, hdr, hdrlen, buf_p, buflen);
		}
		break;
	case GF_OP_TYPE_MOP_REPLY:
		if ((op > GF_MOP_MAXVALUE) || 
		    (op < 0)) {
			gf_log (trans->xl->name, GF_LOG_WARNING,
				"invalid fop '%d'", op);
		} else {
			ret = gf_mops[op] (frame, hdr, hdrlen, buf_p, buflen);
		}
		break;
	case GF_OP_TYPE_CBK_REPLY:
		if ((op > GF_CBK_MAXVALUE) || 
		    (op < 0)) {
			gf_log (trans->xl->name, GF_LOG_WARNING,
				"invalid cbk '%d'", op);
		} else {
			ret = gf_cbks[op] (frame, hdr, hdrlen, buf_p, buflen);
		}
		break;
	default:
		gf_log (trans->xl->name, GF_LOG_ERROR,
			"invalid packet type: %d", type);
		break;
	}

	return ret;
}

/*
 * init - initiliazation function. called during loading of client protocol
 * @this:
 *
 */
int32_t
init (xlator_t *this)
{
	transport_t               *trans = NULL;
	client_conf_t             *conf = NULL;
	client_connection_t       *conn = NULL;
	int32_t                    transport_timeout = 0;
	int32_t                    ping_timeout = 0;
	data_t                    *remote_subvolume = NULL;
	int32_t                    ret = -1;
	int                        i = 0;

	if (this->children) {
		gf_log (this->name, GF_LOG_ERROR,
			"FATAL: client protocol translator cannot have "
			"subvolumes");
		goto out;
	}
	
	if (!this->parents) {
		gf_log (this->name, GF_LOG_WARNING,
			"dangling volume. check volfile ");
	}

	remote_subvolume = dict_get (this->options, "remote-subvolume");
	if (remote_subvolume == NULL) {
		gf_log (this->name, GF_LOG_ERROR,
			"missing 'option remote-subvolume'.");
		goto out;
	}

	ret = dict_get_int32 (this->options, "transport-timeout", 
			      &transport_timeout);
	if (ret >= 0) {
		gf_log (this->name, GF_LOG_DEBUG,
			"setting transport-timeout to %d", transport_timeout);
	} else {
		gf_log (this->name, GF_LOG_DEBUG,
			"defaulting transport-timeout to 42");
		transport_timeout = 42;
	}
	
	ret = dict_get_int32 (this->options, "ping-timeout", 
			      &ping_timeout);
	if (ret >= 0) {
		gf_log (this->name, GF_LOG_DEBUG,
			"setting ping-timeout to %d", ping_timeout);
	} else {
		gf_log (this->name, GF_LOG_DEBUG,
			"defaulting ping-timeout to 10");
		ping_timeout = 10;
	}
	
	conf = CALLOC (1, sizeof (client_conf_t));

	LOCK_INIT (&conf->forget.lock);
	pthread_mutex_init (&conf->mutex, NULL);
	conf->saved_fds = get_new_dict_full (64);

	this->private = conf;

	for (i = 0; i < CHANNEL_MAX; i++) {
		trans = transport_load (this->options, this);
		if (trans == NULL) {
			gf_log (this->name, GF_LOG_ERROR, 
				"Failed to load transport");
			ret = -1;
			goto out;
		}

		conn = CALLOC (1, sizeof (*conn));

		conn->saved_frames = saved_frames_new ();

		conn->callid = 1;

		memset (&(conn->last_sent), 0, sizeof (conn->last_sent));
		memset (&(conn->last_received), 0,
			sizeof (conn->last_received));

		conn->transport_timeout = transport_timeout;
		conn->ping_timeout = ping_timeout;

		pthread_mutex_init (&conn->lock, NULL);

		trans->xl_private = conn;
		conf->transport[i] = transport_ref (trans);
	}

#ifndef GF_DARWIN_HOST_OS
	{
		struct rlimit lim;

		lim.rlim_cur = 1048576;
		lim.rlim_max = 1048576;
		
		ret = setrlimit (RLIMIT_NOFILE, &lim);
		if (ret == -1) {
			gf_log (this->name, GF_LOG_WARNING,
				"WARNING: Failed to set 'ulimit -n 1M': %s",
				strerror(errno));
			lim.rlim_cur = 65536;
			lim.rlim_max = 65536;
			
			ret = setrlimit (RLIMIT_NOFILE, &lim);
			if (ret == -1) {
				gf_log (this->name, GF_LOG_ERROR,
					"Failed to set max open fd to 64k: %s",
					strerror(errno));
			} else {
				gf_log (this->name, GF_LOG_ERROR,
					"max open fd set to 64k");
			}

		}
	}
#endif
	ret = 0;
out:
	return ret;
}

/*
 * fini - finish function called during unloading of client protocol
 * @this:
 *
 */
void
fini (xlator_t *this)
{
	/* TODO: Check if its enough.. how to call transport's fini () */
	client_conf_t *conf = NULL;

	conf = this->private;
	this->private = NULL;

	if (conf) {
		LOCK_DESTROY (&conf->forget.lock);
		FREE (conf);
	}
	return;
}


int
protocol_client_handshake (xlator_t *this, transport_t *trans)
{
	gf_hdr_common_t        *hdr = NULL;
	gf_mop_setvolume_req_t *req = NULL;
	dict_t                 *options = NULL;
	int32_t                 ret = -1;
	int                     hdrlen = 0;
	int                     dict_len = 0;
	call_frame_t           *fr = NULL;
	char                   *process_uuid_xl;

	options = this->options;
	ret = dict_set_str (options, "version", PACKAGE_VERSION);
	if (ret < 0) {
		gf_log (this->name, GF_LOG_ERROR,
			"failed to set version(%s) in options dictionary",
			PACKAGE_VERSION);
	}

	asprintf (&process_uuid_xl, "%s-%s", this->ctx->process_uuid,
		  this->name);
	ret = dict_set_dynstr (options, "process-uuid",
			       process_uuid_xl);
	if (ret < 0) {
		gf_log (this->name, GF_LOG_ERROR,
			"failed to set process-uuid(%s) in options dictionary",
			PACKAGE_VERSION);
	}

	dict_len = dict_serialized_length (options);
	if (dict_len < 0) {
		gf_log (this->name, GF_LOG_ERROR,
			"failed to get serialized length of dict(%p)",
			options);
		ret = dict_len;
		goto fail;
	}

	hdrlen = gf_hdr_len (req, dict_len);
	hdr    = gf_hdr_new (req, dict_len);
	GF_VALIDATE_OR_GOTO(this->name, hdr, fail);

	req    = gf_param (hdr);

	ret = dict_serialize (options, req->buf);
	if (ret < 0) {
		gf_log (this->name, GF_LOG_ERROR,
			"failed to serialize dictionary(%p)",
			options);
		goto fail;
	}

	req->dict_len = hton32 (dict_len);
	fr  = create_frame (this, this->ctx->pool);
	GF_VALIDATE_OR_GOTO(this->name, fr, fail);

	fr->local = trans;
	ret = protocol_client_xfer (fr, this, trans,
				    GF_OP_TYPE_MOP_REQUEST, GF_MOP_SETVOLUME,
				    hdr, hdrlen, NULL, 0, NULL);
	return ret;
fail:
	if (hdr)
		free (hdr);
	return ret;
}


int
protocol_client_pollout (xlator_t *this, transport_t *trans)
{
	client_connection_t *conn = NULL;

	conn = trans->xl_private;

	pthread_mutex_lock (&conn->lock);
	{
		gettimeofday (&conn->last_sent, NULL);
	}
	pthread_mutex_unlock (&conn->lock);

	return 0;
}


int
protocol_client_pollin (xlator_t *this, transport_t *trans)
{
	client_connection_t *conn = NULL;
	int ret = -1;
	char *buf = NULL;
	size_t buflen = 0;
	char *hdr = NULL;
	size_t hdrlen = 0;
	int connected = 0;

	conn = trans->xl_private;

	pthread_mutex_lock (&conn->lock);
	{
		gettimeofday (&conn->last_received, NULL);
		connected = conn->connected;
	}
	pthread_mutex_unlock (&conn->lock);

	ret = transport_receive (trans, &hdr, &hdrlen, &buf, &buflen);

	if (ret == 0)
	{
		ret = protocol_client_interpret (this, trans, hdr, hdrlen,
						 buf, buflen);
	}

	/* TODO: use mem-pool */
	FREE (hdr);

	return ret;
}


/*
 * client_protocol_notify - notify function for client protocol
 * @this:
 * @trans: transport object
 * @event
 *
 */

int32_t
notify (xlator_t *this,
        int32_t event,
        void *data,
        ...)
{
	int ret = -1;
	transport_t *trans = NULL;
	client_connection_t *conn = NULL;

	trans = data;

	switch (event) {
	case GF_EVENT_POLLOUT:
	{
		ret = protocol_client_pollout (this, trans);

		break;
	}
	case GF_EVENT_POLLIN:
	{
		ret = protocol_client_pollin (this, trans);

		break;
	}
	/* no break for ret check to happen below */
	case GF_EVENT_POLLERR:
	{
		ret = -1;
		protocol_client_cleanup (trans);
	}

	conn = trans->xl_private;
	if (conn->connected) {
		xlator_list_t *parent = NULL;

		gf_log (this->name, GF_LOG_INFO, "disconnected");

		parent = this->parents;
		while (parent) {
			parent->xlator->notify (parent->xlator,
						GF_EVENT_CHILD_DOWN,
						this);
			parent = parent->next;
		}

		conn->connected = 0;
		if (conn->reconnect == 0)
			client_protocol_reconnect (trans);
	}
	break;

	case GF_EVENT_PARENT_UP:
	{
		xlator_list_t *parent = NULL;
		client_conf_t *conf = NULL;
		int            i = 0;
		transport_t   *trans = NULL;

		conf = this->private;
		for (i = 0; i < CHANNEL_MAX; i++) {
			trans = conf->transport[i];
			if (!trans) {
				gf_log (this->name, GF_LOG_DEBUG,
					"transport init failed");
				return -1;
			}

			conn = trans->xl_private;

			gf_log (this->name, GF_LOG_DEBUG,
				"got GF_EVENT_PARENT_UP, attempting connect "
				"on transport");

			client_protocol_reconnect (trans);
		}

		/* Let the connection/re-connection happen in 
		 * background, for now, don't hang here,
		 * tell the parents that i am all ok..
		 */
		parent = trans->xl->parents;
		while (parent) {
			parent->xlator->notify (parent->xlator,
						GF_EVENT_CHILD_CONNECTING,
						trans->xl);
			parent = parent->next;
		}
	}
	break;

	case GF_EVENT_CHILD_UP:
	{
		char *handshake = NULL;

		ret = dict_get_str (this->options, "disable-handshake", 
				    &handshake);
		gf_log (this->name, GF_LOG_DEBUG, 
			"got GF_EVENT_CHILD_UP");
		if ((ret < 0) ||
		    (strcasecmp (handshake, "on"))) {
			ret = protocol_client_handshake (this, trans);
		} else {
			conn = trans->xl_private;
			conn->connected = 1;
			ret = default_notify (this, event, trans);
		}

		if (ret)
			transport_disconnect (trans);

	}
	break;

	default:
		gf_log (this->name, GF_LOG_DEBUG,
			"got %d, calling default_notify ()", event);

		default_notify (this, event, data);
		break;
	}

	return ret;
}


struct xlator_fops fops = {
	.stat        = client_stat,
	.readlink    = client_readlink,
	.mknod       = client_mknod,
	.mkdir       = client_mkdir,
	.unlink      = client_unlink,
	.rmdir       = client_rmdir,
	.symlink     = client_symlink,
	.rename      = client_rename,
	.link        = client_link,
	.chmod       = client_chmod,
	.chown       = client_chown,
	.truncate    = client_truncate,
	.utimens     = client_utimens,
	.open        = client_open,
	.readv       = client_readv,
	.writev      = client_writev,
	.statfs      = client_statfs,
	.flush       = client_flush,
	.fsync       = client_fsync,
	.setxattr    = client_setxattr,
	.getxattr    = client_getxattr,
	.removexattr = client_removexattr,
	.opendir     = client_opendir,
	.readdir     = client_readdir,
	.fsyncdir    = client_fsyncdir,
	.access      = client_access,
	.ftruncate   = client_ftruncate,
	.fstat       = client_fstat,
	.create      = client_create,
	.lk          = client_lk,
	.inodelk     = client_inodelk,
	.finodelk    = client_finodelk,
	.entrylk     = client_entrylk,
	.fentrylk    = client_fentrylk,
	.lookup      = client_lookup,
	.fchmod      = client_fchmod,
	.fchown      = client_fchown,
	.setdents    = client_setdents,
	.getdents    = client_getdents,
	.checksum    = client_checksum,
	.xattrop     = client_xattrop,
	.fxattrop    = client_fxattrop,
};

struct xlator_mops mops = {
	.stats     = client_stats,
	.getspec   = client_getspec,
};

struct xlator_cbks cbks = {
	.forget     = client_forget,
	.release    = client_release,
	.releasedir = client_releasedir
};


struct volume_options options[] = {
 	{ .key   = {"username"}, 
	  .type  = GF_OPTION_TYPE_ANY 
	},
 	{ .key   = {"password"}, 
	  .type  = GF_OPTION_TYPE_ANY 
	},
 	{ .key   = {"transport-type"}, 
	  .value = {"tcp", "socket", "ib-verbs", "unix", "ib-sdp", 
		    "tcp/client", "ib-verbs/client"},
	  .type  = GF_OPTION_TYPE_STR 
	},
 	{ .key   = {"remote-host"}, 
	  .type  = GF_OPTION_TYPE_ANY 
	},
 	{ .key   = {"remote-subvolume"}, 
	  .type  = GF_OPTION_TYPE_ANY 
	},
 	{ .key   = {"transport-timeout"}, 
	  .type  = GF_OPTION_TYPE_TIME, 
	  .min   = 5, 
	  .max   = 1013, 
	}, 
	{ .key   = {"ping-timeout"},
	  .type  = GF_OPTION_TYPE_TIME,
	  .min   = 5,
	  .max   = 1013,
	},
	{ .key   = {NULL} },
};
