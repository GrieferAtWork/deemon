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
#ifndef GUARD_DEEMON_CLASS_H
#define GUARD_DEEMON_CLASS_H 1

#include "api.h"

#include <stddef.h>
#include <stdint.h>

#include "object.h"
#include "util/lock.h"

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
 * >> }                                    >>         import NotImplemented from errors;
 *                                         >>         throw NotImplemented("...");
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
 * >>     member bar;                      >>     member bar;
 * >>     private member foobar;           >>     private member foobar; // Not initialized by the default constructor
 * >>     this = default;                  >>     this(foo?, bar?): super() {
 * >> }                                    >>         this.foo = 42;
 *                                         >>         if (foo is bound)
 *                                         >>             this.foo = foo;
 *                                         >>         if (bar is bound)
 *                                         >>             this.bar = bar;
 *                                         >>     }
 *                                         >>     // NOTE: `operator repr' is only implemented if not otherwise defined by the user
 *                                         >>     operator repr(): string {
 *                                         >>         File.Writer fp;
 *                                         >>         fp << "MyClass(";
 *                                         >>         local is_first = true;
 *                                         >>         if (foo is bound) {
 *                                         >>             if (!is_first) fp << ", ";
 *                                         >>             is_first = false;
 *                                         >>             fp << "foo: " << repr foo;
 *                                         >>         }
 *                                         >>         if (bar is bound) {
 *                                         >>             if (!is_first) fp << ", ";
 *                                         >>             is_first = false;
 *                                         >>             fp << "bar: " << repr bar;
 *                                         >>         }
 *                                         >>         fp << ")";
 *                                         >>         return fp.string;
 *                                         >>     }
 *                                         >>     ~this() {
 *                                         >>         del foo;
 *                                         >>         del bar;
 *                                         >>     }
 *                                         >> }
 *
 * >> class MyClass: Base {                >> class MyClass: Base {
 * >>     member foo = 42;                 >>     member foo;
 * >>     this = super;                    >>     this(args..., **kwds): super(args..., **kwds) {
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
 * >>         print "foo =", foo;          >>         foo = 42;
 * >>     }                                >>         {
 * >> }                                    >>             print "foo =", foo;
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
 *  - constructor                          >>         if ((other as MyClass).foo    is bound) foo    = (other as MyClass).foo;
 *  - destructor                           >>         if ((other as MyClass).bar    is bound) bar    = (other as MyClass).bar;
 *  - assign, moveassign                   >>         if ((other as MyClass).foobar is bound) foobar = (other as MyClass).foobar;
 *  - hash, eq, ne, lo, le, gr, ge         >>     }
 * Also note that when providing a `copy'  >>     operator deepcopy(other) {
 * operator, it will also be invoked when  >>         // super.deepcopy(other);
 * `deepcopy' is called, after which each  >>         if (other !is MyClass)
 * of the instance's bound members will be >>             throw Error.TypeError("...");
 * replace with a deep copy of itself.     >>         if ((other as MyClass).foo    is bound) foo    = deepcopy((other as MyClass).foo);
 *                                         >>         if ((other as MyClass).bar    is bound) bar    = deepcopy((other as MyClass).bar);
 * Note that attribute accesses made on    >>         if ((other as MyClass).foobar is bound) foobar = deepcopy((other as MyClass).foobar);
 * `other' in automatic operators will not >>     }
 * invoke a potential `operator getattr',  >>     operator hash() {
 * but will always access the native       >>         return (foo !is bound ? 0 : foo.operator hash()) ^
 * attribute.                              >>                (bar !is bound ? 0 : bar.operator hash()) ^
 *                                         >>                (foobar !is bound ? 0 : foobar.operator hash());
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
 *       much more efficiently that your destructor ever could.
 *       Also note that it does this even when you define a destructor,
 *       so no custom destructor should ever end by unbinding instance
 *       members (as a matter of fact: There should be an optimization
 *       that removes meaningless unbind statements from destructors)
 */



DECL_BEGIN


#ifdef DEE_SOURCE
#define Dee_module_object           module_object
#define Dee_string_object           string_object
#define Dee_class_descriptor_object class_descriptor_object
#define Dee_class_optable           class_optable
#define Dee_class_desc              class_desc
#define Dee_instance_desc           instance_desc
#define Dee_class_operator          class_operator
#define Dee_class_attribute         class_attribute
#define Dee_instancemember_object   instancemember_object
#define class_desc_as_instance      Dee_class_desc_as_instance
#define CLASS_HEADER_OPC1           Dee_CLASS_HEADER_OPC1
#define CLASS_HEADER_OPC2           Dee_CLASS_HEADER_OPC2
#define CLASS_GETSET_GET            Dee_CLASS_GETSET_GET
#define CLASS_GETSET_DEL            Dee_CLASS_GETSET_DEL
#define CLASS_GETSET_SET            Dee_CLASS_GETSET_SET
#define CLASS_GETSET_COUNT          Dee_CLASS_GETSET_COUNT
#define CLASS_ATTRIBUTE_FNORMAL     Dee_CLASS_ATTRIBUTE_FNORMAL
#define CLASS_ATTRIBUTE_FPUBLIC     Dee_CLASS_ATTRIBUTE_FPUBLIC
#define CLASS_ATTRIBUTE_FPRIVATE    Dee_CLASS_ATTRIBUTE_FPRIVATE
#define CLASS_ATTRIBUTE_FVISIBILITY Dee_CLASS_ATTRIBUTE_FVISIBILITY
#define CLASS_ATTRIBUTE_FFINAL      Dee_CLASS_ATTRIBUTE_FFINAL
#define CLASS_ATTRIBUTE_FREADONLY   Dee_CLASS_ATTRIBUTE_FREADONLY
#define CLASS_ATTRIBUTE_FMETHOD     Dee_CLASS_ATTRIBUTE_FMETHOD
#define CLASS_ATTRIBUTE_FGETSET     Dee_CLASS_ATTRIBUTE_FGETSET
#define CLASS_ATTRIBUTE_FCLASSMEM   Dee_CLASS_ATTRIBUTE_FCLASSMEM
#define CLASS_ATTRIBUTE_FMASK       Dee_CLASS_ATTRIBUTE_FMASK
#define CLASS_ATTRIBUTE_ALLOW_AUTOINIT Dee_CLASS_ATTRIBUTE_ALLOW_AUTOINIT
#endif /* DEE_SOURCE */


struct Dee_string_object;
typedef struct Dee_class_descriptor_object DeeClassDescriptorObject;

#define Dee_CLASS_GETSET_GET   0 /* Offset to the getter of a user-defined getset in a class. */
#define Dee_CLASS_GETSET_DEL   1 /* Offset to the delete of a user-defined getset in a class. */
#define Dee_CLASS_GETSET_SET   2 /* Offset to the setter of a user-defined getset in a class. */
#define Dee_CLASS_GETSET_COUNT 3 /* # of operators defined by getsets. */

#define Dee_CLASS_ATTRIBUTE_FNORMAL   0x0000 /* Normal class attribute flags. */
#define Dee_CLASS_ATTRIBUTE_FPUBLIC   0x0000 /* The attribute is publicly available. */
#define Dee_CLASS_ATTRIBUTE_FPRIVATE  0x0001 /* The attribute is only accessible from this-call functions with an instance of the class as this-argument. */
#define Dee_CLASS_ATTRIBUTE_FVISIBILITY (Dee_CLASS_ATTRIBUTE_FPRIVATE) /* Mask of flags affecting symbol visibility. */
#define Dee_CLASS_ATTRIBUTE_FFINAL    0x0002 /* The attribute is accessed directly, and cannot be overwritten by sub-classes. */
#define Dee_CLASS_ATTRIBUTE_FREADONLY 0x0004 /* The attribute can only ever be when not already bound (and it cannot be unbound). */
/*      Dee_CLASS_ATTRIBUTE_F         0x0008  * ... */
#define Dee_CLASS_ATTRIBUTE_FMETHOD   0x0010 /* When accessed as get in `foo.bar', return an `InstanceMethod(MEMBER_TABLE[ca_addr], foo)' (calling `foo.bar()' will similarly perform a this-call). */
#define Dee_CLASS_ATTRIBUTE_FGETSET   0x0020 /* Access to the attribute is done via get/set, with the callbacks being `CLASS_GETSET_*' offsets from `ca_addr'.
                                              * When `Dee_CLASS_ATTRIBUTE_FMETHOD' is set, callbacks are invoked as this-calls.
                                              * When `Dee_CLASS_ATTRIBUTE_FREADONLY' is set, only `CLASS_GETSET_GET' is ever accessed,
                                              * and all other callbacks behave as though they were unbound. */
/*      Dee_CLASS_ATTRIBUTE_F         0x0040  * ... */
#define Dee_CLASS_ATTRIBUTE_FCLASSMEM 0x0080 /* An instance-attribute is stored in class memory (usually set for instance member functions).
                                              * NOTE: Ignored when used by attributes in `cd_cattr_list', where
                                              *       access is always done like it would be when this was set. */
/*      Dee_CLASS_ATTRIBUTE_F         0x0100  * ... */
/*      Dee_CLASS_ATTRIBUTE_F         0x8000  * ... */
#define Dee_CLASS_ATTRIBUTE_FMASK     0x00b7 /* Mask of known flag bits. */

struct Dee_class_attribute {
	DREF struct Dee_string_object *ca_name; /* [0..1][const] Name of this member.
	                                         * NOTE: NULL indicates a sentinel/unused entry. */
	Dee_hash_t                     ca_hash; /* [== cme_name->s_hash][const] */
	DREF struct Dee_string_object *ca_doc;  /* [0..1][const] A Documentation string for this member. */
	uint16_t                       ca_addr; /* [const] Attribute address within the instance / class table. */
	uint16_t                       ca_flag; /* Class member flags (Set of `Dee_CLASS_ATTRIBUTE_F*') */
#if __SIZEOF_POINTER__ > 4
	uint16_t                       ca_pad[(sizeof(void *) / 2) - 2];
#endif /* __SIZEOF_POINTER__ > 4 */
};
#define Dee_CLASS_ATTRIBUTE_ALLOW_AUTOINIT(x)                                       \
	(!((x)->ca_flag & (Dee_CLASS_ATTRIBUTE_FPRIVATE | Dee_CLASS_ATTRIBUTE_FGETSET | \
	                   Dee_CLASS_ATTRIBUTE_FMETHOD | Dee_CLASS_ATTRIBUTE_FCLASSMEM)))


#ifdef CONFIG_BUILDING_DEEMON
/* Check if the current execution context allows access to `self',
 * which is either an instance or class method of `impl_class' */
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL
class_attribute_mayaccess_impl(struct Dee_class_attribute *__restrict self,
                               DeeTypeObject *__restrict impl_class);
#define class_attribute_mayaccess(self, impl_class)   \
	(!((self)->ca_flag & CLASS_ATTRIBUTE_FPRIVATE) || \
	 class_attribute_mayaccess_impl(self, impl_class))
#endif /* CONFIG_BUILDING_DEEMON */


/* A special operator invoked using the same
 * arguments as passed to the class constructor.
 * It should return a sequence that is then cast to a tuple
 * before being used as an argument list for invoking the
 * constructor of the base-class.
 * WARNING: Unlike all other class operators, this one
 *          is _NOT_ called using thiscall conventions,
 *          but instead as a regular function.
 *          This is necessary because at the time that
 *          it is called, the instance then used as
 *          this-argument hasn't yet been initialized.
 * NOTE: When this operator isn't set, super-classes are
 *       always initialized using their default-constructor,
 *       essentially passing an empty argument list. */
#ifdef DEE_SOURCE
#define CLASS_OPERATOR_SUPERARGS     (Dee_OPERATOR_USERCOUNT + 0)
#define CLASS_OPERATOR_PRINT         (Dee_OPERATOR_USERCOUNT + 1) /* `operator str(fp: File): none' */
#define CLASS_OPERATOR_PRINTREPR     (Dee_OPERATOR_USERCOUNT + 2) /* `operator repr(fp: File): none' */
#define CLASS_OPERATOR_USERCOUNT     (Dee_OPERATOR_USERCOUNT + 3)
#endif /* DEE_SOURCE */
#define Dee_CLASS_OPERATOR_USERCOUNT (Dee_OPERATOR_USERCOUNT + 3)


struct Dee_class_operator {
	/* TODO: It must be possible to define operators by-name (as in: "string"), and have the
	 *       `DeeClass_New()' resolve that name using `DeeTypeType_GetOperatorById(Dee_TYPE(base))' */
	Dee_operator_t co_name; /* [const] Operator name (`(Dee_operator_t)-1' for end-of-chain) */
	uint16_t       co_addr; /* [const] Operator address (within the class member table `cd_members').
	                         * Operators are invoked like attributes with the following flags:
	                         * `Dee_CLASS_ATTRIBUTE_FMETHOD|Dee_CLASS_ATTRIBUTE_FCLASSMEM', meaning
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


struct Dee_class_descriptor_object {
	/* The descriptor for the configuration of a user-defined class object:
	 * >> class MyClass {
	 * >>     static function cfunc() {
	 * >>         print "static function";
	 * >>     }
	 * >>     function ifunc() {
	 * >>         print "instance function";
	 * >>     }
	 * >>     static member cmember = "static member";
	 * >>     member imember = "instance member";
	 * >>     static property cprop = {
	 * >>         get()  { return "static member"; }
	 * >>         del()  { print "static member"; }
	 * >>         set(v) { print "static member"; }
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
	 * >>         [0]                    = <function cfunc() { print "static function"; }>,
	 * >>         [1]                    = <cmember = "static member">,
	 * >>         [2 + CLASS_GETSET_GET] = <cprop:get: get()  { return "static member"; }>,
	 * >>         [2 + CLASS_GETSET_DEL] = <cprop:del: del()  { print "static member"; }>,
	 * >>         [2 + CLASS_GETSET_SET] = <cprop:set: set(v) { print "static member"; }>,
	 * >>         [5]                    = <function ifunc() { print "instance function"; }>,
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
	Dee_OBJECT_HEAD
	DREF struct Dee_string_object                      *cd_name;        /* [0..1][const] Name of the class. */
	DREF struct Dee_string_object                      *cd_doc;         /* [0..1][const] Documentation strings for the class itself, and its operators. */
#ifdef DEE_SOURCE
#define CLASS_TP_FAUTOINIT                              Dee_TP_FGC      /* FLAG: When set, the construction operator is implemented to automatically initialize
                                                                         *       class members in compliance to the `this = default;' constructor definition.
                                                                         *       Additionally, if not already defined by the caller, this flag also causes
                                                                         *       `operator repr' to be implemented (see above).
                                                                         * NOTE: This flag should not be used together with `TP_FINHERITCTOR' */
#define CLASS_TP_FSUPERKWDS                             Dee_TP_FHEAP    /* FLAG: When set, the superargs operator actually returns a tuple `(args, kwds)' which
                                                                         *       should then be used to invoke the super-constructor as `super(args..., **kwds)'
                                                                         *       Otherwise, `args' is returned, and the super-constructor is called as `super(args...)' */
#endif /* DEE_SOURCE */
	uint16_t                                            cd_flags;       /* [const] Additional flags to set for the resulting type (set of `TP_F*').
	                                                                     * NOTE: The `TP_FINHERITCTOR' flag has special meaning here,
	                                                                     *       in that its presence causes `CLASS_OPERATOR_SUPERARGS'
	                                                                     *       to be implemented such that it forwards all arguments
	                                                                     *       to the underlying base-type, while also implementing
	                                                                     *       `OPERATOR_CONSTRUCTOR' as a no-op for any number of
	                                                                     *       arguments.
	                                                                     *       If the user overrides `CLASS_OPERATOR_SUPERARGS',
	                                                                     *       the `TP_FINHERITCTOR' flag is simply ignored.
	                                                                     *       If the user overrides `OPERATOR_CONSTRUCTOR',
	                                                                     *       the user's constructor will be invoked, though
	                                                                     *       no arguments will be passed to it (this is done to
	                                                                     *       allow for class member initialization to still take
	                                                                     *       place when no constructor has actually been defined). */
	uint16_t                                            cd_cmemb_size;  /* [const] The allocation size of the class member table. */
	uint16_t                                            cd_imemb_size;  /* [const] The allocation size of the instance member table. */
	Dee_operator_t                                      cd_clsop_mask;  /* [const] Mask for the `cd_clsop_list' hash-vector. */
	size_t                                              cd_cattr_mask;  /* [const] Mask for the `cd_cattr_list' hash-vector. */
	size_t                                              cd_iattr_mask;  /* [const] Mask for the `cd_cattr_list' hash-vector. */
	struct Dee_class_operator                          *cd_clsop_list;  /* [1..cd_clsop_mask+1][owned_if(!= INTERNAL(empty-class-operator-table))]
	                                                                     * [const] The class operator address hash-vector. */
	struct Dee_class_attribute                         *cd_cattr_list;  /* [1..cd_cattr_mask+1][owned_if(!= INTERNAL(empty-class-attribute-table))]
	                                                                     * [const] The class attribute hash-vector. */
	COMPILER_FLEXIBLE_ARRAY(struct Dee_class_attribute, cd_iattr_list); /* [cd_iattr_mask+1] The instance attribute hash-vector. */
};
#define DeeClassDescriptor_CLSOPNEXT(i, perturb) ((i) = (((i) << 2) + (i) + (perturb) + 1), (perturb) >>= 5)
#define DeeClassDescriptor_CATTRNEXT(i, perturb) ((i) = (((i) << 2) + (i) + (perturb) + 1), (perturb) >>= 5)
#define DeeClassDescriptor_IATTRNEXT(i, perturb) ((i) = (((i) << 2) + (i) + (perturb) + 1), (perturb) >>= 5)


DDATDEF DeeTypeObject DeeClassDescriptor_Type;
#define DeeClassDescriptor_Check(x)      DeeObject_InstanceOfExact(x, &DeeClassDescriptor_Type) /* DeeClassDescriptor_Type is final */
#define DeeClassDescriptor_CheckExact(x) DeeObject_InstanceOfExact(x, &DeeClassDescriptor_Type)


/* Lookup class / instance attributes within the given class descriptor.
 * @return: * :   A pointer to attribute that was found.
 * @return: NULL: Attribute could not be found (no error is thrown) */
DFUNDEF WUNUSED NONNULL((1, 2)) struct Dee_class_attribute *(DCALL DeeClassDescriptor_QueryClassAttributeHash)(DeeClassDescriptorObject *self, /*String*/ DeeObject *attr, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2)) struct Dee_class_attribute *(DCALL DeeClassDescriptor_QueryClassAttributeStringHash)(DeeClassDescriptorObject *__restrict self, char const *__restrict attr, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2)) struct Dee_class_attribute *(DCALL DeeClassDescriptor_QueryClassAttributeStringLenHash)(DeeClassDescriptorObject *__restrict self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2)) struct Dee_class_attribute *(DCALL DeeClassDescriptor_QueryInstanceAttributeHash)(DeeClassDescriptorObject *self, /*String*/ DeeObject *attr, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2)) struct Dee_class_attribute *(DCALL DeeClassDescriptor_QueryInstanceAttributeStringHash)(DeeClassDescriptorObject *__restrict self, char const *__restrict attr, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2)) struct Dee_class_attribute *(DCALL DeeClassDescriptor_QueryInstanceAttributeStringLenHash)(DeeClassDescriptorObject *__restrict self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash);
#ifdef __INTELLISENSE__
DFUNDEF WUNUSED NONNULL((1, 2)) struct Dee_class_attribute *(DCALL DeeClassDescriptor_QueryClassAttribute)(DeeClassDescriptorObject *self, /*String*/ DeeObject *attr);
DFUNDEF WUNUSED NONNULL((1, 2)) struct Dee_class_attribute *(DCALL DeeClassDescriptor_QueryClassAttributeString)(DeeClassDescriptorObject *__restrict self, char const *__restrict attr);
DFUNDEF WUNUSED NONNULL((1, 2)) struct Dee_class_attribute *(DCALL DeeClassDescriptor_QueryClassAttributeStringLen)(DeeClassDescriptorObject *__restrict self, char const *__restrict attr, size_t attrlen);
DFUNDEF WUNUSED NONNULL((1, 2)) struct Dee_class_attribute *(DCALL DeeClassDescriptor_QueryInstanceAttribute)(DeeClassDescriptorObject *self, /*String*/ DeeObject *attr);
DFUNDEF WUNUSED NONNULL((1, 2)) struct Dee_class_attribute *(DCALL DeeClassDescriptor_QueryInstanceAttributeString)(DeeClassDescriptorObject *__restrict self, char const *__restrict attr);
DFUNDEF WUNUSED NONNULL((1, 2)) struct Dee_class_attribute *(DCALL DeeClassDescriptor_QueryInstanceAttributeStringLen)(DeeClassDescriptorObject *__restrict self, char const *__restrict attr, size_t attrlen);
#else /* __INTELLISENSE__ */
#define DeeClassDescriptor_QueryClassAttribute(self, attr) \
	DeeClassDescriptor_QueryClassAttributeHash(self, attr, DeeString_Hash(attr))
#define DeeClassDescriptor_QueryInstanceAttribute(self, attr) \
	DeeClassDescriptor_QueryInstanceAttributeHash(self, attr, DeeString_Hash(attr))
#define DeeClassDescriptor_QueryClassAttributeString(self, attr) \
	DeeClassDescriptor_QueryClassAttributeStringHash(self, attr, Dee_HashStr(attr))
#define DeeClassDescriptor_QueryInstanceAttributeString(self, attr) \
	DeeClassDescriptor_QueryInstanceAttributeStringHash(self, attr, Dee_HashStr(attr))
