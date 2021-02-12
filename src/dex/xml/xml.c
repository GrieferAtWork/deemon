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
#ifndef GUARD_DEX_XML_XML_C
#define GUARD_DEX_XML_XML_C 1
#define CONFIG_BUILDING_LIBXML 1
#define DEE_SOURCE 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

#include "libxml.h"
/**/

#include <deemon/alloc.h>
#include <deemon/object.h>
#include <deemon/stringutils.h>
#include <deemon/system-features.h> /* memmem() */

DECL_BEGIN

#ifndef CONFIG_HAVE_memmem
#define memmem dee_memmem
DeeSystem_DEFINE_memmem(dee_memmem)
#endif /* !CONFIG_HAVE_memmem */


INTERN NONNULL((1)) void DCALL
XMLNode_Destroy(XMLNode *__restrict self) {
	DREF XMLNode *iter, *next;
	size_t i;
#ifndef NDEBUG
	ASSERT(self->xn_sib_prev == NULL);
	ASSERT(self->xn_sib_next == NULL);
	ASSERT(!LIST_ISBOUND(self, xn_changed));
#endif /* !NDEBUG */
	iter = LIST_FIRST(&self->xn_changes);
	while (iter) {
		next = LIST_NEXT(iter, xn_changed);
		XMLNode_Decref(iter);
		iter = next;
	}
	if (self->xn_attr.nas_list) {
		for (i = 0; i <= self->xn_attr.nas_mask; ++i) {
			if (!ITER_ISOK(self->xn_attr.nas_list[i].na_name_str))
				continue;
			Dee_Decref(self->xn_attr.nas_list[i].na_owner);
		}
		Dee_Free(self->xn_attr.nas_list);
	}
	if (self->xn_children.ncs_list) {
		for (i = 0; i <= self->xn_children.ncs_mask; ++i) {
			if (!ITER_ISOK(self->xn_children.ncs_list[i].nc_child))
				continue;
			XMLNode_Decref(self->xn_children.ncs_list[i].nc_child);
		}
		Dee_Free(self->xn_children.ncs_list);
	}
	Dee_Decref(self->xn_kind_obj);
	Dee_Decref(self->xn_attr_obj);
	Dee_Decref(self->xn_text_obj);
	DeeSlab_FREE(self);
}


#define XMLNode_ShouldRehashChildren(self) \
	((self)->xn_children.ncs_used >= ((self)->xn_children.ncs_mask * 2) / 3)
#define XMLNode_MustRehashChildren(self) \
	((self)->xn_children.ncs_used >= (self)->xn_children.ncs_mask)

#define XML_NODE_DEFAULT_CHILD_MASK 3

LOCAL NONNULL((1)) bool DCALL
XMLNode_TryRehashChildren(XMLNode *__restrict self) {
	size_t new_mask, old_mask, i;
	struct xml_node_child *new_list, *old_list;
	if (!self->xn_children.ncs_list) {
		new_list = (struct xml_node_child *)Dee_TryCalloc(4 * sizeof(struct xml_node_child));
		if unlikely(!new_list)
			goto err;
		self->xn_children.ncs_mask = XML_NODE_DEFAULT_CHILD_MASK;
		ASSERT(self->xn_children.ncs_size == 0);
		ASSERT(self->xn_children.ncs_used == 0);
		self->xn_children.ncs_list = new_list;
		goto done;
	}
	old_mask = self->xn_children.ncs_mask;
	new_mask = (self->xn_children.ncs_mask << 1) | 1;
	new_list = (struct xml_node_child *)Dee_TryCalloc((new_mask + 1) *
	                                                  sizeof(struct xml_node_child));
	if unlikely(!new_list)
		goto err;
	old_list = self->xn_children.ncs_list;
	/* Re-hash the old child node vector. */
	for (i = 0; i <= old_mask; ++i) {
		dhash_t j, perturb;
		DREF XMLNode *node = old_list[i].nc_child;
		if (!ITER_ISOK(node))
			continue;
		j = perturb = node->xn_kind_hash & new_mask;
		for (;; XMLNodeChildren_NEXT(j, perturb)) {
			dhash_t index = j & new_mask;
			if (new_list[index].nc_child)
				continue;
			new_list[index].nc_child = node; /* Inherit reference */
			break;
		}
	}
	self->xn_children.ncs_size = self->xn_children.ncs_used;
	self->xn_children.ncs_mask = new_mask;
	self->xn_children.ncs_list = new_list;
	Dee_Free(old_list);
done:
	return true;
err:
	return false;
}



/* Return a reference to the previous/next sibling node of `self'
 * @param: self:       The node who's sibling should be queried.
 * @param: parent:     The parent node for `self'.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The requested node does not exist. */
