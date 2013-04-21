/* File: signal.c

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

#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>

#include "signal-utils.h"

/* Setup the signal function 'sig' for 'nsig' signals which are specified
   as 'nsig' optional arguments with integer type. */
static void setup_sigaction(void (*sig)(int), int nsig, ...)
{
  struct sigaction act = { .sa_handler = sig };
  va_list ap;

  if(!sig)
    return;

  sigfillset(&act.sa_mask);

  va_start(ap, nsig);
  while(nsig--)
    sigaction(va_arg(ap, int), &act, NULL);
  va_end(ap);
}

void setup_sig(void (*cleanup)(void),
               void (*sigterm)(int),
               void (*sigusr1)(int))
{
  setup_sigaction(sigterm, 2, SIGTERM, SIGINT);
  setup_sigaction(sigusr1, 1, SIGUSR1);

  atexit(cleanup);
}
