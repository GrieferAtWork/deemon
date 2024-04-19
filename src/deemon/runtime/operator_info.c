/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_RUNTIME_OPERATOR_INFO_C
#define GUARD_DEEMON_RUNTIME_OPERATOR_INFO_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/attribute.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/cached-dict.h>
#include <deemon/class.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/float.h>
#include <deemon/int.h>
#include <deemon/kwds.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/set.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/system-features.h> /* memcpyc(), ... */
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

#include <hybrid/typecore.h>

#include "runtime_error.h"
#include "strings.h"

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
PUBLIC ATTR_PURE WUNUSED NONNULL((1)) struct Dee_opinfo const *DCALL
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
				RETURN(OPERATOR_ITERSELF);
			if (EQAT(name + 4, "self"))
				RETURN(OPERATOR_ITERSELF);
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
	DeeTypeMRO mro;
	DeeTypeObject *base;
	struct type_operator const *result;
	result = DeeType_GetPrivateCustomOperatorById(self, id);
	if (result)
		return result;
	base = DeeTypeMRO_Init(&mro, (DeeTypeObject *)self);
	while ((base = DeeTypeMRO_Next(&mro, base)) != NULL) {
		result = DeeType_GetPrivateCustomOperatorById(self, id);
		if (result)
			return result;
	}

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
 * IMPORTANT: When querying the flags for `OPERATOR_ITERSELF', the `Dee_METHOD_FCONSTCALL',
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

	/* Default flags for certain operators. */
	switch (opname) {
	case OPERATOR_HASH:
	case OPERATOR_DESTRUCTOR:
	case OPERATOR_VISIT:
	case OPERATOR_CLEAR:
	case OPERATOR_PCLEAR:
		return METHOD_FNOTHROW;
	default: break;
	}
	return METHOD_FNORMAL;
}


/* Helper for checking that every cast-like operators is
 * either not implemented, or marked as Dee_METHOD_FCONSTCALL:
 * >> (!DeeType_HasOperator(self, OPERATOR_BOOL) || (DeeType_GetOperatorFlags(self, OPERATOR_BOOL) & Dee_METHOD_FCONSTCALL)) &&
 * >> (!DeeType_HasOperator(self, OPERATOR_INT) || (DeeType_GetOperatorFlags(self, OPERATOR_INT) & Dee_METHOD_FCONSTCALL)) &&
 * >> (!DeeType_HasOperator(self, OPERATOR_FLOAT) || (DeeType_GetOperatorFlags(self, OPERATOR_FLOAT) & Dee_METHOD_FCONSTCALL));
 * >> (!DeeType_HasOperator(self, OPERATOR_ITERSELF) || (DeeType_GetOperatorFlags(self, OPERATOR_ITERSELF) & Dee_METHOD_FCONSTCALL));
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
		OPERATOR_ITERSELF,
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
DeeType_HasPrivateOperator(DeeTypeObject const *__restrict self, Dee_operator_t name) {
	void const *my_ptr;
	struct opinfo const *info;
	DeeTypeObject *base;
	DeeTypeMRO mro;
	/* Special case: must look at what's implemented by the class! */
	if (DeeType_IsClass(self))
		return DeeClass_TryGetPrivateOperatorPtr((DeeTypeObject *)self, name) != NULL;
	switch (name) {

	case OPERATOR_CONSTRUCTOR:
		/* Special case: the constructor operator (which cannot be inherited). */
		return (self->tp_init.tp_alloc.tp_ctor != NULL ||
		        self->tp_init.tp_alloc.tp_any_ctor != NULL ||
		        self->tp_init.tp_alloc.tp_any_ctor_kw != NULL);

	case OPERATOR_STR:
		/* Special case: `operator str' can be implemented in 2 ways */
		if (self->tp_cast.tp_str == NULL && self->tp_cast.tp_print == NULL)
			return false;
		base = DeeTypeMRO_Init(&mro, (DeeTypeObject *)self);
		while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
			if (self->tp_cast.tp_str == base->tp_cast.tp_str &&
			    self->tp_cast.tp_print == base->tp_cast.tp_print)
				return false;
		}
		return true;

	case OPERATOR_REPR:
		/* Special case: `operator repr' can be implemented in 2 ways */
		if (self->tp_cast.tp_repr == NULL && self->tp_cast.tp_printrepr == NULL)
			return false;
		base = DeeTypeMRO_Init(&mro, (DeeTypeObject *)self);
		while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
			if (self->tp_cast.tp_repr == base->tp_cast.tp_repr &&
			    self->tp_cast.tp_printrepr == base->tp_cast.tp_printrepr)
				return false;
		}
		return true;

	default:
		break;
	}
	info = DeeTypeType_GetOperatorById(Dee_TYPE(self), name);
	if (info == NULL)
		return false; /* No such operator */
	my_ptr = DeeType_GetOpPointer(self, info);
	if (my_ptr == NULL) {
		/* Not a standard operator. Check if it may be defined by the type itself */
		return DeeType_GetPrivateCustomOperatorById(self, name) != NULL;
	}
	base = DeeTypeMRO_Init(&mro, (DeeTypeObject *)self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		if (my_ptr == DeeType_GetOpPointer(base, info))
			return false; /* Base has same impl -> operator was inherited */
	}
	return true; /* Operator is distinct from all bases. */
}

