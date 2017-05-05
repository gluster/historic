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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <errno.h>
#include <libgen.h>
#include <stddef.h>

#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <xlator.h>
#include <timer.h>
#include "defaults.h"
#include <time.h>
#include <poll.h>
#include "transport.h"
#include "event.h"
#include "libglusterfsclient.h"
#include "libglusterfsclient-internals.h"
#include "compat.h"
#include "compat-errno.h"

#define XLATOR_NAME "libglusterfsclient"
#define LIBGLUSTERFS_INODE_TABLE_LRU_LIMIT 14057

typedef struct {
        pthread_cond_t init_con_established;
        pthread_mutex_t lock;
        char complete;
}libglusterfs_client_private_t;

typedef struct {
        pthread_mutex_t lock;
        uint32_t previous_lookup_time;
        uint32_t previous_stat_time;
        struct stat stbuf;
} libglusterfs_client_inode_ctx_t;

typedef struct {
        pthread_mutex_t lock;
        off_t offset;
        libglusterfs_client_ctx_t *ctx;
} libglusterfs_client_fd_ctx_t;

typedef struct libglusterfs_client_async_local {
        void *cbk_data;
        union {
                struct {
                        fd_t *fd;
                        glusterfs_readv_cbk_t cbk;
                }readv_cbk;
    
                struct {
                        fd_t *fd;
                        glusterfs_writev_cbk_t cbk;
                }writev_cbk;

                struct {
                        fd_t *fd;
                }close_cbk;

                struct {
                        void *buf;
                        size_t size;
                        loc_t *loc;
                        char is_revalidate;
                        glusterfs_lookup_cbk_t cbk;
                }lookup_cbk;
        }fop;
}libglusterfs_client_async_local_t;

static inline xlator_t *
libglusterfs_graph (xlator_t *graph);

static int first_init = 1;
static int first_fini = 1;


char *
zr_build_process_uuid ()
{
	char           tmp_str[1024] = {0,};
	char           hostname[256] = {0,};
	struct timeval tv = {0,};
	struct tm      now = {0, };
	char           now_str[32];

	if (-1 == gettimeofday(&tv, NULL)) {
		gf_log ("", GF_LOG_ERROR, 
			"gettimeofday: failed %s",
			strerror (errno));		
	}

	if (-1 == gethostname (hostname, 256)) {
		gf_log ("", GF_LOG_ERROR, 
			"gethostname: failed %s",
			strerror (errno));
	}

	localtime_r (&tv.tv_sec, &now);
	strftime (now_str, 32, "%Y/%m/%d-%H:%M:%S", &now);
	snprintf (tmp_str, 1024, "%s-%d-%s:%ld", 
		  hostname, getpid(), now_str, tv.tv_usec);
	
	return strdup (tmp_str);
}


int32_t
libgf_client_forget (xlator_t *this,
		     inode_t *inode)
{
	libglusterfs_client_inode_ctx_t *ctx = NULL;
	
	inode_ctx_del (inode, this, (uint64_t *)ctx);
	FREE (ctx);

        return 0;
}


int32_t
libgf_client_release (xlator_t *this,
		      fd_t *fd)
{
	libglusterfs_client_fd_ctx_t *fd_ctx = NULL;
	data_t *fd_ctx_data = NULL;

        fd_ctx_data = dict_get (fd->ctx, XLATOR_NAME);

        fd_ctx = data_to_ptr (fd_ctx_data);
	pthread_mutex_destroy (&fd_ctx->lock);

	return 0;
}


int32_t
libgf_client_releasedir (xlator_t *this,
			 fd_t *fd)
{
	libglusterfs_client_fd_ctx_t *fd_ctx = NULL;
	data_t *fd_ctx_data = NULL;

        fd_ctx_data = dict_get (fd->ctx, XLATOR_NAME);

        fd_ctx = data_to_ptr (fd_ctx_data);
	pthread_mutex_destroy (&fd_ctx->lock);

	return 0;
}


void *poll_proc (void *ptr)
{
        glusterfs_ctx_t *ctx = ptr;

        event_dispatch (ctx->event_pool);

        return NULL;
}


int32_t
xlator_graph_init (xlator_t *xl)
{
        xlator_t *trav = xl;
        int32_t ret = -1;

        while (trav->prev)
                trav = trav->prev;

        while (trav) {
                if (!trav->ready) {
                        ret = xlator_tree_init (trav);
                        if (ret < 0)
                                break;
                }
                trav = trav->next;
        }

        return ret;
}


void
xlator_graph_fini (xlator_t *xl)
{
	xlator_t *trav = xl;
	while (trav->prev)
		trav = trav->prev;

	while (trav) {
		if (!trav->init_succeeded) {
			break;
		}

		xlator_tree_fini (trav);
		trav = trav->next;
	}
}


static void 
libgf_client_loc_wipe (loc_t *loc)
{
	if (loc->path) {
		FREE (loc->path);
	}

	if (loc->parent) { 
		inode_unref (loc->parent);
		loc->parent = NULL;
	}

	if (loc->inode) {
		inode_unref (loc->inode);
		loc->inode = NULL;
	}
}


static int32_t
libgf_client_loc_fill (loc_t *loc, const char *path, 
		       ino_t ino, libglusterfs_client_ctx_t *ctx)
{
	int32_t op_ret = -1;
	int32_t ret = 0;
	char *dentry_path = NULL;

	loc->inode = NULL;
	/* directory structure is flat. All files are immediate children of root */
	if (path) {
		/* libglusterfsclient accepts only absolute paths */
		if (path[0] != '/') {
			asprintf ((char **) &loc->path, "/%s", path);
		} else {
			loc->path = strdup (path);
		}

		loc->inode = inode_search (ctx->itable, 1, path);
	} else {
		loc->inode = inode_search (ctx->itable, ino, NULL);
		if (loc->inode == NULL) {
			gf_log ("libglusterfsclient", GF_LOG_ERROR,
				"cannot find inode for ino %"PRId64,
				ino);
			goto out;
		}

		ret = inode_path (loc->inode, NULL, &dentry_path);
		if (ret <= 0) {
			gf_log ("libglusterfsclient", GF_LOG_ERROR,
				"inode_path failed for %"PRId64,
				loc->inode->ino);
			inode_unref (loc->inode);
			op_ret = ret;
			goto out;
		} else {
			loc->path = dentry_path;
		}
	}
         
	loc->name = strrchr (loc->path, '/');
	if (loc->name) {
		loc->name++;
	}

        loc->parent = inode_ref (ctx->itable->root);

        if (loc->inode) {
                loc->ino = loc->inode->ino;
        }
	
	op_ret = 0;
out:
	return op_ret;
}


static call_frame_t *
get_call_frame_for_req (libglusterfs_client_ctx_t *ctx, char d)
{
        call_pool_t  *pool = ctx->gf_ctx.pool;
        xlator_t     *this = ctx->gf_ctx.graph;
        call_frame_t *frame = NULL;
  

        frame = create_frame (this, pool);

        frame->root->uid = geteuid ();
        frame->root->gid = getegid ();
        frame->root->pid = getpid ();
        frame->root->unique = ctx->counter++;
  
        if (d) {
                frame->root->req_refs = dict_ref (get_new_dict ());
                /*
                  TODO
                  dict_set (frame->root->req_refs, NULL, priv->buf);
                */
        }

        return frame;
}

void 
libgf_client_fini (xlator_t *this)
{
	FREE (this->private);
        return;
}


int32_t
libgf_client_notify (xlator_t *this, 
                     int32_t event,
                     void *data, 
                     ...)
{
        libglusterfs_client_private_t *priv = this->private;

        switch (event)
        {
        case GF_EVENT_CHILD_UP:
                pthread_mutex_lock (&priv->lock);
                {
                        priv->complete = 1;
                        pthread_cond_broadcast (&priv->init_con_established);
                }
                pthread_mutex_unlock (&priv->lock);
                break;

        default:
                default_notify (this, event, data);
        }

        return 0;
}

int32_t 
libgf_client_init (xlator_t *this)
{
        return 0;
}


