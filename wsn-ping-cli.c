/* File: wsn-ping-cli.c

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
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include <string.h>
#include <getopt.h>
#include <termios.h>
#include <unistd.h>
#include <err.h>

#include "version.h"
#include "protocol.h"
#include "string-utils.h"
#include "uart.h"
#include "dump.h"
#include "help.h"
#include "input.h"
#include "crc32.h"

#define TARGET "Ping-CLI"

/* We have a queue which represents ping messages that are 'on air'.
   To do that simply we use a pipe between the parent and his child.
   This however may introduce some unwanted delay and we can do that
   better. But we really want to keep it simplier as we want to test
   the communication channel, the firmware and the protocol. */
static int air_queue[2];

/* Statistics */
static unsigned int error;
static unsigned int ok;
static unsigned long long sum;
static unsigned long long sq_sum;

static bool flood;

/* This structure represents ping messages lying on the communication channel or
   being processed by the firmware. */
struct on_air {
  unsigned int seqno;
  unsigned int size;
  struct timeval tv;
};

static void apply_crc(const unsigned char *sbuf, size_t size)
{
  unsigned long crc;

  crc = crc32(sbuf, size, 0);

  *((unsigned long *)(sbuf + size)) = crc;
}

static bool check_crc(const unsigned char *buf, size_t size)
{
  unsigned long read_crc = *((unsigned long *)(buf + size));
  unsigned long comp_crc;

  comp_crc = crc32(buf, size, 0);

  return read_crc == comp_crc;
}

static void print_ping(const struct timeval *start,
                       const struct timeval *end,
                       unsigned int size,
                       unsigned int seqno,
                       const char *flood_status,
                       char status)
{
  if(flood)
    write_slit(STDOUT_FILENO, flood_status);
  else {
    struct timeval delta;
    timersub(end, start, &delta);
    printf("(%c) %d bytes: ping_req=%d time=%s\n", status, size, seqno,
           tv_to_str(&delta));
  }
}

static void parse_ping_message(const unsigned char *data, size_t size)
{
  struct on_air air;
  struct timeval tv;
  unsigned int seqno;

  gettimeofday(&tv, NULL); /* time received */

  /* Extract the packet from the air queue */
  read(air_queue[0], &air, sizeof(struct on_air));

  /* Check CRC */
  if(!check_crc(data, size - 2)) {
    error++;
    print_ping(&air.tv, &tv, air.size, air.seqno, "\bE", 'E');
    return;
  }

  /* Check if the sequence number match the on air packet. */
  seqno = *((unsigned int *)data);
  if(seqno != air.seqno) {
    error++;
    print_ping(&air.tv, &tv, air.size, air.seqno, "\be", 'e');
    return;
  }

  print_ping(&air.tv, &tv, air.size, air.seqno, "\b", '*');
  ok++;
}

static void parse_control_message(enum prot_ctype type,
                                  const unsigned char *data,
                                  size_t size)
{
  switch(type) {
  case(PROT_CTYPE_PING):
    parse_ping_message(data, size);
    break;
  default:
    /* We simply ignores other control messages.
       We already captured the more common types
       with prot_preparse_control. */
    break;
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
    /* We intentionally ignore frame messages
       because we may ping a sniffer firmware. */
    break;
  case(PROT_MTYPE_CONTROL):
    if(!prot_preparse_control(data, size))
      parse_control_message(data[0], data + 1, size - 1);
    break;
  default:
    warnx("invalid event ignored");
#ifndef NDEBUG
    hex_dump(data, size);
#endif /* NDEBUG */
    exit(EXIT_FAILURE);
  }

  return true;
}

