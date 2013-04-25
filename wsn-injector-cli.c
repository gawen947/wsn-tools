/* File: wsn-tools-cli.c

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

#include <stdio.h>
#include <sys/time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <assert.h>
#include <unistd.h>
#include <err.h>

#include "mac.h"
#include "mac-decode.h"
#include "mac-encode.h"
#include "mac-display.h"
#include "mac-parse.h"
#include "802154-parse.h"
#include "getflg.h"
#include "version.h"
#include "protocol.h"
#include "protocol-mqueue.h"
#include "signal-utils.h"
#include "string-utils.h"
#include "dump.h"
#include "uart.h"
#include "input.h"
#include "help.h"

#define TARGET "Injector-CLI"

#define FORCED_SAM 0x01
#define FORCED_DAM 0x02

#define PAN_SOURCE      0x01
#define PAN_DESTINATION 0x02

enum input_state { ST_NONE,
                   ST_WAITING_OK,
                   ST_WAITING_ACK };

static enum input_state state;
static struct mac_frame frame;
static prot_mqueue_t mqueue;
static int uart_fd;
static bool ack;

struct addressing {
  /* specified from command line */
  enum mac_addr_mode forced_sam;
  enum mac_addr_mode forced_dam;
  unsigned int forced;

  /* specified from parsed addresses */
  enum mac_addr_mode real_sam;
  enum mac_addr_mode real_dam;
  struct mac_addr saddr;
  struct mac_addr daddr;
  unsigned int pan_specified;
};

static void strtolower(char *s)
{
  for(; *s != '\0' ; s++)
    *s = tolower(*s);
}

static void write_to_file(const char *filename, const char *error,
                          const unsigned char *data, unsigned int size)
{
  ssize_t n;
  int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if(fd < 0)
    err(EXIT_FAILURE, "cannot open '%s'", filename);

  n = write(fd, data, size);
  if(n < 0)
    err(EXIT_FAILURE, "cannot write %s", error);
  else if(n != size)
    errx(EXIT_FAILURE, "cannot write %s", error);

  close(fd);
}

static bool parse_ok(void)
{
  if(state != ST_WAITING_OK)
    errx(EXIT_FAILURE, "unexpected OK");

  if(ack) {
    /* now that we sent our message we may wait for the ACK */
    state = ST_WAITING_ACK;
    return true;
  }
  else {
    /* since we don't want an ACK for this message, we stop here */
    return false;
  }
}

static bool parse_ack(void)
{
  if(!ack || state != ST_WAITING_ACK)
    errx(EXIT_FAILURE, "unexpected ACK");

  /* FIXME: We should display informations here. */
  return false;
}

static bool parse_control_message(enum prot_ctype type,
                                  const unsigned char *data,
                                  size_t size)
{
  switch(type) {
  case(PROT_CTYPE_ACK):
    return parse_ack();
  case(PROT_CTYPE_OK):
    return parse_ok();
  default:
    errx(EXIT_FAILURE, "unmanaged control message %s", prot_ctype_string(type));
  }
}

static bool message_cb(const unsigned char *data,
                       enum prot_mtype type,
                       size_t size)
{
  if(size == 0)
    errx(EXIT_FAILURE, "empty message");

  switch(type) {
  case(PROT_MTYPE_FRAME):
    errx(EXIT_FAILURE, "unexpected frame message: "
         "are you sure this is a injector firmware ?");
  case(PROT_MTYPE_CONTROL):
    /* When we receive a control message we always stay in the reading loop
       except when we receive an ACK. */
    if(!prot_preparse_control(data, size))
      return parse_control_message(data[0], data + 1, size - 1);
    return true;
  default:
    warnx("invalid event ignored");
#ifndef NDEBUG
    hex_dump(data, size);
#endif /* NDEBUG */
    exit(EXIT_FAILURE);
  }

  /* This function is always in bad mood so it always returns false. */
  return false;
}

static void cleanup(void)
{
  /* exit the file descriptor if we need to */
  if(uart_fd)
    close(uart_fd);

  /* we have to free the message queue here */
  prot_mqueue_destroy(mqueue);

  /* don't forget to free the frame (and associated payload) */
  free_mac_frame(&frame);
}

