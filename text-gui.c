/* File: text-gui.c

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
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <err.h>

#include "dump.h"
#include "mac-display.h"
#include "string-utils.h"
#include "pcap-list.h"

#define MAX_LINE 1024

struct list_foreach_data {
  unsigned int count;
  unsigned int curpos;
};

static void (*_exit_cb)(void);
static void (*_save_cb)(void);
static void (*_save_as_cb)(const char *);
static void (*_open_cb)(const char *);

void warn_text_gui(const char *message)
{
  warnx("%s", message);
}

void init_text_gui(void (*exit_cb)(void),
                   void (*save_cb)(void),
                   void (*save_as_cb)(const char *),
                   void (*open_cb)(const char *))
{
  _exit_cb = exit_cb;
  _save_cb = save_cb;
  _open_cb = open_cb;
  _save_as_cb = save_as_cb;
}

static bool cmd_exit(const char *arg)
{
  _exit_cb();
  return false;
}

static void padding(size_t len, size_t width)
{
  for(; len <= width ; len++)
    fputc(' ', stdout);
}

static bool cmd_help(const char *arg)
{

  int max_cmd = 0;
  int max_arg = 0;

  const struct cmd_help {
    const char *name;
    const char *argument;
    const char *help;
  } helps[] = {
    { "quit", NULL, "Quit the program" },
    { "exit", NULL, "Quit the program" },
    { "help", NULL, "Display this help message" },
    { "new",  "force", "Reset the current PCAP" },
    { "open", "file",  "Open and replace PCAP" },
    { "save", "file",  "Save the current PCAP" },
    { "list", NULL, "List the frames in the PCAP" },
    { "delete", "force", "Delete the frame from the PCAP" },
    { "insert", "file",  "Insert a frame to the PCAP" },
    { "extract", "file", "Extract a frame to the PCAP" },
    { "replace", "file", "Replace a frame in the PCAP" },
    { "next", NULL, "Go one step forward in the PCAP" },
    { "prev", NULL, "Go one step backward in the PCAP" },
    { "goto", "position", "Go at the specifiedposition in the PCAP" },
    { "view", "position", "View the content of the current frame" },
    { NULL, NULL }
  };
  const struct cmd_help *h;

  if(arg)
    warn_text_gui("Unexpected argument for the help command");

  /* maximum name size for padding */
  for(h = helps ; h->name ; h++) {
    size_t cmd_size = strlen(h->name);
    size_t arg_size = h->argument ? strlen(h->argument) : 0;

    if(cmd_size > max_cmd)
      max_cmd = cmd_size;
    if(arg_size > max_arg)
      max_arg = arg_size;
  }

  /* print commands, arguments and help messages */
  for(h = helps ; h->name ; h++) {
    printf("%s", h->name);
    if(h->argument) {
      padding(strlen(h->name), max_cmd);

      printf("[%s]", h->argument);
      padding(strlen(h->argument), max_arg);
    }
    else
      padding(strlen(h->name), max_cmd + max_arg + 2);
    printf("%s\n", h->help);
  }

  return true;
}

/* Return true if the user confirms the command or forced it. */
static bool check_force(const char *arg, const char *message)
{
  if(arg && strcmp(arg, "force"))
    return true;
  else {
    int c;

    printf("%s ? [y/N] ", message);
    c = getc(stdin);

    if(c == 'y')
      return true;
  }

  return false;
}

static bool cmd_new(const char *arg)
{
  if(!check_force(arg, "Are you sure you want to reset the PCAP"))
    goto EXIT;

  pcap_list_flush();
EXIT:
  return true;
}

static bool cmd_open(const char *arg)
{
  if(!arg) {
    warn_text_gui("Expected a file for the open command");
    goto EXIT;
  }

  _open_cb(arg);
EXIT:
  return true;
}

static bool cmd_save(const char *arg)
{
  if(!arg)
    _save_cb();
  else
    _save_as_cb(arg);

  return true;
}

static bool list_foreach(const struct pcap_node *node,
                         void *pdata)
{
  struct list_foreach_data *data = pdata;

  data->count++;

  if(data->count == data->curpos)
    fputc('>', stdout);
  else
    fputc(' ', stdout);

  printf("%d @[%s] : ", data->count, tv_to_str(&node->time));

  if(!node->valid_frame)
    printf("invalid frame");
  else {
    mac_display_type(&node->frame);
    printf(" from ");
    mac_display_saddr(&node->frame);
    printf(" to ");
    mac_display_daddr(&node->frame);
  }

  fputc('\n', stdout);

  return true;
}

static bool cmd_list(const char *arg)
{
  struct list_foreach_data data = { .count = 0,
                                    .curpos = pcap_list_cursor_position() };

  if(arg)
    warn_text_gui("Unexpected argument for the list command");

  pcap_list_for_each(list_foreach, &data);

  return true;
}

