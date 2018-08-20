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
#ifndef GUARD_DEEMON_COMPILER_INTERFACE_H
#define GUARD_DEEMON_COMPILER_INTERFACE_H 1

#include "../api.h"
#include "../object.h"
#include "compiler.h"
#include "ast.h"
#include "tpp.h"

DECL_BEGIN

#ifdef CONFIG_BUILDING_DEEMON
typedef struct compiler_keyword_object DeeCompilerKeywordObject;
typedef struct compiler_symbol_object DeeCompilerSymbolObject;
struct compiler_keyword_object { COMPILER_ITEM_OBJECT_HEAD(struct TPPKeyword) };
struct compiler_symbol_object { COMPILER_ITEM_OBJECT_HEAD(struct symbol) };

INTDEF DeeTypeObject DeeCompilerKeyword_Type;         /* item */
INTDEF DeeTypeObject DeeCompilerSymbol_Type;          /* item */
INTDEF DeeTypeObject DeeCompilerFile_Type;            /* item */
INTDEF DeeTypeObject DeeCompilerLexer_Type;           /* wrapper */
INTDEF DeeTypeObject DeeCompilerLexerKeywords_Type;   /* wrapper */
INTDEF DeeTypeObject DeeCompilerLexerExtensions_Type; /* wrapper */
INTDEF DeeTypeObject DeeCompilerLexerWarnings_Type;   /* wrapper */
INTDEF DeeTypeObject DeeCompilerLexerSyspaths_Type;   /* wrapper */
INTDEF DeeTypeObject DeeCompilerLexerIfdef_Type;      /* wrapper */
INTDEF DeeTypeObject DeeCompilerLexerToken_Type;      /* wrapper */


#ifdef __INTELLISENSE__
INTDEF DREF DeeObject *DCALL DeeCompiler_GetKeyword(struct TPPKeyword *__restrict kwd);
INTDEF DREF DeeObject *DCALL DeeCompiler_GetSymbol(struct symbol *__restrict sym);
INTDEF DREF DeeObject *DCALL DeeCompiler_GetFile(struct TPPFile *__restrict file);
INTDEF DREF DeeObject *DCALL DeeCompiler_GetLexer(DeeCompilerObject *__restrict self);
INTDEF DREF DeeObject *DCALL DeeCompiler_GetLexerKeywords(DeeCompilerObject *__restrict self);
INTDEF DREF DeeObject *DCALL DeeCompiler_GetLexerExtensions(DeeCompilerObject *__restrict self);
INTDEF DREF DeeObject *DCALL DeeCompiler_GetLexerWarnings(DeeCompilerObject *__restrict self);
INTDEF DREF DeeObject *DCALL DeeCompiler_GetLexerSyspaths(DeeCompilerObject *__restrict self);
INTDEF DREF DeeObject *DCALL DeeCompiler_GetLexerIfdef(DeeCompilerObject *__restrict self);
INTDEF DREF DeeObject *DCALL DeeCompiler_GetLexerToken(DeeCompilerObject *__restrict self);
#else
#define DeeCompiler_GetKeyword(kwd)          DeeCompiler_GetItem(&DeeCompilerKeyword_Type,kwd)
#define DeeCompiler_GetSymbol(sym)           DeeCompiler_GetItem(&DeeCompilerSymbol_Type,sym)
#define DeeCompiler_GetFile(file)            DeeCompiler_GetItem(&DeeCompilerFile_Type,file)
#define DeeCompiler_GetLexer(self)           DeeCompiler_GetWrapper(self,&DeeCompilerLexer_Type)
#define DeeCompiler_GetLexerKeywords(self)   DeeCompiler_GetWrapper(self,&DeeCompilerLexerKeywords_Type)
#define DeeCompiler_GetLexerExtensions(self) DeeCompiler_GetWrapper(self,&DeeCompilerLexerExtensions_Type)
#define DeeCompiler_GetLexerWarnings(self)   DeeCompiler_GetWrapper(self,&DeeCompilerLexerWarnings_Type)
#define DeeCompiler_GetLexerSyspaths(self)   DeeCompiler_GetWrapper(self,&DeeCompilerLexerSyspaths_Type)
#define DeeCompiler_GetLexerIfdef(self)      DeeCompiler_GetWrapper(self,&DeeCompilerLexerIfdef_Type)
#define DeeCompiler_GetLexerToken(self)      DeeCompiler_GetWrapper(self,&DeeCompilerLexerToken_Type)
#endif

/* Type fields of DeeCompilerItem_Type and DeeCompilerWrapper_Type */
INTDEF struct type_member DeeCompilerItem_Members[];
INTERN void DCALL DeeCompilerItem_Fini(DeeCompilerItemObject *__restrict self);
INTERN void DCALL DeeCompilerItem_Visit(DeeCompilerItemObject *__restrict self, dvisit_t proc, void *arg);

INTDEF void DCALL DeeCompilerWrapper_Fini(DeeCompilerWrapperObject *__restrict self);
#define DeeCompilerWrapper_Visit    DeeCompilerItem_Visit
#define DeeCompilerWrapper_Members  DeeCompilerItem_Members

#endif /* CONFIG_BUILDING_DEEMON */

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_INTERFACE_H */
