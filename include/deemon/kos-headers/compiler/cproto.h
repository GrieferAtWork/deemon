/* Copyright (c) 2017 Griefer@Work                                            *
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

/* Using a patched version of `cproto' that not only dumps function declarations,
 * but also function prototypes, this header performs some fixup to get the most
 * out of documentation generated using it.
 * usage:
 * $ cproto -sivx -D__CPROTO__ -I/include -I/include/i386-kos /include/string.h > decls_string.h
 */

#undef __cplusplus
#ifndef _ALL_SOURCE
#define _ALL_SOURCE 1
#endif

#ifndef __DOCGEN__
#define __DOCGEN__ 1
#endif

#define __CRT_DOS         1
#define __CRT_KOS         1
#define __CRT_GLC         1
#define __CRT_CYG         1

#undef __restrict
#undef __inline__
#undef __asm__
#undef __const
#undef __const__
#undef __volatile
#undef __volatile__

#define __restrict        restrict
#define __inline__        inline
enum{__asm__};
#define __attribute__(...) /* nothing */
#define __asm__(...)      /* nothing */
#define __const           const
#define __const__         const
#define __volatile        volatile
#define __volatile__(...) /* nothing */
typedef char *va_list;
#define __builtin_va_list va_list
typedef unsigned int wchar_t;

#define __LONGLONG   long long int
#define __ULONGLONG  unsigned long long int
#define __NO_ASMNAME 1


typedef struct __IO_FILE FILE;
#define __std_FILE_defined 1
#define __FILE_defined 1
#define __FILE      FILE

typedef int locale_t;
#define __locale_t  locale_t


#ifdef COLLECT_DEPS


#define __DEPNAME(name,asmname)                 name ##$__$## asmname
#define __VDEPNAME(name,asmnamef,vasmnamef)     name ##$__$## asmnamef ##$__$## vasmnamef
#define __REDIRECT(decl,attr,Treturn,cc,name,param,asmname,args)                                            decl attr Treturn (cc __DEPNAME(name,asmname)) __P(param);
#define __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,args)                                    decl attr Treturn __NOTHROW((cc __DEPNAME(name,asmname)) __P(param));
#define __REDIRECT_VOID(decl,attr,cc,name,param,asmname,args)                                               decl attr void (cc __DEPNAME(name,asmname)) __P(param);
#define __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmname,args)                                       decl attr void __NOTHROW((cc __DEPNAME(name,asmname)) __P(param));
#define __VREDIRECT(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)                decl attr Treturn (cc __VDEPNAME(name,asmnamef,vasmnamef)) __P(param);
#define __VREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)        decl attr Treturn __NOTHROW((cc __VDEPNAME(name,asmnamef,vasmnamef)) __P(param));
#define __VREDIRECT_VOID(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)                   decl attr void (cc __VDEPNAME(name,asmnamef,vasmnamef)) __P(param);
#define __VREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)           decl attr void __NOTHROW((cc __VDEPNAME(name,asmnamef,vasmnamef)) __P(param));
#define __XREDIRECT(decl,attr,Treturn,cc,name,param,asmname,code)                                           decl attr Treturn (cc __DEPNAME(name,asmname)) __P(param);
#define __XREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,code)                                   decl attr Treturn __NOTHROW((cc __DEPNAME(name,asmname)) __P(param));
#define __XREDIRECT_VOID(decl,attr,cc,name,param,asmname,code)                                              decl attr void (cc __DEPNAME(name,asmname)) __P(param);
#define __XREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmname,code)                                      decl attr void __NOTHROW((cc __DEPNAME(name,asmname)) __P(param));

#define __DEPNAME_UFS(name,asmname)                  name ##$__$## asmname ##$__$DOS$## asmname
#define __VDEPNAME_UFS(name,asmnamef,vasmnamef)      name ##$__$## asmnamef ##$__$## vasmnamef ##$__$DOS$## asmnamef ##$__$DOS$## vasmnamef
#define __DEPNAME_UFSW16(name,asmname)               name ##$__$u## asmname ##$__$DOS$## asmname
#define __DEPNAME_UFSW32(name,asmname)               name ##$__$## asmname ##$__$DOS$U## asmname
#define __VDEPNAME_UFSW16(name,asmnamef,vasmnamef)   name ##$__$u## asmnamef ##$__$u## vasmnamef ##$__$DOS$## asmnamef ##$__$DOS$## vasmnamef
#define __VDEPNAME_UFSW32(name,asmnamef,vasmnamef)   name ##$__$## asmnamef ##$__$## vasmnamef ##$__$DOS$U## asmnamef ##$__$DOS$U## vasmnamef
#define __DEPNAME_UFSDPW16(name,asmname)             name ##$__$u## asmname ##$__$DOS$_## asmname
#define __VDEPNAME_UFSDPW16(name,asmnamef,vasmnamef) name ##$__$u## asmnamef ##$__$u## vasmnamef ##$__$DOS$_## asmnamef ##$__$DOS$_## vasmnamef

#define __DEPNAME_EXCEPT_UFS(name,asmname)                  name ##$__$## asmname ##$__$X## asmname ##$__$DOS$##asmname
#define __VDEPNAME_EXCEPT_UFS(name,asmnamef,vasmnamef)      name ##$__$## asmnamef ##$__$X## asmnamef ##$__$## vasmnamef ##$__$X## vasmnamef ##$__$DOS$## asmnamef ##$__$DOS$##vasmnamef
#define __DEPNAME_EXCEPT_UFSW16(name,asmname)               name ##$__$u## asmname ##$__$DOS$## asmname ##$__$DOS$X## asmname
#define __DEPNAME_EXCEPT_UFSW32(name,asmname)               name ##$__$## asmname ##$__$X## asmname ##$__$DOS$U## asmname
#define __VDEPNAME_EXCEPT_UFSW16(name,asmnamef,vasmnamef)   name ##$__$u## asmnamef ##$__$u## vasmnamef ##$__$DOS$## asmnamef ##$__$DOS$## vasmnamef ##$__$DOS$X## asmnamef ##$__$DOS$X## vasmnamef
#define __VDEPNAME_EXCEPT_UFSW32(name,asmnamef,vasmnamef)   name ##$__$## asmnamef ##$__$X## asmnamef ##$__$## vasmnamef ##$__$X## vasmnamef ##$__$DOS$U## asmnamef ##$__$DOS$U## vasmnamef
#define __DEPNAME_EXCEPT_UFSDPW16(name,asmname)             name ##$__$u## asmname ##$__$DOS$_## asmname ##$__$DOS$X## asmname
#define __VDEPNAME_EXCEPT_UFSDPW16(name,asmnamef,vasmnamef) name ##$__$u## asmnamef ##$__$u## vasmnamef ##$__$DOS$_## asmnamef ##$__$DOS$X## asmnamef ##$__$DOS$_## vasmnamef ##$__$DOS$X## vasmnamef

