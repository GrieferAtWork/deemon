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
/*!export **/
/*!export DeeClassDesc_**/
/*!export DeeClassDescriptor_**/
/*!export DeeClass_**/
/*!export DeeInstanceMember_**/
/*!export DeeInstance_**/
/*!export Dee_CLASS_ATTRIBUTE_**/
/*!export Dee_CLASS_**/
/*!export Dee_TP_FCLASS_**/
/*!export Dee_class_desc_**/
/*!export Dee_instance_desc_**/
/*!export instance_**/
#ifndef GUARD_DEEMON_CLASS_H
#define GUARD_DEEMON_CLASS_H 1 /*!export-*/

#ifndef _DEE_WITHOUT_INCLUDES
#include "api.h"

#include <hybrid/typecore.h> /* __SIZEOF_POINTER__ */

#include "object.h"
#include "serial.h"    /* Dee_seraddr_t */
#include "util/lock.h" /* Dee_atomic_read_with_atomic_rwlock, Dee_atomic_rwlock_* */

#include <stdarg.h>  /* va_list */
#include <stdbool.h> /* bool */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* uint16_t, uintptr_t */

#ifndef __INTELLISENSE__
#include "string.h" /* DeeString_Hash */
#if !defined(CONFIG_CALLTUPLE_OPTIMIZATIONS) && !defined(__OPTIMIZE_SIZE__)
#include "tuple.h" /* DeeTuple_ELEM, DeeTuple_SIZE */
#endif /* !CONFIG_CALLTUPLE_OPTIMIZATIONS && !__OPTIMIZE_SIZE__ */
#endif /* !__INTELLISENSE__ */
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
                                              * When `Dee_CLASS_ATTRIBUTE_FREADONLY' is set, only `Dee_CLASS_GETSET_GET' is ever accessed,
                                              * and all other callbacks behave as though they were unbound. */
/*      Dee_CLASS_ATTRIBUTE_F         0x0040  * ... */
#define Dee_CLASS_ATTRIBUTE_FCLASSMEM 0x0080 /* An instance-attribute is stored in class memory (usually set for instance member functions).
                                              * NOTE: Ignored when used by attributes in `cd_cattr_list', where
                                              *       access is always done like it would be when this was set. */
/*      Dee_CLASS_ATTRIBUTE_F         0x0100  * ... */
/*      Dee_CLASS_ATTRIBUTE_F         0x8000  * ... */
#define Dee_CLASS_ATTRIBUTE_FMASK     0x00b7 /* Mask of known flag bits. */

struct Dee_string_object;
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
	(!((self)->ca_flag & Dee_CLASS_ATTRIBUTE_FPRIVATE) || \
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
#define Dee_CLASS_OPERATOR_SUPERARGS (Dee_OPERATOR_USERCOUNT + 0)
#define Dee_CLASS_OPERATOR_PRINT     (Dee_OPERATOR_USERCOUNT + 1) /* `operator str(fp: File): none' */
#define Dee_CLASS_OPERATOR_PRINTREPR (Dee_OPERATOR_USERCOUNT + 2) /* `operator repr(fp: File): none' */
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

/* Special values for `DeeClassDescriptorObject::cd_flags' */
#define Dee_TP_FCLASS_AUTOINIT  Dee_TP_FGC       /* FLAG: When set, the construction operator is implemented to automatically initialize
                                                  *       class members in compliance to the `this = default;' constructor definition.
                                                  *       Additionally, if not already defined by the caller, this flag also causes
                                                  *       `operator repr' to be implemented (see above).
                                                  * NOTE: This flag should not be used together with `TP_FINHERITCTOR' */
#define Dee_TP_FCLASS_SUPERKWDS Dee_TP_FHEAP     /* FLAG: When set, the superargs operator actually returns a tuple `(args, kwds)' which
                                                  *       should then be used to invoke the super-constructor as `super(args..., **kwds)'
                                                  *       Otherwise, `args' is returned, and the super-constructor is called as `super(args...)' */
#define Dee_TP_FCLASS_NOBUILTIN Dee_TP_FVARIABLE /* FLAG: Don't auto-define builtin operators */

