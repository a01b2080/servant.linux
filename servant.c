/*
   servant - Server UDP requests and print statistics

   Copyright (C) 2017 twilmer@visteon.com

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#include <grp.h>
#include <pwd.h>
#include <termios.h>
*/

#include "system.h"
#include <getopt.h>
#include <stdio.h>
#include <sys/types.h>

#define EXIT_FAILURE 1

char *xmalloc();
char *xrealloc();
char *xstrdup();

static void usage(int status);

/* The name the program was run with, stripped of any leading path. */
char *program_name;

/* getopt_long return codes */
enum { DUMMY_CODE = 129 };

/* Option flags and variables */

static struct option const long_options[] = {{"help", no_argument, 0, 'h'},
                                             {"version", no_argument, 0, 'V'},
                                             {NULL, 0, NULL, 0}};

static int decode_switches(int argc, char **argv);

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#define LOCAL_SERVER_PORT 13107
#define BUF 255
enum { SAMPLE_COUNT = 1000 };

uint32_t sampleA[SAMPLE_COUNT] = {0};
uint32_t sampleB[SAMPLE_COUNT] = {0};
uint32_t sampleC[SAMPLE_COUNT] = {0};
int main(int argc, char **argv) {
  int s, rc, n, len;
  struct sockaddr_in cliAddr, servAddr;
  uint32_t puffer[100];
  uint32_t reply[100];
  time_t time1;
  char loctime[BUF];
  char *ptr;
  const int y = 1;

  s = socket(AF_INET, SOCK_DGRAM, 0);
  if (s < 0) {
    printf("%s: Kann Socket nicht öffnen ...(%s)\n", argv[0], strerror(errno));
    exit(EXIT_FAILURE);
  }

  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = htons(LOCAL_SERVER_PORT);
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int));
  rc = bind(s, (struct sockaddr *)&servAddr, sizeof(servAddr));
  if (rc < 0) {
    printf("%s: Cannot bind to  port %d  (%s)\n", argv[0], LOCAL_SERVER_PORT,
           strerror(errno));
    exit(EXIT_FAILURE);
  }
  printf("%s: Waiting for data on  (UDP) %u\n", argv[0], LOCAL_SERVER_PORT);

  memset(puffer, 0, sizeof(puffer));
  for (int i = 0; i < SAMPLE_COUNT; i++) {

    len = sizeof(cliAddr);
    n = recvfrom(s, (char *)puffer, BUF, 0, (struct sockaddr *)&cliAddr, &len);
    if (n < 0) {
      printf("%s: Kann keine Daten empfangen ...\n", argv[0]);
      continue;
    }
    rc = sendto(s, (char *)puffer, n, 0, (struct sockaddr *)&cliAddr, len);

    sampleA[i] = puffer[0];
    sampleB[i] = puffer[1];
    sampleC[i] = puffer[2];
    if (i < 10)
      printf("%d, %d, %d\n", sampleA[i], sampleB[i], sampleC[i]);
  }
  printf("Got %d Samples\n", SAMPLE_COUNT);
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    if (sampleA[i] - 1 == sampleC[i])
      printf("%d, %d, %d\n", sampleA[i], sampleB[i], sampleC[i]);
  }
  return EXIT_SUCCESS;
}

static int decode_switches(int argc, char **argv) {
  int c;

  while ((c = getopt_long(argc, argv, "h"  /* help */
                                      "V", /* version */
                          long_options,
                          (int *)0)) != EOF) {
    switch (c) {
    case 'V':
      printf("servant %s\n", VERSION);
      exit(0);

    case 'h':
      usage(0);

    default:
      usage(EXIT_FAILURE);
    }
  }

  return optind;
}

static void usage(int status) {
  printf(_("%s - \
Server UDP requests and print statistics\n"),
         program_name);
  printf(_("Usage: %s [OPTION]... [FILE]...\n"), program_name);
  printf(_("\
Options:\n\
  -h, --help                 display this help and exit\n\
  -V, --version              output version information and exit\n\
"));
  exit(status);
}
