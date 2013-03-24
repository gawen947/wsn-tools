/* File: getflg.h

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

#ifndef _GETFLG_H_
#define _GETFLG_H_

#include <stdint.h>

/* We have to specify manually wether we want 64-bit or 32-bit wide flags. */
#ifdef FLAGS64
# define FLAGS_CHUNK_SIZE 64
typedef uint64_t flags_t;
#else
# define FLAGS_CHUNK_SIZE 32
typedef uint32_t flags_t;
#endif /* FLAGS64 */


#define GET_BIT(flags, bit) \
  (flags)[bit / FLAGS_CHUNK_SIZE]  &  ((flags_t)1 << (bit % FLAGS_CHUNK_SIZE))

#define SET_BIT(flags, bit) \
  (flags)[bit / FLAGS_CHUNK_SIZE] |=   (flags_t)1 << (bit % FLAGS_CHUNK_SIZE)

#define CLEAR_BIT(flags, bit) \
  (flags)[bit / FLAGS_CHUNK_SIZE] &= ~((flags_t)1 << (bit % FLAGS_CHUNK_SIZE))


struct flag_option {
  const char *name; /* long option name */
  char short_name;  /* short option name */
  int bit;          /* position of the flag bit */
};

/* This functions parse a string for several flags parameters and setup these
   flags accordingly in an array of integer. The flags can be either long or
   short. When a string is parsed it can reference only one long flag or
   multiple short flags. If there is any ambiguity between the short and one
   long flag the longest one will be choosen first. */
int getflg(const char *argument, const struct flag_option *opts, flags_t *flags);

#endif /* _GETFLG_H_ */