libglusterfs_handle_t 
glusterfs_init (glusterfs_init_ctx_t *init_ctx)
{
        libglusterfs_client_ctx_t *ctx = NULL;
        libglusterfs_client_private_t *priv = NULL;
        FILE *specfp = NULL;
        xlator_t *graph = NULL, *trav = NULL;
        call_pool_t *pool = NULL;
        int32_t ret = 0;
        struct rlimit lim;
	uint32_t xl_count = 0;

        if (!init_ctx || (!init_ctx->specfile && !init_ctx->specfp)) {
                errno = EINVAL;
                return NULL;
        }

        ctx = CALLOC (1, sizeof (*ctx));
        if (!ctx) {
		fprintf (stderr, 
			 "libglusterfsclient: %s:%s():%d: out of memory\n",
			 __FILE__, __PRETTY_FUNCTION__, __LINE__);

                errno = ENOMEM;
                return NULL;
        }

        ctx->lookup_timeout = init_ctx->lookup_timeout;
        ctx->stat_timeout = init_ctx->stat_timeout;

        pthread_mutex_init (&ctx->gf_ctx.lock, NULL);
  
        pool = ctx->gf_ctx.pool = CALLOC (1, sizeof (call_pool_t));
        if (!pool) {
                errno = ENOMEM;
                FREE (ctx);
                return NULL;
        }

        LOCK_INIT (&pool->lock);
        INIT_LIST_HEAD (&pool->all_frames);

        ctx->gf_ctx.event_pool = event_pool_new (16384);

        lim.rlim_cur = RLIM_INFINITY;
        lim.rlim_max = RLIM_INFINITY;
        setrlimit (RLIMIT_CORE, &lim);
        setrlimit (RLIMIT_NOFILE, &lim);  

        ctx->gf_ctx.cmd_args.log_level = GF_LOG_WARNING;

        if (init_ctx->logfile)
                ctx->gf_ctx.cmd_args.log_file = strdup (init_ctx->logfile);
        else
                ctx->gf_ctx.cmd_args.log_file = strdup ("/dev/stderr");

        if (init_ctx->loglevel) {
                if (!strncasecmp (init_ctx->loglevel, "DEBUG", strlen ("DEBUG"))) {
                        ctx->gf_ctx.cmd_args.log_level = GF_LOG_DEBUG;
                } else if (!strncasecmp (init_ctx->loglevel, "WARNING", strlen ("WARNING"))) {
                        ctx->gf_ctx.cmd_args.log_level = GF_LOG_WARNING;
                } else if (!strncasecmp (init_ctx->loglevel, "CRITICAL", strlen ("CRITICAL"))) {
                        ctx->gf_ctx.cmd_args.log_level = GF_LOG_CRITICAL;
                } else if (!strncasecmp (init_ctx->loglevel, "NONE", strlen ("NONE"))) {
                        ctx->gf_ctx.cmd_args.log_level = GF_LOG_NONE;
                } else if (!strncasecmp (init_ctx->loglevel, "ERROR", strlen ("ERROR"))) {
                        ctx->gf_ctx.cmd_args.log_level = GF_LOG_ERROR;
                } else {
			fprintf (stderr, 
				 "libglusterfsclient: %s:%s():%d: Unrecognized log-level \"%s\", possible values are \"DEBUG|WARNING|[ERROR]|CRITICAL|NONE\"\n", __FILE__, __PRETTY_FUNCTION__, 
				 __LINE__, init_ctx->loglevel);
			FREE (ctx->gf_ctx.cmd_args.log_file);
                        FREE (ctx->gf_ctx.pool);
                        FREE (ctx->gf_ctx.event_pool);
                        FREE (ctx);
                        errno = EINVAL;
                        return NULL;
                }
        }

	if (first_init)
        {
                ret = gf_log_init (ctx->gf_ctx.cmd_args.log_file);
                if (ret == -1) {
			fprintf (stderr, 
				 "libglusterfsclient: %s:%s():%d: failed to open logfile \"%s\"\n", 
				 __FILE__, __PRETTY_FUNCTION__, __LINE__, 
				 ctx->gf_ctx.cmd_args.log_file);
			FREE (ctx->gf_ctx.cmd_args.log_file);
                        FREE (ctx->gf_ctx.pool);
                        FREE (ctx->gf_ctx.event_pool);
                        FREE (ctx);
                        return NULL;
                }

                gf_log_set_loglevel (ctx->gf_ctx.cmd_args.log_level);
        }

        if (init_ctx->specfp) {
                specfp = init_ctx->specfp;
                if (fseek (specfp, 0L, SEEK_SET)) {
			fprintf (stderr, 
				 "libglusterfsclient: %s:%s():%d: fseek on volume file stream failed (%s)\n", __FILE__, __PRETTY_FUNCTION__, __LINE__, strerror (errno));
			FREE (ctx->gf_ctx.cmd_args.log_file);
                        FREE (ctx->gf_ctx.pool);
                        FREE (ctx->gf_ctx.event_pool);
                        FREE (ctx);
                        return NULL;
                }
        } else if (init_ctx->specfile) { 
                specfp = fopen (init_ctx->specfile, "r");
                ctx->gf_ctx.cmd_args.volume_file = strdup (init_ctx->specfile);
        }

        if (!specfp) {
		fprintf (stderr, 
			 "libglusterfsclient: %s:%s():%d: could not open volfile: %s\n", 
			 __FILE__, __PRETTY_FUNCTION__, __LINE__, strerror (errno));
		FREE (ctx->gf_ctx.cmd_args.log_file);
                FREE (ctx->gf_ctx.cmd_args.volume_file);
                FREE (ctx->gf_ctx.pool);
                FREE (ctx->gf_ctx.event_pool);
                FREE (ctx);
                return NULL;
        }

        if (init_ctx->volume_name) {
                ctx->gf_ctx.cmd_args.volume_name = strdup (init_ctx->volume_name);
        }

	graph = file_to_xlator_tree (&ctx->gf_ctx, specfp);
        if (!graph) {
		fprintf (stderr, 
			 "libglusterfsclient: %s:%s():%d: cannot create configuration graph (%s)\n",
			 __FILE__, __PRETTY_FUNCTION__, __LINE__, strerror (errno));

		FREE (ctx->gf_ctx.cmd_args.log_file);
                FREE (ctx->gf_ctx.cmd_args.volume_file);
                FREE (ctx->gf_ctx.cmd_args.volume_name);
                FREE (ctx->gf_ctx.pool);
                FREE (ctx->gf_ctx.event_pool);
                FREE (ctx);
                return NULL;
        }

        if (init_ctx->volume_name) {
                trav = graph;
                while (trav) {
                        if (strcmp (trav->name, init_ctx->volume_name) == 0) {
                                graph = trav;
                                break;
                        }
                        trav = trav->next;
                }
        }

        ctx->gf_ctx.graph = libglusterfs_graph (graph);
        if (!ctx->gf_ctx.graph) {
		fprintf (stderr, 
			 "libglusterfsclient: %s:%s():%d: graph creation failed (%s)\n",
			 __FILE__, __PRETTY_FUNCTION__, __LINE__, strerror (errno));

		xlator_tree_free (graph);
		FREE (ctx->gf_ctx.cmd_args.log_file);
                FREE (ctx->gf_ctx.cmd_args.volume_file);
                FREE (ctx->gf_ctx.cmd_args.volume_name);
                FREE (ctx->gf_ctx.pool);
                FREE (ctx->gf_ctx.event_pool);
                FREE (ctx);
                return NULL;
        }
        graph = ctx->gf_ctx.graph;

	trav = graph;
	while (trav) {
		xl_count++;  /* Getting this value right is very important */
		trav = trav->next;
	}

	ctx->gf_ctx.xl_count = xl_count + 1;

        priv = CALLOC (1, sizeof (*priv));
        if (!priv) {
		fprintf (stderr, 
			 "libglusterfsclient: %s:%s():%d: cannot allocate memory (%s)\n",
			 __FILE__, __PRETTY_FUNCTION__, __LINE__, strerror (errno));

		xlator_tree_free (graph);
		FREE (ctx->gf_ctx.cmd_args.log_file);
                FREE (ctx->gf_ctx.cmd_args.volume_file);
                FREE (ctx->gf_ctx.cmd_args.volume_name);
                FREE (ctx->gf_ctx.pool);
                FREE (ctx->gf_ctx.event_pool);
                /* inode_table_destroy (ctx->itable); */
                FREE (ctx);
         
                return NULL;
        }

        pthread_cond_init (&priv->init_con_established, NULL);
        pthread_mutex_init (&priv->lock, NULL);

        graph->private = priv;
        ctx->itable = inode_table_new (LIBGLUSTERFS_INODE_TABLE_LRU_LIMIT, graph);
        if (!ctx->itable) {
		fprintf (stderr, 
			 "libglusterfsclient: %s:%s():%d: cannot create inode table\n",
			 __FILE__, __PRETTY_FUNCTION__, __LINE__);
		xlator_tree_free (graph); 
		FREE (ctx->gf_ctx.cmd_args.log_file);
                FREE (ctx->gf_ctx.cmd_args.volume_file);
                FREE (ctx->gf_ctx.cmd_args.volume_name);

                FREE (ctx->gf_ctx.pool);
                FREE (ctx->gf_ctx.event_pool);
		xlator_tree_free (graph); 
                /* TODO: destroy graph */
                /* inode_table_destroy (ctx->itable); */
                FREE (ctx);
         
                return NULL;
        }

        if (xlator_graph_init (graph) == -1) {
		fprintf (stderr, 
			 "libglusterfsclient: %s:%s():%d: graph initialization failed\n",
			 __FILE__, __PRETTY_FUNCTION__, __LINE__);
		xlator_tree_free (graph);
		FREE (ctx->gf_ctx.cmd_args.log_file);
                FREE (ctx->gf_ctx.cmd_args.volume_file);
                FREE (ctx->gf_ctx.cmd_args.volume_name);
                FREE (ctx->gf_ctx.pool);
                FREE (ctx->gf_ctx.event_pool);
                /* TODO: destroy graph */
                /* inode_table_destroy (ctx->itable); */
                FREE (ctx);
                return NULL;
        }

	/* Send notify to all translator saying things are ready */
	graph->notify (graph, GF_EVENT_PARENT_UP, graph);

        if (gf_timer_registry_init (&ctx->gf_ctx) == NULL) {
		fprintf (stderr, 
			 "libglusterfsclient: %s:%s():%d: timer init failed (%s)\n", 
			 __FILE__, __PRETTY_FUNCTION__, __LINE__, strerror (errno));

		xlator_graph_fini (graph);
		xlator_tree_free (graph);
		FREE (ctx->gf_ctx.cmd_args.log_file);
                FREE (ctx->gf_ctx.cmd_args.volume_file);
                FREE (ctx->gf_ctx.cmd_args.volume_name);

                FREE (ctx->gf_ctx.pool);
                FREE (ctx->gf_ctx.event_pool);
                /* TODO: destroy graph */
                /* inode_table_destroy (ctx->itable); */
                FREE (ctx);
                return NULL;
        }

        if ((ret = pthread_create (&ctx->reply_thread, NULL, poll_proc, (void *)&ctx->gf_ctx))) {
		fprintf (stderr, 
			 "libglusterfsclient: %s:%s():%d: reply thread creation failed\n", 
			 __FILE__, __PRETTY_FUNCTION__, __LINE__);
		xlator_graph_fini (graph);
		xlator_tree_free (graph);
		FREE (ctx->gf_ctx.cmd_args.log_file);
                FREE (ctx->gf_ctx.cmd_args.volume_file);
                FREE (ctx->gf_ctx.cmd_args.volume_name);

                FREE (ctx->gf_ctx.pool);
                FREE (ctx->gf_ctx.event_pool);
                /* TODO: destroy graph */
                /* inode_table_destroy (ctx->itable); */
                FREE (ctx);
                return NULL;
        }

	set_global_ctx_ptr (&ctx->gf_ctx);
	ctx->gf_ctx.process_uuid = zr_build_process_uuid ();

        pthread_mutex_lock (&priv->lock); 
        {
                while (!priv->complete) {
                        pthread_cond_wait (&priv->init_con_established, &priv->lock);
                }
        }
        pthread_mutex_unlock (&priv->lock);

	first_init = 0;
 
        return ctx;
}


void
glusterfs_reset (void)
{
	first_fini = first_init = 1;
}


void 
glusterfs_log_lock (void)
{
	gf_log_lock ();
}


void glusterfs_log_unlock (void)
{
	gf_log_unlock ();
}


int 
glusterfs_fini (libglusterfs_client_ctx_t *ctx)
{
	FREE (ctx->gf_ctx.cmd_args.log_file);
	FREE (ctx->gf_ctx.cmd_args.volume_file);
	FREE (ctx->gf_ctx.cmd_args.volume_name);
	FREE (ctx->gf_ctx.pool);
        FREE (ctx->gf_ctx.event_pool);
        ((gf_timer_registry_t *)ctx->gf_ctx.timer)->fin = 1;
        /* inode_table_destroy (ctx->itable); */

	xlator_graph_fini (ctx->gf_ctx.graph);
	xlator_tree_free (ctx->gf_ctx.graph);
	ctx->gf_ctx.graph = NULL;

        /* FREE (ctx->gf_ctx.specfile); */

        /* TODO complete cleanup of timer */
        /*TODO 
         * destroy the reply thread 
         * destroy inode table
         * FREE (ctx) 
         */

        FREE (ctx);

	if (first_fini) {
		;
		//gf_log_cleanup ();
	}

        return 0;
}


int32_t 
libgf_client_lookup_cbk (call_frame_t *frame,
                         void *cookie,
                         xlator_t *this,
                         int32_t op_ret,
                         int32_t op_errno,
                         inode_t *inode,
                         struct stat *buf,
                         dict_t *dict)
{
        libgf_client_local_t *local = frame->local;
        libglusterfs_client_ctx_t *ctx = frame->root->state;
	dict_t *xattr_req = NULL;

        if (op_ret == 0) {
                /* flat directory structure */
                inode_t *parent = inode_search (ctx->itable, 1, NULL);

                inode_link (inode, parent, local->fop.lookup.loc->path, buf);
		inode_lookup (inode);
		inode_unref (parent);
        } else {
                if (local->fop.lookup.is_revalidate == 0 && op_errno == ENOENT) {
                        gf_log ("libglusterfsclient", GF_LOG_DEBUG,
                                "%"PRId64": (op_num=%d) %s => -1 (%s)",
				frame->root->unique, frame->root->op,
				local->fop.lookup.loc->path,
				strerror (op_errno));
                } else {
                        gf_log ("libglusterfsclient", GF_LOG_ERROR,
                                "%"PRId64": (op_num=%d) %s => -1 (%s)",
				frame->root->unique, frame->root->op,
				local->fop.lookup.loc->path,
				strerror (op_errno));
                }

                if (local->fop.lookup.is_revalidate == 1) {
			int32_t ret = 0;
                        inode_unref (local->fop.lookup.loc->inode);
                        local->fop.lookup.loc->inode = inode_new (ctx->itable);
                        local->fop.lookup.is_revalidate = 2;

                        if (local->fop.lookup.size > 0) {
                                xattr_req = dict_new ();
                                ret = dict_set (xattr_req, "glusterfs.content",
                                                data_from_uint64 (local->fop.lookup.size));
                                if (ret == -1) {
                                        op_ret = -1;
                                        /* TODO: set proper error code */
                                        op_errno = errno;
                                        inode = NULL;
                                        buf = NULL;
                                        dict = NULL;
                                        dict_unref (xattr_req);
                                        goto out;
                                }
                        }

                        STACK_WIND (frame, libgf_client_lookup_cbk,
                                    FIRST_CHILD (this), FIRST_CHILD (this)->fops->lookup,
                                    local->fop.lookup.loc, xattr_req);

			if (xattr_req) {
				dict_unref (xattr_req);
				xattr_req = NULL;
			}

                        return 0;
                }
        }

out:
        local->reply_stub = fop_lookup_cbk_stub (frame, NULL, op_ret, op_errno, inode, buf, dict);

        pthread_mutex_lock (&local->lock);
        {
                local->complete = 1;
                pthread_cond_broadcast (&local->reply_cond);
        }
        pthread_mutex_unlock (&local->lock);

        return 0;
}

