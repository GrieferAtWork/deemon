/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    in a product, an acknowledgement (see the following) in the product     *
 *    documentation is required:                                              *
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_COMPILER_INTERFACE_ILEXER_C
#define GUARD_DEEMON_COMPILER_INTERFACE_ILEXER_C 1

#include <deemon/compiler/compiler.h>

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/compiler/interface.h>
#include <deemon/compiler/tpp.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/int.h>
#include <deemon/map.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/set.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/system-features.h> /* memcpyc(), ... */
#include <deemon/tuple.h>

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"

DECL_BEGIN

INTERN tok_t DCALL
get_token_from_str(char const *__restrict name, bool create_missing) {
	switch (name[0]) {
	case 0:
		return TOK_EOF; /* End-of-file token. */
		/* Tokens containing 2 or more characters. */

	case '<':
		if (name[1] == '<') {
			if (!name[2])
				return TOK_SHL;
			if (name[2] == '=' && !name[3])
				return TOK_SHL_EQUAL;
			if (name[2] == '<') {
				if (!name[3])
					return TOK_LANGLE3;
				if (name[3] == '=' && !name[4])
					return TOK_LANGLE3_EQUAL;
			}
		} else if (name[1] == '=') {
			if (!name[2])
				return TOK_LOWER_EQUAL;
		} else if (name[1] == '>') {
			if (!name[2])
				return TOK_LOGT;
		}
		break;

	case '>':
		if (name[1] == '>') {
			if (!name[2])
				return TOK_SHR;
			if (name[2] == '=' && !name[3])
				return TOK_SHR_EQUAL;
			if (name[2] == '>') {
				if (!name[3])
					return TOK_RANGLE3;
				if (name[3] == '=' && !name[4])
					return TOK_RANGLE3_EQUAL;
			}
		} else if (name[1] == '=') {
			if (!name[2])
				return TOK_GREATER_EQUAL;
		}
		break;

	case '=':
		if (name[1] == '=') {
			if (!name[2])
				return TOK_EQUAL;
			if (name[2] == '=' && !name[3])
				return TOK_EQUAL3;
		}
		break;
	case '!':
		if (name[1] == '=') {
			if (!name[2])
				return TOK_NOT_EQUAL;
			if (name[2] == '=' && !name[3])
				return TOK_NOT_EQUAL3;
		}
		break;

	case '.':
		if (name[1] == '*') {
			if (!name[2])
				return TOK_DOT_STAR;
		} else if (name[1] == '.') {
			if (!name[2])
				return TOK_DOTDOT;
			if (name[2] == '.' && !name[3])
				return TOK_DOTS;
		}
		break;

	case ':':
		if (name[1] == '=') {
			if (!name[2])
				return TOK_COLLON_EQUAL;
		} else if (name[1] == ':') {
			if (!name[2])
				return TOK_NAMESPACE;
		}
		break;

	case '+':
		if (name[1] == '=') {
			if (!name[2])
				return TOK_ADD_EQUAL;
		} else if (name[1] == '+') {
			if (!name[2])
				return TOK_INC;
		}
		break;

	case '-':
		if (name[1] == '=') {
			if (!name[2])
				return TOK_SUB_EQUAL;
		} else if (name[1] == '-') {
			if (!name[2])
				return TOK_DEC;
		} else if (name[1] == '>') {
			if (!name[2])
				return TOK_ARROW;
			if (name[2] == '*' && !name[3])
				return TOK_ARROW_STAR;
		}
		break;

	case '*':
		if (name[1] == '=') {
			if (!name[2])
				return TOK_MUL_EQUAL;
		} else if (name[1] == '*') {
			if (!name[2])
				return TOK_POW;
			if (name[2] == '=' && !name[3])
				return TOK_POW_EQUAL;
		}
		break;

	case '/':
		if (name[1] == '=') {
			if (!name[2])
				return TOK_DIV_EQUAL;
		}
		break;

	case '%':
		if (name[1] == '=') {
			if (!name[2])
				return TOK_MOD_EQUAL;
		}
		break;

	case '&':
		if (name[1] == '=') {
			if (!name[2])
				return TOK_AND_EQUAL;
		} else if (name[1] == '&') {
			if (!name[2])
				return TOK_LAND;
		}
		break;

	case '|':
		if (name[1] == '=') {
			if (!name[2])
				return TOK_OR_EQUAL;
		} else if (name[1] == '|') {
			if (!name[2])
				return TOK_LOR;
		}
		break;

	case '^':
		if (name[1] == '=') {
			if (!name[2])
				return TOK_XOR_EQUAL;
		} else if (name[1] == '^') {
			if (!name[2])
				return TOK_LXOR;
		}
		break;

	case '@':
		if (name[1] == '=') {
			if (!name[2])
				return TOK_AT_EQUAL;
		}
		break;

	case '#':
		if (name[1] == '#') {
			if (!name[2])
				return TOK_GLUE;
		}
		break;

	case '~':
		if (name[1] == '~') {
			if (!name[2])
				return TOK_TILDE_TILDE;
		}
		break;

	default: break;
	}
	/* Simple case: single-character token. */
	if (!name[1])
		return (tok_t)name[0];
	/* Fallback: lookup a keyword for the token. */
	{
		struct TPPKeyword *keyword;
		keyword = TPPLexer_LookupKeyword(name, strlen(name), create_missing);
		if (keyword)
			return keyword->k_id;
		return create_missing ? TOK_ERR : -2;
	}
}

INTERN tok_t DCALL
get_token_from_obj(DeeObject *__restrict obj, bool create_missing) {
	unsigned int result;
	if (DeeString_Check(obj))
		return get_token_from_str(DeeString_STR(obj), create_missing);
	if (DeeObject_AsUInt(obj, &result))
		return TOK_ERR;
	if unlikely((tok_t)result < 0) {
		err_integer_overflow(obj, sizeof(tok_t) * 8, false);
		result = (unsigned int)TOK_ERR;
	}
	return (tok_t)result;
}

PRIVATE char const largetok_names[][4] = {
	/* [TOK_SHL           - TOK_TWOCHAR_BEGIN] = */ { '<', '<' },
	/* [TOK_SHR           - TOK_TWOCHAR_BEGIN] = */ { '>', '>' },
	/* [TOK_EQUAL         - TOK_TWOCHAR_BEGIN] = */ { '=', '=' },
	/* [TOK_NOT_EQUAL     - TOK_TWOCHAR_BEGIN] = */ { '!', '=' },
	/* [TOK_GREATER_EQUAL - TOK_TWOCHAR_BEGIN] = */ { '>', '=' },
	/* [TOK_LOWER_EQUAL   - TOK_TWOCHAR_BEGIN] = */ { '<', '=' },
	/* [TOK_DOTS          - TOK_TWOCHAR_BEGIN] = */ { '.', '.', '.' },
	/* [TOK_ADD_EQUAL     - TOK_TWOCHAR_BEGIN] = */ { '+', '=' },
	/* [TOK_SUB_EQUAL     - TOK_TWOCHAR_BEGIN] = */ { '-', '=' },
	/* [TOK_MUL_EQUAL     - TOK_TWOCHAR_BEGIN] = */ { '*', '=' },
	/* [TOK_DIV_EQUAL     - TOK_TWOCHAR_BEGIN] = */ { '/', '=' },
	/* [TOK_MOD_EQUAL     - TOK_TWOCHAR_BEGIN] = */ { '%', '=' },
	/* [TOK_SHL_EQUAL     - TOK_TWOCHAR_BEGIN] = */ { '<', '<', '=' },
	/* [TOK_SHR_EQUAL     - TOK_TWOCHAR_BEGIN] = */ { '>', '>', '=' },
	/* [TOK_AND_EQUAL     - TOK_TWOCHAR_BEGIN] = */ { '&', '=' },
	/* [TOK_OR_EQUAL      - TOK_TWOCHAR_BEGIN] = */ { '|', '=' },
	/* [TOK_XOR_EQUAL     - TOK_TWOCHAR_BEGIN] = */ { '^', '=' },
	/* [TOK_POW_EQUAL     - TOK_TWOCHAR_BEGIN] = */ { '*', '*', '=' },
	/* [TOK_AT_EQUAL      - TOK_TWOCHAR_BEGIN] = */ { '@', '=' },
	/* [TOK_GLUE          - TOK_TWOCHAR_BEGIN] = */ { '#', '#' },
	/* [TOK_LAND          - TOK_TWOCHAR_BEGIN] = */ { '&', '&' },
	/* [TOK_LOR           - TOK_TWOCHAR_BEGIN] = */ { '|', '|' },
	/* [TOK_LXOR          - TOK_TWOCHAR_BEGIN] = */ { '^', '^' },
	/* [TOK_INC           - TOK_TWOCHAR_BEGIN] = */ { '+', '+' },
	/* [TOK_DEC           - TOK_TWOCHAR_BEGIN] = */ { '-', '-' },
	/* [TOK_POW           - TOK_TWOCHAR_BEGIN] = */ { '*', '*' },
	/* [TOK_TILDE_TILDE   - TOK_TWOCHAR_BEGIN] = */ { '~', '~' },
	/* [TOK_ARROW         - TOK_TWOCHAR_BEGIN] = */ { '-', '>' },
	/* [TOK_COLLON_EQUAL  - TOK_TWOCHAR_BEGIN] = */ { ':', '=' },
	/* [TOK_NAMESPACE     - TOK_TWOCHAR_BEGIN] = */ { ':', ':' },
	/* [TOK_ARROW_STAR    - TOK_TWOCHAR_BEGIN] = */ { '-', '>', '*' },
	/* [TOK_DOT_STAR      - TOK_TWOCHAR_BEGIN] = */ { '.', '*' },
	/* [TOK_DOTDOT        - TOK_TWOCHAR_BEGIN] = */ { '.', '.' },
	/* [TOK_LOGT          - TOK_TWOCHAR_BEGIN] = */ { '<', '>' },
	/* [TOK_LANGLE3       - TOK_TWOCHAR_BEGIN] = */ { '<', '<', '<' },
	/* [TOK_RANGLE3       - TOK_TWOCHAR_BEGIN] = */ { '>', '>', '>' },
	/* [TOK_LANGLE3_EQUAL - TOK_TWOCHAR_BEGIN] = */ { '<', '<', '<', '=' },
	/* [TOK_RANGLE3_EQUAL - TOK_TWOCHAR_BEGIN] = */ { '>', '>', '>', '=' },
	/* [TOK_EQUAL3        - TOK_TWOCHAR_BEGIN] = */ { '=', '=', '=' },
	/* [TOK_NOT_EQUAL3    - TOK_TWOCHAR_BEGIN] = */ { '!', '=', '=' },
};

STATIC_ASSERT(COMPILER_LENOF(largetok_names) ==
              (TOK_TWOCHAR_END - TOK_TWOCHAR_BEGIN));


INTERN WUNUSED DREF DeeObject *DCALL
get_token_name(tok_t id, struct TPPKeyword *kwd) {
	if ((unsigned int)id <= 255) {
		switch (id) {
		case TOK_EOF: return_empty_string;
		case TOK_FLOAT: return DeeString_NewSized(".0", 2);
		case TOK_COMMENT: return DeeString_NewSized("//", 2);
		default: break;
		}
		return DeeString_Chr((uint8_t)id);
	}
	if (id >= TOK_TWOCHAR_BEGIN &&
	    id < TOK_TWOCHAR_END) {
		char const *result = largetok_names[id - TOK_TWOCHAR_BEGIN];
		return DeeString_NewSized(result, result[2] ? (result[3] ? 4 : 3) : 2);
	}
	if (!kwd)
		kwd = TPPLexer_LookupKeywordID(id);
	if unlikely(!kwd)
		return ITER_DONE;
	return DeeString_NewUtf8(kwd->k_name,
	                         kwd->k_size,
	                         STRING_ERROR_FIGNORE);
}

