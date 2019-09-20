/* Copyright (c) 2019 Griefer@Work                                            *
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
#ifdef __INTELLISENSE__
#include "cfunction.c"
#define VARARGS
#endif

#ifdef VARARGS
INTERN DREF DeeObject *DCALL
cfunction_call_v(DeeCFunctionTypeObject *__restrict tp_self,
                 void (*self)(void),
                 size_t argc, DeeObject **argv)
#else /* VARARGS */
INTERN DREF DeeObject *DCALL
cfunction_call(DeeCFunctionTypeObject *__restrict tp_self,
               void (*self)(void),
               size_t argc, DeeObject **argv)
#endif /* !VARARGS */
{
#ifdef VARARGS
	size_t n_varargs;
	DeeTypeObject *dee_va_type;
	DeeObject *dee_va_arg;
	ffi_type **dee_va_ffi_types, **dee_va_ffi_types_end;
#endif /* VARARGS */
	void *wbuf, **argp_iter;
	union argument *iter, *end;
	DREF DeeObject *result, **arg_iter;
	ffi_type **ffi_argtyv;
#define ret_mem wbuf // Return memory is located at the start of the wbuffer
#ifdef VARARGS
	if (argc < tp_self->ft_argc) {
		DeeError_Throwf(&DeeError_TypeError,
		                "Function `(%k)%p' requires at least %Iu arguments when %Iu were given",
		                tp_self, self, tp_self->ft_argc, argc);
		goto err;
	}
#else /* VARARGS */
	if (argc != tp_self->ft_argc) {
		DeeError_Throwf(&DeeError_TypeError,
		                "Function `(%k)%p' requires %Iu arguments when %Iu were given",
		                tp_self, self, tp_self->ft_argc, argc);
		goto err;
	}
#endif /* !VARARGS */
#ifdef VARARGS
	n_varargs = argc - tp_self->ft_argc;
#define va_data_size                                                    \
	(argc * (sizeof(union argument) + /* Storage for argument value. */ \
	         sizeof(void *) +         /* Pointer to argument data. */   \
	         sizeof(ffi_type *) /* Pointer to argument types. */))
	/* Pointer the memory used for the argument types */
#define va_argmem_mem   (union argument *)((uintptr_t)wbuf + tp_self->ft_woff_argmem)
#define va_argptr_mem   (void **)((uintptr_t)wbuf + tp_self->ft_woff_argmem + (argc * sizeof(union argument)))
#define va_argtypes_mem (ffi_type **)((uintptr_t)wbuf + tp_self->ft_woff_argmem + (argc * (sizeof(union argument) + sizeof(void *))))
#endif /* VARARGS */

#ifdef VARARGS
	wbuf = Dee_AMalloc(tp_self->ft_wsize + va_data_size);
#else /* VARARGS */
	wbuf = Dee_AMalloc(tp_self->ft_wsize);
#endif /* !VARARGS */
	if unlikely(!wbuf)
		goto err;

	/* Initialize arguments */
	iter = (union argument *)((void *)((uintptr_t)wbuf + tp_self->ft_woff_argmem));
#ifdef VARARGS
	end       = (union argument *)((void *)((uintptr_t)wbuf + tp_self->ft_woff_variadic_argmem));
	argp_iter = (void **)((uintptr_t)iter + argc * sizeof(union argument));
	ASSERT(iter == va_argmem_mem);
	ASSERT(argp_iter == va_argptr_mem);
#else /* VARARGS */
	end  = (union argument *)((void *)((uintptr_t)wbuf + tp_self->ft_woff_argptr));
	ASSERTF(end == iter + tp_self->ft_argc, "Invalid wbuf cache");
	argp_iter = (void **)end;
#endif /* !VARARGS */

	if (iter != end) {
		arg_iter = argv;
#define argt tp_self->ft_argv[(size_t)(arg_iter - argv)]
		ffi_argtyv = tp_self->ft_ffi_arg_type_v;
		for (;;) {
			DeeObject *arg = *arg_iter;
			*argp_iter     = (void *)iter;
			switch ((*ffi_argtyv)->type) {

			case FFI_TYPE_VOID: break;
			case FFI_TYPE_INT:
				if (DeeObject_AsInt(arg, &iter->i))
					goto err_wbuf;
				break;

			case FFI_TYPE_FLOAT: {
				double temp;
				if (DeeObject_AsDouble(arg, &temp))
					goto err_wbuf;
				iter->f = (float)temp;
			}	break;

			case FFI_TYPE_DOUBLE:
				if (DeeObject_AsDouble(arg, &iter->d))
					goto err_wbuf;
				break;

#if FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
			case FFI_TYPE_LONGDOUBLE: {
				double temp;
				if (DeeObject_AsDouble(arg, &temp))
					goto err_wbuf;
				iter->ld = (long double)temp;
			}	break;
#endif /* FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE */

			case FFI_TYPE_UINT8:
				if (DeeObject_AsUInt8(arg, &iter->u8))
					goto err_wbuf;
				break;

			case FFI_TYPE_SINT8:
				if (DeeObject_AsInt8(arg, &iter->s8))
					goto err_wbuf;
				break;

			case FFI_TYPE_UINT16:
				if (DeeObject_AsUInt16(arg, &iter->u16))
					goto err_wbuf;
				break;

			case FFI_TYPE_SINT16:
				if (DeeObject_AsInt16(arg, &iter->s16))
					goto err_wbuf;
				break;

			case FFI_TYPE_UINT32:
				if (DeeObject_AsUInt32(arg, &iter->u32))
					goto err_wbuf;
				break;

			case FFI_TYPE_SINT32:
				if (DeeObject_AsInt32(arg, &iter->s32))
					goto err_wbuf;
				break;

			case FFI_TYPE_UINT64:
				if (DeeObject_AsUInt64(arg, &iter->u64))
					goto err_wbuf;
				break;

			case FFI_TYPE_SINT64:
				if (DeeObject_AsInt64(arg, &iter->s64))
					goto err_wbuf;
				break;

			case FFI_TYPE_STRUCT:
				if (DeeObject_AssertType(arg, (DeeTypeObject *)argt))
					goto err_wbuf;
				iter->p = DeeStruct_Data(arg);
				break;

			case FFI_TYPE_POINTER: {
				DeeSTypeObject *arg_type;
				arg_type = argt;
				if (DeeLValueType_Check(arg_type)) {
					if (DeeObject_AssertTypeExact(arg, (DeeTypeObject *)arg_type))
						goto err_wbuf;
					iter->p = ((struct lvalue_object *)arg)->l_ptr.ptr;
				} else {
					ASSERT(DeePointerType_Check(arg_type));
					if (DeeObject_AsPointer(arg, ((DeePointerTypeObject *)arg_type)->pt_orig,
					                        (union pointer *)&iter->p))
						goto err_wbuf;
				}
			}	break;

			default: iter->u64 = 0; break;
			}
#ifdef VARARGS
			++argv, ++argp_iter;
#endif /* VARARGS */
			if (++iter == end)
				break;
#ifndef VARARGS
			++argv, ++argp_iter;
#endif /* !VARARGS */
			++ffi_argtyv;
			++arg_iter;
		}
#undef argt
	}
#ifdef VARARGS
	if (n_varargs) {
		dee_va_ffi_types = va_argtypes_mem;
		// Copy non-vararg types
		memcpy(dee_va_ffi_types, tp_self->ft_ffi_arg_type_v,
		       tp_self->ft_argc * sizeof(ffi_type *));
		dee_va_ffi_types += tp_self->ft_argc;
		dee_va_ffi_types_end = dee_va_ffi_types + n_varargs;
		ASSERT(dee_va_ffi_types != dee_va_ffi_types_end);
		for (;;) {
			dee_va_arg = *argv, dee_va_type = Dee_TYPE(dee_va_arg);
			if (dee_va_type == &DeeString_Type) {
				/* String argument --> Pass the pointer for the UTF-8 encoding. */
				*dee_va_ffi_types = &ffi_type_pointer;
				iter->p           = (void *)DeeString_STR(dee_va_arg);
				*argp_iter        = (void *)iter;
			} else if (DeePointerType_Check(dee_va_type)) {
				/* Pointer argument --> cast to 'void *' */
				*dee_va_ffi_types = &ffi_type_pointer;
				iter->p           = ((struct pointer_object *)dee_va_arg)->p_ptr.ptr;
				*argp_iter        = (void *)iter;
			} else if (DeeLValueType_Check(dee_va_type)) {
				/* Lvalue argument --> cast to lvalue-base */
				dee_va_type       = (DeeTypeObject *)((DeeLValueTypeObject *)dee_va_type)->lt_orig;
				*dee_va_ffi_types = stype_ffitype((DeeSTypeObject *)dee_va_type);
				*argp_iter        = ((struct lvalue_object *)dee_va_arg)->l_ptr.ptr;
			} else if (dee_va_type == &DeeNone_Type) {
				/* none --> NULL pointer */
				*dee_va_ffi_types = &ffi_type_pointer;
				iter->p           = NULL;
				*argp_iter        = (void *)iter;
			} else if (dee_va_type == &DeeBool_Type) {
				/* bool --> int 0/1 */
				*dee_va_ffi_types = &ffi_type_sint;
				iter->i           = DeeBool_IsTrue(dee_va_arg) ? 1 : 0;
				*argp_iter        = (void *)iter;
			} else if (dee_va_type == &DeeFloat_Type) {
				/* float (core type) --> double */
				*dee_va_ffi_types = &ffi_type_double;
				iter->d           = DeeFloat_VALUE(dee_va_arg);
				*argp_iter        = (void *)iter;
			} else if (dee_va_type == &DeeInt_Type) {
				/* int (core type) --> int / unsigned int (limited) */
#if __SIZEOF_INT__ == 4
				if (DeeInt_TryAsS32(dee_va_arg, &iter->s32))
					*dee_va_ffi_types = &ffi_type_sint;
				else {
					if (DeeInt_AsU32(dee_va_arg, &iter->u32))
						goto err_wbuf;
					*dee_va_ffi_types = &ffi_type_uint;
				}
#elif __SIZEOF_INT__ == 8
				if (DeeInt_TryAsS64(dee_va_arg, &iter->s64)) {
					*dee_va_ffi_types = &ffi_type_sint;
				} else {
					if (DeeInt_AsU64(dee_va_arg, &iter->u64))
						goto err_wbuf;
					*dee_va_ffi_types = &ffi_type_uint;
				}
#elif __SIZEOF_INT__ <= 4
				int32_t temp;
				if (DeeInt_TryAsS32(dee_va_arg, &temp)) {
					iter->i           = (int)temp;
					*dee_va_ffi_types = &ffi_type_sint;
				} else {
					if (DeeInt_TryAsU32(dee_va_arg, (uint32_t *)&temp))
						goto err_wbuf;
					iter->u           = (unsigned int)*(uint32_t *)&temp;
					*dee_va_ffi_types = &ffi_type_uint;
				}
#endif
				*argp_iter = (void *)iter;
			} else {
				/* Fallback: Any structured object. */
				if (DeeObject_AssertType((DeeObject *)dee_va_type, &DeeSType_Type))
					goto err_wbuf;
				*dee_va_ffi_types = stype_ffitype((DeeSTypeObject *)dee_va_type);
				if unlikely(!*dee_va_ffi_types)
					goto err_wbuf;
				/* Prefer hard copies */
				switch ((*dee_va_ffi_types)->size) {

				case 1:
					iter->u8 = *(uint8_t *)DeeStruct_Data(dee_va_arg);
					goto def_var_data;

				case 2:
					iter->u16 = *(uint16_t *)DeeStruct_Data(dee_va_arg);
					goto def_var_data;

				case 4:
					iter->u32 = *(uint32_t *)DeeStruct_Data(dee_va_arg);
def_var_data:
					*argp_iter = (void *)iter;
					break;

				case 8:
					iter->u64 = *(uint64_t *)DeeStruct_Data(dee_va_arg);
					goto def_var_data;

				default:
					/* Inline pointer to structure data */
					*argp_iter = (void *)DeeStruct_Data(dee_va_arg);
					break;
				}
			}
			if (++dee_va_ffi_types == dee_va_ffi_types_end)
				break;
			++argp_iter, ++argv, ++iter;
		}
	}
#endif /* VARARGS */

#ifdef CONFIG_HAVE_CTYPES_SEH_GUARD
	__try
#elif defined(CONFIG_HAVE_CTYPES_KOS_GUARD)
	TRY
#endif /* Guard... */
	{
#ifdef VARARGS
		if (n_varargs) {
			ffi_cif va_cif;
			ffi_status error;
			error = ffi_prep_cif_var(&va_cif, tp_self->ft_ffi_cif.abi,
			                         (unsigned int)tp_self->ft_argc,
			                         (unsigned int)argc,
			                         tp_self->ft_ffi_return_type,
			                         va_argtypes_mem);

			if (error != FFI_OK) {
				DeeError_Throwf(&DeeError_RuntimeError,
				                "Failed to create variadic function closure (%d)",
				                (int)error);
				goto err_wbuf;
			}
			ffi_call(&va_cif, self, ret_mem, va_argptr_mem);
		} else
#endif /* VARARGS */
		{
			ffi_call(&tp_self->ft_ffi_cif, self, ret_mem,
			         (void **)((uintptr_t)wbuf + tp_self->ft_woff_argptr));
		}
	}
#ifdef CONFIG_HAVE_CTYPES_SEH_GUARD
	__except (ctypes_seh_guard((struct _EXCEPTION_POINTERS *)_exception_info())) {
		goto err_wbuf;
	}
#elif defined(CONFIG_HAVE_CTYPES_KOS_GUARD)
	EXCEPT {
		ctypes_kos_guard();
		goto err_wbuf;
	}
#endif /* CONFIG_HAVE_CTYPES_SEH_GUARD */

	if (tp_self->ft_orig == &DeeCVoid_Type) {
		result = Dee_None;
		Dee_Incref(result);
	} else {
		size_t datasize;
		DeeSTypeObject *orig_type = tp_self->ft_orig;
		datasize                  = orig_type->st_base.tp_init.tp_alloc.tp_instance_size;
		if unlikely(orig_type->st_base.tp_flags & TP_FGC) {
			/* This can happen when the user creates their own
			 * classes that are derived from structured types. */
			result = (DeeObject *)DeeGCObject_Malloc(datasize);
		} else {
			result = (DeeObject *)DeeObject_Malloc(datasize);
		}
		if unlikely(!result)
			goto done_wbuf;
		/* Construct a new structured type that is returned as result. */
		DeeObject_Init(result, (DeeTypeObject *)orig_type);
		memcpy(DeeStruct_Data(result), ret_mem,
		       datasize - sizeof(DeeObject));
	}
#undef ret_mem
done_wbuf:
	Dee_AFree(wbuf);
done:
	return result;
err_wbuf:
	result = NULL;
	goto done_wbuf;
err:
	result = NULL;
	goto done;
}


#undef VARARGS
