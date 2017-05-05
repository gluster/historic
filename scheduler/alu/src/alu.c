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



/* ALU code needs a complete re-write. This is one of the most important 
 * part of GlusterFS and so needs more and more reviews and testing 
 */

#ifndef _CONFIG_H
#define _CONFIG_H
#include "config.h"
#endif

#include <sys/time.h>
#include <stdint.h>
#include "stack.h"
#include "alu.h"

#define ALU_DISK_USAGE_ENTRY_THRESHOLD_DEFAULT         (1 * GF_UNIT_GB)
#define ALU_DISK_USAGE_EXIT_THRESHOLD_DEFAULT          (512 * GF_UNIT_MB)

#define ALU_WRITE_USAGE_ENTRY_THRESHOLD_DEFAULT        25
#define ALU_WRITE_USAGE_EXIT_THRESHOLD_DEFAULT         5

#define ALU_READ_USAGE_ENTRY_THRESHOLD_DEFAULT         25
#define ALU_READ_USAGE_EXIT_THRESHOLD_DEFAULT          5

#define ALU_OPEN_FILES_USAGE_ENTRY_THRESHOLD_DEFAULT   1000
#define ALU_OPEN_FILES_USAGE_EXIT_THRESHOLD_DEFAULT    100

#define ALU_LIMITS_TOTAL_DISK_SIZE_DEFAULT             100

#define ALU_REFRESH_INTERVAL_DEFAULT                   5
#define ALU_REFRESH_CREATE_COUNT_DEFAULT               5


static int64_t 
get_stats_disk_usage (struct xlator_stats *this)
{
	return this->disk_usage;
}

static int64_t 
get_stats_write_usage (struct xlator_stats *this)
{
	return this->write_usage;
}

static int64_t 
get_stats_read_usage (struct xlator_stats *this)
{
	return this->read_usage;
}

static int64_t 
get_stats_disk_speed (struct xlator_stats *this)
{
	return this->disk_speed;
}

static int64_t 
get_stats_file_usage (struct xlator_stats *this)
{
	/* Avoid warning "defined but not used" */
	(void) &get_stats_file_usage;    

	return this->nr_files;
}

static int64_t 
get_stats_free_disk (struct xlator_stats *this)
{
	if (this->total_disk_size > 0)
		return (this->free_disk * 100) / this->total_disk_size;
	return 0;
}

static int64_t 
get_max_diff_write_usage (struct xlator_stats *max, struct xlator_stats *min)
{
	return (max->write_usage - min->write_usage);
}

static int64_t 
get_max_diff_read_usage (struct xlator_stats *max, struct xlator_stats *min)
{
	return (max->read_usage - min->read_usage);
}

static int64_t 
get_max_diff_disk_usage (struct xlator_stats *max, struct xlator_stats *min)
{
	return (max->disk_usage - min->disk_usage);
}

static int64_t 
get_max_diff_disk_speed (struct xlator_stats *max, struct xlator_stats *min)
{
	return (max->disk_speed - min->disk_speed);
}

static int64_t 
get_max_diff_file_usage (struct xlator_stats *max, struct xlator_stats *min)
{
	return (max->nr_files - min->nr_files);
}


