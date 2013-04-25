/* File: mac-encode.c

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

#include <stdbool.h>
#include <string.h>

#include "mac-encode.h"
#include "mac.h"


#define UINT(size, buf) *((uint ## size ## _t *)(buf))

static unsigned char * encode_address(const struct mac_addr *addr,
                                      enum mac_addr_mode mode,
                                      bool omit_pan,
                                      unsigned char *buf)
{
  switch(mode) {
  case MAM_RESERVED: /* cannot encode a reserved address */
  case MAM_FULL:
    return buf;
  default:
    break;
  }

  if(!omit_pan) {
    UINT(16, buf) = addr->pan;
    buf += 2;
  }

  switch(mode) {
  case MAM_SHORT:
    UINT(16, buf) = addr->mac;
    buf += 2;
    break;
  case MAM_LONG:
    UINT(64, buf) = addr->mac;
    buf += 8;
    break;
  default:
    break;
  }

  return buf;
}

int mac_encode(const struct mac_frame *frame, unsigned char *buf)
{
  const unsigned char *orig = buf;

  UINT(16, buf) = frame->control;
  buf += 2;
  *buf = frame->seqno;
  buf++;

  buf = encode_address(&frame->dst,
                       (frame->control & MC_DAM) >> MC_DAM_SHR,
                       false,
                       buf);

  buf = encode_address(&frame->src,
                       (frame->control & MC_SAM) >> MC_SAM_SHR,
                       frame->control & MC_PANCOMP,
                       buf);

  if(frame->security) {
    /* Auxiliary security headers are not implemented. */
    return -1;
  }

  if(!frame->payload)
    goto EXIT;

  /* check size before appending the payload */
  if(buf + frame->size - orig > 125 /* MAC(127) - CRC(2) */)
    return -2;

  memcpy(buf, frame->payload, frame->size);
  buf += frame->size;

EXIT:
  return buf - orig;
}

