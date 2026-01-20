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
#ifndef GUARD_DEEMON_STRUCT_H
#define GUARD_DEEMON_STRUCT_H 1

#include "api.h"

#include "object.h"
#include "types.h"

#include <stddef.h> /* size_t */

DECL_BEGIN

/*
 * Generic object constructor / destructor / etc. operators
 * for objects that are simple value containers and describe
 * all their values via "struct type_member" (with the same
 * also going for base classes)
 *
 * >> typedef struct {
 * >>     OBJECT_HEAD
 * >>     int a;
 * >>     int b;
 * >>     struct Dee_variant c;
 * >> } MyObject;
 * >>
 * >> PRIVATE struct type_member tpconst my_members[] = {
 * >>     TYPE_MEMBER_FIELD("a", STRUCT_INT, offsetof(MyObject, a)),
 * >>     TYPE_MEMBER_FIELD("b", STRUCT_INT, offsetof(MyObject, b)),
 * >>     TYPE_MEMBER_FIELD("c", STRUCT_VARIANT, offsetof(MyObject, c)),
 * >>     TYPE_MEMBER_END
 * >> };
 *
 * When using these operators with the above members, they will behave like this:
 * >> PRIVATE WUNUSED NONNULL((1)) int DCALL
 * >> my_object_ctor(MyObject *__restrict self) {
 * >>     self->a = 0;
 * >>     self->b = 0;
 * >>     Dee_variant_init_unbound(&self->c);
 * >>     return 0;
 * >> }
 * >> PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
 * >> my_object_copy(MyObject *__restrict self, MyObject *__restrict other) {
 * >>     self->a = other->a;
 * >>     self->b = other->b;
 * >>     Dee_variant_init_copy(&self->c, &other->c);
 * >>     return 0;
 * >> }
 * >> PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
 * >> my_object_deep(MyObject *__restrict self, MyObject *__restrict other) {
 * >>     self->a = other->a;
 * >>     self->b = other->b;
 * >>     return Dee_variant_init_deepcopy(&self->c, &other->c);
 * >> }
 * >> PRIVATE NONNULL((1)) int DCALL
 * >> my_object_init_kw(MyObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
 * >>     static DEFINE_KWLIST(kwlist, { K(a), K(b), K(c), KEND });
 * >>     int result;
 * >>     self->a = 0;
 * >>     self->b = 0;
 * >>     self->c.var_type = Dee_VARIANT_UNBOUND;
 * >>     self->c.var_data.d_object = NULL;
 * >>     result = DeeArg_UnpackKw(argc, argv, kw, kwlist, "|ddo:MyObject", &self->a, &self->b, &self->c.var_data.d_object);
 * >>     if (self->c.var_data.d_object) {
 * >>         Dee_Incref(self->c.var_data.d_object);
 * >>         self->c.var_type = Dee_VARIANT_OBJECT;
 * >>     }
 * >>     return result;
 * >> }
 * >> PRIVATE NONNULL((1)) void DCALL
 * >> my_object_fini(MyObject *__restrict self) {
 * >>     Dee_variant_fini(&self->c);
 * >> }
 * >> PRIVATE NONNULL((1, 2)) void DCALL
 * >> my_object_visit(MyObject *__restrict self, Dee_visit_t proc, void *arg) {
 * >>     Dee_variant_visit(&self->c);
 * >> }
 * >> PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
 * >> my_object_compare(MyObject *__restrict self, MyObject *__restrict other) {
 * >>     if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))
 * >>         return Dee_COMPARE_ERR;
 * >>     if (self->a != other->a)
 * >>         return Dee_CompareNe(self->a, other->a);
 * >>     if (self->b != other->b)
 * >>         return Dee_CompareNe(self->b, other->b);
 * >>     return Dee_variant_compare(&self->c, &other->c);
 * >> }
 * >> ...
 *
 * Constructor/repr parameters are ordered the same as struct members are, with
 * members from base classes coming first, but being allowed to be skipped if
 * a relevant sub-class defines a different name:
 * - the name used for the attribute at some offset is always the first attribute
 *   name encountered during MRO resolution.
 * - Attributes are ordered in reverse MRO resolution order.
 *
 * WARNING: The following struct types CANNOT be used (since they require initialization
 *          which isn't supported by `DeeStructObject_*' which requires that all fields
 *          be optional):
 * - "STRUCT_OBJECT"      (use "STRUCT_OBJECT_OPT" instead)
 * - "STRUCT_WOBJECT"     (use "STRUCT_WOBJECT_OPT" instead)
 * - "STRUCT_CSTR"        (owner wouldn't be reference; use "STRUCT_OBJECT" instead)
 * - "STRUCT_CSTR_OPT"    (owner wouldn't be reference; use "STRUCT_OBJECT" instead)
 * - "STRUCT_CSTR_EMPTY"  (owner wouldn't be reference; use "STRUCT_OBJECT" instead)
 * - "STRUCT_STRING"      (impossible to know max-length; use "STRUCT_OBJECT" instead)
 */
