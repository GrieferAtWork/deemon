/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_REGEX_C
#define GUARD_DEEMON_OBJECTS_UNICODE_REGEX_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/regex.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/system-features.h>
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>

#include <hybrid/align.h>
#include <hybrid/bit.h>
#include <hybrid/minmax.h>
#include <hybrid/overflow.h>
#include <hybrid/unaligned.h>

#include <stdbool.h>
#include <stdint.h>
/**/

#include <__stdinc.h>
#include <hybrid/host.h>

DECL_BEGIN

/* Configure libregex */
#undef LIBREGEX_WANT_PROTOTYPES
#ifndef CONFIG_HAVE_malloc_usable_size
#define LIBREGEX_NO_MALLOC_USABLE_SIZE         /* Tell the library if we're unable to provide it with a working `malloc_usable_size(3)' function */
#endif /* !CONFIG_HAVE_malloc_usable_size */
#define LIBREGEX_NO_RE_CODE_DISASM             /* Don't need debug functions to disassemble regex byte code. */
#define LIBREGEX_NO_SYSTEM_INCLUDES            /* We're providing all of the system includes (so don't try to include any KOS headers) */
#define LIBREGEX_DECL                   INTDEF /* Declare the normally public API as INTERN (which we override again below) */
#define LIBREGEX_DEFINE___CTYPE_C_FLAGS        /* Can't rely on KOS's libc's `__ctype_C_flags' */
#define LIBREGEX_REGEXEC_SINGLE_CHUNK          /* Don't need (or want) iovec support */
#define LIBREGEX_USED__re_max_failures  200000 /* Use a hard-coded, fixed limit on how large the on-fail stack can grow */


/* Configure libregex syntax, and inject our `DEE_REGEX_COMPILE_*' flags. */
#undef RE_SYNTAX_ICASE
#undef RE_SYNTAX_NO_UTF8
#define LIBREGEX_CONSTANT__RE_SYNTAX_BACKSLASH_ESCAPE_IN_LISTS 1
#define LIBREGEX_CONSTANT__RE_SYNTAX_BK_PLUS_QM                0
#define LIBREGEX_CONSTANT__RE_SYNTAX_CHAR_CLASSES              1
#define LIBREGEX_CONSTANT__RE_SYNTAX_CONTEXT_INDEP_ANCHORS     1
#define LIBREGEX_CONSTANT__RE_SYNTAX_CONTEXT_INVALID_OPS       1
#define LIBREGEX_CONSTANT__RE_SYNTAX_DOT_NEWLINE               1 /* XXX: Flag? */
#define LIBREGEX_CONSTANT__RE_SYNTAX_DOT_NOT_NULL              0
#define LIBREGEX_CONSTANT__RE_SYNTAX_HAT_LISTS_NOT_NEWLINE     0 /* XXX: Flag? */
#define LIBREGEX_CONSTANT__RE_SYNTAX_INTERVALS                 1
#define LIBREGEX_CONSTANT__RE_SYNTAX_LIMITED_OPS               0
#define LIBREGEX_CONSTANT__RE_SYNTAX_NEWLINE_ALT               0
#define LIBREGEX_CONSTANT__RE_SYNTAX_NO_BK_BRACES              1
#define LIBREGEX_CONSTANT__RE_SYNTAX_NO_BK_PARENS              1
#define LIBREGEX_CONSTANT__RE_SYNTAX_NO_BK_REFS                0
#define LIBREGEX_CONSTANT__RE_SYNTAX_NO_BK_VBAR                1
#define LIBREGEX_CONSTANT__RE_SYNTAX_NO_EMPTY_RANGES           1
#define LIBREGEX_CONSTANT__RE_SYNTAX_UNMATCHED_RIGHT_PAREN_ORD 0
#define LIBREGEX_CONSTANT__RE_SYNTAX_NO_POSIX_BACKTRACKING     0
#define LIBREGEX_CONSTANT__RE_SYNTAX_NO_GNU_OPS                0
#define LIBREGEX_CONSTANT__RE_SYNTAX_INVALID_INTERVAL_ORD      0
#define RE_SYNTAX_ICASE                                        DEE_REGEX_COMPILE_ICASE
#define LIBREGEX_CONSTANT__RE_SYNTAX_CARET_ANCHORS_HERE        1
#define LIBREGEX_CONSTANT__RE_SYNTAX_CONTEXT_INVALID_DUP       1
#define LIBREGEX_CONSTANT__RE_SYNTAX_ANCHORS_IGNORE_EFLAGS     0
#define RE_SYNTAX_NO_UTF8                                      DEE_REGEX_COMPILE_NOUTF8
#define LIBREGEX_CONSTANT__RE_SYNTAX_NO_KOS_OPS                0

