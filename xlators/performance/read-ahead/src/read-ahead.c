/*
  (C) 2006 Z RESEARCH Inc. <http://www.zresearch.com>
  
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2 of
  the License, or (at your option) any later version.
    
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.
    
  You should have received a copy of the GNU General Public
  License along with this program; if not, write to the Free
  Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301 USA
*/ 

/* 
   TODO:
   - handle O_DIRECT
   - maintain offset, flush on lseek
   - ensure efficient memory managment in case of random seek
   - ra_page_fault
*/

#include "glusterfs.h"
#include "logging.h"
#include "dict.h"
#include "xlator.h"
#include "read-ahead.h"
#include <assert.h>

static void
read_ahead (call_frame_t *frame,
	    ra_file_t *file);

static int32_t
ra_read_cbk (call_frame_t *frame,
	     call_frame_t *prev_frame,
	     xlator_t *this,
	     int32_t op_ret,
	     int32_t op_errno,
	     char *buf);

static int32_t
ra_open_cbk (call_frame_t *frame,
	     call_frame_t *prev_frame,
	     xlator_t *this,
	     int32_t op_ret,
	     int32_t op_errno,
	     dict_t *file_ctx,
	     struct stat *buf)
{
  ra_local_t *local = frame->local;
  ra_conf_t *conf = this->private;

  if (op_ret != -1) {
    ra_file_t *file = calloc (1, sizeof (*file));

    file->file_ctx = file_ctx;
    file->filename = strdup (local->filename);

    dict_set (file_ctx,
	      this->name,
	      int_to_data ((long) ra_file_ref (file)));

    file->offset = (unsigned long long) -1;
    file->size = 0;
    file->conf = conf;
    file->pages.next = &file->pages;
    file->pages.prev = &file->pages;
    file->pages.offset = (unsigned long) -1;
    file->pages.file = file;

    file->next = conf->files.next;
    conf->files.next = file;
    file->next->prev = file;
    file->prev = &conf->files;

    read_ahead (frame, file);
  }

  free (local->filename);
  free (local);
  frame->local = NULL;

  STACK_UNWIND (frame, op_ret, op_errno, file_ctx, buf);

  return 0;
}

static int32_t
ra_open (call_frame_t *frame,
	 xlator_t *this,
	 const char *pathname,
	 int32_t flags,
	 mode_t mode)
{
  ra_local_t *local = calloc (1, sizeof (*local));

  local->mode = mode;
  local->flags = flags;
  local->filename = strdup (pathname);
  frame->local = local;

  STACK_WIND (frame,
	      ra_open_cbk,
	      this->first_child,
	      this->first_child->fops->open,
	      pathname,
	      flags,
	      mode);

  return 0;
}

static int32_t
ra_create (call_frame_t *frame,
	   xlator_t *this,
	   const char *pathname,
	   mode_t mode)
{
  ra_local_t *local = calloc (1, sizeof (*local));

  local->mode = mode;
  local->flags = 0;
  local->filename = strdup (pathname);
  frame->local = local;

  STACK_WIND (frame,
	      ra_open_cbk,
	      this->first_child,
	      this->first_child->fops->create,
	      pathname,
	      mode);

  return 0;
}

/* free cache pages between offset and offset+size,
   does not touch pages with frames waiting on it
*/
static void
flush_region (call_frame_t *frame,
	      ra_file_t *file,
	      off_t offset,
	      size_t size)
{
  ra_page_t *trav = file->pages.next;

  while (trav != &file->pages && trav->offset < (offset + size)) {
    ra_page_t *next = trav->next;
    if (trav->offset >= offset && !trav->waitq) {
      trav->prev->next = trav->next;
      trav->next->prev = trav->prev;

      ra_purge_page (trav);
    }
    trav = next;
  }
}

static int32_t
ra_release_cbk (call_frame_t *frame,
		call_frame_t *prev_frame,
		xlator_t *this,
		int32_t op_ret,
		int32_t op_errno)
{
  frame->local = NULL;
  STACK_UNWIND (frame, op_ret, op_errno);
  return 0;
}

static int32_t
ra_release (call_frame_t *frame,
	    xlator_t *this,
	    dict_t *file_ctx)
{
  ra_file_t *file;

  file = (void *) ((long) data_to_int (dict_get (file_ctx,
						 this->name)));

  flush_region (frame, file, 0, file->pages.next->offset);
  dict_del (file_ctx, this->name);

  ra_file_unref (file);
  file->file_ctx = NULL;

  STACK_WIND (frame,
	      ra_release_cbk,
	      this->first_child,
	      this->first_child->fops->release,
	      file_ctx);
  return 0;
}


