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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_SORT_C
#define GUARD_DEEMON_OBJECTS_SEQ_SORT_C 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/error.h>
#include <deemon/util/string.h>

DECL_BEGIN

PRIVATE int DCALL
mergesort_impl(DREF DeeObject **__restrict dst,
               DeeObject **__restrict temp,
               DREF DeeObject *const *__restrict src,
               size_t objc) {
 int error;
 switch (objc) {

 case 0: break;
 case 1: dst[0] = src[0]; break;

 case 2:
  error = DeeObject_CompareLo(src[0],src[1]);
  if unlikely(error < 0 &&
             !DeeError_Catch(&DeeError_TypeError) &&
             !DeeError_Catch(&DeeError_NotImplemented))
     goto err;
  if (error <= 0) {
   dst[0] = src[1];
   dst[1] = src[0];
  } else {
   dst[0] = src[0];
   dst[1] = src[1];
  }
  break;

 {
  size_t s1,s2;
  DeeObject **iter1,**iter2;
 default:
  s1 = objc/2;
  s2 = objc-s1;
  error = mergesort_impl(temp,dst,src,s1);
  if unlikely(error < 0) goto err;
  error = mergesort_impl(temp+s1,dst+s1,src+s1,s2);
  if unlikely(error < 0) goto err;
  iter1 = temp;
  iter2 = temp+s1;
  while (s1 && s2) {
   error = DeeObject_CompareLo(*iter1,*iter2);
   if unlikely(error < 0) {
    if (!DeeError_Catch(&DeeError_TypeError) &&
        !DeeError_Catch(&DeeError_NotImplemented))
        goto err;
   }
   if (error <= 0) {
    *dst++ = *iter2++;
    --s2;
   } else {
    *dst++ = *iter1++;
    --s1;
   }
  }
  if (s1) {
   ASSERT(!s2);
   MEMCPY_PTR(dst,iter1,s1);
  } else if (s2) {
   MEMCPY_PTR(dst,iter2,s2);
  }
 } break;

 }
 return 0;
err:
 return -1;
}


PRIVATE int DCALL
mergesort_impl_p(DREF DeeObject **__restrict dst,
                 DeeObject **__restrict temp,
                 DREF DeeObject *const *__restrict src,
                 size_t objc,
                 DeeObject *__restrict pred_lo) {
 DREF DeeObject *pred_result;
 int error;
 switch (objc) {

 case 0: break;
 case 1: dst[0] = src[0]; break;

 case 2:
  pred_result = DeeObject_Call(pred_lo,2,(DeeObject **)&src[0]);
  if (!pred_result) error = -1;
  else {
   error = DeeObject_Bool(pred_result);
   Dee_Decref(pred_result);
  }
  if unlikely(error < 0 &&
             !DeeError_Catch(&DeeError_TypeError) &&
             !DeeError_Catch(&DeeError_NotImplemented))
     goto err;
  if (error <= 0) {
   dst[0] = src[1];
   dst[1] = src[0];
  } else {
   dst[0] = src[0];
   dst[1] = src[1];
  }
  break;

 {
  size_t s1,s2;
  DeeObject **iter1,**iter2;
 default:
  s1 = objc/2;
  s2 = objc-s1;
  error = mergesort_impl_p(temp,dst,src,s1,pred_lo);
  if unlikely(error < 0) goto err;
  error = mergesort_impl_p(temp+s1,dst+s1,src+s1,s2,pred_lo);
  if unlikely(error < 0) goto err;
  iter1 = temp;
  iter2 = temp+s1;
  while (s1 && s2) {
   DeeObject *argv[2];
   argv[0] = *iter1;
   argv[1] = *iter2;
   pred_result = DeeObject_Call(pred_lo,2,argv);
   if (!pred_result) error = -1;
   else {
    error = DeeObject_Bool(pred_result);
    Dee_Decref(pred_result);
   }
   if unlikely(error < 0 &&
              !DeeError_Catch(&DeeError_TypeError) &&
              !DeeError_Catch(&DeeError_NotImplemented))
      goto err;
   if (error <= 0) {
    *dst++ = *iter2++;
    --s2;
   } else {
    *dst++ = *iter1++;
    --s1;
   }
  }
  if (s1) {
   ASSERT(!s2);
   MEMCPY_PTR(dst,iter1,s1);
  } else if (s2) {
   MEMCPY_PTR(dst,iter2,s2);
  }
 } break;

 }
 return 0;
err:
 return -1;
}