int32_t
libgf_client_lookup (libglusterfs_client_ctx_t *ctx,
                     loc_t *loc,
                     struct stat *stbuf,
                     dict_t **dict,
		     dict_t *xattr_req)
{
        call_stub_t  *stub = NULL;
        int32_t op_ret;
        libgf_client_local_t *local = NULL;
	xlator_t *this = NULL;
	int32_t ret = -1;
        
        local = CALLOC (1, sizeof (*local));
        if (loc->inode) {
                local->fop.lookup.is_revalidate = 1;
                loc->ino = loc->inode->ino;
        }
        else
                loc->inode = inode_new (ctx->itable);

        local->fop.lookup.loc = loc;

        LIBGF_CLIENT_FOP(ctx, stub, lookup, local, loc, xattr_req);

        op_ret = stub->args.lookup_cbk.op_ret;
        errno = stub->args.lookup_cbk.op_errno;

        if (!op_ret) {
                time_t current = 0;
                libglusterfs_client_inode_ctx_t *inode_ctx = NULL;
		inode_t *inode = stub->args.lookup_cbk.inode;
		uint64_t ptr = 0;

		this = ctx->gf_ctx.graph;
		ret = inode_ctx_get (inode, this, &ptr);
		if (ret == -1) {
			inode_ctx = CALLOC (1, sizeof (*inode_ctx));
			ERR_ABORT (inode_ctx);
			pthread_mutex_init (&inode_ctx->lock, NULL);
		} else {
			inode_ctx = (libglusterfs_client_inode_ctx_t *)(long)ptr;
		}

                current = time (NULL);

		pthread_mutex_lock (&inode_ctx->lock); 
		{
			inode_ctx->previous_lookup_time = current;
			inode_ctx->previous_stat_time = current;
			memcpy (&inode_ctx->stbuf, &stub->args.lookup_cbk.buf, 
				sizeof (inode_ctx->stbuf));
		}
		pthread_mutex_unlock (&inode_ctx->lock);

		ret = inode_ctx_get (inode, this, NULL);
		if (ret == -1) {
			inode_ctx_put (inode, this, (uint64_t)(long)inode_ctx);
		}

                if (stbuf)
                        *stbuf = stub->args.lookup_cbk.buf; 

                if (dict)
                        *dict = dict_ref (stub->args.lookup_cbk.dict);
        }

	call_stub_destroy (stub);
        return op_ret;
}

int 
glusterfs_lookup (libglusterfs_handle_t handle, 
                  const char *path, 
                  void *buf, 
                  size_t size, 
                  struct stat *stbuf)
{
        int32_t op_ret = 0;
        loc_t loc = {0, };
        libglusterfs_client_ctx_t *ctx = handle;
        dict_t *dict = NULL;
	dict_t *xattr_req = NULL;

        op_ret = libgf_client_loc_fill (&loc, path, 0, ctx);
	if (op_ret < 0) {
		gf_log ("libglusterfsclient",
			GF_LOG_ERROR,
			"libgf_client_loc_fill returned -1, returning EINVAL");
		errno = EINVAL;
		goto out;
	}

        if (size < 0)
                size = 0;

        if (size > 0) {
                xattr_req = dict_new ();
                op_ret = dict_set (xattr_req, "glusterfs.content", data_from_uint64 (size));
                if (op_ret < 0) {
                        gf_log ("libglusterfsclient",
                                GF_LOG_ERROR,
                                "setting requested content size dictionary failed");
                        goto out;
                }
        }

        op_ret = libgf_client_lookup (ctx, &loc, stbuf, &dict, xattr_req);

        if (!op_ret && size && stbuf && stbuf->st_size && dict && buf) {
                data_t *mem_data = NULL;
                void *mem = NULL;

                mem_data = dict_get (dict, "glusterfs.content");
                if (mem_data) {
                        mem = data_to_ptr (mem_data);
                }

                if (mem && stbuf->st_size <= size) {
                        memcpy (buf, mem, stbuf->st_size);
                }
        }

        if (dict) {
                dict_unref (dict);
        }

        libgf_client_loc_wipe (&loc);
out:
	if (xattr_req) {
		dict_unref (xattr_req);
	}

        return op_ret;
}

int
libgf_client_lookup_async_cbk (call_frame_t *frame,
                               void *cookie,
                               xlator_t *this,
                               int32_t op_ret,
                               int32_t op_errno,
                               inode_t *inode,
                               struct stat *buf,
                               dict_t *dict)
{
        libglusterfs_client_async_local_t *local = frame->local;
        glusterfs_lookup_cbk_t lookup_cbk = local->fop.lookup_cbk.cbk;
        libglusterfs_client_ctx_t *ctx = frame->root->state;
	dict_t *xattr_req = NULL;
	int32_t ret = 0;

        if (op_ret == 0) {
                time_t current = 0;
                data_t *inode_ctx_data = NULL;
                libglusterfs_client_inode_ctx_t *inode_ctx = NULL;

                /* flat directory structure */
                inode_t *parent = inode_search (ctx->itable, 1, NULL);

                inode_link (inode, parent, local->fop.lookup_cbk.loc->path, buf);
                
		inode_ctx_data = dict_get (inode->ctx, XLATOR_NAME);
                if (inode_ctx_data) {
                        inode_ctx = data_to_ptr (inode_ctx_data);
                }

                if (!inode_ctx) {
                        inode_ctx = CALLOC (1, sizeof (*inode_ctx));
                        pthread_mutex_init (&inode_ctx->lock, NULL);
                }

                current = time (NULL);

                pthread_mutex_lock (&inode_ctx->lock);
                {
                        inode_ctx->previous_lookup_time = current;
                        inode_ctx->previous_stat_time = current;
                        memcpy (&inode_ctx->stbuf, buf, sizeof (inode_ctx->stbuf));
                }
                pthread_mutex_unlock (&inode_ctx->lock);

		ret = inode_ctx_get (inode, this, NULL);
		if (ret == -1) {
			inode_ctx_put (inode, this, (uint64_t)(long)inode_ctx);
		}

                inode_lookup (inode);
                inode_unref (parent);
        } else {
                if (local->fop.lookup_cbk.is_revalidate == 0 && op_errno == ENOENT) {
                        gf_log ("libglusterfsclient", GF_LOG_DEBUG,
                                "%"PRId64": (op_num=%d) %s => -1 (%s)",
				frame->root->unique, frame->root->op,
				local->fop.lookup_cbk.loc->path,
				strerror (op_errno));
                } else {
                        gf_log ("libglusterfsclient", GF_LOG_ERROR,
                                "%"PRId64": (op_num=%d) %s => -1 (%s)",
				frame->root->unique, frame->root->op,
                                local->fop.lookup_cbk.loc->path,
				strerror (op_errno));
                }

                if (local->fop.lookup_cbk.is_revalidate == 1) {
			int32_t ret = 0;
                        inode_unref (local->fop.lookup_cbk.loc->inode);
                        local->fop.lookup_cbk.loc->inode = inode_new (ctx->itable);
                        local->fop.lookup_cbk.is_revalidate = 2;

                        if (local->fop.lookup_cbk.size > 0) {
                                xattr_req = dict_new ();
                                ret = dict_set (xattr_req, "glusterfs.content",
                                                data_from_uint64 (local->fop.lookup_cbk.size));
                                if (ret == -1) {
                                        op_ret = -1;
                                        /* TODO: set proper error code */
                                        op_errno = errno;
                                        inode = NULL;
                                        buf = NULL;
                                        dict = NULL;
                                        dict_unref (xattr_req);
                                        goto out;
                                }
                        }


                        STACK_WIND (frame, libgf_client_lookup_async_cbk,
                                    FIRST_CHILD (this), FIRST_CHILD (this)->fops->lookup,
                                    local->fop.lookup_cbk.loc, xattr_req);
			
			if (xattr_req) {
				dict_unref (xattr_req);
				xattr_req = NULL;
			}

                        return 0;
                }

        }

out:
        if (!op_ret && local->fop.lookup_cbk.size && dict && local->fop.lookup_cbk.buf) {
                data_t *mem_data = NULL;
                void *mem = NULL;

                mem_data = dict_get (dict, "glusterfs.content");
                if (mem_data) {
                        mem = data_to_ptr (mem_data);
                }

                if (mem && buf->st_size <= local->fop.lookup_cbk.size) {
                        memcpy (local->fop.lookup_cbk.buf, mem, buf->st_size);
                }
        }

        lookup_cbk(op_ret, op_errno, local->fop.lookup_cbk.buf, buf, local->cbk_data);

	libgf_client_loc_wipe (local->fop.lookup_cbk.loc);
        free (local->fop.lookup_cbk.loc);

        free (local);
        frame->local = NULL;
        STACK_DESTROY (frame->root);

        return 0;
}

int
glusterfs_lookup_async (libglusterfs_handle_t handle, 
                        const char *path,
                        void *buf,
                        size_t size, 
                        glusterfs_lookup_cbk_t cbk,
                        void *cbk_data)
{
        loc_t *loc = NULL;
        libglusterfs_client_ctx_t *ctx = handle;
        libglusterfs_client_async_local_t *local = NULL;
	int32_t op_ret = 0;
	dict_t *xattr_req = NULL;

        local = CALLOC (1, sizeof (*local));
        local->fop.lookup_cbk.is_revalidate = 1;

        loc = CALLOC (1, sizeof (*loc));
        op_ret = libgf_client_loc_fill (loc, path, 0, ctx);
	if (op_ret < 0) {
		gf_log ("libglusterfsclient",
			GF_LOG_ERROR,
			"libgf_client_loc_fill returned -1, returning EINVAL");
		errno = EINVAL;
		goto out;
	}

        if (!loc->inode) {
                loc->inode = inode_new (ctx->itable);
                local->fop.lookup_cbk.is_revalidate = 0;
        } 

        local->fop.lookup_cbk.cbk = cbk;
        local->fop.lookup_cbk.buf = buf;
        local->fop.lookup_cbk.size = size;
        local->fop.lookup_cbk.loc = loc;
        local->cbk_data = cbk_data;

        if (size < 0)
                size = 0;

        if (size > 0) {
                xattr_req = dict_new ();
                op_ret = dict_set (xattr_req, "glusterfs.content", data_from_uint64 (size));
                if (op_ret < 0) {
                        dict_unref (xattr_req);
                        xattr_req = NULL;
                        goto out;
                }
        }

        LIBGF_CLIENT_FOP_ASYNC (ctx,
                                local,
                                libgf_client_lookup_async_cbk,
                                lookup,
                                loc,
                                xattr_req);
	if (xattr_req) {
		dict_unref (xattr_req);
		xattr_req = NULL;
	}

out:
        return op_ret;
}

