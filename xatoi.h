/* Copyright (c) 2016, David Hauweele <david@hauweele.net>
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

#ifndef _XATOI_H_

#include <stdint.h>
#include <limits.h>

enum XATOI_ERR {
  XATOI_SUCCESS = 0, /* number parsed succesfully */
  XATOI_UNDERFLOW,   /* underflow while parsing the number */
  XATOI_OVERFLOW,    /* overflow while parsing the number */
  XATOI_INVALID      /* invalid character found while parsing */
};

/* xatou() */
#if UINT_MAX == UINT32_MAX
# define xatou(s, err) xatou32(s, err)
#elif UINT_MAX == UINT64_MAX
# define xatou(s, err) xatou64(s, err)
#else
# error "Unsupported 'unsigned int' type width."
#endif /* xatou() */

/* xatoul() */
#if UINT_MAX == UINT32_MAX
# define xatoul(s, err) xatou32(s, err)
#elif UINT_MAX == UINT64_MAX
# define xatoul(s, err) xatou64(s, err)
#else
# error "Unsupported 'unsigned long' type width."
#endif /* xatoul() */

/* xatoull() */
#if UINT_MAX == UINT32_MAX
# define xatoull(s, err) xatou32(s, err)
#elif UINT_MAX == UINT64_MAX
# define xatoull(s, err) xatou64(s, err)
#else
# error "Unsupported 'unsigned long long' type width."
#endif /* xatoull() */

/* Convert an ASCII string to unsigned int (32-bits wide).

   Contrary to the atoi() function, this function only
   accepts digit numbers. Error conditions such as an
   invalid number of underflow/overflow are indicated
   via the second pointer argument.

   It is also faster than its libc counterpart.
*/
uint32_t xatou32(const char *s, int *err);

/* Convert an ASCII string to unsigned long (64-bits wide). */
uint64_t xatou64(const char *s, int *err);

#endif /* _XATOI_H_ */
