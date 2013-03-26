/* File: input.h

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

#ifndef _INPUT_H_
#define _INPUT_H_

#ifdef __cplusplus
extern "C" {
#endif

  /* This function receive a character from the wsn injector client. It will
     will store this character in a buffer and send the frame with the tx_frame
     function when it is complete. */
  void loop_step(char c, void (*tx_frame)(const char *, unsigned int));

#ifdef __cplusplus
}
#endif

#endif /* _INPUT_H_ */
