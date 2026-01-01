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
#ifndef GUARD_DEEMON_SERIAL_H
#define GUARD_DEEMON_SERIAL_H 1

#include "api.h"
/**/

#include "types.h"
/**/

#include <hybrid/typecore.h>

/*
 * Object serialization is a general-purpose API to:
 * - Create (potentially deep) copies of objects
 * - Extract/enumerate relocation positions in objects
 * - Allow binary images of objects to be written to files
 *
 *
 * The general idea is:
 * - Every object that *can* be serialized, should implement "tp_serialize"
 * - The implementation of "tp_serialize" should make use of functions from
 *   this header
 * - The general-purpose interface "DeeSerial" can be implemented by many
 *   feature-consumers and can be used to:
 *   - Generate object dumps for use in ".dec" files
 *   - Produce deep copies of objects (because think about it: what's the
 *     difference between deep-copying an object and writing that same
 *     object into a .dec file, then loading that .dec file into memory)
 *   - Doing regular (shallow) copies of an object (same as deep copy, but
 *     forgoing any kind of recursion)
 *
 *
 * For more information on .dec files, see ./dec.md
 */

DECL_BEGIN

struct Dee_serial;
typedef struct Dee_serial DeeSerial;

/* TODO: "DeeSerial" can also be used to implement "deepcopy" (in a way that
 *       solves the problem of non-GC objects being copied whilst some nested
 *       GC object holding another reference to the original object; in the
 *       current deepcopy impl, this causes the original object to be copied
 *       twice).
 *
 * Even beyond this, "DeeSerial" can also be used to implement "copy", too.
 * For that, simply do what "deepcopy" does, but rather than recurse on each
 * nested object, simply re-encode a reference to the source object instead!
 * (though in this regard, this can only be a fallback "copy", since certain
 * types of objects; specifically certain Iterator types; still need to do
 * certain nested copy operations to ensure the copied iterator is detached
 * from the original iterator). */


/* Serialization address (points into the abstract serialization buffer) */
#ifndef Dee_seraddr_t_DEFINED
#define Dee_seraddr_t_DEFINED
typedef __UINTPTR_TYPE__ Dee_seraddr_t;
#endif /* !Dee_seraddr_t_DEFINED */
#define Dee_SERADDR_INVALID ((Dee_seraddr_t)-1)
#define Dee_SERADDR_ISOK(x) __likely((x) != Dee_SERADDR_INVALID)

struct Dee_serial_type {
	/* NOTE: All operators are [1..1][const] */

	/* Convert address into serialization buffer. The returned pointer may
	 * only be accessed until the next call to this, or any other operators below,
	 * and is only valid for [return,return+SIZEOF_REMAINING_ALLOCATION_OF(addr)]:
	 * >> Dee_seraddr_t addr = DeeSerial_Malloc(self, 42);
	 * >> void *ptr = DeeSerial_Addr2Mem(self, addr + 10, void);
	 * >> // Memory may now be written for range [ptr,ptr+32)
	 *
	 * - Behavior is undefined when "addr" is "Dee_SERADDR_INVALID"
	 * - Behavior is weak undefined when "addr" points at the start
	 *   of 'DeeSerial_Malloc(self, 0)' (meaning that in this case,
	 *   you're not allowed to dereference the returned pointer) */
	ATTR_RETNONNULL_T WUNUSED_T NONNULL_T((1)) void *
	(DCALL *set_addr2mem)(DeeSerial *__restrict self, Dee_seraddr_t addr);

