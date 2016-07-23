/* File: uart.c

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
#include <sys/times.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>

#include "xatoi.h"
#include "protocol.h"

speed_t baud(const char *arg)
{
  int err;

  const struct {
    int     intval;
    speed_t baud;
  } *b, bauds[] = {
    { 230400, B230400 },
    { 115200, B115200 },
    { 57600, B57600 },
    { 38400, B38400 },
    { 19200, B19200 },
    { 9600, B9600 },
    { 4800, B4800 },
    { 2400, B2400 },
    { 1800, B1800 },
    { 1200, B1200 },
    { 300, B300 },
    { 200, B200 },
    { 150, B150 },
    { 134, B134 },
    { 110, B110 },
    { 75, B75 },
    { 50, B50 },
    { 0,  B0 }};

  int arg_val = xatou(arg, &err);
  if(err)
    goto ERR;

  for(b = bauds; b->intval ; b++)
    if(b->intval == arg_val)
      return b->baud;

ERR:
  errx(EXIT_FAILURE, "unrecognized speed");
}

static void wait_firmware(int fd)
{
  /* The firmware will simply send the ready byte when he is ready.
     So we just way for this ready byte before returning control
     to the program. */
  unsigned char ready;
  fd_set rfds;
  int ret;

  /* We setup file descriptor set. */
  FD_ZERO(&rfds);
  FD_SET(fd, &rfds);

  while(1) {
    struct timeval tv = { .tv_sec = READY_TIMEOUT };

    ret = select(fd + 1, &rfds, NULL, NULL, &tv);

    if(ret < 0) {
      /* signal caught */
      if(errno == EINTR)
        continue;
      err(EXIT_FAILURE, "cannot select");
    } else if (!ret)
      /* timeout */
      errx(EXIT_FAILURE, "ready timeout");

    /* There are bytes to read. We will check for our ready byte. */
    break;
  }

  /* Read the one ready byte. */
  if(read(fd, &ready, 1) != 1)
    errx(EXIT_FAILURE, "cannot read the waiting byte");

  /* Check the value of the ready byte. */
  if(ready != READY_BYTE)
    errx(EXIT_FAILURE, "invalid ready byte");
}

int open_uart(const char *path, speed_t speed)
{
  struct termios options = { 0 };
  int fd;

  fd = open(path, O_RDWR | O_NOCTTY);

  if(fd < 0)
    err(EXIT_FAILURE, "cannot open serial port");

  /* initial checks */
  if(!isatty(fd))
    err(EXIT_FAILURE, "invalid serial port");

  /* we only setup the speed if requested */
  if(speed != B0) {
    cfsetspeed(&options, speed);

    /* This setup the serial line in 8N1 and break input.
       We don't need to clear any other bits since we
       zeroed the termios structure at initialization. */
    options.c_iflag |= IGNBRK;
    options.c_cflag |= CS8;

    if(tcsetattr(fd, TCSANOW, &options) < 0)
      err(EXIT_FAILURE, "cannot set tty attributes");
  }

  /* Some operating systems (eg Linux) bufferise the UART input
     even when the file descriptor is not opened. This may be
     useful in a lot of cases. However we may lay with incomplete
     messages on the UART buffer. Therefore we have to flush the
     buffer ourself. However for a reason unknown to me, we have
     to wait a bit before actually flushing. Otherwise the flush
     command would have no effect. */
  usleep(500);
  tcflush(fd, TCIOFLUSH);

  /* Wait that the firmware signals us that he is ready. */
  wait_firmware(fd);

  return fd;
}
