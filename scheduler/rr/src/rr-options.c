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

#include "scheduler.h"
#include "rr-options.h"

#define RR_LIMITS_MIN_FREE_DISK_OPTION_STRING  "scheduler.limits.min-free-disk"
#define RR_LIMITS_MIN_FREE_DISK_VALUE_DEFAULT  15
#define RR_LIMITS_MIN_FREE_DISK_VALUE_MIN      0
#define RR_LIMITS_MIN_FREE_DISK_VALUE_MAX      100

#define RR_REFRESH_INTERVAL_OPTION_STRING      "scheduler.refresh-interval"
#define RR_REFRESH_INTERVAL_VALUE_DEFAULT      10

#define RR_READ_ONLY_SUBVOLUMES_OPTION_STRING  "scheduler.read-only-subvolumes"

#define LOG_ERROR(args...)      gf_log ("rr-options", GF_LOG_ERROR, ##args)
#define LOG_WARNING(args...)    gf_log ("rr-options", GF_LOG_WARNING, ##args)

static int 
_rr_options_min_free_disk_validate (const char *value_string, uint32_t *n)
{
	uint32_t value = 0;
  
	if (value_string == NULL)
	{
		return -1;
	}
  
	if (gf_string2percent (value_string, &value) != 0)
	{
		gf_log ("rr", 
			GF_LOG_ERROR, 
			"invalid number format [%s] of option [%s]", 
			value_string, 
			RR_LIMITS_MIN_FREE_DISK_OPTION_STRING);
		return -1;
	}
  
	if ((value <= RR_LIMITS_MIN_FREE_DISK_VALUE_MIN) || 
	    (value >= RR_LIMITS_MIN_FREE_DISK_VALUE_MAX))
	{
		gf_log ("rr", 
			GF_LOG_ERROR, 
			"out of range [%d] of option [%s].  Allowed range is 0 to 100.", 
			value, 
			RR_LIMITS_MIN_FREE_DISK_OPTION_STRING);
		return -1;
	}
  
	*n = value;
  
	return 0;
}

static int 
_rr_options_refresh_interval_validate (const char *value_string, uint32_t *n)
{
	uint32_t value = 0;
  
	if (value_string == NULL)
	{
		return -1;
	}
  
	if (gf_string2time (value_string, &value) != 0)
	{
		gf_log ("rr", 
			GF_LOG_ERROR, 
			"invalid number format [%s] of option [%s]", 
			value_string, 
			RR_REFRESH_INTERVAL_OPTION_STRING);
		return -1;
	}
  
	*n = value;
  
	return 0;
}

static int 
_rr_options_read_only_subvolumes_validate (const char *value_string, 
					   char ***volume_list, 
					   uint64_t *volume_count)
{
	char **vlist = NULL;
	int vcount = 0;
	int i = 0;
  
	if (value_string == NULL || volume_list == NULL || volume_count)
	{
		return -1;
	}
  
	if (gf_strsplit (value_string, 
			 ", ", 
			 &vlist, 
			 &vcount) != 0)
	{
		gf_log ("rr", 
			GF_LOG_ERROR, 
			"invalid subvolume list [%s] of option [%s]", 
			value_string, 
			RR_READ_ONLY_SUBVOLUMES_OPTION_STRING);
		return -1;
	}
  
	for (i = 0; i < vcount; i++)
	{
		if (gf_volume_name_validate (vlist[i]) != 0)
		{
			gf_log ("rr", 
				GF_LOG_ERROR, 
				"invalid subvolume name [%s] in [%s] of option [%s]", 
				vlist[i], 
				value_string, 
				RR_READ_ONLY_SUBVOLUMES_OPTION_STRING);
			goto free_exit;
		}
	}
  
	*volume_list = vlist;
	*volume_count = vcount;
  
	return 0;
  
 free_exit:
	for (i = 0; i < vcount; i++)
	{
		free (vlist[i]);
	}
	free (vlist);
  
	return -1;
}

int 
rr_options_validate (dict_t *options, rr_options_t *rr_options)
{
	char *value_string = NULL;
  
	if (options == NULL || rr_options == NULL)
	{
		return -1;
	}
  
	if (dict_get (options, RR_LIMITS_MIN_FREE_DISK_OPTION_STRING))
		if (data_to_str (dict_get (options, RR_LIMITS_MIN_FREE_DISK_OPTION_STRING)))
			value_string = data_to_str (dict_get (options, 
							      RR_LIMITS_MIN_FREE_DISK_OPTION_STRING));
	if (value_string != NULL)
	{
		if (_rr_options_min_free_disk_validate (value_string, 
							&rr_options->min_free_disk) != 0)
		{
			return -1;
		}
      
		gf_log ("rr", 
			GF_LOG_WARNING, 
			"using %s = %d", 
			RR_LIMITS_MIN_FREE_DISK_OPTION_STRING, 
			rr_options->min_free_disk);
	}
	else 
	{
		rr_options->min_free_disk = RR_LIMITS_MIN_FREE_DISK_VALUE_DEFAULT;
      
		gf_log ("rr", GF_LOG_DEBUG, 
			"using %s = %d [default]", 
			RR_LIMITS_MIN_FREE_DISK_OPTION_STRING, 
			rr_options->min_free_disk);
	}
  
	value_string = NULL;
	if (dict_get (options, RR_REFRESH_INTERVAL_OPTION_STRING))
		value_string = data_to_str (dict_get (options, 
						      RR_REFRESH_INTERVAL_OPTION_STRING));
	if (value_string != NULL)
	{
		if (_rr_options_refresh_interval_validate (value_string, 
							   &rr_options->refresh_interval) != 0)
		{
			return -1;
		}
      
		gf_log ("rr", 
			GF_LOG_WARNING, 
			"using %s = %d", 
			RR_REFRESH_INTERVAL_OPTION_STRING, 
			rr_options->refresh_interval);
	}
	else 
	{
		rr_options->refresh_interval = RR_REFRESH_INTERVAL_VALUE_DEFAULT;
      
		gf_log ("rr", GF_LOG_DEBUG, 
			"using %s = %d [default]", 
			RR_REFRESH_INTERVAL_OPTION_STRING, 
			rr_options->refresh_interval);
	}
  
	value_string = NULL;
	if (dict_get (options, RR_READ_ONLY_SUBVOLUMES_OPTION_STRING))
		value_string = data_to_str (dict_get (options, 
						      RR_READ_ONLY_SUBVOLUMES_OPTION_STRING));
	if (value_string != NULL)
	{
		if (_rr_options_read_only_subvolumes_validate (value_string, 
							       &rr_options->read_only_subvolume_list, 
							       &rr_options->read_only_subvolume_count) != 0)
		{
			return -1;
		}
      
		gf_log ("rr", 
			GF_LOG_WARNING, 
			"using %s = [%s]", 
			RR_READ_ONLY_SUBVOLUMES_OPTION_STRING, 
			value_string);
	}
  
	return 0;
}

struct volume_options options[] = {
	{ .key   = { "scheduler.refresh-interval", 
		     "rr.refresh-interval" },  
	  .type  = GF_OPTION_TYPE_TIME
	},
	{ .key   = { "scheduler.limits.min-free-disk", 
		     "rr.limits.min-free-disk" },  
	  .type  = GF_OPTION_TYPE_PERCENT
	},
	{ .key   = { "scheduler.read-only-subvolumes", 
		     "rr.read-only-subvolumes" },  
	  .type  = GF_OPTION_TYPE_ANY
	},
	{ .key = {NULL} }
};
