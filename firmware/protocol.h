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

#include "priv-protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

/* This initialization function configures the callbacks and send functions
   which interfaces the firmware with the protocol. The frame_cb is called when
   an entire frame is extracted from the input buffer. The control_cb is called
   when an entire control message is extracted from the input buffer.  The send
   function is called to write data to UART. */
void protocol_init(void (*frame_cb)(unsigned char *, unsigned int),
                   void (*control_cb)(enum prot_ctype,
                                      unsigned char *,
                                      unsigned int),
                   void (*send)(const unsigned char *, unsigned int));

/* This function should be called when a character is read from UART. */
void input_step(unsigned char c);

/* Send a frame on UART. */
void send_frame(const unsigned char *frame, unsigned int size);

/* Send a control message on UART. */
void send_control(enum prot_ctype type,
                  const unsigned char *data,
                  unsigned int size);

/* Send a null terminated control message which may be larger than the maximal
   size of one message. The message will be automatically fragmented into
   smaller chunks. This is useful for debug and informational messages. */
void send_asciiz(enum prot_ctype type, const unsigned char *data);

#ifdef __cplusplus
}
#endif

/* These macros send common informational and debugging messages over UART. */
#define send_info(message)  send_asciiz(PROT_CTYPE_INFO,                \
                                        (const unsigned char *)message)
#define send_debug(message) send_asciiz(PROT_CTYPE_DEBUG,               \
                                        (const unsigned char *)message)

/* These macros are used to easily report
   errors in the firmware or from the client. */
#define cli_error() send_control(PROT_CTYPE_CLI_ERROR, 0, 0)
#define srv_error() send_control(PROT_CTYPE_SRV_ERROR, 0, 0)

#endif /* _PROTOCOL_H_ */
