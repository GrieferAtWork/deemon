/* Copyright (c) 2018 Griefer@Work                                            *
 *                                                                            *
 * This software is provided 'as-is', without any express or implied          *
 * warranty. In no event will the authors be held liable for any damages      *
 * arising from the use of this software.                                     *
 *                                                                            *
 * Permission is granted to anyone to use this software for any purpose,      *
 * including commercial applications, and to alter it and redistribute it     *
 * freely, subject to the following restrictions:                             *
 *                                                                            *
 * 1. The origin of this software must not be misrepresented; you must not    *
 *    claim that you wrote the original software. If you use this software    *
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_STRING_FUNCTIONS_H
#define GUARD_DEEMON_OBJECTS_UNICODE_STRING_FUNCTIONS_H 1
#define _KOS_SOURCE 1 /* memchrb/w/l, memrchrb/w/l, etc... */
#define _GNU_SOURCE 1 /* memrchr */

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/util/string.h>
#include <hybrid/minmax.h>

#include <stddef.h>
#include <string.h>
#ifndef CONFIG_NO_CTYPE
#include <ctype.h>
#endif

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"

#ifndef SSIZE_MAX
#include <hybrid/limitcore.h>
#define SSIZE_MAX  __SSIZE_MAX__
#endif

DECL_BEGIN

#define DeeUni_IsSign(x) ((x)=='+' || (x)=='-')
#define ASCII_SPACE    32 /* ' ' */
#define ASCII_ZERO     48 /* '0' */
#define ASCII_CR       13 /* '\r' */
#define ASCII_LF       10 /* '\n' */
#define ASCII_TAB       9 /* '\t' */

#define UNICODE_SPACE  32 /* ' ' */
#define UNICODE_ZERO   48 /* '0' */
#define UNICODE_CR     13 /* '\r' */
#define UNICODE_LF     10 /* '\n' */

#ifndef __USE_GNU
#define memrchr  dee_memrchr
LOCAL void *dee_memrchr(void const *__restrict p, int c, size_t n) {
 uint8_t *iter = (uint8_t *)p+n;
 while (iter != (uint8_t *)p) {
  if (*--iter == c) return iter;
 }
 return NULL;
}
#define memmem  dee_memmem
LOCAL void *dee_memmem(void const *__restrict haystack, size_t haystack_length,
                       void const *__restrict needle, size_t needle_length) {
 uint8_t *candidate; uint8_t marker;
 if unlikely(!needle_length || needle_length > haystack_length)
    return NULL;
 haystack_length -= needle_length-1,marker = *(uint8_t *)needle;
 while ((candidate = (uint8_t *)memchr(haystack,marker,haystack_length)) != NULL) {
  if (memcmp(candidate,needle,needle_length) == 0)
      return (void *)candidate;
  ++candidate;
  haystack_length = ((uint8_t *)haystack + haystack_length) - candidate;
  haystack        = (void const *)candidate;
 }
 return NULL;
}
#endif /* !__USE_GNU */

#ifndef __USE_KOS
#define memcpyb(dst,src,n)             memcpy(dst,src,n)
#if defined(_MSC_VER) && (defined(__i386__) || defined(__x86_64__))
#ifdef __x86_64__
extern void __movsw(unsigned short *, unsigned short const *, unsigned __int64);
extern void __movsd(unsigned long *, unsigned long const *, unsigned __int64);
extern void __stosw(unsigned short *, unsigned short, unsigned __int64);
extern void __stosd(unsigned long *, unsigned long, unsigned __int64);
#else
extern void __movsw(unsigned short *, unsigned short const *, unsigned long);
extern void __movsd(unsigned long *, unsigned long const *, unsigned long);
extern void __stosw(unsigned short *, unsigned short, unsigned long);
extern void __stosd(unsigned long *, unsigned long, unsigned long);
#endif
#pragma intrinsic(__movsw)
#pragma intrinsic(__movsd)
#pragma intrinsic(__stosw)
#pragma intrinsic(__stosd)
#define memcpyw(dst,src,n) __movsw((unsigned short *)(dst),(unsigned short const *)(src),(size_t)(n))
#define memcpyl(dst,src,n) __movsd((unsigned long *)(dst),(unsigned long const *)(src),(size_t)(n))
#define memsetw(dst,c,n)   __stosw((unsigned short *)(dst),(unsigned short)(c),(size_t)(n))
#define memsetl(dst,c,n)   __stosd((unsigned long *)(dst),(unsigned long)(c),(size_t)(n))
#else
#define memcpyw(dst,src,n)             memcpy(dst,src,(n)*2)
#define memcpyl(dst,src,n)             memcpy(dst,src,(n)*4)
#define memsetw                    dee_memsetw
#define memsetl                    dee_memsetl
LOCAL void dee_memsetw(uint16_t *__restrict p, uint16_t c, size_t n) {
 while (n--) *p++ = c;
}
LOCAL void dee_memsetl(uint32_t *__restrict p, uint32_t c, size_t n) {
 while (n--) *p++ = c;
}
#endif
#define memsetb(p,c,n)     ((uint8_t *)memset(p,c,n))
#define memchrb(p,c,n)     ((uint8_t *)memchr(p,c,n))
#define memrchrb(p,c,n)    ((uint8_t *)memrchr(p,c,n))
#define memchrw                    dee_memchrw
#define memchrl                    dee_memchrl
#define memrchrw                   dee_memrchrw
#define memrchrl                   dee_memrchrl
LOCAL uint16_t *dee_memchrw(uint16_t const *__restrict p, uint16_t c, size_t n) {
 for (; n--; ++p) if (*p == c) return (uint16_t *)p;
 return NULL;
}
LOCAL uint32_t *dee_memchrl(uint32_t const *__restrict p, uint32_t c, size_t n) {
 for (; n--; ++p) if (*p == c) return (uint32_t *)p;
 return NULL;
}
LOCAL uint16_t *dee_memrchrw(uint16_t const *__restrict p, uint16_t c, size_t n) {
 uint16_t *iter = (uint16_t *)p+n;
 while (iter != (uint16_t *)p) {
  if (*--iter == c) return iter;
 }
 return NULL;
}
LOCAL uint32_t *dee_memrchrl(uint32_t const *__restrict p, uint32_t c, size_t n) {
 uint32_t *iter = (uint32_t *)p+n;
 while (iter != (uint32_t *)p) {
  if (*--iter == c) return iter;
 }
 return NULL;
}
#endif /* __USE_KOS */

