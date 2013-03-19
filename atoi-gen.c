/* File: atoi_u.c
   Time-stamp: <2013-03-19 03:17:07 gawen>

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

#include <ctype.h>

#include "atoi-gen.h"

static unsigned int symbol_value(char c)
{
  /* digit */
  if(c <= '9')
    return c - '0';

  /* lowercase */
  if(c > 'Z')
    return c - 'a' + 10;

  /* uppercase */
  return c - 'A' + 10;
}

int atoi_gen(const char *s)
{
  int sgn = 1;
  int val = 0;
  unsigned int base = 10;

#define ZERO_END(c) if(c == '\0') goto RESULT

  /* Skip leading spaces */
  for(; isspace(*s) ; s++)
    ZERO_END(*s);

  ZERO_END(*s);

  if(*s == '-') {
    sgn = -1;
    ZERO_END(*++s);
  }

  if(*s == '0') {
    ZERO_END(*++s);

    switch(*s) {
    case 'x':
    case 'X':
      base = 16;
      s++;
      break;
    case 'b':
    case 'B':
      base = 2;
      s++;
      break;
    default:
      base = 8;
    }
  }

  /* Convert from base */
  for(; !isspace(*s) ; s++) {
    ZERO_END(*s);

    val *= base;
    val += symbol_value(*s);
  }

  for(; isspace(*s) ; s++)
    ZERO_END(*s);

RESULT:
  return val * sgn;
}
