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
#ifdef __INTELLISENSE__
#include "format.c"
//#define DEFINE_sfa_skipexpr
#define DEFINE_sfa_evalexpr
#endif /* __INTELLISENSE__ */

#if (defined(DEFINE_sfa_skipexpr) + defined(DEFINE_sfa_evalexpr)) != 1
#error "Must #define exactly one of these macros"
#endif /* DEFINE_sfa_... */

#ifdef DEFINE_sfa_skipexpr
#define LOCAL_ERRVAL                   (-1)
#define LOCAL_return_type           int
#define LOCAL_OK__or__lhs             0
#define LOCAL_ISOK(x)               ((x) == 0)
#define LOCAL__param_lhs            /* nothing */
#define LOCAL__arg_lhs(x)           /* nothing */
#define LOCAL_sfa_evalunary_base    sfa_skipunary_base
#define LOCAL_sfa_evalunary         sfa_skipunary
#define LOCAL_sfa_evalprod          sfa_skipprod
#define LOCAL_sfa_evalsum           sfa_skipsum
#define LOCAL_sfa_evalshift         sfa_skipshift
#define LOCAL_sfa_evalcmp           sfa_skipcmp
#define LOCAL_sfa_evalcmpeq         sfa_skipcmpeq
#define LOCAL_sfa_evaland           sfa_skipand
#define LOCAL_sfa_evalxor           sfa_skipxor
#define LOCAL_sfa_evalor            sfa_skipor
#define LOCAL_sfa_evalas            sfa_skipas
#define LOCAL_sfa_evalland          sfa_skipland
#define LOCAL_sfa_evallor           sfa_skiplor
#define LOCAL_sfa_evalcond          sfa_skipcond
#define LOCAL_sfa_evalunary_operand sfa_skipunary_operand
#define LOCAL_sfa_evalprod_operand  sfa_skipprod_operand
#define LOCAL_sfa_evalsum_operand   sfa_skipsum_operand
#define LOCAL_sfa_evalshift_operand sfa_skipshift_operand
#define LOCAL_sfa_evalcmp_operand   sfa_skipcmp_operand
#define LOCAL_sfa_evalcmpeq_operand sfa_skipcmpeq_operand
#define LOCAL_sfa_evaland_operand   sfa_skipand_operand
#define LOCAL_sfa_evalxor_operand   sfa_skipxor_operand
#define LOCAL_sfa_evalor_operand    sfa_skipor_operand
#define LOCAL_sfa_evalas_operand    sfa_skipas_operand
#define LOCAL_sfa_evalland_operand  sfa_skipland_operand
#define LOCAL_sfa_evallor_operand   sfa_skiplor_operand
#define LOCAL_sfa_evalcond_operand  sfa_skipcond_operand
#else /* DEFINE_sfa_skipexpr */
#define LOCAL_ERRVAL                   NULL
#define LOCAL_OK__or__lhs             lhs
#define LOCAL_ISOK(x)               ((x) != NULL)
#define LOCAL_return_type           DREF DeeObject *
#define LOCAL__param_lhs            , /*inherit(always)*/ DREF DeeObject *__restrict lhs
#define LOCAL__arg_lhs(x)           , x
#define LOCAL_sfa_evalunary_base    sfa_evalunary_base
#define LOCAL_sfa_evalunary         sfa_evalunary
#define LOCAL_sfa_evalprod          sfa_evalprod
#define LOCAL_sfa_evalsum           sfa_evalsum
#define LOCAL_sfa_evalshift         sfa_evalshift
#define LOCAL_sfa_evalcmp           sfa_evalcmp
#define LOCAL_sfa_evalcmpeq         sfa_evalcmpeq
#define LOCAL_sfa_evaland           sfa_evaland
#define LOCAL_sfa_evalxor           sfa_evalxor
#define LOCAL_sfa_evalor            sfa_evalor
#define LOCAL_sfa_evalas            sfa_evalas
#define LOCAL_sfa_evalland          sfa_evalland
#define LOCAL_sfa_evallor           sfa_evallor
#define LOCAL_sfa_evalcond          sfa_evalcond
#define LOCAL_sfa_evalunary_operand sfa_evalunary_operand
#define LOCAL_sfa_evalprod_operand  sfa_evalprod_operand
#define LOCAL_sfa_evalsum_operand   sfa_evalsum_operand
#define LOCAL_sfa_evalshift_operand sfa_evalshift_operand
#define LOCAL_sfa_evalcmp_operand   sfa_evalcmp_operand
#define LOCAL_sfa_evalcmpeq_operand sfa_evalcmpeq_operand
#define LOCAL_sfa_evaland_operand   sfa_evaland_operand
#define LOCAL_sfa_evalxor_operand   sfa_evalxor_operand
#define LOCAL_sfa_evalor_operand    sfa_evalor_operand
#define LOCAL_sfa_evalas_operand    sfa_evalas_operand
#define LOCAL_sfa_evalland_operand  sfa_evalland_operand
#define LOCAL_sfa_evallor_operand   sfa_evallor_operand
#define LOCAL_sfa_evalcond_operand  sfa_evalcond_operand
#endif /* !DEFINE_sfa_skipexpr */

DECL_BEGIN