#define MEMEQB(a,b,s) (memcmp(a,b,s) == 0)
#ifdef __USE_KOS
#define MEMEQW(a,b,s) (memcmpw((uint16_t *)(a),(uint16_t *)(b),s) == 0)
#define MEMEQL(a,b,s) (memcmpl((uint32_t *)(a),(uint32_t *)(b),s) == 0)
#else
#define MEMEQW(a,b,s) (memcmp(a,b,(s)*2) == 0)
#define MEMEQL(a,b,s) (memcmp(a,b,(s)*4) == 0)
#endif

#define memrmem  dee_memrmem
LOCAL void *dee_memrmem(void const *__restrict haystack, size_t haystack_length,
                        void const *__restrict needle, size_t needle_length) {
 void const *candidate; uint8_t marker;
 if unlikely(!needle_length || needle_length > haystack_length)
    return NULL;
 haystack_length -= needle_length-1,marker = *(uint8_t *)needle;
 while ((candidate = memrchr(haystack,marker,haystack_length)) != NULL) {
  if (MEMEQB(candidate,needle,needle_length))
      return (void *)candidate;
  haystack_length = (uintptr_t)candidate - (uintptr_t)haystack;
 }
 return NULL;
}
#define memmemb(haystack,haystack_length,needle,needle_length)  \
     ((uint8_t *)memmem(haystack,haystack_length,needle,needle_length))
#define memrmemb(haystack,haystack_length,needle,needle_length)  \
     ((uint8_t *)memrmem(haystack,haystack_length,needle,needle_length))

#define memmemw  dee_memmemw
LOCAL uint16_t *dee_memmemw(uint16_t const *__restrict haystack, size_t haystack_length,
                            uint16_t const *__restrict needle, size_t needle_length) {
 uint16_t const *candidate; uint16_t marker;
 if unlikely(!needle_length || needle_length > haystack_length)
    return NULL;
 haystack_length -= needle_length-1,marker = *(uint16_t *)needle;
 while ((candidate = memchrw(haystack,marker,haystack_length)) != NULL) {
  if (MEMEQW(candidate,needle,needle_length))
      return (uint16_t *)candidate;
  ++candidate;
  haystack_length = (haystack + haystack_length) - candidate;
  haystack        = candidate;
 }
 return NULL;
}
#define memrmemw  dee_memrmemw
LOCAL uint16_t *dee_memrmemw(uint16_t const *__restrict haystack, size_t haystack_length,
                             uint16_t const *__restrict needle, size_t needle_length) {
 uint16_t const *candidate; uint16_t marker;
 if unlikely(!needle_length || needle_length > haystack_length)
    return NULL;
 haystack_length -= needle_length-1,marker = *(uint16_t *)needle;
 while ((candidate = memrchrw(haystack,marker,haystack_length)) != NULL) {
  if (MEMEQW(candidate,needle,needle_length))
      return (uint16_t *)candidate;
  haystack_length = candidate - haystack;
 }
 return NULL;
}

#define memmeml  dee_memmeml
LOCAL uint32_t *dee_memmeml(uint32_t const *__restrict haystack, size_t haystack_length,
                            uint32_t const *__restrict needle, size_t needle_length) {
 uint32_t const *candidate; uint32_t marker;
 if unlikely(!needle_length || needle_length > haystack_length)
    return NULL;
 haystack_length -= needle_length-1,marker = *(uint32_t *)needle;
 while ((candidate = memchrl(haystack,marker,haystack_length)) != NULL) {
  if (MEMEQL(candidate,needle,needle_length))
      return (uint32_t *)candidate;
  ++candidate;
  haystack_length = (haystack + haystack_length) - candidate;
  haystack        = candidate;
 }
 return NULL;
}
#define memrmeml  dee_memrmeml
LOCAL uint32_t *dee_memrmeml(uint32_t const *__restrict haystack, size_t haystack_length,
                             uint32_t const *__restrict needle, size_t needle_length) {
 uint32_t const *candidate; uint32_t marker;
 if unlikely(!needle_length || needle_length > haystack_length)
    return NULL;
 haystack_length -= needle_length-1,marker = *(uint32_t *)needle;
 while ((candidate = memrchrl(haystack,marker,haystack_length)) != NULL) {
  if (MEMEQL(candidate,needle,needle_length))
      return (uint32_t *)candidate;
  haystack_length = candidate - haystack;
 }
 return NULL;
}







/* Case-insensitive string functions. */
#define memcasechr(haystack,needle,haystack_length)    dee_memcasechrb((uint8_t *)(haystack),(uint8_t)(needle),haystack_length)
#define memcaserchr(haystack,needle,haystack_length)   dee_memcaserchrb((uint8_t *)(haystack),(uint8_t)(needle),haystack_length)
#define memcasechrb(haystack,needle,haystack_length)   dee_memcasechrb(haystack,needle,haystack_length)
#define memcaserchrb(haystack,needle,haystack_length)  dee_memcaserchrb(haystack,needle,haystack_length)
#define memcasechrw(haystack,needle,haystack_length)   dee_memcasechrw(haystack,needle,haystack_length)
#define memcaserchrw(haystack,needle,haystack_length)  dee_memcaserchrw(haystack,needle,haystack_length)
#define memcasechrl(haystack,needle,haystack_length)   dee_memcasechrl(haystack,needle,haystack_length)
#define memcaserchrl(haystack,needle,haystack_length)  dee_memcaserchrl(haystack,needle,haystack_length)

#ifdef __INTELLISENSE__
#define DEFINE_FOLD_COMPARE(name,T) \
PRIVATE size_t DCALL \
name(T const *__restrict data, size_t datalen, \
     uint32_t fold[UNICODE_FOLDED_MAX], size_t fold_len);
