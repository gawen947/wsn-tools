/* File: string-utils.c

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
#include <stdlib.h>
#include <sys/time.h>

void fill_with_random(unsigned char *buf, unsigned int size)
{
  int i;

  struct timeval tv;
  unsigned int seed;

  gettimeofday(&tv, NULL);

  /* David's magic hash */
  seed = (tv.tv_sec  + (tv.tv_sec  << 11)) ^ \
         (tv.tv_usec + (tv.tv_usec << 17));

  srand(seed);

  for(i = 0 ; i < size ; i++)
    buf[i] = rand() % 0xff;
}

const char * tv_to_str(const struct timeval *tv)
{
  static char buf[sizeof("111.111 minutes")];

  if(tv->tv_sec >= 86400)
    return "> 1 day";
  else if(tv->tv_sec >= 3600)
    sprintf(buf, "%3.3f hours", (double)tv->tv_sec / 3600);
  else if(tv->tv_sec >= 60)
    sprintf(buf, "%3.3f minutes", (double)tv->tv_sec / 60);
  else if(tv->tv_sec > 0)
    sprintf(buf, "%ld.%03ld s", tv->tv_sec, tv->tv_usec / 1000);
  else if(tv->tv_usec > 1000)
    sprintf(buf, "%ld ms", tv->tv_usec / 1000);
  else
    sprintf(buf, "%ld us", tv->tv_usec);

  return buf;
}
