/* Copyright (c) 2018 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_COMPILER_AST_H
#define GUARD_DEEMON_COMPILER_AST_H 1

#include "../api.h"
#include "../object.h"
#include "../code.h"
#include "symbol.h"
#ifdef CONFIG_BUILDING_DEEMON
#include "tpp.h"
#endif
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

DECL_BEGIN

typedef struct ast_object DeeAstObject;

struct catch_expr {
    DREF DeeAstObject   *ce_mask;    /* [0..1] An optional expression evaluated before entry to check.
                                      *        This expression should evaluate to a type that describes
                                      *        a mask which should be used to determine if an error
                                      *        should be handled by this exception handler.
                                      *  NOTE: When this is an AST_MULTIPLE:AST_FMULTIPLE_TUPLE,
                                      *        or a constant tuple, then its elements are used as
                                      *        catch-masks instead. */
    DREF DeeAstObject   *ce_code;    /* [1..1] The expression evaluated to handle the exception.
                                      *        When and how this block is evaluated depends on
                                      *        the `EXCEPTION_HANDLER_FFINALLY' flag in `ce_flags':
                                      * When set this expression is evaluated:
                                      *    - during regular code-flow when `:ast_guard' returns normally
                                      *    - when `:ast_guard' invoked a `return' statement, or
                                      *      before destruction of a yield-iterator that stopped
                                      *      execution within `:ast_guard'.
                                      *    - when `:ast_guard' threw an exception, regardless of
                                      *      whether or not that exception had already been handled
                                      *      by other catch expressions.
                                      * When the flag is not set, the expression is evaluated:   
                                      *    - When `:ast_guard' threw an exception that had not
                                      *      been handled by another catch expression, and
                                      *      `ce_mask' is either `NULL', or contains an expression
                                      *      for which the following code evaluates to true:
                                      *      `CURRENT_EXCEPTION() is ce_mask'.
                                      */
    uint16_t             ce_flags;   /* Catch block flags (Set of `EXCEPTION_HANDLER_F*'). */
#define CATCH_EXPR_FNORMAL 0x0000    /* Normal catch processing. */
#define CATCH_EXPR_FSECOND 0x0001    /* If the catch-expression is a catch-handler (as opposed to being a finally-handler),
                                      * do not generate code to discard secondary exceptions if the handler throws an
                                      * additional primary exception:
                                      * >> try {
                                      * >>     throw "foo";
                                      * >> } catch (...) {
                                      * >>     throw "bar";
                                      * >> }
                                      * With this flag set, any exception thrown in the catch-handler (`"bar"'),
                                      * will cause the previous primary (then secondary) exception (`"foo"') to
                                      * be discarded before exception handling is continued normally.
                                      * However if this flag isn't set, both `"foo"' and `"bar"' remain active
                                      * exception, leaving `"bar"' to be discarded once the associated function
                                      * frame ends.
                                      * This flag is usually set by the AST optimizer if it is detected that
                                      * the handler branch never throws an exception, or is itself protected with
                                      * another handler capable of handling at least all non-interrupt exceptions.
                                      * Note however that if this flag is set and `ce_code' still manages to throw
                                      * an exception, both exceptions will remain active, with all but the one
                                      * thrown first then being discarded during stackframe cleanup. */
    uint16_t             ce_mode;    /* Catch mode flags (Set of `CATCH_EXPR_F*'). */
};

struct class_member {
    DREF DeeAstObject *cm_ast;     /* [1..1] The AST that is evaluated and assigned to the member. */
#define CLASS_MEMBER_MEMBER   0    /* A class or instance-to-class member. */
#define CLASS_MEMBER_OPERATOR 1    /* An operator implemented by the class. */
    uint16_t           cm_type;    /* Class member type (One of `CLASS_MEMBER_*') */
    uint16_t           cm_index;   /* The member index/operator id of the member. */
#if __SIZEOF_POINTER__ > 4
    uint32_t           cm_padding; /* ... */
#endif
};

struct TPPFile;
struct TPPKeyword;
struct string_object;

#ifndef CONFIG_LANGUAGE_NO_ASM
struct TPPString;
struct asm_text {
    /*REF*/struct TPPString    *at_text;  /* [0..1] Assembly text string. */
};
#endif /* !CONFIG_LANGUAGE_NO_ASM */

struct asm_operand {
#ifndef CONFIG_LANGUAGE_NO_ASM
    struct TPPKeyword          *ao_name;  /* [0..1] User-defined name for this operand. */
#endif /* !CONFIG_LANGUAGE_NO_ASM */
    /*REF*/ struct TPPString   *ao_type;  /* [0..1][if((self - :ast_opv) <  :ast_num_o+:ast_num_i,[1..1])]
                                           *       [if((self - :ast_opv) >= :ast_num_o+:ast_num_i,[0..0])]
                                           * Allowed operand types (A string of `ASM_OP_*' from `genasm-userasm.c').
                                           * NOTE: Only, and always `NULL' for label operands. */
    union {
        DREF DeeAstObject      *ao_expr;  /* [1..1][valid_if((self - :ast_opv) < :ast_num_o+:ast_num_i)] Input/output operand expression. */
        struct text_label      *ao_label; /* [1..1][valid_if((self - :ast_opv) >= :ast_num_o+:ast_num_i)] Label operand.
                                           * NOTE: Also holds a reference to `tl_goto' */
    };
};
#define ASM_OPERAND_IS_INOUT(x) ((x)->ao_type->s_text[0] == '+')


struct ast_object {
    OBJECT_HEAD
    DREF DeeScopeObject *ast_scope; /* [1..1] The scope in which this AST exists. */
    struct ast_loc       ast_ddi;   /* [OVERRIDE(.l_file,REF(TPPFile_Decref) [0..1])]
                                     * Debug information describing the location of this AST. */
    uint16_t             ast_type;  /* AST Type (One of `AST_*') */
#define AST_FNORMAL      0x0000     /* Normal AST flags. */
    uint16_t             ast_flag;  /* AST Flags (Set of `AST_F*', dependent on `ast_type') */
    uint16_t             ast_temp;  /* Temporary value used during assembly. */
    uint16_t             ast_pad;   /* ... */
    union {

