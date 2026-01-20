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
#ifdef __INTELLISENSE__
#include "bytes_functions.c.inl"
//#define DEFINE_bytes_strip
//#define DEFINE_bytes_lstrip
//#define DEFINE_bytes_rstrip
//#define DEFINE_bytes_sstrip
//#define DEFINE_bytes_lsstrip
//#define DEFINE_bytes_rsstrip
//#define DEFINE_bytes_casestrip
//#define DEFINE_bytes_caselstrip
//#define DEFINE_bytes_caserstrip
//#define DEFINE_bytes_casesstrip
//#define DEFINE_bytes_caselsstrip
//#define DEFINE_bytes_casersstrip
//#define DEFINE_bytes_striplines
//#define DEFINE_bytes_lstriplines
#define DEFINE_bytes_rstriplines
//#define DEFINE_bytes_sstriplines
//#define DEFINE_bytes_lsstriplines
//#define DEFINE_bytes_rsstriplines
//#define DEFINE_bytes_casestriplines
//#define DEFINE_bytes_caselstriplines
//#define DEFINE_bytes_caserstriplines
//#define DEFINE_bytes_casesstriplines
//#define DEFINE_bytes_caselsstriplines
//#define DEFINE_bytes_casersstriplines
#endif /* __INTELLISENSE__ */

#include <deemon/api.h>

#include <deemon/arg.h>
#include <deemon/bytes.h>
#include <deemon/object.h>
#include <deemon/string.h>

#include "string_functions.h"

#include <stddef.h> /* NULL, size_t */

DECL_BEGIN

#if (defined(DEFINE_bytes_strip) +            \
     defined(DEFINE_bytes_lstrip) +           \
     defined(DEFINE_bytes_rstrip) +           \
     defined(DEFINE_bytes_sstrip) +           \
     defined(DEFINE_bytes_lsstrip) +          \
     defined(DEFINE_bytes_rsstrip) +          \
     defined(DEFINE_bytes_casestrip) +        \
     defined(DEFINE_bytes_caselstrip) +       \
     defined(DEFINE_bytes_caserstrip) +       \
     defined(DEFINE_bytes_casesstrip) +       \
     defined(DEFINE_bytes_caselsstrip) +      \
     defined(DEFINE_bytes_casersstrip) +      \
     defined(DEFINE_bytes_striplines) +       \
     defined(DEFINE_bytes_lstriplines) +      \
     defined(DEFINE_bytes_rstriplines) +      \
     defined(DEFINE_bytes_sstriplines) +      \
     defined(DEFINE_bytes_lsstriplines) +     \
     defined(DEFINE_bytes_rsstriplines) +     \
     defined(DEFINE_bytes_casestriplines) +   \
     defined(DEFINE_bytes_caselstriplines) +  \
     defined(DEFINE_bytes_caserstriplines) +  \
     defined(DEFINE_bytes_casesstriplines) +  \
     defined(DEFINE_bytes_caselsstriplines) + \
     defined(DEFINE_bytes_casersstriplines)) != 1
#error "Must #define exactly one of these macros!"
#endif /* ... */

#ifndef SIZE_MAX
#include <hybrid/limitcore.h> /* __SIZE_MAX__ */
#ifndef SIZE_MAX
#define SIZE_MAX __SIZE_MAX__
#endif /* !SIZE_MAX */
#endif /* !SIZE_MAX */

