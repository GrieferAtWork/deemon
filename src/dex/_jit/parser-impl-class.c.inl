/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifdef __INTELLISENSE__
#include "parser-impl.c.inl"
#endif /* __INTELLISENSE__ */

#include <deemon/util/objectlist.h>

DECL_BEGIN

/* Parse a class declaration, and return the produced class type.
 * Parsing starts after the `class' (or `class final'), meaning
 * that the current token is either:
 *   - `extends', `:' or `('     (followed by the class's base-type)
 *   - A keyword                 (the class name)
 *   - '{'                       (Start of the class body)
 * @param: tp_flags: Set of `0 | TP_FFINAL' */
#ifdef JIT_EVAL
INTERN WUNUSED DREF DeeTypeObject *FCALL
JITLexer_EvalClass(JITLexer *__restrict self, uint16_t tp_flags)
#else /* JIT_EVAL */
INTERN int FCALL
JITLexer_SkipClass(JITLexer *__restrict self)
#endif /* !JIT_EVAL */
{
#ifdef JIT_EVAL
	DREF DeeTypeObject *result;
	DREF DeeTypeObject *cls_base;
	char const *cls_tpname_start;
	char const *cls_tpname_end;
	cls_base         = NULL;
	cls_tpname_start = NULL;
	cls_tpname_end   = NULL;
#endif /* JIT_EVAL */

	/* Check for a class name. */
	if (self->jl_tok == JIT_KEYWORD && !JITLexer_ISTOK(self, "extends")) {
#ifdef JIT_EVAL
		cls_tpname_start = (char const *)self->jl_tokstart;
		cls_tpname_end   = (char const *)self->jl_tokend;
#endif /* JIT_EVAL */
		JITLexer_Yield(self);
	}

	/* Check for a class base declaration. */
	if (self->jl_tok == ':' || JITLexer_ISKWD(self, "extends")) {
		JITLexer_Yield(self);
#ifdef JIT_EVAL
		cls_base = (DREF DeeTypeObject *)JITLexer_EvalUnaryHead(self, JITLEXER_EVAL_FDISALLOWCAST);
		if unlikely(!cls_base)
			goto err;
		if (DeeObject_AssertType(cls_base, &DeeType_Type))
			goto err;
#else /* JIT_EVAL */
		if (JITLexer_SkipUnaryHead(self, JITLEXER_EVAL_FDISALLOWCAST))
			goto err;
#endif /* !JIT_EVAL */
	} else if (self->jl_tok == '(') {
		JITLexer_Yield(self);
#ifdef JIT_EVAL
		cls_base = (DREF DeeTypeObject *)JITLexer_EvalExpression(self, JITLEXER_EVAL_FDISALLOWCAST);
		if unlikely(!cls_base)
			goto err;
		if (DeeObject_AssertType(cls_base, &DeeType_Type))
			goto err;
		if unlikely(self->jl_tok != ')') {
			syn_class_expected_rparen_after_lparen_base(self);
			goto err;
		}
		JITLexer_Yield(self);
#else /* JIT_EVAL */
		if (JITLexer_SkipPair(self, '(', ')'))
			goto err;
#endif /* !JIT_EVAL */
	}

	/* Consume the leading '{'-token */
	if unlikely(self->jl_tok != '{') {
		syn_class_expected_lbrace_after_class(self);
		goto err;
	}
	JITLexer_Yield(self);

#ifdef JIT_EVAL
	/* NOTE: We can (easily) handle class-scope variables by:
	 *  - First parsing the class's entire body, and creating
	 *    unbound symbols for every member/function/property
	 *    that will end up getting declared
	 *  - Scanning the bodies of member-initializers/property
	 *    callbacks and functions during a second pass.
	 *    At that point, we'll already know all of the names that
	 *    are being declared inside of the class, and thus can
	 *    select how to link symbols (all class-scope symbols will
	 *    be linked as class member accessors, as in:
	 *    class MyClass {
	 *        member foo;
	 *        static member bar;
	 *        function meth() {
	 *            foo = 42;    // >> Object.__itable__(this as MyClass)[<indexof(foo)>] = 42;
	 *            return bar;  // >> return Type.__ctable__(MyClass)[<indexof(bar)>];
	 *        }
	 *    }
	 */

	/* TODO: Create the class descriptor that will be returned by this function */

	/* TODO: Parse the class declaration body, and modify/extend the to-be returned
	 *       class type. For every member initializer and property/function body,
	 *       remember where it starts/ends for later, but already allocate slots
	 *       for all of them in the class descriptor's class-object-table. */

	/* TODO: Once the new class descriptor has been finalized, make use of
	 *       `DeeClass_New()' in order to create the to-be returned type object. */

	/* TODO: Create a new scope and:
	 *    - Fill it with `JIT_OBJECT_ENTRY_TYPE_ATTR'-entires for every non-static
	 *      class member, as well as `JIT_OBJECT_ENTRY_TYPE_ATTR_FIXED'-entries for
	 *      every static class member. (in the later case, use the to-be returned
	 *      class type as the bound object)
	 *    - Define a local variable `JIT_RTSYM_CLASS', and store the to-be returned
	 *      class inside. This variable is needed when class members make use of `super',
	 *      in which case it is expected to hold the surrounding class's base-type.
	 */

	/* TODO: If present, do custom processing for `CLASS_OPERATOR_SUPERARGS' */
	/* TODO: If present, do custom processing for `OPERATOR_CONSTRUCTOR'
	 *       Note that in this case, it may be necessary to generate additional
	 *       code in order to initialize class member prior to the normal construct
	 *       being invoked. - This should be quite easy to do, as we can simply
	 *       use a unicode_printer to build the auto-generated portion of the ctor.
	 *       This would also solve the custom handling that's required for custom
	 *       member initialization at the start of a constructor, as in:
	 *       >> this(): foo(42), bar(17), baz = "hello" { ... }
	 *       Which we must execute as:
	 *       >> this() { foo = 42; bar = 17; baz = "hello"; ... } */

	/* TODO: Use `JITFunction_New()' (with `JIT_FUNCTION_FTHISCALL' for non-static functions)
	 *       to construct all of the member functions needed. At the same time (as they're
	 *       being created), use these newly created functions to fill in the class-object-
	 *       table of the to-be returned class type. */

	/* TODO: Pop the class-scope pushed above. */

	(void)cls_tpname_start;
	(void)cls_tpname_end;
	(void)tp_flags;
	result = NULL;
	DeeError_NOTIMPLEMENTED();
	if (!result)
		goto err;

	/* Consume the trailing '}'-token */
	if unlikely(self->jl_tok != '}') {
		syn_class_expected_rbrace_after_class(self);
		goto err;
	}
	JITLexer_Yield(self);
#else /* JIT_EVAL */
	/* Just skip to the end of the class declaration :) */
	if (JITLexer_SkipPair(self, '{', '}'))
		goto err;
#endif /* !JIT_EVAL */

#ifdef JIT_EVAL
	Dee_XDecref(cls_base);
	return result;
#else /* JIT_EVAL */
	return 0;
#endif /* !JIT_EVAL */
err:
#ifdef JIT_EVAL
	Dee_XDecref(cls_base);
#endif /* JIT_EVAL */
	return ERROR;
}

DECL_END
