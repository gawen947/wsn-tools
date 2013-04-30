/* File: pcap-list.c

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
#include <err.h>
#include <assert.h>

#include "string-utils.h"
#include "mac-decode.h"
#include "pcap-read.h"
#include "pcap-list.h"

static struct pcap_node *head;
static struct pcap_node *tail;
static struct pcap_node *cursor;
static size_t size;
static unsigned int index;

static void (*_gui_warn)(const char *message);

static struct pcap_node * load_frame(void)
{
  struct pcap_node *node;
  struct timeval tv;
  size_t size;
  int res;

  unsigned char *frame = pcap_read_frame(&size, &tv);

  if(!frame)
    return NULL;

  node = malloc(sizeof(struct pcap_node));
  node->prev = tail;
  node->next = NULL;
  node->size = size;
  node->time = tv;
  node->data = frame;
  node->valid_frame = true;

  tail->next = node;
  tail = node;

  if(!head) {
    head   = node;
    cursor = node;
  }

  res = mac_decode(&node->frame, frame, true, size);
  if(res < 0) {
    node->valid_frame = false;
    _gui_warn("cannot decode a frame");
  }

  return node;
}

void pcap_list_init(void (*gui_warn)(const char *))
{
  _gui_warn = gui_warn;
}

void pcap_list_load_from_file(const char *filename)
{
  size_t count = 0;

  /* Flush the list if needed. */
  if(head)
    pcap_list_flush();

  open_reading_pcap(filename);

  if(!load_frame())
    goto EOF;
  count++;

EOF:
  size = count;

  close_reading_pcap();
}

/* Free a pcap node's internal structure. */
static void free_pcap_node(struct pcap_node *node)
{
  if(node->valid_frame)
    free_mac_frame(&node->frame);
  free(node->data);
}

void pcap_list_flush(void)
{
  struct pcap_node *o = head;

  while(o) {
    struct pcap_node *t = o;
    o = o->next;

    free_pcap_node(t);
    free(t);
  }

  head   = NULL;
  tail   = NULL;
  cursor = NULL;
  size   = 0;
  index  = 0;
}

size_t pcap_list_size(void)
{
  return size;
}

unsigned int pcap_list_cursor_position(void)
{
  return index;
}

static void seek_forward(int offset)
{
  while(offset-- && cursor->next) {
    cursor = cursor->next;
    index++;
  }
}

static void seek_backward(int offset)
{
  while(offset-- && cursor->prev) {
    cursor = cursor->prev;
    index--;
  }
}

void pcap_list_cursor_seek(int offset, enum cursor_whence whence)
{
  if(!cursor)
    return;

  switch(whence) {
  case(CURSOR_SEEK_SET):
    cursor = head;
    index  = 0;
    break;
  case(CURSOR_SEEK_END):
    cursor = tail;
    index  = size;
    offset = -offset;
  case(CURSOR_SEEK_CUR):
    break;
  default:
    assert(0);
  }

  if(offset == 0)
    return;
  else if(offset > 0)
    seek_forward(offset);
  else /* offset < 0 */
    seek_backward(-offset);
}

void pcap_list_cursor_prev(void)
{
  if(!cursor)
    return;

  if(cursor->prev) {
    cursor = cursor->prev;
    index--;
  }
}

void pcap_list_cursor_next(void)
{
  if(!cursor)
    return;

  if(cursor->next) {
    cursor = cursor->next;
    index++;
  }
}

const struct pcap_node * pcap_list_get_at_cursor(void)
{
  return cursor;
}

void pcap_list_delete_at_cursor(void)
{
  struct pcap_node *new_cursor;

  if(!cursor)
    return;

  size--;

  if(cursor->next) {
    cursor->next->prev = cursor->prev;
    new_cursor = cursor->next;
  }
  else {
    index--;
    new_cursor = cursor->prev;
  }

  if(cursor->prev)
    cursor->prev->next = cursor->next;

  free_pcap_node(cursor);
  free(cursor);

  cursor = new_cursor;

  /* set head/tail accordingly */
  if(!cursor->next)
    tail = cursor;
  if(!cursor->prev)
    head = cursor;
}

static void setup_time(struct pcap_node *node)
{
  if(node->next)
    node->time = node->next->time;
  else if(node->prev)
    node->time = node->prev->time;
  else {
    node->time.tv_sec  = 0;
    node->time.tv_usec = 0;
  }
}

void pcap_list_insert_at_cursor(const unsigned char *frame,
                                size_t size,
                                const struct timeval *tv,
                                bool contains_crc)
{
  int res;
  struct pcap_node *node = malloc(sizeof(struct pcap_node));

  size++;

  node->data = memdup(frame, size);
  node->prev = cursor;
  if(cursor)
    node->next = cursor->next;

  if(node->next)
    node->next->prev = node;
  else
    tail = node;
  if(node->prev)
    node->prev->next = node;
  else
    head = node;

  cursor = node;
  index++;

  /* create the frame */
  res = mac_decode(&node->frame, frame, contains_crc, size);
  if(res < 0)
    node->valid_frame = false;

  /* setup the time */
  if(!tv)
    setup_time(node);
  else
    node->time = *tv;
}

void pcap_list_replace_at_cursor(const unsigned char *frame,
                                 size_t size,
                                 const struct timeval *tv,
                                 bool contains_crc)
{
  int res;

  /* we don't want to replace an element in an empty list */
  assert(cursor);

  free_pcap_node(cursor);

  cursor->data = memdup(frame, size);
  cursor->size = size;

  res = mac_decode(&cursor->frame, frame, contains_crc, size);
  if(res < 0) {
    cursor->valid_frame = false;
    _gui_warn("Cannot decode the frame");
  }

  /* setup the time */
  if(!tv)
    setup_time(cursor);
  else
    cursor->time = *tv;
}

void pcap_list_for_each(bool (*action)(const struct pcap_node *node,
                                       void *data),
                        void *data)
{
  struct pcap_node *o;
  for(o = head ; o ; o = o->next)
    if(!action(o, data))
      return;
}
