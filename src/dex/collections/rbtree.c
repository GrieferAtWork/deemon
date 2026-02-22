/* Copyright (c) 2018-2026 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_COLLECTIONS_RBTREE_C
#define GUARD_DEX_COLLECTIONS_RBTREE_C 1
#define DEE_SOURCE

#include "libcollections.h"
/**/

#include <deemon/api.h>

#include <deemon/alloc.h>           /* DeeObject_*, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC */
#include <deemon/arg.h>             /* DeeArg_Unpack* */
#include <deemon/bool.h>            /* Dee_False, Dee_True, return_bool */
#include <deemon/gc.h>            /* Dee_False, Dee_True, return_bool */
#include <deemon/error-rt.h>        /* DeeRT_Err* */
#include <deemon/error.h>           /* DeeError_* */
#include <deemon/format.h>          /* DeeFormat_PRINT, DeeFormat_Printf */
#include <deemon/int.h>             /* DeeIntObject, DeeInt_NewSize, DeeInt_Type, Dee_DIGIT_MASK, Dee_digit_t */
#include <deemon/method-hints.h>    /* DeeObject_InvokeMethodHint, TYPE_GETSET_HINTREF, TYPE_METHOD_HINT*, type_method_hint */
#include <deemon/none.h>            /* return_none */
#include <deemon/object.h>          /* DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_COMPARE_ISEQ, Dee_COMPARE_ISERR, Dee_Clear, Dee_Decref*, Dee_Incref*, Dee_OBJECT_OFFSETOF_DATA, Dee_TYPE, Dee_XDecref, Dee_foreach_pair_t, Dee_foreach_t, Dee_formatprinter_t, Dee_ssize_t, ITER_DONE, ITER_ISOK, OBJECT_HEAD, OBJECT_HEAD_INIT */
#include <deemon/seq.h>             /* DeeIterator_Type, DeeRange_New */
#include <deemon/serial.h>          /* DeeSerial*, Dee_seraddr_t */
#include <deemon/string.h>          /* Dee_UNICODE_PRINTER_PRINT, Dee_unicode_printer* */
#include <deemon/system-features.h> /* DeeSystem_DEFINE_memsetp */
#include <deemon/thread.h>          /* DeeThread_CheckInterrupt */
#include <deemon/tuple.h>           /* DeeTuple* */
#include <deemon/type.h>            /* DeeObject_Init, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC, Dee_Visit, Dee_visit_t, METHOD_FNOREFESCAPE, STRUCT_*, TF_NONE, TP_FGC, TP_FNORMAL, TYPE_*, type_* */
#include <deemon/util/atomic.h>     /* atomic_* */
#include <deemon/util/lock.h>       /* DeeLock_Acquire2, Dee_atomic_rwlock_* */

#include <hybrid/sequence/list.h> /* SLIST_* */
#include <hybrid/typecore.h>      /* __UINTPTR_TYPE__ */

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, size_t */
#include <stdint.h>  /* uintptr_t */

#undef SWAP
#define SWAP(p_a, p_b)     \
	do {                   \
		DeeObject *_swtmp; \
		_swtmp = *(p_a);   \
		*(p_a) = *(p_b);   \
		*(p_b) = _swtmp;   \
	}	__WHILE0

#undef ABS
#define ABS(x) ((x) < 0 ? -(x) : (x))


#define DOC_ERRMSG_NOTHING_SELECTED \
	"No node selected (iterator has been exhausted)"
#define DOC_ERROR_RuntimeError_CHANGED \
	"#tRuntimeError{Sequence changed after iterator was created}"


DECL_BEGIN

#ifndef CONFIG_HAVE_memsetp
#define CONFIG_HAVE_memsetp
#define memsetp(dst, pointer, num_pointers) \
	dee_memsetp(dst, (__UINTPTR_TYPE__)(pointer), num_pointers)
DeeSystem_DEFINE_memsetp(dee_memsetp)
#endif /* !CONFIG_HAVE_memsetp */

struct rbtree_node;
SLIST_HEAD(rbtree_node_slist, rbtree_node);
struct rbtree_node {
	union {
		struct rbtree_node      *rbtn_par;    /* [0..1][lock(:rbt_lock)] Parent node. */
		SLIST_ENTRY(rbtree_node) rbtn_link;   /* Link in list of nodes (used internally) */
	};
	struct rbtree_node          *rbtn_lhs;    /* [0..1][lock(:rbt_lock)] Left child node. */
	struct rbtree_node          *rbtn_rhs;    /* [0..1][lock(:rbt_lock)] Right child node. */
	DREF DeeObject              *rbtn_minkey; /* [1..1][const] Lower-bound key. */
	DREF DeeObject              *rbtn_maxkey; /* [1..1][const] Upper-bound key. */
	union {
		DREF DeeObject          *rbtn_value;  /* [1..1][const] Value mapped to this range. */
		uintptr_t                rbtn_red;    /* Lowest bit of this indicates if node is red. */
	};
};
#define rbtree_node_tryalloc() DeeObject_TRYMALLOC(struct rbtree_node)
#define rbtree_node_alloc()    DeeObject_MALLOC(struct rbtree_node)
#define rbtree_node_free(self) DeeObject_FREE(Dee_REQUIRES_TYPE(struct rbtree_node *, self))

/* Get referenced objects. */
#define rbtree_node_get_minkey(self)   (self)->rbtn_minkey
#define rbtree_node_get_maxkey(self)   (self)->rbtn_maxkey
#define rbtree_node_get_value(self)    ((DREF DeeObject *)((uintptr_t)(self)->rbtn_value & ~1))
#define rbtree_node_set_value(self, v) (void)((self)->rbtn_value = (DREF DeeObject *)(((uintptr_t)(self)->rbtn_value & 1) | (uintptr_t)(v)))
#define rbtree_node_isred(self)        ((self)->rbtn_red & 1)
#define rbtree_node_setred(self)       (void)((self)->rbtn_red |= 1)
#define rbtree_node_setblack(self)     (void)((self)->rbtn_red &= ~1)


/* Define RBTree API using the hybrid templates. */
DECL_END
#define RBTREE(name)           rbtree_abi_##name
#define RBTREE_T               struct rbtree_node
#define RBTREE_Tkey            DeeObject *
#define RBTREE_GETMINKEY(self) rbtree_node_get_minkey(self)
#define RBTREE_GETMAXKEY(self) rbtree_node_get_maxkey(self)
#define RBTREE_GETLHS(self)    (self)->rbtn_lhs
#define RBTREE_GETRHS(self)    (self)->rbtn_rhs
#define RBTREE_SETLHS(self, v) (void)((self)->rbtn_lhs = (v))
#define RBTREE_SETRHS(self, v) (void)((self)->rbtn_rhs = (v))
#define RBTREE_GETPAR(self)    (self)->rbtn_par
#define RBTREE_SETPAR(self, v) (void)((self)->rbtn_par = (v))
#define RBTREE_REDFIELD        rbtn_red
#define RBTREE_REDBIT          1
#define RBTREE_CC              DFCALL
#define RBTREE_NOTHROW         NOTHROW
#define RBTREE_DECL            PRIVATE
#define RBTREE_IMPL            PRIVATE

/* Key comparison is done by invoking user-operators, which can only
 * happen when *not* holding the R/B-tree's lock. As such, we have to
 * re-implement anything that would need to invoke these operators,
 * and to ensure that generated code doesn't use them, hook them such
 * that they would cause a compiler error */
#define RBTREE_KEY_LO(a, b) DONE_USE_RBTREE_KEY_LO;
#define RBTREE_KEY_EQ(a, b) DONE_USE_RBTREE_KEY_EQ;
#define RBTREE_KEY_NE(a, b) DONE_USE_RBTREE_KEY_NE;
#define RBTREE_KEY_GR(a, b) DONE_USE_RBTREE_KEY_GR;
#define RBTREE_KEY_GE(a, b) DONE_USE_RBTREE_KEY_GE;
#define RBTREE_KEY_LE(a, b) DONE_USE_RBTREE_KEY_LE;

/* Features */
#define RBTREE_WANT_PREV_NEXT_NODE
#define RBTREE_NDEBUG              /* Debug checks might throw for custom key types (but we want weak UB in that case) */
#define RBTREE_OMIT_LOCATE         /* Need custom locate function */
#define RBTREE_OMIT_INSERT         /* Need custom insert function */
#define RBTREE_OMIT_REMOVE         /* Need custom remove function */
#define RBTREE_WANT__INSERT_REPAIR /* Need this internal function to re-implement insert */

/* Generate code... */
#include <hybrid/sequence/rbtree-abi.h>
DECL_BEGIN


/* Return the value of `{ self = copy self; --self; }' */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_Predecessor(DeeObject *__restrict self) {
	self = DeeObject_Copy(self);
	if likely(self) {
		if unlikely(DeeObject_Dec((DeeObject **)&self))
			Dee_Clear(self);
	}
	return self;
}

/* Return the value of `{ self = copy self; ++self; }' */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_Successor(DeeObject *__restrict self) {
	self = DeeObject_Copy(self);
	if likely(self) {
		if unlikely(DeeObject_Inc((DeeObject **)&self))
			Dee_Clear(self);
	}
	return self;
}



typedef struct rbtree_object {
	OBJECT_HEAD
	struct rbtree_node *rbt_root;      /* [0..1][lock(rbt_lock)] Tree root. */
	uintptr_t           rbt_version;   /* [lock(rbt_lock)] Tree version (incremented on change). */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t rbt_lock;      /* Lock for this tree. */
#endif /* !CONFIG_NO_THREADS */
} RBTree;

typedef struct rbtree_iterator_object {
	OBJECT_HEAD
	DREF RBTree        *rbti_tree;    /* [1..1][const] The tree being iterated. */
	uintptr_t           rbti_version; /* [lock(rbti_tree->rbt_lock)] Expected tree version. */
	struct rbtree_node *rbti_next;    /* [0..1][lock(ATOMIC)] Node to iterate next. */
} RBTreeIterator;

#define RBTreeIterator_GetNext(self)         atomic_read(&(self)->rbti_next)
#define RBTreeIterator_VersionOK(self, tree) ((self)->rbti_version == (tree)->rbt_version)

#define RBTree_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->rbt_lock)
#define RBTree_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->rbt_lock)
#define RBTree_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->rbt_lock)
#define RBTree_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->rbt_lock)
#define RBTree_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->rbt_lock)
#define RBTree_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->rbt_lock)
#define RBTree_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->rbt_lock)
#define RBTree_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->rbt_lock)
#define RBTree_LockRead(self)       Dee_atomic_rwlock_read(&(self)->rbt_lock)
#define RBTree_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->rbt_lock)
#define RBTree_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->rbt_lock)
#define RBTree_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->rbt_lock)
#define RBTree_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->rbt_lock)
#define RBTree_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->rbt_lock)
#define RBTree_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->rbt_lock)
#define RBTree_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->rbt_lock)


/* Destroy the given node */
PRIVATE NONNULL((1)) void DCALL
rbtree_node_destroy(struct rbtree_node *__restrict node) {
	Dee_Decref(rbtree_node_get_minkey(node));
	Dee_Decref(rbtree_node_get_maxkey(node));
	Dee_Decref(rbtree_node_get_value(node));
	rbtree_node_free(node);
}

PRIVATE NONNULL((1)) void DCALL
rbtree_node_slist_destroyall(struct rbtree_node_slist *__restrict self) {
	struct rbtree_node *node, *temp;
	SLIST_FOREACH_SAFE (node, self, rbtn_link, temp) {
		rbtree_node_destroy(node);
	}
}





/************************************************************************/
/* RBTreeIterator IMPLEMENTATION                                        */
/************************************************************************/