#define __REDIRECT_UFS(decl,attr,Treturn,cc,name,param,args)                                                       decl attr Treturn (cc __DEPNAME_UFS(name,name)) __P(param);
#define __REDIRECT_UFS_(decl,attr,Treturn,cc,name,param,asmname,args)                                              decl attr Treturn (cc __DEPNAME_UFS(name,asmname)) __P(param);
#define __REDIRECT_UFS_NOTHROW(decl,attr,Treturn,cc,name,param,args)                                               decl attr Treturn __NOTHROW((cc __DEPNAME_UFS(name,name)) __P(param));
#define __REDIRECT_UFS_NOTHROW_(decl,attr,Treturn,cc,name,param,asmname,args)                                      decl attr Treturn __NOTHROW((cc __DEPNAME_UFS(name,asmname)) __P(param));
#define __REDIRECT_UFS_VOID(decl,attr,cc,name,param,args)                                                          decl attr void (cc __DEPNAME_UFS(name,name)) __P(param);
#define __REDIRECT_UFS_VOID_(decl,attr,cc,name,param,asmname,args)                                                 decl attr void (cc __DEPNAME_UFS(name,asmname)) __P(param);
#define __REDIRECT_UFS_VOID_NOTHROW(decl,attr,cc,name,param,args)                                                  decl attr void __NOTHROW((cc __DEPNAME_UFS(name,name)) __P(param));
#define __REDIRECT_UFS_VOID_NOTHROW_(decl,attr,cc,name,param,asmname,args)                                         decl attr void __NOTHROW((cc __DEPNAME_UFS(name,asmname)) __P(param));
#define __VREDIRECT_UFS(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)                            decl attr Treturn (cc __VDEPNAME_UFS(name,name,vasmnamef)) __P(param);
#define __VREDIRECT_UFS_(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)                  decl attr Treturn (cc __VDEPNAME_UFS(name,asmnamef,vasmnamef)) __P(param);
#define __VREDIRECT_UFS_NOTHROW(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)                    decl attr Treturn __NOTHROW((cc __VDEPNAME_UFS(name,name,vasmnamef)) __P(param));
#define __VREDIRECT_UFS_NOTHROW_(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)          decl attr Treturn __NOTHROW((cc __VDEPNAME_UFS(name,asmnamef,vasmnamef)) __P(param));
#define __VREDIRECT_UFS_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                               decl attr void (cc __VDEPNAME_UFS(name,name,vasmnamef)) __P(param);
#define __VREDIRECT_UFS_VOID_(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)                     decl attr void (cc __VDEPNAME_UFS(name,asmnamef,vasmnamef)) __P(param);
#define __VREDIRECT_UFS_VOID_NOTHROW(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                       decl attr void __NOTHROW((cc __VDEPNAME_UFS(name,name,vasmnamef)) __P(param));
#define __VREDIRECT_UFS_VOID_NOTHROW_(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)             decl attr void __NOTHROW((cc __VDEPNAME_UFS(name,asmnamef,vasmnamef)) __P(param));
#define __XREDIRECT_UFS(decl,attr,Treturn,cc,name,param,code)                                                      decl attr Treturn (cc __DEPNAME_UFS(name,name)) __P(param);
#define __XREDIRECT_UFS_(decl,attr,Treturn,cc,name,param,asmname,code)                                             decl attr Treturn (cc __DEPNAME_UFS(name,asmname)) __P(param);
#define __XREDIRECT_UFS_NOTHROW(decl,attr,Treturn,cc,name,param,code)                                              decl attr Treturn __NOTHROW((cc __DEPNAME_UFS(name,name)) __P(param));
#define __XREDIRECT_UFS_NOTHROW_(decl,attr,Treturn,cc,name,param,asmname,code)                                     decl attr Treturn __NOTHROW((cc __DEPNAME_UFS(name,asmname)) __P(param));
#define __XREDIRECT_UFS_VOID(decl,attr,cc,name,param,code)                                                         decl attr void (cc __DEPNAME_UFS(name,name)) __P(param);
#define __XREDIRECT_UFS_VOID_(decl,attr,cc,name,param,asmname,code)                                                decl attr void (cc __DEPNAME_UFS(name,asmname)) __P(param);
#define __XREDIRECT_UFS_VOID_NOTHROW(decl,attr,cc,name,param,code)                                                 decl attr void __NOTHROW((cc __DEPNAME_UFS(name,name)) __P(param));
#define __XREDIRECT_UFS_VOID_NOTHROW_(decl,attr,cc,name,param,asmname,code)                                        decl attr void __NOTHROW((cc __DEPNAME_UFS(name,asmname)) __P(param));
#define __REDIRECT_UFSW16(decl,attr,Treturn,cc,name,param,asmname,args)                                            decl attr Treturn (cc __DEPNAME_UFSW16(name,asmname)) __P(param);
#define __REDIRECT_UFSW32(decl,attr,Treturn,cc,name,param,asmname,args)                                            decl attr Treturn (cc __DEPNAME_UFSW32(name,asmname)) __P(param);
#define __REDIRECT_UFSW16_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,args)                                    decl attr Treturn __NOTHROW((cc __DEPNAME_UFSW16(name,asmname)) __P(param));
#define __REDIRECT_UFSW32_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,args)                                    decl attr Treturn __NOTHROW((cc __DEPNAME_UFSW32(name,asmname)) __P(param));
#define __REDIRECT_UFSW16_VOID(decl,attr,cc,name,param,asmname,args)                                               decl attr void (cc __DEPNAME_UFSW16(name,asmname)) __P(param);
#define __REDIRECT_UFSW32_VOID(decl,attr,cc,name,param,asmname,args)                                               decl attr void (cc __DEPNAME_UFSW32(name,asmname)) __P(param);
#define __REDIRECT_UFSW16_VOID_NOTHROW(decl,attr,cc,name,param,asmname,args)                                       decl attr void __NOTHROW((cc __DEPNAME_UFSW16(name,asmname)) __P(param));
#define __REDIRECT_UFSW32_VOID_NOTHROW(decl,attr,cc,name,param,asmname,args)                                       decl attr void __NOTHROW((cc __DEPNAME_UFSW32(name,asmname)) __P(param));
#define __VREDIRECT_UFSW16(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)                decl attr Treturn (cc __VDEPNAME_UFSW16(name,asmnamef,vasmnamef)) __P(param);
#define __VREDIRECT_UFSW32(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)                decl attr Treturn (cc __VDEPNAME_UFSW32(name,asmnamef,vasmnamef)) __P(param);
#define __VREDIRECT_UFSW16_NOTHROW(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)        decl attr Treturn __NOTHROW((cc __VDEPNAME_UFSW16(name,asmnamef,vasmnamef)) __P(param));
#define __VREDIRECT_UFSW32_NOTHROW(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)        decl attr Treturn __NOTHROW((cc __VDEPNAME_UFSW32(name,asmnamef,vasmnamef)) __P(param));
#define __VREDIRECT_UFSW16_VOID(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)                   decl attr void (cc __VDEPNAME_UFSW16(name,asmnamef,vasmnamef)) __P(param);
#define __VREDIRECT_UFSW32_VOID(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)                   decl attr void (cc __VDEPNAME_UFSW32(name,asmnamef,vasmnamef)) __P(param);
#define __VREDIRECT_UFSW16_VOID_NOTHROW(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)           decl attr void __NOTHROW((cc __VDEPNAME_UFSW16(name,asmnamef,vasmnamef)) __P(param));
#define __VREDIRECT_UFSW32_VOID_NOTHROW(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)           decl attr void __NOTHROW((cc __VDEPNAME_UFSW32(name,asmnamef,vasmnamef)) __P(param));
#define __XREDIRECT_UFSW16(decl,attr,Treturn,cc,name,param,asmname,code)                                           decl attr Treturn (cc __DEPNAME_UFSW16(name,asmname)) __P(param);
#define __XREDIRECT_UFSW32(decl,attr,Treturn,cc,name,param,asmname,code)                                           decl attr Treturn (cc __DEPNAME_UFSW32(name,asmname)) __P(param);
#define __XREDIRECT_UFSW16_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,code)                                   decl attr Treturn __NOTHROW((cc __DEPNAME_UFSW16(name,asmname)) __P(param));
#define __XREDIRECT_UFSW32_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,code)                                   decl attr Treturn __NOTHROW((cc __DEPNAME_UFSW32(name,asmname)) __P(param));
#define __XREDIRECT_UFSW16_VOID(decl,attr,cc,name,param,asmname,code)                                              decl attr void (cc __DEPNAME_UFSW16(name,asmname)) __P(param);
#define __XREDIRECT_UFSW32_VOID(decl,attr,cc,name,param,asmname,code)                                              decl attr void (cc __DEPNAME_UFSW32(name,asmname)) __P(param);
#define __XREDIRECT_UFSW16_VOID_NOTHROW(decl,attr,cc,name,param,asmname,code)                                      decl attr void __NOTHROW((cc __DEPNAME_UFSW16(name,asmname)) __P(param));
#define __XREDIRECT_UFSW32_VOID_NOTHROW(decl,attr,cc,name,param,asmname,code)                                      decl attr void __NOTHROW((cc __DEPNAME_UFSW32(name,asmname)) __P(param));
#define __REDIRECT_UFSDPW16(decl,attr,Treturn,cc,name,param,asmname,args)                                          decl attr Treturn (cc __DEPNAME_UFSDPW16(name,asmname)) __P(param);
#define __REDIRECT_UFSDPW16_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,args)                                  decl attr Treturn __NOTHROW((cc __DEPNAME_UFSDPW16(name,asmname)) __P(param));
#define __REDIRECT_UFSDPW16_VOID(decl,attr,cc,name,param,asmname,args)                                             decl attr void (cc __DEPNAME_UFSDPW16(name,asmname)) __P(param);
#define __REDIRECT_UFSDPW16_VOID_NOTHROW(decl,attr,cc,name,param,asmname,args)                                     decl attr void __NOTHROW((cc __DEPNAME_UFSDPW16(name,asmname)) __P(param));
#define __VREDIRECT_UFSDPW16(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)              decl attr Treturn (cc __VDEPNAME_UFSDPW16(name,asmnamef,vasmnamef)) __P(param);
#define __VREDIRECT_UFSDPW16_NOTHROW(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)      decl attr Treturn __NOTHROW((cc __VDEPNAME_UFSDPW16(name,asmnamef,vasmnamef)) __P(param));
#define __VREDIRECT_UFSDPW16_VOID(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)                 decl attr void (cc __VDEPNAME_UFSDPW16(name,asmnamef,vasmnamef)) __P(param);
#define __VREDIRECT_UFSDPW16_VOID_NOTHROW(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)         decl attr void __NOTHROW((cc __VDEPNAME_UFSDPW16(name,asmnamef,vasmnamef)) __P(param));
#define __XREDIRECT_UFSDPW16(decl,attr,Treturn,cc,name,param,asmname,code)                                         decl attr Treturn (cc __DEPNAME_UFSDPW16(name,asmname)) __P(param);
#define __XREDIRECT_UFSDPW16_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,code)                                 decl attr Treturn __NOTHROW((cc __DEPNAME_UFSDPW16(name,asmname)) __P(param));
#define __XREDIRECT_UFSDPW16_VOID(decl,attr,cc,name,param,asmname,code)                                            decl attr void (cc __DEPNAME_UFSDPW16(name,asmname)) __P(param);
#define __XREDIRECT_UFSDPW16_VOID_NOTHROW(decl,attr,cc,name,param,asmname,code)                                    decl attr void __NOTHROW((cc __DEPNAME_UFSDPW16(name,asmname)) __P(param));
#define __REDIRECT_EXCEPT_UFS(decl,attr,Treturn,cc,name,param,args)                                                decl attr Treturn (cc __DEPNAME_EXCEPT_UFS(name,name)) __P(param);
#define __REDIRECT_EXCEPT_UFS_(decl,attr,Treturn,cc,name,param,asmname,args)                                       decl attr Treturn (cc __DEPNAME_EXCEPT_UFS(name,asmname)) __P(param);
#define __REDIRECT_EXCEPT_UFS_XVOID(decl,attr,Treturn,cc,name,param,args)                                          decl attr Treturn (cc __DEPNAME_EXCEPT_UFS(name,name)) __P(param);
#define __REDIRECT_EXCEPT_UFS_XVOID_(decl,attr,Treturn,cc,name,param,asmname,args)                                 decl attr Treturn (cc __DEPNAME_EXCEPT_UFS(name,asmname)) __P(param);
#define __REDIRECT_EXCEPT_UFS_VOID(decl,attr,cc,name,param,args)                                                   decl attr void (cc __DEPNAME_EXCEPT_UFS(name,name)) __P(param);
#define __REDIRECT_EXCEPT_UFS_VOID_(decl,attr,cc,name,param,asmname,args)                                          decl attr void (cc __DEPNAME_EXCEPT_UFS(name,asmname)) __P(param);
#define __VREDIRECT_EXCEPT_UFS(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)                     decl attr Treturn (cc __VDEPNAME_UFS(name,name,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_UFS_(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)           decl attr Treturn (cc __VDEPNAME_UFS(name,asmnamef,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_UFS_XVOID(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)               decl attr Treturn (cc __VDEPNAME_UFS(name,name,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_UFS_XVOID_(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)     decl attr Treturn (cc __VDEPNAME_UFS(name,asmnamef,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_UFS_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                        decl attr void (cc __VDEPNAME_UFS(name,name,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_UFS_VOID_(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)              decl attr void (cc __VDEPNAME_UFS(name,asmnamef,vasmnamef)) __P(param);
#define __XREDIRECT_EXCEPT_UFS(decl,attr,Treturn,cc,name,param,code)                                               decl attr Treturn (cc __DEPNAME_EXCEPT_UFS(name,name)) __P(param);
#define __XREDIRECT_EXCEPT_UFS_(decl,attr,Treturn,cc,name,param,asmname,code)                                      decl attr Treturn (cc __DEPNAME_EXCEPT_UFS(name,asmname)) __P(param);
#define __XREDIRECT_EXCEPT_UFS_XVOID(decl,attr,Treturn,cc,name,param,code)                                         decl attr Treturn (cc __DEPNAME_EXCEPT_UFS(name,name)) __P(param);
#define __XREDIRECT_EXCEPT_UFS_XVOID_(decl,attr,Treturn,cc,name,param,asmname,code)                                decl attr Treturn (cc __DEPNAME_EXCEPT_UFS(name,asmname)) __P(param);
#define __XREDIRECT_EXCEPT_UFS_VOID(decl,attr,cc,name,param,code)                                                  decl attr void (cc __DEPNAME_EXCEPT_UFS(name,name)) __P(param);
#define __XREDIRECT_EXCEPT_UFS_VOID_(decl,attr,cc,name,param,asmname,code)                                         decl attr void (cc __DEPNAME_EXCEPT_UFS(name,asmname)) __P(param);
#define __REDIRECT_EXCEPT_UFSW16(decl,attr,Treturn,cc,name,param,asmname,args)                                     decl attr Treturn (cc __DEPNAME_EXCEPT_UFSW16(name,asmname)) __P(param);
#define __REDIRECT_EXCEPT_UFSW32(decl,attr,Treturn,cc,name,param,asmname,args)                                     decl attr Treturn (cc __DEPNAME_EXCEPT_UFSW32(name,asmname)) __P(param);
#define __REDIRECT_EXCEPT_UFSW16_XVOID(decl,attr,Treturn,cc,name,param,asmname,args)                               decl attr Treturn (cc __DEPNAME_EXCEPT_UFSW16(name,asmname)) __P(param);
#define __REDIRECT_EXCEPT_UFSW32_XVOID(decl,attr,Treturn,cc,name,param,asmname,args)                               decl attr Treturn (cc __DEPNAME_EXCEPT_UFSW32(name,asmname)) __P(param);
#define __REDIRECT_EXCEPT_UFSW16_VOID(decl,attr,cc,name,param,asmname,args)                                        decl attr void (cc __DEPNAME_EXCEPT_UFSW16(name,asmname)) __P(param);
#define __REDIRECT_EXCEPT_UFSW32_VOID(decl,attr,cc,name,param,asmname,args)                                        decl attr void (cc __DEPNAME_EXCEPT_UFSW32(name,asmname)) __P(param);
#define __VREDIRECT_EXCEPT_UFSW16(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)         decl attr Treturn (cc __VDEPNAME_UFSW16(name,asmnamef,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_UFSW32(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)         decl attr Treturn (cc __VDEPNAME_UFSW32(name,asmnamef,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_UFSW16_XVOID(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)   decl attr Treturn (cc __VDEPNAME_UFSW16(name,asmnamef,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_UFSW32_XVOID(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)   decl attr Treturn (cc __VDEPNAME_UFSW32(name,asmnamef,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_UFSW16_VOID(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)            decl attr void (cc __VDEPNAME_UFSW16(name,asmnamef,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_UFSW32_VOID(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)            decl attr void (cc __VDEPNAME_UFSW32(name,asmnamef,vasmnamef)) __P(param);
#define __XREDIRECT_EXCEPT_UFSW16(decl,attr,Treturn,cc,name,param,asmname,code)                                    decl attr Treturn (cc __DEPNAME_EXCEPT_UFSW16(name,asmname)) __P(param);
#define __XREDIRECT_EXCEPT_UFSW32(decl,attr,Treturn,cc,name,param,asmname,code)                                    decl attr Treturn (cc __DEPNAME_EXCEPT_UFSW32(name,asmname)) __P(param);
#define __XREDIRECT_EXCEPT_UFSW16_XVOID(decl,attr,Treturn,cc,name,param,asmname,code)                              decl attr Treturn (cc __DEPNAME_EXCEPT_UFSW16(name,asmname)) __P(param);
#define __XREDIRECT_EXCEPT_UFSW32_XVOID(decl,attr,Treturn,cc,name,param,asmname,code)                              decl attr Treturn (cc __DEPNAME_EXCEPT_UFSW32(name,asmname)) __P(param);
#define __XREDIRECT_EXCEPT_UFSW16_VOID(decl,attr,cc,name,param,asmname,code)                                       decl attr void (cc __DEPNAME_EXCEPT_UFSW16(name,asmname)) __P(param);
#define __XREDIRECT_EXCEPT_UFSW32_VOID(decl,attr,cc,name,param,asmname,code)                                       decl attr void (cc __DEPNAME_EXCEPT_UFSW32(name,asmname)) __P(param);
#define __REDIRECT_EXCEPT_UFSDPW16(decl,attr,Treturn,cc,name,param,asmname,args)                                   decl attr Treturn (cc __DEPNAME_EXCEPT_UFSDPW16(name,asmname)) __P(param);
#define __REDIRECT_EXCEPT_UFSDPW16_XVOID(decl,attr,Treturn,cc,name,param,asmname,args)                             decl attr Treturn (cc __DEPNAME_EXCEPT_UFSDPW16(name,asmname)) __P(param);
#define __REDIRECT_EXCEPT_UFSDPW16_VOID(decl,attr,cc,name,param,asmname,args)                                      decl attr void (cc __DEPNAME_EXCEPT_UFSDPW16(name,asmname)) __P(param);
#define __VREDIRECT_EXCEPT_UFSDPW16(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)       decl attr Treturn (cc __VDEPNAME_UFSDPW16(name,asmnamef,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_UFSDPW16_XVOID(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start) decl attr Treturn (cc __VDEPNAME_UFSDPW16(name,asmnamef,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_UFSDPW16_VOID(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)          decl attr void (cc __VDEPNAME_UFSDPW16(name,asmnamef,vasmnamef)) __P(param);
#define __XREDIRECT_EXCEPT_UFSDPW16(decl,attr,Treturn,cc,name,param,asmname,code)                                  decl attr Treturn (cc __DEPNAME_EXCEPT_UFSDPW16(name,asmname)) __P(param);
#define __XREDIRECT_EXCEPT_UFSDPW16_XVOID(decl,attr,Treturn,cc,name,param,asmname,code)                            decl attr Treturn (cc __DEPNAME_EXCEPT_UFSDPW16(name,asmname)) __P(param);
#define __XREDIRECT_EXCEPT_UFSDPW16_VOID(decl,attr,cc,name,param,asmname,code)                                     decl attr void (cc __DEPNAME_EXCEPT_UFSDPW16(name,asmname)) __P(param);

