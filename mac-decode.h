/* File: mac-decode.h
   Time-stamp: <2013-03-22 01:01:38 gawen>

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

#ifndef _MAC_DECODE_H_
#define _MAC_DECODE_H_

#include "mac.h"

/* Decode an IEEE 802.15.4 MAC frame. Return a negative number if an error
   occured. */
int mac_decode(struct mac_frame *frame, const unsigned char *raw_frame,
               unsigned int size);

/* Free internal data associated to a decoded mac frame. */
void free_mac_frame(struct mac_frame *frame);

#endif /* _MAC_DECODE_H_ */
