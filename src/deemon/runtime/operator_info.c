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
#ifndef GUARD_DEEMON_RUNTIME_OPERATOR_INFO_C
#define GUARD_DEEMON_RUNTIME_OPERATOR_INFO_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/cached-dict.h>
#include <deemon/computed-operators.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/kwds.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/operator-hints.h>
#include <deemon/seq.h>
#include <deemon/serial.h>
#include <deemon/set.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/system-features.h> /* memcpyc(), ... */
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

#include <hybrid/typecore.h>

#include "method-hint-defaults.h"
#include "strings.h"

#include <stdarg.h> /* va_end, va_list, va_start */
#include <stddef.h> /* NULL, offsetof, size_t */
#include <stdint.h> /* uintptr_t */

DECL_BEGIN

#undef byte_t
#define byte_t __BYTE_TYPE__

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

#define OPNAME(opname) "operator " opname

#ifndef CONFIG_HAVE_strcmp
#define CONFIG_HAVE_strcmp
#undef strcmp
#define strcmp dee_strcmp
DeeSystem_DEFINE_strcmp(dee_strcmp)
#endif /* !CONFIG_HAVE_strcmp */

typedef DeeTypeObject Type;

#define LENGTHOF_type_operators (OPERATOR_USERCOUNT + ((OPERATOR_PRIVMAX - OPERATOR_PRIVMIN) + 1))
INTDEF struct type_operator const type_operators[LENGTHOF_type_operators];

/* Lookup information about operator `id', as defined by `typetype'
 * Returns NULL if the given operator is not known.
 * NOTE: The given `typetype' must be a type-type, meaning it must
 *       be the result of `Dee_TYPE(Dee_TYPE(ob))', in order to return
 *       information about generic operators that can be used on `ob' */
PUBLIC ATTR_PURE WUNUSED NONNULL((1)) struct opinfo const *DCALL
DeeTypeType_GetOperatorById(DeeTypeObject const *__restrict typetype, Dee_operator_t id) {
	/* Fallback: select operator defined by the core "DeeType_Type".
	 * NOTE: This could be done by scanning its table using the below loop,
	 *       but since this is the most common case, it gets optimized here. */
	if (id < OPERATOR_USERCOUNT)
		return &type_operators[id].to_decl;
	if (id >= OPERATOR_PRIVMIN && id <= OPERATOR_PRIVMAX)
		return &type_operators[OPERATOR_USERCOUNT + id - OPERATOR_PRIVMIN].to_decl;

	/* Check for custom operators. */
	while (typetype != &DeeType_Type) {
		size_t lo = 0;
		size_t hi = typetype->tp_operators_size;
		ASSERT(DeeType_IsTypeType(typetype));
		while (lo < hi) {
			size_t mid = (lo + hi) / 2;
			struct type_operator const *info = &typetype->tp_operators[mid];
			if (id < info->to_id) {
				hi = mid;
			} else if (id > info->to_id) {
				lo = mid + 1;
			} else {
				/* Found operator info descriptor! */
				if likely(type_operator_isdecl(info))
					return &info->to_decl;
				/* Check if a neighboring slot might be what we're looking for... */
				if (mid > lo && info[-1].to_id == id) {
					do {
						--info;
						--mid;
						if likely(type_operator_isdecl(info))
							return &info->to_decl;
					} while (mid > lo && info[-1].to_id == id);
				}
				if ((mid + 1) < hi && info[1].to_id == id) {
					do {
						++info;
						++mid;
						if likely(type_operator_isdecl(info))
							return &info->to_decl;
					} while ((mid + 1) < hi && info[1].to_id == id);
				}
				goto next_base;
			}
		}
next_base:
		typetype = DeeType_Base(typetype);
	}
	return NULL;
}

/* Same as `DeeTypeType_GetOperatorById()', but also fill in `*p_declaring_type_type'
 * as the type-type that is declaring the operator "id". This can differ from "typetype"
 * in (e.g.) `DeeTypeType_GetOperatorByIdEx(&DeeFileType_Type, OPERATOR_BOOL)', where
 * `&DeeFileType_Type' is still able to implement "OPERATOR_BOOL", but the declaration
 * originates from `DeeType_Type', so in that case, `*p_declaring_type_type' is set to
 * `DeeType_Type', whereas for `FILE_OPERATOR_READ', it would be `DeeFileType_Type'
 * @param: p_declaring_type_type: [0..1] When non-null, store the declaring type here. */
PUBLIC WUNUSED ATTR_OUT_OPT(3) NONNULL((1)) struct Dee_opinfo const *DCALL
DeeTypeType_GetOperatorByIdEx(DeeTypeObject const *__restrict typetype, Dee_operator_t id,
                              DeeTypeObject **p_declaring_type_type) {
	if (p_declaring_type_type)
		*p_declaring_type_type = &DeeType_Type;
	if (id < OPERATOR_USERCOUNT)
		return &type_operators[id].to_decl;
	if (id >= OPERATOR_PRIVMIN && id <= OPERATOR_PRIVMAX)
		return &type_operators[OPERATOR_USERCOUNT + id - OPERATOR_PRIVMIN].to_decl;

	/* Check for custom operators. */
	while (typetype != &DeeType_Type) {
		size_t lo = 0;
		size_t hi = typetype->tp_operators_size;
		ASSERT(DeeType_IsTypeType(typetype));
		while (lo < hi) {
			size_t mid = (lo + hi) / 2;
			struct type_operator const *info = &typetype->tp_operators[mid];
			if (id < info->to_id) {
				hi = mid;
			} else if (id > info->to_id) {
				lo = mid + 1;
			} else {
				if (p_declaring_type_type)
					*p_declaring_type_type = (DeeTypeObject *)typetype;
				/* Found operator info descriptor! */
				if likely(type_operator_isdecl(info))
					return &info->to_decl;
				/* Check if a neighboring slot might be what we're looking for... */
				if (mid > lo && info[-1].to_id == id) {
					do {
						--info;
						--mid;
						if likely(type_operator_isdecl(info))
							return &info->to_decl;
					} while (mid > lo && info[-1].to_id == id);
				}
				if ((mid + 1) < hi && info[1].to_id == id) {
					do {
						++info;
						++mid;
						if likely(type_operator_isdecl(info))
							return &info->to_decl;
					} while ((mid + 1) < hi && info[1].to_id == id);
				}
				goto next_base;
			}
		}
next_base:
		typetype = DeeType_Base(typetype);
	}
	return NULL;
}


/* Same as `DeeTypeType_GetOperatorById()', but lookup operators by `oi_sname'
 * or `oi_uname' (though `oi_uname' only when that name isn't ambiguous).
 * @param: argc: The number of extra arguments taken by the operator (excluding
 *               the "this"-argument), or `(size_t)-1' if unknown. */
