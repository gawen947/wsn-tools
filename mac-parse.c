/* File: mac-parse.c

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

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>
#include <err.h>

#include "mac-decode.h"
#include "atoi-gen.h"
#include "mac.h"
#include "getflg.h"

void parse_flags(struct mac_frame *frame, const char *flags_arg)
{
  flags_t flags = 0;

  enum flag_opt {
    FLAG_PENDING,
    FLAG_ACK,
    FLAG_PANCOMP
  };

  struct flag_option flag_opts[] = {
    { "pending", 'p', FLAG_PENDING },
    { "ack", 'a', FLAG_ACK },
    { "pan-comp", 'c', FLAG_PANCOMP },
    { NULL, 0, 0 }
  };

  /* convert MAC flags to command line flags */
  if(frame->control & MC_PENDING)
    SET_BIT(&flags, FLAG_PENDING);
  if(frame->control & MC_ACK)
    SET_BIT(&flags, FLAG_ACK);
  if(frame->control & MC_PANCOMP)
    SET_BIT(&flags, FLAG_PANCOMP);

  /* parse command line flags */
  if(getflg(flags_arg, flag_opts, &flags) <= 0)
    errx(EXIT_FAILURE, "invalid flag -- '%s'", flags_arg);

  /* convert command line flags to MAC flags */
  if(GET_BIT(&flags, FLAG_PENDING))
    frame->control |= MC_PENDING;
  else
    frame->control &= ~MC_PENDING;
  if(GET_BIT(&flags, FLAG_ACK))
    frame->control |= MC_ACK;
  else
    frame->control &= ~MC_ACK;
  if(GET_BIT(&flags, FLAG_PANCOMP))
    frame->control |= MC_PANCOMP;
  else
    frame->control &= ~MC_PANCOMP;
}

static unsigned int load_uint(const char *s, unsigned int max)
{
  /* FIXME: We should use strtol_gen instead (when implemented). */
  unsigned int v = atoi_gen(s);

  if(v > max)
    errx(EXIT_FAILURE, "'%s' is too large (max: %d)", s, max);

  return v;
}

static const char * skip_leading_spaces(const char *s)
{
  for(; isspace(*s) ; s++)
    if(*s == '\0')
      errx(EXIT_FAILURE, "empty argument");
  return s;
}

static const char * load_uint_check(const char *s, unsigned int *v,
                                    unsigned int max)
{
  s = skip_leading_spaces(s);

  if(isdigit(*s)) {
    *v = load_uint(s, max);
    return NULL;
  }

  return s;
}

#define LOAD_UINT_PRECHECK(s, max)              \
  {                                             \
    unsigned int v;                             \
    s = load_uint_check(s, &v, max);            \
    if(!s)                                      \
      return v;                                 \
  }                                             \

static enum mac_type load_type(const char *s)
{
  LOAD_UINT_PRECHECK(s, MC_TYPE);

  if(!strcmp(s, "beacon"))
    return MT_BEACON;
  else if(!strcmp(s, "data"))
    return MT_DATA;
  else if(!strcmp(s, "ack"))
    return MT_ACK;
  else if(!strcmp(s, "cmd") || !strcmp(s, "command"))
    return MT_CMD;
  errx(EXIT_FAILURE, "invalid type -- '%s'", s);
}

static enum mac_addr_mode load_addr_mode(const char *s)
{
  LOAD_UINT_PRECHECK(s, MC_DAM >> MC_DAM_SHR);

  if(!strcmp(s, "full"))
    return MAM_FULL;
  else if(!strcmp(s, "reserved"))
    return MAM_RESERVED;
  else if(!strcmp(s, "short"))
    return MAM_SHORT;
  else if(!strcmp(s, "long"))
    return MAM_LONG;

  errx(EXIT_FAILURE, "invalid address mode -- '%s'", s);
}

static enum mac_version load_macver(const char *s)
{
  LOAD_UINT_PRECHECK(s, MC_VERSION  >> MC_VERSION_SHR);

  if(!strcmp(s, "2003"))
    return MV_2003;
  else if(!strcmp(s, "current"))
    return MV_CURRENT;

  errx(EXIT_FAILURE, "invalid MAC version -- '%s'", s);
}

static void eui64_check_byte(unsigned int value)
{
  if(value > 0xff)
    errx(EXIT_FAILURE, "cannot parse eui64");
}

static void u16_check_value(unsigned int value, const char *type)
{
  if(value > 0xffff)
    errx(EXIT_FAILURE, "cannot parse %s: too large", type);
}

static void load_eui64(const char *s, struct mac_addr *addr, unsigned int value)
{
  unsigned int i;
  uint64_t eui64 = value;

  for(i = 1 ; i < 7 ; i++) {
    s = parse_hex_until(++s, ":", &value, "cannot parse eui64", false);

    eui64_check_byte(value);

    eui64 <<= 8;
    eui64 +=  value;
  }

  s = parse_hex_until(++s, ":", &value, "cannot parse eui64", true);

  if(*s == ':')
    errx(EXIT_FAILURE, "%s: too long", "cannot parse eui64");

  eui64_check_byte(value);

  eui64 <<= 8;
  eui64 += value;

  addr->mac = eui64;
}

static void load_mac(const char *s, struct mac_addr *addr,
                     enum mac_addr_mode *mode)
{
  unsigned int value;

  s = parse_hex_until(++s, ":", &value, "cannot parse MAC address", true);

  switch(*s) {
  case ':':
    eui64_check_byte(value);

    *mode = MAM_FULL;
    load_eui64(s, addr, value);
    break;
  case '\0':
    u16_check_value(value, "MAC address");

    addr->mac = value;
    *mode = MAM_SHORT;
  }
}

