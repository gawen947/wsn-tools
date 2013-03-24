/* File: mac.h

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

#ifndef _MAC_H_
#define _MAC_H_

#include <stdint.h>

/* mac control */
#define MC_TYPE         0x7     /* frame type */
#define MC_SECURITY     0x8     /* security enabled */
#define MC_PENDING      0x10    /* frame pending */
#define MC_ACK          0x20    /* ACK request */
#define MC_PANCOMP      0x40    /* PAN ID compression */
#define MC_RESERVED     0x380   /* reserved */
#define MC_RESERVED_SHR 7       /* reserved (shift right) */
#define MC_DAM          0xc00   /* destination addressing mode */
#define MC_DAM_SHR      10      /* destination addressing mode (shift right) */
#define MC_VERSION      0x3000  /* frame version */
#define MC_VERSION_SHR  12      /* frame version (shift right) */
#define MC_SAM          0xc000  /* source addressing mode */
#define MC_SAM_SHR      14      /* source addressing mode (shift right) */

/* mac type */
enum mac_type { MT_BEACON,  /* beacon */
                MT_DATA,    /* data */
                MT_ACK,     /* acknowledgment */
                MT_CMD      /* MAC command */ };

/* addressing mode */
enum mac_addr_mode { MAM_FULL,      /* neither PAN or address fields are present */
                     MAM_RESERVED,  /* reserved */
                     MAM_SHORT,     /* short address */
                     MAM_LONG       /* long address */ };

enum mac_version { MV_2003,    /* IEEE 802.15.4-2003 */
                   MV_CURRENT  /* IEEE 802.15.4 */ };

struct mac_addr {
  uint16_t pan;
  uint64_t mac;
};

struct mac_frame {
  uint16_t control;     /* frame control */
  uint8_t  seqno;       /* sequence number */

  struct mac_addr src;  /* source address */
  struct mac_addr dst;  /* destination address */

  const void *security; /* auxiliary security header (not implemented) */

  const void *payload;  /* frame payload */
  unsigned int size;    /* size of the payload */

  uint16_t fcs;         /* 16-bit CRC (not used) */
};

#endif /* _MAC_H_ */
