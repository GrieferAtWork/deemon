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
#ifndef GUARD_DEEMON_CLASS_H
#define GUARD_DEEMON_CLASS_H 1

#include "api.h"
#include "object.h"
#ifndef CONFIG_NO_THREADS
#include "util/rwlock.h"
#endif
#include <stddef.h>
#include <stdint.h>

DECL_BEGIN

struct string_object;
typedef struct member_table DeeMemberTableObject;
typedef struct instance_method DeeInstanceMethodObject;
typedef struct property_object DeePropertyObject; /* XXX: Should be called `DeeInstancePropertyObject' */
typedef struct instancemember_object DeeInstanceMemberObject;

#define CLASS_PROPERTY_GET 0 /* Index offset for property get callbacks. */
#define CLASS_PROPERTY_DEL 1 /* Index offset for property del callbacks. */
#define CLASS_PROPERTY_SET 2 /* Index offset for property set callbacks. */

struct member_entry {
    DREF struct string_object *cme_name; /* [0..1] Name of this member.
                                          * NOTE: NULL indicates a sentinel/unused entry. */
    dhash_t                    cme_hash; /* [== cme_name->s_hash] */
    DREF struct string_object *cme_doc;  /* [0..1] A Documentation string for this member. */
    uint16_t                   cme_addr; /* Index within an `:ih_vtab' vector in which this member is located. */
#define CLASS_MEMBER_FPUBLIC   0x0000    /* Public member (default). */
#define CLASS_MEMBER_FPRIVATE  0x0001    /* FLAG: Private member - Disallow access when the calling code-frame isn't invoked using a
                                          *        this-call with an instance (or super-wrapper) of the associated class, or one of its sub-classes. */
#define CLASS_MEMBER_FVISIBILITY (CLASS_MEMBER_FPUBLIC|CLASS_MEMBER_FPRIVATE)
#define CLASS_MEMBER_FMASK     0xf001    /* Mask of known `CLASS_MEMBER_F*' flags. */
#define CLASS_MEMBER_FCLASSMEM 0x1000    /* FLAG: Used by members apart to `c_mem'.
                                          *       When set, the variable is stored in the `ih_vtab' vector
                                          *       of the class, rather than that of the instance.
                                          *       This flag is usually set for member functions.
                                          *       When used in class members this flag is ignored.
                                          *      (NOTE: If you look at the implementation you may beg to differ,
                                          *             but when a class member is accessed,
                                          *            `self == DeeClass_DESC(class_type)->c_class' upon entry,
                                          *             meaning that we only re-write `self' itself again)
                                          * NOTE: Instance-table members with this flag set can be
                                          *       accessed from as unbound class members. */
#define CLASS_MEMBER_FREADONLY 0x2000    /* FLAG: Attempting to set this member after a non-NULL
                                          *       value has already been assigned is an error.
                                          *       Since all class/instance fields start out as
                                          *       `NULL' (aka. unbound), we must still allow
                                          *       writing values to unbound fields.
                                          *       With that in mind, `FWRITEONCE' would be a better name.
                                          * NOTE: Despite the possibility of write-once synchronization
                                          *       being possible for this type of member, the fact that
                                          *       user-code is allowed to directly access class/instance
                                          *       members by their index (as those are already known at
                                          *       compile-time for same-module classes), it is quite easy
                                          *       to write assembly that can simply bypass this flag and
                                          *       overwrite or delete a readonly field.
                                          *       In other words: You must still acquire locks normally
                                          *       whenever accessing a readonly field.
                                          * NOTE: When this flag is used alongside `CLASS_MEMBER_FPROPERTY',
                                          *       the behavior is the same as though `:ih_vtab[cme_addr + CLASS_PROPERTY_GET]'
                                          *       and `:ih_vtab[cme_addr + CLASS_PROPERTY_SET]' were set to `NULL'.
                                          * HINT: The compiler will automatically set this flag for member function in
                                          *       order to protect them from accidentally being overwritten once assigned.
                                          * HINT: Since DEL/SET property callbacks are ignored for this flag,
                                          *       you can also use to to generate optimized read-only properties
                                          *       that only require a single slot in the instance table.
                                          *       In relation to this, it is guarantied that `cme_addr + CLASS_PROPERTY_DEL'
                                          *       and `cme_addr + CLASS_PROPERTY_SET' are never so much as looked at when
                                          *       this flag is set, meaning that only `cme_addr + CLASS_PROPERTY_GET' will
                                          *       actually be used. */
#define CLASS_MEMBER_FPROPERTY 0x4000    /* FLAG: This member behaves as a special property member:
                                          *       get: `:ih_vtab[cme_addr + CLASS_PROPERTY_GET]' (Invoke with `tp_call')
                                          *       del: `:ih_vtab[cme_addr + CLASS_PROPERTY_DEL]' (Invoke with `tp_call')
                                          *       set: `:ih_vtab[cme_addr + CLASS_PROPERTY_SET]' (Invoke with `tp_call')
                                          * NOTE: When the `CLASS_MEMBER_FMETHOD' flag is also set, the operator is invoked as a this-call.
                                          *       Otherwise, it is invoked directly (aka. without the associated `this_arg' being passed)
                                          * HINT: Since property callbacks (just as any other member function)
                                          *       are stored as part of the runtime field vector, you can simply
                                          *       leave any unimplemented callback set to `NULL' at runtime.
                                          *       Attempting to access it will then cause an `Error.AttributeError' to be thrown,
                                          *       indicating that the specified field cannot be accessed in the manner requested.
                                          * NOTE: Property callbacks can _only_ be assigned using index-based member assignment,
                                          *       meaning that custom assembly _must_ be generated for user-code to make use of them. */