PRIVATE WUNUSED NONNULL((1)) LOCAL_return_type DFCALL
LOCAL_sfa_evalunary_base(struct string_format_advanced *__restrict self) {
#ifdef DEFINE_sfa_skipexpr
	LOCAL_return_type result = 0;
#else /* DEFINE_sfa_skipexpr */
	LOCAL_return_type result;
#endif /* !DEFINE_sfa_skipexpr */
	switch (__builtin_expect(self->sfa_exprtok, SFA_TOK_KEYWORD)) {

	case SFA_TOK_KEYWORD: {
		size_t kwdlen = sfa_tok_keyword_getlen(self);
#ifndef DEFINE_sfa_skipexpr
		char const *kwd = self->sfa_parser.sfp_iter;
		result = DeeObject_GetItemStringLenHash(self->sfa_args, kwd, kwdlen,
		                                        Dee_HashUtf8(kwd, kwdlen));
		if unlikely(!result)
			goto err;
#endif /* !DEFINE_sfa_skipexpr */
		self->sfa_parser.sfp_iter += kwdlen;
		sfa_yield(self);
	}	break;

	case SFA_TOK_INT: {
		size_t intlen = sfa_tok_int_getlen(self);
#ifndef DEFINE_sfa_skipexpr
		int decode_status;
		size_t index;
		char const *intrepr = self->sfa_parser.sfp_iter;
		decode_status = Dee_TAtoiu(intrepr, intlen, DEEINT_STRING(0, DEEINT_STRING_FTRY), &index);
		if unlikely(decode_status < 0)
			goto err;
		if (decode_status > 0) {
			DREF DeeObject *index_ob;
			index_ob = DeeInt_FromString(intrepr, intlen, DEEINT_STRING(0, DEEINT_STRING_FNORMAL));
			if unlikely(!index_ob)
				goto err;
			result = DeeObject_GetItem(self->sfa_args, index_ob);
			Dee_Decref(index_ob);
		} else {
			result = DeeObject_GetItemIndex(self->sfa_args, index);
		}
		if unlikely(!result)
			goto err;
#endif /* !DEFINE_sfa_skipexpr */
		self->sfa_parser.sfp_iter += intlen;
		sfa_yield(self);
	}	break;

	case SFA_TOK_CHAR:
	case SFA_TOK_STRING:
		/* TODO */
		DeeError_NOTIMPLEMENTED();
		goto err;

	case SFA_TOK_D_TRUE:
#ifndef DEFINE_sfa_skipexpr
		result = Dee_True;
		Dee_Incref(result);
#endif /* !DEFINE_sfa_skipexpr */
		sfa_yield(self);
		break;

	case SFA_TOK_D_FALSE:
#ifndef DEFINE_sfa_skipexpr
		result = Dee_False;
		Dee_Incref(result);
#endif /* !DEFINE_sfa_skipexpr */
		sfa_yield(self);
		break;

	case SFA_TOK_D_NONE:
#ifndef DEFINE_sfa_skipexpr
		result = Dee_None;
		Dee_Incref(result);
#endif /* !DEFINE_sfa_skipexpr */
		sfa_yield(self);
		break;

	case SFA_TOK_D_STR:
	case SFA_TOK_D_REPR:
	case SFA_TOK_D_COPY:
	case SFA_TOK_D_DEEPCOPY:
	case SFA_TOK_D_TYPE: {
#ifndef DEFINE_sfa_skipexpr
		unsigned int tok = self->sfa_exprtok;
#endif /* !DEFINE_sfa_skipexpr */
		bool saved_sfa_inparen;
		sfa_yield(self);
		if unlikely(self->sfa_exprtok != '(') {
			sfa_err_bad_token(self, "(");
			goto err;
		}
		sfa_yield(self);
		saved_sfa_inparen = self->sfa_inparen;
		self->sfa_inparen = true;
		result = LOCAL_sfa_evalcond(self);
		self->sfa_inparen = saved_sfa_inparen;
		if unlikely(!LOCAL_ISOK(result))
			goto err;
		if unlikely(self->sfa_exprtok != ')') {
#ifndef DEFINE_sfa_skipexpr
			Dee_Decref(result);
#endif /* !DEFINE_sfa_skipexpr */
			sfa_err_bad_token(self, ")");
			goto err;
		}
		sfa_yield(self);
#ifndef DEFINE_sfa_skipexpr
		{
			DREF DeeObject *final_result;
			switch (tok) {
			case SFA_TOK_D_STR:
				final_result = DeeObject_Str(result);
				break;
			case SFA_TOK_D_REPR:
				final_result = DeeObject_Repr(result);
				break;
			case SFA_TOK_D_COPY:
				final_result = DeeObject_Copy(result);
				break;
			case SFA_TOK_D_DEEPCOPY:
				final_result = DeeObject_DeepCopy(result);
				break;
			case SFA_TOK_D_TYPE:
				final_result = (DREF DeeObject *)Dee_TYPE(result);
				Dee_Incref(final_result);
				break;
			default: __builtin_unreachable();
			}
			Dee_Decref(result);
			/*if unlikely(!final_result)
				goto err;*/
			result = final_result;
		}
#endif /* !DEFINE_sfa_skipexpr */
	}	break;

	case SFA_TOK_D_INT: {
		size_t intlen = sfa_tok_int_getlen(self);
#ifndef DEFINE_sfa_skipexpr
		char const *intrepr = self->sfa_parser.sfp_iter;
		result = DeeInt_FromString(intrepr, intlen, DEEINT_STRING(0, DEEINT_STRING_FNORMAL));
		/*if unlikely(!result)
			goto err;*/
#endif /* !DEFINE_sfa_skipexpr */
		self->sfa_parser.sfp_iter += intlen;
		sfa_yield(self);
	}	break;

	case SFA_TOK_D_BOUND: {
		bool saved_sfa_inparen;
		sfa_yield(self);
		if unlikely(self->sfa_exprtok != '(') {
			sfa_err_bad_token(self, "(");
			goto err;
		}
		sfa_yield(self);

		saved_sfa_inparen = self->sfa_inparen;
		self->sfa_inparen = true;
		/* TODO */
		DeeError_NOTIMPLEMENTED();
		self->sfa_inparen = saved_sfa_inparen;
		goto err;
	}	break;

	case '#':
	case '+':
	case '-':
	case '~': {
#ifndef DEFINE_sfa_skipexpr
		unsigned int tok = self->sfa_exprtok;
#endif /* !DEFINE_sfa_skipexpr */
		sfa_yield(self);
		result = LOCAL_sfa_evalunary(self);
		if unlikely(!LOCAL_ISOK(result))
			goto err;
#ifndef DEFINE_sfa_skipexpr
		{
			DREF DeeObject *final_result;
			switch (tok) {
			case '#': final_result = DeeObject_SizeOb(result); break;
			case '+': final_result = DeeObject_Pos(result); break;
			case '-': final_result = DeeObject_Neg(result); break;
			case '~': final_result = DeeObject_Inv(result); break;
			default: __builtin_unreachable();
			}
			Dee_Decref(result);
			/*if unlikely(!final_result)
				goto err;*/
			result = final_result;
		}
#endif /* !DEFINE_sfa_skipexpr */
	}	break;

	case '(': {
		bool saved_sfa_inparen;
		sfa_yield(self);
		if (self->sfa_exprtok == ')') {
			sfa_yield(self);
#ifndef DEFINE_sfa_skipexpr
			result = Dee_EmptyTuple;
			Dee_Incref(result);
#endif /* !DEFINE_sfa_skipexpr */
			break;
		}
		saved_sfa_inparen = self->sfa_inparen;
		self->sfa_inparen = true;
		result = LOCAL_sfa_evalcond(self);
		if unlikely(!LOCAL_ISOK(result))
			goto err;
		if (self->sfa_exprtok == ',') {
			sfa_yield(self);
			if (self->sfa_exprtok == ')') {
				sfa_yield(self);
#ifndef DEFINE_sfa_skipexpr
				{
					DREF DeeObject *final_result;
					final_result = DeeTuple_NewVectorSymbolic(1, &result);
					if unlikely(!final_result) {
						Dee_Decref(result);
						goto err;
					}
					result = final_result;
				}
#endif /* !DEFINE_sfa_skipexpr */
			} else {
#ifndef DEFINE_sfa_skipexpr
				DREF DeeTupleObject *items;
				size_t count = 1;
				items = DeeTuple_TryNewUninitialized(4);
				if unlikely(!items) {
					items = DeeTuple_NewUninitialized(2);
					if unlikely(!items) {
						Dee_Decref(result);
						goto err;
					}
				}
				DeeTuple_SET(items, 0, result); /* Inherit reference */
#endif /* !DEFINE_sfa_skipexpr */
				do {
					LOCAL_return_type elem;
					elem = LOCAL_sfa_evalcond(self);
					if unlikely(!LOCAL_ISOK(elem)) {
#ifndef DEFINE_sfa_skipexpr
err_tuple_items_count:
						Dee_Decrefv(DeeTuple_ELEM(items), count);
						DeeTuple_FreeUninitialized(items);
#endif /* !DEFINE_sfa_skipexpr */
						goto err;
					}
#ifndef DEFINE_sfa_skipexpr
					if unlikely(count >= DeeTuple_SIZE(items)) {
						size_t new_size = DeeTuple_SIZE(items) * 2;
						DREF DeeTupleObject *new_items;
						new_items = DeeTuple_TryResizeUninitialized(items, new_size);
						if unlikely(!new_items) {
							new_size = count + 1;
							new_items = DeeTuple_ResizeUninitialized(items, new_size);
							if unlikely(!new_items) {
								Dee_Decref(elem);
								goto err_tuple_items_count;
							}
						}
						items = new_items;
					}
					DeeTuple_SET(items, count, elem); /* Inherit reference */
					++count;
#endif /* !DEFINE_sfa_skipexpr */
					if (self->sfa_exprtok != ',')
						break;
					sfa_yield(self);
				} while (self->sfa_exprtok != ')');
#ifndef DEFINE_sfa_skipexpr
				result = (LOCAL_return_type)DeeTuple_TruncateUninitialized(items, count);
#endif /* !DEFINE_sfa_skipexpr */
				if (self->sfa_exprtok != ')') {
#ifndef DEFINE_sfa_skipexpr
					Dee_Decref(result);
#endif /* !DEFINE_sfa_skipexpr */
					sfa_err_bad_token(self, ")");
					goto err;
				}
			}
		}
		self->sfa_inparen = saved_sfa_inparen;
	}	break;

	case '[': {
		LOCAL_return_type elem;
		bool saved_sfa_inparen;
#ifndef DEFINE_sfa_skipexpr
		DREF DeeTupleObject *items;
		size_t count;
#endif /* !DEFINE_sfa_skipexpr */
		sfa_yield(self);
		if (self->sfa_exprtok == ']') {
			sfa_yield(self);
#ifndef DEFINE_sfa_skipexpr
			result = Dee_EmptyTuple;
			Dee_Incref(result);
#endif /* !DEFINE_sfa_skipexpr */
			break;
		}
#ifndef DEFINE_sfa_skipexpr
		count = 0;
		items = DeeTuple_TryNewUninitialized(2);
		if unlikely(!items) {
			items = DeeTuple_NewUninitialized(1);
			if unlikely(!items)
				goto err;
		}
#endif /* !DEFINE_sfa_skipexpr */
		saved_sfa_inparen = self->sfa_inparen;
		self->sfa_inparen = true;

		do {
			elem = LOCAL_sfa_evalcond(self);
			if unlikely(!LOCAL_ISOK(elem)) {
#ifndef DEFINE_sfa_skipexpr
err_list_items_count:
				Dee_Decrefv(DeeTuple_ELEM(items), count);
				DeeTuple_FreeUninitialized(items);
#endif /* !DEFINE_sfa_skipexpr */
				goto err;
			}
#ifndef DEFINE_sfa_skipexpr
			if unlikely(count >= DeeTuple_SIZE(items)) {
				size_t new_size = DeeTuple_SIZE(items) * 2;
				DREF DeeTupleObject *new_items;
				new_items = DeeTuple_TryResizeUninitialized(items, new_size);
				if unlikely(!new_items) {
					new_size = count + 1;
					new_items = DeeTuple_ResizeUninitialized(items, new_size);
					if unlikely(!new_items) {
						Dee_Decref(elem);
						goto err_list_items_count;
					}
				}
				items = new_items;
			}
			DeeTuple_SET(items, count, elem); /* Inherit reference */
			++count;
#endif /* !DEFINE_sfa_skipexpr */
			if (self->sfa_exprtok != ',')
				break;
			sfa_yield(self);
		} while (self->sfa_exprtok != ']');
#ifndef DEFINE_sfa_skipexpr
		result = (LOCAL_return_type)DeeTuple_TruncateUninitialized(items, count);
#endif /* !DEFINE_sfa_skipexpr */
		if (self->sfa_exprtok != ']') {
#ifndef DEFINE_sfa_skipexpr
			Dee_Decref(result);
#endif /* !DEFINE_sfa_skipexpr */
			sfa_err_bad_token(self, "]");
			goto err;
		}
		self->sfa_inparen = saved_sfa_inparen;
	}	break;

	case '!':
		if likely(!self->sfa_inparen) {
			sfa_yield(self);
			result = LOCAL_sfa_evalunary(self);
			if unlikely(!LOCAL_ISOK(result))
				goto err;
#ifndef DEFINE_sfa_skipexpr
			{
				int asbool = DeeObject_BoolInherited(result);
				if unlikely(asbool < 0)
					goto err;
				result = DeeBool_For(!asbool);
				Dee_Incref(result);
			}
#endif /* !DEFINE_sfa_skipexpr */
			break;
		}
		ATTR_FALLTHROUGH
	default:
		sfa_err_bad_token(self, "<unary expression>");
		goto err;
	}
	return result;
err:
	return LOCAL_ERRVAL;
}