#else
#define DEFINE_FOLD_COMPARE(name,T) \
PRIVATE size_t DCALL \
name(T const *__restrict data, size_t datalen, \
     uint32_t fold[UNICODE_FOLDED_MAX], size_t fold_len) { \
 uint32_t buf[UNICODE_FOLDED_MAX]; \
 size_t buflen; \
 ASSERT(datalen >= 1); \
 buflen = DeeUni_ToFolded(data[0],buf); \
 switch (fold_len) { \
 case 1: \
  if (buf[0] == fold[0]) \
      goto ok_1; \
  if (buflen >= 2 && buf[1] == fold[0]) \
      goto ok_1; \
  if (buflen >= 3 && buf[2] == fold[0]) \
      goto ok_1; \
  break; \
 case 2: \
  if (buflen == 1) { \
   if (buf[0] != fold[0]) break; \
   if (datalen < 2) break; \
   DeeUni_ToFolded(data[1],buf); \
   if (buf[0] == fold[1]) goto ok_2; \
  } else if (buflen == 2) { \
   if (buf[0] == fold[0] && buf[1] == fold[1]) \
       goto ok_1; \
   if (buf[1] == fold[0]) { \
    if (datalen < 2) break; \
    DeeUni_ToFolded(data[1],buf); \
    if (buf[0] == fold[1]) goto ok_2; \
   } \
  } else { \
   if (buf[0] == fold[0] && buf[1] == fold[1]) \
       goto ok_1; \
   if (buf[1] == fold[0] && buf[2] == fold[1]) \
       goto ok_1; \
   if (buf[2] == fold[0]) { \
    if (datalen < 2) break; \
    DeeUni_ToFolded(data[1],buf); \
    if (buf[0] == fold[1]) goto ok_2; \
   } \
  } \
  break; \
 case 3: \
  if (buflen == 1) { \
   if (buf[0] != fold[0]) break; \
   if (datalen < 2) break; \
   buflen = DeeUni_ToFolded(data[1],buf); \
   if (buf[0] != fold[1]) break; \
   if (buflen == 1) { \
    if (datalen < 3) break; \
    DeeUni_ToFolded(data[2],buf); \
    if (buf[0] == fold[2]) \
        goto ok_3; \
   } else { \
    if (buf[1] == fold[2]) \
        goto ok_2; \
   } \
  } else if (buflen == 2) { \
   if (buf[0] == fold[0] && buf[1] == fold[1]) { \
    if (datalen < 2) break; \
    DeeUni_ToFolded(data[1],buf); \
    if (buf[0] == fold[2]) goto ok_2; \
   } \
   if (buf[1] == fold[0]) { \
    if (datalen < 2) break; \
    buflen = DeeUni_ToFolded(data[1],buf); \
    if (buf[0] != fold[1]) break; \
    if (buflen == 1) { \
     if (datalen < 3) break; \
     DeeUni_ToFolded(data[2],buf); \
     if (buf[0] == fold[2]) \
         goto ok_3; \
    } else { \
     if (buf[1] == fold[2]) \
         goto ok_2; \
    } \
   } \
  } else { \
   if (buf[0] == fold[0] && buf[1] == fold[1] && buf[2] == fold[2]) \
       goto ok_1; \
   if (buf[1] == fold[0] && buf[2] == fold[1]) { \
    if (datalen < 2) break; \
    DeeUni_ToFolded(data[1],buf); \
    if (buf[0] == fold[2]) goto ok_2; \
   } \
   if (buf[2] == fold[0]) { \
    if (datalen < 2) break; \
    buflen = DeeUni_ToFolded(data[1],buf); \
    if (buf[0] != fold[1]) break; \
    if (buflen == 1) { \
     if (datalen < 3) break; \
     buflen = DeeUni_ToFolded(data[2],buf); \
     if (buf[0] == fold[2]) \
         goto ok_3; \
    } else { \
     if (buf[1] == fold[2]) \
         goto ok_2; \
    } \
   } \
  } \
  break; \
 default: __builtin_unreachable(); \
 } \
 return 0; \
ok_1: \
 return 1; \
ok_2: \
 return 2; \
ok_3: \
 return 3; \
}
#endif
DEFINE_FOLD_COMPARE(dee_foldcmpb,uint8_t)
DEFINE_FOLD_COMPARE(dee_foldcmpw,uint16_t)
DEFINE_FOLD_COMPARE(dee_foldcmpl,uint32_t)
#undef DEFINE_FOLD_COMPARE



#define DEFINE_MEMCASECHR(name,rname,T,dee_foldcmp) \
LOCAL T *DCALL \
name(T const *__restrict haystack, \
     T needle, size_t haystack_length) { \
 uint32_t fold[UNICODE_FOLDED_MAX]; \
 size_t len = DeeUni_ToFolded(needle,fold); \
 for (; haystack_length; ++haystack,--haystack_length) { \
  if (dee_foldcmp(haystack,haystack_length,fold,len)) \
      return (T *)haystack; \
 } \
 return NULL; \
} \
LOCAL T *DCALL \
rname(T const *__restrict haystack, \
      T needle, size_t haystack_length) { \
 T *iter = (T *)haystack+haystack_length; \
 uint32_t fold[UNICODE_FOLDED_MAX]; \
 size_t len = DeeUni_ToFolded(needle,fold); \
 size_t datalen = 0; \
 while (iter-- != (T *)haystack) { \
  ++datalen; \
  if (dee_foldcmp(iter,datalen,fold,len)) \
      return iter; \
 } \
 return NULL; \
}
DEFINE_MEMCASECHR(dee_memcasechrb,dee_memcaserchrb,uint8_t,dee_foldcmpb)
DEFINE_MEMCASECHR(dee_memcasechrw,dee_memcaserchrw,uint16_t,dee_foldcmpw)
DEFINE_MEMCASECHR(dee_memcasechrl,dee_memcaserchrl,uint32_t,dee_foldcmpl)
#undef DEFINE_MEMCASECHR

#define DEFINE_UNICODE_FOLDREADER_API(name,T) \
struct name { \
    T const *uf_dataptr; \
    size_t   uf_datalen; \
    uint32_t uf_buf[UNICODE_FOLDED_MAX]; \
    uint8_t  uf_len; \
    uint8_t  uf_idx; \
}; \
LOCAL uint32_t DCALL \
name##_getc(struct name *__restrict self) \
{ \
 if (self->uf_idx < self->uf_len) \
     return self->uf_buf[self->uf_idx++]; \
 ASSERT(self->uf_datalen); \
 self->uf_idx = 1; \
 self->uf_len = (uint8_t)DeeUni_ToFolded(*self->uf_dataptr,self->uf_buf); \
 ++self->uf_dataptr; \
 --self->uf_datalen; \
 return self->uf_buf[0]; \
} \
LOCAL uint32_t DCALL \
name##_getc_back(struct name *__restrict self) \
{ \
 if (self->uf_idx < self->uf_len) \
     return self->uf_buf[--self->uf_len]; \
 ASSERT(self->uf_datalen); \
 self->uf_idx = 0; \
 --self->uf_datalen; \
 self->uf_len = (uint8_t)DeeUni_ToFolded(self->uf_dataptr[self->uf_datalen], \
                                         self->uf_buf) - 1; \
 return self->uf_buf[self->uf_len]; \
}
DEFINE_UNICODE_FOLDREADER_API(unicode_foldreaderb,uint8_t)
DEFINE_UNICODE_FOLDREADER_API(unicode_foldreaderw,uint16_t)
DEFINE_UNICODE_FOLDREADER_API(unicode_foldreaderl,uint32_t)
#undef DEFINE_UNICODE_FOLDREADER_API