int 
alu_parse_options (xlator_t *xl, struct alu_sched *alu_sched)
{
	data_t *order = dict_get (xl->options, "scheduler.alu.order");
	if (!order) {
		gf_log (xl->name, GF_LOG_ERROR,
			"option 'scheduler.alu.order' not specified");
		return -1;
	}
	struct alu_threshold *_threshold_fn;
	struct alu_threshold *tmp_threshold;
	data_t *entry_fn = NULL;
	data_t *exit_fn = NULL;
	char *tmp_str = NULL;
	char *order_str = strtok_r (order->data, ":", &tmp_str);
	/* Get the scheduling priority order, specified by the user. */
	while (order_str) {
		gf_log ("alu", GF_LOG_DEBUG,
			"alu_init: order string: %s",
			order_str);
		if (strcmp (order_str, "disk-usage") == 0) {
			/* Disk usage */
			_threshold_fn = 
				CALLOC (1, sizeof (struct alu_threshold));
			ERR_ABORT (_threshold_fn);
			_threshold_fn->diff_value = get_max_diff_disk_usage;
			_threshold_fn->sched_value = get_stats_disk_usage;
			entry_fn = 
				dict_get (xl->options, 
					  "scheduler.alu.disk-usage.entry-threshold");
			if (entry_fn) {
				if (gf_string2bytesize (entry_fn->data, 
							&alu_sched->entry_limit.disk_usage) != 0) {
					gf_log (xl->name, GF_LOG_ERROR, 
						"invalid number format \"%s\" "
						"of \"option scheduler.alu."
						"disk-usage.entry-threshold\"",
						entry_fn->data);
					return -1;
				}
			} else {
				alu_sched->entry_limit.disk_usage = ALU_DISK_USAGE_ENTRY_THRESHOLD_DEFAULT;
			}
			_threshold_fn->entry_value = get_stats_disk_usage;
			exit_fn = dict_get (xl->options, 
					    "scheduler.alu.disk-usage.exit-threshold");
			if (exit_fn) {
				if (gf_string2bytesize (exit_fn->data, &alu_sched->exit_limit.disk_usage) != 0)	{
					gf_log (xl->name, GF_LOG_ERROR, 
						"invalid number format \"%s\" "
						"of \"option scheduler.alu."
						"disk-usage.exit-threshold\"", 
						exit_fn->data);
					return -1;
				}
			} else {
				alu_sched->exit_limit.disk_usage = ALU_DISK_USAGE_EXIT_THRESHOLD_DEFAULT;
			}
			_threshold_fn->exit_value = get_stats_disk_usage;
			tmp_threshold = alu_sched->threshold_fn;
			if (!tmp_threshold) {
				alu_sched->threshold_fn = _threshold_fn;
			} else {
				while (tmp_threshold->next) {
					tmp_threshold = tmp_threshold->next;
				}
				tmp_threshold->next = _threshold_fn;
			}
			gf_log ("alu",
				GF_LOG_DEBUG, "alu_init: = %"PRId64",%"PRId64"", 
				alu_sched->entry_limit.disk_usage, 
				alu_sched->exit_limit.disk_usage);
			
		} else if (strcmp (order_str, "write-usage") == 0) {
			/* Handle "write-usage" */
			
			_threshold_fn = CALLOC (1, sizeof (struct alu_threshold));
			ERR_ABORT (_threshold_fn);
			_threshold_fn->diff_value = get_max_diff_write_usage;
			_threshold_fn->sched_value = get_stats_write_usage;
			entry_fn = dict_get (xl->options, 
					     "scheduler.alu.write-usage.entry-threshold");
			if (entry_fn) {
				if (gf_string2bytesize (entry_fn->data, 
							&alu_sched->entry_limit.write_usage) != 0) {
					gf_log (xl->name, GF_LOG_ERROR, 
						"invalid number format \"%s\" "
						"of option scheduler.alu."
						"write-usage.entry-threshold", 
						entry_fn->data);
					return -1;
				}
			} else {
				alu_sched->entry_limit.write_usage = ALU_WRITE_USAGE_ENTRY_THRESHOLD_DEFAULT;
			}
			_threshold_fn->entry_value = get_stats_write_usage;
			exit_fn = dict_get (xl->options, 
					    "scheduler.alu.write-usage.exit-threshold");
			if (exit_fn) {
				if (gf_string2bytesize (exit_fn->data, 
							&alu_sched->exit_limit.write_usage) != 0) {
					gf_log (xl->name, GF_LOG_ERROR, 
						"invalid number format \"%s\""
						" of \"option scheduler.alu."
						"write-usage.exit-threshold\"",
						exit_fn->data);
					return -1;
				}
			} else {
				alu_sched->exit_limit.write_usage = ALU_WRITE_USAGE_EXIT_THRESHOLD_DEFAULT;
			}
			_threshold_fn->exit_value = get_stats_write_usage;
			tmp_threshold = alu_sched->threshold_fn;
			if (!tmp_threshold) {
				alu_sched->threshold_fn = _threshold_fn;
			} else {
				while (tmp_threshold->next) {
					tmp_threshold = tmp_threshold->next;
				}
				tmp_threshold->next = _threshold_fn;
			}
			gf_log (xl->name, GF_LOG_DEBUG, 
				"alu_init: = %"PRId64",%"PRId64"", 
				alu_sched->entry_limit.write_usage, 
				alu_sched->exit_limit.write_usage);
			
		} else if (strcmp (order_str, "read-usage") == 0) {
			/* Read usage */
			
			_threshold_fn = CALLOC (1, sizeof (struct alu_threshold));
			ERR_ABORT (_threshold_fn);
			_threshold_fn->diff_value = get_max_diff_read_usage;
			_threshold_fn->sched_value = get_stats_read_usage;
			entry_fn = dict_get (xl->options, 
					     "scheduler.alu.read-usage.entry-threshold");
			if (entry_fn) {
				if (gf_string2bytesize (entry_fn->data, 
							&alu_sched->entry_limit.read_usage) != 0) {
					gf_log (xl->name, 
						GF_LOG_ERROR, 
						"invalid number format \"%s\" "
						"of \"option scheduler.alu."
						"read-usage.entry-threshold\"",
						entry_fn->data);
					return -1;
				}
			} else {
				alu_sched->entry_limit.read_usage = ALU_READ_USAGE_ENTRY_THRESHOLD_DEFAULT;
			}
			_threshold_fn->entry_value = get_stats_read_usage;
			exit_fn = dict_get (xl->options, 
					    "scheduler.alu.read-usage.exit-threshold");
			if (exit_fn)
			{
				if (gf_string2bytesize (exit_fn->data, 
							&alu_sched->exit_limit.read_usage) != 0)
				{
					gf_log ("alu", GF_LOG_ERROR, 
						"invalid number format \"%s\" "
						"of \"option scheduler.alu."
						"read-usage.exit-threshold\"", 
						exit_fn->data);
					return -1;
				}
			}
			else
			{
				alu_sched->exit_limit.read_usage = ALU_READ_USAGE_EXIT_THRESHOLD_DEFAULT;
			}
			_threshold_fn->exit_value = get_stats_read_usage;
			tmp_threshold = alu_sched->threshold_fn;
			if (!tmp_threshold) {
				alu_sched->threshold_fn = _threshold_fn;
			}
			else {
				while (tmp_threshold->next) {
					tmp_threshold = tmp_threshold->next;
				}
				tmp_threshold->next = _threshold_fn;
			}
			gf_log ("alu", GF_LOG_DEBUG, 
				"alu_init: = %"PRId64",%"PRId64"", 
				alu_sched->entry_limit.read_usage, 
				alu_sched->exit_limit.read_usage);
			
		} else if (strcmp (order_str, "open-files-usage") == 0) {
			/* Open files counter */
			
			_threshold_fn = CALLOC (1, sizeof (struct alu_threshold));
			ERR_ABORT (_threshold_fn);
			_threshold_fn->diff_value = get_max_diff_file_usage;
			_threshold_fn->sched_value = get_stats_file_usage;
			entry_fn = dict_get (xl->options, 
					     "scheduler.alu.open-files-usage.entry-threshold");
			if (entry_fn) {
				if (gf_string2uint64 (entry_fn->data, 
						      &alu_sched->entry_limit.nr_files) != 0)
				{
					gf_log ("alu", GF_LOG_ERROR, 
						"invalid number format \"%s\" "
						"of \"option scheduler.alu."
						"open-files-usage.entry-"
						"threshold\"", entry_fn->data);
					return -1;
				}
			}
			else
			{
				alu_sched->entry_limit.nr_files = ALU_OPEN_FILES_USAGE_ENTRY_THRESHOLD_DEFAULT;
			}
			_threshold_fn->entry_value = get_stats_file_usage;
			exit_fn = dict_get (xl->options, 
					    "scheduler.alu.open-files-usage.exit-threshold");
			if (exit_fn)
			{
				if (gf_string2uint64 (exit_fn->data, 
						      &alu_sched->exit_limit.nr_files) != 0)
				{
					gf_log ("alu", GF_LOG_ERROR, 
						"invalid number format \"%s\" "
						"of \"option scheduler.alu."
						"open-files-usage.exit-"
						"threshold\"", exit_fn->data);
					return -1;
				}
			}
			else
			{
				alu_sched->exit_limit.nr_files = ALU_OPEN_FILES_USAGE_EXIT_THRESHOLD_DEFAULT;
			}
			_threshold_fn->exit_value = get_stats_file_usage;
			tmp_threshold = alu_sched->threshold_fn;
			if (!tmp_threshold) {
				alu_sched->threshold_fn = _threshold_fn;
			}
			else {
				while (tmp_threshold->next) {
					tmp_threshold = tmp_threshold->next;
				}
				tmp_threshold->next = _threshold_fn;
			}
			gf_log ("alu", GF_LOG_DEBUG, 
				"alu.c->alu_init: = %"PRIu64",%"PRIu64"", 
				alu_sched->entry_limit.nr_files, 
				alu_sched->exit_limit.nr_files);
			
		} else if (strcmp (order_str, "disk-speed-usage") == 0) {
			/* Disk speed */
			
			_threshold_fn = CALLOC (1, sizeof (struct alu_threshold));
			ERR_ABORT (_threshold_fn);
			_threshold_fn->diff_value = get_max_diff_disk_speed;
			_threshold_fn->sched_value = get_stats_disk_speed;
			entry_fn = dict_get (xl->options, 
					     "scheduler.alu.disk-speed-usage.entry-threshold");
			if (entry_fn) {
				gf_log ("alu", GF_LOG_DEBUG,
					"entry-threshold is given, "
					"value is constant");
			}
			_threshold_fn->entry_value = NULL;
			exit_fn = dict_get (xl->options, 
					    "scheduler.alu.disk-speed-usage.exit-threshold");
			if (exit_fn) {
				gf_log ("alu", GF_LOG_DEBUG,
					"exit-threshold is given, "
					"value is constant");
			}
			_threshold_fn->exit_value = NULL;
			tmp_threshold = alu_sched->threshold_fn;
			if (!tmp_threshold) {
				alu_sched->threshold_fn = _threshold_fn;
			}
			else {
				while (tmp_threshold->next) {
					tmp_threshold = tmp_threshold->next;
				}
				tmp_threshold->next = _threshold_fn;
			}
			
		} else {
			gf_log ("alu", GF_LOG_DEBUG,
				"%s, unknown option provided to scheduler",
				order_str);
		}
		order_str = strtok_r (NULL, ":", &tmp_str);
	}
	
	return 0;
}

