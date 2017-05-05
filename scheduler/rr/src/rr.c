/*
  Copyright (c) 2006, 2007, 2008 Z RESEARCH, Inc. <http://www.zresearch.com>
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

#include <sys/time.h>
#include <stdlib.h>

#include <stdint.h>

#include "scheduler.h"

#include "rr-options.h"
#include "rr.h"

#define RR_MIN_FREE_DISK_NOT_REACHED    0
#define RR_MIN_FREE_DISK_REACHED        1

#define RR_SUBVOLUME_OFFLINE    0
#define RR_SUBVOLUME_ONLINE     1

#define LOG_ERROR(args...)      gf_log ("rr", GF_LOG_ERROR, ##args)
#define LOG_WARNING(args...)    gf_log ("rr", GF_LOG_WARNING, ##args)
#define LOG_CRITICAL(args...)    gf_log ("rr", GF_LOG_CRITICAL, ##args)

#define ROUND_ROBIN(index, count)    ((index + 1) % count)

static int 
_cleanup_rr (rr_t *rr)
{
	int i;
  
	if (rr == NULL)
	{
		return -1;
	}
  
	if (rr->options.read_only_subvolume_list != NULL)
	{
		for (i = 0; i < rr->options.read_only_subvolume_count; i++)
		{
			free (rr->options.read_only_subvolume_list[i]);
		}
		free (rr->options.read_only_subvolume_list);
	}
  
	free (rr->subvolume_list);
  
	free (rr);
  
	return 0;
}

int 
rr_init (xlator_t *this_xl)
{
	rr_t *rr = NULL;
	dict_t *options = NULL;
	xlator_list_t *children = NULL;
	uint64_t children_count = 0;
	int i = 0;
	int j = 0;
  
	if (this_xl == NULL)
	{
		return -1;
	}
  
	if ((options = this_xl->options) == NULL)
	{
		return -1;
	}
  
	if ((children = this_xl->children) == NULL)
	{
		return -1;
	}
  
	if ((rr = CALLOC (1, sizeof (rr_t))) == NULL)
	{
		return -1;
	}
  
	if (rr_options_validate (options, &rr->options) != 0)
	{
		free (rr);
		return -1;
	}
  
	for (i = 0; i < rr->options.read_only_subvolume_count; i++)
	{
		char found = 0;
      
		for (children = this_xl->children; 
		     children != NULL; 
		     children = children->next)
		{
			if (strcmp (rr->options.read_only_subvolume_list[i], 
				    children->xlator->name) == 0)
			{
				found = 1;
				break;
			}
		}
      
		if (!found)
		{
			LOG_ERROR ("read-only subvolume [%s] not found in volume list", 
				   rr->options.read_only_subvolume_list[i]);
			_cleanup_rr (rr);
			return -1;
		}
	}
  
	for (children = this_xl->children; 
	     children != NULL; 
	     children = children->next)
	{
		children_count++;
	}
  
	/* bala: excluding read_only_subvolumes */
	if ((rr->subvolume_count = children_count - 
	     rr->options.read_only_subvolume_count) == 0)
	{
		LOG_ERROR ("no writable volumes found for scheduling");
		_cleanup_rr (rr);
		return -1;
	}
  
	if ((rr->subvolume_list = CALLOC (rr->subvolume_count, 
					  sizeof (rr_subvolume_t))) == NULL)
	{
		_cleanup_rr (rr);
		return -1;
	}
  
	i = 0;
	j = 0;
	for (children = this_xl->children; 
	     children != NULL; 
	     children = children->next)
	{
		char found = 0;
      
		for (j = 0; j < rr->options.read_only_subvolume_count; j++)
		{
			if (strcmp (rr->options.read_only_subvolume_list[i], 
				    children->xlator->name) == 0)
			{
				found = 1;
				break;
			}
		}
      
		if (!found)
		{
			rr_subvolume_t *subvolume = NULL;
	  
			subvolume = &rr->subvolume_list[i];
	  
			subvolume->xl = children->xlator;
			subvolume->free_disk_status = RR_MIN_FREE_DISK_NOT_REACHED;
			subvolume->status = RR_SUBVOLUME_ONLINE;
	  
			i++;
		}
	}
  
	rr->schedule_index = UINT64_MAX;
	rr->last_stat_fetched_time.tv_sec = 0;
	rr->last_stat_fetched_time.tv_usec = 0;
	pthread_mutex_init (&rr->mutex, NULL);
  
	*((long *)this_xl->private) = (long)rr;
  
	return 0;
}

void 
rr_fini (xlator_t *this_xl)
{
	rr_t *rr = NULL;
  
	if (this_xl == NULL)
	{
		return;
	}
  
	if ((rr = (rr_t *) *((long *)this_xl->private)) != NULL)
	{
		pthread_mutex_destroy (&rr->mutex);
		_cleanup_rr (rr);
		this_xl->private = NULL;
	}
  
	return;
}

