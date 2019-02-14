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
#ifndef GUARD_DEEMON_CXX_TUPLE_H
#define GUARD_DEEMON_CXX_TUPLE_H 1

#include "api.h"
#include "object.h"
#include "sequence.h"
#include "../seq.h"
#include "../tuple.h"

DEE_CXX_BEGIN

template<class T>
class tuple: public sequence<T> {
public:
    static DeeTypeObject *classtype() DEE_CXX_NOTHROW { return &DeeTuple_Type; }
    static bool check(DeeObject *__restrict ob) DEE_CXX_NOTHROW { return DeeTuple_Check(ob); }
    static bool checkexact(DeeObject *__restrict ob) DEE_CXX_NOTHROW { return DeeTuple_CheckExact(ob); }
public:
    tuple() DEE_CXX_NOTHROW: sequence<T>(nonnull(Dee_EmptyTuple)) { }
    tuple(std::initializer_list<T> const &items): sequence<T>(inherit(DeeTuple_NewVector(items.size(),(DeeObject **)items.begin()))) { }
    tuple(std::initializer_list<DeeObject *> const &items): sequence<T>(inherit(DeeTuple_NewVector(items.size(),items.begin()))) { }
    tuple(size_t objc, DeeObject **__restrict objv): sequence<T>(inherit(DeeTuple_NewVector(objc,objv))) { }
    tuple(size_t objc, DeeObject *const *__restrict objv): sequence<T>(inherit(DeeTuple_NewVector(objc,objv))) { }
    tuple(size_t objc, T **__restrict objv): sequence<T>(inherit(DeeTuple_NewVector(objc,(DeeObject **)objv))) { }
    tuple(size_t objc, T *const *__restrict objv): sequence<T>(inherit(DeeTuple_NewVector(objc,(DeeObject **)objv))) { }
    DEE_CXX_DEFINE_OBJECT_CONSTRUCTORS(tuple,sequence<T>)
    operator obj_tuple() const { return obj_tuple(*this); }
    tuple types() const { return inherit(DeeTuple_Types(*this)); }
    void append(T const &ob) { if likely(DeeTuple_Check(this->ptr())) this->m_ptr = throw_if_null(DeeTuple_Append(*this,ob)); else sequence<T>::append(ob); }
    void append(DeeObject *__restrict ob) { if likely(DeeTuple_Check(this->ptr())) this->m_ptr = throw_if_null(DeeTuple_Append(*this,ob)); else sequence<T>::append(ob); }
    void appenditer(DeeObject *__restrict iter) { if likely(DeeTuple_Check(this->ptr())) this->m_ptr = throw_if_null(DeeTuple_AppendIterator(*this,iter)); else sequence<T>::appenditer(iter); }
};


DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_TUPLE_H */
