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
#ifndef GUARD_DEEMON_RUNTIME_METHOD_HINT_SUPER_C
#define GUARD_DEEMON_RUNTIME_METHOD_HINT_SUPER_C 1

#include <deemon/api.h>

#include <deemon/method-hints.h>
#include <deemon/object.h>
#include <deemon/super.h>

#include "method-hint-super.h"
#include "method-hints.h"

DECL_BEGIN

PRIVATE WUNUSED NONNULL((2, 3)) int DCALL
repack_super_after_inplace_int(int res,
                               /*inherit(always)*/ DREF DeeObject *self,
                               DREF DeeSuperObject **__restrict p_super_self) {
	if likely(self == (*p_super_self)->s_self) {
		Dee_DecrefNokill(self);
	} else if unlikely(res != 0) {
		Dee_Decref(self);
	} else if (!DeeObject_IsShared(*p_super_self)) {
		DREF DeeObject *old_self;
		old_self = (*p_super_self)->s_self; /* Inherit reference */
		(*p_super_self)->s_self = self;     /* Inherit reference */
		Dee_Decref(old_self);
	} else {
		DREF DeeSuperObject *new_super;
		new_super = (DREF DeeSuperObject *)DeeSuper_New((*p_super_self)->s_type, self);
		Dee_Decref_unlikely(self);
		if unlikely(!new_super)
			return -1;
		Dee_Decref_unlikely(*p_super_self);
		*p_super_self = new_super;
	}
	return res;
}


