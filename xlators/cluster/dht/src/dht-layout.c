/*
   Copyright (c) 2008, 2009 Z RESEARCH, Inc. <http://www.zresearch.com>
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


#include "glusterfs.h"
#include "xlator.h"
#include "dht-common.h"
#include "byte-order.h"

#define layout_base_size (sizeof (dht_layout_t))

#define layout_entry_size (sizeof ((dht_layout_t *)NULL)->list[0])

#define layout_size(cnt) (layout_base_size + (cnt * layout_entry_size))


dht_layout_t *
dht_layout_new (xlator_t *this, int cnt)
{
	dht_layout_t *layout = NULL;


	layout = CALLOC (1, layout_size (cnt));
	if (!layout) {
		gf_log (this->name, GF_LOG_ERROR,
			"memory allocation failed :(");
		goto out;
	}

	layout->cnt = cnt;

out:
	return layout;
}


dht_layout_t *
dht_layout_get (xlator_t *this, inode_t *inode)
{
        uint64_t layout = 0;
        int      ret    = -1;

        ret = inode_ctx_get (inode, this, &layout);

        return (dht_layout_t *)(long)layout;
}


xlator_t *
dht_layout_search (xlator_t *this, dht_layout_t *layout, const char *name)
{
	uint32_t   hash = 0;
        xlator_t  *subvol = NULL;
	int        i = 0;
	int        ret = 0;


	ret = dht_hash_compute (layout->type, name, &hash);
	if (ret != 0) {
		gf_log (this->name, GF_LOG_ERROR,
			"hash computation failed for type=%d name=%s",
			layout->type, name);
		goto out;
	}

	for (i = 0; i < layout->cnt; i++) {
		if (layout->list[i].start <= hash
		    && layout->list[i].stop >= hash) {
			subvol = layout->list[i].xlator;
			break;
		}
	}

	if (!subvol) {
		gf_log (this->name, GF_LOG_DEBUG,
			"no subvolume for hash (value) = %u", hash);
	}

out:
	return subvol;
}


dht_layout_t *
dht_layout_for_subvol (xlator_t *this, xlator_t *subvol)
{
	dht_conf_t   *conf = NULL;
	dht_layout_t *layout = NULL;
	int           i = 0;


	conf = this->private;

	for (i = 0; i < conf->subvolume_cnt; i++) {
		if (conf->subvolumes[i] == subvol) {
			layout = conf->file_layouts[i];
			break;
		}
	}

	return layout;
}


int
dht_layouts_init (xlator_t *this, dht_conf_t *conf)
{
	dht_layout_t *layout = NULL;
	int           i = 0;
	int           ret = -1;
	

	conf->file_layouts = CALLOC (conf->subvolume_cnt,
				     sizeof (dht_layout_t *));
	if (!conf->file_layouts) {
		gf_log (this->name, GF_LOG_ERROR,
			"memory allocation failed :(");
		goto out;
	}

	for (i = 0; i < conf->subvolume_cnt; i++) {
		layout = dht_layout_new (this, 1);

		if (!layout) {
			goto out;
		}

		layout->preset = 1;

		layout->list[0].xlator = conf->subvolumes[i];

		conf->file_layouts[i] = layout;
	}

	ret = 0;
out:
	return ret;
}


int
dht_disk_layout_extract (xlator_t *this, dht_layout_t *layout,
			 int pos, int32_t **disk_layout_p)
{
	int      ret = -1;
	int32_t *disk_layout = NULL;

	disk_layout = CALLOC (5, sizeof (int));
	if (!disk_layout) {
		gf_log (this->name, GF_LOG_ERROR,
			"memory allocation failed :(");
		goto out;
	}

	disk_layout[0] = hton32 (1);
	disk_layout[1] = hton32 (layout->type);
	disk_layout[2] = hton32 (layout->list[pos].start);
	disk_layout[3] = hton32 (layout->list[pos].stop);

	if (disk_layout_p)
		*disk_layout_p = disk_layout;
	ret = 0;

out:
	return ret;
}


int
dht_disk_layout_merge (xlator_t *this, dht_layout_t *layout,
		       int pos, int32_t *disk_layout)
{
	int      cnt = 0;
	int      type = 0;
	int      start_off = 0;
	int      stop_off = 0;


	/* TODO: assert disk_layout_ptr is of required length */

	cnt  = ntoh32 (disk_layout[0]);
	if (cnt != 1) {
		gf_log (this->name, GF_LOG_ERROR,
			"disk layout has invalid count %d", cnt);
		return -1;
	}

	/* TODO: assert type is compatible */
	type      = ntoh32 (disk_layout[1]);
	start_off = ntoh32 (disk_layout[2]);
	stop_off  = ntoh32 (disk_layout[3]);

	layout->list[pos].start = start_off;
	layout->list[pos].stop  = stop_off;

	gf_log (this->name, GF_LOG_DEBUG,
		"merged to layout: %u - %u (type %d) from %s",
		start_off, stop_off, type,
		layout->list[pos].xlator->name);

	return 0;
}


