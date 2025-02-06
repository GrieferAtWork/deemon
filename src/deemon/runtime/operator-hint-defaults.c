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
#ifndef GUARD_DEEMON_RUNTIME_OPERATOR_HINT_DEFAULTS_C
#define GUARD_DEEMON_RUNTIME_OPERATOR_HINT_DEFAULTS_C 1

#include <deemon/api.h>
#if defined(CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS) || defined(__DEEMON__)
#include <deemon/class.h>
#include <deemon/operator-hints.h>

DECL_BEGIN

/* clang-format off */
/*[[[deemon (printNativeOperatorHintImpls from "..method-hints.method-hints")();]]]*/
/* tp_seq->tp_iter */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
usrtype__seq_iter(DeeObject *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, self, OPERATOR_ITER, 0, NULL);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *func, *result;
	func = DeeClass_GetOperator(THIS_TYPE, OPERATOR_ITER);
	if unlikely(!func)
		goto err;
	result = DeeObject_ThisCall(func, self, 0, NULL);
	Dee_Decref_unlikely(func);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_iter__with__seq_foreach(DeeObject *__restrict self) {
	/* TODO: Custom iterator type that uses "tp_foreach" */
	(void)THIS_TYPE;
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default__seq_iter__with__seq_foreach_pair(DeeObject *__restrict self) {
	/* TODO: Custom iterator type that uses "tp_foreach_pair" */
	(void)THIS_TYPE;
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__seq_iter(DeeTypeObject *tp_self, DeeObject *self) {
	return (*tp_self->)();
}
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tusrtype__seq_iter(DeeTypeObject *tp_self, DeeObject *self) {
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, self, OPERATOR_ITER, 0, NULL);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *func, *result;
	func = DeeClass_GetOperator(THIS_TYPE, OPERATOR_ITER);
	if unlikely(!func)
		goto err;
	result = DeeObject_ThisCall(func, self, 0, NULL);
	Dee_Decref_unlikely(func);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__seq_iter__with__seq_foreach(DeeTypeObject *tp_self, DeeObject *self) {
	/* TODO: Custom iterator type that uses "tp_foreach" */
	(void)THIS_TYPE;
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tdefault__seq_iter__with__seq_foreach_pair(DeeTypeObject *tp_self, DeeObject *self) {
	/* TODO: Custom iterator type that uses "tp_foreach_pair" */
	(void)THIS_TYPE;
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

/* tp_seq->tp_foreach */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_foreach__with__seq_iter(DeeObject *__restrict self, Dee_foreach_t cb, void *arg) {
	Dee_ssize_t result;
	DREF DeeObject *iter = CALL_DEPENDENCY(tp_seq->tp_iter, self);
	if unlikely(!iter)
		goto err;
	result = DeeIterator_Foreach(iter, cb, arg);
	Dee_Decref_likely(iter);
	return result;
err:
	return -1;
}
#ifndef DEFINED_default_foreach_with_foreach_pair_cb
#define DEFINED_default_foreach_with_foreach_pair_cb
struct default_foreach_with_foreach_pair_data {
	Dee_foreach_t dfwfp_cb;  /* [1..1] Underlying callback. */
	void         *dfwfp_arg; /* Cookie for `dfwfp_cb' */
};

PRIVATE WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
default_foreach_with_foreach_pair_cb(void *arg, DeeObject *key, DeeObject *value) {
	struct default_foreach_with_foreach_pair_data *data;
	Dee_ssize_t result;
	DREF DeeTupleObject *pair;
	data = (struct default_foreach_with_foreach_pair_data *)arg;
	pair = DeeTuple_NewUninitializedPair();
	if unlikely(!pair)
		goto err;
	pair->t_elem[0] = key;   /* Symbolic reference */
	pair->t_elem[1] = value; /* Symbolic reference */
	result = (*data->dfwfp_cb)(data->dfwfp_arg, (DeeObject *)pair);
	DeeTuple_DecrefSymbolic((DREF DeeObject *)pair);
	return result;
err:
	return -1;
}
#endif /* !DEFINED_default_foreach_with_foreach_pair_cb */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_foreach__with__seq_foreach_pair(DeeObject *__restrict self, Dee_foreach_t cb, void *arg) {
	struct default_foreach_with_foreach_pair_data data;
	data.dfwfp_cb  = cb;
	data.dfwfp_arg = arg;
	return CALL_DEPENDENCY(tp_seq->tp_foreach_pair, self, &default_foreach_with_foreach_pair_cb, &data);
}
INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
tdefault__seq_foreach(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t cb, void *arg) {
	return (*tp_self->)();
}
INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
tdefault__seq_foreach__with__seq_iter(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t cb, void *arg) {
	Dee_ssize_t result;
	DREF DeeObject *iter = CALL_DEPENDENCY(tp_seq->tp_iter, self);
	if unlikely(!iter)
		goto err;
	result = DeeIterator_Foreach(iter, cb, arg);
	Dee_Decref_likely(iter);
	return result;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
tdefault__seq_foreach__with__seq_foreach_pair(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t cb, void *arg) {
	struct default_foreach_with_foreach_pair_data data;
	data.dfwfp_cb  = cb;
	data.dfwfp_arg = arg;
	return CALL_DEPENDENCY(tp_seq->tp_foreach_pair, self, &default_foreach_with_foreach_pair_cb, &data);
}

/* tp_seq->tp_foreach_pair */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_foreach_pair__with__seq_iter(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg) {
	Dee_ssize_t result;
	DREF DeeObject *iter = CALL_DEPENDENCY(tp_seq->tp_iter, self);
	if unlikely(!iter)
		goto err;
	result = DeeIterator_ForeachPair(iter, cb, arg);
	Dee_Decref_likely(iter);
	return result;
err:
	return -1;
}
#ifndef DEFINED_default_foreach_pair_with_foreach_cb
#define DEFINED_default_foreach_pair_with_foreach_cb
struct default_foreach_pair_with_foreach_data {
	Dee_foreach_pair_t dfpwf_cb;  /* [1..1] Underlying callback. */
	void              *dfpwf_arg; /* Cookie for `dfpwf_cb' */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_foreach_pair_with_foreach_cb(void *arg, DeeObject *elem) {
	struct default_foreach_pair_with_foreach_data *data;
	Dee_ssize_t result;
	data = (struct default_foreach_pair_with_foreach_data *)arg;
	if likely(DeeTuple_Check(elem) && DeeTuple_SIZE(elem) == 2) {
		result = (*data->dfpwf_cb)(data->dfpwf_arg,
		                             DeeTuple_GET(elem, 0),
		                             DeeTuple_GET(elem, 1));
	} else {
		DREF DeeObject *pair[2];
		if unlikely(DeeObject_Unpack(elem, 2, pair))
			goto err;
		result = (*data->dfpwf_cb)(data->dfpwf_arg, pair[0], pair[1]);
		Dee_Decref_unlikely(pair[1]);
		Dee_Decref_unlikely(pair[0]);
	}
	return result;
err:
	return -1;
}
#endif /* !DEFINED_default_foreach_pair_with_foreach_cb */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default__seq_foreach_pair__with__seq_foreach(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg) {
	struct default_foreach_pair_with_foreach_data data;
	data.dfpwf_cb  = cb;
	data.dfpwf_arg = arg;
	return CALL_DEPENDENCY(tp_seq->tp_foreach, self, &default_foreach_pair_with_foreach_cb, &data);
}
INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
tdefault__seq_foreach_pair(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_pair_t cb, void *arg) {
	return (*tp_self->)();
}
INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
tdefault__seq_foreach_pair__with__seq_iter(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_pair_t cb, void *arg) {
	Dee_ssize_t result;
	DREF DeeObject *iter = CALL_DEPENDENCY(tp_seq->tp_iter, self);
	if unlikely(!iter)
		goto err;
	result = DeeIterator_ForeachPair(iter, cb, arg);
	Dee_Decref_likely(iter);
	return result;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
tdefault__seq_foreach_pair__with__seq_foreach(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_pair_t cb, void *arg) {
	struct default_foreach_pair_with_foreach_data data;
	data.dfpwf_cb  = cb;
	data.dfpwf_arg = arg;
	return CALL_DEPENDENCY(tp_seq->tp_foreach, self, &default_foreach_pair_with_foreach_cb, &data);
}

/* tp_math->tp_add */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
usrtype__math_add(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_ADD, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb, *result;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_ADD);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__math_add(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return (*tp_self->)();
}
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tusrtype__math_add(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_ADD, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb, *result;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_ADD);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}

/* tp_math->tp_inplace_add */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
usrtype__math_inplace_add(DeeObject **__restrict p_lhs, DeeObject *rhs) {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_lhs, OPERATOR_INPLACE_ADD, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INPLACE_ADD);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__math_inplace_add__with__math_add(DeeObject **__restrict p_lhs, DeeObject *rhs) {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_add, *p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__math_inplace_add(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	return (*tp_self->)();
}
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tusrtype__math_inplace_add(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_lhs, OPERATOR_INPLACE_ADD, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INPLACE_ADD);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__math_inplace_add__with__math_add(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_add, *p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

/* tp_math->tp_sub */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
usrtype__math_sub(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_SUB, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb, *result;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_SUB);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__math_sub(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return (*tp_self->)();
}
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tusrtype__math_sub(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_SUB, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb, *result;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_SUB);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}

/* tp_math->tp_inplace_sub */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
usrtype__math_inplace_sub(DeeObject **__restrict p_lhs, DeeObject *rhs) {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_lhs, OPERATOR_INPLACE_SUB, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INPLACE_SUB);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__math_inplace_sub__with__math_sub(DeeObject **__restrict p_lhs, DeeObject *rhs) {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_sub, *p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__math_inplace_sub(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	return (*tp_self->)();
}
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tusrtype__math_inplace_sub(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_lhs, OPERATOR_INPLACE_SUB, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INPLACE_SUB);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__math_inplace_sub__with__math_sub(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_sub, *p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

/* tp_math->tp_mul */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
usrtype__math_mul(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_MUL, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb, *result;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_MUL);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__math_mul(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return (*tp_self->)();
}
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tusrtype__math_mul(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_MUL, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb, *result;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_MUL);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}

/* tp_math->tp_inplace_mul */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
usrtype__math_inplace_mul(DeeObject **__restrict p_lhs, DeeObject *rhs) {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_lhs, OPERATOR_INPLACE_MUL, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INPLACE_MUL);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__math_inplace_mul__with__math_mul(DeeObject **__restrict p_lhs, DeeObject *rhs) {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_mul, *p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__math_inplace_mul(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	return (*tp_self->)();
}
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tusrtype__math_inplace_mul(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_lhs, OPERATOR_INPLACE_MUL, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INPLACE_MUL);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__math_inplace_mul__with__math_mul(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_mul, *p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

/* tp_math->tp_div */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
usrtype__math_div(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_DIV, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb, *result;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_DIV);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__math_div(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return (*tp_self->)();
}
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tusrtype__math_div(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_DIV, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb, *result;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_DIV);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}

/* tp_math->tp_inplace_div */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
usrtype__math_inplace_div(DeeObject **__restrict p_lhs, DeeObject *rhs) {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_lhs, OPERATOR_INPLACE_DIV, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INPLACE_DIV);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__math_inplace_div__with__math_div(DeeObject **__restrict p_lhs, DeeObject *rhs) {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_div, *p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__math_inplace_div(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	return (*tp_self->)();
}
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tusrtype__math_inplace_div(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_lhs, OPERATOR_INPLACE_DIV, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INPLACE_DIV);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__math_inplace_div__with__math_div(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_div, *p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

/* tp_math->tp_mod */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
usrtype__math_mod(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_MOD, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb, *result;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_MOD);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__math_mod(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return (*tp_self->)();
}
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tusrtype__math_mod(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_MOD, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb, *result;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_MOD);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}

/* tp_math->tp_inplace_mod */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
usrtype__math_inplace_mod(DeeObject **__restrict p_lhs, DeeObject *rhs) {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_lhs, OPERATOR_INPLACE_MOD, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INPLACE_MOD);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__math_inplace_mod__with__math_mod(DeeObject **__restrict p_lhs, DeeObject *rhs) {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_mod, *p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__math_inplace_mod(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	return (*tp_self->)();
}
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tusrtype__math_inplace_mod(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_lhs, OPERATOR_INPLACE_MOD, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INPLACE_MOD);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__math_inplace_mod__with__math_mod(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_mod, *p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

/* tp_math->tp_shl */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
usrtype__math_shl(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_SHL, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb, *result;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_SHL);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__math_shl(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return (*tp_self->)();
}
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tusrtype__math_shl(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_SHL, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb, *result;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_SHL);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}

/* tp_math->tp_inplace_shl */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
usrtype__math_inplace_shl(DeeObject **__restrict p_lhs, DeeObject *rhs) {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_lhs, OPERATOR_INPLACE_SHL, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INPLACE_SHL);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__math_inplace_shl__with__math_shl(DeeObject **__restrict p_lhs, DeeObject *rhs) {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_shl, *p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__math_inplace_shl(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	return (*tp_self->)();
}
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tusrtype__math_inplace_shl(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_lhs, OPERATOR_INPLACE_SHL, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INPLACE_SHL);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__math_inplace_shl__with__math_shl(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_shl, *p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

/* tp_math->tp_shr */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
usrtype__math_shr(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_SHR, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb, *result;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_SHR);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__math_shr(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return (*tp_self->)();
}
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tusrtype__math_shr(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_SHR, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb, *result;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_SHR);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}

/* tp_math->tp_inplace_shr */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
usrtype__math_inplace_shr(DeeObject **__restrict p_lhs, DeeObject *rhs) {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_lhs, OPERATOR_INPLACE_SHR, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INPLACE_SHR);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__math_inplace_shr__with__math_shr(DeeObject **__restrict p_lhs, DeeObject *rhs) {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_shr, *p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__math_inplace_shr(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	return (*tp_self->)();
}
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tusrtype__math_inplace_shr(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_lhs, OPERATOR_INPLACE_SHR, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INPLACE_SHR);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__math_inplace_shr__with__math_shr(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_shr, *p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

/* tp_math->tp_and */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
usrtype__math_and(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_AND, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb, *result;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_AND);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__math_and(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return (*tp_self->)();
}
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tusrtype__math_and(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_AND, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb, *result;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_AND);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}

/* tp_math->tp_inplace_and */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
usrtype__math_inplace_and(DeeObject **__restrict p_lhs, DeeObject *rhs) {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_lhs, OPERATOR_INPLACE_AND, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INPLACE_AND);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__math_inplace_and__with__math_and(DeeObject **__restrict p_lhs, DeeObject *rhs) {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_and, *p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__math_inplace_and(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	return (*tp_self->)();
}
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tusrtype__math_inplace_and(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_lhs, OPERATOR_INPLACE_AND, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INPLACE_AND);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__math_inplace_and__with__math_and(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_and, *p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

/* tp_math->tp_or */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
usrtype__math_or(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_OR, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb, *result;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_OR);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__math_or(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return (*tp_self->)();
}
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tusrtype__math_or(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_OR, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb, *result;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_OR);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}

/* tp_math->tp_inplace_or */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
usrtype__math_inplace_or(DeeObject **__restrict p_lhs, DeeObject *rhs) {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_lhs, OPERATOR_INPLACE_OR, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INPLACE_OR);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__math_inplace_or__with__math_or(DeeObject **__restrict p_lhs, DeeObject *rhs) {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_or, *p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__math_inplace_or(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	return (*tp_self->)();
}
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tusrtype__math_inplace_or(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_lhs, OPERATOR_INPLACE_OR, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INPLACE_OR);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__math_inplace_or__with__math_or(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_or, *p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

/* tp_math->tp_xor */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
usrtype__math_xor(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_XOR, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb, *result;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_XOR);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__math_xor(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return (*tp_self->)();
}
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tusrtype__math_xor(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_XOR, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb, *result;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_XOR);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}

/* tp_math->tp_inplace_xor */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
usrtype__math_inplace_xor(DeeObject **__restrict p_lhs, DeeObject *rhs) {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_lhs, OPERATOR_INPLACE_XOR, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INPLACE_XOR);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__math_inplace_xor__with__math_xor(DeeObject **__restrict p_lhs, DeeObject *rhs) {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_xor, *p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__math_inplace_xor(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	return (*tp_self->)();
}
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tusrtype__math_inplace_xor(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_lhs, OPERATOR_INPLACE_XOR, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INPLACE_XOR);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__math_inplace_xor__with__math_xor(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_xor, *p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

/* tp_math->tp_pow */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
usrtype__math_pow(DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_POW, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb, *result;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_POW);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tdefault__math_pow(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	return (*tp_self->)();
}
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
tusrtype__math_pow(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_POW, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb, *result;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_POW);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}

/* tp_math->tp_inplace_pow */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
usrtype__math_inplace_pow(DeeObject **__restrict p_lhs, DeeObject *rhs) {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_lhs, OPERATOR_INPLACE_POW, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INPLACE_POW);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2)) int DCALL
default__math_inplace_pow__with__math_pow(DeeObject **__restrict p_lhs, DeeObject *rhs) {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_pow, *p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__math_inplace_pow(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	return (*tp_self->)();
}
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tusrtype__math_inplace_pow(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_lhs, OPERATOR_INPLACE_POW, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INPLACE_POW);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
tdefault__math_inplace_pow__with__math_pow(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs) {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_pow, *p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

/* tp_math->tp_inc */
INTERN WUNUSED NONNULL((1)) int DCALL
usrtype__math_inc(DeeObject **__restrict p_self) {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_self, OPERATOR_INC, 0, NULL);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INC);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_self, 0, NULL);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1)) int DCALL
default__math_inc__with__math_inplace_add(DeeObject **__restrict p_self) {
	return CALL_DEPENDENCY(tp_math->tp_inplace_add, p_self, DeeInt_One);
}
INTERN WUNUSED NONNULL((1)) int DCALL
default__math_inc__with__math_add(DeeObject **__restrict p_self) {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_add, *p_self, DeeInt_One);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__math_inc(DeeTypeObject *tp_self, DeeObject **p_self) {
	return (*tp_self->)();
}
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tusrtype__math_inc(DeeTypeObject *tp_self, DeeObject **p_self) {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_self, OPERATOR_INC, 0, NULL);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INC);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_self, 0, NULL);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__math_inc__with__math_inplace_add(DeeTypeObject *tp_self, DeeObject **p_self) {
	return CALL_DEPENDENCY(tp_math->tp_inplace_add, p_self, DeeInt_One);
}
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__math_inc__with__math_add(DeeTypeObject *tp_self, DeeObject **p_self) {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_add, *p_self, DeeInt_One);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

/* tp_math->tp_dec */
INTERN WUNUSED NONNULL((1)) int DCALL
usrtype__math_dec(DeeObject **__restrict p_self) {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_self, OPERATOR_DEC, 0, NULL);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_DEC);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_self, 0, NULL);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1)) int DCALL
default__math_dec__with__math_inplace_sub(DeeObject **__restrict p_self) {
	return CALL_DEPENDENCY(tp_math->tp_inplace_sub, p_self, DeeInt_One);
}
INTERN WUNUSED NONNULL((1)) int DCALL
default__math_dec__with__math_sub(DeeObject **__restrict p_self) {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_sub, *p_self, DeeInt_One);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__math_dec(DeeTypeObject *tp_self, DeeObject **p_self) {
	return (*tp_self->)();
}
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tusrtype__math_dec(DeeTypeObject *tp_self, DeeObject **p_self) {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_self, OPERATOR_DEC, 0, NULL);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_DEC);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_self, 0, NULL);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__math_dec__with__math_inplace_sub(DeeTypeObject *tp_self, DeeObject **p_self) {
	return CALL_DEPENDENCY(tp_math->tp_inplace_sub, p_self, DeeInt_One);
}
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__math_dec__with__math_sub(DeeTypeObject *tp_self, DeeObject **p_self) {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_sub, *p_self, DeeInt_One);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

/* tp_with->tp_enter */
INTERN WUNUSED NONNULL((1)) int DCALL
usrtype__with_enter(DeeObject *__restrict self) {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, self, OPERATOR_ENTER, 0, NULL);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_ENTER);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, self, 0, NULL);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result);
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__with_enter(DeeTypeObject *tp_self, DeeObject *self) {
	return (*tp_self->)();
}
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tusrtype__with_enter(DeeTypeObject *tp_self, DeeObject *self) {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, self, OPERATOR_ENTER, 0, NULL);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_ENTER);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, self, 0, NULL);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result);
	return 0;
err:
	return -1;
}

/* tp_with->tp_leave */
INTERN WUNUSED NONNULL((1)) int DCALL
usrtype__with_leave(DeeObject *__restrict self) {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, self, OPERATOR_LEAVE, 0, NULL);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_LEAVE);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, self, 0, NULL);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result);
	return 0;
err:
	return -1;
}
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tdefault__with_leave(DeeTypeObject *tp_self, DeeObject *self) {
	return (*tp_self->)();
}
INTERN WUNUSED NONNULL((1, 2)) int DCALL
tusrtype__with_leave(DeeTypeObject *tp_self, DeeObject *self) {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, self, OPERATOR_LEAVE, 0, NULL);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_LEAVE);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, self, 0, NULL);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result);
	return 0;
err:
	return -1;
}
/*[[[end]]]*/
/* clang-format on */


DECL_END
#endif /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

#endif /* !GUARD_DEEMON_RUNTIME_OPERATOR_HINT_DEFAULTS_C */