DFUNDEF ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
DeeType_HasPrivateNSI(DeeTypeObject const *__restrict self) {
	struct type_nsi const *nsi;
	DeeTypeObject *base;
	DeeTypeMRO mro;
	if unlikely(!self->tp_seq)
		return false;
	nsi = self->tp_seq->tp_nsi;
	if unlikely(!nsi)
		return false;
	base = DeeTypeMRO_Init(&mro, (DeeTypeObject *)self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		if (base->tp_seq && base->tp_seq->tp_nsi == nsi)
			return false;
	}
	return true;
}

DFUNDEF ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
DeeType_HasPrivateNII(DeeTypeObject const *__restrict self) {
	struct type_nii const *nii;
	DeeTypeObject *base;
	DeeTypeMRO mro;
	if unlikely(!self->tp_cmp)
		return false;
	nii = self->tp_cmp->tp_nii;
	if unlikely(!nii)
		return false;
	base = DeeTypeMRO_Init(&mro, (DeeTypeObject *)self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		if (base->tp_cmp && base->tp_cmp->tp_nii == nii)
			return false;
	}
	return true;
}

/* Return the type from `self' inherited its operator `name'.
 * If `name' wasn't inherited, or isn't defined, simply re-return `self'. */
PUBLIC ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeTypeObject *DCALL
DeeType_GetOperatorOrigin(DeeTypeObject const *__restrict self, Dee_operator_t name) {
	void const *my_ptr;
	struct opinfo const *info;
	DeeTypeObject *base, *result;
	DeeTypeMRO mro;
	/* Special case: must look at what's implemented by the class! */
	if (DeeType_IsClass(self)) {
		if (!DeeClass_TryGetPrivateOperatorPtr((DeeTypeObject *)self, name)) {
			base = DeeTypeMRO_Init(&mro, (DeeTypeObject *)self);
			while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
				if (DeeType_HasPrivateOperator(base, name))
					return base;
			}
		}
		return (DeeTypeObject *)self;
	}
	switch (name) {

	case OPERATOR_CONSTRUCTOR:
		/* Special case: the constructor operator (which cannot be inherited). */
		return (DeeTypeObject *)self;

	case OPERATOR_STR:
		/* Special case: `operator str' can be implemented in 2 ways */
		result = (DeeTypeObject *)self;
		if (self->tp_cast.tp_str || self->tp_cast.tp_print) {
			base = DeeTypeMRO_Init(&mro, (DeeTypeObject *)self);
			while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
				if (self->tp_cast.tp_str == base->tp_cast.tp_str &&
				    self->tp_cast.tp_print == base->tp_cast.tp_print) {
					result = base;
				} else {
					break;
				}
			}
		}
		return result;

	case OPERATOR_REPR:
		/* Special case: `operator repr' can be implemented in 2 ways */
		result = (DeeTypeObject *)self;
		if (self->tp_cast.tp_repr || self->tp_cast.tp_printrepr) {
			base = DeeTypeMRO_Init(&mro, (DeeTypeObject *)self);
			while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
				if (self->tp_cast.tp_repr == base->tp_cast.tp_repr &&
				    self->tp_cast.tp_printrepr == base->tp_cast.tp_printrepr) {
					result = base;
				} else {
					break;
				}
			}
		}
		return result;

	default:
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
		return NULL; /* Operator not implemented */
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
				Dee_DPRINTF("[RT] Inherit `" OPNAME("%s") "' from %q into %q\n",
				            info->oi_sname, base->tp_name, self->tp_name);
