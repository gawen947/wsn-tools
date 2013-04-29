/* File: pcap-write.h

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

#ifndef _PCAP_WRITE_H_
#define _PCAP_WRITE_H_

/* Initialize the PCAP output for writing only. */
void open_writing_pcap(const char *path);

/* Append a MAC frame to the PCAP file. */
void pcap_append_frame(const unsigned char *frame, unsigned int size);

/* Flush the PCAP file. */
void pcap_write_flush(void);

/* Close the PCAP file. */
void close_writing_pcap(void);

#endif /* _PCAP_WRITING_H_ */
