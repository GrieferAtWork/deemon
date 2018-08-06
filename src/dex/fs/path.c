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
#include <deemon/bool.h>
#include <deemon/error.h>
#include <hybrid/minmax.h>

#include <string.h>

DECL_BEGIN

#ifdef CONFIG_HOST_WINDOWS
#define SEP              '\\'
#define SEP_S            "\\"
#define ISSEP(x) ((x) == '\\' || (x) == '/')
#define ISABS(x) ((x)[0] && (x)[1] == ':')
#else
#define SEP              '/'
#define SEP_S            "/"
#define ISSEP(x) ((x) == '/')
#define ISABS(x) ((x)[0] == '/')
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
 tailsep = (char *)memrchr(DeeString_STR(path),'/',
                           DeeString_SIZE(path)*
                           sizeof(char));
#ifdef CONFIG_HOST_WINDOWS
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
 tailsep = (char *)memrchr(DeeString_STR(path),'/',
                           DeeString_SIZE(path)*
                           sizeof(char));
#ifdef CONFIG_HOST_WINDOWS
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
 tailsep = (char *)memrchr(DeeString_STR(path),'/',
                           DeeString_SIZE(path)*
                           sizeof(char));
#ifdef CONFIG_HOST_WINDOWS
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
 return ISABS(DeeString_STR(path));
}
INTERN DREF DeeObject *DCALL
fs_pathabs(DeeObject *__restrict path, DeeObject *pwd) {
 DREF DeeObject *result;
 ASSERT_OBJECT_TYPE_EXACT(path,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT_OPT(pwd,&DeeString_Type);
 /* Quick check: If the given path already is absolute,
  *              then we've got nothing to do. */
 if (ISABS(DeeString_STR(path))) {
  if (!pwd)
       return_reference_(path);
  /* If a custom PWD is given, then we have to do this double-callback. */
  path = fs_pathrel(path,NULL);
  if unlikely(!path) goto err;
  ASSERT(!ISABS(DeeString_STR(path)));
  result = fs_pathabs(path,pwd);
  Dee_Decref(path);
  return result;
 }
 /* If the given `pwd' isn't absolute, make it using the real PWD. */
 if (pwd && !ISABS(DeeString_STR(pwd))) {
  pwd = fs_pathabs(pwd,NULL);
  if unlikely(!pwd) goto err;
 } else if (pwd) {
  Dee_Incref(pwd);
 }
 if (!pwd) {
  /* Use a printer to take advantage of `fs_printcwd()' */
  struct ascii_printer printer;
  ascii_printer_init(&printer);
  if (fs_printcwd(&printer))
      goto err_printer;
  if unlikely(!printer.ap_string)
     return_reference_(path); /* ??? */
  /* Append a trailing backslash if the given path doesn't start with one. */
  if (printer.ap_string->s_str[printer.ap_length-1] != SEP &&
     (DeeString_SIZE(path) && !ISSEP(DeeString_STR(path)[0])) &&
      ascii_printer_putc(&printer,SEP)) goto err_printer;
  /* Print the given path. */
  if (ascii_printer_print(&printer,
                            DeeString_STR(path),
                            DeeString_SIZE(path)) < 0)
      goto err_printer;
  result = ascii_printer_pack(&printer);
  if unlikely(!result) goto err_printer;
  goto done;
err_printer:
  ascii_printer_fini(&printer);
  goto err_pwd;
 }
 /* Simply concat the 2 paths and insert a slash in between (if necessary). */
 {
  char *dst,*pwd_start,*pth_start,ch;
  size_t pwd_length,pth_length;
  pwd_start = DeeString_STR(pwd),pwd_length = DeeString_SIZE(pwd);
  pth_start = DeeString_STR(path),pth_length = DeeString_SIZE(path);
  /* Trim the given PWD and PATH strings. */
  while (pth_length && DeeUni_IsSpace(pth_start[pth_length-1]))
         --pth_length;
  for (;;) {
   /* Skip trailing slashes and spaces in the PWD. */
   if (pwd_length &&
      (ch = pwd_start[pwd_length-1],
       DeeUni_IsSpace(ch) || ISSEP(ch))) {
    --pwd_length;
    continue;
   }
   if (pwd_length >= 2 &&
       pwd_start[pwd_length-1] == '.') {
    char *pwd_end = pwd_start+pwd_length-1;
    if (pwd_end[-1] == '.') --pwd_end;
    while (pwd_end != pwd_start &&
           DeeUni_IsSpace(pwd_end[-1])) --pwd_end;
    if (ISSEP(pwd_end[-1])) {
     /* Trailing self-directory or parent directory reference in PWD. */
     if (pwd_start[pwd_length-2] != '.') {
      pwd_length = (size_t)(pwd_end-pwd_start)-1;
      continue;
     }
     /* Parent-directory reference. */
     for (;;) {
      if (--pwd_end == pwd_start)
          goto done_trim_pwd;
      if (!ISSEP(pwd_end[-1]))
          continue;
      pwd_length = (size_t)(pwd_end-pwd_start)-1;
      break;
     }
     continue;
    }
   }
done_trim_pwd:

   /* Skip leading slashes and spaces within the path. */
   if (pth_length &&
      (ch = pth_start[0],DeeUni_IsSpace(ch) || ISSEP(ch))) {
    --pth_length;
    ++pth_start;
    continue;
   }
   if (pth_length >= 2 &&
       pth_start[0] == '.') {
    char *next_slash = pth_start+1;
    if (*next_slash == '.') ++next_slash;
    while (DeeUni_IsSpace(*next_slash)) ++next_slash;
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
   /* Create the result buffer. */
   result = DeeString_NewBuffer(pwd_length+1+pth_length);
   if unlikely(!result) goto err_pwd;
   dst = DeeString_STR(result);
   memcpy(dst,pwd_start,pwd_length*sizeof(char));
   dst += pwd_length;
   *dst++ = SEP;
   memcpy(dst,pth_start,pth_length*sizeof(char));
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
 char *pth_begin,*pth_iter,*pth_end,*dst;
 char *pwd_begin,*pwd_iter,*pwd_end;
 ASSERT_OBJECT_TYPE_EXACT(path,&DeeString_Type);
 /* Quick check: If the given path isn't absolute,
  *              then we've got nothing to do. */
 if (!ISABS(DeeString_STR(path))) {
  if (!pwd)
       return_reference_(path);
  /* If a custom PWD is given, then we have to do this double-callback. */
  path = fs_pathabs(path,NULL);
  if unlikely(!path) goto err;
  ASSERT(ISABS(DeeString_STR(path)));
  /* Now that the path is absolute, re-invoke the relative path creator. */
  result = fs_pathrel(path,pwd);
  Dee_Decref(path);
  return result;
 }
 /* If the given `pwd' isn't absolute, make it using the real PWD. */
 if (pwd && !ISABS(DeeString_STR(pwd))) {
  pwd = fs_pathabs(pwd,NULL);
  if unlikely(!pwd) goto err;
 } else if (pwd) {
  Dee_Incref(pwd);
 } else {
  /* Lookup the real PWD. */
  pwd = fs_getcwd();
  if unlikely(!pwd) goto err;
 }
 pth_end = (pth_iter = DeeString_STR(path))+DeeString_SIZE(path);
 pwd_end = (pwd_iter = DeeString_STR(pwd)) +DeeString_SIZE(pwd);
#ifdef CONFIG_HOST_WINDOWS
 /* Match the drive prefix. */
 for (;;) {
  char a = (char)DeeUni_ToUpper(*pth_iter);
  char b = (char)DeeUni_ToUpper(*pwd_iter);
  ++pth_iter,++pwd_iter;
  if (a != b) {
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
  char a,b;
#ifdef CONFIG_HOST_WINDOWS
  a = (char)DeeUni_ToUpper(*pth_iter);
  b = (char)DeeUni_ToUpper(*pwd_iter);
#else
  a = *pth_iter;
  b = *pwd_iter;
#endif
  ++pth_iter,++pwd_iter;
  if (ISSEP(a)) {
   /* Align differing space in `b' */
   while (DeeUni_IsSpace(b)) b = *pwd_iter++;
   if (!ISSEP(b)) {
    if (!b && pwd_iter == pwd_end+1) --pwd_iter;
    else break;
   }
continue_after_sep:
   while ((a = *pth_iter,DeeUni_IsSpace(a) || ISSEP(a))) ++pth_iter;
   while ((b = *pwd_iter,DeeUni_IsSpace(b) || ISSEP(b))) ++pwd_iter;
continue_after_sep_sp:
   /* Keep track the last shared folder. */
   pth_begin = pth_iter;
   pwd_begin = pwd_iter;
   continue;
  }
  if (ISSEP(b)) {
   /* Align differing space in `a' */
   while (DeeUni_IsSpace(a)) a = *pth_iter++;
   if (!ISSEP(a)) {
    if (!a && pth_iter == pth_end+1) --pth_iter;
    else break;
   }
   goto continue_after_sep;
  }
  if (a != b) {
   if (DeeUni_IsSpace(a) && DeeUni_IsSpace(b)) {
    /* Special handling for differing space characters surrounding slashes. */
    while ((a = *pth_iter,DeeUni_IsSpace(a) || ISSEP(a))) ++pth_iter;
    while ((b = *pwd_iter,DeeUni_IsSpace(b) || ISSEP(b))) ++pwd_iter;
    /* If a slash follows on both sides, continue normally. */
    if (ISSEP(*pth_iter) && ISSEP(*pwd_iter))
        goto continue_after_sep_sp;
   }
   break;
  }
  /* NOTE: When `a' is NUL, we also know that `b' is NUL
   *       because `a != b' breaks out of the loop, so we
   *       wouldn't get here if `a' didn't equal `b'. */
  if (!a && (pth_iter == pth_end+1 ||
             pwd_iter == pwd_end+1))
       break;
 }
 /* Count the amount of folders remaining in 'cwd'
  * >> Depending on it's about, we have to add
  *    additional `..SEP' prefixes to the resulting path. */
 uprefs = 0;
 while (pwd_begin != pwd_end) {
  while (!ISSEP(*pwd_begin)) {
   if (++pwd_begin == pwd_end)
       break;
  }
  //if (pwd_begin == pwd_end)
  //    break;
  ++uprefs;
  while (ISSEP(*pwd_begin) ||
         DeeUni_IsSpace(*pwd_begin))
         ++pwd_begin;
 }
#if 1 /* Small, optional memory-reuse optimization */
 if (pth_begin == DeeString_STR(path) && !uprefs)
     return_reference_((DeeObject *)path);
#endif
 /* Strip trailing whitespace from `path' */
 while (pth_end != pth_begin &&
        DeeUni_IsSpace(pth_end[-1]))
        --pth_end;
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
 result = DeeString_NewBuffer(uprefs*COMPILER_LENOF(aligned_upref_buffer[0])+
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
 DREF DeeObject *result; size_t i;
 char nextsep = SEP;
 struct ascii_printer printer;
 /* Special case: Return `.' when no paths are given. */
 if unlikely(!pathc)
    return_reference_((DeeObject *)&str_single_dot);
 /* Special case: Return `.' when no paths are given. */
 ascii_printer_init(&printer);
 for (i = 0; i < pathc; ++i) {
  char const *begin,*end;
  DeeObject *path = pathv[i];
  /* Validate that the path is actually a string. */
  if (DeeObject_AssertTypeExact(path,&DeeString_Type))
      goto err;
  end = (begin = DeeString_STR(path))+DeeString_SIZE(path);
  while (begin != end && (DeeUni_IsSpace(*begin) || ISSEP(*begin))) ++begin;
  while (begin != end && (DeeUni_IsSpace(end[-1]) || ISSEP(end[-1]))) --end;
  if (printer.ap_length) {
   /* Not the first non-empty path (figure out how, and print a separator). */
   if (ISSEP(begin[-1])) {
    --begin; /* Re-use this separator. */
   } else {
    /* Manually print a separator. */
    if (ascii_printer_putc(&printer,nextsep)) goto err;
   }
  }
  if (ascii_printer_print(&printer,begin,(size_t)(end-begin)) < 0)
      goto err;
  /* Set the separator that should be preferred for the next part. */
  nextsep = end[0];
  if (!ISSEP(nextsep))
       nextsep = SEP;
 }
 result = ascii_printer_pack(&printer);
 if unlikely(!result) goto err;
 return result;
err:
 ascii_printer_fini(&printer);
 return NULL;
}


INTERN DREF DeeObject *DCALL
fs_pathexpand(DeeObject *__restrict path, uint16_t options,
              DeeObject *__restrict environ_mapping) {
 struct ascii_printer printer = ASCII_PRINTER_INIT;
 char *iter,*begin,*end,*flush_start,*flush_end,ch;
 ASSERT_OBJECT(environ_mapping);
 ASSERT_OBJECT_TYPE_EXACT(path,&DeeString_Type);
 /* TODO: Unicode support */
 end = (begin = DeeString_STR(path))+DeeString_SIZE(path);
 ASSERT(*end == '\0');
 if (options & FS_EXPAND_FABS) {
  /* Strip leading space. */
  while (DeeUni_IsSpace(*begin)) ++begin;
  if (!ISABS(begin)) {
   /* Print the current working directory when the given path isn't absolute. */
   if (fs_printcwd(&printer)) goto err;
#ifdef CONFIG_HOST_WINDOWS
   /* Handle drive-relative paths. */
   if (ISSEP(begin[0]) && printer.ap_length) {
    char *first_sep = printer.ap_string->s_str;
    while (*first_sep++ != SEP); /* This sep must exist because it was printed by `print_pwd()' */
    printer.ap_length = (size_t)(first_sep-printer.ap_string->s_str);
    /* Strip leading slashes. */
    while (ISSEP(begin[0]) || DeeUni_IsSpace(begin[0])) ++begin;
   }
#endif
  }
 }
 flush_start = iter = begin;
next:
 ch = *iter++;
 switch (ch) {

 {
  int error;
  char *home_start;
 case '~':
  if (!(options&FS_EXPAND_FHOME))
        goto next;
  if (iter > flush_start) {
   if (ascii_printer_print(&printer,flush_start,
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
                           (size_t)(iter-name_start),
                            name_start);
   if unlikely(!user_ob) {
    if ((options&FS_EXPAND_FNOFAIL) &&
        (DeeError_Catch(&DeeError_SystemError) ||
         DeeError_Catch(&DeeError_ValueError)))
         goto home_lookup_failed;
    goto err;
   }
   homepath = DeeObject_CallAttrString(user_ob,"home",0,NULL);
   Dee_Decref(user_ob);;
   if unlikely(!homepath) {
    if ((options&FS_EXPAND_FNOFAIL) &&
        (DeeError_Catch(&DeeError_SystemError) ||
         DeeError_Catch(&DeeError_ValueError)))
         goto home_lookup_failed;
    goto err;
   }
   /* Print the home path. */
   error = (int)DeeObject_Print(homepath,
                               &ascii_printer_print,
                               &printer);
   Dee_Decref(homepath);
   if unlikely(error < 0) goto err;
   error = 0;
  } else {
   error = fs_printhome(&printer,!!(options&FS_EXPAND_FNOFAIL));
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
   if (ascii_printer_print(&printer,flush_start,
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
   while (DeeUni_IsSymCont(*iter)) ++iter;
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
   key = DeeString_NewSized(name_start,(size_t)(name_end-name_start));
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
    if (!DeeString_Check(item)) {
     DREF DeeObject *new_item;
     if (!(options&FS_EXPAND_FNOFAIL)) {
      DeeObject_TypeAssertFailed(item,&DeeString_Type);
      Dee_Decref(item);
     }
     /* Convert to a string. */
     new_item = DeeObject_Str(item);
     Dee_Decref(item);
     if unlikely(!new_item) goto err;
     item = new_item;
    }
    /* Print the item in place of the variable reference. */
    temp = ascii_printer_print(&printer,
                                 DeeString_STR(item),
                                 DeeString_SIZE(item));
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
   if (ascii_printer_print(&printer,flush_start,
                           (size_t)(env_start-flush_start)) < 0)
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
  while (iter < end && (DeeUni_IsSpace(*iter) || ISSEP(*iter))) ++iter;
  while (flush_end != flush_start &&
         DeeUni_IsSpace(flush_end[-1]))
         --flush_end;
  /* Analyze the last path portion for being a special name (`.' or `..') */
  if (flush_end[-1] == '.') {
   if (flush_end[-2] == '.' && flush_end-2 == flush_start) {
    char *new_end; size_t printer_length;
    /* Parent-directory-reference. */
    /* Delete the last directory that was written. */
    if (!printer.ap_string) goto do_flush_after_sep;
    printer_length = printer.ap_length;
    if (!printer_length) goto do_flush_after_sep;
    if (printer.ap_string->s_str[printer_length-1] == SEP)
        --printer_length;
    new_end = (char *)memrchr(printer.ap_string->s_str,SEP,printer_length);
    if (!new_end) goto do_flush_after_sep;
    ++new_end;
    /* Truncate the valid length of the printer to after the previous slash. */
    printer.ap_length = (size_t)(new_end-printer.ap_string->s_str);
#ifndef NDEBUG
    *new_end = '\0'; /* For better debug readability. */
#endif
    goto done_flush;
   } else if (flush_end[-3] == SEP) {
    /* Parent-directory-reference. */
    char *new_end;
    ASSERT((flush_end-3) >= flush_start);
    new_end = (char *)memrchr(flush_start,SEP,
                             (size_t)((flush_end-3)-flush_start));
    if (!new_end) goto done_flush;
    flush_end = new_end+1; /* Include the previous sep in this flush. */
    if (ascii_printer_print(&printer,flush_start,
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
  if (ascii_printer_print(&printer,flush_start,
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
   if (ascii_printer_putc(&printer,SEP) < 0)
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
  if (!printer.ap_length) {
   ascii_printer_fini(&printer);
#ifdef CONFIG_HOST_WINDOWS
   if (options&FS_EXPAND_FCASE) {
    DREF DeeObject *result; char *dst;
    /* Check for lowercase characters. */
    iter = DeeString_STR(path);
    for (; iter != end; ++iter) {
     if (DeeUni_IsLower(*iter))
         goto return_upper;
    }
    return_reference_(path);
return_upper:
    result = DeeString_NewBuffer(DeeString_SIZE(path));
    if unlikely(!result) goto err_without_printer;
    dst = DeeString_STR(result);
    iter = DeeString_STR(path);
    for (; iter != end; ++iter,++dst)
        *dst = (char)DeeUni_ToUpper(*iter);
    return result;
   }
#endif
   return_reference_(path);
  }
  /* Actually print the remainder. */
  if (ascii_printer_print(&printer,flush_start,
                          (size_t)(iter-flush_start)) < 0)
      goto err;
 }
 /* Pack everything together. */
 path = ascii_printer_pack(&printer);
 if unlikely(!path) goto err;
 if (options&FS_EXPAND_FREL) {
  /* Force the path to become relative. */
  DREF DeeObject *new_path;
  new_path = fs_pathrel(path,NULL);
  Dee_Decref(path);
  path = new_path;
 }
#ifdef CONFIG_HOST_WINDOWS
 if ((options&FS_EXPAND_FCASE) && path) {
  /* Convert everything to upper-case. */
  if unlikely(DeeObject_IsShared(path)) {
   DREF DeeObject *new_path;
   new_path = DeeString_NewSized(DeeString_STR(path),
                                 DeeString_SIZE(path));
   Dee_Decref(path);
   if unlikely(!new_path)
      goto err_without_printer;
   path = new_path;
  }
  end = (iter = DeeString_STR(path))+DeeString_SIZE(path);
  for (; iter != end; ++iter)
      *iter = (char)DeeUni_ToUpper(*iter);
 }
#endif /* CONFIG_HOST_WINDOWS */
 return path;
err:
 ascii_printer_fini(&printer);
#ifdef CONFIG_HOST_WINDOWS
err_without_printer:
#endif
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
