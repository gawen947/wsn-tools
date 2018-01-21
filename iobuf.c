/* Copyright (c) 2012-2018, David Hauweele <david@hauweele.net>
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

#ifndef _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 200112L
#endif /* _POSIX_C_SOURCE */

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

  unsigned int write_size;
  unsigned int read_size;
  char *write_buf;
  char *read_buf;
  char buf[IOBUF_SIZE * 2];
};

static ssize_t fill_buffer(iofile_t file)
{
  ssize_t partial_read = IOBUF_SIZE * 2; /* no refill */

  if(file->read_size == 0) {
    partial_read = read(file->fd, file->buf + IOBUF_SIZE, IOBUF_SIZE);
    if(partial_read < 0) /* read error */
      return partial_read;

    file->read_size = partial_read;
    file->read_buf  = file->buf + IOBUF_SIZE;
  }

  return partial_read;
}

int iobuf_flush(iofile_t file)
{
  int write_size  = file->write_size;

  while(write_size) {
    ssize_t partial_write = write(file->fd, file->buf, write_size);
    if(partial_write < 0)
      return partial_write;

    write_size -= partial_write;
  }

  file->write_size = 0;
  file->write_buf  = file->buf;

  return 0;
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

/* We only declare the access pattern on architectures
   that are known to support posix_fadvise. */
#if defined(__linux__) || defined(__FreeBSD__)
  posix_fadvise(file->fd, 0, 0, POSIX_FADV_SEQUENTIAL);
#endif

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
  if(count > (IOBUF_SIZE - file->write_size)) {
    ssize_t partial_write;

    partial_write = iobuf_flush(file);
    if(partial_write < 0)
      return partial_write;

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
  char *cbuf = buf;

  do {
    ssize_t partial_read = fill_buffer(file);
    if(partial_read == 0)
      goto EXIT;
    else if(partial_read < 0)
      return partial_read;

    partial_read = MIN(count, file->read_size);
    memcpy(cbuf, file->read_buf, partial_read);
    file->read_buf  += partial_read;
    file->read_size -= partial_read;
    count           -= partial_read;
    cbuf            += partial_read;
  } while(count);

EXIT:
  return cbuf - (char *)buf;
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

int iobuf_getc(iofile_t file)
{
  ssize_t partial_read = fill_buffer(file);
  if(partial_read < 0)
    return partial_read;
  else if(!partial_read)
    return GETC_EOF;

  file->read_buf++;
  file->read_size--;
  return *file->read_buf;
}

ssize_t iobuf_gets(iofile_t file, void *buf, size_t count)
{
  char *cbuf = buf;

  count--;

  do {
    char *eol;
    ssize_t partial_read = fill_buffer(file);
    if(partial_read == 0)
      goto EXIT;
    else if(partial_read < 0)
      return partial_read;

    partial_read = MIN(count, file->read_size);

    eol = memchr(file->read_buf, '\n', partial_read);
    if(eol) {
      partial_read  = eol - file->read_buf + 1; /* keep newline */
      count         = partial_read; /* will be zero and break */
    }

    memcpy(cbuf, file->read_buf, partial_read);

    file->read_buf  += partial_read;
    file->read_size -= partial_read;
    count           -= partial_read;
    cbuf            += partial_read;
  } while(count);

EXIT:
  /* mark EOL */
  *cbuf = '\0';

  return cbuf - (char *)buf;
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