PUBLIC ATTR_PURE WUNUSED NONNULL((1, 2)) struct opinfo const *DCALL
DeeTypeType_GetOperatorByName(DeeTypeObject const *__restrict typetype,
                              char const *__restrict name, size_t argc) {
	size_t i;
	struct opinfo const *result;
#ifndef __OPTIMIZE_SIZE__
#define EQAT(ptr, str) (bcmp(ptr, str, sizeof(str)) == 0)
#define RETURN(name)   return &type_operators[name].to_decl
	switch (*name) {

	case '.':
		if (name[1] == '=' && !name[2])
			RETURN(OPERATOR_SETATTR);
		if (argc == 2)
			RETURN(OPERATOR_SETATTR);
		if (argc == 1)
			RETURN(OPERATOR_GETATTR);
		break; /* Ambiguous */

	case '[':
		if (name[1] == ']') {
			if (name[2] == '=' && !name[3])
				RETURN(OPERATOR_SETITEM);
			if (!name[2])
				RETURN(OPERATOR_GETITEM);
		}
		if (name[1] == ':' && name[2] == ']') {
			if (name[3] == '=' && !name[4])
				RETURN(OPERATOR_SETRANGE);
			if (!name[3])
				RETURN(OPERATOR_GETRANGE);
		}
		break; /* Ambiguous */

	case '+':
		if (name[1] == '=' && !name[2])
			RETURN(OPERATOR_INPLACE_ADD);
		if (name[1] == '+' && !name[2])
			RETURN(OPERATOR_INC);
		if (!name[1]) {
			if (argc == 1)
				RETURN(OPERATOR_ADD);
			if (argc == 0)
				RETURN(OPERATOR_POS);
		}
		break; /* Ambiguous */

	case '-':
		if (name[1] == '=' && !name[2])
			RETURN(OPERATOR_INPLACE_SUB);
		if (name[1] == '-' && !name[2])
			RETURN(OPERATOR_DEC);
		if (!name[1]) {
			if (argc == 1)
				RETURN(OPERATOR_SUB);
			if (argc == 0)
				RETURN(OPERATOR_NEG);
		}
		break; /* Ambiguous */

	case '~':
		if (!name[1])
			RETURN(OPERATOR_INV);
		break;

	case '*':
		if (!name[1])
			RETURN(OPERATOR_MUL);
		if (name[1] == '=' && !name[2])
			RETURN(OPERATOR_INPLACE_MUL);
		if (name[1] == '*') {
			if (!name[2])
				RETURN(OPERATOR_POW);
			if (name[2] == '=' && !name[3])
				RETURN(OPERATOR_INPLACE_POW);
		}
		break;

	case '/':
		if (!name[1])
			RETURN(OPERATOR_DIV);
		if (name[1] == '=' && !name[2])
			RETURN(OPERATOR_INPLACE_DIV);
		break;

	case '%':
		if (!name[1])
			RETURN(OPERATOR_MOD);
		if (name[1] == '=' && !name[2])
			RETURN(OPERATOR_INPLACE_MOD);
		break;

	case '&':
		if (!name[1])
			RETURN(OPERATOR_AND);
		if (name[1] == '=' && !name[2])
			RETURN(OPERATOR_INPLACE_AND);
		break;

	case '|':
		if (!name[1])
			RETURN(OPERATOR_OR);
		if (name[1] == '=' && !name[2])
			RETURN(OPERATOR_INPLACE_OR);
		break;

	case '^':
		if (!name[1])
			RETURN(OPERATOR_XOR);
		if (name[1] == '=' && !name[2])
			RETURN(OPERATOR_INPLACE_XOR);
		break;

	case ':':
		if (name[1] != '=')
			break;
		if (!name[2])
			RETURN(OPERATOR_ASSIGN);
		if (EQAT(name + 2, "move"))
			RETURN(OPERATOR_MOVEASSIGN);
		break;

	case '<':
		if (!name[1])
			RETURN(OPERATOR_LO);
		if (name[1] == '<') {
			if (!name[2])
				RETURN(OPERATOR_SHL);
			if (name[2] == '=' && !name[3])
				RETURN(OPERATOR_INPLACE_SHL);
		} else {
			if (name[1] == '=' && !name[2])
				RETURN(OPERATOR_LE);
		}
		break;

	case '>':
		if (!name[1])
			RETURN(OPERATOR_GR);
		if (name[1] == '>') {
			if (!name[2])
				RETURN(OPERATOR_SHR);
			if (name[2] == '=' && !name[3])
				RETURN(OPERATOR_INPLACE_SHR);
		} else {
			if (name[1] == '=' && !name[2])
				RETURN(OPERATOR_GE);
		}
		break;

	case '=':
		if (name[1] == '=' && !name[2])
			RETURN(OPERATOR_EQ);
		break;

	case '!':
		if (name[1] == '=' && !name[2])
			RETURN(OPERATOR_NE);
		break;

	case '#':
		if (!name[1])
			RETURN(OPERATOR_SIZE);
		break;

	case 'a':
		if (EQAT(name + 1, "ssign"))
			RETURN(OPERATOR_ASSIGN);
		if (EQAT(name + 1, "dd"))
			RETURN(OPERATOR_ADD);
		if (EQAT(name + 1, "nd"))
			RETURN(OPERATOR_AND);
		break;

	case 'b':
		if (EQAT(name + 1, "ool"))
			RETURN(OPERATOR_BOOL);
		break;

	case 'c':
		if (EQAT(name + 1, "opy"))
			RETURN(OPERATOR_COPY);
		if (EQAT(name + 1, "all"))
			RETURN(OPERATOR_CALL);
		if (EQAT(name + 1, "ontains"))
			RETURN(OPERATOR_CONTAINS);
		break;

	case 'd':
		if (EQAT(name + 1, "iv"))
			RETURN(OPERATOR_DIV);
		if (name[1] == 'e') {
			if (EQAT(name + 2, "epcopy"))
				RETURN(OPERATOR_DEEPCOPY);
			if (EQAT(name + 2, "structor"))
				RETURN(OPERATOR_DESTRUCTOR);
			if (name[2] == 'c' && !name[3])
				RETURN(OPERATOR_DEC);
			if (name[2] == 'l') {
				if (name[3] == '[') {
					if (name[4] == ']' && !name[5])
						RETURN(OPERATOR_DELITEM);
					if (name[4] == ':' && name[5] == ']' && !name[6])
						RETURN(OPERATOR_DELRANGE);
				}
				if (name[3] == '.' && !name[4])
					RETURN(OPERATOR_DELATTR);
				if (EQAT(name + 3, "item"))
					RETURN(OPERATOR_DELITEM);
				if (EQAT(name + 3, "range"))
					RETURN(OPERATOR_DELRANGE);
				if (EQAT(name + 3, "attr"))
					RETURN(OPERATOR_DELATTR);
			}
		}
		break;

	case 'e':
		if (name[1] == 'q' && !name[2])
			RETURN(OPERATOR_EQ);
		if (EQAT(name + 1, "numattr"))
			RETURN(OPERATOR_ENUMATTR);
		if (EQAT(name + 1, "nter"))
			RETURN(OPERATOR_ENTER);
		break;

	case 'f':
		if (EQAT(name + 1, "loat"))
			RETURN(OPERATOR_FLOAT);
		break;

	case 'g':
		if (name[1] == 'r' && !name[2])
			RETURN(OPERATOR_GR);
		if (name[1] == 'e') {
			if (!name[2])
				RETURN(OPERATOR_GE);
			if (name[2] == 't') {
				if (EQAT(name + 3, "item"))
					RETURN(OPERATOR_GETITEM);
				if (EQAT(name + 3, "range"))
					RETURN(OPERATOR_GETRANGE);
				if (EQAT(name + 3, "attr"))
					RETURN(OPERATOR_GETATTR);
			}
		}
		break;

	case 'h':
		if (EQAT(name + 1, "ash"))
			RETURN(OPERATOR_HASH);
		break;

	case 'i':
		if (name[1] == 't' && name[2] == 'e' && name[3] == 'r') {
			if (!name[4])
				RETURN(OPERATOR_ITER);
			if (EQAT(name + 4, "self"))
				RETURN(OPERATOR_ITER);
			if (EQAT(name + 4, "next"))
				RETURN(OPERATOR_ITERNEXT);
		} else {
			char const *iter;
			if (name[1] == 'n') {
				if (name[2] == 't' && !name[3])
					RETURN(OPERATOR_INT);
				if (name[2] == 'v' && !name[3])
					RETURN(OPERATOR_INV);
				if (name[2] == 'c' && !name[3])
					RETURN(OPERATOR_INC);
			}
			iter = name;
			++iter;
			if (iter[0] == 'n' && iter[1] == 'p' &&
			    iter[2] == 'l' && iter[3] == 'a' &&
			    iter[4] == 'c' && iter[5] == 'e') {
				iter += 6;
				if (iter[0] == '_')
					++iter;
			}
			switch (iter[0]) {

			case 'a':
				if (iter[2] != 'd')
					break;
				if (iter[1] == 'd' && !iter[3])
					RETURN(OPERATOR_INPLACE_ADD);
				if (iter[1] == 'n' && !iter[3])
					RETURN(OPERATOR_INPLACE_AND);
				break;

			case 's':
				if (iter[1] == 'u' && iter[2] == 'b' && !iter[3])
					RETURN(OPERATOR_INPLACE_SUB);
				if (iter[1] == 'h') {
					if (iter[2] == 'l' && !iter[3])
						RETURN(OPERATOR_INPLACE_SHL);
					if (iter[2] == 'r' && !iter[3])
						RETURN(OPERATOR_INPLACE_SHR);
				}
				break;

			case 'm':
				if (iter[1] == 'u' && iter[2] == 'l' && !iter[3])
					RETURN(OPERATOR_INPLACE_MUL);
				if (iter[1] == 'o' && iter[2] == 'd' && !iter[3])
					RETURN(OPERATOR_INPLACE_MOD);
				break;

			case 'd':
				if (iter[1] == 'i' && iter[2] == 'v' && !iter[3])
					RETURN(OPERATOR_INPLACE_DIV);
				break;

			case 'o':
				if (iter[1] == 'r' && !iter[2])
					RETURN(OPERATOR_INPLACE_OR);
				break;

			case 'x':
				if (iter[1] == 'o' && iter[2] == 'r' && !iter[3])
					RETURN(OPERATOR_INPLACE_XOR);
				break;

			case 'p':
				if (iter[1] == 'o' && iter[2] == 'w' && !iter[3])
					RETURN(OPERATOR_INPLACE_POW);
				break;

			default: break;
			}
		}
		break;

	case 'l':
		if (name[1] == 'o' && !name[2])
			RETURN(OPERATOR_LO);
		if (name[1] == 'e') {
			if (!name[2])
				RETURN(OPERATOR_LE);
			if (EQAT(name + 2, "ave"))
				RETURN(OPERATOR_LEAVE);
		}
		break;

	case 'm':
		if (EQAT(name + 1, "oveassign"))
			RETURN(OPERATOR_MOVEASSIGN);
		if (EQAT(name + 1, "ul"))
			RETURN(OPERATOR_MUL);
		if (EQAT(name + 1, "od"))
			RETURN(OPERATOR_MOD);
		break;

	case 'n':
		if (name[1] == 'e') {
			if (!name[2])
				RETURN(OPERATOR_NE);
			if (name[2] == 'g' && !name[3])
				RETURN(OPERATOR_NEG);
			if (name[2] == 'x' && name[3] == 't' && !name[4])
				RETURN(OPERATOR_ITERNEXT);
		}
		break;

	case 'o':
		if (name[1] == 'r' && !name[2])
			RETURN(OPERATOR_OR);
		break;

	case 'p':
		if (name[1] == 'o' && !name[3]) {
			if (name[2] == 's')
				RETURN(OPERATOR_POS);
			if (name[2] == 'w')
				RETURN(OPERATOR_POW);
		}
		break;

	case 'r':
		if (EQAT(name + 1, "epr"))
			RETURN(OPERATOR_REPR);
		break;

	case 's':
		if (EQAT(name + 1, "tr"))
			RETURN(OPERATOR_STR);
		if (EQAT(name + 1, "ub"))
			RETURN(OPERATOR_SUB);
		if (EQAT(name + 1, "hl"))
			RETURN(OPERATOR_SHL);
		if (EQAT(name + 1, "hr"))
			RETURN(OPERATOR_SHR);
		if (EQAT(name + 1, "ize"))
			RETURN(OPERATOR_SIZE);
		if (name[1] == 'e' && name[2] == 't') {
			if (EQAT(name + 3, "item"))
				RETURN(OPERATOR_SETITEM);
			if (EQAT(name + 3, "range"))
				RETURN(OPERATOR_SETRANGE);
			if (EQAT(name + 3, "attr"))
				RETURN(OPERATOR_SETATTR);
		}
		break;

	case 'x':
		if (EQAT(name + 1, "or"))
			RETURN(OPERATOR_XOR);
		break;

	default: break;
	}
#undef RETURN
#undef EQAT
#endif /* !__OPTIMIZE_SIZE__ */

	/* Check for custom operators. */
	ASSERT(DeeType_IsTypeType(typetype));
	while (typetype != &DeeType_Type) {
		for (i = 0; i < typetype->tp_operators_size; ++i) {
			struct type_operator const *info = &typetype->tp_operators[i];
			if (type_operator_isdecl(info)) {
				if (strcmp(info->to_decl.oi_sname, name) == 0)
					return &info->to_decl;
			}
		}
		result = NULL;
		for (i = 0; i < typetype->tp_operators_size; ++i) {
			struct type_operator const *info = &typetype->tp_operators[i];
			if (type_operator_isdecl(info)) {
				if (strcmp(info->to_decl.oi_uname, name) == 0) {
					if (result)
						return NULL; /* Ambiguous name */
					if (argc == (size_t)OPCC_ARGC(info->to_decl.oi_cc) || argc == (size_t)-1)
						result = &info->to_decl;
				}
			}
		}
		if (result)
			return result;
		typetype = typetype->tp_base;
	}

	/* Fallback: scan the operator table of `DeeType_Type' */
	for (i = 0; i < COMPILER_LENOF(type_operators); ++i) {
		struct opinfo const *info = &type_operators[i].to_decl;
		if (strcmp(info->oi_sname, name) == 0)
			return info;
	}
	result = NULL;
	for (i = 0; i < COMPILER_LENOF(type_operators); ++i) {
		struct opinfo const *info = &type_operators[i].to_decl;
		if (strcmp(info->oi_uname, name) == 0) {
			if (result)
				return NULL; /* Ambiguous name */
			if (argc == (size_t)OPCC_ARGC(info->oi_cc) || argc == (size_t)-1)
				result = info;
		}
	}
	return result;
}

