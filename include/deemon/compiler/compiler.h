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
#ifndef GUARD_DEEMON_COMPILER_COMPILER_H
#define GUARD_DEEMON_COMPILER_COMPILER_H 1

#include "../api.h"
#include "../object.h"
#include "../util/lock.h"
#include "../util/rlock.h"

#ifdef CONFIG_BUILDING_DEEMON
#include "symbol.h"
#include "error.h"
#include "lexer.h"
#include "tpp.h"
#endif /* CONFIG_BUILDING_DEEMON */

#include <hybrid/sequence/list.h>
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint16_t */

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_compiler_item_object      compiler_item_object
#define Dee_compiler_item_object_list compiler_item_object_list
#define Dee_compiler_wrapper_object   compiler_wrapper_object
#define Dee_compiler_object           compiler_object
#define Dee_compiler_options          compiler_options
#define Dee_compiler_items            compiler_items
#endif /* DEE_SOURCE */

typedef struct Dee_compiler_object DeeCompilerObject;
struct Dee_compiler_options;

#ifdef CONFIG_BUILDING_DEEMON
#define Dee_COMPILER_ITEM_OBJECT_HEAD(T)                                                                                    \
	Dee_OBJECT_HEAD                                                                                                         \
	DREF DeeCompilerObject              *ci_compiler; /* [1..1][const] The associated compiler. */                          \
	LIST_ENTRY(Dee_compiler_item_object) ci_link;     /* [1..1][lock(co_compiler->cp_item_lock)] Compiler item link. */     \
	T                                   *ci_value;    /* [0..1][lock(DeeCompiler_Lock)] Pointer to the associated object.   \
	                                                   * What exactly is pointed to depends on the compiler item type.      \
	                                                   * NOTE: For compiler items implemented as `DeeCompilerObjItem_Type', \
	                                                   *       this field is [DREF][1..1][const] */

typedef struct Dee_compiler_item_object DeeCompilerItemObject;
typedef struct Dee_compiler_wrapper_object DeeCompilerWrapperObject;
struct Dee_compiler_item_object {
	Dee_COMPILER_ITEM_OBJECT_HEAD(void)
};
struct Dee_compiler_wrapper_object {
	Dee_OBJECT_HEAD
	DREF DeeCompilerObject *cw_compiler; /* [1..1][const] The compiler being wrapped. */
};
#define Dee_COMPILER_ITEM_HASH(x) Dee_HashPointer((x)->ci_value)

INTDEF DeeTypeObject DeeCompilerItem_Type;
INTDEF DeeTypeObject DeeCompilerObjItem_Type;
INTDEF DeeTypeObject DeeCompilerWrapper_Type;

/* Construct and return a wrapper for a sub-component of the current compiler. */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeCompiler_GetWrapper(DeeCompilerObject *__restrict self,
                       DeeTypeObject *__restrict type);

/* Lookup or create a new compiler item for `value' */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeCompiler_GetItem(DeeTypeObject *__restrict type,
                    void *__restrict value);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeCompiler_GetObjItem(DeeTypeObject *__restrict type,
                       DeeObject *__restrict value);

/* Delete (clear) the compiler item associated with `value'. */
INTDEF bool DCALL DeeCompiler_DelItem(void *value);

/* Delete (clear) all compiler items matching the given `type'. */
INTDEF NONNULL((1)) size_t DCALL
DeeCompiler_DelItemType(DeeTypeObject *__restrict type);

/* Returns the value of a given compiler item.
 * @return: * :   A pointer to the item's value.
 * @return: NULL: The item got deleted (a ReferenceError was thrown) */
INTDEF NONNULL((1)) void *(DCALL DeeCompilerItem_GetValue)(DeeObject *__restrict self);
#define DeeCompilerItem_VALUE(self, T) \
	((T *)DeeCompilerItem_GetValue((DeeObject *)Dee_REQUIRES_OBJECT(self)))

