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
#ifndef GUARD_DEEMON_CLASS2_H
#define GUARD_DEEMON_CLASS2_H 1

#include "api.h"
#include "object.h"
#include "util/rwlock.h"
#include <stddef.h>
#include <stdint.h>

DECL_BEGIN

typedef struct class_descriptor_object DeeClassDescriptorObject;
struct string_object;

#define CLASS_GETSET_GET 0 /* Offset to the getter of a user-defined getset in a class. */
#define CLASS_GETSET_DEL 1 /* Offset to the delete of a user-defined getset in a class. */
#define CLASS_GETSET_SET 2 /* Offset to the setter of a user-defined getset in a class. */

#define CLASS_ATTRIBUTE_FNORMAL   0x0000 /* Normal class attribute flags. */
#define CLASS_ATTRIBUTE_FPRIVATE  0x0001 /* This attribute is only accessible from this-call functions with an instance of the class as this-argument. */
#define CLASS_ATTRIBUTE_FREADONLY 0x0002 /* The attribute can only ever be when not already bound (and it cannot be unbound). */
#define CLASS_ATTRIBUTE_FMETHOD   0x0004 /* When accessed as get in `foo.bar', return an `instancemethod(MEMBER_TABLE[ca_addr],foo)' (calling `foo.bar()' will similarly perform a this-call). */
#define CLASS_ATTRIBUTE_FGETSET   0x0008 /* Access to the attribute is done via get/set, with the callbacks being `CLASS_GETSET_*' offsets from `ca_addr'.
                                          * When `CLASS_ATTRIBUTE_FMETHOD' is set, callbacks are invoked as this-calls.
                                          * When `CLASS_ATTRIBUTE_FREADONLY' is set, only `CLASS_GETSET_GET' is ever accessed,
                                          * and all other callbacks behave as though they were unbound. */
#define CLASS_ATTRIBUTE_FCLASSMEM 0x1000 /* An instance-attribute is stored in class memory (usually set for instance member functions).
                                          * NOTE: Ignored when used by attributes in `cd_cattr_list', where
                                          *       operation is always done like it would be when it was set. */
#define CLASS_ATTRIBUTE_FMASK     0x100f /* Mask of known flag bits. */

struct class_attribute {
    DREF struct string_object *ca_name; /* [0..1][const] Name of this member.
                                         * NOTE: NULL indicates a sentinel/unused entry. */
    dhash_t                    ca_hash; /* [== cme_name->s_hash][const] */
    DREF struct string_object *ca_doc;  /* [0..1][const] A Documentation string for this member. */
    uint16_t                   ca_addr; /* [const] Attribute address within the instance / class table. */
    uint16_t                   ca_flag; /* Class member flags (Set of `CLASS_ATTRIBUTE_F*') */
#if __SIZEOF_POINTER__ > 4
    uint16_t                   ca_pad[(sizeof(void *)/2)-2];
#endif
};

/* A special operator invoked using the same
 * arguments as passed to the class constructor.
 * It should return a sequence that is then cast to a tuple
 * before being used as 
 * WARNING: Unlike all other class operators, this one
 *          is _NOT_ called using thiscall conventions,
 *          but instead as a regular function.
 *          This is necessary because at the time that
 *          it is called, the instance then used as
 *          this-argument hasn't yet been initialized.
 * NOTE: When this operator isn't set, super-classes are
 *       always initialized using their default-constructor,
 *       essentially passing an empty argument list. */
#define CLASS_OPERATOR_SUPERARGS   OPERATOR_USERCOUNT
#define CLASS_OPERATOR_USERCOUNT  (OPERATOR_USERCOUNT+1)