/* clang-format off */
/*[[[deemon (printSuperMethodHintWrappers from "..method-hints.method-hints")(impl: true);]]]*/
INTERN WUNUSED NONNULL((1)) int DCALL
super_mh__seq_operator_bool(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_operator_bool, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_bool_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_bool_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__seq_operator_sizeob(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_operator_sizeob, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_sizeob_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_sizeob_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
super_mh__seq_operator_size(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_operator_size, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_size_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_size_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(size_t (DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__seq_operator_iter(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_operator_iter, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_iter_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_iter_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
super_mh__seq_operator_foreach(DeeSuperObject *__restrict self, Dee_foreach_t cb, void *arg) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_operator_foreach, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_foreach_t)specs.smh_cb)(self->s_self, cb, arg);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_foreach_t)specs.smh_cb)(Dee_AsObject(self), cb, arg);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(Dee_ssize_t (DCALL *)(DeeTypeObject *, DeeObject *, Dee_foreach_t, void *))specs.smh_cb)(self->s_type, self->s_self, cb, arg);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
super_mh__seq_operator_foreach_pair(DeeSuperObject *__restrict self, Dee_foreach_pair_t cb, void *arg) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_operator_foreach_pair, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_foreach_pair_t)specs.smh_cb)(self->s_self, cb, arg);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_foreach_pair_t)specs.smh_cb)(Dee_AsObject(self), cb, arg);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(Dee_ssize_t (DCALL *)(DeeTypeObject *, DeeObject *, Dee_foreach_pair_t, void *))specs.smh_cb)(self->s_type, self->s_self, cb, arg);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__seq_operator_getitem(DeeSuperObject *self, DeeObject *index) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_operator_getitem, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_getitem_t)specs.smh_cb)(self->s_self, index);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_getitem_t)specs.smh_cb)(Dee_AsObject(self), index);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, index);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__seq_operator_getitem_index(DeeSuperObject *__restrict self, size_t index) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_operator_getitem_index, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_getitem_index_t)specs.smh_cb)(self->s_self, index);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_getitem_index_t)specs.smh_cb)(Dee_AsObject(self), index);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, size_t))specs.smh_cb)(self->s_type, self->s_self, index);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__seq_operator_trygetitem(DeeSuperObject *self, DeeObject *index) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_operator_trygetitem, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_trygetitem_t)specs.smh_cb)(self->s_self, index);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_trygetitem_t)specs.smh_cb)(Dee_AsObject(self), index);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, index);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__seq_operator_trygetitem_index(DeeSuperObject *__restrict self, size_t index) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_operator_trygetitem_index, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_trygetitem_index_t)specs.smh_cb)(self->s_self, index);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_trygetitem_index_t)specs.smh_cb)(Dee_AsObject(self), index);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, size_t))specs.smh_cb)(self->s_type, self->s_self, index);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__seq_operator_hasitem(DeeSuperObject *self, DeeObject *index) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_operator_hasitem, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_hasitem_t)specs.smh_cb)(self->s_self, index);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_hasitem_t)specs.smh_cb)(Dee_AsObject(self), index);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, index);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) int DCALL
super_mh__seq_operator_hasitem_index(DeeSuperObject *__restrict self, size_t index) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_operator_hasitem_index, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_hasitem_index_t)specs.smh_cb)(self->s_self, index);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_hasitem_index_t)specs.smh_cb)(Dee_AsObject(self), index);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, size_t))specs.smh_cb)(self->s_type, self->s_self, index);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__seq_operator_bounditem(DeeSuperObject *self, DeeObject *index) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_operator_bounditem, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_bounditem_t)specs.smh_cb)(self->s_self, index);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_bounditem_t)specs.smh_cb)(Dee_AsObject(self), index);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, index);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) int DCALL
super_mh__seq_operator_bounditem_index(DeeSuperObject *__restrict self, size_t index) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_operator_bounditem_index, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_bounditem_index_t)specs.smh_cb)(self->s_self, index);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_bounditem_index_t)specs.smh_cb)(Dee_AsObject(self), index);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, size_t))specs.smh_cb)(self->s_type, self->s_self, index);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__seq_operator_delitem(DeeSuperObject *self, DeeObject *index) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_operator_delitem, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_delitem_t)specs.smh_cb)(self->s_self, index);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_delitem_t)specs.smh_cb)(Dee_AsObject(self), index);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, index);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) int DCALL
super_mh__seq_operator_delitem_index(DeeSuperObject *__restrict self, size_t index) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_operator_delitem_index, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_delitem_index_t)specs.smh_cb)(self->s_self, index);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_delitem_index_t)specs.smh_cb)(Dee_AsObject(self), index);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, size_t))specs.smh_cb)(self->s_type, self->s_self, index);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
super_mh__seq_operator_setitem(DeeSuperObject *self, DeeObject *index, DeeObject *value) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_operator_setitem, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_setitem_t)specs.smh_cb)(self->s_self, index, value);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_setitem_t)specs.smh_cb)(Dee_AsObject(self), index, value);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, index, value);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
super_mh__seq_operator_setitem_index(DeeSuperObject *self, size_t index, DeeObject *value) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_operator_setitem_index, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_setitem_index_t)specs.smh_cb)(self->s_self, index, value);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_setitem_index_t)specs.smh_cb)(Dee_AsObject(self), index, value);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, size_t, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, index, value);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
super_mh__seq_operator_getrange(DeeSuperObject *self, DeeObject *start, DeeObject *end) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_operator_getrange, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_getrange_t)specs.smh_cb)(self->s_self, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_getrange_t)specs.smh_cb)(Dee_AsObject(self), start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, start, end);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__seq_operator_getrange_index(DeeSuperObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_operator_getrange_index, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_getrange_index_t)specs.smh_cb)(self->s_self, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_getrange_index_t)specs.smh_cb)(Dee_AsObject(self), start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, Dee_ssize_t, Dee_ssize_t))specs.smh_cb)(self->s_type, self->s_self, start, end);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__seq_operator_getrange_index_n(DeeSuperObject *self, Dee_ssize_t start) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_operator_getrange_index_n, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_getrange_index_n_t)specs.smh_cb)(self->s_self, start);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_getrange_index_n_t)specs.smh_cb)(Dee_AsObject(self), start);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, Dee_ssize_t))specs.smh_cb)(self->s_type, self->s_self, start);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
super_mh__seq_operator_delrange(DeeSuperObject *self, DeeObject *start, DeeObject *end) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_operator_delrange, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_delrange_t)specs.smh_cb)(self->s_self, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_delrange_t)specs.smh_cb)(Dee_AsObject(self), start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, start, end);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) int DCALL
super_mh__seq_operator_delrange_index(DeeSuperObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_operator_delrange_index, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_delrange_index_t)specs.smh_cb)(self->s_self, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_delrange_index_t)specs.smh_cb)(Dee_AsObject(self), start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, Dee_ssize_t, Dee_ssize_t))specs.smh_cb)(self->s_type, self->s_self, start, end);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) int DCALL
super_mh__seq_operator_delrange_index_n(DeeSuperObject *self, Dee_ssize_t start) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_operator_delrange_index_n, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_delrange_index_n_t)specs.smh_cb)(self->s_self, start);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_delrange_index_n_t)specs.smh_cb)(Dee_AsObject(self), start);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, Dee_ssize_t))specs.smh_cb)(self->s_type, self->s_self, start);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
super_mh__seq_operator_setrange(DeeSuperObject *self, DeeObject *start, DeeObject *end, DeeObject *items) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_operator_setrange, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_setrange_t)specs.smh_cb)(self->s_self, start, end, items);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_setrange_t)specs.smh_cb)(Dee_AsObject(self), start, end, items);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, start, end, items);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
super_mh__seq_operator_setrange_index(DeeSuperObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *items) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_operator_setrange_index, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_setrange_index_t)specs.smh_cb)(self->s_self, start, end, items);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_setrange_index_t)specs.smh_cb)(Dee_AsObject(self), start, end, items);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, start, end, items);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
super_mh__seq_operator_setrange_index_n(DeeSuperObject *self, Dee_ssize_t start, DeeObject *items) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_operator_setrange_index_n, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_setrange_index_n_t)specs.smh_cb)(self->s_self, start, items);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_setrange_index_n_t)specs.smh_cb)(Dee_AsObject(self), start, items);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, Dee_ssize_t, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, start, items);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__seq_operator_assign(DeeSuperObject *self, DeeObject *items) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_operator_assign, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_assign_t)specs.smh_cb)(self->s_self, items);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_assign_t)specs.smh_cb)(Dee_AsObject(self), items);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, items);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
super_mh__seq_operator_hash(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_operator_hash, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_hash_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_hash_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(Dee_hash_t (DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__seq_operator_compare(DeeSuperObject *lhs, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(lhs, Dee_TMH_seq_operator_compare, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_compare_t)specs.smh_cb)(lhs->s_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_compare_t)specs.smh_cb)(Dee_AsObject(lhs), rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(lhs->s_type, lhs->s_self, rhs);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__seq_operator_compare_eq(DeeSuperObject *lhs, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(lhs, Dee_TMH_seq_operator_compare_eq, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_compare_eq_t)specs.smh_cb)(lhs->s_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_compare_eq_t)specs.smh_cb)(Dee_AsObject(lhs), rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(lhs->s_type, lhs->s_self, rhs);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__seq_operator_trycompare_eq(DeeSuperObject *lhs, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(lhs, Dee_TMH_seq_operator_trycompare_eq, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_trycompare_eq_t)specs.smh_cb)(lhs->s_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_trycompare_eq_t)specs.smh_cb)(Dee_AsObject(lhs), rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(lhs->s_type, lhs->s_self, rhs);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__seq_operator_eq(DeeSuperObject *lhs, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(lhs, Dee_TMH_seq_operator_eq, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_eq_t)specs.smh_cb)(lhs->s_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_eq_t)specs.smh_cb)(Dee_AsObject(lhs), rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(lhs->s_type, lhs->s_self, rhs);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__seq_operator_ne(DeeSuperObject *lhs, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(lhs, Dee_TMH_seq_operator_ne, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_ne_t)specs.smh_cb)(lhs->s_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_ne_t)specs.smh_cb)(Dee_AsObject(lhs), rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(lhs->s_type, lhs->s_self, rhs);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__seq_operator_lo(DeeSuperObject *lhs, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(lhs, Dee_TMH_seq_operator_lo, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_lo_t)specs.smh_cb)(lhs->s_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_lo_t)specs.smh_cb)(Dee_AsObject(lhs), rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(lhs->s_type, lhs->s_self, rhs);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__seq_operator_le(DeeSuperObject *lhs, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(lhs, Dee_TMH_seq_operator_le, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_le_t)specs.smh_cb)(lhs->s_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_le_t)specs.smh_cb)(Dee_AsObject(lhs), rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(lhs->s_type, lhs->s_self, rhs);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__seq_operator_gr(DeeSuperObject *lhs, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(lhs, Dee_TMH_seq_operator_gr, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_gr_t)specs.smh_cb)(lhs->s_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_gr_t)specs.smh_cb)(Dee_AsObject(lhs), rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(lhs->s_type, lhs->s_self, rhs);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__seq_operator_ge(DeeSuperObject *lhs, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(lhs, Dee_TMH_seq_operator_ge, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_ge_t)specs.smh_cb)(lhs->s_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_ge_t)specs.smh_cb)(Dee_AsObject(lhs), rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(lhs->s_type, lhs->s_self, rhs);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__seq_operator_add(DeeSuperObject *lhs, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(lhs, Dee_TMH_seq_operator_add, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_add_t)specs.smh_cb)(lhs->s_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_add_t)specs.smh_cb)(Dee_AsObject(lhs), rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(lhs->s_type, lhs->s_self, rhs);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__seq_operator_mul(DeeSuperObject *self, DeeObject *repeat) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_operator_mul, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_mul_t)specs.smh_cb)(self->s_self, repeat);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_mul_t)specs.smh_cb)(Dee_AsObject(self), repeat);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, repeat);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__seq_operator_inplace_add(DREF DeeSuperObject **__restrict p_lhs, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper((*p_lhs), Dee_TMH_seq_operator_inplace_add, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF: {
		int _res;
		DREF DeeObject *_self = (*p_lhs)->s_self;
		Dee_Incref(_self);
		_res = (*(DeeMH_seq_operator_inplace_add_t)specs.smh_cb)(&_self, rhs);
		return repack_super_after_inplace_int(_res, _self, p_lhs);
	}	break;
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_inplace_add_t)specs.smh_cb)((DREF DeeObject **)p_lhs, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE: {
		int _res;
		DREF DeeObject *_self = (*p_lhs)->s_self;
		Dee_Incref(_self);
		_res = (*(int (DCALL *)(DeeTypeObject *, DREF DeeObject **, DeeObject *))specs.smh_cb)((*p_lhs)->s_type, &_self, rhs);
		return repack_super_after_inplace_int(_res, _self, p_lhs);
	}	break;
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__seq_operator_inplace_mul(DREF DeeSuperObject **__restrict p_lhs, DeeObject *repeat) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper((*p_lhs), Dee_TMH_seq_operator_inplace_mul, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF: {
		int _res;
		DREF DeeObject *_self = (*p_lhs)->s_self;
		Dee_Incref(_self);
		_res = (*(DeeMH_seq_operator_inplace_mul_t)specs.smh_cb)(&_self, repeat);
		return repack_super_after_inplace_int(_res, _self, p_lhs);
	}	break;
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_inplace_mul_t)specs.smh_cb)((DREF DeeObject **)p_lhs, repeat);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE: {
		int _res;
		DREF DeeObject *_self = (*p_lhs)->s_self;
		Dee_Incref(_self);
		_res = (*(int (DCALL *)(DeeTypeObject *, DREF DeeObject **, DeeObject *))specs.smh_cb)((*p_lhs)->s_type, &_self, repeat);
		return repack_super_after_inplace_int(_res, _self, p_lhs);
	}	break;
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
super_mh__seq_enumerate(DeeSuperObject *__restrict self, Dee_seq_enumerate_t cb, void *arg) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_enumerate, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_enumerate_t)specs.smh_cb)(self->s_self, cb, arg);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_enumerate_t)specs.smh_cb)(Dee_AsObject(self), cb, arg);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(Dee_ssize_t (DCALL *)(DeeTypeObject *, DeeObject *, Dee_seq_enumerate_t, void *))specs.smh_cb)(self->s_type, self->s_self, cb, arg);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
super_mh__seq_enumerate_index(DeeSuperObject *__restrict self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_enumerate_index, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_enumerate_index_t)specs.smh_cb)(self->s_self, cb, arg, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_enumerate_index_t)specs.smh_cb)(Dee_AsObject(self), cb, arg, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(Dee_ssize_t (DCALL *)(DeeTypeObject *, DeeObject *, Dee_seq_enumerate_index_t, void *, size_t, size_t))specs.smh_cb)(self->s_type, self->s_self, cb, arg, start, end);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__seq_makeenumeration(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_makeenumeration, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_makeenumeration_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_makeenumeration_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
super_mh__seq_makeenumeration_with_range(DeeSuperObject *self, DeeObject *start, DeeObject *end) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_makeenumeration_with_range, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_makeenumeration_with_range_t)specs.smh_cb)(self->s_self, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_makeenumeration_with_range_t)specs.smh_cb)(Dee_AsObject(self), start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, start, end);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__seq_makeenumeration_with_intrange(DeeSuperObject *__restrict self, size_t start, size_t end) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_makeenumeration_with_intrange, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_makeenumeration_with_intrange_t)specs.smh_cb)(self->s_self, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_makeenumeration_with_intrange_t)specs.smh_cb)(Dee_AsObject(self), start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, size_t, size_t))specs.smh_cb)(self->s_type, self->s_self, start, end);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
super_mh__seq_unpack(DeeSuperObject *__restrict self, size_t count, DREF DeeObject *result[]) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_unpack, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_unpack_t)specs.smh_cb)(self->s_self, count, result);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_unpack_t)specs.smh_cb)(Dee_AsObject(self), count, result);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, size_t, DREF DeeObject **))specs.smh_cb)(self->s_type, self->s_self, count, result);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 4)) size_t DCALL
super_mh__seq_unpack_ex(DeeSuperObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_unpack_ex, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_unpack_ex_t)specs.smh_cb)(self->s_self, min_count, max_count, result);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_unpack_ex_t)specs.smh_cb)(Dee_AsObject(self), min_count, max_count, result);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(size_t (DCALL *)(DeeTypeObject *, DeeObject *, size_t, size_t, DREF DeeObject **))specs.smh_cb)(self->s_type, self->s_self, min_count, max_count, result);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 4)) size_t DCALL
super_mh__seq_unpack_ub(DeeSuperObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_unpack_ub, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_unpack_ub_t)specs.smh_cb)(self->s_self, min_count, max_count, result);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_unpack_ub_t)specs.smh_cb)(Dee_AsObject(self), min_count, max_count, result);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(size_t (DCALL *)(DeeTypeObject *, DeeObject *, size_t, size_t, DREF DeeObject **))specs.smh_cb)(self->s_type, self->s_self, min_count, max_count, result);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__seq_trygetfirst(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_trygetfirst, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_trygetfirst_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_trygetfirst_t)specs.smh_cb)(Dee_AsObject(self));
/*	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE: // Unused */
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__seq_getfirst(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_getfirst, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_getfirst_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_getfirst_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) int DCALL
super_mh__seq_boundfirst(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_boundfirst, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_boundfirst_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_boundfirst_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) int DCALL
super_mh__seq_delfirst(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_delfirst, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_delfirst_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_delfirst_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__seq_setfirst(DeeSuperObject *self, DeeObject *value) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_setfirst, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_setfirst_t)specs.smh_cb)(self->s_self, value);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_setfirst_t)specs.smh_cb)(Dee_AsObject(self), value);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, value);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__seq_trygetlast(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_trygetlast, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_trygetlast_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_trygetlast_t)specs.smh_cb)(Dee_AsObject(self));
/*	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE: // Unused */
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__seq_getlast(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_getlast, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_getlast_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_getlast_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) int DCALL
super_mh__seq_boundlast(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_boundlast, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_boundlast_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_boundlast_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) int DCALL
super_mh__seq_dellast(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_dellast, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_dellast_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_dellast_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__seq_setlast(DeeSuperObject *self, DeeObject *value) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_setlast, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_setlast_t)specs.smh_cb)(self->s_self, value);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_setlast_t)specs.smh_cb)(Dee_AsObject(self), value);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, value);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__seq_cached(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_cached, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_cached_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_cached_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__seq_frozen(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_frozen, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_frozen_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_frozen_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) int DCALL
super_mh__seq_any(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_any, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_any_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_any_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__seq_any_with_key(DeeSuperObject *self, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_any_with_key, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_any_with_key_t)specs.smh_cb)(self->s_self, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_any_with_key_t)specs.smh_cb)(Dee_AsObject(self), key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) int DCALL
super_mh__seq_any_with_range(DeeSuperObject *__restrict self, size_t start, size_t end) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_any_with_range, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_any_with_range_t)specs.smh_cb)(self->s_self, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_any_with_range_t)specs.smh_cb)(Dee_AsObject(self), start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, size_t, size_t))specs.smh_cb)(self->s_type, self->s_self, start, end);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
super_mh__seq_any_with_range_and_key(DeeSuperObject *self, size_t start, size_t end, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_any_with_range_and_key, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_any_with_range_and_key_t)specs.smh_cb)(self->s_self, start, end, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_any_with_range_and_key_t)specs.smh_cb)(Dee_AsObject(self), start, end, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, size_t, size_t, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, start, end, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) int DCALL
super_mh__seq_all(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_all, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_all_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_all_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__seq_all_with_key(DeeSuperObject *self, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_all_with_key, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_all_with_key_t)specs.smh_cb)(self->s_self, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_all_with_key_t)specs.smh_cb)(Dee_AsObject(self), key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) int DCALL
super_mh__seq_all_with_range(DeeSuperObject *__restrict self, size_t start, size_t end) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_all_with_range, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_all_with_range_t)specs.smh_cb)(self->s_self, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_all_with_range_t)specs.smh_cb)(Dee_AsObject(self), start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, size_t, size_t))specs.smh_cb)(self->s_type, self->s_self, start, end);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
super_mh__seq_all_with_range_and_key(DeeSuperObject *self, size_t start, size_t end, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_all_with_range_and_key, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_all_with_range_and_key_t)specs.smh_cb)(self->s_self, start, end, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_all_with_range_and_key_t)specs.smh_cb)(Dee_AsObject(self), start, end, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, size_t, size_t, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, start, end, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) int DCALL
super_mh__seq_parity(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_parity, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_parity_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_parity_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__seq_parity_with_key(DeeSuperObject *self, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_parity_with_key, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_parity_with_key_t)specs.smh_cb)(self->s_self, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_parity_with_key_t)specs.smh_cb)(Dee_AsObject(self), key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) int DCALL
super_mh__seq_parity_with_range(DeeSuperObject *__restrict self, size_t start, size_t end) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_parity_with_range, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_parity_with_range_t)specs.smh_cb)(self->s_self, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_parity_with_range_t)specs.smh_cb)(Dee_AsObject(self), start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, size_t, size_t))specs.smh_cb)(self->s_type, self->s_self, start, end);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
super_mh__seq_parity_with_range_and_key(DeeSuperObject *self, size_t start, size_t end, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_parity_with_range_and_key, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_parity_with_range_and_key_t)specs.smh_cb)(self->s_self, start, end, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_parity_with_range_and_key_t)specs.smh_cb)(Dee_AsObject(self), start, end, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, size_t, size_t, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, start, end, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__seq_reduce(DeeSuperObject *self, DeeObject *combine) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_reduce, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_reduce_t)specs.smh_cb)(self->s_self, combine);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_reduce_t)specs.smh_cb)(Dee_AsObject(self), combine);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, combine);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
super_mh__seq_reduce_with_init(DeeSuperObject *self, DeeObject *combine, DeeObject *init) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_reduce_with_init, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_reduce_with_init_t)specs.smh_cb)(self->s_self, combine, init);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_reduce_with_init_t)specs.smh_cb)(Dee_AsObject(self), combine, init);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, combine, init);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__seq_reduce_with_range(DeeSuperObject *self, DeeObject *combine, size_t start, size_t end) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_reduce_with_range, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_reduce_with_range_t)specs.smh_cb)(self->s_self, combine, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_reduce_with_range_t)specs.smh_cb)(Dee_AsObject(self), combine, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, size_t, size_t))specs.smh_cb)(self->s_type, self->s_self, combine, start, end);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
super_mh__seq_reduce_with_range_and_init(DeeSuperObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_reduce_with_range_and_init, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_reduce_with_range_and_init_t)specs.smh_cb)(self->s_self, combine, start, end, init);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_reduce_with_range_and_init_t)specs.smh_cb)(Dee_AsObject(self), combine, start, end, init);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, size_t, size_t, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, combine, start, end, init);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__seq_min(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_min, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_min_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_min_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__seq_min_with_key(DeeSuperObject *self, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_min_with_key, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_min_with_key_t)specs.smh_cb)(self->s_self, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_min_with_key_t)specs.smh_cb)(Dee_AsObject(self), key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__seq_min_with_range(DeeSuperObject *__restrict self, size_t start, size_t end) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_min_with_range, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_min_with_range_t)specs.smh_cb)(self->s_self, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_min_with_range_t)specs.smh_cb)(Dee_AsObject(self), start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, size_t, size_t))specs.smh_cb)(self->s_type, self->s_self, start, end);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
super_mh__seq_min_with_range_and_key(DeeSuperObject *self, size_t start, size_t end, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_min_with_range_and_key, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_min_with_range_and_key_t)specs.smh_cb)(self->s_self, start, end, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_min_with_range_and_key_t)specs.smh_cb)(Dee_AsObject(self), start, end, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, size_t, size_t, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, start, end, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__seq_max(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_max, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_max_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_max_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__seq_max_with_key(DeeSuperObject *self, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_max_with_key, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_max_with_key_t)specs.smh_cb)(self->s_self, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_max_with_key_t)specs.smh_cb)(Dee_AsObject(self), key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__seq_max_with_range(DeeSuperObject *__restrict self, size_t start, size_t end) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_max_with_range, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_max_with_range_t)specs.smh_cb)(self->s_self, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_max_with_range_t)specs.smh_cb)(Dee_AsObject(self), start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, size_t, size_t))specs.smh_cb)(self->s_type, self->s_self, start, end);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
super_mh__seq_max_with_range_and_key(DeeSuperObject *self, size_t start, size_t end, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_max_with_range_and_key, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_max_with_range_and_key_t)specs.smh_cb)(self->s_self, start, end, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_max_with_range_and_key_t)specs.smh_cb)(Dee_AsObject(self), start, end, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, size_t, size_t, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, start, end, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__seq_sum(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_sum, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_sum_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_sum_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__seq_sum_with_range(DeeSuperObject *__restrict self, size_t start, size_t end) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_sum_with_range, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_sum_with_range_t)specs.smh_cb)(self->s_self, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_sum_with_range_t)specs.smh_cb)(Dee_AsObject(self), start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, size_t, size_t))specs.smh_cb)(self->s_type, self->s_self, start, end);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
super_mh__seq_count(DeeSuperObject *self, DeeObject *item) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_count, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_count_t)specs.smh_cb)(self->s_self, item);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_count_t)specs.smh_cb)(Dee_AsObject(self), item);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(size_t (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, item);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 3)) size_t DCALL
super_mh__seq_count_with_key(DeeSuperObject *self, DeeObject *item, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_count_with_key, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_count_with_key_t)specs.smh_cb)(self->s_self, item, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_count_with_key_t)specs.smh_cb)(Dee_AsObject(self), item, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(size_t (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, item, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
super_mh__seq_count_with_range(DeeSuperObject *self, DeeObject *item, size_t start, size_t end) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_count_with_range, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_count_with_range_t)specs.smh_cb)(self->s_self, item, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_count_with_range_t)specs.smh_cb)(Dee_AsObject(self), item, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(size_t (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, size_t, size_t))specs.smh_cb)(self->s_type, self->s_self, item, start, end);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
super_mh__seq_count_with_range_and_key(DeeSuperObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_count_with_range_and_key, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_count_with_range_and_key_t)specs.smh_cb)(self->s_self, item, start, end, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_count_with_range_and_key_t)specs.smh_cb)(Dee_AsObject(self), item, start, end, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(size_t (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, size_t, size_t, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, item, start, end, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__seq_contains(DeeSuperObject *self, DeeObject *item) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_contains, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_contains_t)specs.smh_cb)(self->s_self, item);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_contains_t)specs.smh_cb)(Dee_AsObject(self), item);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, item);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
super_mh__seq_contains_with_key(DeeSuperObject *self, DeeObject *item, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_contains_with_key, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_contains_with_key_t)specs.smh_cb)(self->s_self, item, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_contains_with_key_t)specs.smh_cb)(Dee_AsObject(self), item, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, item, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__seq_contains_with_range(DeeSuperObject *self, DeeObject *item, size_t start, size_t end) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_contains_with_range, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_contains_with_range_t)specs.smh_cb)(self->s_self, item, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_contains_with_range_t)specs.smh_cb)(Dee_AsObject(self), item, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, size_t, size_t))specs.smh_cb)(self->s_type, self->s_self, item, start, end);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
super_mh__seq_contains_with_range_and_key(DeeSuperObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_contains_with_range_and_key, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_contains_with_range_and_key_t)specs.smh_cb)(self->s_self, item, start, end, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_contains_with_range_and_key_t)specs.smh_cb)(Dee_AsObject(self), item, start, end, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, size_t, size_t, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, item, start, end, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__seq_operator_contains(DeeSuperObject *self, DeeObject *item) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_operator_contains, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_operator_contains_t)specs.smh_cb)(self->s_self, item);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_operator_contains_t)specs.smh_cb)(Dee_AsObject(self), item);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, item);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
super_mh__seq_locate(DeeSuperObject *self, DeeObject *match, DeeObject *def) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_locate, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_locate_t)specs.smh_cb)(self->s_self, match, def);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_locate_t)specs.smh_cb)(Dee_AsObject(self), match, def);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, match, def);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
super_mh__seq_locate_with_range(DeeSuperObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_locate_with_range, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_locate_with_range_t)specs.smh_cb)(self->s_self, match, start, end, def);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_locate_with_range_t)specs.smh_cb)(Dee_AsObject(self), match, start, end, def);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, size_t, size_t, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, match, start, end, def);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
super_mh__seq_rlocate(DeeSuperObject *self, DeeObject *match, DeeObject *def) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_rlocate, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_rlocate_t)specs.smh_cb)(self->s_self, match, def);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_rlocate_t)specs.smh_cb)(Dee_AsObject(self), match, def);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, match, def);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
super_mh__seq_rlocate_with_range(DeeSuperObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_rlocate_with_range, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_rlocate_with_range_t)specs.smh_cb)(self->s_self, match, start, end, def);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_rlocate_with_range_t)specs.smh_cb)(Dee_AsObject(self), match, start, end, def);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, size_t, size_t, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, match, start, end, def);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__seq_startswith(DeeSuperObject *self, DeeObject *item) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_startswith, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_startswith_t)specs.smh_cb)(self->s_self, item);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_startswith_t)specs.smh_cb)(Dee_AsObject(self), item);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, item);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
super_mh__seq_startswith_with_key(DeeSuperObject *self, DeeObject *item, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_startswith_with_key, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_startswith_with_key_t)specs.smh_cb)(self->s_self, item, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_startswith_with_key_t)specs.smh_cb)(Dee_AsObject(self), item, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, item, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__seq_startswith_with_range(DeeSuperObject *self, DeeObject *item, size_t start, size_t end) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_startswith_with_range, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_startswith_with_range_t)specs.smh_cb)(self->s_self, item, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_startswith_with_range_t)specs.smh_cb)(Dee_AsObject(self), item, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, size_t, size_t))specs.smh_cb)(self->s_type, self->s_self, item, start, end);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
super_mh__seq_startswith_with_range_and_key(DeeSuperObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_startswith_with_range_and_key, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_startswith_with_range_and_key_t)specs.smh_cb)(self->s_self, item, start, end, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_startswith_with_range_and_key_t)specs.smh_cb)(Dee_AsObject(self), item, start, end, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, size_t, size_t, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, item, start, end, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__seq_endswith(DeeSuperObject *self, DeeObject *item) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_endswith, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_endswith_t)specs.smh_cb)(self->s_self, item);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_endswith_t)specs.smh_cb)(Dee_AsObject(self), item);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, item);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
super_mh__seq_endswith_with_key(DeeSuperObject *self, DeeObject *item, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_endswith_with_key, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_endswith_with_key_t)specs.smh_cb)(self->s_self, item, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_endswith_with_key_t)specs.smh_cb)(Dee_AsObject(self), item, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, item, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__seq_endswith_with_range(DeeSuperObject *self, DeeObject *item, size_t start, size_t end) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_endswith_with_range, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_endswith_with_range_t)specs.smh_cb)(self->s_self, item, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_endswith_with_range_t)specs.smh_cb)(Dee_AsObject(self), item, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, size_t, size_t))specs.smh_cb)(self->s_type, self->s_self, item, start, end);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
super_mh__seq_endswith_with_range_and_key(DeeSuperObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_endswith_with_range_and_key, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_endswith_with_range_and_key_t)specs.smh_cb)(self->s_self, item, start, end, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_endswith_with_range_and_key_t)specs.smh_cb)(Dee_AsObject(self), item, start, end, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, size_t, size_t, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, item, start, end, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
super_mh__seq_find(DeeSuperObject *self, DeeObject *item, size_t start, size_t end) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_find, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_find_t)specs.smh_cb)(self->s_self, item, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_find_t)specs.smh_cb)(Dee_AsObject(self), item, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(size_t (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, size_t, size_t))specs.smh_cb)(self->s_type, self->s_self, item, start, end);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
super_mh__seq_find_with_key(DeeSuperObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_find_with_key, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_find_with_key_t)specs.smh_cb)(self->s_self, item, start, end, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_find_with_key_t)specs.smh_cb)(Dee_AsObject(self), item, start, end, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(size_t (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, size_t, size_t, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, item, start, end, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
super_mh__seq_rfind(DeeSuperObject *self, DeeObject *item, size_t start, size_t end) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_rfind, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_rfind_t)specs.smh_cb)(self->s_self, item, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_rfind_t)specs.smh_cb)(Dee_AsObject(self), item, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(size_t (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, size_t, size_t))specs.smh_cb)(self->s_type, self->s_self, item, start, end);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
super_mh__seq_rfind_with_key(DeeSuperObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_rfind_with_key, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_rfind_with_key_t)specs.smh_cb)(self->s_self, item, start, end, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_rfind_with_key_t)specs.smh_cb)(Dee_AsObject(self), item, start, end, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(size_t (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, size_t, size_t, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, item, start, end, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) int DCALL
super_mh__seq_erase(DeeSuperObject *__restrict self, size_t index, size_t count) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_erase, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_erase_t)specs.smh_cb)(self->s_self, index, count);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_erase_t)specs.smh_cb)(Dee_AsObject(self), index, count);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, size_t, size_t))specs.smh_cb)(self->s_type, self->s_self, index, count);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
super_mh__seq_insert(DeeSuperObject *self, size_t index, DeeObject *item) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_insert, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_insert_t)specs.smh_cb)(self->s_self, index, item);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_insert_t)specs.smh_cb)(Dee_AsObject(self), index, item);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, size_t, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, index, item);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
super_mh__seq_insertall(DeeSuperObject *self, size_t index, DeeObject *items) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_insertall, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_insertall_t)specs.smh_cb)(self->s_self, index, items);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_insertall_t)specs.smh_cb)(Dee_AsObject(self), index, items);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, size_t, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, index, items);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__seq_pushfront(DeeSuperObject *self, DeeObject *item) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_pushfront, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_pushfront_t)specs.smh_cb)(self->s_self, item);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_pushfront_t)specs.smh_cb)(Dee_AsObject(self), item);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, item);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__seq_append(DeeSuperObject *self, DeeObject *item) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_append, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_append_t)specs.smh_cb)(self->s_self, item);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_append_t)specs.smh_cb)(Dee_AsObject(self), item);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, item);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__seq_extend(DeeSuperObject *self, DeeObject *items) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_extend, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_extend_t)specs.smh_cb)(self->s_self, items);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_extend_t)specs.smh_cb)(Dee_AsObject(self), items);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, items);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
super_mh__seq_xchitem_index(DeeSuperObject *self, size_t index, DeeObject *item) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_xchitem_index, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_xchitem_index_t)specs.smh_cb)(self->s_self, index, item);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_xchitem_index_t)specs.smh_cb)(Dee_AsObject(self), index, item);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, size_t, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, index, item);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) int DCALL
super_mh__seq_clear(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_clear, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_clear_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_clear_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__seq_pop(DeeSuperObject *self, Dee_ssize_t index) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_pop, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_pop_t)specs.smh_cb)(self->s_self, index);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_pop_t)specs.smh_cb)(Dee_AsObject(self), index);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, Dee_ssize_t))specs.smh_cb)(self->s_type, self->s_self, index);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__seq_remove(DeeSuperObject *self, DeeObject *item, size_t start, size_t end) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_remove, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_remove_t)specs.smh_cb)(self->s_self, item, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_remove_t)specs.smh_cb)(Dee_AsObject(self), item, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, size_t, size_t))specs.smh_cb)(self->s_type, self->s_self, item, start, end);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
super_mh__seq_remove_with_key(DeeSuperObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_remove_with_key, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_remove_with_key_t)specs.smh_cb)(self->s_self, item, start, end, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_remove_with_key_t)specs.smh_cb)(Dee_AsObject(self), item, start, end, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, size_t, size_t, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, item, start, end, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__seq_rremove(DeeSuperObject *self, DeeObject *item, size_t start, size_t end) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_rremove, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_rremove_t)specs.smh_cb)(self->s_self, item, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_rremove_t)specs.smh_cb)(Dee_AsObject(self), item, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, size_t, size_t))specs.smh_cb)(self->s_type, self->s_self, item, start, end);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
super_mh__seq_rremove_with_key(DeeSuperObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_rremove_with_key, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_rremove_with_key_t)specs.smh_cb)(self->s_self, item, start, end, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_rremove_with_key_t)specs.smh_cb)(Dee_AsObject(self), item, start, end, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, size_t, size_t, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, item, start, end, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
super_mh__seq_removeall(DeeSuperObject *self, DeeObject *item, size_t start, size_t end, size_t max) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_removeall, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_removeall_t)specs.smh_cb)(self->s_self, item, start, end, max);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_removeall_t)specs.smh_cb)(Dee_AsObject(self), item, start, end, max);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(size_t (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, size_t, size_t, size_t))specs.smh_cb)(self->s_type, self->s_self, item, start, end, max);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 6)) size_t DCALL
super_mh__seq_removeall_with_key(DeeSuperObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_removeall_with_key, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_removeall_with_key_t)specs.smh_cb)(self->s_self, item, start, end, max, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_removeall_with_key_t)specs.smh_cb)(Dee_AsObject(self), item, start, end, max, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(size_t (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, size_t, size_t, size_t, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, item, start, end, max, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
super_mh__seq_removeif(DeeSuperObject *self, DeeObject *should, size_t start, size_t end, size_t max) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_removeif, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_removeif_t)specs.smh_cb)(self->s_self, should, start, end, max);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_removeif_t)specs.smh_cb)(Dee_AsObject(self), should, start, end, max);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(size_t (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, size_t, size_t, size_t))specs.smh_cb)(self->s_type, self->s_self, should, start, end, max);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
super_mh__seq_resize(DeeSuperObject *self, size_t newsize, DeeObject *filler) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_resize, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_resize_t)specs.smh_cb)(self->s_self, newsize, filler);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_resize_t)specs.smh_cb)(Dee_AsObject(self), newsize, filler);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, size_t, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, newsize, filler);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
super_mh__seq_fill(DeeSuperObject *self, size_t start, size_t end, DeeObject *filler) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_fill, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_fill_t)specs.smh_cb)(self->s_self, start, end, filler);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_fill_t)specs.smh_cb)(Dee_AsObject(self), start, end, filler);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, size_t, size_t, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, start, end, filler);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) int DCALL
super_mh__seq_reverse(DeeSuperObject *self, size_t start, size_t end) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_reverse, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_reverse_t)specs.smh_cb)(self->s_self, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_reverse_t)specs.smh_cb)(Dee_AsObject(self), start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, size_t, size_t))specs.smh_cb)(self->s_type, self->s_self, start, end);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__seq_reversed(DeeSuperObject *self, size_t start, size_t end) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_reversed, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_reversed_t)specs.smh_cb)(self->s_self, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_reversed_t)specs.smh_cb)(Dee_AsObject(self), start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, size_t, size_t))specs.smh_cb)(self->s_type, self->s_self, start, end);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) int DCALL
super_mh__seq_sort(DeeSuperObject *self, size_t start, size_t end) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_sort, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_sort_t)specs.smh_cb)(self->s_self, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_sort_t)specs.smh_cb)(Dee_AsObject(self), start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, size_t, size_t))specs.smh_cb)(self->s_type, self->s_self, start, end);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
super_mh__seq_sort_with_key(DeeSuperObject *self, size_t start, size_t end, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_sort_with_key, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_sort_with_key_t)specs.smh_cb)(self->s_self, start, end, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_sort_with_key_t)specs.smh_cb)(Dee_AsObject(self), start, end, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, size_t, size_t, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, start, end, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__seq_sorted(DeeSuperObject *self, size_t start, size_t end) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_sorted, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_sorted_t)specs.smh_cb)(self->s_self, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_sorted_t)specs.smh_cb)(Dee_AsObject(self), start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, size_t, size_t))specs.smh_cb)(self->s_type, self->s_self, start, end);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
super_mh__seq_sorted_with_key(DeeSuperObject *self, size_t start, size_t end, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_sorted_with_key, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_sorted_with_key_t)specs.smh_cb)(self->s_self, start, end, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_sorted_with_key_t)specs.smh_cb)(Dee_AsObject(self), start, end, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, size_t, size_t, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, start, end, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
super_mh__seq_bfind(DeeSuperObject *self, DeeObject *item, size_t start, size_t end) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_bfind, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_bfind_t)specs.smh_cb)(self->s_self, item, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_bfind_t)specs.smh_cb)(Dee_AsObject(self), item, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(size_t (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, size_t, size_t))specs.smh_cb)(self->s_type, self->s_self, item, start, end);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
super_mh__seq_bfind_with_key(DeeSuperObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_bfind_with_key, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_bfind_with_key_t)specs.smh_cb)(self->s_self, item, start, end, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_bfind_with_key_t)specs.smh_cb)(Dee_AsObject(self), item, start, end, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(size_t (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, size_t, size_t, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, item, start, end, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
super_mh__seq_bposition(DeeSuperObject *self, DeeObject *item, size_t start, size_t end) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_bposition, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_bposition_t)specs.smh_cb)(self->s_self, item, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_bposition_t)specs.smh_cb)(Dee_AsObject(self), item, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(size_t (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, size_t, size_t))specs.smh_cb)(self->s_type, self->s_self, item, start, end);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
super_mh__seq_bposition_with_key(DeeSuperObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_bposition_with_key, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_bposition_with_key_t)specs.smh_cb)(self->s_self, item, start, end, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_bposition_with_key_t)specs.smh_cb)(Dee_AsObject(self), item, start, end, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(size_t (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, size_t, size_t, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, item, start, end, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
super_mh__seq_brange(DeeSuperObject *self, DeeObject *item, size_t start, size_t end, size_t result_range[2]) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_brange, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_brange_t)specs.smh_cb)(self->s_self, item, start, end, result_range);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_brange_t)specs.smh_cb)(Dee_AsObject(self), item, start, end, result_range);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, size_t, size_t, size_t*))specs.smh_cb)(self->s_type, self->s_self, item, start, end, result_range);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 5, 6)) int DCALL
super_mh__seq_brange_with_key(DeeSuperObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key, size_t result_range[2]) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_seq_brange_with_key, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_seq_brange_with_key_t)specs.smh_cb)(self->s_self, item, start, end, key, result_range);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_seq_brange_with_key_t)specs.smh_cb)(Dee_AsObject(self), item, start, end, key, result_range);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, size_t, size_t, DeeObject *, size_t*))specs.smh_cb)(self->s_type, self->s_self, item, start, end, key, result_range);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__set_operator_iter(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_set_operator_iter, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_operator_iter_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_operator_iter_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
super_mh__set_operator_foreach(DeeSuperObject *__restrict self, Dee_foreach_t cb, void *arg) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_set_operator_foreach, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_operator_foreach_t)specs.smh_cb)(self->s_self, cb, arg);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_operator_foreach_t)specs.smh_cb)(Dee_AsObject(self), cb, arg);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(Dee_ssize_t (DCALL *)(DeeTypeObject *, DeeObject *, Dee_foreach_t, void *))specs.smh_cb)(self->s_type, self->s_self, cb, arg);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__set_operator_sizeob(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_set_operator_sizeob, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_operator_sizeob_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_operator_sizeob_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
super_mh__set_operator_size(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_set_operator_size, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_operator_size_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_operator_size_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(size_t (DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
super_mh__set_operator_hash(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_set_operator_hash, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_operator_hash_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_operator_hash_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(Dee_hash_t (DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__set_operator_compare_eq(DeeSuperObject *lhs, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(lhs, Dee_TMH_set_operator_compare_eq, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_operator_compare_eq_t)specs.smh_cb)(lhs->s_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_operator_compare_eq_t)specs.smh_cb)(Dee_AsObject(lhs), rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(lhs->s_type, lhs->s_self, rhs);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__set_operator_trycompare_eq(DeeSuperObject *lhs, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(lhs, Dee_TMH_set_operator_trycompare_eq, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_operator_trycompare_eq_t)specs.smh_cb)(lhs->s_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_operator_trycompare_eq_t)specs.smh_cb)(Dee_AsObject(lhs), rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(lhs->s_type, lhs->s_self, rhs);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__set_operator_eq(DeeSuperObject *lhs, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(lhs, Dee_TMH_set_operator_eq, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_operator_eq_t)specs.smh_cb)(lhs->s_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_operator_eq_t)specs.smh_cb)(Dee_AsObject(lhs), rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(lhs->s_type, lhs->s_self, rhs);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__set_operator_ne(DeeSuperObject *lhs, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(lhs, Dee_TMH_set_operator_ne, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_operator_ne_t)specs.smh_cb)(lhs->s_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_operator_ne_t)specs.smh_cb)(Dee_AsObject(lhs), rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(lhs->s_type, lhs->s_self, rhs);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__set_operator_lo(DeeSuperObject *lhs, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(lhs, Dee_TMH_set_operator_lo, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_operator_lo_t)specs.smh_cb)(lhs->s_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_operator_lo_t)specs.smh_cb)(Dee_AsObject(lhs), rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(lhs->s_type, lhs->s_self, rhs);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__set_operator_le(DeeSuperObject *lhs, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(lhs, Dee_TMH_set_operator_le, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_operator_le_t)specs.smh_cb)(lhs->s_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_operator_le_t)specs.smh_cb)(Dee_AsObject(lhs), rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(lhs->s_type, lhs->s_self, rhs);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__set_operator_gr(DeeSuperObject *lhs, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(lhs, Dee_TMH_set_operator_gr, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_operator_gr_t)specs.smh_cb)(lhs->s_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_operator_gr_t)specs.smh_cb)(Dee_AsObject(lhs), rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(lhs->s_type, lhs->s_self, rhs);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__set_operator_ge(DeeSuperObject *lhs, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(lhs, Dee_TMH_set_operator_ge, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_operator_ge_t)specs.smh_cb)(lhs->s_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_operator_ge_t)specs.smh_cb)(Dee_AsObject(lhs), rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(lhs->s_type, lhs->s_self, rhs);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) int DCALL
super_mh__set_operator_bool(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_set_operator_bool, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_operator_bool_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_operator_bool_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__set_operator_inv(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_set_operator_inv, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_operator_inv_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_operator_inv_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__set_operator_add(DeeSuperObject *lhs, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(lhs, Dee_TMH_set_operator_add, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_operator_add_t)specs.smh_cb)(lhs->s_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_operator_add_t)specs.smh_cb)(Dee_AsObject(lhs), rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(lhs->s_type, lhs->s_self, rhs);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__set_operator_sub(DeeSuperObject *lhs, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(lhs, Dee_TMH_set_operator_sub, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_operator_sub_t)specs.smh_cb)(lhs->s_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_operator_sub_t)specs.smh_cb)(Dee_AsObject(lhs), rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(lhs->s_type, lhs->s_self, rhs);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__set_operator_and(DeeSuperObject *lhs, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(lhs, Dee_TMH_set_operator_and, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_operator_and_t)specs.smh_cb)(lhs->s_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_operator_and_t)specs.smh_cb)(Dee_AsObject(lhs), rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(lhs->s_type, lhs->s_self, rhs);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__set_operator_xor(DeeSuperObject *lhs, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(lhs, Dee_TMH_set_operator_xor, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_operator_xor_t)specs.smh_cb)(lhs->s_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_operator_xor_t)specs.smh_cb)(Dee_AsObject(lhs), rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(lhs->s_type, lhs->s_self, rhs);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__set_operator_inplace_add(DREF DeeSuperObject **__restrict p_self, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper((*p_self), Dee_TMH_set_operator_inplace_add, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF: {
		int _res;
		DREF DeeObject *_self = (*p_self)->s_self;
		Dee_Incref(_self);
		_res = (*(DeeMH_set_operator_inplace_add_t)specs.smh_cb)(&_self, rhs);
		return repack_super_after_inplace_int(_res, _self, p_self);
	}	break;
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_operator_inplace_add_t)specs.smh_cb)((DREF DeeObject **)p_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE: {
		int _res;
		DREF DeeObject *_self = (*p_self)->s_self;
		Dee_Incref(_self);
		_res = (*(int (DCALL *)(DeeTypeObject *, DREF DeeObject **, DeeObject *))specs.smh_cb)((*p_self)->s_type, &_self, rhs);
		return repack_super_after_inplace_int(_res, _self, p_self);
	}	break;
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__set_operator_inplace_sub(DREF DeeSuperObject **__restrict p_self, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper((*p_self), Dee_TMH_set_operator_inplace_sub, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF: {
		int _res;
		DREF DeeObject *_self = (*p_self)->s_self;
		Dee_Incref(_self);
		_res = (*(DeeMH_set_operator_inplace_sub_t)specs.smh_cb)(&_self, rhs);
		return repack_super_after_inplace_int(_res, _self, p_self);
	}	break;
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_operator_inplace_sub_t)specs.smh_cb)((DREF DeeObject **)p_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE: {
		int _res;
		DREF DeeObject *_self = (*p_self)->s_self;
		Dee_Incref(_self);
		_res = (*(int (DCALL *)(DeeTypeObject *, DREF DeeObject **, DeeObject *))specs.smh_cb)((*p_self)->s_type, &_self, rhs);
		return repack_super_after_inplace_int(_res, _self, p_self);
	}	break;
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__set_operator_inplace_and(DREF DeeSuperObject **__restrict p_self, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper((*p_self), Dee_TMH_set_operator_inplace_and, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF: {
		int _res;
		DREF DeeObject *_self = (*p_self)->s_self;
		Dee_Incref(_self);
		_res = (*(DeeMH_set_operator_inplace_and_t)specs.smh_cb)(&_self, rhs);
		return repack_super_after_inplace_int(_res, _self, p_self);
	}	break;
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_operator_inplace_and_t)specs.smh_cb)((DREF DeeObject **)p_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE: {
		int _res;
		DREF DeeObject *_self = (*p_self)->s_self;
		Dee_Incref(_self);
		_res = (*(int (DCALL *)(DeeTypeObject *, DREF DeeObject **, DeeObject *))specs.smh_cb)((*p_self)->s_type, &_self, rhs);
		return repack_super_after_inplace_int(_res, _self, p_self);
	}	break;
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__set_operator_inplace_xor(DREF DeeSuperObject **__restrict p_self, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper((*p_self), Dee_TMH_set_operator_inplace_xor, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF: {
		int _res;
		DREF DeeObject *_self = (*p_self)->s_self;
		Dee_Incref(_self);
		_res = (*(DeeMH_set_operator_inplace_xor_t)specs.smh_cb)(&_self, rhs);
		return repack_super_after_inplace_int(_res, _self, p_self);
	}	break;
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_operator_inplace_xor_t)specs.smh_cb)((DREF DeeObject **)p_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE: {
		int _res;
		DREF DeeObject *_self = (*p_self)->s_self;
		Dee_Incref(_self);
		_res = (*(int (DCALL *)(DeeTypeObject *, DREF DeeObject **, DeeObject *))specs.smh_cb)((*p_self)->s_type, &_self, rhs);
		return repack_super_after_inplace_int(_res, _self, p_self);
	}	break;
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__set_frozen(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_set_frozen, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_frozen_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_frozen_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__set_unify(DeeSuperObject *self, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_set_unify, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_unify_t)specs.smh_cb)(self->s_self, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_unify_t)specs.smh_cb)(Dee_AsObject(self), key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__set_insert(DeeSuperObject *self, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_set_insert, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_insert_t)specs.smh_cb)(self->s_self, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_insert_t)specs.smh_cb)(Dee_AsObject(self), key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__set_insertall(DeeSuperObject *self, DeeObject *keys) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_set_insertall, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_insertall_t)specs.smh_cb)(self->s_self, keys);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_insertall_t)specs.smh_cb)(Dee_AsObject(self), keys);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, keys);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__set_remove(DeeSuperObject *self, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_set_remove, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_remove_t)specs.smh_cb)(self->s_self, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_remove_t)specs.smh_cb)(Dee_AsObject(self), key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__set_removeall(DeeSuperObject *self, DeeObject *keys) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_set_removeall, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_removeall_t)specs.smh_cb)(self->s_self, keys);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_removeall_t)specs.smh_cb)(Dee_AsObject(self), keys);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, keys);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__set_pop(DeeSuperObject *self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_set_pop, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_pop_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_pop_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__set_pop_with_default(DeeSuperObject *self, DeeObject *default_) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_set_pop_with_default, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_pop_with_default_t)specs.smh_cb)(self->s_self, default_);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_pop_with_default_t)specs.smh_cb)(Dee_AsObject(self), default_);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, default_);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__set_trygetfirst(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_set_trygetfirst, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_trygetfirst_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_trygetfirst_t)specs.smh_cb)(Dee_AsObject(self));
/*	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE: // Unused */
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__set_getfirst(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_set_getfirst, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_getfirst_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_getfirst_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) int DCALL
super_mh__set_boundfirst(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_set_boundfirst, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_boundfirst_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_boundfirst_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) int DCALL
super_mh__set_delfirst(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_set_delfirst, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_delfirst_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_delfirst_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__set_setfirst(DeeSuperObject *self, DeeObject *value) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_set_setfirst, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_setfirst_t)specs.smh_cb)(self->s_self, value);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_setfirst_t)specs.smh_cb)(Dee_AsObject(self), value);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, value);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__set_trygetlast(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_set_trygetlast, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_trygetlast_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_trygetlast_t)specs.smh_cb)(Dee_AsObject(self));
/*	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE: // Unused */
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__set_getlast(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_set_getlast, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_getlast_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_getlast_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) int DCALL
super_mh__set_boundlast(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_set_boundlast, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_boundlast_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_boundlast_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) int DCALL
super_mh__set_dellast(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_set_dellast, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_dellast_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_dellast_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__set_setlast(DeeSuperObject *self, DeeObject *value) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_set_setlast, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_set_setlast_t)specs.smh_cb)(self->s_self, value);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_set_setlast_t)specs.smh_cb)(Dee_AsObject(self), value);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, value);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__map_operator_iter(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_operator_iter, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_iter_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_iter_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
super_mh__map_operator_foreach_pair(DeeSuperObject *__restrict self, Dee_foreach_pair_t cb, void *arg) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_operator_foreach_pair, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_foreach_pair_t)specs.smh_cb)(self->s_self, cb, arg);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_foreach_pair_t)specs.smh_cb)(Dee_AsObject(self), cb, arg);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(Dee_ssize_t (DCALL *)(DeeTypeObject *, DeeObject *, Dee_foreach_pair_t, void *))specs.smh_cb)(self->s_type, self->s_self, cb, arg);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__map_operator_sizeob(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_operator_sizeob, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_sizeob_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_sizeob_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
super_mh__map_operator_size(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_operator_size, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_size_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_size_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(size_t (DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
super_mh__map_operator_hash(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_operator_hash, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_hash_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_hash_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(Dee_hash_t (DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__map_operator_getitem(DeeSuperObject *self, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_operator_getitem, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_getitem_t)specs.smh_cb)(self->s_self, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_getitem_t)specs.smh_cb)(Dee_AsObject(self), key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__map_operator_trygetitem(DeeSuperObject *self, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_operator_trygetitem, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_trygetitem_t)specs.smh_cb)(self->s_self, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_trygetitem_t)specs.smh_cb)(Dee_AsObject(self), key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__map_operator_getitem_index(DeeSuperObject *self, size_t key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_operator_getitem_index, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_getitem_index_t)specs.smh_cb)(self->s_self, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_getitem_index_t)specs.smh_cb)(Dee_AsObject(self), key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, size_t))specs.smh_cb)(self->s_type, self->s_self, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__map_operator_trygetitem_index(DeeSuperObject *self, size_t key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_operator_trygetitem_index, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_trygetitem_index_t)specs.smh_cb)(self->s_self, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_trygetitem_index_t)specs.smh_cb)(Dee_AsObject(self), key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, size_t))specs.smh_cb)(self->s_type, self->s_self, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__map_operator_getitem_string_hash(DeeSuperObject *self, char const *key, Dee_hash_t hash) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_operator_getitem_string_hash, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_getitem_string_hash_t)specs.smh_cb)(self->s_self, key, hash);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_getitem_string_hash_t)specs.smh_cb)(Dee_AsObject(self), key, hash);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, char const *, Dee_hash_t))specs.smh_cb)(self->s_type, self->s_self, key, hash);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__map_operator_trygetitem_string_hash(DeeSuperObject *self, char const *key, Dee_hash_t hash) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_operator_trygetitem_string_hash, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_trygetitem_string_hash_t)specs.smh_cb)(self->s_self, key, hash);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_trygetitem_string_hash_t)specs.smh_cb)(Dee_AsObject(self), key, hash);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, char const *, Dee_hash_t))specs.smh_cb)(self->s_type, self->s_self, key, hash);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__map_operator_getitem_string_len_hash(DeeSuperObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_operator_getitem_string_len_hash, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_getitem_string_len_hash_t)specs.smh_cb)(self->s_self, key, keylen, hash);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_getitem_string_len_hash_t)specs.smh_cb)(Dee_AsObject(self), key, keylen, hash);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, char const *, size_t, Dee_hash_t))specs.smh_cb)(self->s_type, self->s_self, key, keylen, hash);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__map_operator_trygetitem_string_len_hash(DeeSuperObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_operator_trygetitem_string_len_hash, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_trygetitem_string_len_hash_t)specs.smh_cb)(self->s_self, key, keylen, hash);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_trygetitem_string_len_hash_t)specs.smh_cb)(Dee_AsObject(self), key, keylen, hash);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, char const *, size_t, Dee_hash_t))specs.smh_cb)(self->s_type, self->s_self, key, keylen, hash);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__map_operator_bounditem(DeeSuperObject *self, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_operator_bounditem, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_bounditem_t)specs.smh_cb)(self->s_self, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_bounditem_t)specs.smh_cb)(Dee_AsObject(self), key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) int DCALL
super_mh__map_operator_bounditem_index(DeeSuperObject *self, size_t key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_operator_bounditem_index, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_bounditem_index_t)specs.smh_cb)(self->s_self, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_bounditem_index_t)specs.smh_cb)(Dee_AsObject(self), key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, size_t))specs.smh_cb)(self->s_type, self->s_self, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__map_operator_bounditem_string_hash(DeeSuperObject *self, char const *key, Dee_hash_t hash) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_operator_bounditem_string_hash, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_bounditem_string_hash_t)specs.smh_cb)(self->s_self, key, hash);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_bounditem_string_hash_t)specs.smh_cb)(Dee_AsObject(self), key, hash);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, char const *, Dee_hash_t))specs.smh_cb)(self->s_type, self->s_self, key, hash);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) int DCALL
super_mh__map_operator_bounditem_string_len_hash(DeeSuperObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_operator_bounditem_string_len_hash, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_bounditem_string_len_hash_t)specs.smh_cb)(self->s_self, key, keylen, hash);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_bounditem_string_len_hash_t)specs.smh_cb)(Dee_AsObject(self), key, keylen, hash);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, char const *, size_t, Dee_hash_t))specs.smh_cb)(self->s_type, self->s_self, key, keylen, hash);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__map_operator_hasitem(DeeSuperObject *self, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_operator_hasitem, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_hasitem_t)specs.smh_cb)(self->s_self, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_hasitem_t)specs.smh_cb)(Dee_AsObject(self), key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) int DCALL
super_mh__map_operator_hasitem_index(DeeSuperObject *self, size_t key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_operator_hasitem_index, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_hasitem_index_t)specs.smh_cb)(self->s_self, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_hasitem_index_t)specs.smh_cb)(Dee_AsObject(self), key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, size_t))specs.smh_cb)(self->s_type, self->s_self, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__map_operator_hasitem_string_hash(DeeSuperObject *self, char const *key, Dee_hash_t hash) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_operator_hasitem_string_hash, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_hasitem_string_hash_t)specs.smh_cb)(self->s_self, key, hash);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_hasitem_string_hash_t)specs.smh_cb)(Dee_AsObject(self), key, hash);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, char const *, Dee_hash_t))specs.smh_cb)(self->s_type, self->s_self, key, hash);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) int DCALL
super_mh__map_operator_hasitem_string_len_hash(DeeSuperObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_operator_hasitem_string_len_hash, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_hasitem_string_len_hash_t)specs.smh_cb)(self->s_self, key, keylen, hash);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_hasitem_string_len_hash_t)specs.smh_cb)(Dee_AsObject(self), key, keylen, hash);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, char const *, size_t, Dee_hash_t))specs.smh_cb)(self->s_type, self->s_self, key, keylen, hash);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__map_operator_delitem(DeeSuperObject *self, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_operator_delitem, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_delitem_t)specs.smh_cb)(self->s_self, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_delitem_t)specs.smh_cb)(Dee_AsObject(self), key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) int DCALL
super_mh__map_operator_delitem_index(DeeSuperObject *self, size_t key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_operator_delitem_index, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_delitem_index_t)specs.smh_cb)(self->s_self, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_delitem_index_t)specs.smh_cb)(Dee_AsObject(self), key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, size_t))specs.smh_cb)(self->s_type, self->s_self, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__map_operator_delitem_string_hash(DeeSuperObject *self, char const *key, Dee_hash_t hash) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_operator_delitem_string_hash, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_delitem_string_hash_t)specs.smh_cb)(self->s_self, key, hash);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_delitem_string_hash_t)specs.smh_cb)(Dee_AsObject(self), key, hash);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, char const *, Dee_hash_t))specs.smh_cb)(self->s_type, self->s_self, key, hash);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) int DCALL
super_mh__map_operator_delitem_string_len_hash(DeeSuperObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_operator_delitem_string_len_hash, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_delitem_string_len_hash_t)specs.smh_cb)(self->s_self, key, keylen, hash);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_delitem_string_len_hash_t)specs.smh_cb)(Dee_AsObject(self), key, keylen, hash);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, char const *, size_t, Dee_hash_t))specs.smh_cb)(self->s_type, self->s_self, key, keylen, hash);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
super_mh__map_operator_setitem(DeeSuperObject *self, DeeObject *key, DeeObject *value) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_operator_setitem, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_setitem_t)specs.smh_cb)(self->s_self, key, value);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_setitem_t)specs.smh_cb)(Dee_AsObject(self), key, value);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, key, value);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
super_mh__map_operator_setitem_index(DeeSuperObject *self, size_t key, DeeObject *value) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_operator_setitem_index, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_setitem_index_t)specs.smh_cb)(self->s_self, key, value);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_setitem_index_t)specs.smh_cb)(Dee_AsObject(self), key, value);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, size_t, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, key, value);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 4)) int DCALL
super_mh__map_operator_setitem_string_hash(DeeSuperObject *self, char const *key, Dee_hash_t hash, DeeObject *value) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_operator_setitem_string_hash, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_setitem_string_hash_t)specs.smh_cb)(self->s_self, key, hash, value);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_setitem_string_hash_t)specs.smh_cb)(Dee_AsObject(self), key, hash, value);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, char const *, Dee_hash_t, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, key, hash, value);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 5)) int DCALL
super_mh__map_operator_setitem_string_len_hash(DeeSuperObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_operator_setitem_string_len_hash, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_setitem_string_len_hash_t)specs.smh_cb)(self->s_self, key, keylen, hash, value);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_setitem_string_len_hash_t)specs.smh_cb)(Dee_AsObject(self), key, keylen, hash, value);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, key, keylen, hash, value);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__map_operator_contains(DeeSuperObject *self, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_operator_contains, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_contains_t)specs.smh_cb)(self->s_self, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_contains_t)specs.smh_cb)(Dee_AsObject(self), key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__map_keys(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_keys, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_keys_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_keys_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__map_iterkeys(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_iterkeys, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_iterkeys_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_iterkeys_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__map_values(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_values, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_values_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_values_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__map_itervalues(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_itervalues, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_itervalues_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_itervalues_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
super_mh__map_enumerate(DeeSuperObject *__restrict self, Dee_seq_enumerate_t cb, void *arg) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_enumerate, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_enumerate_t)specs.smh_cb)(self->s_self, cb, arg);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_enumerate_t)specs.smh_cb)(Dee_AsObject(self), cb, arg);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(Dee_ssize_t (DCALL *)(DeeTypeObject *, DeeObject *, Dee_seq_enumerate_t, void *))specs.smh_cb)(self->s_type, self->s_self, cb, arg);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 4, 5)) Dee_ssize_t DCALL
super_mh__map_enumerate_range(DeeSuperObject *self, Dee_seq_enumerate_t cb, void *arg, DeeObject *start, DeeObject *end) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_enumerate_range, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_enumerate_range_t)specs.smh_cb)(self->s_self, cb, arg, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_enumerate_range_t)specs.smh_cb)(Dee_AsObject(self), cb, arg, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(Dee_ssize_t (DCALL *)(DeeTypeObject *, DeeObject *, Dee_seq_enumerate_t, void *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, cb, arg, start, end);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__map_makeenumeration(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_makeenumeration, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_makeenumeration_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_makeenumeration_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
super_mh__map_makeenumeration_with_range(DeeSuperObject *self, DeeObject *start, DeeObject *end) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_makeenumeration_with_range, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_makeenumeration_with_range_t)specs.smh_cb)(self->s_self, start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_makeenumeration_with_range_t)specs.smh_cb)(Dee_AsObject(self), start, end);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, start, end);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__map_operator_compare_eq(DeeSuperObject *lhs, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(lhs, Dee_TMH_map_operator_compare_eq, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_compare_eq_t)specs.smh_cb)(lhs->s_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_compare_eq_t)specs.smh_cb)(Dee_AsObject(lhs), rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(lhs->s_type, lhs->s_self, rhs);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__map_operator_trycompare_eq(DeeSuperObject *lhs, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(lhs, Dee_TMH_map_operator_trycompare_eq, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_trycompare_eq_t)specs.smh_cb)(lhs->s_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_trycompare_eq_t)specs.smh_cb)(Dee_AsObject(lhs), rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(lhs->s_type, lhs->s_self, rhs);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__map_operator_eq(DeeSuperObject *lhs, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(lhs, Dee_TMH_map_operator_eq, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_eq_t)specs.smh_cb)(lhs->s_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_eq_t)specs.smh_cb)(Dee_AsObject(lhs), rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(lhs->s_type, lhs->s_self, rhs);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__map_operator_ne(DeeSuperObject *lhs, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(lhs, Dee_TMH_map_operator_ne, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_ne_t)specs.smh_cb)(lhs->s_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_ne_t)specs.smh_cb)(Dee_AsObject(lhs), rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(lhs->s_type, lhs->s_self, rhs);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__map_operator_lo(DeeSuperObject *lhs, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(lhs, Dee_TMH_map_operator_lo, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_lo_t)specs.smh_cb)(lhs->s_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_lo_t)specs.smh_cb)(Dee_AsObject(lhs), rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(lhs->s_type, lhs->s_self, rhs);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__map_operator_le(DeeSuperObject *lhs, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(lhs, Dee_TMH_map_operator_le, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_le_t)specs.smh_cb)(lhs->s_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_le_t)specs.smh_cb)(Dee_AsObject(lhs), rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(lhs->s_type, lhs->s_self, rhs);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__map_operator_gr(DeeSuperObject *lhs, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(lhs, Dee_TMH_map_operator_gr, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_gr_t)specs.smh_cb)(lhs->s_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_gr_t)specs.smh_cb)(Dee_AsObject(lhs), rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(lhs->s_type, lhs->s_self, rhs);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__map_operator_ge(DeeSuperObject *lhs, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(lhs, Dee_TMH_map_operator_ge, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_ge_t)specs.smh_cb)(lhs->s_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_ge_t)specs.smh_cb)(Dee_AsObject(lhs), rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(lhs->s_type, lhs->s_self, rhs);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__map_operator_add(DeeSuperObject *lhs, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(lhs, Dee_TMH_map_operator_add, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_add_t)specs.smh_cb)(lhs->s_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_add_t)specs.smh_cb)(Dee_AsObject(lhs), rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(lhs->s_type, lhs->s_self, rhs);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__map_operator_sub(DeeSuperObject *lhs, DeeObject *keys) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(lhs, Dee_TMH_map_operator_sub, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_sub_t)specs.smh_cb)(lhs->s_self, keys);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_sub_t)specs.smh_cb)(Dee_AsObject(lhs), keys);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(lhs->s_type, lhs->s_self, keys);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__map_operator_and(DeeSuperObject *lhs, DeeObject *keys) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(lhs, Dee_TMH_map_operator_and, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_and_t)specs.smh_cb)(lhs->s_self, keys);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_and_t)specs.smh_cb)(Dee_AsObject(lhs), keys);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(lhs->s_type, lhs->s_self, keys);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__map_operator_xor(DeeSuperObject *lhs, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(lhs, Dee_TMH_map_operator_xor, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_operator_xor_t)specs.smh_cb)(lhs->s_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_xor_t)specs.smh_cb)(Dee_AsObject(lhs), rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(lhs->s_type, lhs->s_self, rhs);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__map_operator_inplace_add(DREF DeeSuperObject **__restrict p_self, DeeObject *items) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper((*p_self), Dee_TMH_map_operator_inplace_add, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF: {
		int _res;
		DREF DeeObject *_self = (*p_self)->s_self;
		Dee_Incref(_self);
		_res = (*(DeeMH_map_operator_inplace_add_t)specs.smh_cb)(&_self, items);
		return repack_super_after_inplace_int(_res, _self, p_self);
	}	break;
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_inplace_add_t)specs.smh_cb)((DREF DeeObject **)p_self, items);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE: {
		int _res;
		DREF DeeObject *_self = (*p_self)->s_self;
		Dee_Incref(_self);
		_res = (*(int (DCALL *)(DeeTypeObject *, DREF DeeObject **, DeeObject *))specs.smh_cb)((*p_self)->s_type, &_self, items);
		return repack_super_after_inplace_int(_res, _self, p_self);
	}	break;
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__map_operator_inplace_sub(DREF DeeSuperObject **__restrict p_self, DeeObject *keys) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper((*p_self), Dee_TMH_map_operator_inplace_sub, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF: {
		int _res;
		DREF DeeObject *_self = (*p_self)->s_self;
		Dee_Incref(_self);
		_res = (*(DeeMH_map_operator_inplace_sub_t)specs.smh_cb)(&_self, keys);
		return repack_super_after_inplace_int(_res, _self, p_self);
	}	break;
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_inplace_sub_t)specs.smh_cb)((DREF DeeObject **)p_self, keys);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE: {
		int _res;
		DREF DeeObject *_self = (*p_self)->s_self;
		Dee_Incref(_self);
		_res = (*(int (DCALL *)(DeeTypeObject *, DREF DeeObject **, DeeObject *))specs.smh_cb)((*p_self)->s_type, &_self, keys);
		return repack_super_after_inplace_int(_res, _self, p_self);
	}	break;
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__map_operator_inplace_and(DREF DeeSuperObject **__restrict p_self, DeeObject *keys) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper((*p_self), Dee_TMH_map_operator_inplace_and, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF: {
		int _res;
		DREF DeeObject *_self = (*p_self)->s_self;
		Dee_Incref(_self);
		_res = (*(DeeMH_map_operator_inplace_and_t)specs.smh_cb)(&_self, keys);
		return repack_super_after_inplace_int(_res, _self, p_self);
	}	break;
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_inplace_and_t)specs.smh_cb)((DREF DeeObject **)p_self, keys);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE: {
		int _res;
		DREF DeeObject *_self = (*p_self)->s_self;
		Dee_Incref(_self);
		_res = (*(int (DCALL *)(DeeTypeObject *, DREF DeeObject **, DeeObject *))specs.smh_cb)((*p_self)->s_type, &_self, keys);
		return repack_super_after_inplace_int(_res, _self, p_self);
	}	break;
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__map_operator_inplace_xor(DREF DeeSuperObject **__restrict p_self, DeeObject *rhs) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper((*p_self), Dee_TMH_map_operator_inplace_xor, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF: {
		int _res;
		DREF DeeObject *_self = (*p_self)->s_self;
		Dee_Incref(_self);
		_res = (*(DeeMH_map_operator_inplace_xor_t)specs.smh_cb)(&_self, rhs);
		return repack_super_after_inplace_int(_res, _self, p_self);
	}	break;
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_operator_inplace_xor_t)specs.smh_cb)((DREF DeeObject **)p_self, rhs);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE: {
		int _res;
		DREF DeeObject *_self = (*p_self)->s_self;
		Dee_Incref(_self);
		_res = (*(int (DCALL *)(DeeTypeObject *, DREF DeeObject **, DeeObject *))specs.smh_cb)((*p_self)->s_type, &_self, rhs);
		return repack_super_after_inplace_int(_res, _self, p_self);
	}	break;
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__map_frozen(DeeSuperObject *__restrict self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_frozen, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_frozen_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_frozen_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
super_mh__map_setold(DeeSuperObject *self, DeeObject *key, DeeObject *value) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_setold, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_setold_t)specs.smh_cb)(self->s_self, key, value);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_setold_t)specs.smh_cb)(Dee_AsObject(self), key, value);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, key, value);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
super_mh__map_setold_ex(DeeSuperObject *self, DeeObject *key, DeeObject *value) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_setold_ex, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_setold_ex_t)specs.smh_cb)(self->s_self, key, value);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_setold_ex_t)specs.smh_cb)(Dee_AsObject(self), key, value);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, key, value);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
super_mh__map_setnew(DeeSuperObject *self, DeeObject *key, DeeObject *value) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_setnew, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_setnew_t)specs.smh_cb)(self->s_self, key, value);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_setnew_t)specs.smh_cb)(Dee_AsObject(self), key, value);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, key, value);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
super_mh__map_setnew_ex(DeeSuperObject *self, DeeObject *key, DeeObject *value) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_setnew_ex, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_setnew_ex_t)specs.smh_cb)(self->s_self, key, value);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_setnew_ex_t)specs.smh_cb)(Dee_AsObject(self), key, value);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, key, value);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
super_mh__map_setdefault(DeeSuperObject *self, DeeObject *key, DeeObject *value) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_setdefault, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_setdefault_t)specs.smh_cb)(self->s_self, key, value);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_setdefault_t)specs.smh_cb)(Dee_AsObject(self), key, value);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, key, value);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__map_update(DeeSuperObject *self, DeeObject *items) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_update, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_update_t)specs.smh_cb)(self->s_self, items);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_update_t)specs.smh_cb)(Dee_AsObject(self), items);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, items);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__map_remove(DeeSuperObject *self, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_remove, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_remove_t)specs.smh_cb)(self->s_self, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_remove_t)specs.smh_cb)(Dee_AsObject(self), key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
super_mh__map_removekeys(DeeSuperObject *self, DeeObject *keys) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_removekeys, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_removekeys_t)specs.smh_cb)(self->s_self, keys);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_removekeys_t)specs.smh_cb)(Dee_AsObject(self), keys);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, keys);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_mh__map_pop(DeeSuperObject *self, DeeObject *key) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_pop, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_pop_t)specs.smh_cb)(self->s_self, key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_pop_t)specs.smh_cb)(Dee_AsObject(self), key);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, key);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
super_mh__map_pop_with_default(DeeSuperObject *self, DeeObject *key, DeeObject *default_) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_pop_with_default, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_pop_with_default_t)specs.smh_cb)(self->s_self, key, default_);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_pop_with_default_t)specs.smh_cb)(Dee_AsObject(self), key, default_);
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self, key, default_);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
super_mh__map_popitem(DeeSuperObject *self) {
	struct Dee_super_method_hint specs;
	DeeType_GetMethodHintForSuper(self, Dee_TMH_map_popitem, &specs);
	switch (specs.smh_cc) {
	case Dee_SUPER_METHOD_HINT_CC_WITH_SELF:
		return (*(DeeMH_map_popitem_t)specs.smh_cb)(self->s_self);
	case Dee_SUPER_METHOD_HINT_CC_WITH_SUPER:
		return (*(DeeMH_map_popitem_t)specs.smh_cb)(Dee_AsObject(self));
	case Dee_SUPER_METHOD_HINT_CC_WITH_TYPE:
		return (*(DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *))specs.smh_cb)(self->s_type, self->s_self);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}