/* Prevent collisions with types declared by native system headers */
#undef re_regmatch_t
#undef re_regoff_t
#undef re_sregoff_t
#define re_regmatch_t _Dee_re_regmatch_t
#define re_regoff_t   _Dee_re_regoff_t
#define re_sregoff_t  _Dee_re_sregoff_t

/* Override libregex types & constants */
#undef re_code
#undef RE_REGOFF_UNSET
#undef RE_CODE_FLAG_NORMAL
#undef RE_CODE_FLAG_NEEDGROUPS
#undef RE_CODE_FLAG_OPTGROUPS
#define RE_REGOFF_UNSET         ((re_regoff_t)-1)
#define re_code                 DeeRegexCode
#define RE_CODE_FLAG_NORMAL     DEE_RE_CODE_FLAG_NORMAL
#define RE_CODE_FLAG_NEEDGROUPS DEE_RE_CODE_FLAG_NEEDGROUPS
#define RE_CODE_FLAG_OPTGROUPS  DEE_RE_CODE_FLAG_OPTGROUPS
#define __re_code_defined

#undef RE_EXEC_NOTBOL
#undef RE_EXEC_NOTEOL
#define RE_EXEC_NOTBOL DEE_RE_EXEC_NOTBOL
#define RE_EXEC_NOTEOL DEE_RE_EXEC_NOTEOL
#define __re_regmatch_t_defined
typedef struct DeeRegexMatch re_regmatch_t;

#define __re_regoff_t_defined
typedef size_t re_regoff_t;
typedef Dee_ssize_t re_sregoff_t;

#define __re_exec_defined
#define re_exec DeeRegexExec

#define re_compiler_yield(self)        re_parser_yield(&(self)->rec_parser)
#define re_compiler_yieldat(self, pos) re_parser_yieldat(&(self)->rec_parser, pos)

DECL_END

/* KOS compatibility emulation */
#include <deemon/util/kos-compat.h>

/* Define everything with PRIVATE scoping. */
#undef INTDEF
#undef INTERN
#undef DEFINE_PUBLIC_ALIAS
#define INTDEF PRIVATE
#define INTERN PRIVATE
#define DEFINE_PUBLIC_ALIAS(new, old) /* Disable exports */

/* Pull in libregex's public headers */
/* clang-format off */
#include "../../../libregex/include/regcomp.h" /* 1 */
#include "../../../libregex/include/regexec.h" /* 2 */
/* clang-format on */

/* Pull in libregex's source files. */
/* clang-format off */
#include "../../../libregex/regexec.c" /* 1 */
#include "../../../libregex/regpeep.c" /* 2 */
#include "../../../libregex/regfast.c" /* 3 */
#include "../../../libregex/regcomp.c" /* 4 */
/* clang-format on */

/* Restore normal binding macros */
#undef INTDEF
#undef INTERN
#undef DEFINE_PUBLIC_ALIAS
#define INTDEF __INTDEF
#define INTERN __INTERN
#define DEFINE_PUBLIC_ALIAS __DEFINE_PUBLIC_ALIAS

DECL_BEGIN

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

static_assert(DEE_RE_STATUS_NOMATCH == (-RE_NOMATCH));

PRIVATE ATTR_CONST ATTR_RETNONNULL WUNUSED
char const *DCALL re_strerror(re_errno_t error) {
	char const *result;
	switch (error) {
	case RE_BADPAT: /*  */ result = "Invalid regex pattern"; break;
	case RE_ECOLLATE: /**/ result = "Invalid collation character"; break;
	case RE_ECTYPE: /*  */ result = "Invalid character class name"; break;
	case RE_EESCAPE: /* */ result = "Trailing backslash"; break;
	case RE_ESUBREG: /* */ result = "Invalid back reference"; break;
	case RE_EBRACK: /*  */ result = "Unmatched [, [^, [:, [., or [="; break;
	case RE_EPAREN: /*  */ result = "Unmatched ("; break;
	case RE_EBRACE: /*  */ result = "Unmatched {"; break;
	case RE_BADBR: /*   */ result = "Invalid content of {...}"; break;
	case RE_ERANGE: /*  */ result = "Set-range start is greater than its end"; break;
	case RE_ESPACE: /*  */ result = "Out of memory"; break;
	case RE_BADRPT: /*  */ result = "Nothing precedes +, *, ?, or {"; break;
	case RE_EEND: /*    */ result = "Unexpected end of pattern"; break;
	case RE_ESIZE: /*   */ result = "Regular expression violates a hard upper limit"; break;
	case RE_ERPAREN: /* */ result = "Unmatched )"; break;
	case RE_EILLSEQ: /* */ result = "Illegal unicode character"; break;
	case RE_EILLSET: /* */ result = "Cannot combine raw bytes with unicode characters in charsets"; break;
	default: result = "Unknown regex error"; break;
	}
	return result;
}

