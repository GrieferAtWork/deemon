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
#ifndef GUARD_DEX_XML_LIBXML_H
#define GUARD_DEX_XML_LIBXML_H 1

#include <deemon/api.h>
#include <deemon/dex.h>
#include <deemon/object.h>
#include <deemon/util/rwlock.h>

#include <hybrid/atomic.h>
#include <hybrid/sequence/list.h>

DECL_BEGIN

typedef struct xml_node XMLNode;
typedef struct xml_node_object XMLNodeObject;
typedef struct xml_node_attributes_proxy XMLNodeAttributesProxy;

struct xml_node_attr {
	/*utf-8*/ char const *na_name_str;    /* [0..1] Starting address of the name (`NULL' for sentinel entries / `ITER_DONE' for deleted ones) */
	size_t                na_name_size;   /* Name size (in bytes). */
	dhash_t               na_name_hash;   /* Name hash. */
	/*utf-8*/ char const *na_value_start; /* [1..na_value_size] Attribute value starting pointer. */
	size_t                na_value_size;  /* Value size (in bytes). */
	DREF DeeObject       *na_owner;       /* [1..1] Some object holding references to the owner
	                                       * objects for `na_name_str' and `na_value_start'. */
};

struct xml_node_attributes {
	size_t                nas_mask; /* Allocated hash-mask of node attributes. */
	size_t                nas_size; /* Number of entires with `na_name_str != NULL'. */
	size_t                nas_used; /* Number of entires with `na_name_str != NULL && na_name_str != (char *)ITER_DONE'. */
	struct xml_node_attr *nas_list; /* [0..nas_mask + 1][owned] Hash-vector of XML Node attributes. */
#define XMLNodeAttributes_NEXT(i, perturb) ((i) = (((i) << 2) + (i) + (perturb) + 1), (perturb) >>= 5)
};

struct xml_node_child {
	DREF XMLNode          *nc_child; /* [0..1] The child node (`NULL' for sentinel entries / `ITER_DONE' for deleted ones)
	                                  * NOTE: For this purpose, the node's kind is used as accessor & hash */
};

struct xml_node_children {
	size_t                 ncs_mask; /* Allocated hash-mask of node children. */
	size_t                 ncs_size; /* Number of entires with `nc_child != NULL'. */
	size_t                 ncs_used; /* Number of entires with `nc_child != NULL && nc_child != ITER_DONE'. */
	struct xml_node_child *ncs_list; /* [0..nas_mask + 1][owned] Hash-vector of XML Node attributes. */
	XMLNode               *ncs_head; /* [0..1] The first child node. */
	XMLNode               *ncs_tail; /* [0..1] The last child node. */
#define XMLNodeChildren_NEXT(i, perturb) ((i) = (((i) << 2) + (i) + (perturb) + 1), (perturb) >>= 5)
};



/* To prevent reference recursion, as well as the need to make
 * XML Nodes be GC objects, references are handled as follows:
 *
 *                 REF:children              REF:children
 *                 |                         |
 *      [XMLNode] --------------> [XMLNode] --------------> [XMLNode]
 *          ^                         ^                         ^
 *          |                         |                         |
 *          |                         |                         |
 *          | REF:xno_node            | REF:xno_node            | REF:xno_node
 *          |                         |                         |
 *          |                         |                         |
 * [XMLNodeObject root] <-- [XMLNodeObject child] <-- [XMLNodeObject child]
 *                       |                         |
 *                       REF:xno_parent            REF:xno_parent
 *
 * In this model, the internal XML Nodes are created only once, but don't
 * know their parent nodes. - Access is made via XMLNodeObject structures,
 * which in turn do know their parent.
 *   - As can be seen, this does not cause a reference loop (just follow the arrows)
 *   - With this, accessing an XML tree in user-code means that child nodes can be
 *     loaded lazily, without having to deal with nodes being unloaded due to not
 *     being referenced by user-code anymore, allowing for the assumption that nodes
 *     are only created once (following [lock(WRITE_ONCE)] conventions)
 *   - Re-enumeration of child nodes will re-use the same internal XMLNode objects
 *   - Access to parent nodes is done via the XMLNodeObject abstraction layer
 *   - Access to sibling nodes is also done via the XMLNodeObject abstraction layer
 *   - Enumeration of child nodes creates `XMLNodeObject' wrappers for each enumerated
 *     node, while allowing the internal XPath implementation to function properly
 *     without having to create `XMLNodeObject' wrappers for each Node that doesn't
 *     turn out to actually have to be enumerated.
 */

LIST_HEAD(xml_node_list, xml_node);

