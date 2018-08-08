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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_FASTSEQ_C
#define GUARD_DEEMON_OBJECTS_SEQ_FASTSEQ_C 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/tuple.h>
#include <deemon/list.h>
#include <deemon/none.h>
#include <deemon/int.h>
#include <deemon/thread.h>
#include <deemon/bytes.h>
#include <deemon/string.h>

#include "transform.h"
#include "svec.h"
#include "range.h"
#include "subrange.h"

#include "../../runtime/runtime_error.h"

DECL_BEGIN

STATIC_ASSERT_MSG(DEE_FASTSEQ_NOTFAST == (size_t)-1,
                  "`nsi_getsize_fast' assumes this correlation");


/* Check if `self' is a fast-sequence object, and return its (current)
 * length if it is, or return `DEE_FASTSEQ_NOTFAST' if it isn't.
 * A fast-sequence object is a vector-based object implemented by the
 * deemon C-core, meaning that its size can quickly be determined,
 * and items can quickly be accessed, given their index.
 * The following types function as fast-sequence-compatible:
 *  - tuple
 *  - list
 *  - _refvector      (Returned by `DeeRefVector_New()')
 *  - _sharedvector   (Created by a `ASM_CALL_SEQ' instruction -- `call top, {#X}')
 *  - _subrange       (Only if the sub-ranged sequence is a fast-sequence)
 *  - _transformation (Only if the sequence being transformed is a fast-sequence)
 *  - _intrange
 * Sub-classes of these types are not fast-sequence-compatible. */
PUBLIC size_t DCALL
DeeFastSeq_GetSize(DeeObject *__restrict self) {
 DeeTypeObject *tp_self;
 struct type_seq *seq;
 struct type_nsi *nsi;
 ASSERT_OBJECT(self);
 tp_self = Dee_TYPE(self);
 if ((seq = tp_self->tp_seq) != NULL &&
     (nsi = seq->tp_nsi) != NULL &&
     (nsi->nsi_class == TYPE_SEQX_CLASS_SEQ) &&
     (nsi->nsi_seqlike.nsi_getsize_fast)) {
  ASSERT(nsi->nsi_seqlike.nsi_getitem ||
         nsi->nsi_seqlike.nsi_getitem_fast);
  return (*nsi->nsi_seqlike.nsi_getsize_fast)(self);
 }
 return DEE_FASTSEQ_NOTFAST;
}


/* Returns the `index'th item of `self'.
 * The caller is responsible that `index < DeeFastSeq_GetSize(self)'
 * when `self' is an immutable sequence (anything other than `list' and `_sharedvector').
 * WARNING: These function may _ONLY_ be used if `DeeFastSeq_Check(self)' is true. */
PUBLIC DREF DeeObject *DCALL
DeeFastSeq_GetItem(DeeObject *__restrict self, size_t index) {
 DeeTypeObject *tp_self;
 struct type_seq *seq;
 struct type_nsi *nsi;
 ASSERT_OBJECT(self);
 tp_self = Dee_TYPE(self);
 seq = tp_self->tp_seq;
 ASSERT(seq);
 nsi = seq->tp_nsi;
 ASSERT(nsi);
 ASSERT(nsi->nsi_class == TYPE_SEQX_CLASS_SEQ);
 if (nsi->nsi_seqlike.nsi_getitem_fast) {
  DREF DeeObject *result;
  result = (*nsi->nsi_seqlike.nsi_getitem_fast)(self,index);
  if unlikely(!result)
     err_index_unbound(self,index);
  return result;
 }
 ASSERT(nsi->nsi_seqlike.nsi_getitem);
 return (*nsi->nsi_seqlike.nsi_getitem)(self,index);
}


