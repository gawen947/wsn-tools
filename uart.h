/* File: uart.h
   Time-stamp: <2013-03-21 13:32:19 gawen>

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

#ifndef _UART_H_
#define _UART_H_

#include <sys/types.h>
#include <termios.h>

/* Open and setup the serial line and return a file descriptor to the serial
   line. The line will be left untouched if the speed is B0. Otherwise it will
   use a default configuration for the line (8N1). The mode argument allows you
   to specify the access mode and must includes one of the following: O_RDONLY,
   O_WRONLY or O_RDWR. */
int open_uart(const char *path, speed_t speed, mode_t mode);

/* Convert a string to a serial speed. */
speed_t baud(const char *arg);

#endif /* _UART_H_ */
