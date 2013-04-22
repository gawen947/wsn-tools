/* File: 802154-parse.h

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

#ifndef _802154_PARSE_H_
#define _802154_PARSE_H_

/* TODO: Extend the channel API.

   The API should comprise a channel structure encompassing the channel's
   internal representation. This structure should be convertible to/from an user
   string which will replace the actual parse channel function. This structure
   should also be convertible to/from a protocol representation of the
   channel. Such a representation should represent the canal completely in a
   non-ambiguous way and be aware of the underlying endianess of the
   protocol. */

/* Convert a channel string into a channel number. For now we only blindly
   support the channel number and expect that the transceiver's firmware know
   what to do with it. However a more extended API should be aware of the
   standard (see IEEEE Std 802.15.4 section 8.2.1) and provide extended
   informations about the selected channel (see TODO above). */
int parse_channel(const char *arg);

#endif /* _802154_PARSE_H_ */
