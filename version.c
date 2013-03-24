/* File: version.c

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

#include "version.h"

void version(const char *target)
{
  printf("%s from " PACKAGE_VERSION "\n", target);
}

#ifdef COMMIT
void commit(void)
{
  printf("Commit-Id SHA1 : " COMMIT "\n");
}
#endif /* COMMIT */
