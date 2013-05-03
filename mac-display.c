/* File: mac-display.c

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
#include <assert.h>

#include "sys-endian.h"
#include "mac.h"
#include "mac-display.h"
#include "common.h"

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

void mac_display_type(const struct mac_frame *frame)
{
  display_mac_type(frame->control & MC_TYPE);
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
    printf("ignored");
    return;
  case(MAM_RESERVED):
    printf("(reserved) ");
    break;
  default:
    break;
  }

  printf("%04X-", addr->pan);

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
}

void mac_display_saddr(const struct mac_frame *frame)
{
  display_addr((frame->control & MC_SAM) >> MC_SAM_SHR, &frame->src);
}

void mac_display_daddr(const struct mac_frame *frame)
{
  display_addr((frame->control & MC_DAM) >> MC_DAM_SHR, &frame->dst);
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
    fputc('\n', stdout);

    printf(" Dest. address : ");
    display_addr((frame->control & MC_DAM) >> MC_DAM_SHR, &frame->dst);
    fputc('\n', stdout);
  }

  if(frame->security && info & MI_SECURITY)
    printf(" Security not implemented\n");
}