#define CLASS_MEMBER_FMETHOD   0x8000    /* FLAG: When accessed, the object found in the instance vector
                                          *       is wrapped as a `DeeInstanceMethodObject' object that
                                          *       is used to implement a this-call to a member function.
                                          * HINT: This flag is usually combined with the `CLASS_MEMBER_FCLASSMEM' flag. */
    uint16_t                   cme_flag; /* Class member flags (Set of `CLASS_MEMBER_F*') */
#if __SIZEOF_POINTER__ >= 8
    uint32_t                   cme_padding; /* ... */
#endif
};
struct member_table {
    /* HINT: Member table objects are generated by the compiler and
     *       stored in the constant object vector of code objects. */
    OBJECT_HEAD
    size_t               mt_size; /* [const] Amount of instance variables. (NOTE: doesn't necessarily match the number of members)
                                   * TODO: This should really be a `uint16_t', and no code using
                                   *       this field even accesses any bits beyond that! */
    size_t               mt_mask; /* [const] Mask applied when accessing the dict-style `mt_list' table.
                                   * TODO: This should also be a `uint16_t' */
    struct member_entry *mt_list; /* [1..mt_mask+1][const][owned_if(!= empty_class_members)][const]
                                   * Hash-vector of member descriptors.
                                   * TODO: This vector should be inlined! */
};

#define DeeMemberTable_HASHST(self,hash)  ((hash) & ((DeeMemberTableObject *)REQUIRES_OBJECT(self))->mt_mask)
#define DeeMemberTable_HASHNX(hs,perturb) (((hs) << 2) + (hs) + (perturb) + 1)
#define DeeMemberTable_HASHPT(perturb)    ((perturb) >>= 5) /* This `5' is tunable. */
#define DeeMemberTable_HASHIT(self,i)     (((DeeMemberTableObject *)REQUIRES_OBJECT(self))->mt_list+((i) & ((DeeMemberTableObject *)(self))->mt_mask))


#ifdef GUARD_DEEMON_OBJECTS_CLASS_C
DDATDEF DeeMemberTableObject DeeMemberTable_Empty;
#else
DDATDEF DeeObject DeeMemberTable_Empty;
#endif
DDATDEF DeeTypeObject DeeMemberTable_Type;
#define DeeMemberTable_Check(ob)      DeeObject_InstanceOf(ob,&DeeMemberTable_Type) /* TODO: `DeeMemberTable_Type' should be variable-size+final */
#define DeeMemberTable_CheckExact(ob) DeeObject_InstanceOfExact(ob,&DeeMemberTable_Type)



