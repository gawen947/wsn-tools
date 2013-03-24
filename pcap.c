/* File: pcap.c

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

#include <stdlib.h>
#include <sys/time.h>
#include <stdbool.h>
#include <stdint.h>
#include <err.h>

#include "iobuf.h"

/* Informations about the PCAP file format came from:
   http://wiki.wireshark.org/Development/LibpcapFileFormat */

#define PCAP_MAGIC 0xa1b2c3d4
#define PCAP_MAJOR 2
#define PCAP_MINOR 4

#define LINKTYPE_IEEE802_15_4 195

static iofile_t pcap;

#define WRITE(size)                                               \
  static void write ## size (uint ## size ## _t value) {          \
    ssize_t n = iobuf_write(pcap, &value, sizeof(value));         \
    if(n != size)                                                 \
      err(EXIT_FAILURE, "cannot write to pcap file");             \
  }

WRITE(32)
WRITE(16)

void init_pcap(const char *path)
{
  /* TODO: Append to the file if it already exists.
           Well we could do this but will have to take
           care of endianness. We can also do a tool
           to merge PCAP files (which I will do later). */
  pcap = iobuf_open(path, O_WRONLY | O_CREAT | O_TRUNC, 0666);

  if(!pcap)
    err(EXIT_FAILURE, "cannot open pcap file");

  write32(PCAP_MAGIC);            /* magic number */
  write16(PCAP_MAJOR);            /* PCAP version */
  write16(PCAP_MINOR);
  write32(0);                     /* timezone in seconds (GMT) */
  write32(0);                     /* accuracy of timestamps */
  write32(0xff);                  /* max length of packets */
  write32(LINKTYPE_IEEE802_15_4); /* data link type */
}

void append_frame(const unsigned char *frame, unsigned int size)
{
  ssize_t n;
  struct timeval tv;

  /* If the pcap was not initialized we do nothing. */
  if(!pcap)
    return;

  /* Sometime we are asked to write an empty frame.
     This may be due to an error in the firmare which
     sends empty frame. When such a case arise we just
     ignore it. */
  if(!size)
    return;

  /* Note that POSIX.1-2008 marks this function obsolete and it is recommended
     to use clock_gettime() with nanoseconds timestamps instead. Since we just
     want a microsecond timestamp and we don't care so much about time for now I
     guess it would do perfectly well. */
  gettimeofday(&tv, NULL);

  write32(tv.tv_sec);  /* timestamp seconds */
  write32(tv.tv_usec); /* timestamp microseconds */
  write32(size);       /* number of octets of packet saved in file */
  write32(size);       /* actual length of packet */

  n = iobuf_write(pcap, frame, size);
  if(n != size)
    err(EXIT_FAILURE, "cannot write to pcap file");
}

void flush_pcap(void)
{
  if(pcap)
    iobuf_flush(pcap);
}

void destroy_pcap(void)
{
  if(pcap)
    iobuf_close(pcap);
}
