/* File: pcap-read.c

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

#include <sys/time.h>
#include <stdlib.h>
#include <stdint.h>
#include <endian.h>
#include <limits.h>
#include <err.h>

#include "iobuf.h"
#include "pcap.h"

static iofile_t pcap;
static int timezone;
static uint16_t (*ftoh16)(uint16_t);
static uint32_t (*ftoh32)(uint32_t);

#define READ(size)                                                      \
  static void read ## size (uint ## size ## _t *value) {                \
    ssize_t n = iobuf_read(pcap, value, sizeof(uint ## size ## _t));    \
    if(n == 0)                                                          \
      errx(EXIT_FAILURE, "unexpected end-of-file");                     \
    else if(n != sizeof(uint ## size ## _t))                            \
      err(EXIT_FAILURE, "cannot read from pcap file");                  \
    *value = ftoh ## size (*value);                                     \
  }

READ(32)
READ(16)

#define _FxTOH(endian, size) \
  static uint ## size ## _t _ ## endian ## size ## toh(uint ## size ## _t value) { \
    return endian ## size ## toh(value);                                \
  }

_FxTOH(be, 16)
_FxTOH(le, 16)
_FxTOH(be, 32)
_FxTOH(le, 32)

void open_reading_pcap(const char *path)
{
  ssize_t n;
  uint32_t pcap_magic;
  uint16_t pcap_major;
  uint16_t pcap_minor;
  uint32_t accuracy;
  uint32_t max_length;
  uint32_t data_link_type;

  pcap = iobuf_open(path, O_RDONLY, 0);

  if(!pcap)
    err(EXIT_FAILURE, "cannot open pcap file");

  n = iobuf_read(pcap, &pcap_magic, sizeof(pcap_magic));
  if(n != sizeof(pcap_magic))
    err(EXIT_FAILURE, "cannot read from pcap file");

  /* Check for the magic number's endianness. */
  if(htobe32(PCAP_MAGIC) == pcap_magic) {
    ftoh16 = _be16toh;
    ftoh32 = _be32toh;
  }
  else if (htole32(PCAP_MAGIC) == pcap_magic) {
    ftoh16 = _le16toh;
    ftoh32 = _le32toh;
  }
  else
    errx(EXIT_FAILURE, "invalid magic in pcap file");

  read16(&pcap_major);           /* PCAP version */
  read16(&pcap_minor);
  read32((uint32_t *)&timezone); /* timezone in seconds (GMT) */
  read32(&accuracy);             /* accuracy of timestamps */
  read32(&max_length);           /* max length of packets */
  read32(&data_link_type);       /* data link type */

  /* check version */
  if(pcap_major != PCAP_MAJOR || pcap_minor != PCAP_MINOR)
    errx(EXIT_FAILURE, "incompatible pcap version");

  /* check accuracy */
  if(accuracy != 0)
    errx(EXIT_FAILURE, "non zero accuracies are not supported");

  /* we do not use the max length field */
  if(max_length > UINT16_MAX)
    errx(EXIT_FAILURE, "the maximum length of the pcap file is too large");

  /* check the data link type */
  if(data_link_type != LINKTYPE_IEEE802_15_4)
    errx(EXIT_FAILURE, "data link type not supported");
}

unsigned char * pcap_read_frame(size_t *size, struct timeval *tv)
{
  unsigned char *frame;
  uint32_t actual_size;
  ssize_t n;

  /* If the pcap was not initialized we do nothing */
  if(!pcap)
    return NULL;

  /* We have to do the first read manually because
     we must take care of the end-of-file that could
     only arise when starting a new frame. */
  n = iobuf_read(pcap, &tv->tv_sec, sizeof(uint32_t));
  if(!n) /* end-of-file */
    return NULL;
  else if(n != sizeof(uint32_t))
    err(EXIT_FAILURE, "cannot read from pcap file");

  read32((uint32_t *)&tv->tv_sec);
  read32((uint32_t *)&tv->tv_usec);
  read32((uint32_t *)size);
  read32(&actual_size);

  /* correct the timezone */
  tv->tv_sec += timezone;

  if(*size != actual_size)
    errx(EXIT_FAILURE, "incomplete frame in the pcap file");

  if(actual_size > UINT16_MAX)
    errx(EXIT_FAILURE, "frame too large in the pcap file");

  frame = malloc(actual_size);
  if(!frame)
    errx(EXIT_FAILURE, "out of memory");

  n = iobuf_read(pcap, frame, actual_size);
  if(n != actual_size)
    errx(EXIT_FAILURE, "incomplete read from pcap");

  return frame;
}

void close_reading_pcap(void)
{
  if(pcap)
    iobuf_close(pcap);
}