struct instance_method {
    OBJECT_HEAD
    DREF DeeObject *im_func; /* [1..1] The function to-be called. */
    DREF DeeObject *im_this; /* [1..1] The this argument. */
};

DDATDEF DeeTypeObject DeeInstanceMethod_Type;

/* Create a new instance method.
 * This is a simple wrapper object that simply invokes a thiscall on
 * `im_func', using `this_arg' as the this-argument when called normally.
 * In user-code, it is used to implement the temporary/split type when
 * an instance attribute with the `CLASS_MEMBER_FMETHOD' flag is loaded
 * as an object, rather than being called directly. */
DFUNDEF DREF DeeObject *DCALL
DeeInstanceMethod_New(DeeObject *__restrict func,
                      DeeObject *__restrict this_arg);


struct property_object {
    /* A wrapper object describing an instance
     * property when accessed through the class:
     * >> class my_class {
     * >>     private foo_value = 42;
     * >>     
     * >>     foo = {
     * >>         get() {
     * >>             print "In getter";
     * >>             return foo_value;
     * >>         }
     * >>         set(v) {
     * >>             print "In setter";
     * >>             foo_value = v;
     * >>         }
     * >>     }
     * >> }
     * >>
     * >> local prop = my_class.foo; // This is a `struct property_object'
     * >> print repr prop;         // `property { get = @thiscall function(), set = @thiscall function(v) }'
     * >> local inst = my_class();
     * >> print inst.foo;          // `In getter' `42'
     * >> print prop.get(inst);    // `In getter' `42'
     * Note that property wrappers are always unbound and not actually
     * used when accessing instance members through normal means.
     * They are merely used as syntactical sugar to allow access to
     * public instance properties when only given the class that
     * implements that property.
     */
    OBJECT_HEAD
    DREF DeeObject *p_get; /* [0..1][const] Getter callback. */
    DREF DeeObject *p_del; /* [0..1][const] Delete callback. */
    DREF DeeObject *p_set; /* [0..1][const] Setter callback. */
};

DDATDEF DeeTypeObject DeeProperty_Type;





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


#define CLASS_HEADER_OPC1    8
#define CLASS_HEADER_OPC2  ((CLASS_OPERATOR_USERCOUNT+7)/8)

#if (CLASS_HEADER_OPC1*CLASS_HEADER_OPC2) < CLASS_OPERATOR_USERCOUNT
#error "FIXME: Not enough space for all available operators"
#endif

struct class_optable {
    /* [0..1][lock(:c_class.ih_lock)][*] Table of operators. */
    DREF DeeObject *co_operators[CLASS_HEADER_OPC2];
};

struct instance_desc {
#ifndef CONFIG_NO_THREADS
    rwlock_t        ih_lock;       /* Lock that must be held when accessing  */
#endif
    DREF DeeObject *ih_vtab[1024]; /* [0..1][lock(ih_lock)][:?->mt_size] Vector of member variables. */
};
#ifndef CONFIG_NO_THREADS
#define INSTANCE_DESC_TYPE(num_members) \
 struct { rwlock_t _lock; DREF DeeObject *_vtab[num_members]; }
#define INSTANCE_DESC_INIT { RWLOCK_INIT, }
#else
#define INSTANCE_DESC_TYPE(num_members) \
                 struct { DREF DeeObject *_vtab[num_members]; }
#define INSTANCE_DESC_INIT { }
#endif


#ifndef CONFIG_NO_THREADS
#define INSTANCE_DESC_READ(x)     rwlock_read(&(x)->ih_lock)
#define INSTANCE_DESC_WRITE(x)    rwlock_write(&(x)->ih_lock)
#define INSTANCE_DESC_ENDREAD(x)  rwlock_endread(&(x)->ih_lock)
#define INSTANCE_DESC_ENDWRITE(x) rwlock_endwrite(&(x)->ih_lock)
#else
#define INSTANCE_DESC_READ(x)     (void)0
#define INSTANCE_DESC_WRITE(x)    (void)0
#define INSTANCE_DESC_ENDREAD(x)  (void)0
#define INSTANCE_DESC_ENDWRITE(x) (void)0
#endif