static int32_t
ra_read_cbk (call_frame_t *frame,
	     call_frame_t *prev_frame,
	     xlator_t *this,
	     int32_t op_ret,
	     int32_t op_errno,
	     char *buf)
{
  ra_local_t *local = frame->local;
  off_t pending_offset = local->pending_offset;
  off_t pending_size = local->pending_size;
  ra_file_t *file = local->file;
  ra_conf_t *conf = file->conf;
  ra_page_t *trav;
  off_t trav_offset;
  size_t payload_size;


  trav_offset = pending_offset;  
  payload_size = op_ret;

  if (op_ret < 0) {
    while (trav_offset < (pending_offset + pending_size)) {
      trav = ra_get_page (file, trav_offset);
      if (trav)
	ra_error_page (trav, op_ret, op_errno);
      trav_offset += conf->page_size;
    }
  } else {
    /* read region */
    while (trav_offset < (pending_offset + payload_size)) {
      trav = ra_get_page (file, trav_offset);
      if (!trav) {
	/* page was flushed */
	/* some serious bug ? */
	//	trav = ra_create_page (file, trav_offset);
	gf_log ("read-ahead",
		GF_LOG_DEBUG,
		"wasted copy: %lld[+%d]", trav_offset, conf->page_size);
	trav_offset += conf->page_size;
	continue;
      }
      if (!trav->ptr) {
	if (!frame->root->reply) {
	  trav->ptr = malloc (conf->page_size);
	  memcpy (trav->ptr,
		  &buf[trav_offset-pending_offset],
		  min (pending_offset+payload_size-trav_offset,
		       conf->page_size));
	} else {
	  trav->ptr = &buf[trav_offset-pending_offset];
	  trav->ref = dict_ref (frame->root->reply);
	}
	trav->ready = 1;
	trav->size = min (pending_offset+payload_size-trav_offset,
			  conf->page_size);
      }

      if (trav->waitq) {
	ra_wakeup_page (trav);
      }
      trav_offset += conf->page_size;
    }

    /* region which was not copied, (beyond end of file)
       flush to avoid -ve cache */
    while (trav_offset < (pending_offset + pending_size)) {
      trav = ra_get_page (file, trav_offset);
      if (trav) {
	/* some serious bug */

      //      if (trav->waitq)
	trav->size = 0;
	trav->ready = 1;
	if (trav->waitq) {
	  gf_log ("read-ahead",
		  GF_LOG_DEBUG,
		  "waking up from the left");
	  ra_wakeup_page (trav);
	}
	//	ra_flush_page (trav);
      }
      trav_offset += conf->page_size;
    }
  }

  ra_file_unref (local->file);
  free (frame->local);
  frame->local = NULL;
  STACK_DESTROY (frame->root);
  return 0;
}

static void
read_ahead (call_frame_t *frame,
	    ra_file_t *file)
{
  ra_local_t *local = frame->local;
  ra_conf_t *conf = file->conf;

  off_t ra_offset;
  size_t ra_size;
  off_t trav_offset;
  ra_page_t *trav = NULL;


  /*  ra_offset = roof (local->offset + local->size, conf->page_size);
  ra_size = roof (local->size, conf->page_size);
  */

  ra_size = conf->page_size * conf->page_count;
  ra_offset = floor (local->offset, conf->page_size);
  while (ra_offset < (local->offset + ra_size)) {
    trav = ra_get_page (file, ra_offset);
    if (!trav)
      break;
    ra_offset += conf->page_size;
  }

  if (trav)
    /* comfortable enough */
    return;

  trav_offset = ra_offset;

  trav = file->pages.next;

  while (trav_offset < (ra_offset + ra_size)) {
    /*
    while (trav != &file->pages && trav->offset < trav_offset)
      trav = trav->next;

    if (trav->offset != trav_offset) {
    */
    trav = ra_get_page (file, trav_offset);
    if (!trav) {
      trav = ra_create_page (file, trav_offset);

      call_frame_t *ra_frame = copy_frame (frame);
      ra_local_t *ra_local = calloc (1, sizeof (ra_local_t));
    
      ra_frame->local = ra_local;
      ra_local->pending_offset = trav->offset;
      ra_local->pending_size = conf->page_size;
      ra_local->file = ra_file_ref (file);

      /*
      gf_log ("read-ahead",
	      GF_LOG_DEBUG,
	      "RA: %lld[+%d]", trav_offset, conf->page_size); 
      */
      STACK_WIND (ra_frame,
		  ra_read_cbk,
		  ra_frame->this->first_child,
		  ra_frame->this->first_child->fops->read,
		  file->file_ctx,
		  conf->page_size,
		  trav_offset);
    }
    trav_offset += conf->page_size;
  }
  return ;
}