PRIVATE ATTR_PURE WUNUSED NONNULL((1)) size_t DCALL
rbtree_node_count(struct rbtree_node const *__restrict self);

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
rbtreeiter_mh_iter_getindex(RBTreeIterator *__restrict self) {
	RBTree *tree = self->rbti_tree;
	struct rbtree_node *node;
	size_t result = 0;
	RBTree_LockRead(tree);
	if unlikely(!RBTreeIterator_VersionOK(self, tree))
		goto err_changed_unlock;
	node = RBTreeIterator_GetNext(self);
	if unlikely(!node) {
		/* Special case: points to 1 past the end of the R/B-tree */
		if (tree->rbt_root)
			result = rbtree_node_count(tree->rbt_root);
	} else {
		while ((node = rbtree_abi_prevnode(node)) != NULL)
			++result;
	}
	RBTree_LockEndRead(tree);
	return result;
err_changed_unlock:
	RBTree_LockEndRead(tree);
/*err_changed:*/
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rbtreeiter_mh_iter_setindex(RBTreeIterator *__restrict self, size_t new_index) {
	RBTree *tree = self->rbti_tree;
	struct rbtree_node *node;
	RBTree_LockRead(tree);
	node = tree->rbt_root;
	if (node) {
		while (node->rbtn_lhs)
			node = node->rbtn_lhs;
		while (new_index) {
			--new_index;
			node = rbtree_abi_nextnode(node);
			if (!node)
				break;
		}
	}
	atomic_write(&self->rbti_next, node);
	self->rbti_version = tree->rbt_version;
	RBTree_LockEndRead(tree);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rbtreeiter_mh_iter_rewind(RBTreeIterator *__restrict self) {
	RBTree *tree = self->rbti_tree;
	struct rbtree_node *node;
	RBTree_LockRead(tree);
	node = tree->rbt_root;
	if (node) {
		while (node->rbtn_lhs)
			node = node->rbtn_lhs;
	}
	atomic_write(&self->rbti_next, node);
	self->rbti_version = tree->rbt_version;
	RBTree_LockEndRead(tree);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
rbtreeiter_mh_iter_revert(RBTreeIterator *__restrict self, size_t step) {
	size_t result = step;
	RBTree *tree = self->rbti_tree;
	struct rbtree_node *new_node, *old_node;
	if unlikely(!step)
		return 0;
	RBTree_LockRead(tree);
	if unlikely(!RBTreeIterator_VersionOK(self, tree))
		goto err_changed_unlock;
again_old_node:
	old_node = RBTreeIterator_GetNext(self);
	if likely(old_node) {
		new_node = rbtree_abi_prevnode(old_node);
	} else {
		/* Special case: re-select the tree's last node. */
		new_node = tree->rbt_root;
		if unlikely(!new_node) {
			RBTree_LockEndRead(tree);
			return 0; /* Empty tree */
		}
		while (new_node->rbtn_lhs)
			new_node = new_node->rbtn_lhs;
	}
	while (--step) {
		struct rbtree_node *prev;
		prev = rbtree_abi_prevnode(new_node);
		if (!prev)
			break; /* First node reached */
		new_node = prev;
	}
	if unlikely(!atomic_cmpxch_or_write(&self->rbti_next, old_node, new_node)) {
		step = result;
		goto again_old_node;
	}
	result -= step;
	RBTree_LockEndRead(tree);
	return result;
err_changed_unlock:
	RBTree_LockEndRead(tree);
/*err_changed:*/
	return err_changed_sequence((DeeObject *)tree);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
rbtreeiter_mh_iter_advance(RBTreeIterator *__restrict self, size_t step) {
	size_t result = step;
	RBTree *tree = self->rbti_tree;
	struct rbtree_node *node, *old_node;
	RBTree_LockRead(tree);
	if unlikely(!RBTreeIterator_VersionOK(self, tree))
		goto err_changed_unlock;
again_old_node:
	old_node = RBTreeIterator_GetNext(self);
	if unlikely(!old_node) {
		RBTree_LockEndRead(tree);
		return 0; /* Exhausted == at-end-of-tree --> cannot advance any further */
	}
	node = old_node;
	for (; step; --step) {
		if ((node = rbtree_abi_nextnode(node)) == NULL)
			break;
	}
	if unlikely(!atomic_cmpxch_or_write(&self->rbti_next, old_node, node)) {
		step = result;
		goto again_old_node;
	}
	result -= step;
	RBTree_LockEndRead(tree);
	return 0;
err_changed_unlock:
	RBTree_LockEndRead(tree);
/*err_changed:*/
	return err_changed_sequence((DeeObject *)tree);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rbtreeiter_mh_iter_prev(RBTreeIterator *__restrict self) {
	DREF DeeObject *item[3];
	RBTree *tree = self->rbti_tree;
	struct rbtree_node *new_node, *old_node;
	RBTree_LockRead(tree);
	if unlikely(!RBTreeIterator_VersionOK(self, tree))
		goto err_changed_unlock;
again_old_node:
	old_node = RBTreeIterator_GetNext(self);
	if likely(old_node) {
		new_node = rbtree_abi_prevnode(old_node);
	} else {
		/* Special case: re-select the tree's last node. */
		new_node = tree->rbt_root;
		if unlikely(!new_node) {
			RBTree_LockEndRead(tree);
			return 0; /* Empty tree */
		}
		while (new_node->rbtn_lhs)
			new_node = new_node->rbtn_lhs;
	}
	if unlikely(!atomic_cmpxch_or_write(&self->rbti_next, old_node, new_node))
		goto again_old_node;
	item[0] = rbtree_node_get_minkey(new_node);
	item[1] = rbtree_node_get_maxkey(new_node);
	item[2] = rbtree_node_get_value(new_node);
	Dee_Incref(item[0]);
	Dee_Incref(item[1]);
	Dee_Incref(item[2]);
	RBTree_LockEndRead(tree);
	return DeeTuple_NewVectorInherited(3, item);
err_changed_unlock:
	RBTree_LockEndRead(tree);
/*err_changed:*/
	err_changed_sequence((DeeObject *)tree);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rbtreeiter_bool(RBTreeIterator *__restrict self) {
	RBTree *tree = self->rbti_tree;
	struct rbtree_node *node;
	RBTree_LockRead(tree);
	if unlikely(!RBTreeIterator_VersionOK(self, tree))
		goto err_changed_unlock;
	node = RBTreeIterator_GetNext(self);
	if unlikely(!node) {
		RBTree_LockEndRead(tree);
		return 0;
	}
	node = rbtree_abi_nextnode(node);
	RBTree_LockEndRead(tree);
	return node ? 1 : 0;
err_changed_unlock:
	RBTree_LockEndRead(tree);
/*err_changed:*/
	return err_changed_sequence((DeeObject *)tree);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rbtreeiter_mh_iter_peek(RBTreeIterator *__restrict self) {
	DREF DeeObject *item[3];
	RBTree *tree = self->rbti_tree;
	struct rbtree_node *node;
	RBTree_LockRead(tree);
	if unlikely(!RBTreeIterator_VersionOK(self, tree))
		goto err_changed_unlock;
	node = RBTreeIterator_GetNext(self);
	if unlikely(!node) {
		RBTree_LockEndRead(tree);
		return ITER_DONE;
	}
	item[0] = rbtree_node_get_minkey(node);
	item[1] = rbtree_node_get_maxkey(node);
	item[2] = rbtree_node_get_value(node);
	Dee_Incref(item[0]);
	Dee_Incref(item[1]);
	Dee_Incref(item[2]);
	RBTree_LockEndRead(tree);
	return DeeTuple_NewVectorInherited(3, item);
err_changed_unlock:
	RBTree_LockEndRead(tree);
/*err_changed:*/
	err_changed_sequence((DeeObject *)tree);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rbtreeiter_next(RBTreeIterator *__restrict self) {
	DREF DeeObject *item[3];
	RBTree *tree = self->rbti_tree;
	struct rbtree_node *node, *old_node;
again_old_node:
	RBTree_LockRead(tree);
	if unlikely(!RBTreeIterator_VersionOK(self, tree))
		goto err_changed_unlock;
	old_node = RBTreeIterator_GetNext(self);
	if unlikely(!old_node) {
		RBTree_LockEndRead(tree);
		return ITER_DONE;
	}
	node    = rbtree_abi_nextnode(old_node);
	item[0] = rbtree_node_get_minkey(old_node);
	item[1] = rbtree_node_get_maxkey(old_node);
	item[2] = rbtree_node_get_value(old_node);
	Dee_Incref(item[0]);
	Dee_Incref(item[1]);
	Dee_Incref(item[2]);
	if unlikely(!atomic_cmpxch_or_write(&self->rbti_next, old_node, node)) {
		RBTree_LockEndRead(tree);
		Dee_Decrefv_unlikely(item, 3);
		goto again_old_node;
	}
	RBTree_LockEndRead(tree);
	/* TODO: Instead of a tuple, must return a custom sequence type that:
	 * - Behaves like a 3-element tuple
	 * - But can also be unpacked to 2 elements, in which case those are:
	 *   >> (DeeRange_New(this[0], this[1], NULL), this[2]) */
	return DeeTuple_NewVectorInherited(3, item);
err_changed_unlock:
	RBTree_LockEndRead(tree);
/*err_changed:*/
	err_changed_sequence((DeeObject *)tree);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rbtreeiter_nextpair(RBTreeIterator *__restrict self,
                    DREF DeeObject *key_and_value[2]) {
	DREF DeeObject *range, *minkey, *maxkey, *value;
	RBTree *tree = self->rbti_tree;
	struct rbtree_node *node, *old_node;
again_old_node:
	RBTree_LockRead(tree);
	if unlikely(!RBTreeIterator_VersionOK(self, tree))
		goto err_changed_unlock;
	old_node = RBTreeIterator_GetNext(self);
	if unlikely(!old_node) {
		RBTree_LockEndRead(tree);
		return 1;
	}
	node   = rbtree_abi_nextnode(old_node);
	minkey = rbtree_node_get_minkey(old_node);
	maxkey = rbtree_node_get_maxkey(old_node);
	value  = rbtree_node_get_value(old_node);
	Dee_Incref(minkey);
	Dee_Incref(maxkey);
	Dee_Incref(value);
	if unlikely(!atomic_cmpxch_or_write(&self->rbti_next, old_node, node)) {
		RBTree_LockEndRead(tree);
		Dee_Decref_unlikely(minkey);
		Dee_Decref_unlikely(maxkey);
		Dee_Decref_unlikely(value);
		goto again_old_node;
	}
	RBTree_LockEndRead(tree);
	range = DeeRange_New(minkey, maxkey, NULL);
	Dee_Decref_unlikely(maxkey);
	Dee_Decref_unlikely(minkey);
	if unlikely(!range)
		goto err_value;
	key_and_value[0] = range; /* Inherit reference */
	key_and_value[1] = value; /* Inherit reference */
	return 0;
err_value:
	Dee_Decref_unlikely(value);
	return -1;
err_changed_unlock:
	RBTree_LockEndRead(tree);
/*err_changed:*/
	return err_changed_sequence((DeeObject *)tree);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rbtreeiter_nextkey(RBTreeIterator *__restrict self) {
	DREF DeeObject *result, *minkey, *maxkey;
	RBTree *tree = self->rbti_tree;
	struct rbtree_node *node, *old_node;
again_old_node:
	RBTree_LockRead(tree);
	if unlikely(!RBTreeIterator_VersionOK(self, tree))
		goto err_changed_unlock;
	old_node = RBTreeIterator_GetNext(self);
	if unlikely(!old_node) {
		RBTree_LockEndRead(tree);
		return ITER_DONE;
	}
	node   = rbtree_abi_nextnode(old_node);
	minkey = rbtree_node_get_minkey(old_node);
	maxkey = rbtree_node_get_maxkey(old_node);
	Dee_Incref(minkey);
	Dee_Incref(maxkey);
	if unlikely(!atomic_cmpxch_or_write(&self->rbti_next, old_node, node)) {
		RBTree_LockEndRead(tree);
		Dee_Decref_unlikely(minkey);
		Dee_Decref_unlikely(maxkey);
		goto again_old_node;
	}
	RBTree_LockEndRead(tree);
	result = DeeRange_New(minkey, maxkey, NULL);
	Dee_Decref_unlikely(minkey);
	Dee_Decref_unlikely(maxkey);
	return result;
err_changed_unlock:
	RBTree_LockEndRead(tree);
/*err_changed:*/
	err_changed_sequence((DeeObject *)tree);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rbtreeiter_nextvalue(RBTreeIterator *__restrict self) {
	DREF DeeObject *value;
	RBTree *tree = self->rbti_tree;
	struct rbtree_node *node, *old_node;
again_old_node:
	RBTree_LockRead(tree);
	if unlikely(!RBTreeIterator_VersionOK(self, tree))
		goto err_changed_unlock;
	old_node = RBTreeIterator_GetNext(self);
	if unlikely(!old_node) {
		RBTree_LockEndRead(tree);
		return ITER_DONE;
	}
	node  = rbtree_abi_nextnode(old_node);
	value = rbtree_node_get_value(old_node);
	Dee_Incref(value);
	if unlikely(!atomic_cmpxch_or_write(&self->rbti_next, old_node, node)) {
		RBTree_LockEndRead(tree);
		Dee_Decref_unlikely(value);
		goto again_old_node;
	}
	RBTree_LockEndRead(tree);
	return value;
err_changed_unlock:
	RBTree_LockEndRead(tree);
/*err_changed:*/
	err_changed_sequence((DeeObject *)tree);
	return NULL;
}

PRIVATE struct type_iterator rbtreeiter_iterator = {
	/* .tp_nextpair  = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&rbtreeiter_nextpair,
	/* .tp_nextkey   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rbtreeiter_nextkey,
	/* .tp_nextvalue = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rbtreeiter_nextvalue,
};


PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
rbtreeiter_eq_impl(RBTreeIterator *self, RBTreeIterator *other) {
	struct rbtree_node *lhs_node, *rhs_node;
	if (self == other)
		goto yes;
	if (self->rbti_tree != other->rbti_tree)
		goto no;
	lhs_node = RBTreeIterator_GetNext(self);
	rhs_node = RBTreeIterator_GetNext(other);
	if (lhs_node != rhs_node)
		goto no;
yes:
	return true;
no:
	return false;
}

/* Check if `elem' is reachable from `subtree' (via its lhs/rhs pointers) */
PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
rbtree_node_subtree_contains(struct rbtree_node *subtree,
                             struct rbtree_node *elem) {
again:
	if (subtree == elem)
		return true;
	if (subtree->rbtn_lhs) {
		if (subtree->rbtn_rhs) {
			if (rbtree_node_subtree_contains(subtree->rbtn_rhs, elem))
				return true;
		}
		subtree = subtree->rbtn_lhs;
		goto again;
	}
	if (subtree->rbtn_rhs) {
		subtree = subtree->rbtn_rhs;
		goto again;
	}
	return false;
}

PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
rbtreeiter_lo_impl(RBTreeIterator *self, RBTreeIterator *other) {
	RBTree *tree = self->rbti_tree;
	struct rbtree_node *lhs_node, *rhs_node;
	if (self == other)
		goto no;
	if (tree != other->rbti_tree)
		return tree < other->rbti_tree;
	RBTree_LockRead(tree);
	lhs_node = RBTreeIterator_GetNext(self);
	rhs_node = RBTreeIterator_GetNext(other);
	if (lhs_node == rhs_node)
		goto no_unlock;
	if unlikely(!RBTreeIterator_VersionOK(self, tree))
		goto err_changed_unlock;
	if unlikely(!RBTreeIterator_VersionOK(other, tree))
		goto err_changed_unlock;

	/* Check if `rhs_node' is a node that appears before `lhs_node' */
	for (;;) {
		struct rbtree_node *par;
		if (lhs_node->rbtn_lhs &&
		    rbtree_node_subtree_contains(lhs_node->rbtn_lhs, rhs_node))
			goto yes_unlock;
goto_par:
		par = lhs_node->rbtn_par;
		if (par == NULL)
			break; /* Root reached */
		if (par->rbtn_lhs == lhs_node) {
			/* We're the left-child of `par', so if `par' is
			 * `rhs_node', then that node comes *after* us! */
			if (par == rhs_node)
				goto no_unlock;

			/* `lhs_node' is the left-most child of `par',
			 * so can already move on to the next parent. */
			lhs_node = par;
			goto goto_par;
		}

		/* We're the right-child of `par', so if `par' is
		 * `rhs_node', then that node comes *before* us! */
		if (par == rhs_node)
			goto yes_unlock;
		lhs_node = par;
	}
no_unlock:
	RBTree_LockEndRead(tree);
no:
	return false;
yes_unlock:
	RBTree_LockEndRead(tree);
/*yes:*/
	return true;
err_changed_unlock:
	RBTree_LockEndRead(tree);
/*err_changed:*/
	return lhs_node < rhs_node;
}

PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
rbtreeiter_le_impl(RBTreeIterator *self, RBTreeIterator *other) {
	RBTree *tree = self->rbti_tree;
	struct rbtree_node *lhs_node, *rhs_node;
	if (self == other)
		goto yes;
	if (tree != other->rbti_tree)
		return tree < other->rbti_tree;
	RBTree_LockRead(tree);
	lhs_node = RBTreeIterator_GetNext(self);
	rhs_node = RBTreeIterator_GetNext(other);
	if (lhs_node == rhs_node)
		goto yes_unlock;
	if unlikely(!RBTreeIterator_VersionOK(self, tree))
		goto err_changed_unlock;
	if unlikely(!RBTreeIterator_VersionOK(other, tree))
		goto err_changed_unlock;

	/* Check if `rhs_node' is a node that appears before `lhs_node' */
	for (;;) {
		struct rbtree_node *par;
		if (lhs_node->rbtn_lhs &&
		    rbtree_node_subtree_contains(lhs_node->rbtn_lhs, rhs_node))
			goto yes_unlock;
goto_par:
		par = lhs_node->rbtn_par;
		if (par == NULL)
			break; /* Root reached */
		if (par->rbtn_lhs == lhs_node) {
			/* We're the left-child of `par', so if `par' is
			 * `rhs_node', then that node comes *after* us! */
			if (par == rhs_node)
				goto no_unlock;

			/* `lhs_node' is the left-most child of `par',
			 * so can already move on to the next parent. */
			lhs_node = par;
			goto goto_par;
		}

		/* We're the right-child of `par', so if `par' is
		 * `rhs_node', then that node comes *before* us! */
		if (par == rhs_node)
			goto yes_unlock;
		lhs_node = par;
	}
no_unlock:
	RBTree_LockEndRead(tree);
/*no:*/
	return false;
yes_unlock:
	RBTree_LockEndRead(tree);
yes:
	return true;
err_changed_unlock:
	RBTree_LockEndRead(tree);
/*err_changed:*/
	return lhs_node <= rhs_node;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rbtreeiter_eq(RBTreeIterator *self, RBTreeIterator *other) {
	if (DeeObject_AssertType(other, &RBTreeIterator_Type))
		goto err;
	return_bool(rbtreeiter_eq_impl(self, other));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rbtreeiter_ne(RBTreeIterator *self, RBTreeIterator *other) {
	if (DeeObject_AssertType(other, &RBTreeIterator_Type))
		goto err;
	return_bool(!rbtreeiter_eq_impl(self, other));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rbtreeiter_lo(RBTreeIterator *self, RBTreeIterator *other) {
	if (DeeObject_AssertType(other, &RBTreeIterator_Type))
		goto err;
	return_bool(rbtreeiter_lo_impl(self, other));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rbtreeiter_le(RBTreeIterator *self, RBTreeIterator *other) {
	if (DeeObject_AssertType(other, &RBTreeIterator_Type))
		goto err;
	return_bool(rbtreeiter_le_impl(self, other));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rbtreeiter_gr(RBTreeIterator *self, RBTreeIterator *other) {
	if (DeeObject_AssertType(other, &RBTreeIterator_Type))
		goto err;
	return_bool(rbtreeiter_lo_impl(other, self));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rbtreeiter_ge(RBTreeIterator *self, RBTreeIterator *other) {
	if (DeeObject_AssertType(other, &RBTreeIterator_Type))
		goto err;
	return_bool(rbtreeiter_le_impl(other, self));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rbtreeiter_removenode(RBTreeIterator *self, size_t argc, DeeObject *const *argv) {
	struct rbtree_node *node;
	RBTree *tree = self->rbti_tree;
	DeeArg_Unpack0(err, argc, argv, "removenode");
	RBTree_LockWrite(tree);
	if unlikely(!RBTreeIterator_VersionOK(self, tree))
		goto err_changed_unlock;
	node = RBTreeIterator_GetNext(self);
	if unlikely(!node)
		goto err_nothing_selected_unlock;

	/* Remove node from tree. */
	rbtree_abi_removenode(&tree->rbt_root, node);
	++tree->rbt_version;
	RBTree_LockEndWrite(tree);

	/* Destroy the node that was just removed. */
	rbtree_node_destroy(node);
	return_none;
err_nothing_selected_unlock:
	RBTree_LockEndWrite(tree);
	DeeError_Throwf(&DeeError_ValueError, "No node selected by iterator");
err:
	return NULL;
err_changed_unlock:
	RBTree_LockEndWrite(tree);
/*err_changed:*/
	err_changed_sequence((DeeObject *)tree);
	return NULL;
}

#define DEFINE_RBTREE_NODE_FIELD_GETTER(rbtreeiter_get_FOO, getfield, str_FOO) \
	PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL                         \
	rbtreeiter_get_FOO(RBTreeIterator *self) {                                 \
		DREF DeeObject *result;                                                \
		RBTree *tree = self->rbti_tree;                                        \
		struct rbtree_node *node;                                              \
		RBTree_LockRead(tree);                                                 \
		if unlikely(!RBTreeIterator_VersionOK(self, tree))                     \
			goto err_changed_unlock;                                           \
		node = RBTreeIterator_GetNext(self);                                   \
		if unlikely(!node) {                                                   \
			RBTree_LockEndRead(tree);                                          \
			goto err_unbound;                                                  \
		}                                                                      \
		result = getfield(node);                                               \
		Dee_Incref(result);                                                    \
		RBTree_LockEndRead(tree);                                              \
		return result;                                                         \
	err_unbound:                                                               \
		return DeeRT_ErrTUnboundAttrCStr(&RBTreeIterator_Type, self, str_FOO); \
	err_changed_unlock:                                                        \
		RBTree_LockEndRead(tree);                                              \
	/*err_changed:*/                                                           \
		err_changed_sequence((DeeObject *)tree);                               \
		return NULL;                                                           \
	}
DEFINE_RBTREE_NODE_FIELD_GETTER(rbtreeiter_get_minkey, rbtree_node_get_minkey, "minkey")
DEFINE_RBTREE_NODE_FIELD_GETTER(rbtreeiter_get_maxkey, rbtree_node_get_maxkey, "maxkey")
DEFINE_RBTREE_NODE_FIELD_GETTER(rbtreeiter_get_value, rbtree_node_get_value, "value")
#undef DEFINE_RBTREE_NODE_FIELD_GETTER

PRIVATE WUNUSED NONNULL((1)) int DCALL
rbtreeiter_set_value(RBTreeIterator *self, DeeObject *value) {
	DREF DeeObject *oldval;
	RBTree *tree = self->rbti_tree;
	struct rbtree_node *node;
	RBTree_LockWrite(tree);
	if unlikely(!RBTreeIterator_VersionOK(self, tree))
		goto err_changed_unlock;
	node = RBTreeIterator_GetNext(self);
	if unlikely(!node) {
		RBTree_LockEndWrite(tree);
		goto err_unbound;
	}
	oldval = rbtree_node_get_value(node);
	rbtree_node_set_value(node, value);
	Dee_Incref(value);
	RBTree_LockEndWrite(tree);
	Dee_Decref(oldval);
	return 0;
err_unbound:
	DeeRT_ErrTUnboundAttrCStr(&RBTreeIterator_Type, self, "value");
	return -1;
err_changed_unlock:
	RBTree_LockEndWrite(tree);
/*err_changed:*/
	return err_changed_sequence((DeeObject *)tree);
}


#define DEFINE_RBTREE_NODE_SUBNODE_GETTER(rbtreeiter_get_FOO, rbtn_FOO, str_FOO) \
	PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL                           \
	rbtreeiter_get_FOO(RBTreeIterator *__restrict self) {                        \
		RBTree *tree = self->rbti_tree;                                          \
		DREF RBTreeIterator *result;                                             \
		struct rbtree_node *node;                                                \
		result = DeeObject_MALLOC(RBTreeIterator);                               \
		if unlikely(!result)                                                     \
			goto done;                                                           \
		result->rbti_tree = tree;                                                \
		RBTree_LockRead(tree);                                                   \
		if unlikely(!RBTreeIterator_VersionOK(self, tree))                       \
			goto err_r_changed_unlock;                                           \
		node = RBTreeIterator_GetNext(self);                                     \
		if unlikely(!node)                                                       \
			goto err_r_unlock_unbound;                                           \
		node = node->rbtn_FOO;                                                   \
		result->rbti_version = tree->rbt_version;                                \
		RBTree_LockEndRead(tree);                                                \
		if (!node) {                                                             \
			DeeObject_FREE(result);                                              \
			return_none;                                                         \
		}                                                                        \
		result->rbti_next = node;                                                \
		Dee_Incref(result->rbti_tree);                                           \
		DeeObject_Init(result, &RBTreeIterator_Type);                            \
	done:                                                                        \
		return Dee_AsObject(result);                                             \
	err_r_unlock_unbound:                                                        \
		RBTree_LockEndRead(tree);                                                \
		DeeObject_FREE(result);                                                  \
		return DeeRT_ErrTUnboundAttrCStr(&RBTreeIterator_Type, self, str_FOO);   \
	err_r_changed_unlock:                                                        \
		RBTree_LockEndRead(tree);                                                \
	/*err_r_changed:*/                                                           \
		DeeObject_FREE(result);                                                  \
		err_changed_sequence((DeeObject *)tree);                                 \
		return NULL;                                                             \
	}
DEFINE_RBTREE_NODE_SUBNODE_GETTER(rbtreeiter_get_lhs, rbtn_lhs, "__lhs__")
DEFINE_RBTREE_NODE_SUBNODE_GETTER(rbtreeiter_get_rhs, rbtn_rhs, "__rhs__")
DEFINE_RBTREE_NODE_SUBNODE_GETTER(rbtreeiter_get_parent, rbtn_par, "__parent__")
#undef DEFINE_RBTREE_NODE_SUBNODE_GETTER

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rbtreeiter_get_isred(RBTreeIterator *__restrict self) {
	bool result;
	RBTree *tree = self->rbti_tree;
	struct rbtree_node *node;
	RBTree_LockRead(tree);
	if unlikely(!RBTreeIterator_VersionOK(self, tree))
		goto err_changed_unlock;
	node = RBTreeIterator_GetNext(self);
	if unlikely(!node)
		goto err_unlock_unbound;
	result = rbtree_node_isred(node);
	RBTree_LockEndRead(tree);
	return_bool(result);
err_unlock_unbound:
	RBTree_LockEndRead(tree);
	return DeeRT_ErrTUnboundAttrCStr(&RBTreeIterator_Type, self, "__isred__");
err_changed_unlock:
	RBTree_LockEndRead(tree);
/*err_changed:*/
	err_changed_sequence((DeeObject *)tree);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rbtreeiter_ctor(RBTreeIterator *__restrict self) {
	self->rbti_tree = (DREF RBTree *)DeeObject_NewDefault(&RBTree_Type);
	if unlikely(!self->rbti_tree)
		goto err;
	ASSERT(self->rbti_tree->rbt_version == 0);
	self->rbti_version = 0;
	self->rbti_next    = NULL;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rbtreeiter_copy(RBTreeIterator *self, RBTreeIterator *other) {
	RBTree *tree = other->rbti_tree;
	self->rbti_tree = tree;
	Dee_Incref(tree);
	RBTree_LockRead(tree);
	self->rbti_version = other->rbti_version;
	self->rbti_next    = RBTreeIterator_GetNext(other);
	RBTree_LockEndRead(tree);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rbtreeiter_init(RBTreeIterator *self, size_t argc, DeeObject *const *argv) {
	RBTree *tree;
	DeeArg_Unpack1(err, argc, argv, "_RBTreeIterator", &tree);
	if (DeeObject_AssertType(tree, &RBTree_Type))
		goto err;
	self->rbti_tree = tree;
	Dee_Incref(tree);
	RBTree_LockRead(tree);
	self->rbti_version = tree->rbt_version;
	self->rbti_next    = tree->rbt_root;
	if (self->rbti_next) {
		while (self->rbti_next->rbtn_lhs)
			self->rbti_next = self->rbti_next->rbtn_lhs;
	}
	RBTree_LockEndRead(tree);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rbtreeiter_serialize(RBTreeIterator *__restrict self,
                     DeeSerial *__restrict writer,
                     Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(RBTreeIterator, field))
	RBTreeIterator *out;
	uintptr_t self__rbti_version;
	struct rbtree_node *self__rbti_next;
	bool is_up_to_date;
	RBTree_LockRead(self->rbti_tree);
	self__rbti_version = self->rbti_version;
	self__rbti_next    = RBTreeIterator_GetNext(self);
	RBTree_LockEndRead(self->rbti_tree);
	if (DeeSerial_PutObject(writer, ADDROF(rbti_tree), self->rbti_tree))
		goto err;
	out = DeeSerial_Addr2Mem(writer, addr, RBTreeIterator);
	out->rbti_version = self__rbti_version;
	out->rbti_next = NULL;
	RBTree_LockRead(self->rbti_tree);
	is_up_to_date = self__rbti_version == self->rbti_version;
	RBTree_LockEndRead(self->rbti_tree);
	return is_up_to_date ? DeeSerial_PutPointer(writer, ADDROF(rbti_next), self__rbti_next) : 0;
err:
	return -1;
#undef ADDROF
}

PRIVATE NONNULL((1)) void DCALL
rbtreeiter_fini(RBTreeIterator *__restrict self) {
	Dee_Decref(self->rbti_tree);
}

PRIVATE NONNULL((1, 2)) void DCALL
rbtreeiter_visit(RBTreeIterator *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_Visit(self->rbti_tree);
}

PRIVATE struct type_cmp rbtreeiter_cmp = {
	/* .tp_hash          = */ NULL,
	/* .tp_compare_eq    = */ NULL,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ NULL,
	/* .tp_eq            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rbtreeiter_eq,
	/* .tp_ne            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rbtreeiter_ne,
	/* .tp_lo            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rbtreeiter_lo,
	/* .tp_le            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rbtreeiter_le,
	/* .tp_gr            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rbtreeiter_gr,
	/* .tp_ge            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rbtreeiter_ge,
};

PRIVATE struct type_method tpconst rbtreeiter_methods[] = {
	TYPE_METHOD_HINTREF(Iterator_rewind),
	TYPE_METHOD_HINTREF(Iterator_advance),
	TYPE_METHOD_HINTREF(Iterator_prev),
	TYPE_METHOD_HINTREF(Iterator_peek),
	TYPE_METHOD_F("removenode", &rbtreeiter_removenode, METHOD_FNOREFESCAPE,
	              "()\n"
	              "#tValueError{" DOC_ERRMSG_NOTHING_SELECTED "}"
	              DOC_ERROR_RuntimeError_CHANGED
	              "Remove the node selected by @this ?. from the associated ?#seq. "
	              /**/ "Upon success, the node selected by @this ?. will be cleared"),
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst rbtreeiter_getsets[] = {
	TYPE_GETSET_HINTREF(Iterator_index),
	TYPE_GETTER_F("minkey", &rbtreeiter_get_minkey, METHOD_FNOREFESCAPE,
	              DOC_ERROR_RuntimeError_CHANGED
	              "The lower-bound key of the selected node"),
	TYPE_GETTER_F("maxkey", &rbtreeiter_get_maxkey, METHOD_FNOREFESCAPE,
	              DOC_ERROR_RuntimeError_CHANGED
	              "The upper-bound key of the selected node"),
	TYPE_GETSET_F("value", &rbtreeiter_get_value, NULL, &rbtreeiter_set_value, METHOD_FNOREFESCAPE,
	              "#tUnboundAttribute{" DOC_ERRMSG_NOTHING_SELECTED "}"
	              DOC_ERROR_RuntimeError_CHANGED
	              "Get or set the value of the selected node"),
	TYPE_GETTER_F("__lhs__", &rbtreeiter_get_lhs, METHOD_FNOREFESCAPE,
	              "->?.\n"
	              DOC_ERROR_RuntimeError_CHANGED
	              "#tUnboundAttribute{" DOC_ERRMSG_NOTHING_SELECTED "}"
	              "The left child of the selected node (or ?N if no such node exists)"),
	TYPE_GETTER_F("__rhs__", &rbtreeiter_get_rhs, METHOD_FNOREFESCAPE,
	              "->?.\n"
	              DOC_ERROR_RuntimeError_CHANGED
	              "#tUnboundAttribute{" DOC_ERRMSG_NOTHING_SELECTED "}"
	              "The right child of the selected node (or ?N if no such node exists)"),
	TYPE_GETTER_F("__parent__", &rbtreeiter_get_parent, METHOD_FNOREFESCAPE,
	              "->?.\n"
	              DOC_ERROR_RuntimeError_CHANGED
	              "#tUnboundAttribute{" DOC_ERRMSG_NOTHING_SELECTED "}"
	              "The parent of the selected node (or ?N if the node is the root node)"),
	TYPE_GETTER_F("__isred__", &rbtreeiter_get_isred, METHOD_FNOREFESCAPE,
	              "->?DBool\n"
	              DOC_ERROR_RuntimeError_CHANGED
	              "#tUnboundAttribute{" DOC_ERRMSG_NOTHING_SELECTED "}"
	              "Evaluates to ?t if the selected node is red"),
	TYPE_GETSET_END
};

PRIVATE struct type_method_hint tpconst rbtreeiter_method_hints[] = {
	TYPE_METHOD_HINT(iter_getindex, &rbtreeiter_mh_iter_getindex),
	TYPE_METHOD_HINT(iter_setindex, &rbtreeiter_mh_iter_setindex),
	TYPE_METHOD_HINT(iter_rewind, &rbtreeiter_mh_iter_rewind),
	TYPE_METHOD_HINT(iter_advance, &rbtreeiter_mh_iter_advance),
	TYPE_METHOD_HINT(iter_revert, &rbtreeiter_mh_iter_revert),
	TYPE_METHOD_HINT(iter_prev, &rbtreeiter_mh_iter_prev),
	TYPE_METHOD_HINT(iter_peek, &rbtreeiter_mh_iter_peek),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_member tpconst rbtreeiter_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT_AB, offsetof(RBTreeIterator, rbti_tree), "->?GRBTree"),
	TYPE_MEMBER_FIELD("__version__", STRUCT_UINTPTR_T | STRUCT_ATOMIC | STRUCT_CONST,
	                  offsetof(RBTreeIterator, rbti_version)),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject RBTreeIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RBTreeIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ RBTreeIterator,
			/* tp_ctor:        */ &rbtreeiter_ctor,
			/* tp_copy_ctor:   */ &rbtreeiter_copy,
			/* tp_any_ctor:    */ &rbtreeiter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &rbtreeiter_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rbtreeiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&rbtreeiter_bool
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&rbtreeiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &rbtreeiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rbtreeiter_next,
	/* .tp_iterator      = */ &rbtreeiter_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ rbtreeiter_methods,
	/* .tp_getsets       = */ rbtreeiter_getsets,
	/* .tp_members       = */ rbtreeiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ rbtreeiter_method_hints
};







/************************************************************************/
/* RBTree IMPLEMENTATION                                                */
/************************************************************************/


PRIVATE WUNUSED NONNULL((1)) DREF RBTreeIterator *DCALL
rbtree_iter(RBTree *__restrict self) {
	DREF RBTreeIterator *result;
	result = DeeObject_MALLOC(RBTreeIterator);
	if unlikely(!result)
		goto done;
	RBTree_LockRead(self);
	result->rbti_version = self->rbt_version;
	result->rbti_next    = self->rbt_root;
	if (result->rbti_next) {
		/* Start with the left-most node */
		while (result->rbti_next->rbtn_lhs)
			result->rbti_next = result->rbti_next->rbtn_lhs;
	}
	RBTree_LockEndRead(self);
	result->rbti_tree = self;
	Dee_Incref(self);
	DeeObject_Init(result, &RBTreeIterator_Type);
done:
	return result;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1)) size_t DCALL
rbtree_node_count(struct rbtree_node const *__restrict self) {
	size_t result = 1;
again:
	if (self->rbtn_lhs) {
		if (self->rbtn_rhs)
			result += rbtree_node_count(self->rbtn_rhs);
		self = self->rbtn_lhs;
		++result;
		goto again;
	}
	if (self->rbtn_rhs) {
		self = self->rbtn_rhs;
		++result;
		goto again;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
rbtree_size(RBTree *__restrict self) {
	size_t result = 0;
	RBTree_LockRead(self);
	if (self->rbt_root)
		result = rbtree_node_count(self->rbt_root);
	RBTree_LockEndRead(self);
	return result;
}



struct rbtree_minmax {
	struct rbtree_node *rbtmm_min; /* [1..1] Lowest node */
	struct rbtree_node *rbtmm_max; /* [1..1] Greatest node */
};

/* Lookup the node containing a given `key', where `root' is guarantied to overlap with [minkey:maxkey]
 * NOTE: The caller must currently be holding a read-lock to `self'!
 * @return: 1 : Version changed after (lock is still held)
 * @return: 0 : Success (in this case, a read-lock to `self' is still being held)
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL
rbtree_do_minmaxlocate_node_in_root(RBTree *self, DeeObject *minkey, DeeObject *maxkey,
                                    struct rbtree_minmax *__restrict result,
                                    struct rbtree_node *__restrict root) {
	int temp;
	uintptr_t version;
	struct rbtree_node *min_node;
	struct rbtree_node *max_node;

	/* Check for special case: when the key-range consists of
	 * only a single key, then there can only be max. 1 node,
	 * and that node will always be `root'. */
	if (minkey == maxkey) {
		result->rbtmm_min = root;
		result->rbtmm_max = root;
		return 0;
	}

	version = self->rbt_version;

	/* Helper macro to do all of the necessary work to perform a safe comparison operation. */
#define UNLOCK_COMPARE_LOCK(func, lhs, rhs)       \
	do {                                          \
		DREF DeeObject *_rhs_temp;                \
		_rhs_temp = (rhs);                        \
		Dee_Incref(_rhs_temp);                    \
		RBTree_LockEndRead(self);                 \
		temp = func(lhs, _rhs_temp);              \
		Dee_Decref(_rhs_temp);                    \
		if unlikely(temp < 0)                     \
			goto err;                             \
		RBTree_LockRead(self);                    \
		if unlikely(self->rbt_version != version) \
			goto again;                           \
	}	__WHILE0

	min_node = root;
	for (;;) {
		struct rbtree_node *iter;
		iter = min_node->rbtn_lhs;
		if (!iter)
			break;
		UNLOCK_COMPARE_LOCK(DeeObject_CmpGrAsBool, minkey, rbtree_node_get_maxkey(iter));
		if (!temp) {
			min_node = iter;
			continue;
		}
		/* Check if we can find an in-range key in iter->RHS[->RHS...] */
		iter = iter->rbtn_lhs;
		while (iter) {
			UNLOCK_COMPARE_LOCK(DeeObject_CmpGrAsBool, minkey, rbtree_node_get_maxkey(iter));
			if (!temp) {
				min_node = iter;
				break;
			}
			iter = iter->rbtn_lhs;
		}
		break;
	}
	max_node = root;
	for (;;) {
		struct rbtree_node *iter;
		iter = max_node->rbtn_rhs;
		if (!iter)
			break;
		UNLOCK_COMPARE_LOCK(DeeObject_CmpLoAsBool, maxkey, rbtree_node_get_minkey(iter));
		if (!temp) {
			max_node = iter;
			continue;
		}
		/* Check if we can find an in-range key in iter->LHS[->LHS...] */
		iter = iter->rbtn_lhs;
		while (iter) {
			UNLOCK_COMPARE_LOCK(DeeObject_CmpLoAsBool, maxkey, rbtree_node_get_minkey(iter));
			if (!temp) {
				max_node = iter;
				break;
			}
			iter = iter->rbtn_lhs;
		}
		break;
	}

	/* Because the min/max-range may be spread across different sub-trees,
	 * we must still  check for the  case where the  predecessor/successor
	 * the min/max node continues to be in-bounds! */
	for (;;) {
		struct rbtree_node *iter;
		iter = rbtree_abi_prevnode(min_node);
		if (!iter)
			break;
		UNLOCK_COMPARE_LOCK(DeeObject_CmpGrAsBool, minkey, rbtree_node_get_maxkey(iter));
		if (temp)
			break;
		min_node = iter;
	}
	for (;;) {
		struct rbtree_node *iter;
		iter = rbtree_abi_nextnode(max_node);
		if (!iter)
			break;
		UNLOCK_COMPARE_LOCK(DeeObject_CmpLoAsBool, maxkey, rbtree_node_get_minkey(iter));
		if (temp)
			break;
		max_node = iter;
	}

	/* Write-back our results. */
	result->rbtmm_min = min_node;
	result->rbtmm_max = max_node;
	return 0; /* Note how we don't release the read-lock which we're still holding! */
#undef UNLOCK_COMPARE_LOCK
err:
	return -1;
again:
	return 2;
}

/* Lookup the node containing a given `key'
 * @return: 1 : No nodes overlap with `[minkey:maxkey]' 
 * @return: 0 : Success (in this case, a read-lock to `self' is still being held)
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
rbtree_do_minmaxlocate_node(RBTree *self, DeeObject *minkey, DeeObject *maxkey,
                            struct rbtree_minmax *__restrict result) {
	struct rbtree_node *root;
	uintptr_t version;
	DREF DeeObject *root_minkey;
	DREF DeeObject *root_maxkey;
	RBTree_LockRead(self);
again:
	root    = self->rbt_root;
	version = self->rbt_version;
	while (root) {
		int temp;
		root_minkey = rbtree_node_get_minkey(root);
		root_maxkey = rbtree_node_get_maxkey(root);
		Dee_Incref(root_minkey);
		Dee_Incref(root_maxkey);
		RBTree_LockEndRead(self);

		/* Check if the entire range might be located in a sub-tree */
		temp = DeeObject_CmpLoAsBool(maxkey, root_minkey);
		Dee_Decref_unlikely(root_minkey);
		if (temp != 0) {
			Dee_Decref_unlikely(root_maxkey);
			if unlikely(temp < 0)
				goto err;
			RBTree_LockRead(self);
			if unlikely(self->rbt_version != version)
				goto again;
			root = root->rbtn_lhs;
			continue;
		}
		temp = DeeObject_CmpGrAsBool(minkey, root_maxkey);
		Dee_Decref_unlikely(root_maxkey);
		if (temp != 0) {
			if unlikely(temp < 0)
				goto err;
			RBTree_LockRead(self);
			if unlikely(self->rbt_version != version)
				goto again;
			root = root->rbtn_rhs;
			continue;
		}

		/* Re-acquire lock. */
		RBTree_LockRead(self);
		if unlikely(self->rbt_version != version)
			goto again;

		temp = rbtree_do_minmaxlocate_node_in_root(self, minkey, maxkey, result, root);
		if (temp > 0)
			goto again; /* Version changed */

		/* Note how we don't release the read-lock which we're still holding! */
		return temp; /* Error or success */
	} /* while (root) */
	RBTree_LockEndRead(self);
	return 1;
err:
	return -1;
}



/* Lookup the node containing a given `key'
 * @return: * :        The node containing `key' (in this case,
 *                     a read-lock to `self' is still held)
 * @return: NULL:      Error
 * @return: ITER_DONE: No node containing `key' */
PRIVATE WUNUSED NONNULL((1, 2)) struct rbtree_node *DCALL
rbtree_trygetitem_node(RBTree *self, DeeObject *key) {
	struct rbtree_node *node;
	uintptr_t version;
	DREF DeeObject *node_minkey;
	DREF DeeObject *node_maxkey;
	RBTree_LockRead(self);
again:
	node    = self->rbt_root;
	version = self->rbt_version;
	while (node) {
		int temp;
		node_minkey = rbtree_node_get_minkey(node);
		node_maxkey = rbtree_node_get_maxkey(node);
		Dee_Incref(node_minkey);
		Dee_Incref(node_maxkey);
		RBTree_LockEndRead(self);

		/* Check if `key' is located in the left sub-tree */
		temp = DeeObject_CmpLoAsBool(key, node_minkey);
		Dee_Decref_unlikely(node_minkey);
		if (temp != 0) {
			Dee_Decref_unlikely(node_maxkey);
			if unlikely(temp < 0)
				goto err;
			RBTree_LockRead(self);
			if unlikely(self->rbt_version != version)
				goto again;
			node = node->rbtn_lhs;
			continue;
		}

		/* Check if `key' is located in the right sub-tree */
		temp = DeeObject_CmpGrAsBool(key, node_maxkey);
		Dee_Decref_unlikely(node_maxkey);
		if (temp != 0) {
			if unlikely(temp < 0)
				goto err;
			RBTree_LockRead(self);
			if unlikely(self->rbt_version != version)
				goto again;
			node = node->rbtn_rhs;
			continue;
		}

		/* Key is located in `node' */
		RBTree_LockRead(self);
		if unlikely(self->rbt_version != version)
			goto again;
		return node;
	} /* while (node) */
	RBTree_LockEndRead(self);
	return (struct rbtree_node *)ITER_DONE;
err:
	return (struct rbtree_node *)NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rbtree_trygetitem(RBTree *self, DeeObject *key) {
	DREF DeeObject *result;
	struct rbtree_node *node;
	node = rbtree_trygetitem_node(self, key);
	if (!ITER_ISOK(node))
		return (DREF DeeObject *)node;
	result = rbtree_node_get_value(node);
	Dee_Incref(result);
	RBTree_LockEndRead(self);
	return result;
}


struct rbtree_do_insert_overlap_info {
	struct rbtree_node *rbtdioi_node; /* [1..1] The node */
	uintptr_t           rbtdioi_vers;
};


/* Check if `key' supports the merging of adjacent nodes. */
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
key_supports_adjacent(DeeObject *__restrict key) {
	DeeTypeObject *typ = Dee_TYPE(key);
	return typ == &DeeInt_Type;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
int_a_plus_1_equals_b(DeeIntObject *a, DeeIntObject *b) {
	size_t i, a_size = (size_t)ABS(a->ob_size);
	if (a->ob_size != b->ob_size) {
		if ((a->ob_size + 1) != b->ob_size)
			goto nope;
		/* Special case: `a' should be all FF-bytes, and `b' should
		 * be all `00' bytes, with the last digit being a `1' */
		for (i = 0; i < a_size; ++i) {
			if (a->ob_digit[i] != Dee_DIGIT_MASK)
				goto nope;
			if (b->ob_digit[i] != 0)
				goto nope;
		}
		if (b->ob_digit[i] != 1)
			goto nope;
	} else {
		for (i = 0;;) {
			Dee_digit_t d;
			if (i >= a_size)
				goto nope;
			d = a->ob_digit[i] + 1;
			d &= Dee_DIGIT_MASK;
			if (b->ob_digit[i] != d)
				goto nope;
			++i;
			if (d != 0)
				break; /* Stop when there is no carry */
		}

		/* For the remainder of digits, a and b should be identical */
		for (;i < a_size; ++i) {
			if (a->ob_digit[i] != b->ob_digit[i])
				goto nope;
		}
	}
	return true;
nope:
	return false;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
key_adjacent(DeeObject *a, DeeObject *b) {
	if (Dee_TYPE(a) != Dee_TYPE(b))
		goto nope;
	if (Dee_TYPE(a) == &DeeInt_Type) /* return true if `a + 1 == b' */
		return int_a_plus_1_equals_b((DeeIntObject *)a, (DeeIntObject *)b);
nope:
	return false;
}


/* Try to merge `node' with its neighbors */
PRIVATE NONNULL((1, 2, 3)) void DCALL
rbtree_do_mergenode(RBTree *self, struct rbtree_node *__restrict node,
                    struct rbtree_node_slist *__restrict removed_nodes) {
	if (key_supports_adjacent(rbtree_node_get_minkey(node))) {
		struct rbtree_node *nextnode;
		nextnode = rbtree_abi_prevnode(node);
		if (nextnode && rbtree_node_get_value(nextnode) == rbtree_node_get_value(node) &&
		    key_adjacent(rbtree_node_get_maxkey(nextnode), rbtree_node_get_minkey(node))) {
			/* Remove `nextnode' and extend the caller's node.
			 *
			 * Because we know that this won't change the ordering
			 * of the tree as a whole, we don't need to re-insert
			 * the caller's node! */
			rbtree_abi_removenode(&self->rbt_root, nextnode);
			SWAP(&node->rbtn_minkey, &nextnode->rbtn_minkey);
			SLIST_INSERT_HEAD(removed_nodes, nextnode, rbtn_link);
		}
	}
	if (key_supports_adjacent(rbtree_node_get_maxkey(node))) {
		struct rbtree_node *nextnode;
		nextnode = rbtree_abi_nextnode(node);
		if (nextnode && rbtree_node_get_value(nextnode) == rbtree_node_get_value(node) &&
		    key_adjacent(rbtree_node_get_maxkey(node), rbtree_node_get_minkey(nextnode))) {
			rbtree_abi_removenode(&self->rbt_root, nextnode);
			SWAP(&node->rbtn_maxkey, &nextnode->rbtn_maxkey);
			SLIST_INSERT_HEAD(removed_nodes, nextnode, rbtn_link);
		}
	}
}

/* Implementation for the node-insert-function
 * @return: 1:  This node overlaps with another node (info for
 *              this node is written to `overlap_info' if non-NULL)
 * @return: 0:  Success
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rbtree_do_insert(RBTree *self, /*inherit(on_success)*/ struct rbtree_node *__restrict node,
                 struct rbtree_do_insert_overlap_info *overlap_info) {
	struct rbtree_node_slist removed_nodes;
	struct rbtree_node *root, *nextnode;
	uintptr_t version;
	int temp;
	DREF DeeObject *minkey;
	DREF DeeObject *maxkey;
	RBTree_LockRead(self);
again_version_changed:
	root = self->rbt_root;
	if (root == NULL) {
		if (!RBTree_LockUpgrade(self)) {
			root = self->rbt_root;
			if unlikely(root != NULL) {
				RBTree_LockDowngrade(self);
				goto read_version;
			}
		}

		/* Special case: first node. */
		node->rbtn_par = NULL;
		node->rbtn_lhs = NULL;
		node->rbtn_rhs = NULL;
		rbtree_node_setblack(node);
		self->rbt_root = node;

		/* Increment the version counter. */
		++self->rbt_version;
		RBTree_LockEndWrite(self);
		return 0;
	}
read_version:
	version = self->rbt_version;
again_load_root:
	minkey = rbtree_node_get_minkey(root);
	maxkey = rbtree_node_get_maxkey(root);
	Dee_Incref(minkey);
	Dee_Incref(maxkey);
	RBTree_LockEndRead(self);

	/* Check if `node' must go in the left sub-tree */
	temp = DeeObject_CmpLoAsBool(rbtree_node_get_maxkey(node), minkey);
	Dee_Decref_unlikely(minkey);
	if (temp != 0) {
		Dee_Decref_unlikely(maxkey);
		if unlikely(temp < 0)
			goto err;
		RBTree_LockRead(self);
		if unlikely(version != self->rbt_version)
			goto again_version_changed;
		nextnode = root->rbtn_lhs;
		if (nextnode != NULL) {
			root = nextnode;
			goto again_load_root;
		}
		if (!RBTree_LockUpgrade(self)) {
			if unlikely(version != self->rbt_version)
				goto again_downgrade_version_changed;
			ASSERT(root->rbtn_lhs == NULL);
		}
		root->rbtn_lhs = node;
	} else {
		/* Check if `node' must go in the right sub-tree */
		temp = DeeObject_CmpGrAsBool(rbtree_node_get_minkey(node), maxkey);
		Dee_Decref_unlikely(maxkey);
		if (temp != 0) {
			if unlikely(temp < 0)
				goto err;
			RBTree_LockRead(self);
			if unlikely(version != self->rbt_version)
				goto again_version_changed;
			nextnode = root->rbtn_rhs;
			if (nextnode != NULL) {
				root = nextnode;
				goto again_load_root;
			}
			if (!RBTree_LockUpgrade(self)) {
				if unlikely(version != self->rbt_version)
					goto again_downgrade_version_changed;
				ASSERT(root->rbtn_rhs == NULL);
			}
			root->rbtn_rhs = node;
		} else {
			/* Gracefully fail if the given range is already mapped. */
			if (overlap_info) {
				overlap_info->rbtdioi_node = root;
				overlap_info->rbtdioi_vers = version;
			}
			return 1;
		}
	}
	node->rbtn_par = root;
	node->rbtn_lhs = NULL;
	node->rbtn_rhs = NULL;
	rbtree_node_setred(node);

	/* Repair the RB-tree (this function is defined in <hybrid/sequence/rbtree-abi.h>) */
	rbtree_abi__insert_repair(&self->rbt_root, node, root);

	/* Check if the node can be merged with its predecessor/successor. */
	SLIST_INIT(&removed_nodes);
	rbtree_do_mergenode(self, node, &removed_nodes);

	/* Increment the version counter. */
	++self->rbt_version;
	RBTree_LockEndWrite(self);

	/* Destroy nodes that were removed for the sake of merging. */
	rbtree_node_slist_destroyall(&removed_nodes);

	return 0;
again_downgrade_version_changed:
	RBTree_LockDowngrade(self);
	goto again_version_changed;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
rbtree_mh_setold_ex(RBTree *self, DeeObject *key, DeeObject *value) {
	(void)self;
	(void)key;
	(void)value;
	/* TODO */
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
rbtree_mh_setnew_ex(RBTree *self, DeeObject *key, DeeObject *value) {
	/* Must create a new node. */
	struct rbtree_do_insert_overlap_info overlap;
	struct rbtree_node *newnode;
	DREF DeeObject *old_value;
	int temp;
	newnode = rbtree_node_alloc();
	if unlikely(!newnode)
		goto err;
	newnode->rbtn_minkey = key;
	newnode->rbtn_maxkey = key;
	newnode->rbtn_value  = value;
	Dee_Incref_n(key, 2);
	Dee_Incref(value);

again_insert:
	temp = rbtree_do_insert(self, newnode, &overlap);
	if (temp == 0)
		return ITER_DONE; /* New key added */
	if (temp < 0) {
		rbtree_node_destroy(newnode);
		goto err;
	}

	/* Overlap happened */
	RBTree_LockRead(self);
	if (overlap.rbtdioi_vers != self->rbt_version) {
		RBTree_LockEndRead(self);
		goto again_insert;
	}
	old_value = rbtree_node_get_value(overlap.rbtdioi_node);
	Dee_Incref(old_value);
	RBTree_LockEndRead(self);
	return old_value;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rbtree_contains(RBTree *self, DeeObject *key) {
	DREF DeeObject *result;
	result = rbtree_trygetitem(self, key);
	if likely(result != NULL) {
		if (result == ITER_DONE) {
			result = Dee_False;
		} else {
			Dee_Decref(result);
			result = Dee_True;
		}
		Dee_Incref(result);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rbtree_getitem(RBTree *self, DeeObject *key) {
	DREF DeeObject *result;
	result = rbtree_trygetitem(self, key);
	if unlikely(result == ITER_DONE) {
		DeeRT_ErrUnknownKey(self, key);
		result = NULL;
	}
	return result;
}

/* Insert `newnode' as the immediate successor of `predecessor' */
PRIVATE NONNULL((1, 2, 3)) void DCALL
rbtree_insert_after(RBTree *self,
                    struct rbtree_node *__restrict predecessor,
                    struct rbtree_node *__restrict newnode) {
	if (!predecessor->rbtn_rhs) {
		predecessor->rbtn_rhs = newnode;
	} else {
		predecessor = predecessor->rbtn_rhs;
		while (predecessor->rbtn_lhs)
			predecessor = predecessor->rbtn_lhs;
		predecessor->rbtn_lhs = newnode;
	}
	newnode->rbtn_par = predecessor;
	newnode->rbtn_lhs = NULL;
	newnode->rbtn_rhs = NULL;
	rbtree_node_setred(newnode);
	rbtree_abi__insert_repair(&self->rbt_root, newnode, predecessor);
}

/* Insert `newnode' as the immediate predecessor of `successor' */
PRIVATE NONNULL((1, 2, 3)) void DCALL
rbtree_insert_before(RBTree *self,
                     struct rbtree_node *__restrict successor,
                     struct rbtree_node *__restrict newnode) {
	if (!successor->rbtn_lhs) {
		successor->rbtn_lhs = newnode;
	} else {
		successor = successor->rbtn_lhs;
		while (successor->rbtn_rhs)
			successor = successor->rbtn_rhs;
		successor->rbtn_rhs = newnode;
	}
	newnode->rbtn_par = successor;
	newnode->rbtn_lhs = NULL;
	newnode->rbtn_rhs = NULL;
	rbtree_node_setred(newnode);
	rbtree_abi__insert_repair(&self->rbt_root, newnode, successor);
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
rbtree_setrange(RBTree *self, DeeObject *minkey,
                DeeObject *maxkey, DeeObject *value) {
	int error;
	struct rbtree_node_slist removed_nodes;
	struct rbtree_minmax range;
	struct rbtree_do_insert_overlap_info overlap;
	struct rbtree_node *newnode;
	bool minkey_le_minnode_minkey;
	bool maxkey_ge_maxnode_maxkey;

	/* Construct a new node for the caller-given range. */
	newnode = rbtree_node_alloc();
	if unlikely(!newnode)
		goto err;
	newnode->rbtn_minkey = minkey;
	newnode->rbtn_maxkey = maxkey;
	newnode->rbtn_value  = value;
	Dee_Incref(minkey);
	Dee_Incref(maxkey);
	Dee_Incref(value);

	/* Try to insert the new node into the tree as is (thus checking for collisions) */
again_insert:
	error = rbtree_do_insert(self, newnode, &overlap);
	if (error <= 0) {
		if unlikely(error < 0)
			goto err_newnode;
		return error; /* No conflicts, so `newnode' was inserted into the tree! */
	}

	/* In this case, there it at least 1 other pre-existing
	 * node that overlaps with the caller's key-range, and
	 * we've already been given one of those nodes in `overlap' */
	RBTree_LockRead(self);
	if unlikely(self->rbt_version != overlap.rbtdioi_vers) {
endread_and_again_insert:
		RBTree_LockEndRead(self);
		goto again_insert;
	}

	/* Expand the 1 overlapping node to get *all* overlapping nodes. */
	error = rbtree_do_minmaxlocate_node_in_root(self, minkey, maxkey, &range,
	                                            overlap.rbtdioi_node);
	if unlikely(error != 0) {
		if unlikely(error < 0)
			goto err_newnode;
		/* Version changed -> try to do the insert again. */
		goto endread_and_again_insert;
	}

	/* At this point, we've got the complete set of nodes that overlap
	 * with the caller-given key-range saved in `range'. We must now
	 * check how/where we (might) need to split these nodes.
	 *
	 * For this purpose, we must check if the caller's key-range forms
	 * a full-, or partial overlap with the range of existing nodes.
	 *
	 * -> Figure out `minkey_le_minnode_minkey' and `maxkey_ge_maxnode_maxkey' */
	{
		DREF DeeObject *minnode_minkey;
		DREF DeeObject *maxnode_maxkey;
		minnode_minkey = rbtree_node_get_minkey(range.rbtmm_min);
		maxnode_maxkey = rbtree_node_get_maxkey(range.rbtmm_max);
		Dee_Incref(minnode_minkey);
		Dee_Incref(maxnode_maxkey);
		RBTree_LockEndRead(self);

		/* Check for full overlap on lower bound */
		error = DeeObject_CmpLeAsBool(minkey, minnode_minkey);
		Dee_Decref_unlikely(minnode_minkey);
		if unlikely(error < 0) {
			Dee_Decref_unlikely(maxnode_maxkey);
			goto err_newnode;
		}
		minkey_le_minnode_minkey = error != 0;

		/* Check for full overlap on upper bound */
		error = DeeObject_CmpLoAsBool(maxkey, maxnode_maxkey);
		Dee_Decref_unlikely(maxnode_maxkey);
		if unlikely(error < 0)
			goto err_newnode;
		maxkey_ge_maxnode_maxkey = error == 0;

		RBTree_LockRead(self);
		if unlikely(self->rbt_version != overlap.rbtdioi_vers)
			goto endread_and_again_insert;
	}

	/* Split nodes as necessary */
	if (!minkey_le_minnode_minkey || !maxkey_ge_maxnode_maxkey) {
		DREF DeeObject *minkey_pred = NULL;
		DREF DeeObject *maxkey_succ = NULL;
		bool remove_empty;
		struct rbtree_node *remove_minnode;
		struct rbtree_node *remove_maxnode;
		RBTree_LockEndRead(self);
		if (!minkey_le_minnode_minkey) {
			minkey_pred = DeeObject_Predecessor(minkey);
			if unlikely(!minkey_pred)
				goto err_newnode;
		}
		if (!maxkey_ge_maxnode_maxkey) {
			maxkey_succ = DeeObject_Successor(maxkey);
			if unlikely(!maxkey_succ) {
				Dee_XDecref(minkey_pred);
				goto err_newnode;
			}
		}

		if (minkey_pred && maxkey_succ && (range.rbtmm_min == range.rbtmm_max)) {
			/* Special case: Partial overlap on *both* sides with the same node:
			 * >> local rb = RBTree();
			 * >> rb[10:20] = "foo";
			 * >> rb[12:18] = "bar"; // We are here right now
			 * >> print repr rb;     // RBTree({ [10:11]: "foo", [12:18]: "bar", [19:20]: "foo" })
			 */
			struct rbtree_node *lonode;
			struct rbtree_node *hinode;
			lonode = range.rbtmm_min;
			hinode = rbtree_node_alloc();
			if unlikely(!hinode)
				goto err_newnode;
			RBTree_LockWrite(self);
			if unlikely(self->rbt_version != overlap.rbtdioi_vers) {
				RBTree_LockEndWrite(self);
				rbtree_node_free(hinode);
				Dee_Decref(maxkey_succ);
				Dee_Decref(minkey_pred);
				goto again_insert;
			}

			/* Fill in `hinode' and update `lonode' */
			hinode->rbtn_maxkey = rbtree_node_get_maxkey(lonode); /* Inherit reference */
			hinode->rbtn_minkey = maxkey_succ;                    /* Inherit reference */
			lonode->rbtn_maxkey = minkey_pred;                    /* Inherit reference */
			hinode->rbtn_value  = rbtree_node_get_value(lonode);
			Dee_Incref(hinode->rbtn_value);

			/* Insert `newnode' as the immediate successor of `lonode' */
			rbtree_insert_after(self, lonode, newnode);

			/* Insert `hinode' as the immediate successor of `newnode' */
			rbtree_insert_after(self, newnode, hinode);

			/* Try to merge `newnode' with its neighbors. */
			SLIST_INIT(&removed_nodes);
			rbtree_do_mergenode(self, newnode, &removed_nodes);

			/* And we're done -> increment the version counter. */
			++self->rbt_version;
			RBTree_LockEndWrite(self);
			rbtree_node_slist_destroyall(&removed_nodes);
			return 0;
		}

		RBTree_LockWrite(self);
		if unlikely(self->rbt_version != overlap.rbtdioi_vers) {
			RBTree_LockEndWrite(self);
			Dee_XDecref(maxkey_succ);
			Dee_XDecref(minkey_pred);
			goto again_insert;
		}

		/* Figure out which nodes will need to be removed */
		remove_empty   = false;
		remove_minnode = range.rbtmm_min;
		remove_maxnode = range.rbtmm_max;
		if (minkey_pred) {
			if (remove_minnode == remove_maxnode)
				remove_empty = true;
			SWAP(&range.rbtmm_min->rbtn_maxkey, &minkey_pred);
			remove_minnode = rbtree_abi_nextnode(range.rbtmm_min);
		}
		if (maxkey_succ) {
			if (remove_minnode == remove_maxnode)
				remove_empty = true;
			SWAP(&range.rbtmm_max->rbtn_minkey, &maxkey_succ);
			remove_maxnode = rbtree_abi_prevnode(range.rbtmm_max);
		}

		/* Remove all nodes within the range [remove_minnode,remove_maxnode] */
		SLIST_INIT(&removed_nodes);
		if (remove_empty) {
			/* Special case: no nodes need to be removed. */
		} else {
			struct rbtree_node *iter = remove_minnode;
			for (;;) {
				struct rbtree_node *next;
				if (iter == remove_maxnode) {
					rbtree_abi_removenode(&self->rbt_root, iter);
					SLIST_INSERT(&removed_nodes, iter, rbtn_link);
					break;
				}
				next = rbtree_abi_nextnode(iter);
				rbtree_abi_removenode(&self->rbt_root, iter);
				SLIST_INSERT(&removed_nodes, iter, rbtn_link);
				iter = next;
			}
		}

		/* Insert `newnode' as the immediate successor of `range.rbtmm_min',
		 * but only if `range.rbtmm_min' wasn't just removed. If it was,
		 * then we can assume that `range.rbtmm_max' wasn't removed (since
		 * that would be the case where there are no overlaps, which is
		 * handled below, as it doesn't require nodes being split), which
		 * means that we can just insert *before* `range.rbtmm_max' */
		if (minkey_pred) {
			rbtree_insert_after(self, range.rbtmm_min, newnode);
		} else {
			rbtree_insert_before(self, range.rbtmm_max, newnode);
		}

		/* Try to merge `newnode' with its neighbors. */
		rbtree_do_mergenode(self, newnode, &removed_nodes);

		/* And we're done -> increment the version counter. */
		++self->rbt_version;
		RBTree_LockEndWrite(self);

		/* Cleanup... */
		Dee_XDecref(maxkey_succ);
		Dee_XDecref(minkey_pred);
		rbtree_node_slist_destroyall(&removed_nodes);
		return 0;
	}

	/* Upgrade to a write-lock. */
	if (!RBTree_LockUpgrade(self)) {
		if unlikely(self->rbt_version != overlap.rbtdioi_vers) {
/*endwrite_and_again_insert:*/
			RBTree_LockEndWrite(self);
			goto again_insert;
		}
	}

	/* Remove old nodes (except for the first one, since
	 * we intend to simply override that one so we don't
	 * have to do another insert operation). */
	SLIST_INIT(&removed_nodes);
	if (range.rbtmm_min != range.rbtmm_max) {
		struct rbtree_node *iter;
		iter = rbtree_abi_nextnode(range.rbtmm_min);
		for (;;) {
			struct rbtree_node *next;
			ASSERT(iter);
			if (iter == range.rbtmm_max) {
				rbtree_abi_removenode(&self->rbt_root, iter);
				SLIST_INSERT(&removed_nodes, iter, rbtn_link);
				break;
			}
			next = rbtree_abi_nextnode(iter);
			rbtree_abi_removenode(&self->rbt_root, iter);
			SLIST_INSERT(&removed_nodes, iter, rbtn_link);
			iter = next;
		}
	}

	/* Turn the first node from the pre-existing range into the new node
	 * In an intrusive R/B-tree, we'd need to swap the actual nodes (though
	 * that would also be simple, since there are parent-pointers) */
	SWAP(&range.rbtmm_min->rbtn_minkey, &newnode->rbtn_minkey);
	SWAP(&range.rbtmm_min->rbtn_maxkey, &newnode->rbtn_maxkey);
	SWAP(&range.rbtmm_min->rbtn_value, &newnode->rbtn_value);
	if (rbtree_node_isred(newnode)) {
		/* Keep red-bit */
		rbtree_node_setred(range.rbtmm_min);
		rbtree_node_setblack(newnode);
	} else {
		ASSERT(!rbtree_node_isred(range.rbtmm_min));
	}
	SLIST_INSERT(&removed_nodes, newnode, rbtn_link);

	/* And we're done -> increment the version counter. */
	++self->rbt_version;
	RBTree_LockEndWrite(self);

	/* Cleanup... */
	rbtree_node_slist_destroyall(&removed_nodes);
	return 0;
err_newnode:
	rbtree_node_destroy(newnode);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
rbtree_delrange(RBTree *self, DeeObject *minkey, DeeObject *maxkey) {
	int error;
	uintptr_t version;
	struct rbtree_minmax range;
	struct rbtree_node_slist removed_nodes;
	bool minkey_le_minnode_minkey;
	bool maxkey_ge_maxnode_maxkey;

	/* Construct a new node for the caller-given range. */
again_minmaxlocate:
	error = rbtree_do_minmaxlocate_node(self, minkey, maxkey, &range);
	if (error != 0) {
		if unlikely(error < 0)
			goto err;
		/* No nodes exist in the given range -> nothing to do. */
		return 0;
	}
	version = self->rbt_version;

	/* At this point, we've got the complete set of nodes that overlap
	 * with the caller-given key-range saved in `range'. We must now
	 * check how/where we (might) need to split these nodes.
	 *
	 * For this purpose, we must check if the caller's key-range forms
	 * a full-, or partial overlap with the range of existing nodes.
	 *
	 * -> Figure out `minkey_le_minnode_minkey' and `maxkey_ge_maxnode_maxkey' */
	{
		DREF DeeObject *minnode_minkey;
		DREF DeeObject *maxnode_maxkey;
		minnode_minkey = rbtree_node_get_minkey(range.rbtmm_min);
		maxnode_maxkey = rbtree_node_get_maxkey(range.rbtmm_max);
		Dee_Incref(minnode_minkey);
		Dee_Incref(maxnode_maxkey);
		RBTree_LockEndRead(self);

		/* Check for full overlap on lower bound */
		error = DeeObject_CmpLeAsBool(minkey, minnode_minkey);
		Dee_Decref_unlikely(minnode_minkey);
		if unlikely(error < 0) {
			Dee_Decref_unlikely(maxnode_maxkey);
			goto err;
		}
		minkey_le_minnode_minkey = error != 0;

		/* Check for full overlap on upper bound */
		error = DeeObject_CmpLoAsBool(maxkey, maxnode_maxkey);
		Dee_Decref_unlikely(maxnode_maxkey);
		if unlikely(error < 0)
			goto err;
		maxkey_ge_maxnode_maxkey = error == 0;

		RBTree_LockRead(self);
		if unlikely(self->rbt_version != version) {
/*endread_and_again_minmaxlocate:*/
			RBTree_LockEndRead(self);
			goto again_minmaxlocate;
		}
	}

	/* Split nodes as necessary */
	if (!minkey_le_minnode_minkey || !maxkey_ge_maxnode_maxkey) {
		DREF DeeObject *minkey_pred = NULL;
		DREF DeeObject *maxkey_succ = NULL;
		bool remove_empty;
		struct rbtree_node *remove_minnode;
		struct rbtree_node *remove_maxnode;
		RBTree_LockEndRead(self);
		if (!minkey_le_minnode_minkey) {
			minkey_pred = DeeObject_Predecessor(minkey);
			if unlikely(!minkey_pred)
				goto err;
		}
		if (!maxkey_ge_maxnode_maxkey) {
			maxkey_succ = DeeObject_Successor(maxkey);
			if unlikely(!maxkey_succ) {
				Dee_XDecref(minkey_pred);
				goto err;
			}
		}

		if (minkey_pred && maxkey_succ && (range.rbtmm_min == range.rbtmm_max)) {
			/* Special case: Partial overlap on *both* sides with the same node:
			 * >> local rb = RBTree();
			 * >> rb[10:20] = "foo";
			 * >> del rb[12:18]; // We are here right now
			 * >> print repr rb; // RBTree({ [10:11]: "foo", [19:20]: "foo" })
			 */
			struct rbtree_node *lonode;
			struct rbtree_node *hinode;
			lonode = range.rbtmm_min;
			hinode = rbtree_node_alloc();
			if unlikely(!hinode)
				goto err;
			RBTree_LockWrite(self);
			if unlikely(self->rbt_version != version) {
				RBTree_LockEndWrite(self);
				rbtree_node_free(hinode);
				Dee_Decref(maxkey_succ);
				Dee_Decref(minkey_pred);
				goto again_minmaxlocate;
			}

			/* Fill in `hinode' and update `lonode' */
			hinode->rbtn_maxkey = rbtree_node_get_maxkey(lonode); /* Inherit reference */
			hinode->rbtn_minkey = maxkey_succ;                    /* Inherit reference */
			lonode->rbtn_maxkey = minkey_pred;                    /* Inherit reference */
			hinode->rbtn_value  = rbtree_node_get_value(lonode);
			Dee_Incref(hinode->rbtn_value);

			/* Insert `hinode' as the immediate successor of `lonode' */
			rbtree_insert_after(self, lonode, hinode);

			/* And we're done -> increment the version counter. */
			++self->rbt_version;
			RBTree_LockEndWrite(self);
			return 0;
		}

		RBTree_LockWrite(self);
		if unlikely(self->rbt_version != version) {
			RBTree_LockEndWrite(self);
			Dee_XDecref(maxkey_succ);
			Dee_XDecref(minkey_pred);
			goto again_minmaxlocate;
		}

		/* Figure out which nodes will need to be removed */
		remove_empty   = false;
		remove_minnode = range.rbtmm_min;
		remove_maxnode = range.rbtmm_max;
		if (minkey_pred) {
			if (remove_minnode == remove_maxnode)
				remove_empty = true;
			SWAP(&range.rbtmm_min->rbtn_maxkey, &minkey_pred);
			remove_minnode = rbtree_abi_nextnode(range.rbtmm_min);
		}
		if (maxkey_succ) {
			if (remove_minnode == remove_maxnode)
				remove_empty = true;
			SWAP(&range.rbtmm_max->rbtn_minkey, &maxkey_succ);
			remove_maxnode = rbtree_abi_prevnode(range.rbtmm_max);
		}

		/* Remove all nodes within the range [remove_minnode,remove_maxnode] */
		SLIST_INIT(&removed_nodes);
		if (remove_empty) {
			/* Special case: no nodes need to be removed. */
		} else {
			struct rbtree_node *iter = remove_minnode;
			for (;;) {
				struct rbtree_node *next;
				if (iter == remove_maxnode) {
					rbtree_abi_removenode(&self->rbt_root, iter);
					SLIST_INSERT(&removed_nodes, iter, rbtn_link);
					break;
				}
				next = rbtree_abi_nextnode(iter);
				rbtree_abi_removenode(&self->rbt_root, iter);
				SLIST_INSERT(&removed_nodes, iter, rbtn_link);
				iter = next;
			}
		}

		/* And we're done -> increment the version counter. */
		++self->rbt_version;
		RBTree_LockEndWrite(self);

		/* Cleanup... */
		Dee_XDecref(maxkey_succ);
		Dee_XDecref(minkey_pred);
		rbtree_node_slist_destroyall(&removed_nodes);
		return 0;
	}

	/* Upgrade to a write-lock. */
	if (!RBTree_LockUpgrade(self)) {
		if unlikely(self->rbt_version != version) {
/*endwrite_and_again_insert:*/
			RBTree_LockEndWrite(self);
			goto again_minmaxlocate;
		}
	}

	/* Remove old nodes */
	SLIST_INIT(&removed_nodes);
	{
		struct rbtree_node *iter = range.rbtmm_min;
		for (;;) {
			struct rbtree_node *next;
			ASSERT(iter);
			if (iter == range.rbtmm_max) {
				rbtree_abi_removenode(&self->rbt_root, iter);
				SLIST_INSERT(&removed_nodes, iter, rbtn_link);
				break;
			}
			next = rbtree_abi_nextnode(iter);
			rbtree_abi_removenode(&self->rbt_root, iter);
			SLIST_INSERT(&removed_nodes, iter, rbtn_link);
			iter = next;
		}
	}

	/* And we're done -> increment the version counter. */
	++self->rbt_version;
	RBTree_LockEndWrite(self);

	/* Cleanup... */
	rbtree_node_slist_destroyall(&removed_nodes);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
rbtree_setitem(RBTree *self, DeeObject *key, DeeObject *value) {
	return rbtree_setrange(self, key, key, value);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rbtree_delitem(RBTree *self, DeeObject *key) {
	return rbtree_delrange(self, key, key);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rbtree_locate(RBTree *self, size_t argc, DeeObject *const *argv) {
	struct rbtree_node *node;
	DREF DeeTupleObject *result;
	DeeObject *key;
	DeeArg_Unpack1(err, argc, argv, "locate", &key);
	result = DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
	node = rbtree_trygetitem_node(self, key);
	if (!ITER_ISOK(node)) {
		DeeTuple_FreeUninitialized(result);
		if unlikely(!node)
			goto err;
		return_none;
	}
	DeeTuple_SET(result, 0, rbtree_node_get_minkey(node));
	DeeTuple_SET(result, 1, rbtree_node_get_maxkey(node));
	DeeTuple_SET(result, 2, rbtree_node_get_value(node));
	Dee_Increfv(DeeTuple_ELEM(result), 3);
	RBTree_LockEndRead(self);
	return Dee_AsObject(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
rbtree_rlocate(RBTree *self, size_t argc, DeeObject *const *argv) {
	DeeObject *minkey, *maxkey;
	DeeArg_Unpack2(err, argc, argv, "rlocate", &minkey, &maxkey);
	(void)self;
	/* TODO */
	DeeError_NOTIMPLEMENTED();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rbtree_itlocate(RBTree *self, size_t argc, DeeObject *const *argv) {
	struct rbtree_node *node;
	DREF RBTreeIterator *result;
	DeeObject *key;
	DeeArg_Unpack1(err, argc, argv, "itlocate", &key);
	result = DeeObject_MALLOC(RBTreeIterator);
	if unlikely(!result)
		goto err;
	node = rbtree_trygetitem_node(self, key);
	if (!ITER_ISOK(node)) {
		DeeObject_FREE(result);
		if unlikely(!node)
			goto err;
		return_none;
	}
	result->rbti_version = self->rbt_version;
	result->rbti_next    = node;
	RBTree_LockEndRead(self);
	result->rbti_tree = self;
	Dee_Incref(self);
	DeeObject_Init(result, &RBTreeIterator_Type);
	return Dee_AsObject(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rbtree_remove(RBTree *self, size_t argc, DeeObject *const *argv) {
	struct rbtree_node *node;
	DREF DeeTupleObject *result;
	DeeObject *key;
	DeeArg_Unpack1(err, argc, argv, "locate", &key);
	result = DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
again_locate_node:
	node = rbtree_trygetitem_node(self, key);
	if (!ITER_ISOK(node)) {
		DeeTuple_FreeUninitialized(result);
		if unlikely(!node)
			goto err;
		return_none;
	}
	if (!RBTree_LockTryUpgrade(self)) {
		uintptr_t version = self->rbt_version;
		if (!RBTree_LockUpgrade(self)) {
			if (version != self->rbt_version) {
				RBTree_LockEndWrite(self);
				goto again_locate_node;
			}
		}
	}

	/* Remove the node from the tree. */
	rbtree_abi_removenode(&self->rbt_root, node);
	++self->rbt_version;
	RBTree_LockEndWrite(self);
	DeeTuple_SET(result, 0, rbtree_node_get_minkey(node)); /* Inherit reference */
	DeeTuple_SET(result, 1, rbtree_node_get_maxkey(node)); /* Inherit reference */
	DeeTuple_SET(result, 2, rbtree_node_get_value(node));  /* Inherit reference */
	rbtree_node_free(node); /* NOTE: free; not destroy! (because we stole references) */
	return Dee_AsObject(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rbtree_rremove(RBTree *self, size_t argc, DeeObject *const *argv) {
	DeeObject *minkey, *maxkey;
	DeeArg_Unpack2(err, argc, argv, "rremove", &minkey, &maxkey);
	(void)self;
	/* TODO */
	DeeError_NOTIMPLEMENTED();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rbtree_prevnode(RBTree *self, size_t argc, DeeObject *const *argv) {
	struct rbtree_node *node;
	DREF DeeTupleObject *result;
	DeeObject *key;
	DeeArg_Unpack1(err, argc, argv, "prevnode", &key);
	result = DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
	node = rbtree_trygetitem_node(self, key);
	if (!ITER_ISOK(node)) {
		DeeTuple_FreeUninitialized(result);
		if unlikely(!node)
			goto err;
		return_none;
	}
	node = rbtree_abi_prevnode(node);
	if (!node) {
		RBTree_LockEndRead(self);
		DeeTuple_FreeUninitialized(result);
		return_none;
	}
	DeeTuple_SET(result, 0, rbtree_node_get_minkey(node));
	DeeTuple_SET(result, 1, rbtree_node_get_maxkey(node));
	DeeTuple_SET(result, 2, rbtree_node_get_value(node));
	Dee_Increfv(DeeTuple_ELEM(result), 3);
	RBTree_LockEndRead(self);
	return Dee_AsObject(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rbtree_nextnode(RBTree *self, size_t argc, DeeObject *const *argv) {
	struct rbtree_node *node;
	DREF DeeTupleObject *result;
	DeeObject *key;
	DeeArg_Unpack1(err, argc, argv, "nextnode", &key);
	result = DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
	node = rbtree_trygetitem_node(self, key);
	if (!ITER_ISOK(node)) {
		DeeTuple_FreeUninitialized(result);
		if unlikely(!node)
			goto err;
		return_none;
	}
	node = rbtree_abi_nextnode(node);
	if (!node) {
		RBTree_LockEndRead(self);
		DeeTuple_FreeUninitialized(result);
		return_none;
	}
	DeeTuple_SET(result, 0, rbtree_node_get_minkey(node));
	DeeTuple_SET(result, 1, rbtree_node_get_maxkey(node));
	DeeTuple_SET(result, 2, rbtree_node_get_value(node));
	Dee_Increfv(DeeTuple_ELEM(result), 3);
	RBTree_LockEndRead(self);
	return Dee_AsObject(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rbtree_itprevnode(RBTree *self, size_t argc, DeeObject *const *argv) {
	struct rbtree_node *node;
	DREF RBTreeIterator *result;
	DeeObject *key;
	DeeArg_Unpack1(err, argc, argv, "itprevnode", &key);
	result = DeeObject_MALLOC(RBTreeIterator);
	if unlikely(!result)
		goto err;
	node = rbtree_trygetitem_node(self, key);
	if (!ITER_ISOK(node)) {
		DeeObject_FREE(result);
		if unlikely(!node)
			goto err;
		return_none;
	}
	node = rbtree_abi_prevnode(node);
	if (!node) {
		RBTree_LockEndRead(self);
		DeeObject_FREE(result);
		return_none;
	}
	result->rbti_version = self->rbt_version;
	result->rbti_next    = node;
	RBTree_LockEndRead(self);
	result->rbti_tree = self;
	Dee_Incref(self);
	DeeObject_Init(result, &RBTreeIterator_Type);
	return Dee_AsObject(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rbtree_itnextnode(RBTree *self, size_t argc, DeeObject *const *argv) {
	struct rbtree_node *node;
	DREF RBTreeIterator *result;
	DeeObject *key;
	DeeArg_Unpack1(err, argc, argv, "itnextnode", &key);
	result = DeeObject_MALLOC(RBTreeIterator);
	if unlikely(!result)
		goto err;
	node = rbtree_trygetitem_node(self, key);
	if (!ITER_ISOK(node)) {
		DeeObject_FREE(result);
		if unlikely(!node)
			goto err;
		return_none;
	}
	node = rbtree_abi_nextnode(node);
	if (!node) {
		RBTree_LockEndRead(self);
		DeeObject_FREE(result);
		return_none;
	}
	result->rbti_version = self->rbt_version;
	result->rbti_next    = node;
	RBTree_LockEndRead(self);
	result->rbti_tree = self;
	Dee_Incref(self);
	DeeObject_Init(result, &RBTreeIterator_Type);
	return Dee_AsObject(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rbtree_popfront(RBTree *self, size_t argc, DeeObject *const *argv) {
	struct rbtree_node *node;
	DREF DeeTupleObject *result;
	DeeArg_Unpack0(err, argc, argv, "popfront");
	result = DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
	RBTree_LockWrite(self);
	node = self->rbt_root;
	if unlikely(!node) {
		RBTree_LockEndWrite(self);
		goto err_empty;
	}
	while (node->rbtn_lhs)
		node = node->rbtn_lhs;
	rbtree_abi_removenode(&self->rbt_root, node);
	++self->rbt_version;
	RBTree_LockEndWrite(self);
	DeeTuple_SET(result, 0, rbtree_node_get_minkey(node)); /* Inherit reference */
	DeeTuple_SET(result, 1, rbtree_node_get_maxkey(node)); /* Inherit reference */
	DeeTuple_SET(result, 2, rbtree_node_get_value(node));  /* Inherit reference */
	rbtree_node_free(node); /* NOTE: free; not destroy! (because we stole references) */
	return Dee_AsObject(result);
err_empty:
	DeeRT_ErrEmptySequence(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rbtree_popback(RBTree *self, size_t argc, DeeObject *const *argv) {
	struct rbtree_node *node;
	DREF DeeTupleObject *result;
	DeeArg_Unpack0(err, argc, argv, "popback");
	result = DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
	RBTree_LockWrite(self);
	node = self->rbt_root;
	if unlikely(!node) {
		RBTree_LockEndWrite(self);
		goto err_empty;
	}
	while (node->rbtn_rhs)
		node = node->rbtn_rhs;
	rbtree_abi_removenode(&self->rbt_root, node);
	++self->rbt_version;
	RBTree_LockEndWrite(self);
	DeeTuple_SET(result, 0, rbtree_node_get_minkey(node)); /* Inherit reference */
	DeeTuple_SET(result, 1, rbtree_node_get_maxkey(node)); /* Inherit reference */
	DeeTuple_SET(result, 2, rbtree_node_get_value(node));  /* Inherit reference */
	rbtree_node_free(node); /* NOTE: free; not destroy! (because we stole references) */
	return Dee_AsObject(result);
err_empty:
	DeeRT_ErrEmptySequence(self);
err:
	return NULL;
}

PRIVATE NONNULL((1)) void DCALL
rbtree_node_destroy_tree(struct rbtree_node *__restrict self) {
	struct rbtree_node *lhs;
	struct rbtree_node *rhs;
again:
	lhs = self->rbtn_lhs;
	rhs = self->rbtn_rhs;
	rbtree_node_destroy(self);
	if (lhs) {
		if (rhs)
			rbtree_node_destroy_tree(rhs);
		self = lhs;
		goto again;
	}
	if (rhs) {
		self = rhs;
		goto again;
	}
}

PRIVATE NONNULL((1)) void DCALL
rbtree_clear(RBTree *self) {
	struct rbtree_node *tree;
	RBTree_LockWrite(self);
	tree = self->rbt_root;
	if (!tree) {
		RBTree_LockEndWrite(self);
		return;
	}
	self->rbt_root = NULL;
	++self->rbt_version;
	RBTree_LockEndWrite(self);
	rbtree_node_destroy_tree(tree);
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
rbtree_optimize_impl(RBTree *self) {
	struct rbtree_node_slist removed_nodes;
	size_t i, position = 0;
	uintptr_t version;
	struct rbtree_node *prev, *next;
	SLIST_INIT(&removed_nodes);
again:
	RBTree_LockRead(self);
	prev    = self->rbt_root;
	version = self->rbt_version;
	if (!prev)
		goto done_unlock;
	while (prev->rbtn_lhs)
		prev = prev->rbtn_lhs;
	for (i = 0; i < position; ++i) {
		prev = rbtree_abi_nextnode(prev);
		if unlikely(!prev)
			goto done_unlock;
	}

	/* Try to merge `node' with its successor */
	for (;;) {
again_nextnode:
		next = rbtree_abi_nextnode(prev);
		if (!next)
			goto done_unlock;
		if (rbtree_node_get_value(prev) == rbtree_node_get_value(next)) {
			int temp;
			DREF DeeObject *prev_maxkey, *prev_maxkey_succ;
			DREF DeeObject *next_minkey;
			prev_maxkey = rbtree_node_get_maxkey(prev);
			next_minkey = rbtree_node_get_minkey(next);
			RBTree_LockEndRead(self);

			/* Increment the max-key of the predecessor */
			prev_maxkey_succ = DeeObject_Successor(prev_maxkey);
			Dee_Decref(prev_maxkey);
			if unlikely(!prev_maxkey_succ) {
				Dee_Decref(next_minkey);
				goto err;
			}

			/* Compare keys */
			temp = DeeObject_TryCompareEq(prev_maxkey_succ, next_minkey);
			Dee_Decref(prev_maxkey_succ);
			Dee_Decref(next_minkey);
			if (Dee_COMPARE_ISERR(temp))
				goto err;
			if (Dee_COMPARE_ISEQ(temp)) {
				/* Yes! can merge these 2 nodes! */
				RBTree_LockWrite(self);
				if unlikely(version != self->rbt_version) {
					RBTree_LockEndWrite(self);
					goto check_interrupt_and_again;
				}

				/* Remove the node from the tree. */
				rbtree_abi_removenode(&self->rbt_root, next);

				/* Update the node which we're keeping to reflect the grown range's size. */
				SWAP(&prev->rbtn_maxkey, &next->rbtn_maxkey);

				/* Track the removed node for later disposal. */
				SLIST_INSERT(&removed_nodes, next, rbtn_link);

				++self->rbt_version;
				version = self->rbt_version;
				RBTree_LockDowngrade(self);
				goto again_nextnode;
			}

			/* Just keep going as normal */
			RBTree_LockRead(self);
			if unlikely(version != self->rbt_version) {
				RBTree_LockEndRead(self);
				goto check_interrupt_and_again;
			}
		} /* if (rbtree_node_get_value(prev) == rbtree_node_get_value(next)) */

		/* Can't merge these nodes. */
		prev = next;
		++position;
	}

	/* Done... */
done_unlock:
	RBTree_LockEndRead(self);
	rbtree_node_slist_destroyall(&removed_nodes);
	return 0;
check_interrupt_and_again:
	/* Just to be safe: put an interrupt-check in here! */
	if (DeeThread_CheckInterrupt())
		goto err;
	goto again;
err:
	rbtree_node_slist_destroyall(&removed_nodes);
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rbtree_optimize(RBTree *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "clear");
	if unlikely(rbtree_optimize_impl(self))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rbtree_doclear(RBTree *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "clear");
	rbtree_clear(self);
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rbtree_pop(RBTree *self, size_t argc, DeeObject *const *argv) {
	DeeObject *key, *def = ITER_DONE;
	DeeArg_Unpack1Or2(err, argc, argv, "pop", &key, &def);
	(void)self;
	/* TODO */
	DeeError_NOTIMPLEMENTED();
err:
	return NULL;
}

#define rbtree_popitem rbtree_popfront

/*[[[deemon
(PRIVATE_DEFINE_STRING from rt.gen.string)("str___start__", "__start__");
(PRIVATE_DEFINE_STRING from rt.gen.string)("str___end__", "__end__");
]]]*/
PRIVATE DEFINE_STRING_EX(str___start__, "__start__", 0xee25ff9d, 0x2e9bad284bd1e7d2);
PRIVATE DEFINE_STRING_EX(str___end__, "__end__", 0x226048a5, 0x5b8da67432fe5d43);
/*[[[end]]]*/

PRIVATE NONNULL((1, 2)) int DCALL
unpack_range(DeeObject *range, /*out*/ DREF DeeObject *start_end[2]) {
	start_end[0] = DeeObject_GetAttr(range, Dee_AsObject(&str___start__));
	if unlikely(!start_end[0])
		goto err;
	start_end[1] = DeeObject_GetAttr(range, Dee_AsObject(&str___end__));
	if unlikely(!start_end[1])
		goto err_0;
	return 0;
err_0:
	Dee_Decref(start_end[0]);
err:
	return -1;
}

/* Copy triples from `iter' into `self' */
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
rbtree_insert_foreach_cb(void *arg, DeeObject *item) {
	int temp;
	size_t count;
	DREF DeeObject *items[3];
	RBTree *self = (RBTree *)arg;
	count = DeeObject_InvokeMethodHint(seq_unpack_ex, item, 2, 3, items);
	if unlikely(count == (size_t)-1)
		goto err;
	ASSERT(count == 2 || count == 3);
	if (count == 2) {
		/* In this case, the first item is expected to be a range-object without a step. */
		DREF DeeObject *range;
		int unpack_err;
		items[2] = items[1];
		range    = items[0];
		unpack_err = unpack_range(range, items);
		Dee_Decref(range);
		if unlikely(unpack_err) {
			Dee_Decref(items[2]);
			goto err;
		}
	}

	/* At this point, `items' is {minkey, maxkey, value}
	 * Use these values to fill in a range. */
	temp = rbtree_setrange(self, items[0], items[1], items[2]);
	Dee_Decrefv_unlikely(items, 3);
	return (Dee_ssize_t)temp;
err:
	return -1;
}

/* Copy triples from `seq' into `self' */
#define rbtree_insert_sequence(self, seq) \
	DeeObject_Foreach(seq, &rbtree_insert_foreach_cb, self)

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rbtree_update(RBTree *self, size_t argc, DeeObject *const *argv) {
	DeeObject *items;
	DeeArg_Unpack1(err, argc, argv, "update", &items);
	if unlikely(rbtree_insert_sequence(self, items))
		goto err;
	return_none;
err:
	return NULL;
}



PRIVATE WUNUSED NONNULL((1)) DREF RBTreeIterator *DCALL
rbtree_get_itroot(RBTree *__restrict self) {
	DREF RBTreeIterator *result;
	result = DeeObject_MALLOC(RBTreeIterator);
	if unlikely(!result)
		goto done;
	RBTree_LockRead(self);
	if unlikely(!self->rbt_root)
		goto err_r_unlock_empty;
	result->rbti_next    = self->rbt_root;
	result->rbti_version = self->rbt_version;
	RBTree_LockEndRead(self);
	result->rbti_tree = self;
	Dee_Incref(self);
	DeeObject_Init(result, &RBTreeIterator_Type);
done:
	return result;
err_r_unlock_empty:
	RBTree_LockEndRead(self);
	DeeObject_FREE(result);
	DeeRT_ErrEmptySequence(self);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
rbtree_get_first(RBTree *__restrict self) {
	DREF DeeTupleObject *result;
	struct rbtree_node *node;
	result = DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto done;
	RBTree_LockRead(self);
	node = self->rbt_root;
	if unlikely(!node) {
		RBTree_LockEndRead(self);
		goto err_r_empty;
	}
	while (node->rbtn_lhs)
		node = node->rbtn_lhs;
	DeeTuple_SET(result, 0, rbtree_node_get_minkey(node));
	DeeTuple_SET(result, 1, rbtree_node_get_maxkey(node));
	DeeTuple_SET(result, 2, rbtree_node_get_value(node));
	Dee_Increfv(DeeTuple_ELEM(result), 3);
	RBTree_LockEndRead(self);
done:
	return result;
err_r_empty:
	DeeTuple_FreeUninitialized(result);
	DeeRT_ErrEmptySequence(self);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
rbtree_get_last(RBTree *__restrict self) {
	DREF DeeTupleObject *result;
	struct rbtree_node *node;
	result = DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto done;
	RBTree_LockRead(self);
	node = self->rbt_root;
	if unlikely(!node) {
		RBTree_LockEndRead(self);
		goto err_r_empty;
	}
	while (node->rbtn_rhs)
		node = node->rbtn_rhs;
	DeeTuple_SET(result, 0, rbtree_node_get_minkey(node));
	DeeTuple_SET(result, 1, rbtree_node_get_maxkey(node));
	DeeTuple_SET(result, 2, rbtree_node_get_value(node));
	Dee_Increfv(DeeTuple_ELEM(result), 3);
	RBTree_LockEndRead(self);
done:
	return result;
err_r_empty:
	DeeTuple_FreeUninitialized(result);
	DeeRT_ErrEmptySequence(self);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rbtree_get_depth(RBTree *__restrict self) {
	size_t result = 0;
	struct rbtree_node *node;
	RBTree_LockRead(self);
	node = self->rbt_root;
	while (node) {
		node = node->rbtn_lhs;
		++result;
	}
	RBTree_LockEndRead(self);
	return DeeInt_NewSize(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rbtree_sizeof(RBTree *__restrict self) {
	size_t result;
	result = rbtree_size(self);
	result *= sizeof(struct rbtree_node);
	result += sizeof(RBTree);
	return DeeInt_NewSize(result);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rbtree_ctor(RBTree *__restrict self) {
	self->rbt_root    = NULL;
	self->rbt_version = 0;
	Dee_atomic_rwlock_init(&self->rbt_lock);
	return 0;
}

/* Copy the sub-tree `tree'. The caller is expected to be holding a read-lock
 * @return: * :        A duplicate of `tree' (lock is still held)
 * @return: NULL:      Error (lock was lost)
 * @return: ITER_DONE: Version error (lock was lost) */
PRIVATE WUNUSED NONNULL((1, 2)) struct rbtree_node *DCALL
rbtree_copy_subtree_locked(RBTree *__restrict self,
                           struct rbtree_node *__restrict tree) {
	struct rbtree_node *result;
	result = rbtree_node_tryalloc();
	if likely(!result) {
		uintptr_t version = self->rbt_version;
		RBTree_LockEndRead(self);
		result = rbtree_node_alloc();
		if unlikely(!result)
			return NULL;
		RBTree_LockRead(self);
		if unlikely(self->rbt_version != version) {
			RBTree_LockEndRead(self);
			rbtree_node_free(result);
			return (struct rbtree_node *)ITER_DONE;
		}
	}

	/* Recursively copy sub-trees. */
	result->rbtn_lhs = NULL;
	result->rbtn_rhs = NULL;
	if (tree->rbtn_lhs) {
		struct rbtree_node *copy;
		copy = rbtree_copy_subtree_locked(self, tree->rbtn_lhs);
		if unlikely(!ITER_ISOK((DeeObject *)copy)) {
			rbtree_node_free(result);
			return copy;
		}
		copy->rbtn_par   = result;
		result->rbtn_lhs = copy;
	}
	if (tree->rbtn_rhs) {
		struct rbtree_node *copy;
		copy = rbtree_copy_subtree_locked(self, tree->rbtn_rhs);
		if unlikely(!ITER_ISOK((DeeObject *)copy)) {
			if (result->rbtn_lhs)
				rbtree_node_destroy_tree(result->rbtn_lhs);
			rbtree_node_free(result);
			return copy;
		}
		copy->rbtn_par   = result;
		result->rbtn_rhs = copy;
	}
	/*result->rbtn_par = ...;*/ /* Filled in by the caller! */

	result->rbtn_minkey = tree->rbtn_minkey;
	result->rbtn_maxkey = tree->rbtn_maxkey;
	result->rbtn_value  = tree->rbtn_value;
	Dee_Incref(rbtree_node_get_minkey(result));
	Dee_Incref(rbtree_node_get_maxkey(result));
	Dee_Incref(rbtree_node_get_value(result));
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rbtree_copy(RBTree *__restrict self, RBTree *__restrict other) {
	struct rbtree_node *tree_copy;
again:
	RBTree_LockRead(other);
	if (other->rbt_root) {
		tree_copy = rbtree_copy_subtree_locked(other, other->rbt_root);
		if unlikely(!ITER_ISOK(tree_copy)) {
			if unlikely(!tree_copy)
				goto err; /* Error */
			goto again;   /* Version changed */
		}
		/* Root node has no parent. */
		tree_copy->rbtn_par = NULL;
	} else {
		tree_copy = NULL;
	}
	RBTree_LockEndRead(other);

	/* Fill in members of `self' */
	self->rbt_root    = tree_copy; /* Inherit */
	self->rbt_version = 0;
	Dee_atomic_rwlock_init(&self->rbt_lock);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
rbtree_fini(RBTree *__restrict self) {
	if (self->rbt_root)
		rbtree_node_destroy_tree(self->rbt_root);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rbtree_assign(RBTree *self, DeeObject *other) {
	ATTR_ALIGNED(COMPILER_ALIGNOF(RBTree))
	char buf[sizeof(RBTree) - Dee_OBJECT_OFFSETOF_DATA];
	RBTree *temp = (RBTree *)(buf - Dee_OBJECT_OFFSETOF_DATA);
	struct rbtree_node *old_tree;
	if (DeeObject_InstanceOfExact(other, &RBTree_Type)) {
		if unlikely(self == (RBTree *)other)
			goto done;
		if unlikely(rbtree_copy(temp, (RBTree *)other))
			goto err;
	} else {
		temp->rbt_root    = NULL;
		temp->rbt_version = 0;
		Dee_atomic_rwlock_init(&temp->rbt_lock);
		if unlikely(rbtree_insert_sequence(temp, other)) {
			rbtree_fini(temp);
			goto err;
		}
	}

	RBTree_LockWrite(self);
	old_tree       = self->rbt_root;
	self->rbt_root = temp->rbt_root;
	++self->rbt_version;
	RBTree_LockEndWrite(self);
	if (old_tree)
		rbtree_node_destroy_tree(old_tree);
done:
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rbtree_move_assign(RBTree *self, RBTree *other) {
	struct rbtree_node *old_tree;
	if unlikely(self == other)
		return 0;
	DeeLock_Acquire2(RBTree_LockWrite(self), RBTree_LockTryWrite(self), RBTree_LockEndWrite(self),
	                 RBTree_LockWrite(other), RBTree_LockTryWrite(other), RBTree_LockEndWrite(other));
	old_tree        = self->rbt_root;
	self->rbt_root  = other->rbt_root;
	other->rbt_root = NULL;
	++other->rbt_version;
	++self->rbt_version;
	RBTree_LockEndWrite(other);
	RBTree_LockEndWrite(self);
	if (old_tree)
		rbtree_node_destroy_tree(old_tree);
	return 0;
}

PRIVATE NONNULL((1, 2)) void DCALL
rbtree_node_visit_tree(struct rbtree_node *__restrict self, Dee_visit_t proc, void *arg) {
again:
	Dee_Visit(rbtree_node_get_minkey(self));
	Dee_Visit(rbtree_node_get_maxkey(self));
	Dee_Visit(rbtree_node_get_value(self));
	if (self->rbtn_lhs) {
		if (self->rbtn_rhs)
			rbtree_node_visit_tree(self->rbtn_rhs, proc, arg);
		self = self->rbtn_lhs;
		goto again;
	}
	if (self->rbtn_rhs) {
		self = self->rbtn_rhs;
		goto again;
	}
}

PRIVATE NONNULL((1, 2)) void DCALL
rbtree_visit(RBTree *__restrict self, Dee_visit_t proc, void *arg) {
	if (self->rbt_root)
		rbtree_node_visit_tree(self->rbt_root, proc, arg);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rbtree_init(RBTree *__restrict self, size_t argc, DeeObject *const *argv) {
	DeeObject *seq;
	DeeArg_Unpack1(err, argc, argv, "RBTree", &seq);
	self->rbt_root    = NULL;
	self->rbt_version = 0;
	Dee_atomic_rwlock_init(&self->rbt_lock);
	if unlikely(rbtree_insert_sequence(self, seq))
		goto err_self;
	return 0;
err_self:
	rbtree_fini(self);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rbtree_bool(RBTree *__restrict self) {
	struct rbtree_node *root = atomic_read(&self->rbt_root);
	return root != NULL ? 1 : 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rbtree_repr(RBTree *__restrict self) {
	struct Dee_unicode_printer p;
	struct rbtree_node *node;
	uintptr_t version;
	bool is_first;
again:
	Dee_unicode_printer_init(&p);
	if (Dee_UNICODE_PRINTER_PRINT(&p, "RBTree({ ") < 0)
		goto err;
	is_first = true;
	RBTree_LockRead(self);
	node = self->rbt_root;
	if unlikely(!node)
		goto unlock_and_stop;
	while (node->rbtn_lhs)
		node = node->rbtn_lhs;
	version = self->rbt_version;
	do {
		Dee_ssize_t temp;
		DREF DeeObject *item[3];
		item[0] = rbtree_node_get_minkey(node);
		item[1] = rbtree_node_get_maxkey(node);
		item[2] = rbtree_node_get_value(node);
		Dee_Increfv(item, 3);
		RBTree_LockEndRead(self);
		if (!is_first) {
			if unlikely(Dee_UNICODE_PRINTER_PRINT(&p, ", ") < 0) {
				Dee_Decrefv_unlikely(item, 3);
				goto err;
			}
		}
		temp = Dee_unicode_printer_printf(&p, "[%r:%r]: %r",
		                                  item[0], item[1], item[2]);
		Dee_Decrefv_unlikely(item, 3);
		if unlikely(temp < 0)
			goto err;
		is_first = false;
		RBTree_LockRead(self);
		if unlikely(version != self->rbt_version) {
			RBTree_LockEndRead(self);
			goto restart;
		}
	} while ((node = rbtree_abi_nextnode(node)) != NULL);
unlock_and_stop:
	RBTree_LockEndRead(self);
	if unlikely((is_first ? Dee_UNICODE_PRINTER_PRINT(&p, "})")
	                      : Dee_UNICODE_PRINTER_PRINT(&p, " })")) < 0)
		goto err;
	return Dee_unicode_printer_pack(&p);
restart:
	RBTree_LockEndRead(self);
	Dee_unicode_printer_fini(&p);
	goto again;
err:
	Dee_unicode_printer_fini(&p);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
rbtree_printrepr(RBTree *__restrict self, Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	struct rbtree_node *node;
	uintptr_t version;
	bool is_first;
	result = DeeFormat_PRINT(printer, arg, "RBTree({ ");
	if unlikely(result < 0)
		goto done;
	is_first = true;
	RBTree_LockRead(self);
	node = self->rbt_root;
	if unlikely(!node)
		goto unlock_and_stop;
	while (node->rbtn_lhs)
		node = node->rbtn_lhs;
	version = self->rbt_version;
	do {
		DREF DeeObject *item[3];
		item[0] = rbtree_node_get_minkey(node);
		item[1] = rbtree_node_get_maxkey(node);
		item[2] = rbtree_node_get_value(node);
		Dee_Increfv(item, 3);
		RBTree_LockEndRead(self);
		if (!is_first) {
			temp = DeeFormat_PRINT(printer, arg, ", ");
			if unlikely(temp < 0) {
				Dee_Decrefv_unlikely(item, 3);
				goto err;
			}
			result += temp;
		}
		temp = DeeFormat_Printf(printer, arg, "[%r:%r]: %r",
		                        item[0], item[1], item[2]);
		Dee_Decrefv_unlikely(item, 3);
		if unlikely(temp < 0)
			goto err;
		result += temp;
		is_first = false;
		RBTree_LockRead(self);
		if unlikely(version != self->rbt_version) {
			RBTree_LockEndRead(self);
			temp = DeeFormat_PRINT(printer, arg, ", <RBTree changed while being iterated>");
			if (temp < 0)
				goto err;
			result += temp;
			goto stop_after_changed;
		}
	} while ((node = rbtree_abi_nextnode(node)) != NULL);
unlock_and_stop:
	RBTree_LockEndRead(self);
stop_after_changed:
	temp = is_first ? DeeFormat_PRINT(printer, arg, "})")
	                : DeeFormat_PRINT(printer, arg, " })");
	if (temp < 0)
		goto err;
	result += temp;
done:
	return result;
err:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
rbtree_foreach(RBTree *self, Dee_foreach_t proc, void *arg) {
	DREF DeeTupleObject *tuple;
	DREF DeeObject *start, *end, *value;
	Dee_ssize_t temp, result = 0;
	struct rbtree_node *node;
	uintptr_t version;
	RBTree_LockRead(self);
	node = self->rbt_root;
	if unlikely(!node)
		goto unlock_and_stop;
	while (node->rbtn_lhs)
		node = node->rbtn_lhs;
	version = self->rbt_version;
	do {
		start = rbtree_node_get_minkey(node);
		end   = rbtree_node_get_maxkey(node);
		value = rbtree_node_get_value(node);
		Dee_Incref(start);
		Dee_Incref(end);
		Dee_Incref(value);
		RBTree_LockEndRead(self);
		tuple = DeeTuple_NewUninitialized(3);
		if unlikely(!tuple)
			goto err_start_end_value;
		DeeTuple_SET(tuple, 0, start); /* Inherit reference */
		DeeTuple_SET(tuple, 1, end);   /* Inherit reference */
		DeeTuple_SET(tuple, 2, value); /* Inherit reference */
		temp = (*proc)(arg, (DeeObject *)tuple);
		Dee_Decref(tuple);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		RBTree_LockRead(self);
		if unlikely(version != self->rbt_version) {
			RBTree_LockEndRead(self);
			goto stop_after_changed;
		}
	} while ((node = rbtree_abi_nextnode(node)) != NULL);
unlock_and_stop:
	RBTree_LockEndRead(self);
stop_after_changed:
	return result;
err_temp:
	return temp;
err_start_end_value:
	Dee_Decref_unlikely(value);
	Dee_Decref_unlikely(end);
	Dee_Decref_unlikely(start);
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
rbtree_foreach_pair(RBTree *self, Dee_foreach_pair_t proc, void *arg) {
	DREF DeeObject *range, *start, *end, *value;
	Dee_ssize_t temp, result = 0;
	struct rbtree_node *node;
	uintptr_t version;
	RBTree_LockRead(self);
	node = self->rbt_root;
	if unlikely(!node)
		goto unlock_and_stop;
	while (node->rbtn_lhs)
		node = node->rbtn_lhs;
	version = self->rbt_version;
	do {
		start = rbtree_node_get_minkey(node);
		end   = rbtree_node_get_maxkey(node);
		value = rbtree_node_get_value(node);
		Dee_Incref(start);
		Dee_Incref(end);
		Dee_Incref(value);
		RBTree_LockEndRead(self);
		range = DeeRange_New(start, end, NULL);
		Dee_Decref_unlikely(end);
		Dee_Decref_unlikely(start);
		if unlikely(!range)
			goto err_value;
		temp = (*proc)(arg, range, value);
		Dee_Decref_unlikely(value);
		Dee_Decref(range);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		RBTree_LockRead(self);
		if unlikely(version != self->rbt_version) {
			RBTree_LockEndRead(self);
			goto stop_after_changed;
		}
	} while ((node = rbtree_abi_nextnode(node)) != NULL);
unlock_and_stop:
	RBTree_LockEndRead(self);
stop_after_changed:
	return result;
err_temp:
	return temp;
err_value:
	Dee_Decref_unlikely(value);
	return -1;
}


PRIVATE struct type_seq rbtree_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *))&rbtree_iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rbtree_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rbtree_getitem,
	/* .tp_delitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&rbtree_delitem,
	/* .tp_setitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&rbtree_setitem,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&rbtree_delrange,
	/* .tp_setrange                   = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *, DeeObject *))&rbtree_setrange,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&rbtree_foreach,
	/* .tp_foreach_pair               = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&rbtree_foreach_pair,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *))&rbtree_size,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rbtree_trygetitem,
};

PRIVATE struct type_gc tpconst rbtree_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *__restrict))&rbtree_clear
};

PRIVATE struct type_method tpconst rbtree_methods[] = {
	/* RBTree-specific methods */
	TYPE_METHOD_F("locate", &rbtree_locate, METHOD_FNOREFESCAPE,
	              "(key)->?X2?T3?O?O?O?N\n"
	              "#r{The ${minkey,maxkey,value} triple where @key is located within $minkey and $maxkey, "
	              /**/ "or ?N when no node contains @key}"),
	TYPE_METHOD_F("rlocate", &rbtree_rlocate, METHOD_FNOREFESCAPE,
	              "(minkey,maxkey)->?S?T3?O?O?O\n"
	              "#r{A list of ${minkey,maxkey,value} triples describing nodes which overlap with the given key-range}"),
	TYPE_METHOD("itlocate", &rbtree_itlocate,
	            "(key)->?X2?#Iterator?N\n"
	            "#r{An ?#Iterator bound to the node containing @key, or ?N when no node contains @key}"),
	TYPE_METHOD_F("remove", &rbtree_remove, METHOD_FNOREFESCAPE,
	              "(key)->?X2?T3?O?O?O?N\n"
	              "#r{The ${minkey,maxkey,value} triple where @key was located within $minkey and $maxkey, "
	              /**/ "or ?N when no node contained @key}"
	              "Like ?#locate, but also remove the node"),
	TYPE_METHOD_F("rremove", &rbtree_rremove, METHOD_FNOREFESCAPE,
	              "(minkey,maxkey)->?S?T3?O?O?O\n"
	              "#r{A list of ${minkey,maxkey,value} triples describing nodes which overlapped with the given key-range}"
	              "Like ?#rlocate, but also remove the nodes"),
	TYPE_METHOD_F("prevnode", &rbtree_prevnode, METHOD_FNOREFESCAPE,
	              "(key)->?X2?T3?O?O?O?N\n"
	              "#r{The ${minkey,maxkey,value} triple of the node that comes before "
	              /**/ "the one containing @key, or ?N when no such node exists}"),
	TYPE_METHOD_F("nextnode", &rbtree_nextnode, METHOD_FNOREFESCAPE,
	              "(key)->?X2?T3?O?O?O?N\n"
	              "#r{The ${minkey,maxkey,value} triple of the node that comes after "
	              /**/ "the one containing @key, or ?N when no such node exists}"),
	TYPE_METHOD("itprevnode", &rbtree_itprevnode,
	            "(key)->?X2?#Iterator?N\n"
	            "#r{An ?#Iterator bound to the node that comes before "
	            /**/ "the one containing @key, or ?N when no such node exists}"),
	TYPE_METHOD("itnextnode", &rbtree_itnextnode,
	            "(key)->?X2?#Iterator?N\n"
	            "#r{An ?#Iterator bound to the node that comes after "
	            /**/ "the one containing @key, or ?N when no such node exists}"),

	TYPE_METHOD_F("popfront", &rbtree_popfront, METHOD_FNOREFESCAPE,
	              "->?T3?O?O?O\n"
	              DOC_ERROR_ValueError_EMPTY_SEQUENCE
	              "#r{the lowest element that used to be stored in @this ?. (s.a. ?#first)}"),
	TYPE_METHOD_F("popback", &rbtree_popback, METHOD_FNOREFESCAPE,
	              "->?T3?O?O?O\n"
	              DOC_ERROR_ValueError_EMPTY_SEQUENCE
	              "#r{the greatest element that used to be stored in @this ?. (s.a. ?#last)}"),
	TYPE_METHOD_F("optimize", &rbtree_optimize, METHOD_FNOREFESCAPE,
	              "()\n"
	              "Search for adjacent nodes that can be merged due to having identical (as per ${===}) "
	              /**/ "values, as well as consecutive key-ranges. By default, this sort of optimization "
	              /**/ "is only done when using ?Dint as key type, though if custom key types are used, "
	              /**/ "then this function may be used to perform that same optimization.\n"
	              "The reason why this is only done for ?Dint by default, is because that is the only "
	              /**/ "builtin type where there can truly be distinct predecessor/successor elements. "
	              /**/ "For example, $\"bar\" has no successor. $\"bas\" would come after it, but in-"
	              /**/ "between the two lies an infinite number of other strings, like $\"bara\" or $\"barfoo\". "
	              /**/ "Additionally, doing this sort of thing may trigger user-defined operators, which in "
	              /**/ "turn may call back to @this ?., which would prevent ?#{op:setrange} to function as an "
	              /**/ "atomic operation in regards to all other APIs (since it would be possible to access "
	              /**/ "the tree in an inconsistent state where nodes haven't been merged, yet).\n"
	              "As such, only ?Dint is merged automatically while all other types need to make use of "
	              /**/ "this function to perform that operation, though do note that this function will throw "
	              /**/ "an exception if it encounters a key that doesn't implement ${operator copy} or "
	              /**/ "${operator ++} / ${operator --} (which is another reason why auto-merging is only "
	              /**/ "done for ?Dint by default).\n"
	              "#T{There is #Bno need to call this function when using ?Dint as keys, or some type "
	              /**/ "that does not implement ${operator ++} and ${operator --} (like ?Dstring)}"),

	/* Generic methods */
	TYPE_METHOD_F("clear", &rbtree_doclear, METHOD_FNOREFESCAPE,
	              "()\n"
	              "Clear all values from @this ?."),
	TYPE_METHOD_F("pop", &rbtree_pop, METHOD_FNOREFESCAPE,
	              "(key)->\n"
	              "(key,def)->\n"
	              DOC_ERROR_NotImplemented_CANNOT_SPLIT
	              "#tKeyError{No @def was given and @key was not found}"
	              "Delete @key from @this and return its previously assigned "
	              /**/ "value or @def when @key had no item associated"),
	TYPE_METHOD_F("popitem", &rbtree_popitem, METHOD_FNOREFESCAPE,
	              "->?T3?O?O?O\n"
	              DOC_ERROR_ValueError_EMPTY_SEQUENCE
	              "#r{A random pair minkey-maxkey-value pair that has been removed}"),
	TYPE_METHOD_F("update", &rbtree_update, METHOD_FNOREFESCAPE,
	              "(items:?S?T3?O?O?O)\n"
	              DOC_ERROR_NotImplemented_CANNOT_SPLIT
	              "Iterate @items and unpack each element into 3 others, "
	              /**/ "using them as #C{minkey,maxkey,value} to insert into @this ?."),
	TYPE_METHOD_HINTREF(Mapping_setold_ex),
	TYPE_METHOD_HINTREF(Mapping_setnew_ex),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst rbtree_method_hints[] = {
	TYPE_METHOD_HINT(map_setold_ex, &rbtree_mh_setold_ex),
	TYPE_METHOD_HINT(map_setnew_ex, &rbtree_mh_setnew_ex),
	TYPE_METHOD_HINT_END
};


PRIVATE struct type_getset tpconst rbtree_getsets[] = {
	TYPE_GETTER_F("first", &rbtree_get_first, METHOD_FNOREFESCAPE,
	              "->?T3?O?O?O\n"
	              DOC_ERROR_ValueError_EMPTY_SEQUENCE
	              "Return the triple #C{minkey,maxkey,value} for the lowest range in @this ?."),
	TYPE_GETTER_F("last", &rbtree_get_last, METHOD_FNOREFESCAPE,
	              "->?T3?O?O?O\n"
	              DOC_ERROR_ValueError_EMPTY_SEQUENCE
	              "Return the triple #C{minkey,maxkey,value} for the greatest range in @this ?."),
	TYPE_GETTER_F("__root__", &rbtree_get_itroot, METHOD_FNOREFESCAPE,
	              "->?#Iterator\n"
	              DOC_ERROR_ValueError_EMPTY_SEQUENCE
	              "Return an ?#Iterator for the root node of the tree"),
	TYPE_GETTER_AB_F("__depth__", &rbtree_get_depth, METHOD_FNOREFESCAPE,
	                 "->?Dint\n"
	                 "Depth of the left-most tree node (since the tree is balanced, "
	                 /**/ "this is either the tree's max-depth, or one less than that)"),
	TYPE_GETTER_AB_F("__sizeof__", &rbtree_sizeof, METHOD_FNOREFESCAPE, "->?Dint"),
	TYPE_GETTER_AB("cached", &DeeObject_NewRef, "->?."),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst rbtree_members[] = {
	TYPE_MEMBER_FIELD_DOC("__version__", STRUCT_UINTPTR_T | STRUCT_ATOMIC | STRUCT_CONST, offsetof(RBTree, rbt_version),
	                      "Internal version number of the tree (used to invalidate iterators when the tree changes)"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst rbtree_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &RBTreeIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject RBTree_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "RBTree",
	/* .tp_doc      = */ DOC("Red/Black tree mapping type. Unlike normal mappings that work on "
	                         /**/ "distinct keys, this type of mapping works on key-ranges:\n"

	                         "${"
	                         /**/ "import RBTree from collections;\n"
	                         /**/ "local rb = RBTree();\n"
	                         /**/ "rb[10:20] = \"foo\";\n"
	                         /**/ "print rb.get(9);  /* none */\n"
	                         /**/ "print rb.get(10); /* \"foo\" */\n"
	                         /**/ "print rb.get(15); /* \"foo\" */\n"
	                         /**/ "print rb.get(20); /* \"foo\" */\n"
	                         /**/ "print rb.get(21); /* none */"
	                         "}\n"
	                         "\n"

	                         "()\n"
	                         "Construct an empty ?.\n"
	                         "\n"

	                         "(seq:?S?X2?T3?O?O?O?T2?Ert:SeqRange?O)\n"
	                         "Construct an ?. from a sequence of ${(minkey,maxkey,value)} or ${([minkey:maxkey],value)}\n"
	                         "\n"

	                         "delrange(minkey,maxkey)->\n"
	                         DOC_ERROR_NotImplemented_CANNOT_SPLIT
	                         "Delete all nodes intersecting with ${[minkey:maxkey]}, and split nodes where necessary\n"
	                         "\n"

	                         "setrange(minkey,maxkey,value)->\n"
	                         DOC_ERROR_NotImplemented_CANNOT_SPLIT
	                         "Assign @value to the key-range ${[minkey:maxkey]}\n"
	                         "\n"

	                         "getitem(key)->\n"
	                         DOC_ERROR_KeyError_NO_SUCH_KEY
	                         DOC_ERROR_NotImplemented_CANNOT_SPLIT
	                         "#r{The value that is bound to @key}\n"
	                         "\n"

	                         "delitem(key)->\n"
	                         DOC_ERROR_NotImplemented_CANNOT_SPLIT
	                         "Alias for ?#{op:delrange} called as ${del this[key:key]}\n"
	                         "\n"

	                         "setitem(key,value)->\n"
	                         DOC_ERROR_NotImplemented_CANNOT_SPLIT
	                         "Alias for ?#{op:setrange} called as ${this[key:key] = value}"
	),
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &RangeMap_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ RBTree,
			/* tp_ctor:        */ &rbtree_ctor,
			/* tp_copy_ctor:   */ &rbtree_copy,
			/* tp_any_ctor:    */ &rbtree_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* TODO */
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rbtree_fini,
		/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&rbtree_assign,
		/* .tp_move_assign = */ (int (DCALL *)(DeeObject *, DeeObject *))&rbtree_move_assign,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rbtree_repr,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&rbtree_bool,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&rbtree_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&rbtree_visit,
	/* .tp_gc            = */ &rbtree_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &rbtree_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ rbtree_methods,
	/* .tp_getsets       = */ rbtree_getsets,
	/* .tp_members       = */ rbtree_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ rbtree_class_members,
	/* .tp_method_hints  = */ rbtree_method_hints,
};

DECL_END

#endif /* !GUARD_DEX_COLLECTIONS_RBTREE_C */