struct Dee_string_object;
typedef struct Dee_class_descriptor_object {
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
	 * >>         { "cfunc",   .ca_addr = 0, .ca_flag = Dee_CLASS_ATTRIBUTE_FREADONLY },
	 * >>         { "cmember", .ca_addr = 1, .ca_flag = Dee_CLASS_ATTRIBUTE_FNORMAL },
	 * >>         { "cprop",   .ca_addr = 2, .ca_flag = Dee_CLASS_ATTRIBUTE_FGETSET },
	 * >>     },
	 * >>     .cd_iattr_mask = 7, // ifunc, imember, iprop  (3 -> 7)
	 * >>     .cd_iattr_list = {
	 * >>         { "ifunc",   .ca_addr = 5, .ca_flag = Dee_CLASS_ATTRIBUTE_FCLASSMEM|Dee_CLASS_ATTRIBUTE_FMETHOD },
	 * >>         { "imember", .ca_addr = 0, .ca_flag = Dee_CLASS_ATTRIBUTE_FNORMAL },
	 * >>         { "iprop",   .ca_addr = 6, .ca_flag = Dee_CLASS_ATTRIBUTE_FCLASSMEM|Dee_CLASS_ATTRIBUTE_FMETHOD|Dee_CLASS_ATTRIBUTE_FGETSET },
	 * >>     },
	 * >> }
	 * INSTANCE (class):
	 * >> {
	 * >>     .cd_members = {
	 * >>         [0]                    = <function cfunc() { print "static function"; }>,
	 * >>         [1]                    = <cmember = "static member">,
	 * >>         [2 + Dee_CLASS_GETSET_GET] = <cprop:get: get()  { return "static member"; }>,
	 * >>         [2 + Dee_CLASS_GETSET_DEL] = <cprop:del: del()  { print "static member"; }>,
	 * >>         [2 + Dee_CLASS_GETSET_SET] = <cprop:set: set(v) { print "static member"; }>,
	 * >>         [5]                    = <function ifunc() { print "instance function"; }>,
	 * >>         [6 + Dee_CLASS_GETSET_GET] = <iprop:get: get()  { return "instance member"; }>,
	 * >>         [6 + Dee_CLASS_GETSET_DEL] = <iprop:del: del()  { print "instance member"; }>,
	 * >>         [6 + Dee_CLASS_GETSET_SET] = <iprop:set: set(v) { print "instance member"; }>
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
	uint16_t                                            cd_flags;       /* [const] Additional flags to set for the resulting type (set of `TP_F*').
	                                                                     * NOTE: The `TP_FINHERITCTOR' flag has special meaning here,
	                                                                     *       in that its presence causes `Dee_CLASS_OPERATOR_SUPERARGS'
	                                                                     *       to be implemented such that it forwards all arguments
	                                                                     *       to the underlying base-type, while also implementing
	                                                                     *       `OPERATOR_CONSTRUCTOR' as a no-op for any number of
	                                                                     *       arguments.
	                                                                     *       If the user overrides `Dee_CLASS_OPERATOR_SUPERARGS',
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
} DeeClassDescriptorObject;
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
	 Dee_REQUIRES_OBJECT(DeeTypeObject, self)->tp_class)


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
	((struct Dee_instance_desc *)((uintptr_t)Dee_AsObject(self) + (class_descriptor)->cd_offset))


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
#if !defined(CONFIG_CALLTUPLE_OPTIMIZATIONS) && !defined(__OPTIMIZE_SIZE__) && !defined(__INTELLISENSE__)
#define DeeInstance_CallAttributeTuple(desc, self, this_arg, attr, args) \
	DeeInstance_CallAttribute(desc, self, this_arg, attr, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeInstance_CallAttributeTupleKw(desc, self, this_arg, attr, args, kw) \
	DeeInstance_CallAttributeKw(desc, self, this_arg, attr, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#endif /* !CONFIG_CALLTUPLE_OPTIMIZATIONS && !__OPTIMIZE_SIZE__ && !__INTELLISENSE__ */

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
#define DeeClass_BoundMember(self, addr) (Dee_atomic_read_with_atomic_rwlock(&DeeClass_DESC(self)->cd_members[addr], &DeeClass_DESC(self)->cd_lock) != NULL)


#ifdef CONFIG_BUILDING_DEEMON
struct Dee_attribute_info;
struct Dee_attribute_lookup_rules;
struct Dee_attriter;

/* Iterate user-defined class or instance attributes. */
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL
DeeClass_IterClassAttributes(DeeTypeObject *__restrict self,
                             struct Dee_attriter *iterbuf, size_t bufsize);
INTDEF WUNUSED NONNULL((1, 3)) size_t DCALL
DeeClass_IterInstanceAttributes(DeeTypeObject *__restrict self, DeeObject *instance,
                                struct Dee_attriter *iterbuf, size_t bufsize);

/* Iterate user-defined instance attributes, as
 * accessed by `DeeClass_GetInstanceAttribute()'. */
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL
DeeClass_IterClassInstanceAttributes(DeeTypeObject *__restrict self,
                                     struct Dee_attriter *iterbuf, size_t bufsize);

struct Dee_attrspec;
struct Dee_attrdesc;

/* Find a specific class-, instance- or
 * instance-through-class-attribute, matching the given lookup rules.
 * @return:  0: Attribute found (*result was filled with data).
 * @return:  1: No attribute matching the given requirements was found.
 * @return: -1: An error occurred. */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
DeeClass_FindClassAttribute(DeeTypeObject *tp_invoker, DeeTypeObject *self,
                            struct Dee_attrspec const *__restrict specs,
                            struct Dee_attrdesc *__restrict result);
INTDEF WUNUSED NONNULL((1, 2, 4, 5)) int DCALL
DeeClass_FindInstanceAttribute(DeeTypeObject *tp_invoker, DeeTypeObject *self, DeeObject *instance,
                               struct Dee_attrspec const *__restrict specs,
                               struct Dee_attrdesc *__restrict result);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
DeeClass_FindClassInstanceAttribute(DeeTypeObject *tp_invoker, DeeTypeObject *self,
                                    struct Dee_attrspec const *__restrict specs,
                                    struct Dee_attrdesc *__restrict result);

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
#define DeeClass_GetClassAttribute(class_type, attr)                    DeeInstance_GetAttribute(DeeClass_DESC(class_type), Dee_class_desc_as_instance(DeeClass_DESC(class_type)), (DeeObject *)(class_type), attr)
#define DeeClass_BoundClassAttribute(class_type, attr)                  DeeInstance_BoundAttribute(DeeClass_DESC(class_type), Dee_class_desc_as_instance(DeeClass_DESC(class_type)), (DeeObject *)(class_type), attr)
#define DeeClass_CallClassAttribute(class_type, attr, argc, argv)       DeeInstance_CallAttribute(DeeClass_DESC(class_type), Dee_class_desc_as_instance(DeeClass_DESC(class_type)), (DeeObject *)(class_type), attr, argc, argv)
#define DeeClass_CallClassAttributeKw(class_type, attr, argc, argv, kw) DeeInstance_CallAttributeKw(DeeClass_DESC(class_type), Dee_class_desc_as_instance(DeeClass_DESC(class_type)), (DeeObject *)(class_type), attr, argc, argv, kw)
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
#define DeeClass_CallClassAttributeTuple(class_type, attr, args)        DeeInstance_CallAttributeTuple(DeeClass_DESC(class_type), Dee_class_desc_as_instance(DeeClass_DESC(class_type)), (DeeObject *)(class_type), attr, args)
#define DeeClass_CallClassAttributeTupleKw(class_type, attr, args, kw)  DeeInstance_CallAttributeTupleKw(DeeClass_DESC(class_type), Dee_class_desc_as_instance(DeeClass_DESC(class_type)), (DeeObject *)(class_type), attr, args, kw)
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
#define DeeClass_VCallClassAttributef(class_type, attr, format, args)   DeeInstance_VCallAttributef(DeeClass_DESC(class_type), Dee_class_desc_as_instance(DeeClass_DESC(class_type)), (DeeObject *)(class_type), attr, format, args)
#define DeeClass_DelClassAttribute(class_type, attr)                    DeeInstance_DelAttribute(DeeClass_DESC(class_type), Dee_class_desc_as_instance(DeeClass_DESC(class_type)), (DeeObject *)(class_type), attr)
#define DeeClass_SetClassAttribute(class_type, attr, value)             DeeInstance_SetAttribute(DeeClass_DESC(class_type), Dee_class_desc_as_instance(DeeClass_DESC(class_type)), (DeeObject *)(class_type), attr, value)
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

/* `OPERATOR_CONSTRUCTOR' + `Dee_CLASS_OPERATOR_SUPERARGS' */
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

/* `Dee_CLASS_OPERATOR_SUPERARGS' */
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
#ifndef CONFIG_EXPERIMENTAL_SERIALIZE_OPERATOR
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_builtin_tdeepload(DeeTypeObject *tp_self, DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_builtin_deepload(DeeObject *__restrict self);
#endif /* !CONFIG_EXPERIMENTAL_SERIALIZE_OPERATOR */
#ifdef CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_builtin_nobase_tcopy(DeeTypeObject *tp_self, DeeObject *__restrict self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_builtin_nobase_copy(DeeObject *__restrict self, DeeObject *__restrict other);
#ifndef CONFIG_EXPERIMENTAL_SERIALIZE_OPERATOR
INTDEF WUNUSED NONNULL((1)) int DCALL instance_builtin_nobase_deepload(DeeObject *__restrict self);
#endif /* !CONFIG_EXPERIMENTAL_SERIALIZE_OPERATOR */
#endif /* CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS */
INTDEF NONNULL((1)) void DCALL instance_builtin_destructor(DeeObject *__restrict self); /* No t-variant, because types are unwound automatically during destruction. */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_builtin_tassign(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_builtin_assign(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_builtin_tmoveassign(DeeTypeObject *tp_self, DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_builtin_moveassign(DeeObject *self, DeeObject *other);
struct Dee_serial;
INTDEF WUNUSED NONNULL((1)) int DCALL instance_builtin_serialize(DeeObject *__restrict self, struct Dee_serial *__restrict writer, Dee_seraddr_t addr);

#ifdef Dee_TP_FCLASS_AUTOINIT
/* No predefined construction operators (with `Dee_TP_FCLASS_AUTOINIT'). */
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
#endif /* Dee_TP_FCLASS_AUTOINIT */


/* Hooks when the user-class overrides the associated operator. */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_tcopy(DeeTypeObject *tp_self, DeeObject *__restrict self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_copy(DeeObject *__restrict self, DeeObject *__restrict other);
#ifndef CONFIG_EXPERIMENTAL_SERIALIZE_OPERATOR
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL instance_tdeepcopy(DeeTypeObject *tp_self, DeeObject *__restrict self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_deepcopy(DeeObject *__restrict self, DeeObject *__restrict other);
#endif /* !CONFIG_EXPERIMENTAL_SERIALIZE_OPERATOR */
INTDEF NONNULL((1)) void DCALL instance_destructor(DeeObject *__restrict self); /* No t-variant, because types are unwound automatically during destruction. */

/* GC support for class objects. */
INTDEF NONNULL((1, 2, 3)) void DCALL instance_tvisit(DeeTypeObject *tp_self, DeeObject *__restrict self, Dee_visit_t proc, void *arg);
INTDEF NONNULL((1, 2)) void DCALL instance_visit(DeeObject *__restrict self, Dee_visit_t proc, void *arg);
INTDEF NONNULL((1, 2)) void DCALL instance_tclear(DeeTypeObject *tp_self, DeeObject *__restrict self);
INTDEF NONNULL((1)) void DCALL instance_clear(DeeObject *__restrict self);
INTDEF NONNULL((1, 2)) void DCALL instance_tpclear(DeeTypeObject *tp_self, DeeObject *__restrict self, unsigned int gc_priority);
INTDEF NONNULL((1)) void DCALL instance_pclear(DeeObject *__restrict self, unsigned int gc_priority);
INTDEF struct type_gc Dee_tpconst instance_gc;

struct Dee_attrhint;

INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL instance_iterattr(DeeTypeObject *tp_self, DeeObject *__restrict self, struct Dee_attriter *iterbuf, size_t bufsize, struct Dee_attrhint const *__restrict hint);
#endif /* CONFIG_BUILDING_DEEMON */

/* Instance-member wrapper objects
 * >> class MyClass {
 * >>     member foo = 42;
 * >> }
 * >> print type MyClass.foo; // DeeInstanceMember_Type
 */
typedef struct Dee_instance_member_object {
	Dee_OBJECT_HEAD
	DREF DeeTypeObject               *im_type;      /* [1..1][const] The user-class type, instances of which implement this member. */
	struct Dee_class_attribute const *im_attribute; /* [1..1][const] The instance attribute (`Dee_CLASS_ATTRIBUTE_FCLASSMEM' shouldn't
	                                                 * be set, though this isn't asserted) that should be accessed. */
} DeeInstanceMemberObject;

DDATDEF DeeTypeObject DeeInstanceMember_Type;
#define DeeInstanceMember_Check(ob)      DeeObject_InstanceOfExact(ob, &DeeInstanceMember_Type) /* `_instancemember' is final */
#define DeeInstanceMember_CheckExact(ob) DeeObject_InstanceOfExact(ob, &DeeInstanceMember_Type)

/* Construct a new instance member for the given `attribute' */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeInstanceMember_New(DeeTypeObject *__restrict class_type,
                      struct Dee_class_attribute const *__restrict attr);

DECL_END

#endif /* !GUARD_DEEMON_CLASS_H */