static int32_t
alu_init (xlator_t *xl)
{
	struct alu_sched *alu_sched = NULL;
	struct alu_limits *_limit_fn = NULL;
	struct alu_limits *tmp_limits = NULL;
	uint32_t min_free_disk = 0;
	data_t *limits = NULL;
  
	alu_sched = CALLOC (1, sizeof (struct alu_sched));
	ERR_ABORT (alu_sched);

	{
		alu_parse_options (xl, alu_sched);
	}

	/* Get the limits */
	
	limits = dict_get (xl->options, 
			   "scheduler.limits.min-free-disk");
	if (limits) {
		_limit_fn = CALLOC (1, sizeof (struct alu_limits));
		ERR_ABORT (_limit_fn);
		_limit_fn->min_value = get_stats_free_disk;
		_limit_fn->cur_value = get_stats_free_disk;
		tmp_limits = alu_sched->limits_fn ;
		_limit_fn->next = tmp_limits;
		alu_sched->limits_fn = _limit_fn;
		
		if (gf_string2percent (limits->data, 
				       &min_free_disk) != 0) {
			gf_log ("alu", GF_LOG_ERROR, 
				"invalid number format \"%s\" "
				"of \"option scheduler.limits."
				"min-free-disk\"", limits->data);
			return -1;
		}
		alu_sched->spec_limit.free_disk = min_free_disk;
		
		if (alu_sched->spec_limit.free_disk >= 100) {
			gf_log ("alu", GF_LOG_ERROR,
				"check the \"option scheduler."
				"limits.min-free-disk\", it should "
				"be percentage value");
			return -1;
		}
		alu_sched->spec_limit.total_disk_size = ALU_LIMITS_TOTAL_DISK_SIZE_DEFAULT; /* Its in % */
		gf_log ("alu", GF_LOG_DEBUG,
			"alu.limit.min-disk-free = %"PRId64"", 
			_limit_fn->cur_value (&(alu_sched->spec_limit)));
	}
	
	limits = dict_get (xl->options, 
			   "scheduler.limits.max-open-files");
	if (limits) {
		// Update alu_sched->priority properly
		_limit_fn = CALLOC (1, sizeof (struct alu_limits));
		ERR_ABORT (_limit_fn);
		_limit_fn->max_value = get_stats_file_usage;
		_limit_fn->cur_value = get_stats_file_usage;
		tmp_limits = alu_sched->limits_fn ;
		_limit_fn->next = tmp_limits;
		alu_sched->limits_fn = _limit_fn;
		if (gf_string2uint64_base10 (limits->data, 
					     &alu_sched->spec_limit.nr_files) != 0)
		{
			gf_log ("alu", GF_LOG_ERROR, 
				"invalid number format '%s' of option "
				"scheduler.limits.max-open-files", 
				limits->data);
			return -1;
		}
		
		gf_log ("alu", GF_LOG_DEBUG,
			"alu_init: limit.max-open-files = %"PRId64"",
			_limit_fn->cur_value (&(alu_sched->spec_limit)));
	}


	/* Stats refresh options */
	limits = dict_get (xl->options, 
			   "scheduler.refresh-interval");
	if (limits) {
		if (gf_string2time (limits->data, 
				    &alu_sched->refresh_interval) != 0)	{
			gf_log ("alu", GF_LOG_ERROR, 
				"invalid number format \"%s\" of "
				"option scheduler.refresh-interval", 
				limits->data);
			return -1;
		}
	} else {
		alu_sched->refresh_interval = ALU_REFRESH_INTERVAL_DEFAULT;
	}
	gettimeofday (&(alu_sched->last_stat_fetch), NULL);
	
	
	limits = dict_get (xl->options, 
			   "scheduler.alu.stat-refresh.num-file-create");
	if (limits) {
		if (gf_string2uint32 (limits->data, 
				      &alu_sched->refresh_create_count) != 0)
		{
			gf_log ("alu", GF_LOG_ERROR, 
				"invalid number format \"%s\" of \"option "
				"alu.stat-refresh.num-file-create\"", 
				limits->data);
			return -1;
		}
	} else {
		alu_sched->refresh_create_count = ALU_REFRESH_CREATE_COUNT_DEFAULT;
	}

	{
		/* Build an array of child_nodes */
		struct alu_sched_struct *sched_array = NULL;
		xlator_list_t *trav_xl = xl->children;
		data_t *data = NULL;
		int32_t index = 0;
		
		while (trav_xl) {
			index++;
			trav_xl = trav_xl->next;
		}
		alu_sched->child_count = index;
		sched_array = CALLOC (index, sizeof (struct alu_sched_struct));
		ERR_ABORT (sched_array);
		trav_xl = xl->children;
		index = 0;
		while (trav_xl) {
			sched_array[index].xl = trav_xl->xlator;
			sched_array[index].eligible = 1;
			index++;
			trav_xl = trav_xl->next;
		}
		alu_sched->array = sched_array;

		data = dict_get (xl->options, 
				 "scheduler.read-only-subvolumes");
		if (data) {
			char *child = NULL;
			char *tmp = NULL;
			char *childs_data = strdup (data->data);
      
			child = strtok_r (childs_data, ",", &tmp);
			while (child) {
				for (index = 1; index < alu_sched->child_count; index++) {
					if (strcmp (alu_sched->array[index -1].xl->name, child) == 0) {
						memcpy (&(alu_sched->array[index -1]), 
							&(alu_sched->array[alu_sched->child_count -1]), 
							sizeof (struct alu_sched_struct));
						alu_sched->child_count--;
						break;
					}
				}
				child = strtok_r (NULL, ",", &tmp);
			}
		}
	}

	*((long *)xl->private) = (long)alu_sched;

	/* Initialize all the alu_sched structure's elements */
	{
		alu_sched->sched_nodes_pending = 0;

		alu_sched->min_limit.free_disk = 0x00FFFFFF;
		alu_sched->min_limit.disk_usage = 0xFFFFFFFF;
		alu_sched->min_limit.total_disk_size = 0xFFFFFFFF;
		alu_sched->min_limit.disk_speed = 0xFFFFFFFF;
		alu_sched->min_limit.write_usage = 0xFFFFFFFF;
		alu_sched->min_limit.read_usage = 0xFFFFFFFF;
		alu_sched->min_limit.nr_files = 0xFFFFFFFF;
		alu_sched->min_limit.nr_clients = 0xFFFFFFFF;
	}

	pthread_mutex_init (&alu_sched->alu_mutex, NULL);
	return 0;
}

