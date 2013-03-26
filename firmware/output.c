/* File: output.c

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

#include "event.h"

#define MAX_FRAME 0x7f

int send_frame(const unsigned char *frame, unsigned int size,
               void (*send)(const unsigned char *, unsigned int))
{
  unsigned char information = EV_FRAME | size;

  if(size > MAX_FRAME)
    return -1;

  send(&information, 1);
  send(frame, size);

  return 0;
}

static unsigned int find_zero(const char *b)
{
  unsigned int i;

  for(i = 0 ; *b != '\0' && i < MAX_FRAME ; i++, b++);

  return i;
}

void send_info(const char *info,
               void (*send)(const unsigned char *, unsigned int))
{
  unsigned int size;

  for(; *info != '\0' ; info += size) {
    unsigned char information;

    size = find_zero(info);

    information = EV_INFO | size;

    send(&information, 1);
    send((const unsigned char *)info, size);
  }
}