PUBLIC ATTR_PURE WUNUSED ATTR_INS(2, 3) NONNULL((1)) struct opinfo const *DCALL
DeeTypeType_GetOperatorByNameLen(DeeTypeObject const *__restrict typetype,
                                 char const *__restrict name, size_t namelen,
                                 size_t argc) {
#define LENGTHOF__opinfo__oi_sname COMPILER_LENOF(((struct opinfo *)0)->oi_sname)
#define LENGTHOF__opinfo__oi_uname COMPILER_LENOF(((struct opinfo *)0)->oi_uname)
	char buf[(LENGTHOF__opinfo__oi_sname > LENGTHOF__opinfo__oi_uname
	          ? LENGTHOF__opinfo__oi_sname
	          : LENGTHOF__opinfo__oi_uname) +
	         1];
	if (namelen >= COMPILER_LENOF(buf))
		return NULL; /* No valid operator has that long of a name... */
	*(char *)mempcpyc(buf, name, namelen, sizeof(char)) = '\0';
	return DeeTypeType_GetOperatorByName(typetype, buf, argc);
#undef LENGTHOF__opinfo__oi_sname
#undef LENGTHOF__opinfo__oi_uname
}

#define DeeType_GetPrivateCustomOperatorById(type, id) \
	type_operator_table_get_custom_operator_by_id((self)->tp_operators, (self)->tp_operators_size, id)
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) struct type_operator const *DCALL
type_operator_table_get_custom_operator_by_id(struct type_operator const *table,
                                              size_t count, Dee_operator_t id) {
	/* Check for custom operators. */
	size_t lo = 0;
	size_t hi = count;
	while (lo < hi) {
		size_t mid = (lo + hi) / 2;
		struct type_operator const *info = &table[mid];
		if (id < info->to_id) {
			hi = mid;
		} else if (id > info->to_id) {
			lo = mid + 1;
		} else {
			/* Found operator info descriptor! */
			if likely(type_operator_iscustom(info))
				return info;
			/* Check if a neighboring slot might be what we're looking for... */
			if (mid > lo && info[-1].to_id == id) {
				do {
					--info;
					--mid;
					if likely(type_operator_iscustom(info))
						return info;
				} while (mid > lo && info[-1].to_id == id);
			}
			if ((mid + 1) < hi && info[1].to_id == id) {
				do {
					++info;
					++mid;
					if likely(type_operator_iscustom(info))
						return info;
				} while ((mid + 1) < hi && info[1].to_id == id);
			}
			goto nope;
		}
	}
nope:
	return NULL;
}



PRIVATE struct type_operator const type_operator_flags[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_0006_STR, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0007_REPR, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0028_HASH, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0029_EQ, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST),
	TYPE_OPERATOR_FLAGS(OPERATOR_002A_NE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST),
};

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
noop_custom_operator_cb(DeeTypeObject *tp_self, DeeObject *self,
                        /*0..1*/ DeeObject **p_self,
                        size_t argc, DeeObject *const *argv,
                        Dee_operator_t opname) {
	(void)tp_self;
	(void)self;
	(void)p_self;
	(void)argc;
	(void)argv;
	(void)opname;
	return_none;
}

PRIVATE struct type_operator tpconst noop_custom_operator =
TYPE_OPERATOR_CUSTOM(0, &noop_custom_operator_cb, METHOD_FNOTHROW | METHOD_FCONSTCALL);

/* Check if "self" is defining a custom descriptor for "id", and if so, return it. */
PUBLIC ATTR_PURE WUNUSED NONNULL((1)) struct type_operator const *DCALL
DeeType_GetCustomOperatorById(DeeTypeObject const *__restrict self, Dee_operator_t id) {
	struct type_operator const *result;

	/* Look at the type from which the given operator "id" originates,
	 * since only *it* can specify the flags for said operator. */
	self = DeeType_GetOperatorOrigin(self, id);
	if (!self)
		return NULL;
	result = DeeType_GetPrivateCustomOperatorById(self, id);
	if (result)
		return result;

	/* Special case when querying for "DeeType_Type" (which can't define method
	 * flags for the operators it *itself* implements (e.g. `type_str'), since
	 * its list "type_operators" is a special case that needs to be linear) */
	if (self == &DeeType_Type) {
		return type_operator_table_get_custom_operator_by_id(type_operator_flags,
		                                                     COMPILER_LENOF(type_operator_flags),
		                                                     id);
	}

	/* "none", being the ultimate no-op, provides custom overrides for *any* operator. */
	if (self == &DeeNone_Type) {
		(void)id; /* Ignored! */
		return &noop_custom_operator;
	}

	return NULL;
}


/* Lookup per-type method flags that may be defined for "opname".
 * IMPORTANT: When querying the flags for `OPERATOR_ITER', the `Dee_METHOD_FCONSTCALL',
 *            `Dee_METHOD_FPURECALL', and `Dee_METHOD_FNOREFESCAPE' flags doesn't mean that
 *            you can call `operator iter()' at compile-time. Instead, it means that
 *            *enumerating* the object can be done at compile-time (so-long as the associated
 *            iterator is never exposed). Alternatively, think of this case as allowing a
 *            call to `DeeObject_Foreach()' at compile-time.
 * @return: * : Set of `Dee_METHOD_F*' describing special optimizations possible for "opname".
 * @return: Dee_METHOD_FNORMAL: No special flags are defined for "opname" (or "opname" doesn't have special flags) */
