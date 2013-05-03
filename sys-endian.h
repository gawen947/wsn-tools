/* File: sys-endian.h

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

#ifndef _SYS_ENDIAN_H_

#if defined(__FreeBSD__)
# include <sys/endian.h>
#elif defined(__APPLE__) || defined(__MACOSX__)
# include <machine/endian.h>
#else
# include <endian.h>
#endif

#endif /* _SYS_ENDIAN_H_ */
