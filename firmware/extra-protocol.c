/* File: extra-protocol.c

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
#include <stdio.h>
#include <stdarg.h>

#include "protocol.h"

static void _info(const char *s)
{
  send_info(s);
}

static void _debug(const char *s)
{
  send_debug(s);
}

static int generic_printf(void (*send)(const char *), const char *fmt, va_list ap)
{
  char *buf;
  int size;

  /* We do this in three step.
     1) We get the size of the string.
     2) We allocate a string large enough.
     3) We store the result in this string.
     This is slow but it guarantees that our buffer
     is large enough. */
  size = vsnprintf(NULL, 0, fmt, ap);
  buf  = malloc(size + 1);
  size = vsprintf(buf, fmt, ap);

  /* Now we send the formatted buffer. */
  send(buf);

  /* We also have to free our allocated buffer. */
  free(buf);

  return size;
}

#define _PRINTF(name, func)                        \
  int name ## _printf(const char *fmt, ...)        \
  {                                                \
  int ret;                                         \
                                                   \
  va_list ap;                                      \
  va_start(ap, fmt);                               \
  ret = generic_printf(func, fmt, ap);             \
  va_end(ap);                                      \
                                                   \
  return ret;                                      \
  }

_PRINTF(info, _info);
_PRINTF(debug, _debug);
