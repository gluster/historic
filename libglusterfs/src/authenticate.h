#include "dict.h"

typedef enum {
  AUTH_ACCEPT,
  AUTH_REJECT,
  AUTH_DONT_CARE
} authenticate_t;

authenticate_t authenticate (dict_t *input_params, dict_t *config_params);