#ifdef __INTELLISENSE__
extern "C++" {
uint32_t unicode_foldreader_getc(struct unicode_foldreaderb &x);
uint32_t unicode_foldreader_getc(struct unicode_foldreaderw &x);
uint32_t unicode_foldreader_getc(struct unicode_foldreaderl &x);
uint32_t unicode_foldreader_getc_back(struct unicode_foldreaderb &x);
uint32_t unicode_foldreader_getc_back(struct unicode_foldreaderw &x);
uint32_t unicode_foldreader_getc_back(struct unicode_foldreaderl &x);
}
#elif !defined(__NO_builtin_choose_expr)
#define unicode_foldreader_getc(x) \
      __builtin_choose_expr(sizeof(*(x).uf_dataptr) == 1,unicode_foldreaderb_getc((struct unicode_foldreaderb *)&(x)), \
      __builtin_choose_expr(sizeof(*(x).uf_dataptr) == 2,unicode_foldreaderw_getc((struct unicode_foldreaderw *)&(x)), \
                                                         unicode_foldreaderl_getc((struct unicode_foldreaderl *)&(x)))) \

#define unicode_foldreader_getc_back(x) \
      __builtin_choose_expr(sizeof(*(x).uf_dataptr) == 1,unicode_foldreaderb_getc_back((struct unicode_foldreaderb *)&(x)), \
      __builtin_choose_expr(sizeof(*(x).uf_dataptr) == 2,unicode_foldreaderw_getc_back((struct unicode_foldreaderw *)&(x)), \
                                                         unicode_foldreaderl_getc_back((struct unicode_foldreaderl *)&(x)))) \

#else
#define unicode_foldreader_getc(x) \
      (sizeof(*(x).uf_dataptr) == 1 ? unicode_foldreaderb_getc((struct unicode_foldreaderb *)&(x)) : \
       sizeof(*(x).uf_dataptr) == 2 ? unicode_foldreaderw_getc((struct unicode_foldreaderw *)&(x)) : \
                                      unicode_foldreaderl_getc((struct unicode_foldreaderl *)&(x)))
#define unicode_foldreader_getc_back(x) \
      (sizeof(*(x).uf_dataptr) == 1 ? unicode_foldreaderb_getc_back((struct unicode_foldreaderb *)&(x)) : \
       sizeof(*(x).uf_dataptr) == 2 ? unicode_foldreaderw_getc_back((struct unicode_foldreaderw *)&(x)) : \
                                      unicode_foldreaderl_getc_back((struct unicode_foldreaderl *)&(x)))
#endif
#define unicode_foldreader_init(x,data,len) \
      ((x).uf_dataptr = (data),(x).uf_datalen = (len), \
       (x).uf_len = (x).uf_idx = 0)
#define unicode_foldreader_empty(x) \
     (!(x).uf_datalen && ((x).uf_idx >= (x).uf_len))
#define DEE_PRIVATE_UNICODE_FOLDREADER_uint8_t   unicode_foldreaderb
#define DEE_PRIVATE_UNICODE_FOLDREADER_uint16_t  unicode_foldreaderw
#define DEE_PRIVATE_UNICODE_FOLDREADER_uint32_t  unicode_foldreaderl
#define unicode_foldreader(T)  struct PP_PRIVATE_CAT2(DEE_PRIVATE_UNICODE_FOLDREADER_,T)





/* Use libc functions for case-insensitive UTF-8 string compare when available. */
#define MEMCASEEQB(a,a_size,b,b_size)         dee_memcaseeqb(a,a_size,b,b_size)
#define MEMCASEEQW(a,a_size,b,b_size)         dee_memcaseeqw(a,a_size,b,b_size)
#define MEMCASEEQL(a,a_size,b,b_size)         dee_memcaseeql(a,a_size,b,b_size)
#define MEMCASESTARTSWITHB(a,a_size,b,b_size) dee_memcasestartswithb(a,a_size,b,b_size)
#define MEMCASESTARTSWITHW(a,a_size,b,b_size) dee_memcasestartswithw(a,a_size,b,b_size)
#define MEMCASESTARTSWITHL(a,a_size,b,b_size) dee_memcasestartswithl(a,a_size,b,b_size)
#define MEMCASEENDSWITHB(a,a_size,b,b_size)   dee_memcaseendswithb(a,a_size,b,b_size)
#define MEMCASEENDSWITHW(a,a_size,b,b_size)   dee_memcaseendswithw(a,a_size,b,b_size)
#define MEMCASEENDSWITHL(a,a_size,b,b_size)   dee_memcaseendswithl(a,a_size,b,b_size)

#define DEFINE_MEMCASEEQ(name,T) \
LOCAL bool DCALL \
name(T const *a, size_t a_size, \
     T const *b, size_t b_size) { \
 unicode_foldreader(T) a_reader; \
 unicode_foldreader(T) b_reader; \
 unicode_foldreader_init(a_reader,a,a_size); \
 unicode_foldreader_init(b_reader,b,b_size); \
 while (!unicode_foldreader_empty(a_reader)) { \
  if (unicode_foldreader_empty(b_reader)) \
      return false; \
  if (unicode_foldreader_getc(a_reader) != \
      unicode_foldreader_getc(b_reader)) \
      return false; \
 } \
 return unicode_foldreader_empty(b_reader); \
}
DEFINE_MEMCASEEQ(dee_memcaseeqb,uint8_t)
DEFINE_MEMCASEEQ(dee_memcaseeqw,uint16_t)
DEFINE_MEMCASEEQ(dee_memcaseeql,uint32_t)
#undef DEFINE_MEMCASEEQ

#define DEFINE_MEMCASESTARTSWITH(name,T) \
LOCAL size_t DCALL \
name(T const *a, size_t a_size, \
     T const *b, size_t b_size) { \
 unicode_foldreader(T) a_reader; \
 unicode_foldreader(T) b_reader; \
 unicode_foldreader_init(a_reader,a,a_size); \
 unicode_foldreader_init(b_reader,b,b_size); \
 while (!unicode_foldreader_empty(b_reader)) { \
  if (unicode_foldreader_empty(a_reader) || \
     (unicode_foldreader_getc(a_reader) != \
      unicode_foldreader_getc(b_reader))) \
      return 0; \
 } \
 return (size_t)(a_reader.uf_dataptr - a); \
}
DEFINE_MEMCASESTARTSWITH(dee_memcasestartswithb,uint8_t)
DEFINE_MEMCASESTARTSWITH(dee_memcasestartswithw,uint16_t)
DEFINE_MEMCASESTARTSWITH(dee_memcasestartswithl,uint32_t)
#undef DEFINE_MEMCASESTARTSWITH