#define __DEPNAME_DPA(name)                       name ##$__$## name ##$__$_## name
#define __DEPNAME_DPB(name)                    _##name ##$__$## name ##$__$_## name
#define __VDEPNAME_DPA(name,vasmnamef)            name ##$__$## name ##$__$_## name ##$__$## vasmnamef ##$__$_## vasmnamef
#define __VDEPNAME_DPB(name,vasmnamef)         _##name ##$__$## name ##$__$_## name ##$__$## vasmnamef ##$__$_## vasmnamef
#define __DEPNAME_EXCEPT_DPA(name)                name ##$__$## name ##$__$_## name ##$__$X## name
#define __DEPNAME_EXCEPT_DPB(name)             _##name ##$__$## name ##$__$_## name ##$__$X## name
#define __VDEPNAME_EXCEPT_DPA(name,vasmnamef)     name ##$__$## name ##$__$_## name ##$__$## vasmnamef ##$__$_## vasmnamef ##$__$X## name ##$__$X## vasmnamef
#define __VDEPNAME_EXCEPT_DPB(name,vasmnamef)  _##name ##$__$## name ##$__$_## name ##$__$## vasmnamef ##$__$_## vasmnamef ##$__$X## name ##$__$X## vasmnamef

#define __SYMNAME_DOSPREFIX(x)        x
#define __SYMNAME_DOSPREFIX_IS_SAME   1

#define __REDIRECT_DPA(decl,attr,Treturn,cc,name,param,args)                                                 decl attr Treturn (cc __DEPNAME_DPA(name)) __P(param);
#define __REDIRECT_DPB(decl,attr,Treturn,cc,name,param,args)                                                 decl attr Treturn (cc __DEPNAME_DPB(name)) __P(param);
#define __REDIRECT_DPA_VOID(decl,attr,cc,name,param,args)                                                    decl attr void (cc __DEPNAME_DPA(name)) __P(param);
#define __REDIRECT_DPB_VOID(decl,attr,cc,name,param,args)                                                    decl attr void (cc __DEPNAME_DPB(name)) __P(param);
#define __REDIRECT_DPA_NOTHROW(decl,attr,Treturn,cc,name,param,args)                                         decl attr Treturn __NOTHROW((cc __DEPNAME_DPA(name)) __P(param));
#define __REDIRECT_DPB_NOTHROW(decl,attr,Treturn,cc,name,param,args)                                         decl attr Treturn __NOTHROW((cc __DEPNAME_DPB(name)) __P(param));
#define __REDIRECT_DPA_VOID_NOTHROW(decl,attr,cc,name,param,args)                                            decl attr void __NOTHROW((cc __DEPNAME_DPA(name)) __P(param));
#define __REDIRECT_DPB_VOID_NOTHROW(decl,attr,cc,name,param,args)                                            decl attr void __NOTHROW((cc __DEPNAME_DPB(name)) __P(param));
#define __VREDIRECT_DPA(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)                      decl attr Treturn (cc __VDEPNAME_DPA(name,vasmnamef)) __P(param);
#define __VREDIRECT_DPB(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)                      decl attr Treturn (cc __VDEPNAME_DPB(name,vasmnamef)) __P(param);
#define __VREDIRECT_DPA_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                         decl attr void (cc __VDEPNAME_DPA(name,vasmnamef)) __P(param);
#define __VREDIRECT_DPB_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                         decl attr void (cc __VDEPNAME_DPB(name,vasmnamef)) __P(param);
#define __VREDIRECT_DPA_NOTHROW(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)              decl attr Treturn __NOTHROW((cc __VDEPNAME_DPA(name,vasmnamef)) __P(param));
#define __VREDIRECT_DPB_NOTHROW(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)              decl attr Treturn __NOTHROW((cc __VDEPNAME_DPB(name,vasmnamef)) __P(param));
#define __VREDIRECT_DPA_VOID_NOTHROW(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                 decl attr void __NOTHROW((cc __VDEPNAME_DPA(name,vasmnamef)) __P(param));
#define __VREDIRECT_DPB_VOID_NOTHROW(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                 decl attr void __NOTHROW((cc __VDEPNAME_DPB(name,vasmnamef)) __P(param));
#define __XREDIRECT_DPA(decl,attr,Treturn,cc,name,param,code)                                                decl attr Treturn (cc __DEPNAME_DPA(name)) __P(param);
#define __XREDIRECT_DPB(decl,attr,Treturn,cc,name,param,code)                                                decl attr Treturn (cc __DEPNAME_DPB(name)) __P(param);
#define __XREDIRECT_DPA_VOID(decl,attr,cc,name,param,code)                                                   decl attr void (cc __DEPNAME_DPA(name)) __P(param);
#define __XREDIRECT_DPB_VOID(decl,attr,cc,name,param,code)                                                   decl attr void (cc __DEPNAME_DPB(name)) __P(param);
#define __XREDIRECT_DPA_NOTHROW(decl,attr,Treturn,cc,name,param,code)                                        decl attr Treturn __NOTHROW((cc __DEPNAME_DPA(name)) __P(param));
#define __XREDIRECT_DPB_NOTHROW(decl,attr,Treturn,cc,name,param,code)                                        decl attr Treturn __NOTHROW((cc __DEPNAME_DPB(name)) __P(param));
#define __XREDIRECT_DPA_VOID_NOTHROW(decl,attr,cc,name,param,code)                                           decl attr void __NOTHROW((cc __DEPNAME_DPA(name)) __P(param));
#define __XREDIRECT_DPB_VOID_NOTHROW(decl,attr,cc,name,param,code)                                           decl attr void __NOTHROW((cc __DEPNAME_DPB(name)) __P(param));
#define __REDIRECT_EXCEPT_DPA(decl,attr,Treturn,cc,name,param,args)                                          decl attr Treturn (cc __DEPNAME_EXCEPT_DPA(name)) __P(param);
#define __REDIRECT_EXCEPT_DPB(decl,attr,Treturn,cc,name,param,args)                                          decl attr Treturn (cc __DEPNAME_EXCEPT_DPB(name)) __P(param);
#define __REDIRECT_EXCEPT_DPA_VOID(decl,attr,cc,name,param,args)                                             decl attr void (cc __DEPNAME_EXCEPT_DPA(name)) __P(param);
#define __REDIRECT_EXCEPT_DPB_VOID(decl,attr,cc,name,param,args)                                             decl attr void (cc __DEPNAME_EXCEPT_DPB(name)) __P(param);
#define __REDIRECT_EXCEPT_DPA_XVOID(decl,attr,Treturn,cc,name,param,args)                                    decl attr Treturn (cc __DEPNAME_EXCEPT_DPA(name)) __P(param);
#define __REDIRECT_EXCEPT_DPB_XVOID(decl,attr,Treturn,cc,name,param,args)                                    decl attr Treturn (cc __DEPNAME_EXCEPT_DPB(name)) __P(param);
#define __VREDIRECT_EXCEPT_DPA(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)               decl attr Treturn (cc __VDEPNAME_DPA(name,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_DPB(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)               decl attr Treturn (cc __VDEPNAME_DPB(name,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_DPA_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                  decl attr void (cc __VDEPNAME_DPA(name,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_DPB_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                  decl attr void (cc __VDEPNAME_DPB(name,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_DPA_XVOID(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)         decl attr Treturn (cc __VDEPNAME_DPA(name,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_DPB_XVOID(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)         decl attr Treturn (cc __VDEPNAME_DPB(name,vasmnamef)) __P(param);
#define __XREDIRECT_EXCEPT_DPA(decl,attr,Treturn,cc,name,param,code)                                         decl attr Treturn (cc __DEPNAME_EXCEPT_DPA(name)) __P(param);
#define __XREDIRECT_EXCEPT_DPB(decl,attr,Treturn,cc,name,param,code)                                         decl attr Treturn (cc __DEPNAME_EXCEPT_DPB(name)) __P(param);
#define __XREDIRECT_EXCEPT_DPA_VOID(decl,attr,cc,name,param,code)                                            decl attr void (cc __DEPNAME_EXCEPT_DPA(name)) __P(param);
#define __XREDIRECT_EXCEPT_DPB_VOID(decl,attr,cc,name,param,code)                                            decl attr void (cc __DEPNAME_EXCEPT_DPB(name)) __P(param);
#define __XREDIRECT_EXCEPT_DPA_XVOID(decl,attr,Treturn,cc,name,param,code)                                   decl attr Treturn (cc __DEPNAME_EXCEPT_DPA(name)) __P(param);
#define __XREDIRECT_EXCEPT_DPB_XVOID(decl,attr,Treturn,cc,name,param,code)                                   decl attr Treturn (cc __DEPNAME_EXCEPT_DPB(name)) __P(param);