#define DeeClassDescriptor_QueryClassAttributeStringLen(self, attr, attrlen) \
	DeeClassDescriptor_QueryClassAttributeStringLenHash(self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeClassDescriptor_QueryInstanceAttributeStringLen(self, attr, attrlen) \
	DeeClassDescriptor_QueryInstanceAttributeStringLenHash(self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#endif /* !__INTELLISENSE__ */



#define Dee_CLASS_HEADER_OPC1 8
#define Dee_CLASS_HEADER_OPC2 ((Dee_CLASS_OPERATOR_USERCOUNT + 7) / 8)

#if (Dee_CLASS_HEADER_OPC1 * Dee_CLASS_HEADER_OPC2) < Dee_CLASS_OPERATOR_USERCOUNT
#error "FIXME: Not enough space for all available operators"
#endif /* (Dee_CLASS_HEADER_OPC1 * Dee_CLASS_HEADER_OPC2) < Dee_CLASS_OPERATOR_USERCOUNT */

struct Dee_class_optable {
	/* [0..1][lock(READ(:cd_lock), SET_TO_NULL(:cd_lock),WRITE_ONCE)][*] Table of operators.
	 * NOTE: Individual callback objects may be `ITER_DONE',
	 *       indicative of that operator having been deleted
	 *       explicitly. */
	DREF DeeObject *co_operators[Dee_CLASS_HEADER_OPC2];
};

struct Dee_class_desc {
	/* The class description tail embedded into type-objects
	 * which have been initialized as custom user-classes. */
	DREF DeeClassDescriptorObject            *cd_desc;     /* [1..1][const] The associated class descriptor.
	                                                        * This in turn contains all the relevant fields
	                                                        * required to accessing user-defined attributes. */
	uintptr_t                                 cd_offset;   /* [const] Offset to the `struct Dee_instance_desc' of instances. */
	struct Dee_class_optable                 *cd_ops[Dee_CLASS_HEADER_OPC1];
	                                                       /* [0..1][owned][lock(WRITE_ONCE)][*]
	                                                        * Table of cached operator callbacks. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t                       cd_lock;     /* Lock for accessing the class member table. */
#endif /* !CONFIG_NO_THREADS */
	COMPILER_FLEXIBLE_ARRAY(DREF DeeObject *, cd_members); /* [0..1][lock(cd_lock)][cd_desc->cd_cmemb_size]
	                                                        * The class member table (also contains
	                                                        * instance-methods and operator callbacks). */
};

#define Dee_class_desc_lock_reading(self)    Dee_atomic_rwlock_reading(&(self)->cd_lock)
#define Dee_class_desc_lock_writing(self)    Dee_atomic_rwlock_writing(&(self)->cd_lock)
#define Dee_class_desc_lock_tryread(self)    Dee_atomic_rwlock_tryread(&(self)->cd_lock)
#define Dee_class_desc_lock_trywrite(self)   Dee_atomic_rwlock_trywrite(&(self)->cd_lock)
#define Dee_class_desc_lock_canread(self)    Dee_atomic_rwlock_canread(&(self)->cd_lock)
#define Dee_class_desc_lock_canwrite(self)   Dee_atomic_rwlock_canwrite(&(self)->cd_lock)
#define Dee_class_desc_lock_waitread(self)   Dee_atomic_rwlock_waitread(&(self)->cd_lock)
#define Dee_class_desc_lock_waitwrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->cd_lock)
#define Dee_class_desc_lock_read(self)       Dee_atomic_rwlock_read(&(self)->cd_lock)
#define Dee_class_desc_lock_write(self)      Dee_atomic_rwlock_write(&(self)->cd_lock)
#define Dee_class_desc_lock_tryupgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->cd_lock)
#define Dee_class_desc_lock_upgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->cd_lock)
#define Dee_class_desc_lock_downgrade(self)  Dee_atomic_rwlock_downgrade(&(self)->cd_lock)
#define Dee_class_desc_lock_endwrite(self)   Dee_atomic_rwlock_endwrite(&(self)->cd_lock)
#define Dee_class_desc_lock_endread(self)    Dee_atomic_rwlock_endread(&(self)->cd_lock)
#define Dee_class_desc_lock_end(self)        Dee_atomic_rwlock_end(&(self)->cd_lock)

#ifndef CONFIG_NO_THREADS
#define Dee_class_desc_as_instance(x) ((struct Dee_instance_desc *)&(x)->cd_lock)
#else /* !CONFIG_NO_THREADS */
#define Dee_class_desc_as_instance(x) ((struct Dee_instance_desc *)&(x)->cd_members[0])
#endif /* CONFIG_NO_THREADS */

#define DeeClass_Check(self)    (DeeType_Check(self) && DeeType_IsClass(self))
#define DeeInstance_Check(self)  DeeType_IsClass(Dee_TYPE(self))

/* Returns the descriptor for a given class. */
#define DeeClass_DESC(self)                                                      \
	(ASSERT_OBJECT_TYPE(self, &DeeType_Type), Dee_ASSERT(DeeType_IsClass(self)), \
	 ((DeeTypeObject *)Dee_REQUIRES_OBJECT(self))->tp_class)


#define DeeClassDesc_QueryClassAttribute(self, attr)                                DeeClassDescriptor_QueryClassAttribute((self)->cd_desc, attr)
#define DeeClassDesc_QueryClassAttributeHash(self, attr, hash)                      DeeClassDescriptor_QueryClassAttributeHash((self)->cd_desc, attr, hash)
#define DeeClassDesc_QueryClassAttributeString(self, attr)                          DeeClassDescriptor_QueryClassAttributeString((self)->cd_desc, attr)
#define DeeClassDesc_QueryClassAttributeStringLen(self, attr, attrlen)              DeeClassDescriptor_QueryClassAttributeStringLen((self)->cd_desc, attr, attrlen)
#define DeeClassDesc_QueryClassAttributeStringHash(self, attr, hash)                DeeClassDescriptor_QueryClassAttributeStringHash((self)->cd_desc, attr, hash)
#define DeeClassDesc_QueryClassAttributeStringLenHash(self, attr, attrlen, hash)    DeeClassDescriptor_QueryClassAttributeStringLenHash((self)->cd_desc, attr, attrlen, hash)
#define DeeClassDesc_QueryInstanceAttribute(self, attr)                             DeeClassDescriptor_QueryInstanceAttribute((self)->cd_desc, attr)
#define DeeClassDesc_QueryInstanceAttributeHash(self, attr, hash)                   DeeClassDescriptor_QueryInstanceAttributeHash((self)->cd_desc, attr, hash)
#define DeeClassDesc_QueryInstanceAttributeString(self, attr)                       DeeClassDescriptor_QueryInstanceAttributeString((self)->cd_desc, attr)
#define DeeClassDesc_QueryInstanceAttributeStringLen(self, attr, attrlen)           DeeClassDescriptor_QueryInstanceAttributeStringLen((self)->cd_desc, attr, attrlen)
#define DeeClassDesc_QueryInstanceAttributeStringHash(self, attr, hash)             DeeClassDescriptor_QueryInstanceAttributeStringHash((self)->cd_desc, attr, hash)
#define DeeClassDesc_QueryInstanceAttributeStringLenHash(self, attr, attrlen, hash) DeeClassDescriptor_QueryInstanceAttributeStringLenHash((self)->cd_desc, attr, attrlen, hash)

#define DeeClass_QueryClassAttribute(self, attr)                                DeeClassDesc_QueryClassAttribute(DeeClass_DESC(self), attr)
#define DeeClass_QueryClassAttributeHash(self, attr, hash)                      DeeClassDesc_QueryClassAttributeHash(DeeClass_DESC(self), attr, hash)
#define DeeClass_QueryClassAttributeString(self, attr)                          DeeClassDesc_QueryClassAttributeString(DeeClass_DESC(self), attr)
#define DeeClass_QueryClassAttributeStringLen(self, attr, attrlen)              DeeClassDesc_QueryClassAttributeStringLen(DeeClass_DESC(self), attr, attrlen)
#define DeeClass_QueryClassAttributeStringHash(self, attr, hash)                DeeClassDesc_QueryClassAttributeStringHash(DeeClass_DESC(self), attr, hash)
#define DeeClass_QueryClassAttributeStringLenHash(self, attr, attrlen, hash)    DeeClassDesc_QueryClassAttributeStringLenHash(DeeClass_DESC(self), attr, attrlen, hash)
#define DeeClass_QueryInstanceAttribute(self, attr)                             DeeClassDesc_QueryInstanceAttribute(DeeClass_DESC(self), attr)
#define DeeClass_QueryInstanceAttributeHash(self, attr, hash)                   DeeClassDesc_QueryInstanceAttributeHash(DeeClass_DESC(self), attr, hash)
#define DeeClass_QueryInstanceAttributeString(self, attr)                       DeeClassDesc_QueryInstanceAttributeString(DeeClass_DESC(self), attr)
#define DeeClass_QueryInstanceAttributeStringLen(self, attr, attrlen)           DeeClassDesc_QueryInstanceAttributeStringLen(DeeClass_DESC(self), attr, attrlen)
#define DeeClass_QueryInstanceAttributeStringHash(self, attr, hash)             DeeClassDesc_QueryInstanceAttributeStringHash(DeeClass_DESC(self), attr, hash)
#define DeeClass_QueryInstanceAttributeStringLenHash(self, attr, attrlen, hash) DeeClassDesc_QueryInstanceAttributeStringLenHash(DeeClass_DESC(self), attr, attrlen, hash)


struct Dee_instance_desc {
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t                       id_lock;  /* Lock that must be held when accessing  */
#endif /* !CONFIG_NO_THREADS */
	COMPILER_FLEXIBLE_ARRAY(DREF DeeObject *, id_vtab); /* [0..1][lock(id_lock)]
	                                                     * [DeeClass_DESC(:ob_type)->cd_desc->cd_imemb_size]
	                                                     * Instance member table. */
};

#define Dee_instance_desc_lock_reading(self)    Dee_atomic_rwlock_reading(&(self)->id_lock)
#define Dee_instance_desc_lock_writing(self)    Dee_atomic_rwlock_writing(&(self)->id_lock)
#define Dee_instance_desc_lock_tryread(self)    Dee_atomic_rwlock_tryread(&(self)->id_lock)
#define Dee_instance_desc_lock_trywrite(self)   Dee_atomic_rwlock_trywrite(&(self)->id_lock)
#define Dee_instance_desc_lock_canread(self)    Dee_atomic_rwlock_canread(&(self)->id_lock)
#define Dee_instance_desc_lock_canwrite(self)   Dee_atomic_rwlock_canwrite(&(self)->id_lock)
#define Dee_instance_desc_lock_waitread(self)   Dee_atomic_rwlock_waitread(&(self)->id_lock)
#define Dee_instance_desc_lock_waitwrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->id_lock)
#define Dee_instance_desc_lock_read(self)       Dee_atomic_rwlock_read(&(self)->id_lock)
#define Dee_instance_desc_lock_write(self)      Dee_atomic_rwlock_write(&(self)->id_lock)
#define Dee_instance_desc_lock_tryupgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->id_lock)
#define Dee_instance_desc_lock_upgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->id_lock)
#define Dee_instance_desc_lock_downgrade(self)  Dee_atomic_rwlock_downgrade(&(self)->id_lock)
#define Dee_instance_desc_lock_endwrite(self)   Dee_atomic_rwlock_endwrite(&(self)->id_lock)
#define Dee_instance_desc_lock_endread(self)    Dee_atomic_rwlock_endread(&(self)->id_lock)
#define Dee_instance_desc_lock_end(self)        Dee_atomic_rwlock_end(&(self)->id_lock)

#define DeeInstance_DESC(class_descriptor, self) \
	((struct Dee_instance_desc *)((uintptr_t)Dee_REQUIRES_OBJECT(self) + (class_descriptor)->cd_offset))


/* Get/Call/Del/Set an instance attribute, as acquired
 * through `DeeClassDescriptor_QueryInstanceAttribute()'. */
DFUNDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL
DeeInstance_GetAttribute(struct Dee_class_desc *__restrict desc,
                         struct Dee_instance_desc *__restrict self,
                         DeeObject *__restrict this_arg,
                         struct Dee_class_attribute const *__restrict attr);
DFUNDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
DeeInstance_BoundAttribute(struct Dee_class_desc *__restrict desc,
                           struct Dee_instance_desc *__restrict self,
                           DeeObject *__restrict this_arg,
                           struct Dee_class_attribute const *__restrict attr);
DFUNDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL
DeeInstance_CallAttribute(struct Dee_class_desc *__restrict desc,
                          struct Dee_instance_desc *__restrict self,
                          DeeObject *this_arg,
                          struct Dee_class_attribute const *__restrict attr,
                          size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL
DeeInstance_VCallAttributef(struct Dee_class_desc *__restrict desc,
                            struct Dee_instance_desc *__restrict self,
                            DeeObject *this_arg,
                            struct Dee_class_attribute const *__restrict attr,
                            char const *__restrict format, va_list args);
PUBLIC WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL
DeeInstance_CallAttributeKw(struct Dee_class_desc *__restrict desc,
                            struct Dee_instance_desc *__restrict self,
                            DeeObject *this_arg,
                            struct Dee_class_attribute const *__restrict attr,
                            size_t argc, DeeObject *const *argv, DeeObject *kw);
DFUNDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *
(DCALL DeeInstance_CallAttributeTuple)(struct Dee_class_desc *__restrict desc,
                                       struct Dee_instance_desc *__restrict self,
                                       DeeObject *this_arg,
                                       struct Dee_class_attribute const *__restrict attr,
                                       DeeObject *args);
DFUNDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *
(DCALL DeeInstance_CallAttributeTupleKw)(struct Dee_class_desc *__restrict desc,
                                         struct Dee_instance_desc *__restrict self,
                                         DeeObject *this_arg,
                                         struct Dee_class_attribute const *__restrict attr,
                                         DeeObject *args, DeeObject *kw);
#if !defined(CONFIG_CALLTUPLE_OPTIMIZATIONS) && !defined(__OPTIMIZE_SIZE__)
#define DeeInstance_CallAttributeTuple(desc, self, this_arg, attr, args) \
	DeeInstance_CallAttribute(desc, self, this_arg, attr, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeInstance_CallAttributeTupleKw(desc, self, this_arg, attr, args, kw) \
	DeeInstance_CallAttributeKw(desc, self, this_arg, attr, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#endif /* !CONFIG_CALLTUPLE_OPTIMIZATIONS && !__OPTIMIZE_SIZE__ */

DFUNDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
DeeInstance_DelAttribute(struct Dee_class_desc *__restrict desc,
                         struct Dee_instance_desc *__restrict self,
                         DeeObject *__restrict this_arg,
                         struct Dee_class_attribute const *__restrict attr);
DFUNDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL
DeeInstance_SetAttribute(struct Dee_class_desc *__restrict desc,
                         struct Dee_instance_desc *__restrict self,
                         DeeObject *this_arg,
                         struct Dee_class_attribute const *__restrict attr,
                         DeeObject *value);


/* Instance member access (by addr) */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeInstance_GetMember(/*Class*/ DeeTypeObject *__restrict tp_self, /*Instance*/ DeeObject *__restrict self, uint16_t addr);
DFUNDEF WUNUSED NONNULL((1, 2)) bool DCALL DeeInstance_BoundMember(/*Class*/ DeeTypeObject *__restrict tp_self, /*Instance*/ DeeObject *__restrict self, uint16_t addr);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeInstance_DelMember(/*Class*/ DeeTypeObject *__restrict tp_self, /*Instance*/ DeeObject *__restrict self, uint16_t addr);
DFUNDEF NONNULL((1, 2, 4)) void DCALL DeeInstance_SetMember(/*Class*/ DeeTypeObject *tp_self, /*Instance*/ DeeObject *self, uint16_t addr, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 2, 4)) int DCALL DeeInstance_SetMemberInitial(/*Class*/ DeeTypeObject *tp_self, /*Instance*/ DeeObject *self, uint16_t addr, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeInstance_GetMemberSafe(DeeTypeObject *tp_self, DeeObject *__restrict self, uint16_t addr);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeInstance_BoundMemberSafe(DeeTypeObject *tp_self, DeeObject *__restrict self, uint16_t addr);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeInstance_DelMemberSafe(DeeTypeObject *tp_self, DeeObject *__restrict self, uint16_t addr);
DFUNDEF WUNUSED NONNULL((1, 2, 4)) int DCALL DeeInstance_SetMemberSafe(DeeTypeObject *tp_self, DeeObject *self, uint16_t addr, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 2, 4)) int DCALL DeeInstance_SetMemberInitialSafe(DeeTypeObject *tp_self, DeeObject *self, uint16_t addr, DeeObject *value);

/* Class member access (by addr) */
DFUNDEF NONNULL((1, 3)) void DCALL DeeClass_SetMember(DeeTypeObject *self, uint16_t addr, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 3)) int DCALL DeeClass_SetMemberSafe(DeeTypeObject *self, uint16_t addr, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeClass_GetMember(DeeTypeObject *__restrict self, uint16_t addr);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeClass_GetMemberSafe(DeeTypeObject *__restrict self, uint16_t addr);
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeClass_BoundMemberSafe(DeeTypeObject *__restrict self, uint16_t addr);
#define DeeClass_BoundMember(self, addr) (__hybrid_atomic_load(&DeeClass_DESC(self)->cd_members[addr], __ATOMIC_ACQUIRE) != NULL)


#ifdef CONFIG_BUILDING_DEEMON
struct attribute_info;
struct attribute_lookup_rules;

/* Enumerate user-defined class or instance attributes. */
INTDEF WUNUSED NONNULL((1, 2)) dssize_t DCALL
DeeClass_EnumClassAttributes(DeeTypeObject *__restrict self, denum_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 3)) dssize_t DCALL
DeeClass_EnumInstanceAttributes(DeeTypeObject *__restrict self,
                                DeeObject *instance, denum_t proc, void *arg);

/* Enumerate user-defined instance attributes, as
 * accessed by `DeeClass_GetInstanceAttribute()'. */
INTDEF WUNUSED NONNULL((1, 2)) dssize_t DCALL
DeeClass_EnumClassInstanceAttributes(DeeTypeObject *__restrict self, denum_t proc, void *arg);

/* Find a specific class-, instance- or
 * instance-through-class-attribute, matching the given lookup rules.
 * @return:  0: Attribute found (*result was filled with data).
 * @return:  1: No attribute matching the given requirements was found.
 * @return: -1: An error occurred. */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
DeeClass_FindClassAttribute(DeeTypeObject *tp_invoker, DeeTypeObject *self,
                            struct attribute_info *__restrict result,
                            struct attribute_lookup_rules const *__restrict rules);
INTDEF WUNUSED NONNULL((1, 2, 4, 5)) int DCALL
DeeClass_FindInstanceAttribute(DeeTypeObject *tp_invoker, DeeTypeObject *self, DeeObject *instance,
                               struct attribute_info *__restrict result,
                               struct attribute_lookup_rules const *__restrict rules);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
DeeClass_FindClassInstanceAttribute(DeeTypeObject *tp_invoker, DeeTypeObject *self,
                                    struct attribute_info *__restrict result,
                                    struct attribute_lookup_rules const *__restrict rules);

/* Used to initialize attributes in default constructor calls.
 * @return:  0: Basic attribute successfully set
 * @return: -1: An error occurred. */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
DeeInstance_SetBasicAttribute(struct Dee_class_desc *__restrict desc,
                              struct Dee_instance_desc *__restrict self,
                              struct Dee_class_attribute const *__restrict attr,
                              DeeObject *value);

/* Get/Call/Del/Set a class attribute, as acquired
 * through `DeeClassDescriptor_QueryClassAttribute()'. */
#ifdef __INTELLISENSE__
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeClass_GetClassAttribute(DeeTypeObject *__restrict class_type, struct Dee_class_attribute const *__restrict attr);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeClass_BoundClassAttribute(DeeTypeObject *__restrict class_type, struct Dee_class_attribute const *__restrict attr);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeClass_CallClassAttribute(DeeTypeObject *class_type, struct Dee_class_attribute const *__restrict attr, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeClass_CallClassAttributeKw(DeeTypeObject *class_type, struct Dee_class_attribute const *__restrict attr, size_t argc, DeeObject *const *argv, DeeObject *kw);
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeClass_CallClassAttributeTuple(DeeTypeObject *class_type, struct Dee_class_attribute const *__restrict attr, DeeObject *args);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeClass_CallClassAttributeTupleKw(DeeTypeObject *class_type, struct Dee_class_attribute const *__restrict attr, DeeObject *args, DeeObject *kw);
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeClass_VCallClassAttributef(DeeTypeObject *class_type, struct Dee_class_attribute const *__restrict attr, char const *__restrict format, va_list args);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeClass_DelClassAttribute(DeeTypeObject *__restrict class_type, struct Dee_class_attribute const *__restrict attr);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeClass_SetClassAttribute(DeeTypeObject *class_type, struct Dee_class_attribute const *__restrict attr, DeeObject *value);
#else /* __INTELLISENSE__ */
#define DeeClass_GetClassAttribute(class_type, attr)                    DeeInstance_GetAttribute(DeeClass_DESC(class_type), class_desc_as_instance(DeeClass_DESC(class_type)), (DeeObject *)(class_type), attr)
#define DeeClass_BoundClassAttribute(class_type, attr)                  DeeInstance_BoundAttribute(DeeClass_DESC(class_type), class_desc_as_instance(DeeClass_DESC(class_type)), (DeeObject *)(class_type), attr)
#define DeeClass_CallClassAttribute(class_type, attr, argc, argv)       DeeInstance_CallAttribute(DeeClass_DESC(class_type), class_desc_as_instance(DeeClass_DESC(class_type)), (DeeObject *)(class_type), attr, argc, argv)
#define DeeClass_CallClassAttributeKw(class_type, attr, argc, argv, kw) DeeInstance_CallAttributeKw(DeeClass_DESC(class_type), class_desc_as_instance(DeeClass_DESC(class_type)), (DeeObject *)(class_type), attr, argc, argv, kw)
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
#define DeeClass_CallClassAttributeTuple(class_type, attr, args)        DeeInstance_CallAttributeTuple(DeeClass_DESC(class_type), class_desc_as_instance(DeeClass_DESC(class_type)), (DeeObject *)(class_type), attr, args)
#define DeeClass_CallClassAttributeTupleKw(class_type, attr, args, kw)  DeeInstance_CallAttributeTupleKw(DeeClass_DESC(class_type), class_desc_as_instance(DeeClass_DESC(class_type)), (DeeObject *)(class_type), attr, args, kw)
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
#define DeeClass_VCallClassAttributef(class_type, attr, format, args)   DeeInstance_VCallAttributef(DeeClass_DESC(class_type), class_desc_as_instance(DeeClass_DESC(class_type)), (DeeObject *)(class_type), attr, format, args)
#define DeeClass_DelClassAttribute(class_type, attr)                    DeeInstance_DelAttribute(DeeClass_DESC(class_type), class_desc_as_instance(DeeClass_DESC(class_type)), (DeeObject *)(class_type), attr)
#define DeeClass_SetClassAttribute(class_type, attr, value)             DeeInstance_SetAttribute(DeeClass_DESC(class_type), class_desc_as_instance(DeeClass_DESC(class_type)), (DeeObject *)(class_type), attr, value)
#endif /* !__INTELLISENSE__ */

/* Get/Call/Del/Set a class attribute, as acquired
 * through `DeeClassDescriptor_QueryInstanceAttribute()'.
 * These functions produce and interact with with proxy
 * objects constructed when accessing instance attributes
 * through their defining class:
 * >> class MyClass {
 * >>     public class member class_field = 84;
 * >>     public field = 42;
 * >>     function foo() {
 * >>         print "foo():", field;
 * >>     }
 * >> }
 * >> local x = MyClass();
 * >> print x.field;                 // DeeInstance_GetAttribute("field")
 * >> x.foo();                       // DeeInstance_CallAttribute("foo")
 * >> print MyClass.class_field;     // DeeClass_GetClassAttribute("class_field")
 * >> myclass_field = MyClass.field; // DeeClass_GetInstanceAttribute("field")
 */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeClass_GetInstanceAttribute(DeeTypeObject *__restrict class_type, struct Dee_class_attribute const *__restrict attr);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeClass_CallInstanceAttribute(DeeTypeObject *class_type, struct Dee_class_attribute const *__restrict attr, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeClass_CallInstanceAttributeKw(DeeTypeObject *class_type, struct Dee_class_attribute const *__restrict attr, size_t argc, DeeObject *const *argv, DeeObject *kw);
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeClass_CallInstanceAttributeTuple(DeeTypeObject *class_type, struct Dee_class_attribute const *__restrict attr, DeeObject *args);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeClass_CallInstanceAttributeTupleKw(DeeTypeObject *class_type, struct Dee_class_attribute const *__restrict attr, DeeObject *args, DeeObject *kw);
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeClass_VCallInstanceAttributef(DeeTypeObject *class_type, struct Dee_class_attribute const *__restrict attr, char const *__restrict format, va_list args);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeClass_DelInstanceAttribute(DeeTypeObject *__restrict class_type, struct Dee_class_attribute const *__restrict attr);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeClass_SetInstanceAttribute(DeeTypeObject *class_type, struct Dee_class_attribute const *__restrict attr, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeClass_BoundInstanceAttribute(DeeTypeObject *__restrict class_type, struct Dee_class_attribute const *__restrict attr);
#define DeeClass_SetBasicInstanceAttribute DeeClass_SetInstanceAttribute
#endif /* CONFIG_BUILDING_DEEMON */


struct Dee_module_object;

/* Create a new class type derived from `bases',
 * featuring traits from `descriptor'.
 * @param: bases: The base(s) of the resulting class.
 *                You may pass `Dee_None' to have the resulting
 *                class not be derived from anything (be base-less).
 *                You may also pass a sequence of types, in which
 *                case this sequence (and its order) describe the
 *                class's top-level MRO (thus becoming its __bases__).
 * @param: descriptor: A `DeeClassDescriptor_Type'-object, detailing the class's prototype.
 * @param: declaring_module: When non-NULL, the module that gets stored in `tp_module'
 *                           NOTE: Passing NULL here must be allowed for situations where
 *                                 code is executing without having a module-context (as
 *                                 is the case for code running in a JIT-context)
 * @throw: TypeError: The given `base' is neither `none', nor a type-object.
 * @throw: TypeError: The given `base' is a final or variable type. */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeTypeObject *DCALL
DeeClass_New(DeeObject *bases, DeeObject *descriptor,
             struct Dee_module_object *declaring_module);

/* Return the nearest operator function for `name',
 * implemented by `self', which must be a class type.
 * If the operator doesn't exist, return NULL and throw
 * a NotImplemented error, or return NULL and don't throw
 * an error when `DeeClass_TryGetOperator()' was used. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeClass_GetOperator(DeeTypeObject const *__restrict self, Dee_operator_t name);

/* Same as `DeeClass_GetOperator()', but don't simply return `NULL'
 * if the operator hasn't been implemented, and `ITER_DONE' when it
 * has been, but wasn't assigned anything. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeClass_TryGetOperator(DeeTypeObject const *__restrict self, Dee_operator_t name);

/* Same as `DeeClass_TryGetOperator()', but don't return an operator
 * that has been inherited from a base-class, but return `NULL' instead. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeClass_TryGetPrivateOperator(DeeTypeObject const *__restrict self, Dee_operator_t name);

/* Convenience wrappers for `DeeObject_ThisCall(DeeClass_GetOperator())' */
DFUNDEF WUNUSED ATTR_INS(5, 4) NONNULL((1, 2)) DREF DeeObject *DCALL
DeeClass_CallOperator(DeeTypeObject const *__restrict tp_self, DeeObject *self,
                      Dee_operator_t name, size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *
DeeClass_CallOperatorf(DeeTypeObject const *__restrict tp_self, DeeObject *self,
                       Dee_operator_t name, char const *format, ...);
DFUNDEF WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *DCALL
DeeClass_VCallOperatorf(DeeTypeObject const *__restrict tp_self, DeeObject *self,
                        Dee_operator_t name, char const *format, va_list args);


#ifdef CONFIG_BUILDING_DEEMON

/* Same as `DeeClass_TryGetPrivateOperator()', but don't return a reference */
INTDEF ATTR_PURE WUNUSED NONNULL((1)) DeeObject *DCALL
DeeClass_TryGetPrivateOperatorPtr(DeeTypeObject const *__restrict self, Dee_operator_t name);

/* The functions bound to the C-level type callbacks when a
 * user-defined class provides the associated operator.
 * All of the `instance_*' functions simply call the associated
 * `instance_t*' function, which the proceeds to load (and
 * potentially cache) the operator, before invoking it. */

/* `OPERATOR_CONSTRUCTOR' + `CLASS_OPERATOR_SUPERARGS' */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_super_tctor(DeeTypeObject *tp_self, DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_super_ctor(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_super_tinit(DeeTypeObject *tp_self, DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_super_init(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_super_tinitkw(DeeTypeObject *tp_self, DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_super_initkw(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_kwsuper_tctor(DeeTypeObject *tp_self, DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_kwsuper_ctor(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_kwsuper_tinit(DeeTypeObject *tp_self, DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_kwsuper_init(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_kwsuper_tinitkw(DeeTypeObject *tp_self, DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_kwsuper_initkw(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);

/* `CLASS_OPERATOR_SUPERARGS' */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_builtin_super_tctor(DeeTypeObject *tp_self, DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_builtin_super_ctor(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_builtin_super_tinit(DeeTypeObject *tp_self, DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_builtin_super_init(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_builtin_super_tinitkw(DeeTypeObject *tp_self, DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_builtin_super_initkw(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_builtin_kwsuper_tctor(DeeTypeObject *tp_self, DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_builtin_kwsuper_ctor(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_builtin_kwsuper_tinit(DeeTypeObject *tp_self, DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_builtin_kwsuper_init(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_builtin_kwsuper_tinitkw(DeeTypeObject *tp_self, DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_builtin_kwsuper_initkw(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);

/* `OPERATOR_CONSTRUCTOR' */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_tctor(DeeTypeObject *tp_self, DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_ctor(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_tinit(DeeTypeObject *tp_self, DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_init(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_tinitkw(DeeTypeObject *tp_self, DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_initkw(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);

#ifdef CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS
/* `OPERATOR_CONSTRUCTOR' (but the type doesn't have a base) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_nobase_tctor(DeeTypeObject *tp_self, DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_nobase_ctor(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_nobase_tinit(DeeTypeObject *tp_self, DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_nobase_init(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_nobase_tinitkw(DeeTypeObject *tp_self, DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_nobase_initkw(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
#endif /* CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS */

/* `OPERATOR_CONSTRUCTOR', with the `TP_FINHERITCTOR' flag set.
 * NOTE: These functions always invoke the user-defined constructor without any arguments! */
#define instance_inherited_tctor instance_tctor
#define instance_inherited_ctor  instance_ctor
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_inherited_tinit(DeeTypeObject *tp_self, DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_inherited_init(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_inherited_tinitkw(DeeTypeObject *tp_self, DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_inherited_initkw(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);

/* No predefined construction operators. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_builtin_tctor(DeeTypeObject *tp_self, DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_builtin_ctor(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_builtin_tinit(DeeTypeObject *tp_self, DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_builtin_init(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_builtin_tinitkw(DeeTypeObject *tp_self, DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_builtin_initkw(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);

#ifdef CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS
/* No predefined construction operators. (but the type doesn't have a base) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_builtin_nobase_tctor(DeeTypeObject *tp_self, DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_builtin_nobase_ctor(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_builtin_nobase_tinit(DeeTypeObject *tp_self, DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_builtin_nobase_init(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_builtin_nobase_tinitkw(DeeTypeObject *tp_self, DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_builtin_nobase_initkw(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
#endif /* CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS */

/* No predefined construction operators, but the `TP_FINHERITCTOR' flag is set. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_builtin_inherited_tctor(DeeTypeObject *tp_self, DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_builtin_inherited_ctor(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_builtin_inherited_tinit(DeeTypeObject *tp_self, DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_builtin_inherited_init(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_builtin_inherited_tinitkw(DeeTypeObject *tp_self, DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_builtin_inherited_initkw(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);

/* Builtin (pre-defined) hooks that are used when the user-class doesn't override these operators. */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_builtin_tcopy(DeeTypeObject *tp_self, DeeObject *__restrict self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_builtin_copy(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_builtin_tdeepload(DeeTypeObject *tp_self, DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_builtin_deepload(DeeObject *__restrict self);
#ifdef CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_builtin_nobase_tcopy(DeeTypeObject *tp_self, DeeObject *__restrict self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_builtin_nobase_copy(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_builtin_nobase_deepload(DeeObject *__restrict self);
#endif /* CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS */
INTDEF NONNULL((1)) void DCALL instance_builtin_destructor(DeeObject *__restrict self); /* No t-variant, because types are unwound automatically during destruction. */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_builtin_tassign(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_builtin_assign(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_builtin_tmoveassign(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_builtin_moveassign(DeeObject *self, DeeObject *other);

#ifdef CLASS_TP_FAUTOINIT
/* No predefined construction operators (with `CLASS_TP_FAUTOINIT'). */
#define instance_auto_tctor instance_tctor
#define instance_auto_ctor  instance_ctor
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_auto_tinit(DeeTypeObject *tp_self, DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_auto_init(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_auto_tinitkw(DeeTypeObject *tp_self, DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_auto_initkw(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
#define instance_builtin_auto_tctor instance_builtin_tctor
#define instance_builtin_auto_ctor  instance_builtin_ctor
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_builtin_auto_tinit(DeeTypeObject *tp_self, DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_builtin_auto_init(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_builtin_auto_tinitkw(DeeTypeObject *tp_self, DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_builtin_auto_initkw(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
#ifdef CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS
#define instance_auto_nobase_tctor instance_nobase_tctor
#define instance_auto_nobase_ctor  instance_nobase_ctor
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_auto_nobase_tinit(DeeTypeObject *tp_self, DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_auto_nobase_init(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_auto_nobase_tinitkw(DeeTypeObject *tp_self, DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_auto_nobase_initkw(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
#define instance_builtin_auto_nobase_tctor instance_builtin_nobase_tctor
#define instance_builtin_auto_nobase_ctor  instance_builtin_nobase_ctor
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_builtin_auto_nobase_tinit(DeeTypeObject *tp_self, DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_builtin_auto_nobase_init(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_builtin_auto_nobase_tinitkw(DeeTypeObject *tp_self, DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_builtin_auto_nobase_initkw(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
#endif /* CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS */
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL instance_builtin_auto_tprintrepr(DeeTypeObject *tp_self, DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL instance_builtin_auto_printrepr(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
#endif /* CLASS_TP_FAUTOINIT */


/* Hooks when the user-class overrides the associated operator. */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_tcopy(DeeTypeObject *tp_self, DeeObject *__restrict self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_copy(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_tdeepcopy(DeeTypeObject *tp_self, DeeObject *__restrict self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_deepcopy(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF NONNULL((1)) void DCALL instance_destructor(DeeObject *__restrict self); /* No t-variant, because types are unwound automatically during destruction. */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_tassign(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_assign(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_tmoveassign(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_moveassign(DeeObject *self, DeeObject *other);

/* GC support for class objects. */
INTDEF NONNULL((1, 2, 3)) void DCALL instance_tvisit(DeeTypeObject *tp_self, DeeObject *__restrict self, Dee_visit_t proc, void *arg);
INTDEF NONNULL((1, 2)) void DCALL instance_visit(DeeObject *__restrict self, Dee_visit_t proc, void *arg);
INTDEF NONNULL((1, 2)) void DCALL instance_tclear(DeeTypeObject *tp_self, DeeObject *__restrict self);
INTDEF NONNULL((1)) void DCALL instance_clear(DeeObject *__restrict self);
INTDEF NONNULL((1, 2)) void DCALL instance_tpclear(DeeTypeObject *tp_self, DeeObject *__restrict self, unsigned int gc_priority);
INTDEF NONNULL((1)) void DCALL instance_pclear(DeeObject *__restrict self, unsigned int gc_priority);
INTDEF struct type_gc Dee_tpconst instance_gc;

/* Builtin (standard) operators for hashing and comparing class objects. */
INTDEF WUNUSED NONNULL((1, 2)) Dee_hash_t DCALL instance_builtin_thash(DeeTypeObject *tp_self, DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL instance_builtin_hash(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL instance_builtin_teq(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_builtin_eq(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL instance_builtin_tne(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_builtin_ne(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL instance_builtin_tlo(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_builtin_lo(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL instance_builtin_tle(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_builtin_le(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL instance_builtin_tgr(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_builtin_gr(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL instance_builtin_tge(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_builtin_ge(DeeObject *self, DeeObject *other);
INTDEF struct type_cmp instance_builtin_cmp;

/* Hooks for user-defined operators. */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_tstr(DeeTypeObject *tp_self, DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL instance_str(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_trepr(DeeTypeObject *tp_self, DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL instance_repr(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL instance_tprint(DeeTypeObject *tp_self, DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL instance_print(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL instance_tprintrepr(DeeTypeObject *tp_self, DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL instance_printrepr(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_tstr_by_print(DeeTypeObject *tp_self, DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL instance_str_by_print(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_trepr_by_print(DeeTypeObject *tp_self, DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL instance_repr_by_print(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL instance_tprint_by_print(DeeTypeObject *tp_self, DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL instance_print_by_print(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL instance_tprintrepr_by_print(DeeTypeObject *tp_self, DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL instance_printrepr_by_print(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_tbool(DeeTypeObject *tp_self, DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_bool(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_tcall(DeeTypeObject *tp_self, DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL instance_call(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_tcallkw(DeeTypeObject *tp_self, DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL instance_callkw(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_tnext(DeeTypeObject *tp_self, DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL instance_next(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_tint(DeeTypeObject *tp_self, DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL instance_int(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_tdouble(DeeTypeObject *tp_self, DeeObject *__restrict self, double *__restrict result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_double(DeeObject *__restrict self, double *__restrict result);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_tinv(DeeTypeObject *tp_self, DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL instance_inv(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_tpos(DeeTypeObject *tp_self, DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL instance_pos(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_tneg(DeeTypeObject *tp_self, DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL instance_neg(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL instance_tadd(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_add(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL instance_tsub(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_sub(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL instance_tmul(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_mul(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL instance_tdiv(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_div(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL instance_tmod(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_mod(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL instance_tshl(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_shl(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL instance_tshr(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_shr(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL instance_tand(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_and(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL instance_tor(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_or(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL instance_txor(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_xor(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL instance_tpow(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_pow(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_tinc(DeeTypeObject *tp_self, DeeObject **__restrict p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_inc(DeeObject **__restrict p_self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_tdec(DeeTypeObject *tp_self, DeeObject **__restrict p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_dec(DeeObject **__restrict p_self);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_tiadd(DeeTypeObject *tp_self, DeeObject **__restrict p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_iadd(DeeObject **__restrict p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_tisub(DeeTypeObject *tp_self, DeeObject **__restrict p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_isub(DeeObject **__restrict p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_timul(DeeTypeObject *tp_self, DeeObject **__restrict p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_imul(DeeObject **__restrict p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_tidiv(DeeTypeObject *tp_self, DeeObject **__restrict p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_idiv(DeeObject **__restrict p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_timod(DeeTypeObject *tp_self, DeeObject **__restrict p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_imod(DeeObject **__restrict p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_tishl(DeeTypeObject *tp_self, DeeObject **__restrict p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_ishl(DeeObject **__restrict p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_tishr(DeeTypeObject *tp_self, DeeObject **__restrict p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_ishr(DeeObject **__restrict p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_tiand(DeeTypeObject *tp_self, DeeObject **__restrict p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_iand(DeeObject **__restrict p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_tior(DeeTypeObject *tp_self, DeeObject **__restrict p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_ior(DeeObject **__restrict p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_tixor(DeeTypeObject *tp_self, DeeObject **__restrict p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_ixor(DeeObject **__restrict p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_tipow(DeeTypeObject *tp_self, DeeObject **__restrict p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_ipow(DeeObject **__restrict p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) Dee_hash_t DCALL instance_thash(DeeTypeObject *tp_self, DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL instance_hash(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL instance_teq(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_eq(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL instance_tne(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_ne(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL instance_tlo(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_lo(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL instance_tle(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_le(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL instance_tgr(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_gr(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL instance_tge(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_ge(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_titer(DeeTypeObject *tp_self, DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL instance_iter(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_tsize(DeeTypeObject *tp_self, DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL instance_size(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL instance_tcontains(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_contains(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL instance_tgetitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_getitem(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_tdelitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_delitem(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL instance_tsetitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_setitem(DeeObject *self, DeeObject *other, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL instance_tgetrange(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL instance_getrange(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL instance_tdelrange(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_delrange(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL instance_tsetrange(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL instance_setrange(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_tenter(DeeTypeObject *tp_self, DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_enter(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_tleave(DeeTypeObject *tp_self, DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_leave(DeeObject *__restrict self);

INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL instance_tgetattr(DeeTypeObject *tp_self, DeeObject *self, /*String*/ DeeObject *name);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL instance_getattr(DeeObject *self, /*String*/ DeeObject *name);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_tdelattr(DeeTypeObject *tp_self, DeeObject *self, /*String*/ DeeObject *name);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_delattr(DeeObject *self, /*String*/ DeeObject *name);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL instance_tsetattr(DeeTypeObject *tp_self, DeeObject *self, /*String*/ DeeObject *name, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_setattr(DeeObject *self, /*String*/ DeeObject *name, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) dssize_t DCALL instance_enumattr(DeeTypeObject *tp_self, DeeObject *__restrict self, denum_t proc, void *arg);


#define DeeType_invoke_init_tp_assign_NODEFAULT(tp_self, tp_assign, self, other) \
	((tp_assign) == &instance_assign                                             \
	 ? instance_tassign(tp_self, self, other)                                    \
	 : (*(tp_assign))(self, other))
#define DeeType_invoke_init_tp_move_assign_NODEFAULT(tp_self, tp_move_assign, self, other) \
	((tp_move_assign) == &instance_moveassign                                              \
	 ? instance_tmoveassign(tp_self, self, other)                                          \
	 : (*(tp_move_assign))(self, other))
#define DeeType_invoke_cast_tp_str_NODEFAULT(tp_self, tp_str, self) \
	((tp_str) == &instance_str                                      \
	 ? instance_tstr(tp_self, self)                                 \
	 : (tp_str) == &instance_str_by_print                           \
	   ? instance_tstr_by_print(tp_self, self)                      \
	   : (*(tp_str))(self))
#define DeeType_invoke_cast_tp_print_NODEFAULT(tp_self, tp_print, self, printer, arg) \
	((tp_print) == &instance_print                                                    \
	 ? instance_tprint(tp_self, self, printer, arg)                                   \
	 : (tp_print) == &instance_print_by_print                                         \
	   ? instance_tprint_by_print(tp_self, self, printer, arg)                        \
	   : (*(tp_print))(self, printer, arg))
#define DeeType_invoke_cast_tp_repr_NODEFAULT(tp_self, tp_repr, self) \
	((tp_repr) == &instance_repr                                      \
	 ? instance_trepr(tp_self, self)                                  \
	 : (tp_repr) == &instance_repr_by_print                           \
	   ? instance_trepr_by_print(tp_self, self)                       \
	   : (*(tp_repr))(self))
#ifdef CLASS_TP_FAUTOINIT
#define DeeType_invoke_cast_tp_printrepr_NODEFAULT(tp_self, tp_printrepr, self, printer, arg) \
	((tp_printrepr) == &instance_printrepr                                                    \
	 ? instance_tprintrepr(tp_self, self, printer, arg)                                       \
	 : (tp_printrepr) == &instance_printrepr_by_print                                         \
	   ? instance_tprintrepr_by_print(tp_self, self, printer, arg)                            \
	   : (tp_printrepr) == &instance_builtin_auto_printrepr                                   \
	     ? instance_builtin_auto_tprintrepr(tp_self, self, printer, arg)                      \
	     : (*(tp_printrepr))(self, printer, arg))
#else /* CLASS_TP_FAUTOINIT */
#define DeeType_invoke_cast_tp_printrepr_NODEFAULT(tp_self, tp_printrepr, self, printer, arg) \
	((tp_printrepr) == &instance_printrepr                                                    \
	 ? instance_tprintrepr(tp_self, self, printer, arg)                                       \
	 : (tp_printrepr) == &instance_printrepr_by_print                                         \
	   ? instance_tprintrepr_by_print(tp_self, self, printer, arg)                            \
	   : (*(tp_printrepr))(self, printer, arg))
#endif /* !CLASS_TP_FAUTOINIT */
#define DeeType_invoke_cast_tp_bool_NODEFAULT(tp_self, tp_bool, self) \
	((tp_bool) == &instance_bool                                      \
	 ? instance_tbool(tp_self, self)                                  \
	 : (*(tp_bool))(self))
#define DeeType_invoke_tp_iter_next_NODEFAULT(tp_self, tp_iter_next, self) \
	((tp_iter_next) == &instance_next                                      \
	 ? instance_tnext(tp_self, self)                                       \
	 : (*(tp_iter_next))(self))
#define DeeType_invoke_tp_call_NODEFAULT(tp_self, tp_call, self, argc, argv) \
	((tp_call) == &instance_call                                             \
	 ? instance_tcall(tp_self, self, argc, argv)                             \
	 : (*(tp_call))(self, argc, argv))
#define DeeType_invoke_tp_call_kw_NODEFAULT(tp_self, tp_call_kw, self, argc, argv, kw) \
	((tp_call_kw) == &instance_callkw                                                  \
	 ? instance_tcallkw(tp_self, self, argc, argv, kw)                                 \
	 : (*(tp_call_kw))(self, argc, argv, kw))
#define DeeType_invoke_math_tp_int32_NODEFAULT(tp_self, tp_int32, self, result) (*(tp_int32))(self, result)
#define DeeType_invoke_math_tp_int64_NODEFAULT(tp_self, tp_int64, self, result) (*(tp_int64))(self, result)
#define DeeType_invoke_math_tp_double_NODEFAULT(tp_self, tp_double, self, result) \
	((tp_double) == &instance_double                                              \
	 ? instance_tdouble(tp_self, self, result)                                    \
	 : (*(tp_double))(self, result))
#define DeeType_invoke_math_tp_int_NODEFAULT(tp_self, tp_int, self) \
	((tp_int) == &instance_int                                      \
	 ? instance_tint(tp_self, self)                                 \
	 : (*(tp_int))(self))
#define DeeType_invoke_math_tp_inv_NODEFAULT(tp_self, tp_inv, self) \
	((tp_inv) == &instance_inv                                      \
	 ? instance_tinv(tp_self, self)                                 \
	 : (*(tp_inv))(self))
#define DeeType_invoke_math_tp_pos_NODEFAULT(tp_self, tp_pos, self) \
	((tp_pos) == &instance_pos                                      \
	 ? instance_tpos(tp_self, self)                                 \
	 : (*(tp_pos))(self))
#define DeeType_invoke_math_tp_neg_NODEFAULT(tp_self, tp_neg, self) \
	((tp_neg) == &instance_neg                                      \
	 ? instance_tneg(tp_self, self)                                 \
	 : (*(tp_neg))(self))
#define DeeType_invoke_math_tp_add_NODEFAULT(tp_self, tp_add, self, other) \
	((tp_add) == &instance_add                                             \
	 ? instance_tadd(tp_self, self, other)                                 \
	 : (*(tp_add))(self, other))
#define DeeType_invoke_math_tp_sub_NODEFAULT(tp_self, tp_sub, self, other) \
	((tp_sub) == &instance_sub                                             \
	 ? instance_tsub(tp_self, self, other)                                 \
	 : (*(tp_sub))(self, other))
#define DeeType_invoke_math_tp_mul_NODEFAULT(tp_self, tp_mul, self, other) \
	((tp_mul) == &instance_mul                                             \
	 ? instance_tmul(tp_self, self, other)                                 \
	 : (*(tp_mul))(self, other))
#define DeeType_invoke_math_tp_div_NODEFAULT(tp_self, tp_div, self, other) \
	((tp_div) == &instance_div                                             \
	 ? instance_tdiv(tp_self, self, other)                                 \
	 : (*(tp_div))(self, other))
#define DeeType_invoke_math_tp_mod_NODEFAULT(tp_self, tp_mod, self, other) \
	((tp_mod) == &instance_mod                                             \
	 ? instance_tmod(tp_self, self, other)                                 \
	 : (*(tp_mod))(self, other))
#define DeeType_invoke_math_tp_shl_NODEFAULT(tp_self, tp_shl, self, other) \
	((tp_shl) == &instance_shl                                             \
	 ? instance_tshl(tp_self, self, other)                                 \
	 : (*(tp_shl))(self, other))
#define DeeType_invoke_math_tp_shr_NODEFAULT(tp_self, tp_shr, self, other) \
	((tp_shr) == &instance_shr                                             \
	 ? instance_tshr(tp_self, self, other)                                 \
	 : (*(tp_shr))(self, other))
#define DeeType_invoke_math_tp_and_NODEFAULT(tp_self, tp_and, self, other) \
	((tp_and) == &instance_and                                             \
	 ? instance_tand(tp_self, self, other)                                 \
	 : (*(tp_and))(self, other))
#define DeeType_invoke_math_tp_or_NODEFAULT(tp_self, tp_or, self, other) \
	((tp_or) == &instance_or                                             \
	 ? instance_tor(tp_self, self, other)                                \
	 : (*(tp_or))(self, other))
#define DeeType_invoke_math_tp_xor_NODEFAULT(tp_self, tp_xor, self, other) \
	((tp_xor) == &instance_xor                                             \
	 ? instance_txor(tp_self, self, other)                                 \
	 : (*(tp_xor))(self, other))
#define DeeType_invoke_math_tp_pow_NODEFAULT(tp_self, tp_pow, self, other) \
	((tp_pow) == &instance_pow                                             \
	 ? instance_tpow(tp_self, self, other)                                 \
	 : (*(tp_pow))(self, other))
#define DeeType_invoke_math_tp_inc_NODEFAULT(tp_self, tp_inc, p_self) \
	((tp_inc) == &instance_inc                                        \
	 ? instance_tinc(tp_self, p_self)                                 \
	 : (*(tp_inc))(p_self))
#define DeeType_invoke_math_tp_dec_NODEFAULT(tp_self, tp_dec, p_self) \
	((tp_dec) == &instance_dec                                        \
	 ? instance_tdec(tp_self, p_self)                                 \
	 : (*(tp_dec))(p_self))
#define DeeType_invoke_math_tp_inplace_add_NODEFAULT(tp_self, tp_inplace_add, p_self, other) \
	((tp_inplace_add) == &instance_iadd                                                      \
	 ? instance_tiadd(tp_self, p_self, other)                                                \
	 : (*(tp_inplace_add))(p_self, other))
#define DeeType_invoke_math_tp_inplace_sub_NODEFAULT(tp_self, tp_inplace_sub, p_self, other) \
	((tp_inplace_sub) == &instance_isub                                                      \
	 ? instance_tisub(tp_self, p_self, other)                                                \
	 : (*(tp_inplace_sub))(p_self, other))
#define DeeType_invoke_math_tp_inplace_mul_NODEFAULT(tp_self, tp_inplace_mul, p_self, other) \
	((tp_inplace_mul) == &instance_imul                                                      \
	 ? instance_timul(tp_self, p_self, other)                                                \
	 : (*(tp_inplace_mul))(p_self, other))
#define DeeType_invoke_math_tp_inplace_div_NODEFAULT(tp_self, tp_inplace_div, p_self, other) \
	((tp_inplace_div) == &instance_idiv                                                      \
	 ? instance_tidiv(tp_self, p_self, other)                                                \
	 : (*(tp_inplace_div))(p_self, other))
#define DeeType_invoke_math_tp_inplace_mod_NODEFAULT(tp_self, tp_inplace_mod, p_self, other) \
	((tp_inplace_mod) == &instance_imod                                                      \
	 ? instance_timod(tp_self, p_self, other)                                                \
	 : (*(tp_inplace_mod))(p_self, other))
#define DeeType_invoke_math_tp_inplace_shl_NODEFAULT(tp_self, tp_inplace_shl, p_self, other) \
	((tp_inplace_shl) == &instance_ishl                                                      \
	 ? instance_tishl(tp_self, p_self, other)                                                \
	 : (*(tp_inplace_shl))(p_self, other))
#define DeeType_invoke_math_tp_inplace_shr_NODEFAULT(tp_self, tp_inplace_shr, p_self, other) \
	((tp_inplace_shr) == &instance_ishr                                                      \
	 ? instance_tishr(tp_self, p_self, other)                                                \
	 : (*(tp_inplace_shr))(p_self, other))
#define DeeType_invoke_math_tp_inplace_and_NODEFAULT(tp_self, tp_inplace_and, p_self, other) \
	((tp_inplace_and) == &instance_iand                                                      \
	 ? instance_tiand(tp_self, p_self, other)                                                \
	 : (*(tp_inplace_and))(p_self, other))
#define DeeType_invoke_math_tp_inplace_or_NODEFAULT(tp_self, tp_inplace_or, p_self, other) \
	((tp_inplace_or) == &instance_ior                                                      \
	 ? instance_tior(tp_self, p_self, other)                                               \
	 : (*(tp_inplace_or))(p_self, other))
#define DeeType_invoke_math_tp_inplace_xor_NODEFAULT(tp_self, tp_inplace_xor, p_self, other) \
	((tp_inplace_xor) == &instance_ixor                                                      \
	 ? instance_tixor(tp_self, p_self, other)                                                \
	 : (*(tp_inplace_xor))(p_self, other))
#define DeeType_invoke_math_tp_inplace_pow_NODEFAULT(tp_self, tp_inplace_pow, p_self, other) \
	((tp_inplace_pow) == &instance_ipow                                                      \
	 ? instance_tipow(tp_self, p_self, other)                                                \
	 : (*(tp_inplace_pow))(p_self, other))
#define DeeType_invoke_cmp_tp_hash_NODEFAULT(tp_self, tp_hash, self) \
	((tp_hash) == &instance_hash                                     \
	 ? instance_thash(tp_self, self)                                 \
	 : (tp_hash) == &instance_builtin_hash                           \
	   ? instance_builtin_thash(tp_self, self)                       \
	   : (*(tp_hash))(self))
#define DeeType_invoke_cmp_tp_eq_NODEFAULT(tp_self, tp_eq, self, other) \
	((tp_eq) == &instance_eq                                            \
	 ? instance_teq(tp_self, self, other)                               \
	 : (tp_eq) == &instance_builtin_eq                                  \
	   ? instance_builtin_teq(tp_self, self, other)                     \
	   : (*(tp_eq))(self, other))
#define DeeType_invoke_cmp_tp_ne_NODEFAULT(tp_self, tp_ne, self, other) \
	((tp_ne) == &instance_ne                                            \
	 ? instance_tne(tp_self, self, other)                               \
	 : (tp_ne) == &instance_builtin_ne                                  \
	   ? instance_builtin_tne(tp_self, self, other)                     \
	   : (*(tp_ne))(self, other))
#define DeeType_invoke_cmp_tp_lo_NODEFAULT(tp_self, tp_lo, self, other) \
	((tp_lo) == &instance_lo                                            \
	 ? instance_tlo(tp_self, self, other)                               \
	 : (tp_lo) == &instance_builtin_lo                                  \
	   ? instance_builtin_tlo(tp_self, self, other)                     \
	   : (*(tp_lo))(self, other))
#define DeeType_invoke_cmp_tp_le_NODEFAULT(tp_self, tp_le, self, other) \
	((tp_le) == &instance_le                                            \
	 ? instance_tle(tp_self, self, other)                               \
	 : (tp_le) == &instance_builtin_le                                  \
	   ? instance_builtin_tle(tp_self, self, other)                     \
	   : (*(tp_le))(self, other))
#define DeeType_invoke_cmp_tp_gr_NODEFAULT(tp_self, tp_gr, self, other) \
	((tp_gr) == &instance_gr                                            \
	 ? instance_tgr(tp_self, self, other)                               \
	 : (tp_gr) == &instance_builtin_gr                                  \
	   ? instance_builtin_tgr(tp_self, self, other)                     \
	   : (*(tp_gr))(self, other))
#define DeeType_invoke_cmp_tp_ge_NODEFAULT(tp_self, tp_ge, self, other) \
	((tp_ge) == &instance_ge                                            \
	 ? instance_tge(tp_self, self, other)                               \
	 : (tp_ge) == &instance_builtin_ge                                  \
	   ? instance_builtin_tge(tp_self, self, other)                     \
	   : (*(tp_ge))(self, other))
#define DeeType_invoke_seq_tp_iter_NODEFAULT(tp_self, tp_iter, self) \
	((tp_iter) == &instance_iter                                     \
	 ? instance_titer(tp_self, self)                                 \
	 : (*(tp_iter))(self))
#define DeeType_invoke_seq_tp_sizeob_NODEFAULT(tp_self, tp_sizeob, self) \
	((tp_sizeob) == &instance_size                                       \
	 ? instance_tsize(tp_self, self)                                     \
	 : (*(tp_sizeob))(self))
#define DeeType_invoke_seq_tp_contains_NODEFAULT(tp_self, tp_contains, self, other) \
	((tp_contains) == &instance_contains                                            \
	 ? instance_tcontains(tp_self, self, other)                                     \
	 : (*(tp_contains))(self, other))
#define DeeType_invoke_seq_tp_getitem_NODEFAULT(tp_self, tp_getitem, self, index) \
	((tp_getitem) == &instance_getitem                                            \
	 ? instance_tgetitem(tp_self, self, index)                                    \
	 : (*(tp_getitem))(self, index))
#define DeeType_invoke_seq_tp_delitem_NODEFAULT(tp_self, tp_delitem, self, index) \
	((tp_delitem) == &instance_delitem                                            \
	 ? instance_tdelitem(tp_self, self, index)                                    \
	 : (*(tp_delitem))(self, index))
#define DeeType_invoke_seq_tp_setitem_NODEFAULT(tp_self, tp_setitem, self, index, value) \
	((tp_setitem) == &instance_setitem                                                   \
	 ? instance_tsetitem(tp_self, self, index, value)                                    \
	 : (*(tp_setitem))(self, index, value))
#define DeeType_invoke_seq_tp_getrange_NODEFAULT(tp_self, tp_getrange, self, start, end) \
	((tp_getrange) == &instance_getrange                                                 \
	 ? instance_tgetrange(tp_self, self, start, end)                                     \
	 : (*(tp_getrange))(self, start, end))
#define DeeType_invoke_seq_tp_delrange_NODEFAULT(tp_self, tp_delrange, self, start, end) \
	((tp_delrange) == &instance_delrange                                                 \
	 ? instance_tdelrange(tp_self, self, start, end)                                     \
	 : (*(tp_delrange))(self, start, end))
#define DeeType_invoke_seq_tp_setrange_NODEFAULT(tp_self, tp_setrange, self, start, end, values) \
	((tp_setrange) == &instance_setrange                                                         \
	 ? instance_tsetrange(tp_self, self, start, end, values)                                     \
	 : (*(tp_setrange))(self, start, end, values))
#define DeeType_invoke_seq_tp_foreach_NODEFAULT(tp_self, tp_foreach, self, proc, arg)                       (*tp_foreach)(self, proc, arg)
#define DeeType_invoke_seq_tp_foreach_pair_NODEFAULT(tp_self, tp_foreach_pair, self, proc, arg)             (*tp_foreach_pair)(self, proc, arg)
#define DeeType_invoke_seq_tp_bounditem_NODEFAULT(tp_self, tp_bounditem, self, index)                       (*tp_bounditem)(self, index)
#define DeeType_invoke_seq_tp_hasitem_NODEFAULT(tp_self, tp_hasitem, self, index)                           (*tp_hasitem)(self, index)
#define DeeType_invoke_seq_tp_size_NODEFAULT(tp_self, tp_size, self)                                        (*tp_size)(self)
#define DeeType_invoke_seq_tp_getitem_index_NODEFAULT(tp_self, tp_getitem_index, self, index)               (*tp_getitem_index)(self, index)
#define DeeType_invoke_seq_tp_delitem_index_NODEFAULT(tp_self, tp_delitem_index, self, index)               (*tp_delitem_index)(self, index)
#define DeeType_invoke_seq_tp_setitem_index_NODEFAULT(tp_self, tp_setitem_index, self, index, value)        (*tp_setitem_index)(self, index, value)
#define DeeType_invoke_seq_tp_bounditem_index_NODEFAULT(tp_self, tp_bounditem_index, self, index)           (*tp_bounditem_index)(self, index)
#define DeeType_invoke_seq_tp_hasitem_index_NODEFAULT(tp_self, tp_hasitem_index, self, index)               (*tp_hasitem_index)(self, index)
#define DeeType_invoke_seq_tp_getrange_index_NODEFAULT(tp_self, tp_getrange_index, self, start, end)        (*tp_getrange_index)(self, start, end)
#define DeeType_invoke_seq_tp_delrange_index_NODEFAULT(tp_self, tp_delrange_index, self, start, end)        (*tp_delrange_index)(self, start, end)
#define DeeType_invoke_seq_tp_setrange_index_NODEFAULT(tp_self, tp_setrange_index, self, start, end, value) (*tp_setrange_index)(self, start, end, value)
#define DeeType_invoke_seq_tp_getrange_index_n_NODEFAULT(tp_self, tp_getrange_index_n, self, start)         (*tp_getrange_index_n)(self, start)
#define DeeType_invoke_seq_tp_delrange_index_n_NODEFAULT(tp_self, tp_delrange_index_n, self, start)         (*tp_delrange_index_n)(self, start)
#define DeeType_invoke_seq_tp_setrange_index_n_NODEFAULT(tp_self, tp_setrange_index_n, self, start, value)  (*tp_setrange_index_n)(self, start, value)
#define DeeType_invoke_attr_tp_getattr_NODEFAULT(tp_self, tp_getattr, self, name) \
	((tp_getattr) == &instance_getattr                                            \
	 ? instance_tgetattr(tp_self, self, name)                                     \
	 : (*(tp_getattr))(self, name))
#define DeeType_invoke_attr_tp_delattr_NODEFAULT(tp_self, tp_delattr, self, name) \
	((tp_delattr) == &instance_delattr                                            \
	 ? instance_tdelattr(tp_self, self, name)                                     \
	 : (*(tp_delattr))(self, name))
#define DeeType_invoke_attr_tp_setattr_NODEFAULT(tp_self, tp_setattr, self, name, value) \
	((tp_setattr) == &instance_setattr                                                   \
	 ? instance_tsetattr(tp_self, self, name, value)                                     \
	 : (*(tp_setattr))(self, name, value))
#define DeeType_invoke_with_tp_enter_NODEFAULT(tp_self, tp_enter, self) \
	((tp_enter) == &instance_enter                                      \
	 ? instance_tenter(tp_self, self)                                   \
	 : (*(tp_enter))(self))
#define DeeType_invoke_with_tp_leave_NODEFAULT(tp_self, tp_leave, self) \
	((tp_leave) == &instance_leave                                      \
	 ? instance_tleave(tp_self, self)                                   \
	 : (*(tp_leave))(self))


/*[[[begin:DEFAULT_OPERATORS]]]*/

/* Default wrappers for implementing tp_str/tp_repr <===> tp_print/tp_printrepr */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultStrWithPrint(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultReprWithPrintRepr(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) dssize_t DCALL DeeObject_DefaultPrintWithStr(DeeObject *__restrict self, dformatprinter printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) dssize_t DCALL DeeObject_DefaultPrintReprWithRepr(DeeObject *__restrict self, dformatprinter printer, void *arg);

/* Default wrappers for implementing tp_call <===> tp_call_kw */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultCallWithCallKw(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultCallKwWithCall(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);

/* Default wrappers for implementing tp_int32 <===> tp_int64 <===> tp_int <===> tp_double  */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultInt32WithInt(DeeObject *__restrict self, int32_t *__restrict result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultInt64WithInt(DeeObject *__restrict self, int64_t *__restrict result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultDoubleWithInt(DeeObject *__restrict self, double *__restrict result);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultIntWithInt32(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultInt64WithInt32(DeeObject *__restrict self, int64_t *__restrict result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultDoubleWithInt32(DeeObject *__restrict self, double *__restrict result);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultIntWithInt64(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultInt32WithInt64(DeeObject *__restrict self, int32_t *__restrict result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultDoubleWithInt64(DeeObject *__restrict self, double *__restrict result);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultIntWithDouble(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultInt32WithDouble(DeeObject *__restrict self, int32_t *__restrict result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultInt64WithDouble(DeeObject *__restrict self, int64_t *__restrict result);


/* Default wrappers for implementing math operators using copy + their inplace variants. */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultAddWithInplaceAdd(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultAddWithSub(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultAddWithInplaceSub(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultSubWithInplaceSub(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultSubWithAdd(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultSubWithInplaceAdd(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultMulWithInplaceMul(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultDivWithInplaceDiv(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultModWithInplaceMod(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultShlWithInplaceShl(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultShrWithInplaceShr(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultAndWithInplaceAnd(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultOrWithInplaceOr(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultXorWithInplaceXor(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultPowWithInplacePow(DeeObject *self, DeeObject *other);

/* Default wrappers for implementing inplace math operators using their non-inplace variants. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultInplaceAddWithAdd(DREF DeeObject **p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultInplaceAddWithSub(DREF DeeObject **p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultInplaceAddWithInplaceSub(DREF DeeObject **p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultInplaceSubWithSub(DREF DeeObject **p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultInplaceSubWithAdd(DREF DeeObject **p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultInplaceSubWithInplaceAdd(DREF DeeObject **p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultInplaceMulWithMul(DREF DeeObject **p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultInplaceDivWithDiv(DREF DeeObject **p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultInplaceModWithMod(DREF DeeObject **p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultInplaceShlWithShl(DREF DeeObject **p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultInplaceShrWithShr(DREF DeeObject **p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultInplaceAndWithAnd(DREF DeeObject **p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultInplaceOrWithOr(DREF DeeObject **p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultInplaceXorWithXor(DREF DeeObject **p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultInplacePowWithPow(DREF DeeObject **p_self, DeeObject *other);

/* Default wrappers for implementing inc/dec. */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultIncWithInplaceAdd(DREF DeeObject **p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultIncWithAdd(DREF DeeObject **p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultIncWithInplaceSub(DREF DeeObject **p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultIncWithSub(DREF DeeObject **p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultDecWithInplaceAdd(DREF DeeObject **p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultDecWithAdd(DREF DeeObject **p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultDecWithInplaceSub(DREF DeeObject **p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultDecWithSub(DREF DeeObject **p_self);

/* Default wrappers for implementing ==/!=/</<=/>/>= using their logical inverse. */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultEqWithNe(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultNeWithEq(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultLoWithGe(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultLeWithGr(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultGrWithLe(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultGeWithLo(DeeObject *self, DeeObject *other);

/* Default wrappers for implementing sequence operators. */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultIterWithForeach(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultIterWithForeachPair(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultIterWithSizeAndGetItemIndex(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultIterWithSizeAndGetItemIndexFast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultIterWithGetItemIndex(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultIterWithSizeObDefaultAndGetItemDefault(DeeObject *__restrict self); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultIterWithGetItemDefault(DeeObject *__restrict self); /* May call other DEFAULT operators */

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeObject_DefaultForeachWithIter(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeObject_DefaultForeachWithForeachPair(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultForeachWithSizeAndGetItemIndexFast(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultForeachWithSizeAndGetItemIndex(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault(DeeObject *__restrict self, Dee_foreach_t proc, void *arg); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultForeachWithGetItemIndexDefault(DeeObject *__restrict self, Dee_foreach_t proc, void *arg); /* May call other DEFAULT operators */

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeObject_DefaultForeachPairWithForeach(DeeObject *__restrict self, Dee_foreach_pair_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeObject_DefaultForeachPairWithForeachDefault(DeeObject *__restrict self, Dee_foreach_pair_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeObject_DefaultForeachPairWithIter(DeeObject *__restrict self, Dee_foreach_pair_t proc, void *arg);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultSizeObWithSize(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultSizeObWithSizeDefault(DeeObject *__restrict self); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeObject_DefaultSizeWithSizeOb(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeSeq_DefaultSizeWithForeachPair(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeSeq_DefaultSizeWithForeach(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeSeq_DefaultSizeWithIter(DeeObject *__restrict self);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultContainsWithForeachPair(DeeObject *self, DeeObject *elem);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultContainsWithForeachDefault(DeeObject *self, DeeObject *elem); /* May call other DEFAULT operators */

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultGetItemWithGetItemIndex(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultGetItemWithGetItemIndexDefault(DeeObject *self, DeeObject *index); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultGetItemIndexWithSizeAndGetItemIndexFast(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultGetItemIndexWithGetItem(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetItemIndexWithForeachDefault(DeeObject *__restrict self, size_t index); /* May call other DEFAULT operators */

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultDelItemWithDelItemIndex(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultDelItemWithDelItemIndexDefault(DeeObject *self, DeeObject *index); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultDelItemIndexWithDelItem(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelItemIndexWithDelRangeIndexDefault(DeeObject *__restrict self, size_t index); /* May call other DEFAULT operators */

INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_DefaultSetItemWithSetItemIndex(DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_DefaultSetItemWithSetItemIndexDefault(DeeObject *self, DeeObject *index, DeeObject *value); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeObject_DefaultSetItemIndexWithSetItem(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultSetItemIndexWithSetRangeIndexDefault(DeeObject *self, size_t index, DeeObject *value); /* May call other DEFAULT operators */

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemWithBoundItemIndex(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemWithGetItem(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemWithGetItemIndex(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemWithBoundItemIndexDefault(DeeObject *self, DeeObject *index); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultBoundItemIndexWithBoundItem(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultBoundItemIndexWithSizeAndGetItemIndexFast(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultBoundItemIndexWithGetItemIndex(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultBoundItemIndexWithGetItem(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultBoundItemIndexWithGetItemIndexDefault(DeeObject *__restrict self, size_t index); /* May call other DEFAULT operators */

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemWithHasItemIndex(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemWithBoundItem(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemWithBoundItemIndex(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemWithGetItem(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemWithGetItemIndex(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemWithHasItemIndexDefault(DeeObject *self, DeeObject *index); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultHasItemIndexWithHasItem(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultHasItemIndexWithBoundItemIndex(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultHasItemIndexWithBoundItem(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultHasItemIndexWithGetItemIndex(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultHasItemIndexWithGetItem(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultHasItemIndexWithSize(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultHasItemIndexWithSizeDefault(DeeObject *__restrict self, size_t index); /* May call other DEFAULT operators */

INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_DefaultGetRangeWithGetRangeIndexAndGetRangeIndexN(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_DefaultGetRangeWithGetRangeIndexDefaultAndGetRangeIndexNDefault(DeeObject *self, DeeObject *start, DeeObject *end); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_DefaultGetRangeWithSizeDefaultAndGetItemIndex(DeeObject *self, DeeObject *start, DeeObject *end); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_DefaultGetRangeWithSizeObAndGetItem(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultGetRangeIndexWithGetRange(DeeObject *__restrict self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetRangeIndexWithSizeAndGetItemIndexFast(DeeObject *__restrict self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetRangeIndexWithSizeDefaultAndGetItemIndex(DeeObject *__restrict self, Dee_ssize_t start, Dee_ssize_t end); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetRangeIndexWithSizeDefaultAndGetItem(DeeObject *__restrict self, Dee_ssize_t start, Dee_ssize_t end); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetRangeIndexWithSizeDefaultAndIter(DeeObject *__restrict self, Dee_ssize_t start, Dee_ssize_t end); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetRangeIndexWithSizeDefaultAndIterDefault(DeeObject *__restrict self, Dee_ssize_t start, Dee_ssize_t end); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultGetRangeIndexNWithGetRange(DeeObject *__restrict self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetRangeIndexNWithSizeAndGetRangeIndex(DeeObject *__restrict self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndGetRangeIndex(DeeObject *__restrict self, Dee_ssize_t start); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetRangeIndexNWithSizeAndGetItemIndexFast(DeeObject *__restrict self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndGetItemIndex(DeeObject *__restrict self, Dee_ssize_t start); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndGetItem(DeeObject *__restrict self, Dee_ssize_t start); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndIter(DeeObject *__restrict self, Dee_ssize_t start); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndIterDefault(DeeObject *__restrict self, Dee_ssize_t start); /* May call other DEFAULT operators */

INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_DefaultDelRangeWithDelRangeIndexAndDelRangeIndexN(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_DefaultDelRangeWithDelRangeIndexDefaultAndDelRangeIndexNDefault(DeeObject *self, DeeObject *start, DeeObject *end); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultDelRangeWithSetRangeNone(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultDelRangeWithSetRangeNoneDefault(DeeObject *self, DeeObject *start, DeeObject *end); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultDelRangeIndexWithDelRange(DeeObject *__restrict self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelRangeIndexWithSetRangeIndexNone(DeeObject *__restrict self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelRangeIndexWithSetRangeIndexNoneDefault(DeeObject *__restrict self, Dee_ssize_t start, Dee_ssize_t end); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultDelRangeIndexNWithDelRange(DeeObject *__restrict self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelRangeIndexNWithSizeAndDelRangeIndex(DeeObject *__restrict self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelRangeIndexNWithSizeDefaultAndDelRangeIndex(DeeObject *__restrict self, Dee_ssize_t start); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelRangeIndexNWithSetRangeIndexNNone(DeeObject *__restrict self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelRangeIndexNWithSetRangeIndexNNoneDefault(DeeObject *__restrict self, Dee_ssize_t start); /* May call other DEFAULT operators */

INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL DeeObject_DefaultSetRangeWithSetRangeIndexAndSetRangeIndexN(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL DeeObject_DefaultSetRangeWithSetRangeIndexDefaultAndSetRangeIndexNDefault(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *value); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeObject_DefaultSetRangeIndexWithSetRange(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultSetRangeIndexWithSizeAndDelItemIndexAndSetItemIndex(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultSetRangeIndexWithSizeDefaultAndDelItemIndexDefaultAndSetItemIndexDefault(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *value); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultSetRangeIndexWithSizeAndSetItemIndex(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultSetRangeIndexWithSizeDefaultAndSetItemIndexDefault(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *value); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeObject_DefaultSetRangeIndexNWithSetRange(DeeObject *self, Dee_ssize_t start, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultSetRangeIndexNWithSizeAndSetRangeIndex(DeeObject *self, Dee_ssize_t start, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultSetRangeIndexNWithSizeDefaultAndSetRangeIndex(DeeObject *self, Dee_ssize_t start, DeeObject *value); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultSetRangeIndexNWithSizeAndDelItemIndexAndSetItemIndex(DeeObject *self, Dee_ssize_t start, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultSetRangeIndexNWithSizeDefaultAndDelItemIndexDefaultAndSetItemIndexDefault(DeeObject *self, Dee_ssize_t start, DeeObject *value); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultSetRangeIndexNWithSizeAndSetItemIndex(DeeObject *self, Dee_ssize_t start, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultSetRangeIndexNWithSizeDefaultAndSetItemIndexDefault(DeeObject *self, Dee_ssize_t start, DeeObject *value); /* May call other DEFAULT operators */
/*[[[end:DEFAULT_OPERATORS]]]*/

/* clang-format off */
/*[[[deemon
import * from deemon;
import util;
local fileData = File.open(__FILE__).read()
	.partition("/" "*[[[begin:DEFAULT_OPERATORS]]]*" "/").last
	.partition("/" "*[[[end:DEFAULT_OPERATORS]]]*" "/").first
	.strip()
	.decode("utf-8");
local decls: {(string, string, string)...} = [];
for (local line: fileData.splitlines(false)) {
	local a, b, c;
	try {
		a, b, c = line.rescanf(r"(INTDEF.*DCALL\s+)([^(]+)(\(.*)")...;
	} catch (...) {
		continue;
	}
	decls.append((a, b, c));
}
for (local a, b, c: decls) {
	local nna, nnb, nnc;
	try {
		nna, nnb, nnc = a.rescanf(r"(.*NONNULL\(\()([^)]+)(\)\).*)")...;
	} catch (...) {
		nna = a;
		nnb = nnc = "";
	}
	if (nnb)
		nnb = "1, " + ", ".join(for (local x: nnb.split(",")) int(x.strip()) + 1);
	local nameA, none, nameB = b.partition("_")...;
	c = c[1:].replace("__restrict ", "");
	print(nna, nnb, nnc, nameA, "_T", nameB, "(DeeTypeObject *tp_self, ", c);
}
local groupNames: {string...} = [];
local groups: {string: {string...}} = Dict();
function getGroupName(name: string): string {
	name = name.partition("_").last;
	name = name.partition("Default").last;
	name = name.partition("With").first;
	return name;
}
function getTpName(groupName: string): string {
	local result = "tp_" + groupName.first.lower();
	for (local ch: groupName[1:]) {
		if (ch.isupper())
			result += "_";
		result += ch.lower();
	}
	result = result.replace("get_", "get");
	result = result.replace("del_", "del");
	result = result.replace("set_", "set");
	result = result.replace("has_", "has");
	result = result.replace("bound_", "bound");
	result = result.replace("print_repr", "printrepr");
	result = result.replace("size_ob", "sizeob");
	return result;
}

for (local a, b, c: decls) {
	local name = getGroupName(b);
	local group = groups.get(name);
	if (group is none) {
		groupNames.append(name);
		groups[name] = group = [];
	}
	group.append(b);
}
for (local name: groupNames) {
	local mems = groups[name];
	local tp = getTpName(name);
	print("#define DeeType_MapDefault", name, "(", tp, ", map, default) \\");
	for (local i, mem: util.enumerate(mems)) {
		print("	", i == 0 ? "(" : " "),;
		print("(", tp, ") == &", mem, " ? map(", mem.replace("_", "_T"), ") : "),;
		if (i == #mems - 1) {
			print("default)");
		} else {
			print("\\");
		}
	}
}
for (local name: groupNames) {
	local mems = groups[name];
	local tp = getTpName(name);
	print("#define DeeType_IsDefault", name, "(", tp, ") \\");
	for (local i, mem: util.enumerate(mems)) {
		print("	", i == 0 ? "(" : " "),;
		print("(", tp, ") == &", mem),;
		if (i == #mems - 1) {
			print(")");
		} else {
			print(" || \\");
		}
	}
}
function getOperatorPackage(name: string): string {
	return ("Item" in name || "Range" in name) ? "seq" : {
		"Call" : "",
		"CallKw" : "",
		"Str" : "cast",
		"Repr" : "cast",
		"Print" : "cast",
		"PrintRepr" : "cast",
		"Eq" : "cmp",
		"Ne" : "cmp",
		"Lo" : "cmp",
		"Le" : "cmp",
		"Gr" : "cmp",
		"Ge" : "cmp",
		"Iter" : "seq",
		"Size" : "seq",
		"SizeOb" : "seq",
		"Foreach" : "seq",
		"ForeachPair" : "seq",
		"Contains" : "seq",
	}.get(name, "math");
}
for (local name: groupNames) {
	local mems = groups[name];
	local tp = getTpName(name);
	local package = getOperatorPackage(name);
	if (package)
		package += "_";
	local args = decls.filter(e -> e[1] == mems.first).first.last.rstrip(");");
	local args = ", ".join(for (local a: args.split(",")) a.strip().rerpartition(r"[* ]").last);
	print("#define DeeType_invoke_", package, tp, "_DEFAULT(tp_self, ", tp, ", ", args, ", default) \\");
	for (local i, mem: util.enumerate(mems)) {
		print("	", i == 0 ? "(" : " "),;
		print("(", tp, ") == &", mem, " ? ", mem.replace("_", "_T"), "(tp_self, ", args, ") : "),;
		print("\\");
	}
	print("	 default)");
	//print("	 (*(", tp, "))(", args, "))");
}
for (local name: groupNames) {
	local mems = groups[name];
	local tp = getTpName(name);
	local package = getOperatorPackage(name);
	if (package)
		package += "_";
	local args = decls.filter(e -> e[1] == mems.first).first.last.rstrip(");");
	local args = ", ".join(for (local a: args.split(",")) a.strip().rerpartition(r"[* ]").last);
	print("#define DeeType_invoke_", package, tp, "(tp_self, ", tp, ", ", args, ") \\");
	print("	 DeeType_invoke_", package, tp, "_DEFAULT(tp_self, ", tp, ", ", args, ", DeeType_invoke_", package, tp, "_NODEFAULT(tp_self, ", tp, ", ", args, "))");
}
]]]*/
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultStrWithPrint(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultReprWithPrintRepr(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2, 3)) dssize_t DCALL DeeObject_TDefaultPrintWithStr(DeeTypeObject *tp_self, DeeObject *self, dformatprinter printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) dssize_t DCALL DeeObject_TDefaultPrintReprWithRepr(DeeTypeObject *tp_self, DeeObject *self, dformatprinter printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultCallWithCallKw(DeeTypeObject *tp_self, DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultCallKwWithCall(DeeTypeObject *tp_self, DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultInt32WithInt(DeeTypeObject *tp_self, DeeObject *self, int32_t *result);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultInt64WithInt(DeeTypeObject *tp_self, DeeObject *self, int64_t *result);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultDoubleWithInt(DeeTypeObject *tp_self, DeeObject *self, double *result);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultIntWithInt32(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultInt64WithInt32(DeeTypeObject *tp_self, DeeObject *self, int64_t *result);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultDoubleWithInt32(DeeTypeObject *tp_self, DeeObject *self, double *result);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultIntWithInt64(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultInt32WithInt64(DeeTypeObject *tp_self, DeeObject *self, int32_t *result);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultDoubleWithInt64(DeeTypeObject *tp_self, DeeObject *self, double *result);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultIntWithDouble(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultInt32WithDouble(DeeTypeObject *tp_self, DeeObject *self, int32_t *result);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultInt64WithDouble(DeeTypeObject *tp_self, DeeObject *self, int64_t *result);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultAddWithInplaceAdd(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultAddWithSub(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultAddWithInplaceSub(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultSubWithInplaceSub(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultSubWithAdd(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultSubWithInplaceAdd(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultMulWithInplaceMul(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultDivWithInplaceDiv(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultModWithInplaceMod(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultShlWithInplaceShl(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultShrWithInplaceShr(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultAndWithInplaceAnd(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultOrWithInplaceOr(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultXorWithInplaceXor(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultPowWithInplacePow(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultInplaceAddWithAdd(DeeTypeObject *tp_self, DREF DeeObject **p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultInplaceAddWithSub(DeeTypeObject *tp_self, DREF DeeObject **p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultInplaceAddWithInplaceSub(DeeTypeObject *tp_self, DREF DeeObject **p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultInplaceSubWithSub(DeeTypeObject *tp_self, DREF DeeObject **p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultInplaceSubWithAdd(DeeTypeObject *tp_self, DREF DeeObject **p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultInplaceSubWithInplaceAdd(DeeTypeObject *tp_self, DREF DeeObject **p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultInplaceMulWithMul(DeeTypeObject *tp_self, DREF DeeObject **p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultInplaceDivWithDiv(DeeTypeObject *tp_self, DREF DeeObject **p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultInplaceModWithMod(DeeTypeObject *tp_self, DREF DeeObject **p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultInplaceShlWithShl(DeeTypeObject *tp_self, DREF DeeObject **p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultInplaceShrWithShr(DeeTypeObject *tp_self, DREF DeeObject **p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultInplaceAndWithAnd(DeeTypeObject *tp_self, DREF DeeObject **p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultInplaceOrWithOr(DeeTypeObject *tp_self, DREF DeeObject **p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultInplaceXorWithXor(DeeTypeObject *tp_self, DREF DeeObject **p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultInplacePowWithPow(DeeTypeObject *tp_self, DREF DeeObject **p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultIncWithInplaceAdd(DeeTypeObject *tp_self, DREF DeeObject **p_self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultIncWithAdd(DeeTypeObject *tp_self, DREF DeeObject **p_self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultIncWithInplaceSub(DeeTypeObject *tp_self, DREF DeeObject **p_self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultIncWithSub(DeeTypeObject *tp_self, DREF DeeObject **p_self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultDecWithInplaceAdd(DeeTypeObject *tp_self, DREF DeeObject **p_self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultDecWithAdd(DeeTypeObject *tp_self, DREF DeeObject **p_self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultDecWithInplaceSub(DeeTypeObject *tp_self, DREF DeeObject **p_self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultDecWithSub(DeeTypeObject *tp_self, DREF DeeObject **p_self);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultEqWithNe(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultNeWithEq(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultLoWithGe(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultLeWithGr(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultGrWithLe(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultGeWithLo(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultIterWithForeach(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultIterWithForeachPair(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultIterWithSizeAndGetItemIndex(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultIterWithSizeAndGetItemIndexFast(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultIterWithGetItemIndex(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultIterWithSizeObDefaultAndGetItemDefault(DeeTypeObject *tp_self, DeeObject *self); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultIterWithGetItemDefault(DeeTypeObject *tp_self, DeeObject *self); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeObject_TDefaultForeachWithIter(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeObject_TDefaultForeachWithForeachPair(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeSeq_TDefaultForeachWithSizeAndGetItemIndexFast(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeSeq_TDefaultForeachWithSizeAndGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeSeq_TDefaultForeachWithSizeDefaultAndGetItemIndexDefault(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t proc, void *arg); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeSeq_TDefaultForeachWithGetItemIndexDefault(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t proc, void *arg); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeObject_TDefaultForeachPairWithForeach(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_pair_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeObject_TDefaultForeachPairWithForeachDefault(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_pair_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeObject_TDefaultForeachPairWithIter(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_pair_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultSizeObWithSize(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultSizeObWithSizeDefault(DeeTypeObject *tp_self, DeeObject *self); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeObject_TDefaultSizeWithSizeOb(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_TDefaultSizeWithForeachPair(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_TDefaultSizeWithForeach(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_TDefaultSizeWithIter(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_TDefaultContainsWithForeachPair(DeeTypeObject *tp_self, DeeObject *self, DeeObject *elem);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_TDefaultContainsWithForeachDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *elem); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultGetItemWithGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultGetItemWithGetItemIndexDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultGetItemIndexWithSizeAndGetItemIndexFast(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultGetItemIndexWithGetItem(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultGetItemIndexWithForeachDefault(DeeTypeObject *tp_self, DeeObject *self, size_t index); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultDelItemWithDelItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultDelItemWithDelItemIndexDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultDelItemIndexWithDelItem(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultDelItemIndexWithDelRangeIndexDefault(DeeTypeObject *tp_self, DeeObject *self, size_t index); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL DeeObject_TDefaultSetItemWithSetItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL DeeObject_TDefaultSetItemWithSetItemIndexDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index, DeeObject *value); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL DeeObject_TDefaultSetItemIndexWithSetItem(DeeTypeObject *tp_self, DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL DeeSeq_TDefaultSetItemIndexWithSetRangeIndexDefault(DeeTypeObject *tp_self, DeeObject *self, size_t index, DeeObject *value); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemWithBoundItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemWithGetItem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemWithGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemWithBoundItemIndexDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultBoundItemIndexWithBoundItem(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultBoundItemIndexWithSizeAndGetItemIndexFast(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultBoundItemIndexWithGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultBoundItemIndexWithGetItem(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultBoundItemIndexWithGetItemIndexDefault(DeeTypeObject *tp_self, DeeObject *self, size_t index); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemWithHasItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemWithBoundItem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemWithBoundItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemWithGetItem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemWithGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemWithHasItemIndexDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultHasItemIndexWithHasItem(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultHasItemIndexWithBoundItemIndex(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultHasItemIndexWithBoundItem(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultHasItemIndexWithGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultHasItemIndexWithGetItem(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultHasItemIndexWithSize(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultHasItemIndexWithSizeDefault(DeeTypeObject *tp_self, DeeObject *self, size_t index); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL DeeObject_TDefaultGetRangeWithGetRangeIndexAndGetRangeIndexN(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL DeeObject_TDefaultGetRangeWithGetRangeIndexDefaultAndGetRangeIndexNDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL DeeObject_TDefaultGetRangeWithSizeDefaultAndGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL DeeObject_TDefaultGetRangeWithSizeObAndGetItem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultGetRangeIndexWithGetRange(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultGetRangeIndexWithSizeAndGetItemIndexFast(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultGetRangeIndexWithSizeDefaultAndGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultGetRangeIndexWithSizeDefaultAndGetItem(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultGetRangeIndexWithSizeDefaultAndIter(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultGetRangeIndexWithSizeDefaultAndIterDefault(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultGetRangeIndexNWithGetRange(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultGetRangeIndexNWithSizeAndGetRangeIndex(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultGetRangeIndexNWithSizeDefaultAndGetRangeIndex(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultGetRangeIndexNWithSizeAndGetItemIndexFast(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultGetRangeIndexNWithSizeDefaultAndGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultGetRangeIndexNWithSizeDefaultAndGetItem(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultGetRangeIndexNWithSizeDefaultAndIter(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultGetRangeIndexNWithSizeDefaultAndIterDefault(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL DeeObject_TDefaultDelRangeWithDelRangeIndexAndDelRangeIndexN(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL DeeObject_TDefaultDelRangeWithDelRangeIndexDefaultAndDelRangeIndexNDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL DeeSeq_TDefaultDelRangeWithSetRangeNone(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL DeeSeq_TDefaultDelRangeWithSetRangeNoneDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultDelRangeIndexWithDelRange(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultDelRangeIndexWithSetRangeIndexNone(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultDelRangeIndexWithSetRangeIndexNoneDefault(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultDelRangeIndexNWithDelRange(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultDelRangeIndexNWithSizeAndDelRangeIndex(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultDelRangeIndexNWithSizeDefaultAndDelRangeIndex(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultDelRangeIndexNWithSetRangeIndexNNone(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultDelRangeIndexNWithSetRangeIndexNNoneDefault(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL DeeObject_TDefaultSetRangeWithSetRangeIndexAndSetRangeIndexN(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL DeeObject_TDefaultSetRangeWithSetRangeIndexDefaultAndSetRangeIndexNDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *value); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeObject_TDefaultSetRangeIndexWithSetRange(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_TDefaultSetRangeIndexWithSizeAndDelItemIndexAndSetItemIndex(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_TDefaultSetRangeIndexWithSizeDefaultAndDelItemIndexDefaultAndSetItemIndexDefault(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *value); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_TDefaultSetRangeIndexWithSizeAndSetItemIndex(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_TDefaultSetRangeIndexWithSizeDefaultAndSetItemIndexDefault(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *value); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL DeeObject_TDefaultSetRangeIndexNWithSetRange(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL DeeSeq_TDefaultSetRangeIndexNWithSizeAndSetRangeIndex(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL DeeSeq_TDefaultSetRangeIndexNWithSizeDefaultAndSetRangeIndex(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, DeeObject *value); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL DeeSeq_TDefaultSetRangeIndexNWithSizeAndDelItemIndexAndSetItemIndex(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL DeeSeq_TDefaultSetRangeIndexNWithSizeDefaultAndDelItemIndexDefaultAndSetItemIndexDefault(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, DeeObject *value); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL DeeSeq_TDefaultSetRangeIndexNWithSizeAndSetItemIndex(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL DeeSeq_TDefaultSetRangeIndexNWithSizeDefaultAndSetItemIndexDefault(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, DeeObject *value); /* May call other DEFAULT operators */
#define DeeType_MapDefaultStr(tp_str, map, default) \
	((tp_str) == &DeeObject_DefaultStrWithPrint ? map(DeeObject_TDefaultStrWithPrint) : default)
#define DeeType_MapDefaultRepr(tp_repr, map, default) \
	((tp_repr) == &DeeObject_DefaultReprWithPrintRepr ? map(DeeObject_TDefaultReprWithPrintRepr) : default)
#define DeeType_MapDefaultPrint(tp_print, map, default) \
	((tp_print) == &DeeObject_DefaultPrintWithStr ? map(DeeObject_TDefaultPrintWithStr) : default)
#define DeeType_MapDefaultPrintRepr(tp_printrepr, map, default) \
	((tp_printrepr) == &DeeObject_DefaultPrintReprWithRepr ? map(DeeObject_TDefaultPrintReprWithRepr) : default)
#define DeeType_MapDefaultCall(tp_call, map, default) \
	((tp_call) == &DeeObject_DefaultCallWithCallKw ? map(DeeObject_TDefaultCallWithCallKw) : default)
#define DeeType_MapDefaultCallKw(tp_call_kw, map, default) \
	((tp_call_kw) == &DeeObject_DefaultCallKwWithCall ? map(DeeObject_TDefaultCallKwWithCall) : default)
#define DeeType_MapDefaultInt32(tp_int32, map, default) \
	((tp_int32) == &DeeObject_DefaultInt32WithInt ? map(DeeObject_TDefaultInt32WithInt) : \
	 (tp_int32) == &DeeObject_DefaultInt32WithInt64 ? map(DeeObject_TDefaultInt32WithInt64) : \
	 (tp_int32) == &DeeObject_DefaultInt32WithDouble ? map(DeeObject_TDefaultInt32WithDouble) : default)
#define DeeType_MapDefaultInt64(tp_int64, map, default) \
	((tp_int64) == &DeeObject_DefaultInt64WithInt ? map(DeeObject_TDefaultInt64WithInt) : \
	 (tp_int64) == &DeeObject_DefaultInt64WithInt32 ? map(DeeObject_TDefaultInt64WithInt32) : \
	 (tp_int64) == &DeeObject_DefaultInt64WithDouble ? map(DeeObject_TDefaultInt64WithDouble) : default)
#define DeeType_MapDefaultDouble(tp_double, map, default) \
	((tp_double) == &DeeObject_DefaultDoubleWithInt ? map(DeeObject_TDefaultDoubleWithInt) : \
	 (tp_double) == &DeeObject_DefaultDoubleWithInt32 ? map(DeeObject_TDefaultDoubleWithInt32) : \
	 (tp_double) == &DeeObject_DefaultDoubleWithInt64 ? map(DeeObject_TDefaultDoubleWithInt64) : default)
#define DeeType_MapDefaultInt(tp_int, map, default) \
	((tp_int) == &DeeObject_DefaultIntWithInt32 ? map(DeeObject_TDefaultIntWithInt32) : \
	 (tp_int) == &DeeObject_DefaultIntWithInt64 ? map(DeeObject_TDefaultIntWithInt64) : \
	 (tp_int) == &DeeObject_DefaultIntWithDouble ? map(DeeObject_TDefaultIntWithDouble) : default)
#define DeeType_MapDefaultAdd(tp_add, map, default) \
	((tp_add) == &DeeObject_DefaultAddWithInplaceAdd ? map(DeeObject_TDefaultAddWithInplaceAdd) : \
	 (tp_add) == &DeeObject_DefaultAddWithSub ? map(DeeObject_TDefaultAddWithSub) : \
	 (tp_add) == &DeeObject_DefaultAddWithInplaceSub ? map(DeeObject_TDefaultAddWithInplaceSub) : default)
#define DeeType_MapDefaultSub(tp_sub, map, default) \
	((tp_sub) == &DeeObject_DefaultSubWithInplaceSub ? map(DeeObject_TDefaultSubWithInplaceSub) : \
	 (tp_sub) == &DeeObject_DefaultSubWithAdd ? map(DeeObject_TDefaultSubWithAdd) : \
	 (tp_sub) == &DeeObject_DefaultSubWithInplaceAdd ? map(DeeObject_TDefaultSubWithInplaceAdd) : default)
#define DeeType_MapDefaultMul(tp_mul, map, default) \
	((tp_mul) == &DeeObject_DefaultMulWithInplaceMul ? map(DeeObject_TDefaultMulWithInplaceMul) : default)
#define DeeType_MapDefaultDiv(tp_div, map, default) \
	((tp_div) == &DeeObject_DefaultDivWithInplaceDiv ? map(DeeObject_TDefaultDivWithInplaceDiv) : default)
#define DeeType_MapDefaultMod(tp_mod, map, default) \
	((tp_mod) == &DeeObject_DefaultModWithInplaceMod ? map(DeeObject_TDefaultModWithInplaceMod) : default)
#define DeeType_MapDefaultShl(tp_shl, map, default) \
	((tp_shl) == &DeeObject_DefaultShlWithInplaceShl ? map(DeeObject_TDefaultShlWithInplaceShl) : default)
#define DeeType_MapDefaultShr(tp_shr, map, default) \
	((tp_shr) == &DeeObject_DefaultShrWithInplaceShr ? map(DeeObject_TDefaultShrWithInplaceShr) : default)
#define DeeType_MapDefaultAnd(tp_and, map, default) \
	((tp_and) == &DeeObject_DefaultAndWithInplaceAnd ? map(DeeObject_TDefaultAndWithInplaceAnd) : default)
#define DeeType_MapDefaultOr(tp_or, map, default) \
	((tp_or) == &DeeObject_DefaultOrWithInplaceOr ? map(DeeObject_TDefaultOrWithInplaceOr) : default)
#define DeeType_MapDefaultXor(tp_xor, map, default) \
	((tp_xor) == &DeeObject_DefaultXorWithInplaceXor ? map(DeeObject_TDefaultXorWithInplaceXor) : default)
#define DeeType_MapDefaultPow(tp_pow, map, default) \
	((tp_pow) == &DeeObject_DefaultPowWithInplacePow ? map(DeeObject_TDefaultPowWithInplacePow) : default)
#define DeeType_MapDefaultInplaceAdd(tp_inplace_add, map, default) \
	((tp_inplace_add) == &DeeObject_DefaultInplaceAddWithAdd ? map(DeeObject_TDefaultInplaceAddWithAdd) : \
	 (tp_inplace_add) == &DeeObject_DefaultInplaceAddWithSub ? map(DeeObject_TDefaultInplaceAddWithSub) : \
	 (tp_inplace_add) == &DeeObject_DefaultInplaceAddWithInplaceSub ? map(DeeObject_TDefaultInplaceAddWithInplaceSub) : default)
#define DeeType_MapDefaultInplaceSub(tp_inplace_sub, map, default) \
	((tp_inplace_sub) == &DeeObject_DefaultInplaceSubWithSub ? map(DeeObject_TDefaultInplaceSubWithSub) : \
	 (tp_inplace_sub) == &DeeObject_DefaultInplaceSubWithAdd ? map(DeeObject_TDefaultInplaceSubWithAdd) : \
	 (tp_inplace_sub) == &DeeObject_DefaultInplaceSubWithInplaceAdd ? map(DeeObject_TDefaultInplaceSubWithInplaceAdd) : default)
#define DeeType_MapDefaultInplaceMul(tp_inplace_mul, map, default) \
	((tp_inplace_mul) == &DeeObject_DefaultInplaceMulWithMul ? map(DeeObject_TDefaultInplaceMulWithMul) : default)
#define DeeType_MapDefaultInplaceDiv(tp_inplace_div, map, default) \
	((tp_inplace_div) == &DeeObject_DefaultInplaceDivWithDiv ? map(DeeObject_TDefaultInplaceDivWithDiv) : default)
#define DeeType_MapDefaultInplaceMod(tp_inplace_mod, map, default) \
	((tp_inplace_mod) == &DeeObject_DefaultInplaceModWithMod ? map(DeeObject_TDefaultInplaceModWithMod) : default)
#define DeeType_MapDefaultInplaceShl(tp_inplace_shl, map, default) \
	((tp_inplace_shl) == &DeeObject_DefaultInplaceShlWithShl ? map(DeeObject_TDefaultInplaceShlWithShl) : default)
#define DeeType_MapDefaultInplaceShr(tp_inplace_shr, map, default) \
	((tp_inplace_shr) == &DeeObject_DefaultInplaceShrWithShr ? map(DeeObject_TDefaultInplaceShrWithShr) : default)
#define DeeType_MapDefaultInplaceAnd(tp_inplace_and, map, default) \
	((tp_inplace_and) == &DeeObject_DefaultInplaceAndWithAnd ? map(DeeObject_TDefaultInplaceAndWithAnd) : default)
#define DeeType_MapDefaultInplaceOr(tp_inplace_or, map, default) \
	((tp_inplace_or) == &DeeObject_DefaultInplaceOrWithOr ? map(DeeObject_TDefaultInplaceOrWithOr) : default)
#define DeeType_MapDefaultInplaceXor(tp_inplace_xor, map, default) \
	((tp_inplace_xor) == &DeeObject_DefaultInplaceXorWithXor ? map(DeeObject_TDefaultInplaceXorWithXor) : default)
#define DeeType_MapDefaultInplacePow(tp_inplace_pow, map, default) \
	((tp_inplace_pow) == &DeeObject_DefaultInplacePowWithPow ? map(DeeObject_TDefaultInplacePowWithPow) : default)
#define DeeType_MapDefaultInc(tp_inc, map, default) \
	((tp_inc) == &DeeObject_DefaultIncWithInplaceAdd ? map(DeeObject_TDefaultIncWithInplaceAdd) : \
	 (tp_inc) == &DeeObject_DefaultIncWithAdd ? map(DeeObject_TDefaultIncWithAdd) : \
	 (tp_inc) == &DeeObject_DefaultIncWithInplaceSub ? map(DeeObject_TDefaultIncWithInplaceSub) : \
	 (tp_inc) == &DeeObject_DefaultIncWithSub ? map(DeeObject_TDefaultIncWithSub) : default)
#define DeeType_MapDefaultDec(tp_dec, map, default) \
	((tp_dec) == &DeeObject_DefaultDecWithInplaceAdd ? map(DeeObject_TDefaultDecWithInplaceAdd) : \
	 (tp_dec) == &DeeObject_DefaultDecWithAdd ? map(DeeObject_TDefaultDecWithAdd) : \
	 (tp_dec) == &DeeObject_DefaultDecWithInplaceSub ? map(DeeObject_TDefaultDecWithInplaceSub) : \
	 (tp_dec) == &DeeObject_DefaultDecWithSub ? map(DeeObject_TDefaultDecWithSub) : default)
#define DeeType_MapDefaultEq(tp_eq, map, default) \
	((tp_eq) == &DeeObject_DefaultEqWithNe ? map(DeeObject_TDefaultEqWithNe) : default)
#define DeeType_MapDefaultNe(tp_ne, map, default) \
	((tp_ne) == &DeeObject_DefaultNeWithEq ? map(DeeObject_TDefaultNeWithEq) : default)
#define DeeType_MapDefaultLo(tp_lo, map, default) \
	((tp_lo) == &DeeObject_DefaultLoWithGe ? map(DeeObject_TDefaultLoWithGe) : default)
#define DeeType_MapDefaultLe(tp_le, map, default) \
	((tp_le) == &DeeObject_DefaultLeWithGr ? map(DeeObject_TDefaultLeWithGr) : default)
#define DeeType_MapDefaultGr(tp_gr, map, default) \
	((tp_gr) == &DeeObject_DefaultGrWithLe ? map(DeeObject_TDefaultGrWithLe) : default)
#define DeeType_MapDefaultGe(tp_ge, map, default) \
	((tp_ge) == &DeeObject_DefaultGeWithLo ? map(DeeObject_TDefaultGeWithLo) : default)
#define DeeType_MapDefaultIter(tp_iter, map, default) \
	((tp_iter) == &DeeObject_DefaultIterWithForeach ? map(DeeObject_TDefaultIterWithForeach) : \
	 (tp_iter) == &DeeObject_DefaultIterWithForeachPair ? map(DeeObject_TDefaultIterWithForeachPair) : \
	 (tp_iter) == &DeeSeq_DefaultIterWithSizeAndGetItemIndex ? map(DeeSeq_TDefaultIterWithSizeAndGetItemIndex) : \
	 (tp_iter) == &DeeSeq_DefaultIterWithSizeAndGetItemIndexFast ? map(DeeSeq_TDefaultIterWithSizeAndGetItemIndexFast) : \
	 (tp_iter) == &DeeSeq_DefaultIterWithGetItemIndex ? map(DeeSeq_TDefaultIterWithGetItemIndex) : \
	 (tp_iter) == &DeeSeq_DefaultIterWithSizeObDefaultAndGetItemDefault ? map(DeeSeq_TDefaultIterWithSizeObDefaultAndGetItemDefault) : \
	 (tp_iter) == &DeeSeq_DefaultIterWithGetItemDefault ? map(DeeSeq_TDefaultIterWithGetItemDefault) : default)
#define DeeType_MapDefaultForeach(tp_foreach, map, default) \
	((tp_foreach) == &DeeObject_DefaultForeachWithIter ? map(DeeObject_TDefaultForeachWithIter) : \
	 (tp_foreach) == &DeeObject_DefaultForeachWithForeachPair ? map(DeeObject_TDefaultForeachWithForeachPair) : \
	 (tp_foreach) == &DeeSeq_DefaultForeachWithSizeAndGetItemIndexFast ? map(DeeSeq_TDefaultForeachWithSizeAndGetItemIndexFast) : \
	 (tp_foreach) == &DeeSeq_DefaultForeachWithSizeAndGetItemIndex ? map(DeeSeq_TDefaultForeachWithSizeAndGetItemIndex) : \
	 (tp_foreach) == &DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault ? map(DeeSeq_TDefaultForeachWithSizeDefaultAndGetItemIndexDefault) : \
	 (tp_foreach) == &DeeSeq_DefaultForeachWithGetItemIndexDefault ? map(DeeSeq_TDefaultForeachWithGetItemIndexDefault) : default)
#define DeeType_MapDefaultForeachPair(tp_foreach_pair, map, default) \
	((tp_foreach_pair) == &DeeObject_DefaultForeachPairWithForeach ? map(DeeObject_TDefaultForeachPairWithForeach) : \
	 (tp_foreach_pair) == &DeeObject_DefaultForeachPairWithForeachDefault ? map(DeeObject_TDefaultForeachPairWithForeachDefault) : \
	 (tp_foreach_pair) == &DeeObject_DefaultForeachPairWithIter ? map(DeeObject_TDefaultForeachPairWithIter) : default)
#define DeeType_MapDefaultSizeOb(tp_sizeob, map, default) \
	((tp_sizeob) == &DeeObject_DefaultSizeObWithSize ? map(DeeObject_TDefaultSizeObWithSize) : \
	 (tp_sizeob) == &DeeObject_DefaultSizeObWithSizeDefault ? map(DeeObject_TDefaultSizeObWithSizeDefault) : default)
#define DeeType_MapDefaultSize(tp_size, map, default) \
	((tp_size) == &DeeObject_DefaultSizeWithSizeOb ? map(DeeObject_TDefaultSizeWithSizeOb) : \
	 (tp_size) == &DeeSeq_DefaultSizeWithForeachPair ? map(DeeSeq_TDefaultSizeWithForeachPair) : \
	 (tp_size) == &DeeSeq_DefaultSizeWithForeach ? map(DeeSeq_TDefaultSizeWithForeach) : \
	 (tp_size) == &DeeSeq_DefaultSizeWithIter ? map(DeeSeq_TDefaultSizeWithIter) : default)
#define DeeType_MapDefaultContains(tp_contains, map, default) \
	((tp_contains) == &DeeSeq_DefaultContainsWithForeachPair ? map(DeeSeq_TDefaultContainsWithForeachPair) : \
	 (tp_contains) == &DeeSeq_DefaultContainsWithForeachDefault ? map(DeeSeq_TDefaultContainsWithForeachDefault) : default)
#define DeeType_MapDefaultGetItem(tp_getitem, map, default) \
	((tp_getitem) == &DeeObject_DefaultGetItemWithGetItemIndex ? map(DeeObject_TDefaultGetItemWithGetItemIndex) : \
	 (tp_getitem) == &DeeObject_DefaultGetItemWithGetItemIndexDefault ? map(DeeObject_TDefaultGetItemWithGetItemIndexDefault) : default)
#define DeeType_MapDefaultGetItemIndex(tp_getitem_index, map, default) \
	((tp_getitem_index) == &DeeObject_DefaultGetItemIndexWithSizeAndGetItemIndexFast ? map(DeeObject_TDefaultGetItemIndexWithSizeAndGetItemIndexFast) : \
	 (tp_getitem_index) == &DeeObject_DefaultGetItemIndexWithGetItem ? map(DeeObject_TDefaultGetItemIndexWithGetItem) : \
	 (tp_getitem_index) == &DeeSeq_DefaultGetItemIndexWithForeachDefault ? map(DeeSeq_TDefaultGetItemIndexWithForeachDefault) : default)
#define DeeType_MapDefaultDelItem(tp_delitem, map, default) \
	((tp_delitem) == &DeeObject_DefaultDelItemWithDelItemIndex ? map(DeeObject_TDefaultDelItemWithDelItemIndex) : \
	 (tp_delitem) == &DeeObject_DefaultDelItemWithDelItemIndexDefault ? map(DeeObject_TDefaultDelItemWithDelItemIndexDefault) : default)
#define DeeType_MapDefaultDelItemIndex(tp_delitem_index, map, default) \
	((tp_delitem_index) == &DeeObject_DefaultDelItemIndexWithDelItem ? map(DeeObject_TDefaultDelItemIndexWithDelItem) : \
	 (tp_delitem_index) == &DeeSeq_DefaultDelItemIndexWithDelRangeIndexDefault ? map(DeeSeq_TDefaultDelItemIndexWithDelRangeIndexDefault) : default)
#define DeeType_MapDefaultSetItem(tp_setitem, map, default) \
	((tp_setitem) == &DeeObject_DefaultSetItemWithSetItemIndex ? map(DeeObject_TDefaultSetItemWithSetItemIndex) : \
	 (tp_setitem) == &DeeObject_DefaultSetItemWithSetItemIndexDefault ? map(DeeObject_TDefaultSetItemWithSetItemIndexDefault) : default)
#define DeeType_MapDefaultSetItemIndex(tp_setitem_index, map, default) \
	((tp_setitem_index) == &DeeObject_DefaultSetItemIndexWithSetItem ? map(DeeObject_TDefaultSetItemIndexWithSetItem) : \
	 (tp_setitem_index) == &DeeSeq_DefaultSetItemIndexWithSetRangeIndexDefault ? map(DeeSeq_TDefaultSetItemIndexWithSetRangeIndexDefault) : default)
#define DeeType_MapDefaultBoundItem(tp_bounditem, map, default) \
	((tp_bounditem) == &DeeObject_DefaultBoundItemWithBoundItemIndex ? map(DeeObject_TDefaultBoundItemWithBoundItemIndex) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithGetItem ? map(DeeObject_TDefaultBoundItemWithGetItem) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithGetItemIndex ? map(DeeObject_TDefaultBoundItemWithGetItemIndex) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithBoundItemIndexDefault ? map(DeeObject_TDefaultBoundItemWithBoundItemIndexDefault) : default)
#define DeeType_MapDefaultBoundItemIndex(tp_bounditem_index, map, default) \
	((tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithBoundItem ? map(DeeObject_TDefaultBoundItemIndexWithBoundItem) : \
	 (tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithSizeAndGetItemIndexFast ? map(DeeObject_TDefaultBoundItemIndexWithSizeAndGetItemIndexFast) : \
	 (tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithGetItemIndex ? map(DeeObject_TDefaultBoundItemIndexWithGetItemIndex) : \
	 (tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithGetItem ? map(DeeObject_TDefaultBoundItemIndexWithGetItem) : \
	 (tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithGetItemIndexDefault ? map(DeeObject_TDefaultBoundItemIndexWithGetItemIndexDefault) : default)
#define DeeType_MapDefaultHasItem(tp_hasitem, map, default) \
	((tp_hasitem) == &DeeObject_DefaultHasItemWithHasItemIndex ? map(DeeObject_TDefaultHasItemWithHasItemIndex) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithBoundItem ? map(DeeObject_TDefaultHasItemWithBoundItem) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithBoundItemIndex ? map(DeeObject_TDefaultHasItemWithBoundItemIndex) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithGetItem ? map(DeeObject_TDefaultHasItemWithGetItem) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithGetItemIndex ? map(DeeObject_TDefaultHasItemWithGetItemIndex) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithHasItemIndexDefault ? map(DeeObject_TDefaultHasItemWithHasItemIndexDefault) : default)
#define DeeType_MapDefaultHasItemIndex(tp_hasitem_index, map, default) \
	((tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithHasItem ? map(DeeObject_TDefaultHasItemIndexWithHasItem) : \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithBoundItemIndex ? map(DeeObject_TDefaultHasItemIndexWithBoundItemIndex) : \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithBoundItem ? map(DeeObject_TDefaultHasItemIndexWithBoundItem) : \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithGetItemIndex ? map(DeeObject_TDefaultHasItemIndexWithGetItemIndex) : \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithGetItem ? map(DeeObject_TDefaultHasItemIndexWithGetItem) : \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithSize ? map(DeeObject_TDefaultHasItemIndexWithSize) : \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithSizeDefault ? map(DeeObject_TDefaultHasItemIndexWithSizeDefault) : default)
#define DeeType_MapDefaultGetRange(tp_getrange, map, default) \
	((tp_getrange) == &DeeObject_DefaultGetRangeWithGetRangeIndexAndGetRangeIndexN ? map(DeeObject_TDefaultGetRangeWithGetRangeIndexAndGetRangeIndexN) : \
	 (tp_getrange) == &DeeObject_DefaultGetRangeWithGetRangeIndexDefaultAndGetRangeIndexNDefault ? map(DeeObject_TDefaultGetRangeWithGetRangeIndexDefaultAndGetRangeIndexNDefault) : \
	 (tp_getrange) == &DeeObject_DefaultGetRangeWithSizeDefaultAndGetItemIndex ? map(DeeObject_TDefaultGetRangeWithSizeDefaultAndGetItemIndex) : \
	 (tp_getrange) == &DeeObject_DefaultGetRangeWithSizeObAndGetItem ? map(DeeObject_TDefaultGetRangeWithSizeObAndGetItem) : default)
#define DeeType_MapDefaultGetRangeIndex(tp_getrange_index, map, default) \
	((tp_getrange_index) == &DeeObject_DefaultGetRangeIndexWithGetRange ? map(DeeObject_TDefaultGetRangeIndexWithGetRange) : \
	 (tp_getrange_index) == &DeeSeq_DefaultGetRangeIndexWithSizeAndGetItemIndexFast ? map(DeeSeq_TDefaultGetRangeIndexWithSizeAndGetItemIndexFast) : \
	 (tp_getrange_index) == &DeeSeq_DefaultGetRangeIndexWithSizeDefaultAndGetItemIndex ? map(DeeSeq_TDefaultGetRangeIndexWithSizeDefaultAndGetItemIndex) : \
	 (tp_getrange_index) == &DeeSeq_DefaultGetRangeIndexWithSizeDefaultAndGetItem ? map(DeeSeq_TDefaultGetRangeIndexWithSizeDefaultAndGetItem) : \
	 (tp_getrange_index) == &DeeSeq_DefaultGetRangeIndexWithSizeDefaultAndIter ? map(DeeSeq_TDefaultGetRangeIndexWithSizeDefaultAndIter) : \
	 (tp_getrange_index) == &DeeSeq_DefaultGetRangeIndexWithSizeDefaultAndIterDefault ? map(DeeSeq_TDefaultGetRangeIndexWithSizeDefaultAndIterDefault) : default)
#define DeeType_MapDefaultGetRangeIndexN(tp_getrange_index_n, map, default) \
	((tp_getrange_index_n) == &DeeObject_DefaultGetRangeIndexNWithGetRange ? map(DeeObject_TDefaultGetRangeIndexNWithGetRange) : \
	 (tp_getrange_index_n) == &DeeSeq_DefaultGetRangeIndexNWithSizeAndGetRangeIndex ? map(DeeSeq_TDefaultGetRangeIndexNWithSizeAndGetRangeIndex) : \
	 (tp_getrange_index_n) == &DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndGetRangeIndex ? map(DeeSeq_TDefaultGetRangeIndexNWithSizeDefaultAndGetRangeIndex) : \
	 (tp_getrange_index_n) == &DeeSeq_DefaultGetRangeIndexNWithSizeAndGetItemIndexFast ? map(DeeSeq_TDefaultGetRangeIndexNWithSizeAndGetItemIndexFast) : \
	 (tp_getrange_index_n) == &DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndGetItemIndex ? map(DeeSeq_TDefaultGetRangeIndexNWithSizeDefaultAndGetItemIndex) : \
	 (tp_getrange_index_n) == &DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndGetItem ? map(DeeSeq_TDefaultGetRangeIndexNWithSizeDefaultAndGetItem) : \
	 (tp_getrange_index_n) == &DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndIter ? map(DeeSeq_TDefaultGetRangeIndexNWithSizeDefaultAndIter) : \
	 (tp_getrange_index_n) == &DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndIterDefault ? map(DeeSeq_TDefaultGetRangeIndexNWithSizeDefaultAndIterDefault) : default)
#define DeeType_MapDefaultDelRange(tp_delrange, map, default) \
	((tp_delrange) == &DeeObject_DefaultDelRangeWithDelRangeIndexAndDelRangeIndexN ? map(DeeObject_TDefaultDelRangeWithDelRangeIndexAndDelRangeIndexN) : \
	 (tp_delrange) == &DeeObject_DefaultDelRangeWithDelRangeIndexDefaultAndDelRangeIndexNDefault ? map(DeeObject_TDefaultDelRangeWithDelRangeIndexDefaultAndDelRangeIndexNDefault) : \
	 (tp_delrange) == &DeeSeq_DefaultDelRangeWithSetRangeNone ? map(DeeSeq_TDefaultDelRangeWithSetRangeNone) : \
	 (tp_delrange) == &DeeSeq_DefaultDelRangeWithSetRangeNoneDefault ? map(DeeSeq_TDefaultDelRangeWithSetRangeNoneDefault) : default)
#define DeeType_MapDefaultDelRangeIndex(tp_delrange_index, map, default) \
	((tp_delrange_index) == &DeeObject_DefaultDelRangeIndexWithDelRange ? map(DeeObject_TDefaultDelRangeIndexWithDelRange) : \
	 (tp_delrange_index) == &DeeSeq_DefaultDelRangeIndexWithSetRangeIndexNone ? map(DeeSeq_TDefaultDelRangeIndexWithSetRangeIndexNone) : \
	 (tp_delrange_index) == &DeeSeq_DefaultDelRangeIndexWithSetRangeIndexNoneDefault ? map(DeeSeq_TDefaultDelRangeIndexWithSetRangeIndexNoneDefault) : default)
#define DeeType_MapDefaultDelRangeIndexN(tp_delrange_index_n, map, default) \
	((tp_delrange_index_n) == &DeeObject_DefaultDelRangeIndexNWithDelRange ? map(DeeObject_TDefaultDelRangeIndexNWithDelRange) : \
	 (tp_delrange_index_n) == &DeeSeq_DefaultDelRangeIndexNWithSizeAndDelRangeIndex ? map(DeeSeq_TDefaultDelRangeIndexNWithSizeAndDelRangeIndex) : \
	 (tp_delrange_index_n) == &DeeSeq_DefaultDelRangeIndexNWithSizeDefaultAndDelRangeIndex ? map(DeeSeq_TDefaultDelRangeIndexNWithSizeDefaultAndDelRangeIndex) : \
	 (tp_delrange_index_n) == &DeeSeq_DefaultDelRangeIndexNWithSetRangeIndexNNone ? map(DeeSeq_TDefaultDelRangeIndexNWithSetRangeIndexNNone) : \
	 (tp_delrange_index_n) == &DeeSeq_DefaultDelRangeIndexNWithSetRangeIndexNNoneDefault ? map(DeeSeq_TDefaultDelRangeIndexNWithSetRangeIndexNNoneDefault) : default)
#define DeeType_MapDefaultSetRange(tp_setrange, map, default) \
	((tp_setrange) == &DeeObject_DefaultSetRangeWithSetRangeIndexAndSetRangeIndexN ? map(DeeObject_TDefaultSetRangeWithSetRangeIndexAndSetRangeIndexN) : \
	 (tp_setrange) == &DeeObject_DefaultSetRangeWithSetRangeIndexDefaultAndSetRangeIndexNDefault ? map(DeeObject_TDefaultSetRangeWithSetRangeIndexDefaultAndSetRangeIndexNDefault) : default)
#define DeeType_MapDefaultSetRangeIndex(tp_setrange_index, map, default) \
	((tp_setrange_index) == &DeeObject_DefaultSetRangeIndexWithSetRange ? map(DeeObject_TDefaultSetRangeIndexWithSetRange) : \
	 (tp_setrange_index) == &DeeSeq_DefaultSetRangeIndexWithSizeAndDelItemIndexAndSetItemIndex ? map(DeeSeq_TDefaultSetRangeIndexWithSizeAndDelItemIndexAndSetItemIndex) : \
	 (tp_setrange_index) == &DeeSeq_DefaultSetRangeIndexWithSizeDefaultAndDelItemIndexDefaultAndSetItemIndexDefault ? map(DeeSeq_TDefaultSetRangeIndexWithSizeDefaultAndDelItemIndexDefaultAndSetItemIndexDefault) : \
	 (tp_setrange_index) == &DeeSeq_DefaultSetRangeIndexWithSizeAndSetItemIndex ? map(DeeSeq_TDefaultSetRangeIndexWithSizeAndSetItemIndex) : \
	 (tp_setrange_index) == &DeeSeq_DefaultSetRangeIndexWithSizeDefaultAndSetItemIndexDefault ? map(DeeSeq_TDefaultSetRangeIndexWithSizeDefaultAndSetItemIndexDefault) : default)
#define DeeType_MapDefaultSetRangeIndexN(tp_setrange_index_n, map, default) \
	((tp_setrange_index_n) == &DeeObject_DefaultSetRangeIndexNWithSetRange ? map(DeeObject_TDefaultSetRangeIndexNWithSetRange) : \
	 (tp_setrange_index_n) == &DeeSeq_DefaultSetRangeIndexNWithSizeAndSetRangeIndex ? map(DeeSeq_TDefaultSetRangeIndexNWithSizeAndSetRangeIndex) : \
	 (tp_setrange_index_n) == &DeeSeq_DefaultSetRangeIndexNWithSizeDefaultAndSetRangeIndex ? map(DeeSeq_TDefaultSetRangeIndexNWithSizeDefaultAndSetRangeIndex) : \
	 (tp_setrange_index_n) == &DeeSeq_DefaultSetRangeIndexNWithSizeAndDelItemIndexAndSetItemIndex ? map(DeeSeq_TDefaultSetRangeIndexNWithSizeAndDelItemIndexAndSetItemIndex) : \
	 (tp_setrange_index_n) == &DeeSeq_DefaultSetRangeIndexNWithSizeDefaultAndDelItemIndexDefaultAndSetItemIndexDefault ? map(DeeSeq_TDefaultSetRangeIndexNWithSizeDefaultAndDelItemIndexDefaultAndSetItemIndexDefault) : \
	 (tp_setrange_index_n) == &DeeSeq_DefaultSetRangeIndexNWithSizeAndSetItemIndex ? map(DeeSeq_TDefaultSetRangeIndexNWithSizeAndSetItemIndex) : \
	 (tp_setrange_index_n) == &DeeSeq_DefaultSetRangeIndexNWithSizeDefaultAndSetItemIndexDefault ? map(DeeSeq_TDefaultSetRangeIndexNWithSizeDefaultAndSetItemIndexDefault) : default)
#define DeeType_IsDefaultStr(tp_str) \
	((tp_str) == &DeeObject_DefaultStrWithPrint)
#define DeeType_IsDefaultRepr(tp_repr) \
	((tp_repr) == &DeeObject_DefaultReprWithPrintRepr)
#define DeeType_IsDefaultPrint(tp_print) \
	((tp_print) == &DeeObject_DefaultPrintWithStr)
#define DeeType_IsDefaultPrintRepr(tp_printrepr) \
	((tp_printrepr) == &DeeObject_DefaultPrintReprWithRepr)
#define DeeType_IsDefaultCall(tp_call) \
	((tp_call) == &DeeObject_DefaultCallWithCallKw)
#define DeeType_IsDefaultCallKw(tp_call_kw) \
	((tp_call_kw) == &DeeObject_DefaultCallKwWithCall)
#define DeeType_IsDefaultInt32(tp_int32) \
	((tp_int32) == &DeeObject_DefaultInt32WithInt || \
	 (tp_int32) == &DeeObject_DefaultInt32WithInt64 || \
	 (tp_int32) == &DeeObject_DefaultInt32WithDouble)
#define DeeType_IsDefaultInt64(tp_int64) \
	((tp_int64) == &DeeObject_DefaultInt64WithInt || \
	 (tp_int64) == &DeeObject_DefaultInt64WithInt32 || \
	 (tp_int64) == &DeeObject_DefaultInt64WithDouble)
#define DeeType_IsDefaultDouble(tp_double) \
	((tp_double) == &DeeObject_DefaultDoubleWithInt || \
	 (tp_double) == &DeeObject_DefaultDoubleWithInt32 || \
	 (tp_double) == &DeeObject_DefaultDoubleWithInt64)
#define DeeType_IsDefaultInt(tp_int) \
	((tp_int) == &DeeObject_DefaultIntWithInt32 || \
	 (tp_int) == &DeeObject_DefaultIntWithInt64 || \
	 (tp_int) == &DeeObject_DefaultIntWithDouble)
#define DeeType_IsDefaultAdd(tp_add) \
	((tp_add) == &DeeObject_DefaultAddWithInplaceAdd || \
	 (tp_add) == &DeeObject_DefaultAddWithSub || \
	 (tp_add) == &DeeObject_DefaultAddWithInplaceSub)
#define DeeType_IsDefaultSub(tp_sub) \
	((tp_sub) == &DeeObject_DefaultSubWithInplaceSub || \
	 (tp_sub) == &DeeObject_DefaultSubWithAdd || \
	 (tp_sub) == &DeeObject_DefaultSubWithInplaceAdd)
#define DeeType_IsDefaultMul(tp_mul) \
	((tp_mul) == &DeeObject_DefaultMulWithInplaceMul)
#define DeeType_IsDefaultDiv(tp_div) \
	((tp_div) == &DeeObject_DefaultDivWithInplaceDiv)
#define DeeType_IsDefaultMod(tp_mod) \
	((tp_mod) == &DeeObject_DefaultModWithInplaceMod)
#define DeeType_IsDefaultShl(tp_shl) \
	((tp_shl) == &DeeObject_DefaultShlWithInplaceShl)
#define DeeType_IsDefaultShr(tp_shr) \
	((tp_shr) == &DeeObject_DefaultShrWithInplaceShr)
#define DeeType_IsDefaultAnd(tp_and) \
	((tp_and) == &DeeObject_DefaultAndWithInplaceAnd)
#define DeeType_IsDefaultOr(tp_or) \
	((tp_or) == &DeeObject_DefaultOrWithInplaceOr)
#define DeeType_IsDefaultXor(tp_xor) \
	((tp_xor) == &DeeObject_DefaultXorWithInplaceXor)
#define DeeType_IsDefaultPow(tp_pow) \
	((tp_pow) == &DeeObject_DefaultPowWithInplacePow)
#define DeeType_IsDefaultInplaceAdd(tp_inplace_add) \
	((tp_inplace_add) == &DeeObject_DefaultInplaceAddWithAdd || \
	 (tp_inplace_add) == &DeeObject_DefaultInplaceAddWithSub || \
	 (tp_inplace_add) == &DeeObject_DefaultInplaceAddWithInplaceSub)
#define DeeType_IsDefaultInplaceSub(tp_inplace_sub) \
	((tp_inplace_sub) == &DeeObject_DefaultInplaceSubWithSub || \
	 (tp_inplace_sub) == &DeeObject_DefaultInplaceSubWithAdd || \
	 (tp_inplace_sub) == &DeeObject_DefaultInplaceSubWithInplaceAdd)
#define DeeType_IsDefaultInplaceMul(tp_inplace_mul) \
	((tp_inplace_mul) == &DeeObject_DefaultInplaceMulWithMul)
#define DeeType_IsDefaultInplaceDiv(tp_inplace_div) \
	((tp_inplace_div) == &DeeObject_DefaultInplaceDivWithDiv)
#define DeeType_IsDefaultInplaceMod(tp_inplace_mod) \
	((tp_inplace_mod) == &DeeObject_DefaultInplaceModWithMod)
#define DeeType_IsDefaultInplaceShl(tp_inplace_shl) \
	((tp_inplace_shl) == &DeeObject_DefaultInplaceShlWithShl)
#define DeeType_IsDefaultInplaceShr(tp_inplace_shr) \
	((tp_inplace_shr) == &DeeObject_DefaultInplaceShrWithShr)
#define DeeType_IsDefaultInplaceAnd(tp_inplace_and) \
	((tp_inplace_and) == &DeeObject_DefaultInplaceAndWithAnd)
#define DeeType_IsDefaultInplaceOr(tp_inplace_or) \
	((tp_inplace_or) == &DeeObject_DefaultInplaceOrWithOr)
#define DeeType_IsDefaultInplaceXor(tp_inplace_xor) \
	((tp_inplace_xor) == &DeeObject_DefaultInplaceXorWithXor)
#define DeeType_IsDefaultInplacePow(tp_inplace_pow) \
	((tp_inplace_pow) == &DeeObject_DefaultInplacePowWithPow)
#define DeeType_IsDefaultInc(tp_inc) \
	((tp_inc) == &DeeObject_DefaultIncWithInplaceAdd || \
	 (tp_inc) == &DeeObject_DefaultIncWithAdd || \
	 (tp_inc) == &DeeObject_DefaultIncWithInplaceSub || \
	 (tp_inc) == &DeeObject_DefaultIncWithSub)
#define DeeType_IsDefaultDec(tp_dec) \
	((tp_dec) == &DeeObject_DefaultDecWithInplaceAdd || \
	 (tp_dec) == &DeeObject_DefaultDecWithAdd || \
	 (tp_dec) == &DeeObject_DefaultDecWithInplaceSub || \
	 (tp_dec) == &DeeObject_DefaultDecWithSub)
#define DeeType_IsDefaultEq(tp_eq) \
	((tp_eq) == &DeeObject_DefaultEqWithNe)
#define DeeType_IsDefaultNe(tp_ne) \
	((tp_ne) == &DeeObject_DefaultNeWithEq)
#define DeeType_IsDefaultLo(tp_lo) \
	((tp_lo) == &DeeObject_DefaultLoWithGe)
#define DeeType_IsDefaultLe(tp_le) \
	((tp_le) == &DeeObject_DefaultLeWithGr)
#define DeeType_IsDefaultGr(tp_gr) \
	((tp_gr) == &DeeObject_DefaultGrWithLe)
#define DeeType_IsDefaultGe(tp_ge) \
	((tp_ge) == &DeeObject_DefaultGeWithLo)
#define DeeType_IsDefaultIter(tp_iter) \
	((tp_iter) == &DeeObject_DefaultIterWithForeach || \
	 (tp_iter) == &DeeObject_DefaultIterWithForeachPair || \
	 (tp_iter) == &DeeSeq_DefaultIterWithSizeAndGetItemIndex || \
	 (tp_iter) == &DeeSeq_DefaultIterWithSizeAndGetItemIndexFast || \
	 (tp_iter) == &DeeSeq_DefaultIterWithGetItemIndex || \
	 (tp_iter) == &DeeSeq_DefaultIterWithSizeObDefaultAndGetItemDefault || \
	 (tp_iter) == &DeeSeq_DefaultIterWithGetItemDefault)
#define DeeType_IsDefaultForeach(tp_foreach) \
	((tp_foreach) == &DeeObject_DefaultForeachWithIter || \
	 (tp_foreach) == &DeeObject_DefaultForeachWithForeachPair || \
	 (tp_foreach) == &DeeSeq_DefaultForeachWithSizeAndGetItemIndexFast || \
	 (tp_foreach) == &DeeSeq_DefaultForeachWithSizeAndGetItemIndex || \
	 (tp_foreach) == &DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault || \
	 (tp_foreach) == &DeeSeq_DefaultForeachWithGetItemIndexDefault)
#define DeeType_IsDefaultForeachPair(tp_foreach_pair) \
	((tp_foreach_pair) == &DeeObject_DefaultForeachPairWithForeach || \
	 (tp_foreach_pair) == &DeeObject_DefaultForeachPairWithForeachDefault || \
	 (tp_foreach_pair) == &DeeObject_DefaultForeachPairWithIter)
#define DeeType_IsDefaultSizeOb(tp_sizeob) \
	((tp_sizeob) == &DeeObject_DefaultSizeObWithSize || \
	 (tp_sizeob) == &DeeObject_DefaultSizeObWithSizeDefault)
#define DeeType_IsDefaultSize(tp_size) \
	((tp_size) == &DeeObject_DefaultSizeWithSizeOb || \
	 (tp_size) == &DeeSeq_DefaultSizeWithForeachPair || \
	 (tp_size) == &DeeSeq_DefaultSizeWithForeach || \
	 (tp_size) == &DeeSeq_DefaultSizeWithIter)
#define DeeType_IsDefaultContains(tp_contains) \
	((tp_contains) == &DeeSeq_DefaultContainsWithForeachPair || \
	 (tp_contains) == &DeeSeq_DefaultContainsWithForeachDefault)
#define DeeType_IsDefaultGetItem(tp_getitem) \
	((tp_getitem) == &DeeObject_DefaultGetItemWithGetItemIndex || \
	 (tp_getitem) == &DeeObject_DefaultGetItemWithGetItemIndexDefault)
#define DeeType_IsDefaultGetItemIndex(tp_getitem_index) \
	((tp_getitem_index) == &DeeObject_DefaultGetItemIndexWithSizeAndGetItemIndexFast || \
	 (tp_getitem_index) == &DeeObject_DefaultGetItemIndexWithGetItem || \
	 (tp_getitem_index) == &DeeSeq_DefaultGetItemIndexWithForeachDefault)
#define DeeType_IsDefaultDelItem(tp_delitem) \
	((tp_delitem) == &DeeObject_DefaultDelItemWithDelItemIndex || \
	 (tp_delitem) == &DeeObject_DefaultDelItemWithDelItemIndexDefault)
#define DeeType_IsDefaultDelItemIndex(tp_delitem_index) \
	((tp_delitem_index) == &DeeObject_DefaultDelItemIndexWithDelItem || \
	 (tp_delitem_index) == &DeeSeq_DefaultDelItemIndexWithDelRangeIndexDefault)
#define DeeType_IsDefaultSetItem(tp_setitem) \
	((tp_setitem) == &DeeObject_DefaultSetItemWithSetItemIndex || \
	 (tp_setitem) == &DeeObject_DefaultSetItemWithSetItemIndexDefault)
#define DeeType_IsDefaultSetItemIndex(tp_setitem_index) \
	((tp_setitem_index) == &DeeObject_DefaultSetItemIndexWithSetItem || \
	 (tp_setitem_index) == &DeeSeq_DefaultSetItemIndexWithSetRangeIndexDefault)
#define DeeType_IsDefaultBoundItem(tp_bounditem) \
	((tp_bounditem) == &DeeObject_DefaultBoundItemWithBoundItemIndex || \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithGetItem || \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithGetItemIndex || \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithBoundItemIndexDefault)
#define DeeType_IsDefaultBoundItemIndex(tp_bounditem_index) \
	((tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithBoundItem || \
	 (tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithSizeAndGetItemIndexFast || \
	 (tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithGetItemIndex || \
	 (tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithGetItem || \
	 (tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithGetItemIndexDefault)
#define DeeType_IsDefaultHasItem(tp_hasitem) \
	((tp_hasitem) == &DeeObject_DefaultHasItemWithHasItemIndex || \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithBoundItem || \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithBoundItemIndex || \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithGetItem || \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithGetItemIndex || \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithHasItemIndexDefault)
#define DeeType_IsDefaultHasItemIndex(tp_hasitem_index) \
	((tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithHasItem || \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithBoundItemIndex || \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithBoundItem || \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithGetItemIndex || \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithGetItem || \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithSize || \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithSizeDefault)
#define DeeType_IsDefaultGetRange(tp_getrange) \
	((tp_getrange) == &DeeObject_DefaultGetRangeWithGetRangeIndexAndGetRangeIndexN || \
	 (tp_getrange) == &DeeObject_DefaultGetRangeWithGetRangeIndexDefaultAndGetRangeIndexNDefault || \
	 (tp_getrange) == &DeeObject_DefaultGetRangeWithSizeDefaultAndGetItemIndex || \
	 (tp_getrange) == &DeeObject_DefaultGetRangeWithSizeObAndGetItem)
#define DeeType_IsDefaultGetRangeIndex(tp_getrange_index) \
	((tp_getrange_index) == &DeeObject_DefaultGetRangeIndexWithGetRange || \
	 (tp_getrange_index) == &DeeSeq_DefaultGetRangeIndexWithSizeAndGetItemIndexFast || \
	 (tp_getrange_index) == &DeeSeq_DefaultGetRangeIndexWithSizeDefaultAndGetItemIndex || \
	 (tp_getrange_index) == &DeeSeq_DefaultGetRangeIndexWithSizeDefaultAndGetItem || \
	 (tp_getrange_index) == &DeeSeq_DefaultGetRangeIndexWithSizeDefaultAndIter || \
	 (tp_getrange_index) == &DeeSeq_DefaultGetRangeIndexWithSizeDefaultAndIterDefault)
#define DeeType_IsDefaultGetRangeIndexN(tp_getrange_index_n) \
	((tp_getrange_index_n) == &DeeObject_DefaultGetRangeIndexNWithGetRange || \
	 (tp_getrange_index_n) == &DeeSeq_DefaultGetRangeIndexNWithSizeAndGetRangeIndex || \
	 (tp_getrange_index_n) == &DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndGetRangeIndex || \
	 (tp_getrange_index_n) == &DeeSeq_DefaultGetRangeIndexNWithSizeAndGetItemIndexFast || \
	 (tp_getrange_index_n) == &DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndGetItemIndex || \
	 (tp_getrange_index_n) == &DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndGetItem || \
	 (tp_getrange_index_n) == &DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndIter || \
	 (tp_getrange_index_n) == &DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndIterDefault)
#define DeeType_IsDefaultDelRange(tp_delrange) \
	((tp_delrange) == &DeeObject_DefaultDelRangeWithDelRangeIndexAndDelRangeIndexN || \
	 (tp_delrange) == &DeeObject_DefaultDelRangeWithDelRangeIndexDefaultAndDelRangeIndexNDefault || \
	 (tp_delrange) == &DeeSeq_DefaultDelRangeWithSetRangeNone || \
	 (tp_delrange) == &DeeSeq_DefaultDelRangeWithSetRangeNoneDefault)
#define DeeType_IsDefaultDelRangeIndex(tp_delrange_index) \
	((tp_delrange_index) == &DeeObject_DefaultDelRangeIndexWithDelRange || \
	 (tp_delrange_index) == &DeeSeq_DefaultDelRangeIndexWithSetRangeIndexNone || \
	 (tp_delrange_index) == &DeeSeq_DefaultDelRangeIndexWithSetRangeIndexNoneDefault)
#define DeeType_IsDefaultDelRangeIndexN(tp_delrange_index_n) \
	((tp_delrange_index_n) == &DeeObject_DefaultDelRangeIndexNWithDelRange || \
	 (tp_delrange_index_n) == &DeeSeq_DefaultDelRangeIndexNWithSizeAndDelRangeIndex || \
	 (tp_delrange_index_n) == &DeeSeq_DefaultDelRangeIndexNWithSizeDefaultAndDelRangeIndex || \
	 (tp_delrange_index_n) == &DeeSeq_DefaultDelRangeIndexNWithSetRangeIndexNNone || \
	 (tp_delrange_index_n) == &DeeSeq_DefaultDelRangeIndexNWithSetRangeIndexNNoneDefault)
#define DeeType_IsDefaultSetRange(tp_setrange) \
	((tp_setrange) == &DeeObject_DefaultSetRangeWithSetRangeIndexAndSetRangeIndexN || \
	 (tp_setrange) == &DeeObject_DefaultSetRangeWithSetRangeIndexDefaultAndSetRangeIndexNDefault)
#define DeeType_IsDefaultSetRangeIndex(tp_setrange_index) \
	((tp_setrange_index) == &DeeObject_DefaultSetRangeIndexWithSetRange || \
	 (tp_setrange_index) == &DeeSeq_DefaultSetRangeIndexWithSizeAndDelItemIndexAndSetItemIndex || \
	 (tp_setrange_index) == &DeeSeq_DefaultSetRangeIndexWithSizeDefaultAndDelItemIndexDefaultAndSetItemIndexDefault || \
	 (tp_setrange_index) == &DeeSeq_DefaultSetRangeIndexWithSizeAndSetItemIndex || \
	 (tp_setrange_index) == &DeeSeq_DefaultSetRangeIndexWithSizeDefaultAndSetItemIndexDefault)
#define DeeType_IsDefaultSetRangeIndexN(tp_setrange_index_n) \
	((tp_setrange_index_n) == &DeeObject_DefaultSetRangeIndexNWithSetRange || \
	 (tp_setrange_index_n) == &DeeSeq_DefaultSetRangeIndexNWithSizeAndSetRangeIndex || \
	 (tp_setrange_index_n) == &DeeSeq_DefaultSetRangeIndexNWithSizeDefaultAndSetRangeIndex || \
	 (tp_setrange_index_n) == &DeeSeq_DefaultSetRangeIndexNWithSizeAndDelItemIndexAndSetItemIndex || \
	 (tp_setrange_index_n) == &DeeSeq_DefaultSetRangeIndexNWithSizeDefaultAndDelItemIndexDefaultAndSetItemIndexDefault || \
	 (tp_setrange_index_n) == &DeeSeq_DefaultSetRangeIndexNWithSizeAndSetItemIndex || \
	 (tp_setrange_index_n) == &DeeSeq_DefaultSetRangeIndexNWithSizeDefaultAndSetItemIndexDefault)
#define DeeType_invoke_cast_tp_str_DEFAULT(tp_self, tp_str, self, default) \
	((tp_str) == &DeeObject_DefaultStrWithPrint ? DeeObject_TDefaultStrWithPrint(tp_self, self) : \
	 default)
#define DeeType_invoke_cast_tp_repr_DEFAULT(tp_self, tp_repr, self, default) \
	((tp_repr) == &DeeObject_DefaultReprWithPrintRepr ? DeeObject_TDefaultReprWithPrintRepr(tp_self, self) : \
	 default)
#define DeeType_invoke_cast_tp_print_DEFAULT(tp_self, tp_print, self, printer, arg, default) \
	((tp_print) == &DeeObject_DefaultPrintWithStr ? DeeObject_TDefaultPrintWithStr(tp_self, self, printer, arg) : \
	 default)
#define DeeType_invoke_cast_tp_printrepr_DEFAULT(tp_self, tp_printrepr, self, printer, arg, default) \
	((tp_printrepr) == &DeeObject_DefaultPrintReprWithRepr ? DeeObject_TDefaultPrintReprWithRepr(tp_self, self, printer, arg) : \
	 default)
#define DeeType_invoke_tp_call_DEFAULT(tp_self, tp_call, self, argc, argv, default) \
	((tp_call) == &DeeObject_DefaultCallWithCallKw ? DeeObject_TDefaultCallWithCallKw(tp_self, self, argc, argv) : \
	 default)
#define DeeType_invoke_tp_call_kw_DEFAULT(tp_self, tp_call_kw, self, argc, argv, kw, default) \
	((tp_call_kw) == &DeeObject_DefaultCallKwWithCall ? DeeObject_TDefaultCallKwWithCall(tp_self, self, argc, argv, kw) : \
	 default)
#define DeeType_invoke_math_tp_int32_DEFAULT(tp_self, tp_int32, self, result, default) \
	((tp_int32) == &DeeObject_DefaultInt32WithInt ? DeeObject_TDefaultInt32WithInt(tp_self, self, result) : \
	 (tp_int32) == &DeeObject_DefaultInt32WithInt64 ? DeeObject_TDefaultInt32WithInt64(tp_self, self, result) : \
	 (tp_int32) == &DeeObject_DefaultInt32WithDouble ? DeeObject_TDefaultInt32WithDouble(tp_self, self, result) : \
	 default)
#define DeeType_invoke_math_tp_int64_DEFAULT(tp_self, tp_int64, self, result, default) \
	((tp_int64) == &DeeObject_DefaultInt64WithInt ? DeeObject_TDefaultInt64WithInt(tp_self, self, result) : \
	 (tp_int64) == &DeeObject_DefaultInt64WithInt32 ? DeeObject_TDefaultInt64WithInt32(tp_self, self, result) : \
	 (tp_int64) == &DeeObject_DefaultInt64WithDouble ? DeeObject_TDefaultInt64WithDouble(tp_self, self, result) : \
	 default)
#define DeeType_invoke_math_tp_double_DEFAULT(tp_self, tp_double, self, result, default) \
	((tp_double) == &DeeObject_DefaultDoubleWithInt ? DeeObject_TDefaultDoubleWithInt(tp_self, self, result) : \
	 (tp_double) == &DeeObject_DefaultDoubleWithInt32 ? DeeObject_TDefaultDoubleWithInt32(tp_self, self, result) : \
	 (tp_double) == &DeeObject_DefaultDoubleWithInt64 ? DeeObject_TDefaultDoubleWithInt64(tp_self, self, result) : \
	 default)
#define DeeType_invoke_math_tp_int_DEFAULT(tp_self, tp_int, self, default) \
	((tp_int) == &DeeObject_DefaultIntWithInt32 ? DeeObject_TDefaultIntWithInt32(tp_self, self) : \
	 (tp_int) == &DeeObject_DefaultIntWithInt64 ? DeeObject_TDefaultIntWithInt64(tp_self, self) : \
	 (tp_int) == &DeeObject_DefaultIntWithDouble ? DeeObject_TDefaultIntWithDouble(tp_self, self) : \
	 default)
#define DeeType_invoke_math_tp_add_DEFAULT(tp_self, tp_add, self, other, default) \
	((tp_add) == &DeeObject_DefaultAddWithInplaceAdd ? DeeObject_TDefaultAddWithInplaceAdd(tp_self, self, other) : \
	 (tp_add) == &DeeObject_DefaultAddWithSub ? DeeObject_TDefaultAddWithSub(tp_self, self, other) : \
	 (tp_add) == &DeeObject_DefaultAddWithInplaceSub ? DeeObject_TDefaultAddWithInplaceSub(tp_self, self, other) : \
	 default)
#define DeeType_invoke_math_tp_sub_DEFAULT(tp_self, tp_sub, self, other, default) \
	((tp_sub) == &DeeObject_DefaultSubWithInplaceSub ? DeeObject_TDefaultSubWithInplaceSub(tp_self, self, other) : \
	 (tp_sub) == &DeeObject_DefaultSubWithAdd ? DeeObject_TDefaultSubWithAdd(tp_self, self, other) : \
	 (tp_sub) == &DeeObject_DefaultSubWithInplaceAdd ? DeeObject_TDefaultSubWithInplaceAdd(tp_self, self, other) : \
	 default)
#define DeeType_invoke_math_tp_mul_DEFAULT(tp_self, tp_mul, self, other, default) \
	((tp_mul) == &DeeObject_DefaultMulWithInplaceMul ? DeeObject_TDefaultMulWithInplaceMul(tp_self, self, other) : \
	 default)
#define DeeType_invoke_math_tp_div_DEFAULT(tp_self, tp_div, self, other, default) \
	((tp_div) == &DeeObject_DefaultDivWithInplaceDiv ? DeeObject_TDefaultDivWithInplaceDiv(tp_self, self, other) : \
	 default)
#define DeeType_invoke_math_tp_mod_DEFAULT(tp_self, tp_mod, self, other, default) \
	((tp_mod) == &DeeObject_DefaultModWithInplaceMod ? DeeObject_TDefaultModWithInplaceMod(tp_self, self, other) : \
	 default)
#define DeeType_invoke_math_tp_shl_DEFAULT(tp_self, tp_shl, self, other, default) \
	((tp_shl) == &DeeObject_DefaultShlWithInplaceShl ? DeeObject_TDefaultShlWithInplaceShl(tp_self, self, other) : \
	 default)
#define DeeType_invoke_math_tp_shr_DEFAULT(tp_self, tp_shr, self, other, default) \
	((tp_shr) == &DeeObject_DefaultShrWithInplaceShr ? DeeObject_TDefaultShrWithInplaceShr(tp_self, self, other) : \
	 default)
#define DeeType_invoke_math_tp_and_DEFAULT(tp_self, tp_and, self, other, default) \
	((tp_and) == &DeeObject_DefaultAndWithInplaceAnd ? DeeObject_TDefaultAndWithInplaceAnd(tp_self, self, other) : \
	 default)
#define DeeType_invoke_math_tp_or_DEFAULT(tp_self, tp_or, self, other, default) \
	((tp_or) == &DeeObject_DefaultOrWithInplaceOr ? DeeObject_TDefaultOrWithInplaceOr(tp_self, self, other) : \
	 default)
#define DeeType_invoke_math_tp_xor_DEFAULT(tp_self, tp_xor, self, other, default) \
	((tp_xor) == &DeeObject_DefaultXorWithInplaceXor ? DeeObject_TDefaultXorWithInplaceXor(tp_self, self, other) : \
	 default)
#define DeeType_invoke_math_tp_pow_DEFAULT(tp_self, tp_pow, self, other, default) \
	((tp_pow) == &DeeObject_DefaultPowWithInplacePow ? DeeObject_TDefaultPowWithInplacePow(tp_self, self, other) : \
	 default)
#define DeeType_invoke_math_tp_inplace_add_DEFAULT(tp_self, tp_inplace_add, p_self, other, default) \
	((tp_inplace_add) == &DeeObject_DefaultInplaceAddWithAdd ? DeeObject_TDefaultInplaceAddWithAdd(tp_self, p_self, other) : \
	 (tp_inplace_add) == &DeeObject_DefaultInplaceAddWithSub ? DeeObject_TDefaultInplaceAddWithSub(tp_self, p_self, other) : \
	 (tp_inplace_add) == &DeeObject_DefaultInplaceAddWithInplaceSub ? DeeObject_TDefaultInplaceAddWithInplaceSub(tp_self, p_self, other) : \
	 default)
#define DeeType_invoke_math_tp_inplace_sub_DEFAULT(tp_self, tp_inplace_sub, p_self, other, default) \
	((tp_inplace_sub) == &DeeObject_DefaultInplaceSubWithSub ? DeeObject_TDefaultInplaceSubWithSub(tp_self, p_self, other) : \
	 (tp_inplace_sub) == &DeeObject_DefaultInplaceSubWithAdd ? DeeObject_TDefaultInplaceSubWithAdd(tp_self, p_self, other) : \
	 (tp_inplace_sub) == &DeeObject_DefaultInplaceSubWithInplaceAdd ? DeeObject_TDefaultInplaceSubWithInplaceAdd(tp_self, p_self, other) : \
	 default)
#define DeeType_invoke_math_tp_inplace_mul_DEFAULT(tp_self, tp_inplace_mul, p_self, other, default) \
	((tp_inplace_mul) == &DeeObject_DefaultInplaceMulWithMul ? DeeObject_TDefaultInplaceMulWithMul(tp_self, p_self, other) : \
	 default)
#define DeeType_invoke_math_tp_inplace_div_DEFAULT(tp_self, tp_inplace_div, p_self, other, default) \
	((tp_inplace_div) == &DeeObject_DefaultInplaceDivWithDiv ? DeeObject_TDefaultInplaceDivWithDiv(tp_self, p_self, other) : \
	 default)
#define DeeType_invoke_math_tp_inplace_mod_DEFAULT(tp_self, tp_inplace_mod, p_self, other, default) \
	((tp_inplace_mod) == &DeeObject_DefaultInplaceModWithMod ? DeeObject_TDefaultInplaceModWithMod(tp_self, p_self, other) : \
	 default)
#define DeeType_invoke_math_tp_inplace_shl_DEFAULT(tp_self, tp_inplace_shl, p_self, other, default) \
	((tp_inplace_shl) == &DeeObject_DefaultInplaceShlWithShl ? DeeObject_TDefaultInplaceShlWithShl(tp_self, p_self, other) : \
	 default)
#define DeeType_invoke_math_tp_inplace_shr_DEFAULT(tp_self, tp_inplace_shr, p_self, other, default) \
	((tp_inplace_shr) == &DeeObject_DefaultInplaceShrWithShr ? DeeObject_TDefaultInplaceShrWithShr(tp_self, p_self, other) : \
	 default)
#define DeeType_invoke_math_tp_inplace_and_DEFAULT(tp_self, tp_inplace_and, p_self, other, default) \
	((tp_inplace_and) == &DeeObject_DefaultInplaceAndWithAnd ? DeeObject_TDefaultInplaceAndWithAnd(tp_self, p_self, other) : \
	 default)
#define DeeType_invoke_math_tp_inplace_or_DEFAULT(tp_self, tp_inplace_or, p_self, other, default) \
	((tp_inplace_or) == &DeeObject_DefaultInplaceOrWithOr ? DeeObject_TDefaultInplaceOrWithOr(tp_self, p_self, other) : \
	 default)
#define DeeType_invoke_math_tp_inplace_xor_DEFAULT(tp_self, tp_inplace_xor, p_self, other, default) \
	((tp_inplace_xor) == &DeeObject_DefaultInplaceXorWithXor ? DeeObject_TDefaultInplaceXorWithXor(tp_self, p_self, other) : \
	 default)
#define DeeType_invoke_math_tp_inplace_pow_DEFAULT(tp_self, tp_inplace_pow, p_self, other, default) \
	((tp_inplace_pow) == &DeeObject_DefaultInplacePowWithPow ? DeeObject_TDefaultInplacePowWithPow(tp_self, p_self, other) : \
	 default)
#define DeeType_invoke_math_tp_inc_DEFAULT(tp_self, tp_inc, p_self, default) \
	((tp_inc) == &DeeObject_DefaultIncWithInplaceAdd ? DeeObject_TDefaultIncWithInplaceAdd(tp_self, p_self) : \
	 (tp_inc) == &DeeObject_DefaultIncWithAdd ? DeeObject_TDefaultIncWithAdd(tp_self, p_self) : \
	 (tp_inc) == &DeeObject_DefaultIncWithInplaceSub ? DeeObject_TDefaultIncWithInplaceSub(tp_self, p_self) : \
	 (tp_inc) == &DeeObject_DefaultIncWithSub ? DeeObject_TDefaultIncWithSub(tp_self, p_self) : \
	 default)
#define DeeType_invoke_math_tp_dec_DEFAULT(tp_self, tp_dec, p_self, default) \
	((tp_dec) == &DeeObject_DefaultDecWithInplaceAdd ? DeeObject_TDefaultDecWithInplaceAdd(tp_self, p_self) : \
	 (tp_dec) == &DeeObject_DefaultDecWithAdd ? DeeObject_TDefaultDecWithAdd(tp_self, p_self) : \
	 (tp_dec) == &DeeObject_DefaultDecWithInplaceSub ? DeeObject_TDefaultDecWithInplaceSub(tp_self, p_self) : \
	 (tp_dec) == &DeeObject_DefaultDecWithSub ? DeeObject_TDefaultDecWithSub(tp_self, p_self) : \
	 default)
#define DeeType_invoke_cmp_tp_eq_DEFAULT(tp_self, tp_eq, self, other, default) \
	((tp_eq) == &DeeObject_DefaultEqWithNe ? DeeObject_TDefaultEqWithNe(tp_self, self, other) : \
	 default)
#define DeeType_invoke_cmp_tp_ne_DEFAULT(tp_self, tp_ne, self, other, default) \
	((tp_ne) == &DeeObject_DefaultNeWithEq ? DeeObject_TDefaultNeWithEq(tp_self, self, other) : \
	 default)
#define DeeType_invoke_cmp_tp_lo_DEFAULT(tp_self, tp_lo, self, other, default) \
	((tp_lo) == &DeeObject_DefaultLoWithGe ? DeeObject_TDefaultLoWithGe(tp_self, self, other) : \
	 default)
#define DeeType_invoke_cmp_tp_le_DEFAULT(tp_self, tp_le, self, other, default) \
	((tp_le) == &DeeObject_DefaultLeWithGr ? DeeObject_TDefaultLeWithGr(tp_self, self, other) : \
	 default)
#define DeeType_invoke_cmp_tp_gr_DEFAULT(tp_self, tp_gr, self, other, default) \
	((tp_gr) == &DeeObject_DefaultGrWithLe ? DeeObject_TDefaultGrWithLe(tp_self, self, other) : \
	 default)
#define DeeType_invoke_cmp_tp_ge_DEFAULT(tp_self, tp_ge, self, other, default) \
	((tp_ge) == &DeeObject_DefaultGeWithLo ? DeeObject_TDefaultGeWithLo(tp_self, self, other) : \
	 default)
#define DeeType_invoke_seq_tp_iter_DEFAULT(tp_self, tp_iter, self, default) \
	((tp_iter) == &DeeObject_DefaultIterWithForeach ? DeeObject_TDefaultIterWithForeach(tp_self, self) : \
	 (tp_iter) == &DeeObject_DefaultIterWithForeachPair ? DeeObject_TDefaultIterWithForeachPair(tp_self, self) : \
	 (tp_iter) == &DeeSeq_DefaultIterWithSizeAndGetItemIndex ? DeeSeq_TDefaultIterWithSizeAndGetItemIndex(tp_self, self) : \
	 (tp_iter) == &DeeSeq_DefaultIterWithSizeAndGetItemIndexFast ? DeeSeq_TDefaultIterWithSizeAndGetItemIndexFast(tp_self, self) : \
	 (tp_iter) == &DeeSeq_DefaultIterWithGetItemIndex ? DeeSeq_TDefaultIterWithGetItemIndex(tp_self, self) : \
	 (tp_iter) == &DeeSeq_DefaultIterWithSizeObDefaultAndGetItemDefault ? DeeSeq_TDefaultIterWithSizeObDefaultAndGetItemDefault(tp_self, self) : \
	 (tp_iter) == &DeeSeq_DefaultIterWithGetItemDefault ? DeeSeq_TDefaultIterWithGetItemDefault(tp_self, self) : \
	 default)
#define DeeType_invoke_seq_tp_foreach_DEFAULT(tp_self, tp_foreach, self, proc, arg, default) \
	((tp_foreach) == &DeeObject_DefaultForeachWithIter ? DeeObject_TDefaultForeachWithIter(tp_self, self, proc, arg) : \
	 (tp_foreach) == &DeeObject_DefaultForeachWithForeachPair ? DeeObject_TDefaultForeachWithForeachPair(tp_self, self, proc, arg) : \
	 (tp_foreach) == &DeeSeq_DefaultForeachWithSizeAndGetItemIndexFast ? DeeSeq_TDefaultForeachWithSizeAndGetItemIndexFast(tp_self, self, proc, arg) : \
	 (tp_foreach) == &DeeSeq_DefaultForeachWithSizeAndGetItemIndex ? DeeSeq_TDefaultForeachWithSizeAndGetItemIndex(tp_self, self, proc, arg) : \
	 (tp_foreach) == &DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault ? DeeSeq_TDefaultForeachWithSizeDefaultAndGetItemIndexDefault(tp_self, self, proc, arg) : \
	 (tp_foreach) == &DeeSeq_DefaultForeachWithGetItemIndexDefault ? DeeSeq_TDefaultForeachWithGetItemIndexDefault(tp_self, self, proc, arg) : \
	 default)
#define DeeType_invoke_seq_tp_foreach_pair_DEFAULT(tp_self, tp_foreach_pair, self, proc, arg, default) \
	((tp_foreach_pair) == &DeeObject_DefaultForeachPairWithForeach ? DeeObject_TDefaultForeachPairWithForeach(tp_self, self, proc, arg) : \
	 (tp_foreach_pair) == &DeeObject_DefaultForeachPairWithForeachDefault ? DeeObject_TDefaultForeachPairWithForeachDefault(tp_self, self, proc, arg) : \
	 (tp_foreach_pair) == &DeeObject_DefaultForeachPairWithIter ? DeeObject_TDefaultForeachPairWithIter(tp_self, self, proc, arg) : \
	 default)
#define DeeType_invoke_seq_tp_sizeob_DEFAULT(tp_self, tp_sizeob, self, default) \
	((tp_sizeob) == &DeeObject_DefaultSizeObWithSize ? DeeObject_TDefaultSizeObWithSize(tp_self, self) : \
	 (tp_sizeob) == &DeeObject_DefaultSizeObWithSizeDefault ? DeeObject_TDefaultSizeObWithSizeDefault(tp_self, self) : \
	 default)
#define DeeType_invoke_seq_tp_size_DEFAULT(tp_self, tp_size, self, default) \
	((tp_size) == &DeeObject_DefaultSizeWithSizeOb ? DeeObject_TDefaultSizeWithSizeOb(tp_self, self) : \
	 (tp_size) == &DeeSeq_DefaultSizeWithForeachPair ? DeeSeq_TDefaultSizeWithForeachPair(tp_self, self) : \
	 (tp_size) == &DeeSeq_DefaultSizeWithForeach ? DeeSeq_TDefaultSizeWithForeach(tp_self, self) : \
	 (tp_size) == &DeeSeq_DefaultSizeWithIter ? DeeSeq_TDefaultSizeWithIter(tp_self, self) : \
	 default)
#define DeeType_invoke_seq_tp_contains_DEFAULT(tp_self, tp_contains, self, elem, default) \
	((tp_contains) == &DeeSeq_DefaultContainsWithForeachPair ? DeeSeq_TDefaultContainsWithForeachPair(tp_self, self, elem) : \
	 (tp_contains) == &DeeSeq_DefaultContainsWithForeachDefault ? DeeSeq_TDefaultContainsWithForeachDefault(tp_self, self, elem) : \
	 default)
#define DeeType_invoke_seq_tp_getitem_DEFAULT(tp_self, tp_getitem, self, index, default) \
	((tp_getitem) == &DeeObject_DefaultGetItemWithGetItemIndex ? DeeObject_TDefaultGetItemWithGetItemIndex(tp_self, self, index) : \
	 (tp_getitem) == &DeeObject_DefaultGetItemWithGetItemIndexDefault ? DeeObject_TDefaultGetItemWithGetItemIndexDefault(tp_self, self, index) : \
	 default)
#define DeeType_invoke_seq_tp_getitem_index_DEFAULT(tp_self, tp_getitem_index, self, index, default) \
	((tp_getitem_index) == &DeeObject_DefaultGetItemIndexWithSizeAndGetItemIndexFast ? DeeObject_TDefaultGetItemIndexWithSizeAndGetItemIndexFast(tp_self, self, index) : \
	 (tp_getitem_index) == &DeeObject_DefaultGetItemIndexWithGetItem ? DeeObject_TDefaultGetItemIndexWithGetItem(tp_self, self, index) : \
	 (tp_getitem_index) == &DeeSeq_DefaultGetItemIndexWithForeachDefault ? DeeSeq_TDefaultGetItemIndexWithForeachDefault(tp_self, self, index) : \
	 default)
#define DeeType_invoke_seq_tp_delitem_DEFAULT(tp_self, tp_delitem, self, index, default) \
	((tp_delitem) == &DeeObject_DefaultDelItemWithDelItemIndex ? DeeObject_TDefaultDelItemWithDelItemIndex(tp_self, self, index) : \
	 (tp_delitem) == &DeeObject_DefaultDelItemWithDelItemIndexDefault ? DeeObject_TDefaultDelItemWithDelItemIndexDefault(tp_self, self, index) : \
	 default)
#define DeeType_invoke_seq_tp_delitem_index_DEFAULT(tp_self, tp_delitem_index, self, index, default) \
	((tp_delitem_index) == &DeeObject_DefaultDelItemIndexWithDelItem ? DeeObject_TDefaultDelItemIndexWithDelItem(tp_self, self, index) : \
	 (tp_delitem_index) == &DeeSeq_DefaultDelItemIndexWithDelRangeIndexDefault ? DeeSeq_TDefaultDelItemIndexWithDelRangeIndexDefault(tp_self, self, index) : \
	 default)
#define DeeType_invoke_seq_tp_setitem_DEFAULT(tp_self, tp_setitem, self, index, value, default) \
	((tp_setitem) == &DeeObject_DefaultSetItemWithSetItemIndex ? DeeObject_TDefaultSetItemWithSetItemIndex(tp_self, self, index, value) : \
	 (tp_setitem) == &DeeObject_DefaultSetItemWithSetItemIndexDefault ? DeeObject_TDefaultSetItemWithSetItemIndexDefault(tp_self, self, index, value) : \
	 default)
#define DeeType_invoke_seq_tp_setitem_index_DEFAULT(tp_self, tp_setitem_index, self, index, value, default) \
	((tp_setitem_index) == &DeeObject_DefaultSetItemIndexWithSetItem ? DeeObject_TDefaultSetItemIndexWithSetItem(tp_self, self, index, value) : \
	 (tp_setitem_index) == &DeeSeq_DefaultSetItemIndexWithSetRangeIndexDefault ? DeeSeq_TDefaultSetItemIndexWithSetRangeIndexDefault(tp_self, self, index, value) : \
	 default)
#define DeeType_invoke_seq_tp_bounditem_DEFAULT(tp_self, tp_bounditem, self, index, default) \
	((tp_bounditem) == &DeeObject_DefaultBoundItemWithBoundItemIndex ? DeeObject_TDefaultBoundItemWithBoundItemIndex(tp_self, self, index) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithGetItem ? DeeObject_TDefaultBoundItemWithGetItem(tp_self, self, index) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithGetItemIndex ? DeeObject_TDefaultBoundItemWithGetItemIndex(tp_self, self, index) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithBoundItemIndexDefault ? DeeObject_TDefaultBoundItemWithBoundItemIndexDefault(tp_self, self, index) : \
	 default)
#define DeeType_invoke_seq_tp_bounditem_index_DEFAULT(tp_self, tp_bounditem_index, self, index, default) \
	((tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithBoundItem ? DeeObject_TDefaultBoundItemIndexWithBoundItem(tp_self, self, index) : \
	 (tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithSizeAndGetItemIndexFast ? DeeObject_TDefaultBoundItemIndexWithSizeAndGetItemIndexFast(tp_self, self, index) : \
	 (tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithGetItemIndex ? DeeObject_TDefaultBoundItemIndexWithGetItemIndex(tp_self, self, index) : \
	 (tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithGetItem ? DeeObject_TDefaultBoundItemIndexWithGetItem(tp_self, self, index) : \
	 (tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithGetItemIndexDefault ? DeeObject_TDefaultBoundItemIndexWithGetItemIndexDefault(tp_self, self, index) : \
	 default)
#define DeeType_invoke_seq_tp_hasitem_DEFAULT(tp_self, tp_hasitem, self, index, default) \
	((tp_hasitem) == &DeeObject_DefaultHasItemWithHasItemIndex ? DeeObject_TDefaultHasItemWithHasItemIndex(tp_self, self, index) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithBoundItem ? DeeObject_TDefaultHasItemWithBoundItem(tp_self, self, index) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithBoundItemIndex ? DeeObject_TDefaultHasItemWithBoundItemIndex(tp_self, self, index) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithGetItem ? DeeObject_TDefaultHasItemWithGetItem(tp_self, self, index) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithGetItemIndex ? DeeObject_TDefaultHasItemWithGetItemIndex(tp_self, self, index) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithHasItemIndexDefault ? DeeObject_TDefaultHasItemWithHasItemIndexDefault(tp_self, self, index) : \
	 default)
#define DeeType_invoke_seq_tp_hasitem_index_DEFAULT(tp_self, tp_hasitem_index, self, index, default) \
	((tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithHasItem ? DeeObject_TDefaultHasItemIndexWithHasItem(tp_self, self, index) : \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithBoundItemIndex ? DeeObject_TDefaultHasItemIndexWithBoundItemIndex(tp_self, self, index) : \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithBoundItem ? DeeObject_TDefaultHasItemIndexWithBoundItem(tp_self, self, index) : \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithGetItemIndex ? DeeObject_TDefaultHasItemIndexWithGetItemIndex(tp_self, self, index) : \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithGetItem ? DeeObject_TDefaultHasItemIndexWithGetItem(tp_self, self, index) : \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithSize ? DeeObject_TDefaultHasItemIndexWithSize(tp_self, self, index) : \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithSizeDefault ? DeeObject_TDefaultHasItemIndexWithSizeDefault(tp_self, self, index) : \
	 default)
#define DeeType_invoke_seq_tp_getrange_DEFAULT(tp_self, tp_getrange, self, start, end, default) \
	((tp_getrange) == &DeeObject_DefaultGetRangeWithGetRangeIndexAndGetRangeIndexN ? DeeObject_TDefaultGetRangeWithGetRangeIndexAndGetRangeIndexN(tp_self, self, start, end) : \
	 (tp_getrange) == &DeeObject_DefaultGetRangeWithGetRangeIndexDefaultAndGetRangeIndexNDefault ? DeeObject_TDefaultGetRangeWithGetRangeIndexDefaultAndGetRangeIndexNDefault(tp_self, self, start, end) : \
	 (tp_getrange) == &DeeObject_DefaultGetRangeWithSizeDefaultAndGetItemIndex ? DeeObject_TDefaultGetRangeWithSizeDefaultAndGetItemIndex(tp_self, self, start, end) : \
	 (tp_getrange) == &DeeObject_DefaultGetRangeWithSizeObAndGetItem ? DeeObject_TDefaultGetRangeWithSizeObAndGetItem(tp_self, self, start, end) : \
	 default)
#define DeeType_invoke_seq_tp_getrange_index_DEFAULT(tp_self, tp_getrange_index, self, start, end, default) \
	((tp_getrange_index) == &DeeObject_DefaultGetRangeIndexWithGetRange ? DeeObject_TDefaultGetRangeIndexWithGetRange(tp_self, self, start, end) : \
	 (tp_getrange_index) == &DeeSeq_DefaultGetRangeIndexWithSizeAndGetItemIndexFast ? DeeSeq_TDefaultGetRangeIndexWithSizeAndGetItemIndexFast(tp_self, self, start, end) : \
	 (tp_getrange_index) == &DeeSeq_DefaultGetRangeIndexWithSizeDefaultAndGetItemIndex ? DeeSeq_TDefaultGetRangeIndexWithSizeDefaultAndGetItemIndex(tp_self, self, start, end) : \
	 (tp_getrange_index) == &DeeSeq_DefaultGetRangeIndexWithSizeDefaultAndGetItem ? DeeSeq_TDefaultGetRangeIndexWithSizeDefaultAndGetItem(tp_self, self, start, end) : \
	 (tp_getrange_index) == &DeeSeq_DefaultGetRangeIndexWithSizeDefaultAndIter ? DeeSeq_TDefaultGetRangeIndexWithSizeDefaultAndIter(tp_self, self, start, end) : \
	 (tp_getrange_index) == &DeeSeq_DefaultGetRangeIndexWithSizeDefaultAndIterDefault ? DeeSeq_TDefaultGetRangeIndexWithSizeDefaultAndIterDefault(tp_self, self, start, end) : \
	 default)
#define DeeType_invoke_seq_tp_getrange_index_n_DEFAULT(tp_self, tp_getrange_index_n, self, start, default) \
	((tp_getrange_index_n) == &DeeObject_DefaultGetRangeIndexNWithGetRange ? DeeObject_TDefaultGetRangeIndexNWithGetRange(tp_self, self, start) : \
	 (tp_getrange_index_n) == &DeeSeq_DefaultGetRangeIndexNWithSizeAndGetRangeIndex ? DeeSeq_TDefaultGetRangeIndexNWithSizeAndGetRangeIndex(tp_self, self, start) : \
	 (tp_getrange_index_n) == &DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndGetRangeIndex ? DeeSeq_TDefaultGetRangeIndexNWithSizeDefaultAndGetRangeIndex(tp_self, self, start) : \
	 (tp_getrange_index_n) == &DeeSeq_DefaultGetRangeIndexNWithSizeAndGetItemIndexFast ? DeeSeq_TDefaultGetRangeIndexNWithSizeAndGetItemIndexFast(tp_self, self, start) : \
	 (tp_getrange_index_n) == &DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndGetItemIndex ? DeeSeq_TDefaultGetRangeIndexNWithSizeDefaultAndGetItemIndex(tp_self, self, start) : \
	 (tp_getrange_index_n) == &DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndGetItem ? DeeSeq_TDefaultGetRangeIndexNWithSizeDefaultAndGetItem(tp_self, self, start) : \
	 (tp_getrange_index_n) == &DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndIter ? DeeSeq_TDefaultGetRangeIndexNWithSizeDefaultAndIter(tp_self, self, start) : \
	 (tp_getrange_index_n) == &DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndIterDefault ? DeeSeq_TDefaultGetRangeIndexNWithSizeDefaultAndIterDefault(tp_self, self, start) : \
	 default)
#define DeeType_invoke_seq_tp_delrange_DEFAULT(tp_self, tp_delrange, self, start, end, default) \
	((tp_delrange) == &DeeObject_DefaultDelRangeWithDelRangeIndexAndDelRangeIndexN ? DeeObject_TDefaultDelRangeWithDelRangeIndexAndDelRangeIndexN(tp_self, self, start, end) : \
	 (tp_delrange) == &DeeObject_DefaultDelRangeWithDelRangeIndexDefaultAndDelRangeIndexNDefault ? DeeObject_TDefaultDelRangeWithDelRangeIndexDefaultAndDelRangeIndexNDefault(tp_self, self, start, end) : \
	 (tp_delrange) == &DeeSeq_DefaultDelRangeWithSetRangeNone ? DeeSeq_TDefaultDelRangeWithSetRangeNone(tp_self, self, start, end) : \
	 (tp_delrange) == &DeeSeq_DefaultDelRangeWithSetRangeNoneDefault ? DeeSeq_TDefaultDelRangeWithSetRangeNoneDefault(tp_self, self, start, end) : \
	 default)
#define DeeType_invoke_seq_tp_delrange_index_DEFAULT(tp_self, tp_delrange_index, self, start, end, default) \
	((tp_delrange_index) == &DeeObject_DefaultDelRangeIndexWithDelRange ? DeeObject_TDefaultDelRangeIndexWithDelRange(tp_self, self, start, end) : \
	 (tp_delrange_index) == &DeeSeq_DefaultDelRangeIndexWithSetRangeIndexNone ? DeeSeq_TDefaultDelRangeIndexWithSetRangeIndexNone(tp_self, self, start, end) : \
	 (tp_delrange_index) == &DeeSeq_DefaultDelRangeIndexWithSetRangeIndexNoneDefault ? DeeSeq_TDefaultDelRangeIndexWithSetRangeIndexNoneDefault(tp_self, self, start, end) : \
	 default)
#define DeeType_invoke_seq_tp_delrange_index_n_DEFAULT(tp_self, tp_delrange_index_n, self, start, default) \
	((tp_delrange_index_n) == &DeeObject_DefaultDelRangeIndexNWithDelRange ? DeeObject_TDefaultDelRangeIndexNWithDelRange(tp_self, self, start) : \
	 (tp_delrange_index_n) == &DeeSeq_DefaultDelRangeIndexNWithSizeAndDelRangeIndex ? DeeSeq_TDefaultDelRangeIndexNWithSizeAndDelRangeIndex(tp_self, self, start) : \
	 (tp_delrange_index_n) == &DeeSeq_DefaultDelRangeIndexNWithSizeDefaultAndDelRangeIndex ? DeeSeq_TDefaultDelRangeIndexNWithSizeDefaultAndDelRangeIndex(tp_self, self, start) : \
	 (tp_delrange_index_n) == &DeeSeq_DefaultDelRangeIndexNWithSetRangeIndexNNone ? DeeSeq_TDefaultDelRangeIndexNWithSetRangeIndexNNone(tp_self, self, start) : \
	 (tp_delrange_index_n) == &DeeSeq_DefaultDelRangeIndexNWithSetRangeIndexNNoneDefault ? DeeSeq_TDefaultDelRangeIndexNWithSetRangeIndexNNoneDefault(tp_self, self, start) : \
	 default)
#define DeeType_invoke_seq_tp_setrange_DEFAULT(tp_self, tp_setrange, self, start, end, value, default) \
	((tp_setrange) == &DeeObject_DefaultSetRangeWithSetRangeIndexAndSetRangeIndexN ? DeeObject_TDefaultSetRangeWithSetRangeIndexAndSetRangeIndexN(tp_self, self, start, end, value) : \
	 (tp_setrange) == &DeeObject_DefaultSetRangeWithSetRangeIndexDefaultAndSetRangeIndexNDefault ? DeeObject_TDefaultSetRangeWithSetRangeIndexDefaultAndSetRangeIndexNDefault(tp_self, self, start, end, value) : \
	 default)
#define DeeType_invoke_seq_tp_setrange_index_DEFAULT(tp_self, tp_setrange_index, self, start, end, value, default) \
	((tp_setrange_index) == &DeeObject_DefaultSetRangeIndexWithSetRange ? DeeObject_TDefaultSetRangeIndexWithSetRange(tp_self, self, start, end, value) : \
	 (tp_setrange_index) == &DeeSeq_DefaultSetRangeIndexWithSizeAndDelItemIndexAndSetItemIndex ? DeeSeq_TDefaultSetRangeIndexWithSizeAndDelItemIndexAndSetItemIndex(tp_self, self, start, end, value) : \
	 (tp_setrange_index) == &DeeSeq_DefaultSetRangeIndexWithSizeDefaultAndDelItemIndexDefaultAndSetItemIndexDefault ? DeeSeq_TDefaultSetRangeIndexWithSizeDefaultAndDelItemIndexDefaultAndSetItemIndexDefault(tp_self, self, start, end, value) : \
	 (tp_setrange_index) == &DeeSeq_DefaultSetRangeIndexWithSizeAndSetItemIndex ? DeeSeq_TDefaultSetRangeIndexWithSizeAndSetItemIndex(tp_self, self, start, end, value) : \
	 (tp_setrange_index) == &DeeSeq_DefaultSetRangeIndexWithSizeDefaultAndSetItemIndexDefault ? DeeSeq_TDefaultSetRangeIndexWithSizeDefaultAndSetItemIndexDefault(tp_self, self, start, end, value) : \
	 default)
#define DeeType_invoke_seq_tp_setrange_index_n_DEFAULT(tp_self, tp_setrange_index_n, self, start, value, default) \
	((tp_setrange_index_n) == &DeeObject_DefaultSetRangeIndexNWithSetRange ? DeeObject_TDefaultSetRangeIndexNWithSetRange(tp_self, self, start, value) : \
	 (tp_setrange_index_n) == &DeeSeq_DefaultSetRangeIndexNWithSizeAndSetRangeIndex ? DeeSeq_TDefaultSetRangeIndexNWithSizeAndSetRangeIndex(tp_self, self, start, value) : \
	 (tp_setrange_index_n) == &DeeSeq_DefaultSetRangeIndexNWithSizeDefaultAndSetRangeIndex ? DeeSeq_TDefaultSetRangeIndexNWithSizeDefaultAndSetRangeIndex(tp_self, self, start, value) : \
	 (tp_setrange_index_n) == &DeeSeq_DefaultSetRangeIndexNWithSizeAndDelItemIndexAndSetItemIndex ? DeeSeq_TDefaultSetRangeIndexNWithSizeAndDelItemIndexAndSetItemIndex(tp_self, self, start, value) : \
	 (tp_setrange_index_n) == &DeeSeq_DefaultSetRangeIndexNWithSizeDefaultAndDelItemIndexDefaultAndSetItemIndexDefault ? DeeSeq_TDefaultSetRangeIndexNWithSizeDefaultAndDelItemIndexDefaultAndSetItemIndexDefault(tp_self, self, start, value) : \
	 (tp_setrange_index_n) == &DeeSeq_DefaultSetRangeIndexNWithSizeAndSetItemIndex ? DeeSeq_TDefaultSetRangeIndexNWithSizeAndSetItemIndex(tp_self, self, start, value) : \
	 (tp_setrange_index_n) == &DeeSeq_DefaultSetRangeIndexNWithSizeDefaultAndSetItemIndexDefault ? DeeSeq_TDefaultSetRangeIndexNWithSizeDefaultAndSetItemIndexDefault(tp_self, self, start, value) : \
	 default)
#define DeeType_invoke_cast_tp_str(tp_self, tp_str, self) \
	 DeeType_invoke_cast_tp_str_DEFAULT(tp_self, tp_str, self, DeeType_invoke_cast_tp_str_NODEFAULT(tp_self, tp_str, self))
#define DeeType_invoke_cast_tp_repr(tp_self, tp_repr, self) \
	 DeeType_invoke_cast_tp_repr_DEFAULT(tp_self, tp_repr, self, DeeType_invoke_cast_tp_repr_NODEFAULT(tp_self, tp_repr, self))
#define DeeType_invoke_cast_tp_print(tp_self, tp_print, self, printer, arg) \
	 DeeType_invoke_cast_tp_print_DEFAULT(tp_self, tp_print, self, printer, arg, DeeType_invoke_cast_tp_print_NODEFAULT(tp_self, tp_print, self, printer, arg))
#define DeeType_invoke_cast_tp_printrepr(tp_self, tp_printrepr, self, printer, arg) \
	 DeeType_invoke_cast_tp_printrepr_DEFAULT(tp_self, tp_printrepr, self, printer, arg, DeeType_invoke_cast_tp_printrepr_NODEFAULT(tp_self, tp_printrepr, self, printer, arg))
#define DeeType_invoke_tp_call(tp_self, tp_call, self, argc, argv) \
	 DeeType_invoke_tp_call_DEFAULT(tp_self, tp_call, self, argc, argv, DeeType_invoke_tp_call_NODEFAULT(tp_self, tp_call, self, argc, argv))
#define DeeType_invoke_tp_call_kw(tp_self, tp_call_kw, self, argc, argv, kw) \
	 DeeType_invoke_tp_call_kw_DEFAULT(tp_self, tp_call_kw, self, argc, argv, kw, DeeType_invoke_tp_call_kw_NODEFAULT(tp_self, tp_call_kw, self, argc, argv, kw))
#define DeeType_invoke_math_tp_int32(tp_self, tp_int32, self, result) \
	 DeeType_invoke_math_tp_int32_DEFAULT(tp_self, tp_int32, self, result, DeeType_invoke_math_tp_int32_NODEFAULT(tp_self, tp_int32, self, result))
#define DeeType_invoke_math_tp_int64(tp_self, tp_int64, self, result) \
	 DeeType_invoke_math_tp_int64_DEFAULT(tp_self, tp_int64, self, result, DeeType_invoke_math_tp_int64_NODEFAULT(tp_self, tp_int64, self, result))
#define DeeType_invoke_math_tp_double(tp_self, tp_double, self, result) \
	 DeeType_invoke_math_tp_double_DEFAULT(tp_self, tp_double, self, result, DeeType_invoke_math_tp_double_NODEFAULT(tp_self, tp_double, self, result))
#define DeeType_invoke_math_tp_int(tp_self, tp_int, self) \
	 DeeType_invoke_math_tp_int_DEFAULT(tp_self, tp_int, self, DeeType_invoke_math_tp_int_NODEFAULT(tp_self, tp_int, self))
#define DeeType_invoke_math_tp_add(tp_self, tp_add, self, other) \
	 DeeType_invoke_math_tp_add_DEFAULT(tp_self, tp_add, self, other, DeeType_invoke_math_tp_add_NODEFAULT(tp_self, tp_add, self, other))
#define DeeType_invoke_math_tp_sub(tp_self, tp_sub, self, other) \
	 DeeType_invoke_math_tp_sub_DEFAULT(tp_self, tp_sub, self, other, DeeType_invoke_math_tp_sub_NODEFAULT(tp_self, tp_sub, self, other))
#define DeeType_invoke_math_tp_mul(tp_self, tp_mul, self, other) \
	 DeeType_invoke_math_tp_mul_DEFAULT(tp_self, tp_mul, self, other, DeeType_invoke_math_tp_mul_NODEFAULT(tp_self, tp_mul, self, other))
#define DeeType_invoke_math_tp_div(tp_self, tp_div, self, other) \
	 DeeType_invoke_math_tp_div_DEFAULT(tp_self, tp_div, self, other, DeeType_invoke_math_tp_div_NODEFAULT(tp_self, tp_div, self, other))
#define DeeType_invoke_math_tp_mod(tp_self, tp_mod, self, other) \
	 DeeType_invoke_math_tp_mod_DEFAULT(tp_self, tp_mod, self, other, DeeType_invoke_math_tp_mod_NODEFAULT(tp_self, tp_mod, self, other))
#define DeeType_invoke_math_tp_shl(tp_self, tp_shl, self, other) \
	 DeeType_invoke_math_tp_shl_DEFAULT(tp_self, tp_shl, self, other, DeeType_invoke_math_tp_shl_NODEFAULT(tp_self, tp_shl, self, other))
#define DeeType_invoke_math_tp_shr(tp_self, tp_shr, self, other) \
	 DeeType_invoke_math_tp_shr_DEFAULT(tp_self, tp_shr, self, other, DeeType_invoke_math_tp_shr_NODEFAULT(tp_self, tp_shr, self, other))
#define DeeType_invoke_math_tp_and(tp_self, tp_and, self, other) \
	 DeeType_invoke_math_tp_and_DEFAULT(tp_self, tp_and, self, other, DeeType_invoke_math_tp_and_NODEFAULT(tp_self, tp_and, self, other))
#define DeeType_invoke_math_tp_or(tp_self, tp_or, self, other) \
	 DeeType_invoke_math_tp_or_DEFAULT(tp_self, tp_or, self, other, DeeType_invoke_math_tp_or_NODEFAULT(tp_self, tp_or, self, other))
#define DeeType_invoke_math_tp_xor(tp_self, tp_xor, self, other) \
	 DeeType_invoke_math_tp_xor_DEFAULT(tp_self, tp_xor, self, other, DeeType_invoke_math_tp_xor_NODEFAULT(tp_self, tp_xor, self, other))
#define DeeType_invoke_math_tp_pow(tp_self, tp_pow, self, other) \
	 DeeType_invoke_math_tp_pow_DEFAULT(tp_self, tp_pow, self, other, DeeType_invoke_math_tp_pow_NODEFAULT(tp_self, tp_pow, self, other))
#define DeeType_invoke_math_tp_inplace_add(tp_self, tp_inplace_add, p_self, other) \
	 DeeType_invoke_math_tp_inplace_add_DEFAULT(tp_self, tp_inplace_add, p_self, other, DeeType_invoke_math_tp_inplace_add_NODEFAULT(tp_self, tp_inplace_add, p_self, other))
#define DeeType_invoke_math_tp_inplace_sub(tp_self, tp_inplace_sub, p_self, other) \
	 DeeType_invoke_math_tp_inplace_sub_DEFAULT(tp_self, tp_inplace_sub, p_self, other, DeeType_invoke_math_tp_inplace_sub_NODEFAULT(tp_self, tp_inplace_sub, p_self, other))
#define DeeType_invoke_math_tp_inplace_mul(tp_self, tp_inplace_mul, p_self, other) \
	 DeeType_invoke_math_tp_inplace_mul_DEFAULT(tp_self, tp_inplace_mul, p_self, other, DeeType_invoke_math_tp_inplace_mul_NODEFAULT(tp_self, tp_inplace_mul, p_self, other))
#define DeeType_invoke_math_tp_inplace_div(tp_self, tp_inplace_div, p_self, other) \
	 DeeType_invoke_math_tp_inplace_div_DEFAULT(tp_self, tp_inplace_div, p_self, other, DeeType_invoke_math_tp_inplace_div_NODEFAULT(tp_self, tp_inplace_div, p_self, other))
#define DeeType_invoke_math_tp_inplace_mod(tp_self, tp_inplace_mod, p_self, other) \
	 DeeType_invoke_math_tp_inplace_mod_DEFAULT(tp_self, tp_inplace_mod, p_self, other, DeeType_invoke_math_tp_inplace_mod_NODEFAULT(tp_self, tp_inplace_mod, p_self, other))
#define DeeType_invoke_math_tp_inplace_shl(tp_self, tp_inplace_shl, p_self, other) \
	 DeeType_invoke_math_tp_inplace_shl_DEFAULT(tp_self, tp_inplace_shl, p_self, other, DeeType_invoke_math_tp_inplace_shl_NODEFAULT(tp_self, tp_inplace_shl, p_self, other))
#define DeeType_invoke_math_tp_inplace_shr(tp_self, tp_inplace_shr, p_self, other) \
	 DeeType_invoke_math_tp_inplace_shr_DEFAULT(tp_self, tp_inplace_shr, p_self, other, DeeType_invoke_math_tp_inplace_shr_NODEFAULT(tp_self, tp_inplace_shr, p_self, other))
#define DeeType_invoke_math_tp_inplace_and(tp_self, tp_inplace_and, p_self, other) \
	 DeeType_invoke_math_tp_inplace_and_DEFAULT(tp_self, tp_inplace_and, p_self, other, DeeType_invoke_math_tp_inplace_and_NODEFAULT(tp_self, tp_inplace_and, p_self, other))
#define DeeType_invoke_math_tp_inplace_or(tp_self, tp_inplace_or, p_self, other) \
	 DeeType_invoke_math_tp_inplace_or_DEFAULT(tp_self, tp_inplace_or, p_self, other, DeeType_invoke_math_tp_inplace_or_NODEFAULT(tp_self, tp_inplace_or, p_self, other))
#define DeeType_invoke_math_tp_inplace_xor(tp_self, tp_inplace_xor, p_self, other) \
	 DeeType_invoke_math_tp_inplace_xor_DEFAULT(tp_self, tp_inplace_xor, p_self, other, DeeType_invoke_math_tp_inplace_xor_NODEFAULT(tp_self, tp_inplace_xor, p_self, other))
#define DeeType_invoke_math_tp_inplace_pow(tp_self, tp_inplace_pow, p_self, other) \
	 DeeType_invoke_math_tp_inplace_pow_DEFAULT(tp_self, tp_inplace_pow, p_self, other, DeeType_invoke_math_tp_inplace_pow_NODEFAULT(tp_self, tp_inplace_pow, p_self, other))
#define DeeType_invoke_math_tp_inc(tp_self, tp_inc, p_self) \
	 DeeType_invoke_math_tp_inc_DEFAULT(tp_self, tp_inc, p_self, DeeType_invoke_math_tp_inc_NODEFAULT(tp_self, tp_inc, p_self))
#define DeeType_invoke_math_tp_dec(tp_self, tp_dec, p_self) \
	 DeeType_invoke_math_tp_dec_DEFAULT(tp_self, tp_dec, p_self, DeeType_invoke_math_tp_dec_NODEFAULT(tp_self, tp_dec, p_self))
#define DeeType_invoke_cmp_tp_eq(tp_self, tp_eq, self, other) \
	 DeeType_invoke_cmp_tp_eq_DEFAULT(tp_self, tp_eq, self, other, DeeType_invoke_cmp_tp_eq_NODEFAULT(tp_self, tp_eq, self, other))
#define DeeType_invoke_cmp_tp_ne(tp_self, tp_ne, self, other) \
	 DeeType_invoke_cmp_tp_ne_DEFAULT(tp_self, tp_ne, self, other, DeeType_invoke_cmp_tp_ne_NODEFAULT(tp_self, tp_ne, self, other))
#define DeeType_invoke_cmp_tp_lo(tp_self, tp_lo, self, other) \
	 DeeType_invoke_cmp_tp_lo_DEFAULT(tp_self, tp_lo, self, other, DeeType_invoke_cmp_tp_lo_NODEFAULT(tp_self, tp_lo, self, other))
#define DeeType_invoke_cmp_tp_le(tp_self, tp_le, self, other) \
	 DeeType_invoke_cmp_tp_le_DEFAULT(tp_self, tp_le, self, other, DeeType_invoke_cmp_tp_le_NODEFAULT(tp_self, tp_le, self, other))
#define DeeType_invoke_cmp_tp_gr(tp_self, tp_gr, self, other) \
	 DeeType_invoke_cmp_tp_gr_DEFAULT(tp_self, tp_gr, self, other, DeeType_invoke_cmp_tp_gr_NODEFAULT(tp_self, tp_gr, self, other))
#define DeeType_invoke_cmp_tp_ge(tp_self, tp_ge, self, other) \
	 DeeType_invoke_cmp_tp_ge_DEFAULT(tp_self, tp_ge, self, other, DeeType_invoke_cmp_tp_ge_NODEFAULT(tp_self, tp_ge, self, other))
#define DeeType_invoke_seq_tp_iter(tp_self, tp_iter, self) \
	 DeeType_invoke_seq_tp_iter_DEFAULT(tp_self, tp_iter, self, DeeType_invoke_seq_tp_iter_NODEFAULT(tp_self, tp_iter, self))
#define DeeType_invoke_seq_tp_foreach(tp_self, tp_foreach, self, proc, arg) \
	 DeeType_invoke_seq_tp_foreach_DEFAULT(tp_self, tp_foreach, self, proc, arg, DeeType_invoke_seq_tp_foreach_NODEFAULT(tp_self, tp_foreach, self, proc, arg))
#define DeeType_invoke_seq_tp_foreach_pair(tp_self, tp_foreach_pair, self, proc, arg) \
	 DeeType_invoke_seq_tp_foreach_pair_DEFAULT(tp_self, tp_foreach_pair, self, proc, arg, DeeType_invoke_seq_tp_foreach_pair_NODEFAULT(tp_self, tp_foreach_pair, self, proc, arg))
#define DeeType_invoke_seq_tp_sizeob(tp_self, tp_sizeob, self) \
	 DeeType_invoke_seq_tp_sizeob_DEFAULT(tp_self, tp_sizeob, self, DeeType_invoke_seq_tp_sizeob_NODEFAULT(tp_self, tp_sizeob, self))
#define DeeType_invoke_seq_tp_size(tp_self, tp_size, self) \
	 DeeType_invoke_seq_tp_size_DEFAULT(tp_self, tp_size, self, DeeType_invoke_seq_tp_size_NODEFAULT(tp_self, tp_size, self))
#define DeeType_invoke_seq_tp_contains(tp_self, tp_contains, self, elem) \
	 DeeType_invoke_seq_tp_contains_DEFAULT(tp_self, tp_contains, self, elem, DeeType_invoke_seq_tp_contains_NODEFAULT(tp_self, tp_contains, self, elem))
#define DeeType_invoke_seq_tp_getitem(tp_self, tp_getitem, self, index) \
	 DeeType_invoke_seq_tp_getitem_DEFAULT(tp_self, tp_getitem, self, index, DeeType_invoke_seq_tp_getitem_NODEFAULT(tp_self, tp_getitem, self, index))
#define DeeType_invoke_seq_tp_getitem_index(tp_self, tp_getitem_index, self, index) \
	 DeeType_invoke_seq_tp_getitem_index_DEFAULT(tp_self, tp_getitem_index, self, index, DeeType_invoke_seq_tp_getitem_index_NODEFAULT(tp_self, tp_getitem_index, self, index))
#define DeeType_invoke_seq_tp_delitem(tp_self, tp_delitem, self, index) \
	 DeeType_invoke_seq_tp_delitem_DEFAULT(tp_self, tp_delitem, self, index, DeeType_invoke_seq_tp_delitem_NODEFAULT(tp_self, tp_delitem, self, index))
#define DeeType_invoke_seq_tp_delitem_index(tp_self, tp_delitem_index, self, index) \
	 DeeType_invoke_seq_tp_delitem_index_DEFAULT(tp_self, tp_delitem_index, self, index, DeeType_invoke_seq_tp_delitem_index_NODEFAULT(tp_self, tp_delitem_index, self, index))
#define DeeType_invoke_seq_tp_setitem(tp_self, tp_setitem, self, index, value) \
	 DeeType_invoke_seq_tp_setitem_DEFAULT(tp_self, tp_setitem, self, index, value, DeeType_invoke_seq_tp_setitem_NODEFAULT(tp_self, tp_setitem, self, index, value))
#define DeeType_invoke_seq_tp_setitem_index(tp_self, tp_setitem_index, self, index, value) \
	 DeeType_invoke_seq_tp_setitem_index_DEFAULT(tp_self, tp_setitem_index, self, index, value, DeeType_invoke_seq_tp_setitem_index_NODEFAULT(tp_self, tp_setitem_index, self, index, value))
#define DeeType_invoke_seq_tp_bounditem(tp_self, tp_bounditem, self, index) \
	 DeeType_invoke_seq_tp_bounditem_DEFAULT(tp_self, tp_bounditem, self, index, DeeType_invoke_seq_tp_bounditem_NODEFAULT(tp_self, tp_bounditem, self, index))
#define DeeType_invoke_seq_tp_bounditem_index(tp_self, tp_bounditem_index, self, index) \
	 DeeType_invoke_seq_tp_bounditem_index_DEFAULT(tp_self, tp_bounditem_index, self, index, DeeType_invoke_seq_tp_bounditem_index_NODEFAULT(tp_self, tp_bounditem_index, self, index))
#define DeeType_invoke_seq_tp_hasitem(tp_self, tp_hasitem, self, index) \
	 DeeType_invoke_seq_tp_hasitem_DEFAULT(tp_self, tp_hasitem, self, index, DeeType_invoke_seq_tp_hasitem_NODEFAULT(tp_self, tp_hasitem, self, index))
#define DeeType_invoke_seq_tp_hasitem_index(tp_self, tp_hasitem_index, self, index) \
	 DeeType_invoke_seq_tp_hasitem_index_DEFAULT(tp_self, tp_hasitem_index, self, index, DeeType_invoke_seq_tp_hasitem_index_NODEFAULT(tp_self, tp_hasitem_index, self, index))
#define DeeType_invoke_seq_tp_getrange(tp_self, tp_getrange, self, start, end) \
	 DeeType_invoke_seq_tp_getrange_DEFAULT(tp_self, tp_getrange, self, start, end, DeeType_invoke_seq_tp_getrange_NODEFAULT(tp_self, tp_getrange, self, start, end))
#define DeeType_invoke_seq_tp_getrange_index(tp_self, tp_getrange_index, self, start, end) \
	 DeeType_invoke_seq_tp_getrange_index_DEFAULT(tp_self, tp_getrange_index, self, start, end, DeeType_invoke_seq_tp_getrange_index_NODEFAULT(tp_self, tp_getrange_index, self, start, end))
#define DeeType_invoke_seq_tp_getrange_index_n(tp_self, tp_getrange_index_n, self, start) \
	 DeeType_invoke_seq_tp_getrange_index_n_DEFAULT(tp_self, tp_getrange_index_n, self, start, DeeType_invoke_seq_tp_getrange_index_n_NODEFAULT(tp_self, tp_getrange_index_n, self, start))
#define DeeType_invoke_seq_tp_delrange(tp_self, tp_delrange, self, start, end) \
	 DeeType_invoke_seq_tp_delrange_DEFAULT(tp_self, tp_delrange, self, start, end, DeeType_invoke_seq_tp_delrange_NODEFAULT(tp_self, tp_delrange, self, start, end))
#define DeeType_invoke_seq_tp_delrange_index(tp_self, tp_delrange_index, self, start, end) \
	 DeeType_invoke_seq_tp_delrange_index_DEFAULT(tp_self, tp_delrange_index, self, start, end, DeeType_invoke_seq_tp_delrange_index_NODEFAULT(tp_self, tp_delrange_index, self, start, end))
#define DeeType_invoke_seq_tp_delrange_index_n(tp_self, tp_delrange_index_n, self, start) \
	 DeeType_invoke_seq_tp_delrange_index_n_DEFAULT(tp_self, tp_delrange_index_n, self, start, DeeType_invoke_seq_tp_delrange_index_n_NODEFAULT(tp_self, tp_delrange_index_n, self, start))
#define DeeType_invoke_seq_tp_setrange(tp_self, tp_setrange, self, start, end, value) \
	 DeeType_invoke_seq_tp_setrange_DEFAULT(tp_self, tp_setrange, self, start, end, value, DeeType_invoke_seq_tp_setrange_NODEFAULT(tp_self, tp_setrange, self, start, end, value))
#define DeeType_invoke_seq_tp_setrange_index(tp_self, tp_setrange_index, self, start, end, value) \
	 DeeType_invoke_seq_tp_setrange_index_DEFAULT(tp_self, tp_setrange_index, self, start, end, value, DeeType_invoke_seq_tp_setrange_index_NODEFAULT(tp_self, tp_setrange_index, self, start, end, value))
#define DeeType_invoke_seq_tp_setrange_index_n(tp_self, tp_setrange_index_n, self, start, value) \
	 DeeType_invoke_seq_tp_setrange_index_n_DEFAULT(tp_self, tp_setrange_index_n, self, start, value, DeeType_invoke_seq_tp_setrange_index_n_NODEFAULT(tp_self, tp_setrange_index_n, self, start, value))
/*[[[end]]]*/
/* clang-format on */


#define DeeType_invoke_init_tp_assign_DEFAULT(tp_self, tp_assign, self, other, default) default
#define DeeType_invoke_init_tp_assign(tp_self, tp_assign, self, other) \
	DeeType_invoke_init_tp_assign_DEFAULT(tp_self, tp_assign, self, other, DeeType_invoke_init_tp_assign_NODEFAULT(tp_self, tp_assign, self, other))
#define DeeType_invoke_init_tp_move_assign_DEFAULT(tp_self, tp_move_assign, self, other, default) default
#define DeeType_invoke_init_tp_move_assign(tp_self, tp_move_assign, self, other) \
	DeeType_invoke_init_tp_move_assign_DEFAULT(tp_self, tp_move_assign, self, other, DeeType_invoke_init_tp_move_assign_NODEFAULT(tp_self, tp_move_assign, self, other))
#define DeeType_invoke_cast_tp_bool_DEFAULT(tp_self, tp_bool, self, default) default
#define DeeType_invoke_cast_tp_bool(tp_self, tp_bool, self) \
	DeeType_invoke_cast_tp_bool_DEFAULT(tp_self, tp_bool, self, DeeType_invoke_cast_tp_bool_NODEFAULT(tp_self, tp_bool, self))
#define DeeType_invoke_tp_iter_next_NEFAULT(tp_self, tp_iter_next, self, default) default
#define DeeType_invoke_tp_iter_next(tp_self, tp_iter_next, self) \
	DeeType_invoke_tp_iter_next_NEFAULT(tp_self, tp_iter_next, self, DeeType_invoke_tp_iter_next_NODEFAULT(tp_self, tp_iter_next, self))
#define DeeType_invoke_math_tp_inv_DEFAULT(tp_self, tp_inv, self, default) default
#define DeeType_invoke_math_tp_inv(tp_self, tp_inv, self) \
	DeeType_invoke_math_tp_inv_DEFAULT(tp_self, tp_inv, self, DeeType_invoke_math_tp_inv_NODEFAULT(tp_self, tp_inv, self))
#define DeeType_invoke_math_tp_pos_DEFAULT(tp_self, tp_pos, self, default) default
#define DeeType_invoke_math_tp_pos(tp_self, tp_pos, self) \
	DeeType_invoke_math_tp_pos_DEFAULT(tp_self, tp_pos, self, DeeType_invoke_math_tp_pos_NODEFAULT(tp_self, tp_pos, self))
#define DeeType_invoke_math_tp_neg_DEFAULT(tp_self, tp_neg, self, default) default
#define DeeType_invoke_math_tp_neg(tp_self, tp_neg, self) \
	DeeType_invoke_math_tp_neg_DEFAULT(tp_self, tp_neg, self, DeeType_invoke_math_tp_neg_NODEFAULT(tp_self, tp_neg, self))
#define DeeType_invoke_cmp_tp_hash_DEFAULT(tp_self, tp_hash, self, default) default
#define DeeType_invoke_cmp_tp_hash(tp_self, tp_hash, self) \
	DeeType_invoke_cmp_tp_hash_DEFAULT(tp_self, tp_hash, self, DeeType_invoke_cmp_tp_hash_NODEFAULT(tp_self, tp_hash, self))
#define DeeType_invoke_attr_tp_getattr_DEFAULT(tp_self, tp_getattr, self, name, default) default
#define DeeType_invoke_attr_tp_getattr(tp_self, tp_getattr, self, name) \
	DeeType_invoke_attr_tp_getattr_DEFAULT(tp_self, tp_getattr, self, name, DeeType_invoke_attr_tp_getattr_NODEFAULT(tp_self, tp_getattr, self, name))
#define DeeType_invoke_attr_tp_delattr_DEFAULT(tp_self, tp_delattr, self, name, default) default
#define DeeType_invoke_attr_tp_delattr(tp_self, tp_delattr, self, name) \
	DeeType_invoke_attr_tp_delattr_DEFAULT(tp_self, tp_delattr, self, name, DeeType_invoke_attr_tp_delattr_NODEFAULT(tp_self, tp_delattr, self, name))
#define DeeType_invoke_attr_tp_setattr_DEFAULT(tp_self, tp_setattr, self, name, value, default) default
#define DeeType_invoke_attr_tp_setattr(tp_self, tp_setattr, self, name, value) \
	DeeType_invoke_attr_tp_setattr_DEFAULT(tp_self, tp_setattr, self, name, value, DeeType_invoke_attr_tp_setattr_NODEFAULT(tp_self, tp_setattr, self, name, value))
#define DeeType_invoke_with_tp_enter_DEFAULT(tp_self, tp_enter, self, default) default
#define DeeType_invoke_with_tp_enter(tp_self, tp_enter, self) \
	DeeType_invoke_with_tp_enter_DEFAULT(tp_self, tp_enter, self, DeeType_invoke_with_tp_enter_NODEFAULT(tp_self, tp_enter, self))
#define DeeType_invoke_with_tp_leave_DEFAULT(tp_self, tp_leave, self, default) default
#define DeeType_invoke_with_tp_leave(tp_self, tp_leave, self) \
	DeeType_invoke_with_tp_leave_DEFAULT(tp_self, tp_leave, self, DeeType_invoke_with_tp_leave_NODEFAULT(tp_self, tp_leave, self))

#define DeeType_InvokeInitAssign(tp_self, self, other)                             DeeType_invoke_init_tp_assign(tp_self, (tp_self)->tp_init.tp_assign, self, other)
#define DeeType_InvokeInitAssign_NODEFAULT(tp_self, self, other)                   DeeType_invoke_init_tp_assign_NODEFAULT(tp_self, (tp_self)->tp_init.tp_assign, self, other)
#define DeeType_InvokeInitMoveAssign(tp_self, self, other)                         DeeType_invoke_init_tp_move_assign(tp_self, (tp_self)->tp_init.tp_move_assign, self, other)
#define DeeType_InvokeInitMoveAssign_NODEFAULT(tp_self, self, other)               DeeType_invoke_init_tp_move_assign_NODEFAULT(tp_self, (tp_self)->tp_init.tp_move_assign, self, other)
#define DeeType_InvokeCastStr(tp_self, self)                                       DeeType_invoke_cast_tp_str(tp_self, (tp_self)->tp_cast.tp_str, self)
#define DeeType_InvokeCastStr_NODEFAULT(tp_self, self)                             DeeType_invoke_cast_tp_str_NODEFAULT(tp_self, (tp_self)->tp_cast.tp_str, self)
#define DeeType_InvokeCastPrint(tp_self, self, printer, arg)                       DeeType_invoke_cast_tp_print(tp_self, (tp_self)->tp_cast.tp_print, self, printer, arg)
#define DeeType_InvokeCastPrint_NODEFAULT(tp_self, self, printer, arg)             DeeType_invoke_cast_tp_print_NODEFAULT(tp_self, (tp_self)->tp_cast.tp_print, self, printer, arg)
#define DeeType_InvokeCastRepr(tp_self, self)                                      DeeType_invoke_cast_tp_repr(tp_self, (tp_self)->tp_cast.tp_repr, self)
#define DeeType_InvokeCastRepr_NODEFAULT(tp_self, self)                            DeeType_invoke_cast_tp_repr_NODEFAULT(tp_self, (tp_self)->tp_cast.tp_repr, self)
#define DeeType_InvokeCastPrintRepr(tp_self, self, printer, arg)                   DeeType_invoke_cast_tp_printrepr(tp_self, (tp_self)->tp_cast.tp_printrepr, self, printer, arg)
#define DeeType_InvokeCastPrintRepr_NODEFAULT(tp_self, self, printer, arg)         DeeType_invoke_cast_tp_printrepr_NODEFAULT(tp_self, (tp_self)->tp_cast.tp_printrepr, self, printer, arg)
#define DeeType_InvokeCastBool(tp_self, self)                                      DeeType_invoke_cast_tp_bool(tp_self, (tp_self)->tp_cast.tp_bool, self)
#define DeeType_InvokeCastBool_NODEFAULT(tp_self, self)                            DeeType_invoke_cast_tp_bool_NODEFAULT(tp_self, (tp_self)->tp_cast.tp_bool, self)
#define DeeType_InvokeIterNext(tp_self, self)                                      DeeType_invoke_tp_iter_next(tp_self, (tp_self)->tp_iter_next, self)
#define DeeType_InvokeIterNext_NODEFAULT(tp_self, self)                            DeeType_invoke_tp_iter_next_NODEFAULT(tp_self, (tp_self)->tp_iter_next, self)
#define DeeType_InvokeCall(tp_self, self, argc, argv)                              DeeType_invoke_tp_call(tp_self, (tp_self)->tp_call, self, argc, argv)
#define DeeType_InvokeCall_NODEFAULT(tp_self, self, argc, argv)                    DeeType_invoke_tp_call_NODEFAULT(tp_self, (tp_self)->tp_call, self, argc, argv)
#define DeeType_InvokeCallKw(tp_self, self, argc, argv, kw)                        DeeType_invoke_tp_call_kw(tp_self, (tp_self)->tp_call_kw, self, argc, argv, kw)
#define DeeType_InvokeCallKw_NODEFAULT(tp_self, self, argc, argv, kw)              DeeType_invoke_tp_call_kw_NODEFAULT(tp_self, (tp_self)->tp_call_kw, self, argc, argv, kw)
#define DeeType_InvokeMathInt32(tp_self, self, result)                             DeeType_invoke_math_tp_int32(tp_self, (tp_self)->tp_math->tp_int32, self, result)
#define DeeType_InvokeMathInt32_NODEFAULT(tp_self, self, result)                   DeeType_invoke_math_tp_int32_NODEFAULT(tp_self, (tp_self)->tp_math->tp_int32, self, result)
#define DeeType_InvokeMathInt64(tp_self, self, result)                             DeeType_invoke_math_tp_int64(tp_self, (tp_self)->tp_math->tp_int64, self, result)
#define DeeType_InvokeMathInt64_NODEFAULT(tp_self, self, result)                   DeeType_invoke_math_tp_int64_NODEFAULT(tp_self, (tp_self)->tp_math->tp_int64, self, result)
#define DeeType_InvokeMathDouble(tp_self, self, result)                            DeeType_invoke_math_tp_double(tp_self, (tp_self)->tp_math->tp_double, self, result)
#define DeeType_InvokeMathDouble_NODEFAULT(tp_self, self, result)                  DeeType_invoke_math_tp_double_NODEFAULT(tp_self, (tp_self)->tp_math->tp_double, self, result)
#define DeeType_InvokeMathInt(tp_self, self)                                       DeeType_invoke_math_tp_int(tp_self, (tp_self)->tp_math->tp_int, self)
#define DeeType_InvokeMathInt_NODEFAULT(tp_self, self)                             DeeType_invoke_math_tp_int_NODEFAULT(tp_self, (tp_self)->tp_math->tp_int, self)
#define DeeType_InvokeMathInv(tp_self, self)                                       DeeType_invoke_math_tp_inv(tp_self, (tp_self)->tp_math->tp_inv, self)
#define DeeType_InvokeMathInv_NODEFAULT(tp_self, self)                             DeeType_invoke_math_tp_inv_NODEFAULT(tp_self, (tp_self)->tp_math->tp_inv, self)
#define DeeType_InvokeMathPos(tp_self, self)                                       DeeType_invoke_math_tp_pos(tp_self, (tp_self)->tp_math->tp_pos, self)
#define DeeType_InvokeMathPos_NODEFAULT(tp_self, self)                             DeeType_invoke_math_tp_pos_NODEFAULT(tp_self, (tp_self)->tp_math->tp_pos, self)
#define DeeType_InvokeMathNeg(tp_self, self)                                       DeeType_invoke_math_tp_neg(tp_self, (tp_self)->tp_math->tp_neg, self)
#define DeeType_InvokeMathNeg_NODEFAULT(tp_self, self)                             DeeType_invoke_math_tp_neg_NODEFAULT(tp_self, (tp_self)->tp_math->tp_neg, self)
#define DeeType_InvokeMathAdd(tp_self, self, other)                                DeeType_invoke_math_tp_add(tp_self, (tp_self)->tp_math->tp_add, self, other)
#define DeeType_InvokeMathAdd_NODEFAULT(tp_self, self, other)                      DeeType_invoke_math_tp_add_NODEFAULT(tp_self, (tp_self)->tp_math->tp_add, self, other)
#define DeeType_InvokeMathSub(tp_self, self, other)                                DeeType_invoke_math_tp_sub(tp_self, (tp_self)->tp_math->tp_sub, self, other)
#define DeeType_InvokeMathSub_NODEFAULT(tp_self, self, other)                      DeeType_invoke_math_tp_sub_NODEFAULT(tp_self, (tp_self)->tp_math->tp_sub, self, other)
#define DeeType_InvokeMathMul(tp_self, self, other)                                DeeType_invoke_math_tp_mul(tp_self, (tp_self)->tp_math->tp_mul, self, other)
#define DeeType_InvokeMathMul_NODEFAULT(tp_self, self, other)                      DeeType_invoke_math_tp_mul_NODEFAULT(tp_self, (tp_self)->tp_math->tp_mul, self, other)
#define DeeType_InvokeMathDiv(tp_self, self, other)                                DeeType_invoke_math_tp_div(tp_self, (tp_self)->tp_math->tp_div, self, other)
#define DeeType_InvokeMathDiv_NODEFAULT(tp_self, self, other)                      DeeType_invoke_math_tp_div_NODEFAULT(tp_self, (tp_self)->tp_math->tp_div, self, other)
#define DeeType_InvokeMathMod(tp_self, self, other)                                DeeType_invoke_math_tp_mod(tp_self, (tp_self)->tp_math->tp_mod, self, other)
#define DeeType_InvokeMathMod_NODEFAULT(tp_self, self, other)                      DeeType_invoke_math_tp_mod_NODEFAULT(tp_self, (tp_self)->tp_math->tp_mod, self, other)
#define DeeType_InvokeMathShl(tp_self, self, other)                                DeeType_invoke_math_tp_shl(tp_self, (tp_self)->tp_math->tp_shl, self, other)
#define DeeType_InvokeMathShl_NODEFAULT(tp_self, self, other)                      DeeType_invoke_math_tp_shl_NODEFAULT(tp_self, (tp_self)->tp_math->tp_shl, self, other)
#define DeeType_InvokeMathShr(tp_self, self, other)                                DeeType_invoke_math_tp_shr(tp_self, (tp_self)->tp_math->tp_shr, self, other)
#define DeeType_InvokeMathShr_NODEFAULT(tp_self, self, other)                      DeeType_invoke_math_tp_shr_NODEFAULT(tp_self, (tp_self)->tp_math->tp_shr, self, other)
#define DeeType_InvokeMathAnd(tp_self, self, other)                                DeeType_invoke_math_tp_and(tp_self, (tp_self)->tp_math->tp_and, self, other)
#define DeeType_InvokeMathAnd_NODEFAULT(tp_self, self, other)                      DeeType_invoke_math_tp_and_NODEFAULT(tp_self, (tp_self)->tp_math->tp_and, self, other)
#define DeeType_InvokeMathOr(tp_self, self, other)                                 DeeType_invoke_math_tp_or(tp_self, (tp_self)->tp_math->tp_or, self, other)
#define DeeType_InvokeMathOr_NODEFAULT(tp_self, self, other)                       DeeType_invoke_math_tp_or_NODEFAULT(tp_self, (tp_self)->tp_math->tp_or, self, other)
#define DeeType_InvokeMathXor(tp_self, self, other)                                DeeType_invoke_math_tp_xor(tp_self, (tp_self)->tp_math->tp_xor, self, other)
#define DeeType_InvokeMathXor_NODEFAULT(tp_self, self, other)                      DeeType_invoke_math_tp_xor_NODEFAULT(tp_self, (tp_self)->tp_math->tp_xor, self, other)
#define DeeType_InvokeMathPow(tp_self, self, other)                                DeeType_invoke_math_tp_pow(tp_self, (tp_self)->tp_math->tp_pow, self, other)
#define DeeType_InvokeMathPow_NODEFAULT(tp_self, self, other)                      DeeType_invoke_math_tp_pow_NODEFAULT(tp_self, (tp_self)->tp_math->tp_pow, self, other)
#define DeeType_InvokeMathInc(tp_self, p_self)                                     DeeType_invoke_math_tp_inc(tp_self, (tp_self)->tp_math->tp_inc, p_self)
#define DeeType_InvokeMathInc_NODEFAULT(tp_self, p_self)                           DeeType_invoke_math_tp_inc_NODEFAULT(tp_self, (tp_self)->tp_math->tp_inc, p_self)
#define DeeType_InvokeMathDec(tp_self, p_self)                                     DeeType_invoke_math_tp_dec(tp_self, (tp_self)->tp_math->tp_dec, p_self)
#define DeeType_InvokeMathDec_NODEFAULT(tp_self, p_self)                           DeeType_invoke_math_tp_dec_NODEFAULT(tp_self, (tp_self)->tp_math->tp_dec, p_self)
#define DeeType_InvokeMathInplaceAdd(tp_self, p_self, other)                       DeeType_invoke_math_tp_inplace_add(tp_self, (tp_self)->tp_math->tp_inplace_add, p_self, other)
#define DeeType_InvokeMathInplaceAdd_NODEFAULT(tp_self, p_self, other)             DeeType_invoke_math_tp_inplace_add_NODEFAULT(tp_self, (tp_self)->tp_math->tp_inplace_add, p_self, other)
#define DeeType_InvokeMathInplaceSub(tp_self, p_self, other)                       DeeType_invoke_math_tp_inplace_sub(tp_self, (tp_self)->tp_math->tp_inplace_sub, p_self, other)
#define DeeType_InvokeMathInplaceSub_NODEFAULT(tp_self, p_self, other)             DeeType_invoke_math_tp_inplace_sub_NODEFAULT(tp_self, (tp_self)->tp_math->tp_inplace_sub, p_self, other)
#define DeeType_InvokeMathInplaceMul(tp_self, p_self, other)                       DeeType_invoke_math_tp_inplace_mul(tp_self, (tp_self)->tp_math->tp_inplace_mul, p_self, other)
#define DeeType_InvokeMathInplaceMul_NODEFAULT(tp_self, p_self, other)             DeeType_invoke_math_tp_inplace_mul_NODEFAULT(tp_self, (tp_self)->tp_math->tp_inplace_mul, p_self, other)
#define DeeType_InvokeMathInplaceDiv(tp_self, p_self, other)                       DeeType_invoke_math_tp_inplace_div(tp_self, (tp_self)->tp_math->tp_inplace_div, p_self, other)
#define DeeType_InvokeMathInplaceDiv_NODEFAULT(tp_self, p_self, other)             DeeType_invoke_math_tp_inplace_div_NODEFAULT(tp_self, (tp_self)->tp_math->tp_inplace_div, p_self, other)
#define DeeType_InvokeMathInplaceMod(tp_self, p_self, other)                       DeeType_invoke_math_tp_inplace_mod(tp_self, (tp_self)->tp_math->tp_inplace_mod, p_self, other)
#define DeeType_InvokeMathInplaceMod_NODEFAULT(tp_self, p_self, other)             DeeType_invoke_math_tp_inplace_mod_NODEFAULT(tp_self, (tp_self)->tp_math->tp_inplace_mod, p_self, other)
#define DeeType_InvokeMathInplaceShl(tp_self, p_self, other)                       DeeType_invoke_math_tp_inplace_shl(tp_self, (tp_self)->tp_math->tp_inplace_shl, p_self, other)
#define DeeType_InvokeMathInplaceShl_NODEFAULT(tp_self, p_self, other)             DeeType_invoke_math_tp_inplace_shl_NODEFAULT(tp_self, (tp_self)->tp_math->tp_inplace_shl, p_self, other)
#define DeeType_InvokeMathInplaceShr(tp_self, p_self, other)                       DeeType_invoke_math_tp_inplace_shr(tp_self, (tp_self)->tp_math->tp_inplace_shr, p_self, other)
#define DeeType_InvokeMathInplaceShr_NODEFAULT(tp_self, p_self, other)             DeeType_invoke_math_tp_inplace_shr_NODEFAULT(tp_self, (tp_self)->tp_math->tp_inplace_shr, p_self, other)
#define DeeType_InvokeMathInplaceAnd(tp_self, p_self, other)                       DeeType_invoke_math_tp_inplace_and(tp_self, (tp_self)->tp_math->tp_inplace_and, p_self, other)
#define DeeType_InvokeMathInplaceAnd_NODEFAULT(tp_self, p_self, other)             DeeType_invoke_math_tp_inplace_and_NODEFAULT(tp_self, (tp_self)->tp_math->tp_inplace_and, p_self, other)
#define DeeType_InvokeMathInplaceOr(tp_self, p_self, other)                        DeeType_invoke_math_tp_inplace_or(tp_self, (tp_self)->tp_math->tp_inplace_or, p_self, other)
#define DeeType_InvokeMathInplaceOr_NODEFAULT(tp_self, p_self, other)              DeeType_invoke_math_tp_inplace_or_NODEFAULT(tp_self, (tp_self)->tp_math->tp_inplace_or, p_self, other)
#define DeeType_InvokeMathInplaceXor(tp_self, p_self, other)                       DeeType_invoke_math_tp_inplace_xor(tp_self, (tp_self)->tp_math->tp_inplace_xor, p_self, other)
#define DeeType_InvokeMathInplaceXor_NODEFAULT(tp_self, p_self, other)             DeeType_invoke_math_tp_inplace_xor_NODEFAULT(tp_self, (tp_self)->tp_math->tp_inplace_xor, p_self, other)
#define DeeType_InvokeMathInplacePow(tp_self, p_self, other)                       DeeType_invoke_math_tp_inplace_pow(tp_self, (tp_self)->tp_math->tp_inplace_pow, p_self, other)
#define DeeType_InvokeMathInplacePow_NODEFAULT(tp_self, p_self, other)             DeeType_invoke_math_tp_inplace_pow_NODEFAULT(tp_self, (tp_self)->tp_math->tp_inplace_pow, p_self, other)
#define DeeType_InvokeCmpHash(tp_self, self)                                       DeeType_invoke_cmp_tp_hash(tp_self, (tp_self)->tp_cmp->tp_hash, self)
#define DeeType_InvokeCmpHash_NODEFAULT(tp_self, self)                             DeeType_invoke_cmp_tp_hash_NODEFAULT(tp_self, (tp_self)->tp_cmp->tp_hash, self)
#define DeeType_InvokeCmpEq(tp_self, self, other)                                  DeeType_invoke_cmp_tp_eq(tp_self, (tp_self)->tp_cmp->tp_eq, self, other)
#define DeeType_InvokeCmpEq_NODEFAULT(tp_self, self, other)                        DeeType_invoke_cmp_tp_eq_NODEFAULT(tp_self, (tp_self)->tp_cmp->tp_eq, self, other)
#define DeeType_InvokeCmpNe(tp_self, self, other)                                  DeeType_invoke_cmp_tp_ne(tp_self, (tp_self)->tp_cmp->tp_ne, self, other)
#define DeeType_InvokeCmpNe_NODEFAULT(tp_self, self, other)                        DeeType_invoke_cmp_tp_ne_NODEFAULT(tp_self, (tp_self)->tp_cmp->tp_ne, self, other)
#define DeeType_InvokeCmpLo(tp_self, self, other)                                  DeeType_invoke_cmp_tp_lo(tp_self, (tp_self)->tp_cmp->tp_lo, self, other)
#define DeeType_InvokeCmpLo_NODEFAULT(tp_self, self, other)                        DeeType_invoke_cmp_tp_lo_NODEFAULT(tp_self, (tp_self)->tp_cmp->tp_lo, self, other)
#define DeeType_InvokeCmpLe(tp_self, self, other)                                  DeeType_invoke_cmp_tp_le(tp_self, (tp_self)->tp_cmp->tp_le, self, other)
#define DeeType_InvokeCmpLe_NODEFAULT(tp_self, self, other)                        DeeType_invoke_cmp_tp_le_NODEFAULT(tp_self, (tp_self)->tp_cmp->tp_le, self, other)
#define DeeType_InvokeCmpGr(tp_self, self, other)                                  DeeType_invoke_cmp_tp_gr(tp_self, (tp_self)->tp_cmp->tp_gr, self, other)
#define DeeType_InvokeCmpGr_NODEFAULT(tp_self, self, other)                        DeeType_invoke_cmp_tp_gr_NODEFAULT(tp_self, (tp_self)->tp_cmp->tp_gr, self, other)
#define DeeType_InvokeCmpGe(tp_self, self, other)                                  DeeType_invoke_cmp_tp_ge(tp_self, (tp_self)->tp_cmp->tp_ge, self, other)
#define DeeType_InvokeCmpGe_NODEFAULT(tp_self, self, other)                        DeeType_invoke_cmp_tp_ge_NODEFAULT(tp_self, (tp_self)->tp_cmp->tp_ge, self, other)
#define DeeType_InvokeSeqIter(tp_self, self)                                       DeeType_invoke_seq_tp_iter(tp_self, (tp_self)->tp_seq->tp_iter, self)
#define DeeType_InvokeSeqIter_NODEFAULT(tp_self, self)                             DeeType_invoke_seq_tp_iter_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_iter, self)
#define DeeType_InvokeSeqSizeOb(tp_self, self)                                     DeeType_invoke_seq_tp_sizeob(tp_self, (tp_self)->tp_seq->tp_sizeob, self)
#define DeeType_InvokeSeqSizeOb_NODEFAULT(tp_self, self)                           DeeType_invoke_seq_tp_sizeob_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_sizeob, self)
#define DeeType_InvokeSeqContains(tp_self, self, other)                            DeeType_invoke_seq_tp_contains(tp_self, (tp_self)->tp_seq->tp_contains, self, other)
#define DeeType_InvokeSeqContains_NODEFAULT(tp_self, self, other)                  DeeType_invoke_seq_tp_contains_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_contains, self, other)
#define DeeType_InvokeSeqGetItem(tp_self, self, index)                             DeeType_invoke_seq_tp_getitem(tp_self, (tp_self)->tp_seq->tp_getitem, self, index)
#define DeeType_InvokeSeqGetItem_NODEFAULT(tp_self, self, index)                   DeeType_invoke_seq_tp_getitem_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_getitem, self, index)
#define DeeType_InvokeSeqDelItem(tp_self, self, index)                             DeeType_invoke_seq_tp_delitem(tp_self, (tp_self)->tp_seq->tp_delitem, self, index)
#define DeeType_InvokeSeqDelItem_NODEFAULT(tp_self, self, index)                   DeeType_invoke_seq_tp_delitem_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_delitem, self, index)
#define DeeType_InvokeSeqSetItem(tp_self, self, index, value)                      DeeType_invoke_seq_tp_setitem(tp_self, (tp_self)->tp_seq->tp_setitem, self, index, value)
#define DeeType_InvokeSeqSetItem_NODEFAULT(tp_self, self, index, value)            DeeType_invoke_seq_tp_setitem_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_setitem, self, index, value)
#define DeeType_InvokeSeqGetRange(tp_self, self, start, end)                       DeeType_invoke_seq_tp_getrange(tp_self, (tp_self)->tp_seq->tp_getrange, self, start, end)
#define DeeType_InvokeSeqGetRange_NODEFAULT(tp_self, self, start, end)             DeeType_invoke_seq_tp_getrange_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_getrange, self, start, end)
#define DeeType_InvokeSeqDelRange(tp_self, self, start, end)                       DeeType_invoke_seq_tp_delrange(tp_self, (tp_self)->tp_seq->tp_delrange, self, start, end)
#define DeeType_InvokeSeqDelRange_NODEFAULT(tp_self, self, start, end)             DeeType_invoke_seq_tp_delrange_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_delrange, self, start, end)
#define DeeType_InvokeSeqSetRange(tp_self, self, start, end, values)               DeeType_invoke_seq_tp_setrange(tp_self, (tp_self)->tp_seq->tp_setrange, self, start, end, values)
#define DeeType_InvokeSeqSetRange_NODEFAULT(tp_self, self, start, end, values)     DeeType_invoke_seq_tp_setrange_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_setrange, self, start, end, values)
#define DeeType_InvokeSeqForeach(tp_self, self, proc, arg)                         DeeType_invoke_seq_tp_foreach(tp_self, (tp_self)->tp_seq->tp_foreach, self, proc, arg)
#define DeeType_InvokeSeqForeach_NODEFAULT(tp_self, self, proc, arg)               DeeType_invoke_seq_tp_foreach_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_foreach, self, proc, arg)
#define DeeType_InvokeSeqForeachPair(tp_self, self, proc, arg)                     DeeType_invoke_seq_tp_foreach_pair(tp_self, (tp_self)->tp_seq->tp_foreach_pair, self, proc, arg)
#define DeeType_InvokeSeqForeachPair_NODEFAULT(tp_self, self, proc, arg)           DeeType_invoke_seq_tp_foreach_pair_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_foreach_pair, self, proc, arg)
#define DeeType_InvokeSeqBoundItem(tp_self, self, index)                           DeeType_invoke_seq_tp_bounditem(tp_self, (tp_self)->tp_seq->tp_bounditem, self, index)
#define DeeType_InvokeSeqBoundItem_NODEFAULT(tp_self, self, index)                 DeeType_invoke_seq_tp_bounditem_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_bounditem, self, index)
#define DeeType_InvokeSeqHasItem(tp_self, self, index)                             DeeType_invoke_seq_tp_hasitem(tp_self, (tp_self)->tp_seq->tp_hasitem, self, index)
#define DeeType_InvokeSeqHasItem_NODEFAULT(tp_self, self, index)                   DeeType_invoke_seq_tp_hasitem_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_hasitem, self, index)
#define DeeType_InvokeSeqSize(tp_self, self)                                       DeeType_invoke_seq_tp_size(tp_self, (tp_self)->tp_seq->tp_size, self)
#define DeeType_InvokeSeqSize_NODEFAULT(tp_self, self)                             DeeType_invoke_seq_tp_size_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_size, self)
#define DeeType_InvokeSeqGetItemIndex(tp_self, self, index)                        DeeType_invoke_seq_tp_getitem_index(tp_self, (tp_self)->tp_seq->tp_getitem_index, self, index)
#define DeeType_InvokeSeqGetItemIndex_NODEFAULT(tp_self, self, index)              DeeType_invoke_seq_tp_getitem_index_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_getitem_index, self, index)
#define DeeType_InvokeSeqDelItemIndex(tp_self, self, index)                        DeeType_invoke_seq_tp_delitem_index(tp_self, (tp_self)->tp_seq->tp_delitem_index, self, index)
#define DeeType_InvokeSeqDelItemIndex_NODEFAULT(tp_self, self, index)              DeeType_invoke_seq_tp_delitem_index_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_delitem_index, self, index)
#define DeeType_InvokeSeqSetItemIndex(tp_self, self, index, value)                 DeeType_invoke_seq_tp_setitem_index(tp_self, (tp_self)->tp_seq->tp_setitem_index, self, index, value)
#define DeeType_InvokeSeqSetItemIndex_NODEFAULT(tp_self, self, index, value)       DeeType_invoke_seq_tp_setitem_index_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_setitem_index, self, index, value)
#define DeeType_InvokeSeqBoundItemIndex(tp_self, self, index)                      DeeType_invoke_seq_tp_bounditem_index(tp_self, (tp_self)->tp_seq->tp_bounditem_index, self, index)
#define DeeType_InvokeSeqBoundItemIndex_NODEFAULT(tp_self, self, index)            DeeType_invoke_seq_tp_bounditem_index_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_bounditem_index, self, index)
#define DeeType_InvokeSeqHasItemIndex(tp_self, self, index)                        DeeType_invoke_seq_tp_hasitem_index(tp_self, (tp_self)->tp_seq->tp_hasitem_index, self, index)
#define DeeType_InvokeSeqHasItemIndex_NODEFAULT(tp_self, self, index)              DeeType_invoke_seq_tp_hasitem_index_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_hasitem_index, self, index)
#define DeeType_InvokeSeqGetRangeIndex(tp_self, self, start, end)                  DeeType_invoke_seq_tp_getrange_index(tp_self, (tp_self)->tp_seq->tp_getrange_index, self, start, end)
#define DeeType_InvokeSeqGetRangeIndex_NODEFAULT(tp_self, self, start, end)        DeeType_invoke_seq_tp_getrange_index_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_getrange_index, self, start, end)
#define DeeType_InvokeSeqDelRangeIndex(tp_self, self, start, end)                  DeeType_invoke_seq_tp_delrange_index(tp_self, (tp_self)->tp_seq->tp_delrange_index, self, start, end)
#define DeeType_InvokeSeqDelRangeIndex_NODEFAULT(tp_self, self, start, end)        DeeType_invoke_seq_tp_delrange_index_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_delrange_index, self, start, end)
#define DeeType_InvokeSeqSetRangeIndex(tp_self, self, start, end, value)           DeeType_invoke_seq_tp_setrange_index(tp_self, (tp_self)->tp_seq->tp_setrange_index, self, start, end, value)
#define DeeType_InvokeSeqSetRangeIndex_NODEFAULT(tp_self, self, start, end, value) DeeType_invoke_seq_tp_setrange_index_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_setrange_index, self, start, end, value)
#define DeeType_InvokeSeqGetRangeIndexN(tp_self, self, start)                      DeeType_invoke_seq_tp_getrange_index_n(tp_self, (tp_self)->tp_seq->tp_getrange_index_n, self, start)
#define DeeType_InvokeSeqGetRangeIndexN_NODEFAULT(tp_self, self, start)            DeeType_invoke_seq_tp_getrange_index_n_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_getrange_index_n, self, start)
#define DeeType_InvokeSeqDelRangeIndexN(tp_self, self, start)                      DeeType_invoke_seq_tp_delrange_index_n(tp_self, (tp_self)->tp_seq->tp_delrange_index_n, self, start)
#define DeeType_InvokeSeqDelRangeIndexN_NODEFAULT(tp_self, self, start)            DeeType_invoke_seq_tp_delrange_index_n_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_delrange_index_n, self, start)
#define DeeType_InvokeSeqSetRangeIndexN(tp_self, self, start, value)               DeeType_invoke_seq_tp_setrange_index_n(tp_self, (tp_self)->tp_seq->tp_setrange_index_n, self, start, value)
#define DeeType_InvokeSeqSetRangeIndexN_NODEFAULT(tp_self, self, start, value)     DeeType_invoke_seq_tp_setrange_index_n_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_setrange_index_n, self, start, value)
#define DeeType_InvokeAttrGetAttr(tp_self, self, name)                             DeeType_invoke_attr_tp_getattr(tp_self, (tp_self)->tp_attr->tp_getattr, self, name)
#define DeeType_InvokeAttrGetAttr_NODEFAULT(tp_self, self, name)                   DeeType_invoke_attr_tp_getattr_NODEFAULT(tp_self, (tp_self)->tp_attr->tp_getattr, self, name)
#define DeeType_InvokeAttrDelAttr(tp_self, self, name)                             DeeType_invoke_attr_tp_delattr(tp_self, (tp_self)->tp_attr->tp_delattr, self, name)
#define DeeType_InvokeAttrDelAttr_NODEFAULT(tp_self, self, name)                   DeeType_invoke_attr_tp_delattr_NODEFAULT(tp_self, (tp_self)->tp_attr->tp_delattr, self, name)
#define DeeType_InvokeAttrSetAttr(tp_self, self, name, value)                      DeeType_invoke_attr_tp_setattr(tp_self, (tp_self)->tp_attr->tp_setattr, self, name, value)
#define DeeType_InvokeAttrSetAttr_NODEFAULT(tp_self, self, name, value)            DeeType_invoke_attr_tp_setattr_NODEFAULT(tp_self, (tp_self)->tp_attr->tp_setattr, self, name, value)
#define DeeType_InvokeWithEnter(tp_self, self)                                     DeeType_invoke_with_tp_enter(tp_self, (tp_self)->tp_with->tp_enter, self)
#define DeeType_InvokeWithEnter_NODEFAULT(tp_self, self)                           DeeType_invoke_with_tp_enter_NODEFAULT(tp_self, (tp_self)->tp_with->tp_enter, self)
#define DeeType_InvokeWithLeave(tp_self, self)                                     DeeType_invoke_with_tp_leave(tp_self, (tp_self)->tp_with->tp_leave, self)
#define DeeType_InvokeWithLeave_NODEFAULT(tp_self, self)                           DeeType_invoke_with_tp_leave_NODEFAULT(tp_self, (tp_self)->tp_with->tp_leave, self)
#endif /* CONFIG_BUILDING_DEEMON */


/* Instance-member wrapper objects
 * >> class MyClass {
 * >>     member foo = 42;
 * >> }
 * >> print type MyClass.foo; // DeeInstanceMember_Type
 */
typedef struct Dee_instancemember_object DeeInstanceMemberObject;
struct Dee_instancemember_object {
	Dee_OBJECT_HEAD
	DREF DeeTypeObject               *im_type;      /* [1..1][const] The user-class type, instances of which implement this member. */
	struct Dee_class_attribute const *im_attribute; /* [1..1][const] The instance attribute (`CLASS_ATTRIBUTE_FCLASSMEM' shouldn't
	                                                 * be set, though this isn't asserted) that should be accessed. */
};

DDATDEF DeeTypeObject DeeInstanceMember_Type;
#define DeeInstanceMember_Check(ob)      DeeObject_InstanceOfExact(ob, &DeeInstanceMember_Type) /* `_instancemember' is final */
#define DeeInstanceMember_CheckExact(ob) DeeObject_InstanceOfExact(ob, &DeeInstanceMember_Type)

/* Construct a new instance member for the given `attribute' */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeInstanceMember_New(DeeTypeObject *__restrict class_type,
                      struct Dee_class_attribute const *__restrict attr);

DECL_END

#endif /* !GUARD_DEEMON_CLASS_H */
