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
#include <deemon/alloc.h>
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


/* Given 2 text pointer, return a pointer to the start of
 * the latest path segment, or re-return `pth_begin' if only
 * one, or zero segments exist:
 * >> "/foo/bar/foobar/"
 *     ^        ^      ^
 *     pth_begin|      pth_end
 *              |
 *              +- return
 * >> "/foo/bar/foobar/./"
 *     ^        ^        ^
 *     pth_begin|        pth_end
 *              |
 *              +- return
 * >> "/foo/bar/fiz/baz/../"
 *     ^        ^          ^
 *     pth_begin|          pth_end
 *              |
 *              +- return
 */
PRIVATE /*utf-8*/char *DCALL
find_last_path_segment(/*utf-8*/char *__restrict pth_begin,
                       /*utf-8*/char *__restrict pth_end) {
 char *next; uint32_t ch;
 int name_state;
 size_t count = 0;
again:
 name_state = 0;
 for (;;) {
  if (pth_begin >= pth_end) goto done;
  next = pth_end;
  ch = utf8_readchar_rev((char const **)&next,pth_begin);
  if (!ISSEP(ch) && !DeeUni_IsSpace(ch)) break;
  pth_end = next;
 }
 /* Search for the next SEP and unroll `pth_end' to point directly after it. */
 for (;;) {
  if (pth_begin >= pth_end) goto done;
  next = pth_end;
  /* TODO: special handling for unwinding `.' and `..' segments. */
  ch = utf8_readchar_rev((char const **)&next,pth_begin);
  if (ISSEP(ch)) break;
  if (ch == '.') {
   if (name_state == 0)
    name_state = 1; /* Self-directory reference. */
   else if (name_state == 1)
    name_state = 2; /* Self-directory reference. */
   else {
    name_state = -1; /* Not a special folder */
   }
  } else if (name_state >= 0) {
   if (!DeeUni_IsSpace(ch))
    name_state = -1; /* Not a special folder */
   else {
    if (name_state == 1) {
     name_state = 3; /* Self-directory reference (hard). */
    } else if (name_state == 2) {
     name_state = 4; /* Parent-directory reference (hard). */
    }
   }
  }
  pth_end = next;
 }
 switch (name_state) {
 case 2:
 case 4:
  /* Parent directory reference. */
  ++count;
  goto again;
 case 1:
 case 3:
  /* Self-directory reference. */
  goto again;
 default: break;
 }
 if (count) {
  --count;
  goto again;
 }
done:
 return pth_end;
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
 } else {
  pwd = fs_getcwd();
  if unlikely(!pwd) goto err;
 }
 {
  uint32_t ch; char *next;
  char *pwd_base,*pwd_begin,*pwd_end;
  char *pth_base,*pth_begin,*pth_end;
  pwd_base = DeeString_AsUtf8(pwd);
  if unlikely(!pwd_base) goto err_pwd;
  pth_base = DeeString_AsUtf8(path);
  if unlikely(!pth_base) goto err_pwd;
  pwd_end = (pwd_begin = pwd_base) + WSTR_LENGTH(pwd_base);
  pth_end = (pth_begin = pth_base) + WSTR_LENGTH(pth_base);
  /* Trim the given PWD and PATH strings. */
  while (pth_end > pth_begin) {
   next = pth_end;
   ch = utf8_readchar_rev((char const **)&next,pth_begin);
   if (!DeeUni_IsSpace(ch) && !ISSEP(ch)) break;
   pth_end = next;
  }
  while (pth_begin < pth_end) {
   next = pth_begin;
   ch = utf8_readchar((char const **)&next,pth_end);
   if (!DeeUni_IsSpace(ch) && !ISSEP(ch)) break;
   pth_begin = next;
  }
again_trip_paths:
  while (pwd_end > pwd_begin) {
   next = pwd_end;
   ch = utf8_readchar_rev((char const **)&next,pwd_begin);
   if (!DeeUni_IsSpace(ch) && !ISSEP(ch)) break;
   pwd_end = next;
  }
  /* Check for leading parent-/current-folder references in `pth_begin' */
  if (*pth_begin == '.') {
   bool is_parent_ref;
   next = pth_begin + 1;
   is_parent_ref = *next == '.';
   if (is_parent_ref) ++next;
   /* Check if this segment really only contains 1/2 dots. */
   while (next < pth_end) {
    ch = utf8_readchar((char const **)&next,pth_end);
    if (ISSEP(ch)) break;
    if (!DeeUni_IsSpace(ch)) goto done_merge_paths;
   }
   /* This is a special-reference segment! (skip all additional slashes/spaces) */
   pth_begin = next;
   while (pth_begin < pth_end) {
    next = pth_begin;
    ch = utf8_readchar((char const **)&next,pth_end);
    if (!ISSEP(ch) && !DeeUni_IsSpace(ch)) break;
    pth_begin = next;
   }
   if (is_parent_ref) {
    /* Must strip a trailing path segment from `pwd_begin...pwd_end' */
    pwd_end = find_last_path_segment(pwd_begin,pwd_end);
   }
   goto again_trip_paths;
  }
done_merge_paths:
  /* Special optimizations when one part wasn't used at all.
   * Also: Special handling when one of the 2 paths has gotten empty! */
  if (pwd_begin >= pwd_end) {
   if (pth_begin == pth_base &&
       pth_end == pth_base + WSTR_LENGTH(pth_base)) {
    result = path;
    Dee_Incref(result);
   } else {
    result = DeeString_NewUtf8(pth_begin,pth_end - pth_begin,
                               STRING_ERROR_FIGNORE);
   }
  } else if (pth_begin >= pth_end) {
   if (pwd_begin == pwd_base &&
       pwd_end == pwd_base + WSTR_LENGTH(pwd_base)) {
    result = pwd;
    Dee_Incref(result);
   } else {
    result = DeeString_NewUtf8(pwd_begin,pwd_end - pwd_begin,
                               STRING_ERROR_FIGNORE);
   }
  } else {
   /* Create the result buffer. */
   char *dst;
   size_t pth_length = (size_t)(pth_end - pth_begin);
   size_t pwd_length = (size_t)(pwd_end - pwd_begin);
   result = DeeString_NewBuffer(pwd_length + 1 + pth_length);
   if unlikely(!result) goto err_pwd;
   dst = DeeString_STR(result);
   memcpy(dst,pwd_begin,pwd_length*sizeof(char));
   dst += pwd_length;
   *dst++ = SEP;
   memcpy(dst,pth_begin,pth_length*sizeof(char));
   result = DeeString_SetUtf8(result,STRING_ERROR_FIGNORE);
  }
 }
 Dee_Decref(pwd);
 return result;