PRIVATE int DCALL re_handle_error(re_errno_t error) {
	DeeTypeObject *error_type;
	char const *message;
	switch (error) {
	case RE_ESPACE:
		return Dee_CollectMemory(1) ? 0 : -1;
	case RE_ESIZE:    /* Regex pattern is too complex */
		error_type = &DeeError_ValueError;
		break;
	case RE_EILLSEQ:
	case RE_EILLSET:
		error_type = &DeeError_UnicodeError;
		break;
	case RE_ECOLLATE:
	case RE_ECTYPE:
		error_type = &DeeError_SymbolError;
		break;
	default:
		error_type = &DeeError_SyntaxError;
		break;
	}
	message = re_strerror(error);
	return DeeError_Throwf(error_type, "Regex error: %s", message);
}



/************************************************************************/
/* Regex execute                                                        */
/************************************************************************/

/* Perform a regex match
 * @return: >= 0: The # of matched bytes starting at `exec->rx_startoff'
 * @return: DEE_RE_STATUS_NOMATCH: Nothing was matched
 * @return: DEE_RE_STATUS_ERROR:   An error occurred */
PUBLIC WUNUSED NONNULL((1)) Dee_ssize_t DCALL
DeeRegex_Match(struct DeeRegexExec const *__restrict exec) {
	Dee_ssize_t result;
again:
	result = libre_exec_match(exec);
	if likely(result >= DEE_RE_STATUS_NOMATCH)
		return result;
	if (re_handle_error((re_errno_t)-result) == 0)
		goto again;
	return DEE_RE_STATUS_ERROR;
}

/* Similar to `DeeRegex_Match', try to match a pattern against the given input buffer. Do this
 * with increasing offsets for the first `search_range' bytes, meaning at most `search_range'
 * regex matches will be performed.
 * @param: search_range: One plus the max starting  byte offset (from `exec->rx_startoff')  to
 *                       check. Too great values for `search_range' are automatically clamped.
 * @param: p_match_size: When non-NULL, set to the # of bytes that were actually matched.
 *                       This would have  been the return  value of  `re_exec_match(3R)'.
 * @return: >= 0:        The offset where the matched area starts (in `[exec->rx_startoff, exec->rx_startoff + search_range)').
 * @return: DEE_RE_STATUS_NOMATCH: Nothing was matched
 * @return: DEE_RE_STATUS_ERROR:   An error occurred */
PUBLIC WUNUSED NONNULL((1)) Dee_ssize_t DCALL
DeeRegex_Search(struct DeeRegexExec const *__restrict exec,
                size_t search_range, size_t *p_match_size) {
	Dee_ssize_t result;
again:
	result = libre_exec_search(exec, search_range, p_match_size);
	if likely(result >= DEE_RE_STATUS_NOMATCH)
		return result;
	if (re_handle_error((re_errno_t)-result) == 0)
		goto again;
	return DEE_RE_STATUS_ERROR;
}

/* Similar to `DeeRegex_Search()', but never matches epsilon.
 * Instead, keep on searching if epsilon happens to be matched. */
PUBLIC WUNUSED NONNULL((1)) Dee_ssize_t DCALL
DeeRegex_SearchNoEpsilon(struct DeeRegexExec const *__restrict exec,
                         size_t search_range, size_t *p_match_size) {
	Dee_ssize_t result;
again:
	result = libre_exec_search_noepsilon(exec, search_range, p_match_size);
	if likely(result >= DEE_RE_STATUS_NOMATCH)
		return result;
	if (re_handle_error((re_errno_t)-result) == 0)
		goto again;
	return DEE_RE_STATUS_ERROR;
}


/* Same as `DeeRegex_Search', but perform searching with starting
 * offsets in `[exec->rx_endoff - search_range, exec->rx_endoff)'
 * Too great values for `search_range' are automatically clamped.
 * The return value will thus be the greatest byte-offset where
 * the given pattern matches that is still within that range. */
