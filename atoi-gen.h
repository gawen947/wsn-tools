/* File: atoi-gen.h
   Time-stamp: <2013-03-21 00:04:56 gawen>

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
   occurence of a character in the delim string. It will then store the
   delimiter character found in the delim_found argument. If the zero_message is
   not specified, the null terminator will be accepted as a valid
   delimiter. Otherwise the zero_message will be used to quit the program with
   an error when the null terminator is encountered. This function will also
   check for valid hexadecimal characters and will report an error if the
   character is invalid. */
unsigned int parse_hex_until(const char *s, const char *delim,
                             char *delim_found, const char *zero_message);

#endif /* _ATOI_GEN_H_ */