struct xml_node {
	DWEAK Dee_refcnt_t         xn_refcnt;    /* Reference counter. */
#ifndef CONFIG_NO_THREADS
	rwlock_t                   xn_lock;      /* Lock for modifications made to this node. */
#endif /* !CONFIG_NO_THREADS */
	DREF struct xml_node_list  xn_changes;   /* [0..1][lock(xn_lock)] Chain of modified XML child Nodes. */
	DREF LIST_ENTRY(xml_node)  xn_changed;   /* [lock(:xno_parent->xno_node->xn_lock)]
	                                                                   * [CHAIN(:xno_parent->xno_node->xn_changes)]
	                                          * [1..1] Chain of changed XML Nodes. */
	XMLNode                   *xn_sib_prev;  /* [0..1][lock(:xno_parent->xno_node->xn_lock)] Previous sibling node. */
	XMLNode                   *xn_sib_next;  /* [0..1][lock(:xno_parent->xno_node->xn_lock)] Next sibling node. */
	DREF DeeObject            *xn_kind_obj;  /* [1..1][lock(xn_lock)] Custom data owner for a modified `xn_kind_str' */
	/*utf-8*/ char const      *xn_kind_str;  /* [1..1][lock(xn_lock)] XML kind name start pointer.
	                                          * >> <foo bar="baz">content</foo>
	                                          *     ^
	                                          * NOTE: When `xn_parent' is `NULL', this field cannot be accessed, or set. */
	/*utf-8*/ char const      *xn_kind_end;  /* [1..1][lock(xn_lock)] XML kind name end pointer.
	                                          * >> <foo bar="baz">content</foo>
	                                          *        ^ */
	dhash_t                    xn_kind_hash; /* [lock(xn_lock)] Hash for `xn_kind_str'. */
	struct xml_node_attributes xn_attr;    /* [lock(xn_lock)] Set of cached node attributes. */
	DREF DeeObject            *xn_attr_obj;  /* [1..1][lock(xn_lock)] Custom data owner for a modified `xn_attr_str' */
	/*utf-8*/ char const      *xn_attr_str;  /* [1..1][lock(xn_lock)] XML attribute start pointer.
	                                          * >> <foo bar="baz">content</foo>
	                                          *         ^ */
	/*utf-8*/ char const      *xn_attr_end;  /* [1..1][lock(xn_lock)] XML attribute end pointer.
	                                          * >> <foo bar="baz">content</foo>
	                                          *                  ^ */
	/*utf-8*/ char const      *xn_attr_next; /* [1..1][lock(xn_lock)] Pointer to the next attribute that hasn't been loaded.
	                                          * This pointer is located between `xn_attr_str..xn_attr_end'
	                                          * NOTE: This pointer can only be incremented, and not past `xn_attr_end'.
	                                          *       Once equal to `xn_attr_end', all attributes have been loaded. */
	struct xml_node_children   xn_children;  /* [lock(xn_lock)] Set of loaded XML child nodes. */
	DREF DeeObject            *xn_text_obj;  /* [1..1][lock(xn_lock)] Custom data owner for a modified `xn_text_str' */
	/*utf-8*/ char const      *xn_text_str;  /* [1..1][lock(xn_lock)] XML text start pointer.
	                                          * >> <foo bar="baz">content</foo>
	                                          *                   ^ */
	/*utf-8*/ char const      *xn_text_end;  /* [1..1][lock(xn_lock)] XML text end pointer.
	                                          * >> <foo bar="baz">content</foo>
	                                          *                          ^ */
	/*utf-8*/ char const      *xn_text_next; /* [1..1][lock(xn_lock)] Pointer before the next child node that hasn't been loaded.
	                                          * NOTE: This pointer can only be incremented, and not past `xn_attr_end'.
	                                          *       Once equal to `xn_attr_end', all attributes have been loaded. */
};

#define XMLNode_Incref(x) (ATOMIC_FETCHINC((x)->xn_refcnt))
#define XMLNode_Decref(x) (ATOMIC_DECFETCH((x)->xn_refcnt))

INTDEF NONNULL((1)) void DCALL XMLNode_Destroy(XMLNode *__restrict self);
#define XMLNode_Alloc()    DeeSlab_MALLOC(XMLNode)
#define XMLNode_TryAlloc() DeeSlab_TRYMALLOC(XMLNode)


/* Initialize an XML Node from a given string.
 * NOTE: If the given `text' contains multiple nodes, a pointer to the start
 *       of a potential other node is stored under `self->xn_text_end' upon
 *       success.
 * @param: self:           The node to initialize.
 * @param: data_owner:     The object which owns the given input text.
 * @param: text, text_end: The text that should be parsed for an XML Node.
 * @return: 0:             Successfully parsed a node.
 * @return: 1:             No node found within the given input text.
 * @return: -1:            An error occurred. */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
XMLNode_InitFromString(XMLNode *__restrict self,
                       DeeObject *__restrict data_owner,
                       /*utf-8*/ char const *__restrict text,
                       /*utf-8*/ char const *__restrict text_end);

/* Return a reference to the previous/next sibling node of `self'
 * @param: self:       The node who's sibling should be queried.
 * @param: parent:     The parent node for `self'.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The requested node does not exist. */
INTDEF WUNUSED NONNULL((1, 2)) DREF XMLNode *DCALL
XMLNode_GetPrev(XMLNode *__restrict self, XMLNode *__restrict parent);

INTDEF WUNUSED NONNULL((2)) DREF XMLNode *DCALL
XMLNode_GetNext(XMLNode *self, XMLNode *__restrict parent);

#define XMLNode_GetFirst(self) XMLNode_GetNext(NULL, self)



typedef struct xml_node_object XMLNodeObject;
struct xml_node_object {
	OBJECT_HEAD
	DREF XMLNodeObject *xno_parent; /* [0..1][const] Parent node. */
	DREF XMLNode       *xno_node;   /* [1..1][const] The node being referenced. */
};

INTDEF DeeTypeObject XMLNodeObject_Type;



DECL_END

#endif /* !GUARD_DEX_XML_LIBXML_H */