PUBLIC ATTR_PURE WUNUSED NONNULL((1)) uintptr_t DCALL
DeeType_GetOperatorFlags(DeeTypeObject const *__restrict self,
                         Dee_operator_t opname) {
	struct type_operator const *result;
	result = DeeType_GetCustomOperatorById(self, opname);
	if (result)
		return result->to_custom.s_flags;
	/* TODO: Re-write this function for CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

	/* Check for special "default" operators that use other operators for implementation. */
	if (DeeType_InheritOperator((DeeTypeObject *)self, opname)) {
		Dee_operator_t effective_opname;
		switch (opname) {
		case OPERATOR_EQ:
			if (self->tp_cmp->tp_eq == &default__eq__with__ne) {
				effective_opname = OPERATOR_NE;
check_effective_opname:
				result = DeeType_GetCustomOperatorById(self, effective_opname);
				if (result)
					return result->to_custom.s_flags;
			}
			break;
		case OPERATOR_NE:
			if (self->tp_cmp->tp_ne == &default__ne__with__eq) {
				effective_opname = OPERATOR_EQ;
				goto check_effective_opname;
			}
			break;
		case OPERATOR_LO:
			if (self->tp_cmp->tp_lo == &default__lo__with__ge) {
				effective_opname = OPERATOR_GE;
				goto check_effective_opname;
			}
			break;
		case OPERATOR_LE:
			if (self->tp_cmp->tp_le == &default__le__with__gr) {
				effective_opname = OPERATOR_GR;
				goto check_effective_opname;
			}
			break;
		case OPERATOR_GR:
			if (self->tp_cmp->tp_gr == &default__gr__with__le) {
				effective_opname = OPERATOR_LE;
				goto check_effective_opname;
			}
			break;
		case OPERATOR_GE:
			if (self->tp_cmp->tp_ge == &default__ge__with__lo) {
				effective_opname = OPERATOR_LO;
				goto check_effective_opname;
			}
			break;
		case OPERATOR_INT:
			if (self->tp_math->tp_int == &default__int__with__double) {
				effective_opname = OPERATOR_FLOAT;
				goto check_effective_opname;
			}
			break;
		case OPERATOR_FLOAT:
			if (self->tp_math->tp_double == &default__double__with__int ||
			    self->tp_math->tp_double == &default__double__with__int32 ||
			    self->tp_math->tp_double == &default__double__with__int64) {
				effective_opname = OPERATOR_INT;
				goto check_effective_opname;
			}
			break;

check_effective_opname_with_copy:
			result = DeeType_GetCustomOperatorById(self, opname);
			if (result) {
				uintptr_t flags = result->to_custom.s_flags;
				result = DeeType_GetCustomOperatorById(self, OPERATOR_COPY);
				if (result) {
					flags |= (result->to_custom.s_flags & Dee_METHOD_FNORETURN);
					flags &= (result->to_custom.s_flags | Dee_METHOD_FNORETURN);
					return flags;
				}
			}
			break;

#define HANDLE_MATH_FOO_WITH_INPLACE_FOO(NAME, name)                                            \
		case OPERATOR_INPLACE_##NAME:                                                           \
			if (self->tp_math->tp_inplace_##name == &default__inplace_##name##__with__##name) { \
				effective_opname = OPERATOR_##NAME;                                             \
				goto check_effective_opname_with_copy;                                          \
			}                                                                                   \
			break
		HANDLE_MATH_FOO_WITH_INPLACE_FOO(ADD, add);
		HANDLE_MATH_FOO_WITH_INPLACE_FOO(SUB, sub);
		HANDLE_MATH_FOO_WITH_INPLACE_FOO(MUL, mul);
		HANDLE_MATH_FOO_WITH_INPLACE_FOO(DIV, div);
		HANDLE_MATH_FOO_WITH_INPLACE_FOO(MOD, mod);
		HANDLE_MATH_FOO_WITH_INPLACE_FOO(SHL, shl);
		HANDLE_MATH_FOO_WITH_INPLACE_FOO(SHR, shr);
		HANDLE_MATH_FOO_WITH_INPLACE_FOO(AND, and);
		HANDLE_MATH_FOO_WITH_INPLACE_FOO(OR, or);
		HANDLE_MATH_FOO_WITH_INPLACE_FOO(XOR, xor);
		HANDLE_MATH_FOO_WITH_INPLACE_FOO(POW, pow);
#undef HANDLE_MATH_FOO_WITH_INPLACE_FOO

		default: break;
		}
	}

	/* Default flags for certain operators. */
	switch (opname) {

		/* TODO: Special handling when tp_ctor == &DeeNone_OperatorCtor -> CONSTEXPR */
		/* TODO: Special handling when tp_copy == &DeeNone_OperatorCopy -> CONSTEXPR */
		/* TODO: Special handling when tp_dtor == NULL -> CONSTEXPR */
		/* TODO: Special handling when ... */

	case OPERATOR_DESTRUCTOR:
	case OPERATOR_HASH:
	case OPERATOR_VISIT:
	case OPERATOR_CLEAR:
	case OPERATOR_PCLEAR:
		return METHOD_FNOTHROW;
	default: break;
	}
	return METHOD_FNORMAL;
}


/* Helper for checking that every cast-like operators is
 * either: not implemented, or marked as Dee_METHOD_FCONSTCALL:
 * >> (!DeeType_HasOperator(self, OPERATOR_BOOL) || (DeeType_GetOperatorFlags(self, OPERATOR_BOOL) & Dee_METHOD_FCONSTCALL)) &&
 * >> (!DeeType_HasOperator(self, OPERATOR_INT) || (DeeType_GetOperatorFlags(self, OPERATOR_INT) & Dee_METHOD_FCONSTCALL)) &&
 * >> (!DeeType_HasOperator(self, OPERATOR_FLOAT) || (DeeType_GetOperatorFlags(self, OPERATOR_FLOAT) & Dee_METHOD_FCONSTCALL)) &&
 * >> (!DeeType_HasOperator(self, OPERATOR_ITER) || (DeeType_GetOperatorFlags(self, OPERATOR_ITER) & Dee_METHOD_FCONSTCALL));
 * This is the condition that must be fulfilled by all arguments other than "this" when
 * a function uses "Dee_METHOD_FCONSTCALL_IF_ARGS_CONSTCAST" to make its CONSTCALL flag
 * conditional. */
PUBLIC ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
DeeType_IsConstCastable(DeeTypeObject const *__restrict self) {
	size_t i;
	PRIVATE Dee_operator_t const cast_operators[] = {
		OPERATOR_BOOL,
		OPERATOR_INT,
		OPERATOR_FLOAT,
		OPERATOR_ITER,
	};
	if (self->tp_features & (Dee_TF_NOTCONSTCASTABLE | Dee_TF_ISCONSTCASTABLE))
		return (self->tp_features & Dee_TF_ISCONSTCASTABLE) != 0;
	for (i = 0; i < COMPILER_LENOF(cast_operators); ++i) {
		Dee_operator_t id = cast_operators[i];
		uintptr_t opflags;
		if (!DeeType_HasOperator(self, id))
			continue;
		opflags = DeeType_GetOperatorFlags(self, id);
		if (!(opflags & Dee_METHOD_FCONSTCALL))
			goto nope;
	}
	atomic_or(&((DeeTypeObject *)self)->tp_features, Dee_TF_ISCONSTCASTABLE);
	return true;
nope:
	atomic_or(&((DeeTypeObject *)self)->tp_features, Dee_TF_NOTCONSTCASTABLE);
	return false;
}


PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) void const *DCALL
DeeType_GetOpPointer(DeeTypeObject const *__restrict self,
                     struct opinfo const *__restrict info) {
	ASSERT(info->oi_class != OPCLASS_CUSTOM);
	if (info->oi_class != OPCLASS_TYPE) {
		self = *(DeeTypeObject **)((uintptr_t)self + info->oi_class);
		if (self == NULL)
			return NULL;
	}
	return *(void const **)((uintptr_t)self + info->oi_offset);
}


/* Same as `DeeType_HasOperator()', however don't return `true' if the
 * operator has been inherited implicitly from a base-type of `self'. */
PUBLIC ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
DeeType_HasPrivateOperator(DeeTypeObject *__restrict self, Dee_operator_t name) {
	return DeeType_GetOperatorOrigin(self, name) == self;
}

/* Return the type from `self' inherited its operator `name'.
 * If `name' wasn't inherited, or isn't defined, simply re-return `self'.
 * Returns `NULL' when the operator isn't being implemented. */
PUBLIC ATTR_PURE WUNUSED NONNULL((1)) DeeTypeObject *DCALL
DeeType_GetOperatorOrigin(DeeTypeObject const *__restrict self, Dee_operator_t name) {
	void const *my_ptr;
	struct opinfo const *info;
	DeeTypeObject *base, *result;
	DeeTypeMRO mro;

	/* Check if the type even has the operator in question (this
	 * also causes it to be inherited if that wasn't done already) */
	if (!DeeType_HasOperator(self, name))
		return NULL;

	/* Special cases for certain operators. */
	switch (name) {
	case OPERATOR_CONSTRUCTOR:
	case OPERATOR_COPY:
	case OPERATOR_DEEPCOPY:
	case OPERATOR_DESTRUCTOR:
		return (DeeTypeObject *)self;

	default:
		if (name < Dee_OPERATOR_USERCOUNT) {
			enum Dee_tno_id tno_id = DeeType_GetTnoOfOperator(name);
			if ((unsigned int)tno_id < Dee_TNO_COUNT)
				return DeeType_GetNativeOperatorOrigin((DeeTypeObject *)self, tno_id);
		}
		break;
	}

	info = DeeTypeType_GetOperatorById(Dee_TYPE(self), name);
	if (info == NULL) {
		/* Not a standard operator. Check if it may be
		 * defined by the type or one of its true bases. */
		if (DeeType_GetPrivateCustomOperatorById(self, name) != NULL)
			return (DeeTypeObject *)self;
		base = DeeTypeMRO_Init(&mro, (DeeTypeObject *)self);
		while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
			if (DeeType_GetPrivateCustomOperatorById(base, name) != NULL)
				return base;
		}
		return NULL; /* No such operator */
	}
	my_ptr = DeeType_GetOpPointer(self, info);
	if (my_ptr == NULL)
		return NULL; /* Operator not implemented (Shouldn't get here...) */
	result = (DeeTypeObject *)self;
	base = DeeTypeMRO_Init(&mro, (DeeTypeObject *)self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		if (my_ptr == DeeType_GetOpPointer(base, info)) {
			result = base;
		} else {
			break;
		}
	}
	return result; /* Operator is distinct from all bases. */
}


PRIVATE WUNUSED NONNULL((1)) DeeTypeObject *DCALL
DeeType_GetOperatorContainerOrigin(DeeTypeObject *__restrict self,
                                   struct opinfo const *info,
                                   void const *container_pointer) {
	DeeTypeObject *base;
	DeeTypeMRO mro;
	ASSERT(info->oi_class != OPCLASS_TYPE);
	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		void *base_container = *(void **)((byte_t *)base + info->oi_class);
		if (base_container == container_pointer)
			return base;
	}
	return NULL;
}

/* Ensure that "self" is implementing "info", possibly inheriting it from a base type.
 * @return: true:  Success
 * @return: false: Error (the operator does not appear anywhere in `self.__mro__') */
