/* File: string-utils.h

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

#ifndef _STRING_UTILS_H_
#define _STRING_UTILS_H_

#include <stdlib.h>
#include <sys/time.h>

/* Write a string litteral. */
#define write_slit(fd, s) write(fd, s, sizeof(s) - 1)

/* Fill the size first bytes of the specified buffer with random bytes. */
void fill_with_random(unsigned char *buf, unsigned int size);

/* Convert a timeval structure seen as a duration to a string.
   The most common usage for this function is to represent latencies. */
const char * tv_to_str(const struct timeval *tv);

/* Duplicate a buffer. */
void * memdup(const void *buf, size_t size);

#endif /* _STRING_UTILS_H_ */