int32_t
libgf_client_getxattr_cbk (call_frame_t *frame,
                           void *cookie,
                           xlator_t *this,
                           int32_t op_ret,
                           int32_t op_errno,
                           dict_t *dict)
{

        libgf_client_local_t *local = frame->local;

        local->reply_stub = fop_getxattr_cbk_stub (frame, NULL, op_ret, op_errno, dict);

        pthread_mutex_lock (&local->lock);
        {
                local->complete = 1;
                pthread_cond_broadcast (&local->reply_cond);
        }
        pthread_mutex_unlock (&local->lock);

        return 0;
}

size_t 
libgf_client_getxattr (libglusterfs_client_ctx_t *ctx, 
                       loc_t *loc,
                       const char *name,
                       void *value,
                       size_t size)
{
        call_stub_t  *stub = NULL;
        int32_t op_ret = 0;
        libgf_client_local_t *local = NULL;

        LIBGF_CLIENT_FOP (ctx, stub, getxattr, local, loc, name);

        op_ret = stub->args.getxattr_cbk.op_ret;
        errno = stub->args.getxattr_cbk.op_errno;

        if (op_ret >= 0) {
                /*
                  gf_log ("LIBGF_CLIENT", GF_LOG_DEBUG,
                  "%"PRId64": %s => %d", frame->root->unique,
                  state->fuse_loc.loc.path, op_ret);
                */

                data_t *value_data = dict_get (stub->args.getxattr_cbk.dict, (char *)name);
    
                if (value_data) {
                        int32_t copy_len = 0;
                        op_ret = value_data->len; /* Don't return the value for '\0' */

                        copy_len = size < value_data->len ? size : value_data->len;
                        memcpy (value, value_data->data, copy_len);
                } else {
                        errno = ENODATA;
                        op_ret = -1;
                }
        }
	
	call_stub_destroy (stub);
        return op_ret;
}

ssize_t 
glusterfs_getxattr (libglusterfs_client_ctx_t *ctx, 
                    const char *path, 
                    const char *name,
                    void *value, 
                    size_t size)
{
        int32_t op_ret = 0;
        loc_t loc = {0, };
	dict_t *dict = NULL;

        op_ret = libgf_client_loc_fill (&loc, path, 0, ctx);
	if (op_ret < 0) {
		gf_log ("libglusterfsclient",
			GF_LOG_ERROR,
			"libgf_client_loc_fill returned -1, returning EINVAL");
		errno = EINVAL;
		goto out;
	}

	op_ret = libgf_client_lookup (ctx, &loc, NULL, &dict, NULL);
	if (op_ret == 0) {
		data_t *value_data = dict_get (dict, (char *)name);
			
		if (value_data) {
			int32_t copy_len = 0;
			op_ret = value_data->len; /* Don't return the value for '\0' */
				
			copy_len = size < value_data->len ? size : value_data->len;
			memcpy (value, value_data->data, copy_len);
		} else {
			errno = ENODATA;
			op_ret = -1;
		}
	}

	if (dict) {
		dict_unref (dict);
	}

        libgf_client_loc_wipe (&loc);

out:
        return op_ret;
}

static int32_t
libgf_client_open_cbk (call_frame_t *frame,
                       void *cookie,
                       xlator_t *this,
                       int32_t op_ret,
                       int32_t op_errno,
                       fd_t *fd)
{
        libgf_client_local_t *local = frame->local;

        local->reply_stub = fop_open_cbk_stub (frame, NULL, op_ret, op_errno, fd);

        pthread_mutex_lock (&local->lock);
        {
                local->complete = 1;
                pthread_cond_broadcast (&local->reply_cond);
        }
        pthread_mutex_unlock (&local->lock);

        return 0;
}


int 
libgf_client_open (libglusterfs_client_ctx_t *ctx, 
                   loc_t *loc, 
                   fd_t *fd, 
                   int flags)
{
        call_stub_t *stub = NULL;
        int32_t op_ret = 0;
        libgf_client_local_t *local = NULL;

        LIBGF_CLIENT_FOP (ctx, stub, open, local, loc, flags, fd);

        op_ret = stub->args.open_cbk.op_ret;
        errno = stub->args.open_cbk.op_errno;

	call_stub_destroy (stub);
        return op_ret;
}

static int32_t
libgf_client_create_cbk (call_frame_t *frame,
                         void *cookie,
                         xlator_t *this,
                         int32_t op_ret,
                         int32_t op_errno,
                         fd_t *fd,
                         inode_t *inode,
                         struct stat *buf)     
{
        libgf_client_local_t *local = frame->local;

        local->reply_stub = fop_create_cbk_stub (frame, NULL, op_ret, op_errno, fd, inode, buf);

        pthread_mutex_lock (&local->lock);
        {
                local->complete = 1;
                pthread_cond_broadcast (&local->reply_cond);
        }
        pthread_mutex_unlock (&local->lock);

        return 0;
}

int 
libgf_client_creat (libglusterfs_client_ctx_t *ctx,
                    loc_t *loc,
                    fd_t *fd,
                    int flags,
                    mode_t mode)
{
        call_stub_t *stub = NULL;
        int32_t op_ret = 0;
        libgf_client_local_t *local = NULL;
	xlator_t *this = NULL;

        LIBGF_CLIENT_FOP (ctx, stub, create, local, loc, flags, mode, fd);
  
        if (stub->args.create_cbk.op_ret == 0) {
                inode_t *libgf_inode = NULL;
                time_t current = 0;
		libglusterfs_client_inode_ctx_t *inode_ctx = NULL;
    
                /* flat directory structure */
                inode_t *parent = inode_search (ctx->itable, 1, NULL);
		libgf_inode = stub->args.create_cbk.inode;
                inode_link (libgf_inode, parent,
                            loc->path, &stub->args.create_cbk.buf);

                inode_lookup (libgf_inode);
                inode_unref (parent);

		inode_ctx = CALLOC (1, sizeof (*inode_ctx));
		ERR_ABORT (inode_ctx);
		pthread_mutex_init (&inode_ctx->lock, NULL);
		
                current = time (NULL);

		inode_ctx->previous_lookup_time = current;
		inode_ctx->previous_stat_time = current;
		memcpy (&inode_ctx->stbuf, &stub->args.lookup_cbk.buf, 
			sizeof (inode_ctx->stbuf));

		this = ctx->gf_ctx.graph;
		inode_ctx_put (libgf_inode, this, (uint64_t)(long)inode_ctx); 
        }

        op_ret = stub->args.create_cbk.op_ret;
        errno = stub->args.create_cbk.op_errno;

	call_stub_destroy (stub);
        return op_ret;
}

int32_t
libgf_client_opendir_cbk (call_frame_t *frame,
                          void *cookie,
                          xlator_t *this,
                          int32_t op_ret,
                          int32_t op_errno,
                          fd_t *fd)
{
        libgf_client_local_t *local = frame->local;

        local->reply_stub = fop_opendir_cbk_stub (frame, NULL, op_ret, op_errno, fd);

        pthread_mutex_lock (&local->lock);
        {
                local->complete = 1;
                pthread_cond_broadcast (&local->reply_cond);
        }
        pthread_mutex_unlock (&local->lock);

        return 0;
}

int 
libgf_client_opendir (libglusterfs_client_ctx_t *ctx,
                      loc_t *loc,
                      fd_t *fd)
{
        call_stub_t *stub = NULL;
        int32_t op_ret = 0;
        libgf_client_local_t *local = NULL;

        LIBGF_CLIENT_FOP (ctx, stub, opendir, local, loc, fd);

        op_ret = stub->args.opendir_cbk.op_ret;
        errno = stub->args.opendir_cbk.op_errno;

	call_stub_destroy (stub);
        return 0;
}

unsigned long 
glusterfs_open (libglusterfs_client_ctx_t *ctx, 
                const char *path, 
                int flags, 
                mode_t mode)
{
        loc_t loc = {0, };
        long op_ret = 0;
        fd_t *fd = NULL;
        struct stat stbuf; 
	char lookup_required = 1;
	int32_t ret = -1;
	xlator_t *this = NULL;

        if (!ctx || !path || path[0] != '/') {
                errno = EINVAL;
                return 0;
        }

	this = ctx->gf_ctx.graph;

        op_ret = libgf_client_loc_fill (&loc, path, 0, ctx);
	if (op_ret < 0) {
		gf_log ("libglusterfsclient",
			GF_LOG_ERROR,
			"libgf_client_loc_fill returned -1, returning EINVAL");
		errno = EINVAL;
		fd = NULL;
		goto out;
	}

        if (loc.inode) {
                libglusterfs_client_inode_ctx_t *inode_ctx = NULL;
                time_t current, prev;
		uint64_t ptr = 0;
		
		ret = inode_ctx_get (loc.inode, this, &ptr);
                if (ret == 0) {
			inode_ctx = (libglusterfs_client_inode_ctx_t *)(long)ptr;
			memset (&current, 0, sizeof (current));

			pthread_mutex_lock (&inode_ctx->lock);
			{
				prev = inode_ctx->previous_lookup_time;
			}
			pthread_mutex_unlock (&inode_ctx->lock);

			current = time (NULL);
			if (prev >= 0 && ctx->lookup_timeout >= (current - prev)) {
				lookup_required = 0;
			} 
                }
        }

        if (lookup_required) {
                op_ret = libgf_client_lookup (ctx, &loc, &stbuf, NULL, NULL);
                if (!op_ret && ((flags & O_CREAT) == O_CREAT) && ((flags & O_EXCL) == O_EXCL)) {
                        errno = EEXIST;
                        op_ret = -1;
                }
        }

        if (!op_ret || (op_ret == -1 && errno == ENOENT && ((flags & O_CREAT) == O_CREAT))) {
                fd = fd_create (loc.inode, 0);
                fd->flags = flags;

                if (!op_ret) {
                        if (S_ISDIR (loc.inode->st_mode)) {
                                if (((flags & O_RDONLY) == O_RDONLY) && 
				    ((flags & O_WRONLY) == 0) && 
				    ((flags & O_RDWR) == 0)) { 
                                        op_ret = libgf_client_opendir (ctx, &loc, fd);
				} else {
                                        op_ret = -1;
                                        errno = EISDIR;
                                }
                        } else {  
				op_ret = libgf_client_open (ctx, &loc, fd, flags);
			}
                } else {
			op_ret = libgf_client_creat (ctx, &loc, fd, flags, mode);
                }

                if (op_ret == -1) {
                        fd_unref (fd);
                        fd = NULL;
                } else {
                        libglusterfs_client_fd_ctx_t *fd_ctx = NULL;
			libglusterfs_client_inode_ctx_t *inode_ctx = NULL;
			data_t *ctx_data = NULL;
      
			ctx_data = dict_get (fd->ctx, XLATOR_NAME);
			if (!ctx_data) {
				fd_ctx = CALLOC (1, sizeof (*fd_ctx));
				ERR_ABORT (fd_ctx);
				pthread_mutex_init (&fd_ctx->lock, NULL);
			}

			pthread_mutex_lock (&fd_ctx->lock);
			{
				fd_ctx->ctx = ctx;
			}
			pthread_mutex_unlock (&fd_ctx->lock);

			if (!ctx_data) {
				dict_set (fd->ctx, XLATOR_NAME, data_from_dynptr (fd_ctx, sizeof (*fd_ctx)));
			}

			if ((flags & O_TRUNC) && ((flags & O_RDWR) || (flags & O_WRONLY))) {
				uint64_t ptr = 0;
				ret = inode_ctx_get (fd->inode, this, &ptr);
				if (ret == 0) {
					inode_ctx = (libglusterfs_client_inode_ctx_t *)(long)ptr;
					if (S_ISREG (inode_ctx->stbuf.st_mode)) {
						inode_ctx->stbuf.st_size = inode_ctx->stbuf.st_blocks = 0;
					}
				} else {
					gf_log ("libglusterfsclient", GF_LOG_WARNING,
						"inode_ctx is NULL for inode (%p) belonging to fd (%p)", 
						fd->inode, fd);
				}
			}
                }
        }

        libgf_client_loc_wipe (&loc);

out:
        return (long)fd;
}


