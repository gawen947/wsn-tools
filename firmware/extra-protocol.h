/* File: extra-protocol.h

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

#ifndef _EXTRA_PROTOCOL_H_
#define _EXTRA_PROTOCOL_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Send a formatted string as an informational message on UART. */
int info_printf(const char *fmt, ...);

/* Send a formatted string as a debug message on UART. */
int debug_printf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* _EXTRA_PROTOCOL_H_ */