PRIVATE WUNUSED NONNULL((1, 2, 3)) bool DCALL
DeeType_InheritGenericOperator(DeeTypeObject *__restrict self,
                               DeeTypeObject *type_type,
                               struct opinfo const *info) {
	/* Quick check: is the operator already implemented? */
	if (DeeType_GetOpPointer(self, info) != NULL)
		return true;
	if (info->oi_invoke->opi_inherit) {
		/* Custom inherit function was defined. */
		(*info->oi_invoke->opi_inherit)(self, type_type, info);
	} else {
		DeeTypeObject *base;
		DeeTypeMRO mro;
		/* Do a generic inherit. */
		base = DeeTypeMRO_Init(&mro, self);
		while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
			void const *base_ptr;
			if (!DeeObject_InstanceOf(base, type_type))
				continue; /* This base can't implement the necessary operator. */
			base_ptr = DeeType_GetOpPointer(base, info);
			if (base_ptr != NULL) {
				/* Found a base that is implementing this operator!
				 * -> inheirt it! */
				if (info->oi_class == OPCLASS_TYPE) {
					atomic_cmpxch_or_write((void **)((byte_t *)self + info->oi_offset), NULL, base_ptr);
				} else {
					DeeTypeObject *self_container_origin;
					void **p_self_container = (void **)((byte_t *)self + info->oi_class);
					void *base_container = *(void **)((byte_t *)base + info->oi_class);
					void *self_container = atomic_read(p_self_container);
					if (self_container == NULL) {
						atomic_cmpxch_or_write(p_self_container, NULL, base_container);
						self_container = atomic_read(p_self_container);
						return true;
					}
					ASSERT(self_container != NULL);
					ASSERT(base_container != NULL);
	
					/* Ensure that "self_container" isn't inherited. If it is, then we must inherit the
					 * operator not from the MRO of "self", but from that of the container's origin */
					self_container_origin = DeeType_GetOperatorContainerOrigin(self, info, self_container);
					if (self_container_origin != NULL && self_container_origin != self)
						return DeeType_InheritGenericOperator(self_container_origin, type_type, info);
					atomic_cmpxch_or_write((void **)((byte_t *)self_container + info->oi_offset), NULL, base_ptr);
				}
#if 1
				Dee_DPRINTF("[RT] Inherit `" OPNAME("%s") "' from %s into %s\n",
				            info->oi_sname, DeeType_GetName(base), DeeType_GetName(self));
#endif
				return true;
			}
		}
	}

	/* Re-check if the original type itself was already implementing the type. */
	return DeeType_GetOpPointer(self, info) != NULL;
}

/* For some reason, GCC things that "declaring_type_type" is uninitialized below, when it clearly isn't:
 * Warning: src/deemon/runtime/operator_info.c:1262:32: warning: ‘declaring_type_type’ may be used uninitialized [-Wmaybe-uninitialized]
 *  1262 |                         return DeeType_InheritGenericOperator(self, declaring_type_type, info);
 *       |                                ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * src/deemon/runtime/operator_info.c:1259:32: note: ‘declaring_type_type’ declared here
 *  1259 |                 DeeTypeObject *declaring_type_type;
 *       |                                ^~~~~~~~~~~~~~~~~~~
 */
__pragma_GCC_diagnostic_push_ignored(Wmaybe_uninitialized)

/* Check if the callback slot for `name' in `self' is populated.
 * If it isn't, then search the MRO of `self' for the first type
 * that *does* implement said operator, and cache that base's
 * callback in `self'
 * @return: true:  Either `self' already implemented the operator, it it was successfully inherited.
 * @return: false: `self' doesn't implement the operator, and neither does one of its bases. In this
 *                 case, trying to invoke the operator will result in a NotImplemented error. */
PUBLIC NONNULL((1)) bool DCALL
DeeType_InheritOperator(DeeTypeObject *__restrict self, Dee_operator_t name) {
	/* Some builtin operators require special handling in order to be inherited. */
	switch (name) {
	case OPERATOR_CONSTRUCTOR:
		return self->tp_init.tp_var.tp_ctor ||
		       self->tp_init.tp_var.tp_copy_ctor ||
		       self->tp_init.tp_var.tp_deep_ctor ||
		       self->tp_init.tp_var.tp_any_ctor ||
		       self->tp_init.tp_var.tp_any_ctor_kw ||
		       DeeType_InheritConstructors(self);
	case OPERATOR_GETBUF:
		return (self->tp_buffer && self->tp_buffer->tp_getbuf) ||
		       DeeType_InheritBuffer(self);
	default:
		if (name < Dee_OPERATOR_USERCOUNT) {
			enum Dee_tno_id tno_id = DeeType_GetTnoOfOperator(name);
			if ((unsigned int)tno_id < Dee_TNO_COUNT)
				return DeeType_GetNativeOperatorWithoutUnsupported((DeeTypeObject *)self, tno_id) != NULL;
		}
		break;
	}

	/* Try to inherit a non-standard operator (e.g. operators defined by "DeeFile_Type"). */
	{
		struct opinfo const *info;
		DeeTypeObject *declaring_type_type;
		info = DeeTypeType_GetOperatorByIdEx(Dee_TYPE(self), name, &declaring_type_type);
		if (info)
			return DeeType_InheritGenericOperator(self, declaring_type_type, info);
	}

	/* Check if "name" is implemented as a custom operator. */
	{
		struct type_operator const *info;
		info = DeeType_GetCustomOperatorById(self, name);
		if (info)
			return true; /* Operator is present, and it's custom. */
	}
	return false;
}

__pragma_GCC_diagnostic_pop_ignored(Wmaybe_uninitialized)


PRIVATE WUNUSED ATTR_INS(6, 5) NONNULL((1, 2)) DREF DeeObject *DCALL
invoke_operator(DeeTypeObject *tp_self, DeeObject *self, DREF DeeObject **p_self,
                Dee_operator_t name, size_t argc, DeeObject *const *argv) {
	/* Special case needed for super-wrappers. */
	if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		if (p_self) {
			DREF DeeObject *result;
			DeeObject *real_self = DeeSuper_SELF(self);
			Dee_Incref(real_self);
			ASSERT(self == *p_self);
			result = invoke_operator(tp_self, real_self, &real_self, name, argc, argv);
			ASSERT(self == *p_self);
			if unlikely(!result) {
				Dee_DecrefNokill(real_self);
			} else if (real_self == DeeSuper_SELF(self)) {
				Dee_DecrefNokill(real_self);
			} else {
				Dee_Decref(self);
				*p_self = real_self; /* Inherit reference */
			}
			return result;
		}
		self = DeeSuper_SELF(self);
	}

	/* Check for standard operators in the type-type of "self". */
	{
		struct opinfo const *info;
		info = DeeTypeType_GetOperatorById(Dee_TYPE(tp_self), name);
		if (info) {
			/* Invoke operator using "info" */
			ASSERT(info->oi_invoke);
			ASSERT(info->oi_invoke->opi_invoke);
			ASSERT(info->oi_class != OPCLASS_CUSTOM);
			/* Inherit the operator if necessary. */
			if (DeeType_GetOpPointer(tp_self, info) == NULL &&
			    /* Special case for "OPERATOR_CONSTRUCTOR", which is inherited abnormally,
			     * in that after being inherited, its OpPointer may still be `null'! */
			    likely(name != OPERATOR_CONSTRUCTOR &&
			           name != OPERATOR_GETATTR &&
			           name != OPERATOR_DELATTR &&
			           name != OPERATOR_SETATTR)) {
				if unlikely(!DeeType_InheritOperator(tp_self, name)) {
					/* Special case for operators with custom *__unsupported impls (like "operator hash")
					 * For those, we might get here if deemon was built without computed operators, in
					 * which case we must still invoke the *__unsupported impl like we normally would. */
					enum Dee_tno_id tno_id;
					if (name >= Dee_OPERATOR_USERCOUNT)
						goto err_not_implemented;
					tno_id = DeeType_GetTnoOfOperator(name);
					if ((unsigned int)tno_id >= (unsigned int)Dee_TNO_COUNT)
						goto err_not_implemented;
					if (DeeType_GetNativeOperatorUnsupported(tno_id) == NULL)
						goto err_not_implemented;
				}
			}
			return (*info->oi_invoke->opi_invoke)(tp_self, self, p_self, argc, argv, name);
err_not_implemented:
			DeeError_Throwf(&DeeError_NotImplemented,
			                "Operator `%r." OPNAME("%s") "' is not implemented",
			                tp_self, info->oi_sname);
			goto err;
		}
	}

	/* Check if this operator has a custom per-type implementation. */
	{
		struct type_operator const *
		info = DeeType_GetCustomOperatorById(tp_self, name);
		if (info && info->to_custom.s_invoke)
			return (*info->to_custom.s_invoke)(tp_self, self, p_self, argc, argv, name);
	}

	/* Error: unknown operator (TODO: Invoke "operators.operator()" here). */
	DeeError_Throwf(&DeeError_TypeError,
	                "Unknown operator #%X",
	                name);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
invoke_voperatorf(DeeTypeObject *tp_self, DeeObject *self, DREF DeeObject **p_self,
                  Dee_operator_t name, char const *__restrict format, va_list args) {
	DREF DeeObject *args_tuple, *result;
	args_tuple = DeeTuple_VNewf(format, args);
	if unlikely(!args_tuple)
		goto err;
	result = invoke_operator(tp_self, self, p_self, name,
	                         DeeTuple_SIZE(args_tuple),
	                         DeeTuple_ELEM(args_tuple));
	Dee_Decref_likely(args_tuple);
	return result;
err:
	return NULL;
}



/* Invoke an operator on a given object, given its ID and arguments.
 * NOTE: Using these function, any operator can be invoked, including
 *       extension operators as well as some operators marked as
 *       `OPCC_SPECIAL' (most notably: `tp_int'), as well as throwing
 *       a `Signal.StopIteration' when `tp_iter_next' is exhausted.
 * Operators marked as `oi_private' cannot be invoked and
 * attempting to do so will cause an `Error.TypeError' to be thrown.
 * Attempting to invoke an unknown operator will cause an `Error.TypeError' to be thrown.
 * HINT: `DeeObject_PInvokeOperator' can be used the same way `DeeObject_InvokeOperator'
 *        can be, with the addition of allowing inplace operators to be executed.
 *        Attempting to execute an inplace operator using `DeeObject_InvokeOperator()'
 *        will cause an `Error.TypeError' to be thrown. */
PUBLIC WUNUSED ATTR_INS(4, 3) NONNULL((1)) DREF DeeObject *DCALL
DeeObject_InvokeOperator(DeeObject *self, Dee_operator_t name,
                         size_t argc, DeeObject *const *argv) {
	return invoke_operator(Dee_TYPE(self), self, NULL, name, argc, argv);
}