struct class_operator {
    uint16_t co_name; /* [const] Operator name (`(uint16_t)-1' for end-of-chain) */
    uint16_t co_addr; /* [const] Operator address (within the class member table `cd_members').
                       * Operators are invoked like attributes with the following flags:
                       * `CLASS_ATTRIBUTE_FMETHOD|CLASS_ATTRIBUTE_FCLASSMEM', meaning
                       * they are invoked as this-calls, with the callback itself stored
                       * in class memory.
                       * WARNING: When overwriting the value of a class operator after the previous
                       *          one has already been used may not actually function, as operators
                       *          get cached upon first use, such-as to allow for operators to be
                       *          inherited from base-classes in order to ensure O(1) execution time.
                       * HINT: When the pointed-to class member is `NULL' (unbound), operator
                       *       search won't continue, but rather cause a not-implemented error
                       *       to be thrown, thus allowing you to explicitly delete an operator
                       *       by simply declaring it, but not assigning a callback.
                       * TODO: Implement deleted operators once the new class system reaches user-code:
                       *       >> class MyClass1 {
                       *       >> }
                       *       >> class MyClass2 {
                       *       >>     operator str = del;
                       *       >> }
                       *       >> print MyClass1(); // `MyClass1' (inherited)
                       * TODO: Change the syntax for defining classes without bases from simply
                       *       omitting a base class, to writing `class MyClass: none { }'
                       *       With that in mind, use `object from deemon' as default-base
                       *       for user-defined classes.
                       * TODO: Also add a syntax `this = super;', which has a similar meaning as
                       *      `this(...): super(...) {}', meaning that during construction,
                       *       all arguments are simply forwarded to the super-class.
                       *       NOTE: We can't use `TP_FINHERITCTOR' for this, because additional
                       *             instance fields may require initialization in a custom
                       *             constructor:
                       *          >> class MyClass: foo {
                       *          >>     this = super;
                       *          >>     my_field = 42; // Results in a hidden constructor to assign this value.
                       *          >> }
                       *       
                       */
};