static void sig_cleanup(int signum)
{
  exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
  unsigned char frame_buffer[127];
  int frame_size;

  bool dryrun  = false;
  bool display = false;
  bool random  = false;
  const char *name;
  const char *tty         = NULL;
  const char *frame_out   = NULL;
  const char *payload_out = NULL;
  const char *header_out  = NULL;
  unsigned short channel;
  speed_t speed = B0;
  int timeout = 0;

  /* working frame */
  setup_default_frame(&frame);

  int exit_status = EXIT_FAILURE;

  name = (const char *)strrchr(argv[0], '/');
  name = name ? (name + 1) : argv[0];

  /* The message queue must be initialized before parsing the arguments. */
  mqueue = prot_mqueue_creat();

  enum opt {
    OPT_COMMIT = 0x100,
    OPT_RESERVED,
    OPT_MACVER,
    OPT_EPENDING,
    OPT_DPENDING,
    OPT_SAM,
    OPT_DAM,
    OPT_EACK,
    OPT_DACK,
    OPT_EPANCOMP,
    OPT_DPANCOMP,
    OPT_WRITE_FRAME,
    OPT_WRITE_PAYLOAD,
    OPT_WRITE_HEADER,
    OPT_RANDOM_PAYLOAD
  };

  struct opt_help helps[] = {
    { 'h', "help", "Show this help message" },
    { 'V', "version", "Print version information" },
#ifdef COMMIT
    { 0, "commit", "Display commit information" },
#endif /* COMMIT */
    { 'T', "timeout", "Specify the timeout"},
    { 'C', "channel", "Configure the channel" },
    { 'n', "dry-run", "Do not send the frame on UART" },
    { 'D', "display", "Display the reconstructed frame" },
    { 'b', "baud", "Specify the baud rate"},
    { 'f', "frame", "Load the base frame from a file" },
    { 'F', "flag", "Setup flags for the frame control field" },
    { 't', "type", "Change the type of frame" },
    { 0, "reserved", "Assign a non-standard value to the reserved field" },
    { 0, "sam", "Force the source address mode" },
    { 0, "dam", "Force the destination address mode" },
    { 0, "mac-version", "Set the MAC version" },
    { 's', "saddr", "Set the source address" },
    { 'd', "daddr", "Set the destination address" },
    { 'p', "payload", "Load the payload from a file" },
    { 'S', "seqno", "Set the sequence number" },
    { 0, "enable-pending", "Enable the pending flag" },
    { 0, "disable-pending", "Disable the pending flag" },
    { 0, "enable-ack", "Enable the ACK request flag" },
    { 0, "disable-ack", "Disable the ACK request flag" },
    { 0, "enable-pan-comp", "Enable the PAN-ID compression flag" },
    { 0, "disable-pan-comp", "Disable the PAN-ID compression flag" },
    { 0, "write-frame", "Write the resulting frame to a file" },
    { 0, "write-payload", "Write the payload to a file" },
    { 0, "write-header", "Write the header to a file" },
    { 0, "random-payload", "Fill the frame with a random payload" },
    { 0, NULL, NULL }
  };

  struct option long_opts[] = {
    { "help", no_argument, NULL, 'h' },
    { "version", no_argument, NULL, 'V'},
#ifdef COMMIT
    { "commit", no_argument, NULL, OPT_COMMIT },
#endif /* COMMIT */
    { "timeout", required_argument, NULL, 'T' },
    { "channel", required_argument, NULL, 'C' },
    { "dry-run", no_argument, NULL, 'n' },
    { "display", no_argument, NULL, 'D' },
    { "baud", required_argument, NULL, 'b' },
    { "frame", required_argument, NULL, 'f' },
    { "flag", required_argument, NULL, 'F' },
    { "type", required_argument, NULL, 't' },
    { "reserved", required_argument, NULL, OPT_RESERVED },
    { "sam", required_argument, NULL, OPT_SAM },
    { "dam", required_argument, NULL, OPT_DAM },
    { "mac-version", required_argument, NULL, OPT_MACVER },
    { "saddr", required_argument, NULL, 's' },
    { "daddr", required_argument, NULL, 'd' },
    { "payload", required_argument, NULL, 'p' },
    { "seqno", required_argument, NULL, 'S' },
    { "enable-pending", no_argument, NULL, OPT_EPENDING },
    { "disable-pending", no_argument, NULL, OPT_DPENDING },
    { "enable-ack", no_argument, NULL, OPT_EACK },
    { "disable-ack", no_argument, NULL, OPT_DACK },
    { "enable-pan-comp", no_argument, NULL, OPT_EPANCOMP },
    { "disable-pan-comp", no_argument, NULL, OPT_DPANCOMP },
    { "write-frame", required_argument, NULL, OPT_WRITE_FRAME },
    { "write-payload", required_argument, NULL, OPT_WRITE_PAYLOAD },
    { "write-header", required_argument, NULL, OPT_WRITE_HEADER },
    { "random-payload", no_argument, NULL, OPT_RANDOM_PAYLOAD },
    { NULL, 0, NULL, 0 }
  };

  while(1) {
    int c = getopt_long(argc, argv, "hVnDb:T:C:f:F:t:s:d:p:S:", long_opts, NULL);

    if(c == -1)
      break;

    switch(c) {
    case 'f':
      parse_frame_from_file(&frame, optarg);
      break;
    case 'F':
      parse_flags(&frame, optarg);
      break;
    case 'n':
      dryrun = true;
      break;
    case 'b':
      speed = baud(optarg);
      break;
    case 'D':
      display = true;
      break;
    case('T'):
      timeout = atoi(optarg);
      if(timeout <= 0)
        errx(EXIT_FAILURE, "invalid timeout value");
      break;
    case('C'):
      channel = parse_channel(optarg);
      prot_mqueue_add_control(mqueue,
                              PROT_CTYPE_CONFIG_CHANNEL,
                              &channel,
                              sizeof(unsigned short));
      break;
    case 't':
      strtolower(optarg);
      parse_type(&frame, optarg);
      break;
    case OPT_RESERVED:
      parse_reserved(&frame, optarg);
      break;
    case OPT_SAM:
      strtolower(optarg);
      parse_sam(&frame, optarg);
      break;
    case OPT_DAM:
      strtolower(optarg);
      parse_dam(&frame, optarg);
      break;
    case OPT_MACVER:
      strtolower(optarg);
      parse_version(&frame, optarg);
      break;
    case OPT_WRITE_FRAME:
      frame_out = optarg;
      break;
    case OPT_WRITE_PAYLOAD:
      payload_out = optarg;
      break;
    case OPT_WRITE_HEADER:
      header_out = optarg;
      break;
    case OPT_RANDOM_PAYLOAD:
      random = true;
      break;
    case 's':
      parse_saddr(&frame, optarg);
      break;
    case 'd':
      parse_daddr(&frame, optarg);
      break;
    case 'p':
      setup_payload_from_file(&frame, optarg);
      break;
    case 'S':
      parse_seqno(&frame, optarg);
      break;
    case OPT_EPENDING:
      parse_flag_enable(&frame, MC_PENDING);
      break;
    case OPT_DPENDING:
      parse_flag_disable(&frame, MC_PENDING);
      break;
    case OPT_EACK:
      parse_flag_enable(&frame, MC_ACK);
      break;
    case OPT_DACK:
      parse_flag_disable(&frame, MC_ACK);
      break;
    case OPT_EPANCOMP:
      parse_flag_enable(&frame, MC_PANCOMP);
      break;
    case OPT_DPANCOMP:
      parse_flag_disable(&frame, MC_PANCOMP);
      break;
#ifdef COMMIT
    case OPT_COMMIT:
      commit();
      exit_status = EXIT_SUCCESS;
      goto EXIT;
#endif /* COMMIT */
    case 'V':
      version(TARGET);
      exit_status = EXIT_SUCCESS;
      goto EXIT;
    case 'h':
      exit_status = EXIT_SUCCESS;
    default:
      help(name, "[OPTIONS] ... TTY", helps);
      goto EXIT;
    }
  }

  /* We don't need a tty when we do a dryrun. */
  if(!dryrun) {
    if((argc - optind) != 1)
      errx(EXIT_FAILURE, "except tty device");

    tty = argv[optind];
  }

  /* try to encode first */
  frame_size = mac_encode(&frame, frame_buffer);
  if(frame_size < 0)
    errx(EXIT_FAILURE, "cannot encode frame");

  /* fill the payload with random padding */
  if(random) {
    unsigned int size = sizeof(frame_buffer) - frame_size;
    unsigned char *new_payload = malloc(frame.size + size);

    memcpy(new_payload, frame.payload, frame.size);
    free((void *)frame.payload);

    fill_with_random(new_payload + frame.size, size);
    frame.size    += size;
    frame.payload  = new_payload;

    /* encode again */
    frame_size = mac_encode(&frame, frame_buffer);
    if(frame_size < 0)
      errx(EXIT_FAILURE, "cannot encode frame");
  }

  /* display if requested */
  if(display) {
    mac_display(&frame, MI_ALL);
    if(frame.payload) {
      printf("Payload:\n");
      hex_dump(frame.payload, frame.size);
    }
    putchar('\n');
  }

  /* save the ACK state */
  ack = frame.control & MC_ACK;

  /* write frame if requested */
  if(frame_out)
    write_to_file(frame_out, "frame", frame_buffer, frame_size);

  /* write payload if requested */
  if(payload_out)
    write_to_file(payload_out, "payload", frame.payload, frame.size);

  /* write header if requested */
  if(header_out)
    write_to_file(header_out, "header", frame_buffer, frame_size - frame.size);

  /* Register the cleanup function as the most common way to leave the event
     loop is SIGINT. The program may also quit because of an error or the
     SIGTERM signal. So we need to register an exit hook and signals too. A
     setup function will register all signals and for us so we only care about
     the cleanup functions themselves. */
  setup_sig(cleanup, sig_cleanup, NULL);

  /* Send the frame. */
  if(!dryrun) {
    int ret;

    uart_fd = open_uart(tty, speed);

    /* Initialisation of the transceiver
       with a set of commands. */
    prot_mqueue_sendall(mqueue, uart_fd);

    /* Write the frame to the transceiver */
    prot_write(uart_fd, PROT_MTYPE_FRAME, frame_buffer, frame_size);

    /* Read until timeout if an ACK was requested. */
    state = ST_WAITING_OK;
    ret = input_loop(uart_fd, message_cb, NULL, timeout);

    switch(ret) {
    case(-1):
      warnx("there are messages left on the buffer");
      break;
    case(-2):
      warnx(ack ? "ACK reception has timed out" :
                  "Processing confirmation has timed out");
      break;
    }
  }

  exit_status = EXIT_SUCCESS;

EXIT:
  return exit_status;
}