PUBLIC WUNUSED ATTR_INS(5, 4) NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_TInvokeOperator(DeeTypeObject *tp_self, DeeObject *self,
                          Dee_operator_t name, size_t argc, DeeObject *const *argv) {
	ASSERT_OBJECT_TYPE(self, tp_self);
	return invoke_operator(tp_self, self, NULL, name, argc, argv);
}

PUBLIC WUNUSED ATTR_INS(4, 3) NONNULL((1)) DREF DeeObject *DCALL
DeeObject_PInvokeOperator(DREF DeeObject **__restrict p_self, Dee_operator_t name,
                          size_t argc, DeeObject *const *argv) {
	DeeObject *self = *p_self;
	return invoke_operator(Dee_TYPE(self), self, p_self, name, argc, argv);
}

PUBLIC WUNUSED ATTR_INS(5, 4) NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_PTInvokeOperator(DeeTypeObject *tp_self, DREF DeeObject **__restrict p_self,
                           Dee_operator_t name, size_t argc, DeeObject *const *argv) {
	ASSERT_OBJECT_TYPE(*p_self, tp_self);
	return invoke_operator(tp_self, *p_self, p_self, name, argc, argv);
}

PUBLIC WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *DCALL
DeeObject_VTInvokeOperatorf(DeeTypeObject *tp_self, DeeObject *self,
                            Dee_operator_t name, char const *__restrict format, va_list args) {
	ASSERT_OBJECT_TYPE(self, tp_self);
	return invoke_voperatorf(tp_self, self, NULL, name, format, args);
}

PUBLIC WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *DCALL
DeeObject_VPTInvokeOperatorf(DeeTypeObject *tp_self, DREF DeeObject **__restrict p_self,
                             Dee_operator_t name, char const *__restrict format, va_list args) {
	ASSERT_OBJECT_TYPE(*p_self, tp_self);
	return invoke_voperatorf(tp_self, *p_self, p_self, name, format, args);
}

PUBLIC WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *
DeeObject_TInvokeOperatorf(DeeTypeObject *tp_self, DeeObject *self,
                           Dee_operator_t name, char const *__restrict format, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, format);
	result = DeeObject_VTInvokeOperatorf(tp_self, self, name, format, args);
	va_end(args);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *
DeeObject_PTInvokeOperatorf(DeeTypeObject *tp_self, DREF DeeObject **__restrict p_self,
                            Dee_operator_t name, char const *__restrict format, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, format);
	result = DeeObject_VPTInvokeOperatorf(tp_self, p_self, name, format, args);
	va_end(args);
	return result;
}



#define DO(err, expr)                    \
	do {                                 \
		if unlikely((temp = (expr)) < 0) \
			goto err;                    \
		result += temp;                  \
	}	__WHILE0

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
print_call_args_repr(Dee_formatprinter_t printer, void *arg,
                     size_t argc, DeeObject *const *argv,
                     DeeObject *kw) {
	Dee_ssize_t temp, result = 0;
	size_t i;
	DO(err, DeeFormat_PRINT(printer, arg, "("));
	for (i = 0; i < argc; ++i) {
		if (i != 0)
			DO(err, DeeFormat_PRINT(printer, arg, ", "));
		if (kw && DeeKwds_Check(kw)) {
			size_t first_keyword;
			ASSERT(DeeKwds_SIZE(kw) <= argc);
			first_keyword = argc - DeeKwds_SIZE(kw);
			if (i >= first_keyword) {
				struct kwds_entry *keyword_descr;
				size_t keyword_index;
				keyword_index = i - first_keyword;
				keyword_descr = DeeKwds_GetByIndex(kw, keyword_index);
				DO(err, DeeFormat_Printf(printer, arg, "%k: ", keyword_descr->ke_name));
			}
		}
		DO(err, DeeFormat_PrintObjectRepr(printer, arg, argv[i]));
	}
	if (kw && !DeeKwds_Check(kw)) {
		/* Keywords are specified as per `foo(**{ "x": 10 })'. */
		if (DeeCachedDict_Check(kw))
			kw = DeeCachedDict_MAP(kw);
		DO(err, DeeFormat_Printf(printer, arg, "**%r", kw));
	}
	DO(err, DeeFormat_PRINT(printer, arg, ")"));
	return result;
err:
	return temp;
	goto err;
}



INTDEF WUNUSED NONNULL((1)) bool DCALL
DeeString_IsSymbol(DeeStringObject *__restrict self,
                   size_t start_index,
                   size_t end_index);

/* Print a representation of invoking operator `name' on `self' with the given arguments.
 * This function is used to generate the representation of the expression in the default
 * assertion failure handler.
 * NOTE: This function also accepts "fake" operators (`FAKE_OPERATOR_*') for `name' */
PUBLIC WUNUSED ATTR_INS(6, 5) NONNULL((1, 3)) Dee_ssize_t DCALL
DeeFormat_PrintOperatorRepr(Dee_formatprinter_t printer, void *arg,
                            DeeObject *self, Dee_operator_t name,
                            size_t argc, DeeObject *const *argv,
                            char const *self_prefix, size_t self_prefix_len,
                            char const *self_suffix, size_t self_suffix_len) {
	Dee_ssize_t temp, result = 0;
	struct opinfo const *info;
	info = DeeTypeType_GetOperatorById(Dee_TYPE(DeeObject_Class(self)), name);
	switch (name) {

	case OPERATOR_COPY:
	case OPERATOR_DEEPCOPY:
	case OPERATOR_STR:
	case OPERATOR_REPR:
		ASSERT(info);
		if (argc == 0)
			DO(err, DeeFormat_Printf(printer, arg, "%s ", info->oi_sname));
		break;

	case OPERATOR_BOOL:
		ASSERT(info);
		if (argc == 0)
			DO(err, DeeFormat_PRINT(printer, arg, "!!"));
		break;

	case OPERATOR_INV:
	case OPERATOR_POS:
	case OPERATOR_NEG:
	case OPERATOR_INC:
	case OPERATOR_DEC:
	case OPERATOR_SIZE:
		ASSERT(info);
		if (argc == 0)
			DO(err, DeeFormat_PrintStr(printer, arg, info->oi_uname));
		break;

	case OPERATOR_CONTAINS:
		if (argc == 1)
			DO(err, DeeFormat_Printf(printer, arg, "%r in ", argv[0]));
		break;

	case OPERATOR_DELITEM:
		if (argc == 1)
			DO(err, DeeFormat_PRINT(printer, arg, "del "));
		break;

	case OPERATOR_DELRANGE:
		if (argc == 2)
			DO(err, DeeFormat_PRINT(printer, arg, "del "));
		break;

	case OPERATOR_DELATTR:
		if (argc == 1 && DeeString_Check(argv[0]))
			DO(err, DeeFormat_PRINT(printer, arg, "del "));
		break;

	default: break;
	}

	/* Print the main self-argument */
	DO(err, DeeFormat_Printf(printer, arg, "%$s%r%$s",
	                         self_prefix_len, self_prefix, self,
	                         self_suffix_len, self_suffix));

	switch (name) {

	case OPERATOR_COPY:
	case OPERATOR_DEEPCOPY:
	case OPERATOR_STR:
	case OPERATOR_REPR:
	case OPERATOR_BOOL:
	case OPERATOR_INV:
	case OPERATOR_POS:
	case OPERATOR_NEG:
	case OPERATOR_INC:
	case OPERATOR_DEC:
	case OPERATOR_SIZE:
		if (argc == 0)
			goto done;
		break;

	case OPERATOR_CALL:
		if ((argc == 1 || argc == 2) && DeeTuple_Check(argv[0])) {
			DeeObject *kw = argc == 2 ? argv[1] : NULL;
			if (kw && DeeKwds_Check(kw)) {
				size_t c_argc = DeeTuple_SIZE(argv[0]);
				size_t c_kwdc = DeeKwds_SIZE(kw);
				if (c_kwdc > c_argc)
					break; /* err_keywords_bad_for_argc() */
			}
			DO(err, print_call_args_repr(printer, arg,
			                             DeeTuple_SIZE(argv[0]),
			                             DeeTuple_ELEM(argv[0]),
			                             kw));
			goto done;
		}
		break;

	case OPERATOR_ASSIGN:
	case OPERATOR_ADD:
	case OPERATOR_SUB:
	case OPERATOR_MUL:
	case OPERATOR_DIV:
	case OPERATOR_MOD:
	case OPERATOR_SHL:
	case OPERATOR_SHR:
	case OPERATOR_AND:
	case OPERATOR_OR:
	case OPERATOR_XOR:
	case OPERATOR_POW:
	case OPERATOR_INPLACE_ADD:
	case OPERATOR_INPLACE_SUB:
	case OPERATOR_INPLACE_MUL:
	case OPERATOR_INPLACE_DIV:
	case OPERATOR_INPLACE_MOD:
	case OPERATOR_INPLACE_SHL:
	case OPERATOR_INPLACE_SHR:
	case OPERATOR_INPLACE_AND:
	case OPERATOR_INPLACE_OR:
	case OPERATOR_INPLACE_XOR:
	case OPERATOR_INPLACE_POW:
	case OPERATOR_EQ:
	case OPERATOR_NE:
	case OPERATOR_LO:
	case OPERATOR_LE:
	case OPERATOR_GR:
	case OPERATOR_GE:
		ASSERT(info);
		if (argc == 1) {
			DO(err, DeeFormat_Printf(printer, arg, " %s %r", info->oi_uname, argv[0]));
			goto done;
		}
		break;

	case OPERATOR_CONTAINS:
		if (argc == 1)
			goto done;
		break;

	case OPERATOR_GETITEM:
	case OPERATOR_DELITEM:
		if (argc == 1) {
			DO(err, DeeFormat_Printf(printer, arg, "[%r]", argv[0]));
			goto done;
		}
		break;

	case OPERATOR_SETITEM:
		if (argc == 2) {
			DO(err, DeeFormat_Printf(printer, arg, "[%r] = %r", argv[0], argv[1]));
			goto done;
		}
		break;

	case OPERATOR_GETRANGE:
	case OPERATOR_DELRANGE:
	case OPERATOR_SETRANGE:
		if (argc == (size_t)(name == OPERATOR_SETRANGE ? 3 : 2)) {
			DO(err, DeeFormat_PRINT(printer, arg, "["));
			if (!DeeNone_Check(argv[0]))
				DO(err, DeeFormat_PrintObjectRepr(printer, arg, argv[0]));
			DO(err, DeeFormat_PRINT(printer, arg, ":"));
			if (!DeeNone_Check(argv[1]))
				DO(err, DeeFormat_PrintObjectRepr(printer, arg, argv[1]));
			DO(err, DeeFormat_PRINT(printer, arg, "]"));
			if (name == OPERATOR_SETRANGE)
				DO(err, DeeFormat_Printf(printer, arg, " = %r", argv[2]));
			goto done;
		}
		break;

	case OPERATOR_GETATTR:
	case OPERATOR_DELATTR:
		if (argc == 1 && DeeString_Check(argv[0])) {
			if (DeeString_IsSymbol((DeeStringObject *)argv[0], 0, (size_t)-1)) {
				DO(err, DeeFormat_Printf(printer, arg, ".%k", argv[0]));
			} else {
				DO(err, DeeFormat_Printf(printer, arg, ".operator . (%r)", argv[0]));
			}
			goto done;
		}
		break;

	case OPERATOR_SETATTR:
		if (argc == 2 && DeeString_Check(argv[0])) {
			if (DeeString_IsSymbol((DeeStringObject *)argv[0], 0, (size_t)-1)) {
				DO(err, DeeFormat_Printf(printer, arg, ".%k = %r", argv[0], argv[1]));
			} else {
				DO(err, DeeFormat_Printf(printer, arg, ".operator . (%r) = %r", argv[0], argv[1]));
			}
			goto done;
		}
		break;

		/* Fake operators */
	case FAKE_OPERATOR_IS:
	case FAKE_OPERATOR_SAME_OBJECT:
	case FAKE_OPERATOR_DIFF_OBJECT:
		if (argc == 1) {
			char const *optok;
			switch (name) {
			case FAKE_OPERATOR_IS:
				optok = "is";
				break;
			case FAKE_OPERATOR_SAME_OBJECT:
				optok = "===";
				break;
			case FAKE_OPERATOR_DIFF_OBJECT:
				optok = "!==";
				break;
			default: __builtin_unreachable();
			}
			DO(err, DeeFormat_Printf(printer, arg, " %s %r", optok, argv[0]));
			goto done;
		}
		break;

	default: break;
	}

	/* Fallback: print as `<self>.operator <name> (<argv...>)' */
	{
		size_t i;
		DO(err, DeeFormat_PRINT(printer, arg, ".operator "));
		if (info) {
			DO(err, DeeFormat_PrintStr(printer, arg, info->oi_uname));
		} else {
			DO(err, DeeFormat_Printf(printer, arg, "%" PRFu16, name));
		}
		DO(err, DeeFormat_PRINT(printer, arg, "("));
		for (i = 0; i < argc; ++i) {
			if (i != 0)
				DO(err, DeeFormat_PRINT(printer, arg, ", "));
			DO(err, DeeFormat_PrintObjectRepr(printer, arg, argv[i]));
		}
		DO(err, DeeFormat_PRINT(printer, arg, ")"));
	}
done:
	return result;
err:
	return temp;
}