#define DEFINE_MEMCASEENDSWITH(name,T) \
LOCAL size_t DCALL \
name(T const *a, size_t a_size, \
     T const *b, size_t b_size) { \
 unicode_foldreader(T) a_reader; \
 unicode_foldreader(T) b_reader; \
 unicode_foldreader_init(a_reader,a,a_size); \
 unicode_foldreader_init(b_reader,b,b_size); \
 while (!unicode_foldreader_empty(b_reader)) { \
  if (unicode_foldreader_empty(a_reader) || \
     (unicode_foldreader_getc_back(a_reader) != \
      unicode_foldreader_getc_back(b_reader))) \
      return 0; \
 } \
 return a_size - a_reader.uf_datalen; \
}
DEFINE_MEMCASEENDSWITH(dee_memcaseendswithb,uint8_t)
DEFINE_MEMCASEENDSWITH(dee_memcaseendswithw,uint16_t)
DEFINE_MEMCASEENDSWITH(dee_memcaseendswithl,uint32_t)
#undef DEFINE_MEMCASEENDSWITH

#define memcasememb   dee_memcasememb
#define memcasermemb  dee_memcasermemb
#define memcasememw   dee_memcasememw
#define memcasermemw  dee_memcasermemw
#define memcasememl   dee_memcasememl
#define memcasermeml  dee_memcasermeml

#define DEFINE_MEMCASEMEM(name,rname,T,MEMCASESTARTSWITH,MEMCASEENDSWITH) \
LOCAL T *DCALL \
name(T const *__restrict haystack, size_t haystack_length, \
     T const *__restrict needle, size_t needle_length, \
      size_t *pmatch_length) { \
 for (; haystack_length; --haystack_length,++haystack) { \
  size_t match_length; \
  match_length = MEMCASESTARTSWITH(haystack,haystack_length,needle,needle_length); \
  if (match_length) { \
   if (pmatch_length) \
      *pmatch_length = match_length; \
   return (T *)haystack; \
  } \
 } \
 return NULL; \
} \
LOCAL T *DCALL \
rname(T const *__restrict haystack, size_t haystack_length, \
      T const *__restrict needle, size_t needle_length, \
      size_t *pmatch_length) { \
 for (; haystack_length; --haystack_length) { \
  size_t match_length; \
  match_length = MEMCASEENDSWITH(haystack,haystack_length,needle,needle_length); \
  if (match_length) { \
   if (pmatch_length) \
      *pmatch_length = match_length; \
   return (T *)haystack + haystack_length - match_length; \
  } \
 } \
 return NULL; \
}
DEFINE_MEMCASEMEM(dee_memcasememb,dee_memcasermemb,uint8_t,MEMCASESTARTSWITHB,MEMCASEENDSWITHB)
DEFINE_MEMCASEMEM(dee_memcasememw,dee_memcasermemw,uint16_t,MEMCASESTARTSWITHW,MEMCASEENDSWITHW)
DEFINE_MEMCASEMEM(dee_memcasememl,dee_memcasermeml,uint32_t,MEMCASESTARTSWITHL,MEMCASEENDSWITHL)
#undef DEFINE_MEMCASEMEM

LOCAL int DCALL
dee_memcasecmpb(uint8_t const *a, size_t a_size,
                uint8_t const *b, size_t b_size) {
 unicode_foldreader(uint8_t) a_reader;
 unicode_foldreader(uint8_t) b_reader;
 unicode_foldreader_init(a_reader,a,a_size);
 unicode_foldreader_init(b_reader,b,b_size);
 for (;;) {
  uint32_t cha,chb;
  if (unicode_foldreader_empty(a_reader))
      return unicode_foldreader_empty(b_reader) ? 0 : -1;
  if (unicode_foldreader_empty(b_reader))
      return 1;
  cha = unicode_foldreader_getc(a_reader);
  chb = unicode_foldreader_getc(b_reader);
  if (cha != chb)
      return cha < chb ? -1 : 1;
 }
}



/* ASCII case-insensitive functions (for `bytes'). */
LOCAL bool DCALL
dee_memasciicaseeq(uint8_t const *a, uint8_t const *b, size_t s) {
 while (s--) {
  uint8_t lhs = *a;
  uint8_t rhs = *b;
  if (lhs != rhs) {
   lhs = (uint8_t)DeeUni_ToLower(lhs);
   rhs = (uint8_t)DeeUni_ToLower(rhs);
   if (lhs != rhs)
       return false;
  }
  ++a;
  ++b;
 }
 return true;
}

LOCAL uint8_t *DCALL
dee_memasciicasechr(uint8_t const *__restrict haystack,
                    uint8_t needle, size_t haystack_length) {
 needle = (uint8_t)DeeUni_ToLower(needle);
 while (haystack_length--) {
  if ((uint8_t)DeeUni_ToLower(*haystack) == needle)
       return (uint8_t *)haystack;
  ++haystack;
 }
 return NULL;
}
LOCAL uint8_t *DCALL
dee_memasciicaserchr(uint8_t const *__restrict haystack,
                     uint8_t needle, size_t haystack_length) {
 uint8_t *iter = (uint8_t *)haystack+haystack_length;
 needle = (uint8_t)DeeUni_ToLower(needle);
 while (iter-- != (uint8_t *)haystack) {
  if ((uint8_t)DeeUni_ToLower(*iter) == needle)
       return iter;
 }
 return NULL;
}
LOCAL uint8_t *DCALL
dee_memasciicasemem(uint8_t const *haystack, size_t haystack_length,
                    uint8_t const *needle, size_t needle_length) {
 uint8_t *candidate; uint8_t marker;
 if unlikely(!needle_length || needle_length > haystack_length)
    return NULL;
 haystack_length -= needle_length-1;
 marker = (uint8_t)DeeUni_ToLower(*needle);
 while ((candidate = dee_memasciicasechr(haystack,marker,haystack_length)) != NULL) {
  if (dee_memasciicaseeq(candidate,needle,needle_length))
      return candidate;
  ++candidate;
  haystack_length = (size_t)((haystack + haystack_length) - candidate);
  haystack        = candidate;
 }
 return NULL;
}
LOCAL uint8_t *DCALL
dee_memasciicasermem(uint8_t const *haystack, size_t haystack_length,
                     uint8_t const *needle, size_t needle_length) {
 uint8_t *candidate; uint8_t marker;
 if unlikely(!needle_length || needle_length > haystack_length)
    return NULL;
 marker = (uint8_t)DeeUni_ToLower(*needle);
 haystack_length -= needle_length-1;
 while ((candidate = dee_memasciicaserchr(haystack,marker,haystack_length)) != NULL) {
  if (dee_memasciicaseeq(candidate,needle,needle_length))
      return candidate;
  if unlikely(candidate == haystack) break;
  haystack_length = (size_t)(candidate - haystack);
 }
 return NULL;
}

