/*
  (C) 2006 Vikas Gorur <vikas@80x25.org>
  
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
 * gping: Gluster ping
 */

#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <argp.h>

/* arguments */
static char *input_file;
static char *host;
static char *msg = "ping";
static int gping_port = 2471;
static char only_count = 0;

/* Hash table to keep track of which hosts have replied */
struct host_status {
  in_addr_t host;
  char origname[256];
  int replied;
  char reply[256];
  struct host_status *next; /* next in chain */
};

static struct host_status *host_status[2047];

static unsigned int host_status_hash (in_addr_t addr)
{
  return addr % 2047;
}

struct host_status *lookup (in_addr_t host)
{
  struct host_status *st;
  
  for (st = host_status [host_status_hash (host)];
       st != NULL;
       st = st->next)
    if (st->host == host)
      return st;

  return NULL;
}

char *get_reply (in_addr_t host)
{
  struct host_status *st = lookup (host);
  if (st)
    return st->reply;
  return NULL;
}

void set_reply (in_addr_t host, char *reply)
{
  struct host_status *st = lookup (host);
  
  if (st == NULL) {
    struct host_status *new = calloc (1,sizeof (struct host_status));
    struct host_status *head = host_status [host_status_hash (host)];

    new->host = host;
    new->replied = 1;
    strcpy (new->reply, reply);
    if (new->reply[strlen (new->reply) - 1] == '\n')
      new->reply[strlen (new->reply) -1] = 0;

    host_status [host_status_hash (host)] = new;
    new->next = head;
  }
  else {
    strcpy (st->reply, reply);
    if (st->reply[strlen (st->reply) - 1] == '\n')
      st->reply[strlen (st->reply) -1] = 0;
    st->replied = 1;
  }
}

char *get_original_name (in_addr_t host)
{
  struct host_status *st = lookup (host);
  if (st)
    return st->origname;
  return NULL;
}

void set_original_name (in_addr_t host, char *name)
{
  struct host_status *st = lookup (host);
  
  if (st == NULL) {
    struct host_status *new = malloc (sizeof (struct host_status));
    struct host_status *head = host_status [host_status_hash (host)];

    new->host = host;
    strcpy (new->origname, name);

    host_status [host_status_hash (host)] = new;
    new->next = head;
  }
  else {
    strcpy (st->origname, name);
  }
}

static in_addr_t *hosts;

static in_addr_t lookup_host (const char *host)
{
  in_addr_t ret;
  ret = (in_addr_t) inet_addr (host);

  if (ret == INADDR_NONE) {
    struct hostent *addr = gethostbyname (host);
    if (!addr) {
      fprintf (stderr, "unknown host %s\n", host);
      return INADDR_NONE;
    }
    memcpy (&ret, addr->h_addr_list[0], 4);
  }

  return ret;
}

void load_hosts (const char *filename)
{
  FILE *fp = fopen (filename, "r");
  char line[256];
  char *p;
  int i = 0;
  char *nl;
  int hosts_capacity = 1024;
  hosts = calloc (sizeof (in_addr_t), hosts_capacity);

  if (!fp) {
    fprintf (stderr, "Error: %s: ", filename);
    perror (NULL);
    exit (1);
  }

  p = fgets (line, 256, fp);
  nl = strchr (line, '\n');
  if (nl)
    *nl = '\0';
  hosts[i] = lookup_host (line);
  set_original_name (hosts[i], line);

  while (p) {
    i++;
    if (i == hosts_capacity){
      hosts = realloc (hosts, hosts_capacity * 2 * sizeof (char *));
      hosts_capacity *= 2;
    }

    p = fgets (line, 256, fp);
    nl = strchr (line, '\n');
    if (nl)
      *nl = '\0';

    hosts[i] = lookup_host (line);
    set_original_name (hosts[i], line);
  }

  hosts[i] = INADDR_NONE;
  fclose (fp);
}

void ping (in_addr_t host, int sock, char *data)
{
  struct sockaddr_in peer;

  peer.sin_family = AF_INET;
  peer.sin_port = htons (gping_port);
  peer.sin_addr.s_addr = host;
    
  sendto (sock, data, strlen (data), 0, (struct sockaddr *)&peer, 
	  sizeof (peer));
}

