/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_COLLECTIONS_LIBCOLLECTIONS_C
#define GUARD_DEX_COLLECTIONS_LIBCOLLECTIONS_C 1
#define DEE_SOURCE

#include "libcollections.h"
/**/

#include <deemon/api.h>
#include <deemon/cached-dict.h>
#include <deemon/dex.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/seq.h>

#include "accu.h"

DECL_BEGIN

#define Q3  "??" "?"
#define OPNAME(opname) "operator " opname

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_changed_sequence(DeeObject *__restrict seq) {
	ASSERT_OBJECT(seq);
	return DeeError_Throwf(&DeeError_RuntimeError,
	                       "A sequence `%k' has changed while being iterated: `%k'",
	                       Dee_TYPE(seq), seq);
}

INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_unimplemented_operator)(DeeTypeObject const *__restrict tp, Dee_operator_t operator_name) {
	struct opinfo const *info;
	info = DeeTypeType_GetOperatorById(Dee_TYPE(tp), operator_name);
	ASSERT_OBJECT(tp);
	return DeeError_Throwf(&DeeError_NotImplemented,
	                       "Operator `%r." OPNAME("%s") "' is not implemented",
	                       tp, info ? info->oi_sname : Q3);
}

PRIVATE struct dex_symbol symbols[] = {
	{ "Accu", (DeeObject *)&Accu_Type, MODSYM_FREADONLY },
	{ "Deque", (DeeObject *)&Deque_Type, MODSYM_FREADONLY },
	{ "FixedList", (DeeObject *)&FixedList_Type, MODSYM_FREADONLY },
	{ "UniqueDict", (DeeObject *)&UDict_Type, MODSYM_FREADONLY },
	{ "UniqueSet", (DeeObject *)&USet_Type, MODSYM_FREADONLY },
	{ "RangeMap", (DeeObject *)&RangeMap_Type, MODSYM_FREADONLY },
	{ "RBTree", (DeeObject *)&RBTree_Type, MODSYM_FREADONLY },
	{ "CachedDict", (DeeObject *)&DeeCachedDict_Type, MODSYM_FREADONLY },
	{ "Bitset", (DeeObject *)&Bitset_Type, MODSYM_FREADONLY },
	{ "BitsetView", (DeeObject *)&BitsetView_Type, MODSYM_FREADONLY },
	{ "bits", (DeeObject *)&BitsetView_Type, MODSYM_FREADONLY,
	  DOC("Alias for ?GBitsetView, that should be used as a "
	      /**/ "function to access the bits of buffer-like objects") },

	/* TODO: FixedSet  (set that can only contain a limited set of keys and is internally just a Bitset):
	 * >> class FixedSet: Set {
	 * >>      private final member _keys: {Object: int};
	 * >>      private final member _present: Bitset;
	 * >>      this(keys: {Object...}) {
	 * >>          _keys = Mapping.frozen(for (local i, key: Sequence.enumerate(keys)) (key, i));
	 * >>          _present = Bitset(#_keys);
	 * >>      }
	 * >>      operator contains(ob) {
	 * >>          local index = _keys.get(ob);
	 * >>          if (index is none)
	 * >>              return false;
	 * >>          return _present[index];
	 * >>      }
	 * >> };
	 */
	/* TODO: FixedDict  (dict that can only contain a limited set of keys)
	 * >> class FixedDict: Mapping {
	 * >>      private final member _keys: {Object: int};
	 * >>      private final member _present: FixedList;
	 * >>      this(keys: {Object...}) {
	 * >>          _keys = Mapping.frozen(for (local i, key: Sequence.enumerate(keys)) (key, i));
	 * >>          _present = FixedList(#_keys);
	 * >>      }
	 * >>      operator [] (key) {
	 * >>          local index = _keys.get(ob);
	 * >>          if (index is none)
	 * >>              throw KeyError(...);
	 * >>          return _present[index]; // Can throw UnboundItem
	 * >>      }
	 * >>      operator del[] (key) {
	 * >>          local index = _keys.get(ob);
	 * >>          if (index is none)
	 * >>              throw KeyError(...);
	 * >>          del _present[index];
	 * >>      }
	 * >>      operator []= (key, value) {
	 * >>          local index = _keys.get(ob);
	 * >>          if (index is none)
	 * >>              throw KeyError(...);
	 * >>          _present[index] = value;
	 * >>      }
	 * >> };
	 */

	/* TODO: STailQ (singly linked list; internally: STAILQ) */
	/* TODO: TailQ (double linked list; internally: TAILQ) */
	{ NULL }
};

PRIVATE WUNUSED_T NONNULL_T((1)) int DCALL
libcollections_init(DeeDexObject *__restrict self) {
	(void)self;
	RBTree_Type.tp_cmp = DeeSeq_Type.tp_cmp;
	return 0;
}

PUBLIC struct dex DEX = {
	/* .d_symbols = */ symbols,
	/* .d_init    = */ &libcollections_init,
};

DECL_END


#endif /* !GUARD_DEX_COLLECTIONS_LIBCOLLECTIONS_C */