PUBLIC WUNUSED NONNULL((1)) Dee_ssize_t
DeeRegex_RSearch(struct DeeRegexExec const *__restrict exec,
                 size_t search_range, size_t *p_match_size) {
	Dee_ssize_t result;
again:
	result = libre_exec_rsearch(exec, search_range, p_match_size);
	if likely(result >= DEE_RE_STATUS_NOMATCH)
		return result;
	if (re_handle_error((re_errno_t)-result) == 0)
		goto again;
	return DEE_RE_STATUS_ERROR;
}




/************************************************************************/
/* Regex compile                                                        */
/************************************************************************/

/* Compile the regex pattern of a given string `self' */
PRIVATE WUNUSED NONNULL((1)) struct DeeRegexCode *DCALL
re_compile(DeeObject *__restrict self, unsigned int compile_flags) {
	re_errno_t comp_error;
	struct re_compiler comp;
	char *utf8;
again:
	utf8 = DeeString_AsUtf8(self);
	if unlikely(!utf8)
		goto err;

	/* Put together the regex compiler. */
	re_compiler_init(&comp,
	                 utf8,
	                 utf8 + WSTR_LENGTH(utf8),
	                 compile_flags);

	/* Initiate the compile. */
	comp_error = libre_compiler_compile(&comp);
	if unlikely(comp_error != RE_NOERROR)
		goto err_comp;

	/* Pack together the generated code. */
	return re_compiler_pack(&comp);
err_comp:
	re_compiler_fini(&comp);
	if (re_handle_error(comp_error) == 0)
		goto again;
err:
	return NULL;
}


struct regex_cache_entry {
#define REGEX_CACHE_DUMMY_STR ((DeeStringObject *)-1)
	DeeStringObject     *rce_str;    /* [0..1] Linked string (the string that owns the regex). */
	struct DeeRegexCode *rce_regex;  /* [valid_if(rce_str)] Compiled regex objects. */
	unsigned int         rce_syntax; /* [valid_if(rce_str)] Regex syntax */
};

#define regex_cache_entry_hashstr(str) Dee_HashPointer(str)

PRIVATE struct regex_cache_entry const regex_cache_empty[] = { { NULL, NULL, 0 } };

/* Regex cache */
PRIVATE struct regex_cache_entry *regex_cache_base = (struct regex_cache_entry *)regex_cache_empty;
PRIVATE size_t /*              */ regex_cache_mask = 0;
PRIVATE size_t /*              */ regex_cache_size = 0;
PRIVATE size_t /*              */ regex_cache_used = 0;
#ifndef CONFIG_NO_THREADS
PRIVATE Dee_atomic_rwlock_t regex_cache_lock = DEE_ATOMIC_RWLOCK_INIT;
#endif /* !CONFIG_NO_THREADS */

#define regex_cache_hashst(hash)        ((hash) & regex_cache_mask)
#define regex_cache_hashnx(hs, perturb) (void)((hs) = ((hs) << 2) + (hs) + (perturb) + 1, (perturb) >>= 5) /* This `5' is tunable. */
#define regex_cache_hashit(i)           (regex_cache_base + ((i) & regex_cache_mask))