PRIVATE WUNUSED NONNULL((1)) LOCAL_return_type DFCALL
LOCAL_sfa_evalunary_operand(struct string_format_advanced *__restrict self LOCAL__param_lhs) {
	ASSERT(self->sfa_exprtok == '.' ||
	       self->sfa_exprtok == '[' ||
	       self->sfa_exprtok == '(');
	do {
		switch (self->sfa_exprtok) {

		case '.': {
			sfa_yield(self);
			if (self->sfa_exprtok == '{') {
#ifndef DEFINE_sfa_skipexpr
				DREF DeeObject *attrname;
				LOCAL_return_type result;
#endif /* !DEFINE_sfa_skipexpr */
				sfa_yield(self);
#ifdef DEFINE_sfa_skipexpr
				if unlikely(string_format_advanced_skip1(self))
					goto err_lhs;
#else /* DEFINE_sfa_skipexpr */
				attrname = string_format_advanced_do1_intostr(self);
				if unlikely(!attrname)
					goto err_lhs;
				result = DeeObject_GetAttr(lhs, attrname);
				Dee_Decref(attrname);
				Dee_Decref(lhs);
				if unlikely(!result)
					goto err;
				lhs = result;
#endif /* !DEFINE_sfa_skipexpr */
			} else if (self->sfa_exprtok == SFA_TOK_KEYWORD) {
				size_t kwdlen = sfa_tok_keyword_getlen(self);
#ifndef DEFINE_sfa_skipexpr
				LOCAL_return_type result;
				char const *kwd = self->sfa_parser.sfp_iter;
				result = DeeObject_GetAttrStringLenHash(lhs, kwd, kwdlen, Dee_HashUtf8(kwd, kwdlen));
				Dee_Decref(lhs);
				if unlikely(!result)
					goto err;
				lhs = result;
#endif /* !DEFINE_sfa_skipexpr */
				self->sfa_parser.sfp_iter += kwdlen;
				sfa_yield(self);
			} else {
				sfa_err_bad_token(self, "<keyword>");
			}
		}	break;

		case '[': {
#ifndef DEFINE_sfa_skipexpr
			LOCAL_return_type result;
#endif /* !DEFINE_sfa_skipexpr */
			bool saved_sfa_inparen;
			saved_sfa_inparen = self->sfa_inparen;
			self->sfa_inparen = true;
			sfa_yield(self);
			if (self->sfa_exprtok == ':') {
				sfa_yield(self);
				if (self->sfa_exprtok == ']') {
					sfa_yield(self);
#ifndef DEFINE_sfa_skipexpr
					result = DeeObject_GetRange(lhs, Dee_None, Dee_None);
#endif /* !DEFINE_sfa_skipexpr */
				} else {
					LOCAL_return_type end_index;
					end_index = LOCAL_sfa_evalcond(self);
					if unlikely(!LOCAL_ISOK(end_index))
						goto err_lhs;
					if unlikely(self->sfa_exprtok != ']') {
#ifndef DEFINE_sfa_skipexpr
						Dee_Decref(end_index);
#endif /* !DEFINE_sfa_skipexpr */
						sfa_err_bad_token(self, "]");
						goto err_lhs;
					}
					sfa_yield(self);
#ifndef DEFINE_sfa_skipexpr
					result = DeeObject_GetRange(lhs, Dee_None, end_index);
					Dee_Decref(end_index);
#endif /* !DEFINE_sfa_skipexpr */
				}
			} else {
				LOCAL_return_type index;
				index = LOCAL_sfa_evalcond(self);
				if unlikely(!LOCAL_ISOK(index))
					goto err_lhs;
				if (self->sfa_exprtok == ':') {
					sfa_yield(self);
					if (self->sfa_exprtok == ']') {
						sfa_yield(self);
#ifndef DEFINE_sfa_skipexpr
						result = DeeObject_GetRange(lhs, index, Dee_None);
#endif /* !DEFINE_sfa_skipexpr */
					} else {
						LOCAL_return_type end_index;
						end_index = LOCAL_sfa_evalcond(self);
						if unlikely(!LOCAL_ISOK(end_index)) {
#ifndef DEFINE_sfa_skipexpr
							Dee_Decref(index);
#endif /* !DEFINE_sfa_skipexpr */
							goto err_lhs;
						}
						if unlikely(self->sfa_exprtok != ']') {
#ifndef DEFINE_sfa_skipexpr
							Dee_Decref(end_index);
							Dee_Decref(index);
#endif /* !DEFINE_sfa_skipexpr */
							sfa_err_bad_token(self, "]");
							goto err_lhs;
						}
						sfa_yield(self);
#ifndef DEFINE_sfa_skipexpr
						result = DeeObject_GetRange(lhs, index, end_index);
						Dee_Decref(end_index);
#endif /* !DEFINE_sfa_skipexpr */
					}
				} else {
					if unlikely(self->sfa_exprtok != ']') {
						sfa_err_bad_token(self, "]");
						goto err_lhs;
					}
					sfa_yield(self);
#ifndef DEFINE_sfa_skipexpr
					result = DeeObject_GetItem(lhs, index);
#endif /* !DEFINE_sfa_skipexpr */
				}
#ifndef DEFINE_sfa_skipexpr
				Dee_Decref(index);
#endif /* !DEFINE_sfa_skipexpr */
			}
			self->sfa_inparen = saved_sfa_inparen;
#ifndef DEFINE_sfa_skipexpr
			Dee_Decref(lhs);
			if unlikely(!result)
				goto err;
			lhs = result;
#endif /* !DEFINE_sfa_skipexpr */
		}	break;

		case '(': {
			bool saved_sfa_inparen;
			saved_sfa_inparen = self->sfa_inparen;
			self->sfa_inparen = true;
			/* TODO */
			DeeError_NOTIMPLEMENTED();
			self->sfa_inparen = saved_sfa_inparen;
			goto err_lhs;
		}	break;

		default: __builtin_unreachable();
		}
	} while (self->sfa_exprtok == '.' ||
	         self->sfa_exprtok == '[' ||
	         self->sfa_exprtok == '(');
	return LOCAL_OK__or__lhs;
err_lhs:
#ifndef DEFINE_sfa_skipexpr
	Dee_Decref(lhs);
err:
#endif /* !DEFINE_sfa_skipexpr */
	return LOCAL_ERRVAL;
}