int
dht_layout_merge (xlator_t *this, dht_layout_t *layout, xlator_t *subvol,
		  int op_ret, int op_errno, dict_t *xattr)
{
	int      i     = 0;
	int      ret   = -1;
	int      err   = -1;
	int32_t *disk_layout = NULL;


	if (op_ret != 0) {
		err = op_errno;
	}

	for (i = 0; i < layout->cnt; i++) {
		if (layout->list[i].xlator == NULL) {
			layout->list[i].err    = err;
			layout->list[i].xlator = subvol;
			break;
		}
	}

	if (op_ret != 0) {
		ret = 0;
		goto out;
	}

	if (xattr) {
		/* during lookup and not mkdir */
		ret = dict_get_ptr (xattr, "trusted.glusterfs.dht",
				    VOID(&disk_layout));
	}

	if (ret != 0) {
		layout->list[i].err = -1;
		gf_log (this->name, GF_LOG_DEBUG,
			"missing disk layout on %s. err = %d",
			subvol->name, err);
		ret = 0;
		goto out;
	}

	ret = dht_disk_layout_merge (this, layout, i, disk_layout);
	if (ret != 0) {
		gf_log (this->name, GF_LOG_ERROR,
			"layout merge from subvolume %s failed",
			subvol->name);
		goto out;
	}
	layout->list[i].err = 0;

out:
	return ret;
}


void
dht_layout_entry_swap (dht_layout_t *layout, int i, int j)
{
	uint32_t  start_swap = 0;
	uint32_t  stop_swap = 0;
	xlator_t *xlator_swap = 0;
	int       err_swap = 0;


	start_swap  = layout->list[i].start;
	stop_swap   = layout->list[i].stop;
	xlator_swap = layout->list[i].xlator;
	err_swap    = layout->list[i].err;

	layout->list[i].start  = layout->list[j].start;
	layout->list[i].stop   = layout->list[j].stop;
	layout->list[i].xlator = layout->list[j].xlator;
	layout->list[i].err    = layout->list[j].err;

	layout->list[j].start  = start_swap;
	layout->list[j].stop   = stop_swap;
	layout->list[j].xlator = xlator_swap;
	layout->list[j].err    = err_swap;
}


int64_t
dht_layout_entry_cmp (dht_layout_t *layout, int i, int j)
{
	int64_t diff = 0;

	if (layout->list[i].err || layout->list[j].err)
		diff = layout->list[i].err - layout->list[j].err;
	else
		diff = (int64_t) layout->list[i].start
			- (int64_t) layout->list[j].start;

	return diff;
}


int
dht_layout_sort (dht_layout_t *layout)
{
	int       i = 0;
	int       j = 0;
	int64_t   ret = 0;

	/* TODO: O(n^2) -- bad bad */

	for (i = 0; i < layout->cnt - 1; i++) {
		for (j = i + 1; j < layout->cnt; j++) {
			ret = dht_layout_entry_cmp (layout, i, j);
			if (ret > 0)
				dht_layout_entry_swap (layout, i, j);
		}
	}

	return 0;
}