PRIVATE bool DCALL regex_cache_rehash(int sizedir) {
	struct regex_cache_entry *new_vector, *iter, *end;
	size_t new_mask = regex_cache_mask;
	if (sizedir > 0) {
		new_mask = (new_mask << 1) | 1;
		if unlikely(new_mask == 1)
			new_mask = 16 - 1; /* Start out bigger than 2. */
	} else if (sizedir < 0) {
		if unlikely(!regex_cache_used) {
			ASSERT(!regex_cache_used);
			/* Special case: delete the vector. */
			if (regex_cache_base != regex_cache_empty)
				Dee_Free(regex_cache_base);
			regex_cache_base = (struct regex_cache_entry *)regex_cache_empty;
			regex_cache_mask = 0;
			regex_cache_size = 0;
			return true;
		}
		new_mask = (new_mask >> 1);
		if (regex_cache_used >= new_mask)
			return true;
	}
	ASSERT(regex_cache_used < new_mask);
	ASSERT(regex_cache_used <= regex_cache_size);
	new_vector = (struct regex_cache_entry *)Dee_TryCallocc(new_mask + 1,
	                                                        sizeof(struct regex_cache_entry));
	if unlikely(!new_vector)
		return false;
	new_vector = (struct regex_cache_entry *)Dee_UntrackAlloc(new_vector);
	ASSERT((regex_cache_base == regex_cache_empty) == (regex_cache_mask == 0));
	ASSERT((regex_cache_base == regex_cache_empty) == (regex_cache_size == 0));
	if (regex_cache_base != regex_cache_empty) {
		/* Re-insert all existing items into the new Dict vector. */
		end = (iter = regex_cache_base) + (regex_cache_mask + 1);
		for (; iter < end; ++iter) {
			struct regex_cache_entry *item;
			dhash_t i, perturb;
			/* Skip dummy keys. */
			if (!iter->rce_str || iter->rce_str == REGEX_CACHE_DUMMY_STR)
				continue;
			perturb = i = regex_cache_entry_hashstr(iter->rce_str) & new_mask;
			for (;; regex_cache_hashnx(i, perturb)) {
				item = &new_vector[i & new_mask];
				if (!item->rce_str)
					break; /* Empty slot found. */
			}
			/* Transfer this object. */
			memcpy(item, iter, sizeof(struct regex_cache_entry));
		}
		Dee_Free(regex_cache_base);
		/* With all dummy items gone, the size now equals what is actually used. */
		regex_cache_size = regex_cache_used;
	}
	ASSERT(regex_cache_size == regex_cache_used);
	regex_cache_mask = new_mask;
	regex_cache_base = new_vector;
	return true;
}


/* Destroy the regex cache associated with `self'.
 * Called from `DeeString_Type.tp_fini' when `STRING_UTF_FREGEX' was set. */
INTERN NONNULL((1)) void DCALL
DeeString_DestroyRegex(DeeStringObject *__restrict self) {
	struct regex_cache_entry *item, old_item;
	dhash_t i, perturb, hash;
	hash = regex_cache_entry_hashstr(self);
	Dee_atomic_rwlock_write(&regex_cache_lock);
	perturb = i = regex_cache_hashst(hash);
	for (;; regex_cache_hashnx(i, perturb)) {
		item = regex_cache_hashit(i);
		if (item->rce_str == NULL)
			break;
		if (item->rce_str == self) {
			item->rce_str = REGEX_CACHE_DUMMY_STR;
			Dee_Free(item->rce_regex);
			DBG_memset(&item->rce_syntax, 0xcc, sizeof(item->rce_syntax));
			DBG_memset(&item->rce_regex, 0xcc, sizeof(item->rce_regex));
			ASSERT(regex_cache_used);
			--regex_cache_used;
		}
	}
	memcpy(&old_item, item, sizeof(struct regex_cache_entry));
	if (regex_cache_used <= regex_cache_size / 3)
		regex_cache_rehash(-1);
	Dee_atomic_rwlock_endwrite(&regex_cache_lock);
}


/* Lazily compile `self' as a deemon regex pattern.
 * Regex patterns for strings are compiled once, and cached thereafter,
 * before being destroyed at the same time as the corresponding string.
 * @param: compile_flags: Set of `DEE_REGEX_COMPILE_*'
 * @param: rules:         When non-NULL, a string containing extra rules
 *                        that are or'd into `compile_flags'. For this purpose,
 *                        each character from `rules' is parsed as a flag:
 *                        - "i": DEE_REGEX_COMPILE_ICASE
 * @return: * :   The compiled regex pattern.
 * @return: NULL: An error occurred. */
PUBLIC WUNUSED NONNULL((1)) struct DeeRegexCode *DCALL
DeeString_GetRegex(/*String*/ DeeObject *__restrict self,
                   unsigned int compile_flags,
                   DeeObject *rules) {
	struct DeeRegexCode *result;
	struct regex_cache_entry *first_dummy;
	dhash_t i, perturb, hash;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);

	/* Parse `rules' (if given) */
	if (rules != NULL) {
		char const *iter;
		if (DeeObject_AssertTypeExact(rules, &DeeString_Type))
			goto err;
		iter = DeeString_STR(rules);
again_rules_iter:
		switch (*iter++) {

		case 'i':
			compile_flags |= DEE_REGEX_COMPILE_ICASE;
			goto again_rules_iter;

		case '\0':
			break;

		default:
			DeeError_Throwf(&DeeError_ValueError,
			                "Invalid regex rules string flag %:1q",
			                iter - 1);
			goto err;
		}
	}

	/* Lookup regex in cache */
	hash = regex_cache_entry_hashstr(self);
	Dee_atomic_rwlock_read(&regex_cache_lock);
	perturb = i = regex_cache_hashst(hash);
	for (;; regex_cache_hashnx(i, perturb)) {
		struct regex_cache_entry *item;
		item = regex_cache_hashit(i);
		if (!item->rce_str)
			break; /* End-of-hash-chain */
		if (item->rce_str == (DeeStringObject *)self &&
		    item->rce_syntax == compile_flags) {
			result = item->rce_regex;
			Dee_atomic_rwlock_endread(&regex_cache_lock);
			return result;
		}
	}
	Dee_atomic_rwlock_endread(&regex_cache_lock);

	/* Not found int cache -> create a new regex object. */
	result = re_compile(self, compile_flags);
	if unlikely(!result)
		goto err; /* Error */

	/* Store produced regex object in cache. */
