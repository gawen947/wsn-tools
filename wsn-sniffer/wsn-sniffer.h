/* File: wsn-sniffer.h
   Time-stamp: <2013-03-10 23:31:58 gawen>

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

#ifndef _WSN_SNIFFER_H_
#define _WSN_SNIFFER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* This functions sends a MAC frame to the wsn sniffer client. The frame size
   must be less or equal to 127. If it is not the case this function returns -1
   and sends nothing. Otherwise it returns zero. The send argument is the
   function that will be used to send data on the serial line. It must accept
   a varying size of data from 1 up to 127 bytes. */
  int send_frame(const unsigned char *frame, unsigned int size,
                 void (*send)(const unsigned char *, unsigned int));

/* This functions sends an null terminated information string to the wsn sniffer
   client. There isn't any limit in size and this function will split the string
   in chunks of 127 bytes. The send argument is the function that will be used
   to send data on the serial line. It must accept a varying size of data from 1
   up to 127 bytes. */
  void send_info(const char *info,
                 void (*send)(const unsigned char *, unsigned int));

#ifdef __cplusplus
}
#endif

#endif /* _WSN_SNIFFER_H_ */