PRIVATE WUNUSED NONNULL((1)) LOCAL_return_type DFCALL
LOCAL_sfa_evalprod_operand(struct string_format_advanced *__restrict self LOCAL__param_lhs) {
	LOCAL_return_type rhs;
	ASSERT(self->sfa_exprtok == '*' ||
	       self->sfa_exprtok == '/' ||
	       self->sfa_exprtok == '%' ||
	       self->sfa_exprtok == SFA_TOK_POW);
	do {
#ifndef DEFINE_sfa_skipexpr
		unsigned int tok = self->sfa_exprtok;
#endif /* !DEFINE_sfa_skipexpr */
		sfa_yield(self);
		rhs = LOCAL_sfa_evalunary(self);
		if unlikely(!LOCAL_ISOK(rhs))
			goto err_lhs;
#ifndef DEFINE_sfa_skipexpr
		{
			LOCAL_return_type result;
			switch (tok) {
			case '*':
				result = DeeObject_Mul(lhs, rhs);
				break;
			case '/':
				result = DeeObject_Div(lhs, rhs);
				break;
			case '%':
				result = DeeObject_Mod(lhs, rhs);
				break;
			case SFA_TOK_POW:
				result = DeeObject_Pow(lhs, rhs);
				break;
			default: __builtin_unreachable();;
			}
			Dee_Decref(rhs);
			Dee_Decref(lhs);
			lhs = result;
			if unlikely(!result)
				goto err;
		}
#endif /* !DEFINE_sfa_skipexpr */
	} while (self->sfa_exprtok == '*' ||
	         self->sfa_exprtok == '/' ||
	         self->sfa_exprtok == '%' ||
	         self->sfa_exprtok == SFA_TOK_POW);
	return LOCAL_OK__or__lhs;
err_lhs:
#ifndef DEFINE_sfa_skipexpr
	Dee_Decref(lhs);
err:
#endif /* !DEFINE_sfa_skipexpr */
	return LOCAL_ERRVAL;
}

PRIVATE WUNUSED NONNULL((1)) LOCAL_return_type DFCALL
LOCAL_sfa_evalsum_operand(struct string_format_advanced *__restrict self LOCAL__param_lhs) {
	LOCAL_return_type rhs;
	ASSERT(self->sfa_exprtok == '+' ||
	       self->sfa_exprtok == '-');
	do {
#ifndef DEFINE_sfa_skipexpr
		unsigned int tok = self->sfa_exprtok;
#endif /* !DEFINE_sfa_skipexpr */
		sfa_yield(self);
		rhs = LOCAL_sfa_evalprod(self);
		if unlikely(!LOCAL_ISOK(rhs))
			goto err_lhs;
#ifndef DEFINE_sfa_skipexpr
		{
			LOCAL_return_type result;
			switch (tok) {
			case '+':
				result = DeeObject_Add(lhs, rhs);
				break;
			case '-':
				result = DeeObject_Sub(lhs, rhs);
				break;
			default: __builtin_unreachable();;
			}
			Dee_Decref(rhs);
			Dee_Decref(lhs);
			lhs = result;
			if unlikely(!result)
				goto err;
		}
#endif /* !DEFINE_sfa_skipexpr */
	} while (self->sfa_exprtok == '+' ||
	         self->sfa_exprtok == '-');
	return LOCAL_OK__or__lhs;
err_lhs:
#ifndef DEFINE_sfa_skipexpr
	Dee_Decref(lhs);
err:
#endif /* !DEFINE_sfa_skipexpr */
	return LOCAL_ERRVAL;
}

PRIVATE WUNUSED NONNULL((1)) LOCAL_return_type DFCALL
LOCAL_sfa_evalshift_operand(struct string_format_advanced *__restrict self LOCAL__param_lhs) {
	LOCAL_return_type rhs;
	ASSERT(self->sfa_exprtok == SFA_TOK_SHL ||
	       self->sfa_exprtok == SFA_TOK_SHR);
	do {
#ifndef DEFINE_sfa_skipexpr
		unsigned int tok = self->sfa_exprtok;
#endif /* !DEFINE_sfa_skipexpr */
		sfa_yield(self);
		rhs = LOCAL_sfa_evalsum(self);
		if unlikely(!LOCAL_ISOK(rhs))
			goto err_lhs;
#ifndef DEFINE_sfa_skipexpr
		{
			LOCAL_return_type result;
			switch (tok) {
			case SFA_TOK_SHL:
				result = DeeObject_Shl(lhs, rhs);
				break;
			case SFA_TOK_SHR:
				result = DeeObject_Shr(lhs, rhs);
				break;
			default: __builtin_unreachable();;
			}
			Dee_Decref(rhs);
			Dee_Decref(lhs);
			lhs = result;
			if unlikely(!result)
				goto err;
		}
#endif /* !DEFINE_sfa_skipexpr */
	} while (self->sfa_exprtok == SFA_TOK_SHL ||
	         self->sfa_exprtok == SFA_TOK_SHR);
	return LOCAL_OK__or__lhs;
err_lhs:
#ifndef DEFINE_sfa_skipexpr
	Dee_Decref(lhs);
err:
#endif /* !DEFINE_sfa_skipexpr */
	return LOCAL_ERRVAL;
}

