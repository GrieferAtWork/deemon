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
/* deemon.Object.operator [:]()                                         */
/************************************************************************/

operator {

[[export("DeeObject_{|T}GetRange")]]
[[wunused]] DREF DeeObject *
tp_seq->tp_getrange([[nonnull]] DeeObject *self,
                    [[nonnull]] DeeObject *start,
                    [[nonnull]] DeeObject *end)
%{class {
	DeeObject *args[2];
	args[0] = start;
	args[1] = end;
	return_DeeClass_CallOperator(THIS_TYPE, self, OPERATOR_GETRANGE, 2, args);
}}
%{using [tp_seq->tp_getrange_index, tp_seq->tp_getrange_index_n]: {
	Dee_ssize_t start_index, end_index;
	if (DeeObject_AsSSize(start, &start_index))
		goto err;
	if (DeeNone_Check(end))
		return CALL_DEPENDENCY(tp_seq->tp_getrange_index_n, self, start_index);
	if (DeeObject_AsSSize(end, &end_index))
		goto err;
	return CALL_DEPENDENCY(tp_seq->tp_getrange_index, self, start_index, end_index);
err:
	return NULL;
}} = OPERATOR_GETRANGE;



[[export("DeeObject_{|T}GetRangeIndex")]]
[[wunused]] DREF DeeObject *
tp_seq->tp_getrange_index([[nonnull]] DeeObject *self,
                          Dee_ssize_t start, Dee_ssize_t end)
%{using tp_seq->tp_getrange: {
	DREF DeeObject *result;
	DREF DeeObject *startob, *endob;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	endob = DeeInt_NewSSize(end);
	if unlikely(!endob)
		goto err_startob;
	result = CALL_DEPENDENCY(tp_seq->tp_getrange, self, startob, endob);
	Dee_Decref(endob);   /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
	Dee_Decref(startob); /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
	return result;
err_startob:
	Dee_Decref(startob); /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
err:
	return NULL;
}} = OPERATOR_GETRANGE;




[[export("DeeObject_{|T}GetRangeIndexN")]]
[[wunused]] DREF DeeObject *
tp_seq->tp_getrange_index_n([[nonnull]] DeeObject *self,
                            Dee_ssize_t start)
%{using tp_seq->tp_getrange: {
	DREF DeeObject *result;
	DREF DeeObject *startob;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	result = CALL_DEPENDENCY(tp_seq->tp_getrange, self, startob, Dee_None);
	Dee_Decref(startob); /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
	return result;
err_startob:
	Dee_Decref(startob); /* Would be "_likely", but DeeInt_NewSSize() re-uses values. */
err:
	return NULL;
}} = OPERATOR_GETRANGE;



} /* operator */