struct class_descriptor_object {
    /* The descriptor for the configuration of a user-defined class object:
     * >> class MyClass {
     * >>     class function cfunc() {
     * >>         print "class function";
     * >>     }
     * >>     function ifunc() {
     * >>         print "instance function";
     * >>     }
     * >>     class member cmember = "class member";
     * >>     member imember = "instance member";
     * >>     class property cprop = {
     * >>         get()  { return "class member"; }
     * >>         del()  { print "class member"; }
     * >>         set(v) { print "class member"; }
     * >>     }
     * >>     property iprop = {
     * >>         get()  { return "instance member"; }
     * >>         del()  { print "instance member"; }
     * >>         set(v) { print "instance member"; }
     * >>     }
     * >> }
     * DESCRIPTOR (operators are omitted, as `imember = ...' would actually imply a constructor):
     * >> {
     * >>     .cd_name       = "MyClass",
     * >>     .cd_doc        = NULL,
     * >>     .cd_cmemb_size = 9, // cfunc, ifun, cmember, cprop.get/del/set, iprop.get/del/set
     * >>     .cd_imemb_size = 1, // imember
     * >>     .cd_cattr_mask = 7, // cfunc, cmember, cprop  (3 -> 7)
     * >>     .cd_cattr_list = {
     * >>         { "cfunc",   .ca_addr = 0, .ca_flag = CLASS_ATTRIBUTE_FREADONLY },
     * >>         { "cmember", .ca_addr = 1, .ca_flag = CLASS_ATTRIBUTE_FNORMAL },
     * >>         { "cprop",   .ca_addr = 2, .ca_flag = CLASS_ATTRIBUTE_FGETSET },
     * >>     },
     * >>     .cd_iattr_mask = 7, // ifunc, imember, iprop  (3 -> 7)
     * >>     .cd_iattr_list = {
     * >>         { "ifunc",   .ca_addr = 5, .ca_flag = CLASS_ATTRIBUTE_FCLASSMEM|CLASS_ATTRIBUTE_FMETHOD },
     * >>         { "imember", .ca_addr = 0, .ca_flag = CLASS_ATTRIBUTE_FNORMAL },
     * >>         { "iprop",   .ca_addr = 6, .ca_flag = CLASS_ATTRIBUTE_FCLASSMEM|CLASS_ATTRIBUTE_FMETHOD|CLASS_ATTRIBUTE_FGETSET },
     * >>     },
     * >> }
     * INSTANCE (class):
     * >> {
     * >>     .cd_members = {
     * >>         [0] = <function cfunc() { print "class function"; }>,
     * >>         [1] = <cmember = "class member">,
     * >>         [2 + CLASS_GETSET_GET] = <cprop:get: get()  { return "class member"; }>,
     * >>         [2 + CLASS_GETSET_DEL] = <cprop:del: del()  { print "class member"; }>,
     * >>         [2 + CLASS_GETSET_SET] = <cprop:set: set(v) { print "class member"; }>,
     * >>         [5] = <function ifunc() { print "instance function"; }>,
     * >>         [6 + CLASS_GETSET_GET] = <iprop:get: get()  { return "instance member"; }>,
     * >>         [6 + CLASS_GETSET_DEL] = <iprop:del: del()  { print "instance member"; }>,
     * >>         [6 + CLASS_GETSET_SET] = <iprop:set: set(v) { print "instance member"; }>
     * >>     }
     * >> }
     * INSTANCE (class-instance):
     * >> {
     * >>     .i_members = {
     * >>         [0] = <imember = "instance member">
     * >>     }
     * >> }
     */
    OBJECT_HEAD
    DREF struct string_object *cd_name;          /* [0..1][const] Name of the class. */
    DREF struct string_object *cd_doc;           /* [0..1][const] Documentation strings for the class itself, and its operators. */
    uint16_t                   cd_flags;         /* [const] Additional flags to set for the resulting type. */
    uint16_t                   cd_cmemb_size;    /* [const] The allocation size of the class member table. */
    uint16_t                   cd_imemb_size;    /* [const] The allocation size of the instance member table. */
    uint16_t                   cd_clsop_mask;    /* [const][!0] Mask for the `cd_clsop_list' hash-vector. */
    uint16_t                   cd_cattr_mask;    /* [const][!0] Mask for the `cd_cattr_list' hash-vector. */
    uint16_t                   cd_iattr_mask;    /* [const][!0] Mask for the `cd_cattr_list' hash-vector. */
#if __SIZEOF_POINTER__ > 4
    uint16_t                   cd_pad[sizeof(void *)/2]; /* ... */
#endif
    struct class_operator     *cd_clsop_list;    /* [1..cd_clsop_mask+1][owned_if(!= INTERNAL(empty-class-operator-table))]
                                                  * [const] The class operator address hash-vector. */
    struct class_attribute    *cd_cattr_list;    /* [1..cd_cattr_mask+1][owned_if(!= INTERNAL(empty-class-attribute-table))]
                                                  * [const] The class attribute hash-vector. */
    struct class_attribute     cd_iattr_list[1]; /* [cd_cattr_mask+1] The instance attribute hash-vector. */
};


DDATDEF DeeTypeObject DeeClassDescriptor_Type;
#define DeeClassDescriptor_Check(x)      DeeObject_InstanceOfExact(x,&DeeClassDescriptor_Type) /* DeeClassDescriptor_Type is final */
#define DeeClassDescriptor_CheckExact(x) DeeObject_InstanceOfExact(x,&DeeClassDescriptor_Type)




#define CLASS_HEADER_OPC1    8
#define CLASS_HEADER_OPC2  ((CLASS_OPERATOR_USERCOUNT+7)/8)

#if (CLASS_HEADER_OPC1*CLASS_HEADER_OPC2) < CLASS_OPERATOR_USERCOUNT
#error "FIXME: Not enough space for all available operators"
#endif

struct class_optable {
    /* [0..1][lock(:cd_lock,WRITE_ONCE)][*] Table of operators.
     * NOTE: Individual callback objects may be `ITER_DONE',
     *       indicative of that operator having been deleted
     *       explicitly. */
    DREF DeeObject *co_operators[CLASS_HEADER_OPC2];
};