/*[[[end]]]*/
/* clang-format on */


/* Fully initialized method hint cache for "Super" */
INTERN struct Dee_type_mh_cache super_mhcache = {
/* clang-format off */
/*[[[deemon (printSuperMethodHintCacheInitializer from "..method-hints.method-hints")();]]]*/
	/* .mh_seq_operator_bool                       = */ (DeeMH_seq_operator_bool_t)&super_mh__seq_operator_bool,
	/* .mh_seq_operator_sizeob                     = */ (DeeMH_seq_operator_sizeob_t)&super_mh__seq_operator_sizeob,
	/* .mh_seq_operator_size                       = */ (DeeMH_seq_operator_size_t)&super_mh__seq_operator_size,
	/* .mh_seq_operator_iter                       = */ (DeeMH_seq_operator_iter_t)&super_mh__seq_operator_iter,
	/* .mh_seq_operator_foreach                    = */ (DeeMH_seq_operator_foreach_t)&super_mh__seq_operator_foreach,
	/* .mh_seq_operator_foreach_pair               = */ (DeeMH_seq_operator_foreach_pair_t)&super_mh__seq_operator_foreach_pair,
	/* .mh_seq_operator_getitem                    = */ (DeeMH_seq_operator_getitem_t)&super_mh__seq_operator_getitem,
	/* .mh_seq_operator_getitem_index              = */ (DeeMH_seq_operator_getitem_index_t)&super_mh__seq_operator_getitem_index,
	/* .mh_seq_operator_trygetitem                 = */ (DeeMH_seq_operator_trygetitem_t)&super_mh__seq_operator_trygetitem,
	/* .mh_seq_operator_trygetitem_index           = */ (DeeMH_seq_operator_trygetitem_index_t)&super_mh__seq_operator_trygetitem_index,
	/* .mh_seq_operator_hasitem                    = */ (DeeMH_seq_operator_hasitem_t)&super_mh__seq_operator_hasitem,
	/* .mh_seq_operator_hasitem_index              = */ (DeeMH_seq_operator_hasitem_index_t)&super_mh__seq_operator_hasitem_index,
	/* .mh_seq_operator_bounditem                  = */ (DeeMH_seq_operator_bounditem_t)&super_mh__seq_operator_bounditem,
	/* .mh_seq_operator_bounditem_index            = */ (DeeMH_seq_operator_bounditem_index_t)&super_mh__seq_operator_bounditem_index,
	/* .mh_seq_operator_delitem                    = */ (DeeMH_seq_operator_delitem_t)&super_mh__seq_operator_delitem,
	/* .mh_seq_operator_delitem_index              = */ (DeeMH_seq_operator_delitem_index_t)&super_mh__seq_operator_delitem_index,
	/* .mh_seq_operator_setitem                    = */ (DeeMH_seq_operator_setitem_t)&super_mh__seq_operator_setitem,
	/* .mh_seq_operator_setitem_index              = */ (DeeMH_seq_operator_setitem_index_t)&super_mh__seq_operator_setitem_index,
	/* .mh_seq_operator_getrange                   = */ (DeeMH_seq_operator_getrange_t)&super_mh__seq_operator_getrange,
	/* .mh_seq_operator_getrange_index             = */ (DeeMH_seq_operator_getrange_index_t)&super_mh__seq_operator_getrange_index,
	/* .mh_seq_operator_getrange_index_n           = */ (DeeMH_seq_operator_getrange_index_n_t)&super_mh__seq_operator_getrange_index_n,
	/* .mh_seq_operator_delrange                   = */ (DeeMH_seq_operator_delrange_t)&super_mh__seq_operator_delrange,
	/* .mh_seq_operator_delrange_index             = */ (DeeMH_seq_operator_delrange_index_t)&super_mh__seq_operator_delrange_index,
	/* .mh_seq_operator_delrange_index_n           = */ (DeeMH_seq_operator_delrange_index_n_t)&super_mh__seq_operator_delrange_index_n,
	/* .mh_seq_operator_setrange                   = */ (DeeMH_seq_operator_setrange_t)&super_mh__seq_operator_setrange,
	/* .mh_seq_operator_setrange_index             = */ (DeeMH_seq_operator_setrange_index_t)&super_mh__seq_operator_setrange_index,
	/* .mh_seq_operator_setrange_index_n           = */ (DeeMH_seq_operator_setrange_index_n_t)&super_mh__seq_operator_setrange_index_n,
	/* .mh_seq_operator_assign                     = */ (DeeMH_seq_operator_assign_t)&super_mh__seq_operator_assign,
	/* .mh_seq_operator_hash                       = */ (DeeMH_seq_operator_hash_t)&super_mh__seq_operator_hash,
	/* .mh_seq_operator_compare                    = */ (DeeMH_seq_operator_compare_t)&super_mh__seq_operator_compare,
	/* .mh_seq_operator_compare_eq                 = */ (DeeMH_seq_operator_compare_eq_t)&super_mh__seq_operator_compare_eq,
	/* .mh_seq_operator_trycompare_eq              = */ (DeeMH_seq_operator_trycompare_eq_t)&super_mh__seq_operator_trycompare_eq,
	/* .mh_seq_operator_eq                         = */ (DeeMH_seq_operator_eq_t)&super_mh__seq_operator_eq,
	/* .mh_seq_operator_ne                         = */ (DeeMH_seq_operator_ne_t)&super_mh__seq_operator_ne,
	/* .mh_seq_operator_lo                         = */ (DeeMH_seq_operator_lo_t)&super_mh__seq_operator_lo,
	/* .mh_seq_operator_le                         = */ (DeeMH_seq_operator_le_t)&super_mh__seq_operator_le,
	/* .mh_seq_operator_gr                         = */ (DeeMH_seq_operator_gr_t)&super_mh__seq_operator_gr,
	/* .mh_seq_operator_ge                         = */ (DeeMH_seq_operator_ge_t)&super_mh__seq_operator_ge,
	/* .mh_seq_operator_add                        = */ (DeeMH_seq_operator_add_t)&super_mh__seq_operator_add,
	/* .mh_seq_operator_mul                        = */ (DeeMH_seq_operator_mul_t)&super_mh__seq_operator_mul,
	/* .mh_seq_operator_inplace_add                = */ (DeeMH_seq_operator_inplace_add_t)&super_mh__seq_operator_inplace_add,
	/* .mh_seq_operator_inplace_mul                = */ (DeeMH_seq_operator_inplace_mul_t)&super_mh__seq_operator_inplace_mul,
	/* .mh_seq_enumerate                           = */ (DeeMH_seq_enumerate_t)&super_mh__seq_enumerate,
	/* .mh_seq_enumerate_index                     = */ (DeeMH_seq_enumerate_index_t)&super_mh__seq_enumerate_index,
	/* .mh_seq_makeenumeration                     = */ (DeeMH_seq_makeenumeration_t)&super_mh__seq_makeenumeration,
	/* .mh_seq_makeenumeration_with_range          = */ (DeeMH_seq_makeenumeration_with_range_t)&super_mh__seq_makeenumeration_with_range,
	/* .mh_seq_makeenumeration_with_intrange       = */ (DeeMH_seq_makeenumeration_with_intrange_t)&super_mh__seq_makeenumeration_with_intrange,
	/* .mh_seq_foreach_reverse                     = */ NULL,
	/* .mh_seq_enumerate_index_reverse             = */ NULL,
	/* .mh_seq_unpack                              = */ (DeeMH_seq_unpack_t)&super_mh__seq_unpack,
	/* .mh_seq_unpack_ex                           = */ (DeeMH_seq_unpack_ex_t)&super_mh__seq_unpack_ex,
	/* .mh_seq_unpack_ub                           = */ (DeeMH_seq_unpack_ub_t)&super_mh__seq_unpack_ub,
	/* .mh_seq_trygetfirst                         = */ (DeeMH_seq_trygetfirst_t)&super_mh__seq_trygetfirst,
	/* .mh_seq_getfirst                            = */ (DeeMH_seq_getfirst_t)&super_mh__seq_getfirst,
	/* .mh_seq_boundfirst                          = */ (DeeMH_seq_boundfirst_t)&super_mh__seq_boundfirst,
	/* .mh_seq_delfirst                            = */ (DeeMH_seq_delfirst_t)&super_mh__seq_delfirst,
	/* .mh_seq_setfirst                            = */ (DeeMH_seq_setfirst_t)&super_mh__seq_setfirst,
	/* .mh_seq_trygetlast                          = */ (DeeMH_seq_trygetlast_t)&super_mh__seq_trygetlast,
	/* .mh_seq_getlast                             = */ (DeeMH_seq_getlast_t)&super_mh__seq_getlast,
	/* .mh_seq_boundlast                           = */ (DeeMH_seq_boundlast_t)&super_mh__seq_boundlast,
	/* .mh_seq_dellast                             = */ (DeeMH_seq_dellast_t)&super_mh__seq_dellast,
	/* .mh_seq_setlast                             = */ (DeeMH_seq_setlast_t)&super_mh__seq_setlast,
	/* .mh_seq_cached                              = */ (DeeMH_seq_cached_t)&super_mh__seq_cached,
	/* .mh_seq_frozen                              = */ (DeeMH_seq_frozen_t)&super_mh__seq_frozen,
	/* .mh_seq_any                                 = */ (DeeMH_seq_any_t)&super_mh__seq_any,
	/* .mh_seq_any_with_key                        = */ (DeeMH_seq_any_with_key_t)&super_mh__seq_any_with_key,
	/* .mh_seq_any_with_range                      = */ (DeeMH_seq_any_with_range_t)&super_mh__seq_any_with_range,
	/* .mh_seq_any_with_range_and_key              = */ (DeeMH_seq_any_with_range_and_key_t)&super_mh__seq_any_with_range_and_key,
	/* .mh_seq_all                                 = */ (DeeMH_seq_all_t)&super_mh__seq_all,
	/* .mh_seq_all_with_key                        = */ (DeeMH_seq_all_with_key_t)&super_mh__seq_all_with_key,
	/* .mh_seq_all_with_range                      = */ (DeeMH_seq_all_with_range_t)&super_mh__seq_all_with_range,
	/* .mh_seq_all_with_range_and_key              = */ (DeeMH_seq_all_with_range_and_key_t)&super_mh__seq_all_with_range_and_key,
	/* .mh_seq_parity                              = */ (DeeMH_seq_parity_t)&super_mh__seq_parity,
	/* .mh_seq_parity_with_key                     = */ (DeeMH_seq_parity_with_key_t)&super_mh__seq_parity_with_key,
	/* .mh_seq_parity_with_range                   = */ (DeeMH_seq_parity_with_range_t)&super_mh__seq_parity_with_range,
	/* .mh_seq_parity_with_range_and_key           = */ (DeeMH_seq_parity_with_range_and_key_t)&super_mh__seq_parity_with_range_and_key,
	/* .mh_seq_reduce                              = */ (DeeMH_seq_reduce_t)&super_mh__seq_reduce,
	/* .mh_seq_reduce_with_init                    = */ (DeeMH_seq_reduce_with_init_t)&super_mh__seq_reduce_with_init,
	/* .mh_seq_reduce_with_range                   = */ (DeeMH_seq_reduce_with_range_t)&super_mh__seq_reduce_with_range,
	/* .mh_seq_reduce_with_range_and_init          = */ (DeeMH_seq_reduce_with_range_and_init_t)&super_mh__seq_reduce_with_range_and_init,
	/* .mh_seq_min                                 = */ (DeeMH_seq_min_t)&super_mh__seq_min,
	/* .mh_seq_min_with_key                        = */ (DeeMH_seq_min_with_key_t)&super_mh__seq_min_with_key,
	/* .mh_seq_min_with_range                      = */ (DeeMH_seq_min_with_range_t)&super_mh__seq_min_with_range,
	/* .mh_seq_min_with_range_and_key              = */ (DeeMH_seq_min_with_range_and_key_t)&super_mh__seq_min_with_range_and_key,
	/* .mh_seq_max                                 = */ (DeeMH_seq_max_t)&super_mh__seq_max,
	/* .mh_seq_max_with_key                        = */ (DeeMH_seq_max_with_key_t)&super_mh__seq_max_with_key,
	/* .mh_seq_max_with_range                      = */ (DeeMH_seq_max_with_range_t)&super_mh__seq_max_with_range,
	/* .mh_seq_max_with_range_and_key              = */ (DeeMH_seq_max_with_range_and_key_t)&super_mh__seq_max_with_range_and_key,
	/* .mh_seq_sum                                 = */ (DeeMH_seq_sum_t)&super_mh__seq_sum,
	/* .mh_seq_sum_with_range                      = */ (DeeMH_seq_sum_with_range_t)&super_mh__seq_sum_with_range,
	/* .mh_seq_count                               = */ (DeeMH_seq_count_t)&super_mh__seq_count,
	/* .mh_seq_count_with_key                      = */ (DeeMH_seq_count_with_key_t)&super_mh__seq_count_with_key,
	/* .mh_seq_count_with_range                    = */ (DeeMH_seq_count_with_range_t)&super_mh__seq_count_with_range,
	/* .mh_seq_count_with_range_and_key            = */ (DeeMH_seq_count_with_range_and_key_t)&super_mh__seq_count_with_range_and_key,
	/* .mh_seq_contains                            = */ (DeeMH_seq_contains_t)&super_mh__seq_contains,
	/* .mh_seq_contains_with_key                   = */ (DeeMH_seq_contains_with_key_t)&super_mh__seq_contains_with_key,
	/* .mh_seq_contains_with_range                 = */ (DeeMH_seq_contains_with_range_t)&super_mh__seq_contains_with_range,
	/* .mh_seq_contains_with_range_and_key         = */ (DeeMH_seq_contains_with_range_and_key_t)&super_mh__seq_contains_with_range_and_key,
	/* .mh_seq_operator_contains                   = */ (DeeMH_seq_operator_contains_t)&super_mh__seq_operator_contains,
	/* .mh_seq_locate                              = */ (DeeMH_seq_locate_t)&super_mh__seq_locate,
	/* .mh_seq_locate_with_range                   = */ (DeeMH_seq_locate_with_range_t)&super_mh__seq_locate_with_range,
	/* .mh_seq_rlocate                             = */ (DeeMH_seq_rlocate_t)&super_mh__seq_rlocate,
	/* .mh_seq_rlocate_with_range                  = */ (DeeMH_seq_rlocate_with_range_t)&super_mh__seq_rlocate_with_range,
	/* .mh_seq_startswith                          = */ (DeeMH_seq_startswith_t)&super_mh__seq_startswith,
	/* .mh_seq_startswith_with_key                 = */ (DeeMH_seq_startswith_with_key_t)&super_mh__seq_startswith_with_key,
	/* .mh_seq_startswith_with_range               = */ (DeeMH_seq_startswith_with_range_t)&super_mh__seq_startswith_with_range,
	/* .mh_seq_startswith_with_range_and_key       = */ (DeeMH_seq_startswith_with_range_and_key_t)&super_mh__seq_startswith_with_range_and_key,
	/* .mh_seq_endswith                            = */ (DeeMH_seq_endswith_t)&super_mh__seq_endswith,
	/* .mh_seq_endswith_with_key                   = */ (DeeMH_seq_endswith_with_key_t)&super_mh__seq_endswith_with_key,
	/* .mh_seq_endswith_with_range                 = */ (DeeMH_seq_endswith_with_range_t)&super_mh__seq_endswith_with_range,
	/* .mh_seq_endswith_with_range_and_key         = */ (DeeMH_seq_endswith_with_range_and_key_t)&super_mh__seq_endswith_with_range_and_key,
	/* .mh_seq_find                                = */ (DeeMH_seq_find_t)&super_mh__seq_find,
	/* .mh_seq_find_with_key                       = */ (DeeMH_seq_find_with_key_t)&super_mh__seq_find_with_key,
	/* .mh_seq_rfind                               = */ (DeeMH_seq_rfind_t)&super_mh__seq_rfind,
	/* .mh_seq_rfind_with_key                      = */ (DeeMH_seq_rfind_with_key_t)&super_mh__seq_rfind_with_key,
	/* .mh_seq_erase                               = */ (DeeMH_seq_erase_t)&super_mh__seq_erase,
	/* .mh_seq_insert                              = */ (DeeMH_seq_insert_t)&super_mh__seq_insert,
	/* .mh_seq_insertall                           = */ (DeeMH_seq_insertall_t)&super_mh__seq_insertall,
	/* .mh_seq_pushfront                           = */ (DeeMH_seq_pushfront_t)&super_mh__seq_pushfront,
	/* .mh_seq_append                              = */ (DeeMH_seq_append_t)&super_mh__seq_append,
	/* .mh_seq_extend                              = */ (DeeMH_seq_extend_t)&super_mh__seq_extend,
	/* .mh_seq_xchitem_index                       = */ (DeeMH_seq_xchitem_index_t)&super_mh__seq_xchitem_index,
	/* .mh_seq_clear                               = */ (DeeMH_seq_clear_t)&super_mh__seq_clear,
	/* .mh_seq_pop                                 = */ (DeeMH_seq_pop_t)&super_mh__seq_pop,
	/* .mh_seq_remove                              = */ (DeeMH_seq_remove_t)&super_mh__seq_remove,
	/* .mh_seq_remove_with_key                     = */ (DeeMH_seq_remove_with_key_t)&super_mh__seq_remove_with_key,
	/* .mh_seq_rremove                             = */ (DeeMH_seq_rremove_t)&super_mh__seq_rremove,
	/* .mh_seq_rremove_with_key                    = */ (DeeMH_seq_rremove_with_key_t)&super_mh__seq_rremove_with_key,
	/* .mh_seq_removeall                           = */ (DeeMH_seq_removeall_t)&super_mh__seq_removeall,
	/* .mh_seq_removeall_with_key                  = */ (DeeMH_seq_removeall_with_key_t)&super_mh__seq_removeall_with_key,
	/* .mh_seq_removeif                            = */ (DeeMH_seq_removeif_t)&super_mh__seq_removeif,
	/* .mh_seq_resize                              = */ (DeeMH_seq_resize_t)&super_mh__seq_resize,
	/* .mh_seq_fill                                = */ (DeeMH_seq_fill_t)&super_mh__seq_fill,
	/* .mh_seq_reverse                             = */ (DeeMH_seq_reverse_t)&super_mh__seq_reverse,
	/* .mh_seq_reversed                            = */ (DeeMH_seq_reversed_t)&super_mh__seq_reversed,
	/* .mh_seq_sort                                = */ (DeeMH_seq_sort_t)&super_mh__seq_sort,
	/* .mh_seq_sort_with_key                       = */ (DeeMH_seq_sort_with_key_t)&super_mh__seq_sort_with_key,
	/* .mh_seq_sorted                              = */ (DeeMH_seq_sorted_t)&super_mh__seq_sorted,
	/* .mh_seq_sorted_with_key                     = */ (DeeMH_seq_sorted_with_key_t)&super_mh__seq_sorted_with_key,
	/* .mh_seq_bfind                               = */ (DeeMH_seq_bfind_t)&super_mh__seq_bfind,
	/* .mh_seq_bfind_with_key                      = */ (DeeMH_seq_bfind_with_key_t)&super_mh__seq_bfind_with_key,
	/* .mh_seq_bposition                           = */ (DeeMH_seq_bposition_t)&super_mh__seq_bposition,
	/* .mh_seq_bposition_with_key                  = */ (DeeMH_seq_bposition_with_key_t)&super_mh__seq_bposition_with_key,
	/* .mh_seq_brange                              = */ (DeeMH_seq_brange_t)&super_mh__seq_brange,
	/* .mh_seq_brange_with_key                     = */ (DeeMH_seq_brange_with_key_t)&super_mh__seq_brange_with_key,
	/* .mh_set_operator_iter                       = */ (DeeMH_set_operator_iter_t)&super_mh__set_operator_iter,
	/* .mh_set_operator_foreach                    = */ (DeeMH_set_operator_foreach_t)&super_mh__set_operator_foreach,
	/* .mh_set_operator_sizeob                     = */ (DeeMH_set_operator_sizeob_t)&super_mh__set_operator_sizeob,
	/* .mh_set_operator_size                       = */ (DeeMH_set_operator_size_t)&super_mh__set_operator_size,
	/* .mh_set_operator_hash                       = */ (DeeMH_set_operator_hash_t)&super_mh__set_operator_hash,
	/* .mh_set_operator_compare_eq                 = */ (DeeMH_set_operator_compare_eq_t)&super_mh__set_operator_compare_eq,
	/* .mh_set_operator_trycompare_eq              = */ (DeeMH_set_operator_trycompare_eq_t)&super_mh__set_operator_trycompare_eq,
	/* .mh_set_operator_eq                         = */ (DeeMH_set_operator_eq_t)&super_mh__set_operator_eq,
	/* .mh_set_operator_ne                         = */ (DeeMH_set_operator_ne_t)&super_mh__set_operator_ne,
	/* .mh_set_operator_lo                         = */ (DeeMH_set_operator_lo_t)&super_mh__set_operator_lo,
	/* .mh_set_operator_le                         = */ (DeeMH_set_operator_le_t)&super_mh__set_operator_le,
	/* .mh_set_operator_gr                         = */ (DeeMH_set_operator_gr_t)&super_mh__set_operator_gr,
	/* .mh_set_operator_ge                         = */ (DeeMH_set_operator_ge_t)&super_mh__set_operator_ge,
	/* .mh_set_operator_bool                       = */ (DeeMH_set_operator_bool_t)&super_mh__set_operator_bool,
	/* .mh_set_operator_inv                        = */ (DeeMH_set_operator_inv_t)&super_mh__set_operator_inv,
	/* .mh_set_operator_add                        = */ (DeeMH_set_operator_add_t)&super_mh__set_operator_add,
	/* .mh_set_operator_sub                        = */ (DeeMH_set_operator_sub_t)&super_mh__set_operator_sub,
	/* .mh_set_operator_and                        = */ (DeeMH_set_operator_and_t)&super_mh__set_operator_and,
	/* .mh_set_operator_xor                        = */ (DeeMH_set_operator_xor_t)&super_mh__set_operator_xor,
	/* .mh_set_operator_inplace_add                = */ (DeeMH_set_operator_inplace_add_t)&super_mh__set_operator_inplace_add,
	/* .mh_set_operator_inplace_sub                = */ (DeeMH_set_operator_inplace_sub_t)&super_mh__set_operator_inplace_sub,
	/* .mh_set_operator_inplace_and                = */ (DeeMH_set_operator_inplace_and_t)&super_mh__set_operator_inplace_and,
	/* .mh_set_operator_inplace_xor                = */ (DeeMH_set_operator_inplace_xor_t)&super_mh__set_operator_inplace_xor,
	/* .mh_set_frozen                              = */ (DeeMH_set_frozen_t)&super_mh__set_frozen,
	/* .mh_set_unify                               = */ (DeeMH_set_unify_t)&super_mh__set_unify,
	/* .mh_set_insert                              = */ (DeeMH_set_insert_t)&super_mh__set_insert,
	/* .mh_set_insertall                           = */ (DeeMH_set_insertall_t)&super_mh__set_insertall,
	/* .mh_set_remove                              = */ (DeeMH_set_remove_t)&super_mh__set_remove,
	/* .mh_set_removeall                           = */ (DeeMH_set_removeall_t)&super_mh__set_removeall,
	/* .mh_set_pop                                 = */ (DeeMH_set_pop_t)&super_mh__set_pop,
	/* .mh_set_pop_with_default                    = */ (DeeMH_set_pop_with_default_t)&super_mh__set_pop_with_default,
	/* .mh_set_trygetfirst                         = */ (DeeMH_set_trygetfirst_t)&super_mh__set_trygetfirst,
	/* .mh_set_getfirst                            = */ (DeeMH_set_getfirst_t)&super_mh__set_getfirst,
	/* .mh_set_boundfirst                          = */ (DeeMH_set_boundfirst_t)&super_mh__set_boundfirst,
	/* .mh_set_delfirst                            = */ (DeeMH_set_delfirst_t)&super_mh__set_delfirst,
	/* .mh_set_setfirst                            = */ (DeeMH_set_setfirst_t)&super_mh__set_setfirst,
	/* .mh_set_trygetlast                          = */ (DeeMH_set_trygetlast_t)&super_mh__set_trygetlast,
	/* .mh_set_getlast                             = */ (DeeMH_set_getlast_t)&super_mh__set_getlast,
	/* .mh_set_boundlast                           = */ (DeeMH_set_boundlast_t)&super_mh__set_boundlast,
	/* .mh_set_dellast                             = */ (DeeMH_set_dellast_t)&super_mh__set_dellast,
	/* .mh_set_setlast                             = */ (DeeMH_set_setlast_t)&super_mh__set_setlast,
	/* .mh_map_operator_iter                       = */ (DeeMH_map_operator_iter_t)&super_mh__map_operator_iter,
	/* .mh_map_operator_foreach_pair               = */ (DeeMH_map_operator_foreach_pair_t)&super_mh__map_operator_foreach_pair,
	/* .mh_map_operator_sizeob                     = */ (DeeMH_map_operator_sizeob_t)&super_mh__map_operator_sizeob,
	/* .mh_map_operator_size                       = */ (DeeMH_map_operator_size_t)&super_mh__map_operator_size,
	/* .mh_map_operator_hash                       = */ (DeeMH_map_operator_hash_t)&super_mh__map_operator_hash,
	/* .mh_map_operator_getitem                    = */ (DeeMH_map_operator_getitem_t)&super_mh__map_operator_getitem,
	/* .mh_map_operator_trygetitem                 = */ (DeeMH_map_operator_trygetitem_t)&super_mh__map_operator_trygetitem,
	/* .mh_map_operator_getitem_index              = */ (DeeMH_map_operator_getitem_index_t)&super_mh__map_operator_getitem_index,
	/* .mh_map_operator_trygetitem_index           = */ (DeeMH_map_operator_trygetitem_index_t)&super_mh__map_operator_trygetitem_index,
	/* .mh_map_operator_getitem_string_hash        = */ (DeeMH_map_operator_getitem_string_hash_t)&super_mh__map_operator_getitem_string_hash,
	/* .mh_map_operator_trygetitem_string_hash     = */ (DeeMH_map_operator_trygetitem_string_hash_t)&super_mh__map_operator_trygetitem_string_hash,
	/* .mh_map_operator_getitem_string_len_hash    = */ (DeeMH_map_operator_getitem_string_len_hash_t)&super_mh__map_operator_getitem_string_len_hash,
	/* .mh_map_operator_trygetitem_string_len_hash = */ (DeeMH_map_operator_trygetitem_string_len_hash_t)&super_mh__map_operator_trygetitem_string_len_hash,
	/* .mh_map_operator_bounditem                  = */ (DeeMH_map_operator_bounditem_t)&super_mh__map_operator_bounditem,
	/* .mh_map_operator_bounditem_index            = */ (DeeMH_map_operator_bounditem_index_t)&super_mh__map_operator_bounditem_index,
	/* .mh_map_operator_bounditem_string_hash      = */ (DeeMH_map_operator_bounditem_string_hash_t)&super_mh__map_operator_bounditem_string_hash,
	/* .mh_map_operator_bounditem_string_len_hash  = */ (DeeMH_map_operator_bounditem_string_len_hash_t)&super_mh__map_operator_bounditem_string_len_hash,
	/* .mh_map_operator_hasitem                    = */ (DeeMH_map_operator_hasitem_t)&super_mh__map_operator_hasitem,
	/* .mh_map_operator_hasitem_index              = */ (DeeMH_map_operator_hasitem_index_t)&super_mh__map_operator_hasitem_index,
	/* .mh_map_operator_hasitem_string_hash        = */ (DeeMH_map_operator_hasitem_string_hash_t)&super_mh__map_operator_hasitem_string_hash,
	/* .mh_map_operator_hasitem_string_len_hash    = */ (DeeMH_map_operator_hasitem_string_len_hash_t)&super_mh__map_operator_hasitem_string_len_hash,
	/* .mh_map_operator_delitem                    = */ (DeeMH_map_operator_delitem_t)&super_mh__map_operator_delitem,
	/* .mh_map_operator_delitem_index              = */ (DeeMH_map_operator_delitem_index_t)&super_mh__map_operator_delitem_index,
	/* .mh_map_operator_delitem_string_hash        = */ (DeeMH_map_operator_delitem_string_hash_t)&super_mh__map_operator_delitem_string_hash,
	/* .mh_map_operator_delitem_string_len_hash    = */ (DeeMH_map_operator_delitem_string_len_hash_t)&super_mh__map_operator_delitem_string_len_hash,
	/* .mh_map_operator_setitem                    = */ (DeeMH_map_operator_setitem_t)&super_mh__map_operator_setitem,
	/* .mh_map_operator_setitem_index              = */ (DeeMH_map_operator_setitem_index_t)&super_mh__map_operator_setitem_index,
	/* .mh_map_operator_setitem_string_hash        = */ (DeeMH_map_operator_setitem_string_hash_t)&super_mh__map_operator_setitem_string_hash,
	/* .mh_map_operator_setitem_string_len_hash    = */ (DeeMH_map_operator_setitem_string_len_hash_t)&super_mh__map_operator_setitem_string_len_hash,
	/* .mh_map_operator_contains                   = */ (DeeMH_map_operator_contains_t)&super_mh__map_operator_contains,
	/* .mh_map_keys                                = */ (DeeMH_map_keys_t)&super_mh__map_keys,
	/* .mh_map_iterkeys                            = */ (DeeMH_map_iterkeys_t)&super_mh__map_iterkeys,
	/* .mh_map_values                              = */ (DeeMH_map_values_t)&super_mh__map_values,
	/* .mh_map_itervalues                          = */ (DeeMH_map_itervalues_t)&super_mh__map_itervalues,
	/* .mh_map_enumerate                           = */ (DeeMH_map_enumerate_t)&super_mh__map_enumerate,
	/* .mh_map_enumerate_range                     = */ (DeeMH_map_enumerate_range_t)&super_mh__map_enumerate_range,
	/* .mh_map_makeenumeration                     = */ (DeeMH_map_makeenumeration_t)&super_mh__map_makeenumeration,
	/* .mh_map_makeenumeration_with_range          = */ (DeeMH_map_makeenumeration_with_range_t)&super_mh__map_makeenumeration_with_range,
	/* .mh_map_operator_compare_eq                 = */ (DeeMH_map_operator_compare_eq_t)&super_mh__map_operator_compare_eq,
	/* .mh_map_operator_trycompare_eq              = */ (DeeMH_map_operator_trycompare_eq_t)&super_mh__map_operator_trycompare_eq,
	/* .mh_map_operator_eq                         = */ (DeeMH_map_operator_eq_t)&super_mh__map_operator_eq,
	/* .mh_map_operator_ne                         = */ (DeeMH_map_operator_ne_t)&super_mh__map_operator_ne,
	/* .mh_map_operator_lo                         = */ (DeeMH_map_operator_lo_t)&super_mh__map_operator_lo,
	/* .mh_map_operator_le                         = */ (DeeMH_map_operator_le_t)&super_mh__map_operator_le,
	/* .mh_map_operator_gr                         = */ (DeeMH_map_operator_gr_t)&super_mh__map_operator_gr,
	/* .mh_map_operator_ge                         = */ (DeeMH_map_operator_ge_t)&super_mh__map_operator_ge,
	/* .mh_map_operator_add                        = */ (DeeMH_map_operator_add_t)&super_mh__map_operator_add,
	/* .mh_map_operator_sub                        = */ (DeeMH_map_operator_sub_t)&super_mh__map_operator_sub,
	/* .mh_map_operator_and                        = */ (DeeMH_map_operator_and_t)&super_mh__map_operator_and,
	/* .mh_map_operator_xor                        = */ (DeeMH_map_operator_xor_t)&super_mh__map_operator_xor,
	/* .mh_map_operator_inplace_add                = */ (DeeMH_map_operator_inplace_add_t)&super_mh__map_operator_inplace_add,
	/* .mh_map_operator_inplace_sub                = */ (DeeMH_map_operator_inplace_sub_t)&super_mh__map_operator_inplace_sub,
	/* .mh_map_operator_inplace_and                = */ (DeeMH_map_operator_inplace_and_t)&super_mh__map_operator_inplace_and,
	/* .mh_map_operator_inplace_xor                = */ (DeeMH_map_operator_inplace_xor_t)&super_mh__map_operator_inplace_xor,
	/* .mh_map_frozen                              = */ (DeeMH_map_frozen_t)&super_mh__map_frozen,
	/* .mh_map_setold                              = */ (DeeMH_map_setold_t)&super_mh__map_setold,
	/* .mh_map_setold_ex                           = */ (DeeMH_map_setold_ex_t)&super_mh__map_setold_ex,
	/* .mh_map_setnew                              = */ (DeeMH_map_setnew_t)&super_mh__map_setnew,
	/* .mh_map_setnew_ex                           = */ (DeeMH_map_setnew_ex_t)&super_mh__map_setnew_ex,
	/* .mh_map_setdefault                          = */ (DeeMH_map_setdefault_t)&super_mh__map_setdefault,
	/* .mh_map_update                              = */ (DeeMH_map_update_t)&super_mh__map_update,
	/* .mh_map_remove                              = */ (DeeMH_map_remove_t)&super_mh__map_remove,
	/* .mh_map_removekeys                          = */ (DeeMH_map_removekeys_t)&super_mh__map_removekeys,
	/* .mh_map_pop                                 = */ (DeeMH_map_pop_t)&super_mh__map_pop,
	/* .mh_map_pop_with_default                    = */ (DeeMH_map_pop_with_default_t)&super_mh__map_pop_with_default,
	/* .mh_map_popitem                             = */ (DeeMH_map_popitem_t)&super_mh__map_popitem,
/*[[[end]]]*/
/* clang-format on */
};

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_METHOD_HINT_SUPER_C */
