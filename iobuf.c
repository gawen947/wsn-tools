/* File: iobuf.c
   Time-stamp: <2013-03-22 23:00:30 gawen>

   Copyright (C) 2012-2013 David Hauweele <david@hauweele.net>

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

#define _POSIX_C_SOURCE 200112L

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include "iobuf.h"

#ifndef MIN
# define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif /* MIN */

struct iofile {
  int fd;

  int write_size;
  int read_size;
  char *write_buf;
  char *read_buf;
  char buf[IOBUF_SIZE * 2];
};

ssize_t iobuf_flush(iofile_t file)
{
  ssize_t partial_write;

  partial_write = write(file->fd, file->buf, file->write_size);
  if(partial_write >= 0) {
    file->write_size -= partial_write;
    file->write_buf  -= partial_write;
  }

  return partial_write;
}

iofile_t iobuf_dopen(int fd)
{
  struct iofile *file = malloc(sizeof(struct iofile));
  if(!file)
    return NULL;

  file->fd         = fd;
  file->write_buf  = file->buf;
  file->read_buf   = file->buf + IOBUF_SIZE;
  file->write_size = file->read_size = 0;

  posix_fadvise(file->fd, 0, 0, POSIX_FADV_SEQUENTIAL);

  return file;
}

iofile_t iobuf_open(const char *pathname, int flags, mode_t mode)
{
  int fd = open(pathname, flags, mode);

  if(fd < 0)
    return NULL;

  return iobuf_dopen(fd);
}

ssize_t iobuf_write(iofile_t file, const void *buf, size_t count)
{
  if(count > IOBUF_SIZE - file->write_size) {
    ssize_t partial_write;

    partial_write = iobuf_flush(file);

    if(count > IOBUF_SIZE) {
      ssize_t full_write;
      full_write = write(file->fd, buf, count);
      if(full_write < 0)
        return full_write;
      return partial_write + full_write;
    }
  }

  memcpy(file->write_buf, buf, count);
  file->write_size += count;
  file->write_buf  += count;

  return count;
}

ssize_t iobuf_read(iofile_t file, void *buf, size_t count)
{
  ssize_t ret = count;

  do {
    ssize_t partial_read;

    if(file->read_size == 0) {
      partial_read = read(file->fd, file->buf + IOBUF_SIZE, IOBUF_SIZE);
      if(partial_read <= 0)
        return partial_read;

      file->read_size = partial_read;
      file->read_buf  = file->buf + IOBUF_SIZE;
    }

    partial_read = MIN(count, file->read_size);
    memcpy(buf, file->read_buf, partial_read);
    file->read_buf  += partial_read;
    file->read_size -= partial_read;
    count           -= partial_read;
    buf             += partial_read;
  } while(count);

  return ret;
}

int iobuf_close(iofile_t file)
{
  int ret;

  if(file->write_size) {
    ret = iobuf_flush(file);

    if(ret < 0)
      return ret;
  }

  ret = close(file->fd);
  if(ret < 0)
    return ret;

  free(file);

  return ret;
}

int iobuf_putc(char c, iofile_t file)
{
  if(file->write_size == IOBUF_SIZE) {
    ssize_t partial_write;
    partial_write = iobuf_flush(file);
    if(partial_write < 0)
      return partial_write;
  }

  *file->write_buf = c;
  file->write_size++;
  file->write_buf++;

  return c;
}

off_t iobuf_lseek(iofile_t file, off_t offset, int whence)
{
  if(whence == SEEK_CUR) {
    /* There may be an overflow here. We merely assume that the user won't use
       an offset large enough to overflow. */
    off_t partial = offset - file->read_size;

     /* If we seek inside the buffer (short relative seek) then we may avoid
        having to drain the buffers. This may improve the performances as most
        relative seeks are very short and absolute seeks are generally
        unrecoverable. */
    if(partial > 0)
      offset = partial;
    else if(partial + IOBUF_SIZE < 0)
      offset = - (offset + file->read_size);
    else {
      file->read_buf  += offset;
      file->read_size -= offset;

      /* We do not know about the absolute offset location in the file. As it
         would inflict a slight overhead to other funtions just for the benefit
         of returning the absolute location that most peoples will ignore in
         this specific case, we just ignore and return zero to let the user know
         that one syscall has been spared. */
      return 0;
    }
  }

  off_t res = lseek(file->fd, offset, whence);
  if(res < 0)
    return res;

  file->read_size = 0;
  file->read_buf  = file->buf + 2 * IOBUF_SIZE;

  if(file->write_size)
    iobuf_flush(file);

  return res;
}

#if !defined(__FreeBSD__) && defined(_LARGEFILE64_SOURCE)
off64_t iobuf_lseek64(iofile_t file, off64_t offset, int whence)
{
  if(whence == SEEK_CUR) {
    /* There may be an overflow here. We merely assume that the user won't use
       an offset large enough to overflow. */
    off64_t partial = offset - file->read_size;

     /* If we seek inside the buffer (short relative seek) then we may avoid
        having to drain the buffers. This may improve the performances as most
        relative seeks are very short and absolute seeks are generally
        unrecoverable. */
    if(partial > 0)
      offset = partial;
    else if(partial + IOBUF_SIZE < 0)
      offset = - (offset + file->read_size);
    else {
      file->read_buf  += offset;
      file->read_size -= offset;

      /* We do not know about the absolute offset location in the file. As it
         would inflict a slight overhead to other funtions just for the benefit
         of returning the absolute location that most peoples will ignore in
         this specific case, we just ignore and return zero to let the user know
         that one syscall has been spared. */
      return 0;
    }
  }

  off64_t res = lseek64(file->fd, offset, whence);
  if(res < 0)
    return res;

  file->read_size = 0;
  file->read_buf  = file->buf + 2 * IOBUF_SIZE;

  if(file->write_size)
    iobuf_flush(file);

  return res;
}
#endif