#define memasciicasecmp  dee_memasciicasecmp
LOCAL int dee_memasciicasecmp(uint8_t const *a, uint8_t const *b, size_t s) {
 while (s--) {
  uint8_t lhs = *a;
  uint8_t rhs = *b;
  if (lhs != rhs) {
   lhs = (uint8_t)DeeUni_ToLower(lhs);
   rhs = (uint8_t)DeeUni_ToLower(rhs);
   if (lhs != rhs)
       return (int)lhs - (int)rhs;
  }
  ++a;
  ++b;
 }
 return 0;
}




#define STRCASEEQ(a,b) dee_strcaseeq(a,b)
LOCAL bool DCALL dee_strcaseeq(char const *a, char const *b) {
 while (*a && DeeUni_ToLower(*a) == DeeUni_ToLower(*b)) ++a,++b;
 return !*b;
}

LOCAL bool DCALL dee_asciicaseeq(char const *a, char const *b, size_t length) {
 while (length--) {
  if (DeeUni_ToLower(*a) != DeeUni_ToLower(*b))
      return false;
  ++a,++b;
 }
 return true;
}


/* As found here: https://en.wikipedia.org/wiki/Levenshtein_distance */
#define DEFINE_FUZZY_COMPARE_FUNCTION(name,T,transform) \
PRIVATE dssize_t DCALL \
name(T const *__restrict a, size_t alen, \
     T const *__restrict b, size_t blen) { \
 size_t *v0,*v1,i,j,cost,temp; \
 if unlikely(!alen) return blen; \
 if unlikely(!blen) return alen; \
 v0 = (size_t *)Dee_AMalloc((blen+1)*sizeof(size_t)); \
 if unlikely(!v0) return -1; \
 v1 = (size_t *)Dee_AMalloc((blen+1)*sizeof(size_t)); \
 if unlikely(!v1) { Dee_AFree(v0); return -1; } \
 for (i = 0; i < blen; ++i) v0[i] = i; \
 for (i = 0; i < alen; ++i) { \
  T a_value = transform(a[i]); \
  v1[0] = i+1; \
  for (j = 0; j < blen; ++j) { \
   cost  = (a_value == transform(b[j])) ? 0u : 1u; \
   cost += v0[j]; \
   temp  = v1[j]+1; \
   if (temp < cost) cost = temp; \
   temp  = v0[j+1]+1; \
   if (temp < cost) cost = temp; \
   v1[j+1] = cost; \
  } \
  memcpy(v0,v1,blen*sizeof(size_t)); \
 } \
 temp = v1[blen]; \
 Dee_AFree(v0); \
 Dee_AFree(v1); \
 if (temp > SSIZE_MAX) temp = SSIZE_MAX; \
 return temp; \
}
DEFINE_FUZZY_COMPARE_FUNCTION(fuzzy_compareb,uint8_t,)
DEFINE_FUZZY_COMPARE_FUNCTION(fuzzy_comparew,uint16_t,)
DEFINE_FUZZY_COMPARE_FUNCTION(fuzzy_comparel,uint32_t,)
DEFINE_FUZZY_COMPARE_FUNCTION(fuzzy_asciicasecompareb,uint8_t,(uint8_t)DeeUni_ToLower)
#undef DEFINE_FUZZY_COMPARE_FUNCTION

#define DEFINE_FUZZY_FOLDCOMPARE_FUNCTION(name,T) \
PRIVATE dssize_t DCALL \
name(T const *__restrict a, size_t alen, \
     T const *__restrict b, size_t blen) { \
 size_t *v0,*v1,i,j,cost,temp; \
 unicode_foldreader(T) a_reader; \
 unicode_foldreader(T) b_reader; \
 size_t folded_alen,folded_blen; \
 folded_alen = DeeUni_FoldedLength(a,alen); \
 folded_blen = DeeUni_FoldedLength(b,blen); \
 if unlikely(!folded_alen) return folded_blen; \
 if unlikely(!folded_blen) return folded_alen; \
 v0 = (size_t *)Dee_AMalloc((folded_blen+1)*sizeof(size_t)); \
 if unlikely(!v0) return -1; \
 v1 = (size_t *)Dee_AMalloc((folded_blen+1)*sizeof(size_t)); \
 if unlikely(!v1) { Dee_AFree(v0); return -1; } \
 for (i = 0; i < folded_blen; ++i) v0[i] = i; \
 unicode_foldreader_init(a_reader,a,alen); \
 for (i = 0; i < folded_alen; ++i) { \
  uint32_t a_value = unicode_foldreader_getc(a_reader); \
  v1[0] = i+1; \
  unicode_foldreader_init(b_reader,b,blen); \
  for (j = 0; j < folded_blen; ++j) { \
   cost  = (a_value == unicode_foldreader_getc(b_reader)) ? 0u : 1u; \
   cost += v0[j]; \
   temp  = v1[j]+1; \
   if (temp < cost) cost = temp; \
   temp  = v0[j+1]+1; \
   if (temp < cost) cost = temp; \
   v1[j+1] = cost; \
  } \
  ASSERT(unicode_foldreader_empty(b_reader)); \
  memcpy(v0,v1,folded_blen*sizeof(size_t)); \
 } \
 ASSERT(unicode_foldreader_empty(a_reader)); \
 temp = v1[folded_blen]; \
 Dee_AFree(v0); \
 Dee_AFree(v1); \
 if (temp > SSIZE_MAX) temp = SSIZE_MAX; \
 return temp; \
}
DEFINE_FUZZY_FOLDCOMPARE_FUNCTION(fuzzy_casecompareb,uint8_t)
DEFINE_FUZZY_FOLDCOMPARE_FUNCTION(fuzzy_casecomparew,uint16_t)
DEFINE_FUZZY_FOLDCOMPARE_FUNCTION(fuzzy_casecomparel,uint32_t)
#undef DEFINE_FUZZY_FOLDCOMPARE_FUNCTION

