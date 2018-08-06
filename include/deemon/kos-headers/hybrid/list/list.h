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
#ifndef __GUARD_HYBRID_LIST_LIST_H
#define __GUARD_HYBRID_LIST_LIST_H 1

#include <hybrid/compiler.h>
#include <stddef.h>

#ifdef __CC__
DECL_BEGIN

/* >> struct entry {
 * >>     int                     e_val;
 * >>     LIST_NODE(struct entry) e_link;
 * >> };
 * >> 
 * >> LIST_HEAD(struct entry) my_list;
 * >> 
 * >> #define MY_FOREACH(elem)             LIST_FOREACH(my_list,elem,e_link)
 * >> #define MY_INSERT(entry)             LIST_INSERT(my_list,entry)
 * >> #define MY_INSERT_AFTER(link,entry)  LIST_INSERT_AFTER(link,entry,e_link)
 * >> #define MY_INSERT_BEFORE(link,entry) LIST_INSERT_BEFORE(link,entry,e_link)
 */

#define SLIST_NODE(T) struct { T *le_next; }
#define LIST_NODE(T)  struct { T *le_next,**le_pself; }
#define SLIST_HEAD(T)          T *
#define LIST_HEAD(T)           T *

#define LIST_FOREACH(elem,list,key)                    for ((elem) = (list); (elem); (elem) = (elem)->key.le_next)
#define SLIST_FOREACH(elem,list,key)                   for ((elem) = (list); (elem); (elem) = (elem)->key.le_next)
#define LIST_FOREACH_EX(elem,list,key,nullval)         for ((elem) = (list); (elem) != (nullval); (elem) = (elem)->key.le_next)
#define SLIST_FOREACH_EX(elem,list,key,nullval)        for ((elem) = (list); (elem) != (nullval); (elem) = (elem)->key.le_next)
#define LIST_FOREACH_P(elem,list,getpath)              for ((elem) = (list); (elem); (elem) = getpath(elem).le_next)
#define SLIST_FOREACH_P(elem,list,getpath)             for ((elem) = (list); (elem); (elem) = getpath(elem).le_next)
#define LIST_FOREACH_P_EX(elem,list,getpath,nullval)   for ((elem) = (list); (elem) != (nullval); (elem) = getpath(elem).le_next)
#define SLIST_FOREACH_P_EX(elem,list,getpath,nullval)  for ((elem) = (list); (elem) != (nullval); (elem) = getpath(elem).le_next)


/* Check/set an unbound list entry as such.
 * NOTE: 'LIST_MKUNBOUND()' should only be called on a element after
 *        said element has been removed from its associated list. */
#define LIST_ISUNBOUND(elem,key)      (!(elem)->key.le_pself)
#define LIST_MKUNBOUND(elem,key) (void)((elem)->key.le_pself = NULL)
#define LIST_UNBIND(elem,key) \
 (!LIST_ISUNBOUND(elem,key) ? \
  (LIST_REMOVE(elem,key), \
   LIST_MKUNBOUND(elem,key)) : (void)0)
#define LIST_ISUNBOUND_EX(elem,key,nullval)       ((elem)->key.le_pself == (nullval))
#define LIST_MKUNBOUND_EX(elem,key,nullval) (void)((elem)->key.le_pself = (nullval))
#define LIST_UNBIND_EX(elem,key,nullval) \
 (!LIST_ISUNBOUND_EX(elem,key,nullval) ? \
  (LIST_REMOVE_EX(elem,key,nullval), \
   LIST_MKUNBOUND_EX(elem,key,nullval)) : (void)0)
#define LIST_ISUNBOUND_P(elem,getpath)      (!getpath(elem).le_pself)
#define LIST_MKUNBOUND_P(elem,getpath) (void)(getpath(elem).le_pself = NULL)
#define LIST_UNBIND_P(elem,getpath) \
 (!LIST_ISUNBOUND_P(elem,getpath) ? \
  (LIST_REMOVE_P(elem,getpath), \
   LIST_MKUNBOUND_P(elem,getpath)) : (void)0)
#define LIST_ISUNBOUND_P_EX(elem,getpath,nullval)       (getpath(elem).le_pself == (nullval))
#define LIST_MKUNBOUND_P_EX(elem,getpath,nullval) (void)(getpath(elem).le_pself = (nullval))
#define LIST_UNBIND_P_EX(elem,getpath,nullval) \
 (!LIST_ISUNBOUND_P_EX(elem,getpath,nullval) ? \
  (LIST_REMOVE_P_EX(elem,getpath,nullval), \
   LIST_MKUNBOUND_P_EX(elem,getpath,nullval)) : (void)0)