struct class_desc {
    /* The class description tail embedded into type-objects
     * which have been initialized as custom user-classes. */
    DREF DeeClassDescriptorObject *cd_desc;   /* [1..1][const] The associated class descriptor.
                                               * This in turn contains all the relevant fields
                                               * required to accessing user-defined attributes. */
#ifndef CONFIG_NO_THREADS
    rwlock_t                       cd_lock;   /* Lock for accessing the class member table. */
#endif
    struct class_optable          *cd_ops[CLASS_HEADER_OPC1];
                                              /* [0..1][owned][lock(cd_lock,WRITE_ONCE)][*]
                                               * Table of cached operator callbacks. */
    DREF DeeObject                *cd_members[1024]; /* [0..1][lock(cd_lock)][cd_desc->cd_cmemb_size]
                                                      * The class member table (also contains
                                                      * instance-methods and operator callbacks). */
};

#define DeeClass_Check(self)    (DeeType_Check(self) && DeeType_IsClass(self))
#define DeeInstance_Check(self)  DeeType_IsClass(Dee_TYPE(self))

/* Returns the descriptor for a given class. */
#define DeeClass_DESC(self) \
     (ASSERT_OBJECT_TYPE(self,&DeeType_Type),ASSERT(DeeType_IsClass(self)), \
    ((DeeTypeObject *)REQUIRES_OBJECT(self))->tp_class)



struct instance_desc {
#ifndef CONFIG_NO_THREADS
    rwlock_t        id_lock;       /* Lock that must be held when accessing  */
#endif
    DREF DeeObject *id_vtab[1024]; /* [0..1][lock(id_lock)]
                                    * [DeeClass_DESC(:ob_type)->cd_desc->cd_imemb_size]
                                    * Instance member table. */
};

#define DeeInstance_DESC(class_descriptor,self) \
  ((struct instance_desc *)((uintptr_t)REQUIRES_OBJECT(self)+(class_descriptor)->c_addr))



/* Create a new class type derived from `base',
 * featuring traits from `descriptor'.
 * @param: base: The base of the resulting class.
 *               You may pass `Dee_None' to have the resulting
 *               class not be derived from anything (be base-less).
 * @throw: TypeError: The given `base' is neither `none', nor a type-object.
 * @throw: TypeError: The given `base' is a final or variable type. */
DFUNDEF DREF DeeTypeObject *DCALL
DeeClass_New(DeeTypeObject *__restrict base,
             DeeObject *__restrict descriptor);

/* Initialize a class member of `self' as `value'
 * @param: index: A index into `cd_members' of `self' */
DFUNDEF void DCALL
DeeClass_SetMember(DeeTypeObject *__restrict self,
                   uint16_t index,
                   DeeObject *__restrict value);


/* Return the nearest operator function for `name',
 * implemented by `self', which must be a class type.
 * If the operator doesn't exist, return NULL and throw
 * a NotImplemented error, or return NULL and don't throw
 * an error when `DeeClass_TryGetOperator()' was used. */
DFUNDEF DeeObject *DCALL
DeeClass_GetOperator(DeeTypeObject *__restrict self, uint16_t name);

/* Same as `DeeClass_GetOperator()', but don't simply return `NULL'
 * if the operator hasn't been implemented, and `ITER_DONE' when it
 * has been, but wasn't assigned anything. */
DFUNDEF DeeObject *DCALL
DeeClass_TryGetOperator(DeeTypeObject *__restrict self, uint16_t name);

/* Same as `DeeClass_TryGetOperator()', but don't return an operator
 * that has been inherited from a base-class, but return `NULL' instead. */
DFUNDEF DeeObject *DCALL
DeeClass_TryGetPrivateOperator(DeeTypeObject *__restrict self, uint16_t name);


#ifdef CONFIG_BUILDING_DEEMON

/* The functions bound to the C-level type callbacks when a
 * user-defined class provides the associated operator.
 * All of the `instance_*' functions simply call the associated
 * `instance_t*' function, which the proceeds to load (and
 * potentially cache) the operator, before invoking it. */

