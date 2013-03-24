/* File: dump.c

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

#include <stdio.h>
#include <ctype.h>

#include "dump.h"

/* display configuration */
#define OFFSET_FMT  "$%04x:"
#define BYTE_FMT    "%02x"
#define PADDING_FMT "  "
#define WORD_SZ     8

#define DWORD_SZ    WORD_SZ << 1

void hex_dump(const unsigned char *data, int size)
{
  int i;
  unsigned int offset = 0;

  for(; size >= DWORD_SZ ; size -= DWORD_SZ) {
    printf(OFFSET_FMT " ", offset);

    for(i = 0 ; i < WORD_SZ ; i++, data++)
      printf(BYTE_FMT " ", *data);
    putchar(' ');
    for(i = 0 ; i < WORD_SZ ; i++, data++)
      printf(BYTE_FMT " ", *data);

    data -= DWORD_SZ;
    putchar('|');
    for(i = 0 ; i < DWORD_SZ ; i++, data++) {
      if(isprint(*data))
        putchar(*data);
      else
        putchar('.');
    }
    printf("|\n");

    offset += DWORD_SZ;
  }

  if(size <= 0)
    return;

  printf(OFFSET_FMT " ", offset);

  for(i = 0 ; i < WORD_SZ ; i++, data++, size--) {
    if(size > 0)
      printf(BYTE_FMT " ", *data);
    else
      printf(PADDING_FMT " ");
  }

  putchar(' ');

  for(i = 0 ; i < WORD_SZ ; i++, data++, size--) {
    if(size > 0)
      printf(BYTE_FMT " ", *data);
    else
      printf(PADDING_FMT " ");
  }

  data -= DWORD_SZ;
  size += DWORD_SZ;

  putchar('|');
  for(i = 0 ; i < DWORD_SZ ; i++, data++, size--) {
    if(size <= 0) {
      putchar(' ');
      continue;
    }

    if(isprint(*data))
      putchar(*data);
    else
      putchar('.');
  }
  printf("|\n");
}
