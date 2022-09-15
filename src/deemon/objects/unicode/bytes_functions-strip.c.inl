/* Copyright (c) 2018-2022 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2022 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifdef __INTELLISENSE__
#include "bytes_functions.c.inl"
#define DEFINE_bytes_strip
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
#endif /* __INTELLISENSE__ */

DECL_BEGIN

#if (defined(DEFINE_bytes_strip) +       \
     defined(DEFINE_bytes_lstrip) +      \
     defined(DEFINE_bytes_rstrip) +      \
     defined(DEFINE_bytes_sstrip) +      \
     defined(DEFINE_bytes_lsstrip) +     \
     defined(DEFINE_bytes_rsstrip) +     \
     defined(DEFINE_bytes_casestrip) +   \
     defined(DEFINE_bytes_caselstrip) +  \
     defined(DEFINE_bytes_caserstrip) +  \
     defined(DEFINE_bytes_casesstrip) +  \
     defined(DEFINE_bytes_caselsstrip) + \
     defined(DEFINE_bytes_casersstrip)) != 1
#error "Must #define exactly one of these macros!"
#endif /* ... */

#ifdef DEFINE_bytes_strip
#define LOCAL_bytes_strip      bytes_strip
#define LOCAL_bytes_strip_NAME "strip"
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_RSTRIP
#elif defined(DEFINE_bytes_lstrip)
#define LOCAL_bytes_strip      bytes_lstrip
#define LOCAL_bytes_strip_NAME "lstrip"
#define LOCAL_IS_LSTRIP
#elif defined(DEFINE_bytes_rstrip)
#define LOCAL_bytes_strip      bytes_rstrip
#define LOCAL_bytes_strip_NAME "rstrip"
#define LOCAL_IS_RSTRIP
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
#elif defined(DEFINE_bytes_rsstrip)
#define LOCAL_bytes_strip      bytes_rsstrip
#define LOCAL_bytes_strip_NAME "rsstrip"
#define LOCAL_IS_RSTRIP
#define LOCAL_IS_SSTRIP
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
#elif defined(DEFINE_bytes_caserstrip)
#define LOCAL_bytes_strip      bytes_caserstrip
#define LOCAL_bytes_strip_NAME "caserstrip"
#define LOCAL_IS_RSTRIP
#define LOCAL_IS_NOCASE
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
#elif defined(DEFINE_bytes_casersstrip)
#define LOCAL_bytes_strip      bytes_casersstrip
#define LOCAL_bytes_strip_NAME "casersstrip"
#define LOCAL_IS_RSTRIP
#define LOCAL_IS_SSTRIP
#define LOCAL_IS_NOCASE
#endif /* ... */

#ifdef LOCAL_IS_NOCASE
#define LOCAL_memchr dee_memasciicasechr
#define LOCAL_MEMEQB dee_memasciicaseeq
#else /* LOCAL_IS_NOCASE */
#define LOCAL_memchr memchr
#define LOCAL_MEMEQB MEMEQB
#endif /* !LOCAL_IS_NOCASE */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_bytes_strip(Bytes *self, size_t argc, DeeObject *const *argv) {
	uint8_t *begin;
#ifdef LOCAL_IS_SSTRIP
	Needle needle;
	size_t size;
#else /* LOCAL_IS_SSTRIP */
	uint8_t *end;
#endif /* !LOCAL_IS_SSTRIP */

	/* In sstrip-mode, the `mask' paramter becomes mandatory. */
#ifdef LOCAL_IS_SSTRIP
	DeeObject *mask;
	if (DeeArg_Unpack(argc, argv, "o:" LOCAL_bytes_strip_NAME, &mask))
		goto err;
#else /* LOCAL_IS_SSTRIP */
	DeeObject *mask = NULL;
	if (DeeArg_Unpack(argc, argv, "|o:" LOCAL_bytes_strip_NAME, &mask))
		goto err;
#endif /* !LOCAL_IS_SSTRIP */


	/* Do the actual split */
#ifdef LOCAL_IS_SSTRIP
	if (get_needle(&needle, mask))
		goto err;
	if unlikely(!needle.n_size)
		goto retself;
#define NEED_retself
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
		Needle needle;
		if (get_needle(&needle, mask))
			goto err;
#ifdef LOCAL_IS_LSTRIP
		while (begin < end && LOCAL_memchr(needle.n_data, *begin, needle.n_size))
			++begin;
#endif /* LOCAL_IS_LSTRIP */
#ifdef LOCAL_IS_RSTRIP
		while (end > begin && LOCAL_memchr(needle.n_data, end[-1], needle.n_size))
			--end;
#endif /* LOCAL_IS_RSTRIP */
	} else {
#ifdef LOCAL_IS_LSTRIP
		while (begin < end && DeeUni_IsSpace(*begin))
			++begin;
#endif /* LOCAL_IS_LSTRIP */
#ifdef LOCAL_IS_RSTRIP
		while (end > begin && DeeUni_IsSpace(end[-1]))
			--end;
#endif /* LOCAL_IS_RSTRIP */
	}
#endif /* !LOCAL_IS_SSTRIP */

	/* Check if the begin/end bounds remain unchanged. */
#if defined(LOCAL_IS_LSTRIP) && defined(LOCAL_IS_RSTRIP) && defined(LOCAL_IS_SSTRIP)
	if (begin == DeeBytes_DATA(self) && size == DeeBytes_SIZE(self))
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
#ifdef NEED_retself
#undef NEED_retself
retself:
#endif /* NEED_retself */
		return_reference_((DeeObject *)self);
	}

	/* Create a sub-view of `self' for the still-selected bytes range. */
#ifdef LOCAL_IS_SSTRIP
	return DeeBytes_NewSubView(self, begin, size);
#else /* LOCAL_IS_SSTRIP */
	return DeeBytes_NewSubView(self, begin, (size_t)(end - begin));
#endif /* !LOCAL_IS_SSTRIP */
err:
	return NULL;
}

#undef LOCAL_memchr
#undef LOCAL_MEMEQB

#undef LOCAL_bytes_strip
#undef LOCAL_bytes_strip_NAME

#undef LOCAL_IS_LSTRIP
#undef LOCAL_IS_RSTRIP
#undef LOCAL_IS_SSTRIP
#undef LOCAL_IS_NOCASE

DECL_END

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