LIST_HEAD(Dee_compiler_item_object_list, Dee_compiler_item_object);
struct Dee_compiler_items {
	size_t                                cis_size; /* [lock(cis_lock)] Amount of existing compiler items. */
	size_t                                cis_mask; /* [lock(cis_lock)] Hash-map mask. */
	struct Dee_compiler_item_object_list *cis_list; /* [0..1][0..cis_mask+1][owned][lock(cis_lock)] Hash-map of compiler items. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t                   cis_lock; /* Lock for compiler items. */
#endif /* !CONFIG_NO_THREADS */
};

#define Dee_compiler_items_lock_reading(self)    Dee_atomic_rwlock_reading(&(self)->cis_lock)
#define Dee_compiler_items_lock_writing(self)    Dee_atomic_rwlock_writing(&(self)->cis_lock)
#define Dee_compiler_items_lock_tryread(self)    Dee_atomic_rwlock_tryread(&(self)->cis_lock)
#define Dee_compiler_items_lock_trywrite(self)   Dee_atomic_rwlock_trywrite(&(self)->cis_lock)
#define Dee_compiler_items_lock_canread(self)    Dee_atomic_rwlock_canread(&(self)->cis_lock)
#define Dee_compiler_items_lock_canwrite(self)   Dee_atomic_rwlock_canwrite(&(self)->cis_lock)
#define Dee_compiler_items_lock_waitread(self)   Dee_atomic_rwlock_waitread(&(self)->cis_lock)
#define Dee_compiler_items_lock_waitwrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->cis_lock)
#define Dee_compiler_items_lock_read(self)       Dee_atomic_rwlock_read(&(self)->cis_lock)
#define Dee_compiler_items_lock_write(self)      Dee_atomic_rwlock_write(&(self)->cis_lock)
#define Dee_compiler_items_lock_tryupgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->cis_lock)
#define Dee_compiler_items_lock_upgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->cis_lock)
#define Dee_compiler_items_lock_downgrade(self)  Dee_atomic_rwlock_downgrade(&(self)->cis_lock)
#define Dee_compiler_items_lock_endwrite(self)   Dee_atomic_rwlock_endwrite(&(self)->cis_lock)
#define Dee_compiler_items_lock_endread(self)    Dee_atomic_rwlock_endread(&(self)->cis_lock)
#define Dee_compiler_items_lock_end(self)        Dee_atomic_rwlock_end(&(self)->cis_lock)
#endif /* CONFIG_BUILDING_DEEMON */

struct Dee_compiler_object {
	/* >> Since the compiler is a fairly large system, divided into _a_ _lot_ of
	 *    different functions all operating on the same objects, it proves to
	 *    be quite efficient not to pass contexts and descriptors around through
	 *    arguments when these functions call each other, but rather place them
	 *    in various global variables.
	 * However whenever we need to change which compiler is actually running,
	 * we need to store the state of the previously active one somewhere, and
	 * that is where `DeeCompilerObject' comes into place.
	 * Despite the various very powerful members of this object, it is not meant
	 * to operate upon them directly, but rather do the following whenever it
	 * chooses to perform an operation:
	 * >> COMPILER_BEGIN(self);
	 * >> do_the_operation();
	 * >> COMPILER_END();
	 */
	Dee_OBJECT_HEAD
	DREF DeeCompilerObject *cp_prev;      /* [0..1][lock(DeeCompiler_Lock)]
	                                       * The compiler that was active before this one and
	                                       * will be restored when `DeeCompiler_End()' is called. */
	size_t                  cp_recursion; /* [lock(DeeCompiler_Lock)] Recursion counter for how often `DeeCompiler_Begin()' was invoked for this compiler. */
#ifdef DEE_SOURCE
#define COMPILER_FNORMAL    0x0000        /* Normal compiler flags. */
#define COMPILER_FKEEPLEXER 0x0001        /* Do not save/restore the active TPP lexer. */
#define COMPILER_FKEEPERROR 0x0002        /* Do not save/restore the active parser error state. */
#define COMPILER_FMASK      0x0003        /* Mask of known flags. */
#endif /* DEE_SOURCE */
	uint16_t                cp_flags;     /* [const] Compiler flags (Set of `COMPILER_F*'). */
	uint16_t               _cp_pad[(sizeof(void *) / 2) - 1]; /* ... */
	Dee_WEAKREF_SUPPORT
#ifdef CONFIG_BUILDING_DEEMON
	/* [OVERRIDE(*, [valid_if(self != DeeCompiler_Active.wr_obj)])] */
	struct Dee_compiler_items    cp_items;         /* Hash-map of user-code compiler item wrappers. */
	DREF DeeScopeObject         *cp_scope;         /* [1..1] == ::current_scope */
	struct Dee_compiler_options *cp_inner_options; /* [0..1] == ::inner_compiler_options */
	struct ast_tags              cp_tags;          /* == ::current_tags */
	struct TPPLexer              cp_lexer;         /* [valid_if(!COMPILER_FKEEPLEXER)] == ::TPPLexer_Global */
	struct parser_errors         cp_errors;        /* [valid_if(!COMPILER_FKEEPERROR)] == ::current_parser_errors */
	struct Dee_compiler_options *cp_options;       /* [0..1] User-defined compiler options. */
#ifndef CONFIG_LANGUAGE_NO_ASM
	size_t                   cp_uasm_unique;     /* Unique user-assembly ID. */
#endif /* !CONFIG_LANGUAGE_NO_ASM */
	uint16_t                 cp_parser_flags;    /* == ::parser_flags */
	uint16_t                 cp_optimizer_flags; /* == ::optimizer_flags */
	uint16_t                 cp_unwind_limit;    /* == ::optimizer_unwind_limit */
#endif /* CONFIG_BUILDING_DEEMON */
};


