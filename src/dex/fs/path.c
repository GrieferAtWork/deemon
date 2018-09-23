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
#ifndef GUARD_DEX_FS_PATH_C
#define GUARD_DEX_FS_PATH_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

#include "libfs.h"
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <hybrid/minmax.h>

#include <string.h>

DECL_BEGIN

#ifdef CONFIG_HOST_WINDOWS
#define SEP                  '\\'
#define SEP_S                "\\"
#define ISSEP(x)     ((x) == '\\' || (x) == '/')
#define ISABS(x)     ((x)[0] && (x)[1] == ':')
#define ISABS_STR(x) (DeeString_WLEN(x) >= 2 && DeeString_GetChar(x,1) == ':')
#else
#define SEP                  '/'
#define SEP_S                "/"
#define ISSEP(x)     ((x) == '/')
#define ISABS(x)     ((x)[0] == '/')
#define ISABS_STR(x) (DeeString_WLEN(x) >= 1 && DeeString_GetChar(x,0) == '/')
#endif


#ifndef __USE_GNU
#define memrchr  dee_memrchr
LOCAL void *dee_memrchr(void const *__restrict p, int c, size_t n) {
 uint8_t *iter = (uint8_t *)p+n;
 while (iter != (uint8_t *)p) {
  if (*--iter == c) return iter;
 }
 return NULL;
}
#endif /* !__USE_GNU */

INTERN DREF DeeObject *DCALL
fs_pathhead(DeeObject *__restrict path) {
 char *tailsep;
 ASSERT_OBJECT_TYPE_EXACT(path,&DeeString_Type);
 /* TODO: Unicode support */
 tailsep = (char *)memrchr(DeeString_STR(path),'/',
                           DeeString_SIZE(path)*
                           sizeof(char));
#ifdef CONFIG_HOST_WINDOWS /* TODO: Must use the first one of `["/","\\"]' */
 if (!tailsep)
      tailsep = (char *)memrchr(DeeString_STR(path),'\\',
                                DeeString_SIZE(path)*
                                sizeof(char));
#endif
 if (!tailsep)
      return_empty_string;
 return DeeString_NewSized(DeeString_STR(path),
                  (size_t)((tailsep+1)-DeeString_STR(path)));
}
INTERN DREF DeeObject *DCALL
fs_pathtail(DeeObject *__restrict path) {
 char *tailsep;
 ASSERT_OBJECT_TYPE_EXACT(path,&DeeString_Type);
 /* TODO: Unicode support */
 tailsep = (char *)memrchr(DeeString_STR(path),'/',
                           DeeString_SIZE(path)*
                           sizeof(char));
#ifdef CONFIG_HOST_WINDOWS /* TODO: Must use the first one of `["/","\\"]' */
 if (!tailsep)
      tailsep = (char *)memrchr(DeeString_STR(path),'\\',
                                DeeString_SIZE(path)*
                                sizeof(char));
#endif
 if (!tailsep)
      return_reference_(path);
 ++tailsep;
 return DeeString_NewSized(tailsep,
                  (size_t)(DeeString_END(path)-tailsep));
}
INTERN DREF DeeObject *DCALL
fs_pathfile(DeeObject *__restrict path) {
 char *tailsep,*extpos; size_t tailsize;
 ASSERT_OBJECT_TYPE_EXACT(path,&DeeString_Type);
 /* TODO: Unicode support */
 tailsep = (char *)memrchr(DeeString_STR(path),'/',
                           DeeString_SIZE(path)*
                           sizeof(char));
#ifdef CONFIG_HOST_WINDOWS /* TODO: Must use the first one of `["/","\\"]' */
 if (!tailsep)
      tailsep = (char *)memrchr(DeeString_STR(path),'\\',
                                DeeString_SIZE(path)*
                                sizeof(char));
#endif
 if (tailsep) ++tailsep;
 else tailsep = DeeString_STR(path);
 tailsize = (size_t)(DeeString_END(path)-tailsep);
 extpos = (char *)memrchr(tailsep,'.',tailsize*sizeof(char));
 if (extpos) tailsize = (size_t)(extpos-tailsep);
 return DeeString_NewSized(tailsep,tailsize);
}
INTERN DREF DeeObject *DCALL
fs_pathext(DeeObject *__restrict path) {
 char *extpos;
 ASSERT_OBJECT_TYPE_EXACT(path,&DeeString_Type);
 /* TODO: Unicode support */
 extpos = (char *)memrchr(DeeString_STR(path),'.',
                          DeeString_SIZE(path)*
                          sizeof(char));
 if (!extpos)
      return_empty_string;
 ++extpos;
 return DeeString_NewSized(extpos,
                  (size_t)(DeeString_END(path)-extpos));
}

PRIVATE DEFINE_STRING(str_single_slash,SEP_S);
PRIVATE DEFINE_STRING(str_single_dot,".");