/* Unlink all elements from `list' and call 'cleanup(elem)' for each. */
#define LIST_CLEAR(list,key,cleanup) \
do{ __typeof__(*(list)) *_iter = (list),*_next;\
    (list) = NULL;\
    while (_iter) {\
     _next = _iter->key.le_next;\
     cleanup(_iter);\
     iter = _next;\
    }\
}__WHILE0
#define LIST_CLEAR_EX(list,key,cleanup,nullval) \
do{ __typeof__(*(list)) *_iter = (list),*_next;\
    (list) = (nullval);\
    while ((_iter) != (nullval)) {\
     _next = _iter->key.le_next;\
     cleanup(_iter);\
     iter = _next;\
    }\
}__WHILE0
#define LIST_CLEAR_P(list,getpath,cleanup) \
do{ __typeof__(*(list)) *_iter = (list),*_next;\
    (list) = NULL;\
    while (_iter) {\
     _next = getpath(_iter).le_next;\
     cleanup(_iter);\
     iter = _next;\
    }\
}__WHILE0
#define LIST_CLEAR_P_EX(list,getpath,cleanup,nullval) \
do{ __typeof__(*(list)) *_iter = (list),*_next;\
    (list) = (nullval);\
    while ((_iter) != (nullval)) {\
     _next = getpath(_iter).le_next;\
     cleanup(_iter);\
     iter = _next;\
    }\
}__WHILE0

#define SLIST_INSERT(list,elem,key) \
      ((elem)->key.le_next = (list),(list) = (elem))
#define SLIST_INSERT_EX(list,elem,key,nullval) \
      ((elem)->key.le_next = (list),(list) = (elem))
#define SLIST_INSERT_P(list,elem,getpath) \
      (getpath(elem).le_next = (list),(list) = (elem))
#define SLIST_INSERT_P_EX(list,elem,getpath,nullval) \
      (getpath(elem).le_next = (list),(list) = (elem))

/* Remove 'elem' from a list named `key'. */
#define LIST_REMOVE(elem,key) \
 ((*(elem)->key.le_pself = (elem)->key.le_next) != NULL \
  ? (void)((elem)->key.le_next->key.le_pself = (elem)->key.le_pself) \
  : (void)0)
#define LIST_REMOVE_EX(elem,key,nullval) \
 ((*(elem)->key.le_pself = (elem)->key.le_next) != (nullval) \
  ? (void)((elem)->key.le_next->key.le_pself = (elem)->key.le_pself) \
  : (void)0)
#define LIST_REMOVE_P(elem,getpath) \
 ((*getpath(elem).le_pself = getpath(elem).le_next) != NULL \
  ? (void)(getpath(getpath(elem).le_next).le_pself = getpath(elem).le_pself) \
  : (void)0)
#define LIST_REMOVE_P_EX(elem,getpath,nullval) \
 ((*getpath(elem).le_pself = getpath(elem).le_next) != (nullval) \
  ? (void)(getpath(getpath(elem).le_next).le_pself = getpath(elem).le_pself) \
  : (void)0)

/* Insert a given element 'elem' at the front of a `list' described by `key'. */
#define LIST_INSERT(list,elem,key) \
 ((((elem)->key.le_next = (list)) != NULL \
  ? (void)((list)->key.le_pself = &(elem)->key.le_next) \
  : (void)0), \
 (void)((elem)->key.le_pself = &(list),(list) = (elem)))
#define LIST_INSERT_EX(list,elem,key,nullval) \
 ((((elem)->key.le_next = (list)) != (nullval) \
  ? (void)((list)->key.le_pself = &(elem)->key.le_next) \
  : (void)0), \
 (void)((elem)->key.le_pself = &(list),(list) = (elem)))
#define LIST_INSERT_P(list,elem,getpath) \
 (((getpath(elem).le_next = (list)) != NULL \
  ? (void)(getpath(list).le_pself = &getpath(elem).le_next) \
  : (void)0), \
 (void)(getpath(elem).le_pself = &(list),(list) = (elem)))
#define LIST_INSERT_P_EX(list,elem,getpath,nullval) \
 (((getpath(elem).le_next = (list)) != (nullval) \
  ? (void)(getpath(list).le_pself = &getpath(elem).le_next) \
  : (void)0), \
 (void)(getpath(elem).le_pself = &(list),(list) = (elem)))