#endif
				return true;
			}
		}
	}

	/* Re-check if the original type itself was already implementing the type. */
	return DeeType_GetOpPointer(self, info) != NULL;
}

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
		       self->tp_init.tp_var.tp_any_ctor ||
		       self->tp_init.tp_var.tp_any_ctor_kw ||
		       DeeType_InheritConstructors(self);
	case OPERATOR_STR:
		/* "str" and "repr" are special in that we can *always* substitude str<=>print and repr<=>printrepr
		 * As such, invoke the inherit function even if one has already been implemented, but the other hasn't. */
		return (self->tp_cast.tp_str && self->tp_cast.tp_print) || DeeType_InheritStr(self);
	case OPERATOR_REPR:
		return (self->tp_cast.tp_repr && self->tp_cast.tp_printrepr) || DeeType_InheritRepr(self);
	case OPERATOR_BOOL:
		return self->tp_cast.tp_bool || DeeType_InheritBool(self);
	case OPERATOR_CALL:
		return self->tp_call || self->tp_call_kw || DeeType_InheritCall(self);
	case OPERATOR_HASH:
		return (self->tp_cmp && (self->tp_cmp->tp_hash)) || DeeType_InheritHash(self);
	case OPERATOR_INT:
		return (self->tp_math && (self->tp_math->tp_int || self->tp_math->tp_int32 ||
		                          self->tp_math->tp_int64 || self->tp_math->tp_double)) ||
		       DeeType_InheritInt(self);
	case OPERATOR_INV:
		return (self->tp_math && (self->tp_math->tp_inv)) || DeeType_InheritInv(self);
	case OPERATOR_POS:
		return (self->tp_math && (self->tp_math->tp_pos)) || DeeType_InheritPos(self);
	case OPERATOR_NEG:
		return (self->tp_math && (self->tp_math->tp_neg)) || DeeType_InheritNeg(self);
	case OPERATOR_ADD:
	case OPERATOR_SUB:
	case OPERATOR_INC:
	case OPERATOR_DEC:
	case OPERATOR_INPLACE_ADD:
	case OPERATOR_INPLACE_SUB:
		return (self->tp_math && (self->tp_math->tp_add || self->tp_math->tp_sub ||
		                          self->tp_math->tp_inc || self->tp_math->tp_dec ||
		                          self->tp_math->tp_inplace_add || self->tp_math->tp_inplace_sub)) ||
		       DeeType_InheritAdd(self);
	case OPERATOR_MUL:
	case OPERATOR_INPLACE_MUL:
		return (self->tp_math && (self->tp_math->tp_mul || self->tp_math->tp_inplace_mul)) || DeeType_InheritMul(self);
	case OPERATOR_DIV:
	case OPERATOR_INPLACE_DIV:
		return (self->tp_math && (self->tp_math->tp_div || self->tp_math->tp_inplace_div)) || DeeType_InheritDiv(self);
	case OPERATOR_MOD:
	case OPERATOR_INPLACE_MOD:
		return (self->tp_math && (self->tp_math->tp_mod || self->tp_math->tp_inplace_mod)) || DeeType_InheritMod(self);
	case OPERATOR_SHL:
	case OPERATOR_INPLACE_SHL:
		return (self->tp_math && (self->tp_math->tp_shl || self->tp_math->tp_inplace_shl)) || DeeType_InheritShl(self);
	case OPERATOR_SHR:
	case OPERATOR_INPLACE_SHR:
		return (self->tp_math && (self->tp_math->tp_shr || self->tp_math->tp_inplace_shr)) || DeeType_InheritShr(self);
	case OPERATOR_AND:
	case OPERATOR_INPLACE_AND:
		return (self->tp_math && (self->tp_math->tp_and || self->tp_math->tp_inplace_and)) || DeeType_InheritAnd(self);
	case OPERATOR_OR:
	case OPERATOR_INPLACE_OR:
		return (self->tp_math && (self->tp_math->tp_or || self->tp_math->tp_inplace_or)) || DeeType_InheritOr(self);
	case OPERATOR_XOR:
	case OPERATOR_INPLACE_XOR:
		return (self->tp_math && (self->tp_math->tp_xor || self->tp_math->tp_inplace_xor)) || DeeType_InheritXor(self);
	case OPERATOR_POW:
	case OPERATOR_INPLACE_POW:
		return (self->tp_math && (self->tp_math->tp_pow || self->tp_math->tp_inplace_pow)) || DeeType_InheritPow(self);
	case OPERATOR_EQ:
	case OPERATOR_NE:
	case OPERATOR_LO:
	case OPERATOR_LE:
	case OPERATOR_GR:
	case OPERATOR_GE:
		return (self->tp_cmp && (self->tp_cmp->tp_eq || self->tp_cmp->tp_ne ||
		                         self->tp_cmp->tp_lo || self->tp_cmp->tp_le ||
		                         self->tp_cmp->tp_gr || self->tp_cmp->tp_ge)) ||
		       DeeType_InheritCompare(self);
	case OPERATOR_ITERNEXT:
		return (self->tp_iter_next) || DeeType_InheritIterNext(self);
	case OPERATOR_ITERSELF:
	case OPERATOR_SIZE:
	case OPERATOR_CONTAINS:
	case OPERATOR_GETITEM:
	case OPERATOR_DELITEM:
	case OPERATOR_SETITEM:
	case OPERATOR_GETRANGE:
	case OPERATOR_DELRANGE:
	case OPERATOR_SETRANGE:
		if (!self->tp_seq)
			DeeType_InheritNSI(self);
		switch (name) {
		case OPERATOR_ITERSELF:
			return (self->tp_seq && (self->tp_seq->tp_iter_self)) || DeeType_InheritIterSelf(self);
		case OPERATOR_SIZE:
			return (self->tp_seq && (self->tp_seq->tp_size)) || DeeType_InheritSize(self);
		case OPERATOR_CONTAINS:
			return (self->tp_seq && (self->tp_seq->tp_contains)) || DeeType_InheritContains(self);
		case OPERATOR_GETITEM:
			return (self->tp_seq && (self->tp_seq->tp_get)) || DeeType_InheritGetItem(self);
		case OPERATOR_DELITEM:
			return (self->tp_seq && (self->tp_seq->tp_del)) || DeeType_InheritDelItem(self);
		case OPERATOR_SETITEM:
			return (self->tp_seq && (self->tp_seq->tp_set)) || DeeType_InheritSetItem(self);
		case OPERATOR_GETRANGE:
			return (self->tp_seq && (self->tp_seq->tp_range_get)) || DeeType_InheritGetRange(self);
		case OPERATOR_DELRANGE:
			return (self->tp_seq && (self->tp_seq->tp_range_del)) || DeeType_InheritDelRange(self);
		case OPERATOR_SETRANGE:
			return (self->tp_seq && (self->tp_seq->tp_range_set)) || DeeType_InheritSetRange(self);
		default: __builtin_unreachable();
		}
		__builtin_unreachable();
	case OPERATOR_ENTER:
	case OPERATOR_LEAVE:
		return (self->tp_with && (self->tp_with->tp_enter || self->tp_with->tp_leave)) || DeeType_InheritWith(self);
	case OPERATOR_GETBUF:
		return (self->tp_buffer && (self->tp_buffer->tp_getbuf || self->tp_buffer->tp_putbuf)) || DeeType_InheritBuffer(self);
	default: break;
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
			if (DeeType_GetOpPointer(tp_self, info) == NULL) {
				if unlikely(!DeeType_InheritOperator(tp_self, name))
					goto err_not_implemented;
				ASSERT(DeeType_GetOpPointer(tp_self, info) != NULL);
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

PRIVATE WUNUSED NONNULL((1)) dssize_t DCALL
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
			if (name == OPERATOR_SETRANGE) {
				DO(err, DeeFormat_Printf(printer, arg, " = %r", argv[2]));
			}
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
	OBJECT_HEAD
	DREF DeeTypeObject  *to_type; /* [1..1][const] The type who's operators should be enumerated. */
	DWEAK Dee_operator_t to_opid; /* Next operator ID to check. */
	bool                 to_name; /* [const] When true, try to assign human-readable names to operators. */
} TypeOperatorsIterator;