#define __DEPNAME_UFSDPA(name)                       name ##$__$## name ##$__$DOS$_## name
#define __DEPNAME_UFSDPB(name)                    _##name ##$__$## name ##$__$DOS$_## name
#define __VDEPNAME_UFSDPA(name,vasmnamef)            name ##$__$## name ##$__$DOS$_## name ##$__$## vasmnamef ##$__$DOS$_## vasmnamef
#define __VDEPNAME_UFSDPB(name,vasmnamef)         _##name ##$__$## name ##$__$DOS$_## name ##$__$## vasmnamef ##$__$DOS$_## vasmnamef
#define __DEPNAME_EXCEPT_UFSDPA(name)                name ##$__$## name ##$__$DOS$_## name ##$__$X## name
#define __DEPNAME_EXCEPT_UFSDPB(name)             _##name ##$__$## name ##$__$DOS$_## name ##$__$X## name
#define __VDEPNAME_EXCEPT_UFSDPA(name,vasmnamef)     name ##$__$## name ##$__$DOS$_## name ##$__$## vasmnamef ##$__$DOS$_## vasmnamef ##$__$X## name ##$__$X## vasmnamef
#define __VDEPNAME_EXCEPT_UFSDPB(name,vasmnamef)  _##name ##$__$## name ##$__$DOS$_## name ##$__$## vasmnamef ##$__$DOS$_## vasmnamef ##$__$X## name ##$__$X## vasmnamef

#define __REDIRECT_UFSDPA(decl,attr,Treturn,cc,name,param,args)                                              decl attr Treturn (cc __DEPNAME_UFSDPA(name)) __P(param);
#define __REDIRECT_UFSDPB(decl,attr,Treturn,cc,name,param,args)                                              decl attr Treturn (cc __DEPNAME_UFSDPB(name)) __P(param);
#define __REDIRECT_UFSDPA_VOID(decl,attr,cc,name,param,args)                                                 decl attr void (cc __DEPNAME_UFSDPA(name)) __P(param);
#define __REDIRECT_UFSDPB_VOID(decl,attr,cc,name,param,args)                                                 decl attr void (cc __DEPNAME_UFSDPB(name)) __P(param);
#define __REDIRECT_UFSDPA_NOTHROW(decl,attr,Treturn,cc,name,param,args)                                      decl attr Treturn __NOTHROW((cc __DEPNAME_UFSDPA(name)) __P(param));
#define __REDIRECT_UFSDPB_NOTHROW(decl,attr,Treturn,cc,name,param,args)                                      decl attr Treturn __NOTHROW((cc __DEPNAME_UFSDPB(name)) __P(param));
#define __REDIRECT_UFSDPA_VOID_NOTHROW(decl,attr,cc,name,param,args)                                         decl attr void __NOTHROW((cc __DEPNAME_UFSDPA(name)) __P(param));
#define __REDIRECT_UFSDPB_VOID_NOTHROW(decl,attr,cc,name,param,args)                                         decl attr void __NOTHROW((cc __DEPNAME_UFSDPB(name)) __P(param));
#define __VREDIRECT_UFSDPA(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)                   decl attr Treturn (cc __VDEPNAME_UFSDPA(name,vasmnamef)) __P(param);
#define __VREDIRECT_UFSDPB(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)                   decl attr Treturn (cc __VDEPNAME_UFSDPB(name,vasmnamef)) __P(param);
#define __VREDIRECT_UFSDPA_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                      decl attr void (cc __VDEPNAME_UFSDPA(name,vasmnamef)) __P(param);
#define __VREDIRECT_UFSDPB_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                      decl attr void (cc __VDEPNAME_UFSDPB(name,vasmnamef)) __P(param);
#define __VREDIRECT_UFSDPA_NOTHROW(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)           decl attr Treturn __NOTHROW((cc __VDEPNAME_UFSDPA(name,vasmnamef)) __P(param));
#define __VREDIRECT_UFSDPB_NOTHROW(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)           decl attr Treturn __NOTHROW((cc __VDEPNAME_UFSDPB(name,vasmnamef)) __P(param));
#define __VREDIRECT_UFSDPA_VOID_NOTHROW(decl,attr,cc,name,param,vasmnamef,args,before_va_start)              decl attr void __NOTHROW((cc __VDEPNAME_UFSDPA(name,vasmnamef)) __P(param));
#define __VREDIRECT_UFSDPB_VOID_NOTHROW(decl,attr,cc,name,param,vasmnamef,args,before_va_start)              decl attr void __NOTHROW((cc __VDEPNAME_UFSDPB(name,vasmnamef)) __P(param));
#define __XREDIRECT_UFSDPA(decl,attr,Treturn,cc,name,param,code)                                             decl attr Treturn (cc __DEPNAME_UFSDPA(name)) __P(param);
#define __XREDIRECT_UFSDPB(decl,attr,Treturn,cc,name,param,code)                                             decl attr Treturn (cc __DEPNAME_UFSDPB(name)) __P(param);
#define __XREDIRECT_UFSDPA_VOID(decl,attr,cc,name,param,code)                                                decl attr void (cc __DEPNAME_UFSDPA(name)) __P(param);
#define __XREDIRECT_UFSDPB_VOID(decl,attr,cc,name,param,code)                                                decl attr void (cc __DEPNAME_UFSDPB(name)) __P(param);
#define __XREDIRECT_UFSDPA_NOTHROW(decl,attr,Treturn,cc,name,param,code)                                     decl attr Treturn __NOTHROW((cc __DEPNAME_UFSDPA(name)) __P(param));
#define __XREDIRECT_UFSDPB_NOTHROW(decl,attr,Treturn,cc,name,param,code)                                     decl attr Treturn __NOTHROW((cc __DEPNAME_UFSDPB(name)) __P(param));
#define __XREDIRECT_UFSDPA_VOID_NOTHROW(decl,attr,cc,name,param,code)                                        decl attr void __NOTHROW((cc __DEPNAME_UFSDPA(name)) __P(param));
#define __XREDIRECT_UFSDPB_VOID_NOTHROW(decl,attr,cc,name,param,code)                                        decl attr void __NOTHROW((cc __DEPNAME_UFSDPB(name)) __P(param));
#define __REDIRECT_EXCEPT_UFSDPA(decl,attr,Treturn,cc,name,param,args)                                       decl attr Treturn (cc __DEPNAME_EXCEPT_UFSDPA(name)) __P(param);
#define __REDIRECT_EXCEPT_UFSDPB(decl,attr,Treturn,cc,name,param,args)                                       decl attr Treturn (cc __DEPNAME_EXCEPT_UFSDPB(name)) __P(param);
#define __REDIRECT_EXCEPT_UFSDPA_VOID(decl,attr,cc,name,param,args)                                          decl attr void (cc __DEPNAME_EXCEPT_UFSDPA(name)) __P(param);
#define __REDIRECT_EXCEPT_UFSDPB_VOID(decl,attr,cc,name,param,args)                                          decl attr void (cc __DEPNAME_EXCEPT_UFSDPB(name)) __P(param);
#define __REDIRECT_EXCEPT_UFSDPA_XVOID(decl,attr,Treturn,cc,name,param,args)                                 decl attr Treturn (cc __DEPNAME_EXCEPT_UFSDPA(name)) __P(param);
#define __REDIRECT_EXCEPT_UFSDPB_XVOID(decl,attr,Treturn,cc,name,param,args)                                 decl attr Treturn (cc __DEPNAME_EXCEPT_UFSDPB(name)) __P(param);
#define __VREDIRECT_EXCEPT_UFSDPA(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)            decl attr Treturn (cc __VDEPNAME_UFSDPA(name,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_UFSDPB(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)            decl attr Treturn (cc __VDEPNAME_UFSDPB(name,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_UFSDPA_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)               decl attr void (cc __VDEPNAME_UFSDPA(name,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_UFSDPB_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)               decl attr void (cc __VDEPNAME_UFSDPB(name,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_UFSDPA_XVOID(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)      decl attr Treturn (cc __VDEPNAME_UFSDPA(name,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_UFSDPB_XVOID(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)      decl attr Treturn (cc __VDEPNAME_UFSDPB(name,vasmnamef)) __P(param);
#define __XREDIRECT_EXCEPT_UFSDPA(decl,attr,Treturn,cc,name,param,code)                                      decl attr Treturn (cc __DEPNAME_EXCEPT_UFSDPA(name)) __P(param);
#define __XREDIRECT_EXCEPT_UFSDPB(decl,attr,Treturn,cc,name,param,code)                                      decl attr Treturn (cc __DEPNAME_EXCEPT_UFSDPB(name)) __P(param);
#define __XREDIRECT_EXCEPT_UFSDPA_VOID(decl,attr,cc,name,param,code)                                         decl attr void (cc __DEPNAME_EXCEPT_UFSDPA(name)) __P(param);
#define __XREDIRECT_EXCEPT_UFSDPB_VOID(decl,attr,cc,name,param,code)                                         decl attr void (cc __DEPNAME_EXCEPT_UFSDPB(name)) __P(param);
#define __XREDIRECT_EXCEPT_UFSDPA_XVOID(decl,attr,Treturn,cc,name,param,code)                                decl attr Treturn (cc __DEPNAME_EXCEPT_UFSDPA(name)) __P(param);
#define __XREDIRECT_EXCEPT_UFSDPB_XVOID(decl,attr,Treturn,cc,name,param,code)                                decl attr Treturn (cc __DEPNAME_EXCEPT_UFSDPB(name)) __P(param);