#undef DO

/* When inlining stuff, gcc thinks that `self->tp_mro_iter' below is uninitialized.
 * It's correct, but the the code it's complaining about is unreachable when said
 * field hasn't been initialized, yet. */
__pragma_GCC_diagnostic_push_ignored(Wmaybe_uninitialized)

/* Advance an MRO enumerator, returning the next type in MRO order.
 * @param: tp_iter: The previously enumerated type
 * @return: * :     The next type for the purpose of MRO resolution.
 * @return: NULL:   End of MRO chain has been reached. */
PUBLIC WUNUSED NONNULL((1, 2)) DeeTypeObject *DFCALL
DeeTypeMRO_Next(DeeTypeMRO *__restrict self,
                DeeTypeObject const *tp_iter) {
	DeeTypeObject *result;
	if (tp_iter == self->tp_mro_orig) {
		/* Got here after the first type in the MRO resolution order. */
		self->tp_mro_iter = tp_iter->tp_mro;
		if (self->tp_mro_iter) {
			result = *self->tp_mro_iter++;
			ASSERTF(result != NULL,
			        "tp_mro[0] of type %r is NULL",
			        tp_iter->tp_mro);
			goto done;
		}
	} else if (self->tp_mro_iter != NULL) {
		/* One of the types being enumerated has a custom MRO -> load the next element.
		 * If that element happens to be NULL, then enumeration will end. */
		result = *self->tp_mro_iter++;
		goto done;
	}

	/* Load the next *true* base. */
	result = tp_iter->tp_base;
	ASSERTF(result != self->tp_mro_orig,
	        "Type base loop with %r",
	        result);
done:
	return result;
}

/* Like `DeeTypeMRO_Next()', but only enumerate direct
 * bases of the type passed to `DeeTypeMRO_Init()' */
PUBLIC WUNUSED NONNULL((1, 2)) DeeTypeObject *DFCALL
DeeTypeMRO_NextDirectBase(DeeTypeMRO *__restrict self,
                          DeeTypeObject const *tp_iter) {
	DeeTypeObject *result;
	if (tp_iter == self->tp_mro_orig) {
		/* Got here after the first type in the MRO resolution order. */
		self->tp_mro_iter = tp_iter->tp_mro;
		if (self->tp_mro_iter) {
			ASSERTF(self->tp_mro_iter[0] != NULL,
			        "tp_mro[0] of type %r is NULL",
			        tp_iter->tp_mro);
			goto select_from_mro_iter;
		}

		/* Load only the first *true* base. */
		result = tp_iter->tp_base;
		ASSERTF(result != self->tp_mro_orig,
		        "Type base loop with %r",
		        result);
	} else if (self->tp_mro_iter != NULL) {
		/* One of the types being enumerated has a custom MRO -> load the next element.
		 * If that element happens to be NULL, then enumeration will end. */
		DeeTypeObject *first_non_direct_base;
select_from_mro_iter:
		result = *self->tp_mro_iter++;

		/* The first non-direct base is the first base of the initial type's first base. */
		first_non_direct_base = self->tp_mro_orig->tp_mro[0];
		if (first_non_direct_base->tp_mro) {
			first_non_direct_base = first_non_direct_base->tp_mro[0];
		} else {
			first_non_direct_base = first_non_direct_base->tp_base;
		}
		if (result == first_non_direct_base) {
			/* Stop enumeration once the first non-direct base is reached. */
			result = NULL;
		}
	} else {
		/* Don't enumerate recursive bases. */
		result = NULL;
	}
	return result;
}

__pragma_GCC_diagnostic_pop_ignored(Wmaybe_uninitialized)




typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeTypeObject, toi_type); /* [1..1][const] The type who's operators should be enumerated. */
	Dee_operator_t                      toi_opid;  /* [lock(ATOMIC)] Next operator ID to check. */
	bool                                toi_name;  /* [const] When true, try to assign human-readable names to operators. */
} TypeOperatorsIterator;

typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeTypeObject, to_type); /* [1..1][const] The type who's operators should be enumerated. */
	bool                                to_name;  /* [const] When true, try to assign human-readable names to operators. */
} TypeOperators;

INTDEF DeeTypeObject TypeOperators_Type;
INTDEF DeeTypeObject TypeOperatorsIterator_Type;

STATIC_ASSERT(offsetof(TypeOperators, to_type) == offsetof(ProxyObject, po_obj));
#define to_serialize generic_proxy__serialize_and_memcpy
#define to_fini      generic_proxy__fini_unlikely
#define to_visit     generic_proxy__visit

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
to_copy(TypeOperators *__restrict self, TypeOperators *__restrict other) {
	self->to_type = other->to_type;
	Dee_Incref(self->to_type);
	self->to_name = other->to_name;
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
toi_serialize(TypeOperatorsIterator *__restrict self,
              DeeSerial *__restrict writer, Dee_seraddr_t addr) {
	int result = generic_proxy__serialize((ProxyObject *)self, writer, addr);
	if likely(result == 0) {
		TypeOperatorsIterator *out;
		out = DeeSerial_Addr2Mem(writer, addr, TypeOperatorsIterator);
		out->toi_opid = atomic_read(&self->toi_opid);
		out->toi_name = self->toi_name;
	}
	return result;
}

STATIC_ASSERT(offsetof(TypeOperatorsIterator, toi_type) == offsetof(ProxyObject, po_obj));
#define toi_fini  generic_proxy__fini_unlikely
#define toi_visit generic_proxy__visit

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
to_print(TypeOperators *__restrict self, Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "Operator %ss for %k",
	                        self->to_name ? "name" : "id",
	                        self->to_type);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
to_printrepr(TypeOperators *__restrict self, Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "%r.__operator%ss__",
	                        self->to_type,
	                        self->to_name ? "" : "id");
}

