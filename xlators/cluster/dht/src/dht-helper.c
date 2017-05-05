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

#ifndef _CONFIG_H
#define _CONFIG_H
#include "config.h"
#endif


#include "glusterfs.h"
#include "xlator.h"
#include "dht-common.h"


int
dht_frame_return (call_frame_t *frame)
{
	dht_local_t *local = NULL;
	int          this_call_cnt = -1;

	if (!frame)
		return -1;

	local = frame->local;

	LOCK (&frame->lock);
	{
		this_call_cnt = --local->call_cnt;
	}
	UNLOCK (&frame->lock);

	return this_call_cnt;
}


int
dht_itransform (xlator_t *this, xlator_t *subvol, uint64_t x, uint64_t *y_p)
{
	dht_conf_t *conf = NULL;
	int         cnt = 0;
	int         max = 0;
	uint64_t    y = 0;


	if (x == ((uint64_t) -1)) {
		y = (uint64_t) -1;
		goto out;
	}

	conf = this->private;

	max = conf->subvolume_cnt;
	cnt = dht_subvol_cnt (this, subvol);

	y = ((x * max) + cnt);

out:
	if (y_p)
		*y_p = y;

	return 0;
}


int
dht_deitransform (xlator_t *this, uint64_t y, xlator_t **subvol_p,
		  uint64_t *x_p)
{
	dht_conf_t *conf = NULL;
	int         cnt = 0;
	int         max = 0;
	uint64_t    x = 0;
	xlator_t   *subvol = 0;


	conf = this->private;
	max = conf->subvolume_cnt;

	cnt = y % max;
	x   = y / max;

	subvol = conf->subvolumes[cnt];

	if (subvol_p)
		*subvol_p = subvol;

	if (x_p)
		*x_p = x;

	return 0;
}


void
dht_local_wipe (dht_local_t *local)
{
	if (!local)
		return;

	loc_wipe (&local->loc);
	loc_wipe (&local->loc2);

	if (local->xattr)
		dict_unref (local->xattr);

	if (local->inode)
		inode_unref (local->inode);

	if (local->layout)
		FREE (local->layout);

	loc_wipe (&local->linkfile.loc);

	if (local->linkfile.xattr)
		dict_unref (local->linkfile.xattr);

	if (local->linkfile.inode)
		inode_unref (local->linkfile.inode);

	if (local->fd) {
		fd_unref (local->fd);
		local->fd = NULL;
	}
	
	if (local->xattr_req)
		dict_unref (local->xattr_req);

	FREE (local);
}


dht_local_t *
dht_local_init (call_frame_t *frame)
{
	dht_local_t *local = NULL;

	/* TODO: use mem-pool */
	local = CALLOC (1, sizeof (*local));

	if (!local)
		return NULL;

	local->op_ret = -1;
	local->op_errno = EUCLEAN;

	frame->local = local;

	return local;
}


char *
basestr (const char *str)
{
        char *basestr = NULL;

        basestr = strrchr (str, '/');
        if (basestr)
                basestr ++;

        return basestr;
}

xlator_t *
dht_first_up_child (xlator_t *this)
{
	dht_conf_t *conf = NULL;
	xlator_t   *child = NULL;
	int         i = 0;

	conf = this->private;
	
	LOCK (&conf->subvolume_lock);
	{
		for (i = 0; i < conf->subvolume_cnt; i++) {
			if (conf->subvolume_status[i]) {
				child = conf->subvolumes[i];
				break;
			}
		}
	}
	UNLOCK (&conf->subvolume_lock);
	
	return child;
}

xlator_t *
dht_subvol_get_hashed (xlator_t *this, loc_t *loc)
{
        dht_layout_t *layout = NULL;
        xlator_t     *subvol = NULL;

        if (is_fs_root (loc)) {
                subvol = dht_first_up_child (this);
                goto out;
        }

        layout = dht_layout_get (this, loc->parent);

        if (!layout) {
                gf_log (this->name, GF_LOG_ERROR,
                        "layout missing path=%s parent=%"PRId64,
                        loc->path, loc->parent->ino);
                goto out;
        }

        subvol = dht_layout_search (this, layout, loc->name);

        if (!subvol) {
                gf_log (this->name, GF_LOG_ERROR,
                        "could not find subvolume for path=%s",
                        loc->path);
                goto out;
        }

out:
        return subvol;
}


xlator_t *
dht_subvol_get_cached (xlator_t *this, inode_t *inode)
{
        dht_layout_t *layout = NULL;
        xlator_t     *subvol = NULL;


        layout = dht_layout_get (this, inode);

        if (!layout) {
                goto out;
        }

	subvol = layout->list[0].xlator;

out:
        return subvol;
}


xlator_t *
dht_subvol_next (xlator_t *this, xlator_t *prev)
{
	dht_conf_t *conf = NULL;
	int         i = 0;
	xlator_t   *next = NULL;

	conf = this->private;

	for (i = 0; i < conf->subvolume_cnt; i++) {
		if (conf->subvolumes[i] == prev) {
			if ((i + 1) < conf->subvolume_cnt)
				next = conf->subvolumes[i + 1];
			break;
		}
	}

	return next;
}


int
dht_subvol_cnt (xlator_t *this, xlator_t *subvol)
{
	int i = 0;
	int ret = -1;
	dht_conf_t *conf = NULL;


	conf = this->private;

	for (i = 0; i < conf->subvolume_cnt; i++) {
		if (subvol == conf->subvolumes[i]) {
			ret = i;
			break;
		}
	}

	return ret;
}


#define set_if_greater(a, b) do {		\
		if ((a) < (b))			\
			(a) = (b);		\
	} while (0)

int
dht_stat_merge (xlator_t *this, struct stat *to,
		struct stat *from, xlator_t *subvol)
{
	to->st_dev      = from->st_dev;

	dht_itransform (this, subvol, from->st_ino, &to->st_ino);

	to->st_mode     = from->st_mode;
	to->st_nlink    = from->st_nlink;
	to->st_uid      = from->st_uid;
	to->st_gid      = from->st_gid;
	to->st_rdev     = from->st_rdev;
	to->st_size    += from->st_size;
	to->st_blksize  = from->st_blksize;
	to->st_blocks  += from->st_blocks;

	set_if_greater (to->st_atime, from->st_atime);
	set_if_greater (to->st_mtime, from->st_mtime);
	set_if_greater (to->st_ctime, from->st_ctime);

	return 0;
}