/* NOTE: Because of how large the user-code interface for the compiler is,
 *       combined with the fact that the internal implementation of the
 *       compiler is completely implementation-defined, the actual compiler
 *       type is not exported from `deemon', but rather exported from `rt',
 *       thus providing code that wishes to stick to our deemon implementation
 *       the ability to tinker around with the compiler, while still not having
 *       to standardize any aspect about its inner working what-so-ever. */
DDATDEF DeeTypeObject DeeCompiler_Type; /* Compiler from rt */
#define DeeCompiler_Check(ob)      DeeObject_InstanceOf(ob, &DeeCompiler_Type)
#define DeeCompiler_CheckExact(ob) DeeObject_InstanceOfExact(ob, &DeeCompiler_Type)


/* Construct a new compiler for generating the source for the given `module'.
 * @param: flags: Set of `COMPILER_F*' (see above) */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeCompilerObject *DCALL
DeeCompiler_New(DeeObject *__restrict module, uint16_t flags);



#ifndef CONFIG_NO_THREADS
/* Lock held whenever the compiler is being used. */
DDATDEF Dee_rshared_rwlock_t DeeCompiler_Lock;
#endif /* !CONFIG_NO_THREADS */
#define DeeCompiler_LockReading()    Dee_rshared_rwlock_reading(&DeeCompiler_Lock)
#define DeeCompiler_LockWriting()    Dee_rshared_rwlock_writing(&DeeCompiler_Lock)
#define DeeCompiler_LockTryRead()    Dee_rshared_rwlock_tryread(&DeeCompiler_Lock)
#define DeeCompiler_LockTryWrite()   Dee_rshared_rwlock_trywrite(&DeeCompiler_Lock)
#define DeeCompiler_LockCanRead()    Dee_rshared_rwlock_canread(&DeeCompiler_Lock)
#define DeeCompiler_LockCanWrite()   Dee_rshared_rwlock_canwrite(&DeeCompiler_Lock)
#define DeeCompiler_LockWaitRead()   Dee_rshared_rwlock_waitread(&DeeCompiler_Lock)
#define DeeCompiler_LockWaitWrite()  Dee_rshared_rwlock_waitwrite(&DeeCompiler_Lock)
#define DeeCompiler_LockRead()       Dee_rshared_rwlock_read(&DeeCompiler_Lock)
#define DeeCompiler_LockReadNoInt()  Dee_rshared_rwlock_read_noint(&DeeCompiler_Lock)
#define DeeCompiler_LockWrite()      Dee_rshared_rwlock_write(&DeeCompiler_Lock)
#define DeeCompiler_LockWriteNoInt() Dee_rshared_rwlock_write_noint(&DeeCompiler_Lock)
#define DeeCompiler_LockTryUpgrade() Dee_rshared_rwlock_tryupgrade(&DeeCompiler_Lock)
#define DeeCompiler_LockUpgrade()    Dee_rshared_rwlock_upgrade(&DeeCompiler_Lock)
#define DeeCompiler_LockDowngrade()  Dee_rshared_rwlock_downgrade(&DeeCompiler_Lock)
#define DeeCompiler_LockEndWrite()   Dee_rshared_rwlock_endwrite(&DeeCompiler_Lock)
#define DeeCompiler_LockEndRead()    Dee_rshared_rwlock_endread(&DeeCompiler_Lock)
#define DeeCompiler_LockEnd()        Dee_rshared_rwlock_end(&DeeCompiler_Lock)