static bool cmd_next(const char *arg)
{
  if(arg)
    warn_text_gui("Unexpected argument for the next command");

  pcap_list_cursor_next();

  return true;
}

static bool cmd_prev(const char *arg)
{
  if(arg)
    warn_text_gui("Unexpected argument for the prev command");

  pcap_list_cursor_prev();

  return true;
}

static bool cmd_goto(const char *arg)
{
  unsigned int position;
  if(!arg) {
    warn_text_gui("Expected a position for the goto command");
    goto EXIT;
  }

  position = atoi(arg);
  pcap_list_cursor_seek(position, CURSOR_SEEK_SET);

EXIT:
  return true;
}

static bool cmd_view(const char *arg)
{
  const struct pcap_node *node;

  if(arg)
    cmd_goto(arg);

  node = pcap_list_get_at_cursor();

  if(!node) {
    warn_text_gui("Nothing selected");
    goto EXIT;
  }

  if(!node->valid_frame)
    printf("Invalid frame.\n");
  else {
    mac_display(&node->frame, MI_ALL);

    if(node->frame.payload) {
      printf("Payload:\n");
      hex_dump(node->frame.payload, node->frame.size);
    }
  }

EXIT:
  return true;
}

static bool cmd_delete(const char *arg)
{
  if(!check_force(arg, "Are you sure you want to delete this frame"))
    goto EXIT;

  pcap_list_delete_at_cursor();

EXIT:
  return true;
}

static ssize_t load_file(const char *filename, unsigned char *frame)
{
  ssize_t n;
  int fd = open(filename, O_RDONLY);

  if(fd < 0) {
    warn_text_gui("Cannot open");
    return -1;
  }

  n = read(fd, frame, 128);
  if(n < 0) {
    warn_text_gui("Read error");
    return -1;
  }

  close(fd);

  return n;
}

/* Return true if the user said that the frame contains a final CRC. */
static bool check_crc(void)
{
  int c;

  printf("Does this frame contain a final CRC ? [y/N] ");
  c = getc(stdin);

  if(c == 'y')
    return true;
  return false;
}

static bool cmd_insert(const char *arg)
{
  ssize_t n;
  unsigned char frame[128];

  if(!arg) {
    warn_text_gui("Expected a filename for the insert command");
    goto EXIT;
  }

  n = load_file(arg, frame);
  if(n < 0)
    goto EXIT;

  pcap_list_insert_at_cursor(frame, n, NULL, check_crc());

EXIT:
  return true;
}

static bool cmd_replace(const char *arg)
{
  ssize_t n;
  unsigned char frame[128];

  if(!arg) {
    warn_text_gui("Expected a filename for the replace command");
    goto EXIT;
  }

  n = load_file(arg, frame);
  if(n < 0)
    goto EXIT;

  pcap_list_replace_at_cursor(frame, n, NULL, check_crc());

EXIT:
  return true;
}

static bool cmd_extract(const char *arg)
{
  const struct pcap_node *node;
  ssize_t n;
  int fd;

  if(!arg) {
    warn_text_gui("Expected a filename for the extract command");
    goto EXIT;
  }

  fd = open(arg, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if(fd < 0) {
    warn_text_gui("Cannot open the file");
    goto EXIT;
  }

  node = pcap_list_get_at_cursor();

  if(!node) {
    warn_text_gui("Nothing selected");
    goto EXIT;
  }

  n = write(fd, node->data, node->size);
  if(n != node->size)
    warn_text_gui("Invalid write");

  close(fd);

EXIT:

  return true;
}

static bool check_command(const char *cmd, const char *arg)
{
  const struct command {
    const char *name;
    bool (*func)(const char *arg);
  } cmds[] = {
    { "quit", cmd_exit },
    { "exit", cmd_exit },
    { "help", cmd_help },
    { "new",  cmd_new  },
    { "open", cmd_open },
    { "save", cmd_save },
    { "list", cmd_list },
    { "next", cmd_next },
    { "prev", cmd_prev },
    { "goto", cmd_goto },
    { "view", cmd_view },
    { "delete",  cmd_delete },
    { "insert",  cmd_insert },
    { "extract", cmd_extract },
    { "replace", cmd_replace },
    { NULL, NULL }
  };
  const struct command *c;

  for(c = cmds ; c->name ; c++)
    if(!strcmp(cmd, c->name))
      return c->func(arg);

  warn_text_gui("Unknown command, type 'help' for a list");
  return true;
}

static bool parse_line(char *l)
{
  const char *command;
  const char *argument;

  /* skip leading spaces */
  for(; isspace(*l) ; l++);

  command  = l;
  argument = strtok(l, " ");

  return check_command(command, argument);
}

void main_text_gui(void)
{
  while(1) {
    char line[MAX_LINE];

    fgets(line, MAX_LINE, stdin);

    if(!parse_line(line))
      break;
  }
}

void exit_text_gui(void) {}

