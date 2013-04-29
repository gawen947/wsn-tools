/* File: pcap-read.h

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

#ifndef _PCAP_READ_H_
#define _PCAP_READ_H_

#include <sys/time.h>
#include <stdlib.h>

/* Open a pcap file for reading only. */
void open_reading_pcap(const char *path);

/* Read a frame from a pcap file. The function will allocate the buffer for
   the caller. However the caller has to free the buffer manually afterward.
   The size of the frame is passed through the size pointer argument. */
unsigned char * pcap_read_frame(size_t *size, struct timeval *tv);

/* Close the pcap file. */
void close_reading_pcap(void);

#endif /* _PCAP_READ_H_ */
