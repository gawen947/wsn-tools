/* File: version.h

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

#ifndef _VERSION_H_
#define _VERSION_H_

#define PACKAGE "wsn-tools"

#ifndef VERSION
# define VERSION "unk"
#endif /* VERSION */

#if !(defined COMMIT && defined PARTIAL_COMMIT)
# define PACKAGE_VERSION PACKAGE " v" VERSION
#else
# define PACKAGE_VERSION PACKAGE " v" VERSION " (commit: " PARTIAL_COMMIT ")"
#endif /* COMMIT */

void version(const char *target);

#ifdef COMMIT
void commit(void);
#endif /* COMMIT */

#endif /* _VERSION_H_ */
