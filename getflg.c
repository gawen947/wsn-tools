/* File: getflg.c

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


#include <string.h>

#include "getflg.h"

static void do_flag(flags_t *flags, char type, int bit)
{
  flags_t bitval = (flags_t)1 << (bit % FLAGS_CHUNK_SIZE);
  int idx        = bit / FLAGS_CHUNK_SIZE;

  switch(type) {
  case '+':
    flags[idx] |= bitval;
    break;
  case '-':
    flags[idx] &= ~bitval;
    break;
  default:
    break;
  }
}

int getflg(const char *argument, const struct flag_option *opts, flags_t *flags)
{
  const struct flag_option *o;
  char type;
  int count = 0;

  /* check for empty string */
  if(*argument == '\0')
    return 0;

  /* set or clear */
  switch(*argument) {
  case '+':
  case '-':
    type = *argument++;
    break;
  default:
    type = '+';
    break;
  }

  /* check for long option */
  for(o = opts ; o->name ; o++) {
    if(!strcmp(argument, o->name)) {
      do_flag(flags, type, o->bit);
      return 1;
    }
  }

  /* check for short option */
  for(; *argument ; argument++) {
    for(o = opts ; o->name ; o++) {
      if(*argument == o->short_name) {
        do_flag(flags, type, o->bit);
        count++;
        goto FOUND;
      }
    }

    /* not found */
    return -1;

  FOUND:
    continue;
  }

  return count;
}
