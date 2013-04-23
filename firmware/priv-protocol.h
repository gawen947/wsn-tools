/* File: priv-protocol.h

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

#ifdef _PROTOCOL_H_
#ifndef _PRIV_PROTOCOL_H_
#define _PRIV_PROTOCOL_H_

/* For the most part, this file is a copy of the protocol header used with the
   client. It is not intended to be used directly by the firmware which explains
   the unusual include defines. This ensures that it is included only within the
   protocol header. */

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
                  PROT_MTYPE_CONTROL = 0x80
                  /* no other types are possible */ };

/* protocol control type */
enum prot_ctype { PROT_CTYPE_INFO = 0x00,
                  PROT_CTYPE_DEBUG,
                  PROT_CTYPE_CLI_ERROR,
                  PROT_CTYPE_SRV_ERROR,
                  PROT_CTYPE_PING,
                  PROT_CTYPE_ACK,
                  PROT_CTYPE_CONFIG_CHANNEL
                  /* add new types here */ };

/* The maximum allowed size for a message. */
#define MAX_MESSAGE_SIZE 127

#endif /* _PRIV_PROTOCOL_H_ */
#endif /* _PROTOCOL_H_ */