typedef struct {
	OBJECT_HEAD
	DREF DeeTypeObject *to_type; /* [1..1][const] The type who's operators should be enumerated. */
	bool                to_name; /* [const] When true, try to assign human-readable names to operators. */
} TypeOperators;

INTDEF DeeTypeObject TypeOperators_Type;
INTDEF DeeTypeObject TypeOperatorsIterator_Type;


PRIVATE NONNULL((1)) void DCALL
to_fini(TypeOperators *__restrict self) {
	Dee_Decref_unlikely(self->to_type);
}

PRIVATE NONNULL((1, 2)) void DCALL
to_visit(TypeOperators *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->to_type);
}

STATIC_ASSERT(offsetof(TypeOperatorsIterator, to_type) ==
              offsetof(TypeOperators, to_type));
#define toi_fini  to_fini
#define toi_visit to_visit

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
to_str(TypeOperators *__restrict self) {
	return DeeString_Newf("Operator %ss for %k",
	                      self->to_name ? "name" : "id",
	                      self->to_type);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
to_repr(TypeOperators *__restrict self) {
	return DeeString_Newf("%r.__operator%ss__",
	                      self->to_type,
	                      self->to_name ? "" : "id");
}

PRIVATE WUNUSED NONNULL((1)) DREF TypeOperatorsIterator *DCALL
to_iter(TypeOperators *__restrict self) {
	DREF TypeOperatorsIterator *result;
	result = DeeObject_MALLOC(TypeOperatorsIterator);
	if unlikely(!result)
		goto done;
	result->to_type = self->to_type;
	result->to_opid = 0;
	result->to_name = self->to_name;
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
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&to_iter,
	/* .tp_size      = */ NULL,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&to_contains
};

PRIVATE struct type_member tpconst to_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &TypeOperatorsIterator_Type),
	TYPE_MEMBER_END
};