#define DEFINE_VERSION_COMPARE_FUNCTION(name,T,Ts,transform,IF_TRANSFORM) \
PRIVATE Ts DCALL \
name(T *__restrict a, size_t a_size, \
     T *__restrict b, size_t b_size) { \
 T ca,cb,*a_start = a; \
 while (a_size && b_size) { \
  if ((ca = *a) != (cb = *b) \
       IF_TRANSFORM(&& ((ca = (T)transform(ca)) != (cb = (T)transform(cb))))) { \
   struct unitraits *arec; \
   struct unitraits *brec; \
   unsigned int vala,valb; \
   /* Unwind common digits. */ \
   while (a != a_start) { \
    if (!DeeUni_IsDigit(a[-1])) \
         break; \
    cb = ca = *--a,--b; \
   } \
   /* Check if both strings have digit sequences in the same places. */ \
   arec = DeeUni_Descriptor(ca); \
   brec = DeeUni_Descriptor(cb); \
   if (!(arec->ut_flags & UNICODE_FDIGIT) && \
       !(brec->ut_flags & UNICODE_FDIGIT)) \
        return (int)ca - (int)cb; \
   /* Deal with leading zeros. */ \
   if ((arec->ut_flags & UNICODE_FDIGIT) && arec->ut_digit == 0) return -1; \
   if ((brec->ut_flags & UNICODE_FDIGIT) && brec->ut_digit == 0) return 1; \
   /* Compare digits. */ \
   vala = arec->ut_digit; \
   valb = brec->ut_digit; \
   while (a_size) { \
    ca = *a++,--a_size; \
    arec = DeeUni_Descriptor(ca); \
    if (!(arec->ut_flags & UNICODE_FDIGIT)) \
          break; \
    vala *= 10; \
    vala += arec->ut_digit; \
   } \
   while (b_size) { \
    cb = *b++,--b_size; \
    brec = DeeUni_Descriptor(cb); \
    if (!(brec->ut_flags & UNICODE_FDIGIT)) \
          break; \
    valb *= 10; \
    valb += brec->ut_digit; \
   } \
   return (Ts)vala - (Ts)valb; \
  } \
  ++a,--a_size; \
  ++b,--b_size; \
 } \
 return (Ts)((dssize_t)a_size-(dssize_t)b_size); \
}
#define DEE_PRIVATE_IF_FALSE(x)
#define DEE_PRIVATE_IF_TRUE(x) x
DEFINE_VERSION_COMPARE_FUNCTION(dee_strverscmpb,uint8_t,int8_t,,DEE_PRIVATE_IF_FALSE)
DEFINE_VERSION_COMPARE_FUNCTION(dee_strverscmpw,uint16_t,int16_t,,DEE_PRIVATE_IF_FALSE)
DEFINE_VERSION_COMPARE_FUNCTION(dee_strverscmpl,uint32_t,int32_t,,DEE_PRIVATE_IF_FALSE)
DEFINE_VERSION_COMPARE_FUNCTION(dee_strcaseverscmpb,uint8_t,int8_t,DeeUni_ToLower,DEE_PRIVATE_IF_TRUE)   /* TODO: case-fold */
DEFINE_VERSION_COMPARE_FUNCTION(dee_strcaseverscmpw,uint16_t,int16_t,DeeUni_ToLower,DEE_PRIVATE_IF_TRUE) /* TODO: case-fold */
DEFINE_VERSION_COMPARE_FUNCTION(dee_strcaseverscmpl,uint32_t,int32_t,DeeUni_ToLower,DEE_PRIVATE_IF_TRUE) /* TODO: case-fold */
#undef DEE_PRIVATE_IF_TRUE
#undef DEE_PRIVATE_IF_FALSE
#undef DEFINE_VERSION_COMPARE_FUNCTION


/* Returns a pointer into `scan_str', or NULL if no match was found. */
#define DEFINE_FIND_MATCH_FUNCTION(name,T,memmem,MEMEQ) \
PRIVATE T *DCALL \
name(T *__restrict scan_str, size_t scan_size, \
     T *__restrict open_str, size_t open_size, \
     T *__restrict clos_str, size_t clos_size) { \
 size_t recursion = 0; \
 if unlikely(!clos_size) return NULL; \
 if unlikely(!open_size) \
    return memmem(scan_str,scan_size,clos_str,clos_size); \
 for (;;) { \
  if (scan_size < clos_size) \
      return NULL; \
  if (MEMEQ(scan_str,clos_str,clos_size)) { \
   if (!recursion) break; /* Found it! */ \
   --recursion; \
   scan_str  += clos_size; \
   scan_size -= clos_size; \
   continue; \
  } \
  if (scan_size >= open_size && \
      MEMEQ(scan_str,open_str,open_size)) { \
   ++recursion; \
   scan_str  += open_size; \
   scan_size -= open_size; \
   continue; \
  } \
  ++scan_str; \
  --scan_size; \
 } \
 return scan_str; \
}
#define DEFINE_CASEFIND_MATCH_FUNCTION(name,T,memcasemem,MEMCASESTARTSWITH) \
PRIVATE T *DCALL \
name(T *__restrict scan_str, size_t scan_size, \
     T *__restrict open_str, size_t open_size, \
     T *__restrict clos_str, size_t clos_size, \
     size_t *pmatch_length) { \
 size_t recursion = 0; \
 if unlikely(!clos_size) return NULL; \
 if unlikely(!open_size) \
    return memcasemem(scan_str,scan_size,clos_str,clos_size,pmatch_length); \
 while (scan_size) { \
  size_t length; \
  length = MEMCASESTARTSWITH(scan_str,scan_size,clos_str,clos_size); \
  if (length) { \
   if (!recursion) { \
    if (pmatch_length) \
       *pmatch_length = length; \
    return scan_str; \
   } \
   --recursion; \
   scan_str  += length; \
   scan_size -= length; \
   continue; \
  } \
  length = MEMCASESTARTSWITH(scan_str,scan_size,open_str,open_size); \
  if (length) { \
   ++recursion; \
   scan_str  += length; \
   scan_size -= length; \
   continue; \
  } \
  ++scan_str; \
  --scan_size; \
 } \
 return NULL; \
}
DEFINE_FIND_MATCH_FUNCTION(find_asciicasematchb,uint8_t,dee_memasciicasemem,dee_memasciicaseeq)
DEFINE_FIND_MATCH_FUNCTION(find_matchb,uint8_t,memmemb,MEMEQB)
DEFINE_FIND_MATCH_FUNCTION(find_matchw,uint16_t,memmemw,MEMEQW)
DEFINE_FIND_MATCH_FUNCTION(find_matchl,uint32_t,memmeml,MEMEQL)
DEFINE_CASEFIND_MATCH_FUNCTION(find_casematchb,uint8_t,memcasememb,MEMCASESTARTSWITHB)
DEFINE_CASEFIND_MATCH_FUNCTION(find_casematchw,uint16_t,memcasememw,MEMCASESTARTSWITHW)
DEFINE_CASEFIND_MATCH_FUNCTION(find_casematchl,uint32_t,memcasememl,MEMCASESTARTSWITHL)
#undef DEFINE_CASEFIND_MATCH_FUNCTION
#undef DEFINE_FIND_MATCH_FUNCTION

