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
#include "util/rwlock.h"
#include <stddef.h>
#include <stdint.h>

/*
 * Automatically generated operators / constructors:
 * NOTE: For this purpose, code on right-hand-side does not contain any hidden super-calls,
 *       which is something that is impossible to achieve in user-code.
 *
 * CONSTRUCTORS:
 * >> class MyClass: Base {                >> class MyClass: Base {
 * >> }                                    >>     this(): super() { }
 *                                         >> }
 * 
 * >> class MyClass: Base {                >> class MyClass: Base {
 * >>     this = super;                    >>     this(args...): super(args...) { }
 * >> }                                    >> }
 * 
 * >> class MyClass: Base {                >> class MyClass: Base {
 * >>     this = del;                      >>     this(args...) {
 * >> }                                    >>         import Error from deemon;
 *                                         >>         throw Error.RuntimeError.NotImplemented("...");
 *                                         >>     }
 * 
 * >> class MyClass: Base {                >> class MyClass: Base {
 * >>     member foo = 42;                 >>     member foo;
 * >> }                                    >>     this(): super() {
 *                                         >>         foo = 42;
 *                                         >>     }
 *                                         >>     ~this() {
 *                                         >>         del foo;
 *                                         >>     }
 *                                         >> }
 *
 * >> class MyClass: Base {                >> class MyClass: Base {
 * >>     member foo = 42;                 >>     member foo;
 * >>     this = super;                    >>     this(args...): super(args...) {
 * >> }                                    >>         foo = 42;
 *                                         >>     }
 *                                         >>     ~this() {
 *                                         >>         del foo;
 *                                         >>     }
 *                                         >> }
 *
 * >> class MyClass: Base {                >> class MyClass: Base {
 * >>     member foo = 42;                 >>     member foo;
 * >>     this() {                         >>     this(): super() {
 * >>         print "foo =",foo;           >>         foo = 42;
 * >>     }                                >>         {
 * >> }                                    >>             print "foo =",foo;
 *                                         >>         }
 *                                         >>     }
 *                                         >>     ~this() {
 *                                         >>         del foo;
 *                                         >>     }
 *                                         >> }
 *
 * OPERATORS:
 * >> class MyClass: Base {                >> class MyClass: Base {
 * >>     member foo;                      >>     member foo;   
 * >>     member bar;                      >>     member bar;   
 * >>     member foobar;                   >>     member foobar;
 * >> }                                    >>     this(): super() { }
 * Automatic operators are provided in     >>     ~this() {
 * groups, where the user explicitly       >>         del foo;   
 * implementing any of them will result    >>         del bar;   
 * in all other operators from that        >>         del foobar;
 * group to no longer be automatically     >>     }
 * generated.                              >>     operator copy(other) {
 * For this purpose, the following groups  >>         // super.copy(other);
 * exist:                                  >>         if (other !is MyClass)
 *  - copy                                 >>             throw Error.TypeError("...");
 *  - constructor                          >>         foo    = (other as MyClass).foo;
 *  - destructor                           >>         bar    = (other as MyClass).bar;
 *  - assign, moveassign                   >>         foobar = (other as MyClass).foobar;
 *  - hash, eq, ne, lo, le, gr, ge         >>     }
 * Also note that when providing a `copy'  >>     operator deepcopy(other) {
 * operator, is will also be invoke when   >>         // super.deepcopy(other);
 * `deepcopy' is called, after with each   >>         if (other !is MyClass)
 * of the instance's bound members will be >>             throw Error.TypeError("...");
 * replace with a deep copy of itself.     >>         foo    = deepcopy((other as MyClass).foo);
 *                                         >>         bar    = deepcopy((other as MyClass).bar);
 * Note that attribute accesses made on    >>         foobar = deepcopy((other as MyClass).foobar);
 * `other' in automatic operators will not >>     }
 * invoke a potential `operator getattr',  >>     operator hash() {
 * but will always access the native       >>         return foo.operator hash() ^
 * attribute.                              >>                bar.operator hash() ^
 *                                         >>                foobar.operator hash();
 * During member access, only `member'-    >>     }
 * like fields are accessed, meaning that  >>     operator == (other) {
 * instance-properties will not be invoked >>         if (other !is MyClass)
 *                                         >>             return false;
 * Fields are compared lexicographically,  >>         return foo == (other as MyClass).foo &&
 * following the same order in which they  >>                bar == (other as MyClass).bar &&
 * were orignally defined.                 >>                foobar == (other as MyClass).foobar;
 *                                         >>     }
 *                                         >>     operator != (other) {
 *                                         >>         if (other !is MyClass)
 *                                         >>             return true;
 *                                         >>         return foo != (other as MyClass).foo &&
 *                                         >>                bar != (other as MyClass).bar &&
 *                                         >>                foobar != (other as MyClass).foobar;
 *                                         >>     }
 *                                         >>     operator < (other) {
 *                                         >>         import Error from deemon;
 *                                         >>         if (other !is MyClass)
 *                                         >>             throw Error.TypeError("...");
 *                                         >>         return foo < (other as MyClass).foo || (foo == (other as MyClass).foo &&
 *                                         >>                bar < (other as MyClass).bar || (bar == (other as MyClass).bar &&
 *                                         >>                foobar < (other as MyClass).foobar));
 *                                         >>     }
 *                                         >>     operator <= (other) {
 *                                         >>         import Error from deemon;
 *                                         >>         if (other !is MyClass)
 *                                         >>             throw Error.TypeError("...");
 *                                         >>         return foo < (other as MyClass).foo || (foo == (other as MyClass).foo &&
 *                                         >>                bar < (other as MyClass).bar || (bar == (other as MyClass).bar &&
 *                                         >>                foobar <= (other as MyClass).foobar));
 *                                         >>     }
 *                                         >>     operator > (other) {
 *                                         >>         import Error from deemon;
 *                                         >>         if (other !is MyClass)
 *                                         >>             throw Error.TypeError("...");
 *                                         >>         return foo > (other as MyClass).foo || (foo == (other as MyClass).foo &&
 *                                         >>                bar > (other as MyClass).bar || (bar == (other as MyClass).bar &&
 *                                         >>                foobar > (other as MyClass).foobar));
 *                                         >>     }
 *                                         >>     operator >= (other) {
 *                                         >>         import Error from deemon;
 *                                         >>         if (other !is MyClass)
 *                                         >>             throw Error.TypeError("...");
 *                                         >>         return foo > (other as MyClass).foo || (foo == (other as MyClass).foo &&
 *                                         >>                bar > (other as MyClass).bar || (bar == (other as MyClass).bar &&
 *                                         >>                foobar >= (other as MyClass).foobar));
 *                                         >>     }
 *                                         >> }
 *
 * NOTE: Please don't define destructors like those seen above.
 *       The runtime will automatically unbind all instance members
 *       during destruction of a class instance, and it can do so
 *       must more efficiently that your destructor ever could.
 */



