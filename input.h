/* File: input.h

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

#ifndef _INPUT_H_
#define _INPUT_H_

#include <stdbool.h>

#include "protocol.h"

/* This will start a buffered input loop which will read the specified
   file descriptor for messages. When a message is found, the specified
   callback will be called with the message type and its size. An optional
   waiting message will be displayed along with a spinning indicator if
   specified. The loop will wait for timeout seconds or indefinitely if
   timeout is zero. The return value are :
   0  if the function returned from a callback.
   -1 if the function returned from a callback with a non empty buffer.
   -2 if the function returned from a timeout. */
int input_loop(int fd,
               bool (*callback)(const unsigned char *,
                                enum prot_mtype,
                                size_t),
               const char *w_message,
               int timeout);

#endif /* _INPUT_H_ */
