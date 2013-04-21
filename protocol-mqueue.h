/* File: protocol-mqueue.h

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

#ifndef _PROTOCOL_MQUEUE_H_
#define _PROTOCOL_MQUEUE_H_

#include "protocol.h"

typedef struct prot_mqueue * prot_mqueue_t;

/* This create a protocol message queue. */
prot_mqueue_t prot_mqueue_creat(void);

/* Adds a message to the message queue. */
void prot_mqueue_add_control(prot_mqueue_t mq,
                             enum prot_ctype type,
                             const void *value,
                             size_t size);

/* This will send all messages from the queue to the specified
   file descriptor. */
void prot_mqueue_sendall(prot_mqueue_t mq, int fd);

/* This destroy a protocol message queue and all messages in it. */
void prot_mqueue_destroy(prot_mqueue_t mq);

#endif /* _PROTOCOL_MQUEUE_H_ */