err_pwd:
 Dee_Decref(pwd);
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
#ifdef CONFIG_HOST_WINDOWS
 char *pth_base;
#endif
 bool is_nonempty_segment;
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
 pth_base = pth_iter;
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
    if (!b && pwd_iter >= pwd_end) {
     /* End of the pwd-string when the path-string is at a slash
      * Setup the non matching path portions:
      *  - path: everything after the current slash
      *  - pwd:  empty */
     pwd_begin = pwd_end;
     for (;;) {
      next = pth_iter;
      b = utf8_readchar((char const **)&next,pth_end);
      if (!ISSEP(b) && !DeeUni_IsSpace(b)) break;
      pth_iter = next;
     }
     pth_begin = pth_iter;
    }
    break;
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
    if (!DeeUni_IsSpace(b) && !ISSEP(b)) break;
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
   while (DeeUni_IsSpace(a))
       a = utf8_readchar((char const **)&pth_iter,pth_end);
   if (!ISSEP(a)) {
    if (!a && pth_iter >= pth_end) {
     /* End of the path-string when the pwd-string is at a slash
      * Setup the non matching path portions:
      *  - path: empty
      *  - pwd:  everything after the current slash */
     pth_begin = pth_end;
     for (;;) {
      next = pwd_iter;
      b = utf8_readchar((char const **)&next,pwd_end);
      if (!ISSEP(b) && !DeeUni_IsSpace(b)) break;
      pwd_iter = next;
     }
     pwd_begin = pwd_iter;
    }
    break;
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
     if (!DeeUni_IsSpace(b) && !ISSEP(b)) break;
     pwd_iter = next;
    }
    /* If a slash follows on both sides, continue normally. */
    if (ISSEP(a) && ISSEP(b))
        goto continue_after_sep_sp;
   }
   /* Special handling for `relpath("foo/bar","foo/bar/")' */
   if (!a && pth_iter >= pth_end) {
    next = pwd_iter;
    while (next < pwd_end) {
     b = utf8_readchar((char const **)&next,pwd_end);
     if (!DeeUni_IsSpace(b) && !ISSEP(b)) break;
    }
    if (next >= pwd_end)
        goto return_single_dot;
   }
   /* Special handling for `relpath("foo/bar/","foo/bar")' */
   if (!b && pwd_iter >= pwd_end) {
    next = pth_iter;
    while (next < pth_end) {
     a = utf8_readchar((char const **)&next,pth_end);
     if (!DeeUni_IsSpace(a) && !ISSEP(a)) break;
    }
    if (next >= pth_end)
        goto return_single_dot;
   }
   break;
  }
  /* NOTE: When `a' is NUL, we also know that `b' is NUL
   *       because `a != b' breaks out of the loop, so we
   *       wouldn't get here if `a' didn't equal `b'. */
  if (!a && (pth_iter >= pth_end ||
             pwd_iter >= pwd_end)) {
   /* If both paths are now empty, then they were equal from the get-go. */
   if (pth_iter >= pth_end && pwd_iter >= pwd_end)
       goto return_single_dot;
   break;
  }
 }
 /* Count the amount of folders remaining in 'cwd'
  * >> Depending on it's about, we have to add
  *    additional `..SEP' prefixes to the resulting path. */
 uprefs = 0;
 is_nonempty_segment = false;
