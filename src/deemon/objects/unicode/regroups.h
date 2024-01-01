/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_REGROUPS_H
#define GUARD_DEEMON_OBJECTS_UNICODE_REGROUPS_H 1

#include <deemon/alloc.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/regex.h>

/* Proxy sequence objects for `struct DeeRegexMatch'-arrays */

DECL_BEGIN

typedef struct {
	OBJECT_HEAD
	size_t                                        rg_ngroups; /* # of groups (must be >= 1) */
	COMPILER_FLEXIBLE_ARRAY(struct DeeRegexMatch, rg_groups); /* Array of groups */
} ReGroups;

#define DeeRegexMatch_AsRangeObject(self) \
	((self)->rm_so == (size_t)-1          \
	 ? (Dee_Incref(Dee_None), Dee_None)   \
	 : DeeTuple_Newf(PCKuSIZ PCKuSIZ,     \
	                 (self)->rm_so,       \
	                 (self)->rm_eo))

typedef struct {
	OBJECT_HEAD
	DREF ReGroups *rgi_groups; /* [1..1][const] Linked groups object. */
	size_t         rgi_index;  /* [lock(ATOMIC)] Index of next not-enumerated group. */
} ReGroupsIterator;

#define ReGroupsIterator_GetIndex(x) atomic_read(&(x)->rgi_index)

INTDEF DeeTypeObject ReGroups_Type;
INTDEF DeeTypeObject ReGroupsIterator_Type;

#define ReGroups_Malloc(ngroups)                                  \
	((ReGroups *)DeeObject_Malloc(offsetof(ReGroups, rg_groups) + \
	                              (ngroups) * sizeof(struct DeeRegexMatch)))
#define ReGroups_Free(self) DeeObject_Free(self)
#define ReGroups_Init(self, ngroups)             \
	(void)(DeeObject_Init(self, &ReGroups_Type), \
	       (self)->rg_ngroups = (ngroups))



typedef struct {
	OBJECT_HEAD
	size_t                                        rss_ngroups; /* # of groups (may be 0) */
	DREF DeeObject                               *rss_baseown; /* [1..1][const] Owner of `rss_baseptr' (string or bytes) */
	__BYTE_TYPE__ const                          *rss_baseptr; /* [1..1][const] Base-pointer. */
	COMPILER_FLEXIBLE_ARRAY(struct DeeRegexMatch, rss_groups); /* Array of groups */
} ReSubStrings;

#define DeeRegexMatch_AsSubString(self, baseobj, baseptr)           \
	((self)->rm_so == (size_t)-1                                    \
	 ? (Dee_Incref(Dee_None), Dee_None)                             \
	 : DeeString_NewUtf8((char const *)((baseptr) + (self)->rm_so), \
	                     (self)->rm_eo - (self)->rm_so,             \
	                     Dee_STRING_ERROR_FSTRICT))
#define DeeRegexMatch_AsSubBytes(self, baseobj, baseptr)        \
	((self)->rm_so == (size_t)-1                                \
	 ? (Dee_Incref(Dee_None), Dee_None)                         \
	 : DeeBytes_NewSubView(baseobj,                             \
	                       (void *)((baseptr) + (self)->rm_so), \
	                       (self)->rm_eo - (self)->rm_so))

typedef struct {
	OBJECT_HEAD
	DREF ReSubStrings *rssi_strings; /* [1..1][const] Linked groups object. */
	size_t             rssi_index;   /* [lock(ATOMIC)] Index of next not-enumerated group. */
} ReSubStringsIterator;

#define ReSubStringsIterator_GetIndex(x) atomic_read(&(x)->rssi_index)

INTDEF DeeTypeObject ReSubStrings_Type;
INTDEF DeeTypeObject ReSubStringsIterator_Type;
INTDEF DeeTypeObject ReSubBytes_Type;
INTDEF DeeTypeObject ReSubBytesIterator_Type;

#define ReSubStrings_Malloc(ngroups)                                       \
	((ReSubStrings *)DeeObject_Malloc(offsetof(ReSubStrings, rss_groups) + \
	                                  (ngroups) * sizeof(struct DeeRegexMatch)))
#define ReSubStrings_Free(self) DeeObject_Free(self)
#define ReSubStrings_Init(self, baseown, baseptr, ngroups) \
	(void)(DeeObject_Init(self, &ReSubStrings_Type),       \
	       (self)->rss_ngroups = (ngroups),                \
	       (self)->rss_baseown = (baseown),                \
	       Dee_Incref((self)->rss_baseown),                \
	       (self)->rss_baseptr = (__BYTE_TYPE__ const *)(baseptr))


#define ReSubBytes                  ReSubStrings
#define ReSubBytesIterator          ReSubStringsIterator
#define ReSubBytesIterator_GetIndex ReSubStringsIterator_GetIndex
#define ReSubBytes_Malloc           ReSubStrings_Malloc
#define ReSubBytes_Free             ReSubStrings_Free
#define ReSubBytes_Init(self, baseown, baseptr, ngroups) \
	(void)(DeeObject_Init(self, &ReSubBytes_Type),       \
	       (self)->rss_ngroups = (ngroups),              \
	       (self)->rss_baseown = (baseown),              \
	       Dee_Incref((self)->rss_baseown),              \
	       (self)->rss_baseptr = (__BYTE_TYPE__ const *)(baseptr))

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_REGROUPS_H */
