/* File: wsn-sniffer-cli.c

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

#define _POSIX_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <string.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <err.h>

#include "version.h"
#include "pcap.h"
#include "dump.h"
#include "help.h"
#include "uart.h"
#include "input.h"
#include "protocol.h"
#include "protocol-mqueue.h"
#include "signal-utils.h"
#include "mac-decode.h"
#include "mac-display.h"

#define TARGET "Sniffer-CLI"

static bool payload;
static unsigned int mac_info;
/* static unsigned int payload_info; */

static prot_mqueue_t mqueue;

static void parse_frame_message(const unsigned char *data, size_t size)
{
  struct mac_frame frame;

  /* We   except a raw frame so we don't need to renormalize anything. */
  if(mac_decode(&frame, data, true, size) < 0) {
#ifndef NDEBUG
    hex_dump(data, size);
#endif /* NDEBUG */
    warnx("cannot decode frame");

    /* We do not show invalid frame as most of it
       is probably uninitialized garbage. */
    free_mac_frame(&frame);
    return;
  }

  /*  Display the frame live. */
  mac_display(&frame, mac_info);

  /* For now we do not try decode payload.
     Instead we just dump the packet. */
  if(payload && frame.payload) {
    printf("Payload:\n");
    hex_dump(frame.payload, frame.size);
  }

  putchar('\n');

  /* Append the frame to the PCAP file. */
  append_frame(data, size);

  /* FIXME: This particular free call may be spared if we provided a way for
     mac_decode to avoid copying the payload. */
  free_mac_frame(&frame);
}

static bool message_cb(const unsigned char *data,
                       enum prot_mtype type,
                       size_t size)
{
  if(size == 0)
    errx(EXIT_FAILURE, "empty message");

  switch(type) {
  case(PROT_MTYPE_FRAME):
    parse_frame_message(data, size);
    break;
  case(PROT_MTYPE_CONTROL):
    /* We do not accept any control message for the sniffer.
       Except the common ones. */
    if(!prot_preparse_control(data, size))
      errx(EXIT_FAILURE, "unmanaged control message %s",
           prot_ctype_string(data[0]));
    break;
  default:
    warnx("invalid event ignored");
#ifndef NDEBUG
    hex_dump(data, size);
#endif /* NDEBUG */
    exit(EXIT_FAILURE);
  }

  /* This function is always happy so it always returns true. */
  return true;
}

static void cleanup(void)
{
  /* Ensure that the PCAP file is closed properly to flush buffers. */
  destroy_pcap();

  /* Destroy the message queue. */
  prot_mqueue_destroy(mqueue);
}

static void sig_cleanup(int signum)
{
  exit(EXIT_SUCCESS);
}

static void sig_flush(int signum)
{
  flush_pcap();
}

int main(int argc, char *argv[])
{
  const char *name;
  const char *tty  = NULL;
  const char *pcap = NULL;
  speed_t speed = B0;
  int timeout = 0;
  int fd;

  int exit_status = EXIT_FAILURE;

  name = (const char *)strrchr(argv[0], '/');
  name = name ? (name + 1) : argv[0];

  /* The message queue must be initialized before parsing the arguments. */
  mqueue = prot_mqueue_creat();

  enum opt {
    OPT_COMMIT = 0x100
  };

  struct opt_help helps[] = {
    { 'h', "help", "Show this help message" },
    { 'V', "version", "Print version information" },
#ifdef COMMIT
    { 0, "commit", "Display commit information" },
#endif /* COMMIT */
    { 'T', "timeout", "Specify the timeout" },
    { 'C', "channel", "Configure the channel" },
    { 'b', "baud", "Specify the baud rate" },
    { 'p', "pcap", "Save packets in the specified PCAP file" },
    { 'c', "show-control", "Display frame control information" },
    { 's', "show-seqno", "Display sequence number" },
    { 'a', "show-addr", "Display addresses fields" },
    { 'S', "show-security", "Display security auxiliary field" },
    { 'M', "show-mac", "Display all informations about MAC frames" },
    { 'P', "show-payload", "Try to decode and display the payload" },
    { 'A', "show-all", "Display all informations" },
    { 0, NULL, NULL }
  };

  struct option opts[] = {
    { "help", no_argument, NULL, 'h' },
    { "version", no_argument, NULL, 'V' },
#ifdef COMMIT
    { "commit", no_argument, NULL, OPT_COMMIT },
#endif /* COMMIT */
    { "timeout", required_argument, NULL, 'T'},
    { "channel", required_argument, NULL, 'C' },
    { "baud", required_argument, NULL, 'b' },
    { "pcap", required_argument, NULL, 'p' },
    { "show-control", no_argument, NULL, 'c' },
    { "show-seqno", no_argument, NULL, 's' },
    { "show-addr", no_argument, NULL, 'a' },
    { "show-security", no_argument, NULL, 'S' },
    { "show-mac", no_argument, NULL, 'M' },
    { "show-payload", no_argument, NULL, 'P' },
    { "show-all", no_argument, NULL, 'A' },
    { NULL, 0, NULL, 0 }
  };

  while(1) {
    int c = getopt_long(argc, argv, "hVp:cb:T:saSMPA", opts, NULL);

    if(c == -1)
      break;

    switch(c) {
    case('p'):
      pcap = optarg;
      break;
    case('b'):
      speed = baud(optarg);
      break;
    case('T'):
      timeout = atoi(optarg);
      if(timeout <= 0)
        errx(EXIT_FAILURE, "invalid timeout value");
      break;
    case('c'):
      mac_info |= MI_CONTROL;
      break;
    case('s'):
      mac_info |= MI_SEQNO;
      break;
    case('a'):
      mac_info |= MI_ADDR;
      break;
    case('S'):
      mac_info |= MI_SECURITY;
      break;
    case('M'):
      mac_info = MI_ALL;
      break;
    case('P'):
      payload = true;
      break;
    case('A'):
      mac_info = MI_ALL;
      /* TODO: payload_info = PI_ALL */
      break;
#ifdef COMMIT
    case(OPT_COMMIT):
      commit();
      exit_status = EXIT_SUCCESS;
      goto EXIT;
#endif /* COMMIT */
    case('V'):
      version(TARGET);
      exit_status = EXIT_SUCCESS;
      goto EXIT;
    case('h'):
      exit_status = EXIT_SUCCESS;
    default:
      help(name, "[OPTIONS] ... BAUD-RATE TTY", helps);
      goto EXIT;
    }
  }

  if((argc - optind) != 1)
    errx(EXIT_FAILURE, "except tty device");

  tty = argv[optind];

  if(!pcap && !mac_info /* && !payload_info */)
    warnx("doing nothing as requested");

  if(pcap)
    init_pcap(pcap);

  /* Register the cleanup function as the most common way to leave the event
     loop is SIGINT. The program may also quit because of an error or the
     SIGTERM signal. So we need to register an exit hook and signals too. A
     setup function will register all signals and for us so we only care about
     the cleanup functions themselves. */
  setup_sig(cleanup, sig_cleanup, sig_flush);

  /* That's where we really start the operations. */
  fd = open_uart(tty, speed);

  /* Initialisation of the transceiver
     with a set of commands. */
  prot_mqueue_sendall(mqueue, fd);

  /* Read until timeout (if requested). */
  input_loop(fd, message_cb, "Waiting", timeout);
  exit_status = EXIT_SUCCESS;

EXIT:
  return exit_status;
}