INTERN DREF DeeObject *DCALL
fs_pathdrive(DeeObject *__restrict path) {
#ifdef CONFIG_HOST_WINDOWS
 char *drive_start,*end; DREF DeeObject *result;
 ASSERT_OBJECT_TYPE_EXACT(path,&DeeString_Type);
 /* TODO: Unicode support */
 end = (drive_start = DeeString_STR(path))+DeeString_SIZE(path);
 for (; drive_start != end; ++drive_start) {
  size_t drive_length;
  if (ISSEP(*drive_start)) break; /* Stop on the first slash. */
  if (*drive_start != ':') continue; /* Found the drive character. */
  ++drive_start;
  drive_length = (size_t)(drive_start-DeeString_STR(path));
  result = DeeString_NewBuffer(drive_length+1);
  if likely(result) {
   char *dst = DeeString_STR(result);
   memcpy(dst,DeeString_STR(path),drive_length*sizeof(char));
   /* Alway follow up with a slash. */
   dst[drive_length] = *drive_start == '/' ? '/' : '\\';
  }
  return result;
 }
 return_empty_string;
#else
 ASSERT_OBJECT_TYPE_EXACT(path,&DeeString_Type);
 (void)path;
 return_reference_((DeeObject *)&str_single_slash);
#endif
}

