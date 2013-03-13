/* File: uart.c
   Time-stamp: <2013-03-13 18:49:25 gawen>

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

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <err.h>

int open_uart(const char *path, speed_t speed, mode_t mode)
{
  struct termios options;
  int fd;

  fd = open(path, mode | O_NOCTTY);

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

    /* setup the serial line in 8N1 */
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;     /* 8 data bits */
    options.c_cflag &= ~CSTOPB; /* 1 stop bit */
    options.c_cflag &= ~PARENB; /* no partity */

    if(tcsetattr(fd, TCSANOW, &options) < 0)
      err(EXIT_FAILURE, "cannot set tty attributes");
  }

  return fd;
}
