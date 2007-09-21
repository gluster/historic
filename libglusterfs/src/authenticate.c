#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <fnmatch.h>
#include "authenticate.h"

#define IP_DELIMITER " ,"

authenticate_t authenticate_user (dict_t *input_params, dict_t *config_params);
authenticate_t authenticate_ip (dict_t *input_params, dict_t *config_params);

authenticate_t authenticate (dict_t *input_params, dict_t *config_params) 
{
  char *peer = NULL;
  int user_result = -1, ip_result = -1;

  peer = data_to_str (dict_get (input_params, "peer"));
  if (!peer) {
    gf_log ("authenticate",
	    GF_LOG_ERROR,
	    "Peer information not found\n");
    return AUTH_REJECT;
  }

  user_result = authenticate_user (input_params, config_params);
  if (user_result == AUTH_REJECT)
    return user_result;

  ip_result = authenticate_ip (input_params, config_params);
  if (ip_result == AUTH_REJECT)
    return ip_result;

  if (AUTH_DONT_CARE == ip_result && AUTH_DONT_CARE == user_result)
    return AUTH_REJECT;

  return AUTH_ACCEPT;
} 

authenticate_t authenticate_ip (dict_t *input_params, dict_t *config_params)
{
  char *name = NULL;
  char *searchstr = NULL;
  data_t *allow_ip = NULL, *reject_ip = NULL;
  char *peer = NULL;

  name = data_to_str (dict_get (input_params, "remote-subvolume"));
  if (!name) {
    gf_log ("authenticate/ip",
	    GF_LOG_ERROR,
	    "remote-subvolume not specified");
    return AUTH_REJECT;
  }

  asprintf (&searchstr, "auth.ip.%s.allow", name);
  allow_ip = dict_get (config_params,
		       searchstr);
  free (searchstr);

  peer = data_to_str (dict_get (input_params, "peer"));
  if (!peer) {
    gf_log ("authenticate/ip",
	    GF_LOG_ERROR,
	    "peer not specified");
    return AUTH_REJECT;
  }

  if (allow_ip) {
    char *ip_addr_str = NULL;
    char *tmp;
    char *ip_addr_cpy = strdup (allow_ip->data);
    
    ip_addr_str = strtok_r (ip_addr_cpy, IP_DELIMITER, &tmp);
      
    while (ip_addr_str) {
      char negate = 0, match = 0;
         gf_log (name,  GF_LOG_DEBUG,
	      "allowed = \"%s\", received ip addr = \"%s\"",
	      ip_addr_str, peer);
      if (ip_addr_str[0] == '!') {
	negate = 1;
	ip_addr_str++;
      }

      match = fnmatch (ip_addr_str,
		       peer,
		       0);

      if (negate ? !match : match) {
	free (ip_addr_cpy);
	return AUTH_ACCEPT;
      }
      ip_addr_str = strtok_r (NULL, IP_DELIMITER, &tmp);
    }
    free (ip_addr_cpy);
  }      
  
  asprintf (&searchstr, "auth.ip.%s.reject", name);
  reject_ip = dict_get (config_params,
			searchstr);
  free (searchstr);
  
  if (reject_ip) {
    char *ip_addr_str = NULL;
    char *tmp;
    char *ip_addr_cpy = strdup (reject_ip->data);
      
    ip_addr_str = strtok_r (ip_addr_cpy, IP_DELIMITER, &tmp);
    
    while (ip_addr_str) {
      char negate = 0,  match =0;
      gf_log (name,  GF_LOG_DEBUG,
	      "rejected = \"%s\", received ip addr = \"%s\"",
	      ip_addr_str, peer);
      if (ip_addr_str[0] == '!') {
	negate = 1;
	ip_addr_str++;
      }

      match = fnmatch (ip_addr_str,
		       peer,
		       0);
      if (negate ? !match : match) {
	free (ip_addr_cpy);
	return AUTH_REJECT;
      }
      ip_addr_str = strtok_r (NULL, IP_DELIMITER, &tmp);
    }
    free (ip_addr_cpy);
  }      
  
  return AUTH_DONT_CARE;
}

authenticate_t authenticate_user (dict_t *input_params, dict_t *config_params)
{
  char *username = NULL, *password = NULL;
  char *username_password = NULL;
  char *user_configuration = NULL;
  char *saveptr = NULL;

  username = data_to_str (dict_get (input_params, "username"));
  password = data_to_str (dict_get (input_params, "password"));

  if (!username)
    return AUTH_DONT_CARE;

  user_configuration = data_to_str (dict_get (config_params, "auth.username.password"));
  if (!user_configuration) {
    gf_log ("authenticate/username-password",
	    GF_LOG_ERROR,
	    "user configuration not provided");
    return AUTH_REJECT;
  }

  username_password = strtok_r (user_configuration, ",", &saveptr);
  while (username_password) {
    char *config_username = NULL, *config_password = NULL;
    char *inner_saveptr = NULL;
    
    config_username = strtok_r (username_password, ":", &inner_saveptr);
    config_password = strtok_r (NULL, ":", &inner_saveptr);

    if (!strcmp (username, config_username)) {
      if (!strcmp (password, config_password))
	return AUTH_ACCEPT;
      else
	return AUTH_REJECT;
    }
    
    username_password = strtok_r (NULL, ",", &saveptr);
  }

  return AUTH_REJECT;
}