/* DEPRECATED NAMES */
#define member_mayaccess(class_type,member) class_attribute_mayaccess(member,class_type)

DECL_BEGIN

typedef struct class_descriptor_object DeeClassDescriptorObject;
struct string_object;

#define CLASS_GETSET_GET 0 /* Offset to the getter of a user-defined getset in a class. */
#define CLASS_GETSET_DEL 1 /* Offset to the delete of a user-defined getset in a class. */
#define CLASS_GETSET_SET 2 /* Offset to the setter of a user-defined getset in a class. */

#define CLASS_ATTRIBUTE_FNORMAL   0x0000 /* Normal class attribute flags. */
#define CLASS_ATTRIBUTE_FPUBLIC   0x0000 /* The attribute is publicly available. */
#define CLASS_ATTRIBUTE_FPRIVATE  0x0001 /* The attribute is only accessible from this-call functions with an instance of the class as this-argument. */
#define CLASS_ATTRIBUTE_FVISIBILITY (CLASS_ATTRIBUTE_FPRIVATE) /* Mask of flags affecting symbol visibility. */
#define CLASS_ATTRIBUTE_FFINAL    0x0002 /* The attribute is accessed directly, and cannot be overwritten by sub-classes. */
#define CLASS_ATTRIBUTE_FREADONLY 0x0004 /* The attribute can only ever be when not already bound (and it cannot be unbound). */
/*      CLASS_ATTRIBUTE_F         0x0008  * ... */
#define CLASS_ATTRIBUTE_FMETHOD   0x0010 /* When accessed as get in `foo.bar', return an `instancemethod(MEMBER_TABLE[ca_addr],foo)' (calling `foo.bar()' will similarly perform a this-call). */
#define CLASS_ATTRIBUTE_FGETSET   0x0020 /* Access to the attribute is done via get/set, with the callbacks being `CLASS_GETSET_*' offsets from `ca_addr'.
                                          * When `CLASS_ATTRIBUTE_FMETHOD' is set, callbacks are invoked as this-calls.
                                          * When `CLASS_ATTRIBUTE_FREADONLY' is set, only `CLASS_GETSET_GET' is ever accessed,
                                          * and all other callbacks behave as though they were unbound. */
/*      CLASS_ATTRIBUTE_F         0x0040  * ... */
#define CLASS_ATTRIBUTE_FCLASSMEM 0x0080 /* An instance-attribute is stored in class memory (usually set for instance member functions).
                                          * NOTE: Ignored when used by attributes in `cd_cattr_list', where
                                          *       operation is always done like it would be when it was set. */
/*      CLASS_ATTRIBUTE_F         0x0100  * ... */
/*      CLASS_ATTRIBUTE_F         0x8000  * ... */
#define CLASS_ATTRIBUTE_FMASK     0x00b7 /* Mask of known flag bits. */

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

#ifdef CONFIG_BUILDING_DEEMON
/* Check if the current execution context allows access to `member',
 * which is either an instance or class member of `class_type' */
INTDEF WUNUSED bool DCALL
class_attribute_mayaccess(struct class_attribute *__restrict self,
                          DeeTypeObject *__restrict impl_class);
#endif


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
                       *       by simply declaring it, but not assigning a callback. */
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
    uint16_t                   cd_flags;         /* [const] Additional flags to set for the resulting type (set of `TP_F*').
                                                  * NOTE: The `TP_FINHERITCTOR' flag has special meaning here,
                                                  *       in that its presence causes `CLASS_OPERATOR_SUPERARGS'
                                                  *       to be implemented such that it forwards all arguments
                                                  *       to the underlying base-type, while also implementing
                                                  *      `OPERATOR_CONSTRUCTOR' as a no-op for any number of
                                                  *       arguments.
                                                  *       If the user overrides `CLASS_OPERATOR_SUPERARGS',
                                                  *       the `TP_FINHERITCTOR' flag is simply ignored.
                                                  *       If the user overrides `OPERATOR_CONSTRUCTOR',
                                                  *       the user's constructor will be invoked, though
                                                  *       no arguments will be passed to it.
                                                  */
    uint16_t                   cd_cmemb_size;    /* [const] The allocation size of the class member table. */
    uint16_t                   cd_imemb_size;    /* [const] The allocation size of the instance member table. */
    uint16_t                   cd_clsop_mask;    /* [const] Mask for the `cd_clsop_list' hash-vector. */
    size_t                     cd_cattr_mask;    /* [const] Mask for the `cd_cattr_list' hash-vector. */
    size_t                     cd_iattr_mask;    /* [const] Mask for the `cd_cattr_list' hash-vector. */
    struct class_operator     *cd_clsop_list;    /* [1..cd_clsop_mask+1][owned_if(!= INTERNAL(empty-class-operator-table))]
                                                  * [const] The class operator address hash-vector. */
    struct class_attribute    *cd_cattr_list;    /* [1..cd_cattr_mask+1][owned_if(!= INTERNAL(empty-class-attribute-table))]
                                                  * [const] The class attribute hash-vector. */
    struct class_attribute     cd_iattr_list[1]; /* [cd_cattr_mask+1] The instance attribute hash-vector. */
};
#define DeeClassDescriptor_CLSOPNEXT(i,perturb) ((i) = (((i) << 2) + (i) + (perturb) + 1),(perturb) >>= 5)
#define DeeClassDescriptor_CATTRNEXT(i,perturb) ((i) = (((i) << 2) + (i) + (perturb) + 1),(perturb) >>= 5)
#define DeeClassDescriptor_IATTRNEXT(i,perturb) ((i) = (((i) << 2) + (i) + (perturb) + 1),(perturb) >>= 5)