struct class_desc {
    /* NOTE: This structure is destroyed from the destructor
     *       of `DeeType_Type' when pointed to by `tp_class'. */
    uintptr_t                  c_addr;  /* [const] Offset to an inline-allocated `struct instance_desc'
                                         *         that can be found in every instance of this class.
                                         *   HINT: This field us usually equal to `:tp_base->tp_init.tp_alloc.tp_instance_size',
                                         *         meaning that the instance descriptor is allocated after the underlying object.
                                         *         This way, instance fields can be stacked ontop of each other, allowing for
                                         *         one class to be derived from another, and so on... */
    DREF DeeMemberTableObject *c_mem;   /* [1..1][const] Class instance member table descriptor. */
    DREF DeeMemberTableObject *c_cmem;  /* [1..1][const] Class member table descriptor. (For class functions, etc.) */
    struct class_optable      *c_ops[CLASS_HEADER_OPC1]; /* [0..1][owned][lock(c_class.ih_lock)][*] Table of operators. */
    DREF DeeObject           **c_exopv; /* [0..1][lock(c_class.ih_lock)][0..:ob_type->tp_exops][owned]
                                         * Vector of extended operator callbacks, used by type-type
                                         * extensions, such as `&DeeFileType_Type'.
                                         * NOTE: This vector is lazily allocated on first use. */
    struct instance_desc       c_class; /* Instance descriptor for storing members described by `c_cmem'. */
};

#define DeeClass_Check(self)    (DeeType_Check(self) && DeeType_IsClass(self))
#define DeeInstance_Check(self)  DeeClass_Check(Dee_TYPE(self))


/* Create a new class type derived from `base'
 * and containing the given instance/class members.
 * The caller can then start assigning operators
 * using the `DeeClass_SetOperator()' function.
 * HINT: You may pass `NULL' for base to create a new super-type as a class.
 * @param: type:             The Type-type that should be used to create the class.
 *                           Common values are `DeeType_Type' or `DeeFile_Type'
 *                           This options selects the set of type-specific
 *                           operators that can be implemented by the class.
 *                     HINT: When `NULL' is passed, `Dee_TYPE(base)' or `&DeeType_Type' is used instead.
 * @param: base:             The base that the class should be derived from.
 *                           A Common value is `DeeObject_Type'.
 * @param: name:             The name of the class (or `NULL')
 * @param: doc:              A documentation string describing the class (or `NULL')
 * @param: instance_members: A `DeeMemberTableObject' describing members found in instances of this class.
 *                           When `NULL' is passed, `DeeMemberTable_Empty' is used instead.
 * @param: class_members:    A `DeeMemberTableObject' describing members found in the class itself.
 *                           When `NULL' is passed, `DeeMemberTable_Empty' is used instead.
 * @param: flags:            Set of `TP_F*' describing special flags to-be applied to the new type.
 *                     NOTE: All flags not apart of the following mask `0xf' are ignored. */
DFUNDEF DREF DeeTypeObject *DCALL
DeeClass_New(DeeTypeObject *type,
             DeeTypeObject *base,
             /*StringObject*/DeeObject *name,
             /*StringObject*/DeeObject *doc,
             /*MemberTable*/DeeObject *instance_members,
             /*MemberTable*/DeeObject *class_members,
             uint16_t flags);

/* Assign a `callable' object (should implement `tp_call')
 * to an operator of the given class type `self'.
 * Note that any operator can only ever be assigned _once_,
 * following the WRITE_ONCE locking principle.
 * If the operator has already been assigned, an `Error.ValueError' is thrown.
 * @param: name: One of `OPERATOR_*' or `CLASS_OPERATOR_*'
 * @throws: Error.RuntimeError.NotImplemented: The given operator code is not known or supported.
 * NOTE: All operators (except for `CLASS_OPERATOR_SUPERARGS')
 *       are invoked using the this-call calling convention. */
DFUNDEF int DCALL
DeeClass_SetOperator(DeeTypeObject *__restrict self,
                     unsigned int name,
                     DeeObject *__restrict callback);

/* Set a class member.
 * Usually used to assign methods after a class has been created.
 * NOTE: The caller is responsible to ensure that `index' is in bounds. */