INTERN WUNUSED NONNULL((1, 2)) DREF XMLNode *DCALL
XMLNode_GetPrev(XMLNode *__restrict self,
                XMLNode *__restrict parent) {
	DREF XMLNode *result;
	rwlock_read(&parent->xn_lock);
	result = self->xn_sib_prev;
	if (result) {
		XMLNode_Incref(result);
		rwlock_endread(&parent->xn_lock);
		return result;
	}
	rwlock_endread(&parent->xn_lock);
	return (DREF XMLNode *)ITER_DONE;
}

INTERN WUNUSED NONNULL((1, 2)) DREF XMLNode *DCALL
XMLNode_GetNext(XMLNode *__restrict self,
                XMLNode *__restrict parent) {
	DREF XMLNode *result;
	char *iter, *end, *temp;
	bool ends_with_slash;
again:
	rwlock_read(&parent->xn_lock);
	result = self
	         ? self->xn_sib_next
	         : parent->xn_children.ncs_head;
	if (result) {
		XMLNode_Incref(result);
		rwlock_endread(&parent->xn_lock);
		return result;
	}
	ASSERT(self == parent->xn_children.ncs_tail);
	iter = (char *)parent->xn_text_next;
	end  = (char *)parent->xn_text_end;
	ASSERT(iter <= end);
	if (iter >= end) {
		rwlock_endread(&parent->xn_lock);
		return (DREF XMLNode *)ITER_DONE;
	}
	if (!rwlock_upgrade(&parent->xn_lock)) {
		COMPILER_READ_BARRIER();
		iter = (char *)parent->xn_text_next;
		end  = (char *)parent->xn_text_end;
		ASSERT(iter <= end);
		if (iter >= end) {
			rwlock_endwrite(&parent->xn_lock);
			return (DREF XMLNode *)ITER_DONE;
		}
	}
	/* Search for the next sibling node. */
find_block_start:
	iter = (char *)memchr(iter, '<', (size_t)(end - iter));
	if (!iter) {
	set_end_of_text:
		/* No more child nodes. */
		parent->xn_text_next = end;
		rwlock_endwrite(&parent->xn_lock);
		return (DREF XMLNode *)ITER_DONE;
	}
	++iter;
	if (iter + 3 <= end && iter[0] == '!' &&
	    iter[1] == '-' && iter[2] == '-') {
		/* XML Comment */
		iter = (char *)memmem(iter, (size_t)(end - iter), "-->", 3 * sizeof(char));
		if unlikely(!iter)
			goto set_end_of_text;
		iter += 3;
		goto find_block_start;
	}
	/* Rehash the parent's child hash-vector, if necessary */
	if (XMLNode_ShouldRehashChildren(parent) &&
	    !XMLNode_TryRehashChildren(parent) &&
	    XMLNode_MustRehashChildren(parent)) {
		if (Dee_CollectMemory(1))
			goto again;
		goto err;
	}
	result = XMLNode_TryAlloc();
	if unlikely(!result) {
		parent->xn_text_next = iter;
		rwlock_endwrite(&parent->xn_lock);
		if (Dee_CollectMemory(sizeof(XMLNode)))
			goto again;
		goto err;
	}
	result->xn_refcnt = 1;
	rwlock_init(&result->xn_lock);
	LIST_INIT(&result->xn_changes);
	LIST_ENTRY_UNBOUND_INIT(&result->xn_changed);
	if ((result->xn_sib_prev = self) != NULL) {
		self->xn_sib_next = result;
	} else {
		parent->xn_children.ncs_head = result;
	}
	parent->xn_children.ncs_tail = result;
	result->xn_sib_next          = NULL;
#define SKIP_SPACE()                                     \
	while (iter < end) {                                 \
		uint32_t _ch;                                    \
		temp = iter;                                     \
		_ch  = utf8_readchar((char const **)&temp, end); \
		if (!DeeUni_IsSpace(_ch))                        \
			break;                                       \
		iter = temp;                                     \
	}

	SKIP_SPACE();
	result->xn_kind_str = iter; /* Start of the tag kind name. */
	while (iter < end) {
		uint32_t ch;
		temp = iter;
		ch   = utf8_readchar((char const **)&temp, end);
		if (!DeeUni_IsSymCont(ch) && ch != ':')
			break;
		iter = temp;
	}
	result->xn_kind_end = iter;
	SKIP_SPACE();
	result->xn_attr_next =
	result->xn_attr_str =
	result->xn_attr_end = iter;
	ends_with_slash     = false;
	for (;;) {
		char ch;
		if unlikely(iter >= end) {
eof_in_attr:
			result->xn_attr_end = iter;
			result->xn_text_str = iter;
			result->xn_text_end = iter;
			iter                = end;
			goto done;
		}
		ch = *iter;
		if (ch > 0x7f) {
			uint32_t ch32;
			ch32 = utf8_readchar((char const **)&iter, end);
			if (!DeeUni_IsSpace(ch32))
				result->xn_attr_end = iter;
		} else {
			if (ch == '>')
				break;
			if (ch == '\"') {
				++iter;
				iter = (char *)memchr(iter, '\"', (size_t)(end - iter));
				if unlikely(!iter)
					goto eof_in_attr;
				++iter;
				continue;
			}
			if (ch == '/') {
				ends_with_slash = true;
			} else {
				if (!DeeUni_IsSpace(ch))
					result->xn_attr_end = iter,
					ends_with_slash     = false;
			}
			++iter;
		}
	}
	result->xn_text_str = iter;
	if (!ends_with_slash) {
		/* Search for the end of node text */
		unsigned int recursion = 1;
		while (iter < end) {
			char marker, ch = *iter++;
			if (ch != '<')
				continue;
			if (iter + 3 <= end && iter[0] == '!' &&
			    iter[1] == '-' && iter[2] == '-') {
				/* XML Comment */
				iter = (char *)memmem(iter, (size_t)(end - iter), "-->", 3 * sizeof(char));
				if unlikely(!iter)
					goto set_end_of_text_scan;
				iter += 3;
				continue;
			}
			SKIP_SPACE();
			if unlikely(iter >= end)
				break;
			marker = *iter++;
			while (iter < end) {
				ch = *iter++;
				if (ch == '>')
					break;
				if (ch == '\"') {
					iter = (char *)memchr(iter, '\"', (size_t)(end - iter));
					if unlikely(!iter) {
set_end_of_text_scan:
						iter = end;
						goto done_text;
					}
					++iter;
					continue;
				}
			}
			if (marker == '/') {
				--recursion;
				if (!recursion)
					break;
			} else {
				++recursion;
			}
		}
	}
done_text:
	result->xn_text_end = iter;
done:
	/* Fill in remaining fields. */
	result->xn_text_next = result->xn_text_str;
	result->xn_kind_obj =
	result->xn_attr_obj =
	result->xn_text_obj  = parent->xn_text_obj;
	parent->xn_text_next = iter;
	Dee_Incref_n(parent->xn_text_obj, 3);
	result->xn_kind_hash = Dee_HashUtf8(result->xn_kind_str,
	                                    (size_t)(result->xn_kind_end -
	                                             result->xn_kind_str));

	{
		/* Add the resulting node to the parent's hash-vector. */
		dhash_t i, perturb;
		i = perturb = result->xn_kind_hash & parent->xn_children.ncs_mask;
		for (;; XMLNodeChildren_NEXT(i, perturb)) {
			size_t index = i & parent->xn_children.ncs_mask;
			if (ITER_ISOK(parent->xn_children.ncs_list[index].nc_child))
				continue;
			parent->xn_children.ncs_list[index].nc_child = result;
			break;
		}
	}
	XMLNode_Incref(result); /* The reference that we're returning. */
	rwlock_endwrite(&parent->xn_lock);
	return (DREF XMLNode *)ITER_DONE;
err:
	return NULL;
}


