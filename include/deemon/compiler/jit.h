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
#ifndef GUARD_DEEMON_COMPILER_JIT_H
#define GUARD_DEEMON_COMPILER_JIT_H 1

#include "../api.h"

/* Disable JIT when optimizing for size. */
#ifdef __OPTIMIZE_SIZE__
#define CONFIG_NO_JIT 1
#endif

/* The deemon JIT compiler is mainly intended for execution of
 * simple user expressions, with the aim of minimizing the amount
 * of time spent trying to preprocess, parse, assemble, link, etc.
 * user input, and instead do everything at once.
 * To maintain this goal, severe restrictions have to be imposed
 * on the JIT compiler, the most important of which is the absence
 * of the C preprocessor, removing directives, macros and escaped
 * line-feeds. */
#ifdef CONFIG_BUILDING_DEEMON
#ifndef CONFIG_NO_JIT
#include "../object.h"
#include "../module.h"
#include "tpp.h"

DECL_BEGIN

/* JIT Token ID overrides. */
#define JIT_KEYWORD    TOK_KEYWORD_BEGIN
#define JIT_STRING     TOK_STRING
#define JIT_RAWSTRING  TOK_CHAR

typedef struct jit_small_lexer JITSmallLexer;
typedef struct jit_lexer JITLexer;
typedef struct jit_module JITModule;
typedef struct jit_context JITContext;

struct jit_small_lexer {
    unsigned int            jl_tok;      /* Token ID (One of `TOK_*' from <tpp.h>, or `JIT_KEYWORD' for an arbitrary keyword) */
    /*utf-8*/unsigned char *jl_tokstart; /* [1..1] Token starting pointer. */
    /*utf-8*/unsigned char *jl_tokend;   /* [1..1] Token end pointer. */
    /*utf-8*/unsigned char *jl_end;      /* [1..1] Input end pointer (dereferences to '\0'). */
};

struct jit_lexer {
    unsigned int            jl_tok;      /* Token ID (One of `TOK_*' from <tpp.h>, or `JIT_KEYWORD' for an arbitrary keyword) */
    /*utf-8*/unsigned char *jl_tokstart; /* [1..1] Token starting pointer. */
    /*utf-8*/unsigned char *jl_tokend;   /* [1..1] Token end pointer. */
    /*utf-8*/unsigned char *jl_end;      /* [1..1] Input end pointer (dereferences to '\0'). */
    JITContext             *jl_context;  /* [1..1][const] The associated JIT context. */
    unsigned int            jl_paren;    /* Paren recursion.
                                          * Required to know how many `)' tokens may be skipped
                                          * when searching for suffix expressions in code such
                                          * as `(x[2]) += 3' */
    unsigned int            jl_suffix;   /* Number of pre-consumed suffix expressions.
                                          * Used to explicitly handle expressions such
                                          * as `x[2] += 3' or `foo.bar is bound' */
};

/* Check if the current token is a keyword `x' */
#define JITLexer_ISKWD(self,x) \
   ((self)->jl_tok == JIT_KEYWORD && \
     COMPILER_STRLEN(x) == ((self)->jl_tokend - (self)->jl_tokstart) && \
     memcmp((self)->jl_tokstart,x,COMPILER_STRLEN(x) * sizeof(char)) == 0)
#define JITLexer_ISTOK(self,x) \
   (COMPILER_STRLEN(x) == ((self)->jl_tokend - (self)->jl_tokstart) && \
    memcmp((self)->jl_tokstart,x,COMPILER_STRLEN(x) * sizeof(char)) == 0)

/* Initialize a given JIT lexer for a given data block, and load the first token. */
#define JITLexer_Start(self,start,end) \
      ((self)->jl_tokend = (start), \
       (self)->jl_end    = (end), \
        JITLexer_Yield(self))


/* Starting at `token->jl_tokend', scan for the next input token
 * NOTE: This function may also be used with `JITSmallLexer' */
INTDEF void FCALL JITLexer_Yield(JITLexer *__restrict self);

/* Remember the fact that an exception was thrown
 * when code at `pos' was being executed. */
INTDEF void FCALL
JITLexer_ErrorTrace(JITLexer *__restrict self,
                    unsigned char *__restrict pos);


struct jit_module {
    DeeModuleObject jm_module; /* The underlying module. */
};


struct jit_context {
    DREF JITModule *jc_module; /* [0..1] The JIT-module descriptor (lazily allocated). */
};
#define JITCONTEXT_INIT    { NULL }
#define JITContext_Init(x) ((x)->jc_module = NULL)
#define JITContext_Fini(x) Dee_XDecref((DeeObject *)(x)->jc_module)