	/* Allocate generic heap memory (as per "Dee_Malloc()")
	 * @param: ref: When non-NULL, a reference pointer describing where data originates from.
	 *              This should **ONLY** be given when serializing "[owned][const]" fields of
	 *              a containing object (e.g. "DeeClassDescriptorObject::cd_clsop_list"), but
	 *              **MUST** be "NULL" when serializing "[owned][lock(...)]" fields (e.g.
	 *              "DeeListObject::l_list::ol_elemv").
	 *              The reason why this is needed is to allow later "DeeSerial_PutPointer()"
	 *              calls to connect the dots and understand that a given pointer actually
	 *              points into a block previously returned by `DeeSerial_Malloc()', similar
	 *              to how it can do so for `DeeSerial_ObjectMalloc()'
	 * @return: * : Serialized address of heap buffer
	 * @return: Dee_SERADDR_INVALID: Allocation failed (for "set_malloc" and "set_calloc": error was thrown) */
	WUNUSED_T NONNULL_T((1)) Dee_seraddr_t (DCALL *set_malloc)(DeeSerial *__restrict self, size_t num_bytes, /*0..1*/ void *ref);
	WUNUSED_T NONNULL_T((1)) Dee_seraddr_t (DCALL *set_calloc)(DeeSerial *__restrict self, size_t num_bytes, /*0..1*/ void *ref);
	WUNUSED_T NONNULL_T((1)) Dee_seraddr_t (DCALL *set_trymalloc)(DeeSerial *__restrict self, size_t num_bytes, /*0..1*/ void *ref);
	WUNUSED_T NONNULL_T((1)) Dee_seraddr_t (DCALL *set_trycalloc)(DeeSerial *__restrict self, size_t num_bytes, /*0..1*/ void *ref);

	/* Free generic heap memory (as per "Dee_Free()")
	 * Only allowed to be called for the most-recent non-Dee_SERADDR_INVALID
	 * return value of one of the allocation functions above. Behavior is hard
	 * undefined if you try to free a buffer that isn't the most recent one. */
	NONNULL_T((1)) void (DCALL *set_free)(DeeSerial *__restrict self, Dee_seraddr_t addr, /*0..1*/ void *ref);

	/* Allocate generic object heap memory (as per "DeeObject_Malloc()")
	 * These functions will have automatically pre-initialized:
	 * >> DeeSerial_ADDR2MEM(self, return, DeeObject)->ob_refcnt = 1;
	 * >> DeeSerial_PutObject(self, return + offsetof(DeeObject, ob_type), Dee_TYPE(ref));
	 *
	 * Meaning that the caller need only initialize all non-standard object fields. */
	WUNUSED_T NONNULL_T((1, 3)) Dee_seraddr_t (DCALL *set_object_malloc)(DeeSerial *__restrict self, size_t num_bytes, DeeObject *__restrict ref);
	WUNUSED_T NONNULL_T((1, 3)) Dee_seraddr_t (DCALL *set_object_calloc)(DeeSerial *__restrict self, size_t num_bytes, DeeObject *__restrict ref);
	WUNUSED_T NONNULL_T((1, 3)) Dee_seraddr_t (DCALL *set_object_trymalloc)(DeeSerial *__restrict self, size_t num_bytes, DeeObject *__restrict ref);
	WUNUSED_T NONNULL_T((1, 3)) Dee_seraddr_t (DCALL *set_object_trycalloc)(DeeSerial *__restrict self, size_t num_bytes, DeeObject *__restrict ref);

	/* Free generic heap memory (as per "DeeObject_Free()")
	 * Same restrictions of `set_free' regarding order of free() operations also apply to this */
	NONNULL_T((1, 3)) void (DCALL *set_object_free)(DeeSerial *__restrict self, Dee_seraddr_t addr, DeeObject *__restrict ref);

	/* Same as above, but must be used for GC-objects (as per "DeeGCObject_Malloc()") */
	WUNUSED_T NONNULL_T((1, 3)) Dee_seraddr_t (DCALL *set_gcobject_malloc)(DeeSerial *__restrict self, size_t num_bytes, DeeObject *__restrict ref);
	WUNUSED_T NONNULL_T((1, 3)) Dee_seraddr_t (DCALL *set_gcobject_calloc)(DeeSerial *__restrict self, size_t num_bytes, DeeObject *__restrict ref);
	WUNUSED_T NONNULL_T((1, 3)) Dee_seraddr_t (DCALL *set_gcobject_trymalloc)(DeeSerial *__restrict self, size_t num_bytes, DeeObject *__restrict ref);
	WUNUSED_T NONNULL_T((1, 3)) Dee_seraddr_t (DCALL *set_gcobject_trycalloc)(DeeSerial *__restrict self, size_t num_bytes, DeeObject *__restrict ref);

