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
#ifndef GUARD_DEEMON_OBJECTS_STRING_FUNCTIONS_C
#define GUARD_DEEMON_OBJECTS_STRING_FUNCTIONS_C 1
#define _KOS_SOURCE 1 /* memchrb/w/l, memrchrb/w/l, etc... */
#define _GNU_SOURCE 1 /* memrchr */

#include <deemon/api.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/none.h>
#include <deemon/format.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/tuple.h>
#include <deemon/int.h>
#include <deemon/util/string.h>
#include <hybrid/minmax.h>

#include <stddef.h>
#include <string.h>
#ifndef CONFIG_NO_CTYPE
#include <ctype.h>
#endif

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

#ifndef SSIZE_MAX
#include <hybrid/limitcore.h>
#define SSIZE_MAX  __SSIZE_MAX__
#endif

DECL_BEGIN

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
#define memcpyw(dst,src,n)             memcpy(dst,src,(n)*2)
#define memcpyl(dst,src,n)             memcpy(dst,src,(n)*4)
#define memsetb(p,c,n)     ((uint8_t *)memset(p,c,n))
#define memchrb(p,c,n)     ((uint8_t *)memchr(p,c,n))
#define memrchrb(p,c,n)    ((uint8_t *)memrchr(p,c,n))
#define memsetw                    dee_memsetw
#define memsetl                    dee_memsetl
#define memchrw                    dee_memchrw
#define memchrl                    dee_memchrl
#define memrchrw                   dee_memrchrw
#define memrchrl                   dee_memrchrl
LOCAL void dee_memsetw(uint16_t *__restrict p, uint16_t c, size_t n) {
 while (n--) *p++ = c;
}
LOCAL void dee_memsetl(uint32_t *__restrict p, uint32_t c, size_t n) {
 while (n--) *p++ = c;
}
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
#else
#define MEMCASEEQB(a,b,s)  dee_memcaseeqb((uint8_t *)(a),(uint8_t *)(b),s)
LOCAL bool dee_memcaseeqb(uint8_t const *a, uint8_t const *b, size_t s) {
 while (s--) {
  if (DeeUni_ToLower(*a) != DeeUni_ToLower(*b))
      return false;
  ++a;
  ++b;
 }
 return true;
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



typedef DeeStringObject String;


INTERN dssize_t DCALL
DeeString_Count(DeeObject *__restrict self,
                DeeObject *__restrict other,
                size_t begin, size_t end) {
 dssize_t result = 0; int encoding;
 uint8_t *my_str,*other_str;
 size_t search_size,find_size;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(other,&DeeString_Type);
 encoding = DeeString_UtfEnc(self);
 my_str   = (uint8_t *)DeeString_UtfStr(self);
 if (end > ENCODING_SIZE(my_str))
     end = ENCODING_SIZE(my_str);
 if (end <= begin) goto done; /* Empty search area. */
 search_size = (size_t)(end-begin);
 /* Load the other string with our own encoding. */
 other_str = (uint8_t *)DeeString_AsEncoding(other,encoding);
 if unlikely(!other_str) return -1;
 find_size = ENCODING_SIZE(other_str);
 if (!find_size || find_size > search_size)
     goto done; /* Requested find-string is larger than search area, or find-string is empty. */
 ASSERT(search_size);
 ASSERT(find_size);
 /* Actually perform the search for the given string. */
 SWITCH_ENC_SIZE(encoding) {
 default:
  while (search_size >= find_size) {
   if (MEMEQB(my_str,(uint8_t *)other_str,find_size))
        ++result,my_str += find_size,search_size -= find_size;
   else ++my_str,--search_size;
  }
  break;             
 CASE_ENC_2BYTE:
  while (search_size >= find_size*2) {
   if (MEMEQW((uint16_t *)my_str,(uint16_t *)other_str,find_size))
      ++result,my_str += find_size*2,search_size -= 2*find_size;
   else my_str += 2,search_size -= 2;
  }
  break;             
 CASE_ENC_4BYTE:
  while (search_size >= find_size*4) {
   if (MEMEQL((uint32_t *)my_str,(uint32_t *)other_str,find_size))
      ++result,my_str += find_size*4,search_size -= find_size*4;
   else my_str += 4,search_size -= 4;
  }
  break;             
 }
 ASSERT(result != -1);
done:
 return result;
}
INTERN dssize_t DCALL
DeeString_CaseCount(DeeObject *__restrict self,
                    DeeObject *__restrict other,
                    size_t begin, size_t end) {
 dssize_t result = 0; int encoding;
 uint8_t *my_str,*other_str;
 size_t search_size,find_size;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(other,&DeeString_Type);
 encoding = DeeString_UtfEnc(self);
 my_str   = (uint8_t *)DeeString_UtfStr(self);
 if (end > ENCODING_SIZE(my_str))
     end = ENCODING_SIZE(my_str);
 if (end <= begin) goto done; /* Empty search area. */
 search_size = (size_t)(end-begin);
 /* Load the other string with our own encoding. */
 other_str = (uint8_t *)DeeString_AsEncoding(other,encoding);
 if unlikely(!other_str) return -1;
 find_size = ENCODING_SIZE(other_str);
 if (!find_size || find_size > search_size)
     goto done; /* Requested find-string is larger than search area, or find-string is empty. */
 ASSERT(search_size);
 ASSERT(find_size);
 /* Actually perform the search for the given string. */
 SWITCH_ENC_SIZE(encoding) {
 default:
  while (search_size >= find_size) {
   if (MEMCASEEQB(my_str,(uint8_t *)other_str,find_size))
        ++result,my_str += find_size,search_size -= find_size;
   else ++my_str,--search_size;
  }
  break;             
 CASE_ENC_2BYTE:
  while (search_size >= find_size*2) {
   if (MEMCASEEQW((uint16_t *)my_str,(uint16_t *)other_str,find_size))
      ++result,my_str += find_size*2,search_size -= 2*find_size;
   else my_str += 2,search_size -= 2;
  }
  break;             
 CASE_ENC_4BYTE:
  while (search_size >= find_size*4) {
   if (MEMCASEEQL((uint32_t *)my_str,(uint32_t *)other_str,find_size))
      ++result,my_str += find_size*4,search_size -= find_size*4;
   else my_str += 4,search_size -= 4;
  }
  break;             
 }
 ASSERT(result >= 0);
done:
 return result;
}
INTERN int DCALL
DeeString_Contains(DeeObject *__restrict self,
                   DeeObject *__restrict other,
                   size_t begin, size_t end) {
 int result; int encoding;
 uint8_t *my_str,*other_str;
 size_t search_size,find_size;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(other,&DeeString_Type);
 encoding = DeeString_UtfEnc(self);
 my_str   = (uint8_t *)DeeString_UtfStr(self);
 if (end > ENCODING_SIZE(my_str))
     end = ENCODING_SIZE(my_str);
 if (end <= begin) return 0; /* Empty search area. */
 search_size = (size_t)(end-begin);
 /* Load the other string with our own encoding. */
 other_str = (uint8_t *)DeeString_AsEncoding(other,encoding);
 if unlikely(!other_str) return -1;
 find_size = ENCODING_SIZE(other_str);
 if (!find_size || find_size > search_size)
     return 0; /* Requested find-string is larger than search area, or find-string is empty. */
 ASSERT(search_size);
 ASSERT(find_size);
 /* Actually perform the search for the given string. */
 SWITCH_ENC_SIZE(encoding) {
 default:
  result = memmemb((uint8_t *)my_str,search_size,(uint8_t *)other_str,find_size) != NULL;
  break;             
 CASE_ENC_2BYTE:
  result = memmemw((uint16_t *)my_str,search_size,(uint16_t *)other_str,find_size) != NULL;
  break;             
 CASE_ENC_4BYTE:
  result = memmeml((uint32_t *)my_str,search_size,(uint32_t *)other_str,find_size) != NULL;
  break;             
 }
 return result;
}
INTERN int DCALL
DeeString_CaseContains(DeeObject *__restrict self,
                       DeeObject *__restrict other,
                       size_t begin, size_t end) {
 int result; int encoding;
 uint8_t *my_str,*other_str;
 size_t search_size,find_size;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(other,&DeeString_Type);
 encoding = DeeString_UtfEnc(self);
 my_str   = (uint8_t *)DeeString_UtfStr(self);
 if (end > ENCODING_SIZE(my_str))
     end = ENCODING_SIZE(my_str);
 if (end <= begin) return 0; /* Empty search area. */
 search_size = (size_t)(end-begin);
 /* Load the other string with our own encoding. */
 other_str = (uint8_t *)DeeString_AsEncoding(other,encoding);
 if unlikely(!other_str) return -1;
 find_size = ENCODING_SIZE(other_str);
 if (!find_size || find_size > search_size)
     return 0; /* Requested find-string is larger than search area, or find-string is empty. */
 ASSERT(search_size);
 ASSERT(find_size);
 /* Actually perform the search for the given string. */
 SWITCH_ENC_SIZE(encoding) {
 default:
  result = memcasememb((uint8_t *)my_str,search_size,(uint8_t *)other_str,find_size) != NULL;
  break;             
 CASE_ENC_2BYTE:
  result = memcasememw((uint16_t *)my_str,search_size,(uint16_t *)other_str,find_size) != NULL;
  break;             
 CASE_ENC_4BYTE:
  result = memcasememl((uint32_t *)my_str,search_size,(uint32_t *)other_str,find_size) != NULL;
  break;             
 }
 return result;
}
INTERN dssize_t DCALL
DeeString_Find(DeeObject *__restrict self,
               DeeObject *__restrict other,
               size_t begin, size_t end) {
 dssize_t result = -1; int encoding;
 void *my_str,*other_str,*find_pointer;
 size_t search_size,find_size;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(other,&DeeString_Type);
 encoding = DeeString_UtfEnc(self);
 my_str   = DeeString_UtfStr(self);
 if (end > ENCODING_SIZE(my_str))
     end = ENCODING_SIZE(my_str);
 if (end <= begin) goto done; /* Empty search area. */
 search_size = (size_t)(end-begin);
 /* Load the other string with our own encoding. */
 other_str = DeeString_AsEncoding(other,encoding);
 if unlikely(!other_str) return -2;
 find_size = ENCODING_SIZE(other_str);
 if (!find_size || find_size > search_size)
     goto done; /* Requested find-string is larger than search area, or find-string is empty. */
 ASSERT(search_size);
 ASSERT(find_size);
 /* Actually perform the search for the given string. */
 SWITCH_ENC_SIZE(encoding) {
 default:
  find_pointer = memmemb((uint8_t *)my_str+begin,search_size,
                         (uint8_t *)other_str,find_size);
  if (find_pointer) result = (uint8_t *)find_pointer-(uint8_t *)my_str;
  break;             
 CASE_ENC_2BYTE:
  find_pointer = memmemw((uint16_t *)my_str+begin,search_size,
                         (uint16_t *)other_str,find_size);
  if (find_pointer) result = (uint16_t *)find_pointer-(uint16_t *)my_str;
  break;             
 CASE_ENC_4BYTE:
  find_pointer = memmeml((uint32_t *)my_str+begin,search_size,
                         (uint32_t *)other_str,find_size);
  if (find_pointer) result = (uint32_t *)find_pointer-(uint32_t *)my_str;
  break;             
 }
done:
 return result;
}
INTERN dssize_t DCALL
DeeString_RFind(DeeObject *__restrict self,
                DeeObject *__restrict other,
                size_t begin, size_t end) {
 dssize_t result = -1; int encoding;
 void *my_str,*other_str,*find_pointer;
 size_t search_size,find_size;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(other,&DeeString_Type);
 encoding = DeeString_UtfEnc(self);
 my_str   = DeeString_UtfStr(self);
 if (end > ENCODING_SIZE(my_str))
     end = ENCODING_SIZE(my_str);
 if (end <= begin) goto done; /* Empty search area. */
 search_size = (size_t)(end-begin);
 /* Load the other string with our own encoding. */
 other_str = DeeString_AsEncoding(other,encoding);
 if unlikely(!other_str) return -2;
 find_size = ENCODING_SIZE(other_str);
 if (!find_size || find_size > search_size)
     goto done; /* Requested find-string is larger than search area, or find-string is empty. */
 ASSERT(search_size);
 ASSERT(find_size);
 /* Actually perform the search for the given string. */
 SWITCH_ENC_SIZE(encoding) {
 default:
  find_pointer = memrmemb((uint8_t *)my_str+begin,search_size,
                          (uint8_t *)other_str,find_size);
  if (find_pointer) result = (uint8_t *)find_pointer-(uint8_t *)my_str;
  break;             
 CASE_ENC_2BYTE:
  find_pointer = memrmemw((uint16_t *)my_str+begin,search_size,
                          (uint16_t *)other_str,find_size);
  if (find_pointer) result = (uint16_t *)find_pointer-(uint16_t *)my_str;
  break;             
 CASE_ENC_4BYTE:
  find_pointer = memrmeml((uint32_t *)my_str+begin,search_size,
                          (uint32_t *)other_str,find_size);
  if (find_pointer) result = (uint32_t *)find_pointer-(uint32_t *)my_str;
  break;             
 }
done:
 return result;
}
INTERN dssize_t DCALL
DeeString_CaseFind(DeeObject *__restrict self,
                   DeeObject *__restrict other,
                   size_t begin, size_t end) {
 dssize_t result = -1; int encoding;
 void *my_str,*other_str,*find_pointer;
 size_t search_size,find_size;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(other,&DeeString_Type);
 encoding = DeeString_UtfEnc(self);
 my_str   = DeeString_UtfStr(self);
 if (end > ENCODING_SIZE(my_str))
     end = ENCODING_SIZE(my_str);
 if (end <= begin) goto done; /* Empty search area. */
 search_size = (size_t)(end-begin);
 /* Load the other string with our own encoding. */
 other_str = DeeString_AsEncoding(other,encoding);
 if unlikely(!other_str) return -2;
 find_size = ENCODING_SIZE(other_str);
 if (!find_size || find_size > search_size)
     goto done; /* Requested find-string is larger than search area, or find-string is empty. */
 ASSERT(search_size);
 ASSERT(find_size);
 /* Actually perform the search for the given string. */
 SWITCH_ENC_SIZE(encoding) {
 default:
  find_pointer = memcasememb((uint8_t *)my_str+begin,search_size,
                             (uint8_t *)other_str,find_size);
  if (find_pointer) result = (uint8_t *)find_pointer-(uint8_t *)my_str;
  break;             
 CASE_ENC_2BYTE:
  find_pointer = memcasememw((uint16_t *)my_str+begin,search_size,
                             (uint16_t *)other_str,find_size);
  if (find_pointer) result = (uint16_t *)find_pointer-(uint16_t *)my_str;
  break;             
 CASE_ENC_4BYTE:
  find_pointer = memcasememl((uint32_t *)my_str+begin,search_size,
                             (uint32_t *)other_str,find_size);
  if (find_pointer) result = (uint32_t *)find_pointer-(uint32_t *)my_str;
  break;             
 }
done:
 return result;
}
INTERN dssize_t DCALL
DeeString_CaseRFind(DeeObject *__restrict self,
                    DeeObject *__restrict other,
                    size_t begin, size_t end) {
 dssize_t result = -1; int encoding;
 void *my_str,*other_str,*find_pointer;
 size_t search_size,find_size;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(other,&DeeString_Type);
 encoding = DeeString_UtfEnc(self);
 my_str   = DeeString_UtfStr(self);
 if (end > ENCODING_SIZE(my_str))
     end = ENCODING_SIZE(my_str);
 if (end <= begin) goto done; /* Empty search area. */
 search_size = (size_t)(end-begin);
 /* Load the other string with our own encoding. */
 other_str = DeeString_AsEncoding(other,encoding);
 if unlikely(!other_str) return -2;
 find_size = ENCODING_SIZE(other_str);
 if (!find_size || find_size > search_size)
     goto done; /* Requested find-string is larger than search area, or find-string is empty. */
 ASSERT(search_size);
 ASSERT(find_size);
 /* Actually perform the search for the given string. */
 SWITCH_ENC_SIZE(encoding) {
 default:
  find_pointer = memcasermemb((uint8_t *)my_str+begin,search_size,
                              (uint8_t *)other_str,find_size);
  if (find_pointer) result = (uint8_t *)find_pointer-(uint8_t *)my_str;
  break;             
 CASE_ENC_2BYTE:
  find_pointer = memcasermemw((uint16_t *)my_str+begin,search_size,
                              (uint16_t *)other_str,find_size);
  if (find_pointer) result = (uint16_t *)find_pointer-(uint16_t *)my_str;
  break;             
 CASE_ENC_4BYTE:
  find_pointer = memcasermeml((uint32_t *)my_str+begin,search_size,
                              (uint32_t *)other_str,find_size);
  if (find_pointer) result = (uint32_t *)find_pointer-(uint32_t *)my_str;
  break;             
 }
done:
 return result;
}
INTERN DREF DeeObject *DCALL
DeeString_StripSpc(DeeObject *__restrict self) {
 uint8_t *str,*new_str; int encoding; size_t len,new_len;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 encoding = DeeString_UtfEnc(self);
 str = new_str = (uint8_t *)DeeString_UtfStr(self);
 len = new_len = ENCODING_SIZE(str);
 SWITCH_ENC_SIZE(encoding) {
 default:
  while (new_len && DeeUni_IsSpace(((uint8_t *)new_str)[0])) ++new_str,--new_len;
  while (new_len && DeeUni_IsSpace(((uint8_t *)new_str)[new_len-1])) --new_len;
  if (new_len == len && new_str == str) goto return_self;
  return DeeString_NewSized((char const *)new_str,new_len);
 CASE_ENC_2BYTE:
  while (new_len && DeeUni_IsSpace(((uint16_t *)new_str)[0])) new_str += 2,--new_len;
  while (new_len && DeeUni_IsSpace(((uint16_t *)new_str)[new_len-1])) --new_len;
  break;
 CASE_ENC_4BYTE:
  while (new_len && DeeUni_IsSpace(((uint32_t *)new_str)[0])) new_str += 4,--new_len;
  while (new_len && DeeUni_IsSpace(((uint32_t *)new_str)[new_len-1])) --new_len;
  break;
 }
 if (new_len == len && new_str == str) goto return_self;
 return DeeString_NewSizedWithEncoding(new_str,new_len,encoding);
return_self:
 return_reference_(self);
}
INTERN DREF DeeObject *DCALL
DeeString_LStripSpc(DeeObject *__restrict self) {
 uint8_t *str,*new_str; int encoding; size_t len,new_len;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 encoding = DeeString_UtfEnc(self);
 str = new_str = (uint8_t *)DeeString_UtfStr(self);
 len = new_len = ENCODING_SIZE(str);
 switch (encoding) {
 default:
  while (new_len && DeeUni_IsSpace(((uint8_t *)new_str)[0])) ++new_str,--new_len;
  if (new_len == len) goto return_self;
  return DeeString_NewSized((char const *)new_str,new_len);
 CASE_ENC_2BYTE:
  while (new_len && DeeUni_IsSpace(((uint16_t *)new_str)[0])) new_str += 2,--new_len;
  break;
 CASE_ENC_4BYTE:
  while (new_len && DeeUni_IsSpace(((uint32_t *)new_str)[0])) new_str += 4,--new_len;
  break;
 }
 if (new_len == len) goto return_self;
 return DeeString_NewSizedWithEncoding(new_str,new_len,encoding);
return_self:
 return_reference_(self);
}
INTERN DREF DeeObject *DCALL
DeeString_RStripSpc(DeeObject *__restrict self) {
 uint8_t *str; int encoding; size_t len,new_len;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 encoding = DeeString_UtfEnc(self);
 str = (uint8_t *)DeeString_UtfStr(self);
 len = new_len = ENCODING_SIZE(str);
 SWITCH_ENC_SIZE(encoding) {
 default:
  while (new_len && DeeUni_IsSpace(((uint8_t *)str)[new_len-1])) --new_len;
  if (new_len == len) goto return_self;
  return DeeString_NewSized((char const *)str,new_len);
 CASE_ENC_2BYTE:
  while (new_len && DeeUni_IsSpace(((uint16_t *)str)[new_len-1])) --new_len;
  break;
 CASE_ENC_4BYTE:
  while (new_len && DeeUni_IsSpace(((uint32_t *)str)[new_len-1])) --new_len;
  break;
 }
 if (new_len == len) goto return_self;
 return DeeString_NewSizedWithEncoding(str,new_len,encoding);
return_self:
 return_reference_(self);
}

INTERN DREF DeeObject *DCALL
DeeString_StripMask(DeeObject *__restrict self,
                    DeeObject *__restrict mask) {
 uint8_t *str,*new_str; void *mask_str;
 int encoding; size_t len,new_len,mask_len;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(mask,&DeeString_Type);
 encoding = DeeString_UtfEnc(self);
 str = new_str = (uint8_t *)DeeString_UtfStr(self);
 len = new_len = ENCODING_SIZE(str);
 mask_str = DeeString_AsEncoding(mask,encoding);
 if unlikely(!mask_str) return NULL;
 mask_len = ENCODING_SIZE(mask_str);
 SWITCH_ENC_SIZE(encoding) {
 default:
  while (new_len && memchrb((uint8_t *)mask_str,((uint8_t *)new_str)[0],mask_len)) ++new_str,--new_len;
  while (new_len && memchrb((uint8_t *)mask_str,((uint8_t *)new_str)[new_len-1],mask_len)) --new_len;
  if (new_len == len && new_str == str) goto return_self;
  return DeeString_NewSized((char const *)new_str,new_len);
 CASE_ENC_2BYTE:
  while (new_len && memchrw((uint16_t *)mask_str,((uint16_t *)new_str)[0],mask_len)) new_str += 2,--new_len;
  while (new_len && memchrw((uint16_t *)mask_str,((uint16_t *)new_str)[new_len-1],mask_len)) --new_len;
  break;
 CASE_ENC_4BYTE:
  while (new_len && memchrl((uint32_t *)mask_str,((uint32_t *)new_str)[0],mask_len)) new_str += 4,--new_len;
  while (new_len && memchrl((uint32_t *)mask_str,((uint32_t *)new_str)[new_len-1],mask_len)) --new_len;
  break;
 }
 if (new_len == len && new_str == str) goto return_self;
 return DeeString_NewSizedWithEncoding(new_str,new_len,encoding);
return_self:
 return_reference_(self);
}
INTERN DREF DeeObject *DCALL
DeeString_LStripMask(DeeObject *__restrict self, DeeObject *__restrict mask) {
 uint8_t *str,*new_str; void *mask_str;
 int encoding; size_t len,new_len,mask_len;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(mask,&DeeString_Type);
 encoding = DeeString_UtfEnc(self);
 str = new_str = (uint8_t *)DeeString_UtfStr(self);
 len = new_len = ENCODING_SIZE(str);
 mask_str = DeeString_AsEncoding(mask,encoding);
 if unlikely(!mask_str) return NULL;
 mask_len = ENCODING_SIZE(mask_str);
 SWITCH_ENC_SIZE(encoding) {
 default:
  while (new_len && memchrb((uint8_t *)mask_str,((uint8_t *)new_str)[0],mask_len)) ++new_str,--new_len;
  if (new_len == len) goto return_self;
  return DeeString_NewSized((char const *)new_str,new_len);
 CASE_ENC_2BYTE:
  while (new_len && memchrw((uint16_t *)mask_str,((uint16_t *)new_str)[0],mask_len)) new_str += 2,--new_len;
  break;
 CASE_ENC_4BYTE:
  while (new_len && memchrl((uint32_t *)mask_str,((uint32_t *)new_str)[0],mask_len)) new_str += 4,--new_len;
  break;
 }
 if (new_len == len) goto return_self;
 return DeeString_NewSizedWithEncoding(new_str,new_len,encoding);
return_self:
 return_reference_(self);
}
INTERN DREF DeeObject *DCALL
DeeString_RStripMask(DeeObject *__restrict self, DeeObject *__restrict mask) {
 uint8_t *str; void *mask_str;
 int encoding; size_t len,new_len,mask_len;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(mask,&DeeString_Type);
 encoding = DeeString_UtfEnc(self);
 str = (uint8_t *)DeeString_UtfStr(self);
 len = new_len = ENCODING_SIZE(str);
 mask_str = DeeString_AsEncoding(mask,encoding);
 if unlikely(!mask_str) return NULL;
 mask_len = ENCODING_SIZE(mask_str);
 SWITCH_ENC_SIZE(encoding) {
 default:
  while (new_len && memchrb((uint8_t *)mask_str,((uint8_t *)str)[new_len-1],mask_len)) --new_len;
  if (new_len == len) goto return_self;
  return DeeString_NewSized((char const *)str,new_len);
 CASE_ENC_2BYTE:
  while (new_len && memchrw((uint16_t *)mask_str,((uint16_t *)str)[new_len-1],mask_len)) --new_len;
  break;
 CASE_ENC_4BYTE:
  while (new_len && memchrl((uint32_t *)mask_str,((uint32_t *)str)[new_len-1],mask_len)) --new_len;
  break;
 }
 if (new_len == len) goto return_self;
 return DeeString_NewSizedWithEncoding(str,new_len,encoding);
return_self:
 return_reference_(self);
}
INTERN DREF DeeObject *DCALL
DeeString_Reversed(DeeObject *__restrict self,
                   size_t begin, size_t end) {
 DREF DeeObject *result; int encoding;
 uint8_t *my_str,*dst; size_t flip_size;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 encoding = DeeString_UtfEnc(self);
 my_str   = (uint8_t *)DeeString_UtfStr(self);
 if (end > ENCODING_SIZE(my_str))
     end = ENCODING_SIZE(my_str);
 if (end <= begin) return_empty_string; /* Empty string area. */
 flip_size = (size_t)(end-begin);
 ASSERT(flip_size);
#ifdef CONFIG_USE_NEW_STRING_API
 /* Actually perform the search for the given string. */
 switch (encoding) {
 default:
  result = DeeString_NewBuffer(flip_size);
  if unlikely(!result) goto err;
  dst = (uint8_t *)DeeString_STR(result);
  do {
   *(uint8_t *)dst = ((uint8_t *)my_str)[flip_size-1];
   dst += 1;
  } while (--flip_size);
  break;             
 {
  uint16_t *buf;
 case STRING_WIDTH_2BYTE:
  buf = DeeString_NewUtf16Buffer(flip_size);
  if unlikely(!buf) goto err;
  dst = (uint8_t *)buf;
  do {
   *(uint16_t *)dst = ((uint16_t *)my_str)[flip_size-1];
   dst += 2;
  } while (--flip_size);
  result = DeeString_PackUtf16Buffer(buf,DEE_STRING_CODEC_FIGNORE);
 } break;             
 {
  uint32_t *buf;
 case STRING_WIDTH_4BYTE:
  buf = DeeString_NewUtf32Buffer(flip_size);
  if unlikely(!buf) goto err;
  dst = (uint8_t *)buf;
  do {
   *(uint32_t *)dst = ((uint32_t *)my_str)[flip_size-1];
   dst += 4;
  } while (--flip_size);
  result = DeeString_PackUtf32Buffer(buf,DEE_STRING_CODEC_FIGNORE);
 } break;             
 {
  dwchar_t *buf;
 case STRING_WIDTH_WCHAR:
  buf = DeeString_NewWideBuffer(flip_size);
  if unlikely(!buf) goto err;
  dst = (uint8_t *)buf;
  do {
   *(dwchar_t *)dst = ((dwchar_t *)my_str)[flip_size-1];
   dst += sizeof(dwchar_t);
  } while (--flip_size);
  result = DeeString_PackWideBuffer(buf,DEE_STRING_CODEC_FIGNORE);
 } break;             
 }
 return result;
err:
 return NULL;
#else
 result = DeeString_NewEncodingBuffer(flip_size,encoding,
                                     (void **)&dst);
 if unlikely(!result) goto done;
 /* Actually perform the search for the given string. */
 SWITCH_ENC_SIZE(encoding) {
 default:
  do {
   *(uint8_t *)dst = ((uint8_t *)my_str)[flip_size-1];
   dst += 1;
  } while (--flip_size);
  break;             
 CASE_ENC_2BYTE:
  do {
   *(uint16_t *)dst = ((uint16_t *)my_str)[flip_size-1];
   dst += 2;
  } while (--flip_size);
  break;             
 CASE_ENC_4BYTE:
  do {
   *(uint32_t *)dst = ((uint32_t *)my_str)[flip_size-1];
   dst += 4;
  } while (--flip_size);
  break;             
 }
 result = DeeString_SetEncodingBuffer(result);
done:
 return result;
#endif
}
INTERN DREF DeeObject *DCALL
DeeString_ExpandTabs(DeeObject *__restrict self, size_t tab_width) {
 DREF DeeObject *result; char *iter,*end,*flush_start;
 struct string_printer printer = STRING_PRINTER_INIT;
 size_t line_inset = 0;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 end = (iter = flush_start = DeeString_STR(self))+DeeString_SIZE(self);
 for (; iter != end; ++iter) {
  char *buffer;
  char ch = *iter;
  if (ch != '\t') {
   ++line_inset;
   if (ch == '\r' || ch == '\n')
       line_inset = 0; /* Reset insets at line starts. */
   continue;
  }
  if (string_printer_print(&printer,flush_start,
                          (size_t)(iter-flush_start)) < 0)
      goto err;
  /* Replace with white-space. */
  if likely(tab_width) {
   line_inset = tab_width - (line_inset % tab_width);
   buffer = string_printer_alloc(&printer,line_inset);
   if unlikely(!buffer) goto err;
   memset(buffer,' ',line_inset);
   line_inset = 0;
  }
  flush_start = iter+1;
 }
 if (!printer.sp_length) {
  ASSERT(!printer.sp_string);
  return_reference_(self);
 }
 if (string_printer_print(&printer,flush_start,
                         (size_t)(iter-flush_start)) < 0)
     goto err;
 result = string_printer_pack(&printer);
 if unlikely(!result) goto err;
 return result;
err:
 string_printer_fini(&printer);
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeString_UnifyLines(DeeObject *__restrict self, DeeObject *__restrict replacement) {
 DREF DeeObject *result; char *iter,*end,*flush_start;
 struct string_printer printer = STRING_PRINTER_INIT;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(replacement,&DeeString_Type);
 end = (iter = flush_start = DeeString_STR(self))+DeeString_SIZE(self);
 for (; iter != end; ++iter) {
  if (*iter != '\r') continue;
  if (string_printer_print(&printer,flush_start,
                          (size_t)(iter-flush_start)) < 0 ||
      string_printer_print(&printer,
                           DeeString_STR(replacement),
                           DeeString_SIZE(replacement)))
      goto err;
  if (iter[1] == '\n') ++iter;
  flush_start = iter+1;
 }
 if (!printer.sp_length) {
  ASSERT(!printer.sp_string);
  return_reference_(self);
 }
 if (string_printer_print(&printer,flush_start,
                         (size_t)(iter-flush_start)) < 0)
     goto err;
 result = string_printer_pack(&printer);
 if unlikely(!result) goto err;
 return result;
err:
 string_printer_fini(&printer);
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeString_UnifyLinesLf(DeeObject *__restrict self) {
 DREF DeeObject *result; char *iter,*end,*flush_start;
 struct string_printer printer = STRING_PRINTER_INIT;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 end = (iter = flush_start = DeeString_STR(self))+DeeString_SIZE(self);
 for (; iter != end; ++iter) {
  if (*iter != '\r') continue;
  if (string_printer_print(&printer,flush_start,
                          (size_t)(iter-flush_start)) < 0 ||
      string_printer_putc(&printer,'\n'))
      goto err;
  if (iter[1] == '\n') ++iter;
  flush_start = iter+1;
 }
 if (!printer.sp_length) {
  ASSERT(!printer.sp_string);
  return_reference_(self);
 }
 if (string_printer_print(&printer,flush_start,
                         (size_t)(iter-flush_start)) < 0)
     goto err;
 result = string_printer_pack(&printer);
 if unlikely(!result) goto err;
 return result;
err:
 string_printer_fini(&printer);
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeString_Join(DeeObject *__restrict self, DeeObject *__restrict seq) {
 DREF DeeObject *result,*iter,*elem; bool is_first = true;
 struct string_printer printer = STRING_PRINTER_INIT;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 iter = DeeObject_IterSelf(seq);
 if unlikely(!iter) goto err;
 while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
  /* Print `self' prior to every object, starting with the 2nd one. */
  if unlikely(!is_first &&
               string_printer_print(&printer,
                                     DeeString_STR(self),
                                     DeeString_SIZE(self)) < 0)
     goto err_elem;
  if unlikely(DeeObject_Print(elem,&string_printer_print,&printer) < 0)
     goto err_elem;
  Dee_Decref(elem);
  is_first = false;
 }
 Dee_Decref(iter);
 result = string_printer_pack(&printer);
 if unlikely(!result) goto err;
 return result;
err_elem: Dee_Decref(elem);
/*err_iter:*/ Dee_Decref(iter);
err:
 string_printer_fini(&printer);
 return NULL;
}

INTERN DREF DeeObject *DCALL
DeeString_SStrip(DeeObject *__restrict self,
                 DeeObject *__restrict other) {
 uint8_t *str,*new_str; void *other_str;
 int encoding; size_t len,new_len,other_len;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(other,&DeeString_Type);
 encoding = DeeString_UtfEnc(self);
 str = new_str = (uint8_t *)DeeString_UtfStr(self);
 len = new_len = ENCODING_SIZE(str);
 other_str = DeeString_AsEncoding(other,encoding);
 if unlikely(!other_str) return NULL;
 other_len = ENCODING_SIZE(other_str);
 SWITCH_ENC_SIZE(encoding) {
 default:
  while (new_len >= other_len &&
         MEMEQB((uint8_t *)new_str,(uint8_t *)other_str,other_len))
         new_str += other_len,new_len -= other_len;
  while (new_len >= other_len &&
         MEMEQB((uint8_t *)new_str+(new_len-other_len),
                (uint8_t *)other_str,other_len))
         new_len -= other_len;
  if (new_len == len && new_str == str) goto return_self;
  return DeeString_NewSized((char const *)new_str,new_len);
 CASE_ENC_2BYTE:
  while (new_len >= other_len &&
         MEMEQW((uint16_t *)new_str,(uint16_t *)other_str,other_len))
         new_str += other_len*2,new_len -= other_len;
  while (new_len >= other_len &&
         MEMEQW((uint16_t *)new_str+(new_len-other_len),
                (uint16_t *)other_str,other_len))
         new_len -= other_len;
  break;
 CASE_ENC_4BYTE:
  while (new_len >= other_len &&
         MEMEQL((uint32_t *)new_str,(uint32_t *)other_str,other_len))
         new_str += other_len*4,new_len -= other_len;
  while (new_len >= other_len &&
         MEMEQL((uint32_t *)new_str+(new_len-other_len),
                (uint32_t *)other_str,other_len))
         new_len -= other_len;
  break;
 }
 if (new_len == len && new_str == str) goto return_self;
 return DeeString_NewSizedWithEncoding(new_str,new_len,encoding);
return_self:
 return_reference_(self);
}
INTERN DREF DeeObject *DCALL
DeeString_LSStrip(DeeObject *__restrict self,
                  DeeObject *__restrict other) {
 uint8_t *str,*new_str; void *other_str;
 int encoding; size_t len,new_len,other_len;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(other,&DeeString_Type);
 encoding = DeeString_UtfEnc(self);
 str = new_str = (uint8_t *)DeeString_UtfStr(self);
 len = new_len = ENCODING_SIZE(str);
 other_str = DeeString_AsEncoding(other,encoding);
 if unlikely(!other_str) return NULL;
 other_len = ENCODING_SIZE(other_str);
 SWITCH_ENC_SIZE(encoding) {
 default:
  while (new_len >= other_len &&
         MEMEQB((uint8_t *)new_str,(uint8_t *)other_str,other_len))
         new_str += other_len,new_len -= other_len;
  if (new_len == len) goto return_self;
  return DeeString_NewSized((char const *)new_str,new_len);
 CASE_ENC_2BYTE:
  while (new_len >= other_len &&
         MEMEQW((uint16_t *)new_str,(uint16_t *)other_str,other_len))
         new_str += other_len*2,new_len -= other_len;
  break;
 CASE_ENC_4BYTE:
  while (new_len >= other_len &&
         MEMEQL((uint32_t *)new_str,(uint32_t *)other_str,other_len))
         new_str += other_len*4,new_len -= other_len;
  break;
 }
 if (new_len == len) goto return_self;
 return DeeString_NewSizedWithEncoding(new_str,new_len,encoding);
return_self:
 return_reference_(self);
}
INTERN DREF DeeObject *DCALL
DeeString_RSStrip(DeeObject *__restrict self,
                  DeeObject *__restrict other) {
 uint8_t *str; void *other_str;
 int encoding; size_t len,new_len,other_len;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(other,&DeeString_Type);
 encoding = DeeString_UtfEnc(self);
 str = (uint8_t *)DeeString_UtfStr(self);
 len = new_len = ENCODING_SIZE(str);
 other_str = DeeString_AsEncoding(other,encoding);
 if unlikely(!other_str) return NULL;
 other_len = ENCODING_SIZE(other_str);
 SWITCH_ENC_SIZE(encoding) {
 default:
  while (new_len >= other_len &&
         MEMEQB((uint8_t *)str+(new_len-other_len),
                (uint8_t *)other_str,other_len))
         new_len -= other_len;
  if (new_len == len) goto return_self;
  return DeeString_NewSized((char const *)str,new_len);
 CASE_ENC_2BYTE:
  while (new_len >= other_len &&
         MEMEQW((uint16_t *)str+(new_len-other_len),
                (uint16_t *)other_str,other_len))
         new_len -= other_len;
  break;
 CASE_ENC_4BYTE:
  while (new_len >= other_len &&
         MEMEQL((uint32_t *)str+(new_len-other_len),
                (uint32_t *)other_str,other_len))
         new_len -= other_len;
  break;
 }
 if (new_len == len) goto return_self;
 return DeeString_NewSizedWithEncoding(str,new_len,encoding);
return_self:
 return_reference_(self);
}


INTERN DREF DeeObject *DCALL
DeeString_CaseSStrip(DeeObject *__restrict self,
                     DeeObject *__restrict other) {
 uint8_t *str,*new_str; void *other_str;
 int encoding; size_t len,new_len,other_len;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(other,&DeeString_Type);
 encoding = DeeString_UtfEnc(self);
 str = new_str = (uint8_t *)DeeString_UtfStr(self);
 len = new_len = ENCODING_SIZE(str);
 other_str = DeeString_AsEncoding(other,encoding);
 if unlikely(!other_str) return NULL;
 other_len = ENCODING_SIZE(other_str);
 SWITCH_ENC_SIZE(encoding) {
 default:
  while (new_len >= other_len &&
         MEMCASEEQB((uint8_t *)new_str,(uint8_t *)other_str,other_len))
         new_str += other_len,new_len -= other_len;
  while (new_len >= other_len &&
         MEMCASEEQB((uint8_t *)new_str+(new_len-other_len),
                    (uint8_t *)other_str,other_len))
         new_len -= other_len;
  if (new_len == len && new_str == str) goto return_self;
  return DeeString_NewSized((char const *)new_str,new_len);
 CASE_ENC_2BYTE:
  while (new_len >= other_len &&
         MEMCASEEQW((uint16_t *)new_str,(uint16_t *)other_str,other_len))
         new_str += other_len*2,new_len -= other_len;
  while (new_len >= other_len &&
         MEMCASEEQW((uint16_t *)new_str+(new_len-other_len),
                    (uint16_t *)other_str,other_len))
         new_len -= other_len;
  break;
 CASE_ENC_4BYTE:
  while (new_len >= other_len &&
         MEMCASEEQL((uint32_t *)new_str,(uint32_t *)other_str,other_len))
         new_str += other_len*4,new_len -= other_len;
  while (new_len >= other_len &&
         MEMCASEEQL((uint32_t *)new_str+(new_len-other_len),
                    (uint32_t *)other_str,other_len))
         new_len -= other_len;
  break;
 }
 if (new_len == len && new_str == str) goto return_self;
 return DeeString_NewSizedWithEncoding(new_str,new_len,encoding);
return_self:
 return_reference_(self);
}
INTERN DREF DeeObject *DCALL
DeeString_CaseLSStrip(DeeObject *__restrict self,
                      DeeObject *__restrict other) {
 uint8_t *str,*new_str; void *other_str;
 int encoding; size_t len,new_len,other_len;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(other,&DeeString_Type);
 encoding = DeeString_UtfEnc(self);
 str = new_str = (uint8_t *)DeeString_UtfStr(self);
 len = new_len = ENCODING_SIZE(str);
 other_str = DeeString_AsEncoding(other,encoding);
 if unlikely(!other_str) return NULL;
 other_len = ENCODING_SIZE(other_str);
 SWITCH_ENC_SIZE(encoding) {
 default:
  while (new_len >= other_len &&
         MEMCASEEQB((uint8_t *)new_str,(uint8_t *)other_str,other_len))
         new_str += other_len,new_len -= other_len;
  if (new_len == len) goto return_self;
  return DeeString_NewSized((char const *)new_str,new_len);
 CASE_ENC_2BYTE:
  while (new_len >= other_len &&
         MEMCASEEQW((uint16_t *)new_str,(uint16_t *)other_str,other_len))
         new_str += other_len*2,new_len -= other_len;
  break;
 CASE_ENC_4BYTE:
  while (new_len >= other_len &&
         MEMCASEEQL((uint32_t *)new_str,(uint32_t *)other_str,other_len))
         new_str += other_len*4,new_len -= other_len;
  break;
 }
 if (new_len == len) goto return_self;
 return DeeString_NewSizedWithEncoding(new_str,new_len,encoding);
return_self:
 return_reference_(self);
}
INTERN DREF DeeObject *DCALL
DeeString_CaseRSStrip(DeeObject *__restrict self,
                      DeeObject *__restrict other) {
 uint8_t *str; void *other_str;
 int encoding; size_t len,new_len,other_len;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(other,&DeeString_Type);
 encoding = DeeString_UtfEnc(self);
 str = (uint8_t *)DeeString_UtfStr(self);
 len = new_len = ENCODING_SIZE(str);
 other_str = DeeString_AsEncoding(other,encoding);
 if unlikely(!other_str) return NULL;
 other_len = ENCODING_SIZE(other_str);
 SWITCH_ENC_SIZE(encoding) {
 default:
  while (new_len >= other_len &&
         MEMCASEEQB((uint8_t *)str+(new_len-other_len),
                    (uint8_t *)other_str,other_len))
         new_len -= other_len;
  if (new_len == len) goto return_self;
  return DeeString_NewSized((char const *)str,new_len);
 CASE_ENC_2BYTE:
  while (new_len >= other_len &&
         MEMCASEEQW((uint16_t *)str+(new_len-other_len),
                    (uint16_t *)other_str,other_len))
         new_len -= other_len;
  break;
 CASE_ENC_4BYTE:
  while (new_len >= other_len &&
         MEMCASEEQL((uint32_t *)str+(new_len-other_len),
                    (uint32_t *)other_str,other_len))
         new_len -= other_len;
  break;
 }
 if (new_len == len) goto return_self;
 return DeeString_NewSizedWithEncoding(str,new_len,encoding);
return_self:
 return_reference_(self);
}

#define DeeString_IsAlpha(self,start,end)      DeeString_TestTrait(self,start,end,UNICODE_FALPHA)
#define DeeString_IsLower(self,start,end)      DeeString_TestTrait(self,start,end,UNICODE_FLOWER)
#define DeeString_IsUpper(self,start,end)      DeeString_TestTrait(self,start,end,UNICODE_FUPPER)
#define DeeString_IsAlnum(self,start,end)      DeeString_TestTrait(self,start,end,UNICODE_FALPHA|UNICODE_FDIGIT)
#define DeeString_IsSpace(self,start,end)      DeeString_TestTrait(self,start,end,UNICODE_FSPACE)
#define DeeString_IsPrint(self,start,end)      DeeString_TestTrait(self,start,end,UNICODE_FPRINT)
#define DeeString_IsDigit(self,start,end)      DeeString_TestTrait(self,start,end,UNICODE_FDIGIT)
#define DeeString_IsDecimal(self,start,end)    DeeString_TestTrait(self,start,end,UNICODE_FDECIMAL)
#define DeeString_IsNumeric(self,start,end)    DeeString_TestTrait(self,start,end,UNICODE_FDIGIT|UNICODE_FDECIMAL)
#define DeeString_IsAnyAlpha(self,start,end)   DeeString_TestAnyTrait(self,start,end,UNICODE_FALPHA)
#define DeeString_IsAnyLower(self,start,end)   DeeString_TestAnyTrait(self,start,end,UNICODE_FLOWER)
#define DeeString_IsAnyUpper(self,start,end)   DeeString_TestAnyTrait(self,start,end,UNICODE_FUPPER)
#define DeeString_IsAnyAlnum(self,start,end)   DeeString_TestAnyTrait(self,start,end,UNICODE_FALPHA|UNICODE_FDIGIT)
#define DeeString_IsAnySpace(self,start,end)   DeeString_TestAnyTrait(self,start,end,UNICODE_FSPACE)
#define DeeString_IsAnyPrint(self,start,end)   DeeString_TestAnyTrait(self,start,end,UNICODE_FPRINT)
#define DeeString_IsAnyDigit(self,start,end)   DeeString_TestAnyTrait(self,start,end,UNICODE_FDIGIT)
#define DeeString_IsAnyDecimal(self,start,end) DeeString_TestAnyTrait(self,start,end,UNICODE_FDECIMAL)
#define DeeString_IsAnyNumeric(self,start,end) DeeString_TestAnyTrait(self,start,end,UNICODE_FDIGIT|UNICODE_FDECIMAL)

INTERN bool DCALL
DeeString_TestTrait(DeeObject *__restrict self,
                    size_t start_index,
                    size_t end_index,
                    uniflag_t flags) {
 DeeString_UtfForeach(self,start_index,end_index,iter,end,{
  if (!(DeeUni_Flags(*iter)&flags))
        return false;
 });
 return true;
}
INTERN bool DCALL
DeeString_TestAnyTrait(DeeObject *__restrict self,
                       size_t start_index,
                       size_t end_index,
                       uniflag_t flags) {
 DeeString_UtfForeach(self,start_index,end_index,iter,end,{
  if (DeeUni_Flags(*iter)&flags)
      return true;
 });
 return false;
}


INTERN bool DCALL
DeeString_IsTitle(DeeObject *__restrict self,
                  size_t start_index,
                  size_t end_index) {
 uniflag_t flags = (UNICODE_FTITLE|UNICODE_FUPPER|UNICODE_FSPACE);
 DeeString_UtfForeach(self,start_index,end_index,iter,end,{
  uniflag_t f = DeeUni_Flags(*iter);
  if (!(f&flags)) return false;
  flags = (f&UNICODE_FSPACE) ? (UNICODE_FTITLE|UNICODE_FUPPER|UNICODE_FSPACE)
                             : (UNICODE_FLOWER|UNICODE_FSPACE);
 });
 return true;
}
INTERN bool DCALL
DeeString_IsSymbol(DeeObject *__restrict self,
                   size_t start_index,
                   size_t end_index) {
 uniflag_t flags = (UNICODE_FSYMSTRT|UNICODE_FALPHA);
 DeeString_UtfForeach(self,start_index,end_index,iter,end,{
  if (!(DeeUni_Flags(*iter)&flags))
        return false;
  flags |= (UNICODE_FSYMCONT|UNICODE_FDIGIT);
 });
 return true;
}



INTERN DREF DeeObject *DCALL
DeeString_Indent(DeeObject *__restrict self,
                 DeeObject *__restrict filler) {
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(filler,&DeeString_Type);
 /* Simple case: if the filler, or self-string are
  *              empty, nothing would get inserted! */
 if unlikely(DeeString_IsEmpty(filler) || DeeString_IsEmpty(self))
    return_reference_(self);
 {
  DREF DeeObject *result;
  struct string_printer printer = STRING_PRINTER_INIT;
  char *flush_start,*iter,*end;
  /* Start by inserting the initial, unconditional indentation at the start. */
  if (string_printer_print(&printer,
                            DeeString_STR(filler),
                            DeeString_SIZE(filler)) < 0)
      goto err;
  end = (iter = flush_start = DeeString_STR(self)) + DeeString_SIZE(self);
  while (iter < end) {
   char ch = *iter;
   if (ch == '\n' || ch == '\r') {
    ++iter;
    /* Deal with windows-style linefeeds. */
    if (ch == '\r' && *iter == '\n') ++iter;
    /* Flush all unwritten data up to this point. */
    if (string_printer_print(&printer,flush_start,
                            (size_t)(iter-flush_start)) < 0)
        goto err;
    flush_start = iter;
    /* Insert the filler just before the linefeed. */
    if (string_printer_print(&printer,
                              DeeString_STR(filler),
                              DeeString_SIZE(filler)) < 0)
        goto err;
    continue;
   }
   ++iter;
  }
  if (iter == flush_start) {
   /* Either the string is empty, ends with a line-feed.
    * In either case, we must remove `filler' from its end,
    * because we're not supposed to have the resulting
    * string include it as trailing memory. */
   ASSERT(printer.sp_length >= DeeString_SIZE(filler));
   printer.sp_length -= DeeString_SIZE(filler);
  } else {
   /* Flush the remainder. */
   if (string_printer_print(&printer,flush_start,
                           (size_t)(iter-flush_start)) < 0)
       goto err;
  }
  result = string_printer_pack_enc(&printer,DeeString_UtfEnc(self));
  if unlikely(!result) goto err;
  return result;
err:
  string_printer_fini(&printer);
  return NULL;
 }
}
INTERN DREF DeeObject *DCALL
DeeString_Dedent(DeeObject *__restrict self,
                 size_t max_chars,
                 DeeObject *__restrict mask) {
 DREF DeeObject *result; size_t i;
 struct string_printer printer = STRING_PRINTER_INIT;
 char *flush_start,*iter,*end;
 char *mask_str; size_t mask_len;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(mask,&DeeString_Type);
 /* Simple case: Nothing should be removed. */
 if unlikely(!max_chars)
    return_reference_(self);
 mask_str = DeeString_STR(mask);
 mask_len = DeeString_SIZE(mask);
 end = (iter = DeeString_STR(self)) + DeeString_SIZE(self);
 /* Remove leading characters. */
 for (i = 0; i < max_chars && memchrb(mask_str,*iter,mask_len); ++i) ++iter;
 flush_start = iter;
 while (iter < end) {
  char ch = *iter;
  if (ch == '\r' || ch == '\n') {
   ++iter;
   if (ch == '\r' && *iter == '\n') ++iter;
   /* Flush all unwritten data up to this point. */
   if (string_printer_print(&printer,flush_start,
                           (size_t)(iter-flush_start)) < 0)
       goto err;
   /* Skip up to `max_chars' characters after a linefeed. */
   for (i = 0; i < max_chars && memchrb(mask_str,*iter,mask_len); ++i) ++iter;
   flush_start = iter;
   continue;
  }
  ++iter;
 }
 /* Flush the remainder. */
 if (string_printer_print(&printer,flush_start,
                         (size_t)(iter-flush_start)) < 0)
     goto err;
 result = string_printer_pack_enc(&printer,DeeString_UtfEnc(self));
 if unlikely(!result) goto err;
 return result;
err:
 string_printer_fini(&printer);
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeString_DedentSpc(DeeObject *__restrict self,
                    size_t max_chars) {
 DREF DeeObject *result; size_t i;
 struct string_printer printer = STRING_PRINTER_INIT;
 char *flush_start,*iter,*end;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 /* Simple case: Nothing should be removed. */
 if unlikely(!max_chars)
    return_reference_(self);
 end = (iter = DeeString_STR(self)) + DeeString_SIZE(self);
 /* Remove leading spaces. */
 for (i = 0; i < max_chars && DeeUni_IsSpace(*iter); ++i) ++iter;
 flush_start = iter;
 while (iter < end) {
  char ch = *iter;
  if (ch == '\r' || ch == '\n') {
   ++iter;
   if (ch == '\r' && *iter == '\n') ++iter;
   /* Flush all unwritten data up to this point. */
   if (string_printer_print(&printer,flush_start,
                           (size_t)(iter-flush_start)) < 0)
       goto err;
   /* Skip up to `max_chars' space characters after a linefeed. */
   for (i = 0; i < max_chars && DeeUni_IsSpace(*iter); ++i) ++iter;
   flush_start = iter;
   continue;
  }
  ++iter;
 }
 /* Flush the remainder. */
 if (string_printer_print(&printer,flush_start,
                         (size_t)(iter-flush_start)) < 0)
     goto err;
 result = string_printer_pack_enc(&printer,DeeString_UtfEnc(self));
 if unlikely(!result) goto err;
 return result;
err:
 string_printer_fini(&printer);
 return NULL;
}

#ifdef CONFIG_USE_NEW_STRING_API
PUBLIC DREF DeeObject *DCALL
DeeString_NewUtf16(uint16_t const *__restrict str,
                   size_t length,
                   unsigned int error_mode) {
 uint16_t *buffer;
 buffer = DeeString_NewUtf16Buffer(length);
 if unlikely(!buffer) return NULL;
 memcpyw(buffer,str,length);
 return DeeString_PackUtf16Buffer(buffer,error_mode);
}
PUBLIC DREF DeeObject *DCALL
DeeString_NewUtf32(uint32_t const *__restrict str,
                   size_t length,
                   unsigned int error_mode) {
 uint32_t *buffer;
 buffer = DeeString_NewUtf32Buffer(length);
 if unlikely(!buffer) return NULL;
 memcpyl(buffer,str,length);
 return DeeString_PackUtf32Buffer(buffer,error_mode);
}
PUBLIC DREF DeeObject *DCALL
DeeString_NewWide(dwchar_t const *__restrict str,
                  size_t length,
                  unsigned int error_mode) {
 dwchar_t *buffer;
 buffer = DeeString_NewWideBuffer(length);
 if unlikely(!buffer) return NULL;
#if __SIZEOF_WCHAR_T__ == 2
 memcpyw(buffer,str,length);
#else
 memcpyl(buffer,str,length);
#endif
 return DeeString_PackWideBuffer(buffer,error_mode);
}
PUBLIC DREF DeeObject *DCALL
DeeString_NewUtf16AltEndian(uint16_t const *__restrict str,
                            size_t length,
                            unsigned int error_mode) {
 uint16_t *buffer; size_t i;
 buffer = DeeString_NewUtf16Buffer(length);
 if unlikely(!buffer) return NULL;
 for (i = 0; i < length; ++i)
     buffer[i] = DEE_BSWAP16(str[i]);
 return DeeString_PackUtf16Buffer(buffer,error_mode);
}
PUBLIC DREF DeeObject *DCALL
DeeString_NewUtf32AltEndian(uint32_t const *__restrict str,
                            size_t length,
                            unsigned int error_mode) {
 uint32_t *buffer; size_t i;
 buffer = DeeString_NewUtf32Buffer(length);
 if unlikely(!buffer) return NULL;
 for (i = 0; i < length; ++i)
     buffer[i] = DEE_BSWAP32(str[i]);
 return DeeString_PackUtf32Buffer(buffer,error_mode);
}
PUBLIC DREF DeeObject *DCALL
DeeString_NewWideAltEndian(dwchar_t const *__restrict str,
                           size_t length,
                           unsigned int error_mode) {
 dwchar_t *buffer; size_t i;
 buffer = DeeString_NewWideBuffer(length);
 if unlikely(!buffer) return NULL;
#if __SIZEOF_WCHAR_T__ == 2
 for (i = 0; i < length; ++i)
     ((uint16_t *)buffer)[i] = DEE_BSWAP16(((uint16_t *)str)[i]);
#else
 for (i = 0; i < length; ++i)
     ((uint32_t *)buffer)[i] = DEE_BSWAP32(((uint32_t *)str)[i]);
#endif
 return DeeString_PackWideBuffer(buffer,error_mode);
}
#endif




PRIVATE DREF String *DCALL
string_replace(String *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
 String *find,*replace; size_t max_count = (size_t)-1;
 size_t find_size,replace_size; struct string_printer p;
 char *begin,*end,*sfind,*sreplace,*block_begin;
 if (DeeArg_Unpack(argc,argv,"oo|Iu:replace",&find,&replace,&max_count) ||
     DeeObject_AssertTypeExact((DeeObject *)find,&DeeString_Type) ||
     DeeObject_AssertTypeExact((DeeObject *)replace,&DeeString_Type))
     return NULL;
 find_size = DeeString_SIZE(find);
 /* Handle special cases. */
 if unlikely(find_size > DeeString_SIZE(self))
    goto return_self;
 if unlikely(!find_size) {
  if (DeeString_SIZE(self)) goto return_self;
  return_reference_(replace);
 }
 string_printer_init(&p);
 replace_size = DeeString_SIZE(replace);
 end = (begin = DeeString_STR(self))+(DeeString_SIZE(self)-(find_size-1));
 sfind = DeeString_STR(find),sreplace = DeeString_STR(replace);
 block_begin = begin;
 if likely(max_count) while (begin <= end) {
  if (memcmp(begin,sfind,find_size*sizeof(char)) == 0) {
   /* Found one */
   if (unlikely(string_printer_print(&p,block_begin,(size_t)(begin-block_begin)) < 0) ||
       unlikely(string_printer_print(&p,sreplace,replace_size) < 0))
       goto err;
   begin += find_size;
   block_begin = begin;
   if (begin >= end) break;
   if unlikely(!--max_count) break;
   continue;
  }
  ++begin;
 }
 /* If we never found `find', our printer will still be empty.
  * >> In that case we don't need to write the entire string to it,
  *    but can simply return a reference to the original string,
  *    saving on memory and speeding up the function by a lot. */
 if (p.sp_length == 0 &&
     block_begin == DeeString_STR(self))
     return_reference_(self);
 if unlikely(string_printer_print(&p,block_begin,(size_t)((end-block_begin)+(find_size-1))) < 0)
    goto err;
 /* Pack together a string, following the original string's encoding. */
 find = (DREF String *)string_printer_pack_enc(&p,DeeString_UtfEnc(self));
 if unlikely(!find) goto err;
 return find;
err:
 string_printer_fini(&p);
 return NULL;
return_self:
 return_reference_(self);
}

PRIVATE DREF String *DCALL
string_casereplace(String *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
 String *find,*replace; size_t max_count = (size_t)-1;
 size_t find_size,replace_size; struct string_printer p;
 char *begin,*end,*sfind,*sreplace,*block_begin;
 if (DeeArg_Unpack(argc,argv,"oo|Iu:casereplace",&find,&replace,&max_count) ||
     DeeObject_AssertTypeExact((DeeObject *)find,&DeeString_Type) ||
     DeeObject_AssertTypeExact((DeeObject *)replace,&DeeString_Type))
     return NULL;
 find_size = DeeString_SIZE(find);
 /* Handle special cases. */
 if unlikely(find_size > DeeString_SIZE(self))
    goto return_self;
 if unlikely(!find_size) {
  if (DeeString_SIZE(self)) goto return_self;
  return_reference_(replace);
 }
 string_printer_init(&p);
 replace_size = DeeString_SIZE(replace);
 end = (begin = DeeString_STR(self))+(DeeString_SIZE(self)-(find_size-1));
 sfind = DeeString_STR(find),sreplace = DeeString_STR(replace);
 block_begin = begin;
 if likely(max_count) while (begin <= end) {
  if (MEMCASEEQB(begin,sfind,find_size*sizeof(char))) {
   /* Found one */
   if (unlikely(string_printer_print(&p,block_begin,(size_t)(begin-block_begin)) < 0) ||
       unlikely(string_printer_print(&p,sreplace,replace_size) < 0))
       goto err;
   begin += find_size;
   block_begin = begin;
   if (begin >= end) break;
   if unlikely(!--max_count) break;
   continue;
  }
  ++begin;
 }
 /* If we never found `find', our printer will still be empty.
  * >> In that case we don't need to write the entire string to it,
  *    but can simply return a reference to the original string,
  *    saving on memory and speeding up the function by a lot. */
 if (p.sp_length == 0 &&
     block_begin == DeeString_STR(self))
     return_reference_(self);
 if unlikely(string_printer_print(&p,block_begin,(size_t)((end-block_begin)+(find_size-1))) < 0)
    goto err;
 /* Pack together a string, following the original string's encoding. */
 find = (DREF String *)string_printer_pack_enc(&p,DeeString_UtfEnc(self));
 if unlikely(!find) goto err;
 return find;
err:
 string_printer_fini(&p);
 return NULL;
return_self:
 return_reference_(self);
}

PRIVATE DREF DeeObject *DCALL
string_ord(String *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":ord"))
     return NULL;
 if unlikely(DeeString_UtfLen(self) != 1) {
  err_expected_single_character_string((DeeObject *)self);
  return NULL;
 }
 return DeeInt_NewU32(DeeString_UtfGet((DeeObject *)self,0));
}

#define DEFINE_STRING_TRAIT(name,function) \
PRIVATE DREF DeeObject *DCALL \
string_##name(String *__restrict self, \
              size_t argc, DeeObject **__restrict argv) { \
 size_t start = 0,end = (size_t)-1; \
 if (DeeArg_Unpack(argc,argv,"|IdId" #name,&start,&end)) \
     return NULL; \
 return_bool(function((DeeObject *)self,start,end)); \
}
DEFINE_STRING_TRAIT(isalpha,DeeString_IsAlpha)
DEFINE_STRING_TRAIT(islower,DeeString_IsLower)
DEFINE_STRING_TRAIT(isupper,DeeString_IsUpper)
DEFINE_STRING_TRAIT(isalnum,DeeString_IsAlnum)
DEFINE_STRING_TRAIT(isspace,DeeString_IsSpace)
DEFINE_STRING_TRAIT(isprint,DeeString_IsPrint)
DEFINE_STRING_TRAIT(isdigit,DeeString_IsDigit)
DEFINE_STRING_TRAIT(isdecimal,DeeString_IsDecimal)
DEFINE_STRING_TRAIT(isnumeric,DeeString_IsNumeric)
DEFINE_STRING_TRAIT(isanyalpha,DeeString_IsAnyAlpha)
DEFINE_STRING_TRAIT(isanylower,DeeString_IsAnyLower)
DEFINE_STRING_TRAIT(isanyupper,DeeString_IsAnyUpper)
DEFINE_STRING_TRAIT(isanyalnum,DeeString_IsAnyAlnum)
DEFINE_STRING_TRAIT(isanyspace,DeeString_IsAnySpace)
DEFINE_STRING_TRAIT(isanyprint,DeeString_IsAnyPrint)
DEFINE_STRING_TRAIT(isanydigit,DeeString_IsAnyDigit)
DEFINE_STRING_TRAIT(isanydecimal,DeeString_IsAnyDecimal)
DEFINE_STRING_TRAIT(isanynumeric,DeeString_IsAnyNumeric)
DEFINE_STRING_TRAIT(istitle,DeeString_IsTitle)
DEFINE_STRING_TRAIT(issymbol,DeeString_IsSymbol)
#undef DEFINE_STRING_TRAIT

PRIVATE DREF DeeObject *DCALL
string_lower(String *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":lower"))
     return NULL;
 return DeeString_ToLower((DeeObject *)self);
}
PRIVATE DREF DeeObject *DCALL
string_upper(String *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":upper"))
     return NULL;
 return DeeString_ToUpper((DeeObject *)self);
}
PRIVATE DREF DeeObject *DCALL
string_title(String *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":title"))
     return NULL;
 return DeeString_ToTitle((DeeObject *)self);
}
PRIVATE DREF DeeObject *DCALL
string_capitalize(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":capitalize"))
     return NULL;
 return DeeString_Capitalize((DeeObject *)self);
}
PRIVATE DREF DeeObject *DCALL
string_swapcase(String *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":swapcase"))
     return NULL;
 return DeeString_Swapcase((DeeObject *)self);
}
PRIVATE DREF DeeObject *DCALL
string_find(String *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
 String *other; size_t begin = 0,end = (size_t)-1;
 dssize_t result;
 if (DeeArg_Unpack(argc,argv,"o|IdId:find",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     return NULL;
 result = DeeString_Find((DeeObject *)self,
                         (DeeObject *)other,begin,end);
 if unlikely(result == -2)
    return NULL;
 return DeeInt_NewSSize(result);
}
PRIVATE DREF DeeObject *DCALL
string_rfind(String *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 String *other; size_t begin = 0,end = (size_t)-1;
 dssize_t result;
 if (DeeArg_Unpack(argc,argv,"o|IdId:rfind",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     return NULL;
 result = DeeString_RFind((DeeObject *)self,
                          (DeeObject *)other,begin,end);
 if unlikely(result == -2)
    return NULL;
 return DeeInt_NewSSize(result);
}
PRIVATE DREF DeeObject *DCALL
string_index(String *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 String *other; size_t begin = 0,end = (size_t)-1;
 dssize_t result;
 if (DeeArg_Unpack(argc,argv,"o|IdId:index",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     return NULL;
 result = DeeString_Find((DeeObject *)self,
                         (DeeObject *)other,begin,end);
 if unlikely(result == -1)
    err_index_not_found((DeeObject *)self,(DeeObject *)other);
 if unlikely(result < 0)
    return NULL;
 return DeeInt_NewSize((size_t)result);
}
PRIVATE DREF DeeObject *DCALL
string_rindex(String *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 String *other; size_t begin = 0,end = (size_t)-1;
 dssize_t result;
 if (DeeArg_Unpack(argc,argv,"o|IdId:rindex",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     return NULL;
 result = DeeString_RFind((DeeObject *)self,
                          (DeeObject *)other,begin,end);
 if unlikely(result == -1)
    err_index_not_found((DeeObject *)self,(DeeObject *)other);
 if unlikely(result < 0)
    return NULL;
 return DeeInt_NewSize((size_t)result);
}
PRIVATE DREF DeeObject *DCALL
string_casefind(String *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
 String *other; size_t begin = 0,end = (size_t)-1;
 dssize_t result;
 if (DeeArg_Unpack(argc,argv,"o|IdId:casefind",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     return NULL;
 result = DeeString_CaseFind((DeeObject *)self,
                             (DeeObject *)other,begin,end);
 if unlikely(result == -2)
    return NULL;
 return DeeInt_NewSSize(result);
}
PRIVATE DREF DeeObject *DCALL
string_caserfind(String *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 String *other; size_t begin = 0,end = (size_t)-1;
 dssize_t result;
 if (DeeArg_Unpack(argc,argv,"o|IdId:caserfind",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     return NULL;
 result = DeeString_CaseRFind((DeeObject *)self,
                              (DeeObject *)other,begin,end);
 if unlikely(result == -2)
    return NULL;
 return DeeInt_NewSSize(result);
}
PRIVATE DREF DeeObject *DCALL
string_caseindex(String *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 String *other; size_t begin = 0,end = (size_t)-1;
 dssize_t result;
 if (DeeArg_Unpack(argc,argv,"o|IdId:caseindex",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     return NULL;
 result = DeeString_CaseFind((DeeObject *)self,
                             (DeeObject *)other,begin,end);
 if unlikely(result == -1)
    err_index_not_found((DeeObject *)self,(DeeObject *)other);
 if unlikely(result < 0)
    return NULL;
 return DeeInt_NewSize((size_t)result);
}
PRIVATE DREF DeeObject *DCALL
string_caserindex(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 String *other; size_t begin = 0,end = (size_t)-1;
 dssize_t result;
 if (DeeArg_Unpack(argc,argv,"o|IdId:caserindex",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     return NULL;
 result = DeeString_CaseFind((DeeObject *)self,
                             (DeeObject *)other,begin,end);
 if unlikely(result == -1)
    err_index_not_found((DeeObject *)self,(DeeObject *)other);
 if unlikely(result < 0)
    return NULL;
 return DeeInt_NewSize((size_t)result);
}
PRIVATE DREF String *DCALL
string_getsubstr(String *__restrict self,
                 size_t start, size_t end) {
 DREF String *result;
 void *str = DeeString_UtfStr(self);
 size_t len = ENCODING_SIZE(str);
 if (start == 0 && end >= len) {
  result = self;
  Dee_Incref(result);
 } else {
  if (end >= len) end = len;
  if (start >= end) {
   result = (DREF String *)Dee_EmptyString;
   Dee_Incref(Dee_EmptyString);
  } else {
   int enc = DeeString_UtfEnc(self);
   result = (DREF String *)DeeString_NewSizedWithEncoding((uint8_t *)str+
                                                          (start*DeeEnc_Size(enc)),
                                                           end-(size_t)start,enc);
  }
 }
 return result;
}

PRIVATE DREF DeeObject *DCALL
string_substr(String *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 size_t start = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"|IdId:substr",&start,&end))
     return NULL;
 return (DREF DeeObject *)string_getsubstr(self,start,end);
}
PRIVATE DREF DeeObject *DCALL
string_strip(String *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 DeeObject *mask = NULL;
 if (DeeArg_Unpack(argc,argv,"|o:strip",&mask))
     return NULL;
 return mask ? (DeeObject_AssertTypeExact(mask,&DeeString_Type) ? NULL :
                DeeString_StripMask((DeeObject *)self,mask))
             :  DeeString_StripSpc((DeeObject *)self);
}
PRIVATE DREF DeeObject *DCALL
string_lstrip(String *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 DeeObject *mask = NULL;
 if (DeeArg_Unpack(argc,argv,"|o:lstrip",&mask))
     return NULL;
 return mask ? (DeeObject_AssertTypeExact(mask,&DeeString_Type) ? NULL :
                DeeString_LStripMask((DeeObject *)self,mask))
             :  DeeString_LStripSpc((DeeObject *)self);
}
PRIVATE DREF DeeObject *DCALL
string_rstrip(String *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 DeeObject *mask = NULL;
 if (DeeArg_Unpack(argc,argv,"|o:rstrip",&mask))
     return NULL;
 return mask ? (DeeObject_AssertTypeExact(mask,&DeeString_Type) ? NULL :
                DeeString_RStripMask((DeeObject *)self,mask))
             :  DeeString_RStripSpc((DeeObject *)self);
}
PRIVATE DREF DeeObject *DCALL
string_startswith(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 String *other; size_t begin = 0,end = (size_t)-1;
 uint8_t *my_str,*other_str; size_t my_len,other_len;
 int encoding;
 if (DeeArg_Unpack(argc,argv,"o|IdId:startswith",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     return NULL;
 if unlikely(end <= begin)
    return_false;
 if (begin == 0 && end >= DeeString_SIZE(self)) {
  /* Special case: Since we don't have to count characters, we can simply
   *               match the UTF-8 representations against each other.
   * HINT: We are allowed to check for `end >= DeeString_SIZE(self)' because
   *       the UTF-8 representation has the most amount of characters. */
  if (DeeString_SIZE(other) > DeeString_SIZE(self) ||
      memcmp(DeeString_STR(self),DeeString_STR(other),
             DeeString_SIZE(other)*sizeof(char)) != 0)
      return_false;
  return_true;
 }
 /* Must decode the other string in order to match its contents
  * against data from our string at a specific offset. */
 encoding = DeeString_UtfEnc(self);
 my_str = (uint8_t *)DeeString_UtfStr(self);
 my_len = ENCODING_SIZE(my_str);
 other_str = (uint8_t *)DeeString_AsEncoding((DeeObject *)other,encoding);
 if unlikely(!other_str) return NULL;
 other_len = ENCODING_SIZE(other_str);
 my_len  = MIN(my_len,end);
 my_len -= begin;
 if (other_len > my_len) return_false;
 SWITCH_ENC_SIZE(encoding) {
 default:
  return_bool(MEMEQB(my_str+begin,other_str,other_len*sizeof(char)));
 CASE_ENC_2BYTE:
  return_bool(MEMEQW(my_str+begin*2,other_str,other_len));
 CASE_ENC_4BYTE:
  return_bool(MEMEQL(my_str+begin*4,other_str,other_len));
 }
}
PRIVATE DREF DeeObject *DCALL
string_endswith(String *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
 String *other; size_t begin = 0,end = (size_t)-1;
 uint8_t *my_str,*other_str; size_t my_len,other_len;
 int encoding;
 if (DeeArg_Unpack(argc,argv,"o|IdId:endswith",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     return NULL;
 if unlikely(end <= begin)
    return_false;
 if (begin == 0 && end >= DeeString_SIZE(self)) {
  /* Special case: Since we don't have to count characters, we can simply
   *               match the UTF-8 representations against each other.
   * HINT: We are allowed to check for `end >= DeeString_SIZE(self)' because
   *       the UTF-8 representation has the most amount of characters. */
  if (DeeString_SIZE(other) > DeeString_SIZE(self) ||
      memcmp(DeeString_STR(self)+
            (DeeString_SIZE(self)-DeeString_SIZE(other)),
             DeeString_STR(other),DeeString_SIZE(other)*sizeof(char)) != 0)
      return_false;
  return_true;
 }
 /* Must decode the other string in order to match its contents
  * against data from our string at a specific offset. */
 encoding = DeeString_UtfEnc(self);
 my_str = (uint8_t *)DeeString_UtfStr(self);
 my_len = ENCODING_SIZE(my_str);
 other_str = (uint8_t *)DeeString_AsEncoding((DeeObject *)other,encoding);
 if unlikely(!other_str) return NULL;
 other_len = ENCODING_SIZE(other_str);
 my_len  = MIN(my_len,end);
 my_len -= begin;
 if (other_len > my_len) return_false;
 begin  += my_len;
 begin  -= other_len;
 SWITCH_ENC_SIZE(encoding) {
 default:
  return_bool(MEMEQB(my_str+begin,other_str,other_len*sizeof(char)));
 CASE_ENC_2BYTE:
  return_bool(MEMEQW(my_str+begin*2,other_str,other_len));
 CASE_ENC_4BYTE:
  return_bool(MEMEQL(my_str+begin*4,other_str,other_len));
 }
}
PRIVATE DREF DeeObject *DCALL
string_casestartswith(String *__restrict self,
                      size_t argc, DeeObject **__restrict argv) {
 String *other; size_t begin = 0,end = (size_t)-1;
 uint8_t *my_str,*other_str; size_t my_len,other_len;
 int encoding;
 if (DeeArg_Unpack(argc,argv,"o|IdId:casestartswith",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     return NULL;
 if unlikely(end <= begin)
    return_false;
 if (begin == 0 && end >= DeeString_SIZE(self)) {
  /* Special case: Since we don't have to count characters, we can simply
   *               match the UTF-8 representations against each other.
   * HINT: We are allowed to check for `end >= DeeString_SIZE(self)' because
   *       the UTF-8 representation has the most amount of characters. */
  if (DeeString_SIZE(other) > DeeString_SIZE(self) ||
     !MEMCASEEQB(DeeString_STR(self),DeeString_STR(other),
                 DeeString_SIZE(other)*sizeof(char)))
      return_false;
  return_true;
 }
 /* Must decode the other string in order to match its contents
  * against data from our string at a specific offset. */
 encoding = DeeString_UtfEnc(self);
 my_str = (uint8_t *)DeeString_UtfStr(self);
 my_len = ENCODING_SIZE(my_str);
 other_str = (uint8_t *)DeeString_AsEncoding((DeeObject *)other,encoding);
 if unlikely(!other_str) return NULL;
 other_len = ENCODING_SIZE(other_str);
 my_len  = MIN(my_len,end);
 my_len -= begin;
 if (other_len > my_len) return_false;
 SWITCH_ENC_SIZE(encoding) {
 default:
  return_bool(MEMCASEEQB(my_str+begin,other_str,other_len*sizeof(char)));
 CASE_ENC_2BYTE:
  return_bool(MEMCASEEQW(my_str+begin*2,other_str,other_len));
 CASE_ENC_4BYTE:
  return_bool(MEMCASEEQL(my_str+begin*4,other_str,other_len));
 }
}
PRIVATE DREF DeeObject *DCALL
string_caseendswith(String *__restrict self,
                    size_t argc, DeeObject **__restrict argv) {
 String *other; size_t begin = 0,end = (size_t)-1;
 uint8_t *my_str,*other_str; size_t my_len,other_len;
 int encoding;
 if (DeeArg_Unpack(argc,argv,"o|IdId:caseendswith",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     return NULL;
 if unlikely(end <= begin)
    return_false;
 if (begin == 0 && end >= DeeString_SIZE(self)) {
  /* Special case: Since we don't have to count characters, we can simply
   *               match the UTF-8 representations against each other.
   * HINT: We are allowed to check for `end >= DeeString_SIZE(self)' because
   *       the UTF-8 representation has the most amount of characters. */
  if (DeeString_SIZE(other) > DeeString_SIZE(self) ||
     !MEMCASEEQB(DeeString_STR(self)+
                (DeeString_SIZE(self)-DeeString_SIZE(other)),
                 DeeString_STR(other),DeeString_SIZE(other)*sizeof(char)))
      return_false;
  return_true;
 }
 /* Must decode the other string in order to match its contents
  * against data from our string at a specific offset. */
 encoding = DeeString_UtfEnc(self);
 my_str = (uint8_t *)DeeString_UtfStr(self);
 my_len = ENCODING_SIZE(my_str);
 other_str = (uint8_t *)DeeString_AsEncoding((DeeObject *)other,encoding);
 if unlikely(!other_str) return NULL;
 other_len = ENCODING_SIZE(other_str);
 my_len  = MIN(my_len,end);
 my_len -= begin;
 if (other_len > my_len) return_false;
 begin  += my_len;
 begin  -= other_len;
 SWITCH_ENC_SIZE(encoding) {
 default:
  return_bool(MEMCASEEQB(my_str+begin,other_str,other_len*sizeof(char)));
 CASE_ENC_2BYTE:
  return_bool(MEMCASEEQW(my_str+begin*2,other_str,other_len));
 CASE_ENC_4BYTE:
  return_bool(MEMCASEEQL(my_str+begin*4,other_str,other_len));
 }
}

struct encoding {
    char name[16];
    int  id;
};

struct errors {
    char name[8];
    int  flags;
};

#ifdef CONFIG_USE_NEW_STRING_API

#define STRING_WIDTH_ALTENDIANFLAG STRING_WIDTH_COUNT
#ifdef CONFIG_LITTLE_ENDIAN
#define STRING_WIDTH_BEFLAG  STRING_WIDTH_ALTENDIANFLAG
#define STRING_WIDTH_LEFLAG  0
#else
#define STRING_WIDTH_BEFLAG  0
#define STRING_WIDTH_LEFLAG  STRING_WIDTH_ALTENDIANFLAG
#endif

PRIVATE struct encoding const namedb[] = {
    { "utf-8", DEE_STRING_CODEC_UTF8 },
    { "utf8", DEE_STRING_CODEC_UTF8 },
    { "latin-1", DEE_STRING_CODEC_LATIN1 },
    { "latin1", DEE_STRING_CODEC_LATIN1 },
    { "iso-8859-1", DEE_STRING_CODEC_ISO8859 },
    { "iso8859-1", DEE_STRING_CODEC_ISO8859 },
    { "mscs", DEE_STRING_CODEC_MBCS },
    { "ascii", DEE_STRING_CODEC_ASCII },
    { "utf-16", DEE_STRING_CODEC_COUNT+STRING_WIDTH_2BYTE },
    { "utf-16be", DEE_STRING_CODEC_COUNT+STRING_WIDTH_BEFLAG+STRING_WIDTH_2BYTE },
    { "utf-16le", DEE_STRING_CODEC_COUNT+STRING_WIDTH_LEFLAG+STRING_WIDTH_2BYTE },
    { "utf-16-be", DEE_STRING_CODEC_COUNT+STRING_WIDTH_BEFLAG+STRING_WIDTH_2BYTE },
    { "utf-16-le", DEE_STRING_CODEC_COUNT+STRING_WIDTH_LEFLAG+STRING_WIDTH_2BYTE },
    { "utf16", DEE_STRING_CODEC_COUNT+STRING_WIDTH_2BYTE },
    { "utf16be", DEE_STRING_CODEC_COUNT+STRING_WIDTH_BEFLAG+STRING_WIDTH_2BYTE },
    { "utf16le", DEE_STRING_CODEC_COUNT+STRING_WIDTH_LEFLAG+STRING_WIDTH_2BYTE },
    { "utf16-be", DEE_STRING_CODEC_COUNT+STRING_WIDTH_BEFLAG+STRING_WIDTH_2BYTE },
    { "utf16-le", DEE_STRING_CODEC_COUNT+STRING_WIDTH_LEFLAG+STRING_WIDTH_2BYTE },
    { "utf-32", DEE_STRING_CODEC_COUNT+STRING_WIDTH_4BYTE },
    { "utf-32be", DEE_STRING_CODEC_COUNT+STRING_WIDTH_BEFLAG+STRING_WIDTH_4BYTE },
    { "utf-32le", DEE_STRING_CODEC_COUNT+STRING_WIDTH_LEFLAG+STRING_WIDTH_4BYTE },
    { "utf-32-be", DEE_STRING_CODEC_COUNT+STRING_WIDTH_BEFLAG+STRING_WIDTH_4BYTE },
    { "utf-32-le", DEE_STRING_CODEC_COUNT+STRING_WIDTH_LEFLAG+STRING_WIDTH_4BYTE },
    { "utf32", DEE_STRING_CODEC_COUNT+STRING_WIDTH_4BYTE },
    { "utf32be", DEE_STRING_CODEC_COUNT+STRING_WIDTH_BEFLAG+STRING_WIDTH_4BYTE },
    { "utf32le", DEE_STRING_CODEC_COUNT+STRING_WIDTH_LEFLAG+STRING_WIDTH_4BYTE },
    { "utf32-be", DEE_STRING_CODEC_COUNT+STRING_WIDTH_BEFLAG+STRING_WIDTH_4BYTE },
    { "utf32-le", DEE_STRING_CODEC_COUNT+STRING_WIDTH_LEFLAG+STRING_WIDTH_4BYTE },
    { "wide", DEE_STRING_CODEC_COUNT+STRING_WIDTH_WCHAR },
    { "widebe", DEE_STRING_CODEC_COUNT+STRING_WIDTH_BEFLAG+STRING_WIDTH_WCHAR },
    { "widele", DEE_STRING_CODEC_COUNT+STRING_WIDTH_LEFLAG+STRING_WIDTH_WCHAR },
    { "wide-be", DEE_STRING_CODEC_COUNT+STRING_WIDTH_BEFLAG+STRING_WIDTH_WCHAR },
    { "wide-le", DEE_STRING_CODEC_COUNT+STRING_WIDTH_LEFLAG+STRING_WIDTH_WCHAR },
    { "wchar", DEE_STRING_CODEC_COUNT+STRING_WIDTH_WCHAR },
    { "wcharbe", DEE_STRING_CODEC_COUNT+STRING_WIDTH_BEFLAG+STRING_WIDTH_WCHAR },
    { "wcharle", DEE_STRING_CODEC_COUNT+STRING_WIDTH_LEFLAG+STRING_WIDTH_WCHAR },
    { "wchar-be", DEE_STRING_CODEC_COUNT+STRING_WIDTH_BEFLAG+STRING_WIDTH_WCHAR },
    { "wchar-le", DEE_STRING_CODEC_COUNT+STRING_WIDTH_LEFLAG+STRING_WIDTH_WCHAR },
    { "widestring", DEE_STRING_CODEC_COUNT+STRING_WIDTH_WCHAR },
    { "widestringbe", DEE_STRING_CODEC_COUNT+STRING_WIDTH_BEFLAG+STRING_WIDTH_WCHAR },
    { "widestringle", DEE_STRING_CODEC_COUNT+STRING_WIDTH_LEFLAG+STRING_WIDTH_WCHAR },
    { "widestring-be", DEE_STRING_CODEC_COUNT+STRING_WIDTH_BEFLAG+STRING_WIDTH_WCHAR },
    { "widestring-le", DEE_STRING_CODEC_COUNT+STRING_WIDTH_LEFLAG+STRING_WIDTH_WCHAR },
};
PRIVATE struct errors const errorsdb[] = {
    { "strict", DEE_STRING_CODEC_FSTRICT },
    { "replace", DEE_STRING_CODEC_FREPLAC },
    { "ignore", DEE_STRING_CODEC_FIGNORE }
};


PRIVATE DREF DeeObject *DCALL
string_decode(String *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 char *codec_name,*errors = NULL; size_t i;
 unsigned int codec,errors_mode = DEE_STRING_CODEC_FSTRICT;
 if (DeeArg_Unpack(argc,argv,"s|s:decode",&codec_name,&errors))
     return NULL;
 for (i = 0; i < COMPILER_LENOF(namedb); ++i) {
  if (STRCASEEQ(namedb[i].name,codec_name)) {
   codec = namedb[i].id;
   goto got_encoding;
  }
 }
 DeeError_Throwf(&DeeError_ValueError,
                 "Unknown codec `%s'",
                  codec_name);
 return NULL;
got_encoding:
 if (errors) {
  for (i = 0; i < COMPILER_LENOF(errorsdb); ++i) {
   if (STRCASEEQ(errorsdb[i].name,errors)) {
    errors_mode = errorsdb[i].flags;
    goto got_errors;
   }
  }
  DeeError_Throwf(&DeeError_ValueError,
                  "Unknown error mode `%s'",
                  errors);
  return NULL;
 }
got_errors:
 /* Re-interpret the current string using the given encoding. */
 if (codec >= DEE_STRING_CODEC_COUNT) {
  /* Re-interpret string bytes as multi-byte, fixed-character-width string. */
  switch (codec) {
  case STRING_WIDTH_2BYTE + DEE_STRING_CODEC_COUNT:
   return DeeString_NewUtf16((uint16_t *)DeeString_STR(self),
                              DeeString_SIZE(self)/sizeof(uint16_t),
                              errors_mode);
  case STRING_WIDTH_4BYTE + DEE_STRING_CODEC_COUNT:
   return DeeString_NewUtf32((uint32_t *)DeeString_STR(self),
                              DeeString_SIZE(self)/sizeof(uint32_t),
                              errors_mode);
  case STRING_WIDTH_WCHAR + DEE_STRING_CODEC_COUNT:
   return DeeString_NewWide((dwchar_t *)DeeString_STR(self),
                             DeeString_SIZE(self)/sizeof(dwchar_t),
                             errors_mode);
  case STRING_WIDTH_2BYTE + DEE_STRING_CODEC_COUNT + STRING_WIDTH_ALTENDIANFLAG:
   return DeeString_NewUtf16AltEndian((uint16_t *)DeeString_STR(self),
                                       DeeString_SIZE(self)/sizeof(uint16_t),
                                       errors_mode);
  case STRING_WIDTH_4BYTE + DEE_STRING_CODEC_COUNT + STRING_WIDTH_ALTENDIANFLAG:
   return DeeString_NewUtf32AltEndian((uint32_t *)DeeString_STR(self),
                                       DeeString_SIZE(self)/sizeof(uint32_t),
                                       errors_mode);
  case STRING_WIDTH_WCHAR + DEE_STRING_CODEC_COUNT + STRING_WIDTH_ALTENDIANFLAG:
   return DeeString_NewWideAltEndian((dwchar_t *)DeeString_STR(self),
                                      DeeString_SIZE(self)/sizeof(dwchar_t),
                                      errors_mode);
  default: __builtin_unreachable();
  }
 }
 /* Decode a variant of a multi-byte character string */
 return DeeString_NewWithCodec((unsigned char *)DeeString_STR(self),
                                DeeString_SIZE(self),
                                codec|errors_mode);
}
#else
PRIVATE struct encoding const namedb[] = {
    { "utf-8", STRING_WIDTH_1BYTE },
    { "utf-16", STRING_WIDTH_2BYTE },
    { "utf-32", STRING_WIDTH_4BYTE },
    { "utf8", STRING_WIDTH_1BYTE },
    { "utf16", STRING_WIDTH_2BYTE },
    { "utf32", STRING_WIDTH_4BYTE },
    { "latin-1", STRING_WIDTH_1BYTE },
    { "latin1", STRING_WIDTH_1BYTE },
    { "iso-8859-1", STRING_WIDTH_1BYTE },
    { "iso8859-1", STRING_WIDTH_1BYTE },
#ifdef CONFIG_WCHAR_STRINGS
    { "wchar", STRING_WIDTH_WCHAR },
#endif
    { "ascii", STRING_WIDTH_1BYTE }
};
PRIVATE struct errors const errorsdb[] = {
    { "strict", UNICODE_ERRORS_STRICT },
    { "replace", UNICODE_ERRORS_REPLACE },
    { "ignore", UNICODE_ERRORS_IGNORE }
};


PRIVATE DREF DeeObject *DCALL
string_decode(String *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 void *str; size_t len;
 char const *codec_name = NULL;
 char const *errors   = NULL;
 int codec      = STRING_WIDTH_1BYTE;
 int errors_mode        = UNICODE_ERRORS_STRICT;
 int my_encoding;
 if (DeeArg_Unpack(argc,argv,"|ss:decode",&codec_name,&errors))
     return NULL;
 if (codec_name) {
  struct codec_name const *iter;
  for (iter  = namedb;
       iter != COMPILER_ENDOF(namedb); ++iter) {
   if (STRCASEEQ(iter->name,codec_name)) {
    codec = iter->id;
    goto got_encoding;
   }
  }
  DeeError_Throwf(&DeeError_ValueError,
                  "Unknown encoding `%s'",codec_name);
  return NULL;
 }
got_encoding:
 if (errors) {
  struct errors const *iter;
  for (iter  = errorsdb;
       iter != COMPILER_ENDOF(errorsdb); ++iter) {
   if (STRCASEEQ(iter->name,codec_name)) {
    errors_mode = iter->flags;
    goto got_errors;
   }
  }
  DeeError_Throwf(&DeeError_ValueError,
                  "Unknown error mode `%s'",codec_name);
  return NULL;
 }
got_errors:
 /* Re-interpret the current string using the given encoding. */
 my_encoding = DeeString_UtfEnc(self);
 if (my_encoding == codec)
     return_reference_((DeeObject *)self);
 str  = DeeString_UtfStr(self);
 len  = ENCODING_SIZE(str);
 len /= DeeEnc_Size(my_encoding);
 len *= DeeEnc_Size(codec);
 return DeeString_NewSizedWithEncoding(str,len,
                                       UNICODE_ENC(codec,
                                                   errors_mode));
}
#endif


PRIVATE DREF DeeObject *DCALL
string_front(String *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 void *str = DeeString_UtfStr(self);
 int enc = DeeString_UtfEnc(self);
 if (DeeArg_Unpack(argc,argv,":front"))
     return NULL;
 if unlikely(!ENCODING_SIZE(str)) {
  err_empty_sequence((DeeObject *)self);
  return NULL;
 }
 return DeeString_NewChar(DeeEnc_Get(enc,str,0));
}

PRIVATE DREF DeeObject *DCALL
string_back(String *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
 void *str = DeeString_UtfStr(self);
 int enc = DeeString_UtfEnc(self);
 size_t length = ENCODING_SIZE(str);
 if (DeeArg_Unpack(argc,argv,":front"))
     return NULL;
 if unlikely(!length) {
  err_empty_sequence((DeeObject *)self);
  return NULL;
 }
 return DeeString_NewChar(DeeEnc_Get(enc,str,length-1));
}

PRIVATE DREF DeeObject *DCALL
string_center(String *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 void *str = DeeString_UtfStr(self);
 int enc = DeeString_UtfEnc(self);
 size_t width,length = ENCODING_SIZE(str);
 size_t fill_front,fill_back,encsize;
 DeeObject *filler_ob = NULL; uint8_t *dst;
#ifndef CONFIG_USE_NEW_STRING_API
 DREF String *result;
#else
 uint8_t *result;
#endif
 if (DeeArg_Unpack(argc,argv,"Iu|o:center",&width,&filler_ob))
     goto err;
 if (width <= length) return_reference_((DeeObject *)self);
#ifdef CONFIG_USE_NEW_STRING_API
 dst = result = (uint8_t *)DeeString_NewWidthBuffer(width,enc);
 if unlikely(!result) goto err;
 encsize     = DeeEnc_Size(enc);
 fill_front  = (width-length);
 fill_back   = fill_front/2;
 fill_front -= fill_back;
 if (filler_ob) {
  void *filler_str; size_t filler_len;
  filler_str = DeeString_AsEncoding(filler_ob,enc);
  if unlikely(!filler_str) goto err_r;
  filler_len = ENCODING_SIZE(filler_str);
  if unlikely(!filler_len) {
   DeeError_Throwf(&DeeError_ValueError,
                   "Empty filler");
   goto err_r;
  }
  length     *= encsize;
  fill_front *= encsize;
  fill_back  *= encsize;
  filler_len *= encsize;
  while (fill_front >= filler_len) {
   memcpy(dst,filler_str,filler_len);
   dst        += filler_len;
   fill_front -= filler_len;
  }
  memcpy(dst,filler_str,fill_front);
  dst += fill_front;
  memcpy(dst,str,length);
  dst += length;
  while (fill_back >= filler_len) {
   memcpy(dst,filler_str,filler_len);
   dst       += filler_len;
   fill_back -= filler_len;
  }
  memcpy(dst,filler_str,fill_back);
 } else {
  /* Without filler. */
  SWITCH_ENC_SIZE(enc) {
  CASE_ENC_1BYTE:
   memsetb((uint8_t *)dst,' ',fill_front);
   dst += fill_front;
   memcpyb((uint8_t *)dst,(uint8_t *)str,length);
   dst += length;
   memsetb((uint8_t *)dst,' ',fill_back);
   break;             
  CASE_ENC_2BYTE:
   memsetw((uint16_t *)dst,' ',fill_front);
   dst += fill_front*2;
   memcpyw((uint16_t *)dst,(uint16_t *)str,length);
   dst += length*2;
   memsetw((uint16_t *)dst,' ',fill_back);
   break;             
  CASE_ENC_4BYTE:
   memsetl((uint32_t *)dst,' ',fill_front);
   dst += fill_front*4;
   memcpyl((uint32_t *)dst,(uint32_t *)str,length);
   dst += length*4;
   memsetl((uint32_t *)dst,' ',fill_back);
   break;             
  }
 }
 return DeeString_PackWidthBuffer(result,enc,DEE_STRING_CODEC_FIGNORE);
err_r:
 DeeString_FreeWidthBuffer(result,enc);
err:
 return NULL;
#else
 result = (DREF String *)DeeString_NewEncodingBuffer(width,enc,(void **)&dst);
 if unlikely(!result) goto err;
 encsize     = DeeEnc_Size(enc);
 fill_front  = (width-length);
 fill_back   = fill_front/2;
 fill_front -= fill_back;
 if (filler_ob) {
  void *filler_str; size_t filler_len;
  filler_str = DeeString_AsEncoding(filler_ob,enc);
  if unlikely(!filler_str) goto err_r;
  filler_len = ENCODING_SIZE(filler_str);
  if unlikely(!filler_len) {
   DeeError_Throwf(&DeeError_ValueError,
                   "Empty filler");
   goto err_r;
  }
  length     *= encsize;
  fill_front *= encsize;
  fill_back  *= encsize;
  filler_len *= encsize;
  while (fill_front >= filler_len) {
   memcpy(dst,filler_str,filler_len);
   dst        += filler_len;
   fill_front -= filler_len;
  }
  memcpy(dst,filler_str,fill_front);
  dst += fill_front;
  memcpy(dst,str,length);
  dst += length;
  while (fill_back >= filler_len) {
   memcpy(dst,filler_str,filler_len);
   dst       += filler_len;
   fill_back -= filler_len;
  }
  memcpy(dst,filler_str,fill_back);
 } else {
  /* Without filler. */
  SWITCH_ENC_SIZE(enc) {
  default:
   memsetb((uint8_t *)dst,' ',fill_front);
   dst += fill_front;
   memcpyb((uint8_t *)dst,(uint8_t *)str,length);
   dst += length;
   memsetb((uint8_t *)dst,' ',fill_back);
   break;             
  CASE_ENC_2BYTE:
   memsetw((uint16_t *)dst,' ',fill_front);
   dst += fill_front*2;
   memcpyw((uint16_t *)dst,(uint16_t *)str,length);
   dst += length*2;
   memsetw((uint16_t *)dst,' ',fill_back);
   break;             
  CASE_ENC_4BYTE:
   memsetl((uint32_t *)dst,' ',fill_front);
   dst += fill_front*4;
   memcpyl((uint32_t *)dst,(uint32_t *)str,length);
   dst += length*4;
   memsetl((uint32_t *)dst,' ',fill_back);
   break;             
  }
 }
 /* Set the encoding buffer. */
 return DeeString_SetEncodingBuffer((DREF DeeObject *)result);
err_r:
 Dee_Decref(result);
err:
 return NULL;
#endif
}
PRIVATE DREF DeeObject *DCALL
string_ljust(String *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 void *str = DeeString_UtfStr(self);
 int enc = DeeString_UtfEnc(self);
 size_t width,length = ENCODING_SIZE(str);
 size_t fill_width,encsize;
 DeeObject *filler_ob = NULL; uint8_t *dst;
#ifndef CONFIG_USE_NEW_STRING_API
 DREF String *result;
#else
 uint8_t *result;
#endif
 if (DeeArg_Unpack(argc,argv,"Iu|o:ljust",&width,&filler_ob))
     goto err;
 if (width <= length) return_reference_((DeeObject *)self);
#ifdef CONFIG_USE_NEW_STRING_API
 dst = result = (uint8_t *)DeeString_NewWidthBuffer(width,enc);
 if unlikely(!result) goto err;
 encsize    = DeeEnc_Size(enc);
 fill_width = (width-length);
 if (filler_ob) {
  void *filler_str; size_t filler_len;
  filler_str = DeeString_AsEncoding(filler_ob,enc);
  if unlikely(!filler_str) goto err_r;
  filler_len = ENCODING_SIZE(filler_str);
  if unlikely(!filler_len) {
   DeeError_Throwf(&DeeError_ValueError,
                   "Empty filler");
   goto err_r;
  }
  length     *= encsize;
  fill_width *= encsize;
  filler_len *= encsize;
  memcpy(dst,str,length);
  dst += length;
  while (fill_width >= filler_len) {
   memcpy(dst,filler_str,filler_len);
   dst        += filler_len;
   fill_width -= filler_len;
  }
  memcpy(dst,filler_str,fill_width);
 } else {
  /* Without filler. */
  SWITCH_ENC_SIZE(enc) {
  default:
   memcpyb((uint8_t *)dst,(uint8_t *)str,length);
   dst += length;
   memsetb((uint8_t *)dst,' ',fill_width);
   break;             
  CASE_ENC_2BYTE:
   memcpyw((uint16_t *)dst,(uint16_t *)str,length);
   dst += length*2;
   memsetw((uint16_t *)dst,' ',fill_width);
   break;             
  CASE_ENC_4BYTE:
   memcpyl((uint32_t *)dst,(uint32_t *)str,length);
   dst += length*4;
   memsetl((uint32_t *)dst,' ',fill_width);
   break;             
  }
 }
 /* Set the encoding buffer. */
 return DeeString_PackWidthBuffer(result,enc,DEE_STRING_CODEC_FIGNORE);
err_r:
 Dee_Free(result);
err:
 return NULL;
#else
 result = (DREF String *)DeeString_NewEncodingBuffer(width,enc,(void **)&dst);
 if unlikely(!result) goto err;
 encsize    = DeeEnc_Size(enc);
 fill_width = (width-length);
 if (filler_ob) {
  void *filler_str; size_t filler_len;
  filler_str = DeeString_AsEncoding(filler_ob,enc);
  if unlikely(!filler_str) goto err_r;
  filler_len = ENCODING_SIZE(filler_str);
  if unlikely(!filler_len) {
   DeeError_Throwf(&DeeError_ValueError,
                   "Empty filler");
   goto err_r;
  }
  length     *= encsize;
  fill_width *= encsize;
  filler_len *= encsize;
  memcpy(dst,str,length);
  dst += length;
  while (fill_width >= filler_len) {
   memcpy(dst,filler_str,filler_len);
   dst        += filler_len;
   fill_width -= filler_len;
  }
  memcpy(dst,filler_str,fill_width);
 } else {
  /* Without filler. */
  SWITCH_ENC_SIZE(enc) {
  default:
   memcpyb((uint8_t *)dst,(uint8_t *)str,length);
   dst += length;
   memsetb((uint8_t *)dst,' ',fill_width);
   break;             
  CASE_ENC_2BYTE:
   memcpyw((uint16_t *)dst,(uint16_t *)str,length);
   dst += length*2;
   memsetw((uint16_t *)dst,' ',fill_width);
   break;             
  CASE_ENC_4BYTE:
   memcpyl((uint32_t *)dst,(uint32_t *)str,length);
   dst += length*4;
   memsetl((uint32_t *)dst,' ',fill_width);
   break;             
  }
 }
 /* Set the encoding buffer. */
 return DeeString_SetEncodingBuffer((DREF DeeObject *)result);
err_r:
 Dee_Decref(result);
err:
 return NULL;
#endif
}
PRIVATE DREF DeeObject *DCALL
string_rjust(String *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 void *str = DeeString_UtfStr(self);
 int enc = DeeString_UtfEnc(self);
 size_t width,length = ENCODING_SIZE(str);
 size_t fill_width,encsize;
 DeeObject *filler_ob = NULL; uint8_t *dst;
#ifndef CONFIG_USE_NEW_STRING_API
 DREF String *result;
#else
 uint8_t *result;
#endif
 if (DeeArg_Unpack(argc,argv,"Iu|o:rjust",&width,&filler_ob))
     goto err;
 if (width <= length) return_reference_((DeeObject *)self);
#ifdef CONFIG_USE_NEW_STRING_API
 dst = result = (uint8_t *)DeeString_NewWidthBuffer(width,enc);
 if unlikely(!dst) goto err;
 encsize    = DeeEnc_Size(enc);
 fill_width = (width-length);
 if (filler_ob) {
  void *filler_str; size_t filler_len;
  filler_str = DeeString_AsEncoding(filler_ob,enc);
  if unlikely(!filler_str) goto err_r;
  filler_len = ENCODING_SIZE(filler_str);
  if unlikely(!filler_len) {
   DeeError_Throwf(&DeeError_ValueError,
                   "Empty filler");
   goto err_r;
  }
  length     *= encsize;
  fill_width *= encsize;
  filler_len *= encsize;
  while (fill_width >= filler_len) {
   memcpy(dst,filler_str,filler_len);
   dst        += filler_len;
   fill_width -= filler_len;
  }
  memcpy(dst,filler_str,fill_width);
  dst += fill_width;
  memcpy(dst,str,length);
 } else {
  /* Without filler. */
  SWITCH_ENC_SIZE(enc) {
  default:
   memsetb((uint8_t *)dst,' ',fill_width);
   dst += fill_width;
   memcpyb((uint8_t *)dst,(uint8_t *)str,length);
   break;             
  CASE_ENC_2BYTE:
   memsetw((uint16_t *)dst,' ',fill_width);
   dst += fill_width*2;
   memcpyw((uint16_t *)dst,(uint16_t *)str,length);
   break;             
  CASE_ENC_4BYTE:
   memsetl((uint32_t *)dst,' ',fill_width);
   dst += fill_width*4;
   memcpyl((uint32_t *)dst,(uint32_t *)str,length);
   break;             
  }
 }
 /* Set the encoding buffer. */
 return DeeString_PackWidthBuffer(result,enc,DEE_STRING_CODEC_FIGNORE);
err_r:
 Dee_Free(result);
err:
 return NULL;
#else
 result = (DREF String *)DeeString_NewEncodingBuffer(width,enc,(void **)&dst);
 if unlikely(!result) goto err;
 encsize    = DeeEnc_Size(enc);
 fill_width = (width-length);
 if (filler_ob) {
  void *filler_str; size_t filler_len;
  filler_str = DeeString_AsEncoding(filler_ob,enc);
  if unlikely(!filler_str) goto err_r;
  filler_len = ENCODING_SIZE(filler_str);
  if unlikely(!filler_len) {
   DeeError_Throwf(&DeeError_ValueError,
                   "Empty filler");
   goto err_r;
  }
  length     *= encsize;
  fill_width *= encsize;
  filler_len *= encsize;
  while (fill_width >= filler_len) {
   memcpy(dst,filler_str,filler_len);
   dst        += filler_len;
   fill_width -= filler_len;
  }
  memcpy(dst,filler_str,fill_width);
  dst += fill_width;
  memcpy(dst,str,length);
 } else {
  /* Without filler. */
  SWITCH_ENC_SIZE(enc) {
  default:
   memsetb((uint8_t *)dst,' ',fill_width);
   dst += fill_width;
   memcpyb((uint8_t *)dst,(uint8_t *)str,length);
   break;             
  CASE_ENC_2BYTE:
   memsetw((uint16_t *)dst,' ',fill_width);
   dst += fill_width*2;
   memcpyw((uint16_t *)dst,(uint16_t *)str,length);
   break;             
  CASE_ENC_4BYTE:
   memsetl((uint32_t *)dst,' ',fill_width);
   dst += fill_width*4;
   memcpyl((uint32_t *)dst,(uint32_t *)str,length);
   break;             
  }
 }
 /* Set the encoding buffer. */
 return DeeString_SetEncodingBuffer((DREF DeeObject *)result);
err_r:
 Dee_Decref(result);
err:
 return NULL;
#endif
}
PRIVATE DREF DeeObject *DCALL
string_count(String *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 String *other; size_t begin = 0,end = (size_t)-1;
 dssize_t result;
 if (DeeArg_Unpack(argc,argv,"o|IdId:count",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     return NULL;
 result = DeeString_Count((DeeObject *)self,
                          (DeeObject *)other,
                           begin,end);
 if unlikely(result == -1) return NULL;
 return DeeInt_NewSSize(result);
}
PRIVATE DREF DeeObject *DCALL
string_contains_f(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 String *other; int result;
 size_t begin = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"o|IdId:contains",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     return NULL;
 result = DeeString_Contains((DeeObject *)self,
                             (DeeObject *)other,
                              begin,end);
 if unlikely(result < 0-1) return NULL;
 return_bool_(result);
}
PRIVATE DREF DeeObject *DCALL
string_casecontains_f(String *__restrict self,
                      size_t argc, DeeObject **__restrict argv) {
 String *other; int result;
 size_t begin = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"o|IdId:casecontains",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     return NULL;
 result = DeeString_CaseContains((DeeObject *)self,
                                 (DeeObject *)other,
                                  begin,end);
 if unlikely(result < 0-1) return NULL;
 return_bool_(result);
}
PRIVATE DREF DeeObject *DCALL
string_casecount(String *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 String *other; size_t begin = 0,end = (size_t)-1;
 dssize_t result;
 if (DeeArg_Unpack(argc,argv,"o|IdId:casecount",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     return NULL;
 result = DeeString_CaseCount((DeeObject *)self,
                              (DeeObject *)other,
                               begin,end);
 if unlikely(result == -1) return NULL;
 return DeeInt_NewSSize(result);
}
PRIVATE DREF DeeObject *DCALL
string_zfill(String *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 uint8_t *str = (uint8_t *)DeeString_UtfStr(self);
 int enc = DeeString_UtfEnc(self);
 size_t width,length = ENCODING_SIZE(str);
 size_t fill_width,encsize;
 DeeObject *filler_ob = NULL;
 uint8_t *dst;
#ifndef CONFIG_USE_NEW_STRING_API
 DREF String *result;
#else
 uint8_t *result;
#endif
 if (DeeArg_Unpack(argc,argv,"Iu|o:zfill",&width,&filler_ob))
     goto err;
 if (width <= length) return_reference_((DeeObject *)self);
#ifdef CONFIG_USE_NEW_STRING_API
 dst = result = (uint8_t *)DeeString_NewWidthBuffer(width,enc);
 if unlikely(!result) goto err;
 encsize    = DeeEnc_Size(enc);
 fill_width = (width-length);
 if (filler_ob) {
  void *filler_str; size_t filler_len;
  filler_str = DeeString_AsEncoding(filler_ob,enc);
  if unlikely(!filler_str) goto err_r;
  filler_len = ENCODING_SIZE(filler_str);
  if unlikely(!filler_len) {
   DeeError_Throwf(&DeeError_ValueError,
                   "Empty filler");
   goto err_r;
  }
  /* Copy leading mathematical signs. */
  while (length) {
   uint32_t ch = DeeEnc_Get(enc,str,0);
   if (ch != '+' && ch != '-') break;
   DeeEnc_Set(enc,dst,0,ch);
   dst += encsize;
   str += encsize;
   --length;
  }
  length     *= encsize;
  fill_width *= encsize;
  filler_len *= encsize;
  while (fill_width >= filler_len) {
   memcpy(dst,filler_str,filler_len);
   dst        += filler_len;
   fill_width -= filler_len;
  }
  memcpy(dst,filler_str,fill_width);
  dst += fill_width;
  memcpy(dst,str,length);
 } else {
  /* Without filler. */
  SWITCH_ENC_SIZE(enc) {
  default:
   while (*(uint8_t *)str == '+' ||
          *(uint8_t *)str == '-') {
    *(uint8_t *)dst = *(uint8_t *)str;
    dst += 1;
    str += 1;
    --length;
   }
   memsetb((uint8_t *)dst,'0',fill_width);
   dst += fill_width;
   memcpyb((uint8_t *)dst,(uint8_t *)str,length);
   break;             
  CASE_ENC_2BYTE:
   while (*(uint16_t *)str == '+' ||
          *(uint16_t *)str == '-') {
    *(uint16_t *)dst = *(uint16_t *)str;
    dst += 2;
    str += 2;
    --length;
   }
   memsetw((uint16_t *)dst,'0',fill_width);
   dst += fill_width*2;
   memcpyw((uint16_t *)dst,(uint16_t *)str,length);
   break;             
  CASE_ENC_4BYTE:
   while (*(uint32_t *)str == '+' ||
          *(uint32_t *)str == '-') {
    *(uint32_t *)dst = *(uint32_t *)str;
    dst += 4;
    str += 4;
    --length;
   }
   memsetl((uint32_t *)dst,'0',fill_width);
   dst += fill_width*4;
   memcpyl((uint32_t *)dst,(uint32_t *)str,length);
   break;             
  }
 }
 /* Set the encoding buffer. */
 return DeeString_PackWidthBuffer(result,enc,DEE_STRING_CODEC_FIGNORE);
err_r:
 Dee_Free(result);
err:
 return NULL;
#else
 result = (DREF String *)DeeString_NewEncodingBuffer(width,enc,(void **)&dst);
 if unlikely(!result) goto err;
 encsize    = DeeEnc_Size(enc);
 fill_width = (width-length);
 if (filler_ob) {
  void *filler_str; size_t filler_len;
  filler_str = DeeString_AsEncoding(filler_ob,enc);
  if unlikely(!filler_str) goto err_r;
  filler_len = ENCODING_SIZE(filler_str);
  if unlikely(!filler_len) {
   DeeError_Throwf(&DeeError_ValueError,
                   "Empty filler");
   goto err_r;
  }
  /* Copy leading mathematical signs. */
  while (length) {
   uint32_t ch = DeeEnc_Get(enc,str,0);
   if (ch != '+' && ch != '-') break;
   DeeEnc_Set(enc,dst,0,ch);
   dst += encsize;
   str += encsize;
   --length;
  }
  length     *= encsize;
  fill_width *= encsize;
  filler_len *= encsize;
  while (fill_width >= filler_len) {
   memcpy(dst,filler_str,filler_len);
   dst        += filler_len;
   fill_width -= filler_len;
  }
  memcpy(dst,filler_str,fill_width);
  dst += fill_width;
  memcpy(dst,str,length);
 } else {
  /* Without filler. */
  SWITCH_ENC_SIZE(enc) {
  default:
   while (*(uint8_t *)str == '+' ||
          *(uint8_t *)str == '-') {
    *(uint8_t *)dst = *(uint8_t *)str;
    dst += 1;
    str += 1;
    --length;
   }
   memsetb((uint8_t *)dst,'0',fill_width);
   dst += fill_width;
   memcpyb((uint8_t *)dst,(uint8_t *)str,length);
   break;             
  CASE_ENC_2BYTE:
   while (*(uint16_t *)str == '+' ||
          *(uint16_t *)str == '-') {
    *(uint16_t *)dst = *(uint16_t *)str;
    dst += 2;
    str += 2;
    --length;
   }
   memsetw((uint16_t *)dst,'0',fill_width);
   dst += fill_width*2;
   memcpyw((uint16_t *)dst,(uint16_t *)str,length);
   break;             
  CASE_ENC_4BYTE:
   while (*(uint32_t *)str == '+' ||
          *(uint32_t *)str == '-') {
    *(uint32_t *)dst = *(uint32_t *)str;
    dst += 4;
    str += 4;
    --length;
   }
   memsetl((uint32_t *)dst,'0',fill_width);
   dst += fill_width*4;
   memcpyl((uint32_t *)dst,(uint32_t *)str,length);
   break;             
  }
 }
 /* Set the encoding buffer. */
 return DeeString_SetEncodingBuffer((DREF DeeObject *)result);
err_r:
 Dee_Decref(result);
err:
 return NULL;
#endif
}
PRIVATE DREF DeeObject *DCALL
string_reversed(String *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
 size_t begin = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"|IdId:reversed",&begin,&end))
     return NULL;
 return DeeString_Reversed((DeeObject *)self,begin,end);
}
PRIVATE DREF DeeObject *DCALL
string_expandtabs(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 size_t tab_width = 8;
 if (DeeArg_Unpack(argc,argv,"|Iu:expandtabs",&tab_width))
     return NULL;
 return DeeString_ExpandTabs((DeeObject *)self,tab_width);
}
PRIVATE DREF DeeObject *DCALL
string_join(String *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
 DeeObject *items;
 if (DeeArg_Unpack(argc,argv,"o:join",&items))
     return NULL;
 return DeeString_Join((DeeObject *)self,items);
}
PRIVATE DREF DeeObject *DCALL
string_unifylines(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 DeeObject *replacement = NULL;
 if (DeeArg_Unpack(argc,argv,"|o:unifylines",&replacement))
     return NULL;
 if likely(!replacement)
    return DeeString_UnifyLinesLf((DeeObject *)self);
 return DeeString_UnifyLines((DeeObject *)self,replacement);
}
PRIVATE DREF DeeObject *DCALL
string_dopartat(String *__restrict self,
                String *__restrict other,
                size_t start, size_t end,
                dssize_t partpos) {
 DREF DeeObject *result;
 DREF String *a,*b,*c;
 if (partpos < 0) {
  void *str = DeeString_UtfStr(self);
  size_t len = ENCODING_SIZE(str);
  if (start == 0 && end >= len) {
   a = self;
   Dee_Incref(a);
  } else {
   if (end >= len) end = len;
   if (start >= end) {
    a = (DREF String *)Dee_EmptyString;
    Dee_Incref(Dee_EmptyString);
   } else {
    int enc = DeeString_UtfEnc(self);
    a = (DREF String *)DeeString_NewSizedWithEncoding((uint8_t *)str+
                                                      (start*DeeEnc_Size(enc)),
                                                       end-(size_t)start,enc);
   }
  }
  b = c = (DREF String *)Dee_EmptyString;
#ifdef CONFIG_NO_THREADS
  Dee_EmptyString->ob_refcnt += 2;
#else
  ATOMIC_FETCHADD(Dee_EmptyString->ob_refcnt,2);
#endif
 } else {
  uint8_t *str_self,*str_other; int enc;
  size_t self_len;
  enc       = DeeString_UtfEnc(self);
  str_self  = (uint8_t *)DeeString_UtfStr(self);
  str_other = (uint8_t *)DeeString_AsEncoding((DeeObject *)other,enc);
  if unlikely(!str_other) goto err;
  self_len  = ENCODING_SIZE(str_self);
  if (end > self_len) end = self_len;
  str_self += start;
  self_len  = end-start;
  ASSERT((size_t)partpos+ENCODING_SIZE(str_other) <= self_len);
  ASSERT((size_t)partpos >= start);
  partpos -= start;
  a = (DREF String *)DeeString_NewSizedWithEncoding(str_self,(size_t)partpos,enc);
  if unlikely(!a) goto err;
  partpos += ENCODING_SIZE(str_other);
  c = (DREF String *)DeeString_NewSizedWithEncoding((uint8_t *)str_self+
                                                   ((size_t)partpos*DeeEnc_Size(enc)),
                                                     self_len-(size_t)partpos,
                                                     enc);
  if unlikely(!c) goto err_a;
  b = other;
  Dee_Incref(b);
 }
 result = DeeTuple_PackSymbolic(3,a,b,c);
 if likely(result) return result;
 Dee_Decref(c);
 Dee_Decref(b);
err_a:
 Dee_Decref(a);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
string_parition(String *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
 DeeObject *other; dssize_t partpos;
 size_t begin = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"o|IdId:parition",&other,&begin,&end) ||
     DeeObject_AssertTypeExact(other,&DeeString_Type))
     goto err;
 partpos = DeeString_Find((DeeObject *)self,other,begin,end);
 if unlikely(partpos == -2) goto err;
 return string_dopartat(self,(String *)other,begin,end,partpos);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
string_rparition(String *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 DeeObject *other; dssize_t partpos;
 size_t begin = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"o|IdId:rparition",&other,&begin,&end) ||
     DeeObject_AssertTypeExact(other,&DeeString_Type))
     goto err;
 partpos = DeeString_RFind((DeeObject *)self,other,begin,end);
 if unlikely(partpos == -2) goto err;
 return string_dopartat(self,(String *)other,begin,end,partpos);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
string_caseparition(String *__restrict self,
                    size_t argc, DeeObject **__restrict argv) {
 DeeObject *other; dssize_t partpos;
 size_t begin = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"o|IdId:caseparition",&other,&begin,&end) ||
     DeeObject_AssertTypeExact(other,&DeeString_Type))
     goto err;
 partpos = DeeString_CaseFind((DeeObject *)self,other,begin,end);
 if unlikely(partpos == -2) goto err;
 return string_dopartat(self,(String *)other,begin,end,partpos);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
string_caserparition(String *__restrict self,
                     size_t argc, DeeObject **__restrict argv) {
 DeeObject *other; dssize_t partpos;
 size_t begin = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"o|IdId:caserparition",&other,&begin,&end) ||
     DeeObject_AssertTypeExact(other,&DeeString_Type))
     goto err;
 partpos = DeeString_CaseRFind((DeeObject *)self,other,begin,end);
 if unlikely(partpos == -2) goto err;
 return string_dopartat(self,(String *)other,begin,end,partpos);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_sstrip(String *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 DeeObject *other;
 if (DeeArg_Unpack(argc,argv,"o:sstrip",&other) ||
     DeeObject_AssertTypeExact(other,&DeeString_Type))
     return NULL;
 return DeeString_SStrip((DeeObject *)self,other);
}
PRIVATE DREF DeeObject *DCALL
string_lsstrip(String *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
 DeeObject *other;
 if (DeeArg_Unpack(argc,argv,"o:lsstrip",&other) ||
     DeeObject_AssertTypeExact(other,&DeeString_Type))
     return NULL;
 return DeeString_LSStrip((DeeObject *)self,other);
}
PRIVATE DREF DeeObject *DCALL
string_rsstrip(String *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
 DeeObject *other;
 if (DeeArg_Unpack(argc,argv,"o:rsstrip",&other) ||
     DeeObject_AssertTypeExact(other,&DeeString_Type))
     return NULL;
 return DeeString_RSStrip((DeeObject *)self,other);
}
PRIVATE DREF DeeObject *DCALL
string_casesstrip(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 DeeObject *other;
 if (DeeArg_Unpack(argc,argv,"o:casesstrip",&other) ||
     DeeObject_AssertTypeExact(other,&DeeString_Type))
     return NULL;
 return DeeString_CaseSStrip((DeeObject *)self,other);
}
PRIVATE DREF DeeObject *DCALL
string_caselsstrip(String *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
 DeeObject *other;
 if (DeeArg_Unpack(argc,argv,"o:caselsstrip",&other) ||
     DeeObject_AssertTypeExact(other,&DeeString_Type))
     return NULL;
 return DeeString_CaseLSStrip((DeeObject *)self,other);
}
PRIVATE DREF DeeObject *DCALL
string_casersstrip(String *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
 DeeObject *other;
 if (DeeArg_Unpack(argc,argv,"o:casersstrip",&other) ||
     DeeObject_AssertTypeExact(other,&DeeString_Type))
     return NULL;
 return DeeString_CaseRSStrip((DeeObject *)self,other);
}

struct compare_args {
 DeeObject *other;
 size_t     my_start;
 size_t     my_end;
 size_t     ot_start;
 size_t     ot_end;
};

PRIVATE int DCALL
get_compare_args(struct compare_args *__restrict args,
                 size_t argc, DeeObject **__restrict argv,
                 char const *__restrict funname) {
 args->my_start = 0;
 args->my_end   = (size_t)-1;
 args->ot_start = 0;
 args->ot_end   = (size_t)-1;
 switch (argc) {
 case 1: args->other = argv[0]; break;
 case 2:
  if (DeeString_Check(argv[0])) {
   args->other = argv[0];
   if (DeeObject_AsSSize(argv[1],(dssize_t *)&args->ot_start)) goto err;
  } else {
   if (DeeObject_AsSSize(argv[0],(dssize_t *)&args->my_start)) goto err;
   args->other = argv[1];
  }
  break;
 case 3:
  if (DeeString_Check(argv[0])) {
   args->other = argv[0];
   if (DeeObject_AsSSize(argv[1],(dssize_t *)&args->ot_start)) goto err;
   if (DeeObject_AsSSize(argv[2],(dssize_t *)&args->ot_end)) goto err;
  } else if (DeeString_Check(argv[1])) {
   if (DeeObject_AsSSize(argv[0],(dssize_t *)&args->my_start)) goto err;
   args->other = argv[1];
   if (DeeObject_AsSSize(argv[2],(dssize_t *)&args->ot_start)) goto err;
  } else {
   if (DeeObject_AsSSize(argv[0],(dssize_t *)&args->my_start)) goto err;
   if (DeeObject_AsSSize(argv[1],(dssize_t *)&args->my_end)) goto err;
   args->other = argv[2];
  }
  break;
 case 4:
  if (DeeObject_AsSSize(argv[0],(dssize_t *)&args->my_start)) goto err;
  if (DeeString_Check(argv[1])) {
   args->other = argv[1];
   if (DeeObject_AsSSize(argv[2],(dssize_t *)&args->ot_start)) goto err;
   if (DeeObject_AsSSize(argv[3],(dssize_t *)&args->ot_end)) goto err;
  } else {
   if (DeeObject_AsSSize(argv[1],(dssize_t *)&args->my_end)) goto err;
   args->other = argv[2];
   if (DeeObject_AsSSize(argv[3],(dssize_t *)&args->ot_start)) goto err;
  }
  break;
 case 5:
  if (DeeObject_AsSSize(argv[0],(dssize_t *)&args->my_start)) goto err;
  if (DeeObject_AsSSize(argv[1],(dssize_t *)&args->my_end)) goto err;
  args->other = argv[2];
  if (DeeObject_AsSSize(argv[3],(dssize_t *)&args->ot_start)) goto err;
  if (DeeObject_AsSSize(argv[4],(dssize_t *)&args->ot_end)) goto err;
  break;
 default:
  err_invalid_argc(funname,argc,1,5);
  goto err;
 }
 if (DeeObject_AssertTypeExact(args->other,&DeeString_Type))
     goto err;
 return 0;
err:
 return -1;
}


PRIVATE DREF DeeObject *DCALL
string_compare(String *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
 uint8_t *str_self,*str_other; int enc;
 size_t self_len,other_len; int64_t result;
 struct compare_args args;
 if (get_compare_args(&args,argc,argv,"compare"))
     goto err;
 enc       = DeeString_UtfEnc(self);
 str_self  = (uint8_t *)DeeString_UtfStr(self);
 str_other = (uint8_t *)DeeString_AsEncoding((DeeObject *)args.other,enc);
 if unlikely(!str_other) goto err;
 self_len  = ENCODING_SIZE(str_self);
 other_len = ENCODING_SIZE(str_other);
 if (args.my_end > self_len)
     args.my_end = self_len;
 if (args.my_end <= args.my_start) self_len = 0;
 else str_self += args.my_start,
      self_len  = args.my_end-args.my_start;
 if (args.ot_end > other_len)
     args.ot_end = other_len;
 if (args.ot_end <= args.ot_start) other_len = 0;
 else str_other += args.ot_start,
      other_len  = args.ot_end-args.ot_start;
 SWITCH_ENC_SIZE(enc) {
 default:
  while (other_len && self_len) {
   uint8_t a = *(uint8_t *)str_self;
   uint8_t b = *(uint8_t *)str_other;
   if (a != b) { result = (int16_t)a-(int16_t)b; goto done; }
   str_self += 1,--self_len;
   str_other += 1,--other_len;
  }
  /* */if (other_len) result = (int16_t)0-(int16_t)*(uint8_t *)str_other;
  else if (self_len) result = (int16_t)*(uint8_t *)str_self/*-0*/;
  else result = 0;
  break;
 CASE_ENC_2BYTE:
  while (other_len && self_len) {
   uint16_t a = *(uint16_t *)str_self;
   uint16_t b = *(uint16_t *)str_other;
   if (a != b) { result = (int32_t)a-(int32_t)b; goto done; }
   str_self += 2,--self_len;
   str_other += 2,--other_len;
  }
  /* */if (other_len) result = (int32_t)0-(int32_t)*(uint16_t *)str_other;
  else if (self_len) result = (int32_t)*(uint16_t *)str_self/*-0*/;
  else result = 0;
  break;
 CASE_ENC_4BYTE:
  while (other_len && self_len) {
   uint32_t a = *(uint32_t *)str_self;
   uint32_t b = *(uint32_t *)str_other;
   if (a != b) { result = (int64_t)a-(int64_t)b; goto done; }
   str_self += 4,--self_len;
   str_other += 4,--other_len;
  }
  /* */if (other_len) result = (int64_t)0-(int64_t)*(uint32_t *)str_other;
  else if (self_len) result = (int64_t)*(uint32_t *)str_self/*-0*/;
  else result = 0;
  break;
 }
done:
 return DeeInt_NewS64(result);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_vercompare(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 uint8_t *str_self,*str_other; int enc;
 size_t self_len,other_len; int32_t result;
 struct compare_args args;
 if (get_compare_args(&args,argc,argv,"vercompare"))
     goto err;
 enc       = DeeString_UtfEnc(self);
 str_self  = (uint8_t *)DeeString_UtfStr(self);
 str_other = (uint8_t *)DeeString_AsEncoding((DeeObject *)args.other,enc);
 if unlikely(!str_other) goto err;
 self_len  = ENCODING_SIZE(str_self);
 other_len = ENCODING_SIZE(str_other);
 if (args.my_end > self_len)
     args.my_end = self_len;
 if (args.my_end <= args.my_start) self_len = 0;
 else str_self += args.my_start,
      self_len  = args.my_end-args.my_start;
 if (args.ot_end > other_len)
     args.ot_end = other_len;
 if (args.ot_end <= args.ot_start) other_len = 0;
 else str_other += args.ot_start,
      other_len  = args.ot_end-args.ot_start;
 SWITCH_ENC_SIZE(enc) {
 default:
  result = dee_strverscmpb((uint8_t *)str_self,self_len,(uint8_t *)str_other,other_len);
  break;
 CASE_ENC_2BYTE:
  result = dee_strverscmpw((uint16_t *)str_self,self_len,(uint16_t *)str_other,other_len);
  break;
 CASE_ENC_4BYTE:
  result = dee_strverscmpl((uint32_t *)str_self,self_len,(uint32_t *)str_other,other_len);
  break;
 }
 return DeeInt_NewS32(result);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
string_casevercompare(String *__restrict self,
                      size_t argc, DeeObject **__restrict argv) {
 uint8_t *str_self,*str_other; int enc;
 size_t self_len,other_len; int32_t result;
 struct compare_args args;
 if (get_compare_args(&args,argc,argv,"casevercompare"))
     goto err;
 enc       = DeeString_UtfEnc(self);
 str_self  = (uint8_t *)DeeString_UtfStr(self);
 str_other = (uint8_t *)DeeString_AsEncoding((DeeObject *)args.other,enc);
 if unlikely(!str_other) goto err;
 self_len  = ENCODING_SIZE(str_self);
 other_len = ENCODING_SIZE(str_other);
 if (args.my_end > self_len)
     args.my_end = self_len;
 if (args.my_end <= args.my_start) self_len = 0;
 else str_self += args.my_start,
      self_len  = args.my_end-args.my_start;
 if (args.ot_end > other_len)
     args.ot_end = other_len;
 if (args.ot_end <= args.ot_start) other_len = 0;
 else str_other += args.ot_start,
      other_len  = args.ot_end-args.ot_start;
 SWITCH_ENC_SIZE(enc) {
 default:
  result = dee_strcaseverscmpb((uint8_t *)str_self,self_len,(uint8_t *)str_other,other_len);
  break;
 CASE_ENC_2BYTE:
  result = dee_strcaseverscmpw((uint16_t *)str_self,self_len,(uint16_t *)str_other,other_len);
  break;
 CASE_ENC_4BYTE:
  result = dee_strcaseverscmpl((uint32_t *)str_self,self_len,(uint32_t *)str_other,other_len);
  break;
 }
 return DeeInt_NewS32(result);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_fuzzycompare(String *__restrict self,
                    size_t argc, DeeObject **__restrict argv) {
 uint8_t *str_self,*str_other; int enc;
 size_t self_len,other_len; dssize_t result;
 struct compare_args args;
 if (get_compare_args(&args,argc,argv,"fuzzycompare"))
     goto err;
 enc       = DeeString_UtfEnc(self);
 str_self  = (uint8_t *)DeeString_UtfStr(self);
 str_other = (uint8_t *)DeeString_AsEncoding((DeeObject *)args.other,enc);
 if unlikely(!str_other) goto err;
 self_len  = ENCODING_SIZE(str_self);
 other_len = ENCODING_SIZE(str_other);
 if (args.my_end > self_len)
     args.my_end = self_len;
 if (args.my_end <= args.my_start) self_len = 0;
 else str_self += args.my_start,
      self_len  = args.my_end-args.my_start;
 if (args.ot_end > other_len)
     args.ot_end = other_len;
 if (args.ot_end <= args.ot_start) other_len = 0;
 else str_other += args.ot_start,
      other_len  = args.ot_end-args.ot_start;
 SWITCH_ENC_SIZE(enc) {
 default:
  result = fuzzy_compareb((uint8_t *)str_self,self_len,
                          (uint8_t *)str_other,other_len);
  break;
 CASE_ENC_2BYTE:
  result = fuzzy_comparew((uint16_t *)str_self,self_len,
                          (uint16_t *)str_other,other_len);
  break;
 CASE_ENC_4BYTE:
  result = fuzzy_comparel((uint32_t *)str_self,self_len,
                          (uint32_t *)str_other,other_len);
  break;
 }
 if unlikely(result < 0) goto err;
 return DeeInt_NewSize((size_t)result);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_casefuzzycompare(String *__restrict self,
                        size_t argc, DeeObject **__restrict argv) {
 uint8_t *str_self,*str_other; int enc;
 size_t self_len,other_len; dssize_t result;
 struct compare_args args;
 if (get_compare_args(&args,argc,argv,"casefuzzycompare"))
     goto err;
 enc       = DeeString_UtfEnc(self);
 str_self  = (uint8_t *)DeeString_UtfStr(self);
 str_other = (uint8_t *)DeeString_AsEncoding((DeeObject *)args.other,enc);
 if unlikely(!str_other) goto err;
 self_len  = ENCODING_SIZE(str_self);
 other_len = ENCODING_SIZE(str_other);
 if (args.my_end > self_len)
     args.my_end = self_len;
 if (args.my_end <= args.my_start) self_len = 0;
 else str_self += args.my_start,
      self_len  = args.my_end-args.my_start;
 if (args.ot_end > other_len)
     args.ot_end = other_len;
 if (args.ot_end <= args.ot_start) other_len = 0;
 else str_other += args.ot_start,
      other_len  = args.ot_end-args.ot_start;
 SWITCH_ENC_SIZE(enc) {
 default:
  result = fuzzy_casecompareb((uint8_t *)str_self,self_len,
                              (uint8_t *)str_other,other_len);
  break;
 CASE_ENC_2BYTE:
  result = fuzzy_casecomparew((uint16_t *)str_self,self_len,
                              (uint16_t *)str_other,other_len);
  break;
 CASE_ENC_4BYTE:
  result = fuzzy_casecomparel((uint32_t *)str_self,self_len,
                              (uint32_t *)str_other,other_len);
  break;
 }
 if unlikely(result < 0) goto err;
 return DeeInt_NewSize((size_t)result);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_casecompare(String *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
 uint8_t *str_self,*str_other; int enc;
 size_t self_len,other_len; int64_t result;
 struct compare_args args;
 if (get_compare_args(&args,argc,argv,"casecompare"))
     goto err;
 enc       = DeeString_UtfEnc(self);
 str_self  = (uint8_t *)DeeString_UtfStr(self);
 str_other = (uint8_t *)DeeString_AsEncoding((DeeObject *)args.other,enc);
 if unlikely(!str_other) goto err;
 self_len  = ENCODING_SIZE(str_self);
 other_len = ENCODING_SIZE(str_other);
 if (args.my_end > self_len)
     args.my_end = self_len;
 if (args.my_end <= args.my_start) self_len = 0;
 else str_self += args.my_start,
      self_len  = args.my_end-args.my_start;
 if (args.ot_end > other_len)
     args.ot_end = other_len;
 if (args.ot_end <= args.ot_start) other_len = 0;
 else str_other += args.ot_start,
      other_len  = args.ot_end-args.ot_start;
 SWITCH_ENC_SIZE(enc) {
 default:
  while (other_len && self_len) {
   uint8_t a = *(uint8_t *)str_self;
   uint8_t b = *(uint8_t *)str_other;
   if (a != b) {
    a = (uint8_t)DeeUni_ToLower(a);
    b = (uint8_t)DeeUni_ToLower(b);
    if (a != b) {
     result = (int16_t)a-(int16_t)b;
     goto done;
    }
   }
   str_self += 1,--self_len;
   str_other += 1,--other_len;
  }
  /* */if (other_len) result = (int16_t)0-(int16_t)*(uint8_t *)str_other;
  else if (self_len) result = (int16_t)*(uint8_t *)str_self/*-0*/;
  else result = 0;
  break;
 CASE_ENC_2BYTE:
  while (other_len && self_len) {
   uint16_t a = *(uint16_t *)str_self;
   uint16_t b = *(uint16_t *)str_other;
   if (a != b) {
    a = (uint16_t)DeeUni_ToLower(a);
    b = (uint16_t)DeeUni_ToLower(b);
    if (a != b) {
     result = (int32_t)a-(int32_t)b;
     goto done;
    }
   }
   str_self += 2,--self_len;
   str_other += 2,--other_len;
  }
  /* */if (other_len) result = (int32_t)0-(int32_t)*(uint16_t *)str_other;
  else if (self_len) result = (int32_t)*(uint16_t *)str_self/*-0*/;
  else result = 0;
  break;
 CASE_ENC_4BYTE:
  while (other_len && self_len) {
   uint32_t a = *(uint32_t *)str_self;
   uint32_t b = *(uint32_t *)str_other;
   if (a != b) {
    a = (uint32_t)DeeUni_ToLower(a);
    b = (uint32_t)DeeUni_ToLower(b);
    if (a != b) {
     result = (int64_t)a-(int64_t)b;
     goto done;
    }
   }
   str_self += 4,--self_len;
   str_other += 4,--other_len;
  }
  /* */if (other_len) result = (int64_t)0-(int64_t)*(uint32_t *)str_other;
  else if (self_len) result = (int64_t)*(uint32_t *)str_self/*-0*/;
  else result = 0;
  break;
 }
done:
 return DeeInt_NewS64(result);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_lcommon(String *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
 uint8_t *str_self,*str_other; int enc;
 size_t self_len,other_len,result = 0;
 struct compare_args args;
 if (get_compare_args(&args,argc,argv,"common"))
     goto err;
 enc       = DeeString_UtfEnc(self);
 str_self  = (uint8_t *)DeeString_UtfStr(self);
 str_other = (uint8_t *)DeeString_AsEncoding((DeeObject *)args.other,enc);
 if unlikely(!str_other) goto err;
 self_len  = ENCODING_SIZE(str_self);
 other_len = ENCODING_SIZE(str_other);
 if (args.my_end > self_len)
     args.my_end = self_len;
 if (args.my_end <= args.my_start) self_len = 0;
 else str_self += args.my_start,
      self_len  = args.my_end-args.my_start;
 if (args.ot_end > other_len)
     args.ot_end = other_len;
 if (args.ot_end <= args.ot_start) other_len = 0;
 else str_other += args.ot_start,
      other_len  = args.ot_end-args.ot_start;
 SWITCH_ENC_SIZE(enc) {
 default:
  while (other_len && self_len) {
   uint8_t a = *(uint8_t *)str_self;
   uint8_t b = *(uint8_t *)str_other;
   if (a != b) break;
   str_self += 1,--self_len;
   str_other += 1,--other_len;
   ++result;
  }
  break;
 CASE_ENC_2BYTE:
  while (other_len && self_len) {
   uint16_t a = *(uint16_t *)str_self;
   uint16_t b = *(uint16_t *)str_other;
   if (a != b) break;
   str_self += 2,--self_len;
   str_other += 2,--other_len;
   ++result;
  }
  break;
 CASE_ENC_4BYTE:
  while (other_len && self_len) {
   uint32_t a = *(uint32_t *)str_self;
   uint32_t b = *(uint32_t *)str_other;
   if (a != b) break;
   str_self += 4,--self_len;
   str_other += 4,--other_len;
   ++result;
  }
  break;
 }
 return DeeInt_NewSize(result);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_rcommon(String *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
 uint8_t *str_self,*str_other; int enc;
 size_t self_len,other_len,result = 0;
 struct compare_args args;
 if (get_compare_args(&args,argc,argv,"rcommon"))
     goto err;
 enc       = DeeString_UtfEnc(self);
 str_self  = (uint8_t *)DeeString_UtfStr(self);
 str_other = (uint8_t *)DeeString_AsEncoding((DeeObject *)args.other,enc);
 if unlikely(!str_other) goto err;
 self_len  = ENCODING_SIZE(str_self);
 other_len = ENCODING_SIZE(str_other);
 if (args.my_end > self_len)
     args.my_end = self_len;
 if (args.my_end <= args.my_start) self_len = 0;
 else str_self += args.my_start,
      self_len  = args.my_end-args.my_start;
 if (args.ot_end > other_len)
     args.ot_end = other_len;
 if (args.ot_end <= args.ot_start) other_len = 0;
 else str_other += args.ot_start,
      other_len  = args.ot_end-args.ot_start;
 SWITCH_ENC_SIZE(enc) {
 default:
  while (other_len && self_len) {
   uint8_t a,b;
   --self_len,--other_len;
   a = ((uint8_t *)str_self)[self_len];
   b = ((uint8_t *)str_other)[other_len];
   if (a != b) break;
   ++result;
  }
  break;
 CASE_ENC_2BYTE:
  while (other_len && self_len) {
   uint16_t a,b;
   --self_len,--other_len;
   a = ((uint16_t *)str_self)[self_len];
   b = ((uint16_t *)str_other)[other_len];
   if (a != b) break;
   ++result;
  }
  break;
 CASE_ENC_4BYTE:
  while (other_len && self_len) {
   uint32_t a,b;
   --self_len,--other_len;
   a = ((uint32_t *)str_self)[self_len];
   b = ((uint32_t *)str_other)[other_len];
   if (a != b) break;
   ++result;
  }
  break;
 }
 return DeeInt_NewSize(result);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_caselcommon(String *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
 uint8_t *str_self,*str_other; int enc;
 size_t self_len,other_len,result = 0;
 struct compare_args args;
 if (get_compare_args(&args,argc,argv,"casecommon"))
     goto err;
 enc       = DeeString_UtfEnc(self);
 str_self  = (uint8_t *)DeeString_UtfStr(self);
 str_other = (uint8_t *)DeeString_AsEncoding((DeeObject *)args.other,enc);
 if unlikely(!str_other) goto err;
 self_len  = ENCODING_SIZE(str_self);
 other_len = ENCODING_SIZE(str_other);
 if (args.my_end > self_len)
     args.my_end = self_len;
 if (args.my_end <= args.my_start) self_len = 0;
 else str_self += args.my_start,
      self_len  = args.my_end-args.my_start;
 if (args.ot_end > other_len)
     args.ot_end = other_len;
 if (args.ot_end <= args.ot_start) other_len = 0;
 else str_other += args.ot_start,
      other_len  = args.ot_end-args.ot_start;
 SWITCH_ENC_SIZE(enc) {
 default:
  while (other_len && self_len) {
   uint8_t a = *(uint8_t *)str_self;
   uint8_t b = *(uint8_t *)str_other;
   if (a != b) {
    a = (uint8_t)DeeUni_ToLower(a);
    b = (uint8_t)DeeUni_ToLower(b);
    if (a != b) break;
   }
   str_self += 1,--self_len;
   str_other += 1,--other_len;
   ++result;
  }
  break;
 CASE_ENC_2BYTE:
  while (other_len && self_len) {
   uint16_t a = *(uint16_t *)str_self;
   uint16_t b = *(uint16_t *)str_other;
   if (a != b) {
    a = (uint16_t)DeeUni_ToLower(a);
    b = (uint16_t)DeeUni_ToLower(b);
    if (a != b) break;
   }
   str_self += 2,--self_len;
   str_other += 2,--other_len;
   ++result;
  }
  break;
 CASE_ENC_4BYTE:
  while (other_len && self_len) {
   uint32_t a = *(uint32_t *)str_self;
   uint32_t b = *(uint32_t *)str_other;
   if (a != b) {
    a = (uint32_t)DeeUni_ToLower(a);
    b = (uint32_t)DeeUni_ToLower(b);
    if (a != b) break;
   }
   str_self += 4,--self_len;
   str_other += 4,--other_len;
   ++result;
  }
  break;
 }
 return DeeInt_NewSize(result);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_casercommon(String *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
 uint8_t *str_self,*str_other; int enc;
 size_t self_len,other_len,result = 0;
 struct compare_args args;
 if (get_compare_args(&args,argc,argv,"casercommon"))
     goto err;
 enc       = DeeString_UtfEnc(self);
 str_self  = (uint8_t *)DeeString_UtfStr(self);
 str_other = (uint8_t *)DeeString_AsEncoding((DeeObject *)args.other,enc);
 if unlikely(!str_other) goto err;
 self_len  = ENCODING_SIZE(str_self);
 other_len = ENCODING_SIZE(str_other);
 if (args.my_end > self_len)
     args.my_end = self_len;
 if (args.my_end <= args.my_start) self_len = 0;
 else str_self += args.my_start,
      self_len  = args.my_end-args.my_start;
 if (args.ot_end > other_len)
     args.ot_end = other_len;
 if (args.ot_end <= args.ot_start) other_len = 0;
 else str_other += args.ot_start,
      other_len  = args.ot_end-args.ot_start;
 SWITCH_ENC_SIZE(enc) {
 default:
  while (other_len && self_len) {
   uint8_t a,b;
   --self_len,--other_len;
   a = ((uint8_t *)str_self)[self_len];
   b = ((uint8_t *)str_other)[other_len];
   if (a != b) {
    a = (uint8_t)DeeUni_ToLower(a);
    b = (uint8_t)DeeUni_ToLower(b);
    if (a != b) break;
   }
   ++result;
  }
  break;
 CASE_ENC_2BYTE:
  while (other_len && self_len) {
   uint16_t a,b;
   --self_len,--other_len;
   a = ((uint16_t *)str_self)[self_len];
   b = ((uint16_t *)str_other)[other_len];
   if (a != b) {
    a = (uint16_t)DeeUni_ToLower(a);
    b = (uint16_t)DeeUni_ToLower(b);
    if (a != b) break;
   }
   ++result;
  }
  break;
 CASE_ENC_4BYTE:
  while (other_len && self_len) {
   uint32_t a,b;
   --self_len,--other_len;
   a = ((uint32_t *)str_self)[self_len];
   b = ((uint32_t *)str_other)[other_len];
   if (a != b) {
    a = (uint32_t)DeeUni_ToLower(a);
    b = (uint32_t)DeeUni_ToLower(b);
    if (a != b) break;
   }
   ++result;
  }
  break;
 }
 return DeeInt_NewSize(result);
err:
 return NULL;
}



/* Pull in definitions for wild-compare functions. */
#define T           uint8_t
#define wildcompare wildcompareb
#include "unicode/wildcompare.c.inl"
#define T           uint16_t
#define wildcompare wildcomparew
#include "unicode/wildcompare.c.inl"
#define Treturn     int64_t
#define T           uint32_t
#define wildcompare wildcomparel
#include "unicode/wildcompare.c.inl"

PRIVATE DREF DeeObject *DCALL
string_wildcompare(String *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
 uint8_t *str_self,*str_other; int enc;
 size_t self_len,other_len; int64_t result;
 struct compare_args args;
 if (get_compare_args(&args,argc,argv,"wildcompare"))
     goto err;
 enc       = DeeString_UtfEnc(self);
 str_self  = (uint8_t *)DeeString_UtfStr(self);
 str_other = (uint8_t *)DeeString_AsEncoding((DeeObject *)args.other,enc);
 if unlikely(!str_other) goto err;
 self_len  = ENCODING_SIZE(str_self);
 other_len = ENCODING_SIZE(str_other);
 if (args.my_end > self_len)
     args.my_end = self_len;
 if (args.my_end <= args.my_start) self_len = 0;
 else str_self += args.my_start,
      self_len  = args.my_end-args.my_start;
 if (args.ot_end > other_len)
     args.ot_end = other_len;
 if (args.ot_end <= args.ot_start) other_len = 0;
 else str_other += args.ot_start,
      other_len  = args.ot_end-args.ot_start;
 SWITCH_ENC_SIZE(enc) {
 default:
  result = wildcompareb((uint8_t *)str_self,self_len,
                        (uint8_t *)str_other,other_len);
  break;
 CASE_ENC_2BYTE:
  result = wildcomparew((uint16_t *)str_self,self_len,
                        (uint16_t *)str_other,other_len);
  break;
 CASE_ENC_4BYTE:
  result = wildcomparel((uint32_t *)str_self,self_len,
                        (uint32_t *)str_other,other_len);
  break;
 }
 return DeeInt_NewS64(result);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_wmatch(String *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 uint8_t *str_self,*str_other; int enc;
 size_t self_len,other_len; bool result;
 struct compare_args args;
 if (get_compare_args(&args,argc,argv,"wmatch"))
     goto err;
 enc       = DeeString_UtfEnc(self);
 str_self  = (uint8_t *)DeeString_UtfStr(self);
 str_other = (uint8_t *)DeeString_AsEncoding((DeeObject *)args.other,enc);
 if unlikely(!str_other) goto err;
 self_len  = ENCODING_SIZE(str_self);
 other_len = ENCODING_SIZE(str_other);
 if (args.my_end > self_len)
     args.my_end = self_len;
 if (args.my_end <= args.my_start) self_len = 0;
 else str_self += args.my_start,
      self_len  = args.my_end-args.my_start;
 if (args.ot_end > other_len)
     args.ot_end = other_len;
 if (args.ot_end <= args.ot_start) other_len = 0;
 else str_other += args.ot_start,
      other_len  = args.ot_end-args.ot_start;
 SWITCH_ENC_SIZE(enc) {
 default:
  result = wildcompareb((uint8_t *)str_self,self_len,
                        (uint8_t *)str_other,other_len) == 0;
  break;
 CASE_ENC_2BYTE:
  result = wildcomparew((uint16_t *)str_self,self_len,
                        (uint16_t *)str_other,other_len) == 0;
  break;
 CASE_ENC_4BYTE:
  result = wildcomparel((uint32_t *)str_self,self_len,
                        (uint32_t *)str_other,other_len) == 0;
  break;
 }
 return_bool_(result);
err:
 return NULL;
}


#define NOCASE
#define T           uint8_t
#define wildcompare wildcasecompareb
#include "unicode/wildcompare.c.inl"
#define NOCASE
#define T           uint16_t
#define wildcompare wildcasecomparew
#include "unicode/wildcompare.c.inl"
#define NOCASE
#define Treturn     int64_t
#define T           uint32_t
#define wildcompare wildcasecomparel
#include "unicode/wildcompare.c.inl"

PRIVATE DREF DeeObject *DCALL
string_casewildcompare(String *__restrict self,
                       size_t argc, DeeObject **__restrict argv) {
 uint8_t *str_self,*str_other; int enc;
 size_t self_len,other_len; int64_t result;
 struct compare_args args;
 if (get_compare_args(&args,argc,argv,"casewildcompare"))
     goto err;
 enc       = DeeString_UtfEnc(self);
 str_self  = (uint8_t *)DeeString_UtfStr(self);
 str_other = (uint8_t *)DeeString_AsEncoding((DeeObject *)args.other,enc);
 if unlikely(!str_other) goto err;
 self_len  = ENCODING_SIZE(str_self);
 other_len = ENCODING_SIZE(str_other);
 if (args.my_end > self_len)
     args.my_end = self_len;
 if (args.my_end <= args.my_start) self_len = 0;
 else str_self += args.my_start,
      self_len  = args.my_end-args.my_start;
 if (args.ot_end > other_len)
     args.ot_end = other_len;
 if (args.ot_end <= args.ot_start) other_len = 0;
 else str_other += args.ot_start,
      other_len  = args.ot_end-args.ot_start;
 SWITCH_ENC_SIZE(enc) {
 default:
  result = wildcasecompareb((uint8_t *)str_self,self_len,
                            (uint8_t *)str_other,other_len);
  break;
 CASE_ENC_2BYTE:
  result = wildcasecomparew((uint16_t *)str_self,self_len,
                            (uint16_t *)str_other,other_len);
  break;
 CASE_ENC_4BYTE:
  result = wildcasecomparel((uint32_t *)str_self,self_len,
                            (uint32_t *)str_other,other_len);
  break;
 }
 return DeeInt_NewS64(result);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_casewmatch(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 uint8_t *str_self,*str_other; int enc;
 size_t self_len,other_len; bool result;
 struct compare_args args;
 if (get_compare_args(&args,argc,argv,"casewmatch"))
     goto err;
 enc       = DeeString_UtfEnc(self);
 str_self  = (uint8_t *)DeeString_UtfStr(self);
 str_other = (uint8_t *)DeeString_AsEncoding((DeeObject *)args.other,enc);
 if unlikely(!str_other) goto err;
 self_len  = ENCODING_SIZE(str_self);
 other_len = ENCODING_SIZE(str_other);
 if (args.my_end > self_len)
     args.my_end = self_len;
 if (args.my_end <= args.my_start) self_len = 0;
 else str_self += args.my_start,
      self_len  = args.my_end-args.my_start;
 if (args.ot_end > other_len)
     args.ot_end = other_len;
 if (args.ot_end <= args.ot_start) other_len = 0;
 else str_other += args.ot_start,
      other_len  = args.ot_end-args.ot_start;
 SWITCH_ENC_SIZE(enc) {
 default:
  result = wildcasecompareb((uint8_t *)str_self,self_len,
                            (uint8_t *)str_other,other_len) == 0;
  break;
 CASE_ENC_2BYTE:
  result = wildcasecomparew((uint16_t *)str_self,self_len,
                            (uint16_t *)str_other,other_len) == 0;
  break;
 CASE_ENC_4BYTE:
  result = wildcasecomparel((uint32_t *)str_self,self_len,
                            (uint32_t *)str_other,other_len) == 0;
  break;
 }
 return_bool_(result);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_split(String *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 DeeObject *other;
 if (DeeArg_Unpack(argc,argv,"o:split",&other) ||
     DeeObject_AssertTypeExact(other,&DeeString_Type))
     return NULL;
 return DeeString_Split((DeeObject *)self,other);
}
PRIVATE DREF DeeObject *DCALL
string_casesplit(String *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 DeeObject *other;
 if (DeeArg_Unpack(argc,argv,"o:casesplit",&other) ||
     DeeObject_AssertTypeExact(other,&DeeString_Type))
     return NULL;
 return DeeString_CaseSplit((DeeObject *)self,other);
}

PRIVATE DREF DeeObject *DCALL
string_splitlines(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 bool keepends = false;
 if (DeeArg_Unpack(argc,argv,"|b:splitlines",&keepends))
     return NULL;
 return DeeString_SplitLines((DeeObject *)self,keepends);
}

PRIVATE DREF DeeObject *DCALL
string_indent(String *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 DeeObject *filler = &str_tab;
 if (DeeArg_Unpack(argc,argv,"|o:indent",&filler) ||
     DeeObject_AssertTypeExact(filler,&DeeString_Type))
     return NULL;
 return DeeString_Indent((DeeObject *)self,filler);
}

PRIVATE DREF DeeObject *DCALL
string_dedent(String *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 size_t max_chars = 1; DeeObject *mask = NULL;
 if (DeeArg_Unpack(argc,argv,"|Iuo:dedent",&max_chars,&mask))
     return NULL;
 return mask
      ? DeeString_Dedent((DeeObject *)self,max_chars,mask)
      : DeeString_DedentSpc((DeeObject *)self,max_chars);
}

PRIVATE DREF DeeObject *DCALL
string_format(String *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 DeeObject *args;
 if (DeeArg_Unpack(argc,argv,"o:format",&args))
     return NULL;
 return DeeString_Format((DeeObject *)self,args);
}

PRIVATE DREF DeeObject *DCALL
string_scanf(String *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 DeeObject *args;
 if (DeeArg_Unpack(argc,argv,"o:scanf",&args))
     return NULL;
 return DeeString_Scanf((DeeObject *)self,args);
}



PRIVATE DREF DeeObject *DCALL
string_findmatch(String *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 String *s_open,*s_clos; size_t start = 0,end = (size_t)-1;
 uint8_t *scan_str,*open_str,*clos_str; int enc; void *ptr;
 size_t scan_len,open_len,clos_len; dssize_t result;
 if (DeeArg_Unpack(argc,argv,"oo|IdId:findmatch",&s_open,&s_clos,&start,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)s_open,&DeeString_Type) ||
     DeeObject_AssertTypeExact((DeeObject *)s_clos,&DeeString_Type))
     goto err;
 enc = DeeString_UtfEnc(self);
 scan_str = (uint8_t *)DeeString_UtfStr(self);
 open_str = (uint8_t *)DeeString_AsEncoding((DeeObject *)s_open,enc);
 if unlikely(!open_str) goto err;
 clos_str = (uint8_t *)DeeString_AsEncoding((DeeObject *)s_clos,enc);
 if unlikely(!clos_str) goto err;
 open_len = ENCODING_SIZE(open_str);
 clos_len = ENCODING_SIZE(clos_str);
 if (end > ENCODING_SIZE(scan_str))
     end = ENCODING_SIZE(scan_str);
 if unlikely(end <= start) goto return_neg_one; /* Empty search area. */
 scan_len = end - start;

 SWITCH_ENC_SIZE(enc) {
 default:
  ptr = find_matchb((uint8_t *)scan_str + start,scan_len,
                    (uint8_t *)open_str,open_len,
                    (uint8_t *)clos_str,clos_len);
  if unlikely(!ptr) goto return_neg_one;
  result = (dssize_t)((uint8_t *)ptr - (uint8_t *)scan_str);
  break;
 CASE_ENC_2BYTE:
  ptr = find_matchw((uint16_t *)scan_str + start,scan_len,
                    (uint16_t *)open_str,open_len,
                    (uint16_t *)clos_str,clos_len);
  if unlikely(!ptr) goto return_neg_one;
  result = (dssize_t)((uint16_t *)ptr - (uint16_t *)scan_str);
  break;
 CASE_ENC_4BYTE:
  ptr = find_matchl((uint32_t *)scan_str + start,scan_len,
                    (uint32_t *)open_str,open_len,
                    (uint32_t *)clos_str,clos_len);
  if unlikely(!ptr) goto return_neg_one;
  result = (dssize_t)((uint32_t *)ptr - (uint32_t *)scan_str);
  break;
 }
done:
 return DeeInt_NewSSize(result);
return_neg_one:
 result = -1;
 goto done;
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_indexmatch(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 String *s_open,*s_clos; size_t start = 0,end = (size_t)-1;
 uint8_t *scan_str,*open_str,*clos_str; int enc; void *ptr;
 size_t scan_len,open_len,clos_len; size_t result;
 if (DeeArg_Unpack(argc,argv,"oo|IdId:indexmatch",&s_open,&s_clos,&start,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)s_open,&DeeString_Type) ||
     DeeObject_AssertTypeExact((DeeObject *)s_clos,&DeeString_Type))
     goto err;
 enc = DeeString_UtfEnc(self);
 scan_str = (uint8_t *)DeeString_UtfStr(self);
 open_str = (uint8_t *)DeeString_AsEncoding((DeeObject *)s_open,enc);
 if unlikely(!open_str) goto err;
 clos_str = (uint8_t *)DeeString_AsEncoding((DeeObject *)s_clos,enc);
 if unlikely(!clos_str) goto err;
 open_len = ENCODING_SIZE(open_str);
 clos_len = ENCODING_SIZE(clos_str);
 if (end > ENCODING_SIZE(scan_str))
     end = ENCODING_SIZE(scan_str);
 if unlikely(end <= start) goto err_not_found; /* Empty search area. */
 scan_len = end - start;

 SWITCH_ENC_SIZE(enc) {
 default:
  ptr = find_matchb((uint8_t *)scan_str + start,scan_len,
                    (uint8_t *)open_str,open_len,
                    (uint8_t *)clos_str,clos_len);
  if unlikely(!ptr) goto err_not_found;
  result = (dssize_t)((uint8_t *)ptr - (uint8_t *)scan_str);
  break;
 CASE_ENC_2BYTE:
  ptr = find_matchw((uint16_t *)scan_str + start,scan_len,
                    (uint16_t *)open_str,open_len,
                    (uint16_t *)clos_str,clos_len);
  if unlikely(!ptr) goto err_not_found;
  result = (dssize_t)((uint16_t *)ptr - (uint16_t *)scan_str);
  break;
 CASE_ENC_4BYTE:
  ptr = find_matchl((uint32_t *)scan_str + start,scan_len,
                    (uint32_t *)open_str,open_len,
                    (uint32_t *)clos_str,clos_len);
  if unlikely(!ptr) goto err_not_found;
  result = (dssize_t)((uint32_t *)ptr - (uint32_t *)scan_str);
  break;
 }
 return DeeInt_NewSSize(result);
err_not_found:
 err_index_not_found((DeeObject *)self,
                     (DeeObject *)s_clos);
err:
 return NULL;
}


PRIVATE DREF DeeObject *DCALL
string_casefindmatch(String *__restrict self,
                     size_t argc, DeeObject **__restrict argv) {
 String *s_open,*s_clos; size_t start = 0,end = (size_t)-1;
 uint8_t *scan_str,*open_str,*clos_str; int enc; void *ptr;
 size_t scan_len,open_len,clos_len; dssize_t result;
 if (DeeArg_Unpack(argc,argv,"oo|IdId:casefindmatch",&s_open,&s_clos,&start,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)s_open,&DeeString_Type) ||
     DeeObject_AssertTypeExact((DeeObject *)s_clos,&DeeString_Type))
     goto err;
 enc = DeeString_UtfEnc(self);
 scan_str = (uint8_t *)DeeString_UtfStr(self);
 open_str = (uint8_t *)DeeString_AsEncoding((DeeObject *)s_open,enc);
 if unlikely(!open_str) goto err;
 clos_str = (uint8_t *)DeeString_AsEncoding((DeeObject *)s_clos,enc);
 if unlikely(!clos_str) goto err;
 open_len = ENCODING_SIZE(open_str);
 clos_len = ENCODING_SIZE(clos_str);
 if (end > ENCODING_SIZE(scan_str))
     end = ENCODING_SIZE(scan_str);
 if unlikely(end <= start) goto return_neg_one; /* Empty search area. */
 scan_len = end - start;

 SWITCH_ENC_SIZE(enc) {
 default:
  ptr = find_casematchb((uint8_t *)scan_str + start,scan_len,
                        (uint8_t *)open_str,open_len,
                        (uint8_t *)clos_str,clos_len);
  if unlikely(!ptr) goto return_neg_one;
  result = (dssize_t)((uint8_t *)ptr - (uint8_t *)scan_str);
  break;
 CASE_ENC_2BYTE:
  ptr = find_casematchw((uint16_t *)scan_str + start,scan_len,
                        (uint16_t *)open_str,open_len,
                        (uint16_t *)clos_str,clos_len);
  if unlikely(!ptr) goto return_neg_one;
  result = (dssize_t)((uint16_t *)ptr - (uint16_t *)scan_str);
  break;
 CASE_ENC_4BYTE:
  ptr = find_casematchl((uint32_t *)scan_str + start,scan_len,
                        (uint32_t *)open_str,open_len,
                        (uint32_t *)clos_str,clos_len);
  if unlikely(!ptr) goto return_neg_one;
  result = (dssize_t)((uint32_t *)ptr - (uint32_t *)scan_str);
  break;
 }
done:
 return DeeInt_NewSSize(result);
return_neg_one:
 result = -1;
 goto done;
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_caseindexmatch(String *__restrict self,
                      size_t argc, DeeObject **__restrict argv) {
 String *s_open,*s_clos; size_t start = 0,end = (size_t)-1;
 uint8_t *scan_str,*open_str,*clos_str; int enc; void *ptr;
 size_t scan_len,open_len,clos_len; size_t result;
 if (DeeArg_Unpack(argc,argv,"oo|IdId:caseindexmatch",&s_open,&s_clos,&start,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)s_open,&DeeString_Type) ||
     DeeObject_AssertTypeExact((DeeObject *)s_clos,&DeeString_Type))
     goto err;
 enc = DeeString_UtfEnc(self);
 scan_str = (uint8_t *)DeeString_UtfStr(self);
 open_str = (uint8_t *)DeeString_AsEncoding((DeeObject *)s_open,enc);
 if unlikely(!open_str) goto err;
 clos_str = (uint8_t *)DeeString_AsEncoding((DeeObject *)s_clos,enc);
 if unlikely(!clos_str) goto err;
 open_len = ENCODING_SIZE(open_str);
 clos_len = ENCODING_SIZE(clos_str);
 if (end > ENCODING_SIZE(scan_str))
     end = ENCODING_SIZE(scan_str);
 if unlikely(end <= start) goto err_not_found; /* Empty search area. */
 scan_len = end - start;

 SWITCH_ENC_SIZE(enc) {
 default:
  ptr = find_casematchb((uint8_t *)scan_str + start,scan_len,
                        (uint8_t *)open_str,open_len,
                        (uint8_t *)clos_str,clos_len);
  if unlikely(!ptr) goto err_not_found;
  result = (dssize_t)((uint8_t *)ptr - (uint8_t *)scan_str);
  break;
 CASE_ENC_2BYTE:
  ptr = find_casematchw((uint16_t *)scan_str + start,scan_len,
                        (uint16_t *)open_str,open_len,
                        (uint16_t *)clos_str,clos_len);
  if unlikely(!ptr) goto err_not_found;
  result = (dssize_t)((uint16_t *)ptr - (uint16_t *)scan_str);
  break;
 CASE_ENC_4BYTE:
  ptr = find_casematchl((uint32_t *)scan_str + start,scan_len,
                        (uint32_t *)open_str,open_len,
                        (uint32_t *)clos_str,clos_len);
  if unlikely(!ptr) goto err_not_found;
  result = (dssize_t)((uint32_t *)ptr - (uint32_t *)scan_str);
  break;
 }
 return DeeInt_NewSSize(result);
err_not_found:
 err_index_not_found((DeeObject *)self,
                     (DeeObject *)s_clos);
err:
 return NULL;
}


PRIVATE DREF DeeObject *DCALL
string_rfindmatch(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 String *s_open,*s_clos; size_t start = 0,end = (size_t)-1;
 uint8_t *scan_str,*open_str,*clos_str; int enc; void *ptr;
 size_t scan_len,open_len,clos_len; dssize_t result;
 if (DeeArg_Unpack(argc,argv,"oo|IdId:rfindmatch",&s_open,&s_clos,&start,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)s_open,&DeeString_Type) ||
     DeeObject_AssertTypeExact((DeeObject *)s_clos,&DeeString_Type))
     goto err;
 enc = DeeString_UtfEnc(self);
 scan_str = (uint8_t *)DeeString_UtfStr(self);
 open_str = (uint8_t *)DeeString_AsEncoding((DeeObject *)s_open,enc);
 if unlikely(!open_str) goto err;
 clos_str = (uint8_t *)DeeString_AsEncoding((DeeObject *)s_clos,enc);
 if unlikely(!clos_str) goto err;
 open_len = ENCODING_SIZE(open_str);
 clos_len = ENCODING_SIZE(clos_str);
 if (end > ENCODING_SIZE(scan_str))
     end = ENCODING_SIZE(scan_str);
 if unlikely(end <= start) goto return_neg_one; /* Empty search area. */
 scan_len = end - start;

 SWITCH_ENC_SIZE(enc) {
 default:
  ptr = rfind_matchb((uint8_t *)scan_str + start,scan_len,
                     (uint8_t *)open_str,open_len,
                     (uint8_t *)clos_str,clos_len);
  if unlikely(!ptr) goto return_neg_one;
  result = (dssize_t)((uint8_t *)ptr - (uint8_t *)scan_str);
  break;
 CASE_ENC_2BYTE:
  ptr = rfind_matchw((uint16_t *)scan_str + start,scan_len,
                     (uint16_t *)open_str,open_len,
                     (uint16_t *)clos_str,clos_len);
  if unlikely(!ptr) goto return_neg_one;
  result = (dssize_t)((uint16_t *)ptr - (uint16_t *)scan_str);
  break;
 CASE_ENC_4BYTE:
  ptr = rfind_matchl((uint32_t *)scan_str + start,scan_len,
                     (uint32_t *)open_str,open_len,
                     (uint32_t *)clos_str,clos_len);
  if unlikely(!ptr) goto return_neg_one;
  result = (dssize_t)((uint32_t *)ptr - (uint32_t *)scan_str);
  break;
 }
done:
 return DeeInt_NewSSize(result);
return_neg_one:
 result = -1;
 goto done;
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_rindexmatch(String *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
 String *s_open,*s_clos; size_t start = 0,end = (size_t)-1;
 uint8_t *scan_str,*open_str,*clos_str; int enc; void *ptr;
 size_t scan_len,open_len,clos_len; size_t result;
 if (DeeArg_Unpack(argc,argv,"oo|IdId:rindexmatch",&s_open,&s_clos,&start,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)s_open,&DeeString_Type) ||
     DeeObject_AssertTypeExact((DeeObject *)s_clos,&DeeString_Type))
     goto err;
 enc = DeeString_UtfEnc(self);
 scan_str = (uint8_t *)DeeString_UtfStr(self);
 open_str = (uint8_t *)DeeString_AsEncoding((DeeObject *)s_open,enc);
 if unlikely(!open_str) goto err;
 clos_str = (uint8_t *)DeeString_AsEncoding((DeeObject *)s_clos,enc);
 if unlikely(!clos_str) goto err;
 open_len = ENCODING_SIZE(open_str);
 clos_len = ENCODING_SIZE(clos_str);
 if (end > ENCODING_SIZE(scan_str))
     end = ENCODING_SIZE(scan_str);
 if unlikely(end <= start) goto err_not_found; /* Empty search area. */
 scan_len = end - start;

 SWITCH_ENC_SIZE(enc) {
 default:
  ptr = rfind_matchb((uint8_t *)scan_str + start,scan_len,
                     (uint8_t *)open_str,open_len,
                     (uint8_t *)clos_str,clos_len);
  if unlikely(!ptr) goto err_not_found;
  result = (dssize_t)((uint8_t *)ptr - (uint8_t *)scan_str);
  break;
 CASE_ENC_2BYTE:
  ptr = rfind_matchw((uint16_t *)scan_str + start,scan_len,
                     (uint16_t *)open_str,open_len,
                     (uint16_t *)clos_str,clos_len);
  if unlikely(!ptr) goto err_not_found;
  result = (dssize_t)((uint16_t *)ptr - (uint16_t *)scan_str);
  break;
 CASE_ENC_4BYTE:
  ptr = rfind_matchl((uint32_t *)scan_str + start,scan_len,
                     (uint32_t *)open_str,open_len,
                     (uint32_t *)clos_str,clos_len);
  if unlikely(!ptr) goto err_not_found;
  result = (dssize_t)((uint32_t *)ptr - (uint32_t *)scan_str);
  break;
 }
 return DeeInt_NewSSize(result);
err_not_found:
 err_index_not_found((DeeObject *)self,
                     (DeeObject *)s_clos);
err:
 return NULL;
}


PRIVATE DREF DeeObject *DCALL
string_caserfindmatch(String *__restrict self,
                      size_t argc, DeeObject **__restrict argv) {
 String *s_open,*s_clos; size_t start = 0,end = (size_t)-1;
 uint8_t *scan_str,*open_str,*clos_str; int enc; void *ptr;
 size_t scan_len,open_len,clos_len; dssize_t result;
 if (DeeArg_Unpack(argc,argv,"oo|IdId:caserfindmatch",&s_open,&s_clos,&start,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)s_open,&DeeString_Type) ||
     DeeObject_AssertTypeExact((DeeObject *)s_clos,&DeeString_Type))
     goto err;
 enc = DeeString_UtfEnc(self);
 scan_str = (uint8_t *)DeeString_UtfStr(self);
 open_str = (uint8_t *)DeeString_AsEncoding((DeeObject *)s_open,enc);
 if unlikely(!open_str) goto err;
 clos_str = (uint8_t *)DeeString_AsEncoding((DeeObject *)s_clos,enc);
 if unlikely(!clos_str) goto err;
 open_len = ENCODING_SIZE(open_str);
 clos_len = ENCODING_SIZE(clos_str);
 if (end > ENCODING_SIZE(scan_str))
     end = ENCODING_SIZE(scan_str);
 if unlikely(end <= start) goto return_neg_one; /* Empty search area. */
 scan_len = end - start;

 SWITCH_ENC_SIZE(enc) {
 default:
  ptr = rfind_casematchb((uint8_t *)scan_str + start,scan_len,
                         (uint8_t *)open_str,open_len,
                         (uint8_t *)clos_str,clos_len);
  if unlikely(!ptr) goto return_neg_one;
  result = (dssize_t)((uint8_t *)ptr - (uint8_t *)scan_str);
  break;
 CASE_ENC_2BYTE:
  ptr = rfind_casematchw((uint16_t *)scan_str + start,scan_len,
                         (uint16_t *)open_str,open_len,
                         (uint16_t *)clos_str,clos_len);
  if unlikely(!ptr) goto return_neg_one;
  result = (dssize_t)((uint16_t *)ptr - (uint16_t *)scan_str);
  break;
 CASE_ENC_4BYTE:
  ptr = rfind_casematchl((uint32_t *)scan_str + start,scan_len,
                         (uint32_t *)open_str,open_len,
                         (uint32_t *)clos_str,clos_len);
  if unlikely(!ptr) goto return_neg_one;
  result = (dssize_t)((uint32_t *)ptr - (uint32_t *)scan_str);
  break;
 }
done:
 return DeeInt_NewSSize(result);
return_neg_one:
 result = -1;
 goto done;
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_caserindexmatch(String *__restrict self,
                       size_t argc, DeeObject **__restrict argv) {
 String *s_open,*s_clos; size_t start = 0,end = (size_t)-1;
 uint8_t *scan_str,*open_str,*clos_str; int enc; void *ptr;
 size_t scan_len,open_len,clos_len; size_t result;
 if (DeeArg_Unpack(argc,argv,"oo|IdId:caserindexmatch",&s_open,&s_clos,&start,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)s_open,&DeeString_Type) ||
     DeeObject_AssertTypeExact((DeeObject *)s_clos,&DeeString_Type))
     goto err;
 enc = DeeString_UtfEnc(self);
 scan_str = (uint8_t *)DeeString_UtfStr(self);
 open_str = (uint8_t *)DeeString_AsEncoding((DeeObject *)s_open,enc);
 if unlikely(!open_str) goto err;
 clos_str = (uint8_t *)DeeString_AsEncoding((DeeObject *)s_clos,enc);
 if unlikely(!clos_str) goto err;
 open_len = ENCODING_SIZE(open_str);
 clos_len = ENCODING_SIZE(clos_str);
 if (end > ENCODING_SIZE(scan_str))
     end = ENCODING_SIZE(scan_str);
 if unlikely(end <= start) goto err_not_found; /* Empty search area. */
 scan_len = end - start;

 SWITCH_ENC_SIZE(enc) {
 default:
  ptr = rfind_casematchb((uint8_t *)scan_str + start,scan_len,
                         (uint8_t *)open_str,open_len,
                         (uint8_t *)clos_str,clos_len);
  if unlikely(!ptr) goto err_not_found;
  result = (dssize_t)((uint8_t *)ptr - (uint8_t *)scan_str);
  break;
 CASE_ENC_2BYTE:
  ptr = rfind_casematchw((uint16_t *)scan_str + start,scan_len,
                         (uint16_t *)open_str,open_len,
                         (uint16_t *)clos_str,clos_len);
  if unlikely(!ptr) goto err_not_found;
  result = (dssize_t)((uint16_t *)ptr - (uint16_t *)scan_str);
  break;
 CASE_ENC_4BYTE:
  ptr = rfind_casematchl((uint32_t *)scan_str + start,scan_len,
                         (uint32_t *)open_str,open_len,
                         (uint32_t *)clos_str,clos_len);
  if unlikely(!ptr) goto err_not_found;
  result = (dssize_t)((uint32_t *)ptr - (uint32_t *)scan_str);
  break;
 }
 return DeeInt_NewSSize(result);
err_not_found:
 err_index_not_found((DeeObject *)self,
                     (DeeObject *)s_clos);
err:
 return NULL;
}



PRIVATE DREF DeeObject *DCALL
string_partitionmatch(String *__restrict self,
                      size_t argc, DeeObject **__restrict argv) {
 String *s_open,*s_clos; size_t start = 0,end = (size_t)-1;
 uint8_t *scan_str,*open_str,*clos_str; int enc; void *match_start,*match_end;
 size_t scan_len,open_len,clos_len;
 DREF DeeTupleObject *result;
 if (DeeArg_Unpack(argc,argv,"oo|IdId:partitionmatch",&s_open,&s_clos,&start,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)s_open,&DeeString_Type) ||
     DeeObject_AssertTypeExact((DeeObject *)s_clos,&DeeString_Type))
     goto err;
 enc = DeeString_UtfEnc(self);
 scan_str = (uint8_t *)DeeString_UtfStr(self);
 open_str = (uint8_t *)DeeString_AsEncoding((DeeObject *)s_open,enc);
 if unlikely(!open_str) goto err;
 clos_str = (uint8_t *)DeeString_AsEncoding((DeeObject *)s_clos,enc);
 if unlikely(!clos_str) goto err;
 result = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(3);
 if unlikely(!result) goto err;
 open_len = ENCODING_SIZE(open_str);
 clos_len = ENCODING_SIZE(clos_str);
 if (end > ENCODING_SIZE(scan_str))
     end = ENCODING_SIZE(scan_str);
 if unlikely(end <= start)
    goto match_not_found; /* Empty search area. */
 scan_len = end - start;

#define SET_STRING(a,b,c) \
 do{ \
   if ((result->t_elem[0] = (DREF DeeObject *)(a)) == NULL) goto err_r_0; \
   if ((result->t_elem[1] = (DREF DeeObject *)(b)) == NULL) goto err_r_1; \
   if ((result->t_elem[2] = (DREF DeeObject *)(c)) == NULL) goto err_r_2; \
 }__WHILE0

 SWITCH_ENC_SIZE(enc) {
 default:
  match_start = memmemb((uint8_t *)scan_str + start,scan_len,
                        (uint8_t *)open_str,open_len);
  if unlikely(!match_start) goto match_not_found;
  match_end = find_matchb((uint8_t *)match_start + open_len,scan_len-
                         ((uint8_t *)match_start - ((uint8_t *)scan_str + start)),
                          (uint8_t *)open_str,open_len,
                          (uint8_t *)clos_str,clos_len);
  if unlikely(!match_end) goto match_not_found;
  SET_STRING(DeeString_NewSized((char *)scan_str,
                               ((uint8_t *)match_start-(uint8_t *)scan_str)),
             DeeString_NewSized((char *)match_start,
                               ((uint8_t *)match_end + clos_len) -
                                (uint8_t *)match_start),
             DeeString_NewSized((char *)((uint8_t *)match_end + clos_len),
                               ((uint8_t *)scan_str + end)-
                               ((uint8_t *)match_end + clos_len)));
  break;
 CASE_ENC_2BYTE:
  match_start = memmemw((uint16_t *)scan_str + start,scan_len,
                        (uint16_t *)open_str,open_len);
  if unlikely(!match_start) goto match_not_found;
  match_end = find_matchw((uint16_t *)match_start + open_len,scan_len-
                         ((uint16_t *)match_start - ((uint16_t *)scan_str + start)),
                          (uint16_t *)open_str,open_len,
                          (uint16_t *)clos_str,clos_len);
  if unlikely(!match_end) goto match_not_found;
  SET_STRING(DeeString_NewSizedWithEncoding((uint16_t *)scan_str,
                                           ((uint16_t *)match_start-(uint16_t *)scan_str),enc),
             DeeString_NewSizedWithEncoding((uint16_t *)match_start,
                                           ((uint16_t *)match_end + clos_len) -
                                            (uint16_t *)match_start,enc),
             DeeString_NewSizedWithEncoding((uint16_t *)match_end + clos_len,
                                           ((uint16_t *)scan_str + end)-
                                           ((uint16_t *)match_end + clos_len),enc));
  break;
 CASE_ENC_4BYTE:
  match_start = memmeml((uint32_t *)scan_str + start,scan_len,
                        (uint32_t *)open_str,open_len);
  if unlikely(!match_start) goto match_not_found;
  match_end = find_matchl((uint32_t *)match_start + open_len,scan_len-
                         ((uint32_t *)match_start - ((uint32_t *)scan_str + start)),
                          (uint32_t *)open_str,open_len,
                          (uint32_t *)clos_str,clos_len);
  if unlikely(!match_end) goto match_not_found;
  SET_STRING(DeeString_NewSizedWithEncoding((uint32_t *)scan_str,
                                           ((uint32_t *)match_start-(uint32_t *)scan_str),enc),
             DeeString_NewSizedWithEncoding((uint32_t *)match_start,
                                           ((uint32_t *)match_end + clos_len) -
                                            (uint32_t *)match_start,enc),
             DeeString_NewSizedWithEncoding((uint32_t *)match_end + clos_len,
                                           ((uint32_t *)scan_str + end)-
                                           ((uint32_t *)match_end + clos_len),enc));
  break;
 }
#undef SET_STRING
done:
 return (DREF DeeObject *)result;
match_not_found:
 result->t_elem[0] = (DREF DeeObject *)string_getsubstr(self,start,end);
 if unlikely(!result->t_elem[0]) goto err_r_0;
 result->t_elem[1] = Dee_EmptyString;
 result->t_elem[2] = Dee_EmptyString;
#ifdef CONFIG_NO_THREADS
 Dee_EmptyString->ob_refcnt += 2;
#else
 ATOMIC_FETCHADD(Dee_EmptyString->ob_refcnt,2);
#endif
 goto done;
err_r_2:
 Dee_DecrefDokill(result->t_elem[1]);
err_r_1:
 Dee_DecrefDokill(result->t_elem[0]);
err_r_0:
 DeeTuple_FreeUninitialized((DeeObject *)result);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_rpartitionmatch(String *__restrict self,
                       size_t argc, DeeObject **__restrict argv) {
 String *s_open,*s_clos; size_t start = 0,end = (size_t)-1;
 uint8_t *scan_str,*open_str,*clos_str; int enc; void *match_start,*match_end;
 size_t scan_len,open_len,clos_len;
 DREF DeeTupleObject *result;
 if (DeeArg_Unpack(argc,argv,"oo|IdId:rpartitionmatch",&s_open,&s_clos,&start,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)s_open,&DeeString_Type) ||
     DeeObject_AssertTypeExact((DeeObject *)s_clos,&DeeString_Type))
     goto err;
 enc = DeeString_UtfEnc(self);
 scan_str = (uint8_t *)DeeString_UtfStr(self);
 open_str = (uint8_t *)DeeString_AsEncoding((DeeObject *)s_open,enc);
 if unlikely(!open_str) goto err;
 clos_str = (uint8_t *)DeeString_AsEncoding((DeeObject *)s_clos,enc);
 if unlikely(!clos_str) goto err;
 result = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(3);
 if unlikely(!result) goto err;
 open_len = ENCODING_SIZE(open_str);
 clos_len = ENCODING_SIZE(clos_str);
 if (end > ENCODING_SIZE(scan_str))
     end = ENCODING_SIZE(scan_str);
 if unlikely(end <= start)
    goto match_not_found; /* Empty search area. */
 scan_len = end - start;

#define SET_STRING(a,b,c) \
 do{ \
   if ((result->t_elem[0] = (DREF DeeObject *)(a)) == NULL) goto err_r_0; \
   if ((result->t_elem[1] = (DREF DeeObject *)(b)) == NULL) goto err_r_1; \
   if ((result->t_elem[2] = (DREF DeeObject *)(c)) == NULL) goto err_r_2; \
 }__WHILE0

 SWITCH_ENC_SIZE(enc) {
 default:
  match_end = memrmemb((uint8_t *)scan_str + start,scan_len,
                       (uint8_t *)clos_str,clos_len);
  if unlikely(!match_end) goto match_not_found;
  match_start = rfind_matchb((uint8_t *)scan_str + start,
                             (size_t)((uint8_t *)match_end -
                                     ((uint8_t *)scan_str + start)),
                             (uint8_t *)open_str,open_len,
                             (uint8_t *)clos_str,clos_len);
  if unlikely(!match_start) goto match_not_found;
  SET_STRING(DeeString_NewSized((char *)scan_str,
                               ((uint8_t *)match_start-(uint8_t *)scan_str)),
             DeeString_NewSized((char *)match_start,
                               ((uint8_t *)match_end + clos_len) -
                                (uint8_t *)match_start),
             DeeString_NewSized((char *)((uint8_t *)match_end + clos_len),
                               ((uint8_t *)scan_str + end)-
                               ((uint8_t *)match_end + clos_len)));
  break;
 CASE_ENC_2BYTE:
  match_end = memrmemw((uint16_t *)scan_str + start,scan_len,
                       (uint16_t *)clos_str,clos_len);
  if unlikely(!match_end) goto match_not_found;
  match_start = rfind_matchw((uint16_t *)scan_str + start,
                             (size_t)((uint16_t *)match_end -
                                     ((uint16_t *)scan_str + start)),
                             (uint16_t *)open_str,open_len,
                             (uint16_t *)clos_str,clos_len);
  if unlikely(!match_start) goto match_not_found;
  SET_STRING(DeeString_NewSizedWithEncoding((uint16_t *)scan_str,
                                           ((uint16_t *)match_start-(uint16_t *)scan_str),enc),
             DeeString_NewSizedWithEncoding((uint16_t *)match_start,
                                           ((uint16_t *)match_end + clos_len) -
                                            (uint16_t *)match_start,enc),
             DeeString_NewSizedWithEncoding((uint16_t *)match_end + clos_len,
                                           ((uint16_t *)scan_str + end)-
                                           ((uint16_t *)match_end + clos_len),enc));
  break;
 CASE_ENC_4BYTE:
  match_end = memrmeml((uint32_t *)scan_str + start,scan_len,
                       (uint32_t *)clos_str,clos_len);
  if unlikely(!match_end) goto match_not_found;
  match_start = rfind_matchl((uint32_t *)scan_str + start,
                             (size_t)((uint32_t *)match_end -
                                     ((uint32_t *)scan_str + start)),
                             (uint32_t *)open_str,open_len,
                             (uint32_t *)clos_str,clos_len);
  if unlikely(!match_start) goto match_not_found;
  SET_STRING(DeeString_NewSizedWithEncoding((uint32_t *)scan_str,
                                           ((uint32_t *)match_start-(uint32_t *)scan_str),enc),
             DeeString_NewSizedWithEncoding((uint32_t *)match_start,
                                           ((uint32_t *)match_end + clos_len) -
                                            (uint32_t *)match_start,enc),
             DeeString_NewSizedWithEncoding((uint32_t *)match_end + clos_len,
                                           ((uint32_t *)scan_str + end)-
                                           ((uint32_t *)match_end + clos_len),enc));
  break;
 }
#undef SET_STRING
done:
 return (DREF DeeObject *)result;
match_not_found:
 result->t_elem[0] = (DREF DeeObject *)string_getsubstr(self,start,end);
 if unlikely(!result->t_elem[0]) goto err_r_0;
 result->t_elem[1] = Dee_EmptyString;
 result->t_elem[2] = Dee_EmptyString;
#ifdef CONFIG_NO_THREADS
 Dee_EmptyString->ob_refcnt += 2;
#else
 ATOMIC_FETCHADD(Dee_EmptyString->ob_refcnt,2);
#endif
 goto done;
err_r_2:
 Dee_DecrefDokill(result->t_elem[1]);
err_r_1:
 Dee_DecrefDokill(result->t_elem[0]);
err_r_0:
 DeeTuple_FreeUninitialized((DeeObject *)result);
err:
 return NULL;
}


PRIVATE DREF DeeObject *DCALL
string_casepartitionmatch(String *__restrict self,
                          size_t argc, DeeObject **__restrict argv) {
 String *s_open,*s_clos; size_t start = 0,end = (size_t)-1;
 uint8_t *scan_str,*open_str,*clos_str; int enc; void *match_start,*match_end;
 size_t scan_len,open_len,clos_len;
 DREF DeeTupleObject *result;
 if (DeeArg_Unpack(argc,argv,"oo|IdId:casepartitionmatch",&s_open,&s_clos,&start,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)s_open,&DeeString_Type) ||
     DeeObject_AssertTypeExact((DeeObject *)s_clos,&DeeString_Type))
     goto err;
 enc = DeeString_UtfEnc(self);
 scan_str = (uint8_t *)DeeString_UtfStr(self);
 open_str = (uint8_t *)DeeString_AsEncoding((DeeObject *)s_open,enc);
 if unlikely(!open_str) goto err;
 clos_str = (uint8_t *)DeeString_AsEncoding((DeeObject *)s_clos,enc);
 if unlikely(!clos_str) goto err;
 result = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(3);
 if unlikely(!result) goto err;
 open_len = ENCODING_SIZE(open_str);
 clos_len = ENCODING_SIZE(clos_str);
 if (end > ENCODING_SIZE(scan_str))
     end = ENCODING_SIZE(scan_str);
 if unlikely(end <= start)
    goto match_not_found; /* Empty search area. */
 scan_len = end - start;

#define SET_STRING(a,b,c) \
 do{ \
   if ((result->t_elem[0] = (DREF DeeObject *)(a)) == NULL) goto err_r_0; \
   if ((result->t_elem[1] = (DREF DeeObject *)(b)) == NULL) goto err_r_1; \
   if ((result->t_elem[2] = (DREF DeeObject *)(c)) == NULL) goto err_r_2; \
 }__WHILE0

 SWITCH_ENC_SIZE(enc) {
 default:
  match_start = memcasememb((uint8_t *)scan_str + start,scan_len,
                            (uint8_t *)open_str,open_len);
  if unlikely(!match_start) goto match_not_found;
  match_end = find_casematchb((uint8_t *)match_start + open_len,scan_len-
                             ((uint8_t *)match_start - ((uint8_t *)scan_str + start)),
                              (uint8_t *)open_str,open_len,
                              (uint8_t *)clos_str,clos_len);
  if unlikely(!match_end) goto match_not_found;
  SET_STRING(DeeString_NewSized((char *)scan_str,
                               ((uint8_t *)match_start-(uint8_t *)scan_str)),
             DeeString_NewSized((char *)match_start,
                               ((uint8_t *)match_end + clos_len) -
                                (uint8_t *)match_start),
             DeeString_NewSized((char *)((uint8_t *)match_end + clos_len),
                               ((uint8_t *)scan_str + end)-
                               ((uint8_t *)match_end + clos_len)));
  break;
 CASE_ENC_2BYTE:
  match_start = memcasememw((uint16_t *)scan_str + start,scan_len,
                            (uint16_t *)open_str,open_len);
  if unlikely(!match_start) goto match_not_found;
  match_end = find_casematchw((uint16_t *)match_start + open_len,scan_len-
                             ((uint16_t *)match_start - ((uint16_t *)scan_str + start)),
                              (uint16_t *)open_str,open_len,
                              (uint16_t *)clos_str,clos_len);
  if unlikely(!match_end) goto match_not_found;
  SET_STRING(DeeString_NewSizedWithEncoding((uint16_t *)scan_str,
                                           ((uint16_t *)match_start-(uint16_t *)scan_str),enc),
             DeeString_NewSizedWithEncoding((uint16_t *)match_start,
                                           ((uint16_t *)match_end + clos_len) -
                                            (uint16_t *)match_start,enc),
             DeeString_NewSizedWithEncoding((uint16_t *)match_end + clos_len,
                                           ((uint16_t *)scan_str + end)-
                                           ((uint16_t *)match_end + clos_len),enc));
  break;
 CASE_ENC_4BYTE:
  match_start = memcasememl((uint32_t *)scan_str + start,scan_len,
                            (uint32_t *)open_str,open_len);
  if unlikely(!match_start) goto match_not_found;
  match_end = find_casematchl((uint32_t *)match_start + open_len,scan_len-
                             ((uint32_t *)match_start - ((uint32_t *)scan_str + start)),
                              (uint32_t *)open_str,open_len,
                              (uint32_t *)clos_str,clos_len);
  if unlikely(!match_end) goto match_not_found;
  SET_STRING(DeeString_NewSizedWithEncoding((uint32_t *)scan_str,
                                           ((uint32_t *)match_start-(uint32_t *)scan_str),enc),
             DeeString_NewSizedWithEncoding((uint32_t *)match_start,
                                           ((uint32_t *)match_end + clos_len) -
                                            (uint32_t *)match_start,enc),
             DeeString_NewSizedWithEncoding((uint32_t *)match_end + clos_len,
                                           ((uint32_t *)scan_str + end)-
                                           ((uint32_t *)match_end + clos_len),enc));
  break;
 }
#undef SET_STRING
done:
 return (DREF DeeObject *)result;
match_not_found:
 result->t_elem[0] = (DREF DeeObject *)string_getsubstr(self,start,end);
 if unlikely(!result->t_elem[0]) goto err_r_0;
 result->t_elem[1] = Dee_EmptyString;
 result->t_elem[2] = Dee_EmptyString;
#ifdef CONFIG_NO_THREADS
 Dee_EmptyString->ob_refcnt += 2;
#else
 ATOMIC_FETCHADD(Dee_EmptyString->ob_refcnt,2);
#endif
 goto done;
err_r_2:
 Dee_DecrefDokill(result->t_elem[1]);
err_r_1:
 Dee_DecrefDokill(result->t_elem[0]);
err_r_0:
 DeeTuple_FreeUninitialized((DeeObject *)result);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_caserpartitionmatch(String *__restrict self,
                       size_t argc, DeeObject **__restrict argv) {
 String *s_open,*s_clos; size_t start = 0,end = (size_t)-1;
 uint8_t *scan_str,*open_str,*clos_str; int enc; void *match_start,*match_end;
 size_t scan_len,open_len,clos_len;
 DREF DeeTupleObject *result;
 if (DeeArg_Unpack(argc,argv,"oo|IdId:caserpartitionmatch",&s_open,&s_clos,&start,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)s_open,&DeeString_Type) ||
     DeeObject_AssertTypeExact((DeeObject *)s_clos,&DeeString_Type))
     goto err;
 enc = DeeString_UtfEnc(self);
 scan_str = (uint8_t *)DeeString_UtfStr(self);
 open_str = (uint8_t *)DeeString_AsEncoding((DeeObject *)s_open,enc);
 if unlikely(!open_str) goto err;
 clos_str = (uint8_t *)DeeString_AsEncoding((DeeObject *)s_clos,enc);
 if unlikely(!clos_str) goto err;
 result = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(3);
 if unlikely(!result) goto err;
 open_len = ENCODING_SIZE(open_str);
 clos_len = ENCODING_SIZE(clos_str);
 if (end > ENCODING_SIZE(scan_str))
     end = ENCODING_SIZE(scan_str);
 if unlikely(end <= start)
    goto match_not_found; /* Empty search area. */
 scan_len = end - start;

#define SET_STRING(a,b,c) \
 do{ \
   if ((result->t_elem[0] = (DREF DeeObject *)(a)) == NULL) goto err_r_0; \
   if ((result->t_elem[1] = (DREF DeeObject *)(b)) == NULL) goto err_r_1; \
   if ((result->t_elem[2] = (DREF DeeObject *)(c)) == NULL) goto err_r_2; \
 }__WHILE0

 SWITCH_ENC_SIZE(enc) {
 default:
  match_end = memcasermemb((uint8_t *)scan_str + start,scan_len,
                           (uint8_t *)clos_str,clos_len);
  if unlikely(!match_end) goto match_not_found;
  match_start = rfind_casematchb((uint8_t *)scan_str + start,
                                 (size_t)((uint8_t *)match_end -
                                         ((uint8_t *)scan_str + start)),
                                 (uint8_t *)open_str,open_len,
                                 (uint8_t *)clos_str,clos_len);
  if unlikely(!match_start) goto match_not_found;
  SET_STRING(DeeString_NewSized((char *)scan_str,
                               ((uint8_t *)match_start-(uint8_t *)scan_str)),
             DeeString_NewSized((char *)match_start,
                               ((uint8_t *)match_end + clos_len) -
                                (uint8_t *)match_start),
             DeeString_NewSized((char *)((uint8_t *)match_end + clos_len),
                               ((uint8_t *)scan_str + end)-
                               ((uint8_t *)match_end + clos_len)));
  break;
 CASE_ENC_2BYTE:
  match_end = memcasermemw((uint16_t *)scan_str + start,scan_len,
                           (uint16_t *)clos_str,clos_len);
  if unlikely(!match_end) goto match_not_found;
  match_start = rfind_casematchw((uint16_t *)scan_str + start,
                                 (size_t)((uint16_t *)match_end -
                                         ((uint16_t *)scan_str + start)),
                                 (uint16_t *)open_str,open_len,
                                 (uint16_t *)clos_str,clos_len);
  if unlikely(!match_start) goto match_not_found;
  SET_STRING(DeeString_NewSizedWithEncoding((uint16_t *)scan_str,
                                           ((uint16_t *)match_start-(uint16_t *)scan_str),enc),
             DeeString_NewSizedWithEncoding((uint16_t *)match_start,
                                           ((uint16_t *)match_end + clos_len) -
                                            (uint16_t *)match_start,enc),
             DeeString_NewSizedWithEncoding((uint16_t *)match_end + clos_len,
                                           ((uint16_t *)scan_str + end)-
                                           ((uint16_t *)match_end + clos_len),enc));
  break;
 CASE_ENC_4BYTE:
  match_end = memcasermeml((uint32_t *)scan_str + start,scan_len,
                           (uint32_t *)clos_str,clos_len);
  if unlikely(!match_end) goto match_not_found;
  match_start = rfind_casematchl((uint32_t *)scan_str + start,
                                 (size_t)((uint32_t *)match_end -
                                         ((uint32_t *)scan_str + start)),
                                 (uint32_t *)open_str,open_len,
                                 (uint32_t *)clos_str,clos_len);
  if unlikely(!match_start) goto match_not_found;
  SET_STRING(DeeString_NewSizedWithEncoding((uint32_t *)scan_str,
                                           ((uint32_t *)match_start-(uint32_t *)scan_str),enc),
             DeeString_NewSizedWithEncoding((uint32_t *)match_start,
                                           ((uint32_t *)match_end + clos_len) -
                                            (uint32_t *)match_start,enc),
             DeeString_NewSizedWithEncoding((uint32_t *)match_end + clos_len,
                                           ((uint32_t *)scan_str + end)-
                                           ((uint32_t *)match_end + clos_len),enc));
  break;
 }
#undef SET_STRING
done:
 return (DREF DeeObject *)result;
match_not_found:
 result->t_elem[0] = (DREF DeeObject *)string_getsubstr(self,start,end);
 if unlikely(!result->t_elem[0]) goto err_r_0;
 result->t_elem[1] = Dee_EmptyString;
 result->t_elem[2] = Dee_EmptyString;
#ifdef CONFIG_NO_THREADS
 Dee_EmptyString->ob_refcnt += 2;
#else
 ATOMIC_FETCHADD(Dee_EmptyString->ob_refcnt,2);
#endif
 goto done;
err_r_2:
 Dee_DecrefDokill(result->t_elem[1]);
err_r_1:
 Dee_DecrefDokill(result->t_elem[0]);
err_r_0:
 DeeTuple_FreeUninitialized((DeeObject *)result);
err:
 return NULL;
}





INTERN struct type_method string_methods[] = {

    /* String encoding functions */
    { "decode", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_decode,
      DOC("(string encoding=\"utf8\",string errors=\"strict\")->string\n"
          "@throw ValueError The given @encoding or @errors wasn't recognized\n"
          "@throw UnicodeDecodeError @this string could not be decoded as @encoding and @errors was set to ${\"strict\"}\n"
          "@param errors The way that encoding errors are handled as one of ${\"strict\"}, ${\"replace\"} or ${\"ignore\"}\n"
          "Decode @this string, re-interpreting its underlying basic data stream as @encoding") },
    { "ord", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_ord,
      DOC("()->int\n"
          "@throw ValueError The length of @this string is not equal to ${1}\n"
          "Return the ordinal integral value of @this single-character string") },

    /* String formatting / scanning. */
    { "format", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_format,
      DOC("(sequence args)->string") },
    { "scanf", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_scanf,
      DOC("(string format)->sequence") },
    /* TODO:
     * >> print "You name is $your_name, and I'm ${my_name}"
     * >>       .substitute({ .your_name = "foo", .my_name = "bar" });
     * >> print "You owe $guy $$10 dollar!"
     * >>       .substitute({ .guy = "me" });
     * 
     */

    /* String/Character traits */
    { "isalpha", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isalpha,
      DOC("(int start=0,int end=-1)->bool\nReturns :true if all characters in ${this.substr(start,end)} are alphabetical") },
    { "islower", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_islower,
      DOC("(int start=0,int end=-1)->bool\nReturns :true if all characters in ${this.substr(start,end)} are lower-case") },
    { "isupper", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isupper,
      DOC("(int start=0,int end=-1)->bool\nReturns :true if all characters in ${this.substr(start,end)} are upper-case") },
    { "isalnum", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isalnum,
      DOC("(int start=0,int end=-1)->bool\nReturns :true if all characters in ${this.substr(start,end)} are alpha-numerical") },
    { "isspace", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isspace,
      DOC("(int start=0,int end=-1)->bool\nReturns :true if all characters in ${this.substr(start,end)} are space-characters") },
    { "isprint", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isprint,
      DOC("(int start=0,int end=-1)->bool\nReturns :true if all characters in ${this.substr(start,end)} are printable") },
    { "isdigit", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isdigit,
      DOC("(int start=0,int end=-1)->bool\nReturns :true if all characters in ${this.substr(start,end)} are digits") },
    { "isdecimal", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isdecimal,
      DOC("(int start=0,int end=-1)->bool\nReturns :true if all characters in ${this.substr(start,end)} are dicimal symbol") },
    { "isnumeric", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isnumeric,
      DOC("(int start=0,int end=-1)->bool\nReturns :true if all characters in ${this.substr(start,end)} qualify as digit or decimal characters") },
    { "istitle", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_istitle,
      DOC("(int start=0,int end=-1)->bool\nReturns :true if the sub-string ${this.substr(start,end)} follows title-casing, meaning that space is followed by upper-case") },
    { "issymbol", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_issymbol,
      DOC("(int start=0,int end=-1)->bool\nReturns :true if the sub-string ${this.substr(start,end)} is a valid symbol name") },

    /* any-Character traits */
    { "isanyalpha", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isanyalpha,
      DOC("(int start=0,int end=-1)->bool\nReturns :true if any characters in ${this.substr(start,end)} are alphabetical") },
    { "isanylower", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isanylower,
      DOC("(int start=0,int end=-1)->bool\nReturns :true if any characters in ${this.substr(start,end)} are lower-case") },
    { "isanyupper", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isanyupper,
      DOC("(int start=0,int end=-1)->bool\nReturns :true if any characters in ${this.substr(start,end)} are upper-case") },
    { "isanyalnum", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isanyalnum,
      DOC("(int start=0,int end=-1)->bool\nReturns :true if any characters in ${this.substr(start,end)} are alpha-numerical") },
    { "isanyspace", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isanyspace,
      DOC("(int start=0,int end=-1)->bool\nReturns :true if any characters in ${this.substr(start,end)} are space-characters") },
    { "isanyprint", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isanyprint,
      DOC("(int start=0,int end=-1)->bool\nReturns :true if any characters in ${this.substr(start,end)} are printable") },
    { "isanydigit", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isanydigit,
      DOC("(int start=0,int end=-1)->bool\nReturns :true if any characters in ${this.substr(start,end)} are digits") },
    { "isanydecimal", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isanydecimal,
      DOC("(int start=0,int end=-1)->bool\nReturns :true if any characters in ${this.substr(start,end)} are dicimal symbol") },
    { "isanynumeric", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isanynumeric,
      DOC("(int start=0,int end=-1)->bool\nReturns :true if any characters in ${this.substr(start,end)} qualify as digit or decimal characters") },

    /* String conversion */
    /* TODO: Add start/end index arguments. */
    { "lower", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_lower,
      DOC("()->string\n"
          "Returns @this string converted to lower-case") },
    { "upper", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_upper,
      DOC("()->string\n"
          "Returns @this string converted to upper-case") },
    { "title", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_title,
      DOC("()->string\n"
          "Returns @this string converted to title-casing") },
    { "capitalize", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_capitalize,
      DOC("()->string\n"
          "Returns @this string with each word capitalized") },
    { "swapcase", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_swapcase,
      DOC("()->string\n"
          "Returns @this string with the casing of each "
          "character that has two different casings swapped") },

    /* Case-sensitive query functions */
    { "replace", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_replace,
      DOC("(string find_str,string replace_str,int max_count=int.SIZE_MAX)->string\n"
          "Find up to @max_count occurrances of @find_str and replace each with @replace_str, then return the resulting string") },
    { "find", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_find,
      DOC("(string other,int start=0,int end=-1)->int\n"
          "Find the first instance of @other that can be found within ${this.substr(start,end)}, "
          "and return its starting index, or ${-1} if no such position exists") },
    { "rfind", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_rfind,
      DOC("(string other,int start=0,int end=-1)->int\n"
          "Find the last instance of @other that can be found within ${this.substr(start,end)}, "
          "and return its starting index, or ${-1} if no such position exists") },
    { "index", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_index,
      DOC("(string other,int start=0,int end=-1)->int\n"
          "@throw IndexError No instance of @other can be found within ${this.substr(start,end)}\n"
          "Find the first instance of @other that can be found within ${this.substr(start,end)}, "
          "and return its starting index") },
    { "rindex", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_rindex,
      DOC("(string other,int start=0,int end=-1)->int\n"
          "@throw IndexError No instance of @other can be found within ${this.substr(start,end)}\n"
          "Find the last instance of @other that can be found within ${this.substr(start,end)}, "
          "and return its starting index") },
    { "count", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_count,
      DOC("(string other,int start=0,int end=-1)->int\n"
          "Count the number of instances of @other that can be found within ${this.substr(start,end)}, "
          "and return now many were found") },
    { "contains", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_contains_f,
      DOC("(string other,int start=0,int end=-1)->bool\n"
          "Check if @other can be found within ${this.substr(start,end)}, and return a boolean indicative of that") },
    { "substr", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_substr,
      DOC("(int start=0,int end=-1)->string\n"
          "Similar to ${this[start:end]}, however only integer-convertible objects may "
          "be passed (passing :none will invoke ${(int)none}, which results in $0), and "
          "passing negative values for either @start or @end will cause :int.SIZE_MAX to "
          "be used for that argument:\n"
          "$s = \"foo bar foobar\";\n"
          "$print repr s.substr(0,1);    /* \"f\" */\n"
          "$print repr s[0:1];           /* \"f\" */\n"
          "$print repr s.substr(0,#s);   /* \"foo bar foobar\" */\n"
          "$print repr s[0:#s];          /* \"foo bar foobar\" */\n"
          "$print repr s.substr(0,1234); /* \"foo bar foobar\" */\n"
          "$print repr s[0:1234];        /* \"foo bar foobar\" */\n"
          "$print repr s.substr(0,-1);   /* \"foo bar foobar\" -- Negative indices intentionally underflow into positive infinity */\n"
          "$print repr s[0:-1];          /* \"foo bar fooba\" */\n"
          "Also note that this way of interpreting integer indices is mirrored by all other "
          "string functions that allow start/end-style arguments, including #find, #compare, "
          "as well as many others") },
    { "strip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_strip,
      DOC("()->string\n"
          "(string mask)->string\n"
          "Strip all leading and trailing whitespace-characters, or "
          "characters apart of @mask, and return the resulting string") },
    { "lstrip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_lstrip,
      DOC("()->string\n"
          "(string mask)->string\n"
          "Strip all leading whitespace-characters, or "
          "characters apart of @mask, and return the resulting string") },
    { "rstrip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_rstrip,
      DOC("()->string\n"
          "(string mask)->string\n"
          "Strip all trailing whitespace-characters, or "
          "characters apart of @mask, and return the resulting string") },
    { "sstrip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_sstrip,
      DOC("(string other)->string\n"
          "Strip all leading and trailing instances of @other from @this string\n"
          "$local result = this;\n"
          "$while (result.startswith(other))\n"
          "$       result = result[#other:];\n"
          "$while (result.endswith(other))\n"
          "$       result = result[:#result-#other];\n") },
    { "lsstrip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_lsstrip,
      DOC("(string other)->string\n"
          "Strip all leading instances of @other from @this string\n"
          "$local result = this;\n"
          "$while (result.startswith(other))\n"
          "$       result = result[#other:];\n") },
    { "rsstrip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_rsstrip,
      DOC("(string other)->string\n"
          "Strip all trailing instances of @other from @this string\n"
          "$local result = this;\n"
          "$while (result.endswith(other))\n"
          "$       result = result[:#result-#other];\n") },
    { "startswith", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_startswith,
      DOC("(string other,int start=0,int end=-1)->bool\n"
          "Return :true if the sub-string ${this.substr(start,end)} starts with @other") },
    { "endswith", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_endswith,
      DOC("(string other,int start=0,int end=-1)->bool\n"
          "Return :true if the sub-string ${this.substr(start,end)} ends with @other") },
    { "partition", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_parition,
      DOC("(string other,int start=0,int end=-1)->(string,string,string)\n"
          "Search for the first instance of @other within ${this.substr(start,end)} and "
          "return a 3-element sequence of strings ${(this[:pos],other,this[pos+#other:])}.\n"
          "If @other could not be found, ${(this,\"\",\"\")} is returned") },
    { "rpartition", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_rparition,
      DOC("(string other,int start=0,int end=-1)->(string,string,string)\n"
          "Search for the last instance of @other within ${this.substr(start,end)} and "
          "return a 3-element sequence of strings ${(this[:pos],other,this[pos+#other:])}.\n"
          "If @other could not be found, ${(this,\"\",\"\")} is returned") },
    { "compare", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_compare,
      DOC("(string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,string other,int other_start=0,int other_end=-1)->int\n"
          "Compare the sub-string ${left = this.substr(my_start,my_end)} with ${right = other.substr(other_start,other_end)}, returning "
          "${< 0} if ${left < right}, ${> 0} if ${left > right}, or ${== 0} if they are equal") },
    { "vercompare", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_vercompare,
      DOC("(string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,string other,int other_start=0,int other_end=-1)->int\n"
          "Performs a version-string comparison. This is similar to #compare, but rather than "
          "performing a strict lexicographical comparison, the numbers found in the strings "
          "being compared are comparsed as a whole, fixing the common problem seen in applications "
          "such as file navigators showing a file order of `foo1.txt', `foo10.txt', `foo11.txt', `foo2.txt', etc...\n"
          "This function is a portable implementation of the GNU function "
          "%{link https://linux.die.net/man/3/strverscmp strverscmp}, "
          "for which you may follow the link for further details") },
    { "wildcompare", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_wildcompare,
      DOC("(string pattern,int pattern_start=0,int pattern_end=-1)->int\n"
          "(int my_start,string pattern,int pattern_start=0,int pattern_end=-1)->int\n"
          "(int my_start,int my_end,string pattern,int pattern_start=0,int pattern_end=-1)->int\n"
          "Perform a wild-character-enabled comparising of the sub-string ${left = this.substr(my_start,my_end)} "
          "with ${right = pattern.substr(pattern_start,pattern_end)}, returning ${< 0} if ${left < right}, ${> 0} "
          "if ${left > right}, or ${== 0} if they are equal\n"
          "Wild-compare characters are only parsed from @pattern, allowing ${\"?\"} to "
          "be matched with any single character from @this, and ${\"*\"} to be matched to "
          "any number of characters") },
    { "fuzzycompare", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_fuzzycompare,
      DOC("(string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,string other,int other_start=0,int other_end=-1)->int\n"
          "Perform a fuzzy string comparison between ${this[my_start:my_end]} and ${other[other_start:other_end]}\n"
          "The return value is a similarty-factor that can be used to score how close the two strings look alike.\n"
          "How exactly the scoring is done is implementation-specific, however a score of $0 is reserved for two "
          "strings that are perfectly identical, any two differing strings always have a score ${> 0}, and the closer "
          "the score is to $0, the more alike they actually are\n"
          "The intended use of this function is for auto-completion, as well as warning "
          "messages and recommendations in the sense of I-dont-know-foo-but-did-you-mean-bar\n"
          "Note that there is another version #casefuzzycompare that also ignores casing") },
    { "wmatch", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_wmatch,
      DOC("(string other,int other_start=0,int other_end=-1)->bool\n"
          "(int my_start,string other,int other_start=0,int other_end=-1)->bool\n"
          "(int my_start,int my_end,string other,int other_start=0,int other_end=-1)->bool\n"
          "Same as #wildcompare, returning :true where #wildcompare would return $0, and :false in all other cases") },

    /* Case-insensitive query functions */
    { "casereplace", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casereplace,
      DOC("(string find_str,string replace_str,int max_count=int.SIZE_MAX)->int\n"
          "Same as #replace, however casing is ignored during character comparisons") },
    { "casefind", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casefind,
      DOC("(string other,int start=0,int end=-1)->int\n"
          "Same as #find, however casing is ignored during character comparisons") },
    { "caserfind", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_caserfind,
      DOC("(string other,int start=0,int end=-1)->int\n"
          "Same as #rfind, however casing is ignored during character comparisons") },
    { "caseindex", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_caseindex,
      DOC("(string other,int start=0,int end=-1)->int\n"
          "Same as #index, however casing is ignored during character comparisons") },
    { "caserindex", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_caserindex,
      DOC("(string other,int start=0,int end=-1)->int\n"
          "Same as #rindex, however casing is ignored during character comparisons") },
    { "casecount", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casecount,
      DOC("(string other,int start=0,int end=-1)->int\n"
          "Same as #count, however casing is ignored during character comparisons") },
    { "casecontains", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casecontains_f,
      DOC("(string other,int start=0,int end=-1)->bool\n"
          "Same as #contains, however casing is ignored during character comparisons") },
    { "casesstrip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casesstrip,
      DOC("(string other)->string\n"
          "Same as #sstrip, however casing is ignored during character comparisons") },
    { "caselsstrip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_caselsstrip,
      DOC("(string other)->string\n"
          "Same as #lsstrip, however casing is ignored during character comparisons") },
    { "casersstrip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casersstrip,
      DOC("(string other)->string\n"
          "Same as #rsstrip, however casing is ignored during character comparisons") },
    { "casestartswith", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casestartswith,
      DOC("(string other,int start=0,int end=-1)->bool\n"
          "Same as #startswith, however casing is ignored during character comparisons") },
    { "caseendswith", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_caseendswith,
      DOC("(string other,int start=0,int end=-1)->bool\n"
          "Same as #endswith, however casing is ignored during character comparisons") },
    { "casepartition", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_caseparition,
      DOC("(string other,int start=0,int end=-1)->(string,string,string)\n"
          "Same as #partition, however casing is ignored during character comparisons") },
    { "caserpartition", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_caserparition,
      DOC("(string other,int start=0,int end=-1)->(string,string,string)\n"
          "Same as #rpartition, however casing is ignored during character comparisons") },
    { "casecompare", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casecompare,
      DOC("(string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,string other,int other_start=0,int other_end=-1)->int\n"
          "Same as #compare, however casing is ignored during character comparisons") },
    { "casevercompare", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casevercompare,
      DOC("(string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,string other,int other_start=0,int other_end=-1)->int\n"
          "Same as #vercompare, however casing is ignored during character comparisons") },
    { "casewildcompare", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casewildcompare,
      DOC("(string pattern,int pattern_start=0,int pattern_end=-1)->int\n"
          "(int my_start,string pattern,int pattern_start=0,int pattern_end=-1)->int\n"
          "(int my_start,int my_end,string pattern,int pattern_start=0,int pattern_end=-1)->int\n"
          "Same as #wildcompare, however casing is ignored during character comparisons") },
    { "casefuzzycompare", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casefuzzycompare,
      DOC("(string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,string other,int other_start=0,int other_end=-1)->int\n"
          "Same as #fuzzycompare, however casing is ignored during character comparisons") },
    { "casewmatch", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casewmatch,
      DOC("(string pattern,int pattern_start=0,int pattern_end=-1)->bool\n"
          "(int my_start,string pattern,int pattern_start=0,int pattern_end=-1)->bool\n"
          "(int my_start,int my_end,string pattern,int pattern_start=0,int pattern_end=-1)->bool\n"
          "Same as #wmatch, however casing is ignored during character comparisons") },

    /* String alignment functions. */
    { "center", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_center,
      DOC("(int width,string filler=\" \")->string\n"
          "Use @this string as result, then evenly insert @filler at "
          "the front and back to pad its length to @width characters") },
    { "ljust", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_ljust,
      DOC("(int width,string filler=\" \")->string\n"
          "Use @this string as result, then insert @filler "
          "at the back to pad its length to @width characters") },
    { "rjust", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_rjust,
      DOC("(int width,string filler=\" \")->string\n"
          "Use @this string as result, then insert @filler "
          "at the front to pad its length to @width characters") },
    { "zfill", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_zfill,
      DOC("(int width,string filler=\"0\")->string\n"
          "Skip leading ${\'+\'} and ${\'-\'} characters, then insert @filler "
          "to pad the resulting string to a length of @width characters") },
    { "reversed", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_reversed,
      DOC("(int start=0,int end=-1)->string\n"
          "Return the sub-string ${this.substr(start,end)} with its character order reversed") },
    { "expandtabs", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_expandtabs,
      DOC("(int tabwidth=8)->string\n"
          "Expand tab characters with whitespace offset from the start of "
          "their respective line at multiples of @tabwidth") },
    { "unifylines", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_unifylines,
      DOC("(string replacement=\"\\n\")->string\n"
          "Unify all linefeed character sequences found in @this string to "
          "make exclusive use of @replacement") },

    /* String -- sequence interaction. */
    { "join", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_join,
      DOC("(sequence seq)->string\n"
          "Iterate @seq and convert all items into string, inserting @this "
          "string before each element, starting only with the second") },
    { "split", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_split,
      DOC("(string sep)->sequence\n"
          "Split @this string at each instance of @sep, returning a sequence of the resulting parts") },
    { "casesplit", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casesplit,
      DOC("(string sep)->sequence\n"
          "Same as #split, however casing is ignored during character comparisons") },
    { "splitlines", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_splitlines,
      DOC("(bool keepends=false)->sequence\n"
          "Split @this string at each linefeed, returning a sequence of all contained lines\n"
          "When @keepends is :false, this is identical to ${this.unifylines().split(\"\\n\")}\n"
          "When @keepends is :true, items found in the returned sequence will still have their "
          "original, trailing line-feed appended") },

    /* String indentation. */
    { "indent", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_indent,
      DOC("(string filler=\"\\t\")->string\n"
          "Using @this string as result, insert @filler at the front, as well as after "
          "every linefeed with the exception of one that may be located at its end\n"
          "The inteded use is for generating strings from structured data, such as HTML:\n"
          "$text = get_html();\n"
          "$text = \"<html>\n{}\n</html>\".format({ text.strip().indent() });\n"
          ) },
    { "dedent", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_dedent,
      DOC("(int max_chars=1)->string\n"
          "(int max_chars=1,string mask)->string\n"
          "Using @this string as result, remove up to @max_chars whitespace "
          "(s.a. #isspace) characters, or if given: characters apart of @mask "
          "from the front, as well as following any linefeed") },

    /* Common-character search functions. */
    { "common", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_lcommon,
      DOC("(string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,string other,int other_start=0,int other_end=-1)->int\n"
          "Returns the number of common leading characters shared between @this and @other, "
          "or in other words: the lowest index $i for which ${this[i] != other[i]} is true") },
    { "rcommon", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_rcommon,
      DOC("(string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,string other,int other_start=0,int other_end=-1)->int\n"
          "Returns the number of common trailing characters shared between @this and @other") },
    { "casecommon", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_caselcommon,
      DOC("(string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,string other,int other_start=0,int other_end=-1)->int\n"
          "Same as #common, however casing is ignored during character comparisons") },
    { "casercommon", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casercommon,
      DOC("(string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,string other,int other_start=0,int other_end=-1)->int\n"
          "Same as #rcommon, however casing is ignored during character comparisons") },

    /* Find match character sequences */
    { "findmatch", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_findmatch,
      DOC("(string open,string close,int start=0,int end=-1)->int\n"
          "Similar to #find, but do a recursive search for the "
          "first @close that doesn't have a match @{open}:\n"
          "$s = \"foo(bar(),baz(42),7).strip()\";\n"
          "$lcol = s.find(\"(\");\n"
          "$print lcol; /* 3 */\n"
          "$mtch = s.findmatch(\"(\",\")\",lcol+1);\n"
          "$print repr s[lcol:mtch+1]; /* \"(bar(),baz(42),7)\" */\n"
          "If no @close without a match @open exists, ${-1} is returned\n"
          "Note that @open and @close are not restricted to single-character "
          "strings, are allowed to be of any length") },
    { "indexmatch", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_indexmatch,
      DOC("(string open,string close,int start=0,int end=-1)->int\n"
          "@throw IndexError No instance of @close without a match @open exists within ${this.substr(start,end)}\n"
          "Same as #findmatch, but throw an :IndexError instead of "
          "returning ${-1} if no @close without a match @open exists") },
    { "casefindmatch", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casefindmatch,
      DOC("(string open,string close,int start=0,int end=-1)->int\n"
          "Same as :findmatch, however casing is ignored during character comparisons") },
    { "caseindexmatch", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_caseindexmatch,
      DOC("(string open,string close,int start=0,int end=-1)->int\n"
          "@throw IndexError No instance of @close without a match @open exists within ${this.substr(start,end)}\n"
          "Same as :indexmatch, however casing is ignored during character comparisons") },
    { "rfindmatch", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_rfindmatch,
      DOC("(string open,string close,int start=0,int end=-1)->int\n"
          "Similar to #findmatch, but operate in a mirrored fashion, searching for the "
          "last instance of @open that has no match @close within ${this.substr(start,end)}:\n"
          "$s = \"get_string().foo(bar(),baz(42),7).length\";\n"
          "$lcol = s.find(\")\");\n"
          "$print lcol; /* 19 */\n"
          "$mtch = s.rfindmatch(\"(\",\")\",0,lcol);\n"
          "$print repr s[mtch:lcol+1]; /* \"(bar(),baz(42),7)\" */\n"
          "If no @open without a match @close exists, ${-1} is returned") },
    { "rindexmatch", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_rindexmatch,
      DOC("(string open,string close,int start=0,int end=-1)->int\n"
          "@throw IndexError No instance of @open without a match @close exists within ${this.substr(start,end)}\n"
          "Same as #rfindmatch, but throw an :IndexError instead of "
          "returning ${-1} if no @open without a match @close exists") },
    { "caserfindmatch", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_caserfindmatch,
      DOC("(string open,string close,int start=0,int end=-1)->int\n"
          "Same as :rfindmatch, however casing is ignored during character comparisons") },
    { "caserindexmatch", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_caserindexmatch,
      DOC("(string open,string close,int start=0,int end=-1)->int\n"
          "@throw IndexError No instance of @open without a match @close exists within ${this.substr(start,end)}\n"
          "Same as :rindexmatch, however casing is ignored during character comparisons") },

    /* Using the find-match functionality, also provide a partitioning version */
    { "partitionmatch", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_partitionmatch,
      DOC("(string open,string close,int start=0,int end=-1)->(string,string,string)\n"
          "A hybrid between #find, #findmatch and #partition that returns the strings surrounding "
          "the matched string portion, the first being the substring prior to the match, "
          "the second being the matched string itself (including the @open and @close strings), "
          "and the third being the substring after the match:\n"
          "$s = \"foo {x,y,{13,19,42,{}},w} -- tail {}\";\n"
          "$print repr s.partitionmatch(\"{\",\"}\"); /* { \"foo \", \"{x,y,{13,19,42,{}},w}\", \" -- tail {}\" } */\n"
          "If no matching @open + @close pair could be found, ${(this[start:end],\"\",\"\")} is returned\n"
          "$function partitionmatch(open, close, start = 0, end = -1) {\n"
          "$    local j;\n"
          "$    local i = this.find(open,start,end);\n"
          "$    if (i < 0 || (j = this.findmatch(open,close,i+#open,end)) < 0)\n"
          "$        return (this.substr(start,end),\"\",\"\");\n"
          "$    return (this.substr(start,i),this.substr(i,j+#close),this.substr(j+#close,end))\n"
          "$}") },
    { "rpartitionmatch", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_rpartitionmatch,
      DOC("(string open,string close,int start=0,int end=-1)->(string,string,string)\n"
          "A hybrid between #rfind, #rfindmatch and #rpartition that returns the strings surrounding "
          "the matched string portion, the first being the substring prior to the match, "
          "the second being the matched string itself (including the @open and @close strings), "
          "and the third being the substring after the match:\n"
          "$s = \"{} foo {x,y,{13,19,42,{}},w} -- tail\";\n"
          "$print repr s.rpartitionmatch(\"{\",\"}\"); /* { \"{} foo \", \"{x,y,{13,19,42,{}},w}\", \" -- tail\" } */\n"
          "If no matching @open + @close pair could be found, ${(this[start:end],\"\",\"\")} is returned\n"
          "$function rpartitionmatch(open, close, start = 0, end = -1) {\n"
          "$    local i;\n"
          "$    local j = this.rfind(close,start,end);\n"
          "$    if (j < 0 || (i = this.rfindmatch(open,close,start,j)) < 0)\n"
          "$        return (this.substr(start,end),\"\",\"\");\n"
          "$    return (this.substr(start,i),this.substr(i,j+#close),this.substr(j+#close,end))\n"
          "$}") },
    { "casepartitionmatch", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casepartitionmatch,
      DOC("(string open,string close,int start=0,int end=-1)->(string,string,string)\n"
          "Same as #partitionmatch, however casing is ignored during character comparisons") },
    { "caserpartitionmatch", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_caserpartitionmatch,
      DOC("(string open,string close,int start=0,int end=-1)->(string,string,string)\n"
          "Same as #rpartitionmatch, however casing is ignored during character comparisons") },

//  TODO: DEE_METHODDEF_CONST_v100("segments",member(&_deestring_F(segments)),"(size_t n) -> sequence\n@return: An iterable sequence of @n evenly long strings, based on @this"),

    /* String optimizations for standard sequence functions. */
    { "front", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_front },
    { "back", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_back },

    /* Deprecated functions. */
    { "reverse", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_reversed,
      DOC("(int start=0,int end=-1)->string\nDeprecated alias for #reversed") },
    { NULL }
};

PRIVATE DREF String *DCALL
string_cat(String *__restrict self, DeeObject *__restrict other) {
 DREF String *result;
 if ((other = DeeObject_Str(other)) == NULL) return NULL;
 Dee_Incref(self);
 result = (DREF String *)DeeString_CatInherited((DeeObject *)self,(DeeObject *)other);
 if unlikely(!result) {
  Dee_Decref(self);
  Dee_Decref(other);
 }
#ifndef CONFIG_USE_NEW_STRING_API
 else {
  int encoding = DeeString_UtfEnc(self);
  if (encoding != STRING_WIDTH_1BYTE) {
   if unlikely(result == (String *)Dee_EmptyString) {
    Dee_Decref(result);
    return (DREF String *)DeeString_NewSizedWithEncoding(NULL,0,encoding);
   }
   if unlikely(!DeeString_AsEncoding((DeeObject *)result,encoding))
      goto err;
   /* Set the preferred encoding. */
   ASSERT(DeeString_UTF(result)->su_pref == STRING_WIDTH_1BYTE ||
          DeeString_UTF(result)->su_pref == encoding);
   DeeString_UTF(result)->su_pref = encoding;
  }
 }
#endif
 return result;
#ifndef CONFIG_USE_NEW_STRING_API
err:
 Dee_Decref(result);
 return NULL;
#endif
}

PRIVATE DREF String *DCALL
string_mul(String *__restrict self, DeeObject *__restrict other) {
 DREF String *result; size_t repeat; int encoding;
 char *dst,*src; size_t src_len;
 if (DeeObject_AsSize(other,&repeat))
     return NULL;
 if (self->s_len == 0)
     return_reference_(self);
 result = (DREF String *)DeeString_NewBuffer(self->s_len*repeat);
 if unlikely(!result) return NULL;
 dst     = DeeString_STR(result);
 src     = DeeString_STR(self);
 src_len = DeeString_SIZE(self);
 /* Copy the source string into the destination buffer. */
 while (repeat--) {
  memcpy(dst,src,src_len);
  dst += src_len;
 }
 /* Ensure the same encoding as the source string. */
 encoding = DeeString_UtfEnc(self);
 if unlikely(encoding != STRING_WIDTH_1BYTE) {
  if unlikely(!DeeString_AsEncoding((DeeObject *)result,encoding)) {
   Dee_Decref(result);
   return NULL;
  }
  ASSERT(DeeString_UTF(result)->su_pref == STRING_WIDTH_1BYTE);
  DeeString_UTF(result)->su_pref = encoding;
 }
 return result;
}

PRIVATE DREF String *DCALL
string_mod(String *__restrict self, DeeObject *__restrict args) {
 DREF String *result;
 /* C-style string formating */
 if (DeeTuple_Check(args)) {
  result = (DREF String *)DeeString_CFormat((DeeObject *)self,
                                             DeeTuple_SIZE(args),
                                             DeeTuple_ELEM(args));
 } else {
  result = (DREF String *)DeeString_CFormat((DeeObject *)self,1,
                                            (DeeObject **)&args);
 }
 return result;
}

INTERN struct type_math string_math = {
    /* .tp_int32  = */NULL,
    /* .tp_int64  = */NULL,
    /* .tp_double = */NULL,
    /* .tp_int    = */NULL,
    /* .tp_inv    = */NULL,
    /* .tp_pos    = */NULL,
    /* .tp_neg    = */NULL,
    /* .tp_add    = */(DREF DeeObject *(DCALL*)(DeeObject *__restrict,DeeObject *__restrict))&string_cat,
    /* .tp_sub    = */NULL,
    /* .tp_mul    = */(DREF DeeObject *(DCALL*)(DeeObject *__restrict,DeeObject *__restrict))&string_mul,
    /* .tp_div    = */NULL,
    /* .tp_mod    = */(DREF DeeObject *(DCALL*)(DeeObject *__restrict,DeeObject *__restrict))&string_mod,
    /* .tp_shl    = */NULL,
    /* .tp_shr    = */NULL,
    /* .tp_and    = */NULL,
    /* .tp_or     = */NULL,
    /* .tp_xor    = */NULL,
    /* .tp_pow    = */NULL
};


INTERN DREF DeeObject *DCALL
string_contains(String *__restrict self,
                DeeObject *__restrict some_object) {
 void *me,*other,*ptr; int enc;
 if (DeeObject_AssertTypeExact(some_object,&DeeString_Type))
     return NULL;
 /* Search for an occurrence of `some_object' */
 enc   = DeeString_UtfEnc(self);
 me    = DeeString_UtfStr(self);
 other = DeeString_AsEncoding(some_object,enc);
 if unlikely(!other) return NULL;
 SWITCH_ENC_SIZE(enc) {
 default:
  ptr = memmemb((uint8_t *)me,ENCODING_SIZE(me),(uint8_t *)other,ENCODING_SIZE(other));
  break;             
 CASE_ENC_2BYTE:
  ptr = memmemw((uint16_t *)me,ENCODING_SIZE(me),(uint16_t *)other,ENCODING_SIZE(other));
  break;             
 CASE_ENC_4BYTE:
  ptr = memmeml((uint32_t *)me,ENCODING_SIZE(me),(uint32_t *)other,ENCODING_SIZE(other));
  break;             
 }
 return_bool_(ptr != NULL);
}


DECL_END

#ifndef __INTELLISENSE__
#include "unicode/split.c.inl"
#endif

#endif /* !GUARD_DEEMON_OBJECTS_STRING_FUNCTIONS_C */