        /* Base expressions (symbol / constant) */
#define AST_CONSTEXPR        0x0000          /* `42' */
        DREF DeeObject      *ast_constexpr;  /* [1..1] Constant (aka. compile-time) expression.
                                              * NOTE: When set to `none', this AST can be used as a noop expression. */

#define AST_SYM              0x0001          /* `foo' */
        struct symbol       *ast_sym;        /* [1..1][REF(->s_nread) | REF(->s_wread)] Symbol referenced by this AST.
                                              * NOTE: When ast_flag is non-zero, then the symbol is being written to. */

#define AST_UNBIND           0x0002          /* `del foo' */
        struct symbol       *ast_unbind;     /* [1..1][REF(->s_nwrite)] The symbol that should be unbound. */

#define AST_BNDSYM           0x0003          /* `bound(foo)' */
        struct symbol       *ast_bndsym;     /* [1..1][REF(->s_nbound)] The symbol that should be checked for being bound. */

        /* Statement expressions. */
#define AST_MULTIPLE                    0x0004  /* Multiple, consecutive statements/expressions.
                                                 * Dependent on the context, multiple expressions
                                                 * behave special, such as when used as the element
                                                 * expression in a __foreach statement:
                                                 * >> __foreach(a,b,c: foo)
                                                 * ... in which case they will cause the compiler to
                                                 * generate code for unpacking yielded values and
                                                 * individually assigning them to `a', `b' and `c'.
                                                 * NOTE: You may use `ast_flag' to specify how
                                                 *       multiple expressions should be packed together. */
#   define AST_FMULTIPLE_KEEPLAST       0x0000  /* Default: Evaluate, but discard all but the last expression, which is then propagated as value.
                                                 *          If an `AST_EXPAND' expression appears in the sequence,  */
#   define AST_FMULTIPLE_TUPLE          0x1000  /* Pack all elements into a tuple, correctly handling `AST_EXPAND' elements. */
#   define AST_FMULTIPLE_LIST           0x1001  /* Pack all elements into a list, correctly handling `AST_EXPAND' elements. */
#   define AST_FMULTIPLE_SET            0x1002  /* Pack all elements into a set, correctly handling `AST_EXPAND' elements. */
#   define AST_FMULTIPLE_DICT           0x1003  /* Pack all elements into a dict, correctly handling `AST_EXPAND' elements.
                                                 * NOTE: This sequence class requires that `ast_exprc' is aligned
                                                 *       by 2, with every first used as key, and every second as item.
                                                 *       If it isn't aligned by 2, the last expression is ignored and not compiled. */
#   define AST_FMULTIPLE_GENERIC        0x1009  /* Pack elements into a sequence of unspecified typing (used to encode
                                                 * brace-initializer being encoded as `ASM_CALL_SEQ' instructions). */
#   define AST_FMULTIPLE_GENERIC_KEYS   0x100f  /* Pack elements into a dict-style sequence of unspecified typing (used to
                                                 * encode brace-initializer being encoded as `ASM_CALL_MAP' instructions). */
#   define AST_FMULTIPLE_ISGENERIC(x) (((x)&0x1008) == 0x1008)
#   define AST_FMULTIPLE_ISDICT(x)    (((x)&0x1003) == 0x1003)
        struct {
            size_t              ast_exprc;      /* Amount of expressions. */
            DREF DeeAstObject **ast_exprv;      /* [1..1][0..ast_exprc][owned] Vector of expressions. */
        }                       ast_multiple;   /* Multiple, consecutive statements/expressions. */

#define AST_RETURN           0x0005             /* `return' / `return foo' */
        DREF DeeAstObject   *ast_returnexpr;    /* [0..1] Return expression (If `NULL', `none' is returned). */

#define AST_YIELD            0x0006             /* `yield foo' */
        DREF DeeAstObject   *ast_yieldexpr;     /* [1..1] Yield expression. */

#define AST_THROW            0x0007             /* `throw foo' */
        DREF DeeAstObject   *ast_throwexpr;     /* [0..1] The expression being thrown. NOTE: When NULL, this is a `rethrow' statement. */

#define AST_TRY              0x0008             /* `try' statement. */
        struct {
            DREF DeeAstObject  *ast_guard;      /* [1..1] The expression that is being guarded. */
            size_t              ast_catchc;     /* Amount of catch/finally expression. */
            struct catch_expr  *ast_catchv;     /* [0..ast_catchc][owned] Vector of catch/finally expressions.
                                                 * NOTE: Multiple handlers are executed in order, lower-indexed
                                                 *       handlers being evaluated before greater-index ones. */
        }                    ast_try;           /* `try' statement. */

#define AST_LOOP             0x0009          /* Some type of loop statement. (for, while, do..while)
                                              * NOTE: As far as return values go, `AST_LOOP' always evaluates to `none'.
                                              * HINT: The initializer of a for() statement is encoded as an `AST_MULTIPLE'
                                              *       ast containing the initializer first, followed by the loop.
                                              * NOTE: If any of the expressions (`ast_cond', `ast_next' or `ast_loop') contain
                                              *       a `continue' statement, control will always continue at `ast_cond'.
                                              *       If any of them contain a `break' statement, control will continue after `ast_loop'. */
#   define AST_FLOOP_NORMAL         0x0000   /* Normal loop flags. */
#   define AST_FLOOP_POSTCOND       0x0001   /* `ast_cond' (when non-`NULL') is evaluated after `ast_loop',
                                              *  rather than before, essentially resulting in `do..while' behavior. */
#   define AST_FLOOP_FOREACH        0x0002   /* This is a foreach loop:
                                              * `__foreach(x: y) foo(x)' statement.
                                              * HINT: The regular `for (x: y)' statement is implemented using
                                              *       this AST after applying the `__iterself__()' operator to `y'
                                              * NOTE: This flag supersedes `AST_FLOOP_POSTCOND' when both are set.
                                              * NOTE: As far as return values go, `AST_FOREACH' always returns `none'.
                                              * NOTE: If any expression (`ast_elem', `ast_iter' or `ast_loop') contains a
                                              *       `continue' statement, control will jump to yield the next element
                                              *       from the iterator, meaning that `y' will be evaluated immediately after.
                                              *       If any of the expression contain a `break' statement,
                                              *       control will jump forward to continue after `ast_loop'. */
#   define AST_FLOOP_UNLIKELY       0x8000   /* The loop is unlikely to be run and `ast_loop.ast_loop' should be written to the cold section. */
        struct {
            union {
                DREF DeeAstObject  *ast_elem; /* [0..1][valid_if(AST_FLOOP_FOREACH)]
                                               * The element expression (`x' in the example above).
                                               * NOTE: Just like AST_ACTION:AST_FACTION_STORE, this pointer also holds a write-reference to symbols. */
                DREF DeeAstObject  *ast_cond; /* [0..1][valid_if(!AST_FLOOP_FOREACH)] Loop condition. (Either evaluated before, or after the main loop)
                                               * NOTE: Setting this to `NULL' is the same as a constant true. */
            };
            union {
                DREF DeeAstObject  *ast_iter; /* [1..1][valid_if(AST_FLOOP_FOREACH)] The iterator expression (`y' in the example above).
                                               *                               NOTE: This expression is only evaluated once! */
                DREF DeeAstObject  *ast_next; /* [0..1][valid_if(!AST_FLOOP_FOREACH)] Loop advance expression (Executed after `ast_loop', unless `break' was used) */
            };
            DREF DeeAstObject      *ast_loop; /* [0..1] Loop block. */
        }                           ast_loop; /* Loop statement. */

#define AST_LOOPCTL    0x000a          /* Special loop statement. */
#   define AST_FLOOPCTL_BRK 0x0000     /* `break' statement. */
#   define AST_FLOOPCTL_CON 0x0001     /* `continue' statement. */

#define AST_CONDITIONAL 0x000b         /* `if (foo) bar(); else baz()' / `foo ? bar() : baz()' */
#   define AST_FCOND_EVAL     0x0000   /* Use regular evaluation, returning the actual value of the chosen branch. */
#   define AST_FCOND_BOOL     0x0001   /* Force the result of the conditional expression to evaluate.
                                        * Using this flag, `AST_CONDITIONAL' is used to implement
                                        * logical and (`&&') and or (`||') as follows:
                                        * >> a && b; // (a ? b : ) // With `AST_FCOND_BOOL' set
                                        * >> a || b; // (a ?  : b) // With `AST_FCOND_BOOL' set */
#   define AST_FCOND_LIKELY   0x4000   /* When set, place text for `ast_ff' in the cold text-section (at the end of the function). */
#   define AST_FCOND_UNLIKELY 0x8000   /* When set, place text for `ast_tt' in the cold text-section (at the end of the function). */
        struct {
            DREF DeeAstObject  *ast_cond;       /* [1..1] Conditional expression. */
            DREF DeeAstObject  *ast_tt;         /* [0..1][(!= NULL) != (ast_ff != NULL)]
                                                 * Expression evaluated when the condition is met.
                                                 * NOTE: In the event that the same object is assigned
                                                 *       to this field, as is for `ast_cond', the expression
                                                 *       is only evaluated once at runtime, with the uncast
                                                 *       condition then returned as true value:
                                                 *    >> `print get_string() ?: "The string was empty";'
                                                 *       In the above example, `ast_tt == ast_cond', which
                                                 *       share the same object internally.
                                                 *       With that in mind, the generated assembly will
                                                 *       invoke `get_string', duplicate the result, then
                                                 *       check if that value evaluates to true, and if it
                                                 *       does, returns the original return value of `get_string',
                                                 *       or otherwise the fallback text.
                                                 * NOTE: When the `AST_FCOND_BOOL' flag is set, the expression
                                                 *       will evaluate to the boolean representation of the
                                                 *       result expression. */
            DREF DeeAstObject  *ast_ff;         /* [0..1][(!= NULL) != (ast_tt != NULL)]
                                                 * Expression evaluated when the condition isn't met.
                                                 * NOTE: This branch may also be set to `ast_cond', in which
                                                 *       case the result of a condition evaluating to false
                                                 *       will be the uncasted condition. */
        }                    ast_conditional;

