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
#ifndef GUARD_DEEMON_CXX_OBJECT_H
#define GUARD_DEEMON_CXX_OBJECT_H 1

#include "api.h"
#include "../object.h"
#include "../none.h"

DEE_CXX_BEGIN

FORCELOCAL DREF DeeObject *(DCALL incref)(DeeObject *__restrict obj) { Dee_Incref(obj); return obj; }
FORCELOCAL DeeObject *(DCALL decref)(DREF DeeObject *__restrict obj) { Dee_Decref(obj); return obj; }
FORCELOCAL DeeObject *(DCALL decref_nokill)(DREF DeeObject *__restrict obj) { Dee_DecrefNokill(obj); return obj; }
FORCELOCAL void (DCALL decref_dokill)(DREF DeeObject *__restrict obj) { Dee_DecrefDokill(obj); }
FORCELOCAL bool (DCALL decref_ifone)(DREF DeeObject *__restrict obj) { return Dee_DecrefIfOne(obj); }
FORCELOCAL bool (DCALL decref_ifnotone)(DREF DeeObject *__restrict obj) { return Dee_DecrefIfNotOne(obj); }
#ifdef CONFIG_TRACE_REFCHANGES
FORCELOCAL DREF DeeObject *(DCALL incref_traced)(DeeObject *__restrict obj, char const *file, int line) { Dee_Incref_traced(obj,file,line); return obj; }
FORCELOCAL DeeObject *(DCALL decref_traced)(DREF DeeObject *__restrict obj, char const *file, int line) { Dee_Decref_traced(obj,file,line); return obj; }
FORCELOCAL DeeObject *(DCALL decref_nokill_traced)(DREF DeeObject *__restrict obj, char const *file, int line) { Dee_DecrefNokill_traced(obj,file,line); return obj; }
FORCELOCAL void (DCALL decref_dokill_traced)(DREF DeeObject *__restrict obj, char const *file, int line) { Dee_DecrefDokill_traced(obj,file,line); }
FORCELOCAL bool (DCALL decref_ifone_traced)(DREF DeeObject *__restrict obj, char const *file, int line) { return Dee_DecrefIfOne_traced(obj,file,line); }
FORCELOCAL bool (DCALL decref_ifnotone_traced)(DREF DeeObject *__restrict obj, char const *file, int line) { return Dee_DecrefIfNotOne_traced(obj,file,line); }
#define incref(obj)          incref_traced(obj,__FILE__,__LINE__)
#define decref(obj)          decref_traced(obj,__FILE__,__LINE__)
#define decref_nokill(obj)   decref_nokill_traced(obj,__FILE__,__LINE__)
#define decref_dokill(obj)   decref_dokill_traced(obj,__FILE__,__LINE__)
#define decref_ifone(obj)    decref_ifone_traced(obj,__FILE__,__LINE__)
#define decref_ifnotone(obj) decref_ifnotone_traced(obj,__FILE__,__LINE__)
#endif

template<class T> class object_converter;
#define DEE_CXX_IS_CONVERTIBLE(T) \
    typename ::deemon::object_converter< T >::exists* = 0


class object_converter<DeeObject *> {
public:
    typedef int exists;
    static inline DREF DeeObject *get(DeeObject *obj) DEE_CXX_NOTHROW { return incref(obj); }
};



class object_base {
private:
    DREF DeeObject *m_ptr; /* [1..1][lock(CALLER)] The represented object. */
public:
    inline object_base(void) DEE_CXX_NOTHROW:
        m_ptr(incref(Dee_None)) { }
    inline object_base(DREF DeeObject *__restrict inherited_ref) DEE_CXX_NOTHROW:
        m_ptr(inherited_ref) { }
    inline object_base(object_base const &right) DEE_CXX_NOTHROW:
        m_ptr(incref(right.m_ptr)) { }
    inline ~object_base(void) DEE_CXX_NOTHROW { decref(m_ptr); }

    inline DeeObject *ptr() const DEE_CXX_NOTHROW { return m_ptr; }
    inline bool is_none() const DEE_CXX_NOTHROW { return m_ptr == Dee_None; }
};

class object: public object_base {
public:
    template<class T> inline object(T const &x, DEE_CXX_IS_CONVERTIBLE(T))
        : object_base(object_converter<T>::get(x))
    { }
};


DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_OBJECT_H */
