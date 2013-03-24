/* File: help.h

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

#ifndef _HELP_H_
#define _HELP_H_

struct opt_help {
  char name_short;
  const char *name_long;
  const char *help;
};

/* Display an help message for a list of long and short options. */
void help(const char *name, const char *usage, const struct opt_help opts[]);

#endif /* _HELP_H_ */