	/* Free generic heap memory (as per "DeeGCObject_Free()") */
	NONNULL_T((1, 3)) void (DCALL *set_gcobject_free)(DeeSerial *__restrict self, Dee_seraddr_t addr, DeeObject *__restrict ref);

	/* Serialize a `void *' field at `addrof_pointer' as being populated with the
	 * effectively final value of `DeeSerial_Addr2Mem(self, addrof_target, void)'
	 * @return: 0 : Success
	 * @return: -1: Error */
	WUNUSED_T NONNULL_T((1)) int
	(DCALL *set_putaddr)(DeeSerial *__restrict self,
	                     Dee_seraddr_t addrof_pointer,
	                     Dee_seraddr_t addrof_target);

	/* Serialize a `DREF DeeObject *' field at `addrof_object'
	 * as being populated with a reference to `ob'
	 * @return: 0 : Success
	 * @return: -1: Error */
	WUNUSED_T NONNULL_T((1, 3)) int
	(DCALL *set_putobject)(DeeSerial *__restrict self,
	                       Dee_seraddr_t addrof_object,
	                       DeeObject *__restrict ob);

	/* Same as `set_putobject', but encode a reference ob "ob",
	 * but have the pointer be at `(byte_t *)ob + offset_into_ob'
	 * @return: 0 : Success
	 * @return: -1: Error */
	WUNUSED_T NONNULL_T((1, 3)) int
	(DCALL *set_putobject_ex)(DeeSerial *__restrict self,
	                          Dee_seraddr_t addrof_object,
	                          DeeObject *__restrict ob,
	                          ptrdiff_t offset_into_ob);

	/* Serialize a `void *' field at `addrof_pointer' as being populated with the address of
	 * a static object at `pointer' ("static" here meaning that `DeeModule_OfPointer()' will
	 * return a non-NULL pointer for `pointer'), or as pointing into the payload portion of
	 * another object or heap block that had already been serialized (iow: "pointer" points
	 * into [ref,ref+num_bytes] (yes: closed range; iow: "ref+num_bytes" (1 past last byte) is
	 * still recognized and linked) of a prior `set_object_malloc', `set_gcobject_malloc', ...).
	 * If neither is the case, an error is thrown.
	 * @return: 0 : Success
	 * @return: -1: Error */
	WUNUSED_T NONNULL_T((1, 3)) int
	(DCALL *set_putpointer)(DeeSerial *__restrict self, Dee_seraddr_t addrof_pointer,
	                        void const *__restrict pointer);
};

#define Dee_SERIAL_HEAD \
	struct Dee_serial_type const *ser_type;
struct Dee_serial {
	struct Dee_serial_type const *const ser_type; /* [1..1][const] Type of serializer */
	/* Serializer-specific fields go here... */
};


/* Address buffer lookup */
#define DeeSerial_Addr2Mem(self, addr, T) \
	((T *)(*(self)->ser_type->set_addr2mem)(self, addr))

/* Allocate generic heap memory (as per "Dee_Malloc()")
 * @param: ref: When non-NULL, a reference pointer describing where data originates from.
 *              This should **ONLY** be given when serializing "[owned][const]" fields of
 *              a containing object (e.g. "DeeClassDescriptorObject::cd_clsop_list"), but
 *              **MUST** be "NULL" when serializing "[owned][lock(...)]" fields (e.g.
 *              "DeeListObject::l_list::ol_elemv").
 *              The reason why this is needed is to allow later "DeeSerial_PutPointer()"
 *              calls to connect the dots and understand that a given pointer actually
 *              points into a block previously returned by `DeeSerial_Malloc()', similar
 *              to how it can do so for `DeeSerial_ObjectMalloc()'
 *              TLDR:
 *              - If you use "DeeSerial_TryMalloc()" because of a lock, you probably want to pass "NULL"
 *              - Otherwise, pass the [const] source pointer
 *
 * @return: * : Serialized address of heap buffer
 * @return: Dee_SERADDR_INVALID: Allocation failed (for "DeeSerial_Malloc" and "DeeSerial_Calloc": error was thrown) */
