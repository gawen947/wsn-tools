/* File: common.h

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

#ifndef _COMMON_H_
#define _COMMON_H_

#include "sys-endian.h"

#define U8_TO(type, ptr) *((type *)(ptr))
#define S_BOOLEAN(a) ((a) ? "yes" : "no")

#endif /* _COMMON_H_ */