#define DEFINE_RFIND_MATCH_FUNCTION(name,T,memrmem,MEMEQ) \
PRIVATE T *DCALL \
name(T *__restrict scan_str, size_t scan_size, \
     T *__restrict open_str, size_t open_size, \
     T *__restrict clos_str, size_t clos_size) { \
 size_t recursion = 0; \
 if unlikely(!open_size) return NULL; \
 if unlikely(!clos_size) \
    return memrmem(scan_str,scan_size,open_str,open_size); \
 scan_str += scan_size; \
 for (;;) { \
  if (scan_size < open_size) \
      return NULL; \
  if (MEMEQ(scan_str - open_size,open_str,open_size)) { \
   scan_str -= open_size; \
   if (!recursion) break; /* Found it! */ \
   --recursion; \
   scan_size -= open_size; \
   continue; \
  } \
  if (scan_size >= clos_size && \
      MEMEQ(scan_str - clos_size,clos_str,clos_size)) { \
   ++recursion; \
   scan_str  -= clos_size; \
   scan_size -= clos_size; \
   continue; \
  } \
  --scan_str; \
  --scan_size; \
 } \
 return scan_str; \
}
#define DEFINE_CASERFIND_MATCH_FUNCTION(name,T,memcasermem,MEMCASEENDSWITH) \
PRIVATE T *DCALL \
name(T *__restrict scan_str, size_t scan_size, \
     T *__restrict open_str, size_t open_size, \
     T *__restrict clos_str, size_t clos_size, \
     size_t *pmatch_length) { \
 size_t recursion = 0; \
 if unlikely(!open_size) return NULL; \
 if unlikely(!clos_size) \
    return memcasermem(scan_str,scan_size,open_str,open_size,pmatch_length); \
 while (scan_size) { \
  size_t length; \
  length = MEMCASEENDSWITH(scan_str,scan_size,open_str,open_size); \
  if (length) { \
   scan_size -= length; \
   if (!recursion) { \
    if (pmatch_length) \
       *pmatch_length = length; \
    return scan_str + scan_size; \
   } \
   --recursion; \
   continue; \
  } \
  length = MEMCASEENDSWITH(scan_str,scan_size,clos_str,clos_size); \
  if (length) { \
   ++recursion; \
   scan_size -= length; \
   continue; \
  } \
  --scan_size; \
 } \
 return NULL; \
}
DEFINE_RFIND_MATCH_FUNCTION(rfind_asciicasematchb,uint8_t,dee_memasciicasermem,dee_memasciicaseeq)
DEFINE_RFIND_MATCH_FUNCTION(rfind_matchb,uint8_t,memrmemb,MEMEQB)
DEFINE_RFIND_MATCH_FUNCTION(rfind_matchw,uint16_t,memrmemw,MEMEQW)
DEFINE_RFIND_MATCH_FUNCTION(rfind_matchl,uint32_t,memrmeml,MEMEQL)
DEFINE_CASERFIND_MATCH_FUNCTION(rfind_casematchb,uint8_t,memcasermemb,MEMCASEENDSWITHB)
DEFINE_CASERFIND_MATCH_FUNCTION(rfind_casematchw,uint16_t,memcasermemw,MEMCASEENDSWITHW)
DEFINE_CASERFIND_MATCH_FUNCTION(rfind_casematchl,uint32_t,memcasermeml,MEMCASEENDSWITHL)
#undef DEFINE_CASERFIND_MATCH_FUNCTION
#undef DEFINE_RFIND_MATCH_FUNCTION


/* Pull in definitions for wild-compare functions. */
#define T           uint8_t
#define wildcompare wildcompareb
#include "wildcompare.c.inl"
#define T           uint16_t
#define wildcompare wildcomparew
#include "wildcompare.c.inl"
#define Treturn     int64_t
#define T           uint32_t
#define wildcompare wildcomparel
#include "wildcompare.c.inl"

#define NOCASE
#define T           uint8_t
#define wildcompare dee_wildasccicasecompareb
#include "wildcompare.c.inl"


#define CASEFOLD
#define T           uint8_t
#define wildcompare wildcasecompareb
#include "wildcompare.c.inl"
#define CASEFOLD
#define T           uint16_t
#define wildcompare wildcasecomparew
#include "wildcompare.c.inl"
#define CASEFOLD
#define T           uint32_t
#define wildcompare wildcasecomparel
#include "wildcompare.c.inl"

typedef DeeStringObject String;


PRIVATE void DCALL memfilb(uint8_t *__restrict dst, size_t num_bytes,
                           uint8_t const *__restrict src, size_t src_bytes) {
 ASSERT(src_bytes != 0);
 if (src_bytes == 1) {
  memsetb(dst,src[0],num_bytes);
 } else {
  while (num_bytes > src_bytes) {
   memcpyb(dst,src,src_bytes);
   num_bytes -= src_bytes;
   dst       += src_bytes;
  }
  memcpyb(dst,src,num_bytes);
 }
}
PRIVATE void DCALL memfilw(uint16_t *__restrict dst, size_t num_words,
                           uint16_t const *__restrict src, size_t src_words) {
 ASSERT(src_words != 0);
 if (src_words == 1) {
  memsetw(dst,src[0],num_words);
 } else {
  while (num_words > src_words) {
   memcpyw(dst,src,src_words);
   num_words -= src_words;
   dst       += src_words;
  }
  memcpyw(dst,src,num_words);
 }
}
PRIVATE void DCALL memfill(uint32_t *__restrict dst, size_t num_dwords,
                           uint32_t const *__restrict src, size_t src_dwords) {
 ASSERT(src_dwords != 0);
 if (src_dwords == 1) {
  memsetl(dst,src[0],num_dwords);
 } else {
  while (num_dwords > src_dwords) {
   memcpyl(dst,src,src_dwords);
   num_dwords -= src_dwords;
   dst       += src_dwords;
  }
  memcpyl(dst,src,num_dwords);
 }
}



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_STRING_FUNCTIONS_H */