static void
dispatch_requests (call_frame_t *frame,
		   ra_file_t *file)
{
  ra_local_t *local = frame->local;
  ra_conf_t *conf = file->conf;
  off_t rounded_offset;
  off_t rounded_end;
  off_t trav_offset;
  /*
  off_t dispatch_offset;
  size_t dispatch_size;
  char dispatch_found = 0;
  */
  ra_page_t *trav;

  rounded_offset = floor (local->offset, conf->page_size);
  rounded_end = roof (local->offset + local->size, conf->page_size);

  trav_offset = rounded_offset;

  /*
  dispatch_offset = 0;
  dispatch_size = 0;
  */

  trav = file->pages.next;

  while (trav_offset < rounded_end) {
    /*
    while (trav != &file->pages && trav->offset < trav_offset)
      trav = trav->next;
    if (trav->offset != trav_offset) {
      ra_page_t *newpage = calloc (1, sizeof (*trav));
      newpage->offset = trav_offset;
      newpage->file = file;
      newpage->next = trav;
      newpage->prev = trav->prev;
      newpage->prev->next = newpage;
      newpage->next->prev = newpage;
      trav = newpage;
    */
    trav = ra_get_page (file, trav_offset);
    if (!trav) {
      trav = ra_create_page (file, trav_offset);

      call_frame_t *worker_frame = copy_frame (frame);
      ra_local_t *worker_local = calloc (1, sizeof (ra_local_t));

      /*
      gf_log ("read-ahead",
	      GF_LOG_DEBUG,
	      "MISS: region: %lld[+%d]", trav_offset, conf->page_size);
      */
      worker_frame->local = worker_local;
      worker_local->pending_offset = trav_offset;
      worker_local->pending_size = conf->page_size;
      worker_local->file = ra_file_ref (file);

      STACK_WIND (worker_frame,
		  ra_read_cbk,
		  worker_frame->this->first_child,
		  worker_frame->this->first_child->fops->read,
		  file->file_ctx,
		  conf->page_size,
		  trav_offset);

    /*
      if (!dispatch_found) {
	dispatch_found = 1;
	dispatch_offset = trav_offset;
      }
      dispatch_size = (trav->offset - dispatch_offset + conf->page_size);
    */
    }
    if (trav->ready) {
      ra_fill_frame (trav, frame);
    } else {
      /*
      gf_log ("read-ahead",
	      GF_LOG_DEBUG,
	      "Might catch...?");
      */
      ra_wait_on_page (trav, frame);
    }

    trav_offset += conf->page_size;
  }

  /*
  if (dispatch_found) {
    call_frame_t *worker_frame = copy_frame (frame);
    ra_local_t *worker_local = calloc (1, sizeof (ra_local_t));
    
    gf_log ("read-ahead",
	    GF_LOG_DEBUG,
	    "MISS: region: %lld[+%d]", dispatch_offset, dispatch_size);
    worker_frame->local = worker_local;
    worker_local->pending_offset = dispatch_offset;
    worker_local->pending_size = dispatch_size;
    worker_local->file = ra_file_ref (file);

    STACK_WIND (worker_frame,
		ra_read_cbk,
		worker_frame->this->first_child,
		worker_frame->this->first_child->fops->read,
		file->file_ctx,
		dispatch_size,
		dispatch_offset);
  }
  */
  return ;
}


static int32_t
ra_read (call_frame_t *frame,
	 xlator_t *this,
	 dict_t *file_ctx,
	 size_t size,
	 off_t offset)
{
  /* TODO: do something about atime update on server */
  ra_file_t *file;
  ra_local_t *local;
  ra_conf_t *conf;

  /*
  gf_log ("read-ahead",
	  GF_LOG_DEBUG,
	  "read: %lld[+%d]", offset, size);
  */
  file = (void *) ((long) data_to_int (dict_get (file_ctx,
						 this->name)));
  conf = file->conf;

  local = (void *) calloc (1, sizeof (*local));
  local->ptr = calloc (size, 1);
  local->offset = offset;
  local->size = size;
  local->file = ra_file_ref (file);
  local->wait_count = 1; /* for synchronous STACK_UNWIND from protocol
			    in case of error */
  frame->local = local;

  dispatch_requests (frame, file);

  flush_region (frame, file, 0, floor (offset, conf->page_size));

  read_ahead (frame, file);

  local->wait_count--;
  if (!local->wait_count) {
    /*     gf_log ("read-ahead",
	    GF_LOG_DEBUG,
	    "HIT for %lld[+%d]", offset, size); 
    */
    /* CACHE HIT */
    frame->local = NULL;
    STACK_UNWIND (frame, local->op_ret, local->op_errno, local->ptr);
    ra_file_unref (local->file);
    if (!local->is_static)
      free (local->ptr);
    free (local);
  } else {
    /*
    gf_log ("read-ahead",
	    GF_LOG_DEBUG,
	    "ALMOST HIT for %lld[+%d]", offset, size);
    */
    /* ALMOST HIT (read-ahead data already on way) */
  }

  return 0;
}

