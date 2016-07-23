/* File: 802154-parse.c

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
#include <ctype.h>
#include <err.h>

#include "xatoi.h"
#include "802154-parse.h"

int parse_channel(const char *arg)
{
  const char *s;
  int channel, err;

  /* We basically check that the string is composed
     only of digits. */
  for(s = arg ; *s ; s++)
    if(!isdigit(*s))
      errx(EXIT_FAILURE, "invalid channel number");

  /* Only now that we know that the string is only
     composed of digits can we parse the integer
     value. */
  channel = xatou(arg, &err);

  if(err || channel > 26)
    errx(EXIT_FAILURE, "invalid channel number");

  return channel;
}