#ifdef DEFINE_bytes_strip
#define LOCAL_bytes_strip      bytes_strip
#define LOCAL_bytes_strip_NAME "strip"
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_RSTRIP
#elif defined(DEFINE_bytes_lstrip)
#define LOCAL_bytes_strip      bytes_lstrip
#define LOCAL_bytes_strip_NAME "lstrip"
#define LOCAL_IS_LSTRIP
#define LOCAL_HAVE_max_count
#elif defined(DEFINE_bytes_rstrip)
#define LOCAL_bytes_strip      bytes_rstrip
#define LOCAL_bytes_strip_NAME "rstrip"
#define LOCAL_IS_RSTRIP
#define LOCAL_HAVE_max_count
#elif defined(DEFINE_bytes_sstrip)
#define LOCAL_bytes_strip      bytes_sstrip
#define LOCAL_bytes_strip_NAME "sstrip"
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_RSTRIP
#define LOCAL_IS_SSTRIP
#elif defined(DEFINE_bytes_lsstrip)
#define LOCAL_bytes_strip      bytes_lsstrip
#define LOCAL_bytes_strip_NAME "lsstrip"
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_SSTRIP
#define LOCAL_HAVE_max_count
#elif defined(DEFINE_bytes_rsstrip)
#define LOCAL_bytes_strip      bytes_rsstrip
#define LOCAL_bytes_strip_NAME "rsstrip"
#define LOCAL_IS_RSTRIP
#define LOCAL_IS_SSTRIP
#define LOCAL_HAVE_max_count
#elif defined(DEFINE_bytes_casestrip)
#define LOCAL_bytes_strip      bytes_casestrip
#define LOCAL_bytes_strip_NAME "casestrip"
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_RSTRIP
#define LOCAL_IS_NOCASE
#elif defined(DEFINE_bytes_caselstrip)
#define LOCAL_bytes_strip      bytes_caselstrip
#define LOCAL_bytes_strip_NAME "caselstrip"
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_NOCASE
#define LOCAL_HAVE_max_count
#elif defined(DEFINE_bytes_caserstrip)
#define LOCAL_bytes_strip      bytes_caserstrip
#define LOCAL_bytes_strip_NAME "caserstrip"
#define LOCAL_IS_RSTRIP
#define LOCAL_IS_NOCASE
#define LOCAL_HAVE_max_count
#elif defined(DEFINE_bytes_casesstrip)
#define LOCAL_bytes_strip      bytes_casesstrip
#define LOCAL_bytes_strip_NAME "casesstrip"
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_RSTRIP
#define LOCAL_IS_SSTRIP
#define LOCAL_IS_NOCASE
#elif defined(DEFINE_bytes_caselsstrip)
#define LOCAL_bytes_strip      bytes_caselsstrip
#define LOCAL_bytes_strip_NAME "caselsstrip"
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_SSTRIP
#define LOCAL_IS_NOCASE
#define LOCAL_HAVE_max_count
#elif defined(DEFINE_bytes_casersstrip)
#define LOCAL_bytes_strip      bytes_casersstrip
#define LOCAL_bytes_strip_NAME "casersstrip"
#define LOCAL_IS_RSTRIP
#define LOCAL_IS_SSTRIP
#define LOCAL_IS_NOCASE
#define LOCAL_HAVE_max_count
#elif defined(DEFINE_bytes_striplines)
#define LOCAL_bytes_strip      bytes_striplines
#define LOCAL_bytes_strip_NAME "striplines"
#define LOCAL_IS_LINES
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_RSTRIP
#elif defined(DEFINE_bytes_lstriplines)
#define LOCAL_bytes_strip      bytes_lstriplines
#define LOCAL_bytes_strip_NAME "lstriplines"
#define LOCAL_IS_LINES
#define LOCAL_IS_LSTRIP
#elif defined(DEFINE_bytes_rstriplines)
#define LOCAL_bytes_strip      bytes_rstriplines
#define LOCAL_bytes_strip_NAME "rstriplines"
#define LOCAL_IS_LINES
#define LOCAL_IS_RSTRIP
#elif defined(DEFINE_bytes_sstriplines)
#define LOCAL_bytes_strip      bytes_sstriplines
#define LOCAL_bytes_strip_NAME "sstriplines"
#define LOCAL_IS_LINES
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_RSTRIP
#define LOCAL_IS_SSTRIP
#elif defined(DEFINE_bytes_lsstriplines)
#define LOCAL_bytes_strip      bytes_lsstriplines
#define LOCAL_bytes_strip_NAME "lsstriplines"
#define LOCAL_IS_LINES
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_SSTRIP
#elif defined(DEFINE_bytes_rsstriplines)
#define LOCAL_bytes_strip      bytes_rsstriplines
#define LOCAL_bytes_strip_NAME "rsstriplines"
#define LOCAL_IS_LINES
#define LOCAL_IS_RSTRIP
#define LOCAL_IS_SSTRIP
#elif defined(DEFINE_bytes_casestriplines)
#define LOCAL_bytes_strip      bytes_casestriplines
#define LOCAL_bytes_strip_NAME "casestriplines"
#define LOCAL_IS_LINES
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_RSTRIP
#define LOCAL_IS_NOCASE
#elif defined(DEFINE_bytes_caselstriplines)
#define LOCAL_bytes_strip      bytes_caselstriplines
#define LOCAL_bytes_strip_NAME "caselstriplines"
#define LOCAL_IS_LINES
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_NOCASE
#elif defined(DEFINE_bytes_caserstriplines)
#define LOCAL_bytes_strip      bytes_caserstriplines
#define LOCAL_bytes_strip_NAME "caserstriplines"
#define LOCAL_IS_LINES
#define LOCAL_IS_RSTRIP
#define LOCAL_IS_NOCASE
#elif defined(DEFINE_bytes_casesstriplines)
#define LOCAL_bytes_strip      bytes_casesstriplines
#define LOCAL_bytes_strip_NAME "casesstriplines"
#define LOCAL_IS_LINES
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_RSTRIP
#define LOCAL_IS_SSTRIP
#define LOCAL_IS_NOCASE
#elif defined(DEFINE_bytes_caselsstriplines)
#define LOCAL_bytes_strip      bytes_caselsstriplines
#define LOCAL_bytes_strip_NAME "caselsstriplines"
#define LOCAL_IS_LINES
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_SSTRIP
#define LOCAL_IS_NOCASE
#elif defined(DEFINE_bytes_casersstriplines)
#define LOCAL_bytes_strip      bytes_casersstriplines
#define LOCAL_bytes_strip_NAME "casersstriplines"
#define LOCAL_IS_LINES
#define LOCAL_IS_RSTRIP
#define LOCAL_IS_SSTRIP
#define LOCAL_IS_NOCASE
#endif /* ... */