PRIVATE WUNUSED NONNULL((1)) LOCAL_return_type DFCALL
LOCAL_sfa_evalcmp_operand(struct string_format_advanced *__restrict self LOCAL__param_lhs) {
	LOCAL_return_type rhs;
	ASSERT(self->sfa_exprtok == '<' ||
	       self->sfa_exprtok == '>' ||
	       self->sfa_exprtok == SFA_TOK_LOWER_EQUAL ||
	       self->sfa_exprtok == SFA_TOK_GREATER_EQUAL);
	do {
#ifndef DEFINE_sfa_skipexpr
		unsigned int tok = self->sfa_exprtok;
#endif /* !DEFINE_sfa_skipexpr */
		sfa_yield(self);
		rhs = LOCAL_sfa_evalshift(self);
		if unlikely(!LOCAL_ISOK(rhs))
			goto err_lhs;
#ifndef DEFINE_sfa_skipexpr
		{
			LOCAL_return_type result;
			switch (tok) {
			case '<':
				result = DeeObject_CmpLo(lhs, rhs);
				break;
			case '>':
				result = DeeObject_CmpGr(lhs, rhs);
				break;
			case SFA_TOK_LOWER_EQUAL:
				result = DeeObject_CmpLe(lhs, rhs);
				break;
			case SFA_TOK_GREATER_EQUAL:
				result = DeeObject_CmpGe(lhs, rhs);
				break;
			default: __builtin_unreachable();;
			}
			Dee_Decref(rhs);
			Dee_Decref(lhs);
			lhs = result;
			if unlikely(!result)
				goto err;
		}
#endif /* !DEFINE_sfa_skipexpr */
	} while (self->sfa_exprtok == '<' ||
	         self->sfa_exprtok == '>' ||
	         self->sfa_exprtok == SFA_TOK_LOWER_EQUAL ||
	         self->sfa_exprtok == SFA_TOK_GREATER_EQUAL);
	return LOCAL_OK__or__lhs;
err_lhs:
#ifndef DEFINE_sfa_skipexpr
	Dee_Decref(lhs);
err:
#endif /* !DEFINE_sfa_skipexpr */
	return LOCAL_ERRVAL;
}

PRIVATE WUNUSED NONNULL((1)) LOCAL_return_type DFCALL
LOCAL_sfa_evalcmpeq_operand(struct string_format_advanced *__restrict self LOCAL__param_lhs) {
	unsigned int tok;
	LOCAL_return_type rhs;
	ASSERT(self->sfa_exprtok == SFA_TOK_EQUAL || self->sfa_exprtok == SFA_TOK_EQUAL3 ||
	       self->sfa_exprtok == SFA_TOK_QMARK_QMARK ||
	       self->sfa_exprtok == SFA_TOK_D_IS || self->sfa_exprtok == SFA_TOK_D_IN ||
	       (self->sfa_inparen && (self->sfa_exprtok == '!' ||
	                              self->sfa_exprtok == SFA_TOK_NOT_EQUAL ||
	                              self->sfa_exprtok == SFA_TOK_NOT_EQUAL3)));
	do {
		tok = self->sfa_exprtok;
		sfa_yield(self);
		if (tok == '!') {
			/* Special case: multiple tokens */
#ifndef DEFINE_sfa_skipexpr
			bool invert = true;
#endif /* !DEFINE_sfa_skipexpr */
			char const *orig = self->sfa_parser.sfp_iter;
			ASSERT(self->sfa_inparen);
			while unlikely(self->sfa_exprtok == '!') {
#ifndef DEFINE_sfa_skipexpr
				invert = !invert;
#endif /* !DEFINE_sfa_skipexpr */
				sfa_yield(self);
			}
			switch (self->sfa_exprtok) {
			case SFA_TOK_D_IS:
			case SFA_TOK_D_IN:
#ifdef DEFINE_sfa_skipexpr
				goto do_sfa_evalcmp;
#else /* DEFINE_sfa_skipexpr */
				if (!invert)
					goto do_sfa_evalcmp;
				rhs = LOCAL_sfa_evalcmp(self);
				if unlikely(!LOCAL_ISOK(rhs))
					goto err_lhs;
				{
					int result_bool;
					switch (tok) {
					case SFA_TOK_D_IS: {
						if (DeeNone_Check(rhs)) {
							result_bool = DeeNone_Check(lhs);
						} else if (DeeSuper_Check(lhs)) {
							result_bool = DeeType_Extends(DeeSuper_TYPE(lhs), (DeeTypeObject *)rhs);
						} else {
							result_bool = DeeObject_InstanceOf(lhs, (DeeTypeObject *)rhs);
						}
					}	break;
					case SFA_TOK_D_IN:
						result_bool = DeeObject_ContainsAsBool(rhs, lhs);
						break;
					default: __builtin_unreachable();
					}
					Dee_Decref(rhs);
					Dee_Decref(lhs);
					if unlikely(result_bool < 0)
						goto err;
					lhs = DeeBool_For(!result_bool); /* Invert here! */
					Dee_Incref(lhs);
				}
				break;
#endif /* !DEFINE_sfa_skipexpr */
			default:
				self->sfa_parser.sfp_iter = orig;
				goto done;
			}

#ifndef DEFINE_sfa_skipexpr
		} else if (tok == SFA_TOK_QMARK_QMARK) {
			if (DeeNone_Check(lhs)) {
				Dee_Decref(lhs);
				lhs = LOCAL_sfa_evalcmp(self);
				if unlikely(!lhs)
					goto err;
			} else {
				if unlikely(sfa_skipcmp(self))
					goto err_lhs;
			}
#endif /* !DEFINE_sfa_skipexpr */
		} else {
do_sfa_evalcmp:
			rhs = LOCAL_sfa_evalcmp(self);
			if unlikely(!LOCAL_ISOK(rhs))
				goto err_lhs;
#ifndef DEFINE_sfa_skipexpr
			{
				LOCAL_return_type result;
				switch (tok) {
				case SFA_TOK_NOT_EQUAL:
					result = DeeObject_CmpNe(lhs, rhs);
					break;
				case SFA_TOK_EQUAL:
					result = DeeObject_CmpEq(lhs, rhs);
					break;
				case SFA_TOK_NOT_EQUAL3:
				case SFA_TOK_EQUAL3: {
					bool is_same = lhs == rhs;
					if (tok == SFA_TOK_NOT_EQUAL3)
						is_same = !is_same;
					result = DeeBool_For(is_same);
					Dee_Incref(result);
				}	break;
				case SFA_TOK_D_IS: {
					unsigned int is_instance;
					if (DeeNone_Check(rhs)) {
						is_instance = DeeNone_Check(lhs);
					} else if (DeeSuper_Check(lhs)) {
						is_instance = DeeType_Extends(DeeSuper_TYPE(lhs), (DeeTypeObject *)rhs);
					} else {
						is_instance = DeeObject_InstanceOf(lhs, (DeeTypeObject *)rhs);
					}
					result = DeeBool_For(is_instance);
					Dee_Incref(result);
				}	break;
				case SFA_TOK_D_IN:
					result = DeeObject_Contains(rhs, lhs);
					break;
				default: __builtin_unreachable();
				}
				Dee_Decref(rhs);
				Dee_Decref(lhs);
				lhs = result;
				if unlikely(!result)
					goto err;
			}
#endif /* !DEFINE_sfa_skipexpr */
		}
	} while (self->sfa_exprtok == SFA_TOK_EQUAL || self->sfa_exprtok == SFA_TOK_EQUAL3 ||
	         self->sfa_exprtok == SFA_TOK_QMARK_QMARK ||
	         self->sfa_exprtok == SFA_TOK_D_IS || self->sfa_exprtok == SFA_TOK_D_IN ||
	         (self->sfa_inparen && (self->sfa_exprtok == '!' ||
	                                self->sfa_exprtok == SFA_TOK_NOT_EQUAL ||
	                                self->sfa_exprtok == SFA_TOK_NOT_EQUAL3)));
done:
	return LOCAL_OK__or__lhs;
err_lhs:
#ifndef DEFINE_sfa_skipexpr
	Dee_Decref(lhs);
err:
#endif /* !DEFINE_sfa_skipexpr */
	return LOCAL_ERRVAL;
}