unsigned long 
glusterfs_creat (libglusterfs_client_ctx_t *ctx, 
                 const char *path, 
                 mode_t mode)
{
	return glusterfs_open (ctx, path, 
			       (O_CREAT | O_WRONLY | O_TRUNC), mode);
}


int32_t
libgf_client_flush_cbk (call_frame_t *frame,
                        void *cookie,
                        xlator_t *this,
                        int32_t op_ret,
                        int32_t op_errno)
{
        libgf_client_local_t *local = frame->local;
        
        local->reply_stub = fop_flush_cbk_stub (frame, NULL, op_ret, op_errno);
        
        pthread_mutex_lock (&local->lock);
        {
                local->complete = 1;
                pthread_cond_broadcast (&local->reply_cond);
        }
        pthread_mutex_unlock (&local->lock);
        
        return 0;
}


int 
libgf_client_flush (libglusterfs_client_ctx_t *ctx, fd_t *fd)
{
        call_stub_t *stub;
        int32_t op_ret;
        libgf_client_local_t *local = NULL;

        LIBGF_CLIENT_FOP (ctx, stub, flush, local, fd);
        
        op_ret = stub->args.flush_cbk.op_ret;
        errno = stub->args.flush_cbk.op_errno;
        
	call_stub_destroy (stub);        
        return op_ret;
}


int 
glusterfs_close (unsigned long fd)
{
        int32_t op_ret = -1;
        data_t *fd_ctx_data = NULL;
        libglusterfs_client_ctx_t *ctx = NULL;
        libglusterfs_client_fd_ctx_t *fd_ctx = NULL;

        if (!fd) {
                errno = EINVAL;
		goto out;
        }

        fd_ctx_data = dict_get (((fd_t *) fd)->ctx, XLATOR_NAME);
        if (!fd_ctx_data) {
                errno = EBADF;
		goto out;
        }

        fd_ctx = data_to_ptr (fd_ctx_data);
        ctx = fd_ctx->ctx;

        op_ret = libgf_client_flush (ctx, (fd_t *)fd);

        fd_unref ((fd_t *)fd);

out:
        return op_ret;
}

int32_t
libgf_client_setxattr_cbk (call_frame_t *frame,
                           void *cookie,
                           xlator_t *this,
                           int32_t op_ret,
                           int32_t op_errno)
{
        libgf_client_local_t *local = frame->local;

        local->reply_stub = fop_setxattr_cbk_stub (frame, NULL, op_ret, op_errno);

        pthread_mutex_lock (&local->lock);
        {
                local->complete = 1;
                pthread_cond_broadcast (&local->reply_cond);
        }
        pthread_mutex_unlock (&local->lock);

        return 0;
}

int
libgf_client_setxattr (libglusterfs_client_ctx_t *ctx, 
                       loc_t *loc,
                       const char *name,
                       const void *value,
                       size_t size,
                       int flags)
{
        call_stub_t  *stub = NULL;
        int32_t op_ret = 0;
        dict_t *dict;
        libgf_client_local_t *local = NULL;

        dict = get_new_dict ();

        dict_set (dict, (char *)name,
                  bin_to_data ((void *)value, size));
        dict_ref (dict);


        LIBGF_CLIENT_FOP (ctx, stub, setxattr, local, loc, dict, flags);

        op_ret = stub->args.setxattr_cbk.op_ret;
        errno = stub->args.setxattr_cbk.op_errno;

        dict_unref (dict);
	call_stub_destroy (stub);
        return op_ret;
}

int 
glusterfs_setxattr (libglusterfs_client_ctx_t *ctx, 
                    const char *path, 
                    const char *name,
                    const void *value, 
                    size_t size, 
                    int flags)
{
        int32_t op_ret = 0;
        loc_t loc = {0, };
        char lookup_required = 1;
	xlator_t *this = NULL;

        op_ret = libgf_client_loc_fill (&loc, path, 0, ctx);
	if (op_ret < 0) {
		gf_log ("libglusterfsclient",
			GF_LOG_ERROR,
			"libgf_client_loc_fill returned -1, returning EINVAL");
		errno = EINVAL;
		goto out;
	}

	this = ctx->gf_ctx.graph;
        if (loc.inode) {
                time_t current, prev;
                libglusterfs_client_inode_ctx_t *inode_ctx = NULL;
		uint64_t ptr = 0;

		op_ret = inode_ctx_get (loc.inode, this, &ptr);
                if (op_ret == -1) {
			errno = EINVAL;
			goto out;
		}

		inode_ctx = (libglusterfs_client_inode_ctx_t *)(long)ptr;
                memset (&current, 0, sizeof (current));
                current = time (NULL);

                pthread_mutex_lock (&inode_ctx->lock);
                {
                        prev = inode_ctx->previous_lookup_time;
                }
                pthread_mutex_unlock (&inode_ctx->lock);
    
                if ((prev >= 0) && ctx->lookup_timeout >= (current - prev)) {
                        lookup_required = 0;
                } 
        }

        if (lookup_required) {
                op_ret = libgf_client_lookup (ctx, &loc, NULL, NULL, NULL);
        }

        if (!op_ret)
                op_ret = libgf_client_setxattr (ctx, &loc, name, value, size, flags);

        libgf_client_loc_wipe (&loc);

out:
        return op_ret;
}

int 
glusterfs_lsetxattr (libglusterfs_client_ctx_t *ctx, 
                     const char *path, 
                     const char *name,
                     const void *value, 
                     size_t size, int flags)
{
        return ENOSYS;
}

int 
glusterfs_fsetxattr (unsigned long fd, 
                     const char *name,
                     const void *value, 
                     size_t size, 
                     int flags)
{
	int32_t op_ret = 0;
        fd_t *__fd ;
	char lookup_required = 1;
        libglusterfs_client_ctx_t *ctx = NULL;
        libglusterfs_client_fd_ctx_t *fd_ctx = NULL;
        data_t *fd_ctx_data = NULL;
	loc_t loc = {0, };
	xlator_t *this = NULL;

	__fd = (fd_t *)fd;
        fd_ctx_data = dict_get (__fd->ctx, XLATOR_NAME);
        if (!fd_ctx_data) {
                errno = EBADF;
		op_ret = -1;
		goto out;
        }

        fd_ctx = data_to_ptr (fd_ctx_data);
        ctx = fd_ctx->ctx;

        op_ret = libgf_client_loc_fill (&loc, NULL, __fd->inode->ino, ctx);
	if (op_ret < 0) {
		gf_log ("libglusterfsclient",
			GF_LOG_ERROR,
			"libgf_client_loc_fill returned -1, returning EINVAL");
		errno = EINVAL;
		goto out;
	}

	this = ctx->gf_ctx.graph;
        if (loc.inode) { 
                time_t current, prev;
                libglusterfs_client_inode_ctx_t *inode_ctx = NULL;
		uint64_t ptr = 0;

		op_ret = inode_ctx_get (loc.inode, this, &ptr);
		if (op_ret == -1) {
			errno = EINVAL;
			goto out;
		}

		inode_ctx = (libglusterfs_client_inode_ctx_t *)(long)ptr;
                memset (&current, 0, sizeof (current));
                current = time (NULL);

                pthread_mutex_lock (&inode_ctx->lock);
                {
                        prev = inode_ctx->previous_lookup_time;
                }
                pthread_mutex_unlock (&inode_ctx->lock);
    
                if ( (prev >= 0) && ctx->lookup_timeout >= (current - prev)) {
                        lookup_required = 0;
                } 
        }

        if (lookup_required) {
                op_ret = libgf_client_lookup (ctx, &loc, NULL, NULL, NULL);
        }

        if (!op_ret)
                op_ret = libgf_client_setxattr (ctx, &loc, name, value, size, flags);

        libgf_client_loc_wipe (&loc);
out:
	return op_ret;
}

ssize_t 
glusterfs_lgetxattr (libglusterfs_client_ctx_t *ctx, 
                     const char *path, 
                     const char *name,
                     void *value, 
                     size_t size)
{
        return ENOSYS;
}

ssize_t 
glusterfs_fgetxattr (unsigned long fd, 
                     const char *name,
                     void *value, 
                     size_t size)
{
	int32_t op_ret = 0;
        libglusterfs_client_ctx_t *ctx;
        fd_t *__fd = (fd_t *)fd;
        libglusterfs_client_fd_ctx_t *fd_ctx = NULL;
        data_t *fd_ctx_data = NULL;
	loc_t loc = {0, };
	dict_t *dict = NULL;

        fd_ctx_data = dict_get (__fd->ctx, XLATOR_NAME);
        if (!fd_ctx_data) {
                errno = EBADF;
		op_ret = -1;
		goto out;
        }

        fd_ctx = data_to_ptr (fd_ctx_data);
        ctx = fd_ctx->ctx;

        op_ret = libgf_client_loc_fill (&loc, NULL, __fd->inode->ino, ctx);
	if (op_ret < 0) {
		gf_log ("libglusterfsclient",
			GF_LOG_ERROR,
			"libgf_client_loc_fill returned -1, returning EINVAL");
		errno = EINVAL;
		goto out;
	}

	op_ret = libgf_client_lookup (ctx, &loc, NULL, &dict, NULL);
	if (op_ret == 0) {
		data_t *value_data = dict_get (dict, (char *)name);
			
		if (value_data) {
			int32_t copy_len = 0;
			op_ret = value_data->len; /* Don't return the value for '\0' */
				
			copy_len = size < value_data->len ? size : value_data->len;
			memcpy (value, value_data->data, copy_len);
		} else {
			errno = ENODATA;
			op_ret = -1;
		}
	}

	if (dict) {
		dict_unref (dict);
	}

        libgf_client_loc_wipe (&loc);

out:
	return op_ret;
}

ssize_t 
glusterfs_listxattr (libglusterfs_client_ctx_t *ctx, 
                     const char *path, 
                     char *list,
                     size_t size)
{
        return ENOSYS;
}

ssize_t 
glusterfs_llistxattr (libglusterfs_client_ctx_t *ctx, 
                      const char *path, 
                      char *list,
                      size_t size)
{
        return ENOSYS;
}

ssize_t 
glusterfs_flistxattr (unsigned long fd, 
                      char *list,
                      size_t size)
{
        return ENOSYS;
}

int 
glusterfs_removexattr (libglusterfs_client_ctx_t *ctx, 
                       const char *path, 
                       const char *name)
{
        return ENOSYS;
}

int 
glusterfs_lremovexattr (libglusterfs_client_ctx_t *ctx, 
                        const char *path, 
                        const char *name)
{
        return ENOSYS;
}