PRIVATE WUNUSED NONNULL((1)) DREF TypeOperatorsIterator *DCALL
to_iter(TypeOperators *__restrict self) {
	DREF TypeOperatorsIterator *result;
	result = DeeObject_MALLOC(TypeOperatorsIterator);
	if unlikely(!result)
		goto done;
	result->toi_type = self->to_type;
	result->toi_opid = 0;
	result->toi_name = self->to_name;
	Dee_Incref(self->to_type);
	DeeObject_Init(result, &TypeOperatorsIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
to_contains(TypeOperators *self, DeeObject *name_or_id) {
	Dee_operator_t id;
	if (DeeString_Check(name_or_id)) {
		struct opinfo const *info;
		info = DeeTypeType_GetOperatorByName(Dee_TYPE(self->to_type),
		                                     DeeString_STR(name_or_id),
		                                     (size_t)-1);
		if (info == NULL)
			return_false;
		id = info->oi_id;
	} else {
		if (DeeObject_AsUInt16(name_or_id, &id))
			goto err;
	}
	return_bool(DeeType_HasPrivateOperator(self->to_type, id));
err:
	return NULL;
}


PRIVATE struct type_seq to_seq = {
	/* .tp_iter         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&to_iter,
	/* .tp_sizeob       = */ DEFIMPL(&default__seq_operator_sizeob__with__seq_operator_size),
	/* .tp_contains     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&to_contains,
	/* .tp_getitem      = */ DEFIMPL_UNSUPPORTED(&default__getitem__unsupported),
	/* .tp_delitem      = */ DEFIMPL_UNSUPPORTED(&default__delitem__unsupported),
	/* .tp_setitem      = */ DEFIMPL_UNSUPPORTED(&default__setitem__unsupported),
	/* .tp_getrange     = */ DEFIMPL_UNSUPPORTED(&default__getrange__unsupported),
	/* .tp_delrange     = */ DEFIMPL_UNSUPPORTED(&default__delrange__unsupported),
	/* .tp_setrange     = */ DEFIMPL_UNSUPPORTED(&default__setrange__unsupported),
	/* .tp_foreach      = */ DEFIMPL(&default__foreach__with__iter), /* TODO: tp_foreach */
	/* .tp_foreach_pair = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem    = */ DEFIMPL_UNSUPPORTED(&default__bounditem__unsupported),
	/* .tp_hasitem      = */ DEFIMPL_UNSUPPORTED(&default__hasitem__unsupported),
	/* .tp_size         = */ DEFIMPL(&default__seq_operator_size__with__seq_operator_iter),
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ DEFIMPL_UNSUPPORTED(&default__getitem_index__unsupported),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL_UNSUPPORTED(&default__delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL_UNSUPPORTED(&default__setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL_UNSUPPORTED(&default__bounditem_index__unsupported),
	/* .tp_hasitem_index              = */ DEFIMPL_UNSUPPORTED(&default__hasitem_index__unsupported),
	/* .tp_getrange_index             = */ DEFIMPL_UNSUPPORTED(&default__getrange_index__unsupported),
	/* .tp_delrange_index             = */ DEFIMPL_UNSUPPORTED(&default__delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL_UNSUPPORTED(&default__setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__getrange_index_n__unsupported),
	/* .tp_delrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL_UNSUPPORTED(&default__trygetitem__unsupported),
	/* .tp_trygetitem_index           = */ DEFIMPL_UNSUPPORTED(&default__trygetitem_index__unsupported),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL_UNSUPPORTED(&default__trygetitem_string_hash__unsupported),
	/* .tp_getitem_string_hash        = */ DEFIMPL_UNSUPPORTED(&default__getitem_string_hash__unsupported),
	/* .tp_delitem_string_hash        = */ DEFIMPL_UNSUPPORTED(&default__delitem_string_hash__unsupported),
	/* .tp_setitem_string_hash        = */ DEFIMPL_UNSUPPORTED(&default__setitem_string_hash__unsupported),
	/* .tp_bounditem_string_hash      = */ DEFIMPL_UNSUPPORTED(&default__bounditem_string_hash__unsupported),
	/* .tp_hasitem_string_hash        = */ DEFIMPL_UNSUPPORTED(&default__hasitem_string_hash__unsupported),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL_UNSUPPORTED(&default__trygetitem_string_len_hash__unsupported),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL_UNSUPPORTED(&default__getitem_string_len_hash__unsupported),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL_UNSUPPORTED(&default__delitem_string_len_hash__unsupported),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL_UNSUPPORTED(&default__setitem_string_len_hash__unsupported),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL_UNSUPPORTED(&default__bounditem_string_len_hash__unsupported),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL_UNSUPPORTED(&default__hasitem_string_len_hash__unsupported),
};

PRIVATE struct type_member tpconst to_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &TypeOperatorsIterator_Type),
	TYPE_MEMBER_CONST(STR_KeyType, &DeeString_Type),
	TYPE_MEMBER_CONST(STR_ValueType, &DeeInt_Type),
	TYPE_MEMBER_END
};

#define TOI_GETOPID(x) atomic_read(&(x)->toi_opid)

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
toi_copy(TypeOperatorsIterator *__restrict self,
         TypeOperatorsIterator *__restrict other) {
	self->toi_type = other->toi_type;
	self->toi_opid = TOI_GETOPID(other);
	self->toi_name = other->toi_name;
	Dee_Incref(self->toi_type);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
toi_init(TypeOperatorsIterator *__restrict self,
         size_t argc, DeeObject *const *argv) {
	TypeOperators *ops;
	DeeArg_Unpack1(err, argc, argv, "_TypeOperatorsIterator", &ops);
	if (DeeObject_AssertTypeExact(ops, &TypeOperators_Type))
		goto err;
	self->toi_type = ops->to_type;
	self->toi_opid = 0;
	self->toi_name = ops->to_name;
	Dee_Incref(ops->to_type);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
toi_hash(TypeOperatorsIterator *__restrict self) {
	return (Dee_hash_t)TOI_GETOPID(self);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
toi_compare(TypeOperatorsIterator *self,
            TypeOperatorsIterator *other) {
	if (DeeObject_AssertTypeExact(other, &TypeOperatorsIterator_Type))
		goto err;
	Dee_return_compareT(Dee_operator_t, TOI_GETOPID(self),
	                    /*           */ TOI_GETOPID(other));
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp toi_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *))&toi_hash,
	/* .tp_compare_eq = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&toi_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
toi_next(TypeOperatorsIterator *__restrict self) {
	DeeTypeObject *tp = self->toi_type;
	struct opinfo const *info;
	Dee_operator_t result;
	Dee_operator_t start;
	for (;;) {
		start  = atomic_read(&self->toi_opid);
		result = start;
		for (;; ++result) {
			/* Query information about the given operator. */
			info = DeeTypeType_GetOperatorById(Dee_TYPE(tp), result);
			if (result == OPERATOR_CONSTRUCTOR) {
				/* Special case: the constructor operator (which cannot be inherited). */
				if (tp->tp_init.tp_alloc.tp_ctor != NULL ||
				    tp->tp_init.tp_alloc.tp_any_ctor != NULL ||
				    tp->tp_init.tp_alloc.tp_any_ctor_kw != NULL)
					break;
				continue;
			}
			if (!info) {
				/* If there isn't an operator record, switch to extended operators. */
				if (result < OPERATOR_EXTENDED(0)) {
					result = OPERATOR_EXTENDED(0);
					continue;
				}

				/* If we already were within extended operators, then
				 * we know we've check all of them at this point, meaning
				 * we know that all operators have now been enumerated. */
				return ITER_DONE;
			}

			/* Check if this operator is implemented (though isn't inherited). */
			if (DeeType_HasPrivateOperator(tp, result))
				break;
		}
		if (atomic_cmpxch_weak_or_write(&self->toi_opid, start, result + 1))
			break;
	}
	if (self->toi_name && *info->oi_sname)
		return DeeString_New(info->oi_sname);
	return DeeInt_NewUInt16(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF TypeOperators *DCALL
toi_getseq(TypeOperatorsIterator *__restrict self) {
	DREF TypeOperators *result;
	result = DeeObject_MALLOC(TypeOperators);
	if unlikely(!result)
		goto done;
	result->to_type = self->toi_type;
	result->to_name = self->toi_name;
	Dee_Incref(result->to_type);
	DeeObject_Init(result, &TypeOperators_Type);
done:
	return result;
}

PRIVATE struct type_getset tpconst toi_getset[] = {
	TYPE_GETTER_AB(STR_seq, &toi_getseq, "->?Ert:TypeOperators"),
	TYPE_GETSET_END
};

INTERN DeeTypeObject TypeOperatorsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_TypeOperatorsIterator",
	/* .tp_doc      = */ DOC("(ops:?Ert:TypeOperators)\n"
	                         "\n"
	                         "next->?X2?Dstring?Dint"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ TypeOperatorsIterator,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &toi_copy,
			/* tp_deep_ctor:   */ &toi_copy,
			/* tp_any_ctor:    */ &toi_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &toi_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&toi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&iterator_bool),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&toi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &toi_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&toi_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ toi_getset,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject TypeOperators_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_TypeOperators",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSet_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ TypeOperators,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &to_copy,
			/* tp_deep_ctor:   */ &to_copy,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &to_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&to_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ DEFIMPL(&default__seq_operator_bool__with__seq_operator_iter),
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&to_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&to_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&to_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__F6E3D7B2219AE1EB),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__A5C53AFDF1233C5A),
	/* .tp_seq           = */ &to_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ to_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_get_operators(DeeTypeObject *__restrict self) {
	DREF TypeOperators *result;
	result = DeeObject_MALLOC(TypeOperators);
	if unlikely(!result)
		goto done;
	result->to_type = self;
	result->to_name = true;
	Dee_Incref(self);
	DeeObject_Init(result, &TypeOperators_Type);
done:
	return Dee_AsObject(result);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_get_operatorids(DeeTypeObject *__restrict self) {
	DREF TypeOperators *result;
	result = DeeObject_MALLOC(TypeOperators);
	if unlikely(!result)
		goto done;
	result->to_type = self;
	result->to_name = false;
	Dee_Incref(self);
	DeeObject_Init(result, &TypeOperators_Type);
done:
	return Dee_AsObject(result);
}


DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_OPERATOR_INFO_C */