PRIVATE int DCALL
insertsort_impl(DREF DeeObject **__restrict dst,
                DREF DeeObject *const *__restrict src,
                size_t objc) {
 int temp;
 size_t i,j;
 for (i = 0; i < objc; ++i) {
  DeeObject *ob = src[i];
  for (j = 0; j < i; ++j) {
   /* Check if we need to insert the object in this location. */
   temp = DeeObject_CompareLo(ob,dst[j]);
   if unlikely(temp < 0 &&
              !DeeError_Catch(&DeeError_TypeError) &&
              !DeeError_Catch(&DeeError_NotImplemented))
      goto err;
   if (temp > 0) break;
  }
  MEMMOVE_PTR(&dst[j+1],&dst[j],i-j);
  dst[j] = ob;
 }
 return 0;
err:
 return -1;
}

PRIVATE int DCALL
insertsort_impl_p(DREF DeeObject **__restrict dst,
                  DREF DeeObject *const *__restrict src,
                  size_t objc,
                  DeeObject *__restrict pred_lo) {
 DREF DeeObject *pred_result; int temp;
 size_t i,j;
 for (i = 0; i < objc; ++i) {
  DeeObject *argv[2];
  argv[0] = src[i];
  for (j = 0; j < i; ++j) {
   /* Check if we need to insert the object in this location. */
   argv[1] = dst[j];
   pred_result = DeeObject_Call(pred_lo,2,argv);
   if (!pred_result) temp = -1;
   else {
    temp = DeeObject_Bool(pred_result);
    Dee_Decref(pred_result);
   }
   if unlikely(temp < 0 &&
              !DeeError_Catch(&DeeError_TypeError) &&
              !DeeError_Catch(&DeeError_NotImplemented))
      goto err;
   if (temp > 0) break;
  }
  MEMMOVE_PTR(&dst[j+1],&dst[j],i-j);
  dst[j] = argv[0];
 }
 return 0;
err:
 return -1;
}



INTERN int DCALL
DeeSeq_MergeSort(DREF DeeObject **__restrict dst,
                 DREF DeeObject *const *__restrict src,
                 size_t objc, DeeObject *pred_lo) {
 int result = 0;
 ASSERT(dst != src);
 switch (objc) {
 case 0: break;
 case 1: dst[0] = src[0]; break;

 {
  int temp;
 case 2:
  /* Optimization for sorting 2 objects. */
  if (pred_lo) {
   DREF DeeObject *pred_result;
   pred_result = DeeObject_Call(pred_lo,2,(DeeObject **)&src[0]);
   if unlikely(!pred_result) goto err;
   temp = DeeObject_Bool(pred_result);
   Dee_Decref(pred_result);
  } else {
   temp = DeeObject_CompareLo(src[0],src[1]);
  }
  if unlikely(temp < 0 &&
             !DeeError_Catch(&DeeError_TypeError) &&
             !DeeError_Catch(&DeeError_NotImplemented))
     goto err;
  if (temp <= 0) {
   dst[0] = src[0];
   dst[1] = src[1];
  } else {
   dst[0] = src[1];
   dst[1] = src[0];
  }
 } break;

 {
  DeeObject **temp;
 default:
  /* Default case: Do an actual merge-sort. */
  temp = (DeeObject **)Dee_TryMalloc(objc*sizeof(DeeObject *));
  if unlikely(!temp) {
   /* Use a fallback sorting function */
   result = pred_lo
          ? insertsort_impl_p(dst,src,objc,pred_lo)
          : insertsort_impl(dst,src,objc);
  } else {
   result = pred_lo
          ? mergesort_impl_p(dst,temp,src,objc,pred_lo)
          : mergesort_impl(dst,temp,src,objc);
   Dee_Free(temp);
  }
 } break;
 }
 return result;
err:
 return -1;
}



INTERN int DCALL
DeeSeq_InsertionSort(DREF DeeObject **__restrict dst,
                     DREF DeeObject *const *__restrict src,
                     size_t objc, DeeObject *pred_lo) {
 int result = 0;
 ASSERT(dst != src);
 switch (objc) {
 case 0: break;
 case 1: dst[0] = src[0]; break;

 {
  int temp;
 case 2:
  /* Optimization for sorting 2 objects. */
  if (pred_lo) {
   DREF DeeObject *pred_result;
   pred_result = DeeObject_Call(pred_lo,2,(DeeObject **)&src[0]);
   if unlikely(!pred_result) goto err;
   temp = DeeObject_Bool(pred_result);
   Dee_Decref(pred_result);
  } else {
   temp = DeeObject_CompareLo(src[0],src[1]);
  }
  if unlikely(temp < 0 &&
             !DeeError_Catch(&DeeError_TypeError) &&
             !DeeError_Catch(&DeeError_NotImplemented))
     goto err;
  if (temp <= 0) {
   dst[0] = src[0];
   dst[1] = src[1];
  } else {
   dst[0] = src[1];
   dst[1] = src[0];
  }
 } break;

 default:
  result = pred_lo
         ? insertsort_impl_p(dst,src,objc,pred_lo)
         : insertsort_impl(dst,src,objc);
  break;
 }
 return result;
err:
 return -1;
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_SORT_C */