static void
alu_fini (xlator_t *xl)
{
	if (!xl)
		return;
	struct alu_sched *alu_sched = (struct alu_sched *)*((long *)xl->private);
	struct alu_limits *limit = alu_sched->limits_fn;
	struct alu_threshold *threshold = alu_sched->threshold_fn;
	void *tmp = NULL;
	pthread_mutex_destroy (&alu_sched->alu_mutex);
	free (alu_sched->array);
	while (limit) {
		tmp = limit;
		limit = limit->next;
		free (tmp);
	}
	while (threshold) {
		tmp = threshold;
		threshold = threshold->next;
		free (tmp);
	}
	free (alu_sched);
}

static int32_t 
update_stat_array_cbk (call_frame_t *frame,
		       void *cookie,
		       xlator_t *xl,
		       int32_t op_ret,
		       int32_t op_errno,
		       struct xlator_stats *trav_stats)
{
	struct alu_sched *alu_sched = (struct alu_sched *)*((long *)xl->private);
	struct alu_limits *limits_fn = alu_sched->limits_fn;
	int32_t idx = 0;
  
	pthread_mutex_lock (&alu_sched->alu_mutex);
	for (idx = 0; idx < alu_sched->child_count; idx++) {
		if (alu_sched->array[idx].xl == (xlator_t *)cookie)
			break;
	}
	pthread_mutex_unlock (&alu_sched->alu_mutex);

