/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_CLASS_H
#define GUARD_DEEMON_CLASS_H 1

#ifndef _DEE_WITHOUT_INCLUDES
#include "api.h"

#include <stddef.h>
#include <stdint.h>

#include "object.h"
#include "util/lock.h"
#endif /* !_DEE_WITHOUT_INCLUDES */

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
 *                                         >>     operator repr(fp: File) {
 *                                         >>         print fp: "MyClass(",;
 *                                         >>         local is_first = true;
 *                                         >>         if (foo is bound) {
 *                                         >>             if (!is_first) print fp: ", ",;
 *                                         >>             is_first = false;
 *                                         >>             print fp: ("foo: ", repr foo),;
 *                                         >>         }
 *                                         >>         if (bar is bound) {
 *                                         >>             if (!is_first) print fp: ", ",;
 *                                         >>             is_first = false;
 *                                         >>             print fp: ("bar: ", repr bar),;
 *                                         >>         }
 *                                         >>         print fp: ")",;
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
 * but will always access the native       >>         import hash from deemon;
 * attribute.                              >>         local result = hash();
 *                                         >>         if (foo is bound) result = hash(result, foo);
 * During member access, only `member'-    >>         if (bar is bound) result = hash(result, bar);
 * like fields are accessed, meaning that  >>         if (foobar is bound) result = hash(result, foobar);
 * instance-properties will not be invoked >>         return result;
 *                                         >>     }
 * Fields are compared lexicographically,  >>     operator == (other) {
 * following the same order in which they  >>         if (other !is MyClass)
 * were orignally defined.                 >>             return false;
 *                                         >>         return foo == (other as MyClass).foo &&
 *                                         >>                bar == (other as MyClass).bar &&
 *                                         >>                foobar == (other as MyClass).foobar;
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
#define Dee_CLASS_ATTRIBUTE_FREADONLY 0x0004 /* The attribute can only ever be set when not already bound (and it cannot be unbound). */
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
	                                                        * required to access user-defined attributes. */
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
DFUNDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL
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
 * >>     public static member class_field = 84;
 * >>     public member field = 42;
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
#ifndef CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_tassign(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_assign(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_tmoveassign(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_moveassign(DeeObject *self, DeeObject *other);
#endif /* !CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

/* GC support for class objects. */
INTDEF NONNULL((1, 2, 3)) void DCALL instance_tvisit(DeeTypeObject *tp_self, DeeObject *__restrict self, Dee_visit_t proc, void *arg);
INTDEF NONNULL((1, 2)) void DCALL instance_visit(DeeObject *__restrict self, Dee_visit_t proc, void *arg);
INTDEF NONNULL((1, 2)) void DCALL instance_tclear(DeeTypeObject *tp_self, DeeObject *__restrict self);
INTDEF NONNULL((1)) void DCALL instance_clear(DeeObject *__restrict self);
INTDEF NONNULL((1, 2)) void DCALL instance_tpclear(DeeTypeObject *tp_self, DeeObject *__restrict self, unsigned int gc_priority);
INTDEF NONNULL((1)) void DCALL instance_pclear(DeeObject *__restrict self, unsigned int gc_priority);
INTDEF struct type_gc Dee_tpconst instance_gc;

/* Builtin (standard) operators for hashing and comparing class objects. */
#ifndef CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS /* Replaced with generated `usrtype__*' functions */
INTDEF WUNUSED NONNULL((1, 2)) Dee_hash_t DCALL instance_builtin_thash(DeeTypeObject *tp_self, DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL instance_builtin_hash(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_builtin_tcompare(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_builtin_compare(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_builtin_tcompare_eq(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_builtin_compare_eq(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_builtin_ttrycompare_eq(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_builtin_trycompare_eq(DeeObject *self, DeeObject *other);
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
#endif /* !CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */
INTDEF WUNUSED NONNULL((1, 2)) dssize_t DCALL instance_enumattr(DeeTypeObject *tp_self, DeeObject *__restrict self, denum_t proc, void *arg);


#ifndef CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS
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
#define DeeType_invoke_iterator_tp_nextpair_NODEFAULT(tp_self, tp_nextpair, self, key_and_value) (*(tp_nextpair))(self, key_and_value)
#define DeeType_invoke_iterator_tp_nextkey_NODEFAULT(tp_self, tp_nextkey, self) (*(tp_nextkey))(self)
#define DeeType_invoke_iterator_tp_nextvalue_NODEFAULT(tp_self, tp_nextvalue, self) (*(tp_nextvalue))(self)
#define DeeType_invoke_iterator_tp_advance_NODEFAULT(tp_self, tp_advance, self, step) (*(tp_advance))(self, step)
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
#define DeeType_invoke_cmp_tp_compare_eq_NODEFAULT(tp_self, tp_compare_eq, self, other) \
	((tp_compare_eq) == &instance_builtin_compare_eq                                    \
	   ? instance_builtin_tcompare_eq(tp_self, self, other)                             \
	   : (*(tp_compare_eq))(self, other))
#define DeeType_invoke_cmp_tp_compare_NODEFAULT(tp_self, tp_compare, self, other) \
	((tp_compare) == &instance_builtin_compare                                    \
	   ? instance_builtin_tcompare(tp_self, self, other)                          \
	   : (*(tp_compare))(self, other))
#define DeeType_invoke_cmp_tp_trycompare_eq_NODEFAULT(tp_self, tp_trycompare_eq, self, other) \
	((tp_trycompare_eq) == &instance_builtin_trycompare_eq                                    \
	   ? instance_builtin_ttrycompare_eq(tp_self, self, other)                                \
	   : (*(tp_trycompare_eq))(self, other))
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
#define DeeType_invoke_seq_tp_foreach_NODEFAULT(tp_self, tp_foreach, self, proc, arg)                                                (*tp_foreach)(self, proc, arg)
#define DeeType_invoke_seq_tp_foreach_pair_NODEFAULT(tp_self, tp_foreach_pair, self, proc, arg)                                      (*tp_foreach_pair)(self, proc, arg)
#define DeeType_invoke_seq_tp_enumerate_NODEFAULT(tp_self, tp_enumerate, self, proc, arg)                                            (*tp_enumerate)(self, proc, arg)
#define DeeType_invoke_seq_tp_enumerate_index_NODEFAULT(tp_self, tp_enumerate_index, self, proc, arg, start, end)                    (*tp_enumerate_index)(self, proc, arg, start, end)
#define DeeType_invoke_seq_tp_iterkeys_NODEFAULT(tp_self, tp_iterkeys, self)                                                         (*tp_iterkeys)(self)
#define DeeType_invoke_seq_tp_bounditem_NODEFAULT(tp_self, tp_bounditem, self, index)                                                (*tp_bounditem)(self, index)
#define DeeType_invoke_seq_tp_hasitem_NODEFAULT(tp_self, tp_hasitem, self, index)                                                    (*tp_hasitem)(self, index)
#define DeeType_invoke_seq_tp_size_NODEFAULT(tp_self, tp_size, self)                                                                 (*tp_size)(self)
#define DeeType_invoke_seq_tp_size_fast_NODEFAULT(tp_self, tp_size_fast, self)                                                       (*tp_size_fast)(self)
#define DeeType_invoke_seq_tp_getitem_index_NODEFAULT(tp_self, tp_getitem_index, self, index)                                        (*tp_getitem_index)(self, index)
#define DeeType_invoke_seq_tp_delitem_index_NODEFAULT(tp_self, tp_delitem_index, self, index)                                        (*tp_delitem_index)(self, index)
#define DeeType_invoke_seq_tp_setitem_index_NODEFAULT(tp_self, tp_setitem_index, self, index, value)                                 (*tp_setitem_index)(self, index, value)
#define DeeType_invoke_seq_tp_bounditem_index_NODEFAULT(tp_self, tp_bounditem_index, self, index)                                    (*tp_bounditem_index)(self, index)
#define DeeType_invoke_seq_tp_hasitem_index_NODEFAULT(tp_self, tp_hasitem_index, self, index)                                        (*tp_hasitem_index)(self, index)
#define DeeType_invoke_seq_tp_getrange_index_NODEFAULT(tp_self, tp_getrange_index, self, start, end)                                 (*tp_getrange_index)(self, start, end)
#define DeeType_invoke_seq_tp_delrange_index_NODEFAULT(tp_self, tp_delrange_index, self, start, end)                                 (*tp_delrange_index)(self, start, end)
#define DeeType_invoke_seq_tp_setrange_index_NODEFAULT(tp_self, tp_setrange_index, self, start, end, value)                          (*tp_setrange_index)(self, start, end, value)
#define DeeType_invoke_seq_tp_getrange_index_n_NODEFAULT(tp_self, tp_getrange_index_n, self, start)                                  (*tp_getrange_index_n)(self, start)
#define DeeType_invoke_seq_tp_delrange_index_n_NODEFAULT(tp_self, tp_delrange_index_n, self, start)                                  (*tp_delrange_index_n)(self, start)
#define DeeType_invoke_seq_tp_setrange_index_n_NODEFAULT(tp_self, tp_setrange_index_n, self, start, value)                           (*tp_setrange_index_n)(self, start, value)
#define DeeType_invoke_seq_tp_unpack_NODEFAULT(tp_self, tp_unpack, self, dst_length, dst)                                            (*tp_unpack)(self, dst_length, dst)
#define DeeType_invoke_seq_tp_unpack_ex_NODEFAULT(tp_self, tp_unpack_ex, self, dst_length_min, dst_length_max, dst)                  (*tp_unpack_ex)(self, dst_length_min, dst_length_max, dst)
#define DeeType_invoke_seq_tp_unpack_ub_NODEFAULT(tp_self, tp_unpack_ub, self, dst_length, dst)                                      (*tp_unpack_ub)(self, dst_length, dst)
#define DeeType_invoke_seq_tp_trygetitem_NODEFAULT(tp_self, tp_trygetitem, self, index)                                              (*tp_trygetitem)(self, index)
#define DeeType_invoke_seq_tp_trygetitem_index_NODEFAULT(tp_self, tp_trygetitem_index, self, index)                                  (*tp_trygetitem_index)(self, index)
#define DeeType_invoke_seq_tp_trygetitem_string_hash_NODEFAULT(tp_self, tp_trygetitem_string_hash, self, key, hash)                  (*tp_trygetitem_string_hash)(self, key, hash)
#define DeeType_invoke_seq_tp_getitem_string_hash_NODEFAULT(tp_self, tp_getitem_string_hash, self, key, hash)                        (*tp_getitem_string_hash)(self, key, hash)
#define DeeType_invoke_seq_tp_delitem_string_hash_NODEFAULT(tp_self, tp_delitem_string_hash, self, key, hash)                        (*tp_delitem_string_hash)(self, key, hash)
#define DeeType_invoke_seq_tp_setitem_string_hash_NODEFAULT(tp_self, tp_setitem_string_hash, self, key, hash, value)                 (*tp_setitem_string_hash)(self, key, hash, value)
#define DeeType_invoke_seq_tp_bounditem_string_hash_NODEFAULT(tp_self, tp_bounditem_string_hash, self, key, hash)                    (*tp_bounditem_string_hash)(self, key, hash)
#define DeeType_invoke_seq_tp_hasitem_string_hash_NODEFAULT(tp_self, tp_hasitem_string_hash, self, key, hash)                        (*tp_hasitem_string_hash)(self, key, hash)
#define DeeType_invoke_seq_tp_trygetitem_string_len_hash_NODEFAULT(tp_self, tp_trygetitem_string_len_hash, self, key, keylen, hash)  (*tp_trygetitem_string_len_hash)(self, key, keylen, hash)
#define DeeType_invoke_seq_tp_getitem_string_len_hash_NODEFAULT(tp_self, tp_getitem_string_len_hash, self, key, keylen, hash)        (*tp_getitem_string_len_hash)(self, key, keylen, hash)
#define DeeType_invoke_seq_tp_delitem_string_len_hash_NODEFAULT(tp_self, tp_delitem_string_len_hash, self, key, keylen, hash)        (*tp_delitem_string_len_hash)(self, key, keylen, hash)
#define DeeType_invoke_seq_tp_setitem_string_len_hash_NODEFAULT(tp_self, tp_setitem_string_len_hash, self, key, keylen, hash, value) (*tp_setitem_string_len_hash)(self, key, keylen, hash, value)
#define DeeType_invoke_seq_tp_bounditem_string_len_hash_NODEFAULT(tp_self, tp_bounditem_string_len_hash, self, key, keylen, hash)    (*tp_bounditem_string_len_hash)(self, key, keylen, hash)
#define DeeType_invoke_seq_tp_hasitem_string_len_hash_NODEFAULT(tp_self, tp_hasitem_string_len_hash, self, key, keylen, hash)        (*tp_hasitem_string_len_hash)(self, key, keylen, hash)

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


/* NOTE: All of this stuff will go away after `CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS',
 *       but semantics implemented by stuff marked as "DEPRECATED" will no longer be provided
 *       at the object operator level, but rather at the method hint level (with automatic
 *       inlining of method hints into operators where applicable).
 *
 * iow: Instead of providing the "tp_iter <=> tp_size+tp_getitem_index" alias during operator
 *      inheritance, during inherit of "tp_iter" a check is made if the operator represents
 *      a default impl (e.g. `default__seq_operator_iter__with_callattr___seq_iter__') which
 *      then causes results in the method hint "Dee_TMH_seq_operator_iter" being loaded into
 *      the target type's `tp_seq->tp_iter' operator slot:
 *
 * Reference implementation for how "tp_iter" must be inherited under
 * "CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS". Note that this code will
 * never be written verbatim, but rather be implemented dynamically by
 * using spec DBs like including "Dee_tno_specs" and "mh_init_specs".
 * >> inherit_tp_iter(DeeTypeObject *into, DeeTypeObject *from) {
 * >>     DREF DeeObject *(DCALL *tp_iter)(DeeObject *__restrict);
 * >>     // Check for possible in-type substitutions (as defined by "using ..." defs in /src/method-hints/*.h)
 * >>     // When there are multiple possibility (first matching condition decides):
 * >>     // - prefer those with dependencies that are all: not IS_OPERATOR_HINT_DEAFULT_IMPL
 * >>     // - prefer those that were defined first in /src/method-hints/*.h
 * >>     if (into->tp_seq->tp_foreach) {
 * >>         tp_iter = &default__seq_iter__with__seq_foreach;
 * >>     } else if (into->tp_seq->tp_foreach_pair) {
 * >>         tp_iter = &default__seq_iter__with__seq_foreach_pair;
 * >>     } else {
 * >>         // Actually inherit the operator
 * >>         tp_iter = from->tp_seq->tp_iter;
 * >>         if (!tp_iter) {
 * >>             ... // Recursively inherit in "from"
 * >>             tp_iter = from->tp_seq->tp_iter;
 * >>             if (!tp_iter)
 * >>                 return false; // Cannot inherit from this type
 * >>         }
 * >>
 * >>         if (IS_OPERATOR_HINT_DEAFULT_IMPL(tp_iter)) {
 * >>             ... // Recursively inherit dependencies from "from" into "into"
 * >>                 // - if (tp_iter == default__seq_iter__with__seq_foreach) INHERIT(tp_seq->tp_foreach)
 * >>                 // - if (tp_iter == default__cast_str__with__cast_print) INHERIT(tp_cast.tp_print)
 * >>             tp_iter = tp_iter; // After recursive dependencies are inherited, can re-use the default impl
 * >>         } else if (IS_METHOD_HINT_DEFAULT_IMPL(tp_iter)) {
 * >>             // The operator originates from an inlined method hint
 * >>             // Repeat the inlining operation. The used resolver here
 * >>             // is the method hint that both aliases the operator that
 * >>             // is being inherited (here: "tp_iter"), and whose alias
 * >>             // condition matches "into" first (iow: has its condition
 * >>             // fulfilled the earliest when enumerating "into.__mro__")
 * >>             tp_iter = DeeType_GetSeqClass(into) == Dee_SEQCLASS_SEQ
 * >>                       ? DeeType_RequireMethodHint(into, Dee_TMH_seq_operator_iter)
 * >>                       : DeeType_RequireMethodHint(into, Dee_TMH_set_operator_iter);
 * >>             ASSERTF(tp_iter, "Since 'from' should be a base of 'into', ther *must* be a valid impl");
 * >>         } else {
 * >>             // "tp_iter" can be inherited as-is
 * >>         }
 * >>     }
 * >>     ASSERT(tp_iter);
 * >>     ASSERTF(!into->tp_seq->tp_iter || into->tp_seq->tp_iter == tp_iter,
 * >>             "Operator slot in 'into' may already be assigned if another thread was faster");
 * >>     into->tp_seq->tp_iter = tp_iter;
 * >> }
 * [IS_OPERATOR_HINT_DEAFULT_IMPL]: Checks for "default__seq_iter__with__seq_foreach", ...
 * [IS_METHOD_HINT_DEFAULT_IMPL]:   Checks for "default__seq_operator_iter__with_callattr___seq_iter__", ...
 */

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
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultAddWithSub(DeeObject *self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultAddWithInplaceSub(DeeObject *self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultSubWithInplaceSub(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultSubWithAdd(DeeObject *self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultSubWithInplaceAdd(DeeObject *self, DeeObject *other); /* DEPRECATED */
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
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultInplaceAddWithSub(DREF DeeObject **p_self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultInplaceAddWithInplaceSub(DREF DeeObject **p_self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultInplaceSubWithSub(DREF DeeObject **p_self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultInplaceSubWithAdd(DREF DeeObject **p_self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultInplaceSubWithInplaceAdd(DREF DeeObject **p_self, DeeObject *other); /* DEPRECATED */
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
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultIncWithInplaceSub(DREF DeeObject **p_self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultIncWithSub(DREF DeeObject **p_self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultDecWithInplaceAdd(DREF DeeObject **p_self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultDecWithAdd(DREF DeeObject **p_self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultDecWithInplaceSub(DREF DeeObject **p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultDecWithSub(DREF DeeObject **p_self);

/* Default wrappers for implementing ==/!=/</<=/>/>= using their logical inverse. */

/* tp_bool */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoolWithSize(DeeObject *self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoolWithSizeOb(DeeObject *self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoolWithForeach(DeeObject *self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoolWithCompareEq(DeeObject *self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoolWithEq(DeeObject *self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoolWithNe(DeeObject *self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoolWithForeachDefault(DeeObject *self); /* DEPRECATED */

/* tp_hash */
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL DeeSeq_DefaultHashWithSizeAndGetItemIndexFast(DeeObject *self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL DeeSeq_DefaultHashWithForeach(DeeObject *self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL DeeSeq_DefaultHashWithSizeAndTryGetItemIndex(DeeObject *self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL DeeSeq_DefaultHashWithSizeAndGetItemIndex(DeeObject *self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL DeeSeq_DefaultHashWithSizeObAndGetItem(DeeObject *self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL DeeSeq_DefaultHashWithForeachDefault(DeeObject *self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL DeeSet_DefaultHashWithForeachDefault(DeeObject *self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL DeeMap_DefaultHashWithForeachPairDefault(DeeObject *self); /* DEPRECATED */

/* tp_eq */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultEqWithCompareEq(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultEqWithNe(DeeObject *self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultEqWithLoAndGr(DeeObject *self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultEqWithLeAndGe(DeeObject *self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultEqWithCompareEqDefault(DeeObject *self, DeeObject *other);

/* tp_ne */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultNeWithCompareEq(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultNeWithEq(DeeObject *self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultNeWithLoAndGr(DeeObject *self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultNeWithLeAndGe(DeeObject *self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultNeWithCompareEqDefault(DeeObject *self, DeeObject *other);

/* tp_lo */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultLoWithCompare(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultLoWithGe(DeeObject *self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultLoWithCompareDefault(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultLoWithForeachDefault(DeeObject *self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultLoWithForeachPairDefault(DeeObject *self, DeeObject *other); /* DEPRECATED */

/* tp_le */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultLeWithCompare(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultLeWithGr(DeeObject *self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultLeWithCompareDefault(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultLeWithForeachDefault(DeeObject *self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultLeWithForeachPairDefault(DeeObject *self, DeeObject *other); /* DEPRECATED */

/* tp_gr */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultGrWithCompare(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultGrWithLe(DeeObject *self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultGrWithCompareDefault(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultGrWithForeachDefault(DeeObject *self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultGrWithForeachPairDefault(DeeObject *self, DeeObject *other); /* DEPRECATED */

/* tp_ge */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultGeWithCompare(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultGeWithLo(DeeObject *self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultGeWithCompareDefault(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultGeWithForeachDefault(DeeObject *self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultGeWithForeachPairDefault(DeeObject *self, DeeObject *other); /* DEPRECATED */

/* tp_compare_eq */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultCompareEqWithEq(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultCompareEqWithNe(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultCompareEqWithLoAndGr(DeeObject *self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultCompareEqWithLeAndGe(DeeObject *self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultCompareEqWithForeachDefault(DeeObject *self, DeeObject *other); /* Also use when non-Default would be usable */ /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultCompareEqWithSizeAndGetItemIndexFast(DeeObject *self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultCompareEqWithSizeAndTryGetItemIndex(DeeObject *self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultCompareEqWithSizeAndGetItemIndex(DeeObject *self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultCompareEqWithSizeObAndGetItem(DeeObject *self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultCompareEqWithForeachDefault(DeeObject *self, DeeObject *other); /* Also use when non-Default would be usable */ /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultCompareEqWithForeachPairDefault(DeeObject *self, DeeObject *other); /* Also use when non-Default would be usable */ /* DEPRECATED */

/* tp_trycompare_eq */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultTryCompareEqWithCompareEq(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultTryCompareEqWithEq(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultTryCompareEqWithNe(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultTryCompareEqWithCompare(DeeObject *self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultTryCompareEqWithLoAndGr(DeeObject *self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultTryCompareEqWithLeAndGe(DeeObject *self, DeeObject *other); /* DEPRECATED */
#if 0
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultTryCompareEqWithForeachDefault(DeeObject *self, DeeObject *other); /* Also use when non-Default would be usable */ /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultTryCompareEqWithSizeAndGetItemIndexFast(DeeObject *self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultTryCompareEqWithSizeAndTryGetItemIndex(DeeObject *self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultTryCompareEqWithSizeAndGetItemIndex(DeeObject *self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultTryCompareEqWithSizeObAndGetItem(DeeObject *self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultTryCompareEqWithForeachDefault(DeeObject *self, DeeObject *other); /* Also use when non-Default would be usable */ /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultTryCompareEqWithForeachPairDefault(DeeObject *self, DeeObject *other); /* Also use when non-Default would be usable */ /* DEPRECATED */
#endif
#define DeeSeq_DefaultTryCompareEqWithForeachDefault           DeeSeq_DefaultCompareEqWithForeachDefault /* DEPRECATED */
#define DeeSeq_DefaultTryCompareEqWithSizeAndGetItemIndexFast  DeeSeq_DefaultCompareEqWithSizeAndGetItemIndexFast /* DEPRECATED */
#define DeeSeq_DefaultTryCompareEqWithSizeAndTryGetItemIndex   DeeSeq_DefaultCompareEqWithSizeAndTryGetItemIndex /* DEPRECATED */
#define DeeSeq_DefaultTryCompareEqWithSizeAndGetItemIndex      DeeSeq_DefaultCompareEqWithSizeAndGetItemIndex /* DEPRECATED */
#define DeeSeq_DefaultTryCompareEqWithSizeObAndGetItem         DeeSeq_DefaultCompareEqWithSizeObAndGetItem /* DEPRECATED */
#define DeeSet_DefaultTryCompareEqWithForeachDefault           DeeSet_DefaultCompareEqWithForeachDefault /* DEPRECATED */
#define DeeMap_DefaultTryCompareEqWithForeachPairDefault       DeeMap_DefaultCompareEqWithForeachPairDefault /* DEPRECATED */
#define DeeSeq_TDefaultTryCompareEqWithForeachDefault          DeeSeq_TDefaultCompareEqWithForeachDefault /* DEPRECATED */
#define DeeSeq_TDefaultTryCompareEqWithSizeAndGetItemIndexFast DeeSeq_TDefaultCompareEqWithSizeAndGetItemIndexFast /* DEPRECATED */
#define DeeSeq_TDefaultTryCompareEqWithSizeAndTryGetItemIndex  DeeSeq_TDefaultCompareEqWithSizeAndTryGetItemIndex /* DEPRECATED */
#define DeeSeq_TDefaultTryCompareEqWithSizeAndGetItemIndex     DeeSeq_TDefaultCompareEqWithSizeAndGetItemIndex /* DEPRECATED */
#define DeeSeq_TDefaultTryCompareEqWithSizeObAndGetItem        DeeSeq_TDefaultCompareEqWithSizeObAndGetItem /* DEPRECATED */
#define DeeSet_TDefaultTryCompareEqWithForeachDefault          DeeSet_TDefaultCompareEqWithForeachDefault /* DEPRECATED */
#define DeeMap_TDefaultTryCompareEqWithForeachPairDefault      DeeMap_TDefaultCompareEqWithForeachPairDefault /* DEPRECATED */

/* tp_compare */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultCompareWithEqAndLo(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultCompareWithEqAndLe(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultCompareWithEqAndGr(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultCompareWithEqAndGe(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultCompareWithNeAndLo(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultCompareWithNeAndLe(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultCompareWithNeAndGr(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultCompareWithNeAndGe(DeeObject *self, DeeObject *other);
#if 0
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultCompareWithLoAndGr(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultCompareWithLeAndGe(DeeObject *self, DeeObject *other);
#endif
#define DeeObject_DefaultCompareWithLoAndGr  DeeObject_DefaultCompareEqWithLoAndGr
#define DeeObject_DefaultCompareWithLeAndGe  DeeObject_DefaultCompareEqWithLeAndGe
#define DeeObject_TDefaultCompareWithLoAndGr DeeObject_TDefaultCompareEqWithLoAndGr
#define DeeObject_TDefaultCompareWithLeAndGe DeeObject_TDefaultCompareEqWithLeAndGe
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultCompareWithSizeAndGetItemIndexFast(DeeObject *self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultCompareWithSizeAndTryGetItemIndex(DeeObject *self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultCompareWithSizeAndGetItemIndex(DeeObject *self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultCompareWithSizeObAndGetItem(DeeObject *self, DeeObject *other); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultCompareWithForeachDefault(DeeObject *self, DeeObject *other); /* Also use when non-Default would be usable */ /* DEPRECATED */


INTDEF struct type_cmp DeeSeq_DefaultCmpWithSizeAndGetItemIndexFast; /* DEPRECATED */
INTDEF struct type_cmp DeeSeq_DefaultCmpWithSizeAndTryGetItemIndex; /* DEPRECATED */
INTDEF struct type_cmp DeeSeq_DefaultCmpWithSizeAndGetItemIndex; /* DEPRECATED */
INTDEF struct type_cmp DeeSeq_DefaultCmpWithSizeObAndGetItem; /* DEPRECATED */
INTDEF struct type_cmp DeeSeq_DefaultCmpWithForeachDefault; /* DEPRECATED */
INTDEF struct type_cmp DeeSet_DefaultCmpWithForeachDefault; /* DEPRECATED */
INTDEF struct type_cmp DeeMap_DefaultCmpWithForeachPairDefault; /* DEPRECATED */

/* Default wrappers for implementing iterator operators. */
INTDEF struct type_iterator DeeObject_DefaultIteratorWithIterNext;
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultIterNextWithIterNextPair(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultIterNextPairWithIterNext(DeeObject *__restrict self, /*out*/ DREF DeeObject *key_and_value[2]);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultIterNextKeyWithIterNext(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultIterNextKeyWithIterNextPair(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultIterNextValueWithIterNext(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultIterNextValueWithIterNextPair(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeObject_DefaultIterAdvanceWithIterNext(DeeObject *__restrict self, size_t step);
INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeObject_DefaultIterAdvanceWithIterNextPair(DeeObject *__restrict self, size_t step);
INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeObject_DefaultIterAdvanceWithIterNextKey(DeeObject *__restrict self, size_t step);
INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeObject_DefaultIterAdvanceWithIterNextValue(DeeObject *__restrict self, size_t step);


/* Default wrappers for implementing sequence operators. */

/* tp_iter */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultIterWithForeach(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultIterWithForeachPair(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultIterWithEnumerate(DeeObject *__restrict self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultIterWithEnumerateIndex(DeeObject *__restrict self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultIterWithIterKeysAndTryGetItem(DeeObject *__restrict self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultIterWithIterKeysAndGetItem(DeeObject *__restrict self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultIterWithIterKeysAndTryGetItemDefault(DeeObject *__restrict self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultIterWithSizeAndGetItemIndexFast(DeeObject *__restrict self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultIterWithSizeAndTryGetItemIndex(DeeObject *__restrict self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultIterWithSizeAndGetItemIndex(DeeObject *__restrict self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultIterWithGetItemIndex(DeeObject *__restrict self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultIterWithSizeObAndGetItem(DeeObject *__restrict self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultIterWithGetItem(DeeObject *__restrict self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultIterWithEnumerate(DeeObject *__restrict self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultIterWithEnumerateIndex(DeeObject *__restrict self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultIterWithIterKeysAndTryGetItem(DeeObject *__restrict self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultIterWithIterKeysAndGetItem(DeeObject *__restrict self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultIterWithIterKeysAndTryGetItemDefault(DeeObject *__restrict self); /* DEPRECATED */

/* tp_foreach */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeObject_DefaultForeachWithIter(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeObject_DefaultForeachWithEnumerate(DeeObject *__restrict self, Dee_foreach_t proc, void *arg); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeObject_DefaultForeachWithEnumerateIndex(DeeObject *__restrict self, Dee_foreach_t proc, void *arg); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeObject_DefaultForeachWithForeachPair(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeObject_DefaultForeachWithForeachPairDefault(DeeObject *__restrict self, Dee_foreach_t proc, void *arg); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeObject_DefaultForeachWithIterKeysAndTryGetItem(DeeObject *__restrict self, Dee_foreach_t proc, void *arg); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeObject_DefaultForeachWithIterKeysAndGetItem(DeeObject *__restrict self, Dee_foreach_t proc, void *arg); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeObject_DefaultForeachWithIterKeysAndTryGetItemDefault(DeeObject *__restrict self, Dee_foreach_t proc, void *arg); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultForeachWithSizeAndGetItemIndexFast(DeeObject *__restrict self, Dee_foreach_t proc, void *arg); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultForeachWithSizeAndTryGetItemIndex(DeeObject *__restrict self, Dee_foreach_t proc, void *arg); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultForeachWithSizeAndGetItemIndex(DeeObject *__restrict self, Dee_foreach_t proc, void *arg); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultForeachWithSizeObAndGetItem(DeeObject *__restrict self, Dee_foreach_t proc, void *arg); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault(DeeObject *__restrict self, Dee_foreach_t proc, void *arg); /* May call other DEFAULT operators */ /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultForeachWithGetItemIndexDefault(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);               /* May call other DEFAULT operators */ /* DEPRECATED */

/* tp_foreach_pair */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeObject_DefaultForeachPairWithForeach(DeeObject *__restrict self, Dee_foreach_pair_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeObject_DefaultForeachPairWithEnumerate(DeeObject *__restrict self, Dee_foreach_pair_t proc, void *arg); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeObject_DefaultForeachPairWithEnumerateIndex(DeeObject *__restrict self, Dee_foreach_pair_t proc, void *arg); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeObject_DefaultForeachPairWithForeachDefault(DeeObject *__restrict self, Dee_foreach_pair_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeObject_DefaultForeachPairWithIter(DeeObject *__restrict self, Dee_foreach_pair_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeMap_DefaultForeachPairWithEnumerate(DeeObject *__restrict self, Dee_foreach_pair_t proc, void *arg); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeMap_DefaultForeachPairWithEnumerateIndex(DeeObject *__restrict self, Dee_foreach_pair_t proc, void *arg); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeMap_DefaultForeachPairWithEnumerateDefault(DeeObject *__restrict self, Dee_foreach_pair_t proc, void *arg); /* May call other DEFAULT operators */ /* DEPRECATED */

/* tp_enumerate */ /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeObject_DefaultEnumerateWithEnumerateIndex(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeObject_DefaultEnumerateWithIterKeysAndTryGetItem(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeObject_DefaultEnumerateWithIterKeysAndGetItem(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeObject_DefaultEnumerateWithIterKeysAndTryGetItemDefault(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultEnumerateWithSizeAndGetItemIndexFast(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultEnumerateWithSizeAndTryGetItemIndex(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultEnumerateWithSizeAndGetItemIndex(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultEnumerateWithSizeObAndGetItem(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultEnumerateWithCounterAndForeach(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultEnumerateWithCounterAndIter(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeMap_DefaultEnumerateWithForeachPairDefault(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg); /* DEPRECATED */
#if 0
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeMap_DefaultEnumerateWithIter(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg); /* DEPRECATED */
#endif
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultEnumerateWithSizeDefaultAndGetItemIndexDefault(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg); /* May call other DEFAULT operators */ /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultEnumerateWithCounterAndForeachDefault(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg); /* May call other DEFAULT operators */ /* DEPRECATED */
#define DeeMap_DefaultEnumerateWithIter  DeeObject_DefaultForeachPairWithIter /* DEPRECATED */
#define DeeMap_TDefaultEnumerateWithIter DeeObject_TDefaultForeachPairWithIter /* DEPRECATED */

/* tp_enumerate_index */ /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeObject_DefaultEnumerateIndexWithEnumerate(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultEnumerateIndexWithSizeAndGetItemIndexFast(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultEnumerateIndexWithSizeAndTryGetItemIndex(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultEnumerateIndexWithSizeAndGetItemIndex(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultEnumerateIndexWithSizeObAndGetItem(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultEnumerateIndexWithCounterAndForeach(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultEnumerateIndexWithCounterAndIter(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultEnumerateIndexWithSizeDefaultAndGetItemIndexDefault(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end); /* May call other DEFAULT operators */ /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultEnumerateIndexWithCounterAndForeachDefault(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end); /* May call other DEFAULT operators */ /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeObject_DefaultEnumerateIndexWithEnumerateDefault(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end); /* May call other DEFAULT operators */ /* DEPRECATED */

/* tp_iterkeys */ /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultIterKeysWithEnumerate(DeeObject *__restrict self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultIterKeysWithEnumerateIndex(DeeObject *__restrict self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultIterKeysWithSize(DeeObject *__restrict self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultIterKeysWithSizeOb(DeeObject *__restrict self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultIterKeysWithSizeDefault(DeeObject *__restrict self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultIterKeysWithIter(DeeObject *__restrict self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultIterKeysWithIterDefault(DeeObject *__restrict self); /* DEPRECATED */
#define DeeMap_DefaultIterKeysWithIterDefault DeeMap_DefaultIterKeysWithIter /* DEPRECATED */

/* tp_sizeob */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultSizeObWithSize(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultSizeObWithSizeDefault(DeeObject *__restrict self); /* May call other DEFAULT operators */

/* tp_size */
INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeObject_DefaultSizeWithSizeOb(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeSeq_DefaultSizeWithEnumerateIndex(DeeObject *__restrict self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeSeq_DefaultSizeWithEnumerate(DeeObject *__restrict self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeSeq_DefaultSizeWithForeachPair(DeeObject *__restrict self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeSeq_DefaultSizeWithForeach(DeeObject *__restrict self); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeSeq_DefaultSizeWithIter(DeeObject *__restrict self); /* DEPRECATED */

/* tp_size_fast */
INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeObject_DefaultSizeFastWithErrorNotFast(DeeObject *__restrict self);

/* tp_contains */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultContainsWithForeachDefault(DeeObject *self, DeeObject *elem); /* May call other DEFAULT operators */ /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultContainsWithHasItem(DeeObject *self, DeeObject *elem); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultContainsWithBoundItem(DeeObject *self, DeeObject *elem); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultContainsWithTryGetItem(DeeObject *self, DeeObject *elem); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultContainsWithGetItem(DeeObject *self, DeeObject *elem); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultContainsWithHasItemStringHash(DeeObject *self, DeeObject *elem); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultContainsWithHasItemStringLenHash(DeeObject *self, DeeObject *elem); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultContainsWithHasItemIndex(DeeObject *self, DeeObject *elem); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultContainsWithBoundItemStringHash(DeeObject *self, DeeObject *elem); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultContainsWithBoundItemStringLenHash(DeeObject *self, DeeObject *elem); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultContainsWithBoundItemIndex(DeeObject *self, DeeObject *elem); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultContainsWithTryGetItemStringHash(DeeObject *self, DeeObject *elem); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultContainsWithTryGetItemStringLenHash(DeeObject *self, DeeObject *elem); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultContainsWithTryGetItemIndex(DeeObject *self, DeeObject *elem); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultContainsWithGetItemStringHash(DeeObject *self, DeeObject *elem); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultContainsWithGetItemStringLenHash(DeeObject *self, DeeObject *elem); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultContainsWithGetItemIndex(DeeObject *self, DeeObject *elem); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultContainsWithEnumerate(DeeObject *self, DeeObject *elem); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultContainsWithEnumerateDefault(DeeObject *self, DeeObject *elem); /* DEPRECATED */

/* tp_getitem */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultGetItemWithSizeAndGetItemIndexFast(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultGetItemWithGetItemIndex(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultGetItemWithGetItemStringHash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultGetItemWithGetItemStringLenHash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultGetItemWithTryGetItem(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultGetItemWithTryGetItemIndex(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultGetItemWithTryGetItemStringHash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultGetItemWithTryGetItemStringLenHash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultGetItemWithGetItemIndexDefault(DeeObject *self, DeeObject *index); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultGetItemWithTryGetItemAndSizeOb(DeeObject *self, DeeObject *index); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultGetItemWithTryGetItemAndSize(DeeObject *self, DeeObject *index); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultGetItemWithSizeAndTryGetItemIndexOb(DeeObject *self, DeeObject *index); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultGetItemWithSizeAndTryGetItemIndex(DeeObject *self, DeeObject *index); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultGetItemWithEnumerate(DeeObject *__restrict self, DeeObject *index); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultGetItemWithEnumerateDefault(DeeObject *__restrict self, DeeObject *index); /* May call other DEFAULT operators */ /* DEPRECATED */

/* tp_getitem_index */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultGetItemIndexWithSizeAndGetItemIndexFast(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultGetItemIndexWithTryGetItemIndex(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultGetItemIndexWithGetItem(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultGetItemIndexWithTryGetItem(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultGetItemIndexWithErrorRequiresString(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultGetItemIndexWithGetItemDefault(DeeObject *__restrict self, size_t index); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetItemIndexWithSizeAndTryGetItemIndex(DeeObject *__restrict self, size_t index); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetItemIndexWithSizeAndTryGetItemIndexOb(DeeObject *__restrict self, size_t index); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetItemIndexWithTryGetItemAndSize(DeeObject *__restrict self, size_t index); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetItemIndexWithTryGetItemAndSizeOb(DeeObject *__restrict self, size_t index); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetItemIndexWithForeachDefault(DeeObject *__restrict self, size_t index);    /* May call other DEFAULT operators */ /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultGetItemIndexWithEnumerate(DeeObject *__restrict self, size_t index); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultGetItemIndexWithEnumerateDefault(DeeObject *__restrict self, size_t index); /* May call other DEFAULT operators */ /* DEPRECATED */

/* tp_getitem_string_hash */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultGetItemStringHashWithGetItemStringLenHash(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultGetItemStringHashWithGetItem(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultGetItemStringHashWithTryGetItemStringHash(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultGetItemStringHashWithTryGetItemStringLenHash(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultGetItemStringHashWithTryGetItem(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultGetItemStringHashWithErrorRequiresInt(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultGetItemStringHashWithGetItemDefault(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultGetItemStringHashWithEnumerate(DeeObject *__restrict self, char const *key, Dee_hash_t hash); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultGetItemStringHashWithEnumerateDefault(DeeObject *__restrict self, char const *key, Dee_hash_t hash); /* May call other DEFAULT operators */ /* DEPRECATED */

/* tp_getitem_string_len_hash */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultGetItemStringLenHashWithGetItemStringHash(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultGetItemStringLenHashWithGetItem(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultGetItemStringLenHashWithTryGetItemStringLenHash(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultGetItemStringLenHashWithTryGetItemStringHash(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultGetItemStringLenHashWithTryGetItem(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultGetItemStringLenHashWithErrorRequiresInt(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultGetItemStringLenHashWithGetItemDefault(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultGetItemStringLenHashWithEnumerate(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultGetItemStringLenHashWithEnumerateDefault(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash); /* May call other DEFAULT operators */ /* DEPRECATED */

/* tp_trygetitem */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultTryGetItemWithTryGetItemIndex(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultTryGetItemWithTryGetItemStringHash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultTryGetItemWithTryGetItemStringLenHash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultTryGetItemWithSizeAndGetItemIndexFast(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultTryGetItemWithGetItemIndex(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultTryGetItemWithGetItem(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultTryGetItemWithGetItemStringHash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultTryGetItemWithGetItemStringLenHash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultTryGetItemWithGetItemDefault(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultTryGetItemWithEnumerate(DeeObject *__restrict self, DeeObject *index); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultTryGetItemWithEnumerateDefault(DeeObject *__restrict self, DeeObject *index); /* May call other DEFAULT operators */ /* DEPRECATED */

/* tp_trygetitem_index */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultTryGetItemIndexWithSizeAndGetItemIndexFast(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultTryGetItemIndexWithTryGetItem(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultTryGetItemIndexWithGetItemIndex(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultTryGetItemIndexWithGetItem(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultTryGetItemIndexWithErrorRequiresString(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultTryGetItemIndexWithGetItemIndexDefault(DeeObject *__restrict self, size_t index); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultTryGetItemIndexWithForeachDefault(DeeObject *__restrict self, size_t index);    /* May call other DEFAULT operators */ /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultTryGetItemIndexWithEnumerate(DeeObject *__restrict self, size_t index); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultTryGetItemIndexWithEnumerateDefault(DeeObject *__restrict self, size_t index); /* May call other DEFAULT operators */ /* DEPRECATED */

/* tp_trygetitem_string_hash */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultTryGetItemStringHashWithTryGetItemStringLenHash(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultTryGetItemStringHashWithTryGetItem(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultTryGetItemStringHashWithGetItemStringHash(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultTryGetItemStringHashWithGetItemStringLenHash(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultTryGetItemStringHashWithGetItem(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultTryGetItemStringHashWithErrorRequiresInt(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultTryGetItemStringHashWithTryGetItemDefault(DeeObject *__restrict self, char const *key, Dee_hash_t hash); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultTryGetItemStringHashWithEnumerate(DeeObject *__restrict self, char const *key, Dee_hash_t hash); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultTryGetItemStringHashWithEnumerateDefault(DeeObject *__restrict self, char const *key, Dee_hash_t hash); /* May call other DEFAULT operators */ /* DEPRECATED */

/* tp_trygetitem_string_len_hash */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultTryGetItemStringLenHashWithTryGetItemStringHash(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultTryGetItemStringLenHashWithTryGetItem(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultTryGetItemStringLenHashWithGetItemStringLenHash(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultTryGetItemStringLenHashWithGetItemStringHash(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultTryGetItemStringLenHashWithGetItem(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultTryGetItemStringLenHashWithErrorRequiresInt(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_DefaultTryGetItemStringLenHashWithTryGetItemDefault(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultTryGetItemStringLenHashWithEnumerate(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultTryGetItemStringLenHashWithEnumerateDefault(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash); /* May call other DEFAULT operators */ /* DEPRECATED */

/* tp_delitem */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultDelItemWithDelItemIndex(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultDelItemWithDelItemIndexDefault(DeeObject *self, DeeObject *index); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultDelItemWithDelItemStringHash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultDelItemWithDelItemStringLenHash(DeeObject *self, DeeObject *index);

/* tp_delitem_index */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultDelItemIndexWithDelItem(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultDelItemIndexWithErrorRequiresString(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelItemIndexWithDelRangeIndexDefault(DeeObject *__restrict self, size_t index); /* May call other DEFAULT operators */ /* DEPRECATED */

/* tp_delitem_string_hash */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultDelItemStringHashWithDelItemStringLenHash(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultDelItemStringHashWithDelItem(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultDelItemStringHashWithErrorRequiresInt(DeeObject *__restrict self, char const *key, Dee_hash_t hash);

/* tp_delitem_string_len_hash */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultDelItemStringLenHashWithDelItemStringHash(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultDelItemStringLenHashWithDelItem(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultDelItemStringLenHashWithErrorRequiresInt(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);

/* tp_setitem */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_DefaultSetItemWithSetItemIndex(DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_DefaultSetItemWithSetItemIndexDefault(DeeObject *self, DeeObject *index, DeeObject *value); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_DefaultSetItemWithSetItemStringHash(DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_DefaultSetItemWithSetItemStringLenHash(DeeObject *self, DeeObject *index, DeeObject *value);

/* tp_setitem_index */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeObject_DefaultSetItemIndexWithSetItem(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeObject_DefaultSetItemIndexWithErrorRequiresString(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultSetItemIndexWithSetRangeIndexDefault(DeeObject *self, size_t index, DeeObject *value); /* May call other DEFAULT operators */ /* DEPRECATED */

/* tp_setitem_string_hash */
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL DeeObject_DefaultSetItemStringHashWithSetItemStringLenHash(DeeObject *__restrict self, char const *key, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL DeeObject_DefaultSetItemStringHashWithSetItem(DeeObject *__restrict self, char const *key, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL DeeObject_DefaultSetItemStringHashWithErrorRequiresInt(DeeObject *__restrict self, char const *key, Dee_hash_t hash, DeeObject *value);

/* tp_setitem_string_len_hash */
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeObject_DefaultSetItemStringLenHashWithSetItemStringHash(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeObject_DefaultSetItemStringLenHashWithSetItem(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeObject_DefaultSetItemStringLenHashWithErrorRequiresInt(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value);

/* tp_bounditem */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemWithBoundItemIndex(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemWithBoundItemStringHash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemWithBoundItemStringLenHash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemWithGetItem(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemWithGetItemStringHash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemWithGetItemStringLenHash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemWithGetItemIndex(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemWithTryGetItemAndHasItem(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemWithTryGetItemIndexAndHasItemIndex(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemWithTryGetItemStringLenHashAndHasItemStringLenHash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemWithTryGetItemStringHashAndHasItemStringHash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemWithTryGetItem(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemWithTryGetItemIndex(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemWithTryGetItemStringLenHash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemWithTryGetItemStringHash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemWithGetItemDefault(DeeObject *self, DeeObject *index); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultBoundItemWithTryGetItemAndSizeOb(DeeObject *self, DeeObject *index); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultBoundItemWithSizeAndTryGetItemIndex(DeeObject *self, DeeObject *index); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultBoundItemWithContains(DeeObject *self, DeeObject *index); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultBoundItemWithEnumerate(DeeObject *self, DeeObject *index); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultBoundItemWithEnumerateDefault(DeeObject *self, DeeObject *index); /* May call other DEFAULT operators */ /* DEPRECATED */

/* tp_bounditem_index */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultBoundItemIndexWithBoundItem(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultBoundItemIndexWithSizeAndGetItemIndexFast(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultBoundItemIndexWithGetItemIndex(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultBoundItemIndexWithGetItem(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultBoundItemIndexWithGetItemIndexDefault(DeeObject *__restrict self, size_t index); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultBoundItemIndexWithTryGetItemIndexAndHasItemIndex(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultBoundItemIndexWithTryGetItemAndHasItem(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultBoundItemIndexWithErrorRequiresString(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoundItemIndexWithSizeAndTryGetItemIndex(DeeObject *__restrict self, size_t index); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoundItemIndexWithTryGetItemAndSizeOb(DeeObject *__restrict self, size_t index); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeMap_DefaultBoundItemIndexWithContains(DeeObject *__restrict self, size_t index); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeMap_DefaultBoundItemIndexWithEnumerate(DeeObject *__restrict self, size_t index); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeMap_DefaultBoundItemIndexWithEnumerateDefault(DeeObject *__restrict self, size_t index); /* May call other DEFAULT operators */ /* DEPRECATED */

/* tp_bounditem_string_hash */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemStringHashWithBoundItemStringLenHash(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemStringHashWithBoundItem(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemStringHashWithGetItemStringHash(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemStringHashWithGetItemStringLenHash(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemStringHashWithGetItem(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemStringHashWithTryGetItemStringHashAndHasItemStringHash(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemStringHashWithTryGetItemStringLenHashAndHasItemStringLenHash(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemStringHashWithTryGetItemAndHasItem(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemStringHashWithTryGetItemStringHash(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemStringHashWithTryGetItemStringLenHash(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemStringHashWithTryGetItem(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemStringHashWithBoundItemDefault(DeeObject *__restrict self, char const *key, Dee_hash_t hash); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemStringHashWithErrorRequiresInt(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultBoundItemStringHashWithContains(DeeObject *__restrict self, char const *key, Dee_hash_t hash); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultBoundItemStringHashWithEnumerate(DeeObject *__restrict self, char const *key, Dee_hash_t hash); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultBoundItemStringHashWithEnumerateDefault(DeeObject *__restrict self, char const *key, Dee_hash_t hash); /* May call other DEFAULT operators */ /* DEPRECATED */

/* tp_bounditem_string_len_hash */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemStringLenHashWithBoundItemStringHash(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemStringLenHashWithBoundItem(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemStringLenHashWithGetItemStringLenHash(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemStringLenHashWithGetItemStringHash(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemStringLenHashWithGetItem(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemStringLenHashWithTryGetItemStringLenHashAndHasItemStringLenHash(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemStringLenHashWithTryGetItemStringHashAndHasItemStringHash(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemStringLenHashWithTryGetItemAndHasItem(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemStringLenHashWithTryGetItemStringLenHash(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemStringLenHashWithTryGetItemStringHash(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemStringLenHashWithTryGetItem(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemStringLenHashWithBoundItemDefault(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultBoundItemStringLenHashWithErrorRequiresInt(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultBoundItemStringLenHashWithContains(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultBoundItemStringLenHashWithEnumerate(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultBoundItemStringLenHashWithEnumerateDefault(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash); /* May call other DEFAULT operators */ /* DEPRECATED */

/* tp_hasitem */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemWithHasItemStringHash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemWithHasItemStringLenHash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemWithHasItemIndex(DeeObject *self, DeeObject *index);
#ifndef Dee_BOUND_MAYALIAS_HAS
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemWithBoundItem(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemWithBoundItemStringHash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemWithBoundItemStringLenHash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemWithBoundItemIndex(DeeObject *self, DeeObject *index);
#endif /* !Dee_BOUND_MAYALIAS_HAS */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemWithTryGetItem(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemWithTryGetItemStringHash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemWithTryGetItemStringLenHash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemWithTryGetItemIndex(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemWithGetItem(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemWithGetItemStringHash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemWithGetItemStringLenHash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemWithGetItemIndex(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemWithGetItemDefault(DeeObject *self, DeeObject *index); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultHasItemWithSize(DeeObject *self, DeeObject *index); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultHasItemWithSizeOb(DeeObject *self, DeeObject *index); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultHasItemWithContains(DeeObject *self, DeeObject *index); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultHasItemWithEnumerate(DeeObject *self, DeeObject *index); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultHasItemWithEnumerateDefault(DeeObject *self, DeeObject *index); /* May call other DEFAULT operators */ /* DEPRECATED */

/* tp_hasitem_index */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultHasItemIndexWithHasItem(DeeObject *__restrict self, size_t index);
#ifndef Dee_BOUND_MAYALIAS_HAS
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultHasItemIndexWithBoundItemIndex(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultHasItemIndexWithBoundItem(DeeObject *__restrict self, size_t index);
#endif /* !Dee_BOUND_MAYALIAS_HAS */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultHasItemIndexWithTryGetItemIndex(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultHasItemIndexWithTryGetItem(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultHasItemIndexWithGetItemIndex(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultHasItemIndexWithGetItem(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultHasItemIndexWithGetItemIndexDefault(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultHasItemIndexWithErrorRequiresString(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultHasItemIndexWithSize(DeeObject *__restrict self, size_t index); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultHasItemIndexWithSizeDefault(DeeObject *__restrict self, size_t index); /* May call other DEFAULT operators */ /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeMap_DefaultHasItemIndexWithContains(DeeObject *__restrict self, size_t index); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeMap_DefaultHasItemIndexWithEnumerate(DeeObject *__restrict self, size_t index); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeMap_DefaultHasItemIndexWithEnumerateDefault(DeeObject *__restrict self, size_t index); /* May call other DEFAULT operators */ /* DEPRECATED */

/* tp_hasitem_string_hash */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemStringHashWithHasItemStringLenHash(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
#ifndef Dee_BOUND_MAYALIAS_HAS
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemStringHashWithBoundItemStringHash(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemStringHashWithBoundItemStringLenHash(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
#endif /* !Dee_BOUND_MAYALIAS_HAS */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemStringHashWithTryGetItemStringHash(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemStringHashWithTryGetItemStringLenHash(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemStringHashWithGetItemStringHash(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemStringHashWithGetItemStringLenHash(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemStringHashWithHasItem(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
#ifndef Dee_BOUND_MAYALIAS_HAS
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemStringHashWithBoundItem(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
#endif /* !Dee_BOUND_MAYALIAS_HAS */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemStringHashWithTryGetItem(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemStringHashWithGetItem(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemStringHashWithHasItemDefault(DeeObject *__restrict self, char const *key, Dee_hash_t hash); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemStringHashWithErrorRequiresInt(DeeObject *__restrict self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultHasItemStringHashWithContains(DeeObject *__restrict self, char const *key, Dee_hash_t hash); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultHasItemStringHashWithEnumerate(DeeObject *__restrict self, char const *key, Dee_hash_t hash); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultHasItemStringHashWithEnumerateDefault(DeeObject *__restrict self, char const *key, Dee_hash_t hash); /* May call other DEFAULT operators */ /* DEPRECATED */

/* tp_hasitem_string_len_hash */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemStringLenHashWithHasItemStringHash(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
#ifndef Dee_BOUND_MAYALIAS_HAS
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemStringLenHashWithBoundItemStringHash(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemStringLenHashWithBoundItemStringLenHash(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
#endif /* !Dee_BOUND_MAYALIAS_HAS */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemStringLenHashWithTryGetItemStringLenHash(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemStringLenHashWithTryGetItemStringHash(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemStringLenHashWithGetItemStringLenHash(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemStringLenHashWithGetItemStringHash(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemStringLenHashWithHasItem(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
#ifndef Dee_BOUND_MAYALIAS_HAS
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemStringLenHashWithBoundItem(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
#endif /* !Dee_BOUND_MAYALIAS_HAS */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemStringLenHashWithTryGetItem(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemStringLenHashWithGetItem(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemStringLenHashWithHasItemDefault(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_DefaultHasItemStringLenHashWithErrorRequiresInt(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultHasItemStringLenHashWithContains(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultHasItemStringLenHashWithEnumerate(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultHasItemStringLenHashWithEnumerateDefault(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash); /* May call other DEFAULT operators */ /* DEPRECATED */

/* tp_getrange */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_DefaultGetRangeWithGetRangeIndexAndGetRangeIndexN(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_DefaultGetRangeWithGetRangeIndexDefaultAndGetRangeIndexNDefault(DeeObject *self, DeeObject *start, DeeObject *end); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultGetRangeWithSizeDefaultAndGetItemIndex(DeeObject *self, DeeObject *start, DeeObject *end);                      /* May call other DEFAULT operators */ /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultGetRangeWithSizeDefaultAndTryGetItemIndex(DeeObject *self, DeeObject *start, DeeObject *end);                   /* May call other DEFAULT operators */ /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultGetRangeWithSizeObAndGetItem(DeeObject *self, DeeObject *start, DeeObject *end); /* DEPRECATED */

/* tp_getrange_index */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultGetRangeIndexWithGetRange(DeeObject *__restrict self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetRangeIndexWithSizeAndGetItemIndexFast(DeeObject *__restrict self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetRangeIndexWithSizeDefaultAndGetItemIndex(DeeObject *__restrict self, Dee_ssize_t start, Dee_ssize_t end); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetRangeIndexWithSizeDefaultAndGetItem(DeeObject *__restrict self, Dee_ssize_t start, Dee_ssize_t end);      /* May call other DEFAULT operators */ /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetRangeIndexWithSizeDefaultAndIter(DeeObject *__restrict self, Dee_ssize_t start, Dee_ssize_t end);         /* May call other DEFAULT operators */ /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetRangeIndexWithSizeDefaultAndIterDefault(DeeObject *__restrict self, Dee_ssize_t start, Dee_ssize_t end);  /* May call other DEFAULT operators */ /* DEPRECATED */

/* tp_getrange_index_n */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultGetRangeIndexNWithGetRange(DeeObject *__restrict self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetRangeIndexNWithSizeAndGetItemIndexFast(DeeObject *__restrict self, Dee_ssize_t start); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetRangeIndexNWithSizeAndGetRangeIndex(DeeObject *__restrict self, Dee_ssize_t start); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndGetRangeIndex(DeeObject *__restrict self, Dee_ssize_t start); /* May call other DEFAULT operators */ /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndGetItemIndex(DeeObject *__restrict self, Dee_ssize_t start); /* May call other DEFAULT operators */ /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndGetItem(DeeObject *__restrict self, Dee_ssize_t start);      /* May call other DEFAULT operators */ /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndIter(DeeObject *__restrict self, Dee_ssize_t start);         /* May call other DEFAULT operators */ /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndIterDefault(DeeObject *__restrict self, Dee_ssize_t start);  /* May call other DEFAULT operators */ /* DEPRECATED */

/* tp_delrange */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_DefaultDelRangeWithDelRangeIndexAndDelRangeIndexN(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_DefaultDelRangeWithDelRangeIndexDefaultAndDelRangeIndexNDefault(DeeObject *self, DeeObject *start, DeeObject *end); /* May call other DEFAULT operators */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultDelRangeWithSetRangeNone(DeeObject *self, DeeObject *start, DeeObject *end); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultDelRangeWithSetRangeNoneDefault(DeeObject *self, DeeObject *start, DeeObject *end); /* May call other DEFAULT operators */ /* DEPRECATED */

/* tp_delrange_index */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultDelRangeIndexWithDelRange(DeeObject *__restrict self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelRangeIndexWithSetRangeIndexNone(DeeObject *__restrict self, Dee_ssize_t start, Dee_ssize_t end); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelRangeIndexWithSetRangeIndexNoneDefault(DeeObject *__restrict self, Dee_ssize_t start, Dee_ssize_t end); /* May call other DEFAULT operators */ /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelRangeIndexWithSizeDefaultAndTSCErase(DeeObject *__restrict self, Dee_ssize_t start, Dee_ssize_t end); /* May call other DEFAULT operators */ /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelRangeIndexWithSizeDefaultAndDelItemIndexDefault(DeeObject *__restrict self, Dee_ssize_t start, Dee_ssize_t end); /* May call other DEFAULT operators */ /* DEPRECATED */

/* tp_delrange_index_n */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeObject_DefaultDelRangeIndexNWithDelRange(DeeObject *__restrict self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelRangeIndexNWithSizeAndDelRangeIndex(DeeObject *__restrict self, Dee_ssize_t start); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelRangeIndexNWithSizeDefaultAndDelRangeIndex(DeeObject *__restrict self, Dee_ssize_t start); /* May call other DEFAULT operators */ /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelRangeIndexNWithSetRangeIndexNNone(DeeObject *__restrict self, Dee_ssize_t start); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelRangeIndexNWithSetRangeIndexNNoneDefault(DeeObject *__restrict self, Dee_ssize_t start); /* May call other DEFAULT operators */ /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelRangeIndexNWithSizeDefaultAndTSCErase(DeeObject *__restrict self, Dee_ssize_t start); /* May call other DEFAULT operators */ /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelRangeIndexNWithSizeDefaultAndDelItemIndexDefault(DeeObject *__restrict self, Dee_ssize_t start); /* May call other DEFAULT operators */ /* DEPRECATED */

/* tp_setrange */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL DeeObject_DefaultSetRangeWithSetRangeIndexAndSetRangeIndexN(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL DeeObject_DefaultSetRangeWithSetRangeIndexDefaultAndSetRangeIndexNDefault(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *value); /* May call other DEFAULT operators */

/* tp_setrange_index */
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeObject_DefaultSetRangeIndexWithSetRange(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultSetRangeIndexWithSizeDefaultAndTSCEraseAndTSCInsertAll(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *value); /* DEPRECATED */

/* tp_setrange_index_n */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeObject_DefaultSetRangeIndexNWithSetRange(DeeObject *self, Dee_ssize_t start, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultSetRangeIndexNWithSizeAndSetRangeIndex(DeeObject *self, Dee_ssize_t start, DeeObject *value); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultSetRangeIndexNWithSizeDefaultAndSetRangeIndex(DeeObject *self, Dee_ssize_t start, DeeObject *value); /* May call other DEFAULT operators */ /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultSetRangeIndexNWithSizeDefaultAndSetRangeIndexDefault(DeeObject *self, Dee_ssize_t start, DeeObject *value); /* May call other DEFAULT operators */ /* DEPRECATED */

/* tp_unpack / tp_unpack_ub */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultUnpackWithUnpackEx(DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultUnpackWithAsVector(DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultUnpackWithSizeAndGetItemIndexFast(DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultUnpackWithSizeAndTryGetItemIndex(DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultUnpackWithSizeAndGetItemIndex(DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultUnpackWithSizeDefaultAndTryGetItemIndexDefault(DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst); /* DEPRECATED */
#define DeeSeq_DefaultUnpackWithSizeDefaultAndTryGetItemIndexDefault DeeSeq_DefaultUnpackWithSizeAndTryGetItemIndex /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultUnpackWithSizeDefaultAndGetItemIndexDefault(DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst); /* DEPRECATED */
#define DeeSeq_DefaultUnpackWithSizeDefaultAndGetItemIndexDefault DeeSeq_DefaultUnpackWithSizeAndGetItemIndex /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultUnpackWithForeach(DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultUnpackWithIter(DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultUnpackWithForeachDefault(DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst); /* DEPRECATED */
#define DeeSeq_DefaultUnpackWithForeachDefault DeeSeq_DefaultUnpackWithForeach /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultUnpackUbWithSizeAndGetItemIndexFast(DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultUnpackUbWithSizeAndTryGetItemIndex(DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultUnpackUbWithSizeAndGetItemIndex(DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultUnpackUbWithSizeDefaultAndTryGetItemIndexDefault(DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst); /* DEPRECATED */
#define DeeSeq_DefaultUnpackUbWithSizeDefaultAndTryGetItemIndexDefault DeeSeq_DefaultUnpackUbWithSizeAndTryGetItemIndex /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultUnpackUbWithSizeDefaultAndGetItemIndexDefault(DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst); /* DEPRECATED */
#define DeeSeq_DefaultUnpackUbWithSizeDefaultAndGetItemIndexDefault DeeSeq_DefaultUnpackUbWithSizeAndGetItemIndex /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultUnpackUbWithSizeDefaultAndEnumerateIndex(DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultUnpackUbWithSizeDefaultAndEnumerateIndexDefault(DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst); /* DEPRECATED */
#define DeeSeq_DefaultUnpackUbWithSizeDefaultAndEnumerateIndexDefault DeeSeq_DefaultUnpackUbWithSizeDefaultAndEnumerateIndex /* DEPRECATED */
#if 0
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultUnpackUbWithUnpackEx(DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultUnpackUbWithAsVector(DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultUnpackUbWithForeach(DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultUnpackUbWithIter(DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultUnpackUbWithForeachDefault(DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst); /* DEPRECATED */
#endif
#define DeeSeq_DefaultUnpackUbWithUnpackEx        DeeSeq_DefaultUnpackWithUnpackEx /* DEPRECATED */
#define DeeSeq_TDefaultUnpackUbWithUnpackEx       DeeSeq_TDefaultUnpackWithUnpackEx /* DEPRECATED */
#define DeeSeq_DefaultUnpackUbWithAsVector        DeeSeq_DefaultUnpackWithAsVector /* DEPRECATED */
#define DeeSeq_TDefaultUnpackUbWithAsVector       DeeSeq_TDefaultUnpackWithAsVector /* DEPRECATED */
#define DeeSeq_DefaultUnpackUbWithForeach         DeeSeq_DefaultUnpackWithForeach /* DEPRECATED */
#define DeeSeq_TDefaultUnpackUbWithForeach        DeeSeq_TDefaultUnpackWithForeach /* DEPRECATED */
#define DeeSeq_DefaultUnpackUbWithIter            DeeSeq_DefaultUnpackWithIter /* DEPRECATED */
#define DeeSeq_TDefaultUnpackUbWithIter           DeeSeq_TDefaultUnpackWithIter /* DEPRECATED */
#define DeeSeq_DefaultUnpackUbWithForeachDefault  DeeSeq_DefaultUnpackWithForeachDefault /* DEPRECATED */
#define DeeSeq_TDefaultUnpackUbWithForeachDefault DeeSeq_TDefaultUnpackWithForeachDefault /* DEPRECATED */

/* tp_unpack_ex */
INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeSeq_DefaultUnpackExWithAsVector(DeeObject *self, size_t dst_length_min, size_t dst_length_max, /*out*/ DREF DeeObject **dst); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeSeq_DefaultUnpackExWithSizeAndGetItemIndexFast(DeeObject *self, size_t dst_length_min, size_t dst_length_max, /*out*/ DREF DeeObject **dst); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeSeq_DefaultUnpackExWithSizeAndTryGetItemIndex(DeeObject *self, size_t dst_length_min, size_t dst_length_max, /*out*/ DREF DeeObject **dst); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeSeq_DefaultUnpackExWithSizeAndGetItemIndex(DeeObject *self, size_t dst_length_min, size_t dst_length_max, /*out*/ DREF DeeObject **dst); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeSeq_DefaultUnpackExWithSizeDefaultAndTryGetItemIndexDefault(DeeObject *self, size_t dst_length_min, size_t dst_length_max, /*out*/ DREF DeeObject **dst); /* DEPRECATED */
#define DeeSeq_DefaultUnpackExWithSizeDefaultAndTryGetItemIndexDefault DeeSeq_DefaultUnpackExWithSizeAndTryGetItemIndex /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeSeq_DefaultUnpackExWithSizeDefaultAndGetItemIndexDefault(DeeObject *self, size_t dst_length_min, size_t dst_length_max, /*out*/ DREF DeeObject **dst); /* DEPRECATED */
#define DeeSeq_DefaultUnpackExWithSizeDefaultAndGetItemIndexDefault DeeSeq_DefaultUnpackExWithSizeAndGetItemIndex /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeSeq_DefaultUnpackExWithForeach(DeeObject *self, size_t dst_length_min, size_t dst_length_max, /*out*/ DREF DeeObject **dst); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeSeq_DefaultUnpackExWithIter(DeeObject *self, size_t dst_length_min, size_t dst_length_max, /*out*/ DREF DeeObject **dst); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeSeq_DefaultUnpackExWithForeachDefault(DeeObject *self, size_t dst_length_min, size_t dst_length_max, /*out*/ DREF DeeObject **dst); /* DEPRECATED */
#define DeeSeq_DefaultUnpackExWithForeachDefault DeeSeq_DefaultUnpackExWithForeach /* DEPRECATED */

/*[[[end:DEFAULT_OPERATORS]]]*/

/* Extra map functions that are needed for implementing generic map operator. */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultContainsWithForeachPair(DeeObject *self, DeeObject *elem); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeMap_DefaultEnumerateIndexWithForeachPair(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultGetItemWithForeachPair(DeeObject *__restrict self, DeeObject *key); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultGetItemIndexWithForeachPair(DeeObject *__restrict self, size_t key); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultGetItemStringHashWithForeachPair(DeeObject *__restrict self, char const *key, Dee_hash_t hash); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultGetItemStringLenHashWithForeachPair(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultTryGetItemWithForeachPair(DeeObject *__restrict self, DeeObject *key); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultTryGetItemIndexWithForeachPair(DeeObject *__restrict self, size_t key); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultTryGetItemStringHashWithForeachPair(DeeObject *__restrict self, char const *key, Dee_hash_t hash); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultTryGetItemStringLenHashWithForeachPair(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultBoundItemWithForeachPair(DeeObject *self, DeeObject *key); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeMap_DefaultBoundItemIndexWithForeachPair(DeeObject *__restrict self, size_t key); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultBoundItemStringHashWithForeachPair(DeeObject *__restrict self, char const *key, Dee_hash_t hash); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultBoundItemStringLenHashWithForeachPair(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultHasItemWithForeachPair(DeeObject *self, DeeObject *key); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeMap_DefaultHasItemIndexWithForeachPair(DeeObject *__restrict self, size_t key); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultHasItemStringHashWithForeachPair(DeeObject *__restrict self, char const *key, Dee_hash_t hash); /* DEPRECATED */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultHasItemStringLenHashWithForeachPair(DeeObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash); /* DEPRECATED */
#endif /* !CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */


#ifndef CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS
/* clang-format off */
/*[[[deemon
import * from deemon;
import util;
local fileData = File.open(__FILE__).read()
	.partition("/" "*[[[begin:DEFAULT_OPERATORS]]]*" "/").last
	.partition("/" "*[[[end:DEFAULT_OPERATORS]]]*" "/").first
	.strip()
	.decode("utf-8");

class Decl {
	this = default;
	public member beforeName: string;
	public member name: string;
	public member afterName: string;
	public member ppCond: string;
};

local decls: {Decl...} = [];
local linesWithDecls: {string | Decl...} = [];
local ppCond = [];
for (local line: fileData.splitlines(false)) {
	local a, b, c;
	if (line.startswith("#")) {
		line = line.lstrip("#").strip();
		local cond;
		if (line.startswith("if ")) {
			cond = f"{line[3:].lstrip()}";
		} else if (line.startswith("ifdef ")) {
			cond = f"defined({line[6:].lstrip()})";
		} else if (line.startswith("ifndef ")) {
			cond = f"!defined({line[7:].lstrip()})";
		} else if (line.startswith("else")) {
			cond = f"!{ppCond.pop()}";
		} else if (line.startswith("endif")) {
			cond = none;
			ppCond.pop();
		} else if (line.startswith("elif ")) {
			cond = f"!({ppCond.pop()}) && ({line[5:].lstrip()})";
		} else {
			continue;
		}
		if (cond == "0")
			cond = "";
		if (cond !is none)
			ppCond.append(cond);
		linesWithDecls.append(f"#{line}");
		continue;
	}
	try {
		a, b, c = line.rescanf(r"(INTDEF.*DCALL\s+)([^(]+)(\(.*)")...;
	} catch (...) {
		continue;
	}
	c = c.strip();
	while (c.endswith("*" "/"))
		c = c.rpartition("/" "*").first.rstrip();
	local item = Decl(beforeName: a, name: b, afterName: c,
		ppCond: " && ".join(ppCond.filter(e -> e)) ?: "1");
	decls.append(item);
	linesWithDecls.append(item);
}
local declsByName: {string: Decl} = Dict(decls.map(e -> (e.name, e)));
for (local item: linesWithDecls) {
	if (item is string) {
		print(item);
		continue;
	}
	local nna, nnb, nnc;
	try {
		nna, nnb, nnc = item.beforeName.rescanf(r"(.*NONNULL\(\()([^)]+)(\)\).*)")...;
	} catch (...) {
		nna = item.beforeName;
		nnb = nnc = "";
	}
	if (nnb)
		nnb = "1, " + ", ".join(for (local x: nnb.split(",")) int(x.strip()) + 1);
	local nameA, none, nameB = item.name.partition("_")...;
	local c = item.afterName[1:].replace("__restrict ", "");
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
function hasTypedVariant(name: string): bool {
	return "WithError" !in name;
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
	result = result.replace("try_", "try");
	result = result.replace("iter_keys", "iterkeys");
	result = result.replace("iter_next_key", "nextkey");
	result = result.replace("iter_next_value", "nextvalue");
	result = result.replace("iter_next_pair", "nextpair");
	result = result.replace("iter_advance", "advance");
	return result;
}

for (local decl: decls) {
	local name = getGroupName(decl.name);
	local group = groups.get(name);
	if (group is none) {
		groupNames.append(name);
		groups[name] = group = [];
	}
	group.append(decl.name);
}
for (local name: groupNames) {
	local mems = Tuple(for (local x: groups[name]) if (hasTypedVariant(x)) x);
	local tp = getTpName(name);
	local ppConds = {"1"};
	//local ppConds = mems.map(e -> declsByName[e].ppCond).distinct() ?: {"1"};
	//print File.stderr: type ppConds, repr ppConds; // TODO
	//if (ppConds != {"1"})
	//	ppConds = (for (local e: ppConds) { e, e.startswith("1") ? e[1:] : f"!({e})" }).flatten;
	//print File.stderr: repr ppConds;
	for (local ppCond: ppConds) {
		if (ppCond != "1")
			print("#if ", ppCond);
		local usedMems = mems; // TODO
		//local usedMems = mems.filter(e -> declsByName[e].ppCond in {"1", ppCond}).cached;
		print("#define DeeType_MapDefault", name, "(", tp, ", map, default) "),;
		if (!usedMems) {
			print("default");
		} else {
			print("\\");
			for (local i, mem: usedMems.enumerate()) {
				print("	", i == 0 ? "(" : " "),;
				print("(", tp, ") == &", mem, " ? map(", mem.replace("_", "_T"), ") : "),;
				if (i == #usedMems - 1) {
					print("default)");
				} else {
					print("\\");
				}
			}
		}
		if (ppCond != "1")
			print("#endif /" "* ... *" "/");
	}
}
for (local name: groupNames) {
	local mems = groups[name];
	local tp = getTpName(name);
	print("#define DeeType_IsDefault", name, "(", tp, ") \\");
	for (local i, mem: mems.enumerate()) {
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
		"Bool" : "cast",
		"Eq" : "cmp",
		"Ne" : "cmp",
		"Lo" : "cmp",
		"Le" : "cmp",
		"Gr" : "cmp",
		"Ge" : "cmp",
		"Hash" : "cmp",
		"Compare" : "cmp",
		"CompareEq" : "cmp",
		"TryCompareEq" : "cmp",
		"Iter" : "seq",
		"Size" : "seq",
		"SizeFast" : "seq",
		"SizeOb" : "seq",
		"Foreach" : "seq",
		"ForeachPair" : "seq",
		"Enumerate" : "seq",
		"EnumerateIndex" : "seq",
		"IterKeys" : "seq",
		"Contains" : "seq",
		"Contains" : "seq",
		"Unpack" : "seq",
		"UnpackEx" : "seq",
		"UnpackUb" : "seq",
		"IterNext" : "",
		"IterNextPair" : "iterator",
		"IterNextKey" : "iterator",
		"IterNextValue" : "iterator",
		"IterAdvance" : "iterator",
	}.get(name, "math");
}
for (local name: groupNames) {
	local mems = groups[name];
	local tp = getTpName(name);
	local package = getOperatorPackage(name);
	if (package)
		package += "_";
	local args = decls.locate(e -> e.name == mems.first).afterName.rstrip(");");
	local args = ", ".join(for (local a: args.split(",")) a.strip().rerpartition(r"[* ]").last.partition("[").first);
	local mems = Tuple(for (local x: mems) if (hasTypedVariant(x)) x);
	print("#define DeeType_invoke_", package, tp, "_DEFAULT(tp_self, ", tp, ", ", args, ", default) "),;
	if (!mems) {
		print("default");
		//print("(*(", tp, "))(", args, ")");
	} else {
		print("\\");
		for (local i, mem: mems.enumerate()) {
			print("	", i == 0 ? "(" : " "),;
			print("(", tp, ") == &", mem, " ? ", mem.replace("_", "_T"), "(tp_self, ", args, ") : "),;
			print("\\");
		}
		print("	 default)");
		//print("	 (*(", tp, "))(", args, "))");
	}
}
for (local name: groupNames) {
	local mems = groups[name];
	local tp = getTpName(name);
	local package = getOperatorPackage(name);
	if (package)
		package += "_";
	local args = decls.locate(e -> e.name == mems.first).afterName.rstrip(");");
	local args = ", ".join(for (local a: args.split(",")) a.strip().rerpartition(r"[* ]").last.partition("[").first);
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
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultBoolWithSize(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultBoolWithSizeOb(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultBoolWithForeach(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultBoolWithCompareEq(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultBoolWithEq(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultBoolWithNe(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultBoolWithForeachDefault(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) Dee_hash_t DCALL DeeSeq_TDefaultHashWithSizeAndGetItemIndexFast(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) Dee_hash_t DCALL DeeSeq_TDefaultHashWithForeach(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) Dee_hash_t DCALL DeeSeq_TDefaultHashWithSizeAndTryGetItemIndex(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) Dee_hash_t DCALL DeeSeq_TDefaultHashWithSizeAndGetItemIndex(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) Dee_hash_t DCALL DeeSeq_TDefaultHashWithSizeObAndGetItem(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) Dee_hash_t DCALL DeeSeq_TDefaultHashWithForeachDefault(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) Dee_hash_t DCALL DeeSet_TDefaultHashWithForeachDefault(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) Dee_hash_t DCALL DeeMap_TDefaultHashWithForeachPairDefault(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultEqWithCompareEq(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultEqWithNe(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultEqWithLoAndGr(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultEqWithLeAndGe(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultEqWithCompareEqDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultNeWithCompareEq(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultNeWithEq(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultNeWithLoAndGr(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultNeWithLeAndGe(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultNeWithCompareEqDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultLoWithCompare(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultLoWithGe(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultLoWithCompareDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSet_TDefaultLoWithForeachDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_TDefaultLoWithForeachPairDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultLeWithCompare(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultLeWithGr(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultLeWithCompareDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSet_TDefaultLeWithForeachDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_TDefaultLeWithForeachPairDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultGrWithCompare(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultGrWithLe(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultGrWithCompareDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSet_TDefaultGrWithForeachDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_TDefaultGrWithForeachPairDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultGeWithCompare(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultGeWithLo(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultGeWithCompareDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSet_TDefaultGeWithForeachDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_TDefaultGeWithForeachPairDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultCompareEqWithEq(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultCompareEqWithNe(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultCompareEqWithLoAndGr(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultCompareEqWithLeAndGe(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_TDefaultCompareEqWithForeachDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_TDefaultCompareEqWithSizeAndGetItemIndexFast(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_TDefaultCompareEqWithSizeAndTryGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_TDefaultCompareEqWithSizeAndGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_TDefaultCompareEqWithSizeObAndGetItem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSet_TDefaultCompareEqWithForeachDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_TDefaultCompareEqWithForeachPairDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultTryCompareEqWithCompareEq(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultTryCompareEqWithEq(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultTryCompareEqWithNe(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultTryCompareEqWithCompare(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultTryCompareEqWithLoAndGr(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultTryCompareEqWithLeAndGe(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
#if 0
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_TDefaultTryCompareEqWithForeachDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_TDefaultTryCompareEqWithSizeAndGetItemIndexFast(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_TDefaultTryCompareEqWithSizeAndTryGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_TDefaultTryCompareEqWithSizeAndGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_TDefaultTryCompareEqWithSizeObAndGetItem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSet_TDefaultTryCompareEqWithForeachDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_TDefaultTryCompareEqWithForeachPairDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
#endif
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultCompareWithEqAndLo(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultCompareWithEqAndLe(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultCompareWithEqAndGr(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultCompareWithEqAndGe(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultCompareWithNeAndLo(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultCompareWithNeAndLe(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultCompareWithNeAndGr(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultCompareWithNeAndGe(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
#if 0
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultCompareWithLoAndGr(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultCompareWithLeAndGe(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
#endif
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_TDefaultCompareWithSizeAndGetItemIndexFast(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_TDefaultCompareWithSizeAndTryGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_TDefaultCompareWithSizeAndGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_TDefaultCompareWithSizeObAndGetItem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_TDefaultCompareWithForeachDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultIterNextWithIterNextPair(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultIterNextPairWithIterNext(DeeTypeObject *tp_self, DeeObject *self, /*out*/ DREF DeeObject *key_and_value[2]);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultIterNextKeyWithIterNext(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultIterNextKeyWithIterNextPair(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultIterNextValueWithIterNext(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultIterNextValueWithIterNextPair(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeObject_TDefaultIterAdvanceWithIterNext(DeeTypeObject *tp_self, DeeObject *self, size_t step);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeObject_TDefaultIterAdvanceWithIterNextPair(DeeTypeObject *tp_self, DeeObject *self, size_t step);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeObject_TDefaultIterAdvanceWithIterNextKey(DeeTypeObject *tp_self, DeeObject *self, size_t step);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeObject_TDefaultIterAdvanceWithIterNextValue(DeeTypeObject *tp_self, DeeObject *self, size_t step);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultIterWithForeach(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultIterWithForeachPair(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultIterWithEnumerate(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultIterWithEnumerateIndex(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultIterWithIterKeysAndTryGetItem(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultIterWithIterKeysAndGetItem(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultIterWithIterKeysAndTryGetItemDefault(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultIterWithSizeAndGetItemIndexFast(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultIterWithSizeAndTryGetItemIndex(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultIterWithSizeAndGetItemIndex(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultIterWithGetItemIndex(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultIterWithSizeObAndGetItem(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultIterWithGetItem(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_TDefaultIterWithEnumerate(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_TDefaultIterWithEnumerateIndex(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_TDefaultIterWithIterKeysAndTryGetItem(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_TDefaultIterWithIterKeysAndGetItem(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_TDefaultIterWithIterKeysAndTryGetItemDefault(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeObject_TDefaultForeachWithIter(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeObject_TDefaultForeachWithEnumerate(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeObject_TDefaultForeachWithEnumerateIndex(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeObject_TDefaultForeachWithForeachPair(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeObject_TDefaultForeachWithForeachPairDefault(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeObject_TDefaultForeachWithIterKeysAndTryGetItem(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeObject_TDefaultForeachWithIterKeysAndGetItem(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeObject_TDefaultForeachWithIterKeysAndTryGetItemDefault(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeSeq_TDefaultForeachWithSizeAndGetItemIndexFast(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeSeq_TDefaultForeachWithSizeAndTryGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeSeq_TDefaultForeachWithSizeAndGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeSeq_TDefaultForeachWithSizeObAndGetItem(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeSeq_TDefaultForeachWithSizeDefaultAndGetItemIndexDefault(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeSeq_TDefaultForeachWithGetItemIndexDefault(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeObject_TDefaultForeachPairWithForeach(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_pair_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeObject_TDefaultForeachPairWithEnumerate(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_pair_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeObject_TDefaultForeachPairWithEnumerateIndex(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_pair_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeObject_TDefaultForeachPairWithForeachDefault(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_pair_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeObject_TDefaultForeachPairWithIter(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_pair_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeMap_TDefaultForeachPairWithEnumerate(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_pair_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeMap_TDefaultForeachPairWithEnumerateIndex(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_pair_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeMap_TDefaultForeachPairWithEnumerateDefault(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_pair_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeObject_TDefaultEnumerateWithEnumerateIndex(DeeTypeObject *tp_self, DeeObject *self, Dee_seq_enumerate_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeObject_TDefaultEnumerateWithIterKeysAndTryGetItem(DeeTypeObject *tp_self, DeeObject *self, Dee_seq_enumerate_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeObject_TDefaultEnumerateWithIterKeysAndGetItem(DeeTypeObject *tp_self, DeeObject *self, Dee_seq_enumerate_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeObject_TDefaultEnumerateWithIterKeysAndTryGetItemDefault(DeeTypeObject *tp_self, DeeObject *self, Dee_seq_enumerate_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeSeq_TDefaultEnumerateWithSizeAndGetItemIndexFast(DeeTypeObject *tp_self, DeeObject *self, Dee_seq_enumerate_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeSeq_TDefaultEnumerateWithSizeAndTryGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, Dee_seq_enumerate_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeSeq_TDefaultEnumerateWithSizeAndGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, Dee_seq_enumerate_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeSeq_TDefaultEnumerateWithSizeObAndGetItem(DeeTypeObject *tp_self, DeeObject *self, Dee_seq_enumerate_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeSeq_TDefaultEnumerateWithCounterAndForeach(DeeTypeObject *tp_self, DeeObject *self, Dee_seq_enumerate_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeSeq_TDefaultEnumerateWithCounterAndIter(DeeTypeObject *tp_self, DeeObject *self, Dee_seq_enumerate_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeMap_TDefaultEnumerateWithForeachPairDefault(DeeTypeObject *tp_self, DeeObject *self, Dee_seq_enumerate_t proc, void *arg);
#if 0
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeMap_TDefaultEnumerateWithIter(DeeTypeObject *tp_self, DeeObject *self, Dee_seq_enumerate_t proc, void *arg);
#endif
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeSeq_TDefaultEnumerateWithSizeDefaultAndGetItemIndexDefault(DeeTypeObject *tp_self, DeeObject *self, Dee_seq_enumerate_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeSeq_TDefaultEnumerateWithCounterAndForeachDefault(DeeTypeObject *tp_self, DeeObject *self, Dee_seq_enumerate_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeObject_TDefaultEnumerateIndexWithEnumerate(DeeTypeObject *tp_self, DeeObject *self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeSeq_TDefaultEnumerateIndexWithSizeAndGetItemIndexFast(DeeTypeObject *tp_self, DeeObject *self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeSeq_TDefaultEnumerateIndexWithSizeAndTryGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeSeq_TDefaultEnumerateIndexWithSizeAndGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeSeq_TDefaultEnumerateIndexWithSizeObAndGetItem(DeeTypeObject *tp_self, DeeObject *self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeSeq_TDefaultEnumerateIndexWithCounterAndForeach(DeeTypeObject *tp_self, DeeObject *self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeSeq_TDefaultEnumerateIndexWithCounterAndIter(DeeTypeObject *tp_self, DeeObject *self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeSeq_TDefaultEnumerateIndexWithSizeDefaultAndGetItemIndexDefault(DeeTypeObject *tp_self, DeeObject *self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeSeq_TDefaultEnumerateIndexWithCounterAndForeachDefault(DeeTypeObject *tp_self, DeeObject *self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL DeeObject_TDefaultEnumerateIndexWithEnumerateDefault(DeeTypeObject *tp_self, DeeObject *self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultIterKeysWithEnumerate(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultIterKeysWithEnumerateIndex(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultIterKeysWithSize(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultIterKeysWithSizeOb(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultIterKeysWithSizeDefault(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_TDefaultIterKeysWithIter(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_TDefaultIterKeysWithIterDefault(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultSizeObWithSize(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultSizeObWithSizeDefault(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeObject_TDefaultSizeWithSizeOb(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_TDefaultSizeWithEnumerateIndex(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_TDefaultSizeWithEnumerate(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_TDefaultSizeWithForeachPair(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_TDefaultSizeWithForeach(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_TDefaultSizeWithIter(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeObject_TDefaultSizeFastWithErrorNotFast(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_TDefaultContainsWithForeachDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *elem);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_TDefaultContainsWithHasItem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *elem);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_TDefaultContainsWithBoundItem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *elem);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_TDefaultContainsWithTryGetItem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *elem);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_TDefaultContainsWithGetItem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *elem);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_TDefaultContainsWithHasItemStringHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *elem);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_TDefaultContainsWithHasItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *elem);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_TDefaultContainsWithHasItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *elem);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_TDefaultContainsWithBoundItemStringHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *elem);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_TDefaultContainsWithBoundItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *elem);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_TDefaultContainsWithBoundItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *elem);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_TDefaultContainsWithTryGetItemStringHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *elem);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_TDefaultContainsWithTryGetItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *elem);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_TDefaultContainsWithTryGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *elem);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_TDefaultContainsWithGetItemStringHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *elem);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_TDefaultContainsWithGetItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *elem);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_TDefaultContainsWithGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *elem);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_TDefaultContainsWithEnumerate(DeeTypeObject *tp_self, DeeObject *self, DeeObject *elem);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_TDefaultContainsWithEnumerateDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *elem);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultGetItemWithSizeAndGetItemIndexFast(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultGetItemWithGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultGetItemWithGetItemStringHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultGetItemWithGetItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultGetItemWithTryGetItem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultGetItemWithTryGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultGetItemWithTryGetItemStringHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultGetItemWithTryGetItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultGetItemWithGetItemIndexDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_TDefaultGetItemWithTryGetItemAndSizeOb(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_TDefaultGetItemWithTryGetItemAndSize(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_TDefaultGetItemWithSizeAndTryGetItemIndexOb(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_TDefaultGetItemWithSizeAndTryGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_TDefaultGetItemWithEnumerate(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_TDefaultGetItemWithEnumerateDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultGetItemIndexWithSizeAndGetItemIndexFast(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultGetItemIndexWithTryGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultGetItemIndexWithGetItem(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultGetItemIndexWithTryGetItem(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultGetItemIndexWithErrorRequiresString(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultGetItemIndexWithGetItemDefault(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultGetItemIndexWithSizeAndTryGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultGetItemIndexWithSizeAndTryGetItemIndexOb(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultGetItemIndexWithTryGetItemAndSize(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultGetItemIndexWithTryGetItemAndSizeOb(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultGetItemIndexWithForeachDefault(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_TDefaultGetItemIndexWithEnumerate(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_TDefaultGetItemIndexWithEnumerateDefault(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultGetItemStringHashWithGetItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultGetItemStringHashWithGetItem(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultGetItemStringHashWithTryGetItemStringHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultGetItemStringHashWithTryGetItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultGetItemStringHashWithTryGetItem(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultGetItemStringHashWithErrorRequiresInt(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultGetItemStringHashWithGetItemDefault(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_TDefaultGetItemStringHashWithEnumerate(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_TDefaultGetItemStringHashWithEnumerateDefault(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultGetItemStringLenHashWithGetItemStringHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultGetItemStringLenHashWithGetItem(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultGetItemStringLenHashWithTryGetItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultGetItemStringLenHashWithTryGetItemStringHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultGetItemStringLenHashWithTryGetItem(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultGetItemStringLenHashWithErrorRequiresInt(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultGetItemStringLenHashWithGetItemDefault(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_TDefaultGetItemStringLenHashWithEnumerate(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_TDefaultGetItemStringLenHashWithEnumerateDefault(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultTryGetItemWithTryGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultTryGetItemWithTryGetItemStringHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultTryGetItemWithTryGetItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultTryGetItemWithSizeAndGetItemIndexFast(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultTryGetItemWithGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultTryGetItemWithGetItem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultTryGetItemWithGetItemStringHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultTryGetItemWithGetItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultTryGetItemWithGetItemDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_TDefaultTryGetItemWithEnumerate(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_TDefaultTryGetItemWithEnumerateDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultTryGetItemIndexWithSizeAndGetItemIndexFast(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultTryGetItemIndexWithTryGetItem(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultTryGetItemIndexWithGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultTryGetItemIndexWithGetItem(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultTryGetItemIndexWithErrorRequiresString(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultTryGetItemIndexWithGetItemIndexDefault(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultTryGetItemIndexWithForeachDefault(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_TDefaultTryGetItemIndexWithEnumerate(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_TDefaultTryGetItemIndexWithEnumerateDefault(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultTryGetItemStringHashWithTryGetItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultTryGetItemStringHashWithTryGetItem(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultTryGetItemStringHashWithGetItemStringHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultTryGetItemStringHashWithGetItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultTryGetItemStringHashWithGetItem(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultTryGetItemStringHashWithErrorRequiresInt(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultTryGetItemStringHashWithTryGetItemDefault(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_TDefaultTryGetItemStringHashWithEnumerate(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_TDefaultTryGetItemStringHashWithEnumerateDefault(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultTryGetItemStringLenHashWithTryGetItemStringHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultTryGetItemStringLenHashWithTryGetItem(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultTryGetItemStringLenHashWithGetItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultTryGetItemStringLenHashWithGetItemStringHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultTryGetItemStringLenHashWithGetItem(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultTryGetItemStringLenHashWithErrorRequiresInt(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeObject_TDefaultTryGetItemStringLenHashWithTryGetItemDefault(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_TDefaultTryGetItemStringLenHashWithEnumerate(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_TDefaultTryGetItemStringLenHashWithEnumerateDefault(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultDelItemWithDelItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultDelItemWithDelItemIndexDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultDelItemWithDelItemStringHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultDelItemWithDelItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultDelItemIndexWithDelItem(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultDelItemIndexWithErrorRequiresString(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultDelItemIndexWithDelRangeIndexDefault(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultDelItemStringHashWithDelItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultDelItemStringHashWithDelItem(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultDelItemStringHashWithErrorRequiresInt(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultDelItemStringLenHashWithDelItemStringHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultDelItemStringLenHashWithDelItem(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultDelItemStringLenHashWithErrorRequiresInt(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL DeeObject_TDefaultSetItemWithSetItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL DeeObject_TDefaultSetItemWithSetItemIndexDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL DeeObject_TDefaultSetItemWithSetItemStringHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL DeeObject_TDefaultSetItemWithSetItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL DeeObject_TDefaultSetItemIndexWithSetItem(DeeTypeObject *tp_self, DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL DeeObject_TDefaultSetItemIndexWithErrorRequiresString(DeeTypeObject *tp_self, DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL DeeSeq_TDefaultSetItemIndexWithSetRangeIndexDefault(DeeTypeObject *tp_self, DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 5)) int DCALL DeeObject_TDefaultSetItemStringHashWithSetItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 5)) int DCALL DeeObject_TDefaultSetItemStringHashWithSetItem(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 5)) int DCALL DeeObject_TDefaultSetItemStringHashWithErrorRequiresInt(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 6)) int DCALL DeeObject_TDefaultSetItemStringLenHashWithSetItemStringHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 6)) int DCALL DeeObject_TDefaultSetItemStringLenHashWithSetItem(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 6)) int DCALL DeeObject_TDefaultSetItemStringLenHashWithErrorRequiresInt(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemWithBoundItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemWithBoundItemStringHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemWithBoundItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemWithGetItem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemWithGetItemStringHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemWithGetItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemWithGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemWithTryGetItemAndHasItem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemWithTryGetItemIndexAndHasItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemWithTryGetItemStringLenHashAndHasItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemWithTryGetItemStringHashAndHasItemStringHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_TDefaultBoundItemWithTryGetItemAndSizeOb(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_TDefaultBoundItemWithSizeAndTryGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemWithTryGetItem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemWithTryGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemWithTryGetItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemWithTryGetItemStringHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemWithGetItemDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_TDefaultBoundItemWithContains(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_TDefaultBoundItemWithEnumerate(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_TDefaultBoundItemWithEnumerateDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultBoundItemIndexWithBoundItem(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultBoundItemIndexWithSizeAndGetItemIndexFast(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultBoundItemIndexWithGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultBoundItemIndexWithGetItem(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultBoundItemIndexWithGetItemIndexDefault(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultBoundItemIndexWithTryGetItemIndexAndHasItemIndex(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultBoundItemIndexWithTryGetItemAndHasItem(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultBoundItemIndexWithSizeAndTryGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultBoundItemIndexWithTryGetItemAndSizeOb(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultBoundItemIndexWithErrorRequiresString(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_TDefaultBoundItemIndexWithContains(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_TDefaultBoundItemIndexWithEnumerate(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_TDefaultBoundItemIndexWithEnumerateDefault(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemStringHashWithBoundItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemStringHashWithBoundItem(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemStringHashWithGetItemStringHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemStringHashWithGetItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemStringHashWithGetItem(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemStringHashWithTryGetItemStringHashAndHasItemStringHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemStringHashWithTryGetItemStringLenHashAndHasItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemStringHashWithTryGetItemAndHasItem(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemStringHashWithTryGetItemStringHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemStringHashWithTryGetItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemStringHashWithTryGetItem(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemStringHashWithBoundItemDefault(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemStringHashWithErrorRequiresInt(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_TDefaultBoundItemStringHashWithContains(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_TDefaultBoundItemStringHashWithEnumerate(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_TDefaultBoundItemStringHashWithEnumerateDefault(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemStringLenHashWithBoundItemStringHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemStringLenHashWithBoundItem(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemStringLenHashWithGetItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemStringLenHashWithGetItemStringHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemStringLenHashWithGetItem(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemStringLenHashWithTryGetItemStringLenHashAndHasItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemStringLenHashWithTryGetItemStringHashAndHasItemStringHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemStringLenHashWithTryGetItemAndHasItem(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemStringLenHashWithTryGetItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemStringLenHashWithTryGetItemStringHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemStringLenHashWithTryGetItem(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemStringLenHashWithBoundItemDefault(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultBoundItemStringLenHashWithErrorRequiresInt(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_TDefaultBoundItemStringLenHashWithContains(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_TDefaultBoundItemStringLenHashWithEnumerate(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_TDefaultBoundItemStringLenHashWithEnumerateDefault(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemWithHasItemStringHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemWithHasItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemWithHasItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
#ifndef Dee_BOUND_MAYALIAS_HAS
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemWithBoundItem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemWithBoundItemStringHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemWithBoundItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemWithBoundItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
#endif /* !Dee_BOUND_MAYALIAS_HAS */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemWithTryGetItem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemWithTryGetItemStringHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemWithTryGetItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemWithTryGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemWithGetItem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemWithGetItemStringHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemWithGetItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemWithGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemWithGetItemDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_TDefaultHasItemWithSize(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_TDefaultHasItemWithSizeOb(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_TDefaultHasItemWithContains(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_TDefaultHasItemWithEnumerate(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_TDefaultHasItemWithEnumerateDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultHasItemIndexWithHasItem(DeeTypeObject *tp_self, DeeObject *self, size_t index);
#ifndef Dee_BOUND_MAYALIAS_HAS
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultHasItemIndexWithBoundItemIndex(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultHasItemIndexWithBoundItem(DeeTypeObject *tp_self, DeeObject *self, size_t index);
#endif /* !Dee_BOUND_MAYALIAS_HAS */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultHasItemIndexWithTryGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultHasItemIndexWithTryGetItem(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultHasItemIndexWithGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultHasItemIndexWithGetItem(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultHasItemIndexWithGetItemIndexDefault(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultHasItemIndexWithErrorRequiresString(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultHasItemIndexWithSize(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultHasItemIndexWithSizeDefault(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_TDefaultHasItemIndexWithContains(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_TDefaultHasItemIndexWithEnumerate(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_TDefaultHasItemIndexWithEnumerateDefault(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemStringHashWithHasItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
#ifndef Dee_BOUND_MAYALIAS_HAS
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemStringHashWithBoundItemStringHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemStringHashWithBoundItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
#endif /* !Dee_BOUND_MAYALIAS_HAS */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemStringHashWithTryGetItemStringHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemStringHashWithTryGetItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemStringHashWithGetItemStringHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemStringHashWithGetItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemStringHashWithHasItem(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
#ifndef Dee_BOUND_MAYALIAS_HAS
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemStringHashWithBoundItem(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
#endif /* !Dee_BOUND_MAYALIAS_HAS */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemStringHashWithTryGetItem(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemStringHashWithGetItem(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemStringHashWithHasItemDefault(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemStringHashWithErrorRequiresInt(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_TDefaultHasItemStringHashWithContains(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_TDefaultHasItemStringHashWithEnumerate(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_TDefaultHasItemStringHashWithEnumerateDefault(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemStringLenHashWithHasItemStringHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
#ifndef Dee_BOUND_MAYALIAS_HAS
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemStringLenHashWithBoundItemStringHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemStringLenHashWithBoundItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
#endif /* !Dee_BOUND_MAYALIAS_HAS */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemStringLenHashWithTryGetItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemStringLenHashWithTryGetItemStringHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemStringLenHashWithGetItemStringLenHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemStringLenHashWithGetItemStringHash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemStringLenHashWithHasItem(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
#ifndef Dee_BOUND_MAYALIAS_HAS
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemStringLenHashWithBoundItem(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
#endif /* !Dee_BOUND_MAYALIAS_HAS */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemStringLenHashWithTryGetItem(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemStringLenHashWithGetItem(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemStringLenHashWithHasItemDefault(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeObject_TDefaultHasItemStringLenHashWithErrorRequiresInt(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_TDefaultHasItemStringLenHashWithContains(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_TDefaultHasItemStringLenHashWithEnumerate(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_TDefaultHasItemStringLenHashWithEnumerateDefault(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL DeeObject_TDefaultGetRangeWithGetRangeIndexAndGetRangeIndexN(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL DeeObject_TDefaultGetRangeWithGetRangeIndexDefaultAndGetRangeIndexNDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL DeeSeq_TDefaultGetRangeWithSizeDefaultAndGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL DeeSeq_TDefaultGetRangeWithSizeDefaultAndTryGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL DeeSeq_TDefaultGetRangeWithSizeObAndGetItem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultGetRangeIndexWithGetRange(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultGetRangeIndexWithSizeAndGetItemIndexFast(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultGetRangeIndexWithSizeDefaultAndGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultGetRangeIndexWithSizeDefaultAndGetItem(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultGetRangeIndexWithSizeDefaultAndIter(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultGetRangeIndexWithSizeDefaultAndIterDefault(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeObject_TDefaultGetRangeIndexNWithGetRange(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultGetRangeIndexNWithSizeAndGetRangeIndex(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultGetRangeIndexNWithSizeDefaultAndGetRangeIndex(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultGetRangeIndexNWithSizeAndGetItemIndexFast(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultGetRangeIndexNWithSizeDefaultAndGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultGetRangeIndexNWithSizeDefaultAndGetItem(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultGetRangeIndexNWithSizeDefaultAndIter(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_TDefaultGetRangeIndexNWithSizeDefaultAndIterDefault(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL DeeObject_TDefaultDelRangeWithDelRangeIndexAndDelRangeIndexN(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL DeeObject_TDefaultDelRangeWithDelRangeIndexDefaultAndDelRangeIndexNDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL DeeSeq_TDefaultDelRangeWithSetRangeNone(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL DeeSeq_TDefaultDelRangeWithSetRangeNoneDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultDelRangeIndexWithDelRange(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultDelRangeIndexWithSetRangeIndexNone(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultDelRangeIndexWithSetRangeIndexNoneDefault(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultDelRangeIndexWithSizeDefaultAndTSCErase(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultDelRangeIndexWithSizeDefaultAndDelItemIndexDefault(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeObject_TDefaultDelRangeIndexNWithDelRange(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultDelRangeIndexNWithSizeAndDelRangeIndex(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultDelRangeIndexNWithSizeDefaultAndDelRangeIndex(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultDelRangeIndexNWithSetRangeIndexNNone(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultDelRangeIndexNWithSetRangeIndexNNoneDefault(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultDelRangeIndexNWithSizeDefaultAndTSCErase(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultDelRangeIndexNWithSizeDefaultAndDelItemIndexDefault(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL DeeObject_TDefaultSetRangeWithSetRangeIndexAndSetRangeIndexN(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL DeeObject_TDefaultSetRangeWithSetRangeIndexDefaultAndSetRangeIndexNDefault(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeObject_TDefaultSetRangeIndexWithSetRange(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_TDefaultSetRangeIndexWithSizeDefaultAndTSCEraseAndTSCInsertAll(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL DeeObject_TDefaultSetRangeIndexNWithSetRange(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL DeeSeq_TDefaultSetRangeIndexNWithSizeAndSetRangeIndex(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL DeeSeq_TDefaultSetRangeIndexNWithSizeDefaultAndSetRangeIndex(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL DeeSeq_TDefaultSetRangeIndexNWithSizeDefaultAndSetRangeIndexDefault(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultUnpackWithUnpackEx(DeeTypeObject *tp_self, DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultUnpackWithAsVector(DeeTypeObject *tp_self, DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultUnpackWithSizeAndGetItemIndexFast(DeeTypeObject *tp_self, DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultUnpackWithSizeAndTryGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultUnpackWithSizeAndGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultUnpackWithSizeDefaultAndTryGetItemIndexDefault(DeeTypeObject *tp_self, DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultUnpackWithSizeDefaultAndGetItemIndexDefault(DeeTypeObject *tp_self, DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultUnpackWithForeach(DeeTypeObject *tp_self, DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultUnpackWithIter(DeeTypeObject *tp_self, DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultUnpackWithForeachDefault(DeeTypeObject *tp_self, DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultUnpackUbWithSizeAndGetItemIndexFast(DeeTypeObject *tp_self, DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultUnpackUbWithSizeAndTryGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultUnpackUbWithSizeAndGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultUnpackUbWithSizeDefaultAndTryGetItemIndexDefault(DeeTypeObject *tp_self, DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultUnpackUbWithSizeDefaultAndGetItemIndexDefault(DeeTypeObject *tp_self, DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultUnpackUbWithSizeDefaultAndEnumerateIndex(DeeTypeObject *tp_self, DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultUnpackUbWithSizeDefaultAndEnumerateIndexDefault(DeeTypeObject *tp_self, DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst);
#if 0
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultUnpackUbWithUnpackEx(DeeTypeObject *tp_self, DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultUnpackUbWithAsVector(DeeTypeObject *tp_self, DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultUnpackUbWithForeach(DeeTypeObject *tp_self, DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultUnpackUbWithIter(DeeTypeObject *tp_self, DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_TDefaultUnpackUbWithForeachDefault(DeeTypeObject *tp_self, DeeObject *self, size_t dst_length, /*out*/ DREF DeeObject **dst);
#endif
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_TDefaultUnpackExWithAsVector(DeeTypeObject *tp_self, DeeObject *self, size_t dst_length_min, size_t dst_length_max, /*out*/ DREF DeeObject **dst);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_TDefaultUnpackExWithSizeAndGetItemIndexFast(DeeTypeObject *tp_self, DeeObject *self, size_t dst_length_min, size_t dst_length_max, /*out*/ DREF DeeObject **dst);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_TDefaultUnpackExWithSizeAndTryGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, size_t dst_length_min, size_t dst_length_max, /*out*/ DREF DeeObject **dst);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_TDefaultUnpackExWithSizeAndGetItemIndex(DeeTypeObject *tp_self, DeeObject *self, size_t dst_length_min, size_t dst_length_max, /*out*/ DREF DeeObject **dst);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_TDefaultUnpackExWithSizeDefaultAndTryGetItemIndexDefault(DeeTypeObject *tp_self, DeeObject *self, size_t dst_length_min, size_t dst_length_max, /*out*/ DREF DeeObject **dst);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_TDefaultUnpackExWithSizeDefaultAndGetItemIndexDefault(DeeTypeObject *tp_self, DeeObject *self, size_t dst_length_min, size_t dst_length_max, /*out*/ DREF DeeObject **dst);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_TDefaultUnpackExWithForeach(DeeTypeObject *tp_self, DeeObject *self, size_t dst_length_min, size_t dst_length_max, /*out*/ DREF DeeObject **dst);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_TDefaultUnpackExWithIter(DeeTypeObject *tp_self, DeeObject *self, size_t dst_length_min, size_t dst_length_max, /*out*/ DREF DeeObject **dst);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_TDefaultUnpackExWithForeachDefault(DeeTypeObject *tp_self, DeeObject *self, size_t dst_length_min, size_t dst_length_max, /*out*/ DREF DeeObject **dst);
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
#define DeeType_MapDefaultBool(tp_bool, map, default) \
	((tp_bool) == &DeeSeq_DefaultBoolWithSize ? map(DeeSeq_TDefaultBoolWithSize) : \
	 (tp_bool) == &DeeSeq_DefaultBoolWithSizeOb ? map(DeeSeq_TDefaultBoolWithSizeOb) : \
	 (tp_bool) == &DeeSeq_DefaultBoolWithForeach ? map(DeeSeq_TDefaultBoolWithForeach) : \
	 (tp_bool) == &DeeSeq_DefaultBoolWithCompareEq ? map(DeeSeq_TDefaultBoolWithCompareEq) : \
	 (tp_bool) == &DeeSeq_DefaultBoolWithEq ? map(DeeSeq_TDefaultBoolWithEq) : \
	 (tp_bool) == &DeeSeq_DefaultBoolWithNe ? map(DeeSeq_TDefaultBoolWithNe) : \
	 (tp_bool) == &DeeSeq_DefaultBoolWithForeachDefault ? map(DeeSeq_TDefaultBoolWithForeachDefault) : default)
#define DeeType_MapDefaultHash(tp_hash, map, default) \
	((tp_hash) == &DeeSeq_DefaultHashWithSizeAndGetItemIndexFast ? map(DeeSeq_TDefaultHashWithSizeAndGetItemIndexFast) : \
	 (tp_hash) == &DeeSeq_DefaultHashWithForeach ? map(DeeSeq_TDefaultHashWithForeach) : \
	 (tp_hash) == &DeeSeq_DefaultHashWithSizeAndTryGetItemIndex ? map(DeeSeq_TDefaultHashWithSizeAndTryGetItemIndex) : \
	 (tp_hash) == &DeeSeq_DefaultHashWithSizeAndGetItemIndex ? map(DeeSeq_TDefaultHashWithSizeAndGetItemIndex) : \
	 (tp_hash) == &DeeSeq_DefaultHashWithSizeObAndGetItem ? map(DeeSeq_TDefaultHashWithSizeObAndGetItem) : \
	 (tp_hash) == &DeeSeq_DefaultHashWithForeachDefault ? map(DeeSeq_TDefaultHashWithForeachDefault) : \
	 (tp_hash) == &DeeSet_DefaultHashWithForeachDefault ? map(DeeSet_TDefaultHashWithForeachDefault) : \
	 (tp_hash) == &DeeMap_DefaultHashWithForeachPairDefault ? map(DeeMap_TDefaultHashWithForeachPairDefault) : default)
#define DeeType_MapDefaultEq(tp_eq, map, default) \
	((tp_eq) == &DeeObject_DefaultEqWithCompareEq ? map(DeeObject_TDefaultEqWithCompareEq) : \
	 (tp_eq) == &DeeObject_DefaultEqWithNe ? map(DeeObject_TDefaultEqWithNe) : \
	 (tp_eq) == &DeeObject_DefaultEqWithLoAndGr ? map(DeeObject_TDefaultEqWithLoAndGr) : \
	 (tp_eq) == &DeeObject_DefaultEqWithLeAndGe ? map(DeeObject_TDefaultEqWithLeAndGe) : \
	 (tp_eq) == &DeeObject_DefaultEqWithCompareEqDefault ? map(DeeObject_TDefaultEqWithCompareEqDefault) : default)
#define DeeType_MapDefaultNe(tp_ne, map, default) \
	((tp_ne) == &DeeObject_DefaultNeWithCompareEq ? map(DeeObject_TDefaultNeWithCompareEq) : \
	 (tp_ne) == &DeeObject_DefaultNeWithEq ? map(DeeObject_TDefaultNeWithEq) : \
	 (tp_ne) == &DeeObject_DefaultNeWithLoAndGr ? map(DeeObject_TDefaultNeWithLoAndGr) : \
	 (tp_ne) == &DeeObject_DefaultNeWithLeAndGe ? map(DeeObject_TDefaultNeWithLeAndGe) : \
	 (tp_ne) == &DeeObject_DefaultNeWithCompareEqDefault ? map(DeeObject_TDefaultNeWithCompareEqDefault) : default)
#define DeeType_MapDefaultLo(tp_lo, map, default) \
	((tp_lo) == &DeeObject_DefaultLoWithCompare ? map(DeeObject_TDefaultLoWithCompare) : \
	 (tp_lo) == &DeeObject_DefaultLoWithGe ? map(DeeObject_TDefaultLoWithGe) : \
	 (tp_lo) == &DeeObject_DefaultLoWithCompareDefault ? map(DeeObject_TDefaultLoWithCompareDefault) : \
	 (tp_lo) == &DeeSet_DefaultLoWithForeachDefault ? map(DeeSet_TDefaultLoWithForeachDefault) : \
	 (tp_lo) == &DeeMap_DefaultLoWithForeachPairDefault ? map(DeeMap_TDefaultLoWithForeachPairDefault) : default)
#define DeeType_MapDefaultLe(tp_le, map, default) \
	((tp_le) == &DeeObject_DefaultLeWithCompare ? map(DeeObject_TDefaultLeWithCompare) : \
	 (tp_le) == &DeeObject_DefaultLeWithGr ? map(DeeObject_TDefaultLeWithGr) : \
	 (tp_le) == &DeeObject_DefaultLeWithCompareDefault ? map(DeeObject_TDefaultLeWithCompareDefault) : \
	 (tp_le) == &DeeSet_DefaultLeWithForeachDefault ? map(DeeSet_TDefaultLeWithForeachDefault) : \
	 (tp_le) == &DeeMap_DefaultLeWithForeachPairDefault ? map(DeeMap_TDefaultLeWithForeachPairDefault) : default)
#define DeeType_MapDefaultGr(tp_gr, map, default) \
	((tp_gr) == &DeeObject_DefaultGrWithCompare ? map(DeeObject_TDefaultGrWithCompare) : \
	 (tp_gr) == &DeeObject_DefaultGrWithLe ? map(DeeObject_TDefaultGrWithLe) : \
	 (tp_gr) == &DeeObject_DefaultGrWithCompareDefault ? map(DeeObject_TDefaultGrWithCompareDefault) : \
	 (tp_gr) == &DeeSet_DefaultGrWithForeachDefault ? map(DeeSet_TDefaultGrWithForeachDefault) : \
	 (tp_gr) == &DeeMap_DefaultGrWithForeachPairDefault ? map(DeeMap_TDefaultGrWithForeachPairDefault) : default)
#define DeeType_MapDefaultGe(tp_ge, map, default) \
	((tp_ge) == &DeeObject_DefaultGeWithCompare ? map(DeeObject_TDefaultGeWithCompare) : \
	 (tp_ge) == &DeeObject_DefaultGeWithLo ? map(DeeObject_TDefaultGeWithLo) : \
	 (tp_ge) == &DeeObject_DefaultGeWithCompareDefault ? map(DeeObject_TDefaultGeWithCompareDefault) : \
	 (tp_ge) == &DeeSet_DefaultGeWithForeachDefault ? map(DeeSet_TDefaultGeWithForeachDefault) : \
	 (tp_ge) == &DeeMap_DefaultGeWithForeachPairDefault ? map(DeeMap_TDefaultGeWithForeachPairDefault) : default)
#define DeeType_MapDefaultCompareEq(tp_compare_eq, map, default) \
	((tp_compare_eq) == &DeeObject_DefaultCompareEqWithEq ? map(DeeObject_TDefaultCompareEqWithEq) : \
	 (tp_compare_eq) == &DeeObject_DefaultCompareEqWithNe ? map(DeeObject_TDefaultCompareEqWithNe) : \
	 (tp_compare_eq) == &DeeObject_DefaultCompareEqWithLoAndGr ? map(DeeObject_TDefaultCompareEqWithLoAndGr) : \
	 (tp_compare_eq) == &DeeObject_DefaultCompareEqWithLeAndGe ? map(DeeObject_TDefaultCompareEqWithLeAndGe) : \
	 (tp_compare_eq) == &DeeSeq_DefaultCompareEqWithForeachDefault ? map(DeeSeq_TDefaultCompareEqWithForeachDefault) : \
	 (tp_compare_eq) == &DeeSeq_DefaultCompareEqWithSizeAndGetItemIndexFast ? map(DeeSeq_TDefaultCompareEqWithSizeAndGetItemIndexFast) : \
	 (tp_compare_eq) == &DeeSeq_DefaultCompareEqWithSizeAndTryGetItemIndex ? map(DeeSeq_TDefaultCompareEqWithSizeAndTryGetItemIndex) : \
	 (tp_compare_eq) == &DeeSeq_DefaultCompareEqWithSizeAndGetItemIndex ? map(DeeSeq_TDefaultCompareEqWithSizeAndGetItemIndex) : \
	 (tp_compare_eq) == &DeeSeq_DefaultCompareEqWithSizeObAndGetItem ? map(DeeSeq_TDefaultCompareEqWithSizeObAndGetItem) : \
	 (tp_compare_eq) == &DeeSet_DefaultCompareEqWithForeachDefault ? map(DeeSet_TDefaultCompareEqWithForeachDefault) : \
	 (tp_compare_eq) == &DeeMap_DefaultCompareEqWithForeachPairDefault ? map(DeeMap_TDefaultCompareEqWithForeachPairDefault) : default)
#define DeeType_MapDefaultTryCompareEq(tp_trycompare_eq, map, default) \
	((tp_trycompare_eq) == &DeeObject_DefaultTryCompareEqWithCompareEq ? map(DeeObject_TDefaultTryCompareEqWithCompareEq) : \
	 (tp_trycompare_eq) == &DeeObject_DefaultTryCompareEqWithEq ? map(DeeObject_TDefaultTryCompareEqWithEq) : \
	 (tp_trycompare_eq) == &DeeObject_DefaultTryCompareEqWithNe ? map(DeeObject_TDefaultTryCompareEqWithNe) : \
	 (tp_trycompare_eq) == &DeeObject_DefaultTryCompareEqWithCompare ? map(DeeObject_TDefaultTryCompareEqWithCompare) : \
	 (tp_trycompare_eq) == &DeeObject_DefaultTryCompareEqWithLoAndGr ? map(DeeObject_TDefaultTryCompareEqWithLoAndGr) : \
	 (tp_trycompare_eq) == &DeeObject_DefaultTryCompareEqWithLeAndGe ? map(DeeObject_TDefaultTryCompareEqWithLeAndGe) : \
	 (tp_trycompare_eq) == &DeeSeq_DefaultTryCompareEqWithForeachDefault ? map(DeeSeq_TDefaultTryCompareEqWithForeachDefault) : \
	 (tp_trycompare_eq) == &DeeSeq_DefaultTryCompareEqWithSizeAndGetItemIndexFast ? map(DeeSeq_TDefaultTryCompareEqWithSizeAndGetItemIndexFast) : \
	 (tp_trycompare_eq) == &DeeSeq_DefaultTryCompareEqWithSizeAndTryGetItemIndex ? map(DeeSeq_TDefaultTryCompareEqWithSizeAndTryGetItemIndex) : \
	 (tp_trycompare_eq) == &DeeSeq_DefaultTryCompareEqWithSizeAndGetItemIndex ? map(DeeSeq_TDefaultTryCompareEqWithSizeAndGetItemIndex) : \
	 (tp_trycompare_eq) == &DeeSeq_DefaultTryCompareEqWithSizeObAndGetItem ? map(DeeSeq_TDefaultTryCompareEqWithSizeObAndGetItem) : \
	 (tp_trycompare_eq) == &DeeSet_DefaultTryCompareEqWithForeachDefault ? map(DeeSet_TDefaultTryCompareEqWithForeachDefault) : \
	 (tp_trycompare_eq) == &DeeMap_DefaultTryCompareEqWithForeachPairDefault ? map(DeeMap_TDefaultTryCompareEqWithForeachPairDefault) : default)
#define DeeType_MapDefaultCompare(tp_compare, map, default) \
	((tp_compare) == &DeeObject_DefaultCompareWithEqAndLo ? map(DeeObject_TDefaultCompareWithEqAndLo) : \
	 (tp_compare) == &DeeObject_DefaultCompareWithEqAndLe ? map(DeeObject_TDefaultCompareWithEqAndLe) : \
	 (tp_compare) == &DeeObject_DefaultCompareWithEqAndGr ? map(DeeObject_TDefaultCompareWithEqAndGr) : \
	 (tp_compare) == &DeeObject_DefaultCompareWithEqAndGe ? map(DeeObject_TDefaultCompareWithEqAndGe) : \
	 (tp_compare) == &DeeObject_DefaultCompareWithNeAndLo ? map(DeeObject_TDefaultCompareWithNeAndLo) : \
	 (tp_compare) == &DeeObject_DefaultCompareWithNeAndLe ? map(DeeObject_TDefaultCompareWithNeAndLe) : \
	 (tp_compare) == &DeeObject_DefaultCompareWithNeAndGr ? map(DeeObject_TDefaultCompareWithNeAndGr) : \
	 (tp_compare) == &DeeObject_DefaultCompareWithNeAndGe ? map(DeeObject_TDefaultCompareWithNeAndGe) : \
	 (tp_compare) == &DeeObject_DefaultCompareWithLoAndGr ? map(DeeObject_TDefaultCompareWithLoAndGr) : \
	 (tp_compare) == &DeeObject_DefaultCompareWithLeAndGe ? map(DeeObject_TDefaultCompareWithLeAndGe) : \
	 (tp_compare) == &DeeSeq_DefaultCompareWithSizeAndGetItemIndexFast ? map(DeeSeq_TDefaultCompareWithSizeAndGetItemIndexFast) : \
	 (tp_compare) == &DeeSeq_DefaultCompareWithSizeAndTryGetItemIndex ? map(DeeSeq_TDefaultCompareWithSizeAndTryGetItemIndex) : \
	 (tp_compare) == &DeeSeq_DefaultCompareWithSizeAndGetItemIndex ? map(DeeSeq_TDefaultCompareWithSizeAndGetItemIndex) : \
	 (tp_compare) == &DeeSeq_DefaultCompareWithSizeObAndGetItem ? map(DeeSeq_TDefaultCompareWithSizeObAndGetItem) : \
	 (tp_compare) == &DeeSeq_DefaultCompareWithForeachDefault ? map(DeeSeq_TDefaultCompareWithForeachDefault) : default)
#define DeeType_MapDefaultIterNext(tp_iter_next, map, default) \
	((tp_iter_next) == &DeeObject_DefaultIterNextWithIterNextPair ? map(DeeObject_TDefaultIterNextWithIterNextPair) : default)
#define DeeType_MapDefaultIterNextPair(tp_nextpair, map, default) \
	((tp_nextpair) == &DeeObject_DefaultIterNextPairWithIterNext ? map(DeeObject_TDefaultIterNextPairWithIterNext) : default)
#define DeeType_MapDefaultIterNextKey(tp_nextkey, map, default) \
	((tp_nextkey) == &DeeObject_DefaultIterNextKeyWithIterNext ? map(DeeObject_TDefaultIterNextKeyWithIterNext) : \
	 (tp_nextkey) == &DeeObject_DefaultIterNextKeyWithIterNextPair ? map(DeeObject_TDefaultIterNextKeyWithIterNextPair) : default)
#define DeeType_MapDefaultIterNextValue(tp_nextvalue, map, default) \
	((tp_nextvalue) == &DeeObject_DefaultIterNextValueWithIterNext ? map(DeeObject_TDefaultIterNextValueWithIterNext) : \
	 (tp_nextvalue) == &DeeObject_DefaultIterNextValueWithIterNextPair ? map(DeeObject_TDefaultIterNextValueWithIterNextPair) : default)
#define DeeType_MapDefaultIterAdvance(tp_advance, map, default) \
	((tp_advance) == &DeeObject_DefaultIterAdvanceWithIterNext ? map(DeeObject_TDefaultIterAdvanceWithIterNext) : \
	 (tp_advance) == &DeeObject_DefaultIterAdvanceWithIterNextPair ? map(DeeObject_TDefaultIterAdvanceWithIterNextPair) : \
	 (tp_advance) == &DeeObject_DefaultIterAdvanceWithIterNextKey ? map(DeeObject_TDefaultIterAdvanceWithIterNextKey) : \
	 (tp_advance) == &DeeObject_DefaultIterAdvanceWithIterNextValue ? map(DeeObject_TDefaultIterAdvanceWithIterNextValue) : default)
#define DeeType_MapDefaultIter(tp_iter, map, default) \
	((tp_iter) == &DeeObject_DefaultIterWithForeach ? map(DeeObject_TDefaultIterWithForeach) : \
	 (tp_iter) == &DeeObject_DefaultIterWithForeachPair ? map(DeeObject_TDefaultIterWithForeachPair) : \
	 (tp_iter) == &DeeObject_DefaultIterWithEnumerate ? map(DeeObject_TDefaultIterWithEnumerate) : \
	 (tp_iter) == &DeeObject_DefaultIterWithEnumerateIndex ? map(DeeObject_TDefaultIterWithEnumerateIndex) : \
	 (tp_iter) == &DeeObject_DefaultIterWithIterKeysAndTryGetItem ? map(DeeObject_TDefaultIterWithIterKeysAndTryGetItem) : \
	 (tp_iter) == &DeeObject_DefaultIterWithIterKeysAndGetItem ? map(DeeObject_TDefaultIterWithIterKeysAndGetItem) : \
	 (tp_iter) == &DeeObject_DefaultIterWithIterKeysAndTryGetItemDefault ? map(DeeObject_TDefaultIterWithIterKeysAndTryGetItemDefault) : \
	 (tp_iter) == &DeeSeq_DefaultIterWithSizeAndGetItemIndexFast ? map(DeeSeq_TDefaultIterWithSizeAndGetItemIndexFast) : \
	 (tp_iter) == &DeeSeq_DefaultIterWithSizeAndTryGetItemIndex ? map(DeeSeq_TDefaultIterWithSizeAndTryGetItemIndex) : \
	 (tp_iter) == &DeeSeq_DefaultIterWithSizeAndGetItemIndex ? map(DeeSeq_TDefaultIterWithSizeAndGetItemIndex) : \
	 (tp_iter) == &DeeSeq_DefaultIterWithGetItemIndex ? map(DeeSeq_TDefaultIterWithGetItemIndex) : \
	 (tp_iter) == &DeeSeq_DefaultIterWithSizeObAndGetItem ? map(DeeSeq_TDefaultIterWithSizeObAndGetItem) : \
	 (tp_iter) == &DeeSeq_DefaultIterWithGetItem ? map(DeeSeq_TDefaultIterWithGetItem) : \
	 (tp_iter) == &DeeMap_DefaultIterWithEnumerate ? map(DeeMap_TDefaultIterWithEnumerate) : \
	 (tp_iter) == &DeeMap_DefaultIterWithEnumerateIndex ? map(DeeMap_TDefaultIterWithEnumerateIndex) : \
	 (tp_iter) == &DeeMap_DefaultIterWithIterKeysAndTryGetItem ? map(DeeMap_TDefaultIterWithIterKeysAndTryGetItem) : \
	 (tp_iter) == &DeeMap_DefaultIterWithIterKeysAndGetItem ? map(DeeMap_TDefaultIterWithIterKeysAndGetItem) : \
	 (tp_iter) == &DeeMap_DefaultIterWithIterKeysAndTryGetItemDefault ? map(DeeMap_TDefaultIterWithIterKeysAndTryGetItemDefault) : default)
#define DeeType_MapDefaultForeach(tp_foreach, map, default) \
	((tp_foreach) == &DeeObject_DefaultForeachWithIter ? map(DeeObject_TDefaultForeachWithIter) : \
	 (tp_foreach) == &DeeObject_DefaultForeachWithEnumerate ? map(DeeObject_TDefaultForeachWithEnumerate) : \
	 (tp_foreach) == &DeeObject_DefaultForeachWithEnumerateIndex ? map(DeeObject_TDefaultForeachWithEnumerateIndex) : \
	 (tp_foreach) == &DeeObject_DefaultForeachWithForeachPair ? map(DeeObject_TDefaultForeachWithForeachPair) : \
	 (tp_foreach) == &DeeObject_DefaultForeachWithForeachPairDefault ? map(DeeObject_TDefaultForeachWithForeachPairDefault) : \
	 (tp_foreach) == &DeeObject_DefaultForeachWithIterKeysAndTryGetItem ? map(DeeObject_TDefaultForeachWithIterKeysAndTryGetItem) : \
	 (tp_foreach) == &DeeObject_DefaultForeachWithIterKeysAndGetItem ? map(DeeObject_TDefaultForeachWithIterKeysAndGetItem) : \
	 (tp_foreach) == &DeeObject_DefaultForeachWithIterKeysAndTryGetItemDefault ? map(DeeObject_TDefaultForeachWithIterKeysAndTryGetItemDefault) : \
	 (tp_foreach) == &DeeSeq_DefaultForeachWithSizeAndGetItemIndexFast ? map(DeeSeq_TDefaultForeachWithSizeAndGetItemIndexFast) : \
	 (tp_foreach) == &DeeSeq_DefaultForeachWithSizeAndTryGetItemIndex ? map(DeeSeq_TDefaultForeachWithSizeAndTryGetItemIndex) : \
	 (tp_foreach) == &DeeSeq_DefaultForeachWithSizeAndGetItemIndex ? map(DeeSeq_TDefaultForeachWithSizeAndGetItemIndex) : \
	 (tp_foreach) == &DeeSeq_DefaultForeachWithSizeObAndGetItem ? map(DeeSeq_TDefaultForeachWithSizeObAndGetItem) : \
	 (tp_foreach) == &DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault ? map(DeeSeq_TDefaultForeachWithSizeDefaultAndGetItemIndexDefault) : \
	 (tp_foreach) == &DeeSeq_DefaultForeachWithGetItemIndexDefault ? map(DeeSeq_TDefaultForeachWithGetItemIndexDefault) : default)
#define DeeType_MapDefaultForeachPair(tp_foreach_pair, map, default) \
	((tp_foreach_pair) == &DeeObject_DefaultForeachPairWithForeach ? map(DeeObject_TDefaultForeachPairWithForeach) : \
	 (tp_foreach_pair) == &DeeObject_DefaultForeachPairWithEnumerate ? map(DeeObject_TDefaultForeachPairWithEnumerate) : \
	 (tp_foreach_pair) == &DeeObject_DefaultForeachPairWithEnumerateIndex ? map(DeeObject_TDefaultForeachPairWithEnumerateIndex) : \
	 (tp_foreach_pair) == &DeeObject_DefaultForeachPairWithForeachDefault ? map(DeeObject_TDefaultForeachPairWithForeachDefault) : \
	 (tp_foreach_pair) == &DeeObject_DefaultForeachPairWithIter ? map(DeeObject_TDefaultForeachPairWithIter) : \
	 (tp_foreach_pair) == &DeeMap_DefaultForeachPairWithEnumerate ? map(DeeMap_TDefaultForeachPairWithEnumerate) : \
	 (tp_foreach_pair) == &DeeMap_DefaultForeachPairWithEnumerateIndex ? map(DeeMap_TDefaultForeachPairWithEnumerateIndex) : \
	 (tp_foreach_pair) == &DeeMap_DefaultForeachPairWithEnumerateDefault ? map(DeeMap_TDefaultForeachPairWithEnumerateDefault) : default)
#define DeeType_MapDefaultEnumerate(tp_enumerate, map, default) \
	((tp_enumerate) == &DeeObject_DefaultEnumerateWithEnumerateIndex ? map(DeeObject_TDefaultEnumerateWithEnumerateIndex) : \
	 (tp_enumerate) == &DeeObject_DefaultEnumerateWithIterKeysAndTryGetItem ? map(DeeObject_TDefaultEnumerateWithIterKeysAndTryGetItem) : \
	 (tp_enumerate) == &DeeObject_DefaultEnumerateWithIterKeysAndGetItem ? map(DeeObject_TDefaultEnumerateWithIterKeysAndGetItem) : \
	 (tp_enumerate) == &DeeObject_DefaultEnumerateWithIterKeysAndTryGetItemDefault ? map(DeeObject_TDefaultEnumerateWithIterKeysAndTryGetItemDefault) : \
	 (tp_enumerate) == &DeeSeq_DefaultEnumerateWithSizeAndGetItemIndexFast ? map(DeeSeq_TDefaultEnumerateWithSizeAndGetItemIndexFast) : \
	 (tp_enumerate) == &DeeSeq_DefaultEnumerateWithSizeAndTryGetItemIndex ? map(DeeSeq_TDefaultEnumerateWithSizeAndTryGetItemIndex) : \
	 (tp_enumerate) == &DeeSeq_DefaultEnumerateWithSizeAndGetItemIndex ? map(DeeSeq_TDefaultEnumerateWithSizeAndGetItemIndex) : \
	 (tp_enumerate) == &DeeSeq_DefaultEnumerateWithSizeObAndGetItem ? map(DeeSeq_TDefaultEnumerateWithSizeObAndGetItem) : \
	 (tp_enumerate) == &DeeSeq_DefaultEnumerateWithCounterAndForeach ? map(DeeSeq_TDefaultEnumerateWithCounterAndForeach) : \
	 (tp_enumerate) == &DeeSeq_DefaultEnumerateWithCounterAndIter ? map(DeeSeq_TDefaultEnumerateWithCounterAndIter) : \
	 (tp_enumerate) == &DeeMap_DefaultEnumerateWithForeachPairDefault ? map(DeeMap_TDefaultEnumerateWithForeachPairDefault) : \
	 (tp_enumerate) == &DeeMap_DefaultEnumerateWithIter ? map(DeeMap_TDefaultEnumerateWithIter) : \
	 (tp_enumerate) == &DeeSeq_DefaultEnumerateWithSizeDefaultAndGetItemIndexDefault ? map(DeeSeq_TDefaultEnumerateWithSizeDefaultAndGetItemIndexDefault) : \
	 (tp_enumerate) == &DeeSeq_DefaultEnumerateWithCounterAndForeachDefault ? map(DeeSeq_TDefaultEnumerateWithCounterAndForeachDefault) : default)
#define DeeType_MapDefaultEnumerateIndex(tp_enumerate_index, map, default) \
	((tp_enumerate_index) == &DeeObject_DefaultEnumerateIndexWithEnumerate ? map(DeeObject_TDefaultEnumerateIndexWithEnumerate) : \
	 (tp_enumerate_index) == &DeeSeq_DefaultEnumerateIndexWithSizeAndGetItemIndexFast ? map(DeeSeq_TDefaultEnumerateIndexWithSizeAndGetItemIndexFast) : \
	 (tp_enumerate_index) == &DeeSeq_DefaultEnumerateIndexWithSizeAndTryGetItemIndex ? map(DeeSeq_TDefaultEnumerateIndexWithSizeAndTryGetItemIndex) : \
	 (tp_enumerate_index) == &DeeSeq_DefaultEnumerateIndexWithSizeAndGetItemIndex ? map(DeeSeq_TDefaultEnumerateIndexWithSizeAndGetItemIndex) : \
	 (tp_enumerate_index) == &DeeSeq_DefaultEnumerateIndexWithSizeObAndGetItem ? map(DeeSeq_TDefaultEnumerateIndexWithSizeObAndGetItem) : \
	 (tp_enumerate_index) == &DeeSeq_DefaultEnumerateIndexWithCounterAndForeach ? map(DeeSeq_TDefaultEnumerateIndexWithCounterAndForeach) : \
	 (tp_enumerate_index) == &DeeSeq_DefaultEnumerateIndexWithCounterAndIter ? map(DeeSeq_TDefaultEnumerateIndexWithCounterAndIter) : \
	 (tp_enumerate_index) == &DeeSeq_DefaultEnumerateIndexWithSizeDefaultAndGetItemIndexDefault ? map(DeeSeq_TDefaultEnumerateIndexWithSizeDefaultAndGetItemIndexDefault) : \
	 (tp_enumerate_index) == &DeeSeq_DefaultEnumerateIndexWithCounterAndForeachDefault ? map(DeeSeq_TDefaultEnumerateIndexWithCounterAndForeachDefault) : \
	 (tp_enumerate_index) == &DeeObject_DefaultEnumerateIndexWithEnumerateDefault ? map(DeeObject_TDefaultEnumerateIndexWithEnumerateDefault) : default)
#define DeeType_MapDefaultIterKeys(tp_iterkeys, map, default) \
	((tp_iterkeys) == &DeeObject_DefaultIterKeysWithEnumerate ? map(DeeObject_TDefaultIterKeysWithEnumerate) : \
	 (tp_iterkeys) == &DeeObject_DefaultIterKeysWithEnumerateIndex ? map(DeeObject_TDefaultIterKeysWithEnumerateIndex) : \
	 (tp_iterkeys) == &DeeSeq_DefaultIterKeysWithSize ? map(DeeSeq_TDefaultIterKeysWithSize) : \
	 (tp_iterkeys) == &DeeSeq_DefaultIterKeysWithSizeOb ? map(DeeSeq_TDefaultIterKeysWithSizeOb) : \
	 (tp_iterkeys) == &DeeSeq_DefaultIterKeysWithSizeDefault ? map(DeeSeq_TDefaultIterKeysWithSizeDefault) : \
	 (tp_iterkeys) == &DeeMap_DefaultIterKeysWithIter ? map(DeeMap_TDefaultIterKeysWithIter) : \
	 (tp_iterkeys) == &DeeMap_DefaultIterKeysWithIterDefault ? map(DeeMap_TDefaultIterKeysWithIterDefault) : default)
#define DeeType_MapDefaultSizeOb(tp_sizeob, map, default) \
	((tp_sizeob) == &DeeObject_DefaultSizeObWithSize ? map(DeeObject_TDefaultSizeObWithSize) : \
	 (tp_sizeob) == &DeeObject_DefaultSizeObWithSizeDefault ? map(DeeObject_TDefaultSizeObWithSizeDefault) : default)
#define DeeType_MapDefaultSize(tp_size, map, default) \
	((tp_size) == &DeeObject_DefaultSizeWithSizeOb ? map(DeeObject_TDefaultSizeWithSizeOb) : \
	 (tp_size) == &DeeSeq_DefaultSizeWithEnumerateIndex ? map(DeeSeq_TDefaultSizeWithEnumerateIndex) : \
	 (tp_size) == &DeeSeq_DefaultSizeWithEnumerate ? map(DeeSeq_TDefaultSizeWithEnumerate) : \
	 (tp_size) == &DeeSeq_DefaultSizeWithForeachPair ? map(DeeSeq_TDefaultSizeWithForeachPair) : \
	 (tp_size) == &DeeSeq_DefaultSizeWithForeach ? map(DeeSeq_TDefaultSizeWithForeach) : \
	 (tp_size) == &DeeSeq_DefaultSizeWithIter ? map(DeeSeq_TDefaultSizeWithIter) : default)
#define DeeType_MapDefaultSizeFast(tp_size_fast, map, default) default
#define DeeType_MapDefaultContains(tp_contains, map, default) \
	((tp_contains) == &DeeSeq_DefaultContainsWithForeachDefault ? map(DeeSeq_TDefaultContainsWithForeachDefault) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithHasItem ? map(DeeMap_TDefaultContainsWithHasItem) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithBoundItem ? map(DeeMap_TDefaultContainsWithBoundItem) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithTryGetItem ? map(DeeMap_TDefaultContainsWithTryGetItem) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithGetItem ? map(DeeMap_TDefaultContainsWithGetItem) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithHasItemStringHash ? map(DeeMap_TDefaultContainsWithHasItemStringHash) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithHasItemStringLenHash ? map(DeeMap_TDefaultContainsWithHasItemStringLenHash) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithHasItemIndex ? map(DeeMap_TDefaultContainsWithHasItemIndex) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithBoundItemStringHash ? map(DeeMap_TDefaultContainsWithBoundItemStringHash) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithBoundItemStringLenHash ? map(DeeMap_TDefaultContainsWithBoundItemStringLenHash) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithBoundItemIndex ? map(DeeMap_TDefaultContainsWithBoundItemIndex) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithTryGetItemStringHash ? map(DeeMap_TDefaultContainsWithTryGetItemStringHash) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithTryGetItemStringLenHash ? map(DeeMap_TDefaultContainsWithTryGetItemStringLenHash) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithTryGetItemIndex ? map(DeeMap_TDefaultContainsWithTryGetItemIndex) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithGetItemStringHash ? map(DeeMap_TDefaultContainsWithGetItemStringHash) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithGetItemStringLenHash ? map(DeeMap_TDefaultContainsWithGetItemStringLenHash) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithGetItemIndex ? map(DeeMap_TDefaultContainsWithGetItemIndex) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithEnumerate ? map(DeeMap_TDefaultContainsWithEnumerate) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithEnumerateDefault ? map(DeeMap_TDefaultContainsWithEnumerateDefault) : default)
#define DeeType_MapDefaultGetItem(tp_getitem, map, default) \
	((tp_getitem) == &DeeObject_DefaultGetItemWithSizeAndGetItemIndexFast ? map(DeeObject_TDefaultGetItemWithSizeAndGetItemIndexFast) : \
	 (tp_getitem) == &DeeObject_DefaultGetItemWithGetItemIndex ? map(DeeObject_TDefaultGetItemWithGetItemIndex) : \
	 (tp_getitem) == &DeeObject_DefaultGetItemWithGetItemStringHash ? map(DeeObject_TDefaultGetItemWithGetItemStringHash) : \
	 (tp_getitem) == &DeeObject_DefaultGetItemWithGetItemStringLenHash ? map(DeeObject_TDefaultGetItemWithGetItemStringLenHash) : \
	 (tp_getitem) == &DeeObject_DefaultGetItemWithTryGetItem ? map(DeeObject_TDefaultGetItemWithTryGetItem) : \
	 (tp_getitem) == &DeeObject_DefaultGetItemWithTryGetItemIndex ? map(DeeObject_TDefaultGetItemWithTryGetItemIndex) : \
	 (tp_getitem) == &DeeObject_DefaultGetItemWithTryGetItemStringHash ? map(DeeObject_TDefaultGetItemWithTryGetItemStringHash) : \
	 (tp_getitem) == &DeeObject_DefaultGetItemWithTryGetItemStringLenHash ? map(DeeObject_TDefaultGetItemWithTryGetItemStringLenHash) : \
	 (tp_getitem) == &DeeObject_DefaultGetItemWithGetItemIndexDefault ? map(DeeObject_TDefaultGetItemWithGetItemIndexDefault) : \
	 (tp_getitem) == &DeeSeq_DefaultGetItemWithTryGetItemAndSizeOb ? map(DeeSeq_TDefaultGetItemWithTryGetItemAndSizeOb) : \
	 (tp_getitem) == &DeeSeq_DefaultGetItemWithTryGetItemAndSize ? map(DeeSeq_TDefaultGetItemWithTryGetItemAndSize) : \
	 (tp_getitem) == &DeeSeq_DefaultGetItemWithSizeAndTryGetItemIndexOb ? map(DeeSeq_TDefaultGetItemWithSizeAndTryGetItemIndexOb) : \
	 (tp_getitem) == &DeeSeq_DefaultGetItemWithSizeAndTryGetItemIndex ? map(DeeSeq_TDefaultGetItemWithSizeAndTryGetItemIndex) : \
	 (tp_getitem) == &DeeMap_DefaultGetItemWithEnumerate ? map(DeeMap_TDefaultGetItemWithEnumerate) : \
	 (tp_getitem) == &DeeMap_DefaultGetItemWithEnumerateDefault ? map(DeeMap_TDefaultGetItemWithEnumerateDefault) : default)
#define DeeType_MapDefaultGetItemIndex(tp_getitem_index, map, default) \
	((tp_getitem_index) == &DeeObject_DefaultGetItemIndexWithSizeAndGetItemIndexFast ? map(DeeObject_TDefaultGetItemIndexWithSizeAndGetItemIndexFast) : \
	 (tp_getitem_index) == &DeeObject_DefaultGetItemIndexWithTryGetItemIndex ? map(DeeObject_TDefaultGetItemIndexWithTryGetItemIndex) : \
	 (tp_getitem_index) == &DeeObject_DefaultGetItemIndexWithGetItem ? map(DeeObject_TDefaultGetItemIndexWithGetItem) : \
	 (tp_getitem_index) == &DeeObject_DefaultGetItemIndexWithTryGetItem ? map(DeeObject_TDefaultGetItemIndexWithTryGetItem) : \
	 (tp_getitem_index) == &DeeObject_DefaultGetItemIndexWithGetItemDefault ? map(DeeObject_TDefaultGetItemIndexWithGetItemDefault) : \
	 (tp_getitem_index) == &DeeSeq_DefaultGetItemIndexWithSizeAndTryGetItemIndex ? map(DeeSeq_TDefaultGetItemIndexWithSizeAndTryGetItemIndex) : \
	 (tp_getitem_index) == &DeeSeq_DefaultGetItemIndexWithSizeAndTryGetItemIndexOb ? map(DeeSeq_TDefaultGetItemIndexWithSizeAndTryGetItemIndexOb) : \
	 (tp_getitem_index) == &DeeSeq_DefaultGetItemIndexWithTryGetItemAndSize ? map(DeeSeq_TDefaultGetItemIndexWithTryGetItemAndSize) : \
	 (tp_getitem_index) == &DeeSeq_DefaultGetItemIndexWithTryGetItemAndSizeOb ? map(DeeSeq_TDefaultGetItemIndexWithTryGetItemAndSizeOb) : \
	 (tp_getitem_index) == &DeeSeq_DefaultGetItemIndexWithForeachDefault ? map(DeeSeq_TDefaultGetItemIndexWithForeachDefault) : \
	 (tp_getitem_index) == &DeeMap_DefaultGetItemIndexWithEnumerate ? map(DeeMap_TDefaultGetItemIndexWithEnumerate) : \
	 (tp_getitem_index) == &DeeMap_DefaultGetItemIndexWithEnumerateDefault ? map(DeeMap_TDefaultGetItemIndexWithEnumerateDefault) : default)
#define DeeType_MapDefaultGetItemStringHash(tp_getitem_string_hash, map, default) \
	((tp_getitem_string_hash) == &DeeObject_DefaultGetItemStringHashWithGetItemStringLenHash ? map(DeeObject_TDefaultGetItemStringHashWithGetItemStringLenHash) : \
	 (tp_getitem_string_hash) == &DeeObject_DefaultGetItemStringHashWithGetItem ? map(DeeObject_TDefaultGetItemStringHashWithGetItem) : \
	 (tp_getitem_string_hash) == &DeeObject_DefaultGetItemStringHashWithTryGetItemStringHash ? map(DeeObject_TDefaultGetItemStringHashWithTryGetItemStringHash) : \
	 (tp_getitem_string_hash) == &DeeObject_DefaultGetItemStringHashWithTryGetItemStringLenHash ? map(DeeObject_TDefaultGetItemStringHashWithTryGetItemStringLenHash) : \
	 (tp_getitem_string_hash) == &DeeObject_DefaultGetItemStringHashWithTryGetItem ? map(DeeObject_TDefaultGetItemStringHashWithTryGetItem) : \
	 (tp_getitem_string_hash) == &DeeObject_DefaultGetItemStringHashWithGetItemDefault ? map(DeeObject_TDefaultGetItemStringHashWithGetItemDefault) : \
	 (tp_getitem_string_hash) == &DeeMap_DefaultGetItemStringHashWithEnumerate ? map(DeeMap_TDefaultGetItemStringHashWithEnumerate) : \
	 (tp_getitem_string_hash) == &DeeMap_DefaultGetItemStringHashWithEnumerateDefault ? map(DeeMap_TDefaultGetItemStringHashWithEnumerateDefault) : default)
#define DeeType_MapDefaultGetItemStringLenHash(tp_getitem_string_len_hash, map, default) \
	((tp_getitem_string_len_hash) == &DeeObject_DefaultGetItemStringLenHashWithGetItemStringHash ? map(DeeObject_TDefaultGetItemStringLenHashWithGetItemStringHash) : \
	 (tp_getitem_string_len_hash) == &DeeObject_DefaultGetItemStringLenHashWithGetItem ? map(DeeObject_TDefaultGetItemStringLenHashWithGetItem) : \
	 (tp_getitem_string_len_hash) == &DeeObject_DefaultGetItemStringLenHashWithTryGetItemStringLenHash ? map(DeeObject_TDefaultGetItemStringLenHashWithTryGetItemStringLenHash) : \
	 (tp_getitem_string_len_hash) == &DeeObject_DefaultGetItemStringLenHashWithTryGetItemStringHash ? map(DeeObject_TDefaultGetItemStringLenHashWithTryGetItemStringHash) : \
	 (tp_getitem_string_len_hash) == &DeeObject_DefaultGetItemStringLenHashWithTryGetItem ? map(DeeObject_TDefaultGetItemStringLenHashWithTryGetItem) : \
	 (tp_getitem_string_len_hash) == &DeeObject_DefaultGetItemStringLenHashWithGetItemDefault ? map(DeeObject_TDefaultGetItemStringLenHashWithGetItemDefault) : \
	 (tp_getitem_string_len_hash) == &DeeMap_DefaultGetItemStringLenHashWithEnumerate ? map(DeeMap_TDefaultGetItemStringLenHashWithEnumerate) : \
	 (tp_getitem_string_len_hash) == &DeeMap_DefaultGetItemStringLenHashWithEnumerateDefault ? map(DeeMap_TDefaultGetItemStringLenHashWithEnumerateDefault) : default)
#define DeeType_MapDefaultTryGetItem(tp_trygetitem, map, default) \
	((tp_trygetitem) == &DeeObject_DefaultTryGetItemWithTryGetItemIndex ? map(DeeObject_TDefaultTryGetItemWithTryGetItemIndex) : \
	 (tp_trygetitem) == &DeeObject_DefaultTryGetItemWithTryGetItemStringHash ? map(DeeObject_TDefaultTryGetItemWithTryGetItemStringHash) : \
	 (tp_trygetitem) == &DeeObject_DefaultTryGetItemWithTryGetItemStringLenHash ? map(DeeObject_TDefaultTryGetItemWithTryGetItemStringLenHash) : \
	 (tp_trygetitem) == &DeeObject_DefaultTryGetItemWithSizeAndGetItemIndexFast ? map(DeeObject_TDefaultTryGetItemWithSizeAndGetItemIndexFast) : \
	 (tp_trygetitem) == &DeeObject_DefaultTryGetItemWithGetItemIndex ? map(DeeObject_TDefaultTryGetItemWithGetItemIndex) : \
	 (tp_trygetitem) == &DeeObject_DefaultTryGetItemWithGetItem ? map(DeeObject_TDefaultTryGetItemWithGetItem) : \
	 (tp_trygetitem) == &DeeObject_DefaultTryGetItemWithGetItemStringHash ? map(DeeObject_TDefaultTryGetItemWithGetItemStringHash) : \
	 (tp_trygetitem) == &DeeObject_DefaultTryGetItemWithGetItemStringLenHash ? map(DeeObject_TDefaultTryGetItemWithGetItemStringLenHash) : \
	 (tp_trygetitem) == &DeeObject_DefaultTryGetItemWithGetItemDefault ? map(DeeObject_TDefaultTryGetItemWithGetItemDefault) : \
	 (tp_trygetitem) == &DeeMap_DefaultTryGetItemWithEnumerate ? map(DeeMap_TDefaultTryGetItemWithEnumerate) : \
	 (tp_trygetitem) == &DeeMap_DefaultTryGetItemWithEnumerateDefault ? map(DeeMap_TDefaultTryGetItemWithEnumerateDefault) : default)
#define DeeType_MapDefaultTryGetItemIndex(tp_trygetitem_index, map, default) \
	((tp_trygetitem_index) == &DeeObject_DefaultTryGetItemIndexWithSizeAndGetItemIndexFast ? map(DeeObject_TDefaultTryGetItemIndexWithSizeAndGetItemIndexFast) : \
	 (tp_trygetitem_index) == &DeeObject_DefaultTryGetItemIndexWithTryGetItem ? map(DeeObject_TDefaultTryGetItemIndexWithTryGetItem) : \
	 (tp_trygetitem_index) == &DeeObject_DefaultTryGetItemIndexWithGetItemIndex ? map(DeeObject_TDefaultTryGetItemIndexWithGetItemIndex) : \
	 (tp_trygetitem_index) == &DeeObject_DefaultTryGetItemIndexWithGetItem ? map(DeeObject_TDefaultTryGetItemIndexWithGetItem) : \
	 (tp_trygetitem_index) == &DeeObject_DefaultTryGetItemIndexWithGetItemIndexDefault ? map(DeeObject_TDefaultTryGetItemIndexWithGetItemIndexDefault) : \
	 (tp_trygetitem_index) == &DeeSeq_DefaultTryGetItemIndexWithForeachDefault ? map(DeeSeq_TDefaultTryGetItemIndexWithForeachDefault) : \
	 (tp_trygetitem_index) == &DeeMap_DefaultTryGetItemIndexWithEnumerate ? map(DeeMap_TDefaultTryGetItemIndexWithEnumerate) : \
	 (tp_trygetitem_index) == &DeeMap_DefaultTryGetItemIndexWithEnumerateDefault ? map(DeeMap_TDefaultTryGetItemIndexWithEnumerateDefault) : default)
#define DeeType_MapDefaultTryGetItemStringHash(tp_trygetitem_string_hash, map, default) \
	((tp_trygetitem_string_hash) == &DeeObject_DefaultTryGetItemStringHashWithTryGetItemStringLenHash ? map(DeeObject_TDefaultTryGetItemStringHashWithTryGetItemStringLenHash) : \
	 (tp_trygetitem_string_hash) == &DeeObject_DefaultTryGetItemStringHashWithTryGetItem ? map(DeeObject_TDefaultTryGetItemStringHashWithTryGetItem) : \
	 (tp_trygetitem_string_hash) == &DeeObject_DefaultTryGetItemStringHashWithGetItemStringHash ? map(DeeObject_TDefaultTryGetItemStringHashWithGetItemStringHash) : \
	 (tp_trygetitem_string_hash) == &DeeObject_DefaultTryGetItemStringHashWithGetItemStringLenHash ? map(DeeObject_TDefaultTryGetItemStringHashWithGetItemStringLenHash) : \
	 (tp_trygetitem_string_hash) == &DeeObject_DefaultTryGetItemStringHashWithGetItem ? map(DeeObject_TDefaultTryGetItemStringHashWithGetItem) : \
	 (tp_trygetitem_string_hash) == &DeeObject_DefaultTryGetItemStringHashWithTryGetItemDefault ? map(DeeObject_TDefaultTryGetItemStringHashWithTryGetItemDefault) : \
	 (tp_trygetitem_string_hash) == &DeeMap_DefaultTryGetItemStringHashWithEnumerate ? map(DeeMap_TDefaultTryGetItemStringHashWithEnumerate) : \
	 (tp_trygetitem_string_hash) == &DeeMap_DefaultTryGetItemStringHashWithEnumerateDefault ? map(DeeMap_TDefaultTryGetItemStringHashWithEnumerateDefault) : default)
#define DeeType_MapDefaultTryGetItemStringLenHash(tp_trygetitem_string_len_hash, map, default) \
	((tp_trygetitem_string_len_hash) == &DeeObject_DefaultTryGetItemStringLenHashWithTryGetItemStringHash ? map(DeeObject_TDefaultTryGetItemStringLenHashWithTryGetItemStringHash) : \
	 (tp_trygetitem_string_len_hash) == &DeeObject_DefaultTryGetItemStringLenHashWithTryGetItem ? map(DeeObject_TDefaultTryGetItemStringLenHashWithTryGetItem) : \
	 (tp_trygetitem_string_len_hash) == &DeeObject_DefaultTryGetItemStringLenHashWithGetItemStringLenHash ? map(DeeObject_TDefaultTryGetItemStringLenHashWithGetItemStringLenHash) : \
	 (tp_trygetitem_string_len_hash) == &DeeObject_DefaultTryGetItemStringLenHashWithGetItemStringHash ? map(DeeObject_TDefaultTryGetItemStringLenHashWithGetItemStringHash) : \
	 (tp_trygetitem_string_len_hash) == &DeeObject_DefaultTryGetItemStringLenHashWithGetItem ? map(DeeObject_TDefaultTryGetItemStringLenHashWithGetItem) : \
	 (tp_trygetitem_string_len_hash) == &DeeObject_DefaultTryGetItemStringLenHashWithTryGetItemDefault ? map(DeeObject_TDefaultTryGetItemStringLenHashWithTryGetItemDefault) : \
	 (tp_trygetitem_string_len_hash) == &DeeMap_DefaultTryGetItemStringLenHashWithEnumerate ? map(DeeMap_TDefaultTryGetItemStringLenHashWithEnumerate) : \
	 (tp_trygetitem_string_len_hash) == &DeeMap_DefaultTryGetItemStringLenHashWithEnumerateDefault ? map(DeeMap_TDefaultTryGetItemStringLenHashWithEnumerateDefault) : default)
#define DeeType_MapDefaultDelItem(tp_delitem, map, default) \
	((tp_delitem) == &DeeObject_DefaultDelItemWithDelItemIndex ? map(DeeObject_TDefaultDelItemWithDelItemIndex) : \
	 (tp_delitem) == &DeeObject_DefaultDelItemWithDelItemIndexDefault ? map(DeeObject_TDefaultDelItemWithDelItemIndexDefault) : \
	 (tp_delitem) == &DeeObject_DefaultDelItemWithDelItemStringHash ? map(DeeObject_TDefaultDelItemWithDelItemStringHash) : \
	 (tp_delitem) == &DeeObject_DefaultDelItemWithDelItemStringLenHash ? map(DeeObject_TDefaultDelItemWithDelItemStringLenHash) : default)
#define DeeType_MapDefaultDelItemIndex(tp_delitem_index, map, default) \
	((tp_delitem_index) == &DeeObject_DefaultDelItemIndexWithDelItem ? map(DeeObject_TDefaultDelItemIndexWithDelItem) : \
	 (tp_delitem_index) == &DeeSeq_DefaultDelItemIndexWithDelRangeIndexDefault ? map(DeeSeq_TDefaultDelItemIndexWithDelRangeIndexDefault) : default)
#define DeeType_MapDefaultDelItemStringHash(tp_delitem_string_hash, map, default) \
	((tp_delitem_string_hash) == &DeeObject_DefaultDelItemStringHashWithDelItemStringLenHash ? map(DeeObject_TDefaultDelItemStringHashWithDelItemStringLenHash) : \
	 (tp_delitem_string_hash) == &DeeObject_DefaultDelItemStringHashWithDelItem ? map(DeeObject_TDefaultDelItemStringHashWithDelItem) : default)
#define DeeType_MapDefaultDelItemStringLenHash(tp_delitem_string_len_hash, map, default) \
	((tp_delitem_string_len_hash) == &DeeObject_DefaultDelItemStringLenHashWithDelItemStringHash ? map(DeeObject_TDefaultDelItemStringLenHashWithDelItemStringHash) : \
	 (tp_delitem_string_len_hash) == &DeeObject_DefaultDelItemStringLenHashWithDelItem ? map(DeeObject_TDefaultDelItemStringLenHashWithDelItem) : default)
#define DeeType_MapDefaultSetItem(tp_setitem, map, default) \
	((tp_setitem) == &DeeObject_DefaultSetItemWithSetItemIndex ? map(DeeObject_TDefaultSetItemWithSetItemIndex) : \
	 (tp_setitem) == &DeeObject_DefaultSetItemWithSetItemIndexDefault ? map(DeeObject_TDefaultSetItemWithSetItemIndexDefault) : \
	 (tp_setitem) == &DeeObject_DefaultSetItemWithSetItemStringHash ? map(DeeObject_TDefaultSetItemWithSetItemStringHash) : \
	 (tp_setitem) == &DeeObject_DefaultSetItemWithSetItemStringLenHash ? map(DeeObject_TDefaultSetItemWithSetItemStringLenHash) : default)
#define DeeType_MapDefaultSetItemIndex(tp_setitem_index, map, default) \
	((tp_setitem_index) == &DeeObject_DefaultSetItemIndexWithSetItem ? map(DeeObject_TDefaultSetItemIndexWithSetItem) : \
	 (tp_setitem_index) == &DeeSeq_DefaultSetItemIndexWithSetRangeIndexDefault ? map(DeeSeq_TDefaultSetItemIndexWithSetRangeIndexDefault) : default)
#define DeeType_MapDefaultSetItemStringHash(tp_setitem_string_hash, map, default) \
	((tp_setitem_string_hash) == &DeeObject_DefaultSetItemStringHashWithSetItemStringLenHash ? map(DeeObject_TDefaultSetItemStringHashWithSetItemStringLenHash) : \
	 (tp_setitem_string_hash) == &DeeObject_DefaultSetItemStringHashWithSetItem ? map(DeeObject_TDefaultSetItemStringHashWithSetItem) : default)
#define DeeType_MapDefaultSetItemStringLenHash(tp_setitem_string_len_hash, map, default) \
	((tp_setitem_string_len_hash) == &DeeObject_DefaultSetItemStringLenHashWithSetItemStringHash ? map(DeeObject_TDefaultSetItemStringLenHashWithSetItemStringHash) : \
	 (tp_setitem_string_len_hash) == &DeeObject_DefaultSetItemStringLenHashWithSetItem ? map(DeeObject_TDefaultSetItemStringLenHashWithSetItem) : default)
#define DeeType_MapDefaultBoundItem(tp_bounditem, map, default) \
	((tp_bounditem) == &DeeObject_DefaultBoundItemWithBoundItemIndex ? map(DeeObject_TDefaultBoundItemWithBoundItemIndex) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithBoundItemStringHash ? map(DeeObject_TDefaultBoundItemWithBoundItemStringHash) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithBoundItemStringLenHash ? map(DeeObject_TDefaultBoundItemWithBoundItemStringLenHash) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithGetItem ? map(DeeObject_TDefaultBoundItemWithGetItem) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithGetItemStringHash ? map(DeeObject_TDefaultBoundItemWithGetItemStringHash) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithGetItemStringLenHash ? map(DeeObject_TDefaultBoundItemWithGetItemStringLenHash) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithGetItemIndex ? map(DeeObject_TDefaultBoundItemWithGetItemIndex) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithTryGetItemAndHasItem ? map(DeeObject_TDefaultBoundItemWithTryGetItemAndHasItem) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithTryGetItemIndexAndHasItemIndex ? map(DeeObject_TDefaultBoundItemWithTryGetItemIndexAndHasItemIndex) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithTryGetItemStringLenHashAndHasItemStringLenHash ? map(DeeObject_TDefaultBoundItemWithTryGetItemStringLenHashAndHasItemStringLenHash) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithTryGetItemStringHashAndHasItemStringHash ? map(DeeObject_TDefaultBoundItemWithTryGetItemStringHashAndHasItemStringHash) : \
	 (tp_bounditem) == &DeeSeq_DefaultBoundItemWithTryGetItemAndSizeOb ? map(DeeSeq_TDefaultBoundItemWithTryGetItemAndSizeOb) : \
	 (tp_bounditem) == &DeeSeq_DefaultBoundItemWithSizeAndTryGetItemIndex ? map(DeeSeq_TDefaultBoundItemWithSizeAndTryGetItemIndex) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithTryGetItem ? map(DeeObject_TDefaultBoundItemWithTryGetItem) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithTryGetItemIndex ? map(DeeObject_TDefaultBoundItemWithTryGetItemIndex) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithTryGetItemStringLenHash ? map(DeeObject_TDefaultBoundItemWithTryGetItemStringLenHash) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithTryGetItemStringHash ? map(DeeObject_TDefaultBoundItemWithTryGetItemStringHash) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithGetItemDefault ? map(DeeObject_TDefaultBoundItemWithGetItemDefault) : \
	 (tp_bounditem) == &DeeMap_DefaultBoundItemWithContains ? map(DeeMap_TDefaultBoundItemWithContains) : \
	 (tp_bounditem) == &DeeMap_DefaultBoundItemWithEnumerate ? map(DeeMap_TDefaultBoundItemWithEnumerate) : \
	 (tp_bounditem) == &DeeMap_DefaultBoundItemWithEnumerateDefault ? map(DeeMap_TDefaultBoundItemWithEnumerateDefault) : default)
#define DeeType_MapDefaultBoundItemIndex(tp_bounditem_index, map, default) \
	((tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithBoundItem ? map(DeeObject_TDefaultBoundItemIndexWithBoundItem) : \
	 (tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithSizeAndGetItemIndexFast ? map(DeeObject_TDefaultBoundItemIndexWithSizeAndGetItemIndexFast) : \
	 (tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithGetItemIndex ? map(DeeObject_TDefaultBoundItemIndexWithGetItemIndex) : \
	 (tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithGetItem ? map(DeeObject_TDefaultBoundItemIndexWithGetItem) : \
	 (tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithGetItemIndexDefault ? map(DeeObject_TDefaultBoundItemIndexWithGetItemIndexDefault) : \
	 (tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithTryGetItemIndexAndHasItemIndex ? map(DeeObject_TDefaultBoundItemIndexWithTryGetItemIndexAndHasItemIndex) : \
	 (tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithTryGetItemAndHasItem ? map(DeeObject_TDefaultBoundItemIndexWithTryGetItemAndHasItem) : \
	 (tp_bounditem_index) == &DeeSeq_DefaultBoundItemIndexWithSizeAndTryGetItemIndex ? map(DeeSeq_TDefaultBoundItemIndexWithSizeAndTryGetItemIndex) : \
	 (tp_bounditem_index) == &DeeSeq_DefaultBoundItemIndexWithTryGetItemAndSizeOb ? map(DeeSeq_TDefaultBoundItemIndexWithTryGetItemAndSizeOb) : \
	 (tp_bounditem_index) == &DeeMap_DefaultBoundItemIndexWithContains ? map(DeeMap_TDefaultBoundItemIndexWithContains) : \
	 (tp_bounditem_index) == &DeeMap_DefaultBoundItemIndexWithEnumerate ? map(DeeMap_TDefaultBoundItemIndexWithEnumerate) : \
	 (tp_bounditem_index) == &DeeMap_DefaultBoundItemIndexWithEnumerateDefault ? map(DeeMap_TDefaultBoundItemIndexWithEnumerateDefault) : default)
#define DeeType_MapDefaultBoundItemStringHash(tp_bounditem_string_hash, map, default) \
	((tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithBoundItemStringLenHash ? map(DeeObject_TDefaultBoundItemStringHashWithBoundItemStringLenHash) : \
	 (tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithBoundItem ? map(DeeObject_TDefaultBoundItemStringHashWithBoundItem) : \
	 (tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithGetItemStringHash ? map(DeeObject_TDefaultBoundItemStringHashWithGetItemStringHash) : \
	 (tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithGetItemStringLenHash ? map(DeeObject_TDefaultBoundItemStringHashWithGetItemStringLenHash) : \
	 (tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithGetItem ? map(DeeObject_TDefaultBoundItemStringHashWithGetItem) : \
	 (tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithTryGetItemStringHashAndHasItemStringHash ? map(DeeObject_TDefaultBoundItemStringHashWithTryGetItemStringHashAndHasItemStringHash) : \
	 (tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithTryGetItemStringLenHashAndHasItemStringLenHash ? map(DeeObject_TDefaultBoundItemStringHashWithTryGetItemStringLenHashAndHasItemStringLenHash) : \
	 (tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithTryGetItemAndHasItem ? map(DeeObject_TDefaultBoundItemStringHashWithTryGetItemAndHasItem) : \
	 (tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithTryGetItemStringHash ? map(DeeObject_TDefaultBoundItemStringHashWithTryGetItemStringHash) : \
	 (tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithTryGetItemStringLenHash ? map(DeeObject_TDefaultBoundItemStringHashWithTryGetItemStringLenHash) : \
	 (tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithTryGetItem ? map(DeeObject_TDefaultBoundItemStringHashWithTryGetItem) : \
	 (tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithBoundItemDefault ? map(DeeObject_TDefaultBoundItemStringHashWithBoundItemDefault) : \
	 (tp_bounditem_string_hash) == &DeeMap_DefaultBoundItemStringHashWithContains ? map(DeeMap_TDefaultBoundItemStringHashWithContains) : \
	 (tp_bounditem_string_hash) == &DeeMap_DefaultBoundItemStringHashWithEnumerate ? map(DeeMap_TDefaultBoundItemStringHashWithEnumerate) : \
	 (tp_bounditem_string_hash) == &DeeMap_DefaultBoundItemStringHashWithEnumerateDefault ? map(DeeMap_TDefaultBoundItemStringHashWithEnumerateDefault) : default)
#define DeeType_MapDefaultBoundItemStringLenHash(tp_bounditem_string_len_hash, map, default) \
	((tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithBoundItemStringHash ? map(DeeObject_TDefaultBoundItemStringLenHashWithBoundItemStringHash) : \
	 (tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithBoundItem ? map(DeeObject_TDefaultBoundItemStringLenHashWithBoundItem) : \
	 (tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithGetItemStringLenHash ? map(DeeObject_TDefaultBoundItemStringLenHashWithGetItemStringLenHash) : \
	 (tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithGetItemStringHash ? map(DeeObject_TDefaultBoundItemStringLenHashWithGetItemStringHash) : \
	 (tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithGetItem ? map(DeeObject_TDefaultBoundItemStringLenHashWithGetItem) : \
	 (tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemStringLenHashAndHasItemStringLenHash ? map(DeeObject_TDefaultBoundItemStringLenHashWithTryGetItemStringLenHashAndHasItemStringLenHash) : \
	 (tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemStringHashAndHasItemStringHash ? map(DeeObject_TDefaultBoundItemStringLenHashWithTryGetItemStringHashAndHasItemStringHash) : \
	 (tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemAndHasItem ? map(DeeObject_TDefaultBoundItemStringLenHashWithTryGetItemAndHasItem) : \
	 (tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemStringLenHash ? map(DeeObject_TDefaultBoundItemStringLenHashWithTryGetItemStringLenHash) : \
	 (tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemStringHash ? map(DeeObject_TDefaultBoundItemStringLenHashWithTryGetItemStringHash) : \
	 (tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithTryGetItem ? map(DeeObject_TDefaultBoundItemStringLenHashWithTryGetItem) : \
	 (tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithBoundItemDefault ? map(DeeObject_TDefaultBoundItemStringLenHashWithBoundItemDefault) : \
	 (tp_bounditem_string_len_hash) == &DeeMap_DefaultBoundItemStringLenHashWithContains ? map(DeeMap_TDefaultBoundItemStringLenHashWithContains) : \
	 (tp_bounditem_string_len_hash) == &DeeMap_DefaultBoundItemStringLenHashWithEnumerate ? map(DeeMap_TDefaultBoundItemStringLenHashWithEnumerate) : \
	 (tp_bounditem_string_len_hash) == &DeeMap_DefaultBoundItemStringLenHashWithEnumerateDefault ? map(DeeMap_TDefaultBoundItemStringLenHashWithEnumerateDefault) : default)
#define DeeType_MapDefaultHasItem(tp_hasitem, map, default) \
	((tp_hasitem) == &DeeObject_DefaultHasItemWithHasItemStringHash ? map(DeeObject_TDefaultHasItemWithHasItemStringHash) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithHasItemStringLenHash ? map(DeeObject_TDefaultHasItemWithHasItemStringLenHash) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithHasItemIndex ? map(DeeObject_TDefaultHasItemWithHasItemIndex) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithBoundItem ? map(DeeObject_TDefaultHasItemWithBoundItem) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithBoundItemStringHash ? map(DeeObject_TDefaultHasItemWithBoundItemStringHash) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithBoundItemStringLenHash ? map(DeeObject_TDefaultHasItemWithBoundItemStringLenHash) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithBoundItemIndex ? map(DeeObject_TDefaultHasItemWithBoundItemIndex) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithTryGetItem ? map(DeeObject_TDefaultHasItemWithTryGetItem) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithTryGetItemStringHash ? map(DeeObject_TDefaultHasItemWithTryGetItemStringHash) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithTryGetItemStringLenHash ? map(DeeObject_TDefaultHasItemWithTryGetItemStringLenHash) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithTryGetItemIndex ? map(DeeObject_TDefaultHasItemWithTryGetItemIndex) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithGetItem ? map(DeeObject_TDefaultHasItemWithGetItem) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithGetItemStringHash ? map(DeeObject_TDefaultHasItemWithGetItemStringHash) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithGetItemStringLenHash ? map(DeeObject_TDefaultHasItemWithGetItemStringLenHash) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithGetItemIndex ? map(DeeObject_TDefaultHasItemWithGetItemIndex) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithGetItemDefault ? map(DeeObject_TDefaultHasItemWithGetItemDefault) : \
	 (tp_hasitem) == &DeeSeq_DefaultHasItemWithSize ? map(DeeSeq_TDefaultHasItemWithSize) : \
	 (tp_hasitem) == &DeeSeq_DefaultHasItemWithSizeOb ? map(DeeSeq_TDefaultHasItemWithSizeOb) : \
	 (tp_hasitem) == &DeeMap_DefaultHasItemWithContains ? map(DeeMap_TDefaultHasItemWithContains) : \
	 (tp_hasitem) == &DeeMap_DefaultHasItemWithEnumerate ? map(DeeMap_TDefaultHasItemWithEnumerate) : \
	 (tp_hasitem) == &DeeMap_DefaultHasItemWithEnumerateDefault ? map(DeeMap_TDefaultHasItemWithEnumerateDefault) : default)
#define DeeType_MapDefaultHasItemIndex(tp_hasitem_index, map, default) \
	((tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithHasItem ? map(DeeObject_TDefaultHasItemIndexWithHasItem) : \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithBoundItemIndex ? map(DeeObject_TDefaultHasItemIndexWithBoundItemIndex) : \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithBoundItem ? map(DeeObject_TDefaultHasItemIndexWithBoundItem) : \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithTryGetItemIndex ? map(DeeObject_TDefaultHasItemIndexWithTryGetItemIndex) : \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithTryGetItem ? map(DeeObject_TDefaultHasItemIndexWithTryGetItem) : \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithGetItemIndex ? map(DeeObject_TDefaultHasItemIndexWithGetItemIndex) : \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithGetItem ? map(DeeObject_TDefaultHasItemIndexWithGetItem) : \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithGetItemIndexDefault ? map(DeeObject_TDefaultHasItemIndexWithGetItemIndexDefault) : \
	 (tp_hasitem_index) == &DeeSeq_DefaultHasItemIndexWithSize ? map(DeeSeq_TDefaultHasItemIndexWithSize) : \
	 (tp_hasitem_index) == &DeeSeq_DefaultHasItemIndexWithSizeDefault ? map(DeeSeq_TDefaultHasItemIndexWithSizeDefault) : \
	 (tp_hasitem_index) == &DeeMap_DefaultHasItemIndexWithContains ? map(DeeMap_TDefaultHasItemIndexWithContains) : \
	 (tp_hasitem_index) == &DeeMap_DefaultHasItemIndexWithEnumerate ? map(DeeMap_TDefaultHasItemIndexWithEnumerate) : \
	 (tp_hasitem_index) == &DeeMap_DefaultHasItemIndexWithEnumerateDefault ? map(DeeMap_TDefaultHasItemIndexWithEnumerateDefault) : default)
#define DeeType_MapDefaultHasItemStringHash(tp_hasitem_string_hash, map, default) \
	((tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithHasItemStringLenHash ? map(DeeObject_TDefaultHasItemStringHashWithHasItemStringLenHash) : \
	 (tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithBoundItemStringHash ? map(DeeObject_TDefaultHasItemStringHashWithBoundItemStringHash) : \
	 (tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithBoundItemStringLenHash ? map(DeeObject_TDefaultHasItemStringHashWithBoundItemStringLenHash) : \
	 (tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithTryGetItemStringHash ? map(DeeObject_TDefaultHasItemStringHashWithTryGetItemStringHash) : \
	 (tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithTryGetItemStringLenHash ? map(DeeObject_TDefaultHasItemStringHashWithTryGetItemStringLenHash) : \
	 (tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithGetItemStringHash ? map(DeeObject_TDefaultHasItemStringHashWithGetItemStringHash) : \
	 (tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithGetItemStringLenHash ? map(DeeObject_TDefaultHasItemStringHashWithGetItemStringLenHash) : \
	 (tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithHasItem ? map(DeeObject_TDefaultHasItemStringHashWithHasItem) : \
	 (tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithBoundItem ? map(DeeObject_TDefaultHasItemStringHashWithBoundItem) : \
	 (tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithTryGetItem ? map(DeeObject_TDefaultHasItemStringHashWithTryGetItem) : \
	 (tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithGetItem ? map(DeeObject_TDefaultHasItemStringHashWithGetItem) : \
	 (tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithHasItemDefault ? map(DeeObject_TDefaultHasItemStringHashWithHasItemDefault) : \
	 (tp_hasitem_string_hash) == &DeeMap_DefaultHasItemStringHashWithContains ? map(DeeMap_TDefaultHasItemStringHashWithContains) : \
	 (tp_hasitem_string_hash) == &DeeMap_DefaultHasItemStringHashWithEnumerate ? map(DeeMap_TDefaultHasItemStringHashWithEnumerate) : \
	 (tp_hasitem_string_hash) == &DeeMap_DefaultHasItemStringHashWithEnumerateDefault ? map(DeeMap_TDefaultHasItemStringHashWithEnumerateDefault) : default)
#define DeeType_MapDefaultHasItemStringLenHash(tp_hasitem_string_len_hash, map, default) \
	((tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithHasItemStringHash ? map(DeeObject_TDefaultHasItemStringLenHashWithHasItemStringHash) : \
	 (tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithBoundItemStringHash ? map(DeeObject_TDefaultHasItemStringLenHashWithBoundItemStringHash) : \
	 (tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithBoundItemStringLenHash ? map(DeeObject_TDefaultHasItemStringLenHashWithBoundItemStringLenHash) : \
	 (tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithTryGetItemStringLenHash ? map(DeeObject_TDefaultHasItemStringLenHashWithTryGetItemStringLenHash) : \
	 (tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithTryGetItemStringHash ? map(DeeObject_TDefaultHasItemStringLenHashWithTryGetItemStringHash) : \
	 (tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithGetItemStringLenHash ? map(DeeObject_TDefaultHasItemStringLenHashWithGetItemStringLenHash) : \
	 (tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithGetItemStringHash ? map(DeeObject_TDefaultHasItemStringLenHashWithGetItemStringHash) : \
	 (tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithHasItem ? map(DeeObject_TDefaultHasItemStringLenHashWithHasItem) : \
	 (tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithBoundItem ? map(DeeObject_TDefaultHasItemStringLenHashWithBoundItem) : \
	 (tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithTryGetItem ? map(DeeObject_TDefaultHasItemStringLenHashWithTryGetItem) : \
	 (tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithGetItem ? map(DeeObject_TDefaultHasItemStringLenHashWithGetItem) : \
	 (tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithHasItemDefault ? map(DeeObject_TDefaultHasItemStringLenHashWithHasItemDefault) : \
	 (tp_hasitem_string_len_hash) == &DeeMap_DefaultHasItemStringLenHashWithContains ? map(DeeMap_TDefaultHasItemStringLenHashWithContains) : \
	 (tp_hasitem_string_len_hash) == &DeeMap_DefaultHasItemStringLenHashWithEnumerate ? map(DeeMap_TDefaultHasItemStringLenHashWithEnumerate) : \
	 (tp_hasitem_string_len_hash) == &DeeMap_DefaultHasItemStringLenHashWithEnumerateDefault ? map(DeeMap_TDefaultHasItemStringLenHashWithEnumerateDefault) : default)
#define DeeType_MapDefaultGetRange(tp_getrange, map, default) \
	((tp_getrange) == &DeeObject_DefaultGetRangeWithGetRangeIndexAndGetRangeIndexN ? map(DeeObject_TDefaultGetRangeWithGetRangeIndexAndGetRangeIndexN) : \
	 (tp_getrange) == &DeeObject_DefaultGetRangeWithGetRangeIndexDefaultAndGetRangeIndexNDefault ? map(DeeObject_TDefaultGetRangeWithGetRangeIndexDefaultAndGetRangeIndexNDefault) : \
	 (tp_getrange) == &DeeSeq_DefaultGetRangeWithSizeDefaultAndGetItemIndex ? map(DeeSeq_TDefaultGetRangeWithSizeDefaultAndGetItemIndex) : \
	 (tp_getrange) == &DeeSeq_DefaultGetRangeWithSizeDefaultAndTryGetItemIndex ? map(DeeSeq_TDefaultGetRangeWithSizeDefaultAndTryGetItemIndex) : \
	 (tp_getrange) == &DeeSeq_DefaultGetRangeWithSizeObAndGetItem ? map(DeeSeq_TDefaultGetRangeWithSizeObAndGetItem) : default)
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
	 (tp_delrange_index) == &DeeSeq_DefaultDelRangeIndexWithSetRangeIndexNoneDefault ? map(DeeSeq_TDefaultDelRangeIndexWithSetRangeIndexNoneDefault) : \
	 (tp_delrange_index) == &DeeSeq_DefaultDelRangeIndexWithSizeDefaultAndTSCErase ? map(DeeSeq_TDefaultDelRangeIndexWithSizeDefaultAndTSCErase) : \
	 (tp_delrange_index) == &DeeSeq_DefaultDelRangeIndexWithSizeDefaultAndDelItemIndexDefault ? map(DeeSeq_TDefaultDelRangeIndexWithSizeDefaultAndDelItemIndexDefault) : default)
#define DeeType_MapDefaultDelRangeIndexN(tp_delrange_index_n, map, default) \
	((tp_delrange_index_n) == &DeeObject_DefaultDelRangeIndexNWithDelRange ? map(DeeObject_TDefaultDelRangeIndexNWithDelRange) : \
	 (tp_delrange_index_n) == &DeeSeq_DefaultDelRangeIndexNWithSizeAndDelRangeIndex ? map(DeeSeq_TDefaultDelRangeIndexNWithSizeAndDelRangeIndex) : \
	 (tp_delrange_index_n) == &DeeSeq_DefaultDelRangeIndexNWithSizeDefaultAndDelRangeIndex ? map(DeeSeq_TDefaultDelRangeIndexNWithSizeDefaultAndDelRangeIndex) : \
	 (tp_delrange_index_n) == &DeeSeq_DefaultDelRangeIndexNWithSetRangeIndexNNone ? map(DeeSeq_TDefaultDelRangeIndexNWithSetRangeIndexNNone) : \
	 (tp_delrange_index_n) == &DeeSeq_DefaultDelRangeIndexNWithSetRangeIndexNNoneDefault ? map(DeeSeq_TDefaultDelRangeIndexNWithSetRangeIndexNNoneDefault) : \
	 (tp_delrange_index_n) == &DeeSeq_DefaultDelRangeIndexNWithSizeDefaultAndTSCErase ? map(DeeSeq_TDefaultDelRangeIndexNWithSizeDefaultAndTSCErase) : \
	 (tp_delrange_index_n) == &DeeSeq_DefaultDelRangeIndexNWithSizeDefaultAndDelItemIndexDefault ? map(DeeSeq_TDefaultDelRangeIndexNWithSizeDefaultAndDelItemIndexDefault) : default)
#define DeeType_MapDefaultSetRange(tp_setrange, map, default) \
	((tp_setrange) == &DeeObject_DefaultSetRangeWithSetRangeIndexAndSetRangeIndexN ? map(DeeObject_TDefaultSetRangeWithSetRangeIndexAndSetRangeIndexN) : \
	 (tp_setrange) == &DeeObject_DefaultSetRangeWithSetRangeIndexDefaultAndSetRangeIndexNDefault ? map(DeeObject_TDefaultSetRangeWithSetRangeIndexDefaultAndSetRangeIndexNDefault) : default)
#define DeeType_MapDefaultSetRangeIndex(tp_setrange_index, map, default) \
	((tp_setrange_index) == &DeeObject_DefaultSetRangeIndexWithSetRange ? map(DeeObject_TDefaultSetRangeIndexWithSetRange) : \
	 (tp_setrange_index) == &DeeSeq_DefaultSetRangeIndexWithSizeDefaultAndTSCEraseAndTSCInsertAll ? map(DeeSeq_TDefaultSetRangeIndexWithSizeDefaultAndTSCEraseAndTSCInsertAll) : default)
#define DeeType_MapDefaultSetRangeIndexN(tp_setrange_index_n, map, default) \
	((tp_setrange_index_n) == &DeeObject_DefaultSetRangeIndexNWithSetRange ? map(DeeObject_TDefaultSetRangeIndexNWithSetRange) : \
	 (tp_setrange_index_n) == &DeeSeq_DefaultSetRangeIndexNWithSizeAndSetRangeIndex ? map(DeeSeq_TDefaultSetRangeIndexNWithSizeAndSetRangeIndex) : \
	 (tp_setrange_index_n) == &DeeSeq_DefaultSetRangeIndexNWithSizeDefaultAndSetRangeIndex ? map(DeeSeq_TDefaultSetRangeIndexNWithSizeDefaultAndSetRangeIndex) : \
	 (tp_setrange_index_n) == &DeeSeq_DefaultSetRangeIndexNWithSizeDefaultAndSetRangeIndexDefault ? map(DeeSeq_TDefaultSetRangeIndexNWithSizeDefaultAndSetRangeIndexDefault) : default)
#define DeeType_MapDefaultUnpack(tp_unpack, map, default) \
	((tp_unpack) == &DeeSeq_DefaultUnpackWithUnpackEx ? map(DeeSeq_TDefaultUnpackWithUnpackEx) : \
	 (tp_unpack) == &DeeSeq_DefaultUnpackWithAsVector ? map(DeeSeq_TDefaultUnpackWithAsVector) : \
	 (tp_unpack) == &DeeSeq_DefaultUnpackWithSizeAndGetItemIndexFast ? map(DeeSeq_TDefaultUnpackWithSizeAndGetItemIndexFast) : \
	 (tp_unpack) == &DeeSeq_DefaultUnpackWithSizeAndTryGetItemIndex ? map(DeeSeq_TDefaultUnpackWithSizeAndTryGetItemIndex) : \
	 (tp_unpack) == &DeeSeq_DefaultUnpackWithSizeAndGetItemIndex ? map(DeeSeq_TDefaultUnpackWithSizeAndGetItemIndex) : \
	 (tp_unpack) == &DeeSeq_DefaultUnpackWithSizeDefaultAndTryGetItemIndexDefault ? map(DeeSeq_TDefaultUnpackWithSizeDefaultAndTryGetItemIndexDefault) : \
	 (tp_unpack) == &DeeSeq_DefaultUnpackWithSizeDefaultAndGetItemIndexDefault ? map(DeeSeq_TDefaultUnpackWithSizeDefaultAndGetItemIndexDefault) : \
	 (tp_unpack) == &DeeSeq_DefaultUnpackWithForeach ? map(DeeSeq_TDefaultUnpackWithForeach) : \
	 (tp_unpack) == &DeeSeq_DefaultUnpackWithIter ? map(DeeSeq_TDefaultUnpackWithIter) : \
	 (tp_unpack) == &DeeSeq_DefaultUnpackWithForeachDefault ? map(DeeSeq_TDefaultUnpackWithForeachDefault) : default)
#define DeeType_MapDefaultUnpackUb(tp_unpack_ub, map, default) \
	((tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithSizeAndGetItemIndexFast ? map(DeeSeq_TDefaultUnpackUbWithSizeAndGetItemIndexFast) : \
	 (tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithSizeAndTryGetItemIndex ? map(DeeSeq_TDefaultUnpackUbWithSizeAndTryGetItemIndex) : \
	 (tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithSizeAndGetItemIndex ? map(DeeSeq_TDefaultUnpackUbWithSizeAndGetItemIndex) : \
	 (tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithSizeDefaultAndTryGetItemIndexDefault ? map(DeeSeq_TDefaultUnpackUbWithSizeDefaultAndTryGetItemIndexDefault) : \
	 (tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithSizeDefaultAndGetItemIndexDefault ? map(DeeSeq_TDefaultUnpackUbWithSizeDefaultAndGetItemIndexDefault) : \
	 (tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithSizeDefaultAndEnumerateIndex ? map(DeeSeq_TDefaultUnpackUbWithSizeDefaultAndEnumerateIndex) : \
	 (tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithSizeDefaultAndEnumerateIndexDefault ? map(DeeSeq_TDefaultUnpackUbWithSizeDefaultAndEnumerateIndexDefault) : \
	 (tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithUnpackEx ? map(DeeSeq_TDefaultUnpackUbWithUnpackEx) : \
	 (tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithAsVector ? map(DeeSeq_TDefaultUnpackUbWithAsVector) : \
	 (tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithForeach ? map(DeeSeq_TDefaultUnpackUbWithForeach) : \
	 (tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithIter ? map(DeeSeq_TDefaultUnpackUbWithIter) : \
	 (tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithForeachDefault ? map(DeeSeq_TDefaultUnpackUbWithForeachDefault) : default)
#define DeeType_MapDefaultUnpackEx(tp_unpack_ex, map, default) \
	((tp_unpack_ex) == &DeeSeq_DefaultUnpackExWithAsVector ? map(DeeSeq_TDefaultUnpackExWithAsVector) : \
	 (tp_unpack_ex) == &DeeSeq_DefaultUnpackExWithSizeAndGetItemIndexFast ? map(DeeSeq_TDefaultUnpackExWithSizeAndGetItemIndexFast) : \
	 (tp_unpack_ex) == &DeeSeq_DefaultUnpackExWithSizeAndTryGetItemIndex ? map(DeeSeq_TDefaultUnpackExWithSizeAndTryGetItemIndex) : \
	 (tp_unpack_ex) == &DeeSeq_DefaultUnpackExWithSizeAndGetItemIndex ? map(DeeSeq_TDefaultUnpackExWithSizeAndGetItemIndex) : \
	 (tp_unpack_ex) == &DeeSeq_DefaultUnpackExWithSizeDefaultAndTryGetItemIndexDefault ? map(DeeSeq_TDefaultUnpackExWithSizeDefaultAndTryGetItemIndexDefault) : \
	 (tp_unpack_ex) == &DeeSeq_DefaultUnpackExWithSizeDefaultAndGetItemIndexDefault ? map(DeeSeq_TDefaultUnpackExWithSizeDefaultAndGetItemIndexDefault) : \
	 (tp_unpack_ex) == &DeeSeq_DefaultUnpackExWithForeach ? map(DeeSeq_TDefaultUnpackExWithForeach) : \
	 (tp_unpack_ex) == &DeeSeq_DefaultUnpackExWithIter ? map(DeeSeq_TDefaultUnpackExWithIter) : \
	 (tp_unpack_ex) == &DeeSeq_DefaultUnpackExWithForeachDefault ? map(DeeSeq_TDefaultUnpackExWithForeachDefault) : default)
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
#define DeeType_IsDefaultBool(tp_bool) \
	((tp_bool) == &DeeSeq_DefaultBoolWithSize || \
	 (tp_bool) == &DeeSeq_DefaultBoolWithSizeOb || \
	 (tp_bool) == &DeeSeq_DefaultBoolWithForeach || \
	 (tp_bool) == &DeeSeq_DefaultBoolWithCompareEq || \
	 (tp_bool) == &DeeSeq_DefaultBoolWithEq || \
	 (tp_bool) == &DeeSeq_DefaultBoolWithNe || \
	 (tp_bool) == &DeeSeq_DefaultBoolWithForeachDefault)
#define DeeType_IsDefaultHash(tp_hash) \
	((tp_hash) == &DeeSeq_DefaultHashWithSizeAndGetItemIndexFast || \
	 (tp_hash) == &DeeSeq_DefaultHashWithForeach || \
	 (tp_hash) == &DeeSeq_DefaultHashWithSizeAndTryGetItemIndex || \
	 (tp_hash) == &DeeSeq_DefaultHashWithSizeAndGetItemIndex || \
	 (tp_hash) == &DeeSeq_DefaultHashWithSizeObAndGetItem || \
	 (tp_hash) == &DeeSeq_DefaultHashWithForeachDefault || \
	 (tp_hash) == &DeeSet_DefaultHashWithForeachDefault || \
	 (tp_hash) == &DeeMap_DefaultHashWithForeachPairDefault)
#define DeeType_IsDefaultEq(tp_eq) \
	((tp_eq) == &DeeObject_DefaultEqWithCompareEq || \
	 (tp_eq) == &DeeObject_DefaultEqWithNe || \
	 (tp_eq) == &DeeObject_DefaultEqWithLoAndGr || \
	 (tp_eq) == &DeeObject_DefaultEqWithLeAndGe || \
	 (tp_eq) == &DeeObject_DefaultEqWithCompareEqDefault)
#define DeeType_IsDefaultNe(tp_ne) \
	((tp_ne) == &DeeObject_DefaultNeWithCompareEq || \
	 (tp_ne) == &DeeObject_DefaultNeWithEq || \
	 (tp_ne) == &DeeObject_DefaultNeWithLoAndGr || \
	 (tp_ne) == &DeeObject_DefaultNeWithLeAndGe || \
	 (tp_ne) == &DeeObject_DefaultNeWithCompareEqDefault)
#define DeeType_IsDefaultLo(tp_lo) \
	((tp_lo) == &DeeObject_DefaultLoWithCompare || \
	 (tp_lo) == &DeeObject_DefaultLoWithGe || \
	 (tp_lo) == &DeeObject_DefaultLoWithCompareDefault || \
	 (tp_lo) == &DeeSet_DefaultLoWithForeachDefault || \
	 (tp_lo) == &DeeMap_DefaultLoWithForeachPairDefault)
#define DeeType_IsDefaultLe(tp_le) \
	((tp_le) == &DeeObject_DefaultLeWithCompare || \
	 (tp_le) == &DeeObject_DefaultLeWithGr || \
	 (tp_le) == &DeeObject_DefaultLeWithCompareDefault || \
	 (tp_le) == &DeeSet_DefaultLeWithForeachDefault || \
	 (tp_le) == &DeeMap_DefaultLeWithForeachPairDefault)
#define DeeType_IsDefaultGr(tp_gr) \
	((tp_gr) == &DeeObject_DefaultGrWithCompare || \
	 (tp_gr) == &DeeObject_DefaultGrWithLe || \
	 (tp_gr) == &DeeObject_DefaultGrWithCompareDefault || \
	 (tp_gr) == &DeeSet_DefaultGrWithForeachDefault || \
	 (tp_gr) == &DeeMap_DefaultGrWithForeachPairDefault)
#define DeeType_IsDefaultGe(tp_ge) \
	((tp_ge) == &DeeObject_DefaultGeWithCompare || \
	 (tp_ge) == &DeeObject_DefaultGeWithLo || \
	 (tp_ge) == &DeeObject_DefaultGeWithCompareDefault || \
	 (tp_ge) == &DeeSet_DefaultGeWithForeachDefault || \
	 (tp_ge) == &DeeMap_DefaultGeWithForeachPairDefault)
#define DeeType_IsDefaultCompareEq(tp_compare_eq) \
	((tp_compare_eq) == &DeeObject_DefaultCompareEqWithEq || \
	 (tp_compare_eq) == &DeeObject_DefaultCompareEqWithNe || \
	 (tp_compare_eq) == &DeeObject_DefaultCompareEqWithLoAndGr || \
	 (tp_compare_eq) == &DeeObject_DefaultCompareEqWithLeAndGe || \
	 (tp_compare_eq) == &DeeSeq_DefaultCompareEqWithForeachDefault || \
	 (tp_compare_eq) == &DeeSeq_DefaultCompareEqWithSizeAndGetItemIndexFast || \
	 (tp_compare_eq) == &DeeSeq_DefaultCompareEqWithSizeAndTryGetItemIndex || \
	 (tp_compare_eq) == &DeeSeq_DefaultCompareEqWithSizeAndGetItemIndex || \
	 (tp_compare_eq) == &DeeSeq_DefaultCompareEqWithSizeObAndGetItem || \
	 (tp_compare_eq) == &DeeSet_DefaultCompareEqWithForeachDefault || \
	 (tp_compare_eq) == &DeeMap_DefaultCompareEqWithForeachPairDefault)
#define DeeType_IsDefaultTryCompareEq(tp_trycompare_eq) \
	((tp_trycompare_eq) == &DeeObject_DefaultTryCompareEqWithCompareEq || \
	 (tp_trycompare_eq) == &DeeObject_DefaultTryCompareEqWithEq || \
	 (tp_trycompare_eq) == &DeeObject_DefaultTryCompareEqWithNe || \
	 (tp_trycompare_eq) == &DeeObject_DefaultTryCompareEqWithCompare || \
	 (tp_trycompare_eq) == &DeeObject_DefaultTryCompareEqWithLoAndGr || \
	 (tp_trycompare_eq) == &DeeObject_DefaultTryCompareEqWithLeAndGe || \
	 (tp_trycompare_eq) == &DeeSeq_DefaultTryCompareEqWithForeachDefault || \
	 (tp_trycompare_eq) == &DeeSeq_DefaultTryCompareEqWithSizeAndGetItemIndexFast || \
	 (tp_trycompare_eq) == &DeeSeq_DefaultTryCompareEqWithSizeAndTryGetItemIndex || \
	 (tp_trycompare_eq) == &DeeSeq_DefaultTryCompareEqWithSizeAndGetItemIndex || \
	 (tp_trycompare_eq) == &DeeSeq_DefaultTryCompareEqWithSizeObAndGetItem || \
	 (tp_trycompare_eq) == &DeeSet_DefaultTryCompareEqWithForeachDefault || \
	 (tp_trycompare_eq) == &DeeMap_DefaultTryCompareEqWithForeachPairDefault)
#define DeeType_IsDefaultCompare(tp_compare) \
	((tp_compare) == &DeeObject_DefaultCompareWithEqAndLo || \
	 (tp_compare) == &DeeObject_DefaultCompareWithEqAndLe || \
	 (tp_compare) == &DeeObject_DefaultCompareWithEqAndGr || \
	 (tp_compare) == &DeeObject_DefaultCompareWithEqAndGe || \
	 (tp_compare) == &DeeObject_DefaultCompareWithNeAndLo || \
	 (tp_compare) == &DeeObject_DefaultCompareWithNeAndLe || \
	 (tp_compare) == &DeeObject_DefaultCompareWithNeAndGr || \
	 (tp_compare) == &DeeObject_DefaultCompareWithNeAndGe || \
	 (tp_compare) == &DeeObject_DefaultCompareWithLoAndGr || \
	 (tp_compare) == &DeeObject_DefaultCompareWithLeAndGe || \
	 (tp_compare) == &DeeSeq_DefaultCompareWithSizeAndGetItemIndexFast || \
	 (tp_compare) == &DeeSeq_DefaultCompareWithSizeAndTryGetItemIndex || \
	 (tp_compare) == &DeeSeq_DefaultCompareWithSizeAndGetItemIndex || \
	 (tp_compare) == &DeeSeq_DefaultCompareWithSizeObAndGetItem || \
	 (tp_compare) == &DeeSeq_DefaultCompareWithForeachDefault)
#define DeeType_IsDefaultIterNext(tp_iter_next) \
	((tp_iter_next) == &DeeObject_DefaultIterNextWithIterNextPair)
#define DeeType_IsDefaultIterNextPair(tp_nextpair) \
	((tp_nextpair) == &DeeObject_DefaultIterNextPairWithIterNext)
#define DeeType_IsDefaultIterNextKey(tp_nextkey) \
	((tp_nextkey) == &DeeObject_DefaultIterNextKeyWithIterNext || \
	 (tp_nextkey) == &DeeObject_DefaultIterNextKeyWithIterNextPair)
#define DeeType_IsDefaultIterNextValue(tp_nextvalue) \
	((tp_nextvalue) == &DeeObject_DefaultIterNextValueWithIterNext || \
	 (tp_nextvalue) == &DeeObject_DefaultIterNextValueWithIterNextPair)
#define DeeType_IsDefaultIterAdvance(tp_advance) \
	((tp_advance) == &DeeObject_DefaultIterAdvanceWithIterNext || \
	 (tp_advance) == &DeeObject_DefaultIterAdvanceWithIterNextPair || \
	 (tp_advance) == &DeeObject_DefaultIterAdvanceWithIterNextKey || \
	 (tp_advance) == &DeeObject_DefaultIterAdvanceWithIterNextValue)
#define DeeType_IsDefaultIter(tp_iter) \
	((tp_iter) == &DeeObject_DefaultIterWithForeach || \
	 (tp_iter) == &DeeObject_DefaultIterWithForeachPair || \
	 (tp_iter) == &DeeObject_DefaultIterWithEnumerate || \
	 (tp_iter) == &DeeObject_DefaultIterWithEnumerateIndex || \
	 (tp_iter) == &DeeObject_DefaultIterWithIterKeysAndTryGetItem || \
	 (tp_iter) == &DeeObject_DefaultIterWithIterKeysAndGetItem || \
	 (tp_iter) == &DeeObject_DefaultIterWithIterKeysAndTryGetItemDefault || \
	 (tp_iter) == &DeeSeq_DefaultIterWithSizeAndGetItemIndexFast || \
	 (tp_iter) == &DeeSeq_DefaultIterWithSizeAndTryGetItemIndex || \
	 (tp_iter) == &DeeSeq_DefaultIterWithSizeAndGetItemIndex || \
	 (tp_iter) == &DeeSeq_DefaultIterWithGetItemIndex || \
	 (tp_iter) == &DeeSeq_DefaultIterWithSizeObAndGetItem || \
	 (tp_iter) == &DeeSeq_DefaultIterWithGetItem || \
	 (tp_iter) == &DeeMap_DefaultIterWithEnumerate || \
	 (tp_iter) == &DeeMap_DefaultIterWithEnumerateIndex || \
	 (tp_iter) == &DeeMap_DefaultIterWithIterKeysAndTryGetItem || \
	 (tp_iter) == &DeeMap_DefaultIterWithIterKeysAndGetItem || \
	 (tp_iter) == &DeeMap_DefaultIterWithIterKeysAndTryGetItemDefault)
#define DeeType_IsDefaultForeach(tp_foreach) \
	((tp_foreach) == &DeeObject_DefaultForeachWithIter || \
	 (tp_foreach) == &DeeObject_DefaultForeachWithEnumerate || \
	 (tp_foreach) == &DeeObject_DefaultForeachWithEnumerateIndex || \
	 (tp_foreach) == &DeeObject_DefaultForeachWithForeachPair || \
	 (tp_foreach) == &DeeObject_DefaultForeachWithForeachPairDefault || \
	 (tp_foreach) == &DeeObject_DefaultForeachWithIterKeysAndTryGetItem || \
	 (tp_foreach) == &DeeObject_DefaultForeachWithIterKeysAndGetItem || \
	 (tp_foreach) == &DeeObject_DefaultForeachWithIterKeysAndTryGetItemDefault || \
	 (tp_foreach) == &DeeSeq_DefaultForeachWithSizeAndGetItemIndexFast || \
	 (tp_foreach) == &DeeSeq_DefaultForeachWithSizeAndTryGetItemIndex || \
	 (tp_foreach) == &DeeSeq_DefaultForeachWithSizeAndGetItemIndex || \
	 (tp_foreach) == &DeeSeq_DefaultForeachWithSizeObAndGetItem || \
	 (tp_foreach) == &DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault || \
	 (tp_foreach) == &DeeSeq_DefaultForeachWithGetItemIndexDefault)
#define DeeType_IsDefaultForeachPair(tp_foreach_pair) \
	((tp_foreach_pair) == &DeeObject_DefaultForeachPairWithForeach || \
	 (tp_foreach_pair) == &DeeObject_DefaultForeachPairWithEnumerate || \
	 (tp_foreach_pair) == &DeeObject_DefaultForeachPairWithEnumerateIndex || \
	 (tp_foreach_pair) == &DeeObject_DefaultForeachPairWithForeachDefault || \
	 (tp_foreach_pair) == &DeeObject_DefaultForeachPairWithIter || \
	 (tp_foreach_pair) == &DeeMap_DefaultForeachPairWithEnumerate || \
	 (tp_foreach_pair) == &DeeMap_DefaultForeachPairWithEnumerateIndex || \
	 (tp_foreach_pair) == &DeeMap_DefaultForeachPairWithEnumerateDefault)
#define DeeType_IsDefaultEnumerate(tp_enumerate) \
	((tp_enumerate) == &DeeObject_DefaultEnumerateWithEnumerateIndex || \
	 (tp_enumerate) == &DeeObject_DefaultEnumerateWithIterKeysAndTryGetItem || \
	 (tp_enumerate) == &DeeObject_DefaultEnumerateWithIterKeysAndGetItem || \
	 (tp_enumerate) == &DeeObject_DefaultEnumerateWithIterKeysAndTryGetItemDefault || \
	 (tp_enumerate) == &DeeSeq_DefaultEnumerateWithSizeAndGetItemIndexFast || \
	 (tp_enumerate) == &DeeSeq_DefaultEnumerateWithSizeAndTryGetItemIndex || \
	 (tp_enumerate) == &DeeSeq_DefaultEnumerateWithSizeAndGetItemIndex || \
	 (tp_enumerate) == &DeeSeq_DefaultEnumerateWithSizeObAndGetItem || \
	 (tp_enumerate) == &DeeSeq_DefaultEnumerateWithCounterAndForeach || \
	 (tp_enumerate) == &DeeSeq_DefaultEnumerateWithCounterAndIter || \
	 (tp_enumerate) == &DeeMap_DefaultEnumerateWithForeachPairDefault || \
	 (tp_enumerate) == &DeeMap_DefaultEnumerateWithIter || \
	 (tp_enumerate) == &DeeSeq_DefaultEnumerateWithSizeDefaultAndGetItemIndexDefault || \
	 (tp_enumerate) == &DeeSeq_DefaultEnumerateWithCounterAndForeachDefault)
#define DeeType_IsDefaultEnumerateIndex(tp_enumerate_index) \
	((tp_enumerate_index) == &DeeObject_DefaultEnumerateIndexWithEnumerate || \
	 (tp_enumerate_index) == &DeeSeq_DefaultEnumerateIndexWithSizeAndGetItemIndexFast || \
	 (tp_enumerate_index) == &DeeSeq_DefaultEnumerateIndexWithSizeAndTryGetItemIndex || \
	 (tp_enumerate_index) == &DeeSeq_DefaultEnumerateIndexWithSizeAndGetItemIndex || \
	 (tp_enumerate_index) == &DeeSeq_DefaultEnumerateIndexWithSizeObAndGetItem || \
	 (tp_enumerate_index) == &DeeSeq_DefaultEnumerateIndexWithCounterAndForeach || \
	 (tp_enumerate_index) == &DeeSeq_DefaultEnumerateIndexWithCounterAndIter || \
	 (tp_enumerate_index) == &DeeSeq_DefaultEnumerateIndexWithSizeDefaultAndGetItemIndexDefault || \
	 (tp_enumerate_index) == &DeeSeq_DefaultEnumerateIndexWithCounterAndForeachDefault || \
	 (tp_enumerate_index) == &DeeObject_DefaultEnumerateIndexWithEnumerateDefault)
#define DeeType_IsDefaultIterKeys(tp_iterkeys) \
	((tp_iterkeys) == &DeeObject_DefaultIterKeysWithEnumerate || \
	 (tp_iterkeys) == &DeeObject_DefaultIterKeysWithEnumerateIndex || \
	 (tp_iterkeys) == &DeeSeq_DefaultIterKeysWithSize || \
	 (tp_iterkeys) == &DeeSeq_DefaultIterKeysWithSizeOb || \
	 (tp_iterkeys) == &DeeSeq_DefaultIterKeysWithSizeDefault || \
	 (tp_iterkeys) == &DeeMap_DefaultIterKeysWithIter || \
	 (tp_iterkeys) == &DeeMap_DefaultIterKeysWithIterDefault)
#define DeeType_IsDefaultSizeOb(tp_sizeob) \
	((tp_sizeob) == &DeeObject_DefaultSizeObWithSize || \
	 (tp_sizeob) == &DeeObject_DefaultSizeObWithSizeDefault)
#define DeeType_IsDefaultSize(tp_size) \
	((tp_size) == &DeeObject_DefaultSizeWithSizeOb || \
	 (tp_size) == &DeeSeq_DefaultSizeWithEnumerateIndex || \
	 (tp_size) == &DeeSeq_DefaultSizeWithEnumerate || \
	 (tp_size) == &DeeSeq_DefaultSizeWithForeachPair || \
	 (tp_size) == &DeeSeq_DefaultSizeWithForeach || \
	 (tp_size) == &DeeSeq_DefaultSizeWithIter)
#define DeeType_IsDefaultSizeFast(tp_size_fast) \
	((tp_size_fast) == &DeeObject_DefaultSizeFastWithErrorNotFast)
#define DeeType_IsDefaultContains(tp_contains) \
	((tp_contains) == &DeeSeq_DefaultContainsWithForeachDefault || \
	 (tp_contains) == &DeeMap_DefaultContainsWithHasItem || \
	 (tp_contains) == &DeeMap_DefaultContainsWithBoundItem || \
	 (tp_contains) == &DeeMap_DefaultContainsWithTryGetItem || \
	 (tp_contains) == &DeeMap_DefaultContainsWithGetItem || \
	 (tp_contains) == &DeeMap_DefaultContainsWithHasItemStringHash || \
	 (tp_contains) == &DeeMap_DefaultContainsWithHasItemStringLenHash || \
	 (tp_contains) == &DeeMap_DefaultContainsWithHasItemIndex || \
	 (tp_contains) == &DeeMap_DefaultContainsWithBoundItemStringHash || \
	 (tp_contains) == &DeeMap_DefaultContainsWithBoundItemStringLenHash || \
	 (tp_contains) == &DeeMap_DefaultContainsWithBoundItemIndex || \
	 (tp_contains) == &DeeMap_DefaultContainsWithTryGetItemStringHash || \
	 (tp_contains) == &DeeMap_DefaultContainsWithTryGetItemStringLenHash || \
	 (tp_contains) == &DeeMap_DefaultContainsWithTryGetItemIndex || \
	 (tp_contains) == &DeeMap_DefaultContainsWithGetItemStringHash || \
	 (tp_contains) == &DeeMap_DefaultContainsWithGetItemStringLenHash || \
	 (tp_contains) == &DeeMap_DefaultContainsWithGetItemIndex || \
	 (tp_contains) == &DeeMap_DefaultContainsWithEnumerate || \
	 (tp_contains) == &DeeMap_DefaultContainsWithEnumerateDefault)
#define DeeType_IsDefaultGetItem(tp_getitem) \
	((tp_getitem) == &DeeObject_DefaultGetItemWithSizeAndGetItemIndexFast || \
	 (tp_getitem) == &DeeObject_DefaultGetItemWithGetItemIndex || \
	 (tp_getitem) == &DeeObject_DefaultGetItemWithGetItemStringHash || \
	 (tp_getitem) == &DeeObject_DefaultGetItemWithGetItemStringLenHash || \
	 (tp_getitem) == &DeeObject_DefaultGetItemWithTryGetItem || \
	 (tp_getitem) == &DeeObject_DefaultGetItemWithTryGetItemIndex || \
	 (tp_getitem) == &DeeObject_DefaultGetItemWithTryGetItemStringHash || \
	 (tp_getitem) == &DeeObject_DefaultGetItemWithTryGetItemStringLenHash || \
	 (tp_getitem) == &DeeObject_DefaultGetItemWithGetItemIndexDefault || \
	 (tp_getitem) == &DeeSeq_DefaultGetItemWithTryGetItemAndSizeOb || \
	 (tp_getitem) == &DeeSeq_DefaultGetItemWithTryGetItemAndSize || \
	 (tp_getitem) == &DeeSeq_DefaultGetItemWithSizeAndTryGetItemIndexOb || \
	 (tp_getitem) == &DeeSeq_DefaultGetItemWithSizeAndTryGetItemIndex || \
	 (tp_getitem) == &DeeMap_DefaultGetItemWithEnumerate || \
	 (tp_getitem) == &DeeMap_DefaultGetItemWithEnumerateDefault)
#define DeeType_IsDefaultGetItemIndex(tp_getitem_index) \
	((tp_getitem_index) == &DeeObject_DefaultGetItemIndexWithSizeAndGetItemIndexFast || \
	 (tp_getitem_index) == &DeeObject_DefaultGetItemIndexWithTryGetItemIndex || \
	 (tp_getitem_index) == &DeeObject_DefaultGetItemIndexWithGetItem || \
	 (tp_getitem_index) == &DeeObject_DefaultGetItemIndexWithTryGetItem || \
	 (tp_getitem_index) == &DeeObject_DefaultGetItemIndexWithErrorRequiresString || \
	 (tp_getitem_index) == &DeeObject_DefaultGetItemIndexWithGetItemDefault || \
	 (tp_getitem_index) == &DeeSeq_DefaultGetItemIndexWithSizeAndTryGetItemIndex || \
	 (tp_getitem_index) == &DeeSeq_DefaultGetItemIndexWithSizeAndTryGetItemIndexOb || \
	 (tp_getitem_index) == &DeeSeq_DefaultGetItemIndexWithTryGetItemAndSize || \
	 (tp_getitem_index) == &DeeSeq_DefaultGetItemIndexWithTryGetItemAndSizeOb || \
	 (tp_getitem_index) == &DeeSeq_DefaultGetItemIndexWithForeachDefault || \
	 (tp_getitem_index) == &DeeMap_DefaultGetItemIndexWithEnumerate || \
	 (tp_getitem_index) == &DeeMap_DefaultGetItemIndexWithEnumerateDefault)
#define DeeType_IsDefaultGetItemStringHash(tp_getitem_string_hash) \
	((tp_getitem_string_hash) == &DeeObject_DefaultGetItemStringHashWithGetItemStringLenHash || \
	 (tp_getitem_string_hash) == &DeeObject_DefaultGetItemStringHashWithGetItem || \
	 (tp_getitem_string_hash) == &DeeObject_DefaultGetItemStringHashWithTryGetItemStringHash || \
	 (tp_getitem_string_hash) == &DeeObject_DefaultGetItemStringHashWithTryGetItemStringLenHash || \
	 (tp_getitem_string_hash) == &DeeObject_DefaultGetItemStringHashWithTryGetItem || \
	 (tp_getitem_string_hash) == &DeeObject_DefaultGetItemStringHashWithErrorRequiresInt || \
	 (tp_getitem_string_hash) == &DeeObject_DefaultGetItemStringHashWithGetItemDefault || \
	 (tp_getitem_string_hash) == &DeeMap_DefaultGetItemStringHashWithEnumerate || \
	 (tp_getitem_string_hash) == &DeeMap_DefaultGetItemStringHashWithEnumerateDefault)
#define DeeType_IsDefaultGetItemStringLenHash(tp_getitem_string_len_hash) \
	((tp_getitem_string_len_hash) == &DeeObject_DefaultGetItemStringLenHashWithGetItemStringHash || \
	 (tp_getitem_string_len_hash) == &DeeObject_DefaultGetItemStringLenHashWithGetItem || \
	 (tp_getitem_string_len_hash) == &DeeObject_DefaultGetItemStringLenHashWithTryGetItemStringLenHash || \
	 (tp_getitem_string_len_hash) == &DeeObject_DefaultGetItemStringLenHashWithTryGetItemStringHash || \
	 (tp_getitem_string_len_hash) == &DeeObject_DefaultGetItemStringLenHashWithTryGetItem || \
	 (tp_getitem_string_len_hash) == &DeeObject_DefaultGetItemStringLenHashWithErrorRequiresInt || \
	 (tp_getitem_string_len_hash) == &DeeObject_DefaultGetItemStringLenHashWithGetItemDefault || \
	 (tp_getitem_string_len_hash) == &DeeMap_DefaultGetItemStringLenHashWithEnumerate || \
	 (tp_getitem_string_len_hash) == &DeeMap_DefaultGetItemStringLenHashWithEnumerateDefault)
#define DeeType_IsDefaultTryGetItem(tp_trygetitem) \
	((tp_trygetitem) == &DeeObject_DefaultTryGetItemWithTryGetItemIndex || \
	 (tp_trygetitem) == &DeeObject_DefaultTryGetItemWithTryGetItemStringHash || \
	 (tp_trygetitem) == &DeeObject_DefaultTryGetItemWithTryGetItemStringLenHash || \
	 (tp_trygetitem) == &DeeObject_DefaultTryGetItemWithSizeAndGetItemIndexFast || \
	 (tp_trygetitem) == &DeeObject_DefaultTryGetItemWithGetItemIndex || \
	 (tp_trygetitem) == &DeeObject_DefaultTryGetItemWithGetItem || \
	 (tp_trygetitem) == &DeeObject_DefaultTryGetItemWithGetItemStringHash || \
	 (tp_trygetitem) == &DeeObject_DefaultTryGetItemWithGetItemStringLenHash || \
	 (tp_trygetitem) == &DeeObject_DefaultTryGetItemWithGetItemDefault || \
	 (tp_trygetitem) == &DeeMap_DefaultTryGetItemWithEnumerate || \
	 (tp_trygetitem) == &DeeMap_DefaultTryGetItemWithEnumerateDefault)
#define DeeType_IsDefaultTryGetItemIndex(tp_trygetitem_index) \
	((tp_trygetitem_index) == &DeeObject_DefaultTryGetItemIndexWithSizeAndGetItemIndexFast || \
	 (tp_trygetitem_index) == &DeeObject_DefaultTryGetItemIndexWithTryGetItem || \
	 (tp_trygetitem_index) == &DeeObject_DefaultTryGetItemIndexWithGetItemIndex || \
	 (tp_trygetitem_index) == &DeeObject_DefaultTryGetItemIndexWithGetItem || \
	 (tp_trygetitem_index) == &DeeObject_DefaultTryGetItemIndexWithErrorRequiresString || \
	 (tp_trygetitem_index) == &DeeObject_DefaultTryGetItemIndexWithGetItemIndexDefault || \
	 (tp_trygetitem_index) == &DeeSeq_DefaultTryGetItemIndexWithForeachDefault || \
	 (tp_trygetitem_index) == &DeeMap_DefaultTryGetItemIndexWithEnumerate || \
	 (tp_trygetitem_index) == &DeeMap_DefaultTryGetItemIndexWithEnumerateDefault)
#define DeeType_IsDefaultTryGetItemStringHash(tp_trygetitem_string_hash) \
	((tp_trygetitem_string_hash) == &DeeObject_DefaultTryGetItemStringHashWithTryGetItemStringLenHash || \
	 (tp_trygetitem_string_hash) == &DeeObject_DefaultTryGetItemStringHashWithTryGetItem || \
	 (tp_trygetitem_string_hash) == &DeeObject_DefaultTryGetItemStringHashWithGetItemStringHash || \
	 (tp_trygetitem_string_hash) == &DeeObject_DefaultTryGetItemStringHashWithGetItemStringLenHash || \
	 (tp_trygetitem_string_hash) == &DeeObject_DefaultTryGetItemStringHashWithGetItem || \
	 (tp_trygetitem_string_hash) == &DeeObject_DefaultTryGetItemStringHashWithErrorRequiresInt || \
	 (tp_trygetitem_string_hash) == &DeeObject_DefaultTryGetItemStringHashWithTryGetItemDefault || \
	 (tp_trygetitem_string_hash) == &DeeMap_DefaultTryGetItemStringHashWithEnumerate || \
	 (tp_trygetitem_string_hash) == &DeeMap_DefaultTryGetItemStringHashWithEnumerateDefault)
#define DeeType_IsDefaultTryGetItemStringLenHash(tp_trygetitem_string_len_hash) \
	((tp_trygetitem_string_len_hash) == &DeeObject_DefaultTryGetItemStringLenHashWithTryGetItemStringHash || \
	 (tp_trygetitem_string_len_hash) == &DeeObject_DefaultTryGetItemStringLenHashWithTryGetItem || \
	 (tp_trygetitem_string_len_hash) == &DeeObject_DefaultTryGetItemStringLenHashWithGetItemStringLenHash || \
	 (tp_trygetitem_string_len_hash) == &DeeObject_DefaultTryGetItemStringLenHashWithGetItemStringHash || \
	 (tp_trygetitem_string_len_hash) == &DeeObject_DefaultTryGetItemStringLenHashWithGetItem || \
	 (tp_trygetitem_string_len_hash) == &DeeObject_DefaultTryGetItemStringLenHashWithErrorRequiresInt || \
	 (tp_trygetitem_string_len_hash) == &DeeObject_DefaultTryGetItemStringLenHashWithTryGetItemDefault || \
	 (tp_trygetitem_string_len_hash) == &DeeMap_DefaultTryGetItemStringLenHashWithEnumerate || \
	 (tp_trygetitem_string_len_hash) == &DeeMap_DefaultTryGetItemStringLenHashWithEnumerateDefault)
#define DeeType_IsDefaultDelItem(tp_delitem) \
	((tp_delitem) == &DeeObject_DefaultDelItemWithDelItemIndex || \
	 (tp_delitem) == &DeeObject_DefaultDelItemWithDelItemIndexDefault || \
	 (tp_delitem) == &DeeObject_DefaultDelItemWithDelItemStringHash || \
	 (tp_delitem) == &DeeObject_DefaultDelItemWithDelItemStringLenHash)
#define DeeType_IsDefaultDelItemIndex(tp_delitem_index) \
	((tp_delitem_index) == &DeeObject_DefaultDelItemIndexWithDelItem || \
	 (tp_delitem_index) == &DeeObject_DefaultDelItemIndexWithErrorRequiresString || \
	 (tp_delitem_index) == &DeeSeq_DefaultDelItemIndexWithDelRangeIndexDefault)
#define DeeType_IsDefaultDelItemStringHash(tp_delitem_string_hash) \
	((tp_delitem_string_hash) == &DeeObject_DefaultDelItemStringHashWithDelItemStringLenHash || \
	 (tp_delitem_string_hash) == &DeeObject_DefaultDelItemStringHashWithDelItem || \
	 (tp_delitem_string_hash) == &DeeObject_DefaultDelItemStringHashWithErrorRequiresInt)
#define DeeType_IsDefaultDelItemStringLenHash(tp_delitem_string_len_hash) \
	((tp_delitem_string_len_hash) == &DeeObject_DefaultDelItemStringLenHashWithDelItemStringHash || \
	 (tp_delitem_string_len_hash) == &DeeObject_DefaultDelItemStringLenHashWithDelItem || \
	 (tp_delitem_string_len_hash) == &DeeObject_DefaultDelItemStringLenHashWithErrorRequiresInt)
#define DeeType_IsDefaultSetItem(tp_setitem) \
	((tp_setitem) == &DeeObject_DefaultSetItemWithSetItemIndex || \
	 (tp_setitem) == &DeeObject_DefaultSetItemWithSetItemIndexDefault || \
	 (tp_setitem) == &DeeObject_DefaultSetItemWithSetItemStringHash || \
	 (tp_setitem) == &DeeObject_DefaultSetItemWithSetItemStringLenHash)
#define DeeType_IsDefaultSetItemIndex(tp_setitem_index) \
	((tp_setitem_index) == &DeeObject_DefaultSetItemIndexWithSetItem || \
	 (tp_setitem_index) == &DeeObject_DefaultSetItemIndexWithErrorRequiresString || \
	 (tp_setitem_index) == &DeeSeq_DefaultSetItemIndexWithSetRangeIndexDefault)
#define DeeType_IsDefaultSetItemStringHash(tp_setitem_string_hash) \
	((tp_setitem_string_hash) == &DeeObject_DefaultSetItemStringHashWithSetItemStringLenHash || \
	 (tp_setitem_string_hash) == &DeeObject_DefaultSetItemStringHashWithSetItem || \
	 (tp_setitem_string_hash) == &DeeObject_DefaultSetItemStringHashWithErrorRequiresInt)
#define DeeType_IsDefaultSetItemStringLenHash(tp_setitem_string_len_hash) \
	((tp_setitem_string_len_hash) == &DeeObject_DefaultSetItemStringLenHashWithSetItemStringHash || \
	 (tp_setitem_string_len_hash) == &DeeObject_DefaultSetItemStringLenHashWithSetItem || \
	 (tp_setitem_string_len_hash) == &DeeObject_DefaultSetItemStringLenHashWithErrorRequiresInt)
#define DeeType_IsDefaultBoundItem(tp_bounditem) \
	((tp_bounditem) == &DeeObject_DefaultBoundItemWithBoundItemIndex || \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithBoundItemStringHash || \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithBoundItemStringLenHash || \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithGetItem || \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithGetItemStringHash || \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithGetItemStringLenHash || \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithGetItemIndex || \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithTryGetItemAndHasItem || \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithTryGetItemIndexAndHasItemIndex || \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithTryGetItemStringLenHashAndHasItemStringLenHash || \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithTryGetItemStringHashAndHasItemStringHash || \
	 (tp_bounditem) == &DeeSeq_DefaultBoundItemWithTryGetItemAndSizeOb || \
	 (tp_bounditem) == &DeeSeq_DefaultBoundItemWithSizeAndTryGetItemIndex || \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithTryGetItem || \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithTryGetItemIndex || \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithTryGetItemStringLenHash || \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithTryGetItemStringHash || \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithGetItemDefault || \
	 (tp_bounditem) == &DeeMap_DefaultBoundItemWithContains || \
	 (tp_bounditem) == &DeeMap_DefaultBoundItemWithEnumerate || \
	 (tp_bounditem) == &DeeMap_DefaultBoundItemWithEnumerateDefault)
#define DeeType_IsDefaultBoundItemIndex(tp_bounditem_index) \
	((tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithBoundItem || \
	 (tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithSizeAndGetItemIndexFast || \
	 (tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithGetItemIndex || \
	 (tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithGetItem || \
	 (tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithGetItemIndexDefault || \
	 (tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithTryGetItemIndexAndHasItemIndex || \
	 (tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithTryGetItemAndHasItem || \
	 (tp_bounditem_index) == &DeeSeq_DefaultBoundItemIndexWithSizeAndTryGetItemIndex || \
	 (tp_bounditem_index) == &DeeSeq_DefaultBoundItemIndexWithTryGetItemAndSizeOb || \
	 (tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithErrorRequiresString || \
	 (tp_bounditem_index) == &DeeMap_DefaultBoundItemIndexWithContains || \
	 (tp_bounditem_index) == &DeeMap_DefaultBoundItemIndexWithEnumerate || \
	 (tp_bounditem_index) == &DeeMap_DefaultBoundItemIndexWithEnumerateDefault)
#define DeeType_IsDefaultBoundItemStringHash(tp_bounditem_string_hash) \
	((tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithBoundItemStringLenHash || \
	 (tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithBoundItem || \
	 (tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithGetItemStringHash || \
	 (tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithGetItemStringLenHash || \
	 (tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithGetItem || \
	 (tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithTryGetItemStringHashAndHasItemStringHash || \
	 (tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithTryGetItemStringLenHashAndHasItemStringLenHash || \
	 (tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithTryGetItemAndHasItem || \
	 (tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithTryGetItemStringHash || \
	 (tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithTryGetItemStringLenHash || \
	 (tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithTryGetItem || \
	 (tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithBoundItemDefault || \
	 (tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithErrorRequiresInt || \
	 (tp_bounditem_string_hash) == &DeeMap_DefaultBoundItemStringHashWithContains || \
	 (tp_bounditem_string_hash) == &DeeMap_DefaultBoundItemStringHashWithEnumerate || \
	 (tp_bounditem_string_hash) == &DeeMap_DefaultBoundItemStringHashWithEnumerateDefault)
#define DeeType_IsDefaultBoundItemStringLenHash(tp_bounditem_string_len_hash) \
	((tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithBoundItemStringHash || \
	 (tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithBoundItem || \
	 (tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithGetItemStringLenHash || \
	 (tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithGetItemStringHash || \
	 (tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithGetItem || \
	 (tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemStringLenHashAndHasItemStringLenHash || \
	 (tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemStringHashAndHasItemStringHash || \
	 (tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemAndHasItem || \
	 (tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemStringLenHash || \
	 (tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemStringHash || \
	 (tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithTryGetItem || \
	 (tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithBoundItemDefault || \
	 (tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithErrorRequiresInt || \
	 (tp_bounditem_string_len_hash) == &DeeMap_DefaultBoundItemStringLenHashWithContains || \
	 (tp_bounditem_string_len_hash) == &DeeMap_DefaultBoundItemStringLenHashWithEnumerate || \
	 (tp_bounditem_string_len_hash) == &DeeMap_DefaultBoundItemStringLenHashWithEnumerateDefault)
#define DeeType_IsDefaultHasItem(tp_hasitem) \
	((tp_hasitem) == &DeeObject_DefaultHasItemWithHasItemStringHash || \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithHasItemStringLenHash || \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithHasItemIndex || \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithBoundItem || \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithBoundItemStringHash || \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithBoundItemStringLenHash || \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithBoundItemIndex || \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithTryGetItem || \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithTryGetItemStringHash || \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithTryGetItemStringLenHash || \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithTryGetItemIndex || \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithGetItem || \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithGetItemStringHash || \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithGetItemStringLenHash || \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithGetItemIndex || \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithGetItemDefault || \
	 (tp_hasitem) == &DeeSeq_DefaultHasItemWithSize || \
	 (tp_hasitem) == &DeeSeq_DefaultHasItemWithSizeOb || \
	 (tp_hasitem) == &DeeMap_DefaultHasItemWithContains || \
	 (tp_hasitem) == &DeeMap_DefaultHasItemWithEnumerate || \
	 (tp_hasitem) == &DeeMap_DefaultHasItemWithEnumerateDefault)
#define DeeType_IsDefaultHasItemIndex(tp_hasitem_index) \
	((tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithHasItem || \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithBoundItemIndex || \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithBoundItem || \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithTryGetItemIndex || \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithTryGetItem || \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithGetItemIndex || \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithGetItem || \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithGetItemIndexDefault || \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithErrorRequiresString || \
	 (tp_hasitem_index) == &DeeSeq_DefaultHasItemIndexWithSize || \
	 (tp_hasitem_index) == &DeeSeq_DefaultHasItemIndexWithSizeDefault || \
	 (tp_hasitem_index) == &DeeMap_DefaultHasItemIndexWithContains || \
	 (tp_hasitem_index) == &DeeMap_DefaultHasItemIndexWithEnumerate || \
	 (tp_hasitem_index) == &DeeMap_DefaultHasItemIndexWithEnumerateDefault)
#define DeeType_IsDefaultHasItemStringHash(tp_hasitem_string_hash) \
	((tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithHasItemStringLenHash || \
	 (tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithBoundItemStringHash || \
	 (tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithBoundItemStringLenHash || \
	 (tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithTryGetItemStringHash || \
	 (tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithTryGetItemStringLenHash || \
	 (tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithGetItemStringHash || \
	 (tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithGetItemStringLenHash || \
	 (tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithHasItem || \
	 (tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithBoundItem || \
	 (tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithTryGetItem || \
	 (tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithGetItem || \
	 (tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithHasItemDefault || \
	 (tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithErrorRequiresInt || \
	 (tp_hasitem_string_hash) == &DeeMap_DefaultHasItemStringHashWithContains || \
	 (tp_hasitem_string_hash) == &DeeMap_DefaultHasItemStringHashWithEnumerate || \
	 (tp_hasitem_string_hash) == &DeeMap_DefaultHasItemStringHashWithEnumerateDefault)
#define DeeType_IsDefaultHasItemStringLenHash(tp_hasitem_string_len_hash) \
	((tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithHasItemStringHash || \
	 (tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithBoundItemStringHash || \
	 (tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithBoundItemStringLenHash || \
	 (tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithTryGetItemStringLenHash || \
	 (tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithTryGetItemStringHash || \
	 (tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithGetItemStringLenHash || \
	 (tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithGetItemStringHash || \
	 (tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithHasItem || \
	 (tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithBoundItem || \
	 (tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithTryGetItem || \
	 (tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithGetItem || \
	 (tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithHasItemDefault || \
	 (tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithErrorRequiresInt || \
	 (tp_hasitem_string_len_hash) == &DeeMap_DefaultHasItemStringLenHashWithContains || \
	 (tp_hasitem_string_len_hash) == &DeeMap_DefaultHasItemStringLenHashWithEnumerate || \
	 (tp_hasitem_string_len_hash) == &DeeMap_DefaultHasItemStringLenHashWithEnumerateDefault)
#define DeeType_IsDefaultGetRange(tp_getrange) \
	((tp_getrange) == &DeeObject_DefaultGetRangeWithGetRangeIndexAndGetRangeIndexN || \
	 (tp_getrange) == &DeeObject_DefaultGetRangeWithGetRangeIndexDefaultAndGetRangeIndexNDefault || \
	 (tp_getrange) == &DeeSeq_DefaultGetRangeWithSizeDefaultAndGetItemIndex || \
	 (tp_getrange) == &DeeSeq_DefaultGetRangeWithSizeDefaultAndTryGetItemIndex || \
	 (tp_getrange) == &DeeSeq_DefaultGetRangeWithSizeObAndGetItem)
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
	 (tp_delrange_index) == &DeeSeq_DefaultDelRangeIndexWithSetRangeIndexNoneDefault || \
	 (tp_delrange_index) == &DeeSeq_DefaultDelRangeIndexWithSizeDefaultAndTSCErase || \
	 (tp_delrange_index) == &DeeSeq_DefaultDelRangeIndexWithSizeDefaultAndDelItemIndexDefault)
#define DeeType_IsDefaultDelRangeIndexN(tp_delrange_index_n) \
	((tp_delrange_index_n) == &DeeObject_DefaultDelRangeIndexNWithDelRange || \
	 (tp_delrange_index_n) == &DeeSeq_DefaultDelRangeIndexNWithSizeAndDelRangeIndex || \
	 (tp_delrange_index_n) == &DeeSeq_DefaultDelRangeIndexNWithSizeDefaultAndDelRangeIndex || \
	 (tp_delrange_index_n) == &DeeSeq_DefaultDelRangeIndexNWithSetRangeIndexNNone || \
	 (tp_delrange_index_n) == &DeeSeq_DefaultDelRangeIndexNWithSetRangeIndexNNoneDefault || \
	 (tp_delrange_index_n) == &DeeSeq_DefaultDelRangeIndexNWithSizeDefaultAndTSCErase || \
	 (tp_delrange_index_n) == &DeeSeq_DefaultDelRangeIndexNWithSizeDefaultAndDelItemIndexDefault)
#define DeeType_IsDefaultSetRange(tp_setrange) \
	((tp_setrange) == &DeeObject_DefaultSetRangeWithSetRangeIndexAndSetRangeIndexN || \
	 (tp_setrange) == &DeeObject_DefaultSetRangeWithSetRangeIndexDefaultAndSetRangeIndexNDefault)
#define DeeType_IsDefaultSetRangeIndex(tp_setrange_index) \
	((tp_setrange_index) == &DeeObject_DefaultSetRangeIndexWithSetRange || \
	 (tp_setrange_index) == &DeeSeq_DefaultSetRangeIndexWithSizeDefaultAndTSCEraseAndTSCInsertAll)
#define DeeType_IsDefaultSetRangeIndexN(tp_setrange_index_n) \
	((tp_setrange_index_n) == &DeeObject_DefaultSetRangeIndexNWithSetRange || \
	 (tp_setrange_index_n) == &DeeSeq_DefaultSetRangeIndexNWithSizeAndSetRangeIndex || \
	 (tp_setrange_index_n) == &DeeSeq_DefaultSetRangeIndexNWithSizeDefaultAndSetRangeIndex || \
	 (tp_setrange_index_n) == &DeeSeq_DefaultSetRangeIndexNWithSizeDefaultAndSetRangeIndexDefault)
#define DeeType_IsDefaultUnpack(tp_unpack) \
	((tp_unpack) == &DeeSeq_DefaultUnpackWithUnpackEx || \
	 (tp_unpack) == &DeeSeq_DefaultUnpackWithAsVector || \
	 (tp_unpack) == &DeeSeq_DefaultUnpackWithSizeAndGetItemIndexFast || \
	 (tp_unpack) == &DeeSeq_DefaultUnpackWithSizeAndTryGetItemIndex || \
	 (tp_unpack) == &DeeSeq_DefaultUnpackWithSizeAndGetItemIndex || \
	 (tp_unpack) == &DeeSeq_DefaultUnpackWithSizeDefaultAndTryGetItemIndexDefault || \
	 (tp_unpack) == &DeeSeq_DefaultUnpackWithSizeDefaultAndGetItemIndexDefault || \
	 (tp_unpack) == &DeeSeq_DefaultUnpackWithForeach || \
	 (tp_unpack) == &DeeSeq_DefaultUnpackWithIter || \
	 (tp_unpack) == &DeeSeq_DefaultUnpackWithForeachDefault)
#define DeeType_IsDefaultUnpackUb(tp_unpack_ub) \
	((tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithSizeAndGetItemIndexFast || \
	 (tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithSizeAndTryGetItemIndex || \
	 (tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithSizeAndGetItemIndex || \
	 (tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithSizeDefaultAndTryGetItemIndexDefault || \
	 (tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithSizeDefaultAndGetItemIndexDefault || \
	 (tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithSizeDefaultAndEnumerateIndex || \
	 (tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithSizeDefaultAndEnumerateIndexDefault || \
	 (tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithUnpackEx || \
	 (tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithAsVector || \
	 (tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithForeach || \
	 (tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithIter || \
	 (tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithForeachDefault)
#define DeeType_IsDefaultUnpackEx(tp_unpack_ex) \
	((tp_unpack_ex) == &DeeSeq_DefaultUnpackExWithAsVector || \
	 (tp_unpack_ex) == &DeeSeq_DefaultUnpackExWithSizeAndGetItemIndexFast || \
	 (tp_unpack_ex) == &DeeSeq_DefaultUnpackExWithSizeAndTryGetItemIndex || \
	 (tp_unpack_ex) == &DeeSeq_DefaultUnpackExWithSizeAndGetItemIndex || \
	 (tp_unpack_ex) == &DeeSeq_DefaultUnpackExWithSizeDefaultAndTryGetItemIndexDefault || \
	 (tp_unpack_ex) == &DeeSeq_DefaultUnpackExWithSizeDefaultAndGetItemIndexDefault || \
	 (tp_unpack_ex) == &DeeSeq_DefaultUnpackExWithForeach || \
	 (tp_unpack_ex) == &DeeSeq_DefaultUnpackExWithIter || \
	 (tp_unpack_ex) == &DeeSeq_DefaultUnpackExWithForeachDefault)
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
#define DeeType_invoke_cast_tp_bool_DEFAULT(tp_self, tp_bool, self, default) \
	((tp_bool) == &DeeSeq_DefaultBoolWithSize ? DeeSeq_TDefaultBoolWithSize(tp_self, self) : \
	 (tp_bool) == &DeeSeq_DefaultBoolWithSizeOb ? DeeSeq_TDefaultBoolWithSizeOb(tp_self, self) : \
	 (tp_bool) == &DeeSeq_DefaultBoolWithForeach ? DeeSeq_TDefaultBoolWithForeach(tp_self, self) : \
	 (tp_bool) == &DeeSeq_DefaultBoolWithCompareEq ? DeeSeq_TDefaultBoolWithCompareEq(tp_self, self) : \
	 (tp_bool) == &DeeSeq_DefaultBoolWithEq ? DeeSeq_TDefaultBoolWithEq(tp_self, self) : \
	 (tp_bool) == &DeeSeq_DefaultBoolWithNe ? DeeSeq_TDefaultBoolWithNe(tp_self, self) : \
	 (tp_bool) == &DeeSeq_DefaultBoolWithForeachDefault ? DeeSeq_TDefaultBoolWithForeachDefault(tp_self, self) : \
	 default)
#define DeeType_invoke_cmp_tp_hash_DEFAULT(tp_self, tp_hash, self, default) \
	((tp_hash) == &DeeSeq_DefaultHashWithSizeAndGetItemIndexFast ? DeeSeq_TDefaultHashWithSizeAndGetItemIndexFast(tp_self, self) : \
	 (tp_hash) == &DeeSeq_DefaultHashWithForeach ? DeeSeq_TDefaultHashWithForeach(tp_self, self) : \
	 (tp_hash) == &DeeSeq_DefaultHashWithSizeAndTryGetItemIndex ? DeeSeq_TDefaultHashWithSizeAndTryGetItemIndex(tp_self, self) : \
	 (tp_hash) == &DeeSeq_DefaultHashWithSizeAndGetItemIndex ? DeeSeq_TDefaultHashWithSizeAndGetItemIndex(tp_self, self) : \
	 (tp_hash) == &DeeSeq_DefaultHashWithSizeObAndGetItem ? DeeSeq_TDefaultHashWithSizeObAndGetItem(tp_self, self) : \
	 (tp_hash) == &DeeSeq_DefaultHashWithForeachDefault ? DeeSeq_TDefaultHashWithForeachDefault(tp_self, self) : \
	 (tp_hash) == &DeeSet_DefaultHashWithForeachDefault ? DeeSet_TDefaultHashWithForeachDefault(tp_self, self) : \
	 (tp_hash) == &DeeMap_DefaultHashWithForeachPairDefault ? DeeMap_TDefaultHashWithForeachPairDefault(tp_self, self) : \
	 default)
#define DeeType_invoke_cmp_tp_eq_DEFAULT(tp_self, tp_eq, self, other, default) \
	((tp_eq) == &DeeObject_DefaultEqWithCompareEq ? DeeObject_TDefaultEqWithCompareEq(tp_self, self, other) : \
	 (tp_eq) == &DeeObject_DefaultEqWithNe ? DeeObject_TDefaultEqWithNe(tp_self, self, other) : \
	 (tp_eq) == &DeeObject_DefaultEqWithLoAndGr ? DeeObject_TDefaultEqWithLoAndGr(tp_self, self, other) : \
	 (tp_eq) == &DeeObject_DefaultEqWithLeAndGe ? DeeObject_TDefaultEqWithLeAndGe(tp_self, self, other) : \
	 (tp_eq) == &DeeObject_DefaultEqWithCompareEqDefault ? DeeObject_TDefaultEqWithCompareEqDefault(tp_self, self, other) : \
	 default)
#define DeeType_invoke_cmp_tp_ne_DEFAULT(tp_self, tp_ne, self, other, default) \
	((tp_ne) == &DeeObject_DefaultNeWithCompareEq ? DeeObject_TDefaultNeWithCompareEq(tp_self, self, other) : \
	 (tp_ne) == &DeeObject_DefaultNeWithEq ? DeeObject_TDefaultNeWithEq(tp_self, self, other) : \
	 (tp_ne) == &DeeObject_DefaultNeWithLoAndGr ? DeeObject_TDefaultNeWithLoAndGr(tp_self, self, other) : \
	 (tp_ne) == &DeeObject_DefaultNeWithLeAndGe ? DeeObject_TDefaultNeWithLeAndGe(tp_self, self, other) : \
	 (tp_ne) == &DeeObject_DefaultNeWithCompareEqDefault ? DeeObject_TDefaultNeWithCompareEqDefault(tp_self, self, other) : \
	 default)
#define DeeType_invoke_cmp_tp_lo_DEFAULT(tp_self, tp_lo, self, other, default) \
	((tp_lo) == &DeeObject_DefaultLoWithCompare ? DeeObject_TDefaultLoWithCompare(tp_self, self, other) : \
	 (tp_lo) == &DeeObject_DefaultLoWithGe ? DeeObject_TDefaultLoWithGe(tp_self, self, other) : \
	 (tp_lo) == &DeeObject_DefaultLoWithCompareDefault ? DeeObject_TDefaultLoWithCompareDefault(tp_self, self, other) : \
	 (tp_lo) == &DeeSet_DefaultLoWithForeachDefault ? DeeSet_TDefaultLoWithForeachDefault(tp_self, self, other) : \
	 (tp_lo) == &DeeMap_DefaultLoWithForeachPairDefault ? DeeMap_TDefaultLoWithForeachPairDefault(tp_self, self, other) : \
	 default)
#define DeeType_invoke_cmp_tp_le_DEFAULT(tp_self, tp_le, self, other, default) \
	((tp_le) == &DeeObject_DefaultLeWithCompare ? DeeObject_TDefaultLeWithCompare(tp_self, self, other) : \
	 (tp_le) == &DeeObject_DefaultLeWithGr ? DeeObject_TDefaultLeWithGr(tp_self, self, other) : \
	 (tp_le) == &DeeObject_DefaultLeWithCompareDefault ? DeeObject_TDefaultLeWithCompareDefault(tp_self, self, other) : \
	 (tp_le) == &DeeSet_DefaultLeWithForeachDefault ? DeeSet_TDefaultLeWithForeachDefault(tp_self, self, other) : \
	 (tp_le) == &DeeMap_DefaultLeWithForeachPairDefault ? DeeMap_TDefaultLeWithForeachPairDefault(tp_self, self, other) : \
	 default)
#define DeeType_invoke_cmp_tp_gr_DEFAULT(tp_self, tp_gr, self, other, default) \
	((tp_gr) == &DeeObject_DefaultGrWithCompare ? DeeObject_TDefaultGrWithCompare(tp_self, self, other) : \
	 (tp_gr) == &DeeObject_DefaultGrWithLe ? DeeObject_TDefaultGrWithLe(tp_self, self, other) : \
	 (tp_gr) == &DeeObject_DefaultGrWithCompareDefault ? DeeObject_TDefaultGrWithCompareDefault(tp_self, self, other) : \
	 (tp_gr) == &DeeSet_DefaultGrWithForeachDefault ? DeeSet_TDefaultGrWithForeachDefault(tp_self, self, other) : \
	 (tp_gr) == &DeeMap_DefaultGrWithForeachPairDefault ? DeeMap_TDefaultGrWithForeachPairDefault(tp_self, self, other) : \
	 default)
#define DeeType_invoke_cmp_tp_ge_DEFAULT(tp_self, tp_ge, self, other, default) \
	((tp_ge) == &DeeObject_DefaultGeWithCompare ? DeeObject_TDefaultGeWithCompare(tp_self, self, other) : \
	 (tp_ge) == &DeeObject_DefaultGeWithLo ? DeeObject_TDefaultGeWithLo(tp_self, self, other) : \
	 (tp_ge) == &DeeObject_DefaultGeWithCompareDefault ? DeeObject_TDefaultGeWithCompareDefault(tp_self, self, other) : \
	 (tp_ge) == &DeeSet_DefaultGeWithForeachDefault ? DeeSet_TDefaultGeWithForeachDefault(tp_self, self, other) : \
	 (tp_ge) == &DeeMap_DefaultGeWithForeachPairDefault ? DeeMap_TDefaultGeWithForeachPairDefault(tp_self, self, other) : \
	 default)
#define DeeType_invoke_cmp_tp_compare_eq_DEFAULT(tp_self, tp_compare_eq, self, other, default) \
	((tp_compare_eq) == &DeeObject_DefaultCompareEqWithEq ? DeeObject_TDefaultCompareEqWithEq(tp_self, self, other) : \
	 (tp_compare_eq) == &DeeObject_DefaultCompareEqWithNe ? DeeObject_TDefaultCompareEqWithNe(tp_self, self, other) : \
	 (tp_compare_eq) == &DeeObject_DefaultCompareEqWithLoAndGr ? DeeObject_TDefaultCompareEqWithLoAndGr(tp_self, self, other) : \
	 (tp_compare_eq) == &DeeObject_DefaultCompareEqWithLeAndGe ? DeeObject_TDefaultCompareEqWithLeAndGe(tp_self, self, other) : \
	 (tp_compare_eq) == &DeeSeq_DefaultCompareEqWithForeachDefault ? DeeSeq_TDefaultCompareEqWithForeachDefault(tp_self, self, other) : \
	 (tp_compare_eq) == &DeeSeq_DefaultCompareEqWithSizeAndGetItemIndexFast ? DeeSeq_TDefaultCompareEqWithSizeAndGetItemIndexFast(tp_self, self, other) : \
	 (tp_compare_eq) == &DeeSeq_DefaultCompareEqWithSizeAndTryGetItemIndex ? DeeSeq_TDefaultCompareEqWithSizeAndTryGetItemIndex(tp_self, self, other) : \
	 (tp_compare_eq) == &DeeSeq_DefaultCompareEqWithSizeAndGetItemIndex ? DeeSeq_TDefaultCompareEqWithSizeAndGetItemIndex(tp_self, self, other) : \
	 (tp_compare_eq) == &DeeSeq_DefaultCompareEqWithSizeObAndGetItem ? DeeSeq_TDefaultCompareEqWithSizeObAndGetItem(tp_self, self, other) : \
	 (tp_compare_eq) == &DeeSet_DefaultCompareEqWithForeachDefault ? DeeSet_TDefaultCompareEqWithForeachDefault(tp_self, self, other) : \
	 (tp_compare_eq) == &DeeMap_DefaultCompareEqWithForeachPairDefault ? DeeMap_TDefaultCompareEqWithForeachPairDefault(tp_self, self, other) : \
	 default)
#define DeeType_invoke_cmp_tp_trycompare_eq_DEFAULT(tp_self, tp_trycompare_eq, self, other, default) \
	((tp_trycompare_eq) == &DeeObject_DefaultTryCompareEqWithCompareEq ? DeeObject_TDefaultTryCompareEqWithCompareEq(tp_self, self, other) : \
	 (tp_trycompare_eq) == &DeeObject_DefaultTryCompareEqWithEq ? DeeObject_TDefaultTryCompareEqWithEq(tp_self, self, other) : \
	 (tp_trycompare_eq) == &DeeObject_DefaultTryCompareEqWithNe ? DeeObject_TDefaultTryCompareEqWithNe(tp_self, self, other) : \
	 (tp_trycompare_eq) == &DeeObject_DefaultTryCompareEqWithCompare ? DeeObject_TDefaultTryCompareEqWithCompare(tp_self, self, other) : \
	 (tp_trycompare_eq) == &DeeObject_DefaultTryCompareEqWithLoAndGr ? DeeObject_TDefaultTryCompareEqWithLoAndGr(tp_self, self, other) : \
	 (tp_trycompare_eq) == &DeeObject_DefaultTryCompareEqWithLeAndGe ? DeeObject_TDefaultTryCompareEqWithLeAndGe(tp_self, self, other) : \
	 (tp_trycompare_eq) == &DeeSeq_DefaultTryCompareEqWithForeachDefault ? DeeSeq_TDefaultTryCompareEqWithForeachDefault(tp_self, self, other) : \
	 (tp_trycompare_eq) == &DeeSeq_DefaultTryCompareEqWithSizeAndGetItemIndexFast ? DeeSeq_TDefaultTryCompareEqWithSizeAndGetItemIndexFast(tp_self, self, other) : \
	 (tp_trycompare_eq) == &DeeSeq_DefaultTryCompareEqWithSizeAndTryGetItemIndex ? DeeSeq_TDefaultTryCompareEqWithSizeAndTryGetItemIndex(tp_self, self, other) : \
	 (tp_trycompare_eq) == &DeeSeq_DefaultTryCompareEqWithSizeAndGetItemIndex ? DeeSeq_TDefaultTryCompareEqWithSizeAndGetItemIndex(tp_self, self, other) : \
	 (tp_trycompare_eq) == &DeeSeq_DefaultTryCompareEqWithSizeObAndGetItem ? DeeSeq_TDefaultTryCompareEqWithSizeObAndGetItem(tp_self, self, other) : \
	 (tp_trycompare_eq) == &DeeSet_DefaultTryCompareEqWithForeachDefault ? DeeSet_TDefaultTryCompareEqWithForeachDefault(tp_self, self, other) : \
	 (tp_trycompare_eq) == &DeeMap_DefaultTryCompareEqWithForeachPairDefault ? DeeMap_TDefaultTryCompareEqWithForeachPairDefault(tp_self, self, other) : \
	 default)
#define DeeType_invoke_cmp_tp_compare_DEFAULT(tp_self, tp_compare, self, other, default) \
	((tp_compare) == &DeeObject_DefaultCompareWithEqAndLo ? DeeObject_TDefaultCompareWithEqAndLo(tp_self, self, other) : \
	 (tp_compare) == &DeeObject_DefaultCompareWithEqAndLe ? DeeObject_TDefaultCompareWithEqAndLe(tp_self, self, other) : \
	 (tp_compare) == &DeeObject_DefaultCompareWithEqAndGr ? DeeObject_TDefaultCompareWithEqAndGr(tp_self, self, other) : \
	 (tp_compare) == &DeeObject_DefaultCompareWithEqAndGe ? DeeObject_TDefaultCompareWithEqAndGe(tp_self, self, other) : \
	 (tp_compare) == &DeeObject_DefaultCompareWithNeAndLo ? DeeObject_TDefaultCompareWithNeAndLo(tp_self, self, other) : \
	 (tp_compare) == &DeeObject_DefaultCompareWithNeAndLe ? DeeObject_TDefaultCompareWithNeAndLe(tp_self, self, other) : \
	 (tp_compare) == &DeeObject_DefaultCompareWithNeAndGr ? DeeObject_TDefaultCompareWithNeAndGr(tp_self, self, other) : \
	 (tp_compare) == &DeeObject_DefaultCompareWithNeAndGe ? DeeObject_TDefaultCompareWithNeAndGe(tp_self, self, other) : \
	 (tp_compare) == &DeeObject_DefaultCompareWithLoAndGr ? DeeObject_TDefaultCompareWithLoAndGr(tp_self, self, other) : \
	 (tp_compare) == &DeeObject_DefaultCompareWithLeAndGe ? DeeObject_TDefaultCompareWithLeAndGe(tp_self, self, other) : \
	 (tp_compare) == &DeeSeq_DefaultCompareWithSizeAndGetItemIndexFast ? DeeSeq_TDefaultCompareWithSizeAndGetItemIndexFast(tp_self, self, other) : \
	 (tp_compare) == &DeeSeq_DefaultCompareWithSizeAndTryGetItemIndex ? DeeSeq_TDefaultCompareWithSizeAndTryGetItemIndex(tp_self, self, other) : \
	 (tp_compare) == &DeeSeq_DefaultCompareWithSizeAndGetItemIndex ? DeeSeq_TDefaultCompareWithSizeAndGetItemIndex(tp_self, self, other) : \
	 (tp_compare) == &DeeSeq_DefaultCompareWithSizeObAndGetItem ? DeeSeq_TDefaultCompareWithSizeObAndGetItem(tp_self, self, other) : \
	 (tp_compare) == &DeeSeq_DefaultCompareWithForeachDefault ? DeeSeq_TDefaultCompareWithForeachDefault(tp_self, self, other) : \
	 default)
#define DeeType_invoke_tp_iter_next_DEFAULT(tp_self, tp_iter_next, self, default) \
	((tp_iter_next) == &DeeObject_DefaultIterNextWithIterNextPair ? DeeObject_TDefaultIterNextWithIterNextPair(tp_self, self) : \
	 default)
#define DeeType_invoke_iterator_tp_nextpair_DEFAULT(tp_self, tp_nextpair, self, key_and_value, default) \
	((tp_nextpair) == &DeeObject_DefaultIterNextPairWithIterNext ? DeeObject_TDefaultIterNextPairWithIterNext(tp_self, self, key_and_value) : \
	 default)
#define DeeType_invoke_iterator_tp_nextkey_DEFAULT(tp_self, tp_nextkey, self, default) \
	((tp_nextkey) == &DeeObject_DefaultIterNextKeyWithIterNext ? DeeObject_TDefaultIterNextKeyWithIterNext(tp_self, self) : \
	 (tp_nextkey) == &DeeObject_DefaultIterNextKeyWithIterNextPair ? DeeObject_TDefaultIterNextKeyWithIterNextPair(tp_self, self) : \
	 default)
#define DeeType_invoke_iterator_tp_nextvalue_DEFAULT(tp_self, tp_nextvalue, self, default) \
	((tp_nextvalue) == &DeeObject_DefaultIterNextValueWithIterNext ? DeeObject_TDefaultIterNextValueWithIterNext(tp_self, self) : \
	 (tp_nextvalue) == &DeeObject_DefaultIterNextValueWithIterNextPair ? DeeObject_TDefaultIterNextValueWithIterNextPair(tp_self, self) : \
	 default)
#define DeeType_invoke_iterator_tp_advance_DEFAULT(tp_self, tp_advance, self, step, default) \
	((tp_advance) == &DeeObject_DefaultIterAdvanceWithIterNext ? DeeObject_TDefaultIterAdvanceWithIterNext(tp_self, self, step) : \
	 (tp_advance) == &DeeObject_DefaultIterAdvanceWithIterNextPair ? DeeObject_TDefaultIterAdvanceWithIterNextPair(tp_self, self, step) : \
	 (tp_advance) == &DeeObject_DefaultIterAdvanceWithIterNextKey ? DeeObject_TDefaultIterAdvanceWithIterNextKey(tp_self, self, step) : \
	 (tp_advance) == &DeeObject_DefaultIterAdvanceWithIterNextValue ? DeeObject_TDefaultIterAdvanceWithIterNextValue(tp_self, self, step) : \
	 default)
#define DeeType_invoke_seq_tp_iter_DEFAULT(tp_self, tp_iter, self, default) \
	((tp_iter) == &DeeObject_DefaultIterWithForeach ? DeeObject_TDefaultIterWithForeach(tp_self, self) : \
	 (tp_iter) == &DeeObject_DefaultIterWithForeachPair ? DeeObject_TDefaultIterWithForeachPair(tp_self, self) : \
	 (tp_iter) == &DeeObject_DefaultIterWithEnumerate ? DeeObject_TDefaultIterWithEnumerate(tp_self, self) : \
	 (tp_iter) == &DeeObject_DefaultIterWithEnumerateIndex ? DeeObject_TDefaultIterWithEnumerateIndex(tp_self, self) : \
	 (tp_iter) == &DeeObject_DefaultIterWithIterKeysAndTryGetItem ? DeeObject_TDefaultIterWithIterKeysAndTryGetItem(tp_self, self) : \
	 (tp_iter) == &DeeObject_DefaultIterWithIterKeysAndGetItem ? DeeObject_TDefaultIterWithIterKeysAndGetItem(tp_self, self) : \
	 (tp_iter) == &DeeObject_DefaultIterWithIterKeysAndTryGetItemDefault ? DeeObject_TDefaultIterWithIterKeysAndTryGetItemDefault(tp_self, self) : \
	 (tp_iter) == &DeeSeq_DefaultIterWithSizeAndGetItemIndexFast ? DeeSeq_TDefaultIterWithSizeAndGetItemIndexFast(tp_self, self) : \
	 (tp_iter) == &DeeSeq_DefaultIterWithSizeAndTryGetItemIndex ? DeeSeq_TDefaultIterWithSizeAndTryGetItemIndex(tp_self, self) : \
	 (tp_iter) == &DeeSeq_DefaultIterWithSizeAndGetItemIndex ? DeeSeq_TDefaultIterWithSizeAndGetItemIndex(tp_self, self) : \
	 (tp_iter) == &DeeSeq_DefaultIterWithGetItemIndex ? DeeSeq_TDefaultIterWithGetItemIndex(tp_self, self) : \
	 (tp_iter) == &DeeSeq_DefaultIterWithSizeObAndGetItem ? DeeSeq_TDefaultIterWithSizeObAndGetItem(tp_self, self) : \
	 (tp_iter) == &DeeSeq_DefaultIterWithGetItem ? DeeSeq_TDefaultIterWithGetItem(tp_self, self) : \
	 (tp_iter) == &DeeMap_DefaultIterWithEnumerate ? DeeMap_TDefaultIterWithEnumerate(tp_self, self) : \
	 (tp_iter) == &DeeMap_DefaultIterWithEnumerateIndex ? DeeMap_TDefaultIterWithEnumerateIndex(tp_self, self) : \
	 (tp_iter) == &DeeMap_DefaultIterWithIterKeysAndTryGetItem ? DeeMap_TDefaultIterWithIterKeysAndTryGetItem(tp_self, self) : \
	 (tp_iter) == &DeeMap_DefaultIterWithIterKeysAndGetItem ? DeeMap_TDefaultIterWithIterKeysAndGetItem(tp_self, self) : \
	 (tp_iter) == &DeeMap_DefaultIterWithIterKeysAndTryGetItemDefault ? DeeMap_TDefaultIterWithIterKeysAndTryGetItemDefault(tp_self, self) : \
	 default)
#define DeeType_invoke_seq_tp_foreach_DEFAULT(tp_self, tp_foreach, self, proc, arg, default) \
	((tp_foreach) == &DeeObject_DefaultForeachWithIter ? DeeObject_TDefaultForeachWithIter(tp_self, self, proc, arg) : \
	 (tp_foreach) == &DeeObject_DefaultForeachWithEnumerate ? DeeObject_TDefaultForeachWithEnumerate(tp_self, self, proc, arg) : \
	 (tp_foreach) == &DeeObject_DefaultForeachWithEnumerateIndex ? DeeObject_TDefaultForeachWithEnumerateIndex(tp_self, self, proc, arg) : \
	 (tp_foreach) == &DeeObject_DefaultForeachWithForeachPair ? DeeObject_TDefaultForeachWithForeachPair(tp_self, self, proc, arg) : \
	 (tp_foreach) == &DeeObject_DefaultForeachWithForeachPairDefault ? DeeObject_TDefaultForeachWithForeachPairDefault(tp_self, self, proc, arg) : \
	 (tp_foreach) == &DeeObject_DefaultForeachWithIterKeysAndTryGetItem ? DeeObject_TDefaultForeachWithIterKeysAndTryGetItem(tp_self, self, proc, arg) : \
	 (tp_foreach) == &DeeObject_DefaultForeachWithIterKeysAndGetItem ? DeeObject_TDefaultForeachWithIterKeysAndGetItem(tp_self, self, proc, arg) : \
	 (tp_foreach) == &DeeObject_DefaultForeachWithIterKeysAndTryGetItemDefault ? DeeObject_TDefaultForeachWithIterKeysAndTryGetItemDefault(tp_self, self, proc, arg) : \
	 (tp_foreach) == &DeeSeq_DefaultForeachWithSizeAndGetItemIndexFast ? DeeSeq_TDefaultForeachWithSizeAndGetItemIndexFast(tp_self, self, proc, arg) : \
	 (tp_foreach) == &DeeSeq_DefaultForeachWithSizeAndTryGetItemIndex ? DeeSeq_TDefaultForeachWithSizeAndTryGetItemIndex(tp_self, self, proc, arg) : \
	 (tp_foreach) == &DeeSeq_DefaultForeachWithSizeAndGetItemIndex ? DeeSeq_TDefaultForeachWithSizeAndGetItemIndex(tp_self, self, proc, arg) : \
	 (tp_foreach) == &DeeSeq_DefaultForeachWithSizeObAndGetItem ? DeeSeq_TDefaultForeachWithSizeObAndGetItem(tp_self, self, proc, arg) : \
	 (tp_foreach) == &DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault ? DeeSeq_TDefaultForeachWithSizeDefaultAndGetItemIndexDefault(tp_self, self, proc, arg) : \
	 (tp_foreach) == &DeeSeq_DefaultForeachWithGetItemIndexDefault ? DeeSeq_TDefaultForeachWithGetItemIndexDefault(tp_self, self, proc, arg) : \
	 default)
#define DeeType_invoke_seq_tp_foreach_pair_DEFAULT(tp_self, tp_foreach_pair, self, proc, arg, default) \
	((tp_foreach_pair) == &DeeObject_DefaultForeachPairWithForeach ? DeeObject_TDefaultForeachPairWithForeach(tp_self, self, proc, arg) : \
	 (tp_foreach_pair) == &DeeObject_DefaultForeachPairWithEnumerate ? DeeObject_TDefaultForeachPairWithEnumerate(tp_self, self, proc, arg) : \
	 (tp_foreach_pair) == &DeeObject_DefaultForeachPairWithEnumerateIndex ? DeeObject_TDefaultForeachPairWithEnumerateIndex(tp_self, self, proc, arg) : \
	 (tp_foreach_pair) == &DeeObject_DefaultForeachPairWithForeachDefault ? DeeObject_TDefaultForeachPairWithForeachDefault(tp_self, self, proc, arg) : \
	 (tp_foreach_pair) == &DeeObject_DefaultForeachPairWithIter ? DeeObject_TDefaultForeachPairWithIter(tp_self, self, proc, arg) : \
	 (tp_foreach_pair) == &DeeMap_DefaultForeachPairWithEnumerate ? DeeMap_TDefaultForeachPairWithEnumerate(tp_self, self, proc, arg) : \
	 (tp_foreach_pair) == &DeeMap_DefaultForeachPairWithEnumerateIndex ? DeeMap_TDefaultForeachPairWithEnumerateIndex(tp_self, self, proc, arg) : \
	 (tp_foreach_pair) == &DeeMap_DefaultForeachPairWithEnumerateDefault ? DeeMap_TDefaultForeachPairWithEnumerateDefault(tp_self, self, proc, arg) : \
	 default)
#define DeeType_invoke_seq_tp_enumerate_DEFAULT(tp_self, tp_enumerate, self, proc, arg, default) \
	((tp_enumerate) == &DeeObject_DefaultEnumerateWithEnumerateIndex ? DeeObject_TDefaultEnumerateWithEnumerateIndex(tp_self, self, proc, arg) : \
	 (tp_enumerate) == &DeeObject_DefaultEnumerateWithIterKeysAndTryGetItem ? DeeObject_TDefaultEnumerateWithIterKeysAndTryGetItem(tp_self, self, proc, arg) : \
	 (tp_enumerate) == &DeeObject_DefaultEnumerateWithIterKeysAndGetItem ? DeeObject_TDefaultEnumerateWithIterKeysAndGetItem(tp_self, self, proc, arg) : \
	 (tp_enumerate) == &DeeObject_DefaultEnumerateWithIterKeysAndTryGetItemDefault ? DeeObject_TDefaultEnumerateWithIterKeysAndTryGetItemDefault(tp_self, self, proc, arg) : \
	 (tp_enumerate) == &DeeSeq_DefaultEnumerateWithSizeAndGetItemIndexFast ? DeeSeq_TDefaultEnumerateWithSizeAndGetItemIndexFast(tp_self, self, proc, arg) : \
	 (tp_enumerate) == &DeeSeq_DefaultEnumerateWithSizeAndTryGetItemIndex ? DeeSeq_TDefaultEnumerateWithSizeAndTryGetItemIndex(tp_self, self, proc, arg) : \
	 (tp_enumerate) == &DeeSeq_DefaultEnumerateWithSizeAndGetItemIndex ? DeeSeq_TDefaultEnumerateWithSizeAndGetItemIndex(tp_self, self, proc, arg) : \
	 (tp_enumerate) == &DeeSeq_DefaultEnumerateWithSizeObAndGetItem ? DeeSeq_TDefaultEnumerateWithSizeObAndGetItem(tp_self, self, proc, arg) : \
	 (tp_enumerate) == &DeeSeq_DefaultEnumerateWithCounterAndForeach ? DeeSeq_TDefaultEnumerateWithCounterAndForeach(tp_self, self, proc, arg) : \
	 (tp_enumerate) == &DeeSeq_DefaultEnumerateWithCounterAndIter ? DeeSeq_TDefaultEnumerateWithCounterAndIter(tp_self, self, proc, arg) : \
	 (tp_enumerate) == &DeeMap_DefaultEnumerateWithForeachPairDefault ? DeeMap_TDefaultEnumerateWithForeachPairDefault(tp_self, self, proc, arg) : \
	 (tp_enumerate) == &DeeMap_DefaultEnumerateWithIter ? DeeMap_TDefaultEnumerateWithIter(tp_self, self, proc, arg) : \
	 (tp_enumerate) == &DeeSeq_DefaultEnumerateWithSizeDefaultAndGetItemIndexDefault ? DeeSeq_TDefaultEnumerateWithSizeDefaultAndGetItemIndexDefault(tp_self, self, proc, arg) : \
	 (tp_enumerate) == &DeeSeq_DefaultEnumerateWithCounterAndForeachDefault ? DeeSeq_TDefaultEnumerateWithCounterAndForeachDefault(tp_self, self, proc, arg) : \
	 default)
#define DeeType_invoke_seq_tp_enumerate_index_DEFAULT(tp_self, tp_enumerate_index, self, proc, arg, start, end, default) \
	((tp_enumerate_index) == &DeeObject_DefaultEnumerateIndexWithEnumerate ? DeeObject_TDefaultEnumerateIndexWithEnumerate(tp_self, self, proc, arg, start, end) : \
	 (tp_enumerate_index) == &DeeSeq_DefaultEnumerateIndexWithSizeAndGetItemIndexFast ? DeeSeq_TDefaultEnumerateIndexWithSizeAndGetItemIndexFast(tp_self, self, proc, arg, start, end) : \
	 (tp_enumerate_index) == &DeeSeq_DefaultEnumerateIndexWithSizeAndTryGetItemIndex ? DeeSeq_TDefaultEnumerateIndexWithSizeAndTryGetItemIndex(tp_self, self, proc, arg, start, end) : \
	 (tp_enumerate_index) == &DeeSeq_DefaultEnumerateIndexWithSizeAndGetItemIndex ? DeeSeq_TDefaultEnumerateIndexWithSizeAndGetItemIndex(tp_self, self, proc, arg, start, end) : \
	 (tp_enumerate_index) == &DeeSeq_DefaultEnumerateIndexWithSizeObAndGetItem ? DeeSeq_TDefaultEnumerateIndexWithSizeObAndGetItem(tp_self, self, proc, arg, start, end) : \
	 (tp_enumerate_index) == &DeeSeq_DefaultEnumerateIndexWithCounterAndForeach ? DeeSeq_TDefaultEnumerateIndexWithCounterAndForeach(tp_self, self, proc, arg, start, end) : \
	 (tp_enumerate_index) == &DeeSeq_DefaultEnumerateIndexWithCounterAndIter ? DeeSeq_TDefaultEnumerateIndexWithCounterAndIter(tp_self, self, proc, arg, start, end) : \
	 (tp_enumerate_index) == &DeeSeq_DefaultEnumerateIndexWithSizeDefaultAndGetItemIndexDefault ? DeeSeq_TDefaultEnumerateIndexWithSizeDefaultAndGetItemIndexDefault(tp_self, self, proc, arg, start, end) : \
	 (tp_enumerate_index) == &DeeSeq_DefaultEnumerateIndexWithCounterAndForeachDefault ? DeeSeq_TDefaultEnumerateIndexWithCounterAndForeachDefault(tp_self, self, proc, arg, start, end) : \
	 (tp_enumerate_index) == &DeeObject_DefaultEnumerateIndexWithEnumerateDefault ? DeeObject_TDefaultEnumerateIndexWithEnumerateDefault(tp_self, self, proc, arg, start, end) : \
	 default)
#define DeeType_invoke_seq_tp_iterkeys_DEFAULT(tp_self, tp_iterkeys, self, default) \
	((tp_iterkeys) == &DeeObject_DefaultIterKeysWithEnumerate ? DeeObject_TDefaultIterKeysWithEnumerate(tp_self, self) : \
	 (tp_iterkeys) == &DeeObject_DefaultIterKeysWithEnumerateIndex ? DeeObject_TDefaultIterKeysWithEnumerateIndex(tp_self, self) : \
	 (tp_iterkeys) == &DeeSeq_DefaultIterKeysWithSize ? DeeSeq_TDefaultIterKeysWithSize(tp_self, self) : \
	 (tp_iterkeys) == &DeeSeq_DefaultIterKeysWithSizeOb ? DeeSeq_TDefaultIterKeysWithSizeOb(tp_self, self) : \
	 (tp_iterkeys) == &DeeSeq_DefaultIterKeysWithSizeDefault ? DeeSeq_TDefaultIterKeysWithSizeDefault(tp_self, self) : \
	 (tp_iterkeys) == &DeeMap_DefaultIterKeysWithIter ? DeeMap_TDefaultIterKeysWithIter(tp_self, self) : \
	 (tp_iterkeys) == &DeeMap_DefaultIterKeysWithIterDefault ? DeeMap_TDefaultIterKeysWithIterDefault(tp_self, self) : \
	 default)
#define DeeType_invoke_seq_tp_sizeob_DEFAULT(tp_self, tp_sizeob, self, default) \
	((tp_sizeob) == &DeeObject_DefaultSizeObWithSize ? DeeObject_TDefaultSizeObWithSize(tp_self, self) : \
	 (tp_sizeob) == &DeeObject_DefaultSizeObWithSizeDefault ? DeeObject_TDefaultSizeObWithSizeDefault(tp_self, self) : \
	 default)
#define DeeType_invoke_seq_tp_size_DEFAULT(tp_self, tp_size, self, default) \
	((tp_size) == &DeeObject_DefaultSizeWithSizeOb ? DeeObject_TDefaultSizeWithSizeOb(tp_self, self) : \
	 (tp_size) == &DeeSeq_DefaultSizeWithEnumerateIndex ? DeeSeq_TDefaultSizeWithEnumerateIndex(tp_self, self) : \
	 (tp_size) == &DeeSeq_DefaultSizeWithEnumerate ? DeeSeq_TDefaultSizeWithEnumerate(tp_self, self) : \
	 (tp_size) == &DeeSeq_DefaultSizeWithForeachPair ? DeeSeq_TDefaultSizeWithForeachPair(tp_self, self) : \
	 (tp_size) == &DeeSeq_DefaultSizeWithForeach ? DeeSeq_TDefaultSizeWithForeach(tp_self, self) : \
	 (tp_size) == &DeeSeq_DefaultSizeWithIter ? DeeSeq_TDefaultSizeWithIter(tp_self, self) : \
	 default)
#define DeeType_invoke_seq_tp_size_fast_DEFAULT(tp_self, tp_size_fast, self, default) default
#define DeeType_invoke_seq_tp_contains_DEFAULT(tp_self, tp_contains, self, elem, default) \
	((tp_contains) == &DeeSeq_DefaultContainsWithForeachDefault ? DeeSeq_TDefaultContainsWithForeachDefault(tp_self, self, elem) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithHasItem ? DeeMap_TDefaultContainsWithHasItem(tp_self, self, elem) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithBoundItem ? DeeMap_TDefaultContainsWithBoundItem(tp_self, self, elem) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithTryGetItem ? DeeMap_TDefaultContainsWithTryGetItem(tp_self, self, elem) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithGetItem ? DeeMap_TDefaultContainsWithGetItem(tp_self, self, elem) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithHasItemStringHash ? DeeMap_TDefaultContainsWithHasItemStringHash(tp_self, self, elem) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithHasItemStringLenHash ? DeeMap_TDefaultContainsWithHasItemStringLenHash(tp_self, self, elem) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithHasItemIndex ? DeeMap_TDefaultContainsWithHasItemIndex(tp_self, self, elem) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithBoundItemStringHash ? DeeMap_TDefaultContainsWithBoundItemStringHash(tp_self, self, elem) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithBoundItemStringLenHash ? DeeMap_TDefaultContainsWithBoundItemStringLenHash(tp_self, self, elem) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithBoundItemIndex ? DeeMap_TDefaultContainsWithBoundItemIndex(tp_self, self, elem) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithTryGetItemStringHash ? DeeMap_TDefaultContainsWithTryGetItemStringHash(tp_self, self, elem) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithTryGetItemStringLenHash ? DeeMap_TDefaultContainsWithTryGetItemStringLenHash(tp_self, self, elem) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithTryGetItemIndex ? DeeMap_TDefaultContainsWithTryGetItemIndex(tp_self, self, elem) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithGetItemStringHash ? DeeMap_TDefaultContainsWithGetItemStringHash(tp_self, self, elem) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithGetItemStringLenHash ? DeeMap_TDefaultContainsWithGetItemStringLenHash(tp_self, self, elem) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithGetItemIndex ? DeeMap_TDefaultContainsWithGetItemIndex(tp_self, self, elem) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithEnumerate ? DeeMap_TDefaultContainsWithEnumerate(tp_self, self, elem) : \
	 (tp_contains) == &DeeMap_DefaultContainsWithEnumerateDefault ? DeeMap_TDefaultContainsWithEnumerateDefault(tp_self, self, elem) : \
	 default)
#define DeeType_invoke_seq_tp_getitem_DEFAULT(tp_self, tp_getitem, self, index, default) \
	((tp_getitem) == &DeeObject_DefaultGetItemWithSizeAndGetItemIndexFast ? DeeObject_TDefaultGetItemWithSizeAndGetItemIndexFast(tp_self, self, index) : \
	 (tp_getitem) == &DeeObject_DefaultGetItemWithGetItemIndex ? DeeObject_TDefaultGetItemWithGetItemIndex(tp_self, self, index) : \
	 (tp_getitem) == &DeeObject_DefaultGetItemWithGetItemStringHash ? DeeObject_TDefaultGetItemWithGetItemStringHash(tp_self, self, index) : \
	 (tp_getitem) == &DeeObject_DefaultGetItemWithGetItemStringLenHash ? DeeObject_TDefaultGetItemWithGetItemStringLenHash(tp_self, self, index) : \
	 (tp_getitem) == &DeeObject_DefaultGetItemWithTryGetItem ? DeeObject_TDefaultGetItemWithTryGetItem(tp_self, self, index) : \
	 (tp_getitem) == &DeeObject_DefaultGetItemWithTryGetItemIndex ? DeeObject_TDefaultGetItemWithTryGetItemIndex(tp_self, self, index) : \
	 (tp_getitem) == &DeeObject_DefaultGetItemWithTryGetItemStringHash ? DeeObject_TDefaultGetItemWithTryGetItemStringHash(tp_self, self, index) : \
	 (tp_getitem) == &DeeObject_DefaultGetItemWithTryGetItemStringLenHash ? DeeObject_TDefaultGetItemWithTryGetItemStringLenHash(tp_self, self, index) : \
	 (tp_getitem) == &DeeObject_DefaultGetItemWithGetItemIndexDefault ? DeeObject_TDefaultGetItemWithGetItemIndexDefault(tp_self, self, index) : \
	 (tp_getitem) == &DeeSeq_DefaultGetItemWithTryGetItemAndSizeOb ? DeeSeq_TDefaultGetItemWithTryGetItemAndSizeOb(tp_self, self, index) : \
	 (tp_getitem) == &DeeSeq_DefaultGetItemWithTryGetItemAndSize ? DeeSeq_TDefaultGetItemWithTryGetItemAndSize(tp_self, self, index) : \
	 (tp_getitem) == &DeeSeq_DefaultGetItemWithSizeAndTryGetItemIndexOb ? DeeSeq_TDefaultGetItemWithSizeAndTryGetItemIndexOb(tp_self, self, index) : \
	 (tp_getitem) == &DeeSeq_DefaultGetItemWithSizeAndTryGetItemIndex ? DeeSeq_TDefaultGetItemWithSizeAndTryGetItemIndex(tp_self, self, index) : \
	 (tp_getitem) == &DeeMap_DefaultGetItemWithEnumerate ? DeeMap_TDefaultGetItemWithEnumerate(tp_self, self, index) : \
	 (tp_getitem) == &DeeMap_DefaultGetItemWithEnumerateDefault ? DeeMap_TDefaultGetItemWithEnumerateDefault(tp_self, self, index) : \
	 default)
#define DeeType_invoke_seq_tp_getitem_index_DEFAULT(tp_self, tp_getitem_index, self, index, default) \
	((tp_getitem_index) == &DeeObject_DefaultGetItemIndexWithSizeAndGetItemIndexFast ? DeeObject_TDefaultGetItemIndexWithSizeAndGetItemIndexFast(tp_self, self, index) : \
	 (tp_getitem_index) == &DeeObject_DefaultGetItemIndexWithTryGetItemIndex ? DeeObject_TDefaultGetItemIndexWithTryGetItemIndex(tp_self, self, index) : \
	 (tp_getitem_index) == &DeeObject_DefaultGetItemIndexWithGetItem ? DeeObject_TDefaultGetItemIndexWithGetItem(tp_self, self, index) : \
	 (tp_getitem_index) == &DeeObject_DefaultGetItemIndexWithTryGetItem ? DeeObject_TDefaultGetItemIndexWithTryGetItem(tp_self, self, index) : \
	 (tp_getitem_index) == &DeeObject_DefaultGetItemIndexWithGetItemDefault ? DeeObject_TDefaultGetItemIndexWithGetItemDefault(tp_self, self, index) : \
	 (tp_getitem_index) == &DeeSeq_DefaultGetItemIndexWithSizeAndTryGetItemIndex ? DeeSeq_TDefaultGetItemIndexWithSizeAndTryGetItemIndex(tp_self, self, index) : \
	 (tp_getitem_index) == &DeeSeq_DefaultGetItemIndexWithSizeAndTryGetItemIndexOb ? DeeSeq_TDefaultGetItemIndexWithSizeAndTryGetItemIndexOb(tp_self, self, index) : \
	 (tp_getitem_index) == &DeeSeq_DefaultGetItemIndexWithTryGetItemAndSize ? DeeSeq_TDefaultGetItemIndexWithTryGetItemAndSize(tp_self, self, index) : \
	 (tp_getitem_index) == &DeeSeq_DefaultGetItemIndexWithTryGetItemAndSizeOb ? DeeSeq_TDefaultGetItemIndexWithTryGetItemAndSizeOb(tp_self, self, index) : \
	 (tp_getitem_index) == &DeeSeq_DefaultGetItemIndexWithForeachDefault ? DeeSeq_TDefaultGetItemIndexWithForeachDefault(tp_self, self, index) : \
	 (tp_getitem_index) == &DeeMap_DefaultGetItemIndexWithEnumerate ? DeeMap_TDefaultGetItemIndexWithEnumerate(tp_self, self, index) : \
	 (tp_getitem_index) == &DeeMap_DefaultGetItemIndexWithEnumerateDefault ? DeeMap_TDefaultGetItemIndexWithEnumerateDefault(tp_self, self, index) : \
	 default)
#define DeeType_invoke_seq_tp_getitem_string_hash_DEFAULT(tp_self, tp_getitem_string_hash, self, key, hash, default) \
	((tp_getitem_string_hash) == &DeeObject_DefaultGetItemStringHashWithGetItemStringLenHash ? DeeObject_TDefaultGetItemStringHashWithGetItemStringLenHash(tp_self, self, key, hash) : \
	 (tp_getitem_string_hash) == &DeeObject_DefaultGetItemStringHashWithGetItem ? DeeObject_TDefaultGetItemStringHashWithGetItem(tp_self, self, key, hash) : \
	 (tp_getitem_string_hash) == &DeeObject_DefaultGetItemStringHashWithTryGetItemStringHash ? DeeObject_TDefaultGetItemStringHashWithTryGetItemStringHash(tp_self, self, key, hash) : \
	 (tp_getitem_string_hash) == &DeeObject_DefaultGetItemStringHashWithTryGetItemStringLenHash ? DeeObject_TDefaultGetItemStringHashWithTryGetItemStringLenHash(tp_self, self, key, hash) : \
	 (tp_getitem_string_hash) == &DeeObject_DefaultGetItemStringHashWithTryGetItem ? DeeObject_TDefaultGetItemStringHashWithTryGetItem(tp_self, self, key, hash) : \
	 (tp_getitem_string_hash) == &DeeObject_DefaultGetItemStringHashWithGetItemDefault ? DeeObject_TDefaultGetItemStringHashWithGetItemDefault(tp_self, self, key, hash) : \
	 (tp_getitem_string_hash) == &DeeMap_DefaultGetItemStringHashWithEnumerate ? DeeMap_TDefaultGetItemStringHashWithEnumerate(tp_self, self, key, hash) : \
	 (tp_getitem_string_hash) == &DeeMap_DefaultGetItemStringHashWithEnumerateDefault ? DeeMap_TDefaultGetItemStringHashWithEnumerateDefault(tp_self, self, key, hash) : \
	 default)
#define DeeType_invoke_seq_tp_getitem_string_len_hash_DEFAULT(tp_self, tp_getitem_string_len_hash, self, key, keylen, hash, default) \
	((tp_getitem_string_len_hash) == &DeeObject_DefaultGetItemStringLenHashWithGetItemStringHash ? DeeObject_TDefaultGetItemStringLenHashWithGetItemStringHash(tp_self, self, key, keylen, hash) : \
	 (tp_getitem_string_len_hash) == &DeeObject_DefaultGetItemStringLenHashWithGetItem ? DeeObject_TDefaultGetItemStringLenHashWithGetItem(tp_self, self, key, keylen, hash) : \
	 (tp_getitem_string_len_hash) == &DeeObject_DefaultGetItemStringLenHashWithTryGetItemStringLenHash ? DeeObject_TDefaultGetItemStringLenHashWithTryGetItemStringLenHash(tp_self, self, key, keylen, hash) : \
	 (tp_getitem_string_len_hash) == &DeeObject_DefaultGetItemStringLenHashWithTryGetItemStringHash ? DeeObject_TDefaultGetItemStringLenHashWithTryGetItemStringHash(tp_self, self, key, keylen, hash) : \
	 (tp_getitem_string_len_hash) == &DeeObject_DefaultGetItemStringLenHashWithTryGetItem ? DeeObject_TDefaultGetItemStringLenHashWithTryGetItem(tp_self, self, key, keylen, hash) : \
	 (tp_getitem_string_len_hash) == &DeeObject_DefaultGetItemStringLenHashWithGetItemDefault ? DeeObject_TDefaultGetItemStringLenHashWithGetItemDefault(tp_self, self, key, keylen, hash) : \
	 (tp_getitem_string_len_hash) == &DeeMap_DefaultGetItemStringLenHashWithEnumerate ? DeeMap_TDefaultGetItemStringLenHashWithEnumerate(tp_self, self, key, keylen, hash) : \
	 (tp_getitem_string_len_hash) == &DeeMap_DefaultGetItemStringLenHashWithEnumerateDefault ? DeeMap_TDefaultGetItemStringLenHashWithEnumerateDefault(tp_self, self, key, keylen, hash) : \
	 default)
#define DeeType_invoke_seq_tp_trygetitem_DEFAULT(tp_self, tp_trygetitem, self, index, default) \
	((tp_trygetitem) == &DeeObject_DefaultTryGetItemWithTryGetItemIndex ? DeeObject_TDefaultTryGetItemWithTryGetItemIndex(tp_self, self, index) : \
	 (tp_trygetitem) == &DeeObject_DefaultTryGetItemWithTryGetItemStringHash ? DeeObject_TDefaultTryGetItemWithTryGetItemStringHash(tp_self, self, index) : \
	 (tp_trygetitem) == &DeeObject_DefaultTryGetItemWithTryGetItemStringLenHash ? DeeObject_TDefaultTryGetItemWithTryGetItemStringLenHash(tp_self, self, index) : \
	 (tp_trygetitem) == &DeeObject_DefaultTryGetItemWithSizeAndGetItemIndexFast ? DeeObject_TDefaultTryGetItemWithSizeAndGetItemIndexFast(tp_self, self, index) : \
	 (tp_trygetitem) == &DeeObject_DefaultTryGetItemWithGetItemIndex ? DeeObject_TDefaultTryGetItemWithGetItemIndex(tp_self, self, index) : \
	 (tp_trygetitem) == &DeeObject_DefaultTryGetItemWithGetItem ? DeeObject_TDefaultTryGetItemWithGetItem(tp_self, self, index) : \
	 (tp_trygetitem) == &DeeObject_DefaultTryGetItemWithGetItemStringHash ? DeeObject_TDefaultTryGetItemWithGetItemStringHash(tp_self, self, index) : \
	 (tp_trygetitem) == &DeeObject_DefaultTryGetItemWithGetItemStringLenHash ? DeeObject_TDefaultTryGetItemWithGetItemStringLenHash(tp_self, self, index) : \
	 (tp_trygetitem) == &DeeObject_DefaultTryGetItemWithGetItemDefault ? DeeObject_TDefaultTryGetItemWithGetItemDefault(tp_self, self, index) : \
	 (tp_trygetitem) == &DeeMap_DefaultTryGetItemWithEnumerate ? DeeMap_TDefaultTryGetItemWithEnumerate(tp_self, self, index) : \
	 (tp_trygetitem) == &DeeMap_DefaultTryGetItemWithEnumerateDefault ? DeeMap_TDefaultTryGetItemWithEnumerateDefault(tp_self, self, index) : \
	 default)
#define DeeType_invoke_seq_tp_trygetitem_index_DEFAULT(tp_self, tp_trygetitem_index, self, index, default) \
	((tp_trygetitem_index) == &DeeObject_DefaultTryGetItemIndexWithSizeAndGetItemIndexFast ? DeeObject_TDefaultTryGetItemIndexWithSizeAndGetItemIndexFast(tp_self, self, index) : \
	 (tp_trygetitem_index) == &DeeObject_DefaultTryGetItemIndexWithTryGetItem ? DeeObject_TDefaultTryGetItemIndexWithTryGetItem(tp_self, self, index) : \
	 (tp_trygetitem_index) == &DeeObject_DefaultTryGetItemIndexWithGetItemIndex ? DeeObject_TDefaultTryGetItemIndexWithGetItemIndex(tp_self, self, index) : \
	 (tp_trygetitem_index) == &DeeObject_DefaultTryGetItemIndexWithGetItem ? DeeObject_TDefaultTryGetItemIndexWithGetItem(tp_self, self, index) : \
	 (tp_trygetitem_index) == &DeeObject_DefaultTryGetItemIndexWithGetItemIndexDefault ? DeeObject_TDefaultTryGetItemIndexWithGetItemIndexDefault(tp_self, self, index) : \
	 (tp_trygetitem_index) == &DeeSeq_DefaultTryGetItemIndexWithForeachDefault ? DeeSeq_TDefaultTryGetItemIndexWithForeachDefault(tp_self, self, index) : \
	 (tp_trygetitem_index) == &DeeMap_DefaultTryGetItemIndexWithEnumerate ? DeeMap_TDefaultTryGetItemIndexWithEnumerate(tp_self, self, index) : \
	 (tp_trygetitem_index) == &DeeMap_DefaultTryGetItemIndexWithEnumerateDefault ? DeeMap_TDefaultTryGetItemIndexWithEnumerateDefault(tp_self, self, index) : \
	 default)
#define DeeType_invoke_seq_tp_trygetitem_string_hash_DEFAULT(tp_self, tp_trygetitem_string_hash, self, key, hash, default) \
	((tp_trygetitem_string_hash) == &DeeObject_DefaultTryGetItemStringHashWithTryGetItemStringLenHash ? DeeObject_TDefaultTryGetItemStringHashWithTryGetItemStringLenHash(tp_self, self, key, hash) : \
	 (tp_trygetitem_string_hash) == &DeeObject_DefaultTryGetItemStringHashWithTryGetItem ? DeeObject_TDefaultTryGetItemStringHashWithTryGetItem(tp_self, self, key, hash) : \
	 (tp_trygetitem_string_hash) == &DeeObject_DefaultTryGetItemStringHashWithGetItemStringHash ? DeeObject_TDefaultTryGetItemStringHashWithGetItemStringHash(tp_self, self, key, hash) : \
	 (tp_trygetitem_string_hash) == &DeeObject_DefaultTryGetItemStringHashWithGetItemStringLenHash ? DeeObject_TDefaultTryGetItemStringHashWithGetItemStringLenHash(tp_self, self, key, hash) : \
	 (tp_trygetitem_string_hash) == &DeeObject_DefaultTryGetItemStringHashWithGetItem ? DeeObject_TDefaultTryGetItemStringHashWithGetItem(tp_self, self, key, hash) : \
	 (tp_trygetitem_string_hash) == &DeeObject_DefaultTryGetItemStringHashWithTryGetItemDefault ? DeeObject_TDefaultTryGetItemStringHashWithTryGetItemDefault(tp_self, self, key, hash) : \
	 (tp_trygetitem_string_hash) == &DeeMap_DefaultTryGetItemStringHashWithEnumerate ? DeeMap_TDefaultTryGetItemStringHashWithEnumerate(tp_self, self, key, hash) : \
	 (tp_trygetitem_string_hash) == &DeeMap_DefaultTryGetItemStringHashWithEnumerateDefault ? DeeMap_TDefaultTryGetItemStringHashWithEnumerateDefault(tp_self, self, key, hash) : \
	 default)
#define DeeType_invoke_seq_tp_trygetitem_string_len_hash_DEFAULT(tp_self, tp_trygetitem_string_len_hash, self, key, keylen, hash, default) \
	((tp_trygetitem_string_len_hash) == &DeeObject_DefaultTryGetItemStringLenHashWithTryGetItemStringHash ? DeeObject_TDefaultTryGetItemStringLenHashWithTryGetItemStringHash(tp_self, self, key, keylen, hash) : \
	 (tp_trygetitem_string_len_hash) == &DeeObject_DefaultTryGetItemStringLenHashWithTryGetItem ? DeeObject_TDefaultTryGetItemStringLenHashWithTryGetItem(tp_self, self, key, keylen, hash) : \
	 (tp_trygetitem_string_len_hash) == &DeeObject_DefaultTryGetItemStringLenHashWithGetItemStringLenHash ? DeeObject_TDefaultTryGetItemStringLenHashWithGetItemStringLenHash(tp_self, self, key, keylen, hash) : \
	 (tp_trygetitem_string_len_hash) == &DeeObject_DefaultTryGetItemStringLenHashWithGetItemStringHash ? DeeObject_TDefaultTryGetItemStringLenHashWithGetItemStringHash(tp_self, self, key, keylen, hash) : \
	 (tp_trygetitem_string_len_hash) == &DeeObject_DefaultTryGetItemStringLenHashWithGetItem ? DeeObject_TDefaultTryGetItemStringLenHashWithGetItem(tp_self, self, key, keylen, hash) : \
	 (tp_trygetitem_string_len_hash) == &DeeObject_DefaultTryGetItemStringLenHashWithTryGetItemDefault ? DeeObject_TDefaultTryGetItemStringLenHashWithTryGetItemDefault(tp_self, self, key, keylen, hash) : \
	 (tp_trygetitem_string_len_hash) == &DeeMap_DefaultTryGetItemStringLenHashWithEnumerate ? DeeMap_TDefaultTryGetItemStringLenHashWithEnumerate(tp_self, self, key, keylen, hash) : \
	 (tp_trygetitem_string_len_hash) == &DeeMap_DefaultTryGetItemStringLenHashWithEnumerateDefault ? DeeMap_TDefaultTryGetItemStringLenHashWithEnumerateDefault(tp_self, self, key, keylen, hash) : \
	 default)
#define DeeType_invoke_seq_tp_delitem_DEFAULT(tp_self, tp_delitem, self, index, default) \
	((tp_delitem) == &DeeObject_DefaultDelItemWithDelItemIndex ? DeeObject_TDefaultDelItemWithDelItemIndex(tp_self, self, index) : \
	 (tp_delitem) == &DeeObject_DefaultDelItemWithDelItemIndexDefault ? DeeObject_TDefaultDelItemWithDelItemIndexDefault(tp_self, self, index) : \
	 (tp_delitem) == &DeeObject_DefaultDelItemWithDelItemStringHash ? DeeObject_TDefaultDelItemWithDelItemStringHash(tp_self, self, index) : \
	 (tp_delitem) == &DeeObject_DefaultDelItemWithDelItemStringLenHash ? DeeObject_TDefaultDelItemWithDelItemStringLenHash(tp_self, self, index) : \
	 default)
#define DeeType_invoke_seq_tp_delitem_index_DEFAULT(tp_self, tp_delitem_index, self, index, default) \
	((tp_delitem_index) == &DeeObject_DefaultDelItemIndexWithDelItem ? DeeObject_TDefaultDelItemIndexWithDelItem(tp_self, self, index) : \
	 (tp_delitem_index) == &DeeSeq_DefaultDelItemIndexWithDelRangeIndexDefault ? DeeSeq_TDefaultDelItemIndexWithDelRangeIndexDefault(tp_self, self, index) : \
	 default)
#define DeeType_invoke_seq_tp_delitem_string_hash_DEFAULT(tp_self, tp_delitem_string_hash, self, key, hash, default) \
	((tp_delitem_string_hash) == &DeeObject_DefaultDelItemStringHashWithDelItemStringLenHash ? DeeObject_TDefaultDelItemStringHashWithDelItemStringLenHash(tp_self, self, key, hash) : \
	 (tp_delitem_string_hash) == &DeeObject_DefaultDelItemStringHashWithDelItem ? DeeObject_TDefaultDelItemStringHashWithDelItem(tp_self, self, key, hash) : \
	 default)
#define DeeType_invoke_seq_tp_delitem_string_len_hash_DEFAULT(tp_self, tp_delitem_string_len_hash, self, key, keylen, hash, default) \
	((tp_delitem_string_len_hash) == &DeeObject_DefaultDelItemStringLenHashWithDelItemStringHash ? DeeObject_TDefaultDelItemStringLenHashWithDelItemStringHash(tp_self, self, key, keylen, hash) : \
	 (tp_delitem_string_len_hash) == &DeeObject_DefaultDelItemStringLenHashWithDelItem ? DeeObject_TDefaultDelItemStringLenHashWithDelItem(tp_self, self, key, keylen, hash) : \
	 default)
#define DeeType_invoke_seq_tp_setitem_DEFAULT(tp_self, tp_setitem, self, index, value, default) \
	((tp_setitem) == &DeeObject_DefaultSetItemWithSetItemIndex ? DeeObject_TDefaultSetItemWithSetItemIndex(tp_self, self, index, value) : \
	 (tp_setitem) == &DeeObject_DefaultSetItemWithSetItemIndexDefault ? DeeObject_TDefaultSetItemWithSetItemIndexDefault(tp_self, self, index, value) : \
	 (tp_setitem) == &DeeObject_DefaultSetItemWithSetItemStringHash ? DeeObject_TDefaultSetItemWithSetItemStringHash(tp_self, self, index, value) : \
	 (tp_setitem) == &DeeObject_DefaultSetItemWithSetItemStringLenHash ? DeeObject_TDefaultSetItemWithSetItemStringLenHash(tp_self, self, index, value) : \
	 default)
#define DeeType_invoke_seq_tp_setitem_index_DEFAULT(tp_self, tp_setitem_index, self, index, value, default) \
	((tp_setitem_index) == &DeeObject_DefaultSetItemIndexWithSetItem ? DeeObject_TDefaultSetItemIndexWithSetItem(tp_self, self, index, value) : \
	 (tp_setitem_index) == &DeeSeq_DefaultSetItemIndexWithSetRangeIndexDefault ? DeeSeq_TDefaultSetItemIndexWithSetRangeIndexDefault(tp_self, self, index, value) : \
	 default)
#define DeeType_invoke_seq_tp_setitem_string_hash_DEFAULT(tp_self, tp_setitem_string_hash, self, key, hash, value, default) \
	((tp_setitem_string_hash) == &DeeObject_DefaultSetItemStringHashWithSetItemStringLenHash ? DeeObject_TDefaultSetItemStringHashWithSetItemStringLenHash(tp_self, self, key, hash, value) : \
	 (tp_setitem_string_hash) == &DeeObject_DefaultSetItemStringHashWithSetItem ? DeeObject_TDefaultSetItemStringHashWithSetItem(tp_self, self, key, hash, value) : \
	 default)
#define DeeType_invoke_seq_tp_setitem_string_len_hash_DEFAULT(tp_self, tp_setitem_string_len_hash, self, key, keylen, hash, value, default) \
	((tp_setitem_string_len_hash) == &DeeObject_DefaultSetItemStringLenHashWithSetItemStringHash ? DeeObject_TDefaultSetItemStringLenHashWithSetItemStringHash(tp_self, self, key, keylen, hash, value) : \
	 (tp_setitem_string_len_hash) == &DeeObject_DefaultSetItemStringLenHashWithSetItem ? DeeObject_TDefaultSetItemStringLenHashWithSetItem(tp_self, self, key, keylen, hash, value) : \
	 default)
#define DeeType_invoke_seq_tp_bounditem_DEFAULT(tp_self, tp_bounditem, self, index, default) \
	((tp_bounditem) == &DeeObject_DefaultBoundItemWithBoundItemIndex ? DeeObject_TDefaultBoundItemWithBoundItemIndex(tp_self, self, index) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithBoundItemStringHash ? DeeObject_TDefaultBoundItemWithBoundItemStringHash(tp_self, self, index) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithBoundItemStringLenHash ? DeeObject_TDefaultBoundItemWithBoundItemStringLenHash(tp_self, self, index) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithGetItem ? DeeObject_TDefaultBoundItemWithGetItem(tp_self, self, index) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithGetItemStringHash ? DeeObject_TDefaultBoundItemWithGetItemStringHash(tp_self, self, index) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithGetItemStringLenHash ? DeeObject_TDefaultBoundItemWithGetItemStringLenHash(tp_self, self, index) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithGetItemIndex ? DeeObject_TDefaultBoundItemWithGetItemIndex(tp_self, self, index) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithTryGetItemAndHasItem ? DeeObject_TDefaultBoundItemWithTryGetItemAndHasItem(tp_self, self, index) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithTryGetItemIndexAndHasItemIndex ? DeeObject_TDefaultBoundItemWithTryGetItemIndexAndHasItemIndex(tp_self, self, index) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithTryGetItemStringLenHashAndHasItemStringLenHash ? DeeObject_TDefaultBoundItemWithTryGetItemStringLenHashAndHasItemStringLenHash(tp_self, self, index) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithTryGetItemStringHashAndHasItemStringHash ? DeeObject_TDefaultBoundItemWithTryGetItemStringHashAndHasItemStringHash(tp_self, self, index) : \
	 (tp_bounditem) == &DeeSeq_DefaultBoundItemWithTryGetItemAndSizeOb ? DeeSeq_TDefaultBoundItemWithTryGetItemAndSizeOb(tp_self, self, index) : \
	 (tp_bounditem) == &DeeSeq_DefaultBoundItemWithSizeAndTryGetItemIndex ? DeeSeq_TDefaultBoundItemWithSizeAndTryGetItemIndex(tp_self, self, index) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithTryGetItem ? DeeObject_TDefaultBoundItemWithTryGetItem(tp_self, self, index) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithTryGetItemIndex ? DeeObject_TDefaultBoundItemWithTryGetItemIndex(tp_self, self, index) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithTryGetItemStringLenHash ? DeeObject_TDefaultBoundItemWithTryGetItemStringLenHash(tp_self, self, index) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithTryGetItemStringHash ? DeeObject_TDefaultBoundItemWithTryGetItemStringHash(tp_self, self, index) : \
	 (tp_bounditem) == &DeeObject_DefaultBoundItemWithGetItemDefault ? DeeObject_TDefaultBoundItemWithGetItemDefault(tp_self, self, index) : \
	 (tp_bounditem) == &DeeMap_DefaultBoundItemWithContains ? DeeMap_TDefaultBoundItemWithContains(tp_self, self, index) : \
	 (tp_bounditem) == &DeeMap_DefaultBoundItemWithEnumerate ? DeeMap_TDefaultBoundItemWithEnumerate(tp_self, self, index) : \
	 (tp_bounditem) == &DeeMap_DefaultBoundItemWithEnumerateDefault ? DeeMap_TDefaultBoundItemWithEnumerateDefault(tp_self, self, index) : \
	 default)
#define DeeType_invoke_seq_tp_bounditem_index_DEFAULT(tp_self, tp_bounditem_index, self, index, default) \
	((tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithBoundItem ? DeeObject_TDefaultBoundItemIndexWithBoundItem(tp_self, self, index) : \
	 (tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithSizeAndGetItemIndexFast ? DeeObject_TDefaultBoundItemIndexWithSizeAndGetItemIndexFast(tp_self, self, index) : \
	 (tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithGetItemIndex ? DeeObject_TDefaultBoundItemIndexWithGetItemIndex(tp_self, self, index) : \
	 (tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithGetItem ? DeeObject_TDefaultBoundItemIndexWithGetItem(tp_self, self, index) : \
	 (tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithGetItemIndexDefault ? DeeObject_TDefaultBoundItemIndexWithGetItemIndexDefault(tp_self, self, index) : \
	 (tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithTryGetItemIndexAndHasItemIndex ? DeeObject_TDefaultBoundItemIndexWithTryGetItemIndexAndHasItemIndex(tp_self, self, index) : \
	 (tp_bounditem_index) == &DeeObject_DefaultBoundItemIndexWithTryGetItemAndHasItem ? DeeObject_TDefaultBoundItemIndexWithTryGetItemAndHasItem(tp_self, self, index) : \
	 (tp_bounditem_index) == &DeeSeq_DefaultBoundItemIndexWithSizeAndTryGetItemIndex ? DeeSeq_TDefaultBoundItemIndexWithSizeAndTryGetItemIndex(tp_self, self, index) : \
	 (tp_bounditem_index) == &DeeSeq_DefaultBoundItemIndexWithTryGetItemAndSizeOb ? DeeSeq_TDefaultBoundItemIndexWithTryGetItemAndSizeOb(tp_self, self, index) : \
	 (tp_bounditem_index) == &DeeMap_DefaultBoundItemIndexWithContains ? DeeMap_TDefaultBoundItemIndexWithContains(tp_self, self, index) : \
	 (tp_bounditem_index) == &DeeMap_DefaultBoundItemIndexWithEnumerate ? DeeMap_TDefaultBoundItemIndexWithEnumerate(tp_self, self, index) : \
	 (tp_bounditem_index) == &DeeMap_DefaultBoundItemIndexWithEnumerateDefault ? DeeMap_TDefaultBoundItemIndexWithEnumerateDefault(tp_self, self, index) : \
	 default)
#define DeeType_invoke_seq_tp_bounditem_string_hash_DEFAULT(tp_self, tp_bounditem_string_hash, self, key, hash, default) \
	((tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithBoundItemStringLenHash ? DeeObject_TDefaultBoundItemStringHashWithBoundItemStringLenHash(tp_self, self, key, hash) : \
	 (tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithBoundItem ? DeeObject_TDefaultBoundItemStringHashWithBoundItem(tp_self, self, key, hash) : \
	 (tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithGetItemStringHash ? DeeObject_TDefaultBoundItemStringHashWithGetItemStringHash(tp_self, self, key, hash) : \
	 (tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithGetItemStringLenHash ? DeeObject_TDefaultBoundItemStringHashWithGetItemStringLenHash(tp_self, self, key, hash) : \
	 (tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithGetItem ? DeeObject_TDefaultBoundItemStringHashWithGetItem(tp_self, self, key, hash) : \
	 (tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithTryGetItemStringHashAndHasItemStringHash ? DeeObject_TDefaultBoundItemStringHashWithTryGetItemStringHashAndHasItemStringHash(tp_self, self, key, hash) : \
	 (tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithTryGetItemStringLenHashAndHasItemStringLenHash ? DeeObject_TDefaultBoundItemStringHashWithTryGetItemStringLenHashAndHasItemStringLenHash(tp_self, self, key, hash) : \
	 (tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithTryGetItemAndHasItem ? DeeObject_TDefaultBoundItemStringHashWithTryGetItemAndHasItem(tp_self, self, key, hash) : \
	 (tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithTryGetItemStringHash ? DeeObject_TDefaultBoundItemStringHashWithTryGetItemStringHash(tp_self, self, key, hash) : \
	 (tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithTryGetItemStringLenHash ? DeeObject_TDefaultBoundItemStringHashWithTryGetItemStringLenHash(tp_self, self, key, hash) : \
	 (tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithTryGetItem ? DeeObject_TDefaultBoundItemStringHashWithTryGetItem(tp_self, self, key, hash) : \
	 (tp_bounditem_string_hash) == &DeeObject_DefaultBoundItemStringHashWithBoundItemDefault ? DeeObject_TDefaultBoundItemStringHashWithBoundItemDefault(tp_self, self, key, hash) : \
	 (tp_bounditem_string_hash) == &DeeMap_DefaultBoundItemStringHashWithContains ? DeeMap_TDefaultBoundItemStringHashWithContains(tp_self, self, key, hash) : \
	 (tp_bounditem_string_hash) == &DeeMap_DefaultBoundItemStringHashWithEnumerate ? DeeMap_TDefaultBoundItemStringHashWithEnumerate(tp_self, self, key, hash) : \
	 (tp_bounditem_string_hash) == &DeeMap_DefaultBoundItemStringHashWithEnumerateDefault ? DeeMap_TDefaultBoundItemStringHashWithEnumerateDefault(tp_self, self, key, hash) : \
	 default)
#define DeeType_invoke_seq_tp_bounditem_string_len_hash_DEFAULT(tp_self, tp_bounditem_string_len_hash, self, key, keylen, hash, default) \
	((tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithBoundItemStringHash ? DeeObject_TDefaultBoundItemStringLenHashWithBoundItemStringHash(tp_self, self, key, keylen, hash) : \
	 (tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithBoundItem ? DeeObject_TDefaultBoundItemStringLenHashWithBoundItem(tp_self, self, key, keylen, hash) : \
	 (tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithGetItemStringLenHash ? DeeObject_TDefaultBoundItemStringLenHashWithGetItemStringLenHash(tp_self, self, key, keylen, hash) : \
	 (tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithGetItemStringHash ? DeeObject_TDefaultBoundItemStringLenHashWithGetItemStringHash(tp_self, self, key, keylen, hash) : \
	 (tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithGetItem ? DeeObject_TDefaultBoundItemStringLenHashWithGetItem(tp_self, self, key, keylen, hash) : \
	 (tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemStringLenHashAndHasItemStringLenHash ? DeeObject_TDefaultBoundItemStringLenHashWithTryGetItemStringLenHashAndHasItemStringLenHash(tp_self, self, key, keylen, hash) : \
	 (tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemStringHashAndHasItemStringHash ? DeeObject_TDefaultBoundItemStringLenHashWithTryGetItemStringHashAndHasItemStringHash(tp_self, self, key, keylen, hash) : \
	 (tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemAndHasItem ? DeeObject_TDefaultBoundItemStringLenHashWithTryGetItemAndHasItem(tp_self, self, key, keylen, hash) : \
	 (tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemStringLenHash ? DeeObject_TDefaultBoundItemStringLenHashWithTryGetItemStringLenHash(tp_self, self, key, keylen, hash) : \
	 (tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemStringHash ? DeeObject_TDefaultBoundItemStringLenHashWithTryGetItemStringHash(tp_self, self, key, keylen, hash) : \
	 (tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithTryGetItem ? DeeObject_TDefaultBoundItemStringLenHashWithTryGetItem(tp_self, self, key, keylen, hash) : \
	 (tp_bounditem_string_len_hash) == &DeeObject_DefaultBoundItemStringLenHashWithBoundItemDefault ? DeeObject_TDefaultBoundItemStringLenHashWithBoundItemDefault(tp_self, self, key, keylen, hash) : \
	 (tp_bounditem_string_len_hash) == &DeeMap_DefaultBoundItemStringLenHashWithContains ? DeeMap_TDefaultBoundItemStringLenHashWithContains(tp_self, self, key, keylen, hash) : \
	 (tp_bounditem_string_len_hash) == &DeeMap_DefaultBoundItemStringLenHashWithEnumerate ? DeeMap_TDefaultBoundItemStringLenHashWithEnumerate(tp_self, self, key, keylen, hash) : \
	 (tp_bounditem_string_len_hash) == &DeeMap_DefaultBoundItemStringLenHashWithEnumerateDefault ? DeeMap_TDefaultBoundItemStringLenHashWithEnumerateDefault(tp_self, self, key, keylen, hash) : \
	 default)
#define DeeType_invoke_seq_tp_hasitem_DEFAULT(tp_self, tp_hasitem, self, index, default) \
	((tp_hasitem) == &DeeObject_DefaultHasItemWithHasItemStringHash ? DeeObject_TDefaultHasItemWithHasItemStringHash(tp_self, self, index) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithHasItemStringLenHash ? DeeObject_TDefaultHasItemWithHasItemStringLenHash(tp_self, self, index) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithHasItemIndex ? DeeObject_TDefaultHasItemWithHasItemIndex(tp_self, self, index) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithBoundItem ? DeeObject_TDefaultHasItemWithBoundItem(tp_self, self, index) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithBoundItemStringHash ? DeeObject_TDefaultHasItemWithBoundItemStringHash(tp_self, self, index) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithBoundItemStringLenHash ? DeeObject_TDefaultHasItemWithBoundItemStringLenHash(tp_self, self, index) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithBoundItemIndex ? DeeObject_TDefaultHasItemWithBoundItemIndex(tp_self, self, index) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithTryGetItem ? DeeObject_TDefaultHasItemWithTryGetItem(tp_self, self, index) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithTryGetItemStringHash ? DeeObject_TDefaultHasItemWithTryGetItemStringHash(tp_self, self, index) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithTryGetItemStringLenHash ? DeeObject_TDefaultHasItemWithTryGetItemStringLenHash(tp_self, self, index) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithTryGetItemIndex ? DeeObject_TDefaultHasItemWithTryGetItemIndex(tp_self, self, index) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithGetItem ? DeeObject_TDefaultHasItemWithGetItem(tp_self, self, index) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithGetItemStringHash ? DeeObject_TDefaultHasItemWithGetItemStringHash(tp_self, self, index) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithGetItemStringLenHash ? DeeObject_TDefaultHasItemWithGetItemStringLenHash(tp_self, self, index) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithGetItemIndex ? DeeObject_TDefaultHasItemWithGetItemIndex(tp_self, self, index) : \
	 (tp_hasitem) == &DeeObject_DefaultHasItemWithGetItemDefault ? DeeObject_TDefaultHasItemWithGetItemDefault(tp_self, self, index) : \
	 (tp_hasitem) == &DeeSeq_DefaultHasItemWithSize ? DeeSeq_TDefaultHasItemWithSize(tp_self, self, index) : \
	 (tp_hasitem) == &DeeSeq_DefaultHasItemWithSizeOb ? DeeSeq_TDefaultHasItemWithSizeOb(tp_self, self, index) : \
	 (tp_hasitem) == &DeeMap_DefaultHasItemWithContains ? DeeMap_TDefaultHasItemWithContains(tp_self, self, index) : \
	 (tp_hasitem) == &DeeMap_DefaultHasItemWithEnumerate ? DeeMap_TDefaultHasItemWithEnumerate(tp_self, self, index) : \
	 (tp_hasitem) == &DeeMap_DefaultHasItemWithEnumerateDefault ? DeeMap_TDefaultHasItemWithEnumerateDefault(tp_self, self, index) : \
	 default)
#define DeeType_invoke_seq_tp_hasitem_index_DEFAULT(tp_self, tp_hasitem_index, self, index, default) \
	((tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithHasItem ? DeeObject_TDefaultHasItemIndexWithHasItem(tp_self, self, index) : \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithBoundItemIndex ? DeeObject_TDefaultHasItemIndexWithBoundItemIndex(tp_self, self, index) : \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithBoundItem ? DeeObject_TDefaultHasItemIndexWithBoundItem(tp_self, self, index) : \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithTryGetItemIndex ? DeeObject_TDefaultHasItemIndexWithTryGetItemIndex(tp_self, self, index) : \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithTryGetItem ? DeeObject_TDefaultHasItemIndexWithTryGetItem(tp_self, self, index) : \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithGetItemIndex ? DeeObject_TDefaultHasItemIndexWithGetItemIndex(tp_self, self, index) : \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithGetItem ? DeeObject_TDefaultHasItemIndexWithGetItem(tp_self, self, index) : \
	 (tp_hasitem_index) == &DeeObject_DefaultHasItemIndexWithGetItemIndexDefault ? DeeObject_TDefaultHasItemIndexWithGetItemIndexDefault(tp_self, self, index) : \
	 (tp_hasitem_index) == &DeeSeq_DefaultHasItemIndexWithSize ? DeeSeq_TDefaultHasItemIndexWithSize(tp_self, self, index) : \
	 (tp_hasitem_index) == &DeeSeq_DefaultHasItemIndexWithSizeDefault ? DeeSeq_TDefaultHasItemIndexWithSizeDefault(tp_self, self, index) : \
	 (tp_hasitem_index) == &DeeMap_DefaultHasItemIndexWithContains ? DeeMap_TDefaultHasItemIndexWithContains(tp_self, self, index) : \
	 (tp_hasitem_index) == &DeeMap_DefaultHasItemIndexWithEnumerate ? DeeMap_TDefaultHasItemIndexWithEnumerate(tp_self, self, index) : \
	 (tp_hasitem_index) == &DeeMap_DefaultHasItemIndexWithEnumerateDefault ? DeeMap_TDefaultHasItemIndexWithEnumerateDefault(tp_self, self, index) : \
	 default)
#define DeeType_invoke_seq_tp_hasitem_string_hash_DEFAULT(tp_self, tp_hasitem_string_hash, self, key, hash, default) \
	((tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithHasItemStringLenHash ? DeeObject_TDefaultHasItemStringHashWithHasItemStringLenHash(tp_self, self, key, hash) : \
	 (tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithBoundItemStringHash ? DeeObject_TDefaultHasItemStringHashWithBoundItemStringHash(tp_self, self, key, hash) : \
	 (tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithBoundItemStringLenHash ? DeeObject_TDefaultHasItemStringHashWithBoundItemStringLenHash(tp_self, self, key, hash) : \
	 (tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithTryGetItemStringHash ? DeeObject_TDefaultHasItemStringHashWithTryGetItemStringHash(tp_self, self, key, hash) : \
	 (tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithTryGetItemStringLenHash ? DeeObject_TDefaultHasItemStringHashWithTryGetItemStringLenHash(tp_self, self, key, hash) : \
	 (tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithGetItemStringHash ? DeeObject_TDefaultHasItemStringHashWithGetItemStringHash(tp_self, self, key, hash) : \
	 (tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithGetItemStringLenHash ? DeeObject_TDefaultHasItemStringHashWithGetItemStringLenHash(tp_self, self, key, hash) : \
	 (tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithHasItem ? DeeObject_TDefaultHasItemStringHashWithHasItem(tp_self, self, key, hash) : \
	 (tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithBoundItem ? DeeObject_TDefaultHasItemStringHashWithBoundItem(tp_self, self, key, hash) : \
	 (tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithTryGetItem ? DeeObject_TDefaultHasItemStringHashWithTryGetItem(tp_self, self, key, hash) : \
	 (tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithGetItem ? DeeObject_TDefaultHasItemStringHashWithGetItem(tp_self, self, key, hash) : \
	 (tp_hasitem_string_hash) == &DeeObject_DefaultHasItemStringHashWithHasItemDefault ? DeeObject_TDefaultHasItemStringHashWithHasItemDefault(tp_self, self, key, hash) : \
	 (tp_hasitem_string_hash) == &DeeMap_DefaultHasItemStringHashWithContains ? DeeMap_TDefaultHasItemStringHashWithContains(tp_self, self, key, hash) : \
	 (tp_hasitem_string_hash) == &DeeMap_DefaultHasItemStringHashWithEnumerate ? DeeMap_TDefaultHasItemStringHashWithEnumerate(tp_self, self, key, hash) : \
	 (tp_hasitem_string_hash) == &DeeMap_DefaultHasItemStringHashWithEnumerateDefault ? DeeMap_TDefaultHasItemStringHashWithEnumerateDefault(tp_self, self, key, hash) : \
	 default)
#define DeeType_invoke_seq_tp_hasitem_string_len_hash_DEFAULT(tp_self, tp_hasitem_string_len_hash, self, key, keylen, hash, default) \
	((tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithHasItemStringHash ? DeeObject_TDefaultHasItemStringLenHashWithHasItemStringHash(tp_self, self, key, keylen, hash) : \
	 (tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithBoundItemStringHash ? DeeObject_TDefaultHasItemStringLenHashWithBoundItemStringHash(tp_self, self, key, keylen, hash) : \
	 (tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithBoundItemStringLenHash ? DeeObject_TDefaultHasItemStringLenHashWithBoundItemStringLenHash(tp_self, self, key, keylen, hash) : \
	 (tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithTryGetItemStringLenHash ? DeeObject_TDefaultHasItemStringLenHashWithTryGetItemStringLenHash(tp_self, self, key, keylen, hash) : \
	 (tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithTryGetItemStringHash ? DeeObject_TDefaultHasItemStringLenHashWithTryGetItemStringHash(tp_self, self, key, keylen, hash) : \
	 (tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithGetItemStringLenHash ? DeeObject_TDefaultHasItemStringLenHashWithGetItemStringLenHash(tp_self, self, key, keylen, hash) : \
	 (tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithGetItemStringHash ? DeeObject_TDefaultHasItemStringLenHashWithGetItemStringHash(tp_self, self, key, keylen, hash) : \
	 (tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithHasItem ? DeeObject_TDefaultHasItemStringLenHashWithHasItem(tp_self, self, key, keylen, hash) : \
	 (tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithBoundItem ? DeeObject_TDefaultHasItemStringLenHashWithBoundItem(tp_self, self, key, keylen, hash) : \
	 (tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithTryGetItem ? DeeObject_TDefaultHasItemStringLenHashWithTryGetItem(tp_self, self, key, keylen, hash) : \
	 (tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithGetItem ? DeeObject_TDefaultHasItemStringLenHashWithGetItem(tp_self, self, key, keylen, hash) : \
	 (tp_hasitem_string_len_hash) == &DeeObject_DefaultHasItemStringLenHashWithHasItemDefault ? DeeObject_TDefaultHasItemStringLenHashWithHasItemDefault(tp_self, self, key, keylen, hash) : \
	 (tp_hasitem_string_len_hash) == &DeeMap_DefaultHasItemStringLenHashWithContains ? DeeMap_TDefaultHasItemStringLenHashWithContains(tp_self, self, key, keylen, hash) : \
	 (tp_hasitem_string_len_hash) == &DeeMap_DefaultHasItemStringLenHashWithEnumerate ? DeeMap_TDefaultHasItemStringLenHashWithEnumerate(tp_self, self, key, keylen, hash) : \
	 (tp_hasitem_string_len_hash) == &DeeMap_DefaultHasItemStringLenHashWithEnumerateDefault ? DeeMap_TDefaultHasItemStringLenHashWithEnumerateDefault(tp_self, self, key, keylen, hash) : \
	 default)
#define DeeType_invoke_seq_tp_getrange_DEFAULT(tp_self, tp_getrange, self, start, end, default) \
	((tp_getrange) == &DeeObject_DefaultGetRangeWithGetRangeIndexAndGetRangeIndexN ? DeeObject_TDefaultGetRangeWithGetRangeIndexAndGetRangeIndexN(tp_self, self, start, end) : \
	 (tp_getrange) == &DeeObject_DefaultGetRangeWithGetRangeIndexDefaultAndGetRangeIndexNDefault ? DeeObject_TDefaultGetRangeWithGetRangeIndexDefaultAndGetRangeIndexNDefault(tp_self, self, start, end) : \
	 (tp_getrange) == &DeeSeq_DefaultGetRangeWithSizeDefaultAndGetItemIndex ? DeeSeq_TDefaultGetRangeWithSizeDefaultAndGetItemIndex(tp_self, self, start, end) : \
	 (tp_getrange) == &DeeSeq_DefaultGetRangeWithSizeDefaultAndTryGetItemIndex ? DeeSeq_TDefaultGetRangeWithSizeDefaultAndTryGetItemIndex(tp_self, self, start, end) : \
	 (tp_getrange) == &DeeSeq_DefaultGetRangeWithSizeObAndGetItem ? DeeSeq_TDefaultGetRangeWithSizeObAndGetItem(tp_self, self, start, end) : \
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
	 (tp_delrange_index) == &DeeSeq_DefaultDelRangeIndexWithSizeDefaultAndTSCErase ? DeeSeq_TDefaultDelRangeIndexWithSizeDefaultAndTSCErase(tp_self, self, start, end) : \
	 (tp_delrange_index) == &DeeSeq_DefaultDelRangeIndexWithSizeDefaultAndDelItemIndexDefault ? DeeSeq_TDefaultDelRangeIndexWithSizeDefaultAndDelItemIndexDefault(tp_self, self, start, end) : \
	 default)
#define DeeType_invoke_seq_tp_delrange_index_n_DEFAULT(tp_self, tp_delrange_index_n, self, start, default) \
	((tp_delrange_index_n) == &DeeObject_DefaultDelRangeIndexNWithDelRange ? DeeObject_TDefaultDelRangeIndexNWithDelRange(tp_self, self, start) : \
	 (tp_delrange_index_n) == &DeeSeq_DefaultDelRangeIndexNWithSizeAndDelRangeIndex ? DeeSeq_TDefaultDelRangeIndexNWithSizeAndDelRangeIndex(tp_self, self, start) : \
	 (tp_delrange_index_n) == &DeeSeq_DefaultDelRangeIndexNWithSizeDefaultAndDelRangeIndex ? DeeSeq_TDefaultDelRangeIndexNWithSizeDefaultAndDelRangeIndex(tp_self, self, start) : \
	 (tp_delrange_index_n) == &DeeSeq_DefaultDelRangeIndexNWithSetRangeIndexNNone ? DeeSeq_TDefaultDelRangeIndexNWithSetRangeIndexNNone(tp_self, self, start) : \
	 (tp_delrange_index_n) == &DeeSeq_DefaultDelRangeIndexNWithSetRangeIndexNNoneDefault ? DeeSeq_TDefaultDelRangeIndexNWithSetRangeIndexNNoneDefault(tp_self, self, start) : \
	 (tp_delrange_index_n) == &DeeSeq_DefaultDelRangeIndexNWithSizeDefaultAndTSCErase ? DeeSeq_TDefaultDelRangeIndexNWithSizeDefaultAndTSCErase(tp_self, self, start) : \
	 (tp_delrange_index_n) == &DeeSeq_DefaultDelRangeIndexNWithSizeDefaultAndDelItemIndexDefault ? DeeSeq_TDefaultDelRangeIndexNWithSizeDefaultAndDelItemIndexDefault(tp_self, self, start) : \
	 default)
#define DeeType_invoke_seq_tp_setrange_DEFAULT(tp_self, tp_setrange, self, start, end, value, default) \
	((tp_setrange) == &DeeObject_DefaultSetRangeWithSetRangeIndexAndSetRangeIndexN ? DeeObject_TDefaultSetRangeWithSetRangeIndexAndSetRangeIndexN(tp_self, self, start, end, value) : \
	 (tp_setrange) == &DeeObject_DefaultSetRangeWithSetRangeIndexDefaultAndSetRangeIndexNDefault ? DeeObject_TDefaultSetRangeWithSetRangeIndexDefaultAndSetRangeIndexNDefault(tp_self, self, start, end, value) : \
	 default)
#define DeeType_invoke_seq_tp_setrange_index_DEFAULT(tp_self, tp_setrange_index, self, start, end, value, default) \
	((tp_setrange_index) == &DeeObject_DefaultSetRangeIndexWithSetRange ? DeeObject_TDefaultSetRangeIndexWithSetRange(tp_self, self, start, end, value) : \
	 (tp_setrange_index) == &DeeSeq_DefaultSetRangeIndexWithSizeDefaultAndTSCEraseAndTSCInsertAll ? DeeSeq_TDefaultSetRangeIndexWithSizeDefaultAndTSCEraseAndTSCInsertAll(tp_self, self, start, end, value) : \
	 default)
#define DeeType_invoke_seq_tp_setrange_index_n_DEFAULT(tp_self, tp_setrange_index_n, self, start, value, default) \
	((tp_setrange_index_n) == &DeeObject_DefaultSetRangeIndexNWithSetRange ? DeeObject_TDefaultSetRangeIndexNWithSetRange(tp_self, self, start, value) : \
	 (tp_setrange_index_n) == &DeeSeq_DefaultSetRangeIndexNWithSizeAndSetRangeIndex ? DeeSeq_TDefaultSetRangeIndexNWithSizeAndSetRangeIndex(tp_self, self, start, value) : \
	 (tp_setrange_index_n) == &DeeSeq_DefaultSetRangeIndexNWithSizeDefaultAndSetRangeIndex ? DeeSeq_TDefaultSetRangeIndexNWithSizeDefaultAndSetRangeIndex(tp_self, self, start, value) : \
	 (tp_setrange_index_n) == &DeeSeq_DefaultSetRangeIndexNWithSizeDefaultAndSetRangeIndexDefault ? DeeSeq_TDefaultSetRangeIndexNWithSizeDefaultAndSetRangeIndexDefault(tp_self, self, start, value) : \
	 default)
#define DeeType_invoke_seq_tp_unpack_DEFAULT(tp_self, tp_unpack, self, dst_length, dst, default) \
	((tp_unpack) == &DeeSeq_DefaultUnpackWithUnpackEx ? DeeSeq_TDefaultUnpackWithUnpackEx(tp_self, self, dst_length, dst) : \
	 (tp_unpack) == &DeeSeq_DefaultUnpackWithAsVector ? DeeSeq_TDefaultUnpackWithAsVector(tp_self, self, dst_length, dst) : \
	 (tp_unpack) == &DeeSeq_DefaultUnpackWithSizeAndGetItemIndexFast ? DeeSeq_TDefaultUnpackWithSizeAndGetItemIndexFast(tp_self, self, dst_length, dst) : \
	 (tp_unpack) == &DeeSeq_DefaultUnpackWithSizeAndTryGetItemIndex ? DeeSeq_TDefaultUnpackWithSizeAndTryGetItemIndex(tp_self, self, dst_length, dst) : \
	 (tp_unpack) == &DeeSeq_DefaultUnpackWithSizeAndGetItemIndex ? DeeSeq_TDefaultUnpackWithSizeAndGetItemIndex(tp_self, self, dst_length, dst) : \
	 (tp_unpack) == &DeeSeq_DefaultUnpackWithSizeDefaultAndTryGetItemIndexDefault ? DeeSeq_TDefaultUnpackWithSizeDefaultAndTryGetItemIndexDefault(tp_self, self, dst_length, dst) : \
	 (tp_unpack) == &DeeSeq_DefaultUnpackWithSizeDefaultAndGetItemIndexDefault ? DeeSeq_TDefaultUnpackWithSizeDefaultAndGetItemIndexDefault(tp_self, self, dst_length, dst) : \
	 (tp_unpack) == &DeeSeq_DefaultUnpackWithForeach ? DeeSeq_TDefaultUnpackWithForeach(tp_self, self, dst_length, dst) : \
	 (tp_unpack) == &DeeSeq_DefaultUnpackWithIter ? DeeSeq_TDefaultUnpackWithIter(tp_self, self, dst_length, dst) : \
	 (tp_unpack) == &DeeSeq_DefaultUnpackWithForeachDefault ? DeeSeq_TDefaultUnpackWithForeachDefault(tp_self, self, dst_length, dst) : \
	 default)
#define DeeType_invoke_seq_tp_unpack_ub_DEFAULT(tp_self, tp_unpack_ub, self, dst_length, dst, default) \
	((tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithSizeAndGetItemIndexFast ? DeeSeq_TDefaultUnpackUbWithSizeAndGetItemIndexFast(tp_self, self, dst_length, dst) : \
	 (tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithSizeAndTryGetItemIndex ? DeeSeq_TDefaultUnpackUbWithSizeAndTryGetItemIndex(tp_self, self, dst_length, dst) : \
	 (tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithSizeAndGetItemIndex ? DeeSeq_TDefaultUnpackUbWithSizeAndGetItemIndex(tp_self, self, dst_length, dst) : \
	 (tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithSizeDefaultAndTryGetItemIndexDefault ? DeeSeq_TDefaultUnpackUbWithSizeDefaultAndTryGetItemIndexDefault(tp_self, self, dst_length, dst) : \
	 (tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithSizeDefaultAndGetItemIndexDefault ? DeeSeq_TDefaultUnpackUbWithSizeDefaultAndGetItemIndexDefault(tp_self, self, dst_length, dst) : \
	 (tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithSizeDefaultAndEnumerateIndex ? DeeSeq_TDefaultUnpackUbWithSizeDefaultAndEnumerateIndex(tp_self, self, dst_length, dst) : \
	 (tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithSizeDefaultAndEnumerateIndexDefault ? DeeSeq_TDefaultUnpackUbWithSizeDefaultAndEnumerateIndexDefault(tp_self, self, dst_length, dst) : \
	 (tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithUnpackEx ? DeeSeq_TDefaultUnpackUbWithUnpackEx(tp_self, self, dst_length, dst) : \
	 (tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithAsVector ? DeeSeq_TDefaultUnpackUbWithAsVector(tp_self, self, dst_length, dst) : \
	 (tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithForeach ? DeeSeq_TDefaultUnpackUbWithForeach(tp_self, self, dst_length, dst) : \
	 (tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithIter ? DeeSeq_TDefaultUnpackUbWithIter(tp_self, self, dst_length, dst) : \
	 (tp_unpack_ub) == &DeeSeq_DefaultUnpackUbWithForeachDefault ? DeeSeq_TDefaultUnpackUbWithForeachDefault(tp_self, self, dst_length, dst) : \
	 default)
#define DeeType_invoke_seq_tp_unpack_ex_DEFAULT(tp_self, tp_unpack_ex, self, dst_length_min, dst_length_max, dst, default) \
	((tp_unpack_ex) == &DeeSeq_DefaultUnpackExWithAsVector ? DeeSeq_TDefaultUnpackExWithAsVector(tp_self, self, dst_length_min, dst_length_max, dst) : \
	 (tp_unpack_ex) == &DeeSeq_DefaultUnpackExWithSizeAndGetItemIndexFast ? DeeSeq_TDefaultUnpackExWithSizeAndGetItemIndexFast(tp_self, self, dst_length_min, dst_length_max, dst) : \
	 (tp_unpack_ex) == &DeeSeq_DefaultUnpackExWithSizeAndTryGetItemIndex ? DeeSeq_TDefaultUnpackExWithSizeAndTryGetItemIndex(tp_self, self, dst_length_min, dst_length_max, dst) : \
	 (tp_unpack_ex) == &DeeSeq_DefaultUnpackExWithSizeAndGetItemIndex ? DeeSeq_TDefaultUnpackExWithSizeAndGetItemIndex(tp_self, self, dst_length_min, dst_length_max, dst) : \
	 (tp_unpack_ex) == &DeeSeq_DefaultUnpackExWithSizeDefaultAndTryGetItemIndexDefault ? DeeSeq_TDefaultUnpackExWithSizeDefaultAndTryGetItemIndexDefault(tp_self, self, dst_length_min, dst_length_max, dst) : \
	 (tp_unpack_ex) == &DeeSeq_DefaultUnpackExWithSizeDefaultAndGetItemIndexDefault ? DeeSeq_TDefaultUnpackExWithSizeDefaultAndGetItemIndexDefault(tp_self, self, dst_length_min, dst_length_max, dst) : \
	 (tp_unpack_ex) == &DeeSeq_DefaultUnpackExWithForeach ? DeeSeq_TDefaultUnpackExWithForeach(tp_self, self, dst_length_min, dst_length_max, dst) : \
	 (tp_unpack_ex) == &DeeSeq_DefaultUnpackExWithIter ? DeeSeq_TDefaultUnpackExWithIter(tp_self, self, dst_length_min, dst_length_max, dst) : \
	 (tp_unpack_ex) == &DeeSeq_DefaultUnpackExWithForeachDefault ? DeeSeq_TDefaultUnpackExWithForeachDefault(tp_self, self, dst_length_min, dst_length_max, dst) : \
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
#define DeeType_invoke_cast_tp_bool(tp_self, tp_bool, self) \
	 DeeType_invoke_cast_tp_bool_DEFAULT(tp_self, tp_bool, self, DeeType_invoke_cast_tp_bool_NODEFAULT(tp_self, tp_bool, self))
#define DeeType_invoke_cmp_tp_hash(tp_self, tp_hash, self) \
	 DeeType_invoke_cmp_tp_hash_DEFAULT(tp_self, tp_hash, self, DeeType_invoke_cmp_tp_hash_NODEFAULT(tp_self, tp_hash, self))
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
#define DeeType_invoke_cmp_tp_compare_eq(tp_self, tp_compare_eq, self, other) \
	 DeeType_invoke_cmp_tp_compare_eq_DEFAULT(tp_self, tp_compare_eq, self, other, DeeType_invoke_cmp_tp_compare_eq_NODEFAULT(tp_self, tp_compare_eq, self, other))
#define DeeType_invoke_cmp_tp_trycompare_eq(tp_self, tp_trycompare_eq, self, other) \
	 DeeType_invoke_cmp_tp_trycompare_eq_DEFAULT(tp_self, tp_trycompare_eq, self, other, DeeType_invoke_cmp_tp_trycompare_eq_NODEFAULT(tp_self, tp_trycompare_eq, self, other))
#define DeeType_invoke_cmp_tp_compare(tp_self, tp_compare, self, other) \
	 DeeType_invoke_cmp_tp_compare_DEFAULT(tp_self, tp_compare, self, other, DeeType_invoke_cmp_tp_compare_NODEFAULT(tp_self, tp_compare, self, other))
#define DeeType_invoke_tp_iter_next(tp_self, tp_iter_next, self) \
	 DeeType_invoke_tp_iter_next_DEFAULT(tp_self, tp_iter_next, self, DeeType_invoke_tp_iter_next_NODEFAULT(tp_self, tp_iter_next, self))
#define DeeType_invoke_iterator_tp_nextpair(tp_self, tp_nextpair, self, key_and_value) \
	 DeeType_invoke_iterator_tp_nextpair_DEFAULT(tp_self, tp_nextpair, self, key_and_value, DeeType_invoke_iterator_tp_nextpair_NODEFAULT(tp_self, tp_nextpair, self, key_and_value))
#define DeeType_invoke_iterator_tp_nextkey(tp_self, tp_nextkey, self) \
	 DeeType_invoke_iterator_tp_nextkey_DEFAULT(tp_self, tp_nextkey, self, DeeType_invoke_iterator_tp_nextkey_NODEFAULT(tp_self, tp_nextkey, self))
#define DeeType_invoke_iterator_tp_nextvalue(tp_self, tp_nextvalue, self) \
	 DeeType_invoke_iterator_tp_nextvalue_DEFAULT(tp_self, tp_nextvalue, self, DeeType_invoke_iterator_tp_nextvalue_NODEFAULT(tp_self, tp_nextvalue, self))
#define DeeType_invoke_iterator_tp_advance(tp_self, tp_advance, self, step) \
	 DeeType_invoke_iterator_tp_advance_DEFAULT(tp_self, tp_advance, self, step, DeeType_invoke_iterator_tp_advance_NODEFAULT(tp_self, tp_advance, self, step))
#define DeeType_invoke_seq_tp_iter(tp_self, tp_iter, self) \
	 DeeType_invoke_seq_tp_iter_DEFAULT(tp_self, tp_iter, self, DeeType_invoke_seq_tp_iter_NODEFAULT(tp_self, tp_iter, self))
#define DeeType_invoke_seq_tp_foreach(tp_self, tp_foreach, self, proc, arg) \
	 DeeType_invoke_seq_tp_foreach_DEFAULT(tp_self, tp_foreach, self, proc, arg, DeeType_invoke_seq_tp_foreach_NODEFAULT(tp_self, tp_foreach, self, proc, arg))
#define DeeType_invoke_seq_tp_foreach_pair(tp_self, tp_foreach_pair, self, proc, arg) \
	 DeeType_invoke_seq_tp_foreach_pair_DEFAULT(tp_self, tp_foreach_pair, self, proc, arg, DeeType_invoke_seq_tp_foreach_pair_NODEFAULT(tp_self, tp_foreach_pair, self, proc, arg))
#define DeeType_invoke_seq_tp_enumerate(tp_self, tp_enumerate, self, proc, arg) \
	 DeeType_invoke_seq_tp_enumerate_DEFAULT(tp_self, tp_enumerate, self, proc, arg, DeeType_invoke_seq_tp_enumerate_NODEFAULT(tp_self, tp_enumerate, self, proc, arg))
#define DeeType_invoke_seq_tp_enumerate_index(tp_self, tp_enumerate_index, self, proc, arg, start, end) \
	 DeeType_invoke_seq_tp_enumerate_index_DEFAULT(tp_self, tp_enumerate_index, self, proc, arg, start, end, DeeType_invoke_seq_tp_enumerate_index_NODEFAULT(tp_self, tp_enumerate_index, self, proc, arg, start, end))
#define DeeType_invoke_seq_tp_iterkeys(tp_self, tp_iterkeys, self) \
	 DeeType_invoke_seq_tp_iterkeys_DEFAULT(tp_self, tp_iterkeys, self, DeeType_invoke_seq_tp_iterkeys_NODEFAULT(tp_self, tp_iterkeys, self))
#define DeeType_invoke_seq_tp_sizeob(tp_self, tp_sizeob, self) \
	 DeeType_invoke_seq_tp_sizeob_DEFAULT(tp_self, tp_sizeob, self, DeeType_invoke_seq_tp_sizeob_NODEFAULT(tp_self, tp_sizeob, self))
#define DeeType_invoke_seq_tp_size(tp_self, tp_size, self) \
	 DeeType_invoke_seq_tp_size_DEFAULT(tp_self, tp_size, self, DeeType_invoke_seq_tp_size_NODEFAULT(tp_self, tp_size, self))
#define DeeType_invoke_seq_tp_size_fast(tp_self, tp_size_fast, self) \
	 DeeType_invoke_seq_tp_size_fast_DEFAULT(tp_self, tp_size_fast, self, DeeType_invoke_seq_tp_size_fast_NODEFAULT(tp_self, tp_size_fast, self))
#define DeeType_invoke_seq_tp_contains(tp_self, tp_contains, self, elem) \
	 DeeType_invoke_seq_tp_contains_DEFAULT(tp_self, tp_contains, self, elem, DeeType_invoke_seq_tp_contains_NODEFAULT(tp_self, tp_contains, self, elem))
#define DeeType_invoke_seq_tp_getitem(tp_self, tp_getitem, self, index) \
	 DeeType_invoke_seq_tp_getitem_DEFAULT(tp_self, tp_getitem, self, index, DeeType_invoke_seq_tp_getitem_NODEFAULT(tp_self, tp_getitem, self, index))
#define DeeType_invoke_seq_tp_getitem_index(tp_self, tp_getitem_index, self, index) \
	 DeeType_invoke_seq_tp_getitem_index_DEFAULT(tp_self, tp_getitem_index, self, index, DeeType_invoke_seq_tp_getitem_index_NODEFAULT(tp_self, tp_getitem_index, self, index))
#define DeeType_invoke_seq_tp_getitem_string_hash(tp_self, tp_getitem_string_hash, self, key, hash) \
	 DeeType_invoke_seq_tp_getitem_string_hash_DEFAULT(tp_self, tp_getitem_string_hash, self, key, hash, DeeType_invoke_seq_tp_getitem_string_hash_NODEFAULT(tp_self, tp_getitem_string_hash, self, key, hash))
#define DeeType_invoke_seq_tp_getitem_string_len_hash(tp_self, tp_getitem_string_len_hash, self, key, keylen, hash) \
	 DeeType_invoke_seq_tp_getitem_string_len_hash_DEFAULT(tp_self, tp_getitem_string_len_hash, self, key, keylen, hash, DeeType_invoke_seq_tp_getitem_string_len_hash_NODEFAULT(tp_self, tp_getitem_string_len_hash, self, key, keylen, hash))
#define DeeType_invoke_seq_tp_trygetitem(tp_self, tp_trygetitem, self, index) \
	 DeeType_invoke_seq_tp_trygetitem_DEFAULT(tp_self, tp_trygetitem, self, index, DeeType_invoke_seq_tp_trygetitem_NODEFAULT(tp_self, tp_trygetitem, self, index))
#define DeeType_invoke_seq_tp_trygetitem_index(tp_self, tp_trygetitem_index, self, index) \
	 DeeType_invoke_seq_tp_trygetitem_index_DEFAULT(tp_self, tp_trygetitem_index, self, index, DeeType_invoke_seq_tp_trygetitem_index_NODEFAULT(tp_self, tp_trygetitem_index, self, index))
#define DeeType_invoke_seq_tp_trygetitem_string_hash(tp_self, tp_trygetitem_string_hash, self, key, hash) \
	 DeeType_invoke_seq_tp_trygetitem_string_hash_DEFAULT(tp_self, tp_trygetitem_string_hash, self, key, hash, DeeType_invoke_seq_tp_trygetitem_string_hash_NODEFAULT(tp_self, tp_trygetitem_string_hash, self, key, hash))
#define DeeType_invoke_seq_tp_trygetitem_string_len_hash(tp_self, tp_trygetitem_string_len_hash, self, key, keylen, hash) \
	 DeeType_invoke_seq_tp_trygetitem_string_len_hash_DEFAULT(tp_self, tp_trygetitem_string_len_hash, self, key, keylen, hash, DeeType_invoke_seq_tp_trygetitem_string_len_hash_NODEFAULT(tp_self, tp_trygetitem_string_len_hash, self, key, keylen, hash))
#define DeeType_invoke_seq_tp_delitem(tp_self, tp_delitem, self, index) \
	 DeeType_invoke_seq_tp_delitem_DEFAULT(tp_self, tp_delitem, self, index, DeeType_invoke_seq_tp_delitem_NODEFAULT(tp_self, tp_delitem, self, index))
#define DeeType_invoke_seq_tp_delitem_index(tp_self, tp_delitem_index, self, index) \
	 DeeType_invoke_seq_tp_delitem_index_DEFAULT(tp_self, tp_delitem_index, self, index, DeeType_invoke_seq_tp_delitem_index_NODEFAULT(tp_self, tp_delitem_index, self, index))
#define DeeType_invoke_seq_tp_delitem_string_hash(tp_self, tp_delitem_string_hash, self, key, hash) \
	 DeeType_invoke_seq_tp_delitem_string_hash_DEFAULT(tp_self, tp_delitem_string_hash, self, key, hash, DeeType_invoke_seq_tp_delitem_string_hash_NODEFAULT(tp_self, tp_delitem_string_hash, self, key, hash))
#define DeeType_invoke_seq_tp_delitem_string_len_hash(tp_self, tp_delitem_string_len_hash, self, key, keylen, hash) \
	 DeeType_invoke_seq_tp_delitem_string_len_hash_DEFAULT(tp_self, tp_delitem_string_len_hash, self, key, keylen, hash, DeeType_invoke_seq_tp_delitem_string_len_hash_NODEFAULT(tp_self, tp_delitem_string_len_hash, self, key, keylen, hash))
#define DeeType_invoke_seq_tp_setitem(tp_self, tp_setitem, self, index, value) \
	 DeeType_invoke_seq_tp_setitem_DEFAULT(tp_self, tp_setitem, self, index, value, DeeType_invoke_seq_tp_setitem_NODEFAULT(tp_self, tp_setitem, self, index, value))
#define DeeType_invoke_seq_tp_setitem_index(tp_self, tp_setitem_index, self, index, value) \
	 DeeType_invoke_seq_tp_setitem_index_DEFAULT(tp_self, tp_setitem_index, self, index, value, DeeType_invoke_seq_tp_setitem_index_NODEFAULT(tp_self, tp_setitem_index, self, index, value))
#define DeeType_invoke_seq_tp_setitem_string_hash(tp_self, tp_setitem_string_hash, self, key, hash, value) \
	 DeeType_invoke_seq_tp_setitem_string_hash_DEFAULT(tp_self, tp_setitem_string_hash, self, key, hash, value, DeeType_invoke_seq_tp_setitem_string_hash_NODEFAULT(tp_self, tp_setitem_string_hash, self, key, hash, value))
#define DeeType_invoke_seq_tp_setitem_string_len_hash(tp_self, tp_setitem_string_len_hash, self, key, keylen, hash, value) \
	 DeeType_invoke_seq_tp_setitem_string_len_hash_DEFAULT(tp_self, tp_setitem_string_len_hash, self, key, keylen, hash, value, DeeType_invoke_seq_tp_setitem_string_len_hash_NODEFAULT(tp_self, tp_setitem_string_len_hash, self, key, keylen, hash, value))
#define DeeType_invoke_seq_tp_bounditem(tp_self, tp_bounditem, self, index) \
	 DeeType_invoke_seq_tp_bounditem_DEFAULT(tp_self, tp_bounditem, self, index, DeeType_invoke_seq_tp_bounditem_NODEFAULT(tp_self, tp_bounditem, self, index))
#define DeeType_invoke_seq_tp_bounditem_index(tp_self, tp_bounditem_index, self, index) \
	 DeeType_invoke_seq_tp_bounditem_index_DEFAULT(tp_self, tp_bounditem_index, self, index, DeeType_invoke_seq_tp_bounditem_index_NODEFAULT(tp_self, tp_bounditem_index, self, index))
#define DeeType_invoke_seq_tp_bounditem_string_hash(tp_self, tp_bounditem_string_hash, self, key, hash) \
	 DeeType_invoke_seq_tp_bounditem_string_hash_DEFAULT(tp_self, tp_bounditem_string_hash, self, key, hash, DeeType_invoke_seq_tp_bounditem_string_hash_NODEFAULT(tp_self, tp_bounditem_string_hash, self, key, hash))
#define DeeType_invoke_seq_tp_bounditem_string_len_hash(tp_self, tp_bounditem_string_len_hash, self, key, keylen, hash) \
	 DeeType_invoke_seq_tp_bounditem_string_len_hash_DEFAULT(tp_self, tp_bounditem_string_len_hash, self, key, keylen, hash, DeeType_invoke_seq_tp_bounditem_string_len_hash_NODEFAULT(tp_self, tp_bounditem_string_len_hash, self, key, keylen, hash))
#define DeeType_invoke_seq_tp_hasitem(tp_self, tp_hasitem, self, index) \
	 DeeType_invoke_seq_tp_hasitem_DEFAULT(tp_self, tp_hasitem, self, index, DeeType_invoke_seq_tp_hasitem_NODEFAULT(tp_self, tp_hasitem, self, index))
#define DeeType_invoke_seq_tp_hasitem_index(tp_self, tp_hasitem_index, self, index) \
	 DeeType_invoke_seq_tp_hasitem_index_DEFAULT(tp_self, tp_hasitem_index, self, index, DeeType_invoke_seq_tp_hasitem_index_NODEFAULT(tp_self, tp_hasitem_index, self, index))
#define DeeType_invoke_seq_tp_hasitem_string_hash(tp_self, tp_hasitem_string_hash, self, key, hash) \
	 DeeType_invoke_seq_tp_hasitem_string_hash_DEFAULT(tp_self, tp_hasitem_string_hash, self, key, hash, DeeType_invoke_seq_tp_hasitem_string_hash_NODEFAULT(tp_self, tp_hasitem_string_hash, self, key, hash))
#define DeeType_invoke_seq_tp_hasitem_string_len_hash(tp_self, tp_hasitem_string_len_hash, self, key, keylen, hash) \
	 DeeType_invoke_seq_tp_hasitem_string_len_hash_DEFAULT(tp_self, tp_hasitem_string_len_hash, self, key, keylen, hash, DeeType_invoke_seq_tp_hasitem_string_len_hash_NODEFAULT(tp_self, tp_hasitem_string_len_hash, self, key, keylen, hash))
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
#define DeeType_invoke_seq_tp_unpack(tp_self, tp_unpack, self, dst_length, dst) \
	 DeeType_invoke_seq_tp_unpack_DEFAULT(tp_self, tp_unpack, self, dst_length, dst, DeeType_invoke_seq_tp_unpack_NODEFAULT(tp_self, tp_unpack, self, dst_length, dst))
#define DeeType_invoke_seq_tp_unpack_ub(tp_self, tp_unpack_ub, self, dst_length, dst) \
	 DeeType_invoke_seq_tp_unpack_ub_DEFAULT(tp_self, tp_unpack_ub, self, dst_length, dst, DeeType_invoke_seq_tp_unpack_ub_NODEFAULT(tp_self, tp_unpack_ub, self, dst_length, dst))
#define DeeType_invoke_seq_tp_unpack_ex(tp_self, tp_unpack_ex, self, dst_length_min, dst_length_max, dst) \
	 DeeType_invoke_seq_tp_unpack_ex_DEFAULT(tp_self, tp_unpack_ex, self, dst_length_min, dst_length_max, dst, DeeType_invoke_seq_tp_unpack_ex_NODEFAULT(tp_self, tp_unpack_ex, self, dst_length_min, dst_length_max, dst))
/*[[[end]]]*/
/* clang-format on */
#endif /* !CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

#ifndef CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS
#define DeeType_invoke_init_tp_assign_DEFAULT(tp_self, tp_assign, self, other, default) default
#define DeeType_invoke_init_tp_assign(tp_self, tp_assign, self, other) \
	DeeType_invoke_init_tp_assign_DEFAULT(tp_self, tp_assign, self, other, DeeType_invoke_init_tp_assign_NODEFAULT(tp_self, tp_assign, self, other))
#define DeeType_invoke_init_tp_move_assign_DEFAULT(tp_self, tp_move_assign, self, other, default) default
#define DeeType_invoke_init_tp_move_assign(tp_self, tp_move_assign, self, other) \
	DeeType_invoke_init_tp_move_assign_DEFAULT(tp_self, tp_move_assign, self, other, DeeType_invoke_init_tp_move_assign_NODEFAULT(tp_self, tp_move_assign, self, other))
#define DeeType_invoke_math_tp_inv_DEFAULT(tp_self, tp_inv, self, default) default
#define DeeType_invoke_math_tp_inv(tp_self, tp_inv, self) \
	DeeType_invoke_math_tp_inv_DEFAULT(tp_self, tp_inv, self, DeeType_invoke_math_tp_inv_NODEFAULT(tp_self, tp_inv, self))
#define DeeType_invoke_math_tp_pos_DEFAULT(tp_self, tp_pos, self, default) default
#define DeeType_invoke_math_tp_pos(tp_self, tp_pos, self) \
	DeeType_invoke_math_tp_pos_DEFAULT(tp_self, tp_pos, self, DeeType_invoke_math_tp_pos_NODEFAULT(tp_self, tp_pos, self))
#define DeeType_invoke_math_tp_neg_DEFAULT(tp_self, tp_neg, self, default) default
#define DeeType_invoke_math_tp_neg(tp_self, tp_neg, self) \
	DeeType_invoke_math_tp_neg_DEFAULT(tp_self, tp_neg, self, DeeType_invoke_math_tp_neg_NODEFAULT(tp_self, tp_neg, self))
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

#define DeeType_InvokeInitAssign(tp_self, self, other)                                           DeeType_invoke_init_tp_assign(tp_self, (tp_self)->tp_init.tp_assign, self, other)
#define DeeType_InvokeInitAssign_NODEFAULT(tp_self, self, other)                                 DeeType_invoke_init_tp_assign_NODEFAULT(tp_self, (tp_self)->tp_init.tp_assign, self, other)
#define DeeType_InvokeInitMoveAssign(tp_self, self, other)                                       DeeType_invoke_init_tp_move_assign(tp_self, (tp_self)->tp_init.tp_move_assign, self, other)
#define DeeType_InvokeInitMoveAssign_NODEFAULT(tp_self, self, other)                             DeeType_invoke_init_tp_move_assign_NODEFAULT(tp_self, (tp_self)->tp_init.tp_move_assign, self, other)
#define DeeType_InvokeCastStr(tp_self, self)                                                     DeeType_invoke_cast_tp_str(tp_self, (tp_self)->tp_cast.tp_str, self)
#define DeeType_InvokeCastStr_NODEFAULT(tp_self, self)                                           DeeType_invoke_cast_tp_str_NODEFAULT(tp_self, (tp_self)->tp_cast.tp_str, self)
#define DeeType_InvokeCastPrint(tp_self, self, printer, arg)                                     DeeType_invoke_cast_tp_print(tp_self, (tp_self)->tp_cast.tp_print, self, printer, arg)
#define DeeType_InvokeCastPrint_NODEFAULT(tp_self, self, printer, arg)                           DeeType_invoke_cast_tp_print_NODEFAULT(tp_self, (tp_self)->tp_cast.tp_print, self, printer, arg)
#define DeeType_InvokeCastRepr(tp_self, self)                                                    DeeType_invoke_cast_tp_repr(tp_self, (tp_self)->tp_cast.tp_repr, self)
#define DeeType_InvokeCastRepr_NODEFAULT(tp_self, self)                                          DeeType_invoke_cast_tp_repr_NODEFAULT(tp_self, (tp_self)->tp_cast.tp_repr, self)
#define DeeType_InvokeCastPrintRepr(tp_self, self, printer, arg)                                 DeeType_invoke_cast_tp_printrepr(tp_self, (tp_self)->tp_cast.tp_printrepr, self, printer, arg)
#define DeeType_InvokeCastPrintRepr_NODEFAULT(tp_self, self, printer, arg)                       DeeType_invoke_cast_tp_printrepr_NODEFAULT(tp_self, (tp_self)->tp_cast.tp_printrepr, self, printer, arg)
#define DeeType_InvokeCastBool(tp_self, self)                                                    DeeType_invoke_cast_tp_bool(tp_self, (tp_self)->tp_cast.tp_bool, self)
#define DeeType_InvokeCastBool_NODEFAULT(tp_self, self)                                          DeeType_invoke_cast_tp_bool_NODEFAULT(tp_self, (tp_self)->tp_cast.tp_bool, self)
#define DeeType_InvokeIterNext(tp_self, self)                                                    DeeType_invoke_tp_iter_next(tp_self, (tp_self)->tp_iter_next, self)
#define DeeType_InvokeIterNext_NODEFAULT(tp_self, self)                                          DeeType_invoke_tp_iter_next_NODEFAULT(tp_self, (tp_self)->tp_iter_next, self)
#define DeeType_InvokeIterNextPair(tp_self, self, key_and_value)                                 DeeType_invoke_iterator_tp_nextpair(tp_self, (tp_self)->tp_iterator->tp_nextpair, self, key_and_value)
#define DeeType_InvokeIterNextPair_NODEFAULT(tp_self, self, key_and_value)                       DeeType_invoke_iterator_tp_nextpair_NODEFAULT(tp_self, (tp_self)->tp_iterator->tp_nextpair, self, key_and_value)
#define DeeType_InvokeIterNextKey(tp_self, self)                                                 DeeType_invoke_iterator_tp_nextkey(tp_self, (tp_self)->tp_iterator->tp_nextkey, self)
#define DeeType_InvokeIterNextKey_NODEFAULT(tp_self, self)                                       DeeType_invoke_iterator_tp_nextkey_NODEFAULT(tp_self, (tp_self)->tp_iterator->tp_nextkey, self)
#define DeeType_InvokeIterNextValue(tp_self, self)                                               DeeType_invoke_iterator_tp_nextvalue(tp_self, (tp_self)->tp_iterator->tp_nextvalue, self)
#define DeeType_InvokeIterNextValue_NODEFAULT(tp_self, self)                                     DeeType_invoke_iterator_tp_nextvalue_NODEFAULT(tp_self, (tp_self)->tp_iterator->tp_nextvalue, self)
#define DeeType_InvokeIterAdvance(tp_self, self, step)                                           DeeType_invoke_iterator_tp_advance(tp_self, (tp_self)->tp_iterator->tp_advance, self, step)
#define DeeType_InvokeIterAdvance_NODEFAULT(tp_self, self, step)                                 DeeType_invoke_iterator_tp_advance_NODEFAULT(tp_self, (tp_self)->tp_iterator->tp_advance, self, step)
#define DeeType_InvokeCall(tp_self, self, argc, argv)                                            DeeType_invoke_tp_call(tp_self, (tp_self)->tp_call, self, argc, argv)
#define DeeType_InvokeCall_NODEFAULT(tp_self, self, argc, argv)                                  DeeType_invoke_tp_call_NODEFAULT(tp_self, (tp_self)->tp_call, self, argc, argv)
#define DeeType_InvokeCallKw(tp_self, self, argc, argv, kw)                                      DeeType_invoke_tp_call_kw(tp_self, (tp_self)->tp_call_kw, self, argc, argv, kw)
#define DeeType_InvokeCallKw_NODEFAULT(tp_self, self, argc, argv, kw)                            DeeType_invoke_tp_call_kw_NODEFAULT(tp_self, (tp_self)->tp_call_kw, self, argc, argv, kw)
#define DeeType_InvokeMathInt32(tp_self, self, result)                                           DeeType_invoke_math_tp_int32(tp_self, (tp_self)->tp_math->tp_int32, self, result)
#define DeeType_InvokeMathInt32_NODEFAULT(tp_self, self, result)                                 DeeType_invoke_math_tp_int32_NODEFAULT(tp_self, (tp_self)->tp_math->tp_int32, self, result)
#define DeeType_InvokeMathInt64(tp_self, self, result)                                           DeeType_invoke_math_tp_int64(tp_self, (tp_self)->tp_math->tp_int64, self, result)
#define DeeType_InvokeMathInt64_NODEFAULT(tp_self, self, result)                                 DeeType_invoke_math_tp_int64_NODEFAULT(tp_self, (tp_self)->tp_math->tp_int64, self, result)
#define DeeType_InvokeMathDouble(tp_self, self, result)                                          DeeType_invoke_math_tp_double(tp_self, (tp_self)->tp_math->tp_double, self, result)
#define DeeType_InvokeMathDouble_NODEFAULT(tp_self, self, result)                                DeeType_invoke_math_tp_double_NODEFAULT(tp_self, (tp_self)->tp_math->tp_double, self, result)
#define DeeType_InvokeMathInt(tp_self, self)                                                     DeeType_invoke_math_tp_int(tp_self, (tp_self)->tp_math->tp_int, self)
#define DeeType_InvokeMathInt_NODEFAULT(tp_self, self)                                           DeeType_invoke_math_tp_int_NODEFAULT(tp_self, (tp_self)->tp_math->tp_int, self)
#define DeeType_InvokeMathInv(tp_self, self)                                                     DeeType_invoke_math_tp_inv(tp_self, (tp_self)->tp_math->tp_inv, self)
#define DeeType_InvokeMathInv_NODEFAULT(tp_self, self)                                           DeeType_invoke_math_tp_inv_NODEFAULT(tp_self, (tp_self)->tp_math->tp_inv, self)
#define DeeType_InvokeMathPos(tp_self, self)                                                     DeeType_invoke_math_tp_pos(tp_self, (tp_self)->tp_math->tp_pos, self)
#define DeeType_InvokeMathPos_NODEFAULT(tp_self, self)                                           DeeType_invoke_math_tp_pos_NODEFAULT(tp_self, (tp_self)->tp_math->tp_pos, self)
#define DeeType_InvokeMathNeg(tp_self, self)                                                     DeeType_invoke_math_tp_neg(tp_self, (tp_self)->tp_math->tp_neg, self)
#define DeeType_InvokeMathNeg_NODEFAULT(tp_self, self)                                           DeeType_invoke_math_tp_neg_NODEFAULT(tp_self, (tp_self)->tp_math->tp_neg, self)
#define DeeType_InvokeMathAdd(tp_self, self, other)                                              DeeType_invoke_math_tp_add(tp_self, (tp_self)->tp_math->tp_add, self, other)
#define DeeType_InvokeMathAdd_NODEFAULT(tp_self, self, other)                                    DeeType_invoke_math_tp_add_NODEFAULT(tp_self, (tp_self)->tp_math->tp_add, self, other)
#define DeeType_InvokeMathSub(tp_self, self, other)                                              DeeType_invoke_math_tp_sub(tp_self, (tp_self)->tp_math->tp_sub, self, other)
#define DeeType_InvokeMathSub_NODEFAULT(tp_self, self, other)                                    DeeType_invoke_math_tp_sub_NODEFAULT(tp_self, (tp_self)->tp_math->tp_sub, self, other)
#define DeeType_InvokeMathMul(tp_self, self, other)                                              DeeType_invoke_math_tp_mul(tp_self, (tp_self)->tp_math->tp_mul, self, other)
#define DeeType_InvokeMathMul_NODEFAULT(tp_self, self, other)                                    DeeType_invoke_math_tp_mul_NODEFAULT(tp_self, (tp_self)->tp_math->tp_mul, self, other)
#define DeeType_InvokeMathDiv(tp_self, self, other)                                              DeeType_invoke_math_tp_div(tp_self, (tp_self)->tp_math->tp_div, self, other)
#define DeeType_InvokeMathDiv_NODEFAULT(tp_self, self, other)                                    DeeType_invoke_math_tp_div_NODEFAULT(tp_self, (tp_self)->tp_math->tp_div, self, other)
#define DeeType_InvokeMathMod(tp_self, self, other)                                              DeeType_invoke_math_tp_mod(tp_self, (tp_self)->tp_math->tp_mod, self, other)
#define DeeType_InvokeMathMod_NODEFAULT(tp_self, self, other)                                    DeeType_invoke_math_tp_mod_NODEFAULT(tp_self, (tp_self)->tp_math->tp_mod, self, other)
#define DeeType_InvokeMathShl(tp_self, self, other)                                              DeeType_invoke_math_tp_shl(tp_self, (tp_self)->tp_math->tp_shl, self, other)
#define DeeType_InvokeMathShl_NODEFAULT(tp_self, self, other)                                    DeeType_invoke_math_tp_shl_NODEFAULT(tp_self, (tp_self)->tp_math->tp_shl, self, other)
#define DeeType_InvokeMathShr(tp_self, self, other)                                              DeeType_invoke_math_tp_shr(tp_self, (tp_self)->tp_math->tp_shr, self, other)
#define DeeType_InvokeMathShr_NODEFAULT(tp_self, self, other)                                    DeeType_invoke_math_tp_shr_NODEFAULT(tp_self, (tp_self)->tp_math->tp_shr, self, other)
#define DeeType_InvokeMathAnd(tp_self, self, other)                                              DeeType_invoke_math_tp_and(tp_self, (tp_self)->tp_math->tp_and, self, other)
#define DeeType_InvokeMathAnd_NODEFAULT(tp_self, self, other)                                    DeeType_invoke_math_tp_and_NODEFAULT(tp_self, (tp_self)->tp_math->tp_and, self, other)
#define DeeType_InvokeMathOr(tp_self, self, other)                                               DeeType_invoke_math_tp_or(tp_self, (tp_self)->tp_math->tp_or, self, other)
#define DeeType_InvokeMathOr_NODEFAULT(tp_self, self, other)                                     DeeType_invoke_math_tp_or_NODEFAULT(tp_self, (tp_self)->tp_math->tp_or, self, other)
#define DeeType_InvokeMathXor(tp_self, self, other)                                              DeeType_invoke_math_tp_xor(tp_self, (tp_self)->tp_math->tp_xor, self, other)
#define DeeType_InvokeMathXor_NODEFAULT(tp_self, self, other)                                    DeeType_invoke_math_tp_xor_NODEFAULT(tp_self, (tp_self)->tp_math->tp_xor, self, other)
#define DeeType_InvokeMathPow(tp_self, self, other)                                              DeeType_invoke_math_tp_pow(tp_self, (tp_self)->tp_math->tp_pow, self, other)
#define DeeType_InvokeMathPow_NODEFAULT(tp_self, self, other)                                    DeeType_invoke_math_tp_pow_NODEFAULT(tp_self, (tp_self)->tp_math->tp_pow, self, other)
#define DeeType_InvokeMathInc(tp_self, p_self)                                                   DeeType_invoke_math_tp_inc(tp_self, (tp_self)->tp_math->tp_inc, p_self)
#define DeeType_InvokeMathInc_NODEFAULT(tp_self, p_self)                                         DeeType_invoke_math_tp_inc_NODEFAULT(tp_self, (tp_self)->tp_math->tp_inc, p_self)
#define DeeType_InvokeMathDec(tp_self, p_self)                                                   DeeType_invoke_math_tp_dec(tp_self, (tp_self)->tp_math->tp_dec, p_self)
#define DeeType_InvokeMathDec_NODEFAULT(tp_self, p_self)                                         DeeType_invoke_math_tp_dec_NODEFAULT(tp_self, (tp_self)->tp_math->tp_dec, p_self)
#define DeeType_InvokeMathInplaceAdd(tp_self, p_self, other)                                     DeeType_invoke_math_tp_inplace_add(tp_self, (tp_self)->tp_math->tp_inplace_add, p_self, other)
#define DeeType_InvokeMathInplaceAdd_NODEFAULT(tp_self, p_self, other)                           DeeType_invoke_math_tp_inplace_add_NODEFAULT(tp_self, (tp_self)->tp_math->tp_inplace_add, p_self, other)
#define DeeType_InvokeMathInplaceSub(tp_self, p_self, other)                                     DeeType_invoke_math_tp_inplace_sub(tp_self, (tp_self)->tp_math->tp_inplace_sub, p_self, other)
#define DeeType_InvokeMathInplaceSub_NODEFAULT(tp_self, p_self, other)                           DeeType_invoke_math_tp_inplace_sub_NODEFAULT(tp_self, (tp_self)->tp_math->tp_inplace_sub, p_self, other)
#define DeeType_InvokeMathInplaceMul(tp_self, p_self, other)                                     DeeType_invoke_math_tp_inplace_mul(tp_self, (tp_self)->tp_math->tp_inplace_mul, p_self, other)
#define DeeType_InvokeMathInplaceMul_NODEFAULT(tp_self, p_self, other)                           DeeType_invoke_math_tp_inplace_mul_NODEFAULT(tp_self, (tp_self)->tp_math->tp_inplace_mul, p_self, other)
#define DeeType_InvokeMathInplaceDiv(tp_self, p_self, other)                                     DeeType_invoke_math_tp_inplace_div(tp_self, (tp_self)->tp_math->tp_inplace_div, p_self, other)
#define DeeType_InvokeMathInplaceDiv_NODEFAULT(tp_self, p_self, other)                           DeeType_invoke_math_tp_inplace_div_NODEFAULT(tp_self, (tp_self)->tp_math->tp_inplace_div, p_self, other)
#define DeeType_InvokeMathInplaceMod(tp_self, p_self, other)                                     DeeType_invoke_math_tp_inplace_mod(tp_self, (tp_self)->tp_math->tp_inplace_mod, p_self, other)
#define DeeType_InvokeMathInplaceMod_NODEFAULT(tp_self, p_self, other)                           DeeType_invoke_math_tp_inplace_mod_NODEFAULT(tp_self, (tp_self)->tp_math->tp_inplace_mod, p_self, other)
#define DeeType_InvokeMathInplaceShl(tp_self, p_self, other)                                     DeeType_invoke_math_tp_inplace_shl(tp_self, (tp_self)->tp_math->tp_inplace_shl, p_self, other)
#define DeeType_InvokeMathInplaceShl_NODEFAULT(tp_self, p_self, other)                           DeeType_invoke_math_tp_inplace_shl_NODEFAULT(tp_self, (tp_self)->tp_math->tp_inplace_shl, p_self, other)
#define DeeType_InvokeMathInplaceShr(tp_self, p_self, other)                                     DeeType_invoke_math_tp_inplace_shr(tp_self, (tp_self)->tp_math->tp_inplace_shr, p_self, other)
#define DeeType_InvokeMathInplaceShr_NODEFAULT(tp_self, p_self, other)                           DeeType_invoke_math_tp_inplace_shr_NODEFAULT(tp_self, (tp_self)->tp_math->tp_inplace_shr, p_self, other)
#define DeeType_InvokeMathInplaceAnd(tp_self, p_self, other)                                     DeeType_invoke_math_tp_inplace_and(tp_self, (tp_self)->tp_math->tp_inplace_and, p_self, other)
#define DeeType_InvokeMathInplaceAnd_NODEFAULT(tp_self, p_self, other)                           DeeType_invoke_math_tp_inplace_and_NODEFAULT(tp_self, (tp_self)->tp_math->tp_inplace_and, p_self, other)
#define DeeType_InvokeMathInplaceOr(tp_self, p_self, other)                                      DeeType_invoke_math_tp_inplace_or(tp_self, (tp_self)->tp_math->tp_inplace_or, p_self, other)
#define DeeType_InvokeMathInplaceOr_NODEFAULT(tp_self, p_self, other)                            DeeType_invoke_math_tp_inplace_or_NODEFAULT(tp_self, (tp_self)->tp_math->tp_inplace_or, p_self, other)
#define DeeType_InvokeMathInplaceXor(tp_self, p_self, other)                                     DeeType_invoke_math_tp_inplace_xor(tp_self, (tp_self)->tp_math->tp_inplace_xor, p_self, other)
#define DeeType_InvokeMathInplaceXor_NODEFAULT(tp_self, p_self, other)                           DeeType_invoke_math_tp_inplace_xor_NODEFAULT(tp_self, (tp_self)->tp_math->tp_inplace_xor, p_self, other)
#define DeeType_InvokeMathInplacePow(tp_self, p_self, other)                                     DeeType_invoke_math_tp_inplace_pow(tp_self, (tp_self)->tp_math->tp_inplace_pow, p_self, other)
#define DeeType_InvokeMathInplacePow_NODEFAULT(tp_self, p_self, other)                           DeeType_invoke_math_tp_inplace_pow_NODEFAULT(tp_self, (tp_self)->tp_math->tp_inplace_pow, p_self, other)
#define DeeType_InvokeCmpHash(tp_self, self)                                                     DeeType_invoke_cmp_tp_hash(tp_self, (tp_self)->tp_cmp->tp_hash, self)
#define DeeType_InvokeCmpHash_NODEFAULT(tp_self, self)                                           DeeType_invoke_cmp_tp_hash_NODEFAULT(tp_self, (tp_self)->tp_cmp->tp_hash, self)
#define DeeType_InvokeCmpEq(tp_self, self, other)                                                DeeType_invoke_cmp_tp_eq(tp_self, (tp_self)->tp_cmp->tp_eq, self, other)
#define DeeType_InvokeCmpEq_NODEFAULT(tp_self, self, other)                                      DeeType_invoke_cmp_tp_eq_NODEFAULT(tp_self, (tp_self)->tp_cmp->tp_eq, self, other)
#define DeeType_InvokeCmpNe(tp_self, self, other)                                                DeeType_invoke_cmp_tp_ne(tp_self, (tp_self)->tp_cmp->tp_ne, self, other)
#define DeeType_InvokeCmpNe_NODEFAULT(tp_self, self, other)                                      DeeType_invoke_cmp_tp_ne_NODEFAULT(tp_self, (tp_self)->tp_cmp->tp_ne, self, other)
#define DeeType_InvokeCmpLo(tp_self, self, other)                                                DeeType_invoke_cmp_tp_lo(tp_self, (tp_self)->tp_cmp->tp_lo, self, other)
#define DeeType_InvokeCmpLo_NODEFAULT(tp_self, self, other)                                      DeeType_invoke_cmp_tp_lo_NODEFAULT(tp_self, (tp_self)->tp_cmp->tp_lo, self, other)
#define DeeType_InvokeCmpLe(tp_self, self, other)                                                DeeType_invoke_cmp_tp_le(tp_self, (tp_self)->tp_cmp->tp_le, self, other)
#define DeeType_InvokeCmpLe_NODEFAULT(tp_self, self, other)                                      DeeType_invoke_cmp_tp_le_NODEFAULT(tp_self, (tp_self)->tp_cmp->tp_le, self, other)
#define DeeType_InvokeCmpGr(tp_self, self, other)                                                DeeType_invoke_cmp_tp_gr(tp_self, (tp_self)->tp_cmp->tp_gr, self, other)
#define DeeType_InvokeCmpGr_NODEFAULT(tp_self, self, other)                                      DeeType_invoke_cmp_tp_gr_NODEFAULT(tp_self, (tp_self)->tp_cmp->tp_gr, self, other)
#define DeeType_InvokeCmpGe(tp_self, self, other)                                                DeeType_invoke_cmp_tp_ge(tp_self, (tp_self)->tp_cmp->tp_ge, self, other)
#define DeeType_InvokeCmpGe_NODEFAULT(tp_self, self, other)                                      DeeType_invoke_cmp_tp_ge_NODEFAULT(tp_self, (tp_self)->tp_cmp->tp_ge, self, other)
#define DeeType_InvokeCmpCompareEq(tp_self, self, other)                                         DeeType_invoke_cmp_tp_compare_eq(tp_self, (tp_self)->tp_cmp->tp_compare_eq, self, other)
#define DeeType_InvokeCmpCompareEq_NODEFAULT(tp_self, self, other)                               DeeType_invoke_cmp_tp_compare_eq_NODEFAULT(tp_self, (tp_self)->tp_cmp->tp_compare_eq, self, other)
#define DeeType_InvokeCmpCompare(tp_self, self, other)                                           DeeType_invoke_cmp_tp_compare(tp_self, (tp_self)->tp_cmp->tp_compare, self, other)
#define DeeType_InvokeCmpCompare_NODEFAULT(tp_self, self, other)                                 DeeType_invoke_cmp_tp_compare_NODEFAULT(tp_self, (tp_self)->tp_cmp->tp_compare, self, other)
#define DeeType_InvokeCmpTryCompareEq(tp_self, self, other)                                      DeeType_invoke_cmp_tp_trycompare_eq(tp_self, (tp_self)->tp_cmp->tp_trycompare_eq, self, other)
#define DeeType_InvokeCmpTryCompareEq_NODEFAULT(tp_self, self, other)                            DeeType_invoke_cmp_tp_trycompare_eq_NODEFAULT(tp_self, (tp_self)->tp_cmp->tp_trycompare_eq, self, other)
#define DeeType_InvokeSeqIter(tp_self, self)                                                     DeeType_invoke_seq_tp_iter(tp_self, (tp_self)->tp_seq->tp_iter, self)
#define DeeType_InvokeSeqIter_NODEFAULT(tp_self, self)                                           DeeType_invoke_seq_tp_iter_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_iter, self)
#define DeeType_InvokeSeqSizeOb(tp_self, self)                                                   DeeType_invoke_seq_tp_sizeob(tp_self, (tp_self)->tp_seq->tp_sizeob, self)
#define DeeType_InvokeSeqSizeOb_NODEFAULT(tp_self, self)                                         DeeType_invoke_seq_tp_sizeob_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_sizeob, self)
#define DeeType_InvokeSeqContains(tp_self, self, other)                                          DeeType_invoke_seq_tp_contains(tp_self, (tp_self)->tp_seq->tp_contains, self, other)
#define DeeType_InvokeSeqContains_NODEFAULT(tp_self, self, other)                                DeeType_invoke_seq_tp_contains_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_contains, self, other)
#define DeeType_InvokeSeqGetItem(tp_self, self, index)                                           DeeType_invoke_seq_tp_getitem(tp_self, (tp_self)->tp_seq->tp_getitem, self, index)
#define DeeType_InvokeSeqGetItem_NODEFAULT(tp_self, self, index)                                 DeeType_invoke_seq_tp_getitem_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_getitem, self, index)
#define DeeType_InvokeSeqDelItem(tp_self, self, index)                                           DeeType_invoke_seq_tp_delitem(tp_self, (tp_self)->tp_seq->tp_delitem, self, index)
#define DeeType_InvokeSeqDelItem_NODEFAULT(tp_self, self, index)                                 DeeType_invoke_seq_tp_delitem_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_delitem, self, index)
#define DeeType_InvokeSeqSetItem(tp_self, self, index, value)                                    DeeType_invoke_seq_tp_setitem(tp_self, (tp_self)->tp_seq->tp_setitem, self, index, value)
#define DeeType_InvokeSeqSetItem_NODEFAULT(tp_self, self, index, value)                          DeeType_invoke_seq_tp_setitem_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_setitem, self, index, value)
#define DeeType_InvokeSeqGetRange(tp_self, self, start, end)                                     DeeType_invoke_seq_tp_getrange(tp_self, (tp_self)->tp_seq->tp_getrange, self, start, end)
#define DeeType_InvokeSeqGetRange_NODEFAULT(tp_self, self, start, end)                           DeeType_invoke_seq_tp_getrange_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_getrange, self, start, end)
#define DeeType_InvokeSeqDelRange(tp_self, self, start, end)                                     DeeType_invoke_seq_tp_delrange(tp_self, (tp_self)->tp_seq->tp_delrange, self, start, end)
#define DeeType_InvokeSeqDelRange_NODEFAULT(tp_self, self, start, end)                           DeeType_invoke_seq_tp_delrange_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_delrange, self, start, end)
#define DeeType_InvokeSeqSetRange(tp_self, self, start, end, values)                             DeeType_invoke_seq_tp_setrange(tp_self, (tp_self)->tp_seq->tp_setrange, self, start, end, values)
#define DeeType_InvokeSeqSetRange_NODEFAULT(tp_self, self, start, end, values)                   DeeType_invoke_seq_tp_setrange_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_setrange, self, start, end, values)
#define DeeType_InvokeSeqForeach(tp_self, self, proc, arg)                                       DeeType_invoke_seq_tp_foreach(tp_self, (tp_self)->tp_seq->tp_foreach, self, proc, arg)
#define DeeType_InvokeSeqForeach_NODEFAULT(tp_self, self, proc, arg)                             DeeType_invoke_seq_tp_foreach_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_foreach, self, proc, arg)
#define DeeType_InvokeSeqForeachPair(tp_self, self, proc, arg)                                   DeeType_invoke_seq_tp_foreach_pair(tp_self, (tp_self)->tp_seq->tp_foreach_pair, self, proc, arg)
#define DeeType_InvokeSeqForeachPair_NODEFAULT(tp_self, self, proc, arg)                         DeeType_invoke_seq_tp_foreach_pair_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_foreach_pair, self, proc, arg)
#define DeeType_InvokeSeqEnumerate(tp_self, self, proc, arg)                                     DeeType_invoke_seq_tp_enumerate(tp_self, (tp_self)->tp_seq->tp_enumerate, self, proc, arg)
#define DeeType_InvokeSeqEnumerate_NODEFAULT(tp_self, self, proc, arg)                           DeeType_invoke_seq_tp_enumerate_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_enumerate, self, proc, arg)
#define DeeType_InvokeSeqEnumerateIndex(tp_self, self, proc, arg, start, end)                    DeeType_invoke_seq_tp_enumerate_index(tp_self, (tp_self)->tp_seq->tp_enumerate_index, self, proc, arg, start, end)
#define DeeType_InvokeSeqEnumerateIndex_NODEFAULT(tp_self, self, proc, arg, start, end)          DeeType_invoke_seq_tp_enumerate_index_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_enumerate_index, self, proc, arg, start, end)
#define DeeType_InvokeSeqIterKeys(tp_self, self)                                                 DeeType_invoke_seq_tp_iterkeys(tp_self, (tp_self)->tp_seq->tp_iterkeys, self)
#define DeeType_InvokeSeqIterKeys_NODEFAULT(tp_self, self)                                       DeeType_invoke_seq_tp_iterkeys_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_iterkeys, self)
#define DeeType_InvokeSeqBoundItem(tp_self, self, index)                                         DeeType_invoke_seq_tp_bounditem(tp_self, (tp_self)->tp_seq->tp_bounditem, self, index)
#define DeeType_InvokeSeqBoundItem_NODEFAULT(tp_self, self, index)                               DeeType_invoke_seq_tp_bounditem_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_bounditem, self, index)
#define DeeType_InvokeSeqHasItem(tp_self, self, index)                                           DeeType_invoke_seq_tp_hasitem(tp_self, (tp_self)->tp_seq->tp_hasitem, self, index)
#define DeeType_InvokeSeqHasItem_NODEFAULT(tp_self, self, index)                                 DeeType_invoke_seq_tp_hasitem_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_hasitem, self, index)
#define DeeType_InvokeSeqSize(tp_self, self)                                                     DeeType_invoke_seq_tp_size(tp_self, (tp_self)->tp_seq->tp_size, self)
#define DeeType_InvokeSeqSize_NODEFAULT(tp_self, self)                                           DeeType_invoke_seq_tp_size_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_size, self)
#define DeeType_InvokeSeqSizeFast(tp_self, self)                                                 DeeType_invoke_seq_tp_size_fast(tp_self, (tp_self)->tp_seq->tp_size, self)
#define DeeType_InvokeSeqSizeFast_NODEFAULT(tp_self, self)                                       DeeType_invoke_seq_tp_size_fast_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_size, self)
#define DeeType_InvokeSeqGetItemIndex(tp_self, self, index)                                      DeeType_invoke_seq_tp_getitem_index(tp_self, (tp_self)->tp_seq->tp_getitem_index, self, index)
#define DeeType_InvokeSeqGetItemIndex_NODEFAULT(tp_self, self, index)                            DeeType_invoke_seq_tp_getitem_index_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_getitem_index, self, index)
#define DeeType_InvokeSeqDelItemIndex(tp_self, self, index)                                      DeeType_invoke_seq_tp_delitem_index(tp_self, (tp_self)->tp_seq->tp_delitem_index, self, index)
#define DeeType_InvokeSeqDelItemIndex_NODEFAULT(tp_self, self, index)                            DeeType_invoke_seq_tp_delitem_index_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_delitem_index, self, index)
#define DeeType_InvokeSeqSetItemIndex(tp_self, self, index, value)                               DeeType_invoke_seq_tp_setitem_index(tp_self, (tp_self)->tp_seq->tp_setitem_index, self, index, value)
#define DeeType_InvokeSeqSetItemIndex_NODEFAULT(tp_self, self, index, value)                     DeeType_invoke_seq_tp_setitem_index_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_setitem_index, self, index, value)
#define DeeType_InvokeSeqBoundItemIndex(tp_self, self, index)                                    DeeType_invoke_seq_tp_bounditem_index(tp_self, (tp_self)->tp_seq->tp_bounditem_index, self, index)
#define DeeType_InvokeSeqBoundItemIndex_NODEFAULT(tp_self, self, index)                          DeeType_invoke_seq_tp_bounditem_index_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_bounditem_index, self, index)
#define DeeType_InvokeSeqHasItemIndex(tp_self, self, index)                                      DeeType_invoke_seq_tp_hasitem_index(tp_self, (tp_self)->tp_seq->tp_hasitem_index, self, index)
#define DeeType_InvokeSeqHasItemIndex_NODEFAULT(tp_self, self, index)                            DeeType_invoke_seq_tp_hasitem_index_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_hasitem_index, self, index)
#define DeeType_InvokeSeqGetRangeIndex(tp_self, self, start, end)                                DeeType_invoke_seq_tp_getrange_index(tp_self, (tp_self)->tp_seq->tp_getrange_index, self, start, end)
#define DeeType_InvokeSeqGetRangeIndex_NODEFAULT(tp_self, self, start, end)                      DeeType_invoke_seq_tp_getrange_index_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_getrange_index, self, start, end)
#define DeeType_InvokeSeqDelRangeIndex(tp_self, self, start, end)                                DeeType_invoke_seq_tp_delrange_index(tp_self, (tp_self)->tp_seq->tp_delrange_index, self, start, end)
#define DeeType_InvokeSeqDelRangeIndex_NODEFAULT(tp_self, self, start, end)                      DeeType_invoke_seq_tp_delrange_index_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_delrange_index, self, start, end)
#define DeeType_InvokeSeqSetRangeIndex(tp_self, self, start, end, value)                         DeeType_invoke_seq_tp_setrange_index(tp_self, (tp_self)->tp_seq->tp_setrange_index, self, start, end, value)
#define DeeType_InvokeSeqSetRangeIndex_NODEFAULT(tp_self, self, start, end, value)               DeeType_invoke_seq_tp_setrange_index_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_setrange_index, self, start, end, value)
#define DeeType_InvokeSeqGetRangeIndexN(tp_self, self, start)                                    DeeType_invoke_seq_tp_getrange_index_n(tp_self, (tp_self)->tp_seq->tp_getrange_index_n, self, start)
#define DeeType_InvokeSeqGetRangeIndexN_NODEFAULT(tp_self, self, start)                          DeeType_invoke_seq_tp_getrange_index_n_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_getrange_index_n, self, start)
#define DeeType_InvokeSeqDelRangeIndexN(tp_self, self, start)                                    DeeType_invoke_seq_tp_delrange_index_n(tp_self, (tp_self)->tp_seq->tp_delrange_index_n, self, start)
#define DeeType_InvokeSeqDelRangeIndexN_NODEFAULT(tp_self, self, start)                          DeeType_invoke_seq_tp_delrange_index_n_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_delrange_index_n, self, start)
#define DeeType_InvokeSeqSetRangeIndexN(tp_self, self, start, value)                             DeeType_invoke_seq_tp_setrange_index_n(tp_self, (tp_self)->tp_seq->tp_setrange_index_n, self, start, value)
#define DeeType_InvokeSeqSetRangeIndexN_NODEFAULT(tp_self, self, start, value)                   DeeType_invoke_seq_tp_setrange_index_n_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_setrange_index_n, self, start, value)
#define DeeType_InvokeSeqTryGetItem(tp_self, self, index)                                        DeeType_invoke_seq_tp_trygetitem(tp_self, (tp_self)->tp_seq->tp_trygetitem, self, index)
#define DeeType_InvokeSeqTryGetItem_NODEFAULT(tp_self, self, index)                              DeeType_invoke_seq_tp_trygetitem_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_trygetitem, self, index)
#define DeeType_InvokeSeqTryGetItemIndex(tp_self, self, index)                                   DeeType_invoke_seq_tp_trygetitem_index(tp_self, (tp_self)->tp_seq->tp_trygetitem_index, self, index)
#define DeeType_InvokeSeqTryGetItemIndex_NODEFAULT(tp_self, self, index)                         DeeType_invoke_seq_tp_trygetitem_index_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_trygetitem_index, self, index)
#define DeeType_InvokeSeqTryGetItemStringHash(tp_self, self, key, hash)                          DeeType_invoke_seq_tp_trygetitem_string_hash(tp_self, (tp_self)->tp_seq->tp_trygetitem_string_hash, self, key, hash)
#define DeeType_InvokeSeqTryGetItemStringHash_NODEFAULT(tp_self, self, key, hash)                DeeType_invoke_seq_tp_trygetitem_string_hash_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_trygetitem_string_hash, self, key, hash)
#define DeeType_InvokeSeqGetItemStringHash(tp_self, self, key, hash)                             DeeType_invoke_seq_tp_getitem_string_hash(tp_self, (tp_self)->tp_seq->tp_getitem_string_hash, self, key, hash)
#define DeeType_InvokeSeqGetItemStringHash_NODEFAULT(tp_self, self, key, hash)                   DeeType_invoke_seq_tp_getitem_string_hash_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_getitem_string_hash, self, key, hash)
#define DeeType_InvokeSeqDelItemStringHash(tp_self, self, key, hash)                             DeeType_invoke_seq_tp_delitem_string_hash(tp_self, (tp_self)->tp_seq->tp_delitem_string_hash, self, key, hash)
#define DeeType_InvokeSeqDelItemStringHash_NODEFAULT(tp_self, self, key, hash)                   DeeType_invoke_seq_tp_delitem_string_hash_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_delitem_string_hash, self, key, hash)
#define DeeType_InvokeSeqSetItemStringHash(tp_self, self, key, hash, value)                      DeeType_invoke_seq_tp_setitem_string_hash(tp_self, (tp_self)->tp_seq->tp_setitem_string_hash, self, key, hash, value)
#define DeeType_InvokeSeqSetItemStringHash_NODEFAULT(tp_self, self, key, hash, value)            DeeType_invoke_seq_tp_setitem_string_hash_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_setitem_string_hash, self, key, hash, value)
#define DeeType_InvokeSeqBoundItemStringHash(tp_self, self, key, hash)                           DeeType_invoke_seq_tp_bounditem_string_hash(tp_self, (tp_self)->tp_seq->tp_bounditem_string_hash, self, key, hash)
#define DeeType_InvokeSeqBoundItemStringHash_NODEFAULT(tp_self, self, key, hash)                 DeeType_invoke_seq_tp_bounditem_string_hash_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_bounditem_string_hash, self, key, hash)
#define DeeType_InvokeSeqHasItemStringHash(tp_self, self, key, hash)                             DeeType_invoke_seq_tp_hasitem_string_hash(tp_self, (tp_self)->tp_seq->tp_hasitem_string_hash, self, key, hash)
#define DeeType_InvokeSeqHasItemStringHash_NODEFAULT(tp_self, self, key, hash)                   DeeType_invoke_seq_tp_hasitem_string_hash_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_hasitem_string_hash, self, key, hash)
#define DeeType_InvokeSeqTryGetItemStringLenHash(tp_self, self, key, keylen, hash)               DeeType_invoke_seq_tp_trygetitem_string_len_hash(tp_self, (tp_self)->tp_seq->tp_trygetitem_string_len_hash, self, key, keylen, hash)
#define DeeType_InvokeSeqTryGetItemStringLenHash_NODEFAULT(tp_self, self, key, keylen, hash)     DeeType_invoke_seq_tp_trygetitem_string_len_hash_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_trygetitem_string_len_hash, self, key, keylen, hash)
#define DeeType_InvokeSeqGetItemStringLenHash(tp_self, self, key, keylen, hash)                  DeeType_invoke_seq_tp_getitem_string_len_hash(tp_self, (tp_self)->tp_seq->tp_getitem_string_len_hash, self, key, keylen, hash)
#define DeeType_InvokeSeqGetItemStringLenHash_NODEFAULT(tp_self, self, key, keylen, hash)        DeeType_invoke_seq_tp_getitem_string_len_hash_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_getitem_string_len_hash, self, key, keylen, hash)
#define DeeType_InvokeSeqDelItemStringLenHash(tp_self, self, key, keylen, hash)                  DeeType_invoke_seq_tp_delitem_string_len_hash(tp_self, (tp_self)->tp_seq->tp_delitem_string_len_hash, self, key, keylen, hash)
#define DeeType_InvokeSeqDelItemStringLenHash_NODEFAULT(tp_self, self, key, keylen, hash)        DeeType_invoke_seq_tp_delitem_string_len_hash_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_delitem_string_len_hash, self, key, keylen, hash)
#define DeeType_InvokeSeqSetItemStringLenHash(tp_self, self, key, keylen, hash, value)           DeeType_invoke_seq_tp_setitem_string_len_hash(tp_self, (tp_self)->tp_seq->tp_setitem_string_len_hash, self, key, keylen, hash, value)
#define DeeType_InvokeSeqSetItemStringLenHash_NODEFAULT(tp_self, self, key, keylen, hash, value) DeeType_invoke_seq_tp_setitem_string_len_hash_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_setitem_string_len_hash, self, key, keylen, hash, value)
#define DeeType_InvokeSeqBoundItemStringLenHash(tp_self, self, key, keylen, hash)                DeeType_invoke_seq_tp_bounditem_string_len_hash(tp_self, (tp_self)->tp_seq->tp_bounditem_string_len_hash, self, key, keylen, hash)
#define DeeType_InvokeSeqBoundItemStringLenHash_NODEFAULT(tp_self, self, key, keylen, hash)      DeeType_invoke_seq_tp_bounditem_string_len_hash_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_bounditem_string_len_hash, self, key, keylen, hash)
#define DeeType_InvokeSeqHasItemStringLenHash(tp_self, self, key, keylen, hash)                  DeeType_invoke_seq_tp_hasitem_string_len_hash(tp_self, (tp_self)->tp_seq->tp_hasitem_string_len_hash, self, key, keylen, hash)
#define DeeType_InvokeSeqHasItemStringLenHash_NODEFAULT(tp_self, self, key, keylen, hash)        DeeType_invoke_seq_tp_hasitem_string_len_hash_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_hasitem_string_len_hash, self, key, keylen, hash)
#define DeeType_InvokeSeqUnpack(tp_self, self, dst_length, dst)                                  DeeType_invoke_seq_tp_unpack(tp_self, (tp_self)->tp_seq->tp_unpack, self, dst_length, dst)
#define DeeType_InvokeSeqUnpack_NODEFAULT(tp_self, self, dst_length, dst)                        DeeType_invoke_seq_tp_unpack_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_unpack, self, dst_length, dst)
#define DeeType_InvokeSeqUnpackEx(tp_self, self, dst_length_min, dst_length_max, dst)            DeeType_invoke_seq_tp_unpack_ex(tp_self, (tp_self)->tp_seq->tp_unpack_ex, self, dst_length_min, dst_length_max, dst)
#define DeeType_InvokeSeqUnpackEx_NODEFAULT(tp_self, self, dst_length_min, dst_length_max, dst)  DeeType_invoke_seq_tp_unpack_ex_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_unpack_ex, self, dst_length_min, dst_length_max, dst)
#define DeeType_InvokeSeqUnpackUb(tp_self, self, dst_length, dst)                                DeeType_invoke_seq_tp_unpack_ub(tp_self, (tp_self)->tp_seq->tp_unpack_ub, self, dst_length, dst)
#define DeeType_InvokeSeqUnpackUb_NODEFAULT(tp_self, self, dst_length, dst)                      DeeType_invoke_seq_tp_unpack_ub_NODEFAULT(tp_self, (tp_self)->tp_seq->tp_unpack_ub, self, dst_length, dst)
#define DeeType_InvokeAttrGetAttr(tp_self, self, name)                                           DeeType_invoke_attr_tp_getattr(tp_self, (tp_self)->tp_attr->tp_getattr, self, name)
#define DeeType_InvokeAttrGetAttr_NODEFAULT(tp_self, self, name)                                 DeeType_invoke_attr_tp_getattr_NODEFAULT(tp_self, (tp_self)->tp_attr->tp_getattr, self, name)
#define DeeType_InvokeAttrDelAttr(tp_self, self, name)                                           DeeType_invoke_attr_tp_delattr(tp_self, (tp_self)->tp_attr->tp_delattr, self, name)
#define DeeType_InvokeAttrDelAttr_NODEFAULT(tp_self, self, name)                                 DeeType_invoke_attr_tp_delattr_NODEFAULT(tp_self, (tp_self)->tp_attr->tp_delattr, self, name)
#define DeeType_InvokeAttrSetAttr(tp_self, self, name, value)                                    DeeType_invoke_attr_tp_setattr(tp_self, (tp_self)->tp_attr->tp_setattr, self, name, value)
#define DeeType_InvokeAttrSetAttr_NODEFAULT(tp_self, self, name, value)                          DeeType_invoke_attr_tp_setattr_NODEFAULT(tp_self, (tp_self)->tp_attr->tp_setattr, self, name, value)
#define DeeType_InvokeWithEnter(tp_self, self)                                                   DeeType_invoke_with_tp_enter(tp_self, (tp_self)->tp_with->tp_enter, self)
#define DeeType_InvokeWithEnter_NODEFAULT(tp_self, self)                                         DeeType_invoke_with_tp_enter_NODEFAULT(tp_self, (tp_self)->tp_with->tp_enter, self)
#define DeeType_InvokeWithLeave(tp_self, self)                                                   DeeType_invoke_with_tp_leave(tp_self, (tp_self)->tp_with->tp_leave, self)
#define DeeType_InvokeWithLeave_NODEFAULT(tp_self, self)                                         DeeType_invoke_with_tp_leave_NODEFAULT(tp_self, (tp_self)->tp_with->tp_leave, self)
#endif /* !CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */
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



#ifdef CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS
#ifdef CONFIG_BUILDING_DEEMON
/* Backwards-compatibility */
#include "../../src/deemon/runtime/method-hint-defaults.h"
#define DeeSeq_DefaultForeachWithSizeAndGetItemIndexFast           default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast
#define DeeSeq_DefaultForeachWithSizeAndTryGetItemIndex            default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index
#define DeeSeq_DefaultForeachWithSizeAndGetItemIndex               default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index
#define DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index
#define DeeSeq_DefaultForeachWithSizeObAndGetItem                  default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem
#endif /* CONFIG_BUILDING_DEEMON */
#endif /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */


#endif /* !GUARD_DEEMON_CLASS_H */