DFUNDEF void DCALL
DeeClass_SetMember(DeeTypeObject *__restrict self,
                   unsigned int index,
                   DeeObject *__restrict value);


/* High-level access to instance member variables.
 * NOTE: The caller is responsible to ensure that `index' is in-bounds,
 *       when `DeeObject_InstanceOf(self,tp_self)' checks out
 *      (which is checked by these functions), as well as
 *       `DeeClass_Check(tp_self)' being true.
 * NOTE: In the event of the member not being assigned, 
 *      `DeeInstance_GetMember()' and `DeeInstance_DelMember()'
 *       will throw an `Error.AttributeError'. */
DFUNDEF DREF DeeObject *DCALL
DeeInstance_GetMember(/*Class*/DeeTypeObject *__restrict tp_self,
                      /*Instance*/DeeObject *__restrict self,
                      unsigned int index);
DFUNDEF bool DCALL
DeeInstance_BoundMember(/*Class*/DeeTypeObject *__restrict tp_self,
                        /*Instance*/DeeObject *__restrict self,
                        unsigned int index);
DFUNDEF int DCALL
DeeInstance_DelMember(/*Class*/DeeTypeObject *__restrict tp_self,
                      /*Instance*/DeeObject *__restrict self,
                      unsigned int index);
DFUNDEF void DCALL
DeeInstance_SetMember(/*Class*/DeeTypeObject *__restrict tp_self,
                      /*Instance*/DeeObject *__restrict self,
                      unsigned int index, DeeObject *__restrict value);

#ifdef CONFIG_BUILDING_DEEMON
INTDEF DREF DeeObject *DCALL
DeeInstance_GetMemberSafe(DeeTypeObject *__restrict tp_self,
                          DeeObject *__restrict self,
                          unsigned int index);
INTDEF int DCALL
DeeInstance_BoundMemberSafe(DeeTypeObject *__restrict tp_self,
                            DeeObject *__restrict self,
                            unsigned int index);
INTDEF int DCALL
DeeInstance_DelMemberSafe(DeeTypeObject *__restrict tp_self,
                          DeeObject *__restrict self,
                          unsigned int index);
INTDEF int DCALL
DeeInstance_SetMemberSafe(DeeTypeObject *__restrict tp_self,
                          DeeObject *__restrict self,
                          unsigned int index, DeeObject *__restrict value);
#endif


/* >> struct class_desc *DeeClass_DESC(DeeTypeObject *self);
 * >> struct instance_desc *DeeInstance_DESC(struct class_desc *cdesc, DeeObject *self);
 * Returns pointers to special data blocks used for implementing
 * dynamic user-defined class types for any kind of type. */
#if 1
#define DeeClass_DESC(self) \
 (ASSERT(DeeType_Check(self)),ASSERT(DeeClass_Check(self)),((DeeTypeObject *)REQUIRES_OBJECT(self))->tp_class)
#else
/* This version is slower and doesn't work for custom type classes being destroyed:
 * See. During object destruction, the object type is constantly altered to reflect
 *      the changes in the object's valid component range. However a class is a
 *      top-level extension construct of a type, yet is destroyed last.
 *      Therefor, during destruction, the instance-size of the underlying type
 *      will no longer reflect the class's actual descriptor offset, meaning we
 *      can't rely on the instance-size of the class's type to get that offset. */
#define DeeClass_DESC(self) \
 (ASSERT(DeeType_Check(self)),ASSERT(DeeClass_Check(self)), \
  ((struct class_desc *)REQUIRES_OBJECT((uintptr_t)(self)+Dee_TYPE(self)->tp_init.tp_alloc.tp_instance_size)))
#endif
#define DeeInstance_DESC(cdesc,self) \
  ((struct instance_desc *)((uintptr_t)REQUIRES_OBJECT(self)+(cdesc)->c_addr))

#ifdef CONFIG_BUILDING_DEEMON
/* Return a pointer to the class-member of a given class descriptor.
 * NOTE: Returns `NULL' if the member doesn't exist.
 * HINT: No lock must be held when this function is called, but a
 *       read/write-lock must be held respectively when accessing
 *       the returned pointer. */