	if (op_ret == -1) {
		alu_sched->array[idx].eligible = 0;
	} else {
		memcpy (&(alu_sched->array[idx].stats), trav_stats, sizeof (struct xlator_stats));
    
		/* Get stats from all the child node */
		/* Here check the limits specified by the user to 
		   consider the nodes to be used by scheduler */
		alu_sched->array[idx].eligible = 1;
		limits_fn = alu_sched->limits_fn;
		while (limits_fn){
			if (limits_fn->max_value && 
			    (limits_fn->cur_value (trav_stats) > 
			     limits_fn->max_value (&(alu_sched->spec_limit)))) {
				alu_sched->array[idx].eligible = 0;
			}
			if (limits_fn->min_value && 
			    (limits_fn->cur_value (trav_stats) < 
			     limits_fn->min_value (&(alu_sched->spec_limit)))) {
				alu_sched->array[idx].eligible = 0;
			}
			limits_fn = limits_fn->next;
		}

		/* Select minimum and maximum disk_usage */
		if (trav_stats->disk_usage > alu_sched->max_limit.disk_usage) {
			alu_sched->max_limit.disk_usage = trav_stats->disk_usage;
		}
		if (trav_stats->disk_usage < alu_sched->min_limit.disk_usage) {
			alu_sched->min_limit.disk_usage = trav_stats->disk_usage;
		}

		/* Select minimum and maximum disk_speed */
		if (trav_stats->disk_speed > alu_sched->max_limit.disk_speed) {
			alu_sched->max_limit.disk_speed = trav_stats->disk_speed;
		}
		if (trav_stats->disk_speed < alu_sched->min_limit.disk_speed) {
			alu_sched->min_limit.disk_speed = trav_stats->disk_speed;
		}

		/* Select minimum and maximum number of open files */
		if (trav_stats->nr_files > alu_sched->max_limit.nr_files) {
			alu_sched->max_limit.nr_files = trav_stats->nr_files;
		}
		if (trav_stats->nr_files < alu_sched->min_limit.nr_files) {
			alu_sched->min_limit.nr_files = trav_stats->nr_files;
		}

		/* Select minimum and maximum write-usage */
		if (trav_stats->write_usage > alu_sched->max_limit.write_usage) {
			alu_sched->max_limit.write_usage = trav_stats->write_usage;
		}
		if (trav_stats->write_usage < alu_sched->min_limit.write_usage) {
			alu_sched->min_limit.write_usage = trav_stats->write_usage;
		}

		/* Select minimum and maximum read-usage */
		if (trav_stats->read_usage > alu_sched->max_limit.read_usage) {
			alu_sched->max_limit.read_usage = trav_stats->read_usage;
		}
		if (trav_stats->read_usage < alu_sched->min_limit.read_usage) {
			alu_sched->min_limit.read_usage = trav_stats->read_usage;
		}

		/* Select minimum and maximum free-disk */
		if (trav_stats->free_disk > alu_sched->max_limit.free_disk) {
			alu_sched->max_limit.free_disk = trav_stats->free_disk;
		}
		if (trav_stats->free_disk < alu_sched->min_limit.free_disk) {
			alu_sched->min_limit.free_disk = trav_stats->free_disk;
		}
	}

