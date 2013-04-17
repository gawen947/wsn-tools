/* File: protocol.h

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

#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include <stdlib.h>
#include <stdbool.h>

/*
  The transmission unit used in this protocol is called a message.
  A message consist of an information byte followed by a payload of
  0 to 127 bytes. The information byte specifies the type of the
  message and its size.

  A message can be either a frame message or a control message.
  Frame messages are used to transmit a frame to/from the transceiver.
  The entire payload is used to represent the raw frame.
  Control messages are used to transmit commands or responses to/from
  the transceiver. That is, they permit to configure the transceiver,
  transmit informational textual messages, signal success and error.
*/

/* protocol message typ */
enum prot_mtype { PROT_MTYPE_FRAME   = 0x00,
                  PROT_MTYPE_CONTROL = 0x80 };

/* protocol control type */
enum prot_ctype { PROT_CTYPE_INFO      = 0x00,
                  PROT_CTYPE_DEBUG     = 0x01,
                  PROT_CTYPE_CONFIG    = 0x02,
                  PROT_CTYPE_ACK       = 0x03,
                  PROT_CTYPE_CLI_ERROR = 0x04,
                  PROT_CTYPE_SRV_ERROR = 0x05 };

/* The maximum allowed size for a message. */
#define MAX_MESSAGE_SIZE 127

/* Encode a message with the specified type and payload and write it to the
   specified file descriptor. */
void prot_write(int fd,
                enum prot_mtype mt,
                const unsigned char *message,
                size_t size);

/* Decode a message and call a function with the decoded message. It will return
   a pointer to the position just after the message being read. If the
   callback return false, the n returned pointer will be NULL which instructs
   the read loop to break. */
unsigned char * prot_read(unsigned char *message,
                          bool (*callback)(enum prot_mtype,
                                           unsigned char *,
                                           size_t));

#endif /* _PROTOCOL_H_ */