xlator_t *
rr_schedule (xlator_t *this_xl, const void *path)
{
	rr_t *rr = NULL;
	uint64_t next_schedule_index = 0;
	int i = 0;
  
	if (this_xl == NULL || path == NULL)
	{
		return NULL;
	}
  
	rr = (rr_t *) *((long *)this_xl->private);
	next_schedule_index = ROUND_ROBIN (rr->schedule_index, 
					   rr->subvolume_count);
  
	rr_update (this_xl);
  
	for (i = next_schedule_index; i < rr->subvolume_count; i++)
	{
		if (rr->subvolume_list[i].status == RR_SUBVOLUME_ONLINE && 
		    rr->subvolume_list[i].status == RR_MIN_FREE_DISK_NOT_REACHED)
		{
			pthread_mutex_lock (&rr->mutex);
			rr->schedule_index = i;
			pthread_mutex_unlock (&rr->mutex);
			return rr->subvolume_list[i].xl;
		}
	}
  
	for (i = 0; i < next_schedule_index; i++)
	{
		if (rr->subvolume_list[i].status == RR_SUBVOLUME_ONLINE && 
		    rr->subvolume_list[i].status == RR_MIN_FREE_DISK_NOT_REACHED)
		{
			pthread_mutex_lock (&rr->mutex);
			rr->schedule_index = i;
			pthread_mutex_unlock (&rr->mutex);
			return rr->subvolume_list[i].xl;
		}
	}
  
	for (i = next_schedule_index; i < rr->subvolume_count; i++)
	{
		if (rr->subvolume_list[i].status == RR_SUBVOLUME_ONLINE)
		{
			pthread_mutex_lock (&rr->mutex);
			rr->schedule_index = i;
			pthread_mutex_unlock (&rr->mutex);
			return rr->subvolume_list[i].xl;
		}
	}
  
	for (i = 0; i < next_schedule_index; i++)
	{
		if (rr->subvolume_list[i].status == RR_SUBVOLUME_ONLINE)
		{
			pthread_mutex_lock (&rr->mutex);
			rr->schedule_index = i;
			pthread_mutex_unlock (&rr->mutex);
			return rr->subvolume_list[i].xl;
		}
	}
  
	return NULL;
}

void
rr_update (xlator_t *this_xl)
{
	rr_t *rr = NULL;
	struct timeval ctime = {0, 0};
	int i = 0;
  
	if (this_xl == NULL)
	{
		return ;
	}
  
	if ((rr = (rr_t *) *((long *)this_xl->private)) == NULL)
	{
		return ;
	}
  
	if (gettimeofday (&ctime, NULL) != 0)
	{
		return ;
	}
  
	if (ctime.tv_sec > (rr->options.refresh_interval + 
			    rr->last_stat_fetched_time.tv_sec))
	{
		pthread_mutex_lock (&rr->mutex);
		rr->last_stat_fetched_time = ctime;
		pthread_mutex_unlock (&rr->mutex);
      
		for (i = 0; i < rr->subvolume_count; i++)
		{
			xlator_t *subvolume_xl = NULL;
			call_frame_t *frame = NULL;
			call_pool_t *pool = NULL;
	  
			subvolume_xl = rr->subvolume_list[i].xl;
	  
			pool = this_xl->ctx->pool;
	  
			frame = create_frame (this_xl, pool);

			STACK_WIND_COOKIE (frame,
					   rr_update_cbk, 
					   subvolume_xl->name, 
					   subvolume_xl, 
					   subvolume_xl->mops->stats, 
					   0);
		}
	}
  
	return ;
}

int 
rr_update_cbk (call_frame_t *frame, 
	       void *cookie, 
	       xlator_t *this_xl, 
	       int32_t op_ret, 
	       int32_t op_errno, 
	       struct xlator_stats *stats)
{
	rr_t *rr = NULL;
	rr_subvolume_t *subvolume = NULL;
	uint8_t free_disk_percent = 0;
	int i = 0;
  
	if (frame == NULL)
	{
		return -1;
	}
  
	if (cookie == NULL || this_xl == NULL)
	{
		STACK_DESTROY (frame->root);
		return -1;
	}
  
	if (op_ret == 0 && stats == NULL)
	{
		LOG_CRITICAL ("fatal! op_ret is 0 and stats is NULL.  "
			      "Please report this to <gluster-devel@nongnu.org>");
		STACK_DESTROY (frame->root);
		return -1;
	}
  
	if ((rr = (rr_t *) *((long *)this_xl->private)) == NULL)
	{
		STACK_DESTROY (frame->root);
		return -1;
	}
  
	for (i = 0; i < rr->subvolume_count; i++)
	{
		if (rr->subvolume_list[i].xl->name == (char *) cookie)
		{
			subvolume = &rr->subvolume_list[i];
			break;
		}
	}
  
	if (subvolume == NULL)
	{
		LOG_ERROR ("unknown cookie [%s]", (char *) cookie);
		STACK_DESTROY (frame->root);
		return -1;
	}
  