	STACK_DESTROY (frame->root);

	return 0;
}

static void 
update_stat_array (xlator_t *xl)
{
	/* This function schedules the file in one of the child nodes */
	struct alu_sched *alu_sched = (struct alu_sched *)*((long *)xl->private);
	int32_t idx = 0;
	call_frame_t *frame = NULL;
	call_pool_t *pool = xl->ctx->pool;

	for (idx = 0 ; idx < alu_sched->child_count; idx++) {
		frame = create_frame (xl, pool);

		STACK_WIND_COOKIE (frame,
				   update_stat_array_cbk, 
				   alu_sched->array[idx].xl, //cookie
				   alu_sched->array[idx].xl, 
				   (alu_sched->array[idx].xl)->mops->stats,
				   0); //flag
	}
	return;
}

static void 
alu_update (xlator_t *xl)
{
	struct timeval tv;
	struct alu_sched *alu_sched = (struct alu_sched *)*((long *)xl->private);

	gettimeofday (&tv, NULL);
	if (tv.tv_sec > (alu_sched->refresh_interval + alu_sched->last_stat_fetch.tv_sec)) {
		/* Update the stats from all the server */
		update_stat_array (xl);
		alu_sched->last_stat_fetch.tv_sec = tv.tv_sec;
	}
}