#define __DEPNAME_FS64(name)                       name ##$__$## name ##$__$## name ##64
#define __DEPNAME_UFS64(name)                      name ##$__$## name ##$__$## name ##64$__$DOS$## name ##$__$DOS$## name ##64
#define __VDEPNAME_FS64(name,vasmnamef)            name ##$__$## name ##$__$## name ##64$__$## vasmnamef ##$__$## vasmnamef ##64
#define __VDEPNAME_UFS64(name,vasmnamef)           name ##$__$## name ##$__$## name ##64$__$DOS$## name ##$__$DOS$## name ##64$__$## vasmnamef ##$__$## vasmnamef ##64$__$DOS$## vasmnamef ##$__$DOS$## vasmnamef ##64
#define __DEPNAME_EXCEPT_FS64(name)                name ##$__$## name ##$__$## name ##64$__$X## name ##$__$X## name ##64
#define __DEPNAME_EXCEPT_UFS64(name)               name ##$__$## name ##$__$## name ##64$__$DOS$## name ##$__$DOS$## name ##64$__$X## name ##$__$X## name ##64
#define __VDEPNAME_EXCPET_FS64(name,vasmnamef)     name ##$__$## name ##$__$## name ##64$__$## vasmnamef ##$__$## vasmnamef ##64$__$X## name ##$__$X## name ##64$__$X## vasmnamef ##$__$X## vasmnamef ##64
#define __VDEPNAME_EXCPET_UFS64(name,vasmnamef)    name ##$__$## name ##$__$## name ##64$__$DOS$## name ##$__$DOS$## name ##64$__$## vasmnamef ##$__$## vasmnamef ##64$__$DOS$## vasmnamef ##$__$DOS$## vasmnamef ##64$__$X## name ##$__$X## name ##64$__$X## vasmnamef ##$__$X## vasmnamef ##64
#define __REDIRECT_FS64(decl,attr,Treturn,cc,name,param,args)                                          decl attr Treturn (cc __DEPNAME_FS64(name)) __P(param);
#define __REDIRECT_FS64_VOID(decl,attr,cc,name,param,args)                                             decl attr Treturn (cc __DEPNAME_FS64(name)) __P(param);
#define __REDIRECT_FS64_NOTHROW(decl,attr,Treturn,cc,name,param,args)                                  decl attr Treturn __NOTHROW((cc __DEPNAME_FS64(name)) __P(param));
#define __REDIRECT_FS64_VOID_NOTHROW(decl,attr,cc,name,param,args)                                     decl attr Treturn __NOTHROW((cc __DEPNAME_FS64(name)) __P(param));
#define __VREDIRECT_FS64(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)               decl attr void (cc __VDEPNAME_FS64(name,vasmnamef)) __P(param);
#define __VREDIRECT_FS64_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                  decl attr void (cc __VDEPNAME_FS64(name,vasmnamef)) __P(param);
#define __VREDIRECT_FS64_NOTHROW(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)       decl attr void __NOTHROW((cc __VDEPNAME_FS64(name,vasmnamef)) __P(param));
#define __VREDIRECT_FS64_VOID_NOTHROW(decl,attr,cc,name,param,vasmnamef,args,before_va_start)          decl attr void __NOTHROW((cc __VDEPNAME_FS64(name,vasmnamef)) __P(param));
#define __XREDIRECT_FS64(decl,attr,Treturn,cc,name,param,code)                                         decl attr Treturn (cc __DEPNAME_FS64(name)) __P(param);
#define __XREDIRECT_FS64_VOID(decl,attr,cc,name,param,code)                                            decl attr Treturn (cc __DEPNAME_FS64(name)) __P(param);
#define __XREDIRECT_FS64_NOTHROW(decl,attr,Treturn,cc,name,param,code)                                 decl attr Treturn __NOTHROW((cc __DEPNAME_FS64(name)) __P(param));
#define __XREDIRECT_FS64_VOID_NOTHROW(decl,attr,cc,name,param,code)                                    decl attr Treturn __NOTHROW((cc __DEPNAME_FS64(name)) __P(param));
#define __REDIRECT_UFS64(decl,attr,Treturn,cc,name,param,args)                                         decl attr Treturn (cc __DEPNAME_UFS64(name)) __P(param);
#define __REDIRECT_UFS64_VOID(decl,attr,cc,name,param,args)                                            decl attr Treturn (cc __DEPNAME_UFS64(name)) __P(param);
#define __REDIRECT_UFS64_NOTHROW(decl,attr,Treturn,cc,name,param,args)                                 decl attr Treturn __NOTHROW((cc __DEPNAME_UFS64(name)) __P(param));
#define __REDIRECT_UFS64_VOID_NOTHROW(decl,attr,cc,name,param,args)                                    decl attr Treturn __NOTHROW((cc __DEPNAME_UFS64(name)) __P(param));
#define __VREDIRECT_UFS64(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)              decl attr void (cc __VDEPNAME_UFS64(name,vasmnamef)) __P(param);
#define __VREDIRECT_UFS64_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                 decl attr void (cc __VDEPNAME_UFS64(name,vasmnamef)) __P(param);
#define __VREDIRECT_UFS64_NOTHROW(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)      decl attr void __NOTHROW((cc __VDEPNAME_UFS64(name,vasmnamef)) __P(param));
#define __VREDIRECT_UFS64_VOID_NOTHROW(decl,attr,cc,name,param,vasmnamef,args,before_va_start)         decl attr void __NOTHROW((cc __VDEPNAME_UFS64(name,vasmnamef)) __P(param));
#define __XREDIRECT_UFS64(decl,attr,Treturn,cc,name,param,code)                                        decl attr Treturn (cc __DEPNAME_UFS64(name)) __P(param);
#define __XREDIRECT_UFS64_VOID(decl,attr,cc,name,param,code)                                           decl attr Treturn (cc __DEPNAME_UFS64(name)) __P(param);
#define __XREDIRECT_UFS64_NOTHROW(decl,attr,Treturn,cc,name,param,code)                                decl attr Treturn __NOTHROW((cc __DEPNAME_UFS64(name)) __P(param));
#define __XREDIRECT_UFS64_VOID_NOTHROW(decl,attr,cc,name,param,code)                                   decl attr Treturn __NOTHROW((cc __DEPNAME_UFS64(name)) __P(param));
#define __REDIRECT_EXCEPT_FS64(decl,attr,Treturn,cc,name,param,args)                                   decl attr Treturn (cc __DEPNAME_EXCEPT_FS64(name)) __P(param);
#define __REDIRECT_EXCEPT_FS64_VOID(decl,attr,cc,name,param,args)                                      decl attr Treturn (cc __DEPNAME_EXCEPT_FS64(name)) __P(param);
#define __REDIRECT_EXCEPT_FS64_XVOID(decl,attr,Treturn,cc,name,param,args)                             decl attr Treturn (cc __DEPNAME_EXCEPT_FS64(name)) __P(param);
#define __VREDIRECT_EXCEPT_FS64(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)        decl attr void (cc __VDEPNAME_FS64(name,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_FS64_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)           decl attr void (cc __VDEPNAME_FS64(name,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_FS64_XVOID(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)  decl attr void (cc __VDEPNAME_FS64(name,vasmnamef)) __P(param));
#define __XREDIRECT_EXCEPT_FS64(decl,attr,Treturn,cc,name,param,code)                                  decl attr Treturn (cc __DEPNAME_EXCEPT_FS64(name)) __P(param);
#define __XREDIRECT_EXCEPT_FS64_VOID(decl,attr,cc,name,param,code)                                     decl attr Treturn (cc __DEPNAME_EXCEPT_FS64(name)) __P(param);
#define __XREDIRECT_EXCEPT_FS64_XVOID(decl,attr,Treturn,cc,name,param,code)                            decl attr Treturn (cc __DEPNAME_EXCEPT_FS64(name)) __P(param);
#define __REDIRECT_EXCEPT_UFS64(decl,attr,Treturn,cc,name,param,args)                                  decl attr Treturn (cc __DEPNAME_EXCEPT_UFS64(name)) __P(param);
#define __REDIRECT_EXCEPT_UFS64_VOID(decl,attr,cc,name,param,args)                                     decl attr Treturn (cc __DEPNAME_EXCEPT_UFS64(name)) __P(param);
#define __REDIRECT_EXCEPT_UFS64_XVOID(decl,attr,Treturn,cc,name,param,args)                            decl attr Treturn (cc __DEPNAME_EXCEPT_UFS64(name)) __P(param);
#define __VREDIRECT_EXCEPT_UFS64(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)       decl attr void (cc __VDEPNAME_UFS64(name,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_UFS64_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)          decl attr void (cc __VDEPNAME_UFS64(name,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_UFS64_XVOID(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start) decl attr void (cc __VDEPNAME_UFS64(name,vasmnamef)) __P(param);
#define __XREDIRECT_EXCEPT_UFS64(decl,attr,Treturn,cc,name,param,code)                                 decl attr Treturn (cc __DEPNAME_EXCEPT_UFS64(name)) __P(param);
#define __XREDIRECT_EXCEPT_UFS64_VOID(decl,attr,cc,name,param,code)                                    decl attr Treturn (cc __DEPNAME_EXCEPT_UFS64(name)) __P(param);
#define __XREDIRECT_EXCEPT_UFS64_XVOID(decl,attr,Treturn,cc,name,param,code)                           decl attr Treturn (cc __DEPNAME_EXCEPT_UFS64(name)) __P(param);


