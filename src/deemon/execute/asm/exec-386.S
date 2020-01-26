/* Copyright (c) 2018-2020 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2020 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */

/* #define NDEBUG 1 */
/* #define CODE_IS_REGISTER 1 */
//#define NDEBUG 1
//#define CODE_IS_REGISTER 1

/* For debugging: Print a short message stating the address
 * and first byte of each instruction before it is executed.
 * This can be enabled to figure out the last instruction that
 * was executed before something goes wrong, narrowing down
 * the search for bugs. */
#undef TRACE_EXECUTION
//#define TRACE_EXECUTION 1

#ifdef NDEBUG
/* Always use a register for `code' outside of debug-mode. */
#undef CODE_IS_REGISTER
#define CODE_IS_REGISTER 1
#endif



/*

CODE_PC = (ESI - ((*(unsigned int *)(EBP - 16)) + (60)))
CODE_SIZE = (*(unsigned int *)((*(unsigned int *)(EBP - 16)) + 28))
CODE = (*(unsigned int *)(EBP - 16))
CODE_DDI = (*(unsigned int *)(*(unsigned int *)(EBP - 16) + (48 + 8)))
CODE_DDI_STRTAB = (char *)((*(unsigned int *)((*(unsigned int *)(*(unsigned int *)(EBP - 16) + (48 + 8))) + (3 * 4))) + (4 * 5))
CODE_DDI_SYMBOL_NAME = (char *)((*(unsigned int *)((*(unsigned int *)(*(unsigned int *)(EBP - 16) + (48 + 8))) + (3 * 4))) + (4 * 5)) + ((unsigned int *)(*(unsigned int *)((*(unsigned int *)(*(unsigned int *)(EBP - 16) + (48 + 8))) + (7 * 4))))[0]
CODE_DDI_NAME = (char *)((*(unsigned int *)((*(unsigned int *)(*(unsigned int *)(EBP - 16) + (48 + 8))) + (3 * 4))) + (4 * 5)) + ((unsigned int *)(*(unsigned int *)((*(unsigned int *)(*(unsigned int *)(EBP - 16) + (48 + 8))) + (7 * 4))))[(*(unsigned short *)(*(unsigned int *)(*(unsigned int *)(EBP - 16) + (48 + 8)) + (4 * 16)))]

*/


#ifndef CONFIG_HOST_WINDOWS
#if defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__WINDOWS__) || \
    defined(_WIN16) || defined(WIN16) || defined(_WIN32) || defined(WIN32) || \
    defined(_WIN64) || defined(WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || \
    defined(_WIN32_WCE) || defined(WIN32_WCE) || defined(_MSC_VER)
#define CONFIG_HOST_WINDOWS 1
#endif /* Windows... */
#endif /* !CONFIG_HOST_WINDOWS */

#ifndef __USER_LABEL_PREFIX__
#ifdef CONFIG_HOST_WINDOWS
#   define __USER_LABEL_PREFIX__ _
#else /* CONFIG_HOST_WINDOWS */
#   define __USER_LABEL_PREFIX__ /* nothing */
#endif /* !CONFIG_HOST_WINDOWS */
#endif /* !__USER_LABEL_PREFIX__ */


#ifdef __ELF__
#define END(x)   .size x, . - x
#else /* __ELF__ */
#define END(x)   /* nothing */
#endif /* !__ELF__ */

#ifdef CONFIG_NO_THREADS
#define LOCK_PREFIX  /* nothing */
#else /* CONFIG_NO_THREADS */
#define LOCK_PREFIX  lock
#endif /* !CONFIG_NO_THREADS */

/* Reminder: Optimization:
 * >>	testl %eax, %eax
 * >>	jnz   .except
 * >>	jmp   .disp
 * Considering that the except-branch
 * is unlikely, this can be written as:
 * >>	testl %eax, %eax
 * >>	jz    .disp
 * >>	jmp   .except
 * When written as such, the more likely
 * success-branch uses 1 less instruction! */

#include <deemon/api.h>
#include <deemon/asm.h>
#include <deemon/alloc.h>

#if defined(CONFIG_HAVE_EXEC_ASM) || defined(__DEEMON__)

#ifdef __ELF__
#define CONFIG_USE_CFI 1
#endif /* __ELF__ */


#ifdef CONFIG_USE_CFI
#define IF_USE_CFI(...)             __VA_ARGS__
#ifdef CODE_IS_REGISTER
#define ADJUST_CFA_OFFSET(offset)   .cfi_adjust_cfa_offset  offset;
#else /* CODE_IS_REGISTER */
#define ADJUST_CFA_OFFSET(offset)   /* nothing */
#endif /* !CODE_IS_REGISTER */
#else /* CONFIG_USE_CFI */
#define IF_USE_CFI(...)             /* nothing */
#define ADJUST_CFA_OFFSET(offset)   /* nothing */
#endif /* !CONFIG_USE_CFI */

#undef ASSERT

#ifdef NDEBUG
#define ASSERT(...)                /* nothing */
#define ASSERT_LOWER(a,b)          /* nothing */
#define ASSERT_LOWER_EQUAL(a,b)    /* nothing */
#define ASSERT_BELOW(a,b)          /* nothing */
#define ASSERT_BELOW_EQUAL(a,b)    /* nothing */
#define ASSERT_ABOVE(a,b)          /* nothing */
#define ASSERT_ABOVE_EQUAL(a,b)    /* nothing */
#define ASSERT_EQUAL(a,b)          /* nothing */
#define ASSERT_NOT_EQUAL(a,b)      /* nothing */
#define ASSERT_BIT(w,flag,flagset) /* nothing */
#define WASSERT_LOWER(w,a,b)       /* nothing */
#define WASSERT_LOWER_EQUAL(w,a,b) /* nothing */
#define WASSERT_BELOW(w,a,b)       /* nothing */
#define WASSERT_BELOW_EQUAL(w,a,b) /* nothing */
#define WASSERT_ABOVE(w,a,b)       /* nothing */
#define WASSERT_ABOVE_EQUAL(w,a,b) /* nothing */
#define WASSERT_EQUAL(w,a,b)       /* nothing */
#define WASSERT_NOT_EQUAL(w,a,b)   /* nothing */
#else /* NDEBUG */
#define ASSERT_LOWER(x,y)          cmp y, x; assert l;
#define ASSERT_LOWER_EQUAL(x,y)    cmp y, x; assert le;
#define ASSERT_BELOW(x,y)          cmp y, x; assert b;
#define ASSERT_BELOW_EQUAL(x,y)    cmp y, x; assert be;
#define ASSERT_ABOVE(x,y)          cmp y, x; assert a;
#define ASSERT_ABOVE_EQUAL(x,y)    cmp y, x; assert ae;
#define ASSERT_EQUAL(x,y)          cmp y, x; assert e;
#define ASSERT_NOT_EQUAL(x,y)      cmp y, x; assert ne;
#define WASSERT_LOWER(w,x,y)       cmp##w y, x; assert l;
#define WASSERT_LOWER_EQUAL(w,x,y) cmp##w y, x; assert le;
#define WASSERT_BELOW(w,x,y)       cmp##w y, x; assert b;
#define WASSERT_BELOW_EQUAL(w,x,y) cmp##w y, x; assert be;
#define WASSERT_ABOVE(w,x,y)       cmp##w y, x; assert a;
#define WASSERT_ABOVE_EQUAL(w,x,y) cmp##w y, x; assert ae;
#define WASSERT_EQUAL(w,x,y)       cmp##w y, x; assert e;
#define WASSERT_NOT_EQUAL(w,x,y)   cmp##w y, x; assert ne;
#define ASSERT_BIT(w,flag,flagset) test##w flag, flagset; assert nz;
#define ASSERT(...) __VA_ARGS__; assert
.macro assert test=nz
	j\test 991f
	int $3
991:
.endm
#endif /* !NDEBUG */

#define ASSERT_NONZERO(register) \
	ASSERT(test register, register)

#define ERROR_PRINT_DONTHANDLE 0
#define ERROR_PRINT_DOHANDLE   1
#ifdef CONFIG_NO_THREADS
#define ERROR_PRINT_HANDLEINTR ERROR_PRINT_DOHANDLE
#else /* CONFIG_NO_THREADS */
#define ERROR_PRINT_HANDLEINTR 2
#endif /* !CONFIG_NO_THREADS */

#define DEE_STDIN  0
#define DEE_STDOUT 1
#define DEE_STDERR 2

#define ATOMIC_RWLOCK_RMASK 0x7fffffff
#define ATOMIC_RWLOCK_WFLAG 0x80000000

#define ITER_DONE                 -1
#define CODE_FRAME_NOT_EXECUTING  -1

#define MODULE_FNORMAL        0x0000      /* Normal module flags. */
#define MODULE_FDIDINIT       0x8000      /* The module has been initialized. */

#define EXCEPTION_HANDLER_FNORMAL  0x0000
#define EXCEPTION_HANDLER_FFINALLY 0x0001
#define EXCEPTION_HANDLER_FINTERPT 0x0002
#define EXCEPTION_HANDLER_FHANDLED 0x0004

#define CODE_FNORMAL      0x0000
#define CODE_FYIELDING    0x0001
#define CODE_FCOPYABLE    0x0002
#define CODE_FASSEMBLY    0x0004
#define CODE_FLENIENT     0x0008
#define CODE_FVARARGS     0x0010
#define CODE_FVARKWDS     0x0020
#define CODE_FTHISCALL    0x0040
#define CODE_FHEAPFRAME   0x0080
#define CODE_FFINALLY     0x0100
#define CODE_FCONSTRUCTOR 0x0200

#define CONFIG_HAVE_EXEC_ALTSTACK 1
#define DEE_EXEC_ALTSTACK_PERIOD  1024

#define ERROR_HANDLED_NORMAL    0x0000
#define ERROR_HANDLED_RESTORE   0x0001
#define ERROR_HANDLED_INTERRUPT 0x0002

#define TP_FNORMAL       0x0000
#define TP_FFINAL        0x0001
#define TP_FTRUNCATE     0x0002
#define TP_FINTERRUPT    0x0004
#define TP_FVARIABLE     0x0010
#define TP_FGC           0x0020
#define TP_FINTERHITABLE (TP_FINTERRUPT | TP_FVARIABLE | TP_FGC)
#define TP_FMOVEANY      0x0040
#define TP_FINHERITCTOR  0x1000
#define TP_FNAMEOBJECT   0x2000
#define TP_FDOCOBJECT    0x4000
#define TP_FHEAP         0x8000

#define TRIGGER_BREAKPOINT_MIN (-2)
#define TRIGGER_BREAKPOINT_MAX   4
#define TRIGGER_BREAKPOINT_EXCEPT_EXIT (-2)
#define TRIGGER_BREAKPOINT_EXCEPT   (-1)
#define TRIGGER_BREAKPOINT_CONTINUE   0
#define TRIGGER_BREAKPOINT_CONTSAFE   1
#define TRIGGER_BREAKPOINT_RETURN     2
#define TRIGGER_BREAKPOINT_EXIT       3
#define TRIGGER_BREAKPOINT_EXIT_NOFIN 4


#ifdef CONFIG_TRACE_REFCHANGES
#error "This assembly implementation cannot be used when references are being tracked"
#endif /* CONFIG_TRACE_REFCHANGES */



/* Define member offsets. */
#define DEE_OBJECT_OFFSETOF_REFCNT  0
#define DEE_OBJECT_OFFSETOF_TYPE    4
#define DEE_OBJECT_OFFSETOF_DATA    8


/* struct type_object { */
#define tp_name                8 /* char *tp_name; */
#define tp_doc                12 /* char *tp_doc; */
#define tp_flags              16 /* uint16_t tp_flags; */
#define tp_weakrefs           18 /* uint16_t tp_weakrefs; */
#define tp_features           20 /* uint32_t tp_features; */
#define tp_base               24 /* DeeTypeObject *tp_base; */

/* ... */
/* }; */

/* struct list_object { */
#define l_alloc         8 /* size_t l_alloc; */
#define l_size         12 /* size_t l_size; */
#define l_elem         16 /* DREF DeeObject **l_elem; */
#ifndef CONFIG_NO_THREADS
#define l_lock         20 /* rwlock_t l_lock; */
#endif /* !CONFIG_NO_THREADS */
/* }; */

/* struct tuple_object { */
#define t_size          DEE_OBJECT_OFFSETOF_DATA    /* size_t t_size; */
#define t_elem         (DEE_OBJECT_OFFSETOF_DATA+4) /* DREF DeeObject *t_elem[]; */
/* }; */

/* struct super_object { */
#define s_type          DEE_OBJECT_OFFSETOF_DATA    /* DREF DeeTypeObject *s_type; */
#define s_self         (DEE_OBJECT_OFFSETOF_DATA+4) /* DREF DeeObject *s_self; */
/* }; */


/* struct module_object { */
#define mo_name         DEE_OBJECT_OFFSETOF_DATA     /* DREF struct string_object *mo_name; */
#define mo_pself       (DEE_OBJECT_OFFSETOF_DATA+4)  /* DeeModuleObject **mo_pself; */
#define mo_next        (DEE_OBJECT_OFFSETOF_DATA+8)  /* DeeModuleObject *mo_next; */
#define mo_path        (DEE_OBJECT_OFFSETOF_DATA+12) /* DREF struct string_object *mo_path; */
#ifdef CONFIG_HOST_WINDOWS
#define mo_pathhash    (DEE_OBJECT_OFFSETOF_DATA+16) /* dhash_t mo_pathhash; */
#define mo_globpself   (DEE_OBJECT_OFFSETOF_DATA+20) /* DeeModuleObject **mo_globpself; */
#else
#define mo_globpself   (DEE_OBJECT_OFFSETOF_DATA+16) /* DeeModuleObject **mo_globpself; */
#endif
#define mo_globnext    (mo_globpself+4) /* DeeModuleObject *mo_globnext; */
#define mo_importc     (mo_globpself+8) /* uint16_t mo_importc; */
#define mo_globalc     (mo_globpself+10) /* uint16_t mo_globalc; */
#define mo_flags       (mo_globpself+12) /* uint16_t mo_flags; */
#define mo_bucketm     (mo_globpself+14) /* uint16_t mo_bucketm; */
#define mo_bucketv     (mo_globpself+16) /* struct module_symbol *const *mo_bucketv; */
#define mo_importv     (mo_globpself+20) /* DREF DeeModuleObject *const *mo_importv; */
#define mo_globalv     (mo_globpself+24) /* DREF DeeObject **mo_globalv; */
#define mo_root        (mo_globpself+28) /* DREF DeeCodeObject *mo_root; */
#ifndef CONFIG_NO_THREADS
#define mo_lock        (mo_globpself+32) /* rwlock_t mo_lock; */
#define mo_loader      (mo_globpself+36) /* struct thread_object *mo_loader; */
#endif /* !CONFIG_NO_THREADS */
/* }; */

/* struct except_frame { */
#define sizeof_except_frame 12
#define ef_prev         0 /* struct except_frame *ef_prev; */
#define ef_error        4 /* DREF DeeObject *ef_error; */
#define ef_trace        8 /* DREF struct traceback_object *ef_trace; */
/* }; */

/* struct thread_object { */
#define t_exec          DEE_OBJECT_OFFSETOF_DATA     /* struct code_frame *t_exec; */
#define t_except       (DEE_OBJECT_OFFSETOF_DATA+4)  /* struct except_frame *t_except; */
#define t_exceptsz     (DEE_OBJECT_OFFSETOF_DATA+8)  /* uint16_t t_exceptsz; */
#define t_execsz       (DEE_OBJECT_OFFSETOF_DATA+10) /* uint16_t t_execsz; */
#define t_str_curr     (DEE_OBJECT_OFFSETOF_DATA+12) /* struct repr_frame *t_str_curr; */
#define t_repr_curr    (DEE_OBJECT_OFFSETOF_DATA+16) /* struct repr_frame *t_repr_curr; */
/* }; */

/* struct except_handler { */
#define sizeof_except_handler 20
#define eh_mask         0 /* DREF DeeTypeObject *eh_mask;  */
#define eh_start        4 /* code_addr_t eh_start; */
#define eh_end          8 /* code_addr_t eh_end;   */
#define eh_addr        12 /* code_addr_t eh_addr;  */
#define eh_stack       16 /* uint16_t eh_stack; */
#define eh_flags       18 /* uint16_t eh_flags; */
/* }; */

/* struct code_object { */
#define ob_refcnt       0                            /* Dee_refcnt_t ob_refcnt; */
#define ob_type         DEE_OBJECT_OFFSETOF_TYPE     /* DeeTypeObject *ob_type; */
#define co_flags        DEE_OBJECT_OFFSETOF_DATA     /* uint16_t co_flags; */
#define co_localc      (DEE_OBJECT_OFFSETOF_DATA+2)  /* uint16_t co_localc; */
#define co_staticc     (DEE_OBJECT_OFFSETOF_DATA+4)  /* uint16_t co_staticc; */
#define co_refc        (DEE_OBJECT_OFFSETOF_DATA+6)  /* uint16_t co_refc; */
#define co_exceptc     (DEE_OBJECT_OFFSETOF_DATA+8)  /* uint16_t co_exceptc; */
#define co_argc_min    (DEE_OBJECT_OFFSETOF_DATA+10) /* uint16_t co_argc_min; */
#define co_argc_max    (DEE_OBJECT_OFFSETOF_DATA+12) /* uint16_t co_argc_max; */
#define co_padding     (DEE_OBJECT_OFFSETOF_DATA+14) /* uint16_t co_padding; */
#define co_framesize   (DEE_OBJECT_OFFSETOF_DATA+16) /* uint32_t co_framesize; */
#define co_codebytes   (DEE_OBJECT_OFFSETOF_DATA+20) /* code_size_t co_codebytes; */
#ifndef CONFIG_NO_THREADS
#define co_static_lock (DEE_OBJECT_OFFSETOF_DATA+24) /* rwlock_t co_static_lock; */
#define co_module      (DEE_OBJECT_OFFSETOF_DATA+28) /* DREF DeeModuleObject *co_module; */
#define co_next        (DEE_OBJECT_OFFSETOF_DATA+28) /* DREF DeeCodeObject *co_next; */
#define co_keywords    (DEE_OBJECT_OFFSETOF_DATA+32) /* DREF struct kwds_object *co_keywords; */
#define co_defaultv    (DEE_OBJECT_OFFSETOF_DATA+36) /* DREF DeeObject *const *co_defaultv; */
#define co_staticv     (DEE_OBJECT_OFFSETOF_DATA+40) /* DREF DeeObject **co_staticv; */
#define co_exceptv     (DEE_OBJECT_OFFSETOF_DATA+44) /* struct except_handler *co_exceptv; */
#define co_ddi         (DEE_OBJECT_OFFSETOF_DATA+48) /* DREF DeeDDIObject *co_ddi; */
#define co_code        (DEE_OBJECT_OFFSETOF_DATA+52) /* instruction_t co_code[1]; */
#else /* !CONFIG_NO_THREADS */
#define co_module      (DEE_OBJECT_OFFSETOF_DATA+24) /* DREF DeeModuleObject *co_module; */
#define co_next        (DEE_OBJECT_OFFSETOF_DATA+24) /* DREF DeeCodeObject *co_next; */
#define co_keywords    (DEE_OBJECT_OFFSETOF_DATA+28) /* DREF struct kwds_object *co_keywords; */
#define co_defaultv    (DEE_OBJECT_OFFSETOF_DATA+32) /* DREF DeeObject *const *co_defaultv; */
#define co_staticv     (DEE_OBJECT_OFFSETOF_DATA+36) /* DREF DeeObject **co_staticv; */
#define co_exceptv     (DEE_OBJECT_OFFSETOF_DATA+40) /* struct except_handler *co_exceptv; */
#define co_ddi         (DEE_OBJECT_OFFSETOF_DATA+44) /* DREF DeeDDIObject *co_ddi; */
#define co_code        (DEE_OBJECT_OFFSETOF_DATA+48) /* instruction_t co_code[1]; */
#endif /* CONFIG_NO_THREADS */
/* }; */

/* struct code_frame_kwds { */
#define fk_varkwds   0  /* DREF DeeObject *fk_varkwds; */
#define fk_kw        4  /* DREF DeeObject *fk_kw; */
#define fk_kargv     8  /* DREF DeeObject *fk_kargv[]; */
/* }; */

/* struct code_frame { */
#define cf_prev      0  /* struct code_frame *cf_prev; */
#define cf_func      4  /* DeeFunctionObject *cf_func; */
#define cf_argc      8  /* size_t cf_argc; */
#define cf_argv      12 /* DREF DeeObject **cf_argv; */
#define cf_kw        16 /* struct code_frame_kwds *cf_kw; */
#define cf_frame     20 /* DREF DeeObject **cf_frame; */
#define cf_stack     24 /* DREF DeeObject **cf_stack; */
#define cf_sp        28 /* DeeObject **cf_sp; */
#define cf_ip        32 /* instruction_t *cf_ip; */
#define cf_vargs     36 /* DREF struct tuple_object *cf_vargs; */
#define cf_this      40 /* DeeObject *cf_this; */
#define cf_result    44 /* DeeObject *cf_result; */
#define cf_stacksz   46 /* uint16_t cf_stacksz; */
#define cf_flags     50 /* uint16_t cf_flags; */
/* }; */

/* struct function_object { */
#define fo_code      DEE_OBJECT_OFFSETOF_DATA    /* DREF DeeCodeObject *fo_code; */
#define fo_refv     (DEE_OBJECT_OFFSETOF_DATA+4) /* DREF DeeObject    **fo_refv; */
/* }; */



#define FRAME %ebx /* Dedicated register: Current execution frame. */
#define PC    %esi /* Dedicated register: instruction pointer. */
#define SP    %edi /* Dedicated register: stack pointer. */
#define SP_IS_DI 1

#define TOP     -4(SP)
#define FIRST   -4(SP)
#define SECOND  -8(SP)
#define THIRD  -12(SP)
#define FOURTH -16(SP)


#define FRAME_PREV     cf_prev(FRAME)    /* struct code_frame   *cf_prev; */
#define FRAME_FUNC     cf_func(FRAME)    /* DeeFunctionObject   *cf_func; */
#define FRAME_ARGC     cf_argc(FRAME)    /* size_t               cf_argc; */
#define FRAME_ARGV     cf_argv(FRAME)    /* DREF DeeObject     **cf_argv; */
#define FRAME_KW       cf_kw(FRAME)      /* struct code_frame_kwds *cf_kw; */
#define FRAME_FRAME    cf_frame(FRAME)   /* DREF DeeObject     **cf_frame; */
#define FRAME_STACK    cf_stack(FRAME)   /* DREF DeeObject     **cf_stack; */
#define FRAME_SP       cf_sp(FRAME)      /* DeeObject          **cf_sp; */
#define FRAME_PC       cf_ip(FRAME)      /* instruction_t       *cf_ip; */
#define FRAME_VARARGS  cf_vargs(FRAME)   /* DREF struct tuple_object *cf_vargs; */
#define FRAME_THIS     cf_this(FRAME)    /* DeeObject           *cf_this; */
#define FRAME_RESULT   cf_result(FRAME)  /* DeeObject           *cf_result; */
#define FRAME_STACKSZ  cf_stacksz(FRAME) /* uint16_t             cf_stacksz; */

/* Code access helpers */
#ifdef CODE_IS_REGISTER
#define CODE_OR_LOC    CODE
#define LOAD_CODE(reg) /* nothing */
#define CODE_OR_EAX    CODE
#define CODE_OR_ECX    CODE
#define CODE_OR_EDX    CODE
#define TEST_CODE_FLAG(flag, scratch) \
	testw flag, co_flags(CODE);
#else
#define CODE_OR_LOC    L_CODE
#define LOAD_CODE(reg) movl L_CODE, reg;
#define CODE_OR_EAX    %eax
#define CODE_OR_ECX    %ecx
#define CODE_OR_EDX    %edx
#define TEST_CODE_FLAG(flag, scratch) \
	movl  L_CODE, scratch; \
	testw flag, co_flags(scratch);
#endif


#define SYM3(x, y) x##y
#define SYM2(x, y) SYM3(x, y)
#define SYM(x)     SYM2(__USER_LABEL_PREFIX__, x)
#define fSYM(x, s) ASSEMBLY_NAME(x, s)

.text


#ifdef CONFIG_HOST_WINDOWS
#define DEECODE_EXECFRAMEFAST   @DeeCode_ExecFrameFast@4
.globl  DEECODE_EXECFRAMEFAST
.def    DEECODE_EXECFRAMEFAST; .scl 2; .type 32; .endef
DEECODE_EXECFRAMEFAST:
.section .drectve
	.ascii " -export:\"@DeeCode_ExecFrameFast@4\""
.text
#else
#define DEECODE_EXECFRAMEFAST   SYM(DeeCode_ExecFrameFast)
.global DEECODE_EXECFRAMEFAST
DEECODE_EXECFRAMEFAST:
#endif
IF_USE_CFI(.cfi_startproc)
	//jmp    @DeeCode_ExecFrameSafe@4

	/* Preserve callee-save registers. */
	pushl  %ebx
IF_USE_CFI(.cfi_adjust_cfa_offset 4)
IF_USE_CFI(.cfi_rel_offset %ebx, 0)
	pushl  %esi
IF_USE_CFI(.cfi_adjust_cfa_offset 4)
IF_USE_CFI(.cfi_rel_offset %esi, 0)
	pushl  %edi
IF_USE_CFI(.cfi_adjust_cfa_offset 4)
IF_USE_CFI(.cfi_rel_offset %edi, 0)

#ifdef CODE_IS_REGISTER
	/* Use `%ebp' as the general-purpose register it is. */
	pushl  %ebp
IF_USE_CFI(.cfi_adjust_cfa_offset 4)
IF_USE_CFI(.cfi_rel_offset %ebp, 0)
#elif !defined(NDEBUG)
	/* Don't break tracebacks: re-push the return address. */
	pushl  12(%esp)
IF_USE_CFI(.cfi_adjust_cfa_offset 4)
	pushl  %ebp
IF_USE_CFI(.cfi_adjust_cfa_offset 4)
IF_USE_CFI(.cfi_rel_offset %ebp, 0)
	movl   %esp, %ebp
IF_USE_CFI(.cfi_def_cfa_register %ebp)
#endif

	/* Load the passed frame. */
	movl   %ecx, FRAME


	/* Local variable memory. */
#ifdef CODE_IS_REGISTER
#   define SP_OFFSET(x)         x+
#   define BASIC_LMEM           8
#   define L_THIS_THREAD        0(%esp)
#   define L_EXCEPT_RECURSION   4(%esp)
#ifndef NDEBUG
#   define LMEM                 BASIC_LMEM+4
#   define L_STACK_END          8(%esp)
#else
#   define LMEM                 BASIC_LMEM
#endif
#   define CODE                 %ebp
#else /* CODE_IS_REGISTER */
#   define SP_OFFSET(x)         /* nothing */
#   define BASIC_LMEM            8
#   define L_THIS_THREAD        -4(%ebp)
#   define L_EXCEPT_RECURSION   -8(%ebp)
#ifndef NDEBUG
#   define DEBUG_LMEM           BASIC_LMEM+4
#   define L_STACK_END          -12(%ebp)
#else
#   define DEBUG_LMEM           BASIC_LMEM
#endif
#   define LMEM                 DEBUG_LMEM+4
#   define L_CODE               -16(%ebp)
#endif /* !CODE_IS_REGISTER */

	subl   $(LMEM), %esp /* Allocate memory for local variables. */
	ADJUST_CFA_OFFSET(LMEM)

	/* Load execution state. */
	movl   FRAME_SP,      SP
	movl   FRAME_PC,      PC
#ifdef CODE_IS_REGISTER
	movl   FRAME_FUNC,    CODE
	movl   fo_code(CODE), CODE
#else
	movl   FRAME_FUNC,    %eax
	movl   fo_code(%eax), %eax
	movl   %eax,          L_CODE
#endif
#ifndef NDEBUG
	movl   co_framesize(CODE_OR_EAX), %edx
	movzwl co_localc(CODE_OR_EAX), %ecx
	shll   $2, %ecx
	subl   %ecx, %edx
	addl   FRAME_STACK, %edx
	/* stack_end = (code->co_framesize - code->co_localc*4)+frame->cf_stack */
	movl   %edx, L_STACK_END
#endif
	call   fSYM(DeeThread_Self,0)
	movl   %eax,          L_THIS_THREAD
	movw   t_exceptsz(%eax), %cx
	movw   %cx,           L_EXCEPT_RECURSION


	/* Check for a stack-overflow. */
	movw   t_execsz(%eax), %cx
	cmpw   SYM(DeeExec_StackLimit), %cx
	/* if (t_execsz >= DeeExec_StackLimit)
	 *     goto stack_overflow; */
	jae    stack_overflow

#ifdef CONFIG_HAVE_EXEC_ALTSTACK
#if (DEE_EXEC_ALTSTACK_PERIOD & (DEE_EXEC_ALTSTACK_PERIOD-1)) != 0
#error "`DEE_EXEC_ALTSTACK_PERIOD' must be a power-of-2"
#endif
	andw   $(DEE_EXEC_ALTSTACK_PERIOD-1), %cx
	cmpw   $(DEE_EXEC_ALTSTACK_PERIOD-1), %cx
	/* if (IS_ALTSTACK_PERIOD(this_thread->t_execsz)) goto exec_on_altstack; */
	je     exec_on_altstack
#endif

	/* Register our execution frame. */
	movl   t_exec(%eax), %ecx
	movl   %ecx, cf_prev(FRAME)
	movl   FRAME, t_exec(%eax)
.Linc_execsz_start:
	incw   t_execsz(%eax)

#ifndef CONFIG_NO_THREADS
	/* Re-check the CODE_FASSEMBLY flag.
	 * This needs to be done due to a race condition between the
	 * caller testing the assembly flag and us actually registering
	 * the execution frame. */
	LOAD_CODE(%ecx)
	testl  $(CODE_FASSEMBLY), co_flags(CODE_OR_ECX)
	/* ASSUME(EAX == L_THIS_THREAD) */
	jnz   .execute_in_safe_mode
#endif

.disp: /* DISPATCH() */
#ifndef NDEBUG
	leal   (0-co_code)(PC), %eax
	subl   CODE_OR_LOC, %eax /* EAX == PC */
#endif
#if !defined(CODE_IS_REGISTER) && !defined(NDEBUG)
	/* Assert that the host's stack is correctly aligned. */
	leal   LMEM(%esp), %edx
	ASSERT_EQUAL(%edx,%ebp)
#endif /* !CODE_IS_REGISTER */

	movl   PC, FRAME_PC
	xorl   %eax, %eax
	lodsb  /* Read the next instruction into AL. */

#ifdef TRACE_EXECUTION
	pushl  %eax
	ADJUST_CFA_OFFSET(4)

	movl   SP, %eax
	subl   FRAME_STACK, %eax
	shrl   $2, %eax
	pushl  %eax /* SP */
	ADJUST_CFA_OFFSET(4)

	LOAD_CODE(%edx)
	movl   co_ddi(CODE_OR_EDX), %ecx
	movzwl 44(%ecx), %ecx /* struct ddi_object::d_start.dr_name */
	pushl  %ecx
	ADJUST_CFA_OFFSET(4)
	pushl  CODE_OR_EDX
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeCode_GetDDIString,8)
	ADJUST_CFA_OFFSET(-8)
	pushl  %eax  /* NAME */
	ADJUST_CFA_OFFSET(4)

	movl   FRAME_PC, %edx
	LOAD_CODE(%ecx)
	leal   co_code(CODE_OR_ECX), %ecx
	subl   %ecx, %edx

	pushl  8(%esp) /* OPCODE */
	ADJUST_CFA_OFFSET(4)
	pushl  %edx /* ADDR */
	ADJUST_CFA_OFFSET(4)
	pushl  $1f /* FORMAT */
	ADJUST_CFA_OFFSET(4)
	call   SYM(_Dee_dprintf)
	addl   $20, %esp
	ADJUST_CFA_OFFSET(-20)

	popl   %eax
	ADJUST_CFA_OFFSET(-4)
.section .rodata
1:	.string "TRACE: %.4X - %.2X (in %s) (SP = %u)\n"
.text
#endif

	/* Assert stack bounds  */
	ASSERT_ABOVE_EQUAL(SP,FRAME_STACK) /* ERROR: Stack underflow. */
	ASSERT_BELOW_EQUAL(SP,L_STACK_END) /* ERROR: Stack overflow. */

	/* Execute this instruction.
	 * WARNING: Some code assumes that all but the lowest byte of EAX is clear upon entry!
	 *          Code that assumes this often uses the fact to quickly load
	 *          immediate values using `lodsb', then proceeding to work with the full EAX. */
	jmpl   *inst_table(,%eax,4)
	IF_USE_CFI(.cfi_remember_state)

	/* Invoke finally-handlers. */
.end_return:
	TEST_CODE_FLAG($(CODE_FFINALLY),%eax)
	jnz    run_finally_after_return

	/* Save execution state. */
.end_without_finally:
	movl   PC, FRAME_PC
	movl   SP, FRAME_SP

	/* Unlink the execution frame. */
	movl   L_THIS_THREAD, %eax
	decw   t_execsz(%eax) /* --this_thread->t_execsz */
	movl   cf_prev(FRAME), %ecx
	movl   %ecx, t_exec(%eax) /* this_thread->t_exec = frame->cf_prev; */
	movl   $(CODE_FRAME_NOT_EXECUTING), cf_prev(FRAME) /* frame->cf_prev = CODE_FRAME_NOT_EXECUTING */

.end_nounhook:
	/* Make sure that no more than a single new exception was raised. */
	movw   t_exceptsz(%eax), %cx
	cmpw   L_EXCEPT_RECURSION, %cx
	/* WARNING: This helper assumes `ecx = this_thread->exceptsz' */
	jb     delete_exceptions


	/* Load the used return value. */
.end_load_result:
	movl   FRAME_RESULT, %eax

.end_without_unlink:

/*	pushl  %eax
	ADJUST_CFA_OFFSET(4)
	pushl  %eax
	ADJUST_CFA_OFFSET(4)
	movl   FRAME_PC, %eax
	subl   $(co_code), %eax
	subl   CODE_OR_LOC, %eax
	pushl  %eax
	ADJUST_CFA_OFFSET(4)
	pushl  $1f
	ADJUST_CFA_OFFSET(4)
	call   SYM(_Dee_dprintf)
	addl   $12, %esp
	ADJUST_CFA_OFFSET(-12)
.section .rodata
1:	.string "EXIT_PC = %p\nresult  = %p\n"
.text
	popl   %eax
	ADJUST_CFA_OFFSET(-4)
*/

#ifdef CODE_IS_REGISTER
	addl   $(LMEM), %esp /* Free up local variables. */
	ADJUST_CFA_OFFSET(-LMEM)
	popl   %ebp
	ADJUST_CFA_OFFSET(-4)
	IF_USE_CFI(.cfi_restore %ebp)
/*	pushl  %eax
	pushl  %ebp
	pushl  $1f
	call   SYM(_Dee_dprintf)
	addl   $8, %esp
.section .rodata
1:	.string "EBP = %p\n"
.text
	popl   %eax
*/
#elif !defined(NDEBUG)
	/* Make sure that the host's stack is properly restored.
	 * Non-debug code assumes that no such problems exist,
	 * allowing it to make use of EBP as another GP register. */
	leal   LMEM(%esp), %edx
	ASSERT_EQUAL(%edx,%ebp)
	movl   %edx, %esp
	IF_USE_CFI(.cfi_def_cfa %esp, 24)
	leave
	IF_USE_CFI(.cfi_adjust_cfa_offset -4)
	addl   $4, %esp
	IF_USE_CFI(.cfi_adjust_cfa_offset -4)
#endif /* !CODE_IS_REGISTER */

	/* Restore registers. */
	popl   %edi
	IF_USE_CFI(.cfi_restore %edi)
	IF_USE_CFI(.cfi_adjust_cfa_offset -4)
	popl   %esi
	IF_USE_CFI(.cfi_restore %esi)
	IF_USE_CFI(.cfi_adjust_cfa_offset -4)
	popl   %ebx
	IF_USE_CFI(.cfi_restore %ebp)
	IF_USE_CFI(.cfi_adjust_cfa_offset -4)

	ret
END(DEECODE_EXECFRAMEFAST)


IF_USE_CFI(.cfi_restore_state)
IF_USE_CFI(.cfi_remember_state)

#ifndef CONFIG_NO_THREADS
.execute_in_safe_mode:
	/* ASSUME(EAX == L_THIS_THREAD) */
	movl  FRAME_PREV, %ecx
	movl  %ecx, t_exec(%eax) /* this_thread->t_exec = frame->cf_prev */
	decw  t_execsz(%eax)
	movl  $(CODE_FRAME_NOT_EXECUTING), FRAME_PREV
#ifdef CODE_IS_REGISTER
	addl   $(LMEM), %esp
	IF_USE_CFI(.cfi_adjust_cfa_offset -LMEM)
	popl   %ebp
	IF_USE_CFI(.cfi_adjust_cfa_offset -4)
	IF_USE_CFI(.cfi_restore %ebp)
#elif !defined(NDEBUG)
	leal   LMEM(%esp), %edx
	ASSERT_EQUAL(%edx,%ebp)
	movl   %edx, %esp
	IF_USE_CFI(.cfi_def_cfa %esp, 24)
	leave
	IF_USE_CFI(.cfi_adjust_cfa_offset -4)
	addl   $4, %esp
	IF_USE_CFI(.cfi_adjust_cfa_offset -4)
#endif /* !CODE_IS_REGISTER */
	movl   FRAME, %ecx /* Save the frame pointer back into the first argument */
	popl   %edi
	IF_USE_CFI(.cfi_restore %edi)
	IF_USE_CFI(.cfi_adjust_cfa_offset -4)
	popl   %esi
	IF_USE_CFI(.cfi_restore %esi)
	IF_USE_CFI(.cfi_adjust_cfa_offset -4)
	popl   %ebx
	IF_USE_CFI(.cfi_restore %ebx)
	IF_USE_CFI(.cfi_adjust_cfa_offset -4)
	/* Indicate that the stack hasn't been allocated dynamically. */
	movl   $0, cf_stacksz(%ecx)
	/* At this point, the caller's return address is saved  */
#ifdef CONFIG_HOST_WINDOWS
	jmp    @DeeCode_ExecFrameSafe@4
#else
	jmp    fSYM(DeeCode_ExecFrameSafe,4)
#endif
#endif

IF_USE_CFI(.cfi_restore_state)







/* Utility macros. */
.macro incref ob, offset=0
	LOCK_PREFIX incl ob_refcnt+\offset(\ob)
.endm
.macro incref_n ob, offset=0, n=1
	.if \n == 0
	.elif \n == 1
		LOCK_PREFIX incl ob_refcnt+\offset(\ob)
	.else
		LOCK_PREFIX addl $\n, ob_refcnt+\offset(\ob)
	.endif
.endm
.macro incref_addr addr, offset=0
	LOCK_PREFIX incl ob_refcnt+\offset+\addr
.endm
.macro decref ob, safe_eax=0, safe_ecx=0, safe_edx=0
	LOCK_PREFIX subl $1, ob_refcnt(\ob)
	jnz   998f
.if \safe_eax != 0
	pushl %eax
	ADJUST_CFA_OFFSET(4)
.endif
.if \safe_ecx != 0
	pushl %ecx
	ADJUST_CFA_OFFSET(4)
.endif
.if \safe_edx != 0
	pushl %edx
	ADJUST_CFA_OFFSET(4)
.endif
	pushl \ob
	call  fSYM(DeeObject_Destroy,4)
.if \safe_edx != 0
	popl  %edx
	ADJUST_CFA_OFFSET(-4)
.endif
.if \safe_ecx != 0
	popl  %ecx
	ADJUST_CFA_OFFSET(-4)
.endif
.if \safe_eax != 0
	popl  %eax
	ADJUST_CFA_OFFSET(-4)
.endif
998:
.endm

.macro sp_push ob, iref=0
.if \iref != 0
	incref \ob
.endif
.ifc \ob,%eax
	stosl
.else
	movl   \ob, (SP)
	addl   $4, SP
.endif
.endm
.macro sp_pop ob
	subl   $4, SP
	movl  (SP), \ob
.endm
.macro sp_popref scratch=%eax, safe_eax=0, safe_ecx=0, safe_edx=0
	sp_pop \scratch
	decref \scratch, \safe_eax, \safe_ecx, \safe_edx
.endm
.macro unset_result scratch=%eax, safe_eax=0, safe_ecx=0, safe_edx=0
	movl   FRAME_RESULT, \scratch
	decl   \scratch
	cmpl   $-2, \scratch
	jae    997f
	incl   \scratch
	decref \scratch, \safe_eax, \safe_ecx, \safe_edx
997:
.endm

/* Helper macros for r/w-locks. */
#ifndef CONFIG_NO_THREADS
/* WARNING: Clobber: \clobber, %eax
 * @return: true:  ZF=1
 * @return: false: ZF=0 */
.macro rwlock_trywrite self, clobber=%ecx, eax_is_zero=0
.if \eax_is_zero == 0
	xorl  %eax, %eax
.endif
	movl  $(ATOMIC_RWLOCK_WFLAG), \clobber
	LOCK_PREFIX cmpxchgl \clobber, \self
.endm

/* WARNING: Clobber: \clobber, %eax
 * @return: true:  ZF=1
 * @return: false: ZF=0 */
.macro rwlock_tryread self, clobber=%ecx
995:
	movl  \self, %eax
	testl $(ATOMIC_RWLOCK_WFLAG), %eax
	jnz   994f
	leal  1(%eax), \clobber
	LOCK_PREFIX cmpxchgl \clobber, \self
	jnz   995b
994:
.endm


/* WARNING: Clobber: \clobber, %eax */
.macro rwlock_write self, clobber=%ecx, eax_is_zero=0
996:
	rwlock_trywrite \self, \clobber, \eax_is_zero
#ifdef ARCH_YIELD
	jz     997f
	ARCH_YIELD
	jmp    996b
997:
#else
	jnz    996b
#endif
.endm

/* WARNING: Clobber: \clobber, %eax */
.macro rwlock_read self, clobber=%ecx
996:
	rwlock_tryread \self, \clobber
#ifdef ARCH_YIELD
	jz     997f
	ARCH_YIELD
	jmp    996b
997:
#else
	jnz    996b
#endif
.endm

.macro rwlock_endread self
	LOCK_PREFIX decl \self
.endm
.macro rwlock_endwrite self
	movl   $0, \self
.endm
#endif /* !CONFIG_NO_THREADS */

#define repeat_instruction movl  FRAME_PC, PC

#define ASSERT_YIELDING  \
	ASSERT(LOAD_CODE(%eax); \
	       testw $(CODE_FYIELDING), co_flags(CODE_OR_EAX))





delete_exceptions:
	/* Clear the return value. */
	unset_result %eax
	movl $0, FRAME_RESULT
	/* Additionally (to keep things consistent), despite
	 * allowing multiple exceptions to be raised at once,
	 * deemon now only allows functions to return with a
	 * single new exception set.
	 * With that in mind, we simply discard any additional
	 * exceptions that were raised after the first one was,
	 * greatly simplifying things when compared against the
	 * mess that the old deemon did, with multiple catch blocks
	 * stacked on top of each other, while still not providing
	 * a way of _truely_ catching all exception.
	 * This time around though, we make things much more obvious
	 * in allowing only a single exception to be returned by
	 * any function. */
	/* HINT: ecx == this_thread->t_exceptsz (set by the caller) */
	movw   L_EXCEPT_RECURSION, %ax
	incw   %ax
	subw   %ax, %cx
	jae   .end_load_result /* if (except_recursion+1 >= this_thread->t_exceptsz) return NULL; */
	movzwl %cx, %ecx
	/* NOTE: `%ecx' is the number of errors to discard. */
1:	/* DeeError_Print("Discarding secondary error\n"); */
	pushl  $(ERROR_PRINT_DOHANDLE)
	ADJUST_CFA_OFFSET(4)
	pushl  $99f
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeError_Print,8)
	ADJUST_CFA_OFFSET(-8)
	loop   1b
	jmp   .end_load_result

.section .rodata
99: .string "Discarding secondary error\n"
.text


run_finally_after_return:
	LOAD_CODE(%edx)
	movl   FRAME_PC,                %eax
	subl   CODE_OR_EDX,             %eax
	subl   $(co_code),              %eax /* eax: ip_addr */
	movzwl co_exceptc(CODE_OR_EDX), %ecx /* ecx: exception_hander_count */
	movl   co_exceptv(CODE_OR_EDX), %edx /* edx: exception_hander_vector */
	ASSERT_NONZERO(%ecx)
#if sizeof_except_handler == 20
	leal   (%edx,%ecx,8), %edx /* edx = edx + ecx * 8 */
	leal   (%edx,%ecx,8), %edx /* edx = edx + ecx * 8 */
	leal   (%edx,%ecx,4), %edx /* edx = edx + ecx * 4 */
#else
#error FIXME
#endif
1:	subl   $(sizeof_except_handler), %edx
	testw  $(EXCEPTION_HANDLER_FFINALLY), eh_flags(%edx)
	jz     2f /* if (!EXCEPTION_HANDLER_FFINALLY) continue; */
	cmpl   eh_start(%edx), %eax
	jb     2f /* if (ip_addr < eh_start) continue; */
	cmpl   eh_end(%edx), %eax
	jae    2f /* if (ip_addr >= eh_end) continue; */
	/* Found one that must be executed! */
	jmp    run_except_handler_in_edx
2:	loop   1b
	jmp   .end_without_finally





/* Various cleanup operations before handling an exception. */
	ADJUST_CFA_OFFSET(16)
.except_decref_xpop_x4:
	popl    %edx
	ADJUST_CFA_OFFSET(-4)
	testl   %edx, %edx
	jz      .except_decref_xpop_x3
	decref  %edx
.except_decref_xpop_x3:
	popl    %edx
	ADJUST_CFA_OFFSET(-4)
	testl   %edx, %edx
	jz      .except_decref_xpop_x2
	decref  %edx
.except_decref_xpop_x2:
.except_decref_pop_x2 = .except_decref_xpop_x2
	popl    %edx
	ADJUST_CFA_OFFSET(-4)
	testl   %edx, %edx
	jz      .except_decref_xpop
	decref  %edx
.except_decref_xpop:
	popl    %edx
	ADJUST_CFA_OFFSET(-4)
	testl   %edx, %edx
	jz      .except
	jmp     .except_decref_edx

	ADJUST_CFA_OFFSET(4)
.except_decref_pop:
	popl    %edx
	ADJUST_CFA_OFFSET(-4)
.except_decref_edx:
	decref  %edx
	jmp    .except

	ADJUST_CFA_OFFSET(24)
.except_x24:
	addl   $4, %esp
	ADJUST_CFA_OFFSET(-4)
.except_x20:
	addl   $4, %esp
	ADJUST_CFA_OFFSET(-4)
.except_x16:
	addl   $4, %esp
	ADJUST_CFA_OFFSET(-4)
.except_x12:
	addl   $4, %esp
	ADJUST_CFA_OFFSET(-4)
.except_x8:
	addl   $4, %esp
	ADJUST_CFA_OFFSET(-4)
.except_x4:
	addl   $4, %esp
	ADJUST_CFA_OFFSET(-4)
	/* Entry point for general-purpose exception handlers. */
.except:
	/* STEP #1: Lazily allocate a traceback. */
	movl   L_THIS_THREAD, %edx
	movl   t_except(%edx), %eax
	ASSERT_NONZERO(%eax)
	cmpl   $(ITER_DONE), ef_trace(%eax)
	jne    .except_no_traceback
	pushl  %edx
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeTraceback_New,4)
	ADJUST_CFA_OFFSET(-4)
	movl   L_THIS_THREAD, %edx
	movl   t_except(%edx), %edx
	movl   %eax, ef_trace(%edx)
.except_no_traceback:
	/* STEP #2: Execute exception handlers. */
	LOAD_CODE(%edx)
	movl   FRAME_PC,                %eax
	subl   CODE_OR_EDX,             %eax
	subl   $(co_code),              %eax /* eax: ip_addr */
	movzwl co_exceptc(CODE_OR_EDX), %ecx /* ecx: exception_hander_count */
	movl   co_exceptv(CODE_OR_EDX), %edx /* edx: exception_hander_vector */
	testl  %ecx, %ecx
	jz     fail_with_exception    /* if (!exception_hander_count) return NULL; */
#if sizeof_except_handler == 20
	leal  (%edx,%ecx,8), %edx     /* edx = edx + ecx * 8 */
	leal  (%edx,%ecx,8), %edx     /* edx = edx + ecx * 8 */
	leal  (%edx,%ecx,4), %edx     /* edx = edx + ecx * 4 */
#else
#error FIXME
#endif
1:	subl   $(sizeof_except_handler), %edx
	/* Check affect bounds. */
	cmpl   eh_start(%edx), %eax
	jb     2f /* if (ip_addr < eh_start) continue; */
	cmpl   eh_end(%edx), %eax
	jae    2f /* if (ip_addr >= eh_end) continue; */
	/* Check if the stored mask matches the currently raised exception.
	 * Sadly, we don't have a fourth scratch register, so we must improvise... */
	pushl  %eax
	ADJUST_CFA_OFFSET(4)
	movl   SP_OFFSET(4) L_THIS_THREAD, %eax
	movl   t_except(%eax), %eax /* eax = this_thread->t_except->ef_error */
	ASSERT_NONZERO(%eax)
	movl   ef_error(%eax), %eax /* eax = this_thread->t_except->ef_error */
	ASSERT_NONZERO(%eax)
	/* Check if the error is an interrupt and if so, make
	 * sure that the handler is allowed to process interrupts. */
	pushl  %ecx
	ADJUST_CFA_OFFSET(4)
	movl   ob_type(%eax), %ecx
	testw  $(TP_FINTERRUPT), tp_flags(%ecx)
	jz     3f /* Not an interrupt type. */
	/* Ensure that this is an interrupt handler. */
	testw  $(EXCEPTION_HANDLER_FINTERPT), eh_flags(%edx)
	jz     9f /* pop ECX; pop EAX; continue; */
3:	pushl  %edx
	ADJUST_CFA_OFFSET(4)
	movl   eh_mask(%edx), %edx
	testl  %edx, %edx
	jz     run_except_handler_in_pop_handled_x8 /* if (!eh_mask) execute_handler(); */
	pushl  %edx          /* current_except->eh_mask */
	ADJUST_CFA_OFFSET(4)
	pushl  ob_type(%eax) /* current_exception->ob_type */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeType_IsInherited,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	popl   %edx /* current_except */
	ADJUST_CFA_OFFSET(-4)
	popl   %ecx /* count */
	ADJUST_CFA_OFFSET(-4)
	popl   %eax /* ip_addr */
	ADJUST_CFA_OFFSET(-4)
	/* if (DeeObject_InstanceOf(current_exception,
	 *                          current_except->eh_mask))
	 *     goto exec_except; */
	jnz    run_except_handler_in_edx_handled
2:	loop   1b
fail_with_exception:
	unset_result %eax
	movl   $0, FRAME_RESULT
	jmp   .end_return
9:	popl   %ecx
	popl   %eax
	jmp    2b

run_except_handler_in_pop_handled_x8:
	popl   %edx
	addl   $8, %esp
run_except_handler_in_edx_handled:
	/* If the exception handler requests it, pre-handle the error. */
	testw  $(EXCEPTION_HANDLER_FHANDLED), eh_flags(%edx)
	jz     run_except_handler_in_edx
	/* Pre-handle the exception. */
	pushl  %edx
	ADJUST_CFA_OFFSET(4)
#ifdef CONFIG_NO_THREADS
	call   fSYM(DeeError_HandledNoSMP,0)
#else
	pushl  $(ERROR_HANDLED_INTERRUPT)
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeError_Handled,4)
	ADJUST_CFA_OFFSET(-4)
#endif
	popl   %edx
	ADJUST_CFA_OFFSET(-4)
run_except_handler_in_edx:
	/* Entry point: Setup and start executing an exception
	 *              handler whose descriptor is stored in EDX. */
	/* EDX: `struct except_handler *' */
#ifdef CODE_IS_REGISTER
	leal   co_code(CODE),    PC
	addl   eh_addr(%edx),    PC /* ip = code->co_code + eh_addr; */
#else
	movl   L_CODE,           PC
	addl   $(co_code),       PC
	addl   eh_addr(%edx),    PC /* ip = code->co_code + eh_addr; */
#endif
#ifndef NDEBUG
	LOAD_CODE(%eax)
	movl   co_codebytes(CODE_OR_EAX), %eax
	ASSERT_BELOW(eh_addr(%edx),%eax) /* ASSERT(eh_addr < %eax); */
#endif
	/* Figure out in what manner we need to adjust the stack. */
	movl   SP,             %ecx
	subl   FRAME_STACK,    %ecx
	shrl   $2,             %ecx /* ecx = current_stack_depth; */
	movzwl eh_stack(%edx), %eax /* eax = required_stack_depth; */
	cmpl   %eax, %ecx
	jb     2f /* if (current_stack_depth < required_stack_depth) goto 1f; */
	/* Adjust by popping values (potentially does nothing). */
	subl   %eax, %ecx
	jz    .disp /* Simply continue without adjustment when nothing needed to be done. */
1:	sp_popref    %eax, 0, 1, 0
	loop   1b
	jmp   .disp
2:	/* Adjust by pushing `none' (never does nothing) */
	subl   %ecx, %eax
	movl   %eax, %ecx
	ASSERT_NONZERO(%ecx)
	leal   SYM(DeeNone_Singleton), %eax
1:	sp_push %eax, 1
	loop   1b
	jmp   .disp



.section .rodata
99: .string "Stack overflow"
.text
stack_overflow:
#ifdef CONFIG_HAVE_EXEC_ALTSTACK
#if (DEE_EXEC_ALTSTACK_PERIOD & (DEE_EXEC_ALTSTACK_PERIOD-1)) != 0
#error "`DEE_EXEC_ALTSTACK_PERIOD' must be a power-of-2"
#endif
	andw   $(DEE_EXEC_ALTSTACK_PERIOD-1), %cx
	cmpw   $(DEE_EXEC_ALTSTACK_PERIOD-1), %cx
	jne    1f /* if (!IS_ALTSTACK_PERIOD(this_thread->t_execsz)) goto 1f; */
	cmpl   t_exec(%eax), FRAME
	jne    1f /* if (this_thread->t_exec != frame) goto 1f; */
	movl   FRAME_PREV, %edx
	movl   %edx, t_exec(%eax) /* this_thread->t_exec = frame->cf_prev; */
1:
#endif
	pushl  $99b
	ADJUST_CFA_OFFSET(4)
	pushl  $(SYM(DeeError_StackOverflow))
	ADJUST_CFA_OFFSET(4)
	call   SYM(DeeError_Throwf)
	addl   $8, %esp
	ADJUST_CFA_OFFSET(-8)
	xorl   %eax, %eax /* return NULL; */
	jmp   .end_without_unlink

#ifdef CONFIG_HAVE_EXEC_ALTSTACK
exec_on_altstack:
	cmpl   t_exec(%eax), FRAME
	/* if (this_thread->t_exec == frame) goto .Linc_execsz_start; */
	je     .Linc_execsz_start
	movl   t_exec(%eax), %ecx
	movl   %ecx, cf_prev(FRAME)
	movl   FRAME, t_exec(%eax)
	IF_USE_CFI(.cfi_remember_state)
	addl   $(LMEM), %esp /* Free memory used by local variables. */
	ADJUST_CFA_OFFSET(-LMEM)
	movl   FRAME, %ecx
#ifdef CODE_IS_REGISTER
	/* Use `%ebp' as the general-purpose register it is. */
	popl   %ebp
	IF_USE_CFI(.cfi_adjust_cfa_offset -4)
	IF_USE_CFI(.cfi_same_value %ebp)
#elif !defined(NDEBUG)
	movl   %ebp, %esp
	IF_USE_CFI(.cfi_def_cfa %esp, 28)
	popl   %ebp
	IF_USE_CFI(.cfi_adjust_cfa_offset -4)
	addl   $4, %esp
	IF_USE_CFI(.cfi_adjust_cfa_offset -4)
#endif
	popl   %edi
	IF_USE_CFI(.cfi_adjust_cfa_offset -4)
	IF_USE_CFI(.cfi_same_value %edi)
	popl   %esi
	IF_USE_CFI(.cfi_adjust_cfa_offset -4)
	IF_USE_CFI(.cfi_same_value %esi)
	popl   %ebx
	IF_USE_CFI(.cfi_adjust_cfa_offset -4)
	IF_USE_CFI(.cfi_same_value %ebx)

	/* Redirect to `DeeCode_ExecFrameSafeAltStack()' */
#ifdef CONFIG_HOST_WINDOWS
	jmp    @DeeCode_ExecFrameSafeAltStack@4
#else
	jmp    SYM(DeeCode_ExecFrameSafeAltStack)
#endif
	IF_USE_CFI(.cfi_restore_state)
#endif /* CONFIG_HAVE_EXEC_ALTSTACK */




invop:
	pushl  FRAME_PC
	ADJUST_CFA_OFFSET(4)
	pushl  CODE_OR_LOC
	ADJUST_CFA_OFFSET(4)
	call   fSYM(err_illegal_instruction,8)
	ADJUST_CFA_OFFSET(-8)
	jmp   .except



target_extended1:
	/* Jump forward to an F0-prefixed instruction. */
	lodsb
	jmp   *inst_table_f0(,%eax,4)



target_ret_none:
	unset_result %eax
	TEST_CODE_FLAG($(CODE_FYIELDING),%eax)
	jz     1f
	repeat_instruction
	movl   $(ITER_DONE), FRAME_RESULT
	jmp   .end_return
1:	movl   $(SYM(DeeNone_Singleton)), FRAME_RESULT
	incref_addr SYM(DeeNone_Singleton)
	jmp   .end_return


target_ret:
	/* Check if we're overwriting a previous return value
	 * (which can happen when `return' appears in a finally-block) */
	unset_result %eax
	sp_pop %eax
	movl   %eax, FRAME_RESULT
	TEST_CODE_FLAG($(CODE_FYIELDING),%ecx)
	jz     .end_return /* Execute finally handlers before returning. */
	jmp    .end_without_finally


target_setret:
	unset_result %eax
	sp_pop %eax
	movl   %eax, FRAME_RESULT
	jmp   .disp


target_yieldall:
	ASSERT_YIELDING
	unset_result %eax
	/* Yield the next item from the iterator. */
	pushl  TOP
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_IterNext,4)
	ADJUST_CFA_OFFSET(-4)
	/* Check for errors. */
	testl  %eax, %eax
	jz    .except
	/* Check for iterator-exhausted. */
	cmpl   $(ITER_DONE), %eax
	je     1f
	repeat_instruction
	movl   %eax, FRAME_RESULT
	jmp   .end_without_finally
1:	sp_popref %eax
	jmp   .disp



target_throw:
	pushl  TOP
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeError_Throw,4)
	ADJUST_CFA_OFFSET(-4)
	sp_popref %eax
	jmp   .except



target_rethrow:
	movl   L_THIS_THREAD,      %eax
	movw   t_exceptsz(%eax),   %cx
	cmpw   %cx, L_EXCEPT_RECURSION
	/* We've already thrown at least one exception, meaning
	 * we can simply go back to handling it again! */
	jb    .except /* if (except_recursion < this_thread->t_exceptsz) HANDLE_EXCEPT(); */
	/* Rethrow an exception of the caller. */
	movl   t_except(%eax),     %ecx
	testl  %ecx, %ecx
	/* if (!this_thread->t_except) err_no_active_exception(); */
	jz    .except_no_active_exception
	movl   ef_error(%ecx),     %ecx
	ASSERT_NONZERO(%ecx)
	/* Rethrow an exception of the caller. */
	pushl  %ecx
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeError_Throw,4)
	ADJUST_CFA_OFFSET(-4)
	jmp   .except
.except_no_active_exception:
	call   fSYM(err_no_active_exception,0)
	jmp   .except



target_endcatch:
	movl   L_THIS_THREAD, %eax
	movw   t_exceptsz(%eax), %ax
	cmpw   %ax, L_EXCEPT_RECURSION
#ifndef NDEBUG
	jbe    1f
	int    $3 /* ASSERT_FAIL: `except_recursion <= this_thread->t_exceptsz' */
1:
#endif
	je    .disp /* nothing left to handle. */
#ifdef CONFIG_NO_THREADS
	call   fSYM(DeeError_HandledNoSMP,0)
#else
	pushl  $(ERROR_HANDLED_INTERRUPT)
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeError_Handled,4)
	ADJUST_CFA_OFFSET(-4)
#endif
	jmp   .disp
target_endcatch_n:
	lodsb  /* EAX = nth_except */
	incl   %eax
	movzwl L_EXCEPT_RECURSION, %ecx
	addl   %eax, %ecx /* ECX = except_recursion+nth_except */
	movl   L_THIS_THREAD, %edx
	cmpw   %cx, t_exceptsz(%edx)
	jna   .disp /* if (!(this_thread->t_exceptsz > except_recursion + nth_except + 1)) goto .disp; */
	/* We're allowed to handle the `nth_except' exception. */
	decw   t_exceptsz(%edx) /* --this_thread->t_exceptsz; (The except we're going to remove)
	                         * The C version does this later, but if we do it now, we don't
	                         * have to reload the L_THIS_THREAD local variable into a register. */
	movl   %eax, %ecx       /* ECX = nth_except + 1 */
	leal   t_except(%edx), %eax
1:	movl   (%eax), %edx        /* frame = *pframe; */
	ASSERT_NONZERO(%edx)
	leal   ef_prev(%edx), %eax /* pframe = &frame->ef_prev; */
	loop   1b /* if (--ECX) goto 1b; */
	movl   (%eax), %edx        /* frame = *pframe; */
	ASSERT_NONZERO(%edx)
	/* Remove the exception frame from its chain. */
	movl   ef_prev(%edx), %ecx /* ECX = frame->ef_prev */
	movl   %ecx, (%eax)        /* *pframe = frame->ef_prev */
	/* Destroy the frame in question. (the frame is located in EDX) */
	movl   ef_trace(%edx), %eax
	decl   %eax
	cmpl   $-2, %eax
	jz     1f                  /* if (ITER_ISOK(frame->ef_trace)) Dee_Decref(frame->ef_trace); */
	incl   %eax
	decref %eax, 0, 0, 1       /* Preserve EDX */
1:	movl   ef_error(%edx), %eax/* Dee_Decref(frame->ef_error); */
	decref %eax, 0, 0, 1       /* Preserve EDX */
	pushl  %edx
	ADJUST_CFA_OFFSET(4)
	/* except_frame_free() */
#ifndef CONFIG_NO_OBJECT_SLABS
    /* WARNING: This assumes that sizeof(struct except_frame) >= 12 */
#if DeeSlab_HasSize(3)
	call   fSYM(DeeSlab_Free3,4) /* Free the frame */
#elif DeeSlab_HasSize(4)
	call   fSYM(DeeSlab_Free4,4) /* Free the frame */
#else
	call   fSYM(DeeSlab_Free,4)  /* Free the frame */
#endif
#else
	call   fSYM(Dee_Free,4)     /* Free the frame */
#endif
	ADJUST_CFA_OFFSET(-4)
	jmp   .disp




target_endfinally:
	/* Check if a return value has been assigned.
	 * If one has been, stop execution. */
	cmpl   $0, FRAME_RESULT
	jne   .end_return
	/* Check if unhandled exceptions have occurred, and if so: start handling them. */
	movl   L_THIS_THREAD, %eax
	movw   t_exceptsz(%eax), %ax
	cmpw   %ax, L_EXCEPT_RECURSION
	je    .disp
	jmp   .except
target_endfinally_n:
	lodsb  /* EAX = min_except */
	/* If a return value has been assigned, stop execution. */
	cmpl   $0, FRAME_RESULT
	jne   .end_return
	/* Check for errors, but only handle them if there are more than `min_except+1'. */
	movzwl L_EXCEPT_RECURSION, %ecx
	incl   %eax
	addl   %eax, %ecx /* ECX = except_recursion+min_except+1 */
	movl   L_THIS_THREAD, %edx
	cmpw   %cx, t_exceptsz(%edx)
	jna   .disp /* if (!(this_thread->t_exceptsz > except_recursion+min_except+1))
	             *       goto .disp; */
	jmp   .except



target_push_this_module:
	LOAD_CODE(%eax)
	movl   co_module(CODE_OR_EAX), %eax
	sp_push %eax, 1
	jmp   .disp



target16_push_bnd_extern:
	lodsw
	movl   %eax, %edx
	lodsw
	jmp    1f
target_push_bnd_extern:
	lodsb
	movl   %eax, %edx
	lodsb
1:	/* EAX: MID */
	/* EDX: GID */
	LOAD_CODE(%ecx)
	movl   co_module(CODE_OR_ECX), %ecx
	ASSERT_BELOW(%ax,mo_importc(%ecx))
	movl   mo_importv(%ecx), %ecx
	movl   (%ecx,%eax,4), %ecx
	/* ECX: `DeeModuleObject *' referenced_module */
	ASSERT_BELOW(%dx,mo_globalc(%ecx))
	movl   mo_globalv(%ecx), %eax
	movl  (%eax,%edx,4), %eax
.push_eax_nonnull:
	xorl   %edx, %edx
	testl  %eax, %eax
	setnz  %dl
	leal   SYM(Dee_FalseTrue)(,%edx,8), %eax
	sp_push %eax, 1 /* PUSHREF(DeeBool_For(EXTERNimm != NULL)) */
	jmp   .disp



target16_push_bnd_global:
	lodsw
	jmp    1f
target_push_bnd_global:
	lodsb
1:	LOAD_CODE(%ecx)
	movl   co_module(CODE_OR_ECX), %ecx
	ASSERT_BELOW(%ax,mo_globalc(%ecx))
	movl   mo_globalv(%ecx), %ecx
	movl   (%ecx,%eax,4), %eax
	jmp   .push_eax_nonnull



target16_push_bnd_local:
	lodsw
	jmp    1f
target_push_bnd_local:
	lodsb
1:	movl   FRAME_FRAME, %ecx
	movl   (%ecx,%eax,4), %eax
	jmp   .push_eax_nonnull



target_jf16:
	lodsw
	movswl %ax, %edx
	jmp    1f
target_jf:
	lodsb
	movsbl %al, %edx
1:	pushl  %edx
	ADJUST_CFA_OFFSET(4)
	pushl  TOP
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_Bool,4)
	ADJUST_CFA_OFFSET(-4)
	cmpl   $0, %eax
	jb    .except_x4
	popl   %edx /* Displacement offset */
	ADJUST_CFA_OFFSET(-4)
	jz     1f /* if (bool != 0) DISPATCH() */
	sp_popref %ecx /* POPREF() */
	jmp    .disp
1:
#ifndef CONFIG_NO_THREADS
	cmpl   $0, %edx
	jge    1f
	pushl  %edx
	ADJUST_CFA_OFFSET(4)
	pushl  SP_OFFSET(4) L_THIS_THREAD
	ADJUST_CFA_OFFSET(4)
	/* Check for interrupts when jumping back. */
	call   fSYM(DeeThread_CheckInterruptSelf,4)
	ADJUST_CFA_OFFSET(-4)
	testl  %eax, %eax
	jnz   .except_x4
	popl   %edx
	ADJUST_CFA_OFFSET(-4)
1:
#endif
	addl   %edx, PC /* ip += Simm8/16 */
	sp_popref %ecx /* POPREF() */
#ifndef NDEBUG
.disp_checkip:
	LOAD_CODE(%eax)
	leal   co_code(CODE_OR_EAX), %edx
	cmpl   %edx, PC
	jae    1f
	int    $3 /* ASSERT_FAIL: `ip >= code->co_code' */
1:	addl   co_codebytes(CODE_OR_EAX), %edx
	cmpl   %edx, PC
	jb     1f
	int    $3 /* ASSERT_FAIL: `ip < code->co_code + code->co_codebytes' */
1:
#else
.disp_checkip = .disp
#endif
	jmp   .disp



target_jt16:
	lodsw
	movswl %ax, %edx
	jmp    1f
target_jt:
	lodsb
	movsbl %al, %edx
1:	pushl  %edx
	ADJUST_CFA_OFFSET(4)
	pushl  TOP
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_Bool,4)
	ADJUST_CFA_OFFSET(-4)
	cmpl   $0, %eax
	jb    .except_x4
	popl   %edx /* Displacement offset */
	ADJUST_CFA_OFFSET(-4)
	jnz    1f /* if (bool == 0) DISPATCH() */
	sp_popref %ecx /* POPREF() */
	jmp    .disp
1:
#ifndef CONFIG_NO_THREADS
	cmpl   $0, %edx
	jge    1f
	pushl  %edx
	ADJUST_CFA_OFFSET(4)
	pushl  SP_OFFSET(4) L_THIS_THREAD
	ADJUST_CFA_OFFSET(4)
	/* Check for interrupts when jumping back. */
	call   fSYM(DeeThread_CheckInterruptSelf,4)
	ADJUST_CFA_OFFSET(-4)
	testl  %eax, %eax
	jnz   .except_x4
	popl   %edx
	ADJUST_CFA_OFFSET(-4)
1:
#endif
	addl   %edx, PC /* ip += Simm8/16 */
	sp_popref %ecx /* POPREF() */
	jmp   .disp_checkip



prefix_target_jf16:
	call   get_prefix_object
	pushl  %edx          /* BACKUP(prefix_ob) */
	ADJUST_CFA_OFFSET(4)
	pushl  %edx          /* DeeObject_Bool:self */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_Bool,4)
	ADJUST_CFA_OFFSET(-4)
	xorl   %ecx, %ecx
	cmpl   $0, %eax
	je    .Ldo_jmp_prefix_16   /* if (IS_FALSE) goto DO_JUMP; */
	jl    .except_decref_pop
	jmp   .Ldont_jmp_prefix_16 /* else goto DONT_JUMP; */
	ADJUST_CFA_OFFSET(-4)
prefix_target_jt16:
	call   get_prefix_object
	pushl  %edx          /* BACKUP(prefix_ob) */
	ADJUST_CFA_OFFSET(4)
	pushl  %edx          /* DeeObject_Bool:self */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_Bool,4)
	ADJUST_CFA_OFFSET(-4)
	xorl   %ecx, %ecx
	cmpl   $0, %eax
	je    .Ldont_jmp_prefix_16 /* if (IS_FALSE) goto DONT_JUMP; */
	jl    .except_decref_pop
.Ldo_jmp_prefix_16:
	popl   %edx
	ADJUST_CFA_OFFSET(-4)
	decref %edx
	lodsw
	movswl %ax, %eax
	addl   %eax, PC /* ip += Simm16 */
#ifndef NDEBUG
	jno   .disp_checkip
	int3   /* ASSERTION_FAILED: `ip += Simm16' did overflow. */
#endif
	jmp   .disp
	ADJUST_CFA_OFFSET(4)
.Ldont_jmp_prefix_16:
	popl   %edx
	ADJUST_CFA_OFFSET(-4)
	decref %edx
	addl   $2, PC
	jmp   .disp

prefix_target_jf:
	call   get_prefix_object
	pushl  %edx          /* BACKUP(prefix_ob) */
	ADJUST_CFA_OFFSET(4)
	pushl  %edx          /* DeeObject_Bool:self */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_Bool,4)
	ADJUST_CFA_OFFSET(-4)
	xorl   %ecx, %ecx
	cmpl   $0, %eax
	je    .Ldo_jmp_prefix_8   /* if (IS_FALSE) goto DO_JUMP; */
	jl    .except_decref_pop
	jmp   .Ldont_jmp_prefix_8 /* else goto DONT_JUMP; */
	ADJUST_CFA_OFFSET(-4)
prefix_target_jt:
	call   get_prefix_object
	pushl  %edx          /* BACKUP(prefix_ob) */
	ADJUST_CFA_OFFSET(4)
	pushl  %edx          /* DeeObject_Bool:self */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_Bool,4)
	ADJUST_CFA_OFFSET(-4)
	xorl   %ecx, %ecx
	cmpl   $0, %eax
	je    .Ldont_jmp_prefix_8 /* if (IS_FALSE) goto DONT_JUMP; */
	jl    .except_decref_pop
.Ldo_jmp_prefix_8:
	popl   %edx
	ADJUST_CFA_OFFSET(-4)
	decref %edx
	lodsb
	movsbl %al, %eax
	addl   %eax, PC /* ip += Simm8 */
#ifndef NDEBUG
	jno   .disp_checkip
	int3   /* ASSERTION_FAILED: `ip += Simm8' did overflow. */
#endif
	jmp   .disp
	ADJUST_CFA_OFFSET(4)
.Ldont_jmp_prefix_8:
	popl   %edx
	ADJUST_CFA_OFFSET(-4)
	decref %edx
	incl   PC
	jmp   .disp



target32_jmp:
	lodsl
#ifndef CONFIG_NO_THREADS
	cmpl   $0, %eax
	jge    1f
	pushl  %eax
	ADJUST_CFA_OFFSET(4)
	pushl  SP_OFFSET(4) L_THIS_THREAD
	ADJUST_CFA_OFFSET(4)
	/* Check for interrupts when jumping back. */
	call   fSYM(DeeThread_CheckInterruptSelf,4)
	ADJUST_CFA_OFFSET(-4)
	testl  %eax, %eax
	jnz   .except_x4
	popl   %eax
	ADJUST_CFA_OFFSET(-4)
1:
#endif
	addl   %eax, PC /* ip += Simm32 */
#ifndef NDEBUG
	jno   .disp_checkip
	int3   /* ASSERTION_FAILED: `ip += Simm32' did overflow. */
#endif
	jmp   .disp
target_jmp16:
	lodsw
	movswl %ax, %eax
#ifndef CONFIG_NO_THREADS
	cmpl   $0, %eax
	jge    1f
	pushl  %eax
	ADJUST_CFA_OFFSET(4)
	pushl  SP_OFFSET(4) L_THIS_THREAD
	ADJUST_CFA_OFFSET(4)
	/* Check for interrupts when jumping back. */
	call   fSYM(DeeThread_CheckInterruptSelf,4)
	ADJUST_CFA_OFFSET(-4)
	testl  %eax, %eax
	jnz   .except_x4
	popl   %eax
	ADJUST_CFA_OFFSET(-4)
1:
#endif
	addl   %eax, PC /* ip += Simm16 */
	jmp   .disp_checkip
target_jmp:
	lodsb
	movsbl %al, %eax
#ifndef CONFIG_NO_THREADS
	cmpl   $0, %eax
	jge    1f
	pushl  %eax
	ADJUST_CFA_OFFSET(4)
	pushl  SP_OFFSET(4) L_THIS_THREAD
	ADJUST_CFA_OFFSET(4)
	/* Check for interrupts when jumping back. */
	call   fSYM(DeeThread_CheckInterruptSelf,4)
	ADJUST_CFA_OFFSET(-4)
	testl  %eax, %eax
	jnz   .except_x4
	popl   %eax
	ADJUST_CFA_OFFSET(-4)
1:
#endif
	addl   %eax, PC /* ip += Simm8 */
	jmp   .disp_checkip



target_foreach16:
	lodsw
	movswl %ax, %eax
	jmp    1f
target_iterself:
	/* Put here for the optimization of us
	 * falling through if the next op is foreach. */
	pushl  TOP
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_IterSelf,4)
	ADJUST_CFA_OFFSET(-4)
	testl  %eax, %eax
	jz    .except /* Check for errors. */
	xchgl  TOP, %eax /* Store the result onto the stack. */
	decref %eax
	cmpb   $(ASM_FOREACH), 0(PC)
	jne   .disp /* Unlikely case: some other opcode follows. */
	incl   PC /* Adjust to advance past the FOREACH opcode. */
	/* Fall through to start a foreach loop */
target_foreach:
	lodsb
	movsbl %al, %eax
1:	pushl  %eax
	ADJUST_CFA_OFFSET(4)
	pushl  TOP
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_IterNext,4)
	ADJUST_CFA_OFFSET(-4)
	popl   %edx /* Jump-offset */
	ADJUST_CFA_OFFSET(-4)
	testl  %eax, %eax
	jz    .except /* Check for exception */
	cmpl   $(ITER_DONE), %eax
	je     2f /* Check for iterator exhaustion */
	/* Leave the iterator and push the element. */
	sp_push %eax
	jmp   .disp
2:
#ifndef CONFIG_NO_THREADS
	cmpl   $0, %edx
	jge    1f
	pushl  %edx
	ADJUST_CFA_OFFSET(4)
	pushl  SP_OFFSET(4) L_THIS_THREAD
	ADJUST_CFA_OFFSET(4)
	/* Check for interrupts when jumping back. */
	call   fSYM(DeeThread_CheckInterruptSelf,4)
	ADJUST_CFA_OFFSET(-4)
	testl  %eax, %eax
	jnz   .except_x4
	popl   %edx
	ADJUST_CFA_OFFSET(-4)
1:
#endif
	addl   %edx, PC /* ip += Simm8/16 */
	sp_popref %eax
	jmp   .disp_checkip



target_jmp_pop:
	subl   $4, %esp /* absip */
	ADJUST_CFA_OFFSET(4)
	pushl  %esp /* &absip */
	ADJUST_CFA_OFFSET(4)
	pushl  TOP
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_AsUInt32,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jnz   .except_x4 /* Check for exception */
	sp_popref %eax
.jmp_abs_pop:
	popl   %edx /* absip */
	ADJUST_CFA_OFFSET(-4)
.jmp_abs_edx:
	/* At this point, the absolute PC is stored in EDX */
#ifdef CONFIG_NO_THREADS
#ifdef CODE_IS_REGISTER
	leal   co_code(CODE), PC
	addl   %edx,          PC
#else
	movl   L_CODE,   PC
	addl   $co_code, PC
	addl   %edx,     PC
#endif
#else
#ifdef CODE_IS_REGISTER
	leal   co_code(CODE), %eax
	addl   %edx,          %eax
#else
	movl   L_CODE,   %eax
	addl   $co_code, %eax
	addl   %edx,     %eax
#endif
	cmpl   PC, %eax
	jge    1f
	pushl  %eax
	ADJUST_CFA_OFFSET(4)
	pushl  SP_OFFSET(4) L_THIS_THREAD
	ADJUST_CFA_OFFSET(4)
	/* Check for interrupts when jumping back. */
	call   fSYM(DeeThread_CheckInterruptSelf,4)
	ADJUST_CFA_OFFSET(-4)
	testl  %eax, %eax
	jnz   .except_x4
	popl   %eax
	ADJUST_CFA_OFFSET(-4)
1:	movl   %eax,     PC
#endif
	jmp   .disp_checkip

target_jmp_pop_pop:
	subl   $8, %esp /* absstk, absip */
	ADJUST_CFA_OFFSET(-8)
	pushl  %esp /* &absstk */
	ADJUST_CFA_OFFSET(4)
	pushl  TOP
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_AsUInt16,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jnz   .except_x8 /* Check for exception */
	leal   4(%esp), %eax
	pushl  %eax /* &absip */
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_AsUInt32,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jnz   .except_x8 /* Check for exception */
	/* All right! `absstk == 0(%esp)', `absip == 4(%esp)' */
	popl   %eax /* eax = required_stack_depth */
	ADJUST_CFA_OFFSET(-4)
	movzwl %ax, %eax /* Make sure to clear the upper 2 bytes */
	movl   SP,          %ecx
	subl   FRAME_STACK, %ecx
	shrl   $2,          %ecx /* ecx = current_stack_depth; */
	cmpl   %eax, %ecx
	jb     2f /* if (current_stack_depth < required_stack_depth) goto 1f; */
	/* Adjust by popping values (potentially do nothing). */
	subl   %eax, %ecx
	jz    .jmp_abs_pop /* Simply continue without adjustment when nothing needed to be done. */
1:	sp_popref %eax, 0, 1, 0 /* Preserve ECX in during this operation. */
	loop   1b
	jmp   .jmp_abs_pop
2:	/* Adjust by pushing `none' (never does nothing) */
	subl   %ecx, %eax
	movl   %eax, %ecx
	ASSERT_NONZERO(%ecx)
	leal   SYM(DeeNone_Singleton), %eax
1:	sp_push %eax, 1
	loop   1b
	jmp   .jmp_abs_pop /* Pop the target address from the stack and jump to it. */
ADJUST_CFA_OFFSET(-4)



target_call:
	lodsb  /* eax = n_args */
	leal   (,%eax,4), %ecx
	movl   SP, %edx
	subl   %ecx, %edx
	pushl  %eax /* backup(n_args) */
	ADJUST_CFA_OFFSET(4)
	pushl  %edx /* backup(SP - n_args) */
	ADJUST_CFA_OFFSET(4)
	pushl  %edx /* SP - n_args */
	ADJUST_CFA_OFFSET(4)
	pushl  %eax /* n_args */
	ADJUST_CFA_OFFSET(4)
	pushl  -4(%edx) /* (SP - n_args)[-1] */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_Call,12)
	ADJUST_CFA_OFFSET(-12)
	testl  %eax, %eax
	jz    .except_x8
	popl   %edx /* EDX = SP - n_args; */
	ADJUST_CFA_OFFSET(-4)
	xchgl  -4(%edx), %eax /* Store the result onto the stack. */
	decref %eax /* decref(EAX) (The function that was called) */
	popl   %ecx /* ECX = n_args */
	ADJUST_CFA_OFFSET(-4)
	testl  %ecx, %ecx
	jz    .disp
1:	sp_popref %eax, 0, 1, 0
	loop   1b
	jmp   .disp


target_call_seq:
	lodsb  /* EAX = n_args */
	leal   (,%eax,4), %ecx /* ECX = n_args*4 */
	movl   SP, %edx
	subl   %ecx, %edx      /* EDX = new_sp = SP - n_args */
	pushl  %edx            /* backup(new_sp) */
	ADJUST_CFA_OFFSET(4)
	pushl  %edx            /* argv */
	ADJUST_CFA_OFFSET(4)
	pushl  %eax            /* argc */
	ADJUST_CFA_OFFSET(4)
	/* Construct the shared vector, which inherits the stack-arguments. */
	call   fSYM(DeeSharedVector_NewShared,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jz    .except_x4
	popl   SP              /* SP = new_sp (from the backup) */
	ADJUST_CFA_OFFSET(-4)
	pushl  %eax            /* backup(shared_vector) / callback-argv[0] */
	ADJUST_CFA_OFFSET(4)
	pushl  %esp            /* argv (for DeeObject_Call) -- &shared_vector */
	ADJUST_CFA_OFFSET(4)
	pushl  $1              /* argc (for DeeObject_Call) -- 1 */
	ADJUST_CFA_OFFSET(4)
	pushl  TOP             /* The sequence type being initialized */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_Call,12) /* Construct the sequence using the shared-vector. */
	ADJUST_CFA_OFFSET(-12)
	testl  %eax, %eax
	jz     99f
	/* Save the result of `DeeObject_Call' on the stack,
	 * exchanging the sequence type that was invoked. */
	xchgl  %eax, TOP
	decref %eax /* Decref the sequence type. */
	/* At this point, backup(shared_vector) is still located in the stack,
	 * and assuming that `DeeObject_Call()' didn't modify its argv (which
	 * it's not allowed to), we can simply invoke `DeeSharedVector_Decref()'
	 * to release vector-ownership of the shared-vector. */
	call   fSYM(DeeSharedVector_Decref,4)
	ADJUST_CFA_OFFSET(-4)
	jmp   .disp
	ADJUST_CFA_OFFSET(4)
99:	/* Must still release vector ownership (shared_vector is still in 0(%esp)) */
	call   fSYM(DeeSharedVector_Decref,4)
	ADJUST_CFA_OFFSET(-4)
	jmp   .except


target_call_map:
	lodsb  /* EAX = n_args */
	leal   (,%eax,8), %ecx /* ECX = n_args*4 */
	movl   SP, %edx
	subl   %ecx, %edx      /* EDX = new_sp = SP - n_args*2 */
	pushl  %edx            /* backup(new_sp) */
	ADJUST_CFA_OFFSET(4)
	pushl  %edx            /* argv */
	ADJUST_CFA_OFFSET(4)
	pushl  %eax            /* argc */
	ADJUST_CFA_OFFSET(4)
	/* Construct the shared vector, which inherits the stack-arguments. */
	call   fSYM(DeeSharedMap_NewShared,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jz    .except_x4
	popl   SP              /* SP = new_sp (from the backup) */
	ADJUST_CFA_OFFSET(-4)
	pushl  %eax            /* backup(shared_map) / callback-argv[0] */
	ADJUST_CFA_OFFSET(4)
	pushl  %esp            /* argv (for DeeObject_Call) -- &shared_map */
	ADJUST_CFA_OFFSET(4)
	pushl  $1              /* argc (for DeeObject_Call) -- 1 */
	ADJUST_CFA_OFFSET(4)
	pushl  TOP             /* The sequence type being initialized */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_Call,12) /* Construct the sequence using the shared-vector. */
	ADJUST_CFA_OFFSET(-12)
	testl  %eax, %eax
	jz     99f
	/* Save the result of `DeeObject_Call' on the stack,
	 * exchanging the sequence type that was invoked. */
	xchgl  %eax, TOP
	decref %eax /* Decref the sequence type. */
	/* At this point, backup(shared_map) is still located in the stack,
	 * and assuming that `DeeObject_Call()' didn't modify its argv (which
	 * it's not allowed to), we can simply invoke `DeeSharedMap_Decref()'
	 * to release vector-ownership of the shared-vector. */
	call   fSYM(DeeSharedMap_Decref,4)
	ADJUST_CFA_OFFSET(-4)
	jmp   .disp
	ADJUST_CFA_OFFSET(4)
99:	/* Must still release vector ownership (shared_map is still in 0(%esp)) */
	call   fSYM(DeeSharedMap_Decref,4)
	ADJUST_CFA_OFFSET(-4)
	jmp   .except



target_call_tuple:
	pushl  FIRST
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_CallTuple,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jz    .except /* Check for error */
	xchgl  SECOND, %eax /* Push the function return value. */
	decref %eax /* Decref the called function. */
	sp_popref %eax /* Pop the argument tuple */
	jmp   .disp


target_thiscall_tuple:
	pushl  FIRST   /* DeeObject_ThisCallTuple:args */
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND  /* DeeObject_ThisCallTuple:this_arg */
	ADJUST_CFA_OFFSET(4)
	pushl  THIRD   /* DeeObject_ThisCallTuple:self */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_ThisCallTuple,12)
	ADJUST_CFA_OFFSET(-12)
	testl  %eax, %eax
	jz    .except
	xchgl  THIRD, %eax
	decref %eax
	sp_popref %eax
	sp_popref %eax
	jmp   .disp



target16_call_kw:
	lodsb
	movzbl %al, %ecx /* argc */
	lodsw
	jmp    1f
target_call_kw:
	lodsb
	movzbl %al, %ecx /* argc */
	lodsb
1:	LOAD_CODE(%edx)
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_EDX))
	movl   co_staticv(CODE_OR_EDX), %edx
	subl   $8, %esp      /* RESERVE(backup(new_sp),backup(argc)) */
	ADJUST_CFA_OFFSET(4)
	pushl  (%edx,%eax,4) /* DeeObject_CallKw:kw */
	ADJUST_CFA_OFFSET(4)
	movl   SP, %eax
	leal   (,%ecx,4), %edx
	subl   %edx, %eax    /* EAX = new_sp = SP - argc */
	movl   %eax, 4(%esp) /* backup(new_sp) = new_sp */
	movl   %ecx, 8(%esp) /* backup(argc) = argc */
	pushl  %eax          /* DeeObject_CallKw:argv */
	ADJUST_CFA_OFFSET(4)
	pushl  %ecx          /* DeeObject_CallKw:argc */
	ADJUST_CFA_OFFSET(4)
	pushl  -4(%eax)      /* DeeObject_CallKw:self */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_CallKw,16)
	ADJUST_CFA_OFFSET(-16)
	testl  %eax, %eax
	jz    .except_x8
	popl   %edx          /* EDX = new_sp */
	ADJUST_CFA_OFFSET(-4)
	xchgl  %eax, -4(%edx)/* Save result on the stack. */
	decref %eax
	popl   %ecx          /* ECX = argc */
	ADJUST_CFA_OFFSET(-4)
	/* Pop arguments form the stack. */
	testl  %ecx, %ecx
	jz    .disp
1:	sp_popref %eax, 0, 1, 0
	loop   1b
	jmp   .disp



target16_call_tuple_kw:
	lodsw
	jmp    1f
target_call_tuple_kw:
	lodsb
1:	LOAD_CODE(%ecx)
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
	movl   co_staticv(CODE_OR_ECX), %ecx
	pushl  (%ecx,%eax,4) /* DeeObject_CallTupleKw:kwds */
	ADJUST_CFA_OFFSET(4)
	pushl  FIRST         /* DeeObject_CallTupleKw:args */
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND        /* DeeObject_CallTupleKw:self */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_CallTupleKw,12)
	ADJUST_CFA_OFFSET(-12)
	testl  %eax, %eax
	jz    .except
	xchgl  %eax, SECOND  /* Save the result on the stack. */
	decref %eax
	sp_popref %eax
	jmp   .disp


target_call_tuple_kwds:
	pushl   FIRST        /* DeeObject_CallTupleKw:kw */
	ADJUST_CFA_OFFSET(4)
	pushl   SECOND       /* DeeObject_CallTupleKw:args */
	ADJUST_CFA_OFFSET(4)
	pushl   THIRD        /* DeeObject_CallTupleKw:self */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeObject_CallTupleKw,12)
	ADJUST_CFA_OFFSET(-12)
	testl   %eax, %eax
	jz     .except
	xchgl   %eax, THIRD  /* Save the result on the stack. */
	decref  %eax
	sp_popref %eax
	sp_popref %eax
	jmp    .disp



target16_operator:
	lodsw
	jmp    1f
target_operator:
	lodsb
1:	subl   $12, %esp /* pending... */
	ADJUST_CFA_OFFSET(12)
	pushl  %ecx /* n_args */
	ADJUST_CFA_OFFSET(4)
	pushl  %eax /* operator_name. */
	ADJUST_CFA_OFFSET(4)
	pushl  TOP
	ADJUST_CFA_OFFSET(4)
	xorl   %eax, %eax
	lodsb
	movl   SP, %edx
	leal   (,%eax,4), %ecx
	subl   %ecx, %edx /* EDX = SP - n_args. */
	movl   %eax, 20(%esp) /* BACKUP(n_args). */
	movl   %edx, 16(%esp) /* BACKUP(SP - n_args). */
	movl   %edx, 12(%esp) /* SP - n_args. */
	call   fSYM(DeeObject_InvokeOperator,16)
	ADJUST_CFA_OFFSET(-16)
	testl  %eax, %eax
	jz    .except_x8
	popl   %edx /* EDX = SP - n_args */
	ADJUST_CFA_OFFSET(-4)
	popl   %ecx /* ECX = n_args */
	ADJUST_CFA_OFFSET(-4)
	xchgl  %eax, -4(%edx) /* Save the result onto the stack. */
	decref %eax, 0, 1, 0
	testl  %ecx, %ecx /* Pop arguments. */
	jz    .disp
1:	sp_popref %eax, 0, 1, 0
	loop   1b
	jmp   .disp

target16_operator_tuple:
	lodsw
	jmp    1f
target_operator_tuple:
	lodsb
1:	movl   FIRST, %edx
	WASSERT_EQUAL(l,ob_type(%edx),$(SYM(DeeTuple_Type)))
	leal   t_elem(%edx), %ecx
	pushl  %ecx         /* argv */
	ADJUST_CFA_OFFSET(4)
	pushl  t_size(%edx) /* argc */
	ADJUST_CFA_OFFSET(4)
	pushl  %eax         /* operator_name */
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND       /* self */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_InvokeOperator,16)
	ADJUST_CFA_OFFSET(-16)
	testl  %eax, %eax
	jz    .except
	xchgl  SECOND, %eax /* Save the result onto the stack. */
	decref %eax
	sp_popref %eax
	jmp   .disp

prefix_target16_operator:
	call   get_prefix_object
	lodsw
	jmp    1f
prefix_target_operator:
	call   get_prefix_object
	lodsb
1:	subl   $16, %esp /* pending... (SP - n_args, backup(SP - n_args), backup(n_args), prefix_object) */
	ADJUST_CFA_OFFSET(16)
	movl   %edx, 12(%esp) /* prefix_object */
	leal   12(%esp), %edx /* EDX = &prefix_object */
	pushl  %ecx /* n_args */
	ADJUST_CFA_OFFSET(4)
	pushl  %eax /* operator_name. */
	ADJUST_CFA_OFFSET(4)
	pushl  %edx /* &prefix_object */
	ADJUST_CFA_OFFSET(4)
	xorl   %eax, %eax
	lodsb
	movl   SP, %edx
	leal   (,%eax,4), %ecx
	subl   %ecx, %edx     /* EDX = SP - n_args. */
	movl   %eax, 20(%esp) /* BACKUP(n_args). */
	movl   %edx, 16(%esp) /* BACKUP(SP - n_args). */
	movl   %edx, 12(%esp) /* SP - n_args. */
	call   fSYM(DeeObject_PInvokeOperator,16)
	ADJUST_CFA_OFFSET(-16)
	testl  %eax, %eax
	jz     99f
	popl   %edx /* EDX = SP - n_args */
	ADJUST_CFA_OFFSET(-4)
	popl   %ecx /* ECX = n_args */
	ADJUST_CFA_OFFSET(-4)
	xchgl  %eax, -4(%edx) /* Save the result onto the stack. */
	decref %eax, 0, 1, 0
	testl  %ecx, %ecx /* Pop arguments. */
	jz     1f
2:	sp_popref %eax, 0, 1, 0
	loop   2b
1:	popl   %edx /* Finally, pop the object being prefixed and write it back. */
	jmp    set_prefix_object
	ADJUST_CFA_OFFSET(8)
99:	addl   $8, %esp
	ADJUST_CFA_OFFSET(-8)
	jmp   .except_decref_pop
ADJUST_CFA_OFFSET(-4)


prefix_target16_operator_tuple:
	call   get_prefix_object
	lodsw
	jmp    1f
prefix_target_operator_tuple:
	call   get_prefix_object
	lodsb
1:	movl   TOP, %ecx
	WASSERT_EQUAL(l,ob_type(%ecx),$(SYM(DeeTuple_Type)))
	pushl  %edx /* Save the prefixed object onto the stack, so we can address it. */
	ADJUST_CFA_OFFSET(4)
	leal   t_elem(%ecx), %edx
	pushl  %edx         /* argv */
	ADJUST_CFA_OFFSET(4)
	pushl  t_size(%ecx) /* argc */
	ADJUST_CFA_OFFSET(4)
	pushl  %eax         /* operator_name */
	ADJUST_CFA_OFFSET(4)
	leal   12(%esp), %eax
	pushl  %eax         /* &prefix_object */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_PInvokeOperator,16)
	ADJUST_CFA_OFFSET(-16)
	testl  %eax, %eax
	jz    .except_decref_pop
	xchgl  TOP, %eax    /* Save the result onto the stack. */
	decref %eax
	popl   %edx         /* Write back the new prefix object. */
	ADJUST_CFA_OFFSET(-4)
	jmp    set_prefix_object



target16_del_global:
	lodsw
	jmp    1f
target_del_global:
	lodsb
1:	LOAD_CODE(%ecx)
	movl   co_module(CODE_OR_ECX), %ecx
	/* Must lock global variables. */
#ifndef CONFIG_NO_THREADS
	pushl  %eax /* Preserve EAX */
	ADJUST_CFA_OFFSET(4)
	rwlock_write mo_lock(%ecx), %edx
	popl   %eax
	ADJUST_CFA_OFFSET(-4)
	pushl  %ecx /* `DeeModuleObject *': Popped before endwrite */
	ADJUST_CFA_OFFSET(4)
#endif /* !CONFIG_NO_THREADS */
	ASSERT_BELOW(%ax,mo_globalc(%ecx))
	movl   mo_globalv(%ecx), %edx
	xorl   %ecx, %ecx
	xchgl  %ecx, (%edx,%eax,4) /* Exchange the variable with NULL. */
#ifndef CONFIG_NO_THREADS
	popl   %eax
	ADJUST_CFA_OFFSET(-4)
	rwlock_endwrite mo_lock(%eax)
#endif /* !CONFIG_NO_THREADS */
	/* Drop a reference from the exchanged value when it is non-NULL. */
	testl  %ecx, %ecx
	jz    .disp
	decref %ecx
	jmp   .disp



target16_del_local:
	lodsw
	jmp     1f
target_del_local:
	lodsb
1:	movl    FRAME_FRAME, %ecx
	xorl    %edx, %edx
	xchgl   %ecx, (%ecx,%eax,4) /* Exchange the local variable with `NULL' */
	/* Drop a reference from the exchanged value when it is non-NULL. */
	testl   %ecx, %ecx
	jz     .disp
	decref  %ecx
	jmp    .disp



target_swap:
	movl    FIRST, %eax
	xchgl   SECOND, %eax
	movl    %eax, FIRST
	jmp    .disp



target16_lrot:
	lodsw
	jmp     1f
target_lrot:
	lodsb
	/* NOTE: This assembly implementation is a bit complicated.
	 *       If you wish to understand what happens here,
	 *       I suggest looking at the C implementation. */
1:	leal    12(,%eax,4), %ecx /* SHIFT = (eax * 4) + 12; (aka. `SHIFT = (eax + 3) * 4;') */
	movl    SP, %edx
	subl    %ecx, %edx /* EDX = SP - SHIFT; */
	pushl  (%edx) /* SAVE: *(SP - SHIFT); */
	ADJUST_CFA_OFFSET(4)
#ifdef __KOS__
	decl    %eax
	pushl   %eax /* memmove.arg(2): SHIFT - 1; */
	ADJUST_CFA_OFFSET(4)
#else
	subl    $4, %ecx
	pushl   %ecx /* memmove.arg(2): SHIFT - 1; */
	ADJUST_CFA_OFFSET(4)
#endif
	pushl   %edx /* memmove.arg(1): SP - (SHIFT - 1); */
	ADJUST_CFA_OFFSET(4)
	addl    $4, (%esp)
	pushl   %edx /* memmove.arg(0): SP - SHIFT; */
	ADJUST_CFA_OFFSET(4)
#ifdef __KOS__
	call    SYM(memmovel)
#else
	/* XXX: Assumes that libc's `memmove()' uses cdecl. (Can we assume this?) */
	call    SYM(memmove)
#endif
	addl    $12, %esp
	ADJUST_CFA_OFFSET(-12)
	popl    TOP /* SAVE --> TOP */
	ADJUST_CFA_OFFSET(-4)
	jmp    .disp
target16_rrot:
	lodsw
	jmp     1f
target_rrot:
	lodsb
	/* NOTE: This assembly implementation is a bit complicated.
	 *       If you wish to understand what happens here,
	 *       I suggest looking at the C implementation. */
1:	leal    12(,%eax,4), %ecx /* SHIFT = (eax * 4) + 12; (aka. `SHIFT = (eax + 3) * 4;') */
	movl    SP, %edx
	subl    %ecx, %edx /* EDX = SP - SHIFT; */
	pushl   TOP /* SAVE: *(SP - 1); */
	ADJUST_CFA_OFFSET(4)
	pushl   %edx /* SAVE: SP - SHIFT; */
	ADJUST_CFA_OFFSET(4)
#ifdef __KOS__
	decl    %eax
	pushl   %eax /* memmove.arg(2): SHIFT - 1; */
	ADJUST_CFA_OFFSET(4)
#else
	addl    $4, %ecx
	pushl   %ecx /* memmove.arg(2): SHIFT - 1; */
	ADJUST_CFA_OFFSET(4)
#endif
	pushl   %edx /* memmove.arg(1): SP - SHIFT; */
	ADJUST_CFA_OFFSET(4)
	addl    $4, %edx
	pushl   %edx /* memmove.arg(0): SP - (SHIFT - 1); */
	ADJUST_CFA_OFFSET(4)
#ifdef __KOS__
	call    SYM(memmovel)
#else
	/* XXX: Assumes that libc's `memmove()' uses cdecl. (Can we assume this?) */
	call    SYM(memmove)
#endif
	addl    $12, %esp
	ADJUST_CFA_OFFSET(-12)
	pop     %edx /* SP - SHIFT; */
	ADJUST_CFA_OFFSET(-4)
	pop    (%edx) /* *(SP - SHIFT) = OLD(*(SP - 1)); */
	ADJUST_CFA_OFFSET(-4)
	jmp    .disp



target_dup:
	movl    TOP, %eax
	sp_push %eax, 1
	jmp    .disp
target16_dup_n:
	lodsw
	jmp     1f
target_dup_n:
	lodsb
1:	/* dup #n loads an object offset by n+2:
	 * Why not +0 or +1?
	 *    +0: Would allow for illegal access to `SP-0',
	 *        which points to the next free SP slot and
	 *        doesn't contain a valid object
	 *    +1: Would lead to an ambiguity with `dup' if
	 *        duplicating the top-slot had two ways of
	 *        being encoded.
	 * >> Therefor, we add an offset of +2, preventing
	 *    redundancy as well as the possibility of an
	 *    illegal memory access. */
	leal    8(,%eax,4), %eax /* eax = (eax * 4) + 8 (aka. `eax = (eax + 2) * 4') */
	movl    SP, %edx
	subl    %eax, %edx
	ASSERT_ABOVE_EQUAL(%edx,FRAME_STACK) /* ERROR: Stack underflow. */
	ASSERT_BELOW(%edx,L_STACK_END) /* ERROR: Stack overflow. */
	movl   (%edx), %edx /* Dereference the requested stack object. */
	sp_push %edx, 1 /* Re-push it as a new reference. */
	jmp    .disp



target_pop:
	sp_popref %eax
	jmp    .disp
target16_pop_n:
	lodsw
	jmp     1f
target_pop_n:
	lodsb
1:	leal    8(,%eax,4), %eax /* eax = (eax * 4) + 8. (offset = (offset + 2) * 4) */
	movl    TOP, %ecx
	movl    SP, %edx
	subl    %eax, %edx
	xchgl  (%edx), %ecx /* Write the value of TOP to the pop target. */
	subl    $4, SP /* Pop top. */
	decref  %ecx /* Drop the old object that was overwritten. */
	jmp    .disp



target16_adjstack:
	lodsw
	movswl  %ax, %ecx
	jmp     1f
target_adjstack:
	lodsb
	movsbl  %al, %ecx
1:	cmpl    $0, %ecx
	jge     2f /* if (ECX >= 0) goto 2f; */
	/* while (imm_val++) POPREF(); */
	negl    %ecx
1:	sp_popref %eax, 0, 1, 0 /* Preserve ECX. */
	loop    1b
	jmp    .disp
2:	jz     .disp /* Unlikely case: ECX == 0 (Would indicate an redundant instruction, but is allowed by the specs) */
	/* NOTE: We could write this part as `lock addl %ecx, ob_refcnt(%eax); rep stosl', but since we're usually dealing
	 *       with very small numbers here, the resulting overhead would be too much. */
	leal    SYM(DeeNone_Singleton), %eax
	/* while (imm_val--) PUSHREF(Dee_None); */
1:	sp_push %eax, 1
	loop    1b
	jmp    .disp



target_super:
	/* Create the super-wrapper. */
	pushl   SECOND
	ADJUST_CFA_OFFSET(4)
	pushl   FIRST
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeSuper_New,8)
	ADJUST_CFA_OFFSET(-8)
	testl   %eax, %eax
	jz     .except
	xchgl   SECOND, %eax /* Store the result onto the stack. */
	decref  %eax
	sp_popref %eax
	jmp    .disp

target16_super_this_r:
	lodsw
	jmp     1f
target_super_this_r:
	lodsb
1:
#ifndef NDEBUG
	LOAD_CODE(%ecx)
	ASSERT_BELOW(%ax,co_refc(CODE_OR_ECX))
#endif
	pushl   FRAME_THIS
	ADJUST_CFA_OFFSET(4)
	movl    FRAME_FUNC, %ecx
	pushl   fo_refv(%ecx,%eax,4)
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeSuper_New,8)
	ADJUST_CFA_OFFSET(-8)
	testl   %eax, %eax
	jz     .except
	sp_push %eax
	jmp    .disp
target16_super_this_e:
	lodsw
	LOAD_CODE(%ecx)
	movl    co_module(CODE_OR_ECX), %ecx
	ASSERT_BELOW(%ax,mo_importc(%ecx))
	movl    mo_importv(%ecx), %ecx
	movl   (%ecx,%eax,4), %ecx
	lodsw
	jmp     1f
target_super_this_e:
	lodsb
	LOAD_CODE(%ecx)
	movl    co_module(CODE_OR_ECX), %ecx
	ASSERT_BELOW(%ax,mo_importc(%ecx))
	movl    mo_importv(%ecx), %ecx
	movl   (%ecx,%eax,4), %ecx
	lodsb
	jmp     1f
target16_super_this_g:
	lodsw
	jmp     2f
target_super_this_g:
	lodsb
2:	LOAD_CODE(%ecx)
	movl    co_module(CODE_OR_ECX), %ecx
1:	ASSERT_BELOW(%ax,mo_globalc(%ecx))
#ifndef CONFIG_NO_THREADS
	pushl   %eax
	ADJUST_CFA_OFFSET(4)
	rwlock_read mo_lock(%ecx), %edx
	popl    %eax
	ADJUST_CFA_OFFSET(-4)
#endif
	movl    mo_globalv(%ecx), %edx
	movl   (%edx,%eax,4), %edx
	testl   %edx, %edx
	jz      99f
	incref  %edx
#ifndef CONFIG_NO_THREADS
	rwlock_endread mo_lock(%ecx)
#endif
	pushl   %edx /* Save EDX */
	ADJUST_CFA_OFFSET(4)
	pushl   FRAME_THIS
	ADJUST_CFA_OFFSET(4)
	pushl   %edx
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeSuper_New,8)
	ADJUST_CFA_OFFSET(-8)
	popl    %edx
	ADJUST_CFA_OFFSET(-4)
	decref  %edx, 1, 0, 0 /* Preserve EAX */
	testl   %eax, %eax
	jz     .except
	sp_push %eax /* Push the new super-wrapper. */
	jmp    .disp
99:
#ifndef CONFIG_NO_THREADS
	rwlock_endread mo_lock(%ecx)
#endif
	pushl   %eax /* GID */
	ADJUST_CFA_OFFSET(4)
	pushl   %ecx /* MODULE */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(err_unbound_global,8)
	ADJUST_CFA_OFFSET(-8)
	jmp    .except



target_isnone:
	movl    TOP, %ecx
	/* Manually do the decref() due to our custom code to restore ECX. */
	LOCK_PREFIX subl $1, ob_refcnt(%ecx)
	jnz     1f
	pushl   %ecx
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeObject_Destroy,4)
	ADJUST_CFA_OFFSET(-4)
	movl    TOP, %ecx
	xorl    %eax, %eax
1:	cmpl    $(SYM(DeeNone_Singleton)), %ecx
	/* NOTE: No need to clear EAX because dispatch will
	 *       have left all but the lower 8 bits clear,
	 *       because EAX contains the instruction being
	 *       executed opon target entry. */
	sete    %al
	leal    SYM(Dee_FalseTrue)(,%eax,8), %eax
	incref  %eax
	movl    %eax, TOP
	jmp    .disp



target16_pop_static:
	lodsw
	jmp     1f
target_pop_static:
	lodsb
1:	LOAD_CODE(%ecx)
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
#ifndef CONFIG_NO_THREADS
	pushl   %eax
	ADJUST_CFA_OFFSET(4)
	rwlock_write co_static_lock(CODE_OR_ECX), %edx
	popl    %eax
	ADJUST_CFA_OFFSET(-4)
#endif
	sp_pop  %edx
	movl    co_staticv(CODE_OR_ECX), %ecx
	xchgl  (%ecx,%eax,4), %edx /* Exchange the static variable. */
#ifndef CONFIG_NO_THREADS
	LOAD_CODE(%ecx)
	rwlock_endwrite co_static_lock(CODE_OR_ECX)
#endif
	ASSERT_NONZERO(%edx)
	decref  %edx
	jmp   .disp



target16_pop_extern:
	lodsw
	LOAD_CODE(%ecx)
	movl    co_module(CODE_OR_ECX), %ecx
	ASSERT_BELOW(%ax,mo_importc(%ecx))
	movl    mo_importv(%ecx), %ecx
	movl   (%ecx,%eax,4), %ecx
	lodsw
	jmp     2f
target_pop_extern:
	lodsb
	LOAD_CODE(%ecx)
	movl    co_module(CODE_OR_ECX), %ecx
	ASSERT_BELOW(%ax,mo_importc(%ecx))
	movl    mo_importv(%ecx), %ecx
	movl   (%ecx,%eax,4), %ecx
	lodsb
	jmp     2f

target16_pop_global:
	lodsw
	jmp     1f
target_pop_global:
	lodsb
1:	LOAD_CODE(%ecx)
	movl    co_module(CODE_OR_ECX), %ecx
2:	ASSERT_BELOW(%ax,mo_globalc(%ecx))
#ifndef CONFIG_NO_THREADS
	pushl   %eax
	ADJUST_CFA_OFFSET(4)
	rwlock_write mo_lock(%ecx), %edx
	popl    %eax
	ADJUST_CFA_OFFSET(-4)
	pushl   %ecx
	ADJUST_CFA_OFFSET(4)
#endif
	sp_pop  %edx /* Pop the value to-be written to globals */
	movl    mo_globalv(%ecx), %ecx
	xchgl  (%ecx,%eax,4), %edx
#ifndef CONFIG_NO_THREADS
	popl    %ecx
	ADJUST_CFA_OFFSET(-4)
	rwlock_endwrite mo_lock(%ecx)
#endif
	testl   %edx, %edx
	jz     .disp
	decref  %edx /* Drop a reference from the old global variable. */
	jmp    .disp



target16_pop_local:
	lodsw
	jmp     1f
target_pop_local:
	lodsb
1:	sp_pop  %ecx
	movl    FRAME_FRAME, %edx
	xchgl   (%edx,%eax,4), %ecx /* Store the popped value in locals */
	testl   %ecx, %ecx
	jz     .disp
	decref  %ecx /* decref the previous value. */
	jmp    .disp



/* Push various values onto the stack. */
target16_push_ref:
	lodsw
	jmp     1f
target_push_ref:
	lodsb
1:	movl    FRAME_FUNC, %ecx
	movl    fo_refv(%ecx,%eax,4), %eax
	sp_push %eax, 1
	jmp    .disp



target16_push_bnd_arg:
	lodsw
	jmp     1f
target_push_bnd_arg:
	lodsb
1:	subl    FRAME_ARGC, %eax
	jb      target_push_true /* if (imm_val < frame->cf_argc) return_true; */
	movl    FRAME_KW, %edx
	testl   %edx, %edx
	jz      target_push_false /* if (!frame->cf_kw) return_false; */
	xorl    %ecx, %ecx
	cmpl    $0, fk_kargv(%edx,%eax,4)
	setne   %cl
	leal    SYM(Dee_FalseTrue)(,%ecx,8), %eax
	sp_push %eax, 1
	jmp    .disp


target16_push_arg:
	lodsw
	jmp     1f
target_push_arg:
	lodsb
1:	cmpl    FRAME_ARGC, %eax
	jnb     1f
	/* Simple case: Direct argument/default reference. */
	movl    FRAME_ARGV, %edx
	movl   (%edx,%eax,4), %eax
	sp_push %eax, 1
	jmp    .disp
	/* Check for the argument within the kwds argument overlay */
1:	movl    FRAME_KW, %edx
	testl   %edx, %edx
	jz      2f
	subl    FRAME_ARGC, %eax
	movl    fk_kargv(%edx,%eax,4), %edx
	testl   %edx, %edx
	jz      1f
	sp_push %edx, 1
	jmp    .disp
1:	addl    FRAME_ARGC, %eax
	/* Check for the argument within the code's default argument vector */
2:	LOAD_CODE(%ecx)
	subw    co_argc_min(CODE_OR_ECX), %ax
	movl    co_defaultv(CODE_OR_ECX), %edx
	movl    (%edx,%eax,4), %edx
	testl   %edx, %edx
	jz      1f
	sp_push %edx, 1
	jmp    .disp
1:	addw    co_argc_min(CODE_OR_ECX), %ax
	/* Unbound argument */
	pushl   %eax
	ADJUST_CFA_OFFSET(4)
	pushl   FRAME_PC
	ADJUST_CFA_OFFSET(4)
	pushl   CODE_OR_LOC
	ADJUST_CFA_OFFSET(4)
	call    fSYM(err_unbound_arg,12)
	ADJUST_CFA_OFFSET(-12)
	jmp    .except


target_push_varargs:
	movl    FRAME_VARARGS, %eax
	testl   %eax, %eax
	jz      1f
	sp_push %eax, 1
	jmp    .disp
1:	LOAD_CODE(%ecx)
	ASSERT_BIT(w,$(CODE_FVARARGS),co_flags(CODE_OR_ECX))
	movzwl  co_argc_max(CODE_OR_ECX), %eax
	movl    FRAME_ARGC, %edx
	/* if (frame->cf_argc <= code->co_argc_max) */
	cmpl    %eax, %edx
	jnbe    1f
	/* { */
	leal    SYM(DeeTuple_Empty), %eax
	movl    %eax, FRAME_VARARGS
	incref_n %eax, 2
	sp_push %eax
	jmp    .disp
	/* } */
1:	/* Must construct the varargs tuple. */
	movl    FRAME_ARGV, %edx
	leal    (%edx,%eax,4), %edx /* EDX = frame->cf_argv + code->co_argc_max */
	movl    FRAME_ARGC, %ecx
	subl    %eax, %ecx          /* ECX = frame->cf_argc - code->co_argc_max */
	pushl   %edx
	ADJUST_CFA_OFFSET(4)
	pushl   %ecx
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeTuple_NewVector,8)
	ADJUST_CFA_OFFSET(-8)
	testl   %eax, %eax
	jz     .except
	/* Store the generated varargs object. */
	movl    %eax, FRAME_VARARGS
	sp_push %eax, 1
	jmp    .disp


target_push_varkwds:
	movl    FRAME_KW, %edx
	testl   %edx, %edx
	jz      70f
	movl    fk_varkwds(%edx), %eax
	testl   %eax, %eax
	jz      1f
	sp_push %eax, 1
	jmp    .disp
1:	/* construct_varkwds_mapping() { */
	movl    fk_kw(%edx), %eax
	/* if likely(DeeKwds_Check(kw)) */
	cmpl    $SYM(DeeKwds_Type), ob_type(%eax)
	jne     1f
	/* { */
	movl    FRAME_ARGC, %ecx
	movl    FRAME_ARGV, %edx
	leal    (%edx,%ecx,4), %ecx
	pushl   %ecx /* frame->cf_argv + frame->cf_argc */
	ADJUST_CFA_OFFSET(4)
	pushl   %eax          /* kw */
	ADJUST_CFA_OFFSET(4)
	pushl   FRAME_ARGC    /* positional_argc */
	ADJUST_CFA_OFFSET(4)
	pushl   CODE_OR_LOC   /* code */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(BlackListVarkwds_New,16)
	ADJUST_CFA_OFFSET(-16)
	jmp     60f
	/* } else { */
1:	pushl   %eax          /* kw */
	ADJUST_CFA_OFFSET(4)
	pushl   FRAME_ARGC    /* positional_argc */
	ADJUST_CFA_OFFSET(4)
	pushl   CODE_OR_LOC   /* code */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(BlackListMapping_New,12)
	ADJUST_CFA_OFFSET(-12)
	/* } */
60:	testl   %eax, %eax
	jz     .except
	/* Cache the newly generated varkwds object. */
	movl    FRAME_KW, %ecx
#ifdef CONFIG_NO_THREADS
	movl    fk_varkwds(%ecx), %edx
	testl   %edx, %edx
	jnz     61f
	movl    %eax, fk_varkwds(%ecx) /* Inherit reference */
	sp_push %eax, 1
	jmp    .disp
61:	sp_push %edx, 1
	decref  %eax
	jmp    .disp
#else /* CONFIG_NO_THREADS */
	movl    %eax, %edx
	xorl    %eax, %eax
	/* EAX = ATOMIC_CMPXCH_VAL(frame->cf_kw->fk_varkwds,NULL,EDX); */
	cmpxchgl %edx, fk_varkwds(%ecx)
	/* if (EAX != NULL) */
	jnz     1f
	/* { */
	sp_push %edx, 1
	jmp    .disp
	/* } */
1:	/* Another thread already loaded `EAX'. */
	sp_push %eax, 1
	movl    ob_type(%edx), %ecx
	cmpl    $SYM(DeeKwdsMapping_Type), %ecx
	jne     1f
	/* Cleanup: DeeKwdsMapping_CheckExact() */
	pushl   %edx
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeKwdsMapping_Decref,4)
	ADJUST_CFA_OFFSET(-4)
	jmp     .disp
1:	cmpl    $SYM(BlackListVarkwds_Type), %ecx
	jne     1f
	/* Cleanup: BlackListVarkwds_CheckExact() */
	pushl   %edx
	ADJUST_CFA_OFFSET(4)
	call    fSYM(BlackListVarkwds_Decref,4)
	ADJUST_CFA_OFFSET(-4)
	jmp     .disp
	/* Cleanup: Dee_Decref() */
1:	decref  %edx
	jmp    .disp
#endif /* !CONFIG_NO_THREADS */
70:	leal    SYM(DeeMapping_EmptyInstance), %edx
	sp_push %edx, 1
	jmp    .disp



#if defined(__OPTIMIZE_SIZE__) || defined(CONFIG_NO_THREADS)
#define HAVE_TARGET_PUSH_CONST 1
target16_push_const:
#endif
target16_push_static:
	lodsw
	jmp    1f
#ifdef HAVE_TARGET_PUSH_CONST
target_push_const:
#endif
target_push_static:
	lodsb
1:	LOAD_CODE(%ecx)
	movl   co_staticv(CODE_OR_ECX), %edx
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
#ifndef CONFIG_NO_THREADS
	pushl  %eax
	ADJUST_CFA_OFFSET(4)
	rwlock_read co_static_lock(CODE_OR_ECX), %edx
	popl   %eax
	ADJUST_CFA_OFFSET(-4)
#endif
	movl   (%edx,%eax,4), %edx
	sp_push %edx, 1 /* Push the static variable. */
#ifndef CONFIG_NO_THREADS
	rwlock_endread co_static_lock(CODE_OR_ECX)
#endif
	jmp   .disp



#ifndef HAVE_TARGET_PUSH_CONST
target16_push_const:
	lodsw
	jmp    1f
target_push_const:
	lodsb
1:	LOAD_CODE(%ecx)
	movl   co_staticv(CODE_OR_ECX), %edx
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
	movl   (%edx,%eax,4), %edx
	sp_push %edx, 1 /* Push the static variable. */
	jmp   .disp
#endif



target16_push_extern:
	lodsw
	LOAD_CODE(%ecx)
	movl   co_module(CODE_OR_ECX), %ecx
	ASSERT_BELOW(%ax,mo_importc(%ecx))
	movl   mo_importv(%ecx), %ecx
	movl  (%ecx,%eax,4), %ecx
	lodsw
	jmp    2f
target_push_extern:
	lodsb
	LOAD_CODE(%ecx)
	movl   co_module(CODE_OR_ECX), %ecx
	ASSERT_BELOW(%ax,mo_importc(%ecx))
	movl   mo_importv(%ecx), %ecx
	movl  (%ecx,%eax,4), %ecx
	lodsb
	jmp    2f
target16_push_global:
	lodsw
	jmp    1f
target_push_global:
	lodsb
1:	LOAD_CODE(%ecx)
	movl   co_module(CODE_OR_ECX), %ecx
2:	ASSERT_BELOW(%ax,mo_globalc(%ecx))
#ifndef CONFIG_NO_THREADS
	pushl  %eax
	ADJUST_CFA_OFFSET(4)
	rwlock_write mo_lock(%ecx), %edx
	popl   %eax
	ADJUST_CFA_OFFSET(-4)
#endif
	movl   mo_globalv(%ecx), %edx
	movl  (%edx,%eax,4), %edx
	testl  %edx, %edx
	jz     99f /* Check for unbound globals. */
	sp_push %edx, 1 /* Push the global variable. */
#ifndef CONFIG_NO_THREADS
	rwlock_endwrite mo_lock(%ecx)
#endif
	jmp   .disp
99:
#ifndef CONFIG_NO_THREADS
	rwlock_endwrite mo_lock(%ecx)
#endif
	pushl  %eax /* GID */
	ADJUST_CFA_OFFSET(4)
	pushl  %ecx /* MODULE */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(err_unbound_global,8)
	ADJUST_CFA_OFFSET(-8)
	jmp   .except



target16_push_local:
	lodsw
	jmp    1f
target_push_local:
	lodsb
1:	movl   FRAME_FRAME, %ecx
#ifndef NDEBUG
	LOAD_CODE(%edx)
	ASSERT_BELOW(%ax,co_localc(CODE_OR_EDX))
#endif
	movl  (%ecx,%eax,4), %edx
	testl  %edx, %edx
	jz     99f /* Check for unbound locals. */
	sp_push %edx, 1
	jmp   .disp
99:	pushl  %eax        /* LID */
	ADJUST_CFA_OFFSET(4)
	pushl  FRAME_PC /* PC */
	ADJUST_CFA_OFFSET(4)
	pushl  CODE_OR_LOC /* CODE */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(err_unbound_local,12)
	ADJUST_CFA_OFFSET(-12)
	jmp   .except



target16_pack_tuple:
	lodsw
	jmp    1f
target_pack_tuple:
	lodsb
1:	movl   SP, %edx
	leal   (,%eax,4), %ecx
	subl   %ecx, %edx
	pushl  %edx /* New SP */
	ADJUST_CFA_OFFSET(4)
	pushl  %edx /* SP-n_elem */
	ADJUST_CFA_OFFSET(4)
	pushl  %eax /* n_elem */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeTuple_NewVectorSymbolic,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jz    .except_x4 /* Check for errors. */
	popl   SP /* Load the new SP */
	ADJUST_CFA_OFFSET(-4)
	sp_push %eax /* Push the new tuple. */
	jmp   .disp



target16_pack_list:
	lodsw
	jmp    1f
target_pack_list:
	lodsb
1:	movl   SP, %edx
	leal   (,%eax,4), %ecx
	subl   %ecx, %edx
	pushl  %edx /* New SP */
	ADJUST_CFA_OFFSET(4)
	pushl  %edx /* SP-n_elem */
	ADJUST_CFA_OFFSET(4)
	pushl  %eax /* n_elem */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeList_NewVectorInherited,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jz    .except_x4 /* Check for errors. */
	popl   SP /* Load the new SP */
	ADJUST_CFA_OFFSET(-4)
	sp_push %eax /* Push the new list. */
	jmp   .disp



target16_unpack:
	lodsw
	jmp    1f
target_unpack:
	lodsb
1:	sp_pop %edx
.unpack_sequence_edx_count_eax:
	pushl  %edx            /* backup... (sequence) */
	ADJUST_CFA_OFFSET(4)
	pushl  %eax            /* backup... (objc) */
	ADJUST_CFA_OFFSET(4)
	pushl  SP              /* DeeObject_Unpack:objv */
	ADJUST_CFA_OFFSET(4)
	pushl  %eax            /* DeeObject_Unpack:objc */
	ADJUST_CFA_OFFSET(4)
	pushl  %edx            /* DeeObject_Unpack:self */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_Unpack,12)
	ADJUST_CFA_OFFSET(-12)
	testl  %eax, %eax
	jnz    99f
	popl   %eax            /* objc */
	ADJUST_CFA_OFFSET(-4)
	popl   %edx            /* sequence */
	ADJUST_CFA_OFFSET(-4)
	leal   (SP,%eax,4), SP /* SP += objc; */
	decref %edx
	jmp   .disp
	ADJUST_CFA_OFFSET(8)
99:	addl   $4, %esp
	ADJUST_CFA_OFFSET(-4)
	jmp   .except_decref_pop
	ADJUST_CFA_OFFSET(-4)


/* unpack the prefixed object. */
prefix_target16_unpack:
	call   get_prefix_object
	xorl   %eax, %eax
	lodsw
	jmp    .unpack_sequence_edx_count_eax
prefix_target_unpack:
	call   get_prefix_object
	xorl   %eax, %eax
	lodsb
	jmp   .unpack_sequence_edx_count_eax


/* Unpack varargs. */
target_varargs_unpack:
	LOAD_CODE(%edx)
#ifndef NDEBUG
	ASSERT_BIT(w,$(CODE_FVARARGS),co_flags(CODE_OR_EDX))
#endif
	lodsb
	movl   FRAME_ARGC, %ecx
	subw   co_argc_max(CODE_OR_EDX), %cx
	jo     70f
71:	cmpl   %eax, %ecx
	jne    80f /* Check if unpack size matches actual size. */
	/* Now to do the actual unpack! */
	testl  %ecx, %ecx
	jz    .disp
	pushl  %esi
	ADJUST_CFA_OFFSET(4)
	movl   FRAME_ARGV, %esi
	movzwl co_argc_max(CODE_OR_EDX), %eax
	leal   (%esi,%eax,4), %esi /* ESI = frame->cf_argv + code->co_argc_max; */
#ifndef SP_IS_DI
	pushl  %edi
	ADJUST_CFA_OFFSET(4)
	movl   SP, %edi
#endif
1:	lodsl       /* EAX = *ESI++; */
	incref %eax
	stosl       /* *SP++ = EAX; */
	loop   1b
#ifndef SP_IS_DI
	movl   %edi, SP
	popl   %edi
	ADJUST_CFA_OFFSET(-4)
#endif
	popl   %esi
	ADJUST_CFA_OFFSET(-4)
	jmp   .disp
70:	xorl   %ecx, %ecx
	jmp    71b
80:	pushl  %edx /* va_size */
	ADJUST_CFA_OFFSET(4)
	pushl  %eax /* need_size */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(err_invalid_va_unpack_size,8)
	ADJUST_CFA_OFFSET(-8)
	jmp   .except


target_push_varkwds_ne:
#ifndef NDEBUG
	LOAD_CODE(%eax)
	ASSERT_BIT(w,$(CODE_FVARKWDS),co_flags(CODE_OR_EAX))
#endif
	movl    FRAME_KW, %edx
	testl   %edx, %edx
	jz      target_push_false
	movl    fk_varkwds(%edx), %eax
	testl   %eax, %eax
	jz      1f
.Ltarget_push_varkwds_ne_impl:
	pushl   %eax /* DeeObject_Bool:self */
	ADJUST_CFA_OFFSET(4)
.Ltarget_push_varkwds_ne_impl_onstack:
	call    fSYM(DeeObject_Bool,4)
	ADJUST_CFA_OFFSET(-4)
	xorl    %ecx, %ecx
	cmpl    $0, %eax
	jl     .except
	setg    %cl
	leal    SYM(Dee_FalseTrue)(,%ecx,8), %eax
	sp_push %eax, 1
	jmp    .disp
1:	/* construct_varkwds_mapping() { */
	movl    fk_kw(%edx), %eax
	/* if likely(DeeKwds_Check(kw)) */
	cmpl    $SYM(DeeKwds_Type), ob_type(%eax)
	jne     1f
	/* { */
	movl    FRAME_ARGC, %ecx
	movl    FRAME_ARGV, %edx
	leal    (%edx,%ecx,4), %ecx
	pushl   %ecx /* frame->cf_argv + frame->cf_argc */
	ADJUST_CFA_OFFSET(4)
	pushl   %eax          /* kw */
	ADJUST_CFA_OFFSET(4)
	pushl   FRAME_ARGC    /* positional_argc */
	ADJUST_CFA_OFFSET(4)
	pushl   CODE_OR_LOC   /* code */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(BlackListVarkwds_New,16)
	ADJUST_CFA_OFFSET(-16)
	jmp     60f
	/* } else { */
1:	pushl   %eax          /* kw */
	ADJUST_CFA_OFFSET(4)
	pushl   FRAME_ARGC    /* positional_argc */
	ADJUST_CFA_OFFSET(4)
	pushl   CODE_OR_LOC   /* code */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(BlackListMapping_New,12)
	ADJUST_CFA_OFFSET(-12)
	/* } */
60:	testl   %eax, %eax
	jz     .except
	/* Cache the newly generated varkwds object. */
	movl    FRAME_KW, %ecx
#ifdef CONFIG_NO_THREADS
	movl    fk_varkwds(%ecx), %edx
	testl   %edx, %edx
	jnz     61f
	movl    %eax, fk_varkwds(%ecx) /* Inherit reference */
	jmp    .Ltarget_push_varkwds_ne_impl
61:	decref  %eax, 0, 0, 1 /* Preserve EDX */
	pushl   %edx
	ADJUST_CFA_OFFSET(4)
	jmp    .Ltarget_push_varkwds_ne_impl_onstack
	ADJUST_CFA_OFFSET(-4)
#else /* CONFIG_NO_THREADS */
	movl    %eax, %edx
	xorl    %eax, %eax
	/* EAX = ATOMIC_CMPXCH_VAL(frame->cf_kw->fk_varkwds,NULL,EDX); */
	cmpxchgl %edx, fk_varkwds(%ecx)
	/* if (EAX != NULL) */
	jnz     1f
	/* { */
	pushl   %edx
	ADJUST_CFA_OFFSET(4)
	jmp    .Ltarget_push_varkwds_ne_impl_onstack
	ADJUST_CFA_OFFSET(-4)
	/* } */
1:	/* Another thread already loaded `EAX'. */
	movl    ob_type(%edx), %ecx
	cmpl    $SYM(DeeKwdsMapping_Type), %ecx
	jne     1f
	/* Cleanup: DeeKwdsMapping_CheckExact() */
	pushl   %eax
	ADJUST_CFA_OFFSET(4)
	pushl   %edx
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeKwdsMapping_Decref,4)
	ADJUST_CFA_OFFSET(-4)
	jmp    .Ltarget_push_varkwds_ne_impl_onstack
	ADJUST_CFA_OFFSET(-4)
1:	cmpl    $SYM(BlackListVarkwds_Type), %ecx
	jne     1f
	/* Cleanup: BlackListVarkwds_CheckExact() */
	pushl   %eax
	ADJUST_CFA_OFFSET(4)
	pushl   %edx
	ADJUST_CFA_OFFSET(4)
	call    fSYM(BlackListVarkwds_Decref,4)
	ADJUST_CFA_OFFSET(-4)
	jmp    .Ltarget_push_varkwds_ne_impl_onstack
	ADJUST_CFA_OFFSET(-4)
	/* Cleanup: Dee_Decref() */
1:	pushl   %eax
	ADJUST_CFA_OFFSET(4)
	decref  %edx
	jmp    .Ltarget_push_varkwds_ne_impl_onstack
	ADJUST_CFA_OFFSET(-4)
#endif /* !CONFIG_NO_THREADS */


target_varargs_getsize:
	LOAD_CODE(%ecx)
	ASSERT_BIT(w,$(CODE_FVARARGS),co_flags(CODE_OR_ECX))
	movl   FRAME_ARGC, %eax
	ASSERT_ABOVE_EQUAL(%ax,co_argc_min(CODE_OR_ECX))
	movzwl co_argc_max(CODE_OR_ECX), %ecx
	subl   %ecx, %eax
	jbe    1f /* if (frame->cf_argc <= code->co_argc_max)
	           *     goto 1f;
	           * EAX = frame->cf_argc - code->co_argc_max; */
	pushl  %eax
	ADJUST_CFA_OFFSET(4)
	/* EAX = DeeInt_NewSize((size_t)(frame->cf_argc-code->co_argc_max)) */
	call   fSYM(DeeInt_NewU32,4)
	ADJUST_CFA_OFFSET(-4)
	testl  %eax, %eax
	jz    .except
2:	sp_push %eax
	jmp   .disp
1:	leal   SYM(DeeInt_Zero), %eax
	incref %eax
	jmp    2b

target_varargs_cmp_eq_sz:
	LOAD_CODE(%ecx)
	ASSERT_BIT(w,$(CODE_FVARARGS),co_flags(CODE_OR_ECX))
	movl   FRAME_ARGC, %edx
	ASSERT_ABOVE_EQUAL(%dx,co_argc_min(CODE_OR_ECX))
	movzwl co_argc_max(%ecx), %ecx
	subl   %ecx, %edx
	jnbe   1f /* if (!(frame->cf_argc <= code->co_argc_max)) goto 1f; */
	xorl   %edx, %edx
1:	/* EDX = va_size */
	lodsb
	cmpl   %eax, %edx
	sete   %al
	leal   SYM(Dee_FalseTrue)(,%eax,8), %eax
	sp_push %eax, 1 /* PUSHREF(DeeBool_For(va_size == READ_imm8())) */
	jmp   .disp

target_varargs_cmp_gr_sz:
	LOAD_CODE(%ecx)
	ASSERT_BIT(w,$(CODE_FVARARGS),co_flags(CODE_OR_ECX))
	movl   FRAME_ARGC, %edx
	ASSERT_ABOVE_EQUAL(%dx,co_argc_min(CODE_OR_ECX))
	movzwl co_argc_max(%ecx), %ecx
	subl   %ecx, %edx
	jnbe   1f /* if (!(frame->cf_argc <= code->co_argc_max)) goto 1f; */
	xorl   %edx, %edx
1:	/* EDX = va_size */
	lodsb
	cmpl   %eax, %edx
	setg   %al
	leal   SYM(Dee_FalseTrue)(,%eax,8), %eax
	sp_push %eax, 1 /* PUSHREF(DeeBool_For(va_size == READ_imm8())) */
	jmp   .disp

target_varargs_getitem:
	subl   $4, %esp /* Target memory for `DeeObject_AsSize' */
	ADJUST_CFA_OFFSET(4)
	pushl  %esp     /* DeeObject_AsUInt32:result */
	ADJUST_CFA_OFFSET(4)
	pushl  TOP      /* DeeObject_AsUInt32:self */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_AsUInt32,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jnz   .except_x4
	sp_popref %eax
	popl   %eax
	ADJUST_CFA_OFFSET(-4)
	jmp   .varargs_getitem_eax

target_varargs_getitem_i:
	lodsb
.varargs_getitem_eax:
	LOAD_CODE(%ecx)
	movzwl co_argc_max(CODE_OR_ECX), %ecx
	addl   %ecx, %eax /* EAX = index + code->co_argc_max */
	jo     80f        /* if (DID_OVERFLOW) goto 80f; */
	cmpl   FRAME_ARGC, %eax
	jae    80f        /* if (EAX >= frame->cf_argc) goto 80f; */
	movl   FRAME_ARGV, %edx
	movl   (%edx,%eax,4), %eax /* EAX = frame->cf_argv[index]; */
	sp_push %eax, 1
	jmp   .disp
80:	subl   %ecx, %eax       /* EAX = index; */
	movl   FRAME_ARGC, %ecx
	LOAD_CODE(%edx)
	movzwl co_argc_max(CODE_OR_EDX), %edx
	subl   %edx, %ecx       /* ECX = frame->cf_argc - code->co_argc_max */
	pushl  %ecx             /* err_va_index_out_of_bounds:size */
	ADJUST_CFA_OFFSET(4)
	pushl  %eax             /* err_va_index_out_of_bounds:index */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(err_va_index_out_of_bounds,8)
	ADJUST_CFA_OFFSET(-8)
	jmp   .except




/* Misc. operations. */
target_push_none:
	incref_addr SYM(DeeNone_Singleton)
	sp_push $(SYM(DeeNone_Singleton))
	jmp    .disp
target_push_true:
	incref_addr SYM(Dee_FalseTrue)+8
	sp_push $(SYM(Dee_FalseTrue)+8)
	jmp    .disp
target_push_false:
	incref_addr SYM(Dee_FalseTrue)+0
	sp_push $(SYM(Dee_FalseTrue)+0)
	jmp    .disp



target_typeof:
	movl   TOP, %eax
	movl   ob_type(%eax), %eax
	incref %eax /* Incref the object's type. */
	xchgl  TOP, %eax /* Exchange the object with its type. */
	decref %eax /* decref the object. */
	jmp   .disp
target_classof:
	pushl  TOP
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_Class,4)
	ADJUST_CFA_OFFSET(-4)
	incref %eax /* Incref the object's class. */
	xchgl  TOP, %eax /* Exchange the object with its class. */
	decref %eax /* decref the object. */
	jmp   .disp



target_superof:
	pushl  TOP
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeSuper_Of,4)
	ADJUST_CFA_OFFSET(-4)
	testl  %eax, %eax
	jz    .except
	xchgl  TOP, %eax
	decref %eax
	jmp   .disp



target_instanceof:
	movl   SECOND, %ecx
	movl   FIRST,  %edx
	cmpl   $(SYM(DeeNone_Singleton)), %edx
	/* Special case: The deemon specs allow `none' to be
	 *               written as the second argument to `is' */
	je     1f
	pushl  %edx /* FIRST */
	ADJUST_CFA_OFFSET(4)
	movl   ob_type(%ecx), %eax
	cmpl   $(SYM(DeeSuper_Type)), %eax
	jne    3f
	/* Check against the effective class of the
	 * super-wrapper, not the super-type itself. */
	movl   s_type(%ecx), %eax
3:	pushl  %eax
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeType_IsInherited,8)
	ADJUST_CFA_OFFSET(-8)
	movzbl %al, %eax
2:	ASSERT_BELOW_EQUAL(%eax,$1)
	leal   SYM(Dee_FalseTrue)(,%eax,8), %eax
	incref %eax
	xchgl  SECOND, %eax /* Exchange the lower-order operand with the boolean object. */
	decref %eax
	sp_popref %eax
	jmp   .disp
1:	cmpl   $(SYM(DeeNone_Singleton)), %ecx /* `x is none' - Return true if `x' is the none-singleton. */
	sete   %al
	jmp    2b



target_bool:
	pushl  TOP
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_Bool,4)
	ADJUST_CFA_OFFSET(-4)
	xorl   %ecx, %ecx /* Clear ECX */
	cmpl   $0, %eax
	jb    .except /* Check for errors. */
	setne  %cl /* set ECX to 0/1 indicative of the boolean */
	leal   SYM(Dee_FalseTrue)(,%ecx,8), %eax
	incref %eax
	xchgl  TOP, %eax /* Exchange the stack-object with its boolean value. */
	decref %eax
	jmp   .disp



target_not:
	pushl  TOP
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_Bool,4)
	ADJUST_CFA_OFFSET(-4)
	xorl   %ecx, %ecx /* Clear ECX */
	cmpl   $0, %eax
	jb    .except /* Check for errors. */
	sete   %cl /* set ECX to 0/1 indicative of the reverse boolean */
	leal   SYM(Dee_FalseTrue)(,%ecx,8), %eax
	incref %eax
	xchgl  TOP, %eax /* Exchange the stack-object with its boolean value. */
	decref %eax
	jmp   .disp


target_iternext:
	pushl  TOP
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_IterNext,4)
	ADJUST_CFA_OFFSET(-4)
	testl  %eax, %eax
	jz    .except
	cmpl   $(ITER_DONE), %eax
	je     1f
	xchgl  TOP, %eax /* Load the iter-result onto the stack. */
	decref %eax /* Drop a reference from the iterator. */
	jmp   .disp
1:	/* Throw the stop-iteration signal. */
	pushl  SYM(DeeError_StopIteration_instance)
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeError_Throw,4)
	ADJUST_CFA_OFFSET(-4)
	jmp   .except



target_assign:
	pushl  FIRST
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_Assign,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jnz   .except
	sp_popref %eax
	sp_popref %eax
	jmp   .disp



target_move_assign:
	pushl  FIRST
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_MoveAssign,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jnz   .except
	sp_popref %eax
	sp_popref %eax
	jmp   .disp



/* Push special objects. */
target_push_this_function:
	movl   FRAME_FUNC, %eax
	sp_push %eax, 1 /* Simply push the current function. */
	jmp   .disp
target_push_except:
	movl   L_THIS_THREAD, %eax
	movl   t_except(%eax), %eax
	testl  %eax, %eax
	jz    .except_no_active_exception /* Check if an exception has been set. */
	movl   ef_error(%eax), %eax
	ASSERT_NONZERO(%eax)
	sp_push %eax, 1 /* Push a new reference to the current exception. */
	jmp   .disp

target_push_this:
#ifndef NDEBUG
	LOAD_CODE(%ecx)
	ASSERT_BIT(w,$(CODE_FTHISCALL),co_flags(CODE_OR_ECX))
#endif
	movl   FRAME_THIS, %eax
	sp_push %eax, 1 /* Push a new reference to the this-argument. */
	jmp   .disp



target16_push_module:
	lodsw
	jmp    1f
target_push_module:
	lodsb
1:	LOAD_CODE(%ecx)
	movl   co_module(CODE_OR_ECX), %ecx
	ASSERT_BELOW(%ax,mo_importc(%ecx))
	movl   mo_importv(%ecx), %edx
	movl   (%edx,%eax,4), %edx
	ASSERT_NONZERO(%edx)
	sp_push %edx, 1 /* Push a new reference to the imported module. */
	jmp   .disp



target_concat:
	pushl  FIRST
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND /* Inherited upon success */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_ConcatInherited,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jz    .except
	movl   %eax, SECOND /* Save the concat in the second slot. */
	sp_popref %eax /* Pop the sequence that was appended */
	jmp   .disp
target_extend:
	lodsb  /* EAX = n_args */
	leal   (,%eax,4), %ecx /* ECX = EAX*4 */
	movl   SP, %edx
	subl   %ecx, %edx /* EDX = new_sp */
	pushl  %edx     /* backup(new_sp) */
	ADJUST_CFA_OFFSET(4)
	pushl  %edx     /* argv */
	ADJUST_CFA_OFFSET(4)
	pushl  %eax     /* argc */
	ADJUST_CFA_OFFSET(4)
	pushl  -4(%edx) /* sequence */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_ExtendInherited,12) /* Extend the sequence. */
	ADJUST_CFA_OFFSET(-12)
	testl  %eax, %eax
	jz    .except_x4 /* Check for errors. */
	popl   SP        /* SP = new_sp */
	ADJUST_CFA_OFFSET(-4)
	movl   %eax, TOP /* Save the concat-sequence. */
	jmp   .disp



.macro define_unary_op target, function
\target:
	pushl  TOP
	ADJUST_CFA_OFFSET(4)
	call   \function
	ADJUST_CFA_OFFSET(-4)
	testl  %eax, %eax
	jz    .except /* Check for errors. */
	xchgl  TOP, %eax /* Store EAX in TOP. */
	decref %eax /* Drop a reference from the old object in TOP. */
	jmp   .disp
.endm
.macro define_binary_op, target, function
\target:
	pushl  FIRST
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND
	ADJUST_CFA_OFFSET(4)
	call   \function
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jz    .except /* Check for errors. */
	sp_popref %ecx, 1, 0, 0 /* Preserve EAX */
	xchgl  TOP, %eax /* Store EAX in TOP. */
	decref %eax /* Drop a reference from the old object in TOP. */
	jmp   .disp
.endm

/* Object control operators. */
define_unary_op target_cast_tuple, fSYM(DeeTuple_FromSequence,4)
define_unary_op target_cast_list, fSYM(DeeList_FromSequence,4)
define_unary_op target_cast_hashset, fSYM(DeeHashSet_FromSequence,4)
define_unary_op target_cast_dict, fSYM(DeeDict_FromSequence,4)
define_unary_op target_str, fSYM(DeeObject_Str,4)
define_unary_op target_repr, fSYM(DeeObject_Repr,4)
define_unary_op target_copy, fSYM(DeeObject_Copy,4)
define_unary_op target_deepcopy, fSYM(DeeObject_DeepCopy,4)
define_binary_op target_cmp_eq, fSYM(DeeObject_CompareEqObject,8)
define_binary_op target_cmp_ne, fSYM(DeeObject_CompareNeObject,8)
define_binary_op target_cmp_lo, fSYM(DeeObject_CompareLoObject,8)
define_binary_op target_cmp_le, fSYM(DeeObject_CompareLeObject,8)
define_binary_op target_cmp_gr, fSYM(DeeObject_CompareGrObject,8)
define_binary_op target_cmp_ge, fSYM(DeeObject_CompareGeObject,8)

/* Same object / different object comparisons. */
target_cmp_so:
	sp_pop %edx
	decref %edx
	movl   TOP, %ecx
	decref %ecx
	cmpl   %ecx, %edx
	sete   %al
	leal   SYM(Dee_FalseTrue)(,%eax,8), %eax
	incref %eax
	movl   %eax, TOP
	jmp   .disp
target_cmp_do:
	sp_pop %edx
	decref %edx
	movl   TOP, %ecx
	decref %ecx
	cmpl   %ecx, %edx
	setne  %al
	leal   SYM(Dee_FalseTrue)(,%eax,8), %eax
	incref %eax
	movl   %eax, TOP
	jmp   .disp



/* Class instructions */
target16_class_c:
	lodsw
	jmp    1f
target_class_c:
	lodsb
1:	LOAD_CODE(%ecx)
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
	movl   co_staticv(CODE_OR_ECX), %ecx
	pushl  (%ecx,%eax,4) /* DeeClass_New:descriptor */
	ADJUST_CFA_OFFSET(4)
	pushl  TOP           /* DeeClass_New:base */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeClass_New,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jz    .except
	xchgl  TOP, %eax
	decref %eax
	jmp   .disp

target16_class_ec:
	lodsw
	movl   %eax, %edx
	lodsw
	movl   %eax, %ecx
	lodsw
	jmp    1f
target_class_ec:
	lodsb
	movl   %eax, %edx
	lodsb
	movl   %eax, %ecx
	lodsb
1:	subl   $4, %esp       /* Reserve memory for BASE_BACKUP */
	ADJUST_CFA_OFFSET(4)
	pushl  %ecx           /* backup GID / reserve `DeeClass_New:descriptor' */
	ADJUST_CFA_OFFSET(4)
	LOAD_CODE(%ecx)
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
	movl   co_staticv(CODE_OR_ECX), %ecx
	movl   (%ecx,%eax,4), %ecx            /* ECX = descriptor */
	xchgl  %ecx, 0(%esp)                  /* DeeClass_New:descriptor; ECX = gid */
	/* Read the class base from `extern %EDX:%ECX' */
	LOAD_CODE(%eax)
	movl   co_module(CODE_OR_EAX), %eax   /* EAX = THIS_MODULE */
	ASSERT_BELOW(%dx,mo_importc(%eax))
	movl   mo_importv(%eax), %eax
	movl   (%eax,%edx,4), %edx            /* EDX = IMPORTS[MID] */
	jmp   .create_class_module_edx
	ADJUST_CFA_OFFSET(-8)

target16_class_gc:
	lodsw
	movl   %eax, %ecx
	lodsw
	jmp    1f
target_class_gc:
	lodsb
	movl   %eax, %ecx
	lodsb
1:	subl   $4, %esp       /* Reserve memory for BASE_BACKUP */
	ADJUST_CFA_OFFSET(4)
	LOAD_CODE(%edx)
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_EDX))
	movl   co_staticv(CODE_OR_EDX), %edx
	pushl  (%edx,%eax,4) /* DeeClass_New:descriptor */
	ADJUST_CFA_OFFSET(4)
	LOAD_CODE(%edx)
	movl   co_module(CODE_OR_EDX), %edx
.create_class_module_edx:
	/* Read the class base from `global %ECX'; module = EDX
	 *  >> EAX   -- UNDEFINED
	 *  >> ECX   -- GID (8/16-bit)
	 *  >> EDX   -- `DeeModuleObject *'
	 *  >> STACK -- +8 { 0..3 = `DeeClassDescriptorObject *', 4..7 = RESERVED }
	 */
	ASSERT_BELOW(%cx,mo_globalc(%edx))
#ifndef CONFIG_NO_THREADS
	pushl  %ecx
	ADJUST_CFA_OFFSET(4)
	pushl  %edx
	ADJUST_CFA_OFFSET(4)
	rwlock_read mo_lock(%edx), %ecx
	popl   %edx
	ADJUST_CFA_OFFSET(-4)
	popl   %ecx
	ADJUST_CFA_OFFSET(-4)
#endif
	movl   mo_globalv(%edx), %eax
	movl   (%eax,%ecx,4), %eax
	testl  %eax, %eax /* Check if the global variable is unbound. */
	jz     99f
	incref %eax
#ifndef CONFIG_NO_THREADS
	rwlock_endread mo_lock(%edx)
#endif
	movl   %eax, 4(%esp)  /* BACKUP(base) = base */
	pushl  %eax           /* DeeClass_New:base */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeClass_New,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jz    .except_decref_pop
	sp_push %eax
	popl   %eax  /* BACKUP(base) */
	ADJUST_CFA_OFFSET(-4)
	decref %eax
	jmp   .disp
99:	rwlock_endread mo_lock(%edx)
	pushl  %eax /* GID */
	ADJUST_CFA_OFFSET(4)
	pushl  %edx /* MODULE */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(err_unbound_global,8)
	ADJUST_CFA_OFFSET(-8)
	jmp   .except

target_class:
	movl   TOP, %eax
	cmpl   $SYM(DeeClassDescriptor_Type), ob_type(%eax)
	jne    99f
	pushl  %eax   /* DeeClass_New:descriptor */
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND /* DeeClass_New:base */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeClass_New,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jz    .except
	xchgl  SECOND, %eax  /* Save the result onto the stack. */
	decref %eax          /* Pop the base */
	sp_popref %eax       /* Pop the descriptor */
	jmp   .disp

99:	pushl  $SYM(DeeClassDescriptor_Type)
	ADJUST_CFA_OFFSET(4)
	pushl  %eax
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_TypeAssertFailed,8)
	ADJUST_CFA_OFFSET(-8)
	jmp   .except


target16_defcmember:
	lodsw
	jmp    1f
target_defcmember:
	lodsb
1:	pushl  FIRST
	ADJUST_CFA_OFFSET(4)
	pushl  %eax
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeClass_SetMember,12)
	ADJUST_CFA_OFFSET(-12)
	sp_popref %eax
	jmp    .disp

target16_getcmember_r:
	lodsw
	movl   %eax, %edx
	lodsw
	jmp    1f
target_getcmember_r:
	lodsb
	movl   %eax, %edx
	lodsb
1:	pushl  %eax                 /* DeeClass_GetMember:addr */
	ADJUST_CFA_OFFSET(4)
	movl   FRAME_FUNC, %ecx
	pushl  fo_refv(%ecx,%edx,4) /* DeeClass_GetMember:self */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeClass_GetMember,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jz     .except
	sp_push %eax      /* PUSH(member_value) */
	jmp    .disp

target16_getcmember:
	lodsw
	pushl  %eax                 /* DeeClass_GetMemberSafe:addr */
	ADJUST_CFA_OFFSET(4)
	pushl  TOP                  /* DeeClass_GetMemberSafe:self */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeClass_GetMemberSafe,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jz     .except
	xchgl  TOP, %eax
	decref %eax
	jmp    .disp

target16_callcmember_this_r:
	lodsw
	movl   %eax, %edx
	lodsw
	jmp    1f
target_callcmember_this_r:
	lodsb
	movl   %eax, %edx
	lodsb
1:	pushl  %eax                 /* DeeClass_GetMember:addr */
	ADJUST_CFA_OFFSET(4)
	movl   FRAME_FUNC, %ecx
	pushl  fo_refv(%ecx,%edx,4) /* DeeClass_GetMember:self */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeClass_GetMember,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jz    .except
	pushl  %eax           /* BACKUP(callback) */
	ADJUST_CFA_OFFSET(4)
	subl   $12, %esp      /* Reserve memory for a call to `DeeObject_ThisCall()' + backup for `argc' */
	ADJUST_CFA_OFFSET(12)
	pushl  FRAME_THIS     /* DeeObject_ThisCall:this_arg */
	ADJUST_CFA_OFFSET(4)
	pushl  %eax           /* DeeObject_ThisCall:self */
	ADJUST_CFA_OFFSET(4)
	xorl   %eax, %eax
	lodsb                 /* EAX = argc */
	movl   %eax, 8(%esp)  /* DeeObject_ThisCall:argc */
	movl   %eax, 16(%esp) /* argc_backup = argc; */
	leal   (,%eax,4), %ecx
	movl   SP, %edx
	subl   %ecx, %edx     /* EDX = SP - argc */
	movl   %edx, 12(%esp) /* DeeObject_ThisCall:argv */
	call   fSYM(DeeObject_ThisCall,16)
	ADJUST_CFA_OFFSET(-16)
	popl   %ecx           /* ECX = argc_backup */
	ADJUST_CFA_OFFSET(-4)
	testl  %eax, %eax
	jz    .except_decref_pop
	/* Pop all function arguments */
	testl  %ecx, %ecx
	jz     1f
2:	sp_popref %edx, 1, 1, 0
	loop   2b
1:	sp_push %eax          /* Save the result of the callback onto the stack. */
	popl   %eax           /* callback */
	ADJUST_CFA_OFFSET(-4)
	decref %eax
	jmp   .disp



target16_getmember_this:
	/* `push getmember this, pop, $<imm16>' */
	lodsw
	jmp     1f
target_getmember_this:
	/* `push getmember this, pop, $<imm8>' */
	lodsb
1:	pushl   %eax               /* DeeInstance_GetMember:index */
	ADJUST_CFA_OFFSET(4)
	pushl   FRAME_THIS /* DeeInstance_GetMember:self */
	ADJUST_CFA_OFFSET(4)
	pushl   TOP                /* DeeInstance_GetMember:tp_self */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeInstance_GetMemberSafe,12)
	ADJUST_CFA_OFFSET(-12)
	testl   %eax, %eax
	jz     .except
	xchgl   TOP, %eax
	decref  %eax
	jmp    .disp

target16_boundmember_this:
	/* `push boundmember this, pop, $<imm16>' */
	lodsw
	jmp     1f
target_boundmember_this:
	/* `push boundmember this, pop, $<imm8>' */
	lodsb
1:	pushl   %eax               /* DeeInstance_GetMember:index */
	ADJUST_CFA_OFFSET(4)
	pushl   FRAME_THIS /* DeeInstance_GetMember:self */
	ADJUST_CFA_OFFSET(4)
	pushl   TOP                /* DeeInstance_GetMember:tp_self */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeInstance_BoundMemberSafe,12)
	ADJUST_CFA_OFFSET(-12)
	xorl    %ecx, %ecx
	cmpl    $0, %eax
	jl     .except
	setnz   %cl
	leal    SYM(Dee_FalseTrue)(,%ecx,8), %eax /* Convert to a boolean. */
	incref  %eax
	xchgl   TOP, %eax
	decref  %eax
	jmp    .disp

target16_delmember_this:
	/* `delmember this, pop, $<imm16>' */
	lodsw
	jmp     1f
target_delmember_this:
	/* `delmember this, pop, $<imm8>' */
	lodsb
1:	pushl   %eax               /* DeeInstance_GetMember:index */
	ADJUST_CFA_OFFSET(4)
	pushl   FRAME_THIS /* DeeInstance_GetMember:self */
	ADJUST_CFA_OFFSET(4)
	pushl   TOP                /* DeeInstance_GetMember:tp_self */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeInstance_DelMemberSafe,12)
	ADJUST_CFA_OFFSET(-12)
	testl   %eax, %eax
	jnz    .except
	sp_popref
	jmp    .disp

target16_setmember_this:
	/* `setmember this, pop, $<imm16>, pop' */
	lodsw
	jmp     1f
target_setmember_this:
	/* `setmember this, pop, $<imm8>, pop' */
	lodsb
1:	pushl   FIRST              /* DeeInstance_GetMember:value */
	ADJUST_CFA_OFFSET(4)
	pushl   %eax               /* DeeInstance_GetMember:index */
	ADJUST_CFA_OFFSET(4)
	pushl   FRAME_THIS /* DeeInstance_GetMember:self */
	ADJUST_CFA_OFFSET(4)
	pushl   SECOND             /* DeeInstance_GetMember:tp_self */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeInstance_SetMemberSafe,16)
	ADJUST_CFA_OFFSET(-16)
	testl   %eax, %eax
	jnz    .except
	sp_popref
	sp_popref
	jmp    .disp



target16_getmember_this_r:
	/* `push getmember this, ref <imm16>, $<imm16>' */
	lodsw
	movl    %eax, %ecx
	lodsw
	jmp     1f
target_getmember_this_r:
	/* `push getmember this, ref <imm8>, $<imm8>' */
	lodsb
	movl    %eax, %ecx
	lodsb
1:
#ifndef NDEBUG
	LOAD_CODE(%edx)
	ASSERT_BELOW(%cx,co_refc(CODE_OR_EDX))
#endif
	pushl   %eax                 /* DeeInstance_GetMember:index */
	ADJUST_CFA_OFFSET(4)
	pushl   FRAME_THIS /* DeeInstance_GetMember:self */
	ADJUST_CFA_OFFSET(4)
	movl    FRAME_FUNC, %edx
	pushl   fo_refv(%edx,%ecx,4) /* DeeInstance_GetMember:tp_self */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeInstance_GetMember,12)
	ADJUST_CFA_OFFSET(-12)
	testl   %eax, %eax
	jz     .except
	sp_push %eax
	jmp    .disp

target16_boundmember_this_r:
	/* `push boundmember this, ref <imm16>, $<imm16>' */
	lodsw
	movl    %eax, %ecx
	lodsw
	jmp     1f
target_boundmember_this_r:
	/* `push boundmember this, ref <imm8>, $<imm8>' */
	lodsb
	movl    %eax, %ecx
	lodsb
1:
#ifndef NDEBUG
	LOAD_CODE(%edx)
	ASSERT_BELOW(%cx,co_refc(CODE_OR_EDX))
#endif
	pushl   %eax                 /* DeeInstance_BoundMember:index */
	ADJUST_CFA_OFFSET(4)
	pushl   FRAME_THIS /* DeeInstance_BoundMember:self */
	ADJUST_CFA_OFFSET(4)
	movl    FRAME_FUNC, %edx
	pushl   fo_refv(%edx,%ecx,4) /* DeeInstance_BoundMember:tp_self */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeInstance_BoundMember,12)
	ADJUST_CFA_OFFSET(-12)
	xorl    %ecx, %ecx
	testl   %eax, %eax
	setnz   %cl
	leal    SYM(Dee_FalseTrue)(,%ecx,8), %eax /* Convert to a boolean. */
	sp_push %eax, 1
	jmp    .disp

target16_delmember_this_r:
	/* `delmember this, ref <imm16>, $<imm16>' */
	lodsw
	movl    %eax, %ecx
	lodsw
	jmp     1f
target_delmember_this_r:
	/* `delmember this, ref <imm8>, $<imm8>' */
	lodsb
	movl    %eax, %ecx
	lodsb
1:
#ifndef NDEBUG
	LOAD_CODE(%edx)
	ASSERT_BELOW(%cx,co_refc(CODE_OR_EDX))
#endif
	pushl   %eax                 /* DeeInstance_DelMember:index */
	ADJUST_CFA_OFFSET(4)
	pushl   FRAME_THIS /* DeeInstance_DelMember:self */
	ADJUST_CFA_OFFSET(4)
	movl    FRAME_FUNC, %edx
	pushl   fo_refv(%edx,%ecx,4) /* DeeInstance_DelMember:tp_self */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeInstance_DelMember,12)
	ADJUST_CFA_OFFSET(-12)
	testl   %eax, %eax
	jz     .disp
	jmp    .except


target16_setmember_this_r:
	/* `setmember this, ref <imm16>, $<imm16>, pop' */
	lodsw
	movl    %eax, %ecx
	lodsw
	jmp     1f
target_setmember_this_r:
	/* `setmember this, ref <imm8>, $<imm8>, pop' */
	lodsb
	movl    %eax, %ecx
	lodsb
1:
#ifndef NDEBUG
	LOAD_CODE(%edx)
	ASSERT_BELOW(%cx,co_refc(CODE_OR_EDX))
#endif
	pushl   TOP                  /* DeeInstance_SetMember:value */
	ADJUST_CFA_OFFSET(4)
	pushl   %eax                 /* DeeInstance_SetMember:index */
	ADJUST_CFA_OFFSET(4)
	pushl   FRAME_THIS           /* DeeInstance_SetMember:self */
	ADJUST_CFA_OFFSET(4)
	movl    FRAME_FUNC, %edx
	pushl   fo_refv(%edx,%ecx,4) /* DeeInstance_SetMember:tp_self */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeInstance_SetMember,16)
	ADJUST_CFA_OFFSET(-16)
	sp_popref
	jmp    .disp


target16_getmember:
	/* `getmember top, pop, $<imm16>' */
	lodsw
	jmp     1f
target_getmember:
	/* `getmember top, pop, $<imm8>' */
	lodsb
1:	pushl   %eax   /* DeeInstance_GetMemberSafe:addr */
	ADJUST_CFA_OFFSET(4)
	pushl   SECOND /* DeeInstance_GetMemberSafe:self */
	ADJUST_CFA_OFFSET(4)
	pushl   FIRST  /* DeeInstance_GetMemberSafe:tp_self */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeInstance_GetMemberSafe,12)
	ADJUST_CFA_OFFSET(-12)
	testl   %eax, %eax
	jz     .except
	xchgl   SECOND, %eax
	decref  %eax
	sp_popref %eax
	jmp    .disp


target16_boundmember:
	/* `boundmember top, pop, $<imm16>' */
	lodsw
	jmp     1f
target_boundmember:
	/* `boundmember top, pop, $<imm8>' */
	lodsb
1:	pushl   %eax   /* DeeInstance_BoundMemberSafe:addr */
	ADJUST_CFA_OFFSET(4)
	pushl   SECOND /* DeeInstance_BoundMemberSafe:self */
	ADJUST_CFA_OFFSET(4)
	pushl   FIRST  /* DeeInstance_BoundMemberSafe:tp_self */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeInstance_BoundMemberSafe,12)
	ADJUST_CFA_OFFSET(-12)
	xorl    %ecx, %ecx
	cmpl    $0, %eax
	jl     .except
	setne   %cl /* cl = EAX != 0 */
	leal    SYM(Dee_FalseTrue)(,%ecx,8), %eax
	incref  %eax
	xchgl   SECOND, %eax /* Save the result onto the stack */
	decref  %eax
	sp_popref %eax
	jmp    .disp


target16_delmember:
	/* `delmember pop, pop, $<imm16>' */
	lodsw
	jmp     1f
target_delmember:
	/* `delmember pop, pop, $<imm8>' */
	lodsb
1:	pushl   %eax   /* DeeInstance_DelMemberSafe:addr */
	ADJUST_CFA_OFFSET(4)
	pushl   SECOND /* DeeInstance_DelMemberSafe:self */
	ADJUST_CFA_OFFSET(4)
	pushl   FIRST  /* DeeInstance_DelMemberSafe:tp_self */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeInstance_DelMemberSafe,12)
	ADJUST_CFA_OFFSET(-12)
	testl   %eax, %eax
	jnz    .except
	sp_popref %eax
	sp_popref %eax
	jmp    .disp


target16_setmember:
	/* `setmember pop, pop, $<imm16>' */
	lodsw
	jmp     1f
target_setmember:
	/* `setmember pop, pop, $<imm8>' */
	lodsb
1:	pushl   FIRST  /* DeeInstance_SetMemberSafe:value */
	ADJUST_CFA_OFFSET(4)
	pushl   %eax   /* DeeInstance_SetMemberSafe:addr */
	ADJUST_CFA_OFFSET(4)
	pushl   THIRD  /* DeeInstance_SetMemberSafe:self */
	ADJUST_CFA_OFFSET(4)
	pushl   SECOND /* DeeInstance_SetMemberSafe:tp_self */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeInstance_SetMemberSafe,16)
	ADJUST_CFA_OFFSET(-16)
	testl   %eax, %eax
	jnz    .except
	sp_popref %eax
	sp_popref %eax
	sp_popref %eax
	jmp    .disp







/* Creation of function objects using constants
 * for code and immediate values for refs. */
target16_function_c_16:
	lodsw
	movl    %eax, %ecx
	lodsw
	jmp    .do_function_c
target16_function_c:
	lodsw
	movl    %eax, %ecx
	xorl    %eax, %eax
	lodsb
	jmp    .do_function_c
target_function_c_16:
	lodsb
	movl    %eax, %ecx
	lodsw
	jmp    .do_function_c
target_function_c:
	lodsb
	movl    %eax, %ecx
	lodsb
.do_function_c:
	LOAD_CODE(%edx)
	ASSERT_BELOW(%cx,co_staticc(CODE_OR_EDX))
	movl    co_staticv(CODE_OR_EDX), %edx
	incl    %eax /* EAX = refc */
	subl    $8, %esp
	ADJUST_CFA_OFFSET(8)
	pushl   %eax            /* DeeFunction_NewInherited:refc */
	ADJUST_CFA_OFFSET(4)
	pushl   (%edx,%ecx,4)   /* DeeFunction_NewInherited:code */
	ADJUST_CFA_OFFSET(4)
	movl    SP, %edx
	leal    (,%eax,4), %ecx
	subl    %ecx, %edx      /* EDX = SP - refc */
	movl    %edx, 8(%esp)   /* DeeFunction_NewInherited:refv */
	movl    %edx, 12(%esp)  /* BACKUP(SP - refc) */
	call    fSYM(DeeFunction_NewInherited,12)
	ADJUST_CFA_OFFSET(-12)
	testl   %eax, %eax
	jz     .except_x4
	popl    SP              /* SP = SP - refc */
	ADJUST_CFA_OFFSET(-4)
	sp_push %eax            /* PUSH(function) */
	jmp    .disp





target_reduce_min:
	pushl   $0
	ADJUST_CFA_OFFSET(4)
	pushl   TOP
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeSeq_Min,8)
	ADJUST_CFA_OFFSET(-8)
	testl   %eax, %eax
	jz     .except
	xchgl   TOP, %eax
	decref  %eax
	jmp    .disp
target_reduce_max:
	pushl   $0
	ADJUST_CFA_OFFSET(4)
	pushl   TOP
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeSeq_Max,8)
	ADJUST_CFA_OFFSET(-8)
	testl   %eax, %eax
	jz     .except
	xchgl   TOP, %eax
	decref  %eax
	jmp    .disp
target_reduce_sum:
	pushl   TOP
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeSeq_Sum,4)
	ADJUST_CFA_OFFSET(-4)
	testl   %eax, %eax
	jz     .except
	xchgl   TOP, %eax
	decref  %eax
	jmp    .disp
target_reduce_any:
	pushl   TOP
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeSeq_Any,4)
	ADJUST_CFA_OFFSET(-4)
	xorl    %ecx, %ecx /* clear out ECX */
	cmpl    $0, %eax
	jl     .except /* EAX < 0 */
	setne   %cl /* cl = EAX != 0 */
	leal    SYM(Dee_FalseTrue)(,%ecx,8), %eax
	incref  %eax
	xchgl   TOP, %eax
	decref  %eax
	jmp    .disp
target_reduce_all:
	pushl   TOP
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeSeq_All,4)
	ADJUST_CFA_OFFSET(-4)
	xorl    %ecx, %ecx /* clear out ECX */
	cmpl    $0, %eax
	jl     .except /* EAX < 0 */
	setne   %cl /* cl = EAX != 0 */
	leal    SYM(Dee_FalseTrue)(,%ecx,8), %eax
	incref  %eax
	xchgl   TOP, %eax
	decref  %eax
	jmp    .disp



/* Unary arithmethic/math operators. */
define_unary_op target_cast_int, fSYM(DeeObject_Int,4)
define_unary_op target_inv, fSYM(DeeObject_Inv,4)
define_unary_op target_pos, fSYM(DeeObject_Pos,4)
define_unary_op target_neg, fSYM(DeeObject_Neg,4)


/* Binary arithmethic/math operators. */
define_binary_op target_add, fSYM(DeeObject_Add,8)
define_binary_op target_sub, fSYM(DeeObject_Sub,8)
define_binary_op target_mul, fSYM(DeeObject_Mul,8)
define_binary_op target_div, fSYM(DeeObject_Div,8)
define_binary_op target_mod, fSYM(DeeObject_Mod,8)
define_binary_op target_shl, fSYM(DeeObject_Shl,8)
define_binary_op target_shr, fSYM(DeeObject_Shr,8)
define_binary_op target_and, fSYM(DeeObject_And,8)
define_binary_op target_or,  fSYM(DeeObject_Or,8)
define_binary_op target_xor, fSYM(DeeObject_Xor,8)
define_binary_op target_pow, fSYM(DeeObject_Pow,8)

.macro define_binary_imm_op target,function,width=b,signed=1
\target:
	lods\width
.if \signed != 0
.ifc \width,b
	movsbl  %al, %eax /* Sign-extend EAX */
.else
.ifc \width,w
	movswl  %ax, %eax /* Sign-extend EAX */
.endif
.endif
.endif
	pushl   %eax /* The second operand. */
	ADJUST_CFA_OFFSET(4)
	pushl   TOP
	ADJUST_CFA_OFFSET(4)
	call    \function
	ADJUST_CFA_OFFSET(-8)
	testl   %eax, %eax
	jz     .except /* Check for errors in the math operation. */
	xchgl   TOP, %eax /* Put the operator result onto the stack. */
	decref  %eax /* decref the old (left) operand. */
	jmp    .disp
.endm

define_binary_imm_op target_add_simm8, fSYM(DeeObject_AddS8,8), b,1
define_binary_imm_op target_add_imm32, fSYM(DeeObject_AddInt,8), l,0
define_binary_imm_op target_sub_simm8, fSYM(DeeObject_SubS8,8), b,1
define_binary_imm_op target_sub_imm32, fSYM(DeeObject_SubInt,8), l,0
define_binary_imm_op target_mul_simm8, fSYM(DeeObject_MulInt,8), b,1
define_binary_imm_op target_div_simm8, fSYM(DeeObject_DivInt,8), b,1
define_binary_imm_op target_mod_simm8, fSYM(DeeObject_ModInt,8), b,1
define_binary_imm_op target_and_imm32, fSYM(DeeObject_AndInt,8), l,0
define_binary_imm_op target_or_imm32,  fSYM(DeeObject_OrInt,8), l,0
define_binary_imm_op target_xor_imm32, fSYM(DeeObject_XorInt,8), l,0
define_binary_imm_op target_shl_imm8,  fSYM(DeeObject_ShlInt,8), b,0
define_binary_imm_op target_shr_imm8,  fSYM(DeeObject_ShrInt,8), b,0

/* These instructions are only allowed when
 * they appear with a storage class prefix. */
target_inc = invop
target_dec = invop
target_incpost = invop
target_decpost = invop

/* NOP instruction we can simply re-direct to dispatch.
 * Note that we treat _all_ variations of DELOP/NOP as NOPs. */
target_delop          = .disp
target_nop            = .disp
target16_delop        = .disp
target16_nop          = .disp
prefix_target_delop   = .disp
prefix_target_nop     = .disp
prefix_target16_delop = .disp
prefix_target16_nop   = .disp


/* I/O Print instructions. */
target_print:
	pushl  $(DEE_STDOUT)
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeFile_GetStd,4)
	ADJUST_CFA_OFFSET(-4)
	testl  %eax, %eax
	jz    .except
	pushl  %eax /* backup */
	ADJUST_CFA_OFFSET(4)
	pushl  TOP
	ADJUST_CFA_OFFSET(4)
	pushl  %eax /* DeeFile_Stdout */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeFile_PrintObject,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jnz   .except_decref_pop /* Check for errors. */
	sp_popref %eax
	popl   %eax
	ADJUST_CFA_OFFSET(-4)
	decref %eax /* DeeFile_Stdout */
	jmp   .disp
target_print_sp:
	pushl  $(DEE_STDOUT)
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeFile_GetStd,4)
	ADJUST_CFA_OFFSET(-4)
	testl  %eax, %eax
	jz    .except
	pushl  %eax /* backup */
	ADJUST_CFA_OFFSET(4)
	pushl  TOP
	ADJUST_CFA_OFFSET(4)
	pushl  %eax /* DeeFile_Stdout */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeFile_PrintObjectSp,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jnz   .except_decref_pop /* Check for errors. */
	sp_popref %eax
	popl   %eax
	ADJUST_CFA_OFFSET(-4)
	decref %eax
	jmp   .disp
target_print_nl:
	pushl  $(DEE_STDOUT)
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeFile_GetStd,4)
	ADJUST_CFA_OFFSET(-4)
	testl  %eax, %eax
	jz    .except
	pushl  %eax /* backup */
	ADJUST_CFA_OFFSET(4)
	pushl  TOP
	ADJUST_CFA_OFFSET(4)
	pushl  %eax /* DeeFile_Stdout */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeFile_PrintObjectNl,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jnz   .except_decref_pop /* Check for errors. */
	sp_popref %eax
	popl   %eax
	ADJUST_CFA_OFFSET(-4)
	decref %eax
	jmp   .disp
target_printall:
	pushl  $(DEE_STDOUT)
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeFile_GetStd,4)
	ADJUST_CFA_OFFSET(-4)
	testl  %eax, %eax
	jz    .except
	pushl  %eax /* backup */
	ADJUST_CFA_OFFSET(4)
	pushl  TOP
	ADJUST_CFA_OFFSET(4)
	pushl  %eax /* DeeFile_Stdout */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeFile_PrintAll,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jnz   .except_decref_pop /* Check for errors. */
	sp_popref %eax
	popl   %eax
	ADJUST_CFA_OFFSET(-4)
	decref %eax
	jmp   .disp
target_printall_sp:
	pushl  $(DEE_STDOUT)
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeFile_GetStd,4)
	ADJUST_CFA_OFFSET(-4)
	testl  %eax, %eax
	jz    .except
	pushl  %eax /* backup */
	ADJUST_CFA_OFFSET(4)
	pushl  TOP
	ADJUST_CFA_OFFSET(4)
	pushl  %eax /* DeeFile_Stdout */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeFile_PrintAllSp,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jnz   .except_decref_pop /* Check for errors. */
	sp_popref %eax
	popl   %eax
	ADJUST_CFA_OFFSET(-4)
	decref %eax
	jmp   .disp
target_printall_nl:
	pushl  $(DEE_STDOUT)
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeFile_GetStd,4)
	ADJUST_CFA_OFFSET(-4)
	testl  %eax, %eax
	jz    .except
	pushl  %eax /* backup */
	ADJUST_CFA_OFFSET(4)
	pushl  TOP
	ADJUST_CFA_OFFSET(4)
	pushl  %eax /* DeeFile_Stdout */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeFile_PrintAllNl,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jnz   .except_decref_pop /* Check for errors. */
	sp_popref %eax
	popl   %eax
	ADJUST_CFA_OFFSET(-4)
	decref %eax
	jmp   .disp
target_printnl:
	pushl  $(DEE_STDOUT)
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeFile_GetStd,4)
	ADJUST_CFA_OFFSET(-4)
	testl  %eax, %eax
	jz    .except
	pushl  %eax /* backup */
	ADJUST_CFA_OFFSET(4)
	pushl  %eax /* DeeFile_Stdout */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeFile_PrintNl,4)
	ADJUST_CFA_OFFSET(-4)
	testl  %eax, %eax
	jnz   .except_decref_pop /* Check for errors. */
	popl   %eax
	ADJUST_CFA_OFFSET(-4)
	decref %eax
	jmp   .disp
target_fprint:
	pushl  FIRST
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeFile_PrintObject,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jnz   .except /* Check for errors. */
	sp_popref %eax
	jmp   .disp
target_fprint_sp:
	pushl  FIRST
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeFile_PrintObjectSp,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jnz   .except /* Check for errors. */
	sp_popref %eax
	jmp   .disp
target_fprint_nl:
	pushl  FIRST
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeFile_PrintObjectNl,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jnz   .except /* Check for errors. */
	sp_popref %eax
	jmp   .disp
target_fprintall:
	pushl  FIRST
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeFile_PrintAll,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jnz   .except /* Check for errors. */
	sp_popref %eax
	jmp   .disp
target_fprintall_sp:
	pushl  FIRST
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeFile_PrintAllSp,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jnz   .except /* Check for errors. */
	sp_popref %eax
	jmp   .disp
target_fprintall_nl:
	pushl  FIRST
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeFile_PrintAllNl,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jnz   .except /* Check for errors. */
	sp_popref %eax
	jmp   .disp
target_fprintnl:
	pushl  FIRST
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeFile_PrintNl,4)
	ADJUST_CFA_OFFSET(-4)
	testl  %eax, %eax
	jz    .disp /* Check for errors. */
	jmp   .except



target16_print_c:
	lodsw
	jmp    1f
target_print_c:
	lodsb
1:	LOAD_CODE(%ecx)
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
	movl   co_staticv(CODE_OR_ECX), %ecx
	subl   $4, %esp /* Make space for a backup of `DeeFile_Stdout' */
	ADJUST_CFA_OFFSET(4)
	pushl  (%ecx,%eax,4) /* Push the referenced constant. */
	ADJUST_CFA_OFFSET(4)
	pushl  $(DEE_STDOUT)
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeFile_GetStd,4)
	ADJUST_CFA_OFFSET(-4)
	testl  %eax, %eax
	jz    .except_x8
	movl   %eax, 4(%esp) /* Backup DeeFile_Stdout */
	pushl  %eax /* DeeFile_Stdout */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeFile_PrintObject,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jnz   .except_decref_pop /* Check for errors. */
	popl   %eax
	ADJUST_CFA_OFFSET(-4)
	decref %eax /* DeeFile_Stdout */
	jmp   .disp /* Check for errors. */
target16_print_c_sp:
	lodsw
	jmp    1f
target_print_c_sp:
	lodsb
1:	LOAD_CODE(%ecx)
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
	movl   co_staticv(CODE_OR_ECX), %ecx
	subl   $4, %esp /* Make space for a backup of `DeeFile_Stdout' */
	ADJUST_CFA_OFFSET(4)
	pushl  (%ecx,%eax,4) /* Push the referenced constant. */
	ADJUST_CFA_OFFSET(4)
	pushl  $(DEE_STDOUT)
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeFile_GetStd,4)
	ADJUST_CFA_OFFSET(-4)
	testl  %eax, %eax
	jz    .except_x8
	movl   %eax, 4(%esp) /* Backup DeeFile_Stdout */
	pushl  %eax /* DeeFile_Stdout */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeFile_PrintObjectSp,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jnz   .except_decref_pop /* Check for errors. */
	popl   %eax
	ADJUST_CFA_OFFSET(-4)
	decref %eax /* DeeFile_Stdout */
	jmp   .disp /* Check for errors. */
target16_print_c_nl:
	lodsw
	jmp    1f
target_print_c_nl:
	lodsb
1:	LOAD_CODE(%ecx)
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
	movl   co_staticv(CODE_OR_ECX), %ecx
	subl   $4, %esp /* Make space for a backup of `DeeFile_Stdout' */
	ADJUST_CFA_OFFSET(4)
	pushl  (%ecx,%eax,4) /* Push the referenced constant. */
	ADJUST_CFA_OFFSET(4)
	pushl  $(DEE_STDOUT)
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeFile_GetStd,4)
	ADJUST_CFA_OFFSET(-4)
	testl  %eax, %eax
	jz    .except_x8
	movl   %eax, 4(%esp) /* Backup DeeFile_Stdout */
	pushl  %eax /* DeeFile_Stdout */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeFile_PrintObjectNl,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jnz   .except_decref_pop /* Check for errors. */
	popl   %eax
	ADJUST_CFA_OFFSET(-4)
	decref %eax /* DeeFile_Stdout */
	jmp   .disp /* Check for errors. */



target16_fprint_c:
	lodsw
	jmp    1f
target_fprint_c:
	lodsb
1:	LOAD_CODE(%ecx)
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
	movl   co_staticv(CODE_OR_ECX), %ecx
	pushl  (%ecx,%eax,4) /* Push the referenced constant. */
	ADJUST_CFA_OFFSET(4)
	pushl  TOP /* The file that should be printed to. */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeFile_PrintObject,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jz    .disp /* Check for errors. */
	jmp   .except
target16_fprint_c_sp:
	lodsw
	jmp    1f
target_fprint_c_sp:
	lodsb
1:	LOAD_CODE(%ecx)
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
	movl   co_staticv(CODE_OR_ECX), %ecx
	pushl  (%ecx,%eax,4) /* Push the referenced constant. */
	ADJUST_CFA_OFFSET(4)
	pushl  TOP /* The file that should be printed to. */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeFile_PrintObjectSp,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jz    .disp /* Check for errors. */
	jmp   .except
target16_fprint_c_nl:
	lodsw
	jmp    1f
target_fprint_c_nl:
	lodsb
1:	LOAD_CODE(%ecx)
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
	movl   co_staticv(CODE_OR_ECX), %ecx
	pushl  (%ecx,%eax,4) /* Push the referenced constant. */
	ADJUST_CFA_OFFSET(4)
	pushl  TOP /* The file that should be printed to. */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeFile_PrintObjectNl,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jz    .disp /* Check for errors. */
	jmp   .except



target_range_0_i16:
	lodsw
1:	pushl  $1 /* step */
	ADJUST_CFA_OFFSET(4)
	pushl  %eax /* end */
	ADJUST_CFA_OFFSET(4)
	pushl  $0 /* begin */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeRange_NewInt,12)
	ADJUST_CFA_OFFSET(-12)
	testl  %eax, %eax
	jz    .except
	sp_push %eax
	jmp   .disp
target_range_0_i32:
	lodsl
	testl  $0x80000000, %eax
	jz     1b /* if (EAX <= SSIZE_MAX) goto 1b. */
	/* Special case: We must create an unsigned range, but
	 *               the given target length doesn't fit.
	 *               Therefor, we must create an object-style
	 *               range in order to prevent the potential
	 *               overflow. */
	pushl  %eax
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeInt_NewU32,4)
	ADJUST_CFA_OFFSET(-4)
	testl  %eax, %eax
	jz    .except
	pushl  %eax /* backup */
	ADJUST_CFA_OFFSET(4)
	pushl  $(SYM(DeeInt_One)) /* step */
	ADJUST_CFA_OFFSET(4)
	pushl  %eax /* end */
	ADJUST_CFA_OFFSET(4)
	pushl  $(SYM(DeeInt_Zero)) /* begin */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeRange_New,12)
	ADJUST_CFA_OFFSET(-12)
	testl  %eax, %eax
	jz    .except_decref_pop
	sp_push %eax /* Push the new range onto the stack. */
	popl   %edx /* Drop our temporary object. */
	ADJUST_CFA_OFFSET(-4)
	decref %edx
	jmp   .disp

target_range:
	/* General-purpose range (using objects passed through the stack) */
	pushl  $0 /* step (unused) */
	ADJUST_CFA_OFFSET(4)
	pushl  FIRST /* end */
	ADJUST_CFA_OFFSET(4)
	movl   SECOND, %eax
	cmpl   $(SYM(DeeNone_Singleton)), %eax
	jne    1f
	leal   SYM(DeeInt_Zero), %eax
1:	pushl  %eax /* begin */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeRange_New,12)
	ADJUST_CFA_OFFSET(-12)
	testl  %eax, %eax
	jz    .except
	xchgl  SECOND, %eax
	decref %eax
	sp_popref %eax
	jmp   .disp
target_range_def:
	subl   $4, %esp /* backup(begin) */
	ADJUST_CFA_OFFSET(4)
	pushl  $0 /* step (unused) */
	ADJUST_CFA_OFFSET(4)
	movl   TOP, %eax
	pushl  %eax /* end */
	ADJUST_CFA_OFFSET(4)
	pushl  ob_type(%eax) /* Dee_TYPE(end) */
	ADJUST_CFA_OFFSET(4)
	/* Construct a default-instance of the end type for the start-type. */
	call   fSYM(DeeObject_NewDefault,4)
	ADJUST_CFA_OFFSET(-4)
	testl  %eax, %eax
	jz    .except_x12
	movl   %eax, 8(%esp) /* backup(begin) = begin; */
	pushl  %eax /* begin */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeRange_New,12)
	ADJUST_CFA_OFFSET(-12)
	testl  %eax, %eax
	jz    .except_decref_pop
	xchgl  TOP, %eax
	decref %eax
	popl   %eax /* backup(begin) */
	ADJUST_CFA_OFFSET(-4)
	decref %eax
	jmp   .disp
target_range_step:
	/* General-purpose range (using objects passed through the stack) */
	movl   FIRST, %eax
	cmpl   $(SYM(DeeNone_Singleton)), %eax
	jne    1f
	xorl   %eax, %eax /* Discard the step when its just `none' */
1:	pushl  %eax /* step */
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND /* end */
	ADJUST_CFA_OFFSET(4)
	movl   THIRD, %eax
	cmpl   $(SYM(DeeNone_Singleton)), %eax
	jne    1f
	leal   SYM(DeeInt_Zero), %eax
1:	pushl  %eax /* begin */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeRange_New,12)
	ADJUST_CFA_OFFSET(-12)
	testl  %eax, %eax
	jz    .except
	xchgl  THIRD, %eax
	decref %eax
	sp_popref %eax
	sp_popref %eax
	jmp   .disp
target_range_step_def:
	/* General-purpose range (using objects passed through the stack) */
	subl   $4, %esp /* backup(begin) */
	ADJUST_CFA_OFFSET(4)
	movl   FIRST, %eax
	cmpl   $(SYM(DeeNone_Singleton)), %eax
	jne    1f
	xorl   %eax, %eax /* Discard the step when its just `none' */
1:	pushl  %eax /* step */
	ADJUST_CFA_OFFSET(4)
	movl   SECOND, %eax
	pushl  %eax /* end */
	ADJUST_CFA_OFFSET(4)
	pushl  ob_type(%eax) /* Dee_TYPE(end) */
	ADJUST_CFA_OFFSET(4)
	/* Construct a default-instance of the end type for the start-type. */
	call   fSYM(DeeObject_NewDefault,4)
	ADJUST_CFA_OFFSET(-4)
	testl  %eax, %eax
	jz    .except_x12
	movl   %eax, 8(%esp) /* backup(begin) */
	pushl  %eax /* begin */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeRange_New,12)
	ADJUST_CFA_OFFSET(-12)
	testl  %eax, %eax
	jz    .except_decref_pop
	xchgl  SECOND, %eax
	decref %eax
	sp_popref %eax
	popl   %eax /* backup(begin) */
	ADJUST_CFA_OFFSET(-4)
	decref %eax
	jmp   .disp



target_enter:
	pushl  TOP
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_Enter,4)
	ADJUST_CFA_OFFSET(-4)
	testl  %eax, %eax
	jz    .disp /* Check for errors. */
	jmp   .except
target_leave:
	pushl  TOP
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_Leave,4)
	ADJUST_CFA_OFFSET(-4)
	testl  %eax, %eax
	jnz   .except
	sp_popref %eax /* unlike ENTER, LEAVE pops the operand. */
	jmp   .disp



define_unary_op target_getsize, fSYM(DeeObject_SizeObject,4)
define_binary_op target_contains, fSYM(DeeObject_ContainsObject,8)
define_binary_op target_getitem, fSYM(DeeObject_GetItem,8)
target_setitem:
	pushl  FIRST
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND
	ADJUST_CFA_OFFSET(4)
	pushl  THIRD
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_SetItem,12)
	ADJUST_CFA_OFFSET(-12)
	testl  %eax, %eax
	jnz   .except
	sp_popref %eax
	sp_popref %eax
	sp_popref %eax
	jmp   .disp
target_delitem:
	pushl  FIRST
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_DelItem,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jnz   .except
	sp_popref %eax
	sp_popref %eax
	jmp   .disp
target_getrange:
	pushl  FIRST
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND
	ADJUST_CFA_OFFSET(4)
	pushl  THIRD
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_GetRange,12)
	ADJUST_CFA_OFFSET(-12)
	testl  %eax, %eax
	jz    .except
	xchgl  THIRD, %eax /* Push the result onto the stack. */
	decref %eax /* decref() the old object. */
	sp_popref %eax
	sp_popref %eax
	jmp   .disp
target_delrange:
	pushl  FIRST
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND
	ADJUST_CFA_OFFSET(4)
	pushl  THIRD
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_DelRange,12)
	ADJUST_CFA_OFFSET(-12)
	testl  %eax, %eax
	jnz   .except
	sp_popref %eax
	sp_popref %eax
	sp_popref %eax
	jmp   .disp
target_setrange:
	pushl  FIRST
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND
	ADJUST_CFA_OFFSET(4)
	pushl  THIRD
	ADJUST_CFA_OFFSET(4)
	pushl  FOURTH
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_SetRange,16)
	ADJUST_CFA_OFFSET(-16)
	testl  %eax, %eax
	jnz   .except
	sp_popref %eax
	sp_popref %eax
	sp_popref %eax
	sp_popref %eax
	jmp   .disp

target_bounditem:
	pushl  $1     /* DeeObject_BoundItem:allow_missing */
	ADJUST_CFA_OFFSET(4)
	pushl  FIRST  /* DeeObject_BoundItem:key */
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND /* DeeObject_BoundItem:self */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_BoundItem,12)
	ADJUST_CFA_OFFSET(-12)
	xorl   %ecx, %ecx /* Clear out ECX */
	cmpl   $-1, %eax
	je    .except /* Check for errors. */
	cmpl   $0, %eax
	setg   %cl /* CL = EAX > 0 */
	leal   SYM(Dee_FalseTrue)(,%ecx,8), %eax /* Convert to a boolean. */
	incref %eax /* Incref the boolean value. */
	xchgl  SECOND, %eax /* Store the result onto the stack. */
	decref %eax
	sp_popref %eax
	jmp   .disp


target16_contains_c:
	lodsw
	jmp    1f
target_contains_c:
	lodsb
1:	LOAD_CODE(%ecx)
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
	movl   co_staticv(CODE_OR_ECX), %ecx
	pushl  TOP
	ADJUST_CFA_OFFSET(4)
	pushl  (%ecx,%eax,4) /* Constant set. */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_ContainsObject,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jz    .except /* Check for errors. */
	xchgl  TOP, %eax /* Push the result onto the stack. */
	decref %eax /* decref() the sequence. */
	jmp   .disp



target16_getitem_c:
	lodsw
	jmp    1f
target_getitem_c:
	lodsb
1:	LOAD_CODE(%ecx)
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
	movl   co_staticv(CODE_OR_ECX), %ecx
	pushl  (%ecx,%eax,4) /* Constant index. */
	ADJUST_CFA_OFFSET(4)
	pushl  TOP
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_GetItem,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jz    .except /* Check for errors. */
	xchgl  TOP, %eax /* Push the result onto the stack. */
	decref %eax /* decref() the sequence. */
	jmp   .disp
target16_setitem_c:
	lodsw
	jmp    1f
target_setitem_c:
	lodsb
1:	LOAD_CODE(%ecx)
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
	movl   co_staticv(CODE_OR_ECX), %ecx
	pushl  FIRST
	ADJUST_CFA_OFFSET(4)
	pushl  (%ecx,%eax,4) /* Constant index. */
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_SetItem,12)
	ADJUST_CFA_OFFSET(-12)
	testl  %eax, %eax
	jnz   .except /* Check for errors. */
	sp_popref %eax
	sp_popref %eax
	jmp   .disp
target_getitem_i:
	lodsw
	movswl %ax, %eax
	pushl  %eax
	ADJUST_CFA_OFFSET(4)
	pushl  TOP
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_GetItemIndex,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jz    .except /* Check for errors. */
	xchgl  TOP, %eax /* Push the result onto the stack. */
	decref %eax /* Decref the sequence */
	jmp   .disp
target_setitem_i:
	lodsw
	movswl %ax, %eax
	pushl  FIRST
	ADJUST_CFA_OFFSET(4)
	pushl  %eax
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_SetItemIndex,12)
	ADJUST_CFA_OFFSET(-12)
	testl  %eax, %eax
	jnz   .except /* Check for errors. */
	sp_popref %eax
	sp_popref %eax
	jmp   .disp

target_getrange_pn:
	pushl  $(SYM(DeeNone_Singleton))
	ADJUST_CFA_OFFSET(4)
	pushl  FIRST
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_GetRange,12)
	ADJUST_CFA_OFFSET(-12)
	testl  %eax, %eax
	jz    .except /* Check for errors. */
	xchgl  SECOND, %eax /* Store the result onto the stack. */
	decref %eax
	sp_popref %eax
	jmp   .disp
target_getrange_np:
	pushl  FIRST
	ADJUST_CFA_OFFSET(4)
	pushl  $(SYM(DeeNone_Singleton))
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_GetRange,12)
	ADJUST_CFA_OFFSET(-12)
	testl  %eax, %eax
	jz    .except /* Check for errors. */
	xchgl  SECOND, %eax /* Store the result onto the stack. */
	decref %eax
	sp_popref %eax
	jmp   .disp
target_getrange_pi:
	lodsw
	movswl %ax, %eax /* Sign-extend index. */
	pushl  %eax
	ADJUST_CFA_OFFSET(4)
	pushl  FIRST
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_GetRangeEndIndex,12)
	ADJUST_CFA_OFFSET(-12)
	testl  %eax, %eax
	jz    .except /* Check for errors. */
	xchgl  SECOND, %eax /* Store the result onto the stack. */
	decref %eax
	sp_popref %eax
	jmp   .disp
target_getrange_ip:
	lodsw
	movswl %ax, %eax /* Sign-extend index. */
	pushl  FIRST
	ADJUST_CFA_OFFSET(4)
	pushl  %eax
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_GetRangeBeginIndex,12)
	ADJUST_CFA_OFFSET(-12)
	testl  %eax, %eax
	jz    .except /* Check for errors. */
	xchgl  SECOND, %eax /* Store the result onto the stack. */
	decref %eax
	sp_popref %eax
	jmp   .disp
target_getrange_ni:
	lodsw
	movswl %ax, %eax /* Sign-extend index. */
	pushl  %eax
	ADJUST_CFA_OFFSET(4)
	pushl  $(SYM(DeeNone_Singleton))
	ADJUST_CFA_OFFSET(4)
	pushl  FIRST
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_GetRangeEndIndex,12)
	ADJUST_CFA_OFFSET(-12)
	testl  %eax, %eax
	jz    .except /* Check for errors. */
	xchgl  FIRST, %eax /* Store the result onto the stack. */
	decref %eax
	jmp   .disp
target_getrange_in:
	lodsw
	movswl %ax, %eax /* Sign-extend index. */
	pushl  $(SYM(DeeNone_Singleton))
	ADJUST_CFA_OFFSET(4)
	pushl  %eax
	ADJUST_CFA_OFFSET(4)
	pushl  FIRST
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_GetRangeBeginIndex,12)
	ADJUST_CFA_OFFSET(-12)
	testl  %eax, %eax
	jz    .except /* Check for errors. */
	xchgl  FIRST, %eax /* Store the result onto the stack. */
	decref %eax
	jmp   .disp
target_setrange_pn:
	pushl  FIRST
	ADJUST_CFA_OFFSET(4)
	pushl  $(SYM(DeeNone_Singleton))
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND
	ADJUST_CFA_OFFSET(4)
	pushl  THIRD
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_SetRange,16)
	ADJUST_CFA_OFFSET(-16)
	testl  %eax, %eax
	jnz   .except /* Check for errors. */
	sp_popref %eax
	sp_popref %eax
	sp_popref %eax
	jmp   .disp
target_setrange_np:
	pushl  FIRST
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND
	ADJUST_CFA_OFFSET(4)
	pushl  $(SYM(DeeNone_Singleton))
	ADJUST_CFA_OFFSET(4)
	pushl  THIRD
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_SetRange,16)
	ADJUST_CFA_OFFSET(-16)
	testl  %eax, %eax
	jnz   .except /* Check for errors. */
	sp_popref %eax
	sp_popref %eax
	sp_popref %eax
	jmp   .disp
target_setrange_pi:
	lodsw
	movswl %ax, %eax /* Sign-extend index. */
	pushl  FIRST
	ADJUST_CFA_OFFSET(4)
	pushl  %eax
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND
	ADJUST_CFA_OFFSET(4)
	pushl  THIRD
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_SetRangeEndIndex,16)
	ADJUST_CFA_OFFSET(-16)
	testl  %eax, %eax
	jnz   .except /* Check for errors. */
	sp_popref %eax
	sp_popref %eax
	sp_popref %eax
	jmp   .disp
target_setrange_ip:
	lodsw
	movswl %ax, %eax /* Sign-extend index. */
	pushl  FIRST
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND
	ADJUST_CFA_OFFSET(4)
	pushl  %eax
	ADJUST_CFA_OFFSET(4)
	pushl  THIRD
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_SetRangeBeginIndex,16)
	ADJUST_CFA_OFFSET(-16)
	testl  %eax, %eax
	jnz   .except /* Check for errors. */
	sp_popref %eax
	sp_popref %eax
	sp_popref %eax
	jmp   .disp
target_setrange_ni:
	lodsw
	movswl %ax, %eax /* Sign-extend index. */
	pushl  FIRST
	ADJUST_CFA_OFFSET(4)
	pushl  %eax
	ADJUST_CFA_OFFSET(4)
	pushl  $(SYM(DeeNone_Singleton))
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_SetRangeEndIndex,16)
	ADJUST_CFA_OFFSET(-16)
	testl  %eax, %eax
	jnz   .except /* Check for errors. */
	sp_popref %eax
	sp_popref %eax
	jmp   .disp
target_setrange_in:
	lodsw
	movswl %ax, %eax /* Sign-extend index. */
	pushl  FIRST
	ADJUST_CFA_OFFSET(4)
	pushl  $(SYM(DeeNone_Singleton))
	ADJUST_CFA_OFFSET(4)
	pushl  %eax
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_SetRangeBeginIndex,16)
	ADJUST_CFA_OFFSET(-16)
	testl  %eax, %eax
	jnz   .except /* Check for errors. */
	sp_popref %eax
	sp_popref %eax
	jmp   .disp

target_getrange_ii:
	lodsw
	movswl %ax, %ecx /* Sign-extend begin-index. */
	lodsw
	movswl %ax, %eax /* Sign-extend end-index. */
	pushl  %eax
	ADJUST_CFA_OFFSET(4)
	pushl  %ecx
	ADJUST_CFA_OFFSET(4)
	pushl  TOP
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_GetRangeIndex,12)
	ADJUST_CFA_OFFSET(-12)
	testl  %eax, %eax
	jz    .except
	xchgl  TOP, %eax /* Store the result onto the stack. */
	decref %eax
	jmp   .disp

target_setrange_ii:
	lodsw
	movswl  %ax, %ecx /* Sign-extend begin-index. */
	lodsw
	movswl  %ax, %eax /* Sign-extend end-index. */
	pushl   FIRST
	ADJUST_CFA_OFFSET(4)
	pushl   %eax
	ADJUST_CFA_OFFSET(4)
	pushl   %ecx
	ADJUST_CFA_OFFSET(4)
	pushl   SECOND
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeObject_SetRangeIndex,16)
	ADJUST_CFA_OFFSET(-16)
	testl   %eax, %eax
	jnz    .except
	sp_popref %eax
	sp_popref %eax
	jmp    .disp



/* Call attribute instructions. */
target_callattr:
	lodsb
	leal    (,%eax,4), %ecx
	movl    SP, %edx
	subl    %ecx, %edx
	pushl   %eax /* BACKUP(n_args) */
	ADJUST_CFA_OFFSET(4)
	pushl   %edx /* BACKUP(SP - n_args) */
	ADJUST_CFA_OFFSET(4)
	pushl   %edx /* SP - n_args */
	ADJUST_CFA_OFFSET(4)
	pushl   %eax /* n_args */
	ADJUST_CFA_OFFSET(4)
	movl    -4(%edx), %eax
	cmpl    $(SYM(DeeString_Type)), ob_type(%eax)
	jne     99f  /* Check if the name-operand is a string. */
	pushl   %eax     /* name */
	ADJUST_CFA_OFFSET(4)
	pushl   -8(%edx) /* self */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeObject_CallAttr,16)
	ADJUST_CFA_OFFSET(-16)
	testl   %eax, %eax
	jz     .except_x8 /* Check for errors in the callback */
	popl    %edx /* EDX = SP - n_args */
	ADJUST_CFA_OFFSET(-4)
	xchgl   -8(%edx), %eax /* Save the result onto the stack. */
	decref  %eax /* Decref(self) */
	popl    %ecx /* ECX = n_args */
	ADJUST_CFA_OFFSET(-4)
	incl    %ecx /* ECX = n_args+1 */
1:	/* Pop the attribute name and arguments from the stack.
	 * Combining this, we can simply pop n_args+1 items. */
	sp_popref %eax, 0, 1, 0 /* Preserve ECX */
	loop    1b
	jmp    .disp
	ADJUST_CFA_OFFSET(16)
99:	addl    $16, %esp
	ADJUST_CFA_OFFSET(-16)
	pushl   %eax
	ADJUST_CFA_OFFSET(4)
	decref  %eax /* Drop the argument tuple. */
	call    fSYM(err_expected_string_for_attribute,4)
	ADJUST_CFA_OFFSET(-4)
	jmp    .except

target_callattr_tuple:
	movl    SECOND, %edx
	cmpl    $(SYM(DeeString_Type)), ob_type(%edx)
	jne     99f /* Check if the name operand is a string. */
	pushl   FIRST        /* args */
	ADJUST_CFA_OFFSET(4)
	pushl   %edx         /* name */
	ADJUST_CFA_OFFSET(4)
	pushl   THIRD        /* self */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeObject_CallAttrTuple,12)
	ADJUST_CFA_OFFSET(-12)
	testl   %eax, %eax
	jz     .except
	xchgl   THIRD, %eax  /* Save the result onto the stack. */
	decref  %eax
	sp_popref %eax
	sp_popref %eax
	jmp    .disp
99:	pushl   %edx
	ADJUST_CFA_OFFSET(4)
	call    fSYM(err_expected_string_for_attribute,4)
	ADJUST_CFA_OFFSET(-4)
	jmp    .except



target16_callattr_c:
	lodsw
	jmp     1f
target_callattr_c:
	lodsb
1:	/* At this point, the attribute constant id is stored in EAX */
	LOAD_CODE(%ecx)
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
	movl    co_staticv(CODE_OR_ECX), %ecx
	movl    (%ecx,%eax,4), %edx /* Attribute name object is now stored in EDX */
	ASSERT_NONZERO(%edx)
	WASSERT_EQUAL(l,ob_type(%edx),$(SYM(DeeString_Type)))
	xorl    %eax, %eax
	lodsb
	pushl   %eax            /* BACKUP(n_args) */
	ADJUST_CFA_OFFSET(4)
	subl    $8, %esp        /* BACKUP(SP - n_args), SP - n_args */
	ADJUST_CFA_OFFSET(8)
	pushl   %eax            /* n_args */
	ADJUST_CFA_OFFSET(4)
	pushl   %edx            /* name */
	ADJUST_CFA_OFFSET(4)
	leal    (,%eax,4), %ecx
	movl    SP, %edx
	subl    %ecx, %edx
	movl    %edx, 8(%esp)   /* SP - n_args */
	movl    %edx, 12(%esp)  /* BACKUP(SP - n_args) */
	pushl   -4(%edx)        /* self */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeObject_CallAttr,16)
	ADJUST_CFA_OFFSET(-16)
	/* 0(ESP): BACK(SP - n_args) */
	/* 4(ESP): BACK(n_args) */
	testl   %eax, %eax
	jz     .except_x8
	popl    %edx /* EDX = SP - n_args */
	ADJUST_CFA_OFFSET(-4)
	xchgl   -4(%edx), %eax /* Save the result onto the stack (override `self') */
	decref  %eax
	popl    %ecx /* ECX = n_args */
	ADJUST_CFA_OFFSET(-4)
	testl   %ecx, %ecx
	jz     .disp
1:	/* Pop arguments from the stack */
	sp_popref %eax, 0, 1, 0 /* Preserve ECX */
	loop    1b
	jmp    .disp



target16_callattr_c_tuple:
	lodsw
	jmp     1f
target_callattr_c_tuple:
	lodsb
1:	LOAD_CODE(%ecx)
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
	movl    co_staticv(CODE_OR_ECX), %ecx
	movl    (%ecx,%eax,4), %eax
	WASSERT_EQUAL(l,ob_type(%eax),$(SYM(DeeString_Type)))
	pushl   FIRST        /* DeeObject_CallAttrTuple:args */
	ADJUST_CFA_OFFSET(4)
	pushl   %eax         /* DeeObject_CallAttrTuple:name */
	ADJUST_CFA_OFFSET(4)
	pushl   SECOND       /* DeeObject_CallAttrTuple:self */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeObject_CallAttrTuple,12)
	ADJUST_CFA_OFFSET(-12)
	testl   %eax, %eax
	jz     .except /* Check for errors. */
	xchgl   SECOND, %eax /* Store the result onto the stack. */
	decref  %eax
	sp_popref %eax
	jmp    .disp



target16_callattr_this_c:
	lodsw
	jmp     1f
target_callattr_this_c:
	lodsb
1:	/* At this point, the attribute constant id is stored in EAX */
	LOAD_CODE(%ecx)
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
	ASSERT_BIT(w,$(CODE_FTHISCALL),co_flags(CODE_OR_ECX))
	movl    co_staticv(CODE_OR_ECX), %ecx
	movl    (%ecx,%eax,4), %edx /* Attribute name object is now stored in EDX */
	ASSERT_NONZERO(%edx)
	WASSERT_EQUAL(l,ob_type(%edx),$(SYM(DeeString_Type)))
	xorl    %eax, %eax
	lodsb
	pushl   %eax            /* BACKUP(n_args) */
	ADJUST_CFA_OFFSET(4)
	subl    $4, %esp        /* SP - n_args */
	ADJUST_CFA_OFFSET(4)
	pushl   %eax            /* n_args */
	ADJUST_CFA_OFFSET(4)
	pushl   %edx            /* name */
	ADJUST_CFA_OFFSET(4)
	leal    (,%eax,4), %ecx
	movl    SP, %edx
	subl    %ecx, %edx
	movl    %edx, 8(%esp)   /* SP - n_args */
	pushl   FRAME_THIS /* self */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeObject_CallAttr,16)
	ADJUST_CFA_OFFSET(-16)
	/* 0(ESP): BACK(n_args) */
	testl   %eax, %eax
	jz     .except_x4
	popl    %ecx /* ECX = n_args */
	ADJUST_CFA_OFFSET(-4)
	testl   %ecx, %ecx
	jz      1f
2:	/* Pop arguments from the stack */
	sp_popref %edx, 1, 1, 0 /* Preserve EAX and ECX */
	loop    2b
1:	sp_push %eax /* Push the result onto the stack. */
	jmp    .disp



target16_callattr_this_c_tuple:
	lodsw
	jmp     1f
target_callattr_this_c_tuple:
	lodsb
1:	LOAD_CODE(%ecx)
	ASSERT_BIT(w,$(CODE_FTHISCALL),co_flags(CODE_OR_ECX))
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
	movl    co_staticv(CODE_OR_ECX), %ecx
	movl    (%ecx,%eax,4), %eax
	WASSERT_EQUAL(l,ob_type(%eax),$(SYM(DeeString_Type)))
	pushl   FIRST        /* DeeObject_CallAttrTuple:args */
	ADJUST_CFA_OFFSET(4)
	pushl   %eax         /* DeeObject_CallAttrTuple:name */
	ADJUST_CFA_OFFSET(4)
	pushl   FRAME_THIS   /* DeeObject_CallAttrTuple:self */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeObject_CallAttrTuple,12)
	ADJUST_CFA_OFFSET(-12)
	testl   %eax, %eax
	jz     .except /* Check for errors. */
	xchgl   FIRST, %eax /* Store the result onto the stack. */
	decref  %eax
	jmp    .disp


target16_callattr_c_seq:
	lodsw
	jmp     1f
target_callattr_c_seq:
	lodsb
1:	subl    $4, %esp      /* DeeObject_CallAttr:argv[0] */
	ADJUST_CFA_OFFSET(4)
	pushl   %esp          /* DeeObject_CallAttr:argv */
	ADJUST_CFA_OFFSET(4)
	pushl   $1            /* DeeObject_CallAttr:argc */
	ADJUST_CFA_OFFSET(4)
	LOAD_CODE(%ecx)
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
	movl    co_staticv(CODE_OR_ECX), %ecx
	pushl   (%ecx,%eax,4) /* DeeObject_CallAttr:attr_name */
	ADJUST_CFA_OFFSET(4)
	lodsb                 /* EAX = argc */
	movl    SP, %edx
	leal    (,%eax,4), %ecx
	subl    %ecx, %edx    /* EDX = new_sp = SP - argc */
	pushl   -4(%edx)      /* DeeObject_CallAttr:self */
	ADJUST_CFA_OFFSET(4)
	pushl   %edx          /* BACKUP(new_sp) */
	ADJUST_CFA_OFFSET(4)
	pushl   %edx          /* DeeSharedVector_NewShared:vector */
	ADJUST_CFA_OFFSET(4)
	pushl   %eax          /* DeeSharedVector_NewShared:length */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeSharedVector_NewShared,8)
	ADJUST_CFA_OFFSET(-8)
	testl   %eax, %eax
	jz     .except_x24
	popl    SP            /* SP = new_sp */
	ADJUST_CFA_OFFSET(-4)
	movl    %eax, 16(%esp) /* DeeObject_CallAttr:argv[0] = shared_vector */
	call    fSYM(DeeObject_CallAttr,16)
	ADJUST_CFA_OFFSET(-16)
	testl   %eax, %eax
	jz      99f
	xchgl   %eax, TOP     /* Save the result */
	decref  %eax
	call    fSYM(DeeSharedVector_Decref,4) /* decref() the shared vector. */
	ADJUST_CFA_OFFSET(-4)
	jmp    .disp
	ADJUST_CFA_OFFSET(4)
99:	call    fSYM(DeeSharedVector_Decref,4) /* decref() the shared vector. */
	ADJUST_CFA_OFFSET(-4)
	jmp    .except


target16_callattr_c_map:
	lodsw
	jmp     1f
target_callattr_c_map:
	lodsb
1:	subl    $4, %esp      /* DeeObject_CallAttr:argv[0] */
	ADJUST_CFA_OFFSET(4)
	pushl   %esp          /* DeeObject_CallAttr:argv */
	ADJUST_CFA_OFFSET(4)
	pushl   $1            /* DeeObject_CallAttr:argc */
	ADJUST_CFA_OFFSET(4)
	LOAD_CODE(%ecx)
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
	movl    co_staticv(CODE_OR_ECX), %ecx
	pushl   (%ecx,%eax,4) /* DeeObject_CallAttr:attr_name */
	ADJUST_CFA_OFFSET(4)
	lodsb                 /* EAX = argc */
	movl    SP, %edx
	leal    (,%eax,8), %ecx
	subl    %ecx, %edx    /* EDX = new_sp = SP - argc * 2 */
	pushl   -4(%edx)      /* DeeObject_CallAttr:self */
	ADJUST_CFA_OFFSET(4)
	pushl   %edx          /* BACKUP(new_sp) */
	ADJUST_CFA_OFFSET(4)
	pushl   %edx          /* DeeSharedMap_NewShared:vector */
	ADJUST_CFA_OFFSET(4)
	pushl   %eax          /* DeeSharedMap_NewShared:length */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeSharedMap_NewShared,8)
	ADJUST_CFA_OFFSET(-8)
	testl   %eax, %eax
	jz     .except_x24
	popl    SP            /* SP = new_sp */
	ADJUST_CFA_OFFSET(-4)
	movl    %eax, 16(%esp) /* DeeObject_CallAttr:argv[0] = shared_vector */
	call    fSYM(DeeObject_CallAttr,16)
	ADJUST_CFA_OFFSET(-16)
	testl   %eax, %eax
	jz      99f
	xchgl   %eax, TOP     /* Save the result */
	decref  %eax
	call    fSYM(DeeSharedMap_Decref,4) /* decref() the shared map. */
	ADJUST_CFA_OFFSET(-4)
	jmp    .disp
	ADJUST_CFA_OFFSET(4)
99:	call    fSYM(DeeSharedMap_Decref,4) /* decref() the shared map. */
	ADJUST_CFA_OFFSET(-4)
	jmp    .except


target16_callattr_c_kw:
	lodsw
	movzwl  %ax, %edx
	lodsb
	movzbl  %al, %ecx
	lodsw
	jmp    1f
target_callattr_c_kw:
	lodsb
	movzbl  %al, %edx      /* EDX = ATTR_NAME_CID */
	lodsb
	movzbl  %al, %ecx      /* ECX = ARGC */
	lodsb
1:	subl    $24, %esp      /* Reserve argument memory. */
	ADJUST_CFA_OFFSET(24)
	movl    %ecx, 4(%esp)  /* DeeObject_CallAttrKw:argc */
	movl    %ecx, 20(%esp) /* BACKUP(argc) */
	LOAD_CODE(%ecx)
	ASSERT_BELOW(%dx,co_staticc(CODE_OR_ECX))
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
	movl    co_staticv(CODE_OR_ECX), %ecx
	movl    (%ecx,%edx,4), %edx
	movl    (%ecx,%eax,4), %eax
	movl    %edx, 0(%esp)  /* DeeObject_CallAttrKw:attr_name */
	movl    %eax, 12(%esp) /* DeeObject_CallAttrKw:kw */
	movl    SP, %eax
	movl    4(%esp), %ecx  /* argc */
	leal    (,%ecx,4), %ecx
	subl    %ecx, %eax     /* EAX = new_sp = SP - argc */
	movl    %eax, 8(%esp)  /* DeeObject_CallAttrKw:argv */
	movl    %eax, 16(%esp) /* BACKUP(new_sp) */
	pushl    -4(%eax)      /* DeeObject_CallAttrKw:self */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeObject_CallAttrKw,20)
	ADJUST_CFA_OFFSET(-20)
	testl   %eax, %eax
	jz     .except_x8
	popl    %edx           /* EDX = new_sp */
	ADJUST_CFA_OFFSET(-4)
	xchgl   %eax, -4(%edx) /* Save the result on the stack. */
	decref  %eax
	popl    %ecx           /* ECX = argc */
	ADJUST_CFA_OFFSET(-4)
	/* Drop references from arguments. */
	testl   %ecx, %ecx
	jz     .disp
1:	sp_popref    %eax, 0, 1, 0
	loop    1b
	jmp    .disp



target16_callattr_c_tuple_kw:
	lodsw
	movzwl  %ax, %edx
	lodsw
	jmp     1f
target_callattr_c_tuple_kw:
	lodsb
	movzbl  %al, %edx
	lodsb
1:	LOAD_CODE(%ecx)
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
	ASSERT_BELOW(%dx,co_staticc(CODE_OR_ECX))
	movl    co_staticv(CODE_OR_ECX), %ecx
	movl    (%ecx,%edx,4), %edx /* edx = STATICV[attr_name_cid] */
	pushl   (%ecx,%eax,4)      /* DeeObject_CallAttrTupleKw:kw */
	ADJUST_CFA_OFFSET(4)
	pushl   FIRST              /* DeeObject_CallAttrTupleKw:args */
	ADJUST_CFA_OFFSET(4)
	pushl   %edx               /* DeeObject_CallAttrTupleKw:attr_name */
	ADJUST_CFA_OFFSET(4)
	pushl   SECOND             /* DeeObject_CallAttrTupleKw:self */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeObject_CallAttrTupleKw,16)
	ADJUST_CFA_OFFSET(-16)
	testl   %eax, %eax
	jz     .except
	xchgl   %eax, SECOND
	decref  %eax
	sp_popref %eax
	jmp    .disp


target_callattr_kwds:
	lodsb
	movl    SP, %edx
	leal    4(,%eax,4), %ecx
	subl    %ecx, %edx         /* EDX = SP - (1 + argc) */
	pushl   %eax               /* BACKUP(argc) */
	ADJUST_CFA_OFFSET(4)
	pushl   %edx               /* BACKUP(new_sp) */
	ADJUST_CFA_OFFSET(4)
	pushl   TOP                /* DeeObject_CallAttrKw:kw */
	ADJUST_CFA_OFFSET(4)
	pushl   %edx               /* DeeObject_CallAttrKw:argv */
	ADJUST_CFA_OFFSET(4)
	pushl   %eax               /* DeeObject_CallAttrKw:argc */
	ADJUST_CFA_OFFSET(4)
	movl    -4(%edx), %ecx
	pushl   %ecx               /* DeeObject_CallAttrKw:attr_name */
	ADJUST_CFA_OFFSET(4)
	cmpl    $(SYM(DeeString_Type)), ob_type(%ecx)
	jne     99f
	pushl   -8(%edx)           /* DeeObject_CallAttrKw:self */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeObject_CallAttrKw,20)
	ADJUST_CFA_OFFSET(-20)
	testl   %eax, %eax
	jz     .except_x8
	popl    %edx               /* EDX = new_sp */
	ADJUST_CFA_OFFSET(-4)
	xchgl   %eax, -8(%edx)     /* Save the result */
	decref  %eax
	popl    %ecx               /* ECX = argc */
	ADJUST_CFA_OFFSET(-4)
	/* Decref function arguments. */
1:	sp_popref %eax, 0, 1, 0
	loop    1b
	sp_popref %eax             /* attr_name */
	jmp    .disp
	ADJUST_CFA_OFFSET(24)
99:	call    fSYM(err_expected_string_for_attribute,4)
	ADJUST_CFA_OFFSET(-4)
	jmp    .except_x20
	ADJUST_CFA_OFFSET(-20)



target_callattr_tuple_kwds:
	pushl   FIRST              /* DeeObject_CallAttrTupleKw:kw */
	ADJUST_CFA_OFFSET(4)
	pushl   SECOND             /* DeeObject_CallAttrTupleKw:args */
	ADJUST_CFA_OFFSET(4)
	movl    THIRD, %edx
	pushl   %edx               /* DeeObject_CallAttrTupleKw:attr_name */
	ADJUST_CFA_OFFSET(4)
	cmpl    $(SYM(DeeString_Type)), ob_type(%edx)
	jne     99f
	pushl   FOURTH             /* DeeObject_CallAttrTupleKw:self */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeObject_CallAttrTupleKw,16)
	ADJUST_CFA_OFFSET(-16)
	testl   %eax, %eax
	jz     .except
	xchgl   %eax, FOURTH       /* Save the result */
	decref  %eax
	sp_popref %eax
	sp_popref %eax
	sp_popref %eax
	jmp    .disp
	ADJUST_CFA_OFFSET(16)
99:	call    fSYM(err_expected_string_for_attribute,4)
	ADJUST_CFA_OFFSET(-4)
	jmp    .except_x12
	ADJUST_CFA_OFFSET(-12)



/* Call-to-symbol instructions. */
target16_call_extern:
	lodsw
	LOAD_CODE(%ecx)
	movl   co_module(CODE_OR_ECX), %ecx
	ASSERT_BELOW(%ax,mo_importc(%ecx))
	movl   mo_importv(%ecx), %ecx
	movl   (%ecx,%eax,4), %ecx
	lodsw
	jmp    1f
target_call_extern:
	lodsb
	LOAD_CODE(%ecx)
	movl   co_module(CODE_OR_ECX), %ecx
	ASSERT_BELOW(%ax,mo_importc(%ecx))
	movl   mo_importv(%ecx), %ecx
	movl   (%ecx,%eax,4), %ecx
	lodsb
1:	ASSERT_BELOW(%ax,mo_globalc(%ecx))
#ifndef CONFIG_NO_THREADS
	pushl  %eax
	ADJUST_CFA_OFFSET(4)
	rwlock_read mo_lock(%ecx), %edx
	popl   %eax
	ADJUST_CFA_OFFSET(-4)
#endif
	movl   mo_globalv(%ecx), %edx
	movl   (%edx,%eax,4), %edx
	testl  %edx, %edx
	jz     99f  /* Check if the global variable is unbound. */
	incref %edx /* Always safe a reference because the function may delete itself. */
#ifndef CONFIG_NO_THREADS
	rwlock_endread mo_lock(%ecx)
#endif
	pushl  %edx /* BACKUP(FUNCTION). */
	ADJUST_CFA_OFFSET(4)
	xorl   %eax, %eax
	lodsb
	movl   SP, %edx
	leal   (,%eax,4), %ecx
	subl   %ecx, %edx /* EDX = SP - n_args */
	pushl  %eax     /* BACKUP(n_args) */
	ADJUST_CFA_OFFSET(4)
	pushl  %edx     /* SP - n_args */
	ADJUST_CFA_OFFSET(4)
	pushl  %eax     /* n_args */
	ADJUST_CFA_OFFSET(4)
	pushl  12(%esp) /* FUNCTION */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_Call,12)
	ADJUST_CFA_OFFSET(-12)
	popl   %ecx     /* ECX = n_args */
	ADJUST_CFA_OFFSET(-4)
	testl  %eax, %eax
	jz    .except_decref_pop
	testl  %ecx, %ecx
	jz     3f       /* Pop arguments from the stack. */
2:	sp_popref %edx, 1, 1, 0
	loop   2b
3:	sp_push %eax    /* Push the callback result onto the stack. */
	popl   %eax     /* decref() the function that was called. */
	ADJUST_CFA_OFFSET(-4)
	decref %eax
	jmp   .disp
target16_call_global:
	lodsw
	LOAD_CODE(%ecx)
	movl   co_module(CODE_OR_ECX), %ecx
	jmp    1b
target_call_global:
	lodsb
	LOAD_CODE(%ecx)
	movl   co_module(CODE_OR_ECX), %ecx
	jmp    1b
99:
#ifndef CONFIG_NO_THREADS
	rwlock_endread mo_lock(%ecx)
#endif
	pushl  %eax /* GID */
	ADJUST_CFA_OFFSET(4)
	pushl  %ecx /* MODULE */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(err_unbound_global,8)
	ADJUST_CFA_OFFSET(-4)
	jmp   .except


/* Local function calls use their own code because here we don't
 * need to keep a reference to the function being called. */
target16_call_local:
	lodsw
	jmp    1f
target_call_local:
	lodsb
1:
#ifndef NDEBUG
	LOAD_CODE(%ecx)
	ASSERT_BELOW(%ax,co_localc(CODE_OR_ECX))
#endif
	movl   FRAME_FRAME, %ecx
	movl   (%ecx,%eax,4), %edx
	testl  %edx, %edx
	jz     99f /* Check if the function is unbound. */
	xorl   %eax, %eax
	lodsb  /* EAX = n_args */
	leal   (,%eax,4), %ecx
	negl   %ecx
	addl   SP, %ecx /* ECX = SP - n_args */
	pushl  %eax     /* BACKUP(n_args) */
	ADJUST_CFA_OFFSET(4)
	pushl  %ecx     /* SP - n_args */
	ADJUST_CFA_OFFSET(4)
	pushl  %eax     /* n_args */
	ADJUST_CFA_OFFSET(4)
	pushl  %edx     /* FUNCTION */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_Call,12)
	ADJUST_CFA_OFFSET(-12)
	popl   %ecx     /* ECX = n_args */
	ADJUST_CFA_OFFSET(-4)
	testl  %eax, %eax
	jz    .except
	testl  %ecx, %ecx
	jz     3f       /* Pop arguments from the stack. */
2:	sp_popref %edx, 1, 1, 0
	loop   2b
3:	sp_push %eax    /* Push the callback result onto the stack. */
	jmp   .disp
99:	pushl  %eax        /* LID */
	ADJUST_CFA_OFFSET(4)
	pushl  FRAME_PC    /* PC */
	ADJUST_CFA_OFFSET(4)
	pushl  CODE_OR_LOC /* CODE */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(err_unbound_local,12)
	ADJUST_CFA_OFFSET(-12)
	jmp   .except



target_getattr:
	movl   FIRST, %eax
	pushl  %eax
	ADJUST_CFA_OFFSET(4)
	cmpl   $(SYM(DeeString_Type)), ob_type(%eax)
	jne    99f /* Check if the first operand really is a string. */
	pushl  SECOND /* Operator base object. */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_GetAttr,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jz    .except /* Check for errors. */
	xchgl  SECOND, %eax /* Store the result onto the stack. */
	decref %eax
	sp_popref %eax
	jmp   .disp
	ADJUST_CFA_OFFSET(4)
99:	call   fSYM(err_expected_string_for_attribute,4)
	ADJUST_CFA_OFFSET(-4)
	jmp   .except
target_boundattr:
	movl   FIRST, %eax
	cmpl   $(SYM(DeeString_Type)), ob_type(%eax)
	pushl  %eax
	ADJUST_CFA_OFFSET(4)
	jne    99b /* Check if the first operand really is a string. */
	pushl  SECOND /* Operator base object. */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_BoundAttr,8)
	ADJUST_CFA_OFFSET(-8)
	xorl   %ecx, %ecx /* Clear out ECX */
	cmpl   $-1, %eax
	je    .except /* Check for errors. */
	cmpl   $0, %eax
	setg   %cl /* CL = EAX > 0 */
	leal   SYM(Dee_FalseTrue)(,%ecx,8), %eax /* Convert to a boolean. */
	incref %eax /* Incref the boolean value. */
	xchgl  SECOND, %eax /* Store the result onto the stack. */
	decref %eax
	sp_popref %eax
	jmp   .disp
target_delattr:
	movl   FIRST, %eax
	cmpl   $(SYM(DeeString_Type)), ob_type(%eax)
	pushl  %eax
	ADJUST_CFA_OFFSET(4)
	jne    99b /* Check if the first operand really is a string. */
	pushl  SECOND /* Operator base object. */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_DelAttr,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jnz   .except /* Check for errors. */
	sp_popref %eax
	sp_popref %eax
	jmp   .disp



target_setattr:
	movl   SECOND, %eax
	cmpl   $(SYM(DeeString_Type)), ob_type(%eax)
	jne    99f /* Check if the second operand really is a string. */
	pushl  FIRST /* Value to-be assigned. */
	ADJUST_CFA_OFFSET(4)
	pushl  %eax
	ADJUST_CFA_OFFSET(4)
	pushl  THIRD /* Operator base object. */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_SetAttr,12)
	ADJUST_CFA_OFFSET(-12)
	testl  %eax, %eax
	jnz   .except /* Check for errors. */
	sp_popref %eax
	sp_popref %eax
	sp_popref %eax
	jmp   .disp
99:	pushl  %eax
	call   fSYM(err_expected_string_for_attribute,4)
	jmp   .except



target16_getattr_c:
	lodsw
	jmp    1f
target_getattr_c:
	lodsb
1:	LOAD_CODE(%ecx)
	movl   co_staticv(CODE_OR_ECX), %edx
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
	movl   (%edx,%eax,4), %edx
	WASSERT_EQUAL(l,ob_type(%edx),$(SYM(DeeString_Type)))
	pushl  %edx /* Attribute name. */
	ADJUST_CFA_OFFSET(4)
	pushl  TOP /* base object. */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_GetAttr,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jz    .except /* Check for errors. */
	xchgl  TOP, %eax /* Save the result onto the stack. */
	decref %eax
	jmp   .disp
target16_delattr_c:
	lodsw
	jmp    1f
target_delattr_c:
	lodsb
1:	LOAD_CODE(%ecx)
	movl   co_staticv(CODE_OR_ECX), %edx
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
	movl   (%edx,%eax,4), %edx
	WASSERT_EQUAL(l,ob_type(%edx),$(SYM(DeeString_Type)))
	pushl  %edx /* Attribute name. */
	ADJUST_CFA_OFFSET(4)
	pushl  TOP /* base object. */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_DelAttr,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jnz   .except /* Check for errors. */
	sp_popref %eax
	jmp   .disp
target16_setattr_c:
	lodsw
	jmp    1f
target_setattr_c:
	lodsb
1:	LOAD_CODE(%ecx)
	movl   co_staticv(CODE_OR_ECX), %edx
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
	movl   (%edx,%eax,4), %edx
	WASSERT_EQUAL(l,ob_type(%edx),$(SYM(DeeString_Type)))
	pushl  FIRST /* The value to-be assigned. */
	ADJUST_CFA_OFFSET(4)
	pushl  %edx /* Attribute name. */
	ADJUST_CFA_OFFSET(4)
	pushl  SECOND /* base object. */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_SetAttr,12)
	ADJUST_CFA_OFFSET(-12)
	testl  %eax, %eax
	jnz   .except /* Check for errors. */
	sp_popref %eax
	sp_popref %eax
	jmp   .disp



target16_getattr_this_c:
	lodsw
	jmp    1f
target_getattr_this_c:
	lodsb
1:	LOAD_CODE(%ecx)
	movl   co_staticv(CODE_OR_ECX), %edx
	ASSERT_BIT(w,$(CODE_FTHISCALL),co_flags(CODE_OR_ECX))
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
	movl   (%edx,%eax,4), %edx
	WASSERT_EQUAL(l,ob_type(%edx),$(SYM(DeeString_Type)))
	pushl  %edx /* Attribute name. */
	ADJUST_CFA_OFFSET(4)
	pushl  FRAME_THIS /* base object. */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_GetAttr,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jz    .except /* Check for errors. */
	sp_push %eax /* Push the result onto the stack. */
	jmp   .disp
target16_delattr_this_c:
	lodsw
	jmp    1f
target_delattr_this_c:
	lodsb
1:	LOAD_CODE(%ecx)
	movl   co_staticv(CODE_OR_ECX), %edx
	ASSERT_BIT(w,$(CODE_FTHISCALL),co_flags(CODE_OR_ECX))
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
	movl   (%edx,%eax,4), %edx
	WASSERT_EQUAL(l,ob_type(%edx),$(SYM(DeeString_Type)))
	pushl  %edx /* Attribute name. */
	ADJUST_CFA_OFFSET(4)
	pushl  FRAME_THIS /* base object. */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_DelAttr,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jz    .disp /* Check for errors. */
	jmp   .except
target16_setattr_this_c:
	lodsw
	jmp    1f
target_setattr_this_c:
	lodsb
1:	LOAD_CODE(%ecx)
	movl   co_staticv(CODE_OR_ECX), %edx
	ASSERT_BIT(w,$(CODE_FTHISCALL),co_flags(CODE_OR_ECX))
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
	movl   (%edx,%eax,4), %edx
	WASSERT_EQUAL(l,ob_type(%edx),$(SYM(DeeString_Type)))
	pushl  TOP /* Attribute value. */
	ADJUST_CFA_OFFSET(4)
	pushl  %edx /* Attribute name. */
	ADJUST_CFA_OFFSET(4)
	pushl  FRAME_THIS /* base object. */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_SetAttr,12)
	ADJUST_CFA_OFFSET(-12)
	testl  %eax, %eax
	jnz   .except /* Check for errors. */
	sp_popref %eax
	jmp   .disp


target16_supergetattr_this_rc:
	lodsw
	movl   %eax, %ecx
	lodsw
	jmp    1f
target_supergetattr_this_rc:
	lodsb
	movl   %eax, %ecx
	lodsb
1:	/* EAX: attr_cid */
	/* ECX: tp_this_rid */
	LOAD_CODE(%edx)
	ASSERT_BELOW(%cx,co_refc(CODE_OR_EDX))
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_EDX))
	ASSERT_BIT(w,$(CODE_FTHISCALL),co_flags(CODE_OR_EDX))
	movl   co_staticv(CODE_OR_EDX), %edx
	pushl (%edx,%eax,4)  /* DeeObject_TGetAttr:attr_name */
	ADJUST_CFA_OFFSET(4)
	movl   FRAME_FUNC, %edx
	movl   fo_refv(%edx,%ecx,4), %ecx /* ECX = REFimm */
	movl   FRAME_THIS, %edx           /* EDX = THIS */
	pushl  %edx          /* DeeObject_TGetAttr:self */
	ADJUST_CFA_OFFSET(4)
	pushl  %ecx          /* DeeObject_TGetAttr:tp_self */
	ADJUST_CFA_OFFSET(4)
	pushl  %ecx          /* DeeObject_AssertType:required_type */
	ADJUST_CFA_OFFSET(4)
	pushl  %edx          /* DeeObject_AssertType:self */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_AssertType,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jnz   .except_x12
	call   fSYM(DeeObject_TGetAttr,12)
	ADJUST_CFA_OFFSET(-12)
	testl  %eax, %eax
	jz    .except
	sp_push %eax
	jmp   .disp

target16_supercallattr_this_rc:
	lodsw
	movl   %eax, %ecx
	lodsw
	jmp    1f
target_supercallattr_this_rc:
	lodsb
	movl   %eax, %ecx
	lodsb
1:	/* EAX: attr_cid */
	/* ECX: tp_this_rid */
	subl   $12, %esp /* DeeObject_TCallAttr:argc, DeeObject_TCallAttr:argv, BACKUP(argc) */
	ADJUST_CFA_OFFSET(12)
	LOAD_CODE(%edx)
	ASSERT_BELOW(%cx,co_refc(CODE_OR_EDX))
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_EDX))
	ASSERT_BIT(w,$(CODE_FTHISCALL),co_flags(CODE_OR_EDX))
	movl   co_staticv(CODE_OR_EDX), %edx
	pushl (%edx,%eax,4)    /* DeeObject_TCallAttr:attr_name */
	ADJUST_CFA_OFFSET(4)
	lodsb                  /* EAX = argc; */
	movl   %eax, 4(%esp)   /* DeeObject_TCallAttr:argc */
	movl   %eax, 12(%esp)  /* BACKUP(argc) */
	movl   SP, %edx
	leal   (,%eax,4), %eax /* EAX = argc * 4 */
	subl   %eax, %edx      /* EDX = SP - argc */
	movl   %edx, 8(%esp)   /* DeeObject_TCallAttr:argv */
	movl   FRAME_FUNC, %edx
	movl   fo_refv(%edx,%ecx,4), %ecx /* ECX = REFimm */
	movl   FRAME_THIS, %edx           /* EDX = THIS */
	pushl  %edx            /* DeeObject_TCallAttr:self */
	ADJUST_CFA_OFFSET(4)
	pushl  %ecx            /* DeeObject_TCallAttr:tp_self */
	ADJUST_CFA_OFFSET(4)
	pushl  %ecx            /* DeeObject_AssertType:required_type */
	ADJUST_CFA_OFFSET(4)
	pushl  %edx            /* DeeObject_AssertType:self */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeObject_AssertType,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jnz   .except_x24
	call   fSYM(DeeObject_TCallAttr,20)
	ADJUST_CFA_OFFSET(-20)
	testl  %eax, %eax
	jz    .except_x4
	popl   %ecx
	ADJUST_CFA_OFFSET(-4)
	/* Cleanup arguments passed to callattr */
	testl  %ecx, %ecx
	jz     1f
2:	sp_popref %edx, 1, 1, 0
	loop   2b
1:	sp_push %eax
	jmp   .disp


target16_pack_hashset:
	lodsw
	jmp    1f
target_pack_hashset:
	lodsb
1:	leal   (,%eax,4), %ecx
	movl   SP, %edx
	subl   %ecx, %edx
	pushl  %edx /* backup */
	ADJUST_CFA_OFFSET(4)
	pushl  %edx
	ADJUST_CFA_OFFSET(4)
	pushl  %eax
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeHashSet_NewItemsInherited,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jz    .except_x4
	popl   SP /* Load the new SP */
	ADJUST_CFA_OFFSET(-4)
	sp_push %eax
	jmp   .disp



target16_pack_dict:
	lodsw
	jmp    1f
target_pack_dict:
	lodsb
1:	leal   (,%eax,8), %ecx
	movl   SP, %edx
	subl   %ecx, %edx
	pushl  %edx /* BACKUP(NEW_SP) */
	ADJUST_CFA_OFFSET(4)
	pushl  %edx /* DeeDict_NewKeyItemsInherited:key_items */
	ADJUST_CFA_OFFSET(4)
	pushl  %eax /* DeeDict_NewKeyItemsInherited:num_keyitems */
	ADJUST_CFA_OFFSET(4)
	call   fSYM(DeeDict_NewKeyItemsInherited,8)
	ADJUST_CFA_OFFSET(-8)
	testl  %eax, %eax
	jz    .except_x4
	popl   SP /* Load the new SP */
	ADJUST_CFA_OFFSET(-4)
	sp_push %eax
	jmp   .disp



/* Reserved instructions are implemented as invalid opcodes (for now) */
target_ud        = invop
target_reserved1 = invop
target_reserved2 = invop
target_reserved3 = invop
target_reserved4 = invop
target_reserved5 = invop
target_reserved6 = invop
target_reserved7 = invop

target_breakpoint:
	movl   SP, FRAME_SP /* Safe the correct stack-pointer. */
	pushl  FRAME
	ADJUST_CFA_OFFSET(4)
	call   fSYM(trigger_breakpoint,4) /* Trigger a breakpoint. */
	ADJUST_CFA_OFFSET(-4)
	/* _always_ load new SP/PC from the frame to ensure that we are in a
	 * consistent state before handling potential exception, or moving on.
	 * For an explaination of why we do it this way, look at the C implementation. */
	movl   FRAME_PC, PC
	movl   FRAME_SP, SP
	/* Check how we're to continue execution. */
	cmpl   $(TRIGGER_BREAKPOINT_MIN), %eax
	jl     do_breakpoint_except /* if (eax < TRIGGER_BREAKPOINT_MIN) ... */
	cmpl   $(TRIGGER_BREAKPOINT_MAX), %eax
	jg     do_breakpoint_continue /* if (eax > TRIGGER_BREAKPOINT_MAX) ... */
	jmp   *debug_break_jmptab-(TRIGGER_BREAKPOINT_MIN*4)(,%eax,4)
.data
debug_break_jmptab:
	.long do_breakpoint_except_exit
	.long do_breakpoint_except
	.long do_breakpoint_continue
	.long do_breakpoint_contsafe
	.long do_breakpoint_return
	.long do_breakpoint_exit
	.long do_breakpoint_exit_nofin
.text
do_breakpoint_except_exit:
	unset_result
	movl   $0, FRAME_RESULT
	jmp   .end_without_finally
do_breakpoint_except = .except
do_breakpoint_continue = .disp
do_breakpoint_contsafe:
	/* Unhook frame from the thread-local execution stack.
	 *  - As we're about to re-enter the same frame, we've got no
	 *    way of skipping the frame setup, meaning that this switch
	 *    may result in the frame missing for a tiny moment.
	 * >> But that should be ok... */
	movl   L_THIS_THREAD, %eax
	decw   t_execsz(%eax) /* --this_thread->t_execsz */
	movl   cf_prev(FRAME), %ecx
	movl   %ecx, t_exec(%eax) /* this_thread->t_exec = frame->cf_prev; */
	movl   $(CODE_FRAME_NOT_EXECUTING), cf_prev(FRAME) /* frame->cf_prev = CODE_FRAME_NOT_EXECUTING */
	/* Continue execution in safe-mode. */
	movl   FRAME, %ecx
	/* Indicate that the stack hasn't been allocated dynamically. */
	movl   $0, cf_stacksz(%ecx)
#ifdef CONFIG_HOST_WINDOWS
	call   @DeeCode_ExecFrameSafe@4
#else
	call   SYM(DeeCode_ExecFrameSafe)
#endif
	jmp   .end_nounhook
do_breakpoint_return:
	LOAD_CODE(%ecx)
	testl  $(CODE_FYIELDING), co_flags(CODE_OR_ECX)
	jz     1f
	cmpl   $0, FRAME_RESULT
	jne   .end_without_finally
	/* Replace a NULL-return value with ITER_DONE in yield-functions. */
	movl   $(ITER_DONE), FRAME_RESULT
	jmp   .end_without_finally
1:	cmpl   $0, FRAME_RESULT
	jne   .end_return
	/* Replace a NULL-return value with `none' in non-yield-functions. */
	movl   $(SYM(DeeNone_Singleton)), FRAME_RESULT
	incref_addr SYM(DeeNone_Singleton)
	jmp   .end_return
do_breakpoint_exit:
do_breakpoint_exit_nofin:
	LOAD_CODE(%ecx)
	testl  $(CODE_FYIELDING), co_flags(CODE_OR_ECX)
	jz     1f
	/* Force-return ITER_DONE */
	movl   FRAME_RESULT, %ecx
	decl   %ecx
	cmpl   $-2, %ecx
	jnb    3f
	/* Drop a previously set yield-result */
	incl   %ecx
	decref %ecx, 1, 0, 0 /* Preserve EAX */
3:	movl   $(ITER_DONE), FRAME_RESULT
	jmp    2f
1:	cmpl   $0, FRAME_RESULT
	jne    2f
	/* Return `none' when no return value has been set. */
	movl   $(SYM(DeeNone_Singleton)), FRAME_RESULT
	incref_addr SYM(DeeNone_Singleton)
2:	cmpl   $(TRIGGER_BREAKPOINT_EXIT_NOFIN), %eax
	je    .end_without_finally
	jmp   .end_return

/* Storage class prefix opcodes. */

prefix_invop = invop

target16_extern:
	addl  $4, PC /* Skip the prefix operand. */
	lodsb /* Load the instruction being prefixed. */
	jmp   *prefix_inst_table(,%eax,4)
target_extern:
target16_stack:
target16_static:
target16_global:
target16_local:
	addl  $2, PC /* Skip the prefix operand. */
	lodsb /* Load the instruction being prefixed. */
	jmp   *prefix_inst_table(,%eax,4)
target_stack:
target_static:
target_global:
target_local:
	incl  PC /* Skip the prefix operand. */
	lodsb /* Load the instruction being prefixed. */
	jmp   *prefix_inst_table(,%eax,4)

/* ============ Prefixed instruction targets ============ */

.macro define_inplace_math_op name, function
prefix_target_\name:
	call    get_prefix_object
	pushl   %edx
	ADJUST_CFA_OFFSET(4)
	pushl   TOP
	ADJUST_CFA_OFFSET(4)
	leal    4(%esp), %eax
	pushl   %eax
	ADJUST_CFA_OFFSET(4)
	call    \function
	ADJUST_CFA_OFFSET(-8)
	testl   %eax, %eax
	jnz    .except_decref_pop
	sp_popref %eax
	popl    %edx
	ADJUST_CFA_OFFSET(-4)
	jmp     set_prefix_object
.endm

define_inplace_math_op add, fSYM(DeeObject_InplaceAdd,8)
define_inplace_math_op sub, fSYM(DeeObject_InplaceSub,8)
define_inplace_math_op mul, fSYM(DeeObject_InplaceMul,8)
define_inplace_math_op div, fSYM(DeeObject_InplaceDiv,8)
define_inplace_math_op mod, fSYM(DeeObject_InplaceMod,8)
define_inplace_math_op shl, fSYM(DeeObject_InplaceShl,8)
define_inplace_math_op shr, fSYM(DeeObject_InplaceShr,8)
define_inplace_math_op and, fSYM(DeeObject_InplaceAnd,8)
define_inplace_math_op or,  fSYM(DeeObject_InplaceOr,8)
define_inplace_math_op xor, fSYM(DeeObject_InplaceXor,8)
define_inplace_math_op pow, fSYM(DeeObject_InplacePow,8)

.macro define_inplace_math_op_s8 symbol, function
\symbol:
	call    get_prefix_object
	pushl   %edx
	ADJUST_CFA_OFFSET(4)
	lodsb
	movsbl  %al, %eax
	pushl   %eax /* val */
	ADJUST_CFA_OFFSET(4)
	leal    4(%esp), %eax
	pushl   %eax /* &prefix_ob */
	ADJUST_CFA_OFFSET(4)
	call    \function
	ADJUST_CFA_OFFSET(-8)
	testl   %eax, %eax
	jnz    .except_decref_pop
	popl    %edx
	ADJUST_CFA_OFFSET(-4)
	jmp     set_prefix_object
.endm

.macro define_inplace_math_op_u32 symbol, function
\symbol:
	call    get_prefix_object
	pushl   %edx
	ADJUST_CFA_OFFSET(4)
	lodsl
	pushl   %eax /* val */
	ADJUST_CFA_OFFSET(4)
	leal    4(%esp), %eax
	pushl   %eax /* &prefix_ob */
	ADJUST_CFA_OFFSET(4)
	call    \function
	ADJUST_CFA_OFFSET(-8)
	testl   %eax, %eax
	jnz    .except_decref_pop
	popl    %edx
	ADJUST_CFA_OFFSET(-4)
	jmp     set_prefix_object
.endm

define_inplace_math_op_s8  prefix_target_add_simm8, fSYM(DeeObject_InplaceAddS8,8)
define_inplace_math_op_u32 prefix_target_add_imm32, fSYM(DeeObject_InplaceAddInt,8)
define_inplace_math_op_s8  prefix_target_sub_simm8, fSYM(DeeObject_InplaceSubS8,8)
define_inplace_math_op_u32 prefix_target_sub_imm32, fSYM(DeeObject_InplaceSubInt,8)
define_inplace_math_op_s8  prefix_target_mul_simm8, fSYM(DeeObject_InplaceMulInt,8)
define_inplace_math_op_s8  prefix_target_div_simm8, fSYM(DeeObject_InplaceDivInt,8)
define_inplace_math_op_s8  prefix_target_mod_simm8, fSYM(DeeObject_InplaceModInt,8)
define_inplace_math_op_u32 prefix_target_and_imm32, fSYM(DeeObject_InplaceAndInt,8)
define_inplace_math_op_u32 prefix_target_or_imm32,  fSYM(DeeObject_InplaceOrInt,8)
define_inplace_math_op_u32 prefix_target_xor_imm32, fSYM(DeeObject_InplaceXorInt,8)


prefix_target_extended1:
	/* Extended opcode with prefix. */
	lodsb
	jmpl   *prefix_inst_table_f0(,%eax,4)

/* Prefixed creation of function objects. */
prefix_target16_function_c_16:
	lodsw
	movl    %eax, %ecx
	lodsw
	jmp    .prefix_do_function_c
prefix_target16_function_c:
	lodsw
	movl    %eax, %ecx
	xorl    %eax, %eax
	lodsb
	jmp    .prefix_do_function_c
prefix_target_function_c_16:
	lodsb
	movl    %eax, %ecx
	lodsw
	jmp    .prefix_do_function_c
prefix_target_function_c:
	lodsb
	movl    %eax, %ecx
	lodsb
.prefix_do_function_c:
	LOAD_CODE(%edx)
	ASSERT_BELOW(%cx,co_staticc(CODE_OR_EDX))
	movl    co_staticv(CODE_OR_EDX), %edx
	incl    %eax /* EAX = refc */
	subl    $8, %esp
	ADJUST_CFA_OFFSET(4)
	pushl   %eax            /* DeeFunction_NewInherited:refc */
	ADJUST_CFA_OFFSET(4)
	pushl   (%edx,%ecx,4)   /* DeeFunction_NewInherited:code */
	ADJUST_CFA_OFFSET(4)
	movl    SP, %edx
	leal    (,%eax,4), %ecx
	subl    %ecx, %edx      /* EDX = SP - refc */
	movl    %edx, 8(%esp)   /* DeeFunction_NewInherited:refv */
	movl    %edx, 12(%esp)  /* BACKUP(SP - refc) */
#if 1
	movl 0(%esp), %eax
	WASSERT_EQUAL(l,ob_type(%eax),$SYM(DeeCode_Type))
#endif
	call    fSYM(DeeFunction_NewInherited,12)
	ADJUST_CFA_OFFSET(-12)
	testl   %eax, %eax
	jz     .except_x4
	popl    SP              /* SP = SP - refc */
	ADJUST_CFA_OFFSET(-4)
	movl    %eax, %edx
	jmp     set_prefix_object


prefix_target_inc:
	call    get_prefix_object
	pushl   %edx
	ADJUST_CFA_OFFSET(4)
.do_prefix_target_inc:
	pushl   %esp
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeObject_Inc,4)
	ADJUST_CFA_OFFSET(-4)
	popl    %edx
	ADJUST_CFA_OFFSET(-4)
	testl   %eax, %eax
	jz      set_prefix_object
	jmp    .except_decref_edx

prefix_target_dec:
	call    get_prefix_object
	pushl   %edx
	ADJUST_CFA_OFFSET(4)
.do_prefix_target_dec:
	pushl   %esp
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeObject_Dec,4)
	ADJUST_CFA_OFFSET(-4)
	popl    %edx
	ADJUST_CFA_OFFSET(-4)
	testl   %eax, %eax
	jz      set_prefix_object
	jmp    .except_decref_edx

prefix_target_incpost:
	call    get_prefix_object
	pushl   %edx /* Local storage of prefix object. */
	ADJUST_CFA_OFFSET(4)
	pushl   %edx
	ADJUST_CFA_OFFSET(4)
	/* First: create a copy of the object being prefixed. */
	call    fSYM(DeeObject_Copy,4)
	ADJUST_CFA_OFFSET(-4)
	testl   %eax, %eax
	jz     .except_decref_pop
	/* Then push that copy. */
	sp_push %eax
	/* Then do a regular inplace-inc() operation. */
	jmp    .do_prefix_target_inc
ADJUST_CFA_OFFSET(-4)

prefix_target_decpost:
	call    get_prefix_object
	pushl   %edx /* Local storage of prefix object. */
	ADJUST_CFA_OFFSET(4)
	pushl   %edx
	ADJUST_CFA_OFFSET(4)
	/* First: create a copy of the object being prefixed. */
	call    fSYM(DeeObject_Copy,4)
	ADJUST_CFA_OFFSET(-4)
	testl   %eax, %eax
	jz     .except_decref_pop
	/* Then push that copy. */
	sp_push %eax
	/* Then do a regular inplace-dec() operation. */
	jmp    .do_prefix_target_dec
ADJUST_CFA_OFFSET(-4)


	/* mov-extensions. */
prefix_target16_lrot:
	call    get_prefix_object
	xorl    %eax, %eax
	lodsw
	jmp     1f
prefix_target_lrot:
	call    get_prefix_object
	xorl    %eax, %eax
	lodsb
1:	pushl   %edx /* prefix_object */
	ADJUST_CFA_OFFSET(4)
	leal    4(,%eax,4), %eax /* EAX = (EAX + 1) * 4  (shift - 1) */
	movl    SP, %ecx
	subl    %eax, %ecx /* ECX = SP - (shift - 1) */
	pushl   -4(%ecx) /* drop_object */
	ADJUST_CFA_OFFSET(4)
	pushl   %eax /* memmove.num_bytes = (shift-1) * 4 */
	ADJUST_CFA_OFFSET(4)
	pushl   %ecx /* memmove.src = SP - (shift - 1) */
	ADJUST_CFA_OFFSET(4)
	subl    $4, %ecx
	pushl   %ecx /* memmove.dst = SP - shift */
	ADJUST_CFA_OFFSET(4)
	call    SYM(memmove)
	addl    $12, %esp
	ADJUST_CFA_OFFSET(-12)
	popl    %edx /* drop_object */
	ADJUST_CFA_OFFSET(-4)
	popl    TOP /* prefix_object */
	ADJUST_CFA_OFFSET(-4)
	jmp     set_prefix_object


prefix_target16_rrot:
	call    get_prefix_object
	xorl    %eax, %eax
	lodsw
	jmp     1f
prefix_target_rrot:
	call    get_prefix_object
	xorl    %eax, %eax
	lodsb
1:	pushl   %edx /* prefix_object */
	ADJUST_CFA_OFFSET(4)
	leal    4(,%eax,4), %eax /* EAX = (EAX + 1) * 4  (shift - 1) */
	movl    -4(SP), %ecx
	subl    %eax, %ecx /* ECX = SP - shift */
	pushl   %ecx /* sp - shift */
	pushl   TOP /* drop_object */
	ADJUST_CFA_OFFSET(4)
	pushl   %eax /* memmove.num_bytes = (shift - 1) * 4 */
	ADJUST_CFA_OFFSET(4)
	pushl   %ecx /* memmove.src = SP - shift */
	ADJUST_CFA_OFFSET(4)
	addl    $4, %ecx
	pushl   %ecx /* memmove.dst = SP - (shift - 1) */
	ADJUST_CFA_OFFSET(4)
	call    SYM(memmove)
	addl    $12, %esp
	ADJUST_CFA_OFFSET(-12)
	popl    %edx /* drop_object */
	ADJUST_CFA_OFFSET(-4)
	popl    %ecx /* sp - shift */
	ADJUST_CFA_OFFSET(-4)
	popl    (%ecx) /* *(sp - shift) = prefix_object */
	ADJUST_CFA_OFFSET(-4)
	jmp     set_prefix_object

prefix_target_swap:
	movl    TOP, %edx
	call    xch_prefix_object
	movl    %edx, TOP
	jmp    .disp

prefix_target16_pop_static:
	call    get_prefix_object
	xorl    %eax, %eax
	lodsw
	jmp     1f
prefix_target_pop_static:
	call    get_prefix_object
	xorl    %eax, %eax
	lodsb
1:	LOAD_CODE(%ecx)
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
#ifndef CONFIG_NO_THREADS
	pushl   %eax
	ADJUST_CFA_OFFSET(4)
#ifdef CODE_IS_REGISTER
	rwlock_write co_static_lock(CODE_OR_ECX), %ecx
#else
	pushl   %edx
	ADJUST_CFA_OFFSET(4)
	rwlock_write co_static_lock(CODE_OR_ECX), %edx
	popl    %edx
	ADJUST_CFA_OFFSET(-4)
#endif
	popl    %eax
	ADJUST_CFA_OFFSET(-4)
	LOAD_CODE(%ecx)
#endif
	movl    co_staticv(CODE_OR_ECX), %ecx
	xchgl  (%ecx,%eax,4), %edx /* Exchange the static variable. */
#ifndef CONFIG_NO_THREADS
	LOAD_CODE(%ecx)
	rwlock_endwrite co_static_lock(CODE_OR_ECX)
#endif
	ASSERT_NONZERO(%edx)
	decref  %edx
	jmp    .disp

prefix_target16_pop_extern:
	call    get_prefix_object
	xorl    %eax, %eax
	lodsw
	LOAD_CODE(%ecx)
	movl    co_module(CODE_OR_ECX), %ecx
	ASSERT_BELOW(%ax,mo_importc(%ecx))
	movl    mo_importv(%ecx), %ecx
	movl   (%ecx,%eax,4), %ecx
	lodsw
	jmp     2f
prefix_target_pop_extern:
	call    get_prefix_object
	xorl    %eax, %eax
	lodsb
	LOAD_CODE(%ecx)
	movl    co_module(CODE_OR_ECX), %ecx
	ASSERT_BELOW(%ax,mo_importc(%ecx))
	movl    mo_importv(%ecx), %ecx
	movl   (%ecx,%eax,4), %ecx
	lodsb
	jmp     2f

prefix_target16_pop_global:
	call    get_prefix_object
	xorl    %eax, %eax
	lodsw
	jmp     1f
prefix_target_pop_global:
	call    get_prefix_object
	xorl    %eax, %eax
	lodsb
1:	LOAD_CODE(%ecx)
	movl    co_module(CODE_OR_ECX), %ecx
2:	ASSERT_BELOW(%ax,mo_globalc(%ecx))
#ifndef CONFIG_NO_THREADS
	pushl   %edx
	ADJUST_CFA_OFFSET(4)
	pushl   %eax
	ADJUST_CFA_OFFSET(4)
	rwlock_write mo_lock(%ecx), %edx
	popl    %eax
	ADJUST_CFA_OFFSET(-4)
	popl    %edx
	ADJUST_CFA_OFFSET(-4)
	pushl   %ecx
	ADJUST_CFA_OFFSET(4)
#endif
	movl    mo_globalv(%ecx), %ecx
	xchgl   (%ecx,%eax,4), %edx
#ifndef CONFIG_NO_THREADS
	popl    %ecx
	ADJUST_CFA_OFFSET(-4)
	rwlock_endwrite mo_lock(%ecx)
#endif
	testl   %edx, %edx
	jz     .disp
	decref  %edx /* Drop a reference from the old global variable. */
	jmp    .disp


prefix_target16_pop_local:
	call    get_prefix_object
	xorl    %eax, %eax
	lodsw
	jmp     1f
prefix_target_pop_local:
	call    get_prefix_object
	xorl    %eax, %eax
	lodsb
1:	movl    FRAME_FRAME, %ecx
	xchgl   (%ecx,%eax,4), %edx
	testl   %edx, %edx
	jz     .disp
	decref  %edx
	jmp    .disp

prefix_target_push_simm8:
	/* PREFIX = $Simm8 */
	lodsb
	movsbl  %al, %eax
	pushl   %eax
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeInt_NewS32,4)
	ADJUST_CFA_OFFSET(-4)
	testl   %eax, %eax
	jz     .except
	movl    %eax, %edx
	jmp     set_prefix_object

prefix_target_push_imm32:
	/* PREFIX = $imm32 */
	lodsl
prefix_target_push_imm16:
	/* PREFIX = $imm16 */
	lodsw
1:	pushl   %eax
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeInt_NewU32,4)
	ADJUST_CFA_OFFSET(-4)
	testl   %eax, %eax
	jz     .except
	movl    %eax, %edx
	jmp     set_prefix_object

prefix_target_push_none:
	leal    SYM(DeeNone_Singleton), %edx
	incref  %edx
	jmp     set_prefix_object

prefix_target_push_except:
	movl    L_THIS_THREAD, %eax
	movl    t_except(%eax), %eax
	testl   %eax, %eax
	jz     .except_no_active_exception /* Check if an exception has been set. */
	movl    ef_error(%eax), %edx
	ASSERT_NONZERO(%edx)
	incref  %edx
	jmp     set_prefix_object

prefix_target16_push_module:
	lodsw
	jmp     1f
prefix_target_push_module:
	lodsb
1:	LOAD_CODE(%ecx)
	movl    co_module(CODE_OR_ECX), %ecx
	ASSERT_BELOW(%ax,mo_importc(%ecx))
	movl    mo_importv(%ecx), %edx
	movl    (%edx,%eax,4), %edx
	ASSERT_NONZERO(%edx)
	incref  %edx
	jmp     set_prefix_object

prefix_target_push_this:
#ifndef NDEBUG
	LOAD_CODE(%ecx)
	ASSERT_BIT(w,$(CODE_FTHISCALL),co_flags(CODE_OR_ECX))
#endif
	movl    FRAME_THIS, %edx
	incref  %edx
	jmp     set_prefix_object

prefix_target_push_this_module:
	LOAD_CODE(%eax)
	movl    co_module(CODE_OR_EAX), %edx
	incref  %edx
	jmp     set_prefix_object

prefix_target_push_this_function:
	movl    FRAME_FUNC, %edx
	incref  %edx
	jmp     set_prefix_object

prefix_target16_push_ref:
	lodsw
	jmp     1f
prefix_target_push_ref:
	lodsb
1:	movl    FRAME_FUNC, %ecx
	movl    fo_refv(%ecx,%eax,4), %edx
	incref  %edx
	jmp     set_prefix_object

prefix_target16_push_arg:
	lodsw
	jmp     1f
prefix_target_push_arg:
	lodsb
1:	cmpl    FRAME_ARGC, %eax
	jnb     1f
	/* Simple case: Direct argument/default reference. */
	movl    FRAME_ARGV, %edx
	movl    (%edx,%eax,4), %edx
	jmp     set_prefix_object_incref
	/* Check for the argument within the kwds argument overlay */
1:	movl    FRAME_KW, %edx
	testl   %edx, %edx
	jz      1f
	subl    FRAME_ARGC, %eax
	movl    fk_kargv(%edx,%eax,4), %edx
	testl   %edx, %edx
	jnz     set_prefix_object_incref
	addl    FRAME_ARGC, %eax
	/* Check for the argument within the code's default argument vector */
1:	LOAD_CODE(%ecx)
	subw    co_argc_min(CODE_OR_ECX), %ax
	movl    co_defaultv(CODE_OR_ECX), %edx
	movl    (%edx,%eax,4), %edx
	testl   %edx, %edx
	jnz     set_prefix_object_incref
1:	addw    co_argc_min(CODE_OR_ECX), %ax
	/* Unbound argument */
	pushl   %eax
	ADJUST_CFA_OFFSET(4)
	pushl   FRAME_PC
	ADJUST_CFA_OFFSET(4)
	pushl   CODE_OR_LOC
	ADJUST_CFA_OFFSET(4)
	call    fSYM(err_unbound_arg,12)
	ADJUST_CFA_OFFSET(-12)
	jmp    .except


prefix_target_push_varargs:
	movl    FRAME_VARARGS, %edx
	testl   %edx, %edx
	jnz     set_prefix_object_incref
	LOAD_CODE(%ecx)
	movl    FRAME_ARGC, %eax
	/* if (frame->cf_argc <= code->co_argc_max) */
	cmpl    co_argc_max(CODE_OR_ECX), %eax
	jnbe    1f
	/* { */
	leal    SYM(DeeTuple_Empty), %edx
	movl    %edx, FRAME_VARARGS
	incref_n %edx, 2
	jmp     set_prefix_object
	/* } */
1:	/* Must construct the varargs tuple. */
	movl    co_argc_max(CODE_OR_ECX), %ecx
	movl    FRAME_ARGV, %eax
	leal    (%eax,%ecx,4), %edx /* EDX = frame->cf_argv + code->co_argc_max */
	movl    FRAME_ARGC, %eax
	subl    %ecx, %eax          /* EAX = frame->cf_argc - code->co_argc_max */
	pushl   %edx
	ADJUST_CFA_OFFSET(4)
	pushl   %eax
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeTuple_NewVector,8)
	ADJUST_CFA_OFFSET(-8)
	testl   %eax, %eax
	jz     .except
	/* Store the generated varargs object. */
	movl    %eax, FRAME_VARARGS
	movl    %eax, %edx
	jmp     set_prefix_object_incref


prefix_target_push_varkwds:
	movl    FRAME_KW, %eax
	testl   %eax, %eax
	jz      70f
	movl    fk_varkwds(%eax), %edx
	testl   %edx, %edx
	jnz     set_prefix_object_incref
	/* construct_varkwds_mapping() { */
	movl    fk_kw(%eax), %edx
	/* if likely(DeeKwds_Check(kw)) */
	cmpl    $SYM(DeeKwds_Type), ob_type(%edx)
	jne     1f
	/* { */
	movl    FRAME_ARGC, %ecx
	movl    FRAME_ARGV, %eax
	leal    (%edx,%ecx,4), %ecx
	pushl   %ecx /* frame->cf_argv + frame->cf_argc */
	ADJUST_CFA_OFFSET(4)
	pushl   %edx          /* kw */
	ADJUST_CFA_OFFSET(4)
	pushl   FRAME_ARGC    /* positional_argc */
	ADJUST_CFA_OFFSET(4)
	pushl   CODE_OR_LOC   /* code */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(BlackListVarkwds_New,16)
	ADJUST_CFA_OFFSET(-16)
	jmp     60f
	/* } else { */
1:	pushl   %edx          /* kw */
	ADJUST_CFA_OFFSET(4)
	pushl   FRAME_ARGC    /* positional_argc */
	ADJUST_CFA_OFFSET(4)
	pushl   CODE_OR_LOC   /* code */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(BlackListMapping_New,12)
	ADJUST_CFA_OFFSET(-12)
	/* } */
60:	testl   %eax, %eax
	jz     .except
	/* Cache the newly generated varkwds object. */
	movl    FRAME_KW, %ecx
#ifdef CONFIG_NO_THREADS
	movl    fk_varkwds(%ecx), %edx
	testl   %edx, %edx
	jnz     61f
	movl    %eax, fk_varkwds(%ecx) /* Inherit reference */
	movl    %eax, %edx
	jmp     set_prefix_object_incref
61:	decref  %eax, 0, 0, 1          /* Re-use the old object */
	jmp     set_prefix_object_incref
#else /* CONFIG_NO_THREADS */
	movl    %eax, %edx
	xorl    %eax, %eax
	/* EAX = ATOMIC_CMPXCH_VAL(frame->cf_kw->fk_varkwds,NULL,EDX); */
	cmpxchgl %edx, fk_varkwds(%ecx)
	jz      set_prefix_object_incref /* if (EAX == NULL) ... */
	/* Another thread already loaded `EAX'. */
	pushl   %eax
	ADJUST_CFA_OFFSET(4)
	movl    ob_type(%edx), %ecx
	cmpl    $SYM(DeeKwdsMapping_Type), %ecx
	jne     1f
	/* Cleanup: DeeKwdsMapping_CheckExact() */
	pushl   %edx
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeKwdsMapping_Decref,4)
	ADJUST_CFA_OFFSET(-4)
	jmp     2f
1:	cmpl    $SYM(BlackListVarkwds_Type), %ecx
	jne     1f
	/* Cleanup: BlackListVarkwds_CheckExact() */
	pushl   %edx
	ADJUST_CFA_OFFSET(4)
	call    fSYM(BlackListVarkwds_Decref,4)
	ADJUST_CFA_OFFSET(-4)
	jmp     2f
	/* Cleanup: Dee_Decref() */
1:	decref  %edx
2:	popl    %edx
	ADJUST_CFA_OFFSET(-4)
	jmp     set_prefix_object_incref
#endif /* !CONFIG_NO_THREADS */
70:	leal    SYM(DeeMapping_EmptyInstance), %edx
	jmp     set_prefix_object_incref


#ifdef HAVE_TARGET_PUSH_CONST
prefix_target16_push_const:
#endif
prefix_target16_push_static:
	lodsw
	jmp     1f
#ifdef HAVE_TARGET_PUSH_CONST
prefix_target_push_const:
#endif
prefix_target_push_static:
	lodsb
1:	LOAD_CODE(%ecx)
	movl    co_staticv(CODE_OR_ECX), %edx
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
#ifndef CONFIG_NO_THREADS
	pushl   %eax
	ADJUST_CFA_OFFSET(4)
	rwlock_read co_static_lock(CODE_OR_ECX), %edx
	popl    %eax
	ADJUST_CFA_OFFSET(-4)
#endif
	movl   (%edx,%eax,4), %edx
	incref  %edx
#ifndef CONFIG_NO_THREADS
	rwlock_endread co_static_lock(CODE_OR_ECX)
#endif
	jmp     set_prefix_object



#ifndef HAVE_TARGET_PUSH_CONST
prefix_target16_push_const:
	lodsw
	jmp     1f
prefix_target_push_const:
	lodsb
1:	LOAD_CODE(%ecx)
	movl    co_staticv(CODE_OR_ECX), %edx
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
	movl   (%edx,%eax,4), %edx
	incref  %edx
	jmp     set_prefix_object
#endif


prefix_target16_push_extern:
	lodsw
	LOAD_CODE(%ecx)
	movl    co_module(CODE_OR_ECX), %ecx
	ASSERT_BELOW(%ax,mo_importc(%ecx))
	movl    mo_importv(%ecx), %ecx
	movl   (%ecx,%eax,4), %ecx
	lodsw
	jmp     2f
prefix_target_push_extern:
	lodsb
	LOAD_CODE(%ecx)
	movl    co_module(CODE_OR_ECX), %ecx
	ASSERT_BELOW(%ax,mo_importc(%ecx))
	movl    mo_importv(%ecx), %ecx
	movl   (%ecx,%eax,4), %ecx
	lodsb
	jmp     2f
prefix_target16_push_global:
	lodsw
	jmp     1f
prefix_target_push_global:
	lodsb
1:	LOAD_CODE(%ecx)
	movl    co_module(CODE_OR_ECX), %ecx
2:	ASSERT_BELOW(%ax,mo_globalc(%ecx))
#ifndef CONFIG_NO_THREADS
	pushl   %eax
	ADJUST_CFA_OFFSET(4)
	rwlock_write mo_lock(%ecx), %edx
	popl    %eax
	ADJUST_CFA_OFFSET(-4)
#endif
	movl    mo_globalv(%ecx), %edx
	movl   (%edx,%eax,4), %edx
	testl   %edx, %edx
	jz      99f /* Check for unbound globals. */
	incref  %edx
#ifndef CONFIG_NO_THREADS
	rwlock_endwrite mo_lock(%ecx)
#endif
	jmp     set_prefix_object
99:
#ifndef CONFIG_NO_THREADS
	rwlock_endwrite mo_lock(%ecx)
#endif
	pushl   %eax /* GID */
	ADJUST_CFA_OFFSET(4)
	pushl   %ecx /* MODULE */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(err_unbound_global,8)
	ADJUST_CFA_OFFSET(-8)
	jmp    .except


prefix_target16_push_local:
	lodsw
	jmp     1f
prefix_target_push_local:
	lodsb
1:	movl    FRAME_FRAME, %ecx
#ifndef NDEBUG
	LOAD_CODE(%edx)
	ASSERT_BELOW(%ax,co_localc(CODE_OR_EDX))
#endif
	movl   (%ecx,%eax,4), %edx
	testl   %edx, %edx
	jz      99f /* Check for unbound locals. */
	incref  %edx
	jmp     set_prefix_object
99:	pushl   %eax        /* LID */
	ADJUST_CFA_OFFSET(4)
	pushl   FRAME_PC /* PC */
	ADJUST_CFA_OFFSET(4)
	pushl   CODE_OR_LOC /* CODE */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(err_unbound_local,12)
	ADJUST_CFA_OFFSET(-12)
	jmp    .except


prefix_target_dup:
	movl    TOP, %edx
	incref  %edx
	jmp     set_prefix_object
prefix_target16_dup_n:
	lodsw
	jmp     1f
prefix_target_dup_n:
	lodsb
1:	/* dup #n loads an object offset by n+2:
	 * Why not +0 or +1?
	 *    +0: Would allow for illegal access to `SP-0',
	 *        which points to the next free SP slot and
	 *        doesn't contain a valid object
	 *    +1: Would lead to an ambiguity with `dup' if
	 *        duplicating the top-slot had two ways of
	 *        being encoded.
	 * >> Therefor, we add an offset of +2, preventing
	 *    redundancy as well as the possibility of an
	 *    illegal memory access. */
	leal    8(,%eax,4), %eax /* eax = (eax * 4) + 8 (aka. `eax = (eax + 2) * 4') */
	movl    SP, %edx
	subl    %eax, %edx
	ASSERT_ABOVE_EQUAL(%edx,FRAME_STACK) /* ERROR: Stack underflow. */
	ASSERT_BELOW(%edx,L_STACK_END) /* ERROR: Stack overflow. */
	movl   (%edx), %edx /* Dereference the requested stack object. */
	incref  %edx
	jmp     set_prefix_object

prefix_target_pop:
	call    get_prefix_object
	xchgl   TOP, %edx
	decref  %edx
	jmp    .disp

prefix_target16_pop_n:
	call    get_prefix_object
	xchgl   %eax, %eax
	lodsw
	jmp     1f
prefix_target_pop_n:
	call    get_prefix_object
	xchgl   %eax, %eax
	lodsb
1:	leal    8(,%eax,4), %eax /* eax = (eax * 4) + 8. (offset = (offset + 2) * 4) */
	movl    SP, %ecx
	subl    %eax, %ecx
	xchgl  (%ecx), %edx
	decref  %edx /* Drop the old object that was overwritten. */
	jmp    .disp

prefix_target_ret:
	/* Check if we're overwriting a previous return value
	 * (which can happen when `return' appears in a finally-block) */
	call    get_prefix_object
	unset_result %eax, 0, 0, 1
	movl    %edx, %eax
	movl    %eax, FRAME_RESULT
	TEST_CODE_FLAG($(CODE_FYIELDING),%ecx)
	jz     .end_return /* Execute finally handlers before returning. */
	jmp    .end_without_finally

prefix_target_yieldall:
	call    get_prefix_object
	ASSERT_YIELDING
	unset_result %eax, 0, 0, 1
	/* Yield the next item from the iterator. */
	pushl   %edx /* backup */
	ADJUST_CFA_OFFSET(4)
	pushl   %edx /* iterator */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeObject_IterNext,4)
	ADJUST_CFA_OFFSET(-4)
	popl    %edx /* restore */
	ADJUST_CFA_OFFSET(-4)
	decref  %edx, 1, 0, 0
	/* Check for errors. */
	testl   %eax, %eax
	jz     .except
	/* Check for iterator-exhausted. */
	cmpl    $(ITER_DONE), %eax
	je      1f
	repeat_instruction
	movl    %eax, FRAME_RESULT
	jmp    .end_without_finally
1:	sp_popref %eax
	jmp    .disp

prefix_target_throw:
	call    get_prefix_object
	pushl   %edx /* backup */
	ADJUST_CFA_OFFSET(4)
	pushl   %edx /* error */
	ADJUST_CFA_OFFSET(4)
	call    fSYM(DeeError_Throw,4)
	ADJUST_CFA_OFFSET(-4)
	popl    %edx /* restore */
	ADJUST_CFA_OFFSET(-4)
	decref  %edx
	jmp    .except


prefix_target_push_true:
	leal    SYM(Dee_FalseTrue)+8, %edx
	incref  %edx
	jmp     set_prefix_object
prefix_target_push_false:
	leal    SYM(Dee_FalseTrue)+0, %edx
	incref  %edx
	jmp     set_prefix_object



/* The reverse of `get_prefix_object':
 * Save the prefixed object back to member.
 * Jumping to this label will inherit a reference from `DeeObject *EDX'.
 * In all cases, this label should be jmp'd to and will
 * perform the following dispatch call itself, or still
 * inherit a reference from EDX and throw an error.
 * For a usage example, see the documentation for `get_prefix_object' above. */
set_prefix_object_incref:
	incref %edx
set_prefix_object:
	movl   FRAME_PC, %eax
	movzbl 0(%eax), %ecx
	ASSERT_ABOVE_EQUAL(%ecx,$0xf0)
	ASSERT_BELOW_EQUAL(%ecx,$0xff)
	/* Select the proper functionality for doing this. */
	jmp    *(impl_set_prefix_table-(0xf0*4))(,%ecx,4)
impl_set_prefix16_stack:
	movzwl 2(%eax), %eax
	jmp    1f
impl_set_prefix_stack:
	movzbl 1(%eax), %eax
1:	/* Stack variable prefix. */
	movl   FRAME_STACK, %ecx
#ifndef NDEBUG
	leal   (%ecx,%eax,4), %ecx
	ASSERT_BELOW(%ecx,SP) /* Stack address is too large. */
	ASSERT_ABOVE_EQUAL(%ecx,FRAME_STACK) /* Stack address overflows. */
	xchgl  (%ecx), %edx
#else
	xchgl  (%ecx,%eax,4), %edx
#endif
	decref  %edx /* Drop the value that got exchanged. */
	jmp    .disp
impl_set_prefix16_static:
	LOAD_CODE(%ecx)
#ifndef CONFIG_NO_THREADS
#ifdef CODE_IS_REGISTER
	rwlock_write co_static_lock(CODE), %ecx
#else
	pushl  %edx
	IF_USE_CFI(.cfi_adjust_cfa_offset 4)
	rwlock_write co_static_lock(CODE_OR_ECX), %edx
	popl   %edx
	IF_USE_CFI(.cfi_adjust_cfa_offset -4)
#endif
	movl   FRAME_PC, %eax
#endif
	movzwl 2(%eax), %eax
	jmp    1f
impl_set_prefix_static:
	/* Static variable prefix. */
	LOAD_CODE(%ecx)
#ifndef CONFIG_NO_THREADS
#ifdef CODE_IS_REGISTER
	rwlock_write co_static_lock(CODE), %ecx
#else
	pushl  %edx
	IF_USE_CFI(.cfi_adjust_cfa_offset 4)
	rwlock_write co_static_lock(CODE_OR_ECX), %edx
	popl   %edx
	IF_USE_CFI(.cfi_adjust_cfa_offset -4)
#endif
	movl   FRAME_PC, %eax
#endif
	movzbl 1(%eax), %eax
1:	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
	movl   co_staticv(CODE_OR_ECX), %ecx
	/* Exchange the static variable. */
	ASSERT_NONZERO(%edx)
	xchgl  (%ecx,%eax,4), %edx
	ASSERT_NONZERO(%edx)
#ifndef CONFIG_NO_THREADS
	LOAD_CODE(%ecx)
	rwlock_endwrite co_static_lock(CODE_OR_ECX)
#endif
	decref %edx /* Drop the value that got exchanged. */
	jmp   .disp
impl_set_prefix16_extern:
	LOAD_CODE(%ecx)
	movl   co_module(CODE_OR_ECX), %ecx
	movzwl 2(%eax), %eax
	ASSERT_BELOW(%ax,mo_importc(%ecx))
	movl   mo_importv(%ecx), %ecx
	movl   (%ecx,%eax,4), %ecx
	movl   FRAME_PC, %eax
	movzwl 4(%eax), %eax
	jmp    1f
impl_set_prefix_extern:
	LOAD_CODE(%ecx)
	movl   co_module(CODE_OR_ECX), %ecx
	movzbl 1(%eax), %eax
	ASSERT_BELOW(%ax,mo_importc(%ecx))
	movl   mo_importv(%ecx), %ecx
	movl   (%ecx,%eax,4), %ecx
	movl   FRAME_PC, %eax
	movzbl 2(%eax), %eax
	jmp    1f
impl_set_prefix16_global:
	movzwl 2(%eax), %eax
	jmp    2f
impl_set_prefix_global:
	movzbl 1(%eax), %eax
2:	LOAD_CODE(%ecx)
	movl   co_module(CODE_OR_ECX), %ecx
1:	ASSERT_BELOW(%ax,mo_globalc(%ecx))
	/* Prefix with an extern/global variable. */
#ifndef CONFIG_NO_THREADS
	pushl  %ecx /* Save the module for below. */
	IF_USE_CFI(.cfi_adjust_cfa_offset 4)
	pushl  %edx
	IF_USE_CFI(.cfi_adjust_cfa_offset 4)
	pushl  %eax
	IF_USE_CFI(.cfi_adjust_cfa_offset 4)
	rwlock_write mo_lock(%ecx), %edx
	popl   %eax
	IF_USE_CFI(.cfi_adjust_cfa_offset -4)
	popl   %edx
	IF_USE_CFI(.cfi_adjust_cfa_offset -4)
#endif
	movl   mo_globalv(%ecx), %ecx
	xchgl  (%ecx,%eax,4), %edx /* Exchange the global. */
#ifndef CONFIG_NO_THREADS
	popl   %ecx
	IF_USE_CFI(.cfi_adjust_cfa_offset -4)
	rwlock_endwrite mo_lock(%ecx)
#endif
	testl  %edx, %edx
	jz    .disp
	decref %edx /* Drop a reference from the old object. */
	jmp   .disp
impl_set_prefix16_local:
	movzwl 2(%eax), %eax
	jmp    1f
impl_set_prefix_local:
	movzbl 1(%eax), %eax
1:	/* Prefix with a local variable. */
#ifndef NDEBUG
	LOAD_CODE(%ecx)
	ASSERT_BELOW(%ax,co_localc(%ecx))
#endif
	movl   FRAME_FRAME, %ecx
	xchgl  (%ecx,%eax,4), %edx /* Exchange the prefixed object. */
	testl  %edx, %edx
	jz    .disp
	decref %edx /* Drop a reference from the old object. */
	jmp   .disp
impl_set_prefix_extended:
	movzbl 1(%eax), %ecx
	ASSERT_ABOVE_EQUAL(%ecx,$0xf8)
	ASSERT_BELOW_EQUAL(%ecx,$0xff)
	jmp    *(impl_set_prefix_table_f0-(0xf8*4))(,%ecx,4)
IF_USE_CFI(.cfi_endproc)

.section .rodata
impl_set_prefix_table:
	.long  impl_set_prefix_extended /* 0xf0 : ASM_EXTENDED1 */
	.long  invop                    /* 0xf1 : ASM_RESERVED1 */
	.long  invop                    /* 0xf2 : ASM_RESERVED2 */
	.long  invop                    /* 0xf3 : ASM_RESERVED3 */
	.long  invop                    /* 0xf4 : ASM_RESERVED4 */
	.long  invop                    /* 0xf5 : ASM_RESERVED5 */
	.long  invop                    /* 0xf6 : ASM_RESERVED6 */
	.long  invop                    /* 0xf7 : ASM_RESERVED7 */
	.long  invop                    /* 0xf8 : ??? */
	.long  invop                    /* 0xf9 : ??? */
	.long  invop                    /* 0xfa : ??? */
	.long  impl_set_prefix_stack    /* 0xfb : ASM_STACK */
	.long  impl_set_prefix_static   /* 0xfc : ASM_STATIC */
	.long  impl_set_prefix_extern   /* 0xfd : ASM_EXTERN */
	.long  impl_set_prefix_global   /* 0xfe : ASM_GLOBAL */
	.long  impl_set_prefix_local    /* 0xff : ASM_LOCAL */
END(impl_set_prefix_table)
impl_set_prefix_table_f0:
	.long  invop                    /* 0xf0f8 : ??? */
	.long  invop                    /* 0xf0f9 : ??? */
	.long  invop                    /* 0xf0fa : ??? */
	.long  impl_set_prefix16_stack  /* 0xf0fb : ASM16_STACK */
	.long  impl_set_prefix16_static /* 0xf0fc : ASM16_STATIC */
	.long  impl_set_prefix16_extern /* 0xf0fd : ASM16_EXTERN */
	.long  impl_set_prefix16_global /* 0xf0fe : ASM16_GLOBAL */
	.long  impl_set_prefix16_local  /* 0xf0ff : ASM16_LOCAL */
END(impl_set_prefix_table_f0)



/* Exchange the prefixed object with an object in EDX.
 * In the event that the prefixed object isn't bound, this label
 * does not return, and no reference is inherited from `EDX'
 * >> DREF DeeObject *__asm__("edx")
 * >> xch_prefix_object(DeeObject *newob __asm__("edx"));
 * The following registers may be clobbered:
 *    - EAX (working register)
 *    - ECX (working register)
 *    - EDX (input/output register)
 * This label should be called like a function:
 * >>prefix_target_swap:
 * >>    movl   TOP, %edx
 * >>    call   xch_prefix_object
 * >>    movl   %edx, TOP
 * >>    jmp   .disp
 */
.text
IF_USE_CFI(.cfi_startproc)
xch_prefix_object:
	movl   FRAME_PC, %eax
	movzbl 0(%eax), %ecx
	ASSERT_ABOVE_EQUAL(%ecx,$0xf0)
	ASSERT_BELOW_EQUAL(%ecx,$0xff)
	/* Select the proper functionality for doing this. */
	jmp    *(impl_xch_prefix_table-(0xf0*4))(,%ecx,4)

impl_xch_prefix16_stack:
	movzwl 2(%eax), %ecx
	jmp    1f
impl_xch_prefix_stack:
	movzbl 1(%eax), %ecx
1:	movl   FRAME_STACK, %eax
#ifndef NDEBUG
	leal   (%eax,%ecx,4), %eax
	ASSERT_BELOW(%eax,SP) /* Stack address is too large. */
	ASSERT_ABOVE_EQUAL(%eax,FRAME_STACK) /* Stack address overflows. */
	xchgl  (%eax), %edx   /* Swap objects */
#else
	xchgl  (%eax,%ecx,4), %edx /* Swap objects */
#endif
	ret

impl_xch_prefix16_static:
	LOAD_CODE(%ecx)
#ifndef CONFIG_NO_THREADS
#ifdef CODE_IS_REGISTER
	rwlock_write co_static_lock(CODE), %ecx
#else
	pushl  %edx
	IF_USE_CFI(.cfi_adjust_cfa_offset 4)
	rwlock_write co_static_lock(CODE_OR_ECX), %edx
	popl   %edx
	IF_USE_CFI(.cfi_adjust_cfa_offset -4)
#endif
	movl   FRAME_PC, %eax
#endif
	movzwl 2(%eax), %eax
	jmp    1f
impl_xch_prefix_static:
	/* Static variable prefix. */
	LOAD_CODE(%ecx)
#ifndef CONFIG_NO_THREADS
#ifdef CODE_IS_REGISTER
	rwlock_write co_static_lock(CODE), %ecx
#else
	pushl  %edx
	IF_USE_CFI(.cfi_adjust_cfa_offset 4)
	rwlock_write co_static_lock(CODE_OR_ECX), %edx
	popl   %edx
	IF_USE_CFI(.cfi_adjust_cfa_offset -4)
#endif
	movl   FRAME_PC, %eax
#endif
	movzbl 1(%eax), %eax
1:	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
	movl   co_staticv(CODE_OR_ECX), %ecx
	/* Exchange the static variable. */
	ASSERT_NONZERO(%edx)
	xchgl  (%ecx,%eax,4), %edx
	ASSERT_NONZERO(%edx)
#ifndef CONFIG_NO_THREADS
	LOAD_CODE(%ecx)
	rwlock_endwrite co_static_lock(CODE_OR_ECX)
#endif
	ret
impl_xch_prefix16_extern:
	LOAD_CODE(%ecx)
	movl   co_module(CODE_OR_ECX), %ecx
	movzwl 2(%eax), %eax
	ASSERT_BELOW(%ax,mo_importc(%ecx))
	movl   mo_importv(%ecx), %ecx
	movl   (%ecx,%eax,4), %ecx
	movl   FRAME_PC, %eax
	movzwl 4(%eax), %eax
	jmp    1f
impl_xch_prefix_extern:
	LOAD_CODE(%ecx)
	movl   co_module(CODE_OR_ECX), %ecx
	movzbl 1(%eax), %eax
	ASSERT_BELOW(%ax,mo_importc(%ecx))
	movl   mo_importv(%ecx), %ecx
	movl   (%ecx,%eax,4), %ecx
	movl   FRAME_PC, %eax
	movzbl 2(%eax), %eax
	jmp    1f
impl_xch_prefix16_global:
	movzwl 2(%eax), %eax
	jmp    2f
impl_xch_prefix_global:
	movzbl 1(%eax), %eax
2:	LOAD_CODE(%ecx)
	movl   co_module(CODE_OR_ECX), %ecx
1:	ASSERT_BELOW(%ax,mo_globalc(%ecx))
	/* Prefix with an extern/global variable. */
	pushl  %ecx /* Save the module for below. */
	IF_USE_CFI(.cfi_adjust_cfa_offset 4)
#ifndef CONFIG_NO_THREADS
	pushl  %edx
	IF_USE_CFI(.cfi_adjust_cfa_offset 4)
	pushl  %eax
	IF_USE_CFI(.cfi_adjust_cfa_offset 4)
	rwlock_write mo_lock(%ecx), %edx
	popl   %eax
	IF_USE_CFI(.cfi_adjust_cfa_offset -4)
	popl   %edx
	IF_USE_CFI(.cfi_adjust_cfa_offset -4)
#endif
	movl   mo_globalv(%ecx), %ecx
	xchgl  (%ecx,%eax,4), %edx /* Exchange the global. */
	testl  %edx, %edx
	jz     70f
	popl   %ecx /* ECX = module; */
	IF_USE_CFI(.cfi_adjust_cfa_offset -4)
#ifndef CONFIG_NO_THREADS
	rwlock_endwrite mo_lock(%ecx)
#endif
	ret
	IF_USE_CFI(.cfi_adjust_cfa_offset 4)
70:	xchgl  (%ecx,%eax,4), %edx /* Restore the NULL-entry. */
	popl   %ecx /* ECX = module; */
	IF_USE_CFI(.cfi_adjust_cfa_offset -4)
#ifndef CONFIG_NO_THREADS
	rwlock_endwrite mo_lock(%ecx)
#endif
	decref %edx, 1, 1
	/* err_unbound_global(module,imm_val); */
	pushl  %eax /* imm_val */
	IF_USE_CFI(.cfi_adjust_cfa_offset 4)
	pushl  %ecx /* module */
	IF_USE_CFI(.cfi_adjust_cfa_offset 4)
	call   fSYM(err_unbound_global,8)
	IF_USE_CFI(.cfi_adjust_cfa_offset -8)
	jmp   .except_x4

impl_xch_prefix16_local:
	movzwl 2(%eax), %eax
	jmp    1f
impl_xch_prefix_local:
	movzbl 1(%eax), %eax
1:	/* Prefix with a local variable. */
#ifndef NDEBUG
	LOAD_CODE(%ecx)
	ASSERT_BELOW(%ax,co_localc(%ecx))
#endif
	movl   FRAME_FRAME, %ecx
	xchgl  (%ecx,%eax,4), %edx /* Exchange the prefixed object. */
	testl  %edx, %edx
	jz     70f
	ret
70:	xchgl  (%ecx,%eax,4), %edx /* Restore the NULL-value. */
	decref %edx, 1, 1
	pushl  %eax     /* err_unbound_local:index */
	IF_USE_CFI(.cfi_adjust_cfa_offset 4)
	pushl  FRAME_PC /* err_unbound_local:ip */
	IF_USE_CFI(.cfi_adjust_cfa_offset 4)
	pushl  CODE_OR_LOC /* err_unbound_local:code */
	IF_USE_CFI(.cfi_adjust_cfa_offset 4)
	call   fSYM(err_unbound_local,12)
	IF_USE_CFI(.cfi_adjust_cfa_offset -12)
	jmp   .except_x4


impl_xch_prefix_extended:
	movzbl 1(%eax), %ecx
	ASSERT_ABOVE_EQUAL(%ecx,$0xf8)
	ASSERT_BELOW_EQUAL(%ecx,$0xff)
	jmp    *(impl_xch_prefix_table_f0-(0xf8*4))(,%ecx,4)
IF_USE_CFI(.cfi_endproc)

.section .rodata
impl_xch_prefix_table:
	.long  impl_xch_prefix_extended /* 0xf0 : ASM_EXTENDED1 */
	.long  invop                    /* 0xf1 : ASM_RESERVED1 */
	.long  invop                    /* 0xf2 : ASM_RESERVED2 */
	.long  invop                    /* 0xf3 : ASM_RESERVED3 */
	.long  invop                    /* 0xf4 : ASM_RESERVED4 */
	.long  invop                    /* 0xf5 : ASM_RESERVED5 */
	.long  invop                    /* 0xf6 : ASM_RESERVED6 */
	.long  invop                    /* 0xf7 : ASM_RESERVED7 */
	.long  invop                    /* 0xf8 : ??? */
	.long  invop                    /* 0xf9 : ??? */
	.long  invop                    /* 0xfa : ??? */
	.long  impl_xch_prefix_stack    /* 0xfb : ASM_STACK */
	.long  impl_xch_prefix_static   /* 0xfc : ASM_STATIC */
	.long  impl_xch_prefix_extern   /* 0xfd : ASM_EXTERN */
	.long  impl_xch_prefix_global   /* 0xfe : ASM_GLOBAL */
	.long  impl_xch_prefix_local    /* 0xff : ASM_LOCAL */
END(impl_xch_prefix_table)

.section .rodata
impl_xch_prefix_table_f0:
	.long  invop                    /* 0xf0f8 : ??? */
	.long  invop                    /* 0xf0f9 : ??? */
	.long  invop                    /* 0xf0fa : ??? */
	.long  impl_xch_prefix16_stack  /* 0xf0fb : ASM16_STACK */
	.long  impl_xch_prefix16_static /* 0xf0fc : ASM16_STATIC */
	.long  impl_xch_prefix16_extern /* 0xf0fd : ASM16_EXTERN */
	.long  impl_xch_prefix16_global /* 0xf0fe : ASM16_GLOBAL */
	.long  impl_xch_prefix16_local  /* 0xf0ff : ASM16_LOCAL */
END(impl_xch_prefix_table_f0)



/* Load the prefixed object and store a reference to it in EDX.
 * In the event that the prefixed object isn't bound, this label
 * does not return.
 * The following registers may be clobbered:
 *    - EAX (working register)
 *    - ECX (working register)
 *    - EDX (output register)
 * This label should be called like a function:
 * >>prefix_target_inc:
 * >>    call   get_prefix_object
 * >>    pushl  %edx
 * >>    pushl  %esp
 * >>    call   SYM(DeeObject_Inc@4)
 * >>    popl   %edx
 * >>    testl  %eax, %eax
 * >>    jnz   .except_decref_edx
 * >>    jmp    set_prefix_object // Will jump to `.disp' upon success.
 */
.text
IF_USE_CFI(.cfi_startproc)
get_prefix_object:
	movl   FRAME_PC, %eax
	movzbl 0(%eax), %ecx
	ASSERT_ABOVE_EQUAL(%ecx,$0xf0)
	ASSERT_BELOW_EQUAL(%ecx,$0xff)
	/* Select the proper functionality for doing this. */
	jmp    *(impl_get_prefix_table-(0xf0*4))(,%ecx,4)
impl_get_prefix16_stack:
	movzwl 2(%eax), %ecx
	jmp    1f
impl_get_prefix_stack:
	movzbl 1(%eax), %ecx
1:	/* Stack variable prefix. */
	movl   FRAME_STACK, %edx
#ifndef NDEBUG
	leal   (%edx,%ecx,4), %edx
	ASSERT_BELOW(%edx,SP) /* Stack address is too large. */
	ASSERT_ABOVE_EQUAL(%edx,FRAME_STACK) /* Stack address overflows. */
	movl   (%edx), %edx
#else
	movl   (%edx,%ecx,4), %edx
#endif
	incref  %edx
	ret
impl_get_prefix16_static:
	movzwl 2(%eax), %eax
	jmp    1f
impl_get_prefix_static:
	movzbl 1(%eax), %eax
1:	/* Static variable prefix. */
	LOAD_CODE(%ecx)
#ifndef CONFIG_NO_THREADS
	pushl  %eax
	IF_USE_CFI(.cfi_adjust_cfa_offset 4)
	rwlock_read co_static_lock(CODE_OR_ECX), %edx
	popl   %eax
	IF_USE_CFI(.cfi_adjust_cfa_offset -4)
#endif
	ASSERT_BELOW(%ax,co_staticc(CODE_OR_ECX))
	movl   co_staticv(CODE_OR_ECX), %edx
	movl   (%edx,%eax,4), %edx
	ASSERT_NONZERO(%edx)
	incref %edx /* Save a reference in EDX */
#ifndef CONFIG_NO_THREADS
	rwlock_endread co_static_lock(CODE_OR_ECX)
#endif
	ret /* Return to the caller. */
impl_get_prefix16_extern:
	LOAD_CODE(%ecx)
	movl   co_module(CODE_OR_ECX), %ecx
	movzwl 2(%eax), %edx
	ASSERT_BELOW(%dx,mo_importc(%ecx))
	movl   mo_importv(%ecx), %ecx
	movl   (%ecx,%edx,4), %ecx
	movzwl 4(%eax), %eax
	jmp    1f
impl_get_prefix_extern:
	LOAD_CODE(%ecx)
	movl   co_module(CODE_OR_ECX), %ecx
	movzbl 1(%eax), %edx
	ASSERT_BELOW(%dx,mo_importc(%ecx))
	movl   mo_importv(%ecx), %ecx
	movl   (%ecx,%edx,4), %ecx
	movzbl 2(%eax), %eax
	jmp    1f
impl_get_prefix16_global:
	movzwl 2(%eax), %eax
	jmp    2f
impl_get_prefix_global:
	movzbl 1(%eax), %eax
2:	LOAD_CODE(%ecx)
	movl   co_module(CODE_OR_ECX), %ecx
1:	ASSERT_BELOW(%ax,mo_globalc(%ecx))
	/* Prefix with an extern/global variable. */
#ifndef CONFIG_NO_THREADS
	pushl  %eax
	IF_USE_CFI(.cfi_adjust_cfa_offset 4)
	rwlock_read mo_lock(%ecx), %edx
	popl   %eax
	IF_USE_CFI(.cfi_adjust_cfa_offset -4)
#endif
	movl   mo_globalv(%ecx), %edx
	movl   (%edx,%eax,4), %edx
	testl  %edx, %edx
	jz     99f /* Unbound global. */
	incref %edx
#ifndef CONFIG_NO_THREADS
	rwlock_endread mo_lock(%ecx)
#endif
	ret /* Return to the caller. */
99:
#ifndef CONFIG_NO_THREADS
	rwlock_endread mo_lock(%ecx)
#endif
	pushl  %eax /* GID */
	IF_USE_CFI(.cfi_adjust_cfa_offset 4)
	pushl  %ecx /* MODULE */
	IF_USE_CFI(.cfi_adjust_cfa_offset 4)
	call   fSYM(err_unbound_global,8)
	IF_USE_CFI(.cfi_adjust_cfa_offset -8)
	jmp   .except_x4 /* Pop the return address of `get_prefix_object' */
impl_get_prefix16_local:
	movzwl 2(%eax), %eax
	jmp    1f
impl_get_prefix_local:
	movzbl 1(%eax), %eax
1:	/* Prefix with a local variable. */
#ifndef NDEBUG
	LOAD_CODE(%ecx)
	ASSERT_BELOW(%ax,co_localc(%ecx))
#endif
	movl   FRAME_FRAME, %edx
	movl   (%edx,%eax,4), %edx
	testl  %edx, %edx
	jz     99f
	incref %edx
	ret /* Return to the caller. */
99:	pushl  %eax        /* LID */
	IF_USE_CFI(.cfi_adjust_cfa_offset 4)
	pushl  FRAME_PC    /* PC */
	IF_USE_CFI(.cfi_adjust_cfa_offset 4)
	pushl  CODE_OR_LOC /* CODE */
	IF_USE_CFI(.cfi_adjust_cfa_offset 4)
	call   fSYM(err_unbound_local,12)
	IF_USE_CFI(.cfi_adjust_cfa_offset -12)
	jmp   .except_x4
impl_get_prefix_extended:
	movzbl 1(%eax), %ecx
	ASSERT_ABOVE_EQUAL(%ecx,$0xf8)
	ASSERT_BELOW_EQUAL(%ecx,$0xff)
	jmp    *(impl_get_prefix_table_f0-(0xf8*4))(,%ecx,4)
IF_USE_CFI(.cfi_endproc)

.section .rodata
impl_get_prefix_table:
	.long  impl_get_prefix_extended /* 0xf0 : ASM_EXTENDED1 */
	.long  invop                    /* 0xf1 : ASM_RESERVED1 */
	.long  invop                    /* 0xf2 : ASM_RESERVED2 */
	.long  invop                    /* 0xf3 : ASM_RESERVED3 */
	.long  invop                    /* 0xf4 : ASM_RESERVED4 */
	.long  invop                    /* 0xf5 : ASM_RESERVED5 */
	.long  invop                    /* 0xf6 : ASM_RESERVED6 */
	.long  invop                    /* 0xf7 : ASM_RESERVED7 */
	.long  invop                    /* 0xf8 : ??? */
	.long  invop                    /* 0xf9 : ??? */
	.long  invop                    /* 0xfa : ??? */
	.long  impl_get_prefix_stack    /* 0xfb : ASM_STACK */
	.long  impl_get_prefix_static   /* 0xfc : ASM_STATIC */
	.long  impl_get_prefix_extern   /* 0xfd : ASM_EXTERN */
	.long  impl_get_prefix_global   /* 0xfe : ASM_GLOBAL */
	.long  impl_get_prefix_local    /* 0xff : ASM_LOCAL */
END(impl_get_prefix_table)

.section .rodata
impl_get_prefix_table_f0:
	.long  invop                    /* 0xf0f8 : ??? */
	.long  invop                    /* 0xf0f9 : ??? */
	.long  invop                    /* 0xf0fa : ??? */
	.long  impl_get_prefix16_stack  /* 0xf0fb : ASM16_STACK */
	.long  impl_get_prefix16_static /* 0xf0fc : ASM16_STATIC */
	.long  impl_get_prefix16_extern /* 0xf0fd : ASM16_EXTERN */
	.long  impl_get_prefix16_global /* 0xf0fe : ASM16_GLOBAL */
	.long  impl_get_prefix16_local  /* 0xf0ff : ASM16_LOCAL */
END(impl_get_prefix_table_f0)





.section .rodata
/*[[[deemon
#include <file>
#include <util>
#include <fs>
local codes    = list([none] * 256);
local codes_f0 = list([none] * 256);
fs::chdir(fs::path::head(__FILE__));
local longest_name = 0;

for (local l: file.open("../../../../include/deemon/asm.h")) {
	local name,code,misc;
	try name,code,none,misc = l.scanf(" # define ASM%[^ ] 0x%[0-9a-fA-F] /" "* [ %[^ \\]] ] %[^]")...;
	catch (...) {
		try name,code,misc = l.scanf(" # define ASM%[^ ] 0x%[0-9a-fA-F] /" "* %[^]")...;
		catch (...) {
			continue;
		}
	}
	local desc;
	try none,desc = misc.scanf(" [ %[^\\]] ] ` %[^\'] '")...;
	catch (...) {
		try desc = misc.scanf(" ` %[^\'] '")...;
		catch (...) desc = "-";
	}
	code = (int)("0x"+code);
	local data = pack(name,desc);
	if (code < 256) {
		if (codes[code] is none)
			codes[code] = data;
	} else if (code >= 0xf000 && code <= 0xf0ff) {
		if (codes_f0[code-0xf000] is none)
			codes_f0[code-0xf000] = data;
	}
	if (longest_name < #name)
		longest_name = #name;
}
function print_targets(prefix,codes) {
	for (local id,data: util::enumerate(codes)) {
			if (data is none) {
				print "\t.long "+prefix+"invop";
			} else {
				local name,desc = data...;
				print "\t.long "+prefix+"target"+name.lower()+(" "*(longest_name-#name))+" /" "* `"+desc+"' *" "/";
			}
	}
}
function print_ifndef(prefix,codes) {
	for (local id,data: util::enumerate(codes)) {
		if (data !is none) {
			local name,desc = data...;
			print ".ifndef "+prefix+"target"+name.lower()+"";
			print prefix+"target"+name.lower()+" = "+prefix+"invop";
			print ".endif";
		}
	}
}

//print_ifndef("",codes);
//print_ifndef("",codes_f0);
print_ifndef("prefix_",codes);
print_ifndef("prefix_",codes_f0);

print "inst_table:";
print_targets("",codes);
print "END(inst_table)";

print "inst_table_f0:";
print_targets("",codes_f0);
print "END(inst_table_f0)";

print "prefix_inst_table:";
print_targets("prefix_",codes);
print "END(prefix_inst_table)";

print "prefix_inst_table_f0:";
print_targets("prefix_",codes_f0);
print "END(prefix_inst_table_f0)";

]]]*/
.ifndef prefix_target_ret_none
prefix_target_ret_none = prefix_invop
.endif
.ifndef prefix_target_ret
prefix_target_ret = prefix_invop
.endif
.ifndef prefix_target_yieldall
prefix_target_yieldall = prefix_invop
.endif
.ifndef prefix_target_throw
prefix_target_throw = prefix_invop
.endif
.ifndef prefix_target_rethrow
prefix_target_rethrow = prefix_invop
.endif
.ifndef prefix_target_setret
prefix_target_setret = prefix_invop
.endif
.ifndef prefix_target_endcatch
prefix_target_endcatch = prefix_invop
.endif
.ifndef prefix_target_endfinally
prefix_target_endfinally = prefix_invop
.endif
.ifndef prefix_target_call_kw
prefix_target_call_kw = prefix_invop
.endif
.ifndef prefix_target_call_tuple_kw
prefix_target_call_tuple_kw = prefix_invop
.endif
.ifndef prefix_target_push_bnd_arg
prefix_target_push_bnd_arg = prefix_invop
.endif
.ifndef prefix_target_push_bnd_extern
prefix_target_push_bnd_extern = prefix_invop
.endif
.ifndef prefix_target_push_bnd_global
prefix_target_push_bnd_global = prefix_invop
.endif
.ifndef prefix_target_push_bnd_local
prefix_target_push_bnd_local = prefix_invop
.endif
.ifndef prefix_target_jf
prefix_target_jf = prefix_invop
.endif
.ifndef prefix_target_jf16
prefix_target_jf16 = prefix_invop
.endif
.ifndef prefix_target_jt
prefix_target_jt = prefix_invop
.endif
.ifndef prefix_target_jt16
prefix_target_jt16 = prefix_invop
.endif
.ifndef prefix_target_jmp
prefix_target_jmp = prefix_invop
.endif
.ifndef prefix_target_jmp16
prefix_target_jmp16 = prefix_invop
.endif
.ifndef prefix_target_foreach
prefix_target_foreach = prefix_invop
.endif
.ifndef prefix_target_foreach16
prefix_target_foreach16 = prefix_invop
.endif
.ifndef prefix_target_jmp_pop
prefix_target_jmp_pop = prefix_invop
.endif
.ifndef prefix_target_operator
prefix_target_operator = prefix_invop
.endif
.ifndef prefix_target_operator_tuple
prefix_target_operator_tuple = prefix_invop
.endif
.ifndef prefix_target_call
prefix_target_call = prefix_invop
.endif
.ifndef prefix_target_call_tuple
prefix_target_call_tuple = prefix_invop
.endif
.ifndef prefix_target_del_global
prefix_target_del_global = prefix_invop
.endif
.ifndef prefix_target_del_local
prefix_target_del_local = prefix_invop
.endif
.ifndef prefix_target_swap
prefix_target_swap = prefix_invop
.endif
.ifndef prefix_target_lrot
prefix_target_lrot = prefix_invop
.endif
.ifndef prefix_target_rrot
prefix_target_rrot = prefix_invop
.endif
.ifndef prefix_target_dup
prefix_target_dup = prefix_invop
.endif
.ifndef prefix_target_dup_n
prefix_target_dup_n = prefix_invop
.endif
.ifndef prefix_target_pop
prefix_target_pop = prefix_invop
.endif
.ifndef prefix_target_pop_n
prefix_target_pop_n = prefix_invop
.endif
.ifndef prefix_target_adjstack
prefix_target_adjstack = prefix_invop
.endif
.ifndef prefix_target_super
prefix_target_super = prefix_invop
.endif
.ifndef prefix_target_super_this_r
prefix_target_super_this_r = prefix_invop
.endif
.ifndef prefix_target_pop_static
prefix_target_pop_static = prefix_invop
.endif
.ifndef prefix_target_pop_extern
prefix_target_pop_extern = prefix_invop
.endif
.ifndef prefix_target_pop_global
prefix_target_pop_global = prefix_invop
.endif
.ifndef prefix_target_pop_local
prefix_target_pop_local = prefix_invop
.endif
.ifndef prefix_target_push_none
prefix_target_push_none = prefix_invop
.endif
.ifndef prefix_target_push_varargs
prefix_target_push_varargs = prefix_invop
.endif
.ifndef prefix_target_push_varkwds
prefix_target_push_varkwds = prefix_invop
.endif
.ifndef prefix_target_push_module
prefix_target_push_module = prefix_invop
.endif
.ifndef prefix_target_push_arg
prefix_target_push_arg = prefix_invop
.endif
.ifndef prefix_target_push_const
prefix_target_push_const = prefix_invop
.endif
.ifndef prefix_target_push_ref
prefix_target_push_ref = prefix_invop
.endif
.ifndef prefix_target_push_static
prefix_target_push_static = prefix_invop
.endif
.ifndef prefix_target_push_extern
prefix_target_push_extern = prefix_invop
.endif
.ifndef prefix_target_push_global
prefix_target_push_global = prefix_invop
.endif
.ifndef prefix_target_push_local
prefix_target_push_local = prefix_invop
.endif
.ifndef prefix_target_cast_tuple
prefix_target_cast_tuple = prefix_invop
.endif
.ifndef prefix_target_cast_list
prefix_target_cast_list = prefix_invop
.endif
.ifndef prefix_target_pack_tuple
prefix_target_pack_tuple = prefix_invop
.endif
.ifndef prefix_target_pack_list
prefix_target_pack_list = prefix_invop
.endif
.ifndef prefix_target_unpack
prefix_target_unpack = prefix_invop
.endif
.ifndef prefix_target_concat
prefix_target_concat = prefix_invop
.endif
.ifndef prefix_target_extend
prefix_target_extend = prefix_invop
.endif
.ifndef prefix_target_typeof
prefix_target_typeof = prefix_invop
.endif
.ifndef prefix_target_classof
prefix_target_classof = prefix_invop
.endif
.ifndef prefix_target_superof
prefix_target_superof = prefix_invop
.endif
.ifndef prefix_target_instanceof
prefix_target_instanceof = prefix_invop
.endif
.ifndef prefix_target_str
prefix_target_str = prefix_invop
.endif
.ifndef prefix_target_repr
prefix_target_repr = prefix_invop
.endif
.ifndef prefix_target_bool
prefix_target_bool = prefix_invop
.endif
.ifndef prefix_target_not
prefix_target_not = prefix_invop
.endif
.ifndef prefix_target_assign
prefix_target_assign = prefix_invop
.endif
.ifndef prefix_target_move_assign
prefix_target_move_assign = prefix_invop
.endif
.ifndef prefix_target_copy
prefix_target_copy = prefix_invop
.endif
.ifndef prefix_target_deepcopy
prefix_target_deepcopy = prefix_invop
.endif
.ifndef prefix_target_getattr
prefix_target_getattr = prefix_invop
.endif
.ifndef prefix_target_delattr
prefix_target_delattr = prefix_invop
.endif
.ifndef prefix_target_setattr
prefix_target_setattr = prefix_invop
.endif
.ifndef prefix_target_boundattr
prefix_target_boundattr = prefix_invop
.endif
.ifndef prefix_target_getattr_c
prefix_target_getattr_c = prefix_invop
.endif
.ifndef prefix_target_delattr_c
prefix_target_delattr_c = prefix_invop
.endif
.ifndef prefix_target_setattr_c
prefix_target_setattr_c = prefix_invop
.endif
.ifndef prefix_target_getattr_this_c
prefix_target_getattr_this_c = prefix_invop
.endif
.ifndef prefix_target_delattr_this_c
prefix_target_delattr_this_c = prefix_invop
.endif
.ifndef prefix_target_setattr_this_c
prefix_target_setattr_this_c = prefix_invop
.endif
.ifndef prefix_target_cmp_eq
prefix_target_cmp_eq = prefix_invop
.endif
.ifndef prefix_target_cmp_ne
prefix_target_cmp_ne = prefix_invop
.endif
.ifndef prefix_target_cmp_lo
prefix_target_cmp_lo = prefix_invop
.endif
.ifndef prefix_target_cmp_le
prefix_target_cmp_le = prefix_invop
.endif
.ifndef prefix_target_cmp_gr
prefix_target_cmp_gr = prefix_invop
.endif
.ifndef prefix_target_cmp_ge
prefix_target_cmp_ge = prefix_invop
.endif
.ifndef prefix_target_class_c
prefix_target_class_c = prefix_invop
.endif
.ifndef prefix_target_class_gc
prefix_target_class_gc = prefix_invop
.endif
.ifndef prefix_target_class_ec
prefix_target_class_ec = prefix_invop
.endif
.ifndef prefix_target_defcmember
prefix_target_defcmember = prefix_invop
.endif
.ifndef prefix_target_getcmember_r
prefix_target_getcmember_r = prefix_invop
.endif
.ifndef prefix_target_callcmember_this_r
prefix_target_callcmember_this_r = prefix_invop
.endif
.ifndef prefix_target_function_c
prefix_target_function_c = prefix_invop
.endif
.ifndef prefix_target_function_c_16
prefix_target_function_c_16 = prefix_invop
.endif
.ifndef prefix_target_cast_int
prefix_target_cast_int = prefix_invop
.endif
.ifndef prefix_target_inv
prefix_target_inv = prefix_invop
.endif
.ifndef prefix_target_pos
prefix_target_pos = prefix_invop
.endif
.ifndef prefix_target_neg
prefix_target_neg = prefix_invop
.endif
.ifndef prefix_target_add
prefix_target_add = prefix_invop
.endif
.ifndef prefix_target_sub
prefix_target_sub = prefix_invop
.endif
.ifndef prefix_target_mul
prefix_target_mul = prefix_invop
.endif
.ifndef prefix_target_div
prefix_target_div = prefix_invop
.endif
.ifndef prefix_target_mod
prefix_target_mod = prefix_invop
.endif
.ifndef prefix_target_shl
prefix_target_shl = prefix_invop
.endif
.ifndef prefix_target_shr
prefix_target_shr = prefix_invop
.endif
.ifndef prefix_target_and
prefix_target_and = prefix_invop
.endif
.ifndef prefix_target_or
prefix_target_or = prefix_invop
.endif
.ifndef prefix_target_xor
prefix_target_xor = prefix_invop
.endif
.ifndef prefix_target_pow
prefix_target_pow = prefix_invop
.endif
.ifndef prefix_target_inc
prefix_target_inc = prefix_invop
.endif
.ifndef prefix_target_dec
prefix_target_dec = prefix_invop
.endif
.ifndef prefix_target_add_simm8
prefix_target_add_simm8 = prefix_invop
.endif
.ifndef prefix_target_add_imm32
prefix_target_add_imm32 = prefix_invop
.endif
.ifndef prefix_target_sub_simm8
prefix_target_sub_simm8 = prefix_invop
.endif
.ifndef prefix_target_sub_imm32
prefix_target_sub_imm32 = prefix_invop
.endif
.ifndef prefix_target_mul_simm8
prefix_target_mul_simm8 = prefix_invop
.endif
.ifndef prefix_target_div_simm8
prefix_target_div_simm8 = prefix_invop
.endif
.ifndef prefix_target_mod_simm8
prefix_target_mod_simm8 = prefix_invop
.endif
.ifndef prefix_target_shl_imm8
prefix_target_shl_imm8 = prefix_invop
.endif
.ifndef prefix_target_shr_imm8
prefix_target_shr_imm8 = prefix_invop
.endif
.ifndef prefix_target_and_imm32
prefix_target_and_imm32 = prefix_invop
.endif
.ifndef prefix_target_or_imm32
prefix_target_or_imm32 = prefix_invop
.endif
.ifndef prefix_target_xor_imm32
prefix_target_xor_imm32 = prefix_invop
.endif
.ifndef prefix_target_isnone
prefix_target_isnone = prefix_invop
.endif
.ifndef prefix_target_delop
prefix_target_delop = prefix_invop
.endif
.ifndef prefix_target_nop
prefix_target_nop = prefix_invop
.endif
.ifndef prefix_target_print
prefix_target_print = prefix_invop
.endif
.ifndef prefix_target_print_sp
prefix_target_print_sp = prefix_invop
.endif
.ifndef prefix_target_print_nl
prefix_target_print_nl = prefix_invop
.endif
.ifndef prefix_target_printnl
prefix_target_printnl = prefix_invop
.endif
.ifndef prefix_target_printall
prefix_target_printall = prefix_invop
.endif
.ifndef prefix_target_printall_sp
prefix_target_printall_sp = prefix_invop
.endif
.ifndef prefix_target_printall_nl
prefix_target_printall_nl = prefix_invop
.endif
.ifndef prefix_target_fprint
prefix_target_fprint = prefix_invop
.endif
.ifndef prefix_target_fprint_sp
prefix_target_fprint_sp = prefix_invop
.endif
.ifndef prefix_target_fprint_nl
prefix_target_fprint_nl = prefix_invop
.endif
.ifndef prefix_target_fprintnl
prefix_target_fprintnl = prefix_invop
.endif
.ifndef prefix_target_fprintall
prefix_target_fprintall = prefix_invop
.endif
.ifndef prefix_target_fprintall_sp
prefix_target_fprintall_sp = prefix_invop
.endif
.ifndef prefix_target_fprintall_nl
prefix_target_fprintall_nl = prefix_invop
.endif
.ifndef prefix_target_print_c
prefix_target_print_c = prefix_invop
.endif
.ifndef prefix_target_print_c_sp
prefix_target_print_c_sp = prefix_invop
.endif
.ifndef prefix_target_print_c_nl
prefix_target_print_c_nl = prefix_invop
.endif
.ifndef prefix_target_range_0_i16
prefix_target_range_0_i16 = prefix_invop
.endif
.ifndef prefix_target_enter
prefix_target_enter = prefix_invop
.endif
.ifndef prefix_target_leave
prefix_target_leave = prefix_invop
.endif
.ifndef prefix_target_fprint_c
prefix_target_fprint_c = prefix_invop
.endif
.ifndef prefix_target_fprint_c_sp
prefix_target_fprint_c_sp = prefix_invop
.endif
.ifndef prefix_target_fprint_c_nl
prefix_target_fprint_c_nl = prefix_invop
.endif
.ifndef prefix_target_range
prefix_target_range = prefix_invop
.endif
.ifndef prefix_target_range_def
prefix_target_range_def = prefix_invop
.endif
.ifndef prefix_target_range_step
prefix_target_range_step = prefix_invop
.endif
.ifndef prefix_target_range_step_def
prefix_target_range_step_def = prefix_invop
.endif
.ifndef prefix_target_contains
prefix_target_contains = prefix_invop
.endif
.ifndef prefix_target_contains_c
prefix_target_contains_c = prefix_invop
.endif
.ifndef prefix_target_getitem
prefix_target_getitem = prefix_invop
.endif
.ifndef prefix_target_getitem_i
prefix_target_getitem_i = prefix_invop
.endif
.ifndef prefix_target_getitem_c
prefix_target_getitem_c = prefix_invop
.endif
.ifndef prefix_target_getsize
prefix_target_getsize = prefix_invop
.endif
.ifndef prefix_target_setitem
prefix_target_setitem = prefix_invop
.endif
.ifndef prefix_target_setitem_i
prefix_target_setitem_i = prefix_invop
.endif
.ifndef prefix_target_setitem_c
prefix_target_setitem_c = prefix_invop
.endif
.ifndef prefix_target_iterself
prefix_target_iterself = prefix_invop
.endif
.ifndef prefix_target_delitem
prefix_target_delitem = prefix_invop
.endif
.ifndef prefix_target_getrange
prefix_target_getrange = prefix_invop
.endif
.ifndef prefix_target_getrange_pn
prefix_target_getrange_pn = prefix_invop
.endif
.ifndef prefix_target_getrange_np
prefix_target_getrange_np = prefix_invop
.endif
.ifndef prefix_target_getrange_pi
prefix_target_getrange_pi = prefix_invop
.endif
.ifndef prefix_target_getrange_ip
prefix_target_getrange_ip = prefix_invop
.endif
.ifndef prefix_target_getrange_ni
prefix_target_getrange_ni = prefix_invop
.endif
.ifndef prefix_target_getrange_in
prefix_target_getrange_in = prefix_invop
.endif
.ifndef prefix_target_getrange_ii
prefix_target_getrange_ii = prefix_invop
.endif
.ifndef prefix_target_delrange
prefix_target_delrange = prefix_invop
.endif
.ifndef prefix_target_setrange
prefix_target_setrange = prefix_invop
.endif
.ifndef prefix_target_setrange_pn
prefix_target_setrange_pn = prefix_invop
.endif
.ifndef prefix_target_setrange_np
prefix_target_setrange_np = prefix_invop
.endif
.ifndef prefix_target_setrange_pi
prefix_target_setrange_pi = prefix_invop
.endif
.ifndef prefix_target_setrange_ip
prefix_target_setrange_ip = prefix_invop
.endif
.ifndef prefix_target_setrange_ni
prefix_target_setrange_ni = prefix_invop
.endif
.ifndef prefix_target_setrange_in
prefix_target_setrange_in = prefix_invop
.endif
.ifndef prefix_target_setrange_ii
prefix_target_setrange_ii = prefix_invop
.endif
.ifndef prefix_target_breakpoint
prefix_target_breakpoint = prefix_invop
.endif
.ifndef prefix_target_ud
prefix_target_ud = prefix_invop
.endif
.ifndef prefix_target_callattr_c_kw
prefix_target_callattr_c_kw = prefix_invop
.endif
.ifndef prefix_target_callattr_c_tuple_kw
prefix_target_callattr_c_tuple_kw = prefix_invop
.endif
.ifndef prefix_target_callattr
prefix_target_callattr = prefix_invop
.endif
.ifndef prefix_target_callattr_tuple
prefix_target_callattr_tuple = prefix_invop
.endif
.ifndef prefix_target_callattr_c
prefix_target_callattr_c = prefix_invop
.endif
.ifndef prefix_target_callattr_c_tuple
prefix_target_callattr_c_tuple = prefix_invop
.endif
.ifndef prefix_target_callattr_this_c
prefix_target_callattr_this_c = prefix_invop
.endif
.ifndef prefix_target_callattr_this_c_tuple
prefix_target_callattr_this_c_tuple = prefix_invop
.endif
.ifndef prefix_target_callattr_c_seq
prefix_target_callattr_c_seq = prefix_invop
.endif
.ifndef prefix_target_callattr_c_map
prefix_target_callattr_c_map = prefix_invop
.endif
.ifndef prefix_target_getmember_this_r
prefix_target_getmember_this_r = prefix_invop
.endif
.ifndef prefix_target_delmember_this_r
prefix_target_delmember_this_r = prefix_invop
.endif
.ifndef prefix_target_setmember_this_r
prefix_target_setmember_this_r = prefix_invop
.endif
.ifndef prefix_target_boundmember_this_r
prefix_target_boundmember_this_r = prefix_invop
.endif
.ifndef prefix_target_call_extern
prefix_target_call_extern = prefix_invop
.endif
.ifndef prefix_target_call_global
prefix_target_call_global = prefix_invop
.endif
.ifndef prefix_target_call_local
prefix_target_call_local = prefix_invop
.endif
.ifndef prefix_target_extended1
prefix_target_extended1 = prefix_invop
.endif
.ifndef prefix_target_reserved1
prefix_target_reserved1 = prefix_invop
.endif
.ifndef prefix_target_reserved2
prefix_target_reserved2 = prefix_invop
.endif
.ifndef prefix_target_reserved3
prefix_target_reserved3 = prefix_invop
.endif
.ifndef prefix_target_reserved4
prefix_target_reserved4 = prefix_invop
.endif
.ifndef prefix_target_reserved5
prefix_target_reserved5 = prefix_invop
.endif
.ifndef prefix_target_reserved6
prefix_target_reserved6 = prefix_invop
.endif
.ifndef prefix_target_reserved7
prefix_target_reserved7 = prefix_invop
.endif
.ifndef prefix_target_stack
prefix_target_stack = prefix_invop
.endif
.ifndef prefix_target_static
prefix_target_static = prefix_invop
.endif
.ifndef prefix_target_extern
prefix_target_extern = prefix_invop
.endif
.ifndef prefix_target_global
prefix_target_global = prefix_invop
.endif
.ifndef prefix_target_local
prefix_target_local = prefix_invop
.endif
.ifndef prefix_target_endcatch_n
prefix_target_endcatch_n = prefix_invop
.endif
.ifndef prefix_target_endfinally_n
prefix_target_endfinally_n = prefix_invop
.endif
.ifndef prefix_target16_call_kw
prefix_target16_call_kw = prefix_invop
.endif
.ifndef prefix_target16_call_tuple_kw
prefix_target16_call_tuple_kw = prefix_invop
.endif
.ifndef prefix_target16_push_bnd_arg
prefix_target16_push_bnd_arg = prefix_invop
.endif
.ifndef prefix_target16_push_bnd_extern
prefix_target16_push_bnd_extern = prefix_invop
.endif
.ifndef prefix_target16_push_bnd_global
prefix_target16_push_bnd_global = prefix_invop
.endif
.ifndef prefix_target16_push_bnd_local
prefix_target16_push_bnd_local = prefix_invop
.endif
.ifndef prefix_target32_jmp
prefix_target32_jmp = prefix_invop
.endif
.ifndef prefix_target_jmp_pop_pop
prefix_target_jmp_pop_pop = prefix_invop
.endif
.ifndef prefix_target16_operator
prefix_target16_operator = prefix_invop
.endif
.ifndef prefix_target16_operator_tuple
prefix_target16_operator_tuple = prefix_invop
.endif
.ifndef prefix_target_call_seq
prefix_target_call_seq = prefix_invop
.endif
.ifndef prefix_target_call_map
prefix_target_call_map = prefix_invop
.endif
.ifndef prefix_target_thiscall_tuple
prefix_target_thiscall_tuple = prefix_invop
.endif
.ifndef prefix_target16_del_global
prefix_target16_del_global = prefix_invop
.endif
.ifndef prefix_target16_del_local
prefix_target16_del_local = prefix_invop
.endif
.ifndef prefix_target_call_tuple_kwds
prefix_target_call_tuple_kwds = prefix_invop
.endif
.ifndef prefix_target16_lrot
prefix_target16_lrot = prefix_invop
.endif
.ifndef prefix_target16_rrot
prefix_target16_rrot = prefix_invop
.endif
.ifndef prefix_target16_dup_n
prefix_target16_dup_n = prefix_invop
.endif
.ifndef prefix_target16_pop_n
prefix_target16_pop_n = prefix_invop
.endif
.ifndef prefix_target16_adjstack
prefix_target16_adjstack = prefix_invop
.endif
.ifndef prefix_target16_super_this_r
prefix_target16_super_this_r = prefix_invop
.endif
.ifndef prefix_target16_pop_static
prefix_target16_pop_static = prefix_invop
.endif
.ifndef prefix_target16_pop_extern
prefix_target16_pop_extern = prefix_invop
.endif
.ifndef prefix_target16_pop_global
prefix_target16_pop_global = prefix_invop
.endif
.ifndef prefix_target16_pop_local
prefix_target16_pop_local = prefix_invop
.endif
.ifndef prefix_target_push_except
prefix_target_push_except = prefix_invop
.endif
.ifndef prefix_target_push_this
prefix_target_push_this = prefix_invop
.endif
.ifndef prefix_target_push_this_module
prefix_target_push_this_module = prefix_invop
.endif
.ifndef prefix_target_push_this_function
prefix_target_push_this_function = prefix_invop
.endif
.ifndef prefix_target16_push_module
prefix_target16_push_module = prefix_invop
.endif
.ifndef prefix_target16_push_arg
prefix_target16_push_arg = prefix_invop
.endif
.ifndef prefix_target16_push_const
prefix_target16_push_const = prefix_invop
.endif
.ifndef prefix_target16_push_ref
prefix_target16_push_ref = prefix_invop
.endif
.ifndef prefix_target16_push_static
prefix_target16_push_static = prefix_invop
.endif
.ifndef prefix_target16_push_extern
prefix_target16_push_extern = prefix_invop
.endif
.ifndef prefix_target16_push_global
prefix_target16_push_global = prefix_invop
.endif
.ifndef prefix_target16_push_local
prefix_target16_push_local = prefix_invop
.endif
.ifndef prefix_target_cast_hashset
prefix_target_cast_hashset = prefix_invop
.endif
.ifndef prefix_target_cast_dict
prefix_target_cast_dict = prefix_invop
.endif
.ifndef prefix_target16_pack_tuple
prefix_target16_pack_tuple = prefix_invop
.endif
.ifndef prefix_target16_pack_list
prefix_target16_pack_list = prefix_invop
.endif
.ifndef prefix_target16_unpack
prefix_target16_unpack = prefix_invop
.endif
.ifndef prefix_target_push_true
prefix_target_push_true = prefix_invop
.endif
.ifndef prefix_target_push_false
prefix_target_push_false = prefix_invop
.endif
.ifndef prefix_target_pack_hashset
prefix_target_pack_hashset = prefix_invop
.endif
.ifndef prefix_target_pack_dict
prefix_target_pack_dict = prefix_invop
.endif
.ifndef prefix_target_bounditem
prefix_target_bounditem = prefix_invop
.endif
.ifndef prefix_target16_getattr_c
prefix_target16_getattr_c = prefix_invop
.endif
.ifndef prefix_target16_delattr_c
prefix_target16_delattr_c = prefix_invop
.endif
.ifndef prefix_target16_setattr_c
prefix_target16_setattr_c = prefix_invop
.endif
.ifndef prefix_target16_getattr_this_c
prefix_target16_getattr_this_c = prefix_invop
.endif
.ifndef prefix_target16_delattr_this_c
prefix_target16_delattr_this_c = prefix_invop
.endif
.ifndef prefix_target16_setattr_this_c
prefix_target16_setattr_this_c = prefix_invop
.endif
.ifndef prefix_target_cmp_so
prefix_target_cmp_so = prefix_invop
.endif
.ifndef prefix_target_cmp_do
prefix_target_cmp_do = prefix_invop
.endif
.ifndef prefix_target16_pack_hashset
prefix_target16_pack_hashset = prefix_invop
.endif
.ifndef prefix_target16_pack_dict
prefix_target16_pack_dict = prefix_invop
.endif
.ifndef prefix_target16_getcmember
prefix_target16_getcmember = prefix_invop
.endif
.ifndef prefix_target_class
prefix_target_class = prefix_invop
.endif
.ifndef prefix_target16_class_c
prefix_target16_class_c = prefix_invop
.endif
.ifndef prefix_target16_class_gc
prefix_target16_class_gc = prefix_invop
.endif
.ifndef prefix_target16_class_ec
prefix_target16_class_ec = prefix_invop
.endif
.ifndef prefix_target16_defcmember
prefix_target16_defcmember = prefix_invop
.endif
.ifndef prefix_target16_getcmember_r
prefix_target16_getcmember_r = prefix_invop
.endif
.ifndef prefix_target16_callcmember_this_r
prefix_target16_callcmember_this_r = prefix_invop
.endif
.ifndef prefix_target16_function_c
prefix_target16_function_c = prefix_invop
.endif
.ifndef prefix_target16_function_c_16
prefix_target16_function_c_16 = prefix_invop
.endif
.ifndef prefix_target_supergetattr_this_rc
prefix_target_supergetattr_this_rc = prefix_invop
.endif
.ifndef prefix_target16_supergetattr_this_rc
prefix_target16_supergetattr_this_rc = prefix_invop
.endif
.ifndef prefix_target_supercallattr_this_rc
prefix_target_supercallattr_this_rc = prefix_invop
.endif
.ifndef prefix_target16_supercallattr_this_rc
prefix_target16_supercallattr_this_rc = prefix_invop
.endif
.ifndef prefix_target_incpost
prefix_target_incpost = prefix_invop
.endif
.ifndef prefix_target_decpost
prefix_target_decpost = prefix_invop
.endif
.ifndef prefix_target16_delop
prefix_target16_delop = prefix_invop
.endif
.ifndef prefix_target16_nop
prefix_target16_nop = prefix_invop
.endif
.ifndef prefix_target_reduce_min
prefix_target_reduce_min = prefix_invop
.endif
.ifndef prefix_target_reduce_max
prefix_target_reduce_max = prefix_invop
.endif
.ifndef prefix_target_reduce_sum
prefix_target_reduce_sum = prefix_invop
.endif
.ifndef prefix_target_reduce_any
prefix_target_reduce_any = prefix_invop
.endif
.ifndef prefix_target_reduce_all
prefix_target_reduce_all = prefix_invop
.endif
.ifndef prefix_target16_print_c
prefix_target16_print_c = prefix_invop
.endif
.ifndef prefix_target16_print_c_sp
prefix_target16_print_c_sp = prefix_invop
.endif
.ifndef prefix_target16_print_c_nl
prefix_target16_print_c_nl = prefix_invop
.endif
.ifndef prefix_target_range_0_i32
prefix_target_range_0_i32 = prefix_invop
.endif
.ifndef prefix_target_varargs_unpack
prefix_target_varargs_unpack = prefix_invop
.endif
.ifndef prefix_target_push_varkwds_ne
prefix_target_push_varkwds_ne = prefix_invop
.endif
.ifndef prefix_target16_fprint_c
prefix_target16_fprint_c = prefix_invop
.endif
.ifndef prefix_target16_fprint_c_sp
prefix_target16_fprint_c_sp = prefix_invop
.endif
.ifndef prefix_target16_fprint_c_nl
prefix_target16_fprint_c_nl = prefix_invop
.endif
.ifndef prefix_target_varargs_cmp_eq_sz
prefix_target_varargs_cmp_eq_sz = prefix_invop
.endif
.ifndef prefix_target_varargs_cmp_gr_sz
prefix_target_varargs_cmp_gr_sz = prefix_invop
.endif
.ifndef prefix_target16_contains_c
prefix_target16_contains_c = prefix_invop
.endif
.ifndef prefix_target_varargs_getitem
prefix_target_varargs_getitem = prefix_invop
.endif
.ifndef prefix_target_varargs_getitem_i
prefix_target_varargs_getitem_i = prefix_invop
.endif
.ifndef prefix_target16_getitem_c
prefix_target16_getitem_c = prefix_invop
.endif
.ifndef prefix_target_varargs_getsize
prefix_target_varargs_getsize = prefix_invop
.endif
.ifndef prefix_target16_setitem_c
prefix_target16_setitem_c = prefix_invop
.endif
.ifndef prefix_target_iternext
prefix_target_iternext = prefix_invop
.endif
.ifndef prefix_target_getmember
prefix_target_getmember = prefix_invop
.endif
.ifndef prefix_target16_getmember
prefix_target16_getmember = prefix_invop
.endif
.ifndef prefix_target_delmember
prefix_target_delmember = prefix_invop
.endif
.ifndef prefix_target16_delmember
prefix_target16_delmember = prefix_invop
.endif
.ifndef prefix_target_setmember
prefix_target_setmember = prefix_invop
.endif
.ifndef prefix_target16_setmember
prefix_target16_setmember = prefix_invop
.endif
.ifndef prefix_target_boundmember
prefix_target_boundmember = prefix_invop
.endif
.ifndef prefix_target16_boundmember
prefix_target16_boundmember = prefix_invop
.endif
.ifndef prefix_target_getmember_this
prefix_target_getmember_this = prefix_invop
.endif
.ifndef prefix_target16_getmember_this
prefix_target16_getmember_this = prefix_invop
.endif
.ifndef prefix_target_delmember_this
prefix_target_delmember_this = prefix_invop
.endif
.ifndef prefix_target16_delmember_this
prefix_target16_delmember_this = prefix_invop
.endif
.ifndef prefix_target_setmember_this
prefix_target_setmember_this = prefix_invop
.endif
.ifndef prefix_target16_setmember_this
prefix_target16_setmember_this = prefix_invop
.endif
.ifndef prefix_target_boundmember_this
prefix_target_boundmember_this = prefix_invop
.endif
.ifndef prefix_target16_boundmember_this
prefix_target16_boundmember_this = prefix_invop
.endif
.ifndef prefix_target16_callattr_c_kw
prefix_target16_callattr_c_kw = prefix_invop
.endif
.ifndef prefix_target16_callattr_c_tuple_kw
prefix_target16_callattr_c_tuple_kw = prefix_invop
.endif
.ifndef prefix_target_callattr_kwds
prefix_target_callattr_kwds = prefix_invop
.endif
.ifndef prefix_target_callattr_tuple_kwds
prefix_target_callattr_tuple_kwds = prefix_invop
.endif
.ifndef prefix_target16_callattr_c
prefix_target16_callattr_c = prefix_invop
.endif
.ifndef prefix_target16_callattr_c_tuple
prefix_target16_callattr_c_tuple = prefix_invop
.endif
.ifndef prefix_target16_callattr_this_c
prefix_target16_callattr_this_c = prefix_invop
.endif
.ifndef prefix_target16_callattr_this_c_tuple
prefix_target16_callattr_this_c_tuple = prefix_invop
.endif
.ifndef prefix_target16_callattr_c_seq
prefix_target16_callattr_c_seq = prefix_invop
.endif
.ifndef prefix_target16_callattr_c_map
prefix_target16_callattr_c_map = prefix_invop
.endif
.ifndef prefix_target16_getmember_this_r
prefix_target16_getmember_this_r = prefix_invop
.endif
.ifndef prefix_target16_delmember_this_r
prefix_target16_delmember_this_r = prefix_invop
.endif
.ifndef prefix_target16_setmember_this_r
prefix_target16_setmember_this_r = prefix_invop
.endif
.ifndef prefix_target16_boundmember_this_r
prefix_target16_boundmember_this_r = prefix_invop
.endif
.ifndef prefix_target16_call_extern
prefix_target16_call_extern = prefix_invop
.endif
.ifndef prefix_target16_call_global
prefix_target16_call_global = prefix_invop
.endif
.ifndef prefix_target16_call_local
prefix_target16_call_local = prefix_invop
.endif
.ifndef prefix_target16_stack
prefix_target16_stack = prefix_invop
.endif
.ifndef prefix_target16_static
prefix_target16_static = prefix_invop
.endif
.ifndef prefix_target16_extern
prefix_target16_extern = prefix_invop
.endif
.ifndef prefix_target16_global
prefix_target16_global = prefix_invop
.endif
.ifndef prefix_target16_local
prefix_target16_local = prefix_invop
.endif
inst_table:
	.long target_ret_none                /* `ret' */
	.long target_ret                     /* `ret pop' */
	.long target_yieldall                /* `yield foreach, pop' */
	.long target_throw                   /* `throw pop' */
	.long target_rethrow                 /* `throw except' */
	.long target_setret                  /* `setret pop' */
	.long target_endcatch                /* `end catch' */
	.long target_endfinally              /* `end finally' */
	.long target_call_kw                 /* `call top, #<imm8>, const <imm8>' */
	.long target_call_tuple_kw           /* `call top, pop..., const <imm8>' */
	.long invop
	.long invop
	.long target_push_bnd_arg            /* `push bound arg <imm8>' */
	.long target_push_bnd_extern         /* `push bound extern <imm8>:<imm8>' */
	.long target_push_bnd_global         /* `push bound global <imm8>' */
	.long target_push_bnd_local          /* `push bound local <imm8>' */
	.long target_jf                      /* `jf pop, <Sdisp8>' */
	.long target_jf16                    /* `jf pop, <Sdisp16>' */
	.long target_jt                      /* `jt pop, <Sdisp8>' */
	.long target_jt16                    /* `jt pop, <Sdisp16>' */
	.long target_jmp                     /* `jmp <Sdisp8>' */
	.long target_jmp16                   /* `jmp <Sdisp16>' */
	.long target_foreach                 /* `foreach top, <Sdisp8>' */
	.long target_foreach16               /* `foreach top, <Sdisp16>' */
	.long target_jmp_pop                 /* `jmp pop' */
	.long target_operator                /* `op top, $<imm8>, #<imm8>' */
	.long target_operator_tuple          /* `op top, $<imm8>, pop...' */
	.long target_call                    /* `call top, #<imm8>' */
	.long target_call_tuple              /* `call top, pop...' */
	.long invop
	.long target_del_global              /* `del global <imm8>' */
	.long target_del_local               /* `del local <imm8>' */
	.long target_swap                    /* `swap' */
	.long target_lrot                    /* `lrot #<imm8>+3' */
	.long target_rrot                    /* `rrot #<imm8>+3' */
	.long target_dup                     /* `dup' */
	.long target_dup_n                   /* `dup #SP - <imm8> - 2' */
	.long target_pop                     /* `pop' */
	.long target_pop_n                   /* `pop #SP - <imm8> - 2' */
	.long target_adjstack                /* `adjstack #SP +/- <Simm8>' */
	.long target_super                   /* `super top, pop' */
	.long target_super_this_r            /* `push super this, ref <imm8>' */
	.long invop
	.long invop
	.long target_pop_static              /* `pop static <imm8>' */
	.long target_pop_extern              /* `pop extern <imm8>:<imm8>' */
	.long target_pop_global              /* `pop global <imm8>' */
	.long target_pop_local               /* `pop local <imm8>' */
	.long invop
	.long invop
	.long invop
	.long target_push_none               /* `push none' */
	.long invop
	.long invop
	.long target_push_varargs            /* `push varargs' */
	.long target_push_varkwds            /* `push varkwds' */
	.long target_push_module             /* `push module <imm8>' */
	.long target_push_arg                /* `push arg <imm8>' */
	.long target_push_const              /* `push const <imm8>' */
	.long target_push_ref                /* `push ref <imm8>' */
	.long target_push_static             /* `push static <imm8>' */
	.long target_push_extern             /* `push extern <imm8>:<imm8>' */
	.long target_push_global             /* `push global <imm8>' */
	.long target_push_local              /* `push local <imm8>' */
	.long target_cast_tuple              /* `cast top, Tuple' */
	.long target_cast_list               /* `cast top, List' */
	.long target_pack_tuple              /* `push pack Tuple, #<imm8>' */
	.long target_pack_list               /* `push pack List, #<imm8>' */
	.long invop
	.long invop
	.long target_unpack                  /* `unpack pop, #<imm8>' */
	.long target_concat                  /* `concat top, pop' */
	.long target_extend                  /* `extend top, #<imm8>' */
	.long target_typeof                  /* `typeof top' */
	.long target_classof                 /* `classof top' */
	.long target_superof                 /* `superof top' */
	.long target_instanceof              /* `instanceof top, pop' */
	.long target_str                     /* `str top' */
	.long target_repr                    /* `repr top' */
	.long invop
	.long target_bool                    /* `bool top' */
	.long target_not                     /* `not top' */
	.long target_assign                  /* `assign pop, pop' */
	.long target_move_assign             /* `assign move, pop, pop' */
	.long target_copy                    /* `copy top' */
	.long target_deepcopy                /* `deepcopy top' */
	.long target_getattr                 /* `getattr top, pop' */
	.long target_delattr                 /* `delattr pop, pop' */
	.long target_setattr                 /* `setattr pop, pop, pop' */
	.long target_boundattr               /* `boundattr top, pop' */
	.long target_getattr_c               /* `getattr top, const <imm8>' */
	.long target_delattr_c               /* `delattr pop, const <imm8>' */
	.long target_setattr_c               /* `setattr pop, const <imm8>, pop' */
	.long target_getattr_this_c          /* `push getattr this, const <imm8>' */
	.long target_delattr_this_c          /* `delattr this, const <imm8>' */
	.long target_setattr_this_c          /* `setattr this, const <imm8>, pop' */
	.long target_cmp_eq                  /* `cmp eq, top, pop' */
	.long target_cmp_ne                  /* `cmp ne, top, pop' */
	.long target_cmp_lo                  /* `cmp lo, top, pop' */
	.long target_cmp_le                  /* `cmp le, top, pop' */
	.long target_cmp_gr                  /* `cmp gr, top, pop' */
	.long target_cmp_ge                  /* `cmp ge, top, pop' */
	.long target_class_c                 /* `class top, const <imm8>' */
	.long target_class_gc                /* `push class global <imm8>, const <imm8>' */
	.long target_class_ec                /* `push class extern <imm8>:<imm8>, const <imm8>' */
	.long target_defcmember              /* `defcmember top, $<imm8>, pop' */
	.long target_getcmember_r            /* `push getcmember ref <imm8>, $<imm8>' */
	.long target_callcmember_this_r      /* `push callcmember this, ref <imm8>, $<imm8>, #<imm8>' */
	.long invop
	.long invop
	.long target_function_c              /* `push function const <imm8>, #<imm8>+1' */
	.long target_function_c_16           /* `push function const <imm8>, #<imm16>+1' */
	.long target_cast_int                /* `cast top, int' */
	.long target_inv                     /* `inv top' */
	.long target_pos                     /* `pos top' */
	.long target_neg                     /* `neg top' */
	.long target_add                     /* `add top, pop' */
	.long target_sub                     /* `sub top, pop' */
	.long target_mul                     /* `mul top, pop' */
	.long target_div                     /* `div top, pop' */
	.long target_mod                     /* `mod top, pop' */
	.long target_shl                     /* `shl top, pop' */
	.long target_shr                     /* `shr top, pop' */
	.long target_and                     /* `and top, pop' */
	.long target_or                      /* `or top, pop' */
	.long target_xor                     /* `xor top, pop' */
	.long target_pow                     /* `pow top, pop' */
	.long target_inc                     /* `inc' */
	.long target_dec                     /* `dec' */
	.long target_add_simm8               /* `add top, $<Simm8>' */
	.long target_add_imm32               /* `add top, $<imm32>' */
	.long target_sub_simm8               /* `sub top, $<Simm8>' */
	.long target_sub_imm32               /* `sub top, $<imm32>' */
	.long target_mul_simm8               /* `mul top, $<Simm8>' */
	.long target_div_simm8               /* `div top, $<Simm8>' */
	.long target_mod_simm8               /* `mod top, $<Simm8>' */
	.long target_shl_imm8                /* `shl top, $<imm8>' */
	.long target_shr_imm8                /* `shr top, $<imm8>' */
	.long target_and_imm32               /* `and top, $<imm32>' */
	.long target_or_imm32                /* `or top, $<imm32>' */
	.long target_xor_imm32               /* `xor top, $<imm32>' */
	.long target_isnone                  /* `instanceof top, none' */
	.long invop
	.long target_delop                   /* `-' */
	.long target_nop                     /* `nop' */
	.long target_print                   /* `print pop' */
	.long target_print_sp                /* `print pop, sp' */
	.long target_print_nl                /* `print pop, nl' */
	.long target_printnl                 /* `print nl' */
	.long target_printall                /* `print pop...' */
	.long target_printall_sp             /* `print pop..., sp' */
	.long target_printall_nl             /* `print pop..., nl' */
	.long invop
	.long target_fprint                  /* `print top, pop' */
	.long target_fprint_sp               /* `print top, pop, sp' */
	.long target_fprint_nl               /* `print top, pop, nl' */
	.long target_fprintnl                /* `print top, nl' */
	.long target_fprintall               /* `print top, pop...' */
	.long target_fprintall_sp            /* `print top, pop..., sp' */
	.long target_fprintall_nl            /* `print top, pop..., nl' */
	.long invop
	.long target_print_c                 /* `print const <imm8>' */
	.long target_print_c_sp              /* `print const <imm8>, sp' */
	.long target_print_c_nl              /* `print const <imm8>, nl' */
	.long target_range_0_i16             /* `push range $0, $<imm16>' */
	.long invop
	.long target_enter                   /* `enter top' */
	.long target_leave                   /* `leave pop' */
	.long invop
	.long target_fprint_c                /* `print top, const <imm8>' */
	.long target_fprint_c_sp             /* `print top, const <imm8>, sp' */
	.long target_fprint_c_nl             /* `print top, const <imm8>, nl' */
	.long target_range                   /* `range top, pop' */
	.long target_range_def               /* `push range default, pop' */
	.long target_range_step              /* `range top, pop, pop' */
	.long target_range_step_def          /* `push range default, pop, pop' */
	.long target_contains                /* `contains top, pop' */
	.long target_contains_c              /* `push contains const <imm8>, pop' */
	.long target_getitem                 /* `getitem top, pop' */
	.long target_getitem_i               /* `getitem top, $<Simm16>' */
	.long target_getitem_c               /* `getitem top, const <imm8>' */
	.long target_getsize                 /* `getsize top' */
	.long target_setitem                 /* `setitem pop, pop, pop' */
	.long target_setitem_i               /* `setitem pop, $<Simm16>, pop' */
	.long target_setitem_c               /* `setitem pop, const <imm8>, pop' */
	.long target_iterself                /* `iterself top' */
	.long target_delitem                 /* `delitem pop, pop' */
	.long target_getrange                /* `getrange top, pop, pop' */
	.long target_getrange_pn             /* `getrange top, pop, none' */
	.long target_getrange_np             /* `getrange top, none, pop' */
	.long target_getrange_pi             /* `getrange top, pop, $<Simm16>' */
	.long target_getrange_ip             /* `getrange top, $<Simm16>, pop' */
	.long target_getrange_ni             /* `getrange top, none, $<Simm16>' */
	.long target_getrange_in             /* `getrange top, $<Simm16>, none' */
	.long target_getrange_ii             /* `getrange top, $<Simm16>, $<Simm16>' */
	.long target_delrange                /* `delrange pop, pop, pop' */
	.long target_setrange                /* `setrange pop, pop, pop, pop' */
	.long target_setrange_pn             /* `setrange pop, pop, none, pop' */
	.long target_setrange_np             /* `setrange pop, none, pop, pop' */
	.long target_setrange_pi             /* `setrange pop, pop, $<Simm16>, pop' */
	.long target_setrange_ip             /* `setrange pop, $<Simm16>, pop, pop' */
	.long target_setrange_ni             /* `setrange pop, none, $<Simm16>, pop' */
	.long target_setrange_in             /* `setrange pop, $<Simm16>, none, pop' */
	.long target_setrange_ii             /* `setrange pop, $<Simm16>, $<Simm16>, pop' */
	.long target_breakpoint              /* `debug break' */
	.long target_ud                      /* `ud' */
	.long target_callattr_c_kw           /* `callattr top, const <imm8>, #<imm8>, const <imm8>' */
	.long target_callattr_c_tuple_kw     /* `callattr top, const <imm8>, pop..., const <imm8>' */
	.long target_callattr                /* `callattr top, pop, #<imm8>' */
	.long target_callattr_tuple          /* `callattr top, pop, pop...' */
	.long target_callattr_c              /* `callattr top, const <imm8>, #<imm8>' */
	.long target_callattr_c_tuple        /* `callattr top, const <imm8>, pop...' */
	.long target_callattr_this_c         /* `push callattr this, const <imm8>, #<imm8>' */
	.long target_callattr_this_c_tuple   /* `push callattr this, const <imm8>, pop...' */
	.long target_callattr_c_seq          /* `callattr top, const <imm8>, [#<imm8>]' */
	.long target_callattr_c_map          /* `callattr top, const <imm8>, {#<imm8>*2}' */
	.long invop
	.long target_getmember_this_r        /* `push getmember this, ref <imm8>, $<imm8>' */
	.long target_delmember_this_r        /* `delmember this, ref <imm8>, $<imm8>' */
	.long target_setmember_this_r        /* `setmember this, ref <imm8>, $<imm8>, pop' */
	.long target_boundmember_this_r      /* `push boundmember this, ref <imm8>, $<imm8>' */
	.long target_call_extern             /* `push call extern <imm8>:<imm8>, #<imm8>' */
	.long target_call_global             /* `push call global <imm8>, #<imm8>' */
	.long target_call_local              /* `push call local <imm8>, #<imm8>' */
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long target_extended1               /* `-' */
	.long target_reserved1               /* `-' */
	.long target_reserved2               /* `-' */
	.long target_reserved3               /* `-' */
	.long target_reserved4               /* `-' */
	.long target_reserved5               /* `-' */
	.long target_reserved6               /* `-' */
	.long target_reserved7               /* `-' */
	.long invop
	.long invop
	.long invop
	.long target_stack                   /* `stack #<imm8>' */
	.long target_static                  /* `static <imm8>' */
	.long target_extern                  /* `extern <imm8>:<imm8>' */
	.long target_global                  /* `global <imm8>' */
	.long target_local                   /* `local <imm8>' */
END(inst_table)
inst_table_f0:
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long target_endcatch_n              /* `end catch, #<imm8>+1' */
	.long target_endfinally_n            /* `end finally, #<imm8>+1' */
	.long target16_call_kw               /* `call top, #<imm8>, const <imm16>' */
	.long target16_call_tuple_kw         /* `call top, pop..., const <imm16>' */
	.long invop
	.long invop
	.long target16_push_bnd_arg          /* `push bound arg <imm8>' */
	.long target16_push_bnd_extern       /* `push bound extern <imm16>:<imm16>' */
	.long target16_push_bnd_global       /* `push bound global <imm16>' */
	.long target16_push_bnd_local        /* `push bound local <imm16>' */
	.long invop
	.long invop
	.long invop
	.long invop
	.long target32_jmp                   /* `jmp <Sdisp32>' */
	.long invop
	.long invop
	.long invop
	.long target_jmp_pop_pop             /* `jmp pop, #pop' */
	.long target16_operator              /* `op top, $<imm16>, #<imm8>' */
	.long target16_operator_tuple        /* `op top, $<imm16>, pop' */
	.long target_call_seq                /* `call top, [#<imm8>]' */
	.long target_call_map                /* `call top, {#<imm8>*2}' */
	.long target_thiscall_tuple          /* `call top, pop, pop...' */
	.long target16_del_global            /* `del global <imm16>' */
	.long target16_del_local             /* `del local <imm16>' */
	.long target_call_tuple_kwds         /* `call top, pop..., pop' */
	.long target16_lrot                  /* `lrot #<imm16>+3' */
	.long target16_rrot                  /* `rrot #<imm16>+3' */
	.long invop
	.long target16_dup_n                 /* `dup #SP - <imm16> - 2' */
	.long invop
	.long target16_pop_n                 /* `pop #SP - <imm16> - 2' */
	.long target16_adjstack              /* `adjstack #SP +/- <Simm16>' */
	.long invop
	.long target16_super_this_r          /* `push super this, ref <imm16>' */
	.long invop
	.long invop
	.long target16_pop_static            /* `pop static <imm16>' */
	.long target16_pop_extern            /* `pop extern <imm16>:<imm16>' */
	.long target16_pop_global            /* `pop global <imm16>' */
	.long target16_pop_local             /* `pop local <imm16>' */
	.long invop
	.long invop
	.long invop
	.long invop
	.long target_push_except             /* `push except' */
	.long target_push_this               /* `push this' */
	.long target_push_this_module        /* `push this_module' */
	.long target_push_this_function      /* `push this_function' */
	.long target16_push_module           /* `push module <imm16>' */
	.long target16_push_arg              /* `push arg <imm16>' */
	.long target16_push_const            /* `push const <imm16>' */
	.long target16_push_ref              /* `push ref <imm16>' */
	.long target16_push_static           /* `push static <imm16>' */
	.long target16_push_extern           /* `push extern <imm16>:<imm16>' */
	.long target16_push_global           /* `push global <imm16>' */
	.long target16_push_local            /* `push local <imm16>' */
	.long target_cast_hashset            /* `cast top, HashSet' */
	.long target_cast_dict               /* `cast top, Dict' */
	.long target16_pack_tuple            /* `push pack Tuple, #<imm16>' */
	.long target16_pack_list             /* `push pack List, #<imm16>' */
	.long invop
	.long invop
	.long target16_unpack                /* `unpack pop, #<imm16>' */
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long target_push_true               /* `push true' */
	.long target_push_false              /* `push false' */
	.long target_pack_hashset            /* `push pack HashSet, #<imm8>' */
	.long target_pack_dict               /* `push pack Dict, #<imm8>*2' */
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long target_bounditem               /* `bounditem top, pop' */
	.long target16_getattr_c             /* `getattr top, const <imm16>' */
	.long target16_delattr_c             /* `delattr pop, const <imm16>' */
	.long target16_setattr_c             /* `setattr pop, const <imm16>, pop' */
	.long target16_getattr_this_c        /* `push getattr this, const <imm16>' */
	.long target16_delattr_this_c        /* `delattr this, const <imm16>' */
	.long target16_setattr_this_c        /* `setattr this, const <imm16>, pop' */
	.long target_cmp_so                  /* `cmp so, top, pop' */
	.long target_cmp_do                  /* `cmp do, top, pop' */
	.long target16_pack_hashset          /* `push pack HashSet, #<imm16>' */
	.long target16_pack_dict             /* `push pack Dict, #<imm16>*2' */
	.long target16_getcmember            /* `getcmember top, $<imm16>' */
	.long target_class                   /* `class top, pop' */
	.long target16_class_c               /* `class top, const <imm16>' */
	.long target16_class_gc              /* `push class global <imm16>, const <imm16>' */
	.long target16_class_ec              /* `push class extern <imm16>:<imm16>, const <imm16>' */
	.long target16_defcmember            /* `defcmember top, $<imm16>, pop' */
	.long target16_getcmember_r          /* `push getcmember ref <imm16>, $<imm16>' */
	.long target16_callcmember_this_r    /* `push callcmember this, ref <imm16>, $<imm16>, #<imm8>' */
	.long invop
	.long invop
	.long target16_function_c            /* `push function const <imm16>, #<imm8>+1' */
	.long target16_function_c_16         /* `push function const <imm16>, #<imm16>+1' */
	.long target_supergetattr_this_rc    /* `push getattr this, ref <imm8>, const <imm8>' */
	.long target16_supergetattr_this_rc  /* `push getattr this, ref <imm16>, const <imm16>' */
	.long target_supercallattr_this_rc   /* `push callattr this, ref <imm8>, const <imm8>, #<imm8>' */
	.long target16_supercallattr_this_rc /* `push callattr this, ref <imm16>, const <imm16>, #<imm8>' */
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long target_incpost                 /* `push inc' */
	.long target_decpost                 /* `push dec' */
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long target16_delop                 /* `-' */
	.long target16_nop                   /* `nop16' */
	.long target_reduce_min              /* `reduce top, min' */
	.long target_reduce_max              /* `reduce top, max' */
	.long target_reduce_sum              /* `reduce top, sum' */
	.long target_reduce_any              /* `reduce top, any' */
	.long target_reduce_all              /* `reduce top, all' */
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long target16_print_c               /* `print const <imm16>' */
	.long target16_print_c_sp            /* `print const <imm16>, sp' */
	.long target16_print_c_nl            /* `print const <imm16>, nl' */
	.long target_range_0_i32             /* `push range $0, $<imm32>' */
	.long invop
	.long target_varargs_unpack          /* `unpack varargs, #<imm8>' */
	.long target_push_varkwds_ne         /* `push bool varkwds' */
	.long invop
	.long target16_fprint_c              /* `print top, const <imm16>' */
	.long target16_fprint_c_sp           /* `print top, const <imm16>, sp' */
	.long target16_fprint_c_nl           /* `print top, const <imm16>, nl' */
	.long target_varargs_cmp_eq_sz       /* `push cmp eq, #varargs, $<imm8>' */
	.long target_varargs_cmp_gr_sz       /* `push cmp gr, #varargs, $<imm8>' */
	.long invop
	.long invop
	.long invop
	.long target16_contains_c            /* `push contains const <imm16>, pop' */
	.long target_varargs_getitem         /* `getitem varargs, top' */
	.long target_varargs_getitem_i       /* `push getitem varargs, $<imm8>' */
	.long target16_getitem_c             /* `getitem top, const <imm16>' */
	.long target_varargs_getsize         /* `push getsize varargs' */
	.long invop
	.long invop
	.long target16_setitem_c             /* `setitem pop, const <imm16>, pop' */
	.long target_iternext                /* `iternext top' */
	.long invop
	.long invop
	.long invop
	.long invop
	.long target_getmember               /* `getmember top, pop, $<imm8>' */
	.long target16_getmember             /* `getmember top, pop, $<imm16>' */
	.long target_delmember               /* `delmember pop, pop, $<imm8>' */
	.long target16_delmember             /* `delmember pop, pop, $<imm16>' */
	.long target_setmember               /* `setmember pop, pop, $<imm8>, pop' */
	.long target16_setmember             /* `setmember pop, pop, $<imm16>, pop' */
	.long target_boundmember             /* `boundmember top, pop, $<imm8>' */
	.long target16_boundmember           /* `boundmember top, pop, $<imm16>' */
	.long target_getmember_this          /* `push getmember this, pop, $<imm8>' */
	.long target16_getmember_this        /* `push getmember this, pop, $<imm16>' */
	.long target_delmember_this          /* `delmember this, pop, $<imm8>' */
	.long target16_delmember_this        /* `delmember this, pop, $<imm16>' */
	.long target_setmember_this          /* `setmember this, pop, $<imm8>, pop' */
	.long target16_setmember_this        /* `setmember this, pop, $<imm16>, pop' */
	.long target_boundmember_this        /* `push boundmember this, pop, $<imm8>' */
	.long target16_boundmember_this      /* `push boundmember this, pop, $<imm16>' */
	.long target16_callattr_c_kw         /* `callattr top, const <imm16>, #<imm8>, const <imm16>' */
	.long target16_callattr_c_tuple_kw   /* `callattr top, const <imm16>, pop..., const <imm16>' */
	.long target_callattr_kwds           /* `callattr top, pop, #<imm8>, pop' */
	.long target_callattr_tuple_kwds     /* `callattr top, pop, pop..., pop' */
	.long target16_callattr_c            /* `callattr top, const <imm16>, #<imm8>' */
	.long target16_callattr_c_tuple      /* `callattr top, const <imm16>, pop' */
	.long target16_callattr_this_c       /* `callattr this, const <imm16>, #<imm8>' */
	.long target16_callattr_this_c_tuple /* `callattr this, const <imm16>, pop' */
	.long target16_callattr_c_seq        /* `callattr top, const <imm16>, [#<imm8>]' */
	.long target16_callattr_c_map        /* `callattr top, const <imm16>, {#<imm8>*2}' */
	.long invop
	.long target16_getmember_this_r      /* `push getmember this, ref <imm16>, $<imm16>' */
	.long target16_delmember_this_r      /* `delmember this, ref <imm16>, $<imm16>' */
	.long target16_setmember_this_r      /* `setmember this, ref <imm16>, $<imm16>, pop' */
	.long target16_boundmember_this_r    /* `push boundmember this, ref <imm16>, $<imm16>' */
	.long target16_call_extern           /* `push call extern <imm16>:<imm16>, #<imm8>' */
	.long target16_call_global           /* `push call global <imm16>, #<imm8>' */
	.long target16_call_local            /* `push call local <imm16>, #<imm8>' */
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long invop
	.long target16_stack                 /* `stack #<imm16>' */
	.long target16_static                /* `static <imm16>' */
	.long target16_extern                /* `extern <imm16>:<imm16>' */
	.long target16_global                /* `global <imm16>' */
	.long target16_local                 /* `local  <imm16>' */
END(inst_table_f0)
prefix_inst_table:
	.long prefix_target_ret_none                /* `ret' */
	.long prefix_target_ret                     /* `ret pop' */
	.long prefix_target_yieldall                /* `yield foreach, pop' */
	.long prefix_target_throw                   /* `throw pop' */
	.long prefix_target_rethrow                 /* `throw except' */
	.long prefix_target_setret                  /* `setret pop' */
	.long prefix_target_endcatch                /* `end catch' */
	.long prefix_target_endfinally              /* `end finally' */
	.long prefix_target_call_kw                 /* `call top, #<imm8>, const <imm8>' */
	.long prefix_target_call_tuple_kw           /* `call top, pop..., const <imm8>' */
	.long prefix_invop
	.long prefix_invop
	.long prefix_target_push_bnd_arg            /* `push bound arg <imm8>' */
	.long prefix_target_push_bnd_extern         /* `push bound extern <imm8>:<imm8>' */
	.long prefix_target_push_bnd_global         /* `push bound global <imm8>' */
	.long prefix_target_push_bnd_local          /* `push bound local <imm8>' */
	.long prefix_target_jf                      /* `jf pop, <Sdisp8>' */
	.long prefix_target_jf16                    /* `jf pop, <Sdisp16>' */
	.long prefix_target_jt                      /* `jt pop, <Sdisp8>' */
	.long prefix_target_jt16                    /* `jt pop, <Sdisp16>' */
	.long prefix_target_jmp                     /* `jmp <Sdisp8>' */
	.long prefix_target_jmp16                   /* `jmp <Sdisp16>' */
	.long prefix_target_foreach                 /* `foreach top, <Sdisp8>' */
	.long prefix_target_foreach16               /* `foreach top, <Sdisp16>' */
	.long prefix_target_jmp_pop                 /* `jmp pop' */
	.long prefix_target_operator                /* `op top, $<imm8>, #<imm8>' */
	.long prefix_target_operator_tuple          /* `op top, $<imm8>, pop...' */
	.long prefix_target_call                    /* `call top, #<imm8>' */
	.long prefix_target_call_tuple              /* `call top, pop...' */
	.long prefix_invop
	.long prefix_target_del_global              /* `del global <imm8>' */
	.long prefix_target_del_local               /* `del local <imm8>' */
	.long prefix_target_swap                    /* `swap' */
	.long prefix_target_lrot                    /* `lrot #<imm8>+3' */
	.long prefix_target_rrot                    /* `rrot #<imm8>+3' */
	.long prefix_target_dup                     /* `dup' */
	.long prefix_target_dup_n                   /* `dup #SP - <imm8> - 2' */
	.long prefix_target_pop                     /* `pop' */
	.long prefix_target_pop_n                   /* `pop #SP - <imm8> - 2' */
	.long prefix_target_adjstack                /* `adjstack #SP +/- <Simm8>' */
	.long prefix_target_super                   /* `super top, pop' */
	.long prefix_target_super_this_r            /* `push super this, ref <imm8>' */
	.long prefix_invop
	.long prefix_invop
	.long prefix_target_pop_static              /* `pop static <imm8>' */
	.long prefix_target_pop_extern              /* `pop extern <imm8>:<imm8>' */
	.long prefix_target_pop_global              /* `pop global <imm8>' */
	.long prefix_target_pop_local               /* `pop local <imm8>' */
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_target_push_none               /* `push none' */
	.long prefix_invop
	.long prefix_invop
	.long prefix_target_push_varargs            /* `push varargs' */
	.long prefix_target_push_varkwds            /* `push varkwds' */
	.long prefix_target_push_module             /* `push module <imm8>' */
	.long prefix_target_push_arg                /* `push arg <imm8>' */
	.long prefix_target_push_const              /* `push const <imm8>' */
	.long prefix_target_push_ref                /* `push ref <imm8>' */
	.long prefix_target_push_static             /* `push static <imm8>' */
	.long prefix_target_push_extern             /* `push extern <imm8>:<imm8>' */
	.long prefix_target_push_global             /* `push global <imm8>' */
	.long prefix_target_push_local              /* `push local <imm8>' */
	.long prefix_target_cast_tuple              /* `cast top, Tuple' */
	.long prefix_target_cast_list               /* `cast top, List' */
	.long prefix_target_pack_tuple              /* `push pack Tuple, #<imm8>' */
	.long prefix_target_pack_list               /* `push pack List, #<imm8>' */
	.long prefix_invop
	.long prefix_invop
	.long prefix_target_unpack                  /* `unpack pop, #<imm8>' */
	.long prefix_target_concat                  /* `concat top, pop' */
	.long prefix_target_extend                  /* `extend top, #<imm8>' */
	.long prefix_target_typeof                  /* `typeof top' */
	.long prefix_target_classof                 /* `classof top' */
	.long prefix_target_superof                 /* `superof top' */
	.long prefix_target_instanceof              /* `instanceof top, pop' */
	.long prefix_target_str                     /* `str top' */
	.long prefix_target_repr                    /* `repr top' */
	.long prefix_invop
	.long prefix_target_bool                    /* `bool top' */
	.long prefix_target_not                     /* `not top' */
	.long prefix_target_assign                  /* `assign pop, pop' */
	.long prefix_target_move_assign             /* `assign move, pop, pop' */
	.long prefix_target_copy                    /* `copy top' */
	.long prefix_target_deepcopy                /* `deepcopy top' */
	.long prefix_target_getattr                 /* `getattr top, pop' */
	.long prefix_target_delattr                 /* `delattr pop, pop' */
	.long prefix_target_setattr                 /* `setattr pop, pop, pop' */
	.long prefix_target_boundattr               /* `boundattr top, pop' */
	.long prefix_target_getattr_c               /* `getattr top, const <imm8>' */
	.long prefix_target_delattr_c               /* `delattr pop, const <imm8>' */
	.long prefix_target_setattr_c               /* `setattr pop, const <imm8>, pop' */
	.long prefix_target_getattr_this_c          /* `push getattr this, const <imm8>' */
	.long prefix_target_delattr_this_c          /* `delattr this, const <imm8>' */
	.long prefix_target_setattr_this_c          /* `setattr this, const <imm8>, pop' */
	.long prefix_target_cmp_eq                  /* `cmp eq, top, pop' */
	.long prefix_target_cmp_ne                  /* `cmp ne, top, pop' */
	.long prefix_target_cmp_lo                  /* `cmp lo, top, pop' */
	.long prefix_target_cmp_le                  /* `cmp le, top, pop' */
	.long prefix_target_cmp_gr                  /* `cmp gr, top, pop' */
	.long prefix_target_cmp_ge                  /* `cmp ge, top, pop' */
	.long prefix_target_class_c                 /* `class top, const <imm8>' */
	.long prefix_target_class_gc                /* `push class global <imm8>, const <imm8>' */
	.long prefix_target_class_ec                /* `push class extern <imm8>:<imm8>, const <imm8>' */
	.long prefix_target_defcmember              /* `defcmember top, $<imm8>, pop' */
	.long prefix_target_getcmember_r            /* `push getcmember ref <imm8>, $<imm8>' */
	.long prefix_target_callcmember_this_r      /* `push callcmember this, ref <imm8>, $<imm8>, #<imm8>' */
	.long prefix_invop
	.long prefix_invop
	.long prefix_target_function_c              /* `push function const <imm8>, #<imm8>+1' */
	.long prefix_target_function_c_16           /* `push function const <imm8>, #<imm16>+1' */
	.long prefix_target_cast_int                /* `cast top, int' */
	.long prefix_target_inv                     /* `inv top' */
	.long prefix_target_pos                     /* `pos top' */
	.long prefix_target_neg                     /* `neg top' */
	.long prefix_target_add                     /* `add top, pop' */
	.long prefix_target_sub                     /* `sub top, pop' */
	.long prefix_target_mul                     /* `mul top, pop' */
	.long prefix_target_div                     /* `div top, pop' */
	.long prefix_target_mod                     /* `mod top, pop' */
	.long prefix_target_shl                     /* `shl top, pop' */
	.long prefix_target_shr                     /* `shr top, pop' */
	.long prefix_target_and                     /* `and top, pop' */
	.long prefix_target_or                      /* `or top, pop' */
	.long prefix_target_xor                     /* `xor top, pop' */
	.long prefix_target_pow                     /* `pow top, pop' */
	.long prefix_target_inc                     /* `inc' */
	.long prefix_target_dec                     /* `dec' */
	.long prefix_target_add_simm8               /* `add top, $<Simm8>' */
	.long prefix_target_add_imm32               /* `add top, $<imm32>' */
	.long prefix_target_sub_simm8               /* `sub top, $<Simm8>' */
	.long prefix_target_sub_imm32               /* `sub top, $<imm32>' */
	.long prefix_target_mul_simm8               /* `mul top, $<Simm8>' */
	.long prefix_target_div_simm8               /* `div top, $<Simm8>' */
	.long prefix_target_mod_simm8               /* `mod top, $<Simm8>' */
	.long prefix_target_shl_imm8                /* `shl top, $<imm8>' */
	.long prefix_target_shr_imm8                /* `shr top, $<imm8>' */
	.long prefix_target_and_imm32               /* `and top, $<imm32>' */
	.long prefix_target_or_imm32                /* `or top, $<imm32>' */
	.long prefix_target_xor_imm32               /* `xor top, $<imm32>' */
	.long prefix_target_isnone                  /* `instanceof top, none' */
	.long prefix_invop
	.long prefix_target_delop                   /* `-' */
	.long prefix_target_nop                     /* `nop' */
	.long prefix_target_print                   /* `print pop' */
	.long prefix_target_print_sp                /* `print pop, sp' */
	.long prefix_target_print_nl                /* `print pop, nl' */
	.long prefix_target_printnl                 /* `print nl' */
	.long prefix_target_printall                /* `print pop...' */
	.long prefix_target_printall_sp             /* `print pop..., sp' */
	.long prefix_target_printall_nl             /* `print pop..., nl' */
	.long prefix_invop
	.long prefix_target_fprint                  /* `print top, pop' */
	.long prefix_target_fprint_sp               /* `print top, pop, sp' */
	.long prefix_target_fprint_nl               /* `print top, pop, nl' */
	.long prefix_target_fprintnl                /* `print top, nl' */
	.long prefix_target_fprintall               /* `print top, pop...' */
	.long prefix_target_fprintall_sp            /* `print top, pop..., sp' */
	.long prefix_target_fprintall_nl            /* `print top, pop..., nl' */
	.long prefix_invop
	.long prefix_target_print_c                 /* `print const <imm8>' */
	.long prefix_target_print_c_sp              /* `print const <imm8>, sp' */
	.long prefix_target_print_c_nl              /* `print const <imm8>, nl' */
	.long prefix_target_range_0_i16             /* `push range $0, $<imm16>' */
	.long prefix_invop
	.long prefix_target_enter                   /* `enter top' */
	.long prefix_target_leave                   /* `leave pop' */
	.long prefix_invop
	.long prefix_target_fprint_c                /* `print top, const <imm8>' */
	.long prefix_target_fprint_c_sp             /* `print top, const <imm8>, sp' */
	.long prefix_target_fprint_c_nl             /* `print top, const <imm8>, nl' */
	.long prefix_target_range                   /* `range top, pop' */
	.long prefix_target_range_def               /* `push range default, pop' */
	.long prefix_target_range_step              /* `range top, pop, pop' */
	.long prefix_target_range_step_def          /* `push range default, pop, pop' */
	.long prefix_target_contains                /* `contains top, pop' */
	.long prefix_target_contains_c              /* `push contains const <imm8>, pop' */
	.long prefix_target_getitem                 /* `getitem top, pop' */
	.long prefix_target_getitem_i               /* `getitem top, $<Simm16>' */
	.long prefix_target_getitem_c               /* `getitem top, const <imm8>' */
	.long prefix_target_getsize                 /* `getsize top' */
	.long prefix_target_setitem                 /* `setitem pop, pop, pop' */
	.long prefix_target_setitem_i               /* `setitem pop, $<Simm16>, pop' */
	.long prefix_target_setitem_c               /* `setitem pop, const <imm8>, pop' */
	.long prefix_target_iterself                /* `iterself top' */
	.long prefix_target_delitem                 /* `delitem pop, pop' */
	.long prefix_target_getrange                /* `getrange top, pop, pop' */
	.long prefix_target_getrange_pn             /* `getrange top, pop, none' */
	.long prefix_target_getrange_np             /* `getrange top, none, pop' */
	.long prefix_target_getrange_pi             /* `getrange top, pop, $<Simm16>' */
	.long prefix_target_getrange_ip             /* `getrange top, $<Simm16>, pop' */
	.long prefix_target_getrange_ni             /* `getrange top, none, $<Simm16>' */
	.long prefix_target_getrange_in             /* `getrange top, $<Simm16>, none' */
	.long prefix_target_getrange_ii             /* `getrange top, $<Simm16>, $<Simm16>' */
	.long prefix_target_delrange                /* `delrange pop, pop, pop' */
	.long prefix_target_setrange                /* `setrange pop, pop, pop, pop' */
	.long prefix_target_setrange_pn             /* `setrange pop, pop, none, pop' */
	.long prefix_target_setrange_np             /* `setrange pop, none, pop, pop' */
	.long prefix_target_setrange_pi             /* `setrange pop, pop, $<Simm16>, pop' */
	.long prefix_target_setrange_ip             /* `setrange pop, $<Simm16>, pop, pop' */
	.long prefix_target_setrange_ni             /* `setrange pop, none, $<Simm16>, pop' */
	.long prefix_target_setrange_in             /* `setrange pop, $<Simm16>, none, pop' */
	.long prefix_target_setrange_ii             /* `setrange pop, $<Simm16>, $<Simm16>, pop' */
	.long prefix_target_breakpoint              /* `debug break' */
	.long prefix_target_ud                      /* `ud' */
	.long prefix_target_callattr_c_kw           /* `callattr top, const <imm8>, #<imm8>, const <imm8>' */
	.long prefix_target_callattr_c_tuple_kw     /* `callattr top, const <imm8>, pop..., const <imm8>' */
	.long prefix_target_callattr                /* `callattr top, pop, #<imm8>' */
	.long prefix_target_callattr_tuple          /* `callattr top, pop, pop...' */
	.long prefix_target_callattr_c              /* `callattr top, const <imm8>, #<imm8>' */
	.long prefix_target_callattr_c_tuple        /* `callattr top, const <imm8>, pop...' */
	.long prefix_target_callattr_this_c         /* `push callattr this, const <imm8>, #<imm8>' */
	.long prefix_target_callattr_this_c_tuple   /* `push callattr this, const <imm8>, pop...' */
	.long prefix_target_callattr_c_seq          /* `callattr top, const <imm8>, [#<imm8>]' */
	.long prefix_target_callattr_c_map          /* `callattr top, const <imm8>, {#<imm8>*2}' */
	.long prefix_invop
	.long prefix_target_getmember_this_r        /* `push getmember this, ref <imm8>, $<imm8>' */
	.long prefix_target_delmember_this_r        /* `delmember this, ref <imm8>, $<imm8>' */
	.long prefix_target_setmember_this_r        /* `setmember this, ref <imm8>, $<imm8>, pop' */
	.long prefix_target_boundmember_this_r      /* `push boundmember this, ref <imm8>, $<imm8>' */
	.long prefix_target_call_extern             /* `push call extern <imm8>:<imm8>, #<imm8>' */
	.long prefix_target_call_global             /* `push call global <imm8>, #<imm8>' */
	.long prefix_target_call_local              /* `push call local <imm8>, #<imm8>' */
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_target_extended1               /* `-' */
	.long prefix_target_reserved1               /* `-' */
	.long prefix_target_reserved2               /* `-' */
	.long prefix_target_reserved3               /* `-' */
	.long prefix_target_reserved4               /* `-' */
	.long prefix_target_reserved5               /* `-' */
	.long prefix_target_reserved6               /* `-' */
	.long prefix_target_reserved7               /* `-' */
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_target_stack                   /* `stack #<imm8>' */
	.long prefix_target_static                  /* `static <imm8>' */
	.long prefix_target_extern                  /* `extern <imm8>:<imm8>' */
	.long prefix_target_global                  /* `global <imm8>' */
	.long prefix_target_local                   /* `local <imm8>' */
END(prefix_inst_table)
prefix_inst_table_f0:
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_target_endcatch_n              /* `end catch, #<imm8>+1' */
	.long prefix_target_endfinally_n            /* `end finally, #<imm8>+1' */
	.long prefix_target16_call_kw               /* `call top, #<imm8>, const <imm16>' */
	.long prefix_target16_call_tuple_kw         /* `call top, pop..., const <imm16>' */
	.long prefix_invop
	.long prefix_invop
	.long prefix_target16_push_bnd_arg          /* `push bound arg <imm8>' */
	.long prefix_target16_push_bnd_extern       /* `push bound extern <imm16>:<imm16>' */
	.long prefix_target16_push_bnd_global       /* `push bound global <imm16>' */
	.long prefix_target16_push_bnd_local        /* `push bound local <imm16>' */
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_target32_jmp                   /* `jmp <Sdisp32>' */
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_target_jmp_pop_pop             /* `jmp pop, #pop' */
	.long prefix_target16_operator              /* `op top, $<imm16>, #<imm8>' */
	.long prefix_target16_operator_tuple        /* `op top, $<imm16>, pop' */
	.long prefix_target_call_seq                /* `call top, [#<imm8>]' */
	.long prefix_target_call_map                /* `call top, {#<imm8>*2}' */
	.long prefix_target_thiscall_tuple          /* `call top, pop, pop...' */
	.long prefix_target16_del_global            /* `del global <imm16>' */
	.long prefix_target16_del_local             /* `del local <imm16>' */
	.long prefix_target_call_tuple_kwds         /* `call top, pop..., pop' */
	.long prefix_target16_lrot                  /* `lrot #<imm16>+3' */
	.long prefix_target16_rrot                  /* `rrot #<imm16>+3' */
	.long prefix_invop
	.long prefix_target16_dup_n                 /* `dup #SP - <imm16> - 2' */
	.long prefix_invop
	.long prefix_target16_pop_n                 /* `pop #SP - <imm16> - 2' */
	.long prefix_target16_adjstack              /* `adjstack #SP +/- <Simm16>' */
	.long prefix_invop
	.long prefix_target16_super_this_r          /* `push super this, ref <imm16>' */
	.long prefix_invop
	.long prefix_invop
	.long prefix_target16_pop_static            /* `pop static <imm16>' */
	.long prefix_target16_pop_extern            /* `pop extern <imm16>:<imm16>' */
	.long prefix_target16_pop_global            /* `pop global <imm16>' */
	.long prefix_target16_pop_local             /* `pop local <imm16>' */
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_target_push_except             /* `push except' */
	.long prefix_target_push_this               /* `push this' */
	.long prefix_target_push_this_module        /* `push this_module' */
	.long prefix_target_push_this_function      /* `push this_function' */
	.long prefix_target16_push_module           /* `push module <imm16>' */
	.long prefix_target16_push_arg              /* `push arg <imm16>' */
	.long prefix_target16_push_const            /* `push const <imm16>' */
	.long prefix_target16_push_ref              /* `push ref <imm16>' */
	.long prefix_target16_push_static           /* `push static <imm16>' */
	.long prefix_target16_push_extern           /* `push extern <imm16>:<imm16>' */
	.long prefix_target16_push_global           /* `push global <imm16>' */
	.long prefix_target16_push_local            /* `push local <imm16>' */
	.long prefix_target_cast_hashset            /* `cast top, HashSet' */
	.long prefix_target_cast_dict               /* `cast top, Dict' */
	.long prefix_target16_pack_tuple            /* `push pack Tuple, #<imm16>' */
	.long prefix_target16_pack_list             /* `push pack List, #<imm16>' */
	.long prefix_invop
	.long prefix_invop
	.long prefix_target16_unpack                /* `unpack pop, #<imm16>' */
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_target_push_true               /* `push true' */
	.long prefix_target_push_false              /* `push false' */
	.long prefix_target_pack_hashset            /* `push pack HashSet, #<imm8>' */
	.long prefix_target_pack_dict               /* `push pack Dict, #<imm8>*2' */
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_target_bounditem               /* `bounditem top, pop' */
	.long prefix_target16_getattr_c             /* `getattr top, const <imm16>' */
	.long prefix_target16_delattr_c             /* `delattr pop, const <imm16>' */
	.long prefix_target16_setattr_c             /* `setattr pop, const <imm16>, pop' */
	.long prefix_target16_getattr_this_c        /* `push getattr this, const <imm16>' */
	.long prefix_target16_delattr_this_c        /* `delattr this, const <imm16>' */
	.long prefix_target16_setattr_this_c        /* `setattr this, const <imm16>, pop' */
	.long prefix_target_cmp_so                  /* `cmp so, top, pop' */
	.long prefix_target_cmp_do                  /* `cmp do, top, pop' */
	.long prefix_target16_pack_hashset          /* `push pack HashSet, #<imm16>' */
	.long prefix_target16_pack_dict             /* `push pack Dict, #<imm16>*2' */
	.long prefix_target16_getcmember            /* `getcmember top, $<imm16>' */
	.long prefix_target_class                   /* `class top, pop' */
	.long prefix_target16_class_c               /* `class top, const <imm16>' */
	.long prefix_target16_class_gc              /* `push class global <imm16>, const <imm16>' */
	.long prefix_target16_class_ec              /* `push class extern <imm16>:<imm16>, const <imm16>' */
	.long prefix_target16_defcmember            /* `defcmember top, $<imm16>, pop' */
	.long prefix_target16_getcmember_r          /* `push getcmember ref <imm16>, $<imm16>' */
	.long prefix_target16_callcmember_this_r    /* `push callcmember this, ref <imm16>, $<imm16>, #<imm8>' */
	.long prefix_invop
	.long prefix_invop
	.long prefix_target16_function_c            /* `push function const <imm16>, #<imm8>+1' */
	.long prefix_target16_function_c_16         /* `push function const <imm16>, #<imm16>+1' */
	.long prefix_target_supergetattr_this_rc    /* `push getattr this, ref <imm8>, const <imm8>' */
	.long prefix_target16_supergetattr_this_rc  /* `push getattr this, ref <imm16>, const <imm16>' */
	.long prefix_target_supercallattr_this_rc   /* `push callattr this, ref <imm8>, const <imm8>, #<imm8>' */
	.long prefix_target16_supercallattr_this_rc /* `push callattr this, ref <imm16>, const <imm16>, #<imm8>' */
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_target_incpost                 /* `push inc' */
	.long prefix_target_decpost                 /* `push dec' */
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_target16_delop                 /* `-' */
	.long prefix_target16_nop                   /* `nop16' */
	.long prefix_target_reduce_min              /* `reduce top, min' */
	.long prefix_target_reduce_max              /* `reduce top, max' */
	.long prefix_target_reduce_sum              /* `reduce top, sum' */
	.long prefix_target_reduce_any              /* `reduce top, any' */
	.long prefix_target_reduce_all              /* `reduce top, all' */
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_target16_print_c               /* `print const <imm16>' */
	.long prefix_target16_print_c_sp            /* `print const <imm16>, sp' */
	.long prefix_target16_print_c_nl            /* `print const <imm16>, nl' */
	.long prefix_target_range_0_i32             /* `push range $0, $<imm32>' */
	.long prefix_invop
	.long prefix_target_varargs_unpack          /* `unpack varargs, #<imm8>' */
	.long prefix_target_push_varkwds_ne         /* `push bool varkwds' */
	.long prefix_invop
	.long prefix_target16_fprint_c              /* `print top, const <imm16>' */
	.long prefix_target16_fprint_c_sp           /* `print top, const <imm16>, sp' */
	.long prefix_target16_fprint_c_nl           /* `print top, const <imm16>, nl' */
	.long prefix_target_varargs_cmp_eq_sz       /* `push cmp eq, #varargs, $<imm8>' */
	.long prefix_target_varargs_cmp_gr_sz       /* `push cmp gr, #varargs, $<imm8>' */
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_target16_contains_c            /* `push contains const <imm16>, pop' */
	.long prefix_target_varargs_getitem         /* `getitem varargs, top' */
	.long prefix_target_varargs_getitem_i       /* `push getitem varargs, $<imm8>' */
	.long prefix_target16_getitem_c             /* `getitem top, const <imm16>' */
	.long prefix_target_varargs_getsize         /* `push getsize varargs' */
	.long prefix_invop
	.long prefix_invop
	.long prefix_target16_setitem_c             /* `setitem pop, const <imm16>, pop' */
	.long prefix_target_iternext                /* `iternext top' */
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_target_getmember               /* `getmember top, pop, $<imm8>' */
	.long prefix_target16_getmember             /* `getmember top, pop, $<imm16>' */
	.long prefix_target_delmember               /* `delmember pop, pop, $<imm8>' */
	.long prefix_target16_delmember             /* `delmember pop, pop, $<imm16>' */
	.long prefix_target_setmember               /* `setmember pop, pop, $<imm8>, pop' */
	.long prefix_target16_setmember             /* `setmember pop, pop, $<imm16>, pop' */
	.long prefix_target_boundmember             /* `boundmember top, pop, $<imm8>' */
	.long prefix_target16_boundmember           /* `boundmember top, pop, $<imm16>' */
	.long prefix_target_getmember_this          /* `push getmember this, pop, $<imm8>' */
	.long prefix_target16_getmember_this        /* `push getmember this, pop, $<imm16>' */
	.long prefix_target_delmember_this          /* `delmember this, pop, $<imm8>' */
	.long prefix_target16_delmember_this        /* `delmember this, pop, $<imm16>' */
	.long prefix_target_setmember_this          /* `setmember this, pop, $<imm8>, pop' */
	.long prefix_target16_setmember_this        /* `setmember this, pop, $<imm16>, pop' */
	.long prefix_target_boundmember_this        /* `push boundmember this, pop, $<imm8>' */
	.long prefix_target16_boundmember_this      /* `push boundmember this, pop, $<imm16>' */
	.long prefix_target16_callattr_c_kw         /* `callattr top, const <imm16>, #<imm8>, const <imm16>' */
	.long prefix_target16_callattr_c_tuple_kw   /* `callattr top, const <imm16>, pop..., const <imm16>' */
	.long prefix_target_callattr_kwds           /* `callattr top, pop, #<imm8>, pop' */
	.long prefix_target_callattr_tuple_kwds     /* `callattr top, pop, pop..., pop' */
	.long prefix_target16_callattr_c            /* `callattr top, const <imm16>, #<imm8>' */
	.long prefix_target16_callattr_c_tuple      /* `callattr top, const <imm16>, pop' */
	.long prefix_target16_callattr_this_c       /* `callattr this, const <imm16>, #<imm8>' */
	.long prefix_target16_callattr_this_c_tuple /* `callattr this, const <imm16>, pop' */
	.long prefix_target16_callattr_c_seq        /* `callattr top, const <imm16>, [#<imm8>]' */
	.long prefix_target16_callattr_c_map        /* `callattr top, const <imm16>, {#<imm8>*2}' */
	.long prefix_invop
	.long prefix_target16_getmember_this_r      /* `push getmember this, ref <imm16>, $<imm16>' */
	.long prefix_target16_delmember_this_r      /* `delmember this, ref <imm16>, $<imm16>' */
	.long prefix_target16_setmember_this_r      /* `setmember this, ref <imm16>, $<imm16>, pop' */
	.long prefix_target16_boundmember_this_r    /* `push boundmember this, ref <imm16>, $<imm16>' */
	.long prefix_target16_call_extern           /* `push call extern <imm16>:<imm16>, #<imm8>' */
	.long prefix_target16_call_global           /* `push call global <imm16>, #<imm8>' */
	.long prefix_target16_call_local            /* `push call local <imm16>, #<imm8>' */
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_invop
	.long prefix_target16_stack                 /* `stack #<imm16>' */
	.long prefix_target16_static                /* `static <imm16>' */
	.long prefix_target16_extern                /* `extern <imm16>:<imm16>' */
	.long prefix_target16_global                /* `global <imm16>' */
	.long prefix_target16_local                 /* `local  <imm16>' */
END(prefix_inst_table_f0)
//[[[end]]]


#endif /* CONFIG_HAVE_EXEC_ASM */
