#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <dlfcn.h>
#include "authenticate.h"

static void
init (dict_t *this,
      char *key,
      data_t *value,
      void *data)
{
  void *handle = NULL;
  char *auth_file = NULL;
  auth_fn_t authenticate;

  asprintf (&auth_file, "%s/%s.so", LIBDIR, key);
  handle = dlopen (auth_file, RTLD_LAZY);
  if (!handle) {
    gf_log ("libglusterfs/",
	    GF_LOG_ERROR,
	    "dlopen(%s): %s\n", 
	    auth_file,
	    dlerror ());
    free (auth_file);
    return;
  }
  free (auth_file);
  
  authenticate = dlsym (handle, "auth");
  if (!authenticate) {
    gf_log ("libglusterfs/scheduler",
	    GF_LOG_ERROR,
	    "dlsym(authenticate) on %s\n", 
	    dlerror ());
    return;
  }

  dict_set (this, key, data_from_static_ptr (authenticate));
}

void
auth_init (dict_t *auth_modules)
{
  int32_t dummy;
  dict_foreach (auth_modules, init, &dummy);
}

auth_result_t authenticate (dict_t *input_params, dict_t *config_params, dict_t *auth_modules) 
{
  dict_t *results = NULL;
  int32_t result = AUTH_ACCEPT, auth_dont_care = 1;

  results = get_new_dict ();
  auto void map (dict_t *this,
		 char *key,
		 data_t *value,
		 void *data)
    {
      dict_t *res = data;
      auth_fn_t authenticate = data_to_ptr (value);
      dict_set (res, key, int_to_data (authenticate (input_params, config_params)));
    }

  dict_foreach (auth_modules, map, results);

  auto void reduce (dict_t *this,
		    char *key,
		    data_t *value,
		    void *data)
    {
      int32_t *res = data;
      if (AUTH_REJECT == data_to_int64 (value))
	*res = AUTH_REJECT;
      if (AUTH_DONT_CARE != data_to_int64 (value))
	auth_dont_care = 0;
    }

  dict_foreach (results, reduce, &result);
  if (auth_dont_care) {
    char *name = NULL;
    name = data_to_str (dict_get (input_params, "remote-subvolume"));
    gf_log ("auth",
	    GF_LOG_DEBUG,
	    "Nobody cares to authenticate!! Rejecting the client %s", name);
    result = AUTH_REJECT;
  }
    
  dict_destroy (results);
  return result;
}
