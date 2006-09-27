#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define UDPCAST_AGENT "gcsh"

int
main (int argc, char *argv[])
{
  int fd;

  if (argc != 3) {
    fprintf (stderr, "Usage: %s <interpreter> <script>\n", argv[0]);
    exit (1);
  }

  fd = open (argv[2], O_RDONLY);
  if (fd == -1) {
    fprintf (stderr, "open() on %s failed: %s\n", argv[2], strerror (errno));
    exit (2);
  }
  dup2 (fd, 0);
  argv[0] = UDPCAST_AGENT;
  execlp (UDPCAST_AGENT, argv[0], argv[1], NULL);
  perror ("execlp()");
  return errno;
}
