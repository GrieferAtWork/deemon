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
#ifndef GUARD_DEEMON_COMPILER_SYMBOL_H
#define GUARD_DEEMON_COMPILER_SYMBOL_H 1

#include "../api.h"
#include "../code.h"
#include "../object.h"

#ifdef CONFIG_BUILDING_DEEMON
#include "tpp.h"
#ifndef CONFIG_NO_THREADS
#include <hybrid/__atomic.h>
#endif /* !CONFIG_NO_THREADS */
#endif /* CONFIG_BUILDING_DEEMON */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

DECL_BEGIN

typedef struct scope_object       DeeScopeObject;
typedef struct class_scope_object DeeClassScopeObject;
typedef struct base_scope_object  DeeBaseScopeObject;
typedef struct root_scope_object  DeeRootScopeObject;

struct TPPKeyword;
struct ast;
struct string_object;
struct module_object;
struct asm_sym;
struct class_attribute;

/* Declaration information encoded in documentation strings:
 *
 * Special documentation information is encoded line-wise, with line-feeds allowed to be
 * escaped by prefixing them with a \ character which is later removed. As a matter of
 * fact: Any special character can be escaped by prefixing it with \, with the following
 * being a list of all special characters:
 *    _ _ _ _ _ _ _ _ _ _ _ _ _ _ __ __ __ ____
 *    \ ? ! { } | , < > ( ) [ ] = -> \n \r \r\n
 *
 * Additionally, the following characters will only be escaped when appearing in certain
 * situations:
 *
 *    '\'  Always (as r"\\")
 *    '>'  When encountered after a '-'
 *    '('  When encountered at the start of a line
 *    <LF> When immediately preceded by another LF, such that "\n\n\n" would be escaped as "\n\\\n\\\n"
 *
 * None of the other characters will be escaped for user-defined documentation strings, as
 * only the special character sequences "<START_OF_LINE>(" and "->" are accepted as indices
 * of the (potentially \<LF> - wrapped) line containing declaration information.
 *
 * Since declaration information may also contain user-defined identifier names, which in turn
 * may also contain any of the aforementioned special characters, all of them are escaped by
 * prefixing each of them with a \ character in that context (see the syntax for declaration
 * information below)
 *
 * Declaration information formats:
 *
 * Pattern: "<START_OF_LINE>("
 *    The line describes argument name, and return type information:
 *    NOTE: _NO_ whitespace may appear between individual strings
 *        "<START_OF_LINE>(" ("," ~~ (
 *            ["<ARGUMENT_NAME>"]
 *            ["?" | "!" | "!!"]           // Optional / Varargs / Kwds indicators
 *            [":" "<ARGUMENT_TYPE>"]      // Type defaults to "Object" (encoded as "?O")
 *            ["=" ["<ARGUMENT_DEFAULT>"]]
 *        )...) ")" ["->" "<RETURN_TYPE>"] "<END_OF_LINE>"
 *    
 *    CONTENT-SPECIFIC:
 *      - When encountered in a function-doc, it contains information about the function's intended
 *        invocation. Unnamed arguments are automatically generated from abcdefghijklmnopqrstuvwxyz,
 *        after which naming continues as aa ab ac, etc... The chosen name is the first one
 *        that hasn't already been taken by another explicitly defined name, or another
 *        auto-generated one.
 *        >> ()       // function(): none { ... }
 *        >> (:)      // function(none): none { ... }
 *        >> (,)      // function(none, none): none { ... }
 *        >> (:?Dint) // function(none: int): none { ... }
 *        REMINDER: Using `none' as argument name sets the name to be undefined/reserved.
 *      - When encountered in a member-doc, the member is assumed to be a function
 *      - When encountered in a type-doc, it contains information about the type's constructor operator
 *    
 *    ARGUMENT_NAME: A user-defined identifier describing the name of an argument
 *                   This operand may be prefixed by one of "?", "!" or "!!" to describe
 *                   a variable amount of arguments being passed through this argument.
 *    ARGUMENT_TYPE: An encoded type description of the intended typing of the argument (see TYPE-ENCODING below)
 *    ARGUMENT_DEFAULT: An encoded description of an argument default value.
 *                      When omitted, argument default information is substituted from
 *                      code execution information, or left as undefined when the function
 *                      isn't implemented in user-code and thus doesn't offset execution
 *                      information. (see EXPR-ENCODING below)
 *    RETURN_TYPE:   An encoded type description of the intended return type of the function
 *                   When "->" is omitted, the programmer intended the function to return `none'
 *                   When "<RETURN_TYPE>" is omitted but "->" is present, the programmer
 *                   intended the function to return `Object' (aka anything / unspecified)
 *
 *
 * PATTERN: "->"  (Only when the line doesn't match the "<START_OF_LINE>(" pattern)
 *    The line describes the return type of an argument-less function,
 *    or the expected type of a field/member/variable/global/extern/etc.
 *    In type type-docs, it is used to define the prototype for a specific operator
 *    NOTE: optional whitespace may _NOT_ appear between individual strings
 *        "->" ["|" ~~ "<RETURN_TYPE>"...] "<END_OF_LINE>"
 *
 *    CONTENT-SPECIFIC:
 *      - When encountered in a function-doc, the argument list defaults
 *        to empty, such that "->?Dint" is the same as "<START_OF_LINE>()->?Dint"
 *      - When encountered in a member-doc, the member's occupant's intended type is described.
 *      - When encountered in a type-doc, an operator is described (See OPERATOR DECLARATION)
 *
 *    RETURN_TYPE: An encoded type description of the occupant's intended type.
 *                 When "<RETURN_TYPE>" is omitted, the programmer
 *                 intended the field to contain anything (`Object' / aka anything / unspecified)
 *
 *
 * OPERATOR DECLARATION:
 *    Operator information is described as a sub-set of the "->"-pattern, by defining the
 *    contents and meaning of whatever is contained within the line prior to "->" being encountered
 *    NOTE: optional whitespace may _NOT_ appear between individual strings
 *        "<OPERATOR_NAME>"
 *        ["(" ("," ~~ (
 *            ["<ARGUMENT_NAME>"]
 *            ["?" | "!" | "!!"]                  // Optional / Varargs / Kwds indicators
 *            [":" ["|" ~~ "<ARGUMENT_TYPE>"...]] // Type defaults to "Object" (encoded as "?O")
 *            ["=" "<ARGUMENT_DEFAULT>"]          // Default value (not allowed for optional, varargs, or Kwds arguments)
 *        )...) ")"]
 *        "->"    // Like seen in the "->" pattern
 *            ("<RETURN_TYPE>" |  // Operator return type
 *             // NOTE: The following 2 may only be used when no parameter list was given before
 *             "!D" |             // Indicator that the operator gets deleted (intended for `operator str = del;')
 *             "!S"               // Indicator that the operator is explicitly inherited (intended for `this = super;')
 *             )
 *        "<END_OF_LINE>"
 *    With this, operator declarations are split into 3 sections:
 *      - OPERATOR_NAME
 *      - OPTIONAL(ARGUMENT_LIST)
 *      - OPTIONAL(RETURN_TYPE)
 *    ARGUMENT_LIST is structured the same as seen in the "<START_OF_LINE>(" pattern, with the only
 *    difference being the special action taken when it is omitted (in which case it is default-generated
 *    depending on OPERATOR_NAME)
 *    RETURN_TYPE also behaves the same as in both the "<START_OF_LINE>(" and "->" patterns when
 *    dealing with a function-like declaration. The addition here being that it, too, is default-generated
 *    depending on OPERATOR_NAME when omitted.
 *
 *    OPERATOR_NAME is one of the following, with the other tables describing the default-generated
 *    values for ARGUMENT_LIST and RETURN_TYPE, where <TYPE> refers to the type for which the operator
 *    declaration is written
 *       OPERATOR_NAME         | DEFAULT
 *        copy, deepcopy       | (other:<TYPE>)
 *        ~this                | ()
 *        :=, move :=          | (other:<TYPE>)
 *        str, repr            | ()->?Dstring
 *        bool                 | ()->?Dbool
 *        next                 | ()->?O
 *        call                 | (args!:?O)->?O
 *        int, hash, #         | ()->?Dint
 *        float                | ()->?Dfloat
 *        ~                    | ()-><TYPE>
 *        +                    | ()-><TYPE>               (When argument count indicates 1 argument, default to (other:<TYPE>)-><TYPE>)
 *        -                    | ()-><TYPE>               (When argument count indicates 1 argument, default to (other:<TYPE>)-><TYPE>)
 *        *, /, %, &, |, ^, ** | (other:<TYPE>)-><TYPE>
 *        <<, >>               | (shift:?Dint)-><TYPE>
 *        ++, --               | ()-><TYPE>
 *        +=, -=, *=, /=, %=   | (other:<TYPE>)-><TYPE>
 *        &=, |=, ^=,          | (other:<TYPE>)-><TYPE>
 *        <<=, >>=             | (other:?Dint)-><TYPE>
 *        <, <=, >, >=, ==, != | (other:<TYPE>)->?Dbool
 *        iter                 | ()->?Aiterator<TYPE>
 *        contains             | (item)->?Dbool
 *        []                   | (index:?Dint)->?O
 *        del[]                | (index:?Dint)
 *        []=                  | (index:?Dint,value:?O)
 *        [:]                  | (start:?Dint,end:?Dint)->?S?O
 *        del[:]               | (start:?Dint,end:?Dint)
 *        [:]=                 | (start:?Dint,end:?Dint,values:?S?O)
 *        .                    | (attr:?Dstring)->?O
 *        del.                 | (attr:?Dstring)
 *        .=                   | (attr:?Dstring,value:?O)
 *        enumattr             | ()->?S?DAttribute
 *        enter, leave         | ()
 *    Alternatively to the operator's symbol-like name, one can also use the operators real name,
 *    optionally surrounded by any number of _ (underscores), such that `__le__' is the same as `<='
 *    Also note that missing argument type information is also automatically filled in when the type
 *    is known or guarantied to have a certain typing.
 *    This applies to all instances where a type other than `<TYPE>' is listed above, as well as in
 *    the following special cases:
 *       OPERATOR_NAME   | DEFAULT
 *        deepcopy, copy | (other:<TYPE>)
 *        move :=        | (other:<TYPE>)         // Only when the type doesn't have the `TP_FMOVEANY' flag set.
 *
 *
 *
 *
 *
 * Each pattern line is usually then followed by any number of regular lines containing human-written
 * documentation text (in user-code written as @@TEXT<LF> or @"TEXT"), which is then escaped as described
 * above by the section of character escaping only done in certain situations.
 * These sections of prototype docs are separated from each other by 2 consecutive line-feeds, such that
 * these sections can simply be separated from each other with `__doc__.unifylines().split("\n\n")'
 *
 * NOTE:
 *   - Sections may also appear without any associated declaration pattern, in which case
 *     the declaration pattern is automatically deduced form the documented object, or left
 *     omitted if no object is being documented:
 *       - For types, the doc is appended to the set of strings giving a generic overview of the type
 *       - For a `function foo(x, y, z)' the doc defaults to `(!a,!b,!c)->'
 *          - Since argument name information is lost during this, names are generated
 *            as abcdefghijklmnopqrstuvwxyz, after which naming continues as aa ab ac, etc...
 *       - For member/variables the declaration defaults to `->' (untyped, object)
 *   - Declarations can be grouped by having a section contain more than one line, matching either
 *     the "->" pattern, or the "<START_OF_LINE>(" one.
 *     This may be done to describe available overloads, with the associated human-readable
 *     texts then applying to 
 *     
 *
 *
 *
 * TYPE-ENCODING:  (How type/symbol references are encoded)
 *
 *   ?.              --- Referring to the current type in an operator or static/instance member.
 *   ?N              --- Referring to `none' (or in this context: `type none')
 *   ?O              --- Referring to `Object from deemon'
 *   ?#<NAME>        --- Referring to a field <DECODED_NAME> of the surrounding component (the type of a member/operator, or module or a global, etc., that is expected to contain the type at runtime)
 *   ?D<NAME>        --- Referring to a symbol exported from the `deemon' module (import("deemon").<DECODED_NAME>)
 *   ?U<NAME>        --- Referring to an undefined/private symbol
 *   ?G<NAME>        --- Referring to a global symbol exported from the associated module
 *   ?E<NAME>:<NAME> --- Referring to an external symbol (import("<DECODED_FIRST_NAME>").<DECODED_SECOND_NAME>)
 *   ?A<NAME><TYPE>  --- Referring to an attribute <DECODED_FIRST_NAME> of another TYPE-ENCODING <TYPE>
 *
 * Extended type encodings (Implemented by `doc.TypeExpr', rather than `doc.TypeRef'):
 *   ?C<TYPE><TYPE>    --- Referring to a Cell <FIRST_TYPE> containing an element of type <SECOND_TYPE> (SECOND_TYPE
 *                         is only there to improve meta-information, whilst FIRST_TYPE should implement an instance-
 *                         attribute `value', with when accessed should yield an element of <SECOND_TYPE>)
 *                         This type of encoding is used to represent `WeakRef with Object',
 *                        `TLS with Object' or `Cell with Object'
 *   ?T<N>(<TYPE> * N) --- A Tuple expression containing <N> (encoded as a decimal) other types
 *                         e.g.: `?T2?Dstring?Dint' --- `(string, int)'
 *   ?X<N>(<TYPE> * N) --- A set of <N> (encoded as a decimal) alternative type representations
 *                         e.g.: `?X2?Dstring?Dint' --- `string | int'
 *   ?S<TYPE>          --- A generic Sequence expression for <TYPE>
 *                         e.g.: `?S?Dstring' --- `{string...}'
 *   ?M<TYPE><TYPE>    --- A generic Mapping expression for <TYPE> to <TYPE>
 *                         e.g.: `?M?Dstring?Dint' --- `{string: int}'
 *   ?R<EXPR>]         --- Referring to the result of a given <EXPR> can take on (yes: the trailing `]' is intended)
 *   ?Q<EXPR>]         --- Referring to the types which a given <EXPR> can take on (yes: the trailing `]' is intended)
 *
 *   In all of the aforementioned encodings, <NAME> is encoded as follows:
 *   >> if (!name.issymbol()) {
 *   >>     for (local x: r'\?!{}|,()<>[]=')
 *   >>          name = name.replace(x, r'\' + x);
 *   >>     name = name.replace(r"->", r"-\>");
 *   >>     name = name.replace("\n", "\\\n"); // For any type of line-feed
 *   >>     name = "{" + name + "}";
 *   >> }
 *   Decoding then happens in the reverse direction, decoding \ escape sequences
 *
 *   Any encoded type not starting with one of the disclosed prefixed is considered
 *   to be a legacy-doc symbol reference and handle in a special manner that is not
 *   documented here and should be considered deprecated.
 *
 *
 *
 * EXPR-ENCODING:  (How (simple) expressions are encoded)
 *   
 *   <EXPR>     --- Where `<EXPR>' contains the actual expression, which is encoded as follows:
 *
 *   <EXPR> ::= "!" (
 *       INTEGER_LITERAL |          // 1234 (decimal), 0x12/0X12 (hex), 012 (oct) or 0b10/0B10 (bin)
 *       FLOAT_LITERAL |            // 1.0  (decimal followed by a `.'; prevent ambiguity with decimal-operator)
 *       "t"                        // The true builtin constant
 *       "f"                        // The false builtin constant
 *       "A" <NAME>                 // Referring to another argument <DECODED_NAME>
 *       "A" EXPR EXPR |            // FIRST_EXPR.operator . (SECOND_EXPR)
 *       "B" EXPR EXPR |            // boundattr(FIRST_EXPR, SECOND_EXPR)
 *       "B" <NAME>                 // Referring to true/false indicative of the is-bound state of another argument <DECODED_NAME>
 *       "C" EXPR |                 // copy(EXPR)
 *       "D" <NAME>                 // Referring to a symbol exported from the `deemon' module (import("deemon").<DECODED_NAME>)
 *       "E" <NAME> ":" <NAME>      // Referring to an external symbol (import("<DECODED_FIRST_NAME>").<DECODED_SECOND_NAME>)
 *       "G" <NAME>                 // Referring to a global symbol exported from the associated module
 *       "H<N>" ((EXPR)... * <N>) | // DICT(EXPR * <N>)    A Dict of <N> (encoded as a decimal) elements (every first is a key, every second is the associated value)
 *       "I" EXPR EXPR EXPR |       // FIRST_EXPR ? SECOND_EXPR : THIRD_EXPR  (Non-syntax errors encountered within the dead branch are ignored)
 *       "K" EXPR |                 // str(EXPR)
 *       "L<N>" ((EXPR)... * <N>) | // LIST(EXPR * <N>)    A List of <N> (encoded as a decimal) elements
 *       "M" <NAME>                 // Referring to a module  (import("<DECODED_NAME>"))
 *       "N"                        // The none builtin constant
 *       "O" EXPR |                 // type(EXPR)
 *       "P" <NAME> |               // "foo" (where `foo' is en-/decoded the same way a <NAME> would)
 *       "Q" EXPR |                 // EXPR.operator hash()
 *       "R" EXPR |                 // repr(EXPR)
 *       "S" EXPR EXPR |            // FIRST_EXPR is SECOND_EXPR
 *       "S<N>" ((EXPR)... * <N>) | // HASHSET(EXPR * <N>) A Set of <N> (encoded as a decimal) elements
 *       "T<N>" ((EXPR)... * <N>) | // TUPLE(EXPR * <N>)   A Tuple of <N> (encoded as a decimal) elements
 *       "U" <NAME>                 // Referring to an undefined/private symbol (in expressions, `none' is used, but if produced as result, it's name is used; getattr() also extends ontop of it)
 *       "W" EXPR |                 // #(EXPR)
 *       "X" EXPR |                 // deepcopy(EXPR)
 *       "#" <NAME>                 // Referring to a field <DECODED_NAME> of the surrounding component (the type of a member/operator, or module or a global, etc., that is expected to contain the type at runtime)
 *       "!!!" EXPR |               // !(EXPR)
 *       "+" EXPR |                 // +EXPR
 *       "-" EXPR |                 // -EXPR
 *       "-" INTEGER_LITERAL |      // -INTEGER_LITERAL
 *       "-" FLOAT_LITERAL |        // -FLOAT_LITERAL
 *       "!+" EXPR EXPR |           // FIRST_EXPR + SECOND_EXPR
 *       "!-" EXPR EXPR |           // FIRST_EXPR - SECOND_EXPR
 *       "~" EXPR |                 // ~EXPR
 *       "*" EXPR EXPR |            // FIRST_EXPR * SECOND_EXPR
 *       "/" EXPR EXPR |            // FIRST_EXPR / SECOND_EXPR
 *       "%" EXPR EXPR |            // FIRST_EXPR % SECOND_EXPR
 *       "!!<" EXPR EXPR |          // FIRST_EXPR << SECOND_EXPR
 *       "!!>" EXPR EXPR |          // FIRST_EXPR >> SECOND_EXPR
 *       "&" EXPR EXPR |            // FIRST_EXPR & SECOND_EXPR
 *       "|" EXPR EXPR |            // FIRST_EXPR | SECOND_EXPR
 *       "^" EXPR EXPR |            // FIRST_EXPR ^ SECOND_EXPR
 *       "!*" EXPR EXPR |           // FIRST_EXPR ** SECOND_EXPR
 *       "=" EXPR EXPR |            // FIRST_EXPR == SECOND_EXPR
 *       "!=" EXPR EXPR |           // FIRST_EXPR != SECOND_EXPR
 *       "<" EXPR EXPR |            // FIRST_EXPR < SECOND_EXPR
 *       ">" EXPR EXPR |            // FIRST_EXPR > SECOND_EXPR
 *       "!<" EXPR EXPR |           // FIRST_EXPR <= SECOND_EXPR
 *       "!>" EXPR EXPR |           // FIRST_EXPR >= SECOND_EXPR
 *       "!C" EXPR EXPR |           // FIRST_EXPR.operator contains(SECOND_EXPR)
 *       "[" EXPR EXPR |            // FIRST_EXPR[SECOND_EXPR]
 *       "![" EXPR EXPR EXPR |      // FIRST_EXPR[SECOND_EXPR:THIRD_EXPR]
 *   );
 *
 *
 *   NOTE: Whitespace may not appear with encoded expression and its presence (outside
 *         of a string literal) is considered to be a syntax error.
 *   EXAMPLES:
 *       !Pfoo           // string("foo")
 *       !!+!1!2         // int(1) + int(2)
 *       !C!L2!N!N!Ai    // i in list([none, none])
 *       !A!1.0!P{a-\>b} // float(1.0).operator getattr("a->b")
 *
 *
 *   Any encoded default expression not starting with one of the disclosed prefixed
 *   must be a simple, constant expression and may not reference other arguments,
 *   meaning that exclusively one of the following is allowed:
 *     none    --- The none builtin
 *     true    --- The true builtin
 *     false   --- The false builtin
 *     1234    --- Integer constant    (also accepted with radix prefix `0x', `0X', `0b', `0B')
 *     1.2     --- Float constant
 *     "foo"   --- String constant     (where `foo' is decoded the same way a <NAME> would, before being decoded again as a C-escaped string)
 *     r"foo"  --- String constant     (where `foo' is decoded the same way a <NAME> would, before being decoded again as a raw string literal)
 *
 */




