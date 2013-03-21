/* File: atoi-gen.h
   Time-stamp: <2013-03-21 15:11:17 gawen>

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

#ifndef _ATOI_GEN_H_
#define _ATOI_GEN_H_

#include <stdbool.h>

/* This generic atoi function understand number written in different bases. By
   default it will assume a decimal base. If the string is prefixed with 0x or
   0X it will assume an hexadecimal base. If the string is prefixed with 0b or
   0B it will assume a binary base. If the string is prefixed with a 0 it will
   assume an octal base. Leading and trailing spaces are ignored. There isn't
   any error checking and the result may be incorrect in case of overflow or
   incorrect symbols for the choosen base. For an error aware generic converter
   you may use strtol_gen instead (not implemented yet). */
int atoi_gen(const char *s);

/* This function will parse an hexadecimal string stopping at the first
   occurence of a character in the delim string. It will then the value parsed
   until this delimiter in the v argument and return a pointer to the delimiter
   in the string. If the accept_zero flag is specified, the null terminator will
   be accepted as a valid delimiter. Otherwise an error will be reported. This
   function will also check for valid hexadecimal characters and will report an
   error if the character is invalid. */
const char * parse_hex_until(const char *s, const char *delim,
                             unsigned int *v, const char *error_message,
                             bool accept_zero);

#endif /* _ATOI_GEN_H_ */