DDATDEF DeeTypeObject DeeClassDescriptor_Type;
#define DeeClassDescriptor_Check(x)      DeeObject_InstanceOfExact(x,&DeeClassDescriptor_Type) /* DeeClassDescriptor_Type is final */
#define DeeClassDescriptor_CheckExact(x) DeeObject_InstanceOfExact(x,&DeeClassDescriptor_Type)


/* Lookup class / instance attributes within the given class descriptor.
 * @return: * :   A pointer to attribute that was found.
 * @return: NULL: Attribute could not be found (no error is thrown) */
DFUNDEF struct class_attribute *(DCALL DeeClassDescriptor_QueryClassAttributeWithHash)(DeeClassDescriptorObject *__restrict self, /*String*/DeeObject *__restrict name, dhash_t hash);
DFUNDEF struct class_attribute *(DCALL DeeClassDescriptor_QueryClassAttributeStringWithHash)(DeeClassDescriptorObject *__restrict self, char const *__restrict name, dhash_t hash);
DFUNDEF struct class_attribute *(DCALL DeeClassDescriptor_QueryInstanceAttributeWithHash)(DeeClassDescriptorObject *__restrict self, /*String*/DeeObject *__restrict name, dhash_t hash);
DFUNDEF struct class_attribute *(DCALL DeeClassDescriptor_QueryInstanceAttributeStringWithHash)(DeeClassDescriptorObject *__restrict self, char const *__restrict name, dhash_t hash);
#ifdef __INTELLISENSE__
DFUNDEF struct class_attribute *(DCALL DeeClassDescriptor_QueryClassAttribute)(DeeClassDescriptorObject *__restrict self, /*String*/DeeObject *__restrict name);
DFUNDEF struct class_attribute *(DCALL DeeClassDescriptor_QueryInstanceAttribute)(DeeClassDescriptorObject *__restrict self, /*String*/DeeObject *__restrict name);
DFUNDEF struct class_attribute *(DCALL DeeClassDescriptor_QueryClassAttributeString)(DeeClassDescriptorObject *__restrict self, char const *__restrict name);
DFUNDEF struct class_attribute *(DCALL DeeClassDescriptor_QueryInstanceAttributeString)(DeeClassDescriptorObject *__restrict self, char const *__restrict name);
#else
#define DeeClassDescriptor_QueryClassAttribute(self,name) \
        DeeClassDescriptor_QueryClassAttributeWithHash(self,name,DeeString_Hash(name))
#define DeeClassDescriptor_QueryInstanceAttribute(self,name) \
        DeeClassDescriptor_QueryInstanceAttributeWithHash(self,name,DeeString_Hash(name))
#define DeeClassDescriptor_QueryClassAttributeString(self,name) \
        DeeClassDescriptor_QueryClassAttributeStringWithHash(self,name,hash_str(name))
#define DeeClassDescriptor_QueryInstanceAttributeString(self,name) \
        DeeClassDescriptor_QueryInstanceAttributeStringWithHash(self,name,hash_str(name))
#endif



#define CLASS_HEADER_OPC1    8
#define CLASS_HEADER_OPC2  ((CLASS_OPERATOR_USERCOUNT+7)/8)

#if (CLASS_HEADER_OPC1*CLASS_HEADER_OPC2) < CLASS_OPERATOR_USERCOUNT
#error "FIXME: Not enough space for all available operators"
#endif

struct class_optable {
    /* [0..1][lock(READ(:cd_lock),SET_TO_NULL(:cd_lock),WRITE_ONCE)][*] Table of operators.
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
    uintptr_t                      cd_offset; /* [const] Offset to the `struct instance_desc' of instances. */
    struct class_optable          *cd_ops[CLASS_HEADER_OPC1];
                                              /* [0..1][owned][lock(WRITE_ONCE)][*]
                                               * Table of cached operator callbacks. */
#ifndef CONFIG_NO_THREADS
    rwlock_t                       cd_lock;   /* Lock for accessing the class member table. */
#endif
    DREF DeeObject                *cd_members[1024]; /* [0..1][lock(cd_lock)][cd_desc->cd_cmemb_size]
                                                      * The class member table (also contains
                                                      * instance-methods and operator callbacks). */
};

#ifndef CONFIG_NO_THREADS
#define class_desc_as_instance(x) ((struct instance_desc *)&(x)->cd_lock)
#else
#define class_desc_as_instance(x) ((struct instance_desc *)&(x)->cd_members[0])
#endif

#define DeeClass_Check(self)    (DeeType_Check(self) && DeeType_IsClass(self))
#define DeeInstance_Check(self)  DeeType_IsClass(Dee_TYPE(self))

/* Returns the descriptor for a given class. */
#define DeeClass_DESC(self) \
     (ASSERT_OBJECT_TYPE(self,&DeeType_Type),ASSERT(DeeType_IsClass(self)), \
    ((DeeTypeObject *)REQUIRES_OBJECT(self))->tp_class)


#define DeeClassDesc_QueryClassAttribute(self,name)                       DeeClassDescriptor_QueryClassAttribute((self)->cd_desc,name)
#define DeeClassDesc_QueryClassAttributeWithHash(self,name,hash)          DeeClassDescriptor_QueryClassAttributeWithHash((self)->cd_desc,name,hash)
#define DeeClassDesc_QueryClassAttributeStringWithHash(self,name,hash)    DeeClassDescriptor_QueryClassAttributeStringWithHash((self)->cd_desc,name,hash)
#define DeeClassDesc_QueryClassAttributeString(self,name)                 DeeClassDescriptor_QueryClassAttributeString((self)->cd_desc,name) 
#define DeeClassDesc_QueryInstanceAttribute(self,name)                    DeeClassDescriptor_QueryInstanceAttribute((self)->cd_desc,name)
#define DeeClassDesc_QueryInstanceAttributeWithHash(self,name,hash)       DeeClassDescriptor_QueryInstanceAttributeWithHash((self)->cd_desc,name,hash)
#define DeeClassDesc_QueryInstanceAttributeStringWithHash(self,name,hash) DeeClassDescriptor_QueryInstanceAttributeStringWithHash((self)->cd_desc,name,hash)
#define DeeClassDesc_QueryInstanceAttributeString(self,name)              DeeClassDescriptor_QueryInstanceAttributeString((self)->cd_desc,name) 