/* Lookup a given symbol within a specific JIT context */
INTDEF DREF DeeObject *FCALL JITContext_Lookup(JITContext *__restrict self, /*utf-8*/char const *__restrict name, size_t namelen);
INTDEF DREF DeeObject *FCALL JITContext_LookupNth(JITContext *__restrict self, /*utf-8*/char const *__restrict name, size_t namelen, size_t nth);



/* Parse an operator name, as can be found in an `x.operator <NAME>' expression
 * @param: features: Set of `P_OPERATOR_F*'
 * @return: * : One of `OPERATOR_*' or `AST_OPERATOR_*'
 * @return: -1: An error occurred. */
INTDEF int32_t FCALL JITLexer_ParseOperatorName(JITLexer *__restrict self, uint16_t features);

/* Return the operator function for `opname', as exported from the `operators' module. */
INTDEF DREF DeeObject *FCALL JIT_GetOperatorFunction(uint16_t opname);

/* Parse, evaluate & execute an expression using JIT
 * @param: flags: Set of `JITLEXER_EVAL_F*' */
#define JITLexer_EvalExpression JITLexer_EvalAssign
#define JITLexer_SkipExpression JITLexer_SkipAssign

/* JIT-specific evaluation flags. */
#define JITLEXER_EVAL_FNORMAL       0x0000 /* Normal evaluation flags. */
#define JITLEXER_EVAL_FALLOWINPLACE 0x0010 /* Primary expression evaluation */
#define JITLEXER_EVAL_FALLOWISBOUND 0x0020 /* Primary expression evaluation */
#define JITLEXER_EVAL_FDISALLOWCAST 0x0040 /* Disallow cast expressions. */
#define JITLEXER_EVAL_FPRIMARY     (JITLEXER_EVAL_FALLOWISBOUND | JITLEXER_EVAL_FALLOWINPLACE)

INTDEF DREF DeeObject *FCALL JITLexer_EvalUnaryHead(JITLexer *__restrict self, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalUnary(JITLexer *__restrict self, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalProd(JITLexer *__restrict self, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalSum(JITLexer *__restrict self, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalShift(JITLexer *__restrict self, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalCmp(JITLexer *__restrict self, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalCmpEQ(JITLexer *__restrict self, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalAnd(JITLexer *__restrict self, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalXor(JITLexer *__restrict self, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalOr(JITLexer *__restrict self, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalAs(JITLexer *__restrict self, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalLand(JITLexer *__restrict self, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalLor(JITLexer *__restrict self, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalCond(JITLexer *__restrict self, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalAssign(JITLexer *__restrict self, unsigned int flags); /* NOTE: Also handled inplace operators. */

/* With the current token one of the unary operator symbols, consume
 * it and parse the second operand before returning the combination */
INTDEF DREF DeeObject *FCALL JITLexer_EvalProdOperand(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalSumOperand(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalShiftOperand(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalCmpOperand(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalCmpEQOperand(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalAndOperand(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalXorOperand(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalOrOperand(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalAsOperand(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalLandOperand(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalLorOperand(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalCondOperand(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalAssignOperand(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags);


INTDEF DREF DeeObject *FCALL JITLexer_EvalCommaTupleOperand(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF int FCALL JITLexer_SkipCommaTupleOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalCommaListOperand(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF int FCALL JITLexer_SkipCommaListOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF DREF DeeObject *FCALL JITLexer_EvalCommaDictOperand(JITLexer *__restrict self, /*inherit(always)*/DREF DeeObject *__restrict lhs, unsigned int flags);
INTDEF int FCALL JITLexer_SkipCommaDictOperand(JITLexer *__restrict self, unsigned int flags);



/* Skip evaluation functions. (same as the regular functions,
 * but expressions are skipped, rather than being evaluated)
 * However, syntax error are still thrown. */
INTDEF int FCALL JITLexer_SkipUnaryHead(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipUnary(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipProd(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipSum(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipShift(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipCmp(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipCmpEQ(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipAnd(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipXor(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipOr(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipAs(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipLand(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipLor(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipCond(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipAssign(JITLexer *__restrict self, unsigned int flags); /* NOTE: Also handled inplace operators. */
INTDEF int FCALL JITLexer_SkipProdOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipSumOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipShiftOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipCmpOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipCmpEQOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipAndOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipXorOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipOrOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipAsOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipLandOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipLorOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipCondOperand(JITLexer *__restrict self, unsigned int flags);
INTDEF int FCALL JITLexer_SkipAssignOperand(JITLexer *__restrict self, unsigned int flags);



DECL_END
#endif /* !CONFIG_NO_JIT */
#endif /* CONFIG_BUILDING_DEEMON */

#endif /* !GUARD_DEEMON_COMPILER_JIT_H */