#define DeeSerial_Malloc(self, num_bytes, ref)    (*(self)->ser_type->set_malloc)(self, num_bytes, ref)
#define DeeSerial_Calloc(self, num_bytes, ref)    (*(self)->ser_type->set_calloc)(self, num_bytes, ref)
#define DeeSerial_TryMalloc(self, num_bytes, ref) (*(self)->ser_type->set_trymalloc)(self, num_bytes, ref)
#define DeeSerial_TryCalloc(self, num_bytes, ref) (*(self)->ser_type->set_trycalloc)(self, num_bytes, ref)

/* Free generic heap memory (as per "Dee_Free()")
 * Only allowed to be called for the most-recent non-Dee_SERADDR_INVALID
 * return value of one of the allocation functions above. Behavior is hard
 * undefined if you try to free a buffer that isn't the most recent one.
 *
 * NOTE: DON'T call this method for error-cleanup -- On error, the
 *       owner of the serializer will free all allocated memory! */
#define DeeSerial_Free(self, addr, ref) (*(self)->ser_type->set_free)(self, addr, ref)

/* Allocate generic object heap memory (as per "DeeObject_Malloc()")
 * These functions will have automatically pre-initialized:
 * >> DeeSerial_ADDR2MEM(self, return, DeeObject)->ob_refcnt = 1;
 * >> DeeSerial_PutObject(self, return + offsetof(DeeObject, ob_type), Dee_TYPE(ref));
 *
 * Meaning that the caller need only initialize all non-standard object fields. */
#define DeeSerial_ObjectMalloc(self, num_bytes, ref)    (*(self)->ser_type->set_object_malloc)(self, num_bytes, Dee_AsObject(ref))
#define DeeSerial_ObjectCalloc(self, num_bytes, ref)    (*(self)->ser_type->set_object_calloc)(self, num_bytes, Dee_AsObject(ref))
#define DeeSerial_ObjectTryMalloc(self, num_bytes, ref) (*(self)->ser_type->set_object_trymalloc)(self, num_bytes, Dee_AsObject(ref))
#define DeeSerial_ObjectTryCalloc(self, num_bytes, ref) (*(self)->ser_type->set_object_trycalloc)(self, num_bytes, Dee_AsObject(ref))

/* Free generic heap memory (as per "DeeObject_Free()")
 * Same restrictions of `set_free' regarding order of free() operations also apply to this
 *
 * NOTE: DON'T call this method for error-cleanup -- On error, the
 *       owner of the serializer will free all allocated memory! */
#define DeeSerial_ObjectFree(self, addr, ref) (*(self)->ser_type->set_object_free)(self, addr, ref)

/* Same as above, but must be used for GC-objects (as per "DeeGCObject_Malloc()") */
#define DeeSerial_GCObjectMalloc(self, num_bytes, ref)    (*(self)->ser_type->set_gcobject_malloc)(self, num_bytes, Dee_AsObject(ref))
#define DeeSerial_GCObjectCalloc(self, num_bytes, ref)    (*(self)->ser_type->set_gcobject_calloc)(self, num_bytes, Dee_AsObject(ref))
#define DeeSerial_GCObjectTryMalloc(self, num_bytes, ref) (*(self)->ser_type->set_gcobject_trymalloc)(self, num_bytes, Dee_AsObject(ref))
#define DeeSerial_GCObjectTryCalloc(self, num_bytes, ref) (*(self)->ser_type->set_gcobject_trycalloc)(self, num_bytes, Dee_AsObject(ref))

/* Free generic heap memory (as per "DeeGCObject_Free()")
 *
 * NOTE: DON'T call this method for error-cleanup -- On error, the
 *       owner of the serializer will free all allocated memory! */
#define DeeSerial_GCObjectFree(self, addr, ref) (*(self)->ser_type->set_gcobject_free)(self, addr, ref)


/* Serialize a `void *' field at `addrof_pointer' as being populated with the
 * effectively final value of `DeeSerial_Addr2Mem(self, addrof_target, void)'
 * @return: 0 : Success
 * @return: -1: Error */
#ifdef __INTELLISENSE__
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL DeeSerial_PutAddr)(DeeSerial *__restrict self,
	                      Dee_seraddr_t addrof_pointer,
	                      Dee_seraddr_t addrof_target);
