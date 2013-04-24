/* File: protocol.c

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

#include "protocol.h"
#include "extra-protocol.h"

static void (*_frame_cb)(unsigned char *, unsigned int size);
static void (*_control_cb)(enum prot_ctype type,
                              unsigned char *,
                              unsigned int size);
static void (*_send)(const unsigned char *, unsigned int);

void protocol_init(void (*frame_cb)(unsigned char *, unsigned int),
                   void (*control_cb)(enum prot_ctype,
                                      unsigned char *,
                                      unsigned int),
                   void (*send)(const unsigned char *, unsigned int))
{
  unsigned char ready = READY_BYTE;

  _frame_cb   = frame_cb;
  _control_cb = control_cb;
  _send       = send;

  /* Send the ready byte. */
  send(&ready, 1);
}

static void parse_message(unsigned char *message, unsigned int size)
{
  switch(message[0] & 0x80) {
  case(PROT_MTYPE_FRAME):
    _frame_cb(message + 1, size);
    break;
  case(PROT_MTYPE_CONTROL):
    if(size == 0)
      cli_error(); /* error: we expect at least a control type byte */

    /* catch and respond to ping messages automatically */
    if(message[1] == PROT_CTYPE_PING)
      send_control(PROT_CTYPE_PING, message + 2, size - 1);
    else
      _control_cb(message[1], message + 2, size - 1);
    break;
  }
}

void input_step(unsigned char c)
{
  static unsigned int  idx  = 0;
  static unsigned int  size; /* we may spare this variable: should we ? */
  static unsigned char buffer[MAX_MESSAGE_SIZE + 1];

  buffer[idx] = c;

  if(idx == 0)
    size = c & 0x7f;

  if(idx == size) {
    parse_message(buffer, size);
    idx = 0;
  }
  else
    idx++;
}

void send_frame(const unsigned char *frame, unsigned int size)
{
  unsigned char info = PROT_MTYPE_FRAME | size;

  _send(&info, 1);
  _send(frame, size);
}

void send_control(enum prot_ctype type,
                  const unsigned char *data,
                  unsigned int size)
{
  unsigned char info = PROT_MTYPE_CONTROL | (size + 1);

  _send(&info, 1);

  info = type;
  _send(&info, 1);

  _send(data, size);
}

static unsigned int find_zero(const unsigned char *b)
{
  unsigned int i;

  for(i = 0 ; *b != '\0' && i < (MAX_MESSAGE_SIZE - 1) ;  i++, b++);

  return i;
}

void send_asciiz(enum prot_ctype type, const unsigned char *data)
{
  unsigned int size;

  for(; *data != '\0' ; data += size) {
    size = find_zero(data);
    send_control(type, data, size);
  }
}