#ifdef LOCAL_IS_NOCASE
#define LOCAL_memchr memasciicasechr
#define LOCAL_MEMEQB memasciicaseeq
#else /* LOCAL_IS_NOCASE */
#define LOCAL_memchr memchr
#define LOCAL_MEMEQB MEMEQB
#endif /* !LOCAL_IS_NOCASE */


/* When stripping lines, linefeeds themselves mustn't count as whitespace */
#ifdef LOCAL_IS_LINES
#define LOCAL_isspace DeeUni_IsSpaceNoLf
#else /* LOCAL_IS_LINES */
#define LOCAL_isspace DeeUni_IsSpace
#endif /* !LOCAL_IS_LINES */


/* Suppress warning because compilers don't like the way we use `needle':
 * >> if (mask) {
 * >>     acquire_needle(&needle, mask);
 * >> }
 * >> ...
 * >> if (mask) {
 * >>     use_needle(&needle); // <<< Here
 * >> } */
__pragma_GCC_diagnostic_push_ignored(Wmaybe_uninitialized)

#ifdef LOCAL_HAVE_max_count
#ifdef LOCAL_IS_SSTRIP
#define LOCAL_kwlist kwlist__needle_max
#else /* LOCAL_IS_SSTRIP */
#define LOCAL_kwlist kwlist__mask_max
#endif /* !LOCAL_IS_SSTRIP */

PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL
LOCAL_bytes_strip(Bytes *self, size_t argc,
                  DeeObject *const *argv, DeeObject *kw)
