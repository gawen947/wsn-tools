 /* File: procotol-mqueue.c

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

#include <string.h>
#include <assert.h>

#include "protocol.h"
#include "protocol-mqueue.h"

struct message {
  enum prot_mtype type;
  unsigned char *message;
  size_t size;

  struct message *next;
};

struct prot_mqueue {
  struct message *head;
  struct message *tail;
};

prot_mqueue_t prot_mqueue_creat(void)
{
  struct prot_mqueue *mq = malloc(sizeof(struct prot_mqueue));
  memset(mq, 0, sizeof(struct prot_mqueue));

  return mq;
}

void prot_mqueue_add_control(prot_mqueue_t mq,
                             enum prot_ctype type,
                             const void *message,
                             size_t size)
{
  assert(size < MAX_MESSAGE_SIZE);

  struct message *m = malloc(sizeof(struct message));

  /* Control messages are composed as this:
     [message-info-byte][control-type-byte]<message> */
  m->message    = malloc(size + 1);
  m->message[0] = type;
  memcpy(m->message + 1, message, size);

  m->type    = PROT_MTYPE_CONTROL;
  m->size    = size + 1;

  m->next    = NULL;

  /* Now we attach the node to the list */
  mq->tail->next = m;
  mq->tail       = m;

  if(!mq->head)
    mq->head = m;
}

void prot_mqueue_sendall(prot_mqueue_t mq, int fd)
{
  struct message *m;

  for(m = mq->head ; m ; m = m->next)
    prot_write(fd, m->type, m->message, m->size);
}

void prot_mqueue_destroy(prot_mqueue_t mq)
{
  struct message *m = mq->head;
  struct message *o;

  while(m) {
    o = m;
    m = m->next;

    free(o->message);
    free(o);
  }

  free(mq);
}