/* Insert a given element 'elem' after `link' in a list described by `key'. */
#define LIST_INSERT_AFTER(link,elem,key) \
 ((((elem)->key.le_next = (link)->key.le_next) != NULL \
  ? (void)((elem)->key.le_next->key.le_pself = &(elem)->key.le_next) \
  : (void)0), \
 (void)((elem)->key.le_pself = &(link)->key.le_next, \
        (link)->key.le_next  = (elem)))
#define LIST_INSERT_AFTER_EX(link,elem,key,nullval) \
 ((((elem)->key.le_next = (link)->key.le_next) != (nullval) \
  ? (void)((elem)->key.le_next->key.le_pself = &(elem)->key.le_next) \
  : (void)0), \
 (void)((elem)->key.le_pself = &(link)->key.le_next, \
        (link)->key.le_next  = (elem)))
#define LIST_INSERT_AFTER_P(link,elem,getpath) \
 (((getpath(elem).le_next = getpath(link).le_next) != NULL \
  ? (void)(getpath(getpath(elem).le_next).le_pself = &getpath(elem).le_next) \
  : (void)0), \
 (void)(getpath(elem).le_pself = &getpath(link).le_next, \
        getpath(link).le_next  = (elem)))
#define LIST_INSERT_AFTER_P_EX(link,elem,getpath,nullval) \
 (((getpath(elem).le_next = getpath(link).le_next) != (nullval) \
  ? (void)(getpath(getpath(elem).le_next).le_pself = &getpath(elem).le_next) \
  : (void)0), \
 (void)(getpath(elem).le_pself = &getpath(link).le_next, \
        getpath(link).le_next  = (elem)))

/* Insert a given element 'elem' before `link' in a list described by `key'. */
#define LIST_INSERT_BEFORE(link,elem,key) \
 (void)((elem)->key.le_pself = (link)->key.le_pself, \
       *(elem)->key.le_pself = (elem), \
        (elem)->key.le_next  = (link), \
        (link)->key.le_pself = &(elem)->key.le_next)
#define LIST_INSERT_BEFORE_EX(link,elem,key,nullval) \
        LIST_INSERT_BEFORE(link,elem,key)
#define LIST_INSERT_BEFORE_P(link,elem,getpath) \
 (void)(getpath(elem).le_pself = getpath(link).le_pself, \
       *getpath(elem).le_pself = (elem), \
        getpath(elem).le_next  = (link), \
        getpath(link).le_pself = &getpath(elem).le_next)
#define LIST_INSERT_BEFORE_P_EX(link,elem,getpath,nullval) \
        LIST_INSERT_BEFORE_P(link,elem,getpath)

#define LIST_INSERT_REPLACE(link,elem,key) \
 (void)(*((elem)->key.le_pself = (link)->key.le_pself) = (elem), \
         ((elem)->key.le_next  = (link)->key.le_next) != NULL \
      ? (void)((elem)->key.le_next->key.le_pself = &(elem)->key.le_next) \
      : (void)0)
#define LIST_INSERT_REPLACE_EX(link,elem,key,nullval) \
 (void)(*((elem)->key.le_pself = (link)->key.le_pself) = (elem), \
         ((elem)->key.le_next  = (link)->key.le_next) != (nullval) \
      ? (void)((elem)->key.le_next->key.le_pself = &(elem)->key.le_next) \
      : (void)0)
#define LIST_INSERT_REPLACE_P(link,elem,getpath) \
 (void)(*(getpath(elem).le_pself = getpath(link).le_pself) = (elem), \
         (getpath(elem).le_next  = getpath(link).le_next) != NULL \
      ? (void)(getpath(getpath(elem).le_next).le_pself = &getpath(elem).le_next) \
      : (void)0)
#define LIST_INSERT_REPLACE_P_EX(link,elem,getpath,nullval) \
 (void)(*(getpath(elem).le_pself = getpath(link).le_pself) = (elem), \
         (getpath(elem).le_next  = getpath(link).le_next) != (nullval) \
      ? (void)(getpath(getpath(elem).le_next).le_pself = &getpath(elem).le_next) \
      : (void)0)


#define ARRAY_FOREACH(elem,arr) \
   for ((elem) = (arr); (elem) != COMPILER_ENDOF(arr); ++(elem))


DECL_END
#endif /* __CC__ */

#endif /* !__GUARD_HYBRID_LIST_LIST_H */