#else /* __INTELLISENSE__ */
#define DeeSerial_PutAddr(self, addrof_pointer, addrof_target) \
	__builtin_expect((*(self)->ser_type->set_putaddr)(self, addrof_pointer, addrof_target), 0)
#endif /* !__INTELLISENSE__ */

/* Serialize a `DREF DeeObject *' field at `addrof_object' as being populated with a reference to `ob'
 * @return: 0 : Success
 * @return: -1: Error */
#define DeeSerial_PutObject(self, addrof_object, ob) \
	__builtin_expect((*(self)->ser_type->set_putobject)(self, addrof_object, Dee_AsObject(ob)), 0)

/* Same as `DeeSerial_PutObject', but encode a reference ob "ob",
 * but have the pointer be at `(byte_t *)ob + offset_into_ob'
 * @return: 0 : Success
 * @return: -1: Error */
#define DeeSerial_PutObjectEx(self, addrof_object, ob, offset_into_ob) \
	__builtin_expect((*(self)->ser_type->set_putobject_ex)(self, addrof_object, Dee_AsObject(ob), offset_into_ob), 0)

DFUNDEF WUNUSED NONNULL((1)) int
(DCALL DeeSerial_XPutObject)(DeeSerial *__restrict self,
                             Dee_seraddr_t addrof_object,
                             /*0..1*/ DeeObject *ob);
DFUNDEF WUNUSED NONNULL((1, 3)) int
(DCALL DeeSerial_PutObjectInherited)(DeeSerial *__restrict self, Dee_seraddr_t addrof_object,
                                     /*1..1*/ /*inherit(always)*/ DREF DeeObject *__restrict ob);
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL DeeSerial_XPutObjectInherited)(DeeSerial *__restrict self, Dee_seraddr_t addrof_object,
                                      /*0..1*/ /*inherit(always)*/ DREF DeeObject *ob);
#define DeeSerial_XPutObject(self, addrof_object, /*0..1*/ ob) \
	__builtin_expect((DeeSerial_XPutObject)(self, addrof_object, Dee_AsObject(ob)), 0)
#define DeeSerial_PutObjectInherited(self, addrof_object, /*1..1*/ /*inherit(always)*/ /*DREF*/ ob) \
	__builtin_expect((DeeSerial_PutObjectInherited)(self, addrof_object, Dee_AsObject(ob)), 0)
#define DeeSerial_XPutObjectInherited(self, addrof_object, /*0..1*/ /*inherit(always)*/ /*DREF*/ ob) \
	__builtin_expect((DeeSerial_XPutObjectInherited)(self, addrof_object, Dee_AsObject(ob)), 0)


/* Serialize a `void *' field at `addrof_pointer' as being populated with the address of
 * a static object at `pointer' ("static" here meaning that `DeeModule_OfPointer()' will
 * return a non-NULL pointer for `pointer'), or as pointing into the payload portion of
 * another object or heap block that had already been serialized (iow: "pointer" points
 * into [ref,ref+num_bytes] (yes: closed range; iow: "ref+num_bytes" (1 past last byte) is
 * still recognized and linked) of a prior `DeeSerial_ObjectMalloc', `DeeSerial_GCObjectMalloc',
 * ...). If neither is the case, an error is thrown.
 * @return: 0 : Success
 * @return: -1: Error */
#ifdef __INTELLISENSE__
DFUNDEF WUNUSED NONNULL((1, 3)) int
(DCALL DeeSerial_PutPointer)(DeeSerial *__restrict self,
                             Dee_seraddr_t addrof_pointer,
                             void const *__restrict pointer);
#else /* __INTELLISENSE__ */
#define DeeSerial_PutPointer(self, addrof_pointer, pointer) \
	__builtin_expect((*(self)->ser_type->set_putpointer)(self, addrof_pointer, pointer), 0)
#endif /* !__INTELLISENSE__ */
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL DeeSerial_XPutPointer)(DeeSerial *__restrict self,
                              Dee_seraddr_t addrof_pointer,
                              void const *pointer);
#ifndef __INTELLISENSE__
#define DeeSerial_XPutPointer(self, addrof_pointer, pointer) \
	__builtin_expect(DeeSerial_XPutPointer(self, addrof_pointer, pointer), 0)
#endif /* !__INTELLISENSE__ */