#define membertable_lookup(self,attr,hash) \
        membertable_lookup_string(self,DeeString_STR(attr),hash)
INTDEF struct member_entry *DCALL
membertable_lookup_string(DeeMemberTableObject *__restrict self,
                          char const *__restrict attr, dhash_t hash);
INTDEF dssize_t DCALL
membertable_enum(DeeTypeObject *__restrict tp_self, DeeObject *ob_self,
                 DeeMemberTableObject *__restrict self, denum_t proc, void *arg);
INTDEF dssize_t DCALL
membertable_enum_class(DeeTypeObject *__restrict tp_self,
                       DeeMemberTableObject *__restrict self,
                       denum_t proc, void *arg);
struct attribute_info;
struct attribute_lookup_rules;
INTDEF int DCALL
membertable_find_class(DeeTypeObject *__restrict tp_self,
                       DeeMemberTableObject *__restrict self,
                       struct attribute_info *__restrict result,
                       struct attribute_lookup_rules const *__restrict rules);
INTDEF int DCALL
membertable_find(DeeTypeObject *__restrict tp_self, DeeObject *ob_self,
                 DeeMemberTableObject *__restrict self,
                 struct attribute_info *__restrict result,
                 struct attribute_lookup_rules const *__restrict rules);

/* Check if the current execution context allows access to `member',
 * which is either an instance or class member of `class_type' */
INTDEF bool DCALL
member_mayaccess(DeeTypeObject *__restrict class_type,
                 struct member_entry *__restrict member);