PRIVATE WUNUSED NONNULL((1)) LOCAL_return_type DFCALL
LOCAL_sfa_evalas_operand(struct string_format_advanced *__restrict self LOCAL__param_lhs) {
	LOCAL_return_type rhs;
	ASSERT(self->sfa_exprtok == SFA_TOK_D_AS);
	do {
		sfa_yield(self);
		rhs = LOCAL_sfa_evalcmpeq(self);
		if unlikely(!LOCAL_ISOK(rhs))
			goto err_lhs;
#ifndef DEFINE_sfa_skipexpr
		{
			LOCAL_return_type result;
			result = DeeSuper_New((DeeTypeObject *)rhs, lhs);
			Dee_Decref(rhs);
			Dee_Decref(lhs);
			lhs = result;
			if unlikely(!result)
				goto err;
		}
#endif /* !DEFINE_sfa_skipexpr */
	} while (self->sfa_exprtok == SFA_TOK_D_AS);
	return LOCAL_OK__or__lhs;
err_lhs:
#ifndef DEFINE_sfa_skipexpr
	Dee_Decref(lhs);
err:
#endif /* !DEFINE_sfa_skipexpr */
	return LOCAL_ERRVAL;
}

PRIVATE WUNUSED NONNULL((1)) LOCAL_return_type DFCALL
LOCAL_sfa_evaland_operand(struct string_format_advanced *__restrict self LOCAL__param_lhs) {
	LOCAL_return_type rhs;
	ASSERT(self->sfa_exprtok == '&');
	do {
		sfa_yield(self);
		rhs = LOCAL_sfa_evalas(self);
		if unlikely(!LOCAL_ISOK(rhs))
			goto err_lhs;
#ifndef DEFINE_sfa_skipexpr
		{
			LOCAL_return_type result;
			result = DeeObject_And(lhs, rhs);
			Dee_Decref(rhs);
			Dee_Decref(lhs);
			lhs = result;
			if unlikely(!result)
				goto err;
		}
#endif /* !DEFINE_sfa_skipexpr */
	} while (self->sfa_exprtok == '&');
	return LOCAL_OK__or__lhs;
err_lhs:
#ifndef DEFINE_sfa_skipexpr
	Dee_Decref(lhs);
err:
#endif /* !DEFINE_sfa_skipexpr */
	return LOCAL_ERRVAL;
}

PRIVATE WUNUSED NONNULL((1)) LOCAL_return_type DFCALL
LOCAL_sfa_evalxor_operand(struct string_format_advanced *__restrict self LOCAL__param_lhs) {
	LOCAL_return_type rhs;
	ASSERT(self->sfa_exprtok == '^');
	do {
		sfa_yield(self);
		rhs = LOCAL_sfa_evaland(self);
		if unlikely(!LOCAL_ISOK(rhs))
			goto err_lhs;
#ifndef DEFINE_sfa_skipexpr
		{
			LOCAL_return_type result;
			result = DeeObject_Xor(lhs, rhs);
			Dee_Decref(rhs);
			Dee_Decref(lhs);
			lhs = result;
			if unlikely(!result)
				goto err;
		}
#endif /* !DEFINE_sfa_skipexpr */
	} while (self->sfa_exprtok == '^');
	return LOCAL_OK__or__lhs;
err_lhs:
#ifndef DEFINE_sfa_skipexpr
	Dee_Decref(lhs);
err:
#endif /* !DEFINE_sfa_skipexpr */
	return LOCAL_ERRVAL;
}

PRIVATE WUNUSED NONNULL((1)) LOCAL_return_type DFCALL
LOCAL_sfa_evalor_operand(struct string_format_advanced *__restrict self LOCAL__param_lhs) {
	LOCAL_return_type rhs;
	ASSERT(self->sfa_exprtok == '|');
	do {
		sfa_yield(self);
		rhs = LOCAL_sfa_evalxor(self);
		if unlikely(!LOCAL_ISOK(rhs))
			goto err_lhs;
#ifndef DEFINE_sfa_skipexpr
		{
			LOCAL_return_type result;
			result = DeeObject_Or(lhs, rhs);
			Dee_Decref(rhs);
			Dee_Decref(lhs);
			lhs = result;
			if unlikely(!result)
				goto err;
		}
#endif /* !DEFINE_sfa_skipexpr */
	} while (self->sfa_exprtok == '|');
	return LOCAL_OK__or__lhs;
err_lhs:
#ifndef DEFINE_sfa_skipexpr
	Dee_Decref(lhs);
err:
#endif /* !DEFINE_sfa_skipexpr */
	return LOCAL_ERRVAL;
}

PRIVATE WUNUSED NONNULL((1)) LOCAL_return_type DFCALL
LOCAL_sfa_evalland_operand(struct string_format_advanced *__restrict self LOCAL__param_lhs) {
	ASSERT(self->sfa_exprtok == SFA_TOK_LAND);
	do {
#ifndef DEFINE_sfa_skipexpr
		int lhs_bool;
#endif /* !DEFINE_sfa_skipexpr */
		sfa_yield(self);
#ifdef DEFINE_sfa_skipexpr
		if unlikely(sfa_skipor(self))
			goto err;
#else /* DEFINE_sfa_skipexpr */
		lhs_bool = DeeObject_BoolInherited(lhs);
		if unlikely(lhs_bool < 0)
			goto err;
		if (!lhs_bool) {
			if unlikely(sfa_skipor(self))
				goto err;
			lhs = Dee_False;
		} else {
			lhs = sfa_evalor(self);
			if unlikely(!lhs)
				goto err;
			lhs_bool = DeeObject_BoolInherited(lhs);
			if unlikely(lhs_bool < 0)
				goto err;
			lhs = DeeBool_For(lhs_bool);
		}
		Dee_Incref(lhs);
#endif /* !DEFINE_sfa_skipexpr */
	} while (self->sfa_exprtok == SFA_TOK_LAND);
	return LOCAL_OK__or__lhs;
err:
	return LOCAL_ERRVAL;
}

PRIVATE WUNUSED NONNULL((1)) LOCAL_return_type DFCALL
LOCAL_sfa_evallor_operand(struct string_format_advanced *__restrict self LOCAL__param_lhs) {
	ASSERT(self->sfa_exprtok == SFA_TOK_LOR);
	do {
#ifndef DEFINE_sfa_skipexpr
		int lhs_bool;
#endif /* !DEFINE_sfa_skipexpr */
		sfa_yield(self);
#ifdef DEFINE_sfa_skipexpr
		if unlikely(sfa_skiplor(self))
			goto err;
#else /* DEFINE_sfa_skipexpr */
		lhs_bool = DeeObject_BoolInherited(lhs);
		if unlikely(lhs_bool < 0)
			goto err;
		if (lhs_bool) {
			if unlikely(sfa_skiplor(self))
				goto err;
			lhs = Dee_True;
		} else {
			lhs = sfa_evallor(self);
			if unlikely(!lhs)
				goto err;
			lhs_bool = DeeObject_BoolInherited(lhs);
			if unlikely(lhs_bool < 0)
				goto err;
			lhs = DeeBool_For(lhs_bool);
		}
		Dee_Incref(lhs);
#endif /* !DEFINE_sfa_skipexpr */
	} while (self->sfa_exprtok == SFA_TOK_LOR);
	return LOCAL_OK__or__lhs;
err:
	return LOCAL_ERRVAL;
}

