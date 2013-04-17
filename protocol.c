/* File: protocol.c

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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <assert.h>

#include "protocol.h"

static void full_write(int fd, const void *buf, size_t count, const char *error)
{
  while(count) {
    ssize_t n = write(fd, buf, count);
    if(n < 0)
      err(EXIT_FAILURE, "%s", error);
    count -= n;
    buf   += n;
  }
}

void prot_write(int fd,
                enum prot_mtype mt,
                const unsigned char *message,
                size_t size)
{
  assert(size <= MAX_MESSAGE_SIZE);

  /* We use an internal buffer to avoid calling write twice.
     Note that this also avoid a 'bug-prone' case with the
     transceiver's read buffer. That is, if it cannot reassemble
     the message correctly in the buffer. I would prefer the
     double write approach as it would allow us to test the code
     a bit more rigorously. */
  unsigned char buffer[MAX_MESSAGE_SIZE + 1];

  buffer[0] = mt | size;

  memcpy(buffer + 1, message, size);

  full_write(fd, buffer, size + 1, "cannot write to UART");
}

unsigned char * prot_read(unsigned char *message,
                          bool (*callback)(enum prot_mtype,
                                           unsigned char *,
                                           size_t))
{
  enum prot_mtype mt = *message & 0x80;
  size_t size        = *message & ~0x80;

  /* skip information byte */
  message++;

  /* parse the frame */
  if(!callback(mt, message, size))
    return NULL;
  return message + size;
}