continue_uprefs_normal:
 while (pwd_begin < pwd_end) {
  b = utf8_readchar((char const **)&pwd_begin,pwd_end);
  if (!ISSEP(b) || (!b && pwd_begin >= pwd_end)) {
   bool is_parent_ref;
   /* Deal with trailing `/././.'-like and `/../../..'-like paths! */
   if (b != '.') { is_nonempty_segment = true; continue; }
   is_nonempty_segment = false;
   b = utf8_readchar((char const **)&pwd_begin,pwd_end);
   is_parent_ref = b == '.';
   if (is_parent_ref) b = utf8_readchar((char const **)&pwd_begin,pwd_end);
   while (!ISSEP(b)) {
    if (!DeeUni_IsSpace(b)) goto continue_uprefs_normal;
    b = utf8_readchar((char const **)&pwd_begin,pwd_end);
   }
   /* Got a self/parent directory reference. */
   if (is_parent_ref) {
    /* Simple case: undo an upwards reference. */
    if (uprefs)
        --uprefs;
    else {
     /* relpath("E:/c/dexmon/deemon","E:/c/dexmon/../../d/unrelated");
      * RESULT: "../../c/dexmon/deemon"
      *                [][-----]
      * To implement this, we must retroactively search for the last
      * sep in the given `path' string, and revert `pth_begin' to be
      * located directly past its position.
      * The two brackets denote the portions of the input `path' that
      * had to be retrieved retroactively. */
#ifndef CONFIG_HOST_WINDOWS
     char *pth_base;
     pth_base = DeeString_AsUtf8(path);
     if unlikely(!pth_base) goto err;
#endif
     /* Skip trailing slash/space characters that had been skipped previously. */
     pth_begin = find_last_path_segment(pth_base,pth_begin);
    }
   }
   /* Skip all additional space and SEP-characters. */
   for (;;) {
    next = pwd_begin;
    b = utf8_readchar((char const **)&next,pwd_end);
    if (!ISSEP(b) && !DeeUni_IsSpace(b)) break;
    pwd_begin = next;
   }
   continue;
  }
  ++uprefs;
  is_nonempty_segment = false;
  do b = utf8_readchar((char const **)&pwd_begin,pwd_end);
  while (ISSEP(b) || DeeUni_IsSpace(b));
 }
 if (is_nonempty_segment)
     ++uprefs;

#if 1 /* Small, optional memory-reuse optimization */
 if (pth_begin == DeeString_STR(path) && !uprefs)
     return_reference_((DeeObject *)path);
#endif
 /* Strip leading slashes & whitespace from `path' */
 while (pth_begin < pth_end) {
  next = pth_begin;
  a = utf8_readchar((char const **)&next,pth_end);
  if (!ISSEP(a) && !DeeUni_IsSpace(a)) break;
  pth_begin = next;
 }
 /* Strip trailing whitespace from `path' */
 while (pth_end > pth_begin) {
  next = pth_end;
  a = utf8_readchar_rev((char const **)&next,pth_begin);
  if (!DeeUni_IsSpace(a)) break;
  pth_end = next;
 }
 if (!uprefs && pth_iter >= pth_end) {
  /* Special case: The 2 given paths match each other exactly. */
return_single_dot:
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
   ch = utf8_readchar((char const **)&next,end);
   if (!DeeUni_IsSpace(ch) && /* Don't skip leading SEPs */
      (UNICODE_PRINTER_ISEMPTY(&printer) || !ISSEP(ch)))
       break;
   begin = next;
  }
  while (end > begin) {
   next = end;
   ch = utf8_readchar_rev((char const **)&next,begin);
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
  if (unicode_printer_print(&printer,begin,(size_t)(end - begin)) < 0)
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
 /*utf-8*/char *iter,*begin,*end,*iter_next;
 /*utf-8*/char *flush_start,*flush_end;
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
   homepath = DeeObject_GetAttrString(user_ob,"home");
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
                          &unicode_printer_print,
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
  /*utf-8*/char *env_start;
  /*utf-8*/char *name_start;
  /*utf-8*/char *name_end;
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
   DREF DeeObject *item;
   item = DeeObject_GetItemStringLen(environ_mapping,name_start,
                                    (size_t)(name_end - name_start),
                                     Dee_HashUtf8(name_start,(size_t)(name_end - name_start)));
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
