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

#include <stdint.h>
#include <ctype.h>

#include "xatoi.h"

uint32_t xatou32(const char *s, int *err)
{
  register unsigned long r = 0;

  *err = XATOI_SUCCESS;

  for(; *s ; s++) {
    if(!isnumber(*s)) {
      *err = XATOI_INVALID;
      goto EXIT;
    }

    /* optimizations to handle the overflow */
#if defined(__i386__)
    __asm__(" mov $0xa, %%eax;"
            " mull %[r];"
            " jc 10f;"
            " movzbl (%[s]), %%ecx;"
            " sub    $0x30,  %%ecx;"
            " add    %%ecx,  %%eax;"
            " mov    %%eax,  %[r];"
            " jc 10f;"
            " jmp 20f;"
            "10:"
            " movl %[XATOI_OVERFLOW], (%[err]);"
            "20:"
            : [r] "=r" (r), [err] "=r" (err)
            : "[r]" (r), [s] "r" (s), [XATOI_OVERFLOW] "i" (XATOI_OVERFLOW)
            : "cc", "memory", "edx" /* mull -> edx:eax */, "eax", "ecx");
#elif defined(__x86_64__)
    __asm__(" movzbq (%[s]), %%rax;"
            " lea    -0x30(%%rax, %[r], 8), %%rax;"
            " lea        0(%%rax, %[r], 2), %%rax;"
            " mov    %%rax, %[r];"
            " shr    $0x20, %%rax;"
            " je 10f;"
            " movl %[XATOI_OVERFLOW], (%[err]);"
            "10:"
            : [r] "=r" (r), [err] "=r" (err)
            : "[r]" (r), [s] "r" (s), [XATOI_OVERFLOW] "i" (XATOI_OVERFLOW)
            : "cc", "memory", "rax");
#else
    uint32_t rr;

    rr = r * 10;
    if(r != 0 && rr / r != 10)
      *err = XATOI_OVERFLOW;
    r = rr;

    rr = r + *s - '0';
    if(rr < r)
      *err = XATOI_OVERFLOW;
    r = rr;
#endif
  }

EXIT:
  return r;
}

uint64_t xatou64(const char *s, int *err)
{
  register unsigned long long r = 0;

  *err = XATOI_SUCCESS;

  for(; *s ; s++) {
    if(!isnumber(*s)) {
      *err = XATOI_INVALID;
      goto EXIT;
    }

#if defined(__x86_64__)
    /* We cannot rely on a larger type
       now so there is no magik, we just
       have to rely on the carry. */
    __asm__(" mov $0xa, %%rax;"
            " mulq %[r];"
            " jc 10f;"
            " movzbq (%[s]), %%rcx;"
            " sub    $0x30,  %%rcx;"
            " add    %%rcx,  %%rax;"
            " mov    %%rax,  %[r];"
            " jc 10f;"
            " jmp 20f;"
            "10:"
            " movl %[XATOI_OVERFLOW], (%[err]);"
            "20:"
            : [r] "=r" (r), [err] "=r" (err)
            : "[r]" (r), [s] "r" (s), [XATOI_OVERFLOW] "i" (XATOI_OVERFLOW)
            : "cc", "memory", "rdx" /* mulq -> rdx:rax */, "rax", "rcx");
#else
    uint64_t rr;

    rr = r * 10;
    if(r != 0 && rr / r != 10)
      *err = XATOI_OVERFLOW;
    r = rr;

    rr = r + *s - '0';
    if(rr < r)
      *err = XATOI_OVERFLOW;
    r = rr;
#endif
  }

EXIT:
  return r;
}
