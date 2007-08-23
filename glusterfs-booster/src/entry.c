/*
   Copyright (c) 2007 Z RESEARCH, Inc. <http://www.zresearch.com>
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

#include <dlfcn.h>
#include <sys/types.h>


/* open, open64, creat */
static int (*real_open) (const char *pathname, int flags, mode_t mode);
static int (*real_open64) (const char *pathname, int flags, mode_t mode);
static int (*real_creat) (const char *pathname, mode_t mode);

/* read, readv, pread, pread64 */
/* write, writev, pwrite, pwrite64 */
/* lseek, llseek, lseek64 */
/* close */
/* dup dup2 */


int
open (const char *pathname, int flags, mode_t mode)
{
  int ret;

  if (!real_open)
    {
      real_open = dlsym (RTLD_NEXT, "open");
    }

  ret = real_open (pathname, flags, mode);

  return ret;
}


int
open64 (const char *pathname, int flags, mode_t mode)
{
  int ret;

  if (!real_open64)
    {
      real_open64 = dlsym (RTLD_NEXT, "open64");
    }

  ret = real_open64 (pathname, flags, mode);

  return ret;
}


int
creat (const char *pathname, mode_t mode)
{
  int ret;

  if (!real_creat)
    {
      real_creat = dlsym (RTLD_NEXT, "creat");
    }

  ret = real_creat (pathname, mode);

  return ret;
}
