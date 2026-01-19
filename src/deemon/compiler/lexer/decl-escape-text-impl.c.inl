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
#define N 8
#endif /* __INTELLISENSE__ */

#include <deemon/api.h>

#include <deemon/string.h>
#include <deemon/system-features.h> /* memcpy */

#include <stddef.h> /* size_t */

/* Print the given `text' as encoded documentation text.
 *  - Escape any line-feed immediately following after another
 *  - Escape any instance of "->" with "-\>"
 *  - Escape any line starting with "(" as "\(" */
PRIVATE WUNUSED NONNULL((1, 3, 4)) int DCALL
PP_CAT2(decl_ast_escapetext, N)(PP_CAT3(uint, N, _t) const *__restrict text, size_t text_len,
                                struct unicode_printer *__restrict printer,
                                struct unicode_printer *__restrict source_printer) {
	PP_CAT3(uint, N, _t)
	const *iter, *end, *flush_start = text;
	/* Strip tailing whitespace (would otherwise be stripped by the doc API). */
	while (text_len && DeeUni_IsSpace(text[text_len - 1]))
		--text_len;
	end = (iter = text) + text_len;
	for (; iter < end; ++iter) {
		PP_CAT3(uint, N, _t)
		ch = *iter;
		if (ch == '\\') {
			if unlikely(PP_CAT2(unicode_printer_print, N)(printer, flush_start, (size_t)(iter - flush_start)) < 0)
				goto err;
			if (unicode_printer_putascii(printer, '\\'))
				goto err;
			flush_start = iter;
			continue;
		}
		if (ch == '-' && iter + 1 < end && iter[1] == '>') {
			++iter;
			if unlikely(PP_CAT2(unicode_printer_print, N)(printer, flush_start, (size_t)(iter - flush_start)) < 0)
				goto err;
			if (unicode_printer_putascii(printer, '\\'))
				goto err;
			flush_start = iter;
			continue;
		}
		if ((ch == '(' || ch == '\r' || ch == '\n') &&
		    (iter == text || iter[-1] == '\n' ||
		     (iter[-1] == '\r' && ch != '\n'))) {
			/* <START_OF_LINE>( / <START_OF_LINE><LF> */
			if unlikely(PP_CAT2(unicode_printer_print, N)(printer, flush_start, (size_t)(iter - flush_start)) < 0)
				goto err;
			if (unicode_printer_putascii(printer, '\\'))
				goto err;
			flush_start = iter;
			if (ch == '\r' && iter + 1 < end && iter[1] == '\n')
				++iter;
			continue;
		}
	}
	if (flush_start == text && UNICODE_PRINTER_ISEMPTY(printer)) {
		/* Steal all data from the source printer (that way we don't have to copy anything!). */
		memcpy(printer, source_printer, sizeof(struct unicode_printer));
		unicode_printer_init(source_printer);
	} else {
		if unlikely(PP_CAT2(unicode_printer_print, N)(printer, flush_start, (size_t)(end - flush_start)) < 0)
			goto err;
	}
	return 0;
err:
	return -1;
}

#undef N

