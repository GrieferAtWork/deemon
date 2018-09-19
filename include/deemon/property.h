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
#ifndef GUARD_DEEMON_PROPERTY_H
#define GUARD_DEEMON_PROPERTY_H 1

#include "api.h"
#include "object.h"

DECL_BEGIN

typedef struct property_object DeePropertyObject;

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
#define DeeProperty_Check(ob)      DeeObject_InstanceOf(ob,&DeeProperty_Type)
#define DeeProperty_CheckExact(ob) DeeObject_InstanceOfExact(ob,&DeeProperty_Type)

DECL_END

#endif /* !GUARD_DEEMON_PROPERTY_H */