#define __DEPNAME_TM64(name)                       name ##$__$## name ##$__$## name ##64
#define __DEPNAME_UFSTM64(name)                    name ##$__$## name ##$__$## name ##64$__$DOS$## name ##$__$DOS$## name ##64
#define __VDEPNAME_TM64(name,vasmnamef)            name ##$__$## name ##$__$## name ##64$__$## vasmnamef ##$__$## vasmnamef ##64
#define __VDEPNAME_UFSTM64(name,vasmnamef)         name ##$__$## name ##$__$## name ##64$__$DOS$## name ##$__$DOS$## name ##64$__$## vasmnamef ##$__$## vasmnamef ##64$__$DOS$## vasmnamef ##$__$DOS$## vasmnamef ##64
#define __DEPNAME_EXCEPT_TM64(name)                name ##$__$## name ##$__$## name ##64$__$X## name ##$__$X## name ##64
#define __DEPNAME_EXCEPT_UFSTM64(name)             name ##$__$## name ##$__$## name ##64$__$DOS$## name ##$__$DOS$## name ##64$__$X## name ##$__$X## name ##64
#define __VDEPNAME_EXCPET_TM64(name,vasmnamef)     name ##$__$## name ##$__$## name ##64$__$## vasmnamef ##$__$## vasmnamef ##64$__$X## name ##$__$X## name ##64$__$X## vasmnamef ##$__$X## vasmnamef ##64
#define __VDEPNAME_EXCPET_UFSTM64(name,vasmnamef)  name ##$__$## name ##$__$## name ##64$__$DOS$## name ##$__$DOS$## name ##64$__$## vasmnamef ##$__$## vasmnamef ##64$__$DOS$## vasmnamef ##$__$DOS$## vasmnamef ##64$__$X## name ##$__$X## name ##64$__$X## vasmnamef ##$__$X## vasmnamef ##64
#define __REDIRECT_TM64(decl,attr,Treturn,cc,name,param,args)                                            decl attr Treturn (cc __DEPNAME_TM64(name)) __P(param);
#define __REDIRECT_TM64_VOID(decl,attr,cc,name,param,args)                                               decl attr Treturn (cc __DEPNAME_TM64(name)) __P(param);
#define __REDIRECT_TM64_NOTHROW(decl,attr,Treturn,cc,name,param,args)                                    decl attr Treturn __NOTHROW((cc __DEPNAME_TM64(name)) __P(param));
#define __REDIRECT_TM64_VOID_NOTHROW(decl,attr,cc,name,param,args)                                       decl attr Treturn __NOTHROW((cc __DEPNAME_TM64(name)) __P(param));
#define __VREDIRECT_TM64(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)                 decl attr void (cc __VDEPNAME_TM64(name,vasmnamef)) __P(param);
#define __VREDIRECT_TM64_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                    decl attr void (cc __VDEPNAME_TM64(name,vasmnamef)) __P(param);
#define __VREDIRECT_TM64_NOTHROW(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)         decl attr void __NOTHROW((cc __VDEPNAME_TM64(name,vasmnamef)) __P(param));
#define __VREDIRECT_TM64_VOID_NOTHROW(decl,attr,cc,name,param,vasmnamef,args,before_va_start)            decl attr void __NOTHROW((cc __VDEPNAME_TM64(name,vasmnamef)) __P(param));
#define __XREDIRECT_TM64(decl,attr,Treturn,cc,name,param,code)                                           decl attr Treturn (cc __DEPNAME_TM64(name)) __P(param);
#define __XREDIRECT_TM64_VOID(decl,attr,cc,name,param,code)                                              decl attr Treturn (cc __DEPNAME_TM64(name)) __P(param);
#define __XREDIRECT_TM64_NOTHROW(decl,attr,Treturn,cc,name,param,code)                                   decl attr Treturn __NOTHROW((cc __DEPNAME_TM64(name)) __P(param));
#define __XREDIRECT_TM64_VOID_NOTHROW(decl,attr,cc,name,param,code)                                      decl attr Treturn __NOTHROW((cc __DEPNAME_TM64(name)) __P(param));
#define __REDIRECT_UFSTM64(decl,attr,Treturn,cc,name,param,args)                                         decl attr Treturn (cc __DEPNAME_UFSTM64(name)) __P(param);
#define __REDIRECT_UFSTM64_VOID(decl,attr,cc,name,param,args)                                            decl attr Treturn (cc __DEPNAME_UFSTM64(name)) __P(param);
#define __REDIRECT_UFSTM64_NOTHROW(decl,attr,Treturn,cc,name,param,args)                                 decl attr Treturn __NOTHROW((cc __DEPNAME_UFSTM64(name)) __P(param));
#define __REDIRECT_UFSTM64_VOID_NOTHROW(decl,attr,cc,name,param,args)                                    decl attr Treturn __NOTHROW((cc __DEPNAME_UFSTM64(name)) __P(param));
#define __VREDIRECT_UFSTM64(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)              decl attr void (cc __VDEPNAME_UFSTM64(name,vasmnamef)) __P(param);
#define __VREDIRECT_UFSTM64_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                 decl attr void (cc __VDEPNAME_UFSTM64(name,vasmnamef)) __P(param);
#define __VREDIRECT_UFSTM64_NOTHROW(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)      decl attr void __NOTHROW((cc __VDEPNAME_UFSTM64(name,vasmnamef)) __P(param));
#define __VREDIRECT_UFSTM64_VOID_NOTHROW(decl,attr,cc,name,param,vasmnamef,args,before_va_start)         decl attr void __NOTHROW((cc __VDEPNAME_UFSTM64(name,vasmnamef)) __P(param));
#define __XREDIRECT_UFSTM64(decl,attr,Treturn,cc,name,param,code)                                        decl attr Treturn (cc __DEPNAME_UFSTM64(name)) __P(param);
#define __XREDIRECT_UFSTM64_VOID(decl,attr,cc,name,param,code)                                           decl attr Treturn (cc __DEPNAME_UFSTM64(name)) __P(param);
#define __XREDIRECT_UFSTM64_NOTHROW(decl,attr,Treturn,cc,name,param,code)                                decl attr Treturn __NOTHROW((cc __DEPNAME_UFSTM64(name)) __P(param));
#define __XREDIRECT_UFSTM64_VOID_NOTHROW(decl,attr,cc,name,param,code)                                   decl attr Treturn __NOTHROW((cc __DEPNAME_UFSTM64(name)) __P(param));
#define __REDIRECT_EXCEPT_TM64(decl,attr,Treturn,cc,name,param,args)                                     decl attr Treturn (cc __DEPNAME_EXCEPT_TM64(name)) __P(param);
#define __REDIRECT_EXCEPT_TM64_VOID(decl,attr,cc,name,param,args)                                        decl attr Treturn (cc __DEPNAME_EXCEPT_TM64(name)) __P(param);
#define __REDIRECT_EXCEPT_TM64_XVOID(decl,attr,Treturn,cc,name,param,args)                               decl attr Treturn (cc __DEPNAME_EXCEPT_TM64(name)) __P(param);
#define __VREDIRECT_EXCEPT_TM64(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)          decl attr void (cc __VDEPNAME_TM64(name,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_TM64_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)             decl attr void (cc __VDEPNAME_TM64(name,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_TM64_XVOID(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)    decl attr void (cc __VDEPNAME_TM64(name,vasmnamef)) __P(param));
#define __XREDIRECT_EXCEPT_TM64(decl,attr,Treturn,cc,name,param,code)                                    decl attr Treturn (cc __DEPNAME_EXCEPT_TM64(name)) __P(param);
#define __XREDIRECT_EXCEPT_TM64_VOID(decl,attr,cc,name,param,code)                                       decl attr Treturn (cc __DEPNAME_EXCEPT_TM64(name)) __P(param);
#define __XREDIRECT_EXCEPT_TM64_XVOID(decl,attr,Treturn,cc,name,param,code)                              decl attr Treturn (cc __DEPNAME_EXCEPT_TM64(name)) __P(param);
#define __REDIRECT_EXCEPT_UFSTM64(decl,attr,Treturn,cc,name,param,args)                                  decl attr Treturn (cc __DEPNAME_EXCEPT_UFSTM64(name)) __P(param);
#define __REDIRECT_EXCEPT_UFSTM64_VOID(decl,attr,cc,name,param,args)                                     decl attr Treturn (cc __DEPNAME_EXCEPT_UFSTM64(name)) __P(param);
#define __REDIRECT_EXCEPT_UFSTM64_XVOID(decl,attr,Treturn,cc,name,param,args)                            decl attr Treturn (cc __DEPNAME_EXCEPT_UFSTM64(name)) __P(param);
#define __VREDIRECT_EXCEPT_UFSTM64(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)       decl attr void (cc __VDEPNAME_UFSTM64(name,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_UFSTM64_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)          decl attr void (cc __VDEPNAME_UFSTM64(name,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_UFSTM64_XVOID(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start) decl attr void (cc __VDEPNAME_UFSTM64(name,vasmnamef)) __P(param);
#define __XREDIRECT_EXCEPT_UFSTM64(decl,attr,Treturn,cc,name,param,code)                                 decl attr Treturn (cc __DEPNAME_EXCEPT_UFSTM64(name)) __P(param);
#define __XREDIRECT_EXCEPT_UFSTM64_VOID(decl,attr,cc,name,param,code)                                    decl attr Treturn (cc __DEPNAME_EXCEPT_UFSTM64(name)) __P(param);
#define __XREDIRECT_EXCEPT_UFSTM64_XVOID(decl,attr,Treturn,cc,name,param,code)                           decl attr Treturn (cc __DEPNAME_EXCEPT_UFSTM64(name)) __P(param);


/* Exception redirection functions */
#define __DEPNAME_EXCEPT(name,asmname)                  name ##$__$## asmname ##$__$X## asmname
#define __VDEPNAME_EXCEPT(name,asmnamef,vasmnamef)      name ##$__$## asmnamef ##$__$X## asmnamef ##$__$## vasmnamef ##$__$X## vasmnamef
#define __DEPNAME_EXCEPT_W32(name,asmname)              name ##$__$## asmname ##$__$X## asmname
#define __VDEPNAME_EXCEPT_W32(name,asmnamef,vasmnamef)  name ##$__$## asmnamef ##$__$X## asmnamef ##$__$## vasmnamef ##$__$X## vasmnamef
#define __DEPNAME_EXCEPT_W16(name,asmname)              name ##$__$DOS$## asmname ##$__$DOS$X## asmname
#define __VDEPNAME_EXCEPT_W16(name,asmnamef,vasmnamef)  name ##$__$DOS$## asmnamef ##$__$DOS$X## asmnamef ##$__$DOS$## vasmnamef ##$__$DOS$X## vasmnamef
#define __IF_USE_EXCEPT(x)      /* nothing */
#define __IF_NUSE_EXCEPT(x)     x
#define __EXCEPT_SELECT(tt,ff)  ff