static xlator_t *
alu_scheduler (xlator_t *xl, const void *path)
{
	/* This function schedules the file in one of the child nodes */
	struct alu_sched *alu_sched = (struct alu_sched *)*((long *)xl->private);
	int32_t sched_index = 0;
	int32_t sched_index_orig = 0;
	int32_t idx = 0;

	alu_update (xl);

	/* Now check each threshold one by one if some nodes are classified */
	{
		struct alu_threshold *trav_threshold = alu_sched->threshold_fn;
		struct alu_threshold *tmp_threshold = alu_sched->sched_method;
		struct alu_sched_node *tmp_sched_node;   

		/* This pointer 'trav_threshold' contains function pointers according to spec file
		   give by user, */
		while (trav_threshold) {
			/* This check is needed for seeing if already there are nodes in this criteria 
			   to be scheduled */
			if (!alu_sched->sched_nodes_pending) {
				for (idx = 0; idx < alu_sched->child_count; idx++) {
					if (!alu_sched->array[idx].eligible) {
						continue;
					}
					if (trav_threshold->entry_value) {
						if (trav_threshold->diff_value (&(alu_sched->max_limit),
										&(alu_sched->array[idx].stats)) <
						    trav_threshold->entry_value (&(alu_sched->entry_limit))) {
							continue;
						}
					}
					tmp_sched_node = CALLOC (1, sizeof (struct alu_sched_node));
					ERR_ABORT (tmp_sched_node);
					tmp_sched_node->index = idx;
					if (!alu_sched->sched_node) {
						alu_sched->sched_node = tmp_sched_node;
					} else {
						pthread_mutex_lock (&alu_sched->alu_mutex);
						tmp_sched_node->next = alu_sched->sched_node;
						alu_sched->sched_node = tmp_sched_node;
						pthread_mutex_unlock (&alu_sched->alu_mutex);
					}
					alu_sched->sched_nodes_pending++;
				}
			} /* end of if (sched_nodes_pending) */

			/* This loop is required to check the eligible nodes */
			struct alu_sched_node *trav_sched_node;
			while (alu_sched->sched_nodes_pending) {
				trav_sched_node = alu_sched->sched_node;
				sched_index = trav_sched_node->index;
				if (alu_sched->array[sched_index].eligible)
					break;
				alu_sched->sched_node = trav_sched_node->next;
				free (trav_sched_node);
				alu_sched->sched_nodes_pending--;
			}
			if (alu_sched->sched_nodes_pending) {
				/* There are some node in this criteria to be scheduled, no need 
				 * to sort and check other methods 
				 */
				if (tmp_threshold && tmp_threshold->exit_value) {
					/* verify the exit value && whether node is eligible or not */
					if (tmp_threshold->diff_value (&(alu_sched->max_limit),
								       &(alu_sched->array[sched_index].stats)) >
					    tmp_threshold->exit_value (&(alu_sched->exit_limit))) {
						/* Free the allocated info for the node :) */
						pthread_mutex_lock (&alu_sched->alu_mutex);
						alu_sched->sched_node = trav_sched_node->next;
						free (trav_sched_node);
						trav_sched_node = alu_sched->sched_node;
						alu_sched->sched_nodes_pending--;
						pthread_mutex_unlock (&alu_sched->alu_mutex);
					}
				} else {
					/* if there is no exit value, then exit after scheduling once */
					pthread_mutex_lock (&alu_sched->alu_mutex);
					alu_sched->sched_node = trav_sched_node->next;
					free (trav_sched_node);
					trav_sched_node = alu_sched->sched_node;
					alu_sched->sched_nodes_pending--;
					pthread_mutex_unlock (&alu_sched->alu_mutex);
				}
	
				alu_sched->sched_method = tmp_threshold; /* this is the method used for selecting */

				/* */
				if (trav_sched_node) {
					tmp_sched_node = trav_sched_node;
					while (trav_sched_node->next) {
						trav_sched_node = trav_sched_node->next;
					}
					if (tmp_sched_node->next) {
						pthread_mutex_lock (&alu_sched->alu_mutex);
						alu_sched->sched_node = tmp_sched_node->next;
						tmp_sched_node->next = NULL;
						trav_sched_node->next = tmp_sched_node;
						pthread_mutex_unlock (&alu_sched->alu_mutex);
					}
				}
				/* return the scheduled node */
				return alu_sched->array[sched_index].xl;
			} /* end of if (pending_nodes) */
      
			tmp_threshold = trav_threshold;
			trav_threshold = trav_threshold->next;
		}
	}
  
	/* This is used only when there is everything seems ok, or no eligible nodes */
	sched_index_orig = alu_sched->sched_index;
	alu_sched->sched_method = NULL;
	while (1) {
		//lock
		pthread_mutex_lock (&alu_sched->alu_mutex);
		sched_index = alu_sched->sched_index++;
		alu_sched->sched_index = alu_sched->sched_index % alu_sched->child_count;
		pthread_mutex_unlock (&alu_sched->alu_mutex);
		//unlock
		if (alu_sched->array[sched_index].eligible)
			break;
		if (sched_index_orig == (sched_index + 1) % alu_sched->child_count) {
			gf_log ("alu", GF_LOG_WARNING, "No node is eligible to schedule");
			//lock
			pthread_mutex_lock (&alu_sched->alu_mutex);
			alu_sched->sched_index++;
			alu_sched->sched_index = alu_sched->sched_index % alu_sched->child_count;
			pthread_mutex_unlock (&alu_sched->alu_mutex);
			//unlock
			break;
		}
	}
	return alu_sched->array[sched_index].xl;
}

