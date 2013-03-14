/* File: mac-display.h
   Time-stamp: <2013-03-14 16:52:41 gawen>

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

#ifndef _MAC_DISPLAY_H_
#define _MAC_DISPLAY_H_

#include "mac.h"

/* Display a decoded IEEE 802.15.4 MAC frame. The info flag selects fields of
   the MAC frame to display. */
void mac_display(const struct mac_frame *frame, unsigned int info);

#endif /* _MAC_DISPLAY_H_ */