#define DeeClass_QueryClassAttribute(self,name)                       DeeClassDesc_QueryClassAttribute(DeeClass_DESC(self),name)
#define DeeClass_QueryClassAttributeWithHash(self,name,hash)          DeeClassDesc_QueryClassAttributeWithHash(DeeClass_DESC(self),name,hash)
#define DeeClass_QueryClassAttributeStringWithHash(self,name,hash)    DeeClassDesc_QueryClassAttributeStringWithHash(DeeClass_DESC(self),name,hash)
#define DeeClass_QueryClassAttributeString(self,name)                 DeeClassDesc_QueryClassAttributeString(DeeClass_DESC(self),name) 
#define DeeClass_QueryInstanceAttribute(self,name)                    DeeClassDesc_QueryInstanceAttribute(DeeClass_DESC(self),name)
#define DeeClass_QueryInstanceAttributeWithHash(self,name,hash)       DeeClassDesc_QueryInstanceAttributeWithHash(DeeClass_DESC(self),name,hash)
#define DeeClass_QueryInstanceAttributeStringWithHash(self,name,hash) DeeClassDesc_QueryInstanceAttributeStringWithHash(DeeClass_DESC(self),name,hash)
#define DeeClass_QueryInstanceAttributeString(self,name)              DeeClassDesc_QueryInstanceAttributeString(DeeClass_DESC(self),name) 


struct instance_desc {
#ifndef CONFIG_NO_THREADS
    rwlock_t        id_lock;       /* Lock that must be held when accessing  */
#endif
    DREF DeeObject *id_vtab[1024]; /* [0..1][lock(id_lock)]
                                    * [DeeClass_DESC(:ob_type)->cd_desc->cd_imemb_size]
                                    * Instance member table. */
};

#define DeeInstance_DESC(class_descriptor,self) \
      ((struct instance_desc *)((uintptr_t)REQUIRES_OBJECT(self)+(class_descriptor)->cd_offset))

#ifdef CONFIG_BUILDING_DEEMON
struct attribute_info;
struct attribute_lookup_rules;

/* Instance member access (by addr) */
INTDEF DREF DeeObject *DCALL DeeInstance_GetMember(/*Class*/DeeTypeObject *__restrict tp_self, /*Instance*/DeeObject *__restrict self, uint16_t addr);
INTDEF bool DCALL DeeInstance_BoundMember(/*Class*/DeeTypeObject *__restrict tp_self, /*Instance*/DeeObject *__restrict self, uint16_t addr);
INTDEF int DCALL DeeInstance_DelMember(/*Class*/DeeTypeObject *__restrict tp_self, /*Instance*/DeeObject *__restrict self, uint16_t addr);
INTDEF void DCALL DeeInstance_SetMember(/*Class*/DeeTypeObject *__restrict tp_self, /*Instance*/DeeObject *__restrict self, uint16_t addr, DeeObject *__restrict value);
INTDEF DREF DeeObject *DCALL DeeInstance_GetMemberSafe(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, uint16_t addr);
INTDEF int DCALL DeeInstance_BoundMemberSafe(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, uint16_t addr);
INTDEF int DCALL DeeInstance_DelMemberSafe(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, uint16_t addr);
INTDEF int DCALL DeeInstance_SetMemberSafe(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, uint16_t addr, DeeObject *__restrict value);

/* Class member access (by addr) */
INTDEF void DCALL DeeClass_SetMember(DeeTypeObject *__restrict self, uint16_t addr, DeeObject *__restrict value);
INTDEF int DCALL DeeClass_SetMemberSafe(DeeTypeObject *__restrict self, uint16_t addr, DeeObject *__restrict value);
INTDEF DREF DeeObject *DCALL DeeClass_GetMember(DeeTypeObject *__restrict self, uint16_t addr);
INTDEF DREF DeeObject *DCALL DeeClass_GetMemberSafe(DeeTypeObject *__restrict self, uint16_t addr);


/* Enumerate user-defined class or instance attributes. */
INTDEF dssize_t DCALL DeeClass_EnumClassAttributes(DeeTypeObject *__restrict self, denum_t proc, void *arg);
INTDEF dssize_t DCALL DeeClass_EnumInstanceAttributes(DeeTypeObject *__restrict self, DeeObject *instance, denum_t proc, void *arg);

/* Enumerate user-defined instance attributes, as
 * accessed by `DeeClass_GetInstanceAttribute()'. */
INTDEF dssize_t DCALL DeeClass_EnumClassInstanceAttributes(DeeTypeObject *__restrict self, denum_t proc, void *arg);

/* Find a specific class-, instance- or
 * instance-through-class-attribute, matching the given lookup rules.
 * @return:  0: Attribute found (*result was filled with data).
 * @return:  1: No attribute matching the given requirements was found.
 * @return: -1: An error occurred. */
#ifdef CONFIG_USE_NEW_TYPE_ATTRIBUTE_CACHING
INTDEF int DCALL DeeClass_FindClassAttribute(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict self, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);
INTDEF int DCALL DeeClass_FindInstanceAttribute(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict self, DeeObject *instance, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);
INTDEF int DCALL DeeClass_FindClassInstanceAttribute(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict self, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);
#else
INTDEF int DCALL DeeClass_FindClassAttribute(DeeTypeObject *__restrict self, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);
INTDEF int DCALL DeeClass_FindInstanceAttribute(DeeTypeObject *__restrict self, DeeObject *instance, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);
INTDEF int DCALL DeeClass_FindClassInstanceAttribute(DeeTypeObject *__restrict self, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);
#endif