PRIVATE WUNUSED NONNULL((1)) LOCAL_return_type DFCALL
LOCAL_sfa_evalcond_operand(struct string_format_advanced *__restrict self LOCAL__param_lhs) {
#ifndef DEFINE_sfa_skipexpr
	int lhs_bool;
#endif /* !DEFINE_sfa_skipexpr */
	LOCAL_return_type result;
	ASSERT(self->sfa_exprtok == '?');
#ifndef DEFINE_sfa_skipexpr
	lhs_bool = DeeObject_BoolInherited(lhs);
	if unlikely(lhs_bool < 0)
		goto err;
#endif /* !DEFINE_sfa_skipexpr */
	sfa_yield(self);
#ifdef DEFINE_sfa_skipexpr
	result = sfa_skiplor(self);
	if unlikely(!result)
		goto err;
	if unlikely(self->sfa_exprtok != ':')
		return sfa_err_bad_token(self, ":");
	sfa_yield(self);
	return sfa_skipcond(self);
#else /* DEFINE_sfa_skipexpr */
	if (lhs_bool) {
		result = sfa_evallor(self);
		if unlikely(!result)
			goto err;
		if unlikely(self->sfa_exprtok != ':') {
			sfa_err_bad_token(self, ":");
err_r:
			Dee_Decref(result);
			goto err;
		}
		sfa_yield(self);
		if unlikely(sfa_skipcond(self))
			goto err_r;
		return result;
	}
	if unlikely(sfa_skiplor(self))
		goto err;
	if unlikely(self->sfa_exprtok != ':') {
		sfa_err_bad_token(self, ":");
		goto err;
	}
	sfa_yield(self);
	return sfa_evalcond(self);
#endif /* !DEFINE_sfa_skipexpr */
err:
	return LOCAL_ERRVAL;
}