/* A weak reference to the compiler associated with
 * the currently active global compiler context.
 * WARNING: Do _NOT_ attempt to write to this weak reference! _EVER_! */
#ifdef GUARD_DEEMON_COMPILER_COMPILER_C
DDATDEF struct Dee_weakref DeeCompiler_Active;
#else /* GUARD_DEEMON_COMPILER_COMPILER_C */
DDATDEF struct Dee_weakref const DeeCompiler_Active;
#endif /* !GUARD_DEEMON_COMPILER_COMPILER_C */

/* [0..1][lock(DeeCompiler_Lock)] The currently active compiler.
 * This variable points to the current compiler while inside a
 * `DeeCompiler_Begin()...DeeCompiler_End()' block. */
DDATDEF DREF DeeCompilerObject *DeeCompiler_Current;


/* Ensure that `compiler' describes the currently active compiler context.
 * NOTE: The caller is responsible for holding a lock to `DeeCompiler_Lock'.
 * NOTE: It is possible to use sub-compilers, but it is not allowed to
 *       interweave top-level compilers below lower-level ones:
 *       >> DeeCompilerObject *a, *b;
 *       >> DeeCompiler_Begin(a);
 *       >>    DeeCompiler_Begin(b); // OK!
 *       >>       DeeCompiler_Begin(a); // ILLEGAL: `a' is already apart of the ative compiler stack.
 *       >>       DeeCompiler_End();
 *       >>    DeeCompiler_End();
 *       >> DeeCompiler_End();
 */
DFUNDEF NONNULL((1)) void DCALL
DeeCompiler_Begin(DREF DeeCompilerObject *__restrict compiler);
DFUNDEF void DCALL DeeCompiler_End(void);

/* Check if `compiler' is the currently active one.
 * If it is, make sure to copy all context-sensitive data into
 * the compiler object and mark the global compiler context as
 * being unassigned.
 * NOTE: `DeeCompiler_Active' is _NOT_ used for this check,
 *        but `DeeCompiler_Current' is instead.
 *        This is because this function is intended to be called
 *        from the destructor of `DeeCompiler_Type', before the
 *        destructor will proceed to delete all compiler sub-components.
 * NOTE:  Unlike with `DeeCompiler_Begin()' and `DeeCompiler_End()', the
 *        caller must not be holding any kind of lock on `DeeCompiler_Lock'
 *        when calling this function! */
DFUNDEF NONNULL((1)) void DCALL
DeeCompiler_Unload(DREF DeeCompilerObject *__restrict compiler);
#define Dee_COMPILER_BEGIN_NOINT(c) (DeeCompiler_LockWriteNoInt(), DeeCompiler_Begin(c))
#define Dee_COMPILER_BEGIN(c)       (DeeCompiler_LockWrite() ? -1 : (DeeCompiler_Begin(c), 0))
#define Dee_COMPILER_END()          (DeeCompiler_End(), DeeCompiler_LockEndWrite())

#ifdef DEE_SOURCE
#define COMPILER_BEGIN_NOINT Dee_COMPILER_BEGIN_NOINT
#define COMPILER_BEGIN       Dee_COMPILER_BEGIN
#define COMPILER_END         Dee_COMPILER_END
#endif /* DEE_SOURCE */


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_COMPILER_H */
