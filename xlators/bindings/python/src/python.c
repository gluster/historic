/*
   Copyright (c) 2007 Chris AtLee <chris@atlee.ca>
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
#include <Python.h>

#include "glusterfs.h"
#include "xlator.h"
#include "logging.h"
#include "defaults.h"

static PyObject *pGlusterModule = 0;

typedef struct
{
    char        *scriptname;
    PyObject    *pGlobals, *pLocals;
    PyObject    *pXlator;
} python_private_t;

/*
 * This macro creates converter functions for various gluster types.
 * name is the prefix of the function name
 * ctype is the C type
 * pytype is the name (a quoted string) of one of the types in
 *      glustertypes.py
 * The converted function will call pytype.from_address(ptr) to
 * construct a python representation of the object at the given address
 *
 * TODO: We're probably leaking references here.  Need to Py_DECREF
 *       retval at some point?
 */
#define TypeConverter(name, ctype, pytype) \
static PyObject * \
name##_converter (ctype *ptr) \
{ \
    PyObject *py_type = PyObject_GetAttrString( \
                            pGlusterModule, pytype); \
    PyObject *retval = PyObject_CallMethod(py_type, "from_address", \
            "O&", PyLong_FromVoidPtr, ptr); \
    Py_DECREF(py_type); \
    return retval; \
}

TypeConverter(frame, call_frame_t, "call_frame_t");
TypeConverter(vector, struct iovec, "iovec");
TypeConverter(fd, fd_t, "fd_t");

static int32_t
python_writev (call_frame_t *frame,
              xlator_t *this,
              fd_t *fd,
              struct iovec *vector,
              int32_t count, 
              off_t offset)
{
  python_private_t *priv = (python_private_t *)this->private;
  gf_log("python", GF_LOG_DEBUG, "In writev");
  if (PyObject_HasAttrString(priv->pXlator, "writev"))
  {
      PyObject *retval = PyObject_CallMethod(priv->pXlator, "writev",
              "O& O& O& i l",
              frame_converter, frame,
              fd_converter, fd,
              vector_converter, vector,
              count,
              offset);
      if (PyErr_Occurred())
      {
          PyErr_Print();
      }
      Py_XDECREF(retval);
  }
  else
  {
      return default_writev(frame, this, fd, vector, count, offset);
  }
  return 0;
}

struct xlator_fops fops = {
    .writev       = python_writev
};

struct xlator_mops mops = {
};

int32_t
init (xlator_t *this)
{
  Py_InitializeEx(0);

  if (!this->children) {
    gf_log ("python", GF_LOG_ERROR, 
            "FATAL: python should have exactly one child");
    return -1;
  }

  python_private_t *priv = calloc (sizeof (python_private_t), 1);

  data_t *scriptname = dict_get (this->options, "scriptname");
  if (scriptname) {
      priv->scriptname = data_to_str(scriptname);
  } else {
      gf_log("python", GF_LOG_ERROR,
              "FATAL: python requires the scriptname parameter");
      return -1;
  }

  gf_log("python", GF_LOG_DEBUG,
          "Importing module...%s", priv->scriptname);
  
  priv->pGlobals = 0;
  priv->pLocals = 0;

  pGlusterModule = PyImport_ImportModule(priv->scriptname);
  if (!pGlusterModule)
  {
      gf_log("python", GF_LOG_ERROR, "Error loading %s", priv->scriptname);
      PyErr_Print();
      return -1;
  }

  if (!PyObject_HasAttrString(pGlusterModule, "xlator"))
  {
      gf_log("python", GF_LOG_ERROR, "%s does not have a xlator attribute", priv->scriptname);
      return -1;
  }
  gf_log("python", GF_LOG_DEBUG, "Instantiating translator");
  priv->pXlator = PyObject_CallMethod(pGlusterModule, "xlator", "O&",
          PyLong_FromVoidPtr, this);
  if (PyErr_Occurred() || !priv->pXlator)
  {
      PyErr_Print();
      return -1;
  }

  this->private = priv;

  gf_log ("python", GF_LOG_DEBUG, "python xlator loaded");
  return 0;
}

void 
fini (xlator_t *this)
{
  return;
}