static int32_t
ra_flush_cbk (call_frame_t *frame,
	      call_frame_t *prev_frame,
	      xlator_t *this,
	      int32_t op_ret,
	      int32_t op_errno)
{
  STACK_UNWIND (frame, op_ret, op_errno);
  return 0;
}


static int32_t
ra_flush (call_frame_t *frame,
	  xlator_t *this,
	  dict_t *file_ctx)
{
  ra_file_t *file;

  file = (void *) ((long) data_to_int (dict_get (file_ctx,
						 this->name)));
  flush_region (frame, file, 0, file->pages.next->offset);

  STACK_WIND (frame,
	      ra_flush_cbk,
	      this->first_child,
	      this->first_child->fops->flush,
	      file_ctx);
  return 0;
}

static int32_t
ra_fsync (call_frame_t *frame,
	  xlator_t *this,
	  dict_t *file_ctx,
	  int32_t datasync)
{
  ra_file_t *file;

  file = (void *) ((long) data_to_int (dict_get (file_ctx,
						 this->name)));
  flush_region (frame, file, 0, file->pages.next->offset);

  STACK_WIND (frame,
	      ra_flush_cbk,
	      this->first_child,
	      this->first_child->fops->fsync,
	      file_ctx,
	      datasync);
  return 0;
}

static int32_t
ra_write_cbk (call_frame_t *frame,
	      call_frame_t *prev_frame,
	      xlator_t *this,
	      int32_t op_ret,
	      int32_t op_errno)
{
  STACK_UNWIND (frame, op_ret, op_errno);
  return 0;
}

static int32_t
ra_write (call_frame_t *frame,
	  xlator_t *this,
	  dict_t *file_ctx,
	  char *buf,
	  size_t size,
	  off_t offset)
{
  ra_file_t *file;

  file = (void *) ((long) data_to_int (dict_get (file_ctx,
						 this->name)));

  flush_region (frame, file, 0, file->pages.prev->offset);

  STACK_WIND (frame,
	      ra_write_cbk,
	      this->first_child,
	      this->first_child->fops->write,
	      file_ctx,
	      buf,
	      size,
	      offset);

  return 0;
}

int32_t 
init (struct xlator *this)
{
  ra_conf_t *conf;
  dict_t *options = this->options;

  if (!this->first_child || this->first_child->next_sibling) {
    gf_log ("read-ahead",
	    GF_LOG_ERROR,
	    "FATAL: read-ahead not configured with exactly one child");
    return -1;
  }

  conf = (void *) calloc (1, sizeof (*conf));
  conf->page_size = 1024 * 64;
  conf->page_count = 16;

  if (dict_get (options, "page-size")) {
    conf->page_size = data_to_int (dict_get (options,
					     "page-size"));
    gf_log ("read-ahead",
	    GF_LOG_DEBUG,
	    "Using conf->page_size = 0x%x",
	    conf->page_size);
  }

  if (dict_get (options, "page-count")) {
    conf->page_count = data_to_int (dict_get (options,
					      "page-count"));
    gf_log ("read-ahead",
	    GF_LOG_DEBUG,
	    "Using conf->page_count = 0x%x",
	    conf->page_count);
  }

  conf->files.next = &conf->files;
  conf->files.prev = &conf->files;

  /*
  conf->cache_block = malloc (conf->page_size * conf->page_count);
  conf->pages = (void *) calloc (conf->page_count,
				 sizeof (struct ra_page));

  {
    int i;

    for (i=0; i<conf->page_count; i++) {
      if (i < (conf->page_count - 2))
      	conf->pages[i].next = &conf->pages[i+1];
      conf->pages[i].ptr = conf->cache_block + (i * conf->page_size);
    }
  }
  */
  this->private = conf;
  return 0;
}

void
fini (struct xlator *this)
{
  ra_conf_t *conf = this->private;

  //  free (conf->cache_block);
  //  free (conf->pages);
  free (conf);

  this->private = NULL;
  return;
}

struct xlator_fops fops = {
  .open        = ra_open,
  .create      = ra_create,
  .read        = ra_read,
  .write       = ra_write,
  .flush       = ra_flush,
  .fsync       = ra_fsync,
  .release     = ra_release,
};

struct xlator_mops mops = {
};