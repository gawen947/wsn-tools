/* File: wsn-sniffer-cli.c
   Time-stamp: <2013-03-18 20:43:26 gawen>

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
#include "event.h"
#include "uart.h"
#include "uart-input.h"
#include "mac-decode.h"
#include "mac-display.h"

#define TARGET "Sniffer-CLI"

static bool payload;
static unsigned int mac_info;
/* static unsigned int payload_info; */

static void event(const unsigned char *data, enum event event_type, size_t size)
{
  struct mac_frame frame;

  switch(event_type) {
  case(EV_FRAME):
    /* We except a raw frame so we don't need to renormalize anything. */
    if(mac_decode(&frame, data, size) < 0) {
#ifndef NDEBUG
      hex_dump(data, size);
#endif /* NDEBUG */
      warnx("cannot decode frame");
    }

    /* Display the frame live. */
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

    break;
  case(EV_INFO):
    write(STDOUT_FILENO, data, size);
    break;
  default:
    warnx("invalid event ignored");
#ifndef NDEBUG
    hex_dump(data, size);
#endif /* NDEBUG */
    break;
  }
}

static void cleanup(void)
{
  /* Ensure that the PCAP file is closed properly to flush buffers. */
  destroy_pcap();
}

static void sig_cleanup(int signum)
{
  exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
  struct sigaction act = { .sa_handler = sig_cleanup };
  const char *name;
  const char *tty  = NULL;
  const char *pcap = NULL;
  speed_t speed = B0;

  int exit_status = EXIT_FAILURE;

  name = (const char *)strrchr(argv[0], '/');
  name = name ? (name + 1) : argv[0];

  enum opt {
    OPT_COMMIT = 0x100
  };

  struct opt_help helps[] = {
    { 'h', "help", "Show this help message" },
    { 'V', "version", "Print version information" },
#ifdef COMMIT
    { 0, "commit", "Display commit information" },
#endif /* COMMIT */
    { 'b', "baud", "Specify the baud rate"},
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
    int c = getopt_long(argc, argv, "hVp:cb:saSMPA", opts, NULL);

    if(c == -1)
      break;

    switch(c) {
    case('p'):
      pcap = optarg;
      break;
    case('b'):
      speed = baud(optarg);
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

  /* Register the cleanup function as the most
     common way to leave the event loop is SIGINT.
     The program may also quit because of an error
     or the SIGTERM signal. So we need to register
     an exit hook and signals too. */
  sigfillset(&act.sa_mask);
  sigaction(SIGTERM, &act, NULL);
  sigaction(SIGINT, &act, NULL);
  atexit(cleanup);

  uart_input_loop(tty, speed, event);

EXIT:
  return exit_status;
}
