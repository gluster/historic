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

/*
  This file defines MACROS and static inlines used to emulate a function
  call over asynchronous communication with remote server
*/

#ifndef _STACK_H
#define _STACK_H

#ifndef _CONFIG_H
#define _CONFIG_H
#include "config.h"
#endif

struct _call_stack_t;
typedef struct _call_stack_t call_stack_t;
struct _call_frame_t;
typedef struct _call_frame_t call_frame_t;
struct _call_pool_t;
typedef struct _call_pool_t call_pool_t;

#include "xlator.h"
#include "dict.h"
#include "list.h"
#include "common-utils.h"


typedef int32_t (*ret_fn_t) (call_frame_t *frame,
			     call_frame_t *prev_frame,
			     xlator_t *this,
			     int32_t op_ret,
			     int32_t op_errno,
			     ...);

struct _call_pool_t {
	union {
		struct list_head   all_frames;
		struct {
			call_stack_t *next_call;
			call_stack_t *prev_call;
		} all_stacks;
	};
	int64_t                     cnt;
	gf_lock_t                   lock;
};

struct _call_frame_t {
	call_stack_t *root;        /* stack root */
	call_frame_t *parent;      /* previous BP */
	call_frame_t *next;
	call_frame_t *prev;        /* maintainence list */
	void         *local;       /* local variables */
	xlator_t     *this;        /* implicit object */
	ret_fn_t      ret;         /* op_return address */
	int32_t       ref_count;
	gf_lock_t     lock;
	void         *cookie;      /* unique cookie */
};

struct _call_stack_t {
	union {
		struct list_head      all_frames;
		struct {
			call_stack_t *next_call;
			call_stack_t *prev_call;
		};
	};
	call_pool_t                  *pool;
	void                         *trans;
	uint64_t                      unique;
	void                         *state;  /* pointer to request state */
	uid_t                         uid;
	gid_t                         gid;
	pid_t                         pid;
	call_frame_t                  frames;
	dict_t                       *req_refs;
	dict_t                       *rsp_refs;

	int32_t                       op;
	int8_t                        type;
};


static inline void
FRAME_DESTROY (call_frame_t *frame)
{
	if (frame->next)
		frame->next->prev = frame->prev;
	if (frame->prev)
		frame->prev->next = frame->next;
	if (frame->local)
		FREE (frame->local);
	LOCK_DESTROY (&frame->lock);
	FREE (frame);
}


static inline void
STACK_DESTROY (call_stack_t *stack)
{
	LOCK (&stack->pool->lock);
	{
		list_del_init (&stack->all_frames);
		stack->pool->cnt--;
	}
	UNLOCK (&stack->pool->lock);

	if (stack->frames.local)
		FREE (stack->frames.local);

	LOCK_DESTROY (&stack->frames.lock);

	while (stack->frames.next) {
		FRAME_DESTROY (stack->frames.next);
	}
	FREE (stack);
}


#define cbk(x) cbk_##x


/* make a call */
#define STACK_WIND(frame, rfn, obj, fn, params ...)			\
	do {								\
		call_frame_t *_new = NULL;				\
		                                                        \
                _new = CALLOC (1, sizeof (call_frame_t));	        \
		ERR_ABORT (_new);					\
		typeof(fn##_cbk) tmp_cbk = rfn;				\
		_new->root = frame->root;				\
		_new->next = frame->root->frames.next;			\
		_new->prev = &frame->root->frames;			\
		if (frame->root->frames.next)				\
			frame->root->frames.next->prev = _new;		\
		frame->root->frames.next = _new;			\
		_new->this = obj;					\
		_new->ret = (ret_fn_t) tmp_cbk;				\
		_new->parent = frame;					\
		_new->cookie = _new;					\
		LOCK_INIT (&_new->lock);				\
		frame->ref_count++;					\
									\
		fn (_new, obj, params);					\
	} while (0)


/* make a call with a cookie */
#define STACK_WIND_COOKIE(frame, rfn, cky, obj, fn, params ...)		\
	do {								\
		call_frame_t *_new = CALLOC (1,				\
					     sizeof (call_frame_t));	\
		ERR_ABORT (_new);					\
		typeof(fn##_cbk) tmp_cbk = rfn;				\
		_new->root = frame->root;				\
		_new->next = frame->root->frames.next;			\
		_new->prev = &frame->root->frames;			\
		if (frame->root->frames.next)				\
			frame->root->frames.next->prev = _new;		\
		frame->root->frames.next = _new;			\
		_new->this = obj;					\
		_new->ret = (ret_fn_t) tmp_cbk;				\
		_new->parent = frame;					\
		_new->cookie = cky;					\
		LOCK_INIT (&_new->lock);				\
		frame->ref_count++;					\
		fn##_cbk = rfn;						\
									\
		fn (_new, obj, params);					\
	} while (0)


/* return from function */
#define STACK_UNWIND(frame, params ...)					\
	do {								\
		ret_fn_t fn = frame->ret;				\
		call_frame_t *_parent = frame->parent;			\
		_parent->ref_count--;					\
		fn (_parent, frame->cookie, _parent->this, params);	\
	} while (0)


static inline call_frame_t *
copy_frame (call_frame_t *frame)
{
	call_stack_t *newstack = NULL;
	call_stack_t *oldstack = NULL;

	if (!frame) {
		return NULL;
	}

	newstack = (void *) CALLOC (1, sizeof (*newstack));
	oldstack = frame->root;

	newstack->uid = oldstack->uid;
	newstack->gid = oldstack->gid;
	newstack->pid = oldstack->pid;
	newstack->unique = oldstack->unique;

	newstack->frames.this = frame->this;
	newstack->frames.root = newstack;
	newstack->pool = oldstack->pool;

	LOCK_INIT (&newstack->frames.lock);

	LOCK (&oldstack->pool->lock);
	{
		list_add (&newstack->all_frames, &oldstack->all_frames);
		newstack->pool->cnt++;
		
	}
	UNLOCK (&oldstack->pool->lock);

	return &newstack->frames;
}

static inline call_frame_t *
create_frame (xlator_t *xl, call_pool_t *pool)
{
	call_stack_t *stack = NULL;

	if (!xl || !pool) {
		return NULL;
	}

	stack = CALLOC (1, sizeof (*stack));
	if (!stack)
		return NULL;

	stack->pool = pool;
	stack->frames.root = stack;
	stack->frames.this = xl;

	LOCK (&pool->lock);
	{
		list_add (&stack->all_frames, &pool->all_frames);
		pool->cnt++;
	}
	UNLOCK (&pool->lock);

	LOCK_INIT (&stack->frames.lock);

	return &stack->frames;
}


#endif /* _STACK_H */
