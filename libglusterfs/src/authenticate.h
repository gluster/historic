#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <fnmatch.h>
#include "dict.h"
#include "list.h"

typedef enum {
  AUTH_ACCEPT,
  AUTH_REJECT,
  AUTH_DONT_CARE
} auth_result_t;

typedef auth_result_t (*auth_fn_t) (dict_t *input_params, dict_t *config_params);

auth_result_t authenticate (dict_t *input_params, dict_t *config_params, dict_t *auth_modules);
void auth_init (dict_t *auth_modules);
