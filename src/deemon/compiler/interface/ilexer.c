/* Copyright (c) 2018-2026 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_COMPILER_INTERFACE_ILEXER_C
#define GUARD_DEEMON_COMPILER_INTERFACE_ILEXER_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/compiler/compiler.h>
#include <deemon/compiler/interface.h>
#include <deemon/compiler/lexer.h>
#include <deemon/compiler/tpp.h>
#include <deemon/error-rt.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/map.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/set.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/system-features.h> /* memcpyc(), ... */
#include <deemon/thread.h>
#include <deemon/tuple.h>

#include "../../runtime/kwlist.h"
#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint16_t */

DECL_BEGIN

#ifndef INT_MAX
#define INT_MAX __INT_MAX__
#endif /* !INT_MAX */

/* @return: TOK_ERR: An error occurred (and was thrown)
 * @return: -2:      A keyword wasn't found (and `create_missing' was false) */
INTERN WUNUSED NONNULL((1)) tok_t DCALL
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

	case '?':
		if (name[1] == '?')
			return TOK_QMARK_QMARK;
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
				return TOK_COLON_EQUAL;
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

INTERN WUNUSED NONNULL((1)) tok_t DCALL
get_token_from_obj(DeeObject *__restrict obj, bool create_missing) {
	unsigned int result;
	if (DeeString_Check(obj))
		return get_token_from_str(DeeString_STR(obj), create_missing);
	if (DeeObject_AsUInt(obj, &result))
		return TOK_ERR;
	if unlikely((tok_t)result < 0) {
		DeeRT_ErrIntegerOverflowS((tok_t)result, 0, INT_MAX);
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
	/* [TOK_COLON_EQUAL  - TOK_TWOCHAR_BEGIN] = */ { ':', '=' },
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
	/* [TOK_QMARK_QMARK   - TOK_TWOCHAR_BEGIN] = */ { '?', '?' },
};

STATIC_ASSERT(COMPILER_LENOF(largetok_names) ==
              (TOK_TWOCHAR_END - TOK_TWOCHAR_BEGIN));


/* @return: NULL:      An error occurred (and was thrown)
 * @return: ITER_DONE: The given `id' does not refer to a valid token id. */
INTERN WUNUSED DREF DeeObject *DCALL
get_token_name(tok_t id, struct TPPKeyword *kwd) {
	if ((unsigned int)id <= 255) {
		switch (id) {
		case TOK_EOF: return DeeString_NewEmpty();
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

/*[[[deemon
import define_Dee_HashStr, _Dee_HashSelect from rt.gen.hash;
print define_Dee_HashStr("");
print "#define Dee_HashStr_dot_0", _Dee_HashSelect(".0");
print "#define Dee_HashStr_slash_slash", _Dee_HashSelect("//");
]]]*/
#define Dee_HashStr__ _Dee_HashSelectC(0x0, 0x0)
#define Dee_HashStr_dot_0 _Dee_HashSelectC(0x240d5efb, 0xb57aef495f833b5c)
#define Dee_HashStr_slash_slash _Dee_HashSelectC(0x838f93fb, 0x32ed7842efd9cb44)
/*[[[end]]]*/

INTERN WUNUSED Dee_hash_t DCALL
get_token_namehash(tok_t id, struct TPPKeyword *kwd) {
	if ((unsigned int)id <= 255) {
		char name[2];
		switch (id) {
		case TOK_EOF: return Dee_HashStr__;
		case TOK_FLOAT: return Dee_HashStr_dot_0;
		case TOK_COMMENT: return Dee_HashStr_slash_slash;
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
		return (Dee_hash_t)-1;
	return Dee_HashUtf8(kwd->k_name, kwd->k_size);
}



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
keyword_str(DeeCompilerItemObject *__restrict self) {
	DREF DeeObject *result = NULL;
	struct TPPKeyword *item;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	item = DeeCompilerItem_VALUE(self, struct TPPKeyword);
	if likely(item)
		result = DeeString_NewUtf8(item->k_name, item->k_size, STRING_ERROR_FIGNORE);
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
keyword_print(DeeCompilerItemObject *__restrict self,
              Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t result = -1;
	struct TPPKeyword *item;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	item = DeeCompilerItem_VALUE(self, struct TPPKeyword);
	if likely(item)
		result = DeeFormat_Print(printer, arg, item->k_name, item->k_size);
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
keyword_hash(DeeCompilerItemObject *__restrict self) {
	DREF DeeObject *result = NULL;
	struct TPPKeyword *item;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	item = DeeCompilerItem_VALUE(self, struct TPPKeyword);
	if likely(item)
		result = DeeInt_NewSize(item->k_hash);
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
keyword_macrofile(DeeCompilerItemObject *__restrict self) {
	DREF DeeObject *result = NULL;
	struct TPPKeyword *item;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	item = DeeCompilerItem_VALUE(self, struct TPPKeyword);
	if likely(item) {
		if (!item->k_macro) {
			result = DeeNone_NewRef();
		} else {
			result = DeeCompiler_GetFile(item->k_macro);
		}
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
keyword_oldmacrofile(DeeCompilerItemObject *__restrict self) {
	DREF DeeObject *result = NULL;
	struct TPPKeyword *item;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	item = DeeCompilerItem_VALUE(self, struct TPPKeyword);
	if likely(item) {
		if (!item->k_rare || !item->k_rare->kr_oldmacro) {
			result = DeeNone_NewRef();
		} else {
			result = DeeCompiler_GetFile(item->k_rare->kr_oldmacro);
		}
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
keyword_defmacrofile(DeeCompilerItemObject *__restrict self) {
	DREF DeeObject *result = NULL;
	struct TPPKeyword *item;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	item = DeeCompilerItem_VALUE(self, struct TPPKeyword);
	if likely(item) {
		if (!item->k_rare || !item->k_rare->kr_defmacro) {
			result = DeeNone_NewRef();
		} else {
			result = DeeCompiler_GetFile(item->k_rare->kr_defmacro);
		}
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
keyword_cachedfile(DeeCompilerItemObject *__restrict self) {
	DREF DeeObject *result = NULL;
	struct TPPKeyword *item;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	item = DeeCompilerItem_VALUE(self, struct TPPKeyword);
	if likely(item) {
		if (!item->k_rare || !item->k_rare->kr_file) {
			result = DeeNone_NewRef();
		} else {
			result = DeeCompiler_GetFile(item->k_rare->kr_file);
		}
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
keyword_id(DeeCompilerItemObject *__restrict self) {
	DREF DeeObject *result = NULL;
	struct TPPKeyword *item;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	item = DeeCompilerItem_VALUE(self, struct TPPKeyword);
	if likely(item)
		result = DeeInt_NewUInt(item->k_id);
	COMPILER_END();
done:
	return result;
}

#define DEFINE_KEYWORD_FLAG_FUNCTIONS(name, flag)                                                                     \
	PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL                                                                \
	keyword_get_##name(DeeCompilerItemObject *__restrict self) {                                                      \
		DREF DeeObject *result = NULL;                                                                                \
		struct TPPKeyword *item;                                                                                      \
		if (COMPILER_BEGIN(self->ci_compiler))                                                                        \
			goto done;                                                                                                \
		item = DeeCompilerItem_VALUE(self, struct TPPKeyword);                                                        \
		if likely(item) {                                                                                             \
			uint32_t flags = TPPKeyword_GetFlags(item, 0);                                                            \
			result         = DeeBool_For(flags & flag);                                                               \
			Dee_Incref(result);                                                                                       \
		}                                                                                                             \
		COMPILER_END();                                                                                               \
	done:                                                                                                             \
		return result;                                                                                                \
	}                                                                                                                 \
	PRIVATE WUNUSED NONNULL((1)) int DCALL                                                                            \
	keyword_del_##name(DeeCompilerItemObject *__restrict self) {                                                      \
		int result = -1;                                                                                              \
		struct TPPKeyword *item;                                                                                      \
		if (COMPILER_BEGIN(self->ci_compiler))                                                                        \
			goto done;                                                                                                \
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
	done:                                                                                                             \
		return result;                                                                                                \
	}                                                                                                                 \
	PRIVATE WUNUSED NONNULL((1)) int DCALL                                                                            \
	keyword_set_##name(DeeCompilerItemObject *__restrict self,                                                        \
	                   DeeObject *__restrict value) {                                                                 \
		int newval, result = -1;                                                                                      \
		struct TPPKeyword *item;                                                                                      \
		newval = DeeObject_Bool(value);                                                                               \
		if unlikely(newval < 0)                                                                                       \
			goto done;                                                                                                \
		if (!newval)                                                                                                  \
			return keyword_del_##name(self);                                                                          \
		if (COMPILER_BEGIN(self->ci_compiler))                                                                        \
			goto done;                                                                                                \
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
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	item = DeeCompilerItem_VALUE(self, struct TPPKeyword);
	if likely(item) {
		if (item->k_rare) {
			result = DeeInt_NewInt64(item->k_rare->kr_counter);
		} else {
			result = DeeInt_NewZero();
		}
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
keyword_del_counter(DeeCompilerItemObject *__restrict self) {
	int result = -1;
	struct TPPKeyword *item;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	item = DeeCompilerItem_VALUE(self, struct TPPKeyword);
	if likely(item) {
		if (item->k_rare)
			item->k_rare->kr_counter = 0;
		result = 0;
	}
	COMPILER_END();
done:
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
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
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
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	item = DeeCompilerItem_VALUE(self, struct TPPKeyword);
	if likely(item) {
		if (item->k_rare) {
			result = DeeInt_NewUIntptr((uintptr_t)item->k_rare->kr_user);
		} else {
			result = DeeInt_NewZero();
		}
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
keyword_del_uservalue(DeeCompilerItemObject *__restrict self) {
	int result = -1;
	struct TPPKeyword *item;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	item = DeeCompilerItem_VALUE(self, struct TPPKeyword);
	if likely(item) {
		if (item->k_rare)
			item->k_rare->kr_user = (void *)(uintptr_t)0;
		result = 0;
	}
	COMPILER_END();
done:
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
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
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


PRIVATE struct type_getset tpconst keyword_getsets[] = {
	TYPE_GETTER("id", &keyword_id,
	            "->?Dint\n"
	            "Returns the ID of @this keyword"),
	TYPE_GETTER("name", &keyword_str,
	            "->?Dstring\n"
	            "Returns the name of @this keyword. Same as ?#{op:str}"),
	TYPE_GETTER(STR_hash, &keyword_hash,
	            "->?Dint\n"
	            "Returns the hash of @this keyword"),
	TYPE_GETTER("macrofile", &keyword_macrofile,
	            "->?X2" DR_CFile "?N\n"
	            "Returns the macro definitions file, or ?N if "
	            "@this keyword isn't being used as a user-defined macro"),
	TYPE_GETTER("oldmacrofile", &keyword_oldmacrofile,
	            "->?X2" DR_CFile "?N\n"
	            "Returns the latest old macro file, that is the first macro "
	            "file definition that is preserved when ${##pragma push_macro(\"foo\")} is used"),
	TYPE_GETTER("defmacrofile", &keyword_defmacrofile,
	            "->?X2" DR_CFile "?N\n"
	            "Returns the default definition of a user-overwritten macro"),
	TYPE_GETTER("cachedfile", &keyword_cachedfile,
	            "->?X2" DR_CFile "?N\n"
	            "Returns a file that has been cached under this keyword"),
	TYPE_GETSET("counter",
	            &keyword_get_counter,
	            &keyword_del_counter,
	            &keyword_set_counter,
	            "->?Dint\n"
	            "Get, del (set to $0), or set counter that is used to implement the builtin "
	            "${__TPP_COUNTER} macro, which can be used to generate unique numbers, based "
	            "on keywords, which are used as keys to access those numbers.\n"
	            "When this field is set, the written value is what ${__TPP_COUNTER} will expand "
	            "to, the next time it is invoked with @this keyword.\n"
	            "Note however, that reading this field will _not_ modify the counter, unlike "
	            "accessing it though ${__TPP_COUNTER}, which will increment it once every time"),
	TYPE_GETSET("hasattribute",
	            &keyword_get_hasattribute,
	            &keyword_del_hasattribute,
	            &keyword_set_hasattribute,
	            "->?Dbool\n"
	            "Get, del (set to ?f), or set the has-attribute flag of @this keyword\n"
	            "The has-attribute flag can then be queried via the ${__has_attribute()} builtin macro"),
	TYPE_GETSET("hasbuiltin",
	            &keyword_get_hasbuiltin,
	            &keyword_del_hasbuiltin,
	            &keyword_set_hasbuiltin,
	            "->?Dbool\n"
	            "Get, del (set to ?f), or set the has-builtin flag of @this keyword\n"
	            "The has-builtin flag can then be queried via the ${__has_builtin()} builtin macro"),
	TYPE_GETSET("hascppattribute",
	            &keyword_get_hascppattribute,
	            &keyword_del_hascppattribute,
	            &keyword_set_hascppattribute,
	            "->?Dbool\n"
	            "Get, del (set to ?f), or set the has-cpp_attribute flag of @this keyword\n"
	            "The has-cpp_attribute flag can then be queried via the ${__has_cpp_attribute()} builtin macro"),
	TYPE_GETSET("hasdeclspecattribute",
	            &keyword_get_hasdeclspecattribute,
	            &keyword_del_hasdeclspecattribute,
	            &keyword_set_hasdeclspecattribute,
	            "->?Dbool\n"
	            "Get, del (set to ?f), or set the has-declspec_attribute flag of @this keyword\n"
	            "The has-declspec_attribute flag can then be queried via the ${__has_declspec_attribute()} builtin macro"),
	TYPE_GETSET("hasextension",
	            &keyword_get_hasextension,
	            &keyword_del_hasextension,
	            &keyword_set_hasextension,
	            "->?Dbool\n"
	            "Get, del (set to ?f), or set the has-extension flag of @this keyword\n"
	            "The has-extension flag can then be queried via the ${__has_extension()} builtin macro"),
	TYPE_GETSET("hasfeature",
	            &keyword_get_hasfeature,
	            &keyword_del_hasfeature,
	            &keyword_set_hasfeature,
	            "->?Dbool\n"
	            "Get, del (set to ?f), or set the has-feature flag of @this keyword\n"
	            "The has-feature flag can then be queried via the ${__has_feature()} builtin macro"),
	TYPE_GETSET("isdeprecated",
	            &keyword_get_isdeprecated,
	            &keyword_del_isdeprecated,
	            &keyword_set_isdeprecated,
	            "->?Dbool\n"
	            "Get, del (set to ?f), or set the is-deprecated flag of @this keyword\n"
	            "The is-deprecated flag can then be queried via the ${__is_deprecated()} builtin macro"),
	TYPE_GETSET("ispoisoned",
	            &keyword_get_ispoisoned,
	            &keyword_del_ispoisoned,
	            &keyword_set_ispoisoned,
	            "->?Dbool\n"
	            "Get, del (set to ?f), or set the is-poisoned flag of @this keyword\n"
	            "The is-poisoned flag can then be queried via the ${__is_poisoned()} builtin macro"),
	TYPE_GETSET("hastppbuiltin",
	            &keyword_get_hastppbuiltin,
	            &keyword_del_hastppbuiltin,
	            &keyword_set_hastppbuiltin,
	            "->?Dbool\n"
	            "Get, del (set to ?f), or set the has-tpp_builtin flag of @this keyword\n"
	            "The has-tpp_builtin flag can then be queried via the ${__has_tpp_builtin()} builtin macro"),
	TYPE_GETSET("uservalue",
	            &keyword_get_uservalue,
	            &keyword_del_uservalue,
	            &keyword_set_uservalue,
	            "->?Dint\n"
	            "#tIntegerOverflow{Attempted to write a negative value, or one that is too large}"
	            "Get, del (set to $0), or set a custom user-value which can be stored within "
	            /**/ "keyword descriptors. This value must be an unsigned integer that fits into "
	            /**/ "a single pointer, as used by the host"),
	/* TODO: Access to keyword assertions (`kr_asserts')? */
	TYPE_GETSET_END
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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeCompilerItemObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&keyword_str,
		/* .tp_repr  = */ NULL,
		/* .tp_bool  = */ NULL,
		/* .tp_print = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&keyword_print
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
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


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_keywords(DeeCompilerWrapperObject *__restrict self) {
	return DeeCompiler_GetLexerKeywords(self->cw_compiler);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_extensions(DeeCompilerWrapperObject *__restrict self) {
	return DeeCompiler_GetLexerExtensions(self->cw_compiler);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_warnings(DeeCompilerWrapperObject *__restrict self) {
	return DeeCompiler_GetLexerWarnings(self->cw_compiler);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_syspaths(DeeCompilerWrapperObject *__restrict self) {
	return DeeCompiler_GetLexerSyspaths(self->cw_compiler);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_ifdef(DeeCompilerWrapperObject *__restrict self) {
	return DeeCompiler_GetLexerIfdef(self->cw_compiler);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_token(DeeCompilerWrapperObject *__restrict self) {
	return DeeCompiler_GetLexerToken(self->cw_compiler);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_file(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	if (TPPLexer_Current->l_token.t_file == &TPPFile_Empty) {
		result = DeeNone_NewRef();
	} else {
		result = DeeCompiler_GetFile(TPPLexer_Current->l_token.t_file);
	}
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_textfile(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	struct TPPFile *file;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	file = TPPLexer_Textfile();
	if (file == &TPPFile_Empty) {
		result = DeeNone_NewRef();
	} else {
		result = DeeCompiler_GetFile(file);
	}
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_basefile(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	struct TPPFile *file;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	file = TPPLexer_Basefile();
	if (file == &TPPFile_Empty) {
		result = DeeNone_NewRef();
	} else {
		result = DeeCompiler_GetFile(file);
	}
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_textposition(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	DREF DeeObject *file_ob;
	struct TPPFile *file;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	file = TPPLexer_Current->l_token.t_file;
	if (file == &TPPFile_Empty) {
is_empty_file:
		result = DeeNone_NewRef();
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
			result = DeeTuple_Newf("Odd", file_ob, lc.lc_line, lc.lc_col);
		}
	}
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_textendposition(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	DREF DeeObject *file_ob;
	struct TPPFile *file;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	file = TPPLexer_Current->l_token.t_file;
	if (file == &TPPFile_Empty) {
is_empty_file:
		result = DeeNone_NewRef();
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
			result = DeeTuple_Newf("Odd", file_ob, lc.lc_line, lc.lc_col);
		}
	}
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_tokenposition(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	DREF DeeObject *file_ob;
	struct TPPFile *file;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	file = TPPLexer_Current->l_token.t_file;
	if (file == &TPPFile_Empty) {
		result = DeeNone_NewRef();
	} else {
		struct TPPLCInfo lc;
		TPPFile_LCAt(file, &lc, TPPLexer_Current->l_token.t_begin);
		file_ob = DeeCompiler_GetFile(file);
		if unlikely(!file_ob) {
			result = NULL;
		} else {
			result = DeeTuple_Newf("Odd", file_ob, lc.lc_line, lc.lc_col);
		}
	}
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_tokenendposition(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	DREF DeeObject *file_ob;
	struct TPPFile *file;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	file = TPPLexer_Current->l_token.t_file;
	if (file == &TPPFile_Empty) {
		result = DeeNone_NewRef();
	} else {
		struct TPPLCInfo lc;
		TPPFile_LCAt(file, &lc, TPPLexer_Current->l_token.t_end);
		file_ob = DeeCompiler_GetFile(file);
		if unlikely(!file_ob) {
			result = NULL;
		} else {
			result = DeeTuple_Newf("Odd", file_ob, lc.lc_line, lc.lc_col);
		}
	}
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_atstartofline(DeeCompilerWrapperObject *__restrict self) {
	int result;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	result = TPPLexer_AtStartOfLine();
	COMPILER_END();
	return_bool(result);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_flags(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	result = DeeInt_NewUInt32(TPPLexer_Current->l_flags & ~TPPLEXER_FLAG_MERGEMASK);
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
lexer_set_flags(DeeCompilerWrapperObject *__restrict self,
                DeeObject *__restrict value) {
	uint32_t new_flags;
	if (DeeObject_AsUInt32(value, &new_flags))
		goto err;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	TPPLexer_Current->l_flags &= TPPLEXER_FLAG_MERGEMASK;
	TPPLexer_Current->l_flags |= new_flags & ~TPPLEXER_FLAG_MERGEMASK;
	COMPILER_END();
	return 0;
err:
	return -1;
}

#define DEFINE_LEXER_FLAG_FUNCTIONS(name, flag)                        \
	PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL          \
	lexer_get_##name(DeeCompilerWrapperObject *__restrict self) {      \
		DREF DeeObject *result;                                        \
		if (COMPILER_BEGIN(self->cw_compiler))                         \
			goto err;                                                  \
		result = DeeBool_For((TPPLexer_Current->l_flags & flag) != 0); \
		Dee_Incref(result);                                            \
		COMPILER_END();                                                \
		return result;                                                 \
	err:                                                               \
		return NULL;                                                   \
	}                                                                  \
	PRIVATE WUNUSED NONNULL((1)) int DCALL                      \
	lexer_del_##name(DeeCompilerWrapperObject *__restrict self) {      \
		if (COMPILER_BEGIN(self->cw_compiler))                         \
			goto err;                                                  \
		TPPLexer_Current->l_flags &= ~flag;                            \
		COMPILER_END();                                                \
		return 0;                                                      \
	err:                                                               \
		return -1;                                                     \
	}                                                                  \
	PRIVATE WUNUSED NONNULL((1, 2)) int DCALL                           \
	lexer_set_##name(DeeCompilerWrapperObject *self,                   \
	                 DeeObject *value) {                               \
		int newval = DeeObject_Bool(value);                            \
		if unlikely(newval < 0)                                        \
			goto err;                                                  \
		if (COMPILER_BEGIN(self->cw_compiler))                         \
			goto err;                                                  \
		if (newval) {                                                  \
			TPPLexer_Current->l_flags |= flag;                         \
		} else {                                                       \
			TPPLexer_Current->l_flags &= ~flag;                        \
		}                                                              \
		COMPILER_END();                                                \
		return 0;                                                      \
	err:                                                               \
		return -1;                                                     \
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_eofparen(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	result = DeeInt_NewSize(TPPLexer_Current->l_eof_paren);
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
lexer_del_eofparen(DeeCompilerWrapperObject *__restrict self) {
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	TPPLexer_Current->l_eof_paren = 0;
	COMPILER_END();
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
lexer_set_eofparen(DeeCompilerWrapperObject *__restrict self,
                   DeeObject *__restrict value) {
	size_t new_eofparen;
	if (DeeObject_AsSize(value, &new_eofparen))
		goto err;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	TPPLexer_Current->l_eof_paren = new_eofparen;
	COMPILER_END();
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_eobfile(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	result = DeeCompiler_GetFile(TPPLexer_Current->l_eob_file);
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
lexer_del_eobfile(DeeCompilerWrapperObject *__restrict self) {
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	TPPLexer_Current->l_eob_file = NULL;
	COMPILER_END();
	return 0;
err:
	return -1;
}

INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_invalid_file_compiler)(DeeCompilerItemObject *__restrict obj) {
	(void)obj;
	return DeeError_Throwf(&DeeError_ValueError,
	                       "File is associated with a different compiler");
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
lexer_set_eobfile(DeeCompilerWrapperObject *__restrict self,
                  DeeObject *__restrict value) {
	struct TPPFile *file;
	int result;
	if (DeeNone_Check(value))
		return lexer_del_eobfile(self);
	result = -1;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto done;
	if (!DeeObject_InstanceOf(value, &DeeCompilerFile_Type)) {
		DeeObject_TypeAssertFailed(value, &DeeCompilerFile_Type);
	} else if (((DeeCompilerItemObject *)value)->ci_compiler != self->cw_compiler) {
		err_invalid_file_compiler((DeeCompilerItemObject *)value);
	} else {
		file = DeeCompilerItem_VALUE(value, struct TPPFile);
		if likely(file) {
			TPPLexer_Current->l_eob_file = file;
			result = 0;
		}
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_eoffile(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	result = DeeCompiler_GetFile(TPPLexer_Current->l_eof_file);
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
lexer_del_eoffile(DeeCompilerWrapperObject *__restrict self) {
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	TPPLexer_Current->l_eof_file = NULL;
	COMPILER_END();
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
lexer_set_eoffile(DeeCompilerWrapperObject *__restrict self,
                  DeeObject *__restrict value) {
	struct TPPFile *file;
	int result;
	if (DeeNone_Check(value))
		return lexer_del_eoffile(self);
	result = -1;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto done;
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
done:
	return result;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_macrolimit(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	result = DeeInt_NewSize(TPPLexer_Current->l_limit_mrec);
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
lexer_del_macrolimit(DeeCompilerWrapperObject *__restrict self) {
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	TPPLexer_Current->l_limit_mrec = TPPLEXER_DEFAULT_LIMIT_MREC;
	COMPILER_END();
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
lexer_set_macrolimit(DeeCompilerWrapperObject *__restrict self,
                     DeeObject *__restrict value) {
	size_t new_macrolimit;
	if (DeeObject_AsSize(value, &new_macrolimit))
		goto err;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	TPPLexer_Current->l_limit_mrec = new_macrolimit;
	COMPILER_END();
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_includelimit(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	result = DeeInt_NewSize(TPPLexer_Current->l_limit_incl);
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
lexer_del_includelimit(DeeCompilerWrapperObject *__restrict self) {
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	TPPLexer_Current->l_limit_incl = TPPLEXER_DEFAULT_LIMIT_INCL;
	COMPILER_END();
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
lexer_set_includelimit(DeeCompilerWrapperObject *__restrict self,
                       DeeObject *__restrict value) {
	size_t new_includelimit;
	if (DeeObject_AsSize(value, &new_includelimit))
		goto err;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	TPPLexer_Current->l_limit_incl = new_includelimit;
	COMPILER_END();
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_warningcount(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	result = DeeInt_NewSize(TPPLexer_Current->l_warncount);
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
lexer_del_warningcount(DeeCompilerWrapperObject *__restrict self) {
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	TPPLexer_Current->l_warncount = 0;
	COMPILER_END();
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
lexer_set_warningcount(DeeCompilerWrapperObject *__restrict self,
                       DeeObject *__restrict value) {
	size_t new_warningcount;
	if (DeeObject_AsSize(value, &new_warningcount))
		goto err;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	TPPLexer_Current->l_warncount = new_warningcount;
	COMPILER_END();
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_errorcount(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	result = DeeInt_NewSize(TPPLexer_Current->l_errorcount);
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
lexer_del_errorcount(DeeCompilerWrapperObject *__restrict self) {
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	TPPLexer_Current->l_errorcount = 0;
	COMPILER_END();
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
lexer_set_errorcount(DeeCompilerWrapperObject *__restrict self,
                     DeeObject *__restrict value) {
	size_t new_errorcount;
	if (DeeObject_AsSize(value, &new_errorcount))
		goto err;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	TPPLexer_Current->l_errorcount = new_errorcount;
	COMPILER_END();
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_maxerrors(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	result = DeeInt_NewSize(TPPLexer_Current->l_maxerrors);
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
lexer_del_maxerrors(DeeCompilerWrapperObject *__restrict self) {
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	TPPLexer_Current->l_maxerrors = TPPLEXER_DEFAULT_LIMIT_ECNT;
	COMPILER_END();
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
lexer_set_maxerrors(DeeCompilerWrapperObject *__restrict self,
                    DeeObject *__restrict value) {
	size_t new_maxerrors;
	if (DeeObject_AsSize(value, &new_maxerrors))
		goto err;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	TPPLexer_Current->l_maxerrors = new_maxerrors;
	COMPILER_END();
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_tabsize(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	result = DeeInt_NewSize(TPPLexer_Current->l_tabsize);
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
lexer_del_tabsize(DeeCompilerWrapperObject *__restrict self) {
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	TPPLexer_Current->l_tabsize = TPPLEXER_DEFAULT_TABSIZE;
	COMPILER_END();
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
lexer_set_tabsize(DeeCompilerWrapperObject *__restrict self,
                  DeeObject *__restrict value) {
	size_t new_tabsize;
	if (DeeObject_AsSize(value, &new_tabsize))
		goto err;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	TPPLexer_Current->l_tabsize = new_tabsize;
	COMPILER_END();
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_get_counter(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	result = DeeInt_NewInt64(TPPLexer_Current->l_counter);
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
lexer_del_counter(DeeCompilerWrapperObject *__restrict self) {
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	TPPLexer_Current->l_counter = TPPLEXER_DEFAULT_TABSIZE;
	COMPILER_END();
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
lexer_set_counter(DeeCompilerWrapperObject *__restrict self,
                  DeeObject *__restrict value) {
	int64_t new_counter;
	if (DeeObject_AsInt64(value, &new_counter))
		goto err;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	TPPLexer_Current->l_counter = new_counter;
	COMPILER_END();
	return 0;
err:
	return -1;
}


PRIVATE struct type_getset tpconst lexer_getsets[] = {
	TYPE_GETTER("keywords", &lexer_get_keywords,
	            "->?#Keywords\n"
	            "Returns a descriptor for recognized keywords, as well as their associated bindings"),
	TYPE_GETTER("extensions", &lexer_get_extensions,
	            "->?#Extensions\n"
	            "Returns a descriptor for currently enabled compiler extensions"),
	TYPE_GETTER("warnings", &lexer_get_warnings,
	            "->?#Warnings\n"
	            "Returns a descriptor for the current compiler warning state"),
	TYPE_GETTER("syspaths", &lexer_get_syspaths,
	            "->?#SysPaths\n"
	            "Returns a sequence representing the system include paths currently being used"),
	TYPE_GETTER("ifdef", &lexer_get_ifdef,
	            "->?#Ifdef\n"
	            "Returns a descriptor for the active list of ifdef-blocks"),
	TYPE_GETTER("token", &lexer_get_token,
	            "->?#Token\n"
	            "Returns a descriptor for the currently active token"),
	TYPE_GETTER("file", &lexer_get_file,
	            "->?X2?#File?N\n"
	            "Returns the currently active file, or ?N if no file is currently active. Same as ?Aid?#Token"),
	TYPE_GETTER("textfile", &lexer_get_textfile,
	            "->?X2?#File?N\n"
	            "Same as ?#File, but return the first non-macro file"),
	TYPE_GETTER("basefile", &lexer_get_basefile,
	            "->?X2?#File?N\n"
	            "Similar to ?#File, but return the base-file (that is the first included file) instead"),
	TYPE_GETTER("textposition", &lexer_get_textposition,
	            "->?X2?T3?#File?Dint?Dint?N\n"
	            "Returns a tuple (file, line, column) for the text-position of the current token\n"
	            "In the event that the current file is the result of an expanded macro, the source "
	            /**/ "location of the macro invocation site is returned\n"
	            "In the event that no text file is currently loaded, ?N is returned instead"),
	TYPE_GETTER("textendposition", &lexer_get_textendposition,
	            "->?X2?T3?#File?Dint?Dint?N\n"
	            "Same as ?#textposition, however when the current file isn't the result of an expanded macro, "
	            /**/ "the returned values refer to the end of the current token, rather than its beginning\n"
	            "In the event that no text file is currently loaded, ?N is returned instead"),
	TYPE_GETTER("tokenposition", &lexer_get_tokenposition,
	            "->?X2?T3?#File?Dint?Dint?N\n"
	            "Similar to ?#textposition, however in the event of the current token originating "
	            /**/ "from a macro, return the source position of that token within the macro, rather "
	            /**/ "than the source position of the macro being invoked\n"
	            "Same as ?Aposition?#Token"),
	TYPE_GETTER("tokenendposition", &lexer_get_tokenendposition,
	            "->?X2?T3?#File?Dint?Dint?N\n"
	            "Same as ?#Tokenposition, however return the end position of the current token\n"
	            "Same as ?Aendposition?#Token"),
	TYPE_GETTER("atstartofline", &lexer_get_atstartofline,
	            "->?Dbool\n"
	            "Returns ?t if the current token is located at the "
	            /**/ "start of a line, optionally prefixed by whitespace"),
	TYPE_GETSET("flags", &lexer_get_flags, NULL, &lexer_set_flags,
	            "->?Dint\n"
	            "Get or set the current general purpose lexer configuration as a whole\n"
	            "The individual bits in the returned integer are prone to getting changed, "
	            /**/ "and it is therefor recommended to set the lexer configuration using the "
	            /**/ "individual properties instead"),
	TYPE_GETSET("wantcomments",
	            &lexer_get_wantcomments,
	            &lexer_del_wantcomments,
	            &lexer_set_wantcomments,
	            "->?Dbool\n"
	            "Configure if comment tokens should, or shouldn't be emit\n"
	            "Note: This field is preserved by ?#flags"),
	TYPE_GETSET("wantspace",
	            &lexer_get_wantspace,
	            &lexer_del_wantspace,
	            &lexer_set_wantspace,
	            "->?Dbool\n"
	            "Configure if space-tokens should, or shouldn't be emit\n"
	            "Note: This field is preserved by ?#flags"),
	TYPE_GETSET("wantlf",
	            &lexer_get_wantlf,
	            &lexer_del_wantlf,
	            &lexer_set_wantlf,
	            "->?Dbool\n"
	            "Configure if line-feed-tokens should, or shouldn't be emit\n"
	            "Note: This field is preserved by ?#flags"),
	TYPE_GETSET("noseek_on_eob",
	            &lexer_get_noseek_on_eob,
	            &lexer_del_noseek_on_eob,
	            &lexer_set_noseek_on_eob,
	            "->?Dbool\n"
	            "When ?t, don't seek the next chunk (s.a. ?Anextchunk?#File) "
	            /**/ "when the current one ends. Instead, indicate EOF by setting the "
	            /**/ "current ?Aid?#Token to $0\n"
	            "Note: This field is preserved by ?#flags"),
	TYPE_GETSET("nopop_on_eof",
	            &lexer_get_nopop_on_eof,
	            &lexer_del_nopop_on_eof,
	            &lexer_set_nopop_on_eof,
	            "->?Dbool\n"
	            "When ?t, don't automatically pop the current ?#File when it signals "
	            /**/ "eof in order to continue parsing older files, but instead propagate the "
	            /**/ "EOF by setting the current ?Aid?#Token to $0\n"
	            "Note: This field is preserved by ?#flags"),
	TYPE_GETSET("keepmacrospace",
	            &lexer_get_keepmacrospace,
	            &lexer_del_keepmacrospace,
	            &lexer_set_keepmacrospace,
	            "->?Dbool\n"
	            "When ?t, don't strip whitespace surrounding the text of macros, but "
	            /**/ "keep that whitespace as part of the macro's definition, re-propagating "
	            /**/ "it every time that macro is expanded\n"
	            "Note: This field is preserved by ?#flags"),
	TYPE_GETSET("nonblocking",
	            &lexer_get_nonblocking,
	            &lexer_del_nonblocking,
	            &lexer_set_nonblocking,
	            "->?Dbool\n"
	            "When ?t, operate in non-blocking mode when loading new chunks from "
	            /**/ "files, which essentically means that whenever ?Anextchunk?#File is called, "
	            /**/ "the $nonblocking argument is set to this value\n"
	            "Note: This field is preserved by ?#flags"),
	TYPE_GETSET("terminatestringlf",
	            &lexer_get_terminatestringlf,
	            &lexer_del_terminatestringlf,
	            &lexer_set_terminatestringlf,
	            "->?Dbool\n"
	            "When ?t, regular strings are terminated by line-feeds, which will also "
	            /**/ "cause a warning/error to be emit, alongside the incomplete token still "
	            /**/ "packaged as a complete string\n"
	            "Note that this also affects ?Anextchunk?#File, in that incomplete strings "
	            /**/ "near the end of the input stream can also be terminted by line-feeds\n"
	            "Note: This field is preserved by ?#flags"),
	TYPE_GETSET("nodirectives",
	            &lexer_get_nodirectives,
	            &lexer_del_nodirectives,
	            &lexer_set_nodirectives,
	            "->?Dbool\n"
	            "When ?t, don't process preprocessor directives, but rather "
	            /**/ "re-emit the ${##...} sequences as regular token sequences\n"
	            "Note: This field is preserved by ?#flags"),
	TYPE_GETSET("nomacros",
	            &lexer_get_nomacros,
	            &lexer_del_nomacros,
	            &lexer_set_nomacros,
	            "->?Dbool\n"
	            "When ?t, do not expand user-defined macros. Note however that "
	            /**/ "builtin macros are still expanded, unless ?#nobuiltinmacros "
	            /**/ "is also set to ?t\n"
	            "Note: This field is preserved by ?#flags"),
	TYPE_GETSET("nobuiltinmacros",
	            &lexer_get_nobuiltinmacros,
	            &lexer_del_nobuiltinmacros,
	            &lexer_set_nobuiltinmacros,
	            "->?Dbool\n"
	            "When ?t, do not expand builtin macros. Note however that "
	            /**/ "user-defined macros are still expanded, unless ?#nomacros "
	            /**/ "is also set to ?t\n"
	            "Note: This field is preserved by ?#flags"),
	TYPE_GETSET("asmcomments",
	            &lexer_get_asmcomments,
	            &lexer_del_asmcomments,
	            &lexer_set_asmcomments,
	            "->?Dbool\n"
	            "When ?t, unknown preprocessor directives (or all directives when "
	            /**/ "#nodirectives is true) are instead emit as comment tokens.\n"
	            "Note however that the requirement of directives having to be located "
	            /**/ "at the start of a line, only (and optionally) preceded by whitespace "
	            /**/ "still holds, meaning that assembly-like comments are only recognized "
	            /**/ "when they are found at the start of a line, too\n"
	            "Note: This field is preserved by ?#flags"),
	TYPE_GETSET("directives_noown_lf",
	            &lexer_get_directives_noown_lf,
	            &lexer_del_directives_noown_lf,
	            &lexer_set_directives_noown_lf,
	            "->?Dbool\n"
	            "When ?t, the line-feeds used to terminate a preprocessor directive "
	            /**/ "will be re-emit as a regular token (when ?#wantlf is ?t). Otherwise, "
	            /**/ "that token will be considered to be apart of the directive and not be "
	            /**/ "emit to the caller of ?#next or ?#nextpp\n"
	            "Note: This field is preserved by ?#flags"),
	TYPE_GETSET("comments_noown_lf",
	            &lexer_get_comments_noown_lf,
	            &lexer_del_comments_noown_lf,
	            &lexer_set_comments_noown_lf,
	            "->?Dbool\n"
	            "When ?t, the line-feeds used to terminate line-comments "
	            /**/ "will be re-emit as a regular token (when ?#wantlf is ?t). Otherwise, "
	            /**/ "that token will be considered to be apart of the comment and not be "
	            /**/ "emit to the caller of ?#next, ?#nextpp or ?#nextraw\n"
	            "Note: This field is preserved by ?#flags"),
	TYPE_GETSET("printmessagelocation",
	            &lexer_get_printmessagelocation,
	            &lexer_del_printmessagelocation,
	            &lexer_set_printmessagelocation,
	            "->?Dbool\n"
	            "When ?t, print the source location before "
	            /**/ "the message in ${##pragma message} directives\n"
	            "Note: This field is preserved by ?#flags"),
	TYPE_GETSET("printmessagenolf",
	            &lexer_get_printmessagenolf,
	            &lexer_del_printmessagenolf,
	            &lexer_set_printmessagenolf,
	            "->?Dbool\n"
	            "When ?t, don't append a trailing line-feed after "
	            /**/ "messages printed using ${##pragma message}\n"
	            "Note: This field is preserved by ?#flags"),
	TYPE_GETSET("parseincludestring",
	            &lexer_get_parseincludestring,
	            &lexer_del_parseincludestring,
	            &lexer_set_parseincludestring,
	            "->?Dbool\n"
	            "Parse strings are include-strings, which has the same behavior as parsing "
	            /**/ "all strings as though they were raw string literals, meaning that a "
	            /**/ "backslash-escape sequences are not recognized\n"
	            "The intended use for this is to parse the string of an ${##include} directive\n"
	            "Note that this flag also affects the behavior of ?Adecodestring?#Token, which "
	            /**/ "won't not recognize escape sequences for non-raw string literals, either\n"
	            "Note: This field is preserved by ?#flags"),
	TYPE_GETSET("nolegacyguards",
	            &lexer_get_nolegacyguards,
	            &lexer_del_nolegacyguards,
	            &lexer_set_nolegacyguards,
	            "->?Dbool\n"
	            "When ?t, don't automatically try to detect legacy-style ${##include} guards, "
	            /**/ "that is an ${##include} guard created by surrounding an entire source file with "
	            /**/ "a single ${##ifndef} block\n"
	            "This flag does not, however, affect the functionality of ${##pragma once}\n"
	            "Note: This field is preserved by ?#flags"),
	TYPE_GETSET("werror",
	            &lexer_get_werror,
	            &lexer_del_werror,
	            &lexer_set_werror,
	            "->?Dbool\n"
	            "When ?t, turn all warnings into errors\n"
	            "Note: This field is preserved by ?#flags"),
	TYPE_GETSET("wsystemheaders",
	            &lexer_get_wsystemheaders,
	            &lexer_del_wsystemheaders,
	            &lexer_set_wsystemheaders,
	            "->?Dbool\n"
	            "When ?t, ignore ?Aissystemheader?#File, and still produce "
	            /**/ "warnings in files marked as system headers\n"
	            "Note: This field is preserved by ?#flags"),
	TYPE_GETSET("nodeprecated",
	            &lexer_get_nodeprecated,
	            &lexer_del_nodeprecated,
	            &lexer_set_nodeprecated,
	            "->?Dbool\n"
	            "When ?t, don't emit warnings for keywords marked as "
	            /**/ "?#Keyword.isdeprecated or ?#Keyword.ispoisoned\n"
	            "Note: This field is preserved by ?#flags"),
	TYPE_GETSET("msvcmessages",
	            &lexer_get_msvcmessages,
	            &lexer_del_msvcmessages,
	            &lexer_set_msvcmessages,
	            "->?Dbool\n"
	            "When ?t, the file+line+column in warning and error messages is printed "
	            /**/ "as $\"file(line, column) : \". Otherwise it is printed as $\"file:line:column: \"\n"
	            "Note: This field is preserved by ?#flags"),
	TYPE_GETSET("nowarnings",
	            &lexer_get_nowarnings,
	            &lexer_del_nowarnings,
	            &lexer_set_nowarnings,
	            "->?Dbool\n"
	            "When ?t, the file+line+column in warning and error messages is printed "
	            /**/ "as $\"file(line, column) : \". Otherwise it is printed as $\"file:line:column: \"\n"
	            "Note: This field is preserved by ?#flags"),
	TYPE_GETSET("noencoding",
	            &lexer_get_noencoding,
	            &lexer_del_noencoding,
	            &lexer_set_noencoding,
	            "->?Dbool\n"
	            "When ?t, don't decode input text prior to processing it\n"
	            "This essentically means that whenever ?Anextchunk?#File is called, "
	            /**/ "the $binary is set to this value"),
	TYPE_GETSET("reemitunknownpragma",
	            &lexer_get_reemitunknownpragma,
	            &lexer_del_reemitunknownpragma,
	            &lexer_set_reemitunknownpragma,
	            "->?Dbool\n"
	            "When ?t, unknown pragma directives are re-emit, rather than consumed\n"
	            "Note: This field is preserved by ?#flags"),
#if 0
	TYPE_GETSET("charunsigned",
	            &lexer_get_charunsigned,
	            &lexer_del_charunsigned,
	            &lexer_set_charunsigned,
	            "->?Dbool\n"
	            "When ?t, characters are undefined when they appear in constant expressions"),
#endif

	TYPE_GETSET("eofonparen",
	            &lexer_get_eofonparen,
	            &lexer_del_eofonparen,
	            &lexer_set_eofonparen,
	            "->?Dbool\n"
	            "When ?t, end-of-file is signalled when a matching right-parenthesis "
	            /**/ "token is $\")\" is countered (s.a. ?#eofparen)\n"
	            "Note: This field is preserved by ?#flags"),
	TYPE_GETSET("eofparen",
	            &lexer_get_eofparen,
	            &lexer_del_eofparen,
	            &lexer_set_eofparen,
	            "->?Dint\n"
	            "Used in conjunction with ?#{eofonparen}: The amount of unmatched $\")\" tokens "
	            /**/ "to let through before the next $\")\" token will result in EOF being indicated "
	            /**/ "by setting ?Aid?#Token to $0"),
	TYPE_GETSET("eobfile",
	            &lexer_get_eobfile,
	            &lexer_del_eobfile,
	            &lexer_set_eobfile,
	            "->?#File\n"
	            "When bound, prevent seek-on-end-of-block (that is performing a call to "
	            /**/ "#File.nextchunk) when the current is equal to ?#eobfile\n"
	            "This is essentially the same as setting ?#noseek_on_eob to ?t, however "
	            /**/ "rather than affecting all files, it only affect a specific file"),
	TYPE_GETSET("eoffile",
	            &lexer_get_eoffile,
	            &lexer_del_eoffile,
	            &lexer_set_eoffile,
	            "->?#File\n"
	            "When bound, prevent pop-on-end-of-file (that is popping the current "
	            /**/ "file, as done by ?#popfile) when the current is equal to ?#eoffile\n"
	            "This is essentially the same as setting ?#nopop_on_eof to ?t, however "
	            /**/ "rather than affecting all files, it only affect a specific file"),
	TYPE_GETSET("macrolimit",
	            &lexer_get_macrolimit,
	            &lexer_del_macrolimit,
	            &lexer_set_macrolimit,
	            "->?Dint\n"
	            "The max number of times that a recursive macro (s.a. ?Aallowselfexpansion?#File) "
	            /**/ "is allowed to appear on the macro stack, before an error is emit, and further "
	            /**/ "expansion is prevented (defaults to $" PP_STR(TPPLEXER_DEFAULT_LIMIT_MREC) ", "
	            /**/ "which is also restored when deleting this property)"),
	TYPE_GETSET("includelimit",
	            &lexer_get_includelimit,
	            &lexer_del_includelimit,
	            &lexer_set_includelimit,
	            "->?Dint\n"
	            "The max number of files that can be included recursively before it is "
	            /**/ "determined that an include recursion has occurred, causing the latest "
	            /**/ "inclusion to fail, and an error to be emit (defaults to $" PP_STR(TPPLEXER_DEFAULT_LIMIT_INCL)
	            /**/ ", which is also restored when deleting this property)"),
	TYPE_GETSET("warningcount",
	            &lexer_get_warningcount,
	            &lexer_del_warningcount,
	            &lexer_set_warningcount,
	            "->?Dint\n"
	            "The total number of warnings that have already been emit "
	            /**/ "(including those which have been dismissed)"),
	TYPE_GETSET("errorcount",
	            &lexer_get_errorcount,
	            &lexer_del_errorcount,
	            &lexer_set_errorcount,
	            "->?Dint\n"
	            "The total number of errors that have already been emit\n"
	            "When non-zero, later compilation steps should throw a "
	            /**/ "compiler error, rather than proceeding with compilation\n"
	            "When this value exceeds ?#maxerrors, a :CompilerError is "
	            /**/ "thrown immediately"),
	TYPE_GETSET("maxerrors",
	            &lexer_get_maxerrors,
	            &lexer_del_maxerrors,
	            &lexer_set_maxerrors,
	            "->?Dint\n"
	            "The max number of errors which may occurr before compilation is "
	            /**/ "halted immediately, by setting the lexer to an error-state, and "
	            /**/ "throwing a :CompilerError"),
	TYPE_GETSET("tabsize",
	            &lexer_get_tabsize,
	            &lexer_del_tabsize,
	            &lexer_set_tabsize,
	            "->?Dint\n"
	            "The size of tabs (or rather their alignment multiple), as used "
	            /**/ "when calculating column offsets for source positions"),
	TYPE_GETSET("counter",
	            &lexer_get_counter,
	            &lexer_del_counter,
	            &lexer_set_counter,
	            "->?Dint\n"
	            "The counter value returned and incremented by the "
	            /**/ "$__COUNTER__ builtin macro"),
	TYPE_GETSET_END
 };


PRIVATE struct type_member tpconst lexer_class_members[] = {
	TYPE_MEMBER_CONST("Keyword", &DeeCompilerKeyword_Type),
	TYPE_MEMBER_CONST("Keywords", &DeeCompilerLexerKeywords_Type),
	TYPE_MEMBER_CONST("Extensions", &DeeCompilerLexerExtensions_Type),
	TYPE_MEMBER_CONST("Warnings", &DeeCompilerLexerWarnings_Type),
	TYPE_MEMBER_CONST("SysPaths", &DeeCompilerLexerSyspaths_Type),
	TYPE_MEMBER_CONST("Ifdef", &DeeCompilerLexerIfdef_Type),
	TYPE_MEMBER_CONST("Token", &DeeCompilerLexerToken_Type),
	TYPE_MEMBER_CONST(STR_File, &DeeCompilerFile_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_include(DeeCompilerWrapperObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *arg, *filename = Dee_None;
	DREF DeeObject *stream;
	/*ref*/ struct TPPFile *file;
	DeeArg_Unpack1Or2(err, argc, argv, "include", &arg, &filename);
	if (!DeeNone_Check(filename) &&
	    DeeObject_AssertTypeExact(filename, &DeeString_Type))
		goto err;
	if (DeeString_Check(arg)) {
		stream = DeeFile_Open(arg, OPEN_FRDONLY | OPEN_FCLOEXEC, 0);
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
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err_filename;
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_nextraw(DeeCompilerWrapperObject *self, size_t argc, DeeObject *const *argv) {
	tok_t result;
	uint16_t old_exceptsz;
	DeeArg_Unpack0(err, argc, argv, ":nextraw");
	old_exceptsz = DeeThread_Self()->t_exceptsz;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	result = TPPLexer_YieldRaw();
	COMPILER_END();
	if unlikely(result < 0 && old_exceptsz != DeeThread_Self()->t_exceptsz)
		goto err;
	return DeeInt_NewUInt(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_nextpp(DeeCompilerWrapperObject *self, size_t argc, DeeObject *const *argv) {
	tok_t result;
	uint16_t old_exceptsz;
	DeeArg_Unpack0(err, argc, argv, "nextpp");
	old_exceptsz = DeeThread_Self()->t_exceptsz;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	result = TPPLexer_YieldPP();
	COMPILER_END();
	if unlikely(result < 0 && old_exceptsz != DeeThread_Self()->t_exceptsz)
		goto err;
	return DeeInt_NewUInt(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_next(DeeCompilerWrapperObject *self, size_t argc, DeeObject *const *argv) {
	tok_t result;
	uint16_t old_exceptsz;
	DeeArg_Unpack0(err, argc, argv, "next");
	old_exceptsz = DeeThread_Self()->t_exceptsz;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	result = TPPLexer_Yield();
	COMPILER_END();
	if unlikely(result < 0 && old_exceptsz != DeeThread_Self()->t_exceptsz)
		goto err;
	return DeeInt_NewUInt(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_nextraw_nb(DeeCompilerWrapperObject *self, size_t argc, DeeObject *const *argv) {
	tok_t result;
	uint16_t old_exceptsz;
	DeeArg_Unpack0(err, argc, argv, "nextraw_nb");
	old_exceptsz = DeeThread_Self()->t_exceptsz;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	result = TPPLexer_YieldRawNB();
	COMPILER_END();
	if unlikely(result < 0 && old_exceptsz != DeeThread_Self()->t_exceptsz)
		goto err;
	return DeeInt_NewUInt(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_nextpp_nb(DeeCompilerWrapperObject *self, size_t argc, DeeObject *const *argv) {
	tok_t result;
	uint16_t old_exceptsz;
	DeeArg_Unpack0(err, argc, argv, "nextpp_nb");
	old_exceptsz = DeeThread_Self()->t_exceptsz;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	result = TPPLexer_YieldPPNB();
	COMPILER_END();
	if unlikely(result < 0 && old_exceptsz != DeeThread_Self()->t_exceptsz)
		goto err;
	return DeeInt_NewUInt(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_next_nb(DeeCompilerWrapperObject *self, size_t argc, DeeObject *const *argv) {
	tok_t result;
	uint16_t old_exceptsz;
	DeeArg_Unpack0(err, argc, argv, "next_nb");
	old_exceptsz = DeeThread_Self()->t_exceptsz;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	result = TPPLexer_YieldNB();
	COMPILER_END();
	if unlikely(result < 0 && old_exceptsz != DeeThread_Self()->t_exceptsz)
		goto err;
	return DeeInt_NewUInt(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_seterr(DeeCompilerWrapperObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	DeeArg_Unpack0(err, argc, argv, "seterr");
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	result = TPPLexer_SetErr();
	COMPILER_END();
	return_bool(result != 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_unseterr(DeeCompilerWrapperObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	DeeArg_Unpack0(err, argc, argv, "unseterr");
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	result = TPPLexer_UnsetErr();
	COMPILER_END();
	return_bool(result != 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_popfile(DeeCompilerWrapperObject *self, size_t argc, DeeObject *const *argv) {
	bool result;
	DeeArg_Unpack0(err, argc, argv, "popfile");
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	result = TPPLexer_Current->l_token.t_file != &TPPFile_Empty;
	if (result)
		TPPLexer_PopFile();
	COMPILER_END();
	return_bool(result);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_getkwd(DeeCompilerWrapperObject *self, size_t argc,
             DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	char const *name_utf8;
	struct TPPKeyword *kwd;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("getkwd", params: """
	DeeStringObject *name;
	bool create = true;
""", docStringPrefix: "lexer");]]]*/
#define lexer_getkwd_params "name:?Dstring,create=!t"
	struct {
		DeeStringObject *name;
		bool create;
	} args;
	args.create = true;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__name_create, "o|b:getkwd", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.name, &DeeString_Type))
		goto err;
	name_utf8 = DeeString_AsUtf8(Dee_AsObject(args.name));
	if unlikely(!name_utf8)
		goto err;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	kwd = TPPLexer_LookupKeyword(name_utf8,
	                             WSTR_LENGTH(name_utf8),
	                             args.create);
	if unlikely(!kwd) {
		if (args.create) {
			result = NULL;
		} else {
			result = DeeNone_NewRef();
		}
	} else {
		result = DeeCompiler_GetKeyword(kwd);
	}
	COMPILER_END();
	return result;
err:
	return NULL;
}
DOC_DEF(lexer_getkwd_doc,
        "(" lexer_getkwd_params ")->?X2?#Keyword?N\n"
        "Lookup the keyword associated with @name and return it, or ?N "
        /**/ "when @create is ?f and the keyword hasn't been accessed yet");



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_getxkwd(DeeCompilerWrapperObject *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	char const *name_utf8;
	struct TPPKeyword *kwd;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("getxkwd", params: """
	DeeStringObject *name;
	bool create = true;
""", docStringPrefix: "lexer");]]]*/
#define lexer_getxkwd_params "name:?Dstring,create=!t"
	struct {
		DeeStringObject *name;
		bool create;
	} args;
	args.create = true;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__name_create, "o|b:getxkwd", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.name, &DeeString_Type))
		goto err;
	name_utf8 = DeeString_AsUtf8(Dee_AsObject(args.name));
	if unlikely(!name_utf8)
		goto err;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	kwd = TPPLexer_LookupEscapedKeyword(name_utf8,
	                                    WSTR_LENGTH(name_utf8),
	                                    args.create);
	if unlikely(!kwd) {
		if (args.create) {
			result = NULL;
		} else {
			result = DeeNone_NewRef();
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
        "(" lexer_getxkwd_params ")->?X2?#Keyword?N\n"
        "Same as ?#getkwd, however the given @name may contain escaped "
        /**/ "line-feeds that are removed prior to it being used to lookup "
        /**/ "a keyword");


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_getkwdid(DeeCompilerWrapperObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	struct TPPKeyword *kwd;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("getkwdid", params: """
	unsigned int id;
""", docStringPrefix: "lexer");]]]*/
#define lexer_getkwdid_params "id:?Dint"
	struct {
		unsigned int id;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "getkwdid", &args.id, "u", DeeObject_AsUInt);
/*[[[end]]]*/
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	kwd = TPPLexer_LookupKeywordID((tok_t)args.id);
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
DOC_DEF(lexer_getkwdid_doc,
        "(" lexer_getkwdid_params ")->?X2?#Keyword?N\n"
        "Lookup the keyword associated with the given @id, "
        /**/ "returning it or ?N if no such keyword exists.\n"
        "WARNING: This is an O(n) operation and should be avoided if at all possible");


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_undef(DeeCompilerWrapperObject *self, size_t argc, DeeObject *const *argv) {
	int error;
	char const *utf8_name;
	uint16_t old_exceptsz;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("undef", params: """
	DeeStringObject *name;
""", docStringPrefix: "lexer");]]]*/
#define lexer_undef_params "name:?Dstring"
	struct {
		DeeStringObject *name;
	} args;
	DeeArg_Unpack1(err, argc, argv, "undef", &args.name);
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.name, &DeeString_Type))
		goto err;
	utf8_name = DeeString_AsUtf8(Dee_AsObject(args.name));
	if unlikely(!utf8_name)
		goto err;
	old_exceptsz = DeeThread_Self()->t_exceptsz;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	error = TPPLexer_Undef(utf8_name, WSTR_LENGTH(utf8_name));
	COMPILER_END();
	if unlikely(old_exceptsz != DeeThread_Self()->t_exceptsz)
		goto err;
	return_bool(error != 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_define(DeeCompilerWrapperObject *self, size_t argc,
             DeeObject *const *argv, DeeObject *kw) {
	int error;
	char const *utf8_name, *utf8_value;
	uint16_t old_exceptsz;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("define", params: """
	DeeStringObject *name;
	DeeStringObject *value;
	bool builtin = false;
""", docStringPrefix: "lexer");]]]*/
#define lexer_define_params "name:?Dstring,value:?Dstring,builtin=!f"
	struct {
		DeeStringObject *name;
		DeeStringObject *value;
		bool builtin;
	} args;
	args.builtin = false;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__name_value_builtin, "oo|b:define", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.name, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(args.value, &DeeString_Type))
		goto err;
	utf8_name = DeeString_AsUtf8(Dee_AsObject(args.name));
	if unlikely(!utf8_name)
		goto err;
	utf8_value = DeeString_AsUtf8(Dee_AsObject(args.value));
	if unlikely(!utf8_value)
		goto err;
	old_exceptsz = DeeThread_Self()->t_exceptsz;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	error = TPPLexer_Define(utf8_name, WSTR_LENGTH(utf8_name),
	                        utf8_value, WSTR_LENGTH(utf8_value),
	                        args.builtin ? TPPLEXER_DEFINE_FLAG_BUILTIN
	                                     : TPPLEXER_DEFINE_FLAG_NONE);
	COMPILER_END();
	if unlikely(!error && old_exceptsz != DeeThread_Self()->t_exceptsz)
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_addassert(DeeCompilerWrapperObject *self, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	int error;
	char const *utf8_name, *utf8_value;
	uint16_t old_exceptsz;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("addassert", params: """
	DeeStringObject *predicate;
	DeeStringObject *answer;
""", docStringPrefix: "lexer");]]]*/
#define lexer_addassert_params "predicate:?Dstring,answer:?Dstring"
	struct {
		DeeStringObject *predicate;
		DeeStringObject *answer;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__predicate_answer, "oo:addassert", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.predicate, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(args.answer, &DeeString_Type))
		goto err;
	utf8_name = DeeString_AsUtf8(Dee_AsObject(args.predicate));
	if unlikely(!utf8_name)
		goto err;
	utf8_value = DeeString_AsUtf8(Dee_AsObject(args.answer));
	if unlikely(!utf8_value)
		goto err;
	old_exceptsz = DeeThread_Self()->t_exceptsz;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	error = TPPLexer_AddAssert(utf8_name, WSTR_LENGTH(utf8_name),
	                           utf8_value, WSTR_LENGTH(utf8_value));
	COMPILER_END();
	if unlikely(!error && old_exceptsz != DeeThread_Self()->t_exceptsz)
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_delassert(DeeCompilerWrapperObject *self, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	int error;
	char const *utf8_name, *utf8_value = NULL;
	uint16_t old_exceptsz;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("delassert", params: """
	DeeStringObject *predicate;
	DeeStringObject *answer = NULL;
""", docStringPrefix: "lexer");]]]*/
#define lexer_delassert_params "predicate:?Dstring,answer?:?Dstring"
	struct {
		DeeStringObject *predicate;
		DeeStringObject *answer;
	} args;
	args.answer = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__predicate_answer, "o|o:delassert", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.predicate, &DeeString_Type))
		goto err;
	utf8_name = DeeString_AsUtf8(Dee_AsObject(args.predicate));
	if unlikely(!utf8_name)
		goto err;
	if (args.answer) {
		if (DeeObject_AssertTypeExact(args.answer, &DeeString_Type))
			goto err;
		utf8_value = DeeString_AsUtf8(Dee_AsObject(args.answer));
		if unlikely(!utf8_value)
			goto err;
	}
	old_exceptsz = DeeThread_Self()->t_exceptsz;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	error = TPPLexer_DelAssert(utf8_name, WSTR_LENGTH(utf8_name),
	                           utf8_value, utf8_value ? WSTR_LENGTH(utf8_value) : 0);
	COMPILER_END();
	if unlikely(old_exceptsz != DeeThread_Self()->t_exceptsz)
		goto err;
	return_bool(error != 0);
err:
	return NULL;
}


PRIVATE struct type_method tpconst lexer_methods[] = {
	TYPE_METHOD("include", &lexer_include,
	            "(stream:?DFile,filename:?Dstring=!N)\n"
	            "(filename:?Dstring,filename:?Dstring=!N)\n"
	            "Include a new file, pushing its contents onto the ${##include}-stack\n"
	            "Note that when including a file with the current token being $0 (as indicate of EOF), "
	            /**/ "you must call one of the ?#next-functions in order to load the first token of the newly "
	            /**/ "pushed file. - Failing to do so will cause the compiler to not function properly, as it "
	            /**/ "will think that no input data is available, causing compiler error to be produced:\n"
	            "${"
	            /**/ "import Compiler from rt;\n"
	            /**/ "import File from deemon;\n"
	            /**/ "local com = Compiler();\n"
	            /**/ "com.lexer.include(File.open(\"input.dee\"));\n"
	            /**/ "com.lexer.next(); /* Don't forget to always load the first token */\n"
	            /**/ "local ast = com.parser.parse_allstmt();\n"
	            /**/ "print ast;"
	            "}\n"
	            "Hint: In order to tokenize source code from a string, use ?AReader?DFile"),
	TYPE_METHOD("nextraw", &lexer_nextraw,
	            "->?Dint\n"
	            "Load the next token and return its id (no macros, or preprocessor directives are processed)"),
	TYPE_METHOD("nextpp", &lexer_nextpp,
	            "->?Dint\n"
	            "Load the next token and return its id (no macros are processed)"),
	TYPE_METHOD("next", &lexer_next,
	            "->?Dint\n"
	            "Load the next token and return its id"),
	TYPE_METHOD("nextraw_nb", &lexer_nextraw_nb,
	            "->?Dint\n"
	            "Load the next token and return its id while trying not to block (s.a. ?#nonblocking) (no macros, or preprocessor directives are processed)"),
	TYPE_METHOD("nextpp_nb", &lexer_nextpp_nb,
	            "->?Dint\n"
	            "Load the next token and return its id while trying not to block (s.a. ?#nonblocking) (no macros are processed)"),
	TYPE_METHOD("next_nb", &lexer_next_nb,
	            "->?Dint\n"
	            "Load the next token and return its id while trying not to block (s.a. ?#nonblocking)"),
	TYPE_METHOD("seterr", &lexer_seterr,
	            "->?Dbool\n"
	            "Switch the lexer into an error state"),
	TYPE_METHOD("unseterr", &lexer_unseterr,
	            "->?Dbool\n"
	            "Restore the lexer after it was set to an error state"),
	TYPE_METHOD("popfile", &lexer_popfile,
	            "->?Dbool\n"
	            "Pop the last-?#{include}d file and switch back to the file before then (s.a. ?#File)"),
	TYPE_KWMETHOD("getkwd", &lexer_getkwd, DOC_GET(lexer_getkwd_doc)),
	TYPE_KWMETHOD("getxkwd", &lexer_getxkwd, DOC_GET(lexer_getxkwd_doc)),
	TYPE_METHOD("getkwdid", &lexer_getkwdid, DOC_GET(lexer_getkwdid_doc)),
	TYPE_METHOD("undef", &lexer_undef,
	            "(" lexer_undef_params ")->?Dbool\n"
	            "Delete a user-defined macro definition for a macro @name, returning ?t "
	            /**/ "if such a definition existed and got deleted, or ?f if no such definition "
	            /**/ "existed, and therefor didn't get deleted, either"),
	TYPE_KWMETHOD("define", &lexer_define,
	              "(" lexer_define_params ")\n"
	              "#pbuiltin{When ?t define the macro as builtin, meaning the "
	              /*     */ "definition set by @value is restored when resetting macros}"
	              "Define a new keyword-like macro @name to expand to @value"),
	TYPE_KWMETHOD("addassert", &lexer_addassert,
	              "(" lexer_addassert_params ")\n"
	              "Define an assertion @answer for a given @predicate, such that "
	              /**/ "${##if #predicate(answer)} evaluates to ?t when encountered "
	              "within a preprocessor expression"),
	TYPE_KWMETHOD("delassert", &lexer_delassert,
	              "(" lexer_delassert_params ")->?Dbool\n"
	              "#r{Returns ?t when at least 1 answer got deleted for the given @predicate}"
	              "Delete an assertion @answer, or all assertions made for a given "
	              /**/ "@predicate, such that ${##if #predicate(answer)} no longer evaluates "
	              /**/ "to ?t when encountered within a preprocessor expression"),
	/* TODO */
	TYPE_METHOD_END
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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeCompilerWrapperObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
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
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
lexer_keywords_getitem(DeeCompilerWrapperObject *self,
                       DeeObject *name) {
	DREF DeeObject *result;
	struct TPPKeyword *kwd;
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	kwd = TPPLexer_LookupKeyword(DeeString_STR(name),
	                             DeeString_SIZE(name),
	                             1);
	if unlikely(!kwd) {
		result = NULL;
	} else {
		result = DeeCompiler_GetKeyword(kwd);
	}
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE struct type_seq lexer_keywords_seq = {
	/* .tp_iter     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_keywords_iter,
	/* .tp_sizeob   = */ NULL,
	/* .tp_contains = */ NULL,
	/* .tp_getitem  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&lexer_keywords_getitem,
	/* .tp_delitem  = */ NULL,
	/* .tp_setitem  = */ NULL,
	/* .tp_getrange = */ NULL,
	/* .tp_delrange = */ NULL,
	/* .tp_setrange = */ NULL
};



PRIVATE struct type_method tpconst lexer_keywords_methods[] = {
	TYPE_KWMETHOD("getkwd", &lexer_getkwd, DOC_GET(lexer_getkwd_doc)),
	TYPE_KWMETHOD("getxkwd", &lexer_getxkwd, DOC_GET(lexer_getxkwd_doc)),
	TYPE_METHOD("getkwdid", &lexer_getkwdid, DOC_GET(lexer_getkwdid_doc)),
	TYPE_METHOD_END
};

INTERN DeeTypeObject DeeCompilerLexerKeywords_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_LexerKeywords",
	/* .tp_doc      = */ DOC("getitem(name:?Dstring)->" DR_CKeyword),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeCompilerWrapperObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&DeeCompilerWrapper_Fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&DeeCompilerWrapper_Visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &lexer_keywords_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeCompilerWrapperObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL, /* TODO */
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeCompilerWrapperObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL, /* TODO */
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
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
	DeeArg_Unpack0(err, argc, argv, "push");
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
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
	DeeArg_Unpack0(err, argc, argv, "pop");
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	result = TPPLexer_PopInclude();
	COMPILER_END();
	return_bool(result != 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_syspaths_insert(DeeCompilerWrapperObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	DeeObject *path;
	char *copy;
	char const *utf8;
	DeeArg_Unpack1(err, argc, argv, "insert", &path);
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
	utf8 = DeeString_AsUtf8(path);
	if unlikely(!utf8)
		goto err;
	copy = (char *)Dee_Mallocc(WSTR_LENGTH(path), sizeof(char));
	if unlikely(!copy)
		goto err;
	memcpyc(copy, path, WSTR_LENGTH(path), sizeof(char));
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	result = TPPLexer_AddIncludePath(copy, WSTR_LENGTH(path));
	COMPILER_END();
	Dee_Free(copy);
	if unlikely(!result)
		goto err;
	return_bool(result == 1);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_syspaths_remove(DeeCompilerWrapperObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	DeeObject *path;
	char const *utf8;
	char *copy;
	DeeArg_Unpack1(err, argc, argv, "remove", &path);
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
	utf8 = DeeString_AsUtf8(path);
	if unlikely(!utf8)
		goto err;
	copy = (char *)Dee_Mallocc(WSTR_LENGTH(path), sizeof(char));
	if unlikely(!copy)
		goto err;
	memcpyc(copy, path, WSTR_LENGTH(path), sizeof(char));
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	result = TPPLexer_DelIncludePath(copy, WSTR_LENGTH(path));
	COMPILER_END();
	Dee_Free(copy);
	return_bool(result != 0);
err:
	return NULL;
}

PRIVATE struct type_method tpconst lexer_syspaths_methods[] = {
	TYPE_METHOD("push", &lexer_syspaths_push,
	            "()\n"
	            "Push (remember) the current state of system include paths\n"
	            "This is the same as using ${##pragma TPP include_path(push)}"),
	TYPE_METHOD(STR_pop, &lexer_syspaths_pop,
	            "->?Dbool\n"
	            "Pop (restore) a previously pushed system include path state\n"
	            "This is the same as using ${##pragma TPP include_path(pop)}"),
	TYPE_METHOD(STR_insert, &lexer_syspaths_insert,
	            "(path:?Dstring)->?Dbool\n"
	            "Append the given @path at the end of the list of system include paths"),
	TYPE_METHOD(STR_remove, &lexer_syspaths_remove,
	            "(path:?Dstring)->?Dbool\n"
	            "Remove the given @path from the list of system include paths"),
	TYPE_METHOD_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_syspaths_size(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	result = DeeInt_NewSize(TPPLexer_Current->l_syspaths.il_pathc);
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
lexer_syspaths_getitem(DeeCompilerWrapperObject *self,
                       DeeObject *index) {
	DREF DeeObject *result;
	size_t i;
	if unlikely(DeeObject_AsSize(index, &i)) {
		DeeRT_ErrIndexOverflow(self);
		goto err;
	}
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	if (i >= TPPLexer_Current->l_syspaths.il_pathc) {
		DeeRT_ErrIndexOutOfBounds(self, i, TPPLexer_Current->l_syspaths.il_pathc);
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
	/* .tp_iter     = */ NULL,
	/* .tp_sizeob   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lexer_syspaths_size,
	/* .tp_contains = */ NULL,
	/* .tp_getitem  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&lexer_syspaths_getitem,
	/* .tp_delitem  = */ NULL,
	/* .tp_setitem  = */ NULL,
	/* .tp_getrange = */ NULL,
	/* .tp_delrange = */ NULL,
	/* .tp_setrange = */ NULL
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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeCompilerWrapperObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&DeeCompilerWrapper_Fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&DeeCompilerWrapper_Visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &lexer_syspaths_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeCompilerWrapperObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL, /* TODO */
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
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
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	result = TPPLexer_Global.l_token.t_id > 0;
	COMPILER_END();
	return result;
err:
	return -1;
}

PRIVATE ptrdiff_t
(TPPCALL unicode_printer_tppappend)(void *arg, char const *__restrict buf, size_t bufsize) {
	Dee_ssize_t result;
	result = unicode_printer_print((struct unicode_printer *)arg, buf, bufsize);
	return unlikely(result < 0) ? -1 : 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
token_text(DeeCompilerWrapperObject *__restrict self) {
	ptrdiff_t error;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err_printer;
	error = TPP_PrintToken(&unicode_printer_tppappend, &printer);
	COMPILER_END();
	if unlikely(error)
		goto err_printer;
	return unicode_printer_pack(&printer);
err_printer:
	unicode_printer_fini(&printer);
	return NULL;
}

PRIVATE ptrdiff_t
(TPPCALL unicode_printer_tppappend_escape)(void *arg, char const *__restrict buf, size_t bufsize) {
	Dee_ssize_t result;
	result = DeeFormat_Quote(&unicode_printer_print, arg, buf, bufsize);
	return unlikely(result < 0) ? -1 : 0;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
token_str(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	result = get_token_name(tok, token.t_kwd);
	if unlikely(result == ITER_DONE)
		result = DeeString_Chr((uint32_t)tok); /* Shouldn't normally happen (but may after a partial reset) */
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
token_repr(DeeCompilerWrapperObject *__restrict self) {
	ptrdiff_t error;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err_printer;
	if unlikely(unicode_printer_putc(&printer, '\"'))
		goto err_printer;
	error = TPP_PrintToken(&unicode_printer_tppappend_escape, &printer);
	COMPILER_END();
	if unlikely(error)
		goto err_printer;
	if unlikely(unicode_printer_putc(&printer, '\"'))
		goto err_printer;
	return unicode_printer_pack(&printer);
err_printer:
	unicode_printer_fini(&printer);
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
token_rawtext(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	result = DeeString_NewUtf8(TPPLexer_Current->l_token.t_begin,
	                           (size_t)(TPPLexer_Current->l_token.t_end -
	                                    TPPLexer_Current->l_token.t_begin),
	                           STRING_ERROR_FIGNORE);
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
token_id(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result = NULL;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	result = DeeInt_NewInt(TPPLexer_Current->l_token.t_id);
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
token_delid(DeeCompilerWrapperObject *__restrict self) {
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	TPPLexer_Current->l_token.t_id = 0;
	COMPILER_END();
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
token_setid(DeeCompilerWrapperObject *__restrict self,
            DeeObject *__restrict value) {
	int new_id;
	if (DeeObject_AsInt(value, &new_id))
		goto err;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	TPPLexer_Current->l_token.t_id = new_id;
	COMPILER_END();
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
token_num(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	result = DeeInt_NewULong(TPPLexer_Current->l_token.t_num);
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
token_delnum(DeeCompilerWrapperObject *__restrict self) {
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	TPPLexer_Current->l_token.t_num = 0;
	COMPILER_END();
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
token_setnum(DeeCompilerWrapperObject *__restrict self,
             DeeObject *__restrict value) {
	unsigned long new_num;
	if (DeeObject_AsULong(value, &new_num))
		goto err;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	TPPLexer_Current->l_token.t_num = new_num;
	COMPILER_END();
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
token_keyword(DeeCompilerWrapperObject *__restrict self) {
	DREF DeeObject *result;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	if (TPP_ISKEYWORD(TPPLexer_Current->l_token.t_id)) {
		result = DeeCompiler_GetKeyword(TPPLexer_Current->l_token.t_kwd);
	} else {
		result = Dee_None;
		Dee_Incref(result);
	}
	COMPILER_END();
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
token_setkeyword(DeeCompilerWrapperObject *__restrict self,
                 DeeObject *__restrict value) {
	struct TPPKeyword *kwd;
	int result = -1;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto done;
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
done:
	return result;
}

PRIVATE struct type_getset tpconst lexer_token_getsets[] = {
	TYPE_GETTER("text", &token_text,
	            "->?Dstring\n"
	            "Returns the textual representation of the current token\n"
	            "Escaped linefeeds have been removed within the returned string"),
	TYPE_GETTER("rawtext", &token_rawtext,
	            "->?Dstring\n"
	            "Returns the raw textual representation of the current token\n"
	            "Escaped linefeeds found in the original source are included in the returned string"),
	TYPE_GETSET("id", &token_id, &token_delid, &token_setid,
	            "->?Dint\n"
	            "Get, del (set to $0), or set the id (kind) of the current token"),
	TYPE_GETSET("num", &token_num, &token_delnum, &token_setnum,
	            "->?Dint\n"
	            "Get, del (set to $0), or set the current token number\n"
	            "The current token number is incremented by at least $1 every "
	            /**/ "time one of ?Anext" DR_CLexer ", ?Anextpp" DR_CLexer " "
	            /**/ "or ?Anextraw" DR_CLexer " are called"),
	TYPE_GETSET("keyword", &token_keyword, NULL, &token_setkeyword,
	            "->?X2" DR_CKeyword "?N\n"
	            "Returns the keyword associated with the current token, or "
	            /**/ ":none if the current token doesn't have an associated keyword\n"
	            "When setting this field, both the token's ?#Keyword, as well "
	            /**/ "as its ?#id field are set to the given value"),
	TYPE_GETTER("file", &lexer_get_file,
	            "->?X2?#File?N\n"
	            "Returns the currently active file, or ?N if no file is currently active"),
	TYPE_GETTER("position", &lexer_get_tokenposition,
	            "->?X2?T3?#File?Dint?Dint?N\n"
	            "Return the exact source position of @this token within a macro definition, or text source"),
	TYPE_GETTER("endposition", &lexer_get_tokenendposition,
	            "->?X2?T3?#File?Dint?Dint?N\n"
	            "Same as ?#position, however return the end position of the current token"),
	TYPE_GETTER("atstartofline", &lexer_get_atstartofline,
	            "->?Dbool\n"
	            "Returns ?t if the current token is located at the "
	            /**/ "start of a line, optionally prefixed by whitespace"),
	TYPE_GETSET_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_token_decodestring(DeeCompilerWrapperObject *self, size_t argc, DeeObject *const *argv) {
	int error;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	DeeArg_Unpack0(err_printer, argc, argv, "decodestring");
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err_printer;
	if (TPPLexer_Current->l_token.t_id != TOK_STRING ||
	    (tok == TOK_CHAR && !HAS(EXT_CHARACTER_LITERALS))) {
		error = DeeError_Throwf(&DeeError_ValueError,
		                        "The current token isn't a string");
	} else {
		error = ast_decode_unicode_string(&printer);
	}
	COMPILER_END();
	if unlikely(error)
		goto err_printer;
	return unicode_printer_pack(&printer);
err_printer:
	unicode_printer_fini(&printer);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lexer_token_decodeinteger(DeeCompilerWrapperObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result = NULL;
	bool warnchar = true;
	if (DeeArg_UnpackStruct(argc, argv, "|b:decodeinteger", &warnchar))
		goto done;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto done;
	if (TPPLexer_Current->l_token.t_id == TOK_INT) {
		result = DeeInt_FromString(token.t_begin, (size_t)(token.t_end - token.t_begin),
		                           DEEINT_STRING(0, DEEINT_STRING_FESCAPED));
	} else if (TPPLexer_Current->l_token.t_id == TOK_CHAR) {
		tint_t value;
		if unlikely(TPP_Atoi(&value) == TPP_ATOI_ERR)
			goto done_compiler_end;
		if (warnchar && WARN(W_DEPRECATED_CHARACTER_INT))
			goto done_compiler_end;
		result = DeeInt_NewInt64(value);
	} else {
		DeeError_Throwf(&DeeError_ValueError,
		                "The current token isn't an integer or character");
	}
done_compiler_end:
	COMPILER_END();
done:
	return result;
}

PRIVATE struct type_method tpconst lexer_token_methods[] = {
	TYPE_METHOD("decodestring", &lexer_token_decodestring,
	            "->?Dstring\n"
	            "#tValueError{The current token isn't a string}"
	            "#tUnicodeDecodeError{The string contains an invalid escape-character}"
	            "Decode the current token (which must be a string-token) as a string"),
	TYPE_METHOD("decodeinteger", &lexer_token_decodeinteger,
	            "(warnchar=!t)->?Dint\n"
	            "#tValueError{The current token isn't an integer, or character}"
	            "#tValueError{The current token contains an invalid digit}"
	            "Decode the current token (which must be an integer or character) as an ?Dint object\n"
	            "When @warnchar is ?t, emit a warning when a character is used as an integer"),
	TYPE_METHOD_END
};


PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
token_hash(DeeCompilerWrapperObject *__restrict self) {
	Dee_hash_t result;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	result = get_token_namehash(tok, token.t_kwd);
	COMPILER_END();
	return result;
err:
	DeeError_Print("Error hashing token", Dee_ERROR_PRINT_DOHANDLE);
	return (Dee_hash_t)-1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
token_compare_eq(DeeCompilerWrapperObject *self, DeeObject *other) {
	bool result;
	char const *other_utf8;
	tok_t other_id;
	if (DeeObject_AssertTypeExact(other, &DeeString_Type))
		goto err;
	other_utf8 = DeeString_AsUtf8(other);
	if unlikely(!other_utf8)
		goto err;
	if (COMPILER_BEGIN(self->cw_compiler))
		goto err;
	other_id = get_token_from_str(other_utf8, false);
	result   = tok == other_id;
	COMPILER_END();
	return result ? 0 : 1;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp token_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&token_hash,
	/* .tp_compare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&token_compare_eq,
};


INTERN DeeTypeObject DeeCompilerLexerToken_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_Token",
	/* .tp_doc      = */ DOC("str->\n"
	                         "Returns a string representation for the ?#id of @this token\n"
	                         "For most tokens, this is equivalent to ?#text, with the exception of the following:\n"
	                         "#T{Str-value|Token|Description~"
	                         /**/ "$\"\"|${<EOF>}|End-of-file is encoded as an empty string&"
	                         /**/ "$\"\\\'\"|$\'x\'|Character tokens have a single-quote as str-id&"
	                         /**/ "$\"\\\"\"|$\"foo\"|String tokens have a double-quote as str-id (including raw string literals)&"
	                         /**/ "$\"0\"|$42|Integer tokens have use the digit 0 as str-id&"
	                         /**/ "$\".0\"|${1.5}|Floating point tokens are encoded as `.0' as str-id&"
	                         /**/ "$\"\\n\"|#C{<LF>}|Any kind of line-feed token is encoded as an LF-character&"
	                         /**/ "$\" \"|#C{<SPACE>}|A space-sequence of any sort of length is encoded as a single space character&"
	                         /**/ "$\"//\"|#C{<COMMENT>}|Any kind of comment token is encoded as 2 forward slashes #C{//}"
	                         "}\n"
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
	                         /**/ "switch (com.lexer.token) {\n"
	                         /**/ "case \"foobar\":\n"
	                         /**/ "	print \"keyword: foobar\";\n"
	                         /**/ "	break;\n"
	                         /**/ "case \"(\":\n"
	                         /**/ "	print \"token: lparen\";\n"
	                         /**/ "	break;\n"
	                         /**/ "case \"++\":\n"
	                         /**/ "	print \"token: increment\";\n"
	                         /**/ "	break;\n"
	                         /**/ "case \"\":\n"
	                         /**/ "	print \"Special token: end-of-file\";\n"
	                         /**/ "	break;\n"
	                         /**/ "case \"//\":\n"
	                         /**/ "	print \"Special token: comment\";\n"
	                         /**/ "	break;\n"
	                         /**/ "case \"\\\"\":\n"
	                         /**/ "	print \"Special token: string\", com.lexer.token.decodestring();\n"
	                         /**/ "	break;\n"
	                         /**/ "case \"\\\'\":\n"
	                         /**/ "	print \"Special token: character\", com.lexer.token.decodeinteger();\n"
	                         /**/ "	break;\n"
	                         /**/ "case \"0\":\n"
	                         /**/ "	print \"Special token: integer\", com.lexer.token.decodeinteger();\n"
	                         /**/ "	break;\n"
	                         /**/ "default:\n"
	                         /**/ "	print \"Other: \", repr com.lexer.token;\n"
	                         /**/ "	break;\n"
	                         /**/ "}"
	                         "}"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCompilerWrapper_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeCompilerWrapperObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&token_str,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&token_repr,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&token_bool
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &token_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
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
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		result = DeeString_Newf("<file %$q>",
		                        file->f_namesize,
		                        file->f_name);
	}
	COMPILER_END();
done:
	return result;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_istext(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		result = DeeBool_For(file->f_kind == TPPFILE_KIND_TEXT);
		Dee_Incref(result);
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_ismacro(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		result = DeeBool_For(file->f_kind == TPPFILE_KIND_MACRO);
		Dee_Incref(result);
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_isexpand(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		result = DeeBool_For(file->f_kind == TPPFILE_KIND_EXPLICIT);
		Dee_Incref(result);
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_origin(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (!file->f_prev || file->f_prev == &TPPFile_Empty) {
			result = DeeNone_NewRef();
		} else {
			result = DeeCompiler_GetFile(file->f_prev);
		}
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_alltext(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		result = DeeString_NewUtf8(file->f_begin,
		                           (size_t)(file->f_end - file->f_begin),
		                           STRING_ERROR_FIGNORE);
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_nexttext(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		result = DeeString_NewUtf8(file->f_pos,
		                           (size_t)(file->f_end - file->f_pos),
		                           STRING_ERROR_FIGNORE);
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_position(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		struct TPPLCInfo lc;
		TPPFile_LCAt(file, &lc, file->f_pos);
		result = DeeTuple_Newf("dd", lc.lc_line, lc.lc_col);
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_filename(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		char const *filename;
		size_t filename_length;
		filename = TPPFile_Filename(file, &filename_length);
		result   = DeeString_NewUtf8(filename, filename_length, STRING_ERROR_FIGNORE);
	}
	COMPILER_END();
done:
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
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
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
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
file_setfilename(DeeCompilerItemObject *__restrict self,
                 DeeObject *__restrict value) {
	struct TPPFile *file;
	char const *utf8;
	int result = -1;
	if (DeeObject_AssertTypeExact(value, &DeeString_Type))
		goto done;
	utf8 = DeeString_AsUtf8(value);
	if unlikely(!utf8)
		goto done;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
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
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		char const *filename;
		size_t filename_length;
		filename = TPPFile_RealFilename(file, &filename_length);
		result   = DeeString_NewUtf8(filename, filename_length, STRING_ERROR_FIGNORE);
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_name(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		result = DeeString_NewUtf8(file->f_name,
		                           file->f_namesize,
		                           STRING_ERROR_FIGNORE);
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_lineoffset(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_TEXT) {
			err_not_a_textfile(file);
		} else {
			result = DeeInt_NewInt(file->f_textfile.f_lineoff);
		}
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
file_dellineoffset(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	int result = -1;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
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
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
file_setlineoffset(DeeCompilerItemObject *__restrict self,
                   DeeObject *__restrict value) {
	struct TPPFile *file;
	int result = -1, new_value;
	if (DeeObject_AsInt(value, &new_value))
		goto done;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
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
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_TEXT) {
			err_not_a_textfile(file);
		} else {
			result = Dee_AsObject(file->f_textfile.f_ownedstream);
			if (result == (DREF DeeObject *)TPP_STREAM_INVALID)
				result = Dee_AsObject(file->f_textfile.f_stream);
			if likely(result) {
				Dee_Incref(result);
			} else {
				result = DeeRT_ErrUnboundAttrCStr(self, "stream");
			}
		}
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_getguard(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
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
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
file_delguard(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	int result = -1;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
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
done:
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
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
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
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_getnewguard(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
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
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
file_delnewguard(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	int result = -1;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
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
done:
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
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
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
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_includecount(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
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
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_readcount(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_TEXT) {
			err_not_a_textfile(file);
		} else {
			result = DeeInt_NewSize(file->f_textfile.f_rdata);
		}
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_getdisallowguard(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
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
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
file_deldisallowguard(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	int result = -1;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
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
done:
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
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_TEXT) {
			err_not_a_textfile(file);
		} else {
			if (newval) {
				file->f_textfile.f_flags |= TPP_TEXTFILE_FLAG_NOGUARD;
			} else {
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
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
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
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
file_delissystemheader(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	int result = -1;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
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
done:
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
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_TEXT) {
			err_not_a_textfile(file);
		} else {
			if (newval) {
				file->f_textfile.f_flags |= TPP_TEXTFILE_FLAG_SYSHEADER;
			} else {
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
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
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
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
file_delnonblocking(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	int result = -1;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
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
done:
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
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_TEXT) {
			err_not_a_textfile(file);
		} else {
			if (newval) {
				file->f_textfile.f_flags |= TPP_TEXTFILE_FLAG_NONBLOCK;
			} else {
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
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		result = DeeBool_For(file->f_kind == TPPFILE_KIND_MACRO &&
		                     (file->f_macro.m_flags & TPP_MACROFILE_KIND_FUNCTION));
		Dee_Incref(result);
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_iskeywordmacro(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		result = DeeBool_For(file->f_kind == TPPFILE_KIND_MACRO &&
		                     !(file->f_macro.m_flags & TPP_MACROFILE_KIND_FUNCTION));
		Dee_Incref(result);
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_definitionsfile(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_MACRO) {
			err_not_a_macrofile(file);
		} else if (!file->f_macro.m_deffile) {
			result = DeeNone_NewRef();
		} else {
			result = DeeCompiler_GetFile(file->f_macro.m_deffile);
		}
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_definitionsposition(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_MACRO) {
			err_not_a_macrofile(file);
		} else if (!file->f_macro.m_deffile) {
			result = DeeNone_NewRef();
		} else {
			result = DeeTuple_Newf("dd",
			                       (int)file->f_macro.m_defloc.lc_line,
			                       (int)file->f_macro.m_defloc.lc_col);
		}
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_previousdefinition(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_MACRO) {
			err_not_a_macrofile(file);
		} else if (!file->f_macro.m_pushprev) {
			result = DeeNone_NewRef();
		} else {
			result = DeeCompiler_GetFile(file->f_macro.m_pushprev);
		}
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_pushcount(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_MACRO) {
			err_not_a_macrofile(file);
		} else {
			result = DeeInt_NewSize(file->f_macro.m_pushcount);
		}
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_keywordexpandorigin(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
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
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_isvariadicmacro(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		result = DeeBool_For(file->f_kind == TPPFILE_KIND_MACRO &&
		                     (file->f_macro.m_flags & (TPP_MACROFILE_KIND_FUNCTION | TPP_MACROFILE_FLAG_FUNC_VARIADIC)) ==
		                     (TPP_MACROFILE_KIND_FUNCTION | TPP_MACROFILE_FLAG_FUNC_VARIADIC));
		Dee_Incref(result);
	}
	COMPILER_END();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_getallowselfexpansion(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
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
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
file_delallowselfexpansion(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	int result = -1;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
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
done:
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
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
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
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
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
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
file_delkeepargumentspace(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	int result = -1;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
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
done:
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
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
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
	DREF DeeObject *result = NULL;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
	file = DeeCompilerItem_VALUE(self, struct TPPFile);
	if likely(file) {
		if (file->f_kind != TPPFILE_KIND_MACRO ||
		    !(file->f_macro.m_flags & TPP_MACROFILE_KIND_FUNCTION)) {
			err_not_a_functionmacrofile(file);
		} else {
			char ch;
			switch (file->f_macro.m_flags & TPP_MACROFILE_MASK_FUNC_STARTCH) {

			case TPP_MACROFILE_FUNC_START_LANGLE: 
				ch = '<';
				break;

			case TPP_MACROFILE_FUNC_START_LBRACE:
				ch = '{';
				break;

			case TPP_MACROFILE_FUNC_START_LBRACKET:
				ch = '[';
				break;

			default:
				ch = '(';
				break;
			}
			result = DeeString_Chr((uint8_t)ch);
		}
	}
	COMPILER_END();
done:
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
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
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
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
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
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_functionmacroexpansions(DeeCompilerItemObject *__restrict self) {
	struct TPPFile *file;
	DREF DeeObject *result = NULL;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
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
done:
	return result;
}


PRIVATE struct type_getset tpconst file_getsets[] = {
	TYPE_GETTER("istext", &file_istext,
	            "->?Dbool\n"
	            "Returns ?t if @this File is a text-file (as opposed to the result "
	            /**/ "of macro expansion (s.a. ?#ismacro), or some other kind of file)"),
	TYPE_GETTER("ismacro", &file_ismacro,
	            "->?Dbool\n"
	            "Returns ?t if @this File is a macro-file (as opposed to "
	            /**/ "a text-file (s.a. ?#istext), or some other kind of file)"),
	TYPE_GETTER("isexpanded", &file_isexpand,
	            "->?Dbool\n"
	            "Returns ?t if @this File is an expanded function-like macro-file, "
	            /**/ "or the result of injecting custom text into the token stream\n"
	            "Note that functions and getsets related to ?#ismacro cannot be used "
	            /**/ "when ?#isexpanded is ?t, and that ?#ismacro will return ?f when "
	            /**/ "this field evaluates to ?t"),
	TYPE_GETTER("origin", &file_origin,
	            "->?X2?.?N\n"
	            "Returns the originating location of @this File, "
	            /**/ "or ?N if @this File is the base-file"),
	TYPE_GETTER("alltext", &file_alltext,
	            "->?Dstring\n"
	            "Returns the currently loaded text data of @this File"),
	TYPE_GETTER("nexttext", &file_nexttext,
	            "->?Dstring\n"
	            "Returns sub-portion of text that will be used when reading from the file continues"),
	TYPE_GETTER("position", &file_position,
	            "->?T2?Dint?Dint\n"
	            "Returns the current position (as a pair of integer `line, column', both of which "
	            /**/ "are zero-based, meaning you'll probably have to add ${+1} to get line numbers as "
	            /**/ "they would be used in a text editor)\n"
	            "In the event that @this File is a macro, the positions returned refer to the "
	            /**/ "macro declaration position"),
	TYPE_GETSET(STR_filename,
	            &file_filename,
	            &file_delfilename,
	            &file_setfilename,
	            "->?Dstring\n"
	            "#tValueError{Attempted to delete, or set the filename of a non-text file}"
	            "Returns the filename of a text file, or the filename of the file containing the "
	            /**/ "definition of the @this macro\n"
	            "In the event that a ${##line} directive was used to override the filename, the "
	            /**/ "overwritten name is returned. If this isn't intended, use ?#realfilename instead\n"
	            "In text-files, this field may be written to override the current file name, while "
	            /**/ "deleting it will restore ?#realfilename as the used filename"),
	TYPE_GETTER("realfilename", &file_realfilename,
	            "->?Dstring\n"
	            "Returns the real filename of a text file, or the real filename of the file "
	            /**/ "containing the definition of the @this macro\n"
	            "This is the original, real filename, whereas the name returned by ?#Filename "
	            /**/ "is the one which may have been overwritten by a ${##line} directive"),
	TYPE_GETTER("name", &file_name,
	            "->?Dstring\n"
	            "Returns the name of @this File, that is the filename, or in the event of @this "
	            /**/ "file being a macro, the name of that macro"),
	TYPE_GETSET("lineoffset",
	            &file_lineoffset,
	            &file_dellineoffset,
	            &file_setlineoffset,
	            "->?Dint\n"
	            "#tValueError{@this File isn't a text file (?#istext is ?f)}"
	            "Get, del(set to zero), or set the line-offset within @this "
	            /**/ "text file, as can also be set by the ?#line directive"),
	TYPE_GETTER("stream", &file_stream,
	            "->?DFile\n"
	            "#tValueError{@this File isn't a text file (?#istext is ?f)}"
	            "Returns the file stream from which data is read into @this File"),
	TYPE_GETSET("guard",
	            &file_getguard,
	            &file_delguard,
	            &file_setguard,
	            "->?X2" DR_CKeyword "?N\n"
	            "#tValueError{@this File isn't a text file (?#istext is ?f)}"
	            "Get, delete, or set a keyword that is checked for being defined "
	            /**/ "before allowing @this File to be included by ${##include} again\n"
	            "In the event of the keyword having an associated macro that is also "
	            /**/ "defined, the file will not be included, but simply be skipped.\n"
	            "Setting ?N is the same as deleting the guard, and ?N is returned if no guard is set"),
	TYPE_GETSET("newguard",
	            &file_getnewguard,
	            &file_delnewguard,
	            &file_setnewguard,
	            "->?X2" DR_CKeyword "?N\n"
	            "#tValueError{@this File isn't a text file (?#istext is ?f)}"
	            "Get, delete, or set a keyword that will be set as ?#guard (if no guard has "
	            /**/ "already been set) once @this File is popped from the ${##include}-stack, and "
	            /**/ "?#disallowguard is ?f"),
	TYPE_GETTER("includecount", &file_includecount,
	            "->?Dint\n"
	            "#tValueError{@this File isn't a text file (?#istext is ?f)}"
	            "Return the number of times that @this File exists within the ${##include}-stack"),
	TYPE_GETTER("readcount", &file_readcount,
	            "->?Dint\n"
	            "#tValueError{@this File isn't a text file (?#istext is ?f)}"
	            "Returns the number of bytes already read from the underlying source stream"),
	TYPE_GETSET("disallowguard",
	            &file_getdisallowguard,
	            &file_deldisallowguard,
	            &file_setdisallowguard,
	            "->?Dbool\n"
	            "#tValueError{@this File isn't a text file (?#istext is ?f)}"
	            "Get, del (set to false), or set @this File's disallow-guard property.\n"
	            "When set to ?f, ?#newguard will be applied as ?#guard when the file is "
	            /**/ "popped, allowing the lexer to remember a potential file guard.\n"
	            "This flag is set to ?t automatically when an outer-most ${##ifndef}-block "
	            /**/ "ends, following which more non-whitespace text is encountered, thus "
	            /**/ "preventing the creation of a guard for the file"),
	TYPE_GETSET("issystemheader",
	            &file_getissystemheader,
	            &file_delissystemheader,
	            &file_setissystemheader,
	            "->?Dbool\n"
	            "#tValueError{@this File isn't a text file (?#istext is ?f)}"
	            "Get, del (set to false), or set if @this File is considered a system header\n"
	            "When ?t, all non-error warnings are suppressed\n"
	            "This flag is usually set by a ${##pragma GCC system_header} directive"),
	TYPE_GETSET("nonblocking",
	            &file_getnonblocking,
	            &file_delnonblocking,
	            &file_setnonblocking,
	            "->?Dbool\n"
	            "#tValueError{@this File isn't a text file (?#istext is ?f)}"
	            "Get, del (set to false), or set if the underlying stream allows "
	            /**/ "for non-blocking I/O, and should be performed in non-blocking mode "
	            /**/ "when ?#nextchunk is called with $nonblocking set to ?t and when "
	            /**/ "no incomplete string, or comment exists at the end of currently "
	            /**/ "loaded text"),
	TYPE_GETTER("isfunctionmacro", &file_isfunctionmacro,
	            "->?Dbool\n"
	            "Returns ?t if @this File is a function-like macro-file\n"
	            "Note that in this case, ?#ismacro will also return ?t"),
	TYPE_GETTER("iskeywordmacro", &file_iskeywordmacro,
	            "->?Dbool\n"
	            "Returns ?t if @this File is a keyword-like macro-file\n"
	            "Note that in this case, ?#ismacro will also return ?t"),
	TYPE_GETTER("definitionsfile", &file_definitionsfile,
	            "->?X2?.?N\n"
	            "#tValueError{@this File isn't a macro file (?#ismacro is ?f)}"
	            "Returns the file that was used to define @this macro, or return ?N if "
	            /**/ "the macro was defined through other means, such as via the commandline"),
	TYPE_GETTER("definitionsposition", &file_definitionsposition,
	            "->?X2?T2?Dint?Dint?N\n"
	            "#tValueError{@this File isn't a macro file (?#ismacro is ?f)}"
	            "Return the (line, column) pair of the definition location of @this macro file\n"
	            "Macros not defined through files will return ${(0, 0)}"),
	TYPE_GETTER("previousdefinition", &file_previousdefinition,
	            "->?X2?.?N\n"
	            "#tValueError{@this File isn't a macro file (?#ismacro is ?f)}"
	            "Return the previous definition of pushed macro (as created by ${##pragma push_macro(\"foo\")})\n"
	            "If the macro hasn't been pushed, or is the oldest variant, ?N is returned"),
	TYPE_GETTER("pushcount", &file_pushcount,
	            "->?Dint\n"
	            "#tValueError{@this File isn't a macro file (?#ismacro is ?f)}"
	            "The amount of times ${##pragma push_macro(\"foo\")} was repeated without actually "
	            /**/ "providing a new definition of the macro. Used to handle recursive use of that pragma"),
	TYPE_GETTER("keywordexpandorigin", &file_keywordexpandorigin,
	            "->?.\n"
	            "#tValueError{@this File isn't a keyword-like macro file (?#iskeywordmacro is ?f)}"
	            "The originating file of a keyword-like macro"),
	TYPE_GETTER("isvariadicmacro", &file_isvariadicmacro,
	            "->?Dbool\n"
	            "Returns ?t if @this File is a function-like macro-file (s.a. "
	            /**/ "?#isfunctionmacro) taking a variable number of arguments"),
	TYPE_GETSET("allowselfexpansion",
	            &file_getallowselfexpansion,
	            &file_delallowselfexpansion,
	            &file_setallowselfexpansion,
	            "->?Dbool\n"
	            "#tValueError{@this File isn't a function-like macro file (?#isfunctionmacro is ?f)}"
	            "Get, del (set to ?f), or set if @this function-like macro is allowed to expand to itself\n"
	            "This flag is set for newly defined macros when the $\"macro-recursion\" extension is enabled, "
	            /**/ "and is cleared when that extension is disabled (default)\n"
	            "When set, the macro's body may contain another reference to the function itself, which is then "
	            /**/ "expanded again, so-long as the arguments passed differ from all expansions that are already "
	            /**/ "apart of the current macro-expansion (${##include}) stack."),
	TYPE_GETSET("keepargumentspace",
	            &file_getkeepargumentspace,
	            &file_delkeepargumentspace,
	            &file_setkeepargumentspace,
	            "->?Dbool\n"
	            "#tValueError{@this File isn't a function-like macro file (?#isfunctionmacro is ?f)}"
	            "Get, del (set to ?f), or set if whitespace surrounding arguments passed "
	            /**/ "to @this function-like macro should be kept, or trimmed.\n"
	            "This flag is set for newly defined macros when the $\"macro-argument-whitespace\" extension "
	            /**/ "is enabled, and is cleared when that extension is disabled (default)\n"
	            "${"
	            /**/ "#pragma extension(\"-fno-macro-argument-whitespace\")\n"
	            /**/ "#define STR1(x) #x\n"
	            /**/ "#pragma extension(\"-fmacro-argument-whitespace\")\n"
	            /**/ "#define STR2(x) #x\n"
	            /**/ "print STR1(  foo  ); /* \"foo\" */\n"
	            /**/ "print STR2(  foo  ); /* \"  foo  \" */"
	            "}"),
	TYPE_GETSET("functionmacrovariant", &file_getfunctionmacrovariant, NULL, &file_setfunctionmacrovariant,
	            "->?Dstring\n"
	            "#tValueError{@this File isn't a function-like macro file (?#isfunctionmacro is ?f)}"
	            "#tValueError{Attempted to set a value not apart of ${(\"(\", \"[\", \"{\", \"<\")}}"
	            "Get or set the type of parenthesis used to start the argument list of @this function-like macro"),
	TYPE_GETTER("functionmacroargc", &file_functionmacroargc,
	            "->?Dint\n"
	            "#tValueError{@this File isn't a function-like macro file (?#isfunctionmacro is ?f)}"
	            "Return the number of (non-variadic) argument taken by @this function-like macro"),
	TYPE_GETTER("functionmacroexpansions", &file_functionmacroexpansions,
	            "->?Dint\n"
	            "#tValueError{@this File isn't a function-like macro file (?#isfunctionmacro is ?f)}"
	            "Return the number of times that @this function-like macro is being expanded\n"
	            "Note that normally only function-like macros with ?#allowselfexpansion set to ?t "
	            /**/ "can ever be expanded more than once at the same time"),
	TYPE_GETSET_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_nextchunk(DeeCompilerItemObject *self, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
	unsigned int flags     = 0;
	DREF DeeObject *result = NULL;
	int error;
	struct TPPFile *file;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("nextchunk", params: """
	bool extend      = false;
	bool binary      = false;
	bool nonblocking = false;
""", docStringPrefix: "file", err: "done");]]]*/
#define file_nextchunk_params "extend=!f,binary=!f,nonblocking=!f"
	struct {
		bool extend;
		bool binary;
		bool nonblocking;
	} args;
	args.extend = false;
	args.binary = false;
	args.nonblocking = false;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__extend_binary_nonblocking, "|bbb:nextchunk", &args))
		goto done;
/*[[[end]]]*/
	if (args.extend)
		flags |= TPPFILE_NEXTCHUNK_FLAG_EXTEND;
	if (args.binary)
		flags |= TPPFILE_NEXTCHUNK_FLAG_BINARY;
	if (args.nonblocking)
		flags |= TPPFILE_NEXTCHUNK_FLAG_NOBLCK;
	if (COMPILER_BEGIN(self->ci_compiler))
		goto done;
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

PRIVATE struct type_method tpconst file_methods[] = {
	TYPE_KWMETHOD("nextchunk", &file_nextchunk,
	              "(" file_nextchunk_params ")->?Dbool\n"
	              "#r{Returns ?t if data was read, or ?f if the EOF of the input stream has been reached}"
	              "Try to load the next, or @extend the current chunk of loaded data, by reading a "
	              /**/ "new chunk from ?#stream. When @binary is ?t, don't try to decode unicode data, "
	              /**/ "but read data as-is, without decoding it. When @nonblocking is ?t, and the "
	              /**/ "?#nonblocking is ?t as well, try to read data without blocking when waiting for "
	              /**/ "new data, thus potentially returning ?f, even when the actual end-of-file hasn't "
	              /**/ "been reached, yet"),
	TYPE_METHOD_END
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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeCompilerItemObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_str,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_str,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
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
