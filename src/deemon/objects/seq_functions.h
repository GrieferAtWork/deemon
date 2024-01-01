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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_FUNCTIONS_H
#define GUARD_DEEMON_OBJECTS_SEQ_FUNCTIONS_H 1

#include <deemon/api.h>
#include <deemon/object.h>

DECL_BEGIN

#define has_noninherited_seqfield(tp, seq, field)       \
	((seq)->field != NULL &&                            \
	 (!DeeType_Base(tp) || !DeeType_Base(tp)->tp_seq || \
	  DeeType_Base(tp)->tp_seq->field != (seq)->field))

#define has_noninherited_field(tp, field) \
	((tp)->field != NULL &&               \
	 (!DeeType_Base(tp) || DeeType_Base(tp)->field != (tp)->field))

#define is_noninherited_nsi(tp, seq, nsi) \
	(!DeeType_Base(tp) || DeeType_Base(tp)->tp_seq != (seq))

#define has_noninherited_getrange(tp, seq) has_noninherited_seqfield(tp, seq, tp_range_get)
#define has_noninherited_delrange(tp, seq) has_noninherited_seqfield(tp, seq, tp_range_del)
#define has_noninherited_setrange(tp, seq) has_noninherited_seqfield(tp, seq, tp_range_set)
#define has_noninherited_getitem(tp, seq)  has_noninherited_seqfield(tp, seq, tp_get)
#define has_noninherited_delitem(tp, seq)  has_noninherited_seqfield(tp, seq, tp_del)
#define has_noninherited_setitem(tp, seq)  has_noninherited_seqfield(tp, seq, tp_set)
#define has_noninherited_size(tp, seq)     has_noninherited_seqfield(tp, seq, tp_size)
#define has_noninherited_bool(tp)          has_noninherited_field(tp, tp_cast.tp_bool)

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_FUNCTIONS_H */
