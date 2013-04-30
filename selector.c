/* File: selector.c

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
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>

#include "version.h"
#include "pcap-list.h"
#include "pcap-write.h"
#include "text-gui.h"
#include "help.h"

#define TARGET "Frame-Selector"

static void (*warn_gui)(const char *fmt) = warn_text_gui;
static void (*init_gui)(void (*exit_cb)(void),
                        void (*save_cb)(void),
                        void (*save_as_cb)(const char *),
                        void (*open_cb)(const char *)) = init_text_gui;
static void (*main_gui)(void) = main_text_gui;
static void (*exit_gui)(void) = exit_text_gui;

/* The filename of the current list. This variable can be NULL which
   means that no file has been opened. */
static const char *filename;

static bool pcap_write_action(const struct pcap_node *node, void *data)
{
  pcap_append_frame(node->data, node->size);
  return true;
}

static void save_as_cb(const char *path)
{
  open_writing_pcap(path);
  pcap_list_for_each(pcap_write_action, NULL);
  close_writing_pcap();

  filename = path;
}

static void save_cb(void)
{
  save_as_cb(filename);
}

static void exit_cb(void)
{
  save_cb();
}

static void open_cb(const char *path)
{
  pcap_list_load_from_file(path);
  filename = path;
}

int main(int argc, char *argv[])
{
  const char *name;

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
    { 0, NULL, NULL }
  };

  struct option opts[] = {
    { "help", no_argument, NULL, 'h' },
    { "version", no_argument, NULL, 'V' },
#ifdef COMMIT
    { "commit", no_argument, NULL, OPT_COMMIT },
#endif /* COMMIT */
    { NULL, 0, NULL, 0 }
  };

  while(1) {
    int c = getopt_long(argc, argv, "hV", opts, NULL);

    if(c == -1)
      break;

    switch(c) {
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
      help(name, "[OPTIONS] ... [PCAP FILE]", helps);
      goto EXIT;
    }
  }

  /* Initialise the GUI. */
  init_gui(exit_cb,
           save_cb,
           save_as_cb,
           open_cb);

  pcap_list_init(warn_gui);

  if((argc - optind) == 1) {
    int ret;

    filename = argv[optind];

    ret = access(filename, R_OK);

    switch(ret) {
    case 0:
      pcap_list_load_from_file(filename);
      break;
    case ENOENT:
      break;
    default:
      err(EXIT_FAILURE, "cannot access %s", filename);
    }
  }

  /* Start the GUI. */
  main_gui();

EXIT:
  return exit_status;
}

