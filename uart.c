/* File: uart.c
   Time-stamp: <2013-03-06 23:15:36 gawen>

   Copyright (C) 2013 David Hauweele <david@hauweele.net>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>. */

#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>

#define MAX_EVENT 256 /* maximum event size */

static void wait_message(void)
{
  char wait[]  = { '/', '-', '\\', '|' };
  static int i = 0;

  printf("Waiting for event [%c]\r", wait[i++ % sizeof(wait)]);
  fflush(stdout);
}

static void clear_message(void)
{
  printf("                      \r");
  fflush(stdout);
}

void start_uart(const char *path,
                speed_t speed,
                void (*callback)(const unsigned char *, unsigned int))
{
  unsigned char buf[MAX_EVENT];
  struct termios options;
  size_t size = 0;
  fd_set rfds;
  int fd;

  fd = open(path, O_RDONLY | O_NOCTTY);

  if(fd < 0)
    err(EXIT_FAILURE, "cannot open serial port");

  /* initial checks */
  if(!isatty(fd))
    err(EXIT_FAILURE, "invalid serial port");

  /* we only setup the speed if requested */
  if(speed != B0) {
    if(tcgetattr(fd, &options) < 0)
      err(EXIT_FAILURE, "cannot get tty attributes");

    cfsetspeed(&options, speed);

    if(tcsetattr(fd, TCSANOW, &options) < 0)
      err(EXIT_FAILURE, "cannot set tty attributes");
  }

  /* setup file descriptor set */
  FD_ZERO(&rfds);
  FD_SET(fd, &rfds);

  wait_message();

  while(1) {
    struct timeval tv = { .tv_sec  = 0,
                          .tv_usec = 200000 };
    int ret = select(fd + 1, &rfds, NULL, NULL, &tv);

    if(ret < 0) {
      /* signal caught */
      if(errno == EINTR)
        continue;
      err(EXIT_FAILURE, "cannot select");
    }

    else if(!ret) {
      FD_SET(fd, &rfds);
      wait_message();
      continue;
    }

    size = read(fd, buf, sizeof(buf));

    if(size <= 0) {
      /* signal caught */
      if(errno == EINTR)
        continue;
      err(EXIT_FAILURE, "cannot read");
    }


    clear_message();
    callback(buf, size);
  }
}
