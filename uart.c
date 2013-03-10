/* File: uart.c
   Time-stamp: <2013-03-11 00:42:13 gawen>

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
#include <string.h>
#include <err.h>

#include "event.h"

#define UART_BUFFER_SIZE 1024 /* UART input buffer size */

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

static int open_uart(const char *path, speed_t speed)
{
  struct termios options;
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

  return fd;
}

/* This is where we know about the struture of the frames on the serial line. */
static size_t parse_uart_buffer(unsigned char *buffer, size_t size,
                                void (*callback)(const unsigned char *,
                                                 enum event,
                                                 size_t))
{
  size_t frame_size;
  const unsigned char *buffer_end = buffer + size;
  const unsigned char *p;

  for(p = buffer ; p < buffer_end ; p += frame_size) {
    enum event event_type;

    /* extract frame size and event type */
    frame_size = *p & ~0x80;
    event_type = *p & 0x80;

    /* check that the entire frame is still inside the buffer */
    if(p + frame_size >= buffer_end)
      break;

    /* skip information byte */
    p++;

    /* parse this frame */
    clear_message();
    callback(p, event_type, frame_size);
  }

  /* There are two ways to leave the preceding loop. Either we parsed the buffer
     completely, that is, we lie on a next frame beyond the end of the buffer.
     Either the last frame was incomplete and we lie on the information byte of
     this frame, that is still inside the buffer. There is however one last case
     of interest when the first frame is incomplete and we still lie at the
     beginning of the buffer. In such case we may avoid an unnecessary call to
     memmove which would have copied the frame anyway. */
  if(p >= buffer_end)
    return 0;
  else if(p == buffer)
    return buffer_end - p;
  else { /* p < buffer_end */
    size_t last_frame_size = buffer_end - p;
    memmove(buffer, p, last_frame_size);
    return last_frame_size;
  }
}

static void uart_loop(int fd,
                      void (*callback)(const unsigned char *,
                                       enum event,
                                       size_t))
{
  unsigned char buf[UART_BUFFER_SIZE];
  size_t start = 0;
  fd_set rfds;

  /* We just setup file descriptor set and issue the first wait message. */
  FD_ZERO(&rfds);
  FD_SET(fd, &rfds);

  wait_message();

  /* And now we can loop for events. */
  while(1) {
    int ret;
    ssize_t size;
    struct timeval tv = { .tv_sec  = 0,
                          .tv_usec = 200000 };

    /* Wait for something to read. */
    ret = select(fd + 1, &rfds, NULL, NULL, &tv);

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

    /* Fill the buffer. */
    size = read(fd, buf + start, sizeof(buf) - start);

    if(size <= 0) {
      /* signal caught */
      if(errno == EINTR)
        continue;
      err(EXIT_FAILURE, "cannot read");
    }

#ifndef NDEBUG
    /* This doesn't mean that we will drop frames but instead that the kernel
       will buffer them for us. It is unlikely that the kernel will have to drop
       anything but it may vary with the nature of the stdout file descriptor. */
    if(size == (sizeof(buf) - start))
      warnx("input buffer full");
#endif /* NDEBUG */

    /* This will parse the entire buffer and copy
       the last incomplete frame at the beginning. */
    start = parse_uart_buffer(buf, size + start, callback);
  }
}

void start_uart(const char *path,
                speed_t speed,
                void (*callback)(const unsigned char *,
                                 enum event,
                                 size_t))
{
  int fd;

  /* open and setup the serial line */
  fd = open_uart(path, speed);

  /* start the main loop */
  uart_loop(fd, callback);
}
