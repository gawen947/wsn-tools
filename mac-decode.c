/* File: mac-decode.c

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

#define _BSD_SOURCE

#include <stdbool.h>
#include <stdlib.h>
#include <endian.h>
#include <string.h>
#include <assert.h>

#include "mac.h"
#include "common.h"

/* Check if the new pointer is within the range of the orig buffer. */
#define CHECK(orig, new, size) if(new > orig + size) return -1

static int decode_pan(struct mac_addr *addr, const unsigned char *raw_frame,
                      uint16_t *pan)
{
  if(!pan) {
    addr->pan = le16toh(U8_TO(uint16_t, raw_frame));
    return 2;
  } else {
    addr->pan = *pan;
    return 0;
  }
}

static int decode_address(struct mac_addr *addr, enum mac_addr_mode mode,
                          const unsigned char *raw_frame, uint16_t *pan)
{
  const unsigned char *raw = raw_frame;

  switch(mode) {
  case MAM_FULL:
    return 0;
  case MAM_SHORT:
    raw += decode_pan(addr, raw, pan);

    addr->mac = le16toh(U8_TO(uint16_t, raw));
    raw += sizeof(uint16_t);
    break;
  case MAM_LONG:
  case MAM_RESERVED:
    raw += decode_pan(addr, raw, pan);

    addr->mac = le64toh(U8_TO(uint64_t, raw));
    raw += sizeof(uint64_t);
    break;
  default:
    assert(0);
  }

  return raw - raw_frame;
}

int mac_decode(struct mac_frame *frame, const unsigned char *raw_frame,
               bool decode_crc, unsigned int size)
{
  size_t payload_size;
  const unsigned char *raw = raw_frame;

  /* We have to initialize the payload to avoid
     a buffer overflow with crafted frames. */
  frame->payload = NULL;

  frame->control = le16toh(U8_TO(uint16_t, raw));
  raw += sizeof(uint16_t);

  CHECK(raw_frame, raw, size);
  frame->seqno   = *raw;
  raw++;

  CHECK(raw_frame, raw, size);
  raw += decode_address(&frame->dst, (frame->control & MC_DAM) >> MC_DAM_SHR,
                        raw, NULL);
  CHECK(raw_frame, raw, size);
  raw += decode_address(&frame->src,
                        (frame->control & MC_SAM) >> MC_SAM_SHR, raw,
                        (frame->control & MC_PANCOMP) ? &frame->dst.pan : NULL);

  if(frame->control & MC_SECURITY) {
    /* Auxiliary security headers are not implemented. */
    return -1;
  }

  frame->security = NULL;

  if(decode_crc)
    payload_size = size - sizeof(uint16_t) - (raw - raw_frame);
  else
    payload_size = size - (raw - raw_frame);

  /* We only copy the payload when needed. */
  if(payload_size) {
    frame->payload = malloc(payload_size);
    memcpy((void *)frame->payload, raw, payload_size);
    frame->size    = payload_size;
  }

  /* Check for an overflow which may arise with crafted frames.
     That is minus one converted to unsigned int which would.
     result in a very large payload. */
  if(frame->size > 0xff)
    frame->payload = NULL;
  else
    raw += frame->size;

  CHECK(raw_frame, raw, size);

  if(decode_crc) {
    frame->fcs = be16toh(U8_TO(uint16_t, raw));
    CHECK(raw_frame, raw+2, size);
  }

  return 0;
}

void free_mac_frame(struct mac_frame *frame)
{
  if(frame->payload)
    free((void *)frame->payload);
}
