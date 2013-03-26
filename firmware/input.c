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

#include "input.h"

void loop_step(char c, void (*tx_frame)(const char *, unsigned int))
{
  static unsigned int idx  = 0;
  char buffer[128];

  buffer[idx] = c;

  /* parse the entire frame */
  if(idx == (buffer[0] & 0x80)) {
    tx_frame(buffer, (buffer[0] & 0x80));

    /* reset */
    idx = 0;
  }
  else
    idx++;
}