/* Constructor hooks when the user-class defines a `CLASS_OPERATOR_SUPERARGS' operator. */
INTDEF int DCALL instance_builtin_super_tctor(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTDEF int DCALL instance_builtin_super_ctor(DeeObject *__restrict self);
INTDEF int DCALL instance_builtin_super_tinit(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTDEF int DCALL instance_builtin_super_init(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTDEF int DCALL instance_builtin_super_tinitkw(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF int DCALL instance_builtin_super_initkw(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF int DCALL instance_super_tctor(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTDEF int DCALL instance_super_ctor(DeeObject *__restrict self);
INTDEF int DCALL instance_super_tinit(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTDEF int DCALL instance_super_init(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTDEF int DCALL instance_super_tinitkw(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF int DCALL instance_super_initkw(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);

/* Constructor hooks when the user-class doesn't define a `CLASS_OPERATOR_SUPERARGS' operator. */
INTDEF int DCALL instance_builtin_tctor(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTDEF int DCALL instance_builtin_ctor(DeeObject *__restrict self);
INTDEF int DCALL instance_builtin_tinit(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTDEF int DCALL instance_builtin_init(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTDEF int DCALL instance_builtin_tinitkw(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF int DCALL instance_builtin_initkw(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF int DCALL instance_tctor(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTDEF int DCALL instance_ctor(DeeObject *__restrict self);
INTDEF int DCALL instance_tinit(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTDEF int DCALL instance_init(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTDEF int DCALL instance_tinitkw(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF int DCALL instance_initkw(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);

/* Builtin (pre-defined) hooks that are used when the user-class doesn't override these operators. */
INTDEF int DCALL instance_builtin_tcopy(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF int DCALL instance_builtin_copy(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF int DCALL instance_builtin_tdeepload(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTDEF int DCALL instance_builtin_deepload(DeeObject *__restrict self);
INTDEF void DCALL instance_builtin_destructor(DeeObject *__restrict self); /* No t-variant, because types are unwound automatically during destruction. */
INTDEF int DCALL instance_builtin_tassign(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF int DCALL instance_builtin_assign(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF int DCALL instance_builtin_tmoveassign(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF int DCALL instance_builtin_moveassign(DeeObject *__restrict self, DeeObject *__restrict other);

/* Hooks when the user-class overrides the associated operator. */
INTDEF int DCALL instance_tcopy(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF int DCALL instance_copy(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF int DCALL instance_tdeepcopy(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF int DCALL instance_deepcopy(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF void DCALL instance_destructor(DeeObject *__restrict self); /* No t-variant, because types are unwound automatically during destruction. */
INTDEF int DCALL instance_tassign(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF int DCALL instance_assign(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF int DCALL instance_tmoveassign(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF int DCALL instance_moveassign(DeeObject *__restrict self, DeeObject *__restrict other);

INTDEF DREF DeeObject *DCALL instance_tstr(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL instance_str(DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL instance_trepr(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL instance_repr(DeeObject *__restrict self);
INTDEF int DCALL instance_tbool(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTDEF int DCALL instance_bool(DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL instance_tcall(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTDEF DREF DeeObject *DCALL instance_call(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTDEF DREF DeeObject *DCALL instance_tcallkw(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF DREF DeeObject *DCALL instance_callkw(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF DREF DeeObject *DCALL instance_tnext(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL instance_next(DeeObject *__restrict self);
INTDEF int DCALL instance_tint32(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, int32_t *__restrict result);
INTDEF int DCALL instance_int32(DeeObject *__restrict self, int32_t *__restrict result);
INTDEF int DCALL instance_tint64(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, int64_t *__restrict result);
INTDEF int DCALL instance_int64(DeeObject *__restrict self, int64_t *__restrict result);
INTDEF int DCALL instance_tdouble(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, double *__restrict result);
INTDEF int DCALL instance_double(DeeObject *__restrict self, double *__restrict result);
INTDEF DREF DeeObject *DCALL instance_tint(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL instance_int(DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL instance_tinv(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL instance_inv(DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL instance_tpos(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL instance_pos(DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL instance_tneg(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL instance_neg(DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL instance_tadd(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_add(DeeObject *__restrict self, DeeObject *__restrict other);

#endif


DECL_END

#endif /* !GUARD_DEEMON_CLASS2_H */
