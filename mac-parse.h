/* File: mac-parse.h

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

#ifndef _MAC_PARSE_H_
#define _MAC_PARSE_H_

#include "mac.h"

/* Parse the MAC frame type. */
void parse_type(struct mac_frame *frame, const char *type_arg);

/* Parse the reserved field. */
void parse_reserved(struct mac_frame *frame, const char *reserved_arg);

/* Parse the Source Address Mode. */
void parse_sam(struct mac_frame *frame, const char *sam_arg);

/* Parse the Destination Address Mode. */
void parse_dam(struct mac_frame *frame, const char *dam_arg);

/* Parse the version. */
void parse_version(struct mac_frame *frame, const char *version_arg);

/* Parse the source address. */
void parse_saddr(struct mac_frame *frame, const char *addr_arg);

/* Parse the destination address. */
void parse_daddr(struct mac_frame *frame, const char *addr_arg);

/* Parse the sequence number. */
void parse_seqno(struct mac_frame *frame, const char *seqno_arg);

/* Parse the MAC control flags. */
void parse_flags(struct mac_frame *frame, const char *flags);

/* Parse a flag to enable. */
void parse_flag_enable(struct mac_frame *frame, unsigned int flag);

/* Parse a flag to disable. */
void parse_flag_disable(struct mac_frame *frame, unsigned int flag);

/* Decode a frame from a file. */
void parse_frame_from_file(struct mac_frame *frame, const char *filename);

/* Setup the frame's payload from a file. */
void setup_payload_from_file(struct mac_frame *frame, const char *filename);

/* Setup a frame with the default values. */
void setup_default_frame(struct mac_frame *frame);

#endif /* _MAC_PARSE_H_ */
