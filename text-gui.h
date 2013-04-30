/* File: text-gui.h

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

#ifndef _TEXT_GUI_H_
#define _TEXT_GUI_H_

void warn_text_gui(const char *message);
void init_text_gui(void (*exit_cb)(void),
                             void (*save_cb)(void),
                             void (*save_as_cb)(const char *),
                             void (*open_cb)(const char *));
void main_text_gui(void);
void exit_text_gui(void);

#endif /* _TEXT_GUI_H_ */