INTERN dhash_t DCALL
get_token_namehash(tok_t id, struct TPPKeyword *kwd) {
	if ((unsigned int)id <= 255) {
		char name[2];
		switch (id) {
		case TOK_EOF: return Dee_HashPtr(name, 0);
		case TOK_FLOAT: return Dee_HashPtr(".0", 2);
		case TOK_COMMENT: return Dee_HashPtr("//", 2);
		default: break;
		}
		name[0] = (char)id;
		return Dee_HashPtr(name, 1);
	}
	if (id >= TOK_TWOCHAR_BEGIN &&
	    id < TOK_TWOCHAR_END) {
		char const *result = largetok_names[id - TOK_TWOCHAR_BEGIN];
		return Dee_HashPtr(result, result[2] ? (result[3] ? 4 : 3) : 2);
	}
	if (!kwd)
		kwd = TPPLexer_LookupKeywordID(id);
	if unlikely(!kwd)
		return (dhash_t)-1;
	return Dee_HashUtf8(kwd->k_name, kwd->k_size);
}



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
keyword_str(DeeCompilerItemObject *__restrict self) {
	DREF DeeObject *result = NULL;
	struct TPPKeyword *item;
	COMPILER_BEGIN(self->ci_compiler);
	item = DeeCompilerItem_VALUE(self, struct TPPKeyword);
	if likely(item)
		result = DeeString_NewUtf8(item->k_name, item->k_size, STRING_ERROR_FIGNORE);
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
keyword_hash(DeeCompilerItemObject *__restrict self) {
	DREF DeeObject *result = NULL;
	struct TPPKeyword *item;
	COMPILER_BEGIN(self->ci_compiler);
	item = DeeCompilerItem_VALUE(self, struct TPPKeyword);
	if likely(item)
		result = DeeInt_NewSize(item->k_hash);
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
keyword_macrofile(DeeCompilerItemObject *__restrict self) {
	DREF DeeObject *result = NULL;
	struct TPPKeyword *item;
	COMPILER_BEGIN(self->ci_compiler);
	item = DeeCompilerItem_VALUE(self, struct TPPKeyword);
	if likely(item) {
		if (!item->k_macro) {
			result = Dee_None;
			Dee_Incref(Dee_None);
		} else {
			result = DeeCompiler_GetFile(item->k_macro);
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
keyword_oldmacrofile(DeeCompilerItemObject *__restrict self) {
	DREF DeeObject *result = NULL;
	struct TPPKeyword *item;
	COMPILER_BEGIN(self->ci_compiler);
	item = DeeCompilerItem_VALUE(self, struct TPPKeyword);
	if likely(item) {
		if (!item->k_rare || !item->k_rare->kr_oldmacro) {
			result = Dee_None;
			Dee_Incref(Dee_None);
		} else {
			result = DeeCompiler_GetFile(item->k_rare->kr_oldmacro);
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
keyword_defmacrofile(DeeCompilerItemObject *__restrict self) {
	DREF DeeObject *result = NULL;
	struct TPPKeyword *item;
	COMPILER_BEGIN(self->ci_compiler);
	item = DeeCompilerItem_VALUE(self, struct TPPKeyword);
	if likely(item) {
		if (!item->k_rare || !item->k_rare->kr_defmacro) {
			result = Dee_None;
			Dee_Incref(Dee_None);
		} else {
			result = DeeCompiler_GetFile(item->k_rare->kr_defmacro);
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
keyword_cachedfile(DeeCompilerItemObject *__restrict self) {
	DREF DeeObject *result = NULL;
	struct TPPKeyword *item;
	COMPILER_BEGIN(self->ci_compiler);
	item = DeeCompilerItem_VALUE(self, struct TPPKeyword);
	if likely(item) {
		if (!item->k_rare || !item->k_rare->kr_file) {
			result = Dee_None;
			Dee_Incref(Dee_None);
		} else {
			result = DeeCompiler_GetFile(item->k_rare->kr_file);
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
keyword_id(DeeCompilerItemObject *__restrict self) {
	DREF DeeObject *result = NULL;
	struct TPPKeyword *item;
	COMPILER_BEGIN(self->ci_compiler);
	item = DeeCompilerItem_VALUE(self, struct TPPKeyword);
	if likely(item)
		result = DeeInt_NewUInt(item->k_id);
	COMPILER_END();
	return result;
}

#define DEFINE_KEYWORD_FLAG_FUNCTIONS(name, flag)                                                                     \
	PRIVATE WUNUSED DREF DeeObject *DCALL                                                                                     \
	keyword_get_##name(DeeCompilerItemObject *__restrict self) {                                                      \
		DREF DeeObject *result = NULL;                                                                                \
		struct TPPKeyword *item;                                                                                      \
		COMPILER_BEGIN(self->ci_compiler);                                                                            \
		item = DeeCompilerItem_VALUE(self, struct TPPKeyword);                                                        \
		if likely(item) {                                                                                             \
			uint32_t flags = TPPKeyword_GetFlags(item, 0);                                                            \
			result         = DeeBool_For(flags & flag);                                                               \
			Dee_Incref(result);                                                                                       \
		}                                                                                                             \
		COMPILER_END();                                                                                               \
		return result;                                                                                                \
	}                                                                                                                 \
	PRIVATE int DCALL                                                                                                 \
	keyword_del_##name(DeeCompilerItemObject *__restrict self) {                                                      \
		int result = -1;                                                                                              \
		struct TPPKeyword *item;                                                                                      \
		COMPILER_BEGIN(self->ci_compiler);                                                                            \
		item = DeeCompilerItem_VALUE(self, struct TPPKeyword);                                                        \
		if likely(item) {                                                                                             \
			uint32_t flags = TPPKeyword_GetFlags(item, 0);                                                            \
			if (!(flags & flag)) {                                                                                    \
				result = 0;                                                                                           \
			} else if (!item->k_rare &&                                                                               \
			           (item->k_rare = (struct TPPRareKeyword *)Dee_Calloc(sizeof(struct TPPRareKeyword))) == NULL) { \
			} else {                                                                                                  \
				item->k_rare->kr_flags = flags & ~flag;                                                               \
				result                 = 0;                                                                           \
			}                                                                                                         \
		}                                                                                                             \
		COMPILER_END();                                                                                               \
		return result;                                                                                                \
	}                                                                                                                 \
	PRIVATE int DCALL                                                                                                 \
	keyword_set_##name(DeeCompilerItemObject *__restrict self,                                                        \
	                   DeeObject *__restrict value) {                                                                 \
		int newval, result = -1;                                                                                      \
		struct TPPKeyword *item;                                                                                      \
		newval = DeeObject_Bool(value);                                                                               \
		if unlikely(newval < 0)                                                                                       \
			goto done;                                                                                                \
		if (!newval)                                                                                                  \
			return keyword_del_##name(self);                                                                          \
		COMPILER_BEGIN(self->ci_compiler);                                                                            \
		item = DeeCompilerItem_VALUE(self, struct TPPKeyword);                                                        \
		if likely(item) {                                                                                             \
			uint32_t flags = TPPKeyword_GetFlags(item, 0);                                                            \
			if (flags & flag) {                                                                                       \
				result = 0;                                                                                           \
			} else if (!item->k_rare &&                                                                               \
			           (item->k_rare = (struct TPPRareKeyword *)Dee_Calloc(sizeof(struct TPPRareKeyword))) == NULL) { \
			} else {                                                                                                  \
				item->k_rare->kr_flags = flags | flag;                                                                \
				result                 = 0;                                                                           \
			}                                                                                                         \
		}                                                                                                             \
		COMPILER_END();                                                                                               \
	done:                                                                                                             \
		return result;                                                                                                \
	}
DEFINE_KEYWORD_FLAG_FUNCTIONS(hasattribute, TPP_KEYWORDFLAG_HAS_ATTRIBUTE)
DEFINE_KEYWORD_FLAG_FUNCTIONS(hasbuiltin, TPP_KEYWORDFLAG_HAS_BUILTIN)
DEFINE_KEYWORD_FLAG_FUNCTIONS(hascppattribute, TPP_KEYWORDFLAG_HAS_CPP_ATTRIBUTE)
DEFINE_KEYWORD_FLAG_FUNCTIONS(hasdeclspecattribute, TPP_KEYWORDFLAG_HAS_DECLSPEC_ATTRIBUTE)
DEFINE_KEYWORD_FLAG_FUNCTIONS(hasextension, TPP_KEYWORDFLAG_HAS_EXTENSION)
DEFINE_KEYWORD_FLAG_FUNCTIONS(hasfeature, TPP_KEYWORDFLAG_HAS_FEATURE)
DEFINE_KEYWORD_FLAG_FUNCTIONS(isdeprecated, TPP_KEYWORDFLAG_IS_DEPRECATED)
DEFINE_KEYWORD_FLAG_FUNCTIONS(ispoisoned, TPP_KEYWORDFLAG_IS_POISONED)
DEFINE_KEYWORD_FLAG_FUNCTIONS(hastppbuiltin, TPP_KEYWORDFLAG_HAS_TPP_BUILTIN)
#undef DEFINE_KEYWORD_FLAG_FUNCTIONS

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
keyword_get_counter(DeeCompilerItemObject *__restrict self) {
	DREF DeeObject *result = NULL;
	struct TPPKeyword *item;
	COMPILER_BEGIN(self->ci_compiler);
	item = DeeCompilerItem_VALUE(self, struct TPPKeyword);
	if likely(item) {
		if (item->k_rare) {
			result = DeeInt_NewS64(item->k_rare->kr_counter);
		} else {
			result = &DeeInt_Zero;
			Dee_Incref(&DeeInt_Zero);
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
keyword_del_counter(DeeCompilerItemObject *__restrict self) {
	int result = -1;
	struct TPPKeyword *item;
	COMPILER_BEGIN(self->ci_compiler);
	item = DeeCompilerItem_VALUE(self, struct TPPKeyword);
	if likely(item) {
		if (item->k_rare)
			item->k_rare->kr_counter = 0;
		result = 0;
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
keyword_set_counter(DeeCompilerItemObject *__restrict self,
                    DeeObject *__restrict value) {
	int result = -1;
	struct TPPKeyword *item;
	int64_t newval;
	if (DeeObject_AsInt64(value, &newval))
		goto done;
	COMPILER_BEGIN(self->ci_compiler);
	item = DeeCompilerItem_VALUE(self, struct TPPKeyword);
	if likely(item) {
		if (item->k_rare) {
			item->k_rare->kr_counter = newval;
			result                   = 0;
		} else if (!newval) {
			result = 0;
		} else if ((item->k_rare = (struct TPPRareKeyword *)Dee_Calloc(sizeof(struct TPPRareKeyword))) != NULL) {
			item->k_rare->kr_counter = newval;
			result                   = 0;
		}
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
keyword_get_uservalue(DeeCompilerItemObject *__restrict self) {
	DREF DeeObject *result = NULL;
	struct TPPKeyword *item;
	COMPILER_BEGIN(self->ci_compiler);
	item = DeeCompilerItem_VALUE(self, struct TPPKeyword);
	if likely(item) {
		if (item->k_rare) {
			result = DeeInt_NewUIntptr((uintptr_t)item->k_rare->kr_user);
		} else {
			result = &DeeInt_Zero;
			Dee_Incref(&DeeInt_Zero);
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
keyword_del_uservalue(DeeCompilerItemObject *__restrict self) {
	int result = -1;
	struct TPPKeyword *item;
	COMPILER_BEGIN(self->ci_compiler);
	item = DeeCompilerItem_VALUE(self, struct TPPKeyword);
	if likely(item) {
		if (item->k_rare)
			item->k_rare->kr_user = (void *)(uintptr_t)0;
		result = 0;
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
keyword_set_uservalue(DeeCompilerItemObject *__restrict self,
                      DeeObject *__restrict value) {
	int result = -1;
	struct TPPKeyword *item;
	uintptr_t newval;
	if (DeeObject_AsUIntptr(value, &newval))
		goto done;
	COMPILER_BEGIN(self->ci_compiler);
	item = DeeCompilerItem_VALUE(self, struct TPPKeyword);
	if likely(item) {
		if (item->k_rare) {
			item->k_rare->kr_user = (void *)newval;
			result                = 0;
		} else if (!newval) {
			result = 0;
		} else if ((item->k_rare = (struct TPPRareKeyword *)Dee_Calloc(sizeof(struct TPPRareKeyword))) != NULL) {
			item->k_rare->kr_user = (void *)newval;
			result                = 0;
		}
	}
	COMPILER_END();
done:
	return result;
}


PRIVATE struct type_getset keyword_getsets[] = {
	{ "id",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&keyword_id, NULL, NULL,
	  DOC("->?Dint\n"
	      "Returns the ID of @this keyword") },
	{ "name",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&keyword_str, NULL, NULL,
	  DOC("->?Dstring\n"
	      "Returns the name of @this keyword. Same as ?#{op:str}") },
	{ "hash",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&keyword_hash, NULL, NULL,
	  DOC("->?Dint\n"
	      "Returns the hash of @this keyword") },
	{ "macrofile",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&keyword_macrofile, NULL, NULL,
	  DOC("->?X2?AFile?ALexer?Ert:Compiler?N\n"
	      "Returns the macro definitions file, or ?N if "
	      "@this keyword isn't being used as a user-defined macro") },
	{ "oldmacrofile",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&keyword_oldmacrofile, NULL, NULL,
	  DOC("->?X2?AFile?ALexer?Ert:Compiler?N\n"
	      "Returns the latest old macro file, that is the first macro "
	      "file definition that is preserved when ${##pragma push_macro(\"foo\")} is used") },
	{ "defmacrofile",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&keyword_defmacrofile, NULL, NULL,
	  DOC("->?X2?AFile?ALexer?Ert:Compiler?N\n"
	      "Returns the default definition of a user-overwritten macro") },
	{ "cachedfile",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&keyword_cachedfile, NULL, NULL,
	  DOC("->?X2?AFile?ALexer?Ert:Compiler?N\n"
	      "Returns a file that has been cached under this keyword") },
	{ "counter",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&keyword_get_counter,
	  (int (DCALL *)(DeeObject *__restrict))&keyword_del_counter,
	  (int (DCALL *)(DeeObject *, DeeObject *))&keyword_set_counter,
	  DOC("->?Dint\n"
	      "Get, del (set to $0), or set counter that is used to implement the builtin "
	      "${__TPP_COUNTER} macro, which can be used to generate unique numbers, based "
	      "on keywords, which are used as keys to access those numbers.\n"
	      "When this field is set, the written value is what ${__TPP_COUNTER} will expand "
	      "to, the next time it is invoked with @this keyword.\n"
	      "Note however, that reading this field will _not_ modify the counter, unlike "
	      "accessing it though ${__TPP_COUNTER}, which will increment it once every time") },
	{ "hasattribute",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&keyword_get_hasattribute,
	  (int (DCALL *)(DeeObject *__restrict))&keyword_del_hasattribute,
	  (int (DCALL *)(DeeObject *, DeeObject *))&keyword_set_hasattribute,
	  DOC("->?Dbool\n"
	      "Get, del (set to ?f), or set the has-attribute flag of @this keyword\n"
	      "The has-attribute flag can then be queried via the ${__has_attribute()} builtin macro") },
	{ "hasbuiltin",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&keyword_get_hasbuiltin,
	  (int (DCALL *)(DeeObject *__restrict))&keyword_del_hasbuiltin,
	  (int (DCALL *)(DeeObject *, DeeObject *))&keyword_set_hasbuiltin,
	  DOC("->?Dbool\n"
	      "Get, del (set to ?f), or set the has-builtin flag of @this keyword\n"
	      "The has-builtin flag can then be queried via the ${__has_builtin()} builtin macro") },
	{ "hascppattribute",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&keyword_get_hascppattribute,
	  (int (DCALL *)(DeeObject *__restrict))&keyword_del_hascppattribute,
	  (int (DCALL *)(DeeObject *, DeeObject *))&keyword_set_hascppattribute,
	  DOC("->?Dbool\n"
	      "Get, del (set to ?f), or set the has-cpp_attribute flag of @this keyword\n"
	      "The has-cpp_attribute flag can then be queried via the ${__has_cpp_attribute()} builtin macro") },
	{ "hasdeclspecattribute",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&keyword_get_hasdeclspecattribute,
	  (int (DCALL *)(DeeObject *__restrict))&keyword_del_hasdeclspecattribute,
	  (int (DCALL *)(DeeObject *, DeeObject *))&keyword_set_hasdeclspecattribute,
	  DOC("->?Dbool\n"
	      "Get, del (set to ?f), or set the has-declspec_attribute flag of @this keyword\n"
	      "The has-declspec_attribute flag can then be queried via the ${__has_declspec_attribute()} builtin macro") },
	{ "hasextension",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&keyword_get_hasextension,
	  (int (DCALL *)(DeeObject *__restrict))&keyword_del_hasextension,
	  (int (DCALL *)(DeeObject *, DeeObject *))&keyword_set_hasextension,
	  DOC("->?Dbool\n"
	      "Get, del (set to ?f), or set the has-extension flag of @this keyword\n"
	      "The has-extension flag can then be queried via the ${__has_extension()} builtin macro") },
	{ "hasfeature",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&keyword_get_hasfeature,
	  (int (DCALL *)(DeeObject *__restrict))&keyword_del_hasfeature,
	  (int (DCALL *)(DeeObject *, DeeObject *))&keyword_set_hasfeature,
	  DOC("->?Dbool\n"
	      "Get, del (set to ?f), or set the has-feature flag of @this keyword\n"
	      "The has-feature flag can then be queried via the ${__has_feature()} builtin macro") },
	{ "isdeprecated",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&keyword_get_isdeprecated,
	  (int (DCALL *)(DeeObject *__restrict))&keyword_del_isdeprecated,
	  (int (DCALL *)(DeeObject *, DeeObject *))&keyword_set_isdeprecated,
	  DOC("->?Dbool\n"
	      "Get, del (set to ?f), or set the is-deprecated flag of @this keyword\n"
	      "The is-deprecated flag can then be queried via the ${__is_deprecated()} builtin macro") },
	{ "ispoisoned",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&keyword_get_ispoisoned,
	  (int (DCALL *)(DeeObject *__restrict))&keyword_del_ispoisoned,
	  (int (DCALL *)(DeeObject *, DeeObject *))&keyword_set_ispoisoned,
	  DOC("->?Dbool\n"
	      "Get, del (set to ?f), or set the is-poisoned flag of @this keyword\n"
	      "The is-poisoned flag can then be queried via the ${__is_poisoned()} builtin macro") },
	{ "hastppbuiltin",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&keyword_get_hastppbuiltin,
	  (int (DCALL *)(DeeObject *__restrict))&keyword_del_hastppbuiltin,
	  (int (DCALL *)(DeeObject *, DeeObject *))&keyword_set_hastppbuiltin,
	  DOC("->?Dbool\n"
	      "Get, del (set to ?f), or set the has-tpp_builtin flag of @this keyword\n"
	      "The has-tpp_builtin flag can then be queried via the ${__has_tpp_builtin()} builtin macro") },
	{ "uservalue",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&keyword_get_uservalue,
	  (int (DCALL *)(DeeObject *__restrict))&keyword_del_uservalue,
	  (int (DCALL *)(DeeObject *, DeeObject *))&keyword_set_uservalue,
	  DOC("->?Dint\n"
	      "@throw IntegerOverflow Attempted to write a negative value, or one that is too large\n"
	      "get, del (set to $0), or set a custom user-value which can be stored within "
	      "keyword descriptors. This value must be an unsigned integer that fits into "
	      "a single pointer, as used by the host") },
	/* TODO: Access to keyword assertions (`kr_asserts')? */
	{ NULL }
};



INTERN DeeTypeObject DeeCompilerKeyword_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_Keyword",
	/* .tp_doc      = */ DOC("str->\n"
	                         "Returns the name of @this keyword"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCompilerItem_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(DeeCompilerItemObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&keyword_str,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ keyword_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_keywords(DeeCompilerWrapperObject *__restrict self) {
	return DeeCompiler_GetLexerKeywords(self->cw_compiler);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_extensions(DeeCompilerWrapperObject *__restrict self) {
	return DeeCompiler_GetLexerExtensions(self->cw_compiler);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_warnings(DeeCompilerWrapperObject *__restrict self) {
	return DeeCompiler_GetLexerWarnings(self->cw_compiler);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_syspaths(DeeCompilerWrapperObject *__restrict self) {
	return DeeCompiler_GetLexerSyspaths(self->cw_compiler);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_ifdef(DeeCompilerWrapperObject *__restrict self) {
	return DeeCompiler_GetLexerIfdef(self->cw_compiler);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_token(DeeCompilerWrapperObject *__restrict self) {
	return DeeCompiler_GetLexerToken(self->cw_compiler);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_file(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	COMPILER_BEGIN(self->cw_compiler);
	if (TPPLexer_Current->l_token.t_file == &TPPFile_Empty) {
		result = Dee_None;
		Dee_Incref(Dee_None);
	} else {
		result = DeeCompiler_GetFile(TPPLexer_Current->l_token.t_file);
	}
	COMPILER_END();
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_textfile(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	struct TPPFile *file;
	COMPILER_BEGIN(self->cw_compiler);
	file = TPPLexer_Textfile();
	if (file == &TPPFile_Empty) {
		result = Dee_None;
		Dee_Incref(Dee_None);
	} else {
		result = DeeCompiler_GetFile(file);
	}
	COMPILER_END();
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_basefile(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	struct TPPFile *file;
	COMPILER_BEGIN(self->cw_compiler);
	file = TPPLexer_Basefile();
	if (file == &TPPFile_Empty) {
		result = Dee_None;
		Dee_Incref(Dee_None);
	} else {
		result = DeeCompiler_GetFile(file);
	}
	COMPILER_END();
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_textposition(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	DREF DeeObject *file_ob;
	struct TPPFile *file;
	COMPILER_BEGIN(self->cw_compiler);
	file = TPPLexer_Current->l_token.t_file;
	if (file == &TPPFile_Empty) {
is_empty_file:
		result = Dee_None;
		Dee_Incref(Dee_None);
	} else {
		char *pointer;
		struct TPPLCInfo lc;
		if (file->f_kind == TPPFILE_KIND_TEXT) {
			pointer = TPPLexer_Current->l_token.t_begin;
		} else {
			file = TPPLexer_Textfile();
			if (file == &TPPFile_Empty)
				goto is_empty_file;
			pointer = file->f_pos;
		}
		file_ob = DeeCompiler_GetFile(file);
		if unlikely(!file_ob) {
			result = NULL;
		} else {
			TPPFile_LCAt(file, &lc, pointer);
			result = DeeTuple_Newf("odd", file_ob, lc.lc_line, lc.lc_col);
			Dee_Decref_unlikely(file_ob);
		}
	}
	COMPILER_END();
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_textendposition(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	DREF DeeObject *file_ob;
	struct TPPFile *file;
	COMPILER_BEGIN(self->cw_compiler);
	file = TPPLexer_Current->l_token.t_file;
	if (file == &TPPFile_Empty) {
is_empty_file:
		result = Dee_None;
		Dee_Incref(Dee_None);
	} else {
		char *pointer;
		struct TPPLCInfo lc;
		if (file->f_kind == TPPFILE_KIND_TEXT) {
			pointer = TPPLexer_Current->l_token.t_end;
		} else {
			file = TPPLexer_Textfile();
			if (file == &TPPFile_Empty)
				goto is_empty_file;
			pointer = file->f_pos;
		}
		file_ob = DeeCompiler_GetFile(file);
		if unlikely(!file_ob) {
			result = NULL;
		} else {
			TPPFile_LCAt(file, &lc, pointer);
			result = DeeTuple_Newf("odd", file_ob, lc.lc_line, lc.lc_col);
			Dee_Decref_unlikely(file_ob);
		}
	}
	COMPILER_END();
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_tokenposition(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	DREF DeeObject *file_ob;
	struct TPPFile *file;
	COMPILER_BEGIN(self->cw_compiler);
	file = TPPLexer_Current->l_token.t_file;
	if (file == &TPPFile_Empty) {
		result = Dee_None;
		Dee_Incref(Dee_None);
	} else {
		struct TPPLCInfo lc;
		TPPFile_LCAt(file, &lc, TPPLexer_Current->l_token.t_begin);
		file_ob = DeeCompiler_GetFile(file);
		if unlikely(!file_ob) {
			result = NULL;
		} else {
			result = DeeTuple_Newf("odd", file_ob, lc.lc_line, lc.lc_col);
			Dee_Decref(file_ob);
		}
	}
	COMPILER_END();
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_tokenendposition(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	DREF DeeObject *file_ob;
	struct TPPFile *file;
	COMPILER_BEGIN(self->cw_compiler);
	file = TPPLexer_Current->l_token.t_file;
	if (file == &TPPFile_Empty) {
		result = Dee_None;
		Dee_Incref(Dee_None);
	} else {
		struct TPPLCInfo lc;
		TPPFile_LCAt(file, &lc, TPPLexer_Current->l_token.t_end);
		file_ob = DeeCompiler_GetFile(file);
		if unlikely(!file_ob) {
			result = NULL;
		} else {
			result = DeeTuple_Newf("odd", file_ob, lc.lc_line, lc.lc_col);
			Dee_Decref(file_ob);
		}
	}
	COMPILER_END();
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_atstartofline(DeeCompilerWrapperObject *__restrict self) {
	int result;
	COMPILER_BEGIN(self->cw_compiler);
	result = TPPLexer_AtStartOfLine();
	COMPILER_END();
	return_bool_(result);
}


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_flags(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	COMPILER_BEGIN(self->cw_compiler);
	result = DeeInt_NewU32(TPPLexer_Current->l_flags & ~TPPLEXER_FLAG_MERGEMASK);
	COMPILER_END();
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
lexer_set_flags(DeeCompilerWrapperObject *__restrict self,
                DeeObject *__restrict value) {
	uint32_t new_flags;
	if (DeeObject_AsUInt32(value, &new_flags))
		return -1;
	COMPILER_BEGIN(self->cw_compiler);
	TPPLexer_Current->l_flags &= TPPLEXER_FLAG_MERGEMASK;
	TPPLexer_Current->l_flags |= new_flags & ~TPPLEXER_FLAG_MERGEMASK;
	COMPILER_END();
	return 0;
}

#define DEFINE_LEXER_FLAG_FUNCTIONS(name, flag)                        \
	INTERN WUNUSED WUNUSED NONNULL((1)) DREF DeeObject *DCALL          \
	lexer_get_##name(DeeCompilerWrapperObject *__restrict self) {      \
		DREF DeeObject *result;                                        \
		COMPILER_BEGIN(self->cw_compiler);                             \
		result = DeeBool_For((TPPLexer_Current->l_flags & flag) != 0); \
		Dee_Incref(result);                                            \
		COMPILER_END();                                                \
		return result;                                                 \
	}                                                                  \
	INTERN WUNUSED WUNUSED NONNULL((1)) int DCALL                      \
	lexer_del_##name(DeeCompilerWrapperObject *__restrict self) {      \
		COMPILER_BEGIN(self->cw_compiler);                             \
		TPPLexer_Current->l_flags &= ~flag;                            \
		COMPILER_END();                                                \
		return 0;                                                      \
	}                                                                  \
	INTERN WUNUSED NONNULL((1, 2)) int DCALL                           \
	lexer_set_##name(DeeCompilerWrapperObject *self,                   \
	                 DeeObject *value) {                               \
		int newval = DeeObject_Bool(value);                            \
		if unlikely(newval < 0)                                        \
			return -1;                                                 \
		COMPILER_BEGIN(self->cw_compiler);                             \
		if (newval)                                                    \
			TPPLexer_Current->l_flags |= flag;                         \
		else {                                                         \
			TPPLexer_Current->l_flags &= ~flag;                        \
		}                                                              \
		COMPILER_END();                                                \
		return 0;                                                      \
	}
DEFINE_LEXER_FLAG_FUNCTIONS(wantcomments, TPPLEXER_FLAG_WANTCOMMENTS)
DEFINE_LEXER_FLAG_FUNCTIONS(wantspace, TPPLEXER_FLAG_WANTSPACE)
DEFINE_LEXER_FLAG_FUNCTIONS(wantlf, TPPLEXER_FLAG_WANTLF)
DEFINE_LEXER_FLAG_FUNCTIONS(noseek_on_eob, TPPLEXER_FLAG_NO_SEEK_ON_EOB)
DEFINE_LEXER_FLAG_FUNCTIONS(nopop_on_eof, TPPLEXER_FLAG_NO_POP_ON_EOF)
DEFINE_LEXER_FLAG_FUNCTIONS(keepmacrospace, TPPLEXER_FLAG_KEEP_MACRO_WHITESPACE)
DEFINE_LEXER_FLAG_FUNCTIONS(nonblocking, TPPLEXER_FLAG_NONBLOCKING)
DEFINE_LEXER_FLAG_FUNCTIONS(terminatestringlf, TPPLEXER_FLAG_TERMINATE_STRING_LF)
DEFINE_LEXER_FLAG_FUNCTIONS(nodirectives, TPPLEXER_FLAG_NO_DIRECTIVES)
DEFINE_LEXER_FLAG_FUNCTIONS(nomacros, TPPLEXER_FLAG_NO_MACROS)
DEFINE_LEXER_FLAG_FUNCTIONS(nobuiltinmacros, TPPLEXER_FLAG_NO_BUILTIN_MACROS)
DEFINE_LEXER_FLAG_FUNCTIONS(asmcomments, TPPLEXER_FLAG_ASM_COMMENTS)
DEFINE_LEXER_FLAG_FUNCTIONS(directives_noown_lf, TPPLEXER_FLAG_DIRECTIVE_NOOWN_LF)
DEFINE_LEXER_FLAG_FUNCTIONS(comments_noown_lf, TPPLEXER_FLAG_COMMENT_NOOWN_LF)
DEFINE_LEXER_FLAG_FUNCTIONS(printmessagelocation, TPPLEXER_FLAG_MESSAGE_LOCATION)
DEFINE_LEXER_FLAG_FUNCTIONS(printmessagenolf, TPPLEXER_FLAG_MESSAGE_NOLINEFEED)
DEFINE_LEXER_FLAG_FUNCTIONS(parseincludestring, TPPLEXER_FLAG_INCLUDESTRING)
DEFINE_LEXER_FLAG_FUNCTIONS(nolegacyguards, TPPLEXER_FLAG_NO_LEGACY_GUARDS)
DEFINE_LEXER_FLAG_FUNCTIONS(werror, TPPLEXER_FLAG_WERROR)
DEFINE_LEXER_FLAG_FUNCTIONS(wsystemheaders, TPPLEXER_FLAG_WSYSTEMHEADERS)
DEFINE_LEXER_FLAG_FUNCTIONS(nodeprecated, TPPLEXER_FLAG_NO_DEPRECATED)
DEFINE_LEXER_FLAG_FUNCTIONS(msvcmessages, TPPLEXER_FLAG_MSVC_MESSAGEFORMAT)
DEFINE_LEXER_FLAG_FUNCTIONS(nowarnings, TPPLEXER_FLAG_NO_WARNINGS)
DEFINE_LEXER_FLAG_FUNCTIONS(noencoding, TPPLEXER_FLAG_NO_ENCODING)
DEFINE_LEXER_FLAG_FUNCTIONS(reemitunknownpragma, TPPLEXER_FLAG_REEMIT_UNKNOWN_PRAGMA)
//DEFINE_LEXER_FLAG_FUNCTIONS(charunsigned, TPPLEXER_FLAG_CHAR_UNSIGNED)
DEFINE_LEXER_FLAG_FUNCTIONS(eofonparen, TPPLEXER_FLAG_EOF_ON_PAREN)
#undef DEFINE_LEXER_FLAG_FUNCTIONS

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_eofparen(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	COMPILER_BEGIN(self->cw_compiler);
	result = DeeInt_NewSize(TPPLexer_Current->l_eof_paren);
	COMPILER_END();
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
lexer_del_eofparen(DeeCompilerWrapperObject *__restrict self) {
	COMPILER_BEGIN(self->cw_compiler);
	TPPLexer_Current->l_eof_paren = 0;
	COMPILER_END();
	return 0;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
lexer_set_eofparen(DeeCompilerWrapperObject *__restrict self,
                   DeeObject *__restrict value) {
	size_t new_eofparen;
	if (DeeObject_AsSize(value, &new_eofparen))
		return -1;
	COMPILER_BEGIN(self->cw_compiler);
	TPPLexer_Current->l_eof_paren = new_eofparen;
	COMPILER_END();
	return 0;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_eobfile(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	COMPILER_BEGIN(self->cw_compiler);
	result = DeeCompiler_GetFile(TPPLexer_Current->l_eob_file);
	COMPILER_END();
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
lexer_del_eobfile(DeeCompilerWrapperObject *__restrict self) {
	COMPILER_BEGIN(self->cw_compiler);
	TPPLexer_Current->l_eob_file = NULL;
	COMPILER_END();
	return 0;
}

INTERN ATTR_COLD int DCALL
err_invalid_file_compiler(DeeCompilerItemObject *__restrict obj) {
	(void)obj;
	return DeeError_Throwf(&DeeError_ValueError,
	                       "File is associated with a different compiler");
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
lexer_set_eobfile(DeeCompilerWrapperObject *__restrict self,
                  DeeObject *__restrict value) {
	struct TPPFile *file;
	int result;
	if (DeeNone_Check(value))
		return lexer_del_eobfile(self);
	result = -1;
	COMPILER_BEGIN(self->cw_compiler);
	if (!DeeObject_InstanceOf(value, &DeeCompilerFile_Type)) {
		DeeObject_TypeAssertFailed(value, &DeeCompilerFile_Type);
	} else if (((DeeCompilerItemObject *)value)->ci_compiler != self->cw_compiler) {
		err_invalid_file_compiler((DeeCompilerItemObject *)value);
	} else {
		file = DeeCompilerItem_VALUE(value, struct TPPFile);
		if likely(file) {
			TPPLexer_Current->l_eob_file = file;
			result                       = 0;
		}
	}
	COMPILER_END();
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_eoffile(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	COMPILER_BEGIN(self->cw_compiler);
	result = DeeCompiler_GetFile(TPPLexer_Current->l_eof_file);
	COMPILER_END();
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
lexer_del_eoffile(DeeCompilerWrapperObject *__restrict self) {
	COMPILER_BEGIN(self->cw_compiler);
	TPPLexer_Current->l_eof_file = NULL;
	COMPILER_END();
	return 0;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
lexer_set_eoffile(DeeCompilerWrapperObject *__restrict self,
                  DeeObject *__restrict value) {
	struct TPPFile *file;
	int result;
	if (DeeNone_Check(value))
		return lexer_del_eoffile(self);
	result = -1;
	COMPILER_BEGIN(self->cw_compiler);
	if (!DeeObject_InstanceOf(value, &DeeCompilerFile_Type)) {
		DeeObject_TypeAssertFailed(value, &DeeCompilerFile_Type);
	} else if (((DeeCompilerItemObject *)value)->ci_compiler != self->cw_compiler) {
		err_invalid_file_compiler((DeeCompilerItemObject *)value);
	} else {
		file = DeeCompilerItem_VALUE(value, struct TPPFile);
		if likely(file) {
			TPPLexer_Current->l_eof_file = file;
			result                       = 0;
		}
	}
	COMPILER_END();
	return result;
}


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_macrolimit(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	COMPILER_BEGIN(self->cw_compiler);
	result = DeeInt_NewSize(TPPLexer_Current->l_limit_mrec);
	COMPILER_END();
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
lexer_del_macrolimit(DeeCompilerWrapperObject *__restrict self) {
	COMPILER_BEGIN(self->cw_compiler);
	TPPLexer_Current->l_limit_mrec = TPPLEXER_DEFAULT_LIMIT_MREC;
	COMPILER_END();
	return 0;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
lexer_set_macrolimit(DeeCompilerWrapperObject *__restrict self,
                     DeeObject *__restrict value) {
	size_t new_macrolimit;
	if (DeeObject_AsSize(value, &new_macrolimit))
		return -1;
	COMPILER_BEGIN(self->cw_compiler);
	TPPLexer_Current->l_limit_mrec = new_macrolimit;
	COMPILER_END();
	return 0;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_includelimit(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	COMPILER_BEGIN(self->cw_compiler);
	result = DeeInt_NewSize(TPPLexer_Current->l_limit_incl);
	COMPILER_END();
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
lexer_del_includelimit(DeeCompilerWrapperObject *__restrict self) {
	COMPILER_BEGIN(self->cw_compiler);
	TPPLexer_Current->l_limit_incl = TPPLEXER_DEFAULT_LIMIT_INCL;
	COMPILER_END();
	return 0;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
lexer_set_includelimit(DeeCompilerWrapperObject *__restrict self,
                       DeeObject *__restrict value) {
	size_t new_includelimit;
	if (DeeObject_AsSize(value, &new_includelimit))
		return -1;
	COMPILER_BEGIN(self->cw_compiler);
	TPPLexer_Current->l_limit_incl = new_includelimit;
	COMPILER_END();
	return 0;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_warningcount(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	COMPILER_BEGIN(self->cw_compiler);
	result = DeeInt_NewSize(TPPLexer_Current->l_warncount);
	COMPILER_END();
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
lexer_del_warningcount(DeeCompilerWrapperObject *__restrict self) {
	COMPILER_BEGIN(self->cw_compiler);
	TPPLexer_Current->l_warncount = 0;
	COMPILER_END();
	return 0;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
lexer_set_warningcount(DeeCompilerWrapperObject *__restrict self,
                       DeeObject *__restrict value) {
	size_t new_warningcount;
	if (DeeObject_AsSize(value, &new_warningcount))
		return -1;
	COMPILER_BEGIN(self->cw_compiler);
	TPPLexer_Current->l_warncount = new_warningcount;
	COMPILER_END();
	return 0;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_errorcount(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	COMPILER_BEGIN(self->cw_compiler);
	result = DeeInt_NewSize(TPPLexer_Current->l_errorcount);
	COMPILER_END();
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
lexer_del_errorcount(DeeCompilerWrapperObject *__restrict self) {
	COMPILER_BEGIN(self->cw_compiler);
	TPPLexer_Current->l_errorcount = 0;
	COMPILER_END();
	return 0;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
lexer_set_errorcount(DeeCompilerWrapperObject *__restrict self,
                     DeeObject *__restrict value) {
	size_t new_errorcount;
	if (DeeObject_AsSize(value, &new_errorcount))
		return -1;
	COMPILER_BEGIN(self->cw_compiler);
	TPPLexer_Current->l_errorcount = new_errorcount;
	COMPILER_END();
	return 0;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_maxerrors(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	COMPILER_BEGIN(self->cw_compiler);
	result = DeeInt_NewSize(TPPLexer_Current->l_maxerrors);
	COMPILER_END();
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
lexer_del_maxerrors(DeeCompilerWrapperObject *__restrict self) {
	COMPILER_BEGIN(self->cw_compiler);
	TPPLexer_Current->l_maxerrors = TPPLEXER_DEFAULT_LIMIT_ECNT;
	COMPILER_END();
	return 0;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
lexer_set_maxerrors(DeeCompilerWrapperObject *__restrict self,
                    DeeObject *__restrict value) {
	size_t new_maxerrors;
	if (DeeObject_AsSize(value, &new_maxerrors))
		return -1;
	COMPILER_BEGIN(self->cw_compiler);
	TPPLexer_Current->l_maxerrors = new_maxerrors;
	COMPILER_END();
	return 0;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_tabsize(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	COMPILER_BEGIN(self->cw_compiler);
	result = DeeInt_NewSize(TPPLexer_Current->l_tabsize);
	COMPILER_END();
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
lexer_del_tabsize(DeeCompilerWrapperObject *__restrict self) {
	COMPILER_BEGIN(self->cw_compiler);
	TPPLexer_Current->l_tabsize = TPPLEXER_DEFAULT_TABSIZE;
	COMPILER_END();
	return 0;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
lexer_set_tabsize(DeeCompilerWrapperObject *__restrict self,
                  DeeObject *__restrict value) {
	size_t new_tabsize;
	if (DeeObject_AsSize(value, &new_tabsize))
		return -1;
	COMPILER_BEGIN(self->cw_compiler);
	TPPLexer_Current->l_tabsize = new_tabsize;
	COMPILER_END();
	return 0;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_counter(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	COMPILER_BEGIN(self->cw_compiler);
	result = DeeInt_NewS64(TPPLexer_Current->l_counter);
	COMPILER_END();
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
lexer_del_counter(DeeCompilerWrapperObject *__restrict self) {
	COMPILER_BEGIN(self->cw_compiler);
	TPPLexer_Current->l_counter = TPPLEXER_DEFAULT_TABSIZE;
	COMPILER_END();
	return 0;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
lexer_set_counter(DeeCompilerWrapperObject *__restrict self,
                  DeeObject *__restrict value) {
	int64_t new_counter;
	if (DeeObject_AsInt64(value, &new_counter))
		return -1;
	COMPILER_BEGIN(self->cw_compiler);
	TPPLexer_Current->l_counter = new_counter;
	COMPILER_END();
	return 0;
}


PRIVATE struct type_getset lexer_getsets[] = {
	{ "keywords",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_keywords, NULL, NULL,
	  DOC("->?#Keywords\n"
	      "Returns a descriptor for recognized keywords, as well as their associated bindings") },
	{ "extensions",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_extensions, NULL, NULL,
	  DOC("->?#Extensions\n"
	      "Returns a descriptor for currently enabled compiler extensions") },
	{ "warnings",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_warnings, NULL, NULL,
	  DOC("->?#Warnings\n"
	      "Returns a descriptor for the current compiler warning state") },
	{ "syspaths",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_syspaths, NULL, NULL,
	  DOC("->?#SysPaths\n"
	      "Returns a sequence representing the system include paths currently being used") },
	{ "ifdef",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_ifdef, NULL, NULL,
	  DOC("->?#Ifdef\n"
	      "Returns a descriptor for the active list of ifdef-blocks") },
	{ "token",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_token, NULL, NULL,
	  DOC("->?#Token\n"
	      "Returns a descriptor for the currently active token") },
	{ "file",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_file, NULL, NULL,
	  DOC("->?X2?#File?N\n"
	      "Returns the currently active file, or ?N if no file is currently active. Same as ?Aid?#token") },
	{ "textfile",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_textfile, NULL, NULL,
	  DOC("->?X2?#File?N\n"
	      "Same as ?#File, but return the first non-macro file") },
	{ "basefile",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_basefile, NULL, NULL,
	  DOC("->?X2?#File?N\n"
	      "Similar to ?#File, but return the base-file (that is the first included file) instead") },
	{ "textposition",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_textposition, NULL, NULL,
	  DOC("->?X2?T3?#File?Dint?Dint?N\n"
	      "Returns a tuple (file, line, column) for the text-position of the current token\n"
	      "In the event that the current file is the result of an expanded macro, the source "
	      "location of the macro invocation site is returned\n"
	      "In the event that no text file is currently loaded, ?N is returned instead") },
	{ "textendposition",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_textendposition, NULL, NULL,
	  DOC("->?X2?T3?#File?Dint?Dint?N\n"
	      "Same as ?#textposition, however when the current file isn't the result of an expanded macro, "
	      "the returned values refer to the end of the current token, rather than its beginning\n"
	      "In the event that no text file is currently loaded, ?N is returned instead") },
	{ "tokenposition",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_tokenposition, NULL, NULL,
	  DOC("->?X2?T3?#File?Dint?Dint?N\n"
	      "Similar to ?#textposition, however in the event of the current token originating "
	      "from a macro, return the source position of that token within the macro, rather "
	      "than the source position of the macro being invoked\n"
	      "Same as ?Aposition?#token") },
	{ "tokenendposition",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_tokenendposition, NULL, NULL,
	  DOC("->?X2?T3?#File?Dint?Dint?N\n"
	      "Same as ?#tokenposition, however return the end position of the current token\n"
	      "Same as ?Aendposition?#token") },
	{ "atstartofline",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_atstartofline, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Returns ?t if the current token is located at the "
	      "start of a line, optionally prefixed by whitespace") },
	{ "flags",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_flags,
	  NULL,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_flags,
	  DOC("->?Dint\n"
	      "Get or set the current general purpose lexer configuration as a whole\n"
	      "The individual bits in the returned integer are prone to getting changed, "
	      "and it is therefor recommended to set the lexer configuration using the "
	      "individual properties instead") },
	{ "wantcomments",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_wantcomments,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_wantcomments,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_wantcomments,
	  DOC("->?Dbool\n"
	      "Configure if comment tokens should, or shouldn't be emit\n"
	      "Note: This field is preserved by ?#flags") },
	{ "wantspace",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_wantspace,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_wantspace,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_wantspace,
	  DOC("->?Dbool\n"
	      "Configure if space-tokens should, or shouldn't be emit\n"
	      "Note: This field is preserved by ?#flags") },
	{ "wantlf",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_wantlf,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_wantlf,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_wantlf,
	  DOC("->?Dbool\n"
	      "Configure if line-feed-tokens should, or shouldn't be emit\n"
	      "Note: This field is preserved by ?#flags") },
	{ "noseek_on_eob",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_noseek_on_eob,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_noseek_on_eob,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_noseek_on_eob,
	  DOC("->?Dbool\n"
	      "When ?t, don't seek the next chunk (s.a. ?#File.nextchunk) "
	      "when the current one ends. Instead, indicate EOF by setting the "
	      "current ?Aid?#token to $0\n"
	      "Note: This field is preserved by ?#flags") },
	{ "nopop_on_eof",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_nopop_on_eof,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_nopop_on_eof,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_nopop_on_eof,
	  DOC("->?Dbool\n"
	      "When ?t, don't automatically pop the current ?#File when it signals "
	      "eof in order to continue parsing older files, but instead propagate the "
	      "EOF by setting the current ?Aid?#token to $0\n"
	      "Note: This field is preserved by ?#flags") },
	{ "keepmacrospace",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_keepmacrospace,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_keepmacrospace,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_keepmacrospace,
	  DOC("->?Dbool\n"
	      "When ?t, don't strip whitespace surrounding the text of macros, but "
	      "keep that whitespace as part of the macro's definition, re-propagating "
	      "it every time that macro is expanded\n"
	      "Note: This field is preserved by ?#flags") },
	{ "nonblocking",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_nonblocking,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_nonblocking,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_nonblocking,
	  DOC("->?Dbool\n"
	      "When ?t, operate in non-blocking mode when loading new chunks from "
	      "files, which essentically means that whenever ?#File.nextchunk is called, "
	      "the $nonblocking argument is set to this value\n"
	      "Note: This field is preserved by ?#flags") },
	{ "terminatestringlf",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_terminatestringlf,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_terminatestringlf,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_terminatestringlf,
	  DOC("->?Dbool\n"
	      "When ?t, regular strings are terminated by line-feeds, which will also "
	      "cause a warning/error to be emit, alongside the incomplete token still "
	      "packaged as a complete string\n"
	      "Note that this also affects ?#File.nextchunk, in that incomplete strings "
	      "near the end of the input stream can also be terminted by line-feeds\n"
	      "Note: This field is preserved by ?#flags") },
	{ "nodirectives",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_nodirectives,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_nodirectives,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_nodirectives,
	  DOC("->?Dbool\n"
	      "When ?t, don't process preprocessor directives, but rather "
	      "re-emit the ${##...} sequences as regular token sequences\n"
	      "Note: This field is preserved by ?#flags") },
	{ "nomacros",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_nomacros,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_nomacros,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_nomacros,
	  DOC("->?Dbool\n"
	      "When ?t, do not expand user-defined macros. Note however that "
	      "builtin macros are still expanded, unless ?#nobuiltinmacros is also "
	      "set to ?t\n"
	      "Note: This field is preserved by ?#flags") },
	{ "nobuiltinmacros",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_nobuiltinmacros,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_nobuiltinmacros,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_nobuiltinmacros,
	  DOC("->?Dbool\n"
	      "When ?t, do not expand builtin macros. Note however that "
	      "user-defined macros are still expanded, unless ?#nomacros is also "
	      "set to ?t\n"
	      "Note: This field is preserved by ?#flags") },
	{ "asmcomments",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_asmcomments,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_asmcomments,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_asmcomments,
	  DOC("->?Dbool\n"
	      "When ?t, unknown preprocessor directives (or all directives when "
	      "#nodirectives is true) are instead emit as comment tokens.\n"
	      "Note however that the requirement of directives having to be located "
	      "at the start of a line, only (and optionally) preceded by whitespace "
	      "still holds, meaning that assembly-like comments are only recognized "
	      "when they are found at the start of a line, too\n"
	      "Note: This field is preserved by ?#flags") },
	{ "directives_noown_lf",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_directives_noown_lf,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_directives_noown_lf,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_directives_noown_lf,
	  DOC("->?Dbool\n"
	      "When ?t, the line-feeds used to terminate a preprocessor directive "
	      "will be re-emit as a regular token (when ?#wantlf is ?t). Otherwise, "
	      "that token will be considered to be apart of the directive and not be "
	      "emit to the caller of ?#next or ?#nextpp\n"
	      "Note: This field is preserved by ?#flags") },
	{ "comments_noown_lf",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_comments_noown_lf,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_comments_noown_lf,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_comments_noown_lf,
	  DOC("->?Dbool\n"
	      "When ?t, the line-feeds used to terminate line-comments "
	      "will be re-emit as a regular token (when ?#wantlf is ?t). Otherwise, "
	      "that token will be considered to be apart of the comment and not be "
	      "emit to the caller of ?#next, ?#nextpp or ?#nextraw\n"
	      "Note: This field is preserved by ?#flags") },
	{ "printmessagelocation",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_printmessagelocation,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_printmessagelocation,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_printmessagelocation,
	  DOC("->?Dbool\n"
	      "When ?t, print the source location before "
	      "the message in ${##pragma message} directives\n"
	      "Note: This field is preserved by ?#flags") },
	{ "printmessagenolf",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_printmessagenolf,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_printmessagenolf,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_printmessagenolf,
	  DOC("->?Dbool\n"
	      "When ?t, don't append a trailing line-feed after "
	      "messages printed using ${##pragma message}\n"
	      "Note: This field is preserved by ?#flags") },
	{ "parseincludestring",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_parseincludestring,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_parseincludestring,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_parseincludestring,
	  DOC("->?Dbool\n"
	      "Parse strings are include-strings, which has the same behavior as parsing "
	      "all strings as though they were raw string literals, meaning that a "
	      "backslash-escape sequences are not recognized\n"
	      "The inteded use for this is to parse the string of an ${##include} directive\n"
	      "Note that this flag also affects the behavior of ?Adecodestring?#token, which "
	      "won't not recognize escape sequences for non-raw string literals, either\n"
	      "Note: This field is preserved by ?#flags") },
	{ "nolegacyguards",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_nolegacyguards,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_nolegacyguards,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_nolegacyguards,
	  DOC("->?Dbool\n"
	      "When ?t, don't automatically try to detect legacy-style ${##include} guards, "
	      "that is an ${##include} guard created by surrounding an entire source file with "
	      "a single ${##ifndef} block\n"
	      "This flag does not, however, affect the functionality of ${##pragma once}\n"
	      "Note: This field is preserved by ?#flags") },
	{ "werror",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_werror,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_werror,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_werror,
	  DOC("->?Dbool\n"
	      "When ?t, turn all warnings into errors\n"
	      "Note: This field is preserved by ?#flags") },
	{ "wsystemheaders",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_wsystemheaders,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_wsystemheaders,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_wsystemheaders,
	  DOC("->?Dbool\n"
	      "When ?t, ignore ?#File.issystemheader, and still produce "
	      "warnings in files marked as system headers\n"
	      "Note: This field is preserved by ?#flags") },
	{ "nodeprecated",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_nodeprecated,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_nodeprecated,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_nodeprecated,
	  DOC("->?Dbool\n"
	      "When ?t, don't emit warnings for keywords marked as ?#Keyword.isdeprecated or ?#Keyword.ispoisoned\n"
	      "Note: This field is preserved by ?#flags") },
	{ "msvcmessages",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_msvcmessages,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_msvcmessages,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_msvcmessages,
	  DOC("->?Dbool\n"
	      "When ?t, the file+line+column in warning and error messages is printed "
	      "as $\"file(line, column) : \". Otherwise it is printed as $\"file:line:column: \"\n"
	      "Note: This field is preserved by ?#flags") },
	{ "nowarnings",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_nowarnings,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_nowarnings,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_nowarnings,
	  DOC("->?Dbool\n"
	      "When ?t, the file+line+column in warning and error messages is printed "
	      "as $\"file(line, column) : \". Otherwise it is printed as $\"file:line:column: \"\n"
	      "Note: This field is preserved by ?#flags") },
	{ "noencoding",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_noencoding,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_noencoding,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_noencoding,
	  DOC("->?Dbool\n"
	      "When ?t, don't decode input text prior to processing it\n"
	      "This essentically means that whenever ?#File.nextchunk is called, "
	      "the $binary is set to this value") },
	{ "reemitunknownpragma",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_reemitunknownpragma,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_reemitunknownpragma,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_reemitunknownpragma,
	  DOC("->?Dbool\n"
	      "When ?t, unknown pragma directives are re-emit, rather than consumed\n"
	      "Note: This field is preserved by ?#flags") },
#if 0
	{ "charunsigned",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_charunsigned,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_charunsigned,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_charunsigned,
	  DOC("->?Dbool\n"
	      "When ?t, characters are undefined when they appear in constant expressions") },
#endif

	{ "eofonparen",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_eofonparen,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_eofonparen,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_eofonparen,
	  DOC("->?Dbool\n"
	      "When ?t, end-of-file is signalled when a matching right-parenthesis "
	      "token is $\")\" is countered (s.a. ?#eofparen)\n"
	      "Note: This field is preserved by ?#flags") },
	{ "eofparen",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_eofparen,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_eofparen,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_eofparen,
	  DOC("->?Dint\n"
	      "Used in conjunction with ?#{eofonparen}: The amount of unmatched $\")\" tokens "
	      "to let through before the next $\")\" token will result in EOF being indicated "
	      "by setting ?Aid?#token to $0") },
	{ "eobfile",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_eobfile,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_eobfile,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_eobfile,
	  DOC("->?#File\n"
	      "When bound, prevent seek-on-end-of-block (that is performing a call to "
	      "#File.nextchunk) when the current is equal to ?#eobfile\n"
	      "This is essentially the same as setting ?#noseek_on_eob to ?t, however "
	      "rather than affecting all files, it only affect a specific file") },
	{ "eoffile",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_eoffile,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_eoffile,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_eoffile,
	  DOC("->?#File\n"
	      "When bound, prevent pop-on-end-of-file (that is popping the current "
	      "file, as done by ?#popfile) when the current is equal to ?#eoffile\n"
	      "This is essentially the same as setting ?#nopop_on_eof to ?t, however "
	      "rather than affecting all files, it only affect a specific file") },
	{ "macrolimit",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_macrolimit,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_macrolimit,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_macrolimit,
	  DOC("->?Dint\n"
	      "The max number of times that a recursive macro (s.a. ?#File.allowselfexpansion) "
	      "is allowed to appear on the macro stack, before an error is emit, and further "
	      "expansion is prevented (defaults to $" PP_STR(TPPLEXER_DEFAULT_LIMIT_MREC) ", "
	                                                                                  "which is also restored when deleting this property)") },
	{ "includelimit",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_includelimit,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_includelimit,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_includelimit,
	  DOC("->?Dint\n"
	      "The max number of files that can be included recursively before it is "
	      "determined that an include recursion has occurred, causing the latest "
	      "inclusion to fail, and an error to be emit (defaults to $" PP_STR(TPPLEXER_DEFAULT_LIMIT_INCL) ", which is also restored when deleting this property)") },
	{ "warningcount",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_warningcount,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_warningcount,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_warningcount,
	  DOC("->?Dint\n"
	      "The total number of warnings that have already been emit "
	      "(including those which have been dismissed)") },
	{ "errorcount",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_errorcount,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_errorcount,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_errorcount,
	  DOC("->?Dint\n"
	      "The total number of errors that have already been emit\n"
	      "When non-zero, later compilation steps should throw a "
	      "compiler error, rather than proceeding with compilation\n"
	      "When this value exceeds ?#maxerrors, a :CompilerError is "
	      "thrown immediately") },
	{ "maxerrors",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_maxerrors,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_maxerrors,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_maxerrors,
	  DOC("->?Dint\n"
	      "The max number of errors which may occurr before compilation is "
	      "halted immediately, by setting the lexer to an error-state, and "
	      "throwing a :CompilerError") },
	{ "tabsize",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_tabsize,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_tabsize,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_tabsize,
	  DOC("->?Dint\n"
	      "The size of tabs (or rather their alignment multiple), as used "
	      "when calculating column offsets for source positions") },
	{ "counter",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_tabsize,
	  (int (DCALL *)(DeeObject *__restrict))&lexer_del_tabsize,
	  (int (DCALL *)(DeeObject *, DeeObject *))&lexer_set_tabsize,
	  DOC("->?Dint\n"
	      "The counter value returned and incremented by the "
	      "$__COUNTER__ builtin macro") },
	{ NULL }
};


PRIVATE struct type_member lexer_class_members[] = {
	TYPE_MEMBER_CONST("Keyword", &DeeCompilerKeyword_Type),
	TYPE_MEMBER_CONST("Keywords", &DeeCompilerLexerKeywords_Type),
	TYPE_MEMBER_CONST("Extensions", &DeeCompilerLexerExtensions_Type),
	TYPE_MEMBER_CONST("Warnings", &DeeCompilerLexerWarnings_Type),
	TYPE_MEMBER_CONST("SysPaths", &DeeCompilerLexerSyspaths_Type),
	TYPE_MEMBER_CONST("Ifdef", &DeeCompilerLexerIfdef_Type),
	TYPE_MEMBER_CONST("Token", &DeeCompilerLexerToken_Type),
	TYPE_MEMBER_CONST("File", &DeeCompilerFile_Type),
	TYPE_MEMBER_END
};

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_include(DeeCompilerWrapperObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *arg, *filename = Dee_None;
	DREF DeeObject *stream;
	/*ref*/ struct TPPFile *file;
	if (DeeArg_Unpack(argc, argv, "o|o:include", &arg, &filename))
		goto err;
	if (!DeeNone_Check(filename) &&
	    DeeObject_AssertTypeExact(filename, &DeeString_Type))
		goto err;
	if (DeeString_Check(arg)) {
		stream = DeeFile_Open(arg, OPEN_FRDONLY, 0);
		if unlikely(!stream)
			goto err;
		if (DeeNone_Check(filename)) {
			filename = arg;
			Dee_Incref(arg);
		}
	} else {
		if (DeeNone_Check(filename)) {
			filename = DeeFile_Filename(arg);
			if unlikely(!filename)
				goto err;
		} else {
			Dee_Incref(filename);
		}
		stream = arg;
		Dee_Incref(stream);
	}
	COMPILER_BEGIN(self->cw_compiler);
	file = TPPFile_OpenStream((stream_t)stream,
	                          DeeString_STR(filename));
	if unlikely(!file) {
		COMPILER_END();
		goto err_filename;
	}
	file->f_textfile.f_ownedstream = (stream_t)stream; /* Inherit reference. */
	/* Push the new file onto the include-stack. */
	TPPLexer_PushFileInherited(file);
	COMPILER_END();
	Dee_Decref(filename);
	return_none;
err_filename:
	Dee_Decref(filename);
/*err_stream:*/
	Dee_Decref(stream);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_nextraw(DeeCompilerWrapperObject *self, size_t argc, DeeObject *const *argv) {
	tok_t result;
	uint16_t old_exceptsz;
	if (DeeArg_Unpack(argc, argv, ":nextraw"))
		goto err;
	old_exceptsz = DeeThread_Self()->t_exceptsz;
	COMPILER_BEGIN(self->cw_compiler);
	result = TPPLexer_YieldRaw();
	COMPILER_END();
	if unlikely(result < 0 && old_exceptsz != DeeThread_Self()->t_exceptsz)
		goto err;
	return DeeInt_NewUInt(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_nextpp(DeeCompilerWrapperObject *self, size_t argc, DeeObject *const *argv) {
	tok_t result;
	uint16_t old_exceptsz;
	if (DeeArg_Unpack(argc, argv, ":nextpp"))
		goto err;
	old_exceptsz = DeeThread_Self()->t_exceptsz;
	COMPILER_BEGIN(self->cw_compiler);
	result = TPPLexer_YieldPP();
	COMPILER_END();
	if unlikely(result < 0 && old_exceptsz != DeeThread_Self()->t_exceptsz)
		goto err;
	return DeeInt_NewUInt(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_next(DeeCompilerWrapperObject *self, size_t argc, DeeObject *const *argv) {
	tok_t result;
	uint16_t old_exceptsz;
	if (DeeArg_Unpack(argc, argv, ":next"))
		goto err;
	old_exceptsz = DeeThread_Self()->t_exceptsz;
	COMPILER_BEGIN(self->cw_compiler);
	result = TPPLexer_Yield();
	COMPILER_END();
	if unlikely(result < 0 && old_exceptsz != DeeThread_Self()->t_exceptsz)
		goto err;
	return DeeInt_NewUInt(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_nextraw_nb(DeeCompilerWrapperObject *self, size_t argc, DeeObject *const *argv) {
	tok_t result;
	uint16_t old_exceptsz;
	if (DeeArg_Unpack(argc, argv, ":nextraw_nb"))
		goto err;
	old_exceptsz = DeeThread_Self()->t_exceptsz;
	COMPILER_BEGIN(self->cw_compiler);
	result = TPPLexer_YieldRawNB();
	COMPILER_END();
	if unlikely(result < 0 && old_exceptsz != DeeThread_Self()->t_exceptsz)
		goto err;
	return DeeInt_NewUInt(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_nextpp_nb(DeeCompilerWrapperObject *self, size_t argc, DeeObject *const *argv) {
	tok_t result;
	uint16_t old_exceptsz;
	if (DeeArg_Unpack(argc, argv, ":nextpp_nb"))
		goto err;
	old_exceptsz = DeeThread_Self()->t_exceptsz;
	COMPILER_BEGIN(self->cw_compiler);
	result = TPPLexer_YieldPPNB();
	COMPILER_END();
	if unlikely(result < 0 && old_exceptsz != DeeThread_Self()->t_exceptsz)
		goto err;
	return DeeInt_NewUInt(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_next_nb(DeeCompilerWrapperObject *self, size_t argc, DeeObject *const *argv) {
	tok_t result;
	uint16_t old_exceptsz;
	if (DeeArg_Unpack(argc, argv, ":next_nb"))
		goto err;
	old_exceptsz = DeeThread_Self()->t_exceptsz;
	COMPILER_BEGIN(self->cw_compiler);
	result = TPPLexer_YieldNB();
	COMPILER_END();
	if unlikely(result < 0 && old_exceptsz != DeeThread_Self()->t_exceptsz)
		goto err;
	return DeeInt_NewUInt(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_seterr(DeeCompilerWrapperObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	if (DeeArg_Unpack(argc, argv, ":seterr"))
		goto err;
	COMPILER_BEGIN(self->cw_compiler);
	result = TPPLexer_SetErr();
	COMPILER_END();
	return_bool_(result != 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_unseterr(DeeCompilerWrapperObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	if (DeeArg_Unpack(argc, argv, ":unseterr"))
		goto err;
	COMPILER_BEGIN(self->cw_compiler);
	result = TPPLexer_UnsetErr();
	COMPILER_END();
	return_bool_(result != 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_popfile(DeeCompilerWrapperObject *self, size_t argc, DeeObject *const *argv) {
	bool result;
	if (DeeArg_Unpack(argc, argv, ":popfile"))
		goto err;
	COMPILER_BEGIN(self->cw_compiler);
	result = TPPLexer_Current->l_token.t_file != &TPPFile_Empty;
	if (result)
		TPPLexer_PopFile();
	COMPILER_END();
	return_bool_(result);
err:
	return NULL;
}


DOC_DEF(lexer_getkwd_doc,
        "(name:?Dstring,create=!t)->?X2?#Keyword?N\n"
        "Lookup the keyword associated with @name and return it, or ?N "
        "when @create is ?f and the keyword hasn't been accessed yet");
PRIVATE struct keyword getkwd_kwlist[] = { K(name), K(create), KEND };

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_getkwd(DeeCompilerWrapperObject *self, size_t argc,
             DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	char *name_utf8;
	DeeObject *name;
	bool create = true;
	struct TPPKeyword *kwd;
	if (DeeArg_UnpackKw(argc, argv, kw, getkwd_kwlist, "o|b:getkwd", &name, &create) ||
	    DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	name_utf8 = DeeString_AsUtf8(name);
	if unlikely(!name_utf8)
		goto err;
	COMPILER_BEGIN(self->cw_compiler);
	kwd = TPPLexer_LookupKeyword(name_utf8,
	                             WSTR_LENGTH(name_utf8),
	                             create);
	if unlikely(!kwd) {
		if (create)
			result = NULL;
		else {
			result = Dee_None;
			Dee_Incref(Dee_None);
		}
	} else {
		result = DeeCompiler_GetKeyword(kwd);
	}
	COMPILER_END();
	return result;
err:
	return NULL;
}

DOC_DEF(lexer_getxkwd_doc,
        "(name:?Dstring,create=!t)->?X2?#Keyword?N\n"
        "Same as ?#getkwd, however the given @name may contain escaped "
        "line-feeds that are removed prior to it being used to lookup "
        "a keyword");
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_getxkwd(DeeCompilerWrapperObject *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	char *name_utf8;
	DeeObject *name;
	bool create = true;
	struct TPPKeyword *kwd;
	if (DeeArg_UnpackKw(argc, argv, kw, getkwd_kwlist, "o|b:getxkwd", &name, &create) ||
	    DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	name_utf8 = DeeString_AsUtf8(name);
	if unlikely(!name_utf8)
		goto err;
	COMPILER_BEGIN(self->cw_compiler);
	kwd = TPPLexer_LookupEscapedKeyword(name_utf8,
	                                    WSTR_LENGTH(name_utf8),
	                                    create);
	if unlikely(!kwd) {
		if (create)
			result = NULL;
		else {
			result = Dee_None;
			Dee_Incref(Dee_None);
		}
	} else {
		result = DeeCompiler_GetKeyword(kwd);
	}
	COMPILER_END();
	return result;
err:
	return NULL;
}

DOC_DEF(lexer_getkwdid_doc,
        "(id:?Dint)->?X2?#Keyword?N\n"
        "Lookup the keyword associated with the given @id, "
        "returning it or ?N if no such keyword exists.\n"
        "WARNING: This is an O(n) operation and should be avoided if at all possible");
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_getkwdid(DeeCompilerWrapperObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	unsigned int id;
	struct TPPKeyword *kwd;
	if (DeeArg_Unpack(argc, argv, "u:getkwdid", &id))
		goto err;
	COMPILER_BEGIN(self->cw_compiler);
	kwd = TPPLexer_LookupKeywordID((tok_t)id);
	if unlikely(!kwd) {
		result = Dee_None;
		Dee_Incref(result);
	} else {
		result = DeeCompiler_GetKeyword(kwd);
	}
	COMPILER_END();
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_undef(DeeCompilerWrapperObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *name;
	int error;
	char *utf8_name;
	uint16_t old_exceptsz;
	if (DeeArg_Unpack(argc, argv, "o:undef", &name) ||
	    DeeObject_AssertTypeExact(name, &DeeString_Type) ||
	    (utf8_name = DeeString_AsUtf8(name)) == NULL)
		goto err;
	old_exceptsz = DeeThread_Self()->t_exceptsz;
	COMPILER_BEGIN(self->cw_compiler);
	error = TPPLexer_Undef(utf8_name, WSTR_LENGTH(utf8_name));
	COMPILER_END();
	if unlikely(old_exceptsz != DeeThread_Self()->t_exceptsz)
		goto err;
	return_bool(error != 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_define(DeeCompilerWrapperObject *self, size_t argc,
             DeeObject *const *argv, DeeObject *kw) {
	DeeObject *name, *value;
	int error;
	char *utf8_name, *utf8_value;
	bool builtin = false;
	uint16_t old_exceptsz;
	PRIVATE struct keyword kwlist[] = { K(name), K(value), K(builtin), KEND };
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "oo|b:define", &name, &value, &builtin) ||
	    DeeObject_AssertTypeExact(name, &DeeString_Type) ||
	    DeeObject_AssertTypeExact(value, &DeeString_Type) ||
	    (utf8_name = DeeString_AsUtf8(name)) == NULL ||
	    (utf8_value = DeeString_AsUtf8(value)) == NULL)
		goto err;
	old_exceptsz = DeeThread_Self()->t_exceptsz;
	COMPILER_BEGIN(self->cw_compiler);
	error = TPPLexer_Define(utf8_name, WSTR_LENGTH(utf8_name),
	                        utf8_value, WSTR_LENGTH(utf8_value),
	                        builtin ? TPPLEXER_DEFINE_FLAG_BUILTIN : TPPLEXER_DEFINE_FLAG_NONE);
	COMPILER_END();
	if unlikely(!error && old_exceptsz != DeeThread_Self()->t_exceptsz)
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE struct keyword assertion_kwlist[] = { K(predicate), K(answer), KEND };

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_addassert(DeeCompilerWrapperObject *self, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	DeeObject *name, *value;
	int error;
	char *utf8_name, *utf8_value;
	uint16_t old_exceptsz;
	if (DeeArg_UnpackKw(argc, argv, kw, assertion_kwlist, "oo:addassert", &name, &value) ||
	    DeeObject_AssertTypeExact(name, &DeeString_Type) ||
	    DeeObject_AssertTypeExact(value, &DeeString_Type) ||
	    (utf8_name = DeeString_AsUtf8(name)) == NULL ||
	    (utf8_value = DeeString_AsUtf8(value)) == NULL)
		goto err;
	old_exceptsz = DeeThread_Self()->t_exceptsz;
	COMPILER_BEGIN(self->cw_compiler);
	error = TPPLexer_AddAssert(utf8_name, WSTR_LENGTH(utf8_name),
	                           utf8_value, WSTR_LENGTH(utf8_value));
	COMPILER_END();
	if unlikely(!error && old_exceptsz != DeeThread_Self()->t_exceptsz)
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_delassert(DeeCompilerWrapperObject *self, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	DeeObject *name, *value = NULL;
	int error;
	char *utf8_name, *utf8_value = NULL;
	uint16_t old_exceptsz;
	if (DeeArg_UnpackKw(argc, argv, kw, assertion_kwlist, "o|o:delassert", &name, &value) ||
	    DeeObject_AssertTypeExact(name, &DeeString_Type) ||
	    (value && DeeObject_AssertTypeExact(value, &DeeString_Type)) ||
	    (utf8_name = DeeString_AsUtf8(name)) == NULL ||
	    (value && (utf8_value = DeeString_AsUtf8(value)) == NULL))
		goto err;
	old_exceptsz = DeeThread_Self()->t_exceptsz;
	COMPILER_BEGIN(self->cw_compiler);
	error = TPPLexer_DelAssert(utf8_name, WSTR_LENGTH(utf8_name),
	                           utf8_value, utf8_value ? WSTR_LENGTH(utf8_value) : 0);
	COMPILER_END();
	if unlikely(old_exceptsz != DeeThread_Self()->t_exceptsz)
		goto err;
	return_bool_(error != 0);
err:
	return NULL;
}


PRIVATE struct type_method lexer_methods[] = {
	{ "include",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&lexer_include,
	  DOC("(stream:?DFile,filename:?Dstring=!N)\n"
	      "(filename:?Dstring,filename:?Dstring=!N)\n"
	      "Include a new file, pushing its contents onto the ${##include}-stack\n"
	      "Note that when including a file with the current token being $0 (as indicate of EOF), "
	      "you must call one of the ?#next-functions in order to load the first token of the newly "
	      "pushed file. - Failing to do so will cause the compiler to not function properly, as it "
	      "will think that no input data is available, causing compiler error to be produced:\n${"
	      "import Compiler from rt;\n"
	      "import File from deemon;\n"
	      "local com = Compiler();\n"
	      "com.lexer.include(File.open(\"input.dee\"));\n"
	      "com.lexer.next(); /* Don't forget to always load the first token */\n"
	      "local ast = com.parser.parse_allstmt();\n"
	      "print ast;}\n"
	      "Hint: In order to tokenize source code from a string, use :File.Reader") },
	{ "nextraw",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&lexer_nextraw,
	  DOC("->?Dint\n"
	      "Load the next token and return its id (no macros, or preprocessor directives are processed)") },
	{ "nextpp",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&lexer_nextpp,
	  DOC("->?Dint\n"
	      "Load the next token and return its id (no macros are processed)") },
	{ "next",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&lexer_next,
	  DOC("->?Dint\n"
	      "Load the next token and return its id") },
	{ "nextraw_nb",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&lexer_nextraw_nb,
	  DOC("->?Dint\n"
	      "Load the next token and return its id while trying not to block (s.a. ?#nonblocking) (no macros, or preprocessor directives are processed)") },
	{ "nextpp_nb",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&lexer_nextpp_nb,
	  DOC("->?Dint\n"
	      "Load the next token and return its id while trying not to block (s.a. ?#nonblocking) (no macros are processed)") },
	{ "next_nb",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&lexer_next_nb,
	  DOC("->?Dint\n"
	      "Load the next token and return its id while trying not to block (s.a. ?#nonblocking)") },
	{ "seterr",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&lexer_seterr,
	  DOC("->?Dbool\n"
	      "Switch the lexer into an error state") },
	{ "unseterr",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&lexer_unseterr,
	  DOC("->?Dbool\n"
	      "Restore the lexer after it was set to an error state") },
	{ "popfile",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&lexer_popfile,
	  DOC("->?Dbool\n"
	      "Pop the last-?#{include}d file and switch back to the file before then (s.a. ?#File)") },
	{ "getkwd",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&lexer_getkwd,
	  DOC_GET(lexer_getkwd_doc),
	  TYPE_METHOD_FKWDS },
	{ "getxkwd",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&lexer_getxkwd,
	  DOC_GET(lexer_getxkwd_doc),
	  TYPE_METHOD_FKWDS },
	{ "getkwdid",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&lexer_getkwdid,
	  DOC_GET(lexer_getkwdid_doc) },
	{ "undef",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&lexer_undef,
	  DOC("(name:?Dstring)->?Dbool\n"
	      "Delete a user-defined macro definition for a macro @name, returning ?t "
	      "if such a definition existed and got deleted, or ?f if no such definition "
	      "existed, and therefor didn't get deleted, either") },
	{ "define",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&lexer_define,
	  DOC("(name:?Dstring,value:?Dstring,builtin=!f)\n"
	      "@param builtin When ?t define the macro as builtin, meaning the "
	      "definition set by @value is restored when resetting macros\n"
	      "Define a new keyword-like macro @name to expand to @value"),
	  TYPE_METHOD_FKWDS },
	{ "addassert",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&lexer_addassert,
	  DOC("(predicate:?Dstring,answer:?Dstring)\n"
	      "Define an assertion @answer for a given @predicate, such that "
	      "${##if #predicate(answer)} evaluates to ?t when encountered "
	      "within a preprcessor expression"),
	  TYPE_METHOD_FKWDS },
	{ "delassert",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&lexer_delassert,
	  DOC("(predicate:?Dstring,answer?:?Dstring)->?Dbool\n"
	      "@return Returns ?t when at least 1 answer got deleted for the given @predicate\n"
	      "Delete an assertion @answer, or all assertions made for a given "
	      "@predicate, such that ${##if #predicate(answer)} no longer evaluates "
	      "to ?t when encountered within a preprcessor expression"),
	  TYPE_METHOD_FKWDS },
	/* TODO */
	{ NULL }
};


INTERN DeeTypeObject DeeCompilerLexer_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_Lexer",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCompilerWrapper_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(DeeCompilerWrapperObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ lexer_methods,
	/* .tp_getsets       = */ lexer_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ lexer_class_members
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_keywords_iter(DeeCompilerWrapperObject *__restrict self) {
	(void)self; /* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
lexer_keywords_getitem(DeeCompilerWrapperObject *self,
                       DeeObject *name) {
	DREF DeeObject *result;
	struct TPPKeyword *kwd;
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		return NULL;
	COMPILER_BEGIN(self->cw_compiler);
	kwd = TPPLexer_LookupKeyword(DeeString_STR(name),
	                             DeeString_SIZE(name),
	                             1);
	if unlikely(!kwd)
		result = NULL;
	else {
		result = DeeCompiler_GetKeyword(kwd);
	}
	COMPILER_END();
	return result;
}

PRIVATE struct type_seq lexer_keywords_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_keywords_iter,
	/* .tp_size      = */ NULL,
	/* .tp_contains  = */ NULL,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&lexer_keywords_getitem,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL
};



PRIVATE struct type_method lexer_keywords_methods[] = {
	{ "getkwd",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&lexer_getkwd,
	  DOC_GET(lexer_getkwd_doc),
	  TYPE_METHOD_FKWDS },
	{ "getxkwd",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&lexer_getxkwd,
	  DOC_GET(lexer_getxkwd_doc),
	  TYPE_METHOD_FKWDS },
	{ "getkwdid",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&lexer_getkwdid,
	  DOC_GET(lexer_getkwdid_doc) },
	{ NULL }
};

INTERN DeeTypeObject DeeCompilerLexerKeywords_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_LexerKeywords",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(DeeCompilerWrapperObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&DeeCompilerWrapper_Fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&DeeCompilerWrapper_Visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &lexer_keywords_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ lexer_keywords_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ DeeCompilerWrapper_Members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject DeeCompilerLexerExtensions_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_LexerExtensions",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCompilerWrapper_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(DeeCompilerWrapperObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL, /* TODO */
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL, /* TODO */
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject DeeCompilerLexerWarnings_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_LexerWarnings",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCompilerWrapper_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(DeeCompilerWrapperObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL, /* TODO */
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL, /* TODO */
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_syspaths_push(DeeCompilerWrapperObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	if (DeeArg_Unpack(argc, argv, ":push"))
		goto err;
	COMPILER_BEGIN(self->cw_compiler);
	result = TPPLexer_PushInclude();
	COMPILER_END();
	if unlikely(!result)
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_syspaths_pop(DeeCompilerWrapperObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	if (DeeArg_Unpack(argc, argv, ":pop"))
		goto err;
	COMPILER_BEGIN(self->cw_compiler);
	result = TPPLexer_PopInclude();
	COMPILER_END();
	return_bool_(result != 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_syspaths_insert(DeeCompilerWrapperObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	DeeObject *path;
	char *utf8, *copy;
	if (DeeArg_Unpack(argc, argv, "o:insert", &path) ||
	    DeeObject_AssertTypeExact(path, &DeeString_Type) ||
	    (utf8 = DeeString_AsUtf8(path)) == NULL)
		goto err;
	copy = (char *)Dee_Malloc(WSTR_LENGTH(path) * sizeof(char));
	if unlikely(!copy)
		goto err;
	memcpyc(copy, path, WSTR_LENGTH(path), sizeof(char));
	COMPILER_BEGIN(self->cw_compiler);
	result = TPPLexer_AddIncludePath(copy, WSTR_LENGTH(path));
	COMPILER_END();
	Dee_Free(copy);
	if unlikely(!result)
		goto err;
	return_bool_(result == 1);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_syspaths_remove(DeeCompilerWrapperObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	DeeObject *path;
	char *utf8, *copy;
	if (DeeArg_Unpack(argc, argv, "o:remove", &path) ||
	    DeeObject_AssertTypeExact(path, &DeeString_Type) ||
	    (utf8 = DeeString_AsUtf8(path)) == NULL)
		goto err;
	copy = (char *)Dee_Malloc(WSTR_LENGTH(path) * sizeof(char));
	if unlikely(!copy)
		goto err;
	memcpyc(copy, path, WSTR_LENGTH(path), sizeof(char));
	COMPILER_BEGIN(self->cw_compiler);
	result = TPPLexer_DelIncludePath(copy, WSTR_LENGTH(path));
	COMPILER_END();
	Dee_Free(copy);
	return_bool_(result != 0);
err:
	return NULL;
}

PRIVATE struct type_method lexer_syspaths_methods[] = {
	{ "push",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&lexer_syspaths_push,
	  DOC("()\n"
	      "Push (remember) the current state of system include paths\n"
	      "This is the same as using ${##pragma TPP include_path(push)}") },
	{ DeeString_STR(&str_pop),
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&lexer_syspaths_pop,
	  DOC("->?Dbool\n"
	      "Pop (restore) a previously pushed system include path state\n"
	      "This is the same as using ${##pragma TPP include_path(pop)}") },
	{ DeeString_STR(&str_insert),
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&lexer_syspaths_insert,
	  DOC("(path:?Dstring)->?Dbool\n"
	      "Append the given @path at the end of the list of system include paths") },
	{ DeeString_STR(&str_remove),
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&lexer_syspaths_remove,
	  DOC("(path:?Dstring)->?Dbool\n"
	      "Remove the given @path from the list of system include paths") },
	{ NULL }
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_syspaths_size(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	COMPILER_BEGIN(self->cw_compiler);
	result = DeeInt_NewSize(TPPLexer_Current->l_syspaths.il_pathc);
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
lexer_syspaths_getitem(DeeCompilerWrapperObject *self,
                       DeeObject *index) {
	DREF DeeObject *result;
	size_t i;
	if (DeeObject_AsSize(index, &i))
		goto err;
	COMPILER_BEGIN(self->cw_compiler);
	if (i >= TPPLexer_Current->l_syspaths.il_pathc) {
		err_index_out_of_bounds((DeeObject *)self, i, TPPLexer_Current->l_syspaths.il_pathc);
		result = NULL;
	} else {
		struct TPPString *string;
		string = TPPLexer_Current->l_syspaths.il_pathv[i];
		result = DeeString_NewUtf8(string->s_text,
		                           string->s_size,
		                           STRING_ERROR_FIGNORE);
	}
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE struct type_seq lexer_syspaths_seq = {
	/* .tp_iter_self = */ NULL,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_syspaths_size,
	/* .tp_contains  = */ NULL,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&lexer_syspaths_getitem,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL
};

INTERN DeeTypeObject DeeCompilerLexerSyspaths_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "lexer_syspaths",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSet_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(DeeCompilerWrapperObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&DeeCompilerWrapper_Fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&DeeCompilerWrapper_Visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &lexer_syspaths_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ lexer_syspaths_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ DeeCompilerWrapper_Members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};



INTERN DeeTypeObject DeeCompilerLexerIfdef_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_LexerIfdef",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCompilerWrapper_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(DeeCompilerWrapperObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL, /* TODO */
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL, /* TODO */
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
token_bool(DeeCompilerWrapperObject *__restrict self) {
	int result;
	COMPILER_BEGIN(self->cw_compiler);
	result = TPPLexer_Global.l_token.t_id > 0;
	COMPILER_END();
	return result;
}

PRIVATE int(TPPCALL unicode_printer_tppappend)(char const *__restrict buf, size_t bufsize, void *arg) {
	dssize_t result;
	result = unicode_printer_print((struct unicode_printer *)arg, buf, bufsize);
	return (unlikely(result < 0))
	       ? -1
	       : 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
token_text(DeeCompilerWrapperObject *__restrict self) {
	int error;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	COMPILER_BEGIN(self->cw_compiler);
	error = TPP_PrintToken(&unicode_printer_tppappend, &printer);
	COMPILER_END();
	if unlikely(error)
		goto err;
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

PRIVATE int(TPPCALL unicode_printer_tppappend_escape)(char const *__restrict buf, size_t bufsize, void *arg) {
	dssize_t result;
	result = DeeFormat_Quote(&unicode_printer_print,
	                         arg, buf, bufsize, FORMAT_QUOTE_FPRINTRAW);
	return (unlikely(result < 0))
	       ? -1
	       : 0;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
token_str(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	COMPILER_BEGIN(self->cw_compiler);
	result = get_token_name(tok, token.t_kwd);
	if unlikely(result == ITER_DONE)
		result = DeeString_Chr((uint32_t)tok); /* Shouldn't normally happen (but may after a partial reset) */
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
token_repr(DeeCompilerWrapperObject *__restrict self) {
	int error;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	COMPILER_BEGIN(self->cw_compiler);
	if unlikely(unicode_printer_putc(&printer, '\"'))
		goto err;
	error = TPP_PrintToken(&unicode_printer_tppappend_escape, &printer);
	COMPILER_END();
	if unlikely(error)
		goto err;
	if unlikely(unicode_printer_putc(&printer, '\"'))
		goto err;
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
token_rawtext(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	COMPILER_BEGIN(self->cw_compiler);
	result = DeeString_NewUtf8(TPPLexer_Current->l_token.t_begin,
	                           (size_t)(TPPLexer_Current->l_token.t_end -
	                                    TPPLexer_Current->l_token.t_begin),
	                           STRING_ERROR_FIGNORE);
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
token_id(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->cw_compiler);
	result = DeeInt_NewInt(TPPLexer_Current->l_token.t_id);
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
token_delid(DeeCompilerWrapperObject *__restrict self) {
	COMPILER_BEGIN(self->cw_compiler);
	TPPLexer_Current->l_token.t_id = 0;
	COMPILER_END();
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
token_setid(DeeCompilerWrapperObject *__restrict self,
            DeeObject *__restrict value) {
	int new_id;
	if (DeeObject_AsInt(value, &new_id))
		return -1;
	COMPILER_BEGIN(self->cw_compiler);
	TPPLexer_Current->l_token.t_id = new_id;
	COMPILER_END();
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
token_num(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->cw_compiler);
	result = DeeInt_NewULong(TPPLexer_Current->l_token.t_num);
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
token_delnum(DeeCompilerWrapperObject *__restrict self) {
	COMPILER_BEGIN(self->cw_compiler);
	TPPLexer_Current->l_token.t_num = 0;
	COMPILER_END();
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
token_setnum(DeeCompilerWrapperObject *__restrict self,
             DeeObject *__restrict value) {
	unsigned long new_num;
	if (DeeObject_AsULong(value, &new_num))
		return -1;
	COMPILER_BEGIN(self->cw_compiler);
	TPPLexer_Current->l_token.t_num = new_num;
	COMPILER_END();
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
token_keyword(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->cw_compiler);
	if (TPP_ISKEYWORD(TPPLexer_Current->l_token.t_id)) {
		result = DeeCompiler_GetKeyword(TPPLexer_Current->l_token.t_kwd);
	} else {
		result = Dee_None;
		Dee_Incref(result);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
token_setkeyword(DeeCompilerWrapperObject *__restrict self,
                 DeeObject *__restrict value) {
	struct TPPKeyword *kwd;
	int result = -1;
	COMPILER_BEGIN(self->cw_compiler);
	if (!DeeObject_InstanceOf(value, &DeeCompilerKeyword_Type)) {
		DeeObject_TypeAssertFailed(value, &DeeCompilerKeyword_Type);
	} else if (((DeeCompilerItemObject *)value)->ci_compiler != self->cw_compiler) {
		DeeError_Throwf(&DeeError_ValueError,
		                "Keyword is associated with a different compiler");
	} else {
		kwd = DeeCompilerItem_VALUE(value, struct TPPKeyword);
		if likely(kwd) {
			TPPLexer_Current->l_token.t_kwd = kwd;
			TPPLexer_Current->l_token.t_id  = kwd->k_id;
			result                          = 0;
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE struct type_getset lexer_token_getsets[] = {
	{ "text",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&token_text, NULL, NULL,
	  DOC("->?Dstring\n"
	      "Returns the textual representation of the current token\n"
	      "Escaped linefeeds have been removed within the returned string") },
	{ "rawtext",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&token_rawtext, NULL, NULL,
	  DOC("->?Dstring\n"
	      "Returns the raw textual representation of the current token\n"
	      "Escaped linefeeds found in the original source are included in the returned string") },
	{ "id",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&token_id,
	  (int (DCALL *)(DeeObject *__restrict))&token_delid,
	  (int (DCALL *)(DeeObject *, DeeObject *))&token_setid,
	  DOC("->?Dint\n"
	      "Get, del (set to $0), or set the id (kind) of the current token") },
	{ "num",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&token_num,
	  (int (DCALL *)(DeeObject *__restrict))&token_delnum,
	  (int (DCALL *)(DeeObject *, DeeObject *))&token_setnum,
	  DOC("->?Dint\n"
	      "Get, del (set to $0), or set the current token number\n"
	      "The current token number is incremented by at least $1 every "
	      "time one of :lexer.next, :lexer.nextpp or :lexer.nextraw are called") },
	{ "keyword",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&token_keyword,
	  NULL,
	  (int (DCALL *)(DeeObject *, DeeObject *))&token_setkeyword,
	  DOC("->?X2?AKeyword?ALexer?Ert:Compiler?N\n"
	      "Returns the keyword associated with the current token, or "
	      ":none if the current token doesn't have an associated keyword\n"
	      "When setting this field, both the token's ?#Keyword, as well "
	      "as its ?#id field are set to the given value") },
	{ "file",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_file, NULL, NULL,
	  DOC("->?X2?AFile?ALexer?Ert:Compiler?N\n"
	      "Returns the currently active file, or ?N if no file is currently active") },
	{ "position",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_tokenposition, NULL, NULL,
	  DOC("->?X2?T3?AFile?ALexer?Ert:Compiler?Dint?Dint?N\n"
	      "Return the exact source position of @this token within a macro definition, or text source") },
	{ "endposition",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_tokenendposition, NULL, NULL,
	  DOC("->?X2?T3?AFile?ALexer?Ert:Compiler?Dint?Dint?N\n"
	      "Same as ?#position, however return the end position of the current token") },
	{ "atstartofline",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_get_atstartofline, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Returns ?t if the current token is located at the "
	      "start of a line, optionally prefixed by whitespace") },
	{ NULL }
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_token_decodestring(DeeCompilerWrapperObject *self, size_t argc, DeeObject *const *argv) {
	int error;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	if (DeeArg_Unpack(argc, argv, ":decodestring"))
		goto err;
	COMPILER_BEGIN(self->cw_compiler);
	if (TPPLexer_Current->l_token.t_id != TOK_STRING ||
	    (tok == TOK_CHAR && !HAS(EXT_CHARACTER_LITERALS))) {
		error = DeeError_Throwf(&DeeError_ValueError,
		                        "The current token isn't a string");
	} else {
		error = ast_decode_unicode_string(&printer);
	}
	COMPILER_END();
	if unlikely(error)
		goto err;
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_token_decodeinteger(DeeCompilerWrapperObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result = NULL;
	bool warnchar          = true;
	if (DeeArg_Unpack(argc, argv, "|b:decodeinteger", &warnchar))
		goto done2;
	COMPILER_BEGIN(self->cw_compiler);
	if (TPPLexer_Current->l_token.t_id == TOK_INT) {
		result = DeeInt_FromString(token.t_begin, (size_t)(token.t_end - token.t_begin),
		                           DEEINT_STRING(0, DEEINT_STRING_FESCAPED));
	} else if (TPPLexer_Current->l_token.t_id == TOK_CHAR) {
		tint_t value;
		if unlikely(TPP_Atoi(&value) == TPP_ATOI_ERR)
			goto done;
		if (warnchar && WARN(W_DEPRECATED_CHARACTER_INT))
			goto done;
		result = DeeInt_NewS64(value);
	} else {
		DeeError_Throwf(&DeeError_ValueError,
		                "The current token isn't an integer or character");
	}
done:
	COMPILER_END();
done2:
	return result;
}

PRIVATE struct type_method lexer_token_methods[] = {
	{ "decodestring",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&lexer_token_decodestring,
	  DOC("->?Dstring\n"
	      "@throw ValueError The current token isn't a string\n"
	      "@throw UnicodeDecodeError The string contains an invalid escape-character\n"
	      "Decode the current token (which must be a string-token) as a string") },
	{ "decodeinteger",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&lexer_token_decodeinteger,
	  DOC("(warnchar=!t)->?Dint\n"
	      "@throw ValueError The current token isn't an integer, or character\n"
	      "@throw ValueError The current token contains an invalid digit\n"
	      "Decode the current token (which must be an integer or character) as an :int object\n"
	      "When @warnchar is ?t, emit a warning when a character is used as an integer") },
	{ NULL }
};


PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
token_hash(DeeCompilerWrapperObject *__restrict self) {
	dhash_t result;
	COMPILER_BEGIN(self->cw_compiler);
	result = get_token_namehash(tok, token.t_kwd);
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
token_eq(DeeCompilerWrapperObject *self,
         DeeObject *other) {
	bool result;
	char const *other_utf8;
	tok_t other_id;
	if (DeeObject_AssertTypeExact(other, &DeeString_Type) ||
	    (other_utf8 = DeeString_AsUtf8(other)) == NULL)
		return NULL;
	COMPILER_BEGIN(self->cw_compiler);
	other_id = get_token_from_str(other_utf8, false);
	result   = tok == other_id;
	COMPILER_END();
	return_bool_(result);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
token_ne(DeeCompilerWrapperObject *self,
         DeeObject *other) {
	bool result;
	char const *other_utf8;
	tok_t other_id;
	if (DeeObject_AssertTypeExact(other, &DeeString_Type) ||
	    (other_utf8 = DeeString_AsUtf8(other)) == NULL)
		return NULL;
	COMPILER_BEGIN(self->cw_compiler);
	other_id = get_token_from_str(other_utf8, false);
	result   = tok != other_id;
	COMPILER_END();
	return_bool_(result);
}

PRIVATE struct type_cmp token_cmp = {
	/* .tp_hash = */ (dhash_t (DCALL *)(DeeObject *__restrict))&token_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&token_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&token_ne
};


INTERN DeeTypeObject DeeCompilerLexerToken_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_Token",
	/* .tp_doc      = */ DOC("str->\n"
	                         "Returns a string representation for the ?#id of @this token\n"
	                         "For most tokens, this is equivalent to ?#text, with the exception of the following:\n"

	                         "#T{Str-value|Token|Description~"
	                         "$\"\"|${<EOF>}|End-of-file is encoded as an empty string&"
	                         "$\"\\\'\"|$\'x\'|Character tokens have a single-quote as str-id&"
	                         "$\"\\\"\"|$\"foo\"|String tokens have a double-quote as str-id (including raw string literals)&"
	                         "$\"0\"|$42|Integer tokens have use the digit 0 as str-id&"
	                         "$\".0\"|${1.5}|Floating point tokens are encoded as `.0' as str-id&"
	                         "$\"\\n\"|#C{<LF>}|Any kind of line-feed token is encoded as an LF-character&"
	                         "$\" \"|#C{<SPACE>}|A space-sequence of any sort of length is encoded as a single space character&"
	                         "$\"//\"|#C{<COMMENT>}|Any kind of comment token is encoded as 2 forward slashes #C{//}}\n"

	                         "\n"
	                         "repr->\n"
	                         "Same as ${repr this.text}\n"
	                         "\n"
	                         "bool->\n"
	                         "Returns ?t if @this token has a non-negative and non-zero ?#id\n"
	                         "\n"
	                         "hash->\n"
	                         "Returns the hash value of ${str this}\n"
	                         "\n"
	                         "==(name:?Dstring)->\n"
	                         "!=(name:?Dstring)->\n"
	                         "Compare ${str this} with the given @name\n"
	                         "This operator, alongside ?#{op:hash} allows tokens to be used in switch-statements\n"

	                         "${"
	                         "switch (com.lexer.token) {\n"
	                         "case \"foobar\":\n"
	                         "	print \"keyword: foobar\";\n"
	                         "	break;\n"
	                         "case \"(\":\n"
	                         "	print \"token: lparen\";\n"
	                         "	break;\n"
	                         "case \"++\":\n"
	                         "	print \"token: increment\";\n"
	                         "	break;\n"
	                         "case \"\":\n"
	                         "	print \"Special token: end-of-file\";\n"
	                         "	break;\n"
	                         "case \"//\":\n"
	                         "	print \"Special token: comment\";\n"
	                         "	break;\n"
	                         "case \"\\\"\":\n"
	                         "	print \"Special token: string\", com.lexer.token.decodestring();\n"
	                         "	break;\n"
	                         "case \"\\\'\":\n"
	                         "	print \"Special token: character\", com.lexer.token.decodeinteger();\n"
	                         "	break;\n"
	                         "case \"0\":\n"
	                         "	print \"Special token: integer\", com.lexer.token.decodeinteger();\n"
	                         "	break;\n"
	                         "default:\n"
	                         "	print \"Other: \", repr com.lexer.token;\n"
	                         "	break;\n"
	                         "}}"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCompilerWrapper_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(DeeCompilerWrapperObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&token_str,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&token_repr,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&token_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &token_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ lexer_token_methods,
	/* .tp_getsets       = */ lexer_token_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_str(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		result = DeeString_Newf("<file %$q>",
		                        file->f_namesize,
		                        file->f_name);
	}
	COMPILER_END();
	return result;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_istext(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		result = DeeBool_For(file->f_kind == TPPFILE_KIND_TEXT);
		Dee_Incref(result);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_ismacro(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		result = DeeBool_For(file->f_kind == TPPFILE_KIND_MACRO);
		Dee_Incref(result);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_isexpand(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		result = DeeBool_For(file->f_kind == TPPFILE_KIND_EXPLICIT);
		Dee_Incref(result);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_origin(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (!file->f_prev || file->f_prev == &TPPFile_Empty) {
			result = Dee_None;
			Dee_Incref(Dee_None);
		} else {
			result = DeeCompiler_GetFile(file->f_prev);
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_alltext(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		result = DeeString_NewUtf8(file->f_begin,
		                           (size_t)(file->f_end - file->f_begin),
		                           STRING_ERROR_FIGNORE);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_nexttext(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		result = DeeString_NewUtf8(file->f_pos,
		                           (size_t)(file->f_end - file->f_pos),
		                           STRING_ERROR_FIGNORE);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_position(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		struct TPPLCInfo lc;
		TPPFile_LCAt(file, &lc, file->f_pos);
		result = DeeTuple_Newf("dd", lc.lc_line, lc.lc_col);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_filename(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		char const *filename;
		size_t filename_length;
		filename = TPPFile_Filename(file, &filename_length);
		result   = DeeString_NewUtf8(filename, filename_length, STRING_ERROR_FIGNORE);
	}
	COMPILER_END();
	return result;
}

PRIVATE ATTR_COLD int DCALL
err_not_a_textfile(struct TPPFile *__restrict file) {
	(void)file;
	return DeeError_Throwf(&DeeError_ValueError,
	                       "This file isn't a text file");
}

PRIVATE ATTR_COLD int DCALL
err_not_a_macrofile(struct TPPFile *__restrict file) {
	(void)file;
	return DeeError_Throwf(&DeeError_ValueError,
	                       "This file isn't a macro file");
}

PRIVATE ATTR_COLD int DCALL
err_not_a_keywordmacrofile(struct TPPFile *__restrict file) {
	(void)file;
	return DeeError_Throwf(&DeeError_ValueError,
	                       "This file isn't a keyword-like macro file");
}

PRIVATE ATTR_COLD int DCALL
err_not_a_functionmacrofile(struct TPPFile *__restrict file) {
	(void)file;
	return DeeError_Throwf(&DeeError_ValueError,
	                       "This file isn't a function-like macro file");
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
file_delfilename(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	int result = -1;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_TEXT) {
			err_not_a_textfile(file);
		} else {
			if (file->f_textfile.f_usedname) {
				TPPString_Decref(file->f_textfile.f_usedname);
				file->f_textfile.f_usedname = NULL;
			}
			result = 0;
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
file_setfilename(DeeCompilerItemObject *__restrict self,
                 DeeObject *__restrict value) {
	struct TPPFile *file;
	char *utf8;
	int result = -1;
	if (DeeObject_AssertTypeExact(value, &DeeString_Type))
		goto done;
	utf8 = DeeString_AsUtf8(value);
	if unlikely(!utf8)
		goto done;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_TEXT) {
			err_not_a_textfile(file);
		} else {
			struct TPPString *new_used_name;
			new_used_name = TPPString_New(utf8, WSTR_LENGTH(utf8));
			if likely(new_used_name) {
				if (file->f_textfile.f_usedname)
					TPPString_Decref(file->f_textfile.f_usedname);
				file->f_textfile.f_usedname = new_used_name; /* Inherit reference. */
				result                      = 0;
			}
		}
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_realfilename(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		char const *filename;
		size_t filename_length;
		filename = TPPFile_RealFilename(file, &filename_length);
		result   = DeeString_NewUtf8(filename, filename_length, STRING_ERROR_FIGNORE);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_name(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		result = DeeString_NewUtf8(file->f_name,
		                           file->f_namesize,
		                           STRING_ERROR_FIGNORE);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_lineoffset(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_TEXT) {
			err_not_a_textfile(file);
		} else {
			result = DeeInt_NewInt(file->f_textfile.f_lineoff);
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
file_dellineoffset(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	int result = -1;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_TEXT) {
			err_not_a_textfile(file);
		} else {
			file->f_textfile.f_lineoff = 0;
			result                     = 0;
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
file_setlineoffset(DeeCompilerItemObject *__restrict self,
                   DeeObject *__restrict value) {
	struct TPPFile *file;
	int result = -1, new_value;
	if (DeeObject_AsInt(value, &new_value))
		goto done;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_TEXT) {
			err_not_a_textfile(file);
		} else {
			file->f_textfile.f_lineoff = (line_t)new_value;
			result                     = 0;
		}
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_stream(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_TEXT) {
			err_not_a_textfile(file);
		} else {
			result = (DREF DeeObject *)file->f_textfile.f_ownedstream;
			if (result == (DREF DeeObject *)TPP_STREAM_INVALID)
				result = (DREF DeeObject *)file->f_textfile.f_stream;
			if likely(result)
				Dee_Incref(result);
			else {
				err_unbound_attribute(&DeeCompilerFile_Type, "stream");
			}
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_getguard(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_TEXT) {
			err_not_a_textfile(file);
		} else if (!file->f_textfile.f_guard) {
			result = Dee_None;
			Dee_Incref(result);
		} else {
			result = DeeCompiler_GetKeyword(file->f_textfile.f_guard);
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
file_delguard(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	int result = -1;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_TEXT) {
			err_not_a_textfile(file);
		} else {
			file->f_textfile.f_guard = NULL;
			result                   = 0;
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
file_setguard(DeeCompilerItemObject *__restrict self,
              DeeObject *__restrict value) {
	struct TPPFile *file;
	int result;
	struct TPPKeyword *kwd;
	if (DeeNone_Check(value))
		return file_delguard(self);
	result = -1;
	COMPILER_BEGIN(self->ci_compiler);
	if (!DeeObject_InstanceOf(value, &DeeCompilerKeyword_Type)) {
		DeeObject_TypeAssertFailed(value, &DeeCompilerKeyword_Type);
	} else if (((DeeCompilerItemObject *)value)->ci_compiler != self->ci_compiler) {
		DeeError_Throwf(&DeeError_ValueError,
		                "Keyword is associated with a different compiler");
	} else {
		file = DeeCompilerItem_VALUE(self, struct TPPFile);
		if likely(file) {
			if (file->f_kind != TPPFILE_KIND_TEXT) {
				err_not_a_textfile(file);
			} else {
				kwd = DeeCompilerItem_VALUE(value, struct TPPKeyword);
				if likely(kwd) {
					file->f_textfile.f_guard = kwd;
					result                   = 0;
				}
			}
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_getnewguard(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_TEXT) {
			err_not_a_textfile(file);
		} else if (!file->f_textfile.f_newguard) {
			result = Dee_None;
			Dee_Incref(result);
		} else {
			result = DeeCompiler_GetKeyword(file->f_textfile.f_newguard);
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
file_delnewguard(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	int result = -1;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_TEXT) {
			err_not_a_textfile(file);
		} else {
			file->f_textfile.f_newguard = NULL;
			result                      = 0;
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
file_setnewguard(DeeCompilerItemObject *__restrict self,
                 DeeObject *__restrict value) {
	struct TPPFile *file;
	int result;
	struct TPPKeyword *kwd;
	if (DeeNone_Check(value))
		return file_delguard(self);
	result = -1;
	COMPILER_BEGIN(self->ci_compiler);
	if (!DeeObject_InstanceOf(value, &DeeCompilerKeyword_Type)) {
		DeeObject_TypeAssertFailed(value, &DeeCompilerKeyword_Type);
	} else if (((DeeCompilerItemObject *)value)->ci_compiler != self->ci_compiler) {
		DeeError_Throwf(&DeeError_ValueError,
		                "Keyword is associated with a different compiler");
	} else {
		file = DeeCompilerItem_VALUE(self, struct TPPFile);
		if likely(file) {
			if (file->f_kind != TPPFILE_KIND_TEXT) {
				err_not_a_textfile(file);
			} else {
				kwd = DeeCompilerItem_VALUE(value, struct TPPKeyword);
				if likely(kwd) {
					file->f_textfile.f_newguard = kwd;
					result                      = 0;
				}
			}
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_includecount(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_TEXT) {
			err_not_a_textfile(file);
		} else {
			if (file->f_textfile.f_cacheentry)
				file = file->f_textfile.f_cacheentry;
			ASSERT(!file->f_textfile.f_cacheentry);
			result = DeeInt_NewSize(file->f_textfile.f_cacheinc);
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_readcount(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_TEXT) {
			err_not_a_textfile(file);
		} else {
			result = DeeInt_NewSize(file->f_textfile.f_rdata);
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_getdisallowguard(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_TEXT) {
			err_not_a_textfile(file);
		} else {
			result = DeeBool_For(file->f_textfile.f_flags & TPP_TEXTFILE_FLAG_NOGUARD);
			Dee_Incref(result);
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
file_deldisallowguard(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	int result = -1;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_TEXT) {
			err_not_a_textfile(file);
		} else {
			file->f_textfile.f_flags &= ~TPP_TEXTFILE_FLAG_NOGUARD;
			result = 0;
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
file_setdisallowguard(DeeCompilerItemObject *__restrict self,
                      DeeObject *__restrict value) {
	struct TPPFile *file;
	int result = -1;
	int newval;
	newval = DeeObject_Bool(value);
	if unlikely(newval < 0)
		goto done;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_TEXT) {
			err_not_a_textfile(file);
		} else {
			if (newval)
				file->f_textfile.f_flags |= TPP_TEXTFILE_FLAG_NOGUARD;
			else {
				file->f_textfile.f_flags &= ~TPP_TEXTFILE_FLAG_NOGUARD;
			}
			result = 0;
		}
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_getissystemheader(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_TEXT) {
			err_not_a_textfile(file);
		} else {
			result = DeeBool_For(file->f_textfile.f_flags & TPP_TEXTFILE_FLAG_SYSHEADER);
			Dee_Incref(result);
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
file_delissystemheader(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	int result = -1;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_TEXT) {
			err_not_a_textfile(file);
		} else {
			file->f_textfile.f_flags &= ~TPP_TEXTFILE_FLAG_SYSHEADER;
			result = 0;
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
file_setissystemheader(DeeCompilerItemObject *__restrict self,
                       DeeObject *__restrict value) {
	struct TPPFile *file;
	int result = -1;
	int newval;
	newval = DeeObject_Bool(value);
	if unlikely(newval < 0)
		goto done;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_TEXT) {
			err_not_a_textfile(file);
		} else {
			if (newval)
				file->f_textfile.f_flags |= TPP_TEXTFILE_FLAG_SYSHEADER;
			else {
				file->f_textfile.f_flags &= ~TPP_TEXTFILE_FLAG_SYSHEADER;
			}
			result = 0;
		}
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_getnonblocking(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_TEXT) {
			err_not_a_textfile(file);
		} else {
			result = DeeBool_For(file->f_textfile.f_flags & TPP_TEXTFILE_FLAG_NONBLOCK);
			Dee_Incref(result);
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
file_delnonblocking(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	int result = -1;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_TEXT) {
			err_not_a_textfile(file);
		} else {
			file->f_textfile.f_flags &= ~TPP_TEXTFILE_FLAG_NONBLOCK;
			result = 0;
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
file_setnonblocking(DeeCompilerItemObject *__restrict self,
                    DeeObject *__restrict value) {
	struct TPPFile *file;
	int result = -1;
	int newval;
	newval = DeeObject_Bool(value);
	if unlikely(newval < 0)
		goto done;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_TEXT) {
			err_not_a_textfile(file);
		} else {
			if (newval)
				file->f_textfile.f_flags |= TPP_TEXTFILE_FLAG_NONBLOCK;
			else {
				file->f_textfile.f_flags &= ~TPP_TEXTFILE_FLAG_NONBLOCK;
			}
			result = 0;
		}
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_isfunctionmacro(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		result = DeeBool_For(file->f_kind == TPPFILE_KIND_MACRO &&
		                     (file->f_macro.m_flags & TPP_MACROFILE_KIND_FUNCTION));
		Dee_Incref(result);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_iskeywordmacro(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		result = DeeBool_For(file->f_kind == TPPFILE_KIND_MACRO &&
		                     !(file->f_macro.m_flags & TPP_MACROFILE_KIND_FUNCTION));
		Dee_Incref(result);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_definitionsfile(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_MACRO) {
			err_not_a_macrofile(file);
		} else if (!file->f_macro.m_deffile) {
			result = Dee_None;
			Dee_Incref(Dee_None);
		} else {
			result = DeeCompiler_GetFile(file->f_macro.m_deffile);
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_definitionsposition(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_MACRO) {
			err_not_a_macrofile(file);
		} else if (!file->f_macro.m_deffile) {
			result = Dee_None;
			Dee_Incref(Dee_None);
		} else {
			result = DeeTuple_Newf("dd",
			                       (int)file->f_macro.m_defloc.lc_line,
			                       (int)file->f_macro.m_defloc.lc_col);
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_previousdefinition(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_MACRO) {
			err_not_a_macrofile(file);
		} else if (!file->f_macro.m_pushprev) {
			result = Dee_None;
			Dee_Incref(Dee_None);
		} else {
			result = DeeCompiler_GetFile(file->f_macro.m_pushprev);
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_pushcount(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_MACRO) {
			err_not_a_macrofile(file);
		} else {
			result = DeeInt_NewSize(file->f_macro.m_pushcount);
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_keywordexpandorigin(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_MACRO ||
		    (file->f_macro.m_flags & TPP_MACROFILE_KIND_FUNCTION)) {
			err_not_a_keywordmacrofile(file);
		} else {
			result = DeeCompiler_GetFile(file->f_macro.m_expand.e_expand_origin);
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_isvariadicmacro(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		result = DeeBool_For(file->f_kind == TPPFILE_KIND_MACRO &&
		                     (file->f_macro.m_flags & (TPP_MACROFILE_KIND_FUNCTION | TPP_MACROFILE_FLAG_FUNC_VARIADIC)) ==
		                     (TPP_MACROFILE_KIND_FUNCTION | TPP_MACROFILE_FLAG_FUNC_VARIADIC));
		Dee_Incref(result);
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_getallowselfexpansion(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_MACRO ||
		    !(file->f_macro.m_flags & TPP_MACROFILE_KIND_FUNCTION)) {
			err_not_a_functionmacrofile(file);
		} else {
			result = DeeBool_For(file->f_macro.m_flags & TPP_MACROFILE_FLAG_FUNC_SELFEXPAND);
			Dee_Incref(result);
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
file_delallowselfexpansion(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	int result = -1;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_MACRO ||
		    !(file->f_macro.m_flags & TPP_MACROFILE_KIND_FUNCTION)) {
			err_not_a_functionmacrofile(file);
		} else {
			file->f_macro.m_flags &= ~TPP_MACROFILE_FLAG_FUNC_SELFEXPAND;
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
file_setallowselfexpansion(DeeCompilerItemObject *__restrict self,
                           DeeObject *__restrict value) {
	struct TPPFile *file;
	int newval, result = -1;
	newval = DeeObject_Bool(value);
	if unlikely(newval < 0)
		goto done;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_MACRO ||
		    !(file->f_macro.m_flags & TPP_MACROFILE_KIND_FUNCTION)) {
			err_not_a_functionmacrofile(file);
		} else if (newval) {
			file->f_macro.m_flags |= TPP_MACROFILE_FLAG_FUNC_SELFEXPAND;
		} else {
			file->f_macro.m_flags &= ~TPP_MACROFILE_FLAG_FUNC_SELFEXPAND;
		}
	}
	COMPILER_END();
done:
	return result;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_getkeepargumentspace(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_MACRO ||
		    !(file->f_macro.m_flags & TPP_MACROFILE_KIND_FUNCTION)) {
			err_not_a_functionmacrofile(file);
		} else {
			result = DeeBool_For(file->f_macro.m_flags & TPP_MACROFILE_FLAG_FUNC_KEEPARGSPC);
			Dee_Incref(result);
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
file_delkeepargumentspace(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	int result = -1;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_MACRO ||
		    !(file->f_macro.m_flags & TPP_MACROFILE_KIND_FUNCTION)) {
			err_not_a_functionmacrofile(file);
		} else {
			file->f_macro.m_flags &= ~TPP_MACROFILE_FLAG_FUNC_KEEPARGSPC;
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
file_setkeepargumentspace(DeeCompilerItemObject *__restrict self,
                          DeeObject *__restrict value) {
	struct TPPFile *file;
	int newval, result = -1;
	newval = DeeObject_Bool(value);
	if unlikely(newval < 0)
		goto done;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_MACRO ||
		    !(file->f_macro.m_flags & TPP_MACROFILE_KIND_FUNCTION)) {
			err_not_a_functionmacrofile(file);
		} else if (newval) {
			file->f_macro.m_flags |= TPP_MACROFILE_FLAG_FUNC_KEEPARGSPC;
		} else {
			file->f_macro.m_flags &= ~TPP_MACROFILE_FLAG_FUNC_KEEPARGSPC;
		}
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_getfunctionmacrovariant(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	uint32_t variant;
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_MACRO ||
		    !(file->f_macro.m_flags & TPP_MACROFILE_KIND_FUNCTION)) {
			err_not_a_functionmacrofile(file);
		} else {
			variant = file->f_macro.m_flags & TPP_MACROFILE_MASK_FUNC_STARTCH;
			result  = DeeString_Chr((uint8_t)(variant == TPP_MACROFILE_FUNC_START_LANGLE ? '<' : variant == TPP_MACROFILE_FUNC_START_LBRACE ? '{' : variant == TPP_MACROFILE_FUNC_START_LBRACKET ? '[' : '('));
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
file_setfunctionmacrovariant(DeeCompilerItemObject *__restrict self,
                             DeeObject *__restrict value) {
	struct TPPFile *file;
	int result = -1;
	uint32_t new_variant;
	if (DeeObject_AssertTypeExact(value, &DeeString_Type))
		goto done;
	if (DeeString_WLEN(value) != 1) {
		err_expected_single_character_string(value);
		goto done;
	}
	switch (DeeString_GetChar(value, 0)) {

	case '(':
		new_variant = TPP_MACROFILE_FUNC_START_LPAREN;
		break;

	case '[':
		new_variant = TPP_MACROFILE_FUNC_START_LBRACKET;
		break;

	case '{':
		new_variant = TPP_MACROFILE_FUNC_START_LBRACE;
		break;

	case '<':
		new_variant = TPP_MACROFILE_FUNC_START_LANGLE;
		break;

	default:
		DeeError_Throwf(&DeeError_ValueError,
		                "Unrecognized function-like macro variant character %I32C",
		                DeeString_GetChar(value, 0));
		goto done;
	}

	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_MACRO ||
		    !(file->f_macro.m_flags & TPP_MACROFILE_KIND_FUNCTION)) {
			err_not_a_functionmacrofile(file);
		} else {
			file->f_macro.m_flags &= ~TPP_MACROFILE_MASK_FUNC_STARTCH;
			file->f_macro.m_flags |= new_variant;
		}
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_functionmacroargc(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_MACRO ||
		    !(file->f_macro.m_flags & TPP_MACROFILE_KIND_FUNCTION)) {
			err_not_a_functionmacrofile(file);
		} else {
			result = DeeInt_NewSize(file->f_macro.m_function.f_argc);
		}
	}
	COMPILER_END();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_functionmacroexpansions(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_MACRO ||
		    !(file->f_macro.m_flags & TPP_MACROFILE_KIND_FUNCTION)) {
			err_not_a_functionmacrofile(file);
		} else {
			result = DeeInt_NewSize(file->f_macro.m_function.f_expansions);
		}
	}
	COMPILER_END();
	return result;
}


PRIVATE struct type_getset file_getsets[] = {
	{ "istext",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_istext, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Returns ?t if @this File is a text-file (as opposed "
	      "to the result of macro expansion (s.a. ?#ismacro), or some other kind of file)") },
	{ "ismacro",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_ismacro, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Returns ?t if @this File is a macro-file (as opposed "
	      "to a text-file (s.a. ?#istext), or some other kind of file)") },
	{ "isexpanded",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_isexpand, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Returns ?t if @this File is an expanded function-like macro-file, "
	      "or the result of injecting custom text into the token stream\n"
	      "Note that functions and getsets related to ?#ismacro cannot be used "
	      "when ?#isexpanded is ?t, and that ?#ismacro will return ?f when "
	      "this field evaluates to ?t") },
	{ "origin",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_origin, NULL, NULL,
	  DOC("->?X2?.?N\n"
	      "Returns the originating location of @this File, "
	      "or ?N if @this File is the base-file") },
	{ "alltext",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_alltext, NULL, NULL,
	  DOC("->?Dstring\n"
	      "Returns the currently loaded text data of @this File") },
	{ "nexttext",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_nexttext, NULL, NULL,
	  DOC("->?Dstring\n"
	      "Returns sub-portion of text that will be used when reading from the file continues") },
	{ "position",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_position, NULL, NULL,
	  DOC("->?T2?Dint?Dint\n"
	      "Returns the current position (as a pair of integer `line, column', both of which "
	      "are zero-based, meaning you'll probably have to add ${+1} to get line numbers as "
	      "they would be used in a text editor)\n"
	      "In the event that @this File is a macro, the positions returned refer to the "
	      "macro declaration position") },
	{ DeeString_STR(&str_filename),
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_filename,
	  (int (DCALL *)(DeeObject *__restrict))&file_delfilename,
	  (int (DCALL *)(DeeObject *, DeeObject *))&file_setfilename,
	  DOC("->?Dstring\n"
	      "@throw ValueError Attempted to delete, or set the filename of a non-text file\n"
	      "Returns the filename of a text file, or the filename of the file containing the "
	      "definition of the @this macro\n"
	      "In the event that a ${##line} directive was used to override the filename, the "
	      "overwritten name is returned. If this isn't intended, use ?#realfilename instead\n"
	      "In text-files, this field may be written to override the current file name, while "
	      "deleting it will restore ?#realfilename as the used filename") },
	{ "realfilename",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_realfilename, NULL, NULL,
	  DOC("->?Dstring\n"
	      "Returns the real filename of a text file, or the real filename of the file "
	      "containing the definition of the @this macro\n"
	      "This is the original, real filename, whereas the name returned by ?#Filename "
	      "is the one which may have been overwritten by a ${##line} directive") },
	{ "name",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_name, NULL, NULL,
	  DOC("->?Dstring\n"
	      "Returns the name of @this File, that is the filename, or in the event of @this "
	      "file being a macro, the name of that macro") },
	{ "lineoffset",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_lineoffset,
	  (int (DCALL *)(DeeObject *__restrict))&file_dellineoffset,
	  (int (DCALL *)(DeeObject *, DeeObject *))&file_setlineoffset,
	  DOC("->?Dint\n"
	      "@throw ValueError @this File isn't a text file (?#istext is ?f)\n"
	      "Get, del(set to zero), or set the line-offset within @this "
	      "text file, as can also be set by the ?#line directive") },
	{ "stream",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_stream, NULL, NULL,
	  DOC("->?DFile\n"
	      "@throw ValueError @this File isn't a text file (?#istext is ?f)\n"
	      "Returns the file stream from which data is read into @this File") },
	{ "guard",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_getguard,
	  (int (DCALL *)(DeeObject *__restrict))&file_delguard,
	  (int (DCALL *)(DeeObject *, DeeObject *))&file_setguard,
	  DOC("->?X2?AKeyword?ALexer?Ert:Compiler?N\n"
	      "@throw ValueError @this File isn't a text file (?#istext is ?f)\n"
	      "Get, delete, or set a keyword that is checked for being defined "
	      "before allowing @this File to be included by ${##include} again\n"
	      "In the event of the keyword having an associated macro that is also "
	      "defined, the file will not be included, but simply be skipped.\n"
	      "Setting ?N is the same as deleting the guard, and ?N is returned if no guard is set") },
	{ "newguard",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_getnewguard,
	  (int (DCALL *)(DeeObject *__restrict))&file_delnewguard,
	  (int (DCALL *)(DeeObject *, DeeObject *))&file_setnewguard,
	  DOC("->?X2?AKeyword?ALexer?Ert:Compiler?N\n"
	      "@throw ValueError @this File isn't a text file (?#istext is ?f)\n"
	      "Get, delete, or set a keyword that will be set as ?#guard (if no guard has "
	      "already been set) once @this File is popped from the ${##include}-stack, and "
	      "?#disallowguard is ?f") },
	{ "includecount",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_includecount, NULL, NULL,
	  DOC("->?Dint\n"
	      "@throw ValueError @this File isn't a text file (?#istext is ?f)\n"
	      "Return the number of times that @this File exists within the ${##include}-stack") },
	{ "readcount",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_readcount, NULL, NULL,
	  DOC("->?Dint\n"
	      "@throw ValueError @this File isn't a text file (?#istext is ?f)\n"
	      "Returns the number of bytes already read from the underlying source stream") },
	{ "disallowguard",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_getdisallowguard,
	  (int (DCALL *)(DeeObject *__restrict))&file_deldisallowguard,
	  (int (DCALL *)(DeeObject *, DeeObject *))&file_setdisallowguard,
	  DOC("->?Dbool\n"
	      "@throw ValueError @this File isn't a text file (?#istext is ?f)\n"
	      "Get, del (set to false), or set @this File's disallow-guard property.\n"
	      "When set to ?f, ?#newguard will be applied as ?#guard when the file is "
	      "popped, allowing the lexer to remember a potential file guard.\n"
	      "This flag is set to ?t automatically when an outer-most ${##ifndef}-block "
	      "ends, following which more non-whitespace text is encountered, thus "
	      "preventing the creation of a guard for the file") },
	{ "issystemheader",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_getissystemheader,
	  (int (DCALL *)(DeeObject *__restrict))&file_delissystemheader,
	  (int (DCALL *)(DeeObject *, DeeObject *))&file_setissystemheader,
	  DOC("->?Dbool\n"
	      "@throw ValueError @this File isn't a text file (?#istext is ?f)\n"
	      "Get, del (set to false), or set if @this File is considered a system header\n"
	      "When ?t, all non-error warnings are suppressed\n"
	      "This flag is usually set by a ${##pragma GCC system_header} directive") },
	{ "nonblocking",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_getnonblocking,
	  (int (DCALL *)(DeeObject *__restrict))&file_delnonblocking,
	  (int (DCALL *)(DeeObject *, DeeObject *))&file_setnonblocking,
	  DOC("->?Dbool\n"
	      "@throw ValueError @this File isn't a text file (?#istext is ?f)\n"
	      "Get, del (set to false), or set if the underlying stream allows "
	      "for non-blocking I/O, and should be performed in non-blocking mode "
	      "when ?#nextchunk is called with $nonblocking set to ?t and when "
	      "no incomplete string, or comment exists at the end of currently "
	      "loaded text") },
	{ "isfunctionmacro",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_isfunctionmacro, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Returns ?t if @this File is a function-like macro-file\n"
	      "Note that in this case, ?#ismacro will also return ?t") },
	{ "iskeywordmacro",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_iskeywordmacro, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Returns ?t if @this File is a keyword-like macro-file\n"
	      "Note that in this case, ?#ismacro will also return ?t") },
	{ "definitionsfile",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_definitionsfile, NULL, NULL,
	  DOC("->?X2?.?N\n"
	      "@throw ValueError @this File isn't a macro file (?#ismacro is ?f)\n"
	      "Returns the file that was used to define @this macro, or return ?N if "
	      "the macro was defined through other means, such as via the commandline") },
	{ "definitionsposition",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_definitionsposition, NULL, NULL,
	  DOC("->?X2?T2?Dint?Dint?N\n"
	      "@throw ValueError @this File isn't a macro file (?#ismacro is ?f)\n"
	      "Return the (line, column) pair of the definition location of @this macro file\n"
	      "Macros not defined through files will return ${(0, 0)}") },
	{ "previousdefinition",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_previousdefinition, NULL, NULL,
	  DOC("->?X2?.?N\n"
	      "@throw ValueError @this File isn't a macro file (?#ismacro is ?f)\n"
	      "Return the previous definition of pushed macro (as created by ${##pragma push_macro(\"foo\")})\n"
	      "If the macro hasn't been pushed, or is the oldest variant, ?N is returned") },
	{ "pushcount",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_pushcount, NULL, NULL,
	  DOC("->?Dint\n"
	      "@throw ValueError @this File isn't a macro file (?#ismacro is ?f)\n"
	      "The amount of times ${##pragma push_macro(\"foo\")} was repeated without actually "
	      "providing a new definition of the macro. Used to handle recursive use of that pragma") },
	{ "keywordexpandorigin",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_keywordexpandorigin, NULL, NULL,
	  DOC("->?.\n"
	      "@throw ValueError @this File isn't a keyword-like macro file (?#iskeywordmacro is ?f)\n"
	      "The originating file of a keyword-like macro") },
	{ "isvariadicmacro",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_isvariadicmacro, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Returns ?t if @this File is a function-like macro-file (s.a. "
	      "?#isfunctionmacro) taking a variable number of arguments") },
	{ "allowselfexpansion",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_getallowselfexpansion,
	  (int (DCALL *)(DeeObject *__restrict))&file_delallowselfexpansion,
	  (int (DCALL *)(DeeObject *, DeeObject *))&file_setallowselfexpansion,
	  DOC("->?Dbool\n"
	      "@throw ValueError @this File isn't a function-like macro file (?#isfunctionmacro is ?f)\n"
	      "Get, del (set to ?f), or set if @this function-like macro is allowed to expand to itself\n"
	      "This flag is set for newly defined macros when the $\"macro-recursion\" extension is enabled, "
	      "and is cleared when that extension is disabled (default)\n"
	      "When set, the macro's body may contain another reference to the function itself, which is then "
	      "expanded again, so-long as the arguments passed differ from all expansions that are already "
	      "apart of the current macro-expansion (${##include}) stack.") },
	{ "keepargumentspace",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_getkeepargumentspace,
	  (int (DCALL *)(DeeObject *__restrict))&file_delkeepargumentspace,
	  (int (DCALL *)(DeeObject *, DeeObject *))&file_setkeepargumentspace,
	  DOC("->?Dbool\n"
	      "@throw ValueError @this File isn't a function-like macro file (?#isfunctionmacro is ?f)\n"
	      "Get, del (set to ?f), or set if whitespace surrounding arguments passed "
	      "to @this function-like macro should be kept, or trimmed.\n"
	      "This flag is set for newly defined macros when the $\"macro-argument-whitespace\" extension "
	      "is enabled, and is cleared when that extension is disabled (default)\n"
	      ">#pragma extension(\"-fno-macro-argument-whitespace\")\n"
	      ">#define STR1(x) #x\n"
	      ">#pragma extension(\"-fmacro-argument-whitespace\")\n"
	      ">#define STR2(x) #x\n"
	      ">print STR1(  foo  ); /* \"foo\" */\n"
	      ">print STR2(  foo  ); /* \"  foo  \" */") },
	{ "functionmacrovariant",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_getfunctionmacrovariant,
	  NULL,
	  (int (DCALL *)(DeeObject *, DeeObject *))&file_setfunctionmacrovariant,
	  DOC("->?Dstring\n"
	      "@throw ValueError @this File isn't a function-like macro file (?#isfunctionmacro is ?f)\n"
	      "@throw ValueError Attempted to set a value not apart of ${(\"(\", \"[\", \"{\", \"<\")}\n"
	      "Get or set the type of parenthesis used to start the argument list of @this function-like macro") },
	{ "functionmacroargc",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_functionmacroargc, NULL, NULL,
	  DOC("->?Dint\n"
	      "@throw ValueError @this File isn't a function-like macro file (?#isfunctionmacro is ?f)\n"
	      "Return the number of (non-variadic) argument taken by @this function-like macro") },
	{ "functionmacroexpansions",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_functionmacroexpansions, NULL, NULL,
	  DOC("->?Dint\n"
	      "@throw ValueError @this File isn't a function-like macro file (?#isfunctionmacro is ?f)\n"
	      "Return the number of times that @this function-like macro is being expanded\n"
	      "Note that normally only function-like macros with ?#allowselfexpansion set to ?t "
	      "can ever be expanded more than once at the same time") },
	{ NULL }
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_nextchunk(DeeCompilerItemObject *self, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
	bool extend = false, binary = false, nonblocking = false;
	unsigned int flags     = 0;
	DREF DeeObject *result = NULL;
	int error;
	struct TPPFile *file;
	PRIVATE struct keyword kwlist[] = { K(extend), K(binary), K(nonblocking), KEND };
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "|bbb", &extend, &binary, &nonblocking))
		goto done;
	if (extend)
		flags |= TPPFILE_NEXTCHUNK_FLAG_EXTEND;
	if (binary)
		flags |= TPPFILE_NEXTCHUNK_FLAG_BINARY;
	if (nonblocking)
		flags |= TPPFILE_NEXTCHUNK_FLAG_NOBLCK;
	COMPILER_BEGIN(self->ci_compiler);
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		error = TPPFile_NextChunk(file, flags);
		if (error >= 0) {
			result = DeeBool_For(error);
			Dee_Incref(result);
		}
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE struct type_method file_methods[] = {
	{ "nextchunk",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&file_nextchunk,
	  DOC("(extend=!f,binary=!f,nonblocking=!f)->?Dbool\n"
	      "@return Returns ?t if data was read, or ?f if the EOF of the input stream has been reached\n"
	      "Try to load the next, or @extend the current chunk of loaded data, by reading a "
	      "new chunk from ?#stream. When @binary is ?t, don't try to decode unicode data, "
	      "but read data as-is, without decoding it. When @nonblocking is ?t, and the "
	      "?#nonblocking is ?t as well, try to read data without blocking when waiting for "
	      "new data, thus potentially returning ?f, even when the actual end-of-file hasn't "
	      "been reached, yet"),
	  TYPE_METHOD_FKWDS },
	{ NULL }
};

INTERN DeeTypeObject DeeCompilerFile_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_File",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCompilerItem_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(DeeCompilerItemObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_str,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_str,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ file_methods,
	/* .tp_getsets       = */ file_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_INTERFACE_ILEXER_C */