#else /* LOCAL_HAVE_max_count */
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL
LOCAL_bytes_strip(Bytes *self, size_t argc, DeeObject *const *argv)
#endif /* !LOCAL_HAVE_max_count */
{
	byte_t *begin;
	Needle needle;
#ifdef LOCAL_IS_SSTRIP
	size_t size;
#else /* LOCAL_IS_SSTRIP */
	byte_t *end;
#endif /* !LOCAL_IS_SSTRIP */
#ifdef LOCAL_HAVE_max_count
	size_t max_count = SIZE_MAX;
#define LOCAL_max_count_OR_true max_count
#define LOCAL_max_count_dec()   (void)--max_count
#else /* LOCAL_HAVE_max_count */
#define LOCAL_max_count_OR_true 1
#define LOCAL_max_count_dec()   (void)0
#endif /* !LOCAL_HAVE_max_count */
#ifdef LOCAL_IS_LINES
#ifdef LOCAL_IS_SSTRIP
	byte_t *end;
#endif /* LOCAL_IS_SSTRIP */
	struct bytes_printer printer;
	byte_t *flush_start;
#endif /* LOCAL_IS_LINES */
	DeeObject *mask;

	/* In sstrip-mode, the `mask' parameter becomes mandatory. */
#ifdef LOCAL_HAVE_max_count
#ifdef LOCAL_IS_SSTRIP
	if (DeeArg_UnpackKw(argc, argv, kw, LOCAL_kwlist, "o|" UNPuSIZ ":" LOCAL_bytes_strip_NAME, &mask, &max_count))
		goto err;
#else /* LOCAL_IS_SSTRIP */
	mask = NULL;
	if (DeeArg_UnpackKw(argc, argv, kw, LOCAL_kwlist, "|o" UNPuSIZ ":" LOCAL_bytes_strip_NAME, &mask, &max_count))
		goto err;
#endif /* !LOCAL_IS_SSTRIP */
#else /* LOCAL_HAVE_max_count */
#ifdef LOCAL_IS_SSTRIP
	DeeArg_Unpack1(err, argc, argv, LOCAL_bytes_strip_NAME, &mask);
#else /* LOCAL_IS_SSTRIP */
	mask = NULL;
	DeeArg_Unpack0Or1(err, argc, argv, LOCAL_bytes_strip_NAME, &mask);
#endif /* !LOCAL_IS_SSTRIP */
#endif /* !LOCAL_HAVE_max_count */

	/* Do the actual strip at the front/back */
#ifdef LOCAL_IS_SSTRIP
	if (acquire_needle(&needle, mask)) /* TODO: release_needle() */
		goto err;
	if unlikely(!needle.n_size)
		goto retself_noprinter;
#define NEED_retself_noprinter
	begin = DeeBytes_DATA(self);
	size = DeeBytes_SIZE(self);
#ifdef LOCAL_IS_LSTRIP
	while (size >= needle.n_size) {
		if (!LOCAL_MEMEQB(begin, needle.n_data, needle.n_size))
			break;
		begin += needle.n_size;
		size -= needle.n_size;
	}
#endif /* LOCAL_IS_LSTRIP */
#ifdef LOCAL_IS_RSTRIP
	while (size >= needle.n_size) {
		if (!LOCAL_MEMEQB(begin + size - needle.n_size, needle.n_data, needle.n_size))
			break;
		size -= needle.n_size;
	}
#endif /* LOCAL_IS_RSTRIP */
#else /* LOCAL_IS_SSTRIP */
	begin = DeeBytes_DATA(self);
	end = begin + DeeBytes_SIZE(self);
	if (mask) {
		/* Deal with a custom strip-character/sequence mask. */
#ifndef LOCAL_IS_SSTRIP
		if (acquire_needle(&needle, mask)) /* TODO: release_needle() */
			goto err;
#endif /* !LOCAL_IS_SSTRIP */
#ifdef LOCAL_IS_LSTRIP
		while (begin < end && LOCAL_max_count_OR_true &&
		       LOCAL_memchr(needle.n_data, *begin, needle.n_size)) {
			++begin;
			LOCAL_max_count_dec();
		}
#endif /* LOCAL_IS_LSTRIP */
#ifdef LOCAL_IS_RSTRIP
		while (end > begin && LOCAL_max_count_OR_true &&
		       LOCAL_memchr(needle.n_data, end[-1], needle.n_size)) {
			--end;
			LOCAL_max_count_dec();
		}
#endif /* LOCAL_IS_RSTRIP */
	} else {
#ifdef LOCAL_IS_LSTRIP
		while (begin < end && LOCAL_max_count_OR_true &&
		       LOCAL_isspace(*begin)) {
			++begin;
			LOCAL_max_count_dec();
		}
#endif /* LOCAL_IS_LSTRIP */
#ifdef LOCAL_IS_RSTRIP
		while (end > begin && LOCAL_max_count_OR_true &&
		       LOCAL_isspace(end[-1])) {
			--end;
			LOCAL_max_count_dec();
		}
#endif /* LOCAL_IS_RSTRIP */
	}
#endif /* ... */

	/* Strip around line-feeds and print to a secondary bytes buffer. */
#ifdef LOCAL_IS_LINES
#ifdef LOCAL_IS_SSTRIP
	end = begin + size;
#endif /* LOCAL_IS_SSTRIP */
	flush_start = begin;
	bytes_printer_init(&printer);
	if (mask) {
		/* Deal with a custom strip-character/sequence mask. */
		while (begin < end) {
			byte_t ch = *begin;
			if (ch != ASCII_CR && ch != ASCII_LF) {
				++begin;
				continue; /* Not a line-feed character */
			}

#ifdef LOCAL_IS_RSTRIP
			/* rstrip (at the end of lines) */
			{
				byte_t *flush_end = begin;
#ifdef LOCAL_IS_SSTRIP
				while (flush_end >= (flush_start + needle.n_size) &&
				       LOCAL_MEMEQB(flush_end - needle.n_size, needle.n_data, needle.n_size))
					flush_end -= needle.n_size;
#else /* LOCAL_IS_SSTRIP */
				while (flush_end > flush_start && LOCAL_memchr(needle.n_data, flush_end[-1], needle.n_size))
					--flush_end;
#endif /* !LOCAL_IS_SSTRIP */
				if (flush_end < begin) {
					if unlikely(bytes_printer_append(&printer, flush_start,
					                                 (size_t)(flush_end - flush_start)) < 0)
						goto err_printer;
					flush_start = begin; /* Skip over whitespace */
				}
			}
#endif /* LOCAL_IS_RSTRIP */

			/* Skip over the linefeed (preserving it) */
			++begin;
			if (ch == ASCII_CR && begin < end && *begin == ASCII_LF)
				++begin; /* Deal with CRLF-style linefeeds */

#ifdef LOCAL_IS_LSTRIP
			/* lstrip (at the start of lines) */
			{
				byte_t *new_flush_start = begin;
#ifdef LOCAL_IS_SSTRIP
				while ((new_flush_start + needle.n_size) <= end &&
				       LOCAL_MEMEQB(new_flush_start, needle.n_data, needle.n_size))
					new_flush_start += needle.n_size;
#else /* LOCAL_IS_SSTRIP */
				while (new_flush_start < end && LOCAL_isspace(*new_flush_start))
					++new_flush_start;
#endif /* !LOCAL_IS_SSTRIP */
				if (new_flush_start > begin) {
					if unlikely(bytes_printer_append(&printer, flush_start,
					                                 (size_t)(begin - flush_start)) < 0)
						goto err_printer;
					flush_start = new_flush_start; /* Skip over whitespace */
					begin       = new_flush_start;
				}
			}
#endif /* LOCAL_IS_LSTRIP */
		}
	} else {
		while (begin < end) {
			byte_t ch = *begin;
			if (ch != ASCII_CR && ch != ASCII_LF) {
				++begin;
				continue; /* Not a line-feed character */
			}

#ifdef LOCAL_IS_RSTRIP
			/* rstrip (at the end of lines) */
			{
				byte_t *flush_end = begin;
				while (flush_end > flush_start && LOCAL_isspace(flush_end[-1]))
					--flush_end;
				if (flush_end < begin) {
					if unlikely(bytes_printer_append(&printer, flush_start,
					                                 (size_t)(flush_end - flush_start)) < 0)
						goto err_printer;
					flush_start = begin; /* Skip over whitespace */
				}
			}
#endif /* LOCAL_IS_RSTRIP */

			/* Skip over the linefeed (preserving it) */
			++begin;
			if (ch == ASCII_CR && begin < end && *begin == ASCII_LF)
				++begin; /* Deal with CRLF-style linefeeds */

#ifdef LOCAL_IS_LSTRIP
			/* lstrip (at the start of lines) */
			{
				byte_t *new_flush_start = begin;
				while (new_flush_start < end && LOCAL_isspace(*new_flush_start))
					++new_flush_start;
				if (new_flush_start > begin) {
					if unlikely(bytes_printer_append(&printer, flush_start,
					                                 (size_t)(begin - flush_start)) < 0)
						goto err_printer;
					flush_start = new_flush_start; /* Skip over whitespace */
					begin       = new_flush_start;
				}
			}
#endif /* LOCAL_IS_LSTRIP */
		}
	}
#endif /* LOCAL_IS_LINES */

	/* Check if the begin/end bounds remain unchanged. */
#ifdef LOCAL_IS_LINES
	if (!BYTES_PRINTER_SIZE(&printer))
#endif /* LOCAL_IS_LINES */
	{
#if defined(LOCAL_IS_LSTRIP) && defined(LOCAL_IS_RSTRIP) && defined(LOCAL_IS_SSTRIP)
		if (size == DeeBytes_SIZE(self))
#elif defined(LOCAL_IS_LSTRIP) && defined(LOCAL_IS_RSTRIP)
		if (begin == DeeBytes_DATA(self) && end == begin + DeeBytes_SIZE(self))
#elif defined(LOCAL_IS_LSTRIP)
		if (begin == DeeBytes_DATA(self))
#elif defined(LOCAL_IS_RSTRIP) && defined(LOCAL_IS_SSTRIP)
		if (size == DeeBytes_SIZE(self))
#elif defined(LOCAL_IS_RSTRIP)
		if (end == begin + DeeBytes_SIZE(self))
#endif /* ... */
		{
#ifdef LOCAL_IS_LINES
			bytes_printer_fini(&printer);
#endif /* LOCAL_IS_LINES */
#ifdef NEED_retself_noprinter
#undef NEED_retself_noprinter
retself_noprinter:
#endif /* NEED_retself_noprinter */
			return_reference_(self);
		}
		/* Create a sub-view of `self' for the still-selected bytes range. */
#ifdef LOCAL_IS_SSTRIP
		return (DREF Bytes *)DeeBytes_NewSubView(self, begin, size);
#else /* LOCAL_IS_SSTRIP */
		return (DREF Bytes *)DeeBytes_NewSubView(self, begin, (size_t)(end - begin));
#endif /* !LOCAL_IS_SSTRIP */
	}

#ifdef LOCAL_IS_LINES
	ASSERT(begin == end);
	if unlikely(bytes_printer_append(&printer, flush_start,
	                                 (size_t)(begin - flush_start)) < 0)
		goto err_printer;
	return (DREF Bytes *)bytes_printer_pack(&printer);
err_printer:
	bytes_printer_fini(&printer);
#endif /* LOCAL_IS_LINES */
err:
	return NULL;
}

