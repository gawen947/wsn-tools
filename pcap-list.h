/* File: pcap-list.h

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

#ifndef _PCAP_LIST_H_
#define _PCAP_LIST_H_

#include <stdlib.h>
#include <sys/time.h>
#include <stdbool.h>

#include "mac.h"

struct pcap_node {
  struct mac_frame frame;
  struct timeval time;
  unsigned char *data;
  bool valid_frame;
  size_t size;

  struct pcap_node *prev;
  struct pcap_node *next;
};

enum cursor_whence { CURSOR_SEEK_SET,
                     CURSOR_SEEK_CUR,
                     CURSOR_SEEK_END };

/* Init the list with a function used when a warning
   message has to be issued to the user via the GUI. */
void pcap_list_init(void (*gui_warn)(const char *));

/* Load the list from a PCAP file. If the list was not
   empty then it will be flushed. That is, elements will
   be freed and the list will reset to an empty list. */
void pcap_list_load_from_file(const char *filename);

/* Free the elements and reset to an empty list. */
void pcap_list_flush(void);

/* Get the size of the list. An empty list is of zero size. */
size_t pcap_list_size(void);

/* Get the position of the cursor, starting at one.
   Zero means the position empty in the empty list. */
unsigned int pcap_list_cursor_position(void);

/* Seek into the list. The seek can be relative to the current
   position, from the end or absolute. This function can be used
   to reset the cursor at the beginning or end of the list.
   However you should not use this to go one step forward/backward.
   The two functions pcap_list_cursor_prev/next are better suited
   for that purpose. Note that if it cannot seek further the
function will stop either at the beginning or end of the list. */
void pcap_list_cursor_seek(int offset, enum cursor_whence whence);

/* Seek one step backward. */
void pcap_list_cursor_prev(void);

/* Seek one step forward. */
void pcap_list_cursor_next(void);

/* Get the node at the cursor position. */
const struct pcap_node * pcap_list_get_at_cursor(void);

/* Delete the node at the cursor position. If the list is empty,
   the function does nothing. */
void pcap_list_delete_at_cursor(void);

/* Insert a node at the cursor position. The contains_crc argument
   specifies if the buffer should be decoded as if it contains the
   last two bytes CRC. The timeval argument specifies the time for
   this node in particular. If it is not specified, the time of the
   next or previous frame is used  instead. If the list is empty,
   the time will be set to zero. */
void pcap_list_insert_at_cursor(const unsigned char *frame,
                                size_t size,
                                const struct timeval *tv,
                                bool contains_crc);

/* Replace the node at the cursor position. The old node is deleted
   The contains_crc argument specifies if the buffer should be decoded
   as if it contains the last two bytes CRC. The timeval argument
   specifies the time for this node in particular. If it is not
   specified, the time of the next or previous frame is used instead.
   If the list is empty, the time will be set to zero. */
void pcap_list_replace_at_cursor(const unsigned char *frame,
                                 size_t size,
                                 const struct timeval *tv,
                                 bool contains_crc);

/* Execute an action for each element of the list. The loop will continue
   as long as the action function return true. If it returns false, then
   the loop will stop. */
void pcap_list_for_each(bool (*action)(const struct pcap_node *node,
                                       void *data),
                        void *data);

#endif /* _PCAP_LIST_H_ */