struct Dee_serial;
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeStructObject_Ctor(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeStructObject_Copy(DeeObject *__restrict self, DeeObject *__restrict other);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeStructObject_Deep(DeeObject *__restrict self, DeeObject *__restrict other);
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeStructObject_Init(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeStructObject_InitKw(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeStructObject_Serialize(DeeObject *__restrict self, struct Dee_serial *__restrict writer, Dee_seraddr_t addr);
DFUNDEF NONNULL((1)) void DCALL DeeStructObject_Fini(DeeObject *__restrict self); /* Only finalizes fields defined by "Dee_TYPE(self)" */
DFUNDEF NONNULL((1)) void DCALL DeeStructObject_Visit(DeeTypeObject *tp_self, DeeObject *__restrict self, Dee_visit_t proc, void *arg); /* Remember to set "Dee_TF_TPVISIT" */
DFUNDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeStructObject_PrintRepr(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
DFUNDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL DeeStructObject_Hash(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeStructObject_Compare(DeeObject *lhs, DeeObject *rhs);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeStructObject_CompareEq(DeeObject *lhs, DeeObject *rhs);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeStructObject_TryCompareEq(DeeObject *lhs, DeeObject *rhs);
DDATDEF struct Dee_type_cmp DeeStructObject_Cmp;


#ifdef DEE_SOURCE
#define Dee_type_member type_member
#endif /* DEE_SOURCE */
struct Dee_type_member;

/* Enumerate struct object fields in order as they should be accepted
 * by constructors, and are printed by "DeeStructObject_PrintRepr()".
 *
 * Also does all the handling to prevent fields that have been renamed
 * by sub-classes from being enumerated too early with incorrect names
 *
 * @param: undo: When non-NULL, invoke this for every already-processed
 *               field if `cb' happens to return a negative value.
 *
 * @return: >= 0: Success (return value is the sum of return values from "cb")
 * @return: < 0:  Enumeration stopped prematurely (return value is first
 *                negative return value of "cb" -- this function can't throw
 *                any errors on its own, meaning as long as "cb" doesn't throw
 *                any errors when returning negative, this function returning
 *                negative also means that no exception was thrown) */
typedef NONNULL_T((2, 3)) Dee_ssize_t
(DCALL *Dee_struct_object_foreach_field_cb_t)(void *arg, DeeTypeObject *declaring_type,
                                              struct Dee_type_member const *field);
typedef NONNULL_T((2, 3)) void
(DCALL *Dee_struct_object_foreach_field_undo_t)(void *arg, DeeTypeObject *declaring_type,
                                                struct Dee_type_member const *field);
DFUNDEF NONNULL((1, 2)) Dee_ssize_t DCALL
DeeStructObject_ForeachField(DeeTypeObject *__restrict type,
                             Dee_struct_object_foreach_field_cb_t cb,
                             Dee_struct_object_foreach_field_undo_t undo,
                             void *arg);


DECL_END

#endif /* !GUARD_DEEMON_STRUCT_H */