#ifdef CONFIG_BUILDING_DEEMON
#define DeeSerial_PutStaticDeemon(self, addrof_pointer, pointer)  DeeSerial_PutPointer(self, addrof_pointer, pointer)
#define DeeSerial_XPutStaticDeemon(self, addrof_pointer, pointer) DeeSerial_XPutPointer(self, addrof_pointer, pointer)
#endif /* CONFIG_BUILDING_DEEMON */


/* Helper wrapper for encoding a pointer to the memdup of `data' at `addrof_pointer':
 * >> Dee_seraddr_t addrof_dup = DeeSerial_Malloc(self, num_bytes);
 * >> memcpy(DeeSerial_Addr2Mem(self, addrof_dup, void), data, num_bytes);
 * >> return DeeSerial_PutAddr(self, addrof_pointer, addrof_dup); */
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL DeeSerial_PutMemdup)(DeeSerial *__restrict self,
                            Dee_seraddr_t addrof_pointer,
                            void const *data, size_t num_bytes);
#ifndef __INTELLISENSE__
#define DeeSerial_PutMemdup(self, addrof_pointer, data, num_bytes) \
	__builtin_expect(DeeSerial_PutMemdup(self, addrof_pointer, data, num_bytes), 0)
#endif /* !__INTELLISENSE__ */



/* Inplace-serialize an object references:
 * >> DREF DeeObject *obj = *DeeSerial_Addr2Mem(self, addrof_object, DREF DeeObject *);
 * >> int result = DeeSerial_PutObject(self, addrof_object, obj);
 * >> Dee_Decref(obj);
 * >> return result; */
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL DeeSerial_InplacePutObject)(DeeSerial *__restrict self,
                                   Dee_seraddr_t addrof_object);
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL DeeSerial_XInplacePutObject)(DeeSerial *__restrict self,
                                    Dee_seraddr_t addrof_object);
#ifndef __INTELLISENSE__
#define DeeSerial_InplacePutObject(self, addrof_object) \
	__builtin_expect(DeeSerial_InplacePutObject(self, addrof_object), 0)
#define DeeSerial_XInplacePutObject(self, addrof_object) \
	__builtin_expect(DeeSerial_XInplacePutObject(self, addrof_object), 0)
#endif /* !__INTELLISENSE__ */


/* Inplace-serialize an array of object references. Said
 * array of object references is **ALWAYS** inherited:
 * >> while (objc) {
 * >>     int error = DeeSerial_InplacePutObject(self, addrof_objects);
 * >>     --objc;
 * >>     addrof_objects += sizeof(DREF DeeObject *);
 * >>     if unlikely(error) {
 * >>         Dee_Decrefv(DeeSerial_Addr2Mem(self, addrof_objects, DREF DeeObject *), objc);
 * >>         return error;
 * >>     }
 * >> }
 * >> return 0; */
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL DeeSerial_InplacePutObjectv)(DeeSerial *__restrict self,
                                    Dee_seraddr_t addrof_objects,
                                    size_t objc);
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL DeeSerial_XInplacePutObjectv)(DeeSerial *__restrict self,
                                     Dee_seraddr_t addrof_objects,
                                     size_t objc);
#ifndef __INTELLISENSE__
#define DeeSerial_InplacePutObjectv(self, addrof_objects, objc) \
	__builtin_expect(DeeSerial_InplacePutObjectv(self, addrof_objects, objc), 0)
#define DeeSerial_XInplacePutObjectv(self, addrof_objects, objc) \
	__builtin_expect(DeeSerial_XInplacePutObjectv(self, addrof_objects, objc), 0)
#endif /* !__INTELLISENSE__ */

/* Encode an `DREF DeeObject *addrof_objv[objc]' from "objv" */
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL DeeSerial_PutObjectv)(DeeSerial *__restrict self, Dee_seraddr_t addrof_objv,
                             /*1..1*/ DeeObject *const *objv, size_t objc);
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL DeeSerial_XPutObjectv)(DeeSerial *__restrict self, Dee_seraddr_t addrof_objv,
                              /*0..1*/ DeeObject *const *objv, size_t objc);


DECL_END

#endif /* !GUARD_DEEMON_SERIAL_H */