PUBLIC size_t DCALL
DeeFastSeq_GetSizeNB(DeeObject *__restrict self) {
 DeeTypeObject *tp_self;
 ASSERT_OBJECT(self);
 tp_self = Dee_TYPE(self);
 if (tp_self == &DeeTuple_Type)
     return DeeTuple_SIZE(self);
 if (tp_self == &SharedVector_Type)
     return ((SharedVector *)self)->sv_length;
 if (tp_self == &DeeSubRange_Type) {
  if (DeeFastSeq_GetSizeNB(((SubRange *)self)->sr_seq) == DEE_FASTSEQ_NOTFAST)
      return DEE_FASTSEQ_NOTFAST;
  return ((SubRange *)self)->sr_size;
 }
 return DEE_FASTSEQ_NOTFAST;
}
PUBLIC ATTR_RETNONNULL DREF DeeObject *DCALL
DeeFastSeq_GetItemNB(DeeObject *__restrict self, size_t index) {
 DeeTypeObject *tp_self;
 DREF DeeObject *result;
again:
 ASSERT_OBJECT(self);
 tp_self = Dee_TYPE(self);
 if (tp_self == &DeeTuple_Type) {
  result = DeeTuple_GET(self,index);
  return_reference_(result);
 }
 if (tp_self == &SharedVector_Type) {
  rwlock_read(&((SharedVector *)self)->sv_lock);
  if unlikely(index >= ((SharedVector *)self)->sv_length) {
   rwlock_endread(&((SharedVector *)self)->sv_lock);
   return_none;
  }
  result = ((SharedVector *)self)->sv_vector[index];
  Dee_Incref(result);
  rwlock_endread(&((SharedVector *)self)->sv_lock);
  return result;
 }
 ASSERT(tp_self == &DeeSubRange_Type);
 index += ((SubRange *)self)->sr_begin;
 self   = ((SubRange *)self)->sr_seq;
 goto again;
}


PUBLIC /*owned(Dee_Free)*/DREF DeeObject **DCALL
DeeSeq_AsHeapVector(DeeObject *__restrict self,
                    size_t *__restrict plength) {
 size_t i,fastsize,alloc_size;
 DREF DeeObject **result,*iter,*elem,**new_result;
 fastsize = DeeFastSeq_GetSize(self);
 if (fastsize != DEE_FASTSEQ_NOTFAST) {
  /* Optimization for fast-sequence-compatible objects. */
  *plength = fastsize;
  result = (DREF DeeObject **)Dee_Malloc(fastsize*sizeof(DREF DeeObject *));
  if unlikely(!result) goto err;
  for (i = 0; i < fastsize; ++i) {
   elem = DeeFastSeq_GetItem(self,i);
   if unlikely(!elem) goto err_r_i;
   result[i] = elem;
  }
  goto done;
 }
 /* Must use iterators. */
 iter = DeeObject_IterSelf(self);
 if unlikely(!iter) goto err;
 alloc_size = 16,i = 0;
 result = (DREF DeeObject **)Dee_TryMalloc(alloc_size*sizeof(DREF DeeObject *));
 if unlikely(!result) {
  alloc_size = 1;
  result = (DREF DeeObject **)Dee_Malloc(alloc_size*sizeof(DREF DeeObject *));
  goto err_r_iter;
 }
 /* Iterate items. */
 while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
  ASSERT(i <= alloc_size);
  if unlikely(i >= alloc_size) {
   /* Must allocate more memory. */
   size_t new_alloc_size = alloc_size * 2;
   new_result = (DREF DeeObject **)Dee_TryRealloc(result,new_alloc_size*sizeof(DREF DeeObject *));
   if unlikely(!new_result) {
    new_alloc_size = i+1;
    new_result = (DREF DeeObject **)Dee_Realloc(result,new_alloc_size*sizeof(DREF DeeObject *));
    if unlikely(!new_result) goto err_r_iter_elem;
   }
   result     = new_result;
   alloc_size = new_alloc_size;
  }
  result[i++] = elem; /* Inherit reference. */
  if (DeeThread_CheckInterrupt())
      goto err_r_iter;
 }
 if unlikely(!elem) goto err_r_iter;
 Dee_Decref(iter);
 ASSERT(i <= alloc_size);
 /* Free unused memory. */
 if (i != alloc_size) {
  new_result = (DREF DeeObject **)Dee_TryRealloc(result,i*sizeof(DREF DeeObject *));
  if likely(new_result) result = new_result;
 }
 /* Save the resulting length. */
 *plength = i;
done:
 return result;
err_r_iter_elem:
 Dee_Decref(elem);
err_r_iter:
 Dee_Decref(iter);
err_r_i:
 while (i--)
     Dee_Decref(result[i]);
 Dee_Free(result);
err:
 return NULL;
}




DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_FASTSEQ_C */