/* Get/Call/Del/Set an instance attribute, as acquired
 * through `DeeClassDescriptor_QueryInstanceAttribute()'. */
INTDEF DREF DeeObject *DCALL DeeInstance_GetAttribute(struct class_desc *__restrict desc, struct instance_desc *__restrict self, DeeObject *__restrict this_arg, struct class_attribute *__restrict attr);
INTDEF int DCALL DeeInstance_BoundAttribute(struct class_desc *__restrict desc, struct instance_desc *__restrict self, DeeObject *__restrict this_arg, struct class_attribute *__restrict attr);
INTDEF DREF DeeObject *DCALL DeeInstance_CallAttribute(struct class_desc *__restrict desc, struct instance_desc *__restrict self, DeeObject *__restrict this_arg, struct class_attribute *__restrict attr, size_t argc, DeeObject **__restrict argv);
INTDEF DREF DeeObject *DCALL DeeInstance_VCallAttributef(struct class_desc *__restrict desc, struct instance_desc *__restrict self, DeeObject *__restrict this_arg, struct class_attribute *__restrict attr, char const *__restrict format, va_list args);
INTDEF DREF DeeObject *DCALL DeeInstance_CallAttributeKw(struct class_desc *__restrict desc, struct instance_desc *__restrict self, DeeObject *__restrict this_arg, struct class_attribute *__restrict attr, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#ifdef CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS
INTDEF DREF DeeObject *DCALL DeeInstance_CallAttributeTuple(struct class_desc *__restrict desc, struct instance_desc *__restrict self, DeeObject *__restrict this_arg, struct class_attribute *__restrict attr, DeeObject *__restrict args);
INTDEF DREF DeeObject *DCALL DeeInstance_CallAttributeTupleKw(struct class_desc *__restrict desc, struct instance_desc *__restrict self, DeeObject *__restrict this_arg, struct class_attribute *__restrict attr, DeeObject *__restrict args, DeeObject *kw);
#endif /* CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS */
INTDEF int DCALL DeeInstance_DelAttribute(struct class_desc *__restrict desc, struct instance_desc *__restrict self, DeeObject *__restrict this_arg, struct class_attribute *__restrict attr);
INTDEF int DCALL DeeInstance_SetAttribute(struct class_desc *__restrict desc, struct instance_desc *__restrict self, DeeObject *__restrict this_arg, struct class_attribute *__restrict attr, DeeObject *__restrict value);
/* @return:  2: Attribute isn't a basic one
 * @return:  0: Basic attribute successfully set
 * @return: -1: An error occurred. */
INTDEF int DCALL DeeInstance_SetBasicAttribute(struct class_desc *__restrict desc, struct instance_desc *__restrict self, DeeObject *__restrict this_arg, struct class_attribute *__restrict attr, DeeObject *__restrict value);

/* Get/Call/Del/Set a class attribute, as acquired
 * through `DeeClassDescriptor_QueryClassAttribute()'.
 * TODO: Adjust these functions to split between the type
 *       implementing the attribute, and the type accessing
 *       the attribute! */
#ifdef __INTELLISENSE__
INTDEF DREF DeeObject *DCALL DeeClass_GetClassAttribute(DeeTypeObject *__restrict class_type, struct class_attribute *__restrict attr);
INTDEF int DCALL DeeClass_BoundClassAttribute(DeeTypeObject *__restrict class_type, struct class_attribute *__restrict attr);
INTDEF DREF DeeObject *DCALL DeeClass_CallClassAttribute(DeeTypeObject *__restrict class_type, struct class_attribute *__restrict attr, size_t argc, DeeObject **__restrict argv);
INTDEF DREF DeeObject *DCALL DeeClass_CallClassAttributeKw(DeeTypeObject *__restrict class_type, struct class_attribute *__restrict attr, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#ifdef CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS
INTDEF DREF DeeObject *DCALL DeeClass_CallClassAttributeTuple(DeeTypeObject *__restrict class_type, struct class_attribute *__restrict attr, DeeObject *__restrict args);
INTDEF DREF DeeObject *DCALL DeeClass_CallClassAttributeTupleKw(DeeTypeObject *__restrict class_type, struct class_attribute *__restrict attr, DeeObject *__restrict args, DeeObject *kw);
#endif /* CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS */
INTDEF int DCALL DeeClass_DelClassAttribute(DeeTypeObject *__restrict class_type, struct class_attribute *__restrict attr);
INTDEF int DCALL DeeClass_SetClassAttribute(DeeTypeObject *__restrict class_type, struct class_attribute *__restrict attr, DeeObject *__restrict value);
#else
#define DeeClass_GetClassAttribute(class_type,attr)                 DeeInstance_GetAttribute(DeeClass_DESC(class_type),class_desc_as_instance(DeeClass_DESC(class_type)),(DeeObject *)(class_type),attr)
#define DeeClass_BoundClassAttribute(class_type,attr)               DeeInstance_BoundAttribute(DeeClass_DESC(class_type),class_desc_as_instance(DeeClass_DESC(class_type)),(DeeObject *)(class_type),attr)
#define DeeClass_CallClassAttribute(class_type,attr,argc,argv)      DeeInstance_CallAttribute(DeeClass_DESC(class_type),class_desc_as_instance(DeeClass_DESC(class_type)),(DeeObject *)(class_type),attr,argc,argv)
#define DeeClass_CallClassAttributeKw(class_type,attr,argc,argv,kw) DeeInstance_CallAttributeKw(DeeClass_DESC(class_type),class_desc_as_instance(DeeClass_DESC(class_type)),(DeeObject *)(class_type),attr,argc,argv,kw)
#ifdef CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS
#define DeeClass_CallClassAttributeTuple(class_type,attr,args)      DeeInstance_CallAttributeTuple(DeeClass_DESC(class_type),class_desc_as_instance(DeeClass_DESC(class_type)),(DeeObject *)(class_type),attr,args)
#define DeeClass_CallClassAttributeTupleKw(class_type,attr,args,kw) DeeInstance_CallAttributeTupleKw(DeeClass_DESC(class_type),class_desc_as_instance(DeeClass_DESC(class_type)),(DeeObject *)(class_type),attr,args,kw)
#endif /* CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS */
#define DeeClass_DelClassAttribute(class_type,attr)                 DeeInstance_DelAttribute(DeeClass_DESC(class_type),class_desc_as_instance(DeeClass_DESC(class_type)),(DeeObject *)(class_type),attr)
#define DeeClass_SetClassAttribute(class_type,attr,value)           DeeInstance_SetAttribute(DeeClass_DESC(class_type),class_desc_as_instance(DeeClass_DESC(class_type)),(DeeObject *)(class_type),attr,value)
#endif

/* Get/Call/Del/Set a class attribute, as acquired
 * through `DeeClassDescriptor_QueryInstanceAttribute()'.
 * These functions produce and interact with with proxy
 * objects constructed when accessing instance attributes
 * through their defining class:
 * >> class MyClass {
 * >>     public class member class_field = 84;
 * >>     public field = 42;
 * >>     function foo() {
 * >>         print "foo():",field;
 * >>     }
 * >> }
 * >> local x = MyClass();
 * >> print x.field;                 // DeeInstance_GetAttribute("field")
 * >> x.foo();                       // DeeInstance_CallAttribute("foo")
 * >> print MyClass.class_field;     // DeeClass_GetClassAttribute("class_field")
 * >> myclass_field = MyClass.field; // DeeClass_GetInstanceAttribute("field")
 */
INTDEF DREF DeeObject *DCALL DeeClass_GetInstanceAttribute(DeeTypeObject *__restrict class_type, struct class_attribute *__restrict attr);
INTDEF DREF DeeObject *DCALL DeeClass_CallInstanceAttribute(DeeTypeObject *__restrict class_type, struct class_attribute *__restrict attr, size_t argc, DeeObject **__restrict argv);
INTDEF DREF DeeObject *DCALL DeeClass_CallInstanceAttributeKw(DeeTypeObject *__restrict class_type, struct class_attribute *__restrict attr, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#ifdef CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS
INTDEF DREF DeeObject *DCALL DeeClass_CallInstanceAttributeTuple(DeeTypeObject *__restrict class_type, struct class_attribute *__restrict attr, DeeObject *__restrict args);
INTDEF DREF DeeObject *DCALL DeeClass_CallInstanceAttributeTupleKw(DeeTypeObject *__restrict class_type, struct class_attribute *__restrict attr, DeeObject *__restrict args, DeeObject *kw);
#endif /* CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS */
INTDEF DREF DeeObject *DCALL DeeClass_VCallInstanceAttributef(DeeTypeObject *__restrict class_type, struct class_attribute *__restrict attr, char const *__restrict format, va_list args);
INTDEF int DCALL DeeClass_DelInstanceAttribute(DeeTypeObject *__restrict class_type, struct class_attribute *__restrict attr);
INTDEF int DCALL DeeClass_SetInstanceAttribute(DeeTypeObject *__restrict class_type, struct class_attribute *__restrict attr, DeeObject *__restrict value);
INTDEF int DCALL DeeClass_BoundInstanceAttribute(DeeTypeObject *__restrict class_type, struct class_attribute *__restrict attr);
#define DeeClass_SetBasicInstanceAttribute DeeClass_SetInstanceAttribute

#endif



/* Create a new class type derived from `base',
 * featuring traits from `descriptor'.
 * @param: base: The base of the resulting class.
 *               You may pass `Dee_None' to have the resulting
 *               class not be derived from anything (be base-less).
 * @param: descriptor: A `DeeClassDescriptor_Type'-object, detailing the class's prototype.
 * @throw: TypeError: The given `base' is neither `none', nor a type-object.
 * @throw: TypeError: The given `base' is a final or variable type. */
DFUNDEF DREF DeeTypeObject *DCALL
DeeClass_New(DeeTypeObject *__restrict base,
             DeeObject *__restrict descriptor);

/* Return the nearest operator function for `name',
 * implemented by `self', which must be a class type.
 * If the operator doesn't exist, return NULL and throw
 * a NotImplemented error, or return NULL and don't throw
 * an error when `DeeClass_TryGetOperator()' was used. */
DFUNDEF DREF DeeObject *DCALL
DeeClass_GetOperator(DeeTypeObject *__restrict self, uint16_t name);

/* Same as `DeeClass_GetOperator()', but don't simply return `NULL'
 * if the operator hasn't been implemented, and `ITER_DONE' when it
 * has been, but wasn't assigned anything. */
DFUNDEF DREF DeeObject *DCALL
DeeClass_TryGetOperator(DeeTypeObject *__restrict self, uint16_t name);

/* Same as `DeeClass_TryGetOperator()', but don't return an operator
 * that has been inherited from a base-class, but return `NULL' instead. */
DFUNDEF DREF DeeObject *DCALL
DeeClass_TryGetPrivateOperator(DeeTypeObject *__restrict self, uint16_t name);


#ifdef CONFIG_BUILDING_DEEMON

/* The functions bound to the C-level type callbacks when a
 * user-defined class provides the associated operator.
 * All of the `instance_*' functions simply call the associated
 * `instance_t*' function, which the proceeds to load (and
 * potentially cache) the operator, before invoking it. */

/* `OPERATOR_CONSTRUCTOR' + `CLASS_OPERATOR_SUPERARGS' */
INTDEF int DCALL instance_super_tctor(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTDEF int DCALL instance_super_ctor(DeeObject *__restrict self);
INTDEF int DCALL instance_super_tinit(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTDEF int DCALL instance_super_init(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTDEF int DCALL instance_super_tinitkw(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF int DCALL instance_super_initkw(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);

/* `CLASS_OPERATOR_SUPERARGS' */
INTDEF int DCALL instance_builtin_super_tctor(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTDEF int DCALL instance_builtin_super_ctor(DeeObject *__restrict self);
INTDEF int DCALL instance_builtin_super_tinit(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTDEF int DCALL instance_builtin_super_init(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTDEF int DCALL instance_builtin_super_tinitkw(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF int DCALL instance_builtin_super_initkw(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);

/* `OPERATOR_CONSTRUCTOR' */
INTDEF int DCALL instance_tctor(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTDEF int DCALL instance_ctor(DeeObject *__restrict self);
INTDEF int DCALL instance_tinit(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTDEF int DCALL instance_init(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTDEF int DCALL instance_tinitkw(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF int DCALL instance_initkw(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);

#ifdef CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS
/* `OPERATOR_CONSTRUCTOR' (but the type doesn't have a base) */
INTDEF int DCALL instance_nobase_tctor(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTDEF int DCALL instance_nobase_ctor(DeeObject *__restrict self);
INTDEF int DCALL instance_nobase_tinit(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTDEF int DCALL instance_nobase_init(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTDEF int DCALL instance_nobase_tinitkw(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF int DCALL instance_nobase_initkw(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#endif /* CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS */

/* `OPERATOR_CONSTRUCTOR', with the `TP_FINHERITCTOR' flag set.
 * NOTE: These functions always invoke the user-defined constructor without any arguments! */
#define instance_inherited_tctor instance_tctor
#define instance_inherited_ctor  instance_ctor
INTDEF int DCALL instance_inherited_tinit(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTDEF int DCALL instance_inherited_init(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTDEF int DCALL instance_inherited_tinitkw(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF int DCALL instance_inherited_initkw(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);

/* No predefined construction operators. */
INTDEF int DCALL instance_builtin_tctor(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTDEF int DCALL instance_builtin_ctor(DeeObject *__restrict self);
INTDEF int DCALL instance_builtin_tinit(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTDEF int DCALL instance_builtin_init(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTDEF int DCALL instance_builtin_tinitkw(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF int DCALL instance_builtin_initkw(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);

#ifdef CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS
/* No predefined construction operators. (but the type doesn't have a base) */
INTDEF int DCALL instance_builtin_nobase_tctor(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTDEF int DCALL instance_builtin_nobase_ctor(DeeObject *__restrict self);
INTDEF int DCALL instance_builtin_nobase_tinit(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTDEF int DCALL instance_builtin_nobase_init(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTDEF int DCALL instance_builtin_nobase_tinitkw(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF int DCALL instance_builtin_nobase_initkw(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#endif /* CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS */

/* No predefined construction operators, but the `TP_FINHERITCTOR' flag is set. */
INTDEF int DCALL instance_builtin_inherited_tctor(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTDEF int DCALL instance_builtin_inherited_ctor(DeeObject *__restrict self);
INTDEF int DCALL instance_builtin_inherited_tinit(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTDEF int DCALL instance_builtin_inherited_init(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTDEF int DCALL instance_builtin_inherited_tinitkw(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF int DCALL instance_builtin_inherited_initkw(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);

/* Builtin (pre-defined) hooks that are used when the user-class doesn't override these operators. */
INTDEF int DCALL instance_builtin_tcopy(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF int DCALL instance_builtin_copy(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF int DCALL instance_builtin_tdeepload(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTDEF int DCALL instance_builtin_deepload(DeeObject *__restrict self);
#ifdef CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS
INTDEF int DCALL instance_builtin_nobase_tcopy(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF int DCALL instance_builtin_nobase_copy(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF int DCALL instance_builtin_nobase_deepload(DeeObject *__restrict self);
#endif /* CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS */
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

/* GC support for class objects. */
INTDEF void DCALL instance_tvisit(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, dvisit_t proc, void *arg);
INTDEF void DCALL instance_visit(DeeObject *__restrict self, dvisit_t proc, void *arg);
INTDEF void DCALL instance_tclear(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTDEF void DCALL instance_clear(DeeObject *__restrict self);
INTDEF void DCALL instance_tpclear(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, unsigned int gc_priority);
INTDEF void DCALL instance_pclear(DeeObject *__restrict self, unsigned int gc_priority);
INTDEF struct type_gc instance_gc;

/* Builtin (standard) operators for hashing and comparing class objects. */
INTDEF dhash_t DCALL instance_builtin_thash(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTDEF dhash_t DCALL instance_builtin_hash(DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL instance_builtin_teq(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_builtin_eq(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_builtin_tne(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_builtin_ne(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_builtin_tlo(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_builtin_lo(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_builtin_tle(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_builtin_le(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_builtin_tgr(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_builtin_gr(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_builtin_tge(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_builtin_ge(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF struct type_cmp instance_builtin_cmp;

/* Hooks for user-defined operators. */
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
INTDEF DREF DeeObject *DCALL instance_tint(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL instance_int(DeeObject *__restrict self);
INTDEF int DCALL instance_tdouble(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, double *__restrict result);
INTDEF int DCALL instance_double(DeeObject *__restrict self, double *__restrict result);
INTDEF DREF DeeObject *DCALL instance_tinv(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL instance_inv(DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL instance_tpos(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL instance_pos(DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL instance_tneg(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL instance_neg(DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL instance_tadd(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_add(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_tsub(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_sub(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_tmul(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_mul(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_tdiv(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_div(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_tmod(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_mod(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_tshl(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_shl(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_tshr(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_shr(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_tand(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_and(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_tor(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_or(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_txor(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_xor(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_tpow(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_pow(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF int DCALL instance_tinc(DeeTypeObject *__restrict tp_self, DeeObject **__restrict pself);
INTDEF int DCALL instance_inc(DeeObject **__restrict pself);
INTDEF int DCALL instance_tdec(DeeTypeObject *__restrict tp_self, DeeObject **__restrict pself);
INTDEF int DCALL instance_dec(DeeObject **__restrict pself);
INTDEF int DCALL instance_tiadd(DeeTypeObject *__restrict tp_self, DeeObject **__restrict pself, DeeObject *__restrict other);
INTDEF int DCALL instance_iadd(DeeObject **__restrict pself, DeeObject *__restrict other);
INTDEF int DCALL instance_tisub(DeeTypeObject *__restrict tp_self, DeeObject **__restrict pself, DeeObject *__restrict other);
INTDEF int DCALL instance_isub(DeeObject **__restrict pself, DeeObject *__restrict other);
INTDEF int DCALL instance_timul(DeeTypeObject *__restrict tp_self, DeeObject **__restrict pself, DeeObject *__restrict other);
INTDEF int DCALL instance_imul(DeeObject **__restrict pself, DeeObject *__restrict other);
INTDEF int DCALL instance_tidiv(DeeTypeObject *__restrict tp_self, DeeObject **__restrict pself, DeeObject *__restrict other);
INTDEF int DCALL instance_idiv(DeeObject **__restrict pself, DeeObject *__restrict other);
INTDEF int DCALL instance_timod(DeeTypeObject *__restrict tp_self, DeeObject **__restrict pself, DeeObject *__restrict other);
INTDEF int DCALL instance_imod(DeeObject **__restrict pself, DeeObject *__restrict other);
INTDEF int DCALL instance_tishl(DeeTypeObject *__restrict tp_self, DeeObject **__restrict pself, DeeObject *__restrict other);
INTDEF int DCALL instance_ishl(DeeObject **__restrict pself, DeeObject *__restrict other);
INTDEF int DCALL instance_tishr(DeeTypeObject *__restrict tp_self, DeeObject **__restrict pself, DeeObject *__restrict other);
INTDEF int DCALL instance_ishr(DeeObject **__restrict pself, DeeObject *__restrict other);
INTDEF int DCALL instance_tiand(DeeTypeObject *__restrict tp_self, DeeObject **__restrict pself, DeeObject *__restrict other);
INTDEF int DCALL instance_iand(DeeObject **__restrict pself, DeeObject *__restrict other);
INTDEF int DCALL instance_tior(DeeTypeObject *__restrict tp_self, DeeObject **__restrict pself, DeeObject *__restrict other);
INTDEF int DCALL instance_ior(DeeObject **__restrict pself, DeeObject *__restrict other);
INTDEF int DCALL instance_tixor(DeeTypeObject *__restrict tp_self, DeeObject **__restrict pself, DeeObject *__restrict other);
INTDEF int DCALL instance_ixor(DeeObject **__restrict pself, DeeObject *__restrict other);
INTDEF int DCALL instance_tipow(DeeTypeObject *__restrict tp_self, DeeObject **__restrict pself, DeeObject *__restrict other);
INTDEF int DCALL instance_ipow(DeeObject **__restrict pself, DeeObject *__restrict other);
INTDEF dhash_t DCALL instance_thash(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTDEF dhash_t DCALL instance_hash(DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL instance_teq(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_eq(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_tne(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_ne(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_tlo(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_lo(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_tle(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_le(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_tgr(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_gr(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_tge(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_ge(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_titer(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL instance_iter(DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL instance_tsize(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL instance_size(DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL instance_tcontains(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_contains(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_tgetitem(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL instance_getitem(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF int DCALL instance_tdelitem(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF int DCALL instance_delitem(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF int DCALL instance_tsetitem(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other, DeeObject *__restrict value);
INTDEF int DCALL instance_setitem(DeeObject *__restrict self, DeeObject *__restrict other, DeeObject *__restrict value);
INTDEF DREF DeeObject *DCALL instance_tgetrange(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict start, DeeObject *__restrict end);
INTDEF DREF DeeObject *DCALL instance_getrange(DeeObject *__restrict self, DeeObject *__restrict start, DeeObject *__restrict end);
INTDEF int DCALL instance_tdelrange(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict start, DeeObject *__restrict end);
INTDEF int DCALL instance_delrange(DeeObject *__restrict self, DeeObject *__restrict start, DeeObject *__restrict end);
INTDEF int DCALL instance_tsetrange(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict start, DeeObject *__restrict end, DeeObject *__restrict value);
INTDEF int DCALL instance_setrange(DeeObject *__restrict self, DeeObject *__restrict start, DeeObject *__restrict end, DeeObject *__restrict value);
INTDEF int DCALL instance_tenter(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTDEF int DCALL instance_enter(DeeObject *__restrict self);
INTDEF int DCALL instance_tleave(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTDEF int DCALL instance_leave(DeeObject *__restrict self);

INTDEF DREF DeeObject *DCALL instance_tgetattr(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, /*String*/DeeObject *__restrict name);
INTDEF DREF DeeObject *DCALL instance_getattr(DeeObject *__restrict self, /*String*/DeeObject *__restrict name);
INTDEF int DCALL instance_tdelattr(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, /*String*/DeeObject *__restrict name);
INTDEF int DCALL instance_delattr(DeeObject *__restrict self, /*String*/DeeObject *__restrict name);
INTDEF int DCALL instance_tsetattr(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, /*String*/DeeObject *__restrict name, DeeObject *__restrict value);
INTDEF int DCALL instance_setattr(DeeObject *__restrict self, /*String*/DeeObject *__restrict name, DeeObject *__restrict value);
INTDEF dssize_t DCALL instance_enumattr(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, denum_t proc, void *arg);

#endif


/* Instance-member wrapper objects
 * >> class MyClass {
 * >>     member foo = 42;
 * >> }
 * >> print type MyClass.foo; // DeeInstanceMember_Type
 */
typedef struct instancemember_object DeeInstanceMemberObject;
struct instancemember_object {
    OBJECT_HEAD
    DREF DeeTypeObject     *im_type;      /* [1..1][const] The user-class type, instances of which implement this member. */
    struct class_attribute *im_attribute; /* [1..1][const] The instance attribute (`CLASS_ATTRIBUTE_FCLASSMEM' shouldn't
                                           * be set, though this isn't asserted) that should be accessed. */
};

DDATDEF DeeTypeObject DeeInstanceMember_Type;
#define DeeInstanceMember_Check(ob)      DeeObject_InstanceOfExact(ob,&DeeInstanceMember_Type) /* `_instancemember' is final */
#define DeeInstanceMember_CheckExact(ob) DeeObject_InstanceOfExact(ob,&DeeInstanceMember_Type)

/* Construct a new instance member for the given `attribute' */
DFUNDEF DREF DeeObject *DCALL
DeeInstanceMember_New(DeeTypeObject *__restrict class_type,
                      struct class_attribute *__restrict attribute);

DECL_END

#endif /* !GUARD_DEEMON_CLASS_H */
