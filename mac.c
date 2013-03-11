/* File: mac.c
   Time-stamp: <2013-03-11 01:15:37 gawen>

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

#include <stdio.h>
#include <arpa/inet.h>
#include <endian.h>
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
               unsigned int size)
{
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

  frame->payload  = raw;
  frame->size     = size - sizeof(uint16_t) - (raw - raw_frame);

  /* Check for an overflow which may arise with crafted frames.
     That is minus one converted to unsigned int which would.
     result in a very large payload. */
  if(frame->size > 0xff)
    frame->payload = NULL;

  raw += frame->size;

  CHECK(raw_frame, raw, size);
  frame->fcs = be16toh(U8_TO(uint16_t, raw));

  CHECK(raw_frame, raw+2, size);
  return 0;
}

static void display_mac_type(enum mac_type type)
{
  switch(type) {
  case(MT_BEACON):
    printf("BEACON frame");
    break;
  case(MT_DATA):
    printf("DATA frame");
    break;
  case(MT_ACK):
    printf("ACK frame");
    break;
  case(MT_CMD):
    printf("COMMAND frame");
    break;
  default:
    printf("UNKNOWN frame (0x%x)", type);
    break;
  }
}

static void display_mac_addr_mode(enum mac_addr_mode am)
{
  switch(am) {
  case(MAM_FULL):
    printf("full");
    break;
  case(MAM_RESERVED):
    printf("reserved");
    break;
  case(MAM_SHORT):
    printf("short");
    break;
  case(MAM_LONG):
    printf("long");
    break;
  default:
    assert(0);
  }
}

static void display_addr(enum mac_addr_mode am, const struct mac_addr *addr)
{
  uint64_t mac;

  switch(am) {
  case(MAM_FULL):
    printf("ignored\n");
    return;
  case(MAM_RESERVED):
    printf("(reserved) ");
    break;
  default:
    break;
  }

  printf("%04X ", addr->pan);

  switch(am) {
    int i;

  case(MAM_SHORT):
    printf("%04X", (uint16_t)addr->mac);
    break;
  case(MAM_LONG):
    mac = htobe64(addr->mac);

    for(i = 0 ; i < sizeof(addr->mac) - 1 ; i++) {
      printf("%02X:", (unsigned int)(mac & 0xff));
      mac >>= 8;
    }
    printf("%02X", (unsigned int)mac);
    break;
  default:
    break;
  }
  printf("\n");
}

void mac_display(const struct mac_frame *frame, unsigned int info)
{
  display_mac_type(frame->control & MC_TYPE);

  if(!info) {
    printf("\n");
    return;
  }
  printf(":\n");

  if(info & MI_CONTROL) {
    printf(" Control:\n");

    if(((frame->control & MC_VERSION) >> MC_VERSION_SHR) != MV_CURRENT) {
      printf("  Version   : ");

      switch((frame->control & MC_VERSION) >> MC_VERSION_SHR) {
      case MV_2003:
        printf("IEEE 802.15.4-2003");
        break;
      default:
        printf("unknown (0x%x)",
               (frame->control & MC_VERSION) >> MC_VERSION_SHR);
        break;
      }
      printf("\n");
    }

    printf("  Type      : ");
    display_mac_type(frame->control & MC_TYPE);
    printf("\n");

    printf("  Security  : %s\n", S_BOOLEAN(frame->control & MC_SECURITY));
    printf("  Pending   : %s\n", S_BOOLEAN(frame->control & MC_PENDING));
    printf("  ACK req.  : %s\n", S_BOOLEAN(frame->control & MC_ACK));
    printf("  PAN comp. : %s\n", S_BOOLEAN(frame->control & MC_PANCOMP));

    printf("  DAM       : ");
    display_mac_addr_mode((frame->control & MC_DAM) >> MC_DAM_SHR);
    printf("\n");

    printf("  SAM       : ");
    display_mac_addr_mode((frame->control & MC_SAM) >> MC_SAM_SHR);
    printf("\n");

    if(frame->control & MC_RESERVED)
      printf("  Reserved  : 0x%x\n",
             (frame->control & MC_RESERVED) >> MC_RESERVED_SHR);
  }

  if(info & MI_SEQNO)
    printf(" Sequence number: %d\n", frame->seqno);

  if(info & MI_ADDR) {
    printf(" Src. address  : ");
    display_addr((frame->control & MC_SAM) >> MC_SAM_SHR, &frame->src);

    printf(" Dest. address : ");
    display_addr((frame->control & MC_DAM) >> MC_DAM_SHR, &frame->dst);
  }

  if(frame->security && info & MI_SECURITY) {
    printf(" Security not implemented");
    return;
  }
}
