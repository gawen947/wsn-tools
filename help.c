/* File: help.c
   Time-stamp: <2013-02-06 13:36:00 gawen>

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
#include <string.h>

#include "help.h"

void help(const char *name, const char *usage, const struct opt_help opts[])
{
  const struct opt_help *opt;
  int size;
  int max = 0;

  fprintf(stderr, "usage: %s %s\n", name, usage);

  /* maximum option names size for padding */
  for(opt = opts ; opt->name_long ; opt++) {
    size = strlen(opt->name_long);
    if(size > max)
      max = size;
  }

  /* print options and help messages */
  for(opt = opts ; opt->name_long ; opt++) {
    if(opt->name_short != '\0')
      fprintf(stderr, "  -%c, --%s", opt->name_short, opt->name_long);
    else
      fprintf(stderr, "      --%s", opt->name_long);

    /* padding */
    size = strlen(opt->name_long);
    for(; size <= max ; size++)
      fputc(' ', stderr);
    fprintf(stderr, "%s\n", opt->help);
  }
}