int 
glusterfs_fremovexattr (unsigned long fd, 
                        const char *name)
{
        return ENOSYS;
}

int32_t
libgf_client_readv_cbk (call_frame_t *frame,
                        void *cookie,
                        xlator_t *this,
                        int32_t op_ret,
                        int32_t op_errno,
                        struct iovec *vector,
                        int32_t count,
                        struct stat *stbuf)
{
        libgf_client_local_t *local = frame->local;

        local->reply_stub = fop_readv_cbk_stub (frame, NULL, op_ret, op_errno, vector, count, stbuf);

        pthread_mutex_lock (&local->lock);
        {
                local->complete = 1;
                pthread_cond_broadcast (&local->reply_cond);
        }
        pthread_mutex_unlock (&local->lock);

        return 0;
}

int 
libgf_client_read (libglusterfs_client_ctx_t *ctx, 
                   fd_t *fd,
                   void *buf, 
                   size_t size, 
                   off_t offset)
{
        call_stub_t *stub;
        struct iovec *vector;
        int32_t op_ret = -1;
        int count = 0;
        libgf_client_local_t *local = NULL;

        LIBGF_CLIENT_FOP (ctx, stub, readv, local, fd, size, offset);

        op_ret = stub->args.readv_cbk.op_ret;
        errno = stub->args.readv_cbk.op_errno;
        count = stub->args.readv_cbk.count;
        vector = stub->args.readv_cbk.vector;
        if (op_ret > 0) {
                int i = 0;
                op_ret = 0;
                while (size && (i < count)) {
                        int len = (size < vector[i].iov_len) ? size : vector[i].iov_len;
                        memcpy (buf, vector[i++].iov_base, len);
                        buf += len;
                        size -= len;
                        op_ret += len;
                }
        }

	call_stub_destroy (stub);
        return op_ret;
}

ssize_t 
glusterfs_read (unsigned long fd, 
                void *buf, 
                size_t nbytes)
{
        int32_t op_ret = -1;
        off_t offset = 0;
        libglusterfs_client_ctx_t *ctx = NULL;
        libglusterfs_client_fd_ctx_t *fd_ctx = NULL;
        data_t *fd_ctx_data = NULL;

        if (fd == 0) {
                errno = EINVAL;
		goto out;
        }

        fd_ctx_data = dict_get (((fd_t *) fd)->ctx, XLATOR_NAME);
        if (!fd_ctx_data) {
                errno = EBADF;
		goto out;
        }

        fd_ctx = data_to_ptr (fd_ctx_data);

        pthread_mutex_lock (&fd_ctx->lock);
        {
                ctx = fd_ctx->ctx;
                offset = fd_ctx->offset;
        }
        pthread_mutex_unlock (&fd_ctx->lock);

        op_ret = libgf_client_read (ctx, (fd_t *)fd, buf, nbytes, offset);

        if (op_ret > 0) {
                offset += op_ret;
                pthread_mutex_lock (&fd_ctx->lock);
                {
                        fd_ctx->offset = offset;
                }
                pthread_mutex_unlock (&fd_ctx->lock);
        }

out:
        return op_ret;
}


ssize_t
libgf_client_readv (libglusterfs_client_ctx_t *ctx, 
                    fd_t *fd,
                    const struct iovec *dst_vector,
                    int dst_count,
                    off_t offset)
{
        call_stub_t *stub = NULL;
        struct iovec *src_vector;
        int src_count = 0;
        int32_t op_ret = -1;
        libgf_client_local_t *local = NULL;
        size_t size = 0;
        int32_t i = 0;

        for (i = 0; i < dst_count; i++)
        {
                size += dst_vector[i].iov_len;
        }

        LIBGF_CLIENT_FOP (ctx, stub, readv, local, fd, size, offset);

        op_ret = stub->args.readv_cbk.op_ret;
        errno = stub->args.readv_cbk.op_errno;
        src_count = stub->args.readv_cbk.count;
        src_vector = stub->args.readv_cbk.vector;
        if (op_ret > 0) {
                int src = 0, dst = 0;
                off_t src_offset = 0, dst_offset = 0;
                op_ret = 0;
    
                while ((size != 0) && (dst < dst_count) && (src < src_count)) {
                        int len = 0, src_len, dst_len;
   
                        src_len = src_vector[src].iov_len - src_offset;
                        dst_len = dst_vector[dst].iov_len - dst_offset;

                        len = (src_len < dst_len) ? src_len : dst_len;
                        if (len > size) {
                                len = size;
                        }

                        memcpy (dst_vector[dst].iov_base + dst_offset, 
				src_vector[src].iov_base + src_offset, len);

                        size -= len;
                        src_offset += len;
                        dst_offset += len;

                        if (src_offset == src_vector[src].iov_len) {
                                src_offset = 0;
                                src++;
                        }

                        if (dst_offset == dst_vector[dst].iov_len) {
                                dst_offset = 0;
                                dst++;
                        }
                }
        }
 
	call_stub_destroy (stub);
        return op_ret;
}


ssize_t 
glusterfs_readv (unsigned long fd, const struct iovec *vec, int count)
{
        int32_t op_ret = -1;
        off_t offset = 0;
        libglusterfs_client_ctx_t *ctx = NULL;
        libglusterfs_client_fd_ctx_t *fd_ctx = NULL;
        data_t *fd_ctx_data = NULL;

        if (!fd) {
                errno = EINVAL;
		goto out;
        }

        fd_ctx_data = dict_get (((fd_t *) fd)->ctx, XLATOR_NAME);
        if (!fd_ctx_data) {
                errno = EBADF;
		goto out;
        }

        fd_ctx = data_to_ptr (fd_ctx_data);

        pthread_mutex_lock (&fd_ctx->lock);
        {
                ctx = fd_ctx->ctx;
                offset = fd_ctx->offset;
        }
        pthread_mutex_unlock (&fd_ctx->lock);

        op_ret = libgf_client_readv (ctx, (fd_t *)fd, vec, count, offset);

        if (op_ret > 0) {
                offset += op_ret;
                pthread_mutex_lock (&fd_ctx->lock);
                {
                        fd_ctx->offset = offset;
                }
                pthread_mutex_unlock (&fd_ctx->lock);
        }

out:
        return op_ret;
}


ssize_t 
glusterfs_pread (unsigned long fd, 
                 void *buf, 
                 size_t count, 
                 off_t offset)
{
        int32_t op_ret = -1;
        libglusterfs_client_ctx_t *ctx = NULL;
        libglusterfs_client_fd_ctx_t *fd_ctx = NULL;
        data_t *fd_ctx_data = NULL;

        if (!fd) {
                errno = EINVAL;
		goto out;
        }

        fd_ctx_data = dict_get (((fd_t *) fd)->ctx, XLATOR_NAME);
        if (!fd_ctx_data) {
                errno = EBADF;
		goto out;
        }

        fd_ctx = data_to_ptr (fd_ctx_data);

        ctx = fd_ctx->ctx;

        op_ret = libgf_client_read (ctx, (fd_t *)fd, buf, count, offset);

out:
        return op_ret;
}


int
libgf_client_writev_cbk (call_frame_t *frame,
                         void *cookie,
                         xlator_t *this,
                         int32_t op_ret,
                         int32_t op_errno,
                         struct stat *stbuf)
{
        libgf_client_local_t *local = frame->local;

        local->reply_stub = fop_writev_cbk_stub (frame, NULL, op_ret, op_errno, stbuf);

        pthread_mutex_lock (&local->lock);
        {
                local->complete = 1;
                pthread_cond_broadcast (&local->reply_cond);
        }
        pthread_mutex_unlock (&local->lock);
        return 0;
}

int
libgf_client_writev (libglusterfs_client_ctx_t *ctx, 
                     fd_t *fd, 
                     struct iovec *vector, 
                     int count, 
                     off_t offset)
{
        call_stub_t *stub = NULL;
        int op_ret = -1;
        libgf_client_local_t *local = NULL;

        LIBGF_CLIENT_FOP (ctx, stub, writev, local, fd, vector, count, offset);

        op_ret = stub->args.writev_cbk.op_ret;
        errno = stub->args.writev_cbk.op_errno;

	call_stub_destroy (stub);
        return op_ret;
}


ssize_t 
glusterfs_write (unsigned long fd, 
                 const void *buf, 
                 size_t n)
{
        int32_t op_ret = -1;
        off_t offset = 0;
        struct iovec vector;
        libglusterfs_client_ctx_t *ctx = NULL;
        libglusterfs_client_fd_ctx_t *fd_ctx = NULL;
        data_t *fd_ctx_data = NULL;

        if (!fd) {
                errno = EINVAL;
		goto out;
        }

        fd_ctx_data = dict_get (((fd_t *) fd)->ctx, XLATOR_NAME);
        if (!fd_ctx_data) {
                errno = EBADF;
		goto out;
        }

        fd_ctx = data_to_ptr (fd_ctx_data);

        ctx = fd_ctx->ctx;

        pthread_mutex_lock (&fd_ctx->lock);
        {
                offset = fd_ctx->offset;
        }
        pthread_mutex_unlock (&fd_ctx->lock);

        vector.iov_base = (void *)buf;
        vector.iov_len = n;

        op_ret = libgf_client_writev (ctx,
                                      (fd_t *)fd, 
                                      &vector, 
                                      1, 
                                      offset);

        if (op_ret >= 0) {
                offset += op_ret;
                pthread_mutex_lock (&fd_ctx->lock);
                {
                        fd_ctx->offset = offset;
                }
                pthread_mutex_unlock (&fd_ctx->lock);
        }

out:
        return op_ret;
}

ssize_t 
glusterfs_writev (unsigned long fd, 
                  const struct iovec *vector,
                  size_t count)
{
        int32_t op_ret = -1;
        off_t offset = 0;
        libglusterfs_client_ctx_t *ctx = NULL;
        libglusterfs_client_fd_ctx_t *fd_ctx = NULL;
        data_t *fd_ctx_data = NULL;

        if (!fd) {
                errno = EINVAL;
		goto out;
        }


        fd_ctx_data = dict_get (((fd_t *) fd)->ctx, XLATOR_NAME);
        if (!fd_ctx_data) {
                errno = EBADF;
		goto out;
        }

        fd_ctx = data_to_ptr (fd_ctx_data);

        ctx = fd_ctx->ctx;

        pthread_mutex_lock (&fd_ctx->lock);
        {
                offset = fd_ctx->offset;
        }
        pthread_mutex_unlock (&fd_ctx->lock);


        op_ret = libgf_client_writev (ctx,
                                      (fd_t *)fd, 
                                      (struct iovec *)vector, 
                                      count,
                                      offset);

        if (op_ret >= 0) {
                offset += op_ret;
                pthread_mutex_lock (&fd_ctx->lock);
                {
                        fd_ctx->offset = offset;
                }
                pthread_mutex_unlock (&fd_ctx->lock);
        }

out:
        return op_ret;
}


ssize_t 
glusterfs_pwrite (unsigned long fd, 
                  const void *buf, 
                  size_t count, 
                  off_t offset)
{
        int32_t op_ret = -1;
        struct iovec vector;
        libglusterfs_client_ctx_t *ctx = NULL;
        libglusterfs_client_fd_ctx_t *fd_ctx = NULL;
        data_t *fd_ctx_data = NULL;

        if (!fd) {
                errno = EINVAL;
		goto out;
        }

        fd_ctx_data = dict_get (((fd_t *) fd)->ctx, XLATOR_NAME);
        if (!fd_ctx_data) {
                errno = EBADF;
		goto out;
        }

        fd_ctx = data_to_ptr (fd_ctx_data);

        ctx = fd_ctx->ctx;

        vector.iov_base = (void *)buf;
        vector.iov_len = count;

        op_ret = libgf_client_writev (ctx,
                                      (fd_t *)fd, 
                                      &vector, 
                                      1, 
                                      offset);

out:
        return op_ret;
}


