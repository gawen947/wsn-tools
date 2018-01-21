/* Copyright (c) 2012-2016, David Hauweele <david@hauweele.net>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _IOBUF_H_
#define _IOBUF_H_

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>

#define IOBUF_SIZE 65536
#define GETC_EOF   UCHAR_MAX + 1

/* Remove the single trailing \n
   from a line received from iobuf_gets(). */
#define strip_gets_newline(buf, size) do { \
    if(buf[size-1] == '\n')                \
      buf[size-1] = '\0';                  \
  } while(0)

/* TODO: Define OFF_T_MIN and OFF_T_MAX. */

#define MIN_LSEEK_OFFSET OFF_T_MIN + IOBUF_SIZE
#define MAX_LSEEK_OFFSET OFF_T_MAX - IOBUF_SIZE

#ifndef __FreeBSD__
# define MIN_LSEEK64_OFFSET OFF64_T_MIN + IOBUF_SIZE
# define MAX_LSEEK64_OFFSET OFF64_T_MAX - IOBUF_SIZE
#endif /* __FreeBSD__ */

typedef struct iofile * iofile_t;

/* This creates an opened stream from an already opened file descriptor. */
iofile_t iobuf_dopen(int fd);

/* This opens the file whose name is the string pointed to by pathname
   and associates a stream with it. The arguments flags and mode are
   subject to the same semantic that the ones used in open. */
iofile_t iobuf_open(const char *pathname, int flags, mode_t mode);

/* Write up to count bytes from the buffer pointer buf to the stream
   referred to by file. This is done through an user-space buffer in
   order to avoid useless syscall switch to kernel mode. */
ssize_t iobuf_write(iofile_t file, const void *buf, size_t count);

/* Attemps to read up to count bytes from the stream referred to by
   file. This is done through an user-space buffer in order to avoid
   useless syscall switch to kernel mode. */
ssize_t iobuf_read(iofile_t file, void *buf, size_t count);

/* For output streams, iobuf_flush forces a write of all user-space
   buffered data for the given output. As the standard fflush function
   the kernel buffers are not flushed so you may need to sync manually.
   Unlike the standard fflush function this function does not discards
   the read buffer and only affects the write buffer. */
int iobuf_flush(iofile_t file);

/* Close a stream. This function also take care of flushing the buffers
   when needed. */
int iobuf_close(iofile_t file);

/* Write a single character to the specified file. */
int iobuf_putc(char c, iofile_t file);

/* Read a single character from the specified file.
   Return a negative number in case of error or GETC_EOF
   if the end of file has been reached. */
int iobuf_getc(iofile_t file);

/* Read a single line of maximum count bytes into the buffer.
   This function will never read more than count bytes into
   the target buffer. Negative return values indicate an error.
   A return value of zero indicates than EOF has been reached.
   The buffer must always be at least one byte long because
   it is always marked with the null terminating character.
   The value returned is the length of the string placed in
   the buffer (not including terminal '\0'). */
ssize_t iobuf_gets(iofile_t file, void *buf, size_t count);

/* The iobuf_lseek() function repositions the offset of the open stream
   associated with the file argument to the argument offset according to the
   directive whence. For details see lseek(). There are however two differences
   from the original lseek call. First when using SEEK_CUR directive whence the
   function may return zero instead of the absolute location offset to indicate
   that one lseek syscall has been spared. Second the offset when using the
   SEEK_CUR directive whence must be comprised between two constants
   MIN_LSEEK_OFFSET and MAX_LSEEK_OFFSET. These values are large enough so the
   user may ensure that every seek will be comprised in this interval. */
off_t iobuf_lseek(iofile_t file, off_t offset, int whence);

#if !defined(__FreeBSD__) && defined(_LARGEFILE64_SOURCE)
/* The iobuf_lseek64() function repositions the offset of the open stream
   associated with the file argument to the argument offset according to the
   directive whence. For details see lseek64(). There are however two
   differences from the original lseek call. First when using SEEK_CUR directive
   whence the function may return zero instead of the absolute location offset
   to indicate that one lseek syscall has been spared. Second the offset when
   using the SEEK_CUR directive whence must be comprised between two constants
   MIN_LSEEK64_OFFSET and MAX_LSEEK64_OFFSET. These values are large enough so
   the user may ensure that every seek will be comprised in this interval. */
off64_t iobuf_lseek64(iofile_t file, off64_t offset, int whence);
#endif

#endif /* _IOBUF_H_ */