#define __REDIRECT_EXCEPT(decl,attr,Treturn,cc,name,param,args)                                                          decl attr Treturn (cc __DEPNAME_EXCEPT(name,name)) __P(param);
#define __REDIRECT_EXCEPT_(decl,attr,Treturn,cc,name,param,asmname,args)                                                 decl attr Treturn (cc __DEPNAME_EXCEPT(name,asmname)) __P(param);
#define __REDIRECT_EXCEPT_XVOID(decl,attr,Treturn,cc,name,param,args)                                                    decl attr Treturn (cc __DEPNAME_EXCEPT(name,name)) __P(param);
#define __REDIRECT_EXCEPT_XVOID_(decl,attr,Treturn,cc,name,param,asmname,args)                                           decl attr Treturn (cc __DEPNAME_EXCEPT(name,asmname)) __P(param);
#define __REDIRECT_EXCEPT_VOID(decl,attr,cc,name,param,args)                                                             decl attr void (cc __DEPNAME_EXCEPT(name,name)) __P(param);
#define __REDIRECT_EXCEPT_VOID_(decl,attr,cc,name,param,asmname,args)                                                    decl attr void (cc __DEPNAME_EXCEPT(name,asmname)) __P(param);
#define __VREDIRECT_EXCEPT(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)                               decl attr Treturn (cc __VDEPNAME_EXCEPT(name,name,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)                     decl attr Treturn (cc __VDEPNAME_EXCEPT(name,asmnamef,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_XVOID(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)                         decl attr Treturn (cc __VDEPNAME_EXCEPT(name,name,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_XVOID_(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)               decl attr Treturn (cc __VDEPNAME_EXCEPT(name,asmnamef,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                                  decl attr void (cc __VDEPNAME_EXCEPT(name,name,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_VOID_(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)                        decl attr void (cc __VDEPNAME_EXCEPT(name,asmnamef,vasmnamef)) __P(param);
#define __XREDIRECT_EXCEPT(decl,attr,Treturn,cc,name,param,code)                                                         decl attr Treturn (cc __DEPNAME_EXCEPT(name,name)) __P(param);
#define __XREDIRECT_EXCEPT_(decl,attr,Treturn,cc,name,param,asmname,code)                                                decl attr Treturn (cc __DEPNAME_EXCEPT(name,asmname)) __P(param);
#define __XREDIRECT_EXCEPT_XVOID(decl,attr,Treturn,cc,name,param,code)                                                   decl attr Treturn (cc __DEPNAME_EXCEPT(name,name)) __P(param);
#define __XREDIRECT_EXCEPT_XVOID_(decl,attr,Treturn,cc,name,param,asmname,code)                                          decl attr Treturn (cc __DEPNAME_EXCEPT(name,asmname)) __P(param);
#define __XREDIRECT_EXCEPT_VOID(decl,attr,cc,name,param,code)                                                            decl attr void (cc __DEPNAME_EXCEPT(name,name)) __P(param);
#define __XREDIRECT_EXCEPT_VOID_(decl,attr,cc,name,param,asmname,code)                                                   decl attr void (cc __DEPNAME_EXCEPT(name,asmname)) __P(param);
#define __REDIRECT_EXCEPT_TOW16(decl,attr,Treturn,cc,name,param,args)                                                    decl attr Treturn (cc __DEPNAME_EXCEPT_W16(name,name)) __P(param);
#define __REDIRECT_EXCEPT_TOW16_(decl,attr,Treturn,cc,name,param,asmname,args)                                           decl attr Treturn (cc __DEPNAME_EXCEPT_W16(name,asmname)) __P(param);
#define __REDIRECT_EXCEPT_TOW16_XVOID(decl,attr,Treturn_nexcept,cc,name,param,args)                                      decl attr Treturn (cc __DEPNAME_EXCEPT_W16(name,name)) __P(param);
#define __REDIRECT_EXCEPT_TOW16_XVOID_(decl,attr,Treturn_nexcept,cc,name,param,asmname,args)                             decl attr Treturn (cc __DEPNAME_EXCEPT_W16(name,asmname)) __P(param);
#define __REDIRECT_EXCEPT_TOW16_VOID(decl,attr,cc,name,param,args)                                                       decl attr void (cc __DEPNAME_EXCEPT_W16(name,name)) __P(param);
#define __REDIRECT_EXCEPT_TOW16_VOID_(decl,attr,cc,name,param,asmname,args)                                              decl attr void (cc __DEPNAME_EXCEPT_W16(name,asmname)) __P(param);
#define __VREDIRECT_EXCEPT_TOW16(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)                         decl attr Treturn (cc __VDEPNAME_EXCEPT_W16(name,name,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_TOW16_(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)               decl attr Treturn (cc __VDEPNAME_EXCEPT_W16(name,asmname,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_TOW16_XVOID(decl,attr,Treturn_nexcept,cc,name,param,vasmnamef,args,before_va_start)           decl attr Treturn (cc __VDEPNAME_EXCEPT_W16(name,name,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_TOW16_XVOID_(decl,attr,Treturn_nexcept,cc,name,param,asmnamef,vasmnamef,args,before_va_start) decl attr Treturn (cc __VDEPNAME_EXCEPT_W16(name,asmname,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_TOW16_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                            decl attr void (cc __VDEPNAME_EXCEPT_W16(name,name,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_TOW16_VOID_(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)                  decl attr void (cc __VDEPNAME_EXCEPT_W16(name,asmname,vasmnamef)) __P(param);
#define __XREDIRECT_EXCEPT_TOW16(decl,attr,Treturn,cc,name,param,code)                                                   decl attr Treturn (cc __DEPNAME_EXCEPT_W16(name,name)) __P(param);
#define __XREDIRECT_EXCEPT_TOW16_(decl,attr,Treturn,cc,name,param,asmname,code)                                          decl attr Treturn (cc __DEPNAME_EXCEPT_W16(name,asmname)) __P(param);
#define __XREDIRECT_EXCEPT_TOW16_XVOID(decl,attr,Treturn_nexcept,cc,name,param,code)                                     decl attr Treturn (cc __DEPNAME_EXCEPT_W16(name,name)) __P(param);
#define __XREDIRECT_EXCEPT_TOW16_XVOID_(decl,attr,Treturn_nexcept,cc,name,param,asmname,code)                            decl attr Treturn (cc __DEPNAME_EXCEPT_W16(name,asmname)) __P(param);
#define __XREDIRECT_EXCEPT_TOW16_VOID(decl,attr,cc,name,param,code)                                                      decl attr void (cc __DEPNAME_EXCEPT_W16(name,name)) __P(param);
#define __XREDIRECT_EXCEPT_TOW16_VOID_(decl,attr,cc,name,param,asmname,code)                                             decl attr void (cc __DEPNAME_EXCEPT_W16(name,asmname)) __P(param);
#define __REDIRECT_EXCEPT_TOW32(decl,attr,Treturn,cc,name,param,args)                                                    decl attr Treturn (cc __DEPNAME_EXCEPT_W32(name,name)) __P(param);
#define __REDIRECT_EXCEPT_TOW32_(decl,attr,Treturn,cc,name,param,asmname,args)                                           decl attr Treturn (cc __DEPNAME_EXCEPT_W32(name,asmname)) __P(param);
#define __REDIRECT_EXCEPT_TOW32_XVOID(decl,attr,Treturn_nexcept,cc,name,param,args)                                      decl attr Treturn (cc __DEPNAME_EXCEPT_W32(name,name)) __P(param);
#define __REDIRECT_EXCEPT_TOW32_XVOID_(decl,attr,Treturn_nexcept,cc,name,param,asmname,args)                             decl attr Treturn (cc __DEPNAME_EXCEPT_W32(name,asmname)) __P(param);
#define __REDIRECT_EXCEPT_TOW32_VOID(decl,attr,cc,name,param,args)                                                       decl attr void (cc __DEPNAME_EXCEPT_W32(name,name)) __P(param);
#define __REDIRECT_EXCEPT_TOW32_VOID_(decl,attr,cc,name,param,asmname,args)                                              decl attr void (cc __DEPNAME_EXCEPT_W32(name,asmname)) __P(param);
#define __VREDIRECT_EXCEPT_TOW32(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)                         decl attr Treturn (cc __VDEPNAME_EXCEPT_W32(name,name,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_TOW32_(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)               decl attr Treturn (cc __VDEPNAME_EXCEPT_W32(name,asmname,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_TOW32_XVOID(decl,attr,Treturn_nexcept,cc,name,param,vasmnamef,args,before_va_start)           decl attr Treturn (cc __VDEPNAME_EXCEPT_W32(name,name,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_TOW32_XVOID_(decl,attr,Treturn_nexcept,cc,name,param,asmnamef,vasmnamef,args,before_va_start) decl attr Treturn (cc __VDEPNAME_EXCEPT_W32(name,asmname,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_TOW32_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                            decl attr void (cc __VDEPNAME_EXCEPT_W32(name,name,vasmnamef)) __P(param);
#define __VREDIRECT_EXCEPT_TOW32_VOID_(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)                  decl attr void (cc __VDEPNAME_EXCEPT_W32(name,asmname,vasmnamef)) __P(param);
#define __XREDIRECT_EXCEPT_TOW32(decl,attr,Treturn,cc,name,param,code)                                                   decl attr Treturn (cc __DEPNAME_EXCEPT_W32(name,name)) __P(param);
#define __XREDIRECT_EXCEPT_TOW32_(decl,attr,Treturn,cc,name,param,asmname,code)                                          decl attr Treturn (cc __DEPNAME_EXCEPT_W32(name,asmname)) __P(param);
#define __XREDIRECT_EXCEPT_TOW32_XVOID(decl,attr,Treturn_nexcept,cc,name,param,code)                                     decl attr Treturn (cc __DEPNAME_EXCEPT_W32(name,name)) __P(param);
#define __XREDIRECT_EXCEPT_TOW32_XVOID_(decl,attr,Treturn_nexcept,cc,name,param,asmname,code)                            decl attr Treturn (cc __DEPNAME_EXCEPT_W32(name,asmname)) __P(param);
#define __XREDIRECT_EXCEPT_TOW32_VOID(decl,attr,cc,name,param,code)                                                      decl attr void (cc __DEPNAME_EXCEPT_W32(name,name)) __P(param);
#define __XREDIRECT_EXCEPT_TOW32_VOID_(decl,attr,cc,name,param,asmname,code)                                             decl attr void (cc __DEPNAME_EXCEPT_W32(name,asmname)) __P(param);
#define __REDIRECT_EXCEPT_W16                                                                                            __REDIRECT_EXCEPT_TOW16_
#define __REDIRECT_EXCEPT_W32                                                                                            __REDIRECT_EXCEPT_TOW32_
#define __REDIRECT_EXCEPT_W16_XVOID                                                                                      __REDIRECT_EXCEPT_TOW16_XVOID_
#define __REDIRECT_EXCEPT_W32_XVOID                                                                                      __REDIRECT_EXCEPT_TOW32_XVOID_
#define __REDIRECT_EXCEPT_W16_VOID                                                                                       __REDIRECT_EXCEPT_TOW16_VOID_
#define __REDIRECT_EXCEPT_W32_VOID                                                                                       __REDIRECT_EXCEPT_TOW32_VOID_
#define __VREDIRECT_EXCEPT_W16                                                                                           __VREDIRECT_EXCEPT_TOW16_
#define __VREDIRECT_EXCEPT_W32                                                                                           __VREDIRECT_EXCEPT_TOW32_
#define __VREDIRECT_EXCEPT_W16_XVOID                                                                                     __VREDIRECT_EXCEPT_TOW16_XVOID_
#define __VREDIRECT_EXCEPT_W32_XVOID                                                                                     __VREDIRECT_EXCEPT_TOW32_XVOID_
#define __VREDIRECT_EXCEPT_W16_VOID                                                                                      __VREDIRECT_EXCEPT_TOW16_VOID_
#define __VREDIRECT_EXCEPT_W32_VOID                                                                                      __VREDIRECT_EXCEPT_TOW32_VOID_
#define __XREDIRECT_EXCEPT_W16                                                                                           __XREDIRECT_EXCEPT_TOW16_
#define __XREDIRECT_EXCEPT_W32                                                                                           __XREDIRECT_EXCEPT_TOW32_
#define __XREDIRECT_EXCEPT_W16_XVOID                                                                                     __XREDIRECT_EXCEPT_TOW16_XVOID_
#define __XREDIRECT_EXCEPT_W32_XVOID                                                                                     __XREDIRECT_EXCEPT_TOW32_XVOID_
#define __XREDIRECT_EXCEPT_W16_VOID                                                                                      __XREDIRECT_EXCEPT_TOW16_VOID_
#define __XREDIRECT_EXCEPT_W32_VOID                                                                                      __XREDIRECT_EXCEPT_TOW32_VOID_


#endif

#undef __INT8_TYPE__
#undef __INT16_TYPE__
#undef __INT32_TYPE__
#undef __INT64_TYPE__
#undef __UINT8_TYPE__
#undef __UINT16_TYPE__
#undef __UINT32_TYPE__
#undef __UINT64_TYPE__
#undef __INT_LEAST8_TYPE__
#undef __INT_LEAST16_TYPE__
#undef __INT_LEAST32_TYPE__
#undef __INT_LEAST64_TYPE__
#undef __UINT_LEAST8_TYPE__
#undef __UINT_LEAST16_TYPE__
#undef __UINT_LEAST32_TYPE__
#undef __UINT_LEAST64_TYPE__
#undef __INT_FAST8_TYPE__
#undef __INT_FAST16_TYPE__
#undef __INT_FAST32_TYPE__
#undef __INT_FAST64_TYPE__
#undef __UINT_FAST8_TYPE__
#undef __UINT_FAST16_TYPE__
#undef __UINT_FAST32_TYPE__
#undef __UINT_FAST64_TYPE__
#undef __INTMAX_TYPE__
#undef __UINTMAX_TYPE__
#undef __INTPTR_TYPE__
#undef __UINTPTR_TYPE__
#undef __SIZE_TYPE__
#undef __PTRDIFF_TYPE__

