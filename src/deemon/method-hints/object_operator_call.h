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

/************************************************************************/
/* deemon.Object.operator call()                                        */
/************************************************************************/

operator {

[[export("DeeObject_{|T}Call")]]
[[wunused]] DREF DeeObject *
tp_call([[nonnull]] DeeObject *self,
        size_t argc, DeeObject *const *argv)
%{class using OPERATOR_CALL: {
	return_DeeClass_CallOperator(THIS_TYPE, self, OPERATOR_CALL, argc, argv);
}}
%{using tp_callable->tp_call_kw: {
	return CALL_DEPENDENCY(tp_callable->tp_call_kw, self, argc, argv, NULL);
}} = OPERATOR_CALL;


[[export("DeeObject_{|T}CallKw")]]
[[wunused]] DREF DeeObject *
tp_callable->tp_call_kw([[nonnull]] DeeObject *self,
                        size_t argc, DeeObject *const *argv,
                        DeeObject *kw)
%{class using OPERATOR_CALL: {
	DREF DeeObject *func, *result;
	func = DeeClass_GetOperator(THIS_TYPE, OPERATOR_CALL);
	if unlikely(!func)
		goto err;
	result = DeeObject_ThisCallKw(func, self, argc, argv, kw);
	Dee_Decref_unlikely(func);
	return result;
err:
	return NULL;
}}
%{using tp_call: {
	if (kw) {
		if (DeeKwds_Check(kw)) {
			if (DeeKwds_SIZE(kw) != 0)
				goto err_no_keywords;
		} else {
			size_t kw_length;
			kw_length = DeeObject_Size(kw);
			if unlikely(kw_length == (size_t)-1)
				goto err;
			if (kw_length != 0)
				goto err_no_keywords;
		}
	}
	return CALL_DEPENDENCY(tp_call, self, argc, argv);
err_no_keywords:
	err_keywords_not_accepted(THIS_TYPE, kw);
err:
	return NULL;
}} = OPERATOR_CALL;


[[export("DeeObject_{|T}ThisCall")]]
[[wunused]] DREF DeeObject *
tp_callable->tp_thiscall([[nonnull]] DeeObject *self,
                         [[nonnull]] DeeObject *thisarg,
                         size_t argc, DeeObject *const *argv)
%{using tp_call: {
	DREF DeeObject *result;
	DeeObject **full_argv;
	full_argv = (DeeObject **)Dee_Mallocac(1 + argc, sizeof(DeeObject *));
	if unlikely(!full_argv)
		goto err;
	full_argv[0] = thisarg;
	memcpyc(full_argv + 1, argv, argc, sizeof(DeeObject *));
	result = CALL_DEPENDENCY(tp_call, self, 1 + argc, full_argv);
	Dee_Freea(full_argv);
	return result;
err:
	return NULL;
}} = OPERATOR_CALL;

[[export("DeeObject_{|T}ThisCallKw")]]
[[wunused]] DREF DeeObject *
tp_callable->tp_thiscall_kw([[nonnull]] DeeObject *self,
                            [[nonnull]] DeeObject *thisarg,
                            size_t argc, DeeObject *const *argv,
                            DeeObject *kw)
%{using tp_callable->tp_call_kw: {
	DREF DeeObject *result;
	DeeObject **full_argv;
	full_argv = (DeeObject **)Dee_Mallocac(1 + argc, sizeof(DeeObject *));
	if unlikely(!full_argv)
		goto err;
	full_argv[0] = thisarg;
	memcpyc(full_argv + 1, argv, argc, sizeof(DeeObject *));
	result = CALL_DEPENDENCY(tp_callable->tp_call_kw, self, 1 + argc, full_argv, kw);
	Dee_Freea(full_argv);
	return result;
err:
	return NULL;
}}
%{using tp_callable->tp_thiscall: {
	if (kw) {
		if (DeeKwds_Check(kw)) {
			if (DeeKwds_SIZE(kw) != 0)
				goto err_no_keywords;
		} else {
			size_t kw_length;
			kw_length = DeeObject_Size(kw);
			if unlikely(kw_length == (size_t)-1)
				goto err;
			if (kw_length != 0)
				goto err_no_keywords;
		}
	}
	return CALL_DEPENDENCY(tp_callable->tp_thiscall, self, thisarg, argc, argv);
err_no_keywords:
	err_keywords_not_accepted(THIS_TYPE, kw);
err:
	return NULL;
}} = OPERATOR_CALL;



/* TODO: Only if "CONFIG_CALLTUPLE_OPTIMIZATIONS" */
[[export("DeeObject_{|T}CallTuple")]]
[[wunused]] DREF DeeObject *
tp_callable->tp_call_tuple([[nonnull]] DeeObject *self,
                           [[nonnull]] DeeObject *args)
%{using tp_call: {
	return CALL_DEPENDENCY(tp_call, self, DeeTuple_SIZE(args), DeeTuple_ELEM(args));
}} = OPERATOR_CALL;

/* TODO: Only if "CONFIG_CALLTUPLE_OPTIMIZATIONS" */
[[export("DeeObject_{|T}CallTupleKw")]]
[[wunused]] DREF DeeObject *
tp_callable->tp_call_tuple_kw([[nonnull]] DeeObject *self,
                              [[nonnull]] DeeObject *args,
                              DeeObject *kw)
%{using tp_callable->tp_call_kw: {
	return CALL_DEPENDENCY(tp_callable->tp_call_kw, self, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw);
}} = OPERATOR_CALL;

/* TODO: Only if "CONFIG_CALLTUPLE_OPTIMIZATIONS" */
[[export("DeeObject_{|T}ThisCallTuple")]]
[[wunused]] DREF DeeObject *
tp_callable->tp_thiscall_tuple([[nonnull]] DeeObject *self,
                               [[nonnull]] DeeObject *thisarg,
                               [[nonnull]] DeeObject *args)
%{using tp_callable->tp_thiscall: {
	return CALL_DEPENDENCY(tp_callable->tp_thiscall, self, thisarg, DeeTuple_SIZE(args), DeeTuple_ELEM(args));
}} = OPERATOR_CALL;

/* TODO: Only if "CONFIG_CALLTUPLE_OPTIMIZATIONS" */
[[export("DeeObject_{|T}ThisCallTupleKw")]]
[[wunused]] DREF DeeObject *
tp_callable->tp_thiscall_tuple_kw([[nonnull]] DeeObject *self,
                                  [[nonnull]] DeeObject *thisarg,
                                  [[nonnull]] DeeObject *args,
                                  DeeObject *kw)
%{using tp_callable->tp_thiscall_kw: {
	return CALL_DEPENDENCY(tp_callable->tp_thiscall_kw, self, thisarg, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw);
}} = OPERATOR_CALL;

} /* operator */