#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
#define DAST_NONE    0x0000 /* No declaration information. */
#define DAST_SYMBOL  0x0001 /* `int from deemon' Declaration information is provided as a symbol reference. */
#define DAST_CONST   0x0002 /* `type(0)' Declaration information is provided as a constant type. */
#define DAST_ALT     0x0003 /* `int | bool' Declaration information has multiple, alternative representations. */
#define DAST_TUPLE   0x0004 /* `(int, string, float)' Declaration describes an n-element Tuple of values. */
#define DAST_SEQ     0x0005 /* `{int...}' Declaration describes a variable-length sequence of some element-type. */
#define DAST_MAP     0x0006 /* `{string: int}' Declaration describes an abstract mapping-type */
#define DAST_FUNC    0x0007 /* `(x: int, y: int): int' Declaration describes a variable-length sequence of some element-type. */
#define DAST_ATTR    0x0008 /* `List.Iterator' Access a custom attribute of another declaration. */
#define DAST_WITH    0x0009 /* `WeakRef with Object' Extended type information to describe Cell-like objects */
#define DAST_STRING  0x000a /* __asm__("?T2?O?O") Custom string inserted into the representation. */

#define DAST_FNORMAL 0x0000 /* Normal declaration ast flags */

struct decl_ast {
	uint16_t     da_type;  /* Decl AST Type (One of `DAST_*') */
	uint16_t     da_flag;  /* Decl AST Flags (Set of `DAST_F*') */
	union {
		DREF struct symbol *da_symbol; /* [1..1][DAST_SYMBOL] The referenced type expression symbol. */
		DREF DeeObject     *da_const;  /* [1..1][DAST_CONST] A constant expression type. */
		struct {
			size_t          a_altc;    /* Amount of alternative representations. */
			struct decl_ast*a_altv;    /* [0..a_altc][owned] Vector of alternative representations. */
		}                   da_alt;    /* [DAST_ALT] One of many different representations is acceptable. */
		struct {
			size_t          t_itemc;   /* Amount of Tuple elements. */
			struct decl_ast*t_itemv;   /* [0..a_altc][owned] Vector of Tuple elements. */
		}                   da_tuple;  /* [DAST_TUPLE] The representation is a fixed-length Tuple containing known types. */
		struct decl_ast    *da_seq;    /* [1..1][owned][DAST_SEQ] The sequence element */
		struct {
			struct decl_ast                *f_ret;   /* [0..1][owned] Function return type (or `NULL' when `Object' or `none' is returned) */
			Dee_WEAKREF(DeeBaseScopeObject) f_scope; /* [1..1] The scope containing function argument info, as well as associated
			                                          * type declaration information (through `struct symbol::s_decltype') */
		}                   da_func;   /* [DAST_FUNC] The representation is a function. */
		struct {
			struct decl_ast           *a_base; /* [1..1][owned] Attribute base expression. */
			DREF struct string_object *a_name; /* [1..1] Attribute name. */
		}                   da_attr;   /* [DAST_ATTR] The representation is the attribute of another expression. */
		struct {
			struct decl_ast *m_key_value; /* [2..2][owned] 0: The map key; 1: The map value. */
		}                   da_map;    /* [DAST_MAP] Representation for map-like containers. */
		struct {
			struct decl_ast *w_cell;   /* [2..2][owned] 0: The cell container; 1: The cell element. */
		}                   da_with;   /* [DAST_WITH] Representation for cell-like containers. */
		DREF struct string_object *da_string; /* [1..1][DAST_STRING] Custom string. */
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define da_symbol _dee_aunion.da_symbol
#define da_const  _dee_aunion.da_const
#define da_alt    _dee_aunion.da_alt
#define da_tuple  _dee_aunion.da_tuple
#define da_seq    _dee_aunion.da_seq
#define da_func   _dee_aunion.da_func
#define da_attr   _dee_aunion.da_attr
#define da_with   _dee_aunion.da_with
#define da_string _dee_aunion.da_string
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
};

#ifdef CONFIG_BUILDING_DEEMON
/* Finalize the given declaration ast. */
INTDEF NONNULL((1)) void DCALL
decl_ast_fini(struct decl_ast *__restrict self);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL
decl_ast_copy(struct decl_ast *__restrict self,
              struct decl_ast const *__restrict other);

/* Check if `a' and `b' are exactly identical. */
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL
decl_ast_equal(struct decl_ast const *__restrict a,
               struct decl_ast const *__restrict b);

/* Parse a declaration expression. */
INTDEF WUNUSED NONNULL((1)) int DCALL decl_ast_parse(struct decl_ast *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL decl_ast_parse_for_symbol(struct symbol *__restrict self);
INTDEF WUNUSED int DCALL decl_ast_skip(void);
#endif /* CONFIG_BUILDING_DEEMON */

#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */


struct ast_loc {
	struct TPPFile      *l_file; /* [0..1] Location file. */
#ifdef CONFIG_BUILDING_DEEMON
	union {
		struct TPPLCInfo l_lc;   /* [valid_if(l_file != NULL)] Line/column information. */
		struct {
			int          l_line; /* [valid_if(l_file != NULL)] Location line. */
			int          l_col;  /* [valid_if(l_file != NULL)] Location column. */
		}
#ifndef __COMPILER_HAVE_TRANSPARENT_STRUCT
		_dee_astruct
#endif /* !__COMPILER_HAVE_TRANSPARENT_STRUCT */
		;
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define l_lc       _dee_aunion.l_lc
#ifdef __COMPILER_HAVE_TRANSPARENT_STRUCT
#define l_line     _dee_aunion.l_line
#define l_col      _dee_aunion.l_col
#else /* __COMPILER_HAVE_TRANSPARENT_STRUCT */
#define l_line     _dee_aunion._dee_astruct.l_line
#define l_col      _dee_aunion._dee_astruct.l_col
#endif /* !__COMPILER_HAVE_TRANSPARENT_STRUCT */
#elif !defined(__COMPILER_HAVE_TRANSPARENT_STRUCT)
#define l_line     _dee_astruct.l_line
#define l_col      _dee_astruct.l_col
#endif /* !__COMPILER_HAVE_TRANSPARENT_STRUCT */
	;
#else /* CONFIG_BUILDING_DEEMON */
	int                  l_line; /* [valid_if(l_file != NULL)] Location line. */
	int                  l_col;  /* [valid_if(l_file != NULL)] Location column. */
#endif /* !CONFIG_BUILDING_DEEMON */
};




struct text_label {
	struct text_label     *tl_next; /* [0..1][owned] Next case-label, or the next symbol with
	                                 *               the same modulated `s_name->k_id' */
	union {
#ifdef __INTELLISENSE__
		     struct ast   *tl_expr; /* [0..1][valid_if(CHAIN(bs_swcase|s_cases))][const]
		                             * Expression of a case-label. NOTE: NULL for the default case.
		                             * NOTE: Always NULL in `bs_swdefl|s_default' labels. */
#else /* __INTELLISENSE__ */
		DREF struct ast   *tl_expr; /* [0..1][valid_if(CHAIN(bs_swcase|s_cases))][const]
		                             * Expression of a case-label. NOTE: NULL for the default case.
		                             * NOTE: Always NULL in `bs_swdefl|s_default' labels. */
#endif /* !__INTELLISENSE__ */
		struct TPPKeyword *tl_name; /* [1..1][valid_if(CHAIN(bs_lbl[*]))][const] Name of this label. */
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define tl_expr _dee_aunion.tl_expr
#define tl_name _dee_aunion.tl_name
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
	struct asm_sym        *tl_asym; /* [0..1] Assembly symbol (lazily allocated) */
	unsigned int           tl_goto; /* The number of times this label is used as a goto target. */
};

#define text_label_name(self, is_case)        \
	((is_case)                                \
	 ? ((self)->tl_expr ? "case" : "default") \
	 : (self)->tl_name->k_name)


#undef CONFIG_SYMBOL_HAS_REFCNT
#define CONFIG_SYMBOL_HAS_REFCNT


struct symbol {
#ifdef CONFIG_SYMBOL_HAS_REFCNT
	DWEAK Dee_refcnt_t    s_refcnt;/* Reference counter */
#endif /* CONFIG_SYMBOL_HAS_REFCNT */
	DREF struct symbol   *s_next;  /* [0..1][owned] Next symbol with the same modulated `s_name->k_id' */
	struct TPPKeyword    *s_name;  /* [1..1][const] Name of this symbol. */
	DeeScopeObject       *s_scope; /* [1..1][const] The scope declaring this symbol. */
#define SYMBOL_TYPE_NONE   0x0000  /* Undefined symbol type. */
#define SYMBOL_TYPE_GLOBAL 0x0001  /* A global symbol. */
#define SYMBOL_TYPE_EXTERN 0x0002  /* An external symbol. */
#define SYMBOL_TYPE_MODULE 0x0003  /* An import module. */
#define SYMBOL_TYPE_MYMOD  0x0004  /* The current module. */
#define SYMBOL_TYPE_GETSET 0x0005  /* A get/set property symbol. */
#define SYMBOL_TYPE_CATTR  0x0006  /* Class attribute. */
#define SYMBOL_TYPE_ALIAS  0x0007  /* An alias for a different symbol. */
#define SYMBOL_TYPE_ARG    0x0008  /* An argument passed to a function.
                                    * NOTE: `s_symid' is the argument index in `s_scope->s_base->bs_argv',
                                    *        meaning it may also be referring to the varargs, or varkwds
                                    *        special argument objects. */
#define SYMBOL_TYPE_LOCAL  0x0009  /* A local symbol. */
#define SYMBOL_TYPE_STACK  0x000a  /* A stack symbol. */
#define SYMBOL_TYPE_STATIC 0x000b  /* A static symbol. */
#define SYMBOL_TYPE_EXCEPT 0x000c  /* The current exception. */
#define SYMBOL_TYPE_MYFUNC 0x000d  /* The current function. */
#define SYMBOL_TYPE_THIS   0x000e  /* The this-argument of a function. */
#define SYMBOL_TYPE_AMBIG  0x000f  /* An ambiguous symbol (caused by `import *' when an overlap occurrs). */
#define SYMBOL_TYPE_FWD    0x0010  /* A forward-defined symbol. */
#define SYMBOL_TYPE_CONST  0x0011  /* A symbol that evaluates to a constant expression. */
#define SYMBOL_TYPE_MAYREF(x)       ((x) >= SYMBOL_TYPE_ARG)
	uint16_t             s_type;   /* Symbol class. (One of `SYMBOL_TYPE_*')
	                                * This describes how is the variable addressed, and where does it live. */
	uint16_t             s_flag;   /* Symbol flags (Set of `SYMBOL_F*') */
#define SYMBOL_FNORMAL   0x0000    /* Normal symbol flags. */
#define SYMBOL_FWEAK     0x0001    /* The symbol is defined weakly and can be overwritten by explicit
                                    * declarations, or turned into a non-weak symbol if used. */
#define SYMBOL_FALLOC    0x0002    /* Used during assembly: the symbol has been allocated. */
#define SYMBOL_FALLOCREF 0x0004    /* A reference ID for the symbol has been allocated. */
#define SYMBOL_FFINAL    0x0010    /* The variable was declared as `final' */
#define SYMBOL_FVARYING  0x0020    /* The variable was declared as `varying' */
#define SYMBOL_FSTACK_NOUNBIND_OK 0x0100 /* FLAG: If the symbol appears in a `del' expression, and `sym_bound' is non-ZERO,
                                          *       still don't warn about the fact that a stack variable isn't being unbound,
                                          *       but is only being overwritten. */
	uint16_t             s_symid;  /* [valid_if(SYMBOL_FALLOC)] The dynamic ID allocated for the symbol.
	                                *  - SYMBOL_TYPE_GLOBAL: GID of the exported symbol.
	                                *  - SYMBOL_TYPE_EXTERN: MID of the imported module.
	                                *  - SYMBOL_TYPE_MODULE: MID of the imported module.
	                                *  - SYMBOL_TYPE_LOCAL:  LID of the exported symbol.
	                                *  - SYMBOL_TYPE_STACK:  Absolute stack position of the symbol.
	                                *  - SYMBOL_TYPE_STATIC: SID of the symbol. */
	uint16_t             s_refid;  /* [valid_if(SYMBOL_FALLOCREF)]
	                                * The effective reference ID (RID) of the symbol within the current assembler,
	                                * when the symbol is declared in a different base-scope, and must be
	                                * used in order to construct an inner function making use of it. */
	struct ast_loc       s_decl;   /* [OVERRIDE(.l_file, REF(TPPFile_Decref) [0..1])]
	                                * The source location first referencing where the symbol. */
	uint32_t             s_nread;  /* Number of times the symbol is read */
	uint32_t             s_nwrite; /* Number of times the symbol is written */
	uint32_t             s_nbound; /* Number of times the symbol is checking for being bound */
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
	struct decl_ast      s_decltype; /* Symbol declaration type. */
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
	union {                        /* Type-specific symbol data. */
		struct {
			DREF struct module_object *e_module; /* [1..1] The module from which the symbol is imported. */
			struct module_symbol      *e_symbol; /* [1..1] The symbol imported from another module. */
		}                s_extern; /* [SYMBOL_TYPE_EXTERN] */
		DREF struct module_object     *s_module; /* [SYMBOL_TYPE_MODULE] */
		struct {
			DREF struct string_object *g_doc;    /* [0..1] An optional documentation string of this global symbol. */
		}                s_global; /* [SYMBOL_TYPE_GLOBAL] */
		struct {
			struct class_attribute *a_attr;   /* [1..1] The attribute that is being described. */
			DREF struct symbol     *a_class;  /* [1..1][REF(SYMBOL_NREAD(.))] The class that is defining the symbol. */
			DREF struct symbol     *a_this;   /* [0..1][REF(SYMBOL_NREAD(.))] The instance to which the attribute is bound (NULL when this is a class attribute). */
		}                s_attr;   /* [SYMBOL_TYPE_CATTR] Class / instance attribute */
		struct {
			DREF struct symbol     *gs_get;   /* [0..1][REF(SYMBOL_NREAD(.))] A symbol that must be called as getter. */
			DREF struct symbol     *gs_del;   /* [0..1][REF(SYMBOL_NREAD(.))] A symbol that must be called as delete. */
			DREF struct symbol     *gs_set;   /* [0..1][REF(SYMBOL_NREAD(.))] A symbol that must be called as setter. */
		}                s_getset; /* [SYMBOL_TYPE_GETSET] */
		DREF struct symbol *s_alias;  /* [SYMBOL_TYPE_ALIAS][1..1] The symbol being aliased.
		                               * NOTE: This symbol may be another alias, however is not allowed to produce a loop */
		struct {
			struct ast_loc             a_decl2;  /* [OVERRIDE(.l_file, REF(TPPFile_Decref) [0..1])]
			                                      * The second declaration location. */
			size_t                     a_declc;  /* Number of additional declaration locations. */
			struct ast_loc            *a_declv;  /* [OVERRIDE(.l_file, REF(TPPFile_Decref) [0..1])]
			                                      * [0..a_declc][owned] Additional declaration locations. */
#define symbol_ambig_foreach_decl(i, decl, self)            \
	for ((i) = 0; (i) < (self)->s_ambig.a_declc + 2; ++(i)) \
		if (((decl) = (i) == 0                              \
		              ? &(self)->s_decl                     \
		              : (i) == 1                            \
		                ? &(self)->s_ambig.a_decl2          \
		                : &(self)->s_ambig.a_declv[(i)-2])  \
		    ->l_file == NULL)                               \
			;                                               \
		else

		}                s_ambig;  /* [SYMBOL_TYPE_AMBIG] */
		DREF DeeObject  *s_const;  /* [SYMBOL_TYPE_CONST] The constant that the symbol evaluates to. */
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define s_extern _dee_aunion.s_extern
#define s_module _dee_aunion.s_module
#define s_global _dee_aunion.s_global
#define s_attr   _dee_aunion.s_attr
#define s_getset _dee_aunion.s_getset
#define s_alias  _dee_aunion.s_alias
#define s_ambig  _dee_aunion.s_ambig
#define s_const  _dee_aunion.s_const
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
};


/* Return the alias of a `SYMBOL_TYPE_ALIAS'-typed symbol
 * `x', or `x' itself if it's some other kind of type. */
FORCELOCAL ATTR_PURE WUNUSED NONNULL((1)) struct symbol *DCALL
SYMBOL_UNWIND_ALIAS(struct symbol *__restrict x) {
	while (x->s_type == SYMBOL_TYPE_ALIAS) {
		Dee_ASSERT(x != x->s_alias);
		x = x->s_alias;
	}
	return x;
}

/* Inplace-unwind alias symbol references.
 * -> Same as `x = SYMBOL_UNWIND_ALIAS(x)' */
#define SYMBOL_INPLACE_UNWIND_ALIAS(x)            \
	do {                                          \
		if ((x)->s_type == SYMBOL_TYPE_ALIAS)     \
			(x) = _priv_symbol_dounwind_alias(x); \
	}	__WHILE0

FORCELOCAL ATTR_PURE WUNUSED NONNULL((1)) struct symbol *DCALL
_priv_symbol_dounwind_alias(struct symbol *__restrict x) {
	do {
		Dee_ASSERT(x != x->s_alias);
		x = x->s_alias;
	} while (x->s_type == SYMBOL_TYPE_ALIAS);
	return x;
}

FORCELOCAL NONNULL((1)) void DCALL
_priv_symbol_incread(struct symbol *__restrict x) {
	for (;;) {
		++x->s_nread;
		if (x->s_type != SYMBOL_TYPE_ALIAS)
			break;
		x = x->s_alias;
	}
}

FORCELOCAL NONNULL((1)) void DCALL
_priv_symbol_incwrite(struct symbol *__restrict x) {
	for (;;) {
		++x->s_nwrite;
		if (x->s_type != SYMBOL_TYPE_ALIAS)
			break;
		x = x->s_alias;
	}
}

FORCELOCAL NONNULL((1)) void DCALL
_priv_symbol_incbound(struct symbol *__restrict x) {
	for (;;) {
		++x->s_nbound;
		if (x->s_type != SYMBOL_TYPE_ALIAS)
			break;
		x = x->s_alias;
	}
}

FORCELOCAL NONNULL((1)) void DCALL
_priv_symbol_decread(struct symbol *__restrict x) {
	for (;;) {
		Dee_ASSERT(x->s_nread);
		--x->s_nread;
		if (x->s_type != SYMBOL_TYPE_ALIAS)
			break;
		x = x->s_alias;
	}
}

FORCELOCAL NONNULL((1)) void DCALL
_priv_symbol_decwrite(struct symbol *__restrict x) {
	for (;;) {
		Dee_ASSERT(x->s_nwrite);
		--x->s_nwrite;
		if (x->s_type != SYMBOL_TYPE_ALIAS)
			break;
		x = x->s_alias;
	}
}

FORCELOCAL NONNULL((1)) void DCALL
_priv_symbol_decbound(struct symbol *__restrict x) {
	for (;;) {
		Dee_ASSERT(x->s_nbound);
		--x->s_nbound;
		if (x->s_type != SYMBOL_TYPE_ALIAS)
			break;
		x = x->s_alias;
	}
}

FORCELOCAL NONNULL((1)) void DCALL
_priv_symbol_addread(struct symbol *__restrict x, uint32_t n) {
	if (n) {
		for (;;) {
			x->s_nread += n;
			if (x->s_type != SYMBOL_TYPE_ALIAS)
				break;
			x = x->s_alias;
		}
	}
}

FORCELOCAL NONNULL((1)) void DCALL
_priv_symbol_addwrite(struct symbol *__restrict x, uint32_t n) {
	if (n) {
		for (;;) {
			x->s_nwrite += n;
			if (x->s_type != SYMBOL_TYPE_ALIAS)
				break;
			x = x->s_alias;
		}
	}
}

FORCELOCAL NONNULL((1)) void DCALL
_priv_symbol_addbound(struct symbol *__restrict x, uint32_t n) {
	if (n) {
		for (;;) {
			x->s_nbound += n;
			if (x->s_type != SYMBOL_TYPE_ALIAS)
				break;
			x = x->s_alias;
		}
	}
}

FORCELOCAL NONNULL((1)) void DCALL
_priv_symbol_subread(struct symbol *__restrict x, uint32_t n) {
	if (n) {
		for (;;) {
			Dee_ASSERT(x->s_nread >= n);
			x->s_nread -= n;
			if (x->s_type != SYMBOL_TYPE_ALIAS)
				break;
			x = x->s_alias;
		}
	}
}

FORCELOCAL NONNULL((1)) void DCALL
_priv_symbol_subwrite(struct symbol *__restrict x, uint32_t n) {
	if (n) {
		for (;;) {
			Dee_ASSERT(x->s_nwrite >= n);
			x->s_nwrite -= n;
			if (x->s_type != SYMBOL_TYPE_ALIAS)
				break;
			x = x->s_alias;
		}
	}
}

FORCELOCAL NONNULL((1)) void DCALL
_priv_symbol_subbound(struct symbol *__restrict x, uint32_t n) {
	if (n) {
		for (;;) {
			Dee_ASSERT(x->s_nbound >= n);
			x->s_nbound -= n;
			if (x->s_type != SYMBOL_TYPE_ALIAS)
				break;
			x = x->s_alias;
		}
	}
}


/* Return the name of a given symbol `x' as a `char *' pointer. */
#define SYMBOL_NAME(x)             ((x)->s_name->k_name)

/* Get/inc/dec the read-, write- and bound- access counters. */
#define SYMBOL_NREAD(x)            ((uint32_t const)(x)->s_nread)
#define SYMBOL_NWRITE(x)           ((uint32_t const)(x)->s_nwrite)
#define SYMBOL_NBOUND(x)           ((uint32_t const)(x)->s_nbound)
#define SYMBOL_INC_NREAD(x)        _priv_symbol_incread(x)
#define SYMBOL_INC_NWRITE(x)       _priv_symbol_incwrite(x)
#define SYMBOL_INC_NBOUND(x)       _priv_symbol_incbound(x)
#define SYMBOL_DEC_NREAD(x)        _priv_symbol_decread(x)
#define SYMBOL_DEC_NWRITE(x)       _priv_symbol_decwrite(x)
#define SYMBOL_DEC_NBOUND(x)       _priv_symbol_decbound(x)
#define SYMBOL_ADD_NREAD(x, n)     _priv_symbol_addread(x, n)
#define SYMBOL_ADD_NWRITE(x, n)    _priv_symbol_addwrite(x, n)
#define SYMBOL_ADD_NBOUND(x, n)    _priv_symbol_addbound(x, n)
#define SYMBOL_SUB_NREAD(x, n)     _priv_symbol_subread(x, n)
#define SYMBOL_SUB_NWRITE(x, n)    _priv_symbol_subwrite(x, n)
#define SYMBOL_SUB_NBOUND(x, n)    _priv_symbol_subbound(x, n)

/* Mark the given symbol `x' as in-use, turning a weakly
 * linked symbol into one that is strongly linked.
 * -> Weakly linked symbols can be re-declared retroactively, with
 *    the act of doing so not causing any compiler warnings. */
#define SYMBOL_MARK_USED(x)        (void)((x)->s_flag &= ~SYMBOL_FWEAK)

/* Check if a given symbol `x' has been declared as a weak symbol. */
#define SYMBOL_IS_WEAK(x)          ((x)->s_flag & SYMBOL_FWEAK)

/* Clear the linkage of a given symbol `x', leaving `x->s_type',
 * as well as all type-specific fields undefined. */
#define SYMBOL_CLEAR_WEAK(x) \
	(symbol_fini(x), (x)->s_flag &= ~SYMBOL_FWEAK)

/* Check if a given symbol `x' must be addressed as a reference */
#define SYMBOL_MUST_REFERENCE(x)          \
	(SYMBOL_TYPE_MAYREF((x)->s_type) &&   \
	 ((x)->s_type == SYMBOL_TYPE_THIS     \
	  ? (x) != current_basescope->bs_this \
	  : (x)->s_scope->s_base != current_basescope))

#define SYMBOL_MUST_REFERENCE_THIS(x)             \
	(Dee_ASSERT((x)->s_type == SYMBOL_TYPE_THIS), \
	 (x) != current_basescope->bs_this)

/* Same as `SYMBOL_MUST_REFERENCE()', but the caller already knows
 * that the symbol's type may be referenced (`SYMBOL_TYPE_MAYREF(x->s_type) == true') */
#define SYMBOL_MUST_REFERENCE_TYPEMAY(x)          \
	(Dee_ASSERT(SYMBOL_TYPE_MAYREF((x)->s_type)), \
	 (x)->s_type == SYMBOL_TYPE_THIS              \
	 ? (x) != current_basescope->bs_this          \
	 : (x)->s_scope->s_base != current_basescope)

#define SYMBOL_MUST_REFERENCE_NOTTHIS(x)                                             \
	(Dee_ASSERT(SYMBOL_TYPE_MAYREF((x)->s_type) && (x)->s_type != SYMBOL_TYPE_THIS), \
	 (x)->s_scope->s_base != current_basescope)

/* Check if a given symbol `x' can be addressed as a reference */
#define SYMBOL_MAY_REFERENCE(x)          \
	((x)->s_type == SYMBOL_TYPE_THIS     \
	 ? (x) != current_basescope->bs_this \
	 : (x)->s_scope->s_base != current_basescope)

#define SYMBOL_EXTERN_MODULE(x)    ((x)->s_extern.e_module) /* XXX: Remove me? */
#define SYMBOL_EXTERN_SYMBOL(x)    ((x)->s_extern.e_symbol) /* XXX: Remove me? */
#define SYMBOL_MODULE_MODULE(x)    ((x)->s_module)          /* XXX: Remove me? */
#define SYMBOL_STACK_OFFSET(x)     ((x)->s_symid)           /* XXX: Remove me? */

/* Finalize the given symbol. */
INTDEF NONNULL((1)) void DCALL symbol_fini(struct symbol *__restrict self);

#ifdef CONFIG_SYMBOL_HAS_REFCNT
/* Destroy a given symbol. */
INTDEF NONNULL((1)) void DCALL symbol_destroy(struct symbol *__restrict self);
#define symbol_incref(x)  (void)(__hybrid_atomic_fetchinc(&(x)->s_refcnt, __ATOMIC_SEQ_CST))
#define symbol_decref(x)  (void)(__hybrid_atomic_decfetch(&(x)->s_refcnt, __ATOMIC_SEQ_CST) || (symbol_destroy(x), 0))
#define symbol_xincref(x) (void)(!(x) || (__hybrid_atomic_fetchinc(&(x)->s_refcnt, __ATOMIC_SEQ_CST)))
#define symbol_xdecref(x) (void)(!(x) || (__hybrid_atomic_decfetch(&(x)->s_refcnt, __ATOMIC_SEQ_CST) || (symbol_destroy(x), 0)))
#else /* CONFIG_SYMBOL_HAS_REFCNT */
#define symbol_incref(x)  (void)0
#define symbol_decref(x)  (void)0
#define symbol_xincref(x) (void)0
#define symbol_xdecref(x) (void)0
#endif /* !CONFIG_SYMBOL_HAS_REFCNT */


/* Add a 3rd, 4th, etc. ambiguity location to a given symbol.
 * When `loc' is NULL, the current location is used. */
INTDEF NONNULL((1)) void DCALL
symbol_addambig(struct symbol *__restrict self,
                struct ast_loc *loc);

/* Check if `self' uses `other' when the specified operation is performed. */
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL symbol_uses_symbol_on_get(struct symbol *__restrict self, struct symbol *__restrict other);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL symbol_uses_symbol_on_del(struct symbol *__restrict self, struct symbol *__restrict other);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL symbol_uses_symbol_on_set(struct symbol *__restrict self, struct symbol *__restrict other);
#define symbol_uses_symbol_on_bnd(self, other) symbol_uses_symbol_on_get(self, other)

/* Check if reading from, or checking the given symbol for being bound has side-effects.
 * Note that UnboundLocal errors (as thrown when accessing an unbound local symbol) are
 * not considered true side-effects for this purpose. */
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL symbol_get_haseffect(struct symbol *__restrict self, DeeScopeObject *__restrict caller_scope);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL symbol_set_haseffect(struct symbol *__restrict self, DeeScopeObject *__restrict caller_scope);
#define symbol_bnd_haseffect(self, caller_scope) symbol_get_haseffect(self, caller_scope)
#define symbol_del_haseffect(self, caller_scope) symbol_set_haseffect(self, caller_scope)
#define CONFIG_SYMBOL_BND_HASEFFECT_IS_SYMBOL_GET_HASEFFECT
#define CONFIG_SYMBOL_SET_HASEFFECT_IS_SYMBOL_GET_HASEFFECT

/* Check if the given symbol `self' is reachable from the given `caller_scope' */
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL symbol_reachable(struct symbol *__restrict self, DeeScopeObject *__restrict caller_scope);


#ifdef CONFIG_BUILDING_DEEMON
INTDEF char const symclass_names[0x1f + 1][8];
#define SYMBOL_TYPE_NAME(cls) symclass_names[(cls)&0x1f]
#endif /* CONFIG_BUILDING_DEEMON */


struct scope_object {
	Dee_OBJECT_HEAD
	Dee_WEAKREF_SUPPORT
#ifdef __INTELLISENSE__
	DeeScopeObject          *s_prev;  /* [0..1][const] Previous scope. */
#else                                 
	DREF DeeScopeObject     *s_prev;  /* [0..1][const] Previous scope. */
#endif                                
	DeeBaseScopeObject      *s_base;  /* [1..1][const] The base scope of the surrounding function.
	                                   * HINT: If this is a self-pointer, this scope is actually a `DeeBaseScopeObject'  */
	DeeClassScopeObject     *s_class; /* [0..1][const] A pointer to the nearest class scope, or NULL if outside of any. */
	DREF struct symbol     **s_map;   /* [0..1][owned][0..s_mapa][owned] Hash-map of symbols defined in this scope.
	                                   * HINT: Use the TPP keyword id modulated by `s_mapa' as index. */
	size_t                   s_mapc;  /* Amount of symbols defined within the hash-map `s_map'. */
	size_t                   s_mapa;  /* Allocated vector size of the symbol hash-map `s_map'. */
	DREF struct symbol      *s_del;   /* [0..1][owned] Chain of symbols that have been deleted. (And thereby made invisible) */
#ifndef NDEBUG
	uint16_t                 s_old_stack; /* Used by stack alignment assertions during assembly: The stack depth when the scope was entered. */
	uint16_t                 s_pad[(sizeof(void *)-2)/2];
#endif /* !NDEBUG */
};
#define DeeScope_IsClassScope(x) ((x)->s_class == (DeeClassScopeObject *)(x))


struct class_scope_object {
	DeeScopeObject      cs_scope; /* Underlying regular scope.
	                               * This is also where all of the class's members are defined. */
	struct symbol      *cs_class; /* [0..1] A symbol describing the class being described. */
	struct symbol      *cs_super; /* [0..1] A symbol describing the new class's base-class. */
	DREF struct symbol *cs_this;  /* [1..1][owned] A symbol describing the this-argument of instances of the class. */
};

/* Returns a pointer to the previous class scope, or `NULL' if no such scope exists. */
#define DeeClassScope_Prev(x) ((x)->cs_scope.s_prev ? (x)->cs_scope.s_prev->s_class : NULL)



struct base_scope_object {
	DeeScopeObject      bs_scope;      /* Underlying regular scope. */
	DeeBaseScopeObject *bs_prev;       /* [0..1] The previous base (function) scope.
	                                    * NOTE: When `NULL', this scope is actually a `DeeRootScopeObject'. */
	DeeRootScopeObject *bs_root;       /* [1..1] The module-local root/global scope.
	                                    * HINT: If this is a self-pointer, this scope is actually a `DeeRootScopeObject'. */
	struct TPPKeyword  *bs_name;       /* [0..1][const] Name of the function of this scope.
	                                    * HINT: During creating of a base-scope, the creator is required
	                                    *       to register a symbol for function self-referencing.
	                                    *       With that in mind, unnamed or root-mode functions does have such a symbol. */
	struct text_label **bs_lbl;        /* [0..1][owned][0..bs_lbla][owned] Hash-map of labels defined in this scope.
	                                    * HINT: Use the TPP keyword id modulated by `bs_lbla' as index. */
	size_t              bs_lblc;       /* Amount of labels defined within the hash-map `bs_lbl'. */
	size_t              bs_lbla;       /* Allocated vector size of the labels hash-map `bs_lbl'. */
	struct text_label  *bs_swcase;     /* [0..1][CHAIN(->tl_next)][owned] Chain of switch labels.
	                                    * NOTE: This chain links cases in the reverse order of their appearance. */
	struct text_label  *bs_swdefl;     /* [0..1][owned] Default label in a switch statement. */
	struct symbol      *bs_this;       /* [0..1] The `cs_this' of the class which this base-scope's function is implementing a member function for. */
	DeeCodeObject      *bs_restore;    /* [0..1] Pointer to the generated code object (once that code object has been generated)
	                                    * In the event that assembler must be reset due to a linker truncation,
	                                    * this code object will be used to restore inherited (stolen) data. */
	DREF DeeObject    **bs_default;    /* [0..1][0..(bs_argc_max - bs_argc_min)][owned] Vector of function default arguments.
	                                    * NOTE: NULL entries refer to optional arguments, producing an error
	                                    *       when attempted to be loaded without a user override. */
#define DeeBaseScope_IsVarargs(self, sym) ((self)->bs_varargs == (sym))
#define DeeBaseScope_IsVarkwds(self, sym) ((self)->bs_varkwds == (sym))
	struct symbol      *bs_varargs;    /* [0..1] A symbol for the varargs argument. */
	struct symbol      *bs_varkwds;    /* [0..1] A symbol for keyword arguments. */
	struct symbol     **bs_argv;       /* [1..1][0..bs_argc][owned]
	                                    * Vector of arguments taken by the function implemented by this scope.
	                                    * HINT: This vector is also used to track which arguments was written to
	                                    *      (deemon assembly doesn't allow modifications of arguments), as all
	                                    *       those that was will have been converted into `SYM_CLASS_LOCAL'.
	                                    *       Using this information, the assembly generate will emit code to copy
	                                    *       all modified arguments into local variables at the start of the function:
	                                    *   >> function foo(x) { print x; }
	                                    *   >> function bar(x) { print x; x = 10; print x; }
	                                    * [[*]->s_index == *] The index in this vector _MUST_ mirror the argument index.
	                                    * [[*]->s_scope == self] All symbols referenced _MUST_ be associated with `bs_prev'.
	                                    * [[*]->s_class == SYMBOL_TYPE_ARG] All symbols referenced _MUST_ be argument symbols.
	                                    * Only symbols of the following classes (should) appear as references (because other's don't make sense):
	                                    * ASSEMBLY:
	                                    *   >> foo:
	                                    *   >>    push arg @x
	                                    *   >>    print pop, nl
	                                    *   >>    ret
	                                    *   >> bar:
	                                    *   >>    push arg @x
	                                    *   >>    pop local @x
	                                    *   >>    push local @x // May be optimized into a dup
	                                    *   >>    print pop, nl
	                                    *   >>    push $10
	                                    *   >>    pop local @x
	                                    *   >>    push local @x // May be optimized into a dup
	                                    *   >>    print pop, nl
	                                    *   >>    ret
	                                    */
	uint16_t            bs_argc_min;   /* Min amount of argument symbols defined for this scope. */
	uint16_t            bs_argc_max;   /* Max amount of argument symbols defined for this scope. */
	uint16_t            bs_argc;       /* [== bs_argc_max + (bs_varargs ? 1 : 0) + (bs_varkwds ? 1 : 0)]
	                                    * The actual argument count for this scope. */
	uint16_t            bs_flags;      /* Scope flags (Set of `CODE_F*'). */
#define BASESCOPE_FNORMAL 0x0000       /* Normal base-scope flags. */
#define BASESCOPE_FRETURN 0x0001       /* A non-empty return statement has been encountered. */
#define BASESCOPE_FSWITCH 0x0002       /* The parser is currently allowed to generate switch-labels. */
	uint16_t            bs_cflags;     /* Compile-time scope flags (Set of `BASESCOPE_F*'). */
#if __SIZEOF_POINTER__ > 4
	uint16_t            bs_padding[(sizeof(void *) / 2) - 2];
#endif /* __SIZEOF_POINTER__ > 4 */
};


struct root_scope_object {
	DeeBaseScopeObject         rs_scope;   /* Underlying base scope. */
	DREF struct module_object *rs_module;  /* [1..1][const] The module that is being compiled. */
	/* NOTE: Fields below are only modified during code generation (aka. by the assembler)
	 *    >> Before assembly starts for the first time, they should all be ZERO/NULL-initialized. */
	DREF DeeCodeObject        *rs_code;    /* [0..1][LINK(->co_next)] Linked list of all code objects already generated for this module. */
	DREF struct module_object**rs_importv; /* [1..1][0..rs_importc|ALLOC(rs_importa)] Vector of other modules imported by this one. */
	struct module_symbol      *rs_bucketv; /* [0..rs_bucketm+1][owned_if(!= empty_module_buckets)]
	                                        * Hash-vector for translating a string into a `uint16_t' index for a global variable.
	                                        * This is where module symbol names are stored and also.
	                                        * HINT: This vector is populated by the assembler during code generation. */
	uint16_t                   rs_flags;   /* Module flags (Set of `MODULE_F*') */
	uint16_t                   rs_globalc; /* The total number of global variables. */
	uint16_t                   rs_bucketm; /* Mask applied to symbol buckets. */
	uint16_t                   rs_importc; /* The total number of other modules imported by this one. */
	uint16_t                   rs_importa; /* The allocated amount of memory for the imported module vector. */
};

#ifdef CONFIG_BUILDING_DEEMON
INTDEF DeeTypeObject DeeScope_Type;
INTDEF DeeTypeObject DeeClassScope_Type;
INTDEF DeeTypeObject DeeBaseScope_Type;
INTDEF DeeTypeObject DeeRootScope_Type;

INTDEF DREF DeeScopeObject *current_scope;     /* [lock(DeeCompiler_Lock)][1..1] The current scope. */
INTDEF DeeBaseScopeObject  *current_basescope; /* [lock(DeeCompiler_Lock)][1..1][== current_scope->s_base] The current base scope. */
INTDEF DeeRootScopeObject  *current_rootscope; /* [lock(DeeCompiler_Lock)][1..1][== current_basescope->bs_root] The current root scope. */

/* Begin/end a new scope.
 * NOTE: The caller should then fill in special information in `current_scope'. */
INTDEF WUNUSED int (DCALL scope_push)(void);
INTDEF void DCALL scope_pop(void);

/* Enter a new class-scope. */
INTDEF WUNUSED int (DCALL classscope_push)(void);
INTDEF WUNUSED struct symbol *(DCALL get_current_this)(void);

/* Begin/end a new base-scope.
 * NOTE: The caller should then fill in special information in `current_basescope'. */
INTDEF WUNUSED int (DCALL basescope_push)(void);
INTDEF void DCALL basescope_pop(void);
INTDEF NONNULL((1)) void DCALL basescope_push_ob(DeeBaseScopeObject *__restrict scope);

/* Lookup a symbol for the given `name', following constraints set by `mode'.
 * @param: mode:     Set of `LOOKUP_SYM_*'
 * @param: warn_loc: When non-NULL the location to reference in warnings.
 *                   When NULL, the current location is used instead.
 * @return: * :      A new reference to the symbol requested.
 * @return: NULL:    An error occurred. */
INTDEF WUNUSED NONNULL((2)) struct symbol *DCALL
lookup_symbol(unsigned int mode, struct TPPKeyword *__restrict name,
              struct ast_loc *warn_loc);
#define LOOKUP_SYM_NORMAL    0x0000
#define LOOKUP_SYM_VDEFAULT  0x0000 /* Default visibility. */
#define LOOKUP_SYM_VLOCAL    0x0001 /* Lookup rules when `local' is prefixed. */
#define LOOKUP_SYM_VGLOBAL   0x0002 /* Lookup rules when `global' is prefixed. */
#define LOOKUP_SYM_VMASK     0x0003 /* Mask for visibility options. */
#define LOOKUP_SYM_STATIC    0x0100 /* Create static variables / warn about non-static, existing variables. */
#define LOOKUP_SYM_STACK     0x0200 /* Create stack variables / warn about non-stack, existing variables. */
#define LOOKUP_SYM_FINAL     0x0400 /* Create final (write-once) variables. */
#define LOOKUP_SYM_VARYING   0x0800 /* Create varying variables (when combined with `LOOKUP_SYM_FINAL': don't
                                     * assume that the value stays the same between multiple invocations). */
#define LOOKUP_SYM_ALLOWDECL 0x8000 /* Allow declaration of new variables (HINT: Unless set, warn when new variables are created). */

/* Lookup the nth instance of `name' (starting at 1 for the first)
 * Return `NULL' if no such instance exists or if nth is 0 */
INTDEF WUNUSED NONNULL((2)) struct symbol *DCALL
lookup_nth(unsigned int nth, struct TPPKeyword *__restrict name);

/* Check if `name' is a reserved symbol name. */
INTDEF WUNUSED NONNULL((1)) bool DCALL
is_reserved_symbol_name(struct TPPKeyword *__restrict name);


/* Lookup or create a label, given its name in the current base-scope. */
INTDEF WUNUSED NONNULL((1)) struct text_label *DCALL
lookup_label(struct TPPKeyword *__restrict name);

/* Create a new case label for `expr'.
 * NOTE: The caller is responsible to ensure that the `BASESCOPE_FSWITCH' flag is set. */
INTDEF WUNUSED NONNULL((1)) struct text_label *DCALL
new_case_label(struct ast *__restrict expr);

/* Ensure existence and return the default label of a switch-statement.
 * NOTE: The caller is responsible to ensure that the `BASESCOPE_FSWITCH' flag is set.
 *       Additionally (if this is desired), it is the caller's task to warn if the
 *       default case had already been allocated before. - This function's only
 *       purpose is to lazily allocate a missing default case and initialize it. */
INTDEF WUNUSED struct text_label *DCALL new_default_label(void);


/* Lookup a symbol in the given scope. */
INTDEF WUNUSED NONNULL((1, 2)) struct symbol *DCALL
scope_lookup(DeeScopeObject *__restrict scope,
             struct TPPKeyword *__restrict name);
INTDEF WUNUSED NONNULL((1, 2)) struct symbol *DCALL
scope_lookup_str(DeeScopeObject *__restrict scope,
                 char const *__restrict name,
                 size_t name_length);

/* Copy argument symbols from the given `other' base-scope into the current,
 * alongside defining them as symbols while duplicating default-values and the
 * var-args flag. - Basically, everything that may be inferred from an argument list.
 * This is done when creating superargs class operators. */
INTDEF WUNUSED NONNULL((1)) int DCALL
copy_argument_symbols(DeeBaseScopeObject *__restrict other);

/* Fully link all forward-defined symbols inside of a class scope. */
INTDEF WUNUSED int DCALL link_forward_symbols(void);

/* Allocate and return a new symbol private to the current scope.
 * NOTE: The caller must ensure that no variable with the given name already exist.
 *       Should the symbol already exist, then there will be two defined afterwards,
 *       only one of which will actually be addressable.
 * NOTE: The caller is also required to initialize the returned
 *       symbol, who's class is undefined up until that point. */
INTDEF WUNUSED NONNULL((1)) struct symbol *DCALL new_local_symbol(struct TPPKeyword *__restrict name, struct ast_loc *loc);
INTDEF WUNUSED NONNULL((1)) struct symbol *DCALL get_local_symbol(struct TPPKeyword *__restrict name);
#define has_local_symbol(name) (get_local_symbol(name) != NULL)

/* Delete a given local symbol, making it anonymous.
 * NOTE: The given `sym' doesn't necessarily need to be apart of the current scope. */
INTDEF NONNULL((1)) void DCALL del_local_symbol(struct symbol *__restrict sym);

/* Create a new unnamed (aka. deleted) symbol. */
INTDEF WUNUSED struct symbol *DCALL
new_unnamed_symbol(void);
INTDEF WUNUSED NONNULL((1)) struct symbol *DCALL
new_unnamed_symbol_in_scope(DeeScopeObject *__restrict scope);

INTDEF WUNUSED NONNULL((1, 2)) struct symbol *DCALL
new_local_symbol_in_scope(DeeScopeObject *__restrict scope,
                          struct TPPKeyword *__restrict name,
                          struct ast_loc *loc);
INTDEF WUNUSED NONNULL((1, 2)) struct symbol *DCALL
get_local_symbol_in_scope(DeeScopeObject *__restrict scope,
                          struct TPPKeyword *__restrict name);


#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define scope_push()     __builtin_expect(scope_push(), 0)
#define basescope_push() __builtin_expect(basescope_push(), 0)
#endif /* !__NO_builtin_expect */
#endif /* !__INTELLISENSE__ */

#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION

FORCELOCAL NONNULL((1, 2)) void DCALL
decl_ast_move(struct decl_ast *__restrict dst,
              struct decl_ast *__restrict src) {
	memcpy(dst, src, sizeof(struct decl_ast));
	if (dst->da_type == DAST_FUNC) {
		Dee_weakref_move(&dst->da_func.f_scope,
		                 &src->da_func.f_scope);
	}
}


FORCELOCAL NONNULL((1, 2)) void DCALL
decl_ast_initsym(struct decl_ast *__restrict self,
                 struct symbol *__restrict sym) {
	self->da_type   = DAST_SYMBOL;
	self->da_flag   = DAST_FNORMAL;
	self->da_symbol = sym;
	symbol_incref(sym);
}

FORCELOCAL NONNULL((1, 2)) void DCALL
decl_ast_initconst(struct decl_ast *__restrict self,
                   DeeObject *__restrict constval) {
	self->da_type  = DAST_CONST;
	self->da_flag  = DAST_FNORMAL;
	self->da_const = constval;
	Dee_Incref(constval);
}

FORCELOCAL NONNULL((1)) void DCALL
decl_ast_initalt(struct decl_ast *__restrict self, size_t altc,
                 /*inherit(always)*/ struct decl_ast *altv) {
	self->da_type       = DAST_ALT;
	self->da_flag       = DAST_FNORMAL;
	self->da_alt.a_altc = altc;
	self->da_alt.a_altv = altv;
}

FORCELOCAL NONNULL((1)) void DCALL
decl_ast_inittuple(struct decl_ast *__restrict self, size_t itemc,
                   /*inherit(always)*/ struct decl_ast *itemv) {
	self->da_type          = DAST_TUPLE;
	self->da_flag          = DAST_FNORMAL;
	self->da_tuple.t_itemc = itemc;
	self->da_tuple.t_itemv = itemv;
}

FORCELOCAL NONNULL((1, 2)) void DCALL
decl_ast_initseq(struct decl_ast *__restrict self,
                 /*inherit(always)*/ struct decl_ast *__restrict item_type) {
	self->da_type = DAST_SEQ;
	self->da_flag = DAST_FNORMAL;
	self->da_seq  = item_type;
}

FORCELOCAL NONNULL((1, 3)) void DCALL
decl_ast_initfunc(struct decl_ast *__restrict self,
                  /*inherit(always)*/ struct decl_ast *return_type,
                  DeeBaseScopeObject *__restrict function_scope) {
	self->da_type       = DAST_FUNC;
	self->da_flag       = DAST_FNORMAL;
	self->da_func.f_ret = return_type;
	Dee_weakref_init(&self->da_func.f_scope,
	                 (DeeObject *)function_scope,
	                 NULL);
}

FORCELOCAL WUNUSED NONNULL((1)) DREF DeeBaseScopeObject *DCALL
decl_ast_func_getscope(struct decl_ast const *__restrict self) {
	Dee_ASSERT(self->da_type == DAST_FUNC);
	return (DREF DeeBaseScopeObject *)Dee_weakref_lock(&self->da_func.f_scope);
}

#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */


#endif /* CONFIG_BUILDING_DEEMON */

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_SYMBOL_H */