int32_t
libgf_client_readdir_cbk (call_frame_t *frame,
                          void *cookie,
                          xlator_t *this,
                          int32_t op_ret,
                          int32_t op_errno,
                          gf_dirent_t *entries)
{
        libgf_client_local_t *local = frame->local;

        local->reply_stub = fop_readdir_cbk_stub (frame, NULL, op_ret, op_errno, entries);

        pthread_mutex_lock (&local->lock);
        {
                local->complete = 1;
                pthread_cond_broadcast (&local->reply_cond);
        }
        pthread_mutex_unlock (&local->lock);
        return 0;
}

int 
libgf_client_readdir (libglusterfs_client_ctx_t *ctx, 
                      fd_t *fd, 
                      struct dirent *dirp, 
                      size_t size, 
                      off_t *offset,
		      int32_t num_entries)
{  
        call_stub_t *stub = NULL;
        int op_ret = -1;
        libgf_client_local_t *local = NULL;
	gf_dirent_t *entry = NULL;
	int32_t count = 0; 
	size_t entry_size = 0;

        LIBGF_CLIENT_FOP (ctx, stub, readdir, local, fd, size, *offset);

        op_ret = stub->args.readdir_cbk.op_ret;
        errno = stub->args.readdir_cbk.op_errno;

        if (op_ret > 0) {
		list_for_each_entry (entry, &stub->args.readdir_cbk.entries.list, list) {
			entry_size = offsetof (struct dirent, d_name) + strlen (entry->d_name) + 1;
			
			if ((size < entry_size) || (count == num_entries)) {
				break;
			}

			size -= entry_size;

			dirp->d_ino = entry->d_ino;
			/*
			  #ifdef GF_DARWIN_HOST_OS
			  dirp->d_off = entry->d_seekoff;
			  #endif
			  #ifdef GF_LINUX_HOST_OS
			  dirp->d_off = entry->d_off;
			  #endif
			*/
			
			*offset = dirp->d_off = entry->d_off;
			/* dirp->d_type = entry->d_type; */
			dirp->d_reclen = entry->d_len;
			strncpy (dirp->d_name, entry->d_name, dirp->d_reclen);
			dirp->d_name[dirp->d_reclen] = '\0';

			dirp = (struct dirent *) (((char *) dirp) + entry_size);
			count++;
		}
        }

	call_stub_destroy (stub);
        return op_ret;
}

int
glusterfs_readdir (unsigned long fd, 
                   struct dirent *dirp, 
                   unsigned int count)
{
        int op_ret = -1;
        libglusterfs_client_ctx_t *ctx = NULL;
        off_t offset = 0;
        libglusterfs_client_fd_ctx_t *fd_ctx = NULL;
        data_t *fd_ctx_data = NULL;

        fd_ctx_data = dict_get (((fd_t *) fd)->ctx, XLATOR_NAME);
        if (!fd_ctx_data) {
                errno = EBADF;
		goto out;
        }

        fd_ctx = data_to_ptr (fd_ctx_data);

        pthread_mutex_lock (&fd_ctx->lock);
        {
                ctx = fd_ctx->ctx;
                offset = fd_ctx->offset;
        }
        pthread_mutex_unlock (&fd_ctx->lock);

        op_ret = libgf_client_readdir (ctx, (fd_t *)fd, dirp, sizeof (*dirp), &offset, 1);

        if (op_ret > 0) {
                pthread_mutex_lock (&fd_ctx->lock);
                {
                        fd_ctx->offset = offset;
                }
                pthread_mutex_unlock (&fd_ctx->lock);
		op_ret = 1;
        }

out:
        return op_ret;
}


int
glusterfs_getdents (unsigned long fd, struct dirent *dirp, unsigned int count)
{
        int op_ret = -1;
        libglusterfs_client_ctx_t *ctx = NULL;
        off_t offset = 0;
        libglusterfs_client_fd_ctx_t *fd_ctx = NULL;
        data_t *fd_ctx_data = NULL;

        fd_ctx_data = dict_get (((fd_t *) fd)->ctx, XLATOR_NAME);
        if (!fd_ctx_data) {
                errno = EBADF;
		goto out;
        }

        fd_ctx = data_to_ptr (fd_ctx_data);

        pthread_mutex_lock (&fd_ctx->lock);
        {
                ctx = fd_ctx->ctx;
                offset = fd_ctx->offset;
        }
        pthread_mutex_unlock (&fd_ctx->lock);

        op_ret = libgf_client_readdir (ctx, (fd_t *)fd, dirp, count, &offset, -1);

        if (op_ret > 0) {
                pthread_mutex_lock (&fd_ctx->lock);
                {
                        fd_ctx->offset = offset;
                }
                pthread_mutex_unlock (&fd_ctx->lock);
        }

out:
        return op_ret;
}


static int32_t
libglusterfs_readv_async_cbk (call_frame_t *frame,
                              void *cookie,
                              xlator_t *this,
                              int32_t op_ret,
                              int32_t op_errno,
                              struct iovec *vector,
                              int32_t count,
                              struct stat *stbuf)
{
        glusterfs_read_buf_t *buf;
        libglusterfs_client_async_local_t *local = frame->local;
        fd_t *__fd = local->fop.readv_cbk.fd;
        glusterfs_readv_cbk_t readv_cbk = local->fop.readv_cbk.cbk;

        buf = CALLOC (1, sizeof (*buf));
        ERR_ABORT (buf);

	if (vector) {
		buf->vector = iov_dup (vector, count);
	}

        buf->count = count;
        buf->op_ret = op_ret;
        buf->op_errno = op_errno;

	if (frame->root->rsp_refs) {
		buf->ref = dict_ref (frame->root->rsp_refs);
	}

        if (op_ret > 0) {
                libglusterfs_client_fd_ctx_t *fd_ctx = NULL;
                data_t *fd_ctx_data = NULL;

                fd_ctx_data = dict_get (__fd->ctx, XLATOR_NAME);

                fd_ctx = data_to_ptr (fd_ctx_data);
                pthread_mutex_lock (&fd_ctx->lock);
                {
                        fd_ctx->offset += op_ret;
                }
                pthread_mutex_unlock (&fd_ctx->lock);
        }

        readv_cbk (buf, local->cbk_data); 

	FREE (local);
	frame->local = NULL;
        STACK_DESTROY (frame->root);

        return 0;
}

void 
glusterfs_free (glusterfs_read_buf_t *buf)
{
        //iov_free (buf->vector, buf->count);
        FREE (buf->vector);
        dict_unref ((dict_t *) buf->ref);
        FREE (buf);
}

int 
glusterfs_read_async (unsigned long fd, 
                      size_t nbytes, 
                      off_t offset,
                      glusterfs_readv_cbk_t readv_cbk,
                      void *cbk_data)
{
        libglusterfs_client_ctx_t *ctx;
        fd_t *__fd = (fd_t *)fd;
        libglusterfs_client_async_local_t *local = NULL;
        libglusterfs_client_fd_ctx_t *fd_ctx = NULL;
        data_t *fd_ctx_data = NULL;
	int32_t op_ret = 0;

        local = CALLOC (1, sizeof (*local));
        ERR_ABORT (local);
        local->fop.readv_cbk.fd = __fd;
        local->fop.readv_cbk.cbk = readv_cbk;
        local->cbk_data = cbk_data;

        fd_ctx_data = dict_get (((fd_t *) fd)->ctx, XLATOR_NAME);
        if (!fd_ctx_data) {
                errno = EBADF;
		op_ret = -1;
		goto out;
        }

        fd_ctx = data_to_ptr (fd_ctx_data);
  
        ctx = fd_ctx->ctx;

        if (offset < 0) {
                pthread_mutex_lock (&fd_ctx->lock);
                {
                        offset = fd_ctx->offset;
                }
                pthread_mutex_unlock (&fd_ctx->lock);
        }

        LIBGF_CLIENT_FOP_ASYNC (ctx,
                                local,
                                libglusterfs_readv_async_cbk,
                                readv,
                                __fd,
                                nbytes,
                                offset);

out:
        return op_ret;
}

static int32_t
libglusterfs_writev_async_cbk (call_frame_t *frame,
                               void *cookie,
                               xlator_t *this,
                               int32_t op_ret,
                               int32_t op_errno,
                               struct stat *stbuf)
{
        libglusterfs_client_async_local_t *local = frame->local;
        fd_t *fd = NULL;
        glusterfs_writev_cbk_t writev_cbk;

        writev_cbk = local->fop.writev_cbk.cbk;
        fd = local->fop.writev_cbk.fd;

        if (op_ret > 0) {
                libglusterfs_client_fd_ctx_t *fd_ctx = NULL;
                data_t *fd_ctx_data = NULL;

                fd_ctx_data = dict_get (fd->ctx, XLATOR_NAME);

                fd_ctx = data_to_ptr (fd_ctx_data);

                pthread_mutex_lock (&fd_ctx->lock);
                {
                        fd_ctx->offset += op_ret;  
                }
                pthread_mutex_unlock (&fd_ctx->lock);
        }

        writev_cbk (op_ret, op_errno, local->cbk_data);

        STACK_DESTROY (frame->root);
        return 0;
}

int32_t
glusterfs_write_async (unsigned long fd, 
                       const void *buf, 
                       size_t nbytes, 
                       off_t offset,
                       glusterfs_writev_cbk_t writev_cbk,
                       void *cbk_data)
{
        fd_t *__fd = (fd_t *)fd;
        struct iovec vector;
        off_t __offset = offset;
        libglusterfs_client_ctx_t *ctx = NULL;
        libglusterfs_client_async_local_t *local = NULL;
        libglusterfs_client_fd_ctx_t *fd_ctx = NULL;
        data_t *fd_ctx_data = NULL;
	int32_t op_ret = 0;

        local = CALLOC (1, sizeof (*local));
        ERR_ABORT (local);
        local->fop.writev_cbk.fd = __fd;
        local->fop.writev_cbk.cbk = writev_cbk;
        local->cbk_data = cbk_data;

        vector.iov_base = (void *)buf;
        vector.iov_len = nbytes;
  
        fd_ctx_data = dict_get (__fd->ctx, XLATOR_NAME);
        if (!fd_ctx_data) {
                errno = EBADF;
		op_ret = -1;
		goto out;
        }

        fd_ctx = data_to_ptr (fd_ctx_data);
        ctx = fd_ctx->ctx;
 
        if (offset < 0) {
                pthread_mutex_lock (&fd_ctx->lock);
                {
                        __offset = fd_ctx->offset;
                }
                pthread_mutex_unlock (&fd_ctx->lock);
        }

        LIBGF_CLIENT_FOP_ASYNC (ctx,
                                local,
                                libglusterfs_writev_async_cbk,
                                writev,
                                __fd,
                                &vector,
                                1,
                                __offset);

out:
        return op_ret;
}