        /* Unary/Binary/etc. Expressions. */
#define AST_BOOL             0x000c          /* `!foo' / `!!foo' */
#   define AST_FBOOL_NORMAL  0x0000          /* Directly convert to a boolean. */
#   define AST_FBOOL_NEGATE  0x0001          /* FLAG: Negate the boolean value. (`!' prefix) */
        DREF DeeAstObject   *ast_boolexpr;   /* [1..1] The ast to which the bool operator should be applied. */

#define AST_EXPAND           0x000d          /* `foo...' */
        DREF DeeAstObject   *ast_expandexpr; /* [1..1] The ast affected by the expand expression. */

#define AST_FUNCTION         0x000e          /* A new function definition. */
        struct {
            DREF DeeAstObject  *ast_code;    /* [1..1] The code expression that is executed within this function. */
            DREF DeeBaseScopeObject
                               *ast_scope;   /* The base scope of this function. */
        }                       ast_function;/* A new function definition. */

#define AST_OPERATOR_FUNC    0x000f          /* An operator-function (either bound, or unbound)
                                              * >> x = operator +;   // AST_OPERATOR_POS_OR_ADD (unbound)
                                              * >> x = y.operator +; // AST_OPERATOR_POS_OR_ADD (bound)
                                              * `ast_flag' is one of `OPERATOR_*', or one of `AST_OPERATOR_*'
                                              * When used as function-operand of call-expression, this type
                                              * of AST is transformed into an `AST_OPERATOR' branch.
                                              * Otherwise, it is replaced with an external binding to one
                                              * of the symbols found in the `operators' module:
                                              * >> x = operator +;   // compiles (in this implementation) as `x = __pooad from operators;'
                                              * >> x = y.operator +; // compiles (in this implementation) as `x = (instancemethod from deemon)(y,__pooad from operators);'
                                              */
        struct {
            DREF DeeAstObject  *ast_binding; /* [0..1] The first argument of the operator (if it is bound).
                                              * When set, construct an instancemethod object that is bound to this expression. */
        } ast_operator_func;


#define AST_OPERATOR         0x0010          /* Invocation of some kind of operator.
                                              * Which one specifically is encoded in `ast_flag' as one of `OPERATOR_*'. */
        struct {
            DREF DeeAstObject  *ast_opa;     /* [1..1] First (aka. self) operand. */
            DREF DeeAstObject  *ast_opb;     /* [0..1] Second operand. */
            DREF DeeAstObject  *ast_opc;     /* [0..1] Third operand. */
            DREF DeeAstObject  *ast_opd;     /* [0..1] Fourth operand. */
#define AST_OPERATOR_FNORMAL    0x0000       /* Normal operator flags. */
#define AST_OPERATOR_FPOSTOP    0x0001       /* FLAG: Before the operation, create a copy of the self-operator
                                              *       and discard the return value of the operator, instead opting
                                              *       to exit with the self-copy ontop of the stack.
                                              * NOTE: No copy is created if the value of the expression itself isn't used.
                                              * HINT: This flag is used to implement `a++' as `({ __stack local temp = copy a; ++a; temp; })'  */
#define AST_OPERATOR_FVARARGS   0x0002       /* FLAG: The operator always has 2 arguments, the first of which
                                              *       is the self-operand and the second a tuple of arguments
                                              *       to pass to the operator callback. */
#define AST_OPERATOR_FMAYBEPFX  0x0004       /* FLAG: If the operator is an inplace operation, always generate code and don't
                                              *       cause a compiler error if the operation will cause an error at runtime.
                                              *       This flag is usually set for explicit operator invocations. */
#define AST_OPERATOR_FDONTOPT   0x8000       /* Don't perform constant propagation optimization on this branch.
                                              * Usually just set by the optimizer itself, so-as not to re-attempt
                                              * constant propagation after doing so failed before. */
            uint16_t            ast_exflag;  /* Set of `AST_OPERATOR_F*' */
        }                       ast_operator; /* General purpose operator AST. */
        DREF DeeAstObject      *ast_operator_ops[4];

#define AST_ACTION              0x0011 /* Perform some special action using expressions. */
#   define AST_FACTION_KINDMASK 0x0fff /* MASK: The kind of action (unique). */
#   define AST_FACTION_ARGCMASK 0x7000 /* MASK: The amount of asts referenced by the action. */
#   define AST_FACTION_ARGCSHFT 12     /* SHIFT: Offset for the argument count. */
#   define AST_FACTION_ARGC_GT(x)      (((x)&AST_FACTION_ARGCMASK) >> AST_FACTION_ARGCSHFT)
#   define AST_FACTION_ARGC_ST(x)      (((x) << AST_FACTION_ARGCSHFT)&AST_FACTION_ARGCMASK)
#   define AST_FACTION_MAYBERUN 0x8000 /* FLAG: When set, optimization is allowed to remove branches
                                        *       of this AST even if they may have side-effects.
                                        *       The main intention of this flag is to implement
                                        *       better compile-time optimizations for `typeof()'
                                        *       and `.class'. */
#   define AST_FACTION_CELL0    0x0000 /* `<>'                        - Create an empty cell. */
#   define AST_FACTION_CELL1    0x1001 /* `< <act0> >'                - Create an cell for `<act0>'. */
#   define AST_FACTION_TYPEOF   0x1002 /* `typeof(<act0>)'            - Return the actual type of `<act0>' */
#   define AST_FACTION_CLASSOF  0x1003 /* `<act0>.class'              - Return the effective class of `<act0>' (Dereference super objects) */
#   define AST_FACTION_SUPEROF  0x1004 /* `<act0>.super'              - Return the super-object of `<act0>' */
#   define AST_FACTION_PRINT    0x1005 /* `print <act0>...,;'         - Print the elements of a sequence in `<act0>' (without linefeed) */
#   define AST_FACTION_PRINTLN  0x1006 /* `print <act0>...;'          - Print the elements of a sequence in `<act0>' */
#   define AST_FACTION_FPRINT   0x2007 /* `print <act0>: <act1>...,;' - Print the elements of a sequence in `<act1>' to a file in `<act0>' (without linefeed) */
#   define AST_FACTION_FPRINTLN 0x2008 /* `print <act0>: <act1>...;'  - Print the elements of a sequence in `<act1>' to a file in `<act0>' */
#   define AST_FACTION_RANGE    0x3009 /* `[<act0>:<act1>,<act2>]'    - Create a new range object for enumerating indices from `<act0>' to `<act1>', using `<act2>' as step. (constexpr(none) is used to indicate unset action operands) */
#   define AST_FACTION_IS       0x200a /* `<act0> is <act1>'          - Generate a test to check if `<act0>' ~is~ `<act1>' (instanceof / is none) */
#   define AST_FACTION_IN       0x200b /* `<act0> in <act1>'          - A variant of `AST_OPERATOR:OPERATOR_CONTAINS' that evaluates arguments in reverse order. */
#   define AST_FACTION_AS       0x200c /* `<act0> as <act1>'          - Create a super wrapper object for `<act0>' of type `<act1>' */
#   define AST_FACTION_MIN      0x100d /* `<act0> < ...'              - Return the lowest element of a sequence `<act0>' */
#   define AST_FACTION_MAX      0x100e /* `<act0> > ...'              - Return the greatest element of a sequence `<act0>' */
#   define AST_FACTION_SUM      0x100f /* `<act0> + ...'              - Return the sum of all element in sequence `<act0>' */
#   define AST_FACTION_ANY      0x1010 /* `<act0> || ...'             - Return true if any element of a sequence `<act0>' is true. */
#   define AST_FACTION_ALL      0x1011 /* `<act0> && ...'             - Return true if all elements of a sequence `<act0>' are true. */
#   define AST_FACTION_STORE    0x2012 /* `<act0> = <act1>'           - Store an expression in <act1> into that found in <act0>
                                        *  NOTE: This kind of actions holds special references to every `AST_SYM'
                                        *        apart of `<act0>', including any reached through AST_MULTIPLE:
                                        *        >> --sym_read;
                                        *        >> ++sym_write;
                                        *        This way, symbols can track how often they are written to, or read from. */
#   define AST_FACTION_ASSERT   0x1013 /* `assert <act0>'             - Assert that <act0> evaluates to true at runtime. */
#   define AST_FACTION_ASSERT_M 0x2014 /* `assert <act0>, <act1>'     - Assert that <act0> evaluates to true at runtime,
                                        *                               <act1> is a message that is evaluated and added
                                        *                               to the error message when the assertion fails. */
#   define AST_FACTION_BOUNDATTR 0x2015 /* `<act0>.operator . (<act1>) is bound' - Check if the attribute `<act1>' of `<act0>' is currently bound. */
#   define AST_FACTION_SAMEOBJ  0x2016 /* `<act0> === <act1>'         - Check if <act0> and <act1> are the same object */
#   define AST_FACTION_DIFFOBJ  0x2017 /* `<act0> !== <act1>'         - Check if <act0> and <act1> are the different objects */
#   define AST_FACTION_CALL_KW  0x3018 /* `<act0>(<act1>...,-<act2>-)' - Call `act0' with `act1', while also passing keywords from `act2' */
        struct {
            DREF DeeAstObject  *ast_act0; /* [0..1] Primary action operand or NULL when not used. */
            DREF DeeAstObject  *ast_act1; /* [0..1] Secondary action operand or NULL when not used. */
            DREF DeeAstObject  *ast_act2; /* [0..1] Third action operand or NULL when not used. */
        }                       ast_action;

#define AST_CLASS               0x0012          /* Create a new class object.
                                                 * NOTE: The runtime class flags are stored on `ast_flag & 0xf' */
        struct {
            DREF DeeAstObject  *ast_base;       /* [0..1] Base/super type. */
            DREF DeeAstObject  *ast_name;       /* [0..1] Class name. */
            DREF DeeAstObject  *ast_doc;        /* [0..1] Class documentation string. */
            DREF DeeAstObject  *ast_imem;       /* [0..1] Instance member table. */
            DREF DeeAstObject  *ast_cmem;       /* [0..1] Class member table. */
            struct symbol      *ast_classsym;   /* [0..1] The symbol to which to assign the class before initializing members. */
            struct symbol      *ast_supersym;   /* [0..1] The symbol to which to assign the base before initializing members. */
            size_t              ast_memberc;    /* Amount of member descriptors. */
            struct class_member*ast_memberv;    /* [0..ast_memberc][owned] Vector of member descriptors. */
            size_t              ast_anonc;      /* Amount of anonymous class members. */
            struct member_entry*ast_anonv;      /* [OVERRIDE([*].cme_name,[NOT(DREF)][1..1][== Dee_EmptyString])]
                                                 * [OVERRIDE([*].cme_doc,[NOT(DREF)][0..0])]
                                                 * [0..ast_anonc][owned] Vector of anonymous class members. */
        }                    ast_class;

#define AST_LABEL               0x0013       /* Define a text/case label at the current address.
                                              * HINT: This AST type usually appears inside of another AST_MULTIPLE,
                                              *       followed by the actual expression that is being addressed. */
#define AST_FLABEL_NORMAL       0x0000       /* Normal label flags. */
#define AST_FLABEL_CASE         0x0001       /* FLAG: This is actually a case label.
                                              * -> `ast_label->tl_expr' is valid, but `ast_label->tl_name' isn't. */
        struct {
            struct text_label  *ast_label;      /* [1..1] The text/case label that is being defined. */
            DREF DeeBaseScopeObject *ast_base;  /* [1..1] A reference to the base-scope that owns the label. */
        }                       ast_label;

#define AST_GOTO                0x0014       /* Jump to a given label. */
        struct {
            struct text_label       *ast_label; /* [1..1] The text label to which to jump.
                                                 * NOTE: This field holds a reference to `->tl_goto' */
            DREF DeeBaseScopeObject *ast_base;  /* [1..1] A reference to the base-scope that owns the label. */
        }                    ast_goto;

#define AST_SWITCH              0x0015 /* A switch statement. */
#   define AST_FSWITCH_NORMAL   0x0000 /* Normal switch flags (automatically determine mode) */
#   define AST_FSWITCH_NOJMPTAB 0x0001 /* Never generate a jump table to perform the actual switch.
                                        * Normally, all cases that could be expressed as constant
                                        * key-expressions of a dict, (or even more preferred, indexes of a list)
                                        * are packaged together within a compile-time generated jump table
                                        * that is stored as a constant/static variable and used using
                                        * one of the 2 following code patterns (depending on whether or
                                        * not all of the switch's targets share the same stack-alignment)
                                        * >>#if IDENTICAL_STACK_DEPTHS
                                        * >>     push     const @{ "foo": 1f.SP, "bar": 2f }
                                        * >>#else
                                        * >>     push     const @{ "foo": (1f.SP,1f.IP), "bar": (2f.SP,2f.IP) }
                                        * >>#endif
                                        * >>     push     <ast_expr>
                                        * >>     push     @(3f.SP,3f.IP) // Default case
                                        * >>     callattr top, @"get", #2
                                        * >>#if IDENTICAL_STACK_DEPTHS
                                        * >>     jmp      pop
                                        * >>#else
                                        * >>     unpack   tuple, #2
                                        * >>     jmp      pop, #pop
                                        * >>#endif
                                        * >>1:   // case "foo":
                                        * >>     jmp      .done
                                        * >>2:   // case "bar":
                                        * >>     jmp      .done
                                        * >>3:   // default: (non-constant cases would go here, too)
                                        * >>     jmp      .done
                                        * >>.done:
                                        * Or when all constant case labels are integer
                                        * expressions (non-matching stack code excluded):
                                        * >>     push     <ast_expr>
                                        * >>     dup
                                        * >>     push     $3
                                        * >>     cmp      gr, top, pop
                                        * >>     jt       4f
                                        * >>     push     const @(1f,2f,3f)
                                        * >>     swap
                                        * >>     getitem  top, pop
                                        * >>     jmp      pop
                                        * >>1:   // case 0:
                                        * >>     jmp      .done
                                        * >>2:   // case 1:
                                        * >>     jmp      .done
                                        * >>3:   // case 2:
                                        * >>     jmp      .done
                                        * >>4:   // default: (non-constant cases would go here, too)
                                        * >>     jmp      .done
                                        * >>.done:
                                        */
        struct {
            DREF DeeAstObject *ast_expr;    /* [1..1] The expression on which the switch is enacted. */
            DREF DeeAstObject *ast_block;   /* [1..1] The expression containing all of the labels. */
            struct text_label *ast_cases;   /* [0..1][CHAIN(->tl_next)][owned] Chain of case labels.
                                             * NOTE: Also holds a reference to `tl_goto' of every entry.
                                             * NOTE: This chain links cases in order of first -> last appearance. */
            struct text_label *ast_default; /* [0..1][owned] The default label of the switch statement.
                                             * NOTE: Also holds a reference to `tl_goto'. */
        }                      ast_switch;


#define AST_ASSEMBLY          0x0016   /* User-defined inline assembly (following GCC's __asm__ keyword).
                                        * >> __asm__ [volatile][goto]("text..."
                                        * >>                         [: <ouput_operands>
                                        * >>                         [: <input_operands>
                                        * >>                         [: <clobber_list>    // NOTE: Ignored for now
                                        * >>                         [: <label_list>]]]]);
                                        */
#define AST_FASSEMBLY_NORMAL   0x0000  /* Normal assembly flags. */
#define AST_FASSEMBLY_FORMAT   0x0001  /* Format the assembly text before parsing it. */
#define AST_FASSEMBLY_VOLATILE 0x0002  /* Do not perform peephole optimization on assembly text.
                                        * Internally, this causes the assembler to create a dangling
                                        * symbol for every instruction that makes it appear as though
                                        * it was an anonymous jump target (which can't be optimized).
                                        * NOTE: This does not disable any optimizations
                                        *       on operands passed to the assembly text. */
#define AST_FASSEMBLY_MEMORY   0x0004  /* The assembly branch may not be exchanged with other nearby branches.
                                        * In other words: Act as a read/write compiler barrier. */
#define AST_FASSEMBLY_CLOBSP   0x0008  /* Don't warn if user-assembly leaves the stack in an unaligned state.
                                        * In any case: that state will be fixed by the assembler. */
#define AST_FASSEMBLY_REACH    0x0010  /* User-assembly can be reached through non-conventional means.
                                        * An example for this would be a jump into the assembly block from another one.
                                        * When this flag is set, the optimizer must assembly that it is unpredictable
                                        * whether or not the assembly block returns, or doesn't return, similar to how
                                        * a label definition can return without the previous statement returning. */
#define AST_FASSEMBLY_NORETURN 0x0020  /* The user-assembly statement doesn't return through normal means.
                                        * User-code should contain something along the lines of a `ret' instruction,
                                        * or something similar.
                                        * NOTE: To prevent confusion, this flag is ignored when user-assembly appears
                                        *       within a try-catch block, meaning that the assembly is allowed to set
                                        *       this flag if the intention is to never return by throwing an error.
                                        * NOTE: Setting this flag improperly will only result in weak undefined behavior,
                                        *       as deemon performs dead-code-elimination twice at `-O3'. Once at AST-level,
                                        *       and another time at assembly-level.
                                        *       The more efficient (and important) one is the assembly-level elimination
                                        *       done using peephole optimization. However the other pass done on ASTs
                                        *       themself is the one effect by this flag, and may cause other branches
                                        *       semantically located after the one with this flag to be deleted right
                                        *       there and then, in essence changing the code as though they were never
                                        *       there at all. */
        struct {
#ifndef CONFIG_LANGUAGE_NO_ASM
            struct asm_text     ast_text;  /* The assembly text (not formatted yet). */
#endif
            size_t              ast_num_o; /* Amount of output operands. */
            size_t              ast_num_i; /* Amount of input operands. */
            size_t              ast_num_l; /* Amount of input operands. */
            size_t              ast_opc;   /* [== ast_num_o+ast_num_i+ast_num_l] Total number of operands. */
            struct asm_operand *ast_opv;   /* [0..ast_opc][owned] Vector of assembly operands.
                                            *   - 0..ast_num_o-1 are output operands.
                                            *   - ast_num_o..ast_num_o+ast_num_i-1 are input operands.
                                            *   - ast_num_o+ast_num_i..ast_opc-1 are label operands. */
        }                    ast_assembly;

    };
};


/* Automatically generate moveassign operations when the
 * right-hand-size has been modulated using a copy/deepcopy operator. */
#define AST_SHOULD_MOVEASSIGN(x) \
  ((x)->ast_type == AST_OPERATOR && \
  ((x)->ast_flag == OPERATOR_COPY || \
   (x)->ast_flag == OPERATOR_DEEPCOPY))

#define AST_ISNONE(x) \
 ((x)->ast_type == AST_CONSTEXPR && DeeNone_Check((x)->ast_constexpr))
#define AST_HASDDI(x) ((x)->ast_ddi.l_file != NULL)


DDATDEF DeeTypeObject DeeAst_Type;
#define DeeAst_Check(ob)      DeeObject_InstanceOf(ob,&DeeAst_Type)
#define DeeAst_CheckExact(ob) DeeObject_InstanceOfExact(ob,&DeeAst_Type)




#ifdef CONFIG_BUILDING_DEEMON
/* Set debug information for a given AST.
 * NOTE: This function does not expect the caller to be holding
 *       an actual reference to the file pointed to in `info'.
 *       Instead, it checks if `info->l_file' is equal to the
 *       current source file and only sets debug info when it is.
 *       Otherwise, the assigned debug information will mirror
 *       the current LC location in the active source file.
 * HINT: This function behaves as a noop when `ast' is NULL
 * @return: * : == ast */
INTDEF DeeAstObject *FCALL ast_setddi(DeeAstObject *ast, struct ast_loc *__restrict info);
INTDEF DeeAstObject *FCALL ast_sethere(DeeAstObject *ast);
/* Same as the set functions above, but don't override existing debug information. */
INTDEF DeeAstObject *FCALL ast_putddi(DeeAstObject *ast, struct ast_loc *__restrict info);
INTDEF DeeAstObject *FCALL ast_puthere(DeeAstObject *ast);
/* Fill the given AST location with the current source position. */
INTDEF void FCALL loc_here(struct ast_loc *__restrict info);

/* NOTE: Functions below use `current_scope' to initialize `ast_scope', meaning
 *       that all of them expect the caller to be holding the compiler lock.
 *       Besides that, debug information is initialized to NULL. */

#if defined(NDEBUG) || defined(__INTELLISENSE__)
#define CONFIG_NO_AST_DEBUG 1
#define DEFINE_AST_GENERATOR(name,args) \
   INTDEF DREF DeeAstObject *DCALL name args
#else
#define PRIVATE_AST_GENERATOR_UNPACK_ARGS(...) (char const *file, int line, __VA_ARGS__)
#define DEFINE_AST_GENERATOR(name,args) \
   INTDEF DREF DeeAstObject *DCALL name##_d PRIVATE_AST_GENERATOR_UNPACK_ARGS args
#endif


/* [AST_CONSTEXPR] */
DEFINE_AST_GENERATOR(ast_constexpr,(DeeObject *__restrict constant_expression));
/* [AST_SYM] */
DEFINE_AST_GENERATOR(ast_sym,(struct symbol *__restrict sym));
/* [AST_UNBIND] */
DEFINE_AST_GENERATOR(ast_unbind,(struct symbol *__restrict sym));
/* [AST_BNDSYM] */
DEFINE_AST_GENERATOR(ast_bndsym,(struct symbol *__restrict sym));
/* [AST_MULTIPLE] WARNING: Inherits a heap-allocated vector `exprv' upon success; @param: flags: Set of `AST_FMULTIPLE_*' */
DEFINE_AST_GENERATOR(ast_multiple,(uint16_t flags, size_t exprc, /*inherit*/DREF DeeAstObject **__restrict exprv));
/* [AST_RETURN] NOTE: `return_expr' may be `NULL' */
DEFINE_AST_GENERATOR(ast_return,(DeeAstObject *return_expr));
/* [AST_YIELD] */
DEFINE_AST_GENERATOR(ast_yield,(DeeAstObject *__restrict yield_expr));
/* [AST_THROW] NOTE: `throw_expr' may be `NULL' */
DEFINE_AST_GENERATOR(ast_throw,(DeeAstObject *throw_expr));
/* [AST_TRY] WARNING: Inherits a heap-allocated vector `catchv' upon success */
DEFINE_AST_GENERATOR(ast_try,(DeeAstObject *__restrict guarded_expression, size_t catchc,
                              /*inherit*/struct catch_expr *__restrict catchv));
/* [AST_TRY] Helper function to create a simple try-finally expression using `ast_try' */
DEFINE_AST_GENERATOR(ast_tryfinally,(DeeAstObject *__restrict guarded_expression,
                                     DeeAstObject *__restrict finally_expression));
/* [AST_LOOP] @parma: flags: Set of `AST_FLOOP_*' */
DEFINE_AST_GENERATOR(ast_loop,(uint16_t flags, DeeAstObject *elem_or_cond, DeeAstObject *iter_or_next, DeeAstObject *loop));
/* [AST_LOOPCTL] @parma: flags: Set of `AST_FLOOPCTL_**' */
DEFINE_AST_GENERATOR(ast_loopctl,(uint16_t flags));
/* [AST_CONDITIONAL] @parma: flags: Set of `AST_FCOND_**' */
DEFINE_AST_GENERATOR(ast_conditional,(uint16_t flags, DeeAstObject *__restrict cond,
                                      DeeAstObject *tt_expr, DeeAstObject *ff_expr));
/* [AST_BOOL] @param: flags: Set of `AST_FBOOL_*' */
DEFINE_AST_GENERATOR(ast_bool,(uint16_t flags, DeeAstObject *__restrict expr));
/* [AST_EXPAND] */
DEFINE_AST_GENERATOR(ast_expand,(DeeAstObject *__restrict expr));
/* [AST_FUNCTION] */
DEFINE_AST_GENERATOR(ast_function,(DeeAstObject *__restrict function_code, DeeBaseScopeObject *__restrict scope));
/* [AST_OPERATOR_FUNC] */
DEFINE_AST_GENERATOR(ast_operator_func,(uint16_t operator_name, DeeAstObject *binding));
/* [AST_OPERATOR] @param: operator_name: One of `OPERATOR_*'.
 *                @param: flags: Set of `AST_OPERATOR_F*' */
DEFINE_AST_GENERATOR(ast_operator1,(uint16_t operator_name, uint16_t flags, DeeAstObject *__restrict opa));
DEFINE_AST_GENERATOR(ast_operator2,(uint16_t operator_name, uint16_t flags, DeeAstObject *__restrict opa, DeeAstObject *__restrict opb));
DEFINE_AST_GENERATOR(ast_operator3,(uint16_t operator_name, uint16_t flags, DeeAstObject *__restrict opa, DeeAstObject *__restrict opb, DeeAstObject *__restrict opc));
DEFINE_AST_GENERATOR(ast_operator4,(uint16_t operator_name, uint16_t flags, DeeAstObject *__restrict opa, DeeAstObject *__restrict opb, DeeAstObject *__restrict opc, DeeAstObject *__restrict opd));
/* [AST_ACTION]  @param: action_flags: One of `AST_FACTION_*' (see above). */
DEFINE_AST_GENERATOR(ast_action0,(uint16_t action_flags));
DEFINE_AST_GENERATOR(ast_action1,(uint16_t action_flags, DeeAstObject *__restrict act0));
DEFINE_AST_GENERATOR(ast_action2,(uint16_t action_flags, DeeAstObject *__restrict act0, DeeAstObject *__restrict act1));
DEFINE_AST_GENERATOR(ast_action3,(uint16_t action_flags, DeeAstObject *__restrict act0, DeeAstObject *__restrict act1, DeeAstObject *__restrict act2));
/* [AST_CLASS]   @param: class_flags: Set of `TP_F* & 0xf'.
 * WARNING: Inherits a heap-allocated vector `memberv' upon success. */
DEFINE_AST_GENERATOR(ast_class,(uint16_t class_flags,
                                DeeAstObject *base, DeeAstObject *name, DeeAstObject *doc,
                                DeeAstObject *imem, DeeAstObject *cmem,
                                struct symbol *class_symbol, struct symbol *super_symbol,
                                size_t memberc, struct class_member *__restrict memberv,
                                size_t anonc, struct member_entry *anonv));
/* [AST_LABEL] @param: flags: Set of `AST_FLABEL_*'. */
DEFINE_AST_GENERATOR(ast_label,(uint16_t flags, struct text_label *__restrict lbl, DeeBaseScopeObject *__restrict base_scope));
/* [AST_GOTO] */
DEFINE_AST_GENERATOR(ast_goto,(struct text_label *__restrict lbl, DeeBaseScopeObject *__restrict base_scope));
/* [AST_SWITCH] @param: flags: Set of `AST_FSWITCH_*'.
 * WARNING: Inherits both `cases' and `default_case' upon success. */
DEFINE_AST_GENERATOR(ast_switch,(uint16_t flags, DeeAstObject *__restrict expr, DeeAstObject *__restrict block,
                                 struct text_label *cases, struct text_label *default_case));
/* [AST_ASSEMBLY] WARNING: Inherits a heap-allocated vector `opv' upon success; @param: flags: Set of `AST_FASSEMBLY_*' */
#ifdef CONFIG_LANGUAGE_NO_ASM
DEFINE_AST_GENERATOR(ast_assembly,(uint16_t flags, size_t num_o, size_t num_i, size_t num_l, /*inherit*/struct asm_operand *__restrict opv));
#else /* !CONFIG_LANGUAGE_NO_ASM */
DEFINE_AST_GENERATOR(ast_assembly,(uint16_t flags, struct TPPString *__restrict text, size_t num_o, size_t num_i, size_t num_l, /*inherit*/struct asm_operand *__restrict opv));
#endif /* !CONFIG_LANGUAGE_NO_ASM */

#undef DEFINE_AST_GENERATOR

#ifndef CONFIG_NO_AST_DEBUG
#define ast_constexpr(constant_expression)                     ast_constexpr_d(__FILE__,__LINE__,constant_expression)
#define ast_sym(sym)                                           ast_sym_d(__FILE__,__LINE__,sym)
#define ast_unbind(sym)                                        ast_unbind_d(__FILE__,__LINE__,sym)
#define ast_bndsym(sym)                                        ast_bndsym_d(__FILE__,__LINE__,sym)
#define ast_multiple(flags,exprc,exprv)                        ast_multiple_d(__FILE__,__LINE__,flags,exprc,exprv)
#define ast_return(return_expr)                                ast_return_d(__FILE__,__LINE__,return_expr)
#define ast_yield(yield_expr)                                  ast_yield_d(__FILE__,__LINE__,yield_expr)
#define ast_throw(throw_expr)                                  ast_throw_d(__FILE__,__LINE__,throw_expr)
#define ast_try(guarded_expression,catchc,catchv)              ast_try_d(__FILE__,__LINE__,guarded_expression,catchc,catchv)
#define ast_tryfinally(guarded_expression,finally_expression)  ast_tryfinally_d(__FILE__,__LINE__,guarded_expression,finally_expression)
#define ast_loop(flags,elem_or_cond,iter_or_next,loop)         ast_loop_d(__FILE__,__LINE__,flags,elem_or_cond,iter_or_next,loop)
#define ast_loopctl(flags)                                     ast_loopctl_d(__FILE__,__LINE__,flags)
#define ast_conditional(flags,cond,tt_expr,ff_expr)            ast_conditional_d(__FILE__,__LINE__,flags,cond,tt_expr,ff_expr)
#define ast_bool(flags,expr)                                   ast_bool_d(__FILE__,__LINE__,flags,expr)
#define ast_expand(expr)                                       ast_expand_d(__FILE__,__LINE__,expr)
#define ast_function(function_code,scope)                      ast_function_d(__FILE__,__LINE__,function_code,scope)
#define ast_operator_func(operator_name,binding)               ast_operator_func_d(__FILE__,__LINE__,operator_name,binding)
#define ast_operator1(operator_name,flags,opa)                 ast_operator1_d(__FILE__,__LINE__,operator_name,flags,opa)
#define ast_operator2(operator_name,flags,opa,opb)             ast_operator2_d(__FILE__,__LINE__,operator_name,flags,opa,opb)
#define ast_operator3(operator_name,flags,opa,opb,opc)         ast_operator3_d(__FILE__,__LINE__,operator_name,flags,opa,opb,opc)
#define ast_operator4(operator_name,flags,opa,opb,opc,opd)     ast_operator4_d(__FILE__,__LINE__,operator_name,flags,opa,opb,opc,opd)
#define ast_action0(action_flags)                              ast_action0_d(__FILE__,__LINE__,action_flags)
#define ast_action1(action_flags,act0)                         ast_action1_d(__FILE__,__LINE__,action_flags,act0)
#define ast_action2(action_flags,act0,act1)                    ast_action2_d(__FILE__,__LINE__,action_flags,act0,act1)
#define ast_action3(action_flags,act0,act1,act2)               ast_action3_d(__FILE__,__LINE__,action_flags,act0,act1,act2)
#define ast_class(class_flags,base,name,doc,imem,cmem,class_symbol,super_symbol,memberc,memberv,anonc,anonv) \
        ast_class_d(__FILE__,__LINE__,class_flags,base,name,doc,imem,cmem,class_symbol,super_symbol,memberc,memberv,anonc,anonv)
#define ast_label(flags,lbl,base_scope)                        ast_label_d(__FILE__,__LINE__,flags,lbl,base_scope)
#define ast_goto(lbl,base_scope)                               ast_goto_d(__FILE__,__LINE__,lbl,base_scope)
#define ast_switch(flags,expr,block,cases,default_case)        ast_switch_d(__FILE__,__LINE__,flags,expr,block,cases,default_case)
#ifdef CONFIG_LANGUAGE_NO_ASM
#define ast_assembly(flags,num_o,num_i,num_l,opv)              ast_assembly_d(__FILE__,__LINE__,flags,num_o,num_i,num_l,opv)
#else /* CONFIG_LANGUAGE_NO_ASM */
#define ast_assembly(flags,text,num_o,num_i,num_l,opv)         ast_assembly_d(__FILE__,__LINE__,flags,text,num_o,num_i,num_l,opv)
#endif /* !CONFIG_LANGUAGE_NO_ASM */
#endif /* !CONFIG_NO_AST_DEBUG */


/* Encode some higher-level operations using ASTs. */
#define ast_continue()  ast_loopctl(AST_FLOOPCTL_CON)         /* `continue' */
#define ast_break()     ast_loopctl(AST_FLOOPCTL_BRK)         /* `break' */
#define ast_land(a,b)   ast_conditional(AST_FCOND_BOOL,a,b,a) /* `a && b' */
#define ast_lor(a,b)    ast_conditional(AST_FCOND_BOOL,a,a,b) /* `a || b' */

/* Return true if a given `AST_MULTIPLE' contains expand ASTs. */
INTDEF bool (DCALL ast_multiple_hasexpand)(DeeAstObject *__restrict self);


#endif /* !CONFIG_BUILDING_DEEMON */


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_AST_H */