static bool load_address(const char *s, struct mac_addr *addr,
                         enum mac_addr_mode *mode)
{
  unsigned int value;

  s = skip_leading_spaces(s);

  if(*s == '*' || *s == '#') {
    *mode = MAM_FULL;
    return false;
  }

  s = parse_hex_until(s, "-:", &value, "cannot parse address", true);

  switch(*s) {
  case '-':
    u16_check_value(value, "PAN-ID");

    addr->pan = value;
    load_mac(s, addr, mode);
    return true;
  case ':':
    eui64_check_byte(value);

    load_eui64(s, addr, value);
    *mode = MAM_LONG;
    return false;
  case '\0':
    u16_check_value(value, "MAC address");

    addr->mac = value;
    *mode = MAM_SHORT;
    return false;
  default:
    assert(0);
  }
}

static void file_action(struct mac_frame *frame, const char *filename,
                        int (*action)(struct mac_frame *,
                                      const unsigned char *,
                                      unsigned int),
                        const char *warning_message)
{
  unsigned char buf[128];
  ssize_t n;
  int fd = open(filename, O_RDONLY);

  if(fd < 0)
    err(EXIT_FAILURE, "cannot open '%s'", filename);

  n = read(fd, buf, 128);
  if(n < 0)
    err(EXIT_FAILURE, "cannot read '%s'", filename);

  if(action(frame, buf, n) < 0)
    errx(EXIT_FAILURE, "%s", warning_message);

  close(fd);
}

static int mac_decode_crc(struct mac_frame *frame, const unsigned char *data,
                          unsigned int size)
{
  return mac_decode(frame, data, false, size);
}

static int copy_payload(struct mac_frame *frame, const unsigned char *data,
                        unsigned int size)
{
  if(frame->payload)
    free((void *)frame->payload);
  frame->payload = malloc(size);
  memcpy((void *)frame->payload, data, size);
  frame->size = size;
  return 0;
}

void parse_frame_from_file(struct mac_frame *frame, const char *filename)
{
  file_action(frame, filename, mac_decode_crc, "cannot decode frame");
}

void setup_payload_from_file(struct mac_frame *frame, const char *filename)
{
  file_action(frame, filename, copy_payload, "cannot copy payload");
}


void parse_type(struct mac_frame *frame, const char *type_arg)
{
  enum mac_type type = load_type(type_arg);

  frame->control &= ~MC_TYPE;
  frame->control |= type;
}

void parse_reserved(struct mac_frame *frame, const char *reserved_arg)
{
  unsigned int reserved = load_uint(reserved_arg, MC_RESERVED >> MC_RESERVED_SHR);

  frame->control &= ~MC_RESERVED;
  frame->control |= reserved << MC_RESERVED_SHR;
}

void parse_sam(struct mac_frame *frame, const char *sam_arg)
{
  enum mac_addr_mode mode = load_addr_mode(sam_arg);

  frame->control &= ~MC_SAM;
  frame->control |= mode << MC_SAM_SHR;
}

void parse_dam(struct mac_frame *frame, const char *dam_arg)
{
  enum mac_addr_mode mode = load_addr_mode(dam_arg);

  frame->control &= ~MC_DAM;
  frame->control |= mode << MC_DAM_SHR;
}

void parse_version(struct mac_frame *frame, const char *version_arg)
{
  enum mac_version version = load_macver(version_arg);

  frame->control &= ~MC_VERSION;
  frame->control |= version << MC_VERSION_SHR;
}

void parse_saddr(struct mac_frame *frame, const char *addr_arg)
{
  enum mac_addr_mode sam;
  bool pan = load_address(addr_arg, &frame->src, &sam);

  /* Setup PAN ID compression when needed. */
  if(!pan)
    frame->control |= MC_PANCOMP;
  else
    frame->control &= ~MC_PANCOMP;

  /* Setup SAM */
  frame->control &= ~MC_SAM;
  frame->control |= sam << MC_SAM_SHR;
}

void parse_daddr(struct mac_frame *frame, const char *addr_arg)
{
  enum mac_addr_mode dam;
  bool pan = load_address(addr_arg, &frame->dst, &dam);

  /* Issue a warning when the PAN ID is not specified
     and the address isn't fully compressed.
     (see IEEE 802.15.4 section 5.2.1.1.5) */
  if(!pan && dam != MAM_FULL)
    warnx("no PAN ID for source address");

  /* Setup SAM */
  frame->control &= ~MC_DAM;
  frame->control |= dam << MC_DAM_SHR;
}

void parse_seqno(struct mac_frame *frame, const char *seqno_arg)
{
  frame->seqno = load_uint(seqno_arg, 0xff);
}

void parse_flag_enable(struct mac_frame *frame, unsigned int flag)
{
  frame->control |= flag;
}

void parse_flag_disable(struct mac_frame *frame, unsigned int flag)
{
  frame->control &= ~flag;
}

void setup_default_frame(struct mac_frame *frame)
{
  /* setup mac control field */
  frame->control  = 0;
  frame->control |= MT_DATA;
  frame->control |= MAM_FULL << MC_DAM_SHR;
  frame->control |= MAM_FULL << MC_DAM_SHR;
  frame->control |= MV_2003  << MC_VERSION_SHR;

  /* setup sequence number */
  frame->seqno = 0;

  /* setup addresses and PAN ID
     if any we choose broadcast */
  frame->src.pan = 0xffff;
  frame->src.mac = 0xffffffffffffffffLL;
  frame->src.pan = 0xffff;
  frame->src.mac = 0xffffffffffffffffLL;

  /* disable payload and security */
  frame->security = NULL;
  frame->payload  = NULL;
  frame->size     = 0;

  /* we do not compute fcs ourself */
  frame->fcs      = 0;
}