off_t
glusterfs_lseek (unsigned long fd, off_t offset, int whence)
{
        off_t __offset = 0;
	int32_t op_ret = -1;
        fd_t *__fd = (fd_t *)fd;
        data_t *fd_ctx_data = NULL;
        libglusterfs_client_fd_ctx_t *fd_ctx = NULL;
	libglusterfs_client_inode_ctx_t *inode_ctx = NULL;
	libglusterfs_client_ctx_t *ctx = NULL; 
	xlator_t *this = NULL;

	fd_ctx_data = dict_get (__fd->ctx, XLATOR_NAME);
        if (!fd_ctx_data) {
                errno = EBADFD;
		__offset = -1;
		goto out;
        }

        fd_ctx = data_to_ptr (fd_ctx_data);
	ctx = fd_ctx->ctx;

        switch (whence)
        {
        case SEEK_SET:
                __offset = offset;
                break;

        case SEEK_CUR:
                pthread_mutex_lock (&fd_ctx->lock);
                {
                        __offset = fd_ctx->offset;
                }
                pthread_mutex_unlock (&fd_ctx->lock);

                __offset += offset;
                break;

        case SEEK_END:
	{
		char cache_valid = 0;
		off_t end = 0;
		time_t prev, current;
		loc_t loc = {0, };
		struct stat stbuf = {0, };
		int32_t ret = -1;
		uint64_t ptr = 0;

		ret = inode_ctx_get (__fd->inode, this, &ptr);
		if (ret == 0) {
			inode_ctx = (libglusterfs_client_inode_ctx_t *)(long)ptr;
			memset (&current, 0, sizeof (current));
			current = time (NULL);
			
			pthread_mutex_lock (&inode_ctx->lock);
			{
				prev = inode_ctx->previous_lookup_time;
			}
			pthread_mutex_unlock (&inode_ctx->lock);
			
			if (prev >= 0 && ctx->lookup_timeout >= (current - prev)) {
				cache_valid = 1;
			} 
		}

		if (cache_valid) {
			end = inode_ctx->stbuf.st_size;
		} else {
			op_ret = libgf_client_loc_fill (&loc, NULL, __fd->inode->ino, ctx);
			if (op_ret < 0) {
				gf_log ("libglusterfsclient",
					GF_LOG_ERROR,
					"libgf_client_loc_fill returned -1, returning EINVAL");
				errno = EINVAL;
				__offset = -1;
				goto out;
			}
			
			op_ret = libgf_client_lookup (ctx, &loc, &stbuf, NULL, NULL);
			if (op_ret < 0) {
				__offset = -1;
				goto out;
			}

			end = stbuf.st_size;
		}

                __offset = end + offset; 
	}
	break;

	default:
		gf_log ("libglusterfsclient",
			GF_LOG_ERROR,
			"invalid value for whence");
		__offset = -1;
		errno = EINVAL;
		goto out;
        }

        pthread_mutex_lock (&fd_ctx->lock);
        {
                fd_ctx->offset = __offset;
        }
        pthread_mutex_unlock (&fd_ctx->lock);
 
out: 
        return __offset;
}


int32_t
libgf_client_stat_cbk (call_frame_t *frame,
                       void *cookie,
                       xlator_t *this,
                       int32_t op_ret,
                       int32_t op_errno,
                       struct stat *buf)
{
        libgf_client_local_t *local = frame->local;

        local->reply_stub = fop_stat_cbk_stub (frame, 
                                               NULL, 
                                               op_ret, 
                                               op_errno, 
                                               buf);

        pthread_mutex_lock (&local->lock);
        {
                local->complete = 1;
                pthread_cond_broadcast (&local->reply_cond);
        }
        pthread_mutex_unlock (&local->lock);

        return 0;
}

int32_t 
libgf_client_stat (libglusterfs_client_ctx_t *ctx, 
                   loc_t *loc,
                   struct stat *stbuf)
{
        call_stub_t *stub = NULL;
        int32_t op_ret = 0;
        time_t prev, current;
        libglusterfs_client_inode_ctx_t *inode_ctx = NULL;
        libgf_client_local_t *local = NULL;
	xlator_t *this = NULL;
	uint64_t ptr = 0;

	this = ctx->gf_ctx.graph;
	op_ret = inode_ctx_get (loc->inode, this, &ptr);
        if (op_ret == -1) {
		errno = EINVAL;
		goto out;
	}
	
	inode_ctx = (libglusterfs_client_inode_ctx_t *)(long)ptr;
        current = time (NULL);
        pthread_mutex_lock (&inode_ctx->lock);
        {
                prev = inode_ctx->previous_lookup_time;
        }
        pthread_mutex_unlock (&inode_ctx->lock);

        if ((current - prev) <= ctx->stat_timeout) {
                pthread_mutex_lock (&inode_ctx->lock);
                {
                        memcpy (stbuf, &inode_ctx->stbuf, sizeof (*stbuf));
                }
                pthread_mutex_unlock (&inode_ctx->lock);
		op_ret = 0;
		goto out;
        }
    
        LIBGF_CLIENT_FOP (ctx, stub, stat, local, loc);
 
        op_ret = stub->args.stat_cbk.op_ret;
        errno = stub->args.stat_cbk.op_errno;
        *stbuf = stub->args.stat_cbk.buf;

        pthread_mutex_lock (&inode_ctx->lock);
        {
                memcpy (&inode_ctx->stbuf, stbuf, sizeof (*stbuf));
                current = time (NULL);
                inode_ctx->previous_stat_time = current;
        }
        pthread_mutex_unlock (&inode_ctx->lock);

	call_stub_destroy (stub);

out:
        return op_ret;
}

int32_t  
glusterfs_stat (libglusterfs_handle_t handle, 
                const char *path, 
                struct stat *buf)
{
        int32_t op_ret = 0;
        loc_t loc = {0, };
        char lookup_required = 1;
        libglusterfs_client_ctx_t *ctx = handle;
	xlator_t *this = NULL;

        op_ret = libgf_client_loc_fill (&loc, path, 0, ctx);
	if (op_ret < 0) {
		gf_log ("libglusterfsclient",
			GF_LOG_ERROR,
			"libgf_client_loc_fill returned -1, returning EINVAL");
		errno = EINVAL;
		goto out;
	}

	this = ctx->gf_ctx.graph;
        if (loc.inode) {
                time_t current, prev;
                libglusterfs_client_inode_ctx_t *inode_ctx = NULL;
		uint64_t ptr = 0;

		op_ret = inode_ctx_get (loc.inode, this, &ptr);
                if (op_ret == -1) {
                        inode_unref (loc.inode);
                        errno = EINVAL;
			goto out;
                }
		
		inode_ctx = (libglusterfs_client_inode_ctx_t *)(long)ptr;
                memset (&current, 0, sizeof (current));
                current = time (NULL);

                pthread_mutex_lock (&inode_ctx->lock);
                {
                        prev = inode_ctx->previous_lookup_time;
                }
                pthread_mutex_unlock (&inode_ctx->lock);

                if (prev >= 0 && ctx->lookup_timeout >= (current - prev)) {
                        lookup_required = 0;
                } 
        }

        if (lookup_required) {
                op_ret = libgf_client_lookup (ctx, &loc, buf, NULL, NULL);
        }

        if (!op_ret) {
                op_ret = libgf_client_stat (ctx, &loc, buf);
        }

        libgf_client_loc_wipe (&loc);

out:
        return op_ret;
}

static int32_t
libgf_client_fstat_cbk (call_frame_t *frame,
                        void *cookie,
                        xlator_t *this,
                        int32_t op_ret,
                        int32_t op_errno,
                        struct stat *buf)
{  
        libgf_client_local_t *local = frame->local;

        local->reply_stub = fop_fstat_cbk_stub (frame, 
                                                NULL, 
                                                op_ret, 
                                                op_errno, 
                                                buf);

        pthread_mutex_lock (&local->lock);
        {
                local->complete = 1;
                pthread_cond_broadcast (&local->reply_cond);
        }
        pthread_mutex_unlock (&local->lock);

        return 0;

}

int32_t
libgf_client_fstat (libglusterfs_client_ctx_t *ctx, 
                    fd_t *fd, 
                    struct stat *buf)
{
        call_stub_t *stub = NULL;
        int32_t op_ret = 0;
        fd_t *__fd = fd;
        time_t current, prev;
        libglusterfs_client_inode_ctx_t *inode_ctx = NULL;
        libgf_client_local_t *local = NULL;
	xlator_t *this = NULL;
	uint64_t ptr = 0;

        current = time (NULL);
	op_ret = inode_ctx_get (fd->inode, this, &ptr);
	if (op_ret == -1) {
                errno = EINVAL;
		goto out;
        }

	inode_ctx = (libglusterfs_client_inode_ctx_t *)(long)ptr;
        pthread_mutex_lock (&inode_ctx->lock);
        {
                prev = inode_ctx->previous_stat_time;
        }
        pthread_mutex_unlock (&inode_ctx->lock);

        if ((current - prev) <= ctx->stat_timeout) {
                pthread_mutex_lock (&inode_ctx->lock);
                {
                        memcpy (buf, &inode_ctx->stbuf, sizeof (*buf));
                }
                pthread_mutex_unlock (&inode_ctx->lock);
		op_ret = 0;
		goto out;
        }

        LIBGF_CLIENT_FOP (ctx, stub, fstat, local, __fd);
 
        op_ret = stub->args.fstat_cbk.op_ret;
        errno = stub->args.fstat_cbk.op_errno;
        *buf = stub->args.fstat_cbk.buf;

        pthread_mutex_lock (&inode_ctx->lock);
        {
                memcpy (&inode_ctx->stbuf, buf, sizeof (*buf));
                current = time (NULL);
                inode_ctx->previous_stat_time = current;
        }
        pthread_mutex_unlock (&inode_ctx->lock);

	call_stub_destroy (stub);

out:
        return op_ret;
}

int32_t 
glusterfs_fstat (unsigned long fd, struct stat *buf) 
{
        libglusterfs_client_ctx_t *ctx;
        fd_t *__fd = (fd_t *)fd;
        libglusterfs_client_fd_ctx_t *fd_ctx = NULL;
        data_t *fd_ctx_data = NULL;
	int32_t op_ret = -1;

        fd_ctx_data = dict_get (((fd_t *) fd)->ctx, XLATOR_NAME);
        if (!fd_ctx_data) {
                errno = EBADF;
		op_ret = -1;
		goto out;
        }

        fd_ctx = data_to_ptr (fd_ctx_data);
        ctx = fd_ctx->ctx;

	op_ret = libgf_client_fstat (ctx, __fd, buf);

out:
	return op_ret;
}
 
static struct xlator_fops libgf_client_fops = {
};

static struct xlator_mops libgf_client_mops = {
};

static struct xlator_cbks libgf_client_cbks = {
        .forget      = libgf_client_forget,
	.release     = libgf_client_release,
	.releasedir  = libgf_client_releasedir,
};

static inline xlator_t *
libglusterfs_graph (xlator_t *graph)
{
        xlator_t *top = NULL;
        xlator_list_t *xlchild, *xlparent;

        top = CALLOC (1, sizeof (*top));
        ERR_ABORT (top);

        xlchild = CALLOC (1, sizeof(*xlchild));
        ERR_ABORT (xlchild);
        xlchild->xlator = graph;
        top->children = xlchild;
        top->ctx = graph->ctx;
        top->next = graph;
        top->name = strdup (XLATOR_NAME);

        xlparent = CALLOC (1, sizeof(*xlparent));
        xlparent->xlator = top;
        graph->parents = xlparent;
        asprintf (&top->type, XLATOR_NAME);

        top->init = libgf_client_init;
        top->fops = &libgf_client_fops;
        top->mops = &libgf_client_mops;
        top->cbks = &libgf_client_cbks; 
        top->notify = libgf_client_notify;
        top->fini = libgf_client_fini;
        //  fill_defaults (top);

        return top;
}
