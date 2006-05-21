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
 * gpingd: UDP ping daemon
 */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>

#include <argp.h>

static int gpingd_port = 2471;
static char *reply_file;
static int daemonize = 1;

static struct argp_option options[] = {
  {"port", 'p', "number", 0, "Port number to listen on"},
  {"debug", 'd', 0, 0, "Run in debug mode"},
  { 0 }
};

static error_t parse_opts (int key, char *arg, struct argp_state *state)
{
  switch (key) {
  case 'p': {
    char *p = arg;
    while (*p)
      if (!isdigit (*p++)) {
	fprintf (stderr, "gping: Invalid port specification\n");
	exit (2);
      }
    gpingd_port = atoi (arg);
    break;
  }
  case 'd':
    daemonize = 0;
    break;
  case ARGP_KEY_ARG:
    if (state->arg_num >= 1)
      argp_usage (state);
    if (state->arg_num == 0)
      reply_file = arg;
    break;
  default:
    return ARGP_ERR_UNKNOWN;
  }

  return 0;
}

const char *argp_program_version = "1.0";
const char *argp_program_bug_address = "gluster-devel@nongnu.org";

static char doc[] = "gpingd: UDP based ping tool (server)\n(C) 2006 ZRESEARCH";
static char args_doc[] = "reply_file";

static struct argp argp = {options, parse_opts, args_doc, doc};

int main (int argc, char **argv)
{
  int sock;
  struct sockaddr_in local;

  argp_parse (&argp, argc, argv, 0, 0, 0);

  if (argc < 2) {
    fprintf (stderr, "gpingd: must specify reply file\n");
    exit (2);
  }

  local.sin_family = AF_INET;
  local.sin_port = htons (gpingd_port);
  local.sin_addr.s_addr = INADDR_ANY;

  sock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  bind (sock, (struct sockaddr *)&local, sizeof (local));

  if (daemonize)
    daemon (0, 0);

  while (1) {
    char buf[1024];
    char timebuf[256];
    struct tm *tm;
    
    struct sockaddr_in peer;
    int len = sizeof (peer);
    int buflen;
    time_t tmt;
    int fd;

    char die = 0;
    
    buflen = recvfrom (sock, buf, 1024, 0, (struct sockaddr *)&peer, &len);
    buf[buflen] = '\0';
    
    tmt = time (NULL);
    tm = localtime (&tmt);
    strftime (timebuf, 256, "%Y-%m-%d %H:%M:%S", tm);
    fprintf (stderr, "[%s] %s: %s\n", timebuf, inet_ntoa (peer.sin_addr),
	     buf);

    if (!strcmp (buf, "ping")) {
      /* ping request, simply return the contents */
      fd = open (reply_file, O_RDONLY);
      if (fd < 0) {
	fprintf (stderr, "Error: %s: ", reply_file);
	perror (NULL);
      }

      len = read (fd, buf, 1024);
      if (len < 0) {
	fprintf (stderr, "Error: %s: ", reply_file);
	perror (NULL);
      }
    }
    else {
      if (!strcmp (buf, "die") || !strcmp (buf, "die\n"))
	die = 1;
      fd = open (reply_file, O_WRONLY | O_CREAT | O_TRUNC);
      if (fd < 0) {
	fprintf (stderr, "Error: %s: ", reply_file);
	perror (NULL);
      }

      len = write (fd, buf, buflen);
      if (len < 0) {
	fprintf (stderr, "Error: %s: ", reply_file);
	perror (NULL);
      }
    }

    sendto (sock, buf, len, 0, (struct sockaddr *)&peer, sizeof (peer));
    close (fd);

    if (die)
      exit (0);
  }

  return 0;
}