#define TOI_GETOPID(x) atomic_read(&(x)->to_opid)

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
toi_copy(TypeOperatorsIterator *__restrict self,
         TypeOperatorsIterator *__restrict other) {
	self->to_type = other->to_type;
	self->to_opid = TOI_GETOPID(other);
	self->to_name = other->to_name;
	Dee_Incref(self->to_type);
	return 0;
}

#define DEFINE_TYPEOPREATORITEARTOR_COMPARE(name, op)                      \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL                  \
	name(TypeOperatorsIterator *self, TypeOperatorsIterator *other) {      \
		if (DeeObject_AssertTypeExact(other, &TypeOperatorsIterator_Type)) \
			goto err;                                                      \
		return_bool(TOI_GETOPID(self) op TOI_GETOPID(other));              \
	err:                                                                   \
		return NULL;                                                       \
	}
DEFINE_TYPEOPREATORITEARTOR_COMPARE(toi_eq, ==)
DEFINE_TYPEOPREATORITEARTOR_COMPARE(toi_ne, !=)
DEFINE_TYPEOPREATORITEARTOR_COMPARE(toi_lo, <)
DEFINE_TYPEOPREATORITEARTOR_COMPARE(toi_le, <=)
DEFINE_TYPEOPREATORITEARTOR_COMPARE(toi_gr, >)
DEFINE_TYPEOPREATORITEARTOR_COMPARE(toi_ge, >=)
#undef DEFINE_TYPEOPREATORITEARTOR_COMPARE

PRIVATE struct type_cmp toi_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&toi_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&toi_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&toi_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&toi_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&toi_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&toi_ge,
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
toi_next(TypeOperatorsIterator *__restrict self) {
	DeeTypeObject *tp = self->to_type;
	struct opinfo const *info;
	Dee_operator_t result;
	Dee_operator_t start;
	for (;;) {
		start  = atomic_read(&self->to_opid);
		result = start;
		for (;; ++result) {
			void const *my_ptr;

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
			if ((my_ptr = DeeType_GetOpPointer(tp, info)) != NULL &&
			    (!tp->tp_base || my_ptr != DeeType_GetOpPointer(tp->tp_base, info)))
				break;
		}
		if (atomic_cmpxch_weak_or_write(&self->to_opid, start, result + 1))
			break;
	}
	if (self->to_name && *info->oi_sname)
		return DeeString_New(info->oi_sname);
	return DeeInt_NewUInt16(result);
}


INTERN DeeTypeObject TypeOperatorsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_TypeOperatorsIterator",
	/* .tp_doc      = */ DOC("next->?X2?Dstring?Dint"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor        = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor   = */ (dfunptr_t)&toi_copy,
				/* .tp_deep_ctor   = */ (dfunptr_t)NULL,
				/* .tp_any_ctor    = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(TypeOperatorsIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&toi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&toi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &toi_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&toi_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor        = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor   = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor   = */ (dfunptr_t)NULL,
				/* .tp_any_ctor    = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(TypeOperators)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&to_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&to_str,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&to_repr,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&to_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &to_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ to_class_members
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
	return (DREF DeeObject *)result;
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
	return (DREF DeeObject *)result;
}


DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_OPERATOR_INFO_C */