/* Initialize an XML Node from a given string.
 * NOTE: If the given `text' contains multiple nodes, a pointer to the start
 *       of a potential other node is stored under `self->xn_text_end' upon
 *       success.
 * @param: self:          The node to initialize.
 * @param: data_owner:    The object which owns the given input text.
 * @param: text,text_end: The text that should be parsed for an XML Node.
 * @return: 0:            Successfully parsed a node.
 * @return: 1:            No node found within the given input text.
 * @return: -1:           An error occurred. */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
XMLNode_InitFromString(XMLNode *__restrict self,
                       DeeObject *__restrict data_owner,
                       /*utf-8*/ char const *__restrict text,
                       /*utf-8*/ char const *__restrict text_end);





PRIVATE WUNUSED NONNULL((1)) int DCALL
node_init(XMLNodeObject *__restrict self,
          size_t argc, DeeObject *const *argv) {
	(void)self;
	(void)argc;
	(void)argv;
	/* TODO */
	return 0;
}

PRIVATE NONNULL((1)) void DCALL
node_fini(XMLNodeObject *__restrict self) {
	XMLNode_Decref(self->xno_node);
	Dee_XDecref(self->xno_parent);
}

PRIVATE NONNULL((1, 2)) void DCALL
node_visit(XMLNodeObject *__restrict self,
           dvisit_t proc, void *arg) {
	Dee_XVisit(self->xno_parent);
}


PRIVATE struct type_getset node_getsets[] = {
	{ NULL }
};

PRIVATE struct type_method node_methods[] = {
	{ NULL }
};

INTERN DeeTypeObject XMLNodeObject_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "Node",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ (void *)&node_init,
				TYPE_FIXED_ALLOCATOR(XMLNodeObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&node_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&node_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ node_methods,
	/* .tp_getsets       = */ node_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


DECL_END


#endif /* !GUARD_DEX_XML_XML_C */