#ifdef CORRECT_TYPENAMES
typedef int __INT8_TYPE__;
typedef int __INT16_TYPE__;
typedef int __INT32_TYPE__;
typedef int __INT64_TYPE__;
typedef int __UINT8_TYPE__;
typedef int __UINT16_TYPE__;
typedef int __UINT32_TYPE__;
typedef int __UINT64_TYPE__;
typedef int __INT_LEAST8_TYPE__;
typedef int __INT_LEAST16_TYPE__;
typedef int __INT_LEAST32_TYPE__;
typedef int __INT_LEAST64_TYPE__;
typedef int __UINT_LEAST8_TYPE__;
typedef int __UINT_LEAST16_TYPE__;
typedef int __UINT_LEAST32_TYPE__;
typedef int __UINT_LEAST64_TYPE__;
typedef int __INT_FAST8_TYPE__;
typedef int __INT_FAST16_TYPE__;
typedef int __INT_FAST32_TYPE__;
typedef int __INT_FAST64_TYPE__;
typedef int __UINT_FAST8_TYPE__;
typedef int __UINT_FAST16_TYPE__;
typedef int __UINT_FAST32_TYPE__;
typedef int __UINT_FAST64_TYPE__;
typedef int __INTMAX_TYPE__;
typedef int __UINTMAX_TYPE__;
typedef int __INTPTR_TYPE__;
typedef int __UINTPTR_TYPE__;
typedef int __SIZE_TYPE__;
typedef int __PTRDIFF_TYPE__;

#define __INT8_TYPE__         __INT8_TYPE__
#define __INT16_TYPE__        __INT16_TYPE__
#define __INT32_TYPE__        __INT32_TYPE__
#define __INT64_TYPE__        __INT64_TYPE__
#define __UINT8_TYPE__        __UINT8_TYPE__
#define __UINT16_TYPE__       __UINT16_TYPE__
#define __UINT32_TYPE__       __UINT32_TYPE__
#define __UINT64_TYPE__       __UINT64_TYPE__
#define __INT_LEAST8_TYPE__   __INT_LEAST8_TYPE__
#define __INT_LEAST16_TYPE__  __INT_LEAST16_TYPE__
#define __INT_LEAST32_TYPE__  __INT_LEAST32_TYPE__
#define __INT_LEAST64_TYPE__  __INT_LEAST64_TYPE__
#define __UINT_LEAST8_TYPE__  __UINT_LEAST8_TYPE__
#define __UINT_LEAST16_TYPE__ __UINT_LEAST16_TYPE__
#define __UINT_LEAST32_TYPE__ __UINT_LEAST32_TYPE__
#define __UINT_LEAST64_TYPE__ __UINT_LEAST64_TYPE__
#define __INT_FAST8_TYPE__    __INT_FAST8_TYPE__
#define __INT_FAST16_TYPE__   __INT_FAST16_TYPE__
#define __INT_FAST32_TYPE__   __INT_FAST32_TYPE__
#define __INT_FAST64_TYPE__   __INT_FAST64_TYPE__
#define __UINT_FAST8_TYPE__   __UINT_FAST8_TYPE__
#define __UINT_FAST16_TYPE__  __UINT_FAST16_TYPE__
#define __UINT_FAST32_TYPE__  __UINT_FAST32_TYPE__
#define __UINT_FAST64_TYPE__  __UINT_FAST64_TYPE__
#define __INTMAX_TYPE__       __INTMAX_TYPE__
#define __UINTMAX_TYPE__      __UINTMAX_TYPE__
#define __INTPTR_TYPE__       __INTPTR_TYPE__
#define __UINTPTR_TYPE__      __UINTPTR_TYPE__
#define __SIZE_TYPE__         __SIZE_TYPE__
#define __PTRDIFF_TYPE__      __PTRDIFF_TYPE__
#else
typedef int int8_t;
typedef int int16_t;
typedef int int32_t;
typedef int int64_t;
typedef int uint8_t;
typedef int uint16_t;
typedef int uint32_t;
typedef int uint64_t;
typedef int int_least8_t;
typedef int int_least16_t;
typedef int int_least32_t;
typedef int int_least64_t;
typedef int uint_least8_t;
typedef int uint_least16_t;
typedef int uint_least32_t;
typedef int uint_least64_t;
typedef int int_fast8_t;
typedef int int_fast16_t;
typedef int int_fast32_t;
typedef int int_fast64_t;
typedef int uint_fast8_t;
typedef int uint_fast16_t;
typedef int uint_fast32_t;
typedef int uint_fast64_t;
typedef int intmax_t;
typedef int uintmax_t;
typedef int intptr_t;
typedef int uintptr_t;
typedef int size_t;
typedef int ptrdiff_t;
#define __INT8_TYPE__         int8_t
#define __INT16_TYPE__        int16_t
#define __INT32_TYPE__        int32_t
#define __INT64_TYPE__        int64_t
#define __UINT8_TYPE__        uint8_t
#define __UINT16_TYPE__       uint16_t
#define __UINT32_TYPE__       uint32_t
#define __UINT64_TYPE__       uint64_t
#define __INT_LEAST8_TYPE__   int_least8_t
#define __INT_LEAST16_TYPE__  int_least16_t
#define __INT_LEAST32_TYPE__  int_least32_t
#define __INT_LEAST64_TYPE__  int_least64_t
#define __UINT_LEAST8_TYPE__  uint_least8_t
#define __UINT_LEAST16_TYPE__ uint_least16_t
#define __UINT_LEAST32_TYPE__ uint_least32_t
#define __UINT_LEAST64_TYPE__ uint_least64_t
#define __INT_FAST8_TYPE__    int_fast8_t
#define __INT_FAST16_TYPE__   int_fast16_t
#define __INT_FAST32_TYPE__   int_fast32_t
#define __INT_FAST64_TYPE__   int_fast64_t
#define __UINT_FAST8_TYPE__   uint_fast8_t
#define __UINT_FAST16_TYPE__  uint_fast16_t
#define __UINT_FAST32_TYPE__  uint_fast32_t
#define __UINT_FAST64_TYPE__  uint_fast64_t
#define __INTMAX_TYPE__       intmax_t
#define __UINTMAX_TYPE__      uintmax_t
#define __INTPTR_TYPE__       intptr_t
#define __UINTPTR_TYPE__      uintptr_t
#define __SIZE_TYPE__         size_t
#define __PTRDIFF_TYPE__      ptrdiff_t
#endif

typedef int blkaddr32_t;
typedef int blkaddr64_t;
typedef int blkcnt32_t;
typedef int blkcnt64_t;
typedef int blksize_t;
typedef int byte_t;
typedef int caddr_t;
typedef int clock_t;
typedef int clockid_t;
typedef int cpuid_t;
typedef int daddr_t;
typedef int fsblkcnt32_t;
typedef int fsblkcnt64_t;
typedef int fsfilcnt32_t;
typedef int fsfilcnt64_t;
typedef int fsid_t;
typedef int fsint32_t;
typedef int fsint64_t;
typedef int fsuint32_t;
typedef int fsuint64_t;
typedef int fsword32_t;
typedef int fsword64_t;
typedef int gid_t;
typedef int id_t;
typedef int ino32_t;
typedef int ino64_t;
typedef int jtime32_t;
typedef int jtime64_t;
typedef int key_t;
typedef int loff_t;
typedef int lpos_t;
typedef int mode_t;
typedef int nlink_t;
typedef int off32_t;
typedef int off64_t;
typedef int pid_t;
typedef int pos32_t;
typedef int pos64_t;
typedef int qaddr_t;
typedef int quad_t;
typedef int register_t;
typedef int rlim32_t;
typedef int rlim64_t;
typedef int rlim_t;
typedef int sregister_t;
typedef int socklen_t;
typedef int ssize_t;
typedef int suseconds_t;
typedef int syscall_slong_t;
typedef int syscall_ulong_t;
typedef int time32_t;
typedef int time64_t;
typedef int timer_t;
typedef int u_char;
typedef int u_int;
typedef int u_long;
typedef int u_quad_t;
typedef int u_short;
typedef int uid_t;
typedef int uregister_t;
typedef int useconds_t;
typedef int blkcnt_t;
typedef int blkaddr_t;
typedef int dev_t;
typedef int fsblkcnt_t;
typedef int fsfilcnt_t;
typedef int fsint_t;
typedef int fsuint_t;
typedef int fsword_t;
typedef int ino_t;
typedef int irq_t;
typedef int jtime_t;
typedef int major_t;
typedef int minor_t;
typedef int off_t;
typedef int oflag_t;
typedef int pos_t;
typedef int ref_t;
typedef int sysno_t;
typedef int time_t;
#define __blkaddr32_t       blkaddr32_t
#define __blkaddr64_t       blkaddr64_t
#define __blkcnt32_t        blkcnt32_t
#define __blkcnt64_t        blkcnt64_t
#define __blksize_t         blksize_t
#define __byte_t            byte_t
#define __caddr_t           caddr_t
#define __clock_t           clock_t
#define __clockid_t         clockid_t
#define __cpuid_t           cpuid_t
#define __daddr_t           daddr_t
#define __fsblkcnt32_t      fsblkcnt32_t
#define __fsblkcnt64_t      fsblkcnt64_t
#define __fsfilcnt32_t      fsfilcnt32_t
#define __fsfilcnt64_t      fsfilcnt64_t
#define __fsid_t            fsid_t
#define __fsint32_t         fsint32_t
#define __fsint64_t         fsint64_t
#define __fsuint32_t        fsuint32_t
#define __fsuint64_t        fsuint64_t
#define __fsword32_t        fsword32_t
#define __fsword64_t        fsword64_t
#define __gid_t             gid_t
#define __id_t              id_t
#define __ino32_t           ino32_t
#define __ino64_t           ino64_t
#define __int16_t           int16_t
#define __int32_t           int32_t
#define __int64_t           int64_t
#define __int8_t            int8_t
#define __intptr_t          intptr_t
#define __jtime32_t         jtime32_t
#define __jtime64_t         jtime64_t
#define __key_t             key_t
#define __loff_t            loff_t
#define __lpos_t            lpos_t
#define __mode_t            mode_t
#define __nlink_t           nlink_t
#define __off32_t           off32_t
#define __off64_t           off64_t
#define __pid_t             pid_t
#define __pos32_t           pos32_t
#define __pos64_t           pos64_t
#define __ptrdiff_t         ptrdiff_t
#define __qaddr_t           qaddr_t
#define __quad_t            quad_t
#define __register_t        register_t
#define __rlim32_t          rlim32_t
#define __rlim64_t          rlim64_t
#define __rlim_t            rlim_t
#define __size_t            size_t
#define __sregister_t       sregister_t
#define __socklen_t         socklen_t
#define __ssize_t           ssize_t
#define __suseconds_t       suseconds_t
#define __syscall_slong_t   syscall_slong_t
#define __syscall_ulong_t   syscall_ulong_t
#define __time32_t          time32_t
#define __time64_t          time64_t
#define __timer_t           timer_t
#define __u_char            u_char
#define __u_int             u_int
#define __u_long            u_long
#define __u_quad_t          u_quad_t
#define __u_short           u_short
#define __uid_t             uid_t
#define __uint16_t          uint16_t
#define __uint32_t          uint32_t
#define __uint64_t          uint64_t
#define __uint8_t           uint8_t
#define __uintptr_t         uintptr_t
#define __uregister_t       uregister_t
#define __useconds_t        useconds_t
#define __blkcnt_t          blkcnt_t
#define __blkaddr_t         blkaddr_t
#define __dev_t             dev_t
#define __fsblkcnt_t        fsblkcnt_t
#define __fsfilcnt_t        fsfilcnt_t
#define __fsint_t           fsint_t
#define __fsuint_t          fsuint_t
#define __fsword_t          fsword_t
#define __ino_t             ino_t
#define __irq_t             irq_t
#define __jtime_t           jtime_t
#define __major_t           major_t
#define __minor_t           minor_t
#define __off_t             off_t
#define __oflag_t           oflag_t
#define __pos_t             pos_t
#define __ref_t             ref_t
#define __sysno_t           sysno_t
#define __time_t            time_t

#ifndef CORRECT_ARGNAMES
#include "docgen.h"
#endif