	if (op_ret == 0)
	{
		free_disk_percent = (stats->free_disk * 100) / stats->total_disk_size;
		if (free_disk_percent > rr->options.min_free_disk)
		{
			if (subvolume->free_disk_status != RR_MIN_FREE_DISK_NOT_REACHED)
			{
				pthread_mutex_lock (&rr->mutex);
				subvolume->free_disk_status = RR_MIN_FREE_DISK_NOT_REACHED;
				pthread_mutex_unlock (&rr->mutex);
				LOG_WARNING ("subvolume [%s] is available with free space for scheduling", 
					     subvolume->xl->name);
			}
		}
		else
		{
			if (subvolume->free_disk_status != RR_MIN_FREE_DISK_REACHED)
			{
				pthread_mutex_lock (&rr->mutex);
				subvolume->free_disk_status = RR_MIN_FREE_DISK_REACHED;
				pthread_mutex_unlock (&rr->mutex);
				LOG_WARNING ("subvolume [%s] reached minimum disk space requirement", 
					     subvolume->xl->name);
			}
		}
	}
	else 
	{
		pthread_mutex_lock (&rr->mutex);
		subvolume->status = RR_SUBVOLUME_OFFLINE;
		pthread_mutex_unlock (&rr->mutex);
		LOG_ERROR ("unable to get subvolume [%s] status information and "
			   "scheduling is disabled", 
			   subvolume->xl->name);
	}
  
	STACK_DESTROY (frame->root);
	return 0;
}

void
rr_notify (xlator_t *this_xl, int32_t event, void *data)
{
	rr_t *rr = NULL;
	rr_subvolume_t *subvolume = NULL;
	xlator_t *subvolume_xl = NULL;
	int i = 0, ret = 0;
	call_frame_t *frame = NULL;
	call_pool_t *pool = NULL;
	dict_t *xattr = get_new_dict ();
	int32_t version[1] = {1};

	if (this_xl == NULL || data == NULL) {
		return ;
	}
  
	if ((rr = (rr_t *) *((long *)this_xl->private)) == NULL) {
		return ;
	}
  
	subvolume_xl = (xlator_t *) data;
  
	for (i = 0; i < rr->subvolume_count; i++) {
		if (rr->subvolume_list[i].xl == subvolume_xl) {
			subvolume = &rr->subvolume_list[i];
			break;
		}
	}
  
	switch (event) {
	case GF_EVENT_CHILD_UP:
		/* Seeding, to be done only once */
		if (rr->first_time && (i == rr->subvolume_count)) {
			loc_t loc = {0,};
			xlator_t *trav = NULL;

			pool = this_xl->ctx->pool;
			frame = create_frame (this_xl, pool);
			ret = dict_set_bin (xattr, "trusted.glusterfs.scheduler.rr",
					    version, sizeof (int32_t));
			if (-1 == ret) {
				gf_log (this_xl->name, GF_LOG_ERROR, "rr seed setting failed");
			}
			if (xattr)
				dict_ref (xattr);
			
			loc.path = strdup ("/");
			for (trav = this_xl->parents->xlator; trav; trav = trav->parents->xlator) {
				if (trav->itable) {
					loc.inode = trav->itable->root;
					break;
				}
			}
			STACK_WIND (frame,
				    rr_notify_cbk,
				    (xlator_t *)data,
				    ((xlator_t *)data)->fops->xattrop,
				    &loc,
				    GF_XATTROP_ADD_ARRAY,
				    xattr);
	  
			if (xattr)
				dict_unref (xattr);

			rr->first_time = 0;
		}
		if (subvolume) {
			pthread_mutex_lock (&rr->mutex);
			subvolume->status = RR_SUBVOLUME_ONLINE;
			pthread_mutex_unlock (&rr->mutex);
		}
		break;
	case GF_EVENT_CHILD_DOWN:
		if (subvolume) {
			pthread_mutex_lock (&rr->mutex);
			subvolume->status = RR_SUBVOLUME_OFFLINE;
			pthread_mutex_unlock (&rr->mutex);
		}
		break;
	}
  
	return ;
}

int 
rr_notify_cbk (call_frame_t *frame, 
	       void *cookie, 
	       xlator_t *this_xl, 
	       int32_t op_ret, 
	       int32_t op_errno,
	       dict_t *xattr)
{
	rr_t *rr = NULL;
	int32_t *index = NULL;
	int32_t ret = -1;
	void *tmp_index_ptr = NULL;

	if (frame == NULL) 
	{
		return -1;
	}
  
	if ((this_xl == NULL) || (op_ret == -1))
	{
		STACK_DESTROY (frame->root);
		return -1;
	}
  
	if ((rr = (rr_t *) *((long *)this_xl->private)) == NULL)
	{
		STACK_DESTROY (frame->root);
		return -1;
	}
  
	ret = dict_get_bin (xattr, "trusted.glusterfs.scheduler.rr", &tmp_index_ptr);
	index = tmp_index_ptr;
	if (ret == 0)
		rr->schedule_index = (index[0] % rr->subvolume_count);
	else
		rr->schedule_index = 0;

	STACK_DESTROY (frame->root);
	return 0;
}

struct sched_ops sched = {
	.init     = rr_init,
	.fini     = rr_fini,
	.update   = rr_update,
	.schedule = rr_schedule,
	.notify   = rr_notify
};