int
dht_layout_anomalies (xlator_t *this, loc_t *loc, dht_layout_t *layout,
		      uint32_t *holes_p, uint32_t *overlaps_p,
		      uint32_t *missing_p, uint32_t *down_p, uint32_t *misc_p)
{
	dht_conf_t *conf = NULL;
	uint32_t    holes    = 0;
	uint32_t    overlaps = 0;
	uint32_t    missing  = 0;
	uint32_t    down     = 0;
	uint32_t    misc     = 0;
	uint32_t    hole_cnt = 0;
	uint32_t    overlap_cnt = 0;
	int         i = 0;
	int         ret = 0;
	uint32_t    prev_stop = 0;
	uint32_t    last_stop = 0;
	char        is_virgin = 1;


	conf = this->private;

	/* TODO: explain WTF is happening */

	last_stop = layout->list[0].start - 1;
	prev_stop = last_stop;

	for (i = 0; i < layout->cnt; i++) {
		if (layout->list[i].err) {
			switch (layout->list[i].err) {
			case -1:
			case ENOENT:
				missing++;
				break;
			case ENOTCONN:
				down++;
				break;
			default:
				misc++;
			}
			continue;
		}

		is_virgin = 0;

		if ((prev_stop + 1) < layout->list[i].start) {
			hole_cnt++;
			holes += (layout->list[i].start - (prev_stop + 1));
		}

		if ((prev_stop + 1) > layout->list[i].start) {
			overlap_cnt++;
			overlaps += ((prev_stop + 1) - layout->list[i].start);
		}
		prev_stop = layout->list[i].stop;
	}

	if ((last_stop - prev_stop) || is_virgin)
	    hole_cnt++;
	holes += (last_stop - prev_stop);

	if (holes_p)
		*holes_p = hole_cnt;

	if (overlaps_p)
		*overlaps_p = overlap_cnt;

	if (missing_p)
		*missing_p = missing;

	if (down_p)
		*down_p = down;

	if (misc_p)
		*misc_p = misc;

	return ret;
}


int
dht_layout_normalize (xlator_t *this, loc_t *loc, dht_layout_t *layout)
{
	int          ret   = 0;
	uint32_t     holes = 0;
	uint32_t     overlaps = 0;
	uint32_t     missing = 0;
	uint32_t     down = 0;
	uint32_t     misc = 0;


	ret = dht_layout_sort (layout);
	if (ret == -1) {
		gf_log (this->name, GF_LOG_ERROR,
			"sort failed?! how the ....");
		goto out;
	}

	ret = dht_layout_anomalies (this, loc, layout,
				    &holes, &overlaps,
				    &missing, &down, &misc);
	if (ret == -1) {
		gf_log (this->name, GF_LOG_ERROR,
			"error while finding anomalies in %s -- not good news",
			loc->path);
		goto out;
	}

	if (holes || overlaps) {
		if (missing == layout->cnt) {
			gf_log (this->name, GF_LOG_WARNING,
				"directory %s looked up first time",
				loc->path);
		} else {
			gf_log (this->name, GF_LOG_ERROR,
				"found anomalies in %s. holes=%d overlaps=%d",
				loc->path, holes, overlaps);
		}
		ret = 1;
	}

out:
	return ret;
}


int
dht_layout_dir_mismatch (xlator_t *this, dht_layout_t *layout, xlator_t *subvol,
			 loc_t *loc, dict_t *xattr)
{
	int       idx = 0;
	int       pos = -1;
	int       ret = -1;
	int32_t  *disk_layout = NULL;
	int32_t   count = -1;
	uint32_t  start_off = -1;
	uint32_t  stop_off = -1;


	for (idx = 0; idx < layout->cnt; idx++) {
		if (layout->list[idx].xlator == subvol) {
			pos = idx;
			break;
		}
	}
	
	if (pos == -1) {
		gf_log (this->name, GF_LOG_DEBUG,
			"%s - no layout info for subvolume %s",
			loc->path, subvol->name);
		ret = 1;
		goto out;
	}
	
	if (xattr == NULL) {
		gf_log (this->name, GF_LOG_ERROR,
			"%s - xattr dictionary is NULL",
			loc->path);
		ret = -1;
		goto out;
	}

	ret = dict_get_ptr (xattr, "trusted.glusterfs.dht",
			    VOID(&disk_layout));
	
	if (ret < 0) {
		gf_log (this->name, GF_LOG_ERROR,
			"%s - disk layout missing", loc->path);
		ret = -1;
		goto out;
	} 

	count  = ntoh32 (disk_layout[0]);
	if (count != 1) {
		gf_log (this->name, GF_LOG_ERROR,
			"%s - disk layout has invalid count %d",
			loc->path, count);
		ret = -1;
		goto out;
	}

	start_off = ntoh32 (disk_layout[2]);
	stop_off  = ntoh32 (disk_layout[3]);
	
	if ((layout->list[pos].start != start_off)
	    || (layout->list[pos].stop != stop_off)) {
		gf_log (this->name, GF_LOG_DEBUG,
			"subvol: %s; inode layout - %"PRId32" - %"PRId32"; "
			"disk layout - %"PRId32" - %"PRId32,
			layout->list[pos].xlator->name,
			layout->list[pos].start, layout->list[pos].stop,
			start_off, stop_off);
		ret = 1;
	} else {
		ret = 0;
	}
out:
	return ret;
}

