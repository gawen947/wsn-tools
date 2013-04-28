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
#define _POSIX_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <termios.h>
#include <unistd.h>
#include <limits.h>
#include <math.h>
#include <err.h>

#include "version.h"
#include "protocol.h"
#include "string-utils.h"
#include "signal-utils.h"
#include "uart.h"
#include "dump.h"
#include "help.h"
#include "input.h"
#include "crc32.h"

#define TARGET "Ping-CLI"

/* The sequence number size may change.
   The timevalue will always be defined
   by its structure and the crc will
   always be 32 bits wide. */
typedef unsigned short seqno_t;

#define PING_HDR_SIZE sizeof(seqno_t) + sizeof(struct timeval)
#define PING_CRC_SIZE sizeof(uint32_t)

#define MAX_PING_PADDING_SIZE MAX_MESSAGE_SIZE + 1 -              \
                              (PING_HDR_SIZE + PING_CRC_SIZE + 2)

/* Statistics */
static unsigned int error;
static unsigned int ok;
static unsigned long long min = ULLONG_MAX;
static unsigned long long max = 0;
static unsigned long long sum;
static unsigned long long sq_sum;

static bool flood;
static int count  = -1;
static pid_t pid;
static int fd;

static void apply_crc(const unsigned char *sbuf, size_t size)
{
  unsigned long crc;

  crc = crc32(sbuf, size, 0);

  *((unsigned long *)(sbuf + size)) = crc;
}

static bool check_crc(const unsigned char *buf, size_t size)
{
  uint32_t read_crc = *((unsigned long *)(buf + size));
  uint32_t comp_crc;

  comp_crc = crc32(buf, size, 0);

  return read_crc == comp_crc;
}

static void print_ping(const struct timeval *delta,
                       unsigned int size,
                       seqno_t seqno,
                       const char *flood_status,
                       char status)
{
  if(flood)
    write(STDOUT_FILENO, flood_status, strlen(flood_status));
  else {
    printf("(%c) %d bytes: ping_req=%d time=%s\n", status, size, seqno,
           tv_to_str(delta));
  }
}

static const char * time_ull_to_str(unsigned long long time)
{
  struct timeval tv;

  tv.tv_sec  = time / 1000000;
  tv.tv_usec = time % 1000000;

  return tv_to_str(&tv);
}

#define tv_to_ull(tv) (tv)->tv_usec + 1000000 * (tv)->tv_sec

static void parse_ping_message(const unsigned char *data, size_t size)
{
  struct timeval t_now, t_sent;
  struct timeval delta;

  seqno_t seqno;
  unsigned long long time;

  gettimeofday(&t_now, NULL);        /* time received */

  if(count != -1)
    count--;

  /* Extract the PING header */
  seqno  = *((seqno_t *)data);
  t_sent = *((struct timeval *)(data + sizeof(seqno_t)));

  timersub(&t_now, &t_sent, &delta); /* rtt */

  /* Check CRC */
  if(!check_crc(data, size - PING_CRC_SIZE)) {
    error++;
    print_ping(&delta, size + 2, seqno, "\bE", 'E');
    return;
  }

  /* register statistics */
  time = tv_to_ull(&delta);
  if(time < min)
    min = time;
  if(time > max)
    max = time;
  sum    += time;
  sq_sum += time*time;

  print_ping(&delta, size + 2, seqno, "\b", '*');
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

  if(count != -1 && !count)
    return false;
  return true;
}

static void cleanup(void)
{
  /* At least we have to close the file descriptor.
     So the firmware could reset the next time we
     open it. */
  if(pid) {
    close(fd);
  }
  /* If we are the child however we will display
     statistics if the flood option isn't set. */
  else if(!flood) {
    unsigned int pct_ok = 0;
    printf("\n--- ping statistics ---\n");

    if(ok > 0)
      pct_ok = (100 * error) / ok;

    printf("%d ok, %d errors, %d%% erroneous\n", ok, error, pct_ok);

    if(ok > 0) {
      unsigned long long avg  = sum / ok;
      unsigned long long mdev = sq_sum / ok - avg * avg;

      /* We don't care about the fractional part, we are already
         computing this with microsecond precision anyway. */
      mdev = sqrt(mdev);

      printf("rtt min/avg/max/mdev =  %s /", time_ull_to_str(min));
      printf(" %s /", time_ull_to_str(avg));
      printf(" %s /", time_ull_to_str(max));
      printf(" %s\n", time_ull_to_str(mdev));
    }
  }
}

static void sig_cleanup(int signum)
{
  exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
  const char *name;
  const char *tty = NULL;
  speed_t speed = B0;
  int interval  = 1000000;
  int size      = 64;

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
    { 'i', "interval", "Wait interval milliseconds between each message" },
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
      interval = atoi(optarg) * 1000;
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
      if(size < 0 || size > MAX_PING_PADDING_SIZE)
        errx(EXIT_FAILURE, "invalid size value");
      break;
    case('f'):
      flood = true;
      interval = 0;
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
  setup_sig(cleanup, sig_cleanup, NULL);

  /* That's where we really start the operations */
  fd = open_uart(tty, speed);

  /* We divide the ping process in two. The parent sends pings and the child
     receives and displays the responses. We proceed that way to ensure that we
     use the same mechanisms (in particular the read buffer) that are used in
     others clients in order to flow the steam in the pipes. */
  pid = fork();
  if(pid < 0)
    err(EXIT_FAILURE, "cannot fork");
  else if(pid) { /* parent */
    seqno_t seqno = 0;
    unsigned char sbuf[128];

    /* We setup the send buffer. Note that we construct the ping message manually.
       We do this to avoid copying the message at each ping we want to send. */
    sbuf[0] = PROT_MTYPE_CONTROL | (size + PING_HDR_SIZE + PING_CRC_SIZE + 1);

    /* Control message type. */
    sbuf[1] = PROT_CTYPE_PING;

    /* We fill the message with random bytes. */
    fill_with_random(sbuf + 2 /* mtype + ctype */ + PING_HDR_SIZE, size);

    while(1) {
      struct timeval tv;

      usleep(interval);

      /* Create the ping template */
      gettimeofday(&tv, NULL);

      /* The ping header and apply the CRC */
      *((seqno_t *)(sbuf + 2)) = seqno;
      *((struct timeval *)(sbuf + 2 + sizeof(seqno_t))) = tv;
      apply_crc(sbuf + 2, size + PING_HDR_SIZE);

      /* Send this message */
      write(fd, sbuf, size + 2 + PING_HDR_SIZE + PING_CRC_SIZE);

      /* Stdout if needed */
      if(flood)
        write_slit(STDOUT_FILENO, ".");

      seqno++;
      count--;

      if(count != -1 && !count)
        break;
    }

    /* Wait for our child. */
    wait(NULL);
  }
  else { /* child */
    input_loop(fd, message_cb, NULL, 0);
  }

  exit_status = EXIT_SUCCESS;

EXIT:
  return exit_status;
}