/* Get/Call/Del/Set access to a given member. */
INTDEF DREF DeeObject *DCALL member_get(DeeTypeObject *__restrict class_type, struct instance_desc *__restrict self, DeeObject *__restrict this_arg, struct member_entry *__restrict member);
INTDEF int DCALL member_bound(DeeTypeObject *__restrict class_type, struct instance_desc *__restrict self, DeeObject *__restrict this_arg, struct member_entry *__restrict member);
INTDEF DREF DeeObject *DCALL member_call(DeeTypeObject *__restrict class_type, struct instance_desc *__restrict self, DeeObject *__restrict this_arg, struct member_entry *__restrict member, size_t argc, DeeObject **__restrict argv);
INTDEF DREF DeeObject *DCALL member_call_kw(DeeTypeObject *__restrict class_type, struct instance_desc *__restrict self, DeeObject *__restrict this_arg, struct member_entry *__restrict member, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF int DCALL member_del(DeeTypeObject *__restrict class_type, struct instance_desc *__restrict self, DeeObject *__restrict this_arg, struct member_entry *__restrict member);
INTDEF int DCALL member_set(DeeTypeObject *__restrict class_type, struct instance_desc *__restrict self, DeeObject *__restrict this_arg, struct member_entry *__restrict member, DeeObject *__restrict value);

/* Same as the functions above, but used to access members apart of the
 * instance-member table, when operating on the class type itself.
 * This can be used to access a member functions as unbound objects,
 * or to create property wrappers, etc:
 * >> class my_class: object {
 * >>     foo() {
 * >>         print "In foo:",this;
 * >>     }
 * >> };
 * >> 
 * >> my_class().foo();        // `In foo: my_class'
 * >> my_class.foo(42);        // `In foo: 42'
 * >> print repr my_class.foo; // `@thiscall function foo()'
 */
INTDEF DREF DeeObject *DCALL class_member_get(DeeTypeObject *__restrict class_type, struct instance_desc *__restrict self, struct member_entry *__restrict member);
INTDEF DREF DeeObject *DCALL class_member_call(DeeTypeObject *__restrict class_type, struct instance_desc *__restrict self, struct member_entry *__restrict member, size_t argc, DeeObject **__restrict argv);
INTDEF DREF DeeObject *DCALL class_member_call_kw(DeeTypeObject *__restrict class_type, struct instance_desc *__restrict self, struct member_entry *__restrict member, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF int DCALL class_member_del(DeeTypeObject *__restrict class_type, struct instance_desc *__restrict self, struct member_entry *__restrict member);
INTDEF int DCALL class_member_set(DeeTypeObject *__restrict class_type, struct instance_desc *__restrict self, struct member_entry *__restrict member, DeeObject *__restrict value);
#endif /* CONFIG_BUILDING_DEEMON */


struct instancemember_object {
    OBJECT_HEAD
    DREF DeeTypeObject  *im_type;   /* [1..1][const] The user-class type, instances of which implement this member. */
    struct member_entry *im_member; /* [1..1][const] The instance-member (`CLASS_MEMBER_FCLASSMEM' isn't set) that should be accessed. */
    struct class_desc   *im_desc;   /* [1..1][const][== DeeClass_DESC(im_type)] The class descriptor for `im_type' */
};

DDATDEF DeeTypeObject DeeInstanceMember_Type;

#ifdef CONFIG_BUILDING_DEEMON
/* Create a property-style wrapper to access a specific
 * instance member through a descriptor created from the class:
 * >> class MyClass {
 * >>     public my_member = 42;
 * >> }
 * >> 
 * >> local inst = MyClass();
 * >> print MyClass.my_member.get(inst); // 42
 * >> print MyClass.my_member(inst);     // 42 (`operator ()' is implemented as an alias for member function `get', just like it is for other property-stype types)
 * >> print MyClass.my_member;           // `instancemember'
 * >> print inst.my_member;              // 42
 * >> 
 * >> inst.my_member = 10;
 * >> print MyClass.my_member.get(inst); // 10
 * >> 
 * >> MyClass.my_member.set(inst,20);
 * >> print inst.my_member;              // 20
 * >> 
 * Note that such a wrapper will allow access to members
 * that would otherwise be private, meaning that the user
 * is responsible not to grant access to such a wrapper to
 * anyone.
 * The first argument passed to this member functions and `operator ()'
 * of the returned object is checked to be an instance of `class_type'.
 * The wrapper generated by this is the equivalent of `classmember' for
 * c-style classes. */
INTDEF DREF DeeObject *DCALL
instancemember_wrapper(DeeTypeObject *__restrict class_type,
                       struct member_entry *__restrict member);
#endif /* CONFIG_BUILDING_DEEMON */


#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define DeeClass_SetOperator(self,name,callback)            __builtin_expect(DeeClass_SetOperator(self,name,callback),0)
#define DeeInstance_DelMember(tp_self,self,index)           __builtin_expect(DeeInstance_DelMember(tp_self,self,index),0)
#define DeeInstance_DelMemberSafe(tp_self,self,index)       __builtin_expect(DeeInstance_DelMemberSafe(tp_self,self,index),0)
#define DeeInstance_SetMemberSafe(tp_self,self,index,value) __builtin_expect(DeeInstance_SetMemberSafe(tp_self,self,index,value),0)
#endif
#endif


#ifdef CONFIG_BUILDING_DEEMON
/* Return the nearest operator function for `name',
 * implemented by `self', which must be a class type.
 * If the operator doesn't exist, return NULL and throw
 * a NotImplemented error, or return NULL and don't throw
 * an error when `DeeClass_TryGetOperator()' was used. */
INTDEF DREF DeeObject *DCALL DeeClass_GetOperator(DeeTypeObject *__restrict self, uint16_t name);
INTDEF DREF DeeObject *DCALL DeeClass_TryGetOperator(DeeTypeObject *__restrict self, uint16_t name);
/* Same as `DeeClass_TryGetOperator()', but only return a operator
 * if that operator was implemented by the given class itself, rather
 * that the given class, or one of its base-classes. */
INTDEF DREF DeeObject *DCALL DeeClass_TryGetPrivateOperator(DeeTypeObject *__restrict self, uint16_t name);
#ifdef CONFIG_TYPE_ALLOW_OPERATOR_CACHE_INHERITANCE
/* Similar to `DeeClass_SetOperator()', however used to lazily inherit operators
 * from base-types, caching them for faster retrieval when used the next time. */
INTDEF void DCALL DeeClass_InheritOperator(DeeTypeObject *__restrict self, uint16_t name, DeeObject *__restrict value);
#endif
#endif

DECL_END

#endif /* !GUARD_DEEMON_CLASS_H */