__pragma_GCC_diagnostic_pop_ignored(Wmaybe_uninitialized)

#undef LOCAL_max_count_OR_true
#undef LOCAL_max_count_dec
#undef LOCAL_kwlist
#undef LOCAL_HAVE_max_count

#undef LOCAL_isspace

#undef LOCAL_memchr
#undef LOCAL_MEMEQB

#undef LOCAL_bytes_strip
#undef LOCAL_bytes_strip_NAME

#undef LOCAL_IS_LINES
#undef LOCAL_IS_LSTRIP
#undef LOCAL_IS_RSTRIP
#undef LOCAL_IS_SSTRIP
#undef LOCAL_IS_NOCASE

DECL_END

#undef DEFINE_bytes_casersstriplines
#undef DEFINE_bytes_caselsstriplines
#undef DEFINE_bytes_casesstriplines
#undef DEFINE_bytes_caserstriplines
#undef DEFINE_bytes_caselstriplines
#undef DEFINE_bytes_casestriplines
#undef DEFINE_bytes_rsstriplines
#undef DEFINE_bytes_lsstriplines
#undef DEFINE_bytes_sstriplines
#undef DEFINE_bytes_rstriplines
#undef DEFINE_bytes_lstriplines
#undef DEFINE_bytes_striplines
#undef DEFINE_bytes_casersstrip
#undef DEFINE_bytes_caselsstrip
#undef DEFINE_bytes_casesstrip
#undef DEFINE_bytes_caserstrip
#undef DEFINE_bytes_caselstrip
#undef DEFINE_bytes_casestrip
#undef DEFINE_bytes_rsstrip
#undef DEFINE_bytes_lsstrip
#undef DEFINE_bytes_sstrip
#undef DEFINE_bytes_rstrip
#undef DEFINE_bytes_lstrip
#undef DEFINE_bytes_strip
