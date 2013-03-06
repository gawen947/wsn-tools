/* File: pcap.h
   Time-stamp: <2013-02-05 19:15:30 gawen>

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

#ifndef _PCAP_H_
#define _PCAP_H_

/* Initialize the PCAP output. */
void init_pcap(const char *path);

/* Append a MAC frame to the PCAP file. */
void append_frame(const unsigned char *frame, unsigned int size);

/* Close the PCAP file. */
void destroy_pcap(void);

#endif /* _PCAP_H_ */