PRIVATE WUNUSED NONNULL((1)) LOCAL_return_type DFCALL
LOCAL_sfa_evalunary(struct string_format_advanced *__restrict self) {
	LOCAL_return_type result = LOCAL_sfa_evalunary_base(self);
	if (LOCAL_ISOK(result) && (self->sfa_exprtok == '.' ||
	                           self->sfa_exprtok == '[' ||
	                           self->sfa_exprtok == '(')) {
		result = LOCAL_sfa_evalunary_operand(self LOCAL__arg_lhs(result));
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) LOCAL_return_type DFCALL
LOCAL_sfa_evalprod(struct string_format_advanced *__restrict self) {
	LOCAL_return_type result = LOCAL_sfa_evalunary(self);
	if (LOCAL_ISOK(result) && (self->sfa_exprtok == '*' ||
	                           self->sfa_exprtok == '/' ||
	                           self->sfa_exprtok == '%' ||
	                           self->sfa_exprtok == SFA_TOK_POW)) {
		result = LOCAL_sfa_evalprod_operand(self LOCAL__arg_lhs(result));
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) LOCAL_return_type DFCALL
LOCAL_sfa_evalsum(struct string_format_advanced *__restrict self) {
	LOCAL_return_type result = LOCAL_sfa_evalprod(self);
	if (LOCAL_ISOK(result) && (self->sfa_exprtok == '+' ||
	                           self->sfa_exprtok == '-')) {
		result = LOCAL_sfa_evalsum_operand(self LOCAL__arg_lhs(result));
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) LOCAL_return_type DFCALL
LOCAL_sfa_evalshift(struct string_format_advanced *__restrict self) {
	LOCAL_return_type result = LOCAL_sfa_evalsum(self);
	if (LOCAL_ISOK(result) && (self->sfa_exprtok == SFA_TOK_SHL ||
	                           self->sfa_exprtok == SFA_TOK_SHR)) {
		result = LOCAL_sfa_evalshift_operand(self LOCAL__arg_lhs(result));
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) LOCAL_return_type DFCALL
LOCAL_sfa_evalcmp(struct string_format_advanced *__restrict self) {
	LOCAL_return_type result = LOCAL_sfa_evalshift(self);
	if (LOCAL_ISOK(result) && (self->sfa_exprtok == '<' ||
	                           self->sfa_exprtok == '>' ||
	                           self->sfa_exprtok == SFA_TOK_LOWER_EQUAL ||
	                           self->sfa_exprtok == SFA_TOK_GREATER_EQUAL)) {
		result = LOCAL_sfa_evalcmp_operand(self LOCAL__arg_lhs(result));
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) LOCAL_return_type DFCALL
LOCAL_sfa_evalcmpeq(struct string_format_advanced *__restrict self) {
	LOCAL_return_type result = LOCAL_sfa_evalcmp(self);
	if (LOCAL_ISOK(result)) {
		switch (self->sfa_exprtok) {
		case '!':
		case SFA_TOK_NOT_EQUAL:
		case SFA_TOK_NOT_EQUAL3:
			if (!self->sfa_inparen)
				break;
			ATTR_FALLTHROUGH
		case SFA_TOK_EQUAL:
		case SFA_TOK_EQUAL3:
		case SFA_TOK_QMARK_QMARK:
		case SFA_TOK_D_IS:
		case SFA_TOK_D_IN:
			result = LOCAL_sfa_evalcmpeq_operand(self LOCAL__arg_lhs(result));
			break;
		default:
			break;
		}
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) LOCAL_return_type DFCALL
LOCAL_sfa_evaland(struct string_format_advanced *__restrict self) {
	LOCAL_return_type result = LOCAL_sfa_evalcmpeq(self);
	if (LOCAL_ISOK(result) && self->sfa_exprtok == '&')
		result = LOCAL_sfa_evaland_operand(self LOCAL__arg_lhs(result));
	return result;
}

PRIVATE WUNUSED NONNULL((1)) LOCAL_return_type DFCALL
LOCAL_sfa_evalxor(struct string_format_advanced *__restrict self) {
	LOCAL_return_type result = LOCAL_sfa_evaland(self);
	if (LOCAL_ISOK(result) && self->sfa_exprtok == '^')
		result = LOCAL_sfa_evalxor_operand(self LOCAL__arg_lhs(result));
	return result;
}

PRIVATE WUNUSED NONNULL((1)) LOCAL_return_type DFCALL
LOCAL_sfa_evalor(struct string_format_advanced *__restrict self) {
	LOCAL_return_type result = LOCAL_sfa_evalxor(self);
	if (LOCAL_ISOK(result) && self->sfa_exprtok == '|')
		result = LOCAL_sfa_evalor_operand(self LOCAL__arg_lhs(result));
	return result;
}

PRIVATE WUNUSED NONNULL((1)) LOCAL_return_type DFCALL
LOCAL_sfa_evalas(struct string_format_advanced *__restrict self) {
	LOCAL_return_type result = LOCAL_sfa_evalor(self);
	if (LOCAL_ISOK(result) && self->sfa_exprtok == SFA_TOK_D_AS)
		result = LOCAL_sfa_evalas_operand(self LOCAL__arg_lhs(result));
	return result;
}

PRIVATE WUNUSED NONNULL((1)) LOCAL_return_type DFCALL
LOCAL_sfa_evalland(struct string_format_advanced *__restrict self) {
	LOCAL_return_type result = LOCAL_sfa_evalor(self);
	if (LOCAL_ISOK(result) && self->sfa_exprtok == SFA_TOK_LAND)
		result = LOCAL_sfa_evalland_operand(self LOCAL__arg_lhs(result));
	return result;
}

PRIVATE WUNUSED NONNULL((1)) LOCAL_return_type DFCALL
LOCAL_sfa_evallor(struct string_format_advanced *__restrict self) {
	LOCAL_return_type result = LOCAL_sfa_evalland(self);
	if (LOCAL_ISOK(result) && self->sfa_exprtok == SFA_TOK_LOR)
		result = LOCAL_sfa_evallor_operand(self LOCAL__arg_lhs(result));
	return result;
}

PRIVATE WUNUSED NONNULL((1)) LOCAL_return_type DFCALL
LOCAL_sfa_evalcond(struct string_format_advanced *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	LOCAL_return_type result = LOCAL_sfa_evallor(self);
	if (LOCAL_ISOK(result) && self->sfa_exprtok == '?')
		result = LOCAL_sfa_evalcond_operand(self LOCAL__arg_lhs(result));
	return result;
#else /* __OPTIMIZE_SIZE__ */
	LOCAL_return_type result = LOCAL_sfa_evalunary_base(self);
	if (LOCAL_ISOK(result)) {
		switch (self->sfa_exprtok) {
		case '.':
		case '[':
		case '(':
			result = LOCAL_sfa_evalunary_operand(self LOCAL__arg_lhs(result));
			if unlikely(!LOCAL_ISOK(result))
				goto done;
			if (self->sfa_exprtok == '*' || self->sfa_exprtok == '/' ||
			    self->sfa_exprtok == '%' || self->sfa_exprtok == SFA_TOK_POW) {
		case '*':
		case '/':
		case '%':
		case SFA_TOK_POW:
				result = LOCAL_sfa_evalprod_operand(self LOCAL__arg_lhs(result));
				if unlikely(!LOCAL_ISOK(result))
					goto done;
			}
			if (self->sfa_exprtok == '+' || self->sfa_exprtok == '-') {
		case '+':
		case '-':
				result = LOCAL_sfa_evalsum_operand(self LOCAL__arg_lhs(result));
				if unlikely(!LOCAL_ISOK(result))
					goto done;
			}
			if (self->sfa_exprtok == SFA_TOK_SHL || self->sfa_exprtok == SFA_TOK_SHR) {
		case SFA_TOK_SHL:
		case SFA_TOK_SHR:
				result = LOCAL_sfa_evalshift_operand(self LOCAL__arg_lhs(result));
				if unlikely(!LOCAL_ISOK(result))
					goto done;
			}
			if (self->sfa_exprtok == '<' || self->sfa_exprtok == '>' ||
			    self->sfa_exprtok == SFA_TOK_LOWER_EQUAL ||
			    self->sfa_exprtok == SFA_TOK_GREATER_EQUAL) {
		case '<':
		case '>':
		case SFA_TOK_LOWER_EQUAL:
		case SFA_TOK_GREATER_EQUAL:
				result = LOCAL_sfa_evalcmp_operand(self LOCAL__arg_lhs(result));
				if unlikely(!LOCAL_ISOK(result))
					goto done;
			}
			if (self->sfa_exprtok == SFA_TOK_EQUAL || self->sfa_exprtok == SFA_TOK_EQUAL3 ||
			    self->sfa_exprtok == SFA_TOK_QMARK_QMARK ||
			    self->sfa_exprtok == SFA_TOK_D_IS || self->sfa_exprtok == SFA_TOK_D_IN ||
			    (self->sfa_inparen && (self->sfa_exprtok == '!' ||
			                           self->sfa_exprtok == SFA_TOK_NOT_EQUAL ||
			                           self->sfa_exprtok == SFA_TOK_NOT_EQUAL3))) {
				__IF0 {
		case '!':
		case SFA_TOK_NOT_EQUAL:
		case SFA_TOK_NOT_EQUAL3:
					if (!self->sfa_inparen)
						break;
				}
		case SFA_TOK_EQUAL:
		case SFA_TOK_EQUAL3:
		case SFA_TOK_QMARK_QMARK:
		case SFA_TOK_D_IS:
		case SFA_TOK_D_IN:
				result = LOCAL_sfa_evalcmpeq_operand(self LOCAL__arg_lhs(result));
				if unlikely(!LOCAL_ISOK(result))
					goto done;
			}
			if (self->sfa_exprtok == '&') {
		case '&':
				result = LOCAL_sfa_evaland_operand(self LOCAL__arg_lhs(result));
				if unlikely(!LOCAL_ISOK(result))
					goto done;
			}
			if (self->sfa_exprtok == '^') {
		case '^':
				result = LOCAL_sfa_evalxor_operand(self LOCAL__arg_lhs(result));
				if unlikely(!LOCAL_ISOK(result))
					goto done;
			}
			if (self->sfa_exprtok == '|') {
		case '|':
				result = LOCAL_sfa_evalor_operand(self LOCAL__arg_lhs(result));
				if unlikely(!LOCAL_ISOK(result))
					goto done;
			}
			if (self->sfa_exprtok == SFA_TOK_D_AS) {
		case SFA_TOK_D_AS:
				result = LOCAL_sfa_evalas_operand(self LOCAL__arg_lhs(result));
				if unlikely(!LOCAL_ISOK(result))
					goto done;
			}
			if (self->sfa_exprtok == SFA_TOK_LAND) {
		case SFA_TOK_LAND:
				result = LOCAL_sfa_evalland_operand(self LOCAL__arg_lhs(result));
				if unlikely(!LOCAL_ISOK(result))
					goto done;
			}
			if (self->sfa_exprtok == SFA_TOK_LOR) {
		case SFA_TOK_LOR:
				result = LOCAL_sfa_evallor_operand(self LOCAL__arg_lhs(result));
				if unlikely(!LOCAL_ISOK(result))
					goto done;
			}
			if (self->sfa_exprtok == '?') {
		case '?':
				result = LOCAL_sfa_evalcond_operand(self LOCAL__arg_lhs(result));
				/*if unlikely(!LOCAL_ISOK(result))
					goto done;*/
			}
			break;
		default: break;
		}
	}
done:
	return result;
#endif /* !__OPTIMIZE_SIZE__ */
}


DECL_END

#undef DEFINE_sfa_skipexpr
#undef DEFINE_sfa_evalexpr

#undef LOCAL_ERRVAL
#undef LOCAL_OK__or__lhs
#undef LOCAL_ISOK
#undef LOCAL_return_type
#undef LOCAL__param_lhs
#undef LOCAL__arg_lhs

#undef LOCAL_sfa_evalunary_base
#undef LOCAL_sfa_evalunary
#undef LOCAL_sfa_evalprod
#undef LOCAL_sfa_evalsum
#undef LOCAL_sfa_evalshift
#undef LOCAL_sfa_evalcmp
#undef LOCAL_sfa_evalcmpeq
#undef LOCAL_sfa_evaland
#undef LOCAL_sfa_evalxor
#undef LOCAL_sfa_evalor
#undef LOCAL_sfa_evalas
#undef LOCAL_sfa_evalland
#undef LOCAL_sfa_evallor
#undef LOCAL_sfa_evalcond
#undef LOCAL_sfa_evalunary_operand
#undef LOCAL_sfa_evalprod_operand
#undef LOCAL_sfa_evalsum_operand
#undef LOCAL_sfa_evalshift_operand
#undef LOCAL_sfa_evalcmp_operand
#undef LOCAL_sfa_evalcmpeq_operand
#undef LOCAL_sfa_evaland_operand
#undef LOCAL_sfa_evalxor_operand
#undef LOCAL_sfa_evalor_operand
#undef LOCAL_sfa_evalas_operand
#undef LOCAL_sfa_evalland_operand
#undef LOCAL_sfa_evallor_operand
#undef LOCAL_sfa_evalcond_operand