int main(int argc, char *argv[])
{
  const char *name;
  const char *tty = NULL;
  speed_t speed = B0;
  int interval  = 0;
  int count     = -1;
  int size      = 64;

  pid_t pid;
  int fd;

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
    { 'b', "baud", "Specify the baud rate" },
    { 's', "size", "Number of data bytes to be send (max: 118)" },
    { 'c', "count", "Stop after sending count messages" },
    { 'f', "flood", "Use a period/backspace display for the messages sent" },
    { 'i', "interval", "Wait interval microseconds between each message" },
    { 0, NULL, NULL }
  };

  struct option opts[] = {
    { "help", no_argument, NULL, 'h' },
    { "version", no_argument, NULL, 'V' },
#ifdef COMMIT
    { "commit", no_argument, NULL, OPT_COMMIT },
#endif /* COMMIT */
    { "baud", required_argument, NULL, 'b' },
    { "size", required_argument, NULL, 's' },
    { "adaptive", no_argument, NULL, 'A' },
    { "count", required_argument, NULL, 'c' },
    { "flood", no_argument, NULL, 'f' },
    { "interval", required_argument, NULL, 'i' },
    { NULL, 0, NULL, 0 }
  };

  while(1) {
    int c = getopt_long(argc, argv, "hVb:s:Ac:fi:", opts, NULL);

    if(c == -1)
      break;

    switch(c) {
    case('i'):
      interval = atoi(optarg);
      if(interval < 0)
        errx(EXIT_FAILURE, "invalid interval value");
      break;
    case('c'):
      count = atoi(optarg);
      if(count <= 0)
        errx(EXIT_FAILURE, "invalid count value");
      break;
    case('s'):
      size = atoi(optarg);
      if(size < 0 || size > 118)
        errx(EXIT_FAILURE, "invalid size value");
      break;
    case('f'):
      flood = true;
      break;
    case('b'):
      speed = baud(optarg);
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
      help(name, "[OPTIONS] ... TTY", helps);
      goto EXIT;
    }
  }

  if((argc - optind) != 1)
    errx(EXIT_FAILURE, "except tty device");

  tty = argv[optind];

   /* Register the cleanup function as the most common way to leave the event
     loop is SIGINT. The program may also quit because of an error or the
     SIGTERM signal. So we need to register an exit hook and signals too. A
     setup function will register all signals and for us so we only care about
     the cleanup functions themselves. */
//  setup_sig(cleanup, sig_cleanup, sig_flush);

  /* That's where we really start the operations */
  fd = open_uart(tty, speed);

  /* We create the air queue (see comment about the air queue above) */
  if(pipe(air_queue) < 0)
    err(EXIT_FAILURE, "cannot create air queue");

  /* We divide the ping process in two. The parent sends pings and the child
     receives and displays the responses. We proceed that way to ensure that we
     use the same mechanisms (in particular the read buffer) that are used in
     others clients in order to flow the steam in the pipes. */
  pid = fork();
  if(pid < 0)
    err(EXIT_FAILURE, "cannot fork");
  else if(pid) { /* parent */
    unsigned int seqno = 0;
    unsigned char sbuf[128];

    /* We setup the send buffer. Note that we construct the ping message manually.
       We do this to avoid copying the message at each ping we want to send. */
    sbuf[0] = PROT_MTYPE_CONTROL | (size + 8 + 1); /* size +
                                                      control(8) +
                                                      seqno(32) +
                                                      crc(32) */

    /* Control message type. */
    sbuf[1] = PROT_CTYPE_PING;

    /* We fill the message with random bytes. */
    fill_with_random(sbuf + 2 /* mtype + ctype */ + 4 /* seqno */, size);

    while(1) {
      struct on_air air;

      usleep(interval);

      /* Setup the sequence number */
      *((unsigned int *)(sbuf + 2)) = seqno;
      apply_crc(sbuf + 2, size + 4);

      /* Send this message */
      write(fd, sbuf, size + 2 + 4 + 4);

      /* Create the ping template */
      gettimeofday(&air.tv, NULL);
      air.seqno = seqno;
      air.size  = size + 2 + 4 + 4;

      /* Add the message to the queue */
      write(air_queue[1], &air, sizeof(struct on_air));

      /* Stdout if needed */
      if(flood)
        write_slit(STDOUT_FILENO, ".");

      seqno++;

      if(count != -1 && !count--)
        break;
    }
  }
  else { /* child */
    input_loop(fd, message_cb, NULL, 0);
  }

  exit_status = EXIT_SUCCESS;

EXIT:
  return exit_status;
}
