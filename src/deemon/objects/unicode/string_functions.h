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
LOCAL void *dee_memmem(void const *__restrict haystack, size_t haystack_len,
                       void const *__restrict needle, size_t needle_len) {
 void const *candidate; uint8_t marker;
 if unlikely(!needle_len || needle_len > haystack_len)
    return NULL;
 haystack_len -= needle_len-1,marker = *(uint8_t *)needle;
 while ((candidate = memchr(haystack,marker,haystack_len)) != NULL) {
  if (memcmp(candidate,needle,needle_len) == 0)
      return (void *)candidate;
  haystack_len = ((uintptr_t)haystack+haystack_len)-((uintptr_t)candidate+1);
  haystack     = (void const *)((uintptr_t)candidate+1);
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
LOCAL void *dee_memrmem(void const *__restrict haystack, size_t haystack_len,
                        void const *__restrict needle, size_t needle_len) {
 void const *candidate; uint8_t marker;
 if unlikely(!needle_len || needle_len > haystack_len)
    return NULL;
 haystack_len -= needle_len-1,marker = *(uint8_t *)needle;
 while ((candidate = memrchr(haystack,marker,haystack_len)) != NULL) {
  if (memcmp(candidate,needle,needle_len) == 0)
      return (void *)candidate;
  if unlikely(candidate == haystack) break;
  haystack_len = (uintptr_t)candidate-(uintptr_t)haystack;
 }
 return NULL;
}
#define memmemb(haystack,haystack_len,needle,needle_len)  \
     ((uint8_t *)memmem(haystack,haystack_len,needle,needle_len))
#define memrmemb(haystack,haystack_len,needle,needle_len)  \
     ((uint8_t *)memrmem(haystack,haystack_len,needle,needle_len))

#define memmemw  dee_memmemw
LOCAL uint16_t *dee_memmemw(uint16_t const *__restrict haystack, size_t haystack_len,
                            uint16_t const *__restrict needle, size_t needle_len) {
 uint16_t const *candidate; uint16_t marker;
 if unlikely(!needle_len || needle_len > haystack_len)
    return NULL;
 haystack_len -= needle_len-1,marker = *(uint16_t *)needle;
 while ((candidate = memchrw(haystack,marker,haystack_len)) != NULL) {
  if (MEMEQW(candidate,needle,needle_len))
      return (uint16_t *)candidate;
  haystack_len = (haystack+haystack_len)-(candidate+1);
  haystack     = candidate+1;
 }
 return NULL;
}
#define memrmemw  dee_memrmemw
LOCAL uint16_t *dee_memrmemw(uint16_t const *__restrict haystack, size_t haystack_len,
                             uint16_t const *__restrict needle, size_t needle_len) {
 uint16_t const *candidate; uint16_t marker;
 if unlikely(!needle_len || needle_len > haystack_len)
    return NULL;
 haystack_len -= needle_len-1,marker = *(uint16_t *)needle;
 while ((candidate = memrchrw(haystack,marker,haystack_len)) != NULL) {
  if (MEMEQW(candidate,needle,needle_len))
      return (uint16_t *)candidate;
  if unlikely(candidate == haystack) break;
  haystack_len = candidate-haystack;
 }
 return NULL;
}

#define memmeml  dee_memmeml
LOCAL uint32_t *dee_memmeml(uint32_t const *__restrict haystack, size_t haystack_len,
                            uint32_t const *__restrict needle, size_t needle_len) {
 uint32_t const *candidate; uint32_t marker;
 if unlikely(!needle_len || needle_len > haystack_len)
    return NULL;
 haystack_len -= needle_len-1,marker = *(uint32_t *)needle;
 while ((candidate = memchrl(haystack,marker,haystack_len)) != NULL) {
  if (MEMEQL(candidate,needle,needle_len))
      return (uint32_t *)candidate;
  haystack_len = (haystack+haystack_len)-(candidate+1);
  haystack     = candidate+1;
 }
 return NULL;
}
#define memrmeml  dee_memrmeml
LOCAL uint32_t *dee_memrmeml(uint32_t const *__restrict haystack, size_t haystack_len,
                             uint32_t const *__restrict needle, size_t needle_len) {
 uint32_t const *candidate; uint32_t marker;
 if unlikely(!needle_len || needle_len > haystack_len)
    return NULL;
 haystack_len -= needle_len-1,marker = *(uint32_t *)needle;
 while ((candidate = memrchrl(haystack,marker,haystack_len)) != NULL) {
  if (MEMEQL(candidate,needle,needle_len))
      return (uint32_t *)candidate;
  if unlikely(candidate == haystack) break;
  haystack_len = candidate-haystack;
 }
 return NULL;
}

/* Case-insensitive string functions. */
#ifndef CONFIG_NO_CTYPE
#define memcasechr(p,c,n)    memlowerchr(p,tolower(c),n)
#define memcasechrb(p,c,n)   memlowerchrb(p,(uint8_t)tolower(c),n)
#define memcaserchr(p,c,n)   memlowerrchr(p,tolower(c),n)
#define memcaserchrb(p,c,n)  memlowerrchrb(p,(uint8_t)tolower(c),n)
#else
#define memcasechr(p,c,n)    memlowerchr(p,DeeUni_ToLower(c),n)
#define memcasechrb(p,c,n)   memlowerchrb(p,(uint8_t)DeeUni_ToLower(c),n)
#define memcaserchr(p,c,n)   memlowerrchr(p,DeeUni_ToLower(c),n)
#define memcaserchrb(p,c,n)  memlowerrchrb(p,(uint8_t)DeeUni_ToLower(c),n)
#endif
#define memcasechrw(p,c,n)   memlowerchrw(p,(uint16_t)DeeUni_ToLower(c),n)
#define memcasechrl(p,c,n)   memlowerchrl(p,DeeUni_ToLower(c),n)
#define memcaserchrw(p,c,n)  memlowerrchrw(p,(uint16_t)DeeUni_ToLower(c),n)
#define memcaserchrl(p,c,n)  memlowerrchrl(p,DeeUni_ToLower(c),n)
#define memlowerchr(p,c,n)   memlowerchrb(p,(uint8_t)(c),n)
#define memlowerrchr(p,c,n)  memlowerrchrb(p,(uint8_t)(c),n)
#define memlowerchrb(p,c,n)  ((uint8_t *)dee_memlowerchr(p,c,n))
#define memlowerchrw(p,c,n)  dee_memlowerchrw(p,c,n)
#define memlowerchrl(p,c,n)  dee_memlowerchrl(p,c,n)
#define memlowerrchrb(p,c,n) ((uint8_t *)dee_memlowerrchr(p,c,n))
#define memlowerrchrw(p,c,n) dee_memlowerrchrw(p,c,n)
#define memlowerrchrl(p,c,n) dee_memlowerrchrl(p,c,n)

LOCAL void *dee_memlowerchr(void const *__restrict p, uint8_t c, size_t n) {
 uint8_t *iter = (uint8_t *)p;
 for (; n--; ++iter) {
#ifdef CONFIG_NO_CTYPE
  if ((uint8_t)DeeUni_ToLower(*iter) == c)
       return iter;
#else
  if ((uint8_t)tolower(*iter) == c)
       return iter;
#endif
 }
 return NULL;
}
LOCAL uint16_t *dee_memlowerchrw(uint16_t const *__restrict p, uint16_t c, size_t n) {
 for (; n--; ++p) {
  if ((uint16_t)DeeUni_ToLower(*p) == c)
       return (uint16_t *)p;
 }
 return NULL;
}
LOCAL uint32_t *dee_memlowerchrl(uint32_t const *__restrict p, uint32_t c, size_t n) {
 for (; n--; ++p) {
  if (DeeUni_ToLower(*p) == c)
      return (uint32_t *)p;
 }
 return NULL;
}
LOCAL void *dee_memlowerrchr(void const *__restrict p, uint8_t c, size_t n) {
 uint8_t *iter = (uint8_t *)p+n;
 while (iter-- != (uint8_t *)p) {
#ifdef CONFIG_NO_CTYPE
  if ((uint8_t)DeeUni_ToLower(*iter) == c)
       return iter;
#else
  if ((uint8_t)tolower(*iter) == c)
       return iter;
#endif
 }
 return NULL;
}
LOCAL uint16_t *dee_memlowerrchrw(uint16_t const *__restrict p, uint16_t c, size_t n) {
 uint16_t *iter = (uint16_t *)p+n;
 while (iter-- != (uint16_t *)p) {
  if ((uint16_t)DeeUni_ToLower(*iter) == c)
       return iter;
 }
 return NULL;
}
LOCAL uint32_t *dee_memlowerrchrl(uint32_t const *__restrict p, uint32_t c, size_t n) {
 uint32_t *iter = (uint32_t *)p+n;
 while (iter-- != (uint32_t *)p) {
  if (DeeUni_ToLower(*iter) == c)
      return iter;
 }
 return NULL;
}

/* Use libc functions for case-insensitive UTF-8 string compare when available. */
#if defined(__USE_KOS) && !defined(CONFIG_NO_CTYPE)
#define MEMCASEEQB(a,b,s) (memcasecmp(a,b,s) == 0)
#elif defined(_MSC_VER) && !defined(CONFIG_NO_CTYPE)
#define MEMCASEEQB(a,b,s) (_memicmp(a,b,s) == 0)
#define memcasecmp         _memicmp
#else
#define MEMCASEEQB(a,b,s)  dee_memcaseeqb((uint8_t *)(a),(uint8_t *)(b),s)
LOCAL bool dee_memcaseeqb(uint8_t const *a, uint8_t const *b, size_t s) {
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
#define memcasecmp         dee_memcasecmp
LOCAL int dee_memcasecmp(uint8_t const *a, uint8_t const *b, size_t s) {
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
#endif
#define MEMCASEEQW(a,b,s)  dee_memcaseeqw((uint16_t *)(a),(uint16_t *)(b),s)
#define MEMCASEEQL(a,b,s)  dee_memcaseeql((uint32_t *)(a),(uint32_t *)(b),s)
LOCAL bool dee_memcaseeqw(uint16_t const *a, uint16_t const *b, size_t s) {
 while (s--) {
  if (DeeUni_ToLower(*a) != DeeUni_ToLower(*b))
      return false;
  ++a;
  ++b;
 }
 return true;
}
LOCAL bool dee_memcaseeql(uint32_t const *a, uint32_t const *b, size_t s) {
 while (s--) {
  if (DeeUni_ToLower(*a) != DeeUni_ToLower(*b))
      return false;
  ++a;
  ++b;
 }
 return true;
}

#define memcasememb(haystack,haystack_len,needle,needle_len)  \
     ((uint8_t *)memcasemem(haystack,haystack_len,needle,needle_len))
#define memcasemem  dee_memcasemem
LOCAL void *dee_memcasemem(void const *__restrict haystack, size_t haystack_len,
                           void const *__restrict needle, size_t needle_len) {
 void const *candidate; uint8_t marker;
 if unlikely(!needle_len || needle_len > haystack_len)
    return NULL;
 haystack_len -= needle_len;
#ifdef CONFIG_NO_CTYPE
 marker = (uint8_t)DeeUni_ToLower(*(uint8_t *)needle);
#else
 marker = (uint8_t)tolower(*(uint8_t *)needle);
#endif
 while ((candidate = memlowerchr(haystack,marker,haystack_len)) != NULL) {
  if (MEMCASEEQB(candidate,needle,needle_len))
      return (void *)candidate;
  haystack_len = ((uintptr_t)haystack+haystack_len)-(uintptr_t)candidate;
  haystack     = (void const *)((uintptr_t)candidate+1);
 }
 return NULL;
}
#define memcasermemb(haystack,haystack_len,needle,needle_len)  \
     ((uint8_t *)memcasermem(haystack,haystack_len,needle,needle_len))
#define memcasermem  dee_memcasermem
LOCAL void *dee_memcasermem(void const *__restrict haystack, size_t haystack_len,
                            void const *__restrict needle, size_t needle_len) {
 void const *candidate; uint8_t marker;
 if unlikely(!needle_len || needle_len > haystack_len)
    return NULL;
 haystack_len -= needle_len;
#ifdef CONFIG_NO_CTYPE
 marker = (uint8_t)DeeUni_ToLower(*(uint8_t *)needle);
#else
 marker = (uint8_t)tolower(*(uint8_t *)needle);
#endif
 while ((candidate = memlowerrchr(haystack,marker,haystack_len)) != NULL) {
  if (MEMCASEEQB(candidate,needle,needle_len))
      return (void *)candidate;
  if unlikely(candidate == haystack) break;
  haystack_len = (((uintptr_t)candidate)-1)-(uintptr_t)haystack;
 }
 return NULL;
}

#define memcasememw  dee_memcasememw
LOCAL uint16_t *dee_memcasememw(uint16_t const *__restrict haystack, size_t haystack_len,
                                uint16_t const *__restrict needle, size_t needle_len) {
 uint16_t const *candidate; uint16_t marker;
 if unlikely(!needle_len || needle_len > haystack_len)
    return NULL;
 haystack_len -= needle_len;
 marker = (uint16_t)DeeUni_ToLower(*(uint16_t *)needle);
 while ((candidate = memlowerchrw(haystack,marker,haystack_len)) != NULL) {
  if (MEMCASEEQW(candidate,needle,needle_len))
      return (uint16_t *)candidate;
  haystack_len = (haystack+haystack_len)-candidate;
  haystack     = candidate+1;
 }
 return NULL;
}
#define memcasermemw  dee_memcasermemw
LOCAL uint16_t *dee_memcasermemw(uint16_t const *__restrict haystack, size_t haystack_len,
                                 uint16_t const *__restrict needle, size_t needle_len) {
 uint16_t const *candidate; uint16_t marker;
 if unlikely(!needle_len || needle_len > haystack_len)
    return NULL;
 haystack_len -= needle_len;
 marker = (uint16_t)DeeUni_ToLower(*(uint16_t *)needle);
 while ((candidate = memlowerrchrw(haystack,marker,haystack_len)) != NULL) {
  if (MEMCASEEQW(candidate,needle,needle_len))
      return (uint16_t *)candidate;
  if unlikely(candidate == haystack) break;
  haystack_len = (candidate-1)-haystack;
 }
 return NULL;
}

#define memcasememl  dee_memcasememl
LOCAL uint32_t *dee_memcasememl(uint32_t const *__restrict haystack, size_t haystack_len,
                                uint32_t const *__restrict needle, size_t needle_len) {
 uint32_t const *candidate; uint32_t marker;
 if unlikely(!needle_len || needle_len > haystack_len)
    return NULL;
 haystack_len -= needle_len;
 marker = DeeUni_ToLower(*(uint32_t *)needle);
 while ((candidate = memlowerchrl(haystack,marker,haystack_len)) != NULL) {
  if (MEMCASEEQL(candidate,needle,needle_len))
      return (uint32_t *)candidate;
  haystack_len = (haystack+haystack_len)-candidate;
  haystack     = candidate+1;
 }
 return NULL;
}
#define memcasermeml  dee_memcasermeml
LOCAL uint32_t *dee_memcasermeml(uint32_t const *__restrict haystack, size_t haystack_len,
                                 uint32_t const *__restrict needle, size_t needle_len) {
 uint32_t const *candidate; uint32_t marker;
 if unlikely(!needle_len || needle_len > haystack_len)
    return NULL;
 haystack_len -= needle_len;
 marker = DeeUni_ToLower(*(uint32_t *)needle);
 while ((candidate = memlowerrchrl(haystack,marker,haystack_len)) != NULL) {
  if (MEMCASEEQL(candidate,needle,needle_len))
      return (uint32_t *)candidate;
  if unlikely(candidate == haystack) break;
  haystack_len = (candidate-1)-haystack;
 }
 return NULL;
}

#if defined(__USE_KOS) && !defined(CONFIG_NO_CTYPE)
#define STRCASEEQ(a,b) (strcasecmp(a,b) == 0)
#elif defined(_MSC_VER) && !defined(CONFIG_NO_CTYPE)
#define STRCASEEQ(a,b) (_stricmp(a,b) == 0)
#else
#define STRCASEEQ(a,b) dee_strcaseeq(a,b)
LOCAL bool dee_strcaseeq(char const *a, char const *b) {
 while (*a && DeeUni_ToLower(*a) == DeeUni_ToLower(*b));
 return !*b;
}
#endif


/* As found here: https://en.wikipedia.org/wiki/Levenshtein_distance */
#define DEFINE_FUZZY_COMPARE_FUNCTION(name,T,transform) \
PRIVATE dssize_t DCALL \
name(T const *__restrict a, size_t alen, \
     T const *__restrict b, size_t blen) { \
 size_t *v0,*v1,i,j,cost,temp; bool isheap; \
 if unlikely(!alen) return blen; \
 if unlikely(!blen) return alen; \
 if (blen > (128+1)*sizeof(size_t)) { \
  v0 = (size_t *)Dee_Malloc((blen+1)*sizeof(size_t)); \
  if unlikely(!v0) return -1; \
  v1 = (size_t *)Dee_Malloc((blen+1)*sizeof(size_t)); \
  if unlikely(!v1) { Dee_Free(v0); return -1; } \
  isheap = true; \
 } else { \
  v0 = (size_t *)alloca((blen+1)*sizeof(size_t)); \
  v1 = (size_t *)alloca((blen+1)*sizeof(size_t)); \
  isheap = false; \
 } \
 for (i = 0; i < blen; ++i) v0[i] = i; \
 for (i = 0; i < alen; ++i) { \
  v1[0] = i+1; \
  for (j = 0; j < blen; j++) { \
   cost  = (transform(a[i]) == transform(b[j])) ? 0u : 1u; \
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
 if (isheap) Dee_Free(v0),Dee_Free(v1); \
 if (temp > SSIZE_MAX) temp = SSIZE_MAX; \
 return temp; \
}

DEFINE_FUZZY_COMPARE_FUNCTION(fuzzy_compareb,uint8_t,)
DEFINE_FUZZY_COMPARE_FUNCTION(fuzzy_comparew,uint16_t,)
DEFINE_FUZZY_COMPARE_FUNCTION(fuzzy_comparel,uint32_t,)
DEFINE_FUZZY_COMPARE_FUNCTION(fuzzy_casecompareb,uint8_t,DeeUni_ToLower)
DEFINE_FUZZY_COMPARE_FUNCTION(fuzzy_casecomparew,uint16_t,DeeUni_ToLower)
DEFINE_FUZZY_COMPARE_FUNCTION(fuzzy_casecomparel,uint32_t,DeeUni_ToLower)
#undef DEFINE_FUZZY_COMPARE_FUNCTION

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
DEFINE_VERSION_COMPARE_FUNCTION(dee_strcaseverscmpb,uint8_t,int8_t,DeeUni_ToLower,DEE_PRIVATE_IF_TRUE)
DEFINE_VERSION_COMPARE_FUNCTION(dee_strcaseverscmpw,uint16_t,int16_t,DeeUni_ToLower,DEE_PRIVATE_IF_TRUE)
DEFINE_VERSION_COMPARE_FUNCTION(dee_strcaseverscmpl,uint32_t,int32_t,DeeUni_ToLower,DEE_PRIVATE_IF_TRUE)
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
DEFINE_FIND_MATCH_FUNCTION(find_matchb,uint8_t,memmemb,MEMEQB)
DEFINE_FIND_MATCH_FUNCTION(find_matchw,uint16_t,memmemw,MEMEQW)
DEFINE_FIND_MATCH_FUNCTION(find_matchl,uint32_t,memmeml,MEMEQL)
DEFINE_FIND_MATCH_FUNCTION(find_casematchb,uint8_t,memcasememb,MEMCASEEQB)
DEFINE_FIND_MATCH_FUNCTION(find_casematchw,uint16_t,memcasememw,MEMCASEEQW)
DEFINE_FIND_MATCH_FUNCTION(find_casematchl,uint32_t,memcasememl,MEMCASEEQL)
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
DEFINE_RFIND_MATCH_FUNCTION(rfind_matchb,uint8_t,memrmemb,MEMEQB)
DEFINE_RFIND_MATCH_FUNCTION(rfind_matchw,uint16_t,memrmemw,MEMEQW)
DEFINE_RFIND_MATCH_FUNCTION(rfind_matchl,uint32_t,memrmeml,MEMEQL)
DEFINE_RFIND_MATCH_FUNCTION(rfind_casematchb,uint8_t,memcasermemb,MEMCASEEQB)
DEFINE_RFIND_MATCH_FUNCTION(rfind_casematchw,uint16_t,memcasermemw,MEMCASEEQW)
DEFINE_RFIND_MATCH_FUNCTION(rfind_casematchl,uint32_t,memcasermeml,MEMCASEEQL)
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
#define wildcompare wildcasecompareb
#include "wildcompare.c.inl"
#define NOCASE
#define T           uint16_t
#define wildcompare wildcasecomparew
#include "wildcompare.c.inl"
#define NOCASE
#define Treturn     int64_t
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
