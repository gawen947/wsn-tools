/* File: input.c

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

#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <err.h>

#include "input.h"

#define UART_BUFFER_SIZE 1024   /* UART input buffer size */
#define SELECT_INTERVAL  200000 /* Interval between */

/* We use the timeout scale to adapt it to the
   select interval which we use as our clock. */
#define TO_SCALE    1000000 / SELECT_INTERVAL

/* This structure describe the wait message used with select. */
struct p_wait {
  const char *message;
  char *clear;
};

#define CHECK_DISPLAY(w) if(!(w)->clear) return

/* Prepare the clear message associated with the wait message. */
static void prepare_wait_message(struct p_wait *wait, const char *message)
{
  int size;

  if(!message) {
    wait->clear = NULL;
    return;
  }

  size = strlen(message) + sizeof("[*]");

  wait->message = message;
  wait->clear   = malloc(size);
  memset(wait->clear, ' ', size);
  wait->clear[size] = '\0';
}

static void destroy_wait_message(const struct p_wait *wait)
{
  CHECK_DISPLAY(wait);

  free(wait->clear);
}

static void wait_message(const struct p_wait *wait)
{
  char spin[]  = { '/', '-', '\\', '|' };
  static int i = 0;

  CHECK_DISPLAY(wait);

  printf("%s [%c]\r", wait->message, spin[i++ % sizeof(spin)]);
  fflush(stdout);
}

static void clear_message(const struct p_wait *wait)
{
  CHECK_DISPLAY(wait);

  printf("%s\r", wait->clear);
  fflush(stdout);
}

/* This function will parse the buffer. This is where we bind ourself to the
   protocol. We don't know about the protocol directly here, except that it is
   composed of messages with a certain length and of a certain type. This
   function can return the start position which indicates the last position of
   the unparsed message left in the buffer. When there is no buffer left and the
   buffer is empty it will return 0.  When we exit from a callback we return -1
   if the buffer is empty and -2 if the buffer is non empty. */
static int parse_uart_buffer(unsigned char *buffer,
                             size_t size,
                             bool (*callback)(const unsigned char *,
                                              enum prot_mtype,
                                              size_t),
                             const struct p_wait *w)
{
  size_t frame_size;
  const unsigned char *buffer_end = buffer + size;
  const unsigned char *p;

  for(p = buffer ; p < buffer_end ; p += frame_size) {
    enum prot_mtype type;

    /* extract frame size and event type */
    frame_size = *p & 0x7f;
    type       = *p & 0x80;

    /* check that the entire frame is still inside the buffer */
    if(p + frame_size >= buffer_end)
      break;

    /* skip information byte */
    p++;

    /* parse this frame */
    clear_message(w);

    /* We call back with the frame. If the callback function returns false
       we return from the function, signaling to the the caller. */
    if(!callback(p, type, frame_size)) {
      if(p + frame_size >= buffer_end)
        return -1;
      else
        return -2;
    }
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

int input_loop(int fd,
               bool (*callback)(const unsigned char *,
                                enum prot_mtype,
                                size_t),
               const char *w_message,
               int timeout)
{
  struct p_wait w;
  unsigned char buf[UART_BUFFER_SIZE];
  fd_set rfds;
  int start  = 0;
  int retval = 0;
  int ttl    = timeout * TO_SCALE;

  /* Prepare the wait message. */
  prepare_wait_message(&w, w_message);

  /* We just setup file descriptor set and issue the first wait message. */
  FD_ZERO(&rfds);
  FD_SET(fd, &rfds);

  wait_message(&w);

  /* And now we can loop for messages. */
  while(1) {
    int ret;
    ssize_t size;
    struct timeval tv = { .tv_sec  = 0,
                          .tv_usec = SELECT_INTERVAL };

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
      wait_message(&w);

      /* Since we didn't receive any message we have to check for timeout. */
      if(timeout && ttl-- == 0) {
        retval = -2;
        goto EXIT;
      }

      continue;
    }

    /* Reset the TTL */
    ttl = timeout * TO_SCALE;

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
    start = parse_uart_buffer(buf, size + start, callback, &w);

    /* We may break from here. */
    if(start < 0) {
      switch(start) {
      case(-1):
        retval = 0;
        break;
      case(-2):
        retval = -1;
        break;
      default:
        assert(0);
      }

      goto EXIT;
    }
  }

EXIT:
  /* If we are here then we break out of the loop.
     So we free resources and return. */
  destroy_wait_message(&w);

  return retval;
}