/**
 * notify
 */
void
alu_notify (xlator_t *xl, int32_t event, void *data)
{
	struct alu_sched *alu_sched = NULL; 
	int32_t idx = 0;
  
	alu_sched = (struct alu_sched *)*((long *)xl->private);
	if (!alu_sched)
		return;

	for (idx = 0; idx < alu_sched->child_count; idx++) {
		if (alu_sched->array[idx].xl == (xlator_t *)data)
			break;
	}

	switch (event)
	{
	case GF_EVENT_CHILD_UP:
	{
		//alu_sched->array[idx].eligible = 1;
	}
	break;
	case GF_EVENT_CHILD_DOWN:
	{
		alu_sched->array[idx].eligible = 0;
	}
	break;
	default:
	{
		;
	}
	break;
	}

}

struct sched_ops sched = {
	.init     = alu_init,
	.fini     = alu_fini,
	.update   = alu_update,
	.schedule = alu_scheduler,
	.notify   = alu_notify
};

struct volume_options options[] = {
	{ .key   = { "scheduler.alu.order", "alu.order" },  
	  .type  = GF_OPTION_TYPE_ANY 
	},
	{ .key   = { "scheduler.alu.disk-usage.entry-threshold", 
		     "alu.disk-usage.entry-threshold" },  
	  .type  = GF_OPTION_TYPE_SIZET
	},
	{ .key   = { "scheduler.alu.disk-usage.exit-threshold", 
		     "alu.disk-usage.exit-threshold" },  
	  .type  = GF_OPTION_TYPE_SIZET
	},
	{ .key   = { "scheduler.alu.write-usage.entry-threshold", 
		     "alu.write-usage.entry-threshold" },  
	  .type  = GF_OPTION_TYPE_SIZET
	},
	{ .key   = { "scheduler.alu.write-usage.exit-threshold", 
		     "alu.write-usage.exit-threshold" },  
	  .type  = GF_OPTION_TYPE_SIZET 
	},
	{ .key   = { "scheduler.alu.read-usage.entry-threshold", 
		     "alu.read-usage.entry-threshold" },  
	  .type  = GF_OPTION_TYPE_SIZET
	},
	{ .key   = { "scheduler.alu.read-usage.exit-threshold", 
		     "alu.read-usage.exit-threshold" },  
	  .type  = GF_OPTION_TYPE_SIZET 
	},
	{ .key   = { "scheduler.alu.open-files-usage.entry-threshold", 
		     "alu.open-files-usage.entry-threshold" },  
	  .type  = GF_OPTION_TYPE_INT
	},
	{ .key   = { "scheduler.alu.open-files-usage.exit-threshold", 
		     "alu.open-files-usage.exit-threshold" },  
	  .type  = GF_OPTION_TYPE_INT 
	},
	{ .key   = { "scheduler.read-only-subvolumes",
		     "alu.read-only-subvolumes" },  
	  .type  = GF_OPTION_TYPE_ANY 
	},
	{ .key   = { "scheduler.refresh-interval", 
		     "alu.refresh-interval",
		     "alu.stat-refresh.interval" },
	  .type  = GF_OPTION_TYPE_TIME
	},
	{ .key   = { "scheduler.limits.min-free-disk", 
		     "alu.limits.min-free-disk" },  
	  .type  = GF_OPTION_TYPE_PERCENT
	},
	{ .key   = { "scheduler.alu.stat-refresh.num-file-create"
		     "alu.stat-refresh.num-file-create"},  
	  .type  = GF_OPTION_TYPE_INT
	},
	{ .key  =  {NULL}, }
};