INTERN DREF DeeObject *DCALL
fs_pathinctrail(DeeObject *__restrict path) {
 char endch; DREF DeeObject *result;
 ASSERT_OBJECT_TYPE_EXACT(path,&DeeString_Type);
 /* TODO: Unicode support */
 if (!DeeString_SIZE(path))
      return_reference_((DeeObject *)&str_single_slash);
 endch = DeeString_END(path)[-1];
 if (ISSEP(endch)) return_reference_(path);
 /* The +1 out-of-bounds read is OK because of the trailing \0 */
 result = DeeString_NewSized(DeeString_STR(path),
                             DeeString_SIZE(path)+1);
 if likely(result) DeeString_END(result)[-1] = SEP;
 return result;
}
INTERN DREF DeeObject *DCALL
fs_pathexctrail(DeeObject *__restrict path) {
 size_t pathlen; char endch;
 ASSERT_OBJECT_TYPE_EXACT(path,&DeeString_Type);
 /* TODO: Unicode support */
 pathlen = DeeString_SIZE(path);
 if (!pathlen ||
     (endch = DeeString_STR(path)[pathlen-1],
      ISSEP(endch)))
      return_reference_(path);
 return DeeString_NewSized(DeeString_STR(path),pathlen-1);
}
INTERN bool DCALL
fs_pathisabs(DeeObject *__restrict path) {
 ASSERT_OBJECT_TYPE_EXACT(path,&DeeString_Type);
 return ISABS_STR(path);
}
INTERN DREF DeeObject *DCALL
fs_pathabs(DeeObject *__restrict path, DeeObject *pwd) {
 DREF DeeObject *result;
 ASSERT_OBJECT_TYPE_EXACT(path,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT_OPT(pwd,&DeeString_Type);
 /* Quick check: If the given path already is absolute,
  *              then we've got nothing to do. */
 if (ISABS_STR(path)) {
  if (!pwd)
       return_reference_(path);
  /* If a custom PWD is given, then we have to do this double-callback. */
  path = fs_pathrel(path,NULL);
  if unlikely(!path) goto err;
  ASSERT(!ISABS_STR(path));
  result = fs_pathabs(path,pwd);
  Dee_Decref(path);
  return result;
 }
 /* If the given `pwd' isn't absolute, make it using the real PWD. */
 if (pwd && !ISABS_STR(pwd)) {
  pwd = fs_pathabs(pwd,NULL);
  if unlikely(!pwd) goto err;
 } else if (pwd) {
  Dee_Incref(pwd);
 }
 if (!pwd) {
  /* Use a printer to take advantage of `fs_printcwd()' */
  struct unicode_printer printer = UNICODE_PRINTER_INIT;
  if (fs_printcwd(&printer))
      goto err_printer;
  if unlikely(UNICODE_PRINTER_ISEMPTY(&printer))
     return_reference_(path); /* ??? */
  /* Append a trailing backslash if the given path doesn't start with one. */
  if (UNICODE_PRINTER_GETCHAR(&printer,UNICODE_PRINTER_LENGTH(&printer) - 1) != SEP &&
     (DeeString_SIZE(path) && !ISSEP(DeeString_STR(path)[0])) &&
      unicode_printer_putascii(&printer,SEP)) goto err_printer;
  /* Print the given path. */
  if (unicode_printer_printstring(&printer,path) < 0)
      goto err_printer;
  result = unicode_printer_pack(&printer);
  goto done;
err_printer:
  unicode_printer_fini(&printer);
  goto err_pwd;
 }
 /* Simply concat the 2 paths and insert a slash in between (if necessary). */
 {
  char *dst,*pwd_start,*pth_start,*end;
  uint32_t ch; size_t pwd_length,pth_length;
  pwd_start  = DeeString_AsUtf8(pwd);
  if unlikely(!pwd_start) goto err_pwd;
  pth_start  = DeeString_AsUtf8(path);
  if unlikely(!pth_start) goto err_pwd;
  pwd_length = WSTR_LENGTH(pwd_start);
  pth_length = WSTR_LENGTH(pth_start);
  /* Trim the given PWD and PATH strings. */
  while (pth_length) {
   end = pth_start + pth_length;
   ch = utf8_readchar_rev((char const **)&end,pth_start);
   if (!DeeUni_IsSpace(ch)) break;
   pth_length = end - pth_start;
  }
  for (;;) {
   /* Skip trailing slashes and spaces in the PWD. */
   if (pwd_length) {
    end = pwd_start + pwd_length;
    ch = utf8_readchar_rev((char const **)&end,pwd_start);
    if (!DeeUni_IsSpace(ch) && !ISSEP(ch)) break;
    pwd_length = end - pwd_start;
    continue;
   }
   if (pwd_length >= 2 &&
       pwd_start[pwd_length-1] == '.') {
    char *pwd_end = pwd_start + pwd_length - 1;
    if (pwd_end[-1] == '.') --pwd_end;
    while (pwd_end > pwd_start) {
     end = pwd_end;
     ch = utf8_readchar_rev((char const **)&end,pwd_start);
     if (!DeeUni_IsSpace(ch)) break;
     pwd_end = end;
    }
    if (ISSEP(pwd_end[-1])) {
     /* Trailing self-directory or parent directory reference in PWD. */
     if (pwd_start[pwd_length-2] != '.') {
      pwd_length = (size_t)(pwd_end - pwd_start) - 1;
      continue;
     }
     /* Parent-directory reference. */
     for (;;) {
      if (--pwd_end == pwd_start)
          goto done_trim_pwd;
      if (!ISSEP(pwd_end[-1]))
          continue;
      pwd_length = (size_t)(pwd_end - pwd_start) - 1;
      break;
     }
     continue;
    }
   }
done_trim_pwd:

   /* Skip leading slashes and spaces within the path. */
   if (pth_length) {
    end = pth_start;
    ch = utf8_readchar((char const **)&end,pth_start + pth_length);
    if (!DeeUni_IsSpace(ch) && !ISSEP(ch)) break;
    pth_length -= (size_t)(end - pth_start);
    pth_start = end;
   }
   if (pth_length >= 2 && pth_start[0] == '.') {
    char *next_slash = pth_start+1;
    if (*next_slash == '.') ++next_slash;
    for (;;) {
     end = next_slash;
     ch = utf8_readchar((char const **)&end,pth_start + pth_length);
     if (!DeeUni_IsSpace(ch)) break;
     next_slash = end;
    }
    if (ISSEP(*next_slash)) {
     char *pwd_slash;
     if (pth_start[1] != '.') {
      /* Skip leading self-directory references. */
      pth_length = (size_t)((pth_start+pth_length)-next_slash);
      pth_start  = next_slash;
      continue;
     }
     /* Skip leading parent-directory references. */
     pwd_slash = pwd_start+pwd_length;
     if (pwd_slash == pwd_start) goto done_join;
     for (;;) {
      if (--pwd_slash == pwd_start)
          goto done_join;
      if (!ISSEP(*pwd_slash)) continue;
      pwd_length = (size_t)(pwd_slash-pwd_start);
      pth_length = (size_t)((pth_start+pth_length)-next_slash);
      pth_start  = next_slash;
      break;
     }
     continue;
    }
   }
   break;
  }
done_join:
  /* Special optimizations when one part wasn't used at all. */
  if (!pwd_length && pth_length == DeeString_SIZE(path)) {
   result = path;
   Dee_Incref(result);
  } else if (!pth_length && pwd_length == DeeString_SIZE(pwd)) {
   result = pwd;
   Dee_Incref(result);
  } else {
   /* Create the result buffer.
    * result = string(pwd_start,pwd_length) + "/" + string(pth_start,pth_length) */
   result = DeeString_NewBuffer(pwd_length+1+pth_length);
   if unlikely(!result) goto err_pwd;
   dst = DeeString_STR(result);
   memcpy(dst,pwd_start,pwd_length*sizeof(char));
   dst += pwd_length;
   *dst++ = SEP;
   memcpy(dst,pth_start,pth_length*sizeof(char));
   result = DeeString_SetUtf8(result,STRING_ERROR_FIGNORE);
  }
 }
done:
 Dee_XDecref(pwd);
 return result;
err_pwd:
 Dee_XDecref(pwd);
err:
 return NULL;
}


#define MAX_UPREF_COPY  4
PRIVATE char const aligned_upref_buffer[MAX_UPREF_COPY][3] = {
    { '.', '.', SEP }, { '.', '.', SEP },
    { '.', '.', SEP }, { '.', '.', SEP }
};

INTERN DREF DeeObject *DCALL
fs_pathrel(DeeObject *__restrict path, DeeObject *pwd) {
 DREF DeeObject *result; size_t uprefs,pth_length;
 char *pth_begin,*pth_iter,*pth_end,*dst,*next;
 char *pwd_begin,*pwd_iter,*pwd_end; uint32_t a,b;
 ASSERT_OBJECT_TYPE_EXACT(path,&DeeString_Type);
 /* Quick check: If the given path isn't absolute,
  *              then we've got nothing to do. */
 if (!ISABS_STR(path)) {
  if (!pwd)
       return_reference_(path);
  /* If a custom PWD is given, then we have to do this double-callback. */
  path = fs_pathabs(path,NULL);
  if unlikely(!path) goto err;
  ASSERT(ISABS_STR(path));
  /* Now that the path is absolute, re-invoke the relative path creator. */
  result = fs_pathrel(path,pwd);
  Dee_Decref(path);
  return result;
 }
 /* If the given `pwd' isn't absolute, make it using the real PWD. */
 if (pwd && !ISABS_STR(pwd)) {
  pwd = fs_pathabs(pwd,NULL);
  if unlikely(!pwd) goto err;
 } else if (pwd) {
  Dee_Incref(pwd);
 } else {
  /* Lookup the real PWD. */
  pwd = fs_getcwd();
  if unlikely(!pwd) goto err;
 }
 pth_iter = DeeString_AsUtf8(path);
 if unlikely(!pth_iter) goto err;
 pwd_iter = DeeString_AsUtf8(pwd);
 if unlikely(!pwd_iter) goto err;
 pth_end = pth_iter + WSTR_LENGTH(pth_iter);
 pwd_end = pwd_iter + WSTR_LENGTH(pwd_iter);
#ifdef CONFIG_HOST_WINDOWS
 /* Match the drive prefix. */
 for (;;) {
  a = utf8_readchar((char const **)&pth_iter,pth_end);
  b = utf8_readchar((char const **)&pwd_iter,pwd_end);
  if ((a != b) && (DeeUni_ToUpper(a) != DeeUni_ToUpper(b))) {
   /* Different drives (return the given path as-is) */
   result = path;
   Dee_Incref(result);
   goto done;
  }
  /* Stop when the path prefix is found. */
  if likely(a == ':')
     break;
  /* This shouldn't really happen, but we've got no guaranty that it can't. */
  if unlikely(!a)
     goto return_single_dot;
 }
#endif
 /* Jump here to start, so that we automatically
  * skip leading space and slashes. */
 goto continue_after_sep;

 for (;;) {
  a = utf8_readchar((char const **)&pth_iter,pth_end);
  b = utf8_readchar((char const **)&pwd_iter,pwd_end);
#ifdef CONFIG_HOST_WINDOWS
  a = DeeUni_ToUpper(a);
  b = DeeUni_ToUpper(b);
#endif
  if (ISSEP(a)) {
   /* Align differing space in `b' */
   while (DeeUni_IsSpace(b)) {
    b = utf8_readchar((char const **)&pwd_iter,pwd_end);
   }
   if (!ISSEP(b)) {
    if (!b && pwd_iter == pwd_end+1) --pwd_iter;
    else break;
   }
continue_after_sep:
   while (pth_iter < pth_end) {
    next = pth_iter;
    a = utf8_readchar((char const **)&next,pth_end);
    if (!DeeUni_IsSpace(a) && !ISSEP(a)) break;
    pth_iter = next;
   }
   while (pwd_iter < pwd_end) {
    next = pwd_iter;
    b = utf8_readchar((char const **)&next,pwd_end);
    if (!DeeUni_IsSpace(a) && !ISSEP(a)) break;
    pwd_iter = next;
   }
continue_after_sep_sp:
   /* Keep track the last shared folder. */
   pth_begin = pth_iter;
   pwd_begin = pwd_iter;
   continue;
  }
  if (ISSEP(b)) {
   /* Align differing space in `a' */
   while (DeeUni_IsSpace(a)) {
    b = utf8_readchar((char const **)&pth_iter,pth_end);
   }
   if (!ISSEP(a)) {
    if (!a && pth_iter == pth_end+1) --pth_iter;
    else break;
   }
   goto continue_after_sep;
  }
  if (a != b) {
   if (DeeUni_IsSpace(a) && DeeUni_IsSpace(b)) {
    /* Special handling for differing space characters surrounding slashes. */
    while (pth_iter < pth_end) {
     next = pth_iter;
     a = utf8_readchar((char const **)&next,pth_end);
     if (!DeeUni_IsSpace(a) && !ISSEP(a)) break;
     pth_iter = next;
    }
    while (pwd_iter < pwd_end) {
     next = pwd_iter;
     b = utf8_readchar((char const **)&next,pwd_end);
     if (!DeeUni_IsSpace(a) && !ISSEP(a)) break;
     pwd_iter = next;
    }
    /* If a slash follows on both sides, continue normally. */
    if (ISSEP(a) && ISSEP(b))
        goto continue_after_sep_sp;
   }
   break;
  }
  /* NOTE: When `a' is NUL, we also know that `b' is NUL
   *       because `a != b' breaks out of the loop, so we
   *       wouldn't get here if `a' didn't equal `b'. */
  if (!a && (pth_iter == pth_end + 1 ||
             pwd_iter == pwd_end + 1))
       break;
 }
 /* Count the amount of folders remaining in 'cwd'
  * >> Depending on it's about, we have to add
  *    additional `..SEP' prefixes to the resulting path. */
 uprefs = 0;
 while (pwd_begin < pwd_end) {
  b = utf8_readchar((char const **)&pwd_begin,pwd_end);
  if (ISSEP(b)) continue;
  ++uprefs;
  do b = utf8_readchar((char const **)&pwd_begin,pwd_end);
  while (ISSEP(b) || DeeUni_IsSpace(b));
 }
#if 1 /* Small, optional memory-reuse optimization */
 if (pth_begin == DeeString_STR(path) && !uprefs)
     return_reference_((DeeObject *)path);
#endif
 /* Strip trailing whitespace from `path' */
 while (pth_end > pth_begin) {
  next = pth_end;
  a = utf8_readchar_rev((char const **)&next,pth_begin);
  if (!DeeUni_IsSpace(a)) break;
  pth_end = next;
 }
 if (!uprefs && pth_end == pth_begin) {
  /* Special case: The 2 given paths match each other exactly. */
#ifdef CONFIG_HOST_WINDOWS
return_single_dot:
#endif
  result = (DREF DeeObject *)&str_single_dot;
  Dee_Incref(result);
  goto done;
 }
 /* Create the string that'll be returned. */
 pth_length = (size_t)(pth_end-pth_begin);
 result = DeeString_NewBuffer(uprefs * COMPILER_LENOF(aligned_upref_buffer[0])+
                              pth_length);
 if unlikely(!result) goto err_pwd;
 dst = DeeString_STR(result);
 while (uprefs) {
  size_t part = MIN(uprefs,MAX_UPREF_COPY);
  memcpy(dst,(void *)aligned_upref_buffer,
         part*COMPILER_LENOF(aligned_upref_buffer[0])*sizeof(char));
  dst += part*COMPILER_LENOF(aligned_upref_buffer[0]);
  uprefs -= part;
 }
 /* With upwards references out of the way, copy the remainder of the given path. */
 memcpy(dst,pth_begin,pth_length*sizeof(char));
 result = DeeString_SetUtf8(result,STRING_ERROR_FSTRICT);
done:
 Dee_Decref(pwd);
 return result;
err_pwd:
 Dee_Decref(pwd);
err:
 return NULL;
}

INTERN DREF DeeObject *DCALL
fs_pathjoin(size_t pathc, DeeObject **__restrict pathv) {
 size_t i; char nextsep = SEP;
 struct unicode_printer printer;
 /* Special case: Return `.' when no paths are given. */
 if unlikely(!pathc)
    return_reference_((DeeObject *)&str_single_dot);
 /* Special case: Return `.' when no paths are given. */
 unicode_printer_init(&printer);
 for (i = 0; i < pathc; ++i) {
  char const *begin,*end,*next; uint32_t ch;
  DeeObject *path = pathv[i];
  /* Validate that the path is actually a string. */
  if (DeeObject_AssertTypeExact(path,&DeeString_Type))
      goto err;
  begin = DeeString_AsUtf8(path);
  if unlikely(!begin) goto err;
  end = begin+WSTR_LENGTH(begin);
  while (begin < end) {
   next = begin;
   ch = utf8_readchar(&next,end);
   if (!DeeUni_IsSpace(ch) && !ISSEP(ch)) break;
   begin = next;
  }
  while (end > begin) {
   next = end;
   ch = utf8_readchar_rev(&next,end);
   if (!DeeUni_IsSpace(ch) && !ISSEP(ch)) break;
   end = next;
  }
  if (!UNICODE_PRINTER_ISEMPTY(&printer)) {
   /* Not the first non-empty path (figure out how, and print a separator). */
   if (ISSEP(begin[-1])) {
    --begin; /* Re-use this separator. */
   } else {
    /* Manually print a separator. */
    if (unicode_printer_putascii(&printer,nextsep)) goto err;
   }
  }
  if (unicode_printer_print(&printer,begin,(size_t)(end-begin)) < 0)
      goto err;
  /* Set the separator that should be preferred for the next part. */
  nextsep = end[0];
  if (!ISSEP(nextsep))
       nextsep = SEP;
 }
 return unicode_printer_pack(&printer);
err:
 unicode_printer_fini(&printer);
 return NULL;
}


INTERN DREF DeeObject *DCALL
fs_pathexpand(DeeObject *__restrict path, uint16_t options,
              DeeObject *__restrict environ_mapping) {
 struct unicode_printer printer = ASCII_PRINTER_INIT;
 char *iter,*begin,*end,*flush_start,*flush_end,*iter_next;
 uint32_t ch;
 ASSERT_OBJECT(environ_mapping);
 ASSERT_OBJECT_TYPE_EXACT(path,&DeeString_Type);
 begin = DeeString_AsUtf8(path);
 if unlikely(!begin) goto err;
 end = begin + WSTR_LENGTH(begin);
 ASSERT(*end == '\0');
 if (options & FS_EXPAND_FABS) {
  /* Strip leading space. */
  while (begin < end) {
   iter_next = begin;
   ch = utf8_readchar((char const **)&iter_next,end);
   if (!DeeUni_IsSpace(ch)) break;
   begin = iter_next;
  }
  iter_next = begin;
  ch = utf8_readchar((char const **)&iter_next,end);
#ifdef CONFIG_HOST_WINDOWS
  if (!ch || utf8_readchar((char const **)&iter_next,end) != ':')
#else
  if (ch != '/')
#endif
  {
   /* Print the current working directory when the given path isn't absolute. */
   if (fs_printcwd(&printer)) goto err;
#ifdef CONFIG_HOST_WINDOWS
   /* Handle drive-relative paths. */
   if (ISSEP(begin[0]) && UNICODE_PRINTER_LENGTH(&printer)) {
    size_t new_length = 0;
    while (new_length < UNICODE_PRINTER_LENGTH(&printer) &&
           UNICODE_PRINTER_GETCHAR(&printer,new_length) != SEP)
           ++new_length;
    unicode_printer_truncate(&printer,new_length);
    /* Strip leading slashes. */
    while (begin < end) {
     iter = begin;
     ch = utf8_readchar((char const **)&iter,end);
     if (!ISSEP(ch) && !DeeUni_IsSpace(ch)) break;
     begin = iter;
    }
   }
#endif
  }
 }
 flush_start = iter = begin;
next:
 ch = *iter++;
 switch (ch) {

 {
  dssize_t error;
  char *home_start;
 case '~':
  if (!(options&FS_EXPAND_FHOME))
        goto next;
  if (iter > flush_start) {
   if (unicode_printer_print(&printer,flush_start,
                            (size_t)(iter-flush_start)-1) < 0)
       goto err;
  }
  home_start = iter-1;
  if (DeeUni_IsSymStrt(*iter)) {
   /* Special case: The home path of a specific user. */
   DREF DeeObject *user_ob,*homepath;
   char *name_start = iter;
   do ++iter; while (DeeUni_IsSymCont(*iter));
   user_ob = DeeObject_Newf(&DeeUser_Type,"$s",
                           (size_t)(iter - name_start),
                            name_start);
   if unlikely(!user_ob) {
    if ((options&FS_EXPAND_FNOFAIL) &&
        (DeeError_Catch(&DeeError_SystemError) ||
         DeeError_Catch(&DeeError_ValueError)))
         goto home_lookup_failed;
    goto err;
   }
   homepath = DeeObject_CallAttrString(user_ob,"home",0,NULL);
   Dee_Decref(user_ob);
   if unlikely(!homepath) {
    if ((options&FS_EXPAND_FNOFAIL) &&
        (DeeError_Catch(&DeeError_SystemError) ||
         DeeError_Catch(&DeeError_ValueError)))
         goto home_lookup_failed;
    goto err;
   }
   /* Print the home path. */
   error = DeeObject_Print(homepath,
                          (dformatprinter)&unicode_printer_print,
                           &printer);
   Dee_Decref(homepath);
   if unlikely(error < 0) goto err;
   error = 0;
  } else {
   error = fs_printhome_u(&printer,!!(options&FS_EXPAND_FNOFAIL));
   if unlikely(error < 0) goto err;
  }
  flush_start = iter;
  if (error) {
home_lookup_failed:
   /* Lookup failed, but errors are being ignored. */
   flush_start = home_start;
  }
  goto next;
 }

 {
  char *env_start;
  char *name_start;
  char *name_end;
  int error;
 case '$':
  if (!(options&FS_EXPAND_FVARS))
        goto next;
  env_start = iter-1;
  if (env_start > flush_start) {
   if (unicode_printer_print(&printer,flush_start,
                            (size_t)(env_start-flush_start)) < 0)
       goto err;
  }
  if (*iter == '{') {
   name_start = ++iter;
   while (*iter != '}' && iter != end) ++iter;
   name_end = iter;
   if (iter != end) ++iter;
   flush_start = iter;
  } else {
   name_start  = iter;
   while (iter < end) {
    iter_next = iter;
    ch = utf8_readchar((char const **)&iter_next,end);
    if (!DeeUni_IsSymCont(*iter)) break;
    iter = iter_next;
   }
   name_end    = iter;
   flush_start = iter;
  }
print_env:
  if (environ_mapping == &DeeEnv_Singleton) {
   if (!DeeObject_IsShared(path) || !*name_end) {
    char temp = *name_end;
    *name_end = '\0';
    error = fs_printenv(name_start,&printer,!!(options&FS_EXPAND_FNOFAIL));
    *name_end = temp;
   } else {
    /* Must use a temporary string. */
    size_t length = (size_t)(name_end-name_start);
    char *temp = (char *)Dee_Malloc((length+1)*sizeof(char));
    if unlikely(!temp) goto err;
    memcpy(temp,name_start,length*sizeof(char));
    temp[length] = '\0';
    /* Now print the environment variable. */
    error = fs_printenv(name_start,&printer,!!(options&FS_EXPAND_FNOFAIL));
    Dee_Free(temp);
   }
   if unlikely(error < 0) goto err;
   if (error) flush_start = env_start;
  } else {
   /* Do a key-lookup in the given variable mapping. */
   DREF DeeObject *key,*item;
   key = DeeString_NewUtf8(name_start,(size_t)(name_end - name_start),
                           STRING_ERROR_FSTRICT);
   if unlikely(!key) goto err;
   item = DeeObject_GetItem(environ_mapping,key);
   Dee_Decref(key);
   if unlikely(!item) {
    /* When the item wasn't found, try to handle `KeyError'. */
    if (!(options&FS_EXPAND_FNOFAIL) ||
         !DeeError_Catch(&DeeError_KeyError))
          goto err;
    flush_start = env_start;
   } else {
    dssize_t temp;
    /* Print the item in place of the variable reference. */
    temp = unicode_printer_printobject(&printer,item);
    Dee_Decref(item);
    if unlikely(temp < 0) goto err;
   }
  }
  goto next;

 case '%':
  if (!(options&FS_EXPAND_FWVARS))
        goto next;
  env_start = iter-1;
  if (env_start > flush_start) {
   if (unicode_printer_print(&printer,flush_start,
                            (size_t)(env_start - flush_start)) < 0)
       goto err;
  }
  name_start = iter++;
  while (*iter != '%' && iter != end) ++iter;
  name_end = iter;
  if (iter != end) ++iter;
  flush_start = iter;
  goto print_env;
 }

 /* NOTE: The following part has been copied from `make_absolute' found
  *       in deemon's core implementation file: `execute/modpath.c'
  *       If a bug is found in this code, it should be fixed here, as
  *       well as within the core. */
 {
  char const *sep_loc;
  bool did_print_sep;
#if SEP != '/'
 case '/':
#endif
 case SEP:
 case '\0':
  /* Delete slashes and expand paths. */
  if (!(options&FS_EXPAND_FPATH)) {
   if (iter == end+1)
       goto done;
   goto next;
  }
  sep_loc = flush_end = iter-1;
  /* Skip multiple slashes and whitespace following a path separator. */
  while (iter < end) {
   iter_next = iter;
   ch = utf8_readchar((char const **)&iter_next,end);
   if (!DeeUni_IsSpace(ch) && !ISSEP(ch)) break;
   iter = iter_next;
  }
  while (flush_end > flush_start) {
   iter_next = flush_end;
   ch = utf8_readchar_rev((char const **)&iter_next,flush_start);
   if (!DeeUni_IsSpace(ch)) break;
   flush_end = iter_next;
  }
  /* Analyze the last path portion for being a special name (`.' or `..') */
  if (flush_end[-1] == '.') {
   if (flush_end[-2] == '.' && flush_end-2 == flush_start) {
    size_t printer_length,new_end;
    /* Parent-directory-reference. */
    /* Delete the last directory that was written. */
    printer_length = UNICODE_PRINTER_LENGTH(&printer);
    if (!printer_length) goto do_flush_after_sep;
    if (UNICODE_PRINTER_GETCHAR(&printer,printer_length - 1) == SEP)
        --printer_length;
    new_end = (size_t)unicode_printer_memrchr(&printer,SEP,0,printer_length);
    if (new_end == (size_t)-1) goto do_flush_after_sep;
    ++new_end;
    /* Truncate the valid length of the printer to after the previous slash. */
    unicode_printer_truncate(&printer,new_end);
    goto done_flush;
   } else if (flush_end[-3] == SEP) {
    /* Parent-directory-reference. */
    char *new_end;
    ASSERT((flush_end-3) >= flush_start);
    new_end = (char *)memrchr(flush_start,SEP,
                             (size_t)((flush_end-3)-flush_start));
    if (!new_end) goto done_flush;
    flush_end = new_end+1; /* Include the previous sep in this flush. */
    if (unicode_printer_print(&printer,flush_start,
                             (size_t)(flush_end-flush_start)) < 0)
        goto err;
    goto done_flush;
   } else if (flush_end-1 == flush_start) {
    /* Self-directory-reference. */
done_flush:
    flush_start = iter;
    goto done_flush_nostart;
   } else if (flush_end[-2] == SEP &&
              flush_end-2 >= flush_start) {
    /* Self-directory-reference. */
    flush_end -= 2;
   }
  }
do_flush_after_sep:
  /* Check if we need to fix anything */
  if (flush_end == iter-1
#ifdef CONFIG_HOST_WINDOWS
      && (*sep_loc == SEP || iter == end+1)
#endif
      )
      goto done_flush_nostart;
  /* If we can already include a slash in this part, do so. */
  did_print_sep = false;
  if (sep_loc == flush_end
#ifdef CONFIG_HOST_WINDOWS
      && (*sep_loc == SEP)
#endif
      )
      ++flush_end,did_print_sep = true;
  /* Flush everything prior to the path. */
  ASSERT(flush_end >= flush_start);
  if (unicode_printer_print(&printer,flush_start,
                           (size_t)(flush_end-flush_start)) < 0)
      goto err;
  flush_start = iter;
  if (did_print_sep)
   ; /* The slash has already been been printed: `foo/ bar' */
  else if (sep_loc == iter-1
#ifdef CONFIG_HOST_WINDOWS
           && (!*sep_loc || *sep_loc == SEP)
#endif
           )
   --flush_start; /* The slash will be printed as part of the next flush: `foo /bar' */
  else {
   /* The slash must be printed explicitly: `foo / bar' */
   if (unicode_printer_putascii(&printer,SEP) < 0)
       goto err;
  }
done_flush_nostart:
  if (iter == end+1)
      goto done;
  goto next;
 }
 default: goto next;
 }
done:
 --iter;
 /* Print the remainder. */
 if (iter > flush_start) {

  /* Check for special case: The printer was never used.
   * If this is the case, we can simply re-return the given path. */
  if (UNICODE_PRINTER_ISEMPTY(&printer)) {
#ifdef CONFIG_UNICODE_PRINTER_MUSTFINI_IF_EMPTY
   unicode_printer_fini(&printer);
#endif
#ifdef CONFIG_HOST_WINDOWS
   if (options & FS_EXPAND_FCASE) {
    /* Check for lowercase characters. */
    iter = begin = DeeString_AsUtf8(path);
    if unlikely(!iter) goto err_without_printer;
    while (iter < end) {
     ch = utf8_readchar((char const **)&iter,end);
     if (DeeUni_IsLower(ch))
         goto return_upper;
    }
    return_reference_(path);
return_upper:
    return DeeObject_CallAttrString(path,"upper",0,NULL);
   }
#endif
   return_reference_(path);
  }
  /* Actually print the remainder. */
  if (unicode_printer_print(&printer,flush_start,
                           (size_t)(iter-flush_start)) < 0)
      goto err;
 }
 /* Pack everything together. */
 path = unicode_printer_pack(&printer);
 if unlikely(!path) goto err_without_printer;
 if (options&FS_EXPAND_FREL) {
  /* Force the path to become relative. */
  DREF DeeObject *new_path;
  new_path = fs_pathrel(path,NULL);
  Dee_Decref(path);
  path = new_path;
 }
#ifdef CONFIG_HOST_WINDOWS
 if ((options&FS_EXPAND_FCASE) && path) {
  DREF DeeObject *result;
  /* Convert everything to upper-case. */
  result = DeeObject_CallAttrString(path,"upper",0,NULL);
  Dee_Decref(path);
  return result;
 }
#endif /* CONFIG_HOST_WINDOWS */
 return path;
err:
 unicode_printer_fini(&printer);
err_without_printer:
 return NULL;
}


INTERN int DCALL
fs_getchmod_mask(DeeObject *__restrict mode,
                 uint16_t *__restrict pchmod_mask,
                 uint16_t *__restrict pchmod_flags) {
 if (DeeString_Check(mode)) {
  /* TODO: `+r'  --> `mask=0333'; `flag=0111' */
  /* TODO: `o+r' --> `mask=0773'; `flag=0001' */
  /* You get the idea... */
 }

 /* Fallback: Apply the new flags directly. */
 *pchmod_mask = 0;
 return DeeObject_AsUInt16(mode,pchmod_flags);
}


DECL_END

#endif /* !GUARD_DEX_FS_PATH_C */