void handle_replies (int sock, struct timeval *ts)
{
  char recvbuf[1024];
  struct sockaddr_in peer;
  int len;
  fd_set fs;

  socklen_t slen = sizeof (struct sockaddr_in);
  FD_ZERO (&fs);
  FD_SET (sock, &fs);

  while (select (sock+1, &fs, NULL, NULL, ts) > 0) {
    len = recvfrom (sock, recvbuf, 1024, 0, (struct sockaddr *)&peer, &slen);
    recvbuf[len] = '\0';
    set_reply (peer.sin_addr.s_addr, recvbuf);
  }
}

void ping_hosts (int sock, struct timeval *ts)
{
  int i;
  struct timeval zero;

  zero.tv_sec = 0;
  zero.tv_usec = 0;

  for (i = 0; hosts[i] != INADDR_NONE; i++) {
    if (!lookup (hosts[i])->replied)
      ping (hosts[i], sock, msg);

    handle_replies (sock, &zero);
  }

  handle_replies (sock, ts);
}

int everyone_has_replied (void)
{
  int i;
  for (i = 0; hosts[i] != INADDR_NONE; i++)
    if (!(lookup (hosts[i]))->replied)
      return 0;

  return 1;
}

/* Argument parsing */
static struct argp_option options[] = {
  {"input", 'i', "file", 0, "Read list of hosts to ping from file"},
  {"port", 'p', "number", 0, "Port number to send the ping to"},
  {"message", 'm', "msg", 0, "Message to send (default is 'ping')"},
  {"count", 'c', 0, 0, "Output only count of nodes which are responding"},
  { 0 }
};

static error_t parse_opts (int key, char *arg, struct argp_state *state)
{
  switch (key) {
  case 'i':
    input_file = arg;
    break;
  case 'p': {
    char *p = arg;
    while (*p)
      if (!isdigit (*p++)) {
	fprintf (stderr, "gping: Invalid port specification\n");
	exit (2);
      }
    gping_port = atoi (arg);
    break;
  }
  case 'm':
    msg = arg;
    break;
  case 'c':
    only_count = 1;
    break;
  case ARGP_KEY_ARG:
    if (state->arg_num >= 2)
      argp_usage (state);
    if (state->arg_num == 0)
      host = arg;
    if (state->arg_num == 1)
      msg = arg;
    
    break;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  
  return 0;
}

const char *argp_program_version = "1.0";
const char *argp_program_bug_address = "gluster-devel@nongnu.org";

static char doc[] = "gping: UDP based ping tool\n(C) 2006 ZRESEARCH";
static char args_doc[] = "host [msg]";

static struct argp argp = {options, parse_opts, args_doc, doc};

int main (int argc, char **argv)
{
  int all_replied = 0;

  argp_parse (&argp, argc, argv, 0, 0, 0);

  if (!(host || input_file)) {
    fprintf (stderr, "gping: Must specify hostname or input file\n");
    exit (127);
  }

  if (input_file)
    load_hosts (input_file);
  else {
    hosts = malloc (sizeof (char *));
    hosts[0] = lookup_host (host);
    set_original_name (hosts[0], host);
    hosts[1] = INADDR_NONE;
  }

  {
    int sock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct timeval timeout;

    fcntl (sock, F_SETFL, O_NONBLOCK);

    timeout.tv_sec = 0;
    timeout.tv_usec = 250000;
    ping_hosts (sock, &timeout);

    if (!everyone_has_replied ()) {
      timeout.tv_sec = 0;
      timeout.tv_usec = 500000;
      ping_hosts (sock, &timeout);
    }

    if (!everyone_has_replied ()) {
      timeout.tv_sec = 0;
      timeout.tv_usec = 1000000;
      ping_hosts (sock, &timeout);
    }

    close (sock);
  }

  {
    int i;
    struct in_addr addr;
    unsigned long replied_count = 0;

    for (i = 0; hosts[i] != INADDR_NONE; i++) {
      addr.s_addr = hosts[i];
      if (lookup (hosts[i])->replied) {
	replied_count ++;
	if (!only_count)
	  printf ("%s: %s\n", get_original_name (hosts[i]), 
		  get_reply (hosts[i]));
      } else {
	all_replied = 1;
      }
    }
    if (only_count)
      printf ("%ld\n", replied_count);
  }

  return all_replied;
}
