/* File: mac-display.h

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

/* mac information */
enum mac_info { MI_CONTROL  = 0x1, /* display frame control */
                MI_SEQNO    = 0x2, /* display sequence number */
                MI_ADDR     = 0x4, /* display addressing fields */
                MI_SECURITY = 0x8, /* display auxiliary security header */
                MI_FCS      = 0x10 /* display frame check sequence */ };
#define MI_ALL 0x1f /* display everything */

/* Display a decoded IEEE 802.15.4 MAC frame. The info flag selects fields of
   the MAC frame to display. */
void mac_display(const struct mac_frame *frame, unsigned int info);

/* Display the type of the MAC frame. */
void mac_display_type(const struct mac_frame *frame);

/* Display the source MAC address. */
void mac_display_saddr(const struct mac_frame *frame);

/* Display the destination MAC address. */
void mac_display_daddr(const struct mac_frame *frame);

#endif /* _MAC_DISPLAY_H_ */