again_lock_and_insert_result:
	Dee_atomic_rwlock_write(&regex_cache_lock);
again_insert_result:
	hash = regex_cache_entry_hashstr(self);
	first_dummy = NULL;
	perturb = i = regex_cache_hashst(hash);
	for (;; regex_cache_hashnx(i, perturb)) {
		struct regex_cache_entry *item;
		item = regex_cache_hashit(i);
		if (item->rce_str == NULL) {
			if (first_dummy == NULL)
				first_dummy = item;
			break; /* End-of-hash-chain */
		}
		if (item->rce_str == REGEX_CACHE_DUMMY_STR) {
			first_dummy = item;
			continue;
		}
		if (item->rce_str == (DeeStringObject *)self &&
		    item->rce_syntax == compile_flags) {
			struct DeeRegexCode *existing_regex;

			/* Race condition: another thread was faster (but use their result) */
			ASSERTF(((DeeStringObject *)self)->s_data != NULL,
			        "String is in regex cache, but doesn't have UTF-data allocated?");
			ASSERTF(((DeeStringObject *)self)->s_data->u_flags & STRING_UTF_FREGEX,
			        "String is in regex cache, but doesn't have regex-flag set?");
			existing_regex = item->rce_regex;
			Dee_atomic_rwlock_endwrite(&regex_cache_lock);
			Dee_Free(result);
			return existing_regex;
		}
	}

	/* String doesn't appear in regex cache, yet. */
	if ((first_dummy != NULL) &&
	    (regex_cache_size + 1 < regex_cache_mask ||
	     first_dummy->rce_str != NULL)) {
		/* Make sure that the string's regex flag is set. */
		bool wasdummy;
		struct string_utf *utf;
		utf = ((DeeStringObject *)self)->s_data;
		if (utf == NULL) {
			bool haslock = true;
			utf = Dee_string_utf_tryalloc();
			if unlikely(!utf) {
				Dee_atomic_rwlock_endwrite(&regex_cache_lock);
				utf = Dee_string_utf_alloc();
				haslock = false;
			}
			utf = (struct string_utf *)Dee_UntrackAlloc(utf);
			if unlikely(!atomic_cmpxch(&((DeeStringObject *)self)->s_data, NULL, utf)) {
				Dee_string_utf_free(utf);
				utf = ((DeeStringObject *)self)->s_data;
			}
			if unlikely(!haslock)
				goto again_lock_and_insert_result;
		}
		ASSERT(utf);

		/* Set the regex flag (so that the string destructor will later clean-up the regex cache) */
		atomic_or(&utf->u_flags, STRING_UTF_FREGEX);

		/* Remember the string within the regex cache. */
		ASSERT(first_dummy->rce_str == NULL ||
		       first_dummy->rce_str == REGEX_CACHE_DUMMY_STR);
		wasdummy = first_dummy->rce_str != NULL;
		first_dummy->rce_str    = (DeeStringObject *)self;
		first_dummy->rce_regex  = (struct DeeRegexCode *)Dee_UntrackAlloc(result);
		first_dummy->rce_syntax = compile_flags;
		++regex_cache_used;
		if (!wasdummy) {
			++regex_cache_size;
			if (regex_cache_size * 2 > regex_cache_mask)
				regex_cache_rehash(1);
		}
		Dee_atomic_rwlock_endwrite(&regex_cache_lock);
		return result;
	}

	/* Rehash and try again. */
	if (regex_cache_rehash(1))
		goto again_insert_result;
	Dee_atomic_rwlock_endwrite(&regex_cache_lock);
	if (Dee_CollectMemory(1))
		goto again_lock_and_insert_result;
err:
	return NULL;
}



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_REGEX_C */
